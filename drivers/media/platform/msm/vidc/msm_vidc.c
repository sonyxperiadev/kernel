/* Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
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

#include <linux/dma-direction.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/bsearch.h>
#include <linux/delay.h>
#include <media/msm_vidc.h>
#include "msm_vidc_internal.h"
#include "msm_vidc_debug.h"
#include "msm_vdec.h"
#include "msm_venc.h"
#include "msm_vidc_common.h"
#include "vidc_hfi_api.h"
#include "msm_vidc_dcvs.h"

#define MAX_EVENTS 30

static int get_poll_flags(void *instance)
{
	struct msm_vidc_inst *inst = instance;
	struct vb2_queue *outq = &inst->bufq[OUTPUT_PORT].vb2_bufq;
	struct vb2_queue *capq = &inst->bufq[CAPTURE_PORT].vb2_bufq;
	struct vb2_buffer *out_vb = NULL;
	struct vb2_buffer *cap_vb = NULL;
	unsigned long flags;
	int rc = 0;

	if (v4l2_event_pending(&inst->event_handler))
		rc |= POLLPRI;

	spin_lock_irqsave(&capq->done_lock, flags);
	if (!list_empty(&capq->done_list))
		cap_vb = list_first_entry(&capq->done_list, struct vb2_buffer,
								done_entry);
	if (cap_vb && (cap_vb->state == VB2_BUF_STATE_DONE
				|| cap_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLIN | POLLRDNORM;
	spin_unlock_irqrestore(&capq->done_lock, flags);

	spin_lock_irqsave(&outq->done_lock, flags);
	if (!list_empty(&outq->done_list))
		out_vb = list_first_entry(&outq->done_list, struct vb2_buffer,
								done_entry);
	if (out_vb && (out_vb->state == VB2_BUF_STATE_DONE
				|| out_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLOUT | POLLWRNORM;
	spin_unlock_irqrestore(&outq->done_lock, flags);

	return rc;
}

int msm_vidc_poll(void *instance, struct file *filp,
		struct poll_table_struct *wait)
{
	struct msm_vidc_inst *inst = instance;
	struct vb2_queue *outq = NULL;
	struct vb2_queue *capq = NULL;

	if (!inst)
		return -EINVAL;

	outq = &inst->bufq[OUTPUT_PORT].vb2_bufq;
	capq = &inst->bufq[CAPTURE_PORT].vb2_bufq;

	poll_wait(filp, &inst->event_handler.wait, wait);
	poll_wait(filp, &capq->done_wq, wait);
	poll_wait(filp, &outq->done_wq, wait);
	return get_poll_flags(inst);
}
EXPORT_SYMBOL(msm_vidc_poll);

int msm_vidc_querycap(void *instance, struct v4l2_capability *cap)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !cap)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_querycap(inst, cap);
	else if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_querycap(instance, cap);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_querycap);

int msm_vidc_enum_fmt(void *instance, struct v4l2_fmtdesc *f)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_enum_fmt(inst, f);
	else if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_enum_fmt(instance, f);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_enum_fmt);

int msm_vidc_query_ctrl(void *instance, struct v4l2_queryctrl *ctrl)
{
	struct msm_vidc_inst *inst = instance;
	int rc = 0;

	if (!inst || !ctrl)
		return -EINVAL;

	switch (ctrl->id) {
	case V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE:
		ctrl->maximum = inst->capability.hier_p_hybrid.max;
		ctrl->minimum = inst->capability.hier_p_hybrid.min;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HIER_B_NUM_LAYERS:
		ctrl->maximum = inst->capability.hier_b.max;
		ctrl->minimum = inst->capability.hier_b.min;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS:
		ctrl->maximum = inst->capability.hier_p.max;
		ctrl->minimum = inst->capability.hier_p.min;
		break;
	default:
		rc = -EINVAL;
	}
	return rc;
}
EXPORT_SYMBOL(msm_vidc_query_ctrl);

static int msm_vidc_queryctrl_bsearch_cmp1(const void *key, const void *elt)
{
	return *(int32_t *)key - (int32_t)((struct msm_vidc_ctrl *)elt)->id;
}

static int msm_vidc_queryctrl_bsearch_cmp2(const void *key, const void *elt)
{
	uint32_t id = *(uint32_t *)key;
	struct msm_vidc_ctrl *ctrl = (struct msm_vidc_ctrl *)elt;

	if (id >= ctrl[0].id && id < ctrl[1].id)
		return 0;
	else if (id < ctrl[0].id)
		return -1;
	else
		return 1;
}

int msm_vidc_query_ext_ctrl(void *instance, struct v4l2_query_ext_ctrl *ctrl)
{
	struct msm_vidc_inst *inst = instance;
	bool get_next_ctrl = 0;
	int i, num_ctrls, rc = 0;
	struct msm_vidc_ctrl *key = NULL;
	struct msm_vidc_ctrl *msm_vdec_ctrls;

	if (!inst || !ctrl)
		return -EINVAL;

	i = ctrl->id;
	memset(ctrl, 0, sizeof(struct v4l2_query_ext_ctrl));
	ctrl->id = i;

	if (ctrl->id & V4L2_CTRL_FLAG_NEXT_CTRL)
		get_next_ctrl = 1;
	else if (ctrl->id & V4L2_CTRL_FLAG_NEXT_COMPOUND)
		goto query_ext_ctrl_err;

	ctrl->id &= ~V4L2_CTRL_FLAG_NEXT_CTRL;
	ctrl->id &= ~V4L2_CTRL_FLAG_NEXT_COMPOUND;

	if (ctrl->id > V4L2_CID_PRIVATE_BASE ||
		(ctrl->id >= V4L2_CID_BASE && ctrl->id <= V4L2_CID_LASTP1))
		goto query_ext_ctrl_err;
	else if (ctrl->id == V4L2_CID_PRIVATE_BASE && get_next_ctrl)
		ctrl->id = V4L2_CID_MPEG_MSM_VIDC_BASE;

	if (inst->session_type == MSM_VIDC_DECODER)
		msm_vdec_g_ctrl(&msm_vdec_ctrls, &num_ctrls);
	else
		return -EINVAL;

	if (!get_next_ctrl)
		key = bsearch(&ctrl->id, msm_vdec_ctrls, num_ctrls,
					sizeof(struct msm_vidc_ctrl),
					msm_vidc_queryctrl_bsearch_cmp1);
	else {
		key = bsearch(&ctrl->id, msm_vdec_ctrls, num_ctrls-1,
					sizeof(struct msm_vidc_ctrl),
					msm_vidc_queryctrl_bsearch_cmp2);

		if (key && ctrl->id > key->id)
			key++;
		if (key) {
			for (i = key-msm_vdec_ctrls, key = NULL;
				i < num_ctrls; i++)
				if (!(msm_vdec_ctrls[i].flags &
					V4L2_CTRL_FLAG_DISABLED)) {
					key = &msm_vdec_ctrls[i];
					break;
				}
		}
	}

	if (key) {
		ctrl->id = key->id;
		ctrl->type = key->type;
		strlcpy(ctrl->name, key->name, MAX_NAME_LENGTH);
		ctrl->minimum = key->minimum;
		ctrl->maximum = key->maximum;
		ctrl->step = key->step;
		ctrl->default_value = key->default_value;
		ctrl->flags = key->flags;
		ctrl->elems = 1;
		ctrl->nr_of_dims = 0;
		return rc;
	}

query_ext_ctrl_err:
	ctrl->name[0] = '\0';
	ctrl->flags |= V4L2_CTRL_FLAG_DISABLED;
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_query_ext_ctrl);

int msm_vidc_s_fmt(void *instance, struct v4l2_format *f)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_s_fmt(inst, f);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_s_fmt(instance, f);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_s_fmt);

int msm_vidc_g_fmt(void *instance, struct v4l2_format *f)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_g_fmt(inst, f);
	else if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_g_fmt(instance, f);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_g_fmt);

int msm_vidc_s_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !control)
		return -EINVAL;

	return msm_comm_s_ctrl(instance, control);
}
EXPORT_SYMBOL(msm_vidc_s_ctrl);

int msm_vidc_g_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !control)
		return -EINVAL;

	return msm_comm_g_ctrl(instance, control);
}
EXPORT_SYMBOL(msm_vidc_g_ctrl);

int msm_vidc_s_ext_ctrl(void *instance, struct v4l2_ext_controls *control)
{
	struct msm_vidc_inst *inst = instance;
	if (!inst || !control)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_s_ext_ctrl(inst, control);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_s_ext_ctrl(instance, control);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_s_ext_ctrl);

int msm_vidc_reqbufs(void *instance, struct v4l2_requestbuffers *b)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !b)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_reqbufs(inst, b);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_reqbufs(instance, b);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_reqbufs);

struct buffer_info *get_registered_buf(struct msm_vidc_inst *inst,
		struct v4l2_buffer *b, int idx, int *plane)
{
	struct buffer_info *temp;
	struct buffer_info *ret = NULL;
	int i;
	int fd = b->m.planes[idx].reserved[0];
	u32 buff_off = b->m.planes[idx].reserved[1];
	u32 size = b->m.planes[idx].length;
	ion_phys_addr_t device_addr = b->m.planes[idx].m.userptr;

	if (fd < 0 || !plane) {
		dprintk(VIDC_ERR, "Invalid input\n");
		goto err_invalid_input;
	}

	WARN(!mutex_is_locked(&inst->registeredbufs.lock),
		"Registered buf lock is not acquired for %s", __func__);

	*plane = 0;
	list_for_each_entry(temp, &inst->registeredbufs.list, list) {
		for (i = 0; i < min(temp->num_planes, VIDEO_MAX_PLANES); i++) {
			bool ion_hndl_matches = temp->handle[i] ?
				msm_smem_compare_buffers(inst->mem_client, fd,
				temp->handle[i]->smem_priv) : false;
			bool device_addr_matches = device_addr ==
						temp->device_addr[i];
			bool contains_within = CONTAINS(temp->buff_off[i],
					temp->size[i], buff_off) ||
				CONTAINS(buff_off, size, temp->buff_off[i]);
			bool overlaps = OVERLAPS(buff_off, size,
					temp->buff_off[i], temp->size[i]);

			if (!temp->inactive &&
				(ion_hndl_matches || device_addr_matches) &&
				(contains_within || overlaps)) {
				dprintk(VIDC_DBG,
						"This memory region is already mapped\n");
				ret = temp;
				*plane = i;
				break;
			}
		}
		if (ret)
			break;
	}

err_invalid_input:
	return ret;
}

static struct msm_smem *get_same_fd_buffer(struct msm_vidc_inst *inst, int fd)
{
	struct buffer_info *temp;
	struct msm_smem *same_fd_handle = NULL;

	int i;

	if (!fd)
		return NULL;

	if (!inst || fd < 0) {
		dprintk(VIDC_ERR, "%s: Invalid input\n", __func__);
		goto err_invalid_input;
	}

	mutex_lock(&inst->registeredbufs.lock);
	list_for_each_entry(temp, &inst->registeredbufs.list, list) {
		for (i = 0; i < min(temp->num_planes, VIDEO_MAX_PLANES); i++) {
			bool ion_hndl_matches = temp->handle[i] ?
				msm_smem_compare_buffers(inst->mem_client, fd,
				temp->handle[i]->smem_priv) : false;
			if (ion_hndl_matches && temp->mapped[i])  {
				temp->same_fd_ref[i]++;
				dprintk(VIDC_INFO,
				"Found same fd buffer\n");
				same_fd_handle = temp->handle[i];
				break;
			}
		}
		if (same_fd_handle)
			break;
	}
	mutex_unlock(&inst->registeredbufs.lock);

err_invalid_input:
	return same_fd_handle;
}

struct buffer_info *device_to_uvaddr(struct msm_vidc_list *buf_list,
				ion_phys_addr_t device_addr)
{
	struct buffer_info *temp = NULL;
	bool found = false;
	int i;

	if (!buf_list || !device_addr) {
		dprintk(VIDC_ERR,
			"Invalid input- device_addr: %pa buf_list: %pK\n",
			&device_addr, buf_list);
		goto err_invalid_input;
	}

	mutex_lock(&buf_list->lock);
	list_for_each_entry(temp, &buf_list->list, list) {
		for (i = 0; i < min(temp->num_planes, VIDEO_MAX_PLANES); i++) {
			if (!temp->inactive &&
				temp->device_addr[i] == device_addr)  {
				dprintk(VIDC_INFO,
				"Found same fd buffer\n");
				found = true;
				break;
			}
		}

		if (found)
			break;
	}
	mutex_unlock(&buf_list->lock);

err_invalid_input:
	return temp;
}

static inline void populate_buf_info(struct buffer_info *binfo,
			struct v4l2_buffer *b, u32 i)
{
	if (i >= VIDEO_MAX_PLANES) {
		dprintk(VIDC_ERR, "%s: Invalid input\n", __func__);
		return;
	}
	binfo->type = b->type;
	binfo->fd[i] = b->m.planes[i].reserved[0];
	binfo->buff_off[i] = b->m.planes[i].reserved[1];
	binfo->size[i] = b->m.planes[i].length;
	binfo->uvaddr[i] = b->m.planes[i].m.userptr;
	binfo->num_planes = b->length;
	binfo->memory = b->memory;
	binfo->v4l2_index = b->index;
	binfo->timestamp.tv_sec = b->timestamp.tv_sec;
	binfo->timestamp.tv_usec = b->timestamp.tv_usec;
	dprintk(VIDC_DBG, "%s: fd[%d] = %d b->index = %d",
			__func__, i, binfo->fd[i], b->index);
}

static inline void repopulate_v4l2_buffer(struct v4l2_buffer *b,
					struct buffer_info *binfo)
{
	int i = 0;
	b->type = binfo->type;
	b->length = binfo->num_planes;
	b->memory = binfo->memory;
	b->index = binfo->v4l2_index;
	b->timestamp.tv_sec = binfo->timestamp.tv_sec;
	b->timestamp.tv_usec = binfo->timestamp.tv_usec;
	binfo->dequeued = false;
	for (i = 0; i < binfo->num_planes; ++i) {
		b->m.planes[i].reserved[0] = binfo->fd[i];
		b->m.planes[i].reserved[1] = binfo->buff_off[i];
		b->m.planes[i].length = binfo->size[i];
		b->m.planes[i].m.userptr = binfo->device_addr[i];
		dprintk(VIDC_DBG, "%s %d %d %d %pa\n", __func__, binfo->fd[i],
				binfo->buff_off[i], binfo->size[i],
				&binfo->device_addr[i]);
	}
}

static struct msm_smem *map_buffer(struct msm_vidc_inst *inst,
		struct v4l2_plane *p, enum hal_buffer buffer_type)
{
	struct msm_smem *handle = NULL;
	handle = msm_comm_smem_user_to_kernel(inst,
				p->reserved[0],
				p->length,
				buffer_type);
	if (!handle) {
		dprintk(VIDC_ERR,
			"%s: Failed to get device buffer address\n", __func__);
		return NULL;
	}
	return handle;
}

static inline enum hal_buffer get_hal_buffer_type(
		struct msm_vidc_inst *inst, struct v4l2_buffer *b)
{
	if (b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return HAL_BUFFER_INPUT;
	else if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return HAL_BUFFER_OUTPUT;
	else
		return -EINVAL;
}

static inline bool is_dynamic_output_buffer_mode(struct v4l2_buffer *b,
				struct msm_vidc_inst *inst)
{
	return b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
		inst->buffer_mode_set[CAPTURE_PORT] == HAL_BUFFER_MODE_DYNAMIC;
}

static inline void save_v4l2_buffer(struct v4l2_buffer *b,
						struct buffer_info *binfo)
{
	int i = 0;
	for (i = 0; i < b->length; ++i) {
		if (EXTRADATA_IDX(b->length) &&
			(i == EXTRADATA_IDX(b->length)) &&
			!b->m.planes[i].length) {
			continue;
		}
		populate_buf_info(binfo, b, i);
	}

	if (EXTRADATA_IDX(b->length)) {
		i = EXTRADATA_IDX(b->length);
		if (b->m.planes[i].length)
			binfo->device_addr[i] = binfo->handle[i]->device_addr +
				binfo->buff_off[i];
	}
}

int map_and_register_buf(struct msm_vidc_inst *inst, struct v4l2_buffer *b)
{
	struct buffer_info *binfo = NULL;
	struct buffer_info *temp = NULL, *iterator = NULL;
	int plane = 0;
	int i = 0, rc = 0;
	struct msm_smem *same_fd_handle = NULL;

	if (!b || !inst) {
		dprintk(VIDC_ERR, "%s: invalid input\n", __func__);
		return -EINVAL;
	}

	binfo = kzalloc(sizeof(*binfo), GFP_KERNEL);
	if (!binfo) {
		dprintk(VIDC_ERR, "Out of memory\n");
		rc = -ENOMEM;
		goto exit;
	}
	if (b->length > VIDEO_MAX_PLANES) {
		dprintk(VIDC_ERR, "Num planes exceeds max: %d, %d\n",
			b->length, VIDEO_MAX_PLANES);
		rc = -EINVAL;
		goto exit;
	}

	dprintk(VIDC_DBG,
		"[MAP] Create binfo = %pK fd = %d size = %d type = %d\n",
		binfo, b->m.planes[0].reserved[0],
		b->m.planes[0].length, b->type);

	for (i = 0; i < b->length; ++i) {
		rc = 0;
		if (EXTRADATA_IDX(b->length) &&
			(i == EXTRADATA_IDX(b->length)) &&
			!b->m.planes[i].length) {
			continue;
		}
		mutex_lock(&inst->registeredbufs.lock);
		temp = get_registered_buf(inst, b, i, &plane);
		if (temp && !is_dynamic_output_buffer_mode(b, inst)) {
			dprintk(VIDC_DBG,
				"This memory region has already been prepared\n");
			rc = 0;
			mutex_unlock(&inst->registeredbufs.lock);
			goto exit;
		}

		if (temp && is_dynamic_output_buffer_mode(b, inst) && !i) {
			/*
			* Buffer is already present in registered list
			* increment ref_count, populate new values of v4l2
			* buffer in existing buffer_info struct.
			*
			* We will use the saved buffer info and queue it when
			* we receive RELEASE_BUFFER_REFERENCE EVENT from f/w.
			*/
			dprintk(VIDC_DBG, "[MAP] Buffer already prepared\n");
			temp->inactive = false;
			list_for_each_entry(iterator,
				&inst->registeredbufs.list, list) {
				if (iterator == temp) {
					rc = buf_ref_get(inst, temp);
					save_v4l2_buffer(b, temp);
					break;
				}
			}
		}
		mutex_unlock(&inst->registeredbufs.lock);
		/*
		 * rc == 1,
		 * buffer is mapped, fw has released all reference, so skip
		 * mapping and queue it immediately.
		 *
		 * rc == 2,
		 * buffer is mapped and fw is holding a reference, hold it in
		 * the driver and queue it later when fw has released
		 */
		if (rc == 1) {
			rc = 0;
			goto exit;
		} else if (rc == 2) {
			rc = -EEXIST;
			goto exit;
		}

		same_fd_handle = get_same_fd_buffer(
				inst, b->m.planes[i].reserved[0]);

		populate_buf_info(binfo, b, i);
		if (same_fd_handle) {
			binfo->device_addr[i] =
			same_fd_handle->device_addr + binfo->buff_off[i];
			b->m.planes[i].m.userptr = binfo->device_addr[i];
			binfo->mapped[i] = false;
			binfo->handle[i] = same_fd_handle;
		} else {
			binfo->handle[i] = map_buffer(inst, &b->m.planes[i],
					get_hal_buffer_type(inst, b));
			if (!binfo->handle[i]) {
				rc = -EINVAL;
				goto exit;
			}

			binfo->mapped[i] = true;
			binfo->device_addr[i] = binfo->handle[i]->device_addr +
				binfo->buff_off[i];
			b->m.planes[i].m.userptr = binfo->device_addr[i];
		}

		/* We maintain one ref count for all planes*/
		if (!i && is_dynamic_output_buffer_mode(b, inst)) {
			rc = buf_ref_get(inst, binfo);
			if (rc < 0)
				goto exit;
		}
		dprintk(VIDC_DBG,
			"%s: [MAP] binfo = %pK, handle[%d] = %pK, device_addr = %pa, fd = %d, offset = %d, mapped = %d\n",
			__func__, binfo, i, binfo->handle[i],
			&binfo->device_addr[i], binfo->fd[i],
			binfo->buff_off[i], binfo->mapped[i]);
	}

	mutex_lock(&inst->registeredbufs.lock);
	list_add_tail(&binfo->list, &inst->registeredbufs.list);
	mutex_unlock(&inst->registeredbufs.lock);
	return 0;

exit:
	kfree(binfo);
	return rc;
}

int unmap_and_deregister_buf(struct msm_vidc_inst *inst,
			struct buffer_info *binfo)
{
	int i = 0;
	struct buffer_info *temp = NULL;
	bool found = false, keep_node = false;

	if (!inst || !binfo) {
		dprintk(VIDC_ERR, "%s invalid param: %pK %pK\n",
			__func__, inst, binfo);
		return -EINVAL;
	}

	WARN(!mutex_is_locked(&inst->registeredbufs.lock),
		"Registered buf lock is not acquired for %s", __func__);

	/*
	* Make sure the buffer to be unmapped and deleted
	* from the registered list is present in the list.
	*/
	list_for_each_entry(temp, &inst->registeredbufs.list, list) {
		if (temp == binfo) {
			found = true;
			break;
		}
	}

	/*
	* Free the buffer info only if
	* - buffer info has not been deleted from registered list
	* - vidc client has called dqbuf on the buffer
	* - no references are held on the buffer
	*/
	if (!found || !temp || !temp->pending_deletion || !temp->dequeued)
		goto exit;

	for (i = 0; i < temp->num_planes; i++) {
		dprintk(VIDC_DBG,
			"%s: [UNMAP] binfo = %pK, handle[%d] = %pK, device_addr = %pa, fd = %d, offset = %d, mapped = %d\n",
			__func__, temp, i, temp->handle[i],
			&temp->device_addr[i], temp->fd[i],
			temp->buff_off[i], temp->mapped[i]);
		/*
		* Unmap the handle only if the buffer has been mapped and no
		* other buffer has a reference to this buffer.
		* In case of buffers with same fd, we will map the buffer only
		* once and subsequent buffers will refer to the mapped buffer's
		* device address.
		* For buffers which share the same fd, do not unmap and keep
		* the buffer info in registered list.
		*/
		if (temp->handle[i] && temp->mapped[i] &&
			!temp->same_fd_ref[i]) {
			msm_comm_smem_free(inst,
				temp->handle[i]);
		}

		if (temp->same_fd_ref[i])
			keep_node = true;
		else {
			temp->fd[i] = 0;
			temp->handle[i] = 0;
			temp->device_addr[i] = 0;
			temp->uvaddr[i] = 0;
		}
	}
	if (!keep_node) {
		dprintk(VIDC_DBG, "[UNMAP] AND-FREED binfo: %pK\n", temp);
		list_del(&temp->list);
		kfree(temp);
	} else {
		temp->inactive = true;
		dprintk(VIDC_DBG, "[UNMAP] NOT-FREED binfo: %pK\n", temp);
	}
exit:
	return 0;
}

int qbuf_dynamic_buf(struct msm_vidc_inst *inst,
			struct buffer_info *binfo)
{
	struct v4l2_buffer b = {0};
	struct v4l2_plane plane[VIDEO_MAX_PLANES] = { {0} };

	if (!binfo) {
		dprintk(VIDC_ERR, "%s invalid param: %pK\n", __func__, binfo);
		return -EINVAL;
	}
	dprintk(VIDC_DBG, "%s fd[0] = %d\n", __func__, binfo->fd[0]);

	b.m.planes = plane;
	repopulate_v4l2_buffer(&b, binfo);

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_qbuf(inst, &b);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_qbuf(inst, &b);

	return -EINVAL;
}

int output_buffer_cache_invalidate(struct msm_vidc_inst *inst,
			struct buffer_info *binfo, struct v4l2_buffer *b)
{
	int i = 0;
	int rc = 0;
	int size = -1;

	if (!inst) {
		dprintk(VIDC_ERR, "%s: invalid inst: %pK\n", __func__, inst);
		return -EINVAL;
	}

	if (!binfo) {
		dprintk(VIDC_ERR, "%s: invalid buffer info: %pK\n",
			__func__, inst);
		return -EINVAL;
	}

	if (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		for (i = 0; i < binfo->num_planes; i++) {
			if (binfo->handle[i]) {
				struct msm_smem smem = *binfo->handle[i];

				if (inst->session_type == MSM_VIDC_ENCODER &&
					!i)
					size = b->m.planes[i].bytesused +
						b->m.planes[i].data_offset;
				else
					size = -1;

				smem.offset =
					(unsigned int)(binfo->buff_off[i]);
				smem.size   = binfo->size[i];
				rc = msm_comm_smem_cache_operations(inst,
					&smem, SMEM_CACHE_INVALIDATE,
					size);
				if (rc) {
					dprintk(VIDC_ERR,
						"%s: Failed to clean caches: %d\n",
						__func__, rc);
					return -EINVAL;
				}
			} else
				dprintk(VIDC_DBG,
					"%s: NULL handle for plane %d\n",
					__func__, i);
		}
	}
	return 0;
}

static bool valid_v4l2_buffer(struct v4l2_buffer *b,
		struct msm_vidc_inst *inst) {
	enum vidc_ports port =
		!V4L2_TYPE_IS_MULTIPLANAR(b->type) ? MAX_PORT_NUM :
		b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ? CAPTURE_PORT :
		b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE ? OUTPUT_PORT :
								MAX_PORT_NUM;

	return port != MAX_PORT_NUM &&
		inst->prop.num_planes[port] == b->length;
}

int msm_vidc_prepare_buf(void *instance, struct v4l2_buffer *b)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !inst->core || !b || !valid_v4l2_buffer(b, inst))
		return -EINVAL;

	if (inst->state == MSM_VIDC_CORE_INVALID ||
		inst->core->state == VIDC_CORE_INVALID)
		return -EINVAL;

	if (is_dynamic_output_buffer_mode(b, inst))
		return 0;

	if (map_and_register_buf(inst, b))
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_prepare_buf(inst, b);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_prepare_buf(instance, b);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_prepare_buf);

int msm_vidc_release_buffers(void *instance, int buffer_type)
{
	struct msm_vidc_inst *inst = instance;
	struct buffer_info *bi, *dummy;
	struct v4l2_buffer buffer_info;
	struct v4l2_plane plane[VIDEO_MAX_PLANES];
	struct vb2_buf_entry *temp, *next;
	int i, rc = 0;

	if (!inst)
		return -EINVAL;

	if (!inst->in_reconfig &&
		inst->state > MSM_VIDC_LOAD_RESOURCES &&
		inst->state < MSM_VIDC_RELEASE_RESOURCES_DONE) {
		rc = msm_comm_try_state(inst, MSM_VIDC_RELEASE_RESOURCES_DONE);
		if (rc) {
			dprintk(VIDC_ERR,
					"Failed to move inst: %pK to release res done\n",
					inst);
		}
	}

	/*
	* In dynamic buffer mode, driver needs to release resources,
	* but not call release buffers on firmware, as the buffers
	* were never registered with firmware.
	*/
	if (buffer_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
		inst->buffer_mode_set[CAPTURE_PORT] ==
				HAL_BUFFER_MODE_DYNAMIC) {
		goto free_and_unmap;
	}

	mutex_lock(&inst->registeredbufs.lock);
	list_for_each_entry(bi, &inst->registeredbufs.list, list) {
		bool release_buf = false;

		if (bi->type == buffer_type) {
			buffer_info.type = bi->type;
			for (i = 0; i < min(bi->num_planes, VIDEO_MAX_PLANES);
						i++) {
				plane[i].reserved[0] = bi->fd[i];
				plane[i].reserved[1] = bi->buff_off[i];
				plane[i].length = bi->size[i];
				plane[i].m.userptr = bi->device_addr[i];
				buffer_info.m.planes = plane;
				dprintk(VIDC_DBG,
					"Releasing buffer: %d, %d, %d\n",
					buffer_info.m.planes[i].reserved[0],
					buffer_info.m.planes[i].reserved[1],
					buffer_info.m.planes[i].length);
			}
			buffer_info.length = bi->num_planes;
			release_buf = true;
		}

		if (!release_buf)
			continue;
		if (inst->session_type == MSM_VIDC_DECODER)
			rc = msm_vdec_release_buf(inst,	&buffer_info);
		if (inst->session_type == MSM_VIDC_ENCODER)
			rc = msm_venc_release_buf(instance,
				&buffer_info);
		if (rc)
			dprintk(VIDC_ERR,
				"Failed Release buffer: %d, %d, %d\n",
				buffer_info.m.planes[0].reserved[0],
				buffer_info.m.planes[0].reserved[1],
				buffer_info.m.planes[0].length);
	}
	mutex_unlock(&inst->registeredbufs.lock);

free_and_unmap:
	mutex_lock(&inst->registeredbufs.lock);
	list_for_each_entry_safe(bi, dummy, &inst->registeredbufs.list, list) {
		if (bi->type == buffer_type) {
			list_del(&bi->list);
			for (i = 0; i < bi->num_planes; i++) {
				if (bi->handle[i] && bi->mapped[i]) {
					dprintk(VIDC_DBG,
						"%s: [UNMAP] binfo = %pK, handle[%d] = %pK, device_addr = %pa, fd = %d, offset = %d, mapped = %d\n",
						__func__, bi, i, bi->handle[i],
						&bi->device_addr[i], bi->fd[i],
						bi->buff_off[i], bi->mapped[i]);
					msm_comm_smem_free(inst,
							bi->handle[i]);
				}
			}
			kfree(bi);
		}
	}
	mutex_unlock(&inst->registeredbufs.lock);

	mutex_lock(&inst->pendingq.lock);
	list_for_each_entry_safe(temp, next, &inst->pendingq.list, list) {
		if (temp->vb->type == buffer_type) {
			list_del(&temp->list);
			kfree(temp);
		}
	}
	mutex_unlock(&inst->pendingq.lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_release_buffers);

int msm_vidc_qbuf(void *instance, struct v4l2_buffer *b)
{
	struct msm_vidc_inst *inst = instance;
	struct buffer_info *binfo;
	int plane = 0;
	int rc = 0;
	int i;
	int size = -1;

	if (!inst || !inst->core || !b || !valid_v4l2_buffer(b, inst))
		return -EINVAL;

	if (inst->state == MSM_VIDC_CORE_INVALID ||
		inst->core->state == VIDC_CORE_INVALID)
		return -EINVAL;

	rc = map_and_register_buf(inst, b);
	if (rc == -EEXIST) {
		if (atomic_read(&inst->in_flush) &&
			is_dynamic_output_buffer_mode(b, inst)) {
			dprintk(VIDC_ERR,
				"Flush in progress, do not hold any buffers in driver\n");
			msm_comm_flush_dynamic_buffers(inst);
		}
		return 0;
	}
	if (rc)
		return rc;

	for (i = 0; i < b->length; ++i) {
		if (EXTRADATA_IDX(b->length) &&
			(i == EXTRADATA_IDX(b->length)) &&
			!b->m.planes[i].length) {
			b->m.planes[i].m.userptr = 0;
			continue;
		}
		mutex_lock(&inst->registeredbufs.lock);
		binfo = get_registered_buf(inst, b, i, &plane);
		mutex_unlock(&inst->registeredbufs.lock);
		if (!binfo) {
			dprintk(VIDC_ERR,
				"This buffer is not registered: %d, %d, %d\n",
				b->m.planes[i].reserved[0],
				b->m.planes[i].reserved[1],
				b->m.planes[i].length);
			goto err_invalid_buff;
		}
		b->m.planes[i].m.userptr = binfo->device_addr[i];
		dprintk(VIDC_DBG, "Queueing device address = %pa\n",
				&binfo->device_addr[i]);

		if (inst->fmts[OUTPUT_PORT].fourcc ==
			V4L2_PIX_FMT_HEVC_HYBRID && binfo->handle[i] &&
			b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			rc = msm_comm_smem_cache_operations(inst,
				binfo->handle[i], SMEM_CACHE_INVALIDATE, -1);
			if (rc) {
				dprintk(VIDC_ERR,
					"Failed to inv caches: %d\n", rc);
				goto err_invalid_buff;
			}
		}

		if (binfo->handle[i] &&
			(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)) {
			if (inst->session_type == MSM_VIDC_DECODER && !i)
				size = b->m.planes[i].bytesused +
						b->m.planes[i].data_offset;
			else
				size = -1;
			rc = msm_comm_smem_cache_operations(inst,
					binfo->handle[i], SMEM_CACHE_CLEAN,
					size);
			if (rc) {
				dprintk(VIDC_ERR,
					"Failed to clean caches: %d\n", rc);
				goto err_invalid_buff;
			}
		}
	}

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_qbuf(inst, b);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_qbuf(instance, b);

err_invalid_buff:
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_qbuf);

int msm_vidc_dqbuf(void *instance, struct v4l2_buffer *b)
{
	struct msm_vidc_inst *inst = instance;
	struct buffer_info *buffer_info = NULL;
	int i = 0, rc = 0;

	if (!inst || !b || !valid_v4l2_buffer(b, inst))
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		rc = msm_vdec_dqbuf(instance, b);
	if (inst->session_type == MSM_VIDC_ENCODER)
		rc = msm_venc_dqbuf(instance, b);

	if (rc)
		return rc;

	for (i = b->length - 1; i >= 0 ; i--) {
		if (EXTRADATA_IDX(b->length) &&
			i == EXTRADATA_IDX(b->length)) {
			continue;
		}
		buffer_info = device_to_uvaddr(&inst->registeredbufs,
			b->m.planes[i].m.userptr);

		if (!buffer_info) {
			dprintk(VIDC_ERR,
				"%s no buffer info registered for buffer addr: %#lx\n",
				__func__, b->m.planes[i].m.userptr);
			return -EINVAL;
		}

		b->m.planes[i].m.userptr = buffer_info->uvaddr[i];
		b->m.planes[i].reserved[0] = buffer_info->fd[i];
		b->m.planes[i].reserved[1] = buffer_info->buff_off[i];
		if (!(inst->flags & VIDC_SECURE) && !b->m.planes[i].m.userptr) {
			dprintk(VIDC_ERR,
			"%s: Failed to find user virtual address, %#lx, %d, %d\n",
			__func__, b->m.planes[i].m.userptr, b->type, i);
			return -EINVAL;
		}
	}

	if (!buffer_info) {
		dprintk(VIDC_ERR,
			"%s: error - no buffer info found in registered list\n",
			__func__);
		return -EINVAL;
	}

	rc = output_buffer_cache_invalidate(inst, buffer_info, b);
	if (rc)
		return rc;


	if (is_dynamic_output_buffer_mode(b, inst)) {
		buffer_info->dequeued = true;

		dprintk(VIDC_DBG, "[DEQUEUED]: fd[0] = %d\n",
			buffer_info->fd[0]);
		mutex_lock(&inst->registeredbufs.lock);
		rc = unmap_and_deregister_buf(inst, buffer_info);
		mutex_unlock(&inst->registeredbufs.lock);
	}

	return rc;
}
EXPORT_SYMBOL(msm_vidc_dqbuf);

int msm_vidc_streamon(void *instance, enum v4l2_buf_type i)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_streamon(inst, i);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_streamon(instance, i);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_streamon);

int msm_vidc_streamoff(void *instance, enum v4l2_buf_type i)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst)
		return -EINVAL;

	if (inst->session_type == MSM_VIDC_DECODER)
		return msm_vdec_streamoff(inst, i);
	if (inst->session_type == MSM_VIDC_ENCODER)
		return msm_venc_streamoff(instance, i);
	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_streamoff);

int msm_vidc_enum_framesizes(void *instance, struct v4l2_frmsizeenum *fsize)
{
	struct msm_vidc_inst *inst = instance;
	struct msm_vidc_capability *capability = NULL;
	enum hal_video_codec codec;
	int i;

	if (!inst || !fsize) {
		dprintk(VIDC_ERR, "%s: invalid parameter: %pK %pK\n",
				__func__, inst, fsize);
		return -EINVAL;
	}
	if (!inst->core)
		return -EINVAL;
	if (fsize->index != 0)
		return -EINVAL;

	codec = get_hal_codec(fsize->pixel_format);
	if (codec == HAL_UNUSED_CODEC)
		return -EINVAL;

	for (i = 0; i < VIDC_MAX_SESSIONS; i++) {
		if (inst->core->capabilities[i].codec == codec) {
			capability = &inst->core->capabilities[i];
			break;
		}
	}

	if (capability) {
		fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;
		fsize->stepwise.min_width = capability->width.min;
		fsize->stepwise.max_width = capability->width.max;
		fsize->stepwise.step_width = capability->width.step_size;
		fsize->stepwise.min_height = capability->height.min;
		fsize->stepwise.max_height = capability->height.max;
		fsize->stepwise.step_height = capability->height.step_size;
	} else {
		dprintk(VIDC_ERR, "%s: Invalid Pixel Fmt %#x\n",
				__func__, fsize->pixel_format);
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL(msm_vidc_enum_framesizes);

static void *vidc_get_userptr(void *alloc_ctx, unsigned long vaddr,
				unsigned long size, enum dma_data_direction dma_dir)
{
	return (void *)0xdeadbeef;
}

static void vidc_put_userptr(void *buf_priv)
{
}

static const struct vb2_mem_ops msm_vidc_vb2_mem_ops = {
	.get_userptr = vidc_get_userptr,
	.put_userptr = vidc_put_userptr,
};

static inline int vb2_bufq_init(struct msm_vidc_inst *inst,
		enum v4l2_buf_type type, enum session_type sess)
{
	struct vb2_queue *q = NULL;
	if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		q = &inst->bufq[CAPTURE_PORT].vb2_bufq;
	} else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		q = &inst->bufq[OUTPUT_PORT].vb2_bufq;
	} else {
		dprintk(VIDC_ERR, "buf_type = %d not recognised\n", type);
		return -EINVAL;
	}

	q->type = type;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;

	if (sess == MSM_VIDC_DECODER)
		q->ops = msm_vdec_get_vb2q_ops();
	else if (sess == MSM_VIDC_ENCODER)
		q->ops = msm_venc_get_vb2q_ops();
	q->mem_ops = &msm_vidc_vb2_mem_ops;
	q->drv_priv = inst;
	q->allow_zero_bytesused = 1;
	return vb2_queue_init(q);
}

static int setup_event_queue(void *inst,
				struct video_device *pvdev)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	v4l2_fh_init(&vidc_inst->event_handler, pvdev);
	v4l2_fh_add(&vidc_inst->event_handler);

	return rc;
}

int msm_vidc_subscribe_event(void *inst, const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !sub)
		return -EINVAL;

	rc = v4l2_event_subscribe(&vidc_inst->event_handler, sub, MAX_EVENTS, NULL);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_subscribe_event);

int msm_vidc_unsubscribe_event(void *inst, const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !sub)
		return -EINVAL;

	rc = v4l2_event_unsubscribe(&vidc_inst->event_handler, sub);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_unsubscribe_event);

int msm_vidc_dqevent(void *inst, struct v4l2_event *event)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !event)
		return -EINVAL;

	rc = v4l2_event_dequeue(&vidc_inst->event_handler, event, false);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_dqevent);

static bool msm_vidc_check_for_inst_overload(struct msm_vidc_core *core)
{
	u32 instance_count = 0;
	u32 secure_instance_count = 0;
	struct msm_vidc_inst *inst = NULL;
	bool overload = false;

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		instance_count++;
		/* This flag is not updated yet for the current instance */
		if (inst->flags & VIDC_SECURE)
			secure_instance_count++;
	}
	mutex_unlock(&core->lock);

	/* Instance count includes current instance as well. */

	if ((instance_count > core->resources.max_inst_count) ||
		(secure_instance_count > core->resources.max_secure_inst_count))
		overload = true;
	return overload;
}

void *msm_vidc_open(int core_id, int session_type)
{
	struct msm_vidc_inst *inst = NULL;
	struct msm_vidc_core *core = NULL;
	int rc = 0;
	int i = 0;
	if (core_id >= MSM_VIDC_CORES_MAX ||
			session_type >= MSM_VIDC_MAX_DEVICES) {
		dprintk(VIDC_ERR, "Invalid input, core_id = %d, session = %d\n",
			core_id, session_type);
		goto err_invalid_core;
	}
	core = get_vidc_core(core_id);
	if (!core) {
		dprintk(VIDC_ERR,
			"Failed to find core for core_id = %d\n", core_id);
		goto err_invalid_core;
	}

	inst = kzalloc(sizeof(*inst), GFP_KERNEL);
	if (!inst) {
		dprintk(VIDC_ERR, "Failed to allocate memory\n");
		rc = -ENOMEM;
		goto err_invalid_core;
	}

	pr_info(VIDC_DBG_TAG "Opening video instance: %pK, %d\n",
		VIDC_MSG_PRIO2STRING(VIDC_INFO), inst, session_type);
	mutex_init(&inst->sync_lock);
	mutex_init(&inst->bufq[CAPTURE_PORT].lock);
	mutex_init(&inst->bufq[OUTPUT_PORT].lock);
	mutex_init(&inst->lock);

	INIT_MSM_VIDC_LIST(&inst->pendingq);
	INIT_MSM_VIDC_LIST(&inst->scratchbufs);
	INIT_MSM_VIDC_LIST(&inst->persistbufs);
	INIT_MSM_VIDC_LIST(&inst->pending_getpropq);
	INIT_MSM_VIDC_LIST(&inst->outputbufs);
	INIT_MSM_VIDC_LIST(&inst->registeredbufs);

	kref_init(&inst->kref);

	inst->session_type = session_type;
	inst->state = MSM_VIDC_CORE_UNINIT_DONE;
	inst->core = core;
	inst->bit_depth = MSM_VIDC_BIT_DEPTH_8;
	inst->instant_bitrate = 0;
	inst->pic_struct = MSM_VIDC_PIC_STRUCT_PROGRESSIVE;
	inst->colour_space = MSM_VIDC_BT601_6_525;

	for (i = SESSION_MSG_INDEX(SESSION_MSG_START);
		i <= SESSION_MSG_INDEX(SESSION_MSG_END); i++) {
		init_completion(&inst->completions[i]);
	}
	inst->mem_client = msm_smem_new_client(SMEM_ION,
					&inst->core->resources, session_type);
	if (!inst->mem_client) {
		dprintk(VIDC_ERR, "Failed to create memory client\n");
		goto fail_mem_client;
	}
	if (session_type == MSM_VIDC_DECODER) {
		msm_vdec_inst_init(inst);
		rc = msm_vdec_ctrl_init(inst);
	} else if (session_type == MSM_VIDC_ENCODER) {
		msm_venc_inst_init(inst);
		rc = msm_venc_ctrl_init(inst);
	}

	if (rc)
		goto fail_bufq_capture;

	msm_dcvs_init(inst);
	rc = vb2_bufq_init(inst, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
			session_type);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to initialize vb2 queue on capture port\n");
		goto fail_bufq_capture;
	}
	rc = vb2_bufq_init(inst, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
			session_type);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to initialize vb2 queue on capture port\n");
		goto fail_bufq_output;
	}

	setup_event_queue(inst, &core->vdev[session_type].vdev);

	mutex_lock(&core->lock);
	list_add_tail(&inst->list, &core->instances);
	mutex_unlock(&core->lock);

	rc = msm_comm_try_state(inst, MSM_VIDC_CORE_INIT_DONE);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to move video instance to init state\n");
		goto fail_init;
	}

	if (msm_vidc_check_for_inst_overload(core)) {
		dprintk(VIDC_ERR,
			"Instance count reached Max limit, rejecting session");
		goto fail_init;
	}

	inst->debugfs_root =
		msm_vidc_debugfs_init_inst(inst, core->debugfs_root);

	return inst;
fail_init:
	mutex_lock(&core->lock);
	list_del(&inst->list);
	mutex_unlock(&core->lock);

	v4l2_fh_del(&inst->event_handler);
	v4l2_fh_exit(&inst->event_handler);
	vb2_queue_release(&inst->bufq[OUTPUT_PORT].vb2_bufq);
fail_bufq_output:
	vb2_queue_release(&inst->bufq[CAPTURE_PORT].vb2_bufq);
fail_bufq_capture:
	msm_comm_ctrl_deinit(inst);
	msm_smem_delete_client(inst->mem_client);
fail_mem_client:
	mutex_destroy(&inst->sync_lock);
	mutex_destroy(&inst->bufq[CAPTURE_PORT].lock);
	mutex_destroy(&inst->bufq[OUTPUT_PORT].lock);
	mutex_destroy(&inst->lock);

	DEINIT_MSM_VIDC_LIST(&inst->pendingq);
	DEINIT_MSM_VIDC_LIST(&inst->scratchbufs);
	DEINIT_MSM_VIDC_LIST(&inst->persistbufs);
	DEINIT_MSM_VIDC_LIST(&inst->pending_getpropq);
	DEINIT_MSM_VIDC_LIST(&inst->outputbufs);
	DEINIT_MSM_VIDC_LIST(&inst->registeredbufs);

	kfree(inst);
	inst = NULL;
err_invalid_core:
	return inst;
}
EXPORT_SYMBOL(msm_vidc_open);

static void cleanup_instance(struct msm_vidc_inst *inst)
{
	struct vb2_buf_entry *entry, *dummy;
	if (inst) {

		mutex_lock(&inst->pendingq.lock);
		list_for_each_entry_safe(entry, dummy, &inst->pendingq.list,
				list) {
			list_del(&entry->list);
			kfree(entry);
		}
		mutex_unlock(&inst->pendingq.lock);

		if (msm_comm_release_scratch_buffers(inst, false)) {
			dprintk(VIDC_ERR,
				"Failed to release scratch buffers\n");
		}

		if (msm_comm_release_persist_buffers(inst)) {
			dprintk(VIDC_ERR,
				"Failed to release persist buffers\n");
		}

		if (msm_comm_release_output_buffers(inst)) {
			dprintk(VIDC_ERR,
				"Failed to release output buffers\n");
		}

		mutex_lock(&inst->pending_getpropq.lock);
		if (!list_empty(&inst->pending_getpropq.list)) {
			dprintk(VIDC_ERR,
				"pending_getpropq not empty\n");
			WARN_ON(VIDC_DBG_WARN_ENABLE);
		}
		mutex_unlock(&inst->pending_getpropq.lock);
	}
}

int msm_vidc_destroy(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;

	if (!inst || !inst->core)
		return -EINVAL;

	core = inst->core;

	mutex_lock(&core->lock);
	/* inst->list lives in core->instances */
	list_del(&inst->list);
	mutex_unlock(&core->lock);

	msm_comm_ctrl_deinit(inst);

	DEINIT_MSM_VIDC_LIST(&inst->pendingq);
	DEINIT_MSM_VIDC_LIST(&inst->scratchbufs);
	DEINIT_MSM_VIDC_LIST(&inst->persistbufs);
	DEINIT_MSM_VIDC_LIST(&inst->pending_getpropq);
	DEINIT_MSM_VIDC_LIST(&inst->outputbufs);
	DEINIT_MSM_VIDC_LIST(&inst->registeredbufs);

	v4l2_fh_del(&inst->event_handler);
	v4l2_fh_exit(&inst->event_handler);

	mutex_destroy(&inst->sync_lock);
	mutex_destroy(&inst->bufq[CAPTURE_PORT].lock);
	mutex_destroy(&inst->bufq[OUTPUT_PORT].lock);
	mutex_destroy(&inst->lock);

	msm_vidc_debugfs_deinit_inst(inst);

	pr_info(VIDC_DBG_TAG "Closed video instance: %pK\n",
			VIDC_MSG_PRIO2STRING(VIDC_INFO), inst);
	kfree(inst);
	return 0;
}

static void close_helper(struct kref *kref)
{
	struct msm_vidc_inst *inst = container_of(kref,
			struct msm_vidc_inst, kref);

	msm_vidc_destroy(inst);
}

int msm_vidc_close(void *instance)
{
	struct msm_vidc_inst *inst = instance;
	struct buffer_info *bi, *dummy;
	int rc = 0, i = 0;

	if (!inst || !inst->core)
		return -EINVAL;


	mutex_lock(&inst->registeredbufs.lock);
	list_for_each_entry_safe(bi, dummy, &inst->registeredbufs.list, list) {
		if (bi->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
			int i = 0;

			list_del(&bi->list);

			for (i = 0; i < min(bi->num_planes, VIDEO_MAX_PLANES);
					i++) {
				if (bi->handle[i] && bi->mapped[i])
					msm_comm_smem_free(inst, bi->handle[i]);
			}

			kfree(bi);
		}
	}
	mutex_unlock(&inst->registeredbufs.lock);

	cleanup_instance(inst);
	if (inst->state != MSM_VIDC_CORE_INVALID &&
		inst->core->state != VIDC_CORE_INVALID)
		rc = msm_comm_try_state(inst, MSM_VIDC_CORE_UNINIT);
	else
		rc = msm_comm_force_cleanup(inst);
	if (rc)
		dprintk(VIDC_ERR,
			"Failed to move video instance to uninit state\n");

	msm_comm_session_clean(inst);
	msm_smem_delete_client(inst->mem_client);

	for (i = 0; i < MAX_PORT_NUM; i++) {
		mutex_lock(&inst->bufq[i].lock);
		vb2_queue_release(&inst->bufq[i].vb2_bufq);
		mutex_unlock(&inst->bufq[i].lock);
	}

	kref_put(&inst->kref, close_helper);
	return 0;
}
EXPORT_SYMBOL(msm_vidc_close);

int msm_vidc_suspend(int core_id)
{
	return msm_comm_suspend(core_id);
}
EXPORT_SYMBOL(msm_vidc_suspend);

