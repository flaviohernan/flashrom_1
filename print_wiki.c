/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2009 Carl-Daniel Hailfinger
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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "flash.h"
#include "flashchips.h"

#if INTERNAL_SUPPORT == 1
struct board_info_url {
	const char *vendor;
	const char *name;
	const char *url;
};

struct board_info_notes {
	const char *vendor;
	const char *name;
	const char *note;
};
#endif

const char *wiki_header = "= Supported devices =\n\n\
<div style=\"margin-top:0.5em; padding:0.5em 0.5em 0.5em 0.5em; \
background-color:#eeeeee; align:right; border:1px solid #aabbcc;\"><small>\n\
Please do '''not''' edit these tables in the wiki directly, they are \
generated by pasting '''flashrom -z''' output.<br />\
'''Last update:''' %s(generated by flashrom %s)\n</small></div>\n";

#if INTERNAL_SUPPORT == 1
const char *chipset_th = "{| border=\"0\" style=\"font-size: smaller\"\n\
|- bgcolor=\"#6699dd\"\n! align=\"left\" | Vendor\n\
! align=\"left\" | Southbridge\n! align=\"left\" | PCI IDs\n\
! align=\"left\" | Status\n\n";

const char *board_th = "{| border=\"0\" style=\"font-size: smaller\" \
valign=\"top\"\n|- bgcolor=\"#6699dd\"\n! align=\"left\" | Vendor\n\
! align=\"left\" | Mainboard\n! align=\"left\" | Status\n\n";

const char *board_th2 = "{| border=\"0\" style=\"font-size: smaller\" \
valign=\"top\"\n|- bgcolor=\"#6699dd\"\n! align=\"left\" | Vendor\n\
! align=\"left\" | Mainboard\n! align=\"left\" | Required option\n\
! align=\"left\" | Status\n\n";

const char *board_intro = "\
\n== Supported mainboards ==\n\n\
In general, it is very likely that flashrom works out of the box even if your \
mainboard is not listed below.\n\nThis is a list of mainboards where we have \
verified that they either do or do not need any special initialization to \
make flashrom work (given flashrom supports the respective chipset and flash \
chip), or that they do not yet work at all. If they do not work, support may \
or may not be added later.\n\n\
Mainboards which don't appear in the list may or may not work (we don't \
know, someone has to give it a try). Please report any further verified \
mainboards on the [[Mailinglist|mailing list]].\n";
#endif

const char *chip_th = "{| border=\"0\" style=\"font-size: smaller\" \
valign=\"top\"\n|- bgcolor=\"#6699dd\"\n! align=\"left\" | Vendor\n\
! align=\"left\" | Device\n! align=\"left\" | Size / KB\n\
! align=\"left\" | Type\n! align=\"left\" colspan=\"4\" | Status\n\n\
|- bgcolor=\"#6699ff\"\n| colspan=\"4\" | &nbsp;\n\
| Probe\n| Read\n| Erase\n| Write\n\n";

const char *programmer_section = "\
\n== Supported programmers ==\n\nThis is a list \
of supported PCI devices flashrom can use as programmer:\n\n{| border=\"0\" \
valign=\"top\"\n| valign=\"top\"|\n\n{| border=\"0\" style=\"font-size: \
smaller\" valign=\"top\"\n|- bgcolor=\"#6699dd\"\n! align=\"left\" | Vendor\n\
! align=\"left\" | Device\n! align=\"left\" | PCI IDs\n\
! align=\"left\" | Status\n\n";

#if INTERNAL_SUPPORT == 1
const char *laptop_intro = "\n== Supported laptops/notebooks ==\n\n\
In general, flashing laptops is more difficult because laptops\n\n\
* often use the flash chip for stuff besides the BIOS,\n\
* often have special protection stuff which has to be handled by flashrom,\n\
* often use flash translation circuits which need drivers in flashrom.\n\n\
<div style=\"margin-top:0.5em; padding:0.5em 0.5em 0.5em 0.5em; \
background-color:#ff6666; align:right; border:1px solid #000000;\">\n\
'''IMPORTANT:''' At this point we recommend to '''not''' use flashrom on \
untested laptops unless you have a means to recover from a flashing that goes \
wrong (a working backup flash chip and/or good soldering skills).\n</div>\n";

/* Please keep these lists alphabetically ordered by vendor/board. */
const struct board_info_url boards_url[] = {
	/* Verified working boards that don't need write-enables. */
#if defined(__i386__) || defined(__x86_64__)
	{ "Abit",		"AX8",			"http://www.abit.com.tw/page/en/motherboard/motherboard_detail.php?DEFTITLE=Y&fMTYPE=Socket%20939&pMODEL_NAME=AX8" },
	{ "Abit",		"Fatal1ty F-I90HD",	"http://www.abit.com.tw/page/de/motherboard/motherboard_detail.php?pMODEL_NAME=Fatal1ty+F-I90HD&fMTYPE=LGA775" },
	{ "Advantech",		"PCM-5820", 		"http://www.emacinc.com/sbc_pc_compatible/pcm_5820.htm" },
	{ "ASI",		"MB-5BLMP",		"http://www.hojerteknik.com/winnet.htm" },
	{ "ASRock",		"A770CrossFire",	"http://www.asrock.com/mb/overview.asp?Model=A770CrossFire&s=AM2\%2b" },
	{ "ASRock",		"K8S8X",		"http://www.asrock.com/mb/overview.asp?Model=K8S8X" },
	{ "ASRock",		"M3A790GXH/128M"	"http://www.asrock.com/MB/overview.asp?Model=M3A790GXH/128M" },
	{ "ASUS",		"A7N8X Deluxe",		"http://www.asus.com/product.aspx?P_ID=wAsRYm41KTp78MFC" },
	{ "ASUS",		"A7N8X-E Deluxe",	"http://www.asus.com/product.aspx?P_ID=TmQtPJv4jIxmL9C2" },
	{ "ASUS",		"A7V133",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/socka/kt133a/a7v133/" },
	{ "ASUS",		"A7V400-MX",		"http://www.asus.com/product.aspx?P_ID=hORgEHRBDLMfwAwx" },
	{ "ASUS",		"A7V8X-MX",		"http://www.asus.com/product.aspx?P_ID=SEJOOYqfuQPitx2H" },
	{ "ASUS",		"A8N-E",		"http://www.asus.com/product.aspx?P_ID=DzbA8hgqchMBOVRz" },
	{ "ASUS",		"A8NE-FM/S",		"http://www.hardwareschotte.de/hardware/preise/proid_1266090/preis_ASUS+A8NE-FM" },
	{ "ASUS",		"A8N-SLI",		"http://www.asus.com/product.aspx?P_ID=J9FKa8z2xVId3pDK" },
	{ "ASUS",		"A8N-SLI Premium",	"http://www.asus.com/product.aspx?P_ID=nbulqxniNmzf0mH1" },
	{ "ASUS",		"A8V Deluxe",		"http://www.asus.com/product.aspx?P_ID=tvpdgPNCPaABZRVU" },
	{ "ASUS",		"A8V-E Deluxe",		"http://www.asus.com/product.aspx?P_ID=hQBPIJWEZnnGAZEh" },
	{ "ASUS",		"A8V-E SE",		"http://www.asus.com/product.aspx?P_ID=VMfiJJRYTHM4gXIi" },
	{ "ASUS",		"K8V",			"http://www.asus.com/product.aspx?P_ID=fG2KZOWF7v6MRFRm" },
	{ "ASUS",		"K8V SE Deluxe",	"http://www.asus.com/product.aspx?P_ID=65HeDI8XM1u6Uy6o" },
	{ "ASUS",		"K8V-X SE",		"http://www.asus.com/product.aspx?P_ID=lzDXlbBVHkdckHVr" },
	{ "ASUS",		"M2A-MX",		"http://www.asus.com/product.aspx?P_ID=BmaOnPewi1JgltOZ" },
	{ "ASUS",		"M2A-VM",		"http://www.asus.com/product.aspx?P_ID=St3pWpym8xXpROQS" },
	{ "ASUS",		"M2N-E",		"http://www.asus.com/product.aspx?P_ID=NFlvt10av3F7ayQ9" },
	{ "ASUS",		"M2V",			"http://www.asus.com/product.aspx?P_ID=OqYlEDFfF6ZqZGvp" },
	{ "ASUS",		"M3A78-EM",		"http://www.asus.com/product.aspx?P_ID=KjpYqzmAd9vsTM2D" },
	{ "ASUS",		"P2B",			"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440bx/p2b/" },
	{ "ASUS",		"P2B-D",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440bx/p2b-d/" },
	{ "ASUS",		"P2B-DS",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440bx/p2b-ds/" },
	{ "ASUS",		"P2B-F",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440bx/p2b-d/" },
	{ "ASUS",		"P2L97-S",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440lx/p2l97-s/" },
	{ "ASUS",		"P5B",			"ftp://ftp.asus.com.tw/pub/ASUS/mb/socket775/P5B/" },
	{ "ASUS",		"P5B-Deluxe",		"http://www.asus.com/product.aspx?P_ID=bswT66IBSb2rEWNa" },
	{ "ASUS",		"P5KC",			"http://www.asus.com/product.aspx?P_ID=fFZ8oUIGmLpwNMjj" },
	{ "ASUS",		"P5L-MX",		"http://www.asus.com/product.aspx?P_ID=X70d3NCzH2DE9vWH" },
	{ "ASUS",		"P6T Deluxe",		"http://www.asus.com/product.aspx?P_ID=vXixf82co6Q5v0BZ" },
	{ "ASUS",		"P6T Deluxe V2",	"http://www.asus.com/product.aspx?P_ID=iRlP8RG9han6saZx" },
	{ "A-Trend",		"ATC-6220",		"http://www.motherboard.cz/mb/atrend/atc6220.htm" },
	{ "BCOM",		"WinNET100",		"http://www.coreboot.org/BCOM_WINNET100" },
	{ "DFI",		"Blood-Iron P35 T2RL",	"http://lp.lanparty.com.tw/portal/CM/cmproduct/XX_cmproddetail/XX_WbProdsWindow?itemId=516&downloadFlag=false&action=1" },
	{ "Elitegroup",		"K7S5A",		"http://www.ecs.com.tw/ECSWebSite/Products/ProductsDetail.aspx?detailid=279&CategoryID=1&DetailName=Specification&MenuID=1&LanID=0" },
	{ "Elitegroup",		"P6VAP-A+",		"http://www.ecs.com.tw/ECSWebSite/Products/ProductsDetail.aspx?detailid=117&CategoryID=1&DetailName=Specification&MenuID=1&LanID=0" },
	{ "GIGABYTE",		"GA-6BXC",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1445" },
	{ "GIGABYTE",		"GA-6BXDU",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1429" },
	{ "GIGABYTE",		"GA-6ZMA",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1541" },
	{ "GIGABYTE",		"GA-965P-DS4",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2288" },
	{ "GIGABYTE",		"GA-EX58-UD4P",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2986" },
	{ "GIGABYTE",		"GA-EP35-DS3L",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2778" },
	{ "GIGABYTE",		"GA-MA69VM-S2",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2500" },
	{ "GIGABYTE",		"GA-MA790GP-DS4H",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2887" },
	{ "GIGABYTE",		"GA-MA78GPM-DS2H",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2859" },
	{ "GIGABYTE",		"GA-MA770T-UD3P",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=3096" },
	{ "Intel",		"EP80759",		NULL },
	{ "Jetway",		"J7F4K1G5D-PB",		"http://www.jetway.com.tw/jetway/system/productshow2.asp?id=389&proname=J7F4K1G5D-P" },
	{ "MSI",		"MS-6153",		"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=336" },
	{ "MSI",		"MS-6156",		"http://uk.ts.fujitsu.com/rl/servicesupport/techsupport/boards/Motherboards/MicroStar/Ms6156/MS6156.htm" },
	{ "MSI",		"MS-6330 (K7T Turbo)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=327" },
	{ "MSI",		"MS-6570 (K7N2)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=519" },
	{ "MSI",		"MS-7065",		"http://browse.geekbench.ca/geekbench2/view/53114" },
	{ "MSI",		"MS-7168 (Orion)",	"http://support.packardbell.co.uk/uk/item/index.php?i=spec_orion&pi=platform_honeymoon_istart" },
	{ "MSI",		"MS-7236 (945PL Neo3)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1173" },
	{ "MSI",		"MS-7255 (P4M890M)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1082" },
	{ "MSI",		"MS-7345 (P35 Neo2-FIR)","http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1261" },
	{ "MSI",		"MS-7312 (K9MM-V)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1104" },
	{ "MSI",		"MS-7368 (K9AG Neo2-Digital)", "http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1241" },
	{ "MSI",		"MS-7376 (K9A2 Platinum)", "http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=1332" },
	{ "NEC",		"PowerMate 2000",	"http://support.necam.com/mobilesolutions/hardware/Desktops/pm2000/celeron/" },
	{ "PC Engines",		"Alix.1c",		"http://pcengines.ch/alix1c.htm" },
	{ "PC Engines",		"Alix.2c2",		"http://pcengines.ch/alix2c2.htm" },
	{ "PC Engines",		"Alix.2c3",		"http://pcengines.ch/alix2c3.htm" },
	{ "PC Engines",		"Alix.3c3",		"http://pcengines.ch/alix3c3.htm" },
	{ "PC Engines",		"Alix.3d3",		"http://pcengines.ch/alix3d3.htm" },
	{ "PC Engines",		"WRAP.2E",		"http://pcengines.ch/wrap2e1.htm" },
	{ "RCA",		"RM4100",		"http://www.settoplinux.org/index.php?title=RCA_RM4100" },
	{ "Shuttle",		"FD37",			"http://www.shuttle.eu/products/discontinued/barebones/sd37p2/" },
	{ "Sun",		"Blade x6250",		"http://www.sun.com/servers/blades/x6250/" },
	{ "Supermicro",		"H8QC8",		"http://www.supermicro.com/Aplus/motherboard/Opteron/nforce/H8QC8.cfm" },
	{ "Supermicro",		"X8DTT-F",		"http://www.supermicro.com/products/motherboard/QPI/5500/X8DTT-F.cfm" },
	{ "Tekram",		"P6Pro-A5",		"http://www.motherboard.cz/mb/tekram/P6Pro-A5.htm" },
	{ "Thomson",		"IP1000",		"http://www.settoplinux.org/index.php?title=Thomson_IP1000" },
	{ "TriGem",		"Lomita",		"http://www.e4allupgraders.info/dir1/motherboards/socket370/lomita.shtml" },
	{ "T-Online",		"S-100",		"http://wiki.freifunk-hannover.de/T-Online_S_100" },
	{ "Tyan",		"iS5375-1U",		"http://www.tyan.com/product_board_detail.aspx?pid=610" },
	{ "Tyan",		"S1846",		"http://www.tyan.com/archive/products/html/tsunamiatx.html" },
	{ "Tyan",		"S2466",		"http://www.tyan.com/product_board_detail.aspx?pid=461" },
	{ "Tyan",		"S2881",		"http://www.tyan.com/product_board_detail.aspx?pid=115" },
	{ "Tyan",		"S2882",		"http://www.tyan.com/product_board_detail.aspx?pid=121" },
	{ "Tyan",		"S2882-D",		"http://www.tyan.com/product_board_detail.aspx?pid=127" },
	{ "Tyan",		"S2891",		"http://www.tyan.com/product_board_detail.aspx?pid=144" },
	{ "Tyan",		"S2892",		"http://www.tyan.com/product_board_detail.aspx?pid=145" },
	{ "Tyan",		"S2895",		"http://www.tyan.com/archive/products/html/thunderk8we.html" },
	{ "Tyan",		"S3095",		"http://www.tyan.com/product_board_detail.aspx?pid=181" },
	{ "Tyan",		"S5180",		"http://www.tyan.com/product_board_detail.aspx?pid=456" },
	{ "Tyan",		"S5191",		"http://www.tyan.com/product_board_detail.aspx?pid=343" },
	{ "Tyan",		"S5197",		"http://www.tyan.com/product_board_detail.aspx?pid=349" },
	{ "Tyan",		"S5211",		"http://www.tyan.com/product_board_detail.aspx?pid=591" },
	{ "Tyan",		"S5211-1U",		"http://www.tyan.com/product_board_detail.aspx?pid=593" },
	{ "Tyan",		"S5220",		"http://www.tyan.com/product_board_detail.aspx?pid=597" },
	{ "Tyan",		"S5375",		"http://www.tyan.com/product_board_detail.aspx?pid=566" },
	{ "Tyan",		"S5376G2NR/S5376WAG2NR","http://www.tyan.com/product_board_detail.aspx?pid=605" },
	{ "Tyan",		"S5377",		"http://www.tyan.com/product_SKU_spec.aspx?ProductType=MB&pid=642&SKU=600000017" },
	{ "Tyan",		"S5382 (Tempest i5000PW)", "http://www.tyan.com/product_board_detail.aspx?pid=439" },
	{ "Tyan",		"S5397",		"http://www.tyan.com/product_board_detail.aspx?pid=560" },
	{ "VIA",		"EPIA-EX15000G",	"http://www.via.com.tw/en/products/embedded/ProductDetail.jsp?productLine=1&motherboard_id=450" },
	{ "VIA",		"EPIA-LN",		"http://www.via.com.tw/en/products/mainboards/motherboards.jsp?motherboard_id=473" },
	{ "VIA",		"EPIA-M700",		"http://via.com.tw/servlet/downloadSvl?motherboard_id=670&download_file_id=3700" },
	{ "VIA",		"EPIA-NX15000G",	"http://www.via.com.tw/en/products/embedded/ProductDetail.jsp?productLine=1&motherboard_id=470" },
	{ "VIA",		"NAB74X0",		"http://www.via.com.tw/en/products/mainboards/motherboards.jsp?motherboard_id=590" },
	{ "VIA",		"pc2500e",		"http://www.via.com.tw/en/initiatives/empowered/pc2500_mainboard/index.jsp" },
	{ "VIA",		"VB700X",		"http://www.via.com.tw/en/products/mainboards/motherboards.jsp?motherboard_id=490" },

	/* Verified working boards that DO need write-enables. */
	{ "Abit",		"VT6X4",		"http://www.abit.com.tw/page/en/motherboard/motherboard_detail.php?fMTYPE=Slot%201&pMODEL_NAME=VT6X4" },
	{ "Abit",		"IP35",			"http://www.abit.com.tw/page/en/motherboard/motherboard_detail.php?fMTYPE=LGA775&pMODEL_NAME=IP35" },
	{ "Abit",		"IP35 Pro",		"http://www.abit.com.tw/page/de/motherboard/motherboard_detail.php?fMTYPE=LGA775&pMODEL_NAME=IP35%20Pro" },
	{ "Abit",               "NF7-S",                "http://www.abit.com.tw/page/en/motherboard/motherboard_detail.php?fMTYPE=Socket%20A&pMODEL_NAME=NF7-S"},
	{ "Acorp",		"6A815EPD",		"http://web.archive.org/web/20021206163652/www.acorp.com.tw/English/default.asp" },
	{ "agami",		"Aruma",		"http://web.archive.org/web/20080212111524/http://www.agami.com/site/ais-6000-series" },
	{ "Albatron",		"PM266A Pro",		"http://www.albatron.com.tw/English/Product/MB/pro_detail.asp?rlink=Overview&no=56" }, /* FIXME */
	{ "AOpen",		"vKM400Am-S",		"http://usa.aopen.com/products_detail.aspx?Auno=824" },
	{ "Artec Group",	"DBE61",		"http://wiki.thincan.org/DBE61" },
	{ "Artec Group",	"DBE62",		"http://wiki.thincan.org/DBE62" },
	{ "ASUS",		"A7V600-X",		"http://www.asus.com/product.aspx?P_ID=L2XYS0rmtCjeOr4k" },
	{ "ASUS",		"A7V8X",		"http://www.asus.com/product.aspx?P_ID=qfpaGrAy2kLVo0f2" },
	{ "ASUS",		"A7V8X-MX SE",		"http://www.asus.com/product.aspx?P_ID=1guVBT1qV5oqhHyZ" },
	{ "ASUS",		"A7V8X-X",		"http://www.asus.com/product.aspx?P_ID=YcXfRrWHZ9RKoVmw" },
	{ "ASUS",		"M2NBP-VM CSM",		"http://www.asus.com/product.aspx?P_ID=MnOfzTGd2KkwG0rF" },
	{ "ASUS",		"M2V-MX",		"http://www.asus.com/product.aspx?P_ID=7grf8Ci4yxnqzt3z" },
	{ "ASUS",		"P4B266",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/sock478/p4b266/" },
	{ "ASUS",		"P4C800-E Deluxe",	"http://www.asus.com/product.aspx?P_ID=cFuVCr9bXXCckmcK" },
	{ "ASUS",		"P4B266-LM",		"http://esupport.sony.com/US/perl/swu-list.pl?mdl=PCVRX650" },
	{ "ASUS",		"P4P800-E Deluxe",	"http://www.asus.com/product.aspx?P_ID=INIJUvLlif7LHp3g" },
	{ "ASUS",		"P5ND2-SLI Deluxe",	"http://www.asus.com/product.aspx?P_ID=WY7XroDuUImVbgp5" },
	{ "ASUS",		"P5A",			"ftp://ftp.asus.com.tw/pub/ASUS/mb/sock7/ali/p5a/" },
	{ "Biostar",		"P4M80-M4",		"http://www.biostar-usa.com/mbdetails.asp?model=p4m80-m4" },
	{ "Dell",		"PowerEdge 1850",	"http://support.dell.com/support/edocs/systems/pe1850/en/index.htm" },
	{ "Elitegroup",		"K7S6A",		"http://www.ecs.com.tw/ECSWebSite/Products/ProductsDetail.aspx?detailid=77&CategoryID=1&DetailName=Specification&MenuID=52&LanID=0" },
	{ "Elitegroup",		"K7VTA3",		"http://www.ecs.com.tw/ECSWebSite/Products/ProductsDetail.aspx?detailid=264&CategoryID=1&DetailName=Specification&MenuID=52&LanID=0" },
	{ "EPoX",		"EP-8K5A2",		"http://www.epox.com/product.asp?ID=EP-8K5A2" },
	{ "EPoX",		"EP-8RDA3+",		"http://www.epox.com/product.asp?ID=EP-8RDA3plus" },
	{ "EPoX",		"EP-BX3",		"http://www.epox.com/product.asp?ID=EP-BX3" },
	{ "GIGABYTE",		"GA-2761GXDK",		"http://www.computerbase.de/news/hardware/mainboards/amd-systeme/2007/mai/gigabyte_dtx-mainboard/" },
	{ "GIGABYTE",		"GA-7VT600",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1666" },
	{ "GIGABYTE",		"GA-7ZM",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1366" },
	{ "GIGABYTE",		"GA-K8N-SLI",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=1928" },
	{ "GIGABYTE",		"GA-M57SLI-S4",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2287" },
	{ "GIGABYTE",		"GA-M61P-S3",		"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2434" },
	{ "GIGABYTE",		"GA-MA78G-DS3H",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2800" }, /* TODO: Rev 1.x or 2.x? */
	{ "GIGABYTE",		"GA-MA78GM-S2H",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2758" }, /* TODO: Rev. 1.0, 1.1, or 2.x? */
	{ "GIGABYTE",		"GA-MA790FX-DQ6",	"http://www.gigabyte.com.tw/Products/Motherboard/Products_Spec.aspx?ProductID=2690" },
	{ "HP",			"DL145 G3",		"http://h20000.www2.hp.com/bizsupport/TechSupport/Document.jsp?objectID=c00816835&lang=en&cc=us&taskId=101&prodSeriesId=3219755&prodTypeId=15351" },
	{ "HP",			"Vectra VL400 PC",	"http://h20000.www2.hp.com/bizsupport/TechSupport/Document.jsp?objectID=c00060658&lang=en&cc=us" },
	{ "HP",			"Vectra VL420 SFF PC",	"http://h20000.www2.hp.com/bizsupport/TechSupport/Document.jsp?objectID=c00060661&lang=en&cc=us" },
	{ "IBM",		"x3455",		"http://www-03.ibm.com/systems/x/hardware/rack/x3455/index.html" },
	{ "Intel",		"D201GLY",		"http://www.intel.com/support/motherboards/desktop/d201gly/index.htm" },
	{ "IWILL",		"DK8-HTX",		"http://web.archive.org/web/20060507170150/http://www.iwill.net/product_2.asp?p_id=98" },
	{ "Kontron",		"986LCD-M",		"http://de.kontron.com/products/boards+and+mezzanines/embedded+motherboards/miniitx+motherboards/986lcdmmitx.html" },
	{ "Mitac",		"6513WU",		"http://web.archive.org/web/20050313054828/http://www.mitac.com/micweb/products/tyan/6513wu/6513wu.htm" },
	{ "MSI",		"MS-6590 (KT4 Ultra)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=502" },
	{ "MSI",		"MS-6702E (K8T Neo2-F)","http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=588" },
	{ "MSI",		"MS-6712 (KT4V)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=505" },
	{ "MSI",		"MS-7005 (651M-L)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=559" },
	{ "MSI",		"MS-7046",		"http://www.heimir.de/ms7046/" },
	{ "MSI",		"MS-7135 (K8N Neo3)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=170" },
	{ "Shuttle",		"AK31",			"http://www.motherboard.cz/mb/shuttle/AK31.htm" },
	{ "Shuttle",		"AK38N",		"http://eu.shuttle.com/en/desktopdefault.aspx/tabid-36/558_read-9889/" },
	{ "Shuttle",		"FN25",			"http://www.shuttle.eu/products/discontinued/barebones/sn25p/?0=" },
	{ "Soyo",		"SY-7VCA",		"http://www.tomshardware.com/reviews/12-socket-370-motherboards,196-15.html" },
	{ "Tyan",		"S2498 (Tomcat K7M)",	"http://www.tyan.com/archive/products/html/tomcatk7m.html" },
	{ "VIA",		"EPIA-CN",		"http://www.via.com.tw/en/products/mainboards/motherboards.jsp?motherboard_id=400" },
	{ "VIA",		"EPIA M/MII/...",	"http://www.via.com.tw/en/products/embedded/ProductDetail.jsp?productLine=1&motherboard_id=202" }, /* EPIA-MII link for now */
	{ "VIA",		"EPIA-N/NL",		"http://www.via.com.tw/en/products/embedded/ProductDetail.jsp?productLine=1&motherboard_id=221" }, /* EPIA-N link for now */
	{ "VIA",		"EPIA SP",		"http://www.via.com.tw/en/products/embedded/ProductDetail.jsp?productLine=1&motherboard_id=261" },
	{ "VIA",		"PC3500G",		"http://www.via.com.tw/en/initiatives/empowered/pc3500_mainboard/index.jsp" },
#endif
 
	/* Verified non-working boards (for now). */
#if defined(__i386__) || defined(__x86_64__)
	{ "Abit",		"IS-10",		"http://www.abit.com.tw/page/en/motherboard/motherboard_detail.php?pMODEL_NAME=IS-10&fMTYPE=Socket+478" },
	{ "ASRock",		"K7VT4A+",		"http://www.asrock.com/mb/overview.asp?Model=K7VT4A%%2b&s=" },
	{ "ASUS",		"MEW-AM",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/sock370/810/mew-am/" },
	{ "ASUS",		"MEW-VM",		"http://www.elhvb.com/mboards/OEM/HP/manual/ASUS%20MEW-VM.htm" },
	{ "ASUS",		"P3B-F",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/slot1/440bx/p3b-f/" },
	{ "ASUS",		"P5BV-M",		"ftp://ftp.asus.com.tw/pub/ASUS/mb/socket775/P5B-VM/" },
	{ "Biostar",		"M6TBA",		"ftp://ftp.biostar-usa.com/manuals/M6TBA/" },
	{ "Boser",		"HS-6637",		"http://www.boser.com.tw/manual/HS-62376637v3.4.pdf" },
	{ "DFI",		"855GME-MGF",		"http://www.dfi.com.tw/portal/CM/cmproduct/XX_cmproddetail/XX_WbProdsWindow?action=e&downloadType=&windowstate=normal&mode=view&downloadFlag=false&itemId=433" },
	{ "FIC",		"VA-502",		"ftp://ftp.fic.com.tw/motherboard/manual/socket7/va-502/" },
	{ "MSI",		"MS-6178",		"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=343" },
	{ "MSI",		"MS-7260 (K9N Neo)",	"http://www.msi.com/index.php?func=proddesc&maincat_no=1&prod_no=255" },
	{ "Soyo",		"SY-5VD",		"http://www.soyo.com/content/Downloads/163/&c=80&p=464&l=English" },
	{ "Sun",		"Fire x4540",		"http://www.sun.com/servers/x64/x4540/" },
	{ "Sun",		"Fire x4150",		"http://www.sun.com/servers/x64/x4150/" },
	{ "Sun",		"Fire x4200",		"http://www.sun.com/servers/entry/x4200/" },
	{ "Sun",		"Fire x4600",		"http://www.sun.com/servers/x64/x4600/" },
#endif

	/* Verified working laptops. */
#if defined(__i386__) || defined(__x86_64__)
	{ "Acer",		"Aspire 1520",		"http://support.acer.com/us/en/acerpanam/notebook/0000/Acer/Aspire1520/Aspire1520nv.shtml" },
	{ "Lenovo",		"3000 V100 TF05Cxx",	"http://www5.pc.ibm.com/europe/products.nsf/products?openagent&brand=Lenovo3000Notebook&series=Lenovo+3000+V+Series#viewallmodelstop" },
#endif

	/* Verified non-working laptops (for now). */
#if defined(__i386__) || defined(__x86_64__)
	{ "Acer",		"Aspire One",		NULL },
	{ "ASUS",		"Eee PC 701 4G",	"http://www.asus.com/product.aspx?P_ID=h6SPd3tEzLEsrEiS" },
	{ "Dell",		"Latitude CPi A366XT",	"http://www.coreboot.org/Dell_Latitude_CPi_A366XT" },
	{ "HP/Compaq",		"nx9010",		"http://h20000.www2.hp.com/bizsupport/TechSupport/Document.jsp?lang=en&cc=us&objectID=c00348514" },
	{ "IBM/Lenovo",		"Thinkpad T40p",	"http://www.thinkwiki.org/wiki/Category:T40p" },
	{ "IBM/Lenovo",		"240",			"http://www.stanford.edu/~bresnan//tp240.html" },
#endif

	{ NULL,			NULL,			0 },
};

/* Please keep these lists alphabetically ordered by vendor/board. */
const struct board_info_notes boards_notes[] = {
	/* Verified working boards that don't need write-enables. */
#if defined(__i386__) || defined(__x86_64__)
	{ "ASI",		"MB-5BLMP",		"Used in the IGEL WinNET III thin client." },
	{ "ASRock",		"K8S8X",		"The Super I/O isn't found on this board. See http://www.flashrom.org/pipermail/flashrom/2009-November/000937.html." },
	{ "ASUS",		"A8V-E SE",		"See http://www.coreboot.org/pipermail/coreboot/2007-October/026496.html." },
	{ "ASUS",		"M2A-VM",		"See http://www.coreboot.org/pipermail/coreboot/2007-September/025281.html." },
	{ "BCOM",		"WinNET100",		"Used in the IGEL-316 thin client." },
	{ "GIGABYTE",		"GA-7ZM",		"Works fine if you remove jumper JP9 on the board and disable the flash protection BIOS option." },
	{ "ASUS",		"M2N-E",		"If the machine doesn't come up again after flashing, try resetting the NVRAM(CMOS). The MAC address of the onboard network card will change to the value stored in the new image, so backup the old address first. See http://www.flashrom.org/pipermail/flashrom/2009-November/000879.html" },
#endif

	/* Verified working boards that DO need write-enables. */
#if defined(__i386__) || defined(__x86_64__)
	{ "Acer",		"Aspire One",		"See http://www.coreboot.org/pipermail/coreboot/2009-May/048041.html." },
#endif

	/* Verified non-working boards (for now). */
#if defined(__i386__) || defined(__x86_64__)
	{ "MSI",		"MS-6178",		"Immediately powers off if you try to hot-plug the chip. However, this does '''not''' happen if you use coreboot." },
	{ "MSI",		"MS-7260 (K9N Neo)",	"Interestingly flashrom does not work when the vendor BIOS is booted, but it ''does'' work flawlessly when the machine is booted with coreboot." },
#endif

	/* Verified working laptops. */
	/* None which need comments, yet... */

	/* Verified non-working laptops (for now). */
#if defined(__i386__) || defined(__x86_64__)
	{ "Acer",		"Aspire One",		"http://www.coreboot.org/pipermail/coreboot/2009-May/048041.html" },
	{ "ASUS",		"Eee PC 701 4G",	"It seems the chip (25X40VSIG) is behind some SPI flash translation layer (likely in the EC, the ENE KB3310)." },
	{ "Dell",		"Latitude CPi A366XT",	"The laptop immediately powers off if you try to hot-swap the chip. It's not yet tested if write/erase would work on this laptop." },
	{ "HP/Compaq",		"nx9010",		"Hangs upon '''flashrom -V''' (needs hard power-cycle then)." },
	{ "IBM/Lenovo",		"Thinkpad T40p",	"Seems to (partially) work at first, but one block/sector cannot be written which then leaves you with a bricked laptop. Maybe this can be investigated and fixed in software later." },
#endif

	{ NULL,			NULL,			0 },
};

static int url(const char *vendor, const char *board)
{
	int i;
	const struct board_info_url *b = boards_url;

        for (i = 0; b[i].vendor != NULL; i++) {
		if (!strcmp(vendor, b[i].vendor) && !strcmp(board, b[i].name))
			return i;
	}

	return -1;
}

static int note(const char *vendor, const char *board)
{
	int i;
	const struct board_info_notes *n = boards_notes;

        for (i = 0; n[i].vendor != NULL; i++) {
		if (!strcmp(vendor, n[i].vendor) && !strcmp(board, n[i].name))
			return i;
	}

	return -1;
}

static void print_supported_chipsets_wiki(int cols)
{
	int i, j, enablescount = 0, color = 1;
	const struct penable *e;

	for (e = chipset_enables; e->vendor_name != NULL; e++)
		enablescount++;

	printf("\n== Supported chipsets ==\n\nTotal amount of supported "
	       "chipsets: '''%d'''\n\n{| border=\"0\" valign=\"top\"\n| "
	       "valign=\"top\"|\n\n%s", enablescount, chipset_th);

	e = chipset_enables;
	for (i = 0, j = 0; e[i].vendor_name != NULL; i++, j++) {
		/* Alternate colors if the vendor changes. */
		if (i > 0 && strcmp(e[i].vendor_name, e[i - 1].vendor_name))
			color = !color;

		printf("|- bgcolor=\"#%s\"\n| %s || %s "
		       "|| %04x:%04x || %s\n", (color) ? "eeeeee" : "dddddd",
		       e[i].vendor_name, e[i].device_name,
		       e[i].vendor_id, e[i].device_id,
		       (e[i].status == OK) ? "{{OK}}" : "{{?3}}");

		/* Split table in 'cols' columns. */
		if (j >= (enablescount / cols + 1)) {
			printf("\n|}\n\n| valign=\"top\"|\n\n%s", chipset_th);
			j = 0;
		}
	}

	printf("\n|}\n\n|}\n");
}

static void wiki_helper(const char *heading, const char *status,
			int cols, const struct board_info boards[])
{
	int i, j, k, c, boardcount = 0, color = 1, num_notes = 0;
	const struct board_info *b;
	const struct board_info_url *u = boards_url;
	char *notes = calloc(1, 1);
	char tmp[900 + 1];

	for (b = boards; b->vendor != NULL; b++)
		boardcount++;

	printf("\n'''%s'''\n\nTotal amount of boards: '''%d'''\n\n"
	       "{| border=\"0\" valign=\"top\"\n| valign=\"top\"|\n\n%s",
	       heading, boardcount, board_th);

        for (i = 0, j = 0, b = boards; b[i].vendor != NULL; i++, j++) {
		/* Alternate colors if the vendor changes. */
		if (i > 0 && strcmp(b[i].vendor, b[i - 1].vendor))
			color = !color;

		k = url(b[i].vendor, b[i].name);
		c = note(b[i].vendor, b[i].name);

		printf("|- bgcolor=\"#%s\"\n| %s || %s%s %s%s ||"
		       " {{%s}}", (color) ? "eeeeee" : "dddddd", b[i].vendor,
		       (k != -1 && u[k].url) ? "[" : "",
		       (k != -1 && u[k].url) ? u[k].url : "",
		       b[i].name, (k != -1 && u[k].url) ? "]" : "", status);

		if (c != -1) {
			printf("<sup>%d</sup>\n", num_notes + 1);
			snprintf((char *)&tmp, 900, "<sup>%d</sup> %s<br />\n",
				 1 + num_notes++, boards_notes[c].note);
			notes = strcat_realloc(notes, (char *)&tmp);
		} else {
			printf("\n");
		}

		/* Split table in 'cols' columns. */
		if (j >= (boardcount / cols + 1)) {
			printf("\n|}\n\n| valign=\"top\"|\n\n%s", board_th);
			j = 0;
		}
	}

	printf("\n|}\n\n|}\n");

	if (num_notes > 0)
		printf("\n<small>\n%s</small>\n", notes);
	free(notes);
}

static void wiki_helper2(const char *heading, int cols)
{
	int i, j, k, boardcount = 0, color = 1;
	struct board_pciid_enable *b;
	const struct board_info_url *u = boards_url;

	for (b = board_pciid_enables; b->vendor_name != NULL; b++)
		boardcount++;

	printf("\n'''%s'''\n\nTotal amount of boards: '''%d'''\n\n"
	       "{| border=\"0\" valign=\"top\"\n| valign=\"top\"|\n\n%s",
	       heading, boardcount, board_th2);

	b = board_pciid_enables;
	for (i = 0, j = 0; b[i].vendor_name != NULL; i++, j++) {
		/* Alternate colors if the vendor changes. */
		if (i > 0 && strcmp(b[i].vendor_name, b[i - 1].vendor_name))
			color = !color;

		k = url(b[i].vendor_name, b[i].board_name);

		printf("|- bgcolor=\"#%s\"\n| %s || %s%s %s%s "
		       "|| %s%s%s%s || {{%s}}\n", (color) ? "eeeeee" : "dddddd",
		       b[i].vendor_name, (k != -1 && u[k].url) ? "[" : "",
		       (k != -1 && u[k].url) ? u[k].url : "", b[i].board_name,
		       (k != -1 && u[k].url) ? "]" : "",
		       (b[i].lb_vendor) ? "-m " : "&mdash;",
		       (b[i].lb_vendor) ? b[i].lb_vendor : "",
		       (b[i].lb_vendor) ? ":" : "",
		       (b[i].lb_vendor) ? b[i].lb_part : "",
		       (b[i].status == OK) ? "OK" : "?3");

		/* Split table in 'cols' columns. */
		if (j >= (boardcount / cols + 1)) {
			printf("\n|}\n\n| valign=\"top\"|\n\n%s", board_th2);
			j = 0;
		}
	}

	printf("\n|}\n\n|}\n");
}

static void print_supported_boards_wiki(void)
{
	printf("%s", board_intro);
	wiki_helper("Known good (worked out of the box)", "OK", 3, boards_ok);
	wiki_helper2("Known good (with write-enable code in flashrom)", 2);
	wiki_helper("Not supported (yet)", "No", 3, boards_bad);

	printf("%s", laptop_intro);
	wiki_helper("Known good (worked out of the box)", "OK", 1, laptops_ok);
	wiki_helper("Not supported (yet)", "No", 1, laptops_bad);
}
#endif

static void print_supported_chips_wiki(int cols)
{
	int i = 0, c = 1, chipcount = 0;
	struct flashchip *f, *old = NULL;
	uint32_t t;

	for (f = flashchips; f->name != NULL; f++)
		chipcount++;

	printf("\n== Supported chips ==\n\nTotal amount of supported "
	       "chips: '''%d'''\n\n{| border=\"0\" valign=\"top\"\n"
		"| valign=\"top\"|\n\n%s", chipcount, chip_th);

	for (f = flashchips; f->name != NULL; f++, i++) {
		/* Don't print "unknown XXXX SPI chip" entries. */
		if (!strncmp(f->name, "unknown", 7))
			continue;

		/* Alternate colors if the vendor changes. */
		if (old != NULL && strcmp(old->vendor, f->vendor))
			c = !c;

		t = f->tested;
		printf("|- bgcolor=\"#%s\"\n| %s || %s || %d "
		       "|| %s || {{%s}} || {{%s}} || {{%s}} || {{%s}}\n",
		       (c == 1) ? "eeeeee" : "dddddd", f->vendor, f->name,
		       f->total_size, flashbuses_to_text(f->bustype),
		       (t & TEST_OK_PROBE) ? "OK" :
		       (t & TEST_BAD_PROBE) ? "No" : "?3",
		       (t & TEST_OK_READ) ? "OK" :
		       (t & TEST_BAD_READ) ? "No" : "?3",
		       (t & TEST_OK_ERASE) ? "OK" :
		       (t & TEST_BAD_ERASE) ? "No" : "?3",
		       (t & TEST_OK_WRITE) ? "OK" :
		       (t & TEST_BAD_WRITE) ? "No" : "?3");

		/* Split table into 'cols' columns. */
		if (i >= (chipcount / cols + 1)) {
			printf("\n|}\n\n| valign=\"top\"|\n\n%s", chip_th);
			i = 0;
		}

		old = f;
	}

	printf("\n|}\n\n|}\n");
}

static void print_supported_pcidevs_wiki(struct pcidev_status *devs)
{
	int i = 0;
	static int c = 0;

	/* Alternate colors if the vendor changes. */
	c = !c;

	for (i = 0; devs[i].vendor_name != NULL; i++) {
		printf("|- bgcolor=\"#%s\"\n| %s || %s || "
		       "%04x:%04x || {{%s}}\n", (c) ? "eeeeee" : "dddddd",
		       devs[i].vendor_name, devs[i].device_name,
		       devs[i].vendor_id, devs[i].device_id,
		       (devs[i].status == NT) ? "?3" : "OK");
	}
}

void print_supported_wiki(void)
{
	time_t t = time(NULL);

	printf(wiki_header, ctime(&t), flashrom_version);
#if INTERNAL_SUPPORT == 1
	print_supported_chips_wiki(2);
	print_supported_chipsets_wiki(3);
	print_supported_boards_wiki();
#endif
	printf("%s", programmer_section);
#if NIC3COM_SUPPORT == 1
	print_supported_pcidevs_wiki(nics_3com);
#endif
#if NICREALTEK_SUPPORT == 1
	print_supported_pcidevs_wiki(nics_realtek);
	print_supported_pcidevs_wiki(nics_realteksmc1211);
#endif
#if GFXNVIDIA_SUPPORT == 1
	print_supported_pcidevs_wiki(gfx_nvidia);
#endif
#if DRKAISER_SUPPORT == 1
	print_supported_pcidevs_wiki(drkaiser_pcidev);
#endif
#if SATASII_SUPPORT == 1
	print_supported_pcidevs_wiki(satas_sii);
#endif
#if ATAHPT_SUPPORT == 1
	print_supported_pcidevs_wiki(ata_hpt);
#endif
	printf("\n|}\n");
}

