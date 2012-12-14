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

#include <linux/kernel.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/gpio.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <plat/clock.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <mach/io_map.h>

#include "driver.h"
#include "device.h"
#include "session.h"

#define V3D_DEV_MAJOR	0
#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR
#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148

#ifdef V3D_PERF_SUPPORT
const char *perf_ctr_str[30] = {
	"FEP Valid primitives that result in no rendered pixels, for all rendered tiles",
	"FEP Valid primitives for all rendered tiles. (primitives may be counted in more than one tile)",
	"FEP Early-Z / Near / Far clipped quads",
	"FEP Valid quads",
	"TLB Quads with no pixels passing the stencil test",
	"TLB Quads with no pixels passing the Z and stencil tests",
	"TLB Quads with any pixels passing the Z and stencil tests",
	"TLB Quads with all pixels having zero coverage",
	"TLB Quads with any pixels having non-zero coverage",
	"TLB Quads with valid pixels written to colour buffer",
	"PTB Primitives discarded by being outside the viewport",
	"PTB Primitives that need clipping",
	"PSE Primitives that are discarded because they are reversed",
	"QPU Total idle clock cycles for all QPUs",
	"QPU Total clock cycles for all QPUs doing vertex/coordinate shading",
	"QPU Total clock cycles for all QPUs doing fragment shading",
	"QPU Total clock cycles for all QPUs executing valid instructions",
	"QPU Total clock cycles for all QPUs stalled waiting for TMUs",
	"QPU Total clock cycles for all QPUs stalled waiting for Scoreboard",
	"QPU Total clock cycles for all QPUs stalled waiting for Varyings",
	"QPU Total instruction cache hits for all slices",
	"QPU Total instruction cache misses for all slices",
	"QPU Total uniforms cache hits for all slices",
	"QPU Total uniforms cache misses for all slices",
	"TMU Total texture quads processed",
	"TMU Total texture cache misses (number of fetches from memory/L2cache)",
	"VPM Total clock cycles VDW is stalled waiting for VPM access",
	"VPM Total clock cycles VCD is stalled waiting for VPM access",
	"L2C Total Level 2 cache hits",
	"L2C Total Level 2 cache misses"
};
#endif

/******************************************************************
	V3D kernel prints
*******************************************************************/
#define KLOG_TAG __FILE__

/* Error Logs */
#if 1
#define KLOG_E(fmt, args...) \
					do { printk(KERN_ERR "error: [%s:%s:%d] "fmt"\n", KLOG_TAG, __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_E(x...) do {} while (0)
#endif
/* Debug Logs */
#if 1
#define KLOG_D(fmt, args...) \
		do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
		while (0)
#else
#define KLOG_D(x...) do {} while (0)
#endif
/* Verbose Logs */
#if 0
#define KLOG_V(fmt, args...) \
					do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_V(x...) do {} while (0)
#endif

/*
 *
 * Deferred V3D Task Serializer
 *
 */

/* TODO: consider separate file: dvts.c? */

typedef atomic_t dvts_jobcount_t;
typedef struct _dvts_object_ {
	wait_queue_head_t wq;
	dvts_jobcount_t finished;
} *dvts_object_t;
/* typedef uint32_t dvts_target_t; */
/*
 * Since the completion count is an atomic_t, which guarantees only
 * 24-bits, we need to be careful when comparing with the target,
 * which is 32-bits.  We need to do a 24-bit signed comparison, taking
 * into account wrap-around.  This macro makes it safe by doing
 * unsigned comparison with 23rd bit after masking.
 */
#define dvts_reached_target(obj, target) (((atomic_read(&(obj)->finished) - (target)) & 0xFFFFFF) < 0x800000)

dvts_object_t dvts_create_serializer(void)
{
	struct _dvts_object_ *object;

	object = kmalloc(sizeof(*object), GFP_KERNEL);
	if (object == NULL)
		goto err_kmalloc;

	init_waitqueue_head(&object->wq);
	atomic_set(&object->finished, 0);

	return object;

/*
 * error exit paths
 */
err_kmalloc:
	return NULL;
}

void dvts_destroy_serializer(dvts_object_t obj)
{
	kfree(obj);
}

void dvts_finish(dvts_object_t obj)
{
	atomic_inc(&obj->finished);
	/* TODO: make it wake_up_interruptible_all if/when we can
	 * demise dvts_wait() call */
	wake_up_all(&obj->wq);
}

int dvts_wait_interruptible(dvts_object_t obj, dvts_target_t target)
{
	int s;

	s = wait_event_interruptible(obj->wq,
			dvts_reached_target(obj, target));

	/* We'll return 0 if the dvts target was reached, and
	 * -ERESTARTSYS if the wait was interrupted by a signal */
	return s;
}

/******************************************************************
	END: V3D kernel prints
*******************************************************************/
#define USAGE_PRINT_THRESHOLD_USEC (5000000)
static int v3d_major = V3D_DEV_MAJOR;

typedef struct {
	dvts_object_t shared_dvts_object;
	uint32_t shared_dvts_object_usecount;
	char comm[TASK_COMM_LEN];
	v3d_session_t *session;

	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *proc_status;
	struct proc_dir_entry *proc_version;
} v3d_t;

static struct {
	struct class *v3d_class;
	struct device *v3d_device;
	v3d_device_t *v3d_device0;
	v3d_driver_t *v3d_driver;
} v3d_state;

v3d_driver_t *v3d_driver;

/********************************************************
	Imported stuff
********************************************************/
/* Job Error Handling variables */
#define V3D_ISR_TIMEOUT_IN_MS	(1500)
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS)

v3d_t *v3d_dev;

#define V3D_MAX_JOBS 128
/* Don't add anything at the beginning of this structure
   unless you are changing 'v3d_job_post_t' as well! */
typedef struct v3d_job_t_ {
	uint32_t job_type;
	uint32_t job_id;
	uint32_t v3d_ct0ca;
	uint32_t v3d_ct0ea;
	uint32_t v3d_ct1ca;
	uint32_t v3d_ct1ea;
	uint32_t v3d_vpm_size;
	uint32_t user_cnt;
	uint32_t v3d_srqpc[MAX_USER_JOBS];
	uint32_t v3d_srqua[MAX_USER_JOBS];
	uint32_t v3d_srqul[MAX_USER_JOBS];
	v3d_t *dev;
	volatile v3d_job_status_e job_status;
	u32 job_intern_state;
	u32 job_wait_state;
	wait_queue_head_t v3d_job_done_q;
	struct v3d_job_t_ *next;
} v3d_job_t;


/******************************************************************
	V3D driver functions
*******************************************************************/
static int v3d_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	v3d_t *dev;

	dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		goto kmalloc_fail;
	dev->session = v3d_session_create(v3d_state.v3d_driver, get_task_comm(dev->comm, current));
	if (dev->session == NULL) {
		printk(KERN_ERR "v3d_session_create() failed");
		goto session_fail;
	}

	filp->private_data = dev;

	dev->shared_dvts_object = dvts_create_serializer();
	if (!dev->shared_dvts_object)
		goto dvts_create_fail;

	dev->shared_dvts_object_usecount = 0;

	KLOG_V("%s for dev %p\n", __func__, dev);

end:
	return ret;
dvts_create_fail:
	v3d_session_delete(dev->session);
session_fail:
	kfree(dev);
kmalloc_fail:
	ret = -ENOMEM;
	goto end;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	if (dev->shared_dvts_object_usecount != 0) {
		/* unfortunately most apps don't close cleanly, so it
		   would be rude to pollute the log with this message
		   as an error... so we demote it to a verbose
		   warning.  TODO: can this be fixed? */
		KLOG_V("\n_shared deferred V3D task serializer use count > 0\n");
	}
	dvts_destroy_serializer(dev->shared_dvts_object);

	v3d_session_delete(dev->session);

	kfree(dev); /*Freeing NULL is safe here*/

	return 0;
}

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	KLOG_D("v3d_mmap called\n");
	return 0;
}

static long v3d_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_V3D_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > V3D_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (v3d_t *)(filp->private_data);
	switch (cmd) {
	case V3D_IOCTL_POST_JOB:
		{
			KLOG_V("v3d_ioctl :V3D_IOCTL_POST_JOB");
			ret = arg != 0 ? v3d_session_job_post(dev->session, (const v3d_job_post_t *)arg) : -EFAULT;
		}
		break;

	case V3D_IOCTL_WAIT_JOB:
		{
			v3d_job_status_t job_status = {0, 0, 0};
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_JOB");
			job_status.job_id = v3d_session_wait(dev->session);
			if (copy_to_user
				((v3d_job_status_t *)arg, &job_status,
				sizeof(job_status))) {
				KLOG_E
				    ("V3D_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EPERM;
			}
			KLOG_V("v3d_ioctl done :V3D_IOCTL_WAIT_JOB");
		}
		break;

	case V3D_IOCTL_ACQUIRE_EXCLUSIVE:
		v3d_driver_exclusive_start(v3d_state.v3d_driver, dev->session);
		break;

	case V3D_IOCTL_RELEASE_EXCLUSIVE:
		ret = v3d_driver_exclusive_stop(v3d_state.v3d_driver, dev->session);
		break;

	case V3D_IOCTL_DVTS_CREATE:
		{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_CREATE");
			/* theoretically we should create a new dvts
			 * object here and place it in some table, and
			 * return a handle or index to the caller.
			 * Right now, we don't care about multiple
			 * synchronization paths in parallel, so we
			 * can get away with just one dvts object.
			 * So, we just return a fake handle (777) and
			 * when we later get that id back in a job
			 * post we just use the shared one we already
			 * created upon device open */
			id = 777;
			if (copy_to_user((void *)arg, &id, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_CREATE copy_to_user failed");
				ret = -EPERM;
			}
			dev->shared_dvts_object_usecount += 1;
		}
		break;

	case V3D_IOCTL_DVTS_DESTROY:
		{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_DESTROY");
			if (copy_from_user(&id, (void *)arg, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_DESTROY copy_from_user failed");
				ret = -EPERM;
			}
			if (id != 777)
				ret = -ENOENT;
			dev->shared_dvts_object_usecount -= 1;
		}
		break;

	case V3D_IOCTL_DVTS_FINISH_TASK:
		{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_FINISH_TASK");
			if (copy_from_user(&id, (void *)arg, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_DESTROY copy_from_user failed");
				ret = -EPERM;
			}
			if (id != 777)
				ret = -ENOENT;
			dvts_finish(dev->shared_dvts_object);
		}
		break;

	case V3D_IOCTL_DVTS_AWAIT_TASK:
		{
			dvts_await_task_args_t await_task_args;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_AWAIT_TASK");
			if (copy_from_user
			    (&await_task_args, (void *)arg,
			     sizeof(await_task_args))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_AWAIT_TASK copy_from_user failed");
				ret = -EPERM;
			}
			if (await_task_args.id != 777)
				ret = -ENOENT;
			ret = dvts_wait_interruptible(
					dev->shared_dvts_object,
					await_task_args.target);
		}
		break;
	case V3D_IOCTL_PERF_COUNTER_READ:
		{
			const uint32_t *counts = v3d_session_counters_get(dev->session);
			ret = counts == NULL ? -EBUSY : 0;
			if (ret == 0)
				if (copy_to_user(
					(unsigned int *)arg,
					counts,
					16 * sizeof(*counts)) != 0) {
					KLOG_E("V3D_IOCTL_WAIT_JOB copy_to_user failed\n");
					ret = -EPERM;
				}
		}
		break;
	case V3D_IOCTL_PERF_COUNTER_ENABLE:
		{
			uint32_t enables;
			if (copy_from_user(&enables, (void *) arg, sizeof(enables))) {
				KLOG_E("V3D_IOCTL_PERF_COUNTER_ENABLE failed\n");
				ret = -EPERM;
			} else
				ret = v3d_session_counters_enable(dev->session, enables) == 0 ? 0 : -EALREADY;
		}
		break;
	case V3D_IOCTL_PERF_COUNTER_DISABLE:
		ret = v3d_session_counters_disable(dev->session) == 0 ? 0 : -EALREADY;
		break;

	default:
		KLOG_E("%s: unsupported %08x", __func__, cmd);
		break;
	}

	return ret;
}

static const struct file_operations v3d_fops = {
	.open = v3d_open,
	.release = v3d_release,
	.mmap = v3d_mmap,
	.unlocked_ioctl = v3d_ioctl,
};

/******************************************************************
	V3D proc functions
*******************************************************************/
#ifdef CONFIG_PM
static int v3d_suspend(struct platform_device *pdev, pm_message_t message)
{
	v3d_device_suspend(v3d_state.v3d_device0);
	return 0;
}

static int v3d_resume(struct platform_device *pdev)
{
	v3d_device_resume(v3d_state.v3d_device0);
	return 0;
}

#else

#define	v3d_suspend	NULL
#define	v3d_resume	NULL

#endif

static struct platform_driver v3d_platform_driver = {
	.driver = {
		.name = "V3D_PLATFORM"
	},
#ifdef CONFIG_PM
	.suspend = v3d_suspend,
	.resume  = v3d_resume
#endif
};

static struct platform_device v3d_platform_device = {
	.name = "V3D_PLATFORM",
	.id = -1,
};

int __init v3d_init(void)
{
	int ret = -1;

	KLOG_D("V3D driver init\n");

	/* initialize the V3D struct */
	memset(&v3d_state, 0, sizeof(v3d_state));

	ret = platform_device_register(&v3d_platform_device);
	if (ret) {
		KLOG_E("failed to register platform device\n");
		goto err;
	}

	ret = platform_driver_register(&v3d_platform_driver);
	if (ret) {
		KLOG_E("failed to register platform driver\n");
		goto err;
	}

	ret = register_chrdev(v3d_major, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0) {
		KLOG_E("failed to register char driver\n");
		ret = -EINVAL;
		goto err;
	} else
		v3d_major = ret;

	v3d_state.v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_state.v3d_class)) {
		KLOG_E("failed to create V3D class\n");
		ret = PTR_ERR(v3d_state.v3d_class);
		goto err3;
	}

	v3d_state.v3d_device =
	    device_create(v3d_state.v3d_class, NULL, MKDEV(v3d_major, 0), NULL,
			  V3D_DEV_NAME);
	v3d_state.v3d_device->coherent_dma_mask = ((u64)~0);

	v3d_state.v3d_driver = v3d_driver_create();
	if (v3d_state.v3d_driver == NULL) {
		KLOG_E("v3d_driver_create() failed");
		goto err4;
	}
	v3d_state.v3d_device0 = v3d_device_create(v3d_state.v3d_driver, v3d_state.v3d_device, RHEA_V3D_BASE_PERIPHERAL_ADDRESS, IRQ_GRAPHICS);
	if (v3d_state.v3d_device0 == NULL) {
		KLOG_E("v3d_device_create() failed");
		goto err5;
	}
	v3d_driver_add_device(v3d_state.v3d_driver, v3d_state.v3d_device0);

	v3d_driver = v3d_state.v3d_driver;

	return 0;

err5:
	v3d_driver_delete(v3d_state.v3d_driver);
err4:
	device_destroy(v3d_state.v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_state.v3d_class);
err3:
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
err:
	KLOG_E("V3D init error %d\n", ret);
	return ret;
}

void __exit v3d_exit(void)
{
	KLOG_D("V3D driver exit\n");

	v3d_driver = NULL;
	v3d_driver_delete(v3d_state.v3d_driver);
	v3d_device_delete(v3d_state.v3d_device0);

	device_destroy(v3d_state.v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_state.v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
