GCC=gcc -ansi -pedantic -Wall -o smtpc
CFLAGS=-a -ldflags '-s'

all: build

build:
	$(GCC) smtpc.c

run: build
	./smtpc

stat:
	$(GCC) $(CFLAGS) smtpc.c

clean:
	rm smtpc
