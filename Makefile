# Makefile

CC=gcc -ansi -pedantic -Wall -DDEBUG

main: ensteg.o
	$(CC) -o test ensteg.o

ensteg.o: ensteg.c
	$(CC) -c ensteg.c

clean:
	@echo -n "$$ "
	git checkout pic.ppm
	@echo -n "$$ "
	rm -fv test *.o

test: main
	@echo
	echo -n foo | ./test -f pic.ppm
