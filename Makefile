CC = gcc
SRC = zfs.c recover.c lz4.c fletcher.c list.c gzip.c nvlist.c
OBJS = $(SRC:%.c=%.o)

CFLAGS=-m64 -std=gnu99 -g -msave-args
LDLIBS=-lmd -lz

recover: $(OBJS)
	$(CC) -o recover $(OBJS) $(LDLIBS)
	ctfconvert recover

clean:
	-rm -f *.o
