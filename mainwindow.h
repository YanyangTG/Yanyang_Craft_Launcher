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

private:
    void setupSettingsPage();
    void loadSettings();
    void saveSettings();
    void switchPage(int index);

    Ui::MainWindow *ui;
    bool m_mousePressed;
    QPoint m_mousePos;
    QPoint m_windowPos;

    QStackedWidget *contentStack;
    QWidget *settingsPage;

    QCheckBox *autoStartCheckBox;
    QCheckBox *minimizeToTrayCheckBox;
    QComboBox *themeComboBox;
    QLabel *javaPathLabel;
    QPushButton *javaPathButton;
    QSlider *memorySlider;
    QLabel *memoryValueLabel;
};
#endif // MAINWINDOW_H
