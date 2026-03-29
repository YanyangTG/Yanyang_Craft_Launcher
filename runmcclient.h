#ifndef RUNMCCLIENT_H
#define RUNMCCLIENT_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

class RunMcClient : public QObject
{
    Q_OBJECT

public:
    explicit RunMcClient(QObject *parent = nullptr);
    ~RunMcClient();

    // 启动游戏（PCL 风格）
    bool launch(const QString &javaPath, const QString &gameDir, int memoryMB, const QString &versionName);

    // 检查并下载版本信息
    void checkVersion(const QString &versionName);

    // 获取错误信息
    QString getLastError() const { return lastError; }

signals:
    void launchStarted();
    void launchFinished(int exitCode);
    void versionCheckCompleted(bool success);
    void downloadProgress(qint64 received, qint64 total);
    void statusMessage(const QString &message);

private slots:
    void onVersionDownloaded(QNetworkReply *reply);
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadOutput();

private:
    // PCL 核心启动步骤
    bool prepareLaunch(const QString &javaPath, const QString &gameDir, int memoryMB);
    bool readVersionJson(const QString &versionJsonPath);
    bool downloadLibraries(const QString &gameDir, const QJsonArray &libraries);
    bool downloadFile(const QString &url, const QString &localPath);
    QStringList buildJvmArguments(const QString &javaPath, int memoryMB);
    QStringList buildGameArguments(const QString &gameDir, const QString &versionName);
    QString findMainClass() const;
    QString buildClassPath(const QString &gameDir, const QString &versionName);

    // BMCLAPI 镜像地址（PCL 同款）
    QString getMirrorUrl() const { return "https://bmclapi2.bangbang93.com"; }

    QNetworkAccessManager *networkManager;
    QProcess *gameProcess;
    QString lastError;

    // 版本信息
    QString mainClass;
    QJsonArray libraries;
    QJsonObject minecraftArguments;
};

#endif // RUNMCCLIENT_H
