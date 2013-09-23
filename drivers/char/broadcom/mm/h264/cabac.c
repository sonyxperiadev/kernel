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

#define pr_fmt(fmt) "cabac: " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "cabac.h"

#define MM_CABAC_BASE_ADDR (H264_BASE_ADDR)
#define CABAC_HW_SIZE (6148*1024)

struct cabac_device_t {
	void *vaddr;
};

static void cabac_write(struct cabac_device_t *cabac, u32 reg, u32 value)
{
	mm_write_reg(cabac->vaddr, reg, value);
}

static u32 cabac_read(struct cabac_device_t *cabac, u32 reg)
{
	return mm_read_reg(cabac->vaddr, reg);
}

/*static void print_job_struct(struct cabac_job_t *job)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("cfg:highest_ctxt_used: 0x%x\n", (u32)job->highest_ctxt_used);
	pr_debug("cfg:rd_ctxt_addr: 0x%x\n", (u32)job->rd_ctxt_addr);
	pr_debug("cfg:wt_ctxt_addr: 0x%x\n", (u32)job->wt_ctxt_addr);
	pr_debug("cfg:cmd_buf_addr: 0x%x\n", (u32)job->cmd_buf_addr);
	pr_debug("cfg:log2_cmd_buf_size: 0x%x\n", (u32)job->log2_cmd_buf_size);
	pr_debug("cfg:upstride_base_addr: 0x%x\n",
		(u32)job->upstride_base_addr);
	pr_debug("cfg:num_cmds: 0x%x\n", (u32)job->num_cmds);
	pr_debug("cfg:end_conf: 0x%x\n", (u32)job->end_conf);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}*/

static void print_regs(struct cabac_device_t *cabac)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("H264_REGCS_REGCABAC2BINSIMGCTXLAST_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGCS_REGCABAC2BINSIMGCTXLAST_OFFSET));
	pr_debug("H264_REGCS_REGCABAC2BINSRDCONTEXTBASEADDR_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGCS_REGCABAC2BINSRDCONTEXTBASEADDR_OFFSET));
	pr_debug("H264_REGCS_REGCABAC2BINSWRCONTEXTBASEADDR_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGCS_REGCABAC2BINSWRCONTEXTBASEADDR_OFFSET));
	pr_debug("H264_REGC2_REGCABAC2BINSCOMMANDBUFFERADDR_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERADDR_OFFSET));
	pr_debug("H264_REGC2_REGCABAC2BINSCOMMANDBUFFERCOUNT_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERCOUNT_OFFSET));
	pr_debug("H264_REGC2_REGCABAC2BINSCOMMANDBUFFERLOGSIZE_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERLOGSIZE_OFFSET));
	pr_debug("H264_REGC2_REGCABAC2BINSUPSTRIPEBASEADDR_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGC2_REGCABAC2BINSUPSTRIPEBASEADDR_OFFSET));
	pr_debug("H264_REGC2_REGCABAC2BINSCTL_OFFSET: 0x%x\n",
		cabac_read(cabac,
			H264_REGC2_REGCABAC2BINSCTL_OFFSET));
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static int cabac_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;
	print_regs(id);
	return 0;
}

static int cabac_abort(void *device_id, mm_job_post_t *job)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;
	pr_info("cabac_abort:\n");

	/*Clear all pending commands*/
	cabac_write(id, H264_REGC2_REGCABAC2BINSCOMMANDBUFFERCOUNT_OFFSET, 0x2);
	/*TODO: Any more procedure*/
	cabac_write(id, H264_REGC2_REGCABAC2BINSCTL_OFFSET, 0x1);

	udelay(3);

	return 0;
}

static int cabac_reset(void *device_id)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;

	/*Reset the registers*/
	cabac_write(id, H264_REGC2_REGCABAC2BINSCTL_OFFSET, 0x1);

	udelay(3);

	return 0;
}

static int cabac_block_init(void *device_id)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;
	u32 temp;
	/*Enable CABAC Block*/
	temp = cabac_read(id, H264_VCODEC_GCKENAA_OFFSET);
	temp |= H264_VCODEC_GCKENAA_CABAC_MASK;
	cabac_write(id, H264_VCODEC_GCKENAA_OFFSET, temp);

	/*Reset the registers*/
	cabac_reset(id);

	return 0;
}

static int cabac_block_deinit(void *device_id)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;
	u32 temp;
	/*Disable CABAC Block*/
	temp = cabac_read(id, H264_VCODEC_GCKENAA_OFFSET);
	temp &= (~H264_VCODEC_GCKENAA_CABAC_MASK);
	cabac_write(id, H264_VCODEC_GCKENAA_OFFSET, temp);
	return 0;
}

static mm_isr_type_e process_cabac_irq(void *device_id)
{
	u32 flags;
	u32 pending_cmds;

	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;

	/* Read the interrupt status registers */
	flags = cabac_read(id, H264_REGC2_REGCABAC2BINSCTL_OFFSET);

	/*Reading Pending commands*/
	pending_cmds = cabac_read(id,
		H264_REGC2_REGCABAC2BINSCOMMANDBUFFERCOUNT_OFFSET) & 0x7FF;

	/* Clear interrupts isr is going to handle */
	flags |= (1<<11);
	cabac_write(id, H264_REGC2_REGCABAC2BINSCTL_OFFSET, flags);

	if (pending_cmds != 0)
		irq_retval = MM_ISR_PROCESSED;
	else
		irq_retval = MM_ISR_SUCCESS;

	return irq_retval;
}

bool get_cabac_status(void *device_id)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;

	/*Read the status to find Hardware busy status*/
	if (cabac_read(id, H264_REGC2_REGCABAC2BINSCTL_OFFSET) & 0x400)
		return true;
	return false;
}

mm_job_status_e cabac_start_job(void *device_id , mm_job_post_t *job,
				 u32 profmask)
{
	struct cabac_device_t *id = (struct cabac_device_t *)device_id;
	struct cabac_job_t *jp = (struct cabac_job_t *)job->data;
	int i = 0;

	if (jp == NULL) {
		pr_err("cabac_start_job: id or jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}

	if (job->size != sizeof(struct cabac_job_t)) {
		pr_err("cabac_start_job: job struct size mismatch\n");
		return MM_JOB_STATUS_ERROR;
	}

	if (job->type != H264_CABAC_DEC_JOB &&
		job->type != H264_CABAC_ENC_JOB) {
		pr_err("cabac_start_job: Invalid job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		/*Bound checks*/
		if (jp->rd_ctxt_addr & 0xF) {
			pr_err("cabac_start_job: " \
				"read context address is not 16B alligned\n");
			return MM_JOB_STATUS_ERROR;
		}

		if (jp->wt_ctxt_addr & 0x7FF) {
			pr_err("cabac_start_job: " \
				"read context address is not 2KB alligned\n");
			return MM_JOB_STATUS_ERROR;
		}

		if (jp->num_cmds > 0x7FF) {
			pr_err("cabac_start_job: Too many commands\n");
			return MM_JOB_STATUS_ERROR;
		}

		/*Program CABAC*/

		switch (jp->end_conf) {
		case CABAC_CONF_BE:
			cabac_write(id,
				H264_REGC2_REGCABAC2BINSCTL_OFFSET,
				0x800);
			break;
		case CABAC_CONF_LE:
			cabac_write(id,
				H264_REGC2_REGCABAC2BINSCTL_OFFSET,
				0x10800);
			break;
		case CABAC_CONF_BE_SW_OVERRIDE:
			cabac_write(id,
				H264_REGC2_REGCABAC2BINSCTL_OFFSET,
				0x20800);
			break;
		case CABAC_CONF_LE_SW_OVERRIDE:
			cabac_write(id,
				H264_REGC2_REGCABAC2BINSCTL_OFFSET,
				0x30800);
			break;
		default:
			pr_err("cabac_start_job:Invalid Endianess selection\n");
			return MM_JOB_STATUS_ERROR;
			break;
		}

		if (jp->highest_ctxt_used)
			cabac_write(id,
			H264_REGCS_REGCABAC2BINSIMGCTXLAST_OFFSET,
			jp->highest_ctxt_used);
		cabac_write(id,
			H264_REGCS_REGCABAC2BINSRDCONTEXTBASEADDR_OFFSET,
			jp->rd_ctxt_addr);
		cabac_write(id,
			H264_REGCS_REGCABAC2BINSWRCONTEXTBASEADDR_OFFSET,
			jp->wt_ctxt_addr);
		cabac_write(id,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERADDR_OFFSET,
			jp->cmd_buf_addr);
		cabac_write(id,
			H264_REGC2_REGCABAC2BINSUPSTRIPEBASEADDR_OFFSET,
			jp->upstride_base_addr);
		cabac_write(id,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERLOGSIZE_OFFSET,
			jp->log2_cmd_buf_size);

		for (i = 0; i < jp->num_cmds; i++) {
			cabac_write(id,
			H264_REGC2_REGCABAC2BINSCOMMANDBUFFERCOUNT_OFFSET, 0x1);
		}

		job->status = MM_JOB_STATUS_RUNNING;
		return MM_JOB_STATUS_RUNNING;

	case MM_JOB_STATUS_RUNNING:
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;

	case MM_JOB_STATUS_SUCCESS:
	case MM_JOB_STATUS_ERROR:
	case MM_JOB_STATUS_NOT_FOUND:
	case MM_JOB_STATUS_TIMED_OUT:
	case MM_JOB_STATUS_SKIP:
	case MM_JOB_STATUS_LAST:
	default:
		pr_err("cabac_start_job: Unexpected Job status\n");
	}
	return MM_JOB_STATUS_ERROR;
}
struct cabac_device_t *cabac_device;

void cabac_update_virt(void *virt)
{
	cabac_device->vaddr = virt;
}

int cabac_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	cabac_device = kmalloc(sizeof(struct cabac_device_t), GFP_KERNEL);
	if (cabac_device == NULL) {
		pr_err("cabac_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	cabac_device->vaddr = NULL;

	/*Do any device specific structure initialisation required.*/

	core_param->mm_base_addr = MM_CABAC_BASE_ADDR;
	core_param->mm_hw_size = CABAC_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_MCIN_CBC;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_cabac_status;
	core_param->mm_start_job = cabac_start_job;
	core_param->mm_process_irq = process_cabac_irq;
	core_param->mm_init = cabac_block_init;
	core_param->mm_deinit = cabac_block_deinit;
	core_param->mm_abort = cabac_abort;
	core_param->mm_get_regs = cabac_get_regs;
	core_param->mm_update_virt_addr = cabac_update_virt;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id = (void *)cabac_device;
	core_param->mm_virt_addr = NULL;

	return ret;

err:
	pr_err("cabac_init:Error");
	return ret;
}

void cabac_deinit(void)
{
	kfree(cabac_device);
}
