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
    { 0x01, '1' }, { 0x11, '2' }, { 0x21, '3' }, { 0x31, 'a' }, { 0x41, '(' }, \
    { 0x02, '4' }, { 0x12, '5' }, { 0x22, '6' }, { 0x32, 'b' }, { 0x42, '>' }, \
    { 0x03, '7' }, { 0x13, '8' }, { 0x23, '9' }, { 0x33, 'c' }, { 0x43, '<' }, \
    { 0x04, '*' }, { 0x14, '0' }, { 0x24, '#' }, { 0x34, 'd' }, { 0x44, 'S' }, \
}

#define HW_DEFAULT_POWEROFF { '*', '8', '6' }

#endif
