
/***********************************************************************
* Driver for sr200pc20m (5MP Camera) from SAMSUNG SYSTEM LSI
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
#include "camdrv_ss.h"
#include "camdrv_ss_sr200pc20m.h"           
#include <linux/module.h>


#define SR200PC20M_NAME	"sr200pc20m"
#define SENSOR_ID 2
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x4b0000  // 4915200  = 2560*1920     //0x33F000     //3403776 //2216 * 1536
#define SENSOR_PREVIEW_WIDTH      640 // 1024
#define SENSOR_PREVIEW_HEIGHT     480 // 768
#define AF_OUTER_WINDOW_WIDTH   320
#define AF_OUTER_WINDOW_HEIGHT  266
#define AF_INNER_WINDOW_WIDTH   143
#define AF_INNER_WINDOW_HEIGHT  143
#define MAX_BUFFER			(3072)

#define SR200PC20M_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define SR200PC20M_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8 
#define FORMAT_FLAGS_COMPRESSED 0x3
#define DEFUALT_MCLK		26000000

   
#define SR200PC20M_REGISTER_SIZE 2

#define SR200PC20M_DELAY_DURATION 0xFF

#if defined(CONFIG_MACH_RHEA_SS_CORIPLUS)

#define EXIF_SOFTWARE		""
#define EXIF_MAKE		"Samsung"
#define EXIF_MODEL		"GT-S5301"
#else
#define EXIF_SOFTWARE		""
#define EXIF_MAKE		"Samsung"
#define EXIF_MODEL		"GT-B5330"
#endif

static DEFINE_MUTEX(af_cancel_op);
extern inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd);


extern  int camdrv_ss_i2c_set_config_register(struct i2c_client *client, 
                                         regs_t reg_buffer[], 
          				                 int num_of_regs, 
          				                 char *name);
extern int camdrv_ss_set_preview_size(struct v4l2_subdev *sd);
extern int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff);
static int init=true;
//#define __JPEG_CAPTURE__ 1        //denis_temp ; yuv capture

extern int camera_antibanding_get(); //add anti-banding code

static const struct camdrv_ss_framesize sr200pc20m_supported_preview_framesize_list[] = {
	//{ PREVIEW_SIZE_QCIF,	176,  144 },
	//{ PREVIEW_SIZE_QVGA,	320,  240 },
	//{ PREVIEW_SIZE_CIF,	352,  288 },
	{ PREVIEW_SIZE_VGA,	640,  480 },
	//{ PREVIEW_SIZE_D1,	720,  480 },
	//{ PREVIEW_SIZE_1MP,     1280, 1024 },
	//{ PREVIEW_SIZE_W1MP, 1600,960},
	{ PREVIEW_SIZE_2MP,	1600,  1200 },
};


static const struct camdrv_ss_framesize  sr200pc20m_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA,		640,  480 },
	//{ CAPTURE_SIZE_1MP,		1280, 960 },
	{ CAPTURE_SIZE_2MP,		1600, 1200 },

};
const static struct v4l2_fmtdesc sr200pc20m_fmts[] = 
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

static const struct v4l2_queryctrl sr200pc20m_controls[] = {
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
		.name		= "White Balance",
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
/*
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
*/
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
		.maximum	= (1 << SCENE_MODE_NONE |
						1 << SCENE_MODE_NIGHTSHOT | 1 << SCENE_MODE_LANDSCAPE
						 | 1 << SCENE_MODE_PARTY_INDOOR |
						 1 << SCENE_MODE_SUNSET |
						 1 << SCENE_MODE_CANDLE_LIGHT | /*querymenu?*/
						1 << SCENE_MODE_BACK_LIGHT | 1<< SCENE_MODE_DUSK_DAWN |
						1 << SCENE_MODE_FALL_COLOR),
		.step		= 1,
		.default_value	= SCENE_MODE_NONE,
	},

	{
		.id			= V4L2_CID_CAMERA_SET_AUTO_FOCUS,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Set AutoFocus",
		.minimum	= AUTO_FOCUS_OFF,
		.maximum	= AUTO_FOCUS_OFF, //aska modified
		.step		= 1,
		.default_value	= AUTO_FOCUS_OFF,
	},

	{
		.id 		= V4L2_CID_CAMERA_TOUCH_AF_AREA,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Touchfocus areas",
		.minimum	= 0,
		.maximum	= 0, //aska
		.step		= 1,
		.default_value	= 0, //aska
	},
	{
		.id			= V4L2_CID_CAMERA_FRAME_RATE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Framerate control",
		.minimum	= FRAME_RATE_15,
		.maximum	= (1 << FRAME_RATE_15),
		.step		= 1,
		.default_value	= FRAME_RATE_15,
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


static int camdrv_ss_sr200pc20m_enum_frameintervals(struct v4l2_subdev *sd,struct v4l2_frmivalenum *fival)
{
	int err = 0;
	int size,i;

	if (fival->index >= 1)
		return -EINVAL;
	
	CAM_INFO_PRINTK(" %s :  check w = %d h = %d \n",__func__,fival->width,fival->height);
	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	for(i = 0; i < ARRAY_SIZE(sr200pc20m_supported_preview_framesize_list); i++) {
		if((sr200pc20m_supported_preview_framesize_list[i].width == fival->width) &&
		    (sr200pc20m_supported_preview_framesize_list[i].height == fival->height))
		{
			size = sr200pc20m_supported_preview_framesize_list[i].index;
			break;
		}
	}

	if(i == ARRAY_SIZE(sr200pc20m_supported_preview_framesize_list))
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

static long camdrv_ss_sr200pc20m_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
       p->hor_angle.numerator = 512;
       p->hor_angle.denominator = 10;
       p->ver_angle.numerator = 394;
       p->ver_angle.denominator = 10;
       p->focus_distance[0] = 10;
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

int camdrv_ss_sr200pc20m_set_preview_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( "%s :\n", __func__);

	if (!state->pix.width || !state->pix.height) {
		CAM_ERROR_PRINTK( "%s : width or height is NULL!!!\n",__func__);
		return -EINVAL;
	}

	if (state->mode_switch == PICTURE_CAPTURE_TO_CAMERA_PREVIEW_RETURN) {
		err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_preview_camera_regs, ARRAY_SIZE(sr200pc20m_preview_camera_regs), "preview_camera_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :sr200pc20m_preview_camera_regs IS FAILED\n",__func__);
			return -EIO;
		}
	}
	if (state->mode_switch == INIT_DONE_TO_CAMERA_PREVIEW) {
#if 0
		err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_preview_camera_regs, ARRAY_SIZE(sr200pc20m_preview_camera_regs), "preview_camera_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :sr200pc20m_preview_camera_regs IS FAILED\n",__func__);
			return -EIO;
		}
#endif		
	}
	if(state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW)
	{
		err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_fps_15_regs, ARRAY_SIZE(sr200pc20m_fps_15_regs), "fps_15_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s : sr200pc20m_fps_15_regs is FAILED !!\n", __func__);
			return -EIO;
		}
	}
	else if(state->mode_switch == INIT_DONE_TO_CAMCORDER_PREVIEW)
	{
		/*err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_init_regs, ARRAY_SIZE(sr200pc20m_init_regs), "init_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :sr200pc20m_init_regs IS FAILED\n",__func__);
			return -EIO;
		}*/
		err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_fps_15_regs, ARRAY_SIZE(sr200pc20m_fps_15_regs), "fps_15_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s : sr200pc20m_fps_15_regs is FAILED !!\n", __func__);
			return -EIO;
		}
	}
	else if(state->mode_switch == CAMCORDER_PREVIEW_TO_CAMERA_PREVIEW)
	{
		err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_init_regs, ARRAY_SIZE(sr200pc20m_init_regs), "init_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :sr200pc20m_init_regs IS FAILED\n",__func__);
			return -EIO;
		}
	}



	err = camdrv_ss_set_preview_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK( "%s : camdrv_ss_set_preview_size is FAILED !!\n", __func__);
		return -EIO;
	}

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
static int camdrv_ss_sr200pc20m_set_white_balance(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( " %s :  value =%d\n", __func__, mode);

	switch (mode) {
	case WHITE_BALANCE_AUTO:
	{
		if (sr200pc20m_wb_auto_regs == 0)
			CAM_ERROR_PRINTK( " %s : wb_auto_regs not supported !!!\n", __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_wb_auto_regs, ARRAY_SIZE(sr200pc20m_wb_auto_regs), "wb_auto_regs");

		break;
	}

	
	case WHITE_BALANCE_CLOUDY:
	{
		if (sr200pc20m_wb_cloudy_regs == 0)
			CAM_ERROR_PRINTK( "%s : wb_cloudy_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_wb_cloudy_regs, ARRAY_SIZE(sr200pc20m_wb_cloudy_regs), "wb_cloudy_regs");

		break;
	}


	case WHITE_BALANCE_FLUORESCENT:
	{
		if (sr200pc20m_wb_fluorescent_regs== 0)
			CAM_ERROR_PRINTK( " %s : wb_fluorescent_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,  sr200pc20m_wb_fluorescent_regs, ARRAY_SIZE(sr200pc20m_wb_fluorescent_regs), "wb_fluorescent_regs");

		break;
	}

	
	case WHITE_BALANCE_DAYLIGHT:
	{
		if (sr200pc20m_wb_daylight_regs == 0)
			CAM_ERROR_PRINTK( " %s : wb_daylight_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_wb_daylight_regs, ARRAY_SIZE(sr200pc20m_wb_daylight_regs), "wb_daylight_regs");

		break;
	}
	case WHITE_BALANCE_INCANDESCENT:
	{
		if (sr200pc20m_wb_incandescent_regs == 0)
			CAM_ERROR_PRINTK( "%s : wb_incandescent_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_wb_incandescent_regs, ARRAY_SIZE(sr200pc20m_wb_incandescent_regs), "wb_incandescent_regs");

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
static int camdrv_ss_sr200pc20m_set_iso(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_PRINTK( " %s :  value =%d\n",  __func__, mode);

	switch (mode) {
	case ISO_AUTO:
	{
		if (sr200pc20m_iso_auto_regs == 0)
			CAM_ERROR_PRINTK( " %s : iso_auto_regs not supported !!!\n", __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_iso_auto_regs, ARRAY_SIZE(sr200pc20m_iso_auto_regs), "iso_auto_regs");

		break;
	}
	case ISO_50:
	{
		if (sr200pc20m_iso_50_regs== 0)
			CAM_ERROR_PRINTK( " %s : iso_50_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_iso_50_regs, ARRAY_SIZE(sr200pc20m_iso_50_regs), "iso_50_regs");
	
		break;
	}

	case ISO_100:
	{
		if (sr200pc20m_iso_100_regs == 0)
			CAM_ERROR_PRINTK( " %s : iso_100_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_iso_100_regs, ARRAY_SIZE(sr200pc20m_iso_100_regs), "iso_100_regs");
	
		break;
	}
	case ISO_200:
	{
		if (sr200pc20m_iso_200_regs == 0)
			CAM_ERROR_PRINTK( "%s  : iso_200_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_iso_200_regs, ARRAY_SIZE(sr200pc20m_iso_200_regs), "iso_200_regs");
	
		break;
	}
	case ISO_400:
	{
		if (sr200pc20m_iso_400_regs == 0)
			CAM_ERROR_PRINTK( "%s  : iso_400_regs not supported !!!\n",  __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sr200pc20m_iso_400_regs,ARRAY_SIZE(sr200pc20m_iso_400_regs), "iso_400_regs");
	
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


#define AAT_PULS_HI_TIME    1
#define AAT_PULS_LO_TIME    1
#define AAT_LATCH_TIME      500

// AAT1271 flash control driver.
#if 0
static void camdrv_ss_AAT_flash_write_data(unsigned char count)
{

    unsigned long flags;

    local_irq_save(flags);

    if(HWREV >= 0x03)
    {
        if(count)
        {
            do 
            {
                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 0);
                udelay(AAT_PULS_LO_TIME);

                gpio_set_value(GPIO_CAM_FLASH_SET_NEW, 1);
                udelay(AAT_PULS_HI_TIME);
            } while (--count);

            udelay(AAT_LATCH_TIME);
        }
    }
    else
    {
        if(count)
        {
            do 
            {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
                udelay(AAT_PULS_LO_TIME);

                gpio_set_value(GPIO_CAM_FLASH_SET, 1);
                udelay(AAT_PULS_HI_TIME);
            } while (--count);

            udelay(AAT_LATCH_TIME);
        }
    }
    
    local_irq_restore(flags);


}

#endif

static float camdrv_ss_sr200pc20m_get_exposureTime(struct v4l2_subdev *sd)
{

   struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned int read_value1=0,read_value2=0,read_value3=0;
    int exposureTime = 0;


	//AE off
	camdrv_ss_i2c_write_2_bytes(client, 0x03, 0x20);
	camdrv_ss_i2c_write_2_bytes(client, 0x10, 0x0c);

	camdrv_ss_i2c_read_1_byte(client, 0x80, &read_value1);
	camdrv_ss_i2c_read_1_byte(client, 0x81, &read_value2);
	camdrv_ss_i2c_read_1_byte(client, 0x82, &read_value3);

	exposureTime = (read_value1 << 19 | read_value2 << 11 | read_value3<<3);
	CAM_INFO_PRINTK("%s, exposureTime =%d \n",__func__,exposureTime);
	return ((float)exposureTime);
 
}
//aska add
static int camdrv_ss_sr200pc20m_get_nightmode(struct v4l2_subdev *sd)
{

    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0, read_value2 = 0,read_value3=0;
    int Exptime = 0, Expmax=0;

		CAM_ERROR_PRINTK("camdrv_ss_sr200pc20m_get_nightmode\n");		
#if 1
    camdrv_ss_i2c_write_2_bytes(client, 0x03, 0x20);
		camdrv_ss_i2c_write_2_bytes(client, 0x10, 0x0c);

		camdrv_ss_i2c_read_1_byte(client, 0x80, &read_value1);
		camdrv_ss_i2c_read_1_byte(client, 0x81, &read_value2);
		camdrv_ss_i2c_read_1_byte(client, 0x82, &read_value3);

		Exptime = (read_value1 << 16) | (read_value2 << 8)|(read_value3);
    CAM_ERROR_PRINTK("value1=%x,value2=%x,value3=%x,Exptime=%d\n",read_value1,read_value2,read_value3,Exptime);
    
    camdrv_ss_i2c_read_1_byte(client, 0x88, &read_value1);
		camdrv_ss_i2c_read_1_byte(client, 0x89, &read_value2);
		camdrv_ss_i2c_read_1_byte(client, 0x8A, &read_value3);
		Expmax = (read_value1 << 16) | (read_value2 << 8)|(read_value3);
		CAM_ERROR_PRINTK("value1=%x,value2=%x,value3=%x,Expmax=%d\n",read_value1,read_value2,read_value3,Expmax);
	   //return ((Exptime * 1000) / 400); // us

	if(Exptime <Expmax){
		return 0;	//noraml
	}else{
		return 1;  //dark
	}
#endif
  return 0;
}

static int camdrv_ss_sr200pc20m_get_iso_speed_rate(struct v4l2_subdev *sd)
{
#if 1
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short read_value = 0;
    int GainValue = 0;
    int isospeedrating = 100;
	int rows_num_=0;

/*    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2A18);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value);*/
    camdrv_ss_i2c_write_2_bytes(client, 0x03, 0x20);
    camdrv_ss_i2c_read_1_byte(client, 0xb0, &read_value);

    GainValue = ((read_value / 32) + 0.5);

    if(GainValue < 1.14)
    {
        isospeedrating = 50;
    }
    else if(GainValue < 2.14)
    {
        isospeedrating = 100;
    }
    else if(GainValue < 2.64)
    {
        isospeedrating = 200;
    }
        else if(GainValue < 7.52)
    {
        isospeedrating = 400;
    }
    else
    {
        isospeedrating = 800;
    }

    CAM_ERROR_PRINTK("camdrv_ss_sr200pc20m_get_iso_speed_rate, GainValue =%d, isospeedrating =%d\n", GainValue, isospeedrating );		
    
	   return isospeedrating;
#endif
	/* no implementation yet */
//	return -1;
 
}

static int camdrv_ss_sr200pc20m_get_ae_stable_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
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
        CAM_ERROR_PRINTK( "[%s: %d] ERROR! AE stable check\n", __FILE__, __LINE__);
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


static int camdrv_ss_sr200pc20m_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl, struct camdrv_ss_sensor_cap  *sensor)
{ 
#if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short AF_status = 0;
    int err = 0;
    unsigned short usReadData =0 ;
    unsigned short uiLoop = 0;

    msleep(200);

    CAM_INFO_PRINTK("camdrv_ss_sr200pc20m_get_auto_focus_status E \n");
    do
    {
        err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_get_1st_af_search_status,ARRAY_SIZE(sr200pc20m_get_1st_af_search_status),"get_1st_af_search_status");
        err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12,&usReadData);
        if(err < 0)
        {
            CAM_ERROR_PRINTK( "%s: I2C failed during AF \n", __func__);
            ctrl->value = CAMDRV_SS_AF_FAILED;
            goto CAMDRV_SS_AF_END;
        }
        
        CAM_INFO_PRINTK(  " 1st check status, usReadData : 0x%x\n", usReadData );

        switch( usReadData & 0xFF )
    	{
            case 1:
                CAM_INFO_PRINTK( "1st CAM_AF_PROGRESS\n " );
                AF_status = CAMDRV_SS_AF_SEARCHING;
            break;

            case 2:
                CAM_INFO_PRINTK( "1st CAM_AF_SUCCESS\n " );
                AF_status = CAMDRV_SS_AF_FOCUSED;
            break;

            default:                    
                CAM_INFO_PRINTK( "1st CAM_AF_FAIL. count : %d\n ", uiLoop);
                AF_status = CAMDRV_SS_AF_FAILED;
                goto CAMDRV_SS_AF_END;
            break;
    	}

        msleep(100);
        uiLoop++;
        
    }while ((AF_status == CAMDRV_SS_AF_SEARCHING)&& (uiLoop < 100));

    if (uiLoop >= 100)
        	{
        AF_status = CAMDRV_SS_AF_FAILED;
        CAM_ERROR_PRINTK( "%s, AF failed, over counted : %d\n", __func__, uiLoop);
        goto CAMDRV_SS_AF_END;
        	}

    CAM_INFO_PRINTK( "%s, uiLoop : %d, Start 2nd AF\n", __func__, uiLoop);


    uiLoop =0;
    AF_status = CAMDRV_SS_AF_SEARCHING;
    do
    {
        msleep(100);

        err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_get_2nd_af_search_status,ARRAY_SIZE(sr200pc20m_get_2nd_af_search_status),"get_2nd_af_search_status");
        err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &usReadData);
        if(err < 0)
        {
            CAM_ERROR_PRINTK( "%s: I2C failed during AF \n", __func__);
            ctrl->value = CAMDRV_SS_AF_FAILED;
            goto CAMDRV_SS_AF_END;
        }

        CAM_INFO_PRINTK(  " 2nd check status, usReadData : 0x%x\n", usReadData );

        if (!(usReadData & 0xff00))
        {
            CAM_INFO_PRINTK( "2nd CAM_AF_SUCCESS, cnt : %d\n ", uiLoop);
            AF_status = CAMDRV_SS_AF_FOCUSED;
            break;
        }

        uiLoop++;
    }while ((AF_status == CAMDRV_SS_AF_SEARCHING)&& (uiLoop < 100));

    if (uiLoop >= 100)
    	{
        AF_status = CAMDRV_SS_AF_FAILED;
        CAM_ERROR_PRINTK( "%s, AF failed, over counted : %d\n", __func__, uiLoop);
        goto CAMDRV_SS_AF_END;
    }


    CAMDRV_SS_AF_END: 
    
        CAM_INFO_PRINTK( "AF End : %d\n", AF_status);
        ctrl->value = AF_status;
        return AF_status;
#endif
return 0;
}


static int camdrv_ss_sr200pc20m_get_touch_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
      CAM_INFO_PRINTK("camdrv_ss_sr200pc20m_get_touch_focus_status E \n");
 	return 0;
}


static int camdrv_ss_sr200pc20m_set_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
#if 0
  struct i2c_client *client = v4l2_get_subdevdata(sd);
	  struct camdrv_ss_state *state = to_state(sd);
    int err = 0;
	
    mutex_lock(&af_cancel_op);
	CAM_INFO_PRINTK("camdrv_ss_sr200pc20m_set_auto_focus E \n");

    // Initialize fine search value.
    state->bStartFineSearch = false;
	
    if(ctrl->value == AUTO_FOCUS_ON) 
    {
	    CAM_INFO_PRINTK( "%s, AUTFOCUS ON\n", __func__ );

		if(sr200pc20m_single_af_start_regs == 0)
			CAM_ERROR_PRINTK( "%s : single_af_start_regs  supported !!! \n", __func__);
		else
                {      
	
                        CAM_INFO_PRINTK( "%s , ae lock\n ",__func__);

                        err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_ae_lock_regs, ARRAY_SIZE(sr200pc20m_ae_lock_regs), "ae_lock_regs");

                        CAM_INFO_PRINTK( "%s , AF start\n ",__func__);

			err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_single_af_start_regs, ARRAY_SIZE(sr200pc20m_single_af_start_regs),"single_af_start_regs");
                }

		 if(err < 0)
        {
    		CAM_ERROR_PRINTK( "%s : i2c failed !!! \n", __func__);
    		mutex_unlock(&af_cancel_op);
            return -EIO;
        }

    }
    else if(ctrl->value == AUTO_FOCUS_OFF) 
    {

		CAM_INFO_PRINTK( "%s :  AUTFOCUS OFF  \n", __func__);

        state->camera_flash_fire = 0;
        state->camera_af_flash_checked = 0;
		if(sr200pc20m_single_af_stop_regs == 0)
			CAM_ERROR_PRINTK( "%s : single_af_stop_regs  supported !!! \n", __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,sr200pc20m_single_af_stop_regs,ARRAY_SIZE(sr200pc20m_single_af_stop_regs),"single_af_stop_regs");
		 if(err < 0)
        {
    		CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
    		mutex_unlock(&af_cancel_op);
    		return -EIO;
        }

		
        }
        
    mutex_unlock(&af_cancel_op);
#endif



    return 0;
}
        
        
static int camdrv_ss_sr200pc20m_set_touch_focus(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area)
        {

#if 0
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int err;
	unsigned short FirstWinStartX, FirstWinStartY, SecondWinStartX, SecondWinStartY;
       unsigned short FirstWinEndX, FirstWinEndY, ConvFirstWinStartX, ConvFirstWinStartY;
       unsigned short SecondWinEndX, SecondWinEndY, ConvSecondWinStartX, ConvSecondWinStartY;
	unsigned short DefaultFirstWinStartX, DefaultFirstWinStartY, DefaultSecondWinStartX, DefaultSecondWinStartY;
	int preview_width, preview_height;
	int count, read_value;

	preview_width = state->pix.width;
	preview_height = state->pix.height;

	DefaultFirstWinStartX = preview_width - ((preview_width - AF_OUTER_WINDOW_WIDTH) / 2); /* 640x480 => 160, 800x480 => 240 */
	DefaultFirstWinStartY = preview_height - ((preview_height - AF_OUTER_WINDOW_HEIGHT) / 2); /* 640x480 => 107, 800x480 => 107 */
	DefaultSecondWinStartX = preview_width - ((preview_width - AF_INNER_WINDOW_WIDTH) / 2); /* 640x480 => 248, 800x480 => 160 */
	DefaultSecondWinStartY = preview_height - ((preview_height - AF_INNER_WINDOW_HEIGHT) / 2); /* 640x480 => 328, 800x480 => 160 */

	CAM_INFO_PRINTK("[%s:%d] preview_width %d, preview_height %d\n",__func__, __LINE__, preview_width, preview_height);
	CAM_INFO_PRINTK("[%s:%d] DefaultFirstWinStartX %d, DefaultFirstWinStartY %d\n",__func__, __LINE__, DefaultFirstWinStartX, DefaultFirstWinStartY);
	CAM_INFO_PRINTK("[%s:%d] DefaultSecondWinStartX %d, DefaultSecondWinStartY %d\n",__func__, __LINE__,DefaultSecondWinStartX ,DefaultSecondWinStartY );
            

	/* value 0 is Touch AF Stop, 1 is Touch AF start */
	if(touch_af == TOUCH_AF_STOP)	{ /* AF stop */
		camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
		camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0294);
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultFirstWinStartX << 10) / preview_width)); /* FstWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultFirstWinStartY << 10) / preview_height)); /* FstWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (320 << 10) / preview_width  ); /* FstWinSizeX : 320 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (266 << 10) / preview_height );  /* FstWinSizeY : 266 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultSecondWinStartX << 10) / preview_width)); /* ScndWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultSecondWinStartY << 10) / preview_height)); /* ScndWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (143 << 10) / preview_width  ); /* ScndWinSizeX : 143 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (143 << 10) / preview_height ); /* ScndWinSizeY : 143  */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* WinSizesUpdated */		
        }
	else if(touch_af == TOUCH_AF_START) { /* AF start */
		// Prevent divided-by-zero.
		if(preview_width == 0 || preview_height == 0) {
			CAM_ERROR_PRINTK("%s: Either preview_width or preview_height is zero\n", __func__);
			return -EIO;
        }
            
		FirstWinStartX = SecondWinStartX = touch_area->leftTopX;
		FirstWinStartY = SecondWinStartY = touch_area->leftTopY;
		CAM_INFO_PRINTK("[%s:%d]touch X %d, touch Y %d\n",__func__, __LINE__,  FirstWinStartX, FirstWinStartY);

		// AF Position(Round Down)
		if(FirstWinStartX > AF_OUTER_WINDOW_WIDTH/2) {
			FirstWinStartX -= AF_OUTER_WINDOW_WIDTH/2;

			if(FirstWinStartX + AF_OUTER_WINDOW_WIDTH > preview_width) {
				CAM_ERROR_PRINTK( "%s: X Position Overflow : [%d, %d] \n", __func__, FirstWinStartX, AF_OUTER_WINDOW_WIDTH);

				FirstWinStartX = preview_width - AF_OUTER_WINDOW_WIDTH - 1;
        }
        }
        else
			FirstWinStartX = 0;


		if(FirstWinStartY > AF_OUTER_WINDOW_HEIGHT/2)	{
			FirstWinStartY -= AF_OUTER_WINDOW_HEIGHT/2;
            
			if(FirstWinStartY + AF_OUTER_WINDOW_HEIGHT > preview_height) {
				CAM_ERROR_PRINTK( "%s: Y Position Overflow : [%d, %d] \n", __func__, FirstWinStartY, AF_OUTER_WINDOW_HEIGHT);
            
				FirstWinStartY = preview_height - AF_OUTER_WINDOW_HEIGHT - 1;
        }
        }
        else
			FirstWinStartY = 0;
                CAM_INFO_PRINTK("[%s:%d] FirstWinStartX %d, FirstWinStartY %d\n",__func__, __LINE__, FirstWinStartX, FirstWinStartY);

		// AF Position(Round Down)
		if(SecondWinStartX > AF_INNER_WINDOW_WIDTH/2) {
			SecondWinStartX -= AF_INNER_WINDOW_WIDTH/2;

			if(SecondWinStartX + AF_INNER_WINDOW_WIDTH > preview_width) {
				CAM_ERROR_PRINTK( "%s: X Position Overflow : [%d, %d] \n", __func__, SecondWinStartX, AF_INNER_WINDOW_WIDTH);

				SecondWinStartX = preview_width - AF_INNER_WINDOW_WIDTH - 1;
			}
		}
		else
			SecondWinStartX = 0;

		if(SecondWinStartY > AF_INNER_WINDOW_HEIGHT/2)	{
			SecondWinStartY -= AF_INNER_WINDOW_HEIGHT/2;

			if(SecondWinStartY + AF_INNER_WINDOW_HEIGHT > preview_height) {
				CAM_ERROR_PRINTK( "%s: Y Position Overflow : [%d, %d] \n", __func__, SecondWinStartY, AF_INNER_WINDOW_HEIGHT);
        
				SecondWinStartY = preview_height - AF_INNER_WINDOW_HEIGHT - 1;
			}
		}
		else
			SecondWinStartY = 0;
                CAM_INFO_PRINTK("[%s:%d] SecondWinStartX %d, SecondWinStartY %d\n",__func__, __LINE__, SecondWinStartX, SecondWinStartY);

                // if use mirror/flip, need this code.
                FirstWinEndX = FirstWinStartX + AF_OUTER_WINDOW_WIDTH;
                FirstWinEndY = FirstWinStartY + AF_OUTER_WINDOW_HEIGHT;

                if(preview_width - FirstWinEndX <= 0)
                    ConvFirstWinStartX = 0;
                else
                    ConvFirstWinStartX = preview_width - FirstWinEndX -1;

                if(preview_height - FirstWinEndY <= 0)
                    ConvFirstWinStartY = 0;
            else
                    ConvFirstWinStartY = preview_height - FirstWinEndY -1;
                CAM_INFO_PRINTK("[%s:%d] Conv::FirstWinStartX %d, FirstWinStartY %d\n",__func__, __LINE__, ConvFirstWinStartX, ConvFirstWinStartY);
        
                SecondWinEndX = SecondWinStartX + AF_INNER_WINDOW_WIDTH;
                SecondWinEndY = SecondWinStartY + AF_INNER_WINDOW_HEIGHT;

                if(preview_width - SecondWinEndX <= 0)
                    ConvSecondWinStartX = 0;
    else
                    ConvSecondWinStartX = preview_width - SecondWinEndX -1;

                if(preview_height - SecondWinEndY <= 0)
                    ConvSecondWinStartY = 0;
                else
                    ConvSecondWinStartY = preview_height - SecondWinEndY -1;
                CAM_INFO_PRINTK("[%s:%d] Conv::ConvSecondWinStartX %d, ConvSecondWinStartY %d\n",__func__, __LINE__, ConvSecondWinStartX, ConvSecondWinStartY);

		ConvFirstWinStartX = (unsigned short)((ConvFirstWinStartX << 10) / preview_width);
		ConvFirstWinStartY = (unsigned short)((ConvFirstWinStartY << 10) / preview_height);
        
		//SecondWinStartX = ConvFirstWinStartX + 140;
		//SecondWinStartY = ConvFirstWinStartY + 131;                 
		ConvSecondWinStartX = (unsigned short)((ConvSecondWinStartX << 10) / preview_width);
		ConvSecondWinStartY = (unsigned short)((ConvSecondWinStartY << 10) / preview_height);

		camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
		camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0294);
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvFirstWinStartX); /* FstWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvFirstWinStartY); /* FstWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (320 << 10) / preview_width  ); /* FstWinSizeX : 320 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (266 << 10) / preview_height );  /* FstWinSizeY : 266 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvSecondWinStartX); /* ScndWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvSecondWinStartY); /* ScndWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (72 << 10) / preview_width  ); /* ScndWinSizeX : 72 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (143 << 10) / preview_height ); /* ScndWinSizeY : 143  */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* WinSizesUpdated */
 
//		state->touch_ae_af_state = 1;

		/* Are we need "Touch AE Weight"? */
		/* FIXME */
            }
            

#if 0 //touch_AE
    else if(value == 2){ /* stop touch AE */
#if 0
		camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
		camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x395C);
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0000); /* FDA_bUseFaceAlg, Touched AE&AF support on/off */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0000); /* FDA_bUseConfigChange, Change config */

		state->touch_ae_af_state = 0;
#endif        
	} else if(value == 3){ /* start touch AE */

		unsigned int aeX = 0, aeY = 0;

		aeX = state->position.x;
		aeY = state->position.y; 

		// AE Position(Round Down)
		if(aeX > AE_WINDOW_WIDTH/2) {
			aeX -= AE_WINDOW_WIDTH/2;

			if(aeX + AE_WINDOW_WIDTH > preview_width) {
				CAM_ERROR_PRINTK( "%s:[Touch AE] X Position Overflow : [%d, %d] \n", __func__, aeX, AE_WINDOW_WIDTH);
				aeX = preview_width - AE_WINDOW_WIDTH - 1;
			}
            }
		else
			aeX = 0;

		if(aeY > AE_WINDOW_HEIGHT/2) {
			aeY -= AE_WINDOW_HEIGHT/2;
		
			if(aeY + AE_WINDOW_HEIGHT > preview_height) {
				CAM_ERROR_PRINTK( "%s:[Touch AE] Y Position Overflow : [%d, %d] \n", __func__, aeY, AE_WINDOW_HEIGHT);
				aeY = preview_width - AE_WINDOW_HEIGHT - 1;
            }			
    	}
		else
			aeY = 0;
#if 0        
		if(state->touch_ae_af_state == 0) /* Default setting */
		{
			camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
			camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
			camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x395C);
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* FDA_bUseFaceAlg, Touched AE&AF support on/off */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* FDA_bUseConfigChange, Change config */
			camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x3962); /* FDA_FaceArr */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0000); /* FDA_FaceArr_0_X_Start, region start x position */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0000); /* FDA_FaceArr_0_Y_Start, region start y position */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, preview_width ); /* FDA_FaceArr_0_X_End, region end x position */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, preview_height ); /* FDA_FaceArr_0_Y_End, region end y position */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x003D); /* FDA_FaceArr_0_ABR, region target brightness */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0000); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__UpdateState, region change update */
			camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__bUpdate, use or not use*/
	}
#endif
//		CAM_INFO_PRINTK("[%s:%d] state->touch_ae_af_state %d\n", __func__, __LINE__, state->touch_ae_af_state);
		camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
		camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x3962); /* FDA_FaceArr */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, aeX ); /* FDA_FaceArr_0_X_Start, region start x position */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, aeY ); /* FDA_FaceArr_0_Y_Start, region start y position */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, aeX + AE_WINDOW_WIDTH ); /* FDA_FaceArr_0_X_End, region end x position */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, aeY + AE_WINDOW_HEIGHT ); /* FDA_FaceArr_0_Y_End, region end y position */
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x396C); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0100); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__UpdateState, region change update */
	}

#endif //Touch AE
#endif


	return 0;
}


#if 0

static int camdrv_ss_sr200pc20m_get_light_condition(struct v4l2_subdev *sd, int *Result)
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


static bool camdrv_ss_sr200pc20m_check_flash_needed(struct v4l2_subdev *sd)
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


static int camdrv_ss_sr200pc20m_AAT_flash_control(struct v4l2_subdev *sd, int control_mode)
{
#if 0
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
#endif // CONFIG_FLASH_ENABLE


    return 0;
}



//@HW
//Power (common)
static struct regulator *VCAM_C_1_8_V;  //LDO_HV9
static struct regulator *VCAM_A_2_8_V;   //LDO_CAM 12/12/2011


#ifdef CONFIG_SOC_CAMERA_POWER_USE_ASR   //VE Group
static struct regulator *VCAM_CORE_1_8_V;   //ASR_SW
#else
#define CAM_IO_EN	   37
#endif


//main cam 
#if defined(CONFIG_MACH_RHEA_SS_CORIPLUS)
#define CAM_RST    92
#elif defined(CONFIG_MACH_RHEA_SS_CORSICA)
#define CAM_RST    33
#else
#define CAM_RST    13
#endif
#define CAM_EN    111

#define SENSOR_0_CLK			"dig_ch0_clk"    //(common)
#define SENSOR_0_CLK_FREQ		(26000000) //@HW, need to check how fast this meaning.

static int camdrv_ss_sr200pc20m_copy_files_for_60hz(void)
{

#define COPY_FROM_60HZ_TABLE(TABLE_NAME, ANTI_BANDING_SETTING) \
	memcpy (TABLE_NAME, TABLE_NAME##_##ANTI_BANDING_SETTING, \
	sizeof(TABLE_NAME))
	
	CAM_INFO_PRINTK("%s: Enter \n",__func__);

	//[ltn_to_do] should be rearranged !!!!! just for Testing
	COPY_FROM_60HZ_TABLE (sr200pc20m_init_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_preview_camera_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_none_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_backlight_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_landscape_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_party_indoor_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_sunset_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_duskdawn_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_fall_color_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_nightshot_Normal, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_nightshot_Dark, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_scene_candle_light_regs, 60hz);
	COPY_FROM_60HZ_TABLE (sr200pc20m_fps_15_regs, 60hz);

	CAM_INFO_PRINTK("%s: copy done!\n", __func__);
  return 0;
}
static int camdrv_ss_sr200pc20m_check_table_size_for_60hz(void)
{
#define IS_SAME_NUM_OF_ROWS(TABLE_NAME) \
	(sizeof(TABLE_NAME) == sizeof(TABLE_NAME##_60hz))

	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_init_regs) ) return (-1);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_preview_camera_regs) ) return (-2);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_none_regs) ) return (-3);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_backlight_regs) ) return (-4);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_landscape_regs) ) return (-5);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_party_indoor_regs) ) return (-6);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_sunset_regs) ) return (-7);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_duskdawn_regs) ) return (-8);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_fall_color_regs) ) return (-9);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_nightshot_Normal) ) return (-10);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_nightshot_Dark) ) return (-11);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_scene_candle_light_regs) ) return (-12);
	if ( !IS_SAME_NUM_OF_ROWS(sr200pc20m_fps_15_regs) ) return (-13);

	CAM_INFO_PRINTK("%s: Success !\n", __func__);
	return 0;
}	

static int camdrv_ss_sr200pc20m_sensor_power(int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;
	CAM_INFO_PRINTK("%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret =
		    pi_mgr_dfs_add_request(&unicam_dfs_node,"unicam", PI_MGR_PI_ID_MM,
                                           PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			CAM_ERROR_PRINTK("%s: failed to register PI DFS request\n",
			       __func__);
			return -1;
		}
	}

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (!clock) {
		CAM_ERROR_PRINTK("%s: unable to get clock %s\n", __func__, SENSOR_0_CLK);
		return -1;
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (!axi_clk) {
		CAM_ERROR_PRINTK("%s:unable to get clock csi0_axi_clk\n", __func__);
		return -1;
	}
	VCAM_A_2_8_V = regulator_get(NULL,"cam");
	if(IS_ERR(VCAM_A_2_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_A_2_8_V.8V\n");
		return -1;
	}

	VCAM_C_1_8_V = regulator_get(NULL,"hv9");
	if(IS_ERR(VCAM_C_1_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_C_1_8_V\n");
		return -1;
	}	

#ifdef CONFIG_SOC_CAMERA_POWER_USE_ASR   //VE Group
	VCAM_CORE_1_8_V = regulator_get(NULL,"asr_nm_uc");
	if(IS_ERR(VCAM_CORE_1_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_CORE_1_8_V\n");
		return -1;
	}	
#else
	gpio_request(CAM_IO_EN, "cam_1_2v");
	gpio_direction_output(CAM_IO_EN,0); 
#endif


	gpio_request(CAM_EN, "cam_stnby");
	gpio_direction_output(CAM_EN,0);	
	
	gpio_request(CAM_RST, "cam_rst");
	gpio_direction_output(CAM_RST,0);
	
	if(on)
{
		CAM_INFO_PRINTK("power on the sensor\n");
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			CAM_ERROR_PRINTK("%s:failed to update dfs request for unicam\n",
			       __func__);
			return -1;
		}

		regulator_set_voltage(VCAM_A_2_8_V,2800000,2800000);
		regulator_set_voltage(VCAM_C_1_8_V,1800000,1800000);	

#ifdef CONFIG_SOC_CAMERA_POWER_USE_ASR   //VE Group
		regulator_set_voltage(VCAM_CORE_1_8_V,1800000,1800000);
#endif	

		value = clk_enable(axi_clk);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable csi2 axi clock\n", __func__);
			return -1;
		}

		CAM_INFO_PRINTK("power on the sensor's power supply\n"); //@HW

	
#ifdef CONFIG_SOC_CAMERA_POWER_USE_ASR   //VE Group	
		regulator_enable(VCAM_CORE_1_8_V);
#else
		gpio_set_value(CAM_IO_EN,1); 
#endif		 

		regulator_enable(VCAM_A_2_8_V);
		
		regulator_enable(VCAM_C_1_8_V);
	
		msleep(5);
	
		// mclk control
		value = clk_enable(clock);
		if (value) {
			CAM_ERROR_PRINTK("%s: failed to enable clock %s\n", __func__,
				SENSOR_0_CLK);
			return -1;
		}

		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			CAM_ERROR_PRINTK("%s: failed to set the clock %s to freq %d\n",
					__func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}

		gpio_set_value(CAM_EN,1);

 		msleep(30);

		gpio_set_value(CAM_RST,1);
 		msleep(6);

		CAM_INFO_PRINTK("set cam rst to high\n");
		msleep(2);
	}
	else  //power off
	{
		CAM_INFO_PRINTK("power off the sensor \n"); //@HW

		/* enable reset gpio */
		gpio_set_value(CAM_RST,0);
		msleep(1);
		
		clk_disable(clock);
		clk_disable(axi_clk);

		gpio_set_value(CAM_EN,0);
		msleep(1);

	//	gpio_set_value(CAM1_RESET,0);
	//	msleep(1);
		
		/* enable power down gpio */

	//	gpio_set_value(CAM_AF_EN,0); 
	//	msleep(1);

		regulator_disable(VCAM_C_1_8_V);
		regulator_disable(VCAM_A_2_8_V);

		
#ifdef CONFIG_SOC_CAMERA_POWER_USE_ASR   //VE Group		
		regulator_disable(VCAM_CORE_1_8_V);
#else
		gpio_set_value(CAM_IO_EN, 0);
#endif


		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
					      PI_MGR_DFS_MIN_VALUE)) {
			CAM_ERROR_PRINTK("%s: failed to update dfs request for unicam\n",
				 __func__);
		}
		CAM_INFO_PRINTK("rhea_camera_power off success \n");
	}

	if (ANTI_BANDING_60HZ == camera_antibanding_get()) {
		ret = camdrv_ss_sr200pc20m_check_table_size_for_60hz();
		if(ret != 0) {
			CAM_ERROR_PRINTK("%s: Fail - the table num is %d \n", __func__, ret);
			return -1;
		}
		camdrv_ss_sr200pc20m_copy_files_for_60hz();
	}

	return 0;
}


int camdrv_ss_sr200pc20m_get_sensor_param_for_exif(
	struct v4l2_subdev *sd,
	struct v4l2_exif_sensor_info *exif_param)
{
	char str[20];
	int num = -1;
	int ret = -1;
	float exposureTime = 0.0f;

	strcpy(exif_param->strSoftware,		EXIF_SOFTWARE);
	strcpy(exif_param->strMake,		EXIF_MAKE);
	strcpy(exif_param->strModel,		EXIF_MODEL);

	exposureTime = camdrv_ss_sr200pc20m_get_exposureTime(sd);
	num = (int)exposureTime;
	if (num > 0) 
	{
		snprintf(str, 19, "%d/26000000", num);
		strcpy(exif_param->exposureTime, str);
	} 
	else 
	{
		strcpy(exif_param->exposureTime, "");
	}
	
	CAM_INFO_PRINTK("%s : exposure time =  %s \n",__func__,exif_param->exposureTime);

	num = camdrv_ss_sr200pc20m_get_iso_speed_rate(sd);
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

	strcpy(exif_param->FNumber,		(char *)"28/10");
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


bool camdrv_ss_sr200pc20m_get_esd_status(struct v4l2_subdev *sd)
{
	return false;
}


bool camdrv_ss_sensor_functions_sr200pc20m(struct camdrv_ss_sensor_cap *sensor)
{

	strcpy(sensor->name,SR200PC20M_NAME);
	sensor->supported_preview_framesize_list  = sr200pc20m_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(sr200pc20m_supported_preview_framesize_list);
	
	sensor->supported_capture_framesize_list  =  sr200pc20m_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(sr200pc20m_supported_capture_framesize_list);
	
	sensor->fmts 				   = sr200pc20m_fmts;
	sensor->rows_num_fmts		   =ARRAY_SIZE(sr200pc20m_fmts);
	

	sensor->controls				   =sr200pc20m_controls;
	sensor->rows_num_controls	      =ARRAY_SIZE(sr200pc20m_controls);

	sensor->default_pix_fmt 				   = SR200PC20M_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt			   = SR200PC20M_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size 		  			 = SR200PC20M_REGISTER_SIZE;
	//sensor->skip_frames						 = 5;
	/* sensor dependent functions */
	
	sensor->delay_duration				= SR200PC20M_DELAY_DURATION;
	
	
/* mandatory*/
	sensor->thumbnail_ioctl			       = camdrv_ss_sr200pc20m_ss_ioctl;
	sensor->enum_frameintervals			   = camdrv_ss_sr200pc20m_enum_frameintervals;
	
/*optional*/
	sensor->get_nightmode		   = camdrv_ss_sr200pc20m_get_nightmode; //aska add
	sensor->set_preview_start      = camdrv_ss_sr200pc20m_set_preview_start;//aska
	sensor->set_iso      					 = camdrv_ss_sr200pc20m_set_iso;//aska add
	sensor->set_white_balance      = camdrv_ss_sr200pc20m_set_white_balance;//aska add
	sensor->get_ae_stable_status      =  camdrv_ss_sr200pc20m_get_ae_stable_status;
//	sensor->set_auto_focus		 	  =  camdrv_ss_sr200pc20m_set_auto_focus;
//	sensor->get_auto_focus_status     = camdrv_ss_sr200pc20m_get_auto_focus_status;
	
//	sensor->set_touch_focus		 	  =  camdrv_ss_sr200pc20m_set_touch_focus;
//	sensor->get_touch_focus_status     = camdrv_ss_sr200pc20m_get_touch_focus_status;
			
	/* sensor->flash_control		= camdrv_ss_sr200pc20m_AAT_flash_control; */
//	sensor->i2c_set_data_burst   	   = camdrv_ss_sr200pc20m_i2c_set_data_burst;
	sensor->check_flash_needed   	   = camdrv_ss_sr200pc20m_check_flash_needed;
//	sensor->get_light_condition   = camdrv_ss_sr200pc20m_get_light_condition;

	sensor->sensor_power			= camdrv_ss_sr200pc20m_sensor_power;		  

	sensor->get_exif_sensor_info    = camdrv_ss_sr200pc20m_get_sensor_param_for_exif;
	sensor->getEsdStatus		 =	  camdrv_ss_sr200pc20m_get_esd_status;

	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */

	sensor->init_regs						  = sr200pc20m_init_regs;
	sensor->rows_num_init_regs				  = ARRAY_SIZE(sr200pc20m_init_regs);

	sensor->preview_camera_regs 			  = sr200pc20m_preview_camera_regs;
	sensor->rows_num_preview_camera_regs 	  = ARRAY_SIZE(sr200pc20m_preview_camera_regs);

	/*snapshot mode*/
	sensor->snapshot_normal_regs			  =	sr200pc20m_snapshot_normal_regs;
	sensor->rows_num_snapshot_normal_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_normal_regs);

	sensor->snapshot_lowlight_regs			  =	sr200pc20m_snapshot_lowlight_regs;
	sensor->rows_num_snapshot_lowlight_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_lowlight_regs);

	sensor->snapshot_highlight_regs			  =	sr200pc20m_snapshot_highlight_regs;
	sensor->rows_num_snapshot_highlight_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_highlight_regs);

	sensor->snapshot_nightmode_regs			  =	sr200pc20m_snapshot_nightmode_regs;
	sensor->rows_num_snapshot_nightmode_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_nightmode_regs);

	sensor->snapshot_flash_on_regs			  =	sr200pc20m_snapshot_flash_on_regs;
	sensor->rows_num_snapshot_flash_on_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_flash_on_regs);

	sensor->snapshot_af_preflash_on_regs			  =	sr200pc20m_snapshot_af_preflash_on_regs;
	sensor->rows_num_snapshot_af_preflash_on_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_af_preflash_on_regs);

	sensor->snapshot_af_preflash_off_regs			  =	sr200pc20m_snapshot_af_preflash_off_regs;
	sensor->rows_num_snapshot_af_preflash_off_regs	  = ARRAY_SIZE(sr200pc20m_snapshot_af_preflash_off_regs);

#if 0
	sensor->focus_mode_macro_regs			  =	sr200pc20m_af_macro_mode_regs;
	sensor->rows_num_focus_mode_macro_regs	  = ARRAY_SIZE(sr200pc20m_af_macro_mode_regs);

	sensor->focus_mode_off_regs			  =	sr200pc20m_af_normal_mode_regs;
	sensor->rows_num_focus_mode_off_regs	  = ARRAY_SIZE(sr200pc20m_af_normal_mode_regs);

	sensor->single_af_start_regs			  =	sr200pc20m_single_af_start_regs;
	sensor->rows_num_single_af_start_regs	  = ARRAY_SIZE(sr200pc20m_single_af_start_regs);

	sensor->get_1st_af_search_status			  =	sr200pc20m_get_1st_af_search_status;
	sensor->rows_num_get_1st_af_search_status	  = ARRAY_SIZE(sr200pc20m_get_1st_af_search_status);

	sensor->get_2nd_af_search_status			  =	sr200pc20m_get_2nd_af_search_status;
	sensor->rows_num_get_2nd_af_search_status	  = ARRAY_SIZE(sr200pc20m_get_2nd_af_search_status);

	sensor->single_af_stop_regs			  =	sr200pc20m_single_af_stop_regs;
	sensor->rows_num_single_af_stop_regs	  = ARRAY_SIZE(sr200pc20m_single_af_stop_regs);
#endif


	/*effect*/
	sensor->effect_normal_regs			      =	sr200pc20m_effect_normal_regs;
	sensor->rows_num_effect_normal_regs      = ARRAY_SIZE(sr200pc20m_effect_normal_regs);
	
	sensor->effect_negative_regs		      =	sr200pc20m_effect_negative_regs;
	sensor->rows_num_effect_negative_regs	 = ARRAY_SIZE(sr200pc20m_effect_negative_regs);
	
	sensor->effect_sepia_regs			      =	sr200pc20m_effect_sepia_regs;
	sensor->rows_num_effect_sepia_regs	  	  = ARRAY_SIZE(sr200pc20m_effect_sepia_regs);
	
	sensor->effect_mono_regs			      =	sr200pc20m_effect_mono_regs;
	sensor->rows_num_effect_mono_regs	      = ARRAY_SIZE(sr200pc20m_effect_mono_regs);

	sensor->effect_aqua_regs				  =	sr200pc20m_effect_aqua_regs;
	sensor->rows_num_effect_aqua_regs	  	  = ARRAY_SIZE(sr200pc20m_effect_aqua_regs);
	
	sensor->effect_sharpen_regs 		      =	sr200pc20m_effect_sharpen_regs;
	sensor->rows_num_effect_sharpen_regs     = ARRAY_SIZE(sr200pc20m_effect_sharpen_regs);
	
	sensor->effect_solarization_regs		   = sr200pc20m_effect_solarization_regs;
	sensor->rows_num_effect_solarization_regs = ARRAY_SIZE(sr200pc20m_effect_solarization_regs);
	
	sensor->effect_black_white_regs 	       =	sr200pc20m_effect_black_white_regs;
	sensor->rows_num_effect_black_white_regs  = ARRAY_SIZE(sr200pc20m_effect_black_white_regs);
	

	/*wb*/
	sensor->wb_auto_regs				  =	sr200pc20m_wb_auto_regs;
	sensor->rows_num_wb_auto_regs	  	  = ARRAY_SIZE(sr200pc20m_wb_auto_regs);

	//sensor->wb_sunny_regs				 =	sr200pc20m_wb_sunny_regs;
	//sensor->rows_num_wb_sunny_regs	  	 = ARRAY_SIZE(sr200pc20m_wb_sunny_regs);
	
	sensor->wb_cloudy_regs				 =	sr200pc20m_wb_cloudy_regs;
	sensor->rows_num_wb_cloudy_regs	 = ARRAY_SIZE(sr200pc20m_wb_cloudy_regs);
	
	//sensor->wb_tungsten_regs			 =	sr200pc20m_wb_tungsten_regs;
	//sensor->rows_num_wb_tungsten_regs	 = ARRAY_SIZE(sr200pc20m_wb_tungsten_regs);
	//Changed reg table name to fit UI's name
	sensor->wb_daylight_regs				 =	sr200pc20m_wb_daylight_regs;
	sensor->rows_num_wb_daylight_regs	  	 = ARRAY_SIZE(sr200pc20m_wb_daylight_regs);
	sensor->wb_incandescent_regs				 =	sr200pc20m_wb_incandescent_regs;
	sensor->rows_num_wb_incandescent_regs	  	 = ARRAY_SIZE(sr200pc20m_wb_incandescent_regs);

	sensor->wb_fluorescent_regs 		  =	sr200pc20m_wb_fluorescent_regs;
	sensor->rows_num_wb_fluorescent_regs  = ARRAY_SIZE(sr200pc20m_wb_fluorescent_regs);


	/*metering*/
	sensor->metering_matrix_regs		  =	sr200pc20m_metering_matrix_regs;
	sensor->rows_num_metering_matrix_regs	  	  = ARRAY_SIZE(sr200pc20m_metering_matrix_regs);

	sensor->metering_center_regs		  =	sr200pc20m_metering_center_regs;
	sensor->rows_num_metering_center_regs	  	  = ARRAY_SIZE(sr200pc20m_metering_center_regs);

	sensor->metering_spot_regs			  =	  sr200pc20m_metering_spot_regs;
	sensor->rows_num_metering_spot_regs	  		  = ARRAY_SIZE(sr200pc20m_metering_spot_regs);
	
	/*EV*/
	sensor->ev_minus_4_regs 			 =	sr200pc20m_ev_minus_4_regs;
	sensor->rows_num_ev_minus_4_regs	 = ARRAY_SIZE(sr200pc20m_ev_minus_4_regs);

	sensor->ev_minus_3_regs 			 =	sr200pc20m_ev_minus_3_regs;
	sensor->rows_num_ev_minus_3_regs	 = ARRAY_SIZE(sr200pc20m_ev_minus_3_regs);

	sensor->ev_minus_2_regs 			 =	sr200pc20m_ev_minus_2_regs;
	sensor->rows_num_ev_minus_2_regs	  = ARRAY_SIZE(sr200pc20m_ev_minus_2_regs);

	sensor->ev_minus_1_regs 			 =	sr200pc20m_ev_minus_1_regs;
	sensor->rows_num_ev_minus_1_regs	 = ARRAY_SIZE(sr200pc20m_ev_minus_1_regs);

	sensor->ev_default_regs 			 =	sr200pc20m_ev_default_regs;
	sensor->rows_num_ev_default_regs	 = ARRAY_SIZE(sr200pc20m_ev_default_regs);

	sensor->ev_plus_1_regs				 =	sr200pc20m_ev_plus_1_regs;
	sensor->rows_num_ev_plus_1_regs	 = ARRAY_SIZE(sr200pc20m_ev_plus_1_regs);

	sensor->ev_plus_2_regs				 =	sr200pc20m_ev_plus_2_regs;
	sensor->rows_num_ev_plus_2_regs	 = ARRAY_SIZE(sr200pc20m_ev_plus_2_regs);

	sensor->ev_plus_3_regs				 =	sr200pc20m_ev_plus_3_regs;
	sensor->rows_num_ev_plus_3_regs	 = ARRAY_SIZE(sr200pc20m_ev_plus_3_regs);

	sensor->ev_plus_4_regs				 =	sr200pc20m_ev_plus_4_regs;
	sensor->rows_num_ev_plus_4_regs	 = ARRAY_SIZE(sr200pc20m_ev_plus_4_regs);

	
	/*contrast*/
	sensor->contrast_minus_2_regs		 	 =	sr200pc20m_contrast_minus_2_regs;
	sensor->rows_num_contrast_minus_2_regs	 = ARRAY_SIZE(sr200pc20m_contrast_minus_2_regs);

	sensor->contrast_minus_1_regs		     =	sr200pc20m_contrast_minus_1_regs;
	sensor->rows_num_contrast_minus_1_regs	 = ARRAY_SIZE(sr200pc20m_contrast_minus_1_regs);
  
	sensor->contrast_default_regs			 =	sr200pc20m_contrast_default_regs;
	sensor->rows_num_contrast_default_regs  = ARRAY_SIZE(sr200pc20m_contrast_default_regs);

	sensor->contrast_plus_1_regs			 =	sr200pc20m_contrast_plus_1_regs;
	sensor->rows_num_contrast_plus_1_regs	 = ARRAY_SIZE(sr200pc20m_contrast_plus_1_regs);

	sensor->contrast_plus_2_regs			 =	sr200pc20m_contrast_plus_2_regs;
	sensor->rows_num_contrast_plus_2_regs	 = ARRAY_SIZE(sr200pc20m_contrast_plus_2_regs);
	
	/*sharpness*/ 
	sensor->sharpness_minus_3_regs		     =	sr200pc20m_sharpness_minus_3_regs;
	sensor->rows_num_sharpness_minus_3_regs= ARRAY_SIZE(sr200pc20m_sharpness_minus_3_regs);

	sensor->sharpness_minus_2_regs		     =	sr200pc20m_sharpness_minus_2_regs;
	sensor->rows_num_sharpness_minus_2_regs= ARRAY_SIZE(sr200pc20m_sharpness_minus_2_regs);

	sensor->sharpness_minus_1_regs		 	 =	sr200pc20m_sharpness_minus_1_regs;
	sensor->rows_num_sharpness_minus_1_regs = ARRAY_SIZE(sr200pc20m_sharpness_minus_1_regs);

	sensor->sharpness_default_regs		 	 =	sr200pc20m_sharpness_default_regs;
	sensor->rows_num_sharpness_default_regs  = ARRAY_SIZE(sr200pc20m_sharpness_default_regs);

	sensor->sharpness_plus_1_regs		     =	sr200pc20m_sharpness_plus_1_regs;
	sensor->rows_num_sharpness_plus_1_regs	 =	ARRAY_SIZE(sr200pc20m_sharpness_plus_1_regs);

	sensor->sharpness_plus_2_regs		     =	sr200pc20m_sharpness_plus_2_regs;
	sensor->rows_num_sharpness_plus_2_regs	 =	ARRAY_SIZE(sr200pc20m_sharpness_plus_2_regs);
    
	sensor->sharpness_plus_3_regs		     =	sr200pc20m_sharpness_plus_3_regs;
	sensor->rows_num_sharpness_plus_3_regs	 =	ARRAY_SIZE(sr200pc20m_sharpness_plus_3_regs);

		
	/*saturation*/
	sensor->saturation_minus_2_regs 	      =	sr200pc20m_saturation_minus_2_regs;
	sensor->rows_num_saturation_minus_2_regs = ARRAY_SIZE(sr200pc20m_saturation_minus_2_regs);

	sensor->saturation_minus_1_regs 	 	  =	sr200pc20m_saturation_minus_1_regs;
	sensor->rows_num_saturation_minus_1_regs = ARRAY_SIZE(sr200pc20m_saturation_minus_1_regs);

	sensor->saturation_default_regs 	      =	sr200pc20m_saturation_default_regs;
	sensor->rows_num_saturation_default_regs  = ARRAY_SIZE(sr200pc20m_saturation_default_regs);

	sensor->saturation_plus_1_regs		       =	sr200pc20m_saturation_plus_1_regs;
	sensor->rows_num_saturation_plus_1_regs	= ARRAY_SIZE(sr200pc20m_saturation_plus_1_regs);

	sensor->saturation_plus_2_regs		       =	sr200pc20m_saturation_plus_2_regs;
	sensor->rows_num_saturation_plus_2_regs   = ARRAY_SIZE(sr200pc20m_saturation_plus_2_regs);

	
	/*zoom*/
	sensor->zoom_00_regs					 =	sr200pc20m_zoom_00_regs;
	sensor->rows_num_zoom_00_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_00_regs);

	sensor->zoom_01_regs					 =	sr200pc20m_zoom_01_regs;
	sensor->rows_num_zoom_01_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_01_regs);

	sensor->zoom_02_regs					 =	sr200pc20m_zoom_02_regs;
	sensor->rows_num_zoom_02_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_02_regs);

	sensor->zoom_03_regs					 =	sr200pc20m_zoom_03_regs;
	sensor->rows_num_zoom_03_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_03_regs);

	sensor->zoom_04_regs					 =	sr200pc20m_zoom_04_regs;
	sensor->rows_num_zoom_04_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_04_regs);

	sensor->zoom_05_regs					 =	sr200pc20m_zoom_05_regs;
	sensor->rows_num_zoom_05_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_05_regs);

	sensor->zoom_06_regs					 =	sr200pc20m_zoom_06_regs;
	sensor->rows_num_zoom_06_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_06_regs);

	sensor->zoom_07_regs					 =	sr200pc20m_zoom_07_regs;
	sensor->rows_num_zoom_07_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_07_regs);

	sensor->zoom_08_regs					 =	sr200pc20m_zoom_08_regs;
	sensor->rows_num_zoom_08_regs	  		  = ARRAY_SIZE(sr200pc20m_zoom_08_regs);

	
	/*scene mode*/
	sensor->scene_none_regs 			 		=	sr200pc20m_scene_none_regs;
	sensor->rows_num_scene_none_regs	  		 = ARRAY_SIZE(sr200pc20m_scene_none_regs);

	sensor->scene_portrait_regs 		 		=	sr200pc20m_scene_portrait_regs;
	sensor->rows_num_scene_portrait_regs	  	= ARRAY_SIZE(sr200pc20m_scene_portrait_regs);

	sensor->scene_nightshot_regs			   =	sr200pc20m_scene_nightshot_Normal;
	sensor->rows_num_scene_nightshot_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_nightshot_Normal);
	sensor->scene_nightshot_dark_regs			   =	sr200pc20m_scene_nightshot_Dark;
	sensor->rows_num_scene_nightshot_dark_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_nightshot_Dark);

	sensor->scene_backlight_regs			  =	sr200pc20m_scene_backlight_regs;
	sensor->rows_num_scene_backlight_regs	   = ARRAY_SIZE(sr200pc20m_scene_backlight_regs);

	sensor->scene_landscape_regs			   =	sr200pc20m_scene_landscape_regs;
	sensor->rows_num_scene_landscape_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_landscape_regs);

	sensor->scene_sports_regs			      =	sr200pc20m_scene_sports_regs;
	sensor->rows_num_scene_sports_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_sports_regs);

	sensor->scene_party_indoor_regs 	 	  =	sr200pc20m_scene_party_indoor_regs;
	sensor->rows_num_scene_party_indoor_regs  = ARRAY_SIZE(sr200pc20m_scene_party_indoor_regs);

	sensor->scene_beach_snow_regs				 =	sr200pc20m_scene_beach_snow_regs;
	sensor->rows_num_scene_beach_snow_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_beach_snow_regs);

	sensor->scene_sunset_regs			 		 =	sr200pc20m_scene_sunset_regs;
	sensor->rows_num_scene_sunset_regs	  		  = ARRAY_SIZE(sr200pc20m_scene_sunset_regs);

	sensor->scene_duskdawn_regs 				 =	sr200pc20m_scene_duskdawn_regs;
	sensor->rows_num_scene_duskdawn_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_duskdawn_regs);

	sensor->scene_fall_color_regs				 =	sr200pc20m_scene_fall_color_regs;
	sensor->rows_num_scene_fall_color_regs	  	  = ARRAY_SIZE(sr200pc20m_scene_fall_color_regs);

	sensor->scene_fireworks_regs				 =	sr200pc20m_scene_fireworks_regs;
	sensor->rows_num_scene_fireworks_regs	  	  = ARRAY_SIZE(sr200pc20m_scene_fireworks_regs);
	
	sensor->scene_candle_light_regs 	 		=	sr200pc20m_scene_candle_light_regs;
	sensor->rows_num_scene_candle_light_regs	= ARRAY_SIZE(sr200pc20m_scene_candle_light_regs);

	sensor->scene_text_regs			   =	sr200pc20m_scene_text_regs;
	sensor->rows_num_scene_text_regs	  	 = ARRAY_SIZE(sr200pc20m_scene_text_regs);

		
	/*fps*/
	sensor->fps_auto_regs				 =	sr200pc20m_fps_auto_regs;
	sensor->rows_num_fps_auto_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_auto_regs);

	sensor->fps_5_regs					 =	sr200pc20m_fps_5_regs;
	sensor->rows_num_fps_5_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_5_regs);

	sensor->fps_7_regs					 =	sr200pc20m_fps_7_regs;
	sensor->rows_num_fps_7_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_7_regs);

	sensor->fps_10_regs 				 =	sr200pc20m_fps_10_regs;
	sensor->rows_num_fps_10_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_10_regs);

	sensor->fps_15_regs 				 =	sr200pc20m_fps_15_regs;
	sensor->rows_num_fps_15_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_15_regs);

	sensor->fps_20_regs 				 =	sr200pc20m_fps_20_regs;
	sensor->rows_num_fps_20_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_20_regs);

	sensor->fps_25_regs 				 =	sr200pc20m_fps_25_regs;
	sensor->rows_num_fps_25_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_25_regs);

	sensor->fps_30_regs 				 =	sr200pc20m_fps_30_regs;
	sensor->rows_num_fps_30_regs 		  = ARRAY_SIZE(sr200pc20m_fps_30_regs);
	
	sensor->fps_60_regs 				 =	sr200pc20m_fps_60_regs;
	sensor->rows_num_fps_60_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_60_regs);

	sensor->fps_120_regs 				 =	sr200pc20m_fps_120_regs;
	sensor->rows_num_fps_120_regs	  		  = ARRAY_SIZE(sr200pc20m_fps_120_regs);
	

	
	/*quality*/
	sensor->quality_superfine_regs			 =	sr200pc20m_quality_superfine_regs;
	sensor->rows_num_quality_superfine_regs	  = ARRAY_SIZE(sr200pc20m_quality_superfine_regs);

	sensor->quality_fine_regs			 =	sr200pc20m_quality_fine_regs;
	sensor->rows_num_quality_fine_regs	  = ARRAY_SIZE(sr200pc20m_quality_fine_regs);

	sensor->quality_normal_regs 		   =	sr200pc20m_quality_normal_regs;
	sensor->rows_num_quality_normal_regs  = ARRAY_SIZE(sr200pc20m_effect_normal_regs);

	sensor->quality_economy_regs			 =	sr200pc20m_quality_economy_regs;
	sensor->rows_num_quality_economy_regs   = ARRAY_SIZE(sr200pc20m_quality_economy_regs);

	
	/*preview size */
	sensor->preview_size_176x144_regs	        =	sr200pc20m_preview_size_176x144_regs;
	sensor->rows_num_preview_size_176x144_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_176x144_regs);

	sensor->preview_size_320x240_regs	         =	sr200pc20m_preview_size_320x240_regs; 
	sensor->rows_num_preview_size_320x240_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_320x240_regs);

	sensor->preview_size_352x288_regs	          =	sr200pc20m_preview_size_352x288_regs; 
	sensor->rows_num_preview_size_352x288_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_352x288_regs);

	sensor->preview_size_640x480_regs	          =	sr200pc20m_preview_size_640x480_regs; 
	sensor->rows_num_preview_size_640x480_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_640x480_regs);

	sensor->preview_size_704x576_regs	 		=	sr200pc20m_preview_size_704x576_regs; 
	sensor->rows_num_preview_size_704x576_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_704x576_regs);

	sensor->preview_size_720x480_regs	 		=	sr200pc20m_preview_size_720x480_regs; 
	sensor->rows_num_preview_size_720x480_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_720x480_regs);
	
	sensor->preview_size_800x480_regs	        =	sr200pc20m_preview_size_800x480_regs;
	sensor->rows_num_preview_size_800x480_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_800x480_regs);

	sensor->preview_size_800x600_regs	        =	sr200pc20m_preview_size_800x600_regs;
	sensor->rows_num_preview_size_800x600_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_800x600_regs);

	sensor->preview_size_1024x600_regs	         =	sr200pc20m_preview_size_1024x600_regs; 
	sensor->rows_num_preview_size_1024x600_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_1024x600_regs);

	sensor->preview_size_1024x768_regs	          =	sr200pc20m_preview_size_1024x768_regs; 
	sensor->rows_num_preview_size_1024x768_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_1024x768_regs);

	sensor->preview_size_1280x960_regs	          =	sr200pc20m_preview_size_1280x960_regs; 
	sensor->rows_num_preview_size_1280x960_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_1280x960_regs);

	sensor->preview_size_1600x960_regs	 		=	sr200pc20m_preview_size_1600x960_regs; 
	sensor->rows_num_preview_size_1600x960_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_1600x960_regs);

	sensor->preview_size_1600x1200_regs	 		=	sr200pc20m_preview_size_1600x1200_regs; 
	sensor->rows_num_preview_size_1600x1200_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_1600x1200_regs);

	sensor->preview_size_2048x1232_regs	        =	sr200pc20m_preview_size_2048x1232_regs;
	sensor->rows_num_preview_size_2048x1232_regs	 = ARRAY_SIZE(sr200pc20m_preview_size_2048x1232_regs);

	sensor->preview_size_2048x1536_regs	         =	sr200pc20m_preview_size_2048x1536_regs; 
	sensor->rows_num_preview_size_2048x1536_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_2048x1536_regs);

	sensor->preview_size_2560x1920_regs	          =	sr200pc20m_preview_size_2560x1920_regs; 
	sensor->rows_num_preview_size_2560x1920_regs	  = ARRAY_SIZE(sr200pc20m_preview_size_2560x1920_regs);
  
	
	/*Capture size */
	sensor->capture_size_640x480_regs	 		=	sr200pc20m_capture_size_640x480_regs;
	sensor->rows_num_capture_size_640x480_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_640x480_regs);

	sensor->capture_size_720x480_regs  			=	sr200pc20m_capture_size_720x480_regs; 
	sensor->rows_num_capture_size_720x480_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_720x480_regs);

	sensor->capture_size_800x480_regs	 		=	sr200pc20m_capture_size_800x480_regs;
	sensor->rows_num_capture_size_800x480_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_800x480_regs);

	sensor->capture_size_800x486_regs	 		=	sr200pc20m_capture_size_800x486_regs;
	sensor->rows_num_capture_size_800x486_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_800x486_regs);

	sensor->capture_size_800x600_regs  			=	sr200pc20m_capture_size_800x600_regs; 
	sensor->rows_num_capture_size_800x600_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_800x600_regs);

    	sensor->capture_size_1024x600_regs	 		=	sr200pc20m_capture_size_1024x600_regs;
	sensor->rows_num_capture_size_1024x600_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_1024x600_regs);

	sensor->capture_size_1024x768_regs  			=	sr200pc20m_capture_size_1024x768_regs; 
	sensor->rows_num_capture_size_1024x768_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_1024x768_regs);

	sensor->capture_size_1280x960_regs  			=	sr200pc20m_capture_size_1280x960_regs; 
	sensor->rows_num_capture_size_1280x960_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_1280x960_regs);

    	sensor->capture_size_1600x960_regs	 		=	sr200pc20m_capture_size_1600x960_regs;
	sensor->rows_num_capture_size_1600x960_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_1600x960_regs);

	sensor->capture_size_1600x1200_regs  			=	sr200pc20m_capture_size_1600x1200_regs; 
	sensor->rows_num_capture_size_1600x1200_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_1600x1200_regs);

	sensor->capture_size_2048x1232_regs  			=	sr200pc20m_capture_size_2048x1232_regs; 
	sensor->rows_num_capture_size_2048x1232_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_2048x1232_regs);

	sensor->capture_size_2048x1536_regs  			=	sr200pc20m_capture_size_2048x1536_regs; 
	sensor->rows_num_capture_size_2048x1536_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_2048x1536_regs);

	sensor->capture_size_2560x1536_regs  			=	sr200pc20m_capture_size_2560x1536_regs; 
	sensor->rows_num_capture_size_2560x1536_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_2560x1536_regs);

	sensor->capture_size_2560x1920_regs  			=	sr200pc20m_capture_size_2560x1920_regs; 
	sensor->rows_num_capture_size_2560x1920_regs	 = ARRAY_SIZE(sr200pc20m_capture_size_2560x1920_regs);

	
	/*pattern*/
	sensor->pattern_on_regs 			  = sr200pc20m_pattern_on_regs;
	sensor->rows_num_pattern_on_regs	  = ARRAY_SIZE(sr200pc20m_pattern_on_regs);
	
	sensor->pattern_off_regs			  = sr200pc20m_pattern_off_regs;
	sensor->rows_num_pattern_off_regs	  = ARRAY_SIZE(sr200pc20m_pattern_off_regs);

	/*AE*/
	sensor->ae_lock_regs			  = sr200pc20m_ae_lock_regs;
	sensor->rows_num_ae_lock_regs	  = ARRAY_SIZE(sr200pc20m_ae_lock_regs);

		
	sensor->ae_unlock_regs			  = sr200pc20m_ae_unlock_regs;
	sensor->rows_num_ae_unlock_regs	  = ARRAY_SIZE(sr200pc20m_ae_unlock_regs);


	/*AWB*/

	sensor->awb_lock_regs			  = sr200pc20m_awb_lock_regs;
	sensor->rows_num_awb_lock_regs	  = ARRAY_SIZE(sr200pc20m_awb_lock_regs);

	sensor->awb_unlock_regs			  = sr200pc20m_awb_unlock_regs;
	sensor->rows_num_awb_unlock_regs	  = ARRAY_SIZE(sr200pc20m_awb_unlock_regs);
		
	//ISO//
	sensor->iso_auto_regs			  = sr200pc20m_iso_auto_regs;
	sensor->rows_num_iso_auto_regs	  = ARRAY_SIZE(sr200pc20m_iso_auto_regs);

	sensor->iso_50_regs			  = sr200pc20m_iso_50_regs;
	sensor->rows_num_iso_50_regs	  = ARRAY_SIZE(sr200pc20m_iso_50_regs);

	sensor->iso_100_regs			  = sr200pc20m_iso_100_regs;
	sensor->rows_num_iso_100_regs	  = ARRAY_SIZE(sr200pc20m_iso_100_regs);

	sensor->iso_200_regs			  = sr200pc20m_iso_200_regs;
	sensor->rows_num_iso_200_regs	  = ARRAY_SIZE(sr200pc20m_iso_200_regs);

	sensor->iso_400_regs			  = sr200pc20m_iso_400_regs;
	sensor->rows_num_iso_400_regs	  = ARRAY_SIZE(sr200pc20m_iso_400_regs);

	return true;
};

int camdrv_ss_read_device_id_sr200pc20m(
		struct i2c_client *client, char *device_id)
{
	int ret = -1;
	/* NEED to WRITE THE I2c REad code to read the deviceid */
	return 0;
}

static int __init camdrv_ss_sr200pc20m_mod_init(void)
{
	struct camdrv_ss_sensor_reg sens;

	strncpy(sens.name, SR200PC20M_NAME, sizeof(SR200PC20M_NAME));
	sens.sensor_functions = camdrv_ss_sensor_functions_sr200pc20m;
	sens.sensor_power = camdrv_ss_sr200pc20m_sensor_power;
	sens.read_device_id = camdrv_ss_read_device_id_sr200pc20m;
#ifdef CONFIG_SOC_CAMERA_MAIN_SR200PC20M
	sens.isMainSensor = 1;
#endif

#ifdef CONFIG_SOC_CAMERA_SUB_SR200PC20M
	sens.isMainSensor = 0;
#endif
	camdrv_ss_sensors_register(&sens);

}
module_init(camdrv_ss_sr200pc20m_mod_init);

MODULE_DESCRIPTION("SAMSUNG CAMERA SENSOR SR200PC20M ");
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");

