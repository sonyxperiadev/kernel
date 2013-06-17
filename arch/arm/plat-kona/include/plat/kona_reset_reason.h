
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

#define POWERON_RESET	0x1
#define AP_ONLY_BOOT	0x4
#define CHARGING_STATE	0x3
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
