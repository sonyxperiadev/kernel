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

#define pr_fmt(fmt) "cme: " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "cme.h"

#define MM_CME_BASE_ADDR (H264_BASE_ADDR + H264_CME_CURY_OFFSET)
#define CME_HW_SIZE 0x1000

struct cme_device_t {
	u32 cme_version;
	void *vaddr;
	void *clockaddr;
};

static inline void cme_write(struct cme_device_t *cme, u32 reg, u32 value)
{
	mm_write_reg(cme->vaddr, (reg-H264_CME_CURY_OFFSET), value);
}

static inline u32 cme_read(struct cme_device_t *cme, u32 reg)
{
	return mm_read_reg(cme->vaddr, (reg-H264_CME_CURY_OFFSET));
}

/*static void print_job_struct(struct cme_job_t *job)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("img_type: 0x%x\n", (u32)job->img_type);
	pr_debug("cur_y_addr: 0x%x\n", (u32)job->cur_y_addr);
	pr_debug("cur_c_addr: 0x%x\n", (u32)job->cur_c_addr);
	pr_debug("ref_y_addr: 0x%x\n", (u32)job->ref_y_addr);
	pr_debug("ref_c_addr: 0x%x\n", (u32)job->ref_c_addr);
	pr_debug("img_pitch: 0x%x\n", (u32)job->img_pitch);
	pr_debug("cme_bias: 0x%x\n", (u32)job->cme_bias);
	pr_debug("vetctor_dump_addr: 0x%x\n", (u32)job->vetctor_dump_addr);
	pr_debug("dump_vstride_bytes: 0x%x\n", (u32)job->dump_vstride_bytes);
	pr_debug("dump_hstride_bytes: 0x%x\n", (u32)job->dump_hstride_bytes);
	pr_debug("height_mb: 0x%x\n", (u32)job->height_mb);
	pr_debug("width_mb: 0x%x\n", (u32)job->width_mb);
	pr_debug("vradius_mb: 0x%x\n", (u32)job->vradius_mb);
	pr_debug("hradius_mb: 0x%x\n", (u32)job->hradius_mb);
	pr_debug("cme_autolimit: 0x%x\n", (u32)job->cme_autolimit);
	pr_debug("auto_hrext: 0x%x\n", (uint8_t)job->auto_hrext);
	pr_debug("auto_ignorec: 0x%x\n", (uint8_t)job->auto_ignorec);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}*/

static void print_regs(struct cme_device_t *cme)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug(" H264_CME_CURY_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_CURY_OFFSET));
	pr_debug(" H264_CME_CURC_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_CURC_OFFSET));
	pr_debug(" H264_CME_REFY_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_REFY_OFFSET));
	pr_debug(" H264_CME_REFC_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_REFC_OFFSET));
	pr_debug(" H264_CME_LOADCTRL_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_LOADCTRL_OFFSET));
	pr_debug(" H264_CME_SEARCHCTRL_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_SEARCHCTRL_OFFSET));
	pr_debug(" H264_CME_PITCH_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_PITCH_OFFSET));
	pr_debug(" H264_CME_TOTSAD_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_TOTSAD_OFFSET));
	pr_debug(" H264_CME_INTCS_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_INTCS_OFFSET));
	pr_debug(" H264_CME_BIAS_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_BIAS_OFFSET));
	pr_debug(" H264_CME_DUMPADDR_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_DUMPADDR_OFFSET));
	pr_debug(" H264_CME_DUMPSTRIDE_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_DUMPSTRIDE_OFFSET));
	pr_debug(" H264_CME_AUTOSTRIPE_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_AUTOSTRIPE_OFFSET));
	pr_debug(" H264_CME_AUTOCTRL_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_AUTOCTRL_OFFSET));
	pr_debug(" H264_CME_AUTOSTATUS_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_AUTOSTATUS_OFFSET));
	pr_debug(" H264_CME_AUTOLIMIT_OFFSET: 0x%x\n",
		cme_read(cme, H264_CME_AUTOLIMIT_OFFSET));
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void cme_reg_init(void *device_id)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;

	cme_write(id, H264_CME_LOADCTRL_OFFSET, 0);
	cme_write(id, H264_CME_SEARCHCTRL_OFFSET, 0);
	cme_write(id, H264_CME_TOTSAD_OFFSET, 0);
	cme_write(id, H264_CME_INTCS_OFFSET, 0);
}

static int cme_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;
	print_regs(id);
	return 0;
}

static int cme_reset(void *device_id)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;

	/*Reset the registers*/
	cme_reg_init(id);

	while ((cme_read(id, H264_CME_AUTOSTATUS_OFFSET) >> 31) & 0x1)
		;

	return 0;
}

static int cme_abort(void *device_id, mm_job_post_t *job)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;
	pr_info("cme_abort:\n");

	/*Aborting current operation*/
	cme_write(id, H264_CME_AUTOCTRL_OFFSET, 0x4);

	while ((cme_read(id, H264_CME_AUTOSTATUS_OFFSET) >> 31) & 0x1)
		;

	return 0;
}

static int cme_block_init(void *device_id)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;
	u32 temp;

	/*Enable CME block*/
	temp = readl(id->clockaddr);
	temp |= H264_VCODEC_GCKENAA_CME_MASK;
	writel(temp, id->clockaddr);
	id->cme_version = cme_read(id, H264_CME_VERSION_OFFSET);

	/*Reset the registers*/
	cme_reset(id);

	return 0;
}

static int cme_block_deinit(void *device_id)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;
	u32 temp;
	/*Disable CME Block*/
	temp = readl(id->clockaddr);
	temp &= (~H264_VCODEC_GCKENAA_CME_MASK);
	writel(temp, id->clockaddr);
	return 0;
}

static mm_isr_type_e process_cme_irq(void *device_id)
{
	u32 flags;
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	struct cme_device_t *id = (struct cme_device_t *)device_id;

	/* Read the interrupt status registers */
	flags = cme_read(id, H264_CME_INTCS_OFFSET);

	/* Disable CME Idle Interrupt */
	cme_write(id, H264_CME_INTCS_OFFSET, 0);

	if ((flags & (1<<31)) == 0)
		irq_retval = MM_ISR_SUCCESS;

	return irq_retval;
}

bool get_cme_status(void *device_id)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;

	/*Read the status of AUTO SM and return based on the bit*/
	if ((cme_read(id, H264_CME_AUTOSTATUS_OFFSET) >> 31)&(0x1))
		return true;

	return false;
}

mm_job_status_e cme_start_job(void *device_id,\
			mm_job_post_t *job,\
			unsigned int profmask)
{
	struct cme_device_t *id = (struct cme_device_t *)device_id;
	struct cme_job_t *jp = (struct cme_job_t *)job->data;
	u32 temp;
	u32 var_pitch;

	if (jp == NULL) {
		pr_err("cme_start_job: jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}

	if (job->size != sizeof(struct cme_job_t)) {
		pr_err("cme_start_job: job struct size mismatch\n");
		return MM_JOB_STATUS_ERROR;
	}

	if (job->type != H264_CME_EST_JOB) {
		pr_err("cme_start_job: Invalid job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		/*Bound checks*/
		if (!(jp->hradius_mb >= 1 && jp->hradius_mb <= 6 &&
				jp->vradius_mb >= 1 && jp->vradius_mb <= 4)) {
			pr_err("Invalid Parameter\n");
			return MM_JOB_STATUS_ERROR;
		}

		if (id->cme_version < 0xCE2B && jp->hradius_mb  > 4)
			jp->hradius_mb = 4; /*2708A0 can only search upto +/-4*/

		(jp->hradius_mb)--;
		(jp->vradius_mb)--;

		if (!jp->dump_hstride_bytes)
			jp->dump_hstride_bytes = 4;

		if (!jp->dump_vstride_bytes) {
			jp->dump_vstride_bytes =
				jp->dump_hstride_bytes * jp->width_mb;
		}

			/*Program CME*/
		cme_write(id, H264_CME_CURY_OFFSET, jp->cur_y_addr);
		cme_write(id, H264_CME_CURC_OFFSET, jp->cur_c_addr);
		cme_write(id, H264_CME_REFY_OFFSET, jp->ref_y_addr);
		cme_write(id, H264_CME_REFC_OFFSET, jp->ref_c_addr);

		switch (jp->img_type) {
		case CME_FORMAT_YUV_UV32:
			cme_write(id, H264_CME_PITCH_OFFSET, 32);
			/*NB vc_image->pitch is bytes per slab*/
			cme_write(id, H264_CME_AUTOSTRIPE_OFFSET,
				(jp->img_pitch << 12) +
				(jp->img_pitch >> 3) + 0);
			var_pitch = (127 & (jp->cur_y_addr | jp->cur_c_addr |
				jp->ref_y_addr | jp->ref_c_addr)) ? 1 : 0;
			break;
		case CME_FORMAT_YUV_UV:
			cme_write(id, H264_CME_PITCH_OFFSET, 128);
			cme_write(id, H264_CME_AUTOSTRIPE_OFFSET,
				((jp->img_pitch-128+32) << 12) +
				((jp->img_pitch-128+32) >> 3) + 2);
			var_pitch = 1;
			break;
#if CME_DEFAULT_IGNOREC
		case CME_FORMAT_YUV420:
		case CME_FORMAT_YUV422:
		case CME_FORMAT_YUV422PLANAR:
			/*We can accept planar YUV, but only when
			* ignoring Chroma. Must be 32-byte aligned.*/
			if (31 & (jp->img_pitch | jp->cur_y_addr |
					jp->ref_y_addr)) {
				pr_err("Planar YUV without 32-byte aligned\n");
				return MM_JOB_STATUS_ERROR;
			}
			cme_write(id, H264_CME_PITCH_OFFSET, jp->img_pitch);
			cme_write(id, H264_CME_AUTOSTRIPE_OFFSET,
					((1<<17)|(1<<2)));
			var_pitch = 1;
			break;
#endif
		default:
			pr_err("cme_start_job: Unsupported Format\n");
			return MM_JOB_STATUS_ERROR;
		}

		cme_write(id, H264_CME_DUMPADDR_OFFSET, jp->vetctor_dump_addr);
		cme_write(id, H264_CME_DUMPSTRIDE_OFFSET,
				(jp->dump_vstride_bytes << 16) |
				(jp->dump_hstride_bytes));
		cme_write(id, H264_CME_BIAS_OFFSET, jp->cme_bias);
		cme_write(id, H264_CME_AUTOLIMIT_OFFSET, jp->cme_autolimit);

		temp = (jp->height_mb << 24) | (jp->width_mb << 16) |
			(jp->vradius_mb << 14) | ((jp->hradius_mb & 3) << 12) |
			((jp->hradius_mb & 4) << 3) |
			(jp->auto_ignorec << 4) | (var_pitch << 3) | 1;

		cme_write(id, H264_CME_AUTOCTRL_OFFSET, temp);
		job->status = MM_JOB_STATUS_RUNNING;
		cme_write(id, H264_CME_INTCS_OFFSET, 0x8);
		return MM_JOB_STATUS_RUNNING;

	case MM_JOB_STATUS_RUNNING:
		jp->out_params.totalsad = cme_read(id, H264_CME_TOTSAD_OFFSET);
		jp->out_params.progress = (cme_read(id,
				H264_CME_AUTOSTATUS_OFFSET) >> 2) & 0x1F;
		job->status = MM_JOB_STATUS_SUCCESS;
		return job->status;

	case MM_JOB_STATUS_SUCCESS:
	case MM_JOB_STATUS_ERROR:
	case MM_JOB_STATUS_NOT_FOUND:
	case MM_JOB_STATUS_TIMED_OUT:
	case MM_JOB_STATUS_SKIP:
	case MM_JOB_STATUS_LAST:
	default:
		pr_err("cme_start_job: Unexpected Job status\n");
	}
	return MM_JOB_STATUS_ERROR;
}

struct cme_device_t *cme_device;

void cme_update_virt(void *virt)
{
	cme_device->vaddr = virt;
}

int cme_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	cme_device = kmalloc(sizeof(struct cme_device_t), GFP_KERNEL);
	if (cme_device == NULL) {
		pr_err("cme_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	cme_device->vaddr = NULL;
	cme_device->cme_version = 0;

	cme_device->clockaddr = (void *)ioremap_nocache(\
					H264_BASE_ADDR + \
					H264_VCODEC_GCKENAA_OFFSET, \
					0x1000);
	if (cme_device->clockaddr == NULL) {
		pr_err("cme_init: register mapping failed ");
		goto err;
	}

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr = MM_CME_BASE_ADDR;
	core_param->mm_hw_size = CME_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_CME;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_cme_status;
	core_param->mm_start_job = cme_start_job;
	core_param->mm_process_irq = process_cme_irq;
	core_param->mm_init = cme_block_init;
	core_param->mm_deinit = cme_block_deinit;
	core_param->mm_abort = cme_abort;
	core_param->mm_get_regs = cme_get_regs;
	core_param->mm_update_virt_addr = cme_update_virt;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id = (void *)cme_device;
	core_param->mm_virt_addr = NULL;

	return ret;
err:
	pr_err("cme_init: Error");
	return ret;
}

void cme_deinit(void)
{
	if (cme_device->clockaddr)
		iounmap(cme_device->clockaddr);
	kfree(cme_device);
}
