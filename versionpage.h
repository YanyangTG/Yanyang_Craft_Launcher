#ifndef VERSIONPAGE_H
#define VERSIONPAGE_H

#include <QWidget>

namespace Ui {
class VersionPage;
}

class VersionPage : public QWidget
{
    Q_OBJECT

public:
    explicit VersionPage(QWidget *parent = nullptr);
    ~VersionPage();

signals:
    void versionSelected(const QString &version);
    void installVersionClicked();

private:
    Ui::VersionPage *ui;
};

#endif // VERSIONPAGE_H
