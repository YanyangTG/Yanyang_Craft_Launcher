#include "settingspage.h"
#include "ui_settingspage.h"
#include <QFileDialog>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsPage)
{
    ui->setupUi(this);

    connect(ui->javaPathButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "选择 Java 可执行文件", "",
            "Java Executable (*.exe);;All Files (*)");
        if (!filePath.isEmpty()) {
            setJavaPath(filePath);
        }
    });

    connect(ui->refreshJavaButton, &QPushButton::clicked, this, &SettingsPage::javaPathRefreshRequested);

    connect(ui->javaPathListWidget, &QListWidget::itemClicked, this, &SettingsPage::onJavaPathSelected);

    connect(ui->memorySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->memorySlider, &QSlider::setValue);
    connect(ui->memorySlider, &QSlider::valueChanged, ui->memorySpinBox, &QSpinBox::setValue);

    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsPage::settingsSaved);
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

bool SettingsPage::getAutoStart() const
{
    return ui->autoStartCheckBox->isChecked();
}

bool SettingsPage::getMinimizeToTray() const
{
    return ui->minimizeToTrayCheckBox->isChecked();
}

QString SettingsPage::getTheme() const
{
    return ui->themeComboBox->currentData().toString();
}

QString SettingsPage::getJavaPath() const
{
    return ui->javaPathLabel->text();
}

int SettingsPage::getMemoryMB() const
{
    return ui->memorySlider->value();
}

void SettingsPage::setJavaPath(const QString &path)
{
    ui->javaPathLabel->setText(path);
}

void SettingsPage::setMemoryMB(int mb)
{
    ui->memorySlider->setValue(mb);
}

void SettingsPage::setAutoStart(bool checked)
{
    ui->autoStartCheckBox->setChecked(checked);
}

void SettingsPage::setMinimizeToTray(bool checked)
{
    ui->minimizeToTrayCheckBox->setChecked(checked);
}

void SettingsPage::setTheme(const QString &theme)
{
    int index = (theme == "dark") ? 1 : 0;
    ui->themeComboBox->setCurrentIndex(index);
}

void SettingsPage::loadJavaPaths(const QStringList &paths)
{
    ui->javaPathListWidget->clear();
    for (const QString &path : paths) {
        QString displayVersion = "";
        QString actualPath = path;

        int versionStart = path.lastIndexOf(" (");
        if (versionStart != -1) {
            displayVersion = path.mid(versionStart + 2, path.length() - versionStart - 3);
            actualPath = path.left(versionStart);
        }

        QString displayText;
        if (!displayVersion.isEmpty()) {
            displayText = displayVersion + "  ·  " + actualPath;
        } else {
            displayText = actualPath;
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, actualPath);
        ui->javaPathListWidget->addItem(item);
    }
}

void SettingsPage::onJavaPathSelected(QListWidgetItem *item)
{
    QString selectedPath = item->data(Qt::UserRole).toString();
    ui->javaPathLabel->setText(selectedPath);
}
