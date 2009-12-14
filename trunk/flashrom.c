/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2000 Silicon Integrated System Corporation
 * Copyright (C) 2004 Tyan Corp <yhlu@tyan.com>
 * Copyright (C) 2005-2008 coresystems GmbH
 * Copyright (C) 2008,2009 Carl-Daniel Hailfinger
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "flash.h"
#include "flashchips.h"

const char *flashrom_version = FLASHROM_VERSION;
char *chip_to_probe = NULL;
int verbose = 0;

#if INTERNAL_SUPPORT == 1
enum programmer programmer = PROGRAMMER_INTERNAL;
#elif DUMMY_SUPPORT == 1
enum programmer programmer = PROGRAMMER_DUMMY;
#else
/* Activating the #error explodes on make dep. */
//#error Neither internal nor dummy selected
#endif

char *programmer_param = NULL;

/**
 * flashrom defaults to Parallel/LPC/FWH flash devices. If a known host
 * controller is found, the init routine sets the buses_supported bitfield to
 * contain the supported buses for that controller.
 */
enum chipbustype buses_supported = CHIP_BUSTYPE_NONSPI;

/**
 * Programmers supporting multiple buses can have differing size limits on
 * each bus. Store the limits for each bus in a common struct.
 */
struct decode_sizes max_rom_decode = {
	.parallel	= 0xffffffff,
	.lpc		= 0xffffffff,
	.fwh		= 0xffffffff,
	.spi		= 0xffffffff
};

const struct programmer_entry programmer_table[] = {
#if INTERNAL_SUPPORT == 1
	{
		.name			= "internal",
		.init			= internal_init,
		.shutdown		= internal_shutdown,
		.map_flash_region	= physmap,
		.unmap_flash_region	= physunmap,
		.chip_readb		= internal_chip_readb,
		.chip_readw		= internal_chip_readw,
		.chip_readl		= internal_chip_readl,
		.chip_readn		= internal_chip_readn,
		.chip_writeb		= internal_chip_writeb,
		.chip_writew		= internal_chip_writew,
		.chip_writel		= internal_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if DUMMY_SUPPORT == 1
	{
		.name			= "dummy",
		.init			= dummy_init,
		.shutdown		= dummy_shutdown,
		.map_flash_region	= dummy_map,
		.unmap_flash_region	= dummy_unmap,
		.chip_readb		= dummy_chip_readb,
		.chip_readw		= dummy_chip_readw,
		.chip_readl		= dummy_chip_readl,
		.chip_readn		= dummy_chip_readn,
		.chip_writeb		= dummy_chip_writeb,
		.chip_writew		= dummy_chip_writew,
		.chip_writel		= dummy_chip_writel,
		.chip_writen		= dummy_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if NIC3COM_SUPPORT == 1
	{
		.name			= "nic3com",
		.init			= nic3com_init,
		.shutdown		= nic3com_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= nic3com_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= nic3com_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if GFXNVIDIA_SUPPORT == 1
	{
		.name			= "gfxnvidia",
		.init			= gfxnvidia_init,
		.shutdown		= gfxnvidia_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= gfxnvidia_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= gfxnvidia_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if DRKAISER_SUPPORT == 1
	{
		.name			= "drkaiser",
		.init			= drkaiser_init,
		.shutdown		= drkaiser_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= drkaiser_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= drkaiser_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if SATASII_SUPPORT == 1
	{
		.name			= "satasii",
		.init			= satasii_init,
		.shutdown		= satasii_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= satasii_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= satasii_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if INTERNAL_SUPPORT == 1
	{
		.name			= "it87spi",
		.init			= it87spi_init,
		.shutdown		= noop_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= noop_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= noop_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if FT2232_SPI_SUPPORT == 1
	{
		.name			= "ft2232spi",
		.init			= ft2232_spi_init,
		.shutdown		= noop_shutdown, /* Missing shutdown */
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= noop_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= noop_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

#if SERPROG_SUPPORT == 1
	{
		.name			= "serprog",
		.init			= serprog_init,
		.shutdown		= serprog_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= serprog_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= serprog_chip_readn,
		.chip_writeb		= serprog_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= serprog_delay,
	},
#endif

#if BUSPIRATE_SPI_SUPPORT == 1
	{
		.name			= "buspiratespi",
		.init			= buspirate_spi_init,
		.shutdown		= buspirate_spi_shutdown,
		.map_flash_region	= fallback_map,
		.unmap_flash_region	= fallback_unmap,
		.chip_readb		= noop_chip_readb,
		.chip_readw		= fallback_chip_readw,
		.chip_readl		= fallback_chip_readl,
		.chip_readn		= fallback_chip_readn,
		.chip_writeb		= noop_chip_writeb,
		.chip_writew		= fallback_chip_writew,
		.chip_writel		= fallback_chip_writel,
		.chip_writen		= fallback_chip_writen,
		.delay			= internal_delay,
	},
#endif

	{}, /* This entry corresponds to PROGRAMMER_INVALID. */
};

int programmer_init(void)
{
	return programmer_table[programmer].init();
}

int programmer_shutdown(void)
{
	return programmer_table[programmer].shutdown();
}

void *programmer_map_flash_region(const char *descr, unsigned long phys_addr,
				  size_t len)
{
	return programmer_table[programmer].map_flash_region(descr,
							     phys_addr, len);
}

void programmer_unmap_flash_region(void *virt_addr, size_t len)
{
	programmer_table[programmer].unmap_flash_region(virt_addr, len);
}

void chip_writeb(uint8_t val, chipaddr addr)
{
	programmer_table[programmer].chip_writeb(val, addr);
}

void chip_writew(uint16_t val, chipaddr addr)
{
	programmer_table[programmer].chip_writew(val, addr);
}

void chip_writel(uint32_t val, chipaddr addr)
{
	programmer_table[programmer].chip_writel(val, addr);
}

void chip_writen(uint8_t *buf, chipaddr addr, size_t len)
{
	programmer_table[programmer].chip_writen(buf, addr, len);
}

uint8_t chip_readb(const chipaddr addr)
{
	return programmer_table[programmer].chip_readb(addr);
}

uint16_t chip_readw(const chipaddr addr)
{
	return programmer_table[programmer].chip_readw(addr);
}

uint32_t chip_readl(const chipaddr addr)
{
	return programmer_table[programmer].chip_readl(addr);
}

void chip_readn(uint8_t *buf, chipaddr addr, size_t len)
{
	programmer_table[programmer].chip_readn(buf, addr, len);
}

void programmer_delay(int usecs)
{
	programmer_table[programmer].delay(usecs);
}

void map_flash_registers(struct flashchip *flash)
{
	size_t size = flash->total_size * 1024;
	/* Flash registers live 4 MByte below the flash. */
	/* FIXME: This is incorrect for nonstandard flashbase. */
	flash->virtual_registers = (chipaddr)programmer_map_flash_region("flash chip registers", (0xFFFFFFFF - 0x400000 - size + 1), size);
}

int read_memmapped(struct flashchip *flash, uint8_t *buf, int start, int len)
{
	chip_readn(buf, flash->virtual_memory + start, len);
		
	return 0;
}

unsigned long flashbase = 0;

int min(int a, int b)
{
	return (a < b) ? a : b;
}

int max(int a, int b)
{
	return (a > b) ? a : b;
}

int bitcount(unsigned long a)
{
	int i = 0;
	for (; a != 0; a >>= 1)
		if (a & 1)
			i++;
	return i;
}

char *strcat_realloc(char *dest, const char *src)
{
	dest = realloc(dest, strlen(dest) + strlen(src) + 1);
	if (!dest)
		return NULL;
	strcat(dest, src);
	return dest;
}

/* This is a somewhat hacked function similar in some ways to strtok().
 * It will look for needle in haystack, return a copy of needle and remove
 * everything from the first occurrence of needle to the next delimiter
 * from haystack.
 */
char *extract_param(char **haystack, char *needle, char *delim)
{
	char *param_pos, *rest, *tmp;
	char *dev = NULL;
	int devlen;

	param_pos = strstr(*haystack, needle);
	do {
		if (!param_pos)
			return NULL;
		/* Beginning of the string? */
		if (param_pos == *haystack)
			break;
		/* After a delimiter? */
		if (strchr(delim, *(param_pos - 1)))
			break;
		/* Continue searching. */
		param_pos++;
		param_pos = strstr(param_pos, needle);
	} while (1);
		
	if (param_pos) {
		param_pos += strlen(needle);
		devlen = strcspn(param_pos, delim);
		if (devlen) {
			dev = malloc(devlen + 1);
			if (!dev) {
				fprintf(stderr, "Out of memory!\n");
				exit(1);
			}
			strncpy(dev, param_pos, devlen);
			dev[devlen] = '\0';
		}
		rest = param_pos + devlen;
		rest += strspn(rest, delim);
		param_pos -= strlen(needle);
		memmove(param_pos, rest, strlen(rest) + 1);
		tmp = realloc(*haystack, strlen(*haystack) + 1);
		if (!tmp) {
			fprintf(stderr, "Out of memory!\n");
			exit(1);
		}
		*haystack = tmp;
	}
	

	return dev;
}

/* start is an offset to the base address of the flash chip */
int check_erased_range(struct flashchip *flash, int start, int len)
{
	int ret;
	uint8_t *cmpbuf = malloc(len);

	if (!cmpbuf) {
		fprintf(stderr, "Could not allocate memory!\n");
		exit(1);
	}
	memset(cmpbuf, 0xff, len);
	ret = verify_range(flash, cmpbuf, start, len, "ERASE");
	free(cmpbuf);
	return ret;
}

/**
 * @cmpbuf	buffer to compare against, cmpbuf[0] is expected to match the
		flash content at location start
 * @start	offset to the base address of the flash chip
 * @len		length of the verified area
 * @message	string to print in the "FAILED" message
 * @return	0 for success, -1 for failure
 */
int verify_range(struct flashchip *flash, uint8_t *cmpbuf, int start, int len, char *message)
{
	int i, j, starthere, lenhere, ret = 0;
	int page_size = flash->page_size;
	uint8_t *readbuf = malloc(page_size);
	int failcount = 0;

	if (!len)
		goto out_free;

	if (!flash->read) {
		fprintf(stderr, "ERROR: flashrom has no read function for this flash chip.\n");
		return 1;
	}
	if (!readbuf) {
		fprintf(stderr, "Could not allocate memory!\n");
		exit(1);
	}

	if (start + len > flash->total_size * 1024) {
		fprintf(stderr, "Error: %s called with start 0x%x + len 0x%x >"
			" total_size 0x%x\n", __func__, start, len,
			flash->total_size * 1024);
		ret = -1;
		goto out_free;
	}
	if (!message)
		message = "VERIFY";
	
	/* Warning: This loop has a very unusual condition and body.
	 * The loop needs to go through each page with at least one affected
	 * byte. The lowest page number is (start / page_size) since that
	 * division rounds down. The highest page number we want is the page
	 * where the last byte of the range lives. That last byte has the
	 * address (start + len - 1), thus the highest page number is
	 * (start + len - 1) / page_size. Since we want to include that last
	 * page as well, the loop condition uses <=.
	 */
	for (i = start / page_size; i <= (start + len - 1) / page_size; i++) {
		/* Byte position of the first byte in the range in this page. */
		starthere = max(start, i * page_size);
		/* Length of bytes in the range in this page. */
		lenhere = min(start + len, (i + 1) * page_size) - starthere;
		flash->read(flash, readbuf, starthere, lenhere);
		for (j = 0; j < lenhere; j++) {
			if (cmpbuf[starthere - start + j] != readbuf[j]) {
				/* Only print the first failure. */
				if (!failcount++)
					fprintf(stderr, "%s FAILED at 0x%08x! "
						"Expected=0x%02x, Read=0x%02x,",
						message, starthere + j,
						cmpbuf[starthere - start + j],
						readbuf[j]);
			}
		}
	}
	if (failcount) {
		fprintf(stderr, " failed byte count from 0x%08x-0x%08x: 0x%x\n",
			start, start + len - 1, failcount);
		ret = -1;
	}

out_free:
	free(readbuf);
	return ret;
}

/* This function generates various test patterns useful for testing controller
 * and chip communication as well as chip behaviour.
 *
 * If a byte can be written multiple times, each time keeping 0-bits at 0
 * and changing 1-bits to 0 if the new value for that bit is 0, the effect
 * is essentially an AND operation. That's also the reason why this function
 * provides the result of AND between various patterns.
 *
 * Below is a list of patterns (and their block length).
 * Pattern 0 is 05 15 25 35 45 55 65 75 85 95 a5 b5 c5 d5 e5 f5 (16 Bytes)
 * Pattern 1 is 0a 1a 2a 3a 4a 5a 6a 7a 8a 9a aa ba ca da ea fa (16 Bytes)
 * Pattern 2 is 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f (16 Bytes)
 * Pattern 3 is a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af (16 Bytes)
 * Pattern 4 is 00 10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 (16 Bytes)
 * Pattern 5 is 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f (16 Bytes)
 * Pattern 6 is 00 (1 Byte)
 * Pattern 7 is ff (1 Byte)
 * Patterns 0-7 have a big-endian block number in the last 2 bytes of each 256
 * byte block.
 *
 * Pattern 8 is 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11... (256 B)
 * Pattern 9 is ff fe fd fc fb fa f9 f8 f7 f6 f5 f4 f3 f2 f1 f0 ef ee... (256 B)
 * Pattern 10 is 00 00 00 01 00 02 00 03 00 04... (128 kB big-endian counter)
 * Pattern 11 is ff ff ff fe ff fd ff fc ff fb... (128 kB big-endian downwards)
 * Pattern 12 is 00 (1 Byte)
 * Pattern 13 is ff (1 Byte)
 * Patterns 8-13 have no block number.
 *
 * Patterns 0-3 are created to detect and efficiently diagnose communication
 * slips like missed bits or bytes and their repetitive nature gives good visual
 * cues to the person inspecting the results. In addition, the following holds:
 * AND Pattern 0/1 == Pattern 4
 * AND Pattern 2/3 == Pattern 5
 * AND Pattern 0/1/2/3 == AND Pattern 4/5 == Pattern 6
 * A weakness of pattern 0-5 is the inability to detect swaps/copies between
 * any two 16-byte blocks except for the last 16-byte block in a 256-byte bloc.
 * They work perfectly for detecting any swaps/aliasing of blocks >= 256 bytes.
 * 0x5 and 0xa were picked because they are 0101 and 1010 binary.
 * Patterns 8-9 are best for detecting swaps/aliasing of blocks < 256 bytes.
 * Besides that, they provide for bit testing of the last two bytes of every
 * 256 byte block which contains the block number for patterns 0-6.
 * Patterns 10-11 are special purpose for detecting subblock aliasing with
 * block sizes >256 bytes (some Dataflash chips etc.)
 * AND Pattern 8/9 == Pattern 12
 * AND Pattern 10/11 == Pattern 12
 * Pattern 13 is the completely erased state.
 * None of the patterns can detect aliasing at boundaries which are a multiple
 * of 16 MBytes (but such chips do not exist anyway for Parallel/LPC/FWH/SPI).
 */
int generate_testpattern(uint8_t *buf, uint32_t size, int variant)
{
	int i;

	if (!buf) {
		fprintf(stderr, "Invalid buffer!\n");
		return 1;
	}

	switch (variant) {
	case 0:
		for (i = 0; i < size; i++)
			buf[i] = (i & 0xf) << 4 | 0x5;
		break;
	case 1:
		for (i = 0; i < size; i++)
			buf[i] = (i & 0xf) << 4 | 0xa;
		break;
	case 2:
		for (i = 0; i < size; i++)
			buf[i] = 0x50 | (i & 0xf);
		break;
	case 3:
		for (i = 0; i < size; i++)
			buf[i] = 0xa0 | (i & 0xf);
		break;
	case 4:
		for (i = 0; i < size; i++)
			buf[i] = (i & 0xf) << 4;
		break;
	case 5:
		for (i = 0; i < size; i++)
			buf[i] = i & 0xf;
		break;
	case 6:
		memset(buf, 0x00, size);
		break;
	case 7:
		memset(buf, 0xff, size);
		break;
	case 8:
		for (i = 0; i < size; i++)
			buf[i] = i & 0xff;
		break;
	case 9:
		for (i = 0; i < size; i++)
			buf[i] = ~(i & 0xff);
		break;
	case 10:
		for (i = 0; i < size % 2; i++) {
			buf[i * 2] = (i >> 8) & 0xff;
			buf[i * 2 + 1] = i & 0xff;
		}
		if (size & 0x1)
			buf[i * 2] = (i >> 8) & 0xff;
		break;
	case 11:
		for (i = 0; i < size % 2; i++) {
			buf[i * 2] = ~((i >> 8) & 0xff);
			buf[i * 2 + 1] = ~(i & 0xff);
		}
		if (size & 0x1)
			buf[i * 2] = ~((i >> 8) & 0xff);
		break;
	case 12:
		memset(buf, 0x00, size);
		break;
	case 13:
		memset(buf, 0xff, size);
		break;
	}

	if ((variant >= 0) && (variant <= 7)) {
		/* Write block number in the last two bytes of each 256-byte
		 * block, big endian for easier reading of the hexdump.
		 * Note that this wraps around for chips larger than 2^24 bytes
		 * (16 MB).
		 */
		for (i = 0; i < size / 256; i++) {
			buf[i * 256 + 254] = (i >> 8) & 0xff;
			buf[i * 256 + 255] = i & 0xff;
		}
	}

	return 0;
}

int check_max_decode(enum chipbustype buses, uint32_t size)
{
	int limitexceeded = 0;
	if ((buses & CHIP_BUSTYPE_PARALLEL) &&
	    (max_rom_decode.parallel < size)) {
		limitexceeded++;
		printf_debug("Chip size %u kB is bigger than supported "
			     "size %u kB of chipset/board/programmer "
			     "for %s interface, "
			     "probe/read/erase/write may fail. ", size / 1024,
			     max_rom_decode.parallel / 1024, "Parallel");
	}
	if ((buses & CHIP_BUSTYPE_LPC) && (max_rom_decode.lpc < size)) {
		limitexceeded++;
		printf_debug("Chip size %u kB is bigger than supported "
			     "size %u kB of chipset/board/programmer "
			     "for %s interface, "
			     "probe/read/erase/write may fail. ", size / 1024,
			     max_rom_decode.lpc / 1024, "LPC");
	}
	if ((buses & CHIP_BUSTYPE_FWH) && (max_rom_decode.fwh < size)) {
		limitexceeded++;
		printf_debug("Chip size %u kB is bigger than supported "
			     "size %u kB of chipset/board/programmer "
			     "for %s interface, "
			     "probe/read/erase/write may fail. ", size / 1024,
			     max_rom_decode.fwh / 1024, "FWH");
	}
	if ((buses & CHIP_BUSTYPE_SPI) && (max_rom_decode.spi < size)) {
		limitexceeded++;
		printf_debug("Chip size %u kB is bigger than supported "
			     "size %u kB of chipset/board/programmer "
			     "for %s interface, "
			     "probe/read/erase/write may fail. ", size / 1024,
			     max_rom_decode.spi / 1024, "SPI");
	}
	if (!limitexceeded)
		return 0;
	/* Sometimes chip and programmer have more than one bus in common,
	 * and the limit is not exceeded on all buses. Tell the user.
	 */
	if (bitcount(buses) > limitexceeded)
		printf_debug("There is at least one common chip/programmer "
			     "interface which can support a chip of this size. "
			     "You can try --force at your own risk.\n");
	return 1;
}

struct flashchip *probe_flash(struct flashchip *first_flash, int force)
{
	struct flashchip *flash;
	unsigned long base = 0;
	uint32_t size;
	enum chipbustype buses_common;
	char *tmp;

	for (flash = first_flash; flash && flash->name; flash++) {
		if (chip_to_probe && strcmp(flash->name, chip_to_probe) != 0)
			continue;
		printf_debug("Probing for %s %s, %d KB: ",
			     flash->vendor, flash->name, flash->total_size);
		if (!flash->probe && !force) {
			printf_debug("failed! flashrom has no probe function for this flash chip.\n");
			continue;
		}
		buses_common = buses_supported & flash->bustype;
		if (!buses_common) {
			tmp = flashbuses_to_text(buses_supported);
			printf_debug("skipped. Host bus type %s ", tmp);
			free(tmp);
			tmp = flashbuses_to_text(flash->bustype);
			printf_debug("and chip bus type %s are incompatible.\n", tmp);
			free(tmp);
			continue;
		}

		size = flash->total_size * 1024;
		check_max_decode(buses_common, size);

		base = flashbase ? flashbase : (0xffffffff - size + 1);
		flash->virtual_memory = (chipaddr)programmer_map_flash_region("flash chip", base, size);

		if (force)
			break;

		if (flash->probe(flash) != 1)
			goto notfound;

		if (first_flash == flashchips
		    || flash->model_id != GENERIC_DEVICE_ID)
			break;

notfound:
		programmer_unmap_flash_region((void *)flash->virtual_memory, size);
	}

	if (!flash || !flash->name)
		return NULL;

	printf("Found chip \"%s %s\" (%d KB, %s) at physical address 0x%lx.\n",
	       flash->vendor, flash->name, flash->total_size,
	       flashbuses_to_text(flash->bustype), base);

	return flash;
}

int verify_flash(struct flashchip *flash, uint8_t *buf)
{
	int ret;
	int total_size = flash->total_size * 1024;

	printf("Verifying flash... ");

	ret = verify_range(flash, buf, 0, total_size, NULL);

	if (!ret)
		printf("VERIFIED.          \n");

	return ret;
}

int read_flash(struct flashchip *flash, char *filename)
{
	unsigned long numbytes;
	FILE *image;
	unsigned long size = flash->total_size * 1024;
	unsigned char *buf = calloc(size, sizeof(char));

	if (!filename) {
		printf("Error: No filename specified.\n");
		return 1;
	}
	if ((image = fopen(filename, "w")) == NULL) {
		perror(filename);
		exit(1);
	}
	printf("Reading flash... ");
	if (!flash->read) {
		printf("FAILED!\n");
		fprintf(stderr, "ERROR: flashrom has no read function for this flash chip.\n");
		return 1;
	} else
		flash->read(flash, buf, 0, size);

	numbytes = fwrite(buf, 1, size, image);
	fclose(image);
	free(buf);
	printf("%s.\n", numbytes == size ? "done" : "FAILED");
	if (numbytes != size)
		return 1;
	return 0;
}

int erase_flash(struct flashchip *flash)
{
	int i, j, k, ret = 0, found = 0;
	unsigned int start, len;

	printf("Erasing flash chip... ");
	for (k = 0; k < NUM_ERASEFUNCTIONS; k++) {
		unsigned int done = 0;
		struct block_eraser eraser = flash->block_erasers[k];

		printf_debug("Looking at blockwise erase function %i... ", k);
		if (!eraser.block_erase && !eraser.eraseblocks[0].count) {
			printf_debug("not defined. "
				"Looking for another erase function.\n");
			continue;
		}
		if (!eraser.block_erase && eraser.eraseblocks[0].count) {
			printf_debug("eraseblock layout is known, but no "
				"matching block erase function found. "
				"Looking for another erase function.\n");
			continue;
		}
		if (eraser.block_erase && !eraser.eraseblocks[0].count) {
			printf_debug("block erase function found, but "
				"eraseblock layout is unknown. "
				"Looking for another erase function.\n");
			continue;
		}
		found = 1;
		printf_debug("trying... ");
		for (i = 0; i < NUM_ERASEREGIONS; i++) {
			/* Blocks with zero size are bugs in flashchips.c.
			 * FIXME: This check should be performed on startup.
			 */
			if (eraser.eraseblocks[i].count &&
			    !eraser.eraseblocks[i].size) {
				fprintf(stderr, "ERROR: Erase region with size "
					"0 for this chip. Please report a bug "
					"at flashrom@flashrom.org\n");
				ret = 1;
				break;
			}
			/* count==0 for all automatically initialized array
			 * members so the loop below won't be executed for them.
			 */
			for (j = 0; j < eraser.eraseblocks[i].count; j++) {
				start = done + eraser.eraseblocks[i].size * j;
				len = eraser.eraseblocks[i].size;
				printf_debug("0x%06x-0x%06x, ", start,
					     start + len - 1);
				ret = eraser.block_erase(flash, start, len);
				if (ret)
					break;
			}
			if (ret)
				break;
			done += eraser.eraseblocks[i].count *
				eraser.eraseblocks[i].size;
		}
		printf_debug("\n");
		if (done != flash->total_size * 1024)
			fprintf(stderr, "ERROR: Erase region walking erased "
				"0x%06x bytes total, expected 0x%06x bytes.",
				done, flash->total_size * 1024);
		/* If everything is OK, don't try another erase function. */
		if (!ret)
			break;
	}
	/* If no block erase function was found or block erase failed, retry. */
	if ((!found || ret) && (flash->erase)) {
		found = 1;
		printf_debug("Trying whole-chip erase function... ");
		ret = flash->erase(flash);
	}
	if (!found) {
		fprintf(stderr, "ERROR: flashrom has no erase function for this flash chip.\n");
		return 1;
	}

	if (ret) {
		fprintf(stderr, "FAILED!\n");
	} else {
		printf("SUCCESS.\n");
	}
	return ret;
}

void emergency_help_message(void)
{
	fprintf(stderr, "Your flash chip is in an unknown state.\n"
		"Get help on IRC at irc.freenode.net (channel #flashrom) or\n"
		"mail flashrom@flashrom.org!\n--------------------"
		"-----------------------------------------------------------\n"
		"DO NOT REBOOT OR POWEROFF!\n");
}

void usage(const char *name)
{
	const char *pname;
	int pnamelen;
	int remaining = 0;
	enum programmer p;

	printf("usage: %s [-VfLzhR] [-E|-r file|-w file|-v file] [-c chipname]\n"
              "       [-m [vendor:]part] [-l file] [-i image] [-p programmer]\n\n", name);

	printf("Please note that the command line interface for flashrom will "
		"change before\nflashrom 1.0. Do not use flashrom in scripts "
		"or other automated tools without\nchecking that your flashrom"
		" version won't interpret options in a different way.\n\n");

	printf
	    ("   -r | --read:                      read flash and save into file\n"
	     "   -w | --write:                     write file into flash\n"
	     "   -v | --verify:                    verify flash against file\n"
	     "   -n | --noverify:                  don't verify flash against file\n"
	     "   -E | --erase:                     erase flash device\n"
	     "   -V | --verbose:                   more verbose output\n"
	     "   -c | --chip <chipname>:           probe only for specified flash chip\n"
#if INTERNAL_SUPPORT == 1
	     "   -m | --mainboard <[vendor:]part>: override mainboard settings\n"
#endif
	     "   -f | --force:                     force write without checking image\n"
	     "   -l | --layout <file.layout>:      read ROM layout from file\n"
	     "   -i | --image <name>:              only flash image name from flash layout\n"
	     "   -L | --list-supported:            print supported devices\n"
#if PRINT_WIKI_SUPPORT == 1
	     "   -z | --list-supported-wiki:       print supported devices in wiki syntax\n"
#endif
	     "   -p | --programmer <name>:         specify the programmer device");

	for (p = 0; p < PROGRAMMER_INVALID; p++) {
		pname = programmer_table[p].name;
		pnamelen = strlen(pname);
		if (remaining - pnamelen - 2 < 0) {
			printf("\n                                     ");
			remaining = 43;
		} else {
			printf(" ");
			remaining--;
		}
		if (p == 0) {
			printf("(");
			remaining--;
		}
		printf("%s", pname);
		remaining -= pnamelen;
		if (p < PROGRAMMER_INVALID - 1) {
			printf(",");
			remaining--;
		} else {
			printf(")\n");
		}
	}
		
	printf(
	     "   -h | --help:                      print this help text\n"
	     "   -R | --version:                   print the version (release)\n"
	     "\nYou can specify one of -E, -r, -w, -v or no operation. If no operation is\n"
	     "specified, then all that happens is that flash info is dumped.\n\n");
	exit(1);
}

void print_version(void)
{
	printf("flashrom v%s\n", flashrom_version);
}

int main(int argc, char *argv[])
{
	uint8_t *buf;
	unsigned long size, numbytes;
	FILE *image;
	/* Probe for up to three flash chips. */
	struct flashchip *flash, *flashes[3];
	const char *name;
	int namelen;
	int opt;
	int option_index = 0;
	int force = 0;
	int read_it = 0, write_it = 0, erase_it = 0, verify_it = 0;
	int dont_verify_it = 0, list_supported = 0;
#if PRINT_WIKI_SUPPORT == 1
	int list_supported_wiki = 0;
#endif
	int operation_specified = 0;
	int ret = 0, i;

#if PRINT_WIKI_SUPPORT == 1
	const char *optstring = "rRwvnVEfc:m:l:i:p:Lzh";
#else
	const char *optstring = "rRwvnVEfc:m:l:i:p:Lh";
#endif
	static struct option long_options[] = {
		{"read", 0, 0, 'r'},
		{"write", 0, 0, 'w'},
		{"erase", 0, 0, 'E'},
		{"verify", 0, 0, 'v'},
		{"noverify", 0, 0, 'n'},
		{"chip", 1, 0, 'c'},
		{"mainboard", 1, 0, 'm'},
		{"verbose", 0, 0, 'V'},
		{"force", 0, 0, 'f'},
		{"layout", 1, 0, 'l'},
		{"image", 1, 0, 'i'},
		{"list-supported", 0, 0, 'L'},
#if PRINT_WIKI_SUPPORT == 1
		{"list-supported-wiki", 0, 0, 'z'},
#endif
		{"programmer", 1, 0, 'p'},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'R'},
		{0, 0, 0, 0}
	};

	char *filename = NULL;

	char *tempstr = NULL;

	print_version();

	if (argc > 1) {
		/* Yes, print them. */
		int i;
		printf_debug("The arguments are:\n");
		for (i = 1; i < argc; ++i)
			printf_debug("%s\n", argv[i]);
	}

	/* Safety check. */
	if (ARRAY_SIZE(programmer_table) - 1 != PROGRAMMER_INVALID) {
		fprintf(stderr, "Programmer table miscompilation!\n");
		exit(1);
	}
	if (spi_programmer_count - 1 != SPI_CONTROLLER_INVALID) {
		fprintf(stderr, "SPI programmer table miscompilation!\n");
		exit(1);
	}
#if BITBANG_SPI_SUPPORT == 1
	if (bitbang_spi_master_count - 1 != BITBANG_SPI_INVALID) {
		fprintf(stderr, "Bitbanging SPI master table miscompilation!\n");
		exit(1);
	}
#endif

	setbuf(stdout, NULL);
	while ((opt = getopt_long(argc, argv, optstring,
				  long_options, &option_index)) != EOF) {
		switch (opt) {
		case 'r':
			if (++operation_specified > 1) {
				fprintf(stderr, "More than one operation "
					"specified. Aborting.\n");
				exit(1);
			}
			read_it = 1;
			break;
		case 'w':
			if (++operation_specified > 1) {
				fprintf(stderr, "More than one operation "
					"specified. Aborting.\n");
				exit(1);
			}
			write_it = 1;
			break;
		case 'v':
			//FIXME: gracefully handle superfluous -v
			if (++operation_specified > 1) {
				fprintf(stderr, "More than one operation "
					"specified. Aborting.\n");
				exit(1);
			}
			if (dont_verify_it) {
				fprintf(stderr, "--verify and --noverify are"
					"mutually exclusive. Aborting.\n");
				exit(1);
			}
			verify_it = 1;
			break;
		case 'n':
			if (verify_it) {
				fprintf(stderr, "--verify and --noverify are"
					"mutually exclusive. Aborting.\n");
				exit(1);
			}
			dont_verify_it = 1;
			break;
		case 'c':
			chip_to_probe = strdup(optarg);
			break;
		case 'V':
			verbose = 1;
			break;
		case 'E':
			if (++operation_specified > 1) {
				fprintf(stderr, "More than one operation "
					"specified. Aborting.\n");
				exit(1);
			}
			erase_it = 1;
			break;
#if INTERNAL_SUPPORT == 1
		case 'm':
			tempstr = strdup(optarg);
			lb_vendor_dev_from_string(tempstr);
			break;
#endif
		case 'f':
			force = 1;
			break;
		case 'l':
			tempstr = strdup(optarg);
			if (read_romlayout(tempstr))
				exit(1);
			break;
		case 'i':
			tempstr = strdup(optarg);
			find_romentry(tempstr);
			break;
		case 'L':
			list_supported = 1;
			break;
#if PRINT_WIKI_SUPPORT == 1
		case 'z':
			list_supported_wiki = 1;
			break;
#endif
		case 'p':
			for (programmer = 0; programmer < PROGRAMMER_INVALID; programmer++) {
				name = programmer_table[programmer].name;
				namelen = strlen(name);
				if (strncmp(optarg, name, namelen) == 0) {
					switch (optarg[namelen]) {
					case ':':
						programmer_param = strdup(optarg + namelen + 1);
						break;
					case '\0':
						break;
					default:
						/* The continue refers to the
						 * for loop. It is here to be
						 * able to differentiate between
						 * foo and foobar.
						 */
						continue;
					}
					break;
				}
			}
			if (programmer == PROGRAMMER_INVALID) {
				printf("Error: Unknown programmer %s.\n", optarg);
				exit(1);
			}
			break;
		case 'R':
			/* print_version() is always called during startup. */
			exit(0);
			break;
		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	if (list_supported) {
		print_supported();
		exit(0);
	}

#if PRINT_WIKI_SUPPORT == 1
	if (list_supported_wiki) {
		print_supported_wiki();
		exit(0);
	}
#endif

	if (read_it && write_it) {
		printf("Error: -r and -w are mutually exclusive.\n");
		usage(argv[0]);
	}

	if (optind < argc)
		filename = argv[optind++];
	
	if (optind < argc) {
		printf("Error: Extra parameter found.\n");
		usage(argv[0]);
	}

	if (programmer_init()) {
		fprintf(stderr, "Error: Programmer initialization failed.\n");
		exit(1);
	}

	myusec_calibrate_delay();

	for (i = 0; i < ARRAY_SIZE(flashes); i++) {
		flashes[i] =
		    probe_flash(i ? flashes[i - 1] + 1 : flashchips, 0);
		if (!flashes[i])
			for (i++; i < ARRAY_SIZE(flashes); i++)
				flashes[i] = NULL;
	}

	if (flashes[1]) {
		printf("Multiple flash chips were detected:");
		for (i = 0; i < ARRAY_SIZE(flashes) && flashes[i]; i++)
			printf(" %s", flashes[i]->name);
		printf("\nPlease specify which chip to use with the -c <chipname> option.\n");
		programmer_shutdown();
		exit(1);
	} else if (!flashes[0]) {
		printf("No EEPROM/flash device found.\n");
		if (!force || !chip_to_probe) {
			printf("If you know which flash chip you have, and if this version of flashrom\n");
			printf("supports a similar flash chip, you can try to force read your chip. Run:\n");
			printf("flashrom -f -r -c similar_supported_flash_chip filename\n");
			printf("\n");
			printf("Note: flashrom can never write when the flash chip isn't found automatically.\n");
		}
		if (force && read_it && chip_to_probe) {
			printf("Force read (-f -r -c) requested, forcing chip probe success:\n");
			flashes[0] = probe_flash(flashchips, 1);
			if (!flashes[0]) {
				printf("flashrom does not support a flash chip named '%s'.\n", chip_to_probe);
				printf("Run flashrom -L to view the hardware supported in this flashrom version.\n");
				exit(1);
			}
			printf("Please note that forced reads most likely contain garbage.\n");
			return read_flash(flashes[0], filename);
		}
		// FIXME: flash writes stay enabled!
		programmer_shutdown();
		exit(1);
	}

	flash = flashes[0];

	if (TEST_OK_MASK != (flash->tested & TEST_OK_MASK)) {
		printf("===\n");
		if (flash->tested & TEST_BAD_MASK) {
			printf("This flash part has status NOT WORKING for operations:");
			if (flash->tested & TEST_BAD_PROBE)
				printf(" PROBE");
			if (flash->tested & TEST_BAD_READ)
				printf(" READ");
			if (flash->tested & TEST_BAD_ERASE)
				printf(" ERASE");
			if (flash->tested & TEST_BAD_WRITE)
				printf(" WRITE");
			printf("\n");
		}
		if ((!(flash->tested & TEST_BAD_PROBE) && !(flash->tested & TEST_OK_PROBE)) ||
		    (!(flash->tested & TEST_BAD_READ) && !(flash->tested & TEST_OK_READ)) ||
		    (!(flash->tested & TEST_BAD_ERASE) && !(flash->tested & TEST_OK_ERASE)) ||
		    (!(flash->tested & TEST_BAD_WRITE) && !(flash->tested & TEST_OK_WRITE))) {
			printf("This flash part has status UNTESTED for operations:");
			if (!(flash->tested & TEST_BAD_PROBE) && !(flash->tested & TEST_OK_PROBE))
				printf(" PROBE");
			if (!(flash->tested & TEST_BAD_READ) && !(flash->tested & TEST_OK_READ))
				printf(" READ");
			if (!(flash->tested & TEST_BAD_ERASE) && !(flash->tested & TEST_OK_ERASE))
				printf(" ERASE");
			if (!(flash->tested & TEST_BAD_WRITE) && !(flash->tested & TEST_OK_WRITE))
				printf(" WRITE");
			printf("\n");
		}
		printf("Please email a report to flashrom@flashrom.org if any "
		       "of the above operations\nwork correctly for you with "
		       "this flash part. Please include the flashrom\noutput "
		       "with the additional -V option for all operations you "
		       "tested (-V, -rV,\n-wV, -EV), and mention which "
		       "mainboard or programmer you tested. Thanks for your "
		       "help!\n===\n");
	}

	size = flash->total_size * 1024;
	if (check_max_decode((buses_supported & flash->bustype), size) &&
	    (!force)) {
		fprintf(stderr, "Chip is too big for this programmer "
			"(-V gives details). Use --force to override.\n");
		programmer_shutdown();
		return 1;
	}

	if (!(read_it | write_it | verify_it | erase_it)) {
		printf("No operations were specified.\n");
		// FIXME: flash writes stay enabled!
		programmer_shutdown();
		exit(1);
	}

	if (!filename && !erase_it) {
		printf("Error: No filename specified.\n");
		// FIXME: flash writes stay enabled!
		programmer_shutdown();
		exit(1);
	}

	/* Always verify write operations unless -n is used. */
	if (write_it && !dont_verify_it)
		verify_it = 1;

	buf = (uint8_t *) calloc(size, sizeof(char));

	if (erase_it) {
		if (flash->tested & TEST_BAD_ERASE) {
			fprintf(stderr, "Erase is not working on this chip. ");
			if (!force) {
				fprintf(stderr, "Aborting.\n");
				programmer_shutdown();
				return 1;
			} else {
				fprintf(stderr, "Continuing anyway.\n");
			}
		}
		if (erase_flash(flash)) {
			emergency_help_message();
			programmer_shutdown();
			return 1;
		}
	} else if (read_it) {
		if (read_flash(flash, filename)) {
			programmer_shutdown();
			return 1;
		}
	} else {
		struct stat image_stat;

		if (flash->tested & TEST_BAD_ERASE) {
			fprintf(stderr, "Erase is not working on this chip "
				"and erase is needed for write. ");
			if (!force) {
				fprintf(stderr, "Aborting.\n");
				programmer_shutdown();
				return 1;
			} else {
				fprintf(stderr, "Continuing anyway.\n");
			}
		}
		if (flash->tested & TEST_BAD_WRITE) {
			fprintf(stderr, "Write is not working on this chip. ");
			if (!force) {
				fprintf(stderr, "Aborting.\n");
				programmer_shutdown();
				return 1;
			} else {
				fprintf(stderr, "Continuing anyway.\n");
			}
		}
		if ((image = fopen(filename, "r")) == NULL) {
			perror(filename);
			programmer_shutdown();
			exit(1);
		}
		if (fstat(fileno(image), &image_stat) != 0) {
			perror(filename);
			programmer_shutdown();
			exit(1);
		}
		if (image_stat.st_size != flash->total_size * 1024) {
			fprintf(stderr, "Error: Image size doesn't match\n");
			programmer_shutdown();
			exit(1);
		}

		numbytes = fread(buf, 1, size, image);
#if INTERNAL_SUPPORT == 1
		show_id(buf, size, force);
#endif
		fclose(image);
		if (numbytes != size) {
			fprintf(stderr, "Error: Failed to read file. Got %ld bytes, wanted %ld!\n", numbytes, size);
			programmer_shutdown();
			return 1;
		}
	}

	// This should be moved into each flash part's code to do it 
	// cleanly. This does the job.
	handle_romentries(buf, flash);

	// ////////////////////////////////////////////////////////////

	if (write_it) {
		printf("Writing flash chip... ");
		if (!flash->write) {
			fprintf(stderr, "Error: flashrom has no write function for this flash chip.\n");
			programmer_shutdown();
			return 1;
		}
		ret = flash->write(flash, buf);
		if (ret) {
			fprintf(stderr, "FAILED!\n");
			emergency_help_message();
			programmer_shutdown();
			return 1;
		} else {
			printf("COMPLETE.\n");
		}
	}

	if (verify_it) {
		/* Work around chips which need some time to calm down. */
		if (write_it)
			programmer_delay(1000*1000);
		ret = verify_flash(flash, buf);
		/* If we tried to write, and verification now fails, we
		 * might have an emergency situation.
		 */
		if (ret && write_it)
			emergency_help_message();
	}

	programmer_shutdown();

	return ret;
}
