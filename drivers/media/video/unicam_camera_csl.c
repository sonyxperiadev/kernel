/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * V4L2 Driver for unicam/rhea camera host
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>

#include <linux/broadcom/mobcom_types.h>
#include <plat/csl/csl_cam.h>
#include <linux/videodev2_brcm.h>

#define UNICAM_BUF_MAGIC		0xBABEFACE
#define UNICAM_CAM_DRV_NAME		"unicam-camera"

/* CSL_CAM_CAPTURE_MODE_TRIGGER (Stills) CSL_CAM_CAPTURE_MODE_NORMAL (Video) */
#define UNICAM_CAPTURE_MODE		CSL_CAM_CAPTURE_MODE_TRIGGER
#define iprintk(format, arg...)	\
	printk(KERN_INFO"[%s]: "format"\n", __func__, ##arg)

/* #define UNICAM_DEBUG */

#ifdef UNICAM_DEBUG
#define dprintk(format, arg...) \
	printk(KERN_INFO"[%s]: "format"\n", __func__, ##arg);
#else
#define dprintk(format, arg...)
#endif

#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, \
	((x) >> 24) & 0xff

struct unicam_camera_dev {
	/* soc and vb3 rleated */
	struct soc_camera_device *icd;
	struct vb2_alloc_ctx *alloc_ctx;
	struct soc_camera_host soc_host;
	/* generic driver related */
	unsigned int irq;
	struct device *dev;
	/* h/w specific */
	void __iomem *csi_base;
	void __iomem *mm_cfg_base;
	void __iomem *mm_clk_base;

	CSL_CAM_HANDLE cslCamHandle;
	/* data structure needed to support streaming */
	int sequence;
	spinlock_t lock;
	struct vb2_buffer *active;
	struct list_head capture;
	u8 streaming;
	u32 skip_frames;
	struct v4l2_subdev_sensor_interface_parms if_params;
	struct semaphore stop_sem;
	bool stopping;
	struct semaphore stop_processing_sem;
};

struct unicam_camera_buffer {
	struct vb2_buffer vb;
	struct list_head queue;
	unsigned int magic;
};

static irqreturn_t unicam_camera_isr(int irq, void *arg);

#ifdef UNICAM_DEBUG
/* for debugging purpose */
static void dump_file(char *filename, void *src, int size)
{
	mm_segment_t old_fs;
	int fd = 0, nbytes = 0;

	old_fs = get_fs();
	set_fs(get_ds());

	printk(KERN_ERR "Writing %d bytes to file %s\n", size, filename);
	fd = sys_open(filename, (O_RDWR | O_CREAT), 0777);
	if (fd < 0) {
		printk(KERN_ERR "Error in opening file %s\n", filename);
		goto out;
	}

	nbytes = sys_write(fd, src, size);
	printk(KERN_ERR "Wrote %d bytes to file %s\n", nbytes, filename);
	sys_close(fd);

out:
	set_fs(old_fs);
}
#endif

static struct unicam_camera_buffer *to_unicam_camera_vb(struct vb2_buffer *vb)
{
	return container_of(vb, struct unicam_camera_buffer, vb);
}

/* videobuf operations */

static int unicam_videobuf_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,
				 unsigned int *count, unsigned int *numplanes,
				 unsigned int sizes[], void *alloc_ctxs[])
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev =
	    (struct unicam_camera_dev *)ici->priv;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						     icd->
						     current_fmt->host_fmt);

	dprintk("-enter");

	if (bytes_per_line < 0)
		return bytes_per_line;

	*numplanes = 1;

	unicam_dev->sequence = 0;

	sizes[0] = bytes_per_line * icd->user_height;
	alloc_ctxs[0] = unicam_dev->alloc_ctx;

	if (!*count)
		*count = 2;

	iprintk("no_of_buf=%d size=%u", *count, sizes[0]);
	dprintk("-exit");
	return 0;
}

static int unicam_videobuf_prepare(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						     icd->
						     current_fmt->host_fmt);
	unsigned long size;

	dprintk("-enter");
	if (bytes_per_line < 0)
		return bytes_per_line;

	dprintk("vb=0x%p vbuf=0x%p pbuf=0x%p, size=%lu", vb,
		vb2_plane_vaddr(vb, 0), (void *)vb2_dma_contig_plane_dma_addr(vb,
									   0),
		vb2_get_plane_payload(vb, 0));

	size = icd->user_height * bytes_per_line;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(icd->dev.parent, "Buffer too small (%lu < %lu)\n",
			vb2_plane_size(vb, 0), size);
		return -ENOBUFS;
	}
	vb2_set_plane_payload(vb, 0, size);

	dprintk("-exit");
	return 0;
}

/* should be called with unicam_dev->lock held */
static int unicam_camera_update_buf(struct unicam_camera_dev *unicam_dev)
{

	struct v4l2_subdev *sd = soc_camera_to_subdev(unicam_dev->icd);
	CSL_CAM_BUFFER_st_t cslCamBuffer0;
	CSL_CAM_BUFFER_st_t cslCamBuffer1;
	CSL_CAM_BUFFER_st_t cslCamBufferData0;
	CSL_CAM_BUFFER_st_t cslCamBufferData1;
	dma_addr_t phys_addr;
	unsigned int line_stride;
	struct v4l2_format thumb_fmt;
	struct v4l2_pix_format *pix;
	int thumb = 0, ret;

	dprintk("-enter");

	if (!unicam_dev->active) {
		pr_debug("%s unicam_camera_update_buf no active buffer found:"
			" WARNING\n", __func__);
		return -ENOMEM;
	}

	phys_addr = vb2_dma_contig_plane_dma_addr(unicam_dev->active, 0);

	dprintk("updating buffer phys=0x%p", (void *)phys_addr);

	/* stride is in bytes */
	if (unicam_dev->icd->current_fmt->code != V4L2_MBUS_FMT_JPEG_1X8) {
		line_stride =
		    soc_mbus_bytes_per_line(unicam_dev->icd->user_width,
					    unicam_dev->icd->
					    current_fmt->host_fmt);
		/* image 0 */
		cslCamBuffer0.start_addr = (UInt32) phys_addr;
		cslCamBuffer0.line_stride = (UInt32) line_stride;
		cslCamBuffer0.size = line_stride * unicam_dev->icd->user_height;
		cslCamBuffer0.buffer_wrap_en = 1;
		cslCamBuffer0.mem_type = CSL_CAM_MEM_TYPE_NONE;

		dprintk
		    ("cslCamBuffer0 start_addr = 0x%x, line_stride = %d, size = %d \n",
		     cslCamBuffer0.start_addr, cslCamBuffer0.line_stride,
		     cslCamBuffer0.size);
	} else {
		/* check whether sensor supports thumbnail */
		ret =
		    v4l2_subdev_call(sd, core, ioctl, VIDIOC_THUMB_SUPPORTED,
				     (void *)&thumb);

		if ((!ret) && thumb) {

			ret =
			    v4l2_subdev_call(sd, core, ioctl,
					     VIDIOC_THUMB_G_FMT,
					     (void *)&thumb_fmt);
			if (ret < 0) {
				dev_err(unicam_dev->dev,
					"sensor driver should report thumbnail format\n");
				return -1;
			}
			/* image 0 */
			pix = &thumb_fmt.fmt.pix;
			cslCamBuffer0.start_addr = (UInt32) phys_addr;
			cslCamBuffer0.line_stride = (UInt32) pix->bytesperline;
			cslCamBuffer0.size = pix->sizeimage;
			cslCamBuffer0.buffer_wrap_en = 1;
			cslCamBuffer0.mem_type = CSL_CAM_MEM_TYPE_NONE;
		} else {
			/* no thumbnail supported */
			/* don't set image0 since we are expecting data0
			 * to contain jpeg data
			 */
			cslCamBuffer0.start_addr = 0;
			cslCamBuffer0.line_stride = 0;
			cslCamBuffer0.size = 0;
			cslCamBuffer0.buffer_wrap_en = 0;
			cslCamBuffer0.mem_type = CSL_CAM_MEM_TYPE_NONE;
		}
	}

	/* image 1 */
	cslCamBuffer1.start_addr = 0;
	cslCamBuffer1.line_stride = cslCamBuffer0.line_stride;
	cslCamBuffer1.size = cslCamBuffer0.size;
	cslCamBuffer1.buffer_wrap_en = cslCamBuffer0.buffer_wrap_en;
	cslCamBuffer1.mem_type = cslCamBuffer0.mem_type;

	/* set date buffer 0 */
	if (unicam_dev->icd->current_fmt->code != V4L2_MBUS_FMT_JPEG_1X8) {
		cslCamBufferData0.start_addr = (UInt32) 0;
		cslCamBufferData0.line_stride = 0;
		cslCamBufferData0.buffer_wrap_en = 0;
		cslCamBufferData0.size = 0;
		cslCamBufferData0.mem_type = cslCamBuffer0.mem_type;
	} else {

		pix = &thumb_fmt.fmt.pix;
		line_stride = unicam_dev->icd->user_width;
		/* check if thumbnail is supported */
		if (thumb)
			cslCamBufferData0.start_addr =
			    (UInt32) ((char *)phys_addr + pix->sizeimage);
		else
			cslCamBufferData0.start_addr = (UInt32) phys_addr;

		cslCamBufferData0.line_stride = (UInt32) line_stride;
		/* assume 12bpp */
		cslCamBufferData0.size =
		    (line_stride * unicam_dev->icd->user_height * 3 / 2);
		cslCamBufferData0.buffer_wrap_en = 1;
		cslCamBufferData0.mem_type = CSL_CAM_MEM_TYPE_NONE;
		dprintk
		    ("cslCamBufferData0 start_addr = 0x%x, line_stride = %d, size = %d\n",
		     cslCamBufferData0.start_addr,
		     cslCamBufferData0.line_stride, cslCamBufferData0.size);
	}

	/* set data buffer 1 */
	cslCamBufferData1.start_addr = 0;

	if (cslCamBuffer1.start_addr != 0) {
		if (csl_cam_set_input_addr
		    (unicam_dev->cslCamHandle, &cslCamBuffer0, &cslCamBuffer1,
		     &cslCamBufferData0)) {

			dev_err(unicam_dev->dev,
				"csl_cam_set_input_addr(): FAILED\n");
			return -1;
		}
	} else {
		if (csl_cam_set_input_addr
		    (unicam_dev->cslCamHandle, &cslCamBuffer0, NULL,
		     &cslCamBufferData0)) {

			dev_err(unicam_dev->dev,
				"CamCslSetInputIntf(): csl_cam_set_input_addr(): FAILED\n");
			return -1;
		}
	}
	dprintk("-exit");
	return 0;
}

/* should be called with unicam_dev->lock held */
static int unicam_camera_capture(struct unicam_camera_dev *unicam_dev)
{
	int ret = 0;
	CSL_CAM_FRAME_st_t cslCamFrame;
	dprintk("-enter");

	if (!unicam_dev->active) {
		pr_debug("%s: unicam_camera_capture no active buffer :"
			" WARNING\n", __func__);
		return ret;
	}

	/* enable frame Interrupts */
	cslCamFrame.int_enable = CSL_CAM_INT_FRAME_END;
	/* for testing enabled frame start interrupt */
	cslCamFrame.int_enable |= CSL_CAM_INT_FRAME_START;
	cslCamFrame.int_line_count = 0;
	cslCamFrame.capture_mode = UNICAM_CAPTURE_MODE;
	cslCamFrame.capture_size = 0;
	if (csl_cam_set_frame_control(unicam_dev->cslCamHandle, &cslCamFrame)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_frame_control(): FAILED\n");
		return -1;
	}

	if (UNICAM_CAPTURE_MODE == CSL_CAM_CAPTURE_MODE_TRIGGER) {
		if (csl_cam_trigger_capture(unicam_dev->cslCamHandle) != 0) {
			dev_err(unicam_dev->dev,
				"error in triggering capture\n");
			return -1;
		}
	}
	dprintk("-exit()");
	return ret;
}

static void unicam_videobuf_queue(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;

	dprintk("-enter");
	dprintk("vb=0x%p vbuf=0x%p pbuf=0x%p size=%lu", vb,
		vb2_plane_vaddr(vb, 0), (void *)vb2_dma_contig_plane_dma_addr(vb,
									   0),
		vb2_get_plane_payload(vb, 0));

	spin_lock_irqsave(&unicam_dev->lock, flags);
	list_add_tail(&buf->queue, &unicam_dev->capture);

	if (!unicam_dev->active) {
		unicam_dev->active = vb;
		/* use this buffer to trigger capture */
		/* Configure HW only is streamon has been done
		 * else only update active, HW would be configured
		 * by streamon  */
		if(unicam_dev->streaming){
			unicam_camera_update_buf(unicam_dev);
			if (unicam_dev->if_params.if_mode ==
				V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)
				unicam_camera_capture(unicam_dev);
		}
	}
	spin_unlock_irqrestore(&unicam_dev->lock, flags);
	dprintk("-exit");
}

static void unicam_videobuf_release(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;

	dprintk("-enter");

	dprintk("vb=0x%p vbuf=0x%p pbuf=0x%p size=%lu", vb,
		vb2_plane_vaddr(vb, 0), (void *)vb2_dma_contig_plane_dma_addr(vb,
									   0),
		vb2_get_plane_payload(vb, 0));
	spin_lock_irqsave(&unicam_dev->lock, flags);

	if (buf->magic == UNICAM_BUF_MAGIC)
		list_del_init(&buf->queue);
	spin_unlock_irqrestore(&unicam_dev->lock, flags);

	dprintk("-exit");
}

static int unicam_videobuf_init(struct vb2_buffer *vb)
{
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	INIT_LIST_HEAD(&buf->queue);
	buf->magic = UNICAM_BUF_MAGIC;
	return 0;
}

int unicam_videobuf_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct v4l2_subdev_sensor_interface_parms if_params;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int ret;
	int thumb;
	unsigned long flags;

	CSL_CAM_INTF_CFG_st_t csl_cam_intf_cfg_st;
	CSL_CAM_LANE_CONTROL_st_t cslCamLaneCtrl_st;
	CSL_CAM_PIPELINE_st_t cslCamPipeline;
	CSL_CAM_IMAGE_ID_st_t cslCamImageCtrl;
	CSL_CAM_DATA_st_t cslCamDataCtrl;
	CSL_CAM_FRAME_st_t cslCamFrame;

	dprintk("-enter");
	iprintk("enabling csi");

	spin_lock_irqsave(&unicam_dev->lock, flags);
	unicam_dev->stopping = false;
	spin_unlock_irqrestore(&unicam_dev->lock, flags);
	if (csl_cam_init()) {
		dev_err(unicam_dev->dev, "error initializing csl camera\n");
		return -1;
	}

/*
	ret = v4l2_subdev_call(sd, video, s_stream, 1);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_err(unicam_dev->dev, "error on s_stream(%d)\n", ret);
		return ret;
	}
	*/

	/* get the sensor interface information */
	ret = v4l2_subdev_call(sd, sensor, g_interface_parms, &if_params);
	if (ret < 0) {
		dev_err(unicam_dev->dev, "error on g_inferface_params(%d)\n",
			ret);
		return ret;
	}

	unicam_dev->if_params = if_params;

	/* set camera interface parameters */
	memset(&csl_cam_intf_cfg_st, 0, sizeof(CSL_CAM_INTF_CFG_st_t));

	/* we only support serial and csi2 sensor */
	if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)) {
		csl_cam_intf_cfg_st.intf = CSL_CAM_INTF_CSI;
	} else if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1)) {
		csl_cam_intf_cfg_st.intf = CSL_CAM_INTF_CCP;
	} else {
		dev_err(unicam_dev->dev,
			"CSI2 iface only supported,requested iface %d mode=%d\n",
			unicam_dev->if_params.if_type,
			unicam_dev->if_params.if_mode);
		return -EINVAL;
	}

	if (unicam_dev->if_params.parms.serial.channel == 0)
		csl_cam_intf_cfg_st.afe_port = CSL_CAM_PORT_AFE_0;
	else if (unicam_dev->if_params.parms.serial.channel == 1)
		csl_cam_intf_cfg_st.afe_port = CSL_CAM_PORT_AFE_1;
	else {
		dev_err(unicam_dev->dev,
			"receiver only supports two channels, request channel=%d\n",
			unicam_dev->if_params.parms.serial.channel);
		return -EINVAL;
	}

	csl_cam_intf_cfg_st.frame_time_out = 1000;

	/* open camera interface */
	csl_cam_intf_cfg_st.p_cpi_intf_st = NULL;

	if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)) {
		if (unicam_dev->if_params.parms.serial.lanes == 1)
			csl_cam_intf_cfg_st.input_mode =
				CSL_CAM_INPUT_SINGLE_LANE;
		else if (unicam_dev->if_params.parms.serial.lanes == 2)
			csl_cam_intf_cfg_st.input_mode =
				CSL_CAM_INPUT_DUAL_LANE;
		else {
			dev_err(unicam_dev->dev,
				"receiver only supports max 2 lanes, requested lanes(%d)\n",
				unicam_dev->if_params.parms.serial.lanes);
			return -EINVAL;
		}
	} else {
		 csl_cam_intf_cfg_st.input_mode = CSL_CAM_INPUT_MODE_DATA_CLOCK;
	}

	if (csl_cam_open(&csl_cam_intf_cfg_st, &unicam_dev->cslCamHandle)) {
		dev_err(unicam_dev->dev, "%s: csl_cam_open(): ERROR\n",
			__func__);
		return -1;
	}

	/* set data lane timing */
	cslCamLaneCtrl_st.lane_select = CSL_CAM_DATA_LANE_0;
	cslCamLaneCtrl_st.lane_control = CSL_CAM_LANE_HS_TERM_TIME;
	cslCamLaneCtrl_st.param =
	    unicam_dev->if_params.parms.serial.hs_term_time;

	dprintk("hs_term_time is set to = %d\n", cslCamLaneCtrl_st.param);

	if (csl_cam_set_lane_control
	    (unicam_dev->cslCamHandle, &cslCamLaneCtrl_st)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_lane_control(): FAILED\n");
		return -1;
	}

	if (unicam_dev->if_params.parms.serial.hs_settle_time != 0) {
		cslCamLaneCtrl_st.lane_select = CSL_CAM_DATA_LANE_0;
		cslCamLaneCtrl_st.lane_control = CSL_CAM_LANE_HS_SETTLE_TIME;
		cslCamLaneCtrl_st.param =
			unicam_dev->if_params.parms.serial.hs_settle_time;
		dprintk("hs_settle_time is set to = %d\n",
					cslCamLaneCtrl_st.param);
		if (csl_cam_set_lane_control
		    (unicam_dev->cslCamHandle, &cslCamLaneCtrl_st)) {
			dev_err(unicam_dev->dev,
				"csl_cam_set_lane_control(): FAILED\n");
			return -1;
		}
	}

	/* pipelince decode */
	cslCamPipeline.decode = CSL_CAM_DEC_NONE;
	cslCamPipeline.unpack = CSL_CAM_PIXEL_NONE;
	cslCamPipeline.pack = CSL_CAM_PIXEL_NONE;
	cslCamPipeline.dec_adv_predictor = FALSE;

	cslCamPipeline.encode = CSL_CAM_ENC_NONE;
	cslCamPipeline.enc_adv_predictor = FALSE;
	cslCamPipeline.encode_blk_size = 0x0000;

	/* set pipeline */
	if (csl_cam_set_pipeline_control
	    (unicam_dev->cslCamHandle, &cslCamPipeline)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_pipeline_control(): FAILE\n");
		return -1;
	}

	/* set image identifier (CSI mode only) */
	memset(&cslCamImageCtrl, 0, sizeof(CSL_CAM_IMAGE_ID_st_t));

	/* if thumbnail is supported we expect
	 * thumbnail to be in image ptr format of thumbnails is yuv422
	 * format is checked in try format.
	 * in case where thumbnail is not supported we get jpeg
	 * image in data pointer. so we set the id as 0
	 */

	thumb = 0;
	ret =
	    v4l2_subdev_call(sd, core, ioctl, VIDIOC_THUMB_SUPPORTED,
			     (void *)&thumb);
	if (ret < 0)
		dev_warn(unicam_dev->dev,
			 "sensor returns error(%d) for VIDIOC_THUMB_SUPPORTED\n",
			 ret);

	if ((icd->current_fmt->code == V4L2_MBUS_FMT_JPEG_1X8)
	    && (thumb == 0))
		cslCamImageCtrl.image_data_id0 = 0x0;
		/* thumbnail not supported */
	else
		cslCamImageCtrl.image_data_id0 = 0x1E;

	if (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1)
		cslCamImageCtrl.image_data_id0 = 0x0;	/* CCP2 channel ID 0 */


	if (csl_cam_set_image_type_control
	    (unicam_dev->cslCamHandle, &cslCamImageCtrl)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_image_type_control(): FAILED\n");
		return -1;
	}

	/* set data capture */
	cslCamDataCtrl.int_enable = (CSL_CAM_INTERRUPT_t) (CSL_CAM_INT_DISABLE);
	if (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2) {
		cslCamDataCtrl.line_count = 2;
		cslCamDataCtrl.fsp_decode_enable = FALSE;
	} else {
		cslCamDataCtrl.line_count = 0;
		cslCamDataCtrl.fsp_decode_enable = TRUE;
	}
	cslCamDataCtrl.data_id = 0x00;
	cslCamDataCtrl.data_size = CSL_CAM_PIXEL_8BIT;

	if (csl_cam_set_data_type_control
	    (unicam_dev->cslCamHandle, &cslCamDataCtrl)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_data_type_control(): FAILED\n");
		return -1;
	}

	/* start receiver */
	if (csl_cam_rx_start(unicam_dev->cslCamHandle)) {
		dev_err(unicam_dev->dev, "csl_cam_rx_start(): FAILED\n");
		return -1;
	}

	/* Enabling sensor after enabling unicam */
	ret = v4l2_subdev_call(sd, video, s_stream, 1);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_err(unicam_dev->dev, "error on s_stream(%d)\n", ret);
		return ret;
	}

	if (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1) {
		cslCamFrame.int_enable = CSL_CAM_INT_LINE_COUNT;
		/* CSL_CAM_INT_FRAME_END | CSL_CAM_INT_FRAME_START;
		//CSL_CAM_INT_LINE_COUNT;
		cslCamFrame.int_line_count =
		(unicam_dev->icd->user_height - 1);*/
		cslCamFrame.int_line_count = (unicam_dev->icd->user_height);
		cslCamFrame.capture_mode = CSL_CAM_CAPTURE_MODE_NORMAL;
		/* CSL_CAM_CAPTURE_MODE_NORMAL */
		if (csl_cam_set_frame_control(
			unicam_dev->cslCamHandle, &cslCamFrame)) {
			dev_err(unicam_dev->dev,
			"csl_cam_set_frame_control(): FAILED\n");
			return -1;
		}

	}
	/* Configure HW if buffer is queued ahead of streamon */
	if(unicam_dev->active){
		unicam_camera_update_buf(unicam_dev);
		if (unicam_dev->if_params.if_mode ==
			V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)
			unicam_camera_capture(unicam_dev);
	}
	unicam_dev->streaming = 1;
	csl_cam_register_display(unicam_dev->cslCamHandle);
	dprintk("-exit");
	return 0;
}

int unicam_videobuf_stop_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	CSL_CAM_FRAME_st_t cslCamFrame;
	int ret = 0;
	unsigned long flags;

	if (down_interruptible(&unicam_dev->stop_processing_sem) == 0) {
		if (unicam_dev->streaming)
			csl_cam_register_display(unicam_dev->cslCamHandle);
	} else
		dev_err(unicam_dev->dev, "Unable to dump regs because stop_processing_sem acquire failed\n");

	/* grab the lock */
	spin_lock_irqsave(&unicam_dev->lock, flags);
	dprintk("-enter");
	dprintk("disabling csi");
	iprintk("stopping stream");
	if (!unicam_dev->streaming) {
		dev_err(unicam_dev->dev, "stream already turned off\n");
		goto out;
	}
	/*
	 * stop streaming before grabing spin lock
	 * since this function can sleep.
	 * */
	if (unicam_dev->active) {
		unicam_dev->stopping = true;
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
		ret = down_timeout(&unicam_dev->stop_sem,
				msecs_to_jiffies(500));
		if (ret == -ETIME)
			pr_err("Unicam: semaphore timed out waiting to STOP\n");
	} else {
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
	}
	spin_lock_irqsave(&unicam_dev->lock, flags);

	/* disable frame interrupts */
	cslCamFrame.int_enable = CSL_CAM_INT_DISABLE;
	cslCamFrame.int_line_count = 0;
	cslCamFrame.capture_mode = UNICAM_CAPTURE_MODE;
	cslCamFrame.capture_size = 0;

	if (csl_cam_set_frame_control(unicam_dev->cslCamHandle, &cslCamFrame)) {
		dev_err(unicam_dev->dev,
			"csl_cam_set_frame_control(): FAILED\n");
		ret = -1;
	}

	/* disable receiver */
	if (csl_cam_rx_stop(unicam_dev->cslCamHandle)) {
		dev_err(unicam_dev->dev, "csl_cam_rx_stop(): FAILED\n");
		ret = -1;
	}

	if (csl_cam_close(unicam_dev->cslCamHandle)) {
		dev_err(unicam_dev->dev, "cals_cam_exit(): FAILED\n");
		ret = -1;
	}

	if (csl_cam_exit()) {
		dev_err(unicam_dev->dev, "csl_cam_exit(): FAILED\n");
		ret = -1;
	}

	unicam_dev->active = NULL;
	unicam_dev->streaming = 0;
out:
	dprintk("-exit");
	spin_unlock_irqrestore(&unicam_dev->lock, flags);
	up(&unicam_dev->stop_processing_sem);

	/* Stopping stream after stopping unicam */
	ret = v4l2_subdev_call(sd, video, s_stream, 0);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_err(unicam_dev->dev, "failed to stop sensor streaming\n");
		ret = -1;
	}
	return ret;
}

static struct vb2_ops unicam_videobuf_ops = {
	.queue_setup = unicam_videobuf_setup,
	.buf_prepare = unicam_videobuf_prepare,
	.buf_queue = unicam_videobuf_queue,
	.buf_cleanup = unicam_videobuf_release,
	.buf_init = unicam_videobuf_init,
	.start_streaming = unicam_videobuf_start_streaming,
	.stop_streaming = unicam_videobuf_stop_streaming,
	.wait_prepare = soc_camera_unlock,
	.wait_finish = soc_camera_lock
};

static int unicam_camera_init_videobuf(struct vb2_queue *q,
				       struct soc_camera_device *icd)
{
	dprintk("-enter");
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_READ;
	q->drv_priv = icd;
	q->ops = &unicam_videobuf_ops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->buf_struct_size = sizeof(struct unicam_camera_buffer);
	dprintk("-exit");
	return vb2_queue_init(q);
}

static int unicam_camera_set_bus_param(struct soc_camera_device *icd,
				       __u32 pixfmt)
{
	dprintk("-enter");
	dprintk("-exit");
	return 0;
}

static int unicam_camera_querycap(struct soc_camera_host *ici,
				  struct v4l2_capability *cap)
{
	dprintk("-enter");
	/* cap->name is set by the firendly caller:-> */
	strlcpy(cap->card, "Unicam Camera", sizeof(cap->card));
	cap->version = KERNEL_VERSION(0, 1, 0);
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;

	dprintk("-exit");
	return 0;
}

static unsigned int unicam_camera_poll(struct file *file, poll_table * pt)
{
	struct soc_camera_device *icd = file->private_data;

	dprintk("-enter");
	dprintk("-exit");
	return vb2_poll(&icd->vb2_vidq, file, pt);
}

static int unicam_camera_try_fmt(struct soc_camera_device *icd,
				 struct v4l2_format *f)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	struct v4l2_format thumb_fmt;
	struct v4l2_pix_format *thumb_pix;
	__u32 pixfmt = pix->pixelformat;
	int thumb=0;
	int ret;

	dprintk("-enter");
	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (!xlate) {
		dev_warn(icd->dev.parent, "Format %x not found\n", pixfmt);
		return -EINVAL;
	}

	pix->sizeimage = pix->height * pix->bytesperline;

	/* limit to sensor capabilities */
	mf.width = pix->width;
	mf.height = pix->height;
	mf.field = pix->field;
	mf.colorspace = pix->colorspace;
	mf.code = xlate->code;

	ret = v4l2_subdev_call(sd, video, try_mbus_fmt, &mf);
	if (ret < 0)
		return ret;

	pix->width = mf.width;
	pix->height = mf.height;
	pix->colorspace = mf.colorspace;

	switch (mf.field) {
	case V4L2_FIELD_ANY:
	case V4L2_FIELD_NONE:
		pix->field = V4L2_FIELD_NONE;
		break;
	default:
		dev_err(icd->dev.parent, "Field type %d unsupported.\n",
			mf.field);
		return -EINVAL;
	}

	/* what format can unicam support */
	switch (mf.code) {
	case V4L2_MBUS_FMT_JPEG_1X8:
		/* check here if thumbnail is supported
		and check thumbnail format */
		ret =
		    v4l2_subdev_call(sd, core, ioctl, VIDIOC_THUMB_SUPPORTED,
				     (void *)&thumb);
		if ((!ret) && thumb) {
			ret =
			    v4l2_subdev_call(sd, core, ioctl,
					     VIDIOC_THUMB_G_FMT,
					     (void *)&thumb_fmt);
			if (ret < 0) {
				dev_err(icd->dev.parent,
					"sensor driver should report thumbnail format\n");
				return -EINVAL;
			}
			thumb_pix = &thumb_fmt.fmt.pix;
			switch (thumb_pix->pixelformat) {
			case V4L2_PIX_FMT_YUYV:
			case V4L2_PIX_FMT_UYVY:
				iprintk
				    ("sensor supports thumbnail %c%c%c%c format",
				     pixfmtstr(thumb_pix->pixelformat));
				break;
			default:
				dev_err(icd->dev.parent,
					"sensor thumbnail format %c%c%c%c not supported\n",
					pixfmtstr(thumb_pix->pixelformat));
				return -EINVAL;
			}
		} else
			iprintk
			    ("sensor doesnot support thumbnail (thumb=%d, ret=%d)\n",
			     thumb, ret);

	case V4L2_MBUS_FMT_YUYV8_2X8:
	case V4L2_MBUS_FMT_UYVY8_2X8:
		/* Above formats are supported */
		break;
	default:
		dev_err(icd->dev.parent, "Sensor format code %d unsupported.\n",
			mf.code);
		return -EINVAL;
	}
	iprintk("trying format=%c%c%c%c res=%dx%d success=%d",
		pixfmtstr(pixfmt), mf.width, mf.height, ret);
	dprintk("-exit");
	return ret;
}

static int unicam_camera_set_fmt(struct soc_camera_device *icd,
				 struct v4l2_format *f)
{
	struct device *dev = icd->dev.parent;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	const struct soc_camera_format_xlate *xlate = NULL;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	int ret;
	u32 skip_frames = 0;

	dprintk("-enter");
	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_warn(dev, "Format %x not found\n", pix->pixelformat);
		return -EINVAL;
	}

	mf.width = pix->width;
	mf.height = pix->height;
	mf.field = pix->field;
	mf.colorspace = pix->colorspace;
	mf.code = xlate->code;

	ret = v4l2_subdev_call(sd, video, s_mbus_fmt, &mf);

	if (mf.code != xlate->code)
		return -EINVAL;

	if (ret < 0) {
		dev_warn(dev, "Failed to configure for format %x\n",
			 pix->pixelformat);
		return ret;
	}
	/*TODO limit here any maximum size */

	ret = v4l2_subdev_call(sd, sensor, g_skip_frames, &skip_frames);
	if (ret < 0) {
		dev_warn(dev,
			 "sensor driver doesn't implement g_skip_frames operation\n");
		dev_warn(dev, "assuming zero skip frames\n");
		skip_frames = 0;
		ret = 0;
	}

	unicam_dev->skip_frames = skip_frames;

	pix->width = mf.width;
	pix->height = mf.height;
	pix->field = mf.field;
	pix->colorspace = mf.colorspace;
	icd->current_fmt = xlate;
	iprintk("format set to %c%c%c%c res=%dx%d success=%d",
		pixfmtstr(pix->pixelformat), pix->width, pix->height, ret);
	dprintk("-exit");
	return ret;
}

static int unicam_camera_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int err = 0;

	if (unicam_dev->icd) {
		dev_warn(icd->dev.parent,
			 "Unicam camera driver already attached to another client\n");
		err = -EBUSY;
		goto eicd;
	}

	/* register irq */
	err =
	    request_irq(unicam_dev->irq, unicam_camera_isr,
			IRQF_DISABLED | IRQF_SHARED, UNICAM_CAM_DRV_NAME,
			unicam_dev);
	if (err) {
		dev_err(icd->dev.parent, "cound not install irq %d\n",
			unicam_dev->irq);
		err = -ENODEV;
		goto eirq;
	}

	unicam_dev->icd = icd;

	dev_info(icd->dev.parent,
		 "Unicam Camera driver attached to camera %d\n", icd->devnum);
eirq:
eicd:
	return err;
}

static void unicam_camera_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	BUG_ON(icd != unicam_dev->icd);

	if (unicam_dev->streaming) {
		/* stop streaming */
		/* we should call streamoff from queue operations */
		unicam_videobuf_stop_streaming(&icd->vb2_vidq);
	}

	free_irq(unicam_dev->irq, unicam_dev);

	unicam_dev->icd = NULL;

	dev_info(icd->dev.parent,
		 "Unicam Camera driver detached from camera %d\n", icd->devnum);
}

static struct soc_camera_host_ops unicam_soc_camera_host_ops = {
	.owner = THIS_MODULE,
	.add = unicam_camera_add_device,
	.remove = unicam_camera_remove_device,
	.set_fmt = unicam_camera_set_fmt,
	.try_fmt = unicam_camera_try_fmt,
	.init_videobuf2 = unicam_camera_init_videobuf,
	.poll = unicam_camera_poll,
	.querycap = unicam_camera_querycap,
	.set_bus_param = unicam_camera_set_bus_param,
};

static irqreturn_t unicam_camera_isr(int irq, void *arg)
{
	struct unicam_camera_dev *unicam_dev = (struct unicam_camera_dev *)arg;
	unsigned int status;
	unsigned int reg_status;
	int ret;
	CSL_CAM_BUFFER_STATUS_st_t bufStatus;
	unsigned int bytes_used;
	unsigned long flags;
	static unsigned int t1 = 0, t2 = 0, fps = 0;

	spin_lock_irqsave(&unicam_dev->lock, flags);
	if (!unicam_dev->streaming) {
		pr_err("Interrupt triggered after stopping camera!\n");
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
		return IRQ_HANDLED;
	} else {
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
	}
	/* has the interrupt occured for Channel 0? */
	reg_status =
	    csl_cam_get_rx_status(unicam_dev->cslCamHandle,
				  (CSL_CAM_RX_STATUS_t *) &status);
	dprintk("received unicam interrupt reg_status=0x%x status=0x%x\n",
		reg_status, status);

	if (status & CSL_CAM_RX_INT) {

		/* get and clear interrupt status */
		reg_status =
		    csl_cam_get_intr_status(unicam_dev->cslCamHandle,
					    (CSL_CAM_INTERRUPT_t *) &status);
		if (status & CSL_CAM_INT_FRAME_START) {
			status = status & ~CSL_CAM_INT_FRAME_START;
			if (reg_status & 0x2000)
				pr_err("Camera: Urgent request was signalled at FS!!!\n");
		}

		if ((status & CSL_CAM_INT_FRAME_END) ||
			(status & CSL_CAM_INT_LINE_COUNT)) {
			struct vb2_buffer *vb = unicam_dev->active;
			status &= ~(CSL_CAM_INT_FRAME_END |
					CSL_CAM_INT_LINE_COUNT);
			fps++;
			if (reg_status & 0x2000)
				pr_err("Camera: Urgent request was signalled at FE!!!!\n");
			if (t1 == 0 && t2 == 0)
				t1 = t2 = jiffies_to_msecs(jiffies);

			t2 = jiffies_to_msecs(jiffies);
			if (t2 - t1 > 1000) {
				printk(" sensor fps = %d \n", fps);
				fps = 0;
				t1 = t2;
			}
			dprintk("frame received");
			/* csl_cam_register_display(unicam_dev->cslCamHandle);*/
			if (!vb)
				goto out;
			/* mark  the buffer done */
			/* queue another buffer and trigger capture */
			if (likely(unicam_dev->skip_frames <= 0)) {

				list_del_init(&to_unicam_camera_vb(vb)->queue);
				do_gettimeofday(&vb->v4l2_buf.timestamp);
				vb->v4l2_planes[0].bytesused = 0;

				if (unicam_dev->icd->current_fmt->code ==
				    V4L2_MBUS_FMT_JPEG_1X8) {

					ret =
					    csl_cam_get_buffer_status
					    (unicam_dev->cslCamHandle,
					     CSL_CAM_DATA, &bufStatus);

					if (ret == CSL_CAM_OK) {
						bytes_used =
						    (bufStatus.write_ptr -
						     bufStatus.buffer_st.
						     start_addr);
						vb->v4l2_planes[0].bytesused =
						    bytes_used;
					} else
						dev_warn(unicam_dev->dev,
							 "%s:failed to get buffer status",
							 __func__);
				} else {
					ret =
					    csl_cam_get_buffer_status
					    (unicam_dev->cslCamHandle,
					     CSL_CAM_IMAGE, &bufStatus);
					if (ret == CSL_CAM_OK) {
						dprintk("Buffer Status:");
						dprintk("addr:0x%x size:0x%x"
							"ls:%d wp:0x%x"
							"ppl:%d lpf:%d",
							bufStatus.buffer_st.
							start_addr,
							bufStatus.buffer_st.
							size,
							bufStatus.buffer_st.
							line_stride,
							bufStatus.write_ptr,
							bufStatus.
							bytes_per_line,
							bufStatus.
							lines_per_frame);

					}
				}

				vb2_buffer_done(vb, VB2_BUF_STATE_DONE);

				spin_lock_irqsave(&unicam_dev->lock, flags);
				if (unicam_dev->stopping) {
					up(&unicam_dev->stop_sem);
					unicam_dev->active = NULL;
				} else if (!list_empty(&unicam_dev->capture)) {
					unicam_dev->active =
					    &list_entry(unicam_dev->
							capture.next, struct
							unicam_camera_buffer,
							queue)->vb;
				} else {
					unicam_dev->active = NULL;
				}
				spin_unlock_irqrestore(&unicam_dev->lock,
					flags);
			} else
				unicam_dev->skip_frames--;

			ret = unicam_camera_update_buf(unicam_dev);
			if (ret)
				pr_debug("%s: error while queueing"
					" the buffer\n", __func__);
			if (unicam_dev->if_params.if_mode ==
				V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2) {
				ret = unicam_camera_capture(unicam_dev);
				if (ret)
					dprintk(KERN_INFO "error triggering capture\n");
			}

		}

		if (status)
			dev_err(unicam_dev->dev,
				"interrupt not handled reg_status=0x%x"
				" status=0x%x\n", reg_status, status);
	} else
		dev_err(unicam_dev->dev,
			"rx interrupt not handled reg_status=0x%x"
			" status=0x%x\n", reg_status, status);

out:
	return IRQ_HANDLED;
}

static int __devinit unicam_camera_probe(struct platform_device *pdev)
{
	struct unicam_camera_dev *unicam_dev;
	struct soc_camera_host *soc_host;
	int irq;
	int err = 0;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err = -ENODEV;
		goto edev;
	}

	unicam_dev = vzalloc(sizeof(*unicam_dev));
	if (!unicam_dev) {
		dev_err(&pdev->dev,
			"Could not allocate unicam camera object\n");
		err = -ENOMEM;
		goto ealloc;
	}

	INIT_LIST_HEAD(&unicam_dev->capture);
	spin_lock_init(&unicam_dev->lock);
	sema_init(&unicam_dev->stop_sem, 0);
	sema_init(&unicam_dev->stop_processing_sem, 1);
	unicam_dev->stopping = false;

	unicam_dev->dev = &pdev->dev;
	unicam_dev->irq = irq;
	soc_host = &unicam_dev->soc_host;
	soc_host->drv_name = UNICAM_CAM_DRV_NAME;
	soc_host->ops = &unicam_soc_camera_host_ops;
	soc_host->priv = unicam_dev;
	soc_host->v4l2_dev.dev = &pdev->dev;
	soc_host->nr = pdev->id;

	unicam_dev->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(unicam_dev->alloc_ctx)) {
		err = PTR_ERR(unicam_dev->alloc_ctx);
		goto eallocctx;
	}

	err = soc_camera_host_register(soc_host);
	if (err)
		goto ecamhostreg;

	return 0;

ecamhostreg:
	vb2_dma_contig_cleanup_ctx(unicam_dev->alloc_ctx);
eallocctx:
	vfree(unicam_dev);
ealloc:
edev:
	return err;
}

static int __devexit unicam_camera_remove(struct platform_device *pdev)
{
	/*TODO */
	return 0;
}

static struct platform_driver unicam_camera_driver = {
	.driver = {
		   .name = UNICAM_CAM_DRV_NAME,
		   },
	.probe = unicam_camera_probe,
	.remove = __devexit_p(unicam_camera_remove),
};

static int __init unicam_camera_init(void)
{
	return platform_driver_register(&unicam_camera_driver);
}

static void __exit unicam_camera_exit(void)
{
	platform_driver_unregister(&unicam_camera_driver);
}

module_init(unicam_camera_init);
module_exit(unicam_camera_exit);

MODULE_DESCRIPTION("Unicam Camera Host driver");
MODULE_AUTHOR("Pradeep Sawlani <spradeep@broadcom.com>");
