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

    // 初始化 RunMcClient
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

    QWidget *homePage = new QWidget();
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);

    // 保存控件引用以便后续访问
    this->accountTypeCombo = nullptr;
    this->playerNameEdit = nullptr;
    this->skinLabel = nullptr;
    this->loginStatusLabel = nullptr;

    homeLayout->addStretch();

    // 右下角区域
    QVBoxLayout *rightBottomLayout = new QVBoxLayout();
    rightBottomLayout->setSpacing(15);

// 玩家信息面板
    QGroupBox *playerGroup = new QGroupBox("玩家信息");
    playerGroup->setMaximumWidth(250);
    playerGroup->setMinimumWidth(250);
    playerGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
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
    QVBoxLayout *playerLayout = new QVBoxLayout(playerGroup);
    playerLayout->setSpacing(15);

    // 账号类型选择 - 横向图标按钮
    QHBoxLayout *accountTypeLayout = new QHBoxLayout();
    accountTypeLayout->setSpacing(10);

    // 正版账号按钮
    QPushButton *microsoftBtn = new QPushButton();
    microsoftBtn->setFixedSize(70, 70);
    microsoftBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #f5f5f5; "
        "border: 2px solid #e0e0e0; "
        "border-radius: 10px; "
        "} "
        "QPushButton:hover { "
        "background-color: #e8e8e8; "
        "border: 2px solid #3daee9;"
        "} "
        "QPushButton:checked { "
        "background-color: #e3f2fd; "
        "border: 2px solid #3daee9;"
        "}"
    );
    microsoftBtn->setIcon(QIcon(":/resources/run.png"));
    microsoftBtn->setIconSize(QSize(32, 32));
    microsoftBtn->setCheckable(true);
    microsoftBtn->setChecked(true);
    microsoftBtn->setProperty("accountType", "microsoft");

    // 离线账号按钮
    QPushButton *offlineBtn = new QPushButton();
    offlineBtn->setFixedSize(70, 70);
    offlineBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #f5f5f5; "
        "border: 2px solid #e0e0e0; "
        "border-radius: 10px; "
        "} "
        "QPushButton:hover { "
        "background-color: #e8e8e8; "
        "border: 2px solid #3daee9;"
        "} "
        "QPushButton:checked { "
        "background-color: #e3f2fd; "
        "border: 2px solid #3daee9;"
        "}"
    );
    offlineBtn->setIcon(QIcon(":/resources/run.png"));
    offlineBtn->setIconSize(QSize(32, 32));
    offlineBtn->setCheckable(true);
    offlineBtn->setProperty("accountType", "offline");

    // 第三方账号按钮
    QPushButton *thirdpartyBtn = new QPushButton();
    thirdpartyBtn->setFixedSize(70, 70);
    thirdpartyBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #f5f5f5; "
        "border: 2px solid #e0e0e0; "
        "border-radius: 10px; "
        "} "
        "QPushButton:hover { "
        "background-color: #e8e8e8; "
        "border: 2px solid #3daee9;"
        "} "
        "QPushButton:checked { "
        "background-color: #e3f2fd; "
        "border: 2px solid #3daee9;"
        "}"
    );
    thirdpartyBtn->setIcon(QIcon(":/resources/run.png"));
    thirdpartyBtn->setIconSize(QSize(32, 32));
    thirdpartyBtn->setCheckable(true);
    thirdpartyBtn->setProperty("accountType", "thirdparty");

    accountTypeLayout->addWidget(microsoftBtn);
    accountTypeLayout->addWidget(offlineBtn);
    accountTypeLayout->addWidget(thirdpartyBtn);
    playerLayout->addLayout(accountTypeLayout);

    // 账号类型标签
    QLabel *accountTypeLabel = new QLabel("正版账号");
    accountTypeLabel->setAlignment(Qt::AlignCenter);
    accountTypeLabel->setStyleSheet("font-size: 13px; color: #666666; padding: 5px;");
    playerLayout->addWidget(accountTypeLabel);

    // 皮肤头像和玩家名
    QVBoxLayout *profileLayout = new QVBoxLayout();
    profileLayout->setSpacing(10);

    // 皮肤头像
    QLabel *skinLabel = new QLabel();
    skinLabel->setFixedSize(64, 64);
    skinLabel->setAlignment(Qt::AlignCenter);
    skinLabel->setStyleSheet(
        "QLabel { "
        "background-color: #f0f0f0; "
        "border: 2px solid #e0e0e0; "
        "border-radius: 8px;"
        "}"
    );
    skinLabel->setText("👤");
    skinLabel->setScaledContents(true);

    // 玩家名输入
    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("输入你的游戏昵称");
    nameEdit->setFixedHeight(40);
    nameEdit->setStyleSheet(
        "QLineEdit { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 5px 10px; "
        "font-size: 14px;"
        "} "
        "QLineEdit:focus { "
        "border: 2px solid #3daee9; "
        "background-color: #ffffff;"
        "}"
    );

    profileLayout->addWidget(skinLabel, 0, Qt::AlignHCenter);
    profileLayout->addWidget(nameEdit);
    playerLayout->addLayout(profileLayout);

    // 登录状态
    QLabel *loginStatusLabel = new QLabel("未登录");
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setStyleSheet("font-size: 13px; color: #999999;");
    playerLayout->addWidget(loginStatusLabel);

    rightBottomLayout->addWidget(playerGroup);

    // 启动游戏按钮
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

    rightBottomLayout->addWidget(launchButton);
    rightBottomLayout->setAlignment(launchButton, Qt::AlignRight);

    // 添加右侧弹簧，将内容推到右边
    QHBoxLayout *mainHBoxLayout = new QHBoxLayout();
    mainHBoxLayout->addStretch();
    mainHBoxLayout->addLayout(rightBottomLayout);
    mainHBoxLayout->setContentsMargins(0, 0, 30, 30);

    homeLayout->addLayout(mainHBoxLayout);

    // 保存控件引用
    this->skinLabel = skinLabel;
    this->playerNameEdit = nameEdit;
    this->loginStatusLabel = loginStatusLabel;

    // 账号类型按钮组互斥逻辑
    connect(microsoftBtn, &QPushButton::clicked, [this, microsoftBtn, offlineBtn, thirdpartyBtn, accountTypeLabel]() {
        microsoftBtn->setChecked(true);
        offlineBtn->setChecked(false);
        thirdpartyBtn->setChecked(false);
        accountTypeLabel->setText("正版账号");
    });

    connect(offlineBtn, &QPushButton::clicked, [this, microsoftBtn, offlineBtn, thirdpartyBtn, accountTypeLabel]() {
        offlineBtn->setChecked(true);
        microsoftBtn->setChecked(false);
        thirdpartyBtn->setChecked(false);
        accountTypeLabel->setText("离线账号");
    });

    connect(thirdpartyBtn, &QPushButton::clicked, [this, microsoftBtn, offlineBtn, thirdpartyBtn, accountTypeLabel]() {
        thirdpartyBtn->setChecked(true);
        microsoftBtn->setChecked(false);
        offlineBtn->setChecked(false);
        accountTypeLabel->setText("第三方账号");
    });

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
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    javaPathLabel->setStyleSheet("font-size: 14px; color: #666666; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");
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
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "padding: 5px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif; "
        "font-size: 13px;"
        "} "
        "QListWidget::item { "
        "padding: 10px 8px; "
        "border-bottom: 1px solid #e0e0e0; "
        "color: #666666;"
        "} "
        "QListWidget::item:selected { "
        "background-color: #e3f2fd; "
        "color: #1976d2;"
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
            // 分离版本信息和路径
            QString displayVersion = "";
            QString actualPath = path;

            int versionStart = path.lastIndexOf(" (");
            if (versionStart != -1) {
                displayVersion = path.mid(versionStart + 2, path.length() - versionStart - 3); // 去除括号
                actualPath = path.left(versionStart);
            }

            // 创建自定义项：版本（高亮）+ 路径（灰色）
            QString displayText;
            if (!displayVersion.isEmpty()) {
                displayText = displayVersion + "  ·  " + actualPath;
            } else {
                displayText = actualPath;
            }

            QListWidgetItem *item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, actualPath); // 存储实际路径
            javaPathListWidget->addItem(item);
        }
        if (javaPathListWidget->count() > 0) {
            javaPathListWidget->setCurrentRow(0);
            javaPathLabel->setText(javaPathListWidget->item(0)->data(Qt::UserRole).toString());
        }
    });

    // 添加列表项点击事件 - 一键更改 Java 路径
    connect(javaPathListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString selectedPath = item->data(Qt::UserRole).toString();
        javaPathLabel->setText(selectedPath);
    });

    QGroupBox *memoryGroup = new QGroupBox("内存分配");
    memoryGroup->setStyleSheet(
        "QGroupBox { "
        "background-color: #ffffff; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding: 15px; "
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    memoryMinLabel->setStyleSheet("font-size: 12px; color: #999999; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

    memorySlider = new QSlider(Qt::Horizontal);
    memorySlider->setRange(1024, 16384);
    memorySlider->setSingleStep(256);
    memorySlider->setPageStep(512);
    memorySlider->setValue(4096);
    memorySlider->setFixedHeight(30);

    QLabel *memoryMaxLabel = new QLabel("16GB");
    memoryMaxLabel->setStyleSheet("font-size: 12px; color: #999999; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

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
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    memoryValueLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #3daee9; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    versionLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333333; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

    QLabel *buildLabel = new QLabel("内部构建版本 v260328b1");
    buildLabel->setStyleSheet("font-size: 14px; color: #666666; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

    QLabel *developerLabel = new QLabel("© 2025-2026 晏阳技术组 , 使用 GPLv3 协议开源");
    developerLabel->setStyleSheet("font-size: 14px; color: #666666; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    modpackLabel->setStyleSheet("font-size: 14px; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

    modpackComboBox = new QComboBox();
    modpackComboBox->addItem("官方高配版整合包", "official_high");
    modpackComboBox->addItem("官方中配版整合包", "official_medium");
    modpackComboBox->addItem("官方低配版整合包", "official_low");
    modpackComboBox->addItem("深水优化整合包", "shenshui");
    modpackComboBox->setFixedWidth(200);
    modpackComboBox->setStyleSheet(
        "QComboBox { "
        "background-color: #f5f5f5; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 5px; "
        "padding: 8px; "
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
    javaVersionLabel->setStyleSheet("font-size: 14px; font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;");

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
        "font-size: 14px; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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
        "font-weight: bold; "
        "font-family: 'Source Han Sans CN', '思源黑体', 'Microsoft YaHei', sans-serif;"
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

void MainWindow::onLaunchGameClicked()
{
    // 检查玩家名称
    QString playerName = playerNameEdit ? playerNameEdit->text().trimmed() : "";
    if (playerName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入玩家名称！");
        return;
    }

    // 获取账号类型（从按钮状态判断）
    QString accountType = "offline"; // 默认离线
    // 由于无法直接访问按钮，这里简化处理，默认使用离线账号
    // 如果需要精确判断，可以将按钮保存为成员变量

    // 获取 Java 路径
    QString javaPath = javaPathLabel->text();
    if (javaPath.isEmpty() || javaPath == "未选择 Java 路径") {
        QMessageBox::warning(this, "错误", "请先在设置中选择 Java 路径！");
        return;
    }

    int memoryMB = memorySlider->value();
    QString gameDir = QCoreApplication::applicationDirPath() + "/.minecraft";

    if (!QDir(gameDir).exists()) {
        QMessageBox::warning(this, "错误", "未找到 .minecraft 目录！\n请先下载整合包。");
        return;
    }

    // 获取版本名称
    QDir versionsDir(gameDir + "/versions");
    QFileInfoList versionDirs = versionsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (versionDirs.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到任何游戏版本！");
        return;
    }

    QString versionName = versionDirs.first().fileName();

    // 使用 RunMcClient 启动（PCL 方式）
    bool success = mcClient->launch(javaPath, gameDir, memoryMB, versionName);

    if (!success) {
        QMessageBox::critical(this, "启动失败", mcClient->getLastError());
    }
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
        // 分离版本信息和路径
        QString displayVersion = "";
        QString actualPath = path;

        int versionStart = path.lastIndexOf(" (");
        if (versionStart != -1) {
            displayVersion = path.mid(versionStart + 2, path.length() - versionStart - 3);
            actualPath = path.left(versionStart);
        }

        // 创建自定义项：版本（高亮）+ 路径（灰色）
        QString displayText;
        if (!displayVersion.isEmpty()) {
            displayText = displayVersion + "  ·  " + actualPath;
        } else {
            displayText = actualPath;
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, actualPath);
        javaPathListWidget->addItem(item);
    }

    if (!savedJavaPath.isEmpty()) {
        javaPathLabel->setText(savedJavaPath);

        // 查找匹配的路径
        for (int i = 0; i < javaPathListWidget->count(); i++) {
            QListWidgetItem *item = javaPathListWidget->item(i);
            if (item->data(Qt::UserRole).toString() == savedJavaPath) {
                javaPathListWidget->setCurrentItem(item);
                break;
            }
        }
    } else if (javaPathListWidget->count() > 0) {
        javaPathListWidget->setCurrentRow(0);
        javaPathLabel->setText(javaPathListWidget->item(0)->data(Qt::UserRole).toString());
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
    QSet<QString> uniquePaths; // 使用 Set 自动去重

    // ========== 方法 1: JAVA_HOME 环境变量 ==========
    QString envJavaHome = qgetenv("JAVA_HOME");
    if (!envJavaHome.isEmpty()) {
        possiblePaths << envJavaHome + "\\bin\\javaw.exe";
        possiblePaths << envJavaHome + "\\bin\\java.exe";
    }

    // ========== 方法 2: Path 中的 java 命令 ==========
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

    // ========== 方法 3: 常见安装目录（通配符扫描）==========
    // 标准安装位置
    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Java\\jre*\\bin\\java.exe";

    // 32 位兼容
    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Java\\jre*\\bin\\java.exe";

    // Azul Zulu（PCL 常用的 Java 发行版）
    possiblePaths << "C:\\Program Files\\Zulu\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Zulu\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Zulu\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Zulu\\jdk*\\bin\\java.exe";

    // Adoptium/Temurin
    possiblePaths << "C:\\Program Files\\Eclipse Adoptium\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Eclipse Adoptium\\jdk*\\bin\\java.exe";
    possiblePaths << "C:\\Program Files (x86)\\Eclipse Adoptium\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files (x86)\\Eclipse Adoptium\\jdk*\\bin\\java.exe";

    // Microsoft Build of OpenJDK
    possiblePaths << "C:\\Program Files\\Microsoft\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Microsoft\\jdk*\\bin\\java.exe";

    // Amazon Corretto
    possiblePaths << "C:\\Program Files\\Amazon Corretto\\jdk*\\bin\\javaw.exe";
    possiblePaths << "C:\\Program Files\\Amazon Corretto\\jdk*\\bin\\java.exe";

    // 用户目录下的 Java（便携版或手动安装）
    QString userProfile = qgetenv("USERPROFILE");
    if (!userProfile.isEmpty()) {
        possiblePaths << userProfile + "\\jdks\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\jdks\\*\\bin\\java.exe";
        possiblePaths << userProfile + ".jdks\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + ".jdks\\*\\bin\\java.exe";

        // 常见第三方启动器目录
        possiblePaths << userProfile + "\\PCL\\Java\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\PCL\\Java\\*\\bin\\java.exe";
        possiblePaths << userProfile + "\\Minecraft\\Java\\*\\bin\\javaw.exe";
        possiblePaths << userProfile + "\\Minecraft\\Java\\*\\bin\\java.exe";
    }

    // HMCL 启动器的 Java 目录
    possiblePaths << "C:\\HMCL\\Java\\*\\bin\\javaw.exe";
    possiblePaths << "C:\\HMCL\\Java\\*\\bin\\java.exe";

    // ========== 方法 4: Windows 注册表搜索 ==========
#ifdef Q_OS_WIN
    // 使用 reg query 命令查询注册表中的 Java 安装信息
    QProcess regProcess;

    // 查询 HKEY_LOCAL_MACHINE 中的 JavaSoft 键
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

    // 查询 HKEY_CURRENT_USER 中的 Java 安装
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

    // ========== 方法 5: where 命令搜索 PATH ==========
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

    // ========== 处理所有可能的路径 ==========
    for (const QString &path : possiblePaths) {
        if (path.contains("*")) {
            // 通配符路径处理
            QFileInfo fileInfo(path);
            QDir dir = fileInfo.absoluteDir();
            QString pattern = fileInfo.fileName();

            dir.setNameFilters(QStringList() << pattern);
            dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

            QFileInfoList dirs = dir.entryInfoList();

            // 按版本号排序（优先选择更新的版本）
            std::sort(dirs.begin(), dirs.end(), [](const QFileInfo &a, const QFileInfo &b) {
                return a.fileName() > b.fileName(); // 降序排列
            });

            for (const QFileInfo &dirInfo : dirs) {
                QString baseDir = dirInfo.absoluteFilePath();

                // 添加 javaw.exe（优先）
                QString javaWPath = baseDir + "\\bin\\javaw.exe";
                if (QFile::exists(javaWPath)) {
                    uniquePaths.insert(javaWPath);
                }

                // 添加 java.exe
                QString javaPath = baseDir + "\\bin\\java.exe";
                if (QFile::exists(javaPath)) {
                    uniquePaths.insert(javaPath);
                }
            }
        } else {
            // 直接路径
            if (QFile::exists(path)) {
                uniquePaths.insert(path);
            }
        }
    }

    // ========== 验证并格式化结果 ==========
    for (const QString &path : uniquePaths) {
        if (QFile::exists(path)) {
            // 获取 Java 版本信息（可选）
            QFileInfo fileInfo(path);
            QString versionInfo = getJavaVersion(path);

            if (!versionInfo.isEmpty()) {
                javaPaths << path + " (" + versionInfo + ")";
            } else {
                javaPaths << path;
            }
        }
    }

    // ========== 排序：优先 javaw.exe，然后按路径 ==========
    std::sort(javaPaths.begin(), javaPaths.end(), [](const QString &a, const QString &b) {
        bool aIsJavaW = a.contains("javaw.exe");
        bool bIsJavaW = b.contains("javaw.exe");

        if (aIsJavaW && !bIsJavaW) return true;
        if (!aIsJavaW && bIsJavaW) return false;

        return a < b;
    });

    return javaPaths;
}

// 辅助函数：获取 Java 版本
QString MainWindow::getJavaVersion(const QString &javaPath)
{
    QProcess process;
    process.start(javaPath, QStringList() << "-version");
    process.waitForFinished(3000);

    QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        QString output = QString::fromLocal8Bit(errorOutput);

        // 提取版本信息，例如："java version \"21.0.1\" 2023-10-17 LTS"
        QRegularExpression re(R"(version\s+"([^"]+))");
        QRegularExpressionMatch match = re.match(output);

        if (match.hasMatch()) {
            return "Java " + match.captured(1);
        }

        // 尝试其他格式
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

    settings.setValue("autoStart", autoStartCheckBox->isChecked());
    settings.setValue("minimizeToTray", minimizeToTrayCheckBox->isChecked());
    settings.setValue("theme", themeComboBox->currentData().toString());

    // 提取实际路径（去除版本信息）
    QString javaPath = javaPathLabel->text();
    int versionStart = javaPath.lastIndexOf(" (");
    if (versionStart != -1) {
        javaPath = javaPath.left(versionStart);
    }
    settings.setValue("javaPath", javaPath);

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
