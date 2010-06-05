# Makefile

CC=gcc -ansi -pedantic -Wall -DDEBUG

main: ensteg.o
	$(CC) -o test ensteg.o

ensteg.o: ensteg.c common.h
	$(CC) -c ensteg.c

desteg.o: desteg.c common.h
	$(CC) -c desteg.c

clean:
	@echo -n "$$ "
	git checkout pic.ppm
	@echo -n "$$ "
	rm -fv test *.o

test: main
	@echo
	echo -n "<<" | ./test -f pic.ppm
