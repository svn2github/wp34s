TEMPLATE = app
CONFIG += console
TARGET = 
DEPENDPATH += .
LIBS +=  $(SERIALLIB)
INCLUDEPATH += $(SERIAL_INCLUDE) $(QTSERIAL_INCLUDE)
MOC_DIR = $(OBJECTS_DIR)

SOURCES = WP34sFlashCommand.cpp WP34sFlash.cpp WP34sFlashConsole.cpp WP34sTextConsole.cpp
HEADERS = WP34sFlash.h WP34sFlashConsole.h WP34sTextConsole.h

macx {
	CONFIG-=app_bundle
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi 
}
