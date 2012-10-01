/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
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

#include <linux/init.h>
#include <linux/module.h>

#include <media/v4l2-ioctl.h>
#include <media/videobuf2-dma-contig.h>

#include "unicam.h"
#include "unicam_video.h"

static struct unicam_format_info formats[] = {
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_PIX_FMT_UYVY, 16},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_PIX_FMT_YUYV, 16},
};

/*
 * unicam_video_mbus_to_pix - convert v4l2_mbus_framefmt to v4l2_pix_format
 * @video: unicam video instance.
 * @mbus: v4l2_mbus_framefmt format(input)
 * @pix: v4l2_pix_format format(output)
 *
 * Fill the output pix structure with information from the input mbus format.
 * The bytesperline and sizeimage fields are computed from the requested bytes
 * per line valure int the pix format and information frome the video instance.
 *
 * Return the number of padding bytes at end of line.
 */

static unsigned int unicam_video_mbus_to_pix(const struct unicam_video *video,
		const struct v4l2_mbus_framefmt *mbus,
		struct v4l2_pix_format *pix)
{
	unsigned int bpl = pix->bytesperline;
	unsigned int min_bpl;
	unsigned int i;

	memset(pix, 0, sizeof(*pix));
	pix->width = mbus->width;
	pix->height = mbus->height;

	for (i = 0; i < ARRAY_SIZE(formats)-1; ++i) {
		if (formats[i].code ==  mbus->code)
			break;
	}

	min_bpl = pix->width * ALIGN(formats[i].bpp, 8) / 8;

	if (video->bpl_max)
		bpl = clamp(bpl, min_bpl, video->bpl_max);
	else
		bpl = min_bpl;

	if (!video->bpl_zero_padding || bpl != min_bpl)
		bpl = ALIGN(bpl, video->bpl_alignment);

	pix->pixelformat = formats[i].pixelformat;
	pix->bytesperline = bpl;
	pix->sizeimage = pix->bytesperline * pix->height;
	pix->colorspace = mbus->colorspace;
	pix->field = mbus->field;

	return bpl - min_bpl;
}

static void unicam_video_pix_to_mbus(const struct v4l2_pix_format *pix,
		struct v4l2_mbus_framefmt *mbus)
{
	unsigned int i;

	memset(mbus, 0, sizeof(*mbus));
	mbus->width = pix->width;
	mbus->height = pix->height;

	for (i = 0; i < ARRAY_SIZE(formats); ++i) {
		if (formats[i].pixelformat == pix->pixelformat)
			break;
	}

	if (WARN_ON(i == ARRAY_SIZE(formats)))
		return;

	mbus->code = formats[i].code;
	mbus->colorspace = pix->colorspace;
	mbus->field = pix->field;
}

static struct v4l2_subdev *
unicam_video_remote_subdev(struct unicam_video *video, u32 *pad)
{
	struct media_pad *remote;

	remote = media_entity_remote_source(&video->pad);

	if (remote == NULL ||
		media_entity_type(remote->entity) != MEDIA_ENT_T_V4L2_SUBDEV)
		return NULL;

	if (pad)
		*pad = remote->index;

	return media_entity_to_v4l2_subdev(remote->entity);
}

static int __unicam_video_get_format(struct unicam_video *video,
		struct v4l2_format *format)
{
	struct v4l2_subdev_format fmt;
	struct v4l2_subdev *subdev;
	u32 pad;
	int ret;

	subdev = unicam_video_remote_subdev(video, &pad);
	if (subdev == NULL)
		return -EINVAL;

	mutex_lock(&video->mutex);

	fmt.pad = pad;
	fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	ret = v4l2_subdev_call(subdev, pad, get_fmt, NULL, &fmt);
	if (ret == -ENOIOCTLCMD)
		ret = -EINVAL;

	mutex_unlock(&video->mutex);

	if (ret)
		return ret;

	format->type = video->type;
	return unicam_video_mbus_to_pix(video, &fmt.format, &format->fmt.pix);
}

static int unicam_video_check_format(struct unicam_video *video,
		struct unicam_video_fh *handle)
{
	struct v4l2_format format;
	int ret;

	memcpy(&format, &handle->format, sizeof(format));
	ret = __unicam_video_get_format(video, &format);
	if (ret < 0)
		return ret;

	if (handle->format.fmt.pix.pixelformat != format.fmt.pix.pixelformat ||
		handle->format.fmt.pix.height != format.fmt.pix.height ||
		handle->format.fmt.pix.width != format.fmt.pix.width ||
		handle->format.fmt.pix.bytesperline !=
		format.fmt.pix.bytesperline ||
		handle->format.fmt.pix.sizeimage != format.fmt.pix.sizeimage)
		return -EINVAL;

	return ret;
}

/*---------------------------------------------
 * VB2 Queue operations
 * --------------------------------------------
 */

static int unicam_video_queue_setup(struct vb2_queue *vq,
		unsigned int *count, unsigned int *num_planes,
		unsigned long sizes[], void *alloc_ctxs[])
{
	struct unicam_video_fh *handle = vb2_get_drv_priv(vq);
	struct unicam_video *video = handle->video;

	*num_planes = 1;

	sizes[0] = handle->format.fmt.pix.sizeimage;
	if (sizes[0] == 0)
		return -EINVAL;

	alloc_ctxs[0] = video->alloc_ctx;

	/* limit to 2-6 buffers */
	*count = clamp_t(unsigned int, *count, 2, 6);

	printk(KERN_INFO"no_of_buf=%d size=%lu\n", *count, sizes[0]);

	return 0;
}

static int unicam_video_buf_prepare(struct vb2_buffer *vb)
{
	struct unicam_video_fh *handle = vb2_get_drv_priv(vb->vb2_queue);
	struct unicam_video *video = handle->video;
	unsigned long size = handle->format.fmt.pix.sizeimage;
	struct unicam_buffer *buffer =
		container_of(vb, struct unicam_buffer, vb);
	dma_addr_t addr;

	if (vb2_plane_size(vb, 0) < size) {
		dev_err(video->unicam->dev, "buffer too small (%lu < %lu)\n",
				vb2_plane_size(vb, 0), size);
		return -ENOBUFS;
	}

	addr = vb2_dma_contig_plane_paddr(vb, 0);
	if (!IS_ALIGNED(addr, 32)) {
		dev_dbg(video->unicam->dev, "buffer address must be aligned\n");
		return -EINVAL;
	}

	vb2_set_plane_payload(vb, 0, size);

	buffer->unicam_addr = addr;
	return 0;
}

static void unicam_video_buf_queue(struct vb2_buffer *vb)
{
	struct unicam_video_fh *handle  = vb2_get_drv_priv(vb->vb2_queue);
	struct unicam_video *video = handle->video;
	struct unicam_buffer *buffer =
		container_of(vb, struct unicam_buffer, vb);
	struct unicam_pipeline *pipe = &video->pipe;
	unsigned int empty;
	unsigned long flags;

	spin_lock_irqsave(&video->qlock, flags);
	empty = list_empty(&video->dmaqueue);
	list_add_tail(&buffer->list, &video->dmaqueue);
	spin_unlock_irqrestore(&video->qlock, flags);

	if (empty) {
		spin_lock_irqsave(&pipe->lock, flags);
		video->ops->queue(video, buffer);
		spin_unlock_irqrestore(&pipe->lock, flags);
	}
}

static void unicam_video_buf_cleanup(struct vb2_buffer *vb)
{
	struct unicam_buffer *buffer =
		container_of(vb, struct unicam_buffer, vb);

	buffer->unicam_addr = 0;
}

static struct vb2_ops unicam_video_vb2ops = {
	.queue_setup = unicam_video_queue_setup,
	.buf_prepare = unicam_video_buf_prepare,
	.buf_queue = unicam_video_buf_queue,
	.buf_cleanup = unicam_video_buf_cleanup,
};

struct unicam_buffer *kona_unicam_video_buffer_next(struct unicam_video *video)
{
	struct unicam_pipeline *pipe = &video->pipe;
	struct unicam_buffer *buffer;
	unsigned long flags;
	struct timespec ts;

	spin_lock_irqsave(&video->qlock, flags);
	if (WARN_ON(list_empty(&video->dmaqueue))) {
		spin_unlock_irqrestore(&video->qlock, flags);
		return NULL;
	}

	buffer = list_first_entry(&video->dmaqueue, struct unicam_buffer,
			list);
	list_del(&buffer->list);
	spin_unlock_irqrestore(&video->qlock, flags);

	ktime_get_ts(&ts);
	buffer->vb.v4l2_buf.timestamp.tv_sec = ts.tv_sec;
	buffer->vb.v4l2_buf.timestamp.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	buffer->vb.v4l2_buf.sequence = atomic_inc_return(&pipe->frame_number);

	vb2_buffer_done(&buffer->vb,
		pipe->error ? VB2_BUF_STATE_ERROR : VB2_BUF_STATE_DONE);


	spin_lock_irqsave(&video->qlock, flags);
	if (list_empty(&video->dmaqueue)) {
		spin_unlock_irqrestore(&video->qlock, flags);
		return NULL;
	}

	buffer = list_first_entry(&video->dmaqueue, struct unicam_buffer,
			list);
	spin_unlock_irqrestore(&video->qlock, flags);
	buffer->vb.state = VB2_BUF_STATE_ACTIVE;
	return buffer;
}

/*----------------------------------------------
 * V4L2 ioctl operations
 * ---------------------------------------------
 */
static int
unicam_video_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{
	struct unicam_video *video = video_drvdata(file);

	strlcpy(cap->driver, UNICAM_VIDEO_DRIVER_NAME, sizeof(cap->driver));
	strlcpy(cap->card, video->video.name, sizeof(cap->card));
	strlcpy(cap->bus_info, "media", sizeof(cap->bus_info));

	if (video->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	else
		cap->capabilities = V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_STREAMING;

	return 0;
}

static int
unicam_video_get_format(struct file *file, void *fh, struct v4l2_format *format)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);
	struct unicam_video *video = video_drvdata(file);

	if (format->type != video->type)
		return -EINVAL;

	mutex_lock(&video->mutex);
	*format = handle->format;
	mutex_unlock(&video->mutex);

	return 0;
}

static int
unicam_video_set_format(struct file *file, void *fh, struct v4l2_format *format)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);
	struct unicam_video *video = video_drvdata(file);
	struct v4l2_mbus_framefmt fmt;

	if (format->type != video->type)
		return -EINVAL;

	mutex_lock(&video->mutex);
	/* convert pix to mbus and mbus to pix, to update bpp and sizeimage */
	unicam_video_pix_to_mbus(&format->fmt.pix, &fmt);
	unicam_video_mbus_to_pix(video, &fmt, &format->fmt.pix);

	handle->format = *format;

	mutex_unlock(&video->mutex);

	return 0;
}

static int
unicam_video_try_format(struct file *file, void *fh, struct v4l2_format *format)
{
	struct unicam_video *video = video_drvdata(file);
	struct v4l2_subdev_format fmt;
	struct v4l2_subdev *subdev;
	u32 pad;
	int ret;

	if (format->type != video->type)
		return -EINVAL;

	subdev = unicam_video_remote_subdev(video, &pad);
	if (subdev == NULL)
		return -EINVAL;

	unicam_video_pix_to_mbus(&format->fmt.pix, &fmt.format);

	fmt.pad = pad;
	fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	ret = v4l2_subdev_call(subdev, pad, get_fmt, NULL, &fmt);
	if (ret) {
		dev_err(video->unicam->dev, "subdev(%s) return error for get_fmt\n",
				subdev->name);
		return ret == -ENOIOCTLCMD ? -EINVAL : ret;
	}

	unicam_video_mbus_to_pix(video, &fmt.format, &format->fmt.pix);
	return 0;
}

static int
unicam_video_reqbufs(struct file *file, void *fh,
		struct v4l2_requestbuffers *rb)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);

	return vb2_reqbufs(&handle->queue, rb);
}

static int
unicam_video_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);

	return vb2_querybuf(&handle->queue, b);
}

static int
unicam_video_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);

	return vb2_qbuf(&handle->queue, b);
}

static int
unicam_video_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);

	return vb2_dqbuf(&handle->queue, b, file->f_flags & O_NONBLOCK);
}

static int
unicam_video_streamon(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);
	struct unicam_video *video = video_drvdata(file);
	struct unicam_pipeline *pipe;
	int ret;

	if (type != video->type)
		return -EINVAL;

	mutex_lock(&video->stream_lock);

	if (video->streaming) {
		dev_warn(video->unicam->dev,
				"streaming already enbaled\n");
		mutex_unlock(&video->stream_lock);
		return -EBUSY;
	}

	/* start streaming on the pipeline */
	pipe = &video->pipe;

	pipe->external = NULL;
	pipe->error = 0;

	media_entity_pipeline_start(&video->video.entity, &pipe->pipe);

	/* verify the format */
	ret = unicam_video_check_format(video, handle);
	if (ret < 0) {
		dev_err(video->unicam->dev,
				"check format failed\n");
		goto err_unicam_video_check_format;
	}

	video->bpl_value = handle->format.fmt.pix.bytesperline;
	video->height = handle->format.fmt.pix.height;

	if (video->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		pipe->output = video;
	else {
		dev_err(video->unicam->dev,
				"only capture video device supported\n");
		ret = -EPIPE;
		goto err_unicam_video_check_format;
	}

	video->queue = &handle->queue;
	INIT_LIST_HEAD(&video->dmaqueue);
	spin_lock_init(&video->qlock);
	atomic_set(&pipe->frame_number, -1);

	ret = kona_unicam_pipeline_set_stream(pipe,
			UNICAM_PIPELINE_STREAM_CONTINUOUS);
	if (ret < 0) {
		dev_err(video->unicam->dev,
				"failed to start streaming on pipeline\n");
		goto err_unicam_video_check_format;
	}

	ret = vb2_streamon(&handle->queue, type);
	if (ret < 0) {
		dev_err(video->unicam->dev,
				"failed to start streaming\n");
		goto err_vb2_streamon;
	}


	if (ret < 0) {
err_vb2_streamon:
		kona_unicam_pipeline_set_stream(pipe,
			UNICAM_PIPELINE_STREAM_STOPPED);
err_unicam_video_check_format:
		video->queue = NULL;
		media_entity_pipeline_stop(&video->video.entity);
	}

	if (!ret)
		video->streaming = 1;
	mutex_unlock(&video->stream_lock);
	return ret;
}

static int
unicam_video_streamoff(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(fh);
	struct unicam_video *video = video_drvdata(file);
	struct unicam_pipeline *pipe = &video->pipe;

	if (type != video->type)
		return -EINVAL;

	mutex_lock(&video->stream_lock);
	if (!vb2_is_streaming(&handle->queue)) {
		dev_warn(video->unicam->dev,
				"streaming already disabled\n");
		goto done;
	}

	kona_unicam_pipeline_set_stream(pipe, UNICAM_PIPELINE_STREAM_STOPPED);
	vb2_streamoff(&handle->queue, type);
	video->queue = NULL;
	video->streaming = 0;

	media_entity_pipeline_stop(&video->video.entity);
done:
	mutex_unlock(&video->stream_lock);
	return 0;
}

static int
unicam_video_enum_input(struct file *file, void *fh, struct v4l2_input *input)
{
	if (input->index > 0)
		return -EINVAL;

	strlcpy(input->name, "camera", sizeof(input->name));
	input->type = V4L2_INPUT_TYPE_CAMERA;
	return 0;
}

static int
unicam_video_g_input(struct file *file, void *fh, unsigned int *input)
{
	*input = 0;
	return 0;
}

static const struct v4l2_ioctl_ops unicam_video_ioctl_ops = {
	.vidioc_querycap		= unicam_video_querycap,
	.vidioc_g_fmt_vid_cap	= unicam_video_get_format,
	.vidioc_s_fmt_vid_cap	= unicam_video_set_format,
	.vidioc_try_fmt_vid_cap	= unicam_video_try_format,
	.vidioc_g_fmt_vid_out	= unicam_video_get_format,
	.vidioc_s_fmt_vid_out	= unicam_video_set_format,
	.vidioc_try_fmt_vid_out	= unicam_video_try_format,
	.vidioc_reqbufs			= unicam_video_reqbufs,
	.vidioc_querybuf		= unicam_video_querybuf,
	.vidioc_qbuf			= unicam_video_qbuf,
	.vidioc_dqbuf			= unicam_video_dqbuf,
	.vidioc_streamon		= unicam_video_streamon,
	.vidioc_streamoff		= unicam_video_streamoff,
	.vidioc_enum_input		= unicam_video_enum_input,
	.vidioc_g_input			= unicam_video_g_input,
};

/*------------------------------------------------
 * V4L2 file operations
 * -----------------------------------------------
 */

static int unicam_video_open(struct file *file)
{
	struct unicam_video *video = video_drvdata(file);
	struct unicam_video_fh *handle;
	struct vb2_queue *q;
	int ret = 0;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (handle == NULL) {
		dev_err(video->unicam->dev,
				"failed to allocate unicam_video_fh\n");
		return -ENOMEM;
	}

	v4l2_fh_init(&handle->vfh, &video->video);
	v4l2_fh_add(&handle->vfh);

	/* first user, intialise the pipeline. */
	if (kona_unicam_get(video->unicam) == NULL) {
		dev_err(video->unicam->dev,
				"failed to initialize pipeline\n");
		ret = -EBUSY;
		goto done;
	}


	/* turn on the power */
	ret = kona_unicam_pipeline_pm_use(&video->video.entity, 1);
	if (ret < 0) {
		dev_err(video->unicam->dev,
				"failed to enable power\n");
		kona_unicam_put(video->unicam);
		goto done;
	}


	/* dma contig context */
	video->alloc_ctx = vb2_dma_contig_init_ctx(video->unicam->dev);
	if (IS_ERR(video->alloc_ctx)) {
		dev_err(video->unicam->dev,
				"failed to allocate dma contig context\n");
		ret = PTR_ERR(video->alloc_ctx);
		kona_unicam_put(video->unicam);
		goto done;
	}

	q = &handle->queue;

	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR; /* VB2_READ?*/
	q->drv_priv = handle;
	q->ops = &unicam_video_vb2ops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->buf_struct_size = sizeof(struct unicam_buffer);


	ret = vb2_queue_init(q);
	if (ret) {
		dev_err(video->unicam->dev,
				"failed to initialize queue\n");
		kona_unicam_put(video->unicam);
		goto done;
	}

	memset(&handle->format, 0, sizeof(handle->format));
	handle->format.type = video->type;
	handle->video = video;
	file->private_data = &handle->vfh;

done:
	if (ret < 0) {
		v4l2_fh_del(&handle->vfh);
		kfree(handle);
	}

	return ret;
}

static int unicam_video_release(struct file *file)
{
	struct unicam_video *video = video_drvdata(file);
	struct v4l2_fh *vfh = file->private_data;
	struct unicam_video_fh *handle = to_unicam_video_fh(vfh);

	unicam_video_streamoff(file, vfh, video->type);

	kona_unicam_pipeline_pm_use(&video->video.entity, 0);

	/* release the vb2 queue */
	vb2_queue_release(&handle->queue);

	/* release the file handle */
	v4l2_fh_del(vfh);
	kfree(handle);
	file->private_data = NULL;

	kona_unicam_put(video->unicam);

	return 0;
}

static unsigned int unicam_video_poll(struct file *file, poll_table *wait)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(file->private_data);

	return vb2_poll(&handle->queue, file, wait);
}

static int unicam_video_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct unicam_video_fh *handle = to_unicam_video_fh(file->private_data);
	return vb2_mmap(&handle->queue, vma);
}


static struct v4l2_file_operations unicam_video_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = video_ioctl2,
	.open = unicam_video_open,
	.release = unicam_video_release,
	.poll = unicam_video_poll,
	.mmap = unicam_video_mmap,
};

/* ------------------------------------------------------
 *  Operations called by unicam core for intialization
 * ------------------------------------------------------
 */

int kona_unicam_video_register(struct unicam_video *video,
		struct v4l2_device *vdev)
{
	int ret;

	video->video.v4l2_dev = vdev;

	ret = video_register_device(&video->video, VFL_TYPE_GRABBER, -1);
	if (ret < 0)
		dev_err(video->unicam->dev,
				"failed to register video device\n");

	return ret;
}

void kona_unicam_video_unregister(struct unicam_video *video)
{
	if (video_is_registered(&video->video))
		video_unregister_device(&video->video);
}

static const struct unicam_video_operations unicam_video_dummy_ops = {
};

int kona_unicam_video_init(struct unicam_video *video, const char *name)
{
	const char *direction;
	int ret;

	switch (video->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		direction = "output";
		video->pad.flags = MEDIA_PAD_FL_SINK;
		break;
	default:
		return -EINVAL;

	}
	ret = media_entity_init(&video->video.entity, 1, &video->pad, 0);
	if (ret < 0)
		return ret;

	mutex_init(&video->mutex);
	atomic_set(&video->active, 0);

	spin_lock_init(&video->pipe.lock);
	mutex_init(&video->stream_lock);

	/* initialize the video device */
	if (video->ops == NULL)
		video->ops = &unicam_video_dummy_ops;

	video->video.fops = &unicam_video_fops;
	snprintf(video->video.name, sizeof(video->video.name),
			"KONA Unicam %s %s", name, direction);
	video->video.vfl_type = VFL_TYPE_GRABBER;
	video->video.release = video_device_release_empty;
	video->video.ioctl_ops = &unicam_video_ioctl_ops;
	video->pipe.stream_state = UNICAM_PIPELINE_STREAM_STOPPED;

	video_set_drvdata(&video->video, video);

	return 0;
}

void kona_unicam_video_cleanup(struct unicam_video *video)
{
	media_entity_cleanup(&video->video.entity);
	mutex_destroy(&video->stream_lock);
	mutex_destroy(&video->mutex);
}
