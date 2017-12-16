
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)
HDRS := $(wildcard include/*.h)

CFLAGS := -g `pkg-config fuse --cflags` 
LDLIBS := `pkg-config fuse --libs` -lbsd -lm

nufs: $(SRCS)
	gcc $(CFLAGS) -Iinclude/ -o nufs $(SRCS) $(LDLIBS)

clean: unmount
	rm -f nufs *.o test.log data.nufs
	rmdir mnt || true

mount: nufs
	mkdir -p mnt || true
	#./nufs -d -f mnt data.nufs
	./nufs -s -f mnt data.nufs

unmount:
	fusermount -u mnt || true

test: nufs
	perl test.pl

gdb: nufs
	mkdir -p mnt || true
	gdb --args ./nufs -f mnt data.nufs

.PHONY: clean mount unmount gdb

