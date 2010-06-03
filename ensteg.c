/* ensteg.c
 * Steganography encoder
 * Copyright 2010 Tuomas Starck
 *
 * TODO:
 * - spec: ensteg && desteg
 * + spec: joka tavun mankeloi vähitenmerkitsevä bitti
 * - spec: laske muutetut bitit ja tulosta
 * + spec: kuvatiedostot komentoriviltä
 * + käytä PPM:ää (Wikipedia: Portable Pixmap)
 * + tarvittaessa imagemagick-wrapper voi PNG purkaa/pakata
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

#define EVER       (;;)
#define BIT_ON(x)  (x |= 0x01)
#define BIT_OFF(x) (x &= 0xfe)

const char usage[]      = "USAGE: ./test -f <file>";
const char zerosize[]   = "error: Zero file size";
const char headerfail[] = "error: Invalid file header (PPM format required)";
const char syntaxfail[] = "error: PPM syntax fail :-(";
const char somefail[]   = "error: Could not read ascii decimal";

unsigned int no_comments(char *addr, unsigned int i) {
	unsigned int val = i;

	if (memcmp(addr+i, "\n#", 2) == 0) {
		i += 2;
		while (addr[i] != '\n') i++;
		val = no_comments(addr, i);
	}

	return val;
}

unsigned int req_skip_ws(char *addr, unsigned int i) {
	int loop = true;
	unsigned int start = i;

	while (loop) {
		switch (addr[i]) {
			case 0x0a:
				i = no_comments(addr, i);

			case 0x09: case 0x0d: case 0x20:
				i++;
				break;

			default:
				loop = false;
				break;
		}
	}

	if (start == i)
		errx(EXIT_FAILURE, syntaxfail);

	return i;
}

unsigned int read_dec(char *addr, unsigned int i, int *ptr) {
	if (sscanf(addr+i, "%i", ptr) != 1)
		errx(EXIT_FAILURE, somefail);

	while ('0' <= addr[i] && addr[i] <= '9') i++;

	return i;
}

int write_msg(char *addr, int res, int fit) { /* FIXME: nimi */
	int input;
	int bit = 0x100;
	unsigned int i = 0;

	/* TODO:
	 * pitäis tarkistaa kuvadatan riittävyys
	 * eli että addr[i] pysyy jossain rajoissa
	 */

	/* debug */ printf("write_msg(): res = %i, pix = %i\n", res, 3*res*fit);

	for EVER {
		if (bit > 0x80) {
			if ((input = getchar()) == EOF)
				break;
			bit = 1;
		}

		if (input & bit)
			BIT_ON(addr[i]);
		else
			BIT_OFF(addr[i]);

		bit <<= 1;
		i++;
	}

	for (bit = 1; bit <= 0x80; bit <<= 1) {
		BIT_OFF(addr[i]);
		i++;
	}

	return i;
}

void parse_ppm(char *addr) { /* FIXME: nimi */
	unsigned int i;
	int width, height, scale, fit, done;

	i = 0;

	/* http://netpbm.sourceforge.net/doc/ppm.html
	 * Specification step 1
	 */
	if (memcmp(addr+i, "P6", 2) != 0)
		errx(EXIT_FAILURE, headerfail);
	else
		i += 2;

	/* 2 */
	i = req_skip_ws(addr, i);

	/* 3 */
	i = read_dec(addr, i, &width);

	/* 4 */
	i = req_skip_ws(addr, i);

	/* 5 */
	i = read_dec(addr, i, &height);

	/* 6 */
	i = req_skip_ws(addr, i);

	/* 7 */
	i = read_dec(addr, i, &scale);

	if      (scale <= 0)    errx(EXIT_FAILURE, "minval");
	else if (scale < 256)   fit = true;
	else if (scale < 65536) fit = false;
	else                    errx(EXIT_FAILURE, "maxval");

	/* 8 */
	if (memcmp(addr+i, "\n", 1) != 0)
		errx(EXIT_FAILURE, "Newline required");
	else
		i++;

	/* 9 */
	done = write_msg(addr+i, width*height, fit);

	printf("Pliplap viilattiin %i bittiä.\n", done);

	return;
}

int main(int argc, char* argv[]) {
	int fd;
	char *addr;
	struct stat sb;
	size_t flen;

	if (getopt(argc, argv, "f:") != 'f')
		errx(EXIT_FAILURE, usage);

	if (argv[2] == '\0')
		errx(EXIT_FAILURE, usage);

	if ((fd = open(argv[2], O_RDWR)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);

	if (fstat(fd, &sb) == -1)
		err(EXIT_FAILURE, "fstat()");

	if ((flen = sb.st_size) == 0)
		errx(EXIT_FAILURE, zerosize);

	addr = mmap(NULL, flen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (addr == MAP_FAILED) {
		close(fd);
		err(EXIT_FAILURE, "%s", "mmap()");
	}

	parse_ppm(addr);

	if (munmap(addr, flen) == -1)
		err(EXIT_FAILURE, "munmap()");

	if (close(fd) == -1)
		err(EXIT_FAILURE, "close()");

	return EXIT_SUCCESS;
}
