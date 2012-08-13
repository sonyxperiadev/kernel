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
#define pr_fmt(fmt) "<%s> %s:" fmt "\n",mm_prof->mm_common->single_wq_name,__func__

#include "mm_prof.h"

int mm_prof_notification_handler(struct notifier_block* block,unsigned long param, void* data)
{
	struct timespec diff;
	mm_prof_t *mm_prof = container_of(block, mm_prof_t, mm_fmwk_notifier_blk);

	switch(param) {
		case MM_FMWK_NOTIFY_JOB_COMPLETE:
		case MM_FMWK_NOTIFY_JOB_REMOVE:
			mm_prof->jobs_done++;
			break;
		case MM_FMWK_NOTIFY_CLK_ENABLE:
			getnstimeofday(&mm_prof->ts1);
			break;
		case MM_FMWK_NOTIFY_CLK_DISABLE:
			getnstimeofday(&diff);
			diff = timespec_sub(diff, mm_prof->ts1);
			mm_prof->hw_on_dur += timespec_to_ns(&diff);
			pr_debug("dev stayed on for %llu nanoseconds", (unsigned long long)timespec_to_ns(&diff));
			break;
		case MM_FMWK_NOTIFY_INVALID:
		default:
			break;
		}
	return NOTIFY_DONE;
}

static void prof_timeout_callback (unsigned long data)
{
	mm_prof_t *mm_prof = (mm_prof_t *)data;
	queue_work(mm_prof->mm_common->single_wq, &(mm_prof->prof_work));
}

static void prof_work(struct work_struct* work)
{
	struct timespec diff;
	int percnt = 0, temp = 0;
	mm_prof_t *mm_prof = container_of(work, mm_prof_t, prof_work);

	if( mm_prof->T1 == 0) {
		if(mm_prof->timer_state == true) {
			del_timer_sync(&mm_prof->prof_timeout);
			atomic_notifier_chain_unregister(&mm_prof->mm_common->notifier_head, &mm_prof->mm_fmwk_notifier_blk);
			mm_prof->timer_state = false;
			}
		return;
		}
	
	if(mm_prof->timer_state == false) {
		mm_prof->hw_on_dur = 0;
		mm_prof->jobs_done = 0;
		mm_prof->mm_fmwk_notifier_blk.notifier_call = mm_prof_notification_handler;
		atomic_notifier_chain_register(&mm_prof->mm_common->notifier_head, &mm_prof->mm_fmwk_notifier_blk);
		getnstimeofday(&(mm_prof->proft1));
		init_timer(&(mm_prof->prof_timeout));
		setup_timer(&(mm_prof->prof_timeout), prof_timeout_callback, (unsigned long)mm_prof);
		mod_timer(&mm_prof->prof_timeout, jiffies+msecs_to_jiffies(mm_prof->T1*1000));
		mm_prof->timer_state = true;
		pr_debug("wake prof mode to normal..");
		return;
		}

	if(mm_prof->mm_common->mm_hw_is_on) {
		mm_prof_notification_handler(&mm_prof->mm_fmwk_notifier_blk,MM_FMWK_NOTIFY_CLK_DISABLE,NULL);
		mm_prof_notification_handler(&mm_prof->mm_fmwk_notifier_blk,MM_FMWK_NOTIFY_CLK_ENABLE,NULL);
		}

	getnstimeofday(&diff);
	diff = timespec_sub(diff, mm_prof->proft1);
	getnstimeofday(&(mm_prof->proft1));

	percnt = (mm_prof->hw_on_dur>>12);
	percnt = percnt*100;
	temp = (timespec_to_ns(&diff)>>12);
	percnt = percnt/temp;

	pr_err("hw_usage: ON : %d%% [DVFS:%d] JOBS : %d in %d secs ",  percnt,mm_prof->current_mode,mm_prof->jobs_done, mm_prof->T1);

	mm_prof->hw_on_dur = 0;
	mm_prof->jobs_done = 0;
	mod_timer(&mm_prof->prof_timeout, jiffies+msecs_to_jiffies(mm_prof->T1*1000));
}

void mm_prof_update_handler(struct work_struct* work)
{
	 prof_update_t *update= container_of(work, prof_update_t, work);
	 mm_prof_t *mm_prof = update->mm_prof;
	 unsigned int param = update->param;
	 unsigned int max =0, min = 0;

	 if(!update->is_read) {
	 	switch(update->type){
			case MM_PROF_UPDATE_TIME:
				max = 600;
				if(param > max) pr_err("Enter value <= %u ",max);
				else mm_prof->T1 = param;
				queue_work(mm_prof->mm_common->single_wq, &(mm_prof->prof_work));
				break;
			default:
				pr_err("Not Supported at this time");
				break;
	 		}
	 	}
	 else {
		 switch(update->type){
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


DEFINE_DEBUGFS_HANDLER(TIME,MM_PROF_UPDATE_TIME);

void* mm_prof_init(mm_fmwk_common_t* mm_common, char *dev_name, MM_PROF_HW_IFC *prof_params)
{
	int ret = 0;
	mm_prof_t *mm_prof = kmalloc(sizeof(mm_prof_t),GFP_KERNEL);
	memset(mm_prof,0,sizeof(mm_prof_t));

	mm_prof->mm_common = mm_common;
	INIT_WORK(&(mm_prof->prof_work), prof_work);

	/* Init prof counters */
	mm_prof->prof = *prof_params;
	mm_prof->current_mode = ECONOMY;

	mm_prof->prof_dir = debugfs_create_dir("prof", mm_prof->mm_common->debugfs_dir);
    if(mm_prof->prof_dir == NULL) {
		pr_err("Error %ld creating prof dir for %s",
                           PTR_ERR(mm_prof->prof_dir), dev_name);
		ret = -ENOENT;
		}

	CREATE_DEBUGFS_FILE(mm_prof,TIME,mm_prof->prof_dir);

	return mm_prof;
}


void mm_prof_exit( void *dev_p)
{
	mm_prof_t* mm_prof = (mm_prof_t*)dev_p;
}

