# This file is part of 34S.
# 
# 34S is free software:	you can	redistribute it	and/or modify
# it under the terms of	the GNU	General	Public License as published by
# the Free Software Foundation,	either version 3 of the	License, or
# (at your option) any later version.
# 
# 34S is distributed in	the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along	with 34S.  If not, see <http://www.gnu.org/licenses/>.

.EXPORT_ALL_VARIABLES:

# Define to build the real thing
#REALBUILD = 1

BASE_CFLAGS := -Wall -Werror -g -fno-common -fno-inline-functions \
	-fno-defer-pop -fno-exceptions

# Settings for Unix like environments with gcc
# Creates the Console version of the emulator
# To build the GUI version on Windows use Microsoft Visual C++ Express

SYSTEM := $(shell uname)

CFLAGS = $(BASE_CFLAGS)
CFLAGS += -O0 -DUSECURSES -DDEBUG
OUTPUTDIR := $(SYSTEM)
UTILITIES := $(SYSTEM)
DIRS := $(SYSTEM)
CC := gcc
AR := ar
RANLIB := ranlib
EXE :=
STARTUP :=
LDFLAGS :=
LDCTRL :=

ifndef REALBUILD
# Select the correct parameters and libs for various Unix flavours
ifeq ($(SYSTEM),Linux)
LIBS +=	-lcurses
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
LIBS +=	-lcurses
endif
endif
endif

HOSTCC := $(CC)
HOSTAR := $(AR)
HOSTRANLIB := $(RANLIB)
HOSTCFLAGS := -Wall -Werror -O1 -g

ifdef REALBUILD

# Settings for the Yagarto tool	chain under Windows
# A standard Windows gcc is needed for building	the generated files.
# MinGW	will do	nicely

OUTPUTDIR := realbuild
DIRS += $(OUTPUTDIR)
CFLAGS := $(BASE_CFLAGS) -mthumb -Os -DREALBUILD -Dat91sam7l128 -Iatmel
LDFLAGS := -nostartfiles -Wl,--gc-sections,-Map=$(OUTPUTDIR)/Mapfile.txt
CROSS_COMPILE := arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RANLIB := $(CROSS_COMPILE)ranlib
SIZE := $(CROSS_COMPILE)size
STRIP := $(CROSS_COMPILE)strip
OBJCOPY	:= $(CROSS_COMPILE)objcopy
LIBS +=	-nostdlib -lgcc
EXE := .exe
STARTUP := atmel/board_cstartup.S \
	$(OUTPUTDIR)/board_lowlevel.o $(OUTPUTDIR)/board_memories.o
LDCTRL := atmel/at91sam7l128/flash.lds
LDFLAGS += -T $(LDCTRL)

endif

# Files	and libraries

OBJS := keys.o display.o xeq.o prt.o decn.o complex.o stats.o \
		lcd.o int.o date.o xrom.o consts.o alpha.o charmap.o \
		commands.o string.o
SRCS := $(OBJS:.o=.c)
OBJS := $(OBJS:%.o=$(OUTPUTDIR)/%.o)
HEADERS := alpha.h catalogues.h charset.h charset7.h complex.h consts.h date.h \
		decn.h display.h features.h hp.h int.h keys.h lcd.h lcdmap.h \
		statebits.h stats.h xeq.h xrom.h

LIBS += -L$(OUTPUTDIR) -lconsts
LIBDN := -ldecNumber
CNSTS := $(OUTPUTDIR)/libconsts.a

# Targets and rules

.PHONY:	clean tgz asone

all: calc
calc: $(OUTPUTDIR) $(OUTPUTDIR)/calc

ifdef REALBUILD
flash: $(OUTPUTDIR)/calc.bin
endif

clean:
	-rm -fr $(OUTPUTDIR)
	-rm -fr consts.h consts.c allconsts.c catalogues.h
	-rm -fr *.dSYM
	-@make -C decNumber clean
	-@make -C utilities clean

tgz:
	@make clean
	rm -f sci.tgz
	tar czf	sci.tgz	*

$(DIRS):
	mkdir $@

ifdef REALBUILD
$(OUTPUTDIR)/calc.bin: asone
	$(OBJCOPY) -O binary --gap-fill 0xff $(OUTPUTDIR)/calc $(OUTPUTDIR)/calc.bin
	$(SIZE) $$^ $(OUTPUTDIR)/calc >$(OUTPUTDIR)/sizes.txt
endif

asone: $(DIRS) asone.c $(HEADERS) $(SRCS) $(STARTUP) $(LDCTRL) Makefile
	$(CC) $(CFLAGS)	-IdecNumber -o $(OUTPUTDIR)/calc $(LDFLAGS) \
		$(STARTUP) asone.c $(LIBS) -fwhole-program 

$(OUTPUTDIR)/calc: $(DIRS) $(OUTPUTDIR)/decNumber.a $(CNSTS) $(OBJS) \
		$(STARTUP) $(LDCTRL) Makefile
	$(CC) $(CFLAGS)	$(LDFLAGS) -o $@ $(STARTUP) $(OBJS) $(LIBDN) $(LIBS)

$(OUTPUTDIR)/decNumber.a:
	+@make OUTPUTDIR=../$(OUTPUTDIR) -C decNumber

consts.c consts.h $(OUTPUTDIR)/libconsts.a: $(UTILITIES)/compile_consts$(EXE) \
		Makefile features.h
	cd $(UTILITIES) \
		&& ./compile_consts "../" "../$(OUTPUTDIR)/" \
		&& make "CFLAGS=$(CFLAGS) -I.." -j2 -C consts

catalogues.h: $(UTILITIES)/compile_cats$(EXE) Makefile features.h
	$(UTILITIES)/compile_cats >catalogues.h

lcdmap.h: $(UTILITIES)/lcdgen$(EXE)
	$(UTILITIES)/lcdgen >$@

charset7.h: $(UTILITIES)/genchars7$(EXE)
	$(UTILITIES)/genchars7 >$@

$(UTILITIES)/compile_consts$(EXE): compile_consts.c Makefile features.h
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $<

$(UTILITIES)/compile_cats$(EXE): compile_cats.c consts.h xeq.h charmap.c \
		commands.c string.c prt.c consts.c Makefile features.h
	$(HOSTCC) $(HOSTCFLAGS) -IdecNumber -o $@ $<

$(UTILITIES)/lcdgen$(EXE): lcdgen.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

$(UTILITIES)/genchars7$(EXE): genchars7.c Makefile lcd.h
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

xeq.h: statebits.h
	@touch xeq.h

vpath %.c = . atmel
$(OUTPUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OUTPUTDIR)/board_lowlevel.o: atmel/board_lowlevel.c atmel/board_lowlevel.h \
		atmel/board.h Makefile
$(OUTPUTDIR)/board_memories.o: atmel/board_memories.c atmel/board_memories.h \
		atmel/board.h Makefile
$(OUTPUTDIR)/alpha.o: alpha.c alpha.h xeq.h decn.h int.h display.h consts.h \
		Makefile features.h
$(OUTPUTDIR)/charmap.o:	charmap.c xeq.h	Makefile features.h
$(OUTPUTDIR)/commands.o: commands.c xeq.h Makefile features.h
$(OUTPUTDIR)/complex.o:	complex.c decn.h complex.h xeq.h consts.h \
		Makefile features.h
$(OUTPUTDIR)/consts.o: consts.c	consts.h Makefile features.h
$(OUTPUTDIR)/date.o: date.c date.h consts.h decn.h xeq.h alpha.h \
		Makefile features.h
$(OUTPUTDIR)/decn.o: decn.c decn.h xeq.h consts.h complex.h Makefile features.h
$(OUTPUTDIR)/display.o:	display.c xeq.h	display.h consts.h lcd.h int.h \
		charset.h charset7.h decn.h alpha.h decn.h Makefile features.h
$(OUTPUTDIR)/int.o: int.c int.h	xeq.h Makefile features.h
$(OUTPUTDIR)/lcd.o: lcd.c lcd.h	xeq.h display.h	lcdmap.h Makefile features.h
$(OUTPUTDIR)/keys.o: keys.c catalogues.h xeq.h keys.h consts.h display.h lcd.h \
		int.h xrom.h Makefile features.h
$(OUTPUTDIR)/prt.o: prt.c xeq.h	consts.h display.h Makefile features.h
$(OUTPUTDIR)/stats.o: stats.c xeq.h decn.h stats.h consts.h int.h \
		Makefile features.h
$(OUTPUTDIR)/string.o: string.c	xeq.h Makefile features.h
$(OUTPUTDIR)/xeq.o: xeq.c xeq.h	alpha.h	decn.h complex.h int.h lcd.h stats.h \
		display.h consts.h date.h statebits.h Makefile features.h
$(OUTPUTDIR)/xrom.o: xrom.c xrom.h xeq.h consts.h Makefile features.h


