CC = gcc
SRC = zfs.c recover.c lz4.c fletcher.c list.c gzip.c nvlist.c edonr.c \
      skein_block.c skein_iv.c skein.c
OBJS = $(SRC:%.c=%.o)

CPPFLAGS=-I.
CFLAGS=-std=gnu99 -m64 -g
LDLIBS=-lmd -lz

all: recover search

recover: $(OBJS)
	$(CC) $(CFLAGS) -o recover $(OBJS) $(LDLIBS)

search: search.o lzjb.o
	$(CC) $(CFLAGS) -o search search.o lzjb.o

clean:
	-rm -f *.o recover search
