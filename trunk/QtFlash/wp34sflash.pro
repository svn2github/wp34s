TEMPLATE = app
TARGET = 
DEPENDPATH += .
LIBS +=  $(SERIALLIB)
INCLUDEPATH += $(SERIAL_INCLUDE) $(QTSERIAL_INCLUDE)

macx {
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi 
}

SOURCES = WP34sFlashMain.cpp WP34sFlash.cpp WP34sFlashGui.cpp WP34sFlashConsole.cpp WP34sFlashDialog.cpp
HEADERS = WP34sFlash.h WP34sFlashGui.h WP34sFlashConsole.h WP34sFlashDialog.h

win32 {
	RC_FILE = WP34sFlash.rc
}

macx {
ICON = icons/wp34s-flash-logo.icns
}