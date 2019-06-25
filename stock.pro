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
INCLUDEPATH += 3rdlibrary/cpp_redis/include
INCLUDEPATH += 3rdlibrary/tacopie/include
INCLUDEPATH += 3rdlibrary/zlib/include

LIBS += -L"3rdlibrary/hiredis/lib" -lhiredis
LIBS += -L"3rdlibrary/boost/lib" -lboost_chrono-vc141-mt-x64-1_70 -lboost_system-vc141-mt-x64-1_70
LIBS += -L"3rdlibrary/cpp_redis/lib" -lcpp_redis
LIBS += -L"3rdlibrary/tacopie/lib" -ltacopie
LIBS += -L"3rdlibrary/zlib/lib" -lzlib


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
        ui/categorytreemodel.cpp \
        data/datacenter.cpp \
        data/datafetch.cpp \
        main.cpp \
        ui/maintablemodel.cpp \
        ui/mainwindow.cpp \
        utils/rediscachetools.cpp \
        ui/stockchart.cpp \
        ui/stockchartmodel.cpp \
        data/stockindexfetch.cpp \
        data/stockindexinfo.cpp \
        data/stockinfo.cpp \
        utils/zlibcompress.cpp

HEADERS += \
        calculator.h \
        category.h \
        ui/categorytreemodel.h \
        commonenum.h \
        data/datacenter.h \
        data/datafetch.h \
        excpetions.h \
        ui/maintablemodel.h \
        ui/mainwindow.h \
        utils/rediscachetools.h \
        ui/stockchart.h \
        ui/stockchartmodel.h \
        data/stockindexfetch.h \
        data/stockindexinfo.h \
        data/stockinfo.h \
        threadutils.h \
        utils/zlibcompress.h

FORMS += \
        maincontent.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    stock.qrc
