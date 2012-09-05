TEMPLATE = app
TARGET = 
DEPENDPATH += .
QT += network

macx {
  LIBS += -framework IOKit -framework CoreFoundation -framework Foundation
}
win32 {
  LIBS += -lsetupapi 
}

SOURCES = PrinterEmulatorMain.cpp PrinterEmulator.cpp PaperWidget.cpp ScrollablePaper.cpp font82240b.cpp
HEADERS = PrinterEmulator.h PaperWidget.h ScrollablePaper.h font82240b.h

#win32 {
#	RC_FILE = HP82240B_Emulator.rc
#}

#macx {
#ICON = icons/hp82240b-logo.icns
#}