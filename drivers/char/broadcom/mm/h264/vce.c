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
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include "vce.h"
/*To Be replaced with RDB*/
#include "vce_reg.h"

#include "h264_enc.h"

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
	pr_debug("write reg [0x%08x] val [0x%08x]\n", reg, value);
	mm_write_reg(vce->vaddr, reg, value);
}

u32 h264_read(void *id, u32 reg)
{
	struct vce_device_t *vce = (struct vce_device_t *)id;
	u32 val = mm_read_reg(vce->vaddr, reg);
	pr_debug("read reg [0x%08x] val [0x%08x]\n", reg, val);
	return val;
}
static void vce_write(struct vce_device_t *vce, u32 reg, u32 value)
{
	mm_write_reg(vce->vaddr,
		reg + (VCE_BASE - VIDEOCODEC_BASE_ADDRESS), value);
}

static u32 vce_read(struct vce_device_t *vce, u32 reg)
{
	return mm_read_reg(vce->vaddr,
			reg + (VCE_BASE - VIDEOCODEC_BASE_ADDRESS));
}

static void print_job_struct(void *job)
{
	struct vce_launch_info_t *vce_info;
	vce_info = (struct vce_launch_info_t *) ((u32 *)job + *((u32 *)job+2));
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("datasize: 0x%x\n", vce_info->datasize);
	pr_debug("data_offset: 0x%x\n", vce_info->data_offset);
	pr_debug("codesize: 0x%x\n", vce_info->codesize);
	pr_debug("code_offset: 0x%x\n", vce_info->code_offset);
	pr_debug("startaddr: 0x%x\n", vce_info->startaddr);
	pr_debug("finaladdr: 0x%x\n", vce_info->finaladdr);
	pr_debug("numdownloads: 0x%x\n", vce_info->numdownloads);
	pr_debug("numuploads: 0x%x\n", vce_info->numuploads);
	pr_debug("download_start_offset: 0x%x\n",
				vce_info->download_start_offset);
	pr_debug("upload_start_offset: 0x%x\n", vce_info->upload_start_offset);
	pr_debug("endcode: 0x%x\n", vce_info->endcode);
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void print_regs(struct vce_device_t *vce)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug(" VCE_STATUS: 0x%x\n", vce_read(vce, VCE_STATUS_OFFSET));
	pr_debug(" VERSION: 0x%x\n", vce_read(vce, VCE_VERSION_OFFSET));
	pr_debug(" PC_PF0: 0x%x\n", vce_read(vce, VCE_PC_PF0_OFFSET));
	pr_debug(" PC_IF0: 0x%x\n", vce_read(vce, VCE_PC_IF0_OFFSET));
	pr_debug(" PC_RD0: 0x%x\n", vce_read(vce, VCE_PC_RD0_OFFSET));
	pr_debug(" PC_EX0: 0x%x\n", vce_read(vce, VCE_PC_EX0_OFFSET));
	pr_debug(" BAD_ADDR: 0x%x\n", vce_read(vce, VCE_BAD_ADDR_OFFSET));
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void vce_reg_init(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	pr_debug("vce_reg_init:\n");

	/* Initialise VCE Control register*/
	vce_write(id, VCE_CONTROL_OFFSET, VCE_CONTROL_CLEAR_RUN);

	/*Clear SEM_CLEAR*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 1<<31);

	/*Clear interrupt bit if set*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 0xff);
}

static int vce_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	pr_debug("vce_get_regs:\n");
	print_regs(id);
	return 0;
}

static int vce_reset(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	pr_debug("vce_reset:\n");

	/*do reg init*/
	vce_reg_init(id);

	while (vce_read(id, VCE_STATUS_OFFSET) & VCE_STATUS_BUSYBITS_MASK)
		;

	return 0;
}

static int vce_abort(void *device_id, mm_job_post_t *job)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;

	pr_debug("vce_abort:\n");
	vce_reset(id);

	return 0;
}

static mm_isr_type_e process_vce_irq(void *device_id)
{
	u32 flags;
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;
	struct vce_device_t *id = (struct vce_device_t *)device_id;

	/* Read the interrupt status registers */
	flags = vce_read(id, VCE_STATUS_OFFSET);

	/* Clear interrupts isr(VCINTMASK0) is going to handle */
	vce_write(id, 0x3408b8, 1<<26);

	/*Clear interrupt bit*/
	vce_write(id, VCE_SEMA_CLEAR_OFFSET, 1<<31);

	/*TODO: Implement other reason Handling if required*/
	irq_retval = MM_ISR_SUCCESS;

	return irq_retval;
}

bool get_vce_status(void *device_id)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	u32 status;
	u32 reason;
	pr_debug("get_vce_status:\n");

	/*Read the status to find Hardware status*/
	status  = vce_read(id, VCE_STATUS_OFFSET);
	if (status & VCE_STATUS_BUSYBITS_MASK) {
		reason = (status >> VCE_STATUS_REASON_POS) &
			VCE_STATUS_REASON_MASK;
		/*TODO: Add more checks if required*/
		if (reason == VCE_REASON_END
				/*Encode has reason 0*/
				|| reason == 0) {
			pr_debug("get_vce_status : reason VCE_REASON_END\n");
			return false;
		} else {
			pr_debug("get_vce_status : returns true\n");
			return true;
		}
	}

	pr_debug("get_vce_status : returns false\n");
	return false;
}

mm_job_status_e vce_start_job(void *device_id, mm_job_post_t *job,
						u32 prfmask)
{
	struct vce_device_t *id = (struct vce_device_t *)device_id;
	u32 *jp = (u32 *)job->data;
	struct vce_launch_info_t *vce_info;
	/*Variables for Download-Upload*/
	u32 *transfer_ptr = NULL;
	enum vce_transfer_type_t transfer_dir;
	u32 vce_addr_offset;
	u32 transfer_size;
	u32 *transfer_data_ptr = NULL;

	u32 status = 0;
	u32 i = 0;
	u32 j = 0;

	struct enc_info_t *enc_info = NULL;

	if (jp == NULL) {
		pr_err("vce_start_job: id or jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->type) {
	case H264_VCE_ENC_SLICE_JOB:
		enc_info = (struct enc_info_t *)((u32 *)jp + *((u32 *)jp+2) + \
					(sizeof(struct vce_launch_info_t)/4));
	case H264_VCE_LAUNCH_JOB:
		vce_info = (struct vce_launch_info_t *) \
				((u32 *)jp + *((u32 *)jp+2));
		break;
	default:
		pr_err("unknown job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		switch (job->type) {
		case H264_VCE_ENC_SLICE_JOB:
			encodeSlice(id, enc_info);
		case H264_VCE_LAUNCH_JOB:
		print_regs(id);
		print_job_struct(jp);
		/*Reset VCE*/
		vce_reset(id);
		/*Bound checks*/
		/*vce_info->codesize is in words*/
		if (vce_info->codesize > 4096) {
			pr_err("vce_start_job: Error in CodeSize\n");
			return MM_JOB_STATUS_ERROR;
		}
		/*Program VCE*/

		/*Write the registers passed*/
		for (i = 0; i < VCE_REGISTERS_COUNT; i++) {
			if (vce_info->vce_regpst.changed[i] == 1) {
					vce_write(id, \
					(VCE_REGISTERS_OFFSET+(i*4)), \
					vce_info->vce_regpst.vce_regs[i]);
			}
		}

		/*set return address to the finalising code*/
			vce_write(id, VCE_REGISTERS_OFFSET, \
					vce_info->finaladdr);

		/*Copy Code*/
		for (i = 0; i < (vce_info->codesize); i++) {
			vce_write(id, (VCE_PROGRAM_MEM_OFFSET+(i*4)),
				(u32)*((u32 *)(jp + vce_info->code_offset)+i));
		}
		/*Copy DATA*/
		for (i = 0; i < (vce_info->datasize); i++) {
			vce_write(id, (VCE_DATA_MEM_OFFSET+(i*4)),
				(u32)*((u32 *)(jp + vce_info->data_offset)+i));
		}
		/*Handling Download Variables*/
		transfer_ptr = (u32 *)((u32 *) jp +
					vce_info->download_start_offset);
		for (i = 0; i < (vce_info->numdownloads); i++) {
			transfer_dir = *transfer_ptr;
			if (transfer_dir != VCE_DATA_DOWNLOAD) {
				pr_err("Upload Data in Download loop\n");
				break;
			}
			/*Pointing to toaddr*/
			transfer_ptr++;
			vce_addr_offset = *transfer_ptr;
			/*Pointing to size*/
			transfer_ptr++;
			transfer_size = *transfer_ptr;
			/*Pointing to data*/
			transfer_ptr++;
			transfer_data_ptr = transfer_ptr;
			for (j = 0; j < transfer_size; j++) {
					vce_write(id, (VCE_DATA_MEM_OFFSET + \
							vce_addr_offset+j*4), \
				(u32)*((u32 *)(transfer_data_ptr+j)));
			}
			transfer_ptr += transfer_size;
		}
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
		default:
			pr_err("unknown job type\n");
			return MM_JOB_STATUS_ERROR;
		}

	case MM_JOB_STATUS_RUNNING:
		switch (job->type) {
		case H264_VCE_ENC_SLICE_JOB:
		case H264_VCE_LAUNCH_JOB:
		/*Handle Job completion*/
		status = vce_read(id, VCE_STATUS_OFFSET);

			vce_info->stop_reason =
				(status >> VCE_STATUS_REASON_POS) & \
				VCE_STATUS_REASON_MASK;

		if (vce_read(id, VCE_BAD_ADDR_OFFSET) != 0) {
			pr_err("vce_start_job: Bad Error [0x%08x]\n", \
				vce_read(id, VCE_BAD_ADDR_OFFSET));
			return MM_JOB_STATUS_ERROR;
		}

		/*Backup Registers*/
		for (i = 0; i < VCE_REGISTERS_COUNT; i++) {
			vce_info->vce_regpst.vce_regs[i] =
				vce_read(id, (VCE_REGISTERS_OFFSET+(i*4)));
		}

		/* Handle Upload*/
		transfer_ptr = (u32 *)((u32 *) jp +
					vce_info->upload_start_offset);
		for (i = 0; i < (vce_info->numuploads); i++) {
			transfer_dir = *transfer_ptr;
			if (transfer_dir != VCE_DATA_UPLOAD) {
				pr_err("Upload Data in Download loop\n");
				break;
			}
			/*Pointing to toaddr*/
			transfer_ptr++;
			vce_addr_offset = *transfer_ptr;
			/*Pointing to size*/
			transfer_ptr++;
			transfer_size = *transfer_ptr;
			/*Pointing to data*/
			transfer_ptr++;
			transfer_data_ptr = transfer_ptr;
			for (j = 0; j < transfer_size; j++) {
					*((u32 *)(transfer_data_ptr+j)) = \
					vce_read(id, (VCE_DATA_MEM_OFFSET + \
							vce_addr_offset+j*4));
			}
			transfer_ptr += transfer_size;
		}

		if (enc_info != NULL)
			completeEncodeSlice(id, enc_info);
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;
		default:
			pr_err("unknown job type\n");
			return MM_JOB_STATUS_ERROR;
		}

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
	pr_debug("vce_update_virt:\n");
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
	pr_debug("h264_vce_init: -->\n");

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr = VIDEOCODEC_BASE_ADDRESS;
	core_param->mm_hw_size = H264_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_AOB;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_vce_status;
	core_param->mm_start_job = vce_start_job;
	core_param->mm_process_irq = process_vce_irq;
	core_param->mm_init = vce_reset;
	core_param->mm_deinit = vce_reset;
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
	pr_debug("h264_vce_deinit:\n");
	kfree(vce_device);
}
