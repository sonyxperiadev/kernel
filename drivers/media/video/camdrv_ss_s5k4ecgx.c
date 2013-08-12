
/***********************************************************************
* Driver for S5K4ECGX (5MP Camera) from SAMSUNG SYSTEM LSI
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
#include <camdrv_ss_s5k4ecgx.h>           
#include <linux/module.h>

#define S5K4ECGX_NAME	"s5k4ecgx"
#define SENSOR_ID 2
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x4b0000  // 4915200  = 2560*1920     //0x33F000     //3403776 //2216 * 1536
#define SENSOR_PREVIEW_WIDTH      1024
#define SENSOR_PREVIEW_HEIGHT     768
#define AF_OUTER_WINDOW_WIDTH   512 //320
#define AF_OUTER_WINDOW_HEIGHT  426 //266
#define AF_INNER_WINDOW_WIDTH   230 //143
#define AF_INNER_WINDOW_HEIGHT  230 //143
#define MAX_BUFFER			(4096)

#define S5K4ECGX_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define S5K4ECGX_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8 
#define FORMAT_FLAGS_COMPRESSED 0x3
#define DEFUALT_MCLK		26000000
#define S5K4ECGX_REGISTER_SIZE 4
static bool camdrv_ss_s5k4ecgx_check_flash_needed(struct v4l2_subdev *sd);
static int camdrv_ss_s5k4ecgx_AAT_flash_control(struct v4l2_subdev *sd, int control_mode);
static int camdrv_ss_s5k4ecgx_MIC2871_flash_control(struct v4l2_subdev *sd, int control_mode);
static int camdrv_ss_s5k4ecgx_DS8515_flash_control(struct v4l2_subdev *sd, int control_mode);
static  camdrv_ss_s5k4ecgx_set_ae_stable_status(struct v4l2_subdev *sd, unsigned short value);//Nikhil
static int camdrv_ss_s5k4ecgx_get_ae_stable_status_value(struct v4l2_subdev *sd);//Nikihl
int camdrv_ss_s5k4ecgx_set_preflash_sequence(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
#define S5K4ECGX_DELAY_DURATION 0xFFFF
static bool first_af_status = false;


#ifdef CONFIG_FLASH_MIC2871
#define camdrv_ss_s5k4ecgx_flash_control(a,b)  camdrv_ss_s5k4ecgx_MIC2871_flash_control(a,b)
#else
#ifdef CONFIG_FLASH_DS8515
#define camdrv_ss_s5k4ecgx_flash_control(a,b)  camdrv_ss_s5k4ecgx_DS8515_flash_control(a,b)
#else
#define camdrv_ss_s5k4ecgx_flash_control(a,b)  camdrv_ss_s5k4ecgx_AAT_flash_control(a,b)
#endif
#endif

/***********************************************************/
/* H/W configuration - Start                               */
/***********************************************************/

#define EXIF_SOFTWARE		""
#define EXIF_MAKE		"SAMSUNG"

#define SENSOR_0_CLK			"dig_ch0_clk"
#define SENSOR_0_CLK_FREQ		(26000000)

#define CSI0_LP_FREQ			(100000000)
#define CSI1_LP_FREQ			(100000000)
static struct regulator *VCAM_A_2_8_V;
static struct regulator *VCAM_IO_1_8_V;
static struct regulator *VCAM_CORE_1_2_V;
static struct regulator *VCAM_AF_2_8V;

/* individual configuration */
#if defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV01) \
	|| defined(CONFIG_MACH_JAVA_SS_EVAL)
#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_CORE_1_2V_REGULATOR	"vsrldo"
#define VCAM_AF_2_8V_REGULATOR		"mmcldo2"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1786000
#define VCAM_CORE_1_2V_REGULATOR_uV	1200000
#define VCAM_AF_2_8V_REGULATOR_uV	2800000
#define CAM0_RESET			111
#define CAM0_STNBY			2
#define GPIO_CAM_FLASH_SET		34
#define GPIO_CAM_FLASH_EN		33
#define EXIF_MODEL			"GT-B7272"

#elif defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV01)\
	|| defined(CONFIG_MACH_HAWAII_SS_CODINAN)

#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_AF_2_8V_REGULATOR		"mmcldo2"
#define VCAM_CORE_1_2V_REGULATOR	"vsrldo"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1800000
#define VCAM_CORE_1_2V_REGULATOR_uV	1200000
#define VCAM_AF_2_8V_REGULATOR_uV	2800000
#define CAM0_RESET			111
#define CAM0_STNBY			2
#define GPIO_CAM_FLASH_SET		34
#define GPIO_CAM_FLASH_EN		11
#define EXIF_MODEL			"GT-B7272"

#elif defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV00)

#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"tcxldo1"
#define VCAM_AF_2_8V_REGULATOR		"mmcldo2"
#define VCAM_CORE_1_2V_REGULATOR	"vsrldo"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1800000
#define VCAM_CORE_1_2V_REGULATOR_uV	1200000
#define VCAM_AF_2_8V_REGULATOR_uV	2800000
#define CAM0_RESET			111
#define CAM0_STNBY			2
#define GPIO_CAM_FLASH_SET		34
#define GPIO_CAM_FLASH_EN		11
#define EXIF_MODEL			"GT-B7272"

#else /* NEED TO REDEFINE FOR NEW VARIANT */

#define VCAM_A_2_8V_REGULATOR		"mmcldo1"
#define VCAM_IO_1_8V_REGULATOR		"lvldo1"
#define VCAM_AF_2_8V_REGULATOR		"mmcldo2"
#define VCAM_CORE_1_2V_REGULATOR	"vsrldo"
#define VCAM_A_2_8V_REGULATOR_uV	2800000
#define VCAM_IO_1_8V_REGULATOR_uV	1800000
#define VCAM_CORE_1_2V_REGULATOR_uV	1200000
#define VCAM_AF_2_8V_REGULATOR_uV	2800000
#define CAM0_RESET			111
#define CAM0_STNBY			2
#define GPIO_CAM_FLASH_SET		34
#define GPIO_CAM_FLASH_EN		11
#define EXIF_MODEL			"GT-B7272"

#endif

/***********************************************************/
/* H/W configuration - End                                 */
/***********************************************************/


#define AE_STABLE          1 //nikhil
#define AE_UNSTABLE    0  //nikhil
extern int flash_check; 
static DEFINE_MUTEX(af_cancel_op);
//extern inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd);


extern  int camdrv_ss_i2c_set_config_register(struct i2c_client *client, 
                                         regs_t reg_buffer[], 
          				                 int num_of_regs, 
          				                 char *name);

extern int camdrv_ss_set_preview_size(struct v4l2_subdev *sd);
extern int camdrv_ss_set_capture_size(struct v4l2_subdev *sd);
extern int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff);
extern struct camdrv_ss_state *to_state(struct v4l2_subdev *sd);
int wb_auto=1,iso_auto=1;
//#define __JPEG_CAPTURE__ 1        

static const struct camdrv_ss_framesize s5k4ecgx_supported_preview_framesize_list[] = {
/*	{ PREVIEW_SIZE_QCIF,	176,  144 },
	{ PREVIEW_SIZE_QVGA,	320,  240 },
	{ PREVIEW_SIZE_CIF,	352,  288 },*/
	{ PREVIEW_SIZE_VGA,	640,  480 },
	//{ PREVIEW_SIZE_D1,	720,  480 },
	{ PREVIEW_SIZE_XGA,	1024, 768 },
	{ PREVIEW_SIZE_1MP,	1280, 960 },
#if 0
	{ PREVIEW_SIZE_1MP,	1280, 1024 },
	{ PREVIEW_SIZE_W1MP,	1600,  960 },
	{ PREVIEW_SIZE_2MP,	1600, 1200 },
	{ PREVIEW_SIZE_3MP,	2048, 1536 },
#endif
	{ PREVIEW_SIZE_5MP,	2560, 1920 },
};


static const struct camdrv_ss_framesize  s5k4ecgx_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA,		640,  480 },
//	{ CAPTURE_SIZE_1MP,		1280, 960 },
//	{ CAPTURE_SIZE_2MP,		1600, 1200 },
	{ CAPTURE_SIZE_3MP,		2048, 1536 },
	{ CAPTURE_SIZE_5MP,		2560, 1920 },
};
const static struct v4l2_fmtdesc s5k4ecgx_fmts[] = 
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

static const struct v4l2_queryctrl s5k4ecgx_controls[] = {
	{
		.id		= V4L2_CID_CAMERA_FLASH_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash",
		.minimum	= FLASH_MODE_OFF,
		.maximum	= (1 << FLASH_MODE_OFF | 1 << FLASH_MODE_AUTO | 1 << FLASH_MODE_ON
					| 1 << FLASH_MODE_TORCH_ON),
		.step		= 1,
		.default_value	= FLASH_MODE_OFF,




		/*
		.id			= V4L2_CID_CAMERA_FLASH_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash",
		.minimum	= FLASH_MODE_OFF,
		.maximum	= (1 << FLASH_MODE_OFF),
		.step		= 1,
		.default_value	= FLASH_MODE_OFF,*/
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
		.minimum	= FOCUS_MODE_AUTO,
		.maximum	= (1 << FOCUS_MODE_AUTO | 1 << FOCUS_MODE_MACRO
						| 1 << FOCUS_MODE_INFINITY), /* querymenu ?*/
		.step		= 1,
		.default_value	= FOCUS_MODE_AUTO,
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

	{
		.id		= V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "AEAWUNLOCK",
		.minimum	= AE_UNLOCK_AWB_UNLOCK,
		.maximum	= (1 << AE_UNLOCK_AWB_UNLOCK | 1 <<AE_LOCK_AWB_LOCK ),
		.step		= 1,
		.default_value	= AE_UNLOCK_AWB_UNLOCK,

	},
		
	{
		.id 		= V4L2_CID_CAMERA_TOUCH_AF_AREA,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Touchfocus areas",
		.minimum	= 0,
		.maximum	= 1,
		.step		= 1,
		.default_value	= 1,
	},
	{
		.id			= V4L2_CID_CAMERA_FRAME_RATE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Framerate control",
		.minimum	= FRAME_RATE_AUTO,
		.maximum	= (1 << FRAME_RATE_AUTO /*| 1 << FRAME_RATE_5 | 1 << FRAME_RATE_7 */|1 << FRAME_RATE_10  | 1 << FRAME_RATE_15
						/*| 1 << FRAME_RATE_20*/ | 1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),		
		.step		= 1,
		.default_value	= FRAME_RATE_AUTO,
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

static int camdrv_ss_s5k4ecgx_enum_frameintervals(struct v4l2_subdev *sd,struct v4l2_frmivalenum *fival)
{
	int err = 0;
	int size,i;

	if (fival->index >= 1)
	{
		CAM_ERROR_PRINTK("%s: returning ERROR because index =%d !!!  \n",__func__,fival->index);
		return -EINVAL;
	}
	

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	for(i = 0; i < ARRAY_SIZE(s5k4ecgx_supported_preview_framesize_list); i++) {
		if((s5k4ecgx_supported_preview_framesize_list[i].width == fival->width) &&
		    (s5k4ecgx_supported_preview_framesize_list[i].height == fival->height))
		{
			size = s5k4ecgx_supported_preview_framesize_list[i].index;
			break;
		}
	}

	if(i == ARRAY_SIZE(s5k4ecgx_supported_preview_framesize_list))
	{
		CAM_ERROR_PRINTK("%s unsupported width = %d and height = %d\n",
			__func__, fival->width, fival->height);
		return -EINVAL;
	}

	switch (size) {
	case PREVIEW_SIZE_5MP:
	case PREVIEW_SIZE_3MP:
	case PREVIEW_SIZE_2MP:
	case PREVIEW_SIZE_W1MP:
		fival->discrete.numerator = 2;
		fival->discrete.denominator = 15;
		CAM_INFO_PRINTK("%s: w = %d, h = %d, fps = 7.5\n",
			__func__, fival->width, fival->height);
		break;
	default:
		fival->discrete.numerator = 1;
		fival->discrete.denominator = 30;
		CAM_INFO_PRINTK("%s: w = %d, h = %d, fps = 30\n",
			__func__, fival->width, fival->height);
		break;
	}

	return err;
}


static long camdrv_ss_s5k4ecgx_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
       p->focal_length.numerator = 354;
       p->focal_length.denominator = 100;
			break;	
		}
		default:
			ret = -ENOIOCTLCMD;
			break;
	}

	return ret;
}


int camdrv_ss_s5k4ecgx_set_preview_start(struct v4l2_subdev *sd)
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

	    CAM_INFO_PRINTK("%s , ae unlock\n ",__func__);
            err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_unlock_regs, ARRAY_SIZE(s5k4ecgx_ae_unlock_regs), "ae_unlock_regs");
	    //CAM_INFO_PRINTK("%s , awb unlock\n ",__func__);
	    if (state->currentWB== WHITE_BALANCE_AUTO)
	    	{
	    		CAM_INFO_PRINTK("%s , awb unlock\n ",__func__);
  			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_unlock_regs, ARRAY_SIZE(s5k4ecgx_awb_unlock_regs), "awb_unlock_regs");
	    	}
            //err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_unlock_regs, ARRAY_SIZE(s5k4ecgx_awb_unlock_regs), "awb_unlock_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :s5k4ecgx_ae_unlock_regs IS FAILED\n",__func__);
			return -EIO;
		}
		
		err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_preview_camera_regs, ARRAY_SIZE(s5k4ecgx_preview_camera_regs), "preview_camera_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :s5k4ecgx_preview_camera_regs IS FAILED\n",__func__);
			return -EIO;
		}

		if (state->currentScene == SCENE_MODE_NIGHTSHOT) 
			msleep(400);
		else if (state->currentScene == SCENE_MODE_FIREWORKS)
			msleep(800);
		
	}

	if(state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW)
	{
		/* CSP614861: temporary added fixed FPS for 720p performance test */
		if (state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW || state->mode_switch == INIT_DONE_TO_CAMCORDER_PREVIEW) {
						
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_HD_Camcorder_regs, ARRAY_SIZE(s5k4ecgx_HD_Camcorder_regs), "HD_Camcorder_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK("%s : I2C HD_Camcorder_regs IS FAILED\n", __func__);
				return -EIO;
			}
						
			/* Fixed FPS */
			if (ARRAY_SIZE(s5k4ecgx_fps_30_regs) != NULL)
				err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_fps_30_regs, ARRAY_SIZE(s5k4ecgx_fps_30_regs), "fps_30_regs");
			else if (ARRAY_SIZE(s5k4ecgx_fps_25_regs) != NULL)
				err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_fps_25_regs, ARRAY_SIZE(s5k4ecgx_fps_25_regs), "fps_25_regs");
			else if (ARRAY_SIZE(s5k4ecgx_fps_20_regs) != NULL)
				err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_fps_20_regs, ARRAY_SIZE(s5k4ecgx_fps_20_regs), "fps_20_regs");
			else if (ARRAY_SIZE(s5k4ecgx_fps_15_regs) != NULL)
				err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_fps_15_regs, ARRAY_SIZE(s5k4ecgx_fps_15_regs), "fps_15_regs");
			else
				CAM_ERROR_PRINTK("%s : Fixed FPS setting is not supported for 30,25,20,15 fps !!\n", __func__);

			if (err < 0) {
				CAM_ERROR_PRINTK("%s : Fixed FPS setting is FAILED !!\n", __func__);
				return -EIO;
			}
		}

		if (state->currentScene != SCENE_MODE_NONE) {
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_none_regs, ARRAY_SIZE(s5k4ecgx_scene_none_regs), "scene_none_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK( "%s :s5k4ecgx_scene_none_regs IS FAILED\n",__func__);
				return -EIO;
			}
		}
	}
	else if(state->mode_switch == INIT_DONE_TO_CAMCORDER_PREVIEW)
	{
	}
	else if(state->mode_switch == CAMCORDER_PREVIEW_TO_CAMERA_PREVIEW)
	{
		
		err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_HD_Camcorder_Disable_regs, ARRAY_SIZE(s5k4ecgx_HD_Camcorder_Disable_regs), "HD_Camcorder_Disable_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK("%s : I2C HD_Camcorder_Disable_regs IS FAILED\n", __func__);
			return -EIO;
		}
		
		if (ARRAY_SIZE(s5k4ecgx_fps_auto_regs) != NULL)
		{
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_fps_auto_regs, ARRAY_SIZE(s5k4ecgx_fps_auto_regs), "fps_auto_regs");			
		}
		
		err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_preview_camera_regs, ARRAY_SIZE(s5k4ecgx_preview_camera_regs), "preview_camera_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK( "%s :s5k4ecgx_preview_camera_regs IS FAILED\n",__func__);
			return -EIO;
		}
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


static int camdrv_ss_s5k4ecgx_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;
	int light_state = CAM_NORMAL_LIGHT;

	CAM_INFO_PRINTK( "%s Entered\n", __func__);

	/* Set image size */
	err = camdrv_ss_set_capture_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK( "%s: camdrv_ss_set_capture_size not supported !!!\n", __func__);
		return -EIO;
	}

	if (state->camera_af_flash_checked == 0) {
		if (state->current_flash_mode == FLASH_MODE_ON) {
			state->camera_flash_fire = 1;
			CAM_INFO_PRINTK("%s : Flash mode is ON\n", __func__);
		} else if (state->current_flash_mode == FLASH_MODE_AUTO) {
			if (camdrv_ss_s5k4ecgx_check_flash_needed(sd)) {
				state->camera_flash_fire = 1;
				CAM_INFO_PRINTK(
					"%s: Auto Flash : flash needed\n",
					__func__);
			} else
				CAM_INFO_PRINTK(
					"%s: Auto Flash : flash not needed\n",
					__func__);
		}
   else if (state->current_flash_mode == FLASH_MODE_OFF) {
    state->camera_flash_fire = 0;
			CAM_INFO_PRINTK("%s : Flash mode is OFF\n", __func__);
   }
	}

	/* Set Flash registers */
	if (state->camera_flash_fire) {
		err = camdrv_ss_i2c_set_config_register(
				client, s5k4ecgx_Main_Flash_Start_EVT1,
				ARRAY_SIZE(s5k4ecgx_Main_Flash_Start_EVT1),
				"Main_Flash_Start_EVT1");
		if (err < 0)
			CAM_ERROR_PRINTK(
				"[%s : %d] ERROR! Couldn't Set s5k4ecgx_Main_Flash_Start_EVT1\n",
				__FILE__, __LINE__);

     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_MAX_LEVEL);
	}


	/* Set Snapshot registers */

	err = camdrv_ss_i2c_set_config_register(client,
			s5k4ecgx_snapshot_normal_regs,
			ARRAY_SIZE(s5k4ecgx_snapshot_normal_regs),
			"snapshot_normal_regs");

	if (err < 0) {
		CAM_ERROR_PRINTK(
		"%s : s5k4ecgx_snapshot_normal_regs i2c failed !\n",
		__func__);
		return -EIO;
	}

	return 0;
}


#if 1
int camdrv_ss_s5k4ecgx_set_iso(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_PRINTK( "%s :  value =%d wb_auto=%d, iso_auto=%d\n" , __func__, mode,wb_auto,iso_auto);

	switch (mode) {
	case ISO_AUTO:
	{
		iso_auto=1;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_iso_auto_regs), "iso_auto_regs");
		
		
		break;
	}
	case ISO_50:
	{
		iso_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_iso_50_regs, ARRAY_SIZE(s5k4ecgx_iso_50_regs), "iso_50_regs");
	
		
		break;
	}

	case ISO_100:
	{
		iso_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_iso_100_regs, ARRAY_SIZE(s5k4ecgx_iso_100_regs), "iso_100_regs");
	
		
		break;
	}
	case ISO_200:
	{
		iso_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_iso_200_regs, ARRAY_SIZE(s5k4ecgx_iso_200_regs), "iso_200_regs");
	
		
		break;
	}
	case ISO_400:
	{
		iso_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_iso_400_regs, ARRAY_SIZE(s5k4ecgx_iso_400_regs), "iso_400_regs");
		
		
		break;
	}


	default:
	{
		CAM_ERROR_PRINTK( "%s  : default case supported !!!\n" , __func__);
			break;
        }			
	} /* end of switch */

	return err;
}
int camdrv_ss_s5k4ecgx_set_white_balance(struct v4l2_subdev *sd, int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( "%s :  value =%d wb_auto=%d iso_auto=%d\n" , __func__, mode,wb_auto,iso_auto);

	switch (mode) {
	case WHITE_BALANCE_AUTO:
	{
		wb_auto=1;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_regs), "wb_auto_regs");

		
		break;
	}

	case WHITE_BALANCE_CLOUDY:
	{
		wb_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_cloudy_regs, ARRAY_SIZE(s5k4ecgx_wb_cloudy_regs), "wb_cloudy_regs");

		
		break;
	}


	case WHITE_BALANCE_FLUORESCENT:
	{
		wb_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_fluorescent_regs, ARRAY_SIZE(s5k4ecgx_wb_fluorescent_regs), "wb_fluorescent_regs");

		
		break;
	}

	case WHITE_BALANCE_CWF:
	{
		wb_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		/*if (s5k4ecgx_wb_cwf_regs == 0)
			CAM_ERROR_PRINTK( "%s  : wb_cwf_regs not supported !!!\n" , __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_cwf_regs, ARRAY_SIZE(s5k4ecgx_wb_cwf_regs), "wb_cwf_regs");
		*/
		
		break;
	}
	case WHITE_BALANCE_DAYLIGHT:
	{
		wb_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_daylight_regs, ARRAY_SIZE(s5k4ecgx_wb_daylight_regs), "wb_daylight_regs");

		
		break;
	}
	case WHITE_BALANCE_INCANDESCENT:
	{
		wb_auto=0;
		if(wb_auto==1 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_iso_auto_regs), "wb_iso_auto_regs");
		}else if(wb_auto==0 && iso_auto==1){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_auto_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_auto_regs), "wb_manual_iso_auto_regs");
		}else if(wb_auto==1 &&iso_auto==0){
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_auto_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_auto_iso_manual_regs), "wb_auto_iso_manual_regs");
		}else{
			
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_manual_iso_manual_regs, ARRAY_SIZE(s5k4ecgx_wb_manual_iso_manual_regs), "wb_manual_iso_manual_regs");
		}
		
			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_wb_incandescent_regs, ARRAY_SIZE(s5k4ecgx_wb_incandescent_regs), "wb_incandescent_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK( " %s : default not supported !!!\n" , __func__);
		break;
	}
	}

	state->currentWB = mode;

	return err;
}
#endif
//denis
static int camdrv_ss_s5k4ecgx_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK( "%s : value =%d\n", __func__, ctrl->value);
#if 1
	if (state->current_mode == PICTURE_MODE){
		return 0;
	}
#endif	

    	err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_none_regs, ARRAY_SIZE(s5k4ecgx_scene_none_regs), "scene_none_regs");
	switch (ctrl->value) {
        /*
    	case SCENE_MODE_NONE:
    	{
    		err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_none_regs, ARRAY_SIZE(s5k4ecgx_scene_none_regs), "scene_none_regs");
    		break;
    	}
        */
    	case SCENE_MODE_PORTRAIT:
    	{
    		/* Metering-Center, EV0, WB-Auto, Sharp-1, Sat0, AF-Auto will be set in HAL layer */

   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_portrait_regs, ARRAY_SIZE(s5k4ecgx_scene_portrait_regs), "scene_portrait_regs");
    		break;
    	}

    	case SCENE_MODE_NIGHTSHOT:
    	{
			CAM_ERROR_PRINTK("night mode normal\n");
    		err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_nightshot_regs, ARRAY_SIZE(s5k4ecgx_scene_nightshot_regs), "scene_nightshot_regs");
    		break;
    	}

    	case SCENE_MODE_BACK_LIGHT:
    	{
    		/* Metering-Spot, EV0, WB-Auto, Sharp0, Sat0, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_backlight_regs, ARRAY_SIZE(s5k4ecgx_scene_backlight_regs), "scene_backlight_regs");
    		break;
    	}

    	case SCENE_MODE_LANDSCAPE:
    	{
    		/* Metering-Matrix, EV0, WB-Auto, Sharp+1, Sat+1, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_landscape_regs, ARRAY_SIZE(s5k4ecgx_scene_landscape_regs), "scene_landscape_regs");
    		break;
    	}

    	case SCENE_MODE_SPORTS:
    	{
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_sports_regs, ARRAY_SIZE(s5k4ecgx_scene_sports_regs), "scene_sports_regs");
    		break;
    	}

    	case SCENE_MODE_PARTY_INDOOR:
    	{
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_party_indoor_regs, ARRAY_SIZE(s5k4ecgx_scene_party_indoor_regs), "scene_party_indoor_regs");
    		break;
    	}

    	case SCENE_MODE_BEACH_SNOW:
    	{
    		/* Metering-Center, EV+1, WB-Auto, Sharp0, Sat+1, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_beach_snow_regs, ARRAY_SIZE(s5k4ecgx_scene_beach_snow_regs), "scene_beach_snow_regs");
    		break;
    	}

    	case SCENE_MODE_SUNSET:
    	{
    		/* Metering-Center, EV0, WB-daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_sunset_regs, ARRAY_SIZE(s5k4ecgx_scene_sunset_regs), "scene_sunset_regs");
    		break;
    	}

    	case SCENE_MODE_DUSK_DAWN:
    	{
    		/* Metering-Center, EV0, WB-fluorescent, Sharp0, Sat0, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_duskdawn_regs, ARRAY_SIZE(s5k4ecgx_scene_duskdawn_regs), "scene_duskdawn_regs");
    		break;
    	}

    	case SCENE_MODE_FALL_COLOR:
    	{
    		/* Metering-Center, EV0, WB-Auto, Sharp0, Sat+2, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_fall_color_regs, ARRAY_SIZE(s5k4ecgx_scene_fall_color_regs), "scene_fall_color_regs");
    		break;
    	}

    	case SCENE_MODE_FIREWORKS:
    	{
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_fireworks_regs, ARRAY_SIZE(s5k4ecgx_scene_fireworks_regs), "scene_fireworks_regs");
    		break;
    	}

    	case SCENE_MODE_TEXT:
    	{
    		/* Metering-Center, EV0, WB-Auto, Sharp+2, Sat0, AF-Macro will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_text_regs, ARRAY_SIZE(s5k4ecgx_scene_text_regs), "scene_text_regs");
    		break;
    	}

    	case SCENE_MODE_CANDLE_LIGHT:
    	{
    		/* Metering-Center, EV0, WB-Daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
   			err = camdrv_ss_i2c_set_config_register(client, s5k4ecgx_scene_candle_light_regs, ARRAY_SIZE(s5k4ecgx_scene_candle_light_regs), "scene_candle_light_regs");
    		break;
    	}

    	default:
    	{
    		CAM_ERROR_PRINTK( "%s default not supported !!!\n", __func__);
    		err = -EINVAL;
    		break;
    	}
	}

	state->currentScene = ctrl->value;

	return err;
}


static unsigned char pBurstData[MAX_BUFFER];

static int camdrv_ss_s5k4ecgx_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_t reg_buffer[],int num_of_regs,char *name)
{
	struct i2c_msg msg = {client->addr, 0, 0, 0};	
    unsigned short subaddr=0, data_value=0;
	
	int next_subaddr;
    int i;
	int index = 0;
	int err = 0;
#if 1
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

//				CAM_ERROR_PRINTK("%s : added sleep for  = %d msec in %s !PLEASE CHECK !!! \n", __func__,data_value,name);
                msleep(data_value);
                break;
            }

            case 0xFCFC:
            case 0x0028:
            case 0x002A:
            default:
            {
				      err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value);
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


    return 0;
}

#define DS_PULS_HI_TIME    2
#define DS_PULS_LO_TIME    2
#define DS_LATCH_TIME      500

static void camdrv_ss_DS8515_flash_write_data(unsigned char count)
{CAM_INFO_PRINTK( "%s %d :camdrv_ss_DS8515_flash_write_data  E\n", __func__, count);
    unsigned long flags;

    local_irq_save(flags);

    {
        if(count)
        {
            do 
            {
           
				 gpio_set_value(GPIO_CAM_FLASH_SET, 0); //low till T_off
                udelay(DS_PULS_LO_TIME);

				 gpio_set_value(GPIO_CAM_FLASH_SET, 1); //go high
				 udelay(DS_PULS_HI_TIME);
            } while (--count);

            udelay(DS_LATCH_TIME);
        }
    }
    
    local_irq_restore(flags);
}



#define MIC_PULS_HI_TIME	1
#define MIC_PULS_LO_TIME	1
#define MIC_LATCH_TIME		150
#define MIC_END_TIME		500

/* MIC2871 flash control driver.*/
static void camdrv_ss_MIC2871_flash_write_data(char addr, char data)
{
	CAM_INFO_PRINTK("%s %d %d:  E\n", __func__, addr, data);
	int i;
	/* send address */
	printk(KERN_ALERT "%s addr(%d) data(%d)\n", __func__, addr, data);
	for (i = 0; i < (addr + 1); i++) {
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		udelay(1);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
		udelay(1);
	}
	/* wait T lat */
    udelay(97);
	/* send data */
	for (i = 0; i < (data + 1); i++) {
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		udelay(1);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
		udelay(1);
	}
	/* wait T end */
    udelay(405);
}


#define AAT_PULS_HI_TIME    1
#define AAT_PULS_LO_TIME    1
#define AAT_LATCH_TIME      500

// AAT1271 flash control driver.
static void camdrv_ss_AAT_flash_write_data(unsigned char count)
{CAM_INFO_PRINTK( "%s %d :camdrv_ss_s5k4ecgx_AAT_flash_write_data  E\n", __func__, count);
    unsigned long flags;

    local_irq_save(flags);
/*
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
    else*/
    {
        if(count)
        {
            do 
            {
				/*
				gpio_set_value(GPIO_CAM_FLASH_SET, 0);
				udelay(AAT_PULS_LO_TIME);

				gpio_set_value(GPIO_CAM_FLASH_SET, 1);
				udelay(AAT_PULS_HI_TIME);
				*/
				gpio_set_value(GPIO_CAM_FLASH_SET, 0); /*low till T_off */
				udelay(AAT_PULS_LO_TIME);

				gpio_set_value(GPIO_CAM_FLASH_SET, 1); /* go high */
				udelay(AAT_PULS_LO_TIME);
			} while (--count);

			udelay(AAT_LATCH_TIME);
		}
    }
    
    local_irq_restore(flags);
}


static float camdrv_ss_s5k4ecgx_get_exposureTime(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0, read_value2 = 0;
    int exposureTime = 0;

    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2BC0);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);   // LSB (0x2A14)
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value2);   // MSB (0x2A16)

    exposureTime = (int)read_value2;
    exposureTime = (exposureTime << 16) | (read_value1 & 0xFFFF);
//    return ((exposureTime * 1000) / 400); // us
    return ((1000 * 400)/exposureTime ); // us

 

}
//aska add
static int camdrv_ss_s5k4ecgx_get_nightmode(struct v4l2_subdev *sd)
{
	return 0;
};
static int camdrv_ss_s5k4ecgx_get_iso_speed_rate(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	
	unsigned short read_value1 = 0, read_value2 = 0;
	unsigned short  GainValue = 0;
	int isospeedrating = 100;
	int rows_num_=0;

	camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2BC4);
	camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);
	camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value2);

	CAM_INFO_PRINTK(" camdrv_ss_s5k4ecgx_get_iso_speed_rate read_value1=0x%x, read_value2=0x%x, iso_auto=%d \n", read_value1, read_value2 , iso_auto);
	if(iso_auto) {
		GainValue = ( (read_value1 * read_value2) / 256 )/2 ;
		CAM_INFO_PRINTK(" camdrv_ss_s5k4ecgx_get_iso_speed_rate ISO Auto GainValue=0x%x\n", GainValue );

		if(GainValue < 0xC0)
		{
			isospeedrating = 50;
		}
		else if(GainValue < 0x21F)
		{
			isospeedrating = 100;
		}
		else if(GainValue < 0x37F)
		{
			isospeedrating = 200;
		}
		else
		{
			isospeedrating = 400;
		}
	} else {
		GainValue = read_value1 ;
		CAM_INFO_PRINTK(" camdrv_ss_s5k4ecgx_get_iso_speed_rate ISO Manual GainValue=0x%x\n", GainValue );

		if(GainValue < 0x101)
		{
			isospeedrating = 50;
		}
		else if(GainValue < 0x23F)
		{
			isospeedrating = 100;
		}
		else if(GainValue < 0x37F)
		{
			isospeedrating = 200;
		}
		else
		{
			isospeedrating = 400;
		}
	}

	if (state->currentScene == SCENE_MODE_FIREWORKS)   //denis : set iso value for the specific scene mode
		isospeedrating = 50;
	else if (state->currentScene == SCENE_MODE_BEACH_SNOW)
		isospeedrating = 50;
	else if (state->currentScene == SCENE_MODE_PARTY_INDOOR)
		isospeedrating = 200;

	return isospeedrating;
}
/*NIKHIL*/
static  camdrv_ss_s5k4ecgx_set_ae_stable_status(struct v4l2_subdev *sd, unsigned short value)
{ 
 struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
	CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_get_ae_stable_status E \n");

    //Check AE stable
    err = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0588);
    err += camdrv_ss_i2c_write_2_bytes(client, 0x0F12, value);
    
    if(err < 0)
    {
        CAM_ERROR_PRINTK("[%s: %d] ERROR! AE stable check\n", __FILE__, __LINE__);
    }
	return err;
}

/*NIKHIL*/
#if 0
static int camdrv_ss_s5k4ecgx_get_ae_stable_status_value(struct v4l2_subdev *sd)
{
 struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    unsigned short AE_stable = 0x0000;
    int rows_num_=0;

    //Check AE stable
    err = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x2c74);
    err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &AE_stable);

    if(err < 0)
    {
        CAM_ERROR_PRINTK("[%s: %d] ERROR! AE stable check\n", __FILE__, __LINE__);
    }

   return AE_stable;  
}
#endif
static int camdrv_ss_s5k4ecgx_get_ae_stable_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
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
   struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    unsigned short AE_stable = 0x0000;
    int rows_num_=0;
  CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_get_ae_stable_status E \n");
    //Check AE stable
    err = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
    err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x2c74);
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

    return 0;
}        
// end 


static int camdrv_ss_s5k4ecgx_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{ 
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct camdrv_ss_state *state = to_state(sd);
    int err = 0;
    unsigned short usReadData =0 ;

    CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_get_auto_focus_status E \n");
	if(!first_af_status)
    {
        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_get_1st_af_search_status,ARRAY_SIZE(s5k4ecgx_get_1st_af_search_status),"get_1st_af_search_status");
        if(err < 0)
        {

     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);

            CAM_ERROR_PRINTK("%s: I2C failed during s5k4ecgx_get_1st_af_search_status \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			return -EFAULT;
        }
        err = camdrv_ss_i2c_read_2_bytes(client, 0x0F12,&usReadData);
        if(err < 0)
        {

     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
 
            CAM_ERROR_PRINTK("%s: I2C failed during AF \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			return -EFAULT;
        }
        
        CAM_INFO_PRINTK(" 1st check status, usReadData : 0x%x\n", usReadData );

        switch( usReadData & 0xFF )
    	{
            case 1:
                CAM_ERROR_PRINTK( "1st CAM_AF_STATUS_PROGRESS...\n " );
                ctrl->value =CAMERA_AF_STATUS_SEARCHING;
				return 0;
            case 2:
                CAM_INFO_PRINTK("1st CAM_AF_SUCCESS\n " );
                 ctrl->value = CAMERA_AF_STATUS_SEARCHING;
				 first_af_status = true;
            break;
            default:                    
                CAM_ERROR_PRINTK("1st CAM_AF_FAIL.\n ");
                ctrl->value = CAMERA_AF_STATUS_FAILED;
	        
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_unlock_regs, ARRAY_SIZE(s5k4ecgx_ae_unlock_regs), "ae_unlock_regs");
                 if (state->camera_flash_fire == 1)
	        {
            camdrv_ss_s5k4ecgx_set_ae_stable_status(sd,0x0002); 
           err = camdrv_ss_i2c_set_config_register(client,  s5k4ecgx_Pre_Flash_End_EVT1, ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
                                                   "Pre_Flash_End_EVT1");

		camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);

		//   state->camera_flash_fire = 0 ;
		//CAM_INFO_PRINTK("%d, camera af flash check",  state->camera_flash_fire);
	        }
//        	CAM_INFO_PRINTK("%s , ae unlock\n ",__func__);  //to maintain AE lock in case of af fail
//        	err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_unlock_regs, ARRAY_SIZE(s5k4ecgx_ae_unlock_regs), "ae_unlock_regs");
				return 0;
    	}
    }

    if (first_af_status)
    {
	err = camdrv_ss_i2c_set_config_register(client,
			s5k4ecgx_get_2nd_af_search_status,
			ARRAY_SIZE(s5k4ecgx_get_2nd_af_search_status),
			"get_2nd_af_search_status");
	if (err < 0) {
	        camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
		CAM_ERROR_PRINTK("%s: I2C failed during s5k4ecgx_get_2nd_af_search_status\n", __func__);
		ctrl->value = CAMERA_AF_STATUS_FAILED;
		first_af_status = false;
		return -EFAULT;
	}
	err = camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &usReadData);
        if(err < 0)
        {

	   camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
 	   CAM_ERROR_PRINTK("%s: I2C failed during AF \n", __func__);
	   ctrl->value = CAMERA_AF_STATUS_FAILED;
	   first_af_status = false;
	    return -EFAULT;
        }

        CAM_INFO_PRINTK(" 2nd check status, usReadData : 0x%x\n", usReadData );

        if (!(usReadData & 0xff00))
        {
            CAM_INFO_PRINTK("2nd CAM_AF_SUCCESS, \n ");
            ctrl->value = CAMERA_AF_STATUS_FOCUSED;
			first_af_status = false;

		if (state->camera_flash_fire == 1) {
			camdrv_ss_s5k4ecgx_set_ae_stable_status(sd, 0x0002);
			if (ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1) == NULL) {
			CAM_ERROR_PRINTK(" %s : pre flash start not supported!!!\n", __func__);
		}

		err = camdrv_ss_i2c_set_config_register(client,
			s5k4ecgx_Pre_Flash_End_EVT1,
			ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
			"Pre_Flash_End_EVT1");
		/* state->camera_flash_fire = 0; */
		if (err < 0) {
			CAM_ERROR_PRINTK("[%s : %d] ERROR! Couldn't Set Pre Flash start\n", __FILE__, __LINE__);
		}
           //Nikhil

 		    camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);

          }
         
	if (state->currentScene == SCENE_MODE_NIGHTSHOT) {
		msleep(200);
	}
            return 0;
        }
		else
		{
			CAM_ERROR_PRINTK("2nd CAM_AF_STATUS_PROGRESS.... \n ");
		    ctrl->value = CAMERA_AF_STATUS_SEARCHING;
		}

    }
    	
//    camdrv_ss_s5k4ecgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);	//lucky_preflash
    return 0;
}


static int camdrv_ss_s5k4ecgx_get_touch_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{


    struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
    int err = 0;
    unsigned short usReadData =0 ;

    CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_get_touch_focus_status E \n");
	if(!first_af_status)
    {
        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_get_1st_af_search_status,ARRAY_SIZE(s5k4ecgx_get_1st_af_search_status),"get_1st_af_search_status");
        if(err < 0)
        {
            CAM_ERROR_PRINTK("%s: I2C failed during s5k4ecgx_get_1st_af_search_status \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			return -EFAULT;
        }
        err = camdrv_ss_i2c_read_2_bytes(client, 0x0F12,&usReadData);
        if(err < 0)
        {
            CAM_ERROR_PRINTK("%s: I2C failed during AF \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			return -EFAULT;
        }
        
        CAM_INFO_PRINTK(" 1st check status, usReadData : 0x%x\n", usReadData );

        switch( usReadData & 0xFF )
    	{
            case 1:
                CAM_ERROR_PRINTK( "1st CAM_AF_STATUS_PROGRESS...\n " );
                ctrl->value =CAMERA_AF_STATUS_SEARCHING;
				return 0;
            case 2:
                CAM_INFO_PRINTK("1st CAM_AF_SUCCESS\n " );
                 ctrl->value = CAMERA_AF_STATUS_SEARCHING;
				 first_af_status = true;
            break;
            default:                    
                CAM_ERROR_PRINTK("1st CAM_AF_FAIL.\n ");
                ctrl->value = CAMERA_AF_STATUS_FAILED;
			
		if (state->camera_flash_fire == 1) {
			camdrv_ss_s5k4ecgx_set_ae_stable_status(sd, 0x0002);
			err = camdrv_ss_i2c_set_config_register(client,
				s5k4ecgx_Pre_Flash_End_EVT1,
				ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
				"Pre_Flash_End_EVT1");
		
      		    camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
		
		
		}
		return 0;
    	}
    }

    if (first_af_status)
    {
        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_get_2nd_af_search_status,ARRAY_SIZE(s5k4ecgx_get_2nd_af_search_status),"get_2nd_af_search_status");
		if(err < 0)
        {
            CAM_ERROR_PRINTK("%s: I2C failed during s5k4ecgx_get_2nd_af_search_status \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			first_af_status = false;
			return -EFAULT;
        }
        err = camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &usReadData);
        if(err < 0)
        {
            CAM_ERROR_PRINTK("%s: I2C failed during AF \n", __func__);
            ctrl->value = CAMERA_AF_STATUS_FAILED;
			first_af_status = false;
            return -EFAULT;
        }

        CAM_INFO_PRINTK(" 2nd check status, usReadData : 0x%x\n", usReadData );

		if (!(usReadData & 0xff00)) {
			CAM_INFO_PRINTK("2nd CAM_AF_SUCCESS\n");
			ctrl->value = CAMERA_AF_STATUS_FOCUSED;
			first_af_status = false;
			if ((state->camera_flash_fire == 1) &&
			(state->mode_switch !=
			CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW) &&
			(state->mode_switch !=
			INIT_DONE_TO_CAMCORDER_PREVIEW)) {
				camdrv_ss_s5k4ecgx_set_ae_stable_status(sd, 0x0002);
				err = camdrv_ss_i2c_set_config_register(client,
					s5k4ecgx_Pre_Flash_End_EVT1,
					ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
					"Pre_Flash_End_EVT1");

		     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
	
			}
			return 0;
		} else {
			CAM_ERROR_PRINTK("2nd CAM_AF_STATUS_PROGRESS....\n");
			ctrl->value = CAMERA_AF_STATUS_SEARCHING;
		}

    }
   	
    	
    return 0;
}


static int camdrv_ss_s5k4ecgx_set_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;
         int n=0;//Nikhil

	mutex_lock(&af_cancel_op);
	CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_set_auto_focus E \n");

	first_af_status = false;

	// Initialize fine search value.
	state->bStartFineSearch = false;

	if(ctrl->value == AUTO_FOCUS_ON) 	{



CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_set_auto_focus E  %d\n", state->camera_af_flash_checked);
//Nikhil

    //camdrv_ss_s5k4ecgx_set_ae_stable_status(sd,0x0000); 	


//lucky_preflash
//  if (state->camera_af_flash_checked == 0) 
  //{
//state->camera_flash_fire = 0;

   if (state->current_flash_mode == FLASH_MODE_ON) {
    state->camera_flash_fire = 1;
   } else if (state->current_flash_mode == FLASH_MODE_AUTO) {
    bool bflash_needed = false;
    if (camdrv_ss_s5k4ecgx_check_flash_needed(sd))
     bflash_needed = camdrv_ss_s5k4ecgx_check_flash_needed(sd);
    else
     CAM_ERROR_PRINTK( "%s : check_flash_needed is NULL !!!\n", __func__);

    if (bflash_needed) {
     state->camera_flash_fire = 1;
    }
   }
  //}

#if 0
  if (state->camera_flash_fire) {
   /* Set Snapshot registers */
    //Nikhil
   if ( s5k4ecgx_Pre_Flash_Start_EVT1 == 0) {
    CAM_ERROR_PRINTK( " %s : pre flash start not supported!!!\n",  __func__);
   }
   err = camdrv_ss_i2c_set_config_register(client,  s5k4ecgx_Pre_Flash_Start_EVT1, ARRAY_SIZE( s5k4ecgx_Pre_Flash_Start_EVT1),
       "Pre_Flash_Start_EVT1");
   if (err < 0) {
    CAM_ERROR_PRINTK( "[%s : %d] ERROR! Couldn't Set Pre Flash start \n", __FILE__, __LINE__);
   }
   //Nikhil 
    camdrv_ss_s5k4ecgx_AAT_flash_control(sd, FLASH_CONTROL_MIDDLE_LEVEL);
  }
  #endif
          #if 0
           msleep(200); 
         // while(1)
           {
            // if(camdrv_ss_s5k4ecgx_get_ae_stable_status_value(sd))  
             while(!(camdrv_ss_s5k4ecgx_get_ae_stable_status_value(sd)))
             msleep(200);
             //n++;
           };
		  #endif
           //Nikhil
// }//lucky_preflash







  //NIKHIL 
           /*
           msleep(200); 
           while(n<2)
           {
             if(camdrv_ss_s5k4ecgx_get_ae_stable_status_value(sd))  
             break;

             msleep(200);
             n++;
           };*/
           //Nikhil




		
		CAM_INFO_PRINTK("%s, AUTFOCUS ON\n", __func__ );


		CAM_INFO_PRINTK("%s , ae lock\n ",__func__);
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_lock_regs, ARRAY_SIZE(s5k4ecgx_ae_lock_regs), "ae_lock_regs");
		//CAM_INFO_PRINTK("%s , awb lock\n ",__func__);
		if (state->currentWB== WHITE_BALANCE_AUTO &&
		    state->currentScene != SCENE_MODE_SUNSET  &&
		state->currentScene != SCENE_MODE_DUSK_DAWN  &&
		state->currentScene != SCENE_MODE_CANDLE_LIGHT )
	       {
                 CAM_INFO_PRINTK("%s , awb lock\n ",__func__);
                 err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_lock_regs, ARRAY_SIZE(s5k4ecgx_awb_lock_regs), "awb_lock_regs");
	        }
		//err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_lock_regs, ARRAY_SIZE(s5k4ecgx_awb_lock_regs), "awb_lock_regs");
		CAM_INFO_PRINTK("%s , AF start\n ",__func__);
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_single_af_start_regs, ARRAY_SIZE(s5k4ecgx_single_af_start_regs),"single_af_start_regs");

		if(err < 0)
		{
			CAM_ERROR_PRINTK("%s : i2c failed !!! \n", __func__);
			mutex_unlock(&af_cancel_op);
			return -EIO;
		}
	} else if(ctrl->value == AUTO_FOCUS_OFF) {
		CAM_INFO_PRINTK("%s :  AUTFOCUS OFF , af_mode : %d\n", __func__, state->af_mode);
        /*
               if (state->camera_flash_fire) 
               {
                        camdrv_ss_s5k4ecgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
		state->camera_flash_fire = 0;
	       }
		state->camera_af_flash_checked = 0;
		*/
		if (state->af_mode == FOCUS_MODE_AUTO) {
			//err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs,ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs),"focus_mode_auto_regs");
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel3,ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel3),"focus_mode_auto_regs_cancel3");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			//err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs,ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs),"focus_mode_macro_regs");
                        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel3,ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel3),"focus_mode_macro_regs_cancel3");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		}

		CAM_INFO_PRINTK("%s , ae unlock\n ",__func__);
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_unlock_regs, ARRAY_SIZE(s5k4ecgx_ae_unlock_regs), "ae_unlock_regs");
//		CAM_INFO_PRINTK("%s , awb unlock\n ",__func__);
		if (state->currentWB== WHITE_BALANCE_AUTO)
		{
			CAM_INFO_PRINTK("%s , awb unlock\n ",__func__);
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_unlock_regs, ARRAY_SIZE(s5k4ecgx_awb_unlock_regs), "awb_unlock_regs");
		}
//		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_unlock_regs, ARRAY_SIZE(s5k4ecgx_awb_unlock_regs), "awb_unlock_regs");
		if (state->camera_flash_fire) {
			camdrv_ss_s5k4ecgx_set_ae_stable_status(sd, 0x0002);
			err = camdrv_ss_i2c_set_config_register(client,
				s5k4ecgx_Pre_Flash_End_EVT1,
				ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
				"Pre_Flash_End_EVT1");

		     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);

			state->camera_flash_fire = 0;
		}
	//	msleep(50);  // need to add more delay in case of night or fireworks mode : 300ms
	}
	else if(ctrl->value == AUTO_FOCUS_1ST_CANCEL)
	{
	             if (state->af_mode == FOCUS_MODE_AUTO) {
			
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel1, ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel1),"focus_mode_auto_regs_cancel1");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			
                        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel1,ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel1),"focus_mode_macro_regs_cancel1");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		}

		 
	 }
	    else if(ctrl->value == AUTO_FOCUS_2ND_CANCEL )
	   {
                        if (state->af_mode == FOCUS_MODE_AUTO) {
			
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel2, ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel2),"focus_mode_auto_regs_cancel2");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			
                        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel2, ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel2),"focus_mode_macro_regs_cancel2");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
	}

	 }
	mutex_unlock(&af_cancel_op);

	return 0;
}


static int camdrv_ss_s5k4ecgx_set_touch_focus_area(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	unsigned short FirstWinStartX, FirstWinStartY, SecondWinStartX, SecondWinStartY;
    unsigned short FirstWinEndX, FirstWinEndY, ConvFirstWinStartX, ConvFirstWinStartY;
    unsigned short SecondWinEndX, SecondWinEndY, ConvSecondWinStartX, ConvSecondWinStartY;
	unsigned short DefaultFirstWinStartX, DefaultFirstWinStartY, DefaultSecondWinStartX, DefaultSecondWinStartY;
	int preview_width, preview_height;
    struct v4l2_control ctrl = {0};
    int err = 0;

	first_af_status = false;

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
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_OUTER_WINDOW_WIDTH << 10) / preview_width  ); /* FstWinSizeX : 320 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_OUTER_WINDOW_HEIGHT << 10) / preview_height );  /* FstWinSizeY : 266 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultSecondWinStartX << 10) / preview_width)); /* ScndWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (unsigned short)((DefaultSecondWinStartY << 10) / preview_height)); /* ScndWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_INNER_WINDOW_WIDTH << 10) / preview_width  ); /* ScndWinSizeX : 143 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_INNER_WINDOW_HEIGHT << 10) / preview_height ); /* ScndWinSizeY : 143  */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* WinSizesUpdated */		
	}
	else if(touch_af == TOUCH_AF_START) { /* AF start */
        // Prevent divided-by-zero.
		if(preview_width == 0 || preview_height == 0) {
			CAM_ERROR_PRINTK( "%s: Either preview_width or preview_height is zero\n", __func__);
            return -EIO;
        }

        SecondWinStartX = touch_area->leftTopX - ( (AF_INNER_WINDOW_WIDTH - touch_area->rightBottomX) / 2 );
        /*if (SecondWinStartX < 0)
            SecondWinStartX = 0;*/
        if (SecondWinStartX+AF_INNER_WINDOW_WIDTH > preview_width)
            SecondWinStartX = preview_width-AF_INNER_WINDOW_WIDTH-1;

        SecondWinStartY = touch_area->leftTopY - ( (AF_INNER_WINDOW_HEIGHT- touch_area->rightBottomY) / 2 );
        /*if (SecondWinStartY < 0)
            SecondWinStartY = 0;*/
        if (SecondWinStartY+AF_INNER_WINDOW_HEIGHT > preview_height)
            SecondWinStartY = preview_height-AF_INNER_WINDOW_HEIGHT-1;

        FirstWinStartX = touch_area->leftTopX - ( (AF_OUTER_WINDOW_WIDTH - touch_area->rightBottomX) / 2 );
        /*if (FirstWinStartX < 0)
			FirstWinStartX = 0;*/
        if (FirstWinStartX+AF_OUTER_WINDOW_WIDTH > preview_width)
            FirstWinStartX = preview_width-AF_OUTER_WINDOW_WIDTH-1;
        
        FirstWinStartY = touch_area->leftTopY - ( (AF_OUTER_WINDOW_HEIGHT- touch_area->rightBottomY) / 2 );
        /*if (FirstWinStartY < 0)
			FirstWinStartY = 0;*/
        if (FirstWinStartY+AF_OUTER_WINDOW_HEIGHT > preview_height)
            FirstWinStartY = preview_height-AF_OUTER_WINDOW_HEIGHT-1;


		CAM_INFO_PRINTK("[%s:%d]leftTopX %d, leftTopY %d, rightBottomX %d, rightBottomY %d\n",__func__, __LINE__,  
                touch_area->leftTopX,touch_area->leftTopY, touch_area->rightBottomX, touch_area->rightBottomY);
		CAM_INFO_PRINTK("[%s:%d]SecondWinStartX %d, SecondWinStartY %d\n",__func__, __LINE__,  SecondWinStartX, SecondWinStartY);
		CAM_INFO_PRINTK("[%s:%d]FirstWinStartX %d, FirstWinStartY %d\n",__func__, __LINE__,  FirstWinStartX, FirstWinStartY);
            
            
		ConvFirstWinStartX = (unsigned short)((FirstWinStartX << 10) / preview_width);
		ConvFirstWinStartY = (unsigned short)((FirstWinStartY << 10) / preview_height);

		ConvSecondWinStartX = (unsigned short)((SecondWinStartX << 10) / preview_width);
		ConvSecondWinStartY = (unsigned short)((SecondWinStartY << 10) / preview_height);

		CAM_INFO_PRINTK("[%s:%d]ConvSecondWinStartX %d, ConvSecondWinStartY %d\n",__func__, __LINE__,  ConvSecondWinStartX, ConvSecondWinStartY);
		CAM_INFO_PRINTK("[%s:%d]ConvFirstWinStartX %d, ConvFirstWinStartY %d\n",__func__, __LINE__,  ConvFirstWinStartX, ConvFirstWinStartY);
			
		camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
		camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
		camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0294);
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvFirstWinStartX); /* FstWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvFirstWinStartY); /* FstWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_OUTER_WINDOW_WIDTH << 10) / preview_width  ); /* FstWinSizeX : 320 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_OUTER_WINDOW_HEIGHT << 10) / preview_height );  /* FstWinSizeY : 266 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvSecondWinStartX); /* ScndWinStartX */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, ConvSecondWinStartY); /* ScndWinStartY */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_INNER_WINDOW_WIDTH << 10) / preview_width  ); /* ScndWinSizeX : 72 */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, (AF_INNER_WINDOW_HEIGHT << 10) / preview_height ); /* ScndWinSizeY : 143  */
		camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001); /* WinSizesUpdated */
    }
	return err;
}


static int camdrv_ss_s5k4ecgx_set_touch_focus(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    int err = 0;

    mutex_lock(&af_cancel_op);
	CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_set_touch_focus E \n");

	first_af_status = false;

    // Initialize fine search value.
    state->bStartFineSearch = false;

	if(touch_af == TOUCH_AF_START) {
		CAM_INFO_PRINTK("%s, AUTFOCUS ON\n", __func__ );
	//	CAM_INFO_PRINTK("%s , ae lock\n ",__func__);
	//	err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_lock_regs, ARRAY_SIZE(s5k4ecgx_ae_lock_regs), "ae_lock_regs");
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_single_af_start_regs, ARRAY_SIZE(s5k4ecgx_single_af_start_regs),"single_af_start_regs");

		if(err < 0)
		{
			CAM_ERROR_PRINTK("%s : i2c failed !!! \n", __func__);
			mutex_unlock(&af_cancel_op);
			return -EIO;
		}
	} else if(touch_af == TOUCH_AF_STOP) {
		CAM_INFO_PRINTK("%s :  AUTFOCUS OFF , af_mode : %d\n", __func__, state->af_mode);
        
	//	state->camera_flash_fire = 0;
	//	state->camera_af_flash_checked = 0;
		
		if (state->af_mode == FOCUS_MODE_AUTO) {
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel3,ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel3),"focus_mode_auto_regs_cancel3");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel3,ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel3),"focus_mode_macro_regs_cancel3s");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		}
		if (state->camera_flash_fire) {
			camdrv_ss_s5k4ecgx_set_ae_stable_status(sd, 0x0002);
			err = camdrv_ss_i2c_set_config_register(client,
				s5k4ecgx_Pre_Flash_End_EVT1,
				ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1),
				"Pre_Flash_End_EVT1");

		     camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_OFF);
	
			state->camera_flash_fire = 0;
		}
		CAM_INFO_PRINTK("%s , ae unlock\n ",__func__);
		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_ae_unlock_regs, ARRAY_SIZE(s5k4ecgx_ae_unlock_regs), "ae_unlock_regs");
//		CAM_INFO_PRINTK("%s , awb unlock\n ",__func__);
//		err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_awb_unlock_regs, ARRAY_SIZE(s5k4ecgx_awb_unlock_regs), "awb_unlock_regs");
		
		//msleep(150);  // need to add more delay in case of night or fireworks mode : 300ms
	}
	else if(touch_af == AUTO_FOCUS_1ST_CANCEL)
	{
		if (state->af_mode == FOCUS_MODE_AUTO) {
			
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel1, ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel1),"focus_mode_auto_regs_cancel1");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			
                        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel1,ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel1),"focus_mode_macro_regs_cancel1");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		}

		
	 }
	    else if(touch_af == AUTO_FOCUS_2ND_CANCEL )
	   {
                        if (state->af_mode == FOCUS_MODE_AUTO) {
			
			err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_auto_regs_cancel2, ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel2),"focus_mode_auto_regs_cancel2");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		} else if (state->af_mode == FOCUS_MODE_MACRO) {
			
                        err = camdrv_ss_i2c_set_config_register(client,s5k4ecgx_focus_mode_macro_regs_cancel2, ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel2),"focus_mode_macro_regs_cancel2");
			if(err < 0)
			{
				CAM_ERROR_PRINTK( "%s : i2c failed in OFF !!! \n", __func__);
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}
		}

	}

	mutex_unlock(&af_cancel_op);	

	return err;
}



#if 0
static int camdrv_ss_s5k4ecgx_get_light_condition(struct v4l2_subdev *sd, int *Result)
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


static bool camdrv_ss_s5k4ecgx_check_flash_needed(struct v4l2_subdev *sd)
{
// lucky #if 0
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0;
    unsigned short read_value2 = 0;

    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2C18);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);	 //<7000.2C18h>
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value2);	 //<7000.2C1Ah>

/*    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x29F0);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);*/

    CAM_INFO_PRINTK( "%s [Read Value 1: %X]  [Read Value 2: %X]\n", __func__, read_value1,read_value2);
    
    if(read_value1 <= 0x003A && read_value2 == 0x0000 )
    {   
        return true;
    }
    else
    {
        return false;
    }
//lucky#endif

// return false;

}


static int camdrv_ss_s5k4ecgx_DS8515_flash_control(struct v4l2_subdev *sd, int control_mode)
{
	CAM_INFO_PRINTK("%s %d :  E\n", __func__, control_mode);
     switch(control_mode)
    {
        // USE FLASH MODE
        case FLASH_CONTROL_MAX_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);
            gpio_set_value(GPIO_CAM_FLASH_EN, 1);
            udelay(2);
            break;
        }
    
        // USE FLASH MODE
        case FLASH_CONTROL_HIGH_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);


            msleep(1);   // Flash Mode Set time

            camdrv_ss_DS8515_flash_write_data(2);
            gpio_set_value(GPIO_CAM_FLASH_SET, 1);
            break;
        }

        // USE MOVIE MODE : AF Pre-Flash Mode(Torch Mode)
        case FLASH_CONTROL_MIDDLE_LEVEL:
	{
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            camdrv_ss_DS8515_flash_write_data(5);
            gpio_set_value(GPIO_CAM_FLASH_SET, 1);
            break;
        }

        // USE MOVIE MODE : Movie Mode(Torch Mode)
        case FLASH_CONTROL_LOW_LEVEL:
        {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0); 
            udelay(1);

            camdrv_ss_DS8515_flash_write_data(7);   // 69mA
            gpio_set_value(GPIO_CAM_FLASH_SET, 1);
            break;
        }

        case FLASH_CONTROL_OFF:
        default:
        {
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            break;
        }        
    }

    return 0;

}

static int camdrv_ss_s5k4ecgx_MIC2871_flash_control(struct v4l2_subdev *sd, int control_mode)
{
CAM_INFO_PRINTK( "%s %d :  E\n",  __func__, control_mode);
	spinlock_t lock;
	spin_lock_init(&lock);
	unsigned long flags;

    switch(control_mode)
	{
        // USE FLASH MODE
	case FLASH_CONTROL_MAX_LEVEL:
		{	spin_lock_irqsave(&lock, flags);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
           		camdrv_ss_MIC2871_flash_write_data(5, 1);     /*Setting the safety timer threshold ST_TH(5) to 1(100mA)*/
		//	camdrv_ss_MIC2871_flash_write_data(3, 7); 	/* Setting the safety timer duration register STDUR(3) to 7 (1250msec)*/
           	//	camdrv_ss_MIC2871_flash_write_data(1, 0);     /* write 100%(0) to FEN/FCUR(1) */
			camdrv_ss_MIC2871_flash_write_data(4, 0); 	/* write disable(0) to low battery threshold LB_TH(4) */
		/* enable */
        	//	gpio_set_value(GPIO_CAM_FLASH_EN, 1);
        		camdrv_ss_MIC2871_flash_write_data(1, 16); 
		spin_unlock_irqrestore(&lock, flags);
		break;
	}
        // USE FLASH MODE
	case FLASH_CONTROL_HIGH_LEVEL:
		{	spin_lock_irqsave(&lock, flags);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
			camdrv_ss_MIC2871_flash_write_data(4, 0); 	/* write disable(0) to low battery threshold LB_TH(4) */
		camdrv_ss_MIC2871_flash_write_data(2, 21);
		spin_unlock_irqrestore(&lock, flags);
		break;
	}
        // USE MOVIE MODE : AF Pre-Flash Mode(Torch Mode)
	case FLASH_CONTROL_MIDDLE_LEVEL:
        	{	spin_lock_irqsave(&lock, flags);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
			camdrv_ss_MIC2871_flash_write_data(4, 0);	/* write disable(0) to low battery threshold LB_TH(4) */
		camdrv_ss_MIC2871_flash_write_data(2, 21);
		spin_unlock_irqrestore(&lock, flags);
		break;
	}

        // USE MOVIE MODE : Movie Mode(Torch Mode)
	case FLASH_CONTROL_LOW_LEVEL:
        	{	spin_lock_irqsave(&lock, flags);
		gpio_set_value(GPIO_CAM_FLASH_SET, 1);
			camdrv_ss_MIC2871_flash_write_data(4, 0); 	/* write disable(0) to low battery threshold LB_TH(4) */
		camdrv_ss_MIC2871_flash_write_data(2, 27);
		spin_unlock_irqrestore(&lock, flags);
		break;
	}

	case FLASH_CONTROL_OFF:
	default:
		{	gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
			udelay(500);
		break;
	}
	}
	return 0;
}

//lucky#if 0
static int camdrv_ss_s5k4ecgx_AAT_flash_control(struct v4l2_subdev *sd, int control_mode)
{
	CAM_INFO_PRINTK("%s %d :  E\n", __func__, control_mode);
     switch(control_mode)
    {
        // USE FLASH MODE
	case FLASH_CONTROL_MAX_LEVEL:
	{
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		udelay(1);
		gpio_set_value(GPIO_CAM_FLASH_EN, 1);
		break;
	}

        // USE FLASH MODE
	case FLASH_CONTROL_HIGH_LEVEL:
	{
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
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
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		udelay(1);

		camdrv_ss_AAT_flash_write_data(1);
		break;
	}

        // USE MOVIE MODE : Movie Mode(Torch Mode)
	case FLASH_CONTROL_LOW_LEVEL:
	{
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		udelay(1);

            camdrv_ss_AAT_flash_write_data(7);   // 69mA
		break;
	}

	case FLASH_CONTROL_OFF:
	default:
	{
		gpio_set_value(GPIO_CAM_FLASH_SET, 0);
		gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		break;
	}
	}

	return 0;
}


static int camdrv_ss_s5k4ecgx_sensor_power(int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *lp_clock;
	struct clk *axi_clk;

	CAM_INFO_PRINTK("%s:camera power %s\n", __func__, (on ? "on" : "off"));

	ret = -1;
	lp_clock = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

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
	
	VCAM_AF_2_8V = regulator_get(NULL, VCAM_AF_2_8V_REGULATOR);
	if(IS_ERR(VCAM_AF_2_8V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_AF_2_8V\n");
		return -1;
	}

	VCAM_CORE_1_2_V = regulator_get(NULL, VCAM_CORE_1_2V_REGULATOR);
	if(IS_ERR(VCAM_CORE_1_2_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_CORE_1_2_V\n");
		return -1;
	}

//	gpio_request(CAM_AF_EN, "cam_af_2_8v");
//	gpio_direction_output(CAM_AF_EN,0); 
	
	/* CAM_INFO_PRINTK("set cam_rst cam_stnby  to low\n"); */
	gpio_request(CAM0_RESET, "cam0_rst");
	gpio_direction_output(CAM0_RESET,0);
	
	gpio_request(CAM0_STNBY, "cam0_stnby");
	gpio_direction_output(CAM0_STNBY,0);
		
//	gpio_request(CAM1_RESET, "cam1_rst");
//	gpio_direction_output(CAM1_RESET,0);

//	gpio_request(CAM1_STNBY, "cam1_stnby");
//	gpio_direction_output(CAM1_STNBY,0);

//	value = ioread32(padctl_base + PADCTRLREG_DCLK1_OFFSET) & (~PADCTRLREG_DCLK1_PINSEL_DCLK1_MASK);
//		iowrite32(value, padctl_base + PADCTRLREG_DCLK1_OFFSET);


	if(on)
	{
		CAM_INFO_PRINTK("power on the sensor \n"); //@HW

		value = regulator_set_voltage(VCAM_A_2_8_V, VCAM_A_2_8V_REGULATOR_uV, VCAM_A_2_8V_REGULATOR_uV);
		if (value)
			CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_A_2_8_V failed \n", __func__);

		value = regulator_set_voltage(VCAM_IO_1_8_V, VCAM_IO_1_8V_REGULATOR_uV, VCAM_IO_1_8V_REGULATOR_uV);
		if (value)
			CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_IO_1_8_V failed \n", __func__);

		value = regulator_set_voltage(VCAM_AF_2_8V, VCAM_AF_2_8V_REGULATOR_uV, VCAM_AF_2_8V_REGULATOR_uV);
		if (value)
			CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_AF_2_8V failed \n", __func__);


		value = regulator_set_voltage(VCAM_CORE_1_2_V, VCAM_CORE_1_2V_REGULATOR_uV, VCAM_CORE_1_2V_REGULATOR_uV);
		if (value)
			CAM_ERROR_PRINTK("%s:regulator_set_voltage VCAM_CORE_1_2_V failed \n", __func__);


		if (mm_ccu_set_pll_select(CSI0_BYTE1_PLL, 8)) {
			pr_err("failed to set BYTE1\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI0_BYTE0_PLL, 8)) {
			pr_err("failed to set BYTE0\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI0_CAMPIX_PLL, 8)) {
			pr_err("failed to set PIXPLL\n");
			goto e_clk_pll;
		}

		value = clk_enable(axi_clk);
		if (value) {
			pr_err(KERN_ERR "Failed to enable axi clock\n");
			goto e_clk_axi;
		}

		value = clk_enable(lp_clock);
		if (value) {
			pr_err(KERN_ERR "Failed to enable lp clock\n");
			goto e_clk_lp;
		}

		value = clk_set_rate(lp_clock, CSI0_LP_FREQ);
		if (value) {
			pr_err("Failed to set lp clock\n");
			goto e_clk_set_lp;
		}

		value = regulator_enable(VCAM_A_2_8_V);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable VCAM_A_2_8_V\n", __func__);
			return -1;
		}


		value = regulator_enable(VCAM_IO_1_8_V);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable VCAM_IO_1_8_V\n", __func__);
			return -1;
		}

		value = regulator_enable(VCAM_AF_2_8V);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable VCAM_AF_2_8V \n", __func__);
			return -1;
		}
//		gpio_set_value(CAM_AF_EN,1);

		msleep(1);

          //     gpio_set_value(CAM1_STNBY,1);
		msleep(1);
	
		value = clk_enable(clock);
		if (value) {
			pr_err("Failed to enable sensor 0 clock\n");
			goto e_clk_sensor;
		}

		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			pr_err("Failed to set sensor0 clock\n");
			goto e_clk_set_sensor;
		}

		msleep(4);

	//	gpio_set_value(CAM1_RESET,1);
	//	msleep(1);

		
		value = regulator_enable(VCAM_CORE_1_2_V);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable VCAM_CORE_1_2_V \n", __func__);
			return -1;
		} else
			CAM_ERROR_PRINTK("%s: enabled VCAM_CORE_1_2V \n", __func__);
		msleep(1);

		gpio_set_value(CAM0_STNBY,1);
		msleep(1);

		gpio_set_value(CAM0_RESET,1);
	
		msleep(2);

// Flag is enabled and will be used if camera has flash enabled.		
//#ifdef CONFIG_FLASH_ENABLE
#if 0
				CAM_INFO_PRINTK( "5MP camera S5K4ECGX loaded. HWREV is 0x%x\n", HWREV);

		/*	// FLASH
		if(HWREV >= 0x03)
		{
			ret = gpio_request(GPIO_CAM_FLASH_SET_NEW, "GPJ0[5]");
			if(ret)
			{
				CAM_ERROR_PRINTK( "gpio_request(GPJ0[5]) failed!! \n");
				return 0;
			}
		}
		else*/
		{
			ret = gpio_request(GPIO_CAM_FLASH_SET, "GPJ0[2]");
			if(ret)
			{
				CAM_ERROR_PRINTK( "gpio_request(GPJ0[2]) failed!! \n");
						return 0;
			}
		}

		ret = gpio_request(GPIO_CAM_FLASH_EN, "GPJ0[7]");
		if(ret)
		{
			CAM_ERROR_PRINTK( "gpio_request(GPJ0[7]) failed!! \n");
					return 0;
		}

		/*if(HWREV >= 0x03)
		{
			gpio_direction_output(GPIO_CAM_FLASH_SET_NEW, 0);
		}
		else*/
		{
			gpio_direction_output(GPIO_CAM_FLASH_SET, 0);
		}
		gpio_direction_output(GPIO_CAM_FLASH_EN, 0);
#endif // CONFIG_FLASH_ENABLE
	}
	else
	{
		CAM_INFO_PRINTK("power off the sensor \n"); //@HW

		/* enable reset gpio */
		gpio_set_value(CAM0_RESET,0);
		msleep(1);
		
		clk_disable(clock);
		clk_disable(lp_clock);
		clk_disable(axi_clk);

		gpio_set_value(CAM0_STNBY,0);
		msleep(1);

	//	gpio_set_value(CAM1_RESET,0);
	//	msleep(1);
		
		/* enable power down gpio */

//		gpio_set_value(CAM_AF_EN,0); 
		regulator_disable(VCAM_AF_2_8V);
		msleep(1);
		regulator_disable(VCAM_CORE_1_2_V);

		regulator_disable(VCAM_IO_1_8_V);
		regulator_disable(VCAM_A_2_8_V);


//#ifdef CONFIG_FLASH_ENABLE
// Flag is enabled and will be used if camera has flash enabled.		
#if 0
			// FLASH
			// ?? need to do below?
			//camdrv_ss_AAT_flash_control(sd, FLASH_CONTROL_OFF);
			/*
			if(HWREV >= 0x03)
			{					 
				gpio_free(GPIO_CAM_FLASH_SET_NEW);
			}
			else*/
			{
				gpio_free(GPIO_CAM_FLASH_SET);
			}
			gpio_free(GPIO_CAM_FLASH_EN);
#endif // CONFIG_FLASH_ENABLE
/* Backporting Rhea to Hawaii: end */
         if (!flash_check)
		{
                gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            
                gpio_set_value(GPIO_CAM_FLASH_EN, 0);
		
	    }

		CAM_INFO_PRINTK("rhea_camera_power off success \n");
	}

	return 0;

e_clk_set_sensor:
	clk_disable(clock);
e_clk_sensor:
e_clk_set_lp:
	clk_disable(lp_clock);
e_clk_lp:
	clk_disable(axi_clk);
e_clk_axi:
e_clk_pll:
e_clk_get:
	return ret;
}


int camdrv_ss_s5k4ecgx_get_sensor_param_for_exif(
	struct v4l2_subdev *sd,
	struct v4l2_exif_sensor_info *exif_param)
{	char str[20],str1[20];
	int num = -1;
	int ret = -1;
	float exposureTime = 0.0f;
	struct camdrv_ss_state *state = to_state(sd);
	
	switch(state->current_flash_mode)
	{
		case FLASH_MODE_ON:
		snprintf(str1, 19, "%d", 9);
		break;
		
		case FLASH_MODE_OFF:
		snprintf(str1, 19, "%d", 0);//strcpy(str1,"OFF");
		break;

		case FLASH_MODE_AUTO:
		if(state->camera_flash_fire)
			
		snprintf(str1, 19, "%d", 25);//strcpy(str1,"AUTO");
		else 
		snprintf(str1, 19, "%d", 24);	
		break;
		
		default:
		strcpy(str1,"");
	}

	strcpy(exif_param->strSoftware,		EXIF_SOFTWARE);
	strcpy(exif_param->strMake,		EXIF_MAKE);
	strcpy(exif_param->strModel,		EXIF_MODEL);

	exposureTime = camdrv_ss_s5k4ecgx_get_exposureTime(sd);
	num = (int)exposureTime;
	if (num > 0) 
	{
		snprintf(str, 19, "1/%d", num);
		strcpy(exif_param->exposureTime, str);
	} 
	else 
	{
		strcpy(exif_param->exposureTime, "");
	}
	
	CAM_INFO_PRINTK("%s : exposure time =  %s \n",__func__,exif_param->exposureTime);

	num = camdrv_ss_s5k4ecgx_get_iso_speed_rate(sd);
	if (num > 0) {
		sprintf(str, "%d,", num);
		strcpy(exif_param->isoSpeedRating, str);
	} else {
		strcpy(exif_param->isoSpeedRating, "");
	}
	CAM_INFO_PRINTK("%s :num=%d and isoSpeedRating =  %s \n",__func__,num, exif_param->isoSpeedRating);

	/* sRGB mandatory field! */
	strcpy(exif_param->colorSpaceInfo,	"1");

	strcpy(exif_param->contrast,		"");
	strcpy(exif_param->saturation,		"");
	strcpy(exif_param->sharpness,		"");

	strcpy(exif_param->FNumber,		(char *)"26/10");
	strcpy(exif_param->exposureProgram,	"3");
	strcpy(exif_param->shutterSpeed,	"");
	strcpy(exif_param->aperture,		"");
	strcpy(exif_param->brightness,		"");
	strcpy(exif_param->exposureBias,	"");
	strcpy(exif_param->maxLensAperture,	(char *)"2757/1000");
	strcpy(exif_param->flash,		str1);
	strcpy(exif_param->lensFocalLength,	(char *)"354/100");
	strcpy(exif_param->userComments,	"User comments");
	ret = 0;

	return ret;

}

bool camdrv_ss_s5k4ecgx_get_esd_status(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);	

	bool bEsdStatus = false;
	unsigned short read_value = 0;

//	CAM_INFO_PRINTK("%s = %d \n",__func__,bEsdStatus);

	camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x01A8);
	camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value);

	if (read_value != 0xAAAA) {
		bEsdStatus = true;		//ESD detected
		CAM_ERROR_PRINTK("%s :: ESD detected. 1st read value is 0x%x \n",__func__,read_value);
	}

//	CAM_INFO_PRINTK("%s first detection is ok. readed value is 0x%x \n",__func__,read_value);

	camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0xD000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x1002);
	camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value);

	if (read_value != 0x0000) {
		bEsdStatus = true;		//ESD detected
		CAM_ERROR_PRINTK("%s :: ESD detected. 2nd read value is 0x%x \n",__func__,read_value);
	}

//	CAM_INFO_PRINTK("%s 2nd detection is ok. readed value is 0x%x \n",__func__,read_value);

	return bEsdStatus;
}

enum camdrv_ss_capture_mode_state
	camdrv_ss_s5k4ecgx_get_mode_change(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned short read_value = 0xFFFF;
	int ret = -1;
	camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
	camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x215F);
	ret = camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value);
	if (ret < 0) {
		CAM_ERROR_PRINTK("%s: CAMDRV_SS_CAPTURE_MODE_READ_FAILED\n");
		return CAMDRV_SS_CAPTURE_MODE_READ_FAILED; /* read fail */
	}

	/* CAM_INFO_PRINTK("read mode change value <7000.215Fh>=0x%X\n", read_value); */
	if (read_value == 0x0100) { /* capture mode */
		CAM_INFO_PRINTK(
		"%s: CAPTURE_MODE_READY read_value = 0x%x\n"
		, __func__, read_value);

		return CAMDRV_SS_CAPTURE_MODE_READY;
	} else if (read_value == 0x0000) { /* preview mode */
		CAM_INFO_PRINTK(
		"%s: CAPTURE_MODE_READ_PROCESSING..read_value = 0x%x\n"
		, __func__, read_value);

		return CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING;
	}
	CAM_ERROR_PRINTK(
	"%s: CAPTURE_MODE_READ_FAILED! read_value = 0x%x\n"
	, __func__, read_value);

	return CAMDRV_SS_CAPTURE_MODE_READ_FAILED; /* unknown mode */

}
//New code
int camdrv_ss_s5k4ecgx_set_preflash_sequence(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;
         int n=0;//Nikhil
         int ret =0;
        CAM_INFO_PRINTK("camdrv_ss_s5k4ecgx_set_preflash_sequence E \n");
         camdrv_ss_s5k4ecgx_set_ae_stable_status(sd,0x0000); 	

     //if (state->camera_af_flash_checked == 0) 
    //{
       state->camera_flash_fire = 0;
       if (state->current_flash_mode == FLASH_MODE_ON) {
            state->camera_flash_fire = 1;
       }
       else if (state->current_flash_mode == FLASH_MODE_AUTO) 
       {
          bool bflash_needed = false;
          if (camdrv_ss_s5k4ecgx_check_flash_needed(sd)){
            bflash_needed = camdrv_ss_s5k4ecgx_check_flash_needed(sd);

	} else {
            CAM_ERROR_PRINTK( "%s : check_flash_needed is NULL !!!\n", __func__);
          	}
		  
          if (bflash_needed)
          {
            state->camera_flash_fire = 1;
          }
	}
    //}

	if (state->camera_flash_fire) {
		err = camdrv_ss_i2c_set_config_register(client,
			s5k4ecgx_Pre_Flash_Start_EVT1,
			ARRAY_SIZE(s5k4ecgx_Pre_Flash_Start_EVT1),
			"Pre_Flash_Start_EVT1");
		if (err < 0) {
			CAM_ERROR_PRINTK("[%s : %d] ERROR! Couldn't Set Pre Flash start \n", __FILE__, __LINE__);
		}

		 camdrv_ss_s5k4ecgx_flash_control(sd, FLASH_CONTROL_MIDDLE_LEVEL);
		 ret =1;

	}
 //state->camera_flash_fire = 0;
 return ret;
}
void  camdrv_ss_s5k4ecgx_set_camera_vendorid (char *rear_camera_vendorid)
{
    strcpy(rear_camera_vendorid,"0x0302");
}

//END code
bool camdrv_ss_sensor_functions_s5k4ecgx(struct camdrv_ss_sensor_cap *sensor)
{

	strcpy(sensor->name,S5K4ECGX_NAME);
	sensor->supported_preview_framesize_list  = s5k4ecgx_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(s5k4ecgx_supported_preview_framesize_list);
	
	sensor->supported_capture_framesize_list  =  s5k4ecgx_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(s5k4ecgx_supported_capture_framesize_list);
	
	sensor->fmts 				   = s5k4ecgx_fmts;
	sensor->rows_num_fmts		   =ARRAY_SIZE(s5k4ecgx_fmts);


	sensor->controls				   =s5k4ecgx_controls;
	sensor->rows_num_controls	      =ARRAY_SIZE(s5k4ecgx_controls);
	
	sensor->default_pix_fmt 				   = S5K4ECGX_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt			   = S5K4ECGX_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size 		  			 = S5K4ECGX_REGISTER_SIZE;
	sensor->skip_frames						 = 1;

	sensor->delay_duration				= S5K4ECGX_DELAY_DURATION;

	/* sensor dependent functions */
	
/* mandatory*/
	sensor->thumbnail_ioctl			       = camdrv_ss_s5k4ecgx_ss_ioctl;
	sensor->enum_frameintervals			   = camdrv_ss_s5k4ecgx_enum_frameintervals;
	
/*optional*/
	sensor->get_nightmode		   		 = camdrv_ss_s5k4ecgx_get_nightmode; //aska add
	sensor->set_preview_start      = camdrv_ss_s5k4ecgx_set_preview_start;//aska add
	sensor->set_capture_start      = camdrv_ss_s5k4ecgx_set_capture_start;//aska add

	sensor->set_iso      					 = camdrv_ss_s5k4ecgx_set_iso;//aska add
	sensor->set_white_balance      = camdrv_ss_s5k4ecgx_set_white_balance;//aska add
	sensor->get_ae_stable_status      =  camdrv_ss_s5k4ecgx_get_ae_stable_status;
	sensor->set_auto_focus		 	  =  camdrv_ss_s5k4ecgx_set_auto_focus;
	sensor->get_auto_focus_status     = camdrv_ss_s5k4ecgx_get_auto_focus_status;

	sensor->set_touch_focus_area 	  =  camdrv_ss_s5k4ecgx_set_touch_focus_area;
	sensor->set_touch_focus		 	  =  camdrv_ss_s5k4ecgx_set_touch_focus;
	sensor->get_touch_focus_status     = camdrv_ss_s5k4ecgx_get_touch_focus_status;
	#ifdef CONFIG_FLASH_MIC2871
	sensor->flash_control			= camdrv_ss_s5k4ecgx_MIC2871_flash_control;
	#else
	#ifdef CONFIG_FLASH_DS8515
	sensor->flash_control    	   =	camdrv_ss_s5k4ecgx_DS8515_flash_control;
	#else
	sensor->flash_control			= camdrv_ss_s5k4ecgx_AAT_flash_control;
	#endif
	#endif
	sensor->i2c_set_data_burst   	   = camdrv_ss_s5k4ecgx_i2c_set_data_burst;
	sensor->check_flash_needed   	   = camdrv_ss_s5k4ecgx_check_flash_needed;
//	sensor->get_light_condition   = camdrv_ss_s5k4ecgx_get_light_condition;
	
	sensor->sensor_power 			   = camdrv_ss_s5k4ecgx_sensor_power;

	sensor->get_exif_sensor_info       = camdrv_ss_s5k4ecgx_get_sensor_param_for_exif;
	sensor->getEsdStatus 			   = camdrv_ss_s5k4ecgx_get_esd_status;
	/* use 150ms delay or frame skip instead of register read */
	/* sensor->get_mode_change_reg	= camdrv_ss_s5k4ecgx_get_mode_change; */

	sensor->set_scene_mode 			   = camdrv_ss_s5k4ecgx_set_scene_mode;   //denis
        sensor->get_prefalsh_on                      =  camdrv_ss_s5k4ecgx_set_preflash_sequence; //Nikhil

       sensor->rear_camera_vendorid = camdrv_ss_s5k4ecgx_set_camera_vendorid; // test
	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */
	
	sensor->init_regs						  = s5k4ecgx_init_regs;
	sensor->rows_num_init_regs				  = ARRAY_SIZE(s5k4ecgx_init_regs);
	
	sensor->preview_camera_regs 			  = s5k4ecgx_preview_camera_regs;
	sensor->rows_num_preview_camera_regs 	  = ARRAY_SIZE(s5k4ecgx_preview_camera_regs);
			
	/*snapshot mode*/
	sensor->snapshot_normal_regs			  =	s5k4ecgx_snapshot_normal_regs;
	sensor->rows_num_snapshot_normal_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_normal_regs);

	sensor->snapshot_lowlight_regs			  =	s5k4ecgx_snapshot_lowlight_regs;
	sensor->rows_num_snapshot_lowlight_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_lowlight_regs);

	sensor->snapshot_highlight_regs			  =	s5k4ecgx_snapshot_highlight_regs;
	sensor->rows_num_snapshot_highlight_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_highlight_regs);

	sensor->snapshot_nightmode_regs			  =	s5k4ecgx_snapshot_nightmode_regs;
	sensor->rows_num_snapshot_nightmode_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_nightmode_regs);

	sensor->snapshot_flash_on_regs			  =	s5k4ecgx_snapshot_flash_on_regs;
	sensor->rows_num_snapshot_flash_on_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_flash_on_regs);

	sensor->snapshot_af_preflash_on_regs			  =	s5k4ecgx_snapshot_af_preflash_on_regs;
	sensor->rows_num_snapshot_af_preflash_on_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_af_preflash_on_regs);

	sensor->snapshot_af_preflash_off_regs			  =	s5k4ecgx_snapshot_af_preflash_off_regs;
	sensor->rows_num_snapshot_af_preflash_off_regs	  = ARRAY_SIZE(s5k4ecgx_snapshot_af_preflash_off_regs);


	/*effect*/
	sensor->effect_normal_regs			      =	s5k4ecgx_effect_normal_regs;
	sensor->rows_num_effect_normal_regs      = ARRAY_SIZE(s5k4ecgx_effect_normal_regs);
	
	sensor->effect_negative_regs		      =	s5k4ecgx_effect_negative_regs;
	sensor->rows_num_effect_negative_regs	 = ARRAY_SIZE(s5k4ecgx_effect_negative_regs);
	
	sensor->effect_sepia_regs			      =	s5k4ecgx_effect_sepia_regs;
	sensor->rows_num_effect_sepia_regs	  	  = ARRAY_SIZE(s5k4ecgx_effect_sepia_regs);
	
	sensor->effect_mono_regs			      =	s5k4ecgx_effect_mono_regs;
	sensor->rows_num_effect_mono_regs	      = ARRAY_SIZE(s5k4ecgx_effect_mono_regs);

	sensor->effect_aqua_regs				  =	s5k4ecgx_effect_aqua_regs;
	sensor->rows_num_effect_aqua_regs	  	  = ARRAY_SIZE(s5k4ecgx_effect_aqua_regs);
	
	sensor->effect_sharpen_regs 		      =	s5k4ecgx_effect_sharpen_regs;
	sensor->rows_num_effect_sharpen_regs     = ARRAY_SIZE(s5k4ecgx_effect_sharpen_regs);
	
	sensor->effect_solarization_regs		   = s5k4ecgx_effect_solarization_regs;
	sensor->rows_num_effect_solarization_regs = ARRAY_SIZE(s5k4ecgx_effect_solarization_regs);
	
	sensor->effect_black_white_regs 	       =	s5k4ecgx_effect_black_white_regs;
	sensor->rows_num_effect_black_white_regs  = ARRAY_SIZE(s5k4ecgx_effect_black_white_regs);
	

	/*wb*/
	sensor->wb_auto_regs				  =	s5k4ecgx_wb_auto_regs;
	sensor->rows_num_wb_auto_regs	  	  = ARRAY_SIZE(s5k4ecgx_wb_auto_regs);

	//sensor->wb_sunny_regs				 =	s5k4ecgx_wb_sunny_regs;
	//sensor->rows_num_wb_sunny_regs	  	 = ARRAY_SIZE(s5k4ecgx_wb_sunny_regs);
	
	sensor->wb_cloudy_regs				 =	s5k4ecgx_wb_cloudy_regs;
	sensor->rows_num_wb_cloudy_regs	 = ARRAY_SIZE(s5k4ecgx_wb_cloudy_regs);
	
	//sensor->wb_tungsten_regs			 =	s5k4ecgx_wb_tungsten_regs;
	//sensor->rows_num_wb_tungsten_regs	 = ARRAY_SIZE(s5k4ecgx_wb_tungsten_regs);
	//Changed reg table name to fit UI's name
	sensor->wb_daylight_regs				 =	s5k4ecgx_wb_daylight_regs;
	sensor->rows_num_wb_daylight_regs	  	 = ARRAY_SIZE(s5k4ecgx_wb_daylight_regs);
	sensor->wb_incandescent_regs				 =	s5k4ecgx_wb_incandescent_regs;
	sensor->rows_num_wb_incandescent_regs	  	 = ARRAY_SIZE(s5k4ecgx_wb_incandescent_regs);

	sensor->wb_fluorescent_regs 		  =	s5k4ecgx_wb_fluorescent_regs;
	sensor->rows_num_wb_fluorescent_regs  = ARRAY_SIZE(s5k4ecgx_wb_fluorescent_regs);



	/*metering*/
	sensor->metering_matrix_regs		  =	s5k4ecgx_metering_matrix_regs;
	sensor->rows_num_metering_matrix_regs	  	  = ARRAY_SIZE(s5k4ecgx_metering_matrix_regs);

	sensor->metering_center_regs		  =	s5k4ecgx_metering_center_regs;
	sensor->rows_num_metering_center_regs	  	  = ARRAY_SIZE(s5k4ecgx_metering_center_regs);

	sensor->metering_spot_regs			  =	  s5k4ecgx_metering_spot_regs;
	sensor->rows_num_metering_spot_regs	  		  = ARRAY_SIZE(s5k4ecgx_metering_spot_regs);
	
	/*EV*/
	sensor->ev_minus_4_regs 			 =	s5k4ecgx_ev_minus_4_regs;
	sensor->rows_num_ev_minus_4_regs	 = ARRAY_SIZE(s5k4ecgx_ev_minus_4_regs);

	sensor->ev_minus_3_regs 			 =	s5k4ecgx_ev_minus_3_regs;
	sensor->rows_num_ev_minus_3_regs	 = ARRAY_SIZE(s5k4ecgx_ev_minus_3_regs);

	sensor->ev_minus_2_regs 			 =	s5k4ecgx_ev_minus_2_regs;
	sensor->rows_num_ev_minus_2_regs	  = ARRAY_SIZE(s5k4ecgx_ev_minus_2_regs);

	sensor->ev_minus_1_regs 			 =	s5k4ecgx_ev_minus_1_regs;
	sensor->rows_num_ev_minus_1_regs	 = ARRAY_SIZE(s5k4ecgx_ev_minus_1_regs);

	sensor->ev_default_regs 			 =	s5k4ecgx_ev_default_regs;
	sensor->rows_num_ev_default_regs	 = ARRAY_SIZE(s5k4ecgx_ev_default_regs);

	sensor->ev_plus_1_regs				 =	s5k4ecgx_ev_plus_1_regs;
	sensor->rows_num_ev_plus_1_regs	 = ARRAY_SIZE(s5k4ecgx_ev_plus_1_regs);

	sensor->ev_plus_2_regs				 =	s5k4ecgx_ev_plus_2_regs;
	sensor->rows_num_ev_plus_2_regs	 = ARRAY_SIZE(s5k4ecgx_ev_plus_2_regs);

	sensor->ev_plus_3_regs				 =	s5k4ecgx_ev_plus_3_regs;
	sensor->rows_num_ev_plus_3_regs	 = ARRAY_SIZE(s5k4ecgx_ev_plus_3_regs);

	sensor->ev_plus_4_regs				 =	s5k4ecgx_ev_plus_4_regs;
	sensor->rows_num_ev_plus_4_regs	 = ARRAY_SIZE(s5k4ecgx_ev_plus_4_regs);

	
	/*contrast*/
	sensor->contrast_minus_2_regs		 	 =	s5k4ecgx_contrast_minus_2_regs;
	sensor->rows_num_contrast_minus_2_regs	 = ARRAY_SIZE(s5k4ecgx_contrast_minus_2_regs);

	sensor->contrast_minus_1_regs		     =	s5k4ecgx_contrast_minus_1_regs;
	sensor->rows_num_contrast_minus_1_regs	 = ARRAY_SIZE(s5k4ecgx_contrast_minus_1_regs);
  
	sensor->contrast_default_regs			 =	s5k4ecgx_contrast_default_regs;
	sensor->rows_num_contrast_default_regs  = ARRAY_SIZE(s5k4ecgx_contrast_default_regs);

	sensor->contrast_plus_1_regs			 =	s5k4ecgx_contrast_plus_1_regs;
	sensor->rows_num_contrast_plus_1_regs	 = ARRAY_SIZE(s5k4ecgx_contrast_plus_1_regs);

	sensor->contrast_plus_2_regs			 =	s5k4ecgx_contrast_plus_2_regs;
	sensor->rows_num_contrast_plus_2_regs	 = ARRAY_SIZE(s5k4ecgx_contrast_plus_2_regs);
	
	/*sharpness*/ 
	sensor->sharpness_minus_3_regs		     =	s5k4ecgx_sharpness_minus_3_regs;
	sensor->rows_num_sharpness_minus_3_regs= ARRAY_SIZE(s5k4ecgx_sharpness_minus_3_regs);

	sensor->sharpness_minus_2_regs		     =	s5k4ecgx_sharpness_minus_2_regs;
	sensor->rows_num_sharpness_minus_2_regs= ARRAY_SIZE(s5k4ecgx_sharpness_minus_2_regs);

	sensor->sharpness_minus_1_regs		 	 =	s5k4ecgx_sharpness_minus_1_regs;
	sensor->rows_num_sharpness_minus_1_regs = ARRAY_SIZE(s5k4ecgx_sharpness_minus_1_regs);

	sensor->sharpness_default_regs		 	 =	s5k4ecgx_sharpness_default_regs;
	sensor->rows_num_sharpness_default_regs  = ARRAY_SIZE(s5k4ecgx_sharpness_default_regs);

	sensor->sharpness_plus_1_regs		     =	s5k4ecgx_sharpness_plus_1_regs;
	sensor->rows_num_sharpness_plus_1_regs	 =	ARRAY_SIZE(s5k4ecgx_sharpness_plus_1_regs);

	sensor->sharpness_plus_2_regs		     =	s5k4ecgx_sharpness_plus_2_regs;
	sensor->rows_num_sharpness_plus_2_regs	 =	ARRAY_SIZE(s5k4ecgx_sharpness_plus_2_regs);
    
	sensor->sharpness_plus_3_regs		     =	s5k4ecgx_sharpness_plus_3_regs;
	sensor->rows_num_sharpness_plus_3_regs	 =	ARRAY_SIZE(s5k4ecgx_sharpness_plus_3_regs);

		
	/*saturation*/
	sensor->saturation_minus_2_regs 	      =	s5k4ecgx_saturation_minus_2_regs;
	sensor->rows_num_saturation_minus_2_regs = ARRAY_SIZE(s5k4ecgx_saturation_minus_2_regs);

	sensor->saturation_minus_1_regs 	 	  =	s5k4ecgx_saturation_minus_1_regs;
	sensor->rows_num_saturation_minus_1_regs = ARRAY_SIZE(s5k4ecgx_saturation_minus_1_regs);

	sensor->saturation_default_regs 	      =	s5k4ecgx_saturation_default_regs;
	sensor->rows_num_saturation_default_regs  = ARRAY_SIZE(s5k4ecgx_saturation_default_regs);

	sensor->saturation_plus_1_regs		       =	s5k4ecgx_saturation_plus_1_regs;
	sensor->rows_num_saturation_plus_1_regs	= ARRAY_SIZE(s5k4ecgx_saturation_plus_1_regs);

	sensor->saturation_plus_2_regs		       =	s5k4ecgx_saturation_plus_2_regs;
	sensor->rows_num_saturation_plus_2_regs   = ARRAY_SIZE(s5k4ecgx_saturation_plus_2_regs);

	
	/*zoom*/
	sensor->zoom_00_regs					 =	s5k4ecgx_zoom_00_regs;
	sensor->rows_num_zoom_00_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_00_regs);

	sensor->zoom_01_regs					 =	s5k4ecgx_zoom_01_regs;
	sensor->rows_num_zoom_01_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_01_regs);

	sensor->zoom_02_regs					 =	s5k4ecgx_zoom_02_regs;
	sensor->rows_num_zoom_02_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_02_regs);

	sensor->zoom_03_regs					 =	s5k4ecgx_zoom_03_regs;
	sensor->rows_num_zoom_03_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_03_regs);

	sensor->zoom_04_regs					 =	s5k4ecgx_zoom_04_regs;
	sensor->rows_num_zoom_04_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_04_regs);

	sensor->zoom_05_regs					 =	s5k4ecgx_zoom_05_regs;
	sensor->rows_num_zoom_05_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_05_regs);

	sensor->zoom_06_regs					 =	s5k4ecgx_zoom_06_regs;
	sensor->rows_num_zoom_06_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_06_regs);

	sensor->zoom_07_regs					 =	s5k4ecgx_zoom_07_regs;
	sensor->rows_num_zoom_07_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_07_regs);

	sensor->zoom_08_regs					 =	s5k4ecgx_zoom_08_regs;
	sensor->rows_num_zoom_08_regs	  		  = ARRAY_SIZE(s5k4ecgx_zoom_08_regs);

	
	/*scene mode*/
	sensor->scene_none_regs 			 		=	s5k4ecgx_scene_none_regs;
	sensor->rows_num_scene_none_regs	  		 = ARRAY_SIZE(s5k4ecgx_scene_none_regs);

	sensor->scene_portrait_regs 		 		=	s5k4ecgx_scene_portrait_regs;
	sensor->rows_num_scene_portrait_regs	  	= ARRAY_SIZE(s5k4ecgx_scene_portrait_regs);

	sensor->scene_nightshot_regs			   =	s5k4ecgx_scene_nightshot_regs;
	sensor->rows_num_scene_nightshot_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_nightshot_regs);

	sensor->scene_backlight_regs			  =	s5k4ecgx_scene_backlight_regs;
	sensor->rows_num_scene_backlight_regs	   = ARRAY_SIZE(s5k4ecgx_scene_backlight_regs);

	sensor->scene_landscape_regs			   =	s5k4ecgx_scene_landscape_regs;
	sensor->rows_num_scene_landscape_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_landscape_regs);

	sensor->scene_sports_regs			      =	s5k4ecgx_scene_sports_regs;
	sensor->rows_num_scene_sports_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_sports_regs);

	sensor->scene_party_indoor_regs 	 	  =	s5k4ecgx_scene_party_indoor_regs;
	sensor->rows_num_scene_party_indoor_regs  = ARRAY_SIZE(s5k4ecgx_scene_party_indoor_regs);

	sensor->scene_beach_snow_regs				 =	s5k4ecgx_scene_beach_snow_regs;
	sensor->rows_num_scene_beach_snow_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_beach_snow_regs);

	sensor->scene_sunset_regs			 		 =	s5k4ecgx_scene_sunset_regs;
	sensor->rows_num_scene_sunset_regs	  		  = ARRAY_SIZE(s5k4ecgx_scene_sunset_regs);

	sensor->scene_duskdawn_regs 				 =	s5k4ecgx_scene_duskdawn_regs;
	sensor->rows_num_scene_duskdawn_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_duskdawn_regs);

	sensor->scene_fall_color_regs				 =	s5k4ecgx_scene_fall_color_regs;
	sensor->rows_num_scene_fall_color_regs	  	  = ARRAY_SIZE(s5k4ecgx_scene_fall_color_regs);

	sensor->scene_fireworks_regs				 =	s5k4ecgx_scene_fireworks_regs;
	sensor->rows_num_scene_fireworks_regs	  	  = ARRAY_SIZE(s5k4ecgx_scene_fireworks_regs);
	
	sensor->scene_candle_light_regs 	 		=	s5k4ecgx_scene_candle_light_regs;
	sensor->rows_num_scene_candle_light_regs	= ARRAY_SIZE(s5k4ecgx_scene_candle_light_regs);

	sensor->scene_text_regs			   =	s5k4ecgx_scene_text_regs;
	sensor->rows_num_scene_text_regs	  	 = ARRAY_SIZE(s5k4ecgx_scene_text_regs);

		
	/*fps*/
	sensor->fps_auto_regs				 =	s5k4ecgx_fps_auto_regs;
	sensor->rows_num_fps_auto_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_auto_regs);

	sensor->fps_5_regs					 =	s5k4ecgx_fps_5_regs;
	sensor->rows_num_fps_5_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_5_regs);

	sensor->fps_7_regs					 =	s5k4ecgx_fps_7_regs;
	sensor->rows_num_fps_7_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_7_regs);

	sensor->fps_10_regs 				 =	s5k4ecgx_fps_10_regs;
	sensor->rows_num_fps_10_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_10_regs);

	sensor->fps_15_regs 				 =	s5k4ecgx_fps_15_regs;
	sensor->rows_num_fps_15_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_15_regs);

	sensor->fps_20_regs 				 =	s5k4ecgx_fps_20_regs;
	sensor->rows_num_fps_20_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_20_regs);

	sensor->fps_25_regs 				 =	s5k4ecgx_fps_25_regs;
	sensor->rows_num_fps_25_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_25_regs);

	sensor->fps_30_regs 				 =	s5k4ecgx_fps_30_regs;
	sensor->rows_num_fps_30_regs 		  = ARRAY_SIZE(s5k4ecgx_fps_30_regs);
	
	sensor->fps_60_regs 				 =	s5k4ecgx_fps_60_regs;
	sensor->rows_num_fps_60_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_60_regs);

	sensor->fps_120_regs 				 =	s5k4ecgx_fps_120_regs;
	sensor->rows_num_fps_120_regs	  		  = ARRAY_SIZE(s5k4ecgx_fps_120_regs);
	

	
	/*quality*/
	sensor->quality_superfine_regs			 =	s5k4ecgx_quality_superfine_regs;
	sensor->rows_num_quality_superfine_regs	  = ARRAY_SIZE(s5k4ecgx_quality_superfine_regs);

	sensor->quality_fine_regs			 =	s5k4ecgx_quality_fine_regs;
	sensor->rows_num_quality_fine_regs	  = ARRAY_SIZE(s5k4ecgx_quality_fine_regs);

	sensor->quality_normal_regs 		   =	s5k4ecgx_quality_normal_regs;
	sensor->rows_num_quality_normal_regs  = ARRAY_SIZE(s5k4ecgx_effect_normal_regs);

	sensor->quality_economy_regs			 =	s5k4ecgx_quality_economy_regs;
	sensor->rows_num_quality_economy_regs   = ARRAY_SIZE(s5k4ecgx_quality_economy_regs);

	
	/*preview size */
	sensor->preview_size_176x144_regs	        =	s5k4ecgx_preview_size_176x144_regs;
	sensor->rows_num_preview_size_176x144_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_176x144_regs);

	sensor->preview_size_320x240_regs	         =	s5k4ecgx_preview_size_320x240_regs; 
	sensor->rows_num_preview_size_320x240_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_320x240_regs);

	sensor->preview_size_352x288_regs	          =	s5k4ecgx_preview_size_352x288_regs; 
	sensor->rows_num_preview_size_352x288_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_352x288_regs);

	sensor->preview_size_640x480_regs	          =	s5k4ecgx_preview_size_640x480_regs; 
	sensor->rows_num_preview_size_640x480_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_640x480_regs);

	sensor->preview_size_704x576_regs	 		=	s5k4ecgx_preview_size_704x576_regs; 
	sensor->rows_num_preview_size_704x576_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_704x576_regs);

	sensor->preview_size_720x480_regs	 		=	s5k4ecgx_preview_size_720x480_regs; 
	sensor->rows_num_preview_size_720x480_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_720x480_regs);
	
	sensor->preview_size_800x480_regs	        =	s5k4ecgx_preview_size_800x480_regs;
	sensor->rows_num_preview_size_800x480_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_800x480_regs);

	sensor->preview_size_800x600_regs	        =	s5k4ecgx_preview_size_800x600_regs;
	sensor->rows_num_preview_size_800x600_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_800x600_regs);

	sensor->preview_size_1024x600_regs	         =	s5k4ecgx_preview_size_1024x600_regs; 
	sensor->rows_num_preview_size_1024x600_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_1024x600_regs);

	sensor->preview_size_1024x768_regs	          =	s5k4ecgx_preview_size_1024x768_regs; 
	sensor->rows_num_preview_size_1024x768_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_1024x768_regs);

	sensor->HD_Camcorder_regs	          =	s5k4ecgx_HD_Camcorder_regs; 
	sensor->rows_num_HD_Camcorder_regs	  = ARRAY_SIZE(s5k4ecgx_HD_Camcorder_regs);
	
	sensor->HD_Camcorder_Disable_regs	          =	s5k4ecgx_HD_Camcorder_Disable_regs; 
	sensor->rows_num_HD_Camcorder_Disable_regs	  = ARRAY_SIZE(s5k4ecgx_HD_Camcorder_Disable_regs);

	sensor->preview_size_1280x960_regs	          =	s5k4ecgx_preview_size_1280x960_regs; 
	sensor->rows_num_preview_size_1280x960_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_1280x960_regs);

	sensor->preview_size_1600x960_regs	 		=	s5k4ecgx_preview_size_1600x960_regs; 
	sensor->rows_num_preview_size_1600x960_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_1600x960_regs);

	sensor->preview_size_1600x1200_regs	 		=	s5k4ecgx_preview_size_1600x1200_regs; 
	sensor->rows_num_preview_size_1600x1200_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_1600x1200_regs);

	sensor->preview_size_2048x1232_regs	        =	s5k4ecgx_preview_size_2048x1232_regs;
	sensor->rows_num_preview_size_2048x1232_regs	 = ARRAY_SIZE(s5k4ecgx_preview_size_2048x1232_regs);

	sensor->preview_size_2048x1536_regs	         =	s5k4ecgx_preview_size_2048x1536_regs; 
	sensor->rows_num_preview_size_2048x1536_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_2048x1536_regs);

	sensor->preview_size_2560x1920_regs	          =	s5k4ecgx_preview_size_2560x1920_regs; 
	sensor->rows_num_preview_size_2560x1920_regs	  = ARRAY_SIZE(s5k4ecgx_preview_size_2560x1920_regs);
  
	
	/*Capture size */
	sensor->capture_size_640x480_regs	 		=	s5k4ecgx_capture_size_640x480_regs;
	sensor->rows_num_capture_size_640x480_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_640x480_regs);

	sensor->capture_size_720x480_regs  			=	s5k4ecgx_capture_size_720x480_regs; 
	sensor->rows_num_capture_size_720x480_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_720x480_regs);

	sensor->capture_size_800x480_regs	 		=	s5k4ecgx_capture_size_800x480_regs;
	sensor->rows_num_capture_size_800x480_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_800x480_regs);

	sensor->capture_size_800x486_regs	 		=	s5k4ecgx_capture_size_800x486_regs;
	sensor->rows_num_capture_size_800x486_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_800x486_regs);

	sensor->capture_size_800x600_regs  			=	s5k4ecgx_capture_size_800x600_regs; 
	sensor->rows_num_capture_size_800x600_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_800x600_regs);

    	sensor->capture_size_1024x600_regs	 		=	s5k4ecgx_capture_size_1024x600_regs;
	sensor->rows_num_capture_size_1024x600_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_1024x600_regs);

	sensor->capture_size_1024x768_regs  			=	s5k4ecgx_capture_size_1024x768_regs; 
	sensor->rows_num_capture_size_1024x768_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_1024x768_regs);

	sensor->capture_size_1280x960_regs  			=	s5k4ecgx_capture_size_1280x960_regs; 
	sensor->rows_num_capture_size_1280x960_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_1280x960_regs);

    	sensor->capture_size_1600x960_regs	 		=	s5k4ecgx_capture_size_1600x960_regs;
	sensor->rows_num_capture_size_1600x960_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_1600x960_regs);

	sensor->capture_size_1600x1200_regs  			=	s5k4ecgx_capture_size_1600x1200_regs; 
	sensor->rows_num_capture_size_1600x1200_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_1600x1200_regs);

	sensor->capture_size_2048x1232_regs  			=	s5k4ecgx_capture_size_2048x1232_regs; 
	sensor->rows_num_capture_size_2048x1232_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_2048x1232_regs);

	sensor->capture_size_2048x1536_regs  			=	s5k4ecgx_capture_size_2048x1536_regs; 
	sensor->rows_num_capture_size_2048x1536_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_2048x1536_regs);

	sensor->capture_size_2560x1536_regs  			=	s5k4ecgx_capture_size_2560x1536_regs; 
	sensor->rows_num_capture_size_2560x1536_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_2560x1536_regs);

	sensor->capture_size_2560x1920_regs  			=	s5k4ecgx_capture_size_2560x1920_regs; 
	sensor->rows_num_capture_size_2560x1920_regs	 = ARRAY_SIZE(s5k4ecgx_capture_size_2560x1920_regs);

	
	/*pattern*/
	sensor->pattern_on_regs 			  = s5k4ecgx_pattern_on_regs;
	sensor->rows_num_pattern_on_regs	  = ARRAY_SIZE(s5k4ecgx_pattern_on_regs);
	
	sensor->pattern_off_regs			  = s5k4ecgx_pattern_off_regs;
	sensor->rows_num_pattern_off_regs	  = ARRAY_SIZE(s5k4ecgx_pattern_off_regs);

	/*AE*/
	sensor->ae_lock_regs			  = s5k4ecgx_ae_lock_regs;
	sensor->rows_num_ae_lock_regs	  = ARRAY_SIZE(s5k4ecgx_ae_lock_regs);

		
	sensor->ae_unlock_regs			  = s5k4ecgx_ae_unlock_regs;
	sensor->rows_num_ae_unlock_regs	  = ARRAY_SIZE(s5k4ecgx_ae_unlock_regs);


	/*AWB*/

	sensor->awb_lock_regs			  = s5k4ecgx_awb_lock_regs;
	sensor->rows_num_awb_lock_regs	  = ARRAY_SIZE(s5k4ecgx_awb_lock_regs);
		
	sensor->awb_unlock_regs			  = s5k4ecgx_awb_unlock_regs;
	sensor->rows_num_awb_unlock_regs	  = ARRAY_SIZE(s5k4ecgx_awb_unlock_regs);
	
	//ISO//
	sensor->iso_auto_regs			  = s5k4ecgx_iso_auto_regs;
	sensor->rows_num_iso_auto_regs	  = ARRAY_SIZE(s5k4ecgx_iso_auto_regs);

	sensor->iso_50_regs			  = s5k4ecgx_iso_50_regs;
	sensor->rows_num_iso_50_regs	  = ARRAY_SIZE(s5k4ecgx_iso_50_regs);

	sensor->iso_100_regs			  = s5k4ecgx_iso_100_regs;
	sensor->rows_num_iso_100_regs	  = ARRAY_SIZE(s5k4ecgx_iso_100_regs);

	sensor->iso_200_regs			  = s5k4ecgx_iso_200_regs;
	sensor->rows_num_iso_200_regs	  = ARRAY_SIZE(s5k4ecgx_iso_200_regs);

	sensor->iso_400_regs			  = s5k4ecgx_iso_400_regs;
	sensor->rows_num_iso_400_regs	  = ARRAY_SIZE(s5k4ecgx_iso_400_regs);

    /* WDR */
	sensor->wdr_on_regs			  = s5k4ecgx_wdr_on_regs;
	sensor->rows_num_wdr_on_regs	  = ARRAY_SIZE(s5k4ecgx_wdr_on_regs);

	sensor->wdr_off_regs			  = s5k4ecgx_wdr_off_regs;
	sensor->rows_num_wdr_off_regs	  = ARRAY_SIZE(s5k4ecgx_wdr_off_regs);


    /* CCD EV */
	sensor->ev_camcorder_minus_4_regs 			 =	s5k4ecgx_ev_camcorder_minus_4_regs;
	sensor->rows_num_ev_camcorder_minus_4_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_minus_4_regs);

	sensor->ev_camcorder_minus_3_regs 			 =	s5k4ecgx_ev_camcorder_minus_3_regs;
	sensor->rows_num_ev_camcorder_minus_3_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_minus_3_regs);

	sensor->ev_camcorder_minus_2_regs 			 =	s5k4ecgx_ev_camcorder_minus_2_regs;
	sensor->rows_num_ev_camcorder_minus_2_regs	  = ARRAY_SIZE(s5k4ecgx_ev_camcorder_minus_2_regs);

	sensor->ev_camcorder_minus_1_regs 			 =	s5k4ecgx_ev_camcorder_minus_1_regs;
	sensor->rows_num_ev_camcorder_minus_1_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_minus_1_regs);

	sensor->ev_camcorder_default_regs 			 =	s5k4ecgx_ev_camcorder_default_regs;
	sensor->rows_num_ev_camcorder_default_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_default_regs);

	sensor->ev_camcorder_plus_1_regs				 =	s5k4ecgx_ev_camcorder_plus_1_regs;
	sensor->rows_num_ev_camcorder_plus_1_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_plus_1_regs);

	sensor->ev_camcorder_plus_2_regs				 =	s5k4ecgx_ev_camcorder_plus_2_regs;
	sensor->rows_num_ev_camcorder_plus_2_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_plus_2_regs);

	sensor->ev_camcorder_plus_3_regs				 =	s5k4ecgx_ev_camcorder_plus_3_regs;
	sensor->rows_num_ev_camcorder_plus_3_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_plus_3_regs);

	sensor->ev_camcorder_plus_4_regs				 =	s5k4ecgx_ev_camcorder_plus_4_regs;
	sensor->rows_num_ev_camcorder_plus_4_regs	 = ARRAY_SIZE(s5k4ecgx_ev_camcorder_plus_4_regs);


	/* auto contrast */
	sensor->auto_contrast_on_regs				 =	s5k4ecgx_auto_contrast_on_regs;
	sensor->rows_num_auto_contrast_on_regs	 = ARRAY_SIZE(s5k4ecgx_auto_contrast_on_regs);

	sensor->auto_contrast_off_regs				 =	s5k4ecgx_auto_contrast_off_regs;
	sensor->rows_num_auto_contrast_off_regs	 = ARRAY_SIZE(s5k4ecgx_auto_contrast_off_regs);


	/* af return & focus mode */
	sensor->af_return_inf_pos = s5k4ecgx_af_return_inf_pos;
	sensor->rows_num_af_return_inf_pos     = ARRAY_SIZE(s5k4ecgx_af_return_inf_pos);

	sensor->af_return_macro_pos = s5k4ecgx_af_return_macro_pos;
	sensor->rows_num_af_return_macro_pos     = ARRAY_SIZE(s5k4ecgx_af_return_macro_pos);

	sensor->focus_mode_auto_regs = s5k4ecgx_focus_mode_auto_regs;
	sensor->rows_num_focus_mode_auto_regs     = ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs);

	sensor->focus_mode_macro_regs = s5k4ecgx_focus_mode_macro_regs;
	sensor->rows_num_focus_mode_macro_regs     = ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs);
	sensor->focus_mode_infinity_regs = s5k4ecgx_focus_mode_auto_regs;
	sensor->rows_num_focus_mode_infinity_regs     = ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs);

	sensor->vt_mode_regs = s5k4ecgx_vt_mode_regs;
	sensor->rows_num_vt_mode_regs     = ARRAY_SIZE(s5k4ecgx_vt_mode_regs);
        sensor->main_flash_off_regs    =       s5k4ecgx_Main_Flash_End_EVT1;
        sensor->rows_num_main_flash_off_regs  = ARRAY_SIZE(s5k4ecgx_Main_Flash_End_EVT1);
#if defined(CONFIG_MACH_HAWAII_SS_LOGAN) || defined(CONFIG_MACH_HAWAII_SS_LOGANDS) || defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN)        
  sensor->Pre_Flash_Start_EVT1 = s5k4ecgx_Pre_Flash_Start_EVT1;
	sensor->rows_num_Pre_Flash_Start_EVT1     = ARRAY_SIZE(s5k4ecgx_Pre_Flash_Start_EVT1);
	sensor->Pre_Flash_End_EVT1 = s5k4ecgx_Pre_Flash_End_EVT1;
	sensor->rows_num_Pre_Flash_End_EVT1     = ARRAY_SIZE(s5k4ecgx_Pre_Flash_End_EVT1);
	sensor->Main_Flash_Start_EVT1 = s5k4ecgx_Main_Flash_Start_EVT1;
	sensor->rows_num_Main_Flash_Start_EVT1     = ARRAY_SIZE(s5k4ecgx_Main_Flash_Start_EVT1);
	sensor->Main_Flash_End_EVT1 = s5k4ecgx_Main_Flash_End_EVT1;
	sensor->rows_num_Main_Flash_End_EVT1     = ARRAY_SIZE(s5k4ecgx_Main_Flash_End_EVT1);
	
	sensor->focus_mode_auto_regs_cancel1 = s5k4ecgx_focus_mode_auto_regs_cancel1;
	sensor->rows_num_focus_mode_auto_regs_cancel1     = ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel1);
	sensor->focus_mode_auto_regs_cancel2 = s5k4ecgx_focus_mode_auto_regs_cancel2;
	sensor->rows_num_focus_mode_auto_regs_cancel2     = ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel2);
	sensor->focus_mode_auto_regs_cancel3 = s5k4ecgx_focus_mode_auto_regs_cancel3;
	sensor->rows_num_focus_mode_auto_regs_cancel3     = ARRAY_SIZE(s5k4ecgx_focus_mode_auto_regs_cancel3);
	
	 sensor->focus_mode_macro_regs_cancel1 = s5k4ecgx_focus_mode_macro_regs_cancel1;
	sensor->rows_num_focus_mode_macro_regs_cancel1     = ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel1);
	sensor->focus_mode_macro_regs_cancel2 = s5k4ecgx_Pre_Flash_End_EVT1;
	sensor->rows_num_focus_mode_macro_regs_cancel2     = ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel2);
	sensor->focus_mode_macro_regs_cancel3 = s5k4ecgx_focus_mode_macro_regs_cancel2;
	sensor->rows_num_focus_mode_macro_regs_cancel3     = ARRAY_SIZE(s5k4ecgx_focus_mode_macro_regs_cancel3);
#endif  	
  /* flicker */
	/* sensor->antibanding_50hz_regs	= s5k4ecgx_antibanding_50hz_regs;
	sensor->rows_num_antibanding_50hz_regs	= ARRAY_SIZE(s5k4ecgx_antibanding_50hz_regs);        
	sensor->antibanding_60hz_regs	= s5k4ecgx_antibanding_60hz_regs;
	sensor->rows_num_antibanding_60hz_regs	= ARRAY_SIZE(s5k4ecgx_antibanding_60hz_regs); */

	return true;
};

int camdrv_ss_read_device_id_s5k4ecgx(
		struct i2c_client *client, char *device_id)
{
	int ret = -1;
	/* NEED to WRITE THE I2c REad code to read the deviceid */
	return 0;
}

static int __init camdrv_ss_s5k4ecgx_mod_init(void)
{
	struct camdrv_ss_sensor_reg sens;

	strncpy(sens.name, S5K4ECGX_NAME, sizeof(S5K4ECGX_NAME));
	sens.sensor_functions = camdrv_ss_sensor_functions_s5k4ecgx;
	sens.sensor_power = camdrv_ss_s5k4ecgx_sensor_power;
	sens.read_device_id = camdrv_ss_read_device_id_s5k4ecgx;
#ifdef CONFIG_SOC_CAMERA_MAIN_S5K4ECGX
	sens.isMainSensor = 1;
#endif

#ifdef CONFIG_SOC_CAMERA_SUB_S5K4ECGX
	sens.isMainSensor = 0;
#endif
	camdrv_ss_sensors_register(&sens);

}

module_init(camdrv_ss_s5k4ecgx_mod_init);

MODULE_DESCRIPTION("SAMSUNG CAMERA SENSOR S5K4ECGX ");
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");


