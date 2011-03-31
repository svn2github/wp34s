LIB = libdecNumber.a

INCS  = decContext.h decDPD.h decNumber.h decPacked.h
INCS += decimal128.h decimal32.h decimal64.h
# For some reason this confuses the C++ compiler building dependencies
# INCS += decNumberLocal.h 

SRCS  = decContext.c decNumber.c decPacked.c
#SRCS += decimal128.c decimal32.c decimal64.c
SRCS += decimal64.c

OBJS = $(SRCS:%.c=$(OBJECTDIR)/%.o)

gpath %.o = $(OBJECTDIR)

all: $(OBJECTDIR) $(OBJECTDIR)/$(LIB)

.PHONY: clean
clean:
	rm -rf $(OBJECTDIR)

$(OBJECTDIR):
	mkdir $@

$(OBJECTDIR)/$(LIB): $(OBJS) 
	-rm -f $@
	$(AR) -r $@ $(OBJS)
	$(RANLIB) $@

$(OBJECTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

