

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
#include <camdrv_ss_sr030pc50.h>
  

#define SR030PC50_NAME	"sr030pc50"

#define SR030PC50_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define SR030PC50_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8 
#define SR030PC50_REGISTER_SIZE 2



static const struct camdrv_ss_framesize sr030pc50_supported_preview_framesize_list[] = {
	{ PREVIEW_SIZE_VGA,	640,  480 },
};

static const struct camdrv_ss_framesize  sr030pc50_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA, 640,  480 }

};
const static struct v4l2_fmtdesc sr030pc50_fmts[] = 
{
	{
	.index		= 0,
	.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
	.flags		= 0,
	.description	= "UYVY",
	.pixelformat	= V4L2_MBUS_FMT_UYVY8_2X8,
	},
};

static const struct v4l2_queryctrl sr030pc50_controls[] = {
		{
		},
};


static long sr030pc50_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
//	struct i2c_client *client = v4l2_get_subdevdata(sd);
//	struct camdrv_ss_state *state =
//		container_of(sd, struct camdrv_ss_states, sd);
	int ret = 0;

	switch(cmd) {

		case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
			*p = 0; /* NO */

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
			p->padded =0;
			p->packet_size = 0;
			 break;
		}

		default:
			ret = -ENOIOCTLCMD;
			break;
	}

	return ret;
}

bool camdrv_ss_sensor_init_sub(bool bOn, struct camdrv_ss_sensor_cap *sensor)
{

		
	strcpy(sensor->name,SR030PC50_NAME);
	sensor->supported_preview_framesize_list  = sr030pc50_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(sr030pc50_supported_preview_framesize_list);
	
	sensor->supported_capture_framesize_list  =  sr030pc50_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(sr030pc50_supported_capture_framesize_list);
	
	sensor->fmts 				   = sr030pc50_fmts;
	sensor->rows_num_fmts		   =ARRAY_SIZE(sr030pc50_fmts);


	sensor->controls				   =sr030pc50_controls;
	sensor->rows_num_controls	   =ARRAY_SIZE(sr030pc50_controls);
	
	sensor->default_pix_fmt 				   = SR030PC50_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt			   = SR030PC50_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size 		   = SR030PC50_REGISTER_SIZE;

	/* sensor dependent functions */
	sensor->thumbnail_ioctl			       = sr030pc50_ss_ioctl;


	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */
	
	sensor->init_regs						  = sr030pc50_init_regs;
	sensor->rows_num_init_regs				  = ARRAY_SIZE(sr030pc50_init_regs);
}
	

