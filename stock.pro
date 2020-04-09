#-------------------------------------------------
#
# Project created by QtCreator 2019-05-06T19:41:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql network

DEFINES += BOOST_USE_LIB

# 以下是Windows配置
INCLUDEPATH+= D:\ProgramTools\Boost\boost_1_72_0_b1_rc2
INCLUDEPATH+= ./3rdlibrary/hiredis/include
INCLUDEPATH+= ./3rdlibrary/zlib/include

LIBS += -L$$PWD/3rdlibrary/zlib/lib -lzlib
LIBS += -L$$PWD/3rdlibrary/hiredis/lib -lhiredis
LIBS += -L$$PWD/3rdlibrary/boost/lib -lboost_thread-vc142-mt-gd-x64-1_72 -lboost_system-vc142-mt-gd-x64-1_72 -lboost_chrono-vc142-mt-gd-x64-1_72 -lboost_date_time-vc142-mt-gd-x64-1_72
# Windows配置 -- End

TARGET = stock
TEMPLATE = app

# Linux 配置
# LIBS += -lhiredis -lz -lboost_thread -lboost_system -lboost_chrono -lboost_date_time
# Linux配置End


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
        addconcerndialog.cpp \
        calculator.cpp \
        data/anaresult.cpp \
        data/commonanaresult.cpp \
        data/dailyconcern.cpp \
        data/lastmaxupindexrst.cpp \
        delconcerndialog.cpp \
        holdingchangedialog.cpp \
        indexanalyzer.cpp \
        infodisplay.cpp \
        ui/categorytreemodel.cpp \
        data/datacenter.cpp \
        data/datafetch.cpp \
        main.cpp \
        ui/maintable.cpp \
        ui/maintablemodel.cpp \
        ui/mainwindow.cpp \
        utils/filemanager.cpp \
        utils/rediscachetools.cpp \
        ui/stockchart.cpp \
        ui/stockchartmodel.cpp \
        data/stockindexfetch.cpp \
        data/stockindexinfo.cpp \
        data/stockinfo.cpp \
        utils/zlibcompress.cpp

HEADERS += \
        addconcerndialog.h \
        calculator.h \
        category.h \
        data/anaresult.h \
        data/commonanaresult.h \
        data/dailyconcern.h \
        data/lastmaxupindexrst.h \
        data/stockbaseinfo.h \
        delconcerndialog.h \
        holdingchangedialog.h \
        indexanalyzer.h \
        infodisplay.h \
        ui/categorytreemodel.h \
        commonenum.h \
        data/datacenter.h \
        data/datafetch.h \
        excpetions.h \
        ui/maintable.h \
        ui/maintablemodel.h \
        ui/mainwindow.h \
        utils/filemanager.h \
        utils/rediscachetools.h \
        ui/stockchart.h \
        ui/stockchartmodel.h \
        data/stockindexfetch.h \
        data/stockindexinfo.h \
        data/stockinfo.h \
        threadutils.h \
        utils/zlibcompress.h

FORMS += \
        addconcerndialog.ui \
        delconcerndialog.ui \
        holdingchangedialog.ui \
        infodisplay.ui \
        maincontent.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    stock.qrc
