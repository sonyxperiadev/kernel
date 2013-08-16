/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _MM_PROF_H_
#define _MM_PROF_H_

#include "mm_common.h"
#include "mm_dvfs.h"

enum mm_prof_update {
	MM_PROF_UPDATE_UNKNOWN = 0,
	MM_PROF_UPDATE_TIME,
	MM_PROF_UPDATE_JOB,
	MM_PROF_UPDATE_HW,
};

struct prof_update {
	struct work_struct work;
	enum mm_prof_update type;
	bool is_read;
	u64 param;
	struct _mm_prof *mm_prof;
};

void mm_tmp_prof_update_handler(struct work_struct *work);

#undef DEFINE_DEBUGFS_HANDLER
#undef CREATE_DEBUGFS_FILE

#define DEFINE_DEBUGFS_HANDLER(name, type_name)				\
	static int mm_prof_debugfs_##name##_get(void *root, u64 *param)	\
	{								\
		struct _mm_prof *mm_prof = (struct _mm_prof *)root;	\
		struct prof_update update;				\
		update.type = type_name;				\
		update.param = 0;					\
		update.is_read = true;					\
		update.mm_prof = mm_prof;				\
		INIT_WORK(&(update.work), mm_tmp_prof_update_handler);	\
		SCHEDULER_WORK(mm_prof, &(update.work));		\
		flush_work_sync(&(update.work));			\
		*param = update.param;					\
		return 0;						\
	}								\
	static int mm_prof_debugfs_##name##_set(void *root, u64 param)	\
	{								\
		struct _mm_prof *mm_prof = (struct _mm_prof *)root;	\
		struct prof_update update;				\
		update.type = type_name;				\
		update.param = param;					\
		update.is_read = false;					\
		update.mm_prof = mm_prof;				\
		INIT_WORK(&(update.work), mm_tmp_prof_update_handler);	\
		SCHEDULER_WORK(mm_prof, &(update.work));		\
		flush_work_sync(&(update.work));			\
		return 0;						\
	}								\
	DEFINE_SIMPLE_ATTRIBUTE(mm_prof_debugfs_##name,			\
					mm_prof_debugfs_##name##_get,	\
					mm_prof_debugfs_##name##_set,	\
					#name" : %llu\n");		\


#define CREATE_DEBUGFS_FILE(root, name, dir)				\
	{ root->name = debugfs_create_file(#name,			\
				(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),\
				dir, root, &mm_prof_debugfs_##name); }

#define MAX_JOB_TYPE 4
struct _mm_prof {

	struct mm_common *mm_common;

	struct notifier_block mm_fmwk_notifier_blk;

	/* for job based profiling, 'n' jobs take how many microsecs */
	struct dentry *prof_dir;
	struct dentry *TIME;
	struct dentry *JOB;
	struct dentry *HW;

	/* for prof */
	dvfs_mode_e current_mode;
		/*updated in PROF callback from Power Manager*/
	bool timer_state;
		/* PROF timer state (initialized/unintialized)*/
	MM_PROF_HW_IFC prof;


	unsigned int T1; /* Profiling time*/

	struct timer_list prof_timeout;
	struct work_struct prof_work;

	struct timespec ts1;
	struct timespec proft1;

	s64 hw_on_dur;
	unsigned int jobs_done;
	unsigned int jobs_done_type[MAX_JOB_TYPE];

};

void *mm_tmp_prof_init(struct mm_common *mm_common, \
		const char *mm_dev_name, \
		MM_PROF_HW_IFC *prof_params);
void mm_tmp_prof_exit(void *mm_prof);


#endif
