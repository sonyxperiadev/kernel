/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "osdep.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wlan_fwol_public_structs.h"
#include "wmi_unified_fwol_api.h"

#ifdef WLAN_FEATURE_ELNA
/**
 * send_set_elna_bypass_cmd_tlv() - send set elna bypass cmd to fw
 * @wmi_handle: wmi handle
 * @req: set eLNA bypass request
 *
 * Send WMI_SET_ELNA_BYPASS_CMDID to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_set_elna_bypass_cmd_tlv(wmi_unified_t wmi_handle,
			     struct set_elna_bypass_request *req)
{
	wmi_buf_t buf;
	wmi_set_elna_bypass_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_set_elna_bypass_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_elna_bypass_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_set_elna_bypass_cmd_fixed_param));
	cmd->vdev_id = req->vdev_id;
	cmd->en_dis = req->en_dis;
	wmi_mtrace(WMI_SET_ELNA_BYPASS_CMDID, req->vdev_id, req->en_dis);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_SET_ELNA_BYPASS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_get_elna_bypass_cmd_tlv() - send get elna bypass cmd to fw
 * @wmi_handle: wmi handle
 * @req: get eLNA bypass request
 *
 * Send WMI_GET_ELNA_BYPASS_CMDID to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_get_elna_bypass_cmd_tlv(wmi_unified_t wmi_handle,
			     struct get_elna_bypass_request *req)
{
	wmi_buf_t buf;
	wmi_get_elna_bypass_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_get_elna_bypass_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_get_elna_bypass_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_get_elna_bypass_cmd_fixed_param));
	cmd->vdev_id = req->vdev_id;
	wmi_mtrace(WMI_GET_ELNA_BYPASS_CMDID, req->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_GET_ELNA_BYPASS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * extract_get_elna_bypass_resp_tlv() - Extract WMI get eLNA bypass response
 * @wmi_handle: wmi handle
 * @resp_buf: response buffer
 * @resp: get eLNA bypass response
 *
 * Extract WMI get eLNA bypass response from firmware.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
extract_get_elna_bypass_resp_tlv(struct wmi_unified *wmi_handle, void *resp_buf,
				 struct get_elna_bypass_response *resp)
{
	WMI_GET_ELNA_BYPASS_EVENTID_param_tlvs *param_buf;
	wmi_get_elna_bypass_event_fixed_param *evt;

	param_buf = resp_buf;
	evt = param_buf->fixed_param;
	if (!evt) {
		wmi_err("Invalid get elna bypass event");
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("Get elna bypass %d from vdev %d", evt->en_dis, evt->vdev_id);

	resp->vdev_id = evt->vdev_id;
	resp->en_dis = evt->en_dis;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_ELNA */

#ifdef WLAN_FEATURE_ELNA
static void wmi_fwol_attach_elna_tlv(struct wmi_ops *ops)
{
	ops->send_set_elna_bypass_cmd = send_set_elna_bypass_cmd_tlv;
	ops->send_get_elna_bypass_cmd = send_get_elna_bypass_cmd_tlv;
	ops->extract_get_elna_bypass_resp = extract_get_elna_bypass_resp_tlv;
}
#else
static void wmi_fwol_attach_elna_tlv(struct wmi_ops *ops)
{
}
#endif /* WLAN_FEATURE_ELNA */

#ifdef WLAN_SEND_DSCP_UP_MAP_TO_FW
/**
 * send_dscp_tid_map_cmd_tlv() - send dscp to tid map  cmd to fw
 * @wmi_handle: wmi handle
 * @dscp_to_tid_map: array of dscp to tid map values
 *
 * Send WMI_PDEV_SET_DSCP_TID_MAP_CMDID to fw.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_dscp_tid_map_cmd_tlv(wmi_unified_t wmi_handle,
			  uint32_t *dscp_to_tid_map)
{
	QDF_STATUS status;
	wmi_pdev_set_dscp_tid_map_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed to allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_dscp_tid_map_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_dscp_tid_map_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_set_dscp_tid_map_cmd_fixed_param));
	cmd->reserved0 = WMI_PDEV_ID_SOC;
	qdf_mem_copy(&cmd->dscp_to_tid_map, dscp_to_tid_map,
		     sizeof(uint32_t) * WMI_DSCP_MAP_MAX);

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PDEV_SET_DSCP_TID_MAP_CMDID);
	if (status) {
		wmi_err("Failed to send dscp_up_map_to_fw %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

static void wmi_fwol_attach_dscp_tid_tlv(struct wmi_ops *ops)
{
	ops->send_dscp_tid_map_cmd = send_dscp_tid_map_cmd_tlv;
}
#else
static void wmi_fwol_attach_dscp_tid_tlv(struct wmi_ops *ops)
{
}
#endif /* WLAN_SEND_DSCP_UP_MAP_TO_FW */

void wmi_fwol_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	wmi_fwol_attach_elna_tlv(ops);
	wmi_fwol_attach_dscp_tid_tlv(ops);

}
