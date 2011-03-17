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
#REALBUILD := 1

CFLAGS := -Wall -Werror -g -fno-common -fno-inline-functions -fno-defer-pop

# Optional routines to include...
###########################################################################
# It's better to do this in features.h to assure consistency between builds

# Include a catalogue of the internal commands
#CFLAGS += -DINCLUDE_INTERNAL_CATALOGUE

# Include the aritmetic/geometric mean iteration
#CFLAGS += -DINCLUDE_AGM

# Include Reiman's Zeta function for real and complex arguments
#CFLAGS += -DINCLUDE_ZETA

# Incude the digamma function
#CFLAGS += -DINCLUDE_DIGAMMA

# Include Jacobi's Elliptical Functions: SN, CN & DN for real & complex arguments
#CFLAGS += -DINCLUDE_ELLIPTIC

# Include Bessel functions of first and second kind
# Second kind functions of integer order need digamma
#CFLAGS += -DINCLUDE_BESSEL -DINCLUDE_DIGAMMA
#CFLAGS += -DCOMPLEX_BESSEL

# Inlcude a fused multiply add instruction
# This isn't vital since this can be done using a complex addition.
#CFLAGS += -DINCLUDE_MULADD

# Include cube and cube root functions
#CFLAGS += -DINCLUDE_CUBES

# Include the !! function defined over the complex plane
#CFLAGS += -DINCLUDE_DBLFACT

# Include the !! function defined over the reals
#CFLAGS += -DINCLUDE_SUBFACT

# Inlcude multi-character alpha constants (not keystroke programmable)
#CFLAGS += -DMULTI_ALPHA

# Include a date function to determine the date of Easter in a given year
#CFLAGS += -DINCLUDE_EASTER

# Include code to use a Ridder's method step after a bisection in the solver.
#CFLAGS += -DUSE_RIDDERS
###########################################################################

OBJS := keys.o display.o xeq.o prt.o decn.o complex.o stats.o \
		lcd.o int.o date.o xrom.o consts.o alpha.o charmap.o \
		commands.o string.o
SRCS := $(OBJS:.o=.c)

LIBS := -L. -lconsts
LIBDN := -LdecNumber -ldecNumber

ifdef REALBUILD
CFLAGS += -Os
else
CFLAGS += -O0 -g
USECURSES := 1
ifeq ($(shell uname),Linux)
CC := gcc
else
ifeq ($(shell uname),Darwin)
CC := gcc
CFLAGS += -m32
else
CC := gcc-4
endif
endif
endif


ifdef USECURSES
LIBS += -lcurses
#LIBS += /sw/lib/libncurses.a
CFLAGS += -DUSECURSES
endif

ifndef REALBUILD
CFLAGS += -DDEBUG
endif

CNSTS := libconsts.a

# Choose the cross compiler arm-elf is hpgcc, the other choice is a slightly
# later gcc which produces circa 1% smaller code.
CROSS := arm-elf-
#CROSS := arm-linux-20070808-
HOSTCC := gcc
ifdef REALBUILD
CC := $(CROSS)gcc -mthumb
RANLIB := $(CROSS)ranlib
AR := $(CROSS)ar
CFLAGS += -DREALBUILD
LIBS += -nostdlib -lgcc
else
RANLIB := ranlib
endif

.PHONY: clean tgz asone

all: calc
clean:
	rm -f calc asone *.o wp34s.dat
	rm -fr consts $(CNSTS) consts.h consts.c catalogues.h
	rm -fr compile_consts compile_consts.dSYM lcdgen lcdgen.dSYM
	rm -rf compile_cats compile_cats.dSYM genchars7 genchars7.dSYM
	@make -C decNumber clean
	@make -C utilities clean
tgz:
	@make clean
	rm -f sci.tgz
	tar czf sci.tgz *

calc: decNumber/decNumber.a $(OBJS)
	$(CC) $(CFLAGS) -g -o $@ $(OBJS) $(LIBDN) $(LIBS)

asone: asone.c catalogues.h Makefile decNumber/decNumber.a lcdmap.h features.h \
		$(SRCS) charset7.h
	$(CC) $(CFLAGS) -IdecNumber -g -o calc $< $(LIBS) -fwhole-program

decNumber/decNumber.a:
	+@make -C decNumber

consts.c consts.h: compile_consts Makefile features.h
	./compile_consts
	make -j2 -C consts

catalogues.h: compile_cats Makefile features.h
	./compile_cats >catalogues.h

lcdmap.h: lcdgen
	./lcdgen >$@

charset7.h: genchars7
	./genchars7 >$@

compile_consts: compile_consts.c Makefile features.h
	$(HOSTCC) -IdecNumber -g -O1 -o $@ $<  -Wall -Werror

lcdgen: lcdgen.c Makefile lcd.h
	$(HOSTCC) -g -O1 -o $@ $<  -Wall -Werror

genchars7: genchars7.c Makefile lcd.h
	$(HOSTCC) -g -O1 -o $@ $<  -Wall -Werror

compile_cats: compile_cats.c consts.h xeq.h charmap.c commands.c \
		string.c prt.c consts.c Makefile features.h
	$(HOSTCC) $(CFLAGS) -IdecNumber -g -O1 -o $@ $<  -Wall -Werror

xeq.h: statebits.h
	@touch xeq.h
alpha.o: alpha.c alpha.h xeq.h decn.h int.h display.h consts.h Makefile \
		features.h
charmap.o: charmap.c xeq.h Makefile features.h
commands.o: commands.c xeq.h Makefile features.h
complex.o: complex.c decn.h complex.h xeq.h consts.h Makefile features.h
consts.o: consts.c consts.h Makefile features.h
date.o: date.c date.h consts.h decn.h xeq.h alpha.h Makefile features.h
decn.o: decn.c decn.h xeq.h consts.h complex.h Makefile features.h
display.o: display.c xeq.h display.h consts.h lcd.h int.h charset.h \
		charset7.h decn.h alpha.h decn.h Makefile features.h
int.o: int.c int.h xeq.h Makefile features.h
lcd.o: lcd.c lcd.h xeq.h display.h lcdmap.h Makefile features.h
keys.o: keys.c catalogues.h xeq.h keys.h consts.h display.h lcd.h \
		int.h xrom.h Makefile features.h
prt.o: prt.c xeq.h consts.h display.h Makefile features.h
stats.o: stats.c xeq.h decn.h stats.h consts.h int.h Makefile features.h
string.o: string.c xeq.h Makefile features.h
xeq.o: xeq.c xeq.h alpha.h decn.h complex.h int.h lcd.h stats.h \
		display.h consts.h date.h statebits.h Makefile features.h
xrom.o: xrom.c xrom.h xeq.h consts.h Makefile features.h

