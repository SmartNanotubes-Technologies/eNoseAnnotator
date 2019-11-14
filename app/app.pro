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

SOURCES += \
    addattributedialog.cpp \
    addselectiondialog.cpp \
    annotation.cpp \
    annotationdatasetmodel.cpp \
    attributeeditor.cpp \
    bargraphwidget.cpp \
    classeditor.cpp \
    classifierwidget.cpp \
    editannotationdatawindow.cpp \
    functionalisationdialog.cpp \
    generalsettings.cpp \
    infowidget.cpp \
    linegraph.cpp \
    main.cpp \
    mainwindow.cpp \
    measurementdata.cpp \
    mvector.cpp \
    qcustomplot.cpp \
    sensorcolor.cpp \
    setsensorfailuresdialog.cpp \
    usbdatasource.cpp \
    usbsettingsdialog.cpp

HEADERS += \
    addattributedialog.h \
    addselectiondialog.h \
    annotation.h \
    annotationdatasetmodel.h \
    attributeeditor.h \
    bargraphwidget.h \
    classeditor.h \
    classifierwidget.h \
    editannotationdatawindow.h \
    functionalisationdialog.h \
    generalsettings.h \
    infowidget.h \
    linegraph.h \
    mainwindow.h \
    measurementdata.h \
    mvector.h \
    qcustomplot.h \
    sensorcolor.h \
    setsensorfailuresdialog.h \
    usbdatasource.h \
    usbsettingsdialog.h

FORMS += \
    addattributedialog.ui \
    addselectiondialog.ui \
    attributeeditor.ui \
    bargraphwidget.ui \
    classeditor.ui \
    classifierwidget.ui \
    editannotationdatawindow.ui \
    functionalisationdialog.ui \
    generalsettings.ui \
    infowidget.ui \
    linegraph.ui \
    mainwindow.ui \
    setsensorfailuresdialog.ui \
    usbsettingsdialog.ui

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

DISTFILES += \
    readme
