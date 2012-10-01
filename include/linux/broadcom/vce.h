/*******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _VCE_H_
#define _VCE_H_
#include <linux/ioctl.h>

#define VCE_DEV_NAME	"vce"
#define BCM_VCE_MAGIC	(('V'<<3 ^ 'C'<<1 ^ 'E'<<0)&_IOC_TYPEMASK)
#ifdef __KERNEL__
#endif

enum {
	VCE_CMD_XXYYZZ = 0x80,
	VCE_CMD_WAIT_IRQ,
	VCE_CMD_EXIT_IRQ_WAIT,
	VCE_CMD_HW_ACQUIRE,
	VCE_CMD_HW_RELEASE,
	VCE_CMD_RESET,
	VCE_CMD_ASSERT_IDLE,
	VCE_CMD_USE_ACP,
	VCE_CMD_UNUSE_ACP,
	VCE_CMD_UNINSTALL_ISR,
	VCE_CMD_LAST,

	/* vtq */

	VTQ_CMD_XXYYZZ = 0x60,
	VTQ_CMD_CONFIGURE,
	VTQ_CMD_REGISTER_IMAGE,
	VTQ_CMD_DEREGISTER_IMAGE,
	VTQ_CMD_CREATE_TASK_RAW,
	VTQ_CMD_CREATE_TASK,
	VTQ_CMD_DESTROY_TASK,
	VTQ_CMD_TASK_ADD_ENTRYPOINT,
	VTQ_CMD_QUEUE_JOB,
	VTQ_CMD_AWAIT_JOB,
	VTQ_CMD_WHAT_WAS_LAST_QUEUED_JOB,
	VTQ_CMD_FLUSH, /* flush is equivalent to await(last_queued) */
	VTQ_CMD_ONLOADHOOK,
	VTQ_CMD_QUEUE_JOB_MULTIPLE,
	VTQ_CMD_MULTIPURPOSE_LOCK,
	VTQ_CMD_IMAGECONV_GETTASKS,
	VTQ_CMD_IMAGECONV_READY,
	VTQ_CMD_IMAGECONV_ENQUEUE_DIRECT,
	VTQ_CMD_IMAGECONV_AWAIT,
	VTQ_CMD_REGISTER_IMAGE_BLOB,
	VTQ_CMD_LAST,

	/* debug */

	VCE_CMD_DEBUG_XXYYZZ = 0x40,
	VCE_CMD_DEBUG_FETCH_KSTAT_IRQS,
	VCE_CMD_DEBUG_LOW_LATENCY_HACK,
	VCE_CMD_DEBUG_LAST
};

struct vtq_configure_ioctldata {
	/*
	 * inputs
	 */

	/* TODO: we'll want some way to authenticate the request to
	 * configure.  We reserve space in the ioctl command so that
	 * if/when we get around to this, we don't break stuff. */
	void *reserved0;

	/* Base address of the imageloader image (a.k.a. firmware /
	 * kernel) */
	uint32_t /*progmemoffset*/ loader_base;

	/* First entry point - used when ARM first tells VCE there's
	 * some work for him */
	uint32_t /*progmemoffset*/ loader_run;

	/* Entrypoint of the "load next image" function -- reserved
	 * for future */
	uint32_t /*progmemoffset*/ loadimage_entrypoint;

	/* There will undoutedly be further fixed entry points we will
	 * need in the future and I want this ioctl to have some
	 * longevity, so we reserve space for them here.  Must pass 0
	 * for compatibility when these are not used. */
	uint32_t aux1;
	uint32_t aux2;
	uint32_t aux3;

	/* Relocated text of the imageloader image */
	uint32_t *loader_text;
	size_t loader_textsz;

	/* Start address of DATA memory reservation */
	uint32_t /*datamemoffset*/ datamem_reservation;

	/* Address in LDM of VCE's copy of the write pointer (owned by
	 * ARM) */
	uint32_t /*datamemoffset*/ writepointer_locn;

	/* Address in LDM of read pointer (owned by VCE) */
	uint32_t /*datamemoffset*/ readpointer_locn;

	/* Address of the start of the FIFO */
	uint32_t /*datamemoffset*/ fifo_offset;

	/* Number of entries in the FIFO */
	size_t fifo_length;

	/* Size of each FIFO entry in bytes */
	size_t fifo_entry_size;

	/* Which semaphore to use */
	uint32_t semaphore_id;
};

struct vtq_registerimage_ioctldata {
	/* result */
	int image_id;

	/* inputs */
	const uint32_t *text;
	size_t textsz;
	const uint32_t *data;
	size_t datasz;
	size_t datamemreq;
};

struct vtq_deregisterimage_ioctldata {
	/* input */
	int image_id;
};

struct vtq_createtask_ioctldata {
	/* result */
	int task_id;

	/* inputs */
	int image_id;
	uint32_t entrypoint;

	/* task flags are not used currently, but we want to allow
	 * them in future without breaking the API */
	uint32_t flags;
};

struct vtq_createtask_ioctldata_noflags {
	/* result */
	int task_id;

	/* inputs */
	int image_id;
	uint32_t entrypoint;
};

struct vtq_destroytask_ioctldata {
	/* input */
	int task_id;
};

struct vtq_queuejob_ioctldata {
	/* result */
	uint32_t job_id;

	/* input */
	int task_id;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint32_t arg4;
	uint32_t arg5;

	/* job flags are not used currently, but we want to allow them
	 * in future without breaking the API */
	uint32_t flags;
};

struct vtq_queuejob_ioctldata_noflags {
	/* result */
	uint32_t job_id;

	/* input */
	int task_id;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint32_t arg4;
	uint32_t arg5;
};

struct vtq_onloadhook_ioctldata {
	/* input */
	uint32_t pc;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
};

struct vtq_awaitjob_ioctldata {
	/* input */
	uint32_t job_id;
};

#define VTQ_BLOB_ID_IMAGECONV 0
struct vtq_registerimage_blob_ioctldata {
	/* result */
	int result;

	/* inputs */
	int blobid;
	const uint32_t *blob;
	size_t blobsz;
};

struct vtq_imageconv_gettasks_ioctldata {
	/* result */
	int task_direct;
	int task_bgr24;
	int task_resv1;
	int task_resv2;
	int task_resv3;
};

struct vtq_imageconv_ready_ioctldata {
	/* result */
	int isready;
};

struct vtq_imageconv_enqueue_direct_ioctldata {
	/* result */
	uint32_t job_id;

	/* input */
	uint32_t type;
	uint32_t tformat_baseaddr;
	uint32_t raster_baseaddr;
	uint32_t signedrasterstride;
	uint32_t numtiles_wide;
	uint32_t numtiles_high;
	uint32_t dmacfg;
};

struct vtq_imageconv_await_ioctldata {
	/* input */
	uint32_t job_id;
};

/*
 * reserved
 */
#define VTQ_LOCK_ARMLOCK 1
/*
 * Power Lock stops the unload/release work from running, which has
 * the effect of keeping VCE powered on, but also reduces contention
 * for the power mutex (currently, host_mutex, multipurposed) which
 * will improve performance in the "fifo *just* goes empty, briefly"
 * usecase.  Do *not* use this lock in any other usecase.
 */
#define VTQ_LOCK_POWERLOCK 2

/*
 * low priority lock can be acquired multiple times.  It will always
 * be granted as long as there is no outstanding request for a medium
 * priority lock.  Users of low priority lock get shared access.  With
 * the lock, they acquire the right to enter short-running jobs into
 * the queue and can be assured that their job will be serviced in a
 * reasonable time as their will not be a medium priority long-running
 * job in the way.  Attempts to acquire the lock while there is a
 * medium priority lock held or requested will fail immediately
 */
#define VTQ_LOCK_PRIORITY_LOCK_LOW 0x100
/*
 * A medium priority lock will always be granted, but will block until
 * all low-priority locks have been released.  The owner gains the
 * right to submit a long running job to the VTQ in the knowledge that
 * no low-priority jobs shall get in its way */
#define VTQ_LOCK_PRIORITY_LOCK_MED 0x200
/*
 * reserved
 */
#define VTQ_LOCK_PRIORITY_LOCK_HIGH 0x400

struct vtq_multipurposelock_ioctldata {
	/* result */
	/* uint32_t; */

	/* input */
	uint32_t locks_to_get;
	uint32_t locks_to_put;
	uint32_t flags;
};

/* TODO: review (these were copy/psted from v3d) */
#define VCE_IOCTL_WAIT_IRQ      _IO(BCM_VCE_MAGIC, VCE_CMD_WAIT_IRQ)
#define VCE_IOCTL_EXIT_IRQ_WAIT _IO(BCM_VCE_MAGIC, VCE_CMD_EXIT_IRQ_WAIT)
#define VCE_IOCTL_RESET         _IO(BCM_VCE_MAGIC, VCE_CMD_RESET)
#define VCE_IOCTL_HW_ACQUIRE    _IO(BCM_VCE_MAGIC, VCE_CMD_HW_ACQUIRE)
#define VCE_IOCTL_HW_RELEASE    _IO(BCM_VCE_MAGIC, VCE_CMD_HW_RELEASE)
#define VCE_IOCTL_USE_ACP       _IO(BCM_VCE_MAGIC, VCE_CMD_USE_ACP)
#define VCE_IOCTL_UNUSE_ACP     _IO(BCM_VCE_MAGIC, VCE_CMD_UNUSE_ACP)
#define VCE_IOCTL_ASSERT_IDLE   _IO(BCM_VCE_MAGIC, VCE_CMD_ASSERT_IDLE)
#define VCE_IOCTL_UNINSTALL_ISR _IO(BCM_VCE_MAGIC, VCE_CMD_UNINSTALL_ISR)
#define VTQ_IOCTL_CONFIGURE     _IOWR(BCM_VCE_MAGIC,			\
				      VTQ_CMD_CONFIGURE,		\
				       struct vtq_configure_ioctldata)
#define VTQ_IOCTL_REGISTER_IMAGE _IOWR(BCM_VCE_MAGIC,			\
				       VTQ_CMD_REGISTER_IMAGE,		\
				       struct vtq_registerimage_ioctldata)
#define VTQ_IOCTL_DEREGISTER_IMAGE _IOW(BCM_VCE_MAGIC,			\
					VTQ_CMD_DEREGISTER_IMAGE,	\
					struct vtq_registerimage_ioctldata)
#define VTQ_IOCTL_CREATE_TASK   _IOWR(BCM_VCE_MAGIC,			\
				      VTQ_CMD_CREATE_TASK,		\
				      struct vtq_createtask_ioctldata)
#define VTQ_IOCTL_CREATE_TASK_NOFLAGS _IOWR(BCM_VCE_MAGIC,		\
				      VTQ_CMD_CREATE_TASK,		\
					struct vtq_createtask_ioctldata_noflags)
#define VTQ_IOCTL_DESTROY_TASK   _IOW(BCM_VCE_MAGIC,			\
				      VTQ_CMD_DESTROY_TASK,		\
				      struct vtq_destroytask_ioctldata)
#define VTQ_IOCTL_QUEUE_JOB     _IOWR(BCM_VCE_MAGIC,			\
				      VTQ_CMD_QUEUE_JOB,		\
				      struct vtq_queuejob_ioctldata)
#define VTQ_IOCTL_QUEUE_JOB_NOFLAGS _IOWR(BCM_VCE_MAGIC,		\
				      VTQ_CMD_QUEUE_JOB,		\
				      struct vtq_queuejob_ioctldata_noflags)
#define VTQ_IOCTL_AWAIT_JOB     _IOW(BCM_VCE_MAGIC,			\
				     VTQ_CMD_AWAIT_JOB,			\
				     struct vtq_awaitjob_ioctldata)
#define VTQ_IOCTL_ONLOADHOOK _IOWR(BCM_VCE_MAGIC,			\
				   VTQ_CMD_ONLOADHOOK,			\
				   struct vtq_onloadhook_ioctldata)
#define VTQ_IOCTL_MULTIPURPOSE_LOCK _IOW(BCM_VCE_MAGIC,			\
				      VTQ_CMD_MULTIPURPOSE_LOCK,	\
				      struct vtq_multipurposelock_ioctldata)
#define VTQ_IOCTL_REGISTER_IMAGE_BLOB _IOWR(BCM_VCE_MAGIC,		\
				       VTQ_CMD_REGISTER_IMAGE_BLOB,	\
				       struct vtq_registerimage_blob_ioctldata)
/* TODO: the imageconv ioctls don't really belong among the rest of
 * the VTQ ioctls, but as we have a bit of a hybrid interface to this
 * at the moment, it will do for now */
#define VTQ_IMAGECONV_IOCTL_GETTASKS _IOR(BCM_VCE_MAGIC,                \
		VTQ_CMD_IMAGECONV_GETTASKS,                             \
		struct vtq_imageconv_gettasks_ioctldata)
#define VTQ_IMAGECONV_IOCTL_READY _IOR(BCM_VCE_MAGIC,			\
		VTQ_CMD_IMAGECONV_READY,				\
		struct vtq_imageconv_ready_ioctldata)
#define VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT _IOR(BCM_VCE_MAGIC,		\
		VTQ_CMD_IMAGECONV_ENQUEUE_DIRECT,			\
		struct vtq_imageconv_enqueue_direct_ioctldata)
#define VTQ_IMAGECONV_IOCTL_AWAIT _IOR(BCM_VCE_MAGIC,			\
		VTQ_CMD_IMAGECONV_AWAIT,				\
		struct vtq_imageconv_await_ioctldata)

#define VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS  _IOR(BCM_VCE_MAGIC, VCE_CMD_DEBUG_FETCH_KSTAT_IRQS, unsigned int)
#define VCE_IOCTL_DEBUG_LOW_LATENCY_HACK  _IO(BCM_VCE_MAGIC, \
		VCE_CMD_DEBUG_LOW_LATENCY_HACK)

#endif
