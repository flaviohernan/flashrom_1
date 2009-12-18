/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2000 Silicon Integrated System Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdlib.h>
#include "flash.h"

// I need that Berkeley bit-map printer
void print_lhf00l04_status(uint8_t status)
{
	printf("%s", status & 0x80 ? "Ready:" : "Busy:");
	printf("%s", status & 0x40 ? "BE SUSPEND:" : "BE RUN/FINISH:");
	printf("%s", status & 0x20 ? "BE ERROR:" : "BE OK:");
	printf("%s", status & 0x10 ? "PROG ERR:" : "PROG OK:");
	printf("%s", status & 0x8 ? "VP ERR:" : "VPP OK:");
	printf("%s", status & 0x4 ? "PROG SUSPEND:" : "PROG RUN/FINISH:");
	printf("%s", status & 0x2 ? "WP|TBL#|WP#,ABORT:" : "UNLOCK:");
}

int probe_lhf00l04(struct flashchip *flash)
{
	chipaddr bios = flash->virtual_memory;
	uint8_t id1, id2;

#if 0
	/* Enter ID mode */
	chip_writeb(0xAA, bios + 0x5555);
	chip_writeb(0x55, bios + 0x2AAA);
	chip_writeb(0x90, bios + 0x5555);
#endif

	chip_writeb(0xff, bios);
	programmer_delay(10);
	chip_writeb(0x90, bios);
	programmer_delay(10);

	id1 = chip_readb(bios);
	id2 = chip_readb(bios + 0x01);

	/* Leave ID mode */
	chip_writeb(0xAA, bios + 0x5555);
	chip_writeb(0x55, bios + 0x2AAA);
	chip_writeb(0xF0, bios + 0x5555);

	programmer_delay(10);

	printf_debug("%s: id1 0x%02x, id2 0x%02x\n", __FUNCTION__, id1, id2);

	if (id1 != flash->manufacture_id || id2 != flash->model_id)
		return 0;

	map_flash_registers(flash);

	return 1;
}

uint8_t wait_lhf00l04(chipaddr bios)
{
	uint8_t status;
	uint8_t id1, id2;

	chip_writeb(0x70, bios);
	if ((chip_readb(bios) & 0x80) == 0) {	// it's busy
		while ((chip_readb(bios) & 0x80) == 0) ;
	}

	status = chip_readb(bios);

	// put another command to get out of status register mode

	chip_writeb(0x90, bios);
	programmer_delay(10);

	id1 = chip_readb(bios);
	id2 = chip_readb(bios + 0x01);

	// this is needed to jam it out of "read id" mode
	chip_writeb(0xAA, bios + 0x5555);
	chip_writeb(0x55, bios + 0x2AAA);
	chip_writeb(0xF0, bios + 0x5555);

	return status;
}

int erase_lhf00l04_block(struct flashchip *flash, int offset)
{
	chipaddr bios = flash->virtual_memory + offset;
	chipaddr wrprotect = flash->virtual_registers + offset + 2;
	uint8_t status;

	// clear status register
	chip_writeb(0x50, bios);
	printf("Erase at 0x%lx\n", bios);
	status = wait_lhf00l04(flash->virtual_memory);
	print_lhf00l04_status(status);
	// clear write protect
	printf("write protect is at 0x%lx\n", (wrprotect));
	printf("write protect is 0x%x\n", chip_readb(wrprotect));
	chip_writeb(0, wrprotect);
	printf("write protect is 0x%x\n", chip_readb(wrprotect));

	// now start it
	chip_writeb(0x20, bios);
	chip_writeb(0xd0, bios);
	programmer_delay(10);
	// now let's see what the register is
	status = wait_lhf00l04(flash->virtual_memory);
	print_lhf00l04_status(status);
	printf("DONE BLOCK 0x%x\n", offset);

	if (check_erased_range(flash, offset, flash->page_size)) {
		fprintf(stderr, "ERASE FAILED!\n");
		return -1;
	}
	return 0;
}

int erase_lhf00l04(struct flashchip *flash)
{
	int i;
	unsigned int total_size = flash->total_size * 1024;

	printf("total_size is %d; flash->page_size is %d\n",
	       total_size, flash->page_size);
	for (i = 0; i < total_size; i += flash->page_size)
		if (erase_lhf00l04_block(flash, i)) {
			fprintf(stderr, "ERASE FAILED!\n");
			return -1;
		}
	printf("DONE ERASE\n");

	return 0;
}

void write_page_lhf00l04(chipaddr bios, uint8_t *src,
			 chipaddr dst, int page_size)
{
	int i;

	for (i = 0; i < page_size; i++) {
		/* transfer data from source to destination */
		chip_writeb(0x40, dst);
		chip_writeb(*src++, dst++);
		wait_lhf00l04(bios);
	}
}

int write_lhf00l04(struct flashchip *flash, uint8_t *buf)
{
	int i;
	int total_size = flash->total_size * 1024;
	int page_size = flash->page_size;
	chipaddr bios = flash->virtual_memory;

	if (erase_lhf00l04(flash)) {
		fprintf(stderr, "ERASE FAILED!\n");
		return -1;
	}
	printf("Programming page: ");
	for (i = 0; i < total_size / page_size; i++) {
		printf("%04d at address: 0x%08x", i, i * page_size);
		write_page_lhf00l04(bios, buf + i * page_size,
				    bios + i * page_size, page_size);
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
	printf("\n");
	protect_jedec(bios);

	return 0;
}