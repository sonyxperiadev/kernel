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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_isp.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "mm_isp.h"


#define ISP_HW_SIZE     0x4000
#define IRQ_ISP         BCM_INT_ID_RESERVED153

#define isp_write(reg, value) mm_write_reg(isp->vaddr, reg, value)
#define isp_read(reg) mm_read_reg(isp->vaddr, reg)
#define isp_clr_bit32(reg, bits) mm_clr_bit32(isp->vaddr, reg, bits)
#define isp_write_bit32(reg, bits) mm_write_bit32(isp->vaddr, reg, bits)

struct isp_device_t {
	void *vaddr;
	void *fmwk_handle;
};

void printispregs(struct isp_device_t *isp)
{
	pr_info("ISP_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISP_CTRL_OFFSET)));
	pr_info("ISP_STATUS = 0x%lx\n",
		(unsigned long)(isp_read(ISP_STATUS_OFFSET)));
	pr_info("ISP_ID = 0x%lx\n",
		(unsigned long)(isp_read(ISP_ID_OFFSET)));
	pr_info("ISP_TILE_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISP_TILE_CTRL_OFFSET)));
	pr_info("ISP_TILE_STATUS = 0x%lx\n",
		(unsigned long) (isp_read(ISP_TILE_STATUS_OFFSET)));
	pr_info("ISP_TILE_ADDR = 0x%lx\n",
		(unsigned long) (isp_read(ISP_TILE_ADDR_OFFSET)));
	pr_info("ISPFR_BAYER_EN = 0x%lx\n",
		(unsigned long) (isp_read(ISPFR_BAYER_EN_OFFSET)));
	pr_info("ISPFR_YCBCR_EN = 0x%lx\n",
		(unsigned long) (isp_read(ISPFR_YCBCR_EN_OFFSET)));
	pr_info("ISPYI_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(0x00000700)));
	pr_info("ISPYI_RY_ADDR = 0x%lx\n",
		(unsigned long) (isp_read(0x00000704)));
	pr_info("ISPYI_GU_ADDR = 0x%lx\n",
		(unsigned long) (isp_read(0x00000708)));
	pr_info("ISPYI_BV_ADDR = 0x%lx\n",
		(unsigned long) (isp_read(0x0000070c)));
	pr_info("ISPYI_STRIDE1 = 0x%lx\n",
		(unsigned long) (isp_read(0x00000710)));
	pr_info("ISPYI_STRIDE2 = 0x%lx\n",
		(unsigned long) (isp_read(0x00000714)));
	pr_info("ISPYI_COL_STRIDE1 = 0x%lx\n",
		(unsigned long) (isp_read(0x00000718)));
	pr_info("ISPYI_COL_STRIDE2 = 0x%lx\n",
		(unsigned long) (isp_read(0x0000071c)));
	pr_info("ISPYI_RY_EADDR = 0x%lx\n",
		(unsigned long) (isp_read(0x00000720)));
	pr_info("ISPYI_GU_EADDR = 0x%lx\n",
		(unsigned long) (isp_read(0x00000724)));
	pr_info("ISPYI_BV_EADDR: = 0x%lx\n",
		(unsigned long) (isp_read(0x00000728)));
	pr_info("ISPYC_MATRIX = 0x%lx\n",
		(unsigned long) (isp_read(ISPYC_MATRIX_OFFSET)));
	pr_info("ISPYC_OFFSET = 0x%lx\n",
		(unsigned long) (isp_read(ISPYC_OFFSET_OFFSET)));
	pr_info("ISPLR_TSCALEX = 0x%lx\n",
		(unsigned long) (isp_read(ISPLR_TSCALEX_OFFSET)));
	pr_info("ISPLR_TSCALEY = 0x%lx\n",
		(unsigned long) (isp_read(ISPLR_TSCALEY_OFFSET)));
	pr_info("ISPLR_NORM_0_1 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLR_NORM_0_1_OFFSET)));
	pr_info("ISPLR_NORM_2_3 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLR_NORM_2_3_OFFSET)));
	pr_info("ISPLR_SHIFT = 0x%lx\n",
		(unsigned long) (isp_read(ISPLR_SHIFT_OFFSET)));
	pr_info("ISPLO_CTRL = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_CTRL_OFFSET)));
	pr_info("ISPLO_COL_STRIDE1 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_COL_STRIDE1_OFFSET)));
	pr_info("ISPLO_COL_STRIDE2 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_COL_STRIDE2_OFFSET)));
	pr_info("ISPLO_ADDRESS1 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_ADDRESS1_OFFSET)));
	pr_info("ISPLO_ADDRESS2 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_ADDRESS2_OFFSET)));
	pr_info("ISPLO_ADDRESS3 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_ADDRESS3_OFFSET)));
	pr_info("ISPLO_STRIDE1 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_STRIDE1_OFFSET)));
	pr_info("ISPLO_STRIDE2 = 0x%lx\n",
		(unsigned long) (isp_read(ISPLO_STRIDE2_OFFSET)));
	return;
}

static mm_isr_type_e  process_isp_irq(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	u32 ispStatus = 0;
	u32 ctrl = 0;
	ispStatus = isp_read(ISP_STATUS_OFFSET);
	isp_write(ISP_STATUS_OFFSET, ispStatus);
	ctrl = isp_read(ISP_CTRL_OFFSET);

	if (ispStatus & ISP_CTRL_EOT_IMASK_MASK) {
		/* end of tile interrupt, disable control reg,
		    as queue head job completed, schedule tasklet again*/
		isp_clr_bit32(ISP_CTRL_OFFSET, ISP_CTRL_ENABLE_MASK);
		return MM_ISR_SUCCESS;
	} else if (ispStatus & ISP_CTRL_ERROR_IMASK_MASK) {
		printispregs(isp);
		pr_err("process_isp_irq: Error intr from ISP!!!");
		return MM_ISR_ERROR;
	} else {
		pr_err("process_isp_irq: Unknown intr from ISP!!!");
	}
	return MM_ISR_UNKNOWN;
}

static bool get_isp_status(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	u32 ispStatus = 0;
	ispStatus = isp_read(ISP_STATUS_OFFSET);
	/* Check if hw busy */
	if (ispStatus & 0x1)
		return true;
	return false;
}

static int isp_reset(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	int ret = 0;
	u32 ispStatus = 0;
	u32 ispCtrl = 0;
	u32 dummy;

	ispStatus = isp_read(ISP_STATUS_OFFSET);
	if (ispStatus & ISP_STATUS_STATE_ENABLED) {
		ispCtrl = isp_read(ISP_CTRL_OFFSET);
		if (ispCtrl & ISP_CTRL_ENABLE_MASK) {
			ispCtrl &= ~ISP_CTRL_ENABLE_MASK;
			isp_write(ISP_CTRL_OFFSET,
				(ispCtrl|ISP_CTRL_ERROR_IMASK_MASK));
			do {
				ispStatus = isp_read(ISP_STATUS_OFFSET);
			} while ((ispStatus & (ISP_STATUS_STATE_ENABLED |
					ISP_STATUS_ERROR_INT_MASK)) ==
				ISP_STATUS_STATE_ENABLED);

			if (ispStatus & ISP_STATUS_ERROR_INT_MASK) {
				isp_write(ISP_STATUS_OFFSET,
					ISP_STATUS_ERROR_INT_MASK);
				pr_err("%s: chal_isp_reset: error interrupt\n",
							__func__);
			}

			/* remove the ERROR interrupt */
			isp_write(ISP_CTRL_OFFSET, ispCtrl);
		}
	}

	ispCtrl = isp_read(ISP_CTRL_OFFSET);
	ispCtrl |= ISP_CTRL_FORCE_CLKEN_MASK;
	isp_write(ISP_CTRL_OFFSET, ispCtrl);
	dummy = isp_read(ISP_CTRL_OFFSET);
	ispCtrl = isp_read(ISP_CTRL_OFFSET);
	ispCtrl |= ISP_CTRL_FLUSH_MASK;
	isp_write(ISP_CTRL_OFFSET, ispCtrl);
	ispCtrl = isp_read(ISP_CTRL_OFFSET);
	return ret;
}

static int isp_abort(void *id, mm_job_post_t *job)
{
	int ret;
	ret = isp_reset(id);
	pr_info("isp_abort");
	return ret;
}

int isp_program(struct isp_device_t *isp, struct isp_job_post_t *job_post)
{
	int ret = 0;
	int i;
	ret = isp_reset(isp);
	if (ret != 0) {
		pr_err("isp_reset failed with %d", ret);
		return ret;
	}
	for (i = 0; i < job_post->num_regs; i++)
		isp_write(job_post->isp_regs[i].offset,
				job_post->isp_regs[i].value);
	return ret;
}

int isp_start(struct isp_device_t *isp)
{
	int ret = 0;
	u32 ctrl;
	ctrl = isp_read(ISP_CTRL_OFFSET);
	ctrl |= ISP_CTRL_ENABLE_MASK;
	ctrl &= ~ISP_CTRL_FORCE_CLKEN_MASK;
	isp_write(ISP_CTRL_OFFSET, ctrl);
	return ret;
}

static mm_job_status_e isp_start_job(void *id , mm_job_post_t *job,
						unsigned int profmask)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	struct isp_job_post_t *job_params = (struct isp_job_post_t *)job->data;
	mm_job_status_e ret = 0;
	switch (job->status) {
	case MM_JOB_STATUS_READY:
		{
			ret = isp_program(isp, job_params);
			if (ret != 0) {
				pr_err("isp_program failed with %d", ret);
				job->status = MM_JOB_STATUS_ERROR;
				return MM_JOB_STATUS_ERROR;
			}
			/*printispregs(isp);*/
			ret = isp_start(isp);
			if (ret != 0) {
				pr_err("isp_start failed with %d", ret);
				job->status = MM_JOB_STATUS_ERROR;
				return MM_JOB_STATUS_ERROR;
			}
			job->status = MM_JOB_STATUS_RUNNING;
			return MM_JOB_STATUS_RUNNING;
		}
		break;
	case MM_JOB_STATUS_RUNNING:
		{
			pr_debug("isp_start_job: MM_JOB_STATUS_RUNNING\n");
			job->status = MM_JOB_STATUS_SUCCESS;
			return MM_JOB_STATUS_SUCCESS;
		}
		break;
	}
	return MM_JOB_STATUS_ERROR;
}

static struct isp_device_t *isp_device;

static void mm_isp_update_virt_addr(void *vaddr)
{
	isp_device->vaddr = vaddr;
}

int __init mm_isp_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	isp_device = kmalloc(sizeof(struct isp_device_t), GFP_KERNEL);
	isp_device->vaddr = NULL;
	pr_debug("mm_isp_init: ISP driver Module Init");

	core_param.mm_base_addr = ISP_BASE_ADDR;
	core_param.mm_hw_size = ISP_HW_SIZE;
	core_param.mm_irq = IRQ_ISP;

	/*core_param.mm_timer = DEFAULT_MM_DEV_TIMER_MS;*/
	/*core_param.mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;*/
	core_param.mm_timer = 100;
	core_param.mm_timeout = 100;

	core_param.mm_get_status = get_isp_status;
	core_param.mm_start_job = isp_start_job;
	core_param.mm_process_irq = process_isp_irq;
	core_param.mm_init = isp_reset;
	core_param.mm_deinit = isp_reset;
	core_param.mm_abort = isp_abort;
	core_param.mm_get_regs = NULL;
	core_param.mm_update_virt_addr = mm_isp_update_virt_addr;
	core_param.mm_version_init = NULL;
	core_param.mm_device_id = (void *)isp_device;
	core_param.mm_virt_addr = NULL;
	core_param.core_name = "ISP";

	dvfs_param.__on = 1;
	dvfs_param.__mode = TURBO;
	dvfs_param.__ts = DEFAULT_MM_DEV_DVFS_SAMPLING_MS;
	dvfs_param.eco_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.nor_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.nor_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;
	dvfs_param.tur_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.tur_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;
	dvfs_param.st_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;
	dvfs_param.eco_high = 0;
	dvfs_param.nor_high = 80;
	dvfs_param.nor_low = 0;
	dvfs_param.tur_high = 80;
	dvfs_param.tur_low = 45;
	dvfs_param.st_low = 45;
	dvfs_param.dvfs_bulk_job_cnt = 0;

	isp_device->fmwk_handle = mm_fmwk_register(ISP_DEV_NAME,
					ISP_AXI_BUS_CLK_NAME_STR, 1,
					&core_param, &dvfs_param, &prof_param);

	if ((isp_device->fmwk_handle == NULL) ||
		(isp_device->vaddr == NULL)) {
		ret = -ENOMEM;
		goto err;
	}
	pr_debug("ISP driver Module Init over");
	return ret;

err:
	kfree(isp_device);
	pr_err("ISP driver Module Init Error");
	return ret;
}

void __exit mm_isp_exit(void)
{
	pr_debug("ISP driver Module Exit");
	if (isp_device->fmwk_handle)
		mm_fmwk_unregister(isp_device->fmwk_handle);
	kfree(isp_device);
}

module_init(mm_isp_init);
module_exit(mm_isp_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("ISP device driver");
MODULE_LICENSE("GPL");
