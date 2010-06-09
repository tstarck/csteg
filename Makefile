# Makefile

CFLAGS=-ansi -pedantic -Wall -DDEBUG

all: main

main: desteg.o ensteg.o common.o
	$(CC) $(CFLAGS) -o desteg desteg.o common.o
	$(CC) $(CFLAGS) -o ensteg ensteg.o common.o

common.o: common.c
	$(CC) $(CFLAGS) -c $^

desteg.o: desteg.c
	$(CC) $(CFLAGS) -c $^

ensteg.o: ensteg.c
	$(CC) $(CFLAGS) -c $^

clean:
	@echo -n "$$ "
	rm -fv *.o desteg ensteg
