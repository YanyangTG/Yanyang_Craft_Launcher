QT += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    runmcclient.cpp \
    homepage.cpp \
    versionpage.cpp \
    settingspage.cpp \
    downloadpage.cpp

HEADERS += \
    mainwindow.h \
    runmcclient.h \
    homepage.h \
    versionpage.h \
    settingspage.h \
    downloadpage.h

FORMS += \
    mainwindow.ui \
    homepage.ui \
    versionpage.ui \
    settingspage.ui \
    downloadpage.ui


RESOURCES += \
    resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32-msvc*:
    DEFINES += _CRT_SECURE_NO_WARNINGS
