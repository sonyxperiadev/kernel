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
#define pr_fmt(fmt) "<%s> %s:" fmt "\n", mm_dvfs->mm_common->mm_name, __func__

#include "mm_dvfs.h"

static int mm_dfs_chg_notifier(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct _mm_dvfs *mm_dvfs = container_of(self, \
				struct _mm_dvfs, \
				mm_dfs_chg_notify_blk);
	SCHEDULER_WORK(mm_dvfs, &mm_dvfs->dvfs_notification);

	return 0;
}

static void mm_dfs_notifier(struct work_struct *work)
{
	struct _mm_dvfs *mm_dvfs = container_of(work, \
					struct _mm_dvfs, \
					dvfs_notification);
	mm_dvfs->current_mode = pi_get_active_opp(PI_MGR_PI_ID_MM);
	atomic_notifier_call_chain(&mm_dvfs->mm_common->notifier_head, \
					MM_FMWK_NOTIFY_DVFS_UPDATE, \
					(void *)mm_dvfs->current_mode);
}

int mm_dvfs_notification_handler(struct notifier_block *block, \
				unsigned long param, \
				void *data)
{
	struct timespec diff;
	struct _mm_dvfs *mm_dvfs = container_of(block, \
				struct _mm_dvfs,\
				mm_fmwk_notifier_blk);

	switch (param) {
	case MM_FMWK_NOTIFY_JOB_ADD:
		mm_dvfs->jobs_pend++;
		break;
	case MM_FMWK_NOTIFY_JOB_COMPLETE:
	case MM_FMWK_NOTIFY_JOB_REMOVE:
		mm_dvfs->jobs_done++;
		mm_dvfs->jobs_pend--;
		break;
	case MM_FMWK_NOTIFY_CLK_ENABLE:
		getnstimeofday(&mm_dvfs->ts1);
		if (mm_dvfs->timer_state == false)
			SCHEDULER_WORK(mm_dvfs, &(mm_dvfs->dvfs_work));
		break;
	case MM_FMWK_NOTIFY_CLK_DISABLE:
		getnstimeofday(&diff);
		diff = timespec_sub(diff, mm_dvfs->ts1);
		mm_dvfs->hw_on_dur += timespec_to_ns(&diff);
		pr_debug("dev stayed on for %llu nanoseconds", \
			(unsigned long long)timespec_to_ns(&diff));
		break;
	case MM_FMWK_NOTIFY_INVALID:
	default:
		break;
		}
	return NOTIFY_DONE;
}

static void dvfs_timeout_callback(unsigned long data)
{
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)data;
	SCHEDULER_WORK(mm_dvfs, &(mm_dvfs->dvfs_work));
}

static void dvfs_start_timer(struct _mm_dvfs *mm_dvfs)
{
	mm_dvfs->jobs_done = 0;
	mm_dvfs->hw_on_dur = 0;
	if (mm_dvfs->current_mode == NORMAL)
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T1));
	if (mm_dvfs->current_mode == TURBO)
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T2));
}

static void dvfs_work(struct work_struct *work)
{
	struct timespec diff;
	int percnt = 0;
	int temp = 0;
	struct _mm_dvfs *mm_dvfs = container_of(work, \
					struct _mm_dvfs, \
					dvfs_work);

	if ((mm_dvfs->dvfs.is_dvfs_on == false) &&
		(mm_dvfs->requested_mode \
		!= mm_dvfs->dvfs.user_requested_mode)) {
		mm_dvfs->requested_mode = mm_dvfs->dvfs.user_requested_mode;
		pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
					mm_dvfs->requested_mode);
		return;
		}

	if (mm_dvfs->timer_state == false) {
		mm_dvfs->jobs_done = 0;
		mm_dvfs->hw_on_dur = 0;
		getnstimeofday(&(mm_dvfs->dvfst1));
		init_timer(&(mm_dvfs->dvfs_timeout));
		setup_timer(&(mm_dvfs->dvfs_timeout),  \
				dvfs_timeout_callback, \
				(unsigned long)mm_dvfs);
		mm_dvfs->requested_mode = NORMAL;
		pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
					mm_dvfs->requested_mode);
		dvfs_start_timer(mm_dvfs);
		mm_dvfs->timer_state = true;
		pr_debug("wake dvfs mode to normal..");
		return;
		}

	getnstimeofday(&diff);
	if (mm_dvfs->mm_common->mm_hw_is_on) {
		struct timespec diff2 = timespec_sub(diff, mm_dvfs->ts1);
		mm_dvfs->hw_on_dur += timespec_to_ns(&diff2);
		getnstimeofday(&mm_dvfs->ts1);
		}

	diff = timespec_sub(diff, mm_dvfs->dvfst1);
	getnstimeofday(&(mm_dvfs->dvfst1));

	percnt = (mm_dvfs->hw_on_dur>>12);
	percnt = percnt*100;
	temp = (timespec_to_ns(&diff)>>12);
/*	percnt = percnt/temp;*/


	/* In addition to Hardware ON time,
	  * the Framework also uses the average Job time (in previous slot)
	  * and the number of pending jobs in the list will also be used to
	  * move to TURBO
	  * (this shall not be used as a measure for coming back to NORMAL)*/
	/* access mm_dvfs->jobs_done*/
	/* access mm_dvfs->jobs_pend*/
	pr_debug("dvfs_hw_on_dur %lld dvfs timeslot %d percnt %d", \
				mm_dvfs->hw_on_dur, temp, percnt);

	if ((mm_dvfs->current_mode == NORMAL) &&
		(percnt > (mm_dvfs->dvfs.P1*temp)) &&
		(mm_dvfs->requested_mode != TURBO)) {
		mm_dvfs->requested_mode = TURBO;
		pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
					mm_dvfs->requested_mode);
		}

	if ((mm_dvfs->current_mode == TURBO) &&
		(percnt < (mm_dvfs->dvfs.P2*temp)) &&
		(mm_dvfs->requested_mode != NORMAL)) {
		pr_debug("change dvfs mode to normal..");
		mm_dvfs->requested_mode = NORMAL;
		pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
					mm_dvfs->requested_mode);
		}

	if (percnt != 0) {
		dvfs_start_timer(mm_dvfs);
		}
	else {
		del_timer_sync(&mm_dvfs->dvfs_timeout);
		mm_dvfs->timer_state = false;
		}
}

void mm_dvfs_update_handler(struct work_struct *work)
{
	 struct dvfs_update *update = container_of(work, \
					struct dvfs_update, \
					work);
	 struct _mm_dvfs *mm_dvfs = update->mm_dvfs;
	 unsigned int param = update->param;
	 unsigned int max = 0, min = 0;

	 if (!update->is_read) {
		switch (update->type) {
		case MM_DVFS_UPDATE_ENABLE:
			if (param > 1) {
				pr_err("Enter 0/1 ");
				break;
				}
			if (mm_dvfs->dvfs.is_dvfs_on != param) {
				mm_dvfs->dvfs.is_dvfs_on = param;
				if (param) {
					mm_dvfs->mm_fmwk_notifier_blk.\
					    notifier_call = \
					    mm_dvfs_notification_handler;
					atomic_notifier_chain_register( \
					    &mm_dvfs->mm_common->notifier_head,\
					    &mm_dvfs->mm_fmwk_notifier_blk);
					}
				else{
					atomic_notifier_chain_unregister( \
					    &mm_dvfs->mm_common->notifier_head,\
					    &mm_dvfs->mm_fmwk_notifier_blk);
					}
				SCHEDULER_WORK(mm_dvfs, &(mm_dvfs->dvfs_work));
				}
			break;
		case MM_DVFS_UPDATE_SUSPEND:
			if (param > 1)
				pr_err("Enter 0/1 ");
			else
				; /* no-op now */
			break;
		case MM_DVFS_UPDATE_REQ_MODE:
			if (param > 2)
				pr_err("Enter 0/1/2 ");
			else
				mm_dvfs->dvfs.user_requested_mode = param;
			break;
		case MM_DVFS_UPDATE_T1:
			max = mm_dvfs->dvfs.T2/4;
			if (param > max)
				pr_err("Enter value <= %u ", max);
			else
				mm_dvfs->dvfs.T1 = param;
			break;
		case MM_DVFS_UPDATE_P1:
			max = 100;
			min = (mm_dvfs->dvfs.P2*TURBO_RATE)/NORMAL_RATE;
			if ((param > max) || (param < min))
				pr_err("Enter value between %u and %u ", \
								min, max);
			else
				mm_dvfs->dvfs.P1 = param;
			break;
		case MM_DVFS_UPDATE_T2:
			min = mm_dvfs->dvfs.T1*4;
			if (param < min)
				pr_err("Enter value >= %u ", min);
			else
				mm_dvfs->dvfs.T2 = param;
			break;
		case MM_DVFS_UPDATE_P2:
			min = 100*NORMAL_RATE/TURBO_RATE;
			max = (mm_dvfs->dvfs.P1*NORMAL_RATE)/TURBO_RATE;
			if ((param > max) || (param < min))
				pr_err("Enter value between %u and %u ", \
								min, max);
			else
				mm_dvfs->dvfs.P2 = param;
			break;
		case MM_DVFS_UPDATE_JOB_CNT:
		default:
			pr_err("Not Supported at this time");
			break;
		}
	}
	 else {
		 switch (update->type) {
		 case MM_DVFS_UPDATE_ENABLE:
			 param = mm_dvfs->dvfs.is_dvfs_on ;
			 break;
		 case MM_DVFS_UPDATE_SUSPEND:
			/* no-op now */
			 break;
		 case MM_DVFS_UPDATE_REQ_MODE:
			  param = mm_dvfs->dvfs.user_requested_mode;
			 break;
		 case MM_DVFS_UPDATE_T1:
			 param = mm_dvfs->dvfs.T1;
			 break;
		 case MM_DVFS_UPDATE_P1:
			 param = mm_dvfs->dvfs.P1;
			 break;
		 case MM_DVFS_UPDATE_T2:
			 param = mm_dvfs->dvfs.T2;
			 break;
		 case MM_DVFS_UPDATE_P2:
			 param = mm_dvfs->dvfs.P2;
			 break;
		 case MM_DVFS_UPDATE_JOB_CNT:
		 default:
			 pr_err("Not Supported at this time");
			 break;
		 }
		 update->param = param;
		}
}


DEFINE_DEBUGFS_HANDLER(ON, MM_DVFS_UPDATE_ENABLE);
DEFINE_DEBUGFS_HANDLER(MODE, MM_DVFS_UPDATE_REQ_MODE);
DEFINE_DEBUGFS_HANDLER(T1, MM_DVFS_UPDATE_T1);
DEFINE_DEBUGFS_HANDLER(P1, MM_DVFS_UPDATE_P1);
DEFINE_DEBUGFS_HANDLER(T2, MM_DVFS_UPDATE_T2);
DEFINE_DEBUGFS_HANDLER(P2, MM_DVFS_UPDATE_P2);

void *mm_dvfs_init(struct mm_common *mm_common, \
		const char *dev_name, MM_DVFS_HW_IFC *dvfs_params)
{
	int ret = 0;
	struct _mm_dvfs *mm_dvfs = kmalloc(sizeof(struct _mm_dvfs), GFP_KERNEL);
	memset(mm_dvfs, 0, sizeof(struct _mm_dvfs));

	mm_dvfs->mm_common = mm_common;
	INIT_WORK(&(mm_dvfs->dvfs_work), dvfs_work);
	INIT_WORK(&(mm_dvfs->dvfs_notification), mm_dfs_notifier);

	/* Init prof counters */
	mm_dvfs->dvfs = *dvfs_params;
	mm_dvfs->requested_mode = mm_dvfs->dvfs.user_requested_mode;

	if (mm_dvfs->dvfs.is_dvfs_on) {
		mm_dvfs->requested_mode = NORMAL;
		mm_dvfs->mm_fmwk_notifier_blk.notifier_call \
				= mm_dvfs_notification_handler;
		atomic_notifier_chain_register(\
			&mm_common->notifier_head, \
			&mm_dvfs->mm_fmwk_notifier_blk);
		}
	mm_dvfs->current_mode = mm_dvfs->requested_mode;

	mm_dvfs->dvfs_dir = debugfs_create_dir("dvfs", \
				mm_dvfs->mm_common->debugfs_dir);
	if (mm_dvfs->dvfs_dir == NULL) {
		pr_err("Error %ld creating dvfs dir for %s", \
			PTR_ERR(mm_dvfs->dvfs_dir), dev_name);
		ret = -ENOENT;
		}

	CREATE_DEBUGFS_FILE(mm_dvfs, ON, mm_dvfs->dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, MODE, mm_dvfs->dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, T1, mm_dvfs->dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, P1, mm_dvfs->dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, T2, mm_dvfs->dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, P2, mm_dvfs->dvfs_dir);

	ret = pi_mgr_dfs_add_request(&(mm_dvfs->dev_dfs_node), (char *)dev_name,
					PI_MGR_PI_ID_MM, PI_MGR_DFS_MIN_VALUE);
	if (ret) {
		pr_err("failed to register PI DFS request for %s", dev_name);
		return NULL;
		}
	pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
				mm_dvfs->requested_mode);

	mm_dvfs->mm_dfs_chg_notify_blk.notifier_call = mm_dfs_chg_notifier;
	ret = pi_mgr_register_notifier(PI_MGR_PI_ID_MM,
		&(mm_dvfs->mm_dfs_chg_notify_blk),
		PI_NOTIFY_DFS_CHANGE);
	if (ret != 0) {
		pr_err("Failed to register PM Notifier");
		ret = -ENOENT;
		}

	return mm_dvfs;
}


void mm_dvfs_exit(void *dev_p)
{
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)dev_p;
	atomic_notifier_chain_unregister(\
		&mm_dvfs->mm_common->notifier_head, \
		&mm_dvfs->mm_fmwk_notifier_blk);
	debugfs_remove_recursive(mm_dvfs->dvfs_dir);
	pi_mgr_dfs_request_remove(&(mm_dvfs->dev_dfs_node));
	pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
			&(mm_dvfs->mm_dfs_chg_notify_blk),
			PI_NOTIFY_DFS_CHANGE);

	mm_dvfs->dev_dfs_node.name = NULL;
	kfree(mm_dvfs);
}
