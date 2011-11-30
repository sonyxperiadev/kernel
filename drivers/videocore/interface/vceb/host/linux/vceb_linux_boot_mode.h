/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef VCEB_LINUX_BOOT_MODE_H_
#define VCEB_LINUX_BOOT_MODE_H_

#define VCEB_MAX_BOOT_MODE_LEN  16

// A string which describes how the videocore was booted.
// If the string contains the 4 character sequence "boot" then its assumed that 
// the bootloader initialized the videocore.
// 
// Other values:
//      jtag - The videocore was initialized via jtag
//      skip - The user wishes to skip videocore initialization
//      none - The videocore has not yet been initialized
extern char vceb_boot_mode[VCEB_MAX_BOOT_MODE_LEN];

int  vceb_skip_boot( void );
int  vceb_jtag_boot( void );
int vceb_bootloader_boot( void );

#endif /* VCEB_LINUX_WRAPPER_H_ */

