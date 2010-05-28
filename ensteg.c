/* ensteg.c
 * Encode secret message using steganography
 * Copyright 2010 Tuomas Starck
 *
 * TODO:
 * - spec: ensteg && desteg
 * - spec: joka tavun mankeloi vähitenmerkitsevä bitti
 * - spec: laske muutetut bitit ja tulosta
 * + spec: kuvatiedostot komentoriviltä
 * - käytä PPM:ää (Wikipedia: Portable Pixmap)
 * - tarvittaessa imagemagick-wrapper voi PNG purkaa/pakata
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define TRUE 1
#define FALSE 0

const char usage[] = "usage: ./test -f <file>\n";

int main(int argc, char* argv[]) {
	int c;

	if ((c = getopt(argc, argv, "f:")) != 'f') {
		printf(usage);
		return EXIT_FAILURE;
	}
	if (argv[2] == '\0') {
		printf(usage);
		return EXIT_FAILURE;
	}

	printf("Tiedosto :: %s\n", argv[2]);

	return EXIT_SUCCESS;
}
