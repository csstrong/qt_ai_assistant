QT += core gui widgets network sql

TARGET = TestControl
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

CONFIG += c++11

# 包含本地MQTT模块
INCLUDEPATH += $$PWD/qtmqtt/build/Desktop_Qt_6_6_3_MinGW_64_bit-Debug/include
LIBS += -L$$PWD/qtmqtt/build/Desktop_Qt_6_6_3_MinGW_64_bit-Debug/lib -lQt6Mqtt

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
default: target.path = $$PWD/bin
INSTALLS += target
