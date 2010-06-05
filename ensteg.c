/* ensteg.c
 * Steganography encoder
 * Copyright 2010 Tuomas Starck
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * FIXME TODO: spec: ensteg && desteg
 */

#include "common.h"

/* write_msg(mmap pointer, img resolution, ppm maxval):
 *
 * Write message to mmapped image. Message is read
 * from standard input.
 *
 * Returns the number of bits written (including the
 * eight null bits used to terminate message).
 */
int write_msg(char *addr, int res, int fit) {
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
