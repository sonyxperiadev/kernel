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

#include "driver.h"


typedef struct {
	uint32_t physical;
	void    *virtual;
	int      in_use;
} v3d_bin_memory_t;

typedef enum {
	v3d_mode_user      = 0,
	v3d_mode_render    = 1,
	v3d_mode_undefined = 2
} v3d_mode_t;

struct v3d_device_tag;
typedef struct v3d_device_tag v3d_device_t;
struct v3d_device_tag {
	unsigned int initialised;
	struct {
		void __iomem *base;
	} registers;
	struct {
		v3d_driver_t *instance;
	} driver;

	unsigned int irq;
	v3d_mode_t mode;

	struct {
#define BIN_BLOCKS 32
#define BIN_BLOCK_BYTES (2 << 20)
		struct {
			unsigned int in_use;
			unsigned int allocated;
		} index;
		v3d_bin_memory_t         memory[BIN_BLOCKS];
		struct workqueue_struct *work_queue;
		struct work_struct       bin_allocation;
	} out_of_memory;

	struct {
		v3d_driver_job_t   *job;
		spinlock_t          lock;
	} in_progress;

	struct device      *device;

	struct task_struct *thread;

#ifdef WORKAROUND_GFXH16
	struct task_struct *gfxh16thread;
#endif

	volatile int           idle;

	struct pi_mgr_qos_node qos_node;
	struct pi_mgr_dfs_node dfs_node;
	struct clk            *clock;

	struct mutex suspend;
	struct mutex power;
	int          on;
	struct delayed_work switch_off;

	struct {
		uint32_t physical;
		void    *virtual;
	} dummy;

	ktime_t start_time;
};


extern v3d_device_t *v3d_device_create(
	v3d_driver_t *driver,
	struct device *device,
	uint32_t register_base,
	unsigned int irq);
extern void v3d_device_delete(v3d_device_t *instance);

/* Called to indicate that a job has been posted
   Jobs are fetched when required via calls to GetJob */
extern void v3d_device_job_posted(v3d_device_t *instance);

extern void v3d_device_job_cancel(v3d_device_t *instance, v3d_driver_job_t *job, int flush);

extern void v3d_device_suspend(v3d_device_t *instance);
extern void v3d_device_resume(v3d_device_t *instance);

extern void v3d_device_counters_enable(v3d_device_t *instance, uint32_t enables);
extern void v3d_device_counters_disable(v3d_device_t *instance);
extern void v3d_device_counters_add(v3d_device_t *instance, uint32_t *counters);


#endif /* ifndef V3D_DEVICE_H_ */
