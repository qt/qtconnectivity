TEMPLATE = app
TARGET = sdpscanner

QT = core

SOURCES = main.cpp

CONFIG += link_pkgconfig
PKGCONFIG_PRIVATE += bluez

load(qt_tool)

linux-*: {
    # bluetooth.h is not standards compliant
    CONFIG -= strict_c++
}
