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
 */

#include "common.h"

const char *report = "Flipped %u bits in %u bytes.\n";

/* flip_on(mmap pointer)
 * flip_off(mmap pointer):
 *
 * Turn the least significant bit on or off in a given byte.
 *
 * Return the number of bits flipped (which is 1 at most).
 */
int flip_on(char *addr) {
	if ((addr[0] & 1) == 0) {
		addr[0] |= 0x01;
		return 1;
	}
	return 0;
}
int flip_off(char *addr) {
	if ((addr[0] & 1) == 1) {
		addr[0] &= 0xfe;
		return 1;
	}
	return 0;
}

/* write_msg(mmap pointer, img length):
 *
 * Write message to mmapped image. Message is read from
 * standard input.
 *
 * Returns void.
 */
void write_msg(char *addr, unsigned int pix) {
	int in;
	int bit = 0x81;
	unsigned int i = 0, alt = 0;

	while (true) {
		if (bit > 0x80) {
			bit = 1;

			/* There MUST be enough space for the  */
			/* next char AND terminating null byte */
			if ((pix-16) < i) break;

			if ((in = getchar()) == EOF) break;
		}

		if (in & bit)
			alt += flip_on(addr+i);
		else
			alt += flip_off(addr+i);

		bit <<= 1;
		i++;
	}

	/* Finalize writing with null byte */
	for (bit = 1; bit <= 0x80; bit <<= 1) {
		alt += flip_off(addr+i);
		i++;
	}

	printf(report, alt, i);

	return;
}

int main(int argc, char* argv[]) {
	int fd;
	char *addr;
	struct stat sb;
	size_t flen;
	unsigned int i, pix;

	if (getopt(argc, argv, "f:") != 'f')
		errx(EXIT_FAILURE, USAGE);

	if (argv[2] == '\0')
		errx(EXIT_FAILURE, USAGE);

	if ((fd = open(argv[2], O_RDWR)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);

	if (fstat(fd, &sb) == -1)
		err(EXIT_FAILURE, "%s", "fstat(2)");

	if ((flen = sb.st_size) == 0)
		errx(EXIT_FAILURE, EZEROSIZE);

	addr = mmap(NULL, flen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (addr == MAP_FAILED) {
		close(fd);
		err(EXIT_FAILURE, "%s", "mmap(2)");
	}

	i = parse_ppm(addr, &pix);

	write_msg(addr+i, pix);

	if (munmap(addr, flen) == -1)
		err(EXIT_FAILURE, "%s", "munmap(2)");

	if (close(fd) == -1)
		err(EXIT_FAILURE, "%s", "close(2)");

	return EXIT_SUCCESS;
}
