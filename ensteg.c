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
const char syntaxfail[] = "error: PPM syntax fail :-(";

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

	printf("skip_ads(): i: %u\n", i);

	if ((i = memval(addr, i, "\x0a#", 2)) != -1) {
		debug("skip_ads: eliminating comment");
		while (!memchar(addr+i, "\x0a")) i++;
		val = no_comments(addr, i);
	}
	else {
		debug("skip_ads: no comment");
	}

	return val;
}

unsigned int ws_tool(char *addr, unsigned int i) {
	int chr, whitespace;

	printf("whitespace(): i: %u\n", i);

	do {
		chr = (int)*(addr+i);

		printf("whitespace: chr: %i\n", chr);

		switch (chr) {
		case 0x0a:
			i = no_comments(addr, i);
			printf("whitespace(): i: %u\n", i);
		case 0x09:
		case 0x0d:
		case 0x20:
			i++;
			whitespace = true;
			break;
		default:
			whitespace = false;
		}
	} while (whitespace);

	printf("whitespace(%u)\n", i);

	return i;
}

unsigned int req_skip_ws(char *addr, unsigned int i) {
	unsigned int ret = i;

	if ((ret = ws_tool(addr, i)) == i)
		errx(EXIT_FAILURE, syntaxfail);

	return ret;
}

int funktio(char *addr) {
	unsigned int i;
	int width, height, scale, fit;

	i = 0;

	/* Spec 1 */
	if ((i = memval(addr, i, "P6", 2)) == -1)
		errx(EXIT_FAILURE, headerfail);

	/* Spec 2 */
	i = req_skip_ws(addr, i);

	/* Spec 3 */
	if (sscanf(addr+i, "%i", &width) != 1)
		err(EXIT_FAILURE, "width");

	printf("debug :: i(0)     : %u\n", i);
	printf("debug :: width(0) : %i\n", width);

	/* Spec 4 */
	i = req_skip_ws(addr, i);

	/* Spec 5 */
	if (sscanf(addr+i, "%i", &height) != 1)
		err(EXIT_FAILURE, "height");

	/* Spec 6 */
	i = req_skip_ws(addr, i);

	/* Spec 7 */
	if (sscanf(addr+i, "%i", &scale) != 1)
		err(EXIT_FAILURE, "scale");

	if (scale <= 0)
		errx(EXIT_FAILURE, "minval");
	else if (scale < 256)
		fit = true;
	else if (scale < 65536)
		fit = false;
	else
		errx(EXIT_FAILURE, "maxval");

	printf("debug :: i     : %u\n", i);
	printf("debug :: width : %i\n", width);
	printf("debug :: height: %i\n", height);
	printf("debug :: scale : %i\n", scale);

	/* width = readdec();
	 * tmp = sscanf(addr+i, "%i", &width);
	 * seuraavaksi lue: width (in ascii dec)
	 * int sscanf(const char *str, const char *format, ...);
	 */

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

	if (addr == MAP_FAILED)
		err(EXIT_FAILURE, "%s", "mmap()");

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
