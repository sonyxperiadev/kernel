
/***********************************************************************
* Driver for S5K5CCGX (3MP Camera) from SAMSUNG SYSTEM LSI
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
#include <camdrv_ss.h>
#include <camdrv_ss_s5k5ccgx.h>

#define S5K5CCGX_NAME	"s5k5ccgx"
#define SENSOR_ID 2
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x33F000     //3403776 //2216 * 1536
#define SENSOR_PREVIEW_WIDTH      640 // 1024
#define SENSOR_PREVIEW_HEIGHT     480 // 768
#define AF_OUTER_WINDOW_WIDTH   320
#define AF_OUTER_WINDOW_HEIGHT  266
#define AF_INNER_WINDOW_WIDTH   143
#define AF_INNER_WINDOW_HEIGHT  143
#define MAX_BUFFER			(4096)
#define S5K5CCGX_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define S5K5CCGX_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8 
#define FORMAT_FLAGS_COMPRESSED 0x3
#define DEFUALT_MCLK		26000000
 
#define S5K5CCGX_REGISTER_SIZE 4

#define EXIF_SOFTWARE		""
#define EXIF_MAKE		"Samsung"
#define EXIF_MODEL		"GT-1234"

//#define __JPEG_CAPTURE__ 1

static const struct camdrv_ss_framesize s5k5ccgx_supported_preview_framesize_list[] = {
	{ PREVIEW_SIZE_QCIF,	176,  144 },
	{ PREVIEW_SIZE_QVGA,	320,  240 },
	{ PREVIEW_SIZE_CIF,	352,  288 },
	{ PREVIEW_SIZE_VGA,	640,  480 },
	{ PREVIEW_SIZE_D1,	720,  480 },
#ifndef __JPEG_CAPTURE__
        { PREVIEW_SIZE_1MP,     1280, 1024 },
        { PREVIEW_SIZE_2MP,	1600,  1200 },
        { PREVIEW_SIZE_3MP,	2048,  1536 },
#endif
};


static const struct camdrv_ss_framesize  s5k5ccgx_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA,		640,  480 },
//	{ CAPTURE_SIZE_1MP,		1280, 960 },
//	{ CAPTURE_SIZE_2MP,		1600, 1200 },
	{ CAPTURE_SIZE_3MP,		2048, 1536 },
};
const static struct v4l2_fmtdesc s5k5ccgx_fmts[] = 
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

static const struct v4l2_queryctrl s5k5ccgx_controls[] = {
	{
		.id			= V4L2_CID_CAMERA_FLASH_MODE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Flash",
		.minimum	= FLASH_MODE_OFF,
		.maximum	= (1 << FLASH_MODE_OFF | 1 << FLASH_MODE_AUTO | 1 << FLASH_MODE_ON
                                    | 1 << FLASH_MODE_TORCH_ON),
		.step		= 1,
		.default_value	= FLASH_MODE_AUTO,
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
		.maximum	= (1 << WHITE_BALANCE_AUTO | 1 << WHITE_BALANCE_SUNNY | 1 << WHITE_BALANCE_CLOUDY
						| 1 << WHITE_BALANCE_TUNGSTEN | 1 << WHITE_BALANCE_FLUORESCENT ),
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
		.maximum	= (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_BNW
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
						1 << SCENE_MODE_FIREWORKS | 1 << SCENE_MODE_CANDLE_LIGHT), /*querymenu?*/
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
		.id 		= V4L2_CID_CAMERA_FRAME_RATE,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Framerate control",
		.minimum	= FRAME_RATE_AUTO,
		.maximum	= (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_5 | 1 << FRAME_RATE_7 | 1 << FRAME_RATE_10  | 1 << FRAME_RATE_15
						| 1 << FRAME_RATE_20 | 1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
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

static int camdrv_ss_s5k5ccgx_enum_frameintervals(struct v4l2_subdev *sd,struct v4l2_frmivalenum *fival)
{
	int err = 0;
	int size,i;

	if (fival->index >= 1)  
		return -EINVAL;
		
	CAM_INFO_PRINTK(" %s :  E \n",__func__);

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	for(i = 0; i < ARRAY_SIZE(s5k5ccgx_supported_preview_framesize_list); i++) {
		if((s5k5ccgx_supported_preview_framesize_list[i].width == fival->width) &&
		    (s5k5ccgx_supported_preview_framesize_list[i].height == fival->height))
		{
			size = s5k5ccgx_supported_preview_framesize_list[i].index;
			break;
		}
	}
	if(i == ARRAY_SIZE(s5k5ccgx_supported_preview_framesize_list))
	{
		CAM_ERROR_PRINTK("%s unsupported width = %d and height = %d  \n",__func__,fival->width,fival->height);
		return -EINVAL;
	}

	switch (size) {
	case PREVIEW_SIZE_3MP:
	case PREVIEW_SIZE_2MP:
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

static long camdrv_ss_s5k5ccgx_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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

		default:
			ret = -ENOIOCTLCMD;
			break;
	}

	return ret;
}


static unsigned char pBurstData[MAX_BUFFER];

static int camdrv_ss_s5k5ccgx_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_t reg_buffer[],int num_of_regs,char *name)
{
	struct i2c_msg msg = {client->addr, 0, 0, 0};	
    unsigned short subaddr=0, data_value=0;
	int next_subaddr;
    int i;
	int index = 0;
	int err = 0;
#if	1
	memset(pBurstData, 0, sizeof(pBurstData));
	//CAM_INFO_PRINTK( "%s %s :num_reg=%d  \n",sensor.name, __func__,num_of_regs);
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
                		CAM_ERROR_PRINTK( "[%s: %d] i2c burst write fail\n", __FILE__, __LINE__);	
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
                msleep(data_value);
                break;
            }

            case 0xFCFC:
            case 0x0028:
            case 0x002A:
            default:
            {
				if(S5K5CCGX_REGISTER_SIZE == 4)
				      err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value);
				else if(S5K5CCGX_REGISTER_SIZE == 2)
				{
					unsigned char subaddr8 =0;
					unsigned char data_value8 =0;
					subaddr8 = data_value >> 8;
					data_value8 = data_value & 0xff;
					err = camdrv_ss_i2c_write_2_bytes(client, subaddr8,data_value8);
				}
				
            	if(err < 0)
            	{
            		CAM_ERROR_PRINTK( "%s :i2c transfer failed ! \n", __func__);
            		return -EIO;
            	}
            	break;
            }            
        }
    }
#endif


    return 0;
}


#define AAT_PULS_HI_TIME    1
#define AAT_PULS_LO_TIME    1
#define AAT_LATCH_TIME      500

// AAT1271 flash control driver.
static void camdrv_ss_AAT_flash_write_data(unsigned char count)
{
#if 0
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
#endif

}

static float camdrv_ss_s5k5ccgx_get_exposureTime(struct v4l2_subdev *sd)
{
#if 0

    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0, read_value2 = 0;
    int ShutterSpeed = 0;
	int rows_num_=0;


    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2A14);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value1);   // LSB (0x2A14)
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value2);   // MSB (0x2A16)

    ShutterSpeed = (int)read_value2;
    ShutterSpeed = (ShutterSpeed << 16) | (read_value1 & 0xFFFF);
	   return ((ShutterSpeed * 1000) / 400); // us
#endif
	return -1;
}

 
static int camdrv_ss_s5k5ccgx_get_iso_speed_rate(struct v4l2_subdev *sd)
{
#if 0	
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short read_value = 0;
    int GainValue = 0;
    int isospeedrating = 100;
	int rows_num_=0;

    camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
    camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2A18);
    camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &read_value);

    GainValue = ((read_value * 10) / 256);

    if(GainValue < 19)
    {
        isospeedrating = 50;
    }
    else if(GainValue < 23)
    {
        isospeedrating = 100;
    }
    else if(GainValue < 28)
    {
        isospeedrating = 200;
    }
    else
    {
        isospeedrating = 400;
    }
	   return isospeedrating;
#endif
	/* no implementation yet */
	return -1;
 
}

static int camdrv_ss_s5k5ccgx_get_ae_stable_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
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




static int camdrv_ss_s5k5ccgx_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl, struct camdrv_ss_sensor_cap  sensor)
{ 
#if 0 //BILLA TEMP

    struct camdrv_ss_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short AF_status = 0;
    int err = 0;

    mutex_lock(&af_cancel_op);
	CAM_INFO_PRINTK( "%s: Enter \n", __func__);

    if(!bStartFineSearch)
    {
        err  = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x2D12);
        err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &AF_status);

        if(err < 0)
        {
    		CAM_ERROR_PRINTK( "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);
    		ctrl->value = CAMERA_AF_STATUS_FAIL;
    		goto routine_end;
        }
        
    	if(AF_status & 0x0001)   // Check if AF is in progress
    	{
    	    ctrl->value = CAMERA_AF_STATUS_IN_PROGRESS;
    	}
    	else
    	{
        	if(AF_status & 0x0002) 
        	{
        		CAM_INFO_PRINTK( "%s: AF is success~~~~~~~(Single Search) \n", __func__);

#if 1 // Use Fine Search Algorithm.
        		ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS; // fine search algorithm.
        		bStartFineSearch = true;
#else
        		ctrl->value = CAMERA_AF_STATUS_SUCCESS; // single search algorithm.
#endif
        	}
        	else
        	{
        		CAM_ERROR_PRINTK( "%s: AF is Failure~~~~~~~(Single Search) \n", __func__);
        		ctrl->value = CAMERA_AF_STATUS_FAIL;
        	}
        }
    }
    else // Fine Search
    {
        err  = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002C, 0x7000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002E, 0x1F2F);
        err += camdrv_ss_i2c_read_2_bytes(client, 0x0F12, &AF_status);

        if(err < 0)
        {
    		CAM_ERROR_PRINTK( "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);

    		ctrl->value = CAMERA_AF_STATUS_FAIL;
    		goto routine_end;
        }

    	if((AF_status & 0xFF00) == 0x0000) 
    	{
    		CAM_INFO_PRINTK( "%s: AF is success~~~~~~~(Fine Search) \n", __func__);
    		ctrl->value = CAMERA_AF_STATUS_SUCCESS;
    	}
    	else
    	{
    	    CAM_INFO_PRINTK( "%s: AF is in progress~~~~~~~(Fine Search) \n", __func__);
    		ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS;
    	}
    }

routine_end:
    mutex_unlock(&af_cancel_op);
#endif

    return 0;    
}

static int camdrv_ss_s5k5ccgx_set_touch_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
#if 0 //BILLA TEMP

	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    unsigned short MappedX, MappedY;
	unsigned short FirstWinStartX, FirstWinStartY, SecondWinStartX, SecondWinStartY;
    unsigned int aeX = 0, aeY = 0;
    unsigned int pos = 0;
    int OuterLowerLimit, OuterUpperLimit, InnerLowerLimit, InnerUpperLimit;
	int preview_width, preview_height;
    int err = 0;
	int rows_num_=0;
	

    regs_t ae_weight[METERING_CENTER_NUM_OF_REGS];
	
	CAM_INFO_PRINTK( "%s %s : \n",camdrv_ss_name, __func__);

	

	if(ctrl->value == TOUCH_AF_START)
	{
	    // state->af_info.preview_width, height are preview display size on LCD.
        preview_width = state->af_info.preview_width;
        preview_height = state->af_info.preview_height;

        // Prevent divided-by-zero.
        if(preview_width == 0 || preview_height == 0)
        {
            CAM_ERROR_PRINTK( "%s: Either preview_width or preview_height is zero\n", __func__);
            return -EIO;
        }

        CAM_INFO_PRINTK( "<Touch Pos : [%d, %d]> \n", state->af_info.x, state->af_info.y);

        // Touch position mapping from LCD preview base to Sensor preview base.
        // state->af_info.x, y are LCD based touch position.
        // MappedX, Y are Sensor preview based touch position.
        MappedX = (state->af_info.x * SENSOR_PREVIEW_WIDTH) / preview_width;
        MappedY = (state->af_info.y * SENSOR_PREVIEW_HEIGHT) / preview_height;

        if(MappedX > SENSOR_PREVIEW_WIDTH)
        {
            MappedX = SENSOR_PREVIEW_WIDTH;
        }

        if(MappedY > SENSOR_PREVIEW_HEIGHT)
        {
            MappedY = SENSOR_PREVIEW_HEIGHT;
        }
        
        CAM_INFO_PRINTK( "<Mapped Pos : [%d, %d]> \n", MappedX, MappedY);
        
        // Calculate AF X Position
        OuterLowerLimit = MappedX - (AF_OUTER_WINDOW_WIDTH / 2);
        OuterUpperLimit = MappedX + (AF_OUTER_WINDOW_WIDTH / 2);
        InnerLowerLimit = MappedX - (AF_INNER_WINDOW_WIDTH / 2);
        InnerUpperLimit = MappedX + (AF_INNER_WINDOW_WIDTH / 2);
        
        if(InnerLowerLimit <= 0)
        {
            FirstWinStartX = 0;
            SecondWinStartX = 0;
            
            CAM_ERROR_PRINTK( "%s: X Reached to lower limit : [%d, %d] \n", __func__, MappedX, MappedY);
        }
        else if(OuterLowerLimit <= 0)
        {
            FirstWinStartX = 0;
            SecondWinStartX = MappedX - (AF_INNER_WINDOW_WIDTH / 2);
        }
        else if(InnerUpperLimit >= SENSOR_PREVIEW_WIDTH)
        {
            FirstWinStartX = SENSOR_PREVIEW_WIDTH - AF_OUTER_WINDOW_WIDTH - 1;
            SecondWinStartX = SENSOR_PREVIEW_WIDTH - AF_INNER_WINDOW_WIDTH - 1;;
            
            CAM_ERROR_PRINTK( "%s: X Reached to upper limit : [%d, %d] \n", __func__, MappedX, MappedY);
        }
        else if(OuterUpperLimit >= SENSOR_PREVIEW_WIDTH)
        {
            FirstWinStartX = SENSOR_PREVIEW_WIDTH - AF_OUTER_WINDOW_WIDTH - 1;
            SecondWinStartX = MappedX - (AF_INNER_WINDOW_WIDTH / 2);
        }
        else
        {
            FirstWinStartX = MappedX - (AF_OUTER_WINDOW_WIDTH / 2);
            SecondWinStartX = MappedX - (AF_INNER_WINDOW_WIDTH / 2);
        }

        // Calculate AF Y Position
        OuterLowerLimit = MappedY - (AF_OUTER_WINDOW_HEIGHT / 2);
        OuterUpperLimit = MappedY + (AF_OUTER_WINDOW_HEIGHT / 2);
        InnerLowerLimit = MappedY - (AF_INNER_WINDOW_HEIGHT / 2);
        InnerUpperLimit = MappedY + (AF_INNER_WINDOW_HEIGHT / 2);

        if(InnerLowerLimit <= 0)
        {
            FirstWinStartY = 0;
            SecondWinStartY = 0;
            
            CAM_ERROR_PRINTK( "%s: Y Reached to lower limit : [%d, %d] \n", __func__, MappedX, MappedY);
        }
        else if(OuterLowerLimit <= 0)
        {
            FirstWinStartY = 0;
            SecondWinStartY = MappedY - (AF_INNER_WINDOW_HEIGHT / 2);
        }
        else if(InnerUpperLimit >= SENSOR_PREVIEW_HEIGHT)
        {
            FirstWinStartY = SENSOR_PREVIEW_HEIGHT - AF_OUTER_WINDOW_HEIGHT - 1;
            SecondWinStartY = SENSOR_PREVIEW_HEIGHT - AF_INNER_WINDOW_HEIGHT - 1;;
            
            CAM_ERROR_PRINTK( "%s: Y Reached to upper limit : [%d, %d] \n", __func__, MappedX, MappedY);
        }
        else if(OuterUpperLimit >= SENSOR_PREVIEW_HEIGHT)
        {
            FirstWinStartY = SENSOR_PREVIEW_HEIGHT - AF_OUTER_WINDOW_HEIGHT - 1;
            SecondWinStartY = MappedY - (AF_INNER_WINDOW_HEIGHT / 2);
        }
        else
        {
            FirstWinStartY = MappedY - (AF_OUTER_WINDOW_HEIGHT / 2);
            SecondWinStartY = MappedY - (AF_INNER_WINDOW_HEIGHT / 2);
        }        

        CAM_INFO_PRINTK( "<OuterWin : [%d, %d]> \n", FirstWinStartX, FirstWinStartY);
        CAM_INFO_PRINTK( "<InnerWin : [%d, %d]> \n", SecondWinStartX, SecondWinStartY);

    	FirstWinStartX = (unsigned short)((FirstWinStartX * 1024) / SENSOR_PREVIEW_WIDTH);
    	FirstWinStartY = (unsigned short)((FirstWinStartY * 1024) / SENSOR_PREVIEW_HEIGHT);
        SecondWinStartX = (unsigned short)((SecondWinStartX * 1024) / SENSOR_PREVIEW_WIDTH);
        SecondWinStartY = (unsigned short)((SecondWinStartY * 1024) / SENSOR_PREVIEW_HEIGHT);

        mutex_lock(&af_cancel_op);

        // You don't have to update the Window sizes. 
        // Because the window sizes which will be written to sensor register are same. 
        err  = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x022C);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, FirstWinStartX);         // FirstWinStartX
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, FirstWinStartY);         // FirstWinStartY
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0234);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, SecondWinStartX);        // SecondWinStartX
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, SecondWinStartY);        // SecondWinStartY
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x023C);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001);                 // WindowSizeUpdated

        mutex_unlock(&af_cancel_op);
        
        // Apply Touch AE Weight. (Use center-weighted metering table.)
        memcpy(ae_weight, metering_center_regs, sizeof(metering_center_regs));

        aeX = MappedX / (SENSOR_PREVIEW_WIDTH / 8);
        aeY = MappedY / (SENSOR_PREVIEW_HEIGHT / 8);

        // Find the corresponding index of ae_weight array.
        pos = (((aeY * 8) + aeX) / 2) + 3;  // +3 means {0xFCFC, 0xD000}, {0x0028, 0x7000}, {0x002A, 0x1316}

        if(pos < METERING_CENTER_NUM_OF_REGS)
        {
            if(aeX % 2 == 0)
            {
                ae_weight[pos].value |= 0x0020;  // 0x000F => 0x0020
            }            
            else
            {
                ae_weight[pos].value |= 0x2000;  // 0x0F00 => 0x2000
            }
        }
        
		err += camdrv_ss_i2c_set_data_burst(client, 
		                                  ae_weight, 
		                                  METERING_CENTER_NUM_OF_REGS);

	    CAM_INFO_PRINTK( "%s: Start AF Pos[%d %d]\n", __func__, FirstWinStartX, FirstWinStartY);
    }
    else
    {
        mutex_lock(&af_cancel_op);

        // You don't have to update the Window sizes. 
        // Because the window sizes which will be written to sensor register are same. 
        // These values are Same as the AF Window settings in init settings.            
        err  = camdrv_ss_i2c_write_4_bytes(client, 0xFCFC, 0xD000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0028, 0x7000);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x022C);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0100);                 // FirstWinStartX
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x00E3);                 // FirstWinStartY
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x0234);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x018C);                 // SecondWinStartX
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0166);                 // SecondWinStartY
        err += camdrv_ss_i2c_write_4_bytes(client, 0x002A, 0x023C);
        err += camdrv_ss_i2c_write_4_bytes(client, 0x0F12, 0x0001);                 // WindowSizeUpdated

        mutex_unlock(&af_cancel_op);
        
        // Apply Touch AE Weight.
        // Touch AE restoration. Set to previous metering values.            
    	switch(currentMetering)
    	{
    		case METERING_MATRIX:
    		{
				rows_num_ =	sizeof(*metering_matrix_regs)/sizeof(regs_t);
				if(rows_num_ == 0)
					CAM_ERROR_PRINTK( "%s %s : metering_matrix_regs  NOT supported !!! \n",camdrv_ss_name, __func__);
				else
					err |= camdrv_ss_i2c_set_config_register(client,metering_matrix_regs,rows_num_,"metering_matrix_regs");

				break;
 
            }
            
    		case METERING_CENTER:
    		{
				rows_num_ =	sizeof(*metering_center_regs)/sizeof(regs_t);
				if(rows_num_ == 0)
					CAM_ERROR_PRINTK( "%s %s : metering_center_regs NOT  supported !!! \n",camdrv_ss_name, __func__);
				else
					err |= camdrv_ss_i2c_set_config_register(client,metering_center_regs,rows_num_,"metering_center_regs");

				break;

            }

    		case METERING_SPOT:
    		{
				rows_num_ =	sizeof(*metering_spot_regs)/sizeof(regs_t);
				if(rows_num_ == 0)
					CAM_ERROR_PRINTK( "%s %s : metering_spot_regs NOT  supported !!! \n",camdrv_ss_name, __func__);
				else
					err |= camdrv_ss_i2c_set_config_register(client,metering_spot_regs,rows_num_,"metering_spot_regs");

				break;

            }

    		default:
    		{
   				CAM_ERROR_PRINTK( "%s %s : default  supported !!! \n",camdrv_ss_name, __func__);
		
            }			
    	}
	}


#endif

	return 0;
}


static int camdrv_ss_s5k5ccgx_get_light_condition(struct v4l2_subdev *sd, int *Result)
{
#if 0
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
#endif

	return 0;
}


static bool camdrv_ss_s5k5ccgx_check_flash_needed(struct v4l2_subdev *sd)
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


static int camdrv_ss_s5k5ccgx_AAT_flash_control(struct v4l2_subdev *sd, int control_mode)
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
static struct regulator *VCAM_IO_1_8_V;  //LDO_HV9
static struct regulator *VCAM_A_2_8_V;   //LDO_CAM12/12/2011
#define CAM_CORE_EN                  42
//#define CAM_AF_EN     121

//main cam 
#define CAM0_RESET    33
#define CAM0_STNBY    111

//sub cam
#define CAM1_RESET    23
#define CAM1_STNBY    34

#define SENSOR_0_CLK			"dig_ch0_clk"    //(common)
#define SENSOR_0_CLK_FREQ		(26000000) //@HW, need to check how fast this meaning.




static int camdrv_ss_s5k5ccgx_sensor_power(int on)
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
	
	//ret = regulator_disable(VCAM_A_2_8_V);

	VCAM_IO_1_8_V = regulator_get(NULL,"hv9");
	if(IS_ERR(VCAM_IO_1_8_V))
	{
		CAM_ERROR_PRINTK("can not get VCAM_IO_1.8V\n");
		return -1;
	}	
		
	//ret = regulator_disable(VCAM_IO_1_8_V);

	
	gpio_request(CAM_CORE_EN, "cam_1_2v");
	gpio_direction_output(CAM_CORE_EN,0); 

	//gpio_request(CAM_AF_EN, "cam_af_2_8v");
	//gpio_direction_output(CAM_AF_EN,0); 
	
	CAM_INFO_PRINTK("set cam_rst cam_stnby  to low\n");
	gpio_request(CAM0_RESET, "cam0_rst");
	gpio_direction_output(CAM0_RESET,0);
	
	gpio_request(CAM0_STNBY, "cam0_stnby");
	gpio_direction_output(CAM0_STNBY,0);
		
	gpio_request(CAM1_RESET, "cam1_rst");
	gpio_direction_output(CAM1_RESET,0);

	gpio_request(CAM1_STNBY, "cam1_stnby");
	gpio_direction_output(CAM1_STNBY,0);

//	value = ioread32(padctl_base + PADCTRLREG_DCLK1_OFFSET) & (~PADCTRLREG_DCLK1_PINSEL_DCLK1_MASK);
//		iowrite32(value, padctl_base + PADCTRLREG_DCLK1_OFFSET);


	if(on)
	{
		CAM_INFO_PRINTK("power on the sensor\n");
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
							PI_OPP_NORMAL)) {
			CAM_ERROR_PRINTK("%s:failed to update dfs request for unicam\n", __func__);
			return -1;
		}


		value = clk_enable(axi_clk);
		if (value) {
			CAM_ERROR_PRINTK("%s:failed to enable csi2 axi clock\n", __func__);
			return -1;
		}

		msleep(100);
		CAM_INFO_PRINTK("power on the sensor's power supply\n"); //@HW

	
		gpio_request(CAM_CORE_EN, "cam_1_2v");
		gpio_set_value(CAM_CORE_EN,1); 
		msleep(1);
	
		regulator_set_voltage(VCAM_A_2_8_V,2800000,2800000);
		
		regulator_enable(VCAM_A_2_8_V);
		msleep(1);
		
		regulator_set_voltage(VCAM_IO_1_8_V,1800000,1800000);
		regulator_enable(VCAM_IO_1_8_V);
	
		msleep(1);	
		
		gpio_set_value(CAM1_STNBY,1);
		msleep(5);
	
		value = clk_enable(clock);
		if (value) {
			CAM_ERROR_PRINTK("%s: failed to enable clock %s\n", __func__,
				SENSOR_0_CLK);
			return -1;
		}
		CAM_INFO_PRINTK("enable camera clock\n");
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			CAM_ERROR_PRINTK("%s: failed to set the clock %s to freq %d\n",
					__func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}
		CAM_INFO_PRINTK("set rate\n");
        msleep(1);

		gpio_set_value(CAM1_RESET,1);
 		msleep(6);

		gpio_set_value(CAM1_STNBY,0);
		msleep(1);
		
		gpio_set_value(CAM0_STNBY,1);
		msleep(1);

		gpio_set_value(CAM0_RESET,1);
		msleep(1);
	
		CAM_INFO_PRINTK("set cam rst to high\n");
		msleep(50);
		#ifdef CONFIG_FLASH_ENABLE
				CAM_INFO_PRINTK( "3MP camera S5K5CCGX loaded. HWREV is 0x%x\n", HWREV);
			
				// FLASH
				if(HWREV >= 0x03)
				{					 
					ret = gpio_request(GPIO_CAM_FLASH_SET_NEW, "GPJ0[5]");
					if(ret)
					{
						CAM_ERROR_PRINTK( "gpio_request(GPJ0[5]) failed!! \n");
						return 0;
					}
				}
				else
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
			
				if(HWREV >= 0x03)
				{					 
					gpio_direction_output(GPIO_CAM_FLASH_SET_NEW, 0);
				}
				else
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
		gpio_set_value(CAM0_STNBY,0);
		msleep(1);		
		gpio_set_value(CAM1_RESET,0);
		msleep(1);
		
		clk_disable(clock);
		clk_disable(axi_clk);

		gpio_set_value(CAM1_STNBY,0);
        msleep(1);

		/* enable power down gpio */

	//	regulator_set_voltage(VCAM_IO_1_8_V,0,0);
		regulator_disable(VCAM_IO_1_8_V);
		gpio_set_value(CAM_CORE_EN, 0);
	//	regulator_set_voltage(VCAM_A_2_8_V,0,0);
		regulator_disable(VCAM_A_2_8_V);
		
	#ifdef CONFIG_FLASH_ENABLE
			// FLASH
			camdrv_ss_AAT_flash_control(sd, FLASH_CONTROL_OFF);
			
			if(HWREV >= 0x03)
			{					 
				gpio_free(GPIO_CAM_FLASH_SET_NEW);
			}
			else
			{
				gpio_free(GPIO_CAM_FLASH_SET);
			}
			gpio_free(GPIO_CAM_FLASH_EN);
		#endif // CONFIG_FLASH_ENABLE
	    if (pi_mgr_dfs_request_update(&unicam_dfs_node,
					      PI_MGR_DFS_MIN_VALUE)) {
			CAM_ERROR_PRINTK("%s: failed to update dfs request for unicam\n",
				 __func__);
		}
		CAM_INFO_PRINTK("rhea_camera_power off succ \n");
	}

	
	return 0;
	
}


int camdrv_ss_s5k5ccgx_get_sensor_param_for_exif(
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

	exposureTime = camdrv_ss_s5k5ccgx_get_exposureTime(sd);
	num = (int)(exposureTime * 100);
	if (num > 0) 
	{
	    snprintf(str, 19, "%d/%d", num,100);
		strcpy(exif_param->exposureTime, str);
	} 
	else 
	{
		strcpy(exif_param->exposureTime, "");
	}
	
	CAM_INFO_PRINTK("%s : exposure time =  %s \n",__func__,exif_param->exposureTime);

	num = camdrv_ss_s5k5ccgx_get_iso_speed_rate(sd);
	if (num > 0) {
		sprintf(str, "%d", num);
		strcpy(exif_param->isoSpeedRating, str);
	} else {
		strcpy(exif_param->isoSpeedRating, "");
	}

	/* sRGB mandatory field! */
	strcpy(exif_param->colorSpaceInfo,	"1");

	strcpy(exif_param->contrast,		"0");
	strcpy(exif_param->saturation,		"0");
	strcpy(exif_param->sharpness,		"0");

	strcpy(exif_param->FNumber,		"");
	strcpy(exif_param->exposureProgram,	"");
	strcpy(exif_param->shutterSpeed,	exif_param->exposureTime);
	strcpy(exif_param->aperture,		"");
	strcpy(exif_param->brightness,		"");
	strcpy(exif_param->exposureBias,	"");
	strcpy(exif_param->maxLensAperture,	"");
	strcpy(exif_param->flash,		"");
	strcpy(exif_param->lensFocalLength,	"");
	strcpy(exif_param->userComments,	"");
	ret = 0;

	return ret;

}


bool camdrv_ss_sensor_init_main(bool bOn, struct camdrv_ss_sensor_cap *sensor)
{
	
	strcpy(sensor->name,S5K5CCGX_NAME);
	sensor->supported_preview_framesize_list  = s5k5ccgx_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(s5k5ccgx_supported_preview_framesize_list);
	
	sensor->supported_capture_framesize_list  =  s5k5ccgx_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(s5k5ccgx_supported_capture_framesize_list);
	
	sensor->fmts 				   = s5k5ccgx_fmts;
	sensor->rows_num_fmts		   =ARRAY_SIZE(s5k5ccgx_fmts);


	sensor->controls				   =s5k5ccgx_controls;
	sensor->rows_num_controls	      =ARRAY_SIZE(s5k5ccgx_controls);
	
	sensor->default_pix_fmt 				   = S5K5CCGX_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt			   = S5K5CCGX_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size 		  			 = S5K5CCGX_REGISTER_SIZE;
	sensor->skip_frames 					 = 0;

	/* sensor dependent functions */
	
/* mandatory*/
	sensor->thumbnail_ioctl			       = camdrv_ss_s5k5ccgx_ss_ioctl;
	sensor->enum_frameintervals			   = camdrv_ss_s5k5ccgx_enum_frameintervals;
/*optional*/
	sensor->get_ae_stable_status      =  camdrv_ss_s5k5ccgx_get_ae_stable_status;
	sensor->get_auto_focus_status     = camdrv_ss_s5k5ccgx_get_auto_focus_status;
//	sensor->set_touch_auto_focus      = camdrv_ss_s5k5ccgx_set_touch_auto_focus;
	/* sensor->flash_control		= camdrv_ss_s5k5ccgx_AAT_flash_control; */
	sensor->i2c_set_data_burst   	   = camdrv_ss_s5k5ccgx_i2c_set_data_burst;
	sensor->check_flash_needed   	   = camdrv_ss_s5k5ccgx_check_flash_needed;
//	sensor->get_light_condition   = camdrv_ss_s5k5ccgx_get_light_condition;
	
		  
	//power up
	sensor->sensor_power = camdrv_ss_s5k5ccgx_sensor_power;		  

	sensor->get_exif_sensor_info =
		camdrv_ss_s5k5ccgx_get_sensor_param_for_exif;

	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */
	
	sensor->init_regs						  = s5k5ccgx_init_regs;
	sensor->rows_num_init_regs				  = ARRAY_SIZE(s5k5ccgx_init_regs);
	
	sensor->preview_camera_regs 			  = s5k5ccgx_preview_camera_regs;
	sensor->rows_num_preview_camera_regs 	  = ARRAY_SIZE(s5k5ccgx_preview_camera_regs);
			
	/*snapshot mode*/
	sensor->snapshot_normal_regs			  =	s5k5ccgx_snapshot_normal_regs;
	sensor->rows_num_snapshot_normal_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_normal_regs);

	sensor->snapshot_lowlight_regs			  =	s5k5ccgx_snapshot_lowlight_regs;
	sensor->rows_num_snapshot_lowlight_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_lowlight_regs);

	sensor->snapshot_highlight_regs			  =	s5k5ccgx_snapshot_highlight_regs;
	sensor->rows_num_snapshot_highlight_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_highlight_regs);

	sensor->snapshot_nightmode_regs			  =	s5k5ccgx_snapshot_nightmode_regs;
	sensor->rows_num_snapshot_nightmode_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_nightmode_regs);

	sensor->snapshot_flash_on_regs			  =	s5k5ccgx_snapshot_flash_on_regs;
	sensor->rows_num_snapshot_flash_on_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_flash_on_regs);

	sensor->snapshot_af_preflash_on_regs			  =	s5k5ccgx_snapshot_af_preflash_on_regs;
	sensor->rows_num_snapshot_af_preflash_on_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_af_preflash_on_regs);

	sensor->snapshot_af_preflash_off_regs			  =	s5k5ccgx_snapshot_af_preflash_off_regs;
	sensor->rows_num_snapshot_af_preflash_off_regs	  = ARRAY_SIZE(s5k5ccgx_snapshot_af_preflash_off_regs);


	sensor->focus_mode_macro_regs			  =	s5k5ccgx_af_macro_mode_regs;
	sensor->rows_num_focus_mode_macro_regs	  = ARRAY_SIZE(s5k5ccgx_af_macro_mode_regs);

	sensor->focus_mode_off_regs			  =	s5k5ccgx_af_normal_mode_regs;
	sensor->rows_num_focus_mode_off_regs	  = ARRAY_SIZE(s5k5ccgx_af_normal_mode_regs);



	/*effect*/
	sensor->effect_normal_regs			      =	s5k5ccgx_effect_normal_regs;
	sensor->rows_num_effect_normal_regs      = ARRAY_SIZE(s5k5ccgx_effect_normal_regs);
	
	sensor->effect_negative_regs		      =	s5k5ccgx_effect_negative_regs;
	sensor->rows_num_effect_negative_regs	 = ARRAY_SIZE(s5k5ccgx_effect_negative_regs);
	
	sensor->effect_sepia_regs			      =	s5k5ccgx_effect_sepia_regs;
	sensor->rows_num_effect_sepia_regs	  	  = ARRAY_SIZE(s5k5ccgx_effect_sepia_regs);
	
	sensor->effect_mono_regs			      =	s5k5ccgx_effect_mono_regs;
	sensor->rows_num_effect_mono_regs	      = ARRAY_SIZE(s5k5ccgx_effect_mono_regs);

	sensor->effect_aqua_regs				  =	s5k5ccgx_effect_aqua_regs;
	sensor->rows_num_effect_aqua_regs	  	  = ARRAY_SIZE(s5k5ccgx_effect_aqua_regs);
	
	sensor->effect_sharpen_regs 		      =	s5k5ccgx_effect_sharpen_regs;
	sensor->rows_num_effect_sharpen_regs     = ARRAY_SIZE(s5k5ccgx_effect_sharpen_regs);
	
	sensor->effect_solarization_regs		   = s5k5ccgx_effect_solarization_regs;
	sensor->rows_num_effect_solarization_regs = ARRAY_SIZE(s5k5ccgx_effect_solarization_regs);
	
	sensor->effect_black_white_regs 	       =	s5k5ccgx_effect_black_white_regs;
	sensor->rows_num_effect_black_white_regs  = ARRAY_SIZE(s5k5ccgx_effect_black_white_regs);
	

	/*wb*/
	sensor->wb_auto_regs				  =	s5k5ccgx_wb_auto_regs;
	sensor->rows_num_wb_auto_regs	  	  = ARRAY_SIZE(s5k5ccgx_wb_auto_regs);

	sensor->wb_sunny_regs				 =	s5k5ccgx_wb_auto_regs;
	sensor->rows_num_wb_sunny_regs	  	 = ARRAY_SIZE(s5k5ccgx_wb_auto_regs);
	
	sensor->wb_cloudy_regs				 =	s5k5ccgx_wb_cloudy_regs;
	sensor->rows_num_wb_cloudy_regs	 = ARRAY_SIZE(s5k5ccgx_wb_cloudy_regs);
	
	sensor->wb_tungsten_regs			 =	s5k5ccgx_wb_tungsten_regs;
	sensor->rows_num_wb_tungsten_regs	 = ARRAY_SIZE(s5k5ccgx_wb_tungsten_regs);

	sensor->wb_fluorescent_regs 		  =	s5k5ccgx_wb_fluorescent_regs;
	sensor->rows_num_wb_fluorescent_regs  = ARRAY_SIZE(s5k5ccgx_wb_fluorescent_regs);

	sensor->wb_cwf_regs 		  =	s5k5ccgx_wb_cwf_regs;
	sensor->rows_num_wb_cwf_regs  = ARRAY_SIZE(s5k5ccgx_wb_cwf_regs);


	/*metering*/
	sensor->metering_matrix_regs		  =	s5k5ccgx_metering_matrix_regs;
	sensor->metering_matrix_regs	  	  = ARRAY_SIZE(s5k5ccgx_metering_matrix_regs);

	sensor->metering_center_regs		  =	s5k5ccgx_metering_center_regs;
	sensor->metering_center_regs	  	  = ARRAY_SIZE(s5k5ccgx_metering_center_regs);

	sensor->metering_spot_regs			  =	  s5k5ccgx_metering_spot_regs;
	sensor->metering_spot_regs	  		  = ARRAY_SIZE(s5k5ccgx_metering_spot_regs);
	
	/*EV*/
	sensor->ev_minus_4_regs 			 =	s5k5ccgx_ev_minus_4_regs;
	sensor->rows_num_ev_minus_4_regs	 = ARRAY_SIZE(s5k5ccgx_ev_minus_4_regs);

	sensor->ev_minus_3_regs 			 =	s5k5ccgx_ev_minus_3_regs;
	sensor->rows_num_ev_minus_3_regs	 = ARRAY_SIZE(s5k5ccgx_ev_minus_3_regs);

	sensor->ev_minus_2_regs 			 =	s5k5ccgx_ev_minus_2_regs;
	sensor->rows_num_ev_minus_2_regs	  = ARRAY_SIZE(s5k5ccgx_ev_minus_2_regs);

	sensor->ev_minus_1_regs 			 =	s5k5ccgx_ev_minus_1_regs;
	sensor->rows_num_ev_minus_1_regs	 = ARRAY_SIZE(s5k5ccgx_ev_minus_1_regs);

	sensor->ev_default_regs 			 =	s5k5ccgx_ev_default_regs;
	sensor->rows_num_ev_default_regs	 = ARRAY_SIZE(s5k5ccgx_ev_default_regs);

	sensor->ev_plus_1_regs				 =	s5k5ccgx_ev_plus_1_regs;
	sensor->rows_num_ev_plus_1_regs	 = ARRAY_SIZE(s5k5ccgx_ev_plus_1_regs);

	sensor->ev_plus_2_regs				 =	s5k5ccgx_ev_plus_2_regs;
	sensor->rows_num_ev_plus_2_regs	 = ARRAY_SIZE(s5k5ccgx_ev_plus_2_regs);

	sensor->ev_plus_3_regs				 =	s5k5ccgx_ev_plus_3_regs;
	sensor->rows_num_ev_plus_3_regs	 = ARRAY_SIZE(s5k5ccgx_ev_plus_3_regs);

	sensor->ev_plus_4_regs				 =	s5k5ccgx_ev_plus_4_regs;
	sensor->rows_num_ev_plus_4_regs	 = ARRAY_SIZE(s5k5ccgx_ev_plus_4_regs);

	
	/*contrast*/
	sensor->contrast_minus_2_regs		 	 =	s5k5ccgx_contrast_minus_2_regs;
	sensor->rows_num_contrast_minus_2_regs	 = ARRAY_SIZE(s5k5ccgx_contrast_minus_2_regs);

	sensor->contrast_minus_1_regs		     =	s5k5ccgx_contrast_minus_1_regs;
	sensor->rows_num_contrast_minus_1_regs	 = ARRAY_SIZE(s5k5ccgx_contrast_minus_1_regs);
  
	sensor->contrast_default_regs			 =	s5k5ccgx_contrast_default_regs;
	sensor->rows_num_contrast_default_regs  = ARRAY_SIZE(s5k5ccgx_contrast_default_regs);

	sensor->contrast_plus_1_regs			 =	s5k5ccgx_contrast_plus_1_regs;
	sensor->rows_num_contrast_plus_1_regs	 = ARRAY_SIZE(s5k5ccgx_contrast_plus_1_regs);

	sensor->contrast_plus_2_regs			 =	s5k5ccgx_contrast_plus_2_regs;
	sensor->rows_num_contrast_plus_2_regs	 = ARRAY_SIZE(s5k5ccgx_contrast_plus_2_regs);
	
	/*sharpness*/ 
	sensor->sharpness_minus_3_regs		     =	s5k5ccgx_sharpness_minus_3_regs;
	sensor->rows_num_sharpness_minus_3_regs= ARRAY_SIZE(s5k5ccgx_sharpness_minus_3_regs);

	sensor->sharpness_minus_2_regs		     =	s5k5ccgx_sharpness_minus_2_regs;
	sensor->rows_num_sharpness_minus_2_regs= ARRAY_SIZE(s5k5ccgx_sharpness_minus_2_regs);

	sensor->sharpness_minus_1_regs		 	 =	s5k5ccgx_sharpness_minus_1_regs;
	sensor->rows_num_sharpness_minus_1_regs = ARRAY_SIZE(s5k5ccgx_sharpness_minus_1_regs);

	sensor->sharpness_default_regs		 	 =	s5k5ccgx_sharpness_default_regs;
	sensor->rows_num_sharpness_default_regs  = ARRAY_SIZE(s5k5ccgx_sharpness_default_regs);

	sensor->sharpness_plus_1_regs		     =	s5k5ccgx_sharpness_plus_1_regs;
	sensor->rows_num_sharpness_plus_1_regs	 =	ARRAY_SIZE(s5k5ccgx_sharpness_plus_1_regs);

	sensor->sharpness_plus_2_regs		     =	s5k5ccgx_sharpness_plus_2_regs;
	sensor->rows_num_sharpness_plus_2_regs	 =	ARRAY_SIZE(s5k5ccgx_sharpness_plus_2_regs);

	sensor->sharpness_plus_3_regs		     =	s5k5ccgx_sharpness_plus_3_regs;
	sensor->rows_num_sharpness_plus_3_regs	 =	ARRAY_SIZE(s5k5ccgx_sharpness_plus_3_regs);

		
	/*saturation*/
	sensor->saturation_minus_2_regs 	      =	s5k5ccgx_saturation_minus_2_regs;
	sensor->rows_num_saturation_minus_2_regs = ARRAY_SIZE(s5k5ccgx_saturation_minus_2_regs);

	sensor->saturation_minus_1_regs 	 	  =	s5k5ccgx_saturation_minus_1_regs;
	sensor->rows_num_saturation_minus_1_regs = ARRAY_SIZE(s5k5ccgx_saturation_minus_1_regs);

	sensor->saturation_default_regs 	      =	s5k5ccgx_saturation_default_regs;
	sensor->rows_num_saturation_default_regs  = ARRAY_SIZE(s5k5ccgx_saturation_default_regs);

	sensor->saturation_plus_1_regs		       =	s5k5ccgx_saturation_plus_1_regs;
	sensor->rows_num_saturation_plus_1_regs	= ARRAY_SIZE(s5k5ccgx_saturation_plus_1_regs);

	sensor->saturation_plus_2_regs		       =	s5k5ccgx_saturation_plus_2_regs;
	sensor->rows_num_saturation_plus_2_regs   = ARRAY_SIZE(s5k5ccgx_saturation_plus_2_regs);

	
	/*zoom*/
	sensor->zoom_00_regs					 =	s5k5ccgx_zoom_00_regs;
	sensor->rows_num_zoom_00_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_00_regs);

	sensor->zoom_01_regs					 =	s5k5ccgx_zoom_01_regs;
	sensor->rows_num_zoom_01_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_01_regs);

	sensor->zoom_02_regs					 =	s5k5ccgx_zoom_02_regs;
	sensor->rows_num_zoom_02_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_02_regs);

	sensor->zoom_03_regs					 =	s5k5ccgx_zoom_03_regs;
	sensor->rows_num_zoom_03_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_03_regs);

	sensor->zoom_04_regs					 =	s5k5ccgx_zoom_04_regs;
	sensor->rows_num_zoom_04_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_04_regs);

	sensor->zoom_05_regs					 =	s5k5ccgx_zoom_05_regs;
	sensor->rows_num_zoom_05_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_05_regs);

	sensor->zoom_06_regs					 =	s5k5ccgx_zoom_06_regs;
	sensor->rows_num_zoom_06_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_06_regs);

	sensor->zoom_07_regs					 =	s5k5ccgx_zoom_07_regs;
	sensor->rows_num_zoom_07_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_07_regs);

	sensor->zoom_08_regs					 =	s5k5ccgx_zoom_08_regs;
	sensor->rows_num_zoom_08_regs	  		  = ARRAY_SIZE(s5k5ccgx_zoom_08_regs);

	
	/*scene mode*/
	sensor->scene_none_regs 			 		=	s5k5ccgx_scene_none_regs;
	sensor->rows_num_scene_none_regs	  		 = ARRAY_SIZE(s5k5ccgx_scene_none_regs);

	sensor->scene_portrait_regs 		 		=	s5k5ccgx_scene_portrait_regs;
	sensor->rows_num_scene_portrait_regs	  	= ARRAY_SIZE(s5k5ccgx_scene_portrait_regs);

	sensor->scene_nightshot_regs			   =	s5k5ccgx_scene_nightshot_regs;
	sensor->rows_num_scene_nightshot_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_nightshot_regs);

	sensor->scene_backlight_regs			  =	s5k5ccgx_scene_backlight_regs;
	sensor->rows_num_scene_backlight_regs	   = ARRAY_SIZE(s5k5ccgx_scene_backlight_regs);

	sensor->scene_landscape_regs			   =	s5k5ccgx_scene_landscape_regs;
	sensor->rows_num_scene_landscape_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_landscape_regs);

	sensor->scene_sports_regs			      =	s5k5ccgx_scene_sports_regs;
	sensor->rows_num_scene_sports_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_sports_regs);

	sensor->scene_party_indoor_regs 	 	  =	s5k5ccgx_scene_party_indoor_regs;
	sensor->rows_num_scene_party_indoor_regs  = ARRAY_SIZE(s5k5ccgx_scene_party_indoor_regs);

	sensor->scene_beach_snow_regs				 =	s5k5ccgx_scene_beach_snow_regs;
	sensor->rows_num_scene_beach_snow_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_beach_snow_regs);

	sensor->scene_sunset_regs			 		 =	s5k5ccgx_scene_sunset_regs;
	sensor->rows_num_scene_sunset_regs	  		  = ARRAY_SIZE(s5k5ccgx_scene_sunset_regs);

	sensor->scene_duskdawn_regs 				 =	s5k5ccgx_scene_duskdawn_regs;
	sensor->rows_num_scene_duskdawn_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_duskdawn_regs);

	sensor->scene_fall_color_regs				 =	s5k5ccgx_scene_fall_color_regs;
	sensor->rows_num_scene_fall_color_regs	  	  = ARRAY_SIZE(s5k5ccgx_scene_fall_color_regs);

	sensor->scene_fireworks_regs				 =	s5k5ccgx_scene_fireworks_regs;
	sensor->rows_num_scene_fireworks_regs	  	  = ARRAY_SIZE(s5k5ccgx_scene_fireworks_regs);
	
	sensor->scene_candle_light_regs 	 		=	s5k5ccgx_scene_candle_light_regs;
	sensor->rows_num_scene_candle_light_regs	= ARRAY_SIZE(s5k5ccgx_scene_candle_light_regs);

	sensor->scene_text_regs			   =	s5k5ccgx_scene_text_regs;
	sensor->rows_num_scene_text_regs	  	 = ARRAY_SIZE(s5k5ccgx_scene_text_regs);

		
	/*fps*/
	sensor->fps_auto_regs				 =	s5k5ccgx_fps_auto_regs;
	sensor->rows_num_fps_auto_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_auto_regs);

	sensor->fps_5_regs					 =	s5k5ccgx_fps_5_regs;
	sensor->rows_num_fps_5_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_5_regs);

	sensor->fps_7_regs					 =	s5k5ccgx_fps_7_regs;
	sensor->rows_num_fps_7_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_7_regs);

	sensor->fps_10_regs 				 =	s5k5ccgx_fps_10_regs;
	sensor->rows_num_fps_10_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_10_regs);

	sensor->fps_15_regs 				 =	s5k5ccgx_fps_15_regs;
	sensor->rows_num_fps_15_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_15_regs);

	sensor->fps_20_regs 				 =	s5k5ccgx_fps_20_regs;
	sensor->rows_num_fps_20_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_20_regs);

	sensor->fps_25_regs 				 =	s5k5ccgx_fps_25_regs;
	sensor->rows_num_fps_25_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_25_regs);

	sensor->fps_30_regs 				 =	s5k5ccgx_fps_30_regs;
	sensor->rows_num_fps_30_regs 		  = ARRAY_SIZE(s5k5ccgx_fps_30_regs);
	
	sensor->fps_60_regs 				 =	s5k5ccgx_fps_60_regs;
	sensor->rows_num_fps_60_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_60_regs);

	sensor->fps_120_regs 				 =	s5k5ccgx_fps_120_regs;
	sensor->rows_num_fps_120_regs	  		  = ARRAY_SIZE(s5k5ccgx_fps_120_regs);
	

	
	/*quality*/
	sensor->quality_superfine_regs			 =	s5k5ccgx_quality_superfine_regs;
	sensor->rows_num_quality_superfine_regs	  = ARRAY_SIZE(s5k5ccgx_quality_superfine_regs);

	sensor->quality_fine_regs			 =	s5k5ccgx_quality_fine_regs;
	sensor->rows_num_quality_fine_regs	  = ARRAY_SIZE(s5k5ccgx_quality_fine_regs);

	sensor->quality_normal_regs 		   =	s5k5ccgx_quality_normal_regs;
	sensor->rows_num_quality_normal_regs  = ARRAY_SIZE(s5k5ccgx_quality_normal_regs);

	sensor->quality_economy_regs			 =	s5k5ccgx_quality_economy_regs;
	sensor->rows_num_quality_economy_regs   = ARRAY_SIZE(s5k5ccgx_quality_economy_regs);

	
	/*preview size */
	sensor->preview_size_176x144_regs	        =	s5k5ccgx_preview_size_176x144_regs;
	sensor->rows_num_preview_size_176x144_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_176x144_regs);

	sensor->preview_size_320x240_regs	         =	s5k5ccgx_preview_size_320x240_regs; 
	sensor->rows_num_preview_size_320x240_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_320x240_regs);

	sensor->preview_size_352x288_regs	          =	s5k5ccgx_preview_size_352x288_regs; 
	sensor->rows_num_preview_size_352x288_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_352x288_regs);

	sensor->preview_size_640x480_regs	          =	s5k5ccgx_preview_size_640x480_regs; 
	sensor->rows_num_preview_size_640x480_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_640x480_regs);

	sensor->preview_size_704x576_regs	 		=	s5k5ccgx_preview_size_704x576_regs; 
	sensor->rows_num_preview_size_704x576_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_704x576_regs);

	sensor->preview_size_720x480_regs	 		=	s5k5ccgx_preview_size_720x480_regs; 
	sensor->rows_num_preview_size_720x480_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_720x480_regs);
	
	sensor->preview_size_800x480_regs	        =	s5k5ccgx_preview_size_800x480_regs;
	sensor->rows_num_preview_size_800x480_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_800x480_regs);

	sensor->preview_size_800x600_regs	        =	s5k5ccgx_preview_size_800x600_regs;
	sensor->rows_num_preview_size_800x600_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_800x600_regs);

	sensor->preview_size_1024x600_regs	         =	s5k5ccgx_preview_size_1024x600_regs; 
	sensor->rows_num_preview_size_1024x600_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_1024x600_regs);

	sensor->preview_size_1024x768_regs	          =	s5k5ccgx_preview_size_1024x768_regs; 
	sensor->rows_num_preview_size_1024x768_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_1024x768_regs);

	sensor->preview_size_1280x960_regs	          =	s5k5ccgx_preview_size_1280x960_regs; 
	sensor->rows_num_preview_size_1280x960_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_1280x960_regs);

	sensor->preview_size_1600x960_regs	 		=	s5k5ccgx_preview_size_1600x960_regs; 
	sensor->rows_num_preview_size_1600x960_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_1600x960_regs);

	sensor->preview_size_1600x1200_regs	 		=	s5k5ccgx_preview_size_1600x1200_regs; 
	sensor->rows_num_preview_size_1600x1200_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_1600x1200_regs);

	sensor->preview_size_2048x1232_regs	        =	s5k5ccgx_preview_size_2048x1232_regs;
	sensor->rows_num_preview_size_2048x1232_regs	 = ARRAY_SIZE(s5k5ccgx_preview_size_2048x1232_regs);

	sensor->preview_size_2048x1536_regs	         =	s5k5ccgx_preview_size_2048x1536_regs; 
	sensor->rows_num_preview_size_2048x1536_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_2048x1536_regs);

	sensor->preview_size_2560x1920_regs	          =	s5k5ccgx_preview_size_2560x1920_regs; 
	sensor->rows_num_preview_size_2560x1920_regs	  = ARRAY_SIZE(s5k5ccgx_preview_size_2560x1920_regs);
	
	
	/*Capture size */
	sensor->capture_size_640x480_regs	 		=	s5k5ccgx_capture_size_640x480_regs;
	sensor->rows_num_capture_size_640x480_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_640x480_regs);

	sensor->capture_size_720x480_regs  			=	s5k5ccgx_capture_size_720x480_regs; 
	sensor->rows_num_capture_size_720x480_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_720x480_regs);

	sensor->capture_size_800x480_regs	 		=	s5k5ccgx_capture_size_800x480_regs;
	sensor->rows_num_capture_size_800x480_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_800x480_regs);

	sensor->capture_size_800x486_regs	 		=	s5k5ccgx_capture_size_800x486_regs;
	sensor->rows_num_capture_size_800x486_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_800x486_regs);

	sensor->capture_size_800x600_regs  			=	s5k5ccgx_capture_size_800x600_regs; 
	sensor->rows_num_capture_size_800x600_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_800x600_regs);

    	sensor->capture_size_1024x600_regs	 		=	s5k5ccgx_capture_size_1024x600_regs;
	sensor->rows_num_capture_size_1024x600_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_1024x600_regs);

	sensor->capture_size_1024x768_regs  			=	s5k5ccgx_capture_size_1024x768_regs; 
	sensor->rows_num_capture_size_1024x768_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_1024x768_regs);

	sensor->capture_size_1280x960_regs  			=	s5k5ccgx_capture_size_1280x960_regs; 
	sensor->rows_num_capture_size_1280x960_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_1280x960_regs);

    sensor->capture_size_1600x960_regs	 		=	s5k5ccgx_capture_size_1600x960_regs;
	sensor->rows_num_capture_size_1600x960_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_1600x960_regs);

	sensor->capture_size_1600x1200_regs  			=	s5k5ccgx_capture_size_1600x1200_regs; 
	sensor->rows_num_capture_size_1600x1200_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_1600x1200_regs);

	sensor->capture_size_2048x1232_regs  			=	s5k5ccgx_capture_size_2048x1232_regs; 
	sensor->rows_num_capture_size_2048x1232_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_2048x1232_regs);

	sensor->capture_size_2048x1536_regs  			=	s5k5ccgx_capture_size_2048x1536_regs; 
	sensor->rows_num_capture_size_2048x1536_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_2048x1536_regs);

	sensor->capture_size_2560x1536_regs  			=	s5k5ccgx_capture_size_2560x1536_regs; 
	sensor->rows_num_capture_size_2560x1536_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_2560x1536_regs);

	sensor->capture_size_2560x1920_regs  			=	s5k5ccgx_capture_size_2560x1920_regs; 
	sensor->rows_num_capture_size_2560x1920_regs	 = ARRAY_SIZE(s5k5ccgx_capture_size_2560x1920_regs);

	
	/*pattern*/
	sensor->pattern_on_regs 			  = s5k5ccgx_pattern_on_regs;
	sensor->rows_num_pattern_on_regs	  = ARRAY_SIZE(s5k5ccgx_pattern_on_regs);
	
	sensor->pattern_off_regs			  = s5k5ccgx_pattern_off_regs;
	sensor->rows_num_pattern_off_regs	  = ARRAY_SIZE(s5k5ccgx_pattern_off_regs);

	/*AE*/
	sensor->ae_lock_regs			  = s5k5ccgx_ae_lock_regs;
	sensor->rows_num_ae_lock_regs	  = ARRAY_SIZE(s5k5ccgx_ae_lock_regs);



	sensor->ae_unlock_regs			  = s5k5ccgx_ae_unlock_regs;
	sensor->rows_num_ae_unlock_regs	  = ARRAY_SIZE(s5k5ccgx_ae_unlock_regs);



	/*AWB*/

	sensor->awb_lock_regs			  = s5k5ccgx_awb_lock_regs;
	sensor->rows_num_awb_lock_regs	  = ARRAY_SIZE(s5k5ccgx_awb_lock_regs);



	sensor->awb_unlock_regs			  = s5k5ccgx_awb_unlock_regs;
	sensor->rows_num_awb_unlock_regs	  = ARRAY_SIZE(s5k5ccgx_awb_unlock_regs);



	

};

