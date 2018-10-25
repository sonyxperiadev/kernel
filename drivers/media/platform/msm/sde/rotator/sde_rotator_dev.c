/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#define pr_fmt(fmt)	"%s:%d: " fmt, __func__, __LINE__

#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/of.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-mem2mem.h>

#include "sde_rotator_inline.h"
#include "sde_rotator_base.h"
#include "sde_rotator_core.h"
#include "sde_rotator_dev.h"
#include "sde_rotator_debug.h"
#include "sde_rotator_trace.h"

/* Start v4l2 device number (default allocation) */
#define SDE_ROTATOR_BASE_DEVICE_NUMBER	-1

/* Default value for early_submit flag */
#define SDE_ROTATOR_EARLY_SUBMIT	1

/* Timeout (msec) waiting for stream to turn off. */
#define SDE_ROTATOR_STREAM_OFF_TIMEOUT	500

/* acquire fence time out, following other driver fence time out practice */
#define SDE_ROTATOR_FENCE_TIMEOUT	MSEC_PER_SEC

/* Timeout (msec) waiting for ctx open */
#define SDE_ROTATOR_CTX_OPEN_TIMEOUT	500

/* Rotator default fps */
#define SDE_ROTATOR_DEFAULT_FPS	60

/* Rotator rotation angles */
#define SDE_ROTATOR_DEGREE_270		270
#define SDE_ROTATOR_DEGREE_180		180
#define SDE_ROTATOR_DEGREE_90		90

/* Inline rotator qos request */
#define SDE_ROTATOR_ADD_REQUEST		1
#define SDE_ROTATOR_REMOVE_REQUEST		0


static void sde_rotator_submit_handler(struct kthread_work *work);
static void sde_rotator_retire_handler(struct kthread_work *work);
static void sde_rotator_pm_qos_request(struct sde_rotator_device *rot_dev,
					 bool add_request);
#ifdef CONFIG_COMPAT
static long sde_rotator_compat_ioctl32(struct file *file,
	unsigned int cmd, unsigned long arg);
#endif

/*
 * sde_rotator_ctx_from_fh - Get rotator context from v4l2 fh.
 * @fh: Pointer to v4l2 fh.
 */
static inline struct sde_rotator_ctx *sde_rotator_ctx_from_fh(
		struct v4l2_fh *fh)
{
	return container_of(fh, struct sde_rotator_ctx, fh);
}

/*
 * sde_rotator_get_flags_from_ctx - Get low-level command flag
 * @ctx: Pointer to rotator context.
 */
static uint32_t sde_rotator_get_flags_from_ctx(struct sde_rotator_ctx *ctx)
{
	uint32_t ret_flags = 0;

	if (ctx->rotate == SDE_ROTATOR_DEGREE_270)
		ret_flags |= SDE_ROTATION_270;
	else if (ctx->rotate == SDE_ROTATOR_DEGREE_180)
		ret_flags |= SDE_ROTATION_180;
	else if (ctx->rotate == SDE_ROTATOR_DEGREE_90)
		ret_flags |= SDE_ROTATION_90;
	if (ctx->hflip)
		ret_flags ^= SDE_ROTATION_FLIP_LR;
	if (ctx->vflip)
		ret_flags ^= SDE_ROTATION_FLIP_UD;
	if (ctx->secure)
		ret_flags |= SDE_ROTATION_SECURE;
	if (ctx->secure_camera)
		ret_flags |= SDE_ROTATION_SECURE_CAMERA;
	if (ctx->format_out.fmt.pix.field == V4L2_FIELD_INTERLACED &&
			ctx->format_cap.fmt.pix.field == V4L2_FIELD_NONE)
		ret_flags |= SDE_ROTATION_DEINTERLACE;

	return ret_flags;
}

/*
 * sde_rotator_get_config_from_ctx - Fill rotator configure structure.
 * @ctx: Pointer to rotator ctx.
 * @config: Pointer to config structure.
 */
static void sde_rotator_get_config_from_ctx(struct sde_rotator_ctx *ctx,
		struct sde_rotation_config *config)
{
	memset(config, 0, sizeof(struct sde_rotation_config));
	config->flags = sde_rotator_get_flags_from_ctx(ctx);
	config->frame_rate = (ctx->timeperframe.numerator) ?
				ctx->timeperframe.denominator
					/ ctx->timeperframe.numerator :	0;
	config->session_id = ctx->session_id;
	config->input.width = ctx->crop_out.width;
	config->input.height = ctx->crop_out.height;
	config->input.format = ctx->format_out.fmt.pix.pixelformat;
	config->input.comp_ratio.numer = 1;
	config->input.comp_ratio.denom = 1;
	config->output.width = ctx->crop_cap.width;
	config->output.height = ctx->crop_cap.height;
	config->output.format = ctx->format_cap.fmt.pix.pixelformat;
	config->output.comp_ratio.numer = 1;
	config->output.comp_ratio.denom = 1;

	/*
	 * Use compression ratio of the first buffer to estimate
	 * performance requirement of the session. If core layer does
	 * not support dynamic per buffer compression ratio recalculation,
	 * this configuration will determine the overall static compression
	 * ratio of the session.
	 */
	if (ctx->vbinfo_out)
		config->input.comp_ratio = ctx->vbinfo_out[0].comp_ratio;
	if (ctx->vbinfo_cap)
		config->output.comp_ratio = ctx->vbinfo_cap[0].comp_ratio;

	SDEDEV_DBG(ctx->rot_dev->dev, "config s:%d out_cr:%u/%u cap_cr:%u/%u\n",
			ctx->session_id,
			config->input.comp_ratio.numer,
			config->input.comp_ratio.denom,
			config->output.comp_ratio.numer,
			config->output.comp_ratio.denom);
}

/*
 * sde_rotator_get_item_from_ctx - Fill rotator item structure.
 * @ctx: Pointer to rotator ctx.
 * @item: Pointer to item structure.
 */
static void sde_rotator_get_item_from_ctx(struct sde_rotator_ctx *ctx,
		struct sde_rotation_item *item)
{
	memset(item, 0, sizeof(struct sde_rotation_item));
	item->flags = sde_rotator_get_flags_from_ctx(ctx);
	item->session_id = ctx->session_id;
	item->sequence_id = 0;
	/* assign high/low priority */
	item->wb_idx = (ctx->fh.prio >= V4L2_PRIORITY_DEFAULT) ? 0 : 1;
	item->src_rect.x = ctx->crop_out.left;
	item->src_rect.y = ctx->crop_out.top;
	item->src_rect.w = ctx->crop_out.width;
	item->src_rect.h = ctx->crop_out.height;
	item->input.width = ctx->format_out.fmt.pix.width;
	item->input.height = ctx->format_out.fmt.pix.height;
	item->input.format = ctx->format_out.fmt.pix.pixelformat;
	item->input.planes[0].fd = -1;
	item->input.planes[0].offset = 0;
	item->input.planes[0].stride = ctx->format_out.fmt.pix.bytesperline;
	item->input.plane_count = 1;
	item->input.fence = NULL;
	item->input.comp_ratio.numer = 1;
	item->input.comp_ratio.denom = 1;

	item->dst_rect.x = ctx->crop_cap.left;
	item->dst_rect.y = ctx->crop_cap.top;
	item->dst_rect.w = ctx->crop_cap.width;
	item->dst_rect.h = ctx->crop_cap.height;
	item->output.width = ctx->format_cap.fmt.pix.width;
	item->output.height = ctx->format_cap.fmt.pix.height;
	item->output.format = ctx->format_cap.fmt.pix.pixelformat;
	item->output.planes[0].fd = -1;
	item->output.planes[0].offset = 0;
	item->output.planes[0].stride = ctx->format_cap.fmt.pix.bytesperline;
	item->output.plane_count = 1;
	item->output.fence = NULL;
	item->output.comp_ratio.numer = 1;
	item->output.comp_ratio.denom = 1;
}

/*
 * sde_rotator_format_recalc - Recalculate format parameters.
 * @f: v4l2 format.
 */
static void sde_rotator_format_recalc(struct v4l2_format *f)
{
	int ret;
	struct sde_mdp_format_params *fmt;
	struct sde_mdp_plane_sizes ps;

	fmt = sde_get_format_params(f->fmt.pix.pixelformat);
	if (!fmt) {
		SDEROT_ERR("invalid format\n");
		goto error_fmt;
	}

	ret = sde_mdp_get_plane_sizes(fmt,
		f->fmt.pix.width, f->fmt.pix.height, &ps, 0, 0);
	if (ret) {
		SDEROT_ERR("invalid plane size\n");
		goto error_fmt;
	}

	f->fmt.pix.bytesperline = ps.ystride[0];
	f->fmt.pix.sizeimage = ps.total_size;

	return;
error_fmt:
	f->fmt.pix.bytesperline = 0;
	f->fmt.pix.sizeimage = 0;
}

/*
 * sde_rotator_validate_item - Check if rotator item is valid for processing.
 * @ctx: Pointer to rotator ctx.
 * @item: Pointer to item structure
 */
static int sde_rotator_validate_item(struct sde_rotator_ctx *ctx,
		struct sde_rotation_item *item)
{
	int ret;
	struct sde_rot_entry_container *req;
	struct sde_rotator_device *rot_dev = ctx->rot_dev;

	sde_rot_mgr_lock(rot_dev->mgr);
	req = sde_rotator_req_init(rot_dev->mgr, ctx->private, item, 1, 0);
	if (IS_ERR_OR_NULL(req)) {
		SDEDEV_ERR(rot_dev->dev, "fail allocate item\n");
		return -ENOMEM;
	}

	ret = sde_rotator_validate_request(rot_dev->mgr, ctx->private, req);
	sde_rot_mgr_unlock(rot_dev->mgr);
	devm_kfree(rot_dev->dev, req);
	return ret;
}

/*
 * sde_rotator_queue_setup - vb2_ops queue_setup callback.
 * @q: Pointer to vb2 queue struct.
 * @num_buffers: Pointer of number of buffers requested.
 * @num_planes: Pointer to number of planes requested.
 * @sizes: Array containing sizes of planes.
 * @alloc_ctxs: Array of allocated contexts for each plane.
 */
static int sde_rotator_queue_setup(struct vb2_queue *q,
	unsigned int *num_buffers, unsigned int *num_planes,
	unsigned int sizes[], struct device *alloc_devs[])
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(q);
	int i;

	if (!num_buffers)
		return -EINVAL;

	switch (q->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		sizes[0] = ctx->format_out.fmt.pix.sizeimage;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		sizes[0] = ctx->format_cap.fmt.pix.sizeimage;
		break;
	default:
		return -EINVAL;
	}

	*num_planes = 1;
	alloc_devs[0] = (struct device *)ctx;

	switch (q->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		ctx->nbuf_out = *num_buffers;
		kfree(ctx->vbinfo_out);
		ctx->vbinfo_out = kzalloc(sizeof(struct sde_rotator_vbinfo) *
					ctx->nbuf_out, GFP_KERNEL);
		if (!ctx->vbinfo_out)
			return -ENOMEM;
		for (i = 0; i < ctx->nbuf_out; i++) {
			ctx->vbinfo_out[i].fd = -1;
			ctx->vbinfo_out[i].comp_ratio.numer = 1;
			ctx->vbinfo_out[i].comp_ratio.denom = 1;
		}
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		ctx->nbuf_cap = *num_buffers;
		kfree(ctx->vbinfo_cap);
		ctx->vbinfo_cap = kzalloc(sizeof(struct sde_rotator_vbinfo) *
					ctx->nbuf_cap, GFP_KERNEL);
		if (!ctx->vbinfo_cap)
			return -ENOMEM;
		for (i = 0; i < ctx->nbuf_cap; i++) {
			ctx->vbinfo_cap[i].fd = -1;
			ctx->vbinfo_cap[i].comp_ratio.numer = 1;
			ctx->vbinfo_cap[i].comp_ratio.denom = 1;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * sde_rotator_buf_queue - vb2_ops buf_queue callback.
 * @vb: Pointer to vb2 buffer struct.
 */
static void sde_rotator_buf_queue(struct vb2_buffer *vb)
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);

	v4l2_m2m_buf_queue(ctx->fh.m2m_ctx, vbuf);
}

/*
 * sde_rotator_buf_finish - vb2_ops buf_finish to finalize buffer before going
 *				back to user space
 * @vb: Pointer to vb2 buffer struct.
 */
static void sde_rotator_buf_finish(struct vb2_buffer *vb)
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
	int i;

	SDEDEV_DBG(ctx->rot_dev->dev,
			"buf_finish t:%d i:%d s:%d m:%u np:%d up:%lu\n",
			vb->type, vb->index, vb->state,
			vb->vb2_queue->memory,
			vb->num_planes,
			vb->planes[0].m.userptr);

	if (vb->vb2_queue->memory != VB2_MEMORY_USERPTR)
		return;

	/*
	 * We use userptr to tunnel fd, and fd can be the same across qbuf
	 * even though the underlying buffer is different.  Since vb2 layer
	 * optimizes memory mapping for userptr by first checking if userptr
	 * has changed, it will not trigger put_userptr if fd value does
	 * not change.  In order to force buffer release, we need to clear
	 * userptr when the current buffer is done and ready to go back to
	 * user mode. Since 0 is a valid fd, reset userptr to -1 instead.
	 */
	for (i = 0; i < vb->num_planes; i++)
		vb->planes[i].m.userptr = ~0;
}

/*
 * sde_rotator_return_all_buffers - Return all buffers with the given status.
 * @q: Pointer to vb2 buffer queue struct.
 * @state: State of the buffer
 */
static void sde_rotator_return_all_buffers(struct vb2_queue *q,
		enum vb2_buffer_state state)
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(q);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;

	SDEDEV_DBG(rot_dev->dev,
			"return q t:%d c:%d dc:%d s:%d\n",
			q->type, q->queued_count,
			atomic_read(&q->owned_by_drv_count),
			state);

	/* return buffers according videobuffer2-core.h */
	if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		struct vb2_v4l2_buffer *buf;

		while ((buf = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx))) {
			SDEDEV_DBG(rot_dev->dev,
					"return vb t:%d i:%d\n",
					buf->vb2_buf.type,
					buf->vb2_buf.index);
			v4l2_m2m_buf_done(buf, state);
		}
	} else if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		struct vb2_v4l2_buffer *buf;

		while ((buf = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx))) {
			SDEDEV_DBG(rot_dev->dev,
					"return vb t:%d i:%d\n",
					buf->vb2_buf.type,
					buf->vb2_buf.index);
			v4l2_m2m_buf_done(buf, state);
		}
	} else {
		SDEDEV_ERR(rot_dev->dev, "unsupported vb t:%d\n", q->type);
	}
}

/*
 * sde_rotator_start_streaming - vb2_ops start_streaming callback.
 * @q: Pointer to vb2 queue struct.
 * @count: Number of buffer queued before stream on call.
 */
static int sde_rotator_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(q);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;

	SDEDEV_DBG(rot_dev->dev, "start streaming s:%d t:%d\n",
			ctx->session_id, q->type);

	if (!list_empty(&ctx->pending_list)) {
		SDEDEV_ERR(rot_dev->dev,
				"command pending error s:%d t:%d p:%d\n",
				ctx->session_id, q->type,
				!list_empty(&ctx->pending_list));
		return -EINVAL;
	}

	ctx->abort_pending = 0;

	return 0;
}

/*
 * sde_rotator_stop_streaming - vb2_ops stop_streaming callback.
 * @q: Pointer to vb2 queue struct.
 *
 * This function will block waiting for stream to stop.  Unlock queue
 * lock to avoid deadlock.
 */
static void sde_rotator_stop_streaming(struct vb2_queue *q)
{
	struct sde_rotator_ctx *ctx = vb2_get_drv_priv(q);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotator_request *request;
	struct list_head *curr, *next;
	int i;
	int ret;

	SDEDEV_DBG(rot_dev->dev, "stop streaming s:%d t:%d p:%d\n",
			ctx->session_id, q->type,
			!list_empty(&ctx->pending_list));
	ctx->abort_pending = 1;
	mutex_unlock(q->lock);
	ret = wait_event_timeout(ctx->wait_queue,
			list_empty(&ctx->pending_list),
			msecs_to_jiffies(rot_dev->streamoff_timeout));
	mutex_lock(q->lock);
	if (!ret) {
		SDEDEV_ERR(rot_dev->dev,
				"timeout to stream off s:%d t:%d p:%d\n",
				ctx->session_id, q->type,
				!list_empty(&ctx->pending_list));
		SDEROT_EVTLOG(ctx->session_id, q->type,
				!list_empty(&ctx->pending_list),
				SDE_ROT_EVTLOG_ERROR);
		sde_rot_mgr_lock(rot_dev->mgr);
		sde_rotator_cancel_all_requests(rot_dev->mgr, ctx->private);
		sde_rot_mgr_unlock(rot_dev->mgr);
		list_for_each_safe(curr, next, &ctx->pending_list) {
			request = container_of(curr, struct sde_rotator_request,
						list);

			SDEDEV_DBG(rot_dev->dev, "cancel request s:%d\n",
					ctx->session_id);
			mutex_unlock(q->lock);
			kthread_cancel_work_sync(&request->submit_work);
			kthread_cancel_work_sync(&request->retire_work);
			mutex_lock(q->lock);
			spin_lock(&ctx->list_lock);
			list_del_init(&request->list);
			list_add_tail(&request->list, &ctx->retired_list);
			spin_unlock(&ctx->list_lock);
		}
	}

	sde_rotator_return_all_buffers(q, VB2_BUF_STATE_ERROR);

	/* clear fence for buffer */
	sde_rotator_resync_timeline(ctx->work_queue.timeline);
	if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		for (i = 0; i < ctx->nbuf_cap; i++) {
			struct sde_rotator_vbinfo *vbinfo =
					&ctx->vbinfo_cap[i];

			if (vbinfo->fence) {
				/* fence is not used */
				SDEDEV_DBG(rot_dev->dev,
						"put fence s:%d t:%d i:%d\n",
						ctx->session_id, q->type, i);
				sde_rotator_put_sync_fence(vbinfo->fence);
			}
			vbinfo->fence = NULL;
			vbinfo->fd = -1;
		}
	} else if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		for (i = 0; i < ctx->nbuf_out; i++) {
			struct sde_rotator_vbinfo *vbinfo =
					&ctx->vbinfo_out[i];

			if (vbinfo->fence) {
				SDEDEV_DBG(rot_dev->dev,
						"put fence s:%d t:%d i:%d\n",
						ctx->session_id, q->type, i);
				sde_rotator_put_sync_fence(vbinfo->fence);
			}
			vbinfo->fence = NULL;
			vbinfo->fd = -1;
		}
	}
}

/* Videobuf2 queue callbacks. */
static const struct vb2_ops sde_rotator_vb2_q_ops = {
	.queue_setup     = sde_rotator_queue_setup,
	.buf_queue       = sde_rotator_buf_queue,
	.start_streaming = sde_rotator_start_streaming,
	.stop_streaming  = sde_rotator_stop_streaming,
	.wait_prepare	 = vb2_ops_wait_prepare,
	.wait_finish	 = vb2_ops_wait_finish,
	.buf_finish      = sde_rotator_buf_finish,
};

/*
 * sde_rotator_get_userptr - Map and get buffer handler for user pointer buffer.
 * @dev: device allocated in buf_setup.
 * @vaddr: Virtual addr passed from userpsace (in our case ion fd)
 * @size: Size of the buffer
 * @dma_dir: DMA data direction of the given buffer.
 */
static void *sde_rotator_get_userptr(struct device *dev,
	unsigned long vaddr, unsigned long size,
	enum dma_data_direction dma_dir)
{
	struct sde_rotator_ctx *ctx = (struct sde_rotator_ctx *)dev;
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotator_buf_handle *buf;
	struct ion_client *iclient = rot_dev->mdata->iclient;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	buf->fd = vaddr;
	buf->secure = ctx->secure || ctx->secure_camera;
	buf->ctx = ctx;
	buf->rot_dev = rot_dev;
	if (ctx->secure_camera) {
		buf->handle = ion_import_dma_buf_fd(iclient,
				buf->fd);
		if (IS_ERR_OR_NULL(buf->handle)) {
			SDEDEV_ERR(rot_dev->dev,
				"fail get ion_handler fd:%d r:%ld\n",
				buf->fd, PTR_ERR(buf->buffer));
			goto error_buf_get;
		}
		SDEDEV_DBG(rot_dev->dev,
				"get ion_handle s:%d fd:%d buf:%pad\n",
				buf->ctx->session_id,
				buf->fd, &buf->handle);
	} else {
		buf->buffer = dma_buf_get(buf->fd);
		if (IS_ERR_OR_NULL(buf->buffer)) {
			SDEDEV_ERR(rot_dev->dev,
				"fail get dmabuf fd:%d r:%ld\n",
				buf->fd, PTR_ERR(buf->buffer));
			goto error_buf_get;
		}
		SDEDEV_DBG(rot_dev->dev,
				"get dmabuf s:%d fd:%d buf:%pad\n",
				buf->ctx->session_id,
				buf->fd, &buf->buffer);
	}

	return buf;
error_buf_get:
	kfree(buf);
	return ERR_PTR(-ENOMEM);
}

/*
 * sde_rotator_put_userptr - Unmap and free buffer handler.
 * @buf_priv: Buffer handler allocated get_userptr callback.
 */
static void sde_rotator_put_userptr(void *buf_priv)
{
	struct sde_rotator_buf_handle *buf = buf_priv;

	if (IS_ERR_OR_NULL(buf))
		return;

	if (!buf->rot_dev || !buf->ctx) {
		WARN_ON(!buf->rot_dev || !buf->ctx);
		SDEROT_ERR("null rotator device/context\n");
		return;
	}

	SDEDEV_DBG(buf->rot_dev->dev, "put dmabuf s:%d fd:%d buf:%pad\n",
			buf->ctx->session_id,
			buf->fd, &buf->buffer);

	if (buf->buffer) {
		dma_buf_put(buf->buffer);
		buf->buffer = NULL;
	}

	kfree(buf_priv);
}

/* Videobuf2 memory callbacks. */
static struct vb2_mem_ops sde_rotator_vb2_mem_ops = {
	.get_userptr = sde_rotator_get_userptr,
	.put_userptr = sde_rotator_put_userptr,
};

/*
 * sde_rotator_s_ctx_ctrl - set context control variable to v4l2 control
 * @ctx: Pointer to rotator context.
 * @ctx_ctrl: Pointer to context control variable
 * @ctrl: Pointer to v4l2 control variable
 */
static int sde_rotator_s_ctx_ctrl(struct sde_rotator_ctx *ctx,
		s32 *ctx_ctrl, struct v4l2_ctrl *ctrl)
{
	*ctx_ctrl = ctrl->val;
	return 0;
}

/*
 * sde_rotator_s_ctrl - Set control.
 * @ctrl: Pointer to v4l2 control structure.
 */
static int sde_rotator_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sde_rotator_ctx *ctx =
		container_of(ctrl->handler,
				struct sde_rotator_ctx, ctrl_handler);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	int ret;

	SDEDEV_DBG(rot_dev->dev, "set %s:%d s:%d\n", ctrl->name, ctrl->val,
			ctx->session_id);

	sde_rot_mgr_lock(rot_dev->mgr);

	switch (ctrl->id) {
	case V4L2_CID_HFLIP:
		ret = sde_rotator_s_ctx_ctrl(ctx, &ctx->hflip, ctrl);
		break;

	case V4L2_CID_VFLIP:
		ret = sde_rotator_s_ctx_ctrl(ctx, &ctx->vflip, ctrl);
		break;

	case V4L2_CID_ROTATE:
		ret = sde_rotator_s_ctx_ctrl(ctx, &ctx->rotate, ctrl);
		break;

	case V4L2_CID_SDE_ROTATOR_SECURE:
		ret = sde_rotator_s_ctx_ctrl(ctx, &ctx->secure, ctrl);
		break;

	case V4L2_CID_SDE_ROTATOR_SECURE_CAMERA:
		ret = sde_rotator_s_ctx_ctrl(ctx, &ctx->secure_camera, ctrl);
		break;
	default:
		v4l2_warn(&rot_dev->v4l2_dev, "invalid control %d\n", ctrl->id);
		ret = -EINVAL;
	}

	sde_rot_mgr_unlock(rot_dev->mgr);
	return ret;
}

/*
 * sde_rotator_ctrl_ops - Control operations.
 */
static const struct v4l2_ctrl_ops sde_rotator_ctrl_ops = {
	.s_ctrl = sde_rotator_s_ctrl,
};

/*
 * sde_rotator_ctrl_secure - Non-secure/Secure.
 */
static const struct v4l2_ctrl_config sde_rotator_ctrl_secure = {
	.ops = &sde_rotator_ctrl_ops,
	.id = V4L2_CID_SDE_ROTATOR_SECURE,
	.name = "Non-secure/Secure Domain",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.def = 0,
	.min = 0,
	.max = 1,
	.step = 1,
};

static const struct v4l2_ctrl_config sde_rotator_ctrl_secure_camera = {
	.ops = &sde_rotator_ctrl_ops,
	.id = V4L2_CID_SDE_ROTATOR_SECURE_CAMERA,
	.name = "Secure Camera content",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.def = 0,
	.min = 0,
	.max = 1,
	.step = 1,
};

/*
 * sde_rotator_ctx_show - show context state.
 */
static ssize_t sde_rotator_ctx_show(struct kobject *kobj,
	struct kobj_attribute *attr, char *buf)
{
	size_t len = PAGE_SIZE;
	int cnt = 0;
	struct sde_rotator_ctx *ctx =
		container_of(kobj, struct sde_rotator_ctx, kobj);

	if (!ctx)
		return cnt;

#define SPRINT(fmt, ...) \
		(cnt += scnprintf(buf + cnt, len - cnt, fmt, ##__VA_ARGS__))

	SPRINT("rotate=%d\n", ctx->rotate);
	SPRINT("hflip=%d\n", ctx->hflip);
	SPRINT("vflip=%d\n", ctx->vflip);
	SPRINT("priority=%d\n", ctx->fh.prio);
	SPRINT("secure=%d\n", ctx->secure);
	SPRINT("timeperframe=%u %u\n", ctx->timeperframe.numerator,
			ctx->timeperframe.denominator);
	SPRINT("nbuf_out=%d\n", ctx->nbuf_out);
	SPRINT("nbuf_cap=%d\n", ctx->nbuf_cap);
	SPRINT("crop_out=%u %u %u %u\n",
			ctx->crop_out.left, ctx->crop_out.top,
			ctx->crop_out.width, ctx->crop_out.height);
	SPRINT("crop_cap=%u %u %u %u\n",
			ctx->crop_cap.left, ctx->crop_cap.top,
			ctx->crop_cap.width, ctx->crop_cap.height);
	SPRINT("fmt_out=%c%c%c%c %u %u %u %u\n",
			(ctx->format_out.fmt.pix.pixelformat>>0)&0xff,
			(ctx->format_out.fmt.pix.pixelformat>>8)&0xff,
			(ctx->format_out.fmt.pix.pixelformat>>16)&0xff,
			(ctx->format_out.fmt.pix.pixelformat>>24)&0xff,
			ctx->format_out.fmt.pix.width,
			ctx->format_out.fmt.pix.height,
			ctx->format_out.fmt.pix.bytesperline,
			ctx->format_out.fmt.pix.sizeimage);
	SPRINT("fmt_cap=%c%c%c%c %u %u %u %u\n",
			(ctx->format_cap.fmt.pix.pixelformat>>0)&0xff,
			(ctx->format_cap.fmt.pix.pixelformat>>8)&0xff,
			(ctx->format_cap.fmt.pix.pixelformat>>16)&0xff,
			(ctx->format_cap.fmt.pix.pixelformat>>24)&0xff,
			ctx->format_cap.fmt.pix.width,
			ctx->format_cap.fmt.pix.height,
			ctx->format_cap.fmt.pix.bytesperline,
			ctx->format_cap.fmt.pix.sizeimage);
	SPRINT("abort_pending=%d\n", ctx->abort_pending);
	SPRINT("command_pending=%d\n", !list_empty(&ctx->pending_list));
	SPRINT("sequence=%u\n",
		sde_rotator_get_timeline_commit_ts(ctx->work_queue.timeline));
	SPRINT("timestamp=%u\n",
		sde_rotator_get_timeline_retire_ts(ctx->work_queue.timeline));
	return cnt;
}

static struct kobj_attribute sde_rotator_ctx_attr =
	__ATTR(state, 0664, sde_rotator_ctx_show, NULL);

static struct attribute *sde_rotator_fs_attrs[] = {
	&sde_rotator_ctx_attr.attr,
	NULL
};

static struct attribute_group sde_rotator_fs_attr_group = {
	.attrs = sde_rotator_fs_attrs
};

/*
 * sde_rotator_ctx_show - sysfs show callback.
 */
static ssize_t sde_rotator_fs_show(struct kobject *kobj,
	struct attribute *attr, char *buf)
{
	ssize_t ret = -EIO;
	struct kobj_attribute *kattr =
			container_of(attr, struct kobj_attribute, attr);
	if (kattr->show)
		ret = kattr->show(kobj, kattr, buf);
	return ret;
}

/*
 * sde_rotator_fs_store - sysfs store callback.
 */
static ssize_t sde_rotator_fs_store(struct kobject *kobj,
	struct attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -EIO;
	struct kobj_attribute *kattr =
			container_of(attr, struct kobj_attribute, attr);
	if (kattr->store)
		ret = kattr->store(kobj, kattr, buf, count);
	return ret;
}

static const struct sysfs_ops sde_rotator_fs_ops = {
	.show = sde_rotator_fs_show,
	.store = sde_rotator_fs_store,
};

static struct kobj_type sde_rotator_fs_ktype = {
	.sysfs_ops = &sde_rotator_fs_ops,
};

/*
 * sde_rotator_queue_init - m2m_ops queue_setup callback.
 * @priv: Pointer to rotator ctx.
 * @src_vq: vb2 source queue.
 * @dst_vq: vb2 destination queue.
 */
static int sde_rotator_queue_init(void *priv, struct vb2_queue *src_vq,
	struct vb2_queue *dst_vq)
{
	struct sde_rotator_ctx *ctx = priv;
	int ret;

	src_vq->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	src_vq->io_modes = VB2_USERPTR;
	src_vq->drv_priv = ctx;
	src_vq->mem_ops = &sde_rotator_vb2_mem_ops;
	src_vq->ops = &sde_rotator_vb2_q_ops;
	src_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	src_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	src_vq->lock = &ctx->rot_dev->lock;
	src_vq->min_buffers_needed = 1;
	src_vq->dev = ctx->rot_dev->dev;

	ret = vb2_queue_init(src_vq);
	if (ret) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail init src queue r:%d\n", ret);
		return ret;
	}

	dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst_vq->io_modes = VB2_USERPTR;
	dst_vq->drv_priv = ctx;
	dst_vq->mem_ops = &sde_rotator_vb2_mem_ops;
	dst_vq->ops = &sde_rotator_vb2_q_ops;
	dst_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	dst_vq->lock = &ctx->rot_dev->lock;
	dst_vq->min_buffers_needed = 1;
	src_vq->dev = ctx->rot_dev->dev;

	ret = vb2_queue_init(dst_vq);
	if (ret) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail init dst queue r:%d\n", ret);
		return ret;
	}

	return 0;
}

/*
 * sde_rotator_ctx_open - Rotator device open method.
 * @rot_dev: Pointer to rotator device structure
 * @file: Pointer to file struct (optional)
 * return: Pointer rotator context if success; ptr error code, otherwise.
 */
struct sde_rotator_ctx *sde_rotator_ctx_open(
		struct sde_rotator_device *rot_dev, struct file *file)
{
	struct video_device *video = file ? video_devdata(file) : NULL;
	struct sde_rotator_ctx *ctx;
	struct v4l2_ctrl_handler *ctrl_handler;
	char name[32];
	int i, ret;

	if (atomic_read(&rot_dev->mgr->device_suspended))
		return ERR_PTR(-EPERM);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return ERR_PTR(-ENOMEM);

	if (mutex_lock_interruptible(&rot_dev->lock)) {
		ret = -ERESTARTSYS;
		goto error_lock;
	}

	/* wait until exclusive ctx, if exists, finishes or timeout */
	while (rot_dev->excl_ctx) {
		SDEROT_DBG("waiting to open %s session %d ...\n",
				file ? "v4l2" : "excl",	rot_dev->session_id);
		mutex_unlock(&rot_dev->lock);
		ret = wait_event_interruptible_timeout(rot_dev->open_wq,
				!rot_dev->excl_ctx,
				msecs_to_jiffies(rot_dev->open_timeout));
		if (ret < 0) {
			goto error_lock;
		} else if (!ret) {
			SDEROT_WARN("timeout to open session %d\n",
					rot_dev->session_id);
			SDEROT_EVTLOG(rot_dev->session_id,
					SDE_ROT_EVTLOG_ERROR);
			ret = -EBUSY;
			goto error_lock;
		} else if (mutex_lock_interruptible(&rot_dev->lock)) {
			ret = -ERESTARTSYS;
			goto error_lock;
		}
	}

	ctx->rot_dev = rot_dev;
	ctx->file = file;

	/* Set context defaults */
	ctx->session_id = rot_dev->session_id++;
	SDEDEV_DBG(ctx->rot_dev->dev, "open %d\n", ctx->session_id);
	ctx->timeperframe.numerator = 1;
	ctx->timeperframe.denominator = SDE_ROTATOR_DEFAULT_FPS;
	ctx->hflip = 0;
	ctx->vflip = 0;
	ctx->rotate = 0;
	ctx->secure = 0;
	ctx->abort_pending = 0;
	ctx->kthread_id = -1;
	ctx->format_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ctx->format_cap.fmt.pix.pixelformat = SDE_PIX_FMT_Y_CBCR_H2V2;
	ctx->format_cap.fmt.pix.width = 640;
	ctx->format_cap.fmt.pix.height = 480;
	ctx->crop_cap.width = 640;
	ctx->crop_cap.height = 480;
	ctx->format_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ctx->format_out.fmt.pix.pixelformat = SDE_PIX_FMT_Y_CBCR_H2V2;
	ctx->format_out.fmt.pix.width = 640;
	ctx->format_out.fmt.pix.height = 480;
	ctx->crop_out.width = 640;
	ctx->crop_out.height = 480;
	init_waitqueue_head(&ctx->wait_queue);
	spin_lock_init(&ctx->list_lock);
	INIT_LIST_HEAD(&ctx->pending_list);
	INIT_LIST_HEAD(&ctx->retired_list);

	for (i = 0 ; i < ARRAY_SIZE(ctx->requests); i++) {
		struct sde_rotator_request *request = &ctx->requests[i];

		kthread_init_work(&request->submit_work,
				sde_rotator_submit_handler);
		kthread_init_work(&request->retire_work,
				sde_rotator_retire_handler);
		request->ctx = ctx;
		INIT_LIST_HEAD(&request->list);
		list_add_tail(&request->list, &ctx->retired_list);
	}

	if (ctx->file) {
		v4l2_fh_init(&ctx->fh, video);
		file->private_data = &ctx->fh;
		v4l2_fh_add(&ctx->fh);

		ctx->fh.m2m_ctx = v4l2_m2m_ctx_init(rot_dev->m2m_dev,
			ctx, sde_rotator_queue_init);
		if (IS_ERR_OR_NULL(ctx->fh.m2m_ctx)) {
			ret = PTR_ERR(ctx->fh.m2m_ctx);
			ctx->fh.m2m_ctx = NULL;
			goto error_m2m_init;
		}
	}

	ret = kobject_init_and_add(&ctx->kobj, &sde_rotator_fs_ktype,
			&rot_dev->dev->kobj, "session_%d", ctx->session_id);
	if (ret) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail initialize context kobject\n");
		goto error_kobj_init;
	}

	ret = sysfs_create_group(&ctx->kobj, &sde_rotator_fs_attr_group);
	if (ret) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail register rotator sysfs nodes\n");
		goto error_create_sysfs;
	}

	for (i = 0; i < MAX_ROT_OPEN_SESSION; i++) {
		if (rot_dev->kthread_free[i]) {
			rot_dev->kthread_free[i] = false;
			ctx->kthread_id = i;
			ctx->work_queue.rot_kw = &rot_dev->rot_kw[i];
			ctx->work_queue.rot_thread = rot_dev->rot_thread[i];
			break;
		}
	}

	if (ctx->kthread_id < 0) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail to acquire the kthread\n");
		ret = -EINVAL;
		goto error_alloc_kthread;
	}

	snprintf(name, sizeof(name), "%d_%d", rot_dev->dev->id,
			ctx->session_id);
	ctx->work_queue.timeline = sde_rotator_create_timeline(name);
	if (!ctx->work_queue.timeline)
		SDEDEV_DBG(ctx->rot_dev->dev, "timeline is not available\n");

	sde_rot_mgr_lock(rot_dev->mgr);
	sde_rotator_pm_qos_request(rot_dev,
				 SDE_ROTATOR_ADD_REQUEST);
	ret = sde_rotator_session_open(rot_dev->mgr, &ctx->private,
			ctx->session_id, &ctx->work_queue);
	if (ret < 0) {
		SDEDEV_ERR(ctx->rot_dev->dev, "fail open session\n");
		goto error_open_session;
	}
	sde_rot_mgr_unlock(rot_dev->mgr);

	if (ctx->file) {
		/* Create control */
		ctrl_handler = &ctx->ctrl_handler;
		v4l2_ctrl_handler_init(ctrl_handler, 4);
		v4l2_ctrl_new_std(ctrl_handler,
			&sde_rotator_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
		v4l2_ctrl_new_std(ctrl_handler,
			&sde_rotator_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
		v4l2_ctrl_new_std(ctrl_handler,
			&sde_rotator_ctrl_ops, V4L2_CID_ROTATE, 0, 270, 90, 0);
		v4l2_ctrl_new_custom(ctrl_handler,
			&sde_rotator_ctrl_secure, NULL);
		v4l2_ctrl_new_custom(ctrl_handler,
			&sde_rotator_ctrl_secure_camera, NULL);
		if (ctrl_handler->error) {
			ret = ctrl_handler->error;
			v4l2_ctrl_handler_free(ctrl_handler);
			goto error_ctrl_handler;
		}
		ctx->fh.ctrl_handler = ctrl_handler;
		v4l2_ctrl_handler_setup(ctrl_handler);
	} else {
		/* acquire exclusive context */
		SDEDEV_DBG(rot_dev->dev, "acquire exclusive session id:%u\n",
				ctx->session_id);
		SDEROT_EVTLOG(ctx->session_id);
		rot_dev->excl_ctx = ctx;
	}

	mutex_unlock(&rot_dev->lock);

	SDEDEV_DBG(ctx->rot_dev->dev, "SDE v4l2 rotator open success\n");

	ATRACE_BEGIN(ctx->kobj.name);

	return ctx;
error_ctrl_handler:
error_open_session:
	sde_rot_mgr_unlock(rot_dev->mgr);
	sde_rotator_destroy_timeline(ctx->work_queue.timeline);
	rot_dev->kthread_free[ctx->kthread_id] = true;
	ctx->kthread_id = -1;
error_alloc_kthread:
	sysfs_remove_group(&ctx->kobj, &sde_rotator_fs_attr_group);
error_create_sysfs:
	kobject_put(&ctx->kobj);
error_kobj_init:
	if (ctx->file) {
		v4l2_m2m_ctx_release(ctx->fh.m2m_ctx);
		ctx->fh.m2m_ctx = NULL;
	}
error_m2m_init:
	if (ctx->file) {
		v4l2_fh_del(&ctx->fh);
		v4l2_fh_exit(&ctx->fh);
	}
	mutex_unlock(&rot_dev->lock);
error_lock:
	kfree(ctx);
	ctx = NULL;
	return ERR_PTR(ret);
}

/*
 * sde_rotator_ctx_release - Rotator device release method.
 * @ctx: Pointer rotator context.
 * @file: Pointer to file struct (optional)
 * return: 0 if success; error code, otherwise
 */
static int sde_rotator_ctx_release(struct sde_rotator_ctx *ctx,
		struct file *file)
{
	struct sde_rotator_device *rot_dev;
	u32 session_id;
	struct list_head *curr, *next;

	if (!ctx) {
		SDEROT_DBG("ctx is NULL\n");
		return -EINVAL;
	}

	rot_dev = ctx->rot_dev;
	session_id = ctx->session_id;

	ATRACE_END(ctx->kobj.name);

	SDEDEV_DBG(rot_dev->dev, "release s:%d\n", session_id);
	mutex_lock(&rot_dev->lock);
	if (rot_dev->excl_ctx == ctx) {
		SDEDEV_DBG(rot_dev->dev, "release exclusive session id:%u\n",
				session_id);
		SDEROT_EVTLOG(session_id);
		rot_dev->excl_ctx = NULL;
	}
	if (ctx->file) {
		v4l2_ctrl_handler_free(&ctx->ctrl_handler);
		SDEDEV_DBG(rot_dev->dev, "release streams s:%d\n", session_id);
		if (ctx->fh.m2m_ctx) {
			v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx,
				V4L2_BUF_TYPE_VIDEO_OUTPUT);
			v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx,
				V4L2_BUF_TYPE_VIDEO_CAPTURE);
		}
	}
	mutex_unlock(&rot_dev->lock);
	SDEDEV_DBG(rot_dev->dev, "release submit work s:%d\n", session_id);
	list_for_each_safe(curr, next, &ctx->pending_list) {
		struct sde_rotator_request *request =
			container_of(curr, struct sde_rotator_request, list);

		SDEDEV_DBG(rot_dev->dev, "release submit work s:%d\n",
				session_id);
		kthread_cancel_work_sync(&request->submit_work);
	}
	SDEDEV_DBG(rot_dev->dev, "release session s:%d\n", session_id);
	sde_rot_mgr_lock(rot_dev->mgr);
	sde_rotator_pm_qos_request(rot_dev,
			SDE_ROTATOR_REMOVE_REQUEST);
	sde_rotator_session_close(rot_dev->mgr, ctx->private, session_id);
	sde_rot_mgr_unlock(rot_dev->mgr);
	SDEDEV_DBG(rot_dev->dev, "release retire work s:%d\n", session_id);
	list_for_each_safe(curr, next, &ctx->pending_list) {
		struct sde_rotator_request *request =
			container_of(curr, struct sde_rotator_request, list);

		SDEDEV_DBG(rot_dev->dev, "release retire work s:%d\n",
				session_id);
		kthread_cancel_work_sync(&request->retire_work);
	}
	mutex_lock(&rot_dev->lock);
	SDEDEV_DBG(rot_dev->dev, "release context s:%d\n", session_id);
	sde_rotator_destroy_timeline(ctx->work_queue.timeline);
	if (ctx->kthread_id >= 0 && ctx->work_queue.rot_kw) {
		kthread_flush_worker(ctx->work_queue.rot_kw);
		rot_dev->kthread_free[ctx->kthread_id] = true;
	}
	sysfs_remove_group(&ctx->kobj, &sde_rotator_fs_attr_group);
	kobject_put(&ctx->kobj);
	if (ctx->file) {
		if (ctx->fh.m2m_ctx)
			v4l2_m2m_ctx_release(ctx->fh.m2m_ctx);
		if (ctx->fh.vdev) {
			v4l2_fh_del(&ctx->fh);
			v4l2_fh_exit(&ctx->fh);
		}
	}
	kfree(ctx->vbinfo_out);
	kfree(ctx->vbinfo_cap);
	kfree(ctx);
	wake_up_interruptible(&rot_dev->open_wq);
	mutex_unlock(&rot_dev->lock);
	SDEDEV_DBG(rot_dev->dev, "release complete s:%d\n", session_id);
	return 0;
}

/*
 * sde_rotator_update_retire_sequence - update retired sequence of the context
 *	referenced in the request, and wake up any waiting for update event
 * @request: Pointer to rotator request
 */
static void sde_rotator_update_retire_sequence(
		struct sde_rotator_request *request)
{
	struct sde_rotator_ctx *ctx;

	if (!request || !request->ctx) {
		SDEROT_ERR("invalid parameters\n");
		return;
	}

	ctx = request->ctx;
	ctx->retired_sequence_id = request->sequence_id;

	wake_up(&ctx->wait_queue);

	SDEROT_DBG("update sequence s:%d.%d\n",
				ctx->session_id, ctx->retired_sequence_id);
}

/*
 * sde_rotator_retire_request - retire the given rotator request with
 *	device mutex locked
 * @request: Pointer to rotator request
 */
static void sde_rotator_retire_request(struct sde_rotator_request *request)
{
	struct sde_rotator_ctx *ctx;

	if (!request || !request->ctx) {
		SDEROT_ERR("invalid parameters\n");
		return;
	}

	ctx = request->ctx;

	request->req = NULL;
	request->sequence_id = 0;
	request->committed = false;
	spin_lock(&ctx->list_lock);
	list_del_init(&request->list);
	list_add_tail(&request->list, &ctx->retired_list);
	spin_unlock(&ctx->list_lock);

	wake_up(&ctx->wait_queue);

	SDEROT_DBG("retire request s:%d.%d\n",
				ctx->session_id, ctx->retired_sequence_id);
}

/*
 * sde_rotator_is_request_retired - Return true if given request already expired
 * @request: Pointer to rotator request
 */
static bool sde_rotator_is_request_retired(struct sde_rotator_request *request)
{
	struct sde_rotator_ctx *ctx;
	u32 sequence_id;
	s32 retire_delta;

	if (!request || !request->ctx)
		return true;

	ctx = request->ctx;
	sequence_id = request->sequence_id;

	retire_delta = (s32) (ctx->retired_sequence_id - sequence_id);

	SDEROT_DBG("sequence:%u/%u\n", sequence_id, ctx->retired_sequence_id);

	return retire_delta >= 0;
}

static void sde_rotator_pm_qos_remove(struct sde_rot_data_type *rot_mdata)
{
	struct pm_qos_request *req;
	u32 cpu_mask;

	if (!rot_mdata) {
		SDEROT_DBG("invalid rot device or context\n");
		return;
	}

	cpu_mask = rot_mdata->rot_pm_qos_cpu_mask;

	if (!cpu_mask)
		return;

	req = &rot_mdata->pm_qos_rot_cpu_req;
	pm_qos_remove_request(req);
}

void sde_rotator_pm_qos_add(struct sde_rot_data_type *rot_mdata)
{
	struct pm_qos_request *req;
	u32 cpu_mask;
	int cpu;

	if (!rot_mdata) {
		SDEROT_DBG("invalid rot device or context\n");
		return;
	}

	cpu_mask = rot_mdata->rot_pm_qos_cpu_mask;

	if (!cpu_mask)
		return;

	req = &rot_mdata->pm_qos_rot_cpu_req;
	req->type = PM_QOS_REQ_AFFINE_CORES;
	cpumask_empty(&req->cpus_affine);
	for_each_possible_cpu(cpu) {
		if ((1 << cpu) & cpu_mask)
			cpumask_set_cpu(cpu, &req->cpus_affine);
	}
	pm_qos_add_request(req, PM_QOS_CPU_DMA_LATENCY,
		PM_QOS_DEFAULT_VALUE);

	SDEROT_DBG("rotator pmqos add mask %x latency %x\n",
		rot_mdata->rot_pm_qos_cpu_mask,
		rot_mdata->rot_pm_qos_cpu_dma_latency);
}

static void sde_rotator_pm_qos_request(struct sde_rotator_device *rot_dev,
					 bool add_request)
{
	u32 cpu_mask;
	u32 cpu_dma_latency;
	bool changed = false;

	if (!rot_dev) {
		SDEROT_DBG("invalid rot device or context\n");
		return;
	}

	cpu_mask = rot_dev->mdata->rot_pm_qos_cpu_mask;
	cpu_dma_latency = rot_dev->mdata->rot_pm_qos_cpu_dma_latency;

	if (!cpu_mask)
		return;

	if (add_request) {
		if (rot_dev->mdata->rot_pm_qos_cpu_count == 0)
			changed = true;
		rot_dev->mdata->rot_pm_qos_cpu_count++;
	} else {
		if (rot_dev->mdata->rot_pm_qos_cpu_count != 0) {
			rot_dev->mdata->rot_pm_qos_cpu_count--;
			if (rot_dev->mdata->rot_pm_qos_cpu_count == 0)
				changed = true;
		} else {
			SDEROT_DBG("%s: ref_count is not balanced\n",
				__func__);
		}
	}

	if (!changed)
		return;

	SDEROT_EVTLOG(add_request, cpu_mask, cpu_dma_latency);

	if (!add_request) {
		pm_qos_update_request(&rot_dev->mdata->pm_qos_rot_cpu_req,
			PM_QOS_DEFAULT_VALUE);
		return;
	}

	pm_qos_update_request(&rot_dev->mdata->pm_qos_rot_cpu_req,
		cpu_dma_latency);
}

/*
 * sde_rotator_inline_open - open inline rotator session
 * @pdev: Pointer to rotator platform device
 * @video_mode: true if video mode is requested
 * return: Pointer to new rotator session context
 */
void *sde_rotator_inline_open(struct platform_device *pdev)
{
	struct sde_rotator_device *rot_dev;
	struct sde_rotator_ctx *ctx;
	int rc;

	if (!pdev) {
		SDEROT_ERR("invalid platform device\n");
		return ERR_PTR(-EINVAL);
	}

	rot_dev = (struct sde_rotator_device *) platform_get_drvdata(pdev);
	if (!rot_dev) {
		SDEROT_ERR("invalid rotator device\n");
		return ERR_PTR(-EINVAL);
	}

	ctx = sde_rotator_ctx_open(rot_dev, NULL);
	if (IS_ERR_OR_NULL(ctx)) {
		rc = PTR_ERR(ctx);
		SDEROT_ERR("failed to open rotator context %d\n", rc);
		goto rotator_open_error;
	}

	ctx->slice = llcc_slice_getd(rot_dev->dev, "rotator");
	if (IS_ERR(ctx->slice)) {
		rc = PTR_ERR(ctx->slice);
		SDEROT_ERR("failed to get system cache %d\n", rc);
		goto slice_getd_error;
	}

	if (!rot_dev->disable_syscache) {
		rc = llcc_slice_activate(ctx->slice);
		if (rc) {
			SDEROT_ERR("failed to activate slice %d\n", rc);
			goto activate_error;
		}
		SDEROT_DBG("scid %d size %zukb\n",
				llcc_get_slice_id(ctx->slice),
				llcc_get_slice_size(ctx->slice));
	} else {
		SDEROT_DBG("syscache bypassed\n");
	}

	SDEROT_EVTLOG(ctx->session_id, llcc_get_slice_id(ctx->slice),
			llcc_get_slice_size(ctx->slice),
			rot_dev->disable_syscache);

	return ctx;

activate_error:
	llcc_slice_putd(ctx->slice);
	ctx->slice = NULL;
slice_getd_error:
	sde_rotator_ctx_release(ctx, NULL);
rotator_open_error:
	return ERR_PTR(rc);
}
EXPORT_SYMBOL(sde_rotator_inline_open);

int sde_rotator_inline_release(void *handle)
{
	struct sde_rotator_device *rot_dev;
	struct sde_rotator_ctx *ctx;

	if (!handle) {
		SDEROT_ERR("invalid rotator ctx\n");
		return -EINVAL;
	}

	ctx = handle;
	rot_dev = ctx->rot_dev;

	if (!rot_dev) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	if (ctx->slice) {
		if (!rot_dev->disable_syscache)
			llcc_slice_deactivate(ctx->slice);
		llcc_slice_putd(ctx->slice);
		ctx->slice = NULL;
	}

	SDEROT_EVTLOG(ctx->session_id);

	return sde_rotator_ctx_release(ctx, NULL);
}
EXPORT_SYMBOL(sde_rotator_inline_release);

/*
 * sde_rotator_inline_get_dst_pixfmt - determine output pixel format
 * @pdev: Pointer to platform device
 * @src_pixfmt: input pixel format
 * @dst_pixfmt: Pointer to output pixel format (output)
 * return: 0 if success; error code otherwise
 */
int sde_rotator_inline_get_dst_pixfmt(struct platform_device *pdev,
		u32 src_pixfmt, u32 *dst_pixfmt)
{
	int rc;

	if (!src_pixfmt || !dst_pixfmt)
		return -EINVAL;

	rc = sde_rot_get_base_tilea5x_pixfmt(src_pixfmt, dst_pixfmt);
	if (rc)
		return rc;

	/*
	 * Currently, NV21 tile is not supported as output; hence,
	 * override with NV12 tile.
	 */
	if (*dst_pixfmt == SDE_PIX_FMT_Y_CRCB_H2V2_TILE)
		*dst_pixfmt = SDE_PIX_FMT_Y_CBCR_H2V2_TILE;

	return 0;
}
EXPORT_SYMBOL(sde_rotator_inline_get_dst_pixfmt);

/*
 * sde_rotator_inline_get_downscale_caps - get scaling capability
 * @pdev: Pointer to platform device
 * @caps: string buffer for capability
 * @len: length of string buffer
 * return: length of capability string
 */
int sde_rotator_inline_get_downscale_caps(struct platform_device *pdev,
		char *caps, int len)
{
	struct sde_rotator_device *rot_dev;
	int rc;

	if (!pdev) {
		SDEROT_ERR("invalid platform device\n");
		return -EINVAL;
	}

	rot_dev = (struct sde_rotator_device *) platform_get_drvdata(pdev);
	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	rc = sde_rotator_get_downscale_caps(rot_dev->mgr, caps, len);
	sde_rot_mgr_unlock(rot_dev->mgr);

	return rc;
}
EXPORT_SYMBOL(sde_rotator_inline_get_downscale_caps);

/*
 * sde_rotator_inline_get_maxlinewidth - get maximum line width of rotator
 * @pdev: Pointer to platform device
 * return: maximum line width
 */
int sde_rotator_inline_get_maxlinewidth(struct platform_device *pdev)
{
	struct sde_rotator_device *rot_dev;
	int maxlinewidth;

	if (!pdev) {
		SDEROT_ERR("invalid platform device\n");
		return -EINVAL;
	}

	rot_dev = (struct sde_rotator_device *)platform_get_drvdata(pdev);
	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	maxlinewidth = sde_rotator_get_maxlinewidth(rot_dev->mgr);
	sde_rot_mgr_unlock(rot_dev->mgr);

	return maxlinewidth;
}
EXPORT_SYMBOL(sde_rotator_inline_get_maxlinewidth);

/*
 * sde_rotator_inline_get_pixfmt_caps - get pixel format capability
 * @pdev: Pointer to platform device
 * @pixfmt: array of pixel format buffer
 * @len: length of pixel format buffer
 * return: length of pixel format capability if success; error code otherwise
 */
int sde_rotator_inline_get_pixfmt_caps(struct platform_device *pdev,
		bool input, u32 *pixfmts, int len)
{
	struct sde_rotator_device *rot_dev;
	u32 i, pixfmt;

	if (!pdev) {
		SDEROT_ERR("invalid platform device\n");
		return -EINVAL;
	}

	rot_dev = (struct sde_rotator_device *) platform_get_drvdata(pdev);
	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	for (i = 0;; i++) {
		pixfmt = sde_rotator_get_pixfmt(rot_dev->mgr, i, input,
				SDE_ROTATOR_MODE_SBUF);
		if (!pixfmt)
			break;
		if (pixfmts && i < len)
			pixfmts[i] = pixfmt;
	}
	sde_rot_mgr_unlock(rot_dev->mgr);

	return i;
}
EXPORT_SYMBOL(sde_rotator_inline_get_pixfmt_caps);

/*
 * _sde_rotator_inline_cleanup - perform inline related request cleanup
 *	This function assumes rot_dev->mgr lock has been taken when called.
 * @handle: Pointer to rotator context
 * @request: Pointer to rotation request
 * return: 0 if success; -EAGAIN if cleanup should be retried
 */
static int _sde_rotator_inline_cleanup(void *handle,
		struct sde_rotator_request *request)
{
	struct sde_rotator_ctx *ctx;
	struct sde_rotator_device *rot_dev;
	int ret;

	if (!handle || !request) {
		SDEROT_ERR("invalid rotator handle/request\n");
		return -EINVAL;
	}

	ctx = handle;
	rot_dev = ctx->rot_dev;

	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	if (request->committed) {
		/* wait until request is finished */
		sde_rot_mgr_unlock(rot_dev->mgr);
		mutex_unlock(&rot_dev->lock);
		ret = wait_event_timeout(ctx->wait_queue,
			sde_rotator_is_request_retired(request),
			msecs_to_jiffies(rot_dev->streamoff_timeout));
		mutex_lock(&rot_dev->lock);
		sde_rot_mgr_lock(rot_dev->mgr);

		if (!ret) {
			SDEROT_ERR("timeout w/o retire s:%d\n",
					ctx->session_id);
			SDEROT_EVTLOG(ctx->session_id, SDE_ROT_EVTLOG_ERROR);
			sde_rotator_abort_inline_request(rot_dev->mgr,
					ctx->private, request->req);
			return -EAGAIN;
		} else if (ret == 1) {
			SDEROT_ERR("timeout w/ retire s:%d\n", ctx->session_id);
			SDEROT_EVTLOG(ctx->session_id, SDE_ROT_EVTLOG_ERROR);
		}
	}

	sde_rotator_req_finish(rot_dev->mgr, ctx->private, request->req);
	sde_rotator_retire_request(request);
	return 0;
}

/*
 * sde_rotator_inline_commit - commit given rotator command
 * @handle: Pointer to rotator context
 * @cmd: Pointer to rotator command
 * @cmd_type: command type - validate/prepare/commit/cleanup
 * return: 0 if success; error code otherwise
 */
int sde_rotator_inline_commit(void *handle, struct sde_rotator_inline_cmd *cmd,
		enum sde_rotator_inline_cmd_type cmd_type)
{
	struct sde_rotator_ctx *ctx;
	struct sde_rotator_device *rot_dev;
	struct sde_rotator_request *request = NULL;
	struct sde_rot_entry_container *req = NULL;
	struct sde_rotation_config rotcfg;
	ktime_t *ts;
	u32 flags = 0;
	int i, ret = 0;

	if (!handle || !cmd) {
		SDEROT_ERR("invalid rotator handle/cmd\n");
		return -EINVAL;
	}

	ctx = handle;
	rot_dev = ctx->rot_dev;

	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return -EINVAL;
	}

	SDEROT_DBG(
		"s:%d.%u src:(%u,%u,%u,%u)/%ux%u/%c%c%c%c dst:(%u,%u,%u,%u)/%c%c%c%c r:%d f:%d/%d s:%d fps:%u clk:%llu bw:%llu prefill:%llu wb:%d vid:%d cmd:%d\n",
		ctx->session_id, cmd->sequence_id,
		cmd->src_rect_x, cmd->src_rect_y,
		cmd->src_rect_w, cmd->src_rect_h,
		cmd->src_width, cmd->src_height,
		cmd->src_pixfmt >> 0, cmd->src_pixfmt >> 8,
		cmd->src_pixfmt >> 16, cmd->src_pixfmt >> 24,
		cmd->dst_rect_x, cmd->dst_rect_y,
		cmd->dst_rect_w, cmd->dst_rect_h,
		cmd->dst_pixfmt >> 0, cmd->dst_pixfmt >> 8,
		cmd->dst_pixfmt >> 16, cmd->dst_pixfmt >> 24,
		cmd->rot90, cmd->hflip, cmd->vflip, cmd->secure, cmd->fps,
		cmd->clkrate, cmd->data_bw, cmd->prefill_bw,
		cmd->dst_writeback, cmd->video_mode, cmd_type);
	SDEROT_EVTLOG(ctx->session_id, cmd->sequence_id,
		cmd->src_rect_x, cmd->src_rect_y,
		cmd->src_rect_w, cmd->src_rect_h,
		cmd->src_pixfmt,
		cmd->dst_rect_w, cmd->dst_rect_h,
		cmd->dst_pixfmt,
		cmd->fps, cmd->clkrate, cmd->data_bw, cmd->prefill_bw,
		(cmd->rot90 << 0) | (cmd->hflip << 1) | (cmd->vflip << 2) |
		(cmd->secure << 3) | (cmd->dst_writeback << 4) |
		(cmd->video_mode << 5) |
		(cmd_type << 24));

	mutex_lock(&rot_dev->lock);
	sde_rot_mgr_lock(rot_dev->mgr);

	if (cmd_type == SDE_ROTATOR_INLINE_CMD_VALIDATE ||
			cmd_type == SDE_ROTATOR_INLINE_CMD_COMMIT) {

		struct sde_rotation_item item;
		struct sde_rotator_statistics *stats = &rot_dev->stats;
		int scid = llcc_get_slice_id(ctx->slice);

		/* allocate slot for timestamp */
		ts = stats->ts[stats->count % SDE_ROTATOR_NUM_EVENTS];
		if (cmd_type == SDE_ROTATOR_INLINE_CMD_COMMIT)
			stats->count++;

		if (cmd->rot90)
			flags |= SDE_ROTATION_90;
		if (cmd->hflip)
			flags |= SDE_ROTATION_FLIP_LR;
		if (cmd->vflip)
			flags |= SDE_ROTATION_FLIP_UD;
		if (cmd->secure)
			flags |= SDE_ROTATION_SECURE;

		flags |= SDE_ROTATION_EXT_PERF;

		/* fill in item work structure */
		memset(&item, 0, sizeof(struct sde_rotation_item));
		item.flags = flags | SDE_ROTATION_EXT_IOVA;
		item.trigger = cmd->video_mode ? SDE_ROTATOR_TRIGGER_VIDEO :
				SDE_ROTATOR_TRIGGER_COMMAND;
		item.prefill_bw = cmd->prefill_bw;
		item.session_id = ctx->session_id;
		item.sequence_id = cmd->sequence_id;
		item.src_rect.x = cmd->src_rect_x;
		item.src_rect.y = cmd->src_rect_y;
		item.src_rect.w = cmd->src_rect_w;
		item.src_rect.h = cmd->src_rect_h;
		item.input.width = cmd->src_width;
		item.input.height = cmd->src_height;
		item.input.format = cmd->src_pixfmt;

		for (i = 0; i < SDE_ROTATOR_INLINE_PLANE_MAX; i++) {
			item.input.planes[i].addr = cmd->src_addr[i];
			item.input.planes[i].len = cmd->src_len[i];
			item.input.planes[i].fd = -1;
		}
		item.input.plane_count = cmd->src_planes;
		item.input.comp_ratio.numer = 1;
		item.input.comp_ratio.denom = 1;

		item.output.width = cmd->dst_rect_x + cmd->dst_rect_w;
		item.output.height = cmd->dst_rect_y + cmd->dst_rect_h;
		item.dst_rect.x = cmd->dst_rect_x;
		item.dst_rect.y = cmd->dst_rect_y;
		item.dst_rect.w = cmd->dst_rect_w;
		item.dst_rect.h = cmd->dst_rect_h;
		item.output.sbuf = true;
		item.output.scid = scid;
		item.output.writeback = cmd->dst_writeback;
		item.output.format = cmd->dst_pixfmt;

		for (i = 0; i < SDE_ROTATOR_INLINE_PLANE_MAX; i++) {
			item.output.planes[i].addr = cmd->dst_addr[i];
			item.output.planes[i].len = cmd->dst_len[i];
			item.output.planes[i].fd = -1;
		}
		item.output.plane_count = cmd->dst_planes;
		item.output.comp_ratio.numer = 1;
		item.output.comp_ratio.denom = 1;
		item.sequence_id = ++(ctx->commit_sequence_id);
		item.ts = ts;

		req = sde_rotator_req_init(rot_dev->mgr, ctx->private,
				&item, 1, 0);
		if (IS_ERR_OR_NULL(req)) {
			SDEROT_ERR("fail allocate request s:%d\n",
					ctx->session_id);
			ret = -ENOMEM;
			goto error_init_request;
		}

		/* initialize session configuration */
		memset(&rotcfg, 0, sizeof(struct sde_rotation_config));
		rotcfg.flags = flags;
		rotcfg.frame_rate = cmd->fps;
		rotcfg.clk_rate = cmd->clkrate;
		rotcfg.data_bw = cmd->data_bw;
		rotcfg.session_id = ctx->session_id;
		rotcfg.input.width = cmd->src_rect_w;
		rotcfg.input.height = cmd->src_rect_h;
		rotcfg.input.format = cmd->src_pixfmt;
		rotcfg.input.comp_ratio.numer = 1;
		rotcfg.input.comp_ratio.denom = 1;
		rotcfg.output.width = cmd->dst_rect_w;
		rotcfg.output.height = cmd->dst_rect_h;
		rotcfg.output.format = cmd->dst_pixfmt;
		rotcfg.output.comp_ratio.numer = 1;
		rotcfg.output.comp_ratio.denom = 1;
		rotcfg.output.sbuf = true;
	}

	if (cmd_type == SDE_ROTATOR_INLINE_CMD_VALIDATE) {

		ret = sde_rotator_session_validate(rot_dev->mgr,
				ctx->private, &rotcfg);
		if (ret) {
			SDEROT_WARN("fail session validation s:%d\n",
					ctx->session_id);
			goto error_session_validate;
		}

		devm_kfree(rot_dev->dev, req);
		req = NULL;

	} else if (cmd_type == SDE_ROTATOR_INLINE_CMD_COMMIT) {

		if (memcmp(&rotcfg, &ctx->rotcfg, sizeof(rotcfg))) {
			ret = sde_rotator_session_config(rot_dev->mgr,
					ctx->private, &rotcfg);
			if (ret) {
				SDEROT_ERR("fail session config s:%d\n",
						ctx->session_id);
				goto error_session_config;
			}

			ctx->rotcfg = rotcfg;
		}

		request = list_first_entry_or_null(&ctx->retired_list,
				struct sde_rotator_request, list);
		if (!request) {
			/* should not happen */
			ret = -ENOMEM;
			SDEROT_ERR("no free request s:%d\n", ctx->session_id);
			goto error_retired_list;
		}

		request->req = req;
		request->sequence_id = req->entries[0].item.sequence_id;

		spin_lock(&ctx->list_lock);
		list_del_init(&request->list);
		list_add_tail(&request->list, &ctx->pending_list);
		spin_unlock(&ctx->list_lock);

		ts = req->entries[0].item.ts;
		if (ts) {
			ts[SDE_ROTATOR_TS_SRCQB] = ktime_get();
			ts[SDE_ROTATOR_TS_DSTQB] = ktime_get();
			ts[SDE_ROTATOR_TS_FENCE] = ktime_get();
		} else {
			SDEROT_ERR("invalid stats timestamp\n");
		}
		req->retire_kw = ctx->work_queue.rot_kw;
		req->retire_work = &request->retire_work;

		trace_rot_entry_fence(
			ctx->session_id, cmd->sequence_id,
			req->entries[0].item.wb_idx,
			req->entries[0].item.flags,
			req->entries[0].item.input.format,
			req->entries[0].item.input.width,
			req->entries[0].item.input.height,
			req->entries[0].item.src_rect.x,
			req->entries[0].item.src_rect.y,
			req->entries[0].item.src_rect.w,
			req->entries[0].item.src_rect.h,
			req->entries[0].item.output.format,
			req->entries[0].item.output.width,
			req->entries[0].item.output.height,
			req->entries[0].item.dst_rect.x,
			req->entries[0].item.dst_rect.y,
			req->entries[0].item.dst_rect.w,
			req->entries[0].item.dst_rect.h);

		ret = sde_rotator_handle_request_common(
				rot_dev->mgr, ctx->private, req);
		if (ret) {
			SDEROT_ERR("fail handle request s:%d\n",
					ctx->session_id);
			goto error_handle_request;
		}

		sde_rotator_req_reset_start(rot_dev->mgr, req);

		sde_rotator_queue_request(rot_dev->mgr, ctx->private, req);

		request->committed = true;

		/* save request in private handle */
		cmd->priv_handle = request;

	} else if (cmd_type == SDE_ROTATOR_INLINE_CMD_START) {
		if (!cmd->priv_handle) {
			ret = -EINVAL;
			SDEROT_ERR("invalid private handle\n");
			goto error_invalid_handle;
		}

		request = cmd->priv_handle;
		sde_rotator_req_set_start(rot_dev->mgr, request->req);
	} else if (cmd_type == SDE_ROTATOR_INLINE_CMD_CLEANUP) {
		if (!cmd->priv_handle) {
			ret = -EINVAL;
			SDEROT_ERR("invalid private handle\n");
			goto error_invalid_handle;
		}

		request = cmd->priv_handle;

		/* attempt single retry if first cleanup attempt failed */
		if (_sde_rotator_inline_cleanup(handle, request) == -EAGAIN)
			_sde_rotator_inline_cleanup(handle, request);

		cmd->priv_handle = NULL;
	} else if (cmd_type == SDE_ROTATOR_INLINE_CMD_ABORT) {
		if (!cmd->priv_handle) {
			ret = -EINVAL;
			SDEROT_ERR("invalid private handle\n");
			goto error_invalid_handle;
		}

		request = cmd->priv_handle;
		if (!sde_rotator_is_request_retired(request))
			sde_rotator_abort_inline_request(rot_dev->mgr,
					ctx->private, request->req);
	}

	sde_rot_mgr_unlock(rot_dev->mgr);
	mutex_unlock(&rot_dev->lock);
	return 0;

error_handle_request:
	sde_rotator_update_retire_sequence(request);
	sde_rotator_retire_request(request);
error_retired_list:
error_session_validate:
error_session_config:
	devm_kfree(rot_dev->dev, req);
error_invalid_handle:
error_init_request:
	sde_rot_mgr_unlock(rot_dev->mgr);
	mutex_unlock(&rot_dev->lock);
	return ret;
}
EXPORT_SYMBOL(sde_rotator_inline_commit);

void sde_rotator_inline_reg_dump(struct platform_device *pdev)
{
	struct sde_rotator_device *rot_dev;

	if (!pdev) {
		SDEROT_ERR("invalid platform device\n");
		return;
	}

	rot_dev = (struct sde_rotator_device *) platform_get_drvdata(pdev);
	if (!rot_dev || !rot_dev->mgr) {
		SDEROT_ERR("invalid rotator device\n");
		return;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	sde_rotator_core_dump(rot_dev->mgr);
	sde_rot_mgr_unlock(rot_dev->mgr);
}
EXPORT_SYMBOL(sde_rotator_inline_reg_dump);

/*
 * sde_rotator_open - Rotator device open method.
 * @file: Pointer to file struct.
 */
static int sde_rotator_open(struct file *file)
{
	struct sde_rotator_device *rot_dev = video_drvdata(file);
	struct sde_rotator_ctx *ctx;
	int ret = 0;

	ctx = sde_rotator_ctx_open(rot_dev, file);
	if (IS_ERR_OR_NULL(ctx)) {
		SDEDEV_DBG(rot_dev->dev, "failed to open %d\n", ret);
		ret = PTR_ERR(ctx);
	}

	return ret;
}

/*
 * sde_rotator_release - Rotator device release method.
 * @file: Pointer to file struct.
 */
static int sde_rotator_release(struct file *file)
{
	struct sde_rotator_ctx *ctx =
			sde_rotator_ctx_from_fh(file->private_data);

	return sde_rotator_ctx_release(ctx, file);
}

/*
 * sde_rotator_poll - rotator device pool method.
 * @file: Pointer to file struct.
 * @wait: Pointer to poll table struct.
 */
static unsigned int sde_rotator_poll(struct file *file,
	struct poll_table_struct *wait)
{
	struct sde_rotator_device *rot_dev = video_drvdata(file);
	struct sde_rotator_ctx *ctx =
			sde_rotator_ctx_from_fh(file->private_data);
	int ret;

	mutex_lock(&rot_dev->lock);
	ret = v4l2_m2m_poll(file, ctx->fh.m2m_ctx, wait);
	mutex_unlock(&rot_dev->lock);
	return ret;
}

/* rotator device file operations callbacks */
static const struct v4l2_file_operations sde_rotator_fops = {
	.owner          = THIS_MODULE,
	.open           = sde_rotator_open,
	.release        = sde_rotator_release,
	.poll           = sde_rotator_poll,
	.unlocked_ioctl = video_ioctl2,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = sde_rotator_compat_ioctl32,
#endif
};

/*
 * sde_rotator_querycap - V4l2 ioctl query capability handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @cap: Pointer to v4l2_capability struct need to be filled.
 */
static int sde_rotator_querycap(struct file *file,
	void *fh, struct v4l2_capability *cap)
{
	cap->bus_info[0] = 0;
	strlcpy(cap->driver, SDE_ROTATOR_DRV_NAME, sizeof(cap->driver));
	strlcpy(cap->card, SDE_ROTATOR_DRV_NAME, sizeof(cap->card));
	cap->device_caps = V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_M2M |
			V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_CAPTURE;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

/*
 * sde_rotator_enum_fmt_vid_cap - V4l2 ioctl enumerate output format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_fmtdesc struct need to be filled.
 */
static int sde_rotator_enum_fmt_vid_cap(struct file *file,
	void *fh, struct v4l2_fmtdesc *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_mdp_format_params *fmt;
	u32 i, index, pixfmt;
	bool found = false;

	for (i = 0, index = 0; index <= f->index; i++) {
		pixfmt = sde_rotator_get_pixfmt(rot_dev->mgr, i, false,
				SDE_ROTATOR_MODE_OFFLINE);
		if (!pixfmt)
			return -EINVAL;

		fmt = sde_get_format_params(pixfmt);
		if (!fmt)
			return -EINVAL;

		if (sde_mdp_is_private_format(fmt))
			continue;

		if (index == f->index) {
			found = true;
			break;
		}

		index++;
	}

	if (!found)
		return -EINVAL;

	f->pixelformat = pixfmt;
	strlcpy(f->description, fmt->description, sizeof(f->description));

	return 0;
}

/*
 * sde_rotator_enum_fmt_vid_out - V4l2 ioctl enumerate capture format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_fmtdesc struct need to be filled.
 */
static int sde_rotator_enum_fmt_vid_out(struct file *file,
	void *fh, struct v4l2_fmtdesc *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_mdp_format_params *fmt;
	u32 i, index, pixfmt;
	bool found = false;

	for (i = 0, index = 0; index <= f->index; i++) {
		pixfmt = sde_rotator_get_pixfmt(rot_dev->mgr, i, true,
				SDE_ROTATOR_MODE_OFFLINE);
		if (!pixfmt)
			return -EINVAL;

		fmt = sde_get_format_params(pixfmt);
		if (!fmt)
			return -EINVAL;

		if (sde_mdp_is_private_format(fmt))
			continue;

		if (index == f->index) {
			found = true;
			break;
		}

		index++;
	}

	if (!found)
		return -EINVAL;

	f->pixelformat = pixfmt;
	strlcpy(f->description, fmt->description, sizeof(f->description));

	return 0;
}

/*
 * sde_rotator_g_fmt_cap - V4l2 ioctl get capture format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct need to be filled.
 */
static int sde_rotator_g_fmt_cap(struct file *file, void *fh,
	struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	*f = ctx->format_cap;

	return 0;
}

/*
 * sde_rotator_g_fmt_out - V4l2 ioctl get output format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct need to be filled.
 */
static int sde_rotator_g_fmt_out(struct file *file, void *fh,
	struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	*f = ctx->format_out;

	return 0;
}

/*
 * sde_rotator_try_fmt_vid_cap - V4l2 ioctl try capture format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct.
 */
static int sde_rotator_try_fmt_vid_cap(struct file *file,
	void *fh, struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotation_config config;
	int ret;

	if ((f->fmt.pix.width == 0) || (f->fmt.pix.height == 0)) {
		SDEDEV_WARN(ctx->rot_dev->dev,
				"Not supporting 0 width/height: %dx%d\n",
				f->fmt.pix.width, f->fmt.pix.height);
		return -EINVAL;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	sde_rotator_get_config_from_ctx(ctx, &config);
	config.output.format = f->fmt.pix.pixelformat;
	config.output.width = f->fmt.pix.width;
	config.output.height = f->fmt.pix.height;
	config.flags |= SDE_ROTATION_VERIFY_INPUT_ONLY;
	ret = sde_rotator_verify_config_output(rot_dev->mgr, &config);
	sde_rot_mgr_unlock(rot_dev->mgr);
	if (ret) {
		if ((config.output.width == f->fmt.pix.width) &&
				(config.output.height == f->fmt.pix.height)) {
			SDEDEV_WARN(ctx->rot_dev->dev,
				"invalid capture format 0x%8.8x %dx%d\n",
				f->fmt.pix.pixelformat,
				f->fmt.pix.width,
				f->fmt.pix.height);
			return -EINVAL;
		}
		f->fmt.pix.width = config.output.width;
		f->fmt.pix.height = config.output.height;
	}

	sde_rotator_format_recalc(f);
	return ret;
}

/*
 * sde_rotator_try_fmt_vid_out - V4l2 ioctl try output format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct.
 */
static int sde_rotator_try_fmt_vid_out(struct file *file,
	void *fh, struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotation_config config;
	int ret;

	if ((f->fmt.pix.width == 0) || (f->fmt.pix.height == 0)) {
		SDEDEV_WARN(ctx->rot_dev->dev,
				"Not supporting 0 width/height: %dx%d\n",
				f->fmt.pix.width, f->fmt.pix.height);
		return -EINVAL;
	}

	sde_rot_mgr_lock(rot_dev->mgr);
	sde_rotator_get_config_from_ctx(ctx, &config);
	config.input.format = f->fmt.pix.pixelformat;
	config.input.width = f->fmt.pix.width;
	config.input.height = f->fmt.pix.height;
	config.flags |= SDE_ROTATION_VERIFY_INPUT_ONLY;
	ret = sde_rotator_verify_config_input(rot_dev->mgr, &config);
	sde_rot_mgr_unlock(rot_dev->mgr);
	if (ret) {
		if ((config.input.width == f->fmt.pix.width) &&
				(config.input.height == f->fmt.pix.height)) {
			SDEDEV_WARN(ctx->rot_dev->dev,
				"invalid output format 0x%8.8x %dx%d\n",
				f->fmt.pix.pixelformat,
				f->fmt.pix.width,
				f->fmt.pix.height);
			return -EINVAL;
		}
		f->fmt.pix.width = config.input.width;
		f->fmt.pix.height = config.input.height;
	}

	sde_rotator_format_recalc(f);
	return ret;
}

/*
 * sde_rotator_s_fmt_vid_cap - V4l2 ioctl set capture format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct.
 */
static int sde_rotator_s_fmt_vid_cap(struct file *file,
	void *fh, struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	int ret;

	ret = sde_rotator_try_fmt_vid_cap(file, fh, f);
	if (ret)
		return -EINVAL;

	/* Initialize crop */
	ctx->crop_cap.top = 0;
	ctx->crop_cap.left = 0;
	ctx->crop_cap.width = f->fmt.pix.width;
	ctx->crop_cap.height = f->fmt.pix.height;

	ctx->format_cap = *f;

	SDEDEV_DBG(rot_dev->dev,
		"s_fmt s:%d t:%d fmt:0x%8.8x field:%u (%u,%u)\n",
		ctx->session_id, f->type,
		f->fmt.pix.pixelformat,
		f->fmt.pix.field,
		f->fmt.pix.width, f->fmt.pix.height);

	return 0;
}

/*
 * sde_rotator_s_fmt_vid_out - V4l2 ioctl set output format handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @f: Pointer to v4l2_format struct.
 */
static int sde_rotator_s_fmt_vid_out(struct file *file,
	void *fh, struct v4l2_format *f)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	int ret;

	ret = sde_rotator_try_fmt_vid_out(file, fh, f);
	if (ret)
		return -EINVAL;

	/* Initialize crop */
	ctx->crop_out.top = 0;
	ctx->crop_out.left = 0;
	ctx->crop_out.width = f->fmt.pix.width;
	ctx->crop_out.height = f->fmt.pix.height;

	ctx->format_out = *f;

	SDEDEV_DBG(rot_dev->dev,
		"s_fmt s:%d t:%d fmt:0x%8.8x field:%u (%u,%u)\n",
		ctx->session_id, f->type,
		f->fmt.pix.pixelformat,
		f->fmt.pix.field,
		f->fmt.pix.width, f->fmt.pix.height);

	return 0;
}

/*
 * sde_rotator_reqbufs - V4l2 ioctl request buffers handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @req: Pointer to v4l2_requestbuffer struct.
 */
static int sde_rotator_reqbufs(struct file *file,
	void *fh, struct v4l2_requestbuffers *req)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	return v4l2_m2m_reqbufs(file, ctx->fh.m2m_ctx, req);
}

/*
 * sde_rotator_qbuf - V4l2 ioctl queue buffer handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @buf: Pointer to v4l2_buffer struct.
 */
static int sde_rotator_qbuf(struct file *file, void *fh,
	struct v4l2_buffer *buf)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	int ret;

	/* create fence for capture buffer */
	if ((buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
			&& (buf->index < ctx->nbuf_cap)) {
		int idx = buf->index;

		ctx->vbinfo_cap[idx].fd = -1;
		ctx->vbinfo_cap[idx].fence = sde_rotator_get_sync_fence(
				ctx->work_queue.timeline, NULL,
				&ctx->vbinfo_cap[idx].fence_ts);
		ctx->vbinfo_cap[idx].qbuf_ts = ktime_get();
		ctx->vbinfo_cap[idx].dqbuf_ts = NULL;
		SDEDEV_DBG(ctx->rot_dev->dev,
				"create buffer fence s:%d.%u i:%d f:%p\n",
				ctx->session_id,
				ctx->vbinfo_cap[idx].fence_ts,
				idx,
				ctx->vbinfo_cap[idx].fence);
	} else if ((buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT)
			&& (buf->index < ctx->nbuf_out)) {
		int idx = buf->index;

		ctx->vbinfo_out[idx].qbuf_ts = ktime_get();
		ctx->vbinfo_out[idx].dqbuf_ts = NULL;
	}

	ret = v4l2_m2m_qbuf(file, ctx->fh.m2m_ctx, buf);
	if (ret < 0)
		SDEDEV_ERR(ctx->rot_dev->dev, "fail qbuf s:%d t:%d r:%d\n",
				ctx->session_id, buf->type, ret);
	SDEROT_EVTLOG(buf->type, buf->bytesused, buf->length, buf->m.fd, ret);

	return ret;
}

/*
 * sde_rotator_dqbuf - V4l2 ioctl dequeue buffer handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @buf: Pointer to v4l2_buffer struct.
 */
static int sde_rotator_dqbuf(struct file *file,
	void *fh, struct v4l2_buffer *buf)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	int ret;

	ret = v4l2_m2m_dqbuf(file, ctx->fh.m2m_ctx, buf);

	if (ret) {
		SDEDEV_ERR(ctx->rot_dev->dev,
				"fail dqbuf s:%d t:%d i:%d r:%d\n",
				ctx->session_id, buf->type, buf->index, ret);
		return ret;
	}

	/* clear fence for buffer */
	if ((buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
			&& (buf->index < ctx->nbuf_cap)) {
		int idx = buf->index;

		if (ctx->vbinfo_cap[idx].fence) {
			/* fence is not used */
			SDEDEV_DBG(ctx->rot_dev->dev, "put fence s:%d i:%d\n",
					ctx->session_id, idx);
			sde_rotator_put_sync_fence(ctx->vbinfo_cap[idx].fence);
		}
		ctx->vbinfo_cap[idx].fence = NULL;
		ctx->vbinfo_cap[idx].fd = -1;
		if (ctx->vbinfo_cap[idx].dqbuf_ts)
			*(ctx->vbinfo_cap[idx].dqbuf_ts) = ktime_get();
	} else if ((buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT)
			&& (buf->index < ctx->nbuf_out)) {
		int idx = buf->index;

		ctx->vbinfo_out[idx].fence = NULL;
		ctx->vbinfo_out[idx].fd = -1;
		if (ctx->vbinfo_out[idx].dqbuf_ts)
			*(ctx->vbinfo_out[idx].dqbuf_ts) = ktime_get();
	} else {
		SDEDEV_WARN(ctx->rot_dev->dev, "invalid dq s:%d t:%d i:%d\n",
				ctx->session_id, buf->type, buf->index);
	}

	return 0;
}

/*
 * sde_rotator_querybuf - V4l2 ioctl query buffer handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @buf: Pointer to v4l2_buffer struct.
 */
static int sde_rotator_querybuf(struct file *file,
	void *fh, struct v4l2_buffer *buf)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	return v4l2_m2m_querybuf(file, ctx->fh.m2m_ctx, buf);
}

/*
 * sde_rotator_streamon - V4l2 ioctl stream on handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @buf_type: V4l2 buffer type.
 */
static int sde_rotator_streamon(struct file *file,
	void *fh, enum v4l2_buf_type buf_type)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotation_config config;
	struct vb2_queue *vq;
	int ret;

	SDEDEV_DBG(ctx->rot_dev->dev, "stream on s:%d t:%d\n",
			ctx->session_id, buf_type);

	vq = v4l2_m2m_get_vq(ctx->fh.m2m_ctx,
			buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT ?
			V4L2_BUF_TYPE_VIDEO_CAPTURE :
			V4L2_BUF_TYPE_VIDEO_OUTPUT);

	if (!vq) {
		SDEDEV_ERR(ctx->rot_dev->dev, "fail to get vq on s:%d t:%d\n",
				ctx->session_id, buf_type);
		return -EINVAL;
	}

	if (vb2_is_streaming(vq)) {
		sde_rot_mgr_lock(rot_dev->mgr);
		sde_rotator_get_config_from_ctx(ctx, &config);
		config.flags &= ~SDE_ROTATION_VERIFY_INPUT_ONLY;
		ret = sde_rotator_session_config(rot_dev->mgr, ctx->private,
				&config);
		sde_rot_mgr_unlock(rot_dev->mgr);
		if (ret < 0) {
			SDEDEV_ERR(rot_dev->dev,
				"fail config in stream on s:%d t:%d r:%d\n",
				ctx->session_id, buf_type, ret);
			return ret;
		}
		ctx->rotcfg = config;
	}

	ret = v4l2_m2m_streamon(file, ctx->fh.m2m_ctx, buf_type);
	if (ret < 0)
		SDEDEV_ERR(ctx->rot_dev->dev, "fail stream on s:%d t:%d\n",
				ctx->session_id, buf_type);

	return ret;
}

/*
 * sde_rotator_streamoff - V4l2 ioctl stream off handler.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @buf_type: V4l2 buffer type.
 */
static int sde_rotator_streamoff(struct file *file,
	void *fh, enum v4l2_buf_type buf_type)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	int ret;

	SDEDEV_DBG(ctx->rot_dev->dev, "stream off s:%d t:%d\n",
			ctx->session_id, buf_type);

	ret = v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx, buf_type);
	if (ret < 0)
		SDEDEV_ERR(ctx->rot_dev->dev, "fail stream off s:%d t:%d\n",
				ctx->session_id, buf_type);

	return ret;
}

/*
 * sde_rotator_cropcap - V4l2 ioctl crop capabilities.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @a: Pointer to v4l2_cropcap struct need to be set.
 */
static int sde_rotator_cropcap(struct file *file, void *fh,
	struct v4l2_cropcap *a)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct v4l2_format *format;
	struct v4l2_rect *crop;

	switch (a->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		format = &ctx->format_out;
		crop = &ctx->crop_out;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		format = &ctx->format_cap;
		crop = &ctx->crop_cap;
		break;
	default:
		return -EINVAL;
	}

	a->bounds.top = 0;
	a->bounds.left = 0;
	a->bounds.width = format->fmt.pix.width;
	a->bounds.height = format->fmt.pix.height;

	a->defrect = *crop;

	a->pixelaspect.numerator = 1;
	a->pixelaspect.denominator = 1;

	SDEROT_EVTLOG(format->fmt.pix.width, format->fmt.pix.height, a->type);
	return 0;
}

/*
 * sde_rotator_g_crop - V4l2 ioctl get crop.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @crop: Pointer to v4l2_crop struct need to be set.
 */
static int sde_rotator_g_crop(struct file *file, void *fh,
	struct v4l2_crop *crop)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	switch (crop->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		crop->c = ctx->crop_out;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		crop->c = ctx->crop_cap;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*
 * sde_rotator_s_crop - V4l2 ioctl set crop.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @crop: Pointer to v4l2_crop struct need to be set.
 */
static int sde_rotator_s_crop(struct file *file, void *fh,
	const struct v4l2_crop *crop)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotation_item item;
	struct v4l2_rect rect;

	sde_rotator_get_item_from_ctx(ctx, &item);

	rect.left = max_t(__u32, crop->c.left, 0);
	rect.top = max_t(__u32, crop->c.top, 0);
	rect.height = max_t(__u32, crop->c.height, 0);
	rect.width = max_t(__u32, crop->c.width, 0);

	if (crop->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		rect.left = min_t(__u32, rect.left,
				ctx->format_out.fmt.pix.width - 1);
		rect.top = min_t(__u32, rect.top,
				ctx->format_out.fmt.pix.height - 1);
		rect.width = min_t(__u32, rect.width,
				(ctx->format_out.fmt.pix.width - rect.left));
		rect.height = min_t(__u32, rect.height,
				(ctx->format_out.fmt.pix.height - rect.top));

		item.src_rect.x = rect.left;
		item.src_rect.y = rect.top;
		item.src_rect.w = rect.width;
		item.src_rect.h = rect.height;

		sde_rotator_validate_item(ctx, &item);

		SDEDEV_DBG(rot_dev->dev,
			"s_crop s:%d t:%d (%u,%u,%u,%u)->(%u,%u,%u,%u)\n",
			ctx->session_id, crop->type,
			crop->c.left, crop->c.top,
			crop->c.width, crop->c.height,
			item.src_rect.x, item.src_rect.y,
			item.src_rect.w, item.src_rect.h);

		ctx->crop_out.left = item.src_rect.x;
		ctx->crop_out.top = item.src_rect.y;
		ctx->crop_out.width = item.src_rect.w;
		ctx->crop_out.height = item.src_rect.h;
	} else if (crop->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		rect.left = min_t(__u32, rect.left,
				ctx->format_cap.fmt.pix.width - 1);
		rect.top = min_t(__u32, rect.top,
				ctx->format_cap.fmt.pix.height - 1);
		rect.width = min_t(__u32, rect.width,
				(ctx->format_cap.fmt.pix.width - rect.left));
		rect.height = min_t(__u32, rect.height,
				(ctx->format_cap.fmt.pix.height - rect.top));

		item.dst_rect.x = rect.left;
		item.dst_rect.y = rect.top;
		item.dst_rect.w = rect.width;
		item.dst_rect.h = rect.height;

		sde_rotator_validate_item(ctx, &item);

		SDEDEV_DBG(rot_dev->dev,
			"s_crop s:%d t:%d (%u,%u,%u,%u)->(%u,%u,%u,%u)\n",
			ctx->session_id, crop->type,
			crop->c.left, crop->c.top,
			crop->c.width, crop->c.height,
			item.dst_rect.x, item.dst_rect.y,
			item.dst_rect.w, item.dst_rect.h);

		ctx->crop_cap.left = item.dst_rect.x;
		ctx->crop_cap.top = item.dst_rect.y;
		ctx->crop_cap.width = item.dst_rect.w;
		ctx->crop_cap.height = item.dst_rect.h;
	} else {
		return -EINVAL;
	}

	return 0;
}

/*
 * sde_rotator_g_parm - V4l2 ioctl get parm.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @a: Pointer to v4l2_streamparm struct need to be filled.
 */
static int sde_rotator_g_parm(struct file *file, void *fh,
	struct v4l2_streamparm *a)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	/* Get param is supported only for input buffers */
	if (a->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	a->parm.output.capability = 0;
	a->parm.output.extendedmode = 0;
	a->parm.output.outputmode = 0;
	a->parm.output.writebuffers = 0;
	a->parm.output.timeperframe = ctx->timeperframe;

	return 0;
}

/*
 * sde_rotator_s_parm - V4l2 ioctl set parm.
 * @file: Pointer to file struct.
 * @fh: V4l2 File handle.
 * @a: Pointer to v4l2_streamparm struct need to be set.
 */
static int sde_rotator_s_parm(struct file *file, void *fh,
	struct v4l2_streamparm *a)
{
	struct sde_rotator_ctx *ctx = sde_rotator_ctx_from_fh(fh);

	/* Set param is supported only for input buffers */
	if (a->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	if (!a->parm.output.timeperframe.numerator ||
	    !a->parm.output.timeperframe.denominator)
		return -EINVAL;

	ctx->timeperframe = a->parm.output.timeperframe;
	return 0;
}

/*
 * sde_rotator_private_ioctl - V4l2 private ioctl handler.
 * @file: Pointer to file struct.
 * @fd: V4l2 device file handle.
 * @valid_prio: Priority ioctl valid flag.
 * @cmd: Ioctl command.
 * @arg: Ioctl argument.
 */
static long sde_rotator_private_ioctl(struct file *file, void *fh,
	bool valid_prio, unsigned int cmd, void *arg)
{
	struct sde_rotator_ctx *ctx =
			sde_rotator_ctx_from_fh(file->private_data);
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct msm_sde_rotator_fence *fence = arg;
	struct msm_sde_rotator_comp_ratio *comp_ratio = arg;
	struct sde_rotator_vbinfo *vbinfo;
	int ret;

	switch (cmd) {
	case VIDIOC_S_SDE_ROTATOR_FENCE:
		if (!fence)
			return -EINVAL;

		if (fence->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
			return -EINVAL;

		if (fence->index >= ctx->nbuf_out)
			return -EINVAL;

		SDEDEV_DBG(rot_dev->dev,
				"VIDIOC_S_SDE_ROTATOR_FENCE s:%d i:%d fd:%d\n",
				ctx->session_id, fence->index,
				fence->fd);

		vbinfo = &ctx->vbinfo_out[fence->index];

		if (vbinfo->fd >= 0) {
			if (vbinfo->fence) {
				SDEDEV_DBG(rot_dev->dev,
						"put fence s:%d t:%d i:%d\n",
						ctx->session_id,
						fence->type, fence->index);
				sde_rotator_put_sync_fence(vbinfo->fence);
			}
			vbinfo->fence = NULL;
			vbinfo->fd = -1;
		}

		vbinfo->fd = fence->fd;
		if (vbinfo->fd >= 0) {
			vbinfo->fence =
				sde_rotator_get_fd_sync_fence(vbinfo->fd);
			if (!vbinfo->fence) {
				SDEDEV_WARN(rot_dev->dev,
					"invalid input fence fd s:%d fd:%d\n",
					ctx->session_id, vbinfo->fd);
				vbinfo->fd = -1;
				return -EINVAL;
			}
		} else {
			vbinfo->fence = NULL;
		}
		break;
	case VIDIOC_G_SDE_ROTATOR_FENCE:
		if (!fence)
			return -EINVAL;

		if (fence->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
			return -EINVAL;

		if (fence->index >= ctx->nbuf_cap)
			return -EINVAL;

		vbinfo = &ctx->vbinfo_cap[fence->index];

		if (!vbinfo)
			return -EINVAL;

		if (vbinfo->fence) {
			ret = sde_rotator_get_sync_fence_fd(vbinfo->fence);
			if (ret < 0) {
				SDEDEV_ERR(rot_dev->dev,
						"fail get fence fd s:%d\n",
						ctx->session_id);
				return ret;
			}

			/**
			 * Cache fence descriptor in case user calls this
			 * ioctl multiple times. Cached value would be stale
			 * if user duplicated and closed old descriptor.
			 */
			vbinfo->fd = ret;
		} else if (!sde_rotator_get_fd_sync_fence(vbinfo->fd)) {
			/**
			 * User has closed cached fence descriptor.
			 * Invalidate descriptor cache.
			 */
			vbinfo->fd = -1;
		}
		fence->fd = vbinfo->fd;

		SDEDEV_DBG(rot_dev->dev,
				"VIDIOC_G_SDE_ROTATOR_FENCE s:%d i:%d fd:%d\n",
				ctx->session_id, fence->index,
				fence->fd);
		break;
	case VIDIOC_S_SDE_ROTATOR_COMP_RATIO:
		if (!comp_ratio)
			return -EINVAL;
		else if (!comp_ratio->numer || !comp_ratio->denom)
			return -EINVAL;
		else if (comp_ratio->type == V4L2_BUF_TYPE_VIDEO_OUTPUT &&
				comp_ratio->index < ctx->nbuf_out)
			vbinfo = &ctx->vbinfo_out[comp_ratio->index];
		else if (comp_ratio->type == V4L2_BUF_TYPE_VIDEO_CAPTURE &&
				comp_ratio->index < ctx->nbuf_cap)
			vbinfo = &ctx->vbinfo_cap[comp_ratio->index];
		else
			return -EINVAL;

		vbinfo->comp_ratio.numer = comp_ratio->numer;
		vbinfo->comp_ratio.denom = comp_ratio->denom;

		SDEDEV_DBG(rot_dev->dev,
				"VIDIOC_S_SDE_ROTATOR_COMP_RATIO s:%d i:%d t:%d cr:%u/%u\n",
				ctx->session_id, comp_ratio->index,
				comp_ratio->type,
				vbinfo->comp_ratio.numer,
				vbinfo->comp_ratio.denom);
		break;
	case VIDIOC_G_SDE_ROTATOR_COMP_RATIO:
		if (!comp_ratio)
			return -EINVAL;
		else if (comp_ratio->type == V4L2_BUF_TYPE_VIDEO_OUTPUT &&
				comp_ratio->index < ctx->nbuf_out)
			vbinfo = &ctx->vbinfo_out[comp_ratio->index];
		else if (comp_ratio->type == V4L2_BUF_TYPE_VIDEO_CAPTURE &&
				comp_ratio->index < ctx->nbuf_cap)
			vbinfo = &ctx->vbinfo_cap[comp_ratio->index];
		else
			return -EINVAL;

		comp_ratio->numer = vbinfo->comp_ratio.numer;
		comp_ratio->denom = vbinfo->comp_ratio.denom;

		SDEDEV_DBG(rot_dev->dev,
				"VIDIOC_G_SDE_ROTATOR_COMP_RATIO s:%d i:%d t:%d cr:%u/%u\n",
				ctx->session_id, comp_ratio->index,
				comp_ratio->type,
				comp_ratio->numer,
				comp_ratio->denom);
		break;
	default:
		SDEDEV_WARN(rot_dev->dev, "invalid ioctl type %x\n", cmd);
		return -ENOTTY;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
/*
 * sde_rotator_compat_ioctl32 - Compat ioctl handler function.
 * @file: Pointer to file struct.
 * @cmd: Ioctl command.
 * @arg: Ioctl argument.
 */
static long sde_rotator_compat_ioctl32(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct video_device *vdev = video_devdata(file);
	struct sde_rotator_ctx *ctx =
			sde_rotator_ctx_from_fh(file->private_data);
	long ret;

	mutex_lock(vdev->lock);

	switch (cmd) {
	case VIDIOC_S_SDE_ROTATOR_FENCE:
	case VIDIOC_G_SDE_ROTATOR_FENCE:
	{
		struct msm_sde_rotator_fence fence;

		if (copy_from_user(&fence, (void __user *)arg,
				sizeof(struct msm_sde_rotator_fence)))
			goto ioctl32_error;

		ret = sde_rotator_private_ioctl(file, file->private_data,
			0, cmd, (void *)&fence);

		if (copy_to_user((void __user *)arg, &fence,
				sizeof(struct msm_sde_rotator_fence)))
			goto ioctl32_error;

		break;
	}
	case VIDIOC_S_SDE_ROTATOR_COMP_RATIO:
	case VIDIOC_G_SDE_ROTATOR_COMP_RATIO:
	{
		struct msm_sde_rotator_comp_ratio comp_ratio;

		if (copy_from_user(&comp_ratio, (void __user *)arg,
				sizeof(struct msm_sde_rotator_comp_ratio)))
			goto ioctl32_error;

		ret = sde_rotator_private_ioctl(file, file->private_data,
			0, cmd, (void *)&comp_ratio);

		if (copy_to_user((void __user *)arg, &comp_ratio,
				sizeof(struct msm_sde_rotator_comp_ratio)))
			goto ioctl32_error;

		break;
	}
	default:
		SDEDEV_ERR(ctx->rot_dev->dev, "invalid ioctl32 type:%x\n", cmd);
		ret = -ENOIOCTLCMD;
		break;

	}

	mutex_unlock(vdev->lock);
	return ret;

ioctl32_error:
	mutex_unlock(vdev->lock);
	SDEDEV_ERR(ctx->rot_dev->dev, "error handling ioctl32 cmd:%x\n", cmd);
	return -EFAULT;
}
#endif

static int sde_rotator_ctrl_subscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	return -EINVAL;
}

static int sde_rotator_event_unsubscribe(struct v4l2_fh *fh,
			   const struct v4l2_event_subscription *sub)
{
	return -EINVAL;
}

/* V4l2 ioctl handlers */
static const struct v4l2_ioctl_ops sde_rotator_ioctl_ops = {
	.vidioc_querycap          = sde_rotator_querycap,
	.vidioc_enum_fmt_vid_out  = sde_rotator_enum_fmt_vid_out,
	.vidioc_enum_fmt_vid_cap  = sde_rotator_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_out     = sde_rotator_g_fmt_out,
	.vidioc_g_fmt_vid_cap     = sde_rotator_g_fmt_cap,
	.vidioc_try_fmt_vid_out   = sde_rotator_try_fmt_vid_out,
	.vidioc_try_fmt_vid_cap   = sde_rotator_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_out     = sde_rotator_s_fmt_vid_out,
	.vidioc_s_fmt_vid_cap     = sde_rotator_s_fmt_vid_cap,
	.vidioc_reqbufs           = sde_rotator_reqbufs,
	.vidioc_qbuf              = sde_rotator_qbuf,
	.vidioc_dqbuf             = sde_rotator_dqbuf,
	.vidioc_querybuf          = sde_rotator_querybuf,
	.vidioc_streamon          = sde_rotator_streamon,
	.vidioc_streamoff         = sde_rotator_streamoff,
	.vidioc_cropcap           = sde_rotator_cropcap,
	.vidioc_g_crop            = sde_rotator_g_crop,
	.vidioc_s_crop            = sde_rotator_s_crop,
	.vidioc_g_parm            = sde_rotator_g_parm,
	.vidioc_s_parm            = sde_rotator_s_parm,
	.vidioc_default           = sde_rotator_private_ioctl,
	.vidioc_log_status        = v4l2_ctrl_log_status,
	.vidioc_subscribe_event   = sde_rotator_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = sde_rotator_event_unsubscribe,
};

/*
 * sde_rotator_retire_handler - Invoked by hal when processing is done.
 * @work: Pointer to work structure.
 *
 * This function is scheduled in work queue context.
 */
static void sde_rotator_retire_handler(struct kthread_work *work)
{
	struct vb2_v4l2_buffer *src_buf;
	struct vb2_v4l2_buffer *dst_buf;
	struct sde_rotator_ctx *ctx;
	struct sde_rotator_device *rot_dev;
	struct sde_rotator_request *request;

	request = container_of(work, struct sde_rotator_request, retire_work);
	ctx = request->ctx;

	if (!ctx || !ctx->rot_dev) {
		SDEROT_ERR("null context/device\n");
		return;
	}

	rot_dev = ctx->rot_dev;

	SDEDEV_DBG(rot_dev->dev, "retire handler s:%d\n", ctx->session_id);

	mutex_lock(&rot_dev->lock);
	if (ctx->abort_pending) {
		SDEDEV_DBG(rot_dev->dev, "abort command in retire s:%d\n",
				ctx->session_id);
		sde_rotator_update_retire_sequence(request);
		sde_rotator_retire_request(request);
		mutex_unlock(&rot_dev->lock);
		return;
	}

	if (!ctx->file) {
		sde_rotator_update_retire_sequence(request);
	} else if (rot_dev->early_submit) {
		if (IS_ERR_OR_NULL(request->req)) {
			/* fail pending request or something wrong */
			SDEDEV_ERR(rot_dev->dev,
					"pending request fail in retire s:%d\n",
					ctx->session_id);
		}

		/* pending request. reschedule this context. */
		v4l2_m2m_try_schedule(ctx->fh.m2m_ctx);
	} else {
		/* no pending request. acknowledge the usual way. */
		src_buf = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx);
		dst_buf = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx);

		if (!src_buf || !dst_buf) {
			SDEDEV_ERR(rot_dev->dev,
				"null buffer in retire s:%d sb:%p db:%p\n",
				ctx->session_id,
				src_buf, dst_buf);
		}

		sde_rotator_update_retire_sequence(request);
		sde_rotator_retire_request(request);
		v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_DONE);
		v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_DONE);
		v4l2_m2m_job_finish(rot_dev->m2m_dev, ctx->fh.m2m_ctx);
	}
	mutex_unlock(&rot_dev->lock);
}

/*
 * sde_rotator_process_buffers - Start rotator processing.
 * @ctx: Pointer rotator context.
 * @src_buf: Pointer to Vb2 source buffer.
 * @dst_buf: Pointer to Vb2 destination buffer.
 * @request: Pointer to rotator request
 */
static int sde_rotator_process_buffers(struct sde_rotator_ctx *ctx,
	struct vb2_buffer *src_buf, struct vb2_buffer *dst_buf,
	struct sde_rotator_request *request)
{
	struct sde_rotator_device *rot_dev = ctx->rot_dev;
	struct sde_rotation_item item;
	struct sde_rot_entry_container *req = NULL;
	struct sde_rotator_buf_handle *src_handle;
	struct sde_rotator_buf_handle *dst_handle;
	struct sde_rotator_statistics *stats = &rot_dev->stats;
	struct sde_rotator_vbinfo *vbinfo_out;
	struct sde_rotator_vbinfo *vbinfo_cap;
	ktime_t *ts;
	int ret;

	if (!src_buf || !dst_buf) {
		SDEDEV_ERR(rot_dev->dev, "null vb2 buffers\n");
		ret = -EINVAL;
		goto error_null_buffer;
	}

	src_handle = src_buf->planes[0].mem_priv;
	dst_handle = dst_buf->planes[0].mem_priv;

	if (!src_handle || !dst_handle) {
		SDEDEV_ERR(rot_dev->dev, "null buffer handle\n");
		ret = -EINVAL;
		goto error_null_buffer;
	}

	vbinfo_out = &ctx->vbinfo_out[src_buf->index];
	vbinfo_cap = &ctx->vbinfo_cap[dst_buf->index];

	SDEDEV_DBG(rot_dev->dev,
		"process buffer s:%d.%u src:(%u,%u,%u,%u) dst:(%u,%u,%u,%u) rot:%d flip:%d/%d sec:%d src_cr:%u/%u dst_cr:%u/%u\n",
		ctx->session_id, vbinfo_cap->fence_ts,
		ctx->crop_out.left, ctx->crop_out.top,
		ctx->crop_out.width, ctx->crop_out.height,
		ctx->crop_cap.left, ctx->crop_cap.top,
		ctx->crop_cap.width, ctx->crop_cap.height,
		ctx->rotate, ctx->hflip, ctx->vflip, ctx->secure,
		vbinfo_out->comp_ratio.numer, vbinfo_out->comp_ratio.denom,
		vbinfo_cap->comp_ratio.numer, vbinfo_cap->comp_ratio.denom);

	/* allocate slot for timestamp */
	ts = stats->ts[stats->count++ % SDE_ROTATOR_NUM_EVENTS];
	ts[SDE_ROTATOR_TS_SRCQB] = vbinfo_out->qbuf_ts;
	ts[SDE_ROTATOR_TS_DSTQB] = vbinfo_cap->qbuf_ts;
	vbinfo_out->dqbuf_ts = &ts[SDE_ROTATOR_TS_SRCDQB];
	vbinfo_cap->dqbuf_ts = &ts[SDE_ROTATOR_TS_DSTDQB];

	ts[SDE_ROTATOR_TS_FENCE] = ktime_get();

	trace_rot_entry_fence(
		ctx->session_id, vbinfo_cap->fence_ts,
		ctx->fh.prio,
		(ctx->rotate << 0) | (ctx->hflip << 8) |
			(ctx->hflip << 9) | (ctx->secure << 10),
		ctx->format_out.fmt.pix.pixelformat,
		ctx->format_out.fmt.pix.width,
		ctx->format_out.fmt.pix.height,
		ctx->crop_out.left, ctx->crop_out.top,
		ctx->crop_out.width, ctx->crop_out.height,
		ctx->format_cap.fmt.pix.pixelformat,
		ctx->format_cap.fmt.pix.width,
		ctx->format_cap.fmt.pix.height,
		ctx->crop_cap.left, ctx->crop_cap.top,
		ctx->crop_cap.width, ctx->crop_cap.height);

	if (vbinfo_out->fence) {
		sde_rot_mgr_unlock(rot_dev->mgr);
		mutex_unlock(&rot_dev->lock);
		SDEDEV_DBG(rot_dev->dev, "fence enter s:%d.%d fd:%d\n",
			ctx->session_id, vbinfo_cap->fence_ts, vbinfo_out->fd);
		ret = sde_rotator_wait_sync_fence(vbinfo_out->fence,
				rot_dev->fence_timeout);
		mutex_lock(&rot_dev->lock);
		sde_rot_mgr_lock(rot_dev->mgr);
		sde_rotator_put_sync_fence(vbinfo_out->fence);
		vbinfo_out->fence = NULL;
		if (ret) {
			SDEDEV_ERR(rot_dev->dev,
				"error waiting for fence s:%d.%d fd:%d r:%d\n",
				ctx->session_id,
				vbinfo_cap->fence_ts, vbinfo_out->fd, ret);
			SDEROT_EVTLOG(ctx->session_id, vbinfo_cap->fence_ts,
					vbinfo_out->fd, ret,
					SDE_ROT_EVTLOG_ERROR);
			goto error_fence_wait;
		} else {
			SDEDEV_DBG(rot_dev->dev, "fence exit s:%d.%d fd:%d\n",
				ctx->session_id,
				vbinfo_cap->fence_ts, vbinfo_out->fd);
		}
	}

	/* fill in item work structure */
	sde_rotator_get_item_from_ctx(ctx, &item);
	item.flags |= SDE_ROTATION_EXT_DMA_BUF;
	item.input.planes[0].buffer = src_handle->buffer;
	item.input.planes[0].handle = src_handle->handle;
	item.input.planes[0].offset = src_handle->addr;
	item.input.planes[0].stride = ctx->format_out.fmt.pix.bytesperline;
	item.input.plane_count = 1;
	item.input.fence = NULL;
	item.input.comp_ratio = vbinfo_out->comp_ratio;
	item.output.planes[0].buffer = dst_handle->buffer;
	item.output.planes[0].handle = dst_handle->handle;
	item.output.planes[0].offset = dst_handle->addr;
	item.output.planes[0].stride = ctx->format_cap.fmt.pix.bytesperline;
	item.output.plane_count = 1;
	item.output.fence = NULL;
	item.output.comp_ratio = vbinfo_cap->comp_ratio;
	item.sequence_id = vbinfo_cap->fence_ts;
	item.ts = ts;

	req = sde_rotator_req_init(rot_dev->mgr, ctx->private, &item, 1, 0);
	if (IS_ERR_OR_NULL(req)) {
		SDEDEV_ERR(rot_dev->dev, "fail allocate rotation request\n");
		ret = -ENOMEM;
		goto error_init_request;
	}

	req->retire_kw = ctx->work_queue.rot_kw;
	req->retire_work = &request->retire_work;

	ret = sde_rotator_handle_request_common(
			rot_dev->mgr, ctx->private, req);
	if (ret) {
		SDEDEV_ERR(rot_dev->dev, "fail handle request\n");
		goto error_handle_request;
	}

	sde_rotator_queue_request(rot_dev->mgr, ctx->private, req);
	request->req = req;
	request->sequence_id = item.sequence_id;
	request->committed = true;

	return 0;
error_handle_request:
	devm_kfree(rot_dev->dev, req);
error_init_request:
error_fence_wait:
error_null_buffer:
	request->req = NULL;
	request->sequence_id = 0;
	request->committed = false;
	return ret;
}

/*
 * sde_rotator_submit_handler - Invoked by m2m to submit job.
 * @work: Pointer to work structure.
 *
 * This function is scheduled in work queue context.
 */
static void sde_rotator_submit_handler(struct kthread_work *work)
{
	struct sde_rotator_ctx *ctx;
	struct sde_rotator_device *rot_dev;
	struct vb2_v4l2_buffer *src_buf;
	struct vb2_v4l2_buffer *dst_buf;
	struct sde_rotator_request *request;
	int ret;

	request = container_of(work, struct sde_rotator_request, submit_work);
	ctx = request->ctx;

	if (!ctx || !ctx->rot_dev) {
		SDEROT_ERR("null device\n");
		return;
	}

	rot_dev = ctx->rot_dev;
	SDEDEV_DBG(rot_dev->dev, "submit handler s:%d\n", ctx->session_id);

	mutex_lock(&rot_dev->lock);
	if (ctx->abort_pending) {
		SDEDEV_DBG(rot_dev->dev, "abort command in submit s:%d\n",
				ctx->session_id);
		sde_rotator_update_retire_sequence(request);
		sde_rotator_retire_request(request);
		mutex_unlock(&rot_dev->lock);
		return;
	}

	/* submit new request */
	dst_buf = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);
	src_buf = v4l2_m2m_next_src_buf(ctx->fh.m2m_ctx);
	sde_rot_mgr_lock(rot_dev->mgr);
	ret = sde_rotator_process_buffers(ctx, &src_buf->vb2_buf,
			&dst_buf->vb2_buf, request);
	sde_rot_mgr_unlock(rot_dev->mgr);
	if (ret) {
		SDEDEV_ERR(rot_dev->dev,
			"fail process buffer in submit s:%d\n",
			ctx->session_id);
		/* advance to device run to clean up buffers */
		v4l2_m2m_try_schedule(ctx->fh.m2m_ctx);
	}

	mutex_unlock(&rot_dev->lock);
}

/*
 * sde_rotator_device_run - rotator m2m device run callback
 * @priv: Pointer rotator context.
 */
static void sde_rotator_device_run(void *priv)
{
	struct sde_rotator_ctx *ctx = priv;
	struct sde_rotator_device *rot_dev;
	struct vb2_v4l2_buffer *src_buf;
	struct vb2_v4l2_buffer *dst_buf;
	struct sde_rotator_request *request;
	int ret;

	if (!ctx || !ctx->rot_dev) {
		SDEROT_ERR("null context/device\n");
		return;
	}

	rot_dev = ctx->rot_dev;
	SDEDEV_DBG(rot_dev->dev, "device run s:%d\n", ctx->session_id);

	if (rot_dev->early_submit) {
		request = list_first_entry_or_null(&ctx->pending_list,
				struct sde_rotator_request, list);

		/* pending request mode, check for completion */
		if (!request || IS_ERR_OR_NULL(request->req)) {
			/* pending request fails or something wrong. */
			SDEDEV_ERR(rot_dev->dev,
				"pending request fail in device run s:%d\n",
				ctx->session_id);
			rot_dev->stats.fail_count++;
			ATRACE_INT("fail_count", rot_dev->stats.fail_count);
			goto error_process_buffers;

		} else if (!atomic_read(&request->req->pending_count)) {
			/* pending request completed. signal done. */
			int failed_count =
				atomic_read(&request->req->failed_count);
			SDEDEV_DBG(rot_dev->dev,
				"pending request completed in device run s:%d\n",
				ctx->session_id);

			/* disconnect request (will be freed by core layer) */
			sde_rot_mgr_lock(rot_dev->mgr);
			sde_rotator_req_finish(rot_dev->mgr, ctx->private,
					request->req);
			sde_rot_mgr_unlock(rot_dev->mgr);

			if (failed_count) {
				SDEDEV_ERR(rot_dev->dev,
					"pending request failed in device run s:%d f:%d\n",
					ctx->session_id,
					failed_count);
				rot_dev->stats.fail_count++;
				ATRACE_INT("fail_count",
						rot_dev->stats.fail_count);
				goto error_process_buffers;
			}

			src_buf = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx);
			dst_buf = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx);
			if (!src_buf || !dst_buf) {
				SDEDEV_ERR(rot_dev->dev,
					"null buffer in device run s:%d sb:%p db:%p\n",
					ctx->session_id,
					src_buf, dst_buf);
				goto error_process_buffers;
			}

			sde_rotator_update_retire_sequence(request);
			sde_rotator_retire_request(request);
			v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_DONE);
			v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_DONE);
			v4l2_m2m_job_finish(rot_dev->m2m_dev, ctx->fh.m2m_ctx);
		} else {
			/* pending request not complete. something wrong. */
			SDEDEV_ERR(rot_dev->dev,
				"Incomplete pending request in device run s:%d\n",
				ctx->session_id);

			/* disconnect request (will be freed by core layer) */
			sde_rot_mgr_lock(rot_dev->mgr);
			sde_rotator_req_finish(rot_dev->mgr, ctx->private,
					request->req);
			sde_rot_mgr_unlock(rot_dev->mgr);

			goto error_process_buffers;
		}
	} else {
		request = list_first_entry_or_null(&ctx->retired_list,
				struct sde_rotator_request, list);
		if (!request) {
			SDEDEV_ERR(rot_dev->dev,
				"no free request in device run s:%d\n",
				ctx->session_id);
			goto error_retired_list;
		}

		spin_lock(&ctx->list_lock);
		list_del_init(&request->list);
		list_add_tail(&request->list, &ctx->pending_list);
		spin_unlock(&ctx->list_lock);

		/* no pending request. submit buffer the usual way. */
		dst_buf = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);
		src_buf = v4l2_m2m_next_src_buf(ctx->fh.m2m_ctx);
		if (!src_buf || !dst_buf) {
			SDEDEV_ERR(rot_dev->dev,
				"null buffer in device run s:%d sb:%pK db:%pK\n",
				ctx->session_id,
				src_buf, dst_buf);
			goto error_empty_buffer;
		}

		sde_rot_mgr_lock(rot_dev->mgr);
		ret = sde_rotator_process_buffers(ctx, &src_buf->vb2_buf,
				&dst_buf->vb2_buf, request);
		sde_rot_mgr_unlock(rot_dev->mgr);
		if (ret) {
			SDEDEV_ERR(rot_dev->dev,
				"fail process buffer in device run s:%d\n",
				ctx->session_id);
			rot_dev->stats.fail_count++;
			ATRACE_INT("fail_count", rot_dev->stats.fail_count);
			goto error_process_buffers;
		}
	}

	return;
error_process_buffers:
error_empty_buffer:
error_retired_list:
	sde_rotator_update_retire_sequence(request);
	sde_rotator_retire_request(request);
	src_buf = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx);
	dst_buf = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx);
	if (src_buf)
		v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_ERROR);
	if (dst_buf)
		v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_ERROR);
	sde_rotator_resync_timeline(ctx->work_queue.timeline);
	v4l2_m2m_job_finish(rot_dev->m2m_dev, ctx->fh.m2m_ctx);
}

/*
 * sde_rotator_job_abort - rotator m2m job abort callback
 * @priv: Pointer rotator context.
 */
static void sde_rotator_job_abort(void *priv)
{
	struct sde_rotator_ctx *ctx = priv;
	struct sde_rotator_device *rot_dev;

	if (!ctx || !ctx->rot_dev) {
		SDEROT_ERR("null context/device\n");
		return;
	}

	rot_dev = ctx->rot_dev;
	SDEDEV_DBG(rot_dev->dev, "job abort s:%d\n", ctx->session_id);

	v4l2_m2m_job_finish(rot_dev->m2m_dev, ctx->fh.m2m_ctx);
}

/*
 * sde_rotator_job_ready - rotator m2m job ready callback
 * @priv: Pointer rotator context.
 */
static int sde_rotator_job_ready(void *priv)
{
	struct sde_rotator_ctx *ctx = priv;
	struct sde_rotator_device *rot_dev;
	struct sde_rotator_request *request;
	int ret = 0;

	if (!ctx || !ctx->rot_dev) {
		SDEROT_ERR("null context/device\n");
		return 0;
	}

	rot_dev = ctx->rot_dev;
	SDEDEV_DBG(rot_dev->dev, "job ready s:%d\n", ctx->session_id);

	request = list_first_entry_or_null(&ctx->pending_list,
			struct sde_rotator_request, list);

	if (!rot_dev->early_submit) {
		/* always ready in normal mode. */
		ret = 1;
	} else if (request && IS_ERR_OR_NULL(request->req)) {
		/* if pending request fails, forward to device run state. */
		SDEDEV_DBG(rot_dev->dev,
				"pending request fail in job ready s:%d\n",
				ctx->session_id);
		ret = 1;
	} else if (list_empty(&ctx->pending_list)) {
		/* if no pending request, submit a new request. */
		SDEDEV_DBG(rot_dev->dev,
				"submit job s:%d sc:%d dc:%d p:%d\n",
				ctx->session_id,
				v4l2_m2m_num_src_bufs_ready(ctx->fh.m2m_ctx),
				v4l2_m2m_num_dst_bufs_ready(ctx->fh.m2m_ctx),
				!list_empty(&ctx->pending_list));

		request = list_first_entry_or_null(&ctx->retired_list,
				struct sde_rotator_request, list);
		if (!request) {
			/* should not happen */
			SDEDEV_ERR(rot_dev->dev,
					"no free request in job ready s:%d\n",
					ctx->session_id);
		} else {
			spin_lock(&ctx->list_lock);
			list_del_init(&request->list);
			list_add_tail(&request->list, &ctx->pending_list);
			spin_unlock(&ctx->list_lock);
			kthread_queue_work(ctx->work_queue.rot_kw,
					&request->submit_work);
		}
	} else if (request && !atomic_read(&request->req->pending_count)) {
		/* if pending request completed, forward to device run state */
		SDEDEV_DBG(rot_dev->dev,
				"pending request completed in job ready s:%d\n",
				ctx->session_id);
		ret = 1;
	}

	return ret;
}

/* V4l2 mem2mem handlers */
static struct v4l2_m2m_ops sde_rotator_m2m_ops = {
	.device_run	= sde_rotator_device_run,
	.job_abort	= sde_rotator_job_abort,
	.job_ready	= sde_rotator_job_ready,
};

/* Device tree match struct */
static const struct of_device_id sde_rotator_dt_match[] = {
	{
		.compatible = "qcom,sde_rotator",
		.data = NULL,
	},
	{}
};

/*
 * sde_rotator_get_drv_data - rotator device driver data.
 * @dev: Pointer to device.
 */
static const void *sde_rotator_get_drv_data(struct device *dev)
{
	const struct of_device_id *match;

	match = of_match_node(sde_rotator_dt_match, dev->of_node);

	if (match)
		return match->data;

	return NULL;
}

/*
 * sde_rotator_probe - rotator device probe method.
 * @pdev: Pointer to rotator platform device.
 */
static int sde_rotator_probe(struct platform_device *pdev)
{
	struct sde_rotator_device *rot_dev;
	struct video_device *vdev;
	int ret, i;
	char name[32];

	SDEDEV_DBG(&pdev->dev, "SDE v4l2 rotator probed\n");

	/* sde rotator device struct */
	rot_dev = kzalloc(sizeof(struct sde_rotator_device), GFP_KERNEL);
	if (!rot_dev)
		return -ENOMEM;

	mutex_init(&rot_dev->lock);
	rot_dev->early_submit = SDE_ROTATOR_EARLY_SUBMIT;
	rot_dev->fence_timeout = SDE_ROTATOR_FENCE_TIMEOUT;
	rot_dev->streamoff_timeout = SDE_ROTATOR_STREAM_OFF_TIMEOUT;
	rot_dev->min_rot_clk = 0;
	rot_dev->min_bw = 0;
	rot_dev->min_overhead_us = 0;
	rot_dev->drvdata = sde_rotator_get_drv_data(&pdev->dev);
	rot_dev->open_timeout = SDE_ROTATOR_CTX_OPEN_TIMEOUT;
	init_waitqueue_head(&rot_dev->open_wq);

	rot_dev->pdev = pdev;
	rot_dev->dev = &pdev->dev;
	platform_set_drvdata(pdev, rot_dev);

	ret = sde_rotator_base_init(&rot_dev->mdata, pdev, rot_dev->drvdata);
	if (ret < 0) {
		SDEDEV_ERR(&pdev->dev, "fail init base data %d\n", ret);
		goto error_rotator_base_init;
	}

	ret = sde_rotator_core_init(&rot_dev->mgr, pdev);
	if (ret < 0) {
		if (ret == -EPROBE_DEFER)
			SDEDEV_INFO(&pdev->dev, "probe defer for core init\n");
		else
			SDEDEV_ERR(&pdev->dev, "fail init core %d\n", ret);
		goto error_rotator_core_init;
	}

	/* mem2mem device */
	rot_dev->m2m_dev = v4l2_m2m_init(&sde_rotator_m2m_ops);
	if (IS_ERR(rot_dev->m2m_dev)) {
		ret = PTR_ERR(rot_dev->m2m_dev);
		SDEDEV_ERR(&pdev->dev, "fail init mem2mem device %d\n", ret);
		goto error_m2m_init;
	}

	/* v4l2 device */
	ret = v4l2_device_register(&pdev->dev, &rot_dev->v4l2_dev);
	if (ret < 0) {
		SDEDEV_ERR(&pdev->dev, "fail register v4l2 device %d\n", ret);
		goto error_v4l2_register;
	}

	vdev = video_device_alloc();
	if (!vdev) {
		SDEDEV_ERR(&pdev->dev, "fail allocate video device\n");
		goto error_alloc_video_device;
	}

	vdev->fops = &sde_rotator_fops;
	vdev->ioctl_ops = &sde_rotator_ioctl_ops;
	vdev->lock = &rot_dev->lock;
	vdev->minor = -1;
	vdev->release = video_device_release;
	vdev->v4l2_dev = &rot_dev->v4l2_dev;
	vdev->vfl_dir = VFL_DIR_M2M;
	vdev->vfl_type = VFL_TYPE_GRABBER;
	strlcpy(vdev->name, SDE_ROTATOR_DRV_NAME, sizeof(vdev->name));

	ret = video_register_device(vdev, VFL_TYPE_GRABBER,
			SDE_ROTATOR_BASE_DEVICE_NUMBER);
	if (ret < 0) {
		SDEDEV_ERR(&pdev->dev, "fail register video device %d\n",
				ret);
		goto error_video_register;
	}

	rot_dev->vdev = vdev;
	video_set_drvdata(rot_dev->vdev, rot_dev);

	rot_dev->debugfs_root = sde_rotator_create_debugfs(rot_dev);

	for (i = 0; i < MAX_ROT_OPEN_SESSION; i++) {
		snprintf(name, sizeof(name), "rot_fenceq_%d_%d",
			rot_dev->dev->id, i);
		kthread_init_worker(&rot_dev->rot_kw[i]);
		rot_dev->rot_thread[i] = kthread_run(kthread_worker_fn,
			&rot_dev->rot_kw[i], name);
		if (IS_ERR(rot_dev->rot_thread[i])) {
			SDEDEV_ERR(rot_dev->dev,
				"fail allocate kthread i:%d\n", i);
			ret = -EPERM;
			goto error_kthread_create;
		}
		rot_dev->kthread_free[i] = true;
	}

	SDEDEV_INFO(&pdev->dev, "SDE v4l2 rotator probe success\n");

	return 0;
error_kthread_create:
	for (i--; i >= 0; i--)
		kthread_stop(rot_dev->rot_thread[i]);
	sde_rotator_destroy_debugfs(rot_dev->debugfs_root);
	video_unregister_device(rot_dev->vdev);
error_video_register:
	video_device_release(vdev);
error_alloc_video_device:
	v4l2_device_unregister(&rot_dev->v4l2_dev);
error_v4l2_register:
	v4l2_m2m_release(rot_dev->m2m_dev);
error_m2m_init:
	sde_rotator_core_destroy(rot_dev->mgr);
error_rotator_core_init:
	sde_rotator_base_destroy(rot_dev->mdata);
error_rotator_base_init:
	kfree(rot_dev);
	return ret;
}

/*
 * sde_rotator_remove - rotator device remove method.
 * @pdev: Pointer rotator platform device.
 */
static int sde_rotator_remove(struct platform_device *pdev)
{
	struct sde_rotator_device *rot_dev;
	int i;

	rot_dev = platform_get_drvdata(pdev);
	if (rot_dev == NULL) {
		SDEDEV_ERR(&pdev->dev, "fail get rotator drvdata\n");
		return 0;
	}

	sde_rotator_pm_qos_remove(rot_dev->mdata);
	for (i = MAX_ROT_OPEN_SESSION - 1; i >= 0; i--)
		kthread_stop(rot_dev->rot_thread[i]);
	sde_rotator_destroy_debugfs(rot_dev->debugfs_root);
	video_unregister_device(rot_dev->vdev);
	video_device_release(rot_dev->vdev);
	v4l2_device_unregister(&rot_dev->v4l2_dev);
	v4l2_m2m_release(rot_dev->m2m_dev);
	sde_rotator_core_destroy(rot_dev->mgr);
	sde_rotator_base_destroy(rot_dev->mdata);
	kfree(rot_dev);
	return 0;
}

static const struct dev_pm_ops sde_rotator_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sde_rotator_pm_suspend, sde_rotator_pm_resume)
	SET_RUNTIME_PM_OPS(sde_rotator_runtime_suspend,
			sde_rotator_runtime_resume,
			sde_rotator_runtime_idle)
};

/* SDE Rotator platform driver definition */
static struct platform_driver rotator_driver = {
	.probe = sde_rotator_probe,
	.remove = sde_rotator_remove,
	.suspend = sde_rotator_suspend,
	.resume = sde_rotator_resume,
	.driver = {
		.name = SDE_ROTATOR_DRV_NAME,
		.of_match_table = sde_rotator_dt_match,
		.pm = &sde_rotator_pm_ops,
		.suppress_bind_attrs = true,
	},
};

static int __init sde_rotator_init_module(void)
{
	return platform_driver_register(&rotator_driver);
}

static void __exit sde_rotator_exit_module(void)
{
	platform_driver_unregister(&rotator_driver);
}

module_init(sde_rotator_init_module);
module_exit(sde_rotator_exit_module);
MODULE_DESCRIPTION("MSM SDE ROTATOR driver");
