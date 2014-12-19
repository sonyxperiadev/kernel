/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

/*===========================================================================

                      s a p F s m . C

  OVERVIEW:

  This software unit holds the implementation of the WLAN SAP Finite
  State Machine modules

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2010 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.



  when        who     what, where, why
----------    ---    --------------------------------------------------------
2010-03-15         Created module

===========================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "sapInternal.h"
// Pick up the SME API definitions
#include "sme_Api.h"
// Pick up the PMC API definitions
#include "pmcApi.h"
#include "wlan_nv.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  External declarations for global context
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
#ifdef SOFTAP_CHANNEL_RANGE
static VOS_STATUS sapGetChannelList(ptSapContext sapContext, v_U8_t **channelList,
                                 v_U8_t  *numberOfChannels);
#endif
/*----------------------------------------------------------------------------
 * Externalized Function Definitions
* -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/*==========================================================================
  FUNCTION    sapEventInit

  DESCRIPTION
    Function for initializing sWLAN_SAPEvent structure

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapEvent    : State machine event

  RETURN VALUE

    None

  SIDE EFFECTS
============================================================================*/
static inline void sapEventInit(ptWLAN_SAPEvent sapEvent)
{
   sapEvent->event = eSAP_MAC_SCAN_COMPLETE;
   sapEvent->params = 0;
   sapEvent->u1 = 0;
   sapEvent->u2 = 0;
}

/*==========================================================================
  FUNCTION    sapGotoChannelSel

  DESCRIPTION
    Function for initiating scan request for SME

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapGotoChannelSel
(
    ptSapContext sapContext,
    ptWLAN_SAPEvent sapEvent
)
{
    /* Initiate a SCAN request */
    eHalStatus halStatus;
    tCsrScanRequest scanRequest;/* To be initialised if scan is required */
    v_U32_t    scanRequestID = 0;
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

#ifdef SOFTAP_CHANNEL_RANGE
    v_U8_t     *channelList = NULL;
    v_U8_t     numOfChannels = 0 ;
#endif
    tHalHandle hHal;
    tANI_U8   channel;

    hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_SME, sapContext->pvosGCtx);
    if (NULL == hHal)
    {
        /* we have a serious problem */
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_FATAL,
                   "In %s, invalid hHal", __func__);
        return VOS_STATUS_E_FAULT;
    }

    /*If STA-AP concurrency is enabled take the concurrent connected channel first. In other cases wpa_supplicant should take care */
    if (vos_get_concurrency_mode() == VOS_STA_SAP)
    {
        channel = sme_GetConcurrentOperationChannel(hHal);

        if (channel)
        { /*if a valid channel is returned then use concurrent channel.
                  Else take whatever comes from configuartion*/
            sapContext->channel = channel;
        }
    }

    if (sapContext->channel == AUTO_CHANNEL_SELECT)
    {
        vos_mem_zero(&scanRequest, sizeof(scanRequest));

        /* Set scanType to Passive scan */
        scanRequest.scanType = eSIR_PASSIVE_SCAN;

        /* Set min and max channel time to zero */
        scanRequest.minChnTime = 0;
        scanRequest.maxChnTime = 0;

        /* Set BSSType to default type */
        scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

#ifndef SOFTAP_CHANNEL_RANGE
        /*Scan all the channels */
        scanRequest.ChannelInfo.numOfChannels = 0;

        scanRequest.ChannelInfo.ChannelList = NULL;

        scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;//eCSR_SCAN_REQUEST_11D_SCAN;

#else

        sapGetChannelList(sapContext, &channelList, &numOfChannels);

        /*Scan the channels in the list*/
        scanRequest.ChannelInfo.numOfChannels = numOfChannels;

        scanRequest.ChannelInfo.ChannelList = channelList;

        scanRequest.requestType = eCSR_SCAN_SOFTAP_CHANNEL_RANGE;

        sapContext->channelList = channelList;

#endif
        /* Set requestType to Full scan */

        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, calling sme_ScanRequest", __func__);

        halStatus = sme_ScanRequest(hHal,
                            0,//Not used in csrScanRequest
                            &scanRequest,
                            &scanRequestID,//, when ID == 0 11D scan/active scan with callback, min-maxChntime set in csrScanRequest()?
                            &WLANSAP_ScanCallback,//csrScanCompleteCallback callback,
                            sapContext);//void * pContext scanRequestID filled up
        if (eHAL_STATUS_SUCCESS != halStatus)
        {
            VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "%s:sme_ScanRequest  fail %d!!!", __func__, halStatus);
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "SoftAP Configuring for default channel, Ch= %d", sapContext->channel);
            /* In case of error, switch to default channel */
            sapContext->channel = SAP_DEFAULT_CHANNEL;

#ifdef SOFTAP_CHANNEL_RANGE
            if(sapContext->channelList != NULL)
            {
                sapContext->channel = sapContext->channelList[0];
                vos_mem_free(sapContext->channelList);
                sapContext->channelList = NULL;
            }
#endif
            /* Fill in the event structure */
            sapEventInit(sapEvent);
            /* Handle event */
            vosStatus = sapFsm(sapContext, sapEvent);
        }
        else
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, return from sme_ScanRequest, scanRequestID=%d, Ch= %d",
                   __func__, scanRequestID, sapContext->channel);
        }

    }
    else
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, for configured channel, Ch= %d", __func__, sapContext->channel);
        /* Fill in the event structure */
        // Eventhough scan was not done, means a user set channel was chosen
        sapEventInit(sapEvent);
        /* Handle event */
        vosStatus = sapFsm(sapContext, sapEvent);
    }

    /* If scan failed, get default channel and advance state machine as success with default channel */
    /* Have to wait for the call back to be called to get the channel cannot advance state machine here as said above */
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, before exiting sapGotoChannelSel channel=%d", __func__, sapContext->channel);

    return VOS_STATUS_SUCCESS;
}// sapGotoChannelSel

/*==========================================================================
  FUNCTION    sapGotoStarting

  DESCRIPTION
    Function for initiating start bss request for SME

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    bssType     : Type of bss to start, INRA AP
    status      : Return the SAP status here

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapGotoStarting
(
    ptSapContext sapContext,
    ptWLAN_SAPEvent sapEvent,
    eCsrRoamBssType bssType
)
{
    /* tHalHandle */
    tHalHandle hHal = VOS_GET_HAL_CB(sapContext->pvosGCtx);
    eHalStatus halStatus;

    /*- - - - - - - - TODO:once configs from hdd available - - - - - - - - -*/
    char key_material[32]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1,};
    sapContext->key_type = 0x05;
    sapContext->key_length = 32;
    vos_mem_copy(sapContext->key_material, key_material, sizeof(key_material));  /* Need a key size define */

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s", __func__);
    if (NULL == hHal)
    {
        /* we have a serious problem */
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_FATAL,
                   "In %s, invalid hHal", __func__);
        return VOS_STATUS_E_FAULT;
    }

    //TODO: What shall we do if failure????
    halStatus = pmcRequestFullPower( hHal,
                            WLANSAP_pmcFullPwrReqCB,
                            sapContext,
                            eSME_REASON_OTHER);

    /* Open SME Session for Softap */
    halStatus = sme_OpenSession(hHal,
                        &WLANSAP_RoamCallback,
                        sapContext,
                        sapContext->self_mac_addr,
                        &sapContext->sessionId);

    if(eHAL_STATUS_SUCCESS != halStatus )
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "Error: In %s calling sme_RoamConnect status = %d", __func__, halStatus);
        return VOS_STATUS_E_FAILURE;
    }

    return VOS_STATUS_SUCCESS;
}// sapGotoStarting

/*==========================================================================
  FUNCTION    sapGotoDisconnecting

  DESCRIPTION
    Processing of SAP FSM Disconnecting state

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    status      : Return the SAP status here

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapGotoDisconnecting
(
    ptSapContext sapContext
)
{
    eHalStatus halStatus;
    tHalHandle hHal;

    hHal = VOS_GET_HAL_CB(sapContext->pvosGCtx);
    if (NULL == hHal)
    {
        /* we have a serious problem */
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "In %s, invalid hHal", __func__);
        return VOS_STATUS_E_FAULT;
    }

    sapFreeRoamProfile(&sapContext->csrRoamProfile);
    halStatus = sme_RoamStopBss(hHal, sapContext->sessionId);
    if(eHAL_STATUS_SUCCESS != halStatus )
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "Error: In %s calling sme_RoamStopBss status = %d", __func__, halStatus);
        return VOS_STATUS_E_FAILURE;
    }

    return VOS_STATUS_SUCCESS;
}

static eHalStatus sapRoamSessionCloseCallback(void *pContext)
{
    ptSapContext sapContext = (ptSapContext)pContext;
    return sapSignalHDDevent(sapContext, NULL,
                    eSAP_STOP_BSS_EVENT, (v_PVOID_t) eSAP_STATUS_SUCCESS);
}

/*==========================================================================
  FUNCTION    sapGotoDisconnected

  DESCRIPTION
    Function for setting the SAP FSM to Disconnection state

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    status      : Return the SAP status here

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapGotoDisconnected
(
    ptSapContext sapContext
)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tWLAN_SAPEvent sapEvent;
    // Processing has to be coded
    // Clean up stations from TL etc as AP BSS is shut down then set event
    sapEvent.event = eSAP_MAC_READY_FOR_CONNECTIONS;// hardcoded
    sapEvent.params = 0;
    sapEvent.u1 = 0;
    sapEvent.u2 = 0;
    /* Handle event */
    vosStatus = sapFsm(sapContext, &sapEvent);

    return vosStatus;
}

/*==========================================================================
  FUNCTION    sapSignalHDDevent

  DESCRIPTION
    Function for HDD to send the event notification using callback

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    pCsrRoamInfo : Pointer to CSR roam information
    sapHddevent      : SAP HDD event
    context          : to pass the element for future support

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapSignalHDDevent
(
    ptSapContext sapContext, /* sapContext value */
    tCsrRoamInfo *pCsrRoamInfo,
    eSapHddEvent sapHddevent,
    void         *context
)
{
    VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
    tSap_Event sapApAppEvent; /* This now encodes ALL event types */
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /* Format the Start BSS Complete event to return... */
    VOS_ASSERT(sapContext->pfnSapEventCallback);

    switch (sapHddevent)
    {
        case eSAP_STA_ASSOC_IND:
            //  TODO - Indicate the assoc request indication to OS
            sapApAppEvent.sapHddEventCode = eSAP_STA_ASSOC_IND;

            vos_mem_copy( &sapApAppEvent.sapevt.sapAssocIndication.staMac, pCsrRoamInfo->peerMac,sizeof(tSirMacAddr));
            sapApAppEvent.sapevt.sapAssocIndication.staId = pCsrRoamInfo->staId;
            sapApAppEvent.sapevt.sapAssocIndication.status = 0;
            // Required for indicating the frames to upper layer
            sapApAppEvent.sapevt.sapAssocIndication.beaconLength = pCsrRoamInfo->beaconLength;
            sapApAppEvent.sapevt.sapAssocIndication.beaconPtr = pCsrRoamInfo->beaconPtr;
            sapApAppEvent.sapevt.sapAssocIndication.assocReqLength = pCsrRoamInfo->assocReqLength;
            sapApAppEvent.sapevt.sapAssocIndication.assocReqPtr = pCsrRoamInfo->assocReqPtr;
            sapApAppEvent.sapevt.sapAssocIndication.fWmmEnabled = pCsrRoamInfo->wmmEnabledSta;
            if ( pCsrRoamInfo->u.pConnectedProfile != NULL )
            {
               sapApAppEvent.sapevt.sapAssocIndication.negotiatedAuthType = pCsrRoamInfo->u.pConnectedProfile->AuthType;
               sapApAppEvent.sapevt.sapAssocIndication.negotiatedUCEncryptionType = pCsrRoamInfo->u.pConnectedProfile->EncryptionType;
               sapApAppEvent.sapevt.sapAssocIndication.negotiatedMCEncryptionType = pCsrRoamInfo->u.pConnectedProfile->mcEncryptionType;
               sapApAppEvent.sapevt.sapAssocIndication.fAuthRequired = pCsrRoamInfo->fAuthRequired;
            }
            break;
       case eSAP_START_BSS_EVENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                __func__, "eSAP_START_BSS_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_START_BSS_EVENT;
            sapApAppEvent.sapevt.sapStartBssCompleteEvent.status = (eSapStatus )context;
            if(pCsrRoamInfo != NULL ){
                sapApAppEvent.sapevt.sapStartBssCompleteEvent.staId = pCsrRoamInfo->staId;
            }
            else
                sapApAppEvent.sapevt.sapStartBssCompleteEvent.staId = 0;
            sapApAppEvent.sapevt.sapStartBssCompleteEvent.operatingChannel = (v_U8_t)sapContext->channel;
            break;

        case eSAP_STOP_BSS_EVENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                       __func__, "eSAP_STOP_BSS_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_STOP_BSS_EVENT;
            sapApAppEvent.sapevt.sapStopBssCompleteEvent.status = (eSapStatus )context;
            break;

        case eSAP_STA_ASSOC_EVENT:
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                __func__, "eSAP_STA_ASSOC_EVENT");
            if (pCsrRoamInfo->fReassocReq)
                sapApAppEvent.sapHddEventCode = eSAP_STA_REASSOC_EVENT;
            else
                sapApAppEvent.sapHddEventCode = eSAP_STA_ASSOC_EVENT;

            //TODO: Need to fill the SET KEY information and pass to HDD
            vos_mem_copy( &sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.staMac,
                         pCsrRoamInfo->peerMac,sizeof(tSirMacAddr));
            sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.staId = pCsrRoamInfo->staId ;
            sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.statusCode = pCsrRoamInfo->statusCode;
            sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.iesLen = pCsrRoamInfo->rsnIELen;
            vos_mem_copy(sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.ies, pCsrRoamInfo->prsnIE,
                        pCsrRoamInfo->rsnIELen);

            if(pCsrRoamInfo->addIELen)
            {
                v_U8_t  len = sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.iesLen;
                sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.iesLen
                                                        += pCsrRoamInfo->addIELen;
                vos_mem_copy(&sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.ies[len], pCsrRoamInfo->paddIE,
                            pCsrRoamInfo->addIELen);
            }

            sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.wmmEnabled = pCsrRoamInfo->wmmEnabledSta;
            sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.status = (eSapStatus )context;
            //TODO: Need to fill sapAuthType
            //sapApAppEvent.sapevt.sapStationAssocReassocCompleteEvent.SapAuthType = pCsrRoamInfo->pProfile->negotiatedAuthType;
            break;
        }

        case eSAP_STA_DISASSOC_EVENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                       __func__, "eSAP_STA_DISASSOC_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_STA_DISASSOC_EVENT;

            vos_mem_copy( &sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.staMac,
                          pCsrRoamInfo->peerMac, sizeof(tSirMacAddr));
            sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.staId = pCsrRoamInfo->staId;
            if (pCsrRoamInfo->reasonCode == eCSR_ROAM_RESULT_FORCED)
                sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.reason = eSAP_USR_INITATED_DISASSOC;
            else
                sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.reason = eSAP_MAC_INITATED_DISASSOC;

            sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.statusCode = pCsrRoamInfo->statusCode;
            sapApAppEvent.sapevt.sapStationDisassocCompleteEvent.status = (eSapStatus )context;
            break;

        case eSAP_STA_SET_KEY_EVENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                       __func__, "eSAP_STA_SET_KEY_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_STA_SET_KEY_EVENT;
            sapApAppEvent.sapevt.sapStationSetKeyCompleteEvent.status = (eSapStatus )context;
            vos_mem_copy(&sapApAppEvent.sapevt.sapStationSetKeyCompleteEvent.peerMacAddr,
                         pCsrRoamInfo->peerMac,sizeof(tSirMacAddr));
            break;

        case eSAP_STA_DEL_KEY_EVENT :
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                       __func__, "eSAP_STA_DEL_KEY_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_STA_DEL_KEY_EVENT;
            sapApAppEvent.sapevt.sapStationDeleteKeyCompleteEvent.status = (eSapStatus )context;
            //TODO: Should we need to send the key information
            //sapApAppEvent.sapevt.sapStationDeleteKeyCompleteEvent.keyId = ;
            break;

        case eSAP_STA_MIC_FAILURE_EVENT :
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                        __func__, "eSAP_STA_MIC_FAILURE_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_STA_MIC_FAILURE_EVENT;
            vos_mem_copy( &sapApAppEvent.sapevt.sapStationMICFailureEvent.srcMacAddr,
                          pCsrRoamInfo->u.pMICFailureInfo->srcMacAddr,
                          sizeof(tSirMacAddr));
            vos_mem_copy( &sapApAppEvent.sapevt.sapStationMICFailureEvent.staMac,
                          pCsrRoamInfo->u.pMICFailureInfo->taMacAddr,
                          sizeof(tSirMacAddr));
            vos_mem_copy( &sapApAppEvent.sapevt.sapStationMICFailureEvent.dstMacAddr,
                          pCsrRoamInfo->u.pMICFailureInfo->dstMacAddr,
                          sizeof(tSirMacAddr));
            sapApAppEvent.sapevt.sapStationMICFailureEvent.multicast = pCsrRoamInfo->u.pMICFailureInfo->multicast;
            sapApAppEvent.sapevt.sapStationMICFailureEvent.IV1 = pCsrRoamInfo->u.pMICFailureInfo->IV1;
            sapApAppEvent.sapevt.sapStationMICFailureEvent.keyId = pCsrRoamInfo->u.pMICFailureInfo->keyId;
            vos_mem_copy( sapApAppEvent.sapevt.sapStationMICFailureEvent.TSC,
                          pCsrRoamInfo->u.pMICFailureInfo->TSC,
                          SIR_CIPHER_SEQ_CTR_SIZE);
            break;

        case eSAP_ASSOC_STA_CALLBACK_EVENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, SAP event callback event = %s",
                       __func__, "eSAP_ASSOC_STA_CALLBACK_EVENT");
            break;

        case eSAP_WPS_PBC_PROBE_REQ_EVENT:
            sapApAppEvent.sapHddEventCode = eSAP_WPS_PBC_PROBE_REQ_EVENT;

            vos_mem_copy( &sapApAppEvent.sapevt.sapPBCProbeReqEvent.WPSPBCProbeReq,
                          pCsrRoamInfo->u.pWPSPBCProbeReq,
                          sizeof(tSirWPSPBCProbeReq));
            break;

       case eSAP_INDICATE_MGMT_FRAME:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                                 "In %s, SAP event callback event = %s",
                                __func__, "eSAP_INDICATE_MGMT_FRAME");
            sapApAppEvent.sapHddEventCode = eSAP_INDICATE_MGMT_FRAME;
            sapApAppEvent.sapevt.sapManagementFrameInfo.nFrameLength
                                           = pCsrRoamInfo->nFrameLength;
            sapApAppEvent.sapevt.sapManagementFrameInfo.pbFrames
                                           = pCsrRoamInfo->pbFrames;
            sapApAppEvent.sapevt.sapManagementFrameInfo.frameType
                                           = pCsrRoamInfo->frameType;
            sapApAppEvent.sapevt.sapManagementFrameInfo.rxChan
                                           = pCsrRoamInfo->rxChan;

            break;
       case eSAP_REMAIN_CHAN_READY:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                                 "In %s, SAP event callback event = %s",
                                __func__, "eSAP_REMAIN_CHAN_READY");
           sapApAppEvent.sapHddEventCode = eSAP_REMAIN_CHAN_READY;
            break;
       case eSAP_SEND_ACTION_CNF:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                                 "In %s, SAP event callback event = %s",
                                __func__, "eSAP_SEND_ACTION_CNF");
            sapApAppEvent.sapHddEventCode = eSAP_SEND_ACTION_CNF;
            sapApAppEvent.sapevt.sapActionCnf.actionSendSuccess = (eSapStatus)context;
            break;

       case eSAP_DISCONNECT_ALL_P2P_CLIENT:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                             "In %s, SAP event callback event = %s",
                            __func__, "eSAP_DISCONNECT_ALL_P2P_CLIENT");
            sapApAppEvent.sapHddEventCode = eSAP_DISCONNECT_ALL_P2P_CLIENT;
            sapApAppEvent.sapevt.sapActionCnf.actionSendSuccess = (eSapStatus)context;
            break;

       case eSAP_MAC_TRIG_STOP_BSS_EVENT :
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                             "In %s, SAP event callback event = %s",
                            __func__, "eSAP_MAC_TRIG_STOP_BSS_EVENT");
            sapApAppEvent.sapHddEventCode = eSAP_MAC_TRIG_STOP_BSS_EVENT;
            sapApAppEvent.sapevt.sapActionCnf.actionSendSuccess = (eSapStatus)context;
            break;


        case eSAP_UNKNOWN_STA_JOIN:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                       "In %s, SAP event callback event = %s",
                       __func__, "eSAP_UNKNOWN_STA_JOIN");
            sapApAppEvent.sapHddEventCode = eSAP_UNKNOWN_STA_JOIN;
            vos_mem_copy((v_PVOID_t)sapApAppEvent.sapevt.sapUnknownSTAJoin.macaddr.bytes,
                         (v_PVOID_t)context, sizeof(v_MACADDR_t));
            break;

        case eSAP_MAX_ASSOC_EXCEEDED:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                    "In %s, SAP event callback event = %s",
                    __func__, "eSAP_MAX_ASSOC_EXCEEDED");
            sapApAppEvent.sapHddEventCode = eSAP_MAX_ASSOC_EXCEEDED;
            vos_mem_copy((v_PVOID_t)sapApAppEvent.sapevt.sapMaxAssocExceeded.macaddr.bytes,
                    (v_PVOID_t)pCsrRoamInfo->peerMac, sizeof(v_MACADDR_t));
            break;

        default:
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, SAP Unknown callback event = %d",
                       __func__,sapHddevent);
            break;
    }
    vosStatus = (*sapContext->pfnSapEventCallback)
                (
                 &sapApAppEvent,
                 sapContext->pUsrContext//userdataforcallback - hdd opaque handle
                 );

    return vosStatus;

} /* sapSignalApAppStartBssEvent */

/*==========================================================================
  FUNCTION    sapFsm

  DESCRIPTION
    SAP State machine entry function

  DEPENDENCIES
    NA.

  PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    status      : Return the SAP status here

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
sapFsm
(
    ptSapContext sapContext,    /* sapContext value */
    ptWLAN_SAPEvent sapEvent   /* State machine event */
)
{
   /* Retrieve the phy link state machine structure
     * from the sapContext value
     */
    eSapFsmStates_t stateVar = sapContext->sapsMachine; /*state var that keeps track of state machine*/
    tCsrRoamInfo    *roamInfo = (tCsrRoamInfo *)(sapEvent->params);
    v_U32_t msg = sapEvent->event;  /* State machine input event message */
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

    switch (stateVar)
    {
        case eSAP_DISCONNECTED:
            if ((msg == eSAP_HDD_START_INFRA_BSS))
            {
                /* Transition from eSAP_DISCONNECTED to eSAP_CH_SELECT (both without substates) */
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, new from state %s => %s",
                            __func__, "eSAP_DISCONNECTED", "eSAP_CH_SELECT");

                /* There can be one SAP Session for softap */
                if (sapContext->isSapSessionOpen == eSAP_TRUE)
                {
                   VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_FATAL,
                        "%s:SME Session is already opened\n",__func__);
                   return VOS_STATUS_E_EXISTS;
                }

                sapContext->sessionId = 0xff;

                /* Set SAP device role */
                sapContext->sapsMachine = eSAP_CH_SELECT;

                /* Perform sme_ScanRequest */
                vosStatus = sapGotoChannelSel(sapContext, sapEvent);

                /* Transition from eSAP_DISCONNECTED to eSAP_CH_SELECT (both without substates) */
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                           __func__, "eSAP_DISCONNECTED", "eSAP_CH_SELECT");
            }
            else
            {
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, in state %s, event msg %d",
                             __func__, "eSAP_DISCONNECTED", msg);
            }
            break;

        case eSAP_CH_SELECT:
            if (msg == eSAP_MAC_SCAN_COMPLETE)
            {
                 /* Transition from eSAP_CH_SELECT to eSAP_STARTING (both without substates) */
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                            __func__, "eSAP_CH_SELECT", "eSAP_STARTING");
                 // Channel selected. Now can sapGotoStarting
                 sapContext->sapsMachine = eSAP_STARTING;
                 // Specify the channel
                 sapContext->csrRoamProfile.ChannelInfo.numOfChannels = 1;
                 sapContext->csrRoamProfile.ChannelInfo.ChannelList = &sapContext->csrRoamProfile.operationChannel;
                 sapContext->csrRoamProfile.operationChannel = (tANI_U8)sapContext->channel;
                 vosStatus = sapGotoStarting( sapContext, sapEvent, eCSR_BSS_TYPE_INFRA_AP);
                 /* Transition from eSAP_CH_SELECT to eSAP_STARTING (both without substates) */
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                             __func__, "eSAP_CH_SELECT", "eSAP_STARTING");
            }
            else
            {
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, in state %s, invalid event msg %d",
                            __func__, "eSAP_CH_SELECT", msg);
            }
            break;

        case eSAP_STARTING:
            if (msg == eSAP_MAC_START_BSS_SUCCESS )
            {
                /* Transition from eSAP_STARTING to eSAP_STARTED (both without substates) */
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state channel = %d %s => %s",
                            __func__,sapContext->channel, "eSAP_STARTING", "eSAP_STARTED");

                 sapContext->sapsMachine = eSAP_STARTED;
                 /*Action code for transition */
                 vosStatus = sapSignalHDDevent( sapContext, roamInfo, eSAP_START_BSS_EVENT, (v_PVOID_t)eSAP_STATUS_SUCCESS);

                 /* Transition from eSAP_STARTING to eSAP_STARTED (both without substates) */
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                            __func__, "eSAP_STARTING", "eSAP_STARTED");
             }
             else if (msg == eSAP_MAC_START_FAILS)
             {
                 /*Transition from STARTING to DISCONNECTED (both without substates)*/
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                            __func__, "eSAP_STARTING", "eSAP_DISCONNECTED");

                 /*Action code for transition */
                 vosStatus = sapSignalHDDevent( sapContext, NULL, eSAP_START_BSS_EVENT,(v_PVOID_t) eSAP_STATUS_FAILURE);
                 vosStatus =  sapGotoDisconnected(sapContext);

                 /*Advance outer statevar */
                 sapContext->sapsMachine = eSAP_DISCONNECTED;
             }
             else if (msg == eSAP_HDD_STOP_INFRA_BSS)
             {
                 /*Transition from eSAP_STARTING to eSAP_DISCONNECTING (both without substates)*/
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                             __func__, "eSAP_STARTING", "eSAP_DISCONNECTING");

                 /*Advance outer statevar */
                 sapContext->sapsMachine = eSAP_DISCONNECTED;
                 vosStatus = sapSignalHDDevent( sapContext, NULL, eSAP_START_BSS_EVENT, (v_PVOID_t)eSAP_STATUS_FAILURE);
                 vosStatus = sapGotoDisconnected(sapContext);
                 /* Close the SME session*/

                 if (eSAP_TRUE == sapContext->isSapSessionOpen)
                 {
                    tHalHandle hHal = VOS_GET_HAL_CB(sapContext->pvosGCtx);
                    if (NULL == hHal)
                    {
                       VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                                  "In %s, NULL hHal in state %s, msg %d",
                                  __func__, "eSAP_STARTING", msg);
                    }
                    else if (eHAL_STATUS_SUCCESS ==
                         sme_CloseSession(hHal,
                                         sapContext->sessionId, NULL, NULL))
                     {
                         sapContext->isSapSessionOpen = eSAP_FALSE;
                     }
                 }
             }
             else
             {
                 VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                             "In %s, in state %s, invalid event msg %d",
                             __func__, "eSAP_STARTING", msg);
                 /* Intentionally left blank */
             }
             break;

        case eSAP_STARTED:
            if (msg == eSAP_HDD_STOP_INFRA_BSS)
            {
                /* Transition from eSAP_STARTED to eSAP_DISCONNECTING (both without substates) */
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                           __func__, "eSAP_STARTED", "eSAP_DISCONNECTING");
                sapContext->sapsMachine = eSAP_DISCONNECTING;
                vosStatus = sapGotoDisconnecting(sapContext);
            }
            else
            {
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, in state %s, invalid event msg %d",
                           __func__, "eSAP_STARTED", msg);
            }
            break;

        case eSAP_DISCONNECTING:
            if (msg == eSAP_MAC_READY_FOR_CONNECTIONS)
            {
                /* Transition from eSAP_DISCONNECTING to eSAP_DISCONNECTED (both without substates) */
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, from state %s => %s",
                          __func__, "eSAP_DISCONNECTING", "eSAP_DISCONNECTED");

                sapContext->sapsMachine = eSAP_DISCONNECTED;
                /* Close the SME session*/
                if (eSAP_TRUE == sapContext->isSapSessionOpen)
                {
                    tHalHandle hHal = VOS_GET_HAL_CB(sapContext->pvosGCtx);

                    if (NULL == hHal)
                    {
                        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                                   "In %s, NULL hHal in state %s, msg %d",
                                   __func__, "eSAP_DISCONNECTING", msg);
                    }
                    else
                    {
                        sapContext->isSapSessionOpen = eSAP_FALSE;
                        if (!HAL_STATUS_SUCCESS(
                            sme_CloseSession(hHal,
                                     sapContext->sessionId,
                                     sapRoamSessionCloseCallback, sapContext)))
                        {
                            vosStatus = sapSignalHDDevent(sapContext, NULL,
                                              eSAP_STOP_BSS_EVENT,
                                              (v_PVOID_t) eSAP_STATUS_SUCCESS);
                        }
                    }
                }
            }
            else
            {
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                           "In %s, in state %s, invalid event msg %d",
                          __func__, "eSAP_DISCONNECTING", msg);
            }
            break;
      }
      return vosStatus;
}// sapFsm


eSapStatus
sapconvertToCsrProfile(tsap_Config_t *pconfig_params, eCsrRoamBssType bssType, tCsrRoamProfile *profile)
{
    //Create Roam profile for SoftAP to connect
    profile->BSSType = eCSR_BSS_TYPE_INFRA_AP;
    profile->SSIDs.numOfSSIDs = 1;
    profile->csrPersona = pconfig_params->persona;

    vos_mem_zero(profile->SSIDs.SSIDList[0].SSID.ssId,
                 sizeof(profile->SSIDs.SSIDList[0].SSID.ssId));

    //Flag to not broadcast the SSID information
    profile->SSIDs.SSIDList[0].ssidHidden =  pconfig_params->SSIDinfo.ssidHidden;

    profile->SSIDs.SSIDList[0].SSID.length = pconfig_params->SSIDinfo.ssid.length;
    vos_mem_copy(&profile->SSIDs.SSIDList[0].SSID.ssId, pconfig_params->SSIDinfo.ssid.ssId,
                  sizeof(pconfig_params->SSIDinfo.ssid.ssId));

    profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

    if (pconfig_params->authType == eSAP_OPEN_SYSTEM)
    {
        profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
    }
    else if (pconfig_params->authType == eSAP_SHARED_KEY)
    {
        profile->negotiatedAuthType = eCSR_AUTH_TYPE_SHARED_KEY;
    }
    else
    {
        profile->negotiatedAuthType = eCSR_AUTH_TYPE_AUTOSWITCH;
    }

    profile->AuthType.numEntries = 1;
    profile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;

    //Always set the Encryption Type
    profile->EncryptionType.numEntries = 1;
    profile->EncryptionType.encryptionType[0] = pconfig_params->RSNEncryptType;

    profile->mcEncryptionType.numEntries = 1;
    profile->mcEncryptionType.encryptionType[0] = pconfig_params->mcRSNEncryptType;

    if (pconfig_params->privacy & eSAP_SHARED_KEY)
    {
        profile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;
    }

    profile->privacy = pconfig_params->privacy;
    profile->fwdWPSPBCProbeReq = pconfig_params->fwdWPSPBCProbeReq;

    if (pconfig_params->authType == eSAP_SHARED_KEY)
    {
        profile->csr80211AuthType = eSIR_SHARED_KEY;
    }
    else if (pconfig_params->authType == eSAP_OPEN_SYSTEM)
    {
        profile->csr80211AuthType = eSIR_OPEN_SYSTEM;
    }
    else
    {
        profile->csr80211AuthType = eSIR_AUTO_SWITCH;
    }

    //Initialize we are not going to use it
    profile->pWPAReqIE = NULL;
    profile->nWPAReqIELength = 0;

    //set the RSN/WPA IE
    profile->pRSNReqIE = NULL;
    profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
    if (pconfig_params->RSNWPAReqIELength)
    {
        profile->pRSNReqIE = vos_mem_malloc(pconfig_params->RSNWPAReqIELength);
        if( NULL == profile->pRSNReqIE )
        {
           VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, " %s Fail to alloc memory", __func__);
           return eSAP_STATUS_FAILURE;
        }
        vos_mem_copy(profile->pRSNReqIE, pconfig_params->pRSNWPAReqIE, pconfig_params->RSNWPAReqIELength);
        profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
    }

    // Turn off CB mode
    profile->CBMode = eCSR_CB_OFF;

    //set the phyMode to accept anything
    //Best means everything because it covers all the things we support
    profile->phyMode = pconfig_params->SapHw_mode; /*eCSR_DOT11_MODE_BEST*/

    //Configure beaconInterval
    profile->beaconInterval = (tANI_U16)pconfig_params->beacon_int;

    // set DTIM period
    profile->dtimPeriod = pconfig_params->dtim_period;

    //set Uapsd enable bit
    profile->ApUapsdEnable = pconfig_params->UapsdEnable;

    //Enable protection parameters
    profile->protEnabled       = pconfig_params->protEnabled;
    profile->obssProtEnabled   = pconfig_params->obssProtEnabled;
    profile->cfg_protection    = pconfig_params->ht_capab;

    //country code
    if (pconfig_params->countryCode[0])
        vos_mem_copy(profile->countryCode, pconfig_params->countryCode, WNI_CFG_COUNTRY_CODE_LEN);
    profile->ieee80211d = pconfig_params->ieee80211d;
    //wps config info
    profile->wps_state = pconfig_params->wps_state;

    return eSAP_STATUS_SUCCESS; /* Success.  */
}

/**
 * FUNCTION: sapConvertSapPhyModeToCsrPhyMode
 * Called internally by SAP
 */
eCsrPhyMode sapConvertSapPhyModeToCsrPhyMode( eSapPhyMode sapPhyMode )
{
    switch (sapPhyMode)
    {
      case (eSAP_DOT11_MODE_abg):
         return eCSR_DOT11_MODE_abg;
      case (eSAP_DOT11_MODE_11b):
         return eCSR_DOT11_MODE_11b;
      case (eSAP_DOT11_MODE_11g):
         return eCSR_DOT11_MODE_11g;
      case (eSAP_DOT11_MODE_11n):
         return eCSR_DOT11_MODE_11n;
      case (eSAP_DOT11_MODE_11g_ONLY):
         return eCSR_DOT11_MODE_11g_ONLY;
      case (eSAP_DOT11_MODE_11n_ONLY):
         return eCSR_DOT11_MODE_11n_ONLY;
      case (eSAP_DOT11_MODE_11b_ONLY):
         return eCSR_DOT11_MODE_11b_ONLY;
#ifdef WLAN_FEATURE_11AC
      case (eSAP_DOT11_MODE_11ac_ONLY):
         return eCSR_DOT11_MODE_11ac_ONLY;
      case (eSAP_DOT11_MODE_11ac):
         return eCSR_DOT11_MODE_11ac;
#endif
      default:
         return eCSR_DOT11_MODE_AUTO;
    }
}

void sapFreeRoamProfile(tCsrRoamProfile *profile)
{
   if(profile->pRSNReqIE)
   {
      vos_mem_free(profile->pRSNReqIE);
      profile->pRSNReqIE = NULL;
   }
}


void
sapSortMacList(v_MACADDR_t *macList, v_U8_t size)
{
    v_U8_t outer, inner;
    v_MACADDR_t temp;
    v_SINT_t nRes = -1;

    for(outer = 0; outer < size; outer++)
    {
        for(inner = 0; inner < size - 1; inner++)
        {
            nRes = vos_mem_compare2((macList + inner)->bytes, (macList + inner + 1)->bytes, sizeof(v_MACADDR_t));
            if (nRes > 0)
            {
                vos_mem_copy(temp.bytes, (macList + inner + 1)->bytes, sizeof(v_MACADDR_t));
                vos_mem_copy((macList + inner + 1)->bytes, (macList + inner)->bytes, sizeof(v_MACADDR_t));
                vos_mem_copy((macList + inner)->bytes, temp.bytes, sizeof(v_MACADDR_t));
             }
        }
    }
}

eSapBool
sapSearchMacList(v_MACADDR_t *macList, v_U8_t num_mac, v_U8_t *peerMac, v_U8_t *index)
{
    v_SINT_t nRes = -1;
    v_S7_t nStart = 0, nEnd, nMiddle;
    nEnd = num_mac - 1;

    while (nStart <= nEnd)
    {
        nMiddle = (nStart + nEnd) / 2;
        nRes = vos_mem_compare2(&macList[nMiddle], peerMac, sizeof(v_MACADDR_t));

        if (0 == nRes)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                    "search SUCC");
            // "index equals NULL" means the caller does not need the
            // index value of the peerMac being searched
            if (index != NULL)
            {
                *index = (v_U8_t) nMiddle;
                VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                        "index %d", *index);
            }
            return eSAP_TRUE;
        }
        if (nRes < 0)
            nStart = nMiddle + 1;
        else
            nEnd = nMiddle - 1;
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
            "search not succ");
    return eSAP_FALSE;
}

void
sapAddMacToACL(v_MACADDR_t *macList, v_U8_t *size, v_U8_t *peerMac)
{
    v_SINT_t nRes = -1;
    int i;
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,"add acl entered");
    for (i=((*size)-1); i>=0; i--)
    {
        nRes = vos_mem_compare2(&macList[i], peerMac, sizeof(v_MACADDR_t));
        if (nRes > 0)
        {
            /* Move alphabetically greater mac addresses one index down to allow for insertion
               of new mac in sorted order */
            vos_mem_copy((macList+i+1)->bytes,(macList+i)->bytes, sizeof(v_MACADDR_t));
        }
        else
        {
            break;
        }
    }
    //This should also take care of if the element is the first to be added in the list
    vos_mem_copy((macList+i+1)->bytes, peerMac, sizeof(v_MACADDR_t));
    // increment the list size
    (*size)++;
}

void
sapRemoveMacFromACL(v_MACADDR_t *macList, v_U8_t *size, v_U8_t index)
{
    int i;
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,"remove acl entered");
    /* return if the list passed is empty. Ideally this should never happen since this funcn is always
       called after sapSearchMacList to get the index of the mac addr to be removed and this will
       only get called if the search is successful. Still no harm in having the check */
    if (macList==NULL) return;
    for (i=index; i<((*size)-1); i++)
    {
        /* Move mac addresses starting from "index" passed one index up to delete the void
           created by deletion of a mac address in ACL */
        vos_mem_copy((macList+i)->bytes,(macList+i+1)->bytes, sizeof(v_MACADDR_t));
    }
    // The last space should be made empty since all mac addesses moved one step up
    vos_mem_zero((macList+(*size)-1)->bytes, sizeof(v_MACADDR_t));
    //reduce the list size by 1
    (*size)--;
}

void sapPrintACL(v_MACADDR_t *macList, v_U8_t size)
{
    int i;
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,"print acl entered");
    if (size==0) return;
    for (i=0; i<size; i++)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                "** ACL entry %i - %02x:%02x:%02x:%02x:%02x:%02x", i,
                (macList+i)->bytes[0], (macList+i)->bytes[1], (macList+i)->bytes[2],
                (macList+i)->bytes[3], (macList+i)->bytes[4], (macList+i)->bytes[5]);
    }
    return;
}

VOS_STATUS
sapIsPeerMacAllowed(ptSapContext sapContext, v_U8_t *peerMac)
{
    if (eSAP_ALLOW_ALL == sapContext->eSapMacAddrAclMode)
              return VOS_STATUS_SUCCESS;

    if (sapSearchMacList(sapContext->acceptMacList, sapContext->nAcceptMac, peerMac, NULL))
        return VOS_STATUS_SUCCESS;

    if (sapSearchMacList(sapContext->denyMacList, sapContext->nDenyMac, peerMac, NULL))
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Peer %02x:%02x:%02x:%02x:%02x:%02x in deny list",
                __func__, *peerMac, *(peerMac + 1), *(peerMac + 2), *(peerMac + 3), *(peerMac + 4), *(peerMac + 5));
        return VOS_STATUS_E_FAILURE;
    }

    // A new station CAN associate, unless in deny list. Less stringent mode
    if (eSAP_ACCEPT_UNLESS_DENIED == sapContext->eSapMacAddrAclMode)
        return VOS_STATUS_SUCCESS;

    // A new station CANNOT associate, unless in accept list. More stringent mode
    if (eSAP_DENY_UNLESS_ACCEPTED == sapContext->eSapMacAddrAclMode)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Peer %02x:%02x:%02x:%02x:%02x:%02x denied, Mac filter mode is eSAP_DENY_UNLESS_ACCEPTED",
                __func__,  *peerMac, *(peerMac + 1), *(peerMac + 2), *(peerMac + 3), *(peerMac + 4), *(peerMac + 5));
        return VOS_STATUS_E_FAILURE;
    }

    /* The new STA is neither in accept list nor in deny list. In this case, deny the association
     * but send a wifi event notification indicating the mac address being denied
     */
    if (eSAP_SUPPORT_ACCEPT_AND_DENY == sapContext->eSapMacAddrAclMode)
    {
        sapSignalHDDevent(sapContext, NULL, eSAP_UNKNOWN_STA_JOIN, (v_PVOID_t)peerMac);
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Peer %02x:%02x:%02x:%02x:%02x:%02x denied, Mac filter mode is eSAP_SUPPORT_ACCEPT_AND_DENY",
                __func__,  *peerMac, *(peerMac + 1), *(peerMac + 2), *(peerMac + 3), *(peerMac + 4), *(peerMac + 5));
        return VOS_STATUS_E_FAILURE;
    }
    return VOS_STATUS_SUCCESS;
}

#ifdef SOFTAP_CHANNEL_RANGE
static VOS_STATUS sapGetChannelList(ptSapContext sapContext,
                                 v_U8_t **channelList, v_U8_t *numberOfChannels)
{
    v_U32_t startChannelNum;
    v_U32_t endChannelNum;
    v_U32_t operatingBand;
    v_U8_t  loopCount;
    v_U8_t *list;
    v_U8_t channelCount;
    v_U8_t bandStartChannel;
    v_U8_t bandEndChannel ;
    v_U32_t enableLTECoex;
    tHalHandle hHal = VOS_GET_HAL_CB(sapContext->pvosGCtx);

    if (NULL == hHal)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
             "Invalid HAL pointer from pvosGCtx on sapGetChannelList");
        *numberOfChannels = 0;
        *channelList = NULL;
        return VOS_STATUS_E_FAULT;
    }

    ccmCfgGetInt(hHal, WNI_CFG_SAP_CHANNEL_SELECT_START_CHANNEL, &startChannelNum);
    ccmCfgGetInt(hHal, WNI_CFG_SAP_CHANNEL_SELECT_END_CHANNEL, &endChannelNum);
    ccmCfgGetInt(hHal, WNI_CFG_SAP_CHANNEL_SELECT_OPERATING_BAND, &operatingBand);
    ccmCfgGetInt(hHal, WNI_CFG_ENABLE_LTE_COEX, &enableLTECoex);

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO,
             "%s:sapGetChannelList: startChannel %d,EndChannel %d,Operatingband:%d",
             __func__,startChannelNum,endChannelNum,operatingBand);

    switch(operatingBand)
    {
        case RF_SUBBAND_2_4_GHZ:
           bandStartChannel = RF_CHAN_1;
           bandEndChannel = RF_CHAN_14;
           break;

        case RF_SUBBAND_5_LOW_GHZ:
           bandStartChannel = RF_CHAN_36;
           bandEndChannel = RF_CHAN_64;
           break;

        case RF_SUBBAND_5_MID_GHZ:
           bandStartChannel = RF_CHAN_100;
           bandEndChannel = RF_CHAN_140;
           break;

        case RF_SUBBAND_5_HIGH_GHZ:
           bandStartChannel = RF_CHAN_149;
           bandEndChannel = RF_CHAN_165;
           break;

        default:
           VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
             "sapGetChannelList:OperatingBand not valid ");
           /* assume 2.4 GHz */
           bandStartChannel = RF_CHAN_1;
           bandEndChannel = RF_CHAN_14;
           break;
    }
    /*Check if LTE coex is enabled and 2.4GHz is selected*/
    if (enableLTECoex && (bandStartChannel == RF_CHAN_1)
       && (bandEndChannel == RF_CHAN_14))
    {
        /*Set 2.4GHz upper limit to channel 9 for LTE COEX*/
        bandEndChannel = RF_CHAN_9;
    }
    /* Allocate the max number of channel supported */
    list = (v_U8_t *)vos_mem_malloc(NUM_5GHZ_CHANNELS);
    if (NULL == list)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "%s: Unable to allocate channel list", __func__);
        *numberOfChannels = 0;
        *channelList = NULL;
        return VOS_STATUS_E_RESOURCES;
    }

    /*Search for the Active channels in the given range */
    channelCount = 0;
    for( loopCount = bandStartChannel; loopCount <= bandEndChannel; loopCount++ )
    {
        if((startChannelNum <= rfChannels[loopCount].channelNum)&&
            (endChannelNum >= rfChannels[loopCount].channelNum ))
        {
            if( regChannels[loopCount].enabled )
            {
                list[channelCount] = rfChannels[loopCount].channelNum;
                channelCount++;
            }
        }
    }
    if (0 == channelCount)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
        "sapGetChannelList:No active channels present in the given range for the current region");
        /*LTE COEX: channel range outside the restricted 2.4GHz band limits*/
        if (enableLTECoex && (startChannelNum > bandEndChannel))
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_FATAL,
            "sapGetChannelList:SAP cannot be started as LTE COEX restricted 2.4GHz limits");
        }
    }

    /* return the channel list and number of channels to scan*/
    *numberOfChannels = channelCount;
    if(channelCount != 0)
    {
       *channelList = list;
    }
    else
    {
       *channelList = NULL;
        vos_mem_free(list);
    }
    return VOS_STATUS_SUCCESS;
}
#endif
