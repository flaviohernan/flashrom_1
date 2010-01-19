/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2008 Claus Gindhart <claus.gindhart@kontron.com>
 * Copyright (C) 2009 Sean Nelson <audiohacked@gmail.com>
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

/*
 * This module is designed for supporting the devices
 * ST M50FLW040A (not yet tested)
 * ST M50FLW040B (not yet tested)
 * ST M50FLW080A
 * ST M50FLW080B (not yet tested)
 */

#include <string.h>
#include <stdlib.h>
#include "flash.h"
#include "flashchips.h"

static void wait_stm50flw0x0x(chipaddr bios)
{
	chip_writeb(0x70, bios);
	if ((chip_readb(bios) & 0x80) == 0) {	// it's busy
		while ((chip_readb(bios) & 0x80) == 0) ;
	}

	// put another command to get out of status register mode

	chip_writeb(0x90, bios);
	programmer_delay(10);

	chip_readb(bios); // Read device ID (to make sure?)

	// this is needed to jam it out of "read id" mode
	chip_writeb(0xAA, bios + 0x5555);
	chip_writeb(0x55, bios + 0x2AAA);
	chip_writeb(0xF0, bios + 0x5555);
}

/*
 * claus.gindhart@kontron.com
 * The ST M50FLW080B and STM50FLW080B chips have to be unlocked,
 * before you can erase them or write to them.
 */
int unlock_block_stm50flw0x0x(struct flashchip *flash, int offset)
{
	chipaddr wrprotect = flash->virtual_registers + 2;
	const uint8_t unlock_sector = 0x00;
	int j;

	/*
	 * These chips have to be unlocked before you can erase them or write
	 * to them. The size of the locking sectors depends on the type
	 * of chip.
	 *
	 * Sometimes, the BIOS does this for you; so you propably
	 * don't need to worry about that.
	 */

	/* Check, if it's is a top/bottom-block with 4k-sectors. */
	/* TODO: What about the other types? */
	if ((offset == 0) ||
	    (offset == (flash->model_id == ST_M50FLW080A ? 0xE0000 : 0x10000))
	    || (offset == 0xF0000)) {

		// unlock each 4k-sector
		for (j = 0; j < 0x10000; j += 0x1000) {
			printf_debug("unlocking at 0x%x\n", offset + j);
			chip_writeb(unlock_sector, wrprotect + offset + j);
			if (chip_readb(wrprotect + offset + j) != unlock_sector) {
				printf("Cannot unlock sector @ 0x%x\n",
				       offset + j);
				return -1;
			}
		}
	} else {
		printf_debug("unlocking at 0x%x\n", offset);
		chip_writeb(unlock_sector, wrprotect + offset);
		if (chip_readb(wrprotect + offset) != unlock_sector) {
			printf("Cannot unlock sector @ 0x%x\n", offset);
			return -1;
		}
	}

	return 0;
}

int erase_block_stm50flw0x0x(struct flashchip *flash, unsigned int block, unsigned int blocksize)
{
	chipaddr bios = flash->virtual_memory + block;

	// clear status register
	chip_writeb(0x50, bios);
	printf_debug("Erase at 0x%lx\n", bios);
	// now start it
	chip_writeb(0x20, bios);
	chip_writeb(0xd0, bios);
	programmer_delay(10);

	wait_stm50flw0x0x(flash->virtual_memory);

	if (check_erased_range(flash, block, blocksize)) {
		fprintf(stderr, "ERASE FAILED!\n");
		return -1;
	}
	printf("DONE BLOCK 0x%x\n", block);

	return 0;
}

int erase_sector_stm50flw0x0x(struct flashchip *flash, unsigned int sector, unsigned int sectorsize)
{
	chipaddr bios = flash->virtual_memory + sector;

	// clear status register
	chip_writeb(0x50, bios);
	printf_debug("Erase at 0x%lx\n", bios);
	// now start it
	chip_writeb(0x32, bios);
	chip_writeb(0xd0, bios);
	programmer_delay(10);

	wait_stm50flw0x0x(flash->virtual_memory);

	if (check_erased_range(flash, sector, sectorsize)) {
		fprintf(stderr, "ERASE FAILED!\n");
		return -1;
	}
	printf("DONE BLOCK 0x%x\n", sector);

	return 0;
}

int write_page_stm50flw0x0x(chipaddr bios, uint8_t *src,
			    chipaddr dst, int page_size)
{
	int i, rc = 0;
	chipaddr d = dst;
	uint8_t *s = src;

	/* transfer data from source to destination */
	for (i = 0; i < page_size; i++) {
		chip_writeb(0x40, dst);
		chip_writeb(*src++, dst++);
		wait_stm50flw0x0x(bios);
	}

/* claus.gindhart@kontron.com
 * TODO
 * I think, that verification is not required, but
 * i leave it in anyway
 */
	dst = d;
	src = s;
	for (i = 0; i < page_size; i++) {
		if (chip_readb(dst) != *src) {
			rc = -1;
			break;
		}
		dst++;
		src++;
	}

	if (rc) {
		fprintf(stderr, " page 0x%lx failed!\n",
			(d - bios) / page_size);
	}

	return rc;
}

/* I simply erase block by block
 * I Chip This is not the fastest way, but it works
 */
int erase_stm50flw0x0x(struct flashchip *flash)
{
	int i;
	int total_size = flash->total_size * 1024;
	int page_size = flash->page_size;

	printf("Erasing page:\n");
	for (i = 0; i < total_size / page_size; i++) {
		printf
		    ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		printf("%04d at address: 0x%08x ", i, i * page_size);
		if (unlock_block_stm50flw0x0x(flash, i * page_size)) {
			fprintf(stderr, "UNLOCK FAILED!\n");
			return -1;
		}
		if (erase_block_stm50flw0x0x(flash, i * page_size, page_size)) {
			fprintf(stderr, "ERASE FAILED!\n");
			return -1;
		}
	}
	printf("\n");

	return 0;
}

int erase_chip_stm50flw0x0x(struct flashchip *flash, unsigned int addr, unsigned int blocklen)
{
	if ((addr != 0) || (blocklen != flash->total_size * 1024)) {
		msg_cerr("%s called with incorrect arguments\n",
			__func__);
		return -1;
	}
	return erase_stm50flw0x0x(flash);
}

int write_stm50flw0x0x(struct flashchip *flash, uint8_t * buf)
{
	int i, rc = 0;
	int total_size = flash->total_size * 1024;
	int page_size = flash->page_size;
	chipaddr bios = flash->virtual_memory;
	uint8_t *tmpbuf = malloc(page_size);

	if (!tmpbuf) {
		printf("Could not allocate memory!\n");
		exit(1);
	}
	printf("Programming page: \n");
	for (i = 0; (i < total_size / page_size) && (rc == 0); i++) {
		printf
		    ("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		printf("%04d at address: 0x%08x ", i, i * page_size);

		/* Auto Skip Blocks, which already contain the desired data
		 * Faster, because we only write, what has changed
		 * More secure, because blocks, which are excluded
		 * (with the exclude or layout feature)
		 * are not erased and rewritten; data is retained also
		 * in sudden power off situations
		 */
		chip_readn(tmpbuf, bios + i * page_size, page_size);
		if (!memcmp((void *)(buf + i * page_size), tmpbuf, page_size)) {
			printf("SKIPPED\n");
			continue;
		}

		rc = unlock_block_stm50flw0x0x(flash, i * page_size);
		if (!rc)
			rc = erase_block_stm50flw0x0x(flash, i * page_size, page_size);
		if (!rc)
			write_page_stm50flw0x0x(bios, buf + i * page_size,
					bios + i * page_size, page_size);
	}
	printf("\n");
	free(tmpbuf);

	return rc;
}
