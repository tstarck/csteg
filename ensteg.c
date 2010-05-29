/* ensteg.c
 * Steganography encoder
 * Copyright 2010 Tuomas Starck
 *
 * TODO:
 * - spec: ensteg && desteg
 * - spec: joka tavun mankeloi vähitenmerkitsevä bitti
 * - spec: laske muutetut bitit ja tulosta
 * + spec: kuvatiedostot komentoriviltä
 * - käytä PPM:ää (Wikipedia: Portable Pixmap)
 * - tarvittaessa imagemagick-wrapper voi PNG purkaa/pakata
 *
 * PPM: http://netpbm.sourceforge.net/doc/ppm.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>

#define TRUE 1
#define FALSE 0

const char usage[] = "USAGE: ./test -f <file>";

int main(int argc, char* argv[]) {
	int c, fd;
	char *addr;
	struct stat sb;
	off_t offset, page_aligned;
	size_t length;
	ssize_t done;

	if ((c = getopt(argc, argv, "f:")) != 'f')
		errx(1, "%s", usage);

	if (argv[2] == '\0')
		errx(1, "%s", usage);

	if ((fd = open(argv[2], O_RDONLY)) == -1)
		err(1, "%s", argv[2]);

	if (fstat(fd, &sb) == -1)
		err(1, "%s", "fstat()");

	offset = 15;
	page_aligned = offset & ~(sysconf(_SC_PAGESIZE) -1);

	printf("offset :: %i\n", (int)offset);
	printf("aligned : %i\n", (int)page_aligned);
	printf("pagesize: %li\n", sysconf(_SC_PAGESIZE));

	length = sb.st_size - offset;

	printf("length :: %i\n", (int)length);

	if (offset >= sb.st_size) {
		errx(1, "%s", "Offset beyond file border");
	}

	addr = mmap(NULL, length+offset-page_aligned, PROT_READ, MAP_SHARED, fd, page_aligned);

	if (addr == MAP_FAILED)
		err(1, "%s", "mmap");

	done = write(STDOUT_FILENO, addr+offset-page_aligned, length);

	if (done == -1)
		warn("write()");

	if (done != length)
		warn("Partial write");

	printf("\\o/\n");

	return EXIT_SUCCESS;
}
