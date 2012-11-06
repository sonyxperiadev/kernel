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
#ifndef _SRAM_DEF_RHEA_H_
#define _SRAM_DEF_RHEA_H_

/*****************************************************************************/
/*                                                                           */
/*    RHEA SRAM MEMEORY MAP     		                             */
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
#define INTERNAL_SRAM_SIZE                      0xC000
#endif

/* ------------------------------------------------------------------------
 *    SRAM usage in ROM
 * ------------------------------------------------------------------------
 */
#define SRAM_SECURE_RAM_BASE			SRAM_BASE_ADDR
#define SRAM_SECURE_RAM_SIZE			0xA000

#define SRAM_DSP_RAM_BASE			(SRAM_SECURE_RAM_BASE + \
							SRAM_SECURE_RAM_SIZE)
#define SRAM_DSP_RAM_SIZE			0x0C00

/* Publc RAM can be used after ROM boot up */
#define SRAM_PUBLIC_RAM_BASE			(SRAM_DSP_RAM_BASE + \
							SRAM_DSP_RAM_SIZE)
#define SRAM_PUBLIC_RAM_SIZE			0x1400

/* ------------------------------------------------------------------------
 *    SRAM usage in Platfrom SWs
 * ------------------------------------------------------------------------
 */
#define SRAM_RSVD_BOOTLDR_BASE			(SRAM_PUBLIC_RAM_BASE + \
							SRAM_PUBLIC_RAM_SIZE)
#define SRAM_RSVD_BOOTLDR_SIZE			0x1380

#define SRAM_FREE_SPACE1_BASE			(SRAM_RSVD_BOOTLDR_BASE + \
							SRAM_RSVD_BOOTLDR_SIZE)
#define SRAM_FREE_SPACE1_SIZE			0x20

#define SRAM_RST_REASON_BASE			(SRAM_FREE_SPACE1_BASE + \
							SRAM_FREE_SPACE1_SIZE)
#define SRAM_RST_REASON_SIZE			0x04

#define SRAM_AP_ONLY_BOOT_BASE			(SRAM_FREE_SPACE1_BASE \
							+ SRAM_FREE_SPACE1_SIZE)
#define SRAM_AP_ONLY_BOOT_SIZE			0x04

#define SRAM_AXI_CP_SETTING_BASE		(SRAM_AP_ONLY_BOOT_BASE \
							+ SRAM_AP_ONLY_BOOT_SIZE)
#define SRAM_AXI_CP_SETTING_SIZE		0x04

#define SRAM_FREE_SPACE2_BASE			(SRAM_AXI_CP_SETTING_BASE + \
							SRAM_AXI_CP_SETTING_SIZE)
#define SRAM_FREE_SPACE2_SIZE			0x08

#define SRAM_OTP_INFO_BASE			(SRAM_FREE_SPACE2_BASE \
							+ SRAM_FREE_SPACE2_SIZE)
#define SRAM_OTP_INFO_SIZE			0x10

#define SRAM_MEMC_WK_BASE			(SRAM_OTP_INFO_BASE \
							+ SRAM_OTP_INFO_SIZE)
#define SRAM_MEMC_WK_SIZE			0x0C

#define SRAM_RAMDUMP_INFO_BASE			(SRAM_MEMC_WK_BASE + \
							SRAM_MEMC_WK_SIZE)
#define SRAM_RAMDUMP_INFO_SIZE			0x04

#define SRAM_FREE_SPACE3_BASE			(SRAM_RAMDUMP_INFO_BASE + \
							SRAM_RAMDUMP_INFO_SIZE)
#define SRAM_FREE_SPACE3_SIZE			0x20

#define SRAM_CUST_REBOOT_REASON_BASE		(SRAM_NON_INIT_BASE \
							+ SRAM_NON_INIT_SIZE)
#define SRAM_CUST_REBOOT_REASON_SIZE		0x20

#endif /* _SRAM_DEF_RHEA_H_ */
