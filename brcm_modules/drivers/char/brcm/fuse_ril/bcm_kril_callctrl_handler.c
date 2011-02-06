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
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include "capi2_cc_api.h"
#endif

static char DTMFString;

static RIL_LastCallFailCause ResultToRilFailCause(Result_t inResult);

void KRIL_GetCurrentCallHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCallListState_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCallListState_t);
            memset(pdata->bcm_ril_rsp, 0, sizeof(KrilCallListState_t));
            CAPI2_CC_GetAllCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_AllCallIndex;
            break;
        }

        case BCM_CC_AllCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t*) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            UInt8 i;
            rdata->total_call = rsp->listSz;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLINDEX_RSP::total_call:%d listSz:%d\n", rdata->total_call, rsp->listSz);
            if(0 == rsp->listSz)
            {
                KRIL_ClearCallNumPresent(); // Clear to default presentation allowed
                pdata->handler_state = BCM_FinishCAPI2Cmd;
                break;
            }

            for(i = 0 ; i < rsp->listSz ; i++)
            {
                rdata->KRILCallState[i].index = (int)rsp->indexList[i];
                KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLINDEX_RSP::indexList:%d callIndex:%d\n", (int)rsp->indexList[i], rdata->KRILCallState[i].index);
            }
            CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_GetCallType;
            break;
        }

        case BCM_CC_GetCallType:
        {
            UInt32 *rsp = (UInt32 *)capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLTYPE_RSP::call type:%lu\n",*rsp);
            
            rdata->KRILCallState[rdata->index].isMTVTcall = 0;
            
            switch(*rsp)
            {
                case MOVOICE_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTVOICE_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MODATA_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTDATA_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MOFAX_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTFAX_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                break;

                case MOVIDEO_CALL:
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;

                case MTVIDEO_CALL:
#ifdef VIDEO_TELEPHONY_ENABLE
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 1;
                    rdata->KRILCallState[rdata->index].isMTVTcall = 1;
                break;
#else
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                // If Video telephony not support, reject the VT call.
                {
                    KRIL_DEBUG(DBG_ERROR,"VT call not supported!!!\n");
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;

                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                    pdata->handler_state = BCM_CC_GetVTCallEndResult;
                    return;
                }
#else 
                {
                    CAPI2_MS_Element_t data;
                    CallReleaseInfo_t t;
                    
                    KRIL_DEBUG(DBG_ERROR,"VT call not supported!!!\n");
                    rdata->KRILCallState[rdata->index].isVoice = 0;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                    
                    memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
                    data.inElemType = MS_LOCAL_CC_ELEM_END_CALL_CAUSE;
                    t.callIndex = rdata->KRILCallState[rdata->index].index;
                    t.cause = MNCAUSE_INCOMPATIBLE_DESTINATION;
                    memcpy(&(data.data_u), (void *) &t, sizeof(data.data_u));
                    CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
                    pdata->handler_state = BCM_CC_SetVTCallEndCause;
                    return;
                }
#endif  //CONFIG_BRCM_FUSE_RIL_CIB                
#endif //VIDEO_TELEPHONY_ENABLE

                default:
                    //Unknow dir and mode
                    rdata->KRILCallState[rdata->index].isVoice = 1;
                    rdata->KRILCallState[rdata->index].isMT = 0;
                break;
            }
            
            KRIL_SetCallType(rdata->index, *rsp);
            KRIL_DEBUG(DBG_INFO, "GetCallNumPresent:%d rdata->index:%d rdata->KRILCallState[rdata->index].index:%d\n",KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index), rdata->index, rdata->KRILCallState[rdata->index].index);
            if (CC_PRESENTATION_ALLOWED == KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index) || 0 == rdata->KRILCallState[rdata->index].isMT) //The number is always present for MO call and MT call with presentation allowed
            {
                CAPI2_CC_GetCallNumber(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 0; //Allowed
                pdata->handler_state = BCM_CC_GetCallNumber;
            }
            else
            {
                if (CC_PRESENTATION_RESTRICTED == KRIL_GetCallNumPresent(rdata->KRILCallState[rdata->index].index))
                {
                    rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 1; //Restricted
                }
                else
                {
                    rdata->KRILCallState[rdata->KRILCallState[rdata->index].index].numberPresentation = 2; //Not Specified/Unknown
                }
                CAPI2_CC_IsMultiPartyCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_IsMultiPartyCall;
            }
        }
        break;

        case BCM_CC_GetCallNumber:
        {
            PHONE_NUMBER_STR_t *rsp = (PHONE_NUMBER_STR_t *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLNUMBER_RSP::phone_number:%s\n",rsp->phone_number);

            if(rsp->phone_number[0] == INTERNATIONAL_CODE)
            {
                rdata->KRILCallState[rdata->index].toa = TOA_International;
            }
            else if(rsp->phone_number[0] == '\0')
            {
                rdata->KRILCallState[rdata->index].toa = 0;
            }
            else
            {
                rdata->KRILCallState[rdata->index].toa = TOA_Unknown;
            }
            strcpy(rdata->KRILCallState[rdata->index].number, rsp->phone_number);
            KRIL_DEBUG(DBG_INFO, "MSG_CC_GETCALLNUMBER_RSP::rdata->index:%d KRILCallState->phone_number:%s\n", rdata->index, rdata->KRILCallState[rdata->index].number);
            CAPI2_CC_IsMultiPartyCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_IsMultiPartyCall;
        }
        break;

        case BCM_CC_IsMultiPartyCall:
        {
            Boolean *rsp = (Boolean *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            KRIL_DEBUG(DBG_INFO, "MSG_CC_ISMULTIPARTYCALL_RSP::Ismultipartcall:%d index:%d total_call:%d\n", *rsp, rdata->index, rdata->total_call);
            if((rdata->index + 1) >= rdata->total_call)
            {
                CAPI2_CC_GetAllCallStates(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetAllCallStates;
            }
            else
            {
                rdata->index++;
                CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_GetCallType;
            }
        }
        break;

        case BCM_CC_GetAllCallStates:
        {
            UInt8 i;
            ALL_CALL_STATE_t *rsp = (ALL_CALL_STATE_t *) capi2_rsp->dataBuf;
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;

            for(i = 0 ; i < rsp->listSz ; i++)
            {
                KRIL_DEBUG(DBG_INFO, "BCM_CC_GetAllCallStates::call size:%d state:%d\n", rsp->listSz, rsp->stateList[rdata->KRILCallState[i].index]);
                switch((UInt32)rsp->stateList[rdata->KRILCallState[i].index])
                {
                    case CC_CALL_ACTIVE:
                        rdata->KRILCallState[i].state = RIL_CALL_ACTIVE;
                    break;

                    case CC_CALL_HOLD:
                        rdata->KRILCallState[i].state = RIL_CALL_HOLDING;
                    break;

                    case CC_CALL_CALLING:
                        rdata->KRILCallState[i].state = RIL_CALL_DIALING;
                    break;

                    case CC_CALL_ALERTING:
                    case CC_CALL_CONNECTED:
                        if(1 == rdata->KRILCallState[i].isMT)
                        {
                            rdata->KRILCallState[i].state = RIL_CALL_INCOMING;
                        }
                        else
                        {
                            rdata->KRILCallState[i].state = RIL_CALL_ALERTING;
                        }
                    break;

                    case CC_CALL_WAITING:
                        rdata->KRILCallState[i].state = RIL_CALL_WAITING;
                    break;

                    case CC_CALL_BEGINNING:
                        if(1 == rdata->KRILCallState[i].isMT)
                        {
                            rdata->KRILCallState[i].state = RIL_CALL_INCOMING;
                        }
                        else
                        {
                            rdata->KRILCallState[i].state = RIL_CALL_DIALING;
                        }
                    break;

                    default:
                        rdata->KRILCallState[i].state = 0; //Unknow state
                    break;
                }
                KRIL_DEBUG(DBG_INFO, "MSG_CC_GETALLCALLSTATES_RSP::[%d] state:%d rstate:%d\n", i, rsp->stateList[i], rdata->KRILCallState[i].state);
            }
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
        
        case BCM_CC_SetVTCallEndCause:
        {
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
            pdata->handler_state = BCM_CC_GetVTCallEndResult;
            break;
        }
        
        case BCM_CC_GetVTCallEndResult:
        {
            KrilCallListState_t *rdata = (KrilCallListState_t *)pdata->bcm_ril_rsp;
            
            if (capi2_rsp->result == CC_END_CALL_FAIL)
            {
                KRIL_DEBUG(DBG_ERROR,"End VT call Failed!!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
            
            KRIL_DEBUG(DBG_INFO,"End VT call successfully index:%d\n",rdata->KRILCallState[rdata->index].index);
            
            if ((rdata->index + 1) >= rdata->total_call)
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                rdata->index++;
                CAPI2_CC_GetCallType(GetNewTID(), GetClientID(), rdata->KRILCallState[rdata->index].index);
                pdata->handler_state = BCM_CC_GetCallType;
            }
            
            rdata->total_call -= 1;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_DialHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);
    
    switch (pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
            
            if (tdata->isVTcall)
            {
#ifdef VIDEO_TELEPHONY_ENABLE
                CAPI2_CallConfig_t    CallCongif;

                // Set "AT+CBST:134,1,0" for Video Telephony.
                CallCongif.configType = CALL_CFG_CBST_PARAMS;
                CallCongif.param_u.cbst_params.speed = 134;    // RIL_SPEED_64000_MULTIMEDIA
                CallCongif.param_u.cbst_params.name = 1;       // RIL_BSVCNAME_DATACIRCUIT_SYNC_UDI_MODEM
                CallCongif.param_u.cbst_params.ce = 0;         // RIL_BSVCCE_TRANSPARENT
                
                CAPI2_CC_SetCallCfg(GetNewTID(), GetClientID(), &CallCongif);
                pdata->handler_state = BCM_CC_SetCBSTFinished;
                
#else
                KRIL_DEBUG(DBG_ERROR, "VT call not supported. Error!!!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
#endif //VIDEO_TELEPHONY_ENABLE
            }
            else
            {
                CAPI2_SIM_GetAlsDefaultLine(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_SIM_GetAlsDefaultLine;
                KRIL_SetLastCallFailCause(CALL_FAIL_NORMAL);
            }

            break;
        }

        case BCM_SIM_GetAlsDefaultLine:
        {
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            UInt32* rsp = (UInt32*) capi2_rsp->dataBuf;
            if ( capi2_rsp->result == RESULT_OK )
#else
            SIM_INTEGER_DATA_t *rsp = (SIM_INTEGER_DATA_t *) capi2_rsp->dataBuf;
            if ((rsp->result == SIMACCESS_SUCCESS) ||
               ((rsp->result == SIMACCESS_INCORRECT_CHV ||
                  rsp->result == SIMACCESS_BLOCKED_CHV ||
                  rsp->result == SIMACCESS_NEED_CHV1 ||
                  rsp->result == SIMACCESS_NEED_CHV2 ||
                  rsp->result == SIMACCESS_NOT_NEED_CHV1 ||
                  rsp->result == SIMACCESS_NOT_NEED_CHV2 ||
                  rsp->result == SIMACCESS_INCORRECT_PUK ||
                  rsp->result == SIMACCESS_BLOCKED_PUK ||
                  rsp->result == SIMACCESS_NO_SIM ||
                  rsp->result == SIMACCESS_CANNOT_REPLACE_SMS))
              )
#endif
            {
                KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
                VoiceCallParam_t m_VoiceCallParam;
                Subaddress_t defaultSubAddress = {0,0,0};

                KRIL_DEBUG(DBG_INFO, "address:%s clir:%d\n", tdata->address, tdata->clir);
                memset(&m_VoiceCallParam, 0, sizeof(VoiceCallParam_t));

                m_VoiceCallParam.subAddr = defaultSubAddress;
                m_VoiceCallParam.cug_info.cug_index = CUGINDEX_NONE;
                m_VoiceCallParam.cug_info.suppress_pref_cug = CUGSUPPRESSPREFCUG_DISABLE;
                m_VoiceCallParam.cug_info.suppress_oa = CUGSUPPRESSOA_DISABLE;
                m_VoiceCallParam.clir = tdata->clir;
                m_VoiceCallParam.isFdnChkSkipped = FALSE;
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
                m_VoiceCallParam.auxiliarySpeech = rsp == 1 ? TRUE : FALSE;
#else
                m_VoiceCallParam.auxiliarySpeech = rsp->value == 1 ? TRUE : FALSE;
#endif
                m_VoiceCallParam.isEmergency = tdata->isEmergency;
                KRIL_DEBUG(DBG_INFO, "auxiliarySpeech:%d isEmergency:%d\n", m_VoiceCallParam.auxiliarySpeech, m_VoiceCallParam.isEmergency);
                CAPI2_CC_MakeVoiceCall(GetNewTID(), GetClientID(), (UInt8*)tdata->address, m_VoiceCallParam);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Can't Dial...!\n");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;  // can't dial
            }
            break;
        }

#ifdef VIDEO_TELEPHONY_ENABLE        
        case BCM_CC_SetCBSTFinished:
        {
            CAPI2_ECHO_SetDigitalTxGain(GetNewTID(), GetClientID(), -100);
            pdata->handler_state = BCM_CC_MakeVideoCall;           
            break;
        }

        case BCM_CC_MakeVideoCall:
        {
            KrilDial_t *tdata = (KrilDial_t *)pdata->ril_cmd->data;
            
            if (NULL != tdata->address)
            {
                KRIL_DEBUG(DBG_ERROR, "Phone number: %s\n", tdata->address);
                CAPI2_CC_MakeVideoCall(GetNewTID(), GetClientID(), (UInt8*)tdata->address);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Phone number is Null. Error!!!");
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }
#endif //VIDEO_TELEPHONY_ENABLE
        
        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            
            if (capi2_rsp->msgType == MSG_VOICECALL_CONNECTED_IND)
            {
                VoiceCallConnectMsg_t *rsp = (VoiceCallConnectMsg_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_CONNECTED_IND::callIndex:%d progress_desc:%d\n", rsp->callIndex,rsp->progress_desc);
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
            else if (capi2_rsp->msgType == MSG_VOICECALL_RELEASE_IND || 
                       capi2_rsp->msgType == MSG_VOICECALL_RELEASE_CNF)
            {
                pdata->result = RILErrorResult(capi2_rsp->result);
                KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::result:%d, capi2_rsp->result:%d\n", pdata->result, capi2_rsp->result);
                if (capi2_rsp->dataBuf != NULL)
                {
                    VoiceCallReleaseMsg_t *rsp = (VoiceCallReleaseMsg_t *) capi2_rsp->dataBuf;
                    KRIL_DEBUG(DBG_INFO, "MSG_VOICECALL_RELEASE_IND::callIndex:%d exitCause:%d callCCMUnit:%ld callDuration:%ld\n", rsp->callIndex, rsp->exitCause, rsp->callCCMUnit, rsp->callDuration);
                    KRIL_SetLastCallFailCause( KRIL_MNCauseToRilError(rsp->exitCause) );
                }
                else
                {
                    KRIL_SetLastCallFailCause( ResultToRilFailCause(capi2_rsp->result) );
                }
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
            }
#ifdef VIDEO_TELEPHONY_ENABLE
            else if (capi2_rsp->msgType == MSG_DATACALL_CONNECTED_IND)
            {
                if (NULL != capi2_rsp->dataBuf)
                {
                    DataCallConnectMsg_t *rsp = (DataCallConnectMsg_t *) capi2_rsp->dataBuf;
                    int callIndex = rsp->callIndex;
                    
                    KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_CONNECTED_IND::callIndex: %d\n",rsp->callIndex);
                    KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, &callIndex, sizeof(int));
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "MSG_DATACALL_CONNECTED_IND\n");
                    KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, NULL, 0);                    
                }
            }
            else if (capi2_rsp->msgType == MSG_DATACALL_RELEASE_IND)
            {
                if (NULL != capi2_rsp->dataBuf)
                {
                    DataCallReleaseMsg_t *rsp = (DataCallReleaseMsg_t *) capi2_rsp->dataBuf;
                    int callIndex = rsp->callIndex;
                    
                    KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_RELEASE_IND:: callIndex: %d exitCause: 0x%X\n", rsp->callIndex, rsp->exitCause);
                    KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, &callIndex, sizeof(int));
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR, "MSG_DATACALL_RELEASE_IND\n");
                    KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, NULL, 0);
                }
            }
#endif //VIDEO_TELEPHONY_ENABLE
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Receive Unkenow Message :: msgType:0x%x\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_HungupHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            int *Index = (int *)(pdata->ril_cmd->data);
            KRIL_DEBUG(DBG_TRACE,"Current call Index:%d\n", *Index);
            CAPI2_CC_EndCall(GetNewTID(), GetClientID(), *Index);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_HungupWaitingOrBackgroundHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_CC_GetNextWaitCallIndex (GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_TRACE,"WaitIndex:%d\n", *rsp);

            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_EndNextHeldCall;
            }
            break;
        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_EndNextHeldCall;
            break;
        }

        case BCM_CC_EndNextHeldCall:
        {
            UInt8 *rsp = (UInt8*) capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_TRACE,"HeldIndex:%d\n", *rsp);

            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
                if(KRIL_GetIncomingCallIndex() != INVALID_CALL)
                {
                    CAPI2_CC_EndAllCalls(GetNewTID(), GetClientID());
                    KRIL_SetIncomingCallIndex(INVALID_CALL);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_HungupForegroundResumeBackgroundHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE,"handler state:%lu\n", pdata->handler_state);
            CAPI2_CC_GetAllActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            ALL_CALL_INDEX_t *rsp = (ALL_CALL_INDEX_t *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_ERROR, "rsp->listSz:%d\n", rsp->listSz);
            if (rsp->listSz != 0)
            {
                if (1 == rsp->listSz)
                    CAPI2_CC_EndCall(GetNewTID(), GetClientID(), rsp->indexList[0]);
                else
                    CAPI2_CC_EndMPTYCalls(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_SendCAPI2Cmd;
            }
            else
            {
                CAPI2_CC_GetNextWaitCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            }
            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "WaitCallIndex:%d\n", *rsp);
            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_AcceptVoiceCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_AcceptVoiceCall;
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_AcceptVoiceCall:
        {
            if(capi2_rsp->result == CC_ACCEPT_CALL_FAIL)
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "HeldCallIndex:%d\n", *rsp);
            if(*rsp != INVALID_CALL)
            {
                CAPI2_CC_RetrieveCall(GetNewTID(), GetClientID(), *rsp);
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
            KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd result:%d\n", capi2_rsp->result);
            if(capi2_rsp->result == CC_RESUME_CALL_SUCCESS)
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SwitchWaitingOrHoldingAndActiveHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp) //CHLD=2
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KRIL_SetInHoldCallHandler(TRUE);
            CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->ActiveIndex= *rsp;
            KRIL_DEBUG(DBG_INFO, "ActiveIndex:%d\n", context->ActiveIndex);
            CAPI2_CC_GetNextWaitCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextWaitCallIndex;
            break;
        }

        case BCM_CC_GetNextWaitCallIndex:
        {
            UInt8 *rsp = (UInt8 *) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->WaitIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "WaitIndex:%d\n", context->WaitIndex);
            if(context->ActiveIndex != INVALID_CALL || context->WaitIndex == INVALID_CALL )
            {
                CAPI2_CC_GetMPTYCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetMPTYCallIndex;
            }
            else
            {
                CAPI2_CC_AcceptWaitingCall(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            break;
        }

        case BCM_CC_GetMPTYCallIndex:
        {
            UInt8 *rsp = (UInt8*) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->MptyIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "MptyIndex:%d\n", context->MptyIndex);
            if(context->MptyIndex != INVALID_CALL)
            {
                if(INVALID_CALL != context->ActiveIndex)
                {
                    CAPI2_CC_SwapCall(GetNewTID(), GetClientID(), context->MptyIndex);
                    pdata->handler_state = BCM_CC_SwapCall;
                }
                else
                {
                    CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
                }
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            UInt8 *rsp = (UInt8*) capi2_rsp->dataBuf;
            KrilCallIndex_t *context = (KrilCallIndex_t *)pdata->cmdContext;
            context->HeldIndex = *rsp;
            KRIL_DEBUG(DBG_INFO, "HeldIndex:%d\n", context->HeldIndex);
            if(context->HeldIndex != INVALID_CALL)
            {
                if(context->ActiveIndex != INVALID_CALL)
                {
                    CAPI2_CC_SwapCall(GetNewTID(), GetClientID(), context->HeldIndex);
                    pdata->handler_state = BCM_CC_SwapCall;
                }
                else
                {
                    CAPI2_CC_RetrieveNextHeldCall(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
            }
            else
            {
                if(context->WaitIndex != INVALID_CALL)
                {
                    CAPI2_CC_AcceptWaitingCall(GetNewTID(), GetClientID());
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
                else
                {
                    CAPI2_CC_HoldCall(GetNewTID(), GetClientID(), context->ActiveIndex);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
            }
            break;
        }

        case BCM_CC_SwapCall:
        {
            if(capi2_rsp->result == RESULT_OK || capi2_rsp->result == CC_SWAP_CALL_SUCCESS)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t*) capi2_rsp->dataBuf;
                if(rsp->callResult == CC_RESUME_CALL_SUCCESS || 
                	 rsp->callResult == CC_SWAP_CALL_SUCCESS )
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
                KRIL_SetInHoldCallHandler(FALSE);
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t*) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                if(rsp->callResult == CC_ACCEPT_CALL_SUCCESS || 
                   rsp->callResult == CC_RESUME_CALL_SUCCESS || 
                   rsp->callResult == CC_SWAP_CALL_SUCCESS || 
                   rsp->callResult == CC_HOLD_CALL_SUCCESS || 
                   rsp->callResult == RESULT_OK)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::rsp->callResult:%d\n", rsp->callResult);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            KRIL_SetInHoldCallHandler(FALSE);
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_ConferenceHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "ActiveCallIndex:%d\n", *rsp);
            CAPI2_CC_JoinCall(GetNewTID(), GetClientID(), *rsp);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_UDUBHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "KRIL_GetIncomingCallIndex:%d KRIL_GetWaitingCallIndex:%d\n", KRIL_GetIncomingCallIndex(), KRIL_GetWaitingCallIndex());
            if(KRIL_GetWaitingCallIndex() != INVALID_CALL)
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), KRIL_GetWaitingCallIndex());
            }
            else
            {
                CAPI2_CC_EndCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
            }
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            KRIL_SetIncomingCallIndex(INVALID_CALL);
            KRIL_SetWaitingCallIndex(INVALID_CALL);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_LastCallFailCauseHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    KrilLastCallFailCause_t *rdata;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilLastCallFailCause_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilLastCallFailCause_t);
            memset(pdata->bcm_ril_rsp, 0, sizeof(KrilLastCallFailCause_t));

    // Return the cached failure cause.
    rdata = (KrilLastCallFailCause_t *)pdata->bcm_ril_rsp;
    rdata->failCause = KRIL_GetLastCallFailCause();
    pdata->handler_state = BCM_FinishCAPI2Cmd;
        }

//******************************************************************************
//
// Function Name: ResultToRilFailCause
//
// Description:   Converts the CAPI error code to a RIL last call failure cause.
//
// Notes:
//
//******************************************************************************
RIL_LastCallFailCause ResultToRilFailCause(Result_t inResult)
        {
    RIL_LastCallFailCause failCause;
    switch (inResult)
            {
        case CC_MAKE_CALL_SUCCESS:
        case RESULT_OK:
            failCause = CALL_FAIL_NORMAL;
                    break;

         case CC_FDN_BLOCK_MAKE_CALL:
            failCause = CALL_FAIL_FDN_BLOCKED;
                    break;

        // Don't distinguish between these failures;
        // the UI only requires RIL_LastCallFailCause
        // failure codes.
        case STK_DATASVRC_BUSY:
        case RESULT_DIALSTR_INVALID:
        case RESULT_SIM_NOT_READY:
        case CC_FAIL_MAKE_CALL:
        case CC_FAIL_CALL_SESSION:
        case CC_WRONG_CALL_TYPE:
                default:
            failCause = CALL_FAIL_ERROR_UNSPECIFIED;
            break;
        }

    KRIL_DEBUG(DBG_INFO, "CAPI2 result:%d failCause:%d\n", inResult, failCause);
    return failCause;
    }

void KRIL_AnswerHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            if(INVALID_CALL == KRIL_GetIncomingCallIndex())
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                CAPI2_CC_GetCallType (GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
                pdata->handler_state = BCM_CC_GetCallType;
            }
            break;
        }

        case BCM_CC_GetCallType:
        {
            CCallType_t *callTy = (CCallType_t *)capi2_rsp->dataBuf;

            pdata->handler_state = BCM_RESPCAPI2Cmd;
            if((*callTy == MTDATA_CALL) || (*callTy == MTFAX_CALL)) 
            {
                CAPI2_CC_AcceptDataCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
            } 
            else if(*callTy == MTVIDEO_CALL)
            { 
                CAPI2_CC_AcceptVideoCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
            }
            else if(*callTy == MTVOICE_CALL) 
            {
                CAPI2_CC_AcceptVoiceCall(GetNewTID(), GetClientID(), KRIL_GetIncomingCallIndex());
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Unknow call type:%d...!\n", *callTy);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
#ifdef VIDEO_TELEPHONY_ENABLE
            if (capi2_rsp->msgType == MSG_DATACALL_CONNECTED_IND)
            {
                DataCallConnectMsg_t *rsp = (DataCallConnectMsg_t *) capi2_rsp->dataBuf;
                int callIndex = rsp->callIndex;
                
                KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_CONNECTED_IND::callIndex: %d\n",rsp->callIndex);
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_CONNECT, &callIndex, sizeof(int));
                
            }
            else if (capi2_rsp->msgType == MSG_DATACALL_RELEASE_IND)
            {
                DataCallReleaseMsg_t *rsp = (DataCallReleaseMsg_t *) capi2_rsp->dataBuf;
                int callIndex = rsp->callIndex;
                
                KRIL_DEBUG(DBG_INFO, "MSG_DATACALL_RELEASE_IND:: callIndex: %d exitCause: 0x%X\n", rsp->callIndex, rsp->exitCause);
                KRIL_SendNotify(RIL_UNSOL_RESPONSE_VT_CALL_EVENT_END, &callIndex, sizeof(int));
            }
#endif //VIDEO_TELEPHONY_ENABLE

            pdata->bcm_ril_rsp = NULL;
            pdata->rsp_len = 0;
            KRIL_SetIncomingCallIndex(INVALID_CALL);
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SeparateConnectionHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSeparate_t *tdata = (KrilSeparate_t *)pdata->ril_cmd->data;
            tdata->index--;
            KRIL_DEBUG(DBG_INFO, "call index:%d\n", tdata->index);
            CAPI2_CC_SplitCall(GetNewTID(), GetClientID(), (UInt8) tdata->index);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_ExplicitCallTransferHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            CAPI2_CC_GetNextActiveCallIndex(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_CC_GetNextActiveCallIndex;
            break;
        }

        case BCM_CC_GetNextActiveCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "ActiveCallIndex:%d\n", *rsp);
            if (*rsp != INVALID_CALL)
            {
                CAPI2_CC_TransferCall(GetNewTID(), GetClientID(), *rsp);
                pdata->handler_state = BCM_CC_TransferCall;
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_TransferCall:
        {
            if(capi2_rsp->result == CC_TRANS_CALL_SUCCESS || capi2_rsp->result == RESULT_OK)
            {
                VoiceCallActionMsg_t *rsp = (VoiceCallActionMsg_t *)capi2_rsp->dataBuf;
                if(rsp->callResult == CC_TRANS_CALL_SUCCESS)
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                }
            }
            else
            {
                CAPI2_CC_GetNextHeldCallIndex(GetNewTID(), GetClientID());
                pdata->handler_state = BCM_CC_GetNextHeldCallIndex;
            }
            break;
        }

        case BCM_CC_GetNextHeldCallIndex:
        {
            UInt8 *rsp = (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "HeldCallIndex:%d\n", *rsp);
            CAPI2_CC_TransferCall(GetNewTID(), GetClientID(), *rsp);
            pdata->handler_state = BCM_CC_TransferCall;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_TRACE, "BCM_RESPCAPI2Cmd\n");
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SendDTMFRequestHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
 {
     KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

     if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), context->inCcCallState);
            pdata->handler_state = BCM_CcApi_SetDtmfTimer;
            break;
        }

        case BCM_CcApi_SetDtmfTimer:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->DTMFCallIndex = *(UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d\n", context->DTMFCallIndex);

            if (context->DTMFCallIndex != INVALID_CALL)
            {
                CAPI2_CcApi_SetDtmfTimer(GetNewTID(), GetClientID(), context->DTMFCallIndex, DTMF_TONE_DURATION_TYPE, 100);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), context->inCcCallState);
                pdata->handler_state = BCM_CcApi_SetDtmfTimer;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            ApiDtmf_t DtmfObjPtr;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;

            DTMFString = ((char *)pdata->ril_cmd->data)[0];
            DtmfObjPtr.callIndex = context->DTMFCallIndex;
            DtmfObjPtr.dtmfIndex = 0;
            DtmfObjPtr.dtmfTone = DTMFString;
            DtmfObjPtr.duration = 100;
            DtmfObjPtr.toneValume = 0;
            DtmfObjPtr.isSilent = FALSE;
            KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", context->DTMFCallIndex, DTMFString);
            CAPI2_CcApi_SendDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SendDTMFStartHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), context->inCcCallState);
            pdata->handler_state = BCM_CcApi_ResetDtmfTimer;
            break;
        }

        case BCM_CcApi_ResetDtmfTimer:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->DTMFCallIndex = * (UInt8 *)capi2_rsp->dataBuf;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d inCcCallState:%d\n", context->DTMFCallIndex, context->inCcCallState);
 
            if (context->DTMFCallIndex != INVALID_CALL)
            {
                CAPI2_CcApi_ResetDtmfTimer(GetNewTID(), GetClientID(), context->DTMFCallIndex, DTMF_TONE_DURATION_TYPE);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), CC_CALL_CONNECTED);
                pdata->handler_state = BCM_CcApi_ResetDtmfTimer;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            ApiDtmf_t DtmfObjPtr;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;

            DTMFString = ((char *)pdata->ril_cmd->data)[0];
            DtmfObjPtr.callIndex = context->DTMFCallIndex;
            DtmfObjPtr.dtmfIndex = 0;
            DtmfObjPtr.dtmfTone = DTMFString;
            DtmfObjPtr.duration = 0;
            DtmfObjPtr.toneValume = 0;
            DtmfObjPtr.isSilent = FALSE;
            KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", context->DTMFCallIndex, DTMFString);
            CAPI2_CcApi_SendDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}


void KRIL_SendDTMFStopHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lx::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            context->inCcCallState = CC_CALL_ACTIVE;
            CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), context->inCcCallState);
            pdata->handler_state = BCM_CcApi_SendDtmf;
            break;
        }

        case BCM_CcApi_SendDtmf:
        {
            UInt8 DTMFCallIndex = * (UInt8 *)capi2_rsp->dataBuf;
            KrilDTMFInfo_t *context = (KrilDTMFInfo_t *)pdata->cmdContext;
            KRIL_DEBUG(DBG_INFO, "CallIndex:%d\n", DTMFCallIndex);

            if (DTMFCallIndex != INVALID_CALL)
            {
                ApiDtmf_t DtmfObjPtr;
                DtmfObjPtr.callIndex = DTMFCallIndex;
                DtmfObjPtr.dtmfIndex = 0;
                DtmfObjPtr.dtmfTone = DTMFString;
                DtmfObjPtr.duration = 0;
                DtmfObjPtr.toneValume = 0;
                DtmfObjPtr.isSilent = FALSE;
                KRIL_DEBUG(DBG_INFO, "DTMFCallIndex:%d DTMFString:%c\n", DTMFCallIndex, DTMFString);
                CAPI2_CcApi_StopDtmf(GetNewTID(), GetClientID(), &DtmfObjPtr);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
            }
            else if (CC_CALL_ACTIVE == context->inCcCallState)
            {
                context->inCcCallState = CC_CALL_CONNECTED;
                CAPI2_CC_GetCallIndexInThisState(GetNewTID(), GetClientID(), context->inCcCallState);
                pdata->handler_state = BCM_CcApi_SendDtmf;
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_TRACE, "capi2_rsp->result:%d\n", capi2_rsp->result);
                pdata->result = RILErrorResult(capi2_rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            break;
        }

        default:
        {
            KRIL_DEBUG(DBG_ERROR,"Error handler_state:0x%lX\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}

