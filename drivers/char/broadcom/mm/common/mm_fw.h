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

#ifndef _MM_FW_H_
#define _MM_FW_H_

#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

#define MAX_DEVICES 10
#define MISC_DYNAMIC_MINOR 255

typedef struct {
	dvfs_mode_e user_requested_mode; // forced request state 
	dvfs_mode_e requested_mode; // updated based on profiling and requested to Power Manger
	dvfs_mode_e current_mode; //updated in DVFS callback from Power Manager
	bool suspend_requested; // volatile flag updated in early-suspend/late-resume
	bool timer_state; // DVFS timer state (initialized/unintialized)
	MM_DVFS params; // DVFS tunable parameters
} MM_DVFS_STATE;

typedef struct {
	struct notifier_block dev_dfs_chg_notify_blk;
	void *data;
}PWR_DFS_CHG_NOTIFIER;

typedef struct {
	struct miscdevice mdev;
	int dev_major;
	struct class *dev_class;
	volatile void __iomem *dev_base;
	struct timespec sched_time;
	/* tasklets */
	struct work_struct dev_tasklet;	
	struct timer_list dev_timeout;
	wait_queue_head_t queue;
	/* job list */
	struct list_head job_list;
	uint32_t device_job_id;
	/* device funcs */
	int (*dev_init)(void *device_id);
	int (*dev_deinit)(void *device_id);
	bool (*get_hw_status)(void *device_id);
	mm_job_status_e (*dev_start_job)(void *device_id, mm_job_post_t *job);
	mm_isr_type_e (*process_irq)(void *device_id);
	int (*dev_abort)(void *device_id, mm_job_post_t *job);
	int (*dev_print_regs)(void *device_id);
	/* device specific data */
	void *device_id;
	struct device_power_t {
		struct clk *dev_clk;
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
		struct workqueue_struct *single_wq;
		struct work_struct dev_prof_work;
	    /* Used for exporting per-device information to debugfs */
	    struct dentry *debugfs_dir;
    	/* File in the debugfs directory that exposes this device's information */
	    struct dentry *prof_dbgfs_dentry;
	    struct dentry *dvfs_dbgfs_dentry;
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
		PWR_DFS_CHG_NOTIFIER pwr_dfs_chg_notifier;
#ifdef CONFIG_HAS_EARLYSUSPEND
        struct early_suspend early_suspend_desc;
#endif
	}dev_power;
}device_t;


void dev_timeout_callback (unsigned long data);

#endif
