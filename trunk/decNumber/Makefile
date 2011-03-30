LIB = libdecNumber.a

INCS  = decContext.h decDPD.h decNumber.h decPacked.h
INCS += decimal128.h decimal32.h decimal64.h
# For some reason this confuses the C++ compiler building dependencies
# INCS += decNumberLocal.h 

SRCS  = decContext.c decNumber.c decPacked.c
#SRCS += decimal128.c decimal32.c decimal64.c
SRCS += decimal64.c

OBJS = $(SRCS:%.c=$(OUTPUTDIR)/%.o)

gpath %.o = $(OUTPUTDIR)

all: $(OUTPUTDIR) $(OUTPUTDIR)/$(LIB)

.PHONY: clean
clean:
	rm -f Makefile.deps
	rm -rf $(OUTPUTDIR)

$(OUTPUTDIR):
	mkdir $@

$(OUTPUTDIR)/$(LIB): $(OBJS) $(OUTPUTDIR)
	-rm -f $@
	$(AR) -r $@ $(OBJS)
	$(RANLIB) $@

$(OUTPUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

