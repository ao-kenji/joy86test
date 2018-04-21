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
 * handle PC-9801-86 joystick port via YM2608
 */

#include <fcntl.h>
#include <stdio.h>
#include <time.h>	/* nanosleep(2) */
#include <sys/mman.h>	/* mmap(2) */
#include <machine/pcex.h>

#include "opna.h"

#define DPRINTF(x)      if (opna_debug) printf x

/* global */
int	opna_debug = 0;
int	pcexio_fd;
u_int8_t *pcexio_base;
u_int8_t *opna_bi_reg;	/* YM2608 Basic Index register */
u_int8_t *opna_bd_reg;	/* YM2608 Basic Data register */
u_int8_t *opna_ei_reg;	/* YM2608 Extended Index register */
u_int8_t *opna_ed_reg;	/* YM2608 Extended Data register */

/* prototypes (internal use) */
u_int8_t	opna_read(u_int8_t);
void		opna_write(u_int8_t, u_int8_t);

void
opna_init(void)
{
	u_int8_t data;

	/* enable YM2608(OPNA) mode (default is YM2203(OPN) mode) */
	data = opna_read(0x29);
	data |= 0x80;
	opna_write(0x29, data);

	/* set IOA port to input, IOB port to output mode */
	data = opna_read(0x07);
	data &= 0xbf;		/* set bit6 to 0 */ 
	data |= 0x80;		/* set bit7 to 1 */ 
	opna_write(0x07, data);

	/* set IOB port bit 7 to 1 */
	data = opna_read(0x0f);
	data &= 0x00;	/* clear */
	data |= 0x80;	/* set bit 7 */
	opna_write(0x0f, data);
}

/*
 * Open
 */
int
opna_open(void)
{
	u_int8_t *sound_id, data;

	pcexio_fd = open("/dev/pcexio", O_RDWR, 0600);
	if (pcexio_fd == -1) {
		perror("open");
		goto exit1;
	}

	pcexio_base = (u_int8_t *)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE,
		MAP_SHARED, pcexio_fd, 0x0);

	if (pcexio_base == MAP_FAILED) {
		perror("mmap");
		goto exit2;
	}

#if 0	/* WSN-A[24]F does not support joystick port */
	/* if we find WSN-A[24]F, set it up */
	data = *(pcexio_base + 0x51e3);
	if (data == 0xc2)
		*(pcexio_base + 0x57e3) = 0xf0;	/* sound ID = 0x40 */
#endif

	/* check the existence of PC-9801-86 board */
	sound_id = pcexio_base + 0xa460;
	data = *sound_id;
	if ((data & 0xf0) != 0x40) {
		fprintf(stderr, "can not found PC-9801-86 board\n");
		goto exit2;
	}

        /* enable YM2608 on PC-9801-86 board */
        *sound_id = ((data & 0xfc) | 0x01);

	opna_bi_reg = pcexio_base + 0x188;	
	opna_bd_reg = pcexio_base + 0x18a;	
	opna_ei_reg = pcexio_base + 0x18c;	
	opna_ed_reg = pcexio_base + 0x18e;	

	return pcexio_fd;
exit2:
	close(pcexio_fd);
exit1:
	return -1;
}

/*
 * Close
 */
void
opna_close(void)
{
	munmap((void *)pcexio_base, 0x10000);
	close(pcexio_fd);
}

/*
 * OPNA needs wait cycles
 *  after address write: 17 cycles @ 8MHz = 2.125 us 
 *  after data write   : 83 cycles @ 8MHz = 10.375 us
 */
struct timespec opna_wait_waddr = {
	.tv_sec = 0,
	.tv_nsec = 2125
};

struct timespec opna_wait_wdata_long = {
	.tv_sec = 0,
	.tv_nsec = 10375
};

struct timespec opna_wait_wdata_short = {
	.tv_sec = 0,
	.tv_nsec = 5875
};

/*
 * Register read/write
 */
u_int8_t
opna_read(u_int8_t index)
{
	static u_int8_t pre_index = 0xff;
	u_int8_t ret;

	*opna_bi_reg = index;
	if (index == pre_index)
		nanosleep(&opna_wait_waddr, NULL);
	pre_index = index;
	ret = *opna_bd_reg;

	return ret;
}

void
opna_write(u_int8_t index, u_int8_t data)
{
	static u_int8_t pre_index = 0xff;

	*opna_bi_reg = index;
	if (index == pre_index)
		nanosleep(&opna_wait_waddr, NULL);
	pre_index = index;
	*opna_bd_reg = data;
	if (index < 0xa0)
		nanosleep(&opna_wait_wdata_long, NULL);
	else
		nanosleep(&opna_wait_wdata_short, NULL);
}

int
opna_get_joystick(void)
{
	return (int)opna_read(0x0e);
}

void
opna_set_debug_level(int level)
{
	opna_debug = level;
}

int
opna_wait_ms(int ms)
{
	struct timespec opna_wait_ns;

	if ((ms < 0) || (ms > 100000)) {
		printf("ms %d: out of range\n", ms);
		return -1;
	}

	opna_wait_ns.tv_sec = ms / 1000;
	opna_wait_ns.tv_nsec = (ms % 1000) * 1000 * 1000;

	nanosleep(&opna_wait_ns, NULL);
}
