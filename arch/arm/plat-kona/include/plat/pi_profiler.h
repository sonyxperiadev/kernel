/*****************************************************************************
*
* @file pi_profiler.h
*
* kona Power Island profiler
*
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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
#ifndef _PI_PROFILER_H
#define _PI_PROFILER_H

#include "profiler.h"

#define PI_ON_CNT_OVERFLOW_VAL		(0x1<<30)


#define DEFINE_PI_PROFILER(name)	\
	static struct pi_profiler DEFINE_PROFILER(pi, name)

#define PI_PROFILER(name)	DEFINE_PROFILER(pi, name)
#define PROFILER_PI_DIR_NAME	("pi")

struct pi_profiler;

struct gen_pi_prof_ops {
	int (*clear_counter) (struct pi_profiler *pi_profiler);
};

/**
 * Kona Power Island profiler
 */
struct pi_profiler {
	struct profiler profiler;
	void __iomem *pi_prof_addr_base;
	u32 pi_id;
	u32 counter_offset;
	u32 counter_clear_offset;
	u32 counter_en_mask;
	u32 counter_mask;
	u32 counter_clear_mask;
	u32 counter_en_shift;
	u32 counter_shift;
	u32 counter_clear_shift;
	u32 overflow_mask;

	struct gen_pi_prof_ops *pi_gen_prof_ops;
};

int pi_profiler_register(struct pi_profiler *profiler);
int pi_profiler_unregister(struct pi_profiler *profiler);
int pi_profiler_init(struct dentry *prof_root_dir);
#endif
