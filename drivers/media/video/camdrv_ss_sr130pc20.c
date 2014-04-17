
/***********************************************************************
* Driver for sr130 (3MP Camera) from SAMSUNG SYSTEM LSI
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
***********************************************************************/

  

#include <linux/i2c.h> 


#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include <mach/clock.h>
#include <camdrv_ss.h>
#include <camdrv_ss_sr130pc20.h>
#include <linux/module.h>


#define sr130pc20_NAME	"sr130pc20"
#define SENSOR_ID 2
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x4b0000  // 4915200  = 2560*1920     //0x33F000     //3403776 //2216 * 1536
#define SENSOR_PREVIEW_WIDTH      640 // 1024
#define SENSOR_PREVIEW_HEIGHT     480 // 768
#define AF_OUTER_WINDOW_WIDTH   320
#define AF_OUTER_WINDOW_HEIGHT  266
#define AF_INNER_WINDOW_WIDTH   143
#define AF_INNER_WINDOW_HEIGHT  143
#define MAX_BUFFER			(4096)

#define SR130PC20_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define SR130PC20_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8 
#define FORMAT_FLAGS_COMPRESSED 0x3
#define DEFUALT_MCLK		26000000

static bool first_af_status = false;
#define SR130PC20_REGISTER_SIZE 2

#define SR130PC20_DELAY_DURATION 0xFF

#define EXIF_SOFTWARE		""
#define EXIF_MAKE		"Samsung"
#define EXIF_MODEL		"GT-S6012"

#define SENSOR_0_CLK			"dig_ch0_clk"    
#define SENSOR_0_CLK_FREQ		(26000000) 
#define CSI0_LP_FREQ			(100000000)
#define CSI1_LP_FREQ			(100000000)

static struct regulator *VCAM_IO_1_8_V;
static struct regulator *VCAM_A_2_8_V; 
static struct regulator *VCAM_CORE_1_8_V;  

#ifdef CONFIG_MACH_HAWAII_SS_LT063G_REV00
#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_CORE_1_8V_REGULATOR	"lvldo2"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1786000
#define VCAM_CORE_1_8V_REGULATOR_uV	1786000
#define CAM_RESET	111
#define CAM_STNBY	002

#elif CONFIG_MACH_HAWAII_SS_LT063G_REV01
#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_CORE_1_8V_REGULATOR	"lvldo2"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1786000
#define VCAM_CORE_1_8V_REGULATOR_uV	1786000
#define CAM_RESET	004
#define CAM_STNBY	005

#else
#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_CORE_1_8V_REGULATOR	"lvldo2"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1786000
#define VCAM_CORE_1_8V_REGULATOR_uV	1786000
#define CAM_RESET	004
#define CAM_STNBY	005
#endif


static DEFINE_MUTEX(af_cancel_op);
extern inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd);



extern  int camdrv_ss_i2c_set_config_register(struct i2c_client *client, 
                                         regs_t reg_buffer[], 
          				                 int num_of_regs, 
          				                 char *name);
extern int camdrv_ss_set_preview_size(struct v4l2_subdev *sd);
extern int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff);
extern inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd);

//#define __JPEG_CAPTURE__ 1        //denis_temp ; yuv capture

extern int camera_antibanding_get(); //add anti-banding code

static const struct camdrv_ss_framesize sr130pc20_supported_preview_framesize_list[] = {
	{ PREVIEW_SIZE_VGA,	640,  480 },
//	{ PREVIEW_SIZE_QCIF,	176,  144 },
//	{ PREVIEW_SIZE_QVGA,	320,  240 },
//	{ PREVIEW_SIZE_CIF,	352,  288 },
//	{ PREVIEW_SIZE_D1,	720,  480 },
	{ PREVIEW_SIZE_1MP,     1280, 960 },
//	{ PREVIEW_SIZE_W1MP, 1600,960},
//	{ PREVIEW_SIZE_2MP,	1600,  1200 },
//	{ PREVIEW_SIZE_W2MP,	2048,  1152 },
//	{ PREVIEW_SIZE_3MP,	2048,  1536 },
//	{ PREVIEW_SIZE_5MP,	2560,  1920 },            
};


static const struct camdrv_ss_framesize  sr130pc20_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA,		640,  480 },
	{ CAPTURE_SIZE_1MP,		1280, 960 },
//	{ CAPTURE_SIZE_2MP,		1600, 1200 },
//	{ CAPTURE_SIZE_W2MP,		2048, 1232 },
//	{ CAPTURE_SIZE_3MP,		2048, 1536 },
//	{ CAPTURE_SIZE_5MP,		2560, 1920 },
};
const static struct v4l2_fmtdesc sr130pc20_fmts[] = 
{
    {
        .index          = 0,
        .type           = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .flags          = 0,
        .description    = "UYVY",
        .pixelformat    = V4L2_MBUS_FMT_UYVY8_2X8,
    },

#ifdef __JPEG_CAPTURE__
	{
		.index		= 1,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_COMPRESSED,
		.description	= "JPEG + Postview",
		.pixelformat	= V4L2_MBUS_FMT_JPEG_1X8,
	},
#endif	
};

static const struct v4l2_queryctrl sr130pc20_controls[] = {
	{
		.id			= V4L2_CID_CAMERA_FLASH_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash",
		.minimum	= FLASH_MODE_OFF,
		.maximum	= (1 << FLASH_MODE_OFF),
		.step		= 1,
		.default_value	= FLASH_MODE_OFF,
	},

	{
		.id			= V4L2_CID_CAMERA_BRIGHTNESS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Brightness",
		.minimum	= EV_MINUS_4,
		.maximum	= EV_PLUS_4,
		.step		= 1,
		.default_value	= EV_DEFAULT,
	},

	{
		.id			= V4L2_CID_CAMERA_WHITE_BALANCE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Whilte Balance",
		.minimum	= WHITE_BALANCE_AUTO ,
		.maximum	= (1 << WHITE_BALANCE_AUTO | 1 << WHITE_BALANCE_DAYLIGHT/*WHITE_BALANCE_SUNNY*/ | 1 << WHITE_BALANCE_CLOUDY
			           | 1 << WHITE_BALANCE_INCANDESCENT/*WHITE_BALANCE_TUNGSTEN*/ | 1 << WHITE_BALANCE_FLUORESCENT ),
		.step		= 1,
		.default_value	= WHITE_BALANCE_AUTO,
	},

/* remove zoom setting here to use AP zoom feture
	{
		.id			= V4L2_CID_CAMERA_ZOOM,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Zoom",
		.minimum	= ZOOM_LEVEL_0,
		.maximum	= ZOOM_LEVEL_8,
		.step		= 1,
		.default_value	= ZOOM_LEVEL_0,
	},
*/

	{
		.id			= V4L2_CID_CAMERA_EFFECT,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Color Effects",
		.minimum	= IMAGE_EFFECT_NONE,
		.maximum	= (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_MONO/*IMAGE_EFFECT_BNW*/
						| 1 << IMAGE_EFFECT_SEPIA | 1 << IMAGE_EFFECT_NEGATIVE), /* this should be replace by querymenu */
		.step		= 1,
		.default_value	= IMAGE_EFFECT_NONE,
	},

	{
		.id			= V4L2_CID_CAMERA_ISO,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "ISO",
		.minimum	= ISO_AUTO ,
		.maximum	= (1 << ISO_AUTO | 1 << ISO_50 | 1 << ISO_100 | 1 << ISO_200 | 1 << ISO_400 | 1 << ISO_800 | 1 << ISO_1200 
						| 1 << ISO_1600 | 1 << ISO_2400 | 1 << ISO_3200 | 1 << ISO_SPORTS | 1 << ISO_NIGHT | 1 << ISO_MOVIE),
		.step		= 1,
		.default_value	= ISO_AUTO,
	},

	{
		.id			= V4L2_CID_CAMERA_METERING,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Metering",
		.minimum	= METERING_MATRIX,
		.maximum	= (1 << METERING_MATRIX | 1 << METERING_CENTER | 1 << METERING_SPOT),
		.step		= 1,
		.default_value	= METERING_CENTER,
	},

	{
		.id			= V4L2_CID_CAMERA_CONTRAST,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Contrast",
		.minimum	= CONTRAST_MINUS_2,
		.maximum	= CONTRAST_PLUS_2,
		.step		= 1,
		.default_value	= CONTRAST_DEFAULT,
	},

	{
		.id			= V4L2_CID_CAMERA_SATURATION,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Saturation",
		.minimum	= SATURATION_MINUS_2,
		.maximum	= SATURATION_PLUS_2,
		.step		= 1,
		.default_value	= SATURATION_DEFAULT,
	},

	{
		.id			= V4L2_CID_CAMERA_SHARPNESS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Sharpness",
		.minimum	= SHARPNESS_MINUS_2,
		.maximum	= SHARPNESS_PLUS_2,
		.step		= 1,
		.default_value	= SHARPNESS_DEFAULT,
	},

	{
		.id			= V4L2_CID_CAMERA_WDR,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "WDR",
		.minimum	= WDR_OFF,
		.maximum	= WDR_ON,
		.step		= 1,
		.default_value	= WDR_OFF,
	},
/*
	{
		.id			= V4L2_CID_CAMERA_FACE_DETECTION,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Face Detection",
		.minimum	= FACE_DETECTION_OFF,
		.maximum	= FACE_DETECTION_ON,
		.step		= 1,
		.default_value	= FACE_DETECTION_OFF,
	},
*/
	{
		.id			= V4L2_CID_CAMERA_FOCUS_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Focus",
		.minimum	      = FOCUS_MODE_INFINITY,
		.maximum	=  (1 << FOCUS_MODE_INFINITY),
		.step		= 1,
		.default_value	= FOCUS_MODE_INFINITY,
	},

	{
		.id			= V4L2_CID_CAM_JPEG_QUALITY,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "JPEG Quality",
		.minimum	= 0,
		.maximum	= 100,
		.step		= 1,
		.default_value	= 100,
	},

	{
		.id			= V4L2_CID_CAMERA_SCENE_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Scene Mode",
		.minimum	= SCENE_MODE_NONE,
		.maximum	= (1 << SCENE_MODE_NONE | 1 << SCENE_MODE_PORTRAIT |
						1 << SCENE_MODE_NIGHTSHOT | 1 << SCENE_MODE_LANDSCAPE
						| 1 << SCENE_MODE_SPORTS | 1 << SCENE_MODE_PARTY_INDOOR |
						1 << SCENE_MODE_BEACH_SNOW | 1 << SCENE_MODE_SUNSET |
						1 << SCENE_MODE_FIREWORKS | 1 << SCENE_MODE_CANDLE_LIGHT | /*querymenu?*/
						1 << SCENE_MODE_BACK_LIGHT | 1<< SCENE_MODE_DUSK_DAWN |
						1 << SCENE_MODE_FALL_COLOR | 1<< SCENE_MODE_TEXT),
		.step		= 1,
		.default_value	= SCENE_MODE_NONE,
	},

	{
		.id			= V4L2_CID_CAMERA_SET_AUTO_FOCUS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Set AutoFocus",
		.minimum	= AUTO_FOCUS_OFF,
		.maximum	= AUTO_FOCUS_ON,
		.step		= 1,
		.default_value	= AUTO_FOCUS_OFF,
	},
         /*
	{
		.id 		= V4L2_CID_CAMERA_TOUCH_AF_AREA,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Touchfocus areas",
		.minimum	= 0,
		.maximum	= 1,
		.step		= 1,
		.default_value	= 1,
	},
*/

	{
		.id			= V4L2_CID_CAMERA_FRAME_RATE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Framerate control",
		.minimum	= FRAME_RATE_AUTO,
		.maximum	= (1 << FRAME_RATE_AUTO | /* 1 << FRAME_RATE_5 | 1 << FRAME_RATE_7 | 1 << FRAME_RATE_10  |*/ 1 << FRAME_RATE_15
						| /*1 << FRAME_RATE_20 |*/ 1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
		.step		= 1,
		.default_value	= FRAME_RATE_30,
	},

	{
		.id			= V4L2_CID_CAMERA_CAPTURE,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Capture",
		.minimum	= 0,
		.maximum	= 0,
		.step		= 0,
		.default_value	= 0,
	},

	{
		.id			= V4L2_CID_CAM_PREVIEW_ONOFF,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Preview control",
		.minimum	= 0,
		.maximum	= 1,
		.step		= 1,
		.default_value	= 0,
	},

	{
		.id			= V4L2_CID_CAMERA_CHECK_DATALINE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Check Dataline",
		.minimum	= 0,
		.maximum	= 1,
		.step		= 1,
		.default_value	= 0,
	},

	{
		.id			= V4L2_CID_CAMERA_CHECK_DATALINE_STOP,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Check Dataline Stop",
		.minimum	= 0,
		.maximum	= 0,
		.step		= 0,
		.default_value	= 0,
	},

	{
		.id			= V4L2_CID_CAMERA_RETURN_FOCUS,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Return Focus",
		.minimum	= 0,
		.maximum	= 0,
		.step		= 0,
		.default_value	= 0,
	},

	{
		.id			= V4L2_CID_CAMERA_ANTI_BANDING,
		.type		= V4L2_CTRL_TYPE_BOOLEAN,
		.name		= "Anti Banding",
		.minimum	      = ANTI_BANDING_AUTO,
		.maximum	=  (1 << ANTI_BANDING_AUTO | 1 << ANTI_BANDING_50HZ | 1 << ANTI_BANDING_60HZ
                                    | 1 << ANTI_BANDING_OFF),
		.step		= 1,
		.default_value	= ANTI_BANDING_AUTO,
	},
	{
		.id			= V4L2_CID_CAMERA_VT_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Vtmode",
		.minimum	      = CAM_VT_MODE_3G,
		.maximum	= CAM_VT_MODE_VOIP,
		.step		= 1,
		.default_value	= CAM_VT_MODE_3G,
	},	
	
	{
		.id			= V4L2_CID_CAMERA_SENSOR_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Cam mode",
		.minimum	      = SENSOR_CAMERA,
		.maximum	= SENSOR_MOVIE,
		.step		= 1,
		.default_value	= SENSOR_CAMERA,
	},	
		
};

static int camdrv_ss_sr130pc20_enum_frameintervals(struct v4l2_subdev *sd,struct v4l2_frmivalenum *fival)
{
	int err = 0;
	int size,i;

	if (fival->index >= 1)
	{
		CAM_ERROR_PRINTK("%s: returning ERROR because index =%d !!!  \n",__func__,fival->index);
		return -EINVAL;
	}
	
	CAM_INFO_PRINTK("%s :  check w = %d h = %d \n",__func__,fival->width,fival->height);
	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	for(i = 0; i < ARRAY_SIZE(sr130pc20_supported_preview_framesize_list); i++) {
		if((sr130pc20_supported_preview_framesize_list[i].width == fival->width) &&
		    (sr130pc20_supported_preview_framesize_list[i].height == fival->height))
		{
			size = sr130pc20_supported_preview_framesize_list[i].index;
			break;
		}
	}

	if(i == ARRAY_SIZE(sr130pc20_supported_preview_framesize_list))
	{
		CAM_ERROR_PRINTK("%s unsupported width = %d and height = %d  \n",__func__,fival->width,fival->height);
		return -EINVAL;
	}
	else
		CAM_INFO_PRINTK(" %s :  found i = %d\n",__func__,i);

	switch (size) {
	case PREVIEW_SIZE_5MP:
	case PREVIEW_SIZE_3MP:
	case PREVIEW_SIZE_2MP:
	case PREVIEW_SIZE_W1MP:
	case PREVIEW_SIZE_1MP:
		fival->discrete.numerator = 2;
		fival->discrete.denominator = 15;
		break;
	default:
		fival->discrete.numerator = 1;
		fival->discrete.denominator = 30;
		break;
	}

	return err;
}


static long camdrv_ss_sr130pc20_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
//	struct i2c_client *client = v4l2_get_subdevdata(sd);
//	struct camdrv_ss_state *state =
//		container_of(sd, struct camdrv_ss_states, sd);
	int ret = 0;

	switch(cmd) {

		case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
#ifdef JPEG_THUMB_ACTIVE
			*p =1;
#else
			*p = 0; /* NO */
#endif
			break;
		}

		case VIDIOC_THUMB_G_FMT:
		{
			struct v4l2_format *p = arg;
			struct v4l2_pix_format *pix = &p->fmt.pix;
			p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			/* fixed thumbnail resolution and format */
			pix->width = 640;
			pix->height = 480;
			pix->bytesperline = 640 * 2;
			pix->sizeimage = 640 * 480 * 2;
			pix->field = V4L2_FIELD_ANY;
			pix->colorspace = V4L2_COLORSPACE_JPEG,
			pix->pixelformat = V4L2_PIX_FMT_UYVY;
			break;
		}

		case VIDIOC_THUMB_S_FMT:
		{
		//	struct v4l2_format *p = arg;
			/* for now we don't support setting thumbnail fmt and res */
			ret = -EINVAL;
			break;
		}
		case VIDIOC_JPEG_G_PACKET_INFO:
		{
			struct v4l2_jpeg_packet_info *p = arg;
#ifdef JPEG_THUMB_ACTIVE
			 p->padded = 4;
			 p->packet_size= 0x27c;
#else
			 p->padded = 0;
			 p->packet_size = 0x810;
#endif
			 break;
		}
                 case VIDIOC_SENSOR_G_OPTICAL_INFO:
                {

                         struct v4l2_sensor_optical_info *p= arg;
                           p->hor_angle.numerator = 541;
                           p->hor_angle.denominator = 10;
                           p->ver_angle.numerator = 429;
                           p->ver_angle.denominator = 10;
                           p->focus_distance[0] = 60;
                           p->focus_distance[1] = 120;
                           p->focus_distance[2] = -1;
                           p->focal_length.numerator = 270;
                           p->focal_length.denominator = 100;
                         break;
                }


		default:
			ret = -ENOIOCTLCMD;
			break;
	}

	return ret;
}


int camdrv_ss_sr130pc20_set_preview_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( "%s :\n", __func__);

	if (!state->pix.width || !state->pix.height) {
		CAM_ERROR_PRINTK( "%s : width or height is NULL!!!\n",__func__);
		return -EINVAL;
	}
		if (state->mode_switch == INIT_DONE_TO_CAMERA_PREVIEW) {
			
			if(ANTI_BANDING_60HZ == camera_antibanding_get()){
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_60hz_setting, ARRAY_SIZE(sr130pc20_preview_60hz_setting), "preview_60hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_60hz_setting IS FAILED\n",__func__);
					return -EIO;
				}		
			}
			/*else{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_50hz_setting, ARRAY_SIZE(sr130pc20_preview_50hz_setting), "preview_50hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_50hz_setting IS FAILED\n",__func__);
					return -EIO;
				}
			}*/
			
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_camera_regs, ARRAY_SIZE(sr130pc20_preview_camera_regs), "preview_camera_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK( "%s :sr130pc20_preview_camera_regs IS FAILED\n",__func__);
				return -EIO;
			}
		}

	if (state->mode_switch == PICTURE_CAPTURE_TO_CAMERA_PREVIEW_RETURN) {
		err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_camera_regs, ARRAY_SIZE(sr130pc20_preview_camera_regs), "preview_camera_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :sr130pc20_preview_camera_regs IS FAILED\n",__func__);
			return -EIO;
		}
	}

	if(state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW)
	{
			
			if(ANTI_BANDING_60HZ == camera_antibanding_get()){
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_60hz_setting, ARRAY_SIZE(sr130pc20_preview_60hz_setting), "preview_60hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_60hz_setting IS FAILED\n",__func__);
					return -EIO;
				}		
			}else{			
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_50hz_setting, ARRAY_SIZE(sr130pc20_preview_50hz_setting), "preview_50hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_50hz_setting IS FAILED\n",__func__);
					return -EIO;
				}
			}
			
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_camera_regs, ARRAY_SIZE(sr130pc20_preview_camera_regs), "preview_camera_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK( "%s :sr130pc20_preview_camera_regs IS FAILED\n",__func__);
				return -EIO;
			}
		}
	else if(state->mode_switch == INIT_DONE_TO_CAMCORDER_PREVIEW)
	{
			
			if(ANTI_BANDING_60HZ == camera_antibanding_get()){
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_60hz_setting, ARRAY_SIZE(sr130pc20_preview_60hz_setting), "preview_60hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_60hz_setting IS FAILED\n",__func__);
					return -EIO;
				}		
			}else{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_50hz_setting, ARRAY_SIZE(sr130pc20_preview_50hz_setting), "preview_50hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_50hz_setting IS FAILED\n",__func__);
					return -EIO;
				}
			}
			
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_camera_regs, ARRAY_SIZE(sr130pc20_preview_camera_regs), "preview_camera_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK( "%s :sr130pc20_preview_camera_regs IS FAILED\n",__func__);
				return -EIO;
			}
		}
	else if(state->mode_switch == CAMCORDER_PREVIEW_TO_CAMERA_PREVIEW)
	{
			
			if(ANTI_BANDING_60HZ == camera_antibanding_get()){
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_60hz_setting, ARRAY_SIZE(sr130pc20_preview_60hz_setting), "preview_60hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_60hz_setting IS FAILED\n",__func__);
					return -EIO;
				}		
			}else{			
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_50hz_setting, ARRAY_SIZE(sr130pc20_preview_50hz_setting), "preview_50hz_setting");
				if (err < 0) {
					CAM_ERROR_PRINTK( "%s :sr130pc20_preview_50hz_setting IS FAILED\n",__func__);
					return -EIO;
				}
			}
			
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_preview_camera_regs, ARRAY_SIZE(sr130pc20_preview_camera_regs), "preview_camera_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK( "%s :sr130pc20_preview_camera_regs IS FAILED\n",__func__);
				return -EIO;
			}
		}
/*
	err = camdrv_ss_set_preview_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK( "%s : camdrv_ss_set_preview_size is FAILED !!\n", __func__);
		return -EIO;
	}
*/ //already size setting is included in init registers. we don't need to write again.
	state->camera_flash_fire = 0;
	state->camera_af_flash_checked = 0;

	if (state->check_dataline) { /* Output Test Pattern */
		err = camdrv_ss_set_dataline_onoff(sd, 1);
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s : check_dataline is FAILED !!\n", __func__);
			return -EIO;
		}
	}

	return 0;
}

static int camdrv_ss_sr130pc20_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	CAM_INFO_PRINTK( "%s Entered\n", __func__);

	/* Set Snapshot registers */


	err = camdrv_ss_i2c_set_config_register(client, sr130pc20_snapshot_normal_regs, ARRAY_SIZE(sr130pc20_snapshot_normal_regs), "snapshot_normal_regs");
	if (err < 0) {
	CAM_ERROR_PRINTK( "%s : camdrv_ss_i2c_set_config_register failed  not supported !!!\n",  __func__);
	return -EIO;
	}
	
	
	/* Set image size */
	err = camdrv_ss_set_capture_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK( "%s: camdrv_ss_set_capture_size not supported !!!\n", __func__);
		return -EIO;
	}

	return 0;
}


//aska add
static int camdrv_ss_sr130pc20_set_white_balance(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( " %s :  value =%d\n", __func__, mode);

	switch (mode) {
	case WHITE_BALANCE_AUTO:
	{
		if (sr130pc20_wb_auto_regs == 0)
			CAM_ERROR_PRINTK( " %s : wb_auto_regs not supported !!!\n", __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_wb_auto_regs, ARRAY_SIZE(sr130pc20_wb_auto_regs), "wb_auto_regs");

		break;
	}

	
	case WHITE_BALANCE_CLOUDY:
	{
		if (sr130pc20_wb_cloudy_regs == 0)
			CAM_ERROR_PRINTK( "%s : wb_cloudy_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_wb_cloudy_regs, ARRAY_SIZE(sr130pc20_wb_cloudy_regs), "wb_cloudy_regs");

		break;
	}


	case WHITE_BALANCE_FLUORESCENT:
	{
		if (sr130pc20_wb_fluorescent_regs== 0)
			CAM_ERROR_PRINTK( " %s : wb_fluorescent_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,  sr130pc20_wb_fluorescent_regs, ARRAY_SIZE(sr130pc20_wb_fluorescent_regs), "wb_fluorescent_regs");

		break;
	}

	
	case WHITE_BALANCE_DAYLIGHT:
	{
		if (sr130pc20_wb_daylight_regs == 0)
			CAM_ERROR_PRINTK( " %s : wb_daylight_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_wb_daylight_regs, ARRAY_SIZE(sr130pc20_wb_daylight_regs), "wb_daylight_regs");

		break;
	}
	case WHITE_BALANCE_INCANDESCENT:
	{
		if (sr130pc20_wb_incandescent_regs == 0)
			CAM_ERROR_PRINTK( "%s : wb_incandescent_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_wb_incandescent_regs, ARRAY_SIZE(sr130pc20_wb_incandescent_regs), "wb_incandescent_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK( " %s : default not supported !!!\n",  __func__);
		break;
	}
	}

	state->currentWB = mode;

	return err;
}
//aska add
static int camdrv_ss_sr130pc20_set_iso(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_PRINTK( " %s :  value =%d\n",  __func__, mode);

	switch (mode) {
	case ISO_AUTO:
	{
		if (sr130pc20_iso_auto_regs == 0)
			CAM_ERROR_PRINTK( " %s : iso_auto_regs not supported !!!\n", __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_iso_auto_regs, ARRAY_SIZE(sr130pc20_iso_auto_regs), "iso_auto_regs");

		break;
	}
	case ISO_50:
	{
		if (sr130pc20_iso_50_regs== 0)
			CAM_ERROR_PRINTK( " %s : iso_50_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_iso_50_regs, ARRAY_SIZE(sr130pc20_iso_50_regs), "iso_50_regs");
	
		break;
	}

	case ISO_100:
	{
		if (sr130pc20_iso_100_regs == 0)
			CAM_ERROR_PRINTK( " %s : iso_100_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_iso_100_regs, ARRAY_SIZE(sr130pc20_iso_100_regs), "iso_100_regs");
	
		break;
	}
	case ISO_200:
	{
		if (sr130pc20_iso_200_regs == 0)
			CAM_ERROR_PRINTK( "%s  : iso_200_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,sr130pc20_iso_200_regs, ARRAY_SIZE(sr130pc20_iso_200_regs), "iso_200_regs");
	
		break;
	}
	case ISO_400:
	{
		if (sr130pc20_iso_400_regs == 0)
			CAM_ERROR_PRINTK( "%s  : iso_400_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr130pc20_iso_400_regs,ARRAY_SIZE(sr130pc20_iso_400_regs), "iso_400_regs");
	
		break;
	}

	default:
	{
		CAM_ERROR_PRINTK( " %s : default case supported !!!\n", __func__);
			break;
        }			
	} /* end of switch */

	return err;
}

static unsigned char pBurstData[MAX_BUFFER];
#if 1
#define BURST_MODE_BUFFER_MAX_SIZE 255
#define BURST_REG 0x0e
#define DELAY_REG 0xff
unsigned char sr130pc20_buf_for_burstmode[BURST_MODE_BUFFER_MAX_SIZE];

static int camdrv_ss_sr130pc20_sensor_write(struct i2c_client *sr130pc20_client ,unsigned char subaddr, unsigned char val,int count)
{
	unsigned char buf[2] = { 0 };
	struct i2c_msg msg = { sr130pc20_client->addr, 0, 2, buf };
	
#if defined(SENSOR_DEBUG)
	pr_err("[ 0x%x %x ]\n", subaddr, data);
#endif

	buf[0] = (subaddr);
	buf[1] = (val);

	return i2c_transfer(sr130pc20_client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

static int camdrv_ss_sr130pc20_sensor_burst_write_list(struct i2c_client *sr130pc20_client ,regs_t reg_buffer[], int size, char *name)
{
	int err = -EINVAL;
	int i = 0;
	int idx = 0;
	unsigned short subaddr = 0;
	unsigned short value = 0;
	int burst_flag = 0;
	int burst_cnt = 0;
	struct i2c_msg msg = { sr130pc20_client->addr,
		0, 0, sr130pc20_buf_for_burstmode
	};

	u16 list, list1;
	

#ifdef CONFIG_LOAD_FILE
	err = sr130pc20_regs_table_burst_write(name);
#else
	for (i = 0; i < size; i++) { 

		list1 = reg_buffer[i] >> 16;
        	list = reg_buffer[i];
			
		if (idx > (BURST_MODE_BUFFER_MAX_SIZE - 10)) {
			pr_err("[%s:%d]Burst mode buffer overflow! "
			       "Burst Count %d\n",
			       __func__, __LINE__, burst_cnt);
			#if 0
			pr_err("[%s:%d]count %d, addr %x "
			       "value %x\n", __func__, __LINE__, i,
			       (list[i] >> 8) & 0xff, list[i] & 0xFF);
			pr_err("[%s:%d]addr %x value %x\n",
			       __func__, __LINE__,
			       (list[i - 1] >> 8) & 0xff, list[i - 1] & 0xFF);
			pr_err("[%s:%d]addr %x value %x\n",
			       __func__, __LINE__,
			       (list[i - 2] >> 8) & 0xff, list[i - 2] & 0xFF);
			#endif
			err = -EIO;
			return err;
		}
		subaddr = (list>> 8);
		value = (list & 0xFF);
		if (burst_flag == 0) {
			switch (subaddr) {
			case BURST_REG:
				if (value != 0x00) {
					burst_flag = 1;
					burst_cnt++;
				}
				break;
			case DELAY_REG:
				msleep(value * 10);	/* a step is 10ms */
				break;
			default:
				idx = 0;
				err = camdrv_ss_sr130pc20_sensor_write(sr130pc20_client,subaddr, value,i);
				if(err < 0)
				{
					CAM_ERROR_PRINTK("%s: Burst i2c write fail \n",__func__);
					return err;
				}
				break;
			}
		} else if (burst_flag == 1) {
			if (subaddr == BURST_REG && value == 0x00) {
				msg.len = idx;
				CAM_INFO_PRINTK("burst_cnt %d, idx %d\n",
					     burst_cnt, idx);
				err = i2c_transfer(sr130pc20_client->adapter,
						   &msg, 1);
				if (err < 0) {
					pr_err("[%s:%d]Burst write fail!\n",
					       __func__, __LINE__);
					return err;
				}
				idx = 0;
				burst_flag = 0;
			} else {
				if (idx == 0) {
					sr130pc20_buf_for_burstmode[idx++] =
					    subaddr;
					sr130pc20_buf_for_burstmode[idx++] =
					    value;
				} else {
					sr130pc20_buf_for_burstmode[idx++] =
					    value;
				}
			}
		}
	}
#endif

	if (unlikely(err < 0)) {
		pr_err("[%s:%d] register set failed\n", __func__, __LINE__);
		return err;
	}

	return err;
}
#endif //YGLEE


static int camdrv_ss_sr130pc20_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_t reg_buffer[],int num_of_regs,char *name)
{
	struct i2c_msg msg = {client->addr, 0, 0, 0};	
    unsigned short subaddr=0, data_value=0;
	
	int next_subaddr;
    int i;
	int index = 0;
	int err = 0;
	err = camdrv_ss_sr130pc20_sensor_burst_write_list(client,reg_buffer,num_of_regs,name);
	
#if 0
    memset(pBurstData, 0, sizeof(pBurstData));
    for(i = 0; i < num_of_regs; i++)
    {

        subaddr = reg_buffer[i] >> 16;
        data_value = reg_buffer[i];

        switch(subaddr)
        {
             case START_BURST_MODE:
            {
                // Start Burst datas
                if(index == 0)
                {
                    pBurstData[index++] = subaddr >> 8;
                    pBurstData[index++] = subaddr & 0xFF;	
                }	

                pBurstData[index++] = data_value >> 8;
                pBurstData[index++] = data_value & 0xFF;	

                // Get Next Address
                if((i+1) == num_of_regs)  // The last code
                {
                    next_subaddr = 0xFFFF;   // Dummy
                }
                else
                {
                    next_subaddr = reg_buffer[i+1]>>16;
                }

                // If next subaddr is different from the current subaddr
                // In other words, if burst mode ends, write the all of the burst datas which were gathered until now
                if(next_subaddr != subaddr) 
                {
                    msg.buf = pBurstData;
                    msg.len = index;

                    err = i2c_transfer(client->adapter, &msg, 1);
                	if(err < 0)
                	{
                		CAM_ERROR_PRINTK("[%s: %d] i2c burst write fail\n", __FILE__, __LINE__);	
                		return -EIO;
                	}

                    // Intialize and gather busrt datas again.
                    index = 0;
                    memset(pBurstData, 0, sizeof(pBurstData));
                }
                break;
            }
            case DELAY_SEQ:
            {
			if(data_value == DELAY_SEQ)
				break;				

				CAM_ERROR_PRINTK("%s : added sleep for  = %d msec in %s !PLEASE CHECK !!! \n", __func__,data_value,name);
                msleep(data_value);
                break;
            }

            case 0xFCFC:
            case 0x0028:
            case 0x002A:
            default:
            {
				if(sr130pc20_REGISTER_SIZE == 4)
				      err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value);
				else if(sr130pc20_REGISTER_SIZE == 2)
				{
					unsigned char subaddr8 =0;
					unsigned char data_value8 =0;
					subaddr8 = data_value >> 8;
					data_value8 = data_value & 0xff;
					  err = camdrv_ss_i2c_write_2_bytes(client, subaddr8, data_value8);
				}
				
            	if(err < 0)
            	{
            		CAM_ERROR_PRINTK("%s :i2c transfer failed ! \n", __func__);
            		return -EIO;
            	}
            	break;
            }            
        }
    }
#endif


    return err;
}

 
static float camdrv_ss_sr130pc20_get_exposureTime(struct v4l2_subdev *sd)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned int read_value1=0,read_value2=0,read_value3=0,read_value4=0;
//	unsigned short lsb = 0, msb = 0;
//	unsigned short a_gain = 0;
//	unsigned char data = 0;
   	 int exposureTime = 0;


	//AE off
	camdrv_ss_i2c_write_2_bytes(client, 0x03, 0x20);
	//camdrv_ss_i2c_write_2_bytes(client, 0x10, 0x0c);

	//camdrv_ss_i2c_read_1_byte(client, 0x80, &read_value1);
	//camdrv_ss_i2c_read_1_byte(client, 0x81, &read_value2);
	//camdrv_ss_i2c_read_1_byte(client, 0x82, &read_value3);
	//exposureTime = (read_value1 << 19 | read_value2 << 11 | read_value3<<3);
	
	/* read exposure time */
	camdrv_ss_i2c_read_1_byte(client,0xa0, &read_value1);
//	msb = (unsigned short)data;
	camdrv_ss_i2c_read_1_byte(client,0xa1, &read_value2);
//	msb = (msb << 8) | (unsigned short)data;
	camdrv_ss_i2c_read_1_byte(client,0xa2, &read_value3);
//	lsb = (unsigned short)data;
	camdrv_ss_i2c_read_1_byte(client,0xa3, &read_value4);
//	lsb = (lsb << 8) | (unsigned short)data;

	exposureTime =  (read_value1 << 24 | read_value2 << 16 | read_value3 << 8 | read_value4);
	CAM_INFO_PRINTK("%s, exposureTime =%d \n",__func__,exposureTime);
	return (exposureTime);
	
}
//aska add
static int camdrv_ss_sr130pc20_get_nightmode(struct v4l2_subdev *sd)
{
	return 0;
};


static int camdrv_ss_sr130pc20_get_iso_speed_rate(struct v4l2_subdev *sd)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned short a_gain = 0;
	unsigned char data = 0;

	/* page mode 0x20 */
	camdrv_ss_i2c_write_2_bytes(client, 0x03, 0x20);
	/* read ISO gain */
	camdrv_ss_i2c_read_1_byte(client,0x50, &data);
	a_gain = (unsigned short)data;
	
	// calculate ISO
	if(a_gain < 0x32 )
		return 50;
	else if(a_gain < 0x5C )
		return 100;
	else if(a_gain < 0x83 )
		return 200;
	else
		return 400;
	 
}

static int camdrv_ss_sr130pc20_get_ae_stable_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{   
#if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    unsigned short AE_stable = 0x0000;
	int rows_num_=0;
    
    //Check AE stable
    err = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x1E3C);
    err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &AE_stable);

    if(err < 0)
    {
        CAM_ERROR_PRINTK("[%s: %d] ERROR! AE stable check\n", __FILE__, __LINE__);
    }

    if(AE_stable == 0x0001)
    {
        ctrl->value = AE_STABLE;
    }
    else
    {
        ctrl->value = AE_UNSTABLE;
    }
#endif

    return 0;
}        
// end 

 

#if 0
static int camdrv_ss_sr130pc20_get_light_condition(struct v4l2_subdev *sd, int *Result)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1, read_value2;
    int NB_value = 0;

    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2A3C);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);   // LSB (0x2A3C)
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value2);   // MSB (0x2A3E)

    NB_value = (int)read_value2;
    NB_value = ((NB_value << 16) | (read_value1 & 0xFFFF));
    
    if(NB_value > 0xFFFE)
    {
        *Result = CAM_HIGH_LIGHT;
	    CAM_INFO_PRINTK("%s : Highlight Read(0x%X) \n", __func__, NB_value);
    }
    else if(NB_value > 0x0020)
    {
        *Result = CAM_NORMAL_LIGHT;
	    CAM_INFO_PRINTK("%s : Normallight Read(0x%X) \n", __func__, NB_value);
    }
    else
    {
        *Result = CAM_LOW_LIGHT;
	    CAM_INFO_PRINTK("%s : Lowlight Read(0x%X) \n", __func__, NB_value);
    }

	return 0;
}
#endif


static bool camdrv_ss_sr130pc20_check_flash_needed(struct v4l2_subdev *sd)
{
#if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0;

    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x29F0);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);

    CAM_INFO_PRINTK( "%s [Read Value : %X]\n", __func__, read_value1);
    
    if(read_value1 >= 0x0708)
    {   
        return true;
    }
    else
    {
        return false;
    }
#endif


	return false;
}


#if 0
static int camdrv_ss_sr130pc20_AAT_flash_control(struct v4l2_subdev *sd, int control_mode)
{
    switch(control_mode)
    {
        // USE FLASH MODE
        case FLASH_CONTROL_MAX_LEVEL:
        {
            if(HWREV >= 0x03)
            {                    
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
            }
            else
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            }
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            gpio_set_value(GPIO_CAM_FLASH_EN, 1);
            break;
        }
    
        // USE FLASH MODE
        case FLASH_CONTROL_HIGH_LEVEL:
        {
            if(HWREV >= 0x03)
            {                    
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
            }
            else
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            }
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            gpio_set_value(GPIO_CAM_FLASH_EN, 1);
            udelay(10);    // Flash Mode Set time

            camdrv_ss_AAT_flash_write_data(3);
            break;
        }

        // USE MOVIE MODE : AF Pre-Flash Mode(Torch Mode)
        case FLASH_CONTROL_MIDDLE_LEVEL:
        {
            if(HWREV >= 0x03)
            {                    
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
            }
            else
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            }
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            camdrv_ss_AAT_flash_write_data(1);
            break;
        }

        // USE MOVIE MODE : Movie Mode(Torch Mode)
        case FLASH_CONTROL_LOW_LEVEL:
        {
            if(HWREV >= 0x03)
            {                    
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
            }
            else
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            }
            gpio_set_value(GPIO_CAM_FLASH_EN, 0); 
            udelay(1);

            camdrv_ss_AAT_flash_write_data(7);   // 69mA
            break;
        }

        case FLASH_CONTROL_OFF:
        default:
        {
            if(HWREV >= 0x03)
            {                    
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
            }
            else
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            }
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            break;
        }        
    }

    return 0;
}
#endif // CONFIG_FLASH_ENABLE


//@HW
//Power (common)

static int camdrv_ss_sr130pc20_copy_files_for_60hz(void)
{
#if 0
#define COPY_FROM_60HZ_TABLE(TABLE_NAME, ANTI_BANDING_SETTING) \
	memcpy (TABLE_NAME, TABLE_NAME##_##ANTI_BANDING_SETTING, \
	sizeof(TABLE_NAME))
	
	CAM_INFO_PRINTK("%s: Enter \n",__func__);

	//[ltn_to_do] should be rearranged !!!!! just for Testing
	COPY_FROM_60HZ_TABLE (sr130pc20_init_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_preview_camera_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_snapshot_normal_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_snapshot_nightmode_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_snapshot_fireworks_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_none_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_nightshot_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_backlight_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_landscape_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_sports_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_party_indoor_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_beach_snow_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_sunset_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_duskdawn_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_fall_color_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_fireworks_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_scene_candle_light_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_fps_15_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_fps_25_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr130pc20_fps_30_regs, 60hz);

	CAM_INFO_PRINTK("%s: copy done!\n", __func__);
#endif
}
static int camdrv_ss_sr130pc20_check_table_size_for_60hz(void)
{
#if 0

#define IS_SAME_NUM_OF_ROWS(TABLE_NAME) \
	(sizeof(TABLE_NAME) == sizeof(TABLE_NAME##_60hz))

	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_init_regs) ) return (-1);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_preview_camera_regs) ) return (-2);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_snapshot_normal_regs) ) return (-3);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_snapshot_nightmode_regs) ) return (-4);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_snapshot_fireworks_regs) ) return (-5);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_none_regs) ) return (-6);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_nightshot_regs) ) return (-7);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_backlight_regs) ) return (-8);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_landscape_regs) ) return (-9);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_sports_regs) ) return (-10);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_party_indoor_regs) ) return (-11);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_beach_snow_regs) ) return (-12);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_sunset_regs) ) return (-13);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_duskdawn_regs) ) return (-14);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_fall_color_regs) ) return (-15);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_fireworks_regs) ) return (-16);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_scene_candle_light_regs) ) return (-17);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_fps_15_regs) ) return (-18);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_fps_25_regs) ) return (-19);
	if ( !IS_SAME_NUM_OF_ROWS(sr130pc20_fps_30_regs) ) return (-20);

	CAM_INFO_PRINTK("%s: Success !\n", __func__);
	return 0;
 #endif
}	


static int camdrv_ss_sr130pc20_sensor_power(int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *lp_clock;
	struct clk *lp_clock_1;
	struct clk *axi_clk;
	struct clk *axi_clk_1;
	static struct pi_mgr_dfs_node unicam_dfs_node;
	CAM_ERROR_PRINTK(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	
	ret = -1;
	lp_clock = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock)) {
		CAM_ERROR_PRINTK(KERN_ERR "Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

#ifdef CONFIG_SOC_CAMERA_SUB_SR130
	lp_clock_1 = clk_get(NULL, CSI1_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock_1)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI1_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}
#endif

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		CAM_ERROR_PRINTK("%s: unable to get clock %s\n", __func__, SENSOR_0_CLK);
		goto e_clk_get;
	}

	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		CAM_ERROR_PRINTK("%s:unable to get clock csi0_axi_clk\n", __func__);
		goto e_clk_get;
	}

#ifdef CONFIG_SOC_CAMERA_SUB_SR130
	axi_clk_1 = clk_get(NULL, "csi1_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		printk(KERN_ERR "Unable to get AXI clock 1\n");
		goto e_clk_get;
	}
#endif


	VCAM_A_2_8_V= regulator_get(NULL, VCAM_A_2_8V_REGULATOR);
	if(IS_ERR(VCAM_A_2_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_A_2_8_V.8V\n");
		return -1;
	}


	VCAM_IO_1_8_V = regulator_get(NULL, VCAM_IO_1_8V_REGULATOR);
	if(IS_ERR(VCAM_IO_1_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_IO_1.8V\n");
		return -1;
	}	


	VCAM_CORE_1_8_V = regulator_get(NULL, VCAM_CORE_1_8V_REGULATOR);
	if(IS_ERR(VCAM_CORE_1_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_CORE_1_8_V\n");
		return -1;
	}	

	
	CAM_INFO_PRINTK("set cam_rst cam_stnby  to low\n");
	gpio_request(CAM_RESET, "cam_rst");
	gpio_direction_output(CAM_RESET,0);
	
	gpio_request(CAM_STNBY, "CAM_STNBY");
	gpio_direction_output(CAM_STNBY,0);
		

	if(on)
	{
  		CAM_INFO_PRINTK("power on the sensor \n"); //@HW

  		value = regulator_set_voltage(VCAM_A_2_8_V, VCAM_A_2_8V_REGULATOR_uV, VCAM_A_2_8V_REGULATOR_uV);
  		if (value)
   		CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_A_2_8_V failed \n", __func__);

   		  value = regulator_set_voltage(VCAM_IO_1_8_V, VCAM_IO_1_8V_REGULATOR_uV, VCAM_IO_1_8V_REGULATOR_uV);
		  if (value)
   		CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_IO_1_8_V failed \n", __func__);


  		value = regulator_set_voltage(VCAM_CORE_1_8_V, VCAM_CORE_1_8V_REGULATOR_uV, VCAM_CORE_1_8V_REGULATOR_uV);
		if (value)
			CAM_ERROR_PRINTK(
			"%s:regulator_set_voltage VCAM_CORE_1_8_V failed\n",
			__func__);

#ifdef CONFIG_SOC_CAMERA_MAIN_SR130
 		 if (mm_ccu_set_pll_select(CSI0_BYTE1_PLL, 8)) {
   			CAM_ERROR_PRINTK("failed to set BYTE1\n");
  			 goto e_clk_pll;
 		 }
 		 if (mm_ccu_set_pll_select(CSI0_BYTE0_PLL, 8)) {
   			CAM_ERROR_PRINTK("failed to set BYTE0\n");
  			 goto e_clk_pll;
  		}
  		if (mm_ccu_set_pll_select(CSI0_CAMPIX_PLL, 8)) {
  			 CAM_ERROR_PRINTK("failed to set PIXPLL\n");
   			goto e_clk_pll;
  		}
#else if CONFIG_SOC_CAMERA_SUB_SR130
		if (mm_ccu_set_pll_select(CSI1_BYTE1_PLL, 8)) {
			pr_err("failed to set BYTE1\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI1_BYTE0_PLL, 8)) {
			pr_err("failed to set BYTE0\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI1_CAMPIX_PLL, 8)) {
			pr_err("failed to set PIXPLL\n");
			goto e_clk_pll;
		}
#endif

		value = clk_enable(axi_clk);
		if (value) {
   			CAM_ERROR_PRINTK("%s:failed to enable csi2 axi clock\n", __func__);
  		 	goto e_clk_axi;
 		 }

  		value = clk_enable(lp_clock);
  		if (value) {
   			CAM_ERROR_PRINTK(KERN_ERR "Failed to enable lp clock\n");
  			 goto e_clk_lp;
		}

 		 value = clk_set_rate(lp_clock, CSI0_LP_FREQ);
  		if (value) {
  			 CAM_ERROR_PRINTK("Failed to set lp clock\n");
 			  goto e_clk_set_lp;
 		 }
#ifdef CONFIG_SOC_CAMERA_SUB_SR130
		value = clk_enable(lp_clock_1);
		if (value) {
			pr_err(KERN_ERR "Failed to enable lp clock 1\n");
			goto e_clk_lp1;
		}

		value = clk_set_rate(lp_clock_1, CSI1_LP_FREQ);
		if (value) {
			pr_err("Failed to set lp clock 1\n");
			goto e_clk_set_lp1;
		}

		value = clk_enable(axi_clk_1);
		if (value) {
			printk(KERN_ERR "Failed to enable axi clock 1\n");
			goto e_clk_axi;
		}
#endif

  		value = regulator_enable(VCAM_IO_1_8_V);
  		if (value) {
   			CAM_ERROR_PRINTK("%s:failed to enable VCAM_IO_1_8_V\n", __func__);
			return -1;
		}
		msleep(5);

  		value = regulator_enable(VCAM_A_2_8_V);
		  if (value) {
   			CAM_ERROR_PRINTK("%s:failed to enable VCAM_A_2_8_V\n", __func__);
  			 return -1;
 		 }
		 msleep(5);
 		 value = regulator_enable(VCAM_CORE_1_8_V);
 		 if (value) {
   			CAM_ERROR_PRINTK("%s:failed to enable VCAM_CORE_1_8_V\n", __func__);
 			  return -1;
 		 }	
		msleep(3);
		value = clk_enable(clock);
		if (value) {
   			CAM_ERROR_PRINTK("%s: failed to enable clock %s\n", __func__,SENSOR_0_CLK);
			goto e_clk_clock;
		}

		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
   			CAM_ERROR_PRINTK("%s: failed to set the clock %s to freq %d\n",__func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			goto e_clk_set_clock;
		}

		msleep(20);
  
		gpio_set_value(CAM_STNBY,1);
		msleep(50);
 
		gpio_set_value(CAM_RESET,1);
		msleep(20);
	}

	else
	{
		CAM_INFO_PRINTK("power on the sensor \n"); 
		regulator_disable(VCAM_IO_1_8_V);
		msleep(10);
		regulator_disable(VCAM_A_2_8_V);
		regulator_disable(VCAM_CORE_1_8_V);
		gpio_set_value(CAM_STNBY,0);
		clk_disable(clock);
		msleep(3);
		gpio_set_value(CAM_RESET,0);
		msleep(3);
	
		clk_disable(lp_clock);
	#ifdef CONFIG_SOC_CAMERA_SUB_SR130
		clk_disable(lp_clock_1);
	#endif
		clk_disable(clock);
		clk_disable(axi_clk);
	#ifdef CONFIG_SOC_CAMERA_SUB_SR130
		clk_disable(axi_clk_1);
	#endif
		msleep(1);

		CAM_INFO_PRINTK("powerOFF the sensor\n");

	}
#if 0
	if (ANTI_BANDING_60HZ == camera_antibanding_get()) {
		ret = camdrv_ss_sr130pc20_check_table_size_for_60hz();
		if(ret != 0) {
			printk("%s: Fail - the table num is %d \n", __func__, ret);
			return -1;
		}
		camdrv_ss_sr130pc20_copy_files_for_60hz();
	}
#endif
	return 0;
e_clk_set_clock:
	clk_disable(clock);
e_clk_clock:
e_clk_axi:
clk_disable(axi_clk);
#ifdef CONFIG_SOC_CAMERA_SUB_SR130
clk_disable(axi_clk_1);
e_clk_set_lp1:
	clk_disable(lp_clock_1);
#endif
e_clk_lp1:
e_clk_set_lp:
	clk_disable(lp_clock);
e_clk_lp:
e_clk_pll:
e_clk_get:
	return ret;

}
//power end
#define SENSOR_0_PCLK_FREQ		(52000000)//(104000000) //@HW

int camdrv_ss_sr130pc20_get_sensor_param_for_exif(
	struct v4l2_subdev *sd,
	struct v4l2_exif_sensor_info *exif_param)
{
	char str[20];
	int num = -1;
	int ret = -1;
	//float exposureTime = 0.0f;
	int exposureTime = 0;

	strcpy(exif_param->strSoftware,		EXIF_SOFTWARE);
	strcpy(exif_param->strMake,		EXIF_MAKE);
	strcpy(exif_param->strModel,		EXIF_MODEL);

	exposureTime = (int)camdrv_ss_sr130pc20_get_exposureTime(sd);
	CAM_INFO_PRINTK("%s :exposureTime num=  %d \n",__func__,exposureTime);
	if (exposureTime > 0) 
	{
		//sprintf(str, "%d/13000000", exposureTime);
		sprintf(str, "%d/27300000", exposureTime);
		strcpy(exif_param->exposureTime, str);
	} 
	else 
	{
		strcpy(exif_param->exposureTime, "");
	}

	num = (SENSOR_0_PCLK_FREQ/2)/(int)exposureTime;
	CAM_INFO_PRINTK("%s :shutterSpeed num=  %d fps\n",__func__,num);
	if (num > 0) 
	{
		//snprintf(str, 19, "%d/26000000", num);
		sprintf(str, "%d", num);
		//strcpy(exif_param->shutterSpeed, str);
	} 
	else 
	{
		strcpy(exif_param->shutterSpeed, "");
	}
	
	num = camdrv_ss_sr130pc20_get_iso_speed_rate(sd);
	CAM_INFO_PRINTK("%s :isoSpeedRating =  %d \n",__func__,num);
	if (num > 0) {
		sprintf(str, "%d,", num);
		strcpy(exif_param->isoSpeedRating, str);
	} else {
		strcpy(exif_param->isoSpeedRating, "");
	}

	/* sRGB mandatory field! */
	strcpy(exif_param->colorSpaceInfo,	"1");

	strcpy(exif_param->contrast,		"0");
	strcpy(exif_param->saturation,		"0");
	strcpy(exif_param->sharpness,		"0");

	strcpy(exif_param->FNumber,		"28/10");
	strcpy(exif_param->exposureProgram,	"");
	strcpy(exif_param->shutterSpeed,	"");
	strcpy(exif_param->aperture,		"");
	strcpy(exif_param->brightness,		"");
	strcpy(exif_param->exposureBias,	"");
	strcpy(exif_param->maxLensAperture,	"");
	strcpy(exif_param->flash,		"");
	strcpy(exif_param->lensFocalLength,	"270/100");
	strcpy(exif_param->userComments,	"User Comments");
	ret = 0;
	
	return ret;
		
}

void camdrv_ss_sensor_main_name( struct camdrv_ss_sensor_cap *sensor)
{
     strcpy(sensor->name, sr130pc20_NAME);

}


static int camdrv_ss_sr130pc20_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{

		struct i2c_client *client = v4l2_get_subdevdata(sd);
		struct camdrv_ss_state *state = to_state(sd);
		int err = 0;
	
	CAM_INFO_PRINTK( "%s : value =%d\n", __func__, ctrl->value);
#if 1
		if (state->current_mode == PICTURE_MODE) {
			return 0;
		}
#endif
	
		switch (ctrl->value) {
		case SCENE_MODE_NONE:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_none_regs, ARRAY_SIZE(sr130pc20_scene_none_regs), "scene_none_regs");
			break;
		}
	
		case SCENE_MODE_PORTRAIT:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_portrait_regs, ARRAY_SIZE(sr130pc20_scene_portrait_regs), "scene_portrait_regs");
			break;
		}
	
		case SCENE_MODE_NIGHTSHOT:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_nightshot_regs, ARRAY_SIZE(sr130pc20_scene_nightshot_regs), "scene_nightshot_regs");
			break;
		}
	
		case SCENE_MODE_BACK_LIGHT:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_backlight_regs, ARRAY_SIZE(sr130pc20_scene_backlight_regs), "scene_backlight_regs");
			break;
		}
	
		case SCENE_MODE_LANDSCAPE:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_landscape_regs, ARRAY_SIZE(sr130pc20_scene_landscape_regs), "scene_landscape_regs");
			break;
		}
	
		case SCENE_MODE_SPORTS:
		{
				err = camdrv_ss_i2c_set_config_register(client,sr130pc20_scene_sports_regs, ARRAY_SIZE(sr130pc20_scene_sports_regs), "scene_sports_regs");
			break;
		}
	
		case SCENE_MODE_PARTY_INDOOR:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_party_indoor_regs, ARRAY_SIZE(sr130pc20_scene_party_indoor_regs), "scene_party_indoor_regs");
			break;
		}
	
		case SCENE_MODE_BEACH_SNOW:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_beach_snow_regs, ARRAY_SIZE(sr130pc20_scene_beach_snow_regs), "scene_beach_snow_regs");
			break;
		}
	
		case SCENE_MODE_SUNSET:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_sunset_regs, ARRAY_SIZE(sr130pc20_scene_sunset_regs), "scene_sunset_regs");
			break;
		}
	
		case SCENE_MODE_DUSK_DAWN:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_duskdawn_regs,  ARRAY_SIZE(sr130pc20_scene_duskdawn_regs), "scene_duskdawn_regs");
			break;
		}
	
		case SCENE_MODE_FALL_COLOR:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_fall_color_regs, ARRAY_SIZE(sr130pc20_scene_fall_color_regs), "scene_fall_color_regs");
			break;
		}
	
		case SCENE_MODE_FIREWORKS:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_fireworks_regs,  ARRAY_SIZE(sr130pc20_scene_fireworks_regs), "scene_fireworks_regs");
			break;
		}
	
		case SCENE_MODE_TEXT:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_text_regs,  ARRAY_SIZE(sr130pc20_scene_text_regs), "scene_text_regs");
			break;
		}
	
		case SCENE_MODE_CANDLE_LIGHT:
		{
				err = camdrv_ss_i2c_set_config_register(client, sr130pc20_scene_candle_light_regs, ARRAY_SIZE(sr130pc20_scene_candle_light_regs), "scene_candle_light_regs");
			break;
		}
	
		default:
		{
			CAM_ERROR_PRINTK("%s : default not supported !!!\n", __func__);
			err = -EINVAL;
			break;
		}
		}
	
		state->currentScene = ctrl->value;
	
		return err;
}

bool camdrv_ss_sr130pc20_get_esd_status(struct v4l2_subdev *sd)
{
	return false;
}

bool camdrv_ss_sensor_functions_sr130pc20(struct camdrv_ss_sensor_cap *sensor)
{

	strcpy(sensor->name,sr130pc20_NAME);
	sensor->supported_preview_framesize_list  = sr130pc20_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(sr130pc20_supported_preview_framesize_list);
	
	sensor->supported_capture_framesize_list  =  sr130pc20_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(sr130pc20_supported_capture_framesize_list);
	sensor->skip_frames=1;
	sensor->fmts 				   = sr130pc20_fmts;
	sensor->rows_num_fmts		   =ARRAY_SIZE(sr130pc20_fmts);


	sensor->controls				   =sr130pc20_controls;
	sensor->rows_num_controls	      =ARRAY_SIZE(sr130pc20_controls);
	
	sensor->default_pix_fmt 				   = SR130PC20_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt			   = SR130PC20_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size 		  			 = SR130PC20_REGISTER_SIZE;

	sensor->delay_duration				= SR130PC20_DELAY_DURATION;

	/* sensor dependent functions */
	
/* mandatory*/
	sensor->thumbnail_ioctl			       = camdrv_ss_sr130pc20_ss_ioctl;
	sensor->enum_frameintervals			   = camdrv_ss_sr130pc20_enum_frameintervals;
	
/*optional*/
	sensor->get_nightmode		   		 = camdrv_ss_sr130pc20_get_nightmode; //aska add
	sensor->set_preview_start		  =  camdrv_ss_sr130pc20_set_preview_start;
	sensor->set_capture_start      = camdrv_ss_sr130pc20_set_capture_start;//aska add
	sensor->set_iso      					 = camdrv_ss_sr130pc20_set_iso;//aska add
	sensor->set_white_balance      = camdrv_ss_sr130pc20_set_white_balance;//aska add
	sensor->get_ae_stable_status      =  camdrv_ss_sr130pc20_get_ae_stable_status;
//	sensor->set_auto_focus		 	  =  camdrv_ss_sr130pc20_set_auto_focus;
//	sensor->get_auto_focus_status     = camdrv_ss_sr130pc20_get_auto_focus_status;

//	sensor->set_touch_focus		 	  =  camdrv_ss_sr130pc20_set_touch_focus;
//	sensor->get_touch_focus_status     = camdrv_ss_sr130pc20_get_touch_focus_status;

//	sensor->flash_control    	   = camdrv_ss_sr130pc20_AAT_flash_control;
	sensor->i2c_set_data_burst   	   = camdrv_ss_sr130pc20_i2c_set_data_burst; //YGLEE
	sensor->check_flash_needed   	   = camdrv_ss_sr130pc20_check_flash_needed;
//	sensor->get_light_condition   = camdrv_ss_sr130pc20_get_light_condition;
	
	sensor->sensor_power			= camdrv_ss_sr130pc20_sensor_power;		  

	sensor->get_exif_sensor_info =
		camdrv_ss_sr130pc20_get_sensor_param_for_exif;
	sensor->getEsdStatus 		=     camdrv_ss_sr130pc20_get_esd_status;
	
	sensor->set_scene_mode 			   = camdrv_ss_sr130pc20_set_scene_mode;   //denis

	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */
	
	sensor->init_regs						  = sr130pc20_init_regs;
	sensor->rows_num_init_regs				  = ARRAY_SIZE(sr130pc20_init_regs);
	
	sensor->preview_camera_regs 			  = sr130pc20_preview_camera_regs;
	sensor->rows_num_preview_camera_regs 	  = ARRAY_SIZE(sr130pc20_preview_camera_regs);

	sensor->preview_50hz_setting 			  = sr130pc20_preview_50hz_setting;
	sensor->rows_num_preview_50hz_setting 	  = ARRAY_SIZE(sr130pc20_preview_50hz_setting);
	sensor->preview_60hz_setting 			  = sr130pc20_preview_60hz_setting;
	sensor->rows_num_preview_60hz_setting 	  = ARRAY_SIZE(sr130pc20_preview_60hz_setting);
			
	/*snapshot mode*/
	sensor->snapshot_normal_regs			  =	sr130pc20_snapshot_normal_regs;
	sensor->rows_num_snapshot_normal_regs	  = ARRAY_SIZE(sr130pc20_snapshot_normal_regs);

	sensor->snapshot_lowlight_regs			  =	sr130pc20_snapshot_lowlight_regs;
	sensor->rows_num_snapshot_lowlight_regs	  = ARRAY_SIZE(sr130pc20_snapshot_lowlight_regs);


	sensor->snapshot_nightmode_regs			  =	sr130pc20_snapshot_nightmode_regs;
	sensor->rows_num_snapshot_nightmode_regs	  = ARRAY_SIZE(sr130pc20_snapshot_nightmode_regs);
#if 0

	sensor->snapshot_highlight_regs			  =	sr130pc20_snapshot_highlight_regs; 
	sensor->rows_num_snapshot_highlight_regs	  = ARRAY_SIZE(sr130pc20_snapshot_highlight_regs);

	sensor->snapshot_flash_on_regs			  =	sr130pc20_snapshot_flash_on_regs;
	sensor->rows_num_snapshot_flash_on_regs	  = ARRAY_SIZE(sr130pc20_snapshot_flash_on_regs);

	sensor->snapshot_af_preflash_on_regs			  =	sr130pc20_snapshot_af_preflash_on_regs;
	sensor->rows_num_snapshot_af_preflash_on_regs	  = ARRAY_SIZE(sr130pc20_snapshot_af_preflash_on_regs);

	sensor->snapshot_af_preflash_off_regs			  =	sr130pc20_snapshot_af_preflash_off_regs;
	sensor->rows_num_snapshot_af_preflash_off_regs	  = ARRAY_SIZE(sr130pc20_snapshot_af_preflash_off_regs);


	sensor->focus_mode_macro_regs			  =	sr130pc20_af_macro_mode_regs;
	sensor->rows_num_focus_mode_macro_regs	  = ARRAY_SIZE(sr130pc20_af_macro_mode_regs);

	sensor->focus_mode_off_regs			  =	sr130pc20_af_normal_mode_regs;
	sensor->rows_num_focus_mode_off_regs	  = ARRAY_SIZE(sr130pc20_af_normal_mode_regs);

	sensor->single_af_start_regs			  =	sr130pc20_single_af_start_regs;
	sensor->rows_num_single_af_start_regs	  = ARRAY_SIZE(sr130pc20_single_af_start_regs);

	sensor->get_1st_af_search_status			  =	sr130pc20_get_1st_af_search_status;
	sensor->rows_num_get_1st_af_search_status	  = ARRAY_SIZE(sr130pc20_get_1st_af_search_status);

	sensor->get_2nd_af_search_status			  =	sr130pc20_get_2nd_af_search_status;
	sensor->rows_num_get_2nd_af_search_status	  = ARRAY_SIZE(sr130pc20_get_2nd_af_search_status);

	sensor->single_af_stop_regs			  =	sr130pc20_single_af_stop_regs;
	sensor->rows_num_single_af_stop_regs	  = ARRAY_SIZE(sr130pc20_single_af_stop_regs);
	/*effect*/
 #endif
	sensor->effect_normal_regs			      =	sr130pc20_effect_normal_regs;
	sensor->rows_num_effect_normal_regs      = ARRAY_SIZE(sr130pc20_effect_normal_regs);
	
	sensor->effect_negative_regs		      =	sr130pc20_effect_negative_regs;
	sensor->rows_num_effect_negative_regs	 = ARRAY_SIZE(sr130pc20_effect_negative_regs);
	
	sensor->effect_sepia_regs			      =	sr130pc20_effect_sepia_regs;
	sensor->rows_num_effect_sepia_regs	  	  = ARRAY_SIZE(sr130pc20_effect_sepia_regs);
	
	sensor->effect_mono_regs			      =	sr130pc20_effect_mono_regs;
	sensor->rows_num_effect_mono_regs	      = ARRAY_SIZE(sr130pc20_effect_mono_regs);

	sensor->effect_aqua_regs				  =	sr130pc20_effect_aqua_regs;
	sensor->rows_num_effect_aqua_regs	  	  = ARRAY_SIZE(sr130pc20_effect_aqua_regs);
	
	sensor->effect_sharpen_regs 		      =	sr130pc20_effect_sharpen_regs;
	sensor->rows_num_effect_sharpen_regs     = ARRAY_SIZE(sr130pc20_effect_sharpen_regs);
	
	sensor->effect_solarization_regs		   = sr130pc20_effect_solarization_regs;
	sensor->rows_num_effect_solarization_regs = ARRAY_SIZE(sr130pc20_effect_solarization_regs);
	
	sensor->effect_black_white_regs 	       =	sr130pc20_effect_black_white_regs;
	sensor->rows_num_effect_black_white_regs  = ARRAY_SIZE(sr130pc20_effect_black_white_regs);
	

	/*wb*/
	sensor->wb_auto_regs				  =	sr130pc20_wb_auto_regs;
	sensor->rows_num_wb_auto_regs	  	  = ARRAY_SIZE(sr130pc20_wb_auto_regs);

	//sensor->wb_sunny_regs				 =	sr130pc20_wb_sunny_regs;
	//sensor->rows_num_wb_sunny_regs	  	 = ARRAY_SIZE(sr130pc20_wb_sunny_regs);
	
	sensor->wb_cloudy_regs				 =	sr130pc20_wb_cloudy_regs;
	sensor->rows_num_wb_cloudy_regs	 = ARRAY_SIZE(sr130pc20_wb_cloudy_regs);
	
	//sensor->wb_tungsten_regs			 =	sr130pc20_wb_tungsten_regs;
	//sensor->rows_num_wb_tungsten_regs	 = ARRAY_SIZE(sr130pc20_wb_tungsten_regs);
	//Changed reg table name to fit UI's name
	sensor->wb_daylight_regs				 =	sr130pc20_wb_daylight_regs;
	sensor->rows_num_wb_daylight_regs	  	 = ARRAY_SIZE(sr130pc20_wb_daylight_regs);
	sensor->wb_incandescent_regs				 =	sr130pc20_wb_incandescent_regs;
	sensor->rows_num_wb_incandescent_regs	  	 = ARRAY_SIZE(sr130pc20_wb_incandescent_regs);

	sensor->wb_fluorescent_regs 		  =	sr130pc20_wb_fluorescent_regs;
	sensor->rows_num_wb_fluorescent_regs  = ARRAY_SIZE(sr130pc20_wb_fluorescent_regs);

	//sensor->wb_cwf_regs 		  =	sr130pc20_wb_cwf_regs;
	//sensor->rows_num_wb_cwf_regs  = ARRAY_SIZE(sr130pc20_wb_cwf_regs);


	/*metering*/
	sensor->metering_matrix_regs		  =	sr130pc20_metering_matrix_regs;
	sensor->rows_num_metering_matrix_regs	  	  = ARRAY_SIZE(sr130pc20_metering_matrix_regs);

	sensor->metering_center_regs		  =	sr130pc20_metering_center_regs;
	sensor->rows_num_metering_center_regs	  	  = ARRAY_SIZE(sr130pc20_metering_center_regs);

	sensor->metering_spot_regs			  =	  sr130pc20_metering_spot_regs;
	sensor->rows_num_metering_spot_regs	  		  = ARRAY_SIZE(sr130pc20_metering_spot_regs);
	
	/*EV*/
	sensor->ev_minus_4_regs 			 =	sr130pc20_ev_minus_4_regs;
	sensor->rows_num_ev_minus_4_regs	 = ARRAY_SIZE(sr130pc20_ev_minus_4_regs);

	sensor->ev_minus_3_regs 			 =	sr130pc20_ev_minus_3_regs;
	sensor->rows_num_ev_minus_3_regs	 = ARRAY_SIZE(sr130pc20_ev_minus_3_regs);

	sensor->ev_minus_2_regs 			 =	sr130pc20_ev_minus_2_regs;
	sensor->rows_num_ev_minus_2_regs	  = ARRAY_SIZE(sr130pc20_ev_minus_2_regs);

	sensor->ev_minus_1_regs 			 =	sr130pc20_ev_minus_1_regs;
	sensor->rows_num_ev_minus_1_regs	 = ARRAY_SIZE(sr130pc20_ev_minus_1_regs);

	sensor->ev_default_regs 			 =	sr130pc20_ev_default_regs;
	sensor->rows_num_ev_default_regs	 = ARRAY_SIZE(sr130pc20_ev_default_regs);

	sensor->ev_plus_1_regs				 =	sr130pc20_ev_plus_1_regs;
	sensor->rows_num_ev_plus_1_regs	 = ARRAY_SIZE(sr130pc20_ev_plus_1_regs);

	sensor->ev_plus_2_regs				 =	sr130pc20_ev_plus_2_regs;
	sensor->rows_num_ev_plus_2_regs	 = ARRAY_SIZE(sr130pc20_ev_plus_2_regs);

	sensor->ev_plus_3_regs				 =	sr130pc20_ev_plus_3_regs;
	sensor->rows_num_ev_plus_3_regs	 = ARRAY_SIZE(sr130pc20_ev_plus_3_regs);

	sensor->ev_plus_4_regs				 =	sr130pc20_ev_plus_4_regs;
	sensor->rows_num_ev_plus_4_regs	 = ARRAY_SIZE(sr130pc20_ev_plus_4_regs);

	
	/*contrast*/
	sensor->contrast_minus_2_regs		 	 =	sr130pc20_contrast_minus_2_regs;
	sensor->rows_num_contrast_minus_2_regs	 = ARRAY_SIZE(sr130pc20_contrast_minus_2_regs);

	sensor->contrast_minus_1_regs		     =	sr130pc20_contrast_minus_1_regs;
	sensor->rows_num_contrast_minus_1_regs	 = ARRAY_SIZE(sr130pc20_contrast_minus_1_regs);
  
	sensor->contrast_default_regs			 =	sr130pc20_contrast_default_regs;
	sensor->rows_num_contrast_default_regs  = ARRAY_SIZE(sr130pc20_contrast_default_regs);

	sensor->contrast_plus_1_regs			 =	sr130pc20_contrast_plus_1_regs;
	sensor->rows_num_contrast_plus_1_regs	 = ARRAY_SIZE(sr130pc20_contrast_plus_1_regs);

	sensor->contrast_plus_2_regs			 =	sr130pc20_contrast_plus_2_regs;
	sensor->rows_num_contrast_plus_2_regs	 = ARRAY_SIZE(sr130pc20_contrast_plus_2_regs);
	
	/*sharpness*/ 
	sensor->sharpness_minus_3_regs		     =	sr130pc20_sharpness_minus_3_regs;
	sensor->rows_num_sharpness_minus_3_regs= ARRAY_SIZE(sr130pc20_sharpness_minus_3_regs);

	sensor->sharpness_minus_2_regs		     =	sr130pc20_sharpness_minus_2_regs;
	sensor->rows_num_sharpness_minus_2_regs= ARRAY_SIZE(sr130pc20_sharpness_minus_2_regs);

	sensor->sharpness_minus_1_regs		 	 =	sr130pc20_sharpness_minus_1_regs;
	sensor->rows_num_sharpness_minus_1_regs = ARRAY_SIZE(sr130pc20_sharpness_minus_1_regs);

	sensor->sharpness_default_regs		 	 =	sr130pc20_sharpness_default_regs;
	sensor->rows_num_sharpness_default_regs  = ARRAY_SIZE(sr130pc20_sharpness_default_regs);

	sensor->sharpness_plus_1_regs		     =	sr130pc20_sharpness_plus_1_regs;
	sensor->rows_num_sharpness_plus_1_regs	 =	ARRAY_SIZE(sr130pc20_sharpness_plus_1_regs);

	sensor->sharpness_plus_2_regs		     =	sr130pc20_sharpness_plus_2_regs;
	sensor->rows_num_sharpness_plus_2_regs	 =	ARRAY_SIZE(sr130pc20_sharpness_plus_2_regs);
    
	sensor->sharpness_plus_3_regs		     =	sr130pc20_sharpness_plus_3_regs;
	sensor->rows_num_sharpness_plus_3_regs	 =	ARRAY_SIZE(sr130pc20_sharpness_plus_3_regs);

		
	/*saturation*/
	sensor->saturation_minus_2_regs 	      =	sr130pc20_saturation_minus_2_regs;
	sensor->rows_num_saturation_minus_2_regs = ARRAY_SIZE(sr130pc20_saturation_minus_2_regs);

	sensor->saturation_minus_1_regs 	 	  =	sr130pc20_saturation_minus_1_regs;
	sensor->rows_num_saturation_minus_1_regs = ARRAY_SIZE(sr130pc20_saturation_minus_1_regs);

	sensor->saturation_default_regs 	      =	sr130pc20_saturation_default_regs;
	sensor->rows_num_saturation_default_regs  = ARRAY_SIZE(sr130pc20_saturation_default_regs);

	sensor->saturation_plus_1_regs		       =	sr130pc20_saturation_plus_1_regs;
	sensor->rows_num_saturation_plus_1_regs	= ARRAY_SIZE(sr130pc20_saturation_plus_1_regs);

	sensor->saturation_plus_2_regs		       =	sr130pc20_saturation_plus_2_regs;
	sensor->rows_num_saturation_plus_2_regs   = ARRAY_SIZE(sr130pc20_saturation_plus_2_regs);

	
	/*zoom*/
	sensor->zoom_00_regs					 =	sr130pc20_zoom_00_regs;
	sensor->rows_num_zoom_00_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_00_regs);

	sensor->zoom_01_regs					 =	sr130pc20_zoom_01_regs;
	sensor->rows_num_zoom_01_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_01_regs);

	sensor->zoom_02_regs					 =	sr130pc20_zoom_02_regs;
	sensor->rows_num_zoom_02_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_02_regs);

	sensor->zoom_03_regs					 =	sr130pc20_zoom_03_regs;
	sensor->rows_num_zoom_03_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_03_regs);

	sensor->zoom_04_regs					 =	sr130pc20_zoom_04_regs;
	sensor->rows_num_zoom_04_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_04_regs);

	sensor->zoom_05_regs					 =	sr130pc20_zoom_05_regs;
	sensor->rows_num_zoom_05_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_05_regs);

	sensor->zoom_06_regs					 =	sr130pc20_zoom_06_regs;
	sensor->rows_num_zoom_06_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_06_regs);

	sensor->zoom_07_regs					 =	sr130pc20_zoom_07_regs;
	sensor->rows_num_zoom_07_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_07_regs);

	sensor->zoom_08_regs					 =	sr130pc20_zoom_08_regs;
	sensor->rows_num_zoom_08_regs	  		  = ARRAY_SIZE(sr130pc20_zoom_08_regs);

	
	/*scene mode*/
	sensor->scene_none_regs 			 		=	sr130pc20_scene_none_regs;
	sensor->rows_num_scene_none_regs	  		 = ARRAY_SIZE(sr130pc20_scene_none_regs);

	sensor->scene_portrait_regs 		 		=	sr130pc20_scene_portrait_regs;
	sensor->rows_num_scene_portrait_regs	  	= ARRAY_SIZE(sr130pc20_scene_portrait_regs);

	sensor->scene_nightshot_regs			   =	sr130pc20_scene_nightshot_regs;
	sensor->rows_num_scene_nightshot_regs	  	 = ARRAY_SIZE(sr130pc20_scene_nightshot_regs);

	sensor->scene_backlight_regs			  =	sr130pc20_scene_backlight_regs;
	sensor->rows_num_scene_backlight_regs	   = ARRAY_SIZE(sr130pc20_scene_backlight_regs);

	sensor->scene_landscape_regs			   =	sr130pc20_scene_landscape_regs;
	sensor->rows_num_scene_landscape_regs	  	 = ARRAY_SIZE(sr130pc20_scene_landscape_regs);

	sensor->scene_sports_regs			      =	sr130pc20_scene_sports_regs;
	sensor->rows_num_scene_sports_regs	  	 = ARRAY_SIZE(sr130pc20_scene_sports_regs);

	sensor->scene_party_indoor_regs 	 	  =	sr130pc20_scene_party_indoor_regs;
	sensor->rows_num_scene_party_indoor_regs  = ARRAY_SIZE(sr130pc20_scene_party_indoor_regs);

	sensor->scene_beach_snow_regs				 =	sr130pc20_scene_beach_snow_regs;
	sensor->rows_num_scene_beach_snow_regs	  	 = ARRAY_SIZE(sr130pc20_scene_beach_snow_regs);

	sensor->scene_sunset_regs			 		 =	sr130pc20_scene_sunset_regs;
	sensor->rows_num_scene_sunset_regs	  		  = ARRAY_SIZE(sr130pc20_scene_sunset_regs);

	sensor->scene_duskdawn_regs 				 =	sr130pc20_scene_duskdawn_regs;
	sensor->rows_num_scene_duskdawn_regs	  	 = ARRAY_SIZE(sr130pc20_scene_duskdawn_regs);

	sensor->scene_fall_color_regs				 =	sr130pc20_scene_fall_color_regs;
	sensor->rows_num_scene_fall_color_regs	  	  = ARRAY_SIZE(sr130pc20_scene_fall_color_regs);

	sensor->scene_fireworks_regs				 =	sr130pc20_scene_fireworks_regs;
	sensor->rows_num_scene_fireworks_regs	  	  = ARRAY_SIZE(sr130pc20_scene_fireworks_regs);
	
	sensor->scene_candle_light_regs 	 		=	sr130pc20_scene_candle_light_regs;
	sensor->rows_num_scene_candle_light_regs	= ARRAY_SIZE(sr130pc20_scene_candle_light_regs);

	sensor->scene_text_regs			   =	sr130pc20_scene_text_regs;
	sensor->rows_num_scene_text_regs	  	 = ARRAY_SIZE(sr130pc20_scene_text_regs);

		
	/*fps*/
	sensor->fps_auto_regs				 =	sr130pc20_fps_auto_regs;
	sensor->rows_num_fps_auto_regs	  		  = ARRAY_SIZE(sr130pc20_fps_auto_regs);

	sensor->fps_5_regs					 =	sr130pc20_fps_5_regs;
	sensor->rows_num_fps_5_regs	  		  = ARRAY_SIZE(sr130pc20_fps_5_regs);

	sensor->fps_7_regs					 =	sr130pc20_fps_7_regs;
	sensor->rows_num_fps_7_regs	  		  = ARRAY_SIZE(sr130pc20_fps_7_regs);

	sensor->fps_10_regs 				 =	sr130pc20_fps_10_regs;
	sensor->rows_num_fps_10_regs	  		  = ARRAY_SIZE(sr130pc20_fps_10_regs);

	sensor->fps_15_regs 				 =	sr130pc20_fps_15_regs;
	sensor->rows_num_fps_15_regs	  		  = ARRAY_SIZE(sr130pc20_fps_15_regs);

	sensor->fps_20_regs 				 =	sr130pc20_fps_20_regs;
	sensor->rows_num_fps_20_regs	  		  = ARRAY_SIZE(sr130pc20_fps_20_regs);

	sensor->fps_25_regs 				 =	sr130pc20_fps_25_regs;
	sensor->rows_num_fps_25_regs	  		  = ARRAY_SIZE(sr130pc20_fps_25_regs);

	sensor->fps_30_regs 				 =	sr130pc20_fps_30_regs;
	sensor->rows_num_fps_30_regs 		  = ARRAY_SIZE(sr130pc20_fps_30_regs);
	
	sensor->fps_60_regs 				 =	sr130pc20_fps_60_regs;
	sensor->rows_num_fps_60_regs	  		  = ARRAY_SIZE(sr130pc20_fps_60_regs);

	sensor->fps_120_regs 				 =	sr130pc20_fps_120_regs;
	sensor->rows_num_fps_120_regs	  		  = ARRAY_SIZE(sr130pc20_fps_120_regs);
	

	
	/*quality*/
	sensor->quality_superfine_regs			 =	sr130pc20_quality_superfine_regs;
	sensor->rows_num_quality_superfine_regs	  = ARRAY_SIZE(sr130pc20_quality_superfine_regs);

	sensor->quality_fine_regs			 =	sr130pc20_quality_fine_regs;
	sensor->rows_num_quality_fine_regs	  = ARRAY_SIZE(sr130pc20_quality_fine_regs);

	sensor->quality_normal_regs 		   =	sr130pc20_quality_normal_regs;
	sensor->rows_num_quality_normal_regs  = ARRAY_SIZE(sr130pc20_effect_normal_regs);

	sensor->quality_economy_regs			 =	sr130pc20_quality_economy_regs;
	sensor->rows_num_quality_economy_regs   = ARRAY_SIZE(sr130pc20_quality_economy_regs);

	
	/*preview size */
	sensor->preview_size_176x144_regs	        =	sr130pc20_preview_size_176x144_regs;
	sensor->rows_num_preview_size_176x144_regs	 = ARRAY_SIZE(sr130pc20_preview_size_176x144_regs);

	sensor->preview_size_320x240_regs	         =	sr130pc20_preview_size_320x240_regs; 
	sensor->rows_num_preview_size_320x240_regs	  = ARRAY_SIZE(sr130pc20_preview_size_320x240_regs);

	sensor->preview_size_352x288_regs	          =	sr130pc20_preview_size_352x288_regs; 
	sensor->rows_num_preview_size_352x288_regs	  = ARRAY_SIZE(sr130pc20_preview_size_352x288_regs);

	sensor->preview_size_640x480_regs	          =	sr130pc20_preview_size_640x480_regs; 
	sensor->rows_num_preview_size_640x480_regs	  = ARRAY_SIZE(sr130pc20_preview_size_640x480_regs);

	sensor->preview_size_704x576_regs	 		=	sr130pc20_preview_size_704x576_regs; 
	sensor->rows_num_preview_size_704x576_regs	 = ARRAY_SIZE(sr130pc20_preview_size_704x576_regs);

	sensor->preview_size_720x480_regs	 		=	sr130pc20_preview_size_720x480_regs; 
	sensor->rows_num_preview_size_720x480_regs	 = ARRAY_SIZE(sr130pc20_preview_size_720x480_regs);
	
	sensor->preview_size_800x480_regs	        =	sr130pc20_preview_size_800x480_regs;
	sensor->rows_num_preview_size_800x480_regs	 = ARRAY_SIZE(sr130pc20_preview_size_800x480_regs);

	sensor->preview_size_800x600_regs	        =	sr130pc20_preview_size_800x600_regs;
	sensor->rows_num_preview_size_800x600_regs	 = ARRAY_SIZE(sr130pc20_preview_size_800x600_regs);

	sensor->preview_size_1024x600_regs	         =	sr130pc20_preview_size_1024x600_regs; 
	sensor->rows_num_preview_size_1024x600_regs	  = ARRAY_SIZE(sr130pc20_preview_size_1024x600_regs);

	sensor->preview_size_1024x768_regs	          =	sr130pc20_preview_size_1024x768_regs; 
	sensor->rows_num_preview_size_1024x768_regs	  = ARRAY_SIZE(sr130pc20_preview_size_1024x768_regs);

	sensor->preview_size_1280x960_regs	          =	sr130pc20_preview_size_1280x960_regs; 
	sensor->rows_num_preview_size_1280x960_regs	  = ARRAY_SIZE(sr130pc20_preview_size_1280x960_regs);

	sensor->preview_size_1600x960_regs	 		=	sr130pc20_preview_size_1600x960_regs; 
	sensor->rows_num_preview_size_1600x960_regs	 = ARRAY_SIZE(sr130pc20_preview_size_1600x960_regs);

	sensor->preview_size_1600x1200_regs	 		=	sr130pc20_preview_size_1600x1200_regs; 
	sensor->rows_num_preview_size_1600x1200_regs	 = ARRAY_SIZE(sr130pc20_preview_size_1600x1200_regs);

	sensor->preview_size_2048x1232_regs	        =	sr130pc20_preview_size_2048x1232_regs;
	sensor->rows_num_preview_size_2048x1232_regs	 = ARRAY_SIZE(sr130pc20_preview_size_2048x1232_regs);

	sensor->preview_size_2048x1536_regs	         =	sr130pc20_preview_size_2048x1536_regs; 
	sensor->rows_num_preview_size_2048x1536_regs	  = ARRAY_SIZE(sr130pc20_preview_size_2048x1536_regs);

	sensor->preview_size_2560x1920_regs	          =	sr130pc20_preview_size_2560x1920_regs; 
	sensor->rows_num_preview_size_2560x1920_regs	  = ARRAY_SIZE(sr130pc20_preview_size_2560x1920_regs);
  
	
	/*Capture size */
	sensor->capture_size_640x480_regs	 		=	sr130pc20_capture_size_640x480_regs;
	sensor->rows_num_capture_size_640x480_regs	 = ARRAY_SIZE(sr130pc20_capture_size_640x480_regs);

	sensor->capture_size_720x480_regs  			=	sr130pc20_capture_size_720x480_regs; 
	sensor->rows_num_capture_size_720x480_regs	 = ARRAY_SIZE(sr130pc20_capture_size_720x480_regs);

	sensor->capture_size_800x480_regs	 		=	sr130pc20_capture_size_800x480_regs;
	sensor->rows_num_capture_size_800x480_regs	 = ARRAY_SIZE(sr130pc20_capture_size_800x480_regs);

	sensor->capture_size_800x486_regs	 		=	sr130pc20_capture_size_800x486_regs;
	sensor->rows_num_capture_size_800x486_regs	 = ARRAY_SIZE(sr130pc20_capture_size_800x486_regs);

	sensor->capture_size_800x600_regs  			=	sr130pc20_capture_size_800x600_regs; 
	sensor->rows_num_capture_size_800x600_regs	 = ARRAY_SIZE(sr130pc20_capture_size_800x600_regs);

    	sensor->capture_size_1024x600_regs	 		=	sr130pc20_capture_size_1024x600_regs;
	sensor->rows_num_capture_size_1024x600_regs	 = ARRAY_SIZE(sr130pc20_capture_size_1024x600_regs);

	sensor->capture_size_1024x768_regs  			=	sr130pc20_capture_size_1024x768_regs; 
	sensor->rows_num_capture_size_1024x768_regs	 = ARRAY_SIZE(sr130pc20_capture_size_1024x768_regs);

	sensor->capture_size_1280x960_regs  			=	sr130pc20_capture_size_1280x960_regs; 
	sensor->rows_num_capture_size_1280x960_regs	 = ARRAY_SIZE(sr130pc20_capture_size_1280x960_regs);

    	sensor->capture_size_1600x960_regs	 		=	sr130pc20_capture_size_1600x960_regs;
	sensor->rows_num_capture_size_1600x960_regs	 = ARRAY_SIZE(sr130pc20_capture_size_1600x960_regs);

	sensor->capture_size_1600x1200_regs  			=	sr130pc20_capture_size_1600x1200_regs; 
	sensor->rows_num_capture_size_1600x1200_regs	 = ARRAY_SIZE(sr130pc20_capture_size_1600x1200_regs);

	sensor->capture_size_2048x1232_regs  			=	sr130pc20_capture_size_2048x1232_regs; 
	sensor->rows_num_capture_size_2048x1232_regs	 = ARRAY_SIZE(sr130pc20_capture_size_2048x1232_regs);

	sensor->capture_size_2048x1536_regs  			=	sr130pc20_capture_size_2048x1536_regs; 
	sensor->rows_num_capture_size_2048x1536_regs	 = ARRAY_SIZE(sr130pc20_capture_size_2048x1536_regs);

	sensor->capture_size_2560x1536_regs  			=	sr130pc20_capture_size_2560x1536_regs; 
	sensor->rows_num_capture_size_2560x1536_regs	 = ARRAY_SIZE(sr130pc20_capture_size_2560x1536_regs);

	sensor->capture_size_2560x1920_regs  			=	sr130pc20_capture_size_2560x1920_regs; 
	sensor->rows_num_capture_size_2560x1920_regs	 = ARRAY_SIZE(sr130pc20_capture_size_2560x1920_regs);

	
	/*pattern*/
	sensor->pattern_on_regs 			  = sr130pc20_pattern_on_regs;
	sensor->rows_num_pattern_on_regs	  = ARRAY_SIZE(sr130pc20_pattern_on_regs);
	
	sensor->pattern_off_regs			  = sr130pc20_pattern_off_regs;
	sensor->rows_num_pattern_off_regs	  = ARRAY_SIZE(sr130pc20_pattern_off_regs);

	/*AE*/
	sensor->ae_lock_regs			  = sr130pc20_ae_lock_regs;
	sensor->rows_num_ae_lock_regs	  = ARRAY_SIZE(sr130pc20_ae_lock_regs);

		
	sensor->ae_unlock_regs			  = sr130pc20_ae_unlock_regs;
	sensor->rows_num_ae_unlock_regs	  = ARRAY_SIZE(sr130pc20_ae_unlock_regs);


	/*AWB*/

	sensor->awb_lock_regs			  = sr130pc20_awb_lock_regs;
	sensor->rows_num_awb_lock_regs	  = ARRAY_SIZE(sr130pc20_awb_lock_regs);

	sensor->awb_unlock_regs			  = sr130pc20_awb_unlock_regs;
	sensor->rows_num_awb_unlock_regs	  = ARRAY_SIZE(sr130pc20_awb_unlock_regs);

	//ISO//
	sensor->iso_auto_regs			  = sr130pc20_iso_auto_regs;
	sensor->rows_num_iso_auto_regs	  = ARRAY_SIZE(sr130pc20_iso_auto_regs);

	sensor->iso_50_regs			  = sr130pc20_iso_50_regs;
	sensor->rows_num_iso_50_regs	  = ARRAY_SIZE(sr130pc20_iso_50_regs);

	sensor->iso_100_regs			  = sr130pc20_iso_100_regs;
	sensor->rows_num_iso_100_regs	  = ARRAY_SIZE(sr130pc20_iso_100_regs);

	sensor->iso_200_regs			  = sr130pc20_iso_200_regs;
	sensor->rows_num_iso_200_regs	  = ARRAY_SIZE(sr130pc20_iso_200_regs);

	sensor->iso_400_regs			  = sr130pc20_iso_400_regs;
	sensor->rows_num_iso_400_regs	  = ARRAY_SIZE(sr130pc20_iso_400_regs);

	return 0;
};

int camdrv_ss_read_device_id_sr130pc20(
		struct i2c_client *client, char *device_id)
{
	int ret = -1;
	/* NEED to WRITE THE I2c REad code to read the deviceid */
	return 0;
}

static int __init camdrv_ss_sr130pc20_mod_init(void)
{
	struct camdrv_ss_sensor_reg sens;

	strncpy(sens.name, sr130pc20_NAME, sizeof(sr130pc20_NAME));
	sens.sensor_functions = camdrv_ss_sensor_functions_sr130pc20;
	sens.sensor_power = camdrv_ss_sr130pc20_sensor_power;
	sens.read_device_id = camdrv_ss_read_device_id_sr130pc20;
#ifdef CONFIG_SOC_CAMERA_MAIN_SR130
	sens.isMainSensor = 1;
#endif

#ifdef CONFIG_SOC_CAMERA_SUB_SR130
	sens.isMainSensor = 0;
#endif
	camdrv_ss_sensors_register(&sens);
}

module_init(camdrv_ss_sr130pc20_mod_init);

MODULE_DESCRIPTION("SAMSUNG CAMERA SENSOR SR130 ");
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");

