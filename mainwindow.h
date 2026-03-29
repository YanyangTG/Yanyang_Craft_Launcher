#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPoint>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QListWidget>
#include <QStringList>
#include <QLineEdit>
#include "runmcclient.h"

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
    void onThemeChanged(int index);
    void onJavaPathChanged();
    void onMemoryValueChanged(int value);
    void onSaveSettingsClicked();
    void onDownloadModpackClicked();
    void onDownloadJavaClicked();
    void onLaunchGameClicked();

private:
    void setupSettingsPage();
    void setupDownloadPage();
    void loadSettings();
    void saveSettings();
    void switchPage(int index);

    Ui::MainWindow *ui;
    bool m_mousePressed;
    QPoint m_mousePos;
    QPoint m_windowPos;

    QStackedWidget *contentStack;
    QWidget *settingsPage;
    QWidget *downloadPage;

    QCheckBox *autoStartCheckBox;
    QCheckBox *minimizeToTrayCheckBox;
    QComboBox *themeComboBox;
    QLabel *javaPathLabel;
    QPushButton *javaPathButton;
    QListWidget *javaPathListWidget;
    QSlider *memorySlider;
    QLabel *memoryValueLabel;
    QLabel *memoryMaxLabel;

    QComboBox *modpackComboBox;
    QComboBox *javaComboBox;
    QPushButton *downloadModpackButton;
    QPushButton *downloadJavaButton;

    // Deleted: QStringList findJavaPaths();
    QString getJavaVersion(const QString &javaPath);

    RunMcClient *mcClient;

    // 玩家信息控件
    QComboBox *accountTypeCombo;
    QLineEdit *playerNameEdit;
    QLabel *skinLabel;
    QLabel *loginStatusLabel;

};

#endif // MAINWINDOW_H
