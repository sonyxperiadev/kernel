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
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

struct interlock_device_t {
	void *fmwk_handle;
};

static int interlock_reset(void *device_id)
{
	return 0;
}

static int interlock_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	return 0;
}

static int interlock_abort(void *device_id, mm_job_post_t *job)
{
	return 0;
}

static mm_isr_type_e process_interlock_irq(void *device_id)
{
	return MM_ISR_SUCCESS;
}

bool get_interlock_status(void *device_id)
{
	return false;
}

mm_job_status_e interlock_start_job(void *device_id, mm_job_post_t *job,
						unsigned int profmask)
{
	job->status = MM_JOB_STATUS_SUCCESS;
	return MM_JOB_STATUS_SUCCESS;
}
static struct interlock_device_t *interlock_device;

static void interlock_virt_addr_update(void *vaddr) {}

int __init interlock_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	interlock_device = kmalloc(sizeof(struct interlock_device_t),
							GFP_KERNEL);
	pr_debug("INTERLOCK driver Module Init");

	core_param.mm_base_addr = 0;
	core_param.mm_hw_size = 0;
	core_param.mm_irq = 0;

	core_param.mm_timer = 0;
	core_param.mm_timeout = 0;

	core_param.mm_get_status = get_interlock_status;
	core_param.mm_start_job = interlock_start_job;
	core_param.mm_process_irq = process_interlock_irq;
	core_param.mm_init = interlock_reset;
	core_param.mm_deinit = interlock_reset;
	core_param.mm_abort = interlock_abort;
	core_param.mm_get_regs = interlock_get_regs;
	core_param.mm_update_virt_addr = interlock_virt_addr_update;
	core_param.mm_version_init = NULL;
	core_param.mm_device_id = (void *)interlock_device;
	core_param.mm_virt_addr = NULL;
	core_param.core_name = "INTERLOCK";

	dvfs_param.__on = 0;
	dvfs_param.__mode = ECONOMY;
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

	interlock_device->fmwk_handle = mm_fmwk_register(INTERLOCK_DEV_NAME,
						NULL, 1, &core_param,
						&dvfs_param, &prof_param);

	if (interlock_device->fmwk_handle == NULL) {
		ret = -ENOMEM;
		goto err;
	}
	pr_debug("INTERLOCK driver Module Init over");
	return ret;

err:
	pr_err("INTERLOCK driver Module Init Error");
	return ret;
}

void __exit interlock_exit(void)
{
	pr_debug("INTERLOCK driver Module Exit");
	if (interlock_device->fmwk_handle)
		mm_fmwk_unregister(interlock_device->fmwk_handle);
	kfree(interlock_device);
}


module_init(interlock_init);
module_exit(interlock_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("INTERLOCK device driver");
MODULE_LICENSE("GPL");
