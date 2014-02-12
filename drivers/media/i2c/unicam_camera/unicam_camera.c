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

#define pr_fmt(fmt) "unicam_camera: " fmt

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
#if defined(CONFIG_VIDEOBUF2_DMA_RESERVED)
#include <media/videobuf2-dma-reserved.h>
#define vb2_plane_dma_addr vb2_dma_reserved_plane_dma_addr
#elif defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
#include <media/videobuf2-dma-contig.h>
#define vb2_plane_dma_addr vb2_dma_contig_plane_dma_addr
#else
#error Unicam driver expect DMA_CONTIG or DMA_RESERVED
#endif
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>

#include <linux/broadcom/mobcom_types.h>
#include "mm_csi0.h"
#include <linux/videodev2_brcm.h>

#define UNICAM_BUF_MAGIC		0xBABEFACE
#define UNICAM_CAM_DRV_NAME		"unicam-camera"

#define UNICAM_CAPTURE_MODE		BUFFER_TRIGGER
#define UNPACK_RAW10_TO_16BITS
#define UC_TIMEOUT_MS			500
#define UC_RETRY_CNT			3


#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, \
	((x) >> 24) & 0xff
/* int only_once = 0; */

struct unicam_camera_dev {
	/* soc and vb3 rleated */
	struct soc_camera_device *icd;
#if defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	struct vb2_alloc_ctx *alloc_ctx;
#endif
	struct soc_camera_host soc_host;
	/* generic driver related */
	unsigned int irq;
	struct device *dev;
	/* h/w specific */
	void __iomem *csi_base;
	void __iomem *mm_cfg_base;
	void __iomem *mm_clk_base;

	void *handle;
	/* data structure needed to support streaming */
	int sequence;
	spinlock_t lock;
	struct vb2_buffer *active;
	struct list_head capture;
	struct vb2_buffer *buff[2];
	bool curr;
	atomic_t streaming;
	u32 skip_frames;
	struct v4l2_subdev_sensor_interface_parms if_params;
	struct semaphore stop_sem;
	atomic_t stopping;
	enum buffer_mode b_mode;
	struct v4l2_crop crop;
	int cap_mode;
	int cap_done;
	u32 panic_count;
	atomic_t cam_triggered;
	struct v4l2_format active_fmt;
	int frame_info_en;
	struct v4l2_frame_info frame_info;
};

struct unicam_camera_buffer {
	struct vb2_buffer vb;
	struct list_head queue;
	unsigned int magic;
};
static irqreturn_t unicam_camera_isr(int irq, void *arg);
static irqreturn_t unicam_camera_isr_bh(int irq, void *arg);
static int unicam_stop(void);

#ifdef UNICAM_DEBUG
/* for debugging purpose */
static void dump_file(char *filename, void *src, int size)
{

	mm_segment_t old_fs;
	int fd = 0, nbytes = 0;

	old_fs = get_fs();
	set_fs(get_ds());

	pr_err("Writing %d bytes to file %s\n", size, filename);
	fd = sys_open(filename, (O_RDWR | O_CREAT), 0777);
	if (fd < 0) {
		pr_err("Error in opening file %s\n", filename);
		goto out;
	}

	nbytes = sys_write(fd, src, size);
	pr_err("Wrote %d bytes to file %s\n", nbytes, filename);
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
static int unicam_videobuf_setup(struct vb2_queue *vq,
				const struct v4l2_format *fmt,
				unsigned int *count, unsigned int *numplanes,
				unsigned int sizes[], void *alloc_ctxs[])
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev =
	    (struct unicam_camera_dev *)ici->priv;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						     icd->
						     current_fmt->host_fmt);

	pr_debug("%s(): -enter\n", __func__);

	if (bytes_per_line < 0)
		return bytes_per_line;

	*numplanes = 1;

	unicam_dev->sequence = 0;

	sizes[0] = bytes_per_line * icd->user_height;
#if defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	alloc_ctxs[0] = unicam_dev->alloc_ctx;
#endif

	if (!*count)
		*count = 2;

	pr_debug("%s(): no_of_buf=%d size=%u\n", __func__, *count, sizes[0]);

	pr_debug("%s(): -exit\n", __func__);
	return 0;
}

static int unicam_videobuf_prepare(struct vb2_buffer *vb)
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						     icd->
						     current_fmt->host_fmt);
	unsigned long size;

	pr_debug("%s(): vb=0x%p buf=0x%p, size=%lu\n",
		 __func__,
		 vb,
		 (void *)vb2_plane_dma_addr(vb, 0),
		 vb2_get_plane_payload(vb, 0));

	if (bytes_per_line < 0)
		return bytes_per_line;
	size = icd->user_height * bytes_per_line;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(icd->parent, "Buffer too small (%lu < %lu)\n",
			vb2_plane_size(vb, 0), size);
		return -ENOBUFS;
	}
	vb2_set_plane_payload(vb, 0, size);

	pr_debug("%s(): -exit\n", __func__);
	return 0;
}

/* should be called with unicam_dev->lock held */
static int unicam_camera_update_buf(struct unicam_camera_dev *unicam_dev)
{


	struct v4l2_subdev *sd = soc_camera_to_subdev(unicam_dev->icd);
	struct buffer_desc im0, dat0;
	dma_addr_t phys_addr;
	unsigned int line_stride;
	struct v4l2_format thumb_fmt;
	struct v4l2_pix_format *pix;
	int thumb = 0, ret;

	pr_debug("%s(): -enter\n", __func__);

	if (!unicam_dev->active) {
		pr_err("%s: Invalid buffer to Update", __func__);
		return -ENOMEM;
	}

	phys_addr = vb2_plane_dma_addr(unicam_dev->active, 0);
	pr_debug("%s(): updating buffer phys=0x%p",
		 __func__, (void *)phys_addr);
	if (!phys_addr) {
		unicam_dev->active = NULL;
		pr_err("No valid address. skip capture\n");
		return -ENOMEM;
	}
	/* For crop use-cases only linestride matters that too only for non-JPEG cases */

	/* stride is in bytes */
	if (unicam_dev->icd->current_fmt->code != V4L2_MBUS_FMT_JPEG_1X8) {
		if ((unicam_dev->crop.c.top == 0) ||
			(unicam_dev->crop.c.left == 0)) {
			/* Any one zero means no centering
			   Reject all such crop attempts */
			line_stride =
			    soc_mbus_bytes_per_line(unicam_dev->icd->user_width,
						    unicam_dev->icd->
						    current_fmt->host_fmt);
		} else {
			line_stride =
			    soc_mbus_bytes_per_line(unicam_dev->crop.c.width,
						    unicam_dev->icd->
						    current_fmt->host_fmt);
		}
		/* Non JPEG section of the code */
		/* image 0 */
		im0.start = (UInt32) phys_addr;
		im0.ls = (UInt32) line_stride;
		im0.size = line_stride * unicam_dev->icd->user_height;
		im0.wrap_en = 1;

		/* Coverity Fix: Dead Code */
		/* if(unicam_dev->b_mode == BUFFER_DOUBLE && phys_addr1){ */
			/* image 1 */
		/*	im1.start = phys_addr1;
			im1.ls = im0.ls;
			im1.size = im0.size;
			mm_csi0_update_addr(&im0, &im1, NULL, NULL);
		} else { */
		mm_csi0_update_addr(&im0, NULL, NULL, NULL);
		pr_debug("%s(): Adr 0x%x ls 0x%x size 0x%x\n",
			 __func__,
			 im0.start,
			 im0.ls, im0.size);
		/* } */
	} else {
		/* JPEG section always in DAT0 */
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
			line_stride = unicam_dev->icd->user_width;
			im0.start = (UInt32) phys_addr;
			im0.ls = (UInt32) pix->bytesperline;
			im0.size = pix->sizeimage;
			im0.wrap_en = 1;

			/* DAT0 to an address after THUMB */
			dat0.start = (UInt32) ((char *)phys_addr + pix->sizeimage);
			dat0.ls = line_stride;
			dat0.size = line_stride * unicam_dev->icd->user_height * 3/2;
			mm_csi0_update_addr(&im0, NULL, &dat0, NULL);
		} else {
			/* no thumbnail supported */
			/* don't set image0 since we are expecting data0
			 * to contain jpeg data
			 */
			dat0.start = (UInt32) phys_addr;
			dat0.ls = unicam_dev->icd->user_width;
			dat0.size = unicam_dev->icd->user_width * unicam_dev->icd->user_height * 3/2;
		}
	}
	pr_debug("-exit");
	return 0;
}

/* should be called with unicam_dev->lock held */
static int unicam_camera_capture(struct unicam_camera_dev *unicam_dev)
{

	int ret = 0;
	struct int_desc i_desc;
	pr_debug("%s(): -enter\n", __func__);

	if (!unicam_dev->active) {
		pr_debug("no active buffer");
		return ret;
	}
	if (unicam_dev->b_mode != BUFFER_TRIGGER)
		return 0;

	/* enable frame Interrupts */
	memset(&i_desc, 0x0, sizeof(struct int_desc));
	i_desc.fei = 1;
	i_desc.fsi = 1;
	ret = mm_csi0_trigger_cap();
/*	if (ret == 0) {
		atomic_set(&unicam_dev->cam_triggered, 1);
		if (atomic_read(&unicam_dev->streaming) == 1)
			mod_timer(&unicam_dev->unicam_timer, \
				jiffies + msecs_to_jiffies(UC_TIMEOUT_MS));
	}
*/
	pr_debug("%s(): -exit()\n", __func__);
	return ret;
}

static void unicam_videobuf_queue(struct vb2_buffer *vb)
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;
	struct int_desc idesc;

	pr_debug("%s(): vb=0x%p pbuf=0x%p size=%lu if_mode=%d streaming=%d\n",
		 __func__,
		 vb,
		 (void *)vb2_plane_dma_addr(vb, 0),
		 vb2_get_plane_payload(vb, 0),
		 unicam_dev->if_params.if_mode,
		 unicam_dev->streaming);
	/* pr_info("Q 0x%x\n", vb2_plane_paddr(vb, 0)); */
	spin_lock_irqsave(&unicam_dev->lock, flags);
	list_add_tail(&buf->queue, &unicam_dev->capture);
	if (unicam_dev->cap_mode && unicam_dev->cap_done) {
		pr_info("%s(): Capture mode and already captured\n", __func__);
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
		return;
	}
	if ((!unicam_dev->active)) {
		unicam_dev->active = vb;
		unicam_camera_update_buf(unicam_dev);
		if (atomic_read(&unicam_dev->streaming)) {
			mm_csi0_start_rx();
			/* set data capture */
			if (unicam_dev->if_params.if_mode == V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2) {
				idesc.fsi = 1;
				idesc.fei = 1;
				idesc.lci = 0;
				idesc.die = 1;
				idesc.dataline = 2;
				mm_csi0_config_int(&idesc, IMAGE_BUFFER);
				mm_csi0_config_int(&idesc, DATA_BUFFER);
				unicam_camera_capture(unicam_dev);
			} else {
				idesc.fsi = 0;
				idesc.fei = 0;
				idesc.lci = unicam_dev->icd->user_height;
				idesc.die = 0;
				idesc.dataline = 0;
				mm_csi0_config_int(&idesc, IMAGE_BUFFER);
			}
		}
	}
	spin_unlock_irqrestore(&unicam_dev->lock, flags);
	pr_debug("%s(): -exit", __func__);
}

static void unicam_videobuf_release(struct vb2_buffer *vb)
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);
	unsigned long flags;

	pr_debug("%s(): -enter\n", __func__);

	pr_debug("%s(): vb=0x%p pbuf=0x%p size=%lu\n",
		 __func__,
		 vb,
		 (void *)vb2_plane_dma_addr(vb, 0),
		 vb2_get_plane_payload(vb, 0));
	spin_lock_irqsave(&unicam_dev->lock, flags);

	if (buf->magic == UNICAM_BUF_MAGIC)
		list_del_init(&buf->queue);
	spin_unlock_irqrestore(&unicam_dev->lock, flags);

	pr_debug("-exit");
}

static int unicam_videobuf_init(struct vb2_buffer *vb)
{

	struct unicam_camera_buffer *buf = to_unicam_camera_vb(vb);

	INIT_LIST_HEAD(&buf->queue);
	buf->magic = UNICAM_BUF_MAGIC;
	return 0;
}

static void unicam_camera_get_frame_info_int(struct unicam_camera_dev *ucdev)
{
	int ret;
	struct soc_camera_device *icd = ucdev->icd;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);

	if (!ucdev->frame_info_en)
		return;
	ucdev->frame_info.timestamp = (struct timespec){-1, -1};
	ret = v4l2_subdev_call(sd, core, ioctl, VIDIOC_SENSOR_G_FRAME_INFO,
			&ucdev->frame_info);
	if (ret < 0)
		ucdev->frame_info_en = 0;
	/*
	  pr_debug("%s(): fi: exp=%d ag=%d lens=%d flash=[%d %d] tv=%ld",
		 __func__,
		 ucdev->frame_info.exposure,
		 ucdev->frame_info.an_gain,
		 ucdev->frame_info.lens_pos,
		 ucdev->frame_info.flash_mode,
		 ucdev->frame_info.flash_intensity,
		 ucdev->frame_info.timestamp.tv_nsec);
	*/
}

static void unicam_camera_set_frame_info_int(struct unicam_camera_dev *ucdev)
{
	int ret;
	struct soc_camera_device *icd = ucdev->icd;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct vb2_buffer *vb = ucdev->active;
	struct timeval *tv = &vb->v4l2_buf.timestamp;

	if (!ucdev->frame_info_en)
		return;
	ucdev->frame_info.timestamp =
		(struct timespec){tv->tv_sec, tv->tv_usec * 1000};
	/*
	pr_debug("%s(): fi: exp=%d ag=%d lens=%d flash=[%d %d] tv=%ld",
		 __func__,
		 ucdev->frame_info.exposure,
		 ucdev->frame_info.an_gain,
		 ucdev->frame_info.lens_pos,
		 ucdev->frame_info.flash_mode,
		 ucdev->frame_info.flash_intensity,
		 ucdev->frame_info.timestamp.tv_nsec);
	*/
	ret = v4l2_subdev_call(sd, core, ioctl, VIDIOC_SENSOR_S_FRAME_INFO,
			&ucdev->frame_info);
	if (ret < 0)
		ucdev->frame_info_en = 0;
}

static int unicam_videobuf_start_streaming_int(struct unicam_camera_dev \
						*unicam_dev, unsigned int count)
{

	struct soc_camera_device *icd = unicam_dev->icd;
	struct v4l2_subdev_sensor_interface_parms if_params;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct rx_stat_list rx;
	int ret;
	int lane_err;
	int thumb;
	u32 raw_rx;

	enum afe_num afe;
	enum host_mode hmode;
	enum csi1ccp2_clock_mode ccp2_clock;
	enum csi2_lanes lanes = CSI2_SINGLE_LANE;
	int vc = 0;
	int id = 0;
	struct int_desc idesc;
	struct lane_timing timing;

	pr_debug("%s(): +", __func__);
	unicam_dev->panic_count = 0;
	atomic_set(&unicam_dev->cam_triggered, 0);

	/* get the sensor interface information */
	ret = v4l2_subdev_call(sd, sensor, g_interface_parms, &if_params);
	if (ret < 0) {
		dev_err(unicam_dev->dev, "error on g_inferface_params(%d)\n",
			ret);
		return ret;
	}

	unicam_dev->if_params = if_params;

	/* set camera interface parameters */
	/* we only support serial and csi2 sensor */
	if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)) {
		hmode = CSI2;
		unicam_dev->b_mode = BUFFER_TRIGGER;
	} else if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1)) {
		hmode = CSI1CCP2;
		unicam_dev->b_mode = BUFFER_SINGLE;
	} else {
		dev_err(unicam_dev->dev,
			"CSI2 iface only supported, requested iface %d mode=%d\n",
			unicam_dev->if_params.if_type,
			unicam_dev->if_params.if_mode);
		return -EINVAL;
	}

	if (unicam_dev->if_params.parms.serial.channel == 0)
		afe = AFE0;
	else if (unicam_dev->if_params.parms.serial.channel == 1)
		afe = AFE1;
	else {
		dev_err(unicam_dev->dev,
			"receiver only supports two channels, request channel=%d\n",
			unicam_dev->if_params.parms.serial.channel);
		return -EINVAL;
	}

	if ((unicam_dev->if_params.if_type == V4L2_SUBDEV_SENSOR_SERIAL)
	    && (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)) {
		hmode = CSI2;
		if (unicam_dev->if_params.parms.serial.lanes == 1)
			lanes = CSI2_SINGLE_LANE;
		else if (unicam_dev->if_params.parms.serial.lanes == 2)
			lanes = CSI2_DUAL_LANE;
		else {
			dev_err(unicam_dev->dev,
				"receiver only supports max 2 lanes, requested lanes(%d)\n",
				unicam_dev->if_params.parms.serial.lanes);
			return -EINVAL;
		}
	} else {
		hmode = CSI1CCP2;
		ccp2_clock = DATA_CLOCK;

	}
	unicam_dev->handle = get_mm_csi0_handle(hmode, afe, lanes);
	if (unicam_dev->handle == NULL) {
		pr_err("Unable to get unicam handle\n");
		return -EBUSY;
	}
	ret = mm_csi0_init();
	if (ret) {
		pr_err("Unable to get unicam handle\n");
		mm_csi0_teardown();
		return -EINVAL;
	}
	mm_csi0_set_afe();

	/* Digital PHY Setup */
	/* Compulsary to get these values from sensor for CSI2 */
	/* Don't care for CCP2/CSI1 can send a struct with junk values
	   Will not be read */
	timing.hs_settle_time =
		unicam_dev->if_params.parms.serial.hs_settle_time;
	timing.hs_term_time = unicam_dev->if_params.parms.serial.hs_term_time;
	pr_debug("HS: settle_t = %d, term_t = %d\n",
			timing.hs_settle_time, timing.hs_term_time);
	ret = mm_csi0_set_dig_phy(&timing);
	if (ret) {
		pr_err("Wrong digital timing\n");
		mm_csi0_teardown();
		return -EINVAL;
	}

	/* Set Mode */
	mm_csi0_set_mode(ccp2_clock);

	/* check if frame_info is supported */
	unicam_dev->frame_info_en = 1;
	unicam_camera_get_frame_info_int(unicam_dev);

	/* set image identifier (CSI mode only) */

	/* if thumbnail is supported we expect
	 * thumbnail to be in image ptr format of thumbnails is yuv422
	 * format is checked in try format.
	 * in case where thumbnail is not supported we get jpeg
	 * image in data pointer. so we set the id as 0
	 */

	thumb = 0;
	ret = v4l2_subdev_call(sd, core, ioctl, VIDIOC_THUMB_SUPPORTED,
				(void *)&thumb);
	if (ret < 0)
		dev_warn(unicam_dev->dev,
			 "sensor returns error(%d) for VIDIOC_THUMB_SUPPORTED\n",
			 ret);

	if ((icd->current_fmt->code == V4L2_MBUS_FMT_JPEG_1X8) && (thumb == 0))
		id = 0;

	/* thumbnail not supported */
	/* RAW10 */
	else if ((icd->current_fmt->code == V4L2_MBUS_FMT_SBGGR10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGBRG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGRBG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SRGGB10_1X10))
		id = 0x2b;
	/* RAW8 */
	else if ((icd->current_fmt->code == V4L2_MBUS_FMT_SBGGR8_1X8)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGBRG8_1X8)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGRBG8_1X8)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SRGGB8_1X8))
		id = 0x2a;
	/* YUV422 */
	else
		id = 0x1e;

	if (icd->current_fmt->code == V4L2_MBUS_FMT_JPEG_1X8)
		pr_info("JPEG mode of capture !!!!\n");

	if (unicam_dev->if_params.if_mode ==
		V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1){
		id = 0;
	}
	ret = mm_csi0_cfg_image_id(vc, id);
	if (ret) {
		pr_err("Wrong Image IDs set for a given mode\n");
		mm_csi0_teardown();
		return -EINVAL;
	}
	ret = 0;

	/* pipelince decode */
	/* Set vertical windowing */
	if (unicam_dev->cap_mode) {
		ret |=
		mm_csi0_set_windowing_vertical(unicam_dev->crop.c.top,
				(unicam_dev->crop.c.top
				 + unicam_dev->crop.c.height));
	} else {
		ret |= mm_csi0_set_windowing_vertical(0, 0);
	}

	/* UNPACK */
	if ((icd->current_fmt->code == V4L2_MBUS_FMT_SBGGR10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGBRG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGRBG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SRGGB10_1X10)) {
#ifdef UNPACK_RAW10_TO_16BITS
		ret |= mm_csi0_cfg_pipeline_unpack(PIX_UNPACK_RAW10);
#endif
	} else {
		ret |= mm_csi0_cfg_pipeline_unpack(PIX_UNPACK_NONE);
	}

	/* DPCM decode */
	ret |= mm_csi0_cfg_pipeline_dpcm_dec(DPCM_DEC_NONE);

	/* Set horizontal windowing */
	if (unicam_dev->cap_mode) {
		ret |=
			mm_csi0_set_windowing_horizontal(
					unicam_dev->crop.c.left,
					(unicam_dev->crop.c.left
					 + unicam_dev->crop.c.width));
	} else {
		ret |= mm_csi0_set_windowing_horizontal(0, 0);
	}

	/* DPCM encode */
	ret |= mm_csi0_cfg_pipeline_dpcm_enc(DPCM_ENC_NONE);

	/* PACK */
	if ((icd->current_fmt->code == V4L2_MBUS_FMT_SBGGR10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGBRG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SGRBG10_1X10)
		  || (icd->current_fmt->code == V4L2_MBUS_FMT_SRGGB10_1X10)) {
#ifdef UNPACK_RAW10_TO_16BITS
		ret |= mm_csi0_cfg_pipeline_pack(PIX_PACK_16);
#endif
	} else {
		ret |= mm_csi0_cfg_pipeline_pack(PIX_PACK_NONE);
	}

	/* FSP encode */
	ret |= mm_csi0_enable_fsp_ccp2();

	if (ret) {
		pr_err("Something wrong with pipeline config .. pl go check\n");
		mm_csi0_teardown();
		return -EINVAL;
	}

	/* Output engine */
	mm_csi0_buffering_mode(unicam_dev->b_mode);
	mm_csi0_rx_burst();
	mm_csi0_enable_unicam();

	/* start sensor streaming */
	ret = v4l2_subdev_call(sd, video, s_stream, 1);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_err(unicam_dev->dev, "error on s_stream(%d)\n", ret);
		return ret;
	}

	udelay(30);

	if (unicam_dev->active) {
		/* unicam_camera_update_buf(unicam_dev); */
		mm_csi0_start_rx();
		/* set data capture */
		if (unicam_dev->if_params.if_mode == V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2) {
			idesc.fsi = 1;
			idesc.fei = 1;
			idesc.lci = 0;
			idesc.die = 1;
			idesc.dataline = 2;
			mm_csi0_config_int(&idesc, IMAGE_BUFFER);
			mm_csi0_config_int(&idesc, DATA_BUFFER);
			unicam_camera_capture(unicam_dev);
		} else {
			idesc.fsi = 0;
			idesc.fei = 0;
			idesc.lci = unicam_dev->icd->user_height;
			idesc.die = 0;
			idesc.dataline = 0;
			mm_csi0_config_int(&idesc, IMAGE_BUFFER);
		}

		atomic_set(&unicam_dev->streaming, 1);

		/* Error check code */
		/* Check RX state for errors */
		memset(&rx, 0x00, sizeof(struct rx_stat_list));
		raw_rx = mm_csi0_get_rx_stat(&rx, 1);
		pr_info("raw_rx is 0x%x", raw_rx);

		if (unicam_dev->if_params.if_mode ==
			V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1){
			if (raw_rx & RX_CCP2_ERROR_MASK) {
				pr_info("Error seen pl check for CCP2 errors 0x%x\n",
						raw_rx);
				if (rx.ssc)
					pr_info("Shifted sync code in CCP2\n");
				if (rx.ofo || rx.ifo || rx.bfo || rx.dl)
					pr_info("FIFO errors or data lost\n");
				if (rx.crce)
					pr_info("CRC error\n");
			}
		} else {
			if (raw_rx & RX_CSI2_ERROR_MASK) {
				pr_info("Error seen pl check for CSI2 errors 0x%x\n",
						raw_rx);
				if (rx.sbe || rx.pbe || rx.hoe || rx.ple)
					pr_info("Specific errors in CSI2\n");
				if (rx.ofo || rx.ifo || rx.bfo || rx.dl)
					pr_info("FIFO errors or data lost\n");
				if (rx.crce)
					pr_info("CRC error\n");
			}
		}
		/* Check lane transitions */
		if (unicam_dev->if_params.if_mode ==
			V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2) {
			lane_err = mm_csi0_get_trans();
			if (lane_err) {
				pr_err("Lane errors seen 0x%x\n", lane_err);
				/* return -EFAULT;*/
			}
		}
	}

#ifdef UNICAM_DEBUG
	unicam_reg_dump_dbg();
#endif

/*	if (unicam_dev->active)
		if (unicam_dev->if_params.if_mode == \
				V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2)
			mod_timer(&unicam_dev->unicam_timer, \
				jiffies + msecs_to_jiffies(UC_TIMEOUT_MS));
*/
	pr_debug("-exit");
	return 0;
}

int unicam_videobuf_start_streaming(struct vb2_queue *q, unsigned int count)
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int ret = 0;

	pr_debug("%s(): starting\n", __func__);
	if (!atomic_read(&unicam_dev->streaming))
		ret = unicam_videobuf_start_streaming_int(unicam_dev, count);
	else
		pr_err("unicam_videobuf_start_streaming: already started\n");

	return ret;
}

static int unicam_videobuf_stop_streaming_int(struct unicam_camera_dev \
				*unicam_dev)
{

	struct soc_camera_device *icd = unicam_dev->icd;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int ret = 0;
	unsigned long flags;
	struct rx_stat_list rx;

	/* grab the lock */
	spin_lock_irqsave(&unicam_dev->lock, flags);
	pr_debug("%s(): stopping stream\n", __func__);
	if (!atomic_read(&unicam_dev->streaming)) {
		pr_err("stream already turned off\n");
		goto out;
	}
	if (unicam_dev->active) {
		atomic_set(&unicam_dev->stopping, 1);
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
		ret = down_timeout(&unicam_dev->stop_sem,
				msecs_to_jiffies(500));
		atomic_set(&unicam_dev->stopping, 0);
		if (ret == -ETIME) {
			pr_err("%s(): semaphore timed out waiting to STOP\n",
			       __func__);
#ifdef UNICAM_DEBUG
			unicam_reg_dump_dbg();
#endif
		}
	} else {
		spin_unlock_irqrestore(&unicam_dev->lock, flags);
	}
	usleep_range(50, 60); /*TODO: Need to double-check with ASIC team*/
	spin_lock_irqsave(&unicam_dev->lock, flags);

	unicam_stop();

	/* Restart rx stat */
	mm_csi0_get_rx_stat(&rx, 1);
	/* Don't bother what values were returned */
	mm_csi0_teardown();
	unicam_dev->active = NULL;
	atomic_set(&unicam_dev->streaming, 0);
	memset(&unicam_dev->crop, 0x00, sizeof(struct v4l2_crop));
	unicam_dev->cap_done = 0;
	unicam_dev->cap_mode = 0;

out:
	pr_debug("%s(): -exit", __func__);
	atomic_set(&unicam_dev->cam_triggered, 0);
	spin_unlock_irqrestore(&unicam_dev->lock, flags);

	/* stop sensor streaming after UNICAM is disabled */
	ret = v4l2_subdev_call(sd, video, s_stream, 0);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		pr_err("failed to stop sensor streaming\n");
		ret = -1;
	}

	return ret;
}

int unicam_videobuf_stop_streaming(struct vb2_queue *q)
{

	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int ret = 0;

	if (atomic_read(&unicam_dev->streaming))
		ret = unicam_videobuf_stop_streaming_int(unicam_dev);
	else
		pr_err("unicam_videobuf_start_streaming: already stopped\n");

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

	pr_debug("%s(): -enter\n", __func__);
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_READ;
	q->drv_priv = icd;
	q->ops = &unicam_videobuf_ops;
#if defined(CONFIG_VIDEOBUF2_DMA_RESERVED)
	pr_info("Unicam uses vb2-dma-reserved\n");
	q->mem_ops = &vb2_dma_reserved_memops;
#elif defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	pr_info("Unicam uses vb2-dma-contig\n");
	q->mem_ops = &vb2_dma_contig_memops;
#endif
	q->buf_struct_size = sizeof(struct unicam_camera_buffer);
	q->timestamp_type = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	pr_debug("-exit");
	return vb2_queue_init(q);
}

static int unicam_camera_set_bus_param(struct soc_camera_device *icd)
{

	pr_debug("-enter");
	pr_debug("-exit");
	return 0;
}

static int unicam_camera_querycap(struct soc_camera_host *ici,
				  struct v4l2_capability *cap)
{

	pr_debug("%s(): -enter\n", __func__);
	/* cap->name is set by the firendly caller:-> */
	strlcpy(cap->card, "Unicam Camera", sizeof(cap->card));
	cap->version = KERNEL_VERSION(0, 1, 0);
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;

	pr_debug("%s(): -exit\n", __func__);
	return 0;
}

static unsigned int unicam_camera_poll(struct file *file, poll_table *pt)
{

	struct soc_camera_device *icd = file->private_data;

	pr_debug("%s(): -enter\n", __func__);
	pr_debug("%s(): -exit\n", __func__);
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
	int thumb = 0;
	int ret;

	pr_debug("%s(): -enter\n", __func__);
	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (!xlate) {
		dev_warn(icd->parent, "Format %x not found\n", pixfmt);
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
		dev_err(icd->parent, "Field type %d unsupported.\n",
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
				dev_err(icd->parent,
					"sensor driver should report thumbnail format\n");
				return -EINVAL;
			}
			thumb_pix = &thumb_fmt.fmt.pix;
			switch (thumb_pix->pixelformat) {
			case V4L2_PIX_FMT_YUYV:
			case V4L2_PIX_FMT_UYVY:
				pr_debug
				    ("sensor supports thumbnail %c%c%c%c format",
				     pixfmtstr(thumb_pix->pixelformat));
				break;
			default:
				dev_err(icd->parent,
					"sensor thumbnail format %c%c%c%c not supported\n",
					pixfmtstr(thumb_pix->pixelformat));
				return -EINVAL;
			}
		} else
			pr_debug
			    ("sensor doesnot support thumbnail (thumb=%d, ret=%d)\n",
			     thumb, ret);

	case V4L2_MBUS_FMT_YUYV8_2X8:
	case V4L2_MBUS_FMT_UYVY8_2X8:
		/* Above formats are supported */
		break;
	case V4L2_MBUS_FMT_SBGGR10_1X10:
	case V4L2_MBUS_FMT_SGBRG10_1X10:
	case V4L2_MBUS_FMT_SGRBG10_1X10:
	case V4L2_MBUS_FMT_SRGGB10_1X10:
		/* ISP needs 32 byte input boundary on line stride */
		pix->bytesperline = ((((pix->width * 10) >> 3) + 31) & ~(31));
#ifdef UNPACK_RAW10_TO_16BITS
		pix->bytesperline = ((pix->width * 2) + 31) & ~(31);
#endif
		break;
		/* ISP needs 32 byte input boundary on line stride */
	case V4L2_MBUS_FMT_SBGGR8_1X8:
	case V4L2_MBUS_FMT_SGBRG8_1X8:
	case V4L2_MBUS_FMT_SGRBG8_1X8:
	case V4L2_MBUS_FMT_SRGGB8_1X8:
		pix->bytesperline = ((pix->width + 31) & ~(31));
		break;
	default:
		dev_err(icd->parent, "Sensor format code %d unsupported.\n",
			mf.code);
		return -EINVAL;
	}

	pr_debug("%s(): trying format=%c%c%c%c res=%dx%d success=%d\n",
		 __func__,
		 pixfmtstr(pixfmt), mf.width, mf.height, ret);
	pr_debug("%s(): -exit\n", __func__);
	return ret;
}

static int unicam_stop()
{

	struct int_desc idesc;

	mm_csi0_stop_rx();
	memset(&idesc, 0x00, sizeof(struct int_desc));
	mm_csi0_config_int(&idesc, IMAGE_BUFFER);
	mm_csi0_config_int(&idesc, DATA_BUFFER);
	return 0;
}

static int unicam_camera_s_ctrl(struct v4l2_ctrl *ctl);

static const struct v4l2_ctrl_ops unicam_camera_ctrl_ops = {
	.s_ctrl = unicam_camera_s_ctrl,
};

static const struct v4l2_ctrl_config unicam_camera_controls[] = {
	{
		.ops = &unicam_camera_ctrl_ops,
		.id = V4L2_CID_CAM_CAPTURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Cam Capture",
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 1,
		.flags = 0,
	},
	{
		.ops = &unicam_camera_ctrl_ops,
		.id = V4L2_CID_CAM_CAPTURE_DONE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Cam Capture Done",
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 1,
		.flags = 0,
	},
};

static struct soc_camera_device *ctrl_to_icd(struct v4l2_ctrl *ctrl)
{
	return container_of(ctrl->handler,
				struct soc_camera_device, ctrl_handler);
}

static int unicam_camera_s_ctrl(struct v4l2_ctrl *ctl)
{
	struct soc_camera_device *icd = ctrl_to_icd(ctl);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int ret = 0;

	if (ctl == NULL) {
		pr_err("Wrong host ops s_ctrl\n");
		return -EINVAL;
	}
	switch (ctl->id) {
	case V4L2_CID_CAM_CAPTURE:
		pr_info("V4L2_CID_CAM_CAPTURE\n");
		unicam_dev->cap_mode = 1;
		unicam_dev->cap_done = 0;

		/*for camera driver also invoke s_ctrl */
		ret = -ENOIOCTLCMD;
		break;
	case V4L2_CID_CAM_CAPTURE_DONE:
		pr_info("V4L2_CID_CAM_CAPTURE_DONE\n");
		unicam_dev->cap_mode = 0;

		/*for camera driver also invoke s_ctrl */
		ret = -ENOIOCTLCMD;
		break;
	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}

/* This method shall be used only for unicam windowing
   for zoom use-case */
static int unicam_camera_get_crop(struct soc_camera_device *icd,
				 struct v4l2_crop *crop)
{

	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	if (crop != NULL)
		*crop = unicam_dev->crop;

	return 0;
}
static int unicam_camera_set_crop(struct soc_camera_device *icd,
				const struct v4l2_crop *crop)
{

	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	if (crop == NULL)
		return -EINVAL;
	unicam_dev->crop = *crop;
	return 0;
}
static int unicam_camera_set_fmt_int(struct unicam_camera_dev *unicam_dev)
{

	struct soc_camera_device *icd = unicam_dev->icd;
	struct device *dev = icd->parent;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate = NULL;
	struct v4l2_format *f = &(unicam_dev->active_fmt);
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	int ret;
	u32 skip_frames = 0;

	pr_debug("-enter");
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
	unicam_dev->curr = 0;
	unicam_dev->buff[0] = NULL;
	unicam_dev->buff[1] = NULL;

	pix->width = mf.width;
	pix->height = mf.height;
	pix->field = mf.field;
	pix->colorspace = mf.colorspace;
	icd->current_fmt = xlate;

	/* Initialize crop window for now */
	unicam_dev->crop.c.width = pix->width;
	unicam_dev->crop.c.height = pix->height;
	unicam_dev->crop.c.top = unicam_dev->crop.c.left = 0;

	pr_info("format set to %c%c%c%c res=%dx%d success=%d",
		pixfmtstr(pix->pixelformat), pix->width, pix->height, ret);
	pr_debug("-exit");
	return ret;
}


static int unicam_camera_set_fmt(struct soc_camera_device *icd,
				 struct v4l2_format *f)
{

	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int ret;

	unicam_dev->active_fmt = *f;
	ret = unicam_camera_set_fmt_int(unicam_dev);
	return ret;
}

static int unicam_camera_add_device(struct soc_camera_device *icd)
{

	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;
	int err = 0;

	if (unicam_dev->icd) {
		dev_warn(icd->parent,
			 "Unicam camera driver already attached to another client\n");
		err = -EBUSY;
		goto eicd;
	}

	/* register irq */
	err =
	    request_threaded_irq(unicam_dev->irq, unicam_camera_isr,
			unicam_camera_isr_bh,
			IRQF_DISABLED | IRQF_SHARED, UNICAM_CAM_DRV_NAME,
			unicam_dev);
	if (err) {
		dev_err(icd->parent, "cound not install irq %d\n",
			unicam_dev->irq);
		err = -ENODEV;
		goto eirq;
	}

	unicam_dev->icd = icd;

	dev_info(icd->parent,
		 "Unicam Camera driver attached to camera %d\n", icd->devnum);
#if 0
	for (i = 0; i < ARRAY_SIZE(unicam_camera_controls); ++i)
		v4l2_ctrl_new_custom(&icd->ctrl_handler,
					&unicam_camera_controls[i], NULL);

	if (icd->ctrl_handler.error)
		return icd->ctrl_handler.error;
#endif
eirq:
eicd:
	return err;
}

static void unicam_camera_remove_device(struct soc_camera_device *icd)
{

	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct unicam_camera_dev *unicam_dev = ici->priv;

	BUG_ON(icd != unicam_dev->icd);
	pr_err("unicam_camera_remove_device\n");

	if (atomic_read(&unicam_dev->streaming)) {
		/* stop streaming */
		/* we should call streamoff from queue operations */
		unicam_videobuf_stop_streaming(&icd->vb2_vidq);
	}

	free_irq(unicam_dev->irq, unicam_dev);

	unicam_dev->icd = NULL;

	dev_info(icd->parent,
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
	.set_livecrop = unicam_camera_set_crop,
	.set_crop = unicam_camera_set_crop,
	.get_crop = unicam_camera_get_crop,
};


static irqreturn_t unicam_camera_isr(int irq, void *arg)
{
	struct unicam_camera_dev *unicam_dev = (struct unicam_camera_dev *)arg;
	struct v4l2_subdev *sd = soc_camera_to_subdev(unicam_dev->icd);
	int ret;
	struct int_desc idesc;
	struct rx_stat_list rx;
	u32 isr_status, raw_stat;
	struct buffer_desc im0;
	dma_addr_t dma_addr;
	unsigned long flags;

	/* has the interrupt occured for Channel 0? */
	memset(&rx, 0, sizeof(struct rx_stat_list));
	memset(&idesc, 0, sizeof(idesc));
	raw_stat = mm_csi0_get_rx_stat(&rx, 1);
	if (atomic_read(&unicam_dev->streaming) == 0) {
		isr_status = mm_csi0_get_int_stat(&idesc, 1);
		pr_err("ISR triggered after stop stat=0x%x istat=0x%x\n",
			raw_stat, isr_status);

		goto out;
	} else if (rx.is) {
		isr_status = mm_csi0_get_int_stat(&idesc, 1);
		pr_debug("%s(): fsi=%d fei=%d lci=%d\n",
			 __func__, idesc.fsi, idesc.fei, idesc.lci);
		if (idesc.fsi) {
			if (rx.ps)
				pr_info("Panic at frame start\n");
		}

		if (idesc.fei || idesc.lci) {
			struct vb2_buffer *vb = unicam_dev->active;
			/* FS and FE handling */
			if (rx.ps)
				pr_info("Panic at frame or lineend\n");
			atomic_set(&unicam_dev->cam_triggered, 0);
			pr_debug("frame received\n");
			if (!vb)
				goto out;

			if (likely(unicam_dev->skip_frames <= 0)) {
				struct v4l2_control ctrl;
				int ret = -1;
				ctrl.value = 0;
				ctrl.id = V4L2_CID_CAMERA_READ_MODE_CHANGE_REG;
				ret = v4l2_subdev_call(sd, core, g_ctrl, &ctrl);

				if ((ret >= 0) && (ctrl.value > 0)) {
					/* capture mode is not ready yet */
					unicam_dev->skip_frames = ctrl.value;
					pr_info("%s: sensor mode change in process ,need_skip_frame=%d\n",
					__func__, ctrl.value);
				}
			}
			if (likely(unicam_dev->skip_frames <= 0)) {
				spin_lock_irqsave(&unicam_dev->lock, flags);
				list_del_init(&to_unicam_camera_vb(vb)->queue);
				spin_unlock_irqrestore(&unicam_dev->lock,
								flags);
				do_gettimeofday(&vb->v4l2_buf.timestamp);
				vb->v4l2_planes[0].bytesused = 0;

				if (unicam_dev->icd->current_fmt->code ==
				    V4L2_MBUS_FMT_JPEG_1X8) {
				} else {
					ret = 1;
				}
				unicam_camera_set_frame_info_int(unicam_dev);
				vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
				spin_lock_irqsave(&unicam_dev->lock, flags);
				if (atomic_read(&unicam_dev->stopping) == 1) {
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
				if (unicam_dev->cap_mode == 1) {
					unicam_dev->cap_done = 1;
					goto out;
				}
			} else {
				unicam_dev->skip_frames--;
			}

			if (unicam_dev->active) {
				dma_addr = vb2_plane_dma_addr(
					unicam_dev->active, 0);
				if (!dma_addr || dma_addr & 0xF) {
					unicam_dev->active = NULL;
					pr_err("ISR: No valid address.skip capture\n");
					goto out;
				}
				im0.start = dma_addr;
				im0.size = unicam_dev->icd->user_width *
					unicam_dev->icd->user_height * 2;
				im0.ls = unicam_dev->icd->user_width * 2;
				if (unicam_dev->icd->current_fmt->code !=
					V4L2_MBUS_FMT_JPEG_1X8) {
					ret = mm_csi0_update_one(&im0,
						unicam_dev->curr, IMAGE_BUFFER);
				} else {
					ret = mm_csi0_update_one(&im0,
						unicam_dev->curr, DATA_BUFFER);
				}
				if (!ret)
					unicam_camera_capture(unicam_dev);
				else
					pr_err("Unicam Camera: mm_csi0_update_one fail\n");
			} else {
				pr_err("ran out of buffers\n");
			}
		}

		if (idesc.fei) {
			/* notify driver there's frame end*/
			ret = v4l2_subdev_call(sd, core, ioctl,
					       VIDIOC_SENSOR_FRAME_IRQ,
					       (void *)(isr_status &
						       CAM_ISTA_FEI_MASK));
		}

	}

out:
	return (idesc.fsi && unicam_dev->skip_frames <= 0) ?
		IRQ_WAKE_THREAD : IRQ_HANDLED;
}

static irqreturn_t unicam_camera_isr_bh(int irq, void *arg)
{
	struct unicam_camera_dev *unicam_dev = (struct unicam_camera_dev *)arg;
	unicam_camera_get_frame_info_int(unicam_dev);
#if 0 /* DEBUG - metadata validation */
	{
		struct v4l2_subdev *sd = soc_camera_to_subdev(unicam_dev->icd);
		struct v4l2_control gc = { .id = V4L2_CID_GAIN};
		v4l2_subdev_call(sd, core, g_ctrl, &gc);
		pr_info("metadata exposure LOCAL:%d CID_QUERY:%d -- %s",
				unicam_dev->frame_info.an_gain, gc.value,
				(unicam_dev->frame_info.an_gain == gc.value) ?
						"PASS" : "FAIL");
	}
#endif
	return IRQ_HANDLED;
}


static int unicam_camera_probe(struct platform_device *pdev)
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

	unicam_dev->dev = &pdev->dev;
	unicam_dev->irq = irq;
	soc_host = &unicam_dev->soc_host;
	soc_host->drv_name = UNICAM_CAM_DRV_NAME;
	soc_host->ops = &unicam_soc_camera_host_ops;
	soc_host->priv = unicam_dev;
	soc_host->v4l2_dev.dev = &pdev->dev;
	soc_host->nr = pdev->id;
	atomic_set(&unicam_dev->stopping, 0);
	sema_init(&unicam_dev->stop_sem, 0);

#if defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	unicam_dev->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(unicam_dev->alloc_ctx)) {
		err = PTR_ERR(unicam_dev->alloc_ctx);
		goto eallocctx;
	}
#endif

	err = soc_camera_host_register(soc_host);
	if (err)
		goto ecamhostreg;
	return 0;

ecamhostreg:
#if defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	vb2_dma_contig_cleanup_ctx(unicam_dev->alloc_ctx);
eallocctx:
#endif
	vfree(unicam_dev);
ealloc:
edev:
	return err;
}

static int unicam_camera_remove(struct platform_device *pdev)
{

	/*TODO */
	return 0;
}

static struct platform_driver unicam_camera_driver = {
	.driver = {
		   .name = UNICAM_CAM_DRV_NAME,
		   },
	.probe = unicam_camera_probe,
	.remove = unicam_camera_remove,
};

static int __init unicam_camera_init(void)
{

	return platform_driver_register(&unicam_camera_driver);
}

static void __exit unicam_camera_exit(void)
{

	platform_driver_unregister(&unicam_camera_driver);
}

late_initcall(unicam_camera_init);
module_exit(unicam_camera_exit);

MODULE_DESCRIPTION("Unicam Camera Host driver");
MODULE_AUTHOR("Broadcom Corporation");
