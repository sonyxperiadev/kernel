/*
 *  lowmemorykiller_stats interface
 *
 *  Author: Peter Enderborg <peter.enderborg@sonymobile.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/*
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __LOWMEMORYKILLER_STATS_H
#define __LOWMEMORYKILLER_STATS_H

enum  lmk_kill_stats {
	LMK_SCAN = 1,
	LMK_KILL = 2,
	LMK_WASTE = 3,
	LMK_TIMEOUT = 4,
	LMK_COUNT = 5,
	LMK_SCAN_BUSY = 6,
	LMK_NO_KILL = 7,
	LMK_BUSY = 8,
	LMK_ERROR = 9,
	LMK_ZERO_COUNT = 10,
	LMK_OOM_COUNT = 11,
	LMK_OOM_KILL_COUNT = 12,
	LMK_BALANCE_KILL = 13,
	LMK_BALANCE_WASTE = 14,
	LMK_MORGUE_COUNT = 15,
	LMK_MEM_ERROR = 16,
};

#define LMK_PROCFS_NAME "lmkstats"

#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_STATS
void lmk_inc_stats(int key);
int __init init_procfs_lmk(void);
void exit_procfs_lmk(void);
#else
static inline void lmk_inc_stats(int key) { return; };
static inline int __init init_procfs_lmk(void) { return 0; };
static inline void exit_procfs_lmk(void) { return; };
#endif

#endif
