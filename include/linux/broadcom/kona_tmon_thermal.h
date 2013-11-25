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
#ifndef __KONA_TMON_THERMAL_H__
#define __KONA_TMON_THERMAL_H__

#include <linux/thermal.h>

#if defined(DEBUG)
#define tmon_dbg printk
#else
#define tmon_dbg(log_typ, format...)			\
	do {								\
		if ((log_typ) == TMON_LOG_ERR ||		\
				(log_typ) == TMON_LOG_ALERT)	\
			pr_err("[TMON]:"format);			\
		else if (tmon_dbg_mask & (log_typ))		\
			pr_info("[TMON]:"format);			\
	} while (0)
#endif

enum {
	TMON_PVTMON = 1,
	TMON_VTMON = (1 << 1),
	TMON_SUSPEND_POWEROFF = (1 << 2),
	TMON_RESET_ENABLE = (1 << 3),
};

enum {
	TEMP_RAW,
	TEMP_CELCIUS,
};

struct kona_tmon_trip {
	u32 temp;
	u32 max_freq;
	u32 max_brightness;
	enum thermal_trip_type type;
};

struct kona_tmon_pdata {
	void __iomem *base_addr;
	void __iomem *chipreg_addr;
	int irq;
	u32 poll_rate_ms;
	u32 interval_ms;
	int hysteresis;
	int flags;
	char const *tmon_apb_clk;
	char const *tmon_1m_clk;
	int falling; /* fall back temperature offset*/
	int shutdown_temp;
	int trip_cnt;
	struct kona_tmon_trip *trips;
};

long tmon_get_current_temp(bool celcius, bool avg);

#endif /*__KONA_TMON_THERMAL_H__*/
