/***********************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software
  license agreement governing use of this software, this software
  is licensed to you under the terms of the GNU General Public
  License version 2, available at
  http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you
combine this software in any way with any other Broadcom software
provided under a license other than the GPL, without Broadcom's
express prior written consent.
 **********************************************************************/

#ifndef _MM_DVFS_H_
#define _MM_DVFS_H_

#include "mm_common.h"

#ifdef CONFIG_MEMC_DFS
#include <plat/kona_memc.h>
#endif

#define NUM_DVFS_PROF_SAMPLES 8

struct dvfs_update {
	struct work_struct work;
	u64 param;
	struct _mm_dvfs *mm_dvfs;
};
#undef SCHEDULER_WORK
#define SCHEDULER_WORK(core, work)\
	queue_work_on(0, core->mm_common_ifc->single_wq, work);
#define DEFINE_DEBUGFS_HANDLER(name) \
	static void mm_dvfs_update_##name(struct work_struct *work) \
{ \
	struct dvfs_update *update = container_of(work, \
			struct dvfs_update, \
			work); \
	struct _mm_dvfs *mm_dvfs = update->mm_dvfs; \
	mm_dvfs->dvfs.name = update->param; \
	SCHEDULER_WORK(mm_dvfs, &(mm_dvfs->dvfs_work)); \
} \
static int mm_dvfs_debugfs_##name##_get(void *root, u64 *param) \
{ \
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)root; \
	*param = mm_dvfs->dvfs.name; \
	return 0; \
} \
static int mm_dvfs_debugfs_##name##_set(void *root, u64 param) \
{ \
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)root; \
	struct dvfs_update update; \
	update.param = param; \
	update.mm_dvfs = mm_dvfs; \
	INIT_WORK(&(update.work), mm_dvfs_update_##name); \
	SCHEDULER_WORK(mm_dvfs, &(update.work)); \
	flush_work_sync(&(update.work)); \
	return 0; \
} \
DEFINE_SIMPLE_ATTRIBUTE(mm_dvfs_debugfs_##name, \
		mm_dvfs_debugfs_##name##_get, \
		mm_dvfs_debugfs_##name##_set, #name" : %llu\n");


#define CREATE_DEBUGFS_FILE(root, var, name, dir) \
{	root->var##_##name = debugfs_create_file(#name, \
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP), \
		root->dir, root, &mm_dvfs_debugfs_##var##_##name); } \

struct mm_prof_buff {
	dvfs_mode_e current_mode;
	dvfs_mode_e requested_mode;
	int curr_usage;
	int switch_case;
	int time;
	int data[NUM_DVFS_PROF_SAMPLES];
};
#define BUFF_SIZE 16
struct _mm_dvfs {
	struct _mm_common_ifc *mm_common_ifc;
	struct notifier_block mm_fmwk_notifier_blk;

	unsigned int prof_history[NUM_DVFS_PROF_SAMPLES];

	/* for job based profiling, 'n' jobs take how many microsecs */
	struct dentry *dvfs_dir;
	struct dentry *__on;
	struct dentry *__ts;
	struct dentry *__dvfsprof;

	struct dentry *eco_ns_high;
	struct dentry *nor_ns_high;
	struct dentry *tur_ns_high;
	struct dentry *nor_ns_low;
	struct dentry *tur_ns_low;
	struct dentry *st_ns_low;

	struct dentry *__mode;

	struct dentry *economy_dir;
	struct dentry *normal_dir;
	struct dentry *turbo_dir;
	struct dentry *super_t_dir;

	struct dentry *eco_high;
	struct dentry *nor_high;
	struct dentry *tur_high;
	struct dentry *st_low;
	struct dentry *tur_low;
	struct dentry *nor_low;
	struct pi_mgr_dfs_node dev_dfs_node;
	/* for dvfs */
	dvfs_mode_e requested_mode;
	/* updated based on profiling and requested to Power Manger */
	dvfs_mode_e current_mode;
	/*updated in DVFS callback from Power Manager */
	bool timer_state;
	/* DVFS timer state (initialized/unintialized) */
	MM_DVFS_HW_IFC dvfs;
	/* DVFS tunable parameters */

	struct timer_list dvfs_timeout;
	struct work_struct dvfs_work;

	struct timespec ts1;
	struct timespec dvfst1;

	s64 hw_on_dur;
	unsigned int jobs_done;
	unsigned int jobs_pend;
	struct mm_prof_buff buff[BUFF_SIZE];
	int write_ptr;
#ifdef CONFIG_MEMC_DFS
	struct kona_memc_node *memc_node;
#endif
};

void *mm_dvfs_init(struct _mm_common_ifc *mm_common_ifc,
		const char *mm_dev_name, MM_DVFS_HW_IFC *dvfs_params);
void mm_dvfs_exit(void *mm_dvfs);

#endif
