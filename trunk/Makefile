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

.EXPORT_ALL_VARIABLES:

# Define to build the real thing
#REALBUILD = 1

# Define to disable the watchdog
#NOWD = 1

# Disable support for the crystal oscillator (overrides XTAL)
#DISABLE_XTAL = 1

# Define to use a crystal oscillator on slow clock after hardware modification
#XTAL = 1

# Define to compile code for an IR transmitter on TIOA0
ifdef QTGUI
INFRARED = 1
else
#INFRARED = 1
endif

BASE_CFLAGS := -Wall -Werror -g -fno-common -fno-exceptions 
OPT_CFLAGS := -Os -fira-region=one
USE_CURSES := -DUSECURSES

# Settings for Unix like environments with gcc
# Creates the Console version of the emulator or the real thing
# To build the GUI version on Windows use Microsoft Visual C++ (Express)

ifeq "$(findstring Ios,$(SYSTEM))" "Ios"
ifeq "$(SYSTEM)" "Ios"
    ARCH=armv7
    DEVICE=OS
    CC_FLAGS=-arch $(ARCH) -I$(IOS_DEVROOT)/SDKs/iPhone$(DEVICE).sdk/usr/include 
    BASE_LDFLAGS=-L$(IOS_DEVROOT)/SDKs/iPhone$(DEVICE).sdk/usr/lib
    CFLAGS_FLAGS=-mcpu=cortex-a8 -marm
endif
ifeq "$(SYSTEM)" "Ios64"
    ARCH=arm64
    DEVICE=OS
    CC_FLAGS=-arch $(ARCH) -I$(IOS_DEVROOT)/SDKs/iPhone$(DEVICE).sdk/usr/include -DFIX_64_BITS 
    BASE_LDFLAGS=-L$(IOS_DEVROOT)/SDKs/iPhone$(DEVICE).sdk/usr/lib
endif
ifeq "$(SYSTEM)" "IosSimulator"
    ARCH=i386
    CC_FLAGS=-arch $(ARCH)
    DEVICE=Simulator
endif
ifeq "$(SYSTEM)" "IosSimulator64"
    ARCH=x86_64
    CC_FLAGS=-arch $(ARCH)
    DEVICE=Simulator
endif
# Warning: calling these variables DEVROOT & SDKROOT breaks compilation with some OSX gcc version as HOSTCC starts to behave like CC
IOS_XCODE_ROOT := /Applications/Xcode.app/Contents/Developer
IOS_DEVROOT := $(IOS_XCODE_ROOT)/Platforms/iPhone$(DEVICE).platform/Developer
IOS_SDKROOT := $(IOS_DEVROOT)/SDKs/iPhone$(DEVICE).sdk
CC=$(IOS_XCODE_ROOT)/usr/bin/gcc $(CC_FLAGS)
BASE_CFLAGS += -isysroot ${IOS_SDKROOT} -Iheaders $(CFLAGS_FLAGS) -DIOS -miphoneos-version-min=7.0
USE_CURSES :=
else
SYSTEM := $(shell uname)
endif

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
SYSTEM := windows32
MAKE=mingw32-make
CC=mingw32-gcc
CXX=mingw32-g++
endif
ifeq "$(findstring CYGWIN,$(SYSTEM))" "CYGWIN"
SYSTEM := windows32
ifdef QTGUI
MAKE=mingw32-make
CC=mingw32-gcc
CXX=mingw32-g++
endif
endif
ifeq "$(findstring indows,$(SYSTEM))" "indows"
# Any other Windows GCC is mapped to windows32
SYSTEM := windows32
endif

ifeq ($(SYSTEM),windows32)
EXE := .exe
else
EXE :=
endif

CFLAGS = $(BASE_CFLAGS)
ifdef QTGUI
CFLAGS += -O0 -DDEBUG -DQTGUI
ifdef INFRARED
CFLAGS += -DINFRARED
HOSTCFLAGS += -DINFRARED
endif
ifeq "$(SYSTEM)" "Darwin"
CFLAGS += -DFIX_64_BITS -mmacosx-version-min=10.5
endif 
else
CFLAGS += -O0 -DDEBUG $(USE_CURSES)
endif

ifeq "$(SYSTEM)" "Linux64"
CFLAGS += -DFIX_64_BITS -DFIX_LINUX_64_BITS
endif 

ifndef REALBUILD
ifndef QTGUI
# Select the correct parameters and libs for various Unix flavours
ifeq "$(findstring Linux,$(SYSTEM))" "Linux"
LIBS += -lcurses
else
ifeq ($(SYSTEM),Darwin)
# MacOS - use static ncurses lib if found
CFLAGS += -m32
NCURSES := $(shell find /sw/lib -name libncurses.a)
ifneq "$(NCURSES)" ""
LIBS += $(NCURSES)
else
LIBS += -lcurses
endif
else
ifeq ($(SYSTEM),windows32)
LIBS += -lpdcurses
else
# Any other Unix
LIBS += -lcurses
endif
endif
endif
endif
endif

TOOLS := tools
AR=ar
RANLIB=ranlib
LDFLAGS := $(BASE_LDFLAGS)
LDCTRL :=

# Matches both SYSTEM=iOS and SYSTEM=iOS64
ifeq (Ios, $(subst 64,,$(SYSTEM)))
HOSTCC := gcc
HOSTAR := ar
HOSTRANLIB := ranlib
HOSTCFLAGS := -Wall -Werror -O1 -g -DHOSTBUILD 
else
HOSTCC := $(CC)
HOSTAR := $(AR)
HOSTRANLIB := $(RANLIB)
HOSTCFLAGS := -Wall -O1 -g -DHOSTBUILD 
endif

ifdef REALBUILD
# Settings for the Yagarto tool chain under Windows (or MacOS)
# On Windows a standard gcc is needed for building the generated files.
# MinGW will do nicely

CFLAGS := -mthumb -mcpu=arm7tdmi $(OPT_CFLAGS) $(BASE_CFLAGS)
CFLAGS += -DREALBUILD -Dat91sam7l128 -Iatmel

# ifeq ($(SYSTEM),Darwin)
# MacOS - uses 32 bits pointer or code won't compile
HOSTCFLAGS += -m32
# endif
HOSTCFLAGS += -DREALBUILD
ifdef NOWD
CFLAGS += -DNOWD
endif
ifdef DISABLE_XTAL
CFLAGS += -DDISABLE_XTAL
HOSTCFLAGS += -DDISABLE_XTAL
else
ifdef XTAL
CFLAGS += -DXTAL
HOSTCFLAGS += -DXTAL
ifdef INFRARED
CFLAGS += -DINFRARED
HOSTCFLAGS += -DINFRARED
endif
endif
endif
CFLAGS += -DNO_BACKUP_INIT -DNO_RAM_COPY
LDFLAGS := -nostartfiles 
CROSS_COMPILE := arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RANLIB := $(CROSS_COMPILE)ranlib
# SIZE := $(CROSS_COMPILE)size
# STRIP := $(CROSS_COMPILE)strip
OBJCOPY := $(CROSS_COMPILE)objcopy
NM := $(CROSS_COMPILE)nm
LIBS += -nostdlib -lgcc
EXE := .exe
endif

ifdef REALBUILD
OUTPUTDIR := realbuild
UTILITIES := $(SYSTEM)_realbuild
OBJECTDIR := $(UTILITIES)/obj
DIRS := $(OBJECTDIR) $(UTILITIES)
DNOPTS :=
else
ifdef QTGUI
OUTPUTDIR := $(SYSTEM)_qt
UTILITIES := $(SYSTEM)_qt
else
OUTPUTDIR := $(SYSTEM)
UTILITIES := $(SYSTEM)
endif
OBJECTDIR := $(OUTPUTDIR)/obj
DIRS := $(OBJECTDIR) $(OUTPUTDIR)
DNOPTS := -DNEED_D128TOSTRING
endif

# Files and libraries

SRCS := keys.c display.c xeq.c prt.c decn.c complex.c stats.c \
		lcd.c int.c date.c consts.c alpha.c charmap.c \
		commands.c string.c storage.c serial.c matrix.c \
		stopwatch.c printer.c font.c data.c
ifeq ($(SYSTEM),windows32)
SRCS += winserial.c
endif

HEADERS := alpha.h charset7.h complex.h consts.h data.h \
		date.h decn.h display.h features.h int.h keys.h lcd.h lcdmap.h \
		stats.h xeq.h xrom.h storage.h serial.h matrix.h \
		stopwatch.h printer.h

XROM := $(wildcard xrom/*.wp34s) $(wildcard xrom/distributions/*.wp34s)

OBJS := $(SRCS:%.c=$(OBJECTDIR)/%.o)

LIBS += -L$(OBJECTDIR) -lconsts
LIBDN := -ldecNum34s
CNSTS := $(OBJECTDIR)/libconsts.a

DNSRCS := decNumber.c decContext.c decimal64.c decimal128.c
DNOBJS := $(DNSRCS:%.c=$(OBJECTDIR)/%.o)
DNSRCS := $(DNSRCS:%.c=decNumber/%.c)
DNHDRS := $(DNSRCS:%.c=%.h) 

ifdef REALBUILD
ifdef DISABLE_XTAL
TARGET=calc_noxtal
MAPFILE := $(OUTPUTDIR)/mapfile_noxtal.txt
SUMMARY := $(OUTPUTDIR)/summary_noxtal.txt
SYMBOLS := $(OUTPUTDIR)/symbols_noxtal.txt
else
ifdef XTAL
ifdef INFRARED
TARGET=calc_ir
MAPFILE := $(OUTPUTDIR)/mapfile_ir.txt
SUMMARY := $(OUTPUTDIR)/summary_ir.txt
SYMBOLS := $(OUTPUTDIR)/symbols_ir.txt
else
TARGET=calc_xtal
MAPFILE := $(OUTPUTDIR)/mapfile_xtal.txt
SUMMARY := $(OUTPUTDIR)/summary_xtal.txt
SYMBOLS := $(OUTPUTDIR)/symbols_xtal.txt
endif
else
TARGET=calc
MAPFILE := $(OUTPUTDIR)/mapfile.txt
SUMMARY := $(OUTPUTDIR)/summary.txt
SYMBOLS := $(OUTPUTDIR)/symbols.txt
endif
endif
STARTUP := atmel/board_cstartup.S
ATSRCS := board_lowlevel.c board_memories.c aic.c pmc.c rtc.c slcdc.c supc.c 
ATOBJS := $(ATSRCS:%.c=$(OBJECTDIR)/%.o)
ATSRCS := $(ATSRCS:%.c=atmel/%.c)
ATHDRS := $(ATSRCS:%.c=%.h) atmel/board.h atmel/at91sam7l128/AT91SAM7L128.h 

LDCTRL := wp34s.lds
LDFLAGS += -T $(LDCTRL) -Wl,--gc-sections,-Map=$(MAPFILE)
MAIN := $(OBJECTDIR)/main.o
else
MAIN := $(OBJECTDIR)/console.o
endif
OPCODES := $(TOOLS)/wp34s.op

# Targets and rules

.PHONY: clean tgz flash version qt_gui real_qt_gui qt_clean qt_clean_all

ifdef REALBUILD
all: flash
flash: $(DIRS) $(OUTPUTDIR)/$(TARGET).bin
else
ifdef QTGUI
all: qtobjs
qtobjs: $(DIRS) $(OBJS) $(OBJECTDIR)/libdecNum34s.a $(CNSTS) $(LDCTRL) Makefile
else
all: calc
calc: $(DIRS) $(OUTPUTDIR)/calc
endif
endif

version:
	@$(CC) --version

clean:
	-rm -fr $(DIRS)
	-rm -fr consts.h consts.c allconsts.c catalogues.h xrom.c
	-rm -f xrom_pre.wp34s user_consts.h wp34s_pp.lst xrom_labels.h
#       -$(MAKE) -C decNumber clean
#       -$(MAKE) -C utilities clean

tgz:
	@$(MAKE) clean
	rm -f sci.tgz
	tar czf sci.tgz *

$(DIRS):
	mkdir -p -v $@

ifdef REALBUILD

# Target flash
$(OUTPUTDIR)/$(TARGET).bin: asone.c main.c $(HEADERS) $(SRCS) $(STARTUP) $(ATSRCS) $(ATHDRS) \
		$(DNHDRS) $(OBJECTDIR)/libconsts.a $(OBJECTDIR)/libdecNum34s.a \
		$(LDCTRL) Makefile $(UTILITIES)/post_process$(EXE) $(UTILITIES)/create_revision$(EXE) \
		compile_cats.c xrom.wp34s $(XROM) # $(OPCODES)
	rm -f $(UTILITIES)/compile_cats$(EXE) catalogues.h xrom.c
	$(MAKE) HOSTCC=$(HOSTCC) REALBUILD=1 DISABLE_XTAL=$(DISABLE_XTAL) XTAL=$(XTAL) INFRARED=$(INFRARED) catalogues.h xrom.c
	$(UTILITIES)/create_revision$(EXE) >revision.h
	$(CC) $(CFLAGS) -IdecNumber -o $(OUTPUTDIR)/$(TARGET) $(LDFLAGS) \
		$(STARTUP) asone.c $(LIBS) -fwhole-program -ldecNum34s # -save-temps
	$(NM) -n $(OUTPUTDIR)/$(TARGET) >$(SYMBOLS)
	$(NM) -S $(OUTPUTDIR)/$(TARGET) >>$(SYMBOLS)
	$(OBJCOPY) -O binary --gap-fill 0xff $(OUTPUTDIR)/$(TARGET) $(OUTPUTDIR)/$(TARGET).tmp 
	$(UTILITIES)/post_process$(EXE) $(OUTPUTDIR)/$(TARGET).tmp $@
	@grep "^\.fixed"          $(MAPFILE) | tail -n 1 >  $(SUMMARY)
	@grep "^\.revision"       $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "UserFlash"         $(MAPFILE) | tail -n 2 >> $(SUMMARY)
	@grep "^\.backupflash"    $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.cmdtab"         $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.bss"            $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.slcdcmem"       $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.volatileram"    $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.persistentram"  $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@cat $(SUMMARY)

# $(LDCTRL): wp34s.lds features.h Makefile
#	$(HOSTCC) -E -P -x c wp34s.lds > $(LDCTRL)

# include openocd/Makefile
else

# Target calc, console emulator

$(OUTPUTDIR)/calc: $(OBJS) $(OBJECTDIR)/xrom.o $(OBJECTDIR)/libdecNum34s.a $(CNSTS) \
		$(MAIN) $(LDCTRL) Makefile
	$(HOSTCC) $(CFLAGS) $(LDFLAGS) -o $@ $(MAIN) $(OBJS) $(OBJECTDIR)/xrom.o $(LIBDN) $(LIBS)

revision.h: $(UTILITIES)/create_revision$(EXE) $(HEADERS) $(SRCS) Makefile
	$(UTILITIES)/create_revision$(EXE) >$@
endif

# Build generated files
consts.c consts.h $(OBJECTDIR)/libconsts.a: $(UTILITIES)/compile_consts$(EXE) \
		$(DNHDRS) Makefile
	cd $(UTILITIES) \
		&& ./compile_consts$(EXE) "../" "../$(OBJECTDIR)/" \
		&& $(MAKE) "CFLAGS=$(CFLAGS) -I../.." -j2 -C consts

catalogues.h $(OPCODES): $(UTILITIES)/compile_cats$(EXE) Makefile pretty.h pretty.c font.c charmap.c translate.c font_alias.inc
	$(UTILITIES)/compile_cats$(EXE) >catalogues.h 2>$(OPCODES)

lcdmap.h: $(UTILITIES)/lcdgen$(EXE)
	$(UTILITIES)/lcdgen$(EXE) >$@

charset7.h: $(UTILITIES)/genchars7$(EXE)
	$(UTILITIES)/genchars7$(EXE) >$@

pretty.h font.c charmap.c translate.c: $(UTILITIES)/genfont$(EXE) Makefile
	$(UTILITIES)/genfont$(EXE)

$(UTILITIES)/compile_consts$(EXE): compile_consts.c $(DNSRCS) Makefile features.h \
		licence.h charmap.c pretty.h pretty.c translate.c font_alias.inc
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -DNEED_D64FROMSTRING -DNEED_D128FROMSTRING -o $@ $< $(DNSRCS)

$(UTILITIES)/compile_cats$(EXE): compile_cats.c consts.h xeq.h charmap.c \
		licence.h commands.c string.c prt.c consts.c pretty.c pretty.h Makefile features.h
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $<

$(UTILITIES)/lcdgen$(EXE): lcdgen.c licence.h Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/genchars7$(EXE): genchars7.c licence.h Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/genfont$(EXE): genfont.c font.inc font_alias.inc licence.h Makefile
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/create_revision$(EXE): create_revision.c licence.h Makefile
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/post_process$(EXE): post_process.c Makefile features.h xeq.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

xrom.c xrom_labels.h: xrom.wp34s $(XROM) $(OPCODES) Makefile features.h data.h errors.h
	$(HOSTCC) -E -P -x c -Ixrom -DCOMPILE_XROM xrom.wp34s > xrom_pre.wp34s
	$(TOOLS)/wp34s_asm.pl -pp -op $(OPCODES) -c -o xrom.c xrom_pre.wp34s

xeq.h:
	@touch xeq.h

# Build libs and objects

$(OBJECTDIR)/libdecNum34s.a: $(DNSRCS) $(DNHDRS) features.h decNumber/Makefile Makefile
	+@$(MAKE) OBJECTDIR=../$(OBJECTDIR) "CFLAGS=$(CFLAGS) $(DNOPTS)" "LIB=libdecNum34s.a" \
		-C decNumber

vpath %.c = atmel
$(OBJECTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJECTDIR)/alpha.o: alpha.c alpha.h xeq.h errors.h data.h decn.h int.h display.h consts.h \
		Makefile features.h
$(OBJECTDIR)/charmap.o: charmap.c xeq.h errors.h data.h Makefile features.h
$(OBJECTDIR)/commands.o: commands.c xeq.h errors.h data.h storage.h serial.h Makefile \
		features.h xrom.h xrom_labels.h
$(OBJECTDIR)/complex.o: complex.c decn.h complex.h xeq.h errors.h data.h consts.h \
		Makefile features.h
$(OBJECTDIR)/consts.o: consts.c consts.h Makefile features.h
$(OBJECTDIR)/date.o: date.c date.h consts.h decn.h xeq.h errors.h data.h alpha.h atmel/rtc.h \
		Makefile features.h
$(OBJECTDIR)/decn.o: decn.c decn.h xeq.h errors.h data.h consts.h complex.h int.h serial.h lcd.h Makefile features.h
$(OBJECTDIR)/display.o: display.c xeq.h errors.h data.h display.h consts.h lcd.h int.h revision.h \
		charset7.h decn.h alpha.h decn.h storage.h Makefile features.h
$(OBJECTDIR)/font.o: font.c
$(OBJECTDIR)/int.o: int.c int.h xeq.h errors.h data.h serial.h Makefile features.h
$(OBJECTDIR)/lcd.o: lcd.c lcd.h xeq.h errors.h data.h display.h lcdmap.h atmel/board.h \
		Makefile features.h
$(OBJECTDIR)/keys.o: keys.c catalogues.h xeq.h errors.h data.h keys.h consts.h display.h lcd.h \
		int.h xrom.h xrom_labels.h storage.h Makefile features.h
$(OBJECTDIR)/matrix.o: matrix.c matrix.h xeq.h errors.h decn.h consts.h Makefile features.h
$(OBJECTDIR)/prt.o: prt.c xeq.h errors.h data.h consts.h display.h Makefile features.h
$(OBJECTDIR)/serial.o: serial.c xeq.h errors.h serial.h storage.h Makefile
$(OBJECTDIR)/stats.o: stats.c xeq.h errors.h data.h decn.h stats.h consts.h int.h \
		Makefile features.h
$(OBJECTDIR)/string.o: string.c xeq.h errors.h data.h Makefile features.h
$(OBJECTDIR)/storage.o: storage.c xeq.h errors.h data.h storage.h Makefile features.h
$(OBJECTDIR)/xeq.o: xeq.c xeq.h errors.h data.h alpha.h decn.h complex.h int.h lcd.h stats.h \
		display.h consts.h date.h storage.h xrom.h xrom_labels.h Makefile features.h
$(OBJECTDIR)/xrom.o: xrom.c xrom.h xrom_labels.h xeq.h errors.h data.h consts.h Makefile features.h
$(OBJECTDIR)/stopwatch.o: stopwatch.c stopwatch.h decn.h xeq.h errors.h consts.h alpha.h display.h keys.h \
                Makefile features.h
$(OBJECTDIR)/printer.o: printer.c printer.h xeq.h errors.h alpha.h serial.h Makefile features.h

ifdef REALBUILD
$(OBJECTDIR)/board_lowlevel.o: atmel/board_lowlevel.c atmel/board_lowlevel.h \
		atmel/board.h Makefile
$(OBJECTDIR)/board_memories.o: atmel/board_memories.c atmel/board_memories.h \
		atmel/board.h Makefile
$(OBJECTDIR)/rtc.o: atmel/rtc.c atmel/rtc.h \
		atmel/board.h Makefile

$(OBJECTDIR)/main.o: main.c xeq.h errors.h data.h
else
$(OBJECTDIR)/console.o: console.c catalogues.h xeq.h errors.h data.h keys.h consts.h display.h lcd.h \
		int.h xrom.h xrom_labels.h storage.h Makefile features.h pretty.c pretty.h
ifeq ($(SYSTEM),windows32)
$(OBJECTDIR)/winserial.o: winserial.c serial.h Makefile
endif		
endif


qt_gui:
	$(MAKE) QTGUI=1 real_qt_gui

qt_gui_dist:
	$(MAKE) QTGUI=1 real_qt_gui_dist

qt_gui_no_serial:
	$(MAKE) QTGUI=1 real_qt_gui_no_serial

qt_gui_dist_no_serial:
	$(MAKE) QTGUI=1 real_qt_gui_dist_no_serial


CALCLIB := $(OBJECTDIR)/libCalculator.a
CALCOBJS := $(OBJS) $(OBJECTDIR)/xrom.o
BASELIBS := ../$(OBJECTDIR)/libCalculator.a ../$(OBJECTDIR)/libdecNum34s.a ../$(OBJECTDIR)/libconsts.a
$(CALCLIB): $(CALCOBJS)
	-rm -f $@
	$(AR) -r $@ $(CALCOBJS)
	$(RANLIB) $@

ifdef QTGUI

real_qt_gui: all $(CALCLIB)
	cd QtGui; $(MAKE) BASELIBS="$(BASELIBS)"

real_qt_gui_dist: all $(CALCLIB)
	cd QtGui; $(MAKE) BASELIBS="$(BASELIBS)" dist

real_qt_gui_no_serial: all $(CALCLIB)
	cd QtGui; $(MAKE) BASELIBS="$(BASELIBS)" NO_SERIAL=1 

real_qt_gui_dist_no_serial: all $(CALCLIB)
	cd QtGui; $(MAKE) BASELIBS="$(BASELIBS)" NO_SERIAL=1 dist
else
real_qt_gui:
	@echo "real_qt_gui should be called if QTGUI is not defined"

real_qt_gui_dist:
	@echo "real_qt_gui_dist should be called if QTGUI is not defined"

real_qt_gui_no_serial:
	@echo "real_qt_gui_no_serial should be called if QTGUI is not defined"

real_qt_gui_dist_no_serial:
	@echo "real_qt_gui_dist_no_serial should be called if QTGUI is not defined"	
endif

qt_clean:
	cd QtGui; $(MAKE) clean

qt_clean_all: qt_clean
		$(MAKE) QTGUI=1 clean

qt_clean_dist:
	cd QtGui; $(MAKE) distclean
	
.PHONY: ios ios_lib ios_objs ios_clean
	
ios:
	$(MAKE) SYSTEM=IosSimulator ios_lib
	$(MAKE) SYSTEM=IosSimulator64 ios_lib
	$(MAKE) SYSTEM=Ios ios_lib
	$(MAKE) SYSTEM=Ios64 ios_lib
	lipo -create -arch armv7 Ios/obj/libCalculator.a  -arch arm64 Ios64/obj/libCalculator.a -arch i386 IosSimulator/obj/libCalculator.a -arch x86_64 IosSimulator64/obj/libCalculator.a -output Ios/libCalculator.a
	lipo -create -arch armv7 Ios/obj/libconsts.a -arch arm64 Ios64/obj/libconsts.a -arch i386 IosSimulator/obj/libconsts.a -arch x86_64 IosSimulator64/obj/libconsts.a -output Ios/libconsts.a
	lipo -create -arch armv7 Ios/obj/libdecNum34s.a -arch arm64 Ios64/obj/libdecNum34s.a -arch i386 IosSimulator/obj/libdecNum34s.a -arch x86_64 IosSimulator64/obj/libdecNum34s.a -output Ios/libdecNum34s.a
			
ios_lib: ios_objs $(CALCLIB)

ios_objs: $(DIRS) $(OBJS) $(OBJECTDIR)/libdecNum34s.a $(CNSTS) $(LDCTRL) Makefile

ios_clean:
	rm -rf Ios Ios64 IosSimulator IosSimulator64
	
