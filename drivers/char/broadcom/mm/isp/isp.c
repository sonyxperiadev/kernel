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

#include "mm_isp.h"
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_isp.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>
#ifdef CONFIG_ION
#include <linux/broadcom/kona_ion.h>
#endif

#define ISP_HW_SIZE (1024*512)
#define IRQ_ISP         BCM_INT_ID_RESERVED153


struct isp_device_t {
	void *vaddr;
	void *fmwk_handle;
};

#define isp_write(reg, value) mm_write_reg(isp->vaddr, reg, value)
#define isp_read(reg) mm_read_reg(isp->vaddr, reg)
#define isp_clr_bit32(reg, bits) mm_clr_bit32(isp->vaddr, reg, bits)
#define isp_write_bit32(reg, bits) mm_write_bit32(isp->vaddr, reg, bits)

void printispregs(struct isp_device_t *isp)
{
	pr_err("ISP_STATUS = 0x%lx\n",
		(unsigned long)(isp_read(ISP_STATUS_OFFSET)));
	pr_err("ISP_TILE_STATUS = 0x%lx\n",
		(unsigned long) (isp_read(ISP_TILE_STATUS_OFFSET)));
	pr_err("ispid = 0x%lx\n",
		(unsigned long)(isp_read(ISP_ID_OFFSET)));
	pr_err("ISP_TILE_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISP_TILE_CTRL_OFFSET)));
	pr_err("ISP_TILE_ADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISP_TILE_ADDR_OFFSET)));
	pr_err("ISPFR_SIZE = 0x%lx\n",
		(unsigned long)(isp_read(ISPFR_SIZE_OFFSET)));
	pr_err("ISPFR_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISPFR_CTRL_OFFSET)));
	pr_err("ISPFR_BAYER_EN = 0x%lx\n",
		(unsigned long)(isp_read(ISPFR_BAYER_EN_OFFSET)));
	pr_err("ISPFR_YCBCR_EN = 0x%lx\n",
		(unsigned long)(isp_read(ISPFR_YCBCR_EN_OFFSET)));
	pr_err("ISPLO_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_CTRL_OFFSET)));
	pr_err("ISPLO_ADDRESS1 = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_ADDRESS1_OFFSET)));
	pr_err("ISPLO_ADDRESS2 = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_ADDRESS2_OFFSET)));
	pr_err("ISPLO_ADDRESS3 = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_ADDRESS3_OFFSET)));
	pr_err("ISPLO_STRIDE1 = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_STRIDE1_OFFSET)));
	pr_err("ISPLO_STRIDE2 = 0x%lx\n",
		(unsigned long)(isp_read(ISPLO_STRIDE2_OFFSET)));
	pr_err("ISPYI_CTRL = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_CTRL_OFFSET)));
	pr_err("ISPYI_RY_ADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_RY_ADDR_OFFSET)));
	pr_err("ISPYI_GU_ADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_GU_ADDR_OFFSET)));
	pr_err("ISPYI_BV_ADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_BV_ADDR_OFFSET)));
	pr_err("ISPYI_STRIDE1 = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_STRIDE1_OFFSET)));
	pr_err("ISPYI_STRIDE2 = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_STRIDE2_OFFSET)));
	pr_err("ISPYI_RY_EADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_RY_EADDR_OFFSET)));
	pr_err("ISPYI_GU_EADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_GU_EADDR_OFFSET)));
	pr_err("ISPYI_BV_EADDR = 0x%lx\n",
		(unsigned long)(isp_read(ISPYI_BV_EADDR_OFFSET)));

	return;
}

static mm_isr_type_e  process_isp_irq(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	u32 ispStatus = 0;
	uint32_t ctrl = 0;
	ispStatus = isp_read(ISP_STATUS_OFFSET);
	isp_write(ISP_STATUS_OFFSET, ispStatus);
	ctrl = isp_read(ISP_CTRL_OFFSET);
	pr_debug("ispStatus 0x%x ctrl 0x%x", ispStatus, ctrl);

	if (ispStatus & 0x10) {
		/* end of tile interrupt, disable control reg,
		    as queue head job completed, schedule tasklet again*/
		pr_debug("EOT intr");
		isp_clr_bit32(ISP_CTRL_OFFSET, ISP_CTRL_ENABLE_MASK);
		return MM_ISR_SUCCESS;
	}
	return MM_ISR_UNKNOWN;
}

static bool get_isp_status(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	uint32_t ispStatus = 0;
	ispStatus = isp_read(ISP_STATUS_OFFSET);
	/* Check if hw busy */
	if (ispStatus & 0x1)
		return true;
	return false;
}

int isp_write_frame(struct isp_device_t *isp,
		struct ISP_FRAME_PARAMS_T *pFrameParam)
{

	/* uint32_t mosaic_patterns[] =
		{0xDD22DD22, 0x22DD22DD, 0x88778877, 0x77887788}; */

	uint32_t val = (((uint32_t)pFrameParam->height) << 16) +
			(uint32_t)pFrameParam->width;
	isp_write(ISPFR_SIZE_OFFSET, val);

	/* clear ISPFR_CTRL register */
	isp_write(ISPFR_CTRL_OFFSET, 0);
	isp_clr_bit32(ISPFR_CTRL_OFFSET, ISPFR_CTRL_BAYER_ORDER_MASK);
	isp_clr_bit32(ISPFR_CTRL_OFFSET, ISPFR_CTRL_TRANSPOSED_MASK);
	isp_clr_bit32(ISPFR_CTRL_OFFSET, ISPFR_CTRL_SW_OUT_POS_MASK);
	isp_clr_bit32(ISPFR_CTRL_OFFSET, ISPFR_CTRL_SW_IN_POS_MASK);

	isp_write(ISPFR_BAYER_EN_OFFSET, pFrameParam->bayer_en);
	isp_write(ISPFR_YCBCR_EN_OFFSET, pFrameParam->ycbcr_en);

	/* isp_write( ISPFR_MOSAIC_OFFSET,
			mosaic_patterns[pFrameParam->bayer_order]); */

	return 0;
}

int isp_write_tile_offsets(struct isp_device_t *isp,
			struct ISP_TILE_OFFSET_PARAMS_T *tile_offsets)
{
	isp_clr_bit32(ISPFR_OFF0_OFFSET, ISPFR_OFF0_DM_MASK);
	isp_clr_bit32(ISPFR_OFF0_OFFSET, ISPFR_OFF0_XC_MASK);
	isp_clr_bit32(ISPFR_OFF0_OFFSET, ISPFR_OFF0_RS_MASK);
	isp_clr_bit32(ISPFR_OFF0_OFFSET, ISPFR_OFF0_DP_MASK);
	isp_clr_bit32(ISPFR_OFF1_OFFSET, ISPFR_OFF1_ST_MASK);
	isp_clr_bit32(ISPFR_OFF1_OFFSET, ISPFR_OFF1_DN_MASK);
	isp_clr_bit32(ISPFR_OFF1_OFFSET, ISPFR_OFF1_BL_MASK);
	isp_clr_bit32(ISPFR_OFF1_OFFSET, ISPFR_OFF1_LS_MASK);
	isp_clr_bit32(ISPFR_OFF2_OFFSET, ISPFR_OFF2_HRO_MASK);
	isp_clr_bit32(ISPFR_OFF2_OFFSET, ISPFR_OFF2_FCSH_MASK);
	isp_clr_bit32(ISPFR_OFF2_OFFSET, ISPFR_OFF2_GD_MASK);
	isp_clr_bit32(ISPFR_SWOFF_OFFSET, ISPFR_SWOFF_SW_IN_MASK);
	isp_clr_bit32(ISPFR_SWOFF_OFFSET, ISPFR_SWOFF_SW_OUT_MASK);

	return 0;
}

int isp_write_y_g_coeffs(struct isp_device_t *isp,
			struct ISP_Y_G_COEFFS_PARAMS_T *pYGCoeffsParam)
{
	int ret = 0;
	/* Clear registers */
	isp_write(ISPYG_Y_OFFSET, 0);
	isp_write(ISPYG_OFFSET1_OFFSET, 0);
	isp_write(ISPYG_OFFSET2_OFFSET, 0);
	isp_write(ISPYG_SCALE_OFFSET, 0);

	isp_write_bit32(ISPYG_Y_OFFSET, (SINT4P12(0.114)));
	isp_write_bit32(ISPYG_Y_OFFSET,
			((SINT4P12(0.3))<<ISPYG_Y_COEFF1_SHIFT));
	isp_write_bit32(ISPYG_OFFSET1_OFFSET, 1024);
	isp_write_bit32(ISPYG_OFFSET2_OFFSET, 1024);
	isp_write_bit32(ISPYG_SCALE_OFFSET,
			(UINT4P12(0.59)<<ISPYG_SCALE_U_SHIFT));
	isp_write_bit32(ISPYG_SCALE_OFFSET,
			(UINT4P12(0.7)<<ISPYG_SCALE_V_SHIFT));
	return ret;
}

int isp_write_yuv_input(struct isp_device_t *isp,
			struct ISP_YUV_INPUT_PARAMS_T *pYUVInputParam)
{
	/*Clear ISPYI_CTRL register */
	isp_write(ISPYI_CTRL_OFFSET, 0);

	/* NO need as col_width_isp will be written */
	isp_write_bit32(ISPYI_CTRL_OFFSET,
		(pYUVInputParam->col_width<<ISPYI_CTRL_COL_WIDTH_SHIFT));
	isp_write_bit32(ISPYI_CTRL_OFFSET,
		(pYUVInputParam->col_mode<<ISPYI_CTRL_COL_MODE_SHIFT));
	isp_write_bit32(ISPYI_CTRL_OFFSET,
		(pYUVInputParam->cc_order<<ISPYI_CTRL_CC_ORDER_SHIFT));
	isp_write_bit32(ISPYI_CTRL_OFFSET,
		(pYUVInputParam->data_format<<ISPYI_CTRL_DATA_FORMAT_SHIFT));
	isp_clr_bit32(ISPYI_CTRL_OFFSET, ISPYI_CTRL_BYTES_SAMPLE_MASK);
	isp_write_bit32(ISPYI_CTRL_OFFSET, pYUVInputParam->cs_format);
	isp_write(ISPYI_RY_ADDR_OFFSET, (uint32_t)(pYUVInputParam->ry_address));
	isp_write(ISPYI_GU_ADDR_OFFSET, (uint32_t)(pYUVInputParam->gu_address));
	isp_write(ISPYI_BV_ADDR_OFFSET, (uint32_t)(pYUVInputParam->bv_address));
	isp_write(ISPYI_STRIDE1_OFFSET, pYUVInputParam->y_stride);
	isp_write(ISPYI_STRIDE2_OFFSET, pYUVInputParam->uv_stride);
	isp_write(ISPYI_COL_STRIDE1_OFFSET, pYUVInputParam->col_stride1);
	isp_write(ISPYI_COL_STRIDE2_OFFSET, pYUVInputParam->col_stride2);
	isp_write(ISPYI_RY_EADDR_OFFSET,
			(uint32_t)(pYUVInputParam->ry_end_address));
	isp_write(ISPYI_GU_EADDR_OFFSET,
			(uint32_t)(pYUVInputParam->gu_end_address));
	isp_write(ISPYI_BV_EADDR_OFFSET,
			(uint32_t)(pYUVInputParam->bv_end_address));

	return 0;
}

int isp_write_lresize(struct isp_device_t *isp,
		struct ISP_LRESIZE_PARAMS_T *pLresizeParam)
{
	uint32_t norm;

	/* Clear registers */
	isp_write(ISPLR_TSCALEX_OFFSET, 0);
	isp_write(ISPLR_TSCALEY_OFFSET, 0);

	isp_write_bit32(ISPLR_TSCALEX_OFFSET,
		(pLresizeParam->tscale_x<<ISPLR_TSCALEX_SCALE_FRAC_SHIFT));
	isp_write_bit32(ISPLR_TSCALEY_OFFSET,
		(pLresizeParam->tscale_y<<ISPLR_TSCALEY_SCALE_FRAC_SHIFT));
	isp_write_bit32(ISPLR_TSCALEX_OFFSET,
		((pLresizeParam->tscale_x>>12)<<ISPLR_TSCALEX_SCALE_INT_SHIFT));
	isp_write_bit32(ISPLR_TSCALEY_OFFSET,
		((pLresizeParam->tscale_y>>12)<<ISPLR_TSCALEY_SCALE_INT_SHIFT));

	norm = (pLresizeParam->tnorm_0) | (pLresizeParam->tnorm_1 << 16);
	isp_write(ISPLR_NORM_0_1_OFFSET, norm);
	norm = (pLresizeParam->tnorm_2) | (pLresizeParam->tnorm_3 << 16);
	isp_write(ISPLR_NORM_2_3_OFFSET, norm);

	if (pLresizeParam->rb_swap) {
		isp_write(ISPLR_SHIFT_OFFSET,
			((uint32_t)pLresizeParam->shift | (1<<8)));
	} else
		isp_write(ISPLR_SHIFT_OFFSET, pLresizeParam->shift);

	return 0;
}

int isp_write_lr_shift(struct isp_device_t *isp,
		struct ISP_LRESIZE_PARAMS_T *pLresizeParam)
{
	if (pLresizeParam->rb_swap) {
		isp_write(ISPLR_SHIFT_OFFSET,
			((uint32_t)pLresizeParam->shift | (1<<8)));
	} else
		isp_write(ISPLR_SHIFT_OFFSET, pLresizeParam->shift);
	return 0;
}

int isp_write_lo_res_output(struct isp_device_t *isp,
			struct ISP_OUTPUT_PARAMS_T *pOutputParam)
{
	isp_write(ISPLO_CTRL_OFFSET, 0);

	isp_clr_bit32(ISPLO_CTRL_OFFSET, ISPLO_CTRL_OUTPUT_WIDTH_MASK);
	isp_write_bit32(ISPLO_CTRL_OFFSET,
			((pOutputParam->data_format&0x7) <<
				ISPLO_CTRL_DATA_FORMAT_SHIFT));
	isp_write_bit32(ISPLO_CTRL_OFFSET,
			(((pOutputParam->data_format&0x8)>>3) <<
				ISPLO_CTRL_DATA_FORMAT_BIT_4_SHIFT));
	isp_write_bit32(ISPLO_CTRL_OFFSET,
		(pOutputParam->col_mode<<ISPLO_CTRL_COL_MODE_SHIFT));
	isp_clr_bit32(ISPLO_CTRL_OFFSET, ISPLO_CTRL_VFLIP_MASK);
	isp_write_bit32(ISPLO_CTRL_OFFSET,
		(pOutputParam->shift<<ISPLO_CTRL_SHIFT_SHIFT));
	isp_write_bit32(ISPLO_CTRL_OFFSET,
		(pOutputParam->col_width<<ISPLO_CTRL_COL_WIDTH_SHIFT));
	isp_write(ISPLO_COL_STRIDE1_OFFSET, pOutputParam->col_stride1);
	isp_write(ISPLO_COL_STRIDE2_OFFSET, pOutputParam->col_stride2);
	isp_write(ISPLO_ADDRESS1_OFFSET, (uint32_t)pOutputParam->address1);
	isp_write(ISPLO_ADDRESS2_OFFSET, (uint32_t)pOutputParam->address2);
	isp_write(ISPLO_ADDRESS3_OFFSET, (uint32_t)pOutputParam->address3);
	isp_write(ISPLO_STRIDE1_OFFSET, pOutputParam->stride1);
	isp_write(ISPLO_STRIDE2_OFFSET, pOutputParam->stride2);

	return 0;
}

int isp_write_conversion(struct isp_device_t *isp, uint32_t informat,
				uint32_t outformat)
{
	int ret = 0;
	int i, j;

	struct ISP_CONVERSION_PARAMS_T yuv_conversion;
	int16_t matrix_t[3][3] = {{0x128F, 0, 0x1970},
			{0x128F, 0xF9C3, 0xF30B}, {0x128F, 0x2028, 0} };
	int32_t offsets_t[3] = {-222*8, 150*8, -276*8};

	if ((informat < 4) || (outformat > 3))
		return 0;

	/* Coeff Need to tuned */
	/* yuv_conversion.matrix = {{TRUNC12(1.16), TRUNC12(0),	TRUNC12(1.59)},
			  {TRUNC12(1.16), TRUNC12(-0.39), TRUNC12(-0.81)},
			  {TRUNC12(1.16),  TRUNC12(2.01),	TRUNC12(0)}};*/

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			yuv_conversion.matrix[i][j] = matrix_t[i][j];

	for (i = 0; i < 3; i++)
		yuv_conversion.offsets[i] = offsets_t[i];

	for (i = 0; i < 5; i++)	{
		pr_debug("%s: yuv_conversion.matrix[%d] = 0x%x\n", __func__,
				i, (((uint32_t *)yuv_conversion.matrix)[i]));
		isp_write((ISPYC_MATRIX_OFFSET+(sizeof(uint32_t)*i)),
				(((uint32_t *)yuv_conversion.matrix)[i]));
	}

	for (i = 0; i < 3; i++)	{
		pr_debug("%s: yuv_conversion.offset[%d] = 0x%x", __func__,
				i, ((uint32_t *)yuv_conversion.offsets)[i]);
		isp_write((ISPYC_OFFSET_OFFSET+(sizeof(u32)*i)),
				(((uint32_t *)yuv_conversion.offsets)[i]));
	}

	return ret;

}

int isp_write_hresize(struct isp_device_t *isp,
		struct ISP_HRESIZE_PARAMS_T *pHresizeParam)
{
	isp_write(ISPHR_CTRL_OFFSET, pHresizeParam->interp_filter_mode);
	isp_write_bit32(ISPHR_SCALE_X_OFFSET,
		pHresizeParam->wscalef_x << ISPHR_SCALE_X_SCALE_INT_SHIFT);
	isp_write_bit32(ISPHR_SCALE_X_OFFSET,
		pHresizeParam->fscalef_x << ISPHR_SCALE_X_SCALE_FRAC_SHIFT);
	isp_write_bit32(ISPHR_SCALE_Y_OFFSET,
		pHresizeParam->wscalef_y << ISPHR_SCALE_Y_SCALE_INT_SHIFT);
	isp_write_bit32(ISPHR_SCALE_Y_OFFSET,
		pHresizeParam->fscalef_y << ISPHR_SCALE_Y_SCALE_FRAC_SHIFT);
	isp_write_bit32(ISPHR_NORM_OFFSET,
		pHresizeParam->normf_x << ISPHR_NORM_NORM_X_SHIFT);
	isp_write_bit32(ISPHR_NORM_OFFSET,
		pHresizeParam->normf_y << ISPHR_NORM_NORM_Y_SHIFT);

	return 0;
}


static int isp_reset(void *id)
{
	struct isp_device_t *isp = (struct isp_device_t *)id;
	int ret = 0;
	uint32_t ispStatus = 0;
	uint32_t ispCtrl = 0;
	uint32_t dummy;

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

int isp_clearstat(struct isp_device_t *isp)
{
	int ret = 0;
	isp_write(ISP_CTRL_OFFSET, (1<<ISP_CTRL_CLR_STATS_SHIFT));/* JITENDRA */
	isp_write(ISPBL_SUM_LO_OFFSET, 0);
	isp_write(ISPBL_SUM_HI_OFFSET, 0);
	isp_write(ISPBL_COUNT_OFFSET, 0);
	return ret;
}

int isp_programtiles(struct isp_device_t *isp, struct isp_job_post_t *job_post)
{
	int ret = 0;
	isp_write(ISP_TILE_ADDR_OFFSET, job_post->tile_addr);
	isp_write(ISP_TILE_CTRL_OFFSET,
		(job_post->tile_count<<ISP_TILE_CTRL_TILE_COUNT_SHIFT));
	return ret;
}

int isp_enableint(struct isp_device_t *isp)
{
	int ret = 0;
	isp_clr_bit32(ISP_CTRL_OFFSET, ISP_CTRL_EOT_IMASK_MASK);
	isp_clr_bit32(ISP_CTRL_OFFSET, ISP_CTRL_ERROR_IMASK_MASK);
	return ret;
}

int isp_program(struct isp_device_t *isp, struct isp_job_post_t *job_post)
{
	int ret = 0;
	ret = isp_reset(isp);
	if (ret != 0) {
		pr_err("isp_reset failed with %d", ret);
		return ret;
	}
	ret = isp_write_frame(isp, &job_post->frame);
	if (ret != 0) {
		pr_err("isp_programfrparam failed with %d", ret);
		return ret;
	}
	ret = isp_write_tile_offsets(isp, &job_post->tile_offsets);
	if (ret != 0) {
		pr_err("isp_programtileoff failed with %d", ret);
		return ret;
	}
	ret = isp_write_yuv_input(isp, &job_post->yuv_input);
	if (ret != 0) {
		pr_err("isp_programinput failed with %d", ret);
		return ret;
	}
	ret = isp_write_y_g_coeffs(isp, &job_post->y_g_coeffs);
	if (ret != 0) {
		pr_err("isp_programygcoeff failed with %d", ret);
		return ret;
	}
	ret = isp_write_conversion(isp, job_post->informat,
					job_post->outformat);
	if (ret != 0) {
		pr_err("isp_programccmatrix failed with %d", ret);
		return ret;
	}
	ret = isp_write_lresize(isp, &job_post->lresize);
	if (ret != 0) {
		pr_err("isp_programlresize failed with %d", ret);
		return ret;
	}
	ret = isp_write_lo_res_output(isp, &job_post->lo_res_output);
	if (ret != 0) {
		pr_err("isp_programloresoutput failed with %d", ret);
		return ret;
	}
	ret = isp_write_hresize(isp, &job_post->hresize);
	if (ret != 0) {
		pr_err("isp_programhresize failed with %d", ret);
		return ret;
	}
	ret = isp_clearstat(isp);
	if (ret != 0) {
		pr_err("isp_clearstat failed with %d", ret);
		return ret;
	}
	ret = isp_programtiles(isp, job_post);
	if (ret != 0) {
		pr_err("isp_programtiles failed with %d", ret);
		return ret;
	}
	ret = isp_enableint(isp);
	if (ret != 0) {
		pr_err("isp_enableint failed with %d", ret);
		return ret;
	}

	return ret;
}

int isp_start(struct isp_device_t *isp)
{
	int ret = 0;
	uint32_t ctrl;
	ctrl = isp_read(ISP_CTRL_OFFSET);
	ctrl |= ISP_CTRL_ENABLE_MASK;
	isp_write(ISP_CTRL_OFFSET, ctrl);
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
			pr_debug("Prog the regs and start\n");
			ret = isp_program(isp, job_params);
			if (ret != 0) {
				pr_err("isp_program failed with %d", ret);
				job->status = MM_JOB_STATUS_ERROR;
				return MM_JOB_STATUS_ERROR;
			}
			/* printispregs(isp); */
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
			pr_debug("job completed\n");
			job->status = MM_JOB_STATUS_SUCCESS;
			return MM_JOB_STATUS_SUCCESS;
		}
		break;
	}
	return MM_JOB_STATUS_ERROR;
}

static struct isp_device_t *isp_device;

int __init mm_isp_init(void)
{
	int ret = 0;
	MM_CORE_HW_IFC core_param;
	MM_DVFS_HW_IFC dvfs_param;
	MM_PROF_HW_IFC prof_param;
	isp_device = kmalloc(sizeof(struct isp_device_t), GFP_KERNEL);
	isp_device->vaddr = NULL;
	pr_debug("ISP driver Module Init");

	core_param.mm_base_addr = ISP_BASE_ADDR;
	core_param.mm_hw_size = ISP_HW_SIZE;
	core_param.mm_irq = IRQ_ISP;

	core_param.mm_timer = DEFAULT_MM_DEV_TIMER_MS;
	core_param.mm_timeout = DEFAULT_MM_DEV_TIMEOUT_MS;

	core_param.mm_get_status = get_isp_status;
	core_param.mm_start_job = isp_start_job;
	core_param.mm_process_irq = process_isp_irq;
	core_param.mm_init = isp_reset;
	core_param.mm_deinit = isp_reset;
	core_param.mm_abort = isp_reset;
	core_param.mm_get_regs = NULL;
	core_param.mm_device_id = (void *)isp_device;
	core_param.mm_virt_addr = NULL;

	dvfs_param.is_dvfs_on = 1;
	dvfs_param.user_requested_mode = TURBO;
	dvfs_param.enable_suspend_resume = 0;
	dvfs_param.T1 = 300;
	dvfs_param.P1 = 80;
	dvfs_param.T2 = 3000;
	dvfs_param.P2 = 30;
	dvfs_param.dvfs_bulk_job_cnt = 0;

	isp_device->fmwk_handle = mm_fmwk_register(ISP_DEV_NAME,
					ISP_AXI_BUS_CLK_NAME_STR, 1,
					&core_param, &dvfs_param, &prof_param);
	/* get kva from fmwk */
	isp_device->vaddr = core_param.mm_virt_addr;

	if ((isp_device->fmwk_handle == NULL) ||
		(isp_device->vaddr == NULL)) {
		ret = -ENOMEM;
		goto err;
	}
	pr_debug("ISP driver Module Init over");
	return ret;

err:
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
