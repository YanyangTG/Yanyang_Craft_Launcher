#include "runmcclient.h"
#include <QEventLoop>
#include <QTimer>

RunMcClient::RunMcClient(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    gameProcess = new QProcess(this);

    // 连接进程信号
    connect(gameProcess, &QProcess::started, this, &RunMcClient::onProcessStarted);
    connect(gameProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RunMcClient::onProcessFinished);
    connect(gameProcess, &QProcess::errorOccurred, this, &RunMcClient::onProcessError);
    connect(gameProcess, &QProcess::readyReadStandardOutput, this, &RunMcClient::onReadOutput);
    connect(gameProcess, &QProcess::readyReadStandardError, this, &RunMcClient::onReadOutput);
}

RunMcClient::~RunMcClient()
{
    if (gameProcess->state() == QProcess::Running) {
        gameProcess->terminate();
        gameProcess->waitForFinished(5000);
    }
}

bool RunMcClient::launch(const QString &javaPath, const QString &gameDir, int memoryMB, const QString &versionName)
{
    emit statusMessage("🚀 正在准备启动 Minecraft " + versionName + "...");

    // PCL 启动流程：
    // 1. 检查版本 JSON
    // 2. 下载缺失的 libraries
    // 3. 构建启动参数
    // 4. 启动进程

    QString versionJsonPath = gameDir + "/versions/" + versionName + "/" + versionName + ".json";

    // 步骤 1: 读取或下载版本 JSON
    if (!QFile::exists(versionJsonPath)) {
        emit statusMessage("📥 正在获取版本信息...");
        checkVersion(versionName);

        // 等待版本信息下载完成（实际应该用异步回调）
        QEventLoop loop;
        QTimer::singleShot(10000, &loop, SLOT(quit()));
        loop.exec();
    }

    // 步骤 2: 准备启动环境
    if (!prepareLaunch(javaPath, gameDir, memoryMB)) {
        return false;
    }

    // 步骤 3: 构建命令
    QStringList arguments;
    arguments << buildJvmArguments(javaPath, memoryMB);
    arguments << "-cp" << buildClassPath(gameDir, versionName);
    arguments << findMainClass();
    arguments << buildGameArguments(gameDir, versionName);

    // 步骤 4: 启动进程
    emit statusMessage("⚡ 正在启动 Java 进程...");

    gameProcess->setWorkingDirectory(gameDir);

    QStringList fullArguments;
    fullArguments << buildJvmArguments(javaPath, memoryMB);
    fullArguments << "-cp" << buildClassPath(gameDir, versionName);
    fullArguments << findMainClass();
    fullArguments << buildGameArguments(gameDir, versionName);

    gameProcess->start(javaPath, fullArguments);

    return true;
}

void RunMcClient::checkVersion(const QString &versionName)
{
    QString versionUrl = getMirrorUrl() + "/version/" + versionName;

    emit statusMessage("🌐 从 BMCLAPI 获取版本信息...");

    QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(versionUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onVersionDownloaded(reply);
    });
}

void RunMcClient::onVersionDownloaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject json = doc.object();

        // 保存版本 JSON 到本地
        QString versionName = json["id"].toString();
        QString versionDir = QDir::currentPath() + "/.minecraft/versions/" + versionName;
        QDir().mkpath(versionDir);

        QString versionJsonPath = versionDir + "/" + versionName + ".json";
        QFile file(versionJsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();

            emit statusMessage("✅ 版本信息已保存");

            // 解析关键信息
            mainClass = json["mainClass"].toString();
            libraries = json["libraries"].toArray();

            // 下载 libraries
            if (!libraries.isEmpty()) {
                emit statusMessage("📦 正在检查依赖库...");
                downloadLibraries(QDir::currentPath() + "/.minecraft", libraries);
            }

            emit versionCheckCompleted(true);
        }
    } else {
        lastError = "获取版本信息失败：" + reply->errorString();
        emit statusMessage("❌ " + lastError);
        emit versionCheckCompleted(false);
    }

    reply->deleteLater();
}

bool RunMcClient::prepareLaunch(const QString &javaPath, const QString &gameDir, int /*memoryMB*/)
{
    // 检查 Java 是否存在
    if (!QFile::exists(javaPath)) {
        lastError = "Java 路径不存在：" + javaPath;
        emit statusMessage("❌ " + lastError);
        return false;
    }

    // 检查游戏目录
    if (!QDir(gameDir).exists()) {
        lastError = "游戏目录不存在：" + gameDir;
        emit statusMessage("❌ " + lastError);
        return false;
    }

    // 检查版本 jar
    QDir versionsDir(gameDir + "/versions");
    QFileInfoList versionDirs = versionsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (versionDirs.isEmpty()) {
        lastError = "未找到任何游戏版本";
        emit statusMessage("❌ " + lastError);
        return false;
    }

    return true;
}

bool RunMcClient::downloadLibraries(const QString &gameDir, const QJsonArray &libraries)
{
    int downloaded = 0;
    int skipped = 0;
    int failed = 0;
    int total = libraries.size();

    emit statusMessage(QString("📦 开始检查 %1 个依赖库...").arg(total));

    for (const QJsonValue &libValue : libraries) {
        QJsonObject libObj = libValue.toObject();

        // 检查是否有 rules（某些库有特定规则）
        if (libObj.contains("rules")) {
            // 简化处理：跳过有 rules 的库（如 natives）
            QJsonArray rules = libObj["rules"].toArray();
            bool shouldDownload = false;

            for (const QJsonValue &ruleValue : rules) {
                QJsonObject rule = ruleValue.toObject();
                QString action = rule["action"].toString();
                if (action == "allow") {
                    shouldDownload = true;
                }
            }

            if (!shouldDownload) {
                skipped++;
                continue;
            }
        }

        QString name = libObj["name"].toString();
        if (name.isEmpty()) {
            skipped++;
            continue;
        }

        // 解析库名称格式：group:artifact:version
        QStringList parts = name.split(":");
        if (parts.size() < 3) {
            skipped++;
            continue;
        }

        QString group = parts[0].replace(".", "/");
        QString artifact = parts[1];
        QString version = parts[2];

        // 获取下载路径（优先使用 url 字段）
        QString downloadPath;
        if (libObj.contains("downloads") && libObj["downloads"].isObject()) {
            QJsonObject downloads = libObj["downloads"].toObject();
            if (downloads.contains("jar") && downloads["jar"].isObject()) {
                QJsonObject jarInfo = downloads["jar"].toObject();
                downloadPath = jarInfo["path"].toString();

                // 如果有自定义 URL，使用自定义 URL
                if (jarInfo.contains("url")) {
                    QString customUrl = jarInfo["url"].toString();
                    if (!customUrl.isEmpty()) {
                        // 对于 forge 等自定义源，直接使用原 URL
                        QString finalUrl = customUrl;
                        if (!finalUrl.startsWith("http")) {
                            finalUrl = getMirrorUrl() + "/maven/" + downloadPath;
                        }

                        QString localPath = gameDir + "/libraries/" + downloadPath;
                        if (QFile::exists(localPath)) {
                            downloaded++;
                            continue;
                        }

                        downloadFile(finalUrl, localPath);
                        if (QFile::exists(localPath)) {
                            downloaded++;
                        } else {
                            failed++;
                        }
                        continue;
                    }
                }
            }
        }

        // 默认使用 BMCLAPI Maven 镜像
        if (downloadPath.isEmpty()) {
            downloadPath = group + "/" + artifact + "/" + version + "/" + artifact + "-" + version + ".jar";
        }

        QString mavenPath = downloadPath;
        QString localPath = gameDir + "/libraries/" + mavenPath;

        // 检查是否已存在
        if (QFile::exists(localPath)) {
            downloaded++;
            continue;
        }

        // 从 BMCLAPI 下载
        QString downloadUrl = getMirrorUrl() + "/maven/" + mavenPath;

        emit statusMessage(QString("⬇️ 下载：%1").arg(artifact));

        downloadFile(downloadUrl, localPath);

        if (QFile::exists(localPath)) {
            downloaded++;
            emit statusMessage(QString("✅ 已下载 %1/%2").arg(downloaded).arg(total));
        } else {
            failed++;
            emit statusMessage(QString("❌ 下载失败：%1").arg(artifact));
        }
    }

    emit statusMessage(QString("✅ 依赖库检查完成 - 成功：%1, 跳过：%2, 失败：%3")
                       .arg(downloaded).arg(skipped).arg(failed));
    return failed == 0;
}

bool RunMcClient::downloadFile(const QString &url, const QString &localPath)
{
    QDir().mkpath(QFileInfo(localPath).absolutePath());

    QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(url)));

    // 同步等待下载完成
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            return true;
        }
    }

    reply->deleteLater();
    return false;
}

QStringList RunMcClient::buildJvmArguments(const QString &/*javaPath*/, int memoryMB)
{
    QStringList args;

    // PCL 常用的 JVM 优化参数
    args << "-Xmx" + QString::number(memoryMB) + "M";
    args << "-Xms256M";
    args << "-XX:+UseG1GC";
    args << "-XX:MaxGCPauseMillis=50";
    args << "-XX:-UseAdaptiveSizePolicy";
    args << "-XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe";

    // Java 库路径
    args << "-Djava.library.path=" + QDir::currentPath() + "/.minecraft/bin";

    // 系统属性
    args << "-Dminecraft.client.jar=.minecraft/versions/latest/latest.jar";

    return args;
}

QString RunMcClient::buildClassPath(const QString &gameDir, const QString &versionName)
{
    // 构建类路径，包含所有 libraries
    QStringList classPaths;

    // 添加版本 jar
    QString versionJar = gameDir + "/versions/" + versionName + "/" + versionName + ".jar";
    if (QFile::exists(versionJar)) {
        classPaths << versionJar;
    }

    // 添加 libraries（简化版，实际应该遍历所有 library）
    QString librariesDir = gameDir + "/libraries";
    if (QDir(librariesDir).exists()) {
        // 这里应该递归查找所有 jar 文件
        // 简化处理：只添加常见的几个
        classPaths << librariesDir + "/net/minecraft/client/1.19.2/client-1.19.2.jar";
    }

    return classPaths.join(";");
}

QString RunMcClient::findMainClass() const
{
    return mainClass.isEmpty() ? "net.minecraft.client.main.Main" : mainClass;
}

QStringList RunMcClient::buildGameArguments(const QString &gameDir, const QString &versionName)
{
    QStringList args;

    // PCL 标准启动参数
    args << "--username" << "Player";
    args << "--version" << versionName;
    args << "--gameDir" << gameDir;
    args << "--assetsDir" << gameDir + "/assets";
    args << "--assetIndex" << "latest";
    args << "--uuid" << "00000000-0000-0000-0000-000000000000";
    args << "--accessToken" << "offline";
    args << "--userType" << "mojang";
    args << "--versionType" << "YanyangCraftLauncher";

    return args;
}

void RunMcClient::onProcessStarted()
{
    emit statusMessage("✅ Minecraft 已启动！");
    emit launchStarted();
}

void RunMcClient::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        emit statusMessage("🏁 游戏正常退出 (代码：" + QString::number(exitCode) + ")");
    } else {
        emit statusMessage("⚠️ 游戏异常退出 (代码：" + QString::number(exitCode) + ")");
    }

    emit launchFinished(exitCode);
}

void RunMcClient::onProcessError(QProcess::ProcessError error)
{
    switch (error) {
        case QProcess::FailedToStart:
            lastError = "无法启动游戏进程，请检查 Java 路径是否正确";
            break;
        case QProcess::Crashed:
            lastError = "游戏进程崩溃";
            break;
        case QProcess::Timedout:
            lastError = "启动超时";
            break;
        default:
            lastError = "未知错误";
    }

    emit statusMessage("❌ " + lastError);
}

void RunMcClient::onReadOutput()
{
    // 读取游戏输出（可用于日志显示）
    QByteArray output = gameProcess->readAllStandardOutput();
    QByteArray error = gameProcess->readAllStandardError();

    if (!output.isEmpty()) {
        qDebug() << "[MC]" << QString::fromUtf8(output);
    }

    if (!error.isEmpty()) {
        qWarning() << "[MC Error]" << QString::fromUtf8(error);
    }
}
