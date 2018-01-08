PROGRAMS= netfileserver netclient
SRCS = libnetfiles.c netclient.c 
OBJS = $(SRCS: .c=.o)
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
RM = rm -f
netfileserver: netfileserver.c
	$(CC) $(LFLAGS) netfileserver.c -lpthread -o netfileserver 
netclient: $(OBJS) 
	$(CC) $(LFLAGS) $(OBJS) -o netclient
libnetfiles.o: libnetfiles.c libnetfiles.h
	$(CC) $(CFLAGS) libnetfiles.c
netclient.o: netclient.c
	$(CC) $(CFLAGS) netclient.c
.PHONY: all clean
all: netfileserver netclient
clean:
	$(RM) *.o $(PROGRAMS)