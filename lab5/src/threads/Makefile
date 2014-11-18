# ECE254 lab5 Linux Port
# Makefile: build mqueue produce and consume example code

CC=gcc
CFLAGS=-Wall -g -pthread
LD=gcc
LDFLAGS=-g -lrt -pthread -lm

OBJS1=produce.o

all: produce 

produce: $(OBJS1) 
	$(LD) $(OBJS1) -o $@ $(LDFLAGS) 

produce.o: produce.c
	$(CC) $(CFLAGS) -c produce.c 

.PHONY: clean
clean:
	rm -f *.o *.out produce
