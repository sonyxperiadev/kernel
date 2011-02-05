/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/cfg_global_defines.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef CFG_GLOBAL_DEFINES_H
#define CFG_GLOBAL_DEFINES_H

/* CPU */
#define   ARMEB  1
#define   MIPS32 2
#define   ARM9   3
#define   ARM11  4

/* CHIP */
#define BCM1103 1
#define BCM1161 2
#define BCM1104 3
#define BCM1191 4
#define BCM2153 12
#define BCM2820 6

#define BCM2826 8
#define SMEAGOL 9
#define GOLLUM   10

/* CFG_GLOBAL_BCM116X_TYPE */
#define CFG_GLOBAL_BCM1160    1
#define CFG_GLOBAL_BCM1161    2
#define CFG_GLOBAL_BCM2153    3

/* CFG_GLOBAL_CHIP_FAMILY types */
#define CFG_GLOBAL_CHIP_FAMILY_NONE        0
#define CFG_GLOBAL_CHIP_FAMILY_BCM116X     2
#define CFG_GLOBAL_CHIP_FAMILY_BCMRING     4

/* CFG_GLOBAL_ROOT_FILE_SYSTEM */
#define JFFS2_RFS      1
#define CRAMFS_RFS     2
#define INITRAMFS      3

#endif
