TEMPLATE = app
TARGET = 
DEPENDPATH += .
QMAKE_PRE_LINK = $(MAKE) build_date 
LIBS += $(BASE_LIBS) $(SERIAL_LIB)
POST_TARGETDEPS += $(OUTPUTDIR)/QtEmulatorAdapter.o
INCLUDEPATH += $(SERIAL_INCLUDE)
DEFINES+=$(HAS_SERIAL)

macx {
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi
}

QT += xml

SOURCES = QtGuiMain.cpp QtEmulator.cpp QtBackgroundImage.cpp QtKey.cpp QtKeyCode.cpp QtSkin.cpp QtScreenPainter.cpp \
	QtKeyboard.cpp QtSerialPort.cpp QtScreen.cpp QtCalculatorThread.cpp QtHeartBeatThread.cpp QtPreferencesDialog.cpp \
	QtNumberPaster.cpp
	
HEADERS = QtEmulator.h QtBackgroundImage.h QtKey.h QtKeyCode.h QtSkin.h QtScreenPainter.h \
	QtKeyboard.h QtSerialPort.h QtScreen.h QtCalculatorThread.h QtHeartBeatThread.h QtPreferencesDialog.h \
	QtNumberPaster.h

win32 {
	RC_FILE = WP-34s.rc
}

macx {
ICON = icons/wp34s-logo.icns
}