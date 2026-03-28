#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QPropertyAnimation>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap splashPixmap(":/resources/icon.png");
    QSplashScreen splash(splashPixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    splash.show();
    a.processEvents();

    MainWindow w;

    QTimer::singleShot(2000, &w, [&w]() {
        w.show();
        w.fadeIn(500);
    });

    QTimer::singleShot(2000, &splash, [&splash]() {
        splash.close();
    });

    return a.exec();
}
