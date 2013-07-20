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

#define pr_fmt(fmt) "mcin: " fmt

#include <linux/kernel.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "mcin.h"

#define MM_MCIN_BASE_ADDR (H264_BASE_ADDR+H264_MCODEIN_CONTROL_OFFSET)
#define MCIN_HW_SIZE (4*1024)

struct mcin_device_t {
	void *vaddr;
};

static void mcin_write(struct mcin_device_t *mcin, u32 reg, u32 value)
{
	mm_write_reg(mcin->vaddr, (reg-H264_MCODEIN_CONTROL_OFFSET),
					value);
}

static u32 mcin_read(struct mcin_device_t *mcin, u32 reg)
{
	return mm_read_reg(mcin->vaddr, (reg-H264_MCODEIN_CONTROL_OFFSET));
}

/*static void print_job_struct(struct mcin_job_t *job)
{
	int i = 0;
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("cfg:start_byte: 0x%x\n", (u32)job->mcin_config.start_byte);
	pr_debug("cfg:start_mask: 0x%x\n", (u32)job->mcin_config.start_mask);
	pr_debug("cfg:guard_byte: 0x%x\n", (u32)job->mcin_config.guard_byte);
	pr_debug("cfg:ep_byte: 0x%x\n", (u32)job->mcin_config.ep_byte);
	pr_debug("cfg:flags: 0x%x\n", (u32)job->mcin_config.flags);
	pr_debug("user_data: 0x%x\n", (u32)job->user_data);
	pr_debug("in_addr: 0x%x\n", (u32)job->in_addr);
	pr_debug("in_length: 0x%x\n", (u32)job->in_length);
	pr_debug("out_base: 0x%x\n", (u32)job->out_base);
	pr_debug("out_size: 0x%x\n", (u32)job->out_size);
	pr_debug("out_mark_offset: 0x%x\n", (u32)job->out_mark_offset);
	for (i = 0; i < 7; i++)
		pr_debug("state [%d]: 0x%x\n", i, (u32)job->state[i]);
	for (i = 0; i < 7; i++)
		pr_debug("out_p:state [%d]: 0x%x\n", i,
				(u32)job->out_params.state[i]);
	pr_debug("out_p:remaining_len: 0x%x\n",
			(u32)job->out_params.remaining_len);
	pr_debug("out_p:user_data: 0x%x\n", (u32)job->out_params.user_data);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}*/

static void print_regs(struct mcin_device_t *mcin)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug(" H264_MCODEIN_CONTROL_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_CONTROL_OFFSET));
	pr_debug(" H264_MCODEIN_CONTROL_EXTRA_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_CONTROL_EXTRA_OFFSET));
	pr_debug(" H264_MCODEIN_STATUS_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATUS_OFFSET));
	pr_debug(" H264_MCODEIN_USERDATA_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_USERDATA_OFFSET));
	pr_debug(" H264_MCODEIN_OUTBASE_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_OUTBASE_OFFSET));
	pr_debug(" H264_MCODEIN_OUTSIZE_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_OUTSIZE_OFFSET));
	pr_debug(" H264_MCODEIN_STATE0_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE0_OFFSET));
	pr_debug(" H264_MCODEIN_STATE1_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE1_OFFSET));
	pr_debug(" H264_MCODEIN_STATE2_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE2_OFFSET));
	pr_debug(" H264_MCODEIN_STATE3_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE3_OFFSET));
	pr_debug(" H264_MCODEIN_STATE4_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE4_OFFSET));
	pr_debug(" H264_MCODEIN_STATE5_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE5_OFFSET));
	pr_debug(" H264_MCODEIN_STATE6_OFFSET: 0x%x\n",
			mcin_read(mcin, H264_MCODEIN_STATE6_OFFSET));

	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void mcin_reg_init(void *device_id)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;
	pr_debug("mcin_reg_init:\n");

	/* Initialise MCIN internal state */
	mcin_write(id, H264_MCODEIN_STATE0_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE1_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE2_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE3_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE4_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE5_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATE6_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_USERDATA_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_OUTBASE_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_OUTSIZE_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_OUTMARKOFFSET_OFFSET, 0x0);
}

static int mcin_complete_sequence(struct mcin_device_t *id)
{
	u32 control;
	pr_debug("mcin_complete_sequence:\n");

	while (mcin_read(id, H264_MCODEIN_STATUS_OFFSET) & 0xef)
		;
	control = mcin_read(id, H264_MCODEIN_CONTROL_OFFSET);
	/*Set COMPLETE bit*/
	control |= 1 << 4;
	/*Clearing INTENABLE bit*/
	control &= 0xFFFFFFFE;

	mcin_write(id, H264_MCODEIN_CONTROL_OFFSET, control);

	while (mcin_read(id, H264_MCODEIN_STATUS_OFFSET) & 0xef)
		;
	/*Clear COMPLETE bit*/
	control &= 0xFFFFFEFF;

	mcin_write(id, H264_MCODEIN_CONTROL_OFFSET, control);

	return 0;
}

static int mcin_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;
	pr_debug("mcin_get_regs:\n");
	print_regs(id);
	return 0;
}

static int mcin_reset(void *device_id)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;
	pr_debug("mcin_reset:\n");

	/*Reset the registers*/
	mcin_write(id, H264_MCODEIN_CONTROL_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_CONTROL_EXTRA_OFFSET, 0x0);
	mcin_write(id, H264_MCODEIN_STATUS_OFFSET, 1<<3);

	mcin_reg_init(id);

	while (mcin_read(id, H264_MCODEIN_STATUS_OFFSET) & 0xef)
		;

	return 0;
}

static int mcin_abort(void *device_id, mm_job_post_t *job)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;

	pr_info("mcin_abort:\n");
	mcin_reset(id);

	return 0;
}

static mm_isr_type_e process_mcin_irq(void *device_id)
{
	u32 flags;
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;

	/* Read the interrupt status registers */
	flags = mcin_read(id, H264_MCODEIN_STATUS_OFFSET);

	/* Clear interrupts isr is going to handle */
	mcin_write(id, H264_MCODEIN_STATUS_OFFSET, 1<<3);

	if (flags & (1<<3))
		irq_retval = MM_ISR_SUCCESS;

	return irq_retval;
}

bool get_mcin_status(void *device_id)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;
	pr_debug("get_mcin_status:\n");

	/*Read the status to find Hardware status*/
	if (mcin_read(id, H264_MCODEIN_STATUS_OFFSET) & 0xef)
		return true;

	return false;
}

mm_job_status_e mcin_start_job(void *device_id,
				mm_job_post_t *job, u32 profmask)
{
	struct mcin_device_t *id = (struct mcin_device_t *)device_id;
	struct mcin_job_t *jp = (struct mcin_job_t *)job->data;

	u32 control = 0;
	u32 control_extra = 0;

	if (jp == NULL) {
		pr_err("mcin_start_job: id or jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}
	if (job->size != sizeof(struct mcin_job_t)) {
		pr_err("mcin_start_job: job struct size mismatch\n");
		return MM_JOB_STATUS_ERROR;
	}
	if (job->type != H264_MCIN_EPR_JOB) {
		pr_err("mcin_start_job: Invalid job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		/*Reset MCIN*/
		mcin_reset(id);
		/*Bound checks*/
		if (jp->mcin_config.start_mask == 0) {
			pr_err("mcin_start_job: mcin_mask cant be zero\n");
			return MM_JOB_STATUS_ERROR;
		}
		/*Program MCIN*/

		/* Initialise MCI internal state */
		mcin_write(id, H264_MCODEIN_STATE0_OFFSET, jp->state[0]);
		mcin_write(id, H264_MCODEIN_STATE1_OFFSET, jp->state[1]);
		mcin_write(id, H264_MCODEIN_STATE2_OFFSET, jp->state[2]);
		mcin_write(id, H264_MCODEIN_STATE3_OFFSET, jp->state[3]);
		mcin_write(id, H264_MCODEIN_STATE4_OFFSET, jp->state[4]);
		mcin_write(id, H264_MCODEIN_STATE5_OFFSET, jp->state[5]);
		mcin_write(id, H264_MCODEIN_STATE6_OFFSET, jp->state[6]);

		mcin_write(id, H264_MCODEIN_USERDATA_OFFSET, jp->user_data);

		mcin_write(id, H264_MCODEIN_OUTBASE_OFFSET, jp->out_base);
		mcin_write(id, H264_MCODEIN_OUTSIZE_OFFSET, jp->out_size);
		mcin_write(id, H264_MCODEIN_OUTMARKOFFSET_OFFSET,
				jp->out_mark_offset);

		/*EMUL*/
		control = jp->mcin_config.ep_byte << 24;
		/*MASK*/
		control |= jp->mcin_config.start_mask << 16;
		/*CODE*/
		control |= jp->mcin_config.start_byte << 8;

		if (jp->mcin_config.flags & MCIN_INPUT_CONFIG_AVS)
			/*AVSMODE*/
			control |= 1 << 7;

		if (jp->mcin_config.start_mask == 0xff) {
			/* Startcode occupies full byte;
			* next byte cannot form startcode */
			control |= 1 << 3; /*STARTBYTE*/
		} else {
			/* Startcode occupies partial byte;
			* preserve it to retain remainder */
			control |= 1 << 5; /*SENDSTART*/
		}

		/*INTENABLE*/
		control |= 1;
		/*WRITEBACK*/
		control |= 1 << 6;
		/*STRIP*/
		control |= 1 << 1;

		control_extra = jp->mcin_config.guard_byte << 1;

		if (jp->mcin_config.use_ep == 0) {
			/*EMUL_DISABLE*/
			control_extra |= 1 << 12;
			if (jp->mcin_config.no_startcodes) {
				/*MEMCPY*/
				control_extra |= 1;
			}
		} else if (jp->mcin_config.no_startcodes) {
			/*MASK*/
			control |= 0 << 16;
		}

		mcin_write(id, H264_MCODEIN_CONTROL_OFFSET, control);
		mcin_write(id, H264_MCODEIN_CONTROL_EXTRA_OFFSET,
					control_extra);

		mcin_write(id, H264_MCODEIN_INADDR_OFFSET, jp->in_addr);
		mcin_write(id, H264_MCODEIN_LENGTH_OFFSET, jp->in_length);

		job->status = MM_JOB_STATUS_RUNNING;
		return MM_JOB_STATUS_RUNNING;

	case MM_JOB_STATUS_RUNNING:
		/*Check for complete sequence*/
		if (jp->mcin_config.nal_end == 1)
			mcin_complete_sequence(id);
		/*read back the state variables for client*/
		jp->out_params.state[0] = mcin_read(id,
						H264_MCODEIN_STATE0_OFFSET);
		jp->out_params.state[1] = mcin_read(id,
						H264_MCODEIN_STATE1_OFFSET);
		jp->out_params.state[2] = mcin_read(id,
						H264_MCODEIN_STATE2_OFFSET);
		jp->out_params.state[3] = mcin_read(id,
						H264_MCODEIN_STATE3_OFFSET);
		jp->out_params.state[4] = mcin_read(id,
						H264_MCODEIN_STATE4_OFFSET);
		jp->out_params.state[5] = mcin_read(id,
						H264_MCODEIN_STATE5_OFFSET);
		jp->out_params.state[6] = mcin_read(id,
						H264_MCODEIN_STATE6_OFFSET);
		jp->out_params.remaining_len = mcin_read(id,
						H264_MCODEIN_LENGTH_OFFSET);
		jp->out_params.user_data = mcin_read(id,
						H264_MCODEIN_USERDATA_OFFSET);
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;

	case MM_JOB_STATUS_SUCCESS:
	case MM_JOB_STATUS_ERROR:
	case MM_JOB_STATUS_NOT_FOUND:
	case MM_JOB_STATUS_TIMED_OUT:
	case MM_JOB_STATUS_SKIP:
	case MM_JOB_STATUS_LAST:
	default:
		pr_err("mcin_start_job: Unexpected Job status\n");
	}
	return MM_JOB_STATUS_ERROR;
}
struct mcin_device_t *mcin_device;

void mcin_update_virt(void *virt)
{
	pr_debug("mcin_update_virt:\n");
	mcin_device->vaddr = virt;
}

int mcin_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	mcin_device = kmalloc(sizeof(struct mcin_device_t), GFP_KERNEL);
	if (mcin_device == NULL) {
		pr_err("mcin_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	mcin_device->vaddr = NULL;
	pr_debug("mcin_init: -->\n");

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr = MM_MCIN_BASE_ADDR;
	core_param->mm_hw_size = MCIN_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_MCIN_CBC;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_mcin_status;
	core_param->mm_start_job = mcin_start_job;
	core_param->mm_process_irq = process_mcin_irq;
	core_param->mm_init = mcin_reset;
	core_param->mm_deinit = mcin_reset;
	core_param->mm_abort = mcin_abort;
	core_param->mm_get_regs = mcin_get_regs;
	core_param->mm_update_virt_addr = mcin_update_virt;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id = (void *)mcin_device;
	core_param->mm_virt_addr = NULL;

	return ret;
err:
	pr_err("mcin_init: Error");
	return ret;
}

void mcin_deinit(void)
{
	pr_debug("mcin_deinit:\n");
	kfree(mcin_device);
}
