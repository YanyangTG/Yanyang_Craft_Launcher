#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

    bool getAutoStart() const;
    bool getMinimizeToTray() const;
    QString getTheme() const;
    QString getJavaPath() const;
    int getMemoryMB() const;

    void setJavaPath(const QString &path);
    void setMemoryMB(int mb);
    void setAutoStart(bool checked);
    void setMinimizeToTray(bool checked);
    void setTheme(const QString &theme);
    void loadJavaPaths(const QStringList &paths);

signals:
    void settingsSaved();
    void javaPathRefreshRequested();

private slots:
    void onJavaPathSelected(QListWidgetItem *item);

private:
    Ui::SettingsPage *ui;
};

#endif // SETTINGSPAGE_H
