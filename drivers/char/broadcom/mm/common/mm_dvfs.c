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
#include <linux/slab.h>
#define pr_fmt(fmt) "<%s> %s:" fmt "\n",\
			mm_dvfs->mm_common_ifc->mm_name, __func__

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
	mod_timer(&mm_dvfs->dvfs_timeout,
		jiffies+msecs_to_jiffies(mm_dvfs->dvfs.__ts));
	mm_dvfs->timer_state = true;
}

static inline void add_to_history_buffer(struct _mm_dvfs *dvfs_struct, int val)
{
	int i;
	for (i = 0; i < NUM_DVFS_PROF_SAMPLES - 1; i++)
		dvfs_struct->prof_history[i + 1] = dvfs_struct->prof_history[i];
	dvfs_struct->prof_history[0] = val;
}


static inline void clear_history_buffer(struct _mm_dvfs *dvfs_struct,
						unsigned int val)
{
	int i;
	for (i = 0; i < NUM_DVFS_PROF_SAMPLES; i++)
		dvfs_struct->prof_history[i] = val;
}

static inline bool dvfs_switch_to_lower_mode(struct _mm_dvfs *dvfs_struct,
		unsigned int val, int count)
{
	int i;
	int ret = 1;
	count = count > NUM_DVFS_PROF_SAMPLES ? NUM_DVFS_PROF_SAMPLES : count;
	if (count < 1)
		count = 1;
	for (i = 0; i < count; i++)
		ret = ret && (dvfs_struct->prof_history[i] < val);
	return ret;
}

static inline bool dvfs_switch_to_upper_mode(struct _mm_dvfs *dvfs_struct,
		unsigned int val, int count)
{
	int i;
	int ret = 1;
	count = count > NUM_DVFS_PROF_SAMPLES ? NUM_DVFS_PROF_SAMPLES : count;
	if (count < 1)
		count = 1;
	for (i = 0; i < count; i++)
		ret = ret && (dvfs_struct->prof_history[i] > val);

	return ret;
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
	if (mm_dvfs->dvfs.__on == false) {
		mm_dvfs->requested_mode = \
			mm_dvfs->dvfs.__mode;
		goto dvfs_work_end;
		}

	if (mm_dvfs->timer_state == false) {
		mm_dvfs->jobs_done = 0;
		mm_dvfs->hw_on_dur = 0;
		getnstimeofday(&(mm_dvfs->dvfst1));
		mm_dvfs->requested_mode = NORMAL;
		dvfs_start_timer(mm_dvfs);
		pr_debug("wake dvfs mode to normal..");
		goto dvfs_work_end;
		}

	getnstimeofday(&diff);
	if (mm_dvfs->mm_common_ifc->mm_hw_is_on) {
		struct timespec diff2 = timespec_sub(diff, mm_dvfs->ts1);
		mm_dvfs->hw_on_dur += timespec_to_ns(&diff2);
		getnstimeofday(&mm_dvfs->ts1);
		}

	diff = timespec_sub(diff, mm_dvfs->dvfst1);
	getnstimeofday(&(mm_dvfs->dvfst1));

	percnt = (mm_dvfs->hw_on_dur>>12);
	percnt = percnt*100;
	temp = (timespec_to_ns(&diff)>>12);
	add_to_history_buffer(mm_dvfs, percnt);

	/* In addition to Hardware ON time,
	  * the Framework also uses the average Job time (in previous slot)
	  * and the number of pending jobs in the list will also be used to
	  * move to TURBO
	  * (this shall not be used as a measure for coming back to NORMAL)*/
	/* access mm_dvfs->jobs_done*/
	/* access mm_dvfs->jobs_pend*/
	pr_debug("dvfs_hw_on_dur %lld dvfs timeslot %d percnt %d", \
				mm_dvfs->hw_on_dur, temp, percnt);

	switch (current_mode) {
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
	case SUPER_TURBO:
		if (dvfs_switch_to_lower_mode(mm_dvfs,
			mm_dvfs->dvfs.st_low*temp,
				mm_dvfs->dvfs.st_ns_low) &&
				(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to turbo..");
			mm_dvfs->requested_mode = TURBO;
			clear_history_buffer(mm_dvfs, mm_dvfs->dvfs.tur_low);
#ifdef CONFIG_MEMC_DFS
			memc_update_dfs_req(mm_dvfs->memc_node, MEMC_OPP_ECO);
#endif
			}
		break;
#endif
	case TURBO:
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
		if (dvfs_switch_to_upper_mode(mm_dvfs,
				mm_dvfs->dvfs.tur_high*temp,
					mm_dvfs->dvfs.tur_ns_high)) {
			pr_debug("change dvfs mode to super turbo..");
			mm_dvfs->requested_mode = SUPER_TURBO;
			clear_history_buffer(mm_dvfs, 0xFFFFFFFF);
#ifdef CONFIG_MEMC_DFS
			memc_update_dfs_req(mm_dvfs->memc_node, MEMC_OPP_TURBO);
#endif
			}
#endif
		if ((dvfs_switch_to_lower_mode(mm_dvfs,
			mm_dvfs->dvfs.tur_low*temp,
				mm_dvfs->dvfs.tur_ns_low)) &&
				(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to normal..");
			mm_dvfs->requested_mode = NORMAL;
			clear_history_buffer(mm_dvfs, mm_dvfs->dvfs.nor_low);
			}
		break;
	case NORMAL:
		if (dvfs_switch_to_upper_mode(mm_dvfs,
				mm_dvfs->dvfs.nor_high*temp,
					mm_dvfs->dvfs.nor_ns_high)) {
			pr_debug("change dvfs mode to turbo..");
			mm_dvfs->requested_mode = TURBO;
			clear_history_buffer(mm_dvfs, mm_dvfs->dvfs.tur_high);
			}
		if (dvfs_switch_to_lower_mode(mm_dvfs,
			mm_dvfs->dvfs.nor_low*temp,
				mm_dvfs->dvfs.nor_ns_low) &&
				(current_mode == mm_dvfs->requested_mode)) {
			pr_debug("change dvfs mode to economy..");
			mm_dvfs->requested_mode = ECONOMY;
			clear_history_buffer(mm_dvfs, 0x00000000);
			}
		break;
	case ECONOMY:
	default:
		if (dvfs_switch_to_upper_mode(mm_dvfs,
				mm_dvfs->dvfs.eco_high*temp,
					mm_dvfs->dvfs.eco_ns_high)) {
			pr_debug("change dvfs mode to normal..");
			mm_dvfs->requested_mode = NORMAL;
			clear_history_buffer(mm_dvfs, mm_dvfs->dvfs.nor_high);
			}
		break;
		}

	del_timer_sync(&mm_dvfs->dvfs_timeout);
	mm_dvfs->timer_state = false;
	if (percnt != 0)
		dvfs_start_timer(mm_dvfs);
	else {
		mm_dvfs->requested_mode = ECONOMY;
#ifdef CONFIG_MEMC_DFS
		memc_update_dfs_req(mm_dvfs->memc_node, MEMC_OPP_ECO);
#endif
	}
dvfs_work_end:
	raw_notifier_call_chain(&mm_dvfs->mm_common_ifc->notifier_head, \
					MM_FMWK_NOTIFY_DVFS_UPDATE, \
					(void *)mm_dvfs->requested_mode);
	if (pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
			mm_dvfs->requested_mode)) {
		pr_err("%s: failed to update dfs request\n", __func__);
	}

}


DEFINE_DEBUGFS_HANDLER(__on);
DEFINE_DEBUGFS_HANDLER(__mode);
DEFINE_DEBUGFS_HANDLER(__ts);

DEFINE_DEBUGFS_HANDLER(eco_high);
DEFINE_DEBUGFS_HANDLER(eco_ns_high);

DEFINE_DEBUGFS_HANDLER(nor_high);
DEFINE_DEBUGFS_HANDLER(nor_low);
DEFINE_DEBUGFS_HANDLER(nor_ns_low);
DEFINE_DEBUGFS_HANDLER(nor_ns_high);

DEFINE_DEBUGFS_HANDLER(tur_high);
DEFINE_DEBUGFS_HANDLER(tur_low);
DEFINE_DEBUGFS_HANDLER(tur_ns_high);
DEFINE_DEBUGFS_HANDLER(tur_ns_low);

DEFINE_DEBUGFS_HANDLER(st_low);
DEFINE_DEBUGFS_HANDLER(st_ns_low);

void *mm_dvfs_init(struct _mm_common_ifc *mm_common_ifc, \
		const char *dev_name, MM_DVFS_HW_IFC *dvfs_params)
{
	int ret = 0;
	struct _mm_dvfs *mm_dvfs = kmalloc(sizeof(struct _mm_dvfs), GFP_KERNEL);
	memset(mm_dvfs, 0, sizeof(struct _mm_dvfs));

	mm_dvfs->mm_common_ifc = mm_common_ifc;
	INIT_WORK(&(mm_dvfs->dvfs_work), dvfs_work);

	/* Init prof counters */
	mm_dvfs->dvfs = *dvfs_params;
	if (mm_dvfs->dvfs.__on)
		mm_dvfs->requested_mode = ECONOMY;
	else
		mm_dvfs->requested_mode = mm_dvfs->dvfs.__mode;

	mm_dvfs->mm_fmwk_notifier_blk.notifier_call \
			= mm_dvfs_notification_handler;
	raw_notifier_chain_register(\
		&mm_common_ifc->notifier_head, \
		&mm_dvfs->mm_fmwk_notifier_blk);

	mm_dvfs->dvfs_dir = debugfs_create_dir("dvfs", \
				mm_dvfs->mm_common_ifc->debugfs_dir);
	if (mm_dvfs->dvfs_dir == NULL) {
		pr_err("Error %ld creating dvfs dir for %s", \
			PTR_ERR(mm_dvfs->dvfs_dir), dev_name);
		ret = -ENOENT;
		}

	clear_history_buffer(mm_dvfs, 0x0);
	mm_dvfs->economy_dir = debugfs_create_dir("ECO", mm_dvfs->dvfs_dir);
	mm_dvfs->normal_dir = debugfs_create_dir("NOR", mm_dvfs->dvfs_dir);
	mm_dvfs->turbo_dir = debugfs_create_dir("TUR", mm_dvfs->dvfs_dir);
	mm_dvfs->super_t_dir = debugfs_create_dir("S_TUR", mm_dvfs->dvfs_dir);

	CREATE_DEBUGFS_FILE(mm_dvfs, eco, high, economy_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, eco, ns_high, economy_dir);

	CREATE_DEBUGFS_FILE(mm_dvfs, nor, low, normal_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, nor, high, normal_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, nor, ns_high, normal_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, nor, ns_low, normal_dir);

	CREATE_DEBUGFS_FILE(mm_dvfs, tur, high, turbo_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, tur, low, turbo_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, tur, ns_high, turbo_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, tur, ns_low, turbo_dir);

	CREATE_DEBUGFS_FILE(mm_dvfs, st, low, super_t_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, st, ns_low, super_t_dir);

	CREATE_DEBUGFS_FILE(mm_dvfs, _, on, dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, _, mode, dvfs_dir);
	CREATE_DEBUGFS_FILE(mm_dvfs, _, ts, dvfs_dir);
	ret = pi_mgr_dfs_add_request(&(mm_dvfs->dev_dfs_node), (char *)dev_name,
					PI_MGR_PI_ID_MM, PI_MGR_DFS_MIN_VALUE);
	if (ret) {
		pr_err("failed to register PI DFS request for %s", dev_name);
		return NULL;
		}
	if (pi_mgr_dfs_request_update(&(mm_dvfs->dev_dfs_node), \
				mm_dvfs->requested_mode)) {
		pr_err("%s: failed to update dfs request\n", __func__);
	}

#ifdef CONFIG_MEMC_DFS
	mm_dvfs->memc_node = kmalloc(sizeof(struct kona_memc_node), GFP_KERNEL);
	memset(mm_dvfs->memc_node, 0, sizeof(struct kona_memc_node));
	memc_add_dfs_req(mm_dvfs->memc_node, (char *)dev_name, MEMC_OPP_ECO);
#endif

	return mm_dvfs;
}


void mm_dvfs_exit(void *dev_p)
{
	struct _mm_dvfs *mm_dvfs = (struct _mm_dvfs *)dev_p;
	raw_notifier_chain_unregister(\
		&mm_dvfs->mm_common_ifc->notifier_head, \
		&mm_dvfs->mm_fmwk_notifier_blk);
	debugfs_remove_recursive(mm_dvfs->economy_dir);
	debugfs_remove_recursive(mm_dvfs->normal_dir);
	debugfs_remove_recursive(mm_dvfs->turbo_dir);
	debugfs_remove_recursive(mm_dvfs->super_t_dir);
	debugfs_remove_recursive(mm_dvfs->dvfs_dir);
	pi_mgr_dfs_request_remove(&(mm_dvfs->dev_dfs_node));

	mm_dvfs->dev_dfs_node.name = NULL;
	kfree(mm_dvfs);
}
