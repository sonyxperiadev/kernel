/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef KEYMAP_SETTINGS_H
#define KEYMAP_SETTINGS_H

#define HW_DEFAULT_KEYMAP \
{ \
   { 0x01, 115 }, /* VOL_UP */ \
   { 0x11, 114 }, /* VOL_DOWN */ \
   { 0x21, 102 }, /* HOME */ \
   { 0x31, 158 }, /* BACK */ \
   { 0x41, 59  }, /* MENU */ \
   { 0x02, 108 }, /* DOWN */ \
   { 0x12, 105 }, /* LEFT */ \
   { 0x22, 103 }, /* UP */ \
   { 0x32, 232 }, /* OK (CENTER) */ \
   { 0x42, 106 }, /* RIGHT */ \
   { 0x03, 231 }, /* DIAL */ \
   { 0x13, 107 }, /* HANG_UP */ \
   { 0x23, 127 }, /* SEARCH */ \
}

#define HW_DEFAULT_POWEROFF { } /* Power button uses gpio-keys driver */

#endif
