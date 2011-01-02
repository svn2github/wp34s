LIB = libdecNumber.a
EXAMPLES = example1 example2 example3 example4 example5 example6
CFLAGS += -Os -Wall

INCS  = decContext.h decDPD.h decNumber.h decPacked.h
INCS += decimal128.h decimal32.h decimal64.h
# For some reason this confuses the C++ compiler building dependencies
# INCS += decNumberLocal.h 

SRCS  = decContext.c decNumber.c decPacked.c
#SRCS += decimal128.c decimal32.c decimal64.c
SRCS += decimal64.c

OBJS = $(SRCS:%.c=%.o)

all: $(LIB)

.PHONY: clean
clean:
	rm -f $(LIB) $(OBJS) example[1-6] Makefile.deps

$(LIB): $(OBJS)
	-rm -f $(LIB)
	$(AR) -r $(LIB) $(OBJS)
	ranlib $(LIB)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

examples: $(LIB) $(EXAMPLES)

example1: example1.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber
example2: example2.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber
example3: example3.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber
example4: example4.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber
example5: example5.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber
example6: example6.c Makefile
	$(CC) $(CFLAGS) -o $@ $< -L. -ldecNumber

Makefile.deps: $(SRCS) $(INCS)
	$(CC) $(CFLAGS) -M $? >> Makefile.deps

include Makefile.deps
