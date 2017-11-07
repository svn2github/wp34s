TEMPLATE = lib
CONFIG += staticlib
TARGET = qtserial
DEPENDPATH += .
INCLUDEPATH += $(SERIAL_INCLUDE)

SOURCES = QtSerialPortHelper.cpp
HEADERS = QtSerialPortHelper.h
