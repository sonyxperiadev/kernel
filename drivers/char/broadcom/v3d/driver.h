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

#include "session.h"
#include "statistics.h"

#define V3D_DEV_NAME	"v3d"
#define V3D_VERSION_STR	"2.0.0\n"

#if 0
#define MY_ASSERT(c) \
do { \
	if (!(c)) { \
		volatile int i = 0; \
		printk(KERN_ERR "Assert failed %s:%d: %s\n", __func__, __LINE__, #c); \
		while (i == 0) \
			; \
	} \
} while (0)
#else
#define MY_ASSERT(c) BUG_ON(!(c))
#endif


struct v3d_device_tag;
struct v3d_session_tag;
struct task_struct;

typedef struct v3d_driver_job_tag {
	v3d_job_post_t         user_job;
	struct list_head       link;
	struct {
		struct list_head      link;
		struct v3d_session_tag *instance;
	} session;
	struct v3d_device_tag   *device;
	wait_queue_head_t      wait_for_completion;
	struct kref            waiters;
#define V3DDRIVER_JOB_INVALID     0
#define V3DDRIVER_JOB_INITIALISED 1
#define V3DDRIVER_JOB_ISSUED      2
#define V3DDRIVER_JOB_ACTIVE      3
#define V3DDRIVER_JOB_COMPLETE    4
#define V3DDRIVER_JOB_FAILED      5
	volatile int           state;
	ktime_t                queued;
	ktime_t                start;
	ktime_t                end;
	unsigned int           binning_bytes;

	struct {
		int      enable;
		uint32_t count[16];
	} performance_counter;
} v3d_driver_job_t;

struct v3d_device_tag;
typedef struct v3d_driver_tag {
	unsigned int initialised;

	struct {
		struct {
			struct semaphore count;
			spinlock_t       lock;
			struct list_head list;
		} free;
		struct {
			spinlock_t       lock; /* For both lists */
			struct list_head bin_render;
			struct list_head user;
			struct {
				struct mutex          lock;
				struct v3d_session_tag *owner;
				unsigned int          bin_render_count;
				unsigned int          user_count;
				struct list_head      bin_render;
				struct list_head      user;
			} exclusive;
		} posted;

#define FREE_JOBS 128
		v3d_driver_job_t free_jobs[FREE_JOBS];
	} job;

	ktime_t        start;
	uint32_t       total_run;
	struct {
		statistics_t queue;
		statistics_t run;
		statistics_t binning_bytes;
	} bin_render;
	struct {
		statistics_t queue;
		statistics_t run;
	} user;

	struct v3d_device_tag *device;

#define MAX_SESSIONS 64
	struct v3d_session_tag *sessions[MAX_SESSIONS];

	struct {
		unsigned int           initialised;
		struct proc_dir_entry *directory;
		struct proc_dir_entry *status;
		struct proc_dir_entry *session;
		struct proc_dir_entry *version;
	} proc;
} v3d_driver_t;


extern v3d_driver_t *v3d_driver_create(void);
extern void v3d_driver_add_device(
	v3d_driver_t       *instance,
	struct v3d_device_tag *device);
extern void v3d_driver_delete(v3d_driver_t *instance);

extern int  v3d_driver_job_post(
	v3d_driver_t *instance,
	struct v3d_session_tag *session,
	const v3d_job_post_t *user_job);

extern void v3d_driver_exclusive_start(v3d_driver_t *instance, struct v3d_session_tag *session);
extern int  v3d_driver_exclusive_stop(v3d_driver_t  *instance, struct v3d_session_tag *session);

extern void v3d_driver_reset_statistics(v3d_driver_t *instance);

/* For V3dDevice to use */
static inline unsigned long v3d_driver_job_lock(v3d_driver_t *instance)
{
	unsigned long flags;
	spin_lock_irqsave(&instance->job.posted.lock, flags);
	return flags;
}
static inline void v3d_driver_job_unlock(v3d_driver_t *instance, unsigned long flags)
{
	spin_unlock_irqrestore(&instance->job.posted.lock, flags);
}
extern v3d_driver_job_t *v3d_driver_job_get(
	v3d_driver_t *instance,
	unsigned int   required /* if non-zero, specifies job type required */);

/* For V3dSession use */
extern void v3d_driver_job_complete(
	v3d_driver_t     *instance,
	v3d_driver_job_t *job);
extern int v3d_driver_add_session(v3d_driver_t *instance, struct v3d_session_tag *session);
extern void v3d_driver_remove_session(v3d_driver_t *instance, struct v3d_session_tag *session);
extern void v3d_driver_kick_consumers(v3d_driver_t *instance);

/* Performance counters */
extern void v3d_driver_counters_enable(v3d_driver_t *instance, uint32_t enables);
extern void v3d_driver_counters_disable(v3d_driver_t *instance);

/* For V3dDriver internal use */
extern int  v3d_driver_create_proc_entries(v3d_driver_t *instance);
extern void v3d_driver_delete_proc_entries(v3d_driver_t *instance);


#endif /* ifndef V3D_DRIVER_H_ */
