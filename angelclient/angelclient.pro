#-------------------------------------------------
#
# Project created by QtCreator 2010-11-28T22:55:36
#
#-------------------------------------------------

QT       += core gui network xmlpatterns

TARGET = angelclient
TEMPLATE = app


SOURCES += main.cpp\
        angelclient.cpp

HEADERS  += angelclient.h

#embedd widgets dependency
include(../../../embedded-widgets-1.1.0/src/svgbutton/svgbutton.pri)
include(../../../embedded-widgets-1.1.0/src/common/common.pri)
RESOURCES += ../../../embedded-widgets-1.1.0/skins/beryl_svgbutton.qrc \
    resources.qrc
# embed widgets dependency

FORMS    += angelclient.ui
CONFIG += mobility
MOBILITY = 

symbian {
    LIBS += -lesock

    TARGET.UID3 = 0xea32c6e1
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
