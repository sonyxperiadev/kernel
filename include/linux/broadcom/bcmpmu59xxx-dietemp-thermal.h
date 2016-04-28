/******************************************************************************/
/*                                                                            */
/*  Copyright 2013  Broadcom Corporation                                      */
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
#ifndef __DIETEMP_THERMAL_H__
#define __DIETEMP_THERMAL_H__

#include <linux/thermal.h>

#define MAX_CHARGE_CURRENT	1755
#define MIN_CHARGE_CURRENT	0


struct bcmpmu_dietemp_trip {
	u32 temp;
	u32 max_curr;
	enum thermal_trip_type type;
};

struct bcmpmu_dietemp_temp_zones {
	struct bcmpmu_dietemp_trip *trips;
	int trip_cnt;
};

struct bcmpmu_dietemp_pdata {
	u32 poll_rate_ms;
	int hysteresis;
	u8 temp_adc_channel;
	u8 temp_adc_req_mode;
	struct bcmpmu_dietemp_temp_zones *dtzones;
};

#endif
