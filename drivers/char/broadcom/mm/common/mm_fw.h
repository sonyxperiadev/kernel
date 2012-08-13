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
	
};

typedef struct mm_fmwk_common {
	struct atomic_notifier_head notifier_head;

	/*Framework initializes the below parameters. 
	Do not edit in other files*/
	struct workqueue_struct *single_wq;
	char* single_wq_name;

	/*HW status*/
	bool mm_hw_is_on;

    /* Used for exporting per-device information to debugfs */
    struct dentry *debugfs_dir;
}mm_fmwk_common_t;


void* mm_dvfs_init(mm_fmwk_common_t* mm_common, char *mm_dev_name, MM_DVFS_HW_IFC *dvfs_params);
void mm_dvfs_exit( void *mm_dvfs);

void* mm_prof_init(mm_fmwk_common_t* mm_common, char *mm_dev_name, MM_PROF_HW_IFC *prof_params);
void mm_prof_exit( void *mm_prof);


#endif
