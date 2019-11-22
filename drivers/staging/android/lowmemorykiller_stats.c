/*
 *  lowmemorykiller_stats
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
/* This code is bookkeeping of statistical information
 * from lowmemorykiller and provide a node in proc "/proc/lmkstats".
 */

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "lowmemorykiller_stats.h"

struct lmk_stats {
	atomic_long_t scans; /* counter as in shrinker scans */
	atomic_long_t kills; /* the number of sigkills sent */
	atomic_long_t waste; /* the number of expensive calls that did
			      * not lead to anything
			      */
	atomic_long_t timeout; /* counter for shrinker calls that needed
				* to be cancelled due to pending kills
				*/
	atomic_long_t count; /* number of shrinker count calls */
	atomic_long_t scan_busy; /* mutex held */
	atomic_long_t no_kill; /* mutex held */
	atomic_long_t busy;
	atomic_long_t error;
	atomic_long_t zero_count;
	atomic_long_t oom_count;
	atomic_long_t oom_kill_count;
	atomic_long_t morgue_count;
	atomic_long_t balance_kill;
	atomic_long_t balance_waste;
	atomic_long_t mem_error;

	atomic_long_t unknown; /* internal */
} st;

void lmk_inc_stats(int key)
{
	switch (key) {
	case LMK_SCAN:
		atomic_long_inc(&st.scans);
		break;
	case LMK_KILL:
		atomic_long_inc(&st.kills);
		break;
	case LMK_WASTE:
		atomic_long_inc(&st.waste);
		break;
	case LMK_TIMEOUT:
		atomic_long_inc(&st.timeout);
		break;
	case LMK_COUNT:
		atomic_long_inc(&st.count);
		break;
	case LMK_BUSY:
		atomic_long_inc(&st.busy);
		break;
	case LMK_ERROR:
		atomic_long_inc(&st.error);
		break;
	case LMK_NO_KILL:
		atomic_long_inc(&st.no_kill);
		break;
	case LMK_ZERO_COUNT:
		atomic_long_inc(&st.zero_count);
		break;
	case LMK_OOM_COUNT:
		atomic_long_inc(&st.oom_count);
		break;
	case LMK_OOM_KILL_COUNT:
		atomic_long_inc(&st.oom_kill_count);
		break;
	case LMK_MORGUE_COUNT:
		atomic_long_inc(&st.morgue_count);
		break;
	case LMK_BALANCE_KILL:
		atomic_long_inc(&st.balance_kill);
		break;
	case LMK_BALANCE_WASTE:
		atomic_long_inc(&st.balance_waste);
		break;
	case LMK_MEM_ERROR:
		atomic_long_inc(&st.mem_error);
		break;
	default:
		atomic_long_inc(&st.unknown);
		break;
	}
}

static int lmk_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "kill: %ld\n", atomic_long_read(&st.kills));
	seq_printf(m, "scan: %ld\n", atomic_long_read(&st.scans));
	seq_printf(m, "waste: %ld\n", atomic_long_read(&st.waste));
	seq_printf(m, "timeout: %ld\n", atomic_long_read(&st.timeout));
	seq_printf(m, "count: %ld\n", atomic_long_read(&st.count));
	seq_printf(m, "busy: %ld\n", atomic_long_read(&st.busy));
	seq_printf(m, "error: %ld\n", atomic_long_read(&st.error));
	seq_printf(m, "no kill: %ld\n", atomic_long_read(&st.no_kill));
	seq_printf(m, "zero: %ld\n", atomic_long_read(&st.zero_count));
	seq_printf(m, "oom: %ld\n", atomic_long_read(&st.oom_count));
	seq_printf(m, "oom kill: %ld\n", atomic_long_read(&st.oom_kill_count));
	seq_printf(m, "morgue: %ld\n", atomic_long_read(&st.morgue_count));
	seq_printf(m, "balance kill: %ld\n",
		   atomic_long_read(&st.balance_kill));
	seq_printf(m, "balance waste: %ld\n",
		   atomic_long_read(&st.balance_waste));
	seq_printf(m, "mem error: %ld\n",
		   atomic_long_read(&st.mem_error));
	seq_printf(m, "unknown: %ld (internal)\n",
		   atomic_long_read(&st.unknown));

	return 0;
}

static int lmk_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, lmk_proc_show, PDE_DATA(inode));
}

static const struct file_operations lmk_proc_fops = {
	.open		= lmk_proc_open,
	.read		= seq_read,
	.release	= single_release
};

int __init init_procfs_lmk(void)
{
	proc_create_data(LMK_PROCFS_NAME, 0444, NULL, &lmk_proc_fops, NULL);
	return 0;
}

void exit_procfs_lmk(void)
{
	remove_proc_entry(LMK_PROCFS_NAME, NULL);
}
