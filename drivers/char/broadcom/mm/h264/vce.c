/******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

#define pr_fmt(fmt) "vce: " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include <asm/mach/map.h>
#include "vce.h"
/*To Be replaced with RDB*/
#include "vce_reg.h"
#include "vce_prerun_obj.h"
#include "vce_postrun_obj.h"
#include "h264_enc.h"
#include "h264_dec.h"

#define H264_HW_SIZE (7*1024*1024)

/*From DEC3*/
#define VCE_OBTAIN_SEMAPHORE_ONLY	0x40000000
#define VCE_RELEASE_SEMAPHORE_ONLY	0x80000000
#define VCE_NO_SEMAPHORE			0xc0000000
#define VCE_NO_WAIT					0x20000000
#define VCE_STOP_SYM_RESET_INNER	0x5
#define VCE_REASON_END				0x7

#ifdef H264_VCE_STEP
static int break_var;
#endif

struct vce_device_t {
	void *vaddr;
};

void h264_write(void *id, u32 reg, u32 value)
{
	struct vce_device_t *vce = (struct vce_device_t *)id;
	mm_write_reg(vce->vaddr, reg, value);
}

u32 h264_read(void *id, u32 reg)
{
	struct vce_device_t *vce = (struct vce_device_t *)id;
	u32 val = mm_read_reg(vce->vaddr, reg);
	return val;
}
static void vce_write(struct vce_device_t *vce, u32 reg, u32 value)
{
	*((u32 *)(vce->vaddr + reg + VCE_BASE - VIDEOCODEC_BASE_ADDRESS)) =
									value;
}

static u32 vce_read(struct vce_device_t *vce, u32 reg)
{
	return mm_read_reg(vce->vaddr,
			reg + (VCE_BASE - VIDEOCODEC_BASE_ADDRESS));
}

/*static void print_job_struct(void *job)
{
	struct vce_launch_info_t *vce_info;
	vce_info = (struct vce_launch_info_t *) ((u32 *)job);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("datasize: 0x%x\n", vce_info->datasize);
	pr_debug("data_addr_phys: 0x%x\n", vce_info->data_addr_phys);
	pr_debug("codesize: 0x%x\n", vce_info->codesize);
	pr_debug("code_addr_phys: 0x%x\n", vce_info->code_addr_phys);
	pr_debug("startaddr: 0x%x\n", vce_info->startaddr);
	pr_debug("finaladdr: 0x%x\n", vce_info->finaladdr);
	pr_debug("endcode: 0x%x\n", vce_info->endcode);
	pr_debug("stop_reason: 0x%x\n", vce_info->stop_reason);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}*/

static void print_regs(struct vce_device_t *vce)
{
	pr_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_info(" VCE_STATUS: 0x%x\n", vce_read(vce, VCE_STATUS_OFFSET));
	pr_info(" VERSION: 0x%x\n", vce_read(vce, VCE_VERSION_OFFSET));
	pr_info(" PC_PF0: 0x%x\n", vce_read(vce, VCE_PC_PF0_OFFSET));
	pr_info(" PC_IF0: 0x%x\n", vce_read(vce, VCE_PC_IF0_OFFSET));
	pr_info(" PC_RD0: 0x%x\n", vce_read(vce, VCE_PC_RD0_OFFSET));
	pr_info(" PC_EX0: 0x%x\n", vce_read(vce, VCE_PC_EX0_OFFSET));
	pr_info(" BAD_ADDR: 0x%x\n", vce_read(vce, VCE_BAD_ADDR_OFFSET));
	pr_info(" PC_ERR: 0x%x\n", vce_read(vce, VCE_PC_ERR_OFFSET));
	pr_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static int vce_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	return 0;
}

static int vce_reset(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;

	/* Initialise VCE Control register*/
	vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_CLEAR_RUN);

	/*Clear interrupt bit if set*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 1<<31);

	/*Clear SEM_CLEAR*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 1<<25);

	/*Clear VCE_BAD_ADDR_OFFSET*/
	vce_write(id, VCE_BAD_ADDR_OFFSET, 0x0);

	while (vce_read(id, VCE_STATUS_OFFSET) & VCE_STATUS_BUSYBITS_MASK)
		;

	return 0;
}

static int vce_abort(void *device_id, mm_job_post_t *job)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;

	pr_info("vce_abort:\n");
	print_regs(id);
	vce_reset(id);

	return 0;
}

static int vce_block_init(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	/*Need to do clock gating Here*/

	/*Reset the registers*/
	vce_reset(id);

	return 0;
}

static int vce_block_deinit(void *device_id)
{
	/*Need to do clock gating here*/
	return 0;
}

static mm_isr_type_e process_vce_irq(void *device_id)
{
	u32 flags;

	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	struct vce_device_t *id = (struct vce_device_t *)device_id;

	/* Clear RUN*/
	vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_CLEAR_RUN);

	/* Read the interrupt status registers */
	flags = vce_read(id, VCE_STATUS_OFFSET);

	/* Clear interrupts isr(VCINTMASK0) is going to handle */
	vce_write(id, 0x3408b8, 1<<26);

	/*Clear interrupt bit*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 1<<31);

	irq_retval = MM_ISR_SUCCESS;

	return irq_retval;
}

bool get_vce_status(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	u32 status;
	u32 reason;

	/*Read the status to find Hardware status*/
	status  = vce_read(id, VCE_STATUS_OFFSET);
	if (status & VCE_STATUS_BUSYBITS_MASK) {
		reason = (status >> VCE_STATUS_REASON_POS) &
			VCE_STATUS_REASON_MASK;
		/*TODO: Add more checks if required*/
		if (reason == VCE_REASON_END
				/*Encode has reason 0*/
				|| reason == 0)
			return false;
		return true;
	}

	return false;
}

mm_job_status_e vce_start_job(void *device_id, mm_job_post_t *job,
						u32 prfmask)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	unsigned char *jp = (unsigned char *)job->data;
	struct vce_launch_info_t *vce_info;

	u32 status = 0;
	u32 i = 0;
	int ft_csize, lt_csize;

	struct enc_info_t *enc_info = NULL;
	struct dec_info_t *dec_info = NULL;

	if (jp == NULL) {
		pr_err("vce_start_job: id or jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->type) {
	case H264_VCE_ENC_SLICE_JOB:
		if (job->size != sizeof(struct vce_launch_info_t))
			enc_info = (struct enc_info_t *)(jp + \
					sizeof(struct vce_launch_info_t));

		vce_info = (struct vce_launch_info_t *) \
				(jp);
		break;
	case H264_VCE_DEC_SLICE_JOB:
		dec_info = (struct dec_info_t *)(jp + \
					(sizeof(struct vce_launch_info_t)));
		vce_info = (struct vce_launch_info_t *) \
				(jp);
		break;
	case H264_VCE_LAUNCH_JOB:
		vce_info = (struct vce_launch_info_t *) \
				(jp);
		break;
	case H264_VCE_RESET_CODEC_JOB:
		completeDecodeFrame(id);
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;
	default:
		pr_err("unknown job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		if (enc_info != NULL)
			encodeSlice(id, enc_info);
		if (dec_info != NULL)
			decodeSlice(id, dec_info);
		/*Reset VCE*/
		/*Bound checks*/
		/*vce_info->codesize is in words*/
			if (vce_info->codesize > VCE_PROGRAM_MEM_SIZE) {
				pr_err("vce_start_job: Error in CodeSize\n");
			return MM_JOB_STATUS_ERROR;
		}

		/*Program VCE*/
		/*Program the arguments for prerun*/
		lt_csize = vce_info->codesize - VCE_DMA_LIMIT;

		if (lt_csize > 0)
			ft_csize = VCE_DMA_LIMIT;
		else
			ft_csize = vce_info->codesize;

		vce_write(id, (VCE_REGISTERS_OFFSET+(1*4)), \
			vce_info->code_addr_phys);
		vce_write(id, (VCE_REGISTERS_OFFSET+(2*4)), \
			ft_csize);
		vce_write(id, (VCE_REGISTERS_OFFSET+(3*4)), \
			vce_info->data_addr_phys);
		vce_write(id, (VCE_REGISTERS_OFFSET+(4*4)), \
			vce_info->datasize);

		/*set return address to the finalising code*/
		vce_write(id, VCE_REGISTERS_OFFSET, \
				VCE_DMA_LIMIT);

		/*Copy Prerun Code*/
		for (i = 0; i < (vce_launch_vce_prerun[3]/4); i++) {
			vce_write(id, (VCE_PROGRAM_MEM_OFFSET + \
				(VCE_DMA_LIMIT)+(i*4)), (u32)* \
				((u32 *)((u32 *) \
				vce_launch_vce_prerun[2]) + i));
		}

		/*Write Start address*/
		vce_write(id, VCE_PC_PF0_OFFSET, VCE_DMA_LIMIT);

		/*Taken from DEC3 - TODO: Verify again*/
		if (vce_info->endcode & (~VCE_NO_SEMAPHORE)) {
				/*Pulse the stoppage code 1->0->1 to clear
				* internal wait VCE might be waiting on another
				* semaphore, or bkpt*/
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
				vce_write(id, VCE_SEMA_SET_OFFSET, \
					1<<(vce_info->endcode & \
					(~VCE_NO_SEMAPHORE)) | \
						1<<VCE_STOP_SYM_RESET_INNER);
		} else {
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
		}

		/*Enable Interrupt*/
		vce_write(id, 0x3408b8, 1<<26);

#ifdef H264_VCE_STEP
		/*Debug: For Stepping through VCE Instructions*/
		while (break_var != 0) {
			vce_write(id, VCE_CONTROL_OFFSET,
						VCE_CONTROL_SINGLE_STEP);
		}
#endif

		/*Set RUN Bit*/
		vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_SET_RUN);

		job->status = MM_JOB_STATUS_RUNNING;
		return MM_JOB_STATUS_RUNNING;
	case MM_JOB_STATUS_RUNNING:
		/*Program VCE*/

		/*Copy extra code*/
		lt_csize = vce_info->codesize - VCE_DMA_LIMIT;

		if (lt_csize > 0)
			ft_csize = VCE_DMA_LIMIT;
		else
			ft_csize = vce_info->codesize;

		if (lt_csize > 0) {
			char *va;
			va = (char *)(jp + (sizeof(struct vce_launch_info_t)));

			if (enc_info != NULL)
				va = va + (sizeof(struct enc_info_t));
			if (dec_info != NULL)
				va = va + vce_info->pvt_job_size;

			lt_csize /= 4;
			/*Copy Extra code(After VCE_DMA_LIMIT)*/
			for (i = 0; i < lt_csize; i++) {
				vce_write(id, (VCE_PROGRAM_MEM_OFFSET + \
					ft_csize + (i*4)), (u32)* \
					((u32 *)((u32 *) \
					va + i)));
			}
		}

		/*Write the registers passed*/
		for (i = 0; i < VCE_REGISTERS_COUNT; i++) {
			if (((vce_info->vce_regpst.changed_mask) >> i) & 0x1) {
				vce_write(id, \
					(VCE_REGISTERS_OFFSET+(i*4)), \
					vce_info->vce_regpst.vce_regs[i]);
			}
		}
		/*set return address to the finalising code*/
		vce_write(id, VCE_REGISTERS_OFFSET, \
				vce_info->finaladdr);

		/*Write Start address*/
		vce_write(id, VCE_PC_PF0_OFFSET, vce_info->startaddr);

		/*Taken from DEC3 - TODO: Verify again*/
		if (vce_info->endcode & (~VCE_NO_SEMAPHORE)) {
			/*Pulse the stoppage code 1->0->1 to clear
			* internal wait VCE might be waiting on another
			* semaphore, or bkpt*/
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
			vce_write(id, VCE_SEMA_SET_OFFSET, \
				1<<(vce_info->endcode&(~VCE_NO_SEMAPHORE)) | \
						1<<VCE_STOP_SYM_RESET_INNER);
		} else {
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
		}

		/*Enable Interrupt*/
		vce_write(id, 0x3408b8, 1<<26);

		/*Set RUN Bit*/
		vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_SET_RUN);
		job->status = MM_JOB_STATUS_RUNNING1;

		return MM_JOB_STATUS_RUNNING;

	case MM_JOB_STATUS_RUNNING1:
		/*Handle Job completion*/
		status = vce_read(id, VCE_STATUS_OFFSET);

		vce_info->stop_reason =
			(status >> VCE_STATUS_REASON_POS) & \
			VCE_STATUS_REASON_MASK;

		if (vce_read(id, VCE_BAD_ADDR_OFFSET) != 0) {
			pr_err("vce_start_job: Bad_Addr Err [0x%08x]\n", \
				vce_read(id, VCE_BAD_ADDR_OFFSET));
			pr_err("vce_start_job: PC Err [0x%08x]\n", \
				vce_read(id, VCE_PC_ERR_OFFSET));
			vce_write(id, VCE_BAD_ADDR_OFFSET, 0x0);
			return MM_JOB_STATUS_ERROR;
		}

		/*Backup Registers*/
		for (i = 0; i < VCE_REGISTERS_COUNT; i++) {
			vce_info->vce_regpst.vce_regs[i] =
				vce_read(id, (VCE_REGISTERS_OFFSET+(i*4)));
		}


		/*Write the registers passed*/
		vce_write(id, (VCE_REGISTERS_OFFSET+(1*4)), \
			vce_info->data_upaddr_phys);
		vce_write(id, (VCE_REGISTERS_OFFSET+(2*4)), \
			vce_info->datasize);


		/*set return address to the finalising code*/
		vce_write(id, VCE_REGISTERS_OFFSET, \
					0x0);

		/*Copy Postrun Code*/
		for (i = 0; i < (vce_launch_vce_postrun[3])/4; i++) {
			vce_write(id, (VCE_PROGRAM_MEM_OFFSET + \
				(i*4)), (u32)*((u32 *)((u32 *) \
				vce_launch_vce_postrun[2])+i));
			}

		/*Write Start address*/
		vce_write(id, VCE_PC_PF0_OFFSET, 0x0);

		/*Taken from DEC3 - TODO: Verify again*/
		if (vce_info->endcode & (~VCE_NO_SEMAPHORE)) {
			/*Pulse the stoppage code 1->0->1 to clear
			* internal wait VCE might be waiting on another
			* semaphore, or bkpt*/
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
			vce_write(id, VCE_SEMA_SET_OFFSET, \
				1<<(vce_info->endcode&(~VCE_NO_SEMAPHORE)) | \
						1<<VCE_STOP_SYM_RESET_INNER);
		} else {
			vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
		}

		/*Enable Interrupt*/
		vce_write(id, 0x3408b8, 1<<26);

		/*Set RUN Bit*/
		vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_SET_RUN);
		job->status = MM_JOB_STATUS_RUNNING2;

		return MM_JOB_STATUS_RUNNING;

	case MM_JOB_STATUS_RUNNING2:

		if (enc_info != NULL)
			completeEncodeSlice(id, enc_info);
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;
	case MM_JOB_STATUS_SUCCESS:
	case MM_JOB_STATUS_ERROR:
	case MM_JOB_STATUS_NOT_FOUND:
	case MM_JOB_STATUS_TIMED_OUT:
	case MM_JOB_STATUS_SKIP:
	case MM_JOB_STATUS_LAST:
	default:
		pr_err("vce_start_job: Unexpected Job status\n");
	}
	return MM_JOB_STATUS_ERROR;
}
struct vce_device_t *vce_device;

void h264_vce_update_virt(void *virt)
{
	vce_device->vaddr = virt;
}

int h264_vce_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	vce_device = kmalloc(sizeof(struct vce_device_t), GFP_KERNEL);
	if (vce_device == NULL) {
		pr_err("h264_vce_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	vce_device->vaddr = NULL;

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr = VIDEOCODEC_BASE_ADDRESS;
	core_param->mm_hw_size = H264_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_AOB;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_vce_status;
	core_param->mm_start_job = vce_start_job;
	core_param->mm_process_irq = process_vce_irq;
	core_param->mm_init = vce_block_init;
	core_param->mm_deinit = vce_block_deinit;
	core_param->mm_abort = vce_abort;
	core_param->mm_get_regs = vce_get_regs;
	core_param->mm_update_virt_addr = h264_vce_update_virt;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id = (void *)vce_device;
	core_param->mm_virt_addr = NULL;

	return ret;
err:
	pr_err("h264_vce_init: Error\n");
	return ret;
}

void h264_vce_deinit(void)
{
	kfree(vce_device);
}
