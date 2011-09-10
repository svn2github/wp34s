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

BASE_CFLAGS := -Wall -Werror -g -fno-common \
	-fno-defer-pop -fno-exceptions # -fno-inline-functions 
OPT_CFLAGS := -Os

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
ifeq "$(findstring indows,$(SYSTEM))" "indows"
# Force REALBUILD on windows under MinGW / alternate uname utility
SYSTEM := windows32
REALBUILD := 1
endif

CFLAGS = $(BASE_CFLAGS)
CFLAGS += -O0 -DUSECURSES -DDEBUG
OUTPUTDIR := $(SYSTEM)
UTILITIES := $(SYSTEM)
TOOLS := tools
CC := gcc
AR := ar
RANLIB := ranlib
EXE :=
LDFLAGS :=
LDCTRL :=

ifndef REALBUILD
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
# Any other Unix
LIBS += -lcurses
endif
endif
endif

HOSTCC := $(CC)
HOSTAR := $(AR)
HOSTRANLIB := $(RANLIB)
HOSTCFLAGS := -Wall -Werror -O1 -g

ifdef REALBUILD

# Settings for the Yagarto tool chain under Windows
# A standard Windows gcc is needed for building the generated files.
# MinGW will do nicely

OUTPUTDIR := realbuild
CFLAGS := -mthumb -mcpu=arm7tdmi $(OPT_CFLAGS) $(BASE_CFLAGS)
CFLAGS += -DREALBUILD -Dat91sam7l128 -Iatmel
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
OBJECTDIR := $(OUTPUTDIR)_obj
DIRS += $(OBJECTDIR) $(UTILITIES)
else
OBJECTDIR := $(OUTPUTDIR)/obj
DIRS := $(OUTPUTDIR) $(OBJECTDIR)
endif

# Files and libraries

SRCS := keys.c display.c xeq.c prt.c decn.c complex.c stats.c \
		lcd.c int.c date.c xrom.c consts.c alpha.c charmap.c \
		commands.c string.c storage.c serial.c

HEADERS := alpha.h catalogues.h charset.h charset7.h complex.h consts.h data.h \
		date.h decn.h display.h features.h int.h keys.h lcd.h lcdmap.h \
		stats.h xeq.h xrom.h storage.h serial.h

OBJS := $(SRCS:%.c=$(OBJECTDIR)/%.o)
LIBS += -L$(OBJECTDIR) -lconsts
LIBDN := -ldecNum34s
CNSTS := $(OBJECTDIR)/libconsts.a

ifdef REALBUILD
STARTUP := atmel/board_cstartup.S
ATSRCS := board_lowlevel.c board_memories.c aic.c pmc.c rtc.c slcdc.c supc.c 
ATOBJS := $(ATSRCS:%.c=$(OBJECTDIR)/%.o)
ATSRCS := $(ATSRCS:%.c=atmel/%.c)
ATHDRS := $(ATSRCS:%.c=%.h) atmel/board.h atmel/at91sam7l128/AT91SAM7L128.h 

DNSRCS := decNumber.c decContext.c decimal64.c 
DNOBJS := $(DNSRCS:%.c=$(OBJECTDIR)/%.o)
DNSRCS := $(DNSRCS:%.c=decNumber/%.c)
DNHDRS := $(DNSRCS:%.c=%.h) 

LDCTRL := atmel/at91sam7l128/flash.lds
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

.PHONY: clean tgz asone flash version

ifdef REALBUILD
all: flash
flash: $(DIRS) $(OUTPUTDIR)/calc.bin
else
all: calc
calc: $(DIRS) $(OUTPUTDIR)/calc
endif

version:
	@$(CC) --version

clean:
	-rm -fr $(DIRS)
	-rm -fr consts.h consts.c allconsts.c catalogues.h xrom.c
#       -make -C decNumber clean
#       -make -C utilities clean

tgz:
	@make clean
	rm -f sci.tgz
	tar czf sci.tgz *

$(DIRS):
	mkdir -p -v $@

ifdef REALBUILD

# Target flash

$(OUTPUTDIR)/calc.bin: asone.c main.c $(HEADERS) $(SRCS) $(STARTUP) $(ATSRCS) $(ATHDRS) \
		$(DNHDRS) $(OBJECTDIR)/libconsts.a $(OBJECTDIR)/libdecNum34s.a \
		$(LDCTRL) Makefile $(UTILITIES)/post_process$(EXE)
	$(CC) $(CFLAGS) -IdecNumber -o $(OUTPUTDIR)/calc $(LDFLAGS) \
		$(STARTUP) asone.c $(LIBS) -fwhole-program -ldecNumber -save-temps
	$(NM) -n $(OUTPUTDIR)/calc >$(SYMBOLS)
	$(NM) -S $(OUTPUTDIR)/calc >>$(SYMBOLS)
	$(OBJCOPY) -O binary --gap-fill 0xff $(OUTPUTDIR)/calc $(OUTPUTDIR)/calc.tmp 
	$(UTILITIES)/post_process$(EXE) $(OUTPUTDIR)/calc.tmp $@
	@grep "^\.fixed"     $(MAPFILE) | tail -n 1 >  $(SUMMARY)
	@grep "^\.revision"  $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.userflash" $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.cmdtab"    $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.bss"       $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.slcdcmem"  $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@grep "^\.backup"    $(MAPFILE) | tail -n 1 >> $(SUMMARY)
	@cat $(SUMMARY)

# include openocd/Makefile
else

# Target calc, console emulator

$(OUTPUTDIR)/calc: $(OBJS) $(OBJECTDIR)/libdecNum34s.a $(CNSTS) \
		$(MAIN) $(LDCTRL) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(MAIN) $(OBJS) $(LIBDN) $(LIBS)
endif

# Build generated files

consts.c consts.h $(OBJECTDIR)/libconsts.a: $(UTILITIES)/compile_consts$(EXE) \
		$(DNHDRS) Makefile
	cd $(UTILITIES) \
		&& ./compile_consts$(EXE) "../" "../$(OBJECTDIR)/" \
		&& make "CFLAGS=$(CFLAGS) -I../.." -j2 -C consts

catalogues.h $(OPCODES): $(UTILITIES)/compile_cats$(EXE) Makefile
	echo "# \$$Rev\$$" > $(OPCODES)
	$(UTILITIES)/compile_cats$(EXE) >catalogues.h 2>>$(OPCODES)

lcdmap.h: $(UTILITIES)/lcdgen$(EXE)
	$(UTILITIES)/lcdgen$(EXE) >$@

charset7.h: $(UTILITIES)/genchars7$(EXE)
	$(UTILITIES)/genchars7$(EXE) >$@
	
$(UTILITIES)/compile_consts$(EXE): compile_consts.c Makefile features.h \
		charset.h charmap.c
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $<

$(UTILITIES)/compile_cats$(EXE): compile_cats.c consts.h xeq.h charmap.c \
		commands.c string.c prt.c consts.c pretty.c Makefile features.h
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $< -O0 -g

$(UTILITIES)/lcdgen$(EXE): lcdgen.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/genchars7$(EXE): genchars7.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/post_process$(EXE): post_process.c Makefile features.h xeq.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

xrom.c: xrom.wp34s $(OPCODES) Makefile xeq.h
	tools/wp34s_asm.pl -pp -op $(OPCODES) -c -o xrom.c xrom.wp34s

xeq.h:
	@touch xeq.h

# Build libs and objects

$(OBJECTDIR)/libdecNum34s.a: $(DNSRCS) $(DNHDRS)
	+@make OBJECTDIR=../$(OBJECTDIR) "CFLAGS=$(CFLAGS)" "LIB=libdecNum34s.a" \
		-C decNumber

vpath %.c = atmel
$(OBJECTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJECTDIR)/alpha.o: alpha.c alpha.h xeq.h data.h decn.h int.h display.h consts.h \
		Makefile features.h
$(OBJECTDIR)/charmap.o: charmap.c xeq.h data.h Makefile features.h
$(OBJECTDIR)/commands.o: commands.c xeq.h data.h storage.h serial.h Makefile \
		features.h
$(OBJECTDIR)/complex.o: complex.c decn.h complex.h xeq.h data.h consts.h \
		Makefile features.h
$(OBJECTDIR)/consts.o: consts.c consts.h Makefile features.h
$(OBJECTDIR)/date.o: date.c date.h consts.h decn.h xeq.h data.h alpha.h atmel/rtc.h \
		Makefile features.h
$(OBJECTDIR)/decn.o: decn.c decn.h xeq.h data.h consts.h complex.h int.h serial.h lcd.h Makefile features.h
$(OBJECTDIR)/display.o: display.c xeq.h data.h display.h consts.h lcd.h int.h \
		charset.h charset7.h decn.h alpha.h decn.h storage.h Makefile features.h
$(OBJECTDIR)/int.o: int.c int.h xeq.h data.h serial.h Makefile features.h
$(OBJECTDIR)/lcd.o: lcd.c lcd.h xeq.h data.h display.h lcdmap.h atmel/board.h \
		Makefile features.h
$(OBJECTDIR)/keys.o: keys.c catalogues.h xeq.h data.h keys.h consts.h display.h lcd.h \
		int.h xrom.h storage.h Makefile features.h
$(OBJECTDIR)/prt.o: prt.c xeq.h data.h consts.h display.h Makefile features.h
$(OBJECTDIR)/serial.o: serial.c xeq.h serial.h storage.h Makefile
$(OBJECTDIR)/stats.o: stats.c xeq.h data.h decn.h stats.h consts.h int.h \
		Makefile features.h
$(OBJECTDIR)/string.o: string.c xeq.h data.h Makefile features.h
$(OBJECTDIR)/storage.o: storage.c xeq.h data.h storage.h Makefile features.h
$(OBJECTDIR)/xeq.o: xeq.c xeq.h data.h alpha.h decn.h complex.h int.h lcd.h stats.h \
		display.h consts.h date.h storage.h Makefile features.h
$(OBJECTDIR)/xrom.o: xrom.c xrom.h xeq.h data.h consts.h Makefile features.h

ifdef REALBUILD
$(OBJECTDIR)/board_lowlevel.o: atmel/board_lowlevel.c atmel/board_lowlevel.h \
		atmel/board.h Makefile
$(OBJECTDIR)/board_memories.o: atmel/board_memories.c atmel/board_memories.h \
		atmel/board.h Makefile
$(OBJECTDIR)/rtc.o: atmel/rtc.c atmel/rtc.h \
		atmel/board.h Makefile

$(OBJECTDIR)/main.o: main.c xeq.h data.h
else
$(OBJECTDIR)/console.o: console.c catalogues.h xeq.h data.h keys.h consts.h display.h lcd.h \
		int.h xrom.h storage.h Makefile features.h pretty.c
endif

