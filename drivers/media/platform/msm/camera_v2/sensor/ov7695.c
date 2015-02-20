/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#define OV7695_SENSOR_NAME "ov7695"
#define PLATFORM_DRIVER_NAME "msm_camera_ov7695"
#define ov7695_obj ov7695_##obj

#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif
static ssize_t ov7695_read_id_attr(struct device *dev,struct device_attribute *attr, char *buf);
static DEVICE_ATTR(read_id, 0664, ov7695_read_id_attr, NULL);
static ssize_t ov7695_read_version_attr(struct device *dev,struct device_attribute *attr, char *buf);
static DEVICE_ATTR(read_version, 0664, ov7695_read_version_attr, NULL);
DEFINE_MSM_MUTEX(ov7695_mut);
static struct msm_sensor_ctrl_t ov7695_s_ctrl;

static struct msm_sensor_power_setting ov7695_power_setting[] = {
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,  ///only USE for i2c pull high
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct msm_camera_i2c_reg_conf ov7695_recommend_settings[] = {
	{0x0103 ,0x01},
	{0x3620 ,0x2f},
	{0x3623 ,0x12},
	{0x3718 ,0x88},
	{0x3703 ,0x80},
	{0x3712 ,0x40},
	{0x3706 ,0x40},
	{0x3631 ,0x44},
	{0x3632 ,0x05},
	{0x3013 ,0xd0},
	{0x3705 ,0x1d},
	{0x3713 ,0x0e},
	{0x3012 ,0x0a},
	{0x3717 ,0x18},// 19
	{0x3621 ,0x47},// 44
	{0x0309 ,0x24},
	{0x3820 ,0x90},
	{0x4803 ,0x08},
	{0x0101 ,0x01},
	{0x5100 ,0x01},

	{0x4500 ,0x24},//25

    //gamma OK
	{0x5301 ,0x05},
	{0x5302 ,0x0c},
	{0x5303 ,0x1c},
	{0x5304 ,0x2a},
	{0x5305 ,0x39},
	{0x5306 ,0x45},
	{0x5307 ,0x52},
	{0x5308 ,0x5d},
	{0x5309 ,0x68},
	{0x530a ,0x7f},
	{0x530b ,0x91},
	{0x530c ,0xa5},
	{0x530d ,0xc6},
	{0x530e ,0xde},
	{0x530f ,0xef},
	{0x5310 ,0x16},
	{0x520a ,0xf4},
	{0x520b ,0xf4},
	{0x520c ,0xf4},
	{0x5504 ,0x08},
	{0x5505 ,0x48},
	{0x5506 ,0x07},
	{0x5507 ,0x0b},
	{0x3a18 ,0x01},
	{0x3a19 ,0x00},
	{0x3503 ,0x03},
	{0x3500 ,0x00},
	{0x3501 ,0x21},
	{0x3502 ,0x00},
	{0x350a ,0x00},
	{0x350b ,0x00},
	{0x4008 ,0x02},
	{0x4009 ,0x09},
	{0x3002 ,0x09},
	{0x3024 ,0x00},
	{0x3503 ,0x00},
	{0x0101 ,0x01}, //mirror_on
	{0x5002 ,0x48}, //[7:6] Y source select// [3]LENC bias plus
	{0x5910 ,0x00}, //Y formula
	{0x3a0f ,0x58},
	{0x3a10 ,0x50}, //38 //AEC target
	{0x3a1b ,0x5a}, //40
	{0x3a1e ,0x4e}, //36
	{0x3a11 ,0xa0}, //80
	{0x3a1f ,0x28}, //18
	{0x3a18 ,0x00}, //
	{0x3a19 ,0xe0}, //f8; max gain 15.5x
	{0x3503 ,0x00}, //aec/agc
	{0x3a0d ,0x04},//03//60Hz Max Band Step

	{0x5000 ,0xff}, //lcd,gma,awb,awbg,bc,wc,lenc,isp
	{0x5001 ,0x3f}, //avg, blc,sde,uv_avg,cmx, cip
	{0x5100 ,0x01},
	{0x5101 ,0x48},
	{0x5102 ,0x00},
	{0x5103 ,0xf8},
	{0x5104 ,0x04},
	{0x5105 ,0x00},
	{0x5106 ,0x00},
	{0x5107 ,0x00},
	//G
	{0x5108 ,0x01},
	{0x5109 ,0x48},
	{0x510a ,0x00},
	{0x510b ,0xf8},
	{0x510c ,0x03},
	{0x510d ,0x00},
	{0x510e ,0x00},
	{0x510f ,0x00},
	//B
	{0x5110 ,0x01},
	{0x5111 ,0x48},
	{0x5112 ,0x00},
	{0x5113 ,0xf8},
	{0x5114 ,0x03},
	{0x5115 ,0x00},
	{0x5116 ,0x00},
	{0x5117 ,0x00},

	//AWB
	{0x520a ,0xf4},
	{0x520b ,0xf4},
    {0x520c ,0xb4}, //94,f4
    {0x5004 ,0x45},
	{0x5006 ,0x41},
    
	//@@ Gamma
	{0x5301 ,0x05},
	{0x5302 ,0x0c},
	{0x5303 ,0x1c},
	{0x5304 ,0x2a},
	{0x5305 ,0x39},
	{0x5306 ,0x45},
	{0x5307 ,0x53},
	{0x5308 ,0x5d},
	{0x5309 ,0x68},
	{0x530a ,0x7f},
	{0x530b ,0x91},
	{0x530c ,0xa5},
	{0x530d ,0xc6},
	{0x530e ,0xde},
	{0x530f ,0xef},
	{0x5310 ,0x16},

	//sharpen/denoise
        {0x5003 ,0x80},
        {0x5500 ,0x08},
        {0x5501 ,0x1a}, //48, detect gain
        {0x5502 ,0x38},
        {0x5503 ,0x14},
        {0x5504 ,0x08},
        {0x5505 ,0x48},
        {0x5506 ,0x02}, //0a,02
        {0x5507 ,0x10}, //66,15,12,16
        {0x5508 ,0x2d},
        {0x5509 ,0x08},
        {0x550a ,0x48},
        {0x550b ,0x06},
        {0x550c ,0x04},
        {0x550d ,0x01}, //00,01

	//SDE, for saturation 120% under D65
	{0x5800 ,0x02},
	{0x5803 ,0x2e},//40
	{0x5804 ,0x20},//34

	//@@ CMX QE
	{0x5600 ,0x00},
	{0x5601 ,0x2e}, 
	{0x5602 ,0x60},
	{0x5603 ,0x06},
    
    
	{0x560a ,0x01},
	{0x560b ,0x9c},

	{0x3811 ,0x07},// Tradeoff position to make YUV/RAW x VGA/QVGA x Mirror/Flip all work
	{0x3813 ,0x06},

	{0x3630 ,0x79},//69// ADC7
    {0x5000,0xff},//  ;Lens enable[1]
    //;R
    {0x5100,0x01},//  ;[1:0] R_X[9:8]
    {0x5101,0x48},//  ;R_X [7:0]
    {0x5102,0x00},//  ;[1:0] R_Y[10:8]
    {0x5103,0xf8},//  ;R_Y [7:0]
    {0x5104,0x02},//  ;R_A1 [6:0]       //0x04
    {0x5105,0x00},//  ;R_A2[3:0]	
    {0x5106,0x00},//  ;R_B1 [7:0]        
    {0x5107,0x00},//  ;R_B2 [3:0]	
    //;G
    {0x5108,0x01},//  ;[1:0] G_X[9:8]
    {0x5109,0x48},//  ;G_X [7:0]
    {0x510A,0x00},//  ;[1:0] G_Y[9:8]
    {0x510B,0xf8},//  ;G_Y [7:0]
    {0x510C,0x02},//;03  ;G_A1 [6:0]
    {0x510D,0x00},//  ;G_A2[3:0]	
    {0x510E,0x01},//;00  ;G_B1 [7:0]        
    {0x510F,0x00},//  ;G_B2 [3:0]	
    //;B
    {0x5110,0x01},//  ;[1:0] B_X[9:8]
    {0x5111,0x48},//  ;B_X [7:0]
    {0x5112,0x00},//  ;[1:0] B_Y[9:8]
    {0x5113,0xf8},//  ;B_Y [7:0]
    {0x5114,0x02},//;03  ;B_A1 [6:0]
    {0x5115,0x00},//  ;B_A2[3:0]	
    {0x5116,0x00},//  ;B_B1 [7:0]        
    {0x5117,0x00},//  ;B_B2 [3:0]
    {0x3a11,0x88},

    {0x3a1b,0x42},    
    {0x3a0f,0x40},
    {0x3a10,0x38},
    {0x3a1e,0x36},
    
    {0x3a1f,0x10},
    {0x5604,0x1c}, 
    {0x5605,0x65}, 
    {0x5606,0x81}, 
    {0x5607,0x9f}, 
    {0x5608,0x8a}, 
    {0x5609,0x15}, 
    {0x5602,0x60},//; 20140128-yuv ctx
    {0x5803,0x2c},//;29; main
    {0x5804,0x23},//; second
    {0x5601,0x30},//;2c
    {0x5602,0x60},//
    {0x5104 ,0x04},//;02,04
    {0x510C,0x00},//;02
    {0x5114,0x00},//;02
    {0x5105,0x01},//;00
    {0x510d,0x01},//;00
    {0x5115,0x01},//;00
    {0x5803,0x2c},//;26
    {0x5804,0x24},//;20
    {0x5908 ,0x22},   //62  
    {0x5909 ,0x22},   //26
    {0x590a ,0xe2},   //e6
    {0x590b ,0x2e},   //6e
    {0x590c ,0xe2},   //ea
    {0x590d ,0x2e},   //ae
    {0x590e ,0x22},   //a6
    {0x590f ,0x22},   //6a
    {0x4003 ,0x08},
    {0x5502 ,0x0A},//;1c,38;20;18 ;sharp mt offset1
    {0x5503 ,0x04},//;06,14;08;04 ;sharp mt offset2
    {0x5310 ,0x28},   //16
    {0x5301 ,0x05},   
    {0x5302 ,0x09},   
    {0x5303 ,0x15},   
    {0x5304 ,0x20},   
    {0x5305 ,0x31},   
    {0x5306 ,0x3f},   
    {0x5307 ,0x50},   //51  
    {0x5308 ,0x5b},   //5c
    {0x5309 ,0x65},   //67
    {0x530a ,0x77},   //7f
    {0x530b ,0x83},   //91
    {0x530c ,0x91},   //a5
    {0x530d ,0xad},   //c6
    {0x530e ,0xcb},   //de
    {0x530f ,0xe2},   //ef
    {0x5200,0x20}, //  MWB
    {0x5204,0x04},//R Gain
    {0x5205,0xA0},  
    {0x5206,0x04},//G Gain
    {0x5207,0x00},  
    {0x5208,0x07},//B Gain
    {0x5209,0x8e},

};
static struct msm_camera_i2c_reg_conf ov7695_stop[] = {
	{0x100 ,0x01},
};
static struct msm_camera_i2c_reg_conf ov7695_start[] = {
	{0x100 ,0x01},
};
static struct v4l2_subdev_info ov7695_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};


static const struct i2c_device_id ov7695_i2c_id[] = {
	{OV7695_SENSOR_NAME, (kernel_ulong_t)&ov7695_s_ctrl},
	{ }
};
static struct msm_camera_i2c_reg_conf ov7695_reg_wb_auto[] = {
    {0x5200,0x00},//Gain Man Disable
};

static struct msm_camera_i2c_reg_conf ov7695_reg_wb_inca[] = {
    {0x5200,0x20},//Man En
    {0x5204,0x04},//R Gain
    {0x5205,0x00},//
    {0x5206,0x05},//G Gain
    {0x5207,0x0D},//
    {0x5208,0x0B},//B Gain
    {0x5209,0xE8}
};

static struct msm_camera_i2c_reg_conf ov7695_reg_wb_daylight[] = {
    {0x5200,0x20},//Man En
    {0x5204,0x04},//R Gain
    {0x5205,0xA2},//
    {0x5206,0x04},//G Gain
    {0x5207,0x00},//
    {0x5208,0x05},//B Gain
    {0x5209,0x52}
};

static struct msm_camera_i2c_reg_conf ov7695_reg_wb_fluorescent[] = {
    {0x5200,0x20},// Man En
    {0x5204,0x04},//R Gain
    {0x5205,0xA0},  
    {0x5206,0x04},//G Gain
    {0x5207,0x00},  
    {0x5208,0x07},//B Gain
    {0x5209,0x8e}  
};

static struct msm_camera_i2c_reg_conf ov7695_reg_wb_cloudy[] = {
    {0x5200 ,0x20},//Man En
    {0x5204 ,0x06},//R Gain
    {0x5205 ,0x0B},  
    {0x5206 ,0x04},//G Gain
    {0x5207 ,0x00},  
    {0x5208 ,0x05},//B Gain
    {0x5209 ,0x60}
};

static struct msm_camera_i2c_reg_conf ov7695_reg_antibanding[][1] = {
	/* OFF */
	{
		{0x5002, 0x48},//60Hz
	},
	/* 60Hz */
	{
		{0x5002, 0x48},
	},
	/* 50Hz */
	{
		{0x5002, 0x4a},
	},
	/* AUTO */
	{
		{0x5002, 0x48},//60Hz
	},
};

static struct msm_camera_i2c_reg_conf ov7695_reg_fps[][2] = {
		{/* 15 */
			{0x0342, 0x05},
			{0x0343, 0xd4},
		},
		{/* 16 */
			{0x0342, 0x05},
			{0x0343, 0x8A},
		},
		{/* 17 */
			{0x0342, 0x05},
			{0x0343, 0x40},
		},
		{/* 18 */
			{0x0342, 0x04},
			{0x0343, 0xf6},
		},
		{/* 19 */
			{0x0342, 0x04},
			{0x0343, 0xAC},
		},
		{/* 20 */
			{0x0342, 0x04},
			{0x0343, 0x5f},
		},
		{/* 21 */
			{0x0342, 0x04},
			{0x0343, 0x31},
		},
		{/* 22 */
			{0x0342, 0x04},
			{0x0343, 0x03},
		},
		{/* 23 */
			{0x0342, 0x03},
			{0x0343, 0xd5},
		},
		{/* 24 */
			{0x0342, 0x03},
			{0x0343, 0xa4},
		},
		{/* 25 */
			{0x0342, 0x03},
			{0x0343, 0x85},
		},
		{/* 26 */
			{0x0342, 0x03},
			{0x0343, 0x66},
		},
		{/* 27 */
			{0x0342, 0x03},
			{0x0343, 0x47},
		},
		{/* 28 */
			{0x0342, 0x03},
			{0x0343, 0x28},
		},
		{/* 29 */
			{0x0342, 0x03},
			{0x0343, 0x09},
		},
		{/* 30 */
			{0x0342, 0x02},
			{0x0343, 0xea},
		},
};

static int32_t msm_ov7695_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov7695_s_ctrl);
}

static struct i2c_driver ov7695_i2c_driver = {
	.id_table = ov7695_i2c_id,
	.probe  = msm_ov7695_i2c_probe,
	.driver = {
		.name = OV7695_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov7695_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id ov7695_dt_match[] = {
	{.compatible = "qcom,ov7695", .data = &ov7695_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov7695_dt_match);

static struct platform_driver ov7695_platform_driver = {
	.driver = {
		.name = "qcom,ov7695",
		.owner = THIS_MODULE,
		.of_match_table = ov7695_dt_match,
	},
};

static int32_t ov7695_platform_probe(struct platform_device *pdev)
{
	int32_t rc,ret;
	const struct of_device_id *match;
	match = of_match_device(ov7695_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	ret = device_create_file(&(pdev->dev), &dev_attr_read_id);
	if (0 != ret)
		pr_err("%s:%d creating attribute failed \n", __func__,__LINE__);
	ret = device_create_file(&(pdev->dev), &dev_attr_read_version);
		if (0 != ret)
			pr_err("%s:%d creating attribute failed \n", __func__,__LINE__);
	return rc;
}

static void ov7695_set_white_balance_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int value)
{
	pr_debug("%s %d", __func__, value);
	switch (value) {
	case MSM_CAMERA_WB_MODE_AUTO: {
     		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_auto,
			ARRAY_SIZE(ov7695_reg_wb_auto),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case MSM_CAMERA_WB_MODE_INCANDESCENT: {
     		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_inca,
			ARRAY_SIZE(ov7695_reg_wb_inca),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case MSM_CAMERA_WB_MODE_DAYLIGHT: {
     		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_daylight,
			ARRAY_SIZE(ov7695_reg_wb_daylight),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	}
	case MSM_CAMERA_WB_MODE_FLUORESCENT: {
     		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_fluorescent,
			ARRAY_SIZE(ov7695_reg_wb_fluorescent),
			MSM_CAMERA_I2C_BYTE_DATA);
					break;
	}
	case MSM_CAMERA_WB_MODE_CLOUDY_DAYLIGHT: {
		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_cloudy,
			ARRAY_SIZE(ov7695_reg_wb_cloudy),
			MSM_CAMERA_I2C_BYTE_DATA);
        break;
	}
	default:
     		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_auto,
			ARRAY_SIZE(ov7695_reg_wb_auto),
			MSM_CAMERA_I2C_BYTE_DATA);
	}
}

static void ov7695_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_debug("%s %d", __func__, value);
	if(value< 4)
	{
		s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
		s_ctrl->sensor_i2c_client, ov7695_reg_antibanding[value],
		ARRAY_SIZE(ov7695_reg_antibanding[0]),
		MSM_CAMERA_I2C_BYTE_DATA);
	}
}

static void ov7695_set_fps(struct msm_sensor_ctrl_t *s_ctrl, int value)
{
	pr_err("%s %d", __func__, value);

	if (value >30)
	{
		pr_err("%s error in set fps %d", __func__, value);
		value=30;
	}else if (value < 15)
	{
		pr_err("%s error in set fps %d", __func__, value);
		value=15;
	}
	value = value - 15;
 	s_ctrl->sensor_i2c_client->i2c_func_tbl->	i2c_write_conf_tbl(
	s_ctrl->sensor_i2c_client, ov7695_reg_fps[value],
	ARRAY_SIZE(ov7695_reg_fps[0]),
	MSM_CAMERA_I2C_BYTE_DATA);
}

static int __init ov7695_init_module(void)
{
	int32_t rc;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&ov7695_platform_driver,
		ov7695_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&ov7695_i2c_driver);
}

static void __exit ov7695_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (ov7695_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov7695_s_ctrl);
		platform_driver_unregister(&ov7695_platform_driver);
	} else
		i2c_del_driver(&ov7695_i2c_driver);
	return;
}

int32_t ov7695_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	CDBG("%s:%d %s cfgtype = %d\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++)
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);

		break;
	case CFG_SET_INIT_SETTING:
		/* 1. Write Recommend settings */
		/* 2. Write change settings */
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_recommend_settings,
			ARRAY_SIZE(ov7695_recommend_settings),
			MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_RESOLUTION:
		break;
	case CFG_SET_STOP_STREAM:
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
					i2c_write_conf_tbl(
					s_ctrl->sensor_i2c_client, ov7695_stop,
					ARRAY_SIZE(ov7695_stop),
					MSM_CAMERA_I2C_BYTE_DATA);
		break;
	case CFG_SET_START_STREAM:
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
						i2c_write_conf_tbl(
						s_ctrl->sensor_i2c_client, ov7695_start,
						ARRAY_SIZE(ov7695_start),
						MSM_CAMERA_I2C_BYTE_DATA);
		msleep(100);
															
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_conf_tbl(
			s_ctrl->sensor_i2c_client, ov7695_reg_wb_auto,
			ARRAY_SIZE(ov7695_reg_wb_auto),
			MSM_CAMERA_I2C_BYTE_DATA);
								
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params =
			*s_ctrl->sensordata->sensor_init_params;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_sensor_power_setting_array *power_setting_array;
		int slave_index = 0;
		if (copy_from_user(&sensor_slave_info,
		    (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_sensor_slave_info))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		/* Update sensor slave address */
		if (sensor_slave_info.slave_addr) {
			s_ctrl->sensor_i2c_client->cci_client->sid =
				sensor_slave_info.slave_addr >> 1;
		}

		/* Update sensor address type */
		s_ctrl->sensor_i2c_client->addr_type =
			sensor_slave_info.addr_type;

		/* Update power up / down sequence */
		s_ctrl->power_setting_array =
			sensor_slave_info.power_setting_array;
		power_setting_array = &s_ctrl->power_setting_array;
		power_setting_array->power_setting = kzalloc(
			power_setting_array->size *
			sizeof(struct msm_sensor_power_setting), GFP_KERNEL);
		if (!power_setting_array->power_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(power_setting_array->power_setting,
		    (void *)sensor_slave_info.power_setting_array.power_setting,
		    power_setting_array->size *
		    sizeof(struct msm_sensor_power_setting))) {
			kfree(power_setting_array->power_setting);
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		s_ctrl->free_power_setting = true;
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.slave_addr);
		CDBG("%s sensor addr type %d\n", __func__,
			sensor_slave_info.addr_type);
		CDBG("%s sensor reg %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("%s sensor id %x\n", __func__,
			sensor_slave_info.sensor_id_info.sensor_id);
		for (slave_index = 0; slave_index <
			power_setting_array->size; slave_index++) {
			CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
				slave_index,
				power_setting_array->power_setting[slave_index].
				seq_type,
				power_setting_array->power_setting[slave_index].
				seq_val,
				power_setting_array->power_setting[slave_index].
				config_val,
				power_setting_array->power_setting[slave_index].
				delay);
		}
		kfree(power_setting_array->power_setting);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		if (!conf_array.size) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		if (!conf_array.size) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;

		if (!stop_setting->size) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
		}
		case CFG_SET_SATURATION: {
			int32_t sat_lev;
			if (copy_from_user(&sat_lev, (void *)cdata->cfg.setting,
				sizeof(int32_t))) {
				pr_err("%s:%d failed\n", __func__, __LINE__);
				rc = -EFAULT;
				break;
			}
		pr_debug("%s: Saturation Value is %d", __func__, sat_lev);
		break;
		}
		case CFG_SET_CONTRAST: {
			int32_t con_lev;
			if (copy_from_user(&con_lev, (void *)cdata->cfg.setting,
				sizeof(int32_t))) {
				pr_err("%s:%d failed\n", __func__, __LINE__);
				rc = -EFAULT;
				break;
			}
		pr_debug("%s: Contrast Value is %d", __func__, con_lev);
		break;
		}
		case CFG_SET_SHARPNESS: {
			int32_t shp_lev;
			if (copy_from_user(&shp_lev, (void *)cdata->cfg.setting,
				sizeof(int32_t))) {
				pr_err("%s:%d failed\n", __func__, __LINE__);
				rc = -EFAULT;
				break;
			}
		pr_debug("%s: Sharpness Value is %d", __func__, shp_lev);
		break;
		}
		case CFG_SET_AUTOFOCUS: {
		/* TO-DO: set the Auto Focus */
		pr_debug("%s: Setting Auto Focus", __func__);
		break;
		}
		case CFG_CANCEL_AUTOFOCUS: {
		/* TO-DO: Cancel the Auto Focus */
		pr_debug("%s: Cancelling Auto Focus", __func__);
		break;
		}
		case CFG_SET_WHITE_BALANCE: {
			int32_t wb_mode;
			if (copy_from_user(&wb_mode, (void *)cdata->cfg.setting,
				sizeof(int32_t))) {
				pr_err("%s:%d failed\n", __func__, __LINE__);
				rc = -EFAULT;
				break;
			}
		pr_debug("%s: Setting White Balance", __func__);
              ov7695_set_white_balance_mode(s_ctrl, wb_mode);
		break;
		}
        	case CFG_SET_ANTIBANDING: {
		int32_t antibanding_mode;
		if (copy_from_user(&antibanding_mode,
			(void *)cdata->cfg.setting,
			sizeof(int32_t))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}
		pr_debug("%s: anti-banding mode is %d", __func__,
			antibanding_mode);
		ov7695_set_antibanding(s_ctrl, antibanding_mode);
		break;
        	}
		case CFG_SET_FPS: {
				int32_t fps_value;
				if (copy_from_user(&fps_value, (void *)cdata->cfg.setting,
					sizeof(int32_t))) {
					pr_err("%s:%d failed\n", __func__, __LINE__);
					rc = -EFAULT;
					break;
				}
			pr_debug("%s: Setting FPS %d", __func__,fps_value);
			ov7695_set_fps(s_ctrl,fps_value);
			break;
			}
		default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

static struct msm_sensor_fn_t ov7695_sensor_func_tbl = {
	.sensor_config = ov7695_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
};

static struct msm_sensor_ctrl_t ov7695_s_ctrl = {
	.sensor_i2c_client = &ov7695_sensor_i2c_client,
	.power_setting_array.power_setting = ov7695_power_setting,
	.power_setting_array.size = ARRAY_SIZE(ov7695_power_setting),
	.msm_sensor_mutex = &ov7695_mut,
	.sensor_v4l2_subdev_info = ov7695_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov7695_subdev_info),
	.func_tbl = &ov7695_sensor_func_tbl,
};

module_init(ov7695_init_module);
module_exit(ov7695_exit_module);
MODULE_DESCRIPTION("Aptina 1.26MP YUV sensor driver");
MODULE_LICENSE("GPL v2");

static ssize_t ov7695_read_id_attr(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct msm_sensor_ctrl_t *s_ctrl;
	int32_t power_flag = 0;
	int32_t rc = 0;
	uint16_t chipid = 0;

	pr_err("ov7695_read_id device addr= %x\n",(uint32_t)dev);
	pr_err("get driver addr =%x \n",(uint32_t)dev_get_drvdata(dev));
	s_ctrl = &ov7695_s_ctrl;

	if(s_ctrl->sensor_state != MSM_SENSOR_POWER_UP)
	{
		s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		power_flag =1;
	}

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			s_ctrl->sensordata->slave_info->sensor_id_reg_addr,
			&chipid, MSM_CAMERA_I2C_WORD_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
	}

	if(power_flag == 1)
	{
		s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	}

	 return sprintf(buf, "%x\n", chipid);
}

static ssize_t ov7695_read_version_attr(struct device *dev,struct device_attribute *attr, char *buf)
{
	struct msm_sensor_ctrl_t *s_ctrl;
	int32_t power_flag = 0;
	int32_t rc = 0;
	uint16_t version = 0;

	pr_err("ov7695_read_id device addr= %x\n",(uint32_t)dev);
	pr_err("get driver addr =%x \n",(uint32_t)dev_get_drvdata(dev));
	s_ctrl = &ov7695_s_ctrl;

	if(s_ctrl->sensor_state != MSM_SENSOR_POWER_UP)
	{
		s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		power_flag =1;
	}

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			0x302A,
			&version, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
		pr_err("%s: %s: read id failed\n", __func__,
			s_ctrl->sensordata->sensor_name);
	}

	if(power_flag == 1)
	{
		s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	}

	 return sprintf(buf, "%x\n", version);
}
