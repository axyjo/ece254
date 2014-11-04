# ECE254 lab4 Linux Port
# Makefile: build mqueue produce and consume example code

CC=gcc
CFLAGS=-Wall -g
LD=gcc
LDFLAGS=-g -lrt


OBJS1=produce.o
OBJS2=consume.o

all: produce consume

produce: $(OBJS1) 
	$(LD) $(LDFLAGS) -o $@ $(OBJS1) 

produce.o: produce.c
	$(CC) $(CFLAGS) -c produce.c 

consume: $(OBJS2) 
	$(LD) $(LDFLAGS) -o $@ $(OBJS2) 

consume.o: consume.c
	$(CC) $(CFLAGS) -c consume.c 

.PHONY: clean
clean:
	rm -f *.o *.out produce consume
