/* Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/spinlock.h>
#include <linux/videodev2.h>
#include <linux/proc_fs.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>


#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-core.h>
#include <media/msm_camera.h>
#include <media/msm_isp.h>

#include <linux/qcom_iommu.h>

#include "msm.h"
#include "msm_buf_mgr.h"
#include "cam_smmu_api.h"

#undef CDBG
#define CDBG(fmt, args...) pr_debug(fmt, ##args)

static int msm_buf_check_head_sanity(struct msm_isp_bufq *bufq)
{
	int rc = 0;
	struct list_head *prev = NULL;
	struct list_head *next = NULL;

	if (!bufq) {
		pr_err("%s: Error! Invalid bufq\n", __func__);
		return -EINVAL;
	}

	prev = bufq->head.prev;
	next = bufq->head.next;

	if (prev->next != &bufq->head) {
		pr_err("%s: Error! head prev->next is %p should be %p\n",
			__func__, prev->next, &bufq->head);
		return -EINVAL;
	}

	if (next->prev != &bufq->head) {
		pr_err("%s: Error! head next->prev is %p should be %p\n",
			__func__, next->prev, &bufq->head);
		return -EINVAL;
	}

	prev = bufq->share_head.prev;
	next = bufq->share_head.next;

	if (prev->next != &bufq->share_head) {
		pr_err("%s: Error! share_head prev->next is %p should be %p\n",
			__func__, prev->next, &bufq->share_head);
		return -EINVAL;
	}

	if (next->prev != &bufq->share_head) {
		pr_err("%s: Error! share_head next->prev is %p should be %p\n",
			__func__, next->prev, &bufq->share_head);
		return -EINVAL;
	}

	return rc;
}

struct msm_isp_bufq *msm_isp_get_bufq(
	struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle)
{
	struct msm_isp_bufq *bufq = NULL;
	uint32_t bufq_index = bufq_handle & 0xFF;

	if ((bufq_handle == 0) ||
		(bufq_index > buf_mgr->num_buf_q) ||
		(bufq_index >= BUF_MGR_NUM_BUF_Q) )
		return NULL;

	bufq = &buf_mgr->bufq[bufq_index];
	if (bufq->bufq_handle == bufq_handle)
		return bufq;

	return NULL;
}

static struct msm_isp_buffer *msm_isp_get_buf_ptr(
	struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index)
{
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return buf_info;
	}

	if (bufq->num_bufs <= buf_index) {
		pr_err("%s: Invalid buf index\n", __func__);
		return buf_info;
	}
	buf_info = &bufq->bufs[buf_index];
	return buf_info;
}

static uint32_t msm_isp_get_buf_handle(
	struct msm_isp_buf_mgr *buf_mgr,
	uint32_t session_id, uint32_t stream_id)
{
	int i;
	if ((buf_mgr->buf_handle_cnt << 8) == 0)
		buf_mgr->buf_handle_cnt++;

	for (i = 0; i < buf_mgr->num_buf_q; i++) {
		if (buf_mgr->bufq[i].session_id == session_id &&
			buf_mgr->bufq[i].stream_id == stream_id)
			return 0;
	}

	for (i = 0; i < buf_mgr->num_buf_q; i++) {
		if (buf_mgr->bufq[i].bufq_handle == 0) {
			memset(&buf_mgr->bufq[i],
				0, sizeof(struct msm_isp_bufq));
			buf_mgr->bufq[i].bufq_handle =
				(++buf_mgr->buf_handle_cnt) << 8 | i;
			return buf_mgr->bufq[i].bufq_handle;
		}
	}
	return 0;
}

static int msm_isp_free_buf_handle(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle)
{
	struct msm_isp_bufq *bufq =
		msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq)
		return -EINVAL;

	/* Set everything except lock to 0 */
	bufq->bufq_handle = 0;
	bufq->bufs = 0;
	bufq->session_id = 0;
	bufq->stream_id = 0;
	bufq->num_bufs = 0;
	bufq->buf_type = 0;
	memset(&bufq->head, 0, sizeof(bufq->head));
	memset(&bufq->share_head, 0, sizeof(bufq->share_head));
	bufq->buf_client_count = 0;

	return 0;
}

static void msm_isp_copy_planes_from_v4l2_buffer(
	struct msm_isp_qbuf_buffer *qbuf_buf,
	const struct v4l2_buffer *v4l2_buf)
{
	int i;
	qbuf_buf->num_planes = v4l2_buf->length;
	for (i = 0; i < qbuf_buf->num_planes; i++) {
		qbuf_buf->planes[i].addr = v4l2_buf->m.planes[i].m.userptr;
		qbuf_buf->planes[i].offset = v4l2_buf->m.planes[i].data_offset;
		qbuf_buf->planes[i].length = v4l2_buf->m.planes[i].length;
	}
}

static int msm_isp_prepare_isp_buf(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_buffer *buf_info,
	struct msm_isp_qbuf_buffer *qbuf_buf)
{
	int i, rc = -1;
	int ret;
	struct msm_isp_buffer_mapped_info *mapped_info;
	uint32_t accu_length = 0;
	int iommu_hdl;

	if (buf_mgr->secure_enable == NON_SECURE_MODE)
		iommu_hdl = buf_mgr->ns_iommu_hdl;
	else
		iommu_hdl = buf_mgr->sec_iommu_hdl;

	for (i = 0; i < qbuf_buf->num_planes; i++) {
		mapped_info = &buf_info->mapped_info[i];
		mapped_info->buf_fd = qbuf_buf->planes[i].addr;
		ret = cam_smmu_get_phy_addr(iommu_hdl,
					mapped_info->buf_fd,
					CAM_SMMU_MAP_RW,
					&(mapped_info->paddr),
					(size_t *)&(mapped_info->len));
		if (ret) {
			rc = -EINVAL;
			pr_err_ratelimited("%s: cannot map address", __func__);
			goto get_phy_err;
		}
		mapped_info->paddr += accu_length;
		accu_length += qbuf_buf->planes[i].length;
		CDBG("%s: plane: %d addr:%lu\n",
			__func__, i, (unsigned long)mapped_info->paddr);

	}
	buf_info->num_planes = qbuf_buf->num_planes;
	return 0;

get_phy_err:
	i--;
	for (; i >= 0; i--) {
		cam_smmu_put_phy_addr(iommu_hdl,
				qbuf_buf->planes[i].addr);
	}
	return rc;
}

static void msm_isp_unprepare_v4l2_buf(
	struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_buffer *buf_info)
{
	int i;
	struct msm_isp_buffer_mapped_info *mapped_info;
	int iommu_hdl;

	if (buf_mgr->secure_enable == NON_SECURE_MODE)
		iommu_hdl = buf_mgr->ns_iommu_hdl;
	else
		iommu_hdl = buf_mgr->sec_iommu_hdl;

	for (i = 0; i < buf_info->num_planes; i++) {
		mapped_info = &buf_info->mapped_info[i];
		if (mapped_info != NULL)
			cam_smmu_put_phy_addr(iommu_hdl,
							mapped_info->buf_fd);
	}
	return;
}

static int msm_isp_map_buf(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_buffer_mapped_info *mapped_info, uint32_t fd)
{
	int rc = 0;
	int ret;
	int iommu_hdl;

	if (!buf_mgr || !mapped_info) {
		pr_err_ratelimited("%s: %d] NULL ptr buf_mgr %p mapped_info %p\n",
			__func__, __LINE__, buf_mgr, mapped_info);
		return -EINVAL;
	}

	if (buf_mgr->secure_enable == NON_SECURE_MODE)
		iommu_hdl = buf_mgr->ns_iommu_hdl;
	else
		iommu_hdl = buf_mgr->sec_iommu_hdl;

	ret = cam_smmu_get_phy_addr(iommu_hdl,
				fd,
				CAM_SMMU_MAP_RW,
				&(mapped_info->paddr),
				(size_t *)&(mapped_info->len));

	if (ret) {
		rc = -EINVAL;
		pr_err_ratelimited("%s: cannot map address", __func__);
		goto smmu_map_error;
	}
	CDBG("%s: addr:%lu\n",
		__func__, (unsigned long)mapped_info->paddr);

	return rc;
smmu_map_error:
	cam_smmu_put_phy_addr(iommu_hdl,
			fd);
	return rc;
}

static int msm_isp_unmap_buf(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t fd)
{
	int iommu_hdl;

	if (!buf_mgr) {
		pr_err_ratelimited("%s: %d] NULL ptr buf_mgr\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	if (buf_mgr->secure_enable == NON_SECURE_MODE)
		iommu_hdl = buf_mgr->ns_iommu_hdl;
	else
		iommu_hdl = buf_mgr->sec_iommu_hdl;

	cam_smmu_put_phy_addr(iommu_hdl,
			fd);

	return 0;
}

static int msm_isp_buf_prepare(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_qbuf_info *info, struct vb2_buffer *vb2_buf)
{
	int rc = -1;
	unsigned long flags;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	struct msm_isp_qbuf_buffer buf;

	buf_info = msm_isp_get_buf_ptr(buf_mgr,
		info->handle, info->buf_idx);
	if (!buf_info) {
		pr_err("Invalid buffer prepare\n");
		return rc;
	}

	bufq = msm_isp_get_bufq(buf_mgr, buf_info->bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n",
			__func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED) {
		rc = buf_info->state;
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
		return rc;
	}

	if (buf_info->state != MSM_ISP_BUFFER_STATE_INITIALIZED) {
		pr_err_ratelimited("%s: Invalid buffer state: %d bufq %x buf-id %d\n",
			__func__, buf_info->state, bufq->bufq_handle,
			buf_info->buf_idx);
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
		return rc;
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);

	if (vb2_buf) {
		msm_isp_copy_planes_from_v4l2_buffer(&buf, &vb2_buf->v4l2_buf);
		buf_info->vb2_buf = vb2_buf;
	} else {
		buf = info->buffer;
	}

	rc = msm_isp_prepare_isp_buf(buf_mgr, buf_info, &buf);
	if (rc < 0) {
		pr_err_ratelimited("%s: Prepare buffer error\n", __func__);
		return rc;
	}
	spin_lock_irqsave(&bufq->bufq_lock, flags);
	buf_info->state = MSM_ISP_BUFFER_STATE_PREPARED;
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return rc;
}

static int msm_isp_buf_unprepare_all(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t buf_handle)
{
	int rc = -1, i;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	bufq = msm_isp_get_bufq(buf_mgr, buf_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return rc;
	}

	for (i = 0; i < bufq->num_bufs; i++) {
		buf_info = msm_isp_get_buf_ptr(buf_mgr, buf_handle, i);
		if (!buf_info) {
			pr_err("%s: buf not found\n", __func__);
			return rc;
		}
		if (buf_info->state == MSM_ISP_BUFFER_STATE_UNUSED ||
				buf_info->state ==
					MSM_ISP_BUFFER_STATE_INITIALIZED)
			continue;

		if (MSM_ISP_BUFFER_SRC_HAL == BUF_SRC(bufq->stream_id)) {
			if (buf_info->state == MSM_ISP_BUFFER_STATE_DEQUEUED ||
			buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED)
				buf_mgr->vb2_ops->put_buf(buf_info->vb2_buf,
					bufq->session_id, bufq->stream_id);
		}
		msm_isp_unprepare_v4l2_buf(buf_mgr, buf_info);
	}
	return 0;
}

static int msm_isp_get_buf_by_index(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index,
	struct msm_isp_buffer **buf_info)
{
	int rc = -EINVAL;
	unsigned long flags;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *temp_buf_info;
	uint32_t i = 0;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (buf_index >= bufq->num_bufs) {
		pr_err("%s: Invalid buf index: %d max: %d\n", __func__,
			buf_index, bufq->num_bufs);
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
		return rc;
	}

	*buf_info = NULL;
	for (i = 0; bufq->num_bufs; i++) {
		temp_buf_info = &bufq->bufs[i];
		if (temp_buf_info && temp_buf_info->buf_idx == buf_index) {
			*buf_info = temp_buf_info;
			break;
		}
	}

	if (*buf_info) {
		pr_debug("Found buf in isp buf mgr");
		rc = 0;
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return rc;
}

static int msm_isp_buf_unprepare(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t buf_handle, int32_t buf_idx)
{
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	bufq = msm_isp_get_bufq(buf_mgr, buf_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return -EINVAL;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, buf_handle, buf_idx);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return -EINVAL;
	}
	if (buf_info->state == MSM_ISP_BUFFER_STATE_UNUSED ||
			buf_info->state == MSM_ISP_BUFFER_STATE_INITIALIZED)
		return 0;

	if (MSM_ISP_BUFFER_SRC_HAL == BUF_SRC(bufq->stream_id)) {
		if (buf_info->state == MSM_ISP_BUFFER_STATE_DEQUEUED ||
		buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED)
			buf_mgr->vb2_ops->put_buf(buf_info->vb2_buf,
				bufq->session_id, bufq->stream_id);
	}
	msm_isp_unprepare_v4l2_buf(buf_mgr, buf_info);

	return 0;
}

static int msm_isp_get_buf(struct msm_isp_buf_mgr *buf_mgr, uint32_t id,
	uint32_t bufq_handle, struct msm_isp_buffer **buf_info,
	uint32_t *buf_cnt , uint8_t ping_pong_bit)
{
	int rc = -1;
	unsigned long flags;
	struct msm_isp_buffer *temp_buf_info = NULL, *safe = NULL;
	struct msm_isp_bufq *bufq = NULL;
	struct vb2_buffer *vb2_buf = NULL;
	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err_ratelimited("%s: Invalid bufq\n", __func__);
		return rc;
	}
	if (!bufq->bufq_handle) {
		pr_err_ratelimited("%s: Invalid bufq handle\n", __func__);
		return rc;
	}

	*buf_info = NULL;
	*buf_cnt = 0;
	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (bufq->buf_type == ISP_SHARE_BUF) {
		list_for_each_entry_safe(temp_buf_info,
			safe, &bufq->share_head, share_list) {
			/* Check buffer state before proceeding. Buffers in share list
			 * should be either UNUSED (temp buf) or DEQUEUED */
			if ((MSM_ISP_BUFFER_STATE_DEQUEUED!=temp_buf_info->state) &&
				(MSM_ISP_BUFFER_STATE_UNUSED!=temp_buf_info->state)) {
			    list_del_init(
				    &temp_buf_info->share_list);
			    if (msm_buf_check_head_sanity(bufq)
				    < 0) {
				pr_err("%s buf_handle 0x%x buf_idx %d buf_reuse_flag %d\n",
					__func__,
					bufq->bufq_handle,
					temp_buf_info->buf_idx,
					temp_buf_info->buf_reuse_flag);
				spin_unlock_irqrestore(
					&bufq->bufq_lock, flags);
				dump_stack();
				return -EFAULT;
			    }
			} else if (!temp_buf_info->buf_used[id] &&
				(temp_buf_info->ping_pong_bit ==
				ping_pong_bit)) {
				temp_buf_info->buf_used[id] = 1;
				temp_buf_info->buf_get_count++;
				*buf_cnt = temp_buf_info->buf_get_count;
				if (temp_buf_info->buf_get_count ==
					bufq->buf_client_count) {
					list_del_init(
					&temp_buf_info->share_list);
					if (msm_buf_check_head_sanity(bufq)
						 < 0) {
						pr_err("%s buf_handle 0x%x buf_idx %d buf_reuse_flag %d\n",
						__func__,
						bufq->bufq_handle,
						temp_buf_info->buf_idx,
						temp_buf_info->buf_reuse_flag);
						spin_unlock_irqrestore(
						&bufq->bufq_lock, flags);
						dump_stack();
						return -EFAULT;
					}
				if (temp_buf_info->buf_reuse_flag) {
					kfree(temp_buf_info);
				} else {
					*buf_info = temp_buf_info;
					rc = 0;
				}
				spin_unlock_irqrestore(
					&bufq->bufq_lock, flags);
				return rc;
				} else {
					pr_err("%s: Error! Invalid vfe_id %d buf_index %d bufq %x\n",
						__func__, id, temp_buf_info->buf_idx,
						bufq_handle);
					spin_unlock_irqrestore(
						&bufq->bufq_lock, flags);
					return rc;
				}
			} else if (temp_buf_info->buf_used[id] &&
					temp_buf_info->buf_reuse_flag) {
				spin_unlock_irqrestore(
					&bufq->bufq_lock, flags);
				return rc;
			}
		}
	}

	switch (BUF_SRC(bufq->stream_id)) {
	case MSM_ISP_BUFFER_SRC_NATIVE:
		list_for_each_entry_safe(temp_buf_info, safe, &bufq->head, list) {
			if (temp_buf_info->state ==
					MSM_ISP_BUFFER_STATE_QUEUED) {
			    *buf_info = temp_buf_info;
			    temp_buf_info = NULL;
			    list_for_each_entry(temp_buf_info,
				    &bufq->share_head, share_list) {
				if ((temp_buf_info->buf_idx ==
					    (*buf_info)->buf_idx) &&
					!temp_buf_info->buf_reuse_flag) {
				    pr_err("%s ERROR! Double ADD buf_idx:%d\n",
					    __func__, temp_buf_info->buf_idx);
				    pr_err("state %d buf_get_count %d buf_put_count %d buf_reuse_flag %d buf_used[%d] %d\n", temp_buf_info->state,
					    temp_buf_info->buf_get_count,temp_buf_info->buf_put_count,temp_buf_info->buf_reuse_flag,id,temp_buf_info->buf_used[id]);
				    list_del_init(&temp_buf_info->share_list);
				}
			    }
			    /* found one buf */
			    list_del_init(&(*buf_info)->list);
			    if (msm_buf_check_head_sanity(bufq)
				    < 0) {
				pr_err("%s buf_handle 0x%x buf_idx %d buf_reuse_flag %d\n",
					__func__,
					bufq->bufq_handle,
					(*buf_info)->buf_idx,
					(*buf_info)->buf_reuse_flag);
				spin_unlock_irqrestore(
					&bufq->bufq_lock, flags);
				dump_stack();
				return -EFAULT;
			    }
			    break;
			}
		}
		break;
	case MSM_ISP_BUFFER_SRC_HAL:
		vb2_buf = buf_mgr->vb2_ops->get_buf(
			bufq->session_id, bufq->stream_id);
		if (vb2_buf) {
			if (vb2_buf->v4l2_buf.index < bufq->num_bufs) {
						*buf_info =
					&bufq->bufs[vb2_buf->v4l2_buf.index];
						(*buf_info)->vb2_buf = vb2_buf;
			} else {
				pr_err("%s: Incorrect buf index %d\n",
					__func__, vb2_buf->v4l2_buf.index);
				rc = -EINVAL;
			}
			if ((*buf_info) == NULL) {
				buf_mgr->vb2_ops->put_buf(vb2_buf,
					bufq->session_id, bufq->stream_id);
				pr_err("%s: buf index %d not found!\n",
					__func__, vb2_buf->v4l2_buf.index);
				rc = -EINVAL;

			}
		} else {
			pr_err_ratelimited("%s: No Buffer session_id:%d stream_id:%x\n",
				__func__, bufq->session_id, bufq->stream_id);
			rc = -EINVAL;
		}
		break;
	case MSM_ISP_BUFFER_SRC_SCRATCH:
		/* In scratch buf case we have only on buffer in queue.
		 * We return every time same buffer. */
		*buf_info = list_entry(bufq->head.next, typeof(**buf_info),
				list);
		break;
	default:
		pr_err("%s: Incorrect buf source.\n", __func__);
		rc = -EINVAL;
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
		return rc;
	}

	if (!(*buf_info)) {
		if (bufq->buf_type == ISP_SHARE_BUF) {
			temp_buf_info = kzalloc(
			   sizeof(struct msm_isp_buffer), GFP_ATOMIC);
			if (temp_buf_info) {
				temp_buf_info->buf_reuse_flag = 1;
				temp_buf_info->buf_used[id] = 1;
				temp_buf_info->buf_get_count = 1;
				temp_buf_info->ping_pong_bit = ping_pong_bit;
				INIT_LIST_HEAD(&temp_buf_info->share_list);
				list_add_tail(&temp_buf_info->share_list,
							  &bufq->share_head);
				if (msm_buf_check_head_sanity(bufq)
					 < 0) {
					pr_err("%s buf_handle 0x%x buf_idx %d buf_reuse_flag %d\n",
						__func__,
						bufq->bufq_handle,
						temp_buf_info->buf_idx,
						temp_buf_info->buf_reuse_flag);
					spin_unlock_irqrestore(
					&bufq->bufq_lock, flags);
					dump_stack();
					return -EFAULT;
				}
			} else
				rc = -ENOMEM;
		}
	} else {
		(*buf_info)->state = MSM_ISP_BUFFER_STATE_DEQUEUED;
		if (bufq->buf_type == ISP_SHARE_BUF) {
			memset((*buf_info)->buf_used, 0,
				   sizeof(uint8_t) * bufq->buf_client_count);
			(*buf_info)->buf_used[id] = 1;
			(*buf_info)->buf_get_count = 1;
			(*buf_info)->buf_put_count = 0;
			(*buf_info)->buf_reuse_flag = 0;
			(*buf_info)->ping_pong_bit = ping_pong_bit;
			list_add_tail(&(*buf_info)->share_list,
						  &bufq->share_head);
			if (msm_buf_check_head_sanity(bufq)
				 < 0) {
				pr_err("%s buf_handle 0x%x buf_idx %d buf_reuse_flag %d\n",
					__func__,
					bufq->bufq_handle,
					(*buf_info)->buf_idx,
					(*buf_info)->buf_reuse_flag);
				spin_unlock_irqrestore(&bufq->bufq_lock, flags);
				dump_stack();
				return -EFAULT;
			}
		}
		rc = 0;
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return rc;
}

static int msm_isp_put_buf(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index)
{
	int rc = -1;
	unsigned long flags;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return rc;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);

	buf_info->buf_get_count = 0;
	buf_info->buf_put_count = 0;
	buf_info->ping_pong_bit = 0;
	memset(buf_info->buf_used, 0, sizeof(buf_info->buf_used));

	switch (buf_info->state) {
	case MSM_ISP_BUFFER_STATE_PREPARED:
		if (MSM_ISP_BUFFER_SRC_SCRATCH == BUF_SRC(bufq->stream_id))
			list_add_tail(&buf_info->list, &bufq->head);
	case MSM_ISP_BUFFER_STATE_DEQUEUED:
	case MSM_ISP_BUFFER_STATE_DIVERTED:
		if (MSM_ISP_BUFFER_SRC_NATIVE == BUF_SRC(bufq->stream_id)) {
			list_add_tail(&buf_info->list, &bufq->head);
		} else if (MSM_ISP_BUFFER_SRC_HAL == BUF_SRC(bufq->stream_id)) {
			buf_mgr->vb2_ops->put_buf(buf_info->vb2_buf,
				bufq->session_id, bufq->stream_id);
		}
		buf_info->state = MSM_ISP_BUFFER_STATE_QUEUED;
		rc = 0;
		break;
	case MSM_ISP_BUFFER_STATE_DISPATCHED:
		buf_info->state = MSM_ISP_BUFFER_STATE_QUEUED;
		rc = 0;
		break;
	case MSM_ISP_BUFFER_STATE_QUEUED:
		rc = 0;
		break;
	default:
		pr_err("%s: incorrect state = %d",
			__func__, buf_info->state);
		break;
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);

	return rc;
}

static int msm_isp_put_buf_unsafe(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index)
{
	int rc = -1;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n", __func__);
		return rc;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return rc;
	}

	buf_info->buf_get_count = 0;
	buf_info->buf_put_count = 0;
	buf_info->ping_pong_bit = 0;
	memset(buf_info->buf_used, 0, sizeof(buf_info->buf_used));

	switch (buf_info->state) {
	case MSM_ISP_BUFFER_STATE_PREPARED:
	case MSM_ISP_BUFFER_STATE_DEQUEUED:
	case MSM_ISP_BUFFER_STATE_DIVERTED:
		if (BUF_SRC(bufq->stream_id)) {
			list_add_tail(&buf_info->list, &bufq->head);
		} else {
			buf_mgr->vb2_ops->put_buf(buf_info->vb2_buf,
				bufq->session_id, bufq->stream_id);
		}
		buf_info->state = MSM_ISP_BUFFER_STATE_QUEUED;
		rc = 0;
		break;
	case MSM_ISP_BUFFER_STATE_DISPATCHED:
		buf_info->state = MSM_ISP_BUFFER_STATE_QUEUED;
		rc = 0;
		break;
	case MSM_ISP_BUFFER_STATE_QUEUED:
		rc = 0;
		break;
	default:
		pr_err("%s: incorrect state = %d",
			__func__, buf_info->state);
		break;
	}

	return rc;
}

static int msm_isp_reset_put_buf_cnt(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index)
{
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	unsigned long flags;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq\n");
		return -EINVAL;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (bufq->buf_type == ISP_SHARE_BUF)
		buf_info->buf_put_count = 0;
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return 0;
}

static int msm_isp_update_put_buf_cnt(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index, uint32_t frame_id)
{
	int rc = -1;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	enum msm_isp_buffer_state state;
	unsigned long flags;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq\n");
		return rc;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (bufq->buf_type != ISP_SHARE_BUF ||
		buf_info->buf_put_count == 0) {
		buf_info->frame_id = frame_id;
	}

	state = buf_info->state;
	if (state == MSM_ISP_BUFFER_STATE_DEQUEUED ||
		state == MSM_ISP_BUFFER_STATE_DIVERTED) {
		if (bufq->buf_type == ISP_SHARE_BUF) {
			buf_info->buf_put_count++;
			if (buf_info->buf_put_count != ISP_SHARE_BUF_CLIENT) {
				rc = buf_info->buf_put_count;
				spin_unlock_irqrestore(&bufq->bufq_lock, flags);
				return rc;
			}
		}
	} else {
		pr_err_ratelimited("%s: Invalid state\n", __func__);
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return 0;
}

static int msm_isp_buf_done(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index,
	struct timeval *tv, uint32_t frame_id, uint32_t output_format)
{
	int rc = -1;
	unsigned long flags;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	enum msm_isp_buffer_state state;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq\n");
		return rc;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	state = buf_info->state;
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);

	if (state == MSM_ISP_BUFFER_STATE_DEQUEUED ||
		state == MSM_ISP_BUFFER_STATE_DIVERTED) {
		spin_lock_irqsave(&bufq->bufq_lock, flags);
		buf_info->state = MSM_ISP_BUFFER_STATE_DISPATCHED;
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
		if (MSM_ISP_BUFFER_SRC_HAL == BUF_SRC(bufq->stream_id)) {
			buf_info->vb2_buf->v4l2_buf.timestamp = *tv;
			buf_info->vb2_buf->v4l2_buf.sequence  = frame_id;
			buf_info->vb2_buf->v4l2_buf.reserved  = output_format;
			buf_mgr->vb2_ops->buf_done(buf_info->vb2_buf,
				bufq->session_id, bufq->stream_id);
		} else {
			pr_err("%s: Error wrong buf done %d\n", __func__,
				state);
			rc = msm_isp_put_buf(buf_mgr, buf_info->bufq_handle,
						buf_info->buf_idx);
			if (rc < 0) {
				pr_err("%s: Buf put failed\n", __func__);
				return rc;
			}
		}
	}

	return 0;
}

static int msm_isp_flush_buf(struct msm_isp_buf_mgr *buf_mgr,
		uint32_t bufq_handle, enum msm_isp_buffer_flush_t flush_type)
{
	int rc = -1, i;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	unsigned long flags;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq\n");
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	for (i = 0; i < bufq->num_bufs; i++) {
		buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, i);
		if (!buf_info) {
			pr_err("%s: buf not found\n", __func__);
			continue;
		}
		if (flush_type == MSM_ISP_BUFFER_FLUSH_DIVERTED &&
			buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED) {
			buf_info->state = MSM_ISP_BUFFER_STATE_QUEUED;
		} else if (flush_type == MSM_ISP_BUFFER_FLUSH_ALL) {
			if (buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED) {
				CDBG("%s: no need to queue Diverted buffer\n",
					__func__);
			} else if (buf_info->state ==
				MSM_ISP_BUFFER_STATE_DEQUEUED) {
			    msm_isp_put_buf_unsafe(buf_mgr,
				    bufq_handle, buf_info->buf_idx);
			}
		}
	}

	if (bufq->buf_type == ISP_SHARE_BUF) {
		while (!list_empty(&bufq->share_head)) {
			buf_info = list_entry((&bufq->share_head)->next,
				typeof(*buf_info), share_list);
			list_del_init(&(buf_info->share_list));
			if (buf_info->buf_reuse_flag)
				kfree(buf_info);
		 }
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return 0;
}

static int msm_isp_buf_divert(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t buf_index,
	struct timeval *tv, uint32_t frame_id)
{
	int rc = -1;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	unsigned long flags;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq\n");
		return rc;
	}

	buf_info = msm_isp_get_buf_ptr(buf_mgr, bufq_handle, buf_index);
	if (!buf_info) {
		pr_err("%s: buf not found\n", __func__);
		return rc;
	}

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	if (bufq->buf_type != ISP_SHARE_BUF ||
		buf_info->buf_put_count == 0) {
		buf_info->frame_id = frame_id;
	}

	if (bufq->buf_type == ISP_SHARE_BUF) {
		buf_info->buf_put_count++;
		if (buf_info->buf_put_count != ISP_SHARE_BUF_CLIENT) {
			rc = buf_info->buf_put_count;
			buf_info->frame_id = frame_id;
			spin_unlock_irqrestore(&bufq->bufq_lock, flags);
			return rc;
		}
	}

	if (buf_info->state == MSM_ISP_BUFFER_STATE_DEQUEUED) {
		buf_info->state = MSM_ISP_BUFFER_STATE_DIVERTED;
		buf_info->tv = tv;
	}
	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	return 0;
}

static int msm_isp_buf_enqueue(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_qbuf_info *info)
{
	int rc = -1, buf_state;
	struct msm_isp_bufq *bufq = NULL;
	struct msm_isp_buffer *buf_info = NULL;
	buf_state = msm_isp_buf_prepare(buf_mgr, info, NULL);
	if (buf_state < 0) {
		pr_err_ratelimited("%s: Buf prepare failed\n", __func__);
		return -EINVAL;
	}

	if (buf_state == MSM_ISP_BUFFER_STATE_DIVERTED) {
		buf_info = msm_isp_get_buf_ptr(buf_mgr,
						info->handle, info->buf_idx);
		if (!buf_info) {
			pr_err("%s: buf not found\n", __func__);
			return rc;
		}
		if (info->dirty_buf) {
			rc = msm_isp_put_buf(buf_mgr,
				info->handle, info->buf_idx);
		} else {
			bufq = msm_isp_get_bufq(buf_mgr, info->handle);
			if (!bufq) {
				pr_err("%s: Invalid bufq\n",
					__func__);
				return rc;
			}
			if (BUF_SRC(bufq->stream_id))
				pr_err("%s: Invalid native buffer state\n",
					__func__);
			else
				rc = msm_isp_buf_done(buf_mgr,
					info->handle, info->buf_idx,
					buf_info->tv, buf_info->frame_id, 0);
		}
	} else {
		bufq = msm_isp_get_bufq(buf_mgr, info->handle);
		if (!bufq) {
			pr_err("%s: Invalid bufq\n", __func__);
			return rc;
			}
		if (MSM_ISP_BUFFER_SRC_HAL != BUF_SRC(bufq->stream_id)) {
			rc = msm_isp_put_buf(buf_mgr,
					info->handle, info->buf_idx);
			if (rc < 0) {
				pr_err("%s: Buf put failed stream %x\n",
					__func__, bufq->stream_id);
				return rc;
			}
		}
	}
	return rc;
}

static int msm_isp_buf_dequeue(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_qbuf_info *info)
{
	struct msm_isp_buffer *buf_info = NULL;
	int rc = 0;

	buf_info = msm_isp_get_buf_ptr(buf_mgr, info->handle, info->buf_idx);
	if (!buf_info) {
		pr_err("Invalid buffer dequeue\n");
		return -EINVAL;
	}

	if (buf_info->state == MSM_ISP_BUFFER_STATE_DEQUEUED ||
		buf_info->state == MSM_ISP_BUFFER_STATE_DIVERTED) {
		pr_err("%s: Invalid state %d\n", __func__, buf_info->state);
		return -EINVAL;
	}
	msm_isp_buf_unprepare(buf_mgr, info->handle, info->buf_idx);

	buf_info->state = MSM_ISP_BUFFER_STATE_INITIALIZED;

	return rc;
}

static int msm_isp_get_bufq_handle(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t session_id, uint32_t stream_id)
{
	int i;
	for (i = 0; i < buf_mgr->num_buf_q; i++) {
		if (buf_mgr->bufq[i].session_id == session_id &&
			buf_mgr->bufq[i].stream_id == stream_id) {
			return buf_mgr->bufq[i].bufq_handle;
		}
	}
	pr_err("%s: No match found 0x%x 0x%x\n", __func__,
			session_id, stream_id);
	return 0;
}

static int msm_isp_get_buf_src(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle, uint32_t *buf_src)
{
	struct msm_isp_bufq *bufq = NULL;

	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq\n",
			__func__);
		return -EINVAL;
	}
	*buf_src = BUF_SRC(bufq->stream_id);

	return 0;
}

static int msm_isp_request_bufq(struct msm_isp_buf_mgr *buf_mgr,
	struct msm_isp_buf_request *buf_request)
{
	int rc = -1, i;
	struct msm_isp_bufq *bufq = NULL;
	CDBG("%s: E\n", __func__);

	if (!buf_request->num_buf || buf_request->num_buf > VB2_MAX_FRAME) {
		pr_err("Invalid buffer request\n");
		return rc;
	}

	buf_request->handle = msm_isp_get_buf_handle(buf_mgr,
		buf_request->session_id, buf_request->stream_id);
	if (!buf_request->handle) {
		pr_err("Invalid buffer handle\n");
		return rc;
	}

	bufq = msm_isp_get_bufq(buf_mgr, buf_request->handle);
	if (!bufq) {
		pr_err("%s: Invalid bufq stream_id %x\n",
			__func__, buf_request->stream_id);

		return rc;
	}

	bufq->bufs = kzalloc(sizeof(struct msm_isp_buffer) *
		buf_request->num_buf, GFP_KERNEL);
	if (!bufq->bufs) {
		pr_err("No free memory for buf info\n");
		msm_isp_free_buf_handle(buf_mgr, buf_request->handle);
		return rc;
	}

	spin_lock_init(&bufq->bufq_lock);
	bufq->bufq_handle = buf_request->handle;
	bufq->session_id = buf_request->session_id;
	bufq->stream_id = buf_request->stream_id;
	bufq->num_bufs = buf_request->num_buf;
	bufq->buf_type = buf_request->buf_type;
	if (bufq->buf_type == ISP_SHARE_BUF)
		bufq->buf_client_count = ISP_SHARE_BUF_CLIENT;
	INIT_LIST_HEAD(&bufq->head);
	INIT_LIST_HEAD(&bufq->share_head);
	for (i = 0; i < buf_request->num_buf; i++) {
		bufq->bufs[i].state = MSM_ISP_BUFFER_STATE_INITIALIZED;
		bufq->bufs[i].bufq_handle = bufq->bufq_handle;
		bufq->bufs[i].buf_idx = i;
	}

	return 0;
}

static int msm_isp_release_bufq(struct msm_isp_buf_mgr *buf_mgr,
	uint32_t bufq_handle)
{
	struct msm_isp_bufq *bufq = NULL;
	unsigned long flags;
	int rc = -1;
	mutex_lock(&buf_mgr->lock);
	bufq = msm_isp_get_bufq(buf_mgr, bufq_handle);
	if (!bufq) {
		pr_err("Invalid bufq release\n");
		mutex_unlock(&buf_mgr->lock);
		return rc;
	}

	msm_isp_buf_unprepare_all(buf_mgr, bufq_handle);

	spin_lock_irqsave(&bufq->bufq_lock, flags);
	kfree(bufq->bufs);
	msm_isp_free_buf_handle(buf_mgr, bufq_handle);

	spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	mutex_unlock(&buf_mgr->lock);

	return 0;
}

static void msm_isp_release_all_bufq(
	struct msm_isp_buf_mgr *buf_mgr)
{
	struct msm_isp_bufq *bufq = NULL;
	unsigned long flags;
	int i;
	for (i = 0; i < buf_mgr->num_buf_q; i++) {
		bufq = &buf_mgr->bufq[i];
		if (!bufq->bufq_handle)
			continue;

		msm_isp_buf_unprepare_all(buf_mgr, bufq->bufq_handle);

		spin_lock_irqsave(&bufq->bufq_lock, flags);
		kfree(bufq->bufs);
		msm_isp_free_buf_handle(buf_mgr, bufq->bufq_handle);
		spin_unlock_irqrestore(&bufq->bufq_lock, flags);
	}
}

int msm_isp_smmu_attach(struct msm_isp_buf_mgr *buf_mgr,
	void *arg)
{
	struct msm_vfe_smmu_attach_cmd *cmd = arg;
	int iommu_hdl;
	int rc = 0;

	pr_debug("%s: cmd->security_mode : %d\n", __func__, cmd->security_mode);
	mutex_lock(&buf_mgr->lock);
	if (cmd->iommu_attach_mode == IOMMU_ATTACH) {
		buf_mgr->secure_enable = cmd->security_mode;

		if (buf_mgr->secure_enable == NON_SECURE_MODE)
			iommu_hdl = buf_mgr->ns_iommu_hdl;
		else
			iommu_hdl = buf_mgr->sec_iommu_hdl;

		if ((buf_mgr->secure_enable == SECURE_MODE) &&
			(buf_mgr->num_iommu_secure_ctx < 1)) {
			pr_err("%s: Error! Invalid request for secure ctx\n",
				__func__);
			rc = -1;
			goto iommu_error;
		}
		if (buf_mgr->attach_ref_cnt == 0) {
			rc = cam_smmu_ops(iommu_hdl, CAM_SMMU_ATTACH);
			if (rc < 0) {
				pr_err("%s: smmu attach error, rc :%d\n",
					__func__, rc);
				goto iommu_error;
			}
		}
		buf_mgr->attach_ref_cnt++;
	} else {
		if (buf_mgr->secure_enable == NON_SECURE_MODE)
			iommu_hdl = buf_mgr->ns_iommu_hdl;
		else
			iommu_hdl = buf_mgr->sec_iommu_hdl;

		if (buf_mgr->attach_ref_cnt == 1) {
			rc = cam_smmu_ops(iommu_hdl, CAM_SMMU_DETACH);
			if (rc < 0) {
				pr_err("%s: smmu detach error, rc :%d\n",
					__func__, rc);
				goto iommu_error;
			}
		}
		if (buf_mgr->attach_ref_cnt > 0)
			buf_mgr->attach_ref_cnt--;
		else
			pr_err("%s: Error! Invalid ref_cnt\n", __func__);
	}

iommu_error:
	mutex_unlock(&buf_mgr->lock);
	return rc;
}


static int msm_isp_init_isp_buf_mgr(
	struct msm_isp_buf_mgr *buf_mgr,
	const char *ctx_name, uint16_t num_buf_q)
{
	int rc = -1;
	mutex_lock(&buf_mgr->lock);
	if (buf_mgr->open_count++) {
		mutex_unlock(&buf_mgr->lock);
		return 0;
	}

	if (!num_buf_q) {
		pr_err("Invalid buffer queue number\n");
		mutex_unlock(&buf_mgr->lock);
		return rc;
	}
	CDBG("%s: E\n", __func__);

	buf_mgr->num_buf_q = num_buf_q;
	buf_mgr->bufq =
		kzalloc(sizeof(struct msm_isp_bufq) * num_buf_q,
		GFP_KERNEL);
	if (!buf_mgr->bufq) {
		pr_err("Bufq malloc error\n");
		goto bufq_error;
	}

	rc = cam_smmu_get_handle("vfe", &buf_mgr->ns_iommu_hdl);
	if (rc < 0) {
		pr_err("vfe non secure get handled failed\n");
		goto get_handle_error1;
	}
	rc = cam_smmu_get_handle("vfe_secure", &buf_mgr->sec_iommu_hdl);
	if (rc < 0) {
		pr_err("vfe secure get handled failed\n");
		goto get_handle_error2;
	}
	buf_mgr->buf_handle_cnt = 0;
	buf_mgr->pagefault_debug = 0;
	buf_mgr->frameId_mismatch_recovery = 0;
	mutex_unlock(&buf_mgr->lock);
	return 0;

get_handle_error2:
	cam_smmu_destroy_handle(buf_mgr->ns_iommu_hdl);
get_handle_error1:
	kfree(buf_mgr->bufq);
bufq_error:
	mutex_unlock(&buf_mgr->lock);
	return rc;
}

static int msm_isp_deinit_isp_buf_mgr(
	struct msm_isp_buf_mgr *buf_mgr)
{
	mutex_lock(&buf_mgr->lock);
	if (buf_mgr->open_count > 0)
		buf_mgr->open_count--;

	if (buf_mgr->open_count) {
		mutex_unlock(&buf_mgr->lock);
		return 0;
	}
	msm_isp_release_all_bufq(buf_mgr);
	kfree(buf_mgr->bufq);
	buf_mgr->num_buf_q = 0;
	buf_mgr->pagefault_debug = 0;
	mutex_unlock(&buf_mgr->lock);
	cam_smmu_destroy_handle(buf_mgr->ns_iommu_hdl);
	cam_smmu_destroy_handle(buf_mgr->sec_iommu_hdl);
	buf_mgr->attach_ref_cnt = 0;
	return 0;
}

int msm_isp_proc_buf_cmd(struct msm_isp_buf_mgr *buf_mgr,
	unsigned int cmd, void *arg)
{
	switch (cmd) {
	case VIDIOC_MSM_ISP_REQUEST_BUF: {
		struct msm_isp_buf_request *buf_req = arg;

		buf_mgr->ops->request_buf(buf_mgr, buf_req);
		break;
	}
	case VIDIOC_MSM_ISP_ENQUEUE_BUF: {
		struct msm_isp_qbuf_info *qbuf_info = arg;

		buf_mgr->ops->enqueue_buf(buf_mgr, qbuf_info);
		break;
	}
	case VIDIOC_MSM_ISP_DEQUEUE_BUF: {
		struct msm_isp_qbuf_info *qbuf_info = arg;

		buf_mgr->ops->dequeue_buf(buf_mgr, qbuf_info);
		break;
	}
	case VIDIOC_MSM_ISP_RELEASE_BUF: {
		struct msm_isp_buf_request *buf_req = arg;

		buf_mgr->ops->release_buf(buf_mgr, buf_req->handle);
		break;
	}
	case VIDIOC_MSM_ISP_UNMAP_BUF: {
		struct msm_isp_unmap_buf_req *unmap_req = arg;

		buf_mgr->ops->unmap_buf(buf_mgr, unmap_req->fd);
		break;
	}
	}
	return 0;
}

static int msm_isp_buf_mgr_debug(struct msm_isp_buf_mgr *buf_mgr)
{
	struct msm_isp_buffer *bufs = NULL;
	uint32_t i = 0, j = 0, k = 0, rc = 0;
	char *print_buf = NULL, temp_buf[100];
	uint32_t start_addr = 0, end_addr = 0, print_buf_size = 2000;
	if (!buf_mgr) {
		pr_err_ratelimited("%s: %d] NULL buf_mgr\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	print_buf = kzalloc(print_buf_size, GFP_ATOMIC);
	if (!print_buf) {
		pr_err("%s failed: no memory", __func__);
		return -ENOMEM;
	}
	snprintf(print_buf, print_buf_size, "%s\n", __func__);
	for (i = 0; i < BUF_MGR_NUM_BUF_Q; i++) {
		if (i % 2 == 0 && i > 0) {
			pr_err("%s\n", print_buf);
			print_buf[0] = 0;
		}
		if (buf_mgr->bufq[i].bufq_handle != 0) {
			snprintf(temp_buf, sizeof(temp_buf),
				"handle %x stream %x num_bufs %d\n",
				buf_mgr->bufq[i].bufq_handle,
				buf_mgr->bufq[i].stream_id,
				buf_mgr->bufq[i].num_bufs);
			strlcat(print_buf, temp_buf, print_buf_size);
			for (j = 0; j < buf_mgr->bufq[i].num_bufs; j++) {
				bufs = &buf_mgr->bufq[i].bufs[j];
				if (!bufs) {
					continue;
				}
				for (k = 0; k < bufs->num_planes; k++) {
					start_addr = bufs->
							mapped_info[k].paddr;
					end_addr = bufs->mapped_info[k].paddr +
						bufs->mapped_info[k].len;
					snprintf(temp_buf, sizeof(temp_buf),
						" buf %d plane %d start_addr %x end_addr %x\n",
						j, k, start_addr, end_addr);
					strlcat(print_buf, temp_buf,
						print_buf_size);
				}
			}
			start_addr = 0;
			end_addr = 0;
		}
	}
	pr_err("%s\n", print_buf);
	kfree(print_buf);
	return rc;
}

static struct msm_isp_buf_ops isp_buf_ops = {
	.request_buf = msm_isp_request_bufq,
	.enqueue_buf = msm_isp_buf_enqueue,
	.dequeue_buf = msm_isp_buf_dequeue,
	.release_buf = msm_isp_release_bufq,
	.get_bufq_handle = msm_isp_get_bufq_handle,
	.get_buf_src = msm_isp_get_buf_src,
	.get_buf = msm_isp_get_buf,
	.get_buf_by_index = msm_isp_get_buf_by_index,
	.map_buf = msm_isp_map_buf,
	.unmap_buf = msm_isp_unmap_buf,
	.put_buf = msm_isp_put_buf,
	.flush_buf = msm_isp_flush_buf,
	.buf_done = msm_isp_buf_done,
	.buf_divert = msm_isp_buf_divert,
	.buf_mgr_init = msm_isp_init_isp_buf_mgr,
	.buf_mgr_deinit = msm_isp_deinit_isp_buf_mgr,
	.buf_mgr_debug = msm_isp_buf_mgr_debug,
	.get_bufq = msm_isp_get_bufq,
	.update_put_buf_cnt = msm_isp_update_put_buf_cnt,
	.reset_put_buf_cnt = msm_isp_reset_put_buf_cnt,
};

int msm_isp_create_isp_buf_mgr(
	struct msm_isp_buf_mgr *buf_mgr,
	struct msm_sd_req_vb2_q *vb2_ops,
	struct device *dev)
{
	int rc = 0;
	if (buf_mgr->init_done)
		return rc;

	buf_mgr->ops = &isp_buf_ops;
	buf_mgr->vb2_ops = vb2_ops;
	buf_mgr->open_count = 0;
	buf_mgr->pagefault_debug = 0;
	buf_mgr->secure_enable = NON_SECURE_MODE;
	buf_mgr->attach_state = MSM_ISP_BUF_MGR_DETACH;
	mutex_init(&buf_mgr->lock);
	spin_lock_init(&buf_mgr->bufq_list_lock);

	return 0;
}
