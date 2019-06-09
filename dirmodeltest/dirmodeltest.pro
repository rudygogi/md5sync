#-------------------------------------------------
#
# Project created by QtCreator 2018-11-28T20:45:10
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dirmodeltest
TEMPLATE = app


SOURCES += main.cpp\
    Md5Model.cpp \
    Md5Processor.cpp \
    Md5TableWidget.cpp \
    MainWindow.cpp \
    PreferencesDialog.cpp

HEADERS  += \
    Md5Model.h \
    Md5Processor.h \
    Md5TableWidget.h \
    MainWindow.h \
    PreferencesDialog.h

FORMS    += \
    Md5TableWidget.ui \
    MainWindow.ui \
    PreferencesDialog.ui
