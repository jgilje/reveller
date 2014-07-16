#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T08:10:06
#
#-------------------------------------------------

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SidPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    siditemmodel.cpp \
    siditem.cpp \
    connectdialog.cpp \
    sidheader.cpp \
    sidinfo.cpp

HEADERS  += mainwindow.h \
    siditemmodel.h \
    siditem.h \
    connectdialog.h \
    sidheader.h \
    sidinfo.h

FORMS    += mainwindow.ui \
    connectdialog.ui \
    sidinfo.ui
