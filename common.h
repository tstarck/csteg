/* common.h
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

#ifndef COMMON_H
#define COMMON_H

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

/* Syntactic sugar for the loops.
 */
#define EVER (;;)

/* Flip the least significant bit
 * on or off for any given byte.
 */
#define BIT_ON(x)  (x |= 0x01)
#define BIT_OFF(x) (x &= 0xfe)

/* Error messages
 */
const char usage[]      = "USAGE: ./test -f <file>";
const char zerosize[]   = "error: Zero file size";
const char headerfail[] = "error: Invalid file header (PPM format required)";
const char syntaxfail[] = "error: PPM syntax fail :-(";
const char somefail[]   = "error: Could not read ascii decimal";

/* Prototypes
 */
int write_msg(char *addr, int res, int fit);

/* no_comments(mmap pointer, mmap index):
 *
 * Skip comments (if there are any) by incrementing
 * index to the byte following the comment.
 *
 * Returns the new index after comments.
 */
unsigned int no_comments(char *addr, unsigned int i) {
	while (memcmp(addr+i, "#", 1) == 0) {
		i++;
		while (addr[i] != '\n') i++;
		i++;
	}

	return i;
}

/* req_skip_ws(mmap pointer, mmap index):
 *
 * Require and skip all whitespace characters by incrementing
 * index to the first byte following whitespace. If there is
 * no whitespace characters to skip, program execution is
 * terminated with an error.
 *
 * Returns the new index after whitespace.
 */
unsigned int req_skip_ws(char *addr, unsigned int i) {
	int loop = true;
	unsigned int start = i;

	while (loop) {
		switch (addr[i]) {
			case 0x0a:
				i = no_comments(addr, (i+1));
				break;

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

/* read_dec(mmap pointer, mmap index, int pointer):
 *
 * Read ascii decimal number at index and store it to pointer.
 * Also forward index at the first byte following read number.
 *
 * If no number can be read, terminate program with an error.
 *
 * Returns the new index after number.
 */
unsigned int read_dec(char *addr, unsigned int i, int *ptr) {
	if (sscanf(addr+i, "%i", ptr) != 1)
		errx(EXIT_FAILURE, somefail);

	while ('0' <= addr[i] && addr[i] <= '9') i++;

	return i;
}

/* parse_ppm(mmap pointer):
 *
 * Parse given mmapped file assuming it is a PPM P6 formatted
 * image. If expected headers cannot be read, program will
 * terminate with an error.
 *
 * Conforming to: http://netpbm.sourceforge.net/doc/ppm.html
 *
 * Returns void.
 */
void parse_ppm(char *addr) {
	unsigned int i;
	int width, height, scale, fit, done;

	i = 0;

	/* Specification step 1 */
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

	printf("Pliplap viilattiin %i bittiä.\n", done); /* FIXME */

	return;
}

#endif
