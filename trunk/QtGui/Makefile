# This file is part of 34S.
# 
# 34S is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# 34S is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with 34S.  If not, see <http://www.gnu.org/licenses/>.

# Settings for Unix like environments with gcc
# Creates the Console version of the emulator or the real thing
# To build the GUI version on Windows use Microsoft Visual C++ (Express)

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
# Force REALBUILD on windows under MinGW
ifeq "$(findstring indows,$(SYSTEM))" "indows"
# Force REALBUILD on windows under MinGW / alternate uname utility
SYSTEM := windows32
MAKE=mingw32-make
CC=mingw32-gcc
CXX=mingw32-g++
endif

LINK=$(CXX)

OUTPUTDIR := $(SYSTEM)
DIRS := $(OUTPUTDIR)

# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
MKDIR := mkdir
RM := rm -f
RMDIR := rm -rf

QTMAKEFILE := QtMakefile.$(SYSTEM)
QTMAKEFILE_EXISTS := $(wildcard $(QTMAKEFILE))

ADAPTERSRCS=QtEmulatorAdapter.c
ADAPTEROBJS=$(ADAPTERSRCS:%.c=$(OUTPUTDIR)/%.o)

BUILDDATESRC=QtBuildDate.c
BUILDDATEOBJ=$(BUILDDATESRC:%.c=$(OUTPUTDIR)/%.o)

DIRS_TO_CLEAN := $(DIRS)
EXTRA_FILES_TO_CLEAN := moc_QtBackgroundImage.cpp moc_QtEmulator.cpp moc_QtPreferencesDialog.cpp moc_QtKeyboard.cpp
ifeq "$(SYSTEM)" "windows32"
DIRS_TO_CLEAN += debug release
EXTRA_FILES_TO_CLEAN += $(QTMAKEFILE).debug $(QTMAKEFILE).release object_script.QtGui.debug object_script.QtGui.release
endif

ifeq "$(NO_SERIAL)" ""
	HAS_SERIAL=1
ifeq ($(findstring "$(SYSTEM)", "Darwin" "Linux" "windows32"), "")
	SERIAL_LIB=
else
	SERIAL_LIB=-Lserial/lib/$(SYSTEM) -lqextserialport
endif
ifeq ("$(SYSTEM)", "windows32")
	SERIAL_INCLUDE=serial/include/windows32
else
	SERIAL_INCLUDE=serial/include
endif
else
  SERIAL_LIB=
  SERIAL_INCLUDE=
  HAS_SERIAL=0
endif

$(OUTPUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -I.. -o $@ $<

.PHONY: all clean qtclean

ifeq "$(BASELIBS)" ""
ifdef RECURSIVE
all:
	@echo "Recursive calling, something is wrong in ../Makefile: BASELIBS not defined"
	
dist:
	@echo "Recursive calling, something is wrong in ../Makefile: BASELIBS not defined"
else
all:
	cd ..; $(MAKE) RECURSIVE=1 qt_gui CC=$(CC) CXX=$(CXX) LINK=$(LINK)
	
dist:
	cd ..; $(MAKE) RECURSIVE=1 qt_gui_dist CC=$(CC) CXX=$(CXX) LINK=$(LINK)
endif
else 
all: $(QTMAKEFILE) $(DIRS) $(ADAPTEROBJS)
	$(MAKE) -f $(QTMAKEFILE) BASE_LIBS="$(BASEOBJS) $(BASELIBS)" HAS_SERIAL="HAS_SERIAL=$(HAS_SERIAL)" SERIAL_LIB="$(SERIAL_LIB)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)
	
dist: release
endif

ifeq ($(SYSTEM),windows32)
release: $(QTMAKEFILE) $(DIRS) $(ADAPTEROBJS)
	$(MAKE) -f $(QTMAKEFILE) release BASE_LIBS="$(BASEOBJS) $(BASELIBS)" SERIAL_LIB="$(SERIAL_LIB)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)
else
release: all
endif

build_date: 
	$(CC) -c $(CFLAGS) -o $(BUILDDATEOBJ) $(BUILDDATESRC)

$(QTMAKEFILE): WP-34s.pro
	$(MAKE) qtclean
	$(QMAKE) -makefile DESTDIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE)

$(DIRS):
	$(MKDIR) -p -v $@

clean: qtclean
	-$(RM) $(QTMAKEFILE) $(EXTRA_FILES_TO_CLEAN)
	-$(RMDIR) $(DIRS_TO_CLEAN)


ifeq "$(QTMAKEFILE_EXISTS)" ""
qtclean:
else
qtclean:
	-$(MAKE) -f $(QTMAKEFILE) clean
endif
