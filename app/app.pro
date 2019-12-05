QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# QMAKE_LFLAGS += -no-pie

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# for travis-ci.com:
# create appdir structure
linux-g++ {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target = $$PREFIX/bin
    INSTALLS += target
}


SOURCES += \
    aclass.cpp \
    addattributedialog.cpp \
    attributeeditor.cpp \
    bargraphwidget.cpp \
    classifierwidget.cpp \
    classinputdialog.cpp \
    classselector.cpp \
    datasource.cpp \
    functionalisationdialog.cpp \
    generalsettings.cpp \
    infowidget.cpp \
    linegraphwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    measurementdata.cpp \
    mvector.cpp \
    qcustomplot.cpp \
    sensorcolor.cpp \
    setsensorfailuresdialog.cpp \
    sourcedialog.cpp \
    usbdatasource.cpp \
    usbsettingswidget.cpp

HEADERS += \
    aclass.h \
    addattributedialog.h \
    attributeeditor.h \
    bargraphwidget.h \
    classifierwidget.h \
    classinputdialog.h \
    classselector.h \
    datasource.h \
    functionalisationdialog.h \
    generalsettings.h \
    infowidget.h \
    linegraphwidget.h \
    mainwindow.h \
    measurementdata.h \
    mvector.h \
    qcustomplot.h \
    sensorcolor.h \
    setsensorfailuresdialog.h \
    sourcedialog.h \
    usbdatasource.h \
    usbsettingswidget.h

FORMS += \
    addattributedialog.ui \
    attributeeditor.ui \
    bargraphwidget.ui \
    classifierwidget.ui \
    classselector.ui \
    functionalisationdialog.ui \
    generalsettings.ui \
    infowidget.ui \
    linegraphwidget.ui \
    mainwindow.ui \
    setsensorfailuresdialog.ui \
    sourcedialog.ui \
    usbsettingswidget.ui

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icon.png \
    readme

RESOURCES += \
    eNoseAnnotator.qrc
