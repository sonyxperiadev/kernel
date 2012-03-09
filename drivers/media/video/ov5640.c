/*
 * OmniVision OV5640 sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include "ov5640.h"


/* OV5640 has only one fixed colorspace per pixelcode */
struct ov5640_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct ov5640_timing_cfg {
	u16 x_addr_start;
	u16 y_addr_start;
	u16 x_addr_end;
	u16 y_addr_end;
	u16 h_output_size;
	u16 v_output_size;
	u16 h_total_size;
	u16 v_total_size;
	u16 isp_h_offset;
	u16 isp_v_offset;
	u8 h_odd_ss_inc;
	u8 h_even_ss_inc;
	u8 v_odd_ss_inc;
	u8 v_even_ss_inc;
	u8 out_mode_sel;
	u8 sclk_dividers;
	u8 sys_mipi_clk;

};

static const struct ov5640_datafmt ov5640_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum ov5640_size {
	OV5640_SIZE_QVGA,	/*  320 x 240 */
	OV5640_SIZE_VGA,	/*  640 x 480 */
	OV5640_SIZE_XGA,	/*  1024 x 768  */
	OV5640_SIZE_720P,
	OV5640_SIZE_1280x960,	/*  1280 x 960 (1.2M) */
	OV5640_SIZE_UXGA,	/*  1600 x 1200 (2M) */
	OV5640_SIZE_1080P,
	OV5640_SIZE_QXGA,	/*  2048 x 1536 (3M) */
	OV5640_SIZE_5MP,
	OV5640_SIZE_LAST,
	OV5640_SIZE_MAX
};

static const struct v4l2_frmsize_discrete ov5640_frmsizes[OV5640_SIZE_LAST] = {
	{320, 240},
	{640, 480},
	{1024, 768},
	{1280, 720},
	{1280, 960},
	{1600, 1200},
	{1920, 1080},
	{2048, 1536},
	{2560, 1920},
};

/* Find a data format by a pixel code in an array */
static int ov5640_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov5640_fmts); i++)
		if (ov5640_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov5640_fmts))
		i = ARRAY_SIZE(ov5640_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov5640_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV5640_SIZE_LAST; i++) {
		if ((ov5640_frmsizes[i].width >= width) &&
		    (ov5640_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV5640_SIZE_LAST)
		i = OV5640_SIZE_LAST - 1;

	return i;
}

struct ov5640 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
	int sharpness;
	int saturation;
	int antibanding;
};

static struct ov5640 *to_ov5640(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov5640, subdev);
}


static const struct ov5640_timing_cfg timing_cfg_yuv[OV5640_SIZE_LAST] = {
	[OV5640_SIZE_QVGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 320,
			      .v_output_size = 240,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[OV5640_SIZE_VGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 640,
			      .v_output_size = 480,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[OV5640_SIZE_XGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1024,
			      .v_output_size = 768,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[OV5640_SIZE_720P] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1280,
			      .v_output_size = 720,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			      },
	[OV5640_SIZE_1280x960] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1280,
			      .v_output_size = 960,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x07,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			      },
	[OV5640_SIZE_UXGA] = {
            // Sensor Timing control  2624 x 1952 --> 1600 x 1200
			      .x_addr_start = 512,
			      .y_addr_start = 376,
			      .x_addr_end = 2111,
			      .y_addr_end = 1575,
            // Output image size
			      .h_output_size = 1600,
			      .v_output_size = 1200,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2200,
			      .v_total_size = 1280,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x06,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			      },
	[OV5640_SIZE_1080P] = {
            // Sensor Timing control  2624 x 1952 --> 1952 x 1088
			      .x_addr_start = 336,
			      .y_addr_start = 434,
			      .x_addr_end = 2287,
			      .y_addr_end = 1521,
            // Output image size
			      .h_output_size = 1920,
			      .v_output_size = 1080,
            // ISP Windowing size  1952 x 1088 --> 1920 x 1080
			      .isp_h_offset = 16,
			      .isp_v_offset = 4,
            // Total size (+blanking)
			      .h_total_size = 2500,
			      .v_total_size = 1120,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x06,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[OV5640_SIZE_QXGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 2048,
			      .v_output_size = 1536,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x06,
			      .sclk_dividers = 0x02,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5640_SIZE_5MP] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 2560,
			      .v_output_size = 1920,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Enabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x06,
			      .sclk_dividers = 0x02,
			      .sys_mipi_clk = 0x12,
			      },
};

static const struct ov5640_timing_cfg timing_cfg_jpeg[OV5640_SIZE_LAST] = {
	[OV5640_SIZE_QVGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 320,
			      .v_output_size = 240,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_VGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 640,
			      .v_output_size = 480,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_XGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1024,
			      .v_output_size = 768,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_720P] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1280,
			      .v_output_size = 720,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_1280x960] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1280,
			      .v_output_size = 960,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5640_SIZE_UXGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1600,
			      .v_output_size = 1200,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_1080P] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 1920,
			      .v_output_size = 1080,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_QXGA] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 2048,
			      .v_output_size = 1536,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
	[OV5640_SIZE_5MP] = {
            // Sensor Timing control  2624 x 1952 --> 2560 x 1920
			      .x_addr_start = 32,
			      .y_addr_start = 16,
			      .x_addr_end = 2591,
			      .y_addr_end = 1935,
            // Output image size
			      .h_output_size = 2560,
			      .v_output_size = 1920,
            // ISP Windowing size  2560 x 1920 --> 2560 x 1920
			      .isp_h_offset = 0,
			      .isp_v_offset = 0,
            // Total size (+blanking)
			      .h_total_size = 2844,
			      .v_total_size = 1968,
            // Sensor Read Binning Disabled
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			       },
};

#if 0
static struct v4l2_subdev_sensor_serial_parms mipi_cfgs[OV5640_SIZE_LAST] = {
	[OV5640_SIZE_QVGA] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[OV5640_SIZE_VGA] = {
			     .lanes = 1,
			     .channel = 0,
			     .phy_rate = (336 * 2 * 1000000),
			     .pix_clk = 21,	/* Revisit */
			     },
	[OV5640_SIZE_XGA] = {
			     .lanes = 1,
			     .channel = 0,
			     .phy_rate = (336 * 2 * 1000000),
			     .pix_clk = 21,	/* Revisit */
			     },
	[OV5640_SIZE_1080P] = {
			       .lanes = 1,
			       .channel = 0,
			       .phy_rate = (336 * 2 * 1000000),
			       .pix_clk = 21,	/* Revisit */
			       },
	[OV5640_SIZE_UXGA] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[OV5640_SIZE_1280x960] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[OV5640_SIZE_720P] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[OV5640_SIZE_QXGA] = {
			      .lanes = 1,
			      .channel = 0,
			      .phy_rate = (336 * 2 * 1000000),
			      .pix_clk = 21,	/* Revisit */
			      },
	[OV5640_SIZE_5MP] = {
			     .lanes = 1,
			     .channel = 0,
			     .phy_rate = (336 * 2 * 1000000),
			     .pix_clk = 21,	/* Revisit */
			     },
};
#endif

/**
 *ov5640_reg_read - Read a value from a register in an ov5640 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov5640 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 data[2] = { 0 };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 2,
		.buf = data,
	};

	data[0] = (u8) (reg >> 8);
	data[1] = (u8) (reg & 0xff);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	msg.flags = I2C_M_RD;
	msg.len = 1;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	*val = data[0];
	return 0;

      err:
	dev_err(&client->dev, "Failed reading register 0x%02x!\n", reg);
	return ret;
}

/**
 * Write a value to a register in ov5640 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { (u8) (reg >> 8), (u8) (reg & 0xff), val };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 3,
		.buf = data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	return 0;
}

static const struct v4l2_queryctrl ov5640_controls[] = {
	{
	 .id = V4L2_CID_CAMERA_BRIGHTNESS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Brightness",
	 .minimum = EV_MINUS_1,
	 .maximum = EV_PLUS_1,
	 .step = 1,
	 .default_value = EV_DEFAULT,
	 },
	{
	 .id = V4L2_CID_CAMERA_CONTRAST,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Contrast",
	 .minimum = CONTRAST_MINUS_1,
	 .maximum = CONTRAST_PLUS_1,
	 .step = 1,
	 .default_value = CONTRAST_DEFAULT,
	 },
	{
	 .id = V4L2_CID_CAMERA_EFFECT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Color Effects",
	 .minimum = IMAGE_EFFECT_NONE,
	 .maximum = (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_SEPIA |
			 1 << IMAGE_EFFECT_BNW | 1 << IMAGE_EFFECT_NEGATIVE),
	 .step = 1,
	 .default_value = IMAGE_EFFECT_NONE,
	 },
	{
	 .id = V4L2_CID_CAMERA_ANTI_BANDING,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Anti Banding",
	 .minimum = ANTI_BANDING_AUTO,
	 .maximum = ANTI_BANDING_60HZ,
	 .step = 1,
	 .default_value = ANTI_BANDING_AUTO,
	 },
};

/**
 * Initialize a list of ov5640 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5640_reg_writes(struct i2c_client *client,
			     const struct ov5640_reg reglist[])
{
	int err = 0, index;

	for (index = 0; ((reglist[index].reg != 0xFFFF) && (err == 0)); index++) {
		err |=
		    ov5640_reg_write(client, reglist[index].reg,
				     reglist[index].val);
		/*  Check for Pause condition */
		if ((reglist[index + 1].reg == 0xFFFF)
		    && (reglist[index + 1].val != 0)) {
			msleep(reglist[index + 1].val);
			index += 1;
		}
	}
	return 0;
}

static int ov5640_config_timing(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret, i = ov5640->i_size;
	const struct ov5640_timing_cfg *timing_cfg;

	if (ov5640_fmts[ov5640->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8) {
        timing_cfg = &timing_cfg_jpeg[i];
    }
    else {
        timing_cfg = &timing_cfg_yuv[i];
    }

	ret = ov5640_reg_write(client,
			       0x3800,
			       (timing_cfg->x_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3801, timing_cfg->x_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3802,
			       (timing_cfg->y_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3803, timing_cfg->y_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3804,
			       (timing_cfg->x_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client, 0x3805, timing_cfg->x_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3806,
			       (timing_cfg->y_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client, 0x3807, timing_cfg->y_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3808,
			       (timing_cfg->h_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3809, timing_cfg->h_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380A,
			       (timing_cfg->v_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380B, timing_cfg->v_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380C,
			       (timing_cfg->h_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380D, timing_cfg->h_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380E,
			       (timing_cfg->v_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x380F, timing_cfg->v_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3810,
			       (timing_cfg->isp_h_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3811, timing_cfg->isp_h_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3812,
			       (timing_cfg->isp_v_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3813, timing_cfg->isp_v_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3814,
			       ((timing_cfg->h_odd_ss_inc & 0xF) << 4) |
			       (timing_cfg->h_even_ss_inc & 0xF));
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3815,
			       ((timing_cfg->v_odd_ss_inc & 0xF) << 4) |
			       (timing_cfg->v_even_ss_inc & 0xF));
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3821, timing_cfg->out_mode_sel & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client,
			       0x3108, timing_cfg->sclk_dividers & 0xFF);
	if (ret)
		return ret;

	ret = ov5640_reg_write(client, 0x3035, timing_cfg->sys_mipi_clk & 0xFF);
	if (ret)
		return ret;

	return ret;
}

static int ov5640_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;

	if (enable) {
		ret = ov5640_reg_write(client, 0x3008, 0x02);
		if (ret)
			goto out;
	} else {
		u8 tmpreg = 0;

		ret = ov5640_reg_read(client, 0x3008, &tmpreg);
		if (ret)
			goto out;

		ret = ov5640_reg_write(client, 0x3008, tmpreg | 0x40);
		if (ret)
			goto out;

		ret = ov5640_reg_write(client, 0x300e, 0x3d);	/* MIPI Control  Powered down   */
		if (ret)
			goto out;
	}

      out:
	return ret;
}

static int ov5640_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long ov5640_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
	    SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
	    SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int ov5640_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "ov5640");

	if (icl && icl->priv) {

		plat_parms = icl->priv;
		inp->status = 0;

		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			inp->status |= V4L2_IN_ST_HFLIP;

		if (plat_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			inp->status |= V4L2_IN_ST_BACK;

	}
	return 0;
}

static int ov5640_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);

	mf->width = ov5640_frmsizes[ov5640->i_size].width;
	mf->height = ov5640_frmsizes[ov5640->i_size].height;
	mf->code = ov5640_fmts[ov5640->i_fmt].code;
	mf->colorspace = ov5640_fmts[ov5640->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov5640_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = ov5640_find_datafmt(mf->code);

	mf->code = ov5640_fmts[i_fmt].code;
	mf->colorspace = ov5640_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = ov5640_find_framesize(mf->width, mf->height);

	mf->width = ov5640_frmsizes[i_size].width;
	mf->height = ov5640_frmsizes[i_size].height;

	return 0;
}

static int ov5640_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret = 0;

	ret = ov5640_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

    ret = ov5640_reg_writes(client, configscript_common1);

	ov5640->i_size = ov5640_find_framesize(mf->width, mf->height);
	ov5640->i_fmt = ov5640_find_datafmt(mf->code);


	switch ((u32) ov5640_fmts[ov5640->i_fmt].code) {
	case V4L2_MBUS_FMT_UYVY8_2X8:
		ret = ov5640_reg_writes(client, yuv422_init_common);
		if (ret)
			return ret;
		ret = ov5640_reg_write(client, 0x4300, 0x32);
		if (ret)
			return ret;
		break;
	case V4L2_MBUS_FMT_YUYV8_2X8:
		ret = ov5640_reg_writes(client, yuv422_init_common);
		if (ret)
			return ret;
		ret = ov5640_reg_write(client, 0x4300, 0x30);
		if (ret)
			return ret;
		break;
	case V4L2_MBUS_FMT_JPEG_1X8:
		ret = ov5640_reg_writes(client, jpeg_init_common);
		if (ret)
			return ret;
		break;
	default:
		/* This shouldn't happen */
		ret = -EINVAL;
		return ret;
	}

	ret = ov5640_config_timing(client);
	if (ret)
		return ret;

	return ret;
}

static int ov5640_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV5640;
	id->revision = 0;

	return 0;
}

static int ov5640_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);

	dev_dbg(&client->dev, "ov5640_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:
		ctrl->value = ov5640->brightness;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = ov5640->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = ov5640->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = ov5640->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = ov5640->sharpness;
		break;
	case V4L2_CID_CAMERA_ANTI_BANDING:
		ctrl->value = ov5640->antibanding;
		break;
	}

	return 0;
}

static int ov5640_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret = 0;

	dev_dbg(&client->dev, "ov5640_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:

		if (ctrl->value > EV_PLUS_1)
			return -EINVAL;

		ov5640->brightness = ctrl->value;
		switch (ov5640->brightness) {
		case EV_MINUS_1:
			ret = ov5640_reg_writes(client,
					ov5640_brightness_lv4_tbl);
			break;
		case EV_PLUS_1:
			ret = ov5640_reg_writes(client,
					ov5640_brightness_lv0_tbl);
			break;
		default:
			ret = ov5640_reg_writes(client,
					ov5640_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_CONTRAST:

		if (ctrl->value > CONTRAST_PLUS_1)
			return -EINVAL;

		ov5640->contrast = ctrl->value;
		switch (ov5640->contrast) {
		case CONTRAST_MINUS_1:
			ret = ov5640_reg_writes(client,
					ov5640_contrast_lv5_tbl);
			break;
		case CONTRAST_PLUS_1:
			ret = ov5640_reg_writes(client,
					ov5640_contrast_lv0_tbl);
			break;
		default:
			ret = ov5640_reg_writes(client,
					ov5640_contrast_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_EFFECT:

		if (ctrl->value > IMAGE_EFFECT_BNW)
			return -EINVAL;

		ov5640->colorlevel = ctrl->value;

		switch (ov5640->colorlevel) {
		case IMAGE_EFFECT_BNW:
			ret = ov5640_reg_writes(client,
					ov5640_effect_bw_tbl);
			break;
		case IMAGE_EFFECT_SEPIA:
			ret = ov5640_reg_writes(client,
					ov5640_effect_sepia_tbl);
			break;
		case IMAGE_EFFECT_NEGATIVE:
			ret = ov5640_reg_writes(client,
					ov5640_effect_negative_tbl);
			break;
		default:
			ret = ov5640_reg_writes(client,
					ov5640_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SATURATION:

		if (ctrl->value > OV5640_SATURATION_MAX)
			return -EINVAL;

		ov5640->saturation = ctrl->value;
		switch (ov5640->saturation) {
		case OV5640_SATURATION_MIN:
			ret = ov5640_reg_writes(client,
					ov5640_saturation_lv0_tbl);
			break;
		case OV5640_SATURATION_MAX:
			ret = ov5640_reg_writes(client,
					ov5640_saturation_lv5_tbl);
			break;
		default:
			ret = ov5640_reg_writes(client,
					ov5640_saturation_default_lv3_tbl);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SHARPNESS:

		if (ctrl->value > OV5640_SHARPNESS_MAX)
			return -EINVAL;

		ov5640->sharpness = ctrl->value;
		switch (ov5640->sharpness) {
		case OV5640_SHARPNESS_MIN:
			ret = ov5640_reg_writes(client,
					ov5640_sharpness_lv0_tbl);
			break;
		case OV5640_SHARPNESS_MAX:
			ret = ov5640_reg_writes(client,
					ov5640_sharpness_lv3_tbl);
			break;
		default:
			ret = ov5640_reg_writes(client,
					ov5640_sharpness_default_lv2_tbl);
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_ANTI_BANDING:

		if (ctrl->value > ANTI_BANDING_60HZ)
			return -EINVAL;

		ov5640->antibanding = ctrl->value;

		switch (ov5640->antibanding) {
			case ANTI_BANDING_50HZ:
				ret = ov5640_reg_writes(client,
						ov5640_antibanding_50z_tbl);
				break;
			case ANTI_BANDING_60HZ:
				ret = ov5640_reg_writes(client,
						ov5640_antibanding_60z_tbl);
				break;
			default:
				ret = ov5640_reg_writes(client,
						ov5640_antibanding_auto_tbl);
				break;
		}
		if (ret)
			return ret;
		break;
	}

	return ret;
}

static long ov5640_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	switch (cmd) {
	case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
			*p = 0;	/* no we don't support thumbnail */
			break;
		}
	case VIDIOC_JPEG_G_PACKET_INFO:
		{
			struct v4l2_jpeg_packet_info *p = (struct v4l2_jpeg_packet_info *)arg;
			p->padded = 0;
			p->packet_size = 0x400;
			break;
		}

	case VIDIOC_SENSOR_G_OPTICAL_INFO:
		{
			struct v4l2_sensor_optical_info *p = (struct v4l2_sensor_optical_info *)arg;
			/* assuming 67.5 degree diagonal viewing angle */
			p->hor_angle.numerator = 5401;
			p->hor_angle.denominator = 100;
			p->ver_angle.numerator = 3608;
			p->ver_angle.denominator = 100;
			p->focus_distance[0] = 10; /* near focus in cm */
			p->focus_distance[1] = 100; /* optimal focus in cm */
			p->focus_distance[2] = -1; /* infinity */
			p->focal_length.numerator = 342;
			p->focal_length.denominator = 100;
			break;
		}
	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5640_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov5640_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov5640_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov5640_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static struct soc_camera_ops ov5640_ops = {
	.set_bus_param = ov5640_set_bus_param,
	.query_bus_param = ov5640_query_bus_param,
	.enum_input = ov5640_enum_input,
	.controls = ov5640_controls,
	.num_controls = ARRAY_SIZE(ov5640_controls),
};

static int ov5640_init(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	int ret = 0;

	ret = ov5640_reg_writes(client, configscript_common1);
	if (ret)
		goto out;

	/* default brightness and contrast */
	ov5640->brightness = EV_DEFAULT;
	ov5640->contrast = CONTRAST_DEFAULT;
	ov5640->colorlevel = IMAGE_EFFECT_NONE;
	ov5640->antibanding = ANTI_BANDING_AUTO;

	dev_dbg(&client->dev, "Sensor initialized\n");

      out:
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int ov5640_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	ret = ov5640_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect OV5640 chip\n");
		goto out;
	}
	printk(KERN_ERR "OV5640 value read=%x\n", revision);

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	dev_info(&client->dev, "Detected a OV5640 chip, revision %x\n",
		 revision);

	/* TODO: Do something like ov5640_init */

      out:
	return ret;
}

static void ov5640_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops ov5640_subdev_core_ops = {
	.g_chip_ident = ov5640_g_chip_ident,
	.g_ctrl = ov5640_g_ctrl,
	.s_ctrl = ov5640_s_ctrl,
	.ioctl = ov5640_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov5640_g_register,
	.s_register = ov5640_s_register,
#endif
};

static int ov5640_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov5640_fmts))
		return -EINVAL;

	*code = ov5640_fmts[index].code;
	return 0;
}

static int ov5640_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV5640_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = ov5640_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int ov5640_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov5640_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV5640_SIZE_5MP:
		interval->discrete.numerator = 2;
		interval->discrete.denominator = 15;
		break;
	case OV5640_SIZE_QXGA:
	case OV5640_SIZE_1080P:
	case OV5640_SIZE_UXGA:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case OV5640_SIZE_1280x960:
	case OV5640_SIZE_720P:
	case OV5640_SIZE_XGA:
	case OV5640_SIZE_VGA:
	case OV5640_SIZE_QVGA:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 24;
		break;
	}
/*	printk(KERN_ERR"%s: width=%d height=%d fi=%d/%d\n", __func__, interval->width,
			interval->height, interval->discrete.numerator, interval->discrete.denominator);
			*/
	return 0;
}

static int ov5640_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov5640->i_size) {
	case OV5640_SIZE_5MP:
		cparm->timeperframe.numerator = 2;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5640_SIZE_QXGA:
	case OV5640_SIZE_1080P:
	case OV5640_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5640_SIZE_1280x960:
	case OV5640_SIZE_720P:
	case OV5640_SIZE_XGA:
	case OV5640_SIZE_VGA:
	case OV5640_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}
static int ov5640_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov5640_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov5640_subdev_video_ops = {
	.s_stream = ov5640_s_stream,
	.s_mbus_fmt = ov5640_s_fmt,
	.g_mbus_fmt = ov5640_g_fmt,
	.try_mbus_fmt = ov5640_try_fmt,
	.enum_mbus_fmt = ov5640_enum_fmt,
	.enum_mbus_fsizes = ov5640_enum_framesizes,
	.enum_framesizes = ov5640_enum_framesizes,
	.enum_frameintervals = ov5640_enum_frameintervals,
	.g_parm = ov5640_g_parm,
	.s_parm = ov5640_s_parm,
};
static int ov5640_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}

static int ov5640_g_interface_parms(struct v4l2_subdev *sd,
				    struct v4l2_subdev_sensor_interface_parms
				    *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5640 *ov5640 = to_ov5640(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = ov5640->plat_parms->if_type;
	parms->if_mode = ov5640->plat_parms->if_mode;
	parms->parms = ov5640->plat_parms->parms;

	/* set the hs term time */
	if (ov5640_fmts[ov5640->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		sclk_dividers  = timing_cfg_jpeg[ov5640->i_size].sclk_dividers;
	else
		sclk_dividers = timing_cfg_yuv[ov5640->i_size].sclk_dividers;

	if (sclk_dividers == 0x01)
		parms->parms.serial.hs_term_time = 0x01;
	else
		parms->parms.serial.hs_term_time = 0x08;

	return 0;
}



static struct v4l2_subdev_sensor_ops ov5640_subdev_sensor_ops = {
	.g_skip_frames = ov5640_g_skip_frames,
	.g_interface_parms = ov5640_g_interface_parms,
};

static struct v4l2_subdev_ops ov5640_subdev_ops = {
	.core = &ov5640_subdev_core_ops,
	.video = &ov5640_subdev_video_ops,
	.sensor = &ov5640_subdev_sensor_ops,
};

static int ov5640_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov5640 *ov5640;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;

	if (!icd) {
		dev_err(&client->dev, "OV5640: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "OV5640 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"OV5640 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov5640 = kzalloc(sizeof(struct ov5640), GFP_KERNEL);
	if (!ov5640)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov5640->subdev, client, &ov5640_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &ov5640_ops;

	ov5640->i_size = OV5640_SIZE_VGA;
	ov5640->i_fmt = 0;	/* First format in the list */
	ov5640->plat_parms = icl->priv;

	ret = ov5640_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(ov5640);
		return ret;
	}

	/* init the sensor here */
	ret = ov5640_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

	return ret;
}

static int ov5640_remove(struct i2c_client *client)
{
	struct ov5640 *ov5640 = to_ov5640(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	icd->ops = NULL;
	ov5640_video_remove(icd);
	client->driver = NULL;
	kfree(ov5640);

	return 0;
}

static const struct i2c_device_id ov5640_id[] = {
	{"ov5640", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov5640_id);

static struct i2c_driver ov5640_i2c_driver = {
	.driver = {
		   .name = "ov5640",
		   },
	.probe = ov5640_probe,
	.remove = ov5640_remove,
	.id_table = ov5640_id,
};

static int __init ov5640_mod_init(void)
{
	return i2c_add_driver(&ov5640_i2c_driver);
}

static void __exit ov5640_mod_exit(void)
{
	i2c_del_driver(&ov5640_i2c_driver);
}

module_init(ov5640_mod_init);
module_exit(ov5640_mod_exit);

MODULE_DESCRIPTION("OmniVision OV5640 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
