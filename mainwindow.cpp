#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QSettings>
#include <QScrollArea>
#include <QSpinBox>
#include <QProcess>
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_mousePressed = false;

    mcClient = new RunMcClient(this);

    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::on_minimizeButton_clicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::on_closeButton_clicked);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::on_runButton_clicked);
    connect(ui->versionButton, &QPushButton::clicked, this, &MainWindow::on_versionButton_clicked);
    connect(ui->downloadButton, &QPushButton::clicked, this, &MainWindow::on_downloadButton_clicked);
    connect(ui->settingButton, &QPushButton::clicked, this, &MainWindow::on_settingButton_clicked);

    setMinimumSize(800, 600);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    contentStack = new QStackedWidget(ui->contentArea);
    ui->contentArea->layout()->addWidget(contentStack);

    homePage = new HomePage();
    versionPage = new VersionPage();
    settingsPage = new SettingsPage();
    downloadPage = new DownloadPage();

    contentStack->addWidget(homePage);
    contentStack->addWidget(versionPage);
    contentStack->addWidget(settingsPage);
    contentStack->addWidget(downloadPage);

    connect(homePage, &HomePage::launchGameClicked, this, &MainWindow::onLaunchGameClicked);
    connect(settingsPage, &SettingsPage::settingsSaved, this, &MainWindow::onSaveSettingsClicked);
    connect(settingsPage, &SettingsPage::javaPathRefreshRequested, this, &MainWindow::onRefreshJavaPaths);
    connect(downloadPage, &DownloadPage::downloadModpackClicked, this, &MainWindow::onDownloadModpackClicked);
    connect(downloadPage, &DownloadPage::downloadJavaClicked, this, &MainWindow::onDownloadJavaClicked);

    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPages()
{
}

void MainWindow::onDownloadModpackClicked()
{
    QString modpackType = downloadPage->getSelectedModpack();
    QString modpackName = downloadPage->getSelectedModpack();

    QString downloadUrl;
    if (modpackType == "official_high") {
        downloadUrl = "https://1811814759.v.123pan.cn/1811814759/22200373";
    } else if (modpackType == "official_medium") {
        downloadUrl = "https://1811814759.v.123pan.cn/1811814759/22200371";
    } else if (modpackType == "official_low") {
        downloadUrl = "https://1811814759.v.123pan.cn/1811814759/22200372";
    } else if (modpackType == "shenshui") {
        downloadUrl = "https://1811814759.v.123pan.cn/1811814759/23719321";
    }

    QMessageBox::information(this, "下载整合包", "即将下载：" + modpackName + "\n下载地址：" + downloadUrl);

    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::onDownloadJavaClicked()
{
    QString javaVersion = downloadPage->getSelectedJavaVersion();
    QString javaName = downloadPage->getSelectedJavaVersion();

    QString downloadUrl;
    if (javaVersion == "21") {
        downloadUrl = "https://www.azul.com/core-post-download/?endpoint=zulu&uuid=180f8cc0-bfb8-4c68-99be-3983102c1c97";
    } else if (javaVersion == "25") {
        downloadUrl = "https://www.azul.com/core-post-download/?endpoint=zulu&uuid=1281d8c2-21fa-4e04-8edf-73c14995237a";
    }

    QMessageBox::information(this, "下载 Java", "即将下载：" + javaName + "\n将自动打开安装包");

    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::onLaunchGameClicked()
{
    QString playerName = homePage->getPlayerName();
    if (playerName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入玩家名称！");
        return;
    }

    QString accountType = homePage->getAccountType();

    QString javaPath = settingsPage->getJavaPath();
    if (javaPath.isEmpty() || javaPath == "未选择 Java 路径") {
        QMessageBox::warning(this, "错误", "请先在设置中选择 Java 路径！");
        return;
    }

    int memoryMB = settingsPage->getMemoryMB();
    QString gameDir = QCoreApplication::applicationDirPath() + "/.minecraft";

    if (!QDir(gameDir).exists()) {
        QMessageBox::warning(this, "错误", "未找到 .minecraft 目录！\n请先下载整合包。");
        return;
    }

    QDir versionsDir(gameDir + "/versions");
    QFileInfoList versionDirs = versionsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (versionDirs.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到任何游戏版本！");
        return;
    }

    QString versionName = versionDirs.first().fileName();

    bool success = mcClient->launch(javaPath, gameDir, memoryMB, versionName);

    if (!success) {
        QMessageBox::critical(this, "启动失败", mcClient->getLastError());
    }
}


void MainWindow::loadSettings()
{
    QSettings settings("YanyangTech", "YanyangCraftLauncher");

    settingsPage->setAutoStart(settings.value("autoStart", false).toBool());
    settingsPage->setMinimizeToTray(settings.value("minimizeToTray", false).toBool());

    QString theme = settings.value("theme", "light").toString();
    settingsPage->setTheme(theme);

    QString savedJavaPath = settings.value("javaPath", "").toString();

    QStringList javaPaths = findJavaPaths();
    settingsPage->loadJavaPaths(javaPaths);

    if (!savedJavaPath.isEmpty()) {
        settingsPage->setJavaPath(savedJavaPath);

        for (int i = 0; i < javaPaths.size(); i++) {
            QString path = javaPaths[i];
            int versionStart = path.lastIndexOf(" (");
            if (versionStart != -1) {
                path = path.left(versionStart);
            }
            if (path == savedJavaPath) {
                break;
            }
        }
    } else if (!javaPaths.isEmpty()) {
        QString firstPath = javaPaths.first();
        int versionStart = firstPath.lastIndexOf(" (");
        if (versionStart != -1) {
            firstPath = firstPath.left(versionStart);
        }
        settingsPage->setJavaPath(firstPath);
    } else {
        settingsPage->setJavaPath("未选择 Java 路径");
    }

    int memory = settings.value("memory", 4096).toInt();
    settingsPage->setMemoryMB(memory);
}


QStringList MainWindow::findJavaPaths()
{
    QStringList javaPaths;
    QStringList possiblePaths;
    QSet<QString> uniquePaths;

    QString envJavaHome = qgetenv("JAVA_HOME");
    if (!envJavaHome.isEmpty()) {
        possiblePaths << envJavaHome + "\\bin\\javaw.exe";
        possiblePaths << envJavaHome + "\\bin\\java.exe";
    }

    QString envPath = qgetenv("Path");
    if (!envPath.isEmpty()) {
        QStringList pathList = envPath.split(';', Qt::SkipEmptyParts);
        for (const QString &path : pathList) {
            QString trimmedPath = path.trimmed();
            if (!trimmedPath.isEmpty() && !trimmedPath.contains("?")) {
                possiblePaths << trimmedPath + "\\javaw.exe";
                possiblePaths << trimmedPath + "\\java.exe";
            }
        }
    }

    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\java.exe";

    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\java.exe";

    possiblePaths << "C:\\Program Files\\Zulu\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Zulu\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Zulu\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Zulu\\jdk*\\bin\\java.exe";

    possiblePaths << "C:\\Program Files\\Eclipse Adoptium\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Eclipse Adoptium\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Eclipse Adoptium\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Eclipse Adoptium\\jdk*\\bin\\java.exe";

    possiblePaths << "C:\\Program Files\\Microsoft\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Microsoft\\jdk*\\bin\\java.exe";

    possiblePaths << "C:\\Program Files\\Amazon Corretto\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Amazon Corretto\\jdk*\\bin\\java.exe";

    QString userProfile = qgetenv("USERPROFILE");
    if (!userProfile.isEmpty()) {
        possiblePaths << userProfile + "\\jdks\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\jdks\\*\\bin\\java.exe";
        possiblePaths << userProfile + ".jdks\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + ".jdks\\*\\bin\\java.exe";

        possiblePaths << userProfile + "\\PCL\\Java\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\PCL\\Java\\*\\bin\\java.exe";
        possiblePaths << userProfile + "\\Minecraft\\Java\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\Minecraft\\Java\\*\\bin\\java.exe";
    }

    possiblePaths << "C:\\HMCL\\Java\\*\\bin\\javaw.exe";
    possiblePaths << "C:\\HMCL\\Java\\*\\bin\\java.exe";

#ifdef Q_OS_WIN
    QProcess regProcess;

    regProcess.start("reg", QStringList()
        << "query"
        << "HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit"
        << "/s" << "/f" << "JavaHome"
        << "/reg" << "REG_SZ");

    if (regProcess.waitForFinished(3000)) {
        QByteArray output = regProcess.readAllStandardOutput();
        if (!output.isEmpty()) {
            QString outputStr = QString::fromLocal8Bit(output);
            QStringList lines = outputStr.split("\n", Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.contains("JavaHome") && line.contains("REG_SZ")) {
                    int idx = line.indexOf("REG_SZ");
                    if (idx != -1) {
                        QString javaHome = line.mid(idx + 6).trimmed();
                        if (!javaHome.isEmpty()) {
                            possiblePaths << javaHome + "\\bin\\javaw.exe";
                            possiblePaths << javaHome + "\\bin\\java.exe";
                        }
                    }
                }
            }
        }
    }

    regProcess.start("reg", QStringList()
        << "query"
        << "HKEY_CURRENT_USER\\SOFTWARE\\JavaSoft\\Java Development Kit"
        << "/s" << "/f" << "JavaHome"
        << "/reg" << "REG_SZ");

    if (regProcess.waitForFinished(3000)) {
        QByteArray output = regProcess.readAllStandardOutput();
        if (!output.isEmpty()) {
            QString outputStr = QString::fromLocal8Bit(output);
            QStringList lines = outputStr.split("\n", Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.contains("JavaHome") && line.contains("REG_SZ")) {
                    int idx = line.indexOf("REG_SZ");
                    if (idx != -1) {
                        QString javaHome = line.mid(idx + 6).trimmed();
                        if (!javaHome.isEmpty()) {
                            possiblePaths << javaHome + "\\bin\\javaw.exe";
                            possiblePaths << javaHome + "\\bin\\java.exe";
                        }
                    }
                }
            }
        }
    }
#endif

    QProcess process;

    process.start("where javaw.exe");
    process.waitForFinished(5000);
    QByteArray output = process.readAllStandardOutput();
    if (!output.isEmpty()) {
        QString lines = QString::fromLocal8Bit(output).trimmed();
        QStringList pathList = lines.split("\r\n");
        for (const QString &line : pathList) {
            QString javaPath = line.trimmed();
            if (!javaPath.isEmpty() && !javaPath.contains("?")) {
                uniquePaths.insert(javaPath);
            }
        }
    }

    process.start("where java.exe");
    process.waitForFinished(5000);
    output = process.readAllStandardOutput();
    if (!output.isEmpty()) {
        QString lines = QString::fromLocal8Bit(output).trimmed();
        QStringList pathList = lines.split("\r\n");
        for (const QString &line : pathList) {
            QString javaPath = line.trimmed();
            if (!javaPath.isEmpty() && !javaPath.contains("?")) {
                uniquePaths.insert(javaPath);
            }
        }
    }

    for (const QString &path : possiblePaths) {
        if (path.contains("*")) {
            QFileInfo fileInfo(path);
            QDir dir = fileInfo.absoluteDir();
            QString pattern = fileInfo.fileName();

            dir.setNameFilters(QStringList() << pattern);
            dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

            QFileInfoList dirs = dir.entryInfoList();

            std::sort(dirs.begin(), dirs.end(), [](const QFileInfo &a, const QFileInfo &b) {
                return a.fileName() > b.fileName();
            });

            for (const QFileInfo &dirInfo : dirs) {
                QString baseDir = dirInfo.absoluteFilePath();

                QString javaWPath = baseDir + "\\bin\\javaw.exe";
                if (QFile::exists(javaWPath)) {
                    uniquePaths.insert(javaWPath);
                }

                QString javaPath = baseDir + "\\bin\\java.exe";
                if (QFile::exists(javaPath)) {
                    uniquePaths.insert(javaPath);
                }
            }
        } else {
            if (QFile::exists(path)) {
                uniquePaths.insert(path);
            }
        }
    }

    for (const QString &path : uniquePaths) {
        if (QFile::exists(path)) {
            QFileInfo fileInfo(path);
            QString versionInfo = getJavaVersion(path);

            if (!versionInfo.isEmpty()) {
                javaPaths << path + " (" + versionInfo + ")";
            } else {
                javaPaths << path;
            }
        }
    }

    std::sort(javaPaths.begin(), javaPaths.end(), [](const QString &a, const QString &b) {
        bool aIsJavaW = a.contains("javaw.exe");
        bool bIsJavaW = b.contains("javaw.exe");

        if (aIsJavaW && !bIsJavaW) return true;
        if (!aIsJavaW && bIsJavaW) return false;

        return a < b;
    });

    return javaPaths;
}

QString MainWindow::getJavaVersion(const QString &javaPath)
{
    QProcess process;
    process.start(javaPath, QStringList() << "-version");
    process.waitForFinished(3000);

    QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        QString output = QString::fromLocal8Bit(errorOutput);

        QRegularExpression re(R"(version\s+"([^"]+))");
        QRegularExpressionMatch match = re.match(output);

        if (match.hasMatch()) {
            return "Java " + match.captured(1);
        }

        QRegularExpression re2(R"((\d+\.\d+\.\d+))");
        QRegularExpressionMatch match2 = re2.match(output);
        if (match2.hasMatch()) {
            return "Java " + match2.captured(1);
        }
    }

    return "";
}

void MainWindow::saveSettings()
{
    QSettings settings("YanyangTech", "YanyangCraftLauncher");

    settings.setValue("autoStart", settingsPage->getAutoStart());
    settings.setValue("minimizeToTray", settingsPage->getMinimizeToTray());
    settings.setValue("theme", settingsPage->getTheme());

    QString javaPath = settingsPage->getJavaPath();
    int versionStart = javaPath.lastIndexOf(" (");
    if (versionStart != -1) {
        javaPath = javaPath.left(versionStart);
    }
    settings.setValue("javaPath", javaPath);

    settings.setValue("memory", settingsPage->getMemoryMB());
}

void MainWindow::switchPage(int index)
{
    contentStack->setCurrentIndex(index);
}

void MainWindow::fadeIn(int duration)
{
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (effect) {
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(duration);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && ui->titleBar->geometry().contains(event->pos())) {
        m_mousePressed = true;
        m_mousePos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed) {
        move(event->globalPosition().toPoint() - m_mousePos);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePressed = false;
}

void MainWindow::on_minimizeButton_clicked()
{
    showMinimized();
}

void MainWindow::on_closeButton_clicked()
{
    close();
}

void MainWindow::on_runButton_clicked()
{
    switchPage(0);
}

void MainWindow::on_versionButton_clicked()
{
    switchPage(1);
}

void MainWindow::on_downloadButton_clicked()
{
    switchPage(3);
}

void MainWindow::on_settingButton_clicked()
{
    switchPage(2);
}

void MainWindow::onSaveSettingsClicked()
{
    saveSettings();
    QMessageBox::information(this, "提示", "设置已保存！");
}

void MainWindow::onRefreshJavaPaths()
{
    QStringList javaPaths = findJavaPaths();
    settingsPage->loadJavaPaths(javaPaths);

    if (!javaPaths.isEmpty()) {
        QString firstPath = javaPaths.first();
        int versionStart = firstPath.lastIndexOf(" (");
        if (versionStart != -1) {
            firstPath = firstPath.left(versionStart);
        }
        settingsPage->setJavaPath(firstPath);
    }
}
