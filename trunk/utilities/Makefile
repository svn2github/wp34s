.PHONY: clean all

PROGS := gl zc font

all: $(PROGS)

gl: gausslegendre.c Makefile
	cc -o gl gausslegendre.c

zc: zetaconsts.c Makefile
	cc -o zc zetaconsts.c -I.. -L../decNumber -ldecNumber 

font: font.c Makefile ../charset.h
	cc -o font font.c

clean:
	rm -f *.o $(PROGS)
