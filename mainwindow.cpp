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

    QWidget *homePage = new QWidget();
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);

    QLabel *welcomeLabel = new QLabel("欢迎使用 Yanyang Craft Launcher");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 24px; color: #333333;");
    homeLayout->addWidget(welcomeLabel);

    homeLayout->addStretch();

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    QPushButton *launchButton = new QPushButton("启动游戏");
    launchButton->setFixedSize(200, 50);
    launchButton->setStyleSheet(
        "QPushButton { "
        "background-color: #64b5f6; "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "padding: 10px 20px; "
        "font-size: 18px; "
        "font-weight: bold;"
        "} "
        "QPushButton:hover { "
        "background-color: #42a5f5;"
        "} "
        "QPushButton:pressed { "
        "background-color: #1e88e5;"
        "}"
    );

    bottomLayout->addWidget(launchButton);
    bottomLayout->setContentsMargins(0, 0, 30, 30);

    homeLayout->addLayout(bottomLayout);

    setupSettingsPage();
    setupDownloadPage();

    contentStack->addWidget(homePage);
    contentStack->addWidget(settingsPage);
    contentStack->addWidget(downloadPage);

    loadSettings();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSettingsPage()
{
    settingsPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(settingsPage);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QScrollArea::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);
    scrollLayout->setContentsMargins(10, 10, 10, 10);

    QGroupBox *generalGroup = new QGroupBox("常规设置");
    generalGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *generalLayout = new QVBoxLayout(generalGroup);
    generalLayout->setSpacing(15);

    autoStartCheckBox = new QCheckBox("开机自启动");
    minimizeToTrayCheckBox = new QCheckBox("最小化到系统托盘");
    autoStartCheckBox->setStyleSheet("QCheckBox { spacing: 10px; font-size: 14px; }");
    minimizeToTrayCheckBox->setStyleSheet("QCheckBox { spacing: 10px; font-size: 14px; }");

    generalLayout->addWidget(autoStartCheckBox);
    generalLayout->addWidget(minimizeToTrayCheckBox);

    QGroupBox *appearanceGroup = new QGroupBox("外观设置");
    appearanceGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceGroup);
    appearanceLayout->setSpacing(15);

    QHBoxLayout *themeLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("主题选择:");
    themeLabel->setStyleSheet("font-size: 14px;");
    themeComboBox = new QComboBox();
    themeComboBox->addItem("浅色", "light");
    themeComboBox->addItem("深色", "dark");
    themeComboBox->setFixedWidth(150);
    themeComboBox->setStyleSheet(
        "QComboBox { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 8px; "
        "font-size: 14px;"
        "} "
        "QComboBox::drop-down { "
        "border: none; "
        "width: 30px;"
        "} "
        "QComboBox::down-arrow { "
        "image: none; "
        "border: none;"
        "}"
    );
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeComboBox);
    themeLayout->addStretch();

    appearanceLayout->addLayout(themeLayout);

    QGroupBox *javaGroup = new QGroupBox("Java 设置");
    javaGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );

    QVBoxLayout *javaLayout = new QVBoxLayout(javaGroup);
    javaLayout->setSpacing(15);

    QHBoxLayout *javaPathLayout = new QHBoxLayout();
    javaPathLabel = new QLabel("未选择 Java 路径");
    javaPathLabel->setStyleSheet("font-size: 14px; color: #666666;");
    javaPathLabel->setWordWrap(true);
    javaPathButton = new QPushButton("浏览...");
    javaPathButton->setFixedWidth(100);
    javaPathButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3daee9; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "padding: 8px 15px; "
        "font-size: 14px;"
        "} "
        "QPushButton:hover { "
        "background-color: #2d9ddb;"
        "}"
    );

    QPushButton *refreshButton = new QPushButton("刷新");
    refreshButton->setFixedWidth(80);
    refreshButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4caf50; "
        "color: white; "
        "border: none; "
        "border-radius: 5px; "
        "padding: 8px 15px; "
        "font-size: 14px;"
        "} "
        "QPushButton:hover { "
        "background-color: #45a049;"
        "}"
    );

    javaPathLayout->addWidget(javaPathLabel, 1);
    javaPathLayout->addWidget(refreshButton);
    javaPathLayout->addWidget(javaPathButton);

    javaPathListWidget = new QListWidget();
    javaPathListWidget->setMaximumHeight(150);
    javaPathListWidget->setStyleSheet(
        "QListWidget { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 5px;"
        "} "
        "QListWidget::item { "
        "padding: 8px; "
        "border-bottom: 1px solid #e0e0e0;"
        "} "
        "QListWidget::item:selected { "
        "background-color: #e3f2fd; "
        "color: #333333;"
        "} "
        "QListWidget::item:hover { "
        "background-color: #e8eaf6;"
        "}"
    );

    javaLayout->addLayout(javaPathLayout);
    javaLayout->addWidget(javaPathListWidget);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        javaPathListWidget->clear();
        QStringList javaPaths = findJavaPaths();
        for (const QString &path : javaPaths) {
            javaPathListWidget->addItem(path);
        }
        if (javaPathListWidget->count() > 0) {
            javaPathListWidget->setCurrentRow(0);
            javaPathLabel->setText(javaPathListWidget->currentItem()->text());
        }
    });

    QGroupBox *memoryGroup = new QGroupBox("内存分配");
    memoryGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *memoryLayout = new QVBoxLayout(memoryGroup);
    memoryLayout->setSpacing(15);

    QHBoxLayout *memoryInputLayout = new QHBoxLayout();

    QLabel *memoryMinLabel = new QLabel("1GB");
    memoryMinLabel->setStyleSheet("font-size: 12px; color: #999999;");

    memorySlider = new QSlider(Qt::Horizontal);
    memorySlider->setRange(1024, 16384);
    memorySlider->setSingleStep(256);
    memorySlider->setPageStep(512);
    memorySlider->setValue(4096);
    memorySlider->setFixedHeight(30);

    QLabel *memoryMaxLabel = new QLabel("16GB");
    memoryMaxLabel->setStyleSheet("font-size: 12px; color: #999999;");

    QSpinBox *memorySpinBox = new QSpinBox();
    memorySpinBox->setRange(1024, 16384);
    memorySpinBox->setValue(4096);
    memorySpinBox->setSuffix(" MB");
    memorySpinBox->setFixedWidth(120);
    memorySpinBox->setStyleSheet(
        "QSpinBox { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 5px 10px; "
        "font-size: 14px;"
        "} "
        "QSpinBox::up-button { border: none; }"
        "QSpinBox::down-button { border: none; }"
    );

    memoryInputLayout->addWidget(memoryMinLabel);
    memoryInputLayout->addWidget(memorySlider);
    memoryInputLayout->addWidget(memoryMaxLabel);
    memoryInputLayout->addWidget(memorySpinBox);

    connect(memorySlider, &QSlider::valueChanged, memorySpinBox, &QSpinBox::setValue);
    connect(memorySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), memorySlider, &QSlider::setValue);
    connect(memorySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMemoryValueChanged);

    memorySlider->setStyleSheet(
        "QSlider::groove:horizontal { "
        "border: 1px solid #e0e0e0; "
        "height: 8px; "
        "background: #f5f5f5; "
        "border-radius: 4px; "
        "} "
        "QSlider::handle:horizontal { "
        "background: #3daee9; "
        "border: 1px solid #e0e0e0; "
        "width: 18px; "
        "margin: -2px 0; "
        "border-radius: 9px; "
        "} "
        "QSlider::add-page:horizontal { "
        "background: #f5f5f5; "
        "} "
        "QSlider::sub-page:horizontal { "
        "background: #3daee9;"
        "}"
    );

    memoryValueLabel = new QLabel("2048 MB");
    memoryValueLabel->setMinimumWidth(80);
    memoryValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    memoryValueLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #3daee9;");

    memoryLayout->addLayout(memoryInputLayout);

    QPushButton *saveButton = new QPushButton("保存设置");
    saveButton->setFixedHeight(45);
    saveButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3daee9; "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 20px; "
        "font-size: 16px; "
        "font-weight: bold;"
        "} "
        "QPushButton:hover { "
        "background-color: #2d9ddb;"
        "}"
    );

    scrollLayout->addWidget(generalGroup);
    scrollLayout->addWidget(appearanceGroup);
    scrollLayout->addWidget(javaGroup);
    scrollLayout->addWidget(memoryGroup);

    QGroupBox *aboutGroup = new QGroupBox("关于");
    aboutGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutGroup);
    aboutLayout->setSpacing(10);

    QLabel *versionLabel = new QLabel("版本 v26.1");
    versionLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333333;");

    QLabel *buildLabel = new QLabel("内部构建版本 v260328b1");
    buildLabel->setStyleSheet("font-size: 14px; color: #666666;");

    QLabel *developerLabel = new QLabel("© 2025-2026 晏阳技术组 , 使用 GPLv3 协议开源");
    developerLabel->setStyleSheet("font-size: 14px; color: #666666;");

    aboutLayout->addWidget(versionLabel);
    aboutLayout->addWidget(buildLabel);
    aboutLayout->addWidget(developerLabel);

    scrollLayout->addWidget(aboutGroup);
    scrollLayout->addStretch();
    scrollLayout->addWidget(saveButton);


    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);
    connect(javaPathButton, &QPushButton::clicked,
            this, &MainWindow::onJavaPathChanged);
    connect(memorySlider, &QSlider::valueChanged,
            this, &MainWindow::onMemoryValueChanged);
    connect(saveButton, &QPushButton::clicked,
            this, &MainWindow::onSaveSettingsClicked);
}

void MainWindow::setupDownloadPage()
{
    downloadPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(downloadPage);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QScrollArea::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);
    scrollLayout->setContentsMargins(10, 10, 10, 10);

    QGroupBox *modpackGroup = new QGroupBox("整合包下载");
    modpackGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *modpackLayout = new QVBoxLayout(modpackGroup);
    modpackLayout->setSpacing(15);

    QHBoxLayout *modpackSelectLayout = new QHBoxLayout();
    QLabel *modpackLabel = new QLabel("整合包选择:");
    modpackLabel->setStyleSheet("font-size: 14px;");

    modpackComboBox = new QComboBox();
    modpackComboBox->addItem("官方整合包", "official");
    modpackComboBox->addItem("深水优化整合包", "shenshui");
    modpackComboBox->setFixedWidth(200);
    modpackComboBox->setStyleSheet(
        "QComboBox { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 8px; "
        "font-size: 14px;"
        "} "
        "QComboBox::drop-down { "
        "border: none; "
        "width: 30px;"
        "} "
        "QComboBox::down-arrow { "
        "image: none; "
        "border: none;"
        "}"
    );

    modpackSelectLayout->addWidget(modpackLabel);
    modpackSelectLayout->addWidget(modpackComboBox);
    modpackSelectLayout->addStretch();

    modpackLayout->addLayout(modpackSelectLayout);

    downloadModpackButton = new QPushButton("下载整合包");
    downloadModpackButton->setFixedHeight(45);
    downloadModpackButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4caf50; "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 20px; "
        "font-size: 16px; "
        "font-weight: bold;"
        "} "
        "QPushButton:hover { "
        "background-color: #45a049;"
        "} "
        "QPushButton:pressed { "
        "background-color: #3d8b40;"
        "}"
    );

    modpackLayout->addWidget(downloadModpackButton);

    QGroupBox *javaGroup = new QGroupBox("Java 下载");
    javaGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold;"
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 15px; "
        "padding: 0 5px; "
        "color: #333333;"
        "}"
    );
    QVBoxLayout *javaLayout = new QVBoxLayout(javaGroup);
    javaLayout->setSpacing(15);

    QHBoxLayout *javaSelectLayout = new QHBoxLayout();
    QLabel *javaVersionLabel = new QLabel("Java 版本选择:");
    javaVersionLabel->setStyleSheet("font-size: 14px;");

    javaComboBox = new QComboBox();
    javaComboBox->addItem("Java 21 (JDK 21)", "21");
    javaComboBox->addItem("Java 25 (JDK 25)", "25");
    javaComboBox->setFixedWidth(200);
    javaComboBox->setStyleSheet(
        "QComboBox { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 8px; "
        "font-size: 14px;"
        "} "
        "QComboBox::drop-down { "
        "border: none; "
        "width: 30px;"
        "} "
        "QComboBox::down-arrow { "
        "image: none; "
        "border: none;"
        "}"
    );

    javaSelectLayout->addWidget(javaVersionLabel);
    javaSelectLayout->addWidget(javaComboBox);
    javaSelectLayout->addStretch();

    javaLayout->addLayout(javaSelectLayout);

    downloadJavaButton = new QPushButton("下载 Java");
    downloadJavaButton->setFixedHeight(45);
    downloadJavaButton->setStyleSheet(
        "QPushButton { "
        "background-color: #2196f3; "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 20px; "
        "font-size: 16px; "
        "font-weight: bold;"
        "} "
        "QPushButton:hover { "
        "background-color: #1e88e5;"
        "} "
        "QPushButton:pressed { "
        "background-color: #1976d2;"
        "}"
    );

    javaLayout->addWidget(downloadJavaButton);

    scrollLayout->addWidget(modpackGroup);
    scrollLayout->addWidget(javaGroup);
    scrollLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    connect(downloadModpackButton, &QPushButton::clicked, this, &MainWindow::onDownloadModpackClicked);
    connect(downloadJavaButton, &QPushButton::clicked, this, &MainWindow::onDownloadJavaClicked);
}

void MainWindow::onDownloadModpackClicked()
{
    QString modpackType = modpackComboBox->currentData().toString();
    QString modpackName = modpackComboBox->currentText();

    QString downloadUrl;
    if (modpackType == "official") {
        downloadUrl = "https://example.com/official-modpack.zip";
    } else if (modpackType == "shenshui") {
        downloadUrl = "https://example.com/shenshui-modpack.zip";
    }

    QMessageBox::information(this, "下载整合包", "即将下载：" + modpackName + "\n下载地址：" + downloadUrl);

    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::onDownloadJavaClicked()
{
    QString javaVersion = javaComboBox->currentData().toString();
    QString javaName = javaComboBox->currentText();

    QString downloadUrl;
    if (javaVersion == "21") {
        downloadUrl = "https://www.azul.com/core-post-download/?endpoint=zulu&uuid=180f8cc0-bfb8-4c68-99be-3983102c1c97";
    } else if (javaVersion == "25") {
        downloadUrl = "https://www.azul.com/core-post-download/?endpoint=zulu&uuid=1281d8c2-21fa-4e04-8edf-73c14995237a";
    }

    QMessageBox::information(this, "下载 Java", "即将下载：" + javaName + "\n将自动打开安装包");

    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::loadSettings()
{
    QSettings settings("YanyangTech", "YanyangCraftLauncher");

    autoStartCheckBox->setChecked(settings.value("autoStart", false).toBool());
    minimizeToTrayCheckBox->setChecked(settings.value("minimizeToTray", false).toBool());

    QString theme = settings.value("theme", "light").toString();
    int themeIndex = (theme == "dark") ? 1 : 0;
    themeComboBox->setCurrentIndex(themeIndex);

    QString savedJavaPath = settings.value("javaPath", "").toString();

    javaPathListWidget->clear();
    QStringList javaPaths = findJavaPaths();
    for (const QString &path : javaPaths) {
        javaPathListWidget->addItem(path);
    }

    if (!savedJavaPath.isEmpty()) {
        javaPathLabel->setText(savedJavaPath);
        QList<QListWidgetItem*> items = javaPathListWidget->findItems(savedJavaPath, Qt::MatchExactly);
        if (!items.isEmpty()) {
            javaPathListWidget->setCurrentItem(items.first());
        }
    } else if (javaPathListWidget->count() > 0) {
        javaPathListWidget->setCurrentRow(0);
        javaPathLabel->setText(javaPathListWidget->currentItem()->text());
    } else {
        javaPathLabel->setText("未选择 Java 路径");
    }

    int memory = settings.value("memory", 4096).toInt();
    memorySlider->setValue(memory);
    memoryValueLabel->setText(QString("%1 MB").arg(memory));
}


QStringList MainWindow::findJavaPaths()
{
    QStringList javaPaths;
    QStringList possiblePaths;

    QString envJavaHome = qgetenv("JAVA_HOME");
    if (!envJavaHome.isEmpty()) {
        possiblePaths << envJavaHome + "\\bin\\javaw.exe";
        possiblePaths << envJavaHome + "\\bin\\java.exe";
    }

    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\java.exe";

    for (const QString &path : possiblePaths) {
        if (path.contains("*")) {
            QFileInfo fileInfo(path);
            QDir dir = fileInfo.absoluteDir();
            QString pattern = fileInfo.fileName();
            dir.setNameFilters(QStringList() << pattern);
            dir.setFilter(QDir::Dirs);

            QFileInfoList dirs = dir.entryInfoList();
            for (const QFileInfo &dirInfo : dirs) {
                QString javaPath = dirInfo.absoluteFilePath() + "\\bin\\javaw.exe";
                if (QFile::exists(javaPath) && !javaPaths.contains(javaPath)) {
                    javaPaths << javaPath;
                }

                javaPath = dirInfo.absoluteFilePath() + "\\bin\\java.exe";
                if (QFile::exists(javaPath) && !javaPaths.contains(javaPath)) {
                    javaPaths << javaPath;
                }
            }
        } else {
            if (QFile::exists(path) && !javaPaths.contains(path)) {
                javaPaths << path;
            }
        }
    }

    QProcess process;
    process.start("where javaw.exe");
    process.waitForFinished(5000);
    QByteArray output = process.readAllStandardOutput();
    if (!output.isEmpty()) {
        QString lines = QString::fromLocal8Bit(output).trimmed();
        QStringList pathList = lines.split("\r\n");
        for (const QString &line : pathList) {
            QString javaPath = line.trimmed();
            if (!javaPath.isEmpty() && QFile::exists(javaPath) && !javaPaths.contains(javaPath)) {
                javaPaths << javaPath;
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
            if (!javaPath.isEmpty() && QFile::exists(javaPath) && !javaPaths.contains(javaPath)) {
                javaPaths << javaPath;
            }
        }
    }

    return javaPaths;
}

void MainWindow::saveSettings()
{
    QSettings settings("YanyangTech", "YanyangCraftLauncher");

    settings.setValue("autoStart", autoStartCheckBox->isChecked());
    settings.setValue("minimizeToTray", minimizeToTrayCheckBox->isChecked());
    settings.setValue("theme", themeComboBox->currentData().toString());
    settings.setValue("javaPath", javaPathLabel->text());
    settings.setValue("memory", memorySlider->value());
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
    switchPage(0);
}

void MainWindow::on_downloadButton_clicked()
{
    switchPage(2);
}

void MainWindow::on_settingButton_clicked()
{
    switchPage(1);
}

void MainWindow::onThemeChanged(int)
{
}

void MainWindow::onJavaPathChanged()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择 Java 可执行文件", "",
        "Java Executable (*.exe);;All Files (*)");
    if (!filePath.isEmpty()) {
        javaPathLabel->setText(filePath);
    }
}

void MainWindow::onMemoryValueChanged(int value)
{
    memoryValueLabel->setText(QString("%1 MB").arg(value));
}

void MainWindow::onSaveSettingsClicked()
{
    saveSettings();
}
