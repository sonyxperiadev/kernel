
/****************************************************************************
*
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef __KONA_RESET_REASON_H__
#define __KONA_RESET_REASON_H__

/*
 * IMPORTANT NOTE:
 * ---------------
 * Please note that the below values are not bit masks. They are
 * int values instead. So when checking for a particular reset reason, check
 * the whole 32-bit value instead of doing (val & 0xf) and then comparing with
 * below values. This is because we have 32 bit register at 0x34051F80 defined
 * for SRAM_RST_REASON.
 */
#define RST_REASON_MASK 0xFFFFFFFF

#define POWERON_RESET	0x1
#define CHARGING_STATE	0x3
#define AP_ONLY_BOOT	0x4
#define BOOTLOADER_BOOT	0x5
#define RECOVERY_BOOT   0x6

void do_set_poweron_reset_boot(void);
void do_set_bootloader_boot(void);
void do_set_recovery_boot(void);
void do_set_ap_only_boot(void);
void do_clear_ap_only_boot(void);
unsigned int is_ap_only_boot(void);
extern unsigned int hard_reset_reason;
unsigned int is_charging_state(void);
int is_soft_reset(void);
#endif
