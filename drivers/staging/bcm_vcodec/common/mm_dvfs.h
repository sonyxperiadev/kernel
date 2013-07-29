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

#ifndef _MM_DVFS_H_
#define _MM_DVFS_H_

#include "mm_common.h"

enum mm_dvfs_update {
	MM_DVFS_UPDATE_UNKNOWN = 0,
	MM_DVFS_UPDATE_ENABLE,
	MM_DVFS_UPDATE_SUSPEND,
	MM_DVFS_UPDATE_REQ_MODE,
	MM_DVFS_UPDATE_T1,
	MM_DVFS_UPDATE_P1,
	MM_DVFS_UPDATE_T2,
	MM_DVFS_UPDATE_P2,
	MM_DVFS_UPDATE_JOB_CNT,
};

struct dvfs_update {
	struct work_struct work;
	enum mm_dvfs_update type;
	bool is_read;
	u64 param;
	struct _mm_dvfs *mm_dvfs;
};

void mm_dvfs_update_handler(struct work_struct *work);

#define DEFINE_DEBUGFS_HANDLER(name, type_name)				\
	static int mm_dvfs_debugfs_##name##_get(void *root, u64 *param)	\
	{								\
		struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)root;	\
		struct dvfs_update update;				\
		update.type = type_name;				\
		update.param = 0;					\
		update.is_read = true;					\
		update.mm_dvfs = mm_dvfs;				\
		INIT_WORK(&(update.work), mm_dvfs_update_handler);	\
		SCHEDULER_WORK(mm_dvfs, &(update.work));		\
		flush_work_sync(&(update.work));			\
		*param = update.param;					\
		return 0;						\
	}								\
	static int mm_dvfs_debugfs_##name##_set(void *root, u64 param)	\
	{								\
		struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)root;	\
		struct dvfs_update update;				\
		update.type = type_name;				\
		update.param = param;					\
		update.is_read = false;					\
		update.mm_dvfs = mm_dvfs;				\
		INIT_WORK(&(update.work), mm_dvfs_update_handler);	\
		SCHEDULER_WORK(mm_dvfs, &(update.work));		\
		flush_work_sync(&(update.work));			\
		return 0;						\
	}								\
	DEFINE_SIMPLE_ATTRIBUTE(mm_dvfs_debugfs_##name,			\
				mm_dvfs_debugfs_##name##_get,		\
				mm_dvfs_debugfs_##name##_set,		\
				#name" : %llu\n");			\


#define CREATE_DEBUGFS_FILE(root, name, dir)				\
	{	root->name = debugfs_create_file(#name,			\
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),		\
		dir, root, &mm_dvfs_debugfs_##name); }			\

struct _mm_dvfs {

	struct mm_common *mm_common;

	struct notifier_block mm_fmwk_notifier_blk;
	struct notifier_block mm_dfs_chg_notify_blk;

	struct work_struct dvfs_notification;

	/* for job based profiling, 'n' jobs take how many microsecs */
	struct dentry *dvfs_dir;
	struct dentry *ON;
	struct dentry *T1;
	struct dentry *P1;
	struct dentry *T2;
	struct dentry *P2;
	struct dentry *MODE;

	struct pi_mgr_dfs_node dev_dfs_node;
	/* for dvfs */
	dvfs_mode_e requested_mode;
		/* updated based on profiling and requested to Power Manger*/
	dvfs_mode_e current_mode;
		/*updated in DVFS callback from Power Manager*/
	bool timer_state;
		/* DVFS timer state (initialized/unintialized)*/
	MM_DVFS_HW_IFC dvfs;
		/* DVFS tunable parameters*/

	struct timer_list dvfs_timeout;
	struct work_struct dvfs_work;

	struct timespec ts1;
	struct timespec dvfst1;

	s64 hw_on_dur;
	unsigned int jobs_done;
	unsigned int jobs_pend;
};

void *mm_dvfs_init(struct mm_common *mm_common, \
			const char *mm_dev_name, \
			MM_DVFS_HW_IFC *dvfs_params);
void mm_dvfs_exit(void *mm_dvfs);

#endif
