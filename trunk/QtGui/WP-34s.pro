TEMPLATE = app
TARGET = wp-34s
DEPENDPATH += .
QMAKE_PRE_LINK = $(MAKE) build_date 
LIBS += $(OTHEROBJS) $(BASELIBS) $(SERIALLIB)
PRE_TARGETDEPS += $(BASELIBS)
POST_TARGETDEPS += $(OUTPUTDIR)/QtEmulatorAdapter.o
INCLUDEPATH += $(SERIAL_INCLUDE) $(QTSERIAL_INCLUDE)
DEFINES+=$(HAS_SERIAL)

macx {
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi
}

QT += xml network widgets

SOURCES = QtGuiMain.cpp QtEmulator.cpp QtBackgroundImage.cpp QtKey.cpp QtKeyCode.cpp QtSkin.cpp QtScreenPainter.cpp \
	QtKeyboard.cpp QtSerialPort.cpp QtScreen.cpp QtCalculatorThread.cpp QtHeartBeatThread.cpp QtPreferencesDialog.cpp \
	QtNumberPaster.cpp QtDebugger.cpp QtRegistersModel.cpp QtIrPrint.cpp QtTextPainter.cpp QtNormalTextPainter.cpp \
	QtAccentedTextPainter.cpp QtSubscriptTextPainter.cpp QtSuperscriptTextPainter.cpp QtBoldTextPainter.cpp \
	QtPrinterSymbolTextPainter.cpp QtSpecialDigitPainter.cpp QtCatalogMenuItem.cpp QtCatalogMenu.cpp
	
HEADERS = QtEmulator.h QtBackgroundImage.h QtKey.h QtKeyCode.h QtSkin.h QtScreenPainter.h \
	QtKeyboard.h QtSerialPort.h QtScreen.h QtCalculatorThread.h QtHeartBeatThread.h QtPreferencesDialog.h \
	QtNumberPaster.h QtDebugger.h QtRegistersModel.h  QtTextPainter.h QtNormalTextPainter.h \
	QtAccentedTextPainter.h QtSubscriptTextPainter.h QtSuperscriptTextPainter.h QtBoldTextPainter.h \
	QtPrinterSymbolTextPainter.h QtSpecialDigitPainter.h QtCatalogMenuItem.h QtCatalogMenu.h

win32 {
	RC_FILE = WP-34s.rc
}

macx {
ICON = icons/wp34s-logo.icns
}
