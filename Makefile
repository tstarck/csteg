# Makefile

CC=gcc -ansi -pedantic -Wall

main: ensteg.o
	$(CC) -o test ensteg.o

ensteg.o: ensteg.c
	$(CC) -c ensteg.c

clean:
	@echo -n "$$ "
	rm -fv *.o

test: main
	@echo
	./test -f pic.ppm
