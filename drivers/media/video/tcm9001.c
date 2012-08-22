/*
 * tcm9001 sensor driver
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
#include <linux/io.h>
#include <linux/module.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>

#define ENABLE_COLOR_PATTERN 0
/* #define MIPI_2_LANES */
/* #define FPS_30_MODE            */

/* tcm9001 has only one fixed colorspace per pixelcode */
struct tcm9001_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

static const struct tcm9001_datafmt tcm9001_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	/* V4L2_COLORSPACE_JPEG only means full 256 values
	   on all color components and JPEG
	   Need to check what comes from sensor UYVY or YUYV */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
};

enum TCM9001_size {
	TCM9001_SIZE_QVGA,	/*  320 x 240 */
	TCM9001_SIZE_VGA,	/*  640 x 480 */
	TCM9001_SIZE_MAX
};

static const struct v4l2_frmsize_discrete tcm9001_frmsizes[TCM9001_SIZE_MAX] = {
	{320, 240},
	{640, 480},
};

/* Find a data format by a pixel code in an array */
static int tcm9001_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tcm9001_fmts); i++)
		if (tcm9001_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(tcm9001_fmts))
		i = ARRAY_SIZE(tcm9001_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int tcm9001_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < TCM9001_SIZE_MAX; i++) {
		if ((tcm9001_frmsizes[i].width >= width) &&
		    (tcm9001_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= TCM9001_SIZE_MAX)
		i = TCM9001_SIZE_MAX - 1;

	return i;
}

struct tcm9001 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
	int framerate;
};

static struct tcm9001 *to_tcm9001(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct tcm9001, subdev);
}

/**
 *tcm9001_reg_read - Read a value from a register in an tcm9001 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an tcm9001 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int tcm9001_reg_read(struct i2c_client *client, u16 reg, u8 * val)
{
	int ret;
	u8 reg8 = (u8) reg;
	u8 data[1];
	data[0] = reg8;
	struct i2c_msg msg[2] = {
		{
		 client->addr,
		 client->flags,
		 1,
		 data},
		{
		 client->addr,
		 client->flags | I2C_M_RD,
		 1,
		 val}
	};
	ret = i2c_transfer(client->adapter, msg, 2);
	return 0;
}

/**
 * Write a value to a register in tcm9001 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int tcm9001_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	u8 reg8 = (u8) reg;
	unsigned char data[2];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = client->flags,
		.len = 2,
		.buf = data,
	};
	data[0] = reg8;
	data[1] = val;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		printk(KERN_INFO KERN_ERR "Failed in write_reg writing over I2C\n");
		return ret;
	}
	return 0;
}

static const struct v4l2_queryctrl tcm9001_controls[] = {
	{
	 .id = V4L2_CID_BRIGHTNESS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Brightness",
	 .minimum = -128,
	 .maximum = 127,
	 .step = 1,
	 .default_value = 0,
	 },
	{
	 .id = V4L2_CID_CONTRAST,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Contrast",
	 .minimum = 0,
	 .maximum = 4,
	 .step = 1,
	 .default_value = 1,
	 },
	{
	 .id = V4L2_CID_CAMERA_EFFECT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Color Effects",
	 .minimum = IMAGE_EFFECT_NONE,
	 .maximum = (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_SEPIA |
		     1 << IMAGE_EFFECT_MONO | 1 << IMAGE_EFFECT_NEGATIVE),
	 .step = 1,
	 .default_value = IMAGE_EFFECT_NONE,
	 },
	{
	.id = V4L2_CID_CAMERA_FRAME_RATE,
	.type = V4L2_CTRL_TYPE_INTEGER,
	.name = "Framerate control",
	.minimum = FRAME_RATE_AUTO,
	.maximum = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_5 |
			1 << FRAME_RATE_10 | 1 << FRAME_RATE_15 |
			1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
			.step = 1,
			.default_value = FRAME_RATE_AUTO,
	},
};
static int tcm9001_init(struct i2c_client *client);
static int tcm9001_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);
	int ret = 0;
	int val;
	if (enable) {
		ret = tcm9001_init(client);
		if (tcm9001_frmsizes[tcm9001->i_size].width == 640) {
			printk(KERN_INFO "Setting VGA\n");
			tcm9001_reg_write(client, 0x22, 0x07);
		} else {	/* QVGA*/
			printk(KERN_INFO "Setting QVGA\n");
			tcm9001_reg_write(client, 0x22, 0x03);
		}
	} else {
		tcm9001_reg_read(client, 0xFF, &val);
		val = val | 0x30;
		tcm9001_reg_write(client, 0xFF, val);
		printk(KERN_INFO "Disabling !!!!! STREAM from TCM9001 client\n");
		/* Nothing to do */
	}
	return ret;
}

static int tcm9001_set_bus_param(struct soc_camera_device *icd,
				 unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}

static unsigned long tcm9001_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
	    SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
	    SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */
/* Not sure what this is */
	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

static int tcm9001_enum_input(struct soc_camera_device *icd,
			      struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "tcm9001");

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

static int tcm9001_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);

	mf->width = tcm9001_frmsizes[tcm9001->i_size].width;
	mf->height = tcm9001_frmsizes[tcm9001->i_size].height;
	mf->code = tcm9001_fmts[tcm9001->i_fmt].code;
	mf->colorspace = tcm9001_fmts[tcm9001->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int tcm9001_try_fmt(struct v4l2_subdev *sd,
			   struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = tcm9001_find_datafmt(mf->code);

	mf->code = tcm9001_fmts[i_fmt].code;
	mf->colorspace = tcm9001_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = tcm9001_find_framesize(mf->width, mf->height);

	mf->width = tcm9001_frmsizes[i_size].width;
	mf->height = tcm9001_frmsizes[i_size].height;

	return 0;
}

static int tcm9001_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);
	/* We support only YUV422 8 bits 2 bytes per pix */

	/* Resolution is also VGA */
	/* Change here for QVGA */
	tcm9001->i_size = tcm9001_find_framesize(mf->width, mf->height);
	return 0;
}

static int tcm9001_g_chip_ident(struct v4l2_subdev *sd,
				struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

/*      id->ident = V4L2_IDENT_TCM9001;*/
	id->revision = 0;

	return 0;
}

static int tcm9001_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);

	dev_dbg(&client->dev, "tcm9001_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = tcm9001->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = tcm9001->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = tcm9001->colorlevel;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = tcm9001->framerate;
		break;
	}
	return 0;
}

static int tcm9001_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);
	int ret = 0;
	int temp;
	u8 color = 0;

	dev_dbg(&client->dev, "tcm9001_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		tcm9001->brightness = ctrl->value;
		tcm9001_reg_write(client, 0xB6, (ctrl->value + 128));
		break;
	case V4L2_CID_CONTRAST:
		tcm9001->contrast = ctrl->value;
		temp = (ctrl->value) / 64;
		tcm9001_reg_write(client, 0xB5, temp);
		break;
	case V4L2_CID_CAMERA_EFFECT:
		tcm9001->colorlevel = ctrl->value;
		if (tcm9001->colorlevel == IMAGE_EFFECT_NONE)
			color = 0xC8;
		/*
		   else if(tcm9001->colorlevel == IMAGE_EFFECT_SEPIA)
		   color |= 0x2; */
		else if (tcm9001->colorlevel == IMAGE_EFFECT_MONO)
			color = 0x11;
		/*
		   else if(tcm9001->colorlevel == ) //
		   color |= 0x6; */
		else if (tcm9001->colorlevel == IMAGE_EFFECT_NEGATIVE)
			color = 0x14;
		else if (tcm9001->colorlevel == IMAGE_EFFECT_SEPIA)
			color = 0x13;
		/*
		   else if(tcm9001->colorlevel ==) // Emboss has not V4L2option
		   color |= 0x5;
		   else if(tcm9001->colorlevel ==)
		   color |= 0x7; */
		printk(KERN_INFO "Writing color value 0x%x\n", color);
		tcm9001_reg_write(client, 0xB4, color);
	case V4L2_CID_CAMERA_FRAME_RATE:
		tcm9001->framerate = ctrl->value;
		switch (tcm9001->framerate) {
		case FRAME_RATE_5:
		/* case FRAME_RATE_7: */
		case FRAME_RATE_10:
		case FRAME_RATE_15:
		case FRAME_RATE_20:
		case FRAME_RATE_25:
		case FRAME_RATE_30:
		case FRAME_RATE_AUTO:
		default:
		printk(KERN_INFO "Framerate %d\n", tcm9001->framerate);
		break;
		}
		break;
	default:
		break;
	}

	return ret;
}

static long tcm9001_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	switch (cmd) {
	case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
			*p = 0;	/* no we don't support thumbnail */
			break;
		}

	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int tcm9001_g_register(struct v4l2_subdev *sd,
			      struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (tcm9001_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int tcm9001_s_register(struct v4l2_subdev *sd,
			      struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (tcm9001_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif

static struct soc_camera_ops tcm9001_ops = {
	.set_bus_param = tcm9001_set_bus_param,
	.query_bus_param = tcm9001_query_bus_param,
	.enum_input = tcm9001_enum_input,
	.controls = tcm9001_controls,
	.num_controls = ARRAY_SIZE(tcm9001_controls),
};

/* Init section */
struct tcm9001_set {
	u8 addr;
	u8 val;
};

static struct tcm9001_set tcm9001_init_regset[] = {
	{0x00, 0x48},
	{0x01, 0x10},
	{0x02, 0xD8},		/*alcint_sekiout[7:0];*/
	{0x03, 0x00},		/*alc_ac5060out/alc_les_modeout[2:0]
				///alcint_sekiout[9:;*/
	{0x04, 0x20},		/*alc_agout[7:0];*/
	{0x05, 0x00},		/*alc_agout[11:8];*/
	{0x06, 0x2D},		/*alc_dgout[7:0];*/
	{0x07, 0x9D},		/*alc_esout[7:0];*/
	{0x08, 0x80},		/*alc_okout//alc_esout[13:8];*/
	{0x09, 0x45},		/*awb_uiout[7:0];*/
	{0x0A, 0x17},		/*awb_uiout[15:8];*/
	{0x0B, 0x05},		/*awb_uiout[23:16];*/
	{0x0C, 0x00},		/*awb_uiout[29:24];*/
	{0x0D, 0x39},		/*awb_viout[7:0];*/
	{0x0E, 0x56},		/*awb_viout[15:8];*/
	{0x0F, 0xFC},		/*awb_viout[2316];*/
	{0x10, 0x3F},		/*awb_viout[29:24];*/
	{0x11, 0xA1},		/*awb_pixout[7:0];*/
	{0x12, 0x28},		/*awb_pixout[15:8];*/
	{0x13, 0x03},		/*awb_pixout[18:16];*/
	{0x14, 0x85},		/*awb_rgout[7:0];*/
	{0x15, 0x80},		/*awb_ggout[7:0];*/
	{0x16, 0x6B},		/*awb_bgout[7:0];*/
	{0x17, 0x00},
	{0x18, 0x9C},		/*LSTB*/
	{0x19, 0x04},		/*TSEL[2:0];*/
	{0x1A, 0x90},
	{0x1B, 0x02},		/*CKREF_DIV[4:0];*/
	{0x1C, 0x00},		/*CKVAR_SS0DIV[7:0];*/
	{0x1D, 0x00},		/* SPCK_SEL/EXTCLK_THROUGH/CKVAR_SS0DIV[8];*/
	{0x1E, 0x68},		/*CKVAR_SS1DIV[7:0];*/
	{0x1F, 0x00},		/*MRCK_DIV[3:0]///CKVAR_SS1DIV[8];*/
	{0x20, 0xC0},		/*VCO_DIV[1:0]/CLK_SEL[1:0]/AMON0SEL[1:0]/;*/
	{0x21, 0x0B},
	{0x22, 0x07},		/*TBINV/RLINV//WIN_MODE//HV_INTERMIT[2:0];*/
	{0x23, 0x96},		/*H_COUNT[7:0];*/
	{0x24, 0x00},
	{0x25, 0x42},		/*V_COUNT[7:0];*/
	{0x26, 0x00},		/*V_COUNT[10:8];*/
	{0x27, 0x00},
	{0x28, 0x00},
	{0x29, 0x83},
	{0x2A, 0x84},
	{0x2B, 0xAE},
	{0x2C, 0x21},
	{0x2D, 0x00},
	{0x2E, 0x04},
	{0x2F, 0x7D},
	{0x30, 0x19},
	{0x31, 0x88},
	{0x32, 0x88},
	{0x33, 0x09},
	{0x34, 0x6C},
	{0x35, 0x00},
	{0x36, 0x90},
	{0x37, 0x22},
	{0x38, 0x0B},
	{0x39, 0xAA},
	{0x3A, 0x0A},
	{0x3B, 0x84},
	{0x3C, 0x03},
	{0x3D, 0x10},
	{0x3E, 0x4C},
	{0x3F, 0x1D},
	{0x40, 0x34},
	{0x41, 0x05},
	{0x42, 0x12},
	{0x43, 0xB0},
	{0x44, 0x3F},
	{0x45, 0xFF},
	{0x46, 0x44},
	{0x47, 0x44},
	{0x48, 0x00},
	{0x49, 0xE8},
	{0x4A, 0x00},
	{0x4B, 0x9F},
	{0x4C, 0x9B},
	{0x4D, 0x2B},
	{0x4E, 0x53},
	{0x4F, 0x50},
	{0x50, 0x0E},
	{0x51, 0x00},
	{0x52, 0x00},
	{0x53, 0x04},		/*TP_MODE[4:0]/TPG_DR_SEL/TPG_CBLK_SW/
				TPG_LINE_SW;*/
	{0x54, 0x08},
	{0x55, 0x14},
	{0x56, 0x84},
	{0x57, 0x30},
	{0x58, 0x80},
	{0x59, 0x80},
	{0x5A, 0x00},
	{0x5B, 0x06},
	{0x5C, 0xF0},
	{0x5D, 0x00},
	{0x5E, 0x00},
	{0x5F, 0xB0},
	{0x60, 0x00},
	{0x61, 0x1B},
	{0x62, 0x4F},		/*HYUKO_START[7:0];*/
	{0x63, 0x04},		/*VYUKO_START[7:0];*/
	{0x64, 0x10},
	{0x65, 0x20},
	{0x66, 0x30},
	{0x67, 0x28},
	{0x68, 0x66},
	{0x69, 0xC0},
	{0x6A, 0x30},
	{0x6B, 0x30},
	{0x6C, 0x3F},
	{0x6D, 0xBF},
	{0x6E, 0xAB},
	{0x6F, 0x30},
	{0x70, 0x80},		/*AGMIN_BLACK_ADJ[7:0];*/
	{0x71, 0x90},		/*AGMAX_BLACK_ADJ[7:0];*/
	{0x72, 0x00},		/*IDR_SET[7:0];*/
	{0x73, 0x28},		/*PWB_RG[7:0];*/
	{0x74, 0x00},		/*PWB_GRG[7:0];*/
	{0x75, 0x00},		/*PWB_GBG[7:0];*/
	{0x76, 0x58},		/*PWB_BG[7:0];*/
	{0x77, 0x00},
	{0x78, 0x80},		/*LSSC_SW*/
	{0x79, 0x52},
	{0x7A, 0x4F},
	{0x7B, 0x90},		/*LSSC_LEFT_RG[7:0];*/
	{0x7C, 0x4D},		/*LSSC_LEFT_GG[7:0];*/
	{0x7D, 0x44},		/*LSSC_LEFT_BG[7:0];*/
	{0x7E, 0xC3},		/*LSSC_RIGHT_RG[7:0];*/
	{0x7F, 0x77},		/*LSSC_RIGHT_GG[7:0];*/
	{0x80, 0x67},		/*LSSC_RIGHT_BG[7:0];*/
	{0x81, 0x6D},		/*LSSC_TOP_RG[7:0];*/
	{0x82, 0x50},		/*LSSC_TOP_GG[7:0];*/
	{0x83, 0x3C},		/*LSSC_TOP_BG[7:0];*/
	{0x84, 0x78},		/*LSSC_BOTTOM_RG[7:0];*/
	{0x85, 0x4B},		/*LSSC_BOTTOM_GG[7:0];*/
	{0x86, 0x31},		/*LSSC_BOTTOM_BG[7:0];*/
	{0x87, 0x01},
	{0x88, 0x00},
	{0x89, 0x00},
	{0x8A, 0x40},
	{0x8B, 0x09},
	{0x8C, 0xE0},
	{0x8D, 0x20},
	{0x8E, 0x20},
	{0x8F, 0x20},
	{0x90, 0x20},
	{0x91, 0x80},		/*ANR_SW////TEST_ANR///;*/
	{0x92, 0x30},		/*AGMIN_ANR_WIDTH[7:0];*/
	{0x93, 0x40},		/*AGMAX_ANR_WIDTH[7:0];*/
	{0x94, 0x40},		/*AGMIN_ANR_MP[7:0];*/
	{0x95, 0x80},		/*AGMAX_ANR_MP[7:0];*/
	{0x96, 0x80},		/*DTL_SW////////;*/
	{0x97, 0x20},		/*AGMIN_HDTL_NC[7:0];*/
	{0x98, 0x68},		/*AGMIN_VDTL_NC[7:0];*/
	{0x99, 0xFF},		/*AGMAX_HDTL_NC[7:0];*/
	{0x9A, 0xFF},		/*AGMAX_VDTL_NC[7:0];*/
	{0x9B, 0x5C},		/*AGMIN_HDTL_MG[7:0];*/
	{0x9C, 0x28},		/*AGMIN_HDTL_PG[7:0];*/
	{0x9D, 0x40},		/*AGMIN_VDTL_MG[7:0];*/
	{0x9E, 0x28},		/*AGMIN_VDTL_PG[7:0];*/
	{0x9F, 0x00},		/*AGMAX_HDTL_MG[7:0];*/
	{0xA0, 0x00},		/*AGMAX_HDTL_PG[7:0];*/
	{0xA1, 0x00},		/*AGMAX_VDTL_MG[7:0];*/
	{0xA2, 0x00},		/*AGMAX_VDTL_PG[7:0];*/
	{0xA3, 0x80},
	{0xA4, 0x82},		/*HCBC_SW*/
	{0xA5, 0x38},		/*AGMIN_HCBC_G[7:0];*/
	{0xA6, 0x18},		/*AGMAX_HCBC_G[7:0];*/
	{0xA7, 0x98},
	{0xA8, 0x98},
	{0xA9, 0x98},
	{0xAA, 0x10},		/*LMCC_BMG_SEL/LMCC_BMR_SEL//LMCC_GMB_SEL
				/LMCC_GMR_SEL//;*/
	{0xAB, 0x5B},		/*LMCC_RMG_G[7:0];*/
	{0xAC, 0x00},		/*LMCC_RMB_G[7:0];*/
	{0xAD, 0x00},		/*LMCC_GMR_G[7:0];*/
	{0xAE, 0x00},		/*LMCC_GMB_G[7:0];*/
	{0xAF, 0x00},		/*LMCC_BMR_G[7:0];*/
	{0xB0, 0x48},		/*LMCC_BMG_G[7:0];*/
	{0xB1, 0x82},		/*GAM_SW[1:0]//CGC_DISP/TEST_AWBDISP
				//YUVM_AWBDISP_SW/YU;*/
	{0xB2, 0x4D},		/*R_MATRIX[6:0];*/
	{0xB3, 0x10},		/*B_MATRIX[6:0];*/
	{0xB4, 0xC8},		/*UVG_SEL/BRIGHT_SEL//TEST_YUVM_PE
				/NEG_YMIN_SW/PIC_EFFECT;*/
	{0xB5, 0x5B},		/*CONTRAST[7:0];*/
	{0xB6, 0x4C},		/*BRIGHT[7:0];*/
	{0xB7, 0x00},		/*Y_MIN[7:0];*/
	{0xB8, 0xFF},		/*Y_MAX[7:0];*/
	{0xB9, 0x69},		/*U_GAIN[7:0];*/
	{0xBA, 0x72},		/*V_GAIN[7:0];*/
	{0xBB, 0x78},		/*SEPIA_US[7:0];*/
	{0xBC, 0x90},		/*SEPIA_VS[7:0];*/
	{0xBD, 0x04},		/*U_CORING[7:0];*/
	{0xBE, 0x04},		/*V_CORING[7:0];*/
	{0xBF, 0xC0},
	{0xC0, 0x00},
	{0xC1, 0x00},
	{0xC2, 0x80},		/*ALC_SW/ALC_LOCK*/
	{0xC3, 0x14},		/*MES[7:0];*/
	{0xC4, 0x03},		/*MES[13:8];*/
	{0xC5, 0x00},		/*MDG[7:0];*/
	{0xC6, 0x74},		/*MAG[7:0];*/
	{0xC7, 0x80},		/*AGCONT_SEL[1:0]*/
	{0xC8, 0x10},		/*AG_MIN[7:0];*/
	{0xC9, 0x06},		/*AG_MAX[7:0];*/
	{0xCA, 0x97},		/*AUTO_LES_SW/AUTO_LES_MODE[2:0]
				/ALC_WEIGHT[1:0]/FLCKADJ[1;*/
	{0xCB, 0xD2},		/*ALC_LV[7:0];*/
	{0xCC, 0x10},		/*UPDN_MODE[2:0]/ALC_LV[9:8];*/
	{0xCD, 0x0A},		/*ALC_LVW[7:0];*/
	{0xCE, 0x63},		/*L64P600S[7:0];*/
	{0xCF, 0x06},		/*ALC_VWAIT[2:0]/L64P600S[11:8];*/
	{0xD0, 0x80},		/*UPDN_SPD[7:0];*/
	{0xD1, 0x20},
	{0xD2, 0x80},		/*NEAR_SPD[7:0];*/
	{0xD3, 0x30},
	{0xD4, 0x8A},		/*AC5060//ALC_SAFETY[5:0];*/
	{0xD5, 0x02},		/*ALC_SAFETY2[2:0];*/
	{0xD6, 0x4F},
	{0xD7, 0x08},
	{0xD8, 0x00},
	{0xD9, 0xFF},
	{0xDA, 0x01},
	{0xDB, 0x00},
	{0xDC, 0x14},
	{0xDD, 0x00},
	{0xDE, 0x80},		/*AWB_SW/AWB_LOCK//AWB_TEST
				///HEXG_SLOPE_SEL/COLGATE_SE;*/
	{0xDF, 0x80},		/*WB_MRG[7:0];*/
	{0xE0, 0x80},		/*WB_MGG[7:0];*/
	{0xE1, 0x80},		/*WB_MBG[7:0];*/
	{0xE2, 0x22},		/*WB_RBMIN[7:0];*/
	{0xE3, 0xF8},		/*WB_RBMAX[7:0];*/
	{0xE4, 0x80},		/*HEXA_SW//COLGATE_RANGE[1:0]/
				///COLGATE_OPEN;*/
	{0xE5, 0x2C},		/*RYCUTM[6:0];*/
	{0xE6, 0x54},		/*RYCUTP[6:0];*/
	{0xE7, 0x28},		/*BYCUTM[6:0];*/
	{0xE8, 0x2F},		/*BYCUTP[6:0];*/
	{0xE9, 0xE4},		/*RBCUTL[7:0];*/
	{0xEA, 0x3C},		/*RBCUTH[7:0];*/
	{0xEB, 0x81},		/*SQ1_SW/SQ1_POL//////YGATE_SW;*/
	{0xEC, 0x37},		/*RYCUT1L[7:0];*/
	{0xED, 0x5A},		/*RYCUT1H[7:0];*/
	{0xEE, 0xDE},		/*BYCUT1L[7:0];*/
	{0xEF, 0x08},		/*BYCUT1H[7:0];*/
	{0xF0, 0x18},		/*YGATE_L[7:0];*/
	{0xF1, 0xFE},		/*YGATE_H[7:0];*/
	{0xF2, 0x00},		/*IPIX_DISP_SW*/
	{0xF3, 0x02},
	{0xF4, 0x02},
	{0xF5, 0x04},		/*AWB_WAIT[7:0];*/
	{0xF6, 0x00},		/*AWB_SPDDLY[7:0];*/
	{0xF7, 0x20},		/*//AWB_SPD[5:0];*/
	{0xF8, 0x86},
	{0xF9, 0x00},
	{0xFA, 0x41},		/*MR_HBLK_START[7:0];*/
	{0xFB, 0x50},		/*MR_HBLK_WIDTH[6:0]; /---*/
	{0xFC, 0x0C},		/*MR_VBLK_START[7:0];*/
	{0xFD, 0x3C},		/*MR_VBLK_WIDTH[5:0];/--------*/
	{0xFE, 0x50},		/*PIC_FORMAT[3:0]/PINTEST_SEL[3:0];*/
	{0xFF, 0x85},		/*SLEEP//PARALLEL_OUTSW[1:0]
				/DCLK_POL/DOUT_CBLK_SW//AL;*/
};

static int tcm9001_init(struct i2c_client *client)
{
	int ret = 0;
	int i;
	u32 count = 0;
	struct tcm9001_set *set;
	for (i = 0; i < ARRAY_SIZE(tcm9001_init_regset); i++) {
		set = &tcm9001_init_regset[i];
		count += i;
		tcm9001_reg_write(client, set->addr, set->val);
	}
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */

#define TCM9001_VER_ID 0x4810
static int tcm9001_video_probe(struct soc_camera_device *icd,
			       struct i2c_client *client)
{
	int ret = 0;
	u8 rev, rev1;
	u16 revision;
	struct tcm9001 *tcm9001 = to_tcm9001(client);

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;
	ret = tcm9001_reg_read(client, 0x00, &rev);
	if (ret != 0) {
		printk(KERN_INFO "Read failed stage 1\n");
		return ret;
	}
	ret = tcm9001_reg_read(client, 0x01, &rev1);
	if (ret != 0) {
		printk(KERN_INFO "Read failed stage 2\n");
		return ret;
	}
	revision = (rev << 8) | (rev1);
	printk(KERN_INFO "*************** tcm9001 value	read=%x\n", revision);
	tcm9001->brightness = 0;
	tcm9001->contrast = 0x5B;
	tcm9001->colorlevel = 0;
	tcm9001->framerate = FRAME_RATE_AUTO;
	tcm9001->i_size = TCM9001_SIZE_QVGA;
	return 0;
}

static void tcm9001_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}

static struct v4l2_subdev_core_ops tcm9001_subdev_core_ops = {
	.g_chip_ident = tcm9001_g_chip_ident,
	.g_ctrl = tcm9001_g_ctrl,
	.s_ctrl = tcm9001_s_ctrl,
	.ioctl = tcm9001_ioctl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = tcm9001_g_register,
	.s_register = tcm9001_s_register,
#endif
};

static int tcm9001_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			    enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(tcm9001_fmts))
		return -EINVAL;

	*code = tcm9001_fmts[index].code;
	return 0;
}

static int tcm9001_enum_framesizes(struct v4l2_subdev *sd,
				   struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= TCM9001_SIZE_MAX)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;
	fsize->discrete = tcm9001_frmsizes[fsize->index];
	return 0;
}

/* TCM9001's fps depends on light
   The setting currently is for 15 - 30 fps
   However, due to the limitation at SN65LVDS
   with FSEL=0, nothing more than 10fps can be seen. Parallel
   clock needs to be < 13MHz.
   Also, TCM9001 has a total frame size of 1200*528 pixels
   and hence fps does not change for QVGA */

static int tcm9001_enum_frameintervals(struct v4l2_subdev *sd,
				       struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = tcm9001_find_framesize(interval->width, interval->height);

	switch (size) {
	case TCM9001_SIZE_VGA:
	case TCM9001_SIZE_QVGA:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 30;
		break;
	}
	return 0;
}

static int tcm9001_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (tcm9001->i_size) {
	case TCM9001_SIZE_VGA:
	case TCM9001_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 30;
		break;
	}
	return 0;
}
static int tcm9001_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	cparm = &param->parm.capture;
	/* Implement frame rate control over here */
	return tcm9001_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops tcm9001_subdev_video_ops = {
	.s_stream = tcm9001_s_stream,
	.s_mbus_fmt = tcm9001_s_fmt,
	.g_mbus_fmt = tcm9001_g_fmt,
	.try_mbus_fmt = tcm9001_try_fmt,
	.enum_mbus_fmt = tcm9001_enum_fmt,
	.enum_mbus_fsizes = tcm9001_enum_framesizes,
	.enum_framesizes = tcm9001_enum_framesizes,
	.enum_frameintervals = tcm9001_enum_frameintervals,
	.g_parm = tcm9001_g_parm,
	.s_parm = tcm9001_s_parm,
};
static int tcm9001_g_skip_frames(struct v4l2_subdev *sd, u32 * frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 2;
	return 0;
}

static int tcm9001_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms
			*parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct tcm9001 *tcm9001 = to_tcm9001(client);

	if (!parms)
		return -EINVAL;

	parms->if_type = tcm9001->plat_parms->if_type;
	parms->if_mode = tcm9001->plat_parms->if_mode;
	parms->parms = tcm9001->plat_parms->parms;

	return 0;
}

static struct v4l2_subdev_sensor_ops tcm9001_subdev_sensor_ops = {
	.g_skip_frames = tcm9001_g_skip_frames,
	.g_interface_parms = tcm9001_g_interface_parms,
};

static struct v4l2_subdev_ops tcm9001_subdev_ops = {
	.core = &tcm9001_subdev_core_ops,
	.video = &tcm9001_subdev_video_ops,
	.sensor = &tcm9001_subdev_sensor_ops,	/* Done*/
};

static int tcm9001_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct tcm9001 *tcm9001;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;
	printk(KERN_INFO "TCM9001 mod probe client *********\n");

	if (!icd) {
		dev_err(&client->dev, "tcm9001: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "tcm9001 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"tcm9001 driver needs i/f platform data\n");
		return -EINVAL;
	}

	tcm9001 = kzalloc(sizeof(struct tcm9001), GFP_KERNEL);
	if (!tcm9001)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&tcm9001->subdev, client, &tcm9001_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops = &tcm9001_ops;
	tcm9001->plat_parms = icl->priv;
	ret = tcm9001_video_probe(icd, client);
	return ret;
}

static int tcm9001_remove(struct i2c_client *client)
{
	struct tcm9001 *tcm9001 = to_tcm9001(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	icd->ops = NULL;
	tcm9001_video_remove(icd);
	client->driver = NULL;
	kfree(tcm9001);

	return 0;
}

static const struct i2c_device_id tcm9001_id[] = {
	{"tcm9001", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, tcm9001_id);

static struct i2c_driver tcm9001_i2c_driver = {
	.driver = {
		   .name = "tcm9001",
		   },
	.probe = tcm9001_probe,
	.remove = tcm9001_remove,
	.id_table = tcm9001_id,
};

static int __init tcm9001_mod_init(void)
{
	return i2c_add_driver(&tcm9001_i2c_driver);
}

static void __exit tcm9001_mod_exit(void)
{
	i2c_del_driver(&tcm9001_i2c_driver);
}

module_init(tcm9001_mod_init);
module_exit(tcm9001_mod_exit);

MODULE_DESCRIPTION("Toshiba TCM9001 Camera driver");
MODULE_LICENSE("GPL v2");
