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
#include "capi2_pch_api.h"
#include "capi2_reqrep.h"
#include "capi2_gen_api.h"
#endif // CONFIG_BRCM_FUSE_RIL_CIB

KrilDataCallResponse_t pdp_resp[BCM_NET_MAX_PDP_CNTXS] = {0};

static int ParsePdpFailCause(Result_t value)
{
    RIL_LastDataCallActivateFailCause cause;
    switch (value)
    {
        case PDP_OPERATOR_DETERMINED_BARRING:
            cause = PDP_FAIL_OPERATOR_BARRED;
            break;

        case PDP_INSUFFICIENT_RESOURCES:
            cause = PDP_FAIL_INSUFFICIENT_RESOURCES;
            break;

        case PDP_MISSING_OR_UNKNOWN_APN:
            cause = PDP_FAIL_MISSING_UKNOWN_APN;
            break;

        case PDP_UNKNOWN_PDP_ADDRESS:
            cause = PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE;
            break;

        case PDP_USER_AUTH_FAILED:
            cause = PDP_FAIL_USER_AUTHENTICATION;
            break;

        case PDP_ACTIVATION_REJECTED_BY_GGSN:
            cause = PDP_FAIL_ACTIVATION_REJECT_GGSN;
            break;

        case PDP_ACTIVATION_REJECTED_UNSPECIFIED:
            cause = PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED;
            break;

        case PDP_SERVICE_OPT_NOT_SUPPORTED:
            cause = PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED;
            break;

        case PDP_REQ_SERVICE_NOT_SUBSCRIBED:
            cause = PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED;
            break;

        case PDP_SERVICE_TEMP_OUT_OF_ORDER:
            cause = PDP_FAIL_SERVICE_OPTION_OUT_OF_ORDER;
            break;

        case PDP_NSAPI_ALREADY_USED:
            cause = PDP_FAIL_NSAPI_IN_USE;
            break;

        case PDP_PROTOCOL_ERROR_UNSPECIFIED:
            cause = PDP_FAIL_PROTOCOL_ERRORS;
            break;

        default:
            cause = PDP_FAIL_ERROR_UNSPECIFIED;
            break;
    }
    return (int)cause;
}

static UInt8 GetFreePdpContext(void)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
    {
        if (0 == pdp_resp[i].cid)
        {
            pdp_resp[i].cid = i+1;
            KRIL_DEBUG(DBG_INFO, "GetFreePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_PDP_CNTXS;
}


static UInt8 ReleasePdpContext(UInt8 cid)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            pdp_resp[i].cid = 0;
            //memset(&pdp_resp[i], 0, sizeof(KrilDataCallResponse_t));
            KRIL_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
            return i;
        }
    }
    return BCM_NET_MAX_PDP_CNTXS;
}


static void FillDataResponseTypeApn(UInt8 cid, char* type, char* apn)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            if (NULL != type)
                strcpy(pdp_resp[i].type, type);
            if (NULL != apn)
                strcpy(pdp_resp[i].apn, apn);
            KRIL_DEBUG(DBG_INFO, "FillDataResponseTypeApn[%d]=[%s][%s] \n", i, pdp_resp[i].type, pdp_resp[i].apn);
            return;
        }
    }
}


static void FillDataResponseAddress(UInt8 cid, char* address)
{
    UInt8 i;
    for (i=0; i<BCM_NET_MAX_PDP_CNTXS; i++)
    {
        if (cid == pdp_resp[i].cid)
        {
            pdp_resp[i].active = 2; // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (NULL != address)
                strcpy(pdp_resp[i].address, address);
            KRIL_DEBUG(DBG_INFO, "FillDataResponseAddress[%d]=[%s] \n", i, pdp_resp[i].address);
            return;
        }
    }
}


void KRIL_SetupPdpHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
    static KrilPdpContext_t gContext;

    if (NULL != capi2_rsp)
        KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            UInt8 pindex;
            char chPdpType[PDP_TYPE_LEN_MAX] = "IP";

            if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail with NULL data\n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if (BCM_NET_MAX_PDP_CNTXS == (pindex = GetFreePdpContext()))
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail with over max cid[%d]\n", pindex);
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            memcpy(&gContext, (KrilPdpContext_t *)(pdata->ril_cmd->data), sizeof(KrilPdpContext_t));
            KRIL_DEBUG(DBG_INFO, "KRIL_SetupPdpHandler - Set PDP Context : apn %s \n", gContext.apn);

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            {
                ClientInfo_t clientInfo;
                UInt8 numParms = (gContext.apn == NULL)?2:3;
                CAPI2_InitClientInfo(&clientInfo, GetNewTID(), GetClientID());
                gContext.cid = pdp_resp[pindex].cid;
                FillDataResponseTypeApn(gContext.cid, chPdpType, gContext.apn);
                KRIL_DEBUG(DBG_INFO,"**Calling CAPI2_PdpApi_SetPDPContext numParms %d type:%s apn:%s pindex %d cid %d\n",numParms, chPdpType, (gContext.apn==NULL)?"NULL":gContext.apn, pindex,pdp_resp[pindex].cid  );
            
                CAPI2_PdpApi_SetPDPContext( &clientInfo, 
                                            pdp_resp[pindex].cid, 
                                            numParms, 
                                            chPdpType, 
                                            gContext.apn, 
                                            "", 
                                            0, 
                                            0);
            }
#else
            {
                PDPContext_t tContext;
                memset(&tContext, 0, sizeof(tContext));

                /*1 or 2*/
                tContext.cid = pdp_resp[pindex].cid;

                gContext.cid = tContext.cid;
                strcpy(tContext.pdpType, chPdpType);
                if (NULL != gContext.apn)
                    strcpy(tContext.apn, gContext.apn);

                FillDataResponseTypeApn(gContext.cid, chPdpType, gContext.apn);

                tContext.pchXid.dataComp = FALSE;
                tContext.pchXid.hdrComp = FALSE;

                CAPI2_PDP_SetPDPContext(GetNewTID(), GetClientID(), tContext.cid, &tContext);
            }
#endif // CONFIG_BRCM_FUSE_RIL_CIB
  
            pdata->handler_state = BCM_PDP_SetPdpContext;
        }
        break;

        case BCM_PDP_SetPdpContext:
        {
            PCHProtConfig_t t_PCHP;
            char *username = gContext.username;
            char *password = gContext.password;
            IPConfigAuthType_t t_Authtype = REQUIRE_PAP;/*default setting*/
            //UInt8 ContextID = 1;
            KRIL_DEBUG(DBG_INFO, "KRIL_SetupPdpHandler - Activate PDP context \n");

            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail to SetPDPContext[%d]\n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if (AUTH_PAP == gContext.authtype)
                t_Authtype = REQUIRE_PAP;
            else if (AUTH_CHAP == gContext.authtype)
                t_Authtype = REQUIRE_CHAP;

            memset(&t_PCHP, 0, sizeof(PCHProtConfig_t));
            // **FIXME** MAG - update this to use CAPI2_PCHEx_BuildIpConfigOptions() when api
            // is fixed in CIB CP (likely .30 release or so)
            Capi2BuildIpConfigOptions(&t_PCHP, username, password, t_Authtype);
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            {
                ClientInfo_t clientInfo;
                CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                CAPI2_PchExApi_SendPDPActivateReq( &clientInfo, gContext.cid, ACTIVATE_MMI_IP_RELAY, &t_PCHP );
            }
#else
            CAPI2_PDP_SendPDPActivateReq(GetNewTID(), GetClientID(), gContext.cid, ACTIVATE_MMI_IP_RELAY, &t_PCHP);
#endif // CONFIG_BRCM_FUSE_RIL_CIB
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
            UInt32 u_pDNS1, u_sDNS1, u_pDNS2, u_sDNS2, u_act_pDNS, u_act_sDNS;
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilPdpData_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);

            KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail to SendPDPActivateReq[%d] \n", gContext.cid);
                ReleasePdpContext(gContext.cid);
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPActivateReq_Rsp_t *rsp = (PDP_SendPDPActivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KrilPdpData_t *rdata = pdata->bcm_ril_rsp;

                if((rsp->cause != RESULT_OK) || (rsp->response != PCH_REQ_ACCEPTED))
                {
                    KRIL_DEBUG(DBG_ERROR, "PDPActivate Fail cause %d, resp(1 accept) %d, cid %d\r\n",
                        rsp->cause, rsp->response, rsp->activatedContext.cid);
                    ReleasePdpContext(gContext.cid);
                    rdata->cause = ParsePdpFailCause(rsp->cause);
                    pdata->result = RIL_E_RADIO_NOT_AVAILABLE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

                if (NULL != rsp->activatedContext.pdpAddress)
                {
                    memcpy(rdata->pdpAddress, rsp->activatedContext.pdpAddress, PDP_ADDRESS_LEN_MAX);
                    KRIL_DEBUG(DBG_INFO, "PDP Activate: PDP Address %s \r\n", rdata->pdpAddress);
                }

                u_pDNS1 = u_sDNS1 = u_pDNS2 = u_sDNS2 = u_act_pDNS = u_act_sDNS = 0;
                Capi2ReadDnsSrv(&(rsp->activatedContext.protConfig), &u_pDNS1, &u_sDNS1, &u_pDNS2, &u_sDNS2);

                KRIL_DEBUG(DBG_INFO, "PDP Activate: pDns1 0x%x, sDns1 0x%x, pDns2 0x%x, sDns2 0x%x \r\n",
                    (unsigned int)u_pDNS1, (unsigned int)u_sDNS1, (unsigned int)u_pDNS2, (unsigned int)u_sDNS2);

                if((u_act_pDNS = u_pDNS1) == 0)
                {
                    u_act_pDNS = u_pDNS2;
                }
                if((u_act_sDNS = u_sDNS1) == 0)
                {
                    u_act_sDNS = u_sDNS2;
                }
                rdata->priDNS = u_act_pDNS;
                rdata->secDNS = u_act_sDNS;
                rdata->cid = rsp->activatedContext.cid;
                KRIL_DEBUG(DBG_INFO, "PDP Activate Resp - cid %d \n", rsp->activatedContext.cid);
                FillDataResponseAddress(rdata->cid, rdata->pdpAddress);

                pdata->result = RIL_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RIL_E_GENERIC_FAILURE;
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



void KRIL_DeactivatePdpHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
    KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;

    if (NULL != capi2_rsp)
        KRIL_DEBUG(DBG_INFO, "BCM_RESPCAPI2Cmd::result:0x%x\n", capi2_rsp->result);

    switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
            char *cid = (char *)(pdata->ril_cmd->data);
            UInt8 ContextID = (UInt8)(*cid - 0x30);
            UInt8 i;

            KRIL_DEBUG(DBG_INFO, "KRIL_DeactivatePdpHandler - length %d, Cid:%d \n", pdata->ril_cmd->datalen, ContextID);
            pdata->bcm_ril_rsp = kmalloc(sizeof(KrilPdpData_t), GFP_KERNEL);
            pdata->rsp_len = sizeof(KrilPdpData_t);
            memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
            for (i=0 ; i<BCM_NET_MAX_PDP_CNTXS ; i++)
            {
                if (ContextID == pdp_resp[i].cid) // To find the contextID in pdp_resp list, need to deactivate the context
                {
                    KRIL_DEBUG(DBG_INFO, "ReleasePdpContext[%d]=%d \n", i, pdp_resp[i].cid);
                    break;
                }
                else if ((BCM_NET_MAX_PDP_CNTXS-1) == i) // Return finish state if can't find the contestID in pdp_resp list.
                {
                    pdata->handler_state = BCM_FinishCAPI2Cmd;
                    return;
                }
             }
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            {
                ClientInfo_t clientInfo;
                CAPI2_InitClientInfo( &clientInfo, GetNewTID(), GetClientID());
                CAPI2_PchExApi_SendPDPDeactivateReq( &clientInfo, ContextID );
            }
#else
            CAPI2_PDP_SendPDPDeactivateReq(GetNewTID(), GetClientID(), ContextID);
#endif  // CONFIG_BRCM_FUSE_RIL_CIB
            ReleasePdpContext(ContextID);
            pdata->handler_state = BCM_RESPCAPI2Cmd;
        }
        break;

        case BCM_RESPCAPI2Cmd:
        {
        	   KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "PDPDeActivate Fail to SendPDPDeActivateReq \n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

            if(NULL != capi2_rsp->dataBuf)
            {
                PDP_SendPDPDeactivateReq_Rsp_t *rsp = (PDP_SendPDPDeactivateReq_Rsp_t *)capi2_rsp->dataBuf;
                KrilPdpData_t *rdata = pdata->bcm_ril_rsp;
                if(rsp->response != PCH_REQ_ACCEPTED)
                {
                    KRIL_DEBUG(DBG_ERROR, "PDPDeActivate Fail resp(1 accept) %d, cid %d\r\n", rsp->response, rsp->cid);
                    pdata->result = RIL_E_RADIO_NOT_AVAILABLE;
                    pdata->handler_state = BCM_ErrorCAPI2Cmd;
                    break;
                }

                rdata->cid = rsp->cid;
                KRIL_DEBUG(DBG_INFO, "PDP Deactivate Resp - cid %d \n", rsp->cid);

                pdata->result = RIL_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
            }
            else
            {
                pdata->result = RIL_E_GENERIC_FAILURE;
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

void KRIL_DataStateHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
	static KrilDataState_t gDataState;

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

	switch(pdata->handler_state)
    {
	case BCM_SendCAPI2Cmd:
	{
		UInt16 tid_test;	
		if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "Enter Data State Fail with NULL data\n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		memcpy(&gDataState, (KrilDataState_t *)(pdata->ril_cmd->data), sizeof(KrilDataState_t));
		tid_test = GetNewTID();
		CAPI2_PDP_GetPCHContextState(tid_test, GetClientID(),gDataState.cid);
		pdata->handler_state = BCM_PDP_Verify;
	}
	break;

	case BCM_PDP_Verify:
        {
		UInt16 tid_test;	
		 KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
		 if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "Fail to send Enter Data State \n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	 
		 if(NULL != capi2_rsp->dataBuf)
            {
		PCHContextState_t *rsp = (PCHContextState_t *)capi2_rsp->dataBuf;		

		KRIL_DEBUG(DBG_INFO, "[BCM_PDP_Verify] - rsp:: %d  *rsp:: %d \n", rsp, *rsp);
		if((gDataState.cid != NULL) && (*rsp== CONTEXT_UNDEFINED))
            {
            	   KRIL_DEBUG(DBG_ERROR, "[BCM_PDP_Verify]::CONTEXT_UNDEFINED\n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		tid_test = GetNewTID();
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
            {
                ClientInfo_t clientInfo;
                CAPI2_InitClientInfo( &clientInfo, tid_test, GetClientID());
                CAPI2_PdpApi_DeactivateSNDCPConnection( &clientInfo, gDataState.cid );
            }
#else
		CAPI2_PDP_ActivateSNDCPConnection(tid_test, GetClientID(),gDataState.cid);
#endif
		  KRIL_DEBUG(DBG_INFO, "New tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_RESPCAPI2Cmd;
	 	}
	    
	    break;
        }

	case BCM_RESPCAPI2Cmd:
    {
		  KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - Fail to send Enter Data State \n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	
		else
		{
			 KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - RESULT_OK-> result:0x%x\n\n",  capi2_rsp->result);		
		}
				
        if(NULL != capi2_rsp->dataBuf)
        {
            PDP_DataState_t *rsp = (PDP_DataState_t *)capi2_rsp->dataBuf;

            if(rsp->response != PCH_REQ_ACCEPTED)
            {
                KRIL_DEBUG(DBG_ERROR, "Enter data state Fail resp(1 accept) %d, \r\n", rsp->response);
                pdata->result = RIL_E_RADIO_NOT_AVAILABLE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
            }
            else
            {
		        KRIL_DEBUG(DBG_ERROR, "Enter data state Successfull %d, \r\n", rsp->response);
                pdata->bcm_ril_rsp = kmalloc(sizeof(KrilDataState_t), GFP_KERNEL);
                pdata->rsp_len = sizeof(KrilDataState_t);
                memset(pdata->bcm_ril_rsp, 0, pdata->rsp_len);
                KrilDataState_t *rdata = pdata->bcm_ril_rsp;

                //rdata->cid = rsp->cid;
                //KRIL_DEBUG(DBG_ERROR, "Enter Data State Res- cid %d \n", rsp->cid);

                pdata->result = RIL_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
			}
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

	KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - Command found...!\n");
//	KRIL_SendResponse(cmd_list);
}

void KRIL_SendDataHandler(void *ril_cmd, Kril_CAPI2Info_t *capi2_rsp)
{
	KRIL_CmdList_t *pdata = (KRIL_CmdList_t *)ril_cmd;
	static KrilSendData_t gSendData;
	KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler Entered \n");

    if (capi2_rsp != NULL)    	
        KRIL_DEBUG(DBG_INFO, "handler_state:0x%lX::result:%d\n", pdata->handler_state, capi2_rsp->result);

	switch(pdata->handler_state)
    {
        case BCM_SendCAPI2Cmd:
        {
		UInt32 tid_test;	

		KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler Entered::BCM_SendCAPI2Cmd \n");
		
    		if (NULL == pdata->ril_cmd->data)
            {
                KRIL_DEBUG(DBG_ERROR, "Send Data Fail with NULL data\n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }

		memcpy(&gSendData, (KrilSendData_t *)(pdata->ril_cmd->data), sizeof(KrilSendData_t));

		KRIL_DEBUG(DBG_INFO, "KRIL_SendDataHandler - Send Data : CID %d \n", gSendData.cid);
//		KRIL_DEBUG(DBG_ERROR, "KRIL_SendDataHandler - Send Data : NumberofBytes %d \n", gSendData.numberBytes);
		
		tid_test = GetNewTID();
		CAPI2_PDP_GetPCHContextState(tid_test, GetClientID(),gSendData.cid);		
		  KRIL_DEBUG(DBG_ERROR, "My new tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_PDP_Verify;
        }
		break;

	case BCM_PDP_Verify:
        {
		UInt32 tid_test;	
		 KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
		 if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - Fail to send Enter Data State \n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	 

		 if(NULL != capi2_rsp->dataBuf)
            {
		PCHContextState_t *rsp = (PCHContextState_t *)capi2_rsp->dataBuf;		

		KRIL_DEBUG(DBG_INFO, "[BCM_PDP_Verify] - rsp:: %d  *rsp:: %d \n", rsp, *rsp);
		if((gSendData.cid != NULL) && (*rsp== CONTEXT_UNDEFINED))
            {
            	   KRIL_DEBUG(DBG_ERROR, "[BCM_PDP_Verify]::CONTEXT_UNDEFINED\n");
//             KRIL_DEBUG(DBG_ERROR, "%d CID not supported\n", gDataState.cid);
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }
		tid_test = GetNewTID();
		CAPI2_PDP_SendTBFData(tid_test, GetClientID(), gSendData.cid, gSendData.numberBytes);
		  KRIL_DEBUG(DBG_INFO, "My new tid_test is %d\n", tid_test);
		 pdata->handler_state = BCM_RESPCAPI2Cmd;
	 	}
        }
	break;
	
	case BCM_RESPCAPI2Cmd:
        {
		  KRIL_DEBUG(DBG_INFO, "result:0x%x\n", capi2_rsp->result);
            if(RESULT_OK != capi2_rsp->result)
            {
                KRIL_DEBUG(DBG_ERROR, "KRIL_SendDataHandler - Fail to send data \n");
                pdata->result = RIL_E_GENERIC_FAILURE;
                pdata->handler_state = BCM_ErrorCAPI2Cmd;
                break;
            }	
		else
		{
			 KRIL_DEBUG(DBG_ERROR, "KRIL_DataStateHandler - RESULT_OK-> result:0x%x\n\n",  capi2_rsp->result);		
		}
				
		  pdata->result = RIL_E_SUCCESS;
                pdata->handler_state = BCM_FinishCAPI2Cmd;
	  }	
	break;
	}
}
		



