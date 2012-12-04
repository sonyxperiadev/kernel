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
#ifndef _ISP_H_
#define _ISP_H_
#include <linux/ioctl.h>

#define ISP_VERSION 460
#define HERA_ISP    1
#define HERA_A0_ISP 0


#include <linux/broadcom/mm_fw_usr_ifc.h>

#define BCM_ISP_MAGIC	'I'

#define SINT4P12(x) ((signed short)(((x) > 0) ? ((x)*4096+0.5) : \
						((x)*4096-0.5)))
#define UINT4P12(x) ((uint16_t)((x)*4096+0.5))

#define TRUNC12(x) (((int)(x*4096))&0xFFFF)

/******************************************************************************
Structures. These define the parameters that can be sent to the ISP or ISP
control software, and correspond directly, for the most part, to the registers
of the functional blocks of the ISP.
******************************************************************************/

/* Number of input lines available.*/

struct ISP_LINES_AVAIL_PARAMS_T {
	uint16_t lines_avail;
	uint8_t pad[2];
};


/*  ISP version parameters. */

struct ISP_VERSION_PARAMS_T {
	uint8_t revision;
	uint8_t major_version;
	uint8_t pad[2];
};


/*  Frame parameters. */

struct ISP_FRAME_PARAMS_T {
	uint16_t width;
	uint16_t height;
	int32_t /*ISP_BAYER_ORDER_T*/ bayer_order;
	int32_t /*ISP_SW_POSITION_T*/ sw_out_pos;
	int32_t /*ISP_SW_POSITION_T*/ sw_in_pos;
	uint32_t bayer_en;
	uint32_t ycbcr_en;
	uint8_t transposed;
	uint8_t pad[3];
#if HERA_ISP
	/* For Hera, we'll munge bayer_en and ycbcr_en to enable only those
	* blocks that exist. The requested enables we'll store here,
	* so that they can be fed back unmunged.
	* Note that the hresize bit will be used for the window/crop function */
	uint32_t bayer_en_public;
	uint32_t ycbcr_en_public;
#endif
};

/* Tile offset parameters. */

struct ISP_TILE_OFFSET_PARAMS_T {
	uint8_t dp; /* defective pixel correction */
	uint8_t rs; /* resampling */
	uint8_t fr; /* fringing correction */
	uint8_t xc; /* crosstalk correction */
	uint8_t dm; /* demosaicing */
	uint8_t ls; /* lens shading correction */
	uint8_t bl; /* black level correction */
	uint8_t dn; /* denoising */
	uint8_t st; /* statistics */
	uint8_t gd; /* geometrical distortion correction */
	uint8_t fcsh; /* false colour/sharpening */
	uint8_t hro; /* hires output */
	uint8_t dummy; /* make bs_out aligned */
	uint8_t sw_out; /* software stage out */
	uint8_t sw_in; /* software stage in */
	uint8_t pad[1];
};

/* Bayer input formatter parameters. */

struct ISP_RAW_INPUT_PARAMS_T {
	void *address;
	uint32_t stride;
	void *end_address;
	int32_t /*ISP_RAW_INPUT_FORMAT_T*/ format;
	uint8_t dpcm_adv;
	uint8_t pad[1];
	uint16_t dpcm_blocklen;
};


/* Coefficients used for converting RGB into YCbCr and vice versa. */

struct ISP_Y_G_COEFFS_PARAMS_T {
	signed short y_coeffs[2]; /* B-G and R-G coefficients to make Y
				from R, G and B */
#if HERA_ISP
	unsigned short cb_scale;
	unsigned short cr_scale;
	uint16_t cb_offset;
	uint16_t cr_offset;
#endif
};

/* YUV input formatter. */

struct ISP_YUV_INPUT_PARAMS_T {
	int32_t /*ISP_CS_FORMAT_T*/ cs_format;
	uint8_t bytes_sample;
	uint8_t pad1[3];
	/* \todo - consider just using ISP_DATA_FORMAT_T for symmetry with the
	* output formatters */
	int32_t /*ISP_INTERLEAVE_T*/ data_format;
	int32_t /*ISP_CC_ORDER_T*/ cc_order;
	void *ry_address;
	void *gu_address;
	void *bv_address;
	uint32_t y_stride;
	uint32_t uv_stride;
	uint8_t col_mode;
	uint8_t col_width;   /* 00-32 pix, 01-64, 10-128, 11-256 */
	uint8_t pad2[2];
	uint32_t col_stride1;
	uint32_t col_stride2;
	void *ry_end_address;
	void *gu_end_address;
	void *bv_end_address;
};

/* Colour conversion parameters. */

struct ISP_CONVERSION_PARAMS_T {
	int16_t matrix[3][3];  /* actually agnostic about the fixed/fractional
				* parts here */
	uint8_t pad[2];
	int32_t offsets[3];
};

/* High resolution resize parameters. */

struct ISP_HRESIZE_PARAMS_T {
	/* Fields starting with an underscore do not actually have registers in
	* the hardware, so should not be used in the software simulator. */
	uint16_t _input_offset_x;	/* LH edge of crop window */
	uint16_t _input_offset_y;	/* top edge of crop window */
	uint16_t _input_width;		/* width of input window */
	uint16_t _input_height;		/* height of input window */
	int32_t /*ISP_INTERP_FILTER_T*/ interp_filter_mode; /* type of resize,
							* e.g. bilinear etc. */
	uint16_t wscalef_x;		/* Whole part of the scaling factor
						in the horizontal direction */
	uint16_t fscalef_x;		/* Fractional part of the scaling factor
						 in the horizontal direction */
	uint16_t normf_x;		/* Normalization factor for
					the horizontal direction */
	uint16_t wscalef_y;		/* Whole part of the scaling factor
						in the vertical direction */
	uint16_t fscalef_y;		/* Fractional part of the scaling factor
						in the vertical direction */
	uint16_t normf_y;		/* Normalization factor
						for the vertical direction */
	uint16_t output_width;		/* width of output window */
	uint16_t output_height;		/* height of output window */
};

/* Low resolution resize parameters. */

struct ISP_LRESIZE_PARAMS_T {
	/* Fields starting with an underscore do not actually have registers in
	the hardware, so should not be used in the software simulator. */
	uint16_t scale_x;	/* Scaling factor in the horizontal direction */
	uint16_t scale_y;	/* Scaling factor in the vertical direction */
#if ISP_VERSION >= 300
	uint32_t tscale_x;	/* scale for trap filter x */
	uint32_t tscale_y;	/* scale for trap filter y */
	uint16_t tnorm_0;	/* norm param (x[i]   * y[i]) */
	uint16_t tnorm_1;	/* norm param (x[i]+1 * y[i]) */
	uint16_t tnorm_2;	/* norm param (x[i]   * y[i]+1) */
	uint16_t tnorm_3;	/* norm param (x[i]+1 * y[i]+1) */
	uint16_t _output_width; /* Output Image Width for new
					trapezoidal resizer */
	uint16_t _output_height;/* Output Image Height for new
					trapezoidal resizer */
#else
	uint16_t norm_x;	/* Normalization factor for the
					horizontal direction */
	uint16_t norm_y;	/* Normalization factor for the
					vertical direction */
	uint16_t lnorm_x;	/* Normalization factor for the horizontal
					direction for the last column */
	uint16_t lnorm_y;	/* Normalization factor for the vertical
					direction for the last row */
#endif
	uint8_t shift;		/* Left shift applied to data. */
#if ISP_VERSION >= 410
	uint8_t rb_swap;	/* Swap primary and tertiary components */
	uint8_t pad[2];
#else
	uint8_t pad[3];
#endif
};

/* Output formatter parameters. */

struct ISP_OUTPUT_PARAMS_T {
	uint8_t data_format;
	uint8_t output_width;
	uint8_t col_mode;
	uint8_t hflip;
	uint8_t vflip;
	uint8_t shift;
	uint8_t col_width;	/* 00-32 pix, 01-64, 10-128, 11-256 */
	uint8_t pad1[1];
	uint32_t col_stride1;
	uint32_t col_stride2;
	void *address1;
	void *address2;
	void *address3;
#if ISP_VERSION < 200
	uint32_t stride1;
	uint32_t stride2;
#else
	int32_t stride1;	/* stored -ve when vflipped */
	int32_t stride2;	/* stored -ve when vflipped */
#endif
	uint16_t _buffer_height; /* used only by ISP control s/w,
			not used by the h/w itself. This gets passed to the ISP
			as the vflip flag - it's the version of the flag that's
			been adjusted for the image height parity. */
	uint8_t v_subsample;
	uint8_t pad2[1];
};

struct isp_job_post_t {
	uint16_t informat;
	uint32_t outformat;
	uint32_t tile_addr;
	uint32_t tile_count;
	struct ISP_FRAME_PARAMS_T frame;
	struct ISP_TILE_OFFSET_PARAMS_T tile_offsets;
	struct ISP_LINES_AVAIL_PARAMS_T lines_avail;
	struct ISP_Y_G_COEFFS_PARAMS_T y_g_coeffs;
	struct ISP_RAW_INPUT_PARAMS_T raw_input;
	struct ISP_YUV_INPUT_PARAMS_T yuv_input;
	struct ISP_CONVERSION_PARAMS_T yuv_conversion;
	struct ISP_HRESIZE_PARAMS_T hresize;
	struct ISP_CONVERSION_PARAMS_T conversion;
	struct ISP_LRESIZE_PARAMS_T lresize;
	struct ISP_OUTPUT_PARAMS_T lo_res_output;
	struct ISP_OUTPUT_PARAMS_T hi_res_output;
};

enum {
	ISP_CMD_WAIT_IRQ = 0x80,
	ISP_CMD_CLK_RESET,
	ISP_CMD_RELEASE_IRQ,
	ISP_CMD_LAST
};

#define ISP_IOCTL_WAIT_IRQ _IOR(BCM_ISP_MAGIC, ISP_CMD_WAIT_IRQ, unsigned int)
#define ISP_IOCTL_CLK_RESET _IOR(BCM_ISP_MAGIC, ISP_CMD_CLK_RESET, unsigned int)
#define ISP_IOCTL_RELEASE_IRQ _IOR(BCM_ISP_MAGIC, ISP_CMD_RELEASE_IRQ, \
							unsigned int)


#endif
