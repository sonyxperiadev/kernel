/* drivers/media/video/msm/semc_vpe.c
 *
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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
#include <linux/slab.h>
#include <linux/pm_qos_params.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#include <linux/ion.h>
#else
#include <linux/android_pmem.h>
#endif
#include <linux/clk.h>
#include <mach/clk.h>
#include <asm/div64.h>
#include "msm.h"
#include "semc_vpe.h"

#define MSM_VPE_NAME "msm_vpe_standalone"

static int vpe_update_scaler(struct msm_pp_crop *pcrop);
static struct class *msm_vpe_class;
static dev_t msm_vpe_devno;
static struct vpe_ctrl_type *vpe_ctrl;

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_client *vpe_client;
#endif

#define ERR_COPY_FROM_USER() ERR_USER_COPY(0)

#define CHECKED_COPY_FROM_USER(in) {				\
	if (copy_from_user((in), (void __user *)cmd->value,	\
			cmd->length)) {				\
		rc = -EFAULT;					\
		break;						\
	}							\
}

static long long vpe_do_div(long long num, long long den)
{
	do_div(num, den);
	return num;
}

static int vpe_start(void)
{
	/*  enable the frame irq, bit 0 = Display list 0 ROI done */
	msm_io_w_mb(1, vpe_ctrl->vpebase + VPE_INTR_ENABLE_OFFSET);
	msm_io_dump(vpe_ctrl->vpebase, 0x120);
	msm_io_dump(vpe_ctrl->vpebase + 0x10000, 0x250);
	msm_io_dump(vpe_ctrl->vpebase + 0x30000, 0x20);
	msm_io_dump(vpe_ctrl->vpebase + 0x50000, 0x30);
	msm_io_dump(vpe_ctrl->vpebase + 0x50400, 0x10);
	/* this triggers the operation. */
	msm_io_w(1, vpe_ctrl->vpebase + VPE_DL0_START_OFFSET);
	wmb();
	return 0;
}

static void vpe_reset_state_variables(void)
{
	/* initialize local variables for state control, etc.*/
	vpe_ctrl->op_mode = 0;
	vpe_ctrl->state = VPE_STATE_IDLE;
	spin_lock_init(&vpe_ctrl->lock);
	init_waitqueue_head(&vpe_ctrl->wait);
}

static void vpe_config_axi_default(void)
{
	msm_io_w(0x25, vpe_ctrl->vpebase + VPE_AXI_ARB_2_OFFSET);
	CDBG("%s: yaddr %ld cbcraddr %ld", __func__,
		 vpe_ctrl->out_y_addr, vpe_ctrl->out_cbcr_addr);
	if (!vpe_ctrl->out_y_addr || !vpe_ctrl->out_cbcr_addr)
		return;
	msm_io_w(vpe_ctrl->out_y_addr,
		vpe_ctrl->vpebase + VPE_OUTP0_ADDR_OFFSET);
	/* for video  CbCr address */
	msm_io_w(vpe_ctrl->out_cbcr_addr,
		vpe_ctrl->vpebase + VPE_OUTP1_ADDR_OFFSET);

}

static int vpe_reset(void)
{
	uint32_t vpe_version;
	uint32_t rc = 0;

	vpe_reset_state_variables();
	vpe_version = msm_io_r(vpe_ctrl->vpebase + VPE_HW_VERSION_OFFSET);
	CDBG("vpe_version = 0x%x\n", vpe_version);

	/* disable all interrupts.*/
	msm_io_w(0, vpe_ctrl->vpebase + VPE_INTR_ENABLE_OFFSET);
	/* clear all pending interrupts*/
	msm_io_w(0x1fffff, vpe_ctrl->vpebase + VPE_INTR_CLEAR_OFFSET);
	/* write sw_reset to reset the core. */
	msm_io_w(0x10, vpe_ctrl->vpebase + VPE_SW_RESET_OFFSET);
	/* then poll the reset bit, it should be self-cleared. */
	while (1) {
		rc =
		msm_io_r(vpe_ctrl->vpebase + VPE_SW_RESET_OFFSET) & 0x10;
		if (rc == 0)
			break;
	}

	/*  at this point, hardware is reset. Then pogram to default
		values. */
	msm_io_w(VPE_AXI_RD_ARB_CONFIG_VALUE,
			vpe_ctrl->vpebase + VPE_AXI_RD_ARB_CONFIG_OFFSET);

	msm_io_w(VPE_CGC_ENABLE_VALUE,
			vpe_ctrl->vpebase + VPE_CGC_EN_OFFSET);
	msm_io_w(1, vpe_ctrl->vpebase + VPE_CMD_MODE_OFFSET);
	msm_io_w(VPE_DEFAULT_OP_MODE_VALUE,
			vpe_ctrl->vpebase + VPE_OP_MODE_OFFSET);
	msm_io_w(VPE_DEFAULT_SCALE_CONFIG,
			vpe_ctrl->vpebase + VPE_SCALE_CONFIG_OFFSET);
	vpe_config_axi_default();
	return rc;
}

static int msm_vpe_cfg_update(void *pinfo)
{
	uint32_t  rot_flag, rc = 0;
	struct msm_pp_crop *pcrop = (struct msm_pp_crop *)pinfo;

	rot_flag = msm_io_r(vpe_ctrl->vpebase +
						VPE_OP_MODE_OFFSET) & 0xE00;
	if (pinfo != NULL) {
		CDBG("%s: Crop info in2_w = %d, in2_h = %d "
			"out2_w = %d out2_h = %d\n",
			__func__, pcrop->src_w, pcrop->src_h,
			pcrop->dst_w, pcrop->dst_h);
		rc = vpe_update_scaler(pcrop);
	}
	CDBG("return rc = %d rot_flag = %d\n", rc, rot_flag);
	rc |= rot_flag;

	return rc;
}

static void vpe_update_scale_coef(uint32_t *p)
{
	uint32_t i, offset;
	offset = *p;
	for (i = offset; i < (VPE_SCALE_COEFF_NUM + offset); i++) {
		msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SCALE_COEFF_LSBn(i));
		msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SCALE_COEFF_MSBn(i));
	}
}

static void vpe_input_plane_config(uint32_t *p)
{
	msm_io_w(*p, vpe_ctrl->vpebase + VPE_SRC_FORMAT_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SRC_UNPACK_PATTERN1_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SRC_IMAGE_SIZE_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SRC_YSTRIDE1_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SRC_SIZE_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_SRC_XY_OFFSET);
}

static void vpe_output_plane_config(uint32_t *p)
{
	msm_io_w(*p, vpe_ctrl->vpebase + VPE_OUT_FORMAT_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_OUT_PACK_PATTERN1_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_OUT_YSTRIDE1_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_OUT_SIZE_OFFSET);
	msm_io_w(*(++p), vpe_ctrl->vpebase + VPE_OUT_XY_OFFSET);
}

static int vpe_operation_config(uint32_t *p)
{
	uint32_t w, h, temp;
	msm_io_w(*p, vpe_ctrl->vpebase + VPE_OP_MODE_OFFSET);

	temp = msm_io_r(vpe_ctrl->vpebase + VPE_OUT_SIZE_OFFSET);
	w = temp & 0xFFF;
	h = (temp & 0xFFF0000) >> 16;
	if (*p++ & 0xE00) {
		/* rotation enabled. */
		vpe_ctrl->out_w = h;
		vpe_ctrl->out_h = w;
	} else {
		vpe_ctrl->out_w = w;
		vpe_ctrl->out_h = h;
	}
	CDBG("%s: out_w=%d, out_h=%d", __func__, vpe_ctrl->out_w,
		vpe_ctrl->out_h);
	return 0;
}

/* Later we can separate the rotation and scaler calc. If
*  rotation is enabled, simply swap the destination dimension.
*  And then pass the already swapped output size to this
*  function. */
static int vpe_update_scaler(struct msm_pp_crop *pcrop)
{
	uint32_t out_ROI_width, out_ROI_height;
	uint32_t src_ROI_width, src_ROI_height;

	/*
	* phase_step_x, phase_step_y, phase_init_x and phase_init_y
	* are represented in fixed-point, unsigned 3.29 format
	*/
	uint32_t phase_step_x = 0;
	uint32_t phase_step_y = 0;
	uint32_t phase_init_x = 0;
	uint32_t phase_init_y = 0;

	uint32_t src_roi, src_x, src_y, src_xy, temp;
	uint32_t yscale_filter_sel, xscale_filter_sel;
	uint32_t scale_unit_sel_x, scale_unit_sel_y;
	uint64_t numerator, denominator;

	/* assumption is both direction need zoom. this can be
	improved. */
	temp =
		msm_io_r(vpe_ctrl->vpebase + VPE_OP_MODE_OFFSET) | 0x3;
	msm_io_w(temp, vpe_ctrl->vpebase + VPE_OP_MODE_OFFSET);

	src_ROI_width = pcrop->src_w;
	src_ROI_height = pcrop->src_h;
	out_ROI_width = pcrop->dst_w;
	out_ROI_height = pcrop->dst_h;

	CDBG("src w = 0x%x, h=0x%x, dst w = 0x%x, h =0x%x.\n",
		src_ROI_width, src_ROI_height, out_ROI_width,
		out_ROI_height);
	src_roi = (src_ROI_height << 16) + src_ROI_width;

	msm_io_w(src_roi, vpe_ctrl->vpebase + VPE_SRC_SIZE_OFFSET);

	src_x = pcrop->src_x;
	src_y = pcrop->src_y;

	CDBG("src_x = %d, src_y=%d.\n", src_x, src_y);

	src_xy = src_y*(1<<16) + src_x;
	msm_io_w(src_xy, vpe_ctrl->vpebase +
			VPE_SRC_XY_OFFSET);
	CDBG("src_xy = %d, src_roi=%d.\n", src_xy, src_roi);

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
	if (out_ROI_width == 1)
		phase_step_x = (uint32_t) ((src_ROI_width) <<
						SCALER_PHASE_BITS);

		/* if using FIR scalar */
	else if (scale_unit_sel_x == 0) {

		/* Calculate the quotient ( src_ROI_width - 1 )
			( out_ROI_width - 1)
			with u3.29 precision. Quotient is rounded up to
			the larger 29th decimal point*/
		numerator = (uint64_t)(src_ROI_width - 1) <<
			SCALER_PHASE_BITS;
		/* never equals to 0 because of the
			"(out_ROI_width == 1 )"*/
		denominator = (uint64_t)(out_ROI_width - 1);
		/* divide and round up to the larger 29th
			decimal point.*/
		phase_step_x = (uint32_t) vpe_do_div((numerator +
					denominator - 1), denominator);
	} else if (scale_unit_sel_x == 1) { /* if M/N scalar */
		/* Calculate the quotient ( src_ROI_width ) /
			( out_ROI_width)
			with u3.29 precision. Quotient is rounded down to the
			smaller 29th decimal point.*/
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
	if (out_ROI_height == 1)
		phase_step_y =
		(uint32_t) ((src_ROI_height) << SCALER_PHASE_BITS);

	/* if FIR scalar */
	else if (scale_unit_sel_y == 0) {
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
			( out_ROI_height)
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
		if (out_ROI_width == 1)
			phase_init_x =
				(uint32_t) ((src_ROI_width - 1) <<
							SCALER_PHASE_BITS);
		else
			phase_init_x = 0;
	} else if (scale_unit_sel_x == 1) /* M over N scalar  */
		phase_init_x = 0;

	/* calculate phase init for the y direction
	if using FIR scalar */
	if (scale_unit_sel_y == 0) {
		if (out_ROI_height == 1)
			phase_init_y =
			(uint32_t) ((src_ROI_height -
						1) << SCALER_PHASE_BITS);
		else
			phase_init_y = 0;
	} else if (scale_unit_sel_y == 1) /* M over N scalar   */
		phase_init_y = 0;

	CDBG("phase step x = %d, step y = %d.\n",
		 phase_step_x, phase_step_y);
	CDBG("phase init x = %d, init y = %d.\n",
		 phase_init_x, phase_init_y);

	msm_io_w(phase_step_x, vpe_ctrl->vpebase +
			VPE_SCALE_PHASEX_STEP_OFFSET);
	msm_io_w(phase_step_y, vpe_ctrl->vpebase +
			VPE_SCALE_PHASEY_STEP_OFFSET);

	msm_io_w(phase_init_x, vpe_ctrl->vpebase +
			VPE_SCALE_PHASEX_INIT_OFFSET);

	msm_io_w(phase_init_y, vpe_ctrl->vpebase +
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
	case VPE_CMD_RESET:
		rc = vpe_reset();
		break;

	case VFE_CMD_START:
		rc = vpe_start();
		break;

	case VPE_CMD_INPUT_PLANE_CFG:
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

	case VPE_CMD_OPERATION_MODE_CFG:
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

	case VPE_CMD_OUTPUT_PLANE_CFG:
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

	case VPE_CMD_SCALE_CFG_TYPE:
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
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_OUT_XY_OFFSET);
	/* for dst  Y address */
	p1 = (paddr + y_off);
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_OUTP0_ADDR_OFFSET);
	/* for dst  CbCr address */
	p1 = (paddr + cbcr_off);
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_OUTP1_ADDR_OFFSET);

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
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_SRC_XY_OFFSET);
	/* for src  Y address */
	p1 = (paddr + y_off);
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_SRCP0_ADDR_OFFSET);
	/* for src  CbCr address */
	p1 = (paddr + cbcr_off);
	msm_io_w(p1, vpe_ctrl->vpebase + VPE_SRCP1_ADDR_OFFSET);

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
	unsigned long flags;
	struct msm_vpe_pmem_region *src_region;
	struct msm_vpe_pmem_region *dst_region;
	unsigned long src_paddr;
	unsigned long dst_paddr;
	struct msm_pmem_info *src_info;
	struct msm_pmem_info *dst_info;
	struct msm_vpe_crop_info *src_crop;
	struct msm_vpe_crop_info *dst_crop;
	struct msm_pp_crop crop_info;
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	struct ion_handle *src_ionhandle;
	struct ion_handle *dst_ionhandle;
#else
	unsigned long src_kvstart;
	struct file *src_file;
	unsigned long dst_kvstart;
	struct file *dst_file;
#endif
	unsigned long src_len;
	unsigned long dst_len;
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
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		src_ionhandle = ion_import_fd(vpe_client, src_info->fd);
		if (IS_ERR_OR_NULL(src_ionhandle))
			return 0;
		rc = ion_phys(vpe_client,
			      src_ionhandle,
			      &src_paddr,
			      (size_t *)&src_len);
#else
		rc = get_pmem_file(src_info->fd,
				   &src_paddr,
				   &src_kvstart,
				   &src_len,
				   &src_file);
#endif
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
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		dst_ionhandle = ion_import_fd(vpe_client, dst_info->fd);
		if (IS_ERR_OR_NULL(src_ionhandle))
			return 0;
		rc = ion_phys(vpe_client,
			      dst_ionhandle,
			      &dst_paddr,
			      (size_t *)&dst_len);
#else
		rc = get_pmem_file(dst_info->fd,
				   &dst_paddr,
				   &dst_kvstart,
				   &dst_len,
				   &dst_file);
#endif
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

	crop_info.src_w = transfercmd->src_crop.w;
	crop_info.src_h = transfercmd->src_crop.h;
	crop_info.dst_w = transfercmd->dst_crop.w;
	crop_info.dst_h = transfercmd->dst_crop.h;
	crop_info.src_x = (transfercmd->dst_crop.w - transfercmd->src_crop.w)/2;
	crop_info.src_y = (transfercmd->dst_crop.h - transfercmd->src_crop.h)/2;
	CDBG("Crop info src_w = %d, src_h = %d "
		"dst_w = %d dst_h = %d\n",
		crop_info.src_w,
		crop_info.src_h,
		crop_info.dst_w,
		crop_info.dst_h);

	msm_vpe_cfg_update(&crop_info);

	spin_lock_irqsave(&vpe_ctrl->lock, flags);
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
	vpe_ctrl->state = VPE_STATE_ACTIVE;
	spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
	vpe_start();

	/* wait vpe end */
	/* TODO: if we need asynchornous process ,
	   the waiting function move to GET_PICTURE ioctl */
	CDBG("Waiting for vpe end\n");
	rc = wait_event_interruptible_timeout(
		vpe_ctrl->wait,
		vpe_ctrl->state != VPE_STATE_ACTIVE,
		msecs_to_jiffies(timeout));
	CDBG("Waiting over for vpe end\n");

	if (put_pmem) {
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		ion_free(vpe_client, dst_ionhandle);
		ion_free(vpe_client, src_ionhandle);
#else
		put_pmem_file(dst_file);
		put_pmem_file(src_file);
#endif
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
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	struct ion_handle *ionhandle;
#else
	struct file *file;
	unsigned long kvstart;
#endif
	unsigned long paddr;
	unsigned long len;
	struct msm_vpe_pmem_region *region;

	CDBG("=== msm_vpe_pmem_register start ===\n");

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	ionhandle = ion_import_fd(vpe_client, registercmd->inf.fd);
	if (IS_ERR_OR_NULL(ionhandle))
		return 0;
	rc = ion_phys(vpe_client, ionhandle, &paddr, (size_t *)&len);
#else
	rc = get_pmem_file(registercmd->inf.fd, &paddr, &kvstart, &len, &file);
#endif
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
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	region->ion_handle = ionhandle;
#else
	region->file = file;
#endif
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
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
				ion_free(vpe_client, region->ion_handle);
#else
				put_pmem_file(region->file);
#endif
				kfree(region);
				break;
			}
		}
	}
	CDBG("=== msm_vpe_pmem_unregister end rc:%d===\n", rc);
	return rc;
}

static void vpe_send_outmsg(void)
{
	unsigned long flags;
	spin_lock_irqsave(&vpe_ctrl->lock, flags);
	if (vpe_ctrl->state == VPE_STATE_IDLE) {
		pr_err("%s VPE is in IDLE state. Ignore the ack msg", __func__);
		spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
		return;
	}
	vpe_ctrl->state = VPE_STATE_INIT;   /* put it back to idle. */
	spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
	wake_up(&vpe_ctrl->wait);
}

static void vpe_do_tasklet(unsigned long data)
{
	CDBG("%s: irq_status = 0x%x",
		   __func__, vpe_ctrl->irq_status);
	if (vpe_ctrl->irq_status & 0x1)
		vpe_send_outmsg();

}
DECLARE_TASKLET(vpe_standalone_tasklet, vpe_do_tasklet, 0);

static irqreturn_t vpe_parse_irq(int irq_num, void *data)
{
	vpe_ctrl->irq_status = msm_io_r_mb(vpe_ctrl->vpebase +
							VPE_INTR_STATUS_OFFSET);
	msm_io_w_mb(vpe_ctrl->irq_status, vpe_ctrl->vpebase +
				VPE_INTR_CLEAR_OFFSET);

	msm_io_w(0, vpe_ctrl->vpebase + VPE_INTR_ENABLE_OFFSET);
	CDBG("%s: vpe_parse_irq =0x%x.\n", __func__, vpe_ctrl->irq_status);
	tasklet_schedule(&vpe_standalone_tasklet);
	return IRQ_HANDLED;
}


static struct msm_cam_clk_info vpe_clk_info[] = {
	{"vpe_clk", 160000000},
	{"vpe_pclk", -1},
};

int vpe_enable(uint32_t clk_rate)
{
	int rc = 0;
	unsigned long flags = 0;
	CDBG("%s", __func__);
	/* don't change the order of clock and irq.*/
	spin_lock_irqsave(&vpe_ctrl->lock, flags);
	if (vpe_ctrl->state != VPE_STATE_IDLE) {
		pr_err("%s: VPE already enabled", __func__);
		spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
		return 0;
	}
	vpe_ctrl->state = VPE_STATE_INIT;
	spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
	enable_irq(vpe_ctrl->vpeirq->start);
	vpe_ctrl->fs_vpe = regulator_get(NULL, "fs_vpe");
	if (IS_ERR(vpe_ctrl->fs_vpe)) {
		pr_err("%s: Regulator FS_VPE get failed %ld\n", __func__,
			PTR_ERR(vpe_ctrl->fs_vpe));
		vpe_ctrl->fs_vpe = NULL;
		goto vpe_fs_failed;
	} else if (regulator_enable(vpe_ctrl->fs_vpe)) {
		pr_err("%s: Regulator FS_VPE enable failed\n", __func__);
		regulator_put(vpe_ctrl->fs_vpe);
		goto vpe_fs_failed;
	}

	rc = msm_cam_clk_enable(&vpe_ctrl->pdev->dev, vpe_clk_info,
			vpe_ctrl->vpe_clk, ARRAY_SIZE(vpe_clk_info), 1);
	if (rc < 0)
		goto vpe_clk_failed;

	return rc;

vpe_clk_failed:
	regulator_disable(vpe_ctrl->fs_vpe);
	regulator_put(vpe_ctrl->fs_vpe);
	vpe_ctrl->fs_vpe = NULL;
vpe_fs_failed:
	disable_irq(vpe_ctrl->vpeirq->start);
	vpe_ctrl->state = VPE_STATE_IDLE;
	return rc;
}

int vpe_disable(void)
{
	int rc = 0;
	unsigned long flags = 0;
	CDBG("%s", __func__);
	spin_lock_irqsave(&vpe_ctrl->lock, flags);
	if (vpe_ctrl->state == VPE_STATE_IDLE) {
		CDBG("%s: VPE already disabled", __func__);
		spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
		return rc;
	}
	spin_unlock_irqrestore(&vpe_ctrl->lock, flags);

	msm_cam_clk_enable(&vpe_ctrl->pdev->dev, vpe_clk_info,
			vpe_ctrl->vpe_clk, ARRAY_SIZE(vpe_clk_info), 0);

	regulator_disable(vpe_ctrl->fs_vpe);
	regulator_put(vpe_ctrl->fs_vpe);
	vpe_ctrl->fs_vpe = NULL;
	disable_irq(vpe_ctrl->vpeirq->start);
	tasklet_kill(&vpe_standalone_tasklet);
	spin_lock_irqsave(&vpe_ctrl->lock, flags);
	vpe_ctrl->state = VPE_STATE_IDLE;
	spin_unlock_irqrestore(&vpe_ctrl->lock, flags);
	return rc;
}

static int msm_vpe_open(struct inode *inode, struct file *filep)
{
	int rc = 0;

	CDBG("%s: In\n", __func__);

	vpe_reset_state_variables();

	CDBG("%s: vpe_enable\n", __func__);
	vpe_enable(VPE_TURBO_MODE_CLOCK_RATE);

	/* initialize the data structure - lock, queue etc. */
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

	vpe_disable();

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
	int rc = 0;
	struct device *dev;

	CDBG("%s: device id = %d\n", __func__, pdev->id);
	vpe_ctrl = kzalloc(sizeof(struct vpe_ctrl_type), GFP_KERNEL);
	if (!vpe_ctrl) {
		pr_err("%s: no enough memory\n", __func__);
		return -ENOMEM;
	}

	vpe_ctrl->vpemem = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "semc_vpe.0");
	if (!vpe_ctrl->vpemem) {
		pr_err("%s: no mem resource?\n", __func__);
		rc = -ENODEV;
		goto vpe_no_resource;
	}

	vpe_ctrl->vpeirq = platform_get_resource_byname(pdev,
					IORESOURCE_IRQ, "semc_vpe.0");
	if (!vpe_ctrl->vpeirq) {
		pr_err("%s: no irq resource?\n", __func__);
		rc = -ENODEV;
		goto vpe_no_resource;
	}

	vpe_ctrl->vpeio = request_mem_region(vpe_ctrl->vpemem->start,
		resource_size(vpe_ctrl->vpemem), pdev->name);
	if (!vpe_ctrl->vpeio) {
		pr_err("%s: no valid mem region\n", __func__);
		rc = -EBUSY;
		goto vpe_no_resource;
	}

	vpe_ctrl->vpebase = ioremap(vpe_ctrl->vpemem->start,
		resource_size(vpe_ctrl->vpemem));
	if (!vpe_ctrl->vpebase) {
		rc = -ENOMEM;
		pr_err("%s: vpe ioremap failed\n", __func__);
		goto vpe_release_mem_region;
	}

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

	cdev_init(&vpe_ctrl->cdev, &msm_vpe_fops);
	vpe_ctrl->cdev.owner = THIS_MODULE;

	rc = cdev_add(&vpe_ctrl->cdev, msm_vpe_devno, 1);
	if (rc < 0) {
		rc = -ENODEV;
		goto vpe_device_destroy;
	}

	rc = request_irq(vpe_ctrl->vpeirq->start, vpe_parse_irq,
		IRQF_TRIGGER_HIGH, "semc_vpe.0", 0);
	if (rc < 0) {
		release_mem_region(vpe_ctrl->vpemem->start,
			resource_size(vpe_ctrl->vpemem));
		pr_err("%s: irq request fail:%d\n", __func__, rc);
		rc = -EBUSY;
		goto vpe_no_resource;
	}

	disable_irq(vpe_ctrl->vpeirq->start);

	vpe_ctrl->pdev = pdev;

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	vpe_client = msm_ion_client_create(-1, "msm_vpe_standalone");
#endif

	return 0;

vpe_device_destroy:
	device_destroy(msm_vpe_class, msm_vpe_devno);
vpe_class_destroy:
	class_destroy(msm_vpe_class);
vpe_unregister_chrdev:
	unregister_chrdev_region(msm_vpe_devno, 1);
vpe_iounmap:
	iounmap(vpe_ctrl->vpebase);
vpe_release_mem_region:
	release_mem_region(vpe_ctrl->vpemem->start,
		(vpe_ctrl->vpemem->end - vpe_ctrl->vpemem->start) + 1);
vpe_no_resource:
	kfree(vpe_ctrl);
	return 0;
}

static int semc_vpe_remove(struct platform_device *pdev)
{
	struct resource *vpemem;
	vpemem = vpe_ctrl->vpemem;

	cdev_del(&vpe_ctrl->cdev);
	device_destroy(msm_vpe_class, msm_vpe_devno);
	class_destroy(msm_vpe_class);
	unregister_chrdev_region(msm_vpe_devno, 1);

	iounmap(vpe_ctrl->vpebase);
	release_mem_region(vpemem->start,
			   (vpemem->end - vpemem->start) + 1);

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	ion_client_destroy(vpe_client);
#endif

	return 0;
}

static struct platform_driver semc_vpe_driver = {
	.probe	= semc_vpe_probe,
	.remove	= semc_vpe_remove,
	.driver	= {
		.name = "semc_vpe",
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
