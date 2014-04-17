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


#define pr_fmt(fmt) "h264_secure: %24s: " fmt,  __func__


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>


enum {
	CORE_H264_CME_E = 0,
	CORE_H264_MCIN_E,
	CORE_H264_CABAC_E,
	CORE_H264_VCE_E,
	CORE_H264_OL_E,
	CORE_H264_MAX_CORES_E,
};

/**
 * Software states which should be matching the hardware state.
 * Note: Aborting the job does not abort the hardware, the assumption is
 * that abort happened due to software lockup between secure and non-secure
 * interaction and not a hardware lockup and that hardware was idle.
 */
enum mm_hw_state_e_ {
	MM_HW_STATE_FREE_E,
	MM_HW_STATE_BUSY_E,
};
#define mm_hw_state_e enum mm_hw_state_e_


/**
 * Secure device context.
 * This could get updated asynchronously (from process context) and not from
 * workque and so locks are used.
 */
struct secure_device_t_ {
	int                 core_id;
	struct completion   job_available;
	mm_job_post_t       *curr_job;
	mm_job_status_e     job_status;
	mm_hw_state_e       state;
};
#define secure_device_t   struct secure_device_t_
#define secure_device_ptr struct secure_device_t_ *


/**
 * Array of secure device context pointers - one per core
 */
static secure_device_ptr secure_device[CORE_H264_MAX_CORES_E];


/**
 * Called from ioctl directly (process context) and not as
 * part of mm workque. This will remain blocked till a new
 * secure job gets scheduled.
 * This gets unblocked when a new job gets scheduled, makes
 * a reference of the job internally and passes the job info
 * including the type and job cookie back to userspace.
 */
static int secure_job_wait(void *device_id,
		mm_secure_job_ptr p_secure_job)
{
	mm_job_post_t      *curr_job;
	secure_device_ptr  device = (secure_device_ptr)device_id;

	pr_debug("core(%d)\n", device->core_id);
	/* wait for job post */
	if (0 != wait_for_completion_interruptible(&device->job_available)) {
		pr_err("core(%d) wait interrupted\n", device->core_id);
		return -ERESTARTSYS;
	}
	pr_debug("core(%d) wait over\n", device->core_id);

	curr_job = device->curr_job;
	if (NULL != curr_job) {
		p_secure_job->type   = curr_job->type;
		p_secure_job->id     = curr_job->id;
		p_secure_job->status = curr_job->status;
	} else {
		pr_err("core(%d) Job not found\n", device->core_id);
		p_secure_job->status = MM_JOB_STATUS_NOT_FOUND;
		/* Loop internally ? */
	}

	return 0;
}


/**
 * Called from ioctl directly (process context) and not as
 * part of mm workque.
 * This receives the job information as part of ioctl data
 * and updates the internal state of the the device and job.
 * The readback of status is done via standard mm scheduler.
 */
static int secure_job_done(void *device_id,
		mm_secure_job_ptr p_secure_job)
{
	secure_device_ptr  device = (secure_device_ptr)device_id;

	pr_debug("core(%d)\n", device->core_id);
	if (NULL == device->curr_job) {
		pr_err("Curr job is set to NULL\n");
	} else if (p_secure_job->id != device->curr_job->id) {
		pr_err("Job-id signalled does not match\n");
	} else {
		device->state        = MM_HW_STATE_FREE_E;
		device->job_status   = p_secure_job->status;
	}
	return 0;
}


static int secure_abort(void *device_id, mm_job_post_t *job)
{
	secure_device_ptr  device = (secure_device_ptr)device_id;

	pr_err("core(%d)\n", device->core_id);
	if (job != device->curr_job) {
		pr_err("Curr job does not match\n");
	} else if (job->id != device->curr_job->id) {
		pr_err("Job-id signalled does not match\n");
	} else {
		device->state        = MM_HW_STATE_FREE_E;
		device->job_status   = MM_JOB_STATUS_ERROR;
	}
	return 0;
}


static mm_job_status_e secure_start_job(void *device_id,
				mm_job_post_t *job, u32 profmask)
{
	secure_device_ptr  device = (secure_device_ptr)device_id;
	mm_job_status_e    status = MM_JOB_STATUS_ERROR;

	pr_debug("core(%d) job(%p) status(%d)\n", device->core_id, job,
			job->status);

	switch (job->status) {

	case MM_JOB_STATUS_READY:
		/* share the job details with user thread */
		device->curr_job   = job;
		status             = MM_JOB_STATUS_RUNNING;
		job->status        = status;
		device->state      = MM_HW_STATE_BUSY_E;
		complete(&device->job_available);
		break;

	case MM_JOB_STATUS_RUNNING:
		/* update the job status as complete */
		if (job != device->curr_job)
			pr_err("Curr job does not match\n");
		else if (job->id != device->curr_job->id)
			pr_err("Job-id signalled does not match\n");
		status             = device->job_status;
		job->status        = status;
		device->curr_job   = NULL;
		break;

	default:
		pr_err("Unexpected job_status(%d)\n", job->status);
		break;

	}

	return status;
}

static bool secure_get_status(void *device_id)
{
	secure_device_ptr  device = (secure_device_ptr)device_id;
	bool               status = false;
	pr_debug("core(%d)\n", device->core_id);

	/* Return hardware status as busy based on software state
	 * as register access is not possible */
	if (MM_HW_STATE_BUSY_E == device->state)
		status = true;

	return status;
}

static void secure_update_virt(void *virt)
{
	pr_debug("\n");
}

static int secure_init(void *device_id)
{
	secure_device_ptr device = (secure_device_ptr)device_id;
	pr_debug("core(%d)\n", device->core_id);
	return 0;
}

static int secure_deinit(void *device_id)
{
	secure_device_ptr device = (secure_device_ptr)device_id;
	pr_debug("core(%d)\n", device->core_id);
	return 0;
}

static int secure_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	secure_device_ptr device = (secure_device_ptr)device_id;
	pr_err("core(%d)\n", device->core_id);
	return 0;
}

static mm_isr_type_e secure_process_irq(void *device_id)
{
	secure_device_ptr device = (secure_device_ptr)device_id;
	pr_err("core(%d)\n", device->core_id);
	return MM_ISR_PROCESSED;
}

int secure_device_init(MM_CORE_HW_IFC *core_param, int core_id)
{
	secure_device_ptr device;
	int              ret = 0;

	pr_info("core(%d)\n", core_id);
	secure_device[core_id] = kmalloc(sizeof(secure_device_t), GFP_KERNEL);
	if (secure_device[core_id] == NULL) {
		pr_err("kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}
	device = secure_device[core_id];

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr   = 0;
	core_param->mm_hw_size     = 0;
	core_param->mm_irq         = 0;

	core_param->mm_timer       = DEFAULT_MM_DEV_TIMER_MS * 100;
	core_param->mm_timeout     = DEFAULT_MM_DEV_TIMEOUT_MS * 100;

	core_param->mm_update_virt_addr = secure_update_virt;
	core_param->mm_init        = secure_init;
	core_param->mm_deinit      = secure_deinit;
	core_param->mm_abort       = secure_abort;
	core_param->mm_get_regs    = secure_get_regs;
	core_param->mm_get_status  = secure_get_status;
	core_param->mm_process_irq = secure_process_irq;
	core_param->mm_start_job   = secure_start_job;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id   = (void *)device;
	core_param->mm_virt_addr   = NULL;
	core_param->mm_secure_job_wait = secure_job_wait;
	core_param->mm_secure_job_done = secure_job_done;

	device->core_id            = core_id;
	init_completion(&device->job_available);
	device->curr_job           = NULL;
	device->job_status         = MM_JOB_STATUS_INVALID;
	device->state              = MM_HW_STATE_FREE_E;

	return ret;

err:
	pr_err("Error\n");
	return ret;
}


void secure_device_deinit(int core_id)
{
	secure_device_ptr device = secure_device[core_id];

	pr_info("core(%d)\n", core_id);
	kfree(device);
	secure_device[core_id] = NULL;
}


/* MCIN secure device */
int mcin_secure_init(MM_CORE_HW_IFC *core_param)
{
	return secure_device_init(core_param, CORE_H264_MCIN_E);
}
void mcin_secure_deinit(void)
{
	secure_device_deinit(CORE_H264_MCIN_E);
}


/* Cabac secure device */
int cabac_secure_init(MM_CORE_HW_IFC *core_param)
{
	return secure_device_init(core_param, CORE_H264_CABAC_E);
}
void cabac_secure_deinit(void)
{
	secure_device_deinit(CORE_H264_CABAC_E);
}


/* VCE secure device */
int h264_vce_secure_init(MM_CORE_HW_IFC *core_param)
{
	return secure_device_init(core_param, CORE_H264_VCE_E);
}
void h264_vce_secure_deinit(void)
{
	secure_device_deinit(CORE_H264_VCE_E);
}


/* OL secure device */
int h264_ol_secure_init(MM_CORE_HW_IFC *core_param)
{
	return secure_device_init(core_param, CORE_H264_OL_E);
}
void h264_ol_secure_deinit(void)
{
	secure_device_deinit(CORE_H264_OL_E);
}
