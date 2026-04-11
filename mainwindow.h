#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPoint>
#include <QStackedWidget>
#include "runmcclient.h"
#include "homepage.h"
#include "settingspage.h"
#include "downloadpage.h"
#include "versionpage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void fadeIn(int duration);

    QString findJavaPath();
    QStringList findJavaPaths();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void on_minimizeButton_clicked();
    void on_closeButton_clicked();
    void on_runButton_clicked();
    void on_versionButton_clicked();
    void on_downloadButton_clicked();
    void on_settingButton_clicked();
    void onLaunchGameClicked();
    void onDownloadModpackClicked();
    void onDownloadJavaClicked();
    void onSaveSettingsClicked();
    void onRefreshJavaPaths();

private:
    void setupPages();
    void loadSettings();
    void saveSettings();
    void switchPage(int index);

    Ui::MainWindow *ui;
    bool m_mousePressed;
    QPoint m_mousePos;
    QPoint m_windowPos;

    QStackedWidget *contentStack;
    HomePage *homePage;
    VersionPage *versionPage;
    SettingsPage *settingsPage;
    DownloadPage *downloadPage;

    RunMcClient *mcClient;

    QString getJavaVersion(const QString &javaPath);
};

#endif // MAINWINDOW_H
