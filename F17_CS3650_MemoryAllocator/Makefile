
BINS := collatz-list-sys collatz-ivec-sys \
        collatz-list-hw7 collatz-ivec-hw7 \
        collatz-list-par collatz-ivec-par

HDRS := $(wildcard *.h)
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

CFLAGS := -std=gnu99 -g3
OPT_FLAGS := -o3
DEBUG_FLAGS := -g3 
LDLIBS := -lpthread -lm

all: $(BINS)

collatz-list-sys: list_main.o sys_malloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-sys: ivec_main.o sys_malloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-list-hw7: list_main.o hw07_malloc.o hmalloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-hw7: ivec_main.o hw07_malloc.o hmalloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-list-par: list_main.o par_malloc.o optmalloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

collatz-ivec-par: ivec_main.o par_malloc.o optmalloc.o utils.o
	gcc $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : %.c $(HDRS) Makefile

clean:
	rm -f *.o $(BINS) time.tmp outp.tmp

test:
	perl test.pl

.PHONY: clean test
