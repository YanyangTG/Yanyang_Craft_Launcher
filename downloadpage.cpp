#include "downloadpage.h"
#include "ui_downloadpage.h"

DownloadPage::DownloadPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownloadPage)
{
    ui->setupUi(this);

    connect(ui->downloadModpackButton, &QPushButton::clicked, this, &DownloadPage::downloadModpackClicked);
    connect(ui->downloadJavaButton, &QPushButton::clicked, this, &DownloadPage::downloadJavaClicked);
}

DownloadPage::~DownloadPage()
{
    delete ui;
}

QString DownloadPage::getSelectedModpack() const
{
    return ui->modpackComboBox->currentData().toString();
}

QString DownloadPage::getSelectedJavaVersion() const
{
    return ui->javaComboBox->currentData().toString();
}
