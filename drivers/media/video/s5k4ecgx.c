/*
 * OmniVision S5K4ECGX sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include <s5k4ecgx.h>

//#define FPS_30_MODE		

#define S5K4ECGX_BRIGHTNESS_MIN           0
#define S5K4ECGX_BRIGHTNESS_MAX           200
#define S5K4ECGX_BRIGHTNESS_STEP          100
#define S5K4ECGX_BRIGHTNESS_DEF           100

#define S5K4ECGX_CONTRAST_MIN				0
#define S5K4ECGX_CONTRAST_MAX				200
#define S5K4ECGX_CONTRAST_STEP            100
#define S5K4ECGX_CONTRAST_DEF				100

/* S5K4ECGX has only one fixed colorspace per pixelcode */
struct s5k4ecgx_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};


struct s5k4ecgx_regset {
	u32 size;
	u8 *data;
};

struct s5k4ecgx_regset_table {
	const u32	*reg;
	int		array_size;
};

#define S5K4ECGX_REGSET(x, y)		\
	[(x)] = {					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

#define S5K4ECGX_REGSET_TABLE(y)		\
	{					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}
struct s5k4ecgx_regs {
	struct s5k4ecgx_regset_table init;
	struct s5k4ecgx_regset_table preview;	
};

//@HW temporary code
static const struct s5k4ecgx_regs regs_for_mipi = {
	
	.init = S5K4ECGX_REGSET_TABLE(reg_main_init),
	.preview = S5K4ECGX_REGSET_TABLE(reg_main_preview),
	
};


struct s5k4ecgx_timing_cfg {
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
	u8 isp_scale_down;
	u8 clk_dividers;
	u8 mipi_lanes;
	
};

static const struct s5k4ecgx_datafmt s5k4ecgx_fmts[] = {
	/*
	 * Order important: first natively supported,
	 * second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
};

enum s5k4ecgx_size {
	S5K4ECGX_SIZE_QVGA,       // 320 x 240
	S5K4ECGX_SIZE_VGA,        // 640 x 480
	S5K4ECGX_SIZE_XGA,        // 1024 x 768 
	S5K4ECGX_SIZE_720P,
	S5K4ECGX_SIZE_UXGA,       // 1600 x 1200 (2M)
	S5K4ECGX_SIZE_1080P,
	S5K4ECGX_SIZE_QXGA,       // 2048 x 1536 (3M)
	S5K4ECGX_SIZE_5MP,
	S5K4ECGX_SIZE_LAST,
};

static const struct v4l2_frmsize_discrete s5k4ecgx_frmsizes[S5K4ECGX_SIZE_LAST] = {
	{  320,  240 },
	{  640,  480 },
	{ 1024,  768 },
	{ 1280,  720 },
	{ 1600, 1200 },
	{ 1920, 1080 },
	{ 2048, 1536 },
	{ 2592, 1944 },
};
	

/* Find a data format by a pixel code in an array */
static int s5k4ecgx_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(s5k4ecgx_fmts); i++)
		if (s5k4ecgx_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(s5k4ecgx_fmts))
		i = ARRAY_SIZE(s5k4ecgx_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int s5k4ecgx_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < S5K4ECGX_SIZE_LAST; i++) {
		if ((s5k4ecgx_frmsizes[i].width >= width) &&
		    (s5k4ecgx_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= S5K4ECGX_SIZE_LAST)
		i = S5K4ECGX_SIZE_LAST - 1;

	return i;
}

struct s5k4ecgx {
	struct v4l2_subdev subdev;
//	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
};

static struct s5k4ecgx *to_s5k4ecgx(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct s5k4ecgx, subdev);
}

/**
 * struct s5k4ecgx_reg - s5k4ecgx register format
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 * @length: length of the register
 *
 * Define a structure for S5K4ECGX register initialization values
 */
struct s5k4ecgx_reg {
	u16	reg;
	u8	val;
};

/* TODO: Divide this properly */
static const struct s5k4ecgx_reg configscript_common1[] = {
	//@HW Fixed me, dummy
   { 0xFFFF, 0x00 }
};

static const struct s5k4ecgx_timing_cfg timing_cfg[S5K4ECGX_SIZE_LAST] = {
	[S5K4ECGX_SIZE_QVGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 320,
		.v_output_size = 240,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
    #ifdef FPS_30_MODE		
		.h_total_size = 2176,
		.v_total_size = 1632,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 3,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 3,
    #else		
		.h_total_size = 2844,
		.v_total_size = 1968,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    #endif		
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_VGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 640,
		.v_output_size = 480,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
    #ifdef FPS_30_MODE		
		.h_total_size = 2176,
		.v_total_size = 1632,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 3,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 3,
    #else		
		.h_total_size = 2844,
		.v_total_size = 1968,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    #endif		
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_XGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 1024,
		.v_output_size = 768,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_720P] = {
		.x_addr_start = 336,
		.y_addr_start = 434,
		.x_addr_end = 2287,
		.y_addr_end = 1522,
		.h_output_size = 1280,
		.v_output_size = 720,
		.h_total_size = 2500,
		.v_total_size = 1120,
		.isp_h_offset = 16,
		.isp_v_offset = 4,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x01,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_UXGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 1600,
		.v_output_size = 1200,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x02,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_1080P] = {
		.x_addr_start = 336,
		.y_addr_start = 434,
		.x_addr_end = 2287,
		.y_addr_end = 1522,
		.h_output_size = 1920,
		.v_output_size = 1080,
		.h_total_size = 2500,
		.v_total_size = 1120,
		.isp_h_offset = 16,
		.isp_v_offset = 4,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x03,
    	.clk_dividers = 0x02,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_QXGA] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 2048,
		.v_output_size = 1536,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x23,
    	.clk_dividers = 0x02,
    	.mipi_lanes = 0x25,
	},
	[S5K4ECGX_SIZE_5MP] = {
		.x_addr_start = 0,
		.y_addr_start = 0,
		.x_addr_end = 2623,
		.y_addr_end = 1951,
		.h_output_size = 2592,
		.v_output_size = 1944,
		.h_total_size = 2844,
		.v_total_size = 1968,
		.isp_h_offset = 16,
		.isp_v_offset = 6,
		.h_odd_ss_inc = 1,
		.h_even_ss_inc = 1,
		.v_odd_ss_inc = 1,
		.v_even_ss_inc = 1,
    	.isp_scale_down = 0x03,
    	.clk_dividers = 0x02,
    	.mipi_lanes = 0x25,
	},
};


/**
 * s5k4ecgx_reg_read - Read a value from a register in an s5k4ecgx sensor device
 * @client: i2c driver client structure
 * @reg: register address / offset
 * @val: stores the value that gets read
 *
 * Read a value from a register in an s5k4ecgx sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int s5k4ecgx_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	#if 0 //@HW, dummy code
	int ret;
	u8 data[2] = {0};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= data,
	};

	data[0] = (u8)(reg >> 8);
	data[1] = (u8)(reg & 0xff);

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
	#endif
	return 0;
}


#define POLL_TIME_MS 10
/**
 * s5k4ecgx_i2c_read_twobyte: Read 2 bytes from sensor
 */
static int s5k4ecgx_i2c_read_twobyte(struct i2c_client *client,
				  u16 subaddr, u16 *data)
{
	int err;
	unsigned char buf[2];
	struct i2c_msg msg[2];

	cpu_to_be16s(&subaddr);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = (u8 *)&subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = buf;

	err = i2c_transfer(client->adapter, msg, 2);
	if (unlikely(err != 2)) {
		printk("%s: register read fail\n", __func__);
		return -EIO;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}

/**
 * s5k4ecgx_i2c_write_twobyte: Write (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @cmd: command register
 * @w_data: data to be written
 * @w_len: length of data to be written
 *
 * Returns 0 on success, <0 on error
 */
static int s5k4ecgx_i2c_write_twobyte(struct i2c_client *client,
					 u16 addr, u16 w_data)
{
	int retry_count = 5;
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 4, buf};
	int ret = 0;

	buf[0] = addr >> 8;
	buf[1] = addr;
	buf[2] = w_data >> 8;
	buf[3] = w_data & 0xff;

//	printk("%s : W(0x%02X%02X%02X%02X)\n",
//		__func__, buf[0], buf[1], buf[2], buf[3]);

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
		printk("%s: I2C err %d, retry %d.\n",
			__func__, ret, retry_count);
	} while (retry_count-- > 0);
	if (ret != 1) {
		printk( "%s: I2C is not working.\n", __func__);
		return -EIO;
	}

	return 0;
}


static int s5k4ecgx_write_regs(struct i2c_client *client, const u32 regs[],
			     int size)
{
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		err = s5k4ecgx_i2c_write_twobyte(client,
			(regs[i] >> 16), regs[i]);
		if (unlikely(err != 0)) {
			printk(
				"%s: register write failed\n", __func__);
			return err;
		}
	}

	return 0;
}

static int s5k4ecgx_set_from_table(struct i2c_client *client,
				const char *setting_name,
				const struct s5k4ecgx_regset_table *table,
				int table_size, int index)
{
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk( "%s: set %s index %d\n",
		__func__, setting_name, index);
	if ((index < 0) || (index >= table_size)) {
		printk(
			"%s: index(%d) out of range[0:%d] for table for %s\n",
			__func__, index, table_size, setting_name);
		return -EINVAL;
	}
	table += index;
	if (table->reg == NULL)
		return -EINVAL;
	return s5k4ecgx_write_regs(client, table->reg, table->array_size);
}

/**
 * Write a value to a register in s5k4ecgx sensor device.
 * @client: i2c driver client structure.
 * @reg: Address of the register to read value from.
 * @val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int s5k4ecgx_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	#if 0 //@HW, dummy code
	int ret;
	unsigned char data[3] = { (u8)(reg >> 8), (u8)(reg & 0xff), val };
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}
#endif
	return 0;
}

static const struct v4l2_queryctrl s5k4ecgx_controls[] = {
	{
		.id      	= V4L2_CID_BRIGHTNESS,
		.type    	= V4L2_CTRL_TYPE_INTEGER,
		.name    	= "Brightness",
		.minimum	= S5K4ECGX_BRIGHTNESS_MIN,
		.maximum	= S5K4ECGX_BRIGHTNESS_MAX,
		.step		= S5K4ECGX_BRIGHTNESS_STEP,
		.default_value	= S5K4ECGX_BRIGHTNESS_DEF,
	},
	{
		.id		= V4L2_CID_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.minimum	= S5K4ECGX_CONTRAST_MIN,
		.maximum	= S5K4ECGX_CONTRAST_MAX,
		.step		= S5K4ECGX_CONTRAST_STEP,
		.default_value	= S5K4ECGX_CONTRAST_DEF,
	},
	{
		.id		= V4L2_CID_COLORFX,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Color Effects",
		.minimum	= V4L2_COLORFX_NONE,
		.maximum	= V4L2_COLORFX_NEGATIVE,
		.step		= 1,
		.default_value	= V4L2_COLORFX_NONE,
	},
};

/**
 * Initialize a list of s5k4ecgx registers.
 * The list of registers is terminated by the pair of values
 * @client: i2c driver client structure.
 * @reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int s5k4ecgx_reg_writes(struct i2c_client *client,
			     const struct s5k4ecgx_reg reglist[])
{
	#if 0 //@HW, dummy code
	int err = 0, index;

    for (index=0; ((reglist[index].reg != 0xFFFF) && (err == 0)); index++)
    {
        err |= s5k4ecgx_reg_write( client, reglist[index].reg, reglist[index].val );
    // Check for Pause condition
        if ( (reglist[index+1].reg == 0xFFFF) && (reglist[index+1].val != 0) )
        {
            msleep(reglist[index+1].val);
            index += 1;
       }
    }
	#endif
	return 0;
}

static int s5k4ecgx_reg_set(struct i2c_client *client, u16 reg, u8 val)
{
	#if 0 //@HW, dummy code
	int ret;
	u8 tmpval = 0;

	ret = s5k4ecgx_reg_read(client, reg, &tmpval);
	if (ret)
		return ret;

	return s5k4ecgx_reg_write(client, reg, tmpval | val);
	#endif 
	return 0;
}

static int s5k4ecgx_reg_clr(struct i2c_client *client, u16 reg, u8 val)
{
	
	int ret;
	u8 tmpval = 0;

	ret = s5k4ecgx_reg_read(client, reg, &tmpval);
	if (ret)
		return ret;

	return s5k4ecgx_reg_write(client, reg, tmpval & ~val);
}

static int s5k4ecgx_config_timing(struct i2c_client *client)
{
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	int ret, i = s5k4ecgx->i_size;
	printk("s5k4ecgx_config_timing empty E, Warning\n");


	return 0;
}

static int s5k4ecgx_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = 0;
	printk("s5k4ecgx_s_stream E\n");
	if (s5k4ecgx_set_from_table(client, "init",
					&regs_for_mipi.init, 1, /*ARRAY_SIZE(regs_for_mipi.init),*/
					 0) < 0)
	{
		printk(" init register failure\n");
	}
	

	if (s5k4ecgx_set_from_table(client, "preview",
					&regs_for_mipi.preview, 1,
					 0) < 0)
	{
		printk(" preview register failure\n");
	}

	//@HW, TO do, add real function for samsung sensor
	printk("s5k4ecgx_s_stream X\n");
	return ret;
}

static int s5k4ecgx_set_bus_param(struct soc_camera_device *icd,
				 unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	printk("s5k4ecgx_set_bus_param E\n");
	printk("s5k4ecgx_set_bus_param X\n");
	return 0;
}

static unsigned long s5k4ecgx_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	printk("s5k4ecgx_set_bus_param E\n");

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;
	printk("s5k4ecgx_set_bus_param X\n");

	return flags;
}

static int s5k4ecgx_g_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	printk("s5k4ecgx_g_fmt E\n");

	mf->width	= s5k4ecgx_frmsizes[s5k4ecgx->i_size].width;
	mf->height	= s5k4ecgx_frmsizes[s5k4ecgx->i_size].height;
	mf->code	= s5k4ecgx_fmts[s5k4ecgx->i_fmt].code;
	mf->colorspace	= s5k4ecgx_fmts[s5k4ecgx->i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;
	printk("s5k4ecgx_g_fmt X\n");

	return 0;
}

static int s5k4ecgx_try_fmt(struct v4l2_subdev *sd,
			   struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;
	printk("s5k4ecgx_try_fmt E\n");

	i_fmt = s5k4ecgx_find_datafmt(mf->code);

	mf->code = s5k4ecgx_fmts[i_fmt].code;
	mf->colorspace	= s5k4ecgx_fmts[i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;

	i_size = s5k4ecgx_find_framesize(mf->width, mf->height);

	mf->width = s5k4ecgx_frmsizes[i_size].width;
	mf->height = s5k4ecgx_frmsizes[i_size].height;
	printk("s5k4ecgx_try_fmt X\n");

	return 0;
}

static int s5k4ecgx_s_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	int ret = 0;
	u8 fmtreg = 0, fmtmuxreg = 0;
	printk("s5k4ecgx_s_fmt E\n");

	ret = s5k4ecgx_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	s5k4ecgx->i_size = s5k4ecgx_find_framesize(mf->width, mf->height);
	s5k4ecgx->i_fmt = s5k4ecgx_find_datafmt(mf->code);

	switch ((u32)s5k4ecgx_fmts[s5k4ecgx->i_fmt].code) {
	case V4L2_MBUS_FMT_UYVY8_2X8:
		fmtreg = 0x32;
		fmtmuxreg = 0;
		break;
	case V4L2_MBUS_FMT_YUYV8_2X8:
		fmtreg = 0x30;
		fmtmuxreg = 0;
		break;
	default:
		/* This shouldn't happen */
		ret = -EINVAL;
		return ret;
	}

// Configure common settings
	ret = s5k4ecgx_reg_writes(client, configscript_common1);
	if (ret)
		return ret;

	ret = s5k4ecgx_reg_write(client, 0x4300, fmtreg);
	if (ret)
		return ret;

	ret = s5k4ecgx_reg_write(client, 0x501F, fmtmuxreg);
	if (ret)
		return ret;

	ret = s5k4ecgx_config_timing(client);
	if (ret)
		return ret;
	printk("s5k4ecgx_s_fmt X\n");
	return ret;
}

static int s5k4ecgx_g_chip_ident(struct v4l2_subdev *sd,
				struct v4l2_dbg_chip_ident *id)
{
	printk("s5k4ecgx_g_chip_ident E\n");
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident	= V4L2_IDENT_S5K4ECGX;
	id->revision	= 0;
	printk("s5k4ecgx_g_chip_ident X\n");

	return 0;
}

static int s5k4ecgx_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	printk("s5k4ecgx_g_ctrl E\n");

	dev_dbg(&client->dev, "s5k4ecgx_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = s5k4ecgx->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = s5k4ecgx->contrast;
		break;
	case V4L2_CID_COLORFX:
		ctrl->value = s5k4ecgx->colorlevel;
		break;
	}
	printk("s5k4ecgx_g_ctrl X\n");
	return 0;
}

static int s5k4ecgx_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	int i_sde_ctrl0 = 0;
	int i_sde_ctrl3 = 0, i_sde_ctrl4 = 0;
	int i_sde_ctrl5 = 0, i_sde_ctrl6 = 0;
	int i_sde_ctrl7 = 0, i_sde_ctrl8 = 0;
	int ret = 0;
	printk("s5k4ecgx_s_ctrl E\n");

	dev_dbg(&client->dev, "s5k4ecgx_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:

		if (ctrl->value > S5K4ECGX_BRIGHTNESS_MAX)
			return -EINVAL;

		s5k4ecgx->brightness = ctrl->value;
		ret = s5k4ecgx_reg_write(client, 0x5580, 0x04);
		if (ret)
			return ret;
		switch(s5k4ecgx->brightness) {
		case S5K4ECGX_BRIGHTNESS_MIN:
			i_sde_ctrl7 = 0x40;
			i_sde_ctrl8 = 0x08;
		break;
		case S5K4ECGX_BRIGHTNESS_MAX:
			i_sde_ctrl7 = 0x40;
			i_sde_ctrl8 = 0x00;
		break;
		default:
			i_sde_ctrl7 = 0x00;
			i_sde_ctrl8 = 0x00;
		break;
		}
		ret = s5k4ecgx_reg_write(client, 0x5587, i_sde_ctrl7);
		if (ret)
			return ret;

		ret = s5k4ecgx_reg_write(client, 0x5588, i_sde_ctrl8);
		if (ret)
			return ret;
		break;
	case V4L2_CID_CONTRAST:

		if (ctrl->value > S5K4ECGX_CONTRAST_MAX)
			return -EINVAL;

		s5k4ecgx->contrast = ctrl->value;
		ret = s5k4ecgx_reg_write(client, 0x5580, 0x04);
		if (ret)
			return ret;
		switch(s5k4ecgx->contrast) {
		case S5K4ECGX_CONTRAST_MIN:
			i_sde_ctrl5 = 0x10;
			i_sde_ctrl6 = 0x10;
			i_sde_ctrl8 = 0x00;
		break;
		case S5K4ECGX_CONTRAST_MAX:
			i_sde_ctrl5 = 0x20;
			i_sde_ctrl6 = 0x30;
			i_sde_ctrl8 = 0x08;
		break;
		default:
			i_sde_ctrl5 = 0x00;
			i_sde_ctrl6 = 0x20;
			i_sde_ctrl8 = 0x00;
		break;
		}

		ret = s5k4ecgx_reg_write(client, 0x5585, i_sde_ctrl5);
		if (ret)
			return ret;

		ret = s5k4ecgx_reg_write(client, 0x5586, i_sde_ctrl6);
		if (ret)
			return ret;

		ret = s5k4ecgx_reg_write(client, 0x5588, i_sde_ctrl8);
		if (ret)
			return ret;
		break;
	case V4L2_CID_COLORFX:
		if (ctrl->value > V4L2_COLORFX_NEGATIVE)
			return -EINVAL;

		s5k4ecgx->colorlevel = ctrl->value;

		switch (s5k4ecgx->colorlevel) {
		case V4L2_COLORFX_BW:
			i_sde_ctrl0 = 0x18;
			i_sde_ctrl3 = 0x80;
			i_sde_ctrl4 = 0x80;
			break;
		case V4L2_COLORFX_SEPIA:
			i_sde_ctrl0 = 0x18;
			i_sde_ctrl3 = 0x40;
			i_sde_ctrl4 = 0xA0;
			break;
		case V4L2_COLORFX_NEGATIVE:
			i_sde_ctrl0 = 0x40;
			i_sde_ctrl3 = 0x00;
			i_sde_ctrl4 = 0x00;
			break;
		default:
			i_sde_ctrl0 = 0x00;
			i_sde_ctrl3 = 0x00;
			i_sde_ctrl4 = 0x00;
			break;
		}

		ret = s5k4ecgx_reg_write(client, 0x5580, i_sde_ctrl0);
		if (ret)
			return ret;

		ret = s5k4ecgx_reg_write(client, 0x5583, i_sde_ctrl3);
		if (ret)
			return ret;

		ret = s5k4ecgx_reg_write(client, 0x5584, i_sde_ctrl4);
		if (ret)
			return ret;
		break;
	}
	printk("s5k4ecgx_s_ctrl X\n");

	return ret;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int s5k4ecgx_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("s5k4ecgx_g_register E\n");

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (s5k4ecgx_reg_read(client, reg->reg, &reg->val))
		return -EIO

	printk("s5k4ecgx_g_register X\n");

	return 0;
}

static int s5k4ecgx_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("s5k4ecgx_s_register E\n");

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (s5k4ecgx_reg_write(client, reg->reg, reg->val))
		return -EIO;
	printk("s5k4ecgx_s_register X\n");

	return 0;
}
#endif

static struct soc_camera_ops s5k4ecgx_ops = {
	.set_bus_param		= s5k4ecgx_set_bus_param,
	.query_bus_param	= s5k4ecgx_query_bus_param,
	.controls		= s5k4ecgx_controls,
	.num_controls		= ARRAY_SIZE(s5k4ecgx_controls),
};

static int s5k4ecgx_init(struct i2c_client *client)
{
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	int ret = 0;
	printk("s5k4ecgx_init E\n");
	printk("haipeng, write init table\n");

	/*
	if (s5k4ecgx_set_from_table(client, "init reg 1",
					regs_for_mipi.init, ARRAY_SIZE(regs_for_mipi.init),
					 0) < 0)
	{
		printk("s5k4ecgx_init, write i2c fail\n");
		return -EIO;
	}	
	*/
	printk("s5k4ecgx_init X\n");
	return 0;


}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int s5k4ecgx_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	unsigned long flags;
	int ret = 0;
	u8 revision = 0;
	printk("s5k4ecgx_video_probe E\n");

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	ret = s5k4ecgx_reg_read(client, 0x302A, &revision);
	if (ret) {
		dev_err(&client->dev, "Failure to detect S5K4ECGX chip\n");
		goto out;
	}
	printk(KERN_ERR "S5K4ECGX value read=%x\n", revision);

	revision &= 0xF;

	flags = SOCAM_DATAWIDTH_8;

	dev_info(&client->dev, "Detected a S5K4ECGX chip, revision %x\n",
		 revision);
	printk("s5k4ecgx_video_probe X\n");

	/* TODO: Do something like s5k4ecgx_init */

out:
	return ret;
}

static void s5k4ecgx_video_remove(struct soc_camera_device *icd)
{
	printk("s5k4ecgx_video_remove E\n");
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
	printk("s5k4ecgx_video_remove X\n");
}

static struct v4l2_subdev_core_ops s5k4ecgx_subdev_core_ops = {
	.g_chip_ident	= s5k4ecgx_g_chip_ident,
	.g_ctrl		= s5k4ecgx_g_ctrl,
	.s_ctrl 	= s5k4ecgx_s_ctrl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= s5k4ecgx_g_register,
	.s_register	= s5k4ecgx_s_register,
#endif
};

static int s5k4ecgx_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			    enum v4l2_mbus_pixelcode *code)
{
	printk("s5k4ecgx_enum_fmt E\n");
	if (index >= ARRAY_SIZE(s5k4ecgx_fmts))
		return -EINVAL;

	*code = s5k4ecgx_fmts[index].code;
	printk("s5k4ecgx_enum_fmt X\n");
	return 0;
}

static int s5k4ecgx_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	printk("s5k4ecgx_enum_framesizes E\n");
	if (fsize->index > S5K4ECGX_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = s5k4ecgx_frmsizes[fsize->index];
	printk("s5k4ecgx_enum_framesizes E\n");

	return 0;
}

static int s5k4ecgx_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	struct v4l2_captureparm *cparm;
	printk("s5k4ecgx_g_parm E\n");
	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (s5k4ecgx->i_size) {
	case S5K4ECGX_SIZE_5MP:
		cparm->timeperframe.numerator = 2;
		cparm->timeperframe.denominator = 15;
		break;
	case S5K4ECGX_SIZE_QXGA:
	case S5K4ECGX_SIZE_1080P:
	case S5K4ECGX_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case S5K4ECGX_SIZE_720P:
	case S5K4ECGX_SIZE_XGA:
	case S5K4ECGX_SIZE_VGA:
	case S5K4ECGX_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}
	printk("s5k4ecgx_g_parm X\n");

	return 0;
}
static int s5k4ecgx_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 * flexible enough.
	 */
	 printk("s5k4ecgx_s_parm E\n");
	 printk("s5k4ecgx_s_parm X\n");
	return s5k4ecgx_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops s5k4ecgx_subdev_video_ops = {
	.s_stream	= s5k4ecgx_s_stream,
	.s_mbus_fmt	= s5k4ecgx_s_fmt,
	.g_mbus_fmt	= s5k4ecgx_g_fmt,
	.try_mbus_fmt	= s5k4ecgx_try_fmt,
	.enum_mbus_fmt	= s5k4ecgx_enum_fmt,
	.enum_framesizes = s5k4ecgx_enum_framesizes,
	.g_parm = s5k4ecgx_g_parm,
	.s_parm = s5k4ecgx_s_parm,
};
static int s5k4ecgx_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 5;

	return 0;
}


static struct v4l2_subdev_sensor_ops s5k4ecgx_subdev_sensor_ops = {
	.g_skip_frames	= s5k4ecgx_g_skip_frames,
//	.g_interface_parms = s5k4ecgx_g_interface_parms,
};

static struct v4l2_subdev_ops s5k4ecgx_subdev_ops = {
	.core	= &s5k4ecgx_subdev_core_ops,
	.video	= &s5k4ecgx_subdev_video_ops,
	.sensor	= &s5k4ecgx_subdev_sensor_ops,
};

static int s5k4ecgx_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct s5k4ecgx *s5k4ecgx;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret;
	printk("s5k4ecgx_probe E\n");

	if (!icd) {
		dev_err(&client->dev, "S5K4ECGX: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "S5K4ECGX driver needs platform data\n");
		return -EINVAL;
	}

/*	if (!icl->priv) {
		dev_err(&client->dev,
			"S5K4ECGX driver needs i/f platform data\n");
		return -EINVAL;
	}*/

	s5k4ecgx = kzalloc(sizeof(struct s5k4ecgx), GFP_KERNEL);
	if (!s5k4ecgx)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&s5k4ecgx->subdev, client, &s5k4ecgx_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	icd->ops		= &s5k4ecgx_ops;

	s5k4ecgx->i_size = S5K4ECGX_SIZE_VGA;
	s5k4ecgx->i_fmt = 0; /* First format in the list */
//	s5k4ecgx->plat_parms = icl->priv;

	ret = s5k4ecgx_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(s5k4ecgx);
		return ret;
	}

	/* init the sensor here */
	ret = s5k4ecgx_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}
	printk("s5k4ecgx_probe X\n");

	return ret;
}

static int s5k4ecgx_remove(struct i2c_client *client)
{
	struct s5k4ecgx *s5k4ecgx = to_s5k4ecgx(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	printk("s5k4ecgx_remove E\n");

	icd->ops = NULL;
	s5k4ecgx_video_remove(icd);
	client->driver = NULL;
	kfree(s5k4ecgx);
	printk("s5k4ecgx_remove X\n");

	return 0;
}

static const struct i2c_device_id s5k4ecgx_id[] = {
	{ "s5k4ecgx", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, s5k4ecgx_id);

static struct i2c_driver s5k4ecgx_i2c_driver = {
	.driver = {
		.name = "s5k4ecgx",
	},
	.probe		= s5k4ecgx_probe,
	.remove		= s5k4ecgx_remove,
	.id_table	= s5k4ecgx_id,
};

static int __init s5k4ecgx_mod_init(void)
{
	return i2c_add_driver(&s5k4ecgx_i2c_driver);
}

static void __exit s5k4ecgx_mod_exit(void)
{
	i2c_del_driver(&s5k4ecgx_i2c_driver);
}

module_init(s5k4ecgx_mod_init);
module_exit(s5k4ecgx_mod_exit);

MODULE_DESCRIPTION("OmniVision S5K4ECGX Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
