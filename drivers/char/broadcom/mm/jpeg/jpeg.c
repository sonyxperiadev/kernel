/******************************************************************************* Copyright 2010 Broadcom Corporation.  All rights reserved.
Unless you and Broadcom execute a separate written software license agreement
governing use of this software,  this software is licensed to you under the
terms of the GNU General Public License version 2,  available at
http://www.gnu.org/copyleft/gpl.html  (the "GPL").
Notwithstanding the above,  under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL,  without Broadcom's express prior written consent.
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_jp.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "jpeg_params.h"
#include "jpeg.h"
#define JPEG_HW_SIZE  (4*1024)
#define IRQ_JPEG	(217+32)
struct jpeg_device_t {
	void *vaddr;
	void *fmwk_handle;
};
#define jpeg_write(reg,  value) mm_write_reg(jpeg->vaddr,  reg,  value)
#define jpeg_read(reg) mm_read_reg(jpeg->vaddr,  reg)

static mm_isr_type_e  process_jpeg_irq(void *id)
{
	pr_err("in side irq\n");
	struct jpeg_device_t *jpeg = (struct jpeg_device_t *)id;
	u32 jpegStatus = 0;
	jpegStatus = jpeg_read(JP_ICST_OFFSET);
	jpeg_write(JP_ICST_OFFSET,  jpegStatus);
	if (!(jpegStatus & JICST_SDONE) && !(jpegStatus & JICST_CDONE))
		return MM_ISR_UNKNOWN;
	if ((jpegStatus & JICST_SDONE))
		return MM_ISR_ERROR;
	if ((jpegStatus & JICST_INTE))
		return MM_ISR_ERROR;
	return MM_ISR_SUCCESS;
}
static bool get_jpeg_status(void *id)
{
	u32 jpegStatus = 0;
	struct jpeg_device_t *jpeg = (struct jpeg_device_t *)id;
	jpegStatus = jpeg_read(JP_CTRL_OFFSET);
		if ((jpegStatus & JCTRL_START) &&  (jpegStatus & JCTRL_WOUT))
			return true;
	return false;
}
static int jpeg_reset(void *id)
{
	struct jpeg_device_t *jpeg =  (struct jpeg_device_t *)id;
	int ret = 0;
	int jpegstatus = jpeg_read(JP_CTRL_OFFSET);
	jpeg_write(JP_CTRL_OFFSET,  (jpegstatus | JCTRL_RESET));
	while (jpeg_read(JP_CTRL_OFFSET) & JCTRL_RESET)
		;
	return ret;
}
static int jpeg_abort(void *id,  mm_job_post_t *job)
{
	struct jpeg_device_t *jpeg = (struct jpeg_device_t *)id;
	struct jpeg_value *jp = (struct jpeg_value *)job->data;
	int ret = 0;
	ret = jpeg_reset(id);
	return ret;
}

static mm_job_status_e jpeg_start_job(void *id ,  mm_job_post_t *job, \
	unsigned int profmask)
{
	struct jpeg_device_t *jpeg = (struct jpeg_device_t *)id;
	struct jpeg_value *jp = (struct jpeg_value *)job->data;
	u32 i, j, k, jicst, reg_value, jpegStatus;
	switch (job->status) {
	case MM_JOB_STATUS_READY:
		{
		if (jp->out_size_p > jp->hardware_add_out_p) {
			job->status = MM_JOB_STATUS_ERROR;
			return MM_JOB_STATUS_ERROR;
			}
		jpeg_write(JP_CTRL_OFFSET ,  (1 << 30) | JCTRL_RESET |
		JCTRL_STUFF | JCTRL_DCTEN);
		jpeg_write(JP_MCTRL_OFFSET ,  ((3 * JMCTRL_NUMCMP) |
		((jp->h) * (jp->v) * JMCTRL_CMP(0) | 0*JMCTRL_DC_TAB(0) |
		0 * JMCTRL_AC_TAB(0)) | (1 * JMCTRL_CMP(1) |
		1 * JMCTRL_DC_TAB(1) | 1 * JMCTRL_AC_TAB(1)) |
		(1 * JMCTRL_CMP(2) | 1 * JMCTRL_DC_TAB(2) |
		1 * JMCTRL_AC_TAB(2))));
		if (jp->h == 1 && jp->v == 1) {
			reg_value = jpeg_read(JP_MCTRL_OFFSET);
			jpeg_write(JP_MCTRL_OFFSET ,  reg_value |
			JMCTRL_444_MODE);
		} else if (jp->h == 2 && jp->v == 1) {
			reg_value = jpeg_read(JP_MCTRL_OFFSET);
			jpeg_write(JP_MCTRL_OFFSET ,  (reg_value |
			JMCTRL_422_MODE));
		} else if (jp->h == 2 && jp->v == 2) {
			reg_value = jpeg_read(JP_MCTRL_OFFSET);
			jpeg_write(JP_MCTRL_OFFSET ,  reg_value |
			JMCTRL_420_MODE);
		} else {
			pr_err("Unsupported subsampling mode DCT hardware.");
			return MM_JOB_STATUS_ERROR;
		}
		jpeg_write(JP_HADDR_OFFSET ,  (JHADDR_TABLEF | 0x200));
		for (k = 0; k < 0x10; k++) {
			jpeg_write(JP_HWDATA_OFFSET ,  \
			((huffsize_Y_DC[k] << 16) | huffcode_Y_DC[k]));
		}
		reg_value = jpeg_read(JP_HADDR_OFFSET);
		jpeg_write(JP_HADDR_OFFSET ,  (reg_value & (~JHADDR_TABLEF)));
		jpeg_write(JP_HADDR_OFFSET ,  (JHADDR_TABLEF | 0x0));
		for (k = 0; k < 0x100; k++) {
			jpeg_write(JP_HWDATA_OFFSET , \
		((huffsize_Y_AC[k] << 16) | huffcode_Y_AC[k]));
		}
		reg_value = jpeg_read(JP_HADDR_OFFSET);
		jpeg_write(JP_HADDR_OFFSET ,  (reg_value & (~JHADDR_TABLEF)));
		jpeg_write(JP_HADDR_OFFSET ,  (JHADDR_TABLEF | 0x210));
		for (k = 0; k < 0x10; k++) {
			jpeg_write(JP_HWDATA_OFFSET , \
			((huffsize_C_DC[k] << 16) | huffcode_C_DC[k]));
		}
		reg_value = jpeg_read(JP_HADDR_OFFSET);
		jpeg_write(JP_HADDR_OFFSET ,  (reg_value & (~JHADDR_TABLEF)));
		jpeg_write(JP_HADDR_OFFSET ,  (JHADDR_TABLEF | 0x100));
		for (k = 0; k < 0x100; k++) {
			jpeg_write(JP_HWDATA_OFFSET , \
			((huffsize_C_AC[k] << 16) | huffcode_C_AC[k]));
		}
		reg_value = jpeg_read(JP_HADDR_OFFSET);
		jpeg_write(JP_HADDR_OFFSET ,  (reg_value & (~JHADDR_TABLEF)));
		jpeg_write(JP_QADDR_OFFSET ,  (JQADDR_RAMACC | (0 * 64)));
		for (i = 0; i < 64; i++)
			jpeg_write(JP_QWDATA_OFFSET ,  (hw_quantf_t_Y[i]));
		jpeg_write(JP_QADDR_OFFSET ,  (JQADDR_RAMACC | (1 * 64)));
		for (i = 0; i < 64; i++)
			jpeg_write(JP_QWDATA_OFFSET ,  (hw_quantf_t_C[i]));
		jpeg_write(JP_QADDR_OFFSET ,  0);
		jpeg_write(JP_QCTRL_OFFSET ,  ((0 << 0) | (1 << 2) | (1 << 4)));
		jpeg_write(JP_SDA_OFFSET ,  ((jp->hardware_add_out_p) & ~0xF));
		jpeg_write(JP_SBO_OFFSET ,  (8 * (jp->p & 0xF)));
		jpeg_write(JP_NSB_OFFSET ,  jp->jnsb);
		int sumhv = (jp->h)*(jp->v) + 2;
		jpeg_write(JP_NCB_OFFSET , (64 * (sumhv) * (jp->xmcus) *\
		(jp->ymcus)));
		jpeg_write(JP_CBA_OFFSET ,  (~0));
		jpeg_write(JP_C0BA_OFFSET ,  ((jp->hardware_add_Y)));
		jpeg_write(JP_C1BA_OFFSET ,  ((jp->hardware_add_U)));
		jpeg_write(JP_C2BA_OFFSET ,  ((jp->hardware_add_V)));
		jpeg_write(JP_C0S_OFFSET ,  (jp->stride_0));
		jpeg_write(JP_C1S_OFFSET ,  (jp->stride_1));
		jpeg_write(JP_C2S_OFFSET ,  (jp->stride_2));
		jpeg_write(JP_C0W_OFFSET ,  jp->jc0w);
		jpeg_write(JP_C1W_OFFSET ,  jp->jc1w);
		jpeg_write(JP_C2W_OFFSET ,  jp->jc2w);
		jpeg_write(JP_ICST_OFFSET ,  (JICST_INTCD | JICST_INTSD |
		JICST_INTE));
		reg_value = jpeg_read(JP_CTRL_OFFSET);
		jpeg_write(JP_CTRL_OFFSET ,  (reg_value | JCTRL_START));
		while (jpeg_read(JP_CTRL_OFFSET) & (JCTRL_START | JCTRL_WOUT))
			;
		job->status = MM_JOB_STATUS_RUNNING1;
		return MM_JOB_STATUS_RUNNING1;
		}
		break;
	case MM_JOB_STATUS_RUNNING1:
		{	pr_err("in side kernel\n");
			reg_value = jpeg_read(JP_CTRL_OFFSET);
			jpeg_write(JP_CTRL_OFFSET ,  (reg_value | JCTRL_FLUSH));
			while ((jpeg_read(JP_CTRL_OFFSET)) &
				(JCTRL_FLUSH | JCTRL_WOUT))
				;
			job->status = MM_JOB_STATUS_RUNNING;
			return MM_JOB_STATUS_RUNNING;
		}
		break;
	case MM_JOB_STATUS_RUNNING:
		{	pr_err("in side kernel\n");
			reg_value = jpeg_read(JP_NSB_OFFSET);
			jp->jnsb_callback_value = reg_value;
			job->status = MM_JOB_STATUS_SUCCESS;
			return MM_JOB_STATUS_SUCCESS;
		}
		break;
		}
		return MM_JOB_STATUS_ERROR;
}
static struct jpeg_device_t *jpeg_device;
static void mm_jpeg_update_virt_addr(void *vaddr)
{
	jpeg_device->vaddr = vaddr;
}
int __init mm_jpeg_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	jpeg_device = kmalloc(sizeof(struct jpeg_device_t),  GFP_KERNEL);
	jpeg_device->vaddr = NULL;
	pr_debug("JPEG driver Module Init");
	core_param.mm_base_addr = JP_BASE_ADDR;
	core_param.mm_hw_size = JPEG_HW_SIZE;
	core_param.mm_irq = IRQ_JPEG;
	/*core_param.mm_timer = DEFAULT_MM_DEV_TIMER_MS;*/
	/*core_param.mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;*/
	core_param.mm_timer = 20;
	core_param.mm_timeout = 1000;
	core_param.mm_get_status = get_jpeg_status;
	core_param.mm_start_job = jpeg_start_job;
	core_param.mm_process_irq = process_jpeg_irq;
	core_param.mm_init = jpeg_reset;
	core_param.mm_deinit = jpeg_reset;
	core_param.mm_abort = jpeg_abort;
	core_param.mm_get_regs = NULL;
	core_param.mm_update_virt_addr = mm_jpeg_update_virt_addr;
	core_param.mm_version_init = NULL;
	core_param.mm_device_id = (void *)jpeg_device;
	core_param.mm_virt_addr = NULL;
	core_param.core_name = "JPEG";
	dvfs_param.__on = 1;
	dvfs_param.__mode = TURBO;
	dvfs_param.eco_high = 0;
	dvfs_param.nor_high = 0;
	dvfs_param.nor_low = 0;
	dvfs_param.tur_high = 120;
	dvfs_param.tur_low = 0;
	dvfs_param.st_low = 120;
	dvfs_param.__ts = DEFAULT_MM_DEV_DVFS_SAMPLING_MS;
	dvfs_param.eco_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.nor_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.nor_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;
	dvfs_param.tur_ns_high = DEFAULT_MM_DEV_DVFS_UP_SAMPLES;
	dvfs_param.tur_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;
	dvfs_param.st_ns_low = DEFAULT_MM_DEV_DVFS_DOWN_SAMPLES;

	dvfs_param.dvfs_bulk_job_cnt = 0;
	jpeg_device->fmwk_handle = mm_fmwk_register(JPEG_DEV_NAME, \
	JPEG_AXI_BUS_CLK_NAME_STR,  1, &core_param,  &dvfs_param,  &prof_param);
	if ((jpeg_device->fmwk_handle == NULL) ||
	(jpeg_device->vaddr == NULL)) {
		ret = -ENOMEM;
		goto err;
	}
	return ret;
err:
	pr_err("JPEG driver Module Init Error");
	return ret;
}
void __exit mm_jpeg_exit(void)
{
	pr_debug("JPEG driver Module Exit");
	if (jpeg_device->fmwk_handle)
		mm_fmwk_unregister(jpeg_device->fmwk_handle);
	kfree(jpeg_device);
}
module_init(mm_jpeg_init);
module_exit(mm_jpeg_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("JPEG device driver");
MODULE_LICENSE("GPL");
