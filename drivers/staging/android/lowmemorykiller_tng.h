/*
 *  lowmemorykiller_tng interface
 *
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
#ifndef __LOWMEMORYKILLER_TNG_H
#define __LOWMEMORYKILLER_TNG_H
extern short lowmem_adj[];
extern int lowmem_minfree[];
extern int oom_reaper;

/* basic kill reason */
#define LMK_VMPRESSURE		(0x1)
#define LMK_SHRINKER_SCAN	(0x2)
#define LMK_OOM			(0x4)
#define LMK_SHRINKER_COUNT	(0x8)
/* calc option reason */
#define LMK_LOW_RESERVE		(0x0100)
#define LMK_CANT_SWAP		(0x0200)

/* function in lowmemorykiller.c */
int lowmem_min_param_size(void);
int adjust_minadj(short *min_score_adj);
/* please dont use tune_lmk_param directly without good reason */
void tune_lmk_param(int *other_free, int *other_file,
		    struct shrink_control *sc);
void tune_lmk_param_mask(int *other_free, int *other_file, gfp_t mask);
void __init lowmem_init_tng(struct shrinker *shrinker);
void balance_cache(unsigned long vmpressure);
void mark_lmk_victim(struct task_struct *tsk);
#endif
