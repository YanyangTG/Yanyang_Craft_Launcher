#include "homepage.h"
#include "ui_homepage.h"

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
    , currentAccountType("microsoft")
{
    ui->setupUi(this);

    connect(ui->microsoftBtn, &QPushButton::clicked, [this]() {
        ui->microsoftBtn->setChecked(true);
        ui->offlineBtn->setChecked(false);
        ui->thirdpartyBtn->setChecked(false);
        ui->accountTypeLabel->setText("正版账号");
        currentAccountType = "microsoft";
    });

    connect(ui->offlineBtn, &QPushButton::clicked, [this]() {
        ui->offlineBtn->setChecked(true);
        ui->microsoftBtn->setChecked(false);
        ui->thirdpartyBtn->setChecked(false);
        ui->accountTypeLabel->setText("离线账号");
        currentAccountType = "offline";
    });

    connect(ui->thirdpartyBtn, &QPushButton::clicked, [this]() {
        ui->thirdpartyBtn->setChecked(true);
        ui->microsoftBtn->setChecked(false);
        ui->offlineBtn->setChecked(false);
        ui->accountTypeLabel->setText("第三方账号");
        currentAccountType = "thirdparty";
    });

    connect(ui->launchButton, &QPushButton::clicked, this, &HomePage::launchGameClicked);
}

HomePage::~HomePage()
{
    delete ui;
}

QString HomePage::getPlayerName() const
{
    return ui->playerNameEdit->text().trimmed();
}

QString HomePage::getAccountType() const
{
    return currentAccountType;
}
