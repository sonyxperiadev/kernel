/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

/*
 * This file limRMC.c contains the code
 * for processing RMC messages
 *
 */
#include "wniApi.h"
#include "wniCfg.h"
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limUtils.h"
#include "limTimerUtils.h"
#include "limSendMessages.h"
#include "limSendMessages.h"
#include "limSession.h"
#include "limSessionUtils.h"
#include "wlan_qct_wda.h"
#include "wlan_qct_tli.h"
#include "limRMC.h"

#ifdef WLAN_FEATURE_RMC

static tANI_U8
__rmcGroupHashFunction(tSirMacAddr transmitter)
{
    tANI_U16 hash;

    /*
     * Generate a hash using transmitter address
     */
    hash = transmitter[0] + transmitter[1] + transmitter[2] +
            transmitter[3] + transmitter[4] + transmitter[5];

    return hash & (RMC_MCAST_GROUPS_HASH_SIZE - 1);
}


static tLimRmcGroupContext *
__rmcGroupLookupHashEntry(tpAniSirGlobal pMac, tSirMacAddr transmitter)
{
    tANI_U8 index;
    tLimRmcGroupContext *entry;

    index = __rmcGroupHashFunction(transmitter);

    /* Pick the correct hash table based on role */
    entry = pMac->rmcContext.rmcGroupRxHashTable[index];

    PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Lookup:[%d] transmitter "
                         MAC_ADDRESS_STR ), index,
                         MAC_ADDR_ARRAY(transmitter));)
    while (entry)
    {
        if (vos_mem_compare(transmitter, entry->transmitter,
             sizeof(v_MACADDR_t)))
        {
            return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

static tLimRmcGroupContext *
__rmcGroupInsertHashEntry(tpAniSirGlobal pMac, tSirMacAddr transmitter)
{
    tANI_U8 index;
    tLimRmcGroupContext *entry;
    tLimRmcGroupContext **head;

    index = __rmcGroupHashFunction(transmitter);

    PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Insert:[%d] group " MAC_ADDRESS_STR
                             " transmitter " MAC_ADDRESS_STR), index,
                             MAC_ADDR_ARRAY(mcastGroupAddr),
                             MAC_ADDR_ARRAY(transmitter));)

    head = &pMac->rmcContext.rmcGroupRxHashTable[index];

    entry = __rmcGroupLookupHashEntry(pMac, transmitter);

    if (entry)
    {
        /* If the entry exists, return it at the end */
        PELOGE(limLog(pMac, LOGE, FL("RMC: Hash Insert:"
                 MAC_ADDRESS_STR "exists"), MAC_ADDR_ARRAY(transmitter));)
    }
    else
    {
        entry = (tLimRmcGroupContext *)vos_mem_malloc(sizeof(*entry));

        PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Insert:new entry %p"), entry);)

        if (entry)
        {
            vos_mem_copy(entry->transmitter, transmitter, sizeof(tSirMacAddr));
            entry->isRuler = eRMC_IS_NOT_A_RULER;

            /* chain this entry */
            entry->next = *head;
            *head = entry;
        }
        else
        {
            PELOGE(limLog(pMac, LOGE, FL("RMC: Hash Insert:" MAC_ADDRESS_STR
                             " alloc failed"), MAC_ADDR_ARRAY(transmitter));)
        }
    }

    return entry;
}

/**
 *  __rmcGroupDeleteHashEntry()
 *
 *FUNCTION:
 * This function is called to delete a RMC group entry
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *  Should be called with lkRmcLock held.
 *
 *NOTE:
 * Make sure (for the transmitter role) that the entry is
 * not in the Pending Response queue.
 *
 * @param  transmitter - address of multicast transmitter
 *
 * @return status
 */
static tSirRetStatus
__rmcGroupDeleteHashEntry(tpAniSirGlobal pMac, tSirMacAddr transmitter)
{
    tSirRetStatus status = eSIR_FAILURE;
    tANI_U8 index;
    tLimRmcGroupContext *entry, *prev, **head;

    index = __rmcGroupHashFunction(transmitter);

    head = &pMac->rmcContext.rmcGroupRxHashTable[index];
    entry = *head;
    prev = NULL;

    while (entry)
    {
        if (vos_mem_compare(transmitter, entry->transmitter,
             sizeof(v_MACADDR_t)))
        {
            if (*head == entry)
            {
                *head = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }

            PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Delete: entry %p "
                         " transmitter " MAC_ADDRESS_STR), entry
                             MAC_ADDR_ARRAY(transmitter));)

            /* free the group entry */
            vos_mem_free(entry);

            status = eSIR_SUCCESS;
            break;
        }

        prev = entry;
        entry = entry->next;
    }

    return status;
}

static void
__rmcGroupDeleteAllEntries(tpAniSirGlobal pMac)
{
    tLimRmcGroupContext *entry, **head;
    int index;

    PELOG1(limLog(pMac, LOG1, FL("RMC: Hash_Delete_All"),);)

    for (index = 0; index < RMC_MCAST_GROUPS_HASH_SIZE; index++)
    {
        head = &pMac->rmcContext.rmcGroupRxHashTable[index];

        entry = *head;

        while (entry)
        {
            *head = entry->next;
            /* free the group entry */
            vos_mem_free(entry);
            entry = *head;
        }
    }
}

static void
__limPostMsgRulerReq ( tpAniSirGlobal pMac,
                        tANI_U8 cmd,
                        tSirMacAddr mcastTransmitter)
{
    tSirMsgQ msg;
    tSirRmcRulerReq *pRulerReq;

    pRulerReq = vos_mem_malloc(sizeof(*pRulerReq));
    if (NULL == pRulerReq)
    {
       limLog(pMac, LOGE, FL("AllocateMemory() failed"));
       return;
    }

    pRulerReq->cmd = cmd;

    vos_mem_copy(pRulerReq->mcastTransmitter, mcastTransmitter,
                 sizeof(tSirMacAddr));

    /* Initialize black list */
    vos_mem_zero(pRulerReq->blacklist, sizeof(pRulerReq->blacklist));

    if (eRMC_SUGGEST_RULER_CMD == cmd)
    {
        /* TODO - Set the black list. */
    }

    msg.type = WDA_RMC_RULER_REQ;
    msg.bodyptr = pRulerReq;
    msg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));
    if (eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
    {
        vos_mem_free(pRulerReq);
        limLog(pMac, LOGE, FL("wdaPostCtrlMsg() failed"));
    }

    return;
}

static void
__limPostMsgUpdateInd ( tpAniSirGlobal pMac,
                        tANI_U8 indication,
                        tANI_U8 role,
                        tSirMacAddr mcastTransmitter,
                        tSirMacAddr mcastRuler)
{
    tSirMsgQ msg;
    tSirRmcUpdateInd *pUpdateInd;

    pUpdateInd = vos_mem_malloc(sizeof(*pUpdateInd));
    if ( NULL == pUpdateInd )
    {
       limLog(pMac, LOGE, FL("AllocateMemory() failed"));
       return;
    }

    vos_mem_zero(pUpdateInd, sizeof(*pUpdateInd));

    pUpdateInd->indication = indication;
    pUpdateInd->role = role;

    vos_mem_copy(pUpdateInd->mcastTransmitter,
            mcastTransmitter, sizeof(tSirMacAddr));

    vos_mem_copy(pUpdateInd->mcastRuler,
            mcastRuler, sizeof(tSirMacAddr));

    msg.type = WDA_RMC_UPDATE_IND;
    msg.bodyptr = pUpdateInd;
    msg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));
    if (eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
    {
        vos_mem_free(pUpdateInd);
        limLog(pMac, LOGE, FL("wdaPostCtrlMsg() failed"));
    }

    return;
}

static char *
__limRulerMessageToString(eRmcMessageType msgType)
{
    switch (msgType)
    {
        default:
            return "Invalid";
        case eLIM_RMC_ENABLE_REQ:
            return "RMC_ENABLE_REQ";
        case eLIM_RMC_DISABLE_REQ:
            return "RMC_DISABLE_REQ";
        case eLIM_RMC_RULER_SELECT_RESP:
            return "RMC_RULER_SELECT_RESP";
        case eLIM_RMC_RULER_PICK_NEW:
            return "RMC_RULER_PICK_NEW";
        case eLIM_RMC_OTA_RULER_INFORM_ACK:
            return "RMC_OTA_RULER_INFORM_ACK";
        case eLIM_RMC_OTA_RULER_INFORM_SELECTED:
            return "RMC_OTA_RULER_INFORM_SELECTED";
        case eLIM_RMC_BECOME_RULER_RESP:
            return "RMC_BECOME_RULER_RESP";
        case eLIM_RMC_OTA_RULER_INFORM_CANCELLED:
            return "RMC_OTA_RULER_INFORM_CANCELLED";
    }
}

static char *
__limRulerStateToString(eRmcRulerState state)
{
    switch (state)
    {
        default:
            return "Invalid";
        case eRMC_IS_NOT_A_RULER:
            return "Device Not a Ruler";
        case eRMC_RULER_PENDING:
            return "Pending firmware resp";
        case eRMC_IS_A_RULER:
            return "Device is Ruler";
    }
}

static char *
__limMcastTxStateToString(eRmcMcastTxState state)
{
    switch (state)
    {
        default:
            return "Invalid";
        case eRMC_RULER_NOT_SELECTED:
            return "Not Selected";
        case eRMC_RULER_ENABLE_REQUESTED:
            return "Enable Requested";
        case eRMC_RULER_OTA_REQUEST_SENT:
            return "OTA Request Sent";
        case eRMC_RULER_ACTIVE:
            return "Active";
    }
}

/**
 * __rmcRulerSelectTimerHandler()
 *
 *FUNCTION:
 * This function is called upon timer expiry.
 *
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * Only one entry is processed for every invocation if this routine.
 * This allows us to use a single timer and makes sure we do not
 * timeout a request too early.
 *
 * @param  param - Message corresponding to the timer that expired
 *
 * @return None
 */

void
__rmcRulerSelectTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;
    tSirMacAddr zeroMacAddr = { 0, 0, 0, 0, 0, 0 };
    tSirRetStatus status;
    tSirRMCInfo RMC;
    tpPESession psessionEntry;
    tANI_U32 cfgValue;

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE,
             FL("RMC:__rmcRulerSelectTimerHandler:No active IBSS"));)
        return;
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY,
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get Action Period Frequency value
         * from CFG. Log error.
         */
        limLog(pMac, LOGE, FL("could not retrieve ActionPeriodFrequency"));
    }

    cfgValue = SYS_MS_TO_TICKS(cfgValue);

    if (pMac->rmcContext.rmcTimerValInTicks != cfgValue)
    {
        limLog(pMac, LOG1, FL("RMC RulerSelect timer value changed"));
        if (tx_timer_change(&pMac->rmcContext.gRmcRulerSelectTimer,
                 cfgValue, 0) != TX_SUCCESS)
        {
            limLog(pMac, LOGE,
                FL("Unable to change RulerSelect Timer val"));
        }
        pMac->rmcContext.rmcTimerValInTicks = cfgValue;
    }

    /*
     * If we are in the scanning state then we need to return
     * from this function without any further processing
     */
    if (eLIM_HAL_SCANNING_STATE == pMac->lim.gLimHalScanState)
    {
        limLog(pMac, LOG1, FL("In scanning state, can't send action frm"));
        if (tx_timer_activate(&pMac->rmcContext.gRmcRulerSelectTimer) !=
            TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("In scanning state, "
                                  "couldn't activate RMC RulerSelect timer"));
        }
        return;
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
             FL("__rmcRulerSelectTimerHandler lock acquire failed"));
        if (tx_timer_activate(&pMac->rmcContext.gRmcRulerSelectTimer)!= TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not activate RMC RulerSelect timer"));
        }
        return;
    }

    vos_mem_copy(&RMC.mcastRuler, &pMac->rmcContext.ruler,
                     sizeof(tSirMacAddr));

    if (VOS_FALSE == vos_mem_compare(&zeroMacAddr,
                            &pMac->rmcContext.ruler, sizeof(tSirMacAddr)))
    {
        limLog(pMac, LOG1,
               FL("RMC Periodic Ruler_Select Ruler " MAC_ADDRESS_STR),
                   MAC_ADDR_ARRAY(pMac->rmcContext.ruler));
        /*
         * Re-arm timer
         */
        if (tx_timer_activate(&pMac->rmcContext.gRmcRulerSelectTimer)!=
            TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not activate RMC Response timer"));
        }

        if (!VOS_IS_STATUS_SUCCESS
                (vos_lock_release(&pMac->rmcContext.lkRmcLock)))
        {
            limLog(pMac, LOGE,
                FL("RMC: __rmcRulerSelectTimerHandler lock release failed"));
        }
    }
    else
    {
        limLog(pMac, LOGE,
               FL("RMC Deactivating timer because no ruler was selected"));

        if (!VOS_IS_STATUS_SUCCESS
                (vos_lock_release(&pMac->rmcContext.lkRmcLock)))
        {
            limLog(pMac, LOGE,
                FL("RMC: __rmcRulerSelectTimerHandler lock release failed"));
        }

        return;
    }

    RMC.dialogToken = 0;
    RMC.action = SIR_MAC_RMC_RULER_INFORM_SELECTED;

    status = limSendRMCActionFrame(pMac,
                          SIR_MAC_RMC_MCAST_ADDRESS,
                          &RMC,
                          psessionEntry);

    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
         FL("RMC:__rmcRulerSelectTimerHandler Action frame send failed"));)
    }

    return;
}

static void
__limProcessRMCEnableRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSetRMCReq *setRmcReq = (tSirSetRMCReq *)pMsgBuf;
    tpPESession psessionEntry;

    if (!setRmcReq)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Enable:NULL message") );)
        return;
    }

    pMac->rmcContext.rmcEnabled = TRUE;

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Enable RMC request no active IBSS"));)
        pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;
        return;
    }

    /* Send RULER_REQ to f/w */
    __limPostMsgRulerReq(pMac, eRMC_SUGGEST_RULER_CMD,
                        setRmcReq->mcastTransmitter);

    pMac->rmcContext.state = eRMC_RULER_ENABLE_REQUESTED;
}

static void
__limProcessRMCDisableRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpPESession psessionEntry;
    tSirRMCInfo RMC;
    tSirSetRMCReq *setRmcReq = (tSirSetRMCReq *)pMsgBuf;
    tSirRetStatus status;
    v_PVOID_t pvosGCtx;
    VOS_STATUS vos_status;
    v_MACADDR_t vosMcastTransmitter;

    pMac->rmcContext.rmcEnabled = FALSE;

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Disable:No active IBSS"));)
        return;
    }

    if (!setRmcReq)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Disable:NULL message") );)
        return;
    }

    /* Cancel pending timer */
    tx_timer_deactivate(&pMac->rmcContext.gRmcRulerSelectTimer);

    vosMcastTransmitter.bytes[0] = psessionEntry->selfMacAddr[0];
    vosMcastTransmitter.bytes[1] = psessionEntry->selfMacAddr[1];
    vosMcastTransmitter.bytes[2] = psessionEntry->selfMacAddr[2];
    vosMcastTransmitter.bytes[3] = psessionEntry->selfMacAddr[3];
    vosMcastTransmitter.bytes[4] = psessionEntry->selfMacAddr[4];
    vosMcastTransmitter.bytes[5] = psessionEntry->selfMacAddr[5];

    pvosGCtx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *) pMac);
    vos_status = WLANTL_DisableRMC(pvosGCtx, &vosMcastTransmitter);

    if (VOS_STATUS_SUCCESS != vos_status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Disable: TL disable failed"));)
    }

    if (pMac->rmcContext.state == eRMC_RULER_ACTIVE)
    {
        RMC.dialogToken = 0;
        RMC.action = SIR_MAC_RMC_RULER_INFORM_CANCELLED;
        vos_mem_copy(&RMC.mcastRuler, &pMac->rmcContext.ruler, sizeof(tSirMacAddr));

        status = limSendRMCActionFrame(pMac, pMac->rmcContext.ruler,
                             &RMC, psessionEntry);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE, FL("RMC:Disable: Action frame send failed"));)
        }

        pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;
    }

    __limPostMsgUpdateInd(pMac, eRMC_RULER_CANCELLED, eRMC_TRANSMITTER_ROLE,
                         setRmcReq->mcastTransmitter, pMac->rmcContext.ruler);

    vos_mem_zero(pMac->rmcContext.ruler, sizeof(tSirMacAddr));

}

static void
__limProcessRMCRulerSelectResponse(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcRulerSelectInd *pRmcRulerSelectInd;
    tpPESession psessionEntry;
    tSirRetStatus status;
    v_PVOID_t pvosGCtx;
    VOS_STATUS vos_status;
    v_MACADDR_t vosMcastTransmitter;
    tSirRMCInfo RMC;

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Select_Resp:NULL message"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Ruler_Select_Resp:No active IBSS"));)
        pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;
        return;
    }

    pRmcRulerSelectInd = (tSirRmcRulerSelectInd *)pMsgBuf;

    if (pMac->rmcContext.state != eRMC_RULER_ENABLE_REQUESTED)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Select_Resp:Bad state %s"),
                        __limMcastTxStateToString(pMac->rmcContext.state) );)
        return;
    }

    if (pRmcRulerSelectInd->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Ruler_Select_Resp:FW Status %d"),
                        pRmcRulerSelectInd->status);)
        pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;
        return;
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Ruler_Select_Resp:lock acquire failed"));
        pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;
        return;
    }

    vos_mem_copy(&pMac->rmcContext.ruler, &pRmcRulerSelectInd->ruler[0],
                 sizeof(tSirMacAddr));

    if (!VOS_IS_STATUS_SUCCESS
            (vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC: Ruler_Select_Resp: lock release failed"));
    }

    RMC.dialogToken = 0;
    RMC.action = SIR_MAC_RMC_RULER_INFORM_SELECTED;
    vos_mem_copy(&RMC.mcastRuler, &pRmcRulerSelectInd->ruler[0],
                 sizeof(tSirMacAddr));

    PELOG1(limLog(pMac, LOG1, FL("RMC: Ruler_Select :ruler " MAC_ADDRESS_STR),
             MAC_ADDR_ARRAY(pRmcRulerSelectInd->ruler[0]));)

    status = limSendRMCActionFrame(pMac,
                          SIR_MAC_RMC_MCAST_ADDRESS,
                          &RMC,
                          psessionEntry);

    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
         FL("RMC: Ruler_Select_Resp: Action send failed"));)
    }

    __limPostMsgUpdateInd(pMac, eRMC_RULER_ACCEPTED, eRMC_TRANSMITTER_ROLE,
                 psessionEntry->selfMacAddr, pMac->rmcContext.ruler);

    vosMcastTransmitter.bytes[0] = psessionEntry->selfMacAddr[0];
    vosMcastTransmitter.bytes[1] = psessionEntry->selfMacAddr[1];
    vosMcastTransmitter.bytes[2] = psessionEntry->selfMacAddr[2];
    vosMcastTransmitter.bytes[3] = psessionEntry->selfMacAddr[3];
    vosMcastTransmitter.bytes[4] = psessionEntry->selfMacAddr[4];
    vosMcastTransmitter.bytes[5] = psessionEntry->selfMacAddr[5];

    /* Enable TL */
    pvosGCtx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *) pMac);
    vos_status = WLANTL_EnableRMC(pvosGCtx, &vosMcastTransmitter);

    pMac->rmcContext.state = eRMC_RULER_ACTIVE;

    if (tx_timer_activate(&pMac->rmcContext.gRmcRulerSelectTimer)!= TX_SUCCESS)
    {
        limLog(pMac, LOGE,
         FL("Ruler_Select_Resp:Activate RMC Response timer failed"));
    }
}

static void
__limProcessRMCRulerPickNew(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcUpdateInd *pRmcUpdateInd;
    tpPESession psessionEntry;
    tSirRetStatus status;
    tSirRMCInfo RMC;
    v_PVOID_t pvosGCtx;
    VOS_STATUS vos_status;
    v_MACADDR_t vosMcastTransmitter;
    tSirMacAddr zeroMacAddr = { 0, 0, 0, 0, 0, 0 };

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Pick_New:NULL message"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Pick_New:No active IBSS"));)
        return;
    }

    pvosGCtx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *) pMac);

    pRmcUpdateInd = (tSirRmcUpdateInd *)pMsgBuf;

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Ruler_Pick_New:lock acquire failed"));
        return;
    }


    /* Fill out Action frame parameters */
    RMC.dialogToken = 0;

    if (VOS_FALSE == vos_mem_compare(&zeroMacAddr,
                        &pRmcUpdateInd->mcastRuler,
                        sizeof(tSirMacAddr)))
    {

        vos_mem_copy(&RMC.mcastRuler, &pRmcUpdateInd->mcastRuler,
                     sizeof(tSirMacAddr));

        RMC.action = SIR_MAC_RMC_RULER_INFORM_CANCELLED;
        status = limSendRMCActionFrame(pMac,
                         pRmcUpdateInd->mcastRuler,
                         &RMC, psessionEntry);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE,
                FL("RMC:Ruler_Pick_New: Inform_Cancel Action send failed"));)
            goto done;
        }

        vosMcastTransmitter.bytes[0] = psessionEntry->selfMacAddr[0];
        vosMcastTransmitter.bytes[1] = psessionEntry->selfMacAddr[1];
        vosMcastTransmitter.bytes[2] = psessionEntry->selfMacAddr[2];
        vosMcastTransmitter.bytes[3] = psessionEntry->selfMacAddr[3];
        vosMcastTransmitter.bytes[4] = psessionEntry->selfMacAddr[4];
        vosMcastTransmitter.bytes[5] = psessionEntry->selfMacAddr[5];

        vos_status = WLANTL_DisableRMC(pvosGCtx, &vosMcastTransmitter);

        if (VOS_STATUS_SUCCESS != vos_status)
        {
            PELOGE(limLog(pMac, LOGE,
                 FL("RMC:Ruler_Pick_New: TL disable failed"));)
        }
    }

    vos_mem_copy(pMac->rmcContext.ruler, pRmcUpdateInd->ruler[0],
                 sizeof(tSirMacAddr));

    pMac->rmcContext.state = eRMC_RULER_NOT_SELECTED;

    if (VOS_TRUE == vos_mem_compare(&zeroMacAddr,
                        pMac->rmcContext.ruler,
                        sizeof(tSirMacAddr)))
    {
        PELOGE(limLog(pMac, LOGE,
           FL("RMC:Ruler_Pick_New: No candidate rulers available"));)
        goto done;
    }


    RMC.action = SIR_MAC_RMC_RULER_INFORM_SELECTED;
    vos_mem_copy(&RMC.mcastRuler, &pMac->rmcContext.ruler,
                     sizeof(tSirMacAddr));
    status = limSendRMCActionFrame(pMac, SIR_MAC_RMC_MCAST_ADDRESS,
                         &RMC, psessionEntry);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
           FL("RMC:Ruler_Pick_New: Inform_Selected Action send failed"));)
        goto done;
    }

    __limPostMsgUpdateInd(pMac, eRMC_RULER_ACCEPTED, eRMC_TRANSMITTER_ROLE,
                         psessionEntry->selfMacAddr, pMac->rmcContext.ruler);

    vosMcastTransmitter.bytes[0] = psessionEntry->selfMacAddr[0];
    vosMcastTransmitter.bytes[1] = psessionEntry->selfMacAddr[1];
    vosMcastTransmitter.bytes[2] = psessionEntry->selfMacAddr[2];
    vosMcastTransmitter.bytes[3] = psessionEntry->selfMacAddr[3];
    vosMcastTransmitter.bytes[4] = psessionEntry->selfMacAddr[4];
    vosMcastTransmitter.bytes[5] = psessionEntry->selfMacAddr[5];

    /* Enable TL */
    vos_status = WLANTL_EnableRMC(pvosGCtx, &vosMcastTransmitter);

    if (VOS_STATUS_SUCCESS != vos_status)
    {
        PELOGE(limLog(pMac, LOGE,
            FL("RMC:Ruler_Pick_New: TL enable failed"));)
        goto done;
    }

    pMac->rmcContext.state = eRMC_RULER_ACTIVE;

    if (tx_timer_activate(&pMac->rmcContext.gRmcRulerSelectTimer)!= TX_SUCCESS)
    {
        limLog(pMac, LOGE,
         FL("Ruler_Pick_New:Activate RMC Response timer failed"));
    }

done:
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Ruler_Pick_New: lock release failed"));
    }
}

static void
__limProcessRMCRulerInformSelected(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirMacMgmtHdr pHdr;
    tANI_U8 *pFrameData;
    tANI_U32 frameLen;
    tLimRmcGroupContext *entry;
    tpPESession psessionEntry;
    tSirRetStatus status;

    if (!pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Inform:NULL msg"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Become_Ruler_Resp:No active IBSS"));)
        return;
    }

    /*
     * Get the frame header
     */
    pHdr = WDA_GET_RX_MAC_HEADER((tANI_U8 *)pMsgBuf);

    frameLen = WDA_GET_RX_PAYLOAD_LEN((tANI_U8 *)pMsgBuf);
    if (frameLen < sizeof(tSirMacIbssExtNetworkFrameHdr))
    {
        PELOGE(limLog(pMac, LOGE,
             FL("RMC: Ruler_Inform:Bad length %d "), frameLen);)
        return;
    }

    pFrameData = WDA_GET_RX_MPDU_DATA((tANI_U8 *)pMsgBuf) +
                    sizeof(tSirMacIbssExtNetworkFrameHdr);

    if (!pFrameData)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Inform:NULL data"));)
        return;
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Become_Ruler_Resp:lock acquire failed"));
        return;
    }

    /*
     * Check if this transmitter exists in our database.
     */
    entry = __rmcGroupLookupHashEntry(pMac, pHdr->sa);

    if (VOS_FALSE == vos_mem_compare(pFrameData, psessionEntry->selfMacAddr,
                                     sizeof(tSirMacAddr)))
    {
        if (entry)
        {
            PELOG1(limLog(pMac, LOG1,
                 FL("RMC: Ruler_Inform: Ruler Cancelled"));)

            __limPostMsgUpdateInd(pMac, eRMC_RULER_CANCELLED,
                      eRMC_RULER_ROLE, pHdr->sa, psessionEntry->selfMacAddr);

            /*
             * Delete hash entry for this Group address.
             */
            status = __rmcGroupDeleteHashEntry(pMac, pHdr->sa);
            if (eSIR_FAILURE == status)
            {
                PELOGE(limLog(pMac, LOGE,
                      FL("RMC: Ruler_Inform:hash delete failed"));)
            }
        }
    }
    else
    {
        if (NULL == entry)
        {
            /* Add the transmitter address to the hash */
            entry = __rmcGroupInsertHashEntry(pMac, pHdr->sa);
            if (entry)
            {
                if (entry->isRuler != eRMC_RULER_PENDING)
                {
                    __limPostMsgRulerReq(pMac, eRMC_BECOME_RULER_CMD,
                                         pHdr->sa);
                    entry->isRuler = eRMC_RULER_PENDING;
                }
            }
            else
            {
                PELOGE(limLog(pMac, LOGE,
                         FL("RMC: Ruler_Inform:Hash insert failed"));)
            }

        }
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Ruler_Inform: lock release failed"));
    }

}

static void
__limProcessRMCBecomeRulerResp(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcBecomeRulerInd *pRmcBecomeRulerInd;
    tLimRmcGroupContext *entry;
    tSirRetStatus status = eSIR_SUCCESS;

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Ruler_Resp:NULL message"));)
        return;
    }

    pRmcBecomeRulerInd = (tSirRmcBecomeRulerInd *)pMsgBuf;

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Become_Ruler_Resp:lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac,
                  pRmcBecomeRulerInd->mcastTransmitter);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Ruler_Resp: No entry"));)
        goto done;
    }

    if (pRmcBecomeRulerInd->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Become_Ruler_Resp:FW Status %d"),
                        pRmcBecomeRulerInd->status);)
        status = eSIR_FAILURE;
        goto done;
    }

    if (entry->isRuler != eRMC_RULER_PENDING)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Ruler_Resp:Bad state: %s"),
                        __limRulerStateToString(entry->isRuler) );)
        status = eSIR_FAILURE;
        goto done;
    }

    entry->isRuler = eRMC_IS_A_RULER;

done:
    if (eSIR_FAILURE == status)
    {
        status = __rmcGroupDeleteHashEntry(pMac,
                       pRmcBecomeRulerInd->mcastTransmitter);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE,
                      FL("RMC: Become_Ruler_Resp:hash delete failed"));)
        }
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Become_Ruler_Resp: lock release failed"));
    }

    return;
}

static void
__limProcessRMCRulerInformCancelled(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirMacMgmtHdr pHdr;
    tANI_U8 *pFrameData;
    tANI_U32 frameLen;
    tSirRetStatus status;
    tLimRmcGroupContext *entry;
    tpPESession psessionEntry;

    if (!pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Inform_Cancel:NULL msg"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE,
             FL("RMC:Ruler_Inform_Cancel:No active IBSS"));)
        return;
    }

    pHdr = WDA_GET_RX_MAC_HEADER((tANI_U8 *)pMsgBuf);

    frameLen = WDA_GET_RX_PAYLOAD_LEN((tANI_U8 *)pMsgBuf);
    if (frameLen < sizeof(tSirMacIbssExtNetworkFrameHdr))
    {
        PELOGE(limLog(pMac, LOGE,
             FL("RMC: Ruler_Inform:Bad length %d "), frameLen);)
        return;
    }

    pFrameData = WDA_GET_RX_MPDU_DATA((tANI_U8 *)pMsgBuf) +
                    sizeof(tSirMacIbssExtNetworkFrameHdr);

    if (!pFrameData)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Inform_Cancel:NULL data"));)
        return;
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Ruler_Inform_Cancel lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac, pHdr->sa);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Ruler_Inform_Cancel: No entry"));)
        goto done;
    }

    __limPostMsgUpdateInd(pMac, eRMC_RULER_CANCELLED,
                     eRMC_RULER_ROLE, pHdr->sa, psessionEntry->selfMacAddr);

    /*
     * Delete hash entry for this Group address.
     */
    status = __rmcGroupDeleteHashEntry(pMac, pHdr->sa);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
                  FL("RMC: Ruler_Inform_Cancel:hash delete failed"));)
    }

done:
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Ruler_Inform_Cancel: lock release failed"));
    }
    return;
}

void
limProcessRMCMessages(tpAniSirGlobal pMac, eRmcMessageType msgType,
                      tANI_U32 *pMsgBuf)
{

    if (pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Buffer is Pointing to NULL"));)
        return;
    }

    limLog(pMac, LOG1, FL("RMC: limProcessRMCMessages: %s"),
                        __limRulerMessageToString(msgType));

    switch (msgType)
    {
        case eLIM_RMC_ENABLE_REQ:
            __limProcessRMCEnableRequest(pMac, pMsgBuf);
            break;

        case eLIM_RMC_DISABLE_REQ:
            __limProcessRMCDisableRequest(pMac, pMsgBuf);
            break;

        case eLIM_RMC_RULER_SELECT_RESP:
            __limProcessRMCRulerSelectResponse(pMac, pMsgBuf);
            break;

        case eLIM_RMC_RULER_PICK_NEW:
            __limProcessRMCRulerPickNew(pMac, pMsgBuf);
            break;

        case eLIM_RMC_OTA_RULER_INFORM_SELECTED:
            __limProcessRMCRulerInformSelected(pMac, pMsgBuf);
            break;

        case eLIM_RMC_BECOME_RULER_RESP:
            __limProcessRMCBecomeRulerResp(pMac, pMsgBuf);
            break;

        case eLIM_RMC_OTA_RULER_INFORM_CANCELLED:
            __limProcessRMCRulerInformCancelled(pMac, pMsgBuf);
            break;


        default:
            break;
    } // switch (msgType)
    return;
} /*** end limProcessRMCMessages() ***/

void
limRmcInit(tpAniSirGlobal pMac)
{
    tANI_U32 cfgValue;

    if (wlan_cfgGetInt(pMac, WNI_CFG_RMC_ACTION_PERIOD_FREQUENCY,
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get Action Period Frequency value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP, FL("could not retrieve ActionPeriodFrequency"));
    }

    cfgValue = SYS_MS_TO_TICKS(cfgValue);

    vos_mem_zero(&pMac->rmcContext, sizeof(pMac->rmcContext));

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_init(&pMac->rmcContext.lkRmcLock)))
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC lock init failed!"));)
    }

    if (tx_timer_create(&pMac->rmcContext.gRmcRulerSelectTimer,
                            "RMC RSP TIMEOUT",
                            __rmcRulerSelectTimerHandler,
                            0 /* param */,
                            cfgValue, 0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        limLog(pMac, LOGE, FL("could not create RMC response timer"));
    }

    pMac->rmcContext.rmcTimerValInTicks = cfgValue;
}

void
limRmcCleanup(tpAniSirGlobal pMac)
{
    limRmcIbssDelete(pMac);

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_destroy(&pMac->rmcContext.lkRmcLock)))
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC lock destroy failed!"));)
    }

    tx_timer_delete(&pMac->rmcContext.gRmcRulerSelectTimer);
}

void
limRmcTransmitterDelete(tpAniSirGlobal pMac, tSirMacAddr transmitter)
{
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
             FL("RMC: limRMCTransmitterDelete lock acquire failed"));
        return;
    }

    __rmcGroupDeleteHashEntry(pMac, transmitter);

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: limRMCTransmitterDelete lock release failed"));
    }

    limLog(pMac, LOG1, FL("RMC: limRmcTransmitterDelete complete"));
}

void
limRmcIbssDelete(tpAniSirGlobal pMac)
{
    tpPESession psessionEntry;
    tSirMacAddr zeroMacAddr = { 0, 0, 0, 0, 0, 0 };

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: limRmcIbssDelete:No active IBSS"));)
        return;
    }

    if (VOS_FALSE == vos_mem_compare(&zeroMacAddr,
                            &pMac->rmcContext.ruler, sizeof(tSirMacAddr)))
    {
        __limPostMsgUpdateInd(pMac, eRMC_RULER_CANCELLED,
                         eRMC_TRANSMITTER_ROLE, psessionEntry->selfMacAddr,
                         pMac->rmcContext.ruler);
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
             FL("RMC: limRmcIbssDelete lock acquire failed"));
        return;
    }

    /* Cancel pending timer */
    tx_timer_deactivate(&pMac->rmcContext.gRmcRulerSelectTimer);

    /* Delete all entries from Ruler database. */
    __rmcGroupDeleteAllEntries(pMac);

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: limRmcIbssDelete lock release failed"));
    }

    limLog(pMac, LOG1, FL("RMC: limRmcIbssDelete complete"));
}

void
limRmcDumpStatus(tpAniSirGlobal pMac)
{
    tLimRmcGroupContext *entry;
    int index, count;

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
             FL("RMC: limRmcDumpStatus lock acquire failed"));
        return;
    }


    limLog(pMac, LOGE, FL(" ----- RMC Transmitter Information ----- \n"));
    limLog(pMac, LOGE,
         FL("   Ruler Address   |  RMC State \n"));

    if (pMac->rmcContext.state != eRMC_RULER_NOT_SELECTED)
    {
        limLog(pMac,LOGE, FL( MAC_ADDRESS_STR " | %s\n"),
                         MAC_ADDR_ARRAY(pMac->rmcContext.ruler),
                        __limMcastTxStateToString(pMac->rmcContext.state));
    }

    limLog( pMac,LOGE, FL(" ----- RMC Ruler Information ----- \n"));
    limLog( pMac,LOGE, FL("  Transmitter Address\n"));

    count = 0;
    for (index = 0; index < RMC_MCAST_GROUPS_HASH_SIZE; index++)
    {
        entry = pMac->rmcContext.rmcGroupRxHashTable[index];

        while (entry)
        {
            count++;
            limLog( pMac,LOGE, FL("%d. " MAC_ADDRESS_STR " \n"),
                    count, MAC_ADDR_ARRAY(entry->transmitter));
            entry = entry->next;
        }
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: limRmcDumpStatus lock release failed"));
    }

    return;

}

VOS_STATUS
limRmcTriggerRulerSelection(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    if ((TRUE == pMac->rmcContext.rmcEnabled) &&
        (eRMC_RULER_NOT_SELECTED == pMac->rmcContext.state))
    {
        limLog(pMac, LOG1,
          FL("Ruler selection trigerred in FW"));

        __limPostMsgRulerReq(pMac, eRMC_SUGGEST_RULER_CMD, macAddr);

        pMac->rmcContext.state = eRMC_RULER_ENABLE_REQUESTED;

        return VOS_STATUS_SUCCESS;
    }
    else
    {
        limLog(pMac, LOG1,
          FL("Could not trigger ruler selection: RMC state %d rmcEnabled %d"),
          pMac->rmcContext.state, pMac->rmcContext.rmcEnabled);

        return VOS_STATUS_E_FAILURE;
    }
}

#endif /* WLAN_FEATURE_RMC */
