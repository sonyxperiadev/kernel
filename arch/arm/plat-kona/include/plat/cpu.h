/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#ifndef __CPU_H__
#define __CPU_H__

/* CPU revisions */
#define RHEA_CHIP_REV_B0	0x1
#define RHEA_CHIP_REV_B1	0x2
#define RHEA_CHIP_REV_B2	0x3

/* mach specific implementation */
int get_chip_rev_id(void);

#define cpu_is_rhea_B0()  (get_chip_rev_id() == RHEA_CHIP_REV_B0 ? 1 : 0)
#define cpu_is_rhea_B1()  (get_chip_rev_id() == RHEA_CHIP_REV_B1 ? 1 : 0)
#define cpu_is_rhea_B2()  (get_chip_rev_id() == RHEA_CHIP_REV_B2 ? 1 : 0)

#endif /* __CPU_H__ */
