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
	init_timer(&(mm_dvfs->dvfs_timeout));
	setup_timer(&(mm_dvfs->dvfs_timeout),  \
			dvfs_timeout_callback, \
			(unsigned long)mm_dvfs);

	switch (mm_dvfs->requested_mode) {
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
	case SUPER_TURBO:
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T3));
		break;
#endif
	case TURBO:
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T2));
		break;
	case NORMAL:
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T1));
		break;
	case ECONOMY:
	default:
		mod_timer(&mm_dvfs->dvfs_timeout, \
			jiffies+msecs_to_jiffies(mm_dvfs->dvfs.T0));
		break;
	}
	mm_dvfs->timer_state = true;

}

static void dvfs_work(struct work_struct *work)
{
	struct timespec diff;
	int percnt = 0;
	int temp = 0;
	struct _mm_dvfs *mm_dvfs = container_of(work, \
					struct _mm_dvfs, \
					dvfs_work);
	int current_mode = pi_get_active_opp(PI_MGR_PI_ID_MM);
	if (mm_dvfs->dvfs.ON == false) {
		mm_dvfs->requested_mode = \
			mm_dvfs->dvfs.MODE;
		goto dvfs_work_end;
		}

	if (mm_dvfs->timer_state == false) {
		mm_dvfs->jobs_done = 0;
		mm_dvfs->hw_on_dur = 0;
		getnstimeofday(&(mm_dvfs->dvfst1));
		mm_dvfs->requested_mode = TURBO;
		dvfs_start_timer(mm_dvfs);
		pr_debug("wake dvfs mode to normal..");
		goto dvfs_work_end;
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

	/* In addition to Hardware ON time,
	  * the Framework also uses the average Job time (in previous slot)
	  * and the number of pending jobs in the list will also be used to
	  * move to TURBO
	  * (this shall not be used as a measure for coming back to NORMAL)*/
	/* access mm_dvfs->jobs_done*/
	/* access mm_dvfs->jobs_pend*/
	pr_debug("dvfs_hw_on_dur %lld dvfs timeslot %d percnt %d", \
				mm_dvfs->hw_on_dur, temp, percnt);

	switch (mm_dvfs->requested_mode) {
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
	case SUPER_TURBO:
		if ((percnt < mm_dvfs->dvfs.P3L*temp) &&
			(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to turbo..");
			mm_dvfs->requested_mode = TURBO;
			}
		break;
#endif
	case TURBO:
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
		if (percnt > (mm_dvfs->dvfs.P2*temp)) {
			pr_debug("change dvfs mode to super turbo..");
			mm_dvfs->requested_mode = SUPER_TURBO;
			}
#endif
		if ((percnt < mm_dvfs->dvfs.P2L*temp) &&
			(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to normal..");
			mm_dvfs->requested_mode = NORMAL;
			}
		break;
	case NORMAL:
		if (percnt > (mm_dvfs->dvfs.P1*temp)) {
			pr_debug("change dvfs mode to turbo..");
			mm_dvfs->requested_mode = TURBO;
			}
		if ((percnt < mm_dvfs->dvfs.P1L*temp) &&
			(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to economy..");
			mm_dvfs->requested_mode = ECONOMY;
			}
		break;
	case ECONOMY:
	default:
		if (percnt > (mm_dvfs->dvfs.P0*temp)) {
			pr_debug("change dvfs mode to normal..");
			mm_dvfs->requested_mode = NORMAL;
			}
		break;
		}

	del_timer_sync(&mm_dvfs->dvfs_timeout);
	mm_dvfs->timer_state = false;
	if (percnt != 0)
		dvfs_start_timer(mm_dvfs);
	else
		mm_dvfs->requested_mode = ECONOMY;

dvfs_work_end:
	raw_notifier_call_chain(&mm_dvfs->mm_common->notifier_head, \
					MM_FMWK_NOTIFY_DVFS_UPDATE, \
					(void *)mm_dvfs->requested_mode);
	pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
			mm_dvfs->requested_mode);

}

DEFINE_DEBUGFS_HANDLER(ON);
DEFINE_DEBUGFS_HANDLER(MODE);
DEFINE_DEBUGFS_HANDLER(T0);
DEFINE_DEBUGFS_HANDLER(P0);
DEFINE_DEBUGFS_HANDLER(T1);
DEFINE_DEBUGFS_HANDLER(P1);
DEFINE_DEBUGFS_HANDLER(P1L);
DEFINE_DEBUGFS_HANDLER(T2);
DEFINE_DEBUGFS_HANDLER(P2);
DEFINE_DEBUGFS_HANDLER(P2L);
DEFINE_DEBUGFS_HANDLER(T3);
DEFINE_DEBUGFS_HANDLER(P3L);

void *mm_dvfs_init(struct mm_common *mm_common, \
		const char *dev_name, MM_DVFS_HW_IFC *dvfs_params)
{
	int ret = 0;
	struct _mm_dvfs *mm_dvfs = kmalloc(sizeof(struct _mm_dvfs), GFP_KERNEL);
	memset(mm_dvfs, 0, sizeof(struct _mm_dvfs));

	mm_dvfs->mm_common = mm_common;
	INIT_WORK(&(mm_dvfs->dvfs_work), dvfs_work);

	/* Init prof counters */
	mm_dvfs->dvfs = *dvfs_params;
	if (mm_dvfs->dvfs.ON)
		mm_dvfs->requested_mode = ECONOMY;
	else
		mm_dvfs->requested_mode = mm_dvfs->dvfs.MODE;

	mm_dvfs->mm_fmwk_notifier_blk.notifier_call \
			= mm_dvfs_notification_handler;
	raw_notifier_chain_register(\
		&mm_common->notifier_head, \
		&mm_dvfs->mm_fmwk_notifier_blk);

	mm_dvfs->dvfs_dir = debugfs_create_dir("dvfs", \
				mm_dvfs->mm_common->debugfs_dir);
	if (mm_dvfs->dvfs_dir == NULL) {
		pr_err("Error %ld creating dvfs dir for %s", \
			PTR_ERR(mm_dvfs->dvfs_dir), dev_name);
		ret = -ENOENT;
		}

	CREATE_DEBUGFS_FILE(mm_dvfs, ON);
	CREATE_DEBUGFS_FILE(mm_dvfs, MODE);
	CREATE_DEBUGFS_FILE(mm_dvfs, T0);
	CREATE_DEBUGFS_FILE(mm_dvfs, P0);
	CREATE_DEBUGFS_FILE(mm_dvfs, T1);
	CREATE_DEBUGFS_FILE(mm_dvfs, P1);
	CREATE_DEBUGFS_FILE(mm_dvfs, P1L);
	CREATE_DEBUGFS_FILE(mm_dvfs, T2);
	CREATE_DEBUGFS_FILE(mm_dvfs, P2);
	CREATE_DEBUGFS_FILE(mm_dvfs, P2L);
	CREATE_DEBUGFS_FILE(mm_dvfs, T3);
	CREATE_DEBUGFS_FILE(mm_dvfs, P3L);

	ret = pi_mgr_dfs_add_request(&(mm_dvfs->dev_dfs_node), (char *)dev_name,
					PI_MGR_PI_ID_MM, PI_MGR_DFS_MIN_VALUE);
	if (ret) {
		pr_err("failed to register PI DFS request for %s", dev_name);
		return NULL;
		}
	pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
				mm_dvfs->requested_mode);

	return mm_dvfs;
}


void mm_dvfs_exit(void *dev_p)
{
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)dev_p;
	raw_notifier_chain_unregister(\
		&mm_dvfs->mm_common->notifier_head, \
		&mm_dvfs->mm_fmwk_notifier_blk);
	debugfs_remove_recursive(mm_dvfs->dvfs_dir);
	pi_mgr_dfs_request_remove(&(mm_dvfs->dev_dfs_node));

	mm_dvfs->dev_dfs_node.name = NULL;
	kfree(mm_dvfs);
}
