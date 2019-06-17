#-------------------------------------------------
#
# Project created by QtCreator 2019-05-06T19:41:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql network

DEFINES += BOOST_USE_LIB

TARGET = stock
TEMPLATE = app

INCLUDEPATH += 3rdlibrary/hiredis/include
INCLUDEPATH += 3rdlibrary/boost/include

LIBS += -L"3rdlibrary/hiredis/lib" -lhiredis
LIBS += -L"3rdlibrary/boost/lib" -lboost_chrono-vc141-mt-x64-1_70 -lboost_system-vc141-mt-x64-1_70


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        categorytreemodel.cpp \
        datacenter.cpp \
        datafetch.cpp \
        main.cpp \
        maintablemodel.cpp \
        mainwindow.cpp \
        rediscachetools.cpp \
        stockchart.cpp \
        stockchartmodel.cpp \
        stockindexfetch.cpp \
        stockindexinfo.cpp \
        stockinfo.cpp

HEADERS += \
        category.h \
        categorytreemodel.h \
        commonenum.h \
        datacenter.h \
        datafetch.h \
        maintablemodel.h \
        mainwindow.h \
        rediscachetools.h \
        stockchart.h \
        stockchartmodel.h \
        stockindexfetch.h \
        stockindexinfo.h \
        stockinfo.h \
        threadutils.h

FORMS += \
        maincontent.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    stock.qrc
