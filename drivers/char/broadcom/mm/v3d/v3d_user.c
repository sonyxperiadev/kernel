
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

#define pr_fmt(fmt) "v3d_user: " fmt

#include <linux/kernel.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

#include "v3d_user.h"

#define V3D_HW_SIZE (1024*4)
#define IRQ_V3D	BCM_INT_ID_RESERVED148
#define v3d_user_device_t struct _v3d_user_device_t

struct _v3d_user_device_t {
	void __iomem *vaddr;
};

static inline void v3d_write(v3d_user_device_t *v3d,
		unsigned int reg, unsigned int value)
{
	mm_write_reg((void *)v3d->vaddr, reg, value);
}

static inline unsigned int v3d_read(v3d_user_device_t *v3d, unsigned int reg)
{
	return mm_read_reg((void *)v3d->vaddr, reg);
}

static int v3d_user_reset(void *device_id)
{
	v3d_user_device_t *id = (v3d_user_device_t *)device_id;
	pr_debug("v3d_user_reset:\n");
	v3d_write(id, V3D_SRQCS_OFFSET, 1 | (1 << 8) | (1 << 16));
	v3d_write(id, V3D_VPACNTL_OFFSET, 0);
	v3d_write(id, V3D_SLCACTL_OFFSET, 0xf0f0f0f);
	v3d_write(id, V3D_DBQITC_OFFSET, 0xffff);
	v3d_write(id, V3D_DBQITE_OFFSET, 0xffff);

	return 0;
}

static int v3d_user_abort(void *device_id)
{
	v3d_user_device_t *id = (v3d_user_device_t *)device_id;
	pr_debug("v3d_user_abort:\n");
	v3d_user_reset(id);
	return 0;
}

static int v3d_u_abort(void *id, mm_job_post_t *job)
{
	int ret;
	ret = v3d_user_abort(id);
	return ret;
}

static mm_isr_type_e process_v3d_user_irq(void *device_id)
{
	u32 flags_qpu, numCompletedJobs;
	v3d_user_device_t *id = (v3d_user_device_t *)device_id;
	flags_qpu = v3d_read(id, V3D_DBQITC_OFFSET);
	if (flags_qpu) {
		v3d_write(id, V3D_DBQITC_OFFSET, flags_qpu);
		numCompletedJobs = (v3d_read(id, V3D_SRQCS_OFFSET)
					>> 16) & 0xff;
		if (numCompletedJobs == v3d_read(id, V3D_SCRATCH_OFFSET)) {
			if ((v3d_read(id, V3D_PCS_OFFSET)&0xF) == 0)
				v3d_write(id, V3D_VPMBASE_OFFSET, 0);
			return MM_ISR_SUCCESS;
			}
		return MM_ISR_PROCESSED;
	}
	if (v3d_read(id, V3D_VPMBASE_OFFSET) != 16) {
		if ((v3d_read(id, V3D_PCS_OFFSET)&0xF) == 0) {
			v3d_write(id, V3D_VPMBASE_OFFSET, 16);
			return MM_ISR_SUCCESS;
			}
		}
	return MM_ISR_UNKNOWN;
}

mm_job_status_e v3d_user_start_job(void *device_id,
		mm_job_post_t *job, u32 profmask)
{
	v3d_user_device_t *id = (v3d_user_device_t *)device_id;
	v3d_user_job_t *job_params  = (v3d_user_job_t *)job->data;
	int i;

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		{
			if (v3d_read(id, V3D_VPMBASE_OFFSET) != 16) {
				if ((v3d_read(id, V3D_PCS_OFFSET)&0xF) == 0)
					v3d_write(id, V3D_VPMBASE_OFFSET, 16);
				else
					return MM_JOB_STATUS_RUNNING;
				}
			v3d_user_reset(id);
			if (job->type == V3D_USER_JOB) {
				if ((v3d_read(id, V3D_SRQCS_OFFSET) & 0x3F) ==
					MAX_USER_JOBS) {
					job->status = MM_JOB_STATUS_ERROR;
					pr_err("User job queue is full %08x",
					v3d_read(id, V3D_SRQCS_OFFSET));
					return MM_JOB_STATUS_ERROR;
				}
				if (job_params->numUserJobs <= 0 ||
				job_params->numUserJobs > MAX_USER_JOBS) {
					pr_err("Invalied number of user jobs passed, %d",
						job_params->numUserJobs);
					return MM_JOB_STATUS_ERROR;
				}
				v3d_write(id, V3D_SCRATCH_OFFSET,
					job_params->numUserJobs);
				job->status = MM_JOB_STATUS_RUNNING;
				for (i = 0; i < job_params->numUserJobs; i++) {
					pr_debug("Submitting user job %x : %x (%x)\n",
						job_params->v3d_srqpc[i],
						job_params->v3d_srqua[i],
						job_params->v3d_srqul[i]);
					v3d_write(id, V3D_SRQUL_OFFSET,
						job_params->v3d_srqul[i]);
					v3d_write(id, V3D_SRQUA_OFFSET,
						job_params->v3d_srqua[i]);
					v3d_write(id, V3D_SRQPC_OFFSET,
						job_params->v3d_srqpc[i]);
				}
				return MM_JOB_STATUS_RUNNING;
			}
		}
		break;
	case MM_JOB_STATUS_RUNNING:
		{
			job->status = MM_JOB_STATUS_SUCCESS;
			return MM_JOB_STATUS_SUCCESS;
		}
		break;
	default:
		break;
	}

	pr_err("v3d_user_start_job :: job type = %x job status = %x\n",
		job->type, job->status);
	return MM_JOB_STATUS_ERROR;
}

static v3d_user_device_t *v3d_user_device;

void v3d_user_update_virt(void *virt)
{
	pr_debug("v3d_user_update_virt:\n");
	v3d_user_device->vaddr = virt;
}

bool get_v3d_user_status(void *device_id)
{
	v3d_user_device_t *id = (v3d_user_device_t *)device_id;

	/*Read the status to find Hardware busy status*/
	uint32_t status = v3d_read(id, V3D_SRQCS_OFFSET);
	if (((status >> 8) & 0xff) != ((status >> 16) & 0xff))
		return true;

	return false;
}

void v3d_user_deinit(void)
{
	kfree(v3d_user_device);
}

int v3d_user_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;
	v3d_user_device = kmalloc(sizeof(v3d_user_device_t), GFP_KERNEL);

	v3d_user_device->vaddr = NULL;
	pr_debug("v3d_user_init: -->\n");

	core_param->mm_base_addr = MM_V3D_BASE_ADDR;
	core_param->mm_hw_size = V3D_HW_SIZE;
	core_param->mm_irq = IRQ_V3D;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_v3d_user_status;
	core_param->mm_start_job = v3d_user_start_job;
	core_param->mm_process_irq = process_v3d_user_irq;
	core_param->mm_init = v3d_user_reset;
	core_param->mm_deinit = v3d_user_reset;
	core_param->mm_abort = v3d_u_abort;
	core_param->mm_version_init = NULL;
	core_param->mm_update_virt_addr = v3d_user_update_virt;
	core_param->mm_get_regs = NULL;
	core_param->mm_device_id = (void *)v3d_user_device;
	core_param->mm_virt_addr = NULL;
	return ret;
}
