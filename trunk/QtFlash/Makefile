SYSTEM := $(shell uname)
ifeq "$(SYSTEM)" ""
SYSTEM := Output
endif
ifeq "$(findstring MINGW,$(SYSTEM))" "MINGW"
# Force REALBUILD on windows under MinGW
SYSTEM := windows32
endif
ifeq "$(findstring CYGWIN,$(SYSTEM))" "CYGWIN"
SYSTEM := windows32
endif
ifeq "$(findstring indows,$(SYSTEM))" "indows"
SYSTEM := windows32
MAKE=mingw32-make
CC=mingw32-gcc
CXX=mingw32-g++
endif

MKDIR := mkdir -p
RM := rm -f
RMDIR := rm -rf
MV := mv -f
CP := cp
CPDIR := cp -r
ZIP := zip -r
TAR := tar czf

LINK=$(CXX)

OUTPUTDIR := $(SYSTEM)
DIRS := $(OUTPUTDIR)
ifeq "$(SYSTEM)" "windows32"
DISTDIR := wp34sFlash
DISTFILE := wp-34s-flash-windows.zip
RESOURCESDIR := $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Linux"
DISTDIR := wp34sFlash
DISTFILE := wp-34s-flash-linux.tgz
RESOURCESDIR := $(DISTDIR)
LINUX_EXTRA_FILES := icons/wp34s-flash-logo.png
endif
ifeq "$(SYSTEM)" "Darwin"
DISTDIR := $(OUTPUTDIR)/WP34sFlash.app
RESOURCESDIR := $(DISTDIR)/Contents/Resources
MACDEPLOYQT := macdeployqt
BUILTDMG := $(OUTPUTDIR)/WP34sFlash.dmg
DISTDMG := ./wp-34s-flash-macosx.dmg
endif
# Warning the trailing / is mandatory
QTLIBSDIR := c:/QtSDK/Desktop/Qt/4.8.0/mingw/bin/
QTLIBS := $(addprefix $(QTLIBSDIR), qtcore4.dll qtgui4.dll qtxml4.dll mingwm10.dll libgcc_s_dw2-1.dll)

ifeq "$(SYSTEM)" "windows32"
WP34SFLASHEXE := $(OUTPUTDIR)/WP34sFlash.exe
endif
ifeq "$(SYSTEM)" "Linux"
WP34SFLASHEXE := $(OUTPUTDIR)/WP34sFlash
endif


# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
QTMAKEFILE := QtMakefile.$(SYSTEM)
QTMAKEFILE_EXISTS := $(wildcard $(QTMAKEFILE))


DIRS_TO_CLEAN := $(DIRS) 
EXTRA_FILES_TO_CLEAN := moc_WP34sFlash.cpp moc_WP34sFlashGui.cpp moc_WP34sFlashDialog.cpp
ifeq "$(SYSTEM)" "windows32"
DIRS_TO_CLEAN += debug release
EXTRA_FILES_TO_CLEAN += $(QTMAKEFILE).debug $(QTMAKEFILE).release 
endif

QTSERIALDIR=../QtSerial
	HAS_SERIAL=1
ifeq ($(findstring "$(SYSTEM)", "Darwin" "Linux" "windows32"), "")
	SERIALLIB=
	QTSERIALLIB=
else
	SERIALLIB=-L$(QTSERIALDIR)/$(SYSTEM) -lQtSerial -L$(QTSERIALDIR)/lib/$(SYSTEM) -lqextserialport
	QTSERIALLIB=$(QTSERIALDIR)/$(SYSTEM)/libQtSerial.a
endif
ifeq ("$(SYSTEM)", "windows32")
	SERIAL_INCLUDE=$(QTSERIALDIR)/include/windows32
else
	SERIAL_INCLUDE=$(QTSERIALDIR)/include
endif
	QTSERIAL_INCLUDE=$(QTSERIALDIR)


.PHONY: all clean qtserial

all: $(QTMAKEFILE) $(DIRS)
	$(MAKE) -f $(QTMAKEFILE) SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)


$(QTMAKEFILE): wp34sflash.pro
	$(QMAKE) -makefile DESTDIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE)

qtserial:
	cd $(QTSERIALDIR); $(MAKE)
	
$(DIRS):
	$(MKDIR) $@

dist: distclean release $(DISTDIR) 
ifeq "$(SYSTEM)" "windows32"
	$(CP) $(WP34SFLASHEXE) $(DISTDIR)
	$(CP) $(QTLIBS) $(DISTDIR)
	$(ZIP) $(DISTFILE) $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Darwin"
	$(MACDEPLOYQT) $(DISTDIR) -dmg
	$(MV) $(BUILTDMG) $(DISTDMG)
endif
ifeq "$(SYSTEM)" "Linux"
	$(CP) $(WP34SFLASHEXE) $(DISTDIR)
	$(CP) $(LINUX_EXTRA_FILES) $(DISTDIR)
	$(TAR) $(DISTFILE) $(DISTDIR)
endif

$(DISTDIR):
	$(MKDIR) $(DISTDIR)

ifeq ($(SYSTEM),windows32)
release: $(QTMAKEFILE) $(DIRS) $(EXTRAOBJS)
	$(MAKE) -f $(QTMAKEFILE) release BASELIBS="$(BASELIBS)" OTHEROBJS="$(EXTRAOBJS) $(BASEOBJS)" HAS_SERIAL="HAS_SERIAL=$(HAS_SERIAL)" SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)
else
release: all
endif

clean:
	-$(RM) $(QTMAKEFILE) $(EXTRA_FILES_TO_CLEAN)
	-$(RMDIR) $(DIRS_TO_CLEAN)

distclean:
	-$(RMDIR) $(DISTDIR)
ifeq "$(SYSTEM)" "Darwin"
	-$(RM) $(BUILTDMG)
	-$(RM) $(DISTDMG)
endif	
