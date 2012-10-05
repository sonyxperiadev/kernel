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

#ifndef V3D_DRIVER_H_
#define V3D_DRIVER_H_

#include <stddef.h> /* offsetof */
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/wait.h> /* wait_queue_head_t */
#include <linux/list.h>
#include <linux/kref.h>
#include <linux/broadcom/v3d.h>


struct V3dSessionTag;
struct task_struct;

typedef struct V3dDriver_JobTag {
	v3d_job_post_t         UserJob;
	struct list_head       Link;
	struct {
		struct list_head      Link;
		struct V3dSessionTag *Instance;
	} Session;
	wait_queue_head_t      WaitForCompletion;
	struct kref            Waiters;
#define V3DDRIVER_JOB_INITIALISED 0
#define V3DDRIVER_JOB_ISSUED      1
#define V3DDRIVER_JOB_ACTIVE      2
#define V3DDRIVER_JOB_COMPLETE    3
	volatile int           State;
	ktime_t                Queued;
	ktime_t                Start;
	ktime_t                End;
} V3dDriver_JobType;

struct V3dDeviceTag;
typedef struct {
	unsigned int Initialised;

	struct {
		struct {
			struct semaphore Count;
			spinlock_t       Lock;
			struct list_head List;
		} Free;
		struct {
			spinlock_t       Lock; /* For both lists */
			struct list_head BinRender;
			struct list_head User;
			struct {
				struct mutex          Lock;
				struct V3dSessionTag *Owner;
				unsigned int          BinRenderCount;
				unsigned int          UserCount;
				struct list_head      BinRender;
				struct list_head      User;
			} Exclusive;
		} Posted;

#define FREE_JOBS 128
		V3dDriver_JobType FreeJobs[FREE_JOBS];

#if 1
		struct {
			unsigned int Queue;
			unsigned int Run;
		} Times[45];
		unsigned int TimeIndex;
#endif
	} Job;

	struct V3dDeviceTag *Device;
} V3dDriverType;


extern V3dDriverType *V3dDriver_Create(void);
extern void V3dDriver_AddDevice(
	V3dDriverType       *Instance,
	struct V3dDeviceTag *Device);
extern void V3dDriver_Delete(V3dDriverType *Instance);

extern int  V3dDriver_JobPost(
	V3dDriverType *Instance,
	struct V3dSessionTag *Session,
	const v3d_job_post_t *UserJob);

extern void V3dDriver_ExclusiveStart(V3dDriverType *Instance, struct V3dSessionTag *Session);
extern int  V3dDriver_ExclusiveStop(V3dDriverType  *Instance, struct V3dSessionTag *Session);


/* For V3dDevice to use */
static inline unsigned long V3dDriver_JobLock(V3dDriverType *Instance)
{
	unsigned long Flags;
	spin_lock_irqsave(&Instance->Job.Posted.Lock, Flags);
	return Flags;
}
static inline void V3dDriver_JobUnlock(V3dDriverType *Instance, unsigned long Flags)
{
	spin_unlock_irqrestore(&Instance->Job.Posted.Lock, Flags);
}
extern V3dDriver_JobType *V3dDriver_JobGet(
	V3dDriverType *Instance,
	unsigned int   Required /* if non-zero, specifies job type required */);

/* For V3dSession use */
extern void V3dDriver_JobComplete(
	V3dDriverType     *Instance,
	V3dDriver_JobType *Job);


#endif /* ifndef V3D_DRIVER_H_ */
