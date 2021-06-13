CC = gcc
SRC = zfs.c recover.c lz4.c fletcher.c list.c gzip.c nvlist.c
OBJS = $(SRC:%.c=%.o)

CFLAGS=-m64 -std=gnu99 -g
LDLIBS=-lmd -lz

recover: $(OBJS)
	$(CC) -o recover $(OBJS) $(LDLIBS)

clean:
	-rm -f *.o
