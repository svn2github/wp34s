TEMPLATE = app
TARGET = 
DEPENDPATH += .
QMAKE_PRE_LINK = $(MAKE) build_date 
LIBS += $(OUTPUTDIR)/QtBuildDate.o $(OUTPUTDIR)/QtEmulatorAdapter.o $(BASELIBS) 
POST_TARGETDEPS += $(OUTPUTDIR)/QtEmulatorAdapter.o

QT += xml

SOURCES = QtGuiMain.cpp QtEmulator.cpp QtBackgroundImage.cpp QtKey.cpp QtSkin.cpp QtScreenPainter.cpp \
	QtKeyboard.cpp QtSerialPort.cpp QtScreen.cpp QtCalculatorThread.cpp QtHeartBeatThread.cpp QtPreferencesDialog.cpp
	
HEADERS = QtEmulator.h QtBackgroundImage.h QtKey.h QtSkin.h QtScreenPainter.h \
	QtKeyboard.h QtSerialPort.h QtScreen.h QtCalculatorThread.h QtHeartBeatThread.h QtPreferencesDialog.h
