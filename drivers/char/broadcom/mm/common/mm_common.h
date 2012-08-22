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

#ifndef _MM_COMMON_H_
#define _MM_COMMON_H_

#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

#define TURBO_RATE 250
#define NORMAL_RATE 166

enum {
	MM_FMWK_NOTIFY_INVALID=0,
	MM_FMWK_NOTIFY_JOB_ADD,
	MM_FMWK_NOTIFY_JOB_REMOVE,
	MM_FMWK_NOTIFY_JOB_STARTED,
	MM_FMWK_NOTIFY_JOB_COMPLETE,

	MM_FMWK_NOTIFY_CLK_ENABLE,
	MM_FMWK_NOTIFY_CLK_DISABLE,

	MM_FMWK_NOTIFY_DVFS_UPDATE,
	
};

typedef struct mm_common {
	struct atomic_notifier_head notifier_head;
	struct miscdevice mdev;
	struct list_head list;

	/*Framework initializes the below parameters. 
	Do not edit in other files*/
	wait_queue_head_t queue;
	struct workqueue_struct *single_wq;
	char* single_wq_name;
	char* mm_name;

	/*HW status*/
	unsigned int mm_hw_is_on;
	struct clk *common_clk;

	void* mm_core[MAX_ASYMMETRIC_PROC];
	void* mm_dvfs;
	void* mm_prof;

    /* Used for exporting per-device information to debugfs */
    struct dentry *debugfs_dir;
}mm_common_t;

void mm_common_enable_clock(mm_common_t *common);
void mm_common_disable_clock(mm_common_t *common);


#endif
