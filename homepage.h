#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

namespace Ui {
class HomePage;
}

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    QString getPlayerName() const;
    QString getAccountType() const;

signals:
    void launchGameClicked();

private:
    Ui::HomePage *ui;
    QString currentAccountType;
};

#endif // HOMEPAGE_H
