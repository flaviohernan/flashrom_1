/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2000 Silicon Integrated System Corporation
 * Copyright (C) 2005-2007 coresystems GmbH
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

#include "flash.h"
#include "chipdrivers.h"

static int write_lockbits_block_49lfxxxc(struct flashchip *flash, unsigned long address, unsigned char bits)
{
	unsigned long lock = flash->virtual_registers + address + 2;
	msg_cdbg("lockbits at address=0x%08lx is 0x%01x\n", lock, chip_readb(lock));
	chip_writeb(bits, lock);

	return 0;
}

static int write_lockbits_49lfxxxc(struct flashchip *flash, unsigned char bits)
{
	chipaddr registers = flash->virtual_registers;
	int i, left = flash->total_size * 1024;
	unsigned long address;

	msg_cdbg("\nbios=0x%08lx\n", registers);
	for (i = 0; left > 65536; i++, left -= 65536) {
		write_lockbits_block_49lfxxxc(flash, i * 65536, bits);
	}
	address = i * 65536;
	write_lockbits_block_49lfxxxc(flash, address, bits);
	address += 32768;
	write_lockbits_block_49lfxxxc(flash, address, bits);
	address += 8192;
	write_lockbits_block_49lfxxxc(flash, address, bits);
	address += 8192;
	write_lockbits_block_49lfxxxc(flash, address, bits);

	return 0;
}

int unlock_49lfxxxc(struct flashchip *flash)
{
	return write_lockbits_49lfxxxc(flash, 0);
}

int erase_sector_49lfxxxc(struct flashchip *flash, unsigned int address, unsigned int sector_size)
{
	uint8_t status;
	chipaddr bios = flash->virtual_memory;

	chip_writeb(0x30, bios);
	chip_writeb(0xD0, bios + address);

	status = wait_82802ab(bios);

	if (check_erased_range(flash, address, sector_size)) {
		msg_cerr("ERASE FAILED!\n");
		return -1;
	}
	return 0;
}

int write_49lfxxxc(struct flashchip *flash, uint8_t *buf)
{
	int i;
	int total_size = flash->total_size * 1024;
	int page_size = flash->page_size;
	chipaddr bios = flash->virtual_memory;

	write_lockbits_49lfxxxc(flash, 0);
	msg_cinfo("Programming page: ");
	for (i = 0; i < total_size / page_size; i++) {
		/* erase the page before programming */
		if (erase_sector_49lfxxxc(flash, i * page_size, flash->page_size)) {
			msg_cerr("ERASE FAILED!\n");
			return -1;
		}

		/* write to the sector */
		msg_cinfo("%04d at address: 0x%08x", i, i * page_size);
		write_page_82802ab(bios, buf + i * page_size,
				      bios + i * page_size, page_size);
		msg_cinfo("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
	msg_cinfo("\n");

	chip_writeb(0xFF, bios);

	return 0;
}