#include "versionpage.h"
#include "ui_versionpage.h"

VersionPage::VersionPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VersionPage)
{
    ui->setupUi(this);

    connect(ui->installButton, &QPushButton::clicked, this, &VersionPage::installVersionClicked);
}

VersionPage::~VersionPage()
{
    delete ui;
}
