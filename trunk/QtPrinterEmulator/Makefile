SYSTEM := $(shell uname)
ifeq "$(SYSTEM)" ""
SYSTEM := Output
endif
ifeq "$(SYSTEM)" "Linux"
LINUXSIZE := $(shell getconf LONG_BIT)
ifeq "$(LINUXSIZE)" "64"
SYSTEM := Linux64
endif
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

LINUX := $(findstring "$(SYSTEM)", "Linux" "Linux64")
	
OUTPUTDIR := $(SYSTEM)
DESTDIR := $(OUTPUTDIR)

ifeq "$(SYSTEM)" "windows32"
DISTDIR := $(SYSTEM)/dist/HP-82240B
DISTFILE := HP-82240B.zip
RESOURCESDIR := $(DISTDIR)
endif
ifneq "$(LINUX)" ""
DISTDIR := $(SYSTEM)/dist/HP-82240B
ifeq "$(SYSTEM)" "Linux64"
DISTFILE := HP-82240B-linux64.tgz
else
DISTFILE := HP-82240B-linux.tgz
endif
RESOURCESDIR := $(DISTDIR)
LINUX_EXTRA_FILES := icons/HP-82240B.png
endif
ifeq "$(SYSTEM)" "Darwin"
DESTDIR := $(OUTPUTDIR)/app
DISTDIR := $(DESTDIR)/HP-82240B.app
RESOURCESDIR := $(DISTDIR)/Contents/Resources
MACDEPLOYQT := macdeployqt
endif
# Warning the trailing / is mandatory
QTLIBSDIR := c:/QtSDK/Desktop/Qt/4.8.0/mingw/bin/
QTLIBS := $(addprefix $(QTLIBSDIR), qtcore4.dll qtgui4.dll qtxml4.dll mingwm10.dll libgcc_s_dw2-1.dll)

ifeq "$(SYSTEM)" "windows32"
HP82240B_EXE := $(DESTDIR)/HP-82240B.exe
endif
ifneq "$(LINUX)" ""
HP82240B_EXE := $(DESTDIR)/HP-82240B
endif



DIRS := $(OUTPUTDIR)
ifneq "$(OUTPUTDIR)" "$(DESTDIR)"
DIRS += $(DESTDIR)
endif


# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
QTMAKEFILE := QtMakefile.$(SYSTEM)
QTPROJECTFILE := HP-82240B.pro


DIRS_TO_CLEAN := $(DIRS)
EXTRA_FILES_TO_CLEAN :=
ifeq "$(SYSTEM)" "windows32"
DIRS_TO_CLEAN += debug release
EXTRA_FILES_TO_CLEAN += $(QTMAKEFILE).debug $(QTMAKEFILE).release 
endif


.PHONY: all clean

all: $(QTMAKEFILE) $(DIRS)
	$(MAKE) -f $(QTMAKEFILE) CC=$(CC) CXX=$(CXX) LINK=$(LINK)


$(QTMAKEFILE): $(QTPROJECTFILE)
	$(QMAKE) -makefile DESTDIR=$(DESTDIR) MOC_DIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE) $(QTPROJECTFILE)


$(DIRS):
	$(MKDIR) $@

dist: distclean release $(DISTDIR)
ifeq "$(SYSTEM)" "windows32"
	$(CP) $(HP82240B_EXE) $(DISTDIR)
	$(CP) $(QTLIBS) $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Darwin"
	$(MACDEPLOYQT) $(DISTDIR)
endif
ifneq "$(LINUX)" ""
	$(CP) $(HP82240B_EXE) $(DISTDIR)
	$(CP) $(LINUX_EXTRA_FILES) $(DISTDIR)
endif

$(DISTDIR):
	$(MKDIR) $(DISTDIR)

ifeq ($(SYSTEM),windows32)
release: $(QTMAKEFILE) $(DIRS)
	$(MAKE) -f $(QTMAKEFILE) release CC=$(CC) CXX=$(CXX) LINK=$(LINK)
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
