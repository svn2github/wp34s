TEMPLATE = app
TARGET = HP-82240B
DEPENDPATH += .
QT += network

macx {
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi 
}

SOURCES = PrinterEmulatorMain.cpp PrinterEmulator.cpp PrintDataReader.cpp PaperWidget.cpp ScrollablePaper.cpp font82240b.cpp
HEADERS = PrinterEmulator.h PrintDataReader.h PaperWidget.h ScrollablePaper.h font82240b.h

win32 {
	RC_FILE = HP-82240B.rc
}

macx {
ICON = icons/HP-82240B.icns
}