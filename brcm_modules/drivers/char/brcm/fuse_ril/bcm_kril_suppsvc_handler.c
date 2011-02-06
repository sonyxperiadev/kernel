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

CallIndex_t gUssdID = CALLINDEX_INVALID;
CallIndex_t gPreviousUssdID = CALLINDEX_INVALID;

typedef struct
{
    UInt8                         *name;
    SS_CallBarType_t   type;
} CBfacid_t;


//
// Call barring types array
//
SS_CallBarType_t ssBarringTypes[] =
{
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_OUT_ALL,				///< All outgoing 
  SS_CALLBAR_TYPE_OUT_INTL	,			///< Outgoing international
  SS_CALLBAR_TYPE_OUT_INTL_EXCL_HPLMN,	///< Outgoing international excluding HomePLMN (home country)
  SS_CALLBAR_TYPE_INC_ALL,				///< All incoming
  SS_CALLBAR_TYPE_INC_ROAM_OUTSIDE_HPLMN,	///< All incoming when roaming outside HomePLMN (home country)
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_NOTSPECIFIED,
  SS_CALLBAR_TYPE_ALL,					///< All calls(incoming and outgoing)
  SS_CALLBAR_TYPE_OUTG,					///< Outgoing
  SS_CALLBAR_TYPE_INC						///< Incoming	  
};

static const SS_CallFwdReason_t Kril_FwdReason[] =
{
    SS_CALLFWD_REASON_UNCONDITIONAL,
    SS_CALLFWD_REASON_BUSY,
    SS_CALLFWD_REASON_NO_REPLY,
    SS_CALLFWD_REASON_NOT_REACHABLE,
    SS_CALLFWD_REASON_ALL_CF,
    SS_CALLFWD_REASON_ALL_CONDITIONAL,
};

static const SS_CallFwdReason_t Kril_FwdMode[] =
{
    SS_MODE_DISABLE,
    SS_MODE_ENABLE,
    SS_MODE_INTERROGATE,
    SS_MODE_REGISTER,
    SS_MODE_ERASE,
};

/* ATC Service Class values defined in GSM 07.07 */
typedef enum
{
	ATC_NOT_SPECIFIED = 0,
	ATC_VOICE = 1,
	ATC_DATA = 2,
	ATC_FAX = 4,
	ATC_SMS = 8,
	ATC_DATA_CIRCUIT_SYNC = 16,
	ATC_DATA_CIRCUIT_ASYNC = 32,
	ATC_DPA = 64,
	ATC_DPAD = 128,
} ATC_CLASS_t;


SS_SvcCls_t GetServiceClass (int InfoClass)
{
    SS_SvcCls_t  svcCls = SS_SVCCLS_UNKNOWN;

    switch(InfoClass)
    {
        case 1:
            svcCls = SS_SVCCLS_SPEECH;
        break;

        case 2:
            svcCls = SS_SVCCLS_DATA;
        break;

        case 4:
            svcCls = SS_SVCCLS_FAX;
        break;

        case 5:
            svcCls = SS_SVCCLS_ALL_TELE_SERVICES;
        break;

        case 8:
            svcCls = SS_SVCCLS_SMS;
        break;

        case 10:
            svcCls = SS_SVCCLS_ALL_SYNC_SERVICES;
        break;

        case 11:
            svcCls = SS_SVCCLS_ALL_BEARER_SERVICES;
        break;

        case 13: // need to check
            svcCls = SS_SVCCLS_ALL_TELE_SERVICES;
        break;

        case 16:
            svcCls = SS_SVCCLS_DATA_CIRCUIT_SYNC;
        break;

        case 32:
            svcCls = SS_SVCCLS_DATA_CIRCUIT_ASYNC;
        break;

        case 64:
            svcCls = SS_SVCCLS_DEDICATED_PACKET;
        break;

        case 80:
            svcCls = SS_SVCCLS_ALL_SYNC_SERVICES;
        break;

        case 128:
            svcCls = SS_SVCCLS_DEDICATED_PAD;
        break;

        case 135 : 
            svcCls = SS_SVCCLS_ALL_TELESERVICE_EXCEPT_SMS;
        break;

        case 160:
            svcCls = SS_SVCCLS_ALL_ASYNC_SERVICES ;
        break;

        case 240:
            svcCls = SS_SVCCLS_ALL_BEARER_SERVICES;
        break;

        default:
            svcCls = SS_SVCCLS_NOTSPECIFIED;
    }
    return svcCls;
}


int SvcClassToATClass (SS_SvcCls_t InfoClass)
{
    ATC_CLASS_t ATCClass;

    switch((int)InfoClass)
    {
        case SS_SVCCLS_NOTSPECIFIED:
            ATCClass = ATC_NOT_SPECIFIED;
            break;

        case SS_SVCCLS_SPEECH:
        case SS_SVCCLS_ALT_SPEECH:
            ATCClass = ATC_VOICE;
            break;

        case SS_SVCCLS_ALL_TELE_SERVICES:
            ATCClass = ATC_VOICE | ATC_FAX | ATC_SMS;
            break;

        case SS_SVCCLS_ALL_TELESERVICE_EXCEPT_SMS:
            ATCClass = ATC_VOICE | ATC_FAX;
            break;

        case SS_SVCCLS_DATA:
            ATCClass = ATC_DATA;
            break;

        case SS_SVCCLS_FAX:
            ATCClass = ATC_FAX;
            break;

        case SS_SVCCLS_SMS:
            ATCClass = ATC_SMS;
            break;

        case SS_SVCCLS_DATA_CIRCUIT_SYNC:
            ATCClass = ATC_DATA_CIRCUIT_SYNC;
            break;

        case SS_SVCCLS_DATA_CIRCUIT_ASYNC:
            ATCClass = ATC_DATA_CIRCUIT_ASYNC;
            break;

        case SS_SVCCLS_DEDICATED_PACKET:
            ATCClass = ATC_DPA;
            break;

        case SS_SVCCLS_DEDICATED_PAD:
            ATCClass = ATC_DPAD;
            break;

        case SS_SVCCLS_ALL_SYNC_SERVICES:
            ATCClass = ATC_DATA_CIRCUIT_SYNC |ATC_DPA;
            break;

        case SS_SVCCLS_ALL_ASYNC_SERVICES:
            ATCClass = ATC_DATA_CIRCUIT_ASYNC |ATC_DPAD;
            break;

        case SS_SVCCLS_ALL_BEARER_SERVICES:
            ATCClass = ATC_DATA_CIRCUIT_SYNC | ATC_DATA_CIRCUIT_ASYNC |ATC_DPA |ATC_DPAD;
            break;

        default:
            ATCClass = ATC_NOT_SPECIFIED;
            break;
    }
    return ATCClass;
}


void KRIL_SendUSSDHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilSendUSSDInfo_t *tdata = (KrilSendUSSDInfo_t *)pdata->ril_cmd->data;
            if(tdata->StringSize <= 0)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            KRIL_DEBUG(DBG_INFO, "gUssdID:%d m_USSDString:%s used_size:%d\n", gUssdID, tdata->USSDString, tdata->StringSize);
            if(gUssdID == CALLINDEX_INVALID) //new USSD request
            {
                USSDString_t *ussd = kmalloc(sizeof(USSDString_t), GFP_KERNEL);
                pdata->bcm_ril_rsp = kmalloc(sizeof(KrilReceiveUSSDInfo_t), GFP_KERNEL);
                memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
                pdata->rsp_len = sizeof(KrilReceiveUSSDInfo_t);

                memset (ussd, 0x00, sizeof(USSDString_t));
                ussd->used_size = tdata->StringSize;
                ussd->dcs = 0x0F;
                memcpy((void*)ussd->string, (void*)tdata->USSDString, ussd->used_size);
                CAPI2_SS_SendUSSDConnectReq(GetNewTID(), GetClientID(), ussd);
                kfree(ussd);
            }
            else
            {
                pdata->bcm_ril_rsp = kmalloc(sizeof(KrilReceiveUSSDInfo_t), GFP_KERNEL);
                memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
                pdata->rsp_len = sizeof(KrilReceiveUSSDInfo_t);
                CAPI2_SS_SendUSSDData(GetNewTID(), GetClientID(), gUssdID, 0x0F, tdata->StringSize, (UInt8 *) tdata->USSDString);
            }
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                int i;
                USSDataInfo_t *rsp = (USSDataInfo_t *) capi2_rsp->dataBuf;
                KrilReceiveUSSDInfo_t *rdata = (KrilReceiveUSSDInfo_t *) pdata->bcm_ril_rsp;

                KRIL_DEBUG(DBG_INFO, "call_index:%d service_type:%d oldindex:%d newindex:%d prob_tag:%d prob_code:%d err_code:%d code_type:0x%x used_size:%d\n", rsp->call_index,rsp->ussd_data.service_type,rsp->ussd_data.oldindex,rsp->ussd_data.newindex,rsp->ussd_data.prob_tag,
                rsp->ussd_data.prob_code,rsp->ussd_data.err_code,rsp->ussd_data.code_type,rsp->ussd_data.used_size);

                if (USSD_REQUEST == rsp->ussd_data.service_type || 
                	   USSD_REGISTRATION == rsp->ussd_data.service_type)
                {
                    rdata->type = 1;
                }
                else if (USSD_NOTIFICATION == rsp->ussd_data.service_type)
                {
                    rdata->type = 0;
                }
                else if (USSD_RELEASE_COMPLETE_RETURN_RESULT == rsp->ussd_data.service_type)
                {
                    rdata->type = 2;
                    if (rsp->ussd_data.used_size < 0)
                    {
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                        return;
                    }
                }
                else if (USSD_FACILITY_RETURN_RESULT == rsp->ussd_data.service_type)
                {
                    if (rsp->ussd_data.used_size > 0)
                    {
                        rdata->type = 1;
                    }
                    if (rsp->ussd_data.used_size < 0)
                    {
                        rdata->type = 0;
                        memset(rdata->USSDString, 0, PHASE1_MAX_USSD_STRING_SIZE+1);
                        pdata->handler_state = BCM_FinishCAPI2Cmd;
                        return;
                    }
                }
                else if (USSD_FACILITY_REJECT == rsp->ussd_data.service_type ||
                            USSD_RELEASE_COMPLETE_REJECT == rsp->ussd_data.service_type)
                {
                    gUssdID = CALLINDEX_INVALID;
                    rdata->type = 4;
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                    return;
                }
                else if (USSD_FACILITY_RETURN_ERROR == rsp->ussd_data.service_type ||
                            USSD_RELEASE_COMPLETE_RETURN_ERROR == rsp->ussd_data.service_type)
                {
                    gUssdID = CALLINDEX_INVALID;
                    rdata->type = 5;
                }
                else if (USSD_RESEND == rsp->ussd_data.service_type)
                {
                    if (gUssdID == rsp->ussd_data.oldindex)
                    {
                        KrilSendUSSDInfo_t *tdata = (KrilSendUSSDInfo_t *)pdata->ril_cmd->data;
                        gUssdID = rsp->ussd_data.newindex;
                        CAPI2_SS_SendUSSDData(GetNewTID(), GetClientID(), gUssdID, 0x0F, tdata->StringSize, (UInt8 *) tdata->USSDString);
                    }
                    else
                    {
                        gUssdID = CALLINDEX_INVALID;
                        gPreviousUssdID = CALLINDEX_INVALID;
                        pdata->handler_state = BCM_ErrorCAPI2Cmd;
                   }
                    return;
                }
                rdata->Length = rsp->ussd_data.used_size;
                rdata->codetype = rsp->ussd_data.code_type;

                for (i = 0 ; i < rdata->Length ; i++)
                    KRIL_DEBUG(DBG_TRACE2, "string:0x%x\n", rsp->ussd_data.string[i]);

                memset(rdata->USSDString, 0, PHASE1_MAX_USSD_STRING_SIZE+1);
                memcpy(rdata->USSDString, rsp->ussd_data.string, rdata->Length);
                pdata->handler_state = BCM_FinishCAPI2Cmd;
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


void KRIL_CancelUSSDHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KRIL_DEBUG(DBG_INFO, "gUssdID:%d\n", gUssdID);
            if(gUssdID != CALLINDEX_INVALID)
            {
                CAPI2_SS_EndUSSDConnectReq(GetNewTID(), GetClientID(), gUssdID);
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
            if (capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            gUssdID = CALLINDEX_INVALID;

            if(gPreviousUssdID != CALLINDEX_INVALID)
            {
                gUssdID = gPreviousUssdID;
                gPreviousUssdID = CALLINDEX_INVALID;
            } 
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


void KRIL_GetCLIRHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCLIRInfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCLIRInfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_SS_QueryCallingLineRestrictionStatus(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_SS_GetElement;
        }
        break;

        case BCM_SS_GetElement:
        {
            KrilCLIRInfo_t *rdata = (KrilCLIRInfo_t *) pdata->bcm_ril_rsp;

            if (capi2_rsp->result != RESULT_OK)
            {
                rdata->value2 = SS_SERVICE_STATUS_UKNOWN;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                SS_ProvisionStatus_t *rsp = (SS_ProvisionStatus_t*) capi2_rsp->dataBuf;
                if(rsp->netCause == GSMCAUSE_SUCCESS)
                {
                    KRIL_DEBUG(DBG_INFO, "provision_status:%d serviceStatus:%d netCause:%d\n", rsp->provision_status, rsp->serviceStatus, rsp->netCause);
                    rdata->value2 = rsp->serviceStatus;
                    // Get the CLIR value from CAPI.
                    CAPI2_MS_GetElement(GetNewTID(), GetClientID(), MS_LOCAL_SS_ELEM_CLIR);
                    pdata->handler_state = BCM_RESPCAPI2Cmd;
                }
                else
                {
                    rdata->value2 = SS_SERVICE_STATUS_UKNOWN;
                    KRIL_DEBUG(DBG_INFO, "netCause:%d \n", rsp->netCause);
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                }
            }
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            CAPI2_MS_Element_t *rsp = (CAPI2_MS_Element_t *) capi2_rsp->dataBuf;
            KrilCLIRInfo_t *rdata = (KrilCLIRInfo_t *) pdata->bcm_ril_rsp;

            rdata->value1 = rsp->data_u.u8Data;
            KRIL_DEBUG(DBG_INFO, "CLIR:%d\n", rdata->value1);
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


void KRIL_SetCLIRHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            KrilCLIRValue_t *tdata = (KrilCLIRValue_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "value:%d\n", tdata->value);
            if(tdata->value == 1)
            {
                CAPI2_SS_SetCallingLineRestrictionStatus(GetNewTID(), GetClientID(), CLIRMODE_INVOKED);
            }
            else if(tdata->value == 2)
            {
                CAPI2_SS_SetCallingLineRestrictionStatus(GetNewTID(), GetClientID(), CLIRMODE_SUPPRESSED);
            }
            else if(tdata->value == 0)
            {
                CAPI2_SS_SetCallingLineRestrictionStatus(GetNewTID(), GetClientID(), CLIRMODE_DEFAULT);
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
            pdata->handler_state = BCM_SS_SetElement;
        }
        break;

        case BCM_SS_SetElement:
        {
            KrilCLIRValue_t *tdata = (KrilCLIRValue_t *)pdata->ril_cmd->data;

            if(capi2_rsp->result != RESULT_OK)
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else if(capi2_rsp->msgType == MSG_SS_CALL_REQ_FAIL)
            {
                SsCallReqFail_t *rsp = (SsCallReqFail_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "result:%d\n", rsp->result);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
                MS_LocalElemNotifyInd_t *rsp = (MS_LocalElemNotifyInd_t *) capi2_rsp->dataBuf;
                CAPI2_MS_Element_t data;
                KRIL_DEBUG(DBG_INFO, "elementType:%d\n", rsp->elementType);
                memset((UInt8*)&data, 0x00, sizeof(CAPI2_MS_Element_t));
                data.inElemType = MS_LOCAL_SS_ELEM_CLIR;
                data.data_u.u8Data = tdata->value;
                CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
                pdata->handler_state = BCM_RESPCAPI2Cmd;
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



void KRIL_QueryCallForwardStatusHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilCallForwardStatus_t *tdata = (KrilCallForwardStatus_t *)pdata->ril_cmd->data;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCallForwardinfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCallForwardinfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            KRIL_DEBUG(DBG_INFO, "Kril_FwdReason[%d]:%d ss_class:%d\n", tdata->reason, Kril_FwdReason[tdata->reason], GetServiceClass(tdata->ss_class));
            CAPI2_SS_QueryCallForwardStatus(GetNewTID(), GetClientID(), Kril_FwdReason[tdata->reason], GetServiceClass(tdata->ss_class));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "msgType:0x%x\n", capi2_rsp->msgType);
            if (capi2_rsp->msgType == MSG_SS_CALL_FORWARD_RSP)
            {
                NetworkCause_t *rsp = (NetworkCause_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_ERROR, "MSG_SS_CALL_FORWARD_RSP::netCause:%d\n", *rsp);
                if(*rsp != GSMCAUSE_SUCCESS)
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else if (capi2_rsp->msgType == MSG_SS_CALL_FORWARD_STATUS_RSP)
            {
                CallForwardStatus_t *rsp = (CallForwardStatus_t *) capi2_rsp->dataBuf;
                KrilCallForwardinfo_t *rdata = (KrilCallForwardinfo_t *)pdata->bcm_ril_rsp;
                KRIL_DEBUG(DBG_INFO, "netCause:%d reason:%d\n", rsp->netCause, rsp->reason);
                if(rsp->netCause == GSMCAUSE_SUCCESS)
                {
                    int i;
                    rdata->class_size = rsp->class_size;
                    rdata->reason = rsp->reason;
                    KRIL_DEBUG(DBG_INFO, "class_size:%d\n", rsp->class_size);
                    for(i = 0 ; i < rsp->class_size ; i++)
                    {
                        rdata->call_forward_class_info_list[i].activated = rsp->call_forward_class_info_list[i].activated;
                        rdata->call_forward_class_info_list[i].ss_class = SvcClassToATClass(rsp->call_forward_class_info_list[i].ss_class);
                        rdata->call_forward_class_info_list[i].ton = rsp->call_forward_class_info_list[i].forwarded_to_number.ton;
                        rdata->call_forward_class_info_list[i].npi = rsp->call_forward_class_info_list[i].forwarded_to_number.npi;
                        strcpy(rdata->call_forward_class_info_list[i].number, rsp->call_forward_class_info_list[i].forwarded_to_number.number);
                        rdata->call_forward_class_info_list[i].noReplyTime = rsp->call_forward_class_info_list[i].noReplyTime;
                        KRIL_DEBUG(DBG_INFO, "activated:%d ss_class:%d noReplyTime:%d\n", rsp->call_forward_class_info_list[i].activated, rsp->call_forward_class_info_list[i].ss_class, rsp->call_forward_class_info_list[i].noReplyTime);
                        KRIL_DEBUG(DBG_INFO, "ton:%d npi:%d number:%s\n", rsp->call_forward_class_info_list[i].forwarded_to_number.ton, rsp->call_forward_class_info_list[i].forwarded_to_number.npi, rsp->call_forward_class_info_list[i].forwarded_to_number.number);
                    }
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
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


void KRIL_SetCallForwardStatusHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilCallForwardStatus_t *tdata = (KrilCallForwardStatus_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "Kril_FwdMode[%d]:%d Kril_FwdReason[%d]:%d ss_class:%d timeSeconds:%d number:%s\n", tdata->mode, Kril_FwdMode[tdata->mode], tdata->reason, Kril_FwdReason[tdata->reason], GetServiceClass(tdata->ss_class), tdata->timeSeconds, tdata->number);
            CAPI2_SS_SendCallForwardReq(GetNewTID(), GetClientID(), Kril_FwdMode[tdata->mode], Kril_FwdReason[tdata->reason], GetServiceClass(tdata->ss_class), tdata->timeSeconds, (UInt8 *)tdata->number);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "msgType:0x%x\n", capi2_rsp->msgType);
            if (capi2_rsp->msgType == MSG_SS_CALL_FORWARD_RSP)
            {
                NetworkCause_t *rsp = (NetworkCause_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_ERROR, "MSG_SS_CALL_FORWARD_RSP::netCause:%d\n", *rsp);
                if(*rsp != GSMCAUSE_SUCCESS)
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else if (capi2_rsp->msgType == MSG_SS_CALL_FORWARD_STATUS_RSP)
            {
                 CallForwardStatus_t *rsp = (CallForwardStatus_t *) capi2_rsp->dataBuf;
                 KRIL_DEBUG(DBG_INFO, "netCause:%d reason:%d\n", rsp->netCause, rsp->reason);
                 if(rsp->netCause == GSMCAUSE_SUCCESS)
                 {
                     int i;
                     KRIL_DEBUG(DBG_INFO, "class_size:%d\n", rsp->class_size);
                     for(i = 0 ; i < rsp->class_size ; i++)
                     {
                         KRIL_DEBUG(DBG_INFO, "activated:%d ss_class:%d noReplyTime:%d\n", rsp->call_forward_class_info_list[i].activated, rsp->call_forward_class_info_list[i].ss_class, rsp->call_forward_class_info_list[i].noReplyTime);
                         KRIL_DEBUG(DBG_INFO, "ton:%d npi:%d number:%s\n", rsp->call_forward_class_info_list[i].forwarded_to_number.ton, rsp->call_forward_class_info_list[i].forwarded_to_number.npi, rsp->call_forward_class_info_list[i].forwarded_to_number.number);
                     }
                 }
                 else
                 {
                     pdata->handler_state = BCM_ErrorCAPI2Cmd;
                     return;
                 }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR, "Receive error MsgType:0x%x...!\n", capi2_rsp->msgType);
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
				return;
            }
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

void KRIL_QueryCallWaitingHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilCallWaitingInfo_t *tdata = (KrilCallWaitingInfo_t *)pdata->ril_cmd->data;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCallWaitingClass_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCallWaitingClass_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            KRIL_DEBUG(DBG_INFO, "GetServiceClass(%d):%d\n", tdata->ss_class, GetServiceClass(tdata->ss_class));
            CAPI2_SS_QueryCallWaitingStatus(GetNewTID(), GetClientID(), tdata->ss_class);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "msgType:0x%x\n", capi2_rsp->msgType);
            if (capi2_rsp->msgType == MSG_SS_CALL_WAITING_RSP)
            {
                NetworkCause_t *rsp = (NetworkCause_t *) capi2_rsp->dataBuf;
                if(*rsp != GSMCAUSE_SUCCESS)
                {
                    KRIL_DEBUG(DBG_ERROR, "MSG_SS_CALL_WAITING_RSP::netCause:%d\n", *rsp);
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else if (capi2_rsp->msgType == MSG_SS_CALL_WAITING_STATUS_RSP)
            {
                SS_ActivationStatus_t *rsp = (SS_ActivationStatus_t *) capi2_rsp->dataBuf;
                KrilCallWaitingInfo_t *tdata = (KrilCallWaitingInfo_t *)pdata->ril_cmd->data;
                KrilCallWaitingClass_t *rdata = (KrilCallWaitingClass_t *)pdata->bcm_ril_rsp;
                KRIL_DEBUG(DBG_INFO, "netCause:%d\n", rsp->netCause);
                if(rsp->netCause == GSMCAUSE_SUCCESS)
                {
                    int i;
                    KRIL_DEBUG(DBG_INFO, "class_size:%d\n", rsp->class_size);
                    for(i = 0 ; i < rsp->class_size ; i++)
                    {
                        KRIL_DEBUG(DBG_INFO, "activated:%d ss_class:%d\n", rsp->ss_activation_class_info[i].activated,  rsp->ss_activation_class_info[i].ss_class);
                        if (TRUE == rsp->ss_activation_class_info[i].activated)
                        {
                            if ((SvcClassToATClass(rsp->ss_activation_class_info[i].ss_class) == tdata->ss_class) || (SS_SVCCLS_NOTSPECIFIED == tdata->ss_class))
                            {
                                rdata->activated = 1;
                                rdata->ss_class |= SvcClassToATClass(rsp->ss_activation_class_info[i].ss_class);
                            }
                        }
                    }
                    KRIL_DEBUG(DBG_INFO, "Final state activated:%d ss_class:%d\n", rdata->activated, rdata->ss_class);
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
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


void KRIL_SetCallWaitingHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilCallWaitingInfo_t *tdata = (KrilCallWaitingInfo_t *)pdata->ril_cmd->data;
            SS_Mode_t SS_Mode;
            KRIL_DEBUG(DBG_INFO, "state:%d GetServiceClass(%d):%d\n", tdata->state, tdata->ss_class, GetServiceClass(tdata->ss_class));
            if (1 == tdata->state) 
                SS_Mode  = SS_MODE_ENABLE;
            else 
                SS_Mode  = SS_MODE_DISABLE;
            CAPI2_SS_SendCallWaitingReq(GetNewTID(), GetClientID(), SS_Mode, GetServiceClass(tdata->ss_class));
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KRIL_DEBUG(DBG_INFO, "msgType:0x%x\n", capi2_rsp->msgType);
            if (capi2_rsp->msgType == MSG_SS_CALL_WAITING_RSP)
            {
                NetworkCause_t *rsp = (NetworkCause_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_ERROR, "MSG_SS_CALL_FORWARD_RSP::netCause:%d\n", *rsp);
                if(*rsp != GSMCAUSE_SUCCESS)
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else if (capi2_rsp->msgType == MSG_SS_CALL_WAITING_STATUS_RSP)
            {
                SS_ActivationStatus_t *rsp = (SS_ActivationStatus_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "netCause:%d\n", rsp->netCause);
                if(rsp->netCause == GSMCAUSE_SUCCESS)
                {
                    int i;
                    KRIL_DEBUG(DBG_INFO, "class_size:%d\n", rsp->class_size);
                    for(i = 0 ; i < rsp->class_size ; i++)
                    {
                        KRIL_DEBUG(DBG_INFO, "activated:%d ss_class:%d\n", rsp->ss_activation_class_info[i].activated,  rsp->ss_activation_class_info[i].ss_class);
                    }
                }
                else
                {
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    return;
                }
            }
            else
            {
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                return;
            }
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


void KRIL_ChangeBarringPasswordHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            KrilCallBarringPasswd_t *tdata = (KrilCallBarringPasswd_t *)pdata->ril_cmd->data;
            KRIL_DEBUG(DBG_INFO, "fac_id:%d OldPasswd:%s NewPasswd:%s\n", tdata->fac_id, tdata->OldPasswd, tdata->NewPasswd);
            CAPI2_SS_SendCallBarringPWDChangeReq(GetNewTID(), GetClientID(), ssBarringTypes[tdata->fac_id], (UInt8*) tdata->OldPasswd, (UInt8*) tdata->NewPasswd, (UInt8*) tdata->NewPasswd);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
                NetworkCause_t *presult = (NetworkCause_t *) capi2_rsp->dataBuf;
                KRIL_DEBUG(DBG_INFO, "NetworkCause:%d\n", *presult);
            pdata->handler_state = BCM_FinishCAPI2Cmd;

                if(GSMCAUSE_SUCCESS != *presult)
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


void KRIL_QueryCLIPHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilCLIPInfo_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilCLIPInfo_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            CAPI2_SS_QueryCallingLineIDStatus(GetNewTID(), GetClientID());
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            KrilCLIPInfo_t *rdata = (KrilCLIPInfo_t *)pdata->bcm_ril_rsp;
            if(capi2_rsp->result != RESULT_OK)
            {
                KRIL_DEBUG(DBG_INFO, "LastCLIP:%d\n", KRIL_GetCLIPValue());
                rdata->value = SS_SERVICE_STATUS_UKNOWN;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else         
            {
                SS_ProvisionStatus_t *rsp = (SS_ProvisionStatus_t*) capi2_rsp->dataBuf;
                if(rsp->netCause == GSMCAUSE_SUCCESS)
                {
                    KRIL_DEBUG(DBG_INFO, "provision_status:%d serviceStatus:%d netCause:%d\n", rsp->provision_status, rsp->serviceStatus, rsp->netCause);
                    rdata->value = rsp->serviceStatus;
                }
                else
                {
                    rdata->value = SS_SERVICE_STATUS_UKNOWN;
                    KRIL_DEBUG(DBG_INFO, "netCause:%d LastCLIP:%d\n", rsp->netCause, KRIL_GetCLIPValue());
                }
                pdata->handler_state = BCM_FinishCAPI2Cmd;
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

void KRIL_SetSuppSvcNotificationHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
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
        // **FIXME** MAG - MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH not currently included in CIB; need to be integrated from 2157?
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
        case BCM_SendCAPI2Cmd:
        {
            int *iEnable = (int *)(pdata->ril_cmd->data);
            CAPI2_MS_Element_t data;
            memset((UInt8*)&data, 0, sizeof(CAPI2_MS_Element_t));
            data.inElemType = MS_LOCAL_SS_ELEM_NOTIFICATION_SWITCH;
            data.data_u.u8Data = *iEnable;
            KRIL_DEBUG(DBG_INFO, "iEnable:%d\n", *iEnable);
            CAPI2_MS_SetElement(GetNewTID(), GetClientID(), &data);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
            break;
        }

        case BCM_RESPCAPI2Cmd:
        {
            pdata->handler_state = BCM_FinishCAPI2Cmd;
            break;
        }
#endif
        default:
        {
            KRIL_DEBUG(DBG_ERROR, "handler_state:%lu error...!\n", pdata->handler_state);
            pdata->handler_state = BCM_ErrorCAPI2Cmd;
            break;
        }
    }
}
