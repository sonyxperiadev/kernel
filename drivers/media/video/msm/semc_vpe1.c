/* drivers/media/video/msm/semc_vpe1.c
 *
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/io.h>
#define CONFIG_MSM_CAMERA_DEBUG
#include "semc_vpe1.h"
#include <linux/pm_qos_params.h>
#include <linux/android_pmem.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <mach/clk.h>
#include <asm/div64.h>

#define MSM_VPE_NAME "msm_vpe_standalone"
/*
#undef CDBG
#define CDBG(fmt, args...) pr_info(fmt, ##args)
*/

static int vpe_update_scaler(struct video_crop_t *pcrop);
static struct class *msm_vpe_class;
static dev_t msm_vpe_devno;
static struct msm_vpe_device vpe_device_data;
static struct msm_vpe_device *vpe_device;
static struct vpe_ctrl_type *vpe_ctrl;

#define ERR_USER_COPY(to) pr_err("%s(%d): copy %s user\n", \
				__func__, __LINE__, ((to) ? "to" : "from"))
#define ERR_COPY_FROM_USER() ERR_USER_COPY(0)

#define CHECKED_COPY_FROM_USER(in) {				\
	if (copy_from_user((in), (void __user *)cmd->value,	\
			cmd->length)) {				\
		rc = -EFAULT;					\
		break;						\
	}							\
}

/*
static   struct vpe_cmd_type vpe_cmd[] = {
		{VPE_DUMMY_0, 0},
		{VPE_SET_CLK, 0},
		{VPE_RESET, 0},
		{VPE_START, 0},
		{VPE_ABORT, 0},
		{VPE_OPERATION_MODE_CFG, VPE_OPERATION_MODE_CFG_LEN},
		{VPE_INPUT_PLANE_CFG, VPE_INPUT_PLANE_CFG_LEN},
		{VPE_OUTPUT_PLANE_CFG, VPE_OUTPUT_PLANE_CFG_LEN},
		{VPE_INPUT_PLANE_UPDATE, VPE_INPUT_PLANE_UPDATE_LEN},
		{VPE_SCALE_CFG_TYPE, VPE_SCALER_CONFIG_LEN},
		{VPE_ROTATION_CFG_TYPE, 0},
		{VPE_AXI_OUT_CFG, 0},
		{VPE_CMD_DIS_OFFSET_CFG, VPE_DIS_OFFSET_CFG_LEN},
};
*/

static long long vpe_do_div(long long num, long long den)
{
	do_div(num, den);
	return num;
}

static int vpe_start(void)
{
	/*  enable the frame irq, bit 0 = Display list 0 ROI done */
	msm_io_w(1, vpe_device->vpebase + VPE_INTR_ENABLE_OFFSET);
	msm_io_dump(vpe_device->vpebase + 0x10000, 0x250);
	/* this triggers the operation. */
	msm_io_w(1, vpe_device->vpebase + VPE_DL0_START_OFFSET);

	return 0;
}

static void vpe_reset_state_variables(void)
{
	/* initialize local variables for state control, etc.*/
	vpe_ctrl->op_mode = 0;
	vpe_ctrl->state = 0;
	spin_lock_init(&vpe_ctrl->tasklet_lock);
	spin_lock_init(&vpe_ctrl->state_lock);
	INIT_LIST_HEAD(&vpe_ctrl->tasklet_q);
	init_waitqueue_head(&vpe_ctrl->wait);
}

static int vpe_reset(void)
{
	uint32_t vpe_version;
	uint32_t rc = 0;

	vpe_reset_state_variables();
	vpe_version = msm_io_r(vpe_device->vpebase + VPE_HW_VERSION_OFFSET);
	CDBG("vpe_version = 0x%x\n", vpe_version);

	/* disable all interrupts.*/
	msm_io_w(0, vpe_device->vpebase + VPE_INTR_ENABLE_OFFSET);
	/* clear all pending interrupts*/
	msm_io_w(0x1fffff, vpe_device->vpebase + VPE_INTR_CLEAR_OFFSET);

	/* write sw_reset to reset the core. */
	msm_io_w(0x10, vpe_device->vpebase + VPE_SW_RESET_OFFSET);

	/* then poll the reset bit, it should be self-cleared. */
	while (1) {
		rc =
		msm_io_r(vpe_device->vpebase + VPE_SW_RESET_OFFSET) & 0x10;
		if (rc == 0)
			break;
	}

	/*  at this point, hardware is reset. Then pogram to default
		values. */
	msm_io_w(VPE_AXI_RD_ARB_CONFIG_VALUE,
		 vpe_device->vpebase + VPE_AXI_RD_ARB_CONFIG_OFFSET);

	msm_io_w(VPE_CGC_ENABLE_VALUE,
		 vpe_device->vpebase + VPE_CGC_EN_OFFSET);

	msm_io_w(1, vpe_device->vpebase + VPE_CMD_MODE_OFFSET);

	msm_io_w(VPE_DEFAULT_OP_MODE_VALUE,
		 vpe_device->vpebase + VPE_OP_MODE_OFFSET);

	msm_io_w(VPE_DEFAULT_SCALE_CONFIG,
		 vpe_device->vpebase + VPE_SCALE_CONFIG_OFFSET);

	return rc;
}

static void vpe_update_scale_coef(uint32_t *p)
{
	uint32_t i, offset;
	offset = *p;
	for (i = offset; i < (VPE_SCALE_COEFF_NUM + offset); i++) {
		msm_io_w(*(++p), vpe_device->vpebase + VPE_SCALE_COEFF_LSBn(i));
		msm_io_w(*(++p), vpe_device->vpebase + VPE_SCALE_COEFF_MSBn(i));
	}
}

static void vpe_input_plane_config(uint32_t *p)
{
	msm_io_w(*p, vpe_device->vpebase + VPE_SRC_FORMAT_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_SRC_UNPACK_PATTERN1_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_SRC_IMAGE_SIZE_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_SRC_YSTRIDE1_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_SRC_SIZE_OFFSET);
	vpe_ctrl->in_h_w = *p;
	msm_io_w(*(++p), vpe_device->vpebase + VPE_SRC_XY_OFFSET);
}

static void vpe_output_plane_config(uint32_t *p)
{
	msm_io_w(*p, vpe_device->vpebase + VPE_OUT_FORMAT_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_OUT_PACK_PATTERN1_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_OUT_YSTRIDE1_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_OUT_SIZE_OFFSET);
	msm_io_w(*(++p), vpe_device->vpebase + VPE_OUT_XY_OFFSET);
	vpe_ctrl->pcbcr_dis_offset = *(++p);
}

static int vpe_operation_config(uint32_t *p)
{
	uint32_t  out_w, out_h, temp;
	msm_io_w(*p, vpe_device->vpebase + VPE_OP_MODE_OFFSET);

	temp = msm_io_r(vpe_device->vpebase + VPE_OUT_SIZE_OFFSET);
	out_w = temp & 0xFFF;
	out_h = (temp & 0xFFF0000) >> 16;

	if (*p++ & 0xE00) {
		/* rotation enabled. */
		vpe_ctrl->out_w = out_h;
		vpe_ctrl->out_h = out_w;
	} else {
		vpe_ctrl->out_w = out_w;
		vpe_ctrl->out_h = out_h;
	}
	vpe_ctrl->dis_en = *p;
	return 0;
}

/* Later we can separate the rotation and scaler calc. If
*  rotation is enabled, simply swap the destination dimension.
*  And then pass the already swapped output size to this
*  function. */
static int vpe_update_scaler(struct video_crop_t *pcrop)
{
	uint32_t out_ROI_width, out_ROI_height;
	uint32_t src_ROI_width, src_ROI_height;

	uint32_t rc = 0;  /* default to no zoom. */
	/*
	* phase_step_x, phase_step_y, phase_init_x and phase_init_y
	* are represented in fixed-point, unsigned 3.29 format
	*/
	uint32_t phase_step_x = 0;
	uint32_t phase_step_y = 0;
	uint32_t phase_init_x = 0;
	uint32_t phase_init_y = 0;

	uint32_t src_roi, /*src_x, src_y, src_xy,*/ temp;
	uint32_t yscale_filter_sel, xscale_filter_sel;
	uint32_t scale_unit_sel_x, scale_unit_sel_y;
	uint64_t numerator, denominator;

	if ((pcrop->in2_w == pcrop->out2_w) &&
		(pcrop->in2_h == pcrop->out2_h)) {
		CDBG(" =======VPE no zoom needed.\n");

		temp = msm_io_r(vpe_device->vpebase + VPE_OP_MODE_OFFSET)
		& 0xfffffffc;
		msm_io_w(temp, vpe_device->vpebase + VPE_OP_MODE_OFFSET);

		return rc;
	}
	/* If fall through then scaler is needed.*/

	CDBG("========VPE zoom needed.\n");
	/* assumption is both direction need zoom. this can be
	improved. */
	temp = msm_io_r(vpe_device->vpebase + VPE_OP_MODE_OFFSET) | 0x3;
	msm_io_w(temp, vpe_device->vpebase + VPE_OP_MODE_OFFSET);

	src_ROI_width = pcrop->in2_w;
	src_ROI_height = pcrop->in2_h;
	out_ROI_width = pcrop->out2_w;
	out_ROI_height = pcrop->out2_h;

	CDBG("src w = 0x%x, h=0x%x, dst w = 0x%x, h =0x%x.\n",
		src_ROI_width, src_ROI_height, out_ROI_width,
		out_ROI_height);
	src_roi = (src_ROI_height << 16) + src_ROI_width;

	msm_io_w(src_roi, vpe_device->vpebase + VPE_SRC_SIZE_OFFSET);

	/* decide whether to use FIR or M/N for scaling */
	if ((out_ROI_width == 1 && src_ROI_width < 4) ||
		(src_ROI_width < 4 * out_ROI_width - 3))
		scale_unit_sel_x = 0;/* use FIR scalar */
	else
		scale_unit_sel_x = 1;/* use M/N scalar */

	if ((out_ROI_height == 1 && src_ROI_height < 4) ||
		(src_ROI_height < 4 * out_ROI_height - 3))
		scale_unit_sel_y = 0;/* use FIR scalar */
	else
		scale_unit_sel_y = 1;/* use M/N scalar */

	/* calculate phase step for the x direction */

	/* if destination is only 1 pixel wide,
	the value of phase_step_x
	is unimportant. Assigning phase_step_x to
	src ROI width as an arbitrary value. */
	if (out_ROI_width == 1) {
		phase_step_x = (uint32_t) ((src_ROI_width) <<
					SCALER_PHASE_BITS);

		/* if using FIR scalar */
	} else if (scale_unit_sel_x == 0) {

		/* Calculate the quotient ( src_ROI_width - 1 )
		/ ( out_ROI_width - 1)
		with u3.29 precision. Quotient is rounded up to
		the larger 29th decimal point. */
		numerator = (uint64_t)(src_ROI_width - 1) <<
					SCALER_PHASE_BITS;
		/* never equals to 0 because of the
		"(out_ROI_width == 1 )"*/
		denominator = (uint64_t)(out_ROI_width - 1);
		/* divide and round up to the larger 29th
		decimal point. */
		phase_step_x = (uint32_t) vpe_do_div((numerator +
					denominator - 1), denominator);
	} else if (scale_unit_sel_x == 1) { /* if M/N scalar */
		/* Calculate the quotient ( src_ROI_width ) /
		( out_ROI_width)
		with u3.29 precision. Quotient is rounded down to the
		smaller 29th decimal point. */
		numerator = (uint64_t)(src_ROI_width) <<
			SCALER_PHASE_BITS;
		denominator = (uint64_t)(out_ROI_width);
		phase_step_x =
			(uint32_t) vpe_do_div(numerator, denominator);
	}
	/* calculate phase step for the y direction */

	/* if destination is only 1 pixel wide, the value of
		phase_step_x is unimportant. Assigning phase_step_x
		to src ROI width as an arbitrary value. */
	if (out_ROI_height == 1) {
		phase_step_y =
		(uint32_t) ((src_ROI_height) << SCALER_PHASE_BITS);

	/* if FIR scalar */
	} else if (scale_unit_sel_y == 0) {
		/* Calculate the quotient ( src_ROI_height - 1 ) /
		( out_ROI_height - 1)
		with u3.29 precision. Quotient is rounded up to the
		larger 29th decimal point. */
		numerator = (uint64_t)(src_ROI_height - 1) <<
			SCALER_PHASE_BITS;
		/* never equals to 0 because of the "
		( out_ROI_height == 1 )" case */
		denominator = (uint64_t)(out_ROI_height - 1);
		/* Quotient is rounded up to the larger
		29th decimal point. */
		phase_step_y =
		(uint32_t) vpe_do_div(
			(numerator + denominator - 1), denominator);
	} else if (scale_unit_sel_y == 1) { /* if M/N scalar */
		/* Calculate the quotient ( src_ROI_height )
		/ ( out_ROI_height)
		with u3.29 precision. Quotient is rounded down
		to the smaller 29th decimal point. */
		numerator = (uint64_t)(src_ROI_height) <<
			SCALER_PHASE_BITS;
		denominator = (uint64_t)(out_ROI_height);
		phase_step_y = (uint32_t) vpe_do_div(
			numerator, denominator);
	}

	/* decide which set of FIR coefficients to use */
	if (phase_step_x > HAL_MDP_PHASE_STEP_2P50)
		xscale_filter_sel = 0;
	else if (phase_step_x > HAL_MDP_PHASE_STEP_1P66)
		xscale_filter_sel = 1;
	else if (phase_step_x > HAL_MDP_PHASE_STEP_1P25)
		xscale_filter_sel = 2;
	else
		xscale_filter_sel = 3;

	if (phase_step_y > HAL_MDP_PHASE_STEP_2P50)
		yscale_filter_sel = 0;
	else if (phase_step_y > HAL_MDP_PHASE_STEP_1P66)
		yscale_filter_sel = 1;
	else if (phase_step_y > HAL_MDP_PHASE_STEP_1P25)
		yscale_filter_sel = 2;
	else
		yscale_filter_sel = 3;

	/* calculate phase init for the x direction */

	/* if using FIR scalar */
	if (scale_unit_sel_x == 0) {
		if (out_ROI_width == 1) {
			phase_init_x =
				(uint32_t) ((src_ROI_width - 1) <<
							SCALER_PHASE_BITS);
		} else {
			phase_init_x = 0;
		}
	} else if (scale_unit_sel_x == 1) {
		/* M over N scalar  */
		phase_init_x = 0;
	}

	/* calculate phase init for the y direction
	if using FIR scalar */
	if (scale_unit_sel_y == 0) {
		if (out_ROI_height == 1) {
			phase_init_y =
			(uint32_t) ((src_ROI_height -
						1) << SCALER_PHASE_BITS);
		} else {
			phase_init_y = 0;
		}
	} else if (scale_unit_sel_y == 1) { /* M over N scalar   */
		phase_init_y = 0;
	}

	CDBG("phase step x = %d, step y = %d.\n",
		 phase_step_x, phase_step_y);
	CDBG("phase init x = %d, init y = %d.\n",
		 phase_init_x, phase_init_y);

	msm_io_w(phase_step_x, vpe_device->vpebase +
			VPE_SCALE_PHASEX_STEP_OFFSET);
	msm_io_w(phase_step_y, vpe_device->vpebase +
			VPE_SCALE_PHASEY_STEP_OFFSET);

	msm_io_w(phase_init_x, vpe_device->vpebase +
			VPE_SCALE_PHASEX_INIT_OFFSET);

	msm_io_w(phase_init_y, vpe_device->vpebase +
			VPE_SCALE_PHASEY_INIT_OFFSET);

	return 1;
}


static int vpe_proc_general(struct msm_vpe_cmd *cmd)
{
	int rc = 0;
	uint32_t *cmdp = NULL;
	CDBG("vpe_proc_general: cmdID = %d, length = %d\n",
		cmd->id,
		cmd->length);
	switch (cmd->id) {
	case VPE_RESET:
	case VPE_ABORT:
		rc = vpe_reset();
		break;

	case VPE_START:
		rc = vpe_start();
		break;

	case VPE_INPUT_PLANE_CFG:
		cmdp = kmalloc(cmd->length, GFP_ATOMIC);
		if (!cmdp) {
			rc = -ENOMEM;
			goto vpe_proc_general_done;
		}
		if (copy_from_user(cmdp,
			(void __user *)(cmd->value),
			cmd->length)) {
			rc = -EFAULT;
			goto vpe_proc_general_done;
		}
		vpe_input_plane_config(cmdp);
		break;

	case VPE_OPERATION_MODE_CFG:
		CDBG("cmd->length = %d\n", cmd->length);
		if (cmd->length != VPE_OPERATION_MODE_CFG_LEN) {
			rc = -EINVAL;
			goto vpe_proc_general_done;
		}
		cmdp = kmalloc(VPE_OPERATION_MODE_CFG_LEN,
					GFP_ATOMIC);
		if (copy_from_user(cmdp,
			(void __user *)(cmd->value),
			VPE_OPERATION_MODE_CFG_LEN)) {
			rc = -EFAULT;
			goto vpe_proc_general_done;
		}
		rc = vpe_operation_config(cmdp);
		CDBG("rc = %d\n", rc);
		break;

	case VPE_OUTPUT_PLANE_CFG:
		cmdp = kmalloc(cmd->length, GFP_ATOMIC);
		if (!cmdp) {
			rc = -ENOMEM;
			goto vpe_proc_general_done;
		}
		if (copy_from_user(cmdp,
			(void __user *)(cmd->value),
			cmd->length)) {
			rc = -EFAULT;
			goto vpe_proc_general_done;
		}
		vpe_output_plane_config(cmdp);
		break;

	case VPE_SCALE_CFG_TYPE:
		cmdp = kmalloc(cmd->length, GFP_ATOMIC);
		if (!cmdp) {
			rc = -ENOMEM;
			goto vpe_proc_general_done;
		}
		if (copy_from_user(cmdp,
			(void __user *)(cmd->value),
			cmd->length)) {
			rc = -EFAULT;
			goto vpe_proc_general_done;
		}
		vpe_update_scale_coef(cmdp);
		break;

	default:
		break;
	}
vpe_proc_general_done:
	kfree(cmdp);
	return rc;
}

static int vpe_set_dst_addr(unsigned long paddr,
			    unsigned long y_off,
			    unsigned long cbcr_off,
			    unsigned long x,
			    unsigned long y)
{
	uint32_t p1;
	p1 = ((y & 0xfff) << 16) | ((x & 0xfff) << 0);
	msm_io_w(p1, vpe_device->vpebase + VPE_OUT_XY_OFFSET);
	/* for dst  Y address */
	p1 = (paddr + y_off);
	msm_io_w(p1, vpe_device->vpebase + VPE_OUTP0_ADDR_OFFSET);
	/* for dst  CbCr address */
	p1 = (paddr + cbcr_off);
	msm_io_w(p1, vpe_device->vpebase + VPE_OUTP1_ADDR_OFFSET);

	return 0;
}

static int vpe_set_src_addr(unsigned long paddr,
			    unsigned long y_off,
			    unsigned long cbcr_off,
			    unsigned long x,
			    unsigned long y)
{
	uint32_t p1;
	p1 = ((y & 0x1fff) << 16) | ((x & 0x1fff) << 0);
	msm_io_w(p1, vpe_device->vpebase + VPE_SRC_XY_OFFSET);
	/* for src  Y address */
	p1 = (paddr + y_off);
	msm_io_w(p1, vpe_device->vpebase + VPE_SRCP0_ADDR_OFFSET);
	/* for src  CbCr address */
	p1 = (paddr + cbcr_off);
	msm_io_w(p1, vpe_device->vpebase + VPE_SRCP1_ADDR_OFFSET);

	return 0;
}

static int msm_vpe_config(struct msm_vpe_cfg_cmd *cmd, void *data)
{
	struct msm_vpe_cmd vpecmd;
	int rc = 0;
	if (copy_from_user(&vpecmd,
			   (void __user *)(cmd->value),
			   sizeof(vpecmd))) {
		pr_err("%s %d: copy_from_user failed\n", __func__,
			__LINE__);
		return -EFAULT;
	}
	CDBG("%s: cmd_type %d\n", __func__, cmd->cmd_type);
	switch (cmd->cmd_type) {
	case CMD_VPE:
		if (vpecmd.length > VPE_MAX_CMD_LENGTH) {
			pr_err("%s %d: cmd length over %d\n", __func__,
			__LINE__, vpecmd.length);
			return -EFAULT;
		} else {
			rc = vpe_proc_general(&vpecmd);
			CDBG(" rc = %d\n", rc);
		}
		break;

	default:
		break;
	}
	return rc;
}

static int msm_vpe_start_transfer(struct msm_vpe_transfer_cfg *transfercmd,
				  void *data)
{
	int rc = 0;
	struct msm_vpe_pmem_region *src_region;
	struct msm_vpe_pmem_region *dst_region;
	unsigned long src_paddr;
	unsigned long dst_paddr;
	struct msm_pmem_info *src_info;
	struct msm_pmem_info *dst_info;
	struct msm_vpe_crop_info *src_crop;
	struct msm_vpe_crop_info *dst_crop;
	struct video_crop_t crop_info;
	unsigned long src_kvstart;
	unsigned long src_len;
	struct file *src_file;
	unsigned long dst_kvstart;
	unsigned long dst_len;
	struct file *dst_file;
	bool put_pmem = false;
	unsigned long timeout = 2000;

	src_info = &transfercmd->src_info;
	dst_info = &transfercmd->dst_info;
	src_crop = &transfercmd->src_crop;
	dst_crop = &transfercmd->dst_crop;

	if (!list_empty(&vpe_ctrl->pmem_buf)) {
		list_for_each_entry(src_region, &vpe_ctrl->pmem_buf, list) {
			if (src_region->info.vaddr == transfercmd->srcAddr)
				break;
		}
		src_paddr = src_region->paddr;
		src_paddr += src_region->info.offset;
		list_for_each_entry(dst_region, &vpe_ctrl->pmem_buf, list) {
			if (dst_region->info.vaddr == transfercmd->dstAddr)
				break;
		}
		dst_paddr = dst_region->paddr;
		dst_paddr += dst_region->info.offset;
	} else {
		put_pmem = true;
		/* get pmem file for source */
		rc = get_pmem_file(src_info->fd,
				   &src_paddr,
				   &src_kvstart,
				   &src_len,
				   &src_file);
		if (rc < 0) {
			pr_err("%s: get_pmem_file fd %d error %d\n",
				__func__,
				src_info->fd, rc);
		}
		if (!src_info->len)
			src_info->len = src_len;
		src_paddr += src_info->offset;
		src_len = src_info->len;
		CDBG("%s:src type %d, paddr 0x%lx, vaddr 0x%lx\n",
		     __func__,
		     src_info->type,
		     src_paddr,
		     (unsigned long)src_info->vaddr);

	  /* get pmem file for destination */
	  rc = get_pmem_file(dst_info->fd,
			     &dst_paddr,
			     &dst_kvstart,
			     &dst_len,
			     &dst_file);
	  if (rc < 0) {
		pr_err("%s: get_pmem_file fd %d error %d\n",
				__func__,
				dst_info->fd, rc);
	  }
	  dst_paddr += dst_info->offset;
	  dst_len = dst_info->len;
	  CDBG("%s:dst type %d, paddr 0x%lx, vaddr 0x%lx\n",
			__func__,
			dst_info->type,
			dst_paddr,
			(unsigned long)dst_info->vaddr);
	}

	crop_info.in2_w = transfercmd->src_crop.w;
	crop_info.in2_h = transfercmd->src_crop.h;
	crop_info.out2_w = transfercmd->dst_crop.w;
	crop_info.out2_h = transfercmd->dst_crop.h;
	CDBG("Crop info in2_w = %d, in2_h = %d "
		"out2_h = %d out2_w = %d\n",
		crop_info.in2_w,
		crop_info.in2_h,
		crop_info.out2_h,
		crop_info.out2_w);

	vpe_update_scaler(&crop_info);

	vpe_set_src_addr(src_paddr,
			 src_info->y_off,
			 src_info->cbcr_off,
			 src_crop->x,
			 src_crop->y);
	vpe_set_dst_addr(dst_paddr,
			 dst_info->y_off,
			 dst_info->cbcr_off,
			 dst_crop->x,
			 dst_crop->y);

	/* kick vpe */
	vpe_ctrl->state = 1;
	vpe_start();

	/* wait vpe end */
	/* TODO: if we need asynchornous process ,
	   the waiting function move to GET_PICTURE ioctl */
	CDBG("Waiting for vpe end\n");
	rc = wait_event_interruptible_timeout(
		vpe_ctrl->wait,
		!vpe_ctrl->state,
		msecs_to_jiffies(timeout));
	CDBG("Waiting over for vpe end\n");

	if (put_pmem) {
		put_pmem_file(dst_file);
		put_pmem_file(src_file);

	}
	return rc;
}

static int check_pmem_info(struct msm_pmem_info *info, int len)
{
	if (info->offset < len &&
		info->offset + info->len <= len &&
		info->y_off < len &&
		info->cbcr_off < len)
		return 0;

	pr_err("%s: check failed: off %d len %d y %d cbcr %d (total len %d)\n",
		__func__,
		info->offset,
		info->len,
		info->y_off,
		info->cbcr_off,
		len);
	return -EINVAL;
}

static int msm_vpe_pmem_register(struct msm_vpe_register_cfg *registercmd,
				 void *data)
{
	int rc = 0;
	struct file *file;
	unsigned long paddr;
	unsigned long kvstart;
	unsigned long len;
	struct msm_vpe_pmem_region *region;

	CDBG("=== msm_vpe_pmem_register start ===\n");

	rc = get_pmem_file(registercmd->inf.fd, &paddr, &kvstart, &len, &file);
	if (rc < 0) {
		pr_err("%s: get_pmem_file fd %d error %d\n",
			__func__,
			registercmd->inf.fd, rc);
		return rc;
	}

	rc = check_pmem_info(&registercmd->inf, len);
	if (rc < 0)
		return rc;

	region = kmalloc(sizeof(struct msm_vpe_pmem_region), GFP_KERNEL);
	if (!region)
		return -ENOMEM;

	region->paddr = paddr;
	region->len = len;
	region->file = file;
	memcpy(&region->info, &registercmd->inf, sizeof(region->info));
	memcpy(&region->crop, &registercmd->crop, sizeof(region->crop));

	list_add_tail(&(region->list), &vpe_ctrl->pmem_buf);

	CDBG("=== msm_vpe_pmem_register end rc:%d===\n", rc);
	return rc;
}

static int msm_vpe_pmem_unregister(struct msm_vpe_unregister_cfg *unregistercmd,
				   void *data)
{
	int rc = 0;
	struct msm_vpe_pmem_region *region;

	CDBG("=== msm_vpe_pmem_unregister start ===\n");

	if (!list_empty(&vpe_ctrl->pmem_buf)) {
		list_for_each_entry(region, &vpe_ctrl->pmem_buf, list) {
			if (region->info.vaddr == unregistercmd->baseAddr) {
				list_del(&region->list);
				put_pmem_file(region->file);
				kfree(region);
				break;
			}
		}
	}
	CDBG("=== msm_vpe_pmem_unregister end rc:%d===\n", rc);
	return rc;
}

static void vpe_do_tasklet(unsigned long data)
{
	unsigned long flags;
	struct vpe_isr_queue_cmd_type *qcmd = NULL;

	CDBG("=== vpe_do_tasklet start ===\n");

	spin_lock_irqsave(&vpe_ctrl->tasklet_lock, flags);
	qcmd = list_first_entry(&vpe_ctrl->tasklet_q,
		struct vpe_isr_queue_cmd_type, list);

	if (!qcmd) {
		spin_unlock_irqrestore(&vpe_ctrl->tasklet_lock, flags);
		return;
	}

	list_del(&qcmd->list);
	spin_unlock_irqrestore(&vpe_ctrl->tasklet_lock, flags);

	/* interrupt to be processed,  *qcmd has the payload.  */
	if (qcmd->irq_status & 0x1) {
		CDBG("vpe plane0 frame done.\n");
		vpe_ctrl->state = 0;   /* put it back to idle. */
		wake_up(&vpe_ctrl->wait);
	}
}
DECLARE_TASKLET(vpe_standalone_tasklet, vpe_do_tasklet, 0);

static irqreturn_t vpe_parse_irq(int irq_num, void *data)
{
	unsigned long flags;
	uint32_t irq_status = 0;
	struct vpe_isr_queue_cmd_type *qcmd;

	CDBG("vpe_parse_irq.\n");
	/* read and clear back-to-back. */
	irq_status = msm_io_r_mb(vpe_device->vpebase +
							VPE_INTR_STATUS_OFFSET);
	msm_io_w_mb(irq_status, vpe_device->vpebase +
				VPE_INTR_CLEAR_OFFSET);

	msm_io_w(0, vpe_device->vpebase + VPE_INTR_ENABLE_OFFSET);

	if (irq_status == 0) {
		printk(KERN_ERR "vpe_parse_irq: irq_status = 0"
						"!!!! Something is wrong!\n");
		return IRQ_HANDLED;
	}
	irq_status &= 0x1;
	/* apply mask. only interested in bit 0.  */
	if (irq_status) {
		qcmd = kzalloc(sizeof(struct vpe_isr_queue_cmd_type),
			GFP_ATOMIC);
		if (!qcmd) {
			CDBG("vpe_parse_irq: qcmd malloc failed!\n");
			return IRQ_HANDLED;
		}
		/* must be 0x1 now. so in bottom half we don't really
		need to check. */
		qcmd->irq_status = irq_status & 0x1;
		spin_lock_irqsave(&vpe_ctrl->tasklet_lock, flags);
		list_add_tail(&qcmd->list, &vpe_ctrl->tasklet_q);
		spin_unlock_irqrestore(&vpe_ctrl->tasklet_lock, flags);
		tasklet_schedule(&vpe_standalone_tasklet);
	}
	return IRQ_HANDLED;
}

static int vpe_enable_irq(void)
{
	uint32_t   rc = 0;
	rc = request_irq(vpe_device->vpeirq,
				vpe_parse_irq,
				IRQF_TRIGGER_HIGH, "vpe", 0);
	return rc;
}

static int msm_vpe_open(struct inode *inode, struct file *filep)
{
	int rc = 0;

	CDBG("%s: In\n", __func__);

	vpe_ctrl = kzalloc(sizeof(struct vpe_ctrl_type), GFP_KERNEL);
	if (!vpe_ctrl) {
		pr_err("%s: no memory!\n", __func__);
		return -ENOMEM;
	}
	/* don't change the order of clock and irq.*/
	CDBG("%s: enable_clock\n", __func__);
	rc = msm_camio_vpe_clk_enable(VPE_TURBO_MODE_CLOCK_RATE);

	CDBG("%s: enable_irq\n", __func__);
	vpe_enable_irq();

	/* initialize the data structure - lock, queue etc. */
	spin_lock_init(&vpe_ctrl->tasklet_lock);
	INIT_LIST_HEAD(&vpe_ctrl->tasklet_q);
	INIT_LIST_HEAD(&vpe_ctrl->pmem_buf);

	CDBG("%s: Out\n", __func__);

	return rc;
}

static int msm_vpe_release(struct inode *node, struct file *filep)
{
	/* clean up....*/
	/* drain the queue, etc. */
	/* don't change the order of clock and irq.*/
	int rc = 0;

	CDBG("%s: In\n", __func__);

	free_irq(vpe_device->vpeirq, 0);
	rc = msm_camio_vpe_clk_disable();
	kfree(vpe_ctrl);

	CDBG("%s: Out\n", __func__);
	return rc;
}


static long msm_vpe_ioctl(struct file *filep,
			  unsigned int cmd,
			  unsigned long arg)
{
	int rc = -EINVAL;
	void __user *argp = (void __user *)arg;
	CDBG("%s: cmd %d\n", __func__, _IOC_NR(cmd));
	switch (cmd) {
	case MSM_VPE_IOCTL_CONFIG_VPE:
	{
		struct msm_vpe_cfg_cmd cfgcmd;
		if (copy_from_user(&cfgcmd, argp, sizeof(cfgcmd))) {
			ERR_COPY_FROM_USER();
			return -EFAULT;
		}
		CDBG("%s: cmd_type %d\n", __func__, cfgcmd.cmd_type);
		switch (cfgcmd.cmd_type) {
		case CMD_VPE:
			rc = msm_vpe_config(&cfgcmd, NULL);
			break;
		default:
			pr_err("%s: unknown command type %d\n",
				__func__, cfgcmd.cmd_type);
		}
		break;
	}
	case MSM_VPE_IOCTL_VPE_TRANSFER:
	{
		struct msm_vpe_transfer_cfg transfercmd;
		if (copy_from_user(&transfercmd, argp, sizeof(transfercmd))) {
			ERR_COPY_FROM_USER();
			return -EFAULT;
		}
		rc = msm_vpe_start_transfer(&transfercmd, NULL);
		break;
	}
	case MSM_VPE_IOCTL_VPE_REGISTER:
	{
	    struct msm_vpe_register_cfg registercmd;
		if (copy_from_user(&registercmd, argp, sizeof(registercmd))) {
			ERR_COPY_FROM_USER();
			return -EFAULT;
		}
		rc = msm_vpe_pmem_register(&registercmd, NULL);
	    break;
	}
	case MSM_VPE_IOCTL_VPE_UNREGISTER:
	{
		struct msm_vpe_unregister_cfg unregistercmd;
		if (copy_from_user(&unregistercmd,
				   argp,
				   sizeof(unregistercmd))) {
			ERR_COPY_FROM_USER();
			return -EFAULT;
		}
		rc = msm_vpe_pmem_unregister(&unregistercmd, NULL);
	    break;
	}
	}
	CDBG("%s: cmd %d Done\n", __func__, _IOC_NR(cmd));
	return rc;
}
static const struct file_operations msm_vpe_fops = {
	.owner		= THIS_MODULE,
	.open		= msm_vpe_open,
	.release	= msm_vpe_release,
	.unlocked_ioctl	= msm_vpe_ioctl,
};

static int semc_vpe_probe(struct platform_device *pdev)
{
	int rc = -1;
	struct resource *vpemem, *vpeirq, *vpeio;
	struct device *dev;
	void __iomem *vpebase;

	/* first allocate */

	vpe_device = &vpe_device_data;
	memset(vpe_device, 0, sizeof(struct msm_vpe_device));

	vpe_device->pdev = pdev;

	/* does the device exist? */
	vpeirq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!vpeirq) {
		pr_err("%s: no vpe irq resource.\n", __func__);
		rc = -ENODEV;
		goto vpe_free_device;
	}
	vpemem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!vpemem) {
		pr_err("%s: no vpe mem resource!\n", __func__);
		rc = -ENODEV;
		goto vpe_free_device;
	}
	vpeio = request_mem_region(vpemem->start,
			resource_size(vpemem), pdev->name);
	if (!vpeio) {
		pr_err("%s: VPE region already claimed.\n", __func__);
		rc = -EBUSY;
		goto vpe_free_device;
	}

	vpebase =
		ioremap(vpemem->start,
				(vpemem->end - vpemem->start) + 1);
	if (!vpebase) {
		pr_err("%s: vpe ioremap failed.\n", __func__);
		rc = -ENOMEM;
		goto vpe_release_mem_region;
	}

	/* Fall through, _probe is successful. */
	vpe_device->vpeirq = vpeirq->start;
	vpe_device->vpemem = vpemem;
	vpe_device->vpeio = vpeio;
	vpe_device->vpebase = vpebase;

	/* setting chrdev */
	rc = alloc_chrdev_region(&msm_vpe_devno, 0, 1, MSM_VPE_NAME);
	if (rc < 0)
		goto vpe_iounmap;

	if (!msm_vpe_class) {
		msm_vpe_class = class_create(THIS_MODULE, MSM_VPE_NAME);
		if (IS_ERR(msm_vpe_class)) {
			rc = PTR_ERR(msm_vpe_class);
			goto vpe_unregister_chrdev;
		}
	}

	dev = device_create(msm_vpe_class, NULL,
			MKDEV(MAJOR(msm_vpe_devno), MINOR(msm_vpe_devno)), NULL,
			"%s", MSM_VPE_NAME);
	if (IS_ERR(dev)) {
		rc = -ENODEV;
		goto vpe_class_destroy;
	}

	cdev_init(&vpe_device->cdev, &msm_vpe_fops);
	vpe_device->cdev.owner = THIS_MODULE;

	rc = cdev_add(&vpe_device->cdev, msm_vpe_devno, 1);
	if (rc < 0) {
		rc = -ENODEV;
		goto vpe_device_destroy;
	}

	return rc;  /* this rc should be zero.*/

	/* cdev_del(&vpe_device->cdev); */ /* this path should never occur */

/* from this part it is error handling. */
vpe_device_destroy:
	device_destroy(msm_vpe_class, msm_vpe_devno);
vpe_class_destroy:
	class_destroy(msm_vpe_class);
vpe_unregister_chrdev:
	unregister_chrdev_region(msm_vpe_devno, 1);
vpe_iounmap:
	iounmap(vpe_device->vpebase);
vpe_release_mem_region:
	release_mem_region(vpemem->start, (vpemem->end - vpemem->start) + 1);
vpe_free_device:
	return rc;  /* this rc should have error code. */
}

static int semc_vpe_remove(struct platform_device *pdev)
{
	struct resource *vpemem;
	vpemem = vpe_device->vpemem;

	cdev_del(&vpe_device->cdev);
	device_destroy(msm_vpe_class, msm_vpe_devno);
	class_destroy(msm_vpe_class);
	unregister_chrdev_region(msm_vpe_devno, 1);

	iounmap(vpe_device->vpebase);
	release_mem_region(vpemem->start,
			   (vpemem->end - vpemem->start) + 1);
	return 0;
}

static struct platform_driver semc_vpe_driver = {
	.probe	= semc_vpe_probe,
	.remove	= semc_vpe_remove,
	.driver	= {
		.name = "semc_vpe1",
		.owner = THIS_MODULE,
	},
};

static int __init semc_vpe_init(void)
{
	return platform_driver_register(&semc_vpe_driver);
}

module_init(semc_vpe_init);

static void __exit semc_vpe_exit(void)
{
	platform_driver_unregister(&semc_vpe_driver);
}

module_exit(semc_vpe_exit);

MODULE_DESCRIPTION("SEMC VPE driver");
MODULE_LICENSE("GPL v2");
