.PHONY: clean all

PROGS := gl zc font genchars7

all: $(PROGS)

gl: gausslegendre.c Makefile
	cc -o gl gausslegendre.c

zc: zetaconsts.c Makefile
	cc -o zc zetaconsts.c -I.. -L../decNumber -ldecNumber 

font: font.c Makefile ../charset.h
	cc -o font font.c

genchars7: genchars7.c Makefile
	cc -o genchars7 genchars7.c

clean:
	rm -f *.o $(PROGS)
