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

.PHONY: clean all

PROGS := gl zc font

all: $(PROGS)

gl: gausslegendre.c Makefile
	cc -o gl gausslegendre.c

zc: zetaconsts.c Makefile
	cc -o zc zetaconsts.c -I.. -L../decNumber -ldecNumber 

font: font.c Makefile ../font.c ../charmap.c
	cc -o font font.c

clean:
	rm -f *.o $(PROGS)
