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

#define MAX_BUF_LEN 72

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
}mm_fmwk_common_t;

void* dev_power_init(mm_fmwk_common_t* mm_common, char *dev_name, MM_DVFS_HW_IFC *dvfs_params);
void dev_power_exit( void *dev_power);

#define DEFINE_DEBUGFS_HANDLER(name,parameter,MIN,MAX,SCALE,T,notify)			\
	static int mm_fmwk_debugfs_##name##_get(void* root, u64* param) {			\
		T* obj = (T*)root;														\
		*param = obj->parameter;												\
		return 0;																\
	}																			\
	static int mm_fmwk_debugfs_##name##_set(void* root, u64 param) {			\
		T* obj = (T*)root;														\
		if( (param >= ((MIN)*(obj->SCALE))) && 									\
			(param <= ((MAX)*(obj->SCALE)))) {									\
			obj->parameter = param;												\
			notify(obj);														\
			err_print("set to %d\n",obj->parameter);							\
			}																	\
		else {																	\
			err_print("valid values are from %d to %d\n",						\
						(MIN)*(obj->SCALE),(MAX)*(obj->SCALE) );				\
			}																	\
		return 0;																\
	}																			\
	DEFINE_SIMPLE_ATTRIBUTE(mm_fmwk_debugfs_##name,mm_fmwk_debugfs_##name##_get,\
									mm_fmwk_debugfs_##name##_set,#name" : %llu\n");\


#define CREATE_DEBUGFS_FILE(obj,name,dir)								\
	obj->name = debugfs_create_file(#name, 								\
									(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),\
									dir, obj, &mm_fmwk_debugfs_##name)

#endif
