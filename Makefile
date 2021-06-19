CC = gcc
SRC = zfs.c recover.c lz4.c fletcher.c list.c gzip.c nvlist.c edonr.c \
      skein_block.c skein_iv.c skein.c
OBJS = $(SRC:%.c=%.o)

CPPFLAGS=-I.
CFLAGS=-std=gnu99 -m64 -g
LDLIBS=-lmd -lz

recover: $(OBJS)
	$(CC) $(CFLAGS) -o recover $(OBJS) $(LDLIBS)

clean:
	-rm -f *.o recover
