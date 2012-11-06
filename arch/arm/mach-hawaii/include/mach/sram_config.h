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
#define SRAM_SECURE_RAM_SIZE			0xE000

/* Publc RAM can be used after ROM boot up */
#define SRAM_PUBLIC_RAM_BASE              (SRAM_SECURE_RAM_BASE + SRAM_SECURE_RAM_SIZE)
#define SRAM_PUBLIC_RAM_SIZE              0x4000

/* ------------------------------------------------------------------------
 *    SRAM usage in Platfrom SWs
 * ------------------------------------------------------------------------
 */
#define SRAM_ROM_DORMANT_EXIT_BASE		(SRAM_PUBLIC_RAM_BASE)
#define SRAM_ROM_DORMANT_EXIT_SIZE		0x1700
#define SRAM_SHARED_BUF_BASE			(SRAM_ROM_DORMANT_EXIT_BASE + 0x400)
#define SRAM_SHARED_BUF_SIZE			0x400

#define SRAM_WAKEUP_ADDR_EN_BASE		(SRAM_ROM_DORMANT_EXIT_BASE \
						+ SRAM_ROM_DORMANT_EXIT_SIZE)
#define SRAM_WAKEUP_ADDR_EN_SIZE		0x80

#define SRAM_RST_REASON_BASE			(SRAM_WAKEUP_ADDR_EN_BASE \
						+ SRAM_WAKEUP_ADDR_EN_SIZE)
#define SRAM_RST_REASON_SIZE			0x04

/* ROM doesn't touch this area  : Value can be retained thorugh reset*/
#define SRAM_NON_INIT_RST_BASE			(SRAM_RST_REASON_BASE \
						+ SRAM_RST_REASON_SIZE)
#define SRAM_NON_INIT_RST_SIZE			0x14

#define SRAM_AP_ONLY_BOOT_BASE			(SRAM_NON_INIT_RST_BASE \
						+ SRAM_NON_INIT_RST_SIZE)
#define SRAM_AP_ONLY_BOOT_SIZE			0x04

#define SRAM_AXI_CP_SETTING_BASE		(SRAM_AP_ONLY_BOOT_BASE \
						+ SRAM_AP_ONLY_BOOT_SIZE)
#define SRAM_AXI_CP_SETTING_SIZE		0x04

#define SRAM_OTP_INFO_BASE			(SRAM_AXI_CP_SETTING_BASE \
						+ SRAM_AXI_CP_SETTING_SIZE)
#define SRAM_OTP_INFO_SIZE			0x20

/* ROM doesn't touch this area  : Value can be retained thorugh reset*/
#define SRAM_NON_INIT_BASE			(SRAM_OTP_INFO_BASE \
						+ SRAM_OTP_INFO_SIZE)
#define SRAM_NON_INIT_SIZE			0x20

#define SRAM_CUST_REBOOT_REASON_BASE		(SRAM_NON_INIT_BASE \
						+ SRAM_NON_INIT_SIZE)
#define SRAM_CUST_REBOOT_REASON_SIZE		0x20

#define SRAM_RAMDUMP_INFO_BASE			(SRAM_CUST_REBOOT_REASON_BASE)
#define SRAM_RAMDUMP_INFO_SIZE			0x04

#endif /* _SRAM_DEF_HAWAII_H_ */
