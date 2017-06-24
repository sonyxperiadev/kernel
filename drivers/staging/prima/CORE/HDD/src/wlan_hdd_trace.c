/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include "vos_trace.h"
#include "vos_types.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_main.h"

static tANI_U8* hddTraceGetEventString(tANI_U32 code)
{
    switch(code)
    {
           CASE_RETURN_STRING(TRACE_CODE_HDD_OPEN_REQUEST);
           CASE_RETURN_STRING(TRACE_CODE_HDD_STOP_REQUEST);
           CASE_RETURN_STRING(TRACE_CODE_HDD_TX_TIMEOUT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_P2P_DEV_ADDR_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETSUSPENDMODE_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETROAMTRIGGER_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETROAMTRIGGER_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETROAMSCANPERIOD_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETROAMSCANPERIOD_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETROAMDELTA_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETROAMDELTA_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETBAND_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETCOUNTRYREV_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETROAMSCANCHANNELS_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETROAMSCANCHANNELS_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_OPEN_REQUEST);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_STOP_REQUEST);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_UNINIT_REQUEST);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SOFTAP_TX_TIMEOUT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_SET_MAC_ADDR);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_P2P_SET_NOA_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_P2P_SET_PS_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_HOSTAPD_SET_SAP_CHANNEL_LIST_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_ADD_VIRTUAL_INTF);
           CASE_RETURN_STRING(TRACE_CODE_HDD_DEL_VIRTUAL_INTF);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CHANGE_VIRTUAL_INTF);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_START_AP);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_CHANGE_BEACON);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_STOP_AP);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_CHANGE_BSS);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_ADD_KEY);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_GET_KEY    );
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_DEFAULT_KEY);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_CONNECT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_DISCONNECT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_JOIN_IBSS);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_LEAVE_IBSS);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_WIPHY_PARAMS);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_TXPOWER);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_GET_TXPOWER);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SCAN);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SCHED_SCAN_START);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SCHED_SCAN_STOP);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_CHANNEL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_ADD_BEACON);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_BEACON);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_CHANGE_IFACE);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CHANGE_STATION);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_UPDATE_BSS);
           CASE_RETURN_STRING(TRACE_CODE_HDD_REMAIN_ON_CHANNEL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_REMAINCHANREADYHANDLER);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_CANCEL_REMAIN_ON_CHANNEL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_ACTION);
           CASE_RETURN_STRING(TRACE_CODE_HDD_MGMT_TX_CANCEL_WAIT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_GET_STA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_POWER_MGMT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_DEL_STA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_ADD_STA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_PMKSA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_DEL_PMKSA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_UPDATE_FT_IES);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_TDLS_MGMT);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_TDLS_OPER);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_REKEY_DATA);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_RESUME_WLAN);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SUSPEND_WLAN);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_SET_MAC_ACL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_TESTMODE);
           CASE_RETURN_STRING(TRACE_CODE_HDD_CFG80211_DUMP_SURVEY);
           CASE_RETURN_STRING(TRACE_CODE_HDD_UNSUPPORTED_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_SETROAMSCANCHANNELMINTIME_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_GETROAMSCANCHANNELMINTIME_IOCTL);
           CASE_RETURN_STRING(TRACE_CODE_HDD_STOP_NETDEV);
           CASE_RETURN_STRING(TRACE_CODE_HDD_WAKE_NETDEV);
           CASE_RETURN_STRING(TRACE_CODE_HDD_FLUSH_TX_QUEUES);
           default:
               return ("UNKNOWN");
               break;
    }
}

void hddTraceDump(void *pMac, tpvosTraceRecord pRecord, tANI_U16 recIndex)
{
    if (TRACE_CODE_HDD_RX_SME_MSG == pRecord->code)
    {
        hddLog(LOG1, "%04d %012u S%d %-14s %-30s(0x%x)",
            recIndex, pRecord->time, pRecord->session, "RX SME MSG:",
            get_eRoamCmdStatus_str(pRecord->data), pRecord->data);
    }
    else
    {
        hddLog(LOG1, "%04d %012u S%d %-14s %-30s(0x%x)",
            recIndex, pRecord->time, pRecord->session, "HDD Event:",
            hddTraceGetEventString(pRecord->code), pRecord->data);
    }
}

void hddTraceInit()
{
    vosTraceRegister(VOS_MODULE_ID_HDD, (tpvosTraceCb)&hddTraceDump);
}

/**
 * hdd_state_info_dump() - prints state information of hdd layer
 */
static void hdd_state_info_dump(void)
{
    v_CONTEXT_t vos_ctx_ptr;
    hdd_context_t *hdd_ctx_ptr = NULL;
    hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
    VOS_STATUS status;
    hdd_station_ctx_t *hdd_sta_ctx = NULL;
    hdd_adapter_t *adapter =NULL;

    /* get the global voss context */
    vos_ctx_ptr = vos_get_global_context(VOS_MODULE_ID_VOSS, NULL);

    if (!vos_ctx_ptr) {
        hddLog(LOGE, FL("Invalid Global VOSS Context"));
        VOS_ASSERT(0);
        return;
    }
    hdd_ctx_ptr = vos_get_context(VOS_MODULE_ID_HDD, vos_ctx_ptr);
    if (!hdd_ctx_ptr) {
       hddLog(LOGE, FL("HDD context is Null"));
       return;
    }

    hddLog(LOG1,
           FL("mScanPending %d isWlanSuspended %d disable_dfs_flag %d"),
           hdd_ctx_ptr->scan_info.mScanPending,
           hdd_ctx_ptr->isWlanSuspended, hdd_ctx_ptr->disable_dfs_flag);

    status = hdd_get_front_adapter(hdd_ctx_ptr, &adapter_node);

    while (NULL != adapter_node && VOS_STATUS_SUCCESS == status) {
       adapter = adapter_node->pAdapter;
       if (adapter->dev)
           hddLog(LOG1, FL("device name: %s"), adapter->dev->name);
       switch (adapter->device_mode) {
       case WLAN_HDD_INFRA_STATION:
       case WLAN_HDD_P2P_CLIENT:
           hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
           hddLog(LOG1, FL("connState: %d device_mode: %d"),
                  hdd_sta_ctx->conn_info.connState, adapter->device_mode);
           break;

       default:
           break;
       }
       status = hdd_get_next_adapter(hdd_ctx_ptr, adapter_node, &next);
       adapter_node = next;
    }
}

/**
 * hdd_register_debug_callback() - registration function for hdd layer
 * to print hdd state information
 */
void hdd_register_debug_callback()
{
    vos_register_debug_callback(VOS_MODULE_ID_HDD, &hdd_state_info_dump);
}
