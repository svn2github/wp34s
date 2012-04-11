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
OUTPUTDIRCMD := cmd/$(SYSTEM)
DESTDIR := $(OUTPUTDIR)
DESTDIRCMD := $(OUTPUTDIRCMD)

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
DESTDIR := $(OUTPUTDIR)/app
DISTDIR := $(DESTDIR)/WP34sFlash.app
RESOURCESDIR := $(DISTDIR)/Contents/Resources
MACDEPLOYQT := macdeployqt
DISTDMG := ./wp-34s-flash-macosx.dmg
BUILDDMG :=  hdiutil create -format UDBZ -quiet -volname WP34sFlash -srcfolder $(DESTDIR) $(DISTDMG)
endif
# Warning the trailing / is mandatory
QTLIBSDIR := c:/QtSDK/Desktop/Qt/4.8.0/mingw/bin/
QTLIBS := $(addprefix $(QTLIBSDIR), qtcore4.dll qtgui4.dll qtxml4.dll mingwm10.dll libgcc_s_dw2-1.dll)

ifeq "$(SYSTEM)" "windows32"
WP34SFLASHEXE := $(DESTDIR)/WP34sFlash.exe
WP34SFLASHCMDEXE := $(DESTDIRCMD)/wp34sflashcmd.exe
endif
ifeq "$(SYSTEM)" "Linux"
WP34SFLASHEXE := $(DESTDIR)/WP34sFlash
WP34SFLASHCMDEXE := $(DESTDIRCMD)/wp34sflashcmd
endif
ifeq "$(SYSTEM)" "Darwin"
WP34SFLASHCMDEXE := $(DESTDIRCMD)/wp34sflashcmd
endif


DIRS := $(OUTPUTDIR)
ifneq "$(OUTPUTDIR)" "$(DESTDIR)"
DIRS += $(DESTDIR)
endif
DIRS += $(OUTPUTDIRCMD)
ifneq "$(OUTPUTDIRCMD)" "$(DESTDIRCMD)"
DIRS += $(DESTDIRCMD)
endif


# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
QTMAKEFILE := QtMakefile.$(SYSTEM)
QTPROJECTFILE := wp34sflash.pro
QTMAKEFILECMD := QtMakefileCmd.$(SYSTEM)
QTPROJECTFILECMD := wp34sflashcmd.pro


DIRS_TO_CLEAN := $(DIRS)
EXTRA_FILES_TO_CLEAN :=
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

all: $(QTMAKEFILE) $(DIRS) qtserial
	$(MAKE) -f $(QTMAKEFILE) SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)

cmd: $(QTMAKEFILECMD) $(DIRS) qtserial
	$(MAKE) -f $(QTMAKEFILECMD) SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)


$(QTMAKEFILE): $(QTPROJECTFILE)
	$(QMAKE) -makefile DESTDIR=$(DESTDIR) MOC_DIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE) $(QTPROJECTFILE)

$(QTMAKEFILECMD): $(QTPROJECTFILECMD)
	$(QMAKE) -makefile DESTDIR=$(DESTDIRCMD) MOC_DIR=$(OUTPUTDIR)  OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILECMD) $(QTPROJECTFILECMD)


qtserial:
	cd $(QTSERIALDIR); $(MAKE)
	
$(DIRS):
	$(MKDIR) $@
	
dist: distclean release $(DISTDIR) cmd
ifeq "$(SYSTEM)" "windows32"
	$(CP) $(WP34SFLASHEXE) $(DISTDIR)
	$(CP) $(WP34SFLASHCMDEXE) $(DISTDIR)
	$(CP) $(QTLIBS) $(DISTDIR)
	$(ZIP) $(DISTFILE) $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Darwin"
	$(MACDEPLOYQT) $(DISTDIR)
	$(CP) $(WP34SFLASHCMDEXE) $(DESTDIR) 
	$(BUILDDMG)
endif
ifeq "$(SYSTEM)" "Linux"
	$(CP) $(WP34SFLASHEXE) $(DISTDIR)
	$(CP) $(WP34SFLASHCMDEXE) $(DISTDIR)
	$(CP) $(LINUX_EXTRA_FILES) $(DISTDIR)
	$(TAR) $(DISTFILE) $(DISTDIR)
endif

$(DISTDIR):
	$(MKDIR) $(DISTDIR)

ifeq ($(SYSTEM),windows32)
release: $(QTMAKEFILE) $(DIRS) qtserial
	$(MAKE) -f $(QTMAKEFILE) release SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)
else
release: all
endif

clean:
	-$(RM) $(QTMAKEFILE) $(QTMAKEFILECMD) $(EXTRA_FILES_TO_CLEAN)
	-$(RMDIR) $(DIRS_TO_CLEAN)

distclean:
	-$(RMDIR) $(DISTDIR)
ifeq "$(SYSTEM)" "Darwin"
	-$(RM) $(BUILTDMG)
	-$(RM) $(DISTDMG)
endif	
