/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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

/*============================================================================
   FILE:         host_diag_log.c

   OVERVIEW:     This source file contains definitions for WLAN UTIL diag APIs

   DEPENDENCIES:
   ============================================================================*/

#include "qdf_types.h"
#include "i_host_diag_core_log.h"
#include "host_diag_core_event.h"
#include "wlan_nlink_common.h"
#include "cds_sched.h"
#include "wlan_ptt_sock_svc.h"
#include "wlan_nlink_srv.h"
#include "cds_api.h"
#include "wlan_ps_wow_diag.h"
#include "qdf_str.h"

#define PTT_MSG_DIAG_CMDS_TYPE   (0x5050)

#define DIAG_TYPE_LOGS   (1)
#define DIAG_TYPE_EVENTS (2)

#define DIAG_SWAP16(A) ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))

typedef struct event_report_s {
	uint32_t diag_type;
	uint16_t event_id;
	uint16_t length;
} event_report_t;

/**---------------------------------------------------------------------------

   \brief host_diag_log_set_code() -

   This function sets the logging code in the given log record.

   \param  - ptr - Pointer to the log header type.
		- code - log code.
   \return - None

   --------------------------------------------------------------------------*/

void host_diag_log_set_code(void *ptr, uint16_t code)
{
	if (ptr) {
		/* All log packets are required to start with 'log_header_type' */
		((log_hdr_type *) ptr)->code = code;
	}
}

/**---------------------------------------------------------------------------

   \brief host_diag_log_set_length() -

   This function sets the length field in the given log record.

   \param  - ptr - Pointer to the log header type.
		- length - log length.

   \return - None

   --------------------------------------------------------------------------*/

void host_diag_log_set_length(void *ptr, uint16_t length)
{
	if (ptr) {
		/* All log packets are required to start with 'log_header_type' */
		((log_hdr_type *) ptr)->len = (uint16_t) length;
	}
}

/**---------------------------------------------------------------------------

   \brief host_diag_log_submit() -

   This function sends the log data to the ptt socket app only if it is registered with the driver.

   \param  - ptr - Pointer to the log header type.

   \return - None

   --------------------------------------------------------------------------*/

void host_diag_log_submit(void *plog_hdr_ptr)
{
	log_hdr_type *pHdr = (log_hdr_type *) plog_hdr_ptr;
	tAniHdr *wmsg = NULL;
	uint8_t *pBuf;
	uint16_t data_len;
	uint16_t total_len;

	if (cds_is_load_or_unload_in_progress())
		return;

	if (nl_srv_is_initialized() != 0)
		return;

	if (cds_is_multicast_logging()) {
		data_len = pHdr->len;

		total_len = sizeof(tAniHdr) + sizeof(uint32_t) + data_len;

		pBuf = (uint8_t *) qdf_mem_malloc(total_len);

		if (!pBuf)
			return;

		wmsg = (tAniHdr *) pBuf;
		wmsg->type = PTT_MSG_DIAG_CMDS_TYPE;
		wmsg->length = total_len;
		wmsg->length = DIAG_SWAP16(wmsg->length);
		pBuf += sizeof(tAniHdr);

		/*  Diag Type events or log */
		*(uint32_t *) pBuf = DIAG_TYPE_LOGS;
		pBuf += sizeof(uint32_t);

		memcpy(pBuf, pHdr, data_len);
		ptt_sock_send_msg_to_app (wmsg, 0, ANI_NL_MSG_PUMAC,
			INVALID_PID);
		qdf_mem_free((void *)wmsg);
	}
	return;
}

/**
 * host_diag_log_wlock() - This function is used to send wake lock diag events
 * @reason: Reason why the wakelock was taken or released
 * @wake_lock_name: Function in which the wakelock was taken or released
 * @timeout: Timeout value in case of timed wakelocks
 * @status: Status field indicating whether the wake lock was taken/released
 *
 * This function is used to send wake lock diag events to user space
 *
 * Return: None
 *
 */
void host_diag_log_wlock(uint32_t reason, const char *wake_lock_name,
		uint32_t timeout, uint32_t status)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event,
			struct host_event_wlan_wake_lock);

	if ((nl_srv_is_initialized() != 0) ||
	    (cds_is_wakelock_enabled() == false))
		return;

	wlan_diag_event.status = status;
	wlan_diag_event.reason = reason;
	wlan_diag_event.timeout = timeout;
	wlan_diag_event.name_len = strlen(wake_lock_name);
	strlcpy(&wlan_diag_event.name[0],
			wake_lock_name,
			wlan_diag_event.name_len+1);

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event, EVENT_WLAN_WAKE_LOCK);
}

/**---------------------------------------------------------------------------

   \brief host_diag_event_report_payload() -

   This function sends the event data to the ptt socket app only if it is
   registered with the driver.

   \param  - ptr - Pointer to the log header type.

   \return - None

   --------------------------------------------------------------------------*/

void host_diag_event_report_payload(uint16_t event_Id, uint16_t length,
				    void *pPayload)
{
	tAniHdr *wmsg = NULL;
	uint8_t *pBuf;
	event_report_t *pEvent_report;
	uint16_t total_len;

	if (cds_is_load_or_unload_in_progress())
		return;

	if (nl_srv_is_initialized() != 0)
		return;

	if (cds_is_multicast_logging()) {
		total_len = sizeof(tAniHdr) + sizeof(event_report_t) + length;

		pBuf = (uint8_t *) qdf_mem_malloc(total_len);

		if (!pBuf)
			return;

		wmsg = (tAniHdr *) pBuf;
		wmsg->type = PTT_MSG_DIAG_CMDS_TYPE;
		wmsg->length = total_len;
		wmsg->length = DIAG_SWAP16(wmsg->length);
		pBuf += sizeof(tAniHdr);

		pEvent_report = (event_report_t *) pBuf;
		pEvent_report->diag_type = DIAG_TYPE_EVENTS;
		pEvent_report->event_id = event_Id;
		pEvent_report->length = length;

		pBuf += sizeof(event_report_t);

		memcpy(pBuf, pPayload, length);

		if (ptt_sock_send_msg_to_app
			    (wmsg, 0, ANI_NL_MSG_PUMAC, INVALID_PID) < 0) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
				  "Ptt Socket error sending message to the app!!");
			qdf_mem_free((void *)wmsg);
			return;
		}

		qdf_mem_free((void *)wmsg);
	}

	return;

}

/**
 * host_log_low_resource_failure() - This function is used to send low
 * resource failure event
 * @event_sub_type: Reason why the failure was observed
 *
 * This function is used to send low resource failure events to user space
 *
 * Return: None
 *
 */
void host_log_low_resource_failure(uint8_t event_sub_type)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event,
			struct host_event_wlan_low_resource_failure);

	wlan_diag_event.event_sub_type = event_sub_type;

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event,
					EVENT_WLAN_LOW_RESOURCE_FAILURE);
}

void host_log_rsn_info(uint8_t *ucast_cipher, uint8_t *mcast_cipher,
		       uint8_t *akm_suite, uint8_t *group_mgmt)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event,
				 struct event_wlan_csr_rsn_info);

	qdf_mem_copy(wlan_diag_event.ucast_cipher, ucast_cipher,
		     RSN_OUI_SIZE);
	qdf_mem_copy(wlan_diag_event.mcast_cipher, mcast_cipher,
		     RSN_OUI_SIZE);
	qdf_mem_copy(wlan_diag_event.akm_suite, akm_suite,
		     RSN_OUI_SIZE);
	qdf_mem_copy(wlan_diag_event.group_mgmt, group_mgmt,
		     RSN_OUI_SIZE);

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event,
				    EVENT_WLAN_RSN_INFO);
}

void
host_log_wlan_auth_info(uint16_t auth_algo_num, uint16_t auth_tx_seq_num,
			uint16_t auth_status_code)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event,
				 struct event_wlan_lim_auth_info);

	wlan_diag_event.auth_algo_num = auth_algo_num;
	wlan_diag_event.auth_transaction_seq_num = auth_tx_seq_num;
	wlan_diag_event.auth_status_code = auth_status_code;

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event,
				    EVENT_WLAN_AUTH_INFO);
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * qdf_wow_wakeup_host_event()- send wow wakeup event
 * @wow_wakeup_cause: WOW wakeup reason code
 *
 * This function sends wow wakeup reason code diag event
 *
 * Return: void.
 */
void qdf_wow_wakeup_host_event(uint8_t wow_wakeup_cause)
{
	WLAN_HOST_DIAG_EVENT_DEF(wowRequest,
		host_event_wlan_powersave_wow_payload_type);
	qdf_mem_zero(&wowRequest, sizeof(wowRequest));

	wowRequest.event_subtype = WLAN_WOW_WAKEUP;
	wowRequest.wow_wakeup_cause = wow_wakeup_cause;
	WLAN_HOST_DIAG_EVENT_REPORT(&wowRequest,
		EVENT_WLAN_POWERSAVE_WOW);
}

void host_log_acs_req_event(uint8_t *intf, const uint8_t *hw_mode, uint16_t bw,
			    uint8_t ht, uint8_t vht, uint16_t chan_start,
			    uint16_t chan_end)
{
	WLAN_HOST_DIAG_EVENT_DEF(acs_req, struct host_event_wlan_acs_req);

	qdf_str_lcopy(acs_req.intf, intf, HOST_EVENT_INTF_STR_LEN);
	qdf_str_lcopy(acs_req.hw_mode, hw_mode, HOST_EVENT_HW_MODE_STR_LEN);
	acs_req.bw = bw;
	acs_req.ht = ht;
	acs_req.vht = vht;
	acs_req.chan_start = chan_start;
	acs_req.chan_end = chan_end;

	WLAN_HOST_DIAG_EVENT_REPORT(&acs_req, EVENT_WLAN_ACS_REQ);
}

void host_log_acs_scan_start(uint32_t scan_id, uint8_t vdev_id)
{
	WLAN_HOST_DIAG_EVENT_DEF(acs_scan_start,
				 struct host_event_wlan_acs_scan_start);

	acs_scan_start.scan_id = scan_id;
	acs_scan_start.vdev_id = vdev_id;

	WLAN_HOST_DIAG_EVENT_REPORT(&acs_scan_start,
				    EVENT_WLAN_ACS_SCAN_START);
}

void host_log_acs_scan_done(const uint8_t *status,
			    uint8_t vdev_id, uint32_t scan_id)
{
	WLAN_HOST_DIAG_EVENT_DEF(acs_scan_done,
				 struct host_event_wlan_acs_scan_done);

	qdf_str_lcopy(acs_scan_done.status, status, HOST_EVENT_STATUS_STR_LEN);
	acs_scan_done.vdev_id = vdev_id;
	acs_scan_done.scan_id = scan_id;

	WLAN_HOST_DIAG_EVENT_REPORT(&acs_scan_done, EVENT_WLAN_ACS_SCAN_DONE);
}

void host_log_acs_chan_spect_weight(uint16_t chan, uint16_t weight,
				    int32_t rssi, uint16_t bss_count)
{
	WLAN_HOST_DIAG_EVENT_DEF(
		acs_chan_spect_weight,
		struct host_event_wlan_acs_chan_spectral_weight);

	acs_chan_spect_weight.chan = chan;
	acs_chan_spect_weight.weight = weight;
	acs_chan_spect_weight.rssi = rssi;
	acs_chan_spect_weight.bss_count = bss_count;

	WLAN_HOST_DIAG_EVENT_REPORT(&acs_chan_spect_weight,
				    EVENT_WLAN_ACS_CHANNEL_SPECTRAL_WEIGHT);
}

void host_log_acs_best_chan(uint16_t chan, uint16_t weight)
{
	WLAN_HOST_DIAG_EVENT_DEF(acs_best_chan,
				 struct host_event_wlan_acs_best_chan);

	acs_best_chan.chan = chan;
	acs_best_chan.weight = weight;

	WLAN_HOST_DIAG_EVENT_REPORT(&acs_best_chan,
				    EVENT_WLAN_ACS_BEST_CHANNEL);
}

#endif
