#-------------------------------------------------
#
# Project created by QtCreator 2022-04-11T14:08:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DBMS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        dialog.cpp \
    dbentity.cpp \
    tableentity.cpp \
    fieldentity.cpp \
    recordentity.cpp \
    dbdao.cpp \
    tabledao.cpp \
    fielddao.cpp \
    recorddao.cpp \
    dbservice.cpp \
    dbcreatediqlog.cpp \
    fileutil.cpp \
    tableservice.cpp \
    fieldservice.cpp \
    recordservice.cpp \
    tbcreatedialog.cpp \
    fielddialog.cpp \
    tbmodifynamedialog.cpp \
    recordcreatedialog.cpp \
    selectdialog.cpp \
    logindialog.cpp \
    sqlanalyze.cpp \
    export.cpp

HEADERS += \
        dialog.h \
    dbentity.h \
    tableentity.h \
    fieldentity.h \
    recordentity.h \
    dbdao.h \
    tabledao.h \
    fielddao.h \
    recorddao.h \
    global.h \
    dbservice.h \
    dbcreatediqlog.h \
    fileutil.h \
    tableservice.h \
    fieldservice.h \
    recordservice.h \
    tbcreatedialog.h \
    fielddialog.h \
    tbmodifynamedialog.h \
    recordcreatedialog.h \
    selectdialog.h \
    logindialog.h \
    sqlanalyze.h \
    export.h

FORMS += \
        dialog.ui \
    dbcreatediqlog.ui \
    tbcreatedialog.ui \
    fielddialog.ui \
    tbmodifynamedialog.ui \
    recordcreatedialog.ui \
    selectdialog.ui \
    logindialog.ui

RESOURCES += \
    qq.qrc
