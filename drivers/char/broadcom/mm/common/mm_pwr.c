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

#define MAX_BUF_LEN 72

static int dev_dfs_chg_notifier(struct notifier_block *self,
                               unsigned long event, void *data)
{
	PWR_DFS_CHG_NOTIFIER *tmp_container =
                container_of(self, PWR_DFS_CHG_NOTIFIER, dev_dfs_chg_notify_blk);
	struct device_power_t *dev_power = (struct device_power_t *)tmp_container->data;
    struct pi_notify_param *p = data;
	if(!dev_power){}

    if(p->pi_id != PI_MGR_PI_ID_MM){
		err_print("wrong PI_MGR_PI_ID\n");
		return -EINVAL;
    }
	dev_power->dvfs_state.current_mode = p->new_value;
	return 0;
}

void dev_clock_enable(device_t* dev)
{
	struct device_power_t *dev_power = &dev->dev_power;

	if(!(dev_power->dev_is_on))
	{
		clk_enable(dev_power->dev_clk);
		dev_power->dev_is_on = 1;
		dbg_print("dev turned on \n");
		clk_reset(dev_power->dev_clk);
		dev->dev_init(dev->device_id);
		init_timer(&(dev->dev_timeout));
		setup_timer(&(dev->dev_timeout), dev_timeout_callback, (unsigned long)dev);
		getnstimeofday(&dev_power->ts1);
		queue_work(dev_power->single_wq, &(dev_power->dvfs_work));
	}
}

void dev_clock_disable(device_t* dev)
{
	struct timespec diff;
	struct device_power_t *dev_power = &dev->dev_power;

	if(dev->dev_power.dev_is_on) {
		del_timer_sync(&(dev->dev_timeout));
		dbg_print("dev stayed on for %llu nanoseconds\n", (unsigned long long)timespec_to_ns(&diff));
		clk_disable(dev_power->dev_clk);
		dev->dev_power.dev_is_on = 0;
		getnstimeofday(&dev_power->ts2);
		diff.tv_sec = 0;
		diff.tv_nsec = 0;
		diff = timespec_sub(dev_power->ts2, dev_power->ts1);
		/* for profiling */
		dev_power->hw_on_duration += timespec_to_ns(&diff);
		/* for dvfs */
		dev_power->dvfs_hw_on_dur += timespec_to_ns(&diff);
	}
}

static void dvfs_timeout_callback (unsigned long data)
{
	struct device_power_t *dev_power = (struct device_power_t *)data;
	queue_work(dev_power->single_wq, &(dev_power->dvfs_work));
}

static void dvfs_start_timer(struct device_power_t *dev_power)
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
	struct device_power_t *dev_power =
                container_of(work, struct device_power_t, dvfs_work);
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
			dev_power->dvfs_state.user_requested_mode)
		{
			dev_power->dvfs_state.requested_mode = dev_power->dvfs_state.user_requested_mode;
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

static void start_prof_timer(struct device_power_t *dev_power, int secs)
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
	struct device_power_t *dev_power =
                container_of(work, struct device_power_t, dev_prof_work);

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
	struct device_power_t *dev_power = (struct device_power_t *)data;
	queue_work(dev_power->single_wq, &(dev_power->dev_prof_work));
}

static int dev_dbgfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static void profile_hw_usage(struct device_power_t *dev_power, int secs)
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
	struct device_power_t *dev_power = (struct device_power_t *)(file->private_data);
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

static ssize_t dbgfs_dvfs_handler(struct file *file, const char __user *buf,
        size_t count, loff_t *pos)
{
	/* get device power context srtruct */
	struct device_power_t *dev_power = (struct device_power_t *)(file->private_data);
    char *lbuf = NULL;
	char *str1 = NULL;
	char *str2 = NULL;
	char *str3 = NULL;
	int number = 0;

    if (count >= MAX_BUF_LEN)
            return -EINVAL;

	lbuf = kmalloc(MAX_BUF_LEN, GFP_KERNEL);
    if (copy_from_user(lbuf, buf, count))
            return -EFAULT;
    lbuf[count] = '\0';

	str1 = strsep(&lbuf, " ");
	str2 = strsep(&lbuf, " ");
	str3 = strsep(&lbuf, " ");

	if(!strncmp(str1, "enable", 6)) {
		err_print("enable dvfs..\n");
		dev_power->dvfs_state.params.is_dvfs_on = true;
	} else if(!strncmp(str1, "disable", 7)) {
		err_print("disable dvfs..\n");
		dev_power->dvfs_state.params.is_dvfs_on = false;
		if((!strncmp(str2, "NORMAL", 6))||(!strncmp(str2, "normal", 6))) {
			dev_power->dvfs_state.user_requested_mode = NORMAL;
		} else if((!strncmp(str2, "TURBO", 5))||(!strncmp(str2, "turbo", 5))) {
			dev_power->dvfs_state.user_requested_mode = TURBO;
		} else if((!strncmp(str2, "ECONOMY", 7))||(!strncmp(str2, "economy", 7))) {
			dev_power->dvfs_state.user_requested_mode = ECONOMY;
		}
		queue_work(dev_power->single_wq, &(dev_power->dvfs_work));
	}

	if(!strncmp(str1, "set", 3)) {
		number = simple_strtol(str3, NULL, 0);
		if(!strncmp(str2, "T1", 2)) {
			dev_power->dvfs_state.params.T1 = number;
		} else if(!strncmp(str2, "T2", 2)) {
			dev_power->dvfs_state.params.T2 = number;
		} else if(!strncmp(str2, "P1", 2)) {
			dev_power->dvfs_state.params.P1 = number;
		} else if(!strncmp(str2, "P2", 2)) {
			dev_power->dvfs_state.params.P2 = number;
		} else if(!strncmp(str2, "BLK_JOB_CNT", 11)) {
			dev_power->bulk_jobs_count = number;
		}
	}

	kfree(lbuf);
    return count;
}

static const struct file_operations dbgfs_prof_fops = {
	.open			= dev_dbgfs_open,
	.write			= dbgfs_prof_handler,
};

static const struct file_operations dbgfs_dvfs_fops = {
	.open			= dev_dbgfs_open,
	.write			= dbgfs_dvfs_handler,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void dev_early_suspend(struct early_suspend *desc)
{
	struct device_power_t *dev_power =
                container_of(desc, struct device_power_t, early_suspend_desc);
	dev_power->dvfs_state.suspend_requested = true;
	queue_work(dev_power->single_wq, &(dev_power->dvfs_work));
}

static void dev_late_resume(struct early_suspend *desc)
{
	struct device_power_t *dev_power =
                container_of(desc, struct device_power_t, early_suspend_desc);
	device_t *dev = 
				container_of(dev_power, device_t, dev_power);
	dev_power->dvfs_state.suspend_requested = false;
	queue_work(dev_power->single_wq, &(dev_power->dvfs_work));
}
#endif

int dev_power_init(struct device_power_t *dev_power, 
					  char *dev_name, 
					  const char *dev_clk_name, 
					  MM_DVFS *dvfs_params,
					  unsigned int bulk_job_count)
{
	int ret = 0;
	unsigned long rate = 0;
	char debugfs_name[16] = "";

	/* Init prof counters */
	dev_power->hw_on_duration = 0;
	dev_power->jp_job_count = 0;
	dev_power->prof_hw_usage = false;
	dev_power->secs = 0;
	dev_power->prof_job_cnt = false;
	dev_power->jp_ts1_noted = false;
	dev_power->hw_prof_job_count = 0;
	dev_power->dvfs_state.params.is_dvfs_on = dvfs_params->is_dvfs_on;
	dev_power->dvfs_state.params.user_requested_mode = dvfs_params->user_requested_mode;
	dev_power->dvfs_state.params.enable_suspend_resume = dvfs_params->enable_suspend_resume;
	dev_power->dvfs_state.params.T1 = dvfs_params->T1;
	dev_power->dvfs_state.params.P1 = dvfs_params->P1;
	dev_power->dvfs_state.params.T2 = dvfs_params->T2;
	dev_power->dvfs_state.params.P2 = dvfs_params->P2;
	dev_power->dvfs_hw_on_dur = 0;
	dev_power->bulk_jobs_count = bulk_job_count;
	dev_power->dvfs_jobs_pend = 0;
	dev_power->dvfs_state.suspend_requested = false;
	dev_power->dvfs_state.timer_state = false;
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
	/* clk disabled initially */
	dev_power->dev_is_on = 0;

//	scu_standby(false);

	dev_power->dev_clk = clk_get(NULL, dev_clk_name);
	if (!dev_power->dev_clk) {
		err_print("error get clock %s for %s dev\n", dev_clk_name, dev_name);
		ret = -EIO;
	}

	rate = clk_get_rate(dev_power->dev_clk);
	dbg_print("%s clk rate %lu\n", dev_clk_name, rate);

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

	/* for dvfs */
	strcpy(debugfs_name, "");
	strcat(debugfs_name, "dvfs");
    dev_power->dvfs_dbgfs_dentry = debugfs_create_file(debugfs_name,
											(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
											 dev_power->debugfs_dir,
											 dev_power,
											 &dbgfs_dvfs_fops);
	if(!dev_power->dvfs_dbgfs_dentry) {
		err_print("Error creating debugfs file %s/%s\n", dev_name, debugfs_name);
		debugfs_remove_recursive(dev_power->debugfs_dir);
		ret = -ENOENT;
    }

	dev_power->single_wq = alloc_ordered_workqueue("mm_pwr_wq",WQ_HIGHPRI);
	if (dev_power->single_wq == NULL) {
		ret = -ENOMEM;
	}

	ret =
	    pi_mgr_dfs_add_request(&(dev_power->dev_dfs_node), dev_name, PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		err_print("failed to register PI DFS request for %s\n", dev_name);
		return ret;
	}

	dev_power->pwr_dfs_chg_notifier.dev_dfs_chg_notify_blk.notifier_call = dev_dfs_chg_notifier;
	dev_power->pwr_dfs_chg_notifier.data = (void *)dev_power;
    ret = pi_mgr_register_notifier(PI_MGR_PI_ID_MM,
                    &(dev_power->pwr_dfs_chg_notifier.dev_dfs_chg_notify_blk),
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
                &(dev_power->pwr_dfs_chg_notifier.dev_dfs_chg_notify_blk),
                PI_NOTIFY_DFS_CHANGE);
		return -EIO;
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

	return ret;
}

void dev_power_exit(struct device_power_t *dev_power, const char *dev_name)
{
	if (dev_power->debugfs_dir)
		debugfs_remove_recursive(dev_power->debugfs_dir);
	destroy_workqueue(dev_power->single_wq);
	if (pi_mgr_dfs_request_update(&(dev_power->dev_dfs_node), PI_MGR_DFS_MIN_VALUE))
	{
		err_print("failed to update dfs request for %s", dev_name);
	}

	pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
			&(dev_power->pwr_dfs_chg_notifier.dev_dfs_chg_notify_blk),
			PI_NOTIFY_DFS_CHANGE);

	pi_mgr_dfs_request_remove(&(dev_power->dev_dfs_node));
	dev_power->dev_dfs_node.name = NULL;

	scu_standby(true);
	del_timer(&(dev_power->dev_prof_timeout));
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&dev_power->early_suspend_desc);
#endif
}

