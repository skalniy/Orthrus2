#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T16:19:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Orthrus2
TEMPLATE = app


INCLUDEPATH += "/usr/local/include"

LIBS    += -L"/usr/local/lib" \
        -lboost_system \
        -lpthread


SOURCES += main.cpp\
        mainwindow.cpp \
        routercontroller.cpp \
        router.cpp \
        peer.cpp

HEADERS  += mainwindow.h \
        routercontroller.h \
        router.h \
        peer.h

FORMS    += mainwindow.ui
