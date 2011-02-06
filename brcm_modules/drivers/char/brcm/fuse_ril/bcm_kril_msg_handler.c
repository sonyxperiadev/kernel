/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
#include "bcm_kril_common.h"
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

//Common Define
const UInt16 Select_Mf_Path = APDUFILEID_MF;
#define SIM_GetMfPathLen() ( sizeof(Select_Mf_Path) / sizeof(UInt16) )
#define SIM_GetMfPath() (&Select_Mf_Path)

static UInt8 schnlIDs[CHNL_IDS_SIZE];
static UInt8 scodings[CHNL_IDS_SIZE];

static const SIMSMSMesgStatus_t Kril_SMSMsgStatuss[] =
{
    SIMSMSMESGSTATUS_UNREAD,
    SIMSMSMESGSTATUS_READ,
    SIMSMSMESGSTATUS_UNSENT,
    SIMSMSMESGSTATUS_SENT,
};


void KRIL_SendSMSHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
   KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
 
    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            KRIL_SetInSendSMSHandler(FALSE);
            if (KRIL_GetSendSMSNumber() > 0) // if the SMS have more bodies, need to trigger the cmd queue event to process next SMS body
            {
                KRIL_DecrementSendSMSNumber();
                KRIL_CmdQueueWork();
            }
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }
 
    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSendMsgInfo_t *tdata = (KrilSendMsgInfo_t *)pdata->ril_cmd->data;
            UInt8 *pszMsg = NULL;
            Sms_411Addr_t sca; memset(&sca, 0, sizeof(Sms_411Addr_t));
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSendSMSResponse_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilSendSMSResponse_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            sca.Toa = tdata->Toa;
            sca.Len = tdata->Len;
            memcpy(sca.Val, tdata->Val, sca.Len);
            pszMsg = (UInt8 *) tdata->Pdu;
#if 0
            int a;
            for(a = 0 ; a <= tdata->Length; a++)
                KRIL_DEBUG(DBG_INFO, "pdu[%d]:0x%x\n", a, tdata->Pdu[a]);
#endif
            KRIL_DEBUG(DBG_INFO, "Toa:%d Len:%d\n", sca.Toa, sca.Len);
            KRIL_SetInSendSMSHandler(TRUE);
            CAPI2_SMS_SendSMSPduReq(GetNewTID(), GetClientID(), tdata->Length, pszMsg, &sca);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            SmsSubmitRspMsg_t* rsp = (SmsSubmitRspMsg_t*) capi2_rsp->dataBuf;
            KrilSendSMSResponse_t *rdata = (KrilSendSMSResponse_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "InternalErrCause:%d NetworkErrCause:0x%x submitRspType:%d tpMr:%d\n", rsp->InternalErrCause, rsp->NetworkErrCause, rsp->submitRspType, rsp->tpMr);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // enum renamed in CIB
            // **FIXME** MAG - check how this will affect user space... (if at all)
            if(MS_MN_SMS_NO_ERROR == rsp->NetworkErrCause)
#else
            if(MN_SMS_NO_ERROR == rsp->NetworkErrCause)
#endif
            {
                rdata->messageRef = rsp->tpMr;
                rdata->errorCode = rsp->NetworkErrCause;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RILErrorResult(rsp->InternalErrCause);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            KRIL_SetInSendSMSHandler(FALSE);
            if (KRIL_GetSendSMSNumber() > 0) // if more SMS in queue, need to trigger the cmd queue event to process next SMS body
            {
                KRIL_DecrementSendSMSNumber();
                KRIL_CmdQueueWork();
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

static Boolean sMoreMessageToSend = FALSE; // keep the value to check whether we need to enable the more message to send
static UInt32 sSMSExpectMoreState = 0; // store the handler state
void KRIL_SendSMSExpectMoreHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            KRIL_SetInSendSMSHandler(FALSE);
            if (KRIL_GetSendSMSNumber() > 0) // if the SMS have more bodies, need to trigger the cmd queue event to process next SMS body
            {
                KRIL_DecrementSendSMSNumber();
                KRIL_CmdQueueWork();
            }
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            if (FALSE == sMoreMessageToSend)
            {
                CAPI2_MS_Element_t data;
                memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
                data.inElemType = MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND;
                data.data_u.u8Data = 1; // enable the more message to send
                sMoreMessageToSend = TRUE;
                KRIL_SetInSendSMSHandler(TRUE);
                CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
                pdata->handler_state = BCM_SMS_SendSMSPduReq;
                return;
            }
        }

        case BCM_SMS_SendSMSPduReq:
        {
            KrilSendMsgInfo_t *tdata = (KrilSendMsgInfo_t *)pdata->ril_cmd->data;
            UInt8 *pszMsg = NULL;
            Sms_411Addr_t sca; memset(&sca, 0, sizeof(Sms_411Addr_t));
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilSendSMSResponse_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilSendSMSResponse_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            sca.Toa = tdata->Toa;
            sca.Len = tdata->Len;
            memcpy(sca.Val, tdata->Val, sca.Len);
            pszMsg = (UInt8 *) tdata->Pdu;
#if 0
            int a;
            for(a = 0 ; a <= tdata->Length; a++)
                KRIL_DEBUG(DBG_INFO, "pdu[%d]:0x%x\n", a, tdata->Pdu[a]);
#endif
            KRIL_DEBUG(DBG_INFO, "Toa:%d Len:%d\n", sca.Toa, sca.Len);
            KRIL_SetInSendSMSHandler(TRUE);
            CAPI2_SMS_SendSMSPduReq(GetNewTID(), GetClientID(), tdata->Length, pszMsg, &sca);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            SmsSubmitRspMsg_t* rsp = (SmsSubmitRspMsg_t*) capi2_rsp->dataBuf;
            KrilSendSMSResponse_t *rdata = (KrilSendSMSResponse_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "InternalErrCause:%d NetworkErrCause:0x%x submitRspType:%d tpMr:%d\n", rsp->InternalErrCause, rsp->NetworkErrCause, rsp->submitRspType, rsp->tpMr);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // enum renamed in CIB
            // **FIXME** MAG - check how this will affect user space... (if at all)
            if(MS_MN_SMS_NO_ERROR == rsp->NetworkErrCause)
#else
            if(MN_SMS_NO_ERROR == rsp->NetworkErrCause)
#endif // CONFIG_BRCM_FUSE_RIL_CIB
            {
                rdata->messageRef = rsp->tpMr;
                rdata->errorCode = rsp->NetworkErrCause;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RILErrorResult(rsp->InternalErrCause);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            if (KRIL_GetSendSMSNumber() > 0) // if the SMS have more bodies, need to trigger the cmd queue event to process next SMS body
            {
                KRIL_SetInSendSMSHandler(FALSE);
                KRIL_DecrementSendSMSNumber();
                KRIL_CmdQueueWork();
            }
            else // if the SMS body is last, we need to disable the more message to send
            {
                CAPI2_MS_Element_t data;
                memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
                data.inElemType = MS_LOCAL_SMS_ELEM_MORE_MESSAGE_TO_SEND;
                data.data_u.u8Data = 0; // disable the more message to send
                sSMSExpectMoreState = pdata->handler_state;
                CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
                pdata->handler_state = BCM_SMS_DisableMoreMessageToSend;
            }
        }
        break;

        case BCM_SMS_DisableMoreMessageToSend:
        {
            KRIL_SetInSendSMSHandler(FALSE);
            sMoreMessageToSend = FALSE;
            pdata->handler_state = sSMSExpectMoreState;
            sSMSExpectMoreState = 0;
        }
        break;
 
        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_WriteSMSToSIMHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
  
    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            if(TRUE == GetIsRevClass2SMS() && BCM_RESPCAPI2Cmd == pdata->handler_state) // send ack to network only for writeing Class 2 SMS in SIM fail
            {
                CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SMS_ACK_ERROR);
                pdata->handler_state = BCM_SMS_SendAckToNetwork;
                return;
            }
            KRIL_SetInUpdateSMSInSIMHandler(FALSE);
            KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS body
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }
  
    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 szMessage[SMSMESG_DATA_SZ];
            UInt16 rec_no;
            KrilWriteMsgInfo_t *tdata = (KrilWriteMsgInfo_t *)pdata->ril_cmd->data;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilMsgIndexInfo_t), GFP_KERNEL);
            memset(szMessage, SIM_RAW_EMPTY_VALUE, SMSMESG_DATA_SZ); // Fill the 0xFF in the struct
            pdata->rsp_len = sizeof(KrilMsgIndexInfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            memcpy(szMessage, tdata->Pdu, tdata->Length);
#if 0
            int i;
            for (i =0 ; i < tdata->Length; i++)
                KRIL_DEBUG(DBG_INFO, "szMessage[%d]:0x%x\n", i, szMessage[i]);
#endif
            KRIL_SetInUpdateSMSInSIMHandler(TRUE);
            rec_no = CheckFreeSMSIndex();
            if (rec_no != SMS_FULL)
            {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                // **FIXME** MAG - CAPI2_SMS_WriteSMSPduToSIMRecordReq not currently supported under CIB
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
#else
                CAPI2_SMS_WriteSMSPduToSIMRecordReq(GetNewTID(), GetClientID(), SMSMESG_DATA_SZ, szMessage, Kril_SMSMsgStatuss[tdata->MsgState], rec_no);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
            }
            else
            {
                if(GetIsRevClass2SMS() != TRUE)
                {
                    KRIL_SendNotify(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);
                    KRIL_SetInUpdateSMSInSIMHandler(FALSE);
                    KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
                else
                {
                    if (TRUE == KRIL_GetMESMSAvailable())
                    {
                        CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SMS_ACK_PROTOCOL_ERROR);
                        pdata->handler_state = BCM_SMS_SendAckToNetwork;
                    }
                    else
                    {
                        CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SMS_ACK_MEM_EXCEEDED);
                        pdata->handler_state = BCM_SIM_UpdateSMSCapExceededFlag;
                    }
                    KRIL_SendNotify(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);
                }
            }
        }
        break;
  
        case BCM_RESPCAPI2Cmd:
        {
            if (MSG_SMS_WRITE_RSP_IND == capi2_rsp->msgType)
            {
                SmsIncMsgStoredResult_t *pSmsInfo = (SmsIncMsgStoredResult_t *)capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "rec_no:%d\n", pSmsInfo->rec_no);
                if(GetIsRevClass2SMS() != TRUE)
                {
                    KrilMsgIndexInfo_t *rdata = (KrilMsgIndexInfo_t *)pdata->bcm_ril_rsp;
                    rdata->result = RILErrorSIMResult(pSmsInfo->result);
                    if (SIMACCESS_SUCCESS == pSmsInfo->result)
                    {
                        KrilWriteMsgInfo_t *tdata = (KrilWriteMsgInfo_t *)pdata->ril_cmd->data;
                        rdata->index = pSmsInfo->rec_no;
                        rdata->index++;
                        SetSMSMesgStatus((UInt8)pSmsInfo->rec_no, Kril_SMSMsgStatuss[tdata->MsgState]);
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                    }
                    else
                    {
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    }
                    KRIL_SetInUpdateSMSInSIMHandler(FALSE);
                    KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS
                }
                else // for Class 2 SMS
                {
                    KrilWriteMsgInfo_t *tdata = (KrilWriteMsgInfo_t *)pdata->ril_cmd->data;
                    KRIL_DEBUG(DBG_INFO, "pSmsInfo result:%d\n", pSmsInfo->result);
                    if(SIMACCESS_SUCCESS == pSmsInfo->result)
                    {
                        KrilMsgIndexInfo_t msg;
                        msg.result = RILErrorSIMResult(pSmsInfo->result);
                        msg.index = pSmsInfo->rec_no;
                        msg.index++;
                        SetSMSMesgStatus((UInt8)pSmsInfo->rec_no, Kril_SMSMsgStatuss[tdata->MsgState]);
                        if (0 == tdata->MoreSMSToReceive)
                        {
                            CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SMS_ACK_SUCCESS);
                            pdata->handler_state = BCM_SMS_SendAckToNetwork;
                    }
                    else
                    {
                            SetIsRevClass2SMS(FALSE);
                            KRIL_SetInUpdateSMSInSIMHandler(FALSE);
                            KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS
                            pdata->handler_state = BCM_FinishCAPI2Cmd;
                        }
                        KRIL_SendNotify(RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, &msg, sizeof(KrilMsgIndexInfo_t));
                    }
                    else
                    {
                        CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SMS_ACK_ERROR);
                        pdata->handler_state = BCM_SMS_SendAckToNetwork;
                        }
                    }
            }
            else
            {
                KRIL_SetSMSToSIMTID(capi2_rsp->tid);
            }
        }
        break;

        case BCM_SIM_UpdateSMSCapExceededFlag:
        {
            UInt8 data[]={0xFE};
            CAPI2_SIM_SubmitBinaryEFileUpdateReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMSS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, 1, data, 1, SIM_GetMfPathLen(), SIM_GetMfPath());
            pdata->handler_state = BCM_SMS_SendAckToNetwork;
        }
        break;

        case BCM_SMS_SendAckToNetwork:
        {
            SetIsRevClass2SMS(FALSE);
            KRIL_SetInUpdateSMSInSIMHandler(FALSE);
            KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS
            pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_DeleteSMSOnSIMHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
 
    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            KRIL_SetInUpdateSMSInSIMHandler(FALSE);
            KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS body
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }
 
    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *index = (int *)(pdata->ril_cmd->data);
            UInt8 szMessage[SMSMESG_DATA_SZ];
            memset(szMessage,SIM_RAW_EMPTY_VALUE, SMSMESG_DATA_SZ);
            KRIL_DEBUG(DBG_INFO, "index:%d\n", (*index-1));
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            // **FIXME** MAG - CAPI2_SMS_WriteSMSPduToSIMRecordReq not currently supported under CIB
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            KRIL_DEBUG(DBG_ERROR, "** needs fix for CIB\n");
#else
            KRIL_SetInUpdateSMSInSIMHandler(TRUE);
            CAPI2_SMS_WriteSMSPduToSIMRecordReq(GetNewTID(), GetClientID(), SMSMESG_DATA_SZ, szMessage, SIMSMSMESGSTATUS_FREE, (*index-1));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
#endif
        }
        break;
 
        case BCM_RESPCAPI2Cmd:
        {
             if (MSG_SMS_WRITE_RSP_IND == capi2_rsp->msgType)
             {
                 SmsIncMsgStoredResult_t *pSmsInfo = (SmsIncMsgStoredResult_t *)capi2_rsp->dataBuf;
                 if (SIMACCESS_SUCCESS == pSmsInfo->result)
                 {
                    KRIL_DEBUG(DBG_INFO, "rec_no:%d\n", pSmsInfo->rec_no);
                    SetSMSMesgStatus((UInt8)pSmsInfo->rec_no, SIMSMSMESGSTATUS_FREE);
                     pdata->handler_state = BCM_FinishCAPI2Cmd;
                 }
                 else
                 {
                     pdata->handler_state = BCM_ErrorCAPI2Cmd;
                 }
                 KRIL_SetInUpdateSMSInSIMHandler(FALSE);
                 KRIL_GetUpdateSMSNumber();// if more update SMS in queue, need to trigger the cmd queue event to process next SMS
             }
             else
             {
                 KRIL_SetSMSToSIMTID(capi2_rsp->tid);
             }
#if 0
             if(1)//TRUE == g_SMSStorageOverflow)
             {
                 pdata->handler_state = BCM_FinishCAPI2Cmd;
             }
             else
            {
                //CAPI2_SIM_SubmitBinaryEFileReadReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMSS, (m_CPAI2h->GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, 1, 1, SIM_GetMfPathLen(), SIM_GetMfPath());
                //pdata->handler_state = BCM_SIM_GetSmsMemExceededFlag;
            }
#endif
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SMSAcknowledgeHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilMsgAckInfo_t *tdata = (KrilMsgAckInfo_t *)pdata->ril_cmd->data;
            SmsAckNetworkType_t SmsAckNetworkType;

            KRIL_DEBUG(DBG_INFO, "AckType:0x%x FailCause:%d\n", tdata->AckType, tdata->FailCause);
            if (1 == tdata->AckType)
            {
                SmsAckNetworkType = SMS_ACK_SUCCESS;
            }
            else
            {
                if (0xd3 == tdata->FailCause)
                {
                    SmsAckNetworkType = SMS_ACK_MEM_EXCEEDED;
                }
                else if (0xd0 == tdata->FailCause || 0xd1 == tdata->FailCause)
                {
                    SmsAckNetworkType = SMS_ACK_PROTOCOL_ERROR;
                }
                else
                {
                    SmsAckNetworkType = SMS_ACK_ERROR;
                }
            }
            CAPI2_SMS_SendAckToNetwork(GetNewTID(), GetClientID(), KRIL_GetSmsMti(), SmsAckNetworkType);
            pdata->handler_state = BCM_SIM_UpdateSMSCapExceededFlag;
        }
        break;

        case BCM_SIM_UpdateSMSCapExceededFlag:
        {
            KrilMsgAckInfo_t *tdata = (KrilMsgAckInfo_t *)pdata->ril_cmd->data;
            if (0xd3 == tdata->FailCause)
            {
                UInt8 data[]={0xFE}; // SMS full
                CAPI2_SIM_SubmitBinaryEFileUpdateReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMSS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, 1, data, 1, SIM_GetMfPathLen(), SIM_GetMfPath());
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetSMSCAddressHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
 
    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }
 
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(krilGetSMSCAddress_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(krilGetSMSCAddress_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_SMS_GetSMSrvCenterNumber(GetNewTID(), GetClientID(), USE_DEFAULT_SCA_NUMBER);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
 
        case BCM_RESPCAPI2Cmd:
        {
            SmsAddress_t *rsp = (SmsAddress_t *)capi2_rsp->dataBuf;
            krilGetSMSCAddress_t *rdata = (krilGetSMSCAddress_t *)pdata->bcm_ril_rsp;
            if (145 == rsp->TypeOfAddress)
            {
                rdata->smsc[0] = '+';
                strcpy(&rdata->smsc[1], rsp->Number);
            }
            else
            {
                strcpy(rdata->smsc, rsp->Number);
            }
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd:: smsc:%s TypeOfAddress:%d\n", rdata->smsc, rsp->TypeOfAddress);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetSMSCAddressHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
  
    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }
  
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSetSMSCAddress_t *tdata = (KrilSetSMSCAddress_t *)pdata->ril_cmd->data;
            SmsAddress_t psca;
            strcpy(psca.Number, tdata->smsc);
            if('+' == tdata->smsc[0])
                psca.TypeOfAddress = 145;
            else
                psca.TypeOfAddress = 129;

            KRIL_DEBUG(DBG_ERROR, "TypeOfAddress:%d Number:%s...!\n", psca.TypeOfAddress, psca.Number);
            CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq(GetNewTID(), GetClientID(), &psca, USE_DEFAULT_SCA_NUMBER);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }
  
        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_ReportSMSMemoryStatusHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->result = RILErrorResult(capi2_rsp->result);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int available = *((int *)pdata->ril_cmd->data);
            if (available != KRIL_GetMESMSAvailable())
            {
                KRIL_SetMESMSAvailable(available);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
                if (TRUE == available) // if memory is available, need to send a indication to network and update the EF-SMSS status in SIM.
                {
                    CAPI2_SIM_SubmitBinaryEFileReadReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMSS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, 1, 1, SIM_GetMfPathLen(), SIM_GetMfPath());
                    pdata->handler_state = BCM_SIM_UpdateSMSCapExceededFlag;
                }
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        case BCM_SIM_UpdateSMSCapExceededFlag:
        {
            UInt8 data[]={0xFF};
            CAPI2_SIM_SubmitBinaryEFileUpdateReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMSS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, 1, data, 1, SIM_GetMfPathLen(), SIM_GetMfPath());
            pdata->handler_state = BCM_SMS_SendMemAvailInd;
            break;
        }

        case BCM_SMS_SendMemAvailInd:
        {
            SIM_EFILE_DATA_t *rsp = (SIM_EFILE_DATA_t *)capi2_rsp->dataBuf;
            if (0xFE == *rsp->ptr)
            {
                CAPI2_SMS_SendMemAvailInd(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_GetBroadcastSmsHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilGsmBroadcastGetSmsConfigInfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilGsmBroadcastGetSmsConfigInfo_t);
            CAPI2_SMS_GetCBMI(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_SMS_GetCBMI;
            break;
        }

        case BCM_SMS_GetCBMI:
        {
            UInt8 i;
            KrilGsmBroadcastGetSmsConfigInfo_t *rdata = (KrilGsmBroadcastGetSmsConfigInfo_t *)pdata->bcm_ril_rsp;
            SMS_CB_MSG_IDS_t *rsp = (SMS_CB_MSG_IDS_t *)capi2_rsp->dataBuf;

            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }

            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            pdata->rsp_len = rsp->nbr_of_msg_id_ranges * sizeof(iKrilGetCBSMSConfigInfo_t);

            for(i = 0 ; i < rsp->nbr_of_msg_id_ranges ; i++)
            {
                rdata->content[i].fromServiceId = rsp->msg_id_range_list.A[i].start_pos;
                rdata->content[i].toServiceId = rsp->msg_id_range_list.A[i].stop_pos;
                KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "start_pos:0x%x stop_pos:0x%x\n", rsp->msg_id_range_list.A[i].start_pos, rsp->msg_id_range_list.A[i].stop_pos);
            }

            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "nbr_of_msg_id_ranges:%d\n",rsp->nbr_of_msg_id_ranges);
            *(UInt8*)(pdata->cmdContext) = rsp->nbr_of_msg_id_ranges;

            CAPI2_SMS_GetCbLanguage(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            UInt8 i,j;
            UInt8 k = 0;
            UInt8 record = 0;
            SMS_CB_MSG_ID_RANGE_LIST_t map_CodeScheme;

            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            else
            {
                KrilGsmBroadcastGetSmsConfigInfo_t *rdata = (KrilGsmBroadcastGetSmsConfigInfo_t *)pdata->bcm_ril_rsp;
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                MS_T_MN_CB_LANGUAGES* rsp = (MS_T_MN_CB_LANGUAGES *)capi2_rsp->dataBuf;
#else
                T_MN_CB_LANGUAGES* rsp = (T_MN_CB_LANGUAGES *)capi2_rsp->dataBuf;
#endif // CONFIG_BRCM_FUSE_RIL_CIB

                if (0 < rsp->nbr_of_languages)
                {
                    if (1 == rsp->nbr_of_languages) //only one set of code scheme for each set of language pairs
                    {
                        for (i = 0; i < *(UInt8*)(pdata->cmdContext); i++)
                        {
                            rdata->content[i].fromCodeScheme = rsp->language_list.A[0];
                            rdata->content[i].toCodeScheme = rsp->language_list.A[rsp->nbr_of_languages - 1];
                            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "fromCodeScheme:0x%x toCodeScheme:0x%x\n", rsp->language_list.A[0], rsp->language_list.A[rsp->nbr_of_languages - 1]);
                        }
                    }
                    else
                    {
                        for (j = 0; j < rsp->nbr_of_languages; j++)
                        {
                            if ((0 == j)||(k != record)) //this item of code scheme is a starting number of a new set
                            {
                                map_CodeScheme.A[k].start_pos = rsp->language_list.A[j];
                            }
                            if ((rsp->nbr_of_languages - 1) == j) //for the last item of code scheme list
                            {
                                map_CodeScheme.A[k].stop_pos = rsp->language_list.A[j];
                                break;
                            }
                            if (rsp->language_list.A[j+1] == (rsp->language_list.A[j] + 1)) //this item is a continuous number from previous one
                            {
                                map_CodeScheme.A[k].stop_pos = rsp->language_list.A[j];
                                record = k;
                            }
                            else if (rsp->language_list.A[j+1] != (rsp->language_list.A[j] + 1)) //the stop of this code scheme pair
                            {
                                map_CodeScheme.A[k].stop_pos = rsp->language_list.A[j];
                                k++;
                            }
                        }

                        if ((k+1) != *(UInt8*)(pdata->cmdContext)) //just for check
                        {
                            KRIL_DEBUG(DBG_ERROR, "Nbr of msg id:%d, Nbr of code scheme:%d\n", *(UInt8*)(pdata->cmdContext), k+1);
                        }

                        for (i = 0; i < *(UInt8*)(pdata->cmdContext); i++)
                        {
                            j = i;
                            if (k < i) //number of code scheme is smaller than number of message id 
                                j = k;
                            rdata->content[i].fromCodeScheme = map_CodeScheme.A[j].start_pos;
                            rdata->content[i].toCodeScheme = map_CodeScheme.A[j].stop_pos;
                            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "fromCodeScheme:0x%x toCodeScheme:0x%x\n", rdata->content[i].fromCodeScheme, rdata->content[i].toCodeScheme);
                        }
                    }
                }
                KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "nbr_of_languages:%d\n", rsp->nbr_of_languages);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SetBroadcastSmsHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilGsmBroadcastSmsConfigInfo_t *tdata = (KrilGsmBroadcastSmsConfigInfo_t *)pdata->ril_cmd->data;
            UInt8* pchnlIDs = tdata->mids;
            UInt8* pcodings = tdata->dcss;
            UInt8  mode;

            mode = tdata->selected;
            memset(schnlIDs, 0, CHNL_IDS_SIZE);
            strncpy(schnlIDs, pchnlIDs, CHNL_IDS_SIZE);

            memset(scodings, 0, CHNL_IDS_SIZE);
            strncpy(scodings, pcodings, CHNL_IDS_SIZE);

            if (NULL != pchnlIDs)
                KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "mid:%s\n", schnlIDs);
            if (NULL != pcodings)
                KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "dcs:%s\n", scodings);

            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "mode:%d\n", mode);
            CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), mode, pchnlIDs, pcodings);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SmsBroadcastActivationHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 mode = *((UInt8 *)pdata->ril_cmd->data);
            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "mode:%d \n", mode);

            if (0 == mode) //Active
            {
                CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), mode, schnlIDs, scodings);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else //turn-off
            {
                CAPI2_SMS_SetCellBroadcastMsgTypeReq(GetNewTID(), GetClientID(), mode, schnlIDs, scodings);
                pdata->handler_state = BCM_SMS_SetCBOff;
            }
            break;
        }

        case BCM_SMS_SetCBOff:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            KRIL_DEBUG(DBG_ERROR/*DBG_INFO*/, "BCM_SMS_SetCBOff: StopReceivingCellBroadcastReq\n");
            CAPI2_SMS_StopReceivingCellBroadcastReq(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK )
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_QuerySMSInSIMHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
    {
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
        if(capi2_rsp->result != RESULT_OK)
        {
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            return;
        }
    }

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_SIM_SubmitEFileInfoReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, SIM_GetMfPathLen(), SIM_GetMfPath());
            pdata->handler_state = BCM_SIM_SubmitRecordEFileReadReq;
        }
        break;

        case BCM_SIM_SubmitRecordEFileReadReq:
        {
            SIM_EFILE_INFO_t *rsp = (SIM_EFILE_INFO_t *) capi2_rsp->dataBuf;
            if (SIMACCESS_SUCCESS == rsp->result)
            {
                *(UInt8*)(pdata->cmdContext) = 0;
                KRIL_SetTotalSMSInSIM(rsp->file_size/rsp->record_length);
                KRIL_DEBUG(DBG_INFO, "KRIL_GetTotalSMSInSIM:%d\n", KRIL_GetTotalSMSInSIM());
                CAPI2_SIM_SubmitRecordEFileReadReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, (*(UInt8*)(pdata->cmdContext)+1), SMSMESG_DATA_SZ+1, SIM_GetMfPathLen(), SIM_GetMfPath());
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            SIM_EFILE_DATA_t *rsp = (SIM_EFILE_DATA_t *) capi2_rsp->dataBuf;
            if (SIMACCESS_SUCCESS == rsp->result)
            {
                SetSMSMesgStatus(*(UInt8*)(pdata->cmdContext), *rsp->ptr);
                KRIL_DEBUG(DBG_INFO, "KRIL_GetTotalSMSInSIM:%d GetSMSMesgStatus[%d]:%d\n", KRIL_GetTotalSMSInSIM(), *(UInt8 *)pdata->cmdContext, GetSMSMesgStatus(*(UInt8 *)pdata->cmdContext));
                (*(UInt8 *)pdata->cmdContext)++;
                if((*(UInt8 *)pdata->cmdContext) < KRIL_GetTotalSMSInSIM())
                {
                    CAPI2_SIM_SubmitRecordEFileReadReq(GetNewTID(), GetClientID(), USIM_BASIC_CHANNEL_SOCKET_ID, APDUFILEID_EF_SMS, (KRIL_GetSimAppType() == SIM_APPL_2G)?APDUFILEID_DF_TELECOM : APDUFILEID_USIM_ADF, (*(UInt8*)(pdata->cmdContext)+1), SMSMESG_DATA_SZ+1, SIM_GetMfPathLen(), SIM_GetMfPath());
                }
                else
                {
                    if (SMS_FULL == CheckFreeSMSIndex())
                    {
                        KRIL_SendNotify(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);
                    }
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}
