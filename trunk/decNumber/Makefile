LIB = libdecNumber.a

INCS  = decContext.h decDPD.h decNumber.h decPacked.h
INCS += decimal128.h decimal32.h decimal64.h
INCS += ../features.h
# For some reason this confuses the C++ compiler building dependencies
# INCS += decNumberLocal.h 

SRCS  = decContext.c decNumber.c decPacked.c
#SRCS += decimal128.c decimal32.c decimal64.c
SRCS += decimal128.c decimal64.c

OBJS = $(SRCS:%.c=$(OBJECTDIR)/%.o)

gpath = $(OBJECTDIR)

SYSTEM := $(shell uname)
ifeq "$(SYSTEM)" "Darwin"
CFLAGS += -mmacosx-version-min=10.5
endif 

all: $(OBJECTDIR) $(OBJECTDIR)/$(LIB)


.PHONY: clean
clean:
	rm -rf $(OBJECTDIR)

$(OBJECTDIR):
	mkdir -p $@

$(OBJECTDIR)/$(LIB): $(OBJS) Makefile 
	-rm -f $@
	$(AR) -r $@ $(OBJS)
	$(RANLIB) $@

$(OBJECTDIR)/%.o: %.c $(INCS) Makefile
	$(CC) -c $(CFLAGS) -o $@ $<

