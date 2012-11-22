/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef V3D_DEVICE_H_
#define V3D_DEVICE_H_

#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/ktime.h>

#include <plat/pi_mgr.h>

#include "Driver.h"


typedef struct {
	uint32_t Physical;
	void    *Virtual;
	int      InUse;
} V3dBinMemoryType;

typedef enum {
	V3dMode_User      = 0,
	V3dMode_Render    = 1,
	V3dMode_Undefined = 2
} V3dModeType;

struct V3dDeviceTag;
typedef struct V3dDeviceTag V3dDeviceType;
struct V3dDeviceTag {
	unsigned int Initialised;
	struct {
		uint32_t Base;
	} Register;
	struct {
		V3dDriverType *Instance;
	} Driver;

	V3dModeType Mode;

	struct {
		spinlock_t Lock;
#define BIN_BLOCKS 4
#define BIN_BLOCK_BYTES (2 << 20)
		struct {
			unsigned int InUse;
			unsigned int Allocated;
		} Index;
		V3dBinMemoryType         Memory[BIN_BLOCKS];
		struct workqueue_struct *WorkQueue;
		struct work_struct       BinAllocation;
	} OutOfMemory;

	struct {
		V3dDriver_JobType *BinRender;
#define V3D_USER_FIFO_LOG2_LENGTH 4
#define V3D_USER_FIFO_LENGTH (1 << V3D_USER_FIFO_LOG2_LENGTH)
		V3dDriver_JobType *User[V3D_USER_FIFO_LENGTH];
		int          Head;
		int          Tail;
		unsigned int LastCompleted;
	} InProgress;

	struct device      *Device;

	struct task_struct *Thread;

#ifdef WORKAROUND_GFXH16
	struct task_struct *Gfxh16Thread;
#endif

	volatile int           Idle;

	struct pi_mgr_qos_node QosNode;
	struct pi_mgr_dfs_node DfsNode;
	struct clk            *Clock;

	struct mutex Suspend;
	struct mutex Power;
	int          On;
	struct delayed_work SwitchOff;

	struct {
		uint32_t Physical;
		void    *Virtual;
	} Dummy;

	ktime_t StartTime;
};


extern V3dDeviceType *V3dDevice_Create(
	V3dDriverType *Driver,
	struct device *Device,
	uint32_t       RegisterBase);
extern void V3dDevice_Delete(V3dDeviceType *Instance);

/* Called to indicate that a job has been posted
   Jobs are fetched when required via calls to GetJob */
extern void V3dDevice_JobPosted(V3dDeviceType *Instance);

extern void V3dDevice_Suspend(V3dDeviceType *Instance);
extern void V3dDevice_Resume(V3dDeviceType *Instance);


#endif /* ifndef V3D_DEVICE_H_ */
