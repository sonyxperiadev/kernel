/******************************************************************************/
/*                                                                            */
/*  Copyright 2012  Broadcom Corporation                                      */
/*                                                                            */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed    */
/*     to you under the terms of the GNU General Public License version 2     */
/*     (the GPL), available at                                                */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and     */
/*     to copy and distribute the resulting executable under terms of your    */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module. An     */
/*     independent module is a module which is not derived from this          */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine      */
/*     this software in any way with any other Broadcom software provided     */
/*     under a license other than the GPL, without Broadcom's express prior   */
/*     written consent.                                                       */
/*                                                                            */
/******************************************************************************/
#ifndef __KONA_SECURE_MEMC_SETTINGS_H__
#define __KONA_SECURE_MEMC_SETTINGS_H__

#define NUM_OF_MEMC_PORTS		4
#define NUM_OF_GROUPS			8
#define NUM_OF_REGIONS			8
#define NUM_OF_ALLOWED_MASTERS		4

#define KONA_SECURE_MEMC_IRQ		92

struct kona_secure_memc_pdata {
	u32 kona_s_memc_base;
	u32 num_of_memc_ports;
	u32 num_of_groups;
	u32 num_of_regions;
	u32 cp_area_start;
	u32 cp_area_end;
	u32 ap_area_start;
	u32 ap_area_end;
	u32 ddr_start;
	u32 ddr_end;
	char *masters[NUM_OF_ALLOWED_MASTERS];
	char *default_master_map[NUM_OF_MEMC_PORTS];
	u32 flags;
};

#endif /*__KONA_SECURE_MEMC_SETTINGS_H__*/



