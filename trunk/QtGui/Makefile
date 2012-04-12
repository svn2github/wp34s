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


TOOLS=../tools

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
PPDIR := -pp_dir $(TOOLS)
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
ifeq "$(SYSTEM)" "windows32"
DISTDIR := wp-34s
DISTFILE := wp-34s-emulator-windows.zip
RESOURCESDIR := $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Linux"
DISTDIR := wp-34s
DISTFILE := wp-34s-emulator-linux.tgz
RESOURCESDIR := $(DISTDIR)
LINUX_EXTRA_FILES := README_Linux.txt icons/wp34s-logo.png
endif
ifeq "$(SYSTEM)" "Darwin"
DISTDIR := $(OUTPUTDIR)/WP-34s.app
RESOURCESDIR := $(DISTDIR)/Contents/Resources
MACDEPLOYQT := macdeployqt
BUILTDMG := $(OUTPUTDIR)/WP-34s.dmg
DISTDMG := ./wp-34s-emulator-macosx.dmg
endif
DISTMEMORYDIR := $(RESOURCESDIR)/memory
DISTMEMORYFILE := $(DISTMEMORYDIR)/wp34s-lib.dat
# Warning the trailing / is mandatory
QTLIBSDIR := c:/QtSDK/Desktop/Qt/4.8.0/mingw/bin/
QTLIBS := $(addprefix $(QTLIBSDIR), qtcore4.dll qtgui4.dll qtxml4.dll mingwm10.dll libgcc_s_dw2-1.dll)

# Warning the trailing / is mandatory
WP34SLIBDIR=../../library/
WP34SMATRIXFILES :=  $(addprefix $(WP34SLIBDIR), matrix.wp34s matrixedit.wp34s vectors.wp34s)
WP34SSOLVERFILES :=  $(addprefix $(WP34SLIBDIR), TVM.wp34s)
WP34STRIGOFACTOFILES :=  $(addprefix $(WP34SLIBDIR), TRIGON.wp34s PF.wp34s)
WP34SLIB=$(TOOLS)/wp34s_lib.pl -d 0


DOCFILENAME := Manual_wp_34s_3_0.pdf
DOCDIR := ../../doc
DOCFILE := $(DOCDIR)/$(DOCFILENAME)
DISTDOCDIR := $(RESOURCESDIR)/doc
DISTDOCDIR := $(RESOURCESDIR)/doc
DISTDOCFILE := $(DISTDOCDIR)/$(DOCFILENAME)

# we need the MAKE variable to be defined
# if not, we try the default value
ifeq "$(MAKE)" ""
MAKE := make
endif

QMAKE := qmake
MKDIR := mkdir -p
RM := rm -f
RMDIR := rm -rf
MV := mv -f
CP := cp
CPDIR := cp -r
ZIP := zip -r
TAR := tar czf
QTMAKEFILE := QtMakefile.$(SYSTEM)
QTMAKEFILE_EXISTS := $(wildcard $(QTMAKEFILE))

ADAPTERSRCS=QtEmulatorAdapter.c 
ADAPTEROBJS=$(ADAPTERSRCS:%.c=$(OUTPUTDIR)/%.o)

BUILDDATESRC=QtBuildDate.c
BUILDDATEOBJ=$(BUILDDATESRC:%.c=$(OUTPUTDIR)/%.o)

EXTRAOBJS=$(ADAPTEROBJS) $(BUILDDATEOBJ)

DIRS_TO_CLEAN := $(DIRS) $(DISTDIR)
EXTRA_FILES_TO_CLEAN := $(DISTFILE)
ifeq "$(SYSTEM)" "windows32"
DIRS_TO_CLEAN += debug release
EXTRA_FILES_TO_CLEAN += $(QTMAKEFILE).debug $(QTMAKEFILE).release object_script.WP-34s.debug object_script.WP-34s.release
endif

ifeq "$(NO_SERIAL)" ""
	QTSERIALDIR=../QtSerial
	HAS_SERIAL=1
ifeq ($(findstring "$(SYSTEM)", "Darwin" "Linux" "windows32"), "")
	SERIALLIB=
	QTSERIALLIB=
else
	SERIALLIB=-L$(QTSERIALDIR)/$(SYSTEM) -lQtSerial -L$(QTSERIALDIR)/lib/$(SYSTEM) -lqextserialport
	QTSERIALLIB=$(QTSERIALDIR)/$(SYSTEM)/libQtSerial.a
endif
	SERIAL_INCLUDE=$(QTSERIALDIR)/include
	QTSERIAL_INCLUDE=$(QTSERIALDIR)
else
  SERIALLIB=
  QTSERIALLIB=
  SERIAL_INCLUDE=
  QTSERIAL_INCLUDE=
  HAS_SERIAL=0
endif

$(OUTPUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -I.. -o $@ $<

.PHONY: all clean qtclean qtserial

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
all: qtserial $(QTMAKEFILE) $(DIRS) $(EXTRAOBJS)
	$(MAKE) -f $(QTMAKEFILE) BASELIBS="$(BASELIBS)" OTHEROBJS="$(EXTRAOBJS) $(BASEOBJS)" HAS_SERIAL="HAS_SERIAL=$(HAS_SERIAL)" SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)

ifeq "$(SYSTEM)" "windows32"
WP34SEXE := $(OUTPUTDIR)/WP-34s.exe
endif
ifeq "$(SYSTEM)" "Linux"
WP34SEXE := $(OUTPUTDIR)/WP-34s
endif

ifeq "$(HAS_SERIAL)" "0"
qtserial:
else
qtserial:
	cd $(QTSERIALDIR); $(MAKE)
endif

dist: distclean release $(DISTDIR) $(DISTMEMORYDIR) $(DISTMEMORYFILE) $(DISTDOCDIR) $(DISTDOCFILE)
ifeq "$(SYSTEM)" "windows32"
	$(CP) $(WP34SEXE) $(DISTDIR)
	$(MKDIR) $(DISTDIR)/skins
	$(CPDIR) skins/* $(DISTDIR)/skins
	$(MKDIR) $(DISTDIR)/images
	$(CPDIR) images/* $(DISTDIR)/images
	$(CP) $(QTLIBS) $(DISTDIR)
	$(ZIP) $(DISTFILE) $(DISTDIR)
endif
ifeq "$(SYSTEM)" "Darwin"
	$(MKDIR) $(RESOURCESDIR)/skins
	$(CPDIR) skins/* $(RESOURCESDIR)/skins
	$(MKDIR) $(RESOURCESDIR)/images
	$(CPDIR) images/* $(RESOURCESDIR)/images
	$(MACDEPLOYQT) $(DISTDIR) -dmg
	$(MV) $(BUILTDMG) $(DISTDMG)
endif
ifeq "$(SYSTEM)" "Linux"
	$(CP) $(WP34SEXE) $(DISTDIR)
	$(MKDIR) $(DISTDIR)/skins
	$(CPDIR) skins/* $(DISTDIR)/skins
	$(MKDIR) $(DISTDIR)/images
	$(CPDIR) images/* $(DISTDIR)/images
	$(CP) $(LINUX_EXTRA_FILES) $(DISTDIR)
	$(TAR) $(DISTFILE) $(DISTDIR)
endif

$(DISTDIR):
	$(MKDIR) $(DISTDIR)

$(DISTMEMORYDIR): 
	$(MKDIR) $(DISTMEMORYDIR)

$(DISTMEMORYFILE): $(WP34SMATRIXFILES) $(WP34SSOLVERFILES) $(WP34STRIGOFACTOFILES)
	$(WP34SLIB) -pp $(PPDIR) $(WP34SMATRIXFILES) -olib $(DISTMEMORYFILE)
	$(WP34SLIB) $(WP34SSOLVERFILES) -ilib $(DISTMEMORYFILE)	-olib $(DISTMEMORYFILE)
	$(WP34SLIB) -pp $(PPDIR) $(WP34STRIGOFACTOFILES) -ilib $(DISTMEMORYFILE)	-olib $(DISTMEMORYFILE)

$(DISTDOCDIR): 
	$(MKDIR) $(DISTDOCDIR)

$(DISTDOCFILE): $(DOCFILE) 
	$(CP) $(DOCFILE) $(DISTDOCFILE)

endif

ifeq ($(SYSTEM),windows32)
release: $(QTMAKEFILE) $(DIRS) $(EXTRAOBJS)
	$(MAKE) -f $(QTMAKEFILE) release BASELIBS="$(BASELIBS)" OTHEROBJS="$(EXTRAOBJS) $(BASEOBJS)" HAS_SERIAL="HAS_SERIAL=$(HAS_SERIAL)" SERIALLIB="$(SERIALLIB)" QTSERIAL_INCLUDE="$(QTSERIAL_INCLUDE)" SERIAL_INCLUDE="$(SERIAL_INCLUDE)" CC=$(CC) CXX=$(CXX) LINK=$(LINK)
else
release: all
endif

build_date: 
	$(CC) -c $(CFLAGS) -o $(BUILDDATEOBJ) $(BUILDDATESRC)

$(QTMAKEFILE): WP-34s.pro
	$(MAKE) qtclean
	$(QMAKE) -makefile DESTDIR=$(OUTPUTDIR) MOC_DIR=$(OUTPUTDIR) OBJECTS_DIR=$(OUTPUTDIR) -o $(QTMAKEFILE)

$(DIRS):
	$(MKDIR) $@

clean: qtclean
	-$(RM) $(QTMAKEFILE) $(EXTRA_FILES_TO_CLEAN)
	-$(RMDIR) $(DIRS_TO_CLEAN)


ifeq "$(QTMAKEFILE_EXISTS)" ""
qtclean:
else
qtclean:
	-$(MAKE) -f $(QTMAKEFILE) clean
endif

distclean:
	-$(RMDIR) $(DISTDIR)
ifeq "$(SYSTEM)" "Darwin"
	-$(RM) $(BUILTDMG)
	-$(RM) $(DISTDMG)
endif	