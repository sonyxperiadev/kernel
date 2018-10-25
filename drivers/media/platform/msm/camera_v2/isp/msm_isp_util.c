/* Copyright (c) 2013-2018, The Linux Foundation. All rights reserved.
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
#include <linux/mutex.h>
#include <linux/io.h>
#include <media/v4l2-subdev.h>
#include <linux/ratelimit.h>

#include "msm.h"
#include "msm_isp_util.h"
#include "msm_isp_axi_util.h"
#include "msm_isp_stats_util.h"
#include "msm_camera_io_util.h"
#include "cam_smmu_api.h"
#include "msm_isp48.h"
#define CREATE_TRACE_POINTS
#include "trace/events/msm_cam.h"


#define MAX_ISP_V4l2_EVENTS 100
#define MAX_ISP_REG_LIST 100
static DEFINE_MUTEX(bandwidth_mgr_mutex);
static struct msm_isp_bandwidth_mgr isp_bandwidth_mgr;

#define MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev) { \
	if (vfe_dev->is_split && vfe_dev->pdev->id == ISP_VFE0) { \
		struct vfe_device *vfe1_dev = vfe_dev->common_data-> \
					dual_vfe_res->vfe_dev[ISP_VFE1]; \
		mutex_lock(&vfe1_dev->core_mutex); \
	} \
}

#define MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev) { \
	if (vfe_dev->is_split && vfe_dev->pdev->id == ISP_VFE0) { \
		struct vfe_device *vfe1_dev = vfe_dev->common_data-> \
					dual_vfe_res->vfe_dev[ISP_VFE1]; \
		mutex_unlock(&vfe1_dev->core_mutex); \
	} \
}

static uint64_t msm_isp_cpp_clk_rate;

#define VFE40_8974V2_VERSION 0x1001001A

void msm_isp_print_fourcc_error(const char *origin, uint32_t fourcc_format)
{
	int i;
	char text[5];

	text[4] = '\0';
	for (i = 0; i < 4; i++) {
		text[i] = (char)(((fourcc_format) >> (i * 8)) & 0xFF);
		if ((text[i] < '0') || (text[i] > 'z')) {
			pr_err("%s: Invalid output format %d (unprintable)\n",
				origin, fourcc_format);
			return;
		}
	}
	pr_err("%s: Invalid output format %s\n",
		origin, text);
}

int msm_isp_init_bandwidth_mgr(struct vfe_device *vfe_dev,
			enum msm_isp_hw_client client)
{
	int rc = 0;

	mutex_lock(&bandwidth_mgr_mutex);
	if (isp_bandwidth_mgr.client_info[client].active) {
		mutex_unlock(&bandwidth_mgr_mutex);
		return rc;
	}
	isp_bandwidth_mgr.client_info[client].active = 1;
	isp_bandwidth_mgr.use_count++;
	if (vfe_dev && !isp_bandwidth_mgr.bus_client) {
		rc = vfe_dev->hw_info->vfe_ops.platform_ops.init_bw_mgr(vfe_dev,
				&isp_bandwidth_mgr);
		if (!rc) {
			isp_bandwidth_mgr.update_bw =
			vfe_dev->hw_info->vfe_ops.platform_ops.update_bw;
			isp_bandwidth_mgr.deinit_bw_mgr =
			vfe_dev->hw_info->vfe_ops.platform_ops.deinit_bw_mgr;
		}
	}
	if (rc) {
		isp_bandwidth_mgr.use_count--;
		isp_bandwidth_mgr.client_info[client].active = 0;
	}

	mutex_unlock(&bandwidth_mgr_mutex);
	return rc;
}

int msm_isp_update_bandwidth(enum msm_isp_hw_client client,
	uint64_t ab, uint64_t ib)
{
	int rc;

	mutex_lock(&bandwidth_mgr_mutex);
	if (!isp_bandwidth_mgr.use_count ||
		!isp_bandwidth_mgr.bus_client) {
		pr_err("%s:error bandwidth manager inactive use_cnt:%d bus_clnt:%d\n",
			__func__, isp_bandwidth_mgr.use_count,
			isp_bandwidth_mgr.bus_client);
		mutex_unlock(&bandwidth_mgr_mutex);
		return -EINVAL;
	}

	isp_bandwidth_mgr.client_info[client].ab = ab;
	isp_bandwidth_mgr.client_info[client].ib = ib;
	rc = isp_bandwidth_mgr.update_bw(&isp_bandwidth_mgr);
	mutex_unlock(&bandwidth_mgr_mutex);
	return 0;
}

void msm_isp_deinit_bandwidth_mgr(enum msm_isp_hw_client client)
{
	if (client >= MAX_ISP_CLIENT) {
		pr_err("invalid Client id %d", client);
		return;
	}
	mutex_lock(&bandwidth_mgr_mutex);
	memset(&isp_bandwidth_mgr.client_info[client], 0,
			sizeof(struct msm_isp_bandwidth_info));
	if (isp_bandwidth_mgr.use_count) {
		isp_bandwidth_mgr.use_count--;
		if (isp_bandwidth_mgr.use_count) {
			mutex_unlock(&bandwidth_mgr_mutex);
			return;
		}

		if (!isp_bandwidth_mgr.bus_client) {
			pr_err("%s:%d error: bus client invalid\n",
				__func__, __LINE__);
			mutex_unlock(&bandwidth_mgr_mutex);
			return;
		}

		isp_bandwidth_mgr.deinit_bw_mgr(
				&isp_bandwidth_mgr);
	}
	mutex_unlock(&bandwidth_mgr_mutex);
}

void msm_isp_util_get_bandwidth_stats(struct vfe_device *vfe_dev,
				      struct msm_isp_statistics *stats)
{
	stats->isp_vfe0_active = isp_bandwidth_mgr.client_info[ISP_VFE0].active;
	stats->isp_vfe0_ab = isp_bandwidth_mgr.client_info[ISP_VFE0].ab;
	stats->isp_vfe0_ib = isp_bandwidth_mgr.client_info[ISP_VFE0].ib;

	stats->isp_vfe1_active = isp_bandwidth_mgr.client_info[ISP_VFE1].active;
	stats->isp_vfe1_ab = isp_bandwidth_mgr.client_info[ISP_VFE1].ab;
	stats->isp_vfe1_ib = isp_bandwidth_mgr.client_info[ISP_VFE1].ib;

	stats->isp_cpp_active = isp_bandwidth_mgr.client_info[ISP_CPP].active;
	stats->isp_cpp_ab = isp_bandwidth_mgr.client_info[ISP_CPP].ab;
	stats->isp_cpp_ib = isp_bandwidth_mgr.client_info[ISP_CPP].ib;
	stats->last_overflow_ab = vfe_dev->msm_isp_last_overflow_ab;
	stats->last_overflow_ib = vfe_dev->msm_isp_last_overflow_ib;
	stats->vfe_clk_rate = vfe_dev->vfe_clk_info[
				vfe_dev->hw_info->vfe_clk_idx].clk_rate;
	stats->cpp_clk_rate = msm_isp_cpp_clk_rate;
}

void msm_isp_util_update_clk_rate(long clock_rate)
{
	msm_isp_cpp_clk_rate = clock_rate;
}

uint32_t msm_isp_get_framedrop_period(
	enum msm_vfe_frame_skip_pattern frame_skip_pattern)
{
	switch (frame_skip_pattern) {
	case NO_SKIP:
	case EVERY_2FRAME:
	case EVERY_3FRAME:
	case EVERY_4FRAME:
	case EVERY_5FRAME:
	case EVERY_6FRAME:
	case EVERY_7FRAME:
	case EVERY_8FRAME:
		return frame_skip_pattern + 1;
	case EVERY_16FRAME:
		return 16;
	case EVERY_32FRAME:
		return 32;
	case SKIP_ALL:
		return SKIP_ALL;
	default:
		return 1;
	}
	return 1;
}

void msm_isp_get_timestamp(struct msm_isp_timestamp *time_stamp,
	struct vfe_device *vfe_dev)
{
	struct timespec ts;

	do_gettimeofday(&(time_stamp->event_time));
	if (vfe_dev->vt_enable) {
		msm_isp_get_avtimer_ts(time_stamp);
		time_stamp->buf_time.tv_sec    = time_stamp->vt_time.tv_sec;
		time_stamp->buf_time.tv_usec   = time_stamp->vt_time.tv_usec;
	} else {
		get_monotonic_boottime(&ts);
		time_stamp->buf_time.tv_sec    = ts.tv_sec;
		time_stamp->buf_time.tv_usec   = ts.tv_nsec/1000;
	}
}

static inline u32 msm_isp_evt_mask_to_isp_event(u32 evt_mask)
{
	u32 evt_id = ISP_EVENT_SUBS_MASK_NONE;

	switch (evt_mask) {
	case ISP_EVENT_MASK_INDEX_STATS_NOTIFY:
		evt_id = ISP_EVENT_STATS_NOTIFY;
		break;
	case ISP_EVENT_MASK_INDEX_ERROR:
		evt_id = ISP_EVENT_ERROR;
		break;
	case ISP_EVENT_MASK_INDEX_IOMMU_P_FAULT:
		evt_id = ISP_EVENT_IOMMU_P_FAULT;
		break;
	case ISP_EVENT_MASK_INDEX_STREAM_UPDATE_DONE:
		evt_id = ISP_EVENT_STREAM_UPDATE_DONE;
		break;
	case ISP_EVENT_MASK_INDEX_REG_UPDATE:
		evt_id = ISP_EVENT_REG_UPDATE;
		break;
	case ISP_EVENT_MASK_INDEX_SOF:
		evt_id = ISP_EVENT_SOF;
		break;
	case ISP_EVENT_MASK_INDEX_BUF_DIVERT:
		evt_id = ISP_EVENT_BUF_DIVERT;
		break;
	case ISP_EVENT_MASK_INDEX_BUF_DONE:
		evt_id = ISP_EVENT_BUF_DONE;
		break;
	case ISP_EVENT_MASK_INDEX_COMP_STATS_NOTIFY:
		evt_id = ISP_EVENT_COMP_STATS_NOTIFY;
		break;
	case ISP_EVENT_MASK_INDEX_MASK_FE_READ_DONE:
		evt_id = ISP_EVENT_FE_READ_DONE;
		break;
	case ISP_EVENT_MASK_INDEX_PING_PONG_MISMATCH:
		evt_id = ISP_EVENT_PING_PONG_MISMATCH;
		break;
	case ISP_EVENT_MASK_INDEX_REG_UPDATE_MISSING:
		evt_id = ISP_EVENT_REG_UPDATE_MISSING;
		break;
	case ISP_EVENT_MASK_INDEX_BUF_FATAL_ERROR:
		evt_id = ISP_EVENT_BUF_FATAL_ERROR;
		break;
	default:
		evt_id = ISP_EVENT_SUBS_MASK_NONE;
		break;
	}

	return evt_id;
}

static inline int msm_isp_subscribe_event_mask(struct v4l2_fh *fh,
		struct v4l2_event_subscription *sub, int evt_mask_index,
		u32 evt_id, bool subscribe_flag)
{
	int rc = 0, i, interface;

	if (evt_mask_index == ISP_EVENT_MASK_INDEX_STATS_NOTIFY) {
		for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
			sub->type = evt_id + i;
			if (subscribe_flag)
				rc = v4l2_event_subscribe(fh, sub,
					MAX_ISP_V4l2_EVENTS, NULL);
			else
				rc = v4l2_event_unsubscribe(fh, sub);
			if (rc != 0) {
				pr_err("%s: Subs event_type =0x%x failed\n",
					__func__, sub->type);
				return rc;
			}
		}
	} else if (evt_mask_index == ISP_EVENT_MASK_INDEX_SOF ||
		   evt_mask_index == ISP_EVENT_MASK_INDEX_REG_UPDATE ||
		   evt_mask_index == ISP_EVENT_MASK_INDEX_STREAM_UPDATE_DONE) {
		for (interface = 0; interface < VFE_SRC_MAX; interface++) {
			sub->type = evt_id | interface;
			if (subscribe_flag)
				rc = v4l2_event_subscribe(fh, sub,
					MAX_ISP_V4l2_EVENTS, NULL);
			else
				rc = v4l2_event_unsubscribe(fh, sub);
			if (rc != 0) {
				pr_err("%s: Subs event_type =0x%x failed\n",
					__func__, sub->type);
				return rc;
			}
		}
	} else {
		sub->type = evt_id;
		if (subscribe_flag)
			rc = v4l2_event_subscribe(fh, sub,
				MAX_ISP_V4l2_EVENTS, NULL);
		else
			rc = v4l2_event_unsubscribe(fh, sub);
		if (rc != 0) {
			pr_err("%s: Subs event_type =0x%x failed\n",
				__func__, sub->type);
			return rc;
		}
	}
	return rc;
}

static inline int msm_isp_process_event_subscription(struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub, bool subscribe_flag)
{
	int rc = 0, evt_mask_index = 0;
	u32 evt_mask = sub->type;
	u32 evt_id = 0;

	if (evt_mask == ISP_EVENT_SUBS_MASK_NONE) {
		pr_err("%s: Subs event_type is None=0x%x\n",
			__func__, evt_mask);
		return 0;
	}

	for (evt_mask_index = ISP_EVENT_MASK_INDEX_STATS_NOTIFY;
		evt_mask_index <= ISP_EVENT_MASK_INDEX_BUF_FATAL_ERROR;
		evt_mask_index++) {
		if (evt_mask & (1<<evt_mask_index)) {
			evt_id = msm_isp_evt_mask_to_isp_event(evt_mask_index);
			rc = msm_isp_subscribe_event_mask(fh, sub,
				evt_mask_index, evt_id, subscribe_flag);
			if (rc != 0) {
				pr_err("%s: Subs event index:%d failed\n",
					__func__, evt_mask_index);
				return rc;
			}
		}
	}
	return rc;
}

int msm_isp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return msm_isp_process_event_subscription(fh, sub, true);
}

int msm_isp_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return msm_isp_process_event_subscription(fh, sub, false);
}

static int msm_isp_start_fetch_engine(struct vfe_device *vfe_dev,
	void *arg)
{
	struct msm_vfe_fetch_eng_start *fe_cfg = arg;
	/*
	 * For Offline VFE, HAL expects same frame id
	 * for offline output which it requested in do_reprocess.
	 */
	vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id =
		fe_cfg->frame_id;
	return vfe_dev->hw_info->vfe_ops.core_ops.
		start_fetch_eng(vfe_dev, arg);
}

static int msm_isp_start_fetch_engine_multi_pass(struct vfe_device *vfe_dev,
	void *arg)
{
	struct msm_vfe_fetch_eng_multi_pass_start *fe_cfg = arg;
	struct msm_vfe_axi_stream *stream_info = NULL;
	int i = 0, rc;
	uint32_t wm_reload_mask = 0;
	int vfe_idx;
	/*
	 * For Offline VFE, HAL expects same frame id
	 * for offline output which it requested in do_reprocess.
	 */
	vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id =
		fe_cfg->frame_id;
	if (fe_cfg->offline_pass == OFFLINE_SECOND_PASS) {
		stream_info = msm_isp_get_stream_common_data(vfe_dev,
			HANDLE_TO_IDX(fe_cfg->output_stream_id));
		if (stream_info == NULL) {
			pr_err("%s: Error in Offline process\n", __func__);
			return -EINVAL;
		}
		vfe_idx = msm_isp_get_vfe_idx_for_stream(vfe_dev, stream_info);
		msm_isp_reset_framedrop(vfe_dev, stream_info);

		mutex_lock(&vfe_dev->buf_mgr->lock);
		rc = msm_isp_cfg_offline_ping_pong_address(vfe_dev, stream_info,
			VFE_PING_FLAG, fe_cfg->output_buf_idx);
		mutex_unlock(&vfe_dev->buf_mgr->lock);
		if (rc < 0) {
			pr_err("%s: Fetch engine config failed\n", __func__);
			return -EINVAL;
		}
		for (i = 0; i < stream_info->num_planes; i++) {
			vfe_dev->hw_info->vfe_ops.axi_ops.enable_wm(
				vfe_dev->vfe_base,
				stream_info->wm[vfe_idx][i], 1);
			wm_reload_mask |= (1 << stream_info->wm[vfe_idx][i]);
		}
		vfe_dev->hw_info->vfe_ops.core_ops.reg_update(vfe_dev,
			VFE_SRC_MAX);
		vfe_dev->hw_info->vfe_ops.axi_ops.reload_wm(vfe_dev,
			vfe_dev->vfe_base, wm_reload_mask);
	}
	return vfe_dev->hw_info->vfe_ops.core_ops.
		start_fetch_eng_multi_pass(vfe_dev, arg);
}

void msm_isp_fetch_engine_done_notify(struct vfe_device *vfe_dev,
	struct msm_vfe_fetch_engine_info *fetch_engine_info)
{
	struct msm_isp_event_data fe_rd_done_event;

	memset(&fe_rd_done_event, 0, sizeof(struct msm_isp_event_data));
	fe_rd_done_event.frame_id =
		vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id;
	fe_rd_done_event.u.fetch_done.session_id =
		fetch_engine_info->session_id;
	fe_rd_done_event.u.fetch_done.stream_id = fetch_engine_info->stream_id;
	fe_rd_done_event.u.fetch_done.handle = fetch_engine_info->bufq_handle;
	fe_rd_done_event.u.fetch_done.buf_idx = fetch_engine_info->buf_idx;
	fe_rd_done_event.u.fetch_done.fd = fetch_engine_info->fd;
	fe_rd_done_event.u.fetch_done.offline_mode =
		fetch_engine_info->offline_mode;

	ISP_DBG("%s:VFE%d ISP_EVENT_FE_READ_DONE buf_idx %d\n",
		__func__, vfe_dev->pdev->id, fetch_engine_info->buf_idx);
	fetch_engine_info->is_busy = 0;
	msm_isp_send_event(vfe_dev, ISP_EVENT_FE_READ_DONE, &fe_rd_done_event);
}

static int msm_isp_cfg_pix(struct vfe_device *vfe_dev,
	struct msm_vfe_input_cfg *input_cfg)
{
	int rc = 0;
	struct msm_vfe_pix_cfg *pix_cfg = NULL;

	if (vfe_dev->axi_data.src_info[VFE_PIX_0].active) {
		pr_err("%s: src %d path is active\n", __func__, VFE_PIX_0);
		return -EINVAL;
	}

	pix_cfg = &input_cfg->d.pix_cfg;
	vfe_dev->hvx_cmd = pix_cfg->hvx_cmd;
	vfe_dev->is_split = input_cfg->d.pix_cfg.is_split;

	vfe_dev->axi_data.src_info[VFE_PIX_0].pixel_clock =
		input_cfg->input_pix_clk;
	vfe_dev->axi_data.src_info[VFE_PIX_0].input_mux =
		input_cfg->d.pix_cfg.input_mux;
	vfe_dev->axi_data.src_info[VFE_PIX_0].input_format =
		input_cfg->d.pix_cfg.input_format;
	vfe_dev->axi_data.src_info[VFE_PIX_0].sof_counter_step = 1;

	/*
	 * Fill pixel_clock into input_pix_clk so that user space
	 * can use rounded clk rate
	 */
	input_cfg->input_pix_clk =
		vfe_dev->axi_data.src_info[VFE_PIX_0].pixel_clock;

	ISP_DBG("%s: input mux is %d CAMIF %d io_format 0x%x\n", __func__,
		input_cfg->d.pix_cfg.input_mux, CAMIF,
		input_cfg->d.pix_cfg.input_format);

	if (input_cfg->d.pix_cfg.input_mux == CAMIF ||
		input_cfg->d.pix_cfg.input_mux == TESTGEN) {
		if (input_cfg->d.pix_cfg.input_mux == CAMIF)
			vfe_dev->axi_data.src_info[VFE_PIX_0].width =
				input_cfg->d.pix_cfg.camif_cfg.pixels_per_line;
		if (input_cfg->d.pix_cfg.input_mux == TESTGEN)
			vfe_dev->axi_data.src_info[VFE_PIX_0].width =
			input_cfg->d.pix_cfg.testgen_cfg.pixels_per_line;
		if (input_cfg->d.pix_cfg.camif_cfg.subsample_cfg.
			sof_counter_step > 0) {
			vfe_dev->axi_data.src_info[VFE_PIX_0].
				sof_counter_step = input_cfg->d.pix_cfg.
				camif_cfg.subsample_cfg.sof_counter_step;
		}
	} else if (input_cfg->d.pix_cfg.input_mux == EXTERNAL_READ) {
		vfe_dev->axi_data.src_info[VFE_PIX_0].width =
			input_cfg->d.pix_cfg.fetch_engine_cfg.buf_stride;
	}
	vfe_dev->hw_info->vfe_ops.core_ops.cfg_input_mux(
			vfe_dev, &input_cfg->d.pix_cfg);
	vfe_dev->hw_info->vfe_ops.core_ops.reg_update(vfe_dev, VFE_PIX_0);
	return rc;
}

static int msm_isp_cfg_rdi(struct vfe_device *vfe_dev,
	struct msm_vfe_input_cfg *input_cfg)
{
	int rc = 0;

	if (vfe_dev->axi_data.src_info[input_cfg->input_src].active) {
		pr_err("%s: RAW%d path is active\n", __func__,
			   input_cfg->input_src - VFE_RAW_0);
		return -EINVAL;
	}

	vfe_dev->axi_data.
		src_info[input_cfg->input_src].sof_counter_step = 1;

	vfe_dev->axi_data.src_info[input_cfg->input_src].pixel_clock =
		input_cfg->input_pix_clk;
	vfe_dev->hw_info->vfe_ops.core_ops.cfg_rdi_reg(
		vfe_dev, &input_cfg->d.rdi_cfg, input_cfg->input_src);
	return rc;
}

int msm_isp_cfg_input(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0;
	struct msm_vfe_input_cfg *input_cfg = arg;
	long pixel_clock = 0;

	switch (input_cfg->input_src) {
	case VFE_PIX_0:
		rc = msm_isp_cfg_pix(vfe_dev, input_cfg);
		break;
	case VFE_RAW_0:
	case VFE_RAW_1:
	case VFE_RAW_2:
		rc = msm_isp_cfg_rdi(vfe_dev, input_cfg);
		break;
	default:
		pr_err("%s: Invalid input source\n", __func__);
		rc = -EINVAL;
	}

	pixel_clock = input_cfg->input_pix_clk;
	/*
	 * Only set rate to higher, do not lower higher
	 * rate needed by another input
	 */
	if (pixel_clock > vfe_dev->vfe_clk_info[
				vfe_dev->hw_info->vfe_clk_idx].clk_rate) {
		rc = vfe_dev->hw_info->vfe_ops.platform_ops.set_clk_rate(
			vfe_dev,
			&pixel_clock);
		if (rc < 0) {
			pr_err("%s: clock set rate failed\n", __func__);
			return rc;
		}
	}
	return rc;
}

static int msm_isp_dual_hw_master_slave_sync(struct vfe_device *vfe_dev,
						void *arg)
{
	int rc = 0;

	struct msm_isp_dual_hw_master_slave_sync *link = arg;
	unsigned long flags;
	struct master_slave_resource_info *ms_res =
			&vfe_dev->common_data->ms_resource;
	int i;
	struct msm_vfe_src_info *src_info = NULL;

	spin_lock_irqsave(
			&vfe_dev->common_data->common_dev_data_lock,
			flags);
	ms_res->dual_sync_mode = link->sync_mode;
	if (ms_res->dual_sync_mode == MSM_ISP_DUAL_CAM_ASYNC) {
		for (i = 0; i < MAX_VFE * VFE_SRC_MAX; i++) {
			if (ms_res->src_info[i] == NULL)
				continue;
			src_info = ms_res->src_info[i];
			if (src_info->dual_hw_ms_info.sync_state ==
				MSM_ISP_DUAL_CAM_ASYNC)
				continue;
			ms_res->active_src_mask &= ~(1 <<
				src_info->dual_hw_ms_info.index);
			ms_res->src_sof_mask &= ~(1 <<
				src_info->dual_hw_ms_info.index);
			src_info->dual_hw_ms_info.sync_state =
				MSM_ISP_DUAL_CAM_ASYNC;
		}
	}
	spin_unlock_irqrestore(
			&vfe_dev->common_data->common_dev_data_lock,
			flags);
	return rc;
}

static int msm_isp_set_dual_HW_master_slave_mode(
	struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0, i;
	struct msm_isp_set_dual_hw_ms_cmd *dual_hw_ms_cmd = NULL;
	struct msm_vfe_src_info *src_info = NULL;
	unsigned long flags;
	struct master_slave_resource_info *ms_res =
			&vfe_dev->common_data->ms_resource;

	if (!vfe_dev || !arg) {
		pr_err("%s: Error! Invalid input vfe_dev %pK arg %pK\n",
			__func__, vfe_dev, arg);
		return -EINVAL;
	}

	spin_lock_irqsave(&vfe_dev->common_data->common_dev_data_lock, flags);
	dual_hw_ms_cmd = (struct msm_isp_set_dual_hw_ms_cmd *)arg;
	vfe_dev->common_data->ms_resource.dual_hw_type = DUAL_HW_MASTER_SLAVE;
	vfe_dev->vfe_ub_policy = MSM_WM_UB_EQUAL_SLICING;
	if (dual_hw_ms_cmd->primary_intf < VFE_SRC_MAX) {
		ISP_DBG("%s: vfe %d primary_intf %d\n", __func__,
			vfe_dev->pdev->id, dual_hw_ms_cmd->primary_intf);
		src_info = &vfe_dev->axi_data.
			src_info[dual_hw_ms_cmd->primary_intf];
		src_info->dual_hw_type = DUAL_HW_MASTER_SLAVE;
		src_info->dual_hw_ms_info.dual_hw_ms_type =
			dual_hw_ms_cmd->dual_hw_ms_type;
		src_info->dual_hw_ms_info.index = dual_hw_ms_cmd->
			primary_intf + VFE_SRC_MAX * vfe_dev->pdev->id;
		ms_res->src_info[src_info->dual_hw_ms_info.index] = src_info;
		ms_res->num_src++;
		if (dual_hw_ms_cmd->dual_hw_ms_type == MS_TYPE_MASTER) {
			ms_res->master_index = src_info->dual_hw_ms_info.index;
			ms_res->sof_delta_threshold =
				dual_hw_ms_cmd->sof_delta_threshold;
		} else {
			ms_res->primary_slv_idx =
				src_info->dual_hw_ms_info.index;
		}
	}
	ISP_DBG("%s: vfe %d num_src %d\n", __func__, vfe_dev->pdev->id,
		dual_hw_ms_cmd->num_src);
	if (dual_hw_ms_cmd->num_src > VFE_SRC_MAX) {
		pr_err("%s: Error! Invalid num_src %d\n", __func__,
			dual_hw_ms_cmd->num_src);
		spin_unlock_irqrestore(&vfe_dev->common_data->
			common_dev_data_lock, flags);
		return -EINVAL;
	}
	/* This for loop is for non-primary intf to be marked with Master/Slave
	 * in order for frame id sync. But their timestamp is not saved.
	 * So no sof_info resource is allocated
	 */
	for (i = 0; i < dual_hw_ms_cmd->num_src; i++) {
		if (dual_hw_ms_cmd->input_src[i] >= VFE_SRC_MAX) {
			pr_err("%s: Error! Invalid SRC param %d\n", __func__,
				dual_hw_ms_cmd->input_src[i]);
			spin_unlock_irqrestore(&vfe_dev->common_data->
					common_dev_data_lock, flags);
			return -EINVAL;
		}
		ISP_DBG("%s: vfe %d src %d type %d\n", __func__,
			vfe_dev->pdev->id, dual_hw_ms_cmd->input_src[i],
			dual_hw_ms_cmd->dual_hw_ms_type);
		src_info = &vfe_dev->axi_data.
			src_info[dual_hw_ms_cmd->input_src[i]];
		src_info->dual_hw_type = DUAL_HW_MASTER_SLAVE;
		src_info->dual_hw_ms_info.dual_hw_ms_type =
			dual_hw_ms_cmd->dual_hw_ms_type;
		src_info->dual_hw_ms_info.index = dual_hw_ms_cmd->
			input_src[i] + VFE_SRC_MAX * vfe_dev->pdev->id;
		ms_res->src_info[src_info->dual_hw_ms_info.index] = src_info;
		ms_res->num_src++;
	}
	spin_unlock_irqrestore(&vfe_dev->common_data->common_dev_data_lock,
				flags);
	return rc;
}

static int msm_isp_proc_cmd_list_unlocked(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0;
	uint32_t count = 0;
	struct msm_vfe_cfg_cmd_list *proc_cmd =
		(struct msm_vfe_cfg_cmd_list *)arg;
	struct msm_vfe_cfg_cmd_list cmd, cmd_next;

	if (!vfe_dev || !arg) {
		pr_err("%s:%d failed: vfe_dev %pK arg %pK", __func__, __LINE__,
			vfe_dev, arg);
		return -EINVAL;
	}

	rc = msm_isp_proc_cmd(vfe_dev, &proc_cmd->cfg_cmd);
	if (rc < 0)
		pr_err("%s:%d failed: rc %d", __func__, __LINE__, rc);

	cmd = *proc_cmd;

	while (cmd.next) {
		if (cmd.next_size != sizeof(struct msm_vfe_cfg_cmd_list)) {
			pr_err("%s:%d failed: next size %u != expected %zu\n",
				__func__, __LINE__, cmd.next_size,
				sizeof(struct msm_vfe_cfg_cmd_list));
			break;
		}
		if (++count >= MAX_ISP_REG_LIST) {
			pr_err("%s:%d Error exceeding the max register count:%u\n",
				__func__, __LINE__, count);
			rc = -EINVAL;
			break;
		}
		if (copy_from_user(&cmd_next, (void __user *)cmd.next,
			sizeof(struct msm_vfe_cfg_cmd_list))) {
			rc = -EFAULT;
			continue;
		}

		rc = msm_isp_proc_cmd(vfe_dev, &cmd_next.cfg_cmd);
		if (rc < 0)
			pr_err("%s:%d failed: rc %d", __func__, __LINE__, rc);

		cmd = cmd_next;
	}
	return rc;
}

#ifdef CONFIG_COMPAT
struct msm_vfe_cfg_cmd2_32 {
	uint16_t num_cfg;
	uint16_t cmd_len;
	compat_caddr_t cfg_data;
	compat_caddr_t cfg_cmd;
};

struct msm_vfe_cfg_cmd_list_32 {
	struct msm_vfe_cfg_cmd2_32   cfg_cmd;
	compat_caddr_t               next;
	uint32_t                     next_size;
};

#define VIDIOC_MSM_VFE_REG_CFG_COMPAT \
	_IOWR('V', BASE_VIDIOC_PRIVATE, struct msm_vfe_cfg_cmd2_32)
#define VIDIOC_MSM_VFE_REG_LIST_CFG_COMPAT \
	_IOWR('V', BASE_VIDIOC_PRIVATE+14, struct msm_vfe_cfg_cmd_list_32)

static void msm_isp_compat_to_proc_cmd(struct msm_vfe_cfg_cmd2 *proc_cmd,
	struct msm_vfe_cfg_cmd2_32 *proc_cmd_ptr32)
{
	proc_cmd->num_cfg = proc_cmd_ptr32->num_cfg;
	proc_cmd->cmd_len = proc_cmd_ptr32->cmd_len;
	proc_cmd->cfg_data = compat_ptr(proc_cmd_ptr32->cfg_data);
	proc_cmd->cfg_cmd = compat_ptr(proc_cmd_ptr32->cfg_cmd);
}

static int msm_isp_proc_cmd_list_compat(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0;
	uint32_t count = 0;
	struct msm_vfe_cfg_cmd_list_32 *proc_cmd =
		(struct msm_vfe_cfg_cmd_list_32 *)arg;
	struct msm_vfe_cfg_cmd_list_32 cmd, cmd_next;
	struct msm_vfe_cfg_cmd2 current_cmd;

	if (!vfe_dev || !arg) {
		pr_err("%s:%d failed: vfe_dev %pK arg %pK", __func__, __LINE__,
			vfe_dev, arg);
		return -EINVAL;
	}
	msm_isp_compat_to_proc_cmd(&current_cmd, &proc_cmd->cfg_cmd);
	rc = msm_isp_proc_cmd(vfe_dev, &current_cmd);
	if (rc < 0)
		pr_err("%s:%d failed: rc %d", __func__, __LINE__, rc);

	cmd = *proc_cmd;

	while (compat_ptr(cmd.next) != NULL) {
		if (cmd.next_size != sizeof(struct msm_vfe_cfg_cmd_list_32)) {
			pr_err("%s:%d failed: next size %u != expected %zu\n",
				__func__, __LINE__, cmd.next_size,
				sizeof(struct msm_vfe_cfg_cmd_list));
			break;
		}
		if (++count >= MAX_ISP_REG_LIST) {
			pr_err("%s:%d Error exceeding the max register count:%u\n",
				__func__, __LINE__, count);
			rc = -EINVAL;
			break;
		}
		if (copy_from_user(&cmd_next, compat_ptr(cmd.next),
			sizeof(struct msm_vfe_cfg_cmd_list_32))) {
			rc = -EFAULT;
			continue;
		}

		msm_isp_compat_to_proc_cmd(&current_cmd, &cmd_next.cfg_cmd);
		rc = msm_isp_proc_cmd(vfe_dev, &current_cmd);
		if (rc < 0)
			pr_err("%s:%d failed: rc %d", __func__, __LINE__, rc);

		cmd = cmd_next;
	}
	return rc;
}

static int msm_isp_proc_cmd_list(struct vfe_device *vfe_dev, void *arg)
{
	if (is_compat_task())
		return msm_isp_proc_cmd_list_compat(vfe_dev, arg);
	else
		return msm_isp_proc_cmd_list_unlocked(vfe_dev, arg);
}
#else /* CONFIG_COMPAT */
static int msm_isp_proc_cmd_list(struct vfe_device *vfe_dev, void *arg)
{
	return msm_isp_proc_cmd_list_unlocked(vfe_dev, arg);
}
#endif /* CONFIG_COMPAT */

static long msm_isp_ioctl_unlocked(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long rc = 0;
	long rc2 = 0;
	struct vfe_device *vfe_dev = v4l2_get_subdevdata(sd);

	if (!vfe_dev || !vfe_dev->vfe_base) {
		pr_err("%s:%d failed: invalid params %pK\n",
			__func__, __LINE__, vfe_dev);
		if (vfe_dev)
			pr_err("%s:%d failed %pK\n", __func__,
				__LINE__, vfe_dev->vfe_base);
		return -EINVAL;
	}

	/* use real time mutex for hard real-time ioctls such as
	 * buffer operations and register updates.
	 * Use core mutex for other ioctls that could take
	 * longer time to complete such as start/stop ISP streams
	 * which blocks until the hardware start/stop streaming
	 */
	ISP_DBG("%s: cmd: %d\n", __func__, _IOC_TYPE(cmd));
	switch (cmd) {
	case VIDIOC_MSM_VFE_REG_CFG: {
		mutex_lock(&vfe_dev->realtime_mutex);
		rc = msm_isp_proc_cmd(vfe_dev, arg);
		mutex_unlock(&vfe_dev->realtime_mutex);
		break;
	}
	case VIDIOC_MSM_VFE_REG_LIST_CFG: {
		mutex_lock(&vfe_dev->realtime_mutex);
		rc = msm_isp_proc_cmd_list(vfe_dev, arg);
		mutex_unlock(&vfe_dev->realtime_mutex);
		break;
	}
	case VIDIOC_MSM_ISP_REQUEST_BUF:
	case VIDIOC_MSM_ISP_REQUEST_BUF_VER2:
		/* fallthrough */
	case VIDIOC_MSM_ISP_ENQUEUE_BUF:
		/* fallthrough */
	case VIDIOC_MSM_ISP_DEQUEUE_BUF:
		/* fallthrough */
	case VIDIOC_MSM_ISP_UNMAP_BUF: {
		mutex_lock(&vfe_dev->buf_mgr->lock);
		rc = msm_isp_proc_buf_cmd(vfe_dev->buf_mgr, cmd, arg);
		mutex_unlock(&vfe_dev->buf_mgr->lock);
		break;
	}
	case VIDIOC_MSM_ISP_RELEASE_BUF: {
		if (vfe_dev->buf_mgr == NULL) {
			pr_err("%s: buf mgr NULL! rc = -1\n", __func__);
			rc = -EINVAL;
			return rc;
		}
		mutex_lock(&vfe_dev->buf_mgr->lock);
		rc = msm_isp_proc_buf_cmd(vfe_dev->buf_mgr, cmd, arg);
		mutex_unlock(&vfe_dev->buf_mgr->lock);
		break;
	}
	case VIDIOC_MSM_ISP_REQUEST_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_request_axi_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_RELEASE_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_release_axi_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_CFG_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_cfg_axi_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_AXI_HALT:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_axi_halt(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_AXI_RESET:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		if (atomic_read(&vfe_dev->error_info.overflow_state)
			!= HALT_ENFORCED) {
			rc = msm_isp_stats_reset(vfe_dev);
			rc2 = msm_isp_axi_reset(vfe_dev, arg);
			if (!rc && rc2)
				rc = rc2;
		} else {
			pr_err_ratelimited("%s: no HW reset, halt enforced.\n",
				__func__);
		}
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_AXI_RESTART:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		mutex_lock(&vfe_dev->buf_mgr->lock);
		if (atomic_read(&vfe_dev->error_info.overflow_state)
			!= HALT_ENFORCED) {
			rc = msm_isp_stats_restart(vfe_dev);
			rc2 = msm_isp_axi_restart(vfe_dev, arg);
			if (!rc && rc2)
				rc = rc2;
		} else {
			pr_err_ratelimited("%s: no AXI restart, halt enforced.\n",
				__func__);
		}
		mutex_unlock(&vfe_dev->buf_mgr->lock);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_INPUT_CFG:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_cfg_input(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_AHB_CLK_CFG:
		mutex_lock(&vfe_dev->core_mutex);
		if (vfe_dev->hw_info->vfe_ops.core_ops.ahb_clk_cfg)
			rc = vfe_dev->hw_info->vfe_ops.core_ops.
					ahb_clk_cfg(vfe_dev, arg);
		else
			rc = -EOPNOTSUPP;
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_SET_DUAL_HW_MASTER_SLAVE:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_set_dual_HW_master_slave_mode(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_dual_hw_master_slave_sync(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_ab_ib_update_lpm_mode(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_FETCH_ENG_START:
	case VIDIOC_MSM_ISP_MAP_BUF_START_FE:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_start_fetch_engine(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;

	case VIDIOC_MSM_ISP_FETCH_ENG_MULTI_PASS_START:
	case VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_start_fetch_engine_multi_pass(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_REG_UPDATE_CMD:
		if (arg) {
			enum msm_vfe_input_src frame_src =
				*((enum msm_vfe_input_src *)arg);
			vfe_dev->hw_info->vfe_ops.core_ops.
				reg_update(vfe_dev, frame_src);
		}
		break;
	case VIDIOC_MSM_ISP_SET_SRC_STATE:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_set_src_state(vfe_dev, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_REQUEST_STATS_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_request_stats_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_RELEASE_STATS_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_release_stats_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_CFG_STATS_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_cfg_stats_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_UPDATE_STATS_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_update_stats_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_UPDATE_STREAM:
		mutex_lock(&vfe_dev->core_mutex);
		MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
		rc = msm_isp_update_axi_stream(vfe_dev, arg);
		MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case VIDIOC_MSM_ISP_SMMU_ATTACH:
		mutex_lock(&vfe_dev->core_mutex);
		rc = msm_isp_smmu_attach(vfe_dev->buf_mgr, arg);
		mutex_unlock(&vfe_dev->core_mutex);
		break;
	case MSM_SD_NOTIFY_FREEZE:
		vfe_dev->isp_sof_debug = 0;
		vfe_dev->isp_raw0_debug = 0;
		vfe_dev->isp_raw1_debug = 0;
		vfe_dev->isp_raw2_debug = 0;
		break;
	case MSM_SD_UNNOTIFY_FREEZE:
	case MSM_SD_SHUTDOWN:
		break;

	default:
		pr_err_ratelimited("%s: Invalid ISP command %d\n", __func__,
				    cmd);
		rc = -EINVAL;
	}
	return rc;
}


#ifdef CONFIG_COMPAT
static long msm_isp_ioctl_compat(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	struct vfe_device *vfe_dev = v4l2_get_subdevdata(sd);
	long rc = 0;

	if (!vfe_dev || !vfe_dev->vfe_base) {
		pr_err("%s:%d failed: invalid params %pK\n",
			__func__, __LINE__, vfe_dev);
		if (vfe_dev)
			pr_err("%s:%d failed %pK\n", __func__,
				__LINE__, vfe_dev->vfe_base);
		return -EINVAL;
	}

	switch (cmd) {
	case VIDIOC_MSM_VFE_REG_CFG_COMPAT: {
		struct msm_vfe_cfg_cmd2 proc_cmd;

		mutex_lock(&vfe_dev->realtime_mutex);
		msm_isp_compat_to_proc_cmd(&proc_cmd,
			(struct msm_vfe_cfg_cmd2_32 *) arg);
		rc = msm_isp_proc_cmd(vfe_dev, &proc_cmd);
		mutex_unlock(&vfe_dev->realtime_mutex);
		break;
	}
	case VIDIOC_MSM_VFE_REG_LIST_CFG_COMPAT: {
		mutex_lock(&vfe_dev->realtime_mutex);
		rc = msm_isp_proc_cmd_list(vfe_dev, arg);
		mutex_unlock(&vfe_dev->realtime_mutex);
		break;
	}
	default:
		return msm_isp_ioctl_unlocked(sd, cmd, arg);
	}

	return rc;
}

long msm_isp_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	return msm_isp_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long msm_isp_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	return msm_isp_ioctl_unlocked(sd, cmd, arg);
}
#endif /* CONFIG_COMPAT */

static int msm_isp_send_hw_cmd(struct vfe_device *vfe_dev,
	struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
	uint32_t *cfg_data, uint32_t cmd_len)
{
	if (!vfe_dev || !reg_cfg_cmd) {
		pr_err("%s:%d failed: vfe_dev %pK reg_cfg_cmd %pK\n", __func__,
			__LINE__, vfe_dev, reg_cfg_cmd);
		return -EINVAL;
	}
	if ((reg_cfg_cmd->cmd_type != VFE_CFG_MASK) &&
		(!cfg_data || !cmd_len)) {
		pr_err("%s:%d failed: cmd type %d cfg_data %pK cmd_len %d\n",
			__func__, __LINE__, reg_cfg_cmd->cmd_type, cfg_data,
			cmd_len);
		return -EINVAL;
	}

	/* Validate input parameters */
	switch (reg_cfg_cmd->cmd_type) {
	case VFE_WRITE:
	case VFE_READ:
	case VFE_WRITE_MB: {
		if ((reg_cfg_cmd->u.rw_info.reg_offset >
			(UINT_MAX - reg_cfg_cmd->u.rw_info.len)) ||
			((reg_cfg_cmd->u.rw_info.reg_offset +
			reg_cfg_cmd->u.rw_info.len) >
			vfe_dev->vfe_base_size) ||
			(reg_cfg_cmd->u.rw_info.reg_offset & 0x3)) {
			pr_err_ratelimited("%s:%d regoffset %d len %d res %d\n",
				__func__, __LINE__,
				reg_cfg_cmd->u.rw_info.reg_offset,
				reg_cfg_cmd->u.rw_info.len,
				(uint32_t)vfe_dev->vfe_base_size);
			return -EINVAL;
		}

		if ((reg_cfg_cmd->u.rw_info.cmd_data_offset >
			(UINT_MAX - reg_cfg_cmd->u.rw_info.len)) ||
			((reg_cfg_cmd->u.rw_info.cmd_data_offset +
			reg_cfg_cmd->u.rw_info.len) > cmd_len)) {
			pr_err_ratelimited("%s:%d cmd_data_offset %d len %d cmd_len %d\n",
				__func__, __LINE__,
				reg_cfg_cmd->u.rw_info.cmd_data_offset,
				reg_cfg_cmd->u.rw_info.len, cmd_len);
			return -EINVAL;
		}
		break;
	}

	case VFE_WRITE_DMI_16BIT:
	case VFE_WRITE_DMI_32BIT:
	case VFE_WRITE_DMI_64BIT:
	case VFE_READ_DMI_16BIT:
	case VFE_READ_DMI_32BIT:
	case VFE_READ_DMI_64BIT: {
		if (reg_cfg_cmd->cmd_type == VFE_WRITE_DMI_64BIT ||
			reg_cfg_cmd->cmd_type == VFE_READ_DMI_64BIT) {
			if ((reg_cfg_cmd->u.dmi_info.hi_tbl_offset <=
				reg_cfg_cmd->u.dmi_info.lo_tbl_offset) ||
				(reg_cfg_cmd->u.dmi_info.hi_tbl_offset -
				reg_cfg_cmd->u.dmi_info.lo_tbl_offset !=
				(sizeof(uint32_t)))) {
				pr_err("%s:%d hi %d lo %d\n",
					__func__, __LINE__,
					reg_cfg_cmd->u.dmi_info.hi_tbl_offset,
					reg_cfg_cmd->u.dmi_info.hi_tbl_offset);
				return -EINVAL;
			}
			if (reg_cfg_cmd->u.dmi_info.len <= sizeof(uint32_t)) {
				pr_err("%s:%d len %d\n",
					__func__, __LINE__,
					reg_cfg_cmd->u.dmi_info.len);
				return -EINVAL;
			}
			if (((UINT_MAX -
				reg_cfg_cmd->u.dmi_info.hi_tbl_offset) <
				(reg_cfg_cmd->u.dmi_info.len -
				sizeof(uint32_t))) ||
				((reg_cfg_cmd->u.dmi_info.hi_tbl_offset +
				reg_cfg_cmd->u.dmi_info.len -
				sizeof(uint32_t)) > cmd_len)) {
				pr_err("%s:%d hi_tbl_offset %d len %d cmd %d\n",
					__func__, __LINE__,
					reg_cfg_cmd->u.dmi_info.hi_tbl_offset,
					reg_cfg_cmd->u.dmi_info.len, cmd_len);
				return -EINVAL;
			}
		}
		if ((reg_cfg_cmd->u.dmi_info.lo_tbl_offset >
			(UINT_MAX - reg_cfg_cmd->u.dmi_info.len)) ||
			((reg_cfg_cmd->u.dmi_info.lo_tbl_offset +
			reg_cfg_cmd->u.dmi_info.len) > cmd_len)) {
			pr_err("%s:%d lo_tbl_offset %d len %d cmd_len %d\n",
				__func__, __LINE__,
				reg_cfg_cmd->u.dmi_info.lo_tbl_offset,
				reg_cfg_cmd->u.dmi_info.len, cmd_len);
			return -EINVAL;
		}
		break;
	}

	default:
		break;
	}

	switch (reg_cfg_cmd->cmd_type) {
	case VFE_WRITE: {
		msm_camera_io_memcpy(vfe_dev->vfe_base +
			reg_cfg_cmd->u.rw_info.reg_offset,
			cfg_data + reg_cfg_cmd->u.rw_info.cmd_data_offset/4,
			reg_cfg_cmd->u.rw_info.len);
		break;
	}
	case VFE_WRITE_MB: {
		msm_camera_io_memcpy_mb(vfe_dev->vfe_base +
			reg_cfg_cmd->u.rw_info.reg_offset,
			cfg_data + reg_cfg_cmd->u.rw_info.cmd_data_offset/4,
			reg_cfg_cmd->u.rw_info.len);
		break;
	}
	case VFE_CFG_MASK: {
		uint32_t temp;
		bool grab_lock;
		unsigned long flags;

		if ((UINT_MAX - sizeof(temp) <
			reg_cfg_cmd->u.mask_info.reg_offset) ||
			(vfe_dev->vfe_base_size <
			reg_cfg_cmd->u.mask_info.reg_offset +
			sizeof(temp)) ||
			(reg_cfg_cmd->u.mask_info.reg_offset & 0x3)) {
			pr_err("%s: VFE_CFG_MASK: Invalid length\n", __func__);
			return -EINVAL;
		}
		grab_lock = vfe_dev->hw_info->vfe_ops.core_ops.
			is_module_cfg_lock_needed(reg_cfg_cmd->
			u.mask_info.reg_offset);
		if (grab_lock)
			spin_lock_irqsave(&vfe_dev->shared_data_lock, flags);
		else
			__acquire(&vfe_dev->shared_data_lock);
		temp = msm_camera_io_r(vfe_dev->vfe_base +
			reg_cfg_cmd->u.mask_info.reg_offset);

		temp &= ~reg_cfg_cmd->u.mask_info.mask;
		temp |= reg_cfg_cmd->u.mask_info.val;
		msm_camera_io_w(temp, vfe_dev->vfe_base +
			reg_cfg_cmd->u.mask_info.reg_offset);
		if (grab_lock)
			spin_unlock_irqrestore(&vfe_dev->shared_data_lock,
				flags);
		else
			__release(&vfe_dev->shared_data_lock);
		break;
	}
	case VFE_WRITE_DMI_16BIT:
	case VFE_WRITE_DMI_32BIT:
	case VFE_WRITE_DMI_64BIT: {
		int i;
		uint32_t *hi_tbl_ptr = NULL, *lo_tbl_ptr = NULL;
		uint32_t hi_val, lo_val, lo_val1;

		if (reg_cfg_cmd->cmd_type == VFE_WRITE_DMI_64BIT) {
			hi_tbl_ptr = cfg_data +
				reg_cfg_cmd->u.dmi_info.hi_tbl_offset/4;
		}
		lo_tbl_ptr = cfg_data +
			reg_cfg_cmd->u.dmi_info.lo_tbl_offset/4;
		if (reg_cfg_cmd->cmd_type == VFE_WRITE_DMI_64BIT)
			reg_cfg_cmd->u.dmi_info.len =
				reg_cfg_cmd->u.dmi_info.len / 2;
		for (i = 0; i < reg_cfg_cmd->u.dmi_info.len/4; i++) {
			lo_val = *lo_tbl_ptr++;
			if (reg_cfg_cmd->cmd_type == VFE_WRITE_DMI_16BIT) {
				lo_val1 = lo_val & 0x0000FFFF;
				lo_val = (lo_val & 0xFFFF0000)>>16;
				msm_camera_io_w(lo_val1, vfe_dev->vfe_base +
					vfe_dev->hw_info->dmi_reg_offset + 0x4);
			} else if (reg_cfg_cmd->cmd_type ==
					   VFE_WRITE_DMI_64BIT) {
				lo_tbl_ptr++;
				hi_val = *hi_tbl_ptr;
				hi_tbl_ptr = hi_tbl_ptr + 2;
				msm_camera_io_w(hi_val, vfe_dev->vfe_base +
					vfe_dev->hw_info->dmi_reg_offset);
			}
			msm_camera_io_w(lo_val, vfe_dev->vfe_base +
				vfe_dev->hw_info->dmi_reg_offset + 0x4);
		}
		break;
	}
	case VFE_READ_DMI_16BIT:
	case VFE_READ_DMI_32BIT:
	case VFE_READ_DMI_64BIT: {
		int i;
		uint32_t *hi_tbl_ptr = NULL, *lo_tbl_ptr = NULL;
		uint32_t hi_val, lo_val, lo_val1;

		if (reg_cfg_cmd->cmd_type == VFE_READ_DMI_64BIT) {
			hi_tbl_ptr = cfg_data +
				reg_cfg_cmd->u.dmi_info.hi_tbl_offset/4;
		}

		lo_tbl_ptr = cfg_data +
			reg_cfg_cmd->u.dmi_info.lo_tbl_offset/4;

		if (reg_cfg_cmd->cmd_type == VFE_READ_DMI_64BIT)
			reg_cfg_cmd->u.dmi_info.len =
				reg_cfg_cmd->u.dmi_info.len / 2;

		for (i = 0; i < reg_cfg_cmd->u.dmi_info.len/4; i++) {
			lo_val = msm_camera_io_r(vfe_dev->vfe_base +
					vfe_dev->hw_info->dmi_reg_offset + 0x4);

			if (reg_cfg_cmd->cmd_type == VFE_READ_DMI_16BIT) {
				lo_val1 = msm_camera_io_r(vfe_dev->vfe_base +
					vfe_dev->hw_info->dmi_reg_offset + 0x4);
				lo_val |= lo_val1 << 16;
			}
			*lo_tbl_ptr++ = lo_val;
			if (reg_cfg_cmd->cmd_type == VFE_READ_DMI_64BIT) {
				hi_val = msm_camera_io_r(vfe_dev->vfe_base +
					vfe_dev->hw_info->dmi_reg_offset);
				*hi_tbl_ptr = hi_val;
				hi_tbl_ptr += 2;
				lo_tbl_ptr++;
			}
		}
		break;
	}
	case VFE_HW_UPDATE_LOCK: {
		uint32_t update_id =
			vfe_dev->axi_data.src_info[VFE_PIX_0].last_updt_frm_id;
		if (vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id != *cfg_data
			|| update_id == *cfg_data) {
			pr_err("%s hw update lock failed acq %d, cur id %u, last id %u\n",
				__func__,
				*cfg_data,
				vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id,
				update_id);
			return -EINVAL;
		}
		break;
	}
	case VFE_HW_UPDATE_UNLOCK: {
		if (vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id
			!= *cfg_data) {
			pr_err("hw update across frame boundary,begin id %u, end id %d\n",
				*cfg_data,
				vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id);
		}
		vfe_dev->axi_data.src_info[VFE_PIX_0].last_updt_frm_id =
			vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id;
		break;
	}
	case VFE_READ: {
		int i;
		uint32_t *data_ptr = cfg_data +
			reg_cfg_cmd->u.rw_info.cmd_data_offset/4;
		for (i = 0; i < reg_cfg_cmd->u.rw_info.len/4; i++) {
			if ((data_ptr < cfg_data) ||
				(UINT_MAX / sizeof(*data_ptr) <
				 (data_ptr - cfg_data)) ||
				(sizeof(*data_ptr) * (data_ptr - cfg_data) >=
				 cmd_len))
				return -EINVAL;
			*data_ptr++ = msm_camera_io_r(vfe_dev->vfe_base +
				reg_cfg_cmd->u.rw_info.reg_offset);
			reg_cfg_cmd->u.rw_info.reg_offset += 4;
		}
		break;
	}
	case GET_MAX_CLK_RATE: {
		int rc = 0;
		unsigned long rate;

		if (cmd_len != sizeof(__u32)) {
			pr_err("%s:%d failed: invalid cmd len %u exp %zu\n",
				__func__, __LINE__, cmd_len,
				sizeof(__u32));
			return -EINVAL;
		}
		rc = vfe_dev->hw_info->vfe_ops.platform_ops.get_max_clk_rate(
							vfe_dev, &rate);
		if (rc < 0) {
			pr_err("%s:%d failed: rc %d\n", __func__, __LINE__, rc);
			return -EINVAL;
		}

		*(__u32 *)cfg_data = (__u32)rate;

		break;
	}
	case GET_CLK_RATES: {
		int rc = 0;
		struct msm_isp_clk_rates rates;
		struct msm_isp_clk_rates *user_data =
			(struct msm_isp_clk_rates *)cfg_data;
		if (cmd_len != sizeof(struct msm_isp_clk_rates)) {
			pr_err("%s:%d failed: invalid cmd len %u exp %zu\n",
				__func__, __LINE__, cmd_len,
				sizeof(struct msm_isp_clk_rates));
			return -EINVAL;
		}
		rc = vfe_dev->hw_info->vfe_ops.platform_ops.get_clk_rates(
							vfe_dev, &rates);
		if (rc < 0) {
			pr_err("%s:%d failed: rc %d\n", __func__, __LINE__, rc);
			return -EINVAL;
		}
		user_data->svs_rate = rates.svs_rate;
		user_data->nominal_rate = rates.nominal_rate;
		user_data->high_rate = rates.high_rate;
		break;
	}
	case GET_ISP_ID: {
		uint32_t *isp_id = NULL;

		if (cmd_len < sizeof(uint32_t)) {
			pr_err("%s:%d failed: invalid cmd len %u exp %zu\n",
				__func__, __LINE__, cmd_len,
				sizeof(uint32_t));
			return -EINVAL;
		}

		isp_id = (uint32_t *)cfg_data;
		*isp_id = vfe_dev->pdev->id;
		break;
	}
	case SET_WM_UB_SIZE:
		break;
	case SET_UB_POLICY: {

		if (cmd_len < sizeof(vfe_dev->vfe_ub_policy)) {
			pr_err("%s:%d failed: invalid cmd len %u exp %zu\n",
				__func__, __LINE__, cmd_len,
				sizeof(vfe_dev->vfe_ub_policy));
			return -EINVAL;
		}
		vfe_dev->vfe_ub_policy = *cfg_data;
		break;
	}
	case GET_VFE_HW_LIMIT: {
		uint32_t *hw_limit = NULL;

		if (cmd_len < sizeof(uint32_t)) {
			pr_err("%s:%d failed: invalid cmd len %u exp %zu\n",
				__func__, __LINE__, cmd_len,
				sizeof(uint32_t));
			return -EINVAL;
		}

		hw_limit = (uint32_t *)cfg_data;
		*hw_limit = vfe_dev->vfe_hw_limit;
		break;
	}
	}
	return 0;
}

int msm_isp_proc_cmd(struct vfe_device *vfe_dev, void *arg)
{
	int rc = 0, i;
	struct msm_vfe_cfg_cmd2 *proc_cmd = arg;
	struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd;
	uint32_t *cfg_data = NULL;

	if (!proc_cmd->num_cfg) {
		pr_err("%s: Passed num_cfg as 0\n", __func__);
		return -EINVAL;
	}

	reg_cfg_cmd = kzalloc(sizeof(struct msm_vfe_reg_cfg_cmd)*
		proc_cmd->num_cfg, GFP_KERNEL);
	if (!reg_cfg_cmd) {
		rc = -ENOMEM;
		goto reg_cfg_failed;
	}

	if (copy_from_user(reg_cfg_cmd,
		(void __user *)(proc_cmd->cfg_cmd),
		sizeof(struct msm_vfe_reg_cfg_cmd) * proc_cmd->num_cfg)) {
		rc = -EFAULT;
		goto copy_cmd_failed;
	}

	if (proc_cmd->cmd_len > 0) {
		cfg_data = kzalloc(proc_cmd->cmd_len, GFP_KERNEL);
		if (!cfg_data) {
			pr_err("%s: cfg_data alloc failed\n", __func__);
			rc = -ENOMEM;
			goto cfg_data_failed;
		}

		if (copy_from_user(cfg_data,
			(void __user *)(proc_cmd->cfg_data),
			proc_cmd->cmd_len)) {
			rc = -EFAULT;
			goto copy_cmd_failed;
		}
	}

	for (i = 0; i < proc_cmd->num_cfg; i++)
		rc = msm_isp_send_hw_cmd(vfe_dev, &reg_cfg_cmd[i],
			cfg_data, proc_cmd->cmd_len);

	if (copy_to_user(proc_cmd->cfg_data,
			cfg_data, proc_cmd->cmd_len)) {
		rc = -EFAULT;
		goto copy_cmd_failed;
	}

copy_cmd_failed:
	kfree(cfg_data);
cfg_data_failed:
	kfree(reg_cfg_cmd);
reg_cfg_failed:
	return rc;
}

int msm_isp_send_event(struct vfe_device *vfe_dev,
	uint32_t event_type,
	struct msm_isp_event_data *event_data)
{
	struct v4l2_event isp_event;

	memset(&isp_event, 0, sizeof(struct v4l2_event));
	isp_event.id = 0;
	isp_event.type = event_type;
	memcpy(&isp_event.u.data[0], event_data,
		sizeof(struct msm_isp_event_data));
	v4l2_event_queue(vfe_dev->subdev.sd.devnode, &isp_event);
	return 0;
}

#define CAL_WORD(width, M, N) ((width * M + N - 1) / N)

int msm_isp_cal_word_per_line(uint32_t output_format,
	uint32_t pixel_per_line)
{
	int val = -1;

	switch (output_format) {
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
	case V4L2_PIX_FMT_QBGGR8:
	case V4L2_PIX_FMT_QGBRG8:
	case V4L2_PIX_FMT_QGRBG8:
	case V4L2_PIX_FMT_QRGGB8:
	case V4L2_PIX_FMT_JPEG:
	case V4L2_PIX_FMT_META:
	case V4L2_PIX_FMT_GREY:
		val = CAL_WORD(pixel_per_line, 1, 8);
		break;
	case V4L2_PIX_FMT_SBGGR10:
	case V4L2_PIX_FMT_SGBRG10:
	case V4L2_PIX_FMT_SGRBG10:
	case V4L2_PIX_FMT_SRGGB10:
	case V4L2_PIX_FMT_SBGGR10DPCM6:
	case V4L2_PIX_FMT_SGBRG10DPCM6:
	case V4L2_PIX_FMT_SGRBG10DPCM6:
	case V4L2_PIX_FMT_SRGGB10DPCM6:
	case V4L2_PIX_FMT_SBGGR10DPCM8:
	case V4L2_PIX_FMT_SGBRG10DPCM8:
	case V4L2_PIX_FMT_SGRBG10DPCM8:
	case V4L2_PIX_FMT_SRGGB10DPCM8:
	case V4L2_PIX_FMT_META10:
		val = CAL_WORD(pixel_per_line, 5, 32);
		break;
	case V4L2_PIX_FMT_SBGGR12:
	case V4L2_PIX_FMT_SGBRG12:
	case V4L2_PIX_FMT_SGRBG12:
	case V4L2_PIX_FMT_SRGGB12:
		val = CAL_WORD(pixel_per_line, 3, 16);
		break;
	case V4L2_PIX_FMT_SBGGR14:
	case V4L2_PIX_FMT_SGBRG14:
	case V4L2_PIX_FMT_SGRBG14:
	case V4L2_PIX_FMT_SRGGB14:
		val = CAL_WORD(pixel_per_line, 7, 32);
		break;
	case V4L2_PIX_FMT_QBGGR10:
	case V4L2_PIX_FMT_QGBRG10:
	case V4L2_PIX_FMT_QGRBG10:
	case V4L2_PIX_FMT_QRGGB10:
		val = CAL_WORD(pixel_per_line, 1, 6);
		break;
	case V4L2_PIX_FMT_QBGGR12:
	case V4L2_PIX_FMT_QGBRG12:
	case V4L2_PIX_FMT_QGRBG12:
	case V4L2_PIX_FMT_QRGGB12:
		val = CAL_WORD(pixel_per_line, 1, 5);
		break;
	case V4L2_PIX_FMT_QBGGR14:
	case V4L2_PIX_FMT_QGBRG14:
	case V4L2_PIX_FMT_QGRBG14:
	case V4L2_PIX_FMT_QRGGB14:
		val = CAL_WORD(pixel_per_line, 1, 4);
		break;
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV14:
	case V4L2_PIX_FMT_NV41:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
		val = CAL_WORD(pixel_per_line, 1, 8);
		break;
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
		val = CAL_WORD(pixel_per_line, 2, 8);
	break;
	case V4L2_PIX_FMT_P16BGGR10:
	case V4L2_PIX_FMT_P16GBRG10:
	case V4L2_PIX_FMT_P16GRBG10:
	case V4L2_PIX_FMT_P16RGGB10:
	case V4L2_PIX_FMT_P16BGGR12:
	case V4L2_PIX_FMT_P16GBRG12:
	case V4L2_PIX_FMT_P16GRBG12:
	case V4L2_PIX_FMT_P16RGGB12:
		val = CAL_WORD(pixel_per_line, 1, 4);
	break;
	case V4L2_PIX_FMT_NV24:
	case V4L2_PIX_FMT_NV42:
		val = CAL_WORD(pixel_per_line, 1, 8);
	break;
		/*TD: Add more image format*/
	default:
		msm_isp_print_fourcc_error(__func__, output_format);
		break;
	}
	return val;
}

enum msm_isp_pack_fmt msm_isp_get_pack_format(uint32_t output_format)
{
	switch (output_format) {
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
	case V4L2_PIX_FMT_SBGGR10:
	case V4L2_PIX_FMT_SGBRG10:
	case V4L2_PIX_FMT_SGRBG10:
	case V4L2_PIX_FMT_SRGGB10:
	case V4L2_PIX_FMT_SBGGR10DPCM6:
	case V4L2_PIX_FMT_SGBRG10DPCM6:
	case V4L2_PIX_FMT_SGRBG10DPCM6:
	case V4L2_PIX_FMT_SRGGB10DPCM6:
	case V4L2_PIX_FMT_SBGGR10DPCM8:
	case V4L2_PIX_FMT_SGBRG10DPCM8:
	case V4L2_PIX_FMT_SGRBG10DPCM8:
	case V4L2_PIX_FMT_SRGGB10DPCM8:
	case V4L2_PIX_FMT_SBGGR12:
	case V4L2_PIX_FMT_SGBRG12:
	case V4L2_PIX_FMT_SGRBG12:
	case V4L2_PIX_FMT_SRGGB12:
	case V4L2_PIX_FMT_SBGGR14:
	case V4L2_PIX_FMT_SGBRG14:
	case V4L2_PIX_FMT_SGRBG14:
	case V4L2_PIX_FMT_SRGGB14:
		return MIPI;
	case V4L2_PIX_FMT_QBGGR8:
	case V4L2_PIX_FMT_QGBRG8:
	case V4L2_PIX_FMT_QGRBG8:
	case V4L2_PIX_FMT_QRGGB8:
	case V4L2_PIX_FMT_QBGGR10:
	case V4L2_PIX_FMT_QGBRG10:
	case V4L2_PIX_FMT_QGRBG10:
	case V4L2_PIX_FMT_QRGGB10:
	case V4L2_PIX_FMT_QBGGR12:
	case V4L2_PIX_FMT_QGBRG12:
	case V4L2_PIX_FMT_QGRBG12:
	case V4L2_PIX_FMT_QRGGB12:
	case V4L2_PIX_FMT_QBGGR14:
	case V4L2_PIX_FMT_QGBRG14:
	case V4L2_PIX_FMT_QGRBG14:
	case V4L2_PIX_FMT_QRGGB14:
		return QCOM;
	case V4L2_PIX_FMT_P16BGGR10:
	case V4L2_PIX_FMT_P16GBRG10:
	case V4L2_PIX_FMT_P16GRBG10:
	case V4L2_PIX_FMT_P16RGGB10:
	case V4L2_PIX_FMT_P16BGGR12:
	case V4L2_PIX_FMT_P16GBRG12:
	case V4L2_PIX_FMT_P16GRBG12:
	case V4L2_PIX_FMT_P16RGGB12:
		return PLAIN16;
	default:
		msm_isp_print_fourcc_error(__func__, output_format);
		break;
	}
	return -EINVAL;
}

int msm_isp_get_bit_per_pixel(uint32_t output_format)
{
	switch (output_format) {
	case V4L2_PIX_FMT_Y4:
		return 4;
	case V4L2_PIX_FMT_Y6:
		return 6;
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
	case V4L2_PIX_FMT_QBGGR8:
	case V4L2_PIX_FMT_QGBRG8:
	case V4L2_PIX_FMT_QGRBG8:
	case V4L2_PIX_FMT_QRGGB8:
	case V4L2_PIX_FMT_JPEG:
	case V4L2_PIX_FMT_META:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV14:
	case V4L2_PIX_FMT_NV41:
	case V4L2_PIX_FMT_YVU410:
	case V4L2_PIX_FMT_YVU420:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YYUV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_YUV422P:
	case V4L2_PIX_FMT_YUV411P:
	case V4L2_PIX_FMT_Y41P:
	case V4L2_PIX_FMT_YUV444:
	case V4L2_PIX_FMT_YUV555:
	case V4L2_PIX_FMT_YUV565:
	case V4L2_PIX_FMT_YUV32:
	case V4L2_PIX_FMT_YUV410:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_GREY:
	case V4L2_PIX_FMT_PAL8:
	case V4L2_PIX_FMT_UV8:
	case MSM_V4L2_PIX_FMT_META:
		return 8;
	case V4L2_PIX_FMT_SBGGR10:
	case V4L2_PIX_FMT_SGBRG10:
	case V4L2_PIX_FMT_SGRBG10:
	case V4L2_PIX_FMT_SRGGB10:
	case V4L2_PIX_FMT_SBGGR10DPCM6:
	case V4L2_PIX_FMT_SGBRG10DPCM6:
	case V4L2_PIX_FMT_SGRBG10DPCM6:
	case V4L2_PIX_FMT_SRGGB10DPCM6:
	case V4L2_PIX_FMT_SBGGR10DPCM8:
	case V4L2_PIX_FMT_SGBRG10DPCM8:
	case V4L2_PIX_FMT_SGRBG10DPCM8:
	case V4L2_PIX_FMT_SRGGB10DPCM8:
	case V4L2_PIX_FMT_QBGGR10:
	case V4L2_PIX_FMT_QGBRG10:
	case V4L2_PIX_FMT_QGRBG10:
	case V4L2_PIX_FMT_QRGGB10:
	case V4L2_PIX_FMT_Y10:
	case V4L2_PIX_FMT_Y10BPACK:
	case V4L2_PIX_FMT_P16BGGR10:
	case V4L2_PIX_FMT_P16GBRG10:
	case V4L2_PIX_FMT_P16GRBG10:
	case V4L2_PIX_FMT_P16RGGB10:
	case V4L2_PIX_FMT_META10:
	case MSM_V4L2_PIX_FMT_META10:
		return 10;
	case V4L2_PIX_FMT_SBGGR12:
	case V4L2_PIX_FMT_SGBRG12:
	case V4L2_PIX_FMT_SGRBG12:
	case V4L2_PIX_FMT_SRGGB12:
	case V4L2_PIX_FMT_QBGGR12:
	case V4L2_PIX_FMT_QGBRG12:
	case V4L2_PIX_FMT_QGRBG12:
	case V4L2_PIX_FMT_QRGGB12:
	case V4L2_PIX_FMT_Y12:
	case V4L2_PIX_FMT_P16BGGR12:
	case V4L2_PIX_FMT_P16GBRG12:
	case V4L2_PIX_FMT_P16GRBG12:
	case V4L2_PIX_FMT_P16RGGB12:
		return 12;
	case V4L2_PIX_FMT_SBGGR14:
	case V4L2_PIX_FMT_SGBRG14:
	case V4L2_PIX_FMT_SGRBG14:
	case V4L2_PIX_FMT_SRGGB14:
	case V4L2_PIX_FMT_QBGGR14:
	case V4L2_PIX_FMT_QGBRG14:
	case V4L2_PIX_FMT_QGRBG14:
	case V4L2_PIX_FMT_QRGGB14:
		return 14;
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
	case V4L2_PIX_FMT_Y16:
		return 16;
	case V4L2_PIX_FMT_NV24:
	case V4L2_PIX_FMT_NV42:
		return 24;
		/*TD: Add more image format*/
	default:
		msm_isp_print_fourcc_error(__func__, output_format);
		pr_err("%s: Invalid output format %x\n",
			__func__, output_format);
		return -EINVAL;
	}
}

void msm_isp_update_error_frame_count(struct vfe_device *vfe_dev)
{
	struct msm_vfe_error_info *error_info = &vfe_dev->error_info;

	error_info->info_dump_frame_count++;
}


static int msm_isp_process_iommu_page_fault(struct vfe_device *vfe_dev)
{
	int rc = vfe_dev->buf_mgr->pagefault_debug_disable;
	uint32_t irq_status0, irq_status1;
	uint32_t overflow_mask;
	unsigned long irq_flags;

	/* Check if any overflow bit is set */
	vfe_dev->hw_info->vfe_ops.core_ops.
		get_overflow_mask(&overflow_mask);
	vfe_dev->hw_info->vfe_ops.irq_ops.
		read_irq_status(vfe_dev, &irq_status0, &irq_status1);
	overflow_mask &= irq_status1;
	spin_lock_irqsave(
		&vfe_dev->common_data->common_dev_data_lock, irq_flags);
	if (overflow_mask ||
		atomic_read(&vfe_dev->error_info.overflow_state) !=
			NO_OVERFLOW) {
		spin_unlock_irqrestore(
			&vfe_dev->common_data->common_dev_data_lock, irq_flags);
		pr_err_ratelimited("%s: overflow detected during IOMMU\n",
			__func__);
		/* Don't treat the Overflow + Page fault scenario as fatal.
		 * Instead try to do a recovery. Using an existing event as
		 * as opposed to creating a new event.
		 */
		msm_isp_halt_send_error(vfe_dev, ISP_EVENT_PING_PONG_MISMATCH);
	} else {
		spin_unlock_irqrestore(
			&vfe_dev->common_data->common_dev_data_lock, irq_flags);
		pr_err("%s:%d] VFE%d Handle Page fault! vfe_dev %pK\n",
			__func__, __LINE__,  vfe_dev->pdev->id, vfe_dev);
		vfe_dev->hw_info->vfe_ops.axi_ops.halt(vfe_dev, 0);
		msm_isp_halt_send_error(vfe_dev, ISP_EVENT_IOMMU_P_FAULT);
	}

	if (vfe_dev->buf_mgr->pagefault_debug_disable == 0) {
		vfe_dev->buf_mgr->pagefault_debug_disable = 1;
		vfe_dev->buf_mgr->ops->buf_mgr_debug(vfe_dev->buf_mgr,
			vfe_dev->page_fault_addr);
		msm_isp_print_ping_pong_address(vfe_dev,
			vfe_dev->page_fault_addr);
		vfe_dev->hw_info->vfe_ops.axi_ops.
			read_wm_ping_pong_addr(vfe_dev);
	}
	return rc;
}

void msm_isp_process_error_info(struct vfe_device *vfe_dev)
{
	struct msm_vfe_error_info *error_info = &vfe_dev->error_info;

	if (error_info->error_count == 1 ||
		!(error_info->info_dump_frame_count % 100)) {
		vfe_dev->hw_info->vfe_ops.core_ops.
			process_error_status(vfe_dev);
		error_info->error_mask0 = 0;
		error_info->error_mask1 = 0;
		error_info->camif_status = 0;
		error_info->violation_status = 0;
	}
}

static inline void msm_isp_update_error_info(struct vfe_device *vfe_dev,
	uint32_t error_mask0, uint32_t error_mask1)
{
	vfe_dev->error_info.error_mask0 |= error_mask0;
	vfe_dev->error_info.error_mask1 |= error_mask1;
	vfe_dev->error_info.error_count++;
}

int msm_isp_process_overflow_irq(
	struct vfe_device *vfe_dev,
	uint32_t *irq_status0, uint32_t *irq_status1,
	uint8_t force_overflow)
{
	uint32_t overflow_mask;
	uint32_t bus_err = 0;
	unsigned long flags;

	/* if there are no active streams - do not start recovery */
	if (!vfe_dev->axi_data.num_active_stream)
		return 0;

	if (vfe_dev->hw_info->vfe_ops.core_ops.
		get_bus_err_mask)
		vfe_dev->hw_info->vfe_ops.core_ops.get_bus_err_mask(
			vfe_dev, &bus_err, irq_status1);
	/* Mask out all other irqs if recovery is started */
	if (atomic_read(&vfe_dev->error_info.overflow_state) != NO_OVERFLOW) {
		uint32_t halt_restart_mask0, halt_restart_mask1;

		vfe_dev->hw_info->vfe_ops.core_ops.
		get_halt_restart_mask(&halt_restart_mask0,
			&halt_restart_mask1);
		*irq_status0 &= halt_restart_mask0;
		*irq_status1 &= halt_restart_mask1;

		return 0;
	}

	/* Check if any overflow bit is set */
	vfe_dev->hw_info->vfe_ops.core_ops.
		get_overflow_mask(&overflow_mask);
	overflow_mask &= *irq_status1;

	if (overflow_mask || force_overflow) {
		struct msm_isp_event_data error_event;
		int i;
		struct msm_vfe_axi_shared_data *axi_data = &vfe_dev->axi_data;

		spin_lock_irqsave(
			&vfe_dev->common_data->common_dev_data_lock, flags);

		if (atomic_cmpxchg(&vfe_dev->error_info.overflow_state,
				NO_OVERFLOW, OVERFLOW_DETECTED)) {
			spin_unlock_irqrestore(
				 &vfe_dev->common_data->common_dev_data_lock,
				 flags);
			return 0;
		}

		if (vfe_dev->reset_pending == 1) {
			pr_err_ratelimited("%s:%d overflow %x during reset\n",
				__func__, __LINE__, overflow_mask);
			/* Clear overflow bits since reset is pending */
			*irq_status1 &= ~overflow_mask;
			spin_unlock_irqrestore(
				 &vfe_dev->common_data->common_dev_data_lock,
				 flags);
			return 0;
		}
		pr_err_ratelimited("%s: vfe %d overflowmask %x,bus_error %x\n",
			__func__, vfe_dev->pdev->id, overflow_mask, bus_err);
		for (i = 0; i < axi_data->hw_info->num_wm; i++) {
			if (!axi_data->free_wm[i])
				continue;
			ISP_DBG("%s:wm %d assigned to stream handle %x\n",
				__func__, i, axi_data->free_wm[i]);
		}
		vfe_dev->recovery_irq0_mask = vfe_dev->irq0_mask;
		vfe_dev->recovery_irq1_mask = vfe_dev->irq1_mask;
		vfe_dev->hw_info->vfe_ops.core_ops.
				set_halt_restart_mask(vfe_dev);
		vfe_dev->hw_info->vfe_ops.axi_ops.halt(vfe_dev, 0);
		/* mask off other vfe if dual vfe is used */
		if (vfe_dev->is_split) {
			int other_vfe_id;
			struct vfe_device *temp_vfe;

			other_vfe_id = (vfe_dev->pdev->id == ISP_VFE0) ?
				ISP_VFE1 : ISP_VFE0;
			temp_vfe = vfe_dev->common_data->
				dual_vfe_res->vfe_dev[other_vfe_id];

			atomic_set(&temp_vfe->error_info.overflow_state,
				OVERFLOW_DETECTED);
			temp_vfe->recovery_irq0_mask = temp_vfe->irq0_mask;
			temp_vfe->recovery_irq1_mask = temp_vfe->irq1_mask;
			temp_vfe->hw_info->vfe_ops.core_ops.
				set_halt_restart_mask(temp_vfe);
			temp_vfe->hw_info->vfe_ops.axi_ops.halt(temp_vfe, 0);
		}

		/* reset irq status so skip further process */
		*irq_status0 = 0;
		*irq_status1 = 0;

		if (atomic_read(&vfe_dev->error_info.overflow_state)
			!= HALT_ENFORCED) {
			memset(&error_event, 0, sizeof(error_event));
			error_event.frame_id =
				vfe_dev->axi_data.src_info[VFE_PIX_0].frame_id;
			error_event.u.error_info.err_type =
				ISP_ERROR_BUS_OVERFLOW;
			msm_isp_send_event(vfe_dev,
				ISP_EVENT_ERROR, &error_event);
		}
		spin_unlock_irqrestore(
			&vfe_dev->common_data->common_dev_data_lock,
			flags);
		return 1;
	}
	return 0;
}

void msm_isp_reset_burst_count_and_frame_drop(
	struct vfe_device *vfe_dev, struct msm_vfe_axi_stream *stream_info)
{
	if (stream_info->state != ACTIVE ||
		stream_info->stream_type != BURST_STREAM) {
		return;
	}
	if (stream_info->num_burst_capture != 0)
		msm_isp_reset_framedrop(vfe_dev, stream_info);
}

void msm_isp_prepare_irq_debug_info(struct vfe_device *vfe_dev,
	uint32_t irq_status0, uint32_t irq_status1)
{

	unsigned long flags;
	struct msm_vfe_irq_debug_info *irq_debug;
	uint8_t current_index;

	spin_lock_irqsave(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_irq_dump_lock, flags);
	/* Fill current VFE debug info */
	current_index = vfe_dev->common_data->vfe_irq_dump.
		current_irq_index % MAX_VFE_IRQ_DEBUG_DUMP_SIZE;
	irq_debug = &vfe_dev->common_data->vfe_irq_dump.
		irq_debug[current_index];
	irq_debug->vfe_id = vfe_dev->pdev->id;
	irq_debug->core_id = smp_processor_id();
	msm_isp_get_timestamp(&irq_debug->ts, vfe_dev);
	irq_debug->irq_status0[vfe_dev->pdev->id] = irq_status0;
	irq_debug->irq_status1[vfe_dev->pdev->id] = irq_status1;
	irq_debug->ping_pong_status[vfe_dev->pdev->id] =
		vfe_dev->hw_info->vfe_ops.axi_ops.
			get_pingpong_status(vfe_dev);
	if (vfe_dev->is_split &&
		(vfe_dev->common_data->
		dual_vfe_res->vfe_dev[!vfe_dev->pdev->id])
		&& (vfe_dev->common_data->dual_vfe_res->
		vfe_dev[!vfe_dev->pdev->id]->vfe_open_cnt)) {
		/* Fill other VFE debug Info */
		vfe_dev->hw_info->vfe_ops.irq_ops.read_irq_status(
			vfe_dev->common_data->dual_vfe_res->
			vfe_dev[!vfe_dev->pdev->id],
			&irq_debug->irq_status0[!vfe_dev->pdev->id],
			&irq_debug->irq_status1[!vfe_dev->pdev->id]);
		irq_debug->ping_pong_status[!vfe_dev->pdev->id] =
			vfe_dev->hw_info->vfe_ops.axi_ops.
			get_pingpong_status(vfe_dev->common_data->
			dual_vfe_res->vfe_dev[!vfe_dev->pdev->id]);
	}
	vfe_dev->common_data->vfe_irq_dump.current_irq_index++;
	spin_unlock_irqrestore(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_irq_dump_lock, flags);
}

void msm_isp_prepare_tasklet_debug_info(struct vfe_device *vfe_dev,
	uint32_t irq_status0, uint32_t irq_status1,
	struct msm_isp_timestamp ts)
{
	struct msm_vfe_irq_debug_info *irq_debug;
	uint8_t current_index;
	unsigned long flags;

	spin_lock_irqsave(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_tasklet_dump_lock, flags);
	current_index = vfe_dev->common_data->vfe_irq_dump.
		current_tasklet_index % MAX_VFE_IRQ_DEBUG_DUMP_SIZE;
	irq_debug = &vfe_dev->common_data->vfe_irq_dump.
		tasklet_debug[current_index];
	irq_debug->vfe_id = vfe_dev->pdev->id;
	irq_debug->core_id = smp_processor_id();
	irq_debug->ts = ts;
	irq_debug->irq_status0[vfe_dev->pdev->id] = irq_status0;
	irq_debug->irq_status1[vfe_dev->pdev->id] = irq_status1;
	irq_debug->ping_pong_status[vfe_dev->pdev->id] =
		vfe_dev->hw_info->vfe_ops.axi_ops.
		get_pingpong_status(vfe_dev);
	vfe_dev->common_data->vfe_irq_dump.current_tasklet_index++;
	spin_unlock_irqrestore(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_tasklet_dump_lock, flags);
}

static void msm_isp_enqueue_tasklet_cmd(struct vfe_device *vfe_dev,
	uint32_t irq_status0, uint32_t irq_status1,
	uint32_t ping_pong_status)
{
	unsigned long flags;
	struct msm_vfe_tasklet_queue_cmd *queue_cmd = NULL;
	struct msm_vfe_tasklet *tasklet;

	if (vfe_dev->is_split)
		tasklet = &vfe_dev->common_data->tasklets[MAX_VFE];
	else
		tasklet = &vfe_dev->common_data->tasklets[vfe_dev->pdev->id];

	spin_lock_irqsave(&tasklet->tasklet_lock, flags);
	queue_cmd = &tasklet->tasklet_queue_cmd[tasklet->taskletq_idx];
	if (queue_cmd->cmd_used) {
		pr_err_ratelimited("%s: Tasklet queue overflow: %d\n",
			__func__, vfe_dev->pdev->id);
		spin_unlock_irqrestore(&tasklet->tasklet_lock, flags);
		return;
	}
	atomic_add(1, &vfe_dev->irq_cnt);
	queue_cmd->vfeInterruptStatus0 = irq_status0;
	queue_cmd->vfeInterruptStatus1 = irq_status1;
	queue_cmd->vfe_pingpong_status = ping_pong_status;
	msm_isp_get_timestamp(&queue_cmd->ts, vfe_dev);
	queue_cmd->cmd_used = 1;
	queue_cmd->vfe_dev = vfe_dev;

	tasklet->taskletq_idx = (tasklet->taskletq_idx + 1) %
		MSM_VFE_TASKLETQ_SIZE;
	list_add_tail(&queue_cmd->list, &tasklet->tasklet_q);
	spin_unlock_irqrestore(&tasklet->tasklet_lock, flags);
	tasklet_schedule(&tasklet->tasklet);
}

irqreturn_t msm_isp_process_irq(int irq_num, void *data)
{
	struct vfe_device *vfe_dev = (struct vfe_device *) data;
	uint32_t irq_status0, irq_status1, ping_pong_status;
	uint32_t error_mask0, error_mask1;

	vfe_dev->hw_info->vfe_ops.irq_ops.
		read_and_clear_irq_status(vfe_dev, &irq_status0, &irq_status1);

	if ((irq_status0 == 0) && (irq_status1 == 0)) {
		ISP_DBG("%s:VFE%d irq_status0 & 1 are both 0\n",
			__func__, vfe_dev->pdev->id);
		return IRQ_HANDLED;
	}
	ping_pong_status = vfe_dev->hw_info->vfe_ops.axi_ops.
		get_pingpong_status(vfe_dev);
	if (vfe_dev->hw_info->vfe_ops.irq_ops.preprocess_camif_irq) {
		vfe_dev->hw_info->vfe_ops.irq_ops.preprocess_camif_irq(
				vfe_dev, irq_status0);
	}
	if (msm_isp_process_overflow_irq(vfe_dev,
		&irq_status0, &irq_status1, 0)) {
		/* if overflow initiated no need to handle the interrupts */
		pr_err("overflow processed\n");
		return IRQ_HANDLED;
	}

	vfe_dev->hw_info->vfe_ops.core_ops.
		get_error_mask(&error_mask0, &error_mask1);
	error_mask0 &= irq_status0;
	error_mask1 &= irq_status1;
	irq_status0 &= ~error_mask0;
	irq_status1 &= ~error_mask1;
	if ((error_mask0 != 0) || (error_mask1 != 0))
		msm_isp_update_error_info(vfe_dev, error_mask0, error_mask1);

	if ((irq_status0 == 0) && (irq_status1 == 0) &&
		(!(((error_mask0 != 0) || (error_mask1 != 0)) &&
		 vfe_dev->error_info.error_count == 1))) {
		ISP_DBG("%s: error_mask0/1 & error_count are set!\n", __func__);
		return IRQ_HANDLED;
	}
	msm_isp_prepare_irq_debug_info(vfe_dev, irq_status0, irq_status1);
	msm_isp_enqueue_tasklet_cmd(vfe_dev, irq_status0, irq_status1,
					ping_pong_status);

	return IRQ_HANDLED;
}

void msm_isp_do_tasklet(unsigned long data)
{
	unsigned long flags;
	struct msm_vfe_tasklet *tasklet = (struct msm_vfe_tasklet *)data;
	struct vfe_device *vfe_dev;
	struct msm_vfe_irq_ops *irq_ops;
	struct msm_vfe_tasklet_queue_cmd *queue_cmd;
	struct msm_isp_timestamp ts;
	uint32_t irq_status0, irq_status1, pingpong_status;

	while (1) {
		spin_lock_irqsave(&tasklet->tasklet_lock, flags);
		queue_cmd = list_first_entry_or_null(&tasklet->tasklet_q,
			struct msm_vfe_tasklet_queue_cmd, list);
		if (!queue_cmd) {
			spin_unlock_irqrestore(&tasklet->tasklet_lock, flags);
			break;
		}
		list_del_init(&queue_cmd->list);
		vfe_dev = queue_cmd->vfe_dev;
		queue_cmd->cmd_used = 0;
		queue_cmd->vfe_dev = NULL;
		irq_status0 = queue_cmd->vfeInterruptStatus0;
		irq_status1 = queue_cmd->vfeInterruptStatus1;
		pingpong_status = queue_cmd->vfe_pingpong_status;
		ts = queue_cmd->ts;
		spin_unlock_irqrestore(&tasklet->tasklet_lock, flags);
		if (vfe_dev->vfe_open_cnt == 0) {
			pr_err("%s: VFE%d open cnt = %d, irq %x/%x\n",
			__func__, vfe_dev->pdev->id, vfe_dev->vfe_open_cnt,
			irq_status0, irq_status1);
			continue;
		}
		atomic_sub(1, &vfe_dev->irq_cnt);
		msm_isp_prepare_tasklet_debug_info(vfe_dev,
			irq_status0, irq_status1, ts);
		irq_ops = &vfe_dev->hw_info->vfe_ops.irq_ops;
		irq_ops->process_reset_irq(vfe_dev,
			irq_status0, irq_status1);
		irq_ops->process_halt_irq(vfe_dev,
			irq_status0, irq_status1);
		if (atomic_read(&vfe_dev->error_info.overflow_state)
			!= NO_OVERFLOW) {
			ISP_DBG("%s: Recovery in processing, Ignore IRQs!!!\n",
				__func__);
			continue;
		}
		msm_isp_process_error_info(vfe_dev);
		irq_ops->process_stats_irq(vfe_dev,
			irq_status0, irq_status1,
			pingpong_status, &ts);
		irq_ops->process_axi_irq(vfe_dev,
			irq_status0, irq_status1,
			pingpong_status, &ts);
		irq_ops->process_camif_irq(vfe_dev,
			irq_status0, irq_status1, &ts);
		irq_ops->process_reg_update(vfe_dev,
			irq_status0, irq_status1, &ts);
		irq_ops->process_epoch_irq(vfe_dev,
			irq_status0, irq_status1, &ts);
	}
}

int msm_isp_set_src_state(struct vfe_device *vfe_dev, void *arg)
{
	struct msm_vfe_axi_src_state *src_state = arg;

	if (src_state->input_src >= VFE_SRC_MAX)
		return -EINVAL;
	vfe_dev->axi_data.src_info[src_state->input_src].active =
	src_state->src_active;
	vfe_dev->axi_data.src_info[src_state->input_src].frame_id =
	src_state->src_frame_id;
	return 0;
}

static void msm_vfe_iommu_fault_handler(struct iommu_domain *domain,
	struct device *dev, unsigned long iova, int flags, void *token)
{
	struct vfe_device *vfe_dev = NULL;

	if (token) {
		vfe_dev = (struct vfe_device *)token;
		vfe_dev->page_fault_addr = iova;
		if (!vfe_dev->buf_mgr || !vfe_dev->buf_mgr->ops ||
			!vfe_dev->axi_data.num_active_stream) {
			pr_err("%s:%d buf_mgr %pK active strms %d\n", __func__,
				__LINE__, vfe_dev->buf_mgr,
				vfe_dev->axi_data.num_active_stream);
			goto end;
		}

		mutex_lock(&vfe_dev->core_mutex);
		if (vfe_dev->vfe_open_cnt > 0) {
			pr_err_ratelimited("%s: fault address is %lx\n",
				__func__, iova);
			msm_isp_process_iommu_page_fault(vfe_dev);
		} else {
			pr_err("%s: no handling, vfe open cnt = %d\n",
				__func__, vfe_dev->vfe_open_cnt);
		}
		mutex_unlock(&vfe_dev->core_mutex);
	} else {
		ISP_DBG("%s:%d] no token received: %pK\n",
			__func__, __LINE__, token);
		goto end;
	}
end:
	return;
}

int msm_isp_open_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct vfe_device *vfe_dev = v4l2_get_subdevdata(sd);
	long rc = 0;

	ISP_DBG("%s open_cnt %u\n", __func__, vfe_dev->vfe_open_cnt);

	if (vfe_dev->common_data == NULL ||
		vfe_dev->common_data->dual_vfe_res == NULL) {
		pr_err("%s: Error in probe. No common_data or dual vfe res\n",
			__func__);
		return -EINVAL;
	}

	mutex_lock(&vfe_dev->realtime_mutex);
	mutex_lock(&vfe_dev->core_mutex);

	if (vfe_dev->vfe_open_cnt++) {
		mutex_unlock(&vfe_dev->core_mutex);
		mutex_unlock(&vfe_dev->realtime_mutex);
		return 0;
	}

	vfe_dev->reset_pending = 0;
	vfe_dev->isp_sof_debug = 0;
	vfe_dev->isp_raw0_debug = 0;
	vfe_dev->isp_raw1_debug = 0;
	vfe_dev->isp_raw2_debug = 0;

	if (vfe_dev->hw_info->vfe_ops.core_ops.init_hw(vfe_dev) < 0) {
		pr_err("%s: init hardware failed\n", __func__);
		vfe_dev->vfe_open_cnt--;
		mutex_unlock(&vfe_dev->core_mutex);
		mutex_unlock(&vfe_dev->realtime_mutex);
		return -EBUSY;
	}

	memset(&vfe_dev->error_info, 0, sizeof(vfe_dev->error_info));
	atomic_set(&vfe_dev->error_info.overflow_state, NO_OVERFLOW);

	vfe_dev->hw_info->vfe_ops.core_ops.clear_status_reg(vfe_dev);

	vfe_dev->vfe_hw_version = msm_camera_io_r(vfe_dev->vfe_base);
	ISP_DBG("%s: HW Version: 0x%x\n", __func__, vfe_dev->vfe_hw_version);
	rc = vfe_dev->hw_info->vfe_ops.core_ops.reset_hw(vfe_dev, 1, 1);
	if (rc <= 0) {
		pr_err("%s: reset timeout\n", __func__);
		vfe_dev->hw_info->vfe_ops.core_ops.release_hw(vfe_dev);
		vfe_dev->vfe_open_cnt--;
		mutex_unlock(&vfe_dev->core_mutex);
		mutex_unlock(&vfe_dev->realtime_mutex);
		return -EINVAL;
	}

	vfe_dev->hw_info->vfe_ops.core_ops.init_hw_reg(vfe_dev);

	vfe_dev->buf_mgr->ops->buf_mgr_init(vfe_dev->buf_mgr,
		"msm_isp");

	memset(&vfe_dev->axi_data, 0, sizeof(struct msm_vfe_axi_shared_data));
	memset(&vfe_dev->stats_data, 0,
		sizeof(struct msm_vfe_stats_shared_data));
	memset(&vfe_dev->error_info, 0, sizeof(vfe_dev->error_info));
	memset(&vfe_dev->fetch_engine_info, 0,
		sizeof(vfe_dev->fetch_engine_info));
	vfe_dev->axi_data.hw_info = vfe_dev->hw_info->axi_hw_info;
	vfe_dev->axi_data.enable_frameid_recovery = 0;
	vfe_dev->vt_enable = 0;
	vfe_dev->reg_update_requested = 0;
	/* Register page fault handler */
	vfe_dev->buf_mgr->pagefault_debug_disable = 0;
	/* initialize pd_buf_idx with an invalid index 0xF */
	vfe_dev->common_data->pd_buf_idx = 0xF;

	cam_smmu_reg_client_page_fault_handler(
			vfe_dev->buf_mgr->iommu_hdl,
			msm_vfe_iommu_fault_handler,
			NULL,
			vfe_dev);
	mutex_unlock(&vfe_dev->core_mutex);
	mutex_unlock(&vfe_dev->realtime_mutex);
	return 0;
}

#ifdef CONFIG_MSM_AVTIMER
static void msm_isp_end_avtimer(void)
{
	msm_isp_stop_avtimer();
}
#else
static void msm_isp_end_avtimer(void)
{
	pr_err("AV Timer is not supported\n");
}
#endif

int msm_isp_close_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	long rc = 0;
	int wm;
	int i;
	struct vfe_device *vfe_dev = v4l2_get_subdevdata(sd);

	ISP_DBG("%s E open_cnt %u\n", __func__, vfe_dev->vfe_open_cnt);
	mutex_lock(&vfe_dev->realtime_mutex);
	mutex_lock(&vfe_dev->core_mutex);

	if (!vfe_dev->vfe_open_cnt) {
		pr_err("%s invalid state open cnt %d\n", __func__,
			vfe_dev->vfe_open_cnt);
		mutex_unlock(&vfe_dev->core_mutex);
		mutex_unlock(&vfe_dev->realtime_mutex);
		return -EINVAL;
	}

	if (vfe_dev->vfe_open_cnt > 1) {
		vfe_dev->vfe_open_cnt--;
		mutex_unlock(&vfe_dev->core_mutex);
		mutex_unlock(&vfe_dev->realtime_mutex);
		return 0;
	}
	MSM_ISP_DUAL_VFE_MUTEX_LOCK(vfe_dev);
	msm_isp_release_all_axi_stream(vfe_dev);
	msm_isp_release_all_stats_stream(vfe_dev);

	/* Unregister page fault handler */
	cam_smmu_reg_client_page_fault_handler(
		vfe_dev->buf_mgr->iommu_hdl,
		NULL, NULL, vfe_dev);

	rc = vfe_dev->hw_info->vfe_ops.axi_ops.halt(vfe_dev, 1);
	if (rc <= 0)
		pr_err("%s: halt timeout rc=%ld\n", __func__, rc);

	vfe_dev->hw_info->vfe_ops.core_ops.
		update_camif_state(vfe_dev, DISABLE_CAMIF_IMMEDIATELY);
	vfe_dev->hw_info->vfe_ops.core_ops.reset_hw(vfe_dev, 0, 0);

	/* put scratch buf in all the wm */
	for (wm = 0; wm < vfe_dev->axi_data.hw_info->num_wm; wm++) {
		msm_isp_cfg_wm_scratch(vfe_dev, wm, VFE_PING_FLAG);
		msm_isp_cfg_wm_scratch(vfe_dev, wm, VFE_PONG_FLAG);
	}
	vfe_dev->hw_info->vfe_ops.core_ops.release_hw(vfe_dev);
	/* after regular hw stop, reduce open cnt */
	vfe_dev->vfe_open_cnt--;
	vfe_dev->buf_mgr->ops->buf_mgr_deinit(vfe_dev->buf_mgr);
	if (vfe_dev->vt_enable) {
		msm_isp_end_avtimer();
		vfe_dev->vt_enable = 0;
	}
	for (i = 0; i < VFE_SRC_MAX; i++)
		vfe_dev->axi_data.src_info[i].lpm = 0;
	MSM_ISP_DUAL_VFE_MUTEX_UNLOCK(vfe_dev);
	vfe_dev->is_split = 0;

	mutex_unlock(&vfe_dev->core_mutex);
	mutex_unlock(&vfe_dev->realtime_mutex);
	return 0;
}

void msm_isp_flush_tasklet(struct vfe_device *vfe_dev)
{
	unsigned long flags;
	int i;
	struct msm_vfe_tasklet_queue_cmd *queue_cmd, *q_cmd_next;
	struct msm_vfe_tasklet *tasklet;

	for (i = 0; i <= MAX_VFE; i++) {
		if (i != vfe_dev->pdev->id &&
			i != MAX_VFE)
			continue;
		tasklet = &vfe_dev->common_data->tasklets[i];
		spin_lock_irqsave(&tasklet->tasklet_lock, flags);
		list_for_each_entry_safe(queue_cmd, q_cmd_next,
			&tasklet->tasklet_q, list) {
			if (queue_cmd->vfe_dev != vfe_dev)
				continue;
			list_del_init(&queue_cmd->list);
			queue_cmd->cmd_used = 0;
		}
		spin_unlock_irqrestore(&tasklet->tasklet_lock, flags);
		tasklet_kill(&tasklet->tasklet);
	}
	atomic_set(&vfe_dev->irq_cnt, 0);

}

void msm_isp_irq_debug_dump(struct vfe_device *vfe_dev)
{

	uint8_t i, dump_index;
	unsigned long flags;

	spin_lock_irqsave(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_irq_dump_lock, flags);
	dump_index = vfe_dev->common_data->vfe_irq_dump.
		current_irq_index;
	for (i = 0; i < MAX_VFE_IRQ_DEBUG_DUMP_SIZE; i++) {
		trace_msm_cam_ping_pong_debug_dump(
			vfe_dev->common_data->vfe_irq_dump.
			irq_debug[dump_index % MAX_VFE_IRQ_DEBUG_DUMP_SIZE]);
		dump_index++;
	}
	spin_unlock_irqrestore(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_irq_dump_lock, flags);
}


void msm_isp_tasklet_debug_dump(struct vfe_device *vfe_dev)
{

	uint8_t i, dump_index;
	unsigned long flags;

	spin_lock_irqsave(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_tasklet_dump_lock, flags);
	dump_index = vfe_dev->common_data->vfe_irq_dump.
		current_tasklet_index;
	for (i = 0; i < MAX_VFE_IRQ_DEBUG_DUMP_SIZE; i++) {
		trace_msm_cam_tasklet_debug_dump(
			vfe_dev->common_data->vfe_irq_dump.
			tasklet_debug[
			dump_index % MAX_VFE_IRQ_DEBUG_DUMP_SIZE]);
		dump_index++;
	}
	spin_unlock_irqrestore(&vfe_dev->common_data->vfe_irq_dump.
		common_dev_tasklet_dump_lock, flags);
}

void msm_isp_dump_ping_pong_mismatch(struct vfe_device *vfe_dev)
{

	trace_msm_cam_string(" ***** msm_isp_dump_irq_debug ****");
	msm_isp_irq_debug_dump(vfe_dev);
	trace_msm_cam_string(" ***** msm_isp_dump_taskelet_debug ****");
	msm_isp_tasklet_debug_dump(vfe_dev);
}
