/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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

#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <soc/qcom/subsystem_restart.h>
#include <asm/div64.h>
#include "msm_vidc_common.h"
#include "vidc_hfi_api.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_dcvs.h"

#define IS_ALREADY_IN_STATE(__p, __d) ({\
	int __rc = (__p >= __d);\
	__rc; \
})

#define IS_VALID_DCVS_SESSION(__cur_mbpf, __min_mbpf) \
		((__cur_mbpf) >= (__min_mbpf))

#define SUM_ARRAY(__arr, __start, __end) ({\
		int __index;\
		typeof((__arr)[0]) __sum = 0;\
		for (__index = (__start); __index <= (__end); __index++) {\
			if (__index >= 0 && __index < ARRAY_SIZE(__arr))\
				__sum += __arr[__index];\
		} \
		__sum;\
})

#define V4L2_EVENT_SEQ_CHANGED_SUFFICIENT \
		V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_SUFFICIENT
#define V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT \
		V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT
#define V4L2_EVENT_RELEASE_BUFFER_REFERENCE \
		V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE

#define IS_SESSION_CMD_VALID(cmd) (((cmd) >= SESSION_MSG_START) && \
		((cmd) <= SESSION_MSG_END))
#define IS_SYS_CMD_VALID(cmd) (((cmd) >= SYS_MSG_START) && \
		((cmd) <= SYS_MSG_END))

struct getprop_buf {
	struct list_head list;
	void *data;
};

static void msm_comm_generate_session_error(struct msm_vidc_inst *inst);
static void msm_comm_generate_sys_error(struct msm_vidc_inst *inst);
static void handle_session_error(enum command_response cmd, void *data);

bool msm_comm_turbo_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_TURBO);
}

static inline bool is_thumbnail_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_THUMBNAIL);
}

static inline bool is_nominal_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_NOMINAL);
}

static inline bool is_non_realtime_session(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct v4l2_control ctrl = {
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY
	};
	rc = v4l2_g_ctrl(&inst->ctrl_handler, &ctrl);
	return (!rc && ctrl.value);
}

enum multi_stream msm_comm_get_stream_output_mode(struct msm_vidc_inst *inst)
{
	if (inst->session_type == MSM_VIDC_DECODER) {
		int rc = 0;
		struct v4l2_control ctrl = {
			.id = V4L2_CID_MPEG_VIDC_VIDEO_STREAM_OUTPUT_MODE
		};
		rc = v4l2_g_ctrl(&inst->ctrl_handler, &ctrl);
		if (!rc && ctrl.value)
			return HAL_VIDEO_DECODER_SECONDARY;
	}
	return HAL_VIDEO_DECODER_PRIMARY;
}

static int msm_comm_get_mbs_per_sec(struct msm_vidc_inst *inst)
{
	int output_port_mbs, capture_port_mbs;
	int fps, rc;
	struct v4l2_control ctrl;

	output_port_mbs = NUM_MBS_PER_FRAME(inst->prop.width[OUTPUT_PORT],
		inst->prop.height[OUTPUT_PORT]);
	capture_port_mbs = NUM_MBS_PER_FRAME(inst->prop.width[CAPTURE_PORT],
		inst->prop.height[CAPTURE_PORT]);

	ctrl.id = V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE;
	rc = v4l2_g_ctrl(&inst->ctrl_handler, &ctrl);
	if (!rc && ctrl.value) {
		fps = (ctrl.value >> 16)? ctrl.value >> 16: 1;
		return max(output_port_mbs, capture_port_mbs) * fps;
	} else
		return max(output_port_mbs, capture_port_mbs) * inst->prop.fps;
}

int msm_comm_get_inst_load(struct msm_vidc_inst *inst,
		enum load_calc_quirks quirks)
{
	int load = 0;

	if (!(inst->state >= MSM_VIDC_OPEN_DONE &&
		inst->state < MSM_VIDC_STOP_DONE))
		return 0;

	load = msm_comm_get_mbs_per_sec(inst);

	if (is_thumbnail_session(inst)) {
		if (quirks & LOAD_CALC_IGNORE_THUMBNAIL_LOAD)
			load = 0;
	}

	if (msm_comm_turbo_session(inst)) {
		if (!(quirks & LOAD_CALC_IGNORE_TURBO_LOAD))
			load = inst->core->resources.max_load;
	}

	if (is_non_realtime_session(inst) &&
		(quirks & LOAD_CALC_IGNORE_NON_REALTIME_LOAD))
		load = msm_comm_get_mbs_per_sec(inst) / inst->prop.fps;
	return load;
}

int msm_comm_get_load(struct msm_vidc_core *core,
	enum session_type type, enum load_calc_quirks quirks)
{
	struct msm_vidc_inst *inst = NULL;
	int num_mbs_per_sec = 0;

	if (!core) {
		dprintk(VIDC_ERR, "Invalid args: %p\n", core);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (inst->session_type != type)
			continue;

		num_mbs_per_sec += msm_comm_get_inst_load(inst, quirks);
	}
	mutex_unlock(&core->lock);

	return num_mbs_per_sec;
}

static enum hal_domain get_hal_domain(int session_type)
{
	enum hal_domain domain;
	switch (session_type) {
	case MSM_VIDC_ENCODER:
		domain = HAL_VIDEO_DOMAIN_ENCODER;
		break;
	case MSM_VIDC_DECODER:
		domain = HAL_VIDEO_DOMAIN_DECODER;
		break;
	default:
		dprintk(VIDC_ERR, "Wrong domain\n");
		domain = HAL_UNUSED_DOMAIN;
		break;
	}

	return domain;
}

enum hal_video_codec get_hal_codec_type(int fourcc)
{
	enum hal_video_codec codec;
	dprintk(VIDC_DBG, "codec is 0x%x\n", fourcc);
	switch (fourcc) {
	case V4L2_PIX_FMT_H264:
	case V4L2_PIX_FMT_H264_NO_SC:
		codec = HAL_VIDEO_CODEC_H264;
		break;
	case V4L2_PIX_FMT_H264_MVC:
		codec = HAL_VIDEO_CODEC_MVC;
		break;
	case V4L2_PIX_FMT_H263:
		codec = HAL_VIDEO_CODEC_H263;
		break;
	case V4L2_PIX_FMT_MPEG1:
		codec = HAL_VIDEO_CODEC_MPEG1;
		break;
	case V4L2_PIX_FMT_MPEG2:
		codec = HAL_VIDEO_CODEC_MPEG2;
		break;
	case V4L2_PIX_FMT_MPEG4:
		codec = HAL_VIDEO_CODEC_MPEG4;
		break;
	case V4L2_PIX_FMT_VC1_ANNEX_G:
	case V4L2_PIX_FMT_VC1_ANNEX_L:
		codec = HAL_VIDEO_CODEC_VC1;
		break;
	case V4L2_PIX_FMT_VP8:
		codec = HAL_VIDEO_CODEC_VP8;
		break;
	case V4L2_PIX_FMT_DIVX_311:
		codec = HAL_VIDEO_CODEC_DIVX_311;
		break;
	case V4L2_PIX_FMT_DIVX:
		codec = HAL_VIDEO_CODEC_DIVX;
		break;
	case V4L2_PIX_FMT_HEVC:
		codec = HAL_VIDEO_CODEC_HEVC;
		break;
	case V4L2_PIX_FMT_HEVC_HYBRID:
		codec = HAL_VIDEO_CODEC_HEVC_HYBRID;
		break;
	default:
		dprintk(VIDC_ERR, "Wrong codec: %d\n", fourcc);
		codec = HAL_UNUSED_CODEC;
		break;
	}

	return codec;
}

static int msm_comm_vote_bus(struct msm_vidc_core *core)
{
	int rc = 0, vote_data_count = 0, i = 0;
	struct hfi_device *hdev;
	struct msm_vidc_inst *inst = NULL;
	struct vidc_bus_vote_data *vote_data = NULL;

	if (!core) {
		dprintk(VIDC_ERR, "%s Invalid args: %p\n", __func__, core);
		return -EINVAL;
	}

	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "%s Invalid device handle: %p\n",
			__func__, hdev);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list)
		++vote_data_count;

	vote_data = kzalloc(sizeof(*vote_data) * vote_data_count,
			GFP_TEMPORARY);
	if (!vote_data) {
		dprintk(VIDC_ERR, "%s: failed to allocate memory\n", __func__);
		rc = -ENOMEM;
		goto fail_alloc;
	}

	list_for_each_entry(inst, &core->instances, list) {
		int codec = 0;
		codec = inst->session_type == MSM_VIDC_DECODER ?
			inst->fmts[OUTPUT_PORT]->fourcc :
			inst->fmts[CAPTURE_PORT]->fourcc;

		vote_data[i].session = VIDC_VOTE_DATA_SESSION_VAL(
				get_hal_codec_type(codec),
				get_hal_domain(inst->session_type));
		vote_data[i].load = msm_comm_get_inst_load(inst,
				LOAD_CALC_NO_QUIRKS);
		vote_data[i].low_power = !!(inst->flags & VIDC_POWER_SAVE);
		i++;
	}
	mutex_unlock(&core->lock);

	rc = call_hfi_op(hdev, vote_bus, hdev->hfi_device_data, vote_data,
			vote_data_count);
	if (rc)
		dprintk(VIDC_ERR, "Failed to scale bus: %d\n", rc);

	kfree(vote_data);
	return rc;

fail_alloc:
	mutex_unlock(&core->lock);
	return rc;
}

static void msm_comm_unvote_buses(struct msm_vidc_core *core)
{
	struct hfi_device *hdev;

	if (!core || !core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return;
	}

	hdev = core->device;
	if (call_hfi_op(hdev, unvote_bus, hdev->hfi_device_data))
		dprintk(VIDC_WARN,
				"Failed to unvote for buses\n");

}

struct msm_vidc_core *get_vidc_core(int core_id)
{
	struct msm_vidc_core *core;
	int found = 0;
	if (core_id > MSM_VIDC_CORES_MAX) {
		dprintk(VIDC_ERR, "Core id = %d is greater than max = %d\n",
			core_id, MSM_VIDC_CORES_MAX);
		return NULL;
	}
	mutex_lock(&vidc_driver->lock);
	list_for_each_entry(core, &vidc_driver->cores, list) {
		if (core->id == core_id) {
			found = 1;
			break;
		}
	}
	mutex_unlock(&vidc_driver->lock);
	if (found)
		return core;
	return NULL;
}

const struct msm_vidc_format *msm_comm_get_pixel_fmt_index(
	const struct msm_vidc_format fmt[], int size, int index, int fmt_type)
{
	int i, k = 0;
	if (!fmt || index < 0) {
		dprintk(VIDC_ERR, "Invalid inputs, fmt = %p, index = %d\n",
						fmt, index);
		return NULL;
	}
	for (i = 0; i < size; i++) {
		if (fmt[i].type != fmt_type)
			continue;
		if (k == index)
			break;
		k++;
	}
	if (i == size) {
		dprintk(VIDC_INFO, "Format not found\n");
		return NULL;
	}
	return &fmt[i];
}
struct msm_vidc_format *msm_comm_get_pixel_fmt_fourcc(
	struct msm_vidc_format fmt[], int size, int fourcc, int fmt_type)
{
	int i;
	if (!fmt) {
		dprintk(VIDC_ERR, "Invalid inputs, fmt = %p\n", fmt);
		return NULL;
	}
	for (i = 0; i < size; i++) {
		if (fmt[i].fourcc == fourcc)
				break;
	}
	if (i == size) {
		dprintk(VIDC_INFO, "Format not found\n");
		return NULL;
	}
	return &fmt[i];
}

struct buf_queue *msm_comm_get_vb2q(
		struct msm_vidc_inst *inst, enum v4l2_buf_type type)
{
	if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return &inst->bufq[CAPTURE_PORT];
	if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
		return &inst->bufq[OUTPUT_PORT];
	return NULL;
}

static void handle_sys_init_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_core *core;
	struct vidc_hal_sys_init_done *sys_init_msg;
	unsigned int index;

	if (!IS_SYS_CMD_VALID(cmd)) {
		dprintk(VIDC_ERR, "%s - invalid cmd\n", __func__);
		return;
	}
	index = SYS_MSG_INDEX(cmd);

	if (!response) {
		dprintk(VIDC_ERR,
			"Failed to get valid response for sys init\n");
		return;
	}
	core = get_vidc_core(response->device_id);
	if (!core) {
		dprintk(VIDC_ERR, "Wrong device_id received\n");
		return;
	}
	sys_init_msg = response->data;
	if (!sys_init_msg) {
		dprintk(VIDC_ERR, "sys_init_done message not proper\n");
		return;
	}
	core->enc_codec_supported = sys_init_msg->enc_codec_supported;
	core->dec_codec_supported = sys_init_msg->dec_codec_supported;
	core->max_supported_instances = MAX_SUPPORTED_INSTANCES_COUNT;
	if (core->id == MSM_VIDC_CORE_VENUS &&
		(core->dec_codec_supported & HAL_VIDEO_CODEC_H264))
			core->dec_codec_supported |=
				HAL_VIDEO_CODEC_MVC;
	dprintk(VIDC_DBG, "supported_codecs: enc = 0x%x, dec = 0x%x\n",
		core->enc_codec_supported, core->dec_codec_supported);
	dprintk(VIDC_DBG, "ptr[%d] = %p\n", index, &(core->completions[index]));
	complete(&(core->completions[index]));
}

static void handle_session_release_buf_done(enum command_response cmd,
	void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	struct internal_buf *buf;
	struct list_head *ptr, *next;
	struct hal_buffer_info *buffer;
	u32 buf_found = false;
	u32 address;

	if (!response || !response->data) {
		dprintk(VIDC_ERR, "Invalid release_buf_done response\n");
		return;
	}

	inst = (struct msm_vidc_inst *)response->session_id;
	buffer = (struct hal_buffer_info *) response->data;
	address = buffer->buffer_addr;

	mutex_lock(&inst->scratchbufs.lock);
	list_for_each_safe(ptr, next, &inst->scratchbufs.list) {
		buf = list_entry(ptr, struct internal_buf, list);
		if (address == (u32)buf->handle->device_addr) {
			dprintk(VIDC_DBG, "releasing scratch: 0x%pa\n",
					&buf->handle->device_addr);
			buf_found = true;
		}
	}
	mutex_unlock(&inst->scratchbufs.lock);

	mutex_lock(&inst->persistbufs.lock);
	list_for_each_safe(ptr, next, &inst->persistbufs.list) {
		buf = list_entry(ptr, struct internal_buf, list);
		if (address == (u32)buf->handle->device_addr) {
			dprintk(VIDC_DBG, "releasing persist: 0x%pa\n",
					&buf->handle->device_addr);
			buf_found = true;
		}
	}
	mutex_unlock(&inst->persistbufs.lock);

	if (!buf_found)
		dprintk(VIDC_ERR, "invalid buffer received from firmware");
	if (IS_SESSION_CMD_VALID(cmd)) {
		complete(&inst->completions[SESSION_MSG_INDEX(cmd)]);
	} else {
		dprintk(VIDC_ERR, "Invalid inst cmd response: %d\n", cmd);
		return;
	}
}

static void handle_sys_release_res_done(
	enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_core *core;
	if (!response) {
		dprintk(VIDC_ERR,
			"Failed to get valid response for sys init\n");
		return;
	}
	core = get_vidc_core(response->device_id);
	if (!core) {
		dprintk(VIDC_ERR, "Wrong device_id received\n");
		return;
	}
	complete(&core->completions[SYS_MSG_INDEX(RELEASE_RESOURCE_DONE)]);
}

static void change_inst_state(struct msm_vidc_inst *inst,
	enum instance_state state)
{
	if (!inst) {
		dprintk(VIDC_ERR, "Invalid parameter %s\n", __func__);
		return;
	}
	mutex_lock(&inst->lock);
	if (inst->state == MSM_VIDC_CORE_INVALID) {
		dprintk(VIDC_DBG,
			"Inst: %p is in bad state can't change state to %d\n",
			inst, state);
		goto exit;
	}
	dprintk(VIDC_DBG, "Moved inst: %p from state: %d to state: %d\n",
		   inst, inst->state, state);
	inst->state = state;
exit:
	mutex_unlock(&inst->lock);
}

static int signal_session_msg_receipt(enum command_response cmd,
		struct msm_vidc_inst *inst)
{
	if (!inst) {
		dprintk(VIDC_ERR, "Invalid(%p) instance id\n", inst);
		return -EINVAL;
	}
	if (IS_SESSION_CMD_VALID(cmd)) {
		complete(&inst->completions[SESSION_MSG_INDEX(cmd)]);
	} else {
		dprintk(VIDC_ERR, "Invalid inst cmd response: %d\n", cmd);
		return -EINVAL;
	}
	return 0;
}

static int wait_for_sess_signal_receipt(struct msm_vidc_inst *inst,
	enum command_response cmd)
{
	int rc = 0;

	if (!IS_SESSION_CMD_VALID(cmd)) {
		dprintk(VIDC_ERR, "Invalid inst cmd response: %d\n", cmd);
		return -EINVAL;
	}
	rc = wait_for_completion_timeout(
		&inst->completions[SESSION_MSG_INDEX(cmd)],
		msecs_to_jiffies(msm_vidc_hw_rsp_timeout));
	if (!rc) {
		dprintk(VIDC_ERR, "Wait interrupted or timedout: %d\n", rc);
		msm_comm_kill_session(inst);
		rc = -EIO;
	} else {
		rc = 0;
	}
	return rc;
}

static int wait_for_state(struct msm_vidc_inst *inst,
	enum instance_state flipped_state,
	enum instance_state desired_state,
	enum command_response hal_cmd)
{
	int rc = 0;
	if (IS_ALREADY_IN_STATE(flipped_state, desired_state)) {
		dprintk(VIDC_INFO, "inst: %p is already in state: %d\n",
						inst, inst->state);
		goto err_same_state;
	}
	dprintk(VIDC_DBG, "Waiting for hal_cmd: %d\n", hal_cmd);
	rc = wait_for_sess_signal_receipt(inst, hal_cmd);
	if (!rc)
		change_inst_state(inst, desired_state);
err_same_state:
	return rc;
}

void msm_vidc_queue_v4l2_event(struct msm_vidc_inst *inst, int event_type)
{
	struct v4l2_event event = {.id = 0, .type = event_type};
	v4l2_event_queue_fh(&inst->event_handler, &event);
	wake_up(&inst->kernel_event_queue);
}

static void msm_comm_generate_max_clients_error(struct msm_vidc_inst *inst)
{
	enum command_response cmd = SESSION_ERROR;
	struct msm_vidc_cb_cmd_done response = {0};

	if (!inst) {
		dprintk(VIDC_ERR, "%s: invalid input parameters\n", __func__);
		return;
	}
	dprintk(VIDC_ERR, "%s: Too many clients\n", __func__);
	response.session_id = inst;
	response.status = VIDC_ERR_MAX_CLIENTS;
	handle_session_error(cmd, (void *)&response);
}

static void handle_session_init_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst = NULL;
	struct hfi_device *hdev;

	if (response) {
		struct vidc_hal_session_init_done *session_init_done =
			(struct vidc_hal_session_init_done *)
			response->data;
		inst = (struct msm_vidc_inst *)response->session_id;
		if (!inst || !inst->core || !inst->core->device) {
			dprintk(VIDC_ERR, "%s: invalid parameters (0x%p)\n",
				__func__, inst);
			return;
		}

		hdev = inst->core->device;

		if (!response->status && session_init_done) {
			inst->capability.width = session_init_done->width;
			inst->capability.height = session_init_done->height;
			inst->capability.frame_rate =
				session_init_done->frame_rate;
			inst->capability.scale_x = session_init_done->scale_x;
			inst->capability.scale_y = session_init_done->scale_y;
			inst->capability.hier_p = session_init_done->hier_p;
			inst->capability.ltr_count =
				session_init_done->ltr_count;
			inst->capability.pixelprocess_capabilities =
				call_hfi_op(hdev, get_core_capabilities);
			inst->capability.mbs_per_frame =
				session_init_done->mbs_per_frame;
			inst->capability.capability_set = true;
			inst->capability.buffer_mode[CAPTURE_PORT] =
				session_init_done->alloc_mode_out;
			inst->capability.secure_output2_threshold =
				session_init_done->secure_output2_threshold;
		} else {
			dprintk(VIDC_ERR,
				"Session init response from FW : 0x%x\n",
				response->status);
			if (response->status == VIDC_ERR_MAX_CLIENTS)
				msm_comm_generate_max_clients_error(inst);
			else
				msm_comm_generate_session_error(inst);
		}
		signal_session_msg_receipt(cmd, inst);
	} else {
		dprintk(VIDC_ERR,
				"Failed to get valid response for session init\n");
	}
}

static void handle_event_change(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	struct v4l2_control control = {0};
	struct msm_vidc_cb_event *event_notify;
	int event = V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT;
	int rc = 0;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		event_notify = (struct msm_vidc_cb_event *) response->data;
		switch (event_notify->hal_event_type) {
		case HAL_EVENT_SEQ_CHANGED_SUFFICIENT_RESOURCES:
			event = V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT;
			control.id =
				V4L2_CID_MPEG_VIDC_VIDEO_CONTINUE_DATA_TRANSFER;
			rc = v4l2_g_ctrl(&inst->ctrl_handler, &control);
			if (rc)
				dprintk(VIDC_WARN,
					"Failed to get Smooth streamng flag\n");
			if (!rc && control.value == true)
				event = V4L2_EVENT_SEQ_CHANGED_SUFFICIENT;
			break;
		case HAL_EVENT_SEQ_CHANGED_INSUFFICIENT_RESOURCES:
			event = V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT;
			break;
		case HAL_EVENT_RELEASE_BUFFER_REFERENCE:
		{
			struct v4l2_event buf_event = {0};
			struct buffer_info *binfo = NULL, *temp = NULL;
			u32 *ptr = NULL;

			dprintk(VIDC_DBG,
				"%s - inst: %p buffer: 0x%pa extra: 0x%pa\n",
				__func__, inst, &event_notify->packet_buffer,
				&event_notify->extra_data_buffer);

			/*
			* If buffer release event is received with inst->state
			* greater than STOP means client called STOP directly
			* without FLUSH. This also means that they don't expect
			* these buffers back. Processing these commands will not
			* add any value. This can also results deadlocks between
			* try_state and event_notify due to inst->sync_lock.
			*/

			mutex_lock(&inst->lock);
			if (inst->state >= MSM_VIDC_STOP ||
				inst->core->state == VIDC_CORE_INVALID) {
				dprintk(VIDC_ERR,
					"Release buffer reference called in invalid state\n");
				mutex_unlock(&inst->lock);
				return;
			}
			mutex_unlock(&inst->lock);

			/*
			* Get the buffer_info entry for the
			* device address.
			*/
			binfo = device_to_uvaddr(&inst->registeredbufs,
				event_notify->packet_buffer);
			if (!binfo) {
				dprintk(VIDC_ERR,
					"%s buffer not found in registered list\n",
					__func__);
				return;
			}

			/* Fill event data to be sent to client*/
			buf_event.type = V4L2_EVENT_RELEASE_BUFFER_REFERENCE;
			ptr = (u32 *)buf_event.u.data;
			ptr[0] = binfo->fd[0];
			ptr[1] = binfo->buff_off[0];

			dprintk(VIDC_DBG,
				"RELEASE REFERENCE EVENT FROM F/W - fd = %d offset = %d\n",
				ptr[0], ptr[1]);

			/* Decrement buffer reference count*/
			mutex_lock(&inst->registeredbufs.lock);
			list_for_each_entry(temp, &inst->registeredbufs.list,
					list) {
				if (temp == binfo) {
					buf_ref_put(inst, binfo);
					break;
				}
			}

			/*
			* Release buffer and remove from list
			* if reference goes to zero.
			*/
			if (unmap_and_deregister_buf(inst, binfo))
				dprintk(VIDC_ERR,
				"%s: buffer unmap failed\n", __func__);
			mutex_unlock(&inst->registeredbufs.lock);
			/*send event to client*/
			v4l2_event_queue_fh(&inst->event_handler, &buf_event);
			wake_up(&inst->kernel_event_queue);
			return;
		}
		default:
			break;
		}
		if (event == V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT) {
			dprintk(VIDC_DBG,
				"V4L2_EVENT_SEQ_CHANGED_INSUFFICIENT\n");
			inst->reconfig_height = event_notify->height;
			inst->reconfig_width = event_notify->width;
			inst->in_reconfig = true;
		} else {
			dprintk(VIDC_DBG,
				"V4L2_EVENT_SEQ_CHANGED_SUFFICIENT\n");
			if (msm_comm_get_stream_output_mode(inst) !=
				HAL_VIDEO_DECODER_SECONDARY) {
				dprintk(VIDC_DBG,
					"event_notify->height = %d event_notify->width = %d\n",
					event_notify->height,
					event_notify->width);
				inst->prop.height[CAPTURE_PORT] =
					event_notify->height;
				inst->prop.width[CAPTURE_PORT] =
					event_notify->width;
				inst->prop.height[OUTPUT_PORT] =
					event_notify->height;
				inst->prop.width[OUTPUT_PORT] =
					event_notify->width;
			}
		}

		if (inst->session_type == MSM_VIDC_DECODER)
			msm_dcvs_init_load(inst);
		rc = msm_vidc_check_session_supported(inst);
		if (!rc) {
			msm_vidc_queue_v4l2_event(inst, event);
		} else if (rc == -ENOTSUPP) {
			msm_vidc_queue_v4l2_event(inst,
				V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED);
		} else if (rc == -EBUSY) {
			msm_vidc_queue_v4l2_event(inst,
				V4L2_EVENT_MSM_VIDC_HW_OVERLOAD);
		}

		return;
	} else {
		dprintk(VIDC_ERR,
			"Failed to get valid response for event_change\n");
	}
}

static void handle_session_prop_info(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct getprop_buf *getprop;
	struct msm_vidc_inst *inst;
	if (!response || !response->data) {
		dprintk(VIDC_ERR,
			"Failed to get valid response for prop info\n");
		return;
	}
	inst = (struct msm_vidc_inst *)response->session_id;

	getprop = kzalloc(sizeof(*getprop), GFP_KERNEL);
	if (!getprop) {
		dprintk(VIDC_ERR, "%s: getprop kzalloc failed\n", __func__);
		return;
	}

	getprop->data = kmemdup(response->data, response->size, GFP_KERNEL);
	if (!getprop->data) {
		dprintk(VIDC_ERR, "%s: kmemdup failed\n", __func__);
		kfree(getprop);
		return;
	}

	mutex_lock(&inst->pending_getpropq.lock);
	list_add_tail(&getprop->list, &inst->pending_getpropq.list);
	mutex_unlock(&inst->pending_getpropq.lock);

	signal_session_msg_receipt(cmd, inst);
	return;
}

static void handle_load_resource_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		if (response->status) {
			dprintk(VIDC_ERR,
				"Load resource response from FW : 0x%x\n",
				response->status);
			msm_comm_generate_session_error(inst);
		}
	} else
		dprintk(VIDC_ERR,
			"Failed to get valid response for load resource\n");
}

static void handle_start_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		signal_session_msg_receipt(cmd, inst);
	} else {
		dprintk(VIDC_ERR,
			"Failed to get valid response for start\n");
	}
}

static void handle_stop_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		signal_session_msg_receipt(cmd, inst);
	} else {
		dprintk(VIDC_ERR,
			"Failed to get valid response for stop\n");
	}
}

static void handle_release_res_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		signal_session_msg_receipt(cmd, inst);
	} else {
		dprintk(VIDC_ERR,
			"Failed to get valid response for release resource\n");
	}
}
void validate_output_buffers(struct msm_vidc_inst *inst)
{
	struct internal_buf *binfo;
	u32 buffers_owned_by_driver = 0;
	struct hal_buffer_requirements *output_buf;
	output_buf = get_buff_req_buffer(inst, HAL_BUFFER_OUTPUT);
	if (!output_buf) {
		dprintk(VIDC_DBG,
			"This output buffer not required, buffer_type: %x\n",
			HAL_BUFFER_OUTPUT);
		return;
	}
	mutex_lock(&inst->outputbufs.lock);
	list_for_each_entry(binfo, &inst->outputbufs.list, list) {
		if (binfo->buffer_ownership != DRIVER) {
			dprintk(VIDC_DBG,
				"This buffer is with FW 0x%pa\n",
				&binfo->handle->device_addr);
			continue;
		}
		buffers_owned_by_driver++;
	}
	mutex_unlock(&inst->outputbufs.lock);

	if (buffers_owned_by_driver != output_buf->buffer_count_actual)
		dprintk(VIDC_WARN,
			"OUTPUT Buffer count mismatch %d of %d\n",
			buffers_owned_by_driver,
			output_buf->buffer_count_actual);

	return;
}

int msm_comm_queue_output_buffers(struct msm_vidc_inst *inst)
{
	struct internal_buf *binfo;
	struct hfi_device *hdev;
	struct msm_smem *handle;
	struct vidc_frame_data frame_data = {0};
	struct hal_buffer_requirements *output_buf;
	int rc = 0;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;

	output_buf = get_buff_req_buffer(inst, HAL_BUFFER_OUTPUT);
	if (!output_buf) {
		dprintk(VIDC_DBG,
			"This output buffer not required, buffer_type: %x\n",
			HAL_BUFFER_OUTPUT);
		return 0;
	}
	dprintk(VIDC_DBG,
		"output: num = %d, size = %d\n",
		output_buf->buffer_count_actual,
		output_buf->buffer_size);

	mutex_lock(&inst->outputbufs.lock);
	list_for_each_entry(binfo, &inst->outputbufs.list, list) {
		if (binfo->buffer_ownership != DRIVER)
			continue;
		handle = binfo->handle;
		frame_data.alloc_len = output_buf->buffer_size;
		frame_data.filled_len = 0;
		frame_data.offset = 0;
		frame_data.device_addr = handle->device_addr;
		frame_data.flags = 0;
		frame_data.extradata_addr = handle->device_addr +
		output_buf->buffer_size;
		frame_data.buffer_type = HAL_BUFFER_OUTPUT;
		rc = call_hfi_op(hdev, session_ftb,
			(void *) inst->session, &frame_data);
		binfo->buffer_ownership = FIRMWARE;
	}
	mutex_unlock(&inst->outputbufs.lock);

	return 0;
}

static void handle_session_flush(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;
	int rc;
	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		if (msm_comm_get_stream_output_mode(inst) ==
			HAL_VIDEO_DECODER_SECONDARY) {
			validate_output_buffers(inst);
			if (!inst->in_reconfig) {
				rc = msm_comm_queue_output_buffers(inst);
				if (rc) {
					dprintk(VIDC_ERR,
						"Failed to queue output buffers: %d\n",
						rc);
				}
			}
		}
		msm_vidc_queue_v4l2_event(inst, V4L2_EVENT_MSM_VIDC_FLUSH_DONE);
	} else {
		dprintk(VIDC_ERR, "Failed to get valid response for flush\n");
	}
}

static void handle_sys_idle(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_core *core = NULL;
	int rc = 0;

	if (!response) {
		rc = -EINVAL;
		goto exit;
	}

	core = get_vidc_core(response->device_id);
	if (!core) {
		dprintk(VIDC_ERR, "Received IDLE with invalid core\n");
		rc = -EINVAL;
		goto exit;
	}

	dprintk(VIDC_DBG, "SYS_IDLE received for core %p\n", core);
	if (core->resources.dynamic_bw_update) {
		mutex_lock(&core->lock);
		core->idle_stats.start_time = ktime_get();
		dprintk(VIDC_DBG, "%s: start idle time %llu us\n",
				__func__, ktime_to_us(
					core->idle_stats.start_time));
		core->idle_stats.idle = true;
		mutex_unlock(&core->lock);
	}

exit:
	if (rc)
		dprintk(VIDC_WARN, "Failed to handle idle message\n");
}

static void handle_session_error(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct hfi_device *hdev = NULL;
	struct msm_vidc_inst *inst = NULL;

	if (!response) {
		dprintk(VIDC_ERR,
			"Failed to get valid response for session error\n");
		return;
	}

	inst = (struct msm_vidc_inst *)response->session_id;

	if (!inst || !inst->session || !inst->core->device) {
		dprintk(VIDC_ERR,
				"Session (%p) not in a stable enough state to handle session error\n",
				inst);
		return;
	}

	hdev = inst->core->device;
	dprintk(VIDC_WARN, "Session error received for session %p\n", inst);
	change_inst_state(inst, MSM_VIDC_CORE_INVALID);

	if (response->status == VIDC_ERR_MAX_CLIENTS) {
		dprintk(VIDC_WARN,
			"send max clients reached error to client: %p\n",
			inst);
		msm_vidc_queue_v4l2_event(inst,
			V4L2_EVENT_MSM_VIDC_MAX_CLIENTS);
	} else {
		dprintk(VIDC_ERR,
			"send session error to client: %p\n",
			inst);
		msm_vidc_queue_v4l2_event(inst,
			V4L2_EVENT_MSM_VIDC_SYS_ERROR);
	}
}

static void msm_comm_clean_notify_client(struct msm_vidc_core *core)
{
	struct msm_vidc_inst *inst = NULL;
	if (!core) {
		dprintk(VIDC_ERR, "%s: Invalid params\n", __func__);
		return;
	}

	dprintk(VIDC_WARN, "%s: Core %p\n", __func__, core);
	mutex_lock(&core->lock);
	core->state = VIDC_CORE_INVALID;

	list_for_each_entry(inst, &core->instances, list) {
		mutex_lock(&inst->lock);
		inst->state = MSM_VIDC_CORE_INVALID;
		mutex_unlock(&inst->lock);
		dprintk(VIDC_WARN,
			"%s Send sys error for inst %p\n", __func__, inst);
		msm_vidc_queue_v4l2_event(inst,
				V4L2_EVENT_MSM_VIDC_SYS_ERROR);
	}
	mutex_unlock(&core->lock);
}

struct sys_err_handler_data {
	struct msm_vidc_core *core;
	struct delayed_work work;
};

static void handle_sys_error(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_core *core = NULL;
	struct hfi_device *hdev = NULL;
	int rc = 0;

	subsystem_crashed("venus");
	if (!response) {
		dprintk(VIDC_ERR,
			"Failed to get valid response for sys error\n");
		return;
	}

	core = get_vidc_core(response->device_id);
	if (!core) {
		dprintk(VIDC_ERR,
				"Got SYS_ERR but unable to identify core\n");
		return;
	}

	dprintk(VIDC_WARN, "SYS_ERROR %d received for core %p\n", cmd, core);
	msm_comm_clean_notify_client(core);
	hdev = core->device;
	mutex_lock(&core->lock);
	if (core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_DBG, "Calling core_release\n");
		rc = call_hfi_op(hdev, core_release,
			hdev->hfi_device_data);
		if (rc) {
			dprintk(VIDC_ERR, "core_release failed\n");
			mutex_unlock(&core->lock);
			return;
		}

		core->state = VIDC_CORE_UNINIT;
		call_hfi_op(hdev, unload_fw, hdev->hfi_device_data);
		dprintk(VIDC_DBG, "Firmware unloaded\n");
		msm_comm_unvote_buses(core);
	}
	mutex_unlock(&core->lock);

}

void msm_comm_session_clean(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev = NULL;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid params\n", __func__);
		return;
	}

	hdev = inst->core->device;
	mutex_lock(&inst->lock);
	if (hdev && inst->session) {
		dprintk(VIDC_DBG, "cleaning up instance: 0x%p\n", inst);
		rc = call_hfi_op(hdev, session_clean,
				(void *) inst->session);
		if (rc) {
			dprintk(VIDC_ERR,
				"Session clean failed :%p\n", inst);
		}
		inst->session = NULL;
	}
	mutex_unlock(&inst->lock);
}

static void handle_session_close(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_cmd_done *response = data;
	struct msm_vidc_inst *inst;

	if (response) {
		inst = (struct msm_vidc_inst *)response->session_id;
		if (!inst || !inst->core || !inst->core->device) {
			dprintk(VIDC_ERR, "%s invalid params\n", __func__);
			return;
		}
		signal_session_msg_receipt(cmd, inst);
		show_stats(inst);
	} else {
		dprintk(VIDC_ERR,
			"Failed to get valid response for session close\n");
	}
}

static struct vb2_buffer *get_vb_from_device_addr(struct buf_queue *bufq,
		unsigned long dev_addr)
{
	struct vb2_buffer *vb = NULL;
	struct vb2_queue *q = NULL;
	int found = 0;
	if (!bufq) {
		dprintk(VIDC_ERR, "Invalid parameter\n");
		return NULL;
	}
	q = &bufq->vb2_bufq;
	mutex_lock(&bufq->lock);
	list_for_each_entry(vb, &q->queued_list, queued_entry) {
		if (vb->v4l2_planes[0].m.userptr == dev_addr &&
			vb->state == VB2_BUF_STATE_ACTIVE) {
			found = 1;
			dprintk(VIDC_DBG, "Found v4l2_buf index : %d\n",
					vb->v4l2_buf.index);
			break;
		}
	}
	mutex_unlock(&bufq->lock);
	if (!found) {
		dprintk(VIDC_DBG,
			"Failed to find buffer in queued list: 0x%lx, qtype = %d\n",
			dev_addr, q->type);
		vb = NULL;
	}
	return vb;
}

static void handle_ebd(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_data_done *response = data;
	struct vb2_buffer *vb;
	struct msm_vidc_inst *inst;
	struct vidc_hal_ebd *empty_buf_done;

	if (!response) {
		dprintk(VIDC_ERR, "Invalid response from vidc_hal\n");
		return;
	}

	inst = (struct msm_vidc_inst *)response->session_id;
	if (!inst) {
		dprintk(VIDC_ERR, "%s Invalid response from vidc_hal\n",
			__func__);
		return;
	}
	vb = get_vb_from_device_addr(&inst->bufq[OUTPUT_PORT],
				response->clnt_data);
	if (vb) {
		vb->v4l2_planes[0].bytesused = response->input_done.filled_len;
		vb->v4l2_planes[0].data_offset = response->input_done.offset;
		if (vb->v4l2_planes[0].data_offset > vb->v4l2_planes[0].length)
			dprintk(VIDC_INFO, "data_offset overflow length\n");
		if (vb->v4l2_planes[0].bytesused > vb->v4l2_planes[0].length)
			dprintk(VIDC_INFO, "bytesused overflow length\n");
		if (vb->v4l2_planes[0].m.userptr !=
			response->input_done.packet_buffer)
			dprintk(VIDC_INFO, "Unexpected buffer address\n");
		empty_buf_done = (struct vidc_hal_ebd *)&response->input_done;
		if (empty_buf_done) {
			if (empty_buf_done->status == VIDC_ERR_NOT_SUPPORTED) {
				dprintk(VIDC_INFO,
					"Failed : Unsupported input stream\n");
				vb->v4l2_buf.flags |=
					V4L2_QCOM_BUF_INPUT_UNSUPPORTED;
			}
			if (empty_buf_done->status == VIDC_ERR_BITSTREAM_ERR) {
				dprintk(VIDC_INFO,
					"Failed : Corrupted input stream\n");
				vb->v4l2_buf.flags |=
					V4L2_QCOM_BUF_DATA_CORRUPT;
			}
			if (empty_buf_done->status ==
				VIDC_ERR_START_CODE_NOT_FOUND) {
				vb->v4l2_buf.flags |=
					V4L2_MSM_VIDC_BUF_START_CODE_NOT_FOUND;
				dprintk(VIDC_INFO,
					"Failed: Start code not found\n");
			}
		}
		dprintk(VIDC_DBG,
			"Got ebd from hal: device_addr: 0x%pa, alloc: %d, status: 0x%x, pic_type: 0x%x, flags: 0x%x\n",
			&empty_buf_done->packet_buffer,
			empty_buf_done->alloc_len, empty_buf_done->status,
			empty_buf_done->picture_type, empty_buf_done->flags);

		mutex_lock(&inst->bufq[OUTPUT_PORT].lock);
		vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
		mutex_unlock(&inst->bufq[OUTPUT_PORT].lock);
		wake_up(&inst->kernel_event_queue);
		msm_vidc_debugfs_update(inst, MSM_VIDC_DEBUGFS_EVENT_EBD);
	}
}

int buf_ref_get(struct msm_vidc_inst *inst, struct buffer_info *binfo)
{
	int cnt = 0;

	if (!inst || !binfo)
		return -EINVAL;

	atomic_inc(&binfo->ref_count);
	cnt = atomic_read(&binfo->ref_count);
	if (cnt > 2) {
		dprintk(VIDC_DBG, "%s: invalid ref_cnt: %d\n", __func__, cnt);
		cnt = -EINVAL;
	}
	dprintk(VIDC_DBG, "REF_GET[%d] fd[0] = %d\n", cnt, binfo->fd[0]);

	return cnt;
}

int buf_ref_put(struct msm_vidc_inst *inst, struct buffer_info *binfo)
{
	int rc = 0;
	int cnt;
	bool release_buf = false;
	bool qbuf_again = false;

	if (!inst || !binfo)
		return -EINVAL;

	atomic_dec(&binfo->ref_count);
	cnt = atomic_read(&binfo->ref_count);
	dprintk(VIDC_DBG, "REF_PUT[%d] fd[0] = %d\n", cnt, binfo->fd[0]);
	if (cnt == 0)
		release_buf = true;
	else if (cnt == 1)
		qbuf_again = true;
	else {
		dprintk(VIDC_DBG, "%s: invalid ref_cnt: %d\n", __func__, cnt);
		cnt = -EINVAL;
	}

	if (cnt < 0)
		return cnt;

	rc = output_buffer_cache_invalidate(inst, binfo);
	if (rc)
		return rc;

	if (release_buf) {
		/*
		* We can not delete binfo here as we need to set the user
		* virtual address saved in binfo->uvaddr to the dequeued v4l2
		* buffer.
		*
		* We will set the pending_deletion flag to true here and delete
		* binfo from registered list in dqbuf after setting the uvaddr.
		*/
		dprintk(VIDC_DBG, "fd[0] = %d -> pending_deletion = true\n",
			binfo->fd[0]);
		binfo->pending_deletion = true;
	} else if (qbuf_again) {
		rc = qbuf_dynamic_buf(inst, binfo);
		if (!rc)
			return rc;
	}
	return cnt;
}

static void handle_dynamic_buffer(struct msm_vidc_inst *inst,
		ion_phys_addr_t device_addr, u32 flags)
{
	struct buffer_info *binfo = NULL, *temp = NULL;

	/*
	 * Update reference count and release OR queue back the buffer,
	 * only when firmware is not holding a reference.
	 */
	if (inst->buffer_mode_set[CAPTURE_PORT] == HAL_BUFFER_MODE_DYNAMIC) {
		binfo = device_to_uvaddr(&inst->registeredbufs, device_addr);
		if (!binfo) {
			dprintk(VIDC_ERR,
				"%s buffer not found in registered list\n",
				__func__);
			return;
		}
		if (flags & HAL_BUFFERFLAG_READONLY) {
			dprintk(VIDC_DBG,
				"FBD fd[0] = %d -> Reference with f/w, addr: 0x%pa\n",
				binfo->fd[0], &device_addr);
		} else {
			dprintk(VIDC_DBG,
				"FBD fd[0] = %d -> FBD_ref_released, addr: 0x%pa\n",
				binfo->fd[0], &device_addr);

			mutex_lock(&inst->registeredbufs.lock);
			list_for_each_entry(temp, &inst->registeredbufs.list,
							list) {
				if (temp == binfo) {
					buf_ref_put(inst, binfo);
					break;
				}
			}
			mutex_unlock(&inst->registeredbufs.lock);
		}
	}
}

static int handle_multi_stream_buffers(struct msm_vidc_inst *inst,
		ion_phys_addr_t dev_addr)
{
	struct internal_buf *binfo;
	struct msm_smem *handle;
	bool found = false;

	mutex_lock(&inst->outputbufs.lock);
	list_for_each_entry(binfo, &inst->outputbufs.list, list) {
		handle = binfo->handle;
		if (handle && dev_addr == handle->device_addr) {
			if (binfo->buffer_ownership == DRIVER) {
				dprintk(VIDC_ERR,
					"FW returned same buffer: 0x%pa\n",
					&dev_addr);
				break;
			}
			binfo->buffer_ownership = DRIVER;
			found = true;
			break;
		}
	}
	mutex_unlock(&inst->outputbufs.lock);

	if (!found) {
		dprintk(VIDC_ERR,
			"Failed to find output buffer in queued list: 0x%pa\n",
			&dev_addr);
	}

	return 0;
}

enum hal_buffer msm_comm_get_hal_output_buffer(struct msm_vidc_inst *inst)
{
	if (msm_comm_get_stream_output_mode(inst) ==
		HAL_VIDEO_DECODER_SECONDARY)
		return HAL_BUFFER_OUTPUT2;
	else
		return HAL_BUFFER_OUTPUT;
}

static void handle_fbd(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_data_done *response = data;
	struct msm_vidc_inst *inst;
	struct vb2_buffer *vb = NULL;
	struct vidc_hal_fbd *fill_buf_done;
	enum hal_buffer buffer_type;
	int extra_idx = 0;
	int64_t time_usec = 0;

	if (!response) {
		dprintk(VIDC_ERR, "Invalid response from vidc_hal\n");
		return;
	}
	inst = (struct msm_vidc_inst *)response->session_id;
	fill_buf_done = (struct vidc_hal_fbd *)&response->output_done;
	buffer_type = msm_comm_get_hal_output_buffer(inst);
	if (fill_buf_done->buffer_type == buffer_type) {
		vb = get_vb_from_device_addr(&inst->bufq[CAPTURE_PORT],
				fill_buf_done->packet_buffer1);
	} else {
		if (handle_multi_stream_buffers(inst,
				fill_buf_done->packet_buffer1))
			dprintk(VIDC_ERR,
				"Failed : Output buffer not found 0x%pa\n",
				&fill_buf_done->packet_buffer1);
		return;
	}
	if (vb) {
		vb->v4l2_planes[0].bytesused = fill_buf_done->filled_len1;
		vb->v4l2_planes[0].data_offset = fill_buf_done->offset1;
		vb->v4l2_planes[0].reserved[2] = fill_buf_done->start_x_coord;
		vb->v4l2_planes[0].reserved[3] = fill_buf_done->start_y_coord;
		vb->v4l2_planes[0].reserved[4] = fill_buf_done->frame_width;
		vb->v4l2_planes[0].reserved[5] = fill_buf_done->frame_height;
		vb->v4l2_planes[0].reserved[6] =
			inst->prop.width[CAPTURE_PORT];
		vb->v4l2_planes[0].reserved[7] =
			inst->prop.height[CAPTURE_PORT];
		if (vb->v4l2_planes[0].data_offset > vb->v4l2_planes[0].length)
			dprintk(VIDC_INFO,
				"fbd:Overflow data_offset = %d; length = %d\n",
				vb->v4l2_planes[0].data_offset,
				vb->v4l2_planes[0].length);
		if (vb->v4l2_planes[0].bytesused > vb->v4l2_planes[0].length)
			dprintk(VIDC_INFO,
				"fbd:Overflow bytesused = %d; length = %d\n",
				vb->v4l2_planes[0].bytesused,
				vb->v4l2_planes[0].length);
		if (!(fill_buf_done->flags1 &
			HAL_BUFFERFLAG_TIMESTAMPINVALID) &&
			fill_buf_done->filled_len1) {
			time_usec = fill_buf_done->timestamp_hi;
			time_usec = (time_usec << 32) |
				fill_buf_done->timestamp_lo;
		} else {
			time_usec = 0;
			dprintk(VIDC_DBG,
					"Set zero timestamp for buffer 0x%pa, filled: %d, (hi:%u, lo:%u)\n",
					&fill_buf_done->packet_buffer1,
					fill_buf_done->filled_len1,
					fill_buf_done->timestamp_hi,
					fill_buf_done->timestamp_lo);
		}
		vb->v4l2_buf.timestamp =
			ns_to_timeval(time_usec * NSEC_PER_USEC);
		vb->v4l2_buf.flags = 0;
		extra_idx =
			EXTRADATA_IDX(inst->fmts[CAPTURE_PORT]->num_planes);
		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
			vb->v4l2_planes[extra_idx].m.userptr =
				(unsigned long)fill_buf_done->extra_data_buffer;
			vb->v4l2_planes[extra_idx].bytesused =
				vb->v4l2_planes[extra_idx].length;
			vb->v4l2_planes[extra_idx].data_offset = 0;
		}

		handle_dynamic_buffer(inst, fill_buf_done->packet_buffer1,
					fill_buf_done->flags1);
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_READONLY)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_READONLY;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_EOS)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_EOS;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_CODECCONFIG)
			vb->v4l2_buf.flags &= ~V4L2_QCOM_BUF_FLAG_CODECCONFIG;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_SYNCFRAME)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_IDRFRAME;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_EOSEQ)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_EOSEQ;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_DECODEONLY)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_DECODEONLY;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_DATACORRUPT)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_DATA_CORRUPT;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_DROP_FRAME)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_DROP_FRAME;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_MBAFF)
			vb->v4l2_buf.flags |= V4L2_MSM_BUF_FLAG_MBAFF;
		if (fill_buf_done->flags1 &
			HAL_BUFFERFLAG_TS_DISCONTINUITY)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_TS_DISCONTINUITY;
		if (fill_buf_done->flags1 & HAL_BUFFERFLAG_TS_ERROR)
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_TS_ERROR;
		switch (fill_buf_done->picture_type) {
		case HAL_PICTURE_IDR:
			vb->v4l2_buf.flags |= V4L2_QCOM_BUF_FLAG_IDRFRAME;
			vb->v4l2_buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
			break;
		case HAL_PICTURE_I:
			vb->v4l2_buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
			break;
		case HAL_PICTURE_P:
			vb->v4l2_buf.flags |= V4L2_BUF_FLAG_PFRAME;
			break;
		case HAL_PICTURE_B:
			vb->v4l2_buf.flags |= V4L2_BUF_FLAG_BFRAME;
			break;
		case HAL_FRAME_NOTCODED:
		case HAL_UNUSED_PICT:
			/* Do we need to care about these? */
		case HAL_FRAME_YUV:
			break;
		default:
			break;
		}
		inst->count.fbd++;
		if (fill_buf_done->filled_len1)
			msm_vidc_debugfs_update(inst,
				MSM_VIDC_DEBUGFS_EVENT_FBD);

		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
			dprintk(VIDC_DBG,
				"extradata: userptr = %p;"
				" bytesused = %d; length = %d\n",
				(u8 *)vb->v4l2_planes[extra_idx].m.userptr,
				vb->v4l2_planes[extra_idx].bytesused,
				vb->v4l2_planes[extra_idx].length);
		}
		dprintk(VIDC_DBG,
		"Got fbd from hal: device_addr: 0x%pa, alloc: %d, filled: %d, offset: %d, ts: %lld, flags: 0x%x, crop: %d %d %d %d, pic_type: 0x%x\n",
		&fill_buf_done->packet_buffer1, fill_buf_done->alloc_len1,
		fill_buf_done->filled_len1, fill_buf_done->offset1, time_usec,
		fill_buf_done->flags1, fill_buf_done->start_x_coord,
		fill_buf_done->start_y_coord, fill_buf_done->frame_width,
		fill_buf_done->frame_height, fill_buf_done->picture_type);

		mutex_lock(&inst->bufq[CAPTURE_PORT].lock);
		vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
		mutex_unlock(&inst->bufq[CAPTURE_PORT].lock);
		wake_up(&inst->kernel_event_queue);
	}
}

static void handle_seq_hdr_done(enum command_response cmd, void *data)
{
	struct msm_vidc_cb_data_done *response = data;
	struct msm_vidc_inst *inst;
	struct vb2_buffer *vb;
	struct vidc_hal_fbd *fill_buf_done;
	if (!response) {
		dprintk(VIDC_ERR, "Invalid response from vidc_hal\n");
		return;
	}
	inst = (struct msm_vidc_inst *)response->session_id;
	fill_buf_done = (struct vidc_hal_fbd *)&response->output_done;
	vb = get_vb_from_device_addr(&inst->bufq[CAPTURE_PORT],
				fill_buf_done->packet_buffer1);
	if (!vb) {
		dprintk(VIDC_ERR,
				"Failed to find video buffer for seq_hdr_done\n");
		return;
	}

	vb->v4l2_planes[0].bytesused = fill_buf_done->filled_len1;
	vb->v4l2_planes[0].data_offset = fill_buf_done->offset1;

	vb->v4l2_buf.flags = V4L2_QCOM_BUF_FLAG_CODECCONFIG;
	vb->v4l2_buf.timestamp = ns_to_timeval(0);

	dprintk(VIDC_DBG, "Filled length = %d; offset = %d; flags %x\n",
				vb->v4l2_planes[0].bytesused,
				vb->v4l2_planes[0].data_offset,
				vb->v4l2_buf.flags);
	mutex_lock(&inst->bufq[CAPTURE_PORT].lock);
	vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
	mutex_unlock(&inst->bufq[CAPTURE_PORT].lock);
}

void handle_cmd_response(enum command_response cmd, void *data)
{
	dprintk(VIDC_DBG, "Command response = %d\n", cmd);
	switch (cmd) {
	case SYS_INIT_DONE:
		handle_sys_init_done(cmd, data);
		break;
	case RELEASE_RESOURCE_DONE:
		handle_sys_release_res_done(cmd, data);
		break;
	case SESSION_INIT_DONE:
		handle_session_init_done(cmd, data);
		break;
	case SESSION_PROPERTY_INFO:
		handle_session_prop_info(cmd, data);
		break;
	case SESSION_LOAD_RESOURCE_DONE:
		handle_load_resource_done(cmd, data);
		break;
	case SESSION_START_DONE:
		handle_start_done(cmd, data);
		break;
	case SESSION_ETB_DONE:
		handle_ebd(cmd, data);
		break;
	case SESSION_FTB_DONE:
		handle_fbd(cmd, data);
		break;
	case SESSION_STOP_DONE:
		handle_stop_done(cmd, data);
		break;
	case SESSION_RELEASE_RESOURCE_DONE:
		handle_release_res_done(cmd, data);
		break;
	case SESSION_END_DONE:
	case SESSION_ABORT_DONE:
		handle_session_close(cmd, data);
		break;
	case VIDC_EVENT_CHANGE:
		handle_event_change(cmd, data);
		break;
	case SESSION_FLUSH_DONE:
		handle_session_flush(cmd, data);
		break;
	case SESSION_GET_SEQ_HDR_DONE:
		handle_seq_hdr_done(cmd, data);
		break;
	case SYS_WATCHDOG_TIMEOUT:
	case SYS_ERROR:
		handle_sys_error(cmd, data);
		break;
	case SESSION_ERROR:
		handle_session_error(cmd, data);
		break;
	case SESSION_RELEASE_BUFFER_DONE:
		handle_session_release_buf_done(cmd, data);
		break;
	case SYS_IDLE:
		handle_sys_idle(cmd, data);
		break;
	default:
		dprintk(VIDC_ERR, "response unhandled\n");
		break;
	}
}

int msm_comm_scale_clocks(struct msm_vidc_core *core)
{
	int num_mbs_per_sec =
		msm_comm_get_load(core, MSM_VIDC_ENCODER, LOAD_CALC_NO_QUIRKS) +
		msm_comm_get_load(core, MSM_VIDC_DECODER, LOAD_CALC_NO_QUIRKS);
	return msm_comm_scale_clocks_load(core, num_mbs_per_sec);
}

int msm_comm_scale_clocks_load(struct msm_vidc_core *core, int num_mbs_per_sec)
{
	u32 codecs_enabled = 0;
	int rc = 0;
	struct hfi_device *hdev;
	struct msm_vidc_inst *inst = NULL;
	bool is_nominal = false;
	struct msm_vidc_platform_resources *res;

	if (!core) {
		dprintk(VIDC_ERR, "%s Invalid args: %p\n", __func__, core);
		return -EINVAL;
	}

	hdev = core->device;
	res = &core->resources;

	if (!hdev) {
		dprintk(VIDC_ERR, "%s Invalid device handle: %p\n",
			__func__, hdev);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		int codec = 0;

		codec = inst->session_type == MSM_VIDC_DECODER ?
			inst->fmts[OUTPUT_PORT]->fourcc :
			inst->fmts[CAPTURE_PORT]->fourcc;

		codecs_enabled |= VIDC_VOTE_DATA_SESSION_VAL(
				get_hal_codec_type(codec),
				get_hal_domain(inst->session_type));

		if (is_nominal_session(inst))
			is_nominal = true;
	}
	mutex_unlock(&core->lock);

	dprintk(VIDC_INFO, "num_mbs_per_sec = %d codecs_enabled 0x%x\n",
			num_mbs_per_sec, codecs_enabled);

	if (is_nominal && num_mbs_per_sec) {
		struct load_freq_table *table = res->load_freq_tbl;
		u32 table_size = res->load_freq_tbl_size;
		u32 low_freq = table[table_size - 1].freq;
		int i;

		/*
		* Parse the load frequency table from highest index and
		* whenever there is a change in frequency detected, it is
		* assumed as nominal frequency  Check the current load
		* against the load corresponding to nominal frequency and
		* update num_mbs_per_sec accordingly.
		*/
		for (i = table_size - 1; i >= 0; i--) {
			if (table[i].freq > low_freq) {
				if (num_mbs_per_sec < table[i].load) {
					num_mbs_per_sec = table[i].load;
					dprintk(VIDC_DBG,
						"updated num_mbs_per_sec: %d\n",
						num_mbs_per_sec);
				}
				break;
			}
		}
	}

	rc = call_hfi_op(hdev, scale_clocks,
		hdev->hfi_device_data, num_mbs_per_sec, codecs_enabled);
	if (rc)
		dprintk(VIDC_ERR, "Failed to set clock rate: %d\n", rc);
	return rc;
}

void msm_comm_scale_clocks_and_bus(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s Invalid params\n", __func__);
		return;
	}
	core = inst->core;
	hdev = core->device;

	if (msm_comm_scale_clocks(core)) {
		dprintk(VIDC_WARN,
				"Failed to scale clocks. Performance might be impacted\n");
	}
	if (msm_comm_vote_bus(core)) {
		dprintk(VIDC_WARN,
				"Failed to scale DDR bus. Performance might be impacted\n");
	}
}

static inline enum msm_vidc_thermal_level msm_comm_vidc_thermal_level(int level)
{
	switch (level) {
	case 0:
		return VIDC_THERMAL_NORMAL;
	case 1:
		return VIDC_THERMAL_LOW;
	case 2:
		return VIDC_THERMAL_HIGH;
	default:
		return VIDC_THERMAL_CRITICAL;
	}
}

static unsigned long msm_comm_get_clock_rate(struct msm_vidc_core *core)
{
	struct hfi_device *hdev;
	unsigned long freq = 0;

	if (!core || !core->device) {
		dprintk(VIDC_ERR, "%s Invalid params\n", __func__);
		return -EINVAL;
	}
	hdev = core->device;

	freq = call_hfi_op(hdev, get_core_clock_rate, hdev->hfi_device_data);
	dprintk(VIDC_DBG, "clock freq %ld\n", freq);

	return freq;
}

static bool is_core_turbo(struct msm_vidc_core *core, unsigned long freq)
{
	int i = 0;
	struct msm_vidc_platform_resources *res = &core->resources;
	struct load_freq_table *table = res->load_freq_tbl;
	u32 max_freq = 0;

	for (i = 0; i < res->load_freq_tbl_size; i++) {
		if (max_freq < table[i].freq)
			max_freq = table[i].freq;
	}
	return freq >= max_freq;
}

static bool is_thermal_permissible(struct msm_vidc_core *core)
{
	enum msm_vidc_thermal_level tl;
	unsigned long freq = 0;
	bool is_turbo = false;

	if (!core->resources.thermal_mitigable)
		return true;

	if (msm_vidc_thermal_mitigation_disabled) {
		dprintk(VIDC_DBG,
			"Thermal mitigation not enabled. debugfs %d\n",
			msm_vidc_thermal_mitigation_disabled);
		return true;
	}

	tl = msm_comm_vidc_thermal_level(vidc_driver->thermal_level);
	freq = msm_comm_get_clock_rate(core);

	is_turbo = is_core_turbo(core, freq);
	dprintk(VIDC_DBG,
		"Core freq %ld Thermal level %d Turbo mode %d\n",
		freq, tl, is_turbo);

	if (is_turbo && tl >= VIDC_THERMAL_LOW) {
		dprintk(VIDC_ERR,
			"Video session not allowed. Turbo mode %d Thermal level %d\n",
			is_turbo, tl);
		return false;
	}
	return true;
}

static int msm_comm_session_abort(struct msm_vidc_inst *inst)
{
	int rc = 0, abort_completion = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid params\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;
	abort_completion = SESSION_MSG_INDEX(SESSION_ABORT_DONE);
	init_completion(&inst->completions[abort_completion]);

	rc = call_hfi_op(hdev, session_abort, (void *)inst->session);
	if (rc) {
		dprintk(VIDC_ERR,
			"%s session_abort failed rc: %d\n", __func__, rc);
		return rc;
	}
	rc = wait_for_completion_timeout(
			&inst->completions[abort_completion],
			msecs_to_jiffies(msm_vidc_hw_rsp_timeout));
	if (!rc) {
		dprintk(VIDC_ERR,
				"%s: Wait interrupted or timed out [%p]: %d\n",
				__func__, inst, abort_completion);
		rc = -EBUSY;
	} else {
		rc = 0;
	}
	msm_comm_session_clean(inst);
	return rc;
}

static void handle_thermal_event(struct msm_vidc_core *core)
{
	int rc = 0;
	struct msm_vidc_inst *inst;

	if (!core || !core->device) {
		dprintk(VIDC_ERR, "%s Invalid params\n", __func__);
		return;
	}
	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (!inst->session)
			continue;

		mutex_unlock(&core->lock);
		if (inst->state >= MSM_VIDC_OPEN_DONE &&
			inst->state < MSM_VIDC_CLOSE_DONE) {
			dprintk(VIDC_WARN, "%s: abort inst %p\n",
				__func__, inst);
			rc = msm_comm_session_abort(inst);
			if (rc) {
				dprintk(VIDC_ERR,
					"%s session_abort failed rc: %d\n",
					__func__, rc);
				goto err_sess_abort;
			}
			change_inst_state(inst, MSM_VIDC_CORE_INVALID);
			dprintk(VIDC_WARN,
				"%s Send sys error for inst %p\n",
				__func__, inst);
			msm_vidc_queue_v4l2_event(inst,
					V4L2_EVENT_MSM_VIDC_SYS_ERROR);
		} else {
			msm_comm_generate_session_error(inst);
		}
		mutex_lock(&core->lock);
	}
	mutex_unlock(&core->lock);
	return;

err_sess_abort:
	msm_comm_clean_notify_client(core);
	return;
}

void msm_comm_handle_thermal_event()
{
	struct msm_vidc_core *core;

	list_for_each_entry(core, &vidc_driver->cores, list) {
		if (!is_thermal_permissible(core)) {
			dprintk(VIDC_WARN,
				"Thermal level critical, stop all active sessions!\n");
			handle_thermal_event(core);
		}
	}
}

int msm_comm_check_core_init(struct msm_vidc_core *core)
{
	int rc = 0;

	mutex_lock(&core->lock);
	if (core->state >= VIDC_CORE_INIT_DONE) {
		dprintk(VIDC_INFO, "Video core: %d is already in state: %d\n",
				core->id, core->state);
		goto exit;
	}
	dprintk(VIDC_DBG, "Waiting for SYS_INIT_DONE\n");
	rc = wait_for_completion_timeout(
		&core->completions[SYS_MSG_INDEX(SYS_INIT_DONE)],
		msecs_to_jiffies(msm_vidc_hw_rsp_timeout));
	if (!rc) {
		dprintk(VIDC_ERR, "%s: Wait interrupted or timed out: %d\n",
				__func__, SYS_MSG_INDEX(SYS_INIT_DONE));
		rc = -EIO;
		goto exit;
	} else {
		core->state = VIDC_CORE_INIT_DONE;
		rc = 0;
	}
	dprintk(VIDC_DBG, "SYS_INIT_DONE!!!\n");
exit:
	mutex_unlock(&core->lock);
	return rc;
}

static int msm_comm_init_core_done(struct msm_vidc_inst *inst)
{
	int rc = 0;

	rc = msm_comm_check_core_init(inst->core);
	if (rc) {
		dprintk(VIDC_ERR, "%s - failed to initialize core\n", __func__);
		return rc;
	}
	change_inst_state(inst, MSM_VIDC_CORE_INIT_DONE);
	return rc;
}

int msm_comm_load_fw(struct msm_vidc_core *core)
{
	int rc = 0;
	struct hfi_device *hdev;

	if (!core || !core->device)
		return -EINVAL;
	hdev = core->device;

	mutex_lock(&core->lock);
	if (core->state >= VIDC_CORE_INIT) {
		dprintk(VIDC_INFO, "Video core: %d is already in state: %d\n",
				core->id, core->state);
		goto core_already_inited;
	}
	mutex_unlock(&core->lock);

	rc = msm_comm_vote_bus(core);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to scale DDR bus: %d\n", rc);
		goto fail_vote_bus;
	}

	mutex_lock(&core->lock);
	if (core->state < VIDC_CORE_LOADED) {
		rc = call_hfi_op(hdev, load_fw, hdev->hfi_device_data);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to load video firmware\n");
			mutex_unlock(&core->lock);
			goto fail_load_fw;
		}
		core->state = VIDC_CORE_LOADED;
		dprintk(VIDC_DBG, "Firmware downloaded\n");
	}
	mutex_unlock(&core->lock);
	rc = msm_comm_scale_clocks(core);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to scale clocks: %d\n", rc);
		goto fail_core_init;
	}

	mutex_lock(&core->lock);
	if (core->state == VIDC_CORE_LOADED) {
		init_completion(&core->completions
			[SYS_MSG_INDEX(SYS_INIT_DONE)]);
		rc = call_hfi_op(hdev, core_init, hdev->hfi_device_data);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to init core, id = %d\n",
				core->id);
			mutex_unlock(&core->lock);
			goto fail_core_init;
		}
		core->state = VIDC_CORE_INIT;
	}

core_already_inited:
	mutex_unlock(&core->lock);
	return rc;

fail_core_init:
	mutex_lock(&core->lock);
	call_hfi_op(hdev, unload_fw, hdev->hfi_device_data);
	core->state = VIDC_CORE_UNINIT;
	mutex_unlock(&core->lock);
fail_load_fw:
	msm_comm_unvote_buses(core);
fail_vote_bus:
	return rc;
}

static int msm_comm_init_core(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core)
		return -EINVAL;

	rc = msm_comm_load_fw(inst->core);
	if (rc) {
		dprintk(VIDC_ERR, "%s - firmware loading failed\n", __func__);
		return rc;
	}

	change_inst_state(inst, MSM_VIDC_CORE_INIT);
	return rc;
}

static int msm_vidc_deinit_core(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	hdev = core->device;

	mutex_lock(&core->lock);
	if (core->state == VIDC_CORE_UNINIT) {
		dprintk(VIDC_INFO, "Video core: %d is already in state: %d\n",
				core->id, core->state);
		goto core_already_uninited;
	}
	mutex_unlock(&core->lock);

	msm_comm_scale_clocks_and_bus(inst);

	mutex_lock(&core->lock);

	/*
	 * If firmware is configured to be always loaded in memory,
	 * then unload it only if the core has gone in to bad state.
	 */
	if (core->resources.early_fw_load &&
		core->state != VIDC_CORE_INVALID) {
			goto core_already_uninited;
	}

	if (list_empty(&core->instances)) {
		cancel_delayed_work(&core->fw_unload_work);

		/*
		 * Delay unloading of firmware. This is useful
		 * in avoiding firmware download delays in cases where we
		 * will have a burst of back to back video playback sessions
		 * e.g. thumbnail generation.
		 */
		schedule_delayed_work(&core->fw_unload_work,
			msecs_to_jiffies(core->state == VIDC_CORE_INVALID ?
					0 : msm_vidc_firmware_unload_delay));

		dprintk(VIDC_DBG, "firmware unload delayed by %u ms\n",
			core->state == VIDC_CORE_INVALID ?
			0 : msm_vidc_firmware_unload_delay);
	}

core_already_uninited:
	change_inst_state(inst, MSM_VIDC_CORE_UNINIT);
	mutex_unlock(&core->lock);
	return 0;
}

int msm_comm_force_cleanup(struct msm_vidc_inst *inst)
{
	msm_comm_kill_session(inst);
	return msm_vidc_deinit_core(inst);
}

static int msm_comm_session_init(int flipped_state,
	struct msm_vidc_inst *inst)
{
	int rc = 0;
	int fourcc = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_OPEN)) {
		dprintk(VIDC_INFO, "inst: %p is already in state: %d\n",
						inst, inst->state);
		goto exit;
	}
	if (inst->session_type == MSM_VIDC_DECODER) {
		fourcc = inst->fmts[OUTPUT_PORT]->fourcc;
	} else if (inst->session_type == MSM_VIDC_ENCODER) {
		fourcc = inst->fmts[CAPTURE_PORT]->fourcc;
	} else {
		dprintk(VIDC_ERR, "Invalid session\n");
		return -EINVAL;
	}
	init_completion(
		&inst->completions[SESSION_MSG_INDEX(SESSION_INIT_DONE)]);

	inst->session = call_hfi_op(hdev, session_init, hdev->hfi_device_data,
			inst, get_hal_domain(inst->session_type),
			get_hal_codec_type(fourcc));

	if (!inst->session) {
		dprintk(VIDC_ERR,
			"Failed to call session init for: %p, %p, %d, %d\n",
			inst->core->device, inst,
			inst->session_type, fourcc);
		rc = -EINVAL;
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_OPEN);
exit:
	return rc;
}

static void msm_vidc_print_running_insts(struct msm_vidc_core *core)
{
	struct msm_vidc_inst *temp;
	dprintk(VIDC_ERR, "Running instances:\n");
	dprintk(VIDC_ERR, "%4s|%4s|%4s|%4s|%4s\n",
			"type", "w", "h", "fps", "prop");

	mutex_lock(&core->lock);
	list_for_each_entry(temp, &core->instances, list) {
		if (temp->state >= MSM_VIDC_OPEN_DONE &&
				temp->state < MSM_VIDC_STOP_DONE) {
			char properties[4] = "";

			if (is_thumbnail_session(temp))
				strlcat(properties, "N", sizeof(properties));

			if (msm_comm_turbo_session(temp))
				strlcat(properties, "T", sizeof(properties));

			dprintk(VIDC_ERR, "%4d|%4d|%4d|%4d|%4s\n",
					temp->session_type,
					max(temp->prop.width[CAPTURE_PORT],
						temp->prop.width[OUTPUT_PORT]),
					max(temp->prop.height[CAPTURE_PORT],
						temp->prop.height[OUTPUT_PORT]),
					temp->prop.fps, properties);
		}
	}
	mutex_unlock(&core->lock);
}

static int msm_vidc_load_resources(int flipped_state,
	struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev;
	int num_mbs_per_sec = 0;
	struct msm_vidc_core *core;
	enum load_calc_quirks quirks = LOAD_CALC_IGNORE_TURBO_LOAD |
		LOAD_CALC_IGNORE_THUMBNAIL_LOAD |
		LOAD_CALC_IGNORE_NON_REALTIME_LOAD;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	if (core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
				"Core is in bad state can't do load res\n");
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
				"Instance is in invalid state can't do load res\n");
		return -EINVAL;
	}

	num_mbs_per_sec =
		msm_comm_get_load(core, MSM_VIDC_DECODER, quirks) +
		msm_comm_get_load(core, MSM_VIDC_ENCODER, quirks);

	if (num_mbs_per_sec > core->resources.max_load) {
		dprintk(VIDC_ERR, "HW is overloaded, needed: %d max: %d\n",
			num_mbs_per_sec, core->resources.max_load);
		msm_vidc_print_running_insts(core);
		inst->state = MSM_VIDC_CORE_INVALID;
		msm_comm_kill_session(inst);
		return -EBUSY;
	}

	hdev = core->device;
	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_LOAD_RESOURCES)) {
		dprintk(VIDC_INFO, "inst: %p is already in state: %d\n",
						inst, inst->state);
		goto exit;
	}

	rc = call_hfi_op(hdev, session_load_res, (void *) inst->session);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to send load resources\n");
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_LOAD_RESOURCES);
exit:
	return rc;
}

static int msm_vidc_start(int flipped_state, struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	if (inst->state == MSM_VIDC_CORE_INVALID ||
			inst->core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
				"Core is in bad state can't do start\n");
		return -EINVAL;
	}

	hdev = inst->core->device;

	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_START)) {
		dprintk(VIDC_INFO,
			"inst: %p is already in state: %d\n",
			inst, inst->state);
		goto exit;
	}
	init_completion(
		&inst->completions[SESSION_MSG_INDEX(SESSION_START_DONE)]);
	rc = call_hfi_op(hdev, session_start, (void *) inst->session);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to send start\n");
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_START);
exit:
	return rc;
}

static int msm_vidc_stop(int flipped_state, struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_STOP)) {
		dprintk(VIDC_INFO,
			"inst: %p is already in state: %d\n",
			inst, inst->state);
		goto exit;
	}
	dprintk(VIDC_DBG, "Send Stop to hal\n");
	init_completion(
		&inst->completions[SESSION_MSG_INDEX(SESSION_STOP_DONE)]);
	rc = call_hfi_op(hdev, session_stop, (void *) inst->session);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to send stop\n");
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_STOP);
exit:
	return rc;
}

static int msm_vidc_release_res(int flipped_state, struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_RELEASE_RESOURCES)) {
		dprintk(VIDC_INFO,
			"inst: %p is already in state: %d\n",
			inst, inst->state);
		goto exit;
	}
	dprintk(VIDC_DBG,
		"Send release res to hal\n");
	init_completion(
	&inst->completions[SESSION_MSG_INDEX(SESSION_RELEASE_RESOURCE_DONE)]);
	rc = call_hfi_op(hdev, session_release_res, (void *) inst->session);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to send release resources\n");
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_RELEASE_RESOURCES);
exit:
	return rc;
}

static int msm_comm_session_close(int flipped_state,
			struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid params\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;
	if (IS_ALREADY_IN_STATE(flipped_state, MSM_VIDC_CLOSE)) {
		dprintk(VIDC_INFO,
			"inst: %p is already in state: %d\n",
						inst, inst->state);
		goto exit;
	}
	dprintk(VIDC_DBG,
		"Send session close to hal\n");
	init_completion(
		&inst->completions[SESSION_MSG_INDEX(SESSION_END_DONE)]);
	rc = call_hfi_op(hdev, session_end, (void *) inst->session);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to send close\n");
		goto exit;
	}
	change_inst_state(inst, MSM_VIDC_CLOSE);
exit:
	return rc;
}

int msm_comm_suspend(int core_id)
{
	struct hfi_device *hdev;
	struct msm_vidc_core *core;
	int rc = 0;

	core = get_vidc_core(core_id);
	if (!core) {
		dprintk(VIDC_ERR,
			"%s: Failed to find core for core_id = %d\n",
			__func__, core_id);
		return -EINVAL;
	}

	hdev = (struct hfi_device *)core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "%s Invalid device handle\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	rc = call_hfi_op(hdev, suspend, hdev->hfi_device_data);
	if (rc)
		dprintk(VIDC_WARN, "Failed to suspend\n");
	mutex_unlock(&core->lock);

	return rc;
}

static int get_flipped_state(int present_state,
	int desired_state)
{
	int flipped_state = present_state;
	if (flipped_state < MSM_VIDC_STOP
			&& desired_state > MSM_VIDC_STOP) {
		flipped_state = MSM_VIDC_STOP + (MSM_VIDC_STOP - flipped_state);
		flipped_state &= 0xFFFE;
		flipped_state = flipped_state - 1;
	} else if (flipped_state > MSM_VIDC_STOP
			&& desired_state < MSM_VIDC_STOP) {
		flipped_state = MSM_VIDC_STOP -
			(flipped_state - MSM_VIDC_STOP + 1);
		flipped_state &= 0xFFFE;
		flipped_state = flipped_state - 1;
	}
	return flipped_state;
}

struct hal_buffer_requirements *get_buff_req_buffer(
		struct msm_vidc_inst *inst, enum hal_buffer buffer_type)
{
	int i;
	for (i = 0; i < HAL_BUFFER_MAX; i++) {
		if (inst->buff_req.buffer[i].buffer_type == buffer_type)
			return &inst->buff_req.buffer[i];
	}
	return NULL;
}

static int set_output_buffers(struct msm_vidc_inst *inst,
	enum hal_buffer buffer_type)
{
	int rc = 0;
	struct msm_smem *handle;
	struct internal_buf *binfo;
	struct vidc_buffer_addr_info buffer_info = {0};
	u32 smem_flags = 0, buffer_size;
	struct hal_buffer_requirements *output_buf, *extradata_buf;
	int i;
	struct hfi_device *hdev;

	hdev = inst->core->device;

	output_buf = get_buff_req_buffer(inst, buffer_type);
	if (!output_buf) {
		dprintk(VIDC_DBG,
			"This output buffer not required, buffer_type: %x\n",
			buffer_type);
		return 0;
	}
	dprintk(VIDC_DBG,
		"output: num = %d, size = %d\n",
		output_buf->buffer_count_actual,
		output_buf->buffer_size);

	buffer_size = output_buf->buffer_size;

	extradata_buf = get_buff_req_buffer(inst, HAL_BUFFER_EXTRADATA_OUTPUT);
	if (extradata_buf) {
		dprintk(VIDC_DBG,
			"extradata: num = %d, size = %d\n",
			extradata_buf->buffer_count_actual,
			extradata_buf->buffer_size);
		buffer_size += extradata_buf->buffer_size;
	} else {
		dprintk(VIDC_DBG,
			"This extradata buffer not required, buffer_type: %x\n",
			buffer_type);
	}

	if (inst->flags & VIDC_SECURE)
		smem_flags |= SMEM_SECURE;

	if (output_buf->buffer_size) {
		for (i = 0; i < output_buf->buffer_count_actual;
				i++) {
			handle = msm_comm_smem_alloc(inst,
					buffer_size, 1, smem_flags,
					buffer_type, 0);
			if (!handle) {
				dprintk(VIDC_ERR,
					"Failed to allocate output memory\n");
				rc = -ENOMEM;
				goto err_no_mem;
			}
			rc = msm_comm_smem_cache_operations(inst,
					handle, SMEM_CACHE_CLEAN);
			if (rc) {
				dprintk(VIDC_WARN,
					"Failed to clean cache may cause undefined behavior\n");
			}
			binfo = kzalloc(sizeof(*binfo), GFP_KERNEL);
			if (!binfo) {
				dprintk(VIDC_ERR, "Out of memory\n");
				rc = -ENOMEM;
				goto fail_kzalloc;
			}

			binfo->handle = handle;
			buffer_info.buffer_size = output_buf->buffer_size;
			buffer_info.buffer_type = buffer_type;
			binfo->buffer_type = buffer_type;
			buffer_info.num_buffers = 1;
			binfo->buffer_ownership = DRIVER;
			buffer_info.align_device_addr = handle->device_addr;
			buffer_info.extradata_addr = handle->device_addr +
				output_buf->buffer_size;
			if (extradata_buf) {
				buffer_info.extradata_size =
					extradata_buf->buffer_size;
			}
			dprintk(VIDC_DBG, "Output buffer address: 0x%pa\n",
					&buffer_info.align_device_addr);
			dprintk(VIDC_DBG, "Output extradata address: 0x%pa\n",
					&buffer_info.extradata_addr);
			rc = call_hfi_op(hdev, session_set_buffers,
					(void *) inst->session, &buffer_info);
			if (rc) {
				dprintk(VIDC_ERR,
					"%s : session_set_buffers failed\n",
					__func__);
				goto fail_set_buffers;
			}
			mutex_lock(&inst->outputbufs.lock);
			list_add_tail(&binfo->list, &inst->outputbufs.list);
			mutex_unlock(&inst->outputbufs.lock);
		}
	}
	return rc;
fail_set_buffers:
	kfree(binfo);
fail_kzalloc:
	msm_comm_smem_free(inst, handle);
err_no_mem:
	return rc;
}

static inline char *get_internal_buffer_name(enum hal_buffer buffer_type)
{
	switch (buffer_type) {
	case HAL_BUFFER_INTERNAL_SCRATCH: return "scratch";
	case HAL_BUFFER_INTERNAL_SCRATCH_1: return "scratch_1";
	case HAL_BUFFER_INTERNAL_SCRATCH_2: return "scratch_2";
	case HAL_BUFFER_INTERNAL_PERSIST: return "persist";
	case HAL_BUFFER_INTERNAL_PERSIST_1: return "persist_1";
	default: return "unknown";
	}
}

static int set_internal_buf_on_fw(struct msm_vidc_inst *inst,
				enum hal_buffer buffer_type,
				struct msm_smem *handle, bool reuse)
{
	struct vidc_buffer_addr_info buffer_info;
	struct hfi_device *hdev;
	int rc = 0;

	if (!inst || !inst->core || !inst->core->device || !handle) {
		dprintk(VIDC_ERR, "%s - invalid params\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;

	rc = msm_comm_smem_cache_operations(inst,
					handle, SMEM_CACHE_CLEAN);
	if (rc) {
		dprintk(VIDC_WARN,
			"Failed to clean cache. Undefined behavior\n");
	}

	buffer_info.buffer_size = handle->size;
	buffer_info.buffer_type = buffer_type;
	buffer_info.num_buffers = 1;
	buffer_info.align_device_addr = handle->device_addr;
	dprintk(VIDC_DBG, "%s %s buffer : 0x%pa\n",
				reuse ? "Reusing" : "Allocated",
				get_internal_buffer_name(buffer_type),
				&buffer_info.align_device_addr);

	rc = call_hfi_op(hdev, session_set_buffers,
		(void *) inst->session, &buffer_info);
	if (rc) {
		dprintk(VIDC_ERR,
			"vidc_hal_session_set_buffers failed\n");
		return rc;
	}
	return 0;
}

static bool reuse_internal_buffers(struct msm_vidc_inst *inst,
		enum hal_buffer buffer_type, struct msm_vidc_list *buf_list)
{
	struct internal_buf *buf;
	int rc = 0;
	bool reused = false;

	if (!inst || !buf_list) {
		dprintk(VIDC_ERR, "%s: invalid params\n", __func__);
		return false;
	}

	mutex_lock(&buf_list->lock);
	list_for_each_entry(buf, &buf_list->list, list) {
		if (!buf->handle) {
			reused = false;
			break;
		}

		if (buf->buffer_type != buffer_type)
			continue;

		/*
		 * Persist buffer size won't change with resolution. If they
		 * are in queue means that they are already allocated and
		 * given to HW. HW can use them without reallocation. These
		 * buffers are not released as part of port reconfig. So
		 * driver no need to set them again.
		*/

		if (buffer_type != HAL_BUFFER_INTERNAL_PERSIST
			&& buffer_type != HAL_BUFFER_INTERNAL_PERSIST_1) {

			rc = set_internal_buf_on_fw(inst, buffer_type,
					buf->handle, true);
			if (rc) {
				dprintk(VIDC_ERR,
					"%s: session_set_buffers failed\n",
					__func__);
				reused = false;
				break;
			}
		}
		reused = true;
	}
	mutex_unlock(&buf_list->lock);
	return reused;
}

static int allocate_and_set_internal_bufs(struct msm_vidc_inst *inst,
			struct hal_buffer_requirements *internal_bufreq,
			struct msm_vidc_list *buf_list)
{
	struct msm_smem *handle;
	struct internal_buf *binfo;
	u32 smem_flags = 0;
	int rc = 0;
	int i = 0;

	if (!inst || !internal_bufreq || !buf_list)
		return -EINVAL;

	if (!internal_bufreq->buffer_size)
		return 0;

	if (inst->flags & VIDC_SECURE)
		smem_flags |= SMEM_SECURE;

	for (i = 0; i < internal_bufreq->buffer_count_actual; i++) {
		handle = msm_comm_smem_alloc(inst, internal_bufreq->buffer_size,
				1, smem_flags, internal_bufreq->buffer_type, 0);
		if (!handle) {
			dprintk(VIDC_ERR,
				"Failed to allocate scratch memory\n");
			rc = -ENOMEM;
			goto err_no_mem;
		}

		binfo = kzalloc(sizeof(*binfo), GFP_KERNEL);
		if (!binfo) {
			dprintk(VIDC_ERR, "Out of memory\n");
			rc = -ENOMEM;
			goto fail_kzalloc;
		}

		binfo->handle = handle;
		binfo->buffer_type = internal_bufreq->buffer_type;

		rc = set_internal_buf_on_fw(inst, internal_bufreq->buffer_type,
				handle, false);
		if (rc)
			goto fail_set_buffers;

		mutex_lock(&buf_list->lock);
		list_add_tail(&binfo->list, &buf_list->list);
		mutex_unlock(&buf_list->lock);
	}
	return rc;

fail_set_buffers:
	kfree(binfo);
fail_kzalloc:
	msm_comm_smem_free(inst, handle);
err_no_mem:
	return rc;

}

static int set_internal_buffers(struct msm_vidc_inst *inst,
	enum hal_buffer buffer_type, struct msm_vidc_list *buf_list)
{
	struct hal_buffer_requirements *internal_buf;

	internal_buf = get_buff_req_buffer(inst, buffer_type);
	if (!internal_buf) {
		dprintk(VIDC_DBG,
			"This internal buffer not required, buffer_type: %x\n",
			buffer_type);
		return 0;
	}

	dprintk(VIDC_DBG, "Buffer type %s: num = %d, size = %d\n",
		get_internal_buffer_name(buffer_type),
		internal_buf->buffer_count_actual, internal_buf->buffer_size);

	/*
	* Try reusing existing internal buffers first.
	* If it's not possible to reuse, allocate new buffers.
	*/
	if (reuse_internal_buffers(inst, buffer_type, buf_list))
		return 0;

	return allocate_and_set_internal_bufs(inst, internal_buf,
				buf_list);
}

int msm_comm_try_state(struct msm_vidc_inst *inst, int state)
{
	int rc = 0;
	int flipped_state;
	struct msm_vidc_core *core;
	if (!inst) {
		dprintk(VIDC_ERR,
				"Invalid instance pointer = %p\n", inst);
		return -EINVAL;
	}
	dprintk(VIDC_DBG,
			"Trying to move inst: %p from: 0x%x to 0x%x\n",
			inst, inst->state, state);
	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
				"Invalid core pointer = %p\n", inst);
		return -EINVAL;
	}
	mutex_lock(&inst->sync_lock);
	if (inst->state == MSM_VIDC_CORE_INVALID ||
			core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
				"Core is in bad state can't change the state\n");
		rc = -EINVAL;
		goto exit;
	}
	flipped_state = get_flipped_state(inst->state, state);
	dprintk(VIDC_DBG,
			"flipped_state = 0x%x\n", flipped_state);
	switch (flipped_state) {
	case MSM_VIDC_CORE_UNINIT_DONE:
	case MSM_VIDC_CORE_INIT:
		rc = msm_comm_init_core(inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_CORE_INIT_DONE:
		rc = msm_comm_init_core_done(inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_OPEN:
		rc = msm_comm_session_init(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_OPEN_DONE:
		rc = wait_for_state(inst, flipped_state, MSM_VIDC_OPEN_DONE,
			SESSION_INIT_DONE);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_LOAD_RESOURCES:
		rc = msm_vidc_load_resources(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_LOAD_RESOURCES_DONE:
	case MSM_VIDC_START:
		rc = msm_vidc_start(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_START_DONE:
		rc = wait_for_state(inst, flipped_state, MSM_VIDC_START_DONE,
				SESSION_START_DONE);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_STOP:
		rc = msm_vidc_stop(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_STOP_DONE:
		rc = wait_for_state(inst, flipped_state, MSM_VIDC_STOP_DONE,
				SESSION_STOP_DONE);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
		dprintk(VIDC_DBG, "Moving to Stop Done state\n");
	case MSM_VIDC_RELEASE_RESOURCES:
		rc = msm_vidc_release_res(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_RELEASE_RESOURCES_DONE:
		rc = wait_for_state(inst, flipped_state,
			MSM_VIDC_RELEASE_RESOURCES_DONE,
			SESSION_RELEASE_RESOURCE_DONE);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
		dprintk(VIDC_DBG,
				"Moving to release resources done state\n");
	case MSM_VIDC_CLOSE:
		rc = msm_comm_session_close(flipped_state, inst);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_CLOSE_DONE:
		rc = wait_for_state(inst, flipped_state, MSM_VIDC_CLOSE_DONE,
				SESSION_END_DONE);
		if (rc || state <= get_flipped_state(inst->state, state))
			break;
	case MSM_VIDC_CORE_UNINIT:
	case MSM_VIDC_CORE_INVALID:
		dprintk(VIDC_DBG, "Sending core uninit\n");
		rc = msm_vidc_deinit_core(inst);
		if (rc || state == get_flipped_state(inst->state, state))
			break;
	default:
		dprintk(VIDC_ERR, "State not recognized\n");
		rc = -EINVAL;
		break;
	}
exit:
	mutex_unlock(&inst->sync_lock);
	if (rc)
		dprintk(VIDC_ERR,
				"Failed to move from state: %d to %d\n",
				inst->state, state);
	else
		trace_msm_vidc_common_state_change((void *)inst,
				inst->state, state);
	return rc;
}

int msm_comm_compute_idle_time(struct msm_vidc_inst *inst)
{
	int j = 0, rc = 0, last_sample_index = 0;
	struct msm_vidc_idle_stats *idle_stats;
	struct msm_vidc_core *core;
	const ktime_t zero_ktime = ktime_set(0, 0);
	ktime_t cumulative_idle_time = zero_ktime;
	u64 temp;

	if (!inst || !inst->core) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	core = inst->core;

	mutex_lock(&core->lock);
	idle_stats = &core->idle_stats;
	last_sample_index = idle_stats->last_sample_index;

	idle_stats->samples[last_sample_index] = idle_stats->idle ?
		ktime_sub(ktime_get(), idle_stats->start_time) : zero_ktime;

	idle_stats->sample_count++;
	if (idle_stats->sample_count > IDLE_TIME_WINDOW_SIZE)
		idle_stats->sample_count = IDLE_TIME_WINDOW_SIZE;

	for (j = 0; j < idle_stats->sample_count; j++) {
		cumulative_idle_time = ktime_add(cumulative_idle_time,
			idle_stats->samples[j]);
	}

	temp = ktime_to_ns(cumulative_idle_time);
	do_div(temp, idle_stats->sample_count);
	idle_stats->avg_idle_time = ns_to_ktime(temp);

	idle_stats->idle = true;
	idle_stats->last_sample_index++;
	idle_stats->last_sample_index %= sizeof(idle_stats->samples);
	dprintk(VIDC_DBG, "%s: Idle stats: current %llu us, average %llu us\n",
		__func__, ktime_to_us(idle_stats->samples[last_sample_index]),
		ktime_to_us(idle_stats->avg_idle_time));
	mutex_unlock(&core->lock);

	idle_stats->fb_err_level = ktime_compare(idle_stats->avg_idle_time,
			zero_ktime) == 0 ? 3 : 0;
	if (idle_stats->fb_err_level != idle_stats->prev_fb_err_level) {
		idle_stats->prev_fb_err_level = idle_stats->fb_err_level;
		if (msm_comm_vote_bus(core)) {
			dprintk(VIDC_WARN,
			"Failed to scale DDR bus. Performance might be impacted\n");
		}
	}

	return rc;
}

int msm_comm_qbuf(struct vb2_buffer *vb)
{
	int rc = 0;
	struct vb2_queue *q;
	struct msm_vidc_inst *inst;
	struct vb2_buf_entry *entry;
	struct vidc_frame_data frame_data;
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	int extra_idx = 0;

	if (!vb || !vb->vb2_queue)  {
		dprintk(VIDC_ERR, "%s: Invalid input: %p\n",
			__func__, vb);
		return -EINVAL;
	}

	q = vb->vb2_queue;
	inst = q->drv_priv;
	if (!inst) {
		dprintk(VIDC_ERR, "%s: Invalid input: %p\n",
			__func__, vb);
		return -EINVAL;
	}

	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
			"Invalid input: %p, %p, %p\n", inst, core, vb);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "%s: Invalid input: %p\n",
			__func__, hdev);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_CORE_INVALID ||
		core->state == VIDC_CORE_INVALID ||
		core->state == VIDC_CORE_UNINIT) {
		dprintk(VIDC_ERR, "Core is in bad state. Can't Queue\n");
		return -EINVAL;
	}
	if (inst->state != MSM_VIDC_START_DONE) {
		entry = kzalloc(sizeof(*entry), GFP_KERNEL);
		if (!entry) {
			dprintk(VIDC_ERR, "Out of memory\n");
			goto err_no_mem;
		}
		entry->vb = vb;

		mutex_lock(&inst->pendingq.lock);
		list_add_tail(&entry->list, &inst->pendingq.list);
		mutex_unlock(&inst->pendingq.lock);
	} else {
		int64_t time_usec = timeval_to_ns(&vb->v4l2_buf.timestamp);
		do_div(time_usec, NSEC_PER_USEC);
		memset(&frame_data, 0 , sizeof(struct vidc_frame_data));
		frame_data.alloc_len = vb->v4l2_planes[0].length;
		frame_data.filled_len = vb->v4l2_planes[0].bytesused;
		frame_data.offset = vb->v4l2_planes[0].data_offset;
		frame_data.device_addr = vb->v4l2_planes[0].m.userptr;
		frame_data.timestamp = time_usec;
		frame_data.flags = 0;
		frame_data.clnt_data = frame_data.device_addr;
		if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE &&
			(frame_data.filled_len > frame_data.alloc_len ||
			frame_data.offset > frame_data.alloc_len)) {
			dprintk(VIDC_ERR,
				"Buffer will overflow, not queueing it\n");
			rc = -EINVAL;
			goto err_bad_input;
		}

		if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
			frame_data.buffer_type = HAL_BUFFER_INPUT;
			if (vb->v4l2_buf.flags & V4L2_QCOM_BUF_FLAG_EOS) {
				frame_data.flags |= HAL_BUFFERFLAG_EOS;
				dprintk(VIDC_DBG,
					"Received EOS on output capability\n");
			}

			if (vb->v4l2_buf.flags &
				V4L2_MSM_BUF_FLAG_YUV_601_709_CLAMP) {
				frame_data.flags |=
					HAL_BUFFERFLAG_YUV_601_709_CSC_CLAMP;
				dprintk(VIDC_DBG,
					"Received buff with 601to709 clamp\n");
			}

			if (vb->v4l2_buf.flags &
					V4L2_QCOM_BUF_FLAG_CODECCONFIG) {
				frame_data.flags |= HAL_BUFFERFLAG_CODECCONFIG;
				dprintk(VIDC_DBG,
					"Received CODECCONFIG on output cap\n");
			}

			if (vb->v4l2_buf.flags &
					V4L2_QCOM_BUF_FLAG_DECODEONLY) {
				frame_data.flags |= HAL_BUFFERFLAG_DECODEONLY;
				dprintk(VIDC_DBG,
					"Received DECODEONLY on output cap\n");
			}

			if (vb->v4l2_buf.flags &
				V4L2_QCOM_BUF_TIMESTAMP_INVALID)
				frame_data.timestamp = LLONG_MAX;

			if (vb->v4l2_buf.flags &
					V4L2_QCOM_BUF_TS_DISCONTINUITY) {
				frame_data.flags |=
					HAL_BUFFERFLAG_TS_DISCONTINUITY;
				dprintk(VIDC_DBG,
					"Received TS_DISCONTINUE on output\n");
			}

			if (vb->v4l2_buf.flags & V4L2_QCOM_BUF_TS_ERROR) {
				frame_data.flags |=
					HAL_BUFFERFLAG_TS_ERROR;
				dprintk(VIDC_DBG,
					"Received TS_ERROR on output cap\n");
			}

			extra_idx =
				EXTRADATA_IDX(inst->fmts[OUTPUT_PORT]->
					num_planes);
			if (extra_idx && (extra_idx < VIDEO_MAX_PLANES) &&
					vb->v4l2_planes[extra_idx].m.userptr) {
				frame_data.extradata_addr =
					vb->v4l2_planes[extra_idx].m.userptr;
				frame_data.flags |= HAL_BUFFERFLAG_EXTRADATA;
			}

			dprintk(VIDC_DBG,
				"Sending etb to hal: device_addr: 0x%pa, alloc: %d, filled: %d, offset: %d, ts: %lld, flags = 0x%x, v4l2_buf index = %d\n",
				&frame_data.device_addr, frame_data.alloc_len,
				frame_data.filled_len, frame_data.offset,
				frame_data.timestamp, frame_data.flags,
				vb->v4l2_buf.index);

			if (core->resources.dynamic_bw_update)
				msm_comm_compute_idle_time(inst);

			msm_dcvs_check_and_scale_clocks(inst, true);
			rc = call_hfi_op(hdev, session_etb, (void *)
					inst->session, &frame_data);
			if (!rc)
				msm_vidc_debugfs_update(inst,
					MSM_VIDC_DEBUGFS_EVENT_ETB);
			dprintk(VIDC_DBG, "Sent etb to HAL\n");
		} else if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			struct vidc_seq_hdr seq_hdr;
			frame_data.filled_len = 0;
			frame_data.offset = 0;
			frame_data.alloc_len = vb->v4l2_planes[0].length;
			frame_data.buffer_type =
				msm_comm_get_hal_output_buffer(inst);

			extra_idx =
			EXTRADATA_IDX(inst->fmts[CAPTURE_PORT]->num_planes);
			if (extra_idx && (extra_idx < VIDEO_MAX_PLANES) &&
				vb->v4l2_planes[extra_idx].m.userptr) {
				frame_data.extradata_addr =
					vb->v4l2_planes[extra_idx].m.userptr;
				frame_data.extradata_size =
					vb->v4l2_planes[extra_idx].length;
			}

			dprintk(VIDC_DBG,
				"Sending ftb to hal: device_addr: 0x%pa, alloc: %d, buffer_type: %d, ts: %lld, flags = 0x%x, v4l2_buf index = %d\n",
				&frame_data.device_addr, frame_data.alloc_len,
				frame_data.buffer_type, frame_data.timestamp,
				frame_data.flags, vb->v4l2_buf.index);

			if (core->resources.dynamic_bw_update)
				msm_comm_compute_idle_time(inst);

			if (atomic_read(&inst->seq_hdr_reqs) &&
			   inst->session_type == MSM_VIDC_ENCODER) {
				seq_hdr.seq_hdr = vb->v4l2_planes[0].
					m.userptr;
				seq_hdr.seq_hdr_len = vb->v4l2_planes[0].length;
				rc = call_hfi_op(hdev, session_get_seq_hdr,
					(void *) inst->session, &seq_hdr);
				if (!rc) {
					inst->vb2_seq_hdr = vb;
					dprintk(VIDC_DBG, "Seq_hdr: %p\n",
						inst->vb2_seq_hdr);
				}
				atomic_dec(&inst->seq_hdr_reqs);
			} else {
				msm_dcvs_check_and_scale_clocks(inst, false);
				rc = call_hfi_op(hdev, session_ftb,
					(void *) inst->session, &frame_data);
				if (!rc)
					msm_vidc_debugfs_update(inst,
						MSM_VIDC_DEBUGFS_EVENT_FTB);
			}
		} else {
			dprintk(VIDC_ERR,
				"This capability is not supported: %d\n",
				q->type);
			rc = -EINVAL;
		}
	}
err_bad_input:
	if (rc)
		dprintk(VIDC_ERR, "Failed to queue buffer\n");
err_no_mem:
	return rc;
}

int msm_comm_try_get_bufreqs(struct msm_vidc_inst *inst)
{
	struct buffer_requirements buf_req;
	int rc = 0;
	int i = 0;
	union hal_get_property hprop;

	rc = msm_comm_try_get_prop(inst,
					HAL_PARAM_GET_BUFFER_REQUIREMENTS,
					&hprop);
	if (rc) {
		dprintk(VIDC_ERR, "%s Error rc:%d\n", __func__, rc);
		return rc;
	}
	buf_req = hprop.buf_req;
	memcpy(&inst->buff_req, &buf_req,
			sizeof(struct buffer_requirements));
	for (i = 0; i < HAL_BUFFER_MAX; i++) {
		dprintk(VIDC_DBG,
				"buffer type: %d, count : %d, size: %d\n",
				inst->buff_req.buffer[i].buffer_type,
				inst->buff_req.buffer[i].buffer_count_actual,
				inst->buff_req.buffer[i].buffer_size);
	}
	dprintk(VIDC_PROF, "Input buffers: %d, Output buffers: %d\n",
			inst->buff_req.buffer[0].buffer_count_actual,
			inst->buff_req.buffer[1].buffer_count_actual);
	return rc;
}

int msm_comm_try_get_prop(struct msm_vidc_inst *inst, enum hal_property ptype,
				union hal_get_property *hprop)
{
	int rc = 0;
	struct hfi_device *hdev;
	struct getprop_buf *buf;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_CORE_INVALID ||
			inst->core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
			"Core is in bad state can't query get_bufreqs()\n");
		return -EAGAIN;
	}
	hdev = inst->core->device;
	mutex_lock(&inst->sync_lock);
	if (inst->state < MSM_VIDC_OPEN_DONE || inst->state >= MSM_VIDC_CLOSE) {
		dprintk(VIDC_ERR,
			"%s Not in proper state\n", __func__);
		rc = -EAGAIN;
		goto exit;
	}
	init_completion(
		&inst->completions[SESSION_MSG_INDEX(SESSION_PROPERTY_INFO)]);
	switch (ptype) {
	case HAL_PARAM_PROFILE_LEVEL_CURRENT:
		rc = call_hfi_op(hdev, session_get_property,
					(void *) inst->session, ptype);
		if (rc) {
			dprintk(VIDC_ERR, "%s Failed: PROFILE_LEVEL_INFO\n",
						__func__);
			rc = -EAGAIN;
			goto exit;
		}
		break;
	case HAL_PARAM_GET_BUFFER_REQUIREMENTS:
		rc = call_hfi_op(hdev, session_get_buf_req,
						(void *) inst->session);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to get property\n");
			rc = -EAGAIN;
			goto exit;
		}
		break;
	default:
		rc = -EAGAIN;
		dprintk(VIDC_ERR, "%s id:%d not supported\n", __func__, ptype);
		break;
	}
	rc = wait_for_completion_timeout(
		&inst->completions[SESSION_MSG_INDEX(SESSION_PROPERTY_INFO)],
		msecs_to_jiffies(msm_vidc_hw_rsp_timeout));
	if (!rc) {
		dprintk(VIDC_ERR,
			"%s: Wait interrupted or timed out [%p]: %d\n",
			__func__, inst,
			SESSION_MSG_INDEX(SESSION_PROPERTY_INFO));
		inst->state = MSM_VIDC_CORE_INVALID;
		msm_comm_kill_session(inst);
		rc = -EIO;
		goto exit;
	}

	mutex_lock(&inst->pending_getpropq.lock);
	if (!list_empty(&inst->pending_getpropq.list)) {
		buf = list_first_entry(&inst->pending_getpropq.list,
					struct getprop_buf, list);
		*hprop = *((union hal_get_property *) buf->data);
		kfree(buf->data);
		list_del(&buf->list);
		kfree(buf);
		rc = 0;
	} else {
		dprintk(VIDC_ERR, "%s getprop list empty\n", __func__);
		rc = -EINVAL;
	}
	mutex_unlock(&inst->pending_getpropq.lock);
exit:
	mutex_unlock(&inst->sync_lock);
	return rc;
}

int msm_comm_release_output_buffers(struct msm_vidc_inst *inst)
{
	struct msm_smem *handle;
	struct internal_buf *buf, *dummy;
	struct vidc_buffer_addr_info buffer_info;
	int rc = 0;
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	if (!inst) {
		dprintk(VIDC_ERR,
				"Invalid instance pointer = %p\n", inst);
		return -EINVAL;
	}
	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
				"Invalid core pointer = %p\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "Invalid device pointer = %p\n", hdev);
		return -EINVAL;
	}
	mutex_lock(&inst->outputbufs.lock);
	list_for_each_entry_safe(buf, dummy, &inst->outputbufs.list, list) {
		handle = buf->handle;
		if (!handle) {
			dprintk(VIDC_ERR, "%s - invalid handle\n", __func__);
			goto exit;
		}

		buffer_info.buffer_size = handle->size;
		buffer_info.buffer_type = buf->buffer_type;
		buffer_info.num_buffers = 1;
		buffer_info.align_device_addr = handle->device_addr;
		if (inst->state != MSM_VIDC_CORE_INVALID &&
				core->state != VIDC_CORE_INVALID) {
			buffer_info.response_required = false;
			rc = call_hfi_op(hdev, session_release_buffers,
				(void *)inst->session, &buffer_info);
			if (rc) {
				dprintk(VIDC_WARN,
					"Rel output buf fail:0x%pa, %d\n",
					&buffer_info.align_device_addr,
					buffer_info.buffer_size);
			}
		}

		list_del(&buf->list);
		msm_comm_smem_free(inst, buf->handle);
		kfree(buf);
	}

exit:
	mutex_unlock(&inst->outputbufs.lock);
	return rc;
}

static enum hal_buffer scratch_buf_sufficient(struct msm_vidc_inst *inst,
				enum hal_buffer buffer_type)
{
	struct hal_buffer_requirements *bufreq = NULL;
	struct internal_buf *buf;
	int count = 0;

	if (!inst) {
		dprintk(VIDC_ERR, "%s - invalid param\n", __func__);
		goto not_sufficient;
	}

	bufreq = get_buff_req_buffer(inst, buffer_type);
	if (!bufreq)
		goto not_sufficient;

	/* Check if current scratch buffers are sufficient */
	mutex_lock(&inst->scratchbufs.lock);

	list_for_each_entry(buf, &inst->scratchbufs.list, list) {
		if (!buf->handle) {
			dprintk(VIDC_ERR, "%s: invalid buf handle\n", __func__);
			mutex_unlock(&inst->scratchbufs.lock);
			goto not_sufficient;
		}
		if (buf->buffer_type == buffer_type &&
			buf->handle->size >= bufreq->buffer_size)
			count++;
	}
	mutex_unlock(&inst->scratchbufs.lock);

	if (count != bufreq->buffer_count_actual)
		goto not_sufficient;

	dprintk(VIDC_DBG,
		"Existing scratch buffer is sufficient for buffer type 0x%x\n",
		buffer_type);

	return buffer_type;

not_sufficient:
	return HAL_BUFFER_NONE;
}

int msm_comm_release_scratch_buffers(struct msm_vidc_inst *inst,
					bool check_for_reuse)
{
	struct msm_smem *handle;
	struct internal_buf *buf, *dummy;
	struct vidc_buffer_addr_info buffer_info;
	int rc = 0;
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	enum hal_buffer sufficiency = HAL_BUFFER_NONE;
	if (!inst) {
		dprintk(VIDC_ERR,
				"Invalid instance pointer = %p\n", inst);
		return -EINVAL;
	}
	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
				"Invalid core pointer = %p\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "Invalid device pointer = %p\n", hdev);
		return -EINVAL;
	}

	if (check_for_reuse) {
		sufficiency |= scratch_buf_sufficient(inst,
					HAL_BUFFER_INTERNAL_SCRATCH);

		sufficiency |= scratch_buf_sufficient(inst,
					HAL_BUFFER_INTERNAL_SCRATCH_1);

		sufficiency |= scratch_buf_sufficient(inst,
					HAL_BUFFER_INTERNAL_SCRATCH_2);
	}

	mutex_lock(&inst->scratchbufs.lock);
	list_for_each_entry_safe(buf, dummy, &inst->scratchbufs.list, list) {
		if (!buf->handle) {
			dprintk(VIDC_ERR, "%s - buf->handle NULL\n", __func__);
			rc = -EINVAL;
			goto exit;
		}

		handle = buf->handle;
		buffer_info.buffer_size = handle->size;
		buffer_info.buffer_type = buf->buffer_type;
		buffer_info.num_buffers = 1;
		buffer_info.align_device_addr = handle->device_addr;
		if (inst->state != MSM_VIDC_CORE_INVALID &&
				core->state != VIDC_CORE_INVALID) {
			buffer_info.response_required = true;
			init_completion(&inst->completions[SESSION_MSG_INDEX
			   (SESSION_RELEASE_BUFFER_DONE)]);
			rc = call_hfi_op(hdev, session_release_buffers,
				(void *)inst->session, &buffer_info);
			if (rc) {
				dprintk(VIDC_WARN,
					"Rel scrtch buf fail:0x%pa, %d\n",
					&buffer_info.align_device_addr,
					buffer_info.buffer_size);
			}
			mutex_unlock(&inst->scratchbufs.lock);
			rc = wait_for_sess_signal_receipt(inst,
				SESSION_RELEASE_BUFFER_DONE);
			if (rc) {
				change_inst_state(inst,
					MSM_VIDC_CORE_INVALID);
				msm_comm_kill_session(inst);
			}
			mutex_lock(&inst->scratchbufs.lock);
		}

		/*If scratch buffers can be reused, do not free the buffers*/
		if (sufficiency & buf->buffer_type)
			continue;

		list_del(&buf->list);
		msm_comm_smem_free(inst, buf->handle);
		kfree(buf);
	}

exit:
	mutex_unlock(&inst->scratchbufs.lock);
	return rc;
}

int msm_comm_release_persist_buffers(struct msm_vidc_inst *inst)
{
	struct msm_smem *handle;
	struct list_head *ptr, *next;
	struct internal_buf *buf;
	struct vidc_buffer_addr_info buffer_info;
	int rc = 0;
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	if (!inst) {
		dprintk(VIDC_ERR,
				"Invalid instance pointer = %p\n", inst);
		return -EINVAL;
	}
	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
				"Invalid core pointer = %p\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "Invalid device pointer = %p\n", hdev);
		return -EINVAL;
	}

	mutex_lock(&inst->persistbufs.lock);
	list_for_each_safe(ptr, next, &inst->persistbufs.list) {
		buf = list_entry(ptr, struct internal_buf, list);
		handle = buf->handle;
		buffer_info.buffer_size = handle->size;
		buffer_info.buffer_type = buf->buffer_type;
		buffer_info.num_buffers = 1;
		buffer_info.align_device_addr = handle->device_addr;
		if (inst->state != MSM_VIDC_CORE_INVALID &&
				core->state != VIDC_CORE_INVALID) {
			buffer_info.response_required = true;
			init_completion(
			   &inst->completions[SESSION_MSG_INDEX
			   (SESSION_RELEASE_BUFFER_DONE)]);
			rc = call_hfi_op(hdev, session_release_buffers,
				(void *)inst->session, &buffer_info);
			if (rc) {
				dprintk(VIDC_WARN,
					"Rel prst buf fail:0x%pa, %d\n",
					&buffer_info.align_device_addr,
					buffer_info.buffer_size);
			}
			mutex_unlock(&inst->persistbufs.lock);
			rc = wait_for_sess_signal_receipt(inst,
				SESSION_RELEASE_BUFFER_DONE);
			if (rc) {
				change_inst_state(inst, MSM_VIDC_CORE_INVALID);
				msm_comm_kill_session(inst);
			}
			mutex_lock(&inst->persistbufs.lock);
		}
		list_del(&buf->list);
		msm_comm_smem_free(inst, buf->handle);
		kfree(buf);
	}
	mutex_unlock(&inst->persistbufs.lock);
	return rc;
}

int msm_comm_try_set_prop(struct msm_vidc_inst *inst,
	enum hal_property ptype, void *pdata)
{
	int rc = 0;
	struct hfi_device *hdev;
	if (!inst) {
		dprintk(VIDC_ERR, "Invalid input: %p\n", inst);
		return -EINVAL;
	}

	if (!inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	mutex_lock(&inst->sync_lock);
	if (inst->state < MSM_VIDC_OPEN_DONE || inst->state >= MSM_VIDC_CLOSE) {
		dprintk(VIDC_ERR, "Not in proper state to set property\n");
		rc = -EAGAIN;
		goto exit;
	}
	rc = call_hfi_op(hdev, session_set_property, (void *)inst->session,
			ptype, pdata);
	if (rc)
		dprintk(VIDC_ERR, "Failed to set hal property for framesize\n");
exit:
	mutex_unlock(&inst->sync_lock);
	return rc;
}

int msm_comm_set_output_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (msm_comm_release_output_buffers(inst))
		dprintk(VIDC_WARN, "Failed to release output buffers\n");

	rc = set_output_buffers(inst, HAL_BUFFER_OUTPUT);
	if (rc)
		goto error;
	return rc;
error:
	msm_comm_release_output_buffers(inst);
	return rc;
}

int msm_comm_set_scratch_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (msm_comm_release_scratch_buffers(inst, true))
		dprintk(VIDC_WARN, "Failed to release scratch buffers\n");

	rc = set_internal_buffers(inst, HAL_BUFFER_INTERNAL_SCRATCH,
		&inst->scratchbufs);
	if (rc)
		goto error;

	rc = set_internal_buffers(inst, HAL_BUFFER_INTERNAL_SCRATCH_1,
		&inst->scratchbufs);
	if (rc)
		goto error;

	rc = set_internal_buffers(inst, HAL_BUFFER_INTERNAL_SCRATCH_2,
		&inst->scratchbufs);
	if (rc)
		goto error;

	return rc;
error:
	msm_comm_release_scratch_buffers(inst, false);
	return rc;
}

int msm_comm_set_persist_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	rc = set_internal_buffers(inst, HAL_BUFFER_INTERNAL_PERSIST,
		&inst->persistbufs);
	if (rc)
		goto error;

	rc = set_internal_buffers(inst, HAL_BUFFER_INTERNAL_PERSIST_1,
		&inst->persistbufs);
	if (rc)
		goto error;
	return rc;
error:
	msm_comm_release_persist_buffers(inst);
	return rc;
}

static void msm_comm_flush_in_invalid_state(struct msm_vidc_inst *inst)
{
	struct list_head *ptr, *next;
	enum vidc_ports ports[] = {OUTPUT_PORT, CAPTURE_PORT};
	int c = 0;

	for (c = 0; c < ARRAY_SIZE(ports); ++c) {
		enum vidc_ports port = ports[c];

		dprintk(VIDC_DBG, "Flushing buffers of type %d in bad state\n",
				port);
		list_for_each_safe(ptr, next, &inst->bufq[port].
				vb2_bufq.queued_list) {
			struct vb2_buffer *vb = container_of(ptr,
					struct vb2_buffer, queued_entry);

			vb->v4l2_planes[0].bytesused = 0;
			vb->v4l2_planes[0].data_offset = 0;

			mutex_lock(&inst->bufq[port].lock);
			vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
			mutex_unlock(&inst->bufq[port].lock);
		}
	}

	msm_vidc_queue_v4l2_event(inst, V4L2_EVENT_MSM_VIDC_FLUSH_DONE);
	return;
}

void msm_comm_flush_dynamic_buffers(struct msm_vidc_inst *inst)
{
	struct buffer_info *binfo = NULL;

	if (inst->buffer_mode_set[CAPTURE_PORT] != HAL_BUFFER_MODE_DYNAMIC)
		return;

	/*
	* dynamic buffer mode:- if flush is called during seek
	* driver should not queue any new buffer it has been holding.
	*
	* Each dynamic o/p buffer can have one of following ref_count:
	* ref_count : 0 - f/w has released reference and sent fbd back.
	*		  The buffer has been returned back to client.
	*
	* ref_count : 1 - f/w is holding reference. f/w may have released
	*                 fbd as read_only OR fbd is pending. f/w will
	*		  release reference before sending flush_done.
	*
	* ref_count : 2 - f/w is holding reference, f/w has released fbd as
	*                 read_only, which client has queued back to driver.
	*                 driver holds this buffer and will queue back
	*                 only when f/w releases the reference. During
	*		  flush_done, f/w will release the reference but driver
	*		  should not queue back the buffer to f/w.
	*		  Flush all buffers with ref_count 2.
	*/
	mutex_lock(&inst->registeredbufs.lock);
	if (!list_empty(&inst->registeredbufs.list)) {
		struct v4l2_event buf_event = {0};
		u32 *ptr = NULL;

		list_for_each_entry(binfo, &inst->registeredbufs.list, list) {
			if ((binfo->type ==
				V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
				(atomic_read(&binfo->ref_count) == 2)) {

				atomic_dec(&binfo->ref_count);
				buf_event.type =
				V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER;
				ptr = (u32 *)buf_event.u.data;
				ptr[0] = binfo->fd[0];
				ptr[1] = binfo->buff_off[0];
				ptr[2] = binfo->uvaddr[0];
				ptr[3] = (u32) binfo->timestamp.tv_sec;
				ptr[4] = (u32) binfo->timestamp.tv_usec;
				ptr[5] = binfo->v4l2_index;
				dprintk(VIDC_DBG,
					"released buffer held in driver before issuing flush: 0x%pa fd[0]: %d\n",
					&binfo->device_addr[0], binfo->fd[0]);
				/*send event to client*/
				v4l2_event_queue_fh(&inst->event_handler,
					&buf_event);
				wake_up(&inst->kernel_event_queue);
			}
		}
	}
	mutex_unlock(&inst->registeredbufs.lock);
}

void msm_comm_flush_pending_dynamic_buffers(struct msm_vidc_inst *inst)
{
	struct buffer_info *binfo = NULL;

	if (!inst)
		return;

	if (inst->buffer_mode_set[CAPTURE_PORT] != HAL_BUFFER_MODE_DYNAMIC)
		return;

	if (list_empty(&inst->pendingq.list) ||
		list_empty(&inst->registeredbufs.list))
		return;

	/*
	* Dynamic Buffer mode - Since pendingq is not empty
	* no output buffers have been sent to firmware yet.
	* Hence remove reference to all pendingq o/p buffers
	* before flushing them.
	*/

	mutex_lock(&inst->registeredbufs.lock);
	list_for_each_entry(binfo, &inst->registeredbufs.list, list) {
		if (binfo->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			dprintk(VIDC_DBG,
				"%s: binfo = %p device_addr = 0x%pa\n",
				__func__, binfo, &binfo->device_addr[0]);
			buf_ref_put(inst, binfo);
		}
	}
	mutex_unlock(&inst->registeredbufs.lock);
}

int msm_comm_flush(struct msm_vidc_inst *inst, u32 flags)
{
	int rc =  0;
	bool ip_flush = false;
	bool op_flush = false;
	struct list_head *ptr, *next;
	struct vb2_buf_entry *temp;
	struct mutex *lock;
	struct msm_vidc_core *core;
	struct hfi_device *hdev;
	if (!inst) {
		dprintk(VIDC_ERR,
				"Invalid instance pointer = %p\n", inst);
		return -EINVAL;
	}
	core = inst->core;
	if (!core) {
		dprintk(VIDC_ERR,
				"Invalid core pointer = %p\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(VIDC_ERR, "Invalid device pointer = %p\n", hdev);
		return -EINVAL;
	}

	ip_flush = flags & V4L2_QCOM_CMD_FLUSH_OUTPUT;
	op_flush = flags & V4L2_QCOM_CMD_FLUSH_CAPTURE;

	if (ip_flush && !op_flush) {
		dprintk(VIDC_INFO, "Input only flush not supported\n");
		return 0;
	}

	msm_comm_flush_dynamic_buffers(inst);
	if (inst->state == MSM_VIDC_CORE_INVALID ||
		core->state == VIDC_CORE_UNINIT ||
		core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
				"Core %p and inst %p are in bad state\n",
					core, inst);
		msm_comm_flush_in_invalid_state(inst);
		return 0;
	}

	if (inst->in_reconfig && !ip_flush && op_flush) {
		mutex_lock(&inst->pendingq.lock);
		if (!list_empty(&inst->pendingq.list)) {
			/*
			 * Execution can never reach here since port reconfig
			 * wont happen unless pendingq is emptied out
			 * (both pendingq and flush being secured with same
			 * lock). Printing a message here incase this breaks.
			 */
			dprintk(VIDC_WARN,
			"FLUSH BUG: Pending q not empty! It should be empty\n");
		}
		mutex_unlock(&inst->pendingq.lock);
		rc = call_hfi_op(hdev, session_flush, inst->session,
				HAL_FLUSH_OUTPUT);
		if (!rc && (msm_comm_get_stream_output_mode(inst) ==
			HAL_VIDEO_DECODER_SECONDARY))
			rc = call_hfi_op(hdev, session_flush, inst->session,
				HAL_FLUSH_OUTPUT2);

	} else {
		msm_comm_flush_pending_dynamic_buffers(inst);
		/*
		 * If flush is called after queueing buffers but before
		 * streamon driver should flush the pending queue
		 */
		mutex_lock(&inst->pendingq.lock);
		list_for_each_safe(ptr, next, &inst->pendingq.list) {
			temp =
			list_entry(ptr, struct vb2_buf_entry, list);
			if (temp->vb->v4l2_buf.type ==
				V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
				lock = &inst->bufq[CAPTURE_PORT].lock;
			else
				lock = &inst->bufq[OUTPUT_PORT].lock;
			temp->vb->v4l2_planes[0].bytesused = 0;
			mutex_lock(lock);
			vb2_buffer_done(temp->vb, VB2_BUF_STATE_DONE);
			mutex_unlock(lock);
			list_del(&temp->list);
			kfree(temp);
		}
		mutex_unlock(&inst->pendingq.lock);

		/*Do not send flush in case of session_error */
		if (!(inst->state == MSM_VIDC_CORE_INVALID &&
			  core->state != VIDC_CORE_INVALID))
			rc = call_hfi_op(hdev, session_flush, inst->session,
				HAL_FLUSH_ALL);
	}

	return rc;
}


enum hal_extradata_id msm_comm_get_hal_extradata_index(
	enum v4l2_mpeg_vidc_extradata index)
{
	int ret = 0;
	switch (index) {
	case V4L2_MPEG_VIDC_EXTRADATA_NONE:
		ret = HAL_EXTRADATA_NONE;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_MB_QUANTIZATION:
		ret = HAL_EXTRADATA_MB_QUANTIZATION;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_INTERLACE_VIDEO:
		ret = HAL_EXTRADATA_INTERLACE_VIDEO;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_VC1_FRAMEDISP:
		ret = HAL_EXTRADATA_VC1_FRAMEDISP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_VC1_SEQDISP:
		ret = HAL_EXTRADATA_VC1_SEQDISP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_TIMESTAMP:
		ret = HAL_EXTRADATA_TIMESTAMP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_S3D_FRAME_PACKING:
		ret = HAL_EXTRADATA_S3D_FRAME_PACKING;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_FRAME_RATE:
		ret = HAL_EXTRADATA_FRAME_RATE;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_PANSCAN_WINDOW:
		ret = HAL_EXTRADATA_PANSCAN_WINDOW;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_RECOVERY_POINT_SEI:
		ret = HAL_EXTRADATA_RECOVERY_POINT_SEI;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_MULTISLICE_INFO:
		ret = HAL_EXTRADATA_MULTISLICE_INFO;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_NUM_CONCEALED_MB:
		ret = HAL_EXTRADATA_NUM_CONCEALED_MB;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_METADATA_FILLER:
		ret = HAL_EXTRADATA_METADATA_FILLER;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_ASPECT_RATIO:
		ret = HAL_EXTRADATA_ASPECT_RATIO;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_INPUT_CROP:
		ret = HAL_EXTRADATA_INPUT_CROP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_DIGITAL_ZOOM:
		ret = HAL_EXTRADATA_DIGITAL_ZOOM;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_MPEG2_SEQDISP:
		ret = HAL_EXTRADATA_MPEG2_SEQDISP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_STREAM_USERDATA:
		ret = HAL_EXTRADATA_STREAM_USERDATA;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_FRAME_QP:
		ret = HAL_EXTRADATA_FRAME_QP;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_FRAME_BITS_INFO:
		ret = HAL_EXTRADATA_FRAME_BITS_INFO;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_LTR:
		ret = HAL_EXTRADATA_LTR_INFO;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_METADATA_MBI:
		ret = HAL_EXTRADATA_METADATA_MBI;
		break;
	case V4L2_MPEG_VIDC_EXTRADATA_VUI_DISPLAY:
		ret = HAL_EXTRADATA_VUI_DISPLAY_INFO;
		break;
	default:
		dprintk(VIDC_WARN, "Extradata not found: %d\n", index);
		break;
	}
	return ret;
};

enum hal_buffer_layout_type msm_comm_get_hal_buffer_layout(
	enum v4l2_mpeg_vidc_video_mvc_layout index)
{
	int ret = 0;
	switch (index) {
	case V4L2_MPEG_VIDC_VIDEO_MVC_SEQUENTIAL:
		ret = HAL_BUFFER_LAYOUT_SEQ;
		break;
	case V4L2_MPEG_VIDC_VIDEO_MVC_TOP_BOTTOM:
		ret = HAL_BUFFER_LAYOUT_TOP_BOTTOM;
		break;
	default:
		break;
	}
	return ret;
}

int msm_comm_get_domain_partition(struct msm_vidc_inst *inst, u32 flags,
	enum v4l2_buf_type buf_type, int *domain, int *partition)
{
	struct hfi_device *hdev;
	u32 hal_buffer_type = 0;
	if (!inst || !inst->core || !inst->core->device)
		return -EINVAL;

	hdev = inst->core->device;

	/*
	 * TODO: Due to the way in which the underlying smem mechanism
	 * maps buffer types to corresponding IOMMU domains, we need to
	 * pass in HAL_BUFFER_OUTPUT for input buffers (and vice versa)
	 * so that buffers are mapped into the correct domains. In the
	 * future, we should try to remove this workaround.
	 */
	switch (buf_type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		hal_buffer_type = (inst->session_type == MSM_VIDC_ENCODER) ?
			HAL_BUFFER_INPUT : HAL_BUFFER_OUTPUT;
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		hal_buffer_type = (inst->session_type == MSM_VIDC_ENCODER) ?
			HAL_BUFFER_OUTPUT : HAL_BUFFER_INPUT;
		break;
	default:
		dprintk(VIDC_ERR, "v4l2 buf type not found %d\n", buf_type);
		return -ENOTSUPP;
	}
	return call_hfi_op(hdev, iommu_get_domain_partition,
		hdev->hfi_device_data, flags, hal_buffer_type, domain,
		partition);
};

int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
	enum hal_ssr_trigger_type type)
{
	int rc = 0;
	struct hfi_device *hdev;
	if (!core || !core->device) {
		dprintk(VIDC_WARN, "Invalid parameters: %p\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (core->state == VIDC_CORE_INIT_DONE)
		rc = call_hfi_op(hdev, core_trigger_ssr,
				hdev->hfi_device_data, type);
	return rc;
}

static int msm_vidc_load_supported(struct msm_vidc_inst *inst)
{
	int num_mbs_per_sec = 0;
	enum load_calc_quirks quirks = LOAD_CALC_IGNORE_TURBO_LOAD |
		LOAD_CALC_IGNORE_THUMBNAIL_LOAD |
		LOAD_CALC_IGNORE_NON_REALTIME_LOAD;

	if (inst->state == MSM_VIDC_OPEN_DONE) {
		num_mbs_per_sec = msm_comm_get_load(inst->core,
					MSM_VIDC_DECODER, quirks);
		num_mbs_per_sec += msm_comm_get_load(inst->core,
					MSM_VIDC_ENCODER, quirks);
		if (num_mbs_per_sec > inst->core->resources.max_load) {
			dprintk(VIDC_ERR,
				"H/W is overloaded. needed: %d max: %d\n",
				num_mbs_per_sec,
				inst->core->resources.max_load);
			msm_vidc_print_running_insts(inst->core);
			return -EBUSY;
		}
	}
	return 0;
}

int msm_vidc_check_scaling_supported(struct msm_vidc_inst *inst)
{
	u32 x_min, x_max, y_min, y_max;
	u32 input_height, input_width, output_height, output_width;

	input_height = inst->prop.height[OUTPUT_PORT];
	input_width = inst->prop.width[OUTPUT_PORT];
	output_height = inst->prop.height[CAPTURE_PORT];
	output_width = inst->prop.width[CAPTURE_PORT];

	if (!input_height || !input_width || !output_height || !output_width) {
		dprintk(VIDC_ERR,
			"Invalid : Input height = %d width = %d"
			" output height = %d width = %d\n",
			input_height, input_width, output_height,
			output_width);
		return -ENOTSUPP;
	}

	if (!inst->capability.scale_x.min ||
		!inst->capability.scale_x.max ||
		!inst->capability.scale_y.min ||
		!inst->capability.scale_y.max) {

		if ((input_width * input_height) !=
			(output_width * output_height)) {
			dprintk(VIDC_ERR,
				"%s: scaling is not supported (%dx%d != %dx%d)\n",
				__func__, input_width, input_height,
				output_width, output_height);
			return -ENOTSUPP;
		} else {
			dprintk(VIDC_DBG, "%s: supported WxH = %dx%d\n",
				__func__, input_width, input_height);
			return 0;
		}
	}

	x_min = (1<<16)/inst->capability.scale_x.min;
	y_min = (1<<16)/inst->capability.scale_y.min;
	x_max = inst->capability.scale_x.max >> 16;
	y_max = inst->capability.scale_y.max >> 16;

	if (input_height > output_height) {
		if (input_height/output_height > x_min) {
			dprintk(VIDC_ERR,
				"Unsupported height downscale ratio %d vs %d\n",
				input_height/output_height, x_min);
			return -ENOTSUPP;
		}
	} else {
		if (input_height/output_height > x_max) {
			dprintk(VIDC_ERR,
				"Unsupported height upscale ratio %d vs %d\n",
				input_height/output_height, x_max);
			return -ENOTSUPP;
		}
	}
	if (input_width > output_width) {
		if (input_width/output_width > y_min) {
			dprintk(VIDC_ERR,
				"Unsupported width downscale ratio %d vs %d\n",
				input_width/output_width, y_min);
			return -ENOTSUPP;
		}
	} else {
		if (input_width/output_width > y_max) {
			dprintk(VIDC_ERR,
				"Unsupported width upscale ratio %d vs %d\n",
				input_width/output_width, y_max);
			return -ENOTSUPP;
		}
	}
	return 0;
}

int msm_vidc_check_session_supported(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core_capability *capability;
	int rc = 0;
	struct hfi_device *hdev;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_WARN, "%s: Invalid parameter\n", __func__);
		return -EINVAL;
	}
	capability = &inst->capability;
	hdev = inst->core->device;
	core = inst->core;
	rc = msm_vidc_load_supported(inst);
	if (rc) {
		change_inst_state(inst, MSM_VIDC_CORE_INVALID);
		msm_comm_kill_session(inst);
		dprintk(VIDC_WARN,
			"%s: Hardware is overloaded\n", __func__);
		return rc;
	}

	if (!is_thermal_permissible(core)) {
		dprintk(VIDC_WARN,
			"Thermal level critical, stop all active sessions!\n");
		return -ENOTSUPP;
	}

	if (!rc && inst->capability.capability_set) {
		if (inst->prop.width[CAPTURE_PORT] < capability->width.min ||
			inst->prop.height[CAPTURE_PORT] <
			capability->height.min) {
			dprintk(VIDC_ERR,
				"Unsupported WxH = (%u)x(%u), min supported is - (%u)x(%u)\n",
				inst->prop.width[CAPTURE_PORT],
				inst->prop.height[CAPTURE_PORT],
				capability->width.min,
				capability->height.min);
			rc = -ENOTSUPP;
		}
		if (!rc && (inst->prop.width[CAPTURE_PORT] >
			capability->width.max)) {
			dprintk(VIDC_ERR,
				"Unsupported width = %u supported max width = %u",
				inst->prop.width[CAPTURE_PORT],
				capability->width.max);
				rc = -ENOTSUPP;
		}

		if (!rc && (inst->prop.height[CAPTURE_PORT]
			* inst->prop.width[CAPTURE_PORT] >
			capability->width.max * capability->height.max)) {
			dprintk(VIDC_ERR,
			"Unsupported WxH = (%u)x(%u), max supported is - (%u)x(%u)\n",
			inst->prop.width[CAPTURE_PORT],
			inst->prop.height[CAPTURE_PORT],
			capability->width.max, capability->height.max);
			rc = -ENOTSUPP;
		}
	}
	if (rc) {
		change_inst_state(inst, MSM_VIDC_CORE_INVALID);
		msm_comm_kill_session(inst);
		dprintk(VIDC_ERR,
			"%s: Resolution unsupported\n", __func__);
	}
	return rc;
}

static void msm_comm_generate_session_error(struct msm_vidc_inst *inst)
{
	enum command_response cmd = SESSION_ERROR;
	struct msm_vidc_cb_cmd_done response = {0};

	dprintk(VIDC_WARN, "msm_comm_generate_session_error\n");
	if (!inst || !inst->core) {
		dprintk(VIDC_ERR, "%s: invalid input parameters\n", __func__);
		return;
	}

	response.session_id = inst;
	response.status = VIDC_ERR_FAIL;
	handle_session_error(cmd, (void *)&response);
}

static void msm_comm_generate_sys_error(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	enum command_response cmd = SYS_ERROR;
	struct msm_vidc_cb_cmd_done response  = {0};
	if (!inst || !inst->core) {
		dprintk(VIDC_ERR, "%s: invalid input parameters\n", __func__);
		return;
	}
	core = inst->core;
	response.device_id = (u32) core->id;
	handle_sys_error(cmd, (void *) &response);

}

int msm_comm_kill_session(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s: invalid input parameters\n", __func__);
		return -EINVAL;
	} else if (!inst->session) {
		/* There's no hfi session to kill */
		return 0;
	}

	/*
	 * We're internally forcibly killing the session, if fw is aware of
	 * the session send session_abort to firmware to clean up and release
	 * the session, else just kill the session inside the driver.
	 */
	if ((inst->state >= MSM_VIDC_OPEN_DONE &&
			inst->state < MSM_VIDC_CLOSE_DONE) ||
			inst->state == MSM_VIDC_CORE_INVALID) {
		rc = msm_comm_session_abort(inst);
		if (rc == -EBUSY) {
			msm_comm_generate_sys_error(inst);
			return 0;
		} else if (rc)
			return rc;

		change_inst_state(inst, MSM_VIDC_CLOSE_DONE);
	} else {
		dprintk(VIDC_WARN,
				"Inactive session %p, triggering an internal session error\n",
				inst);
		msm_comm_generate_session_error(inst);

	}

	return rc;
}

static inline int power_on_for_smem(struct msm_vidc_inst *inst)
{
	struct hfi_device *hdev = NULL;
	int rc = 0;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s: invalid inst handle\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;
	rc = call_hfi_op(hdev, power_enable, hdev->hfi_device_data);
	if (rc)
		dprintk(VIDC_ERR, "%s: failed to power on fw\n", __func__);
	return rc;
}

struct msm_smem *msm_comm_smem_alloc(struct msm_vidc_inst *inst,
			size_t size, u32 align, u32 flags,
			enum hal_buffer buffer_type, int map_kernel)
{
	struct msm_smem *m = NULL;

	if (!inst || !inst->core) {
		dprintk(VIDC_ERR, "%s: invalid inst: %p\n", __func__, inst);
		return NULL;
	}
	mutex_lock(&inst->core->lock);
	if (power_on_for_smem(inst))
		goto err_power_on;

	m = msm_smem_alloc(inst->mem_client, size, align,
				flags, buffer_type, map_kernel);
err_power_on:
	mutex_unlock(&inst->core->lock);
	return m;
}

void msm_comm_smem_free(struct msm_vidc_inst *inst, struct msm_smem *mem)
{
	if (!inst || !inst->core || !mem) {
		dprintk(VIDC_ERR,
			"%s: invalid params: %p %p\n", __func__, inst, mem);
		return;
	}

	mutex_lock(&inst->core->lock);
	if (inst->state != MSM_VIDC_CORE_INVALID) {
		if (power_on_for_smem(inst))
			goto err_power_on;
	}
	msm_smem_free(inst->mem_client, mem);
err_power_on:
	mutex_unlock(&inst->core->lock);
}

int msm_comm_smem_cache_operations(struct msm_vidc_inst *inst,
		struct msm_smem *mem, enum smem_cache_ops cache_ops)
{
	if (!inst || !mem) {
		dprintk(VIDC_ERR,
			"%s: invalid params: %p %p\n", __func__, inst, mem);
		return -EINVAL;
	}
	return msm_smem_cache_operations(inst->mem_client, mem, cache_ops);
}

struct msm_smem *msm_comm_smem_user_to_kernel(struct msm_vidc_inst *inst,
			int fd, u32 offset, enum hal_buffer buffer_type)
{
	struct msm_smem *m = NULL;

	if (!inst || !inst->core) {
		dprintk(VIDC_ERR, "%s: invalid inst: %p\n", __func__, inst);
		return NULL;
	}

	if (inst->state == MSM_VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR, "Core in Invalid state, returning from %s\n",
			__func__);
		return NULL;
	}

	mutex_lock(&inst->core->lock);
	if (power_on_for_smem(inst))
		goto err_power_on;

	m = msm_smem_user_to_kernel(inst->mem_client,
			fd, offset, buffer_type);
err_power_on:
	mutex_unlock(&inst->core->lock);
	return m;
}

int msm_comm_smem_get_domain_partition(struct msm_vidc_inst *inst,
			u32 flags, enum hal_buffer buffer_type,
			int *domain_num, int *partition_num)
{
	if (!inst || !domain_num || !partition_num) {
		dprintk(VIDC_ERR, "%s: invalid params: %p %p %p\n",
			__func__, inst, domain_num, partition_num);
		return -EINVAL;
	}
	return msm_smem_get_domain_partition(inst->mem_client, flags,
			buffer_type, domain_num, partition_num);
}

void msm_vidc_fw_unload_handler(struct work_struct *work)
{
	struct msm_vidc_core *core = NULL;
	struct hfi_device *hdev = NULL;
	int rc = 0;

	core = container_of(work, struct msm_vidc_core, fw_unload_work.work);
	if (!core || !core->device) {
		dprintk(VIDC_ERR, "%s - invalid work or core handle\n",
				__func__);
		return;
	}

	hdev = core->device;

	mutex_lock(&core->lock);
	if (list_empty(&core->instances) &&
		core->state != VIDC_CORE_UNINIT) {
		if (core->state > VIDC_CORE_INIT) {
			dprintk(VIDC_DBG, "Calling vidc_hal_core_release\n");
			rc = call_hfi_op(hdev, core_release,
					hdev->hfi_device_data);
			if (rc) {
				dprintk(VIDC_ERR,
					"Failed to release core, id = %d\n",
					core->id);
				mutex_unlock(&core->lock);
				return;
			}
		}

		core->state = VIDC_CORE_UNINIT;

		call_hfi_op(hdev, unload_fw, hdev->hfi_device_data);
		dprintk(VIDC_DBG, "Firmware unloaded\n");
		msm_comm_unvote_buses(core);
	}
	mutex_unlock(&core->lock);
}
