/* Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/qdsp6v2/apr.h>
#include <linux/ratelimit.h>
#include <soc/qcom/smd.h>
#include <soc/qcom/ocmem.h>
#include <sound/q6core.h>
#include <sound/audio_cal_utils.h>

#define TIMEOUT_MS 1000
/*
 * AVS bring up in the modem is optimitized for the new
 * Sub System Restart design and 100 milliseconds timeout
 * is sufficient to make sure the Q6 will be ready.
 */
#define Q6_READY_TIMEOUT_MS 100

enum {
	META_CAL,
	CUST_TOP_CAL,
	CORE_MAX_CAL
};

struct q6core_str {
	struct apr_svc *core_handle_q;
	wait_queue_head_t bus_bw_req_wait;
	wait_queue_head_t cmd_req_wait;
	u32 bus_bw_resp_received;
	enum cmd_flags {
		FLAG_NONE,
		FLAG_CMDRSP_LICENSE_RESULT
	} cmd_resp_received_flag;
	struct mutex cmd_lock;
	union {
		struct avcs_cmdrsp_get_license_validation_result
						cmdrsp_license_result;
	} cmd_resp_payload;
	struct avcs_cmd_rsp_get_low_power_segments_info_t lp_ocm_payload;
	u32 param;
	struct cal_type_data *cal_data[CORE_MAX_CAL];
	uint32_t mem_map_cal_handle;
	int32_t adsp_status;
};

static struct q6core_str q6core_lcl;

struct generic_get_data_ {
	int valid;
	int size_in_ints;
	int ints[];
};
static struct generic_get_data_ *generic_get_data;

static int32_t aprv2_core_fn_q(struct apr_client_data *data, void *priv)
{
	uint32_t *payload1;
	uint32_t nseg;
	int i, j;

	if (data == NULL) {
		pr_err("%s: data argument is null\n", __func__);
		return -EINVAL;
	}

	pr_debug("%s: core msg: payload len = %u, apr resp opcode = 0x%x\n",
		__func__,
		data->payload_size, data->opcode);

	switch (data->opcode) {

	case APR_BASIC_RSP_RESULT:{

		if (data->payload_size == 0) {
			pr_err("%s: APR_BASIC_RSP_RESULT No Payload ",
					__func__);
			return 0;
		}

		payload1 = data->payload;

		switch (payload1[0]) {

		case AVCS_CMD_GET_LOW_POWER_SEGMENTS_INFO:
			pr_info("%s: Cmd = AVCS_CMD_GET_LOW_POWER_SEGMENTS_INFO status[0x%x]\n",
				__func__, payload1[1]);
			break;
		case AVCS_CMD_SHARED_MEM_UNMAP_REGIONS:
			pr_debug("%s: Cmd = AVCS_CMD_SHARED_MEM_UNMAP_REGIONS status[0x%x]\n",
				__func__, payload1[1]);
			q6core_lcl.bus_bw_resp_received = 1;
			wake_up(&q6core_lcl.bus_bw_req_wait);
			break;
		case AVCS_CMD_SHARED_MEM_MAP_REGIONS:
			pr_debug("%s: Cmd = AVCS_CMD_SHARED_MEM_MAP_REGIONS status[0x%x]\n",
				__func__, payload1[1]);
			q6core_lcl.bus_bw_resp_received = 1;
			wake_up(&q6core_lcl.bus_bw_req_wait);
			break;
		case AVCS_CMD_REGISTER_TOPOLOGIES:
			pr_debug("%s: Cmd = AVCS_CMD_REGISTER_TOPOLOGIES status[0x%x]\n",
				__func__, payload1[1]);
			/* -ADSP status to match Linux error standard */
			q6core_lcl.adsp_status = -payload1[1];
			q6core_lcl.bus_bw_resp_received = 1;
			wake_up(&q6core_lcl.bus_bw_req_wait);
			break;
		case AVCS_CMD_DEREGISTER_TOPOLOGIES:
			pr_debug("%s: Cmd = AVCS_CMD_DEREGISTER_TOPOLOGIES status[0x%x]\n",
				__func__, payload1[1]);
			q6core_lcl.bus_bw_resp_received = 1;
			wake_up(&q6core_lcl.bus_bw_req_wait);
			break;
		default:
			pr_err("%s: Invalid cmd rsp[0x%x][0x%x] opcode %d\n",
					__func__,
					payload1[0], payload1[1], data->opcode);
			break;
		}
		break;
	}

	case AVCS_CMDRSP_GET_LOW_POWER_SEGMENTS_INFO:
		payload1 = data->payload;
		pr_info("%s: cmd = AVCS_CMDRSP_GET_LOW_POWER_SEGMENTS_INFO num_segments = 0x%x\n",
					__func__, payload1[0]);
		nseg = payload1[0];
		q6core_lcl.lp_ocm_payload.num_segments = nseg;
		q6core_lcl.lp_ocm_payload.bandwidth = payload1[1];
		for (i = 0, j = 2; i < nseg; i++) {
			q6core_lcl.lp_ocm_payload.mem_segment[i].type =
					(payload1[j] & 0xffff);
			q6core_lcl.lp_ocm_payload.mem_segment[i].category =
					((payload1[j++] >> 16) & 0xffff);
			q6core_lcl.lp_ocm_payload.mem_segment[i].size =
					payload1[j++];
			q6core_lcl.lp_ocm_payload.
				mem_segment[i].start_address_lsw =
				payload1[j++];
			q6core_lcl.lp_ocm_payload.
				mem_segment[i].start_address_msw =
				payload1[j++];
		}

		q6core_lcl.bus_bw_resp_received = 1;
		wake_up(&q6core_lcl.bus_bw_req_wait);
		break;

	case RESET_EVENTS:{
		pr_debug("%s: Reset event received in Core service\n",
			__func__);
		apr_reset(q6core_lcl.core_handle_q);
		q6core_lcl.core_handle_q = NULL;
		break;
	}
	case AVCS_CMDRSP_SHARED_MEM_MAP_REGIONS:
		payload1 = data->payload;
		pr_debug("%s: AVCS_CMDRSP_SHARED_MEM_MAP_REGIONS handle %d\n",
			__func__, payload1[0]);
		q6core_lcl.mem_map_cal_handle = payload1[0];
		q6core_lcl.bus_bw_resp_received = 1;
		wake_up(&q6core_lcl.bus_bw_req_wait);
		break;
	case AVCS_CMDRSP_ADSP_EVENT_GET_STATE:
		payload1 = data->payload;
		q6core_lcl.param = payload1[0];
		pr_debug("%s: Received ADSP get state response 0x%x\n",
			 __func__, q6core_lcl.param);
		/* ensure .param is updated prior to .bus_bw_resp_received */
		wmb();
		q6core_lcl.bus_bw_resp_received = 1;
		wake_up(&q6core_lcl.bus_bw_req_wait);
		break;
	case AVCS_CMDRSP_GET_LICENSE_VALIDATION_RESULT:
		payload1 = data->payload;
		pr_debug("%s: cmd = LICENSE_VALIDATION_RESULT, result = 0x%x\n",
				__func__, payload1[0]);
		q6core_lcl.cmd_resp_payload.cmdrsp_license_result.result
								= payload1[0];
		q6core_lcl.cmd_resp_received_flag = FLAG_CMDRSP_LICENSE_RESULT;
		wake_up(&q6core_lcl.cmd_req_wait);
		break;
	default:
		pr_err("%s: Message id from adsp core svc: 0x%x\n",
			__func__, data->opcode);
		if (generic_get_data) {
			generic_get_data->valid = 1;
			generic_get_data->size_in_ints =
				data->payload_size/sizeof(int);
			pr_debug("DTS_EAGLE_CORE callback size = %i\n",
				 data->payload_size);
			memcpy(generic_get_data->ints, data->payload,
				data->payload_size);
			q6core_lcl.bus_bw_resp_received = 1;
			wake_up(&q6core_lcl.bus_bw_req_wait);
			break;
		}
		break;
	}

	return 0;
}

void ocm_core_open(void)
{
	static DEFINE_RATELIMIT_STATE(rl, HZ/2, 1);

	if (q6core_lcl.core_handle_q == NULL)
		q6core_lcl.core_handle_q = apr_register("ADSP", "CORE",
					aprv2_core_fn_q, 0xFFFFFFFF, NULL);
	pr_debug("%s: Open_q %pK\n", __func__, q6core_lcl.core_handle_q);
	if (q6core_lcl.core_handle_q == NULL) {
			if (__ratelimit(&rl))
				pr_err("%s: Unable to register CORE\n",
					__func__);
	}
}

int32_t core_set_license(uint32_t key, uint32_t module_id)
{
	struct avcs_cmd_set_license *cmd_setl = NULL;
	struct audio_cal_info_metainfo *metainfo = NULL;
	struct cal_block_data *cal_block = NULL;
	int rc = 0, packet_size = 0;

	pr_debug("%s: key:0x%x, id:0x%x\n", __func__, key, module_id);

	mutex_lock(&(q6core_lcl.cmd_lock));
	if (q6core_lcl.cal_data[META_CAL] == NULL) {
		pr_err("%s: cal_data not initialized yet!!\n", __func__);
		rc = -EINVAL;
		goto cmd_unlock;
	}

	mutex_lock(&((q6core_lcl.cal_data[META_CAL])->lock));
	cal_block =
		cal_utils_get_only_cal_block(q6core_lcl.cal_data[META_CAL]);
	if (cal_block == NULL ||
		cal_block->cal_data.kvaddr == NULL ||
		cal_block->cal_data.size <= 0) {
		pr_err("%s: Invalid cal block to send", __func__);
		rc = -EINVAL;
		goto cal_data_unlock;
	}
	metainfo = (struct audio_cal_info_metainfo *)cal_block->cal_info;
	if (metainfo == NULL) {
		pr_err("%s: No metainfo!!!", __func__);
		rc = -EINVAL;
		goto cal_data_unlock;
	}
	if (metainfo->nKey != key) {
		pr_err("%s: metainfo key mismatch!!! found:%x, needed:%x\n",
				__func__, metainfo->nKey, key);
		rc = -EINVAL;
		goto cal_data_unlock;
	} else if (key == 0) {
		pr_err("%s: metainfo key is %d a invalid key", __func__, key);
		goto cal_data_unlock;
	}

	packet_size = sizeof(struct avcs_cmd_set_license) +
					cal_block->cal_data.size;
	/*round up total packet_size to next 4 byte boundary*/
	packet_size = ((packet_size + 0x3)>>2)<<2;

	cmd_setl = kzalloc(packet_size, GFP_KERNEL);
	if (cmd_setl == NULL) {
		pr_err("%s: kzalloc for cmd_set_license failed for size %d\n",
							__func__, packet_size);
		rc  = -ENOMEM;
		goto cal_data_unlock;
	}

	ocm_core_open();
	if (q6core_lcl.core_handle_q == NULL) {
		pr_err("%s: apr registration for CORE failed\n", __func__);
		rc  = -ENODEV;
		goto fail_cmd;
	}

	cmd_setl->hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	cmd_setl->hdr.pkt_size = packet_size;
	cmd_setl->hdr.src_port = 0;
	cmd_setl->hdr.dest_port = 0;
	cmd_setl->hdr.token = 0;
	cmd_setl->hdr.opcode = AVCS_CMD_SET_LICENSE;
	cmd_setl->id = module_id;
	cmd_setl->overwrite = 1;
	cmd_setl->size = cal_block->cal_data.size;
	memcpy((uint8_t *)cmd_setl + sizeof(struct avcs_cmd_set_license),
		cal_block->cal_data.kvaddr,
		cal_block->cal_data.size);
	pr_info("%s: Set license opcode=0x%x ,key=0x%x, id =0x%x, size = %d\n",
			__func__, cmd_setl->hdr.opcode,
			metainfo->nKey, cmd_setl->id, cmd_setl->size);
	rc = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *)cmd_setl);
	if (rc < 0)
		pr_err("%s: SET_LICENSE failed op[0x%x]rc[%d]\n",
					__func__, cmd_setl->hdr.opcode, rc);

fail_cmd:
	kfree(cmd_setl);
cal_data_unlock:
	mutex_unlock(&((q6core_lcl.cal_data[META_CAL])->lock));
cmd_unlock:
	mutex_unlock(&(q6core_lcl.cmd_lock));

	return rc;
}

int32_t core_get_license_status(uint32_t module_id)
{
	struct avcs_cmd_get_license_validation_result get_lvr_cmd;
	int ret = 0;

	pr_debug("%s: module_id 0x%x", __func__, module_id);

	mutex_lock(&(q6core_lcl.cmd_lock));
	ocm_core_open();
	if (q6core_lcl.core_handle_q == NULL) {
		pr_err("%s: apr registration for CORE failed\n", __func__);
		ret  = -ENODEV;
		goto fail_cmd;
	}

	get_lvr_cmd.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	get_lvr_cmd.hdr.pkt_size =
		sizeof(struct avcs_cmd_get_license_validation_result);

	get_lvr_cmd.hdr.src_port = 0;
	get_lvr_cmd.hdr.dest_port = 0;
	get_lvr_cmd.hdr.token = 0;
	get_lvr_cmd.hdr.opcode = AVCS_CMD_GET_LICENSE_VALIDATION_RESULT;
	get_lvr_cmd.id = module_id;


	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *) &get_lvr_cmd);
	if (ret < 0) {
		pr_err("%s: license_validation request failed, err %d\n",
							__func__, ret);
		ret = -EREMOTE;
		goto fail_cmd;
	}

	q6core_lcl.cmd_resp_received_flag &= ~(FLAG_CMDRSP_LICENSE_RESULT);
	mutex_unlock(&(q6core_lcl.cmd_lock));
	ret = wait_event_timeout(q6core_lcl.cmd_req_wait,
			(q6core_lcl.cmd_resp_received_flag ==
				FLAG_CMDRSP_LICENSE_RESULT),
				msecs_to_jiffies(TIMEOUT_MS));
	mutex_lock(&(q6core_lcl.cmd_lock));
	if (!ret) {
		pr_err("%s: wait_event timeout for CMDRSP_LICENSE_RESULT\n",
				__func__);
		ret = -ETIME;
		goto fail_cmd;
	}
	q6core_lcl.cmd_resp_received_flag &= ~(FLAG_CMDRSP_LICENSE_RESULT);
	ret = q6core_lcl.cmd_resp_payload.cmdrsp_license_result.result;

fail_cmd:
	mutex_unlock(&(q6core_lcl.cmd_lock));
	pr_info("%s: cmdrsp_license_result.result = 0x%x for module 0x%x\n",
				__func__, ret, module_id);
	return ret;
}

int core_dts_eagle_set(int size, char *data)
{
	struct adsp_dts_eagle *payload = NULL;
	int rc = 0, size_aligned4byte;

	pr_debug("DTS_EAGLE_CORE - %s\n", __func__);
	if (size <= 0 || !data) {
		pr_err("DTS_EAGLE_CORE - %s: invalid size %i or pointer %pK.\n",
			__func__, size, data);
		return -EINVAL;
	}

	size_aligned4byte = (size+3) & 0xFFFFFFFC;
	ocm_core_open();
	if (q6core_lcl.core_handle_q) {
		payload = kzalloc(sizeof(struct adsp_dts_eagle) +
				  size_aligned4byte, GFP_KERNEL);
		if (!payload) {
			pr_err("DTS_EAGLE_CORE - %s: out of memory (aligned size %i).\n",
				__func__, size_aligned4byte);
			return -ENOMEM;
		}
		payload->hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
						APR_HDR_LEN(APR_HDR_SIZE),
						APR_PKT_VER);
		payload->hdr.pkt_size = sizeof(struct adsp_dts_eagle) +
					       size_aligned4byte;
		payload->hdr.src_port = 0;
		payload->hdr.dest_port = 0;
		payload->hdr.token = 0;
		payload->hdr.opcode = ADSP_CMD_SET_DTS_EAGLE_DATA_ID;
		payload->id = DTS_EAGLE_LICENSE_ID;
		payload->overwrite = 1;
		payload->size = size;
		memcpy(payload->data, data, size);
		rc = apr_send_pkt(q6core_lcl.core_handle_q,
				(uint32_t *)payload);
		if (rc < 0) {
			pr_err("DTS_EAGLE_CORE - %s: failed op[0x%x]rc[%d]\n",
				__func__, payload->hdr.opcode, rc);
		}
		kfree(payload);
	}
	return rc;
}

int core_dts_eagle_get(int id, int size, char *data)
{
	struct apr_hdr ah;
	int rc = 0;

	pr_debug("DTS_EAGLE_CORE - %s\n", __func__);
	if (size <= 0 || !data) {
		pr_err("DTS_EAGLE_CORE - %s: invalid size %i or pointer %pK.\n",
			__func__, size, data);
		return -EINVAL;
	}
	ocm_core_open();
	if (q6core_lcl.core_handle_q) {
		ah.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		ah.pkt_size = sizeof(struct apr_hdr);
		ah.src_port = 0;
		ah.dest_port = 0;
		ah.token = 0;
		ah.opcode = id;

		q6core_lcl.bus_bw_resp_received = 0;
		generic_get_data = kzalloc(sizeof(struct generic_get_data_)
					   + size, GFP_KERNEL);
		if (!generic_get_data) {
			pr_err("DTS_EAGLE_CORE - %s: error allocating memory of size %i\n",
				__func__, size);
			return -ENOMEM;
		}

		rc = apr_send_pkt(q6core_lcl.core_handle_q,
				(uint32_t *)&ah);
		if (rc < 0) {
			pr_err("DTS_EAGLE_CORE - %s: failed op[0x%x]rc[%d]\n",
				__func__, ah.opcode, rc);
			goto fail_cmd_2;
		}

		rc = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
		if (!rc) {
			pr_err("DTS_EAGLE_CORE - %s: EAGLE get params timed out\n",
				__func__);
			rc = -EINVAL;
			goto fail_cmd_2;
		}
		if (generic_get_data->valid) {
			rc = 0;
			memcpy(data, generic_get_data->ints, size);
		} else {
			rc = -EINVAL;
			pr_err("DTS_EAGLE_CORE - %s: EAGLE get params problem getting data - check callback error value\n",
				__func__);
		}
	}

fail_cmd_2:
	kfree(generic_get_data);
	generic_get_data = NULL;
	return rc;
}

uint32_t core_set_dolby_manufacturer_id(int manufacturer_id)
{
	struct adsp_dolby_manufacturer_id payload;
	int rc = 0;
	static DEFINE_RATELIMIT_STATE(rl, HZ/2, 1);

	pr_debug("%s: manufacturer_id :%d\n", __func__, manufacturer_id);
	mutex_lock(&(q6core_lcl.cmd_lock));
	ocm_core_open();
	if (q6core_lcl.core_handle_q) {
		payload.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_EVENT,
			APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
		payload.hdr.pkt_size =
			sizeof(struct adsp_dolby_manufacturer_id);
		payload.hdr.src_port = 0;
		payload.hdr.dest_port = 0;
		payload.hdr.token = 0;
		payload.hdr.opcode = ADSP_CMD_SET_DOLBY_MANUFACTURER_ID;
		payload.manufacturer_id = manufacturer_id;
		pr_debug("%s: Send Dolby security opcode=0x%x manufacturer ID = %d\n",
			__func__,
			payload.hdr.opcode, payload.manufacturer_id);
		rc = apr_send_pkt(q6core_lcl.core_handle_q,
						(uint32_t *)&payload);
		if (rc < 0) {
			if (__ratelimit(&rl))
				pr_err("%s: SET_DOLBY_MANUFACTURER_ID failed op[0x%x]rc[%d]\n",
					__func__, payload.hdr.opcode, rc);
		}
	}
	mutex_unlock(&(q6core_lcl.cmd_lock));
	return rc;
}

int core_get_low_power_segments(
		struct avcs_cmd_rsp_get_low_power_segments_info_t **lp_memseg)
{
	struct avcs_cmd_get_low_power_segments_info lp_ocm_cmd;
	int ret = 0;

	pr_debug("%s:", __func__);

	ocm_core_open();
	if (q6core_lcl.core_handle_q == NULL) {
		pr_info("%s: apr registration for CORE failed\n", __func__);
		return -ENODEV;
	}


	lp_ocm_cmd.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	lp_ocm_cmd.hdr.pkt_size =
		sizeof(struct avcs_cmd_get_low_power_segments_info);

	lp_ocm_cmd.hdr.src_port = 0;
	lp_ocm_cmd.hdr.dest_port = 0;
	lp_ocm_cmd.hdr.token = 0;
	lp_ocm_cmd.hdr.opcode = AVCS_CMD_GET_LOW_POWER_SEGMENTS_INFO;


	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *) &lp_ocm_cmd);
	if (ret < 0) {
		pr_err("%s: CORE low power segment request failed %d\n",
			__func__, ret);
		goto fail_cmd;
	}

	ret = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: wait_event timeout for GET_LOW_POWER_SEGMENTS\n",
				__func__);
		ret = -ETIME;
		goto fail_cmd;
	}

	*lp_memseg = &q6core_lcl.lp_ocm_payload;
	return 0;

fail_cmd:
	return ret;
}

bool q6core_is_adsp_ready(void)
{
	int rc;
	bool ret = false;
	struct apr_hdr hdr;

	pr_debug("%s: enter\n", __func__);
	memset(&hdr, 0, sizeof(hdr));
	hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
				      APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	hdr.pkt_size = APR_PKT_SIZE(APR_HDR_SIZE, 0);
	hdr.opcode = AVCS_CMD_ADSP_EVENT_GET_STATE;

	ocm_core_open();
	q6core_lcl.bus_bw_resp_received = 0;
	rc = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *)&hdr);
	if (rc < 0) {
		pr_err("%s: Get ADSP state APR packet send event %d\n",
			__func__, rc);
		goto bail;
	}

	rc = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(Q6_READY_TIMEOUT_MS));
	if (rc > 0 && q6core_lcl.bus_bw_resp_received) {
		/* ensure to read updated param by callback thread */
		rmb();
		ret = !!q6core_lcl.param;
	}
bail:
	pr_debug("%s: leave, rc %d, adsp ready %d\n", __func__, rc, ret);
	return ret;
}


static int q6core_map_memory_regions(phys_addr_t *buf_add, uint32_t mempool_id,
			uint32_t *bufsz, uint32_t bufcnt, uint32_t *map_handle)
{
	struct avs_cmd_shared_mem_map_regions *mmap_regions = NULL;
	struct avs_shared_map_region_payload *mregions = NULL;
	void *mmap_region_cmd = NULL;
	void *payload = NULL;
	int ret = 0;
	int i = 0;
	int cmd_size = 0;

	cmd_size = sizeof(struct avs_cmd_shared_mem_map_regions)
			+ sizeof(struct avs_shared_map_region_payload)
			* bufcnt;

	mmap_region_cmd = kzalloc(cmd_size, GFP_KERNEL);
	if (mmap_region_cmd == NULL)
		return -ENOMEM;

	mmap_regions = (struct avs_cmd_shared_mem_map_regions *)mmap_region_cmd;
	mmap_regions->hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
						APR_HDR_LEN(APR_HDR_SIZE),
								APR_PKT_VER);
	mmap_regions->hdr.pkt_size = cmd_size;
	mmap_regions->hdr.src_port = 0;
	mmap_regions->hdr.dest_port = 0;
	mmap_regions->hdr.token = 0;
	mmap_regions->hdr.opcode = AVCS_CMD_SHARED_MEM_MAP_REGIONS;
	mmap_regions->mem_pool_id = ADSP_MEMORY_MAP_SHMEM8_4K_POOL & 0x00ff;
	mmap_regions->num_regions = bufcnt & 0x00ff;
	mmap_regions->property_flag = 0x00;

	payload = ((u8 *) mmap_region_cmd +
				sizeof(struct avs_cmd_shared_mem_map_regions));
	mregions = (struct avs_shared_map_region_payload *)payload;

	for (i = 0; i < bufcnt; i++) {
		mregions->shm_addr_lsw = lower_32_bits(buf_add[i]);
		mregions->shm_addr_msw = upper_32_bits(buf_add[i]);
		mregions->mem_size_bytes = bufsz[i];
		++mregions;
	}

	pr_debug("%s: sending memory map, addr %pa, size %d, bufcnt = %d\n",
		__func__, buf_add, bufsz[0], mmap_regions->num_regions);

	*map_handle = 0;
	q6core_lcl.bus_bw_resp_received = 0;
	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *)
		mmap_regions);
	if (ret < 0) {
		pr_err("%s: mmap regions failed %d\n",
			__func__, ret);
		ret = -EINVAL;
		goto done;
	}

	ret = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: timeout. waited for memory map\n", __func__);
		ret = -ETIME;
		goto done;
	}

	*map_handle = q6core_lcl.mem_map_cal_handle;
done:
	kfree(mmap_region_cmd);
	return ret;
}

static int q6core_memory_unmap_regions(uint32_t mem_map_handle)
{
	struct avs_cmd_shared_mem_unmap_regions unmap_regions;
	int ret = 0;

	memset(&unmap_regions, 0, sizeof(unmap_regions));
	unmap_regions.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
		APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	unmap_regions.hdr.pkt_size = APR_PKT_SIZE(APR_HDR_SIZE,
		sizeof(unmap_regions));
	unmap_regions.hdr.src_svc = APR_SVC_ADSP_CORE;
	unmap_regions.hdr.src_domain = APR_DOMAIN_APPS;
	unmap_regions.hdr.src_port = 0;
	unmap_regions.hdr.dest_svc = APR_SVC_ADSP_CORE;
	unmap_regions.hdr.dest_domain = APR_DOMAIN_ADSP;
	unmap_regions.hdr.dest_port = 0;
	unmap_regions.hdr.token = 0;
	unmap_regions.hdr.opcode = AVCS_CMD_SHARED_MEM_UNMAP_REGIONS;
	unmap_regions.mem_map_handle = mem_map_handle;

	q6core_lcl.bus_bw_resp_received = 0;

	pr_debug("%s: unmap regions map handle %d\n",
		__func__, mem_map_handle);

	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *)
		&unmap_regions);
	if (ret < 0) {
		pr_err("%s: unmap regions failed %d\n",
			__func__, ret);
		ret = -EINVAL;
		goto done;
	}

	ret = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: timeout. waited for memory_unmap\n",
		       __func__);
		ret = -ETIME;
		goto done;
	}
done:
	return ret;
}

static int q6core_dereg_all_custom_topologies(void)
{
	int ret = 0;
	struct avcs_cmd_deregister_topologies dereg_top;

	memset(&dereg_top, 0, sizeof(dereg_top));
	dereg_top.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
		APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	dereg_top.hdr.pkt_size = APR_PKT_SIZE(APR_HDR_SIZE, sizeof(dereg_top));
	dereg_top.hdr.src_svc = APR_SVC_ADSP_CORE;
	dereg_top.hdr.src_domain = APR_DOMAIN_APPS;
	dereg_top.hdr.src_port = 0;
	dereg_top.hdr.dest_svc = APR_SVC_ADSP_CORE;
	dereg_top.hdr.dest_domain = APR_DOMAIN_ADSP;
	dereg_top.hdr.dest_port = 0;
	dereg_top.hdr.token = 0;
	dereg_top.hdr.opcode = AVCS_CMD_DEREGISTER_TOPOLOGIES;
	dereg_top.payload_addr_lsw = 0;
	dereg_top.payload_addr_msw = 0;
	dereg_top.mem_map_handle = 0;
	dereg_top.payload_size = 0;
	dereg_top.mode = AVCS_MODE_DEREGISTER_ALL_CUSTOM_TOPOLOGIES;

	q6core_lcl.bus_bw_resp_received = 0;

	pr_debug("%s: Deregister topologies mode %d\n",
		__func__, dereg_top.mode);

	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *) &dereg_top);
	if (ret < 0) {
		pr_err("%s: Deregister topologies failed %d\n",
			__func__, ret);
		goto done;
	}

	ret = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: wait_event timeout for Deregister topologies\n",
			__func__);
		goto done;
	}
done:
	return ret;
}

static int q6core_send_custom_topologies(void)
{
	int ret = 0;
	int ret2 = 0;
	struct cal_block_data *cal_block = NULL;
	struct avcs_cmd_register_topologies reg_top;

	memset(&reg_top, 0, sizeof(reg_top));
	mutex_lock(&q6core_lcl.cal_data[CUST_TOP_CAL]->lock);
	mutex_lock(&q6core_lcl.cmd_lock);

	cal_block = cal_utils_get_only_cal_block(
		q6core_lcl.cal_data[CUST_TOP_CAL]);
	if (cal_block == NULL) {
		pr_debug("%s: cal block is NULL!\n", __func__);
		goto unlock;
	}
	if (cal_block->cal_data.size <= 0) {
		pr_debug("%s: cal size is %zd not sending\n",
			__func__, cal_block->cal_data.size);
		goto unlock;
	}

	if (!q6core_is_adsp_ready()) {
		pr_err("%s: ADSP is not ready!\n", __func__);
		ret  = -ENODEV;
		goto unlock;
	}

	q6core_dereg_all_custom_topologies();

	ret = q6core_map_memory_regions(&cal_block->cal_data.paddr, 0,
		(uint32_t *)&cal_block->map_data.map_size, 1,
		&cal_block->map_data.q6map_handle);
	if (!ret)  {
		pr_err("%s: q6core_map_memory_regions failed\n", __func__);
		goto unlock;
	}

	reg_top.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
		APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
	reg_top.hdr.pkt_size = APR_PKT_SIZE(APR_HDR_SIZE, sizeof(reg_top));
	reg_top.hdr.src_svc = APR_SVC_ADSP_CORE;
	reg_top.hdr.src_domain = APR_DOMAIN_APPS;
	reg_top.hdr.src_port = 0;
	reg_top.hdr.dest_svc = APR_SVC_ADSP_CORE;
	reg_top.hdr.dest_domain = APR_DOMAIN_ADSP;
	reg_top.hdr.dest_port = 0;
	reg_top.hdr.token = 0;
	reg_top.hdr.opcode = AVCS_CMD_REGISTER_TOPOLOGIES;
	reg_top.payload_addr_lsw =
		lower_32_bits(cal_block->cal_data.paddr);
	reg_top.payload_addr_msw =
		upper_32_bits(cal_block->cal_data.paddr);
	reg_top.mem_map_handle = cal_block->map_data.q6map_handle;
	reg_top.payload_size = cal_block->cal_data.size;

	q6core_lcl.adsp_status = 0;
	q6core_lcl.bus_bw_resp_received = 0;

	pr_debug("%s: Register topologies addr %pa, size %zd, map handle %d\n",
		__func__, &cal_block->cal_data.paddr, cal_block->cal_data.size,
		cal_block->map_data.q6map_handle);

	ret = apr_send_pkt(q6core_lcl.core_handle_q, (uint32_t *) &reg_top);
	if (ret < 0) {
		pr_err("%s: Register topologies failed %d\n",
			__func__, ret);
		goto unmap;
	}

	ret = wait_event_timeout(q6core_lcl.bus_bw_req_wait,
				(q6core_lcl.bus_bw_resp_received == 1),
				msecs_to_jiffies(TIMEOUT_MS));
	if (!ret) {
		pr_err("%s: wait_event timeout for Register topologies\n",
			__func__);
		goto unmap;
	}

	if (q6core_lcl.adsp_status < 0)
		ret = q6core_lcl.adsp_status;
unmap:
	ret2 = q6core_memory_unmap_regions(cal_block->map_data.q6map_handle);
	if (!ret2)  {
		pr_err("%s: q6core_memory_unmap_regions failed for map handle %d\n",
			__func__, cal_block->map_data.q6map_handle);
		ret = ret2;
		goto unlock;
	}

unlock:
	mutex_unlock(&q6core_lcl.cmd_lock);
	mutex_unlock(&q6core_lcl.cal_data[CUST_TOP_CAL]->lock);

	return ret;
}

static int get_cal_type_index(int32_t cal_type)
{
	int ret = -EINVAL;

	switch (cal_type) {
	case AUDIO_CORE_METAINFO_CAL_TYPE:
		ret = META_CAL;
		break;
	case CORE_CUSTOM_TOPOLOGIES_CAL_TYPE:
		ret = CUST_TOP_CAL;
		break;
	default:
		pr_err("%s: invalid cal type %d!\n", __func__, cal_type);
	}
	return ret;
}

static int q6core_alloc_cal(int32_t cal_type,
			    size_t data_size, void *data)
{
	int ret = 0;
	int cal_index;

	cal_index = get_cal_type_index(cal_type);
	if (cal_index < 0) {
		pr_err("%s: could not get cal index %d!\n",
			__func__, cal_index);
		ret = -EINVAL;
		goto done;
	}


	ret = cal_utils_alloc_cal(data_size, data,
		q6core_lcl.cal_data[cal_index], 0, NULL);
	if (ret < 0) {
		pr_err("%s: cal_utils_alloc_block failed, ret = %d, cal type = %d!\n",
			__func__, ret, cal_type);
		goto done;
	}
done:
	return ret;
}

static int q6core_dealloc_cal(int32_t cal_type,
			      size_t data_size, void *data)
{
	int ret = 0;
	int cal_index;

	cal_index = get_cal_type_index(cal_type);
	if (cal_index < 0) {
		pr_err("%s: could not get cal index %d!\n",
			__func__, cal_index);
		ret = -EINVAL;
		goto done;
	}


	ret = cal_utils_dealloc_cal(data_size, data,
					q6core_lcl.cal_data[cal_index]);
	if (ret < 0) {
		pr_err("%s: cal_utils_dealloc_block failed, ret = %d, cal type = %d!\n",
			__func__, ret, cal_type);
		goto done;
	}
done:
	return ret;
}

static int q6core_set_cal(int32_t cal_type,
	size_t data_size, void *data)
{
	int ret = 0;
	int cal_index;

	cal_index = get_cal_type_index(cal_type);
	if (cal_index < 0) {
		pr_err("%s: could not get cal index %d!\n",
			__func__, cal_index);
		ret = -EINVAL;
		goto done;
	}


	ret = cal_utils_set_cal(data_size, data,
				    q6core_lcl.cal_data[cal_index], 0, NULL);
	if (ret < 0) {
		pr_err("%s: cal_utils_set_cal failed, ret = %d, cal type = %d!\n",
		__func__, ret, cal_type);
		goto done;
	}

	if (cal_index == CUST_TOP_CAL)
		ret = q6core_send_custom_topologies();
done:
	return ret;
}

static void q6core_delete_cal_data(void)
{
	pr_debug("%s:\n", __func__);

	cal_utils_destroy_cal_types(CORE_MAX_CAL, q6core_lcl.cal_data);
	return;
}


static int q6core_init_cal_data(void)
{
	int ret = 0;
	struct cal_type_info    cal_type_info[] = {
		{{AUDIO_CORE_METAINFO_CAL_TYPE,
		{q6core_alloc_cal, q6core_dealloc_cal, NULL,
		q6core_set_cal, NULL, NULL} },
		{NULL, NULL, cal_utils_match_buf_num} },

		{{CORE_CUSTOM_TOPOLOGIES_CAL_TYPE,
		{q6core_alloc_cal, q6core_dealloc_cal, NULL,
		q6core_set_cal, NULL, NULL} },
		{NULL, NULL, cal_utils_match_buf_num} }
	};
	pr_debug("%s:\n", __func__);

	ret = cal_utils_create_cal_types(CORE_MAX_CAL,
		q6core_lcl.cal_data, cal_type_info);
	if (ret < 0) {
		pr_err("%s: could not create cal type!\n",
			__func__);
		goto err;
	}

	return ret;
err:
	q6core_delete_cal_data();
	return ret;
}

static int __init core_init(void)
{
	init_waitqueue_head(&q6core_lcl.bus_bw_req_wait);
	q6core_lcl.bus_bw_resp_received = 0;

	q6core_lcl.core_handle_q = NULL;

	init_waitqueue_head(&q6core_lcl.cmd_req_wait);
	q6core_lcl.cmd_resp_received_flag = FLAG_NONE;
	mutex_init(&q6core_lcl.cmd_lock);
	q6core_lcl.mem_map_cal_handle = 0;
	q6core_lcl.adsp_status = 0;

	q6core_init_cal_data();
	return 0;
}
module_init(core_init);

static void __exit core_exit(void)
{
	mutex_destroy(&q6core_lcl.cmd_lock);
	q6core_delete_cal_data();
}
module_exit(core_exit);
MODULE_DESCRIPTION("ADSP core driver");
MODULE_LICENSE("GPL v2");

