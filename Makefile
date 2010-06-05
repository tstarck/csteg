# Makefile

CC=gcc -ansi -pedantic -Wall -DDEBUG

main: ensteg.o common.o
	$(CC) -o test ensteg.o

ensteg.o: ensteg.c
	$(CC) -c ensteg.c

desteg.o: desteg.c
	$(CC) -c desteg.c

common.o: common.h
	$(CC) -c common.h

clean:
	@echo -n "$$ "
	git checkout pic.ppm
	@echo -n "$$ "
	rm -fv test *.o

test: main
	@echo
	echo -n "<<" | ./test -f pic.ppm
