# ECE254 lab5 Linux Port

CC=gcc
CFLAGS=-Wall -g -pthread
LD=gcc
LDFLAGS=-g -lrt -pthread -lm

OBJS1=produce.o
OBJS2=consume.o

all: produce consume

produce: $(OBJS1) 
	$(LD) $(OBJS1) -o $@ $(LDFLAGS) 

produce.o: produce.c
	$(CC) $(CFLAGS) -c produce.c 

consume: $(OBJS2) 
	$(LD) $(OBJS2) -o $@ $(LDFLAGS) 

consume.o: consume.c
	$(CC) $(CFLAGS) -c consume.c 

.PHONY: clean
clean:
	rm -f *.o *.out produce consume
