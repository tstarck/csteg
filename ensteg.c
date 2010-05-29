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
#include <stdbool.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>

const char usage[] = "USAGE: ./test -f <file>";
const char zerosize[] = "error: Zero file size";
const char headerfail[] = "error: Invalid file header (PPM format required)";

void debug(const char *msg) {
	printf("debug :: %s\n", msg);
}

int memval(char *addr, unsigned int i, char *cmp, int n) {
	if (memcmp(addr+i, cmp, n) != 0) return -1;
	return i+n;
}

int memchar(char *addr, char *cmp) {
	if (memcmp(addr, cmp, 1) == 0) return true;
	return false;
}

int something(char *addr) {
	unsigned int i = 0;

	if ((i = memval(addr, i, "P6", 2)) == -1)
		errx(EXIT_FAILURE, headerfail);

	i = skip_ads(addr, i);
}

int skip_ads(char *addr, unsigned int i) {
	unsigned int val = i;

	if ((i = memval(addr, i, "0x0a#", 2)) != -1) {
		debug("kommentti löytys");
		while (!memchar(addr+i, '0x0a')) i++;
		val = skip_ads(addr, i);
	}

	return val;
}

int main(int argc, char* argv[]) {
	int c, fd;
	char *addr;
	struct stat sb;
	size_t flen;

	if ((c = getopt(argc, argv, "f:")) != 'f')
		errx(EXIT_FAILURE, "%s", usage);

	if (argv[2] == '\0')
		errx(EXIT_FAILURE, "%s", usage);

	if ((fd = open(argv[2], O_RDWR)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);

	if (fstat(fd, &sb) == -1)
		err(EXIT_FAILURE, "fstat()");

	if ((flen = sb.st_size) == 0)
		errx(EXIT_FAILURE, zerosize);

	printf("debug :: flength  :: %i\n", (int)flen);
	printf("debug :: pagesize :: %li\n", sysconf(_SC_PAGESIZE));

	addr = mmap(NULL, flen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (addr == MAP_FAILED)
		err(EXIT_FAILURE, "%s", "mmap()");

	if (something(addr))
		debug("something :: yes");
	else
		debug("something :: nou");

	if (munmap(addr, flen) == -1)
		err(EXIT_FAILURE, "munmap()");

	if (close(fd) == -1)
		err(EXIT_FAILURE, "close()");

	return EXIT_SUCCESS;
}
