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

#include "mm_pwr.h"


typedef struct {
	dvfs_mode_e user_requested_mode; // forced request state 
	dvfs_mode_e requested_mode; // updated based on profiling and requested to Power Manger
	dvfs_mode_e current_mode; //updated in DVFS callback from Power Manager
	bool suspend_requested; // volatile flag updated in early-suspend/late-resume
	bool timer_state; // DVFS timer state (initialized/unintialized)
	MM_DVFS_HW_IFC params; // DVFS tunable parameters
} MM_DVFS_STATE;

typedef struct device_pwr {

	mm_fmwk_common_t* mm_common;

	struct notifier_block mm_fmwk_notifier;
	struct notifier_block mm_dfs_chg_notify_blk;

	int default_scale;
	
	volatile int dev_is_on;
	/* for hw profiling */
	unsigned long long hw_on_duration; /* in nano secs  */
	bool prof_hw_usage;
	int secs;
	unsigned long hw_prof_job_count;
	struct timespec ts1;
	struct timespec ts2;
	struct timespec p1;
	struct timespec p2;
	struct timer_list dev_prof_timeout;
	/* for job based profiling, 'n' jobs take how many microsecs */
	unsigned long jp_job_count;
	bool prof_job_cnt;
	bool jp_ts1_noted;
	struct timespec jp_ts1;
	struct timespec jp_ts2;
	struct work_struct dev_prof_work;
    /* Used for exporting per-device information to debugfs */
    struct dentry *debugfs_dir;
    struct dentry *dvfs_dir;
	/* File in the debugfs directory that exposes this device's information */
    struct dentry *prof_dbgfs_dentry;

    struct dentry *on;
    struct dentry *T1;
	struct dentry *P1;
	struct dentry *T2;
	struct dentry *P2;

	struct pi_mgr_dfs_node dev_dfs_node;
	/* for dvfs */
	MM_DVFS_STATE dvfs_state;//dvfs state
	struct timer_list dvfs_timeout;
	struct work_struct dvfs_work;
	struct timespec dvfst1;
	struct timespec dvfst2;
	unsigned long dvfs_hw_on_dur;
	unsigned int dvfs_jobs_done;
	unsigned int dvfs_jobs_pend;
	unsigned int bulk_jobs_count;
	/* for PM notification callback*/
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend_desc;
#endif
} dev_power_t;


int mm_pwr_notification(struct notifier_block* block,unsigned long param, void* data)
{
	struct timespec diff;
	unsigned long jp_jobs_duration = 0;

	dev_power_t *dev_power = container_of(block, dev_power_t, mm_fmwk_notifier);
	switch(param) {
		case MM_FMWK_NOTIFY_JOB_ADD:
			dev_power->dvfs_jobs_pend++;
			break;
		case MM_FMWK_NOTIFY_JOB_STARTED:
			if(dev_power->prof_job_cnt && !(dev_power->jp_ts1_noted))
			{
				getnstimeofday(&(dev_power->jp_ts1));
				dev_power->jp_ts1_noted = true;
			}
			break;
		case MM_FMWK_NOTIFY_JOB_COMPLETE:
		case MM_FMWK_NOTIFY_JOB_REMOVE:
			if(dev_power->prof_job_cnt && dev_power->jp_ts1_noted) {
				dev_power->jp_job_count--;
				if(dev_power->jp_job_count == 0) {
					getnstimeofday(&(dev_power->jp_ts2));
					dev_power->jp_ts1_noted = false;
					diff.tv_sec = 0;
					diff.tv_nsec = 0;
					diff = timespec_sub(dev_power->jp_ts2, dev_power->jp_ts1);
					jp_jobs_duration = timespec_to_ns(&diff);
					err_print("job_prof Time taken: %lu microsecs", jp_jobs_duration/1000);
				}
			}
			/* profile in 'n' time how many jobs execute */
			if(dev_power->prof_hw_usage) {
				 dev_power->hw_prof_job_count++;
			}
			if(dev_power->dvfs_state.params.is_dvfs_on) {
				dev_power->dvfs_jobs_done++;
			}
			dev_power->dvfs_jobs_pend--;
			break;
		
		case MM_FMWK_NOTIFY_CLK_ENABLE:
			dev_power->dev_is_on = 1;
			getnstimeofday(&dev_power->ts1);
			queue_work(dev_power->mm_common->single_wq, &(dev_power->dvfs_work));
			break;
		case MM_FMWK_NOTIFY_CLK_DISABLE:
			dev_power->dev_is_on = 0;
			getnstimeofday(&dev_power->ts2);
			diff.tv_sec = 0;
			diff.tv_nsec = 0;
			diff = timespec_sub(dev_power->ts2, dev_power->ts1);
			/* for profiling */
			dev_power->hw_on_duration += timespec_to_ns(&diff);
			/* for dvfs */
			dev_power->dvfs_hw_on_dur += timespec_to_ns(&diff);
			dbg_print("dev stayed on for %llu nanoseconds\n", (unsigned long long)timespec_to_ns(&diff));
			break;
		case MM_FMWK_NOTIFY_INVALID:
			break;
		}
	return NOTIFY_DONE;
}

static int mm_dfs_chg_notifier(struct notifier_block *self,
                               unsigned long event, void *data)
{
	dev_power_t *dev_power = container_of(self, dev_power_t, mm_dfs_chg_notify_blk);

    struct pi_notify_param *p = data;

    if(p->pi_id != PI_MGR_PI_ID_MM){
		err_print("wrong PI_MGR_PI_ID\n");
		return -EINVAL;
    }
	dev_power->dvfs_state.current_mode = p->new_value;
	return 0;
}

static void dvfs_timeout_callback (unsigned long data)
{
	dev_power_t *dev_power = (dev_power_t *)data;
	queue_work(dev_power->mm_common->single_wq, &(dev_power->dvfs_work));
}

static void dvfs_start_timer(dev_power_t *dev_power)
{
	struct timespec diff;
	if(!(dev_power->dvfs_state.suspend_requested)) {
		/* get dvfs start ts */
		getnstimeofday(&(dev_power->dvfst1));
		dev_power->dvfs_jobs_done = 0;
		if(dev_power->dev_is_on) {
			diff.tv_sec = 0;
			diff.tv_nsec = 0;
			diff = timespec_sub(dev_power->dvfst1, dev_power->ts1);
			dev_power->dvfs_hw_on_dur = -(timespec_to_ns(&diff));
		} else {
			dev_power->dvfs_hw_on_dur = 0;
		}
		if(dev_power->dvfs_state.current_mode == NORMAL)
			mod_timer(&dev_power->dvfs_timeout, jiffies+msecs_to_jiffies(dev_power->dvfs_state.params.T1));
		if(dev_power->dvfs_state.current_mode == TURBO)
			mod_timer(&dev_power->dvfs_timeout, jiffies+msecs_to_jiffies(dev_power->dvfs_state.params.T2));
	}
}

static void dvfs_work(struct work_struct* work)
{
	struct timespec diff;
	int percnt = 0;
	int temp = 0;
	int temp2 = 0;
	dev_power_t *dev_power =
                container_of(work, dev_power_t, dvfs_work);
	if(dev_power->dvfs_state.suspend_requested)
	{
		dev_power->dvfs_state.requested_mode = ECONOMY;
		if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), ECONOMY))
		{
			err_print("failed to update dfs request");
			pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
		}
		/* the dvfs timer will be stopped in early suspend */
		if(dev_power->dvfs_state.timer_state)
			del_timer_sync(&dev_power->dvfs_timeout);
		dev_power->dvfs_state.timer_state = false;
		return;
	}

	if(!(dev_power->dvfs_state.params.is_dvfs_on))
	{
		if(dev_power->dvfs_state.requested_mode != 
			dev_power->dvfs_state.params.user_requested_mode)
		{
			dev_power->dvfs_state.requested_mode = dev_power->dvfs_state.params.user_requested_mode;
			if(pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), dev_power->dvfs_state.requested_mode))
			{
				err_print("failed to update dfs request");
				pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
			}			
		}
		return;
	}
	
	if(!(dev_power->dvfs_state.timer_state))
	{
		init_timer(&(dev_power->dvfs_timeout));
		setup_timer(&(dev_power->dvfs_timeout), dvfs_timeout_callback, (unsigned long)dev_power);
		dev_power->dvfs_state.timer_state = true;
		dbg_print("wake dvfs mode to normal..\n");
		dev_power->dvfs_state.requested_mode = NORMAL;
		if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), 
				dev_power->dvfs_state.requested_mode))
		{
			err_print("failed to update dfs request");
			pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
		}
		return;
	}

	if(timer_pending(&(dev_power->dvfs_timeout))) {
		return;
	}

	/* get dvfs end ts */
	getnstimeofday(&(dev_power->dvfst2));
	if(dev_power->dev_is_on) {
		if(dev_power->dvfs_hw_on_dur < 0) {
			/* hw was on all the time during last timer phase */
			diff = timespec_sub(dev_power->dvfst2, dev_power->dvfst1);
			dev_power->dvfs_hw_on_dur = timespec_to_ns(&diff);
		} else {
			diff.tv_sec = 0;
			diff.tv_nsec = 0;
			diff = timespec_sub(dev_power->dvfst2, dev_power->ts1);
			dev_power->dvfs_hw_on_dur += timespec_to_ns(&diff);
		}
	}
	diff.tv_sec = 0;
	diff.tv_nsec = 0;
	diff = timespec_sub(dev_power->dvfst2, dev_power->dvfst1);
	temp = (int)(dev_power->dvfs_hw_on_dur/1000000);//convert hw_on_dur in millisecs
	temp2 = (int) ((unsigned long)timespec_to_ns(&diff)/1000000);//convert last dvfs timeslot in millisecs
	if(temp2 != 0)
	percnt = (temp*100)/temp2;
	else
		percnt = 100;

	if(percnt > 100)
		percnt = 100;

	dbg_print("dvfs_hw_on_dur %lu dvfs timeslot %lu percnt %d", dev_power->dvfs_hw_on_dur, (unsigned long)timespec_to_ns(&diff)/1000000, percnt);

	if(dev_power->dvfs_state.current_mode == NORMAL)
	{
		/* In addition to Hardware ON time, the Framework also uses the average Job time (in previous slot)
	   	  * and the number of pending jobs in the list will also be used to move to TURBO 
	   	  * (this shall not be used as a measure for coming back to NORMAL)*/
		// access dev_power->dvfs_jobs_done
		// access dev_power->dvfs_jobs_pend

		/* check if need to change to turbo mode*/
		if(percnt > dev_power->dvfs_state.params.P1)
		{
			dbg_print("change dvfs mode to turbo..\n");
			if(dev_power->dvfs_state.requested_mode != TURBO)
			{				
				dev_power->dvfs_state.requested_mode = TURBO;
				if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), 
						dev_power->dvfs_state.requested_mode))
				{
					err_print("failed to update dfs request");
					pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
				}
			}
		}
	}

	if(dev_power->dvfs_state.current_mode == TURBO)
	{
		/* check if need to change to normal mode*/
		if(percnt < dev_power->dvfs_state.params.P2)
		{
			if(dev_power->dvfs_state.requested_mode != NORMAL)
			{
				dbg_print("change dvfs mode to normal..\n");
				dev_power->dvfs_state.requested_mode = NORMAL;
				if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), 
						dev_power->dvfs_state.requested_mode))
				{
					err_print("failed to update dfs request");
					pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
				}
			}
		}
	}

	if(percnt!=0)
		dvfs_start_timer(dev_power);
	else 
	{
		if(dev_power->dvfs_state.timer_state)
		{
			del_timer_sync(&dev_power->dvfs_timeout);
			dev_power->dvfs_state.timer_state = false;
		}
	}
}

static void start_prof_timer(dev_power_t *dev_power, int secs)
{
	struct timespec diff;
	if(dev_power->dev_is_on) {
		getnstimeofday(&(dev_power->p1));
		diff.tv_sec = 0;
		diff.tv_nsec = 0;
		diff = timespec_sub(dev_power->p1, dev_power->ts1);
		dev_power->hw_on_duration = -(timespec_to_ns(&diff));
	} else {
		dev_power->hw_on_duration = 0;
	}
	mod_timer(&dev_power->dev_prof_timeout, jiffies+msecs_to_jiffies(secs*MSEC_PER_SEC));
	dbg_print("mod_timer for prof_timer %lx + %lx",jiffies,msecs_to_jiffies(secs*MSEC_PER_SEC));
}

static void dev_prof_work(struct work_struct* work)
{
	struct timespec diff;
	int percnt = 0;
	unsigned long temp = 0;
	dev_power_t *dev_power =
                container_of(work, dev_power_t, dev_prof_work);

	if(dev_power->dev_is_on) {
		getnstimeofday(&(dev_power->p2));
		if(dev_power->hw_on_duration < 0) {
			diff = timespec_sub(dev_power->p2, dev_power->p1);
			dev_power->hw_on_duration = timespec_to_ns(&diff);
		} else {
			diff.tv_sec = 0;
			diff.tv_nsec = 0;
			diff = timespec_sub(dev_power->p2, dev_power->ts1);
			dev_power->hw_on_duration += timespec_to_ns(&diff);
		}
	}
	temp = (unsigned long)(dev_power->hw_on_duration)/1000;
	percnt = (int)(temp/(unsigned long)(dev_power->secs*10000));
	err_print("hw_usage: ON : %d%% [DVFS:%d] JOBS : %d in %d secs ",  percnt,dev_power->dvfs_state.current_mode,dev_power->hw_prof_job_count, dev_power->secs);
	dev_power->hw_prof_job_count = 0;

	if(dev_power->prof_hw_usage)
		start_prof_timer(dev_power, dev_power->secs);
}

static void dev_prof_timeout_callback (unsigned long data)
{
	dev_power_t *dev_power = (dev_power_t *)data;
	queue_work(dev_power->mm_common->single_wq, &(dev_power->dev_prof_work));
}

static int dev_dbgfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static void profile_hw_usage(dev_power_t *dev_power, int secs)
{
	dev_power->secs = secs;
	init_timer(&(dev_power->dev_prof_timeout));
	setup_timer(&(dev_power->dev_prof_timeout), dev_prof_timeout_callback, (unsigned long)dev_power);
	start_prof_timer(dev_power, secs);
}

static ssize_t dbgfs_prof_handler(struct file *file, const char __user *buf,
        size_t count, loff_t *pos)
{
	/* get device power context srtruct */
	dev_power_t *dev_power = (dev_power_t *)(file->private_data);
    char *lbuf = NULL;
	char *str_endis = NULL;
	char *str_proftype = NULL;
	char *str_num = NULL;
	int number = 0;

    if (count >= MAX_BUF_LEN)
            return -EINVAL;

	lbuf = kmalloc(MAX_BUF_LEN, GFP_KERNEL);
    if (copy_from_user(lbuf, buf, count))
            return -EFAULT;
    lbuf[count] = '\0';

	str_endis = strsep(&lbuf, " ");
	str_proftype = strsep(&lbuf, " ");
	str_num = strsep(&lbuf, " ");

	if(!strcmp(str_endis, "enable")) {
		number = simple_strtol(str_num, NULL, 0);
		if(!strcmp(str_proftype, "hw_prof")) {
			err_print("enable hw_prof");
			dev_power->prof_hw_usage = true;
			dev_power->hw_prof_job_count = 0;
			profile_hw_usage(dev_power, number);
		}
		if(!strcmp(str_proftype, "job_prof")) {
			err_print("enable job_prof");
			dev_power->prof_job_cnt = true;
			dev_power->jp_job_count = number;
		}
	} else 	if(!strncmp(str_endis, "disable", 7)) {
		if(!strncmp(str_proftype, "hw_prof", 7)) {
			err_print("disable hw_prof");
			dev_power->prof_hw_usage = false;
			dev_power->hw_on_duration = 0;
			dev_power->hw_prof_job_count = 0;
			del_timer_sync(&(dev_power->dev_prof_timeout));
		}
		if(!strncmp(str_proftype, "job_prof", 8)) {
			err_print("disable job_prof");
			dev_power->prof_job_cnt = false;
			dev_power->jp_job_count = 0;
			dev_power->jp_ts1_noted = false;
		}
	}

	kfree(lbuf);
    return count;
}

static const struct file_operations dbgfs_prof_fops = {
	.open			= dev_dbgfs_open,
	.write			= dbgfs_prof_handler,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void dev_early_suspend(struct early_suspend *desc)
{
	dev_power_t *dev_power =
                container_of(desc, dev_power_t, early_suspend_desc);
	dev_power->dvfs_state.suspend_requested = true;
	queue_work(dev_power->mm_common->single_wq, &(dev_power->dvfs_work));
}

static void dev_late_resume(struct early_suspend *desc)
{
	dev_power_t *dev_power =
                container_of(desc, dev_power_t, early_suspend_desc);
	dev_power->dvfs_state.suspend_requested = false;
	queue_work(dev_power->mm_common->single_wq, &(dev_power->dvfs_work));
}
#endif

void dvfs_notification(dev_power_t* dev_power) {
	queue_work(dev_power->mm_common->single_wq, &(dev_power->dvfs_work));
}

DEFINE_DEBUGFS_HANDLER(on,dvfs_state.params.is_dvfs_on,0,1,default_scale,dev_power_t,dvfs_notification);
DEFINE_DEBUGFS_HANDLER(T1,dvfs_state.params.T1,50,500,default_scale,dev_power_t,dvfs_notification);
DEFINE_DEBUGFS_HANDLER(P1,dvfs_state.params.P1,60,99,default_scale,dev_power_t,dvfs_notification);
DEFINE_DEBUGFS_HANDLER(T2,dvfs_state.params.T2,3,20,dvfs_state.params.T1,dev_power_t,dvfs_notification);
DEFINE_DEBUGFS_HANDLER(P2,dvfs_state.params.P2,0,6,dvfs_state.params.P1/10,dev_power_t,dvfs_notification);

void* dev_power_init(mm_fmwk_common_t* mm_common, char *dev_name, MM_DVFS_HW_IFC *dvfs_params)
{
	int ret = 0;
	char debugfs_name[16] = "";
	dev_power_t *dev_power = kmalloc(sizeof(dev_power_t),GFP_KERNEL);
	memset(dev_power,0,sizeof(dev_power_t));

	dev_power->mm_common = mm_common;
	dev_power->default_scale = 1;

	dev_power->mm_fmwk_notifier.notifier_call = mm_pwr_notification;

	 atomic_notifier_chain_register(&mm_common->notifier_head, &dev_power->mm_fmwk_notifier);

	/* Init prof counters */
	dev_power->dvfs_state.params = *dvfs_params;
	dev_power->bulk_jobs_count = dvfs_params->dvfs_bulk_job_cnt;
	if(dev_power->dvfs_state.params.is_dvfs_on) {	
		dev_power->dvfs_state.requested_mode = NORMAL;	
		dev_power->dvfs_state.user_requested_mode = NORMAL;
		dev_power->dvfs_state.current_mode = NORMAL;
		}
	else {
		dev_power->dvfs_state.requested_mode = dvfs_params->user_requested_mode;	
		dev_power->dvfs_state.user_requested_mode = dvfs_params->user_requested_mode;
		dev_power->dvfs_state.current_mode = dvfs_params->user_requested_mode;
		}

	dev_power->debugfs_dir = debugfs_create_dir(dev_name, NULL);
    if(!dev_power->debugfs_dir)
	{
		err_print("Error %ld creating debugfs dir for %s\n",
                           PTR_ERR(dev_power->debugfs_dir), dev_name);
		ret = -ENOENT;
    }

	/* create debugfs file for profiling */
	strcat(debugfs_name, "prof");
    dev_power->prof_dbgfs_dentry = debugfs_create_file(debugfs_name, 
											(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
                                             dev_power->debugfs_dir,
                                             dev_power,
                                             &dbgfs_prof_fops);
	if(!dev_power->prof_dbgfs_dentry) {
		err_print("Error creating debugfs file %s/%s\n", dev_name, debugfs_name);
		debugfs_remove(dev_power->debugfs_dir);
		ret = -ENOENT;
    }

	dev_power->dvfs_dir = debugfs_create_dir("dvfs", dev_power->debugfs_dir);
    if(!dev_power->dvfs_dir)
	{
		err_print("Error %ld creating dvfs dir for %s\n",
                           PTR_ERR(dev_power->dvfs_dir), dev_name);
		ret = -ENOENT;
    }

	CREATE_DEBUGFS_FILE(dev_power,on,dev_power->dvfs_dir);
	CREATE_DEBUGFS_FILE(dev_power,T1,dev_power->dvfs_dir);
	CREATE_DEBUGFS_FILE(dev_power,P1,dev_power->dvfs_dir);
	CREATE_DEBUGFS_FILE(dev_power,T2,dev_power->dvfs_dir);
	CREATE_DEBUGFS_FILE(dev_power,P2,dev_power->dvfs_dir);

	ret =
	    pi_mgr_dfs_add_request(&(dev_power->dev_dfs_node), dev_name, PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		err_print("failed to register PI DFS request for %s\n", dev_name);
		return NULL;
	}

	dev_power->mm_dfs_chg_notify_blk.notifier_call = mm_dfs_chg_notifier;
    ret = pi_mgr_register_notifier(PI_MGR_PI_ID_MM,
                    &(dev_power->mm_dfs_chg_notify_blk),
                    PI_NOTIFY_DFS_CHANGE);
    if (ret != 0) {
            err_print("Failed to register PM Notifier\n");
            ret = -ENOENT;
    }

	/* start dvfs with normal*/
	if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), dev_power->dvfs_state.requested_mode))
	{
		err_print("failed to update dfs request for %s\n", dev_name);
		pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
        pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
                &(dev_power->mm_dfs_chg_notify_blk),
                PI_NOTIFY_DFS_CHANGE);
		return NULL;
	}

	INIT_WORK(&(dev_power->dev_prof_work), dev_prof_work);
	INIT_WORK(&(dev_power->dvfs_work), dvfs_work);

	init_timer(&(dev_power->dev_prof_timeout));
	setup_timer(&(dev_power->dev_prof_timeout), dev_prof_timeout_callback, (unsigned long)dev_power);

	if(dev_power->dvfs_state.params.enable_suspend_resume)
	{
#ifdef CONFIG_HAS_EARLYSUSPEND
	    dev_power->early_suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	    dev_power->early_suspend_desc.suspend = dev_early_suspend;
	    dev_power->early_suspend_desc.resume = dev_late_resume;
	    register_early_suspend(&dev_power->early_suspend_desc);
#endif
	}

	return dev_power;
}


void dev_power_exit( void *dev_p)
{
	dev_power_t* dev_power = (dev_power_t*)dev_p;
	if (dev_power->debugfs_dir)
		debugfs_remove_recursive(dev_power->debugfs_dir);
	if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), PI_MGR_DFS_MIN_VALUE))
	{
//		err_print("failed to update dfs request for %s", dev_name);
	}

	pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
			&(dev_power->mm_dfs_chg_notify_blk),
			PI_NOTIFY_DFS_CHANGE);

	pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
	dev_power->dev_dfs_node.name = NULL;

	del_timer(&(dev_power->dev_prof_timeout));
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&dev_power->early_suspend_desc);
#endif
}

