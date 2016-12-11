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
    core/peer.cpp \
    core/router.cpp \
    core/router_controller.cpp

HEADERS  += mainwindow.h \
    core/peer.hpp \
    core/router.hpp \
    core/router_controller.hpp

FORMS    += mainwindow.ui
