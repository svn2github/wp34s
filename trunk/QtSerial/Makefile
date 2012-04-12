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

RM := rm -f
RMDIR := rm -rf

LINK=$(CXX)

OUTPUTDIR := $(SYSTEM)
DIRS := $(OUTPUTDIR)

# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
QTMAKEFILE := QtMakefile.$(SYSTEM)
QTMAKEFILE_EXISTS := $(wildcard $(QTMAKEFILE))


DIRS_TO_CLEAN := $(DIRS) 
EXTRA_FILES_TO_CLEAN :=
ifeq "$(SYSTEM)" "windows32"
DIRS_TO_CLEAN += debug release
EXTRA_FILES_TO_CLEAN += $(QTMAKEFILE).debug $(QTMAKEFILE).release 
endif

SERIAL_INCLUDE=include


.PHONY: all clean

all: $(QTMAKEFILE) $(DIRS)
	$(MAKE) -f $(QTMAKEFILE) SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)


$(QTMAKEFILE): QtSerial.pro
	$(QMAKE) -makefile DESTDIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE)

$(DIRS):
	$(MKDIR) $@

clean:
	-$(RM) $(QTMAKEFILE) $(EXTRA_FILES_TO_CLEAN)
	-$(RMDIR) $(DIRS_TO_CLEAN)


