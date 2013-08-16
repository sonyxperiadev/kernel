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

#define pr_fmt(fmt) "ol: " fmt

#include <linux/kernel.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#include <asm/mach/map.h>
#include "ol.h"
#include "h264_parse/ol_parse_ifc.h"

#define VIDEOCODEC_BASE		0x3d000000
#define OL_BASE				0x3d00cc00
#define OL_HW_SIZE			0x1000

struct ol_device_t {
	void *vaddr;
};

struct ol_device_t *ol_device;

void ol_write(u32 reg, u32 value)
{
	mm_write_reg(ol_device->vaddr,
		reg - (OL_BASE - VIDEOCODEC_BASE), value);
}

u32 ol_read(u32 reg)
{
	return mm_read_reg(ol_device->vaddr,
			reg - (OL_BASE - VIDEOCODEC_BASE));
}

static void print_regs(struct ol_device_t *ol)
{
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	pr_debug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

static void ol_reg_init(void *device_id)
{
}

static int ol_get_regs(void *device_id, MM_REG_VALUE *ptr, int count)
{
	struct ol_device_t *id = (struct ol_device_t *)device_id;
	pr_debug("ol_get_regs:\n");
	print_regs(id);
	return 0;
}

static int ol_reset(void *device_id)
{
	struct ol_device_t *id = (struct ol_device_t *)device_id;
	pr_debug("ol_reset:\n");

	/*do reg init*/
	ol_reg_init(id);

	return 0;
}

static int ol_abort(void *device_id, mm_job_post_t *job)
{
	struct ol_device_t *id = (struct ol_device_t *)device_id;

	pr_info("ol_abort:\n");
	ol_reset(id);

	return 0;
}

static mm_isr_type_e process_ol_irq(void *device_id)
{
	mm_isr_type_e irq_retval = MM_ISR_UNKNOWN;

	return irq_retval;
}

bool get_ol_status(void *device_id)
{
	return false;
}

mm_job_status_e ol_start_job(void *device_id , mm_job_post_t *job,
				 u32 profmask)
{
	struct ol_device_t *id = (struct ol_device_t *)device_id;
	struct ol_job_info_t *jp = (struct ol_job_info_t *)job->data;
	u8 *ptr = (u8 *) jp;
	u8 *spl_ptr = (u8 *) job->spl_data_ptr;
	ptr += sizeof(struct ol_job_info_t);

	if (jp == NULL) {
		pr_err("ol_start_job: id or jp is null\n");
		return MM_JOB_STATUS_ERROR;
	}

	if (job->type <= H264_OL_INVALID_JOB ||
		job->type >= H264_OL_LAST_JOB) {
		pr_err("ol_start_job: Invalid job type\n");
		return MM_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MM_JOB_STATUS_READY:
		/*Reset OL*/
		ol_reset(id);
		/*Bound checks*/
		/*Program OL*/
		/*Setup*/
		/*Attach*/
		olsi_attach_ifc((u8 *) jp + sizeof(struct ol_job_info_t), jp->buffer_base_phys, jp->buffer_end_phys, jp->start_pos, jp->length);
		if (jp->in_nal_bits) {
			SiAdvanceS_ifc((u8 *) jp + sizeof(struct ol_job_info_t), jp->in_nal_bits);
		}
		/*Job Pgm*/
		switch (job->type) {
		/*H264*/
		case H264_GET_OFFSET_TYPE:
			jp->error =  0;
			break;
		case  H264_SPS_PARSE_TYPE:
			jp->error =  h264_parse_sps_ifc(ptr);
			break;
		case  H264_PPS_PARSE_TYPE:
			jp->error =  h264_parse_pps_ifc(ptr, spl_ptr);
			break;
		case H264_SLICE_HDR1_PARSE_TYPE:
			jp->error =  h264_parse_slice_header_1_ifc(ptr, spl_ptr);
			break;
		case H264_SLICE_HDR2_PARSE_TYPE:
			jp->error =  h264_parse_slice_header_2_ifc(ptr, spl_ptr);
			break;
		case H264_SPS_EXT_PARSE_TYPE:
			jp->error =  h264_parse_sps_extension_ifc(ptr);
			break;
		case H264_MVC_SSPS_PARSE_TYPE:
			jp->error =  h264_parse_mvc_ssps_ifc(ptr);
			break;
		case H264_AU_DELIMITER_PARSE_TYPE:
			jp->error =  h264_parse_au_delimiter_ifc(ptr);
			break;
		case H264_NAL_HDR_EXT_PARSE_TYPE:
			jp->error =  h264_parse_nal_header_extension_ifc(ptr);
			break;
		case H264_SLICE_ID_PARSE_TYPE:
			jp->error =  h264_parse_slice_id_ifc(ptr);
			break;
		case H264_SLICE_BC_PARTN_PARSE_TYPE:
			jp->error =  h264_parse_slice_bc_partition_ifc(ptr, spl_ptr);
			break;
		case H264_PPS_SPSID_PARSE_TYPE:
			jp->error =  h264_parse_pps_spsid_ifc(ptr);
			break;
		case H264_HDR1_PPSID_PARSE_TYPE:
			jp->error =  h264_parse_header_1_ppsid_ifc(ptr);
			break;
		/*MP4*/
		case MP4_VOL_PARSE_TYPE:
			jp->error =  mp4_parse_video_object_layer_ifc(ptr);
			break;
		case MP4_VOP_HDR_PARSE_TYPE:
			jp->error =  mp4_parse_vop_header_ifc(ptr);
			break;
		case MP4_VISUAL_OBJ_PARSE_TYPE:
			jp->error =  mp4_parse_visual_object_ifc(ptr);
			break;
		/*VC1*/
		case VC1_SEQUENCE_PARSE_TYPE:
			jp->error =  vc1_parse_sequence_ifc(ptr);
			break;
		case VC1_ENTRY_POINT_PARSE_TYPE:
			jp->error =  vc1_parse_entrypoint_ifc(ptr);
			break;
		case VC1_PICTURE_PARSE_TYPE:
			jp->error =  vc1_parse_picture_ifc(ptr);
			break;
		case VC1_PICTURE_PARSE_2_TYPE:
			jp->error =  vc1_parse_picture_2_ifc(ptr);
			break;
		case VC1_SLICE_PARSE_TYPE:
			jp->error =  vc1_parse_slice_ifc(ptr);
			break;
		default:
			pr_err("ol_start_job: Invalid Job Type \n");
		}
		/*detatch*/
		if (jp->error) {
			pr_err("%s: jp->error = 0x%x and job->type = 0x%x \n", __func__, jp->error, job->type);
		}
		jp->end_pos = SiDetachS_ifc((u8 *) jp + sizeof(struct ol_job_info_t));
		jp->in_nal_bits = jp->end_pos;
		job->status = MM_JOB_STATUS_SUCCESS;
		return MM_JOB_STATUS_SUCCESS;

	case MM_JOB_STATUS_RUNNING:
	case MM_JOB_STATUS_SUCCESS:
	case MM_JOB_STATUS_ERROR:
	case MM_JOB_STATUS_NOT_FOUND:
	case MM_JOB_STATUS_TIMED_OUT:
	case MM_JOB_STATUS_SKIP:
	case MM_JOB_STATUS_LAST:
	default:
		pr_err("ol_start_job: Unexpected Job status\n");
	}
	return MM_JOB_STATUS_ERROR;
}

void h264_ol_update_virt(void *virt)
{
	pr_debug("ol_update_virt:\n");
	ol_device->vaddr = virt;
}

int h264_ol_init(MM_CORE_HW_IFC *core_param)
{
	int ret = 0;

	ol_device = kmalloc(sizeof(struct ol_device_t), GFP_KERNEL);
	if (ol_device == NULL) {
		pr_err("h264_ol_init: kmalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	ol_device->vaddr = NULL;
	pr_debug("h264_ol_init: -->\n");

	/*Do any device specific structure initialisation required.*/
	core_param->mm_base_addr = OL_BASE;
	core_param->mm_hw_size = OL_HW_SIZE;
	core_param->mm_irq = BCM_INT_ID_H264_AOB;

	core_param->mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param->mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param->mm_get_status = get_ol_status;
	core_param->mm_start_job = ol_start_job;
	core_param->mm_process_irq = process_ol_irq;
	core_param->mm_init = ol_reset;
	core_param->mm_deinit = ol_reset;
	core_param->mm_abort = ol_abort;
	core_param->mm_get_regs = ol_get_regs;
	core_param->mm_update_virt_addr = h264_ol_update_virt;
	core_param->mm_version_init = NULL;
	core_param->mm_device_id = (void *)ol_device;
	core_param->mm_virt_addr = NULL;

	return ret;
err:
	pr_err("h264_ol_init: Error\n");
	return ret;
}

void h264_ol_deinit(void)
{
	pr_debug("h264_ol_deinit:\n");
	kfree(ol_device);
}
