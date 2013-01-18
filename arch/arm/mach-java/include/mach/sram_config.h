/***************************************************************************
*
* (c)1999-2011 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to
* you under the terms of the GNU General Public License version 2,
* available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
***************************************************************************/
#ifndef _SRAM_DEF_HAWAII_H_
#define _SRAM_DEF_HAWAII_H_

/*****************************************************************************/
/*                                                                           */
/*    HAWAII SRAM MEMEORY MAP     		                             */
/*                                                                           */
/*****************************************************************************/
/*      name                   ROM address     RAM address     size          */
/*      ================       ==========      ==========      ==========    */
#if defined(SRAM_BASE_ADDRESS)
#define SRAM_BASE_ADDR                          SRAM_BASE_ADDRESS
#else
#define SRAM_BASE_ADDR                          0x34040000
#endif
#if defined(SRAM_SIZE)
#define INTERNAL_SRAM_SIZE                      SRAM_SIZE
#else
#define INTERNAL_SRAM_SIZE                      0x012000
#endif

/* ------------------------------------------------------------------------
 *    SRAM usage in ROM
 * ------------------------------------------------------------------------
 */
#define SRAM_SECURE_RAM_BASE			SRAM_BASE_ADDR
						/* 0x34040000 -- 0x3404DFFF */
#define SRAM_SECURE_RAM_SIZE			0xE000

/* Publc RAM can be used after ROM boot up */
#define SRAM_PUBLIC_RAM_BASE			(SRAM_SECURE_RAM_BASE \
						+ SRAM_SECURE_RAM_SIZE)
						/* 0x3404e000 -- 0x34052000 */
#define SRAM_PUBLIC_RAM_SIZE			0x4000

/* ------------------------------------------------------------------------
 *    SRAM usage in Platfrom SWs
 * ------------------------------------------------------------------------
 */
#define SRAM_ROM_DORMANT_EXIT_BASE		(SRAM_PUBLIC_RAM_BASE)
						/* 0x3404e000 -- 0x3404E3FF */
#define SRAM_ROM_DORMANT_EXIT_SIZE		0x400
#define SRAM_SHARED_BUF_BASE			(SRAM_ROM_DORMANT_EXIT_BASE \
						+ SRAM_ROM_DORMANT_EXIT_SIZE)
						/* 0x3404e400 -- 0x3404E7FF */
#define SRAM_SHARED_BUF_SIZE			0x400

#define SRAM_DSP_AREA1_BASE			(SRAM_SHARED_BUF_BASE \
						+ SRAM_SHARED_BUF_SIZE)
						/* 0x3404E800 -- 0x340507ff */
#define SRAM_DSP_AREA1_SIZE			0x2000

#define SRAM_SROM_BOOT_TRACE_BASE		(SRAM_DSP_AREA1_BASE \
						+ SRAM_DSP_AREA1_SIZE)
						/* 0x34050800 -- 0x3405081F */
#define SRAM_SROM_BOOT_TRACE_SIZE		0x20

#define SRAM_DSP_AREA2_BASE			(SRAM_SROM_BOOT_TRACE_BASE \
						+ SRAM_SROM_BOOT_TRACE_SIZE)
						/* 0x34050820 -- 0x34051F7f */
#define SRAM_DSP_AREA2_SIZE			0x1760

/* ROM doesn't touch this area  : Value can be retained thorugh reset*/
#define SRAM_RST_REASON_BASE			(SRAM_DSP_AREA2_BASE \
						+ SRAM_DSP_AREA2_SIZE)
						/* 0x34051F80 -- 0x34051F83 */
#define SRAM_RST_REASON_SIZE			0x04

#define SRAM_AXI_CP_SETTING_BASE		(SRAM_RST_REASON_BASE \
						+ SRAM_RST_REASON_SIZE)
						/* 0x34051F84 -- 0x34051F87 */
#define SRAM_AXI_CP_SETTING_SIZE		0x04

#define SRAM_NON_INIT_FREE0_BASE		(SRAM_AXI_CP_SETTING_BASE \
						+ SRAM_AXI_CP_SETTING_SIZE)
						/* 0x34051F88 -- 0x34051F8B */
#define SRAM_NON_INIT_FREE0_SIZE		0x04

#define SRAM_AP_ONLY_BOOT_BASE			(SRAM_NON_INIT_FREE0_BASE \
						+ SRAM_NON_INIT_FREE0_SIZE)
						/* 0x34051F8C -- 0x34051F8F */
#define SRAM_AP_ONLY_BOOT_SIZE			0x04

#define SRAM_NON_INIT_FREE1_BASE		(SRAM_AP_ONLY_BOOT_BASE \
						+ SRAM_AP_ONLY_BOOT_SIZE)
						/* 0x34051F90 -- 0x34051F9F */
#define SRAM_NON_INIT_FREE1_SIZE		0x10

/* ROM doesn't touch this area  : Value can be retained thorugh reset*/
#define SRAM_OTP_INFO_BASE			(SRAM_NON_INIT_FREE1_BASE \
						+ SRAM_NON_INIT_FREE1_SIZE)
						/* 0x34051FA0 -- 0x34051FAF */
#define SRAM_OTP_INFO_SIZE			0x10

#define SRAM_NON_INIT_FREE2_BASE		(SRAM_OTP_INFO_BASE \
						+ SRAM_OTP_INFO_SIZE)
						/* 0x34051FB0 -- 0x34051FBF */
#define SRAM_NON_INIT_FREE2_SIZE		0x10

#define SRAM_RAMDUMP_INFO_BASE			(SRAM_NON_INIT_FREE2_BASE \
						+ SRAM_NON_INIT_FREE2_SIZE)
						/* 0x34051FC0 -- 0x34051FC3 */
#define SRAM_RAMDUMP_INFO_SIZE			0x04

#define SRAM_NON_INIT_FREE3_BASE		(SRAM_RAMDUMP_INFO_BASE \
						+ SRAM_RAMDUMP_INFO_SIZE)
						/* 0X34051FC4 -- 0X34051FDF */
#define SRAM_NON_INIT_FREE3_SIZE		0x1C

#define SRAM_CUST_REBOOT_REASON_BASE		(SRAM_NON_INIT_FREE3_BASE \
						+ SRAM_NON_INIT_FREE3_SIZE)
						/* 0x34051FE0 -- 0x34051FFF */
#define SRAM_CUST_REBOOT_REASON_SIZE		0x20

#endif /* _SRAM_DEF_HAWAII_H_ */
