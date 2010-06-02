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

#define EVER (;;)

const char usage[]      = "USAGE: ./test -f <file>";
const char zerosize[]   = "error: Zero file size";
const char headerfail[] = "error: Invalid file header (PPM format required)";
const char syntaxfail[] = "error: PPM syntax fail :-(";
const char somefail[]   = "error: Could not read ascii decimal";

void debug(const char *msg) {
#ifdef DEBUG
	printf("debug :: %s\n", msg);
#endif
}

int memchar(char *addr, char *cmp) {
	if (memcmp(addr, cmp, 1) == 0) return true;
	return false;
}

unsigned int memval(char *addr, unsigned int i, char *cmp, int n) {
	if (memcmp(addr+i, cmp, n) != 0) return -1;
	return i+n;
}

unsigned int no_comments(char *addr, unsigned int i) {
	unsigned int val = i;

	if ((i = memval(addr, i, "\n#", 2)) != -1) {
		/* FIXME: parempi? while (addr[i] != '\n') i++; */
		debug("skip_ads: eliminating comment");
		while (!memchar(addr+i, "\n")) i++;
		val = no_comments(addr, i);
	}
	else {
		debug("skip_ads: no comment");
	}

	return val;
}

unsigned int ws_tool(char *addr, unsigned int i) {
	int ws;

	do {
		switch (addr[i]) {
			case 0x0a:
				i = no_comments(addr, i);
			case 0x09:
			case 0x0d:
			case 0x20:
				i++;
				ws = true;
				break;
			default:
				ws = false;
		}
	} while (ws);

	return i;
}

unsigned int req_skip_ws(char *addr, unsigned int i) {
	/* FIXME: tää kannattais mergeä ws_toolin kanssa */

	unsigned int ret = i;

	if ((ret = ws_tool(addr, i)) == i)
		errx(EXIT_FAILURE, syntaxfail);

	return ret;
}

unsigned int read_dec(char *addr, unsigned int i, int *ptr) {
	char chr;

	if (sscanf(addr+i, "%i", ptr) != 1)
		errx(EXIT_FAILURE, somefail);

	for EVER { /* FIXME: tän voi kai tehdä paremmin */
		chr = (char)*(addr+i);
		if (chr < '0' || '9' < chr)
			break;
		i++;
	}

	/* kuten näin :: while ('0' <= addr[i] && addr[i] <= '9') i++; */

	return i;
}

int just_do_it(char *addr, int res, int fit) {
	int input;
	int bit = 0x100;
	unsigned int i = 0;

	/* TODO:
	 * pitäis tarkistaa kuvadatan riittävyys
	 * eli että addr[i] pysyy jossain rajoissa
	 */

	/* debug */ printf("just_do_it(): addr[0] = %i, res = %i, fit = %i\n",
		(unsigned int)addr[0], res, fit);

	for EVER {
		if (bit > 0x80) {
			if ((input = getchar()) == EOF)
				break;
			bit = 1;
		}

		if (input & bit) {
			addr[i] |= 0x01; /* 0000 0001 */
		}
		else {
			addr[i] &= 0xfe; /* 1111 1110 */
		}

		bit <<= 1;
		i++;
	}

	for (bit = 1; bit <= 0x80; bit <<= 1) {
		addr[i] &= 0xfe;
		i++;
	}

	return i;
}

int funktio(char *addr) {
	unsigned int i;
	int width, height, scale, fit;
	int tmp;

	i = 0;

	/* Spec step 1 */
	if ((i = memval(addr, i, "P6", 2)) == -1)
		errx(EXIT_FAILURE, headerfail);

	/* Spec step 2 */
	i = req_skip_ws(addr, i);

	/* Spec step 3 */
	i = read_dec(addr, i, &width);

	/* Spec step 4 */
	i = req_skip_ws(addr, i);

	/* Spec step 5 */
	i = read_dec(addr, i, &height);

	/* Spec step 6 */
	i = req_skip_ws(addr, i);

	/* Spec step 7 */
	i = read_dec(addr, i, &scale);

	if      (scale <= 0)    errx(EXIT_FAILURE, "minval");
	else if (scale < 256)   fit = true;
	else if (scale < 65536) fit = false;
	else                    errx(EXIT_FAILURE, "maxval");

	printf("debug :: i     : %u\n", i);
	printf("debug :: width : %i\n", width);
	printf("debug :: height: %i\n", height);
	printf("debug :: scale : %i\n", scale);
	printf("debug :: fit   : %i\n", fit);

	/* Spec step 8 */
	if (memchar(addr+i, "\n")) i++;
	else errx(EXIT_FAILURE, "Newline required");

	printf("header done @(%i)\n", i);

	/* Spec step 9 */
	tmp = just_do_it(addr+i, width*height, fit);

	printf("just_do_it(%i)\n", tmp);
	printf("funktio(%i)\n", i);

	return i;
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

	printf("debug :: flength  :: %i\n", (int)flen);
	printf("debug :: pagesize :: %li\n", sysconf(_SC_PAGESIZE));

	addr = mmap(NULL, flen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (addr == MAP_FAILED) {
		close(fd);
		err(EXIT_FAILURE, "%s", "mmap()");
	}

	if (funktio(addr))
		debug("funktio(): true");
	else
		debug("funktio(): false");

	if (munmap(addr, flen) == -1)
		err(EXIT_FAILURE, "munmap()");

	if (close(fd) == -1)
		err(EXIT_FAILURE, "close()");

	return EXIT_SUCCESS;
}
