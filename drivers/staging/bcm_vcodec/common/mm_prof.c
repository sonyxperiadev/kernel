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
#define pr_fmt(fmt) "<%s> %s:" fmt "\n", mm_prof->mm_common->mm_name, __func__

#include "mm_prof.h"

static int mm_prof_notification_handler(struct notifier_block *block, \
				unsigned long param, \
				void *data)
{
	struct timespec diff;
	struct _mm_prof *mm_prof = container_of(block, struct _mm_prof, \
					mm_fmwk_notifier_blk);

	switch (param) {
	case MM_FMWK_NOTIFY_JOB_COMPLETE:
	case MM_FMWK_NOTIFY_JOB_REMOVE:
		mm_prof->jobs_done++;
		mm_prof->jobs_done_type\
		[(unsigned int)data & (MAX_JOB_TYPE-1)]++;
		break;
	case MM_FMWK_NOTIFY_CLK_ENABLE:
		getnstimeofday(&mm_prof->ts1);
		break;
	case MM_FMWK_NOTIFY_CLK_DISABLE:
		getnstimeofday(&diff);
		diff = timespec_sub(diff, mm_prof->ts1);
		mm_prof->hw_on_dur += timespec_to_ns(&diff);
		pr_debug("dev stayed on for %llu nanoseconds", \
			(unsigned long long)timespec_to_ns(&diff));
		break;
	case MM_FMWK_NOTIFY_DVFS_UPDATE:
		mm_prof->current_mode = (dvfs_mode_e)data;
		break;
	case MM_FMWK_NOTIFY_INVALID:
	default:
		break;
	}
	return NOTIFY_DONE;
}

static void prof_timeout_callback(unsigned long data)
{
	struct _mm_prof *mm_prof = (struct _mm_prof *)data;
	SCHEDULER_WORK(mm_prof, &(mm_prof->prof_work));
}

static void prof_work(struct work_struct *work)
{
	struct timespec diff;
	int percnt = 0, temp = 0;
	struct _mm_prof *mm_prof = container_of(work, \
					struct _mm_prof, \
					prof_work);

	if (mm_prof->T1 == 0) {
		if (mm_prof->timer_state == true) {
			del_timer_sync(&mm_prof->prof_timeout);
			atomic_notifier_chain_unregister(\
				&mm_prof->mm_common->notifier_head, \
				&mm_prof->mm_fmwk_notifier_blk);
			mm_prof->timer_state = false;
			}
		return;
		}

	if (mm_prof->timer_state == false) {
		mm_prof->hw_on_dur = 0;
		mm_prof->jobs_done = 0;
		mm_prof->jobs_done_type[0] = 0;
		mm_prof->jobs_done_type[1] = 0;
		mm_prof->jobs_done_type[2] = 0;
		mm_prof->jobs_done_type[3] = 0;
		mm_prof->mm_fmwk_notifier_blk.notifier_call = \
					mm_prof_notification_handler;
		atomic_notifier_chain_register(\
					&mm_prof->mm_common->notifier_head, \
					&mm_prof->mm_fmwk_notifier_blk);
		getnstimeofday(&(mm_prof->proft1));
		init_timer(&(mm_prof->prof_timeout));
		setup_timer(&(mm_prof->prof_timeout), \
			prof_timeout_callback, \
			(unsigned long)mm_prof);
		mod_timer(&mm_prof->prof_timeout, \
			jiffies+msecs_to_jiffies(mm_prof->T1*1000));
		mm_prof->timer_state = true;
		pr_debug("wake prof mode to normal..");
		return;
		}

	getnstimeofday(&diff);
	if (mm_prof->mm_common->mm_hw_is_on) {
		struct timespec diff2 = timespec_sub(diff, mm_prof->ts1);
		mm_prof->hw_on_dur += timespec_to_ns(&diff2);
		getnstimeofday(&mm_prof->ts1);
		}

	diff = timespec_sub(diff, mm_prof->proft1);
	getnstimeofday(&(mm_prof->proft1));

	percnt = (mm_prof->hw_on_dur>>12);
	percnt = percnt*100;
	temp = (timespec_to_ns(&diff)>>12);
	percnt = percnt/temp;

	pr_err("hw_usage: ON : %d%% [DVFS:%d] JOBS : %d [%d %d %d %d] in %d secs ",
				percnt, mm_prof->current_mode,
				mm_prof->jobs_done, mm_prof->jobs_done_type[0],
				mm_prof->jobs_done_type[1], \
				mm_prof->jobs_done_type[2], \
				mm_prof->jobs_done_type[3], \
				mm_prof->T1);

	mm_prof->hw_on_dur = 0;
	mm_prof->jobs_done = 0;
	mm_prof->jobs_done_type[0] = 0;
	mm_prof->jobs_done_type[1] = 0;
	mm_prof->jobs_done_type[2] = 0;
	mm_prof->jobs_done_type[3] = 0;
	mod_timer(&mm_prof->prof_timeout, \
		jiffies+msecs_to_jiffies(mm_prof->T1*1000));
}

void mm_tmp_prof_update_handler(struct work_struct *work)
{
	 struct prof_update *update = container_of(work, \
						struct prof_update, \
						work);
	 struct _mm_prof *mm_prof = update->mm_prof;
	 unsigned int param = update->param;
	 unsigned int max = 0;

	 if (!update->is_read) {
		switch (update->type) {
		case MM_PROF_UPDATE_TIME:
			max = 600;
			if (param > max)
				pr_err("Enter value <= %u ", max);
			else
				mm_prof->T1 = param;
			SCHEDULER_WORK(mm_prof, &(mm_prof->prof_work));
			break;
		default:
			pr_err("Not Supported at this time");
			break;
		}
	}
	 else {
		 switch (update->type) {
		 case MM_PROF_UPDATE_TIME:
			 param = mm_prof->T1;
			 break;
		 default:
			 pr_err("Not Supported at this time");
			 break;
		 }
		 update->param = param;
	}
}


DEFINE_DEBUGFS_HANDLER(TIME, MM_PROF_UPDATE_TIME);

void *mm_tmp_prof_init(struct mm_common *mm_common, \
			const char *dev_name, \
			MM_PROF_HW_IFC * prof_params)
{
	int ret = 0;
	struct _mm_prof *mm_prof = kmalloc(sizeof(struct _mm_prof), GFP_KERNEL);
	memset(mm_prof, 0, sizeof(struct _mm_prof));

	mm_prof->mm_common = mm_common;
	INIT_WORK(&(mm_prof->prof_work), prof_work);

	/* Init prof counters */
	mm_prof->prof = *prof_params;
	mm_prof->current_mode = ECONOMY;

	mm_prof->prof_dir = \
		debugfs_create_dir("prof", mm_prof->mm_common->debugfs_dir);
	if (mm_prof->prof_dir == NULL) {
		pr_err("Error %ld creating prof dir for %s", \
					PTR_ERR(mm_prof->prof_dir), \
					dev_name);
		ret = -ENOENT;
		}

	CREATE_DEBUGFS_FILE(mm_prof, TIME, mm_prof->prof_dir);

	return mm_prof;
}


void mm_tmp_prof_exit(void *dev_p)
{
	struct _mm_prof *mm_prof = (struct _mm_prof *)dev_p;

	if (mm_prof->prof_dir)
		debugfs_remove_recursive(mm_prof->prof_dir);

	kfree(mm_prof);
}
