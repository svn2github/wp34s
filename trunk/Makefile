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

# Define to use a crystal oscillator on slow clock after hardware modification
#XTAL = 1

BASE_CFLAGS := -Wall -Werror -g -fno-common -fno-exceptions 
OPT_CFLAGS := -Os -fira-region=one

# Settings for Unix like environments with gcc
# Creates the Console version of the emulator or the real thing
# To build the GUI version on Windows use Microsoft Visual C++ (Express)

SYSTEM := $(shell uname)
ifeq "$(SYSTEM)" ""
SYSTEM := Output
endif
ifeq "$(findstring MINGW,$(SYSTEM))" "MINGW"
SYSTEM := windows32
endif
ifeq "$(findstring CYGWIN,$(SYSTEM))" "CYGWIN"
SYSTEM := windows32
endif
ifeq "$(findstring indows,$(SYSTEM))" "indows"
# Any other Windows GCC is mapped to windows32
SYSTEM := windows32
endif

ifeq ($(SYSTEM),windows32)
EXE := .exe
#ifdef QTGUI
MAKE=mingw32-make
CC=mingw32-gcc
CXX=mingw32-g++
#endif
else
EXE :=
endif

CFLAGS = $(BASE_CFLAGS)
ifdef QTGUI
CFLAGS += -O0 -DDEBUG -DQTGUI 
else
CFLAGS += -O0 -DDEBUG -DUSECURSES
endif

ifndef REALBUILD
ifndef QTGUI
# Select the correct parameters and libs for various Unix flavours
ifeq ($(SYSTEM),Linux)
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
LDFLAGS :=
LDCTRL :=

HOSTCC := $(CC)
HOSTAR := $(AR)
HOSTRANLIB := $(RANLIB)
HOSTCFLAGS := -Wall -Werror -O1 -g

ifdef REALBUILD
# Settings for the Yagarto tool chain under Windows (or MacOS)
# On Windows a standard gcc is needed for building the generated files.
# MinGW will do nicely

CFLAGS := -mthumb -mcpu=arm7tdmi $(OPT_CFLAGS) $(BASE_CFLAGS)
CFLAGS += -DREALBUILD -Dat91sam7l128 -Iatmel

ifeq ($(SYSTEM),Darwin)
# MacOS - uses 32 bits pointer or code won't compile
HOSTCFLAGS += -m32
endif
HOSTCFLAGS += -DREALBUILD
ifdef NOWD
CFLAGS += -DNOWD
endif
ifdef XTAL
CFLAGS += -DXTAL
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
		lcd.c int.c date.c xrom.c consts.c alpha.c charmap.c \
		commands.c string.c storage.c serial.c matrix.c \
		stopwatch.c
ifeq ($(SYSTEM),windows32)
SRCS += winserial.c
endif

HEADERS := alpha.h catalogues.h charset.h charset7.h complex.h consts.h data.h \
		date.h decn.h display.h features.h int.h keys.h lcd.h lcdmap.h \
		stats.h xeq.h xrom.h xrom_labels.h storage.h serial.h matrix.h \
		stopwatch.h 

XROM := $(wildcard xrom/*.wp34s)

OBJS := $(SRCS:%.c=$(OBJECTDIR)/%.o)

LIBS += -L$(OBJECTDIR) -lconsts
LIBDN := -ldecNum34s
CNSTS := $(OBJECTDIR)/libconsts.a

DNSRCS := decNumber.c decContext.c decimal64.c decimal128.c
DNOBJS := $(DNSRCS:%.c=$(OBJECTDIR)/%.o)
DNSRCS := $(DNSRCS:%.c=decNumber/%.c)
DNHDRS := $(DNSRCS:%.c=%.h) 

ifdef REALBUILD
STARTUP := atmel/board_cstartup.S
ATSRCS := board_lowlevel.c board_memories.c aic.c pmc.c rtc.c slcdc.c supc.c 
ATOBJS := $(ATSRCS:%.c=$(OBJECTDIR)/%.o)
ATSRCS := $(ATSRCS:%.c=atmel/%.c)
ATHDRS := $(ATSRCS:%.c=%.h) atmel/board.h atmel/at91sam7l128/AT91SAM7L128.h 

LDCTRL := wp34s.lds
MAPFILE := $(OUTPUTDIR)/mapfile.txt
SUMMARY := $(OUTPUTDIR)/summary.txt
SYMBOLS := $(OUTPUTDIR)/symbols.txt
LDFLAGS += -T $(LDCTRL) -Wl,--gc-sections,-Map=$(MAPFILE)
MAIN := $(OBJECTDIR)/main.o
else
MAIN := $(OBJECTDIR)/console.o
endif
OPCODES := $(TOOLS)/wp34s.op

# Targets and rules

.PHONY: clean tgz asone flash version qt_gui real_qt_gui qt_clean qt_clean_all

ifdef REALBUILD
all: flash
flash: $(DIRS) $(OUTPUTDIR)/calc.bin
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
	-rm -f xrom_pre.wp34s
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

$(OUTPUTDIR)/calc.bin: asone.c main.c $(HEADERS) $(SRCS) $(STARTUP) $(ATSRCS) $(ATHDRS) \
		$(DNHDRS) $(OBJECTDIR)/libconsts.a $(OBJECTDIR)/libdecNum34s.a \
		$(LDCTRL) Makefile $(UTILITIES)/post_process$(EXE) $(UTILITIES)/create_revision$(EXE)
	$(UTILITIES)/create_revision$(EXE) >revision.h
	$(CC) $(CFLAGS) -IdecNumber -o $(OUTPUTDIR)/calc $(LDFLAGS) \
		$(STARTUP) asone.c $(LIBS) -fwhole-program -ldecNum34s -save-temps
	$(NM) -n $(OUTPUTDIR)/calc >$(SYMBOLS)
	$(NM) -S $(OUTPUTDIR)/calc >>$(SYMBOLS)
	$(OBJCOPY) -O binary --gap-fill 0xff $(OUTPUTDIR)/calc $(OUTPUTDIR)/calc.tmp 
	$(UTILITIES)/post_process$(EXE) $(OUTPUTDIR)/calc.tmp $@
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

$(OUTPUTDIR)/calc: $(OBJS) $(OBJECTDIR)/libdecNum34s.a $(CNSTS) \
		$(MAIN) $(LDCTRL) Makefile
	$(HOSTCC) $(CFLAGS) $(LDFLAGS) -o $@ $(MAIN) $(OBJS) $(LIBDN) $(LIBS)

revision.h: $(UTILITIES)/create_revision$(EXE) $(HEADERS) $(SRCS) Makefile
	$(UTILITIES)/create_revision$(EXE) >$@
endif

# Build generated files

consts.c consts.h $(OBJECTDIR)/libconsts.a: $(UTILITIES)/compile_consts$(EXE) \
		$(DNHDRS) Makefile
	cd $(UTILITIES) \
		&& ./compile_consts$(EXE) "../" "../$(OBJECTDIR)/" \
		&& make "CFLAGS=$(CFLAGS) -I../.." -j2 -C consts
		
catalogues.h $(OPCODES): $(UTILITIES)/compile_cats$(EXE) Makefile
	$(UTILITIES)/compile_cats$(EXE) >catalogues.h 2>$(OPCODES)

lcdmap.h: $(UTILITIES)/lcdgen$(EXE)
	$(UTILITIES)/lcdgen$(EXE) >$@

charset7.h: $(UTILITIES)/genchars7$(EXE)
	$(UTILITIES)/genchars7$(EXE) >$@

$(UTILITIES)/compile_consts$(EXE): compile_consts.c $(DNSRCS) Makefile features.h \
		charset.h charmap.c
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -DNEED_D64FROMSTRING -DNEED_D128FROMSTRING -o $@ $< $(DNSRCS)

$(UTILITIES)/compile_cats$(EXE): compile_cats.c consts.h xeq.h charmap.c \
		commands.c string.c prt.c consts.c pretty.c Makefile features.h
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $< -O0 -g

$(UTILITIES)/lcdgen$(EXE): lcdgen.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/genchars7$(EXE): genchars7.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/create_revision$(EXE): create_revision.c Makefile
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/post_process$(EXE): post_process.c Makefile features.h xeq.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

xrom.c xrom_labels.h: xrom.wp34s $(XROM) $(OPCODES) Makefile features.h data.h errors.h
	$(HOSTCC) -E -P -x c -Ixrom -DCOMPILE_XROM xrom.wp34s > xrom_pre.wp34s
	tools/wp34s_asm.pl -pp -op $(OPCODES) -c -o xrom.c xrom_pre.wp34s

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
		features.h
$(OBJECTDIR)/complex.o: complex.c decn.h complex.h xeq.h errors.h data.h consts.h \
		Makefile features.h
$(OBJECTDIR)/consts.o: consts.c consts.h Makefile features.h
$(OBJECTDIR)/date.o: date.c date.h consts.h decn.h xeq.h errors.h data.h alpha.h atmel/rtc.h \
		Makefile features.h
$(OBJECTDIR)/decn.o: decn.c decn.h xeq.h errors.h data.h consts.h complex.h int.h serial.h lcd.h Makefile features.h
$(OBJECTDIR)/display.o: display.c xeq.h errors.h data.h display.h consts.h lcd.h int.h revision.h \
		charset.h charset7.h decn.h alpha.h decn.h storage.h Makefile features.h
$(OBJECTDIR)/int.o: int.c int.h xeq.h errors.h data.h serial.h Makefile features.h
$(OBJECTDIR)/lcd.o: lcd.c lcd.h xeq.h errors.h data.h display.h lcdmap.h atmel/board.h \
		Makefile features.h
$(OBJECTDIR)/keys.o: keys.c catalogues.h xeq.h errors.h data.h keys.h consts.h display.h lcd.h \
		int.h xrom.h storage.h Makefile features.h
$(OBJECTDIR)/matrix.o: matrix.c matrix.h xeq.h errors.h decn.h consts.h Makefile features.h
$(OBJECTDIR)/prt.o: prt.c xeq.h errors.h data.h consts.h display.h Makefile features.h
$(OBJECTDIR)/serial.o: serial.c xeq.h errors.h serial.h storage.h Makefile
$(OBJECTDIR)/stats.o: stats.c xeq.h errors.h data.h decn.h stats.h consts.h int.h \
		Makefile features.h
$(OBJECTDIR)/string.o: string.c xeq.h errors.h data.h Makefile features.h
$(OBJECTDIR)/storage.o: storage.c xeq.h errors.h data.h storage.h Makefile features.h
$(OBJECTDIR)/xeq.o: xeq.c xeq.h errors.h data.h alpha.h decn.h complex.h int.h lcd.h stats.h \
		display.h consts.h date.h storage.h Makefile features.h
$(OBJECTDIR)/xrom.o: xrom.c xrom.h xeq.h errors.h data.h consts.h Makefile features.h
$(OBJECTDIR)/stopwatch.o: stopwatch.c stopwatch.h decn.h xeq.h errors.h consts.h alpha.h display.h keys.h \
                Makefile features.h

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
		int.h xrom.h storage.h Makefile features.h pretty.c
ifeq ($(SYSTEM),windows32)
$(OBJECTDIR)/winserial.o: winserial.c serial.h Makefile
endif		
endif


qt_gui:
	$(MAKE) QTGUI=1 REALBUILD="" real_qt_gui

ifdef QTGUI

CALCLIB := $(OBJECTDIR)/libCalculator.a
$(CALCLIB): $(OBS)
	-rm -f $@
	$(AR) -r $@ $(OBJS)
	$(RANLIB) $@

real_qt_gui: all $(CALCLIB)
	cd QtGui; $(MAKE) BASELIBS="-L../$(OBJECTDIR) -lCalculator -ldecNum34s -lconsts"
else
real_qt_gui:
	@echo "real_qt_gui should be called if QTGUI is not defined"
endif

qt_clean:
	cd QtGui; $(MAKE) clean

qt_clean_all: qt_clean
		$(MAKE) QTGUI=1 clean	
