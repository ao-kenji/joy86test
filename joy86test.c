/*
 * Copyright (c) 2018 Kenji Aoyama.
 *
 * Permission to use, copy, modify, and distribute this software for any
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

/*
 * PC-9801 extension board bus test program
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>	/* getprogname(3) */
#include <unistd.h>	/* getopt(3) */

#include "opna.h"

void	usage(void);

int
main(int argc, char **argv)
{
	int i, data;

	/*
	 * parse options
	 */
	int ch;
	extern char *optarg;
	extern int optind, opterr;

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':	/* debug flag */
			opna_set_debug_level(1);
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0) {
		usage();
		return 1;
	}

	if (opna_open() == -1)
		return 1;

	opna_init();

	printf("start...\n");

	for (i = 0; i < 256; i++) {
		data = opna_get_joystick();
		if ((data & 0x01) == 0)
			printf("Up     \n");
		else if ((data & 0x02) == 0)
			printf("Down   \n");
		else if ((data & 0x04) == 0)
			printf("Left   \n");
		else if ((data & 0x08) == 0)
			printf("Right  \n");
		else if ((data & 0x10) == 0)
			printf("Trigger\n");
		else
			printf("       \n");
#if 0
		printf("%04d 0x%x\n",i,  data);
#endif
	}

	opna_close();

	return 0;
}

/*
 * Usage
 */
void
usage(void)
{
	printf("Usage: %s [options]\n", getprogname());
	printf("\t-d        : debug flag\n");
	exit(1);
}
