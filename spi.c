/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2007, 2008 Carl-Daniel Hailfinger
 * Copyright (C) 2008 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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
 * Contains the generic SPI framework
 */

#include <stdio.h>
#include <pci/pci.h>
#include <stdint.h>
#include <string.h>
#include "flash.h"
#include "spi.h"


void spi_prettyprint_status_register(struct flashchip *flash);

int spi_command(unsigned int writecnt, unsigned int readcnt, const unsigned char *writearr, unsigned char *readarr)
{
	if (it8716f_flashport)
		return it8716f_spi_command(writecnt, readcnt, writearr, readarr);
       else if (ich7_detected)
               return ich_spi_command(writecnt, readcnt, writearr, readarr);
	else if (ich9_detected)
		return ich_spi_command(writecnt, readcnt, writearr, readarr);
	printf_debug("%s called, but no SPI chipset detected\n", __FUNCTION__);
	return 1;
}

static int spi_rdid(unsigned char *readarr)
{
	const unsigned char cmd[JEDEC_RDID_OUTSIZE] = {JEDEC_RDID};

	if (spi_command(JEDEC_RDID_OUTSIZE, JEDEC_RDID_INSIZE, cmd, readarr))
		return 1;
	printf_debug("RDID returned %02x %02x %02x.\n", readarr[0], readarr[1], readarr[2]);
	return 0;
}

static int spi_res(unsigned char *readarr)
{
	const unsigned char cmd[JEDEC_RES_OUTSIZE] = {JEDEC_RES, 0, 0, 0};

	if (spi_command(JEDEC_RES_OUTSIZE, JEDEC_RES_INSIZE, cmd, readarr))
		return 1;
	printf_debug("RES returned %02x.\n", readarr[0]);
	return 0;
}

void spi_write_enable()
{
	const unsigned char cmd[JEDEC_WREN_OUTSIZE] = {JEDEC_WREN};

	/* Send WREN (Write Enable) */
	spi_command(JEDEC_WREN_OUTSIZE, JEDEC_WREN_INSIZE, cmd, NULL);
}

void spi_write_disable()
{
	const unsigned char cmd[JEDEC_WRDI_OUTSIZE] = {JEDEC_WRDI};

	/* Send WRDI (Write Disable) */
	spi_command(JEDEC_WRDI_OUTSIZE, JEDEC_WRDI_INSIZE, cmd, NULL);
}

int probe_spi_rdid(struct flashchip *flash)
{
	unsigned char readarr[3];
	uint32_t manuf_id;
	uint32_t model_id;

	if (spi_rdid(readarr))
		return 0;

	if (!oddparity(readarr[0]))
		printf_debug("RDID byte 0 parity violation.\n");

	/* Check if this is a continuation vendor ID */
	if (readarr[0] == 0x7f) {
		if (!oddparity(readarr[1]))
			printf_debug("RDID byte 1 parity violation.\n");
		manuf_id = (readarr[0] << 8) | readarr[1];
		model_id = readarr[2];
	} else {
		manuf_id = readarr[0];
		model_id = (readarr[1] << 8) | readarr[2];
	}

	printf_debug("%s: id1 0x%x, id2 0x%x\n", __FUNCTION__, manuf_id, model_id);

	if (manuf_id == flash->manufacture_id &&
	    model_id == flash->model_id) {
		/* Print the status register to tell the
		 * user about possible write protection.
		 */
		spi_prettyprint_status_register(flash);

		return 1;
	}

	/* Test if this is a pure vendor match. */
	if (manuf_id == flash->manufacture_id &&
	    GENERIC_DEVICE_ID == flash->model_id)
		return 1;

	return 0;
}

int probe_spi_res(struct flashchip *flash)
{
	unsigned char readarr[3];
	uint32_t model_id;

	if (spi_rdid(readarr))
		/* We couldn't issue RDID, it's pointless to try RES. */
		return 0;

	/* Check if RDID returns 0xff 0xff 0xff, then we use RES. */
	if ((readarr[0] != 0xff) || (readarr[1] != 0xff) ||
	    (readarr[2] != 0xff))
		return 0;

	if (spi_res(readarr))
		return 0;

	model_id = readarr[0];
	printf_debug("%s: id 0x%x\n", __FUNCTION__, model_id);
	if (model_id != flash->model_id)
		return 0;

	/* Print the status register to tell the
	 * user about possible write protection.
	 */
	spi_prettyprint_status_register(flash);
	return 1;
}

uint8_t spi_read_status_register()
{
	const unsigned char cmd[JEDEC_RDSR_OUTSIZE] = {JEDEC_RDSR};
	unsigned char readarr[1];

	/* Read Status Register */
	spi_command(JEDEC_RDSR_OUTSIZE, JEDEC_RDSR_INSIZE, cmd, readarr);
	return readarr[0];
}

/* Prettyprint the status register. Common definitions.
 */
void spi_prettyprint_status_register_common(uint8_t status)
{
	printf_debug("Chip status register: Bit 5 / Block Protect 3 (BP3) is "
		"%sset\n", (status & (1 << 5)) ? "" : "not ");
	printf_debug("Chip status register: Bit 4 / Block Protect 2 (BP2) is "
		"%sset\n", (status & (1 << 4)) ? "" : "not ");
	printf_debug("Chip status register: Bit 3 / Block Protect 1 (BP1) is "
		"%sset\n", (status & (1 << 3)) ? "" : "not ");
	printf_debug("Chip status register: Bit 2 / Block Protect 0 (BP0) is "
		"%sset\n", (status & (1 << 2)) ? "" : "not ");
	printf_debug("Chip status register: Write Enable Latch (WEL) is "
		"%sset\n", (status & (1 << 1)) ? "" : "not ");
	printf_debug("Chip status register: Write In Progress (WIP/BUSY) is "
		"%sset\n", (status & (1 << 0)) ? "" : "not ");
}

/* Prettyprint the status register. Works for
 * ST M25P series
 * MX MX25L series
 */
void spi_prettyprint_status_register_st_m25p(uint8_t status)
{
	printf_debug("Chip status register: Status Register Write Disable "
		"(SRWD) is %sset\n", (status & (1 << 7)) ? "" : "not ");
	printf_debug("Chip status register: Bit 6 is "
		"%sset\n", (status & (1 << 6)) ? "" : "not ");
	spi_prettyprint_status_register_common(status);
}

/* Prettyprint the status register. Works for
 * SST 25VF016
 */
void spi_prettyprint_status_register_sst25vf016(uint8_t status)
{
	const char *bpt[] = {
		"none",
		"1F0000H-1FFFFFH",
		"1E0000H-1FFFFFH",
		"1C0000H-1FFFFFH",
		"180000H-1FFFFFH",
		"100000H-1FFFFFH",
		"all", "all"
	};
	printf_debug("Chip status register: Block Protect Write Disable "
		"(BPL) is %sset\n", (status & (1 << 7)) ? "" : "not ");
	printf_debug("Chip status register: Auto Address Increment Programming "
		"(AAI) is %sset\n", (status & (1 << 6)) ? "" : "not ");
	spi_prettyprint_status_register_common(status);
	printf_debug("Resulting block protection : %s\n",
		bpt[(status & 0x1c) >> 2]);
}

void spi_prettyprint_status_register(struct flashchip *flash)
{
	uint8_t status;

	status = spi_read_status_register();
	printf_debug("Chip status register is %02x\n", status);
	switch (flash->manufacture_id) {
	case ST_ID:
		if (((flash->model_id & 0xff00) == 0x2000) ||
		    ((flash->model_id & 0xff00) == 0x2500))
			spi_prettyprint_status_register_st_m25p(status);
		break;
	case MX_ID:
		if ((flash->model_id & 0xff00) == 0x2000)
			spi_prettyprint_status_register_st_m25p(status);
		break;
	case SST_ID:
		if (flash->model_id == SST_25VF016B)
			spi_prettyprint_status_register_sst25vf016(status);
		break;
	}
}
	
int spi_chip_erase_c7(struct flashchip *flash)
{
	const unsigned char cmd[JEDEC_CE_C7_OUTSIZE] = {JEDEC_CE_C7};
	
	spi_disable_blockprotect();
	spi_write_enable();
	/* Send CE (Chip Erase) */
	spi_command(JEDEC_CE_C7_OUTSIZE, JEDEC_CE_C7_INSIZE, cmd, NULL);
	/* Wait until the Write-In-Progress bit is cleared.
	 * This usually takes 1-85 s, so wait in 1 s steps.
	 */
	while (spi_read_status_register() & JEDEC_RDSR_BIT_WIP)
		sleep(1);
	return 0;
}

/* Block size is usually
 * 64k for Macronix
 * 32k for SST
 * 4-32k non-uniform for EON
 */
int spi_block_erase_d8(const struct flashchip *flash, unsigned long addr)
{
	unsigned char cmd[JEDEC_BE_D8_OUTSIZE] = {JEDEC_BE_D8};

	cmd[1] = (addr & 0x00ff0000) >> 16;
	cmd[2] = (addr & 0x0000ff00) >> 8;
	cmd[3] = (addr & 0x000000ff);
	spi_write_enable();
	/* Send BE (Block Erase) */
	spi_command(JEDEC_BE_D8_OUTSIZE, JEDEC_BE_D8_INSIZE, cmd, NULL);
	/* Wait until the Write-In-Progress bit is cleared.
	 * This usually takes 100-4000 ms, so wait in 100 ms steps.
	 */
	while (spi_read_status_register() & JEDEC_RDSR_BIT_WIP)
		usleep(100 * 1000);
	return 0;
}

/* Sector size is usually 4k, though Macronix eliteflash has 64k */
int spi_sector_erase(const struct flashchip *flash, unsigned long addr)
{
	unsigned char cmd[JEDEC_SE_OUTSIZE] = {JEDEC_SE};
	cmd[1] = (addr & 0x00ff0000) >> 16;
	cmd[2] = (addr & 0x0000ff00) >> 8;
	cmd[3] = (addr & 0x000000ff);

	spi_write_enable();
	/* Send SE (Sector Erase) */
	spi_command(JEDEC_SE_OUTSIZE, JEDEC_SE_INSIZE, cmd, NULL);
	/* Wait until the Write-In-Progress bit is cleared.
	 * This usually takes 15-800 ms, so wait in 10 ms steps.
	 */
	while (spi_read_status_register() & JEDEC_RDSR_BIT_WIP)
		usleep(10 * 1000);
	return 0;
}

void spi_page_program(int block, uint8_t *buf, uint8_t *bios)
{
	if (it8716f_flashport) {
		it8716f_spi_page_program(block, buf, bios);
		return;
	}
	printf_debug("%s called, but no SPI chipset detected\n", __FUNCTION__);
}

/*
 * This is according the SST25VF016 datasheet, who knows it is more
 * generic that this...
 */
void spi_write_status_register(int status)
{
	const unsigned char cmd[JEDEC_WRSR_OUTSIZE] = {JEDEC_WRSR, (unsigned char)status};

	/* Send WRSR (Write Status Register) */
	spi_command(JEDEC_WRSR_OUTSIZE, JEDEC_WRSR_INSIZE, cmd, NULL);
}

void spi_byte_program(int address, uint8_t byte)
{
	const unsigned char cmd[JEDEC_BYTE_PROGRAM_OUTSIZE] = {JEDEC_BYTE_PROGRAM,
		(address>>16)&0xff,
		(address>>8)&0xff,
		(address>>0)&0xff,
		byte
	};

	/* Send Byte-Program */
	spi_command(JEDEC_BYTE_PROGRAM_OUTSIZE, JEDEC_BYTE_PROGRAM_INSIZE, cmd, NULL);
}

void spi_disable_blockprotect(void)
{
	uint8_t status;

	status = spi_read_status_register();
	/* If there is block protection in effect, unprotect it first. */
	if ((status & 0x3c) != 0) {
		printf_debug("Some block protection in effect, disabling\n");
		spi_write_enable();
		spi_write_status_register(status & ~0x3c);
	}
}

void spi_nbyte_read(int address, uint8_t *bytes, int len)
{
	const unsigned char cmd[JEDEC_READ_OUTSIZE] = {JEDEC_READ,
		(address >> 16) & 0xff,
		(address >> 8) & 0xff,
		(address >> 0) & 0xff,
	};

	/* Send Read */
	spi_command(JEDEC_READ_OUTSIZE, len, cmd, bytes);
}

int spi_chip_read(struct flashchip *flash, uint8_t *buf)
{
	if (it8716f_flashport)
		return it8716f_spi_chip_read(flash, buf);
       else if (ich7_detected)
               return ich_spi_read(flash, buf);
	else if (ich9_detected)
		return ich_spi_read(flash, buf);
	printf_debug("%s called, but no SPI chipset detected\n", __FUNCTION__);
	return 1;
}

int spi_chip_write(struct flashchip *flash, uint8_t *buf)
{
	if (it8716f_flashport)
		return it8716f_spi_chip_write(flash, buf);
       else if (ich7_detected)
               return ich_spi_write(flash, buf);
	else if (ich9_detected)
		return ich_spi_write(flash, buf);
	printf_debug("%s called, but no SPI chipset detected\n", __FUNCTION__);
	return 1;
}

