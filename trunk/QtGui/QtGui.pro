TEMPLATE = app
TARGET = 
DEPENDPATH += .
LIBS += -L$(OUTPUTDIR) -lAdapter $(BASELIBS) 

QT += xml

SOURCES = QtGuiMain.cpp QtEmulator.cpp QtBackgroundImage.cpp QtKey.cpp QtSkin.cpp QtScreenPainter.cpp \
	QtKeyboard.cpp QtSerialPort.cpp QtScreen.cpp QtCalculatorThread.cpp QtHeartBeatThread.cpp
	
HEADERS = QtEmulator.h QtBackgroundImage.h QtKey.h QtSkin.h QtScreenPainter.h \
	QtKeyboard.h QtSerialPort.h QtScreen.h QtCalculatorThread.h QtHeartBeatThread.h
