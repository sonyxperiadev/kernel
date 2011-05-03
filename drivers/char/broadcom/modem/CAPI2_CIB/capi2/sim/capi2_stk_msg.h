/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
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
/**
*
*   @file   capi2_stk_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/


#ifndef CAPI2_STK_MSG_H
#define CAPI2_STK_MSG_H




__BEGIN_DECLS
XDR_ENUM_DECLARE(Unicode_t)
XDR_ENUM_DECLARE(SATK_EVENTS_t)
XDR_ENUM_DECLARE(SATK_ResultCode_t)
XDR_ENUM_DECLARE(SATK_ResultCode2_t)
XDR_ENUM_DECLARE(SATKInKeyType_t)
XDR_ENUM_DECLARE(SATKToneType_t)
XDR_ENUM_DECLARE(SATKSSType_t)
XDR_ENUM_DECLARE(SATKCallType_t)
XDR_ENUM_DECLARE(STKRefreshType_t)
XDR_ENUM_DECLARE(SIM_LAUNCH_BROWSER_ACTION_t)
XDR_ENUM_DECLARE(SATKDataServiceType_t)
XDR_ENUM_DECLARE(StkCallSetupFailResult_t)
XDR_ENUM_DECLARE(StkCallControl_t)
XDR_ENUM_DECLARE(StkCallControlResult_t)
XDR_ENUM_DECLARE(STK_DATA_SERV_BEARER_TYPE_t)
XDR_ENUM_DECLARE(SATK_CHAN_SOCKET_t)
XDR_ENUM_DECLARE(SATK_CHAN_STATE_t)
XDR_ENUM_DECLARE(STKMenuSelectionResCode_t)

XDR_ENUM_DECLARE(EnvelopeRspData_t)
XDR_ENUM_DECLARE(TerminalResponseRspData_t)
XDR_ENUM_DECLARE(StkCallControlSetupRsp_t)
XDR_ENUM_DECLARE(StkCallControlSsRsp_t)
XDR_ENUM_DECLARE(StkCallControlUssdRsp_t)
XDR_ENUM_DECLARE(StkCallControlSmsRsp_t)
XDR_ENUM_DECLARE(StkCallSetupFail_t)
XDR_ENUM_DECLARE(StkCallControlDisplay_t)
XDR_ENUM_DECLARE(RunAT_Request)
XDR_ENUM_DECLARE(StkDataServReq_t)
XDR_ENUM_DECLARE(SATKDataCloseType_t)

bool_t xdr_STKIconListId_t(XDR*, STKIconListId_t*);
bool_t xdr_SATKIcon_t(XDR*, SATKIcon_t*);
bool_t xdr_SATKString_t(XDR*, SATKString_t*);
bool_t xdr_SetupMenu_t(XDR*, SetupMenu_t*);
bool_t xdr_StkCmdRespond_t(XDR*, StkCmdRespond_t*);
bool_t xdr_SATKNum_t(XDR*, SATKNum_t*);
bool_t xdr_REFRESH_FILE_t(XDR*, REFRESH_FILE_t*);
bool_t xdr_ProvisionFile_t(XDR*, ProvisionFile_t*);
bool_t xdr_STK_SMS_DATA_t(XDR*, STK_SMS_DATA_t*);
bool_t xdr_DisplayText_t(XDR*, DisplayText_t*);
bool_t xdr_GetInkey_t(XDR*, GetInkey_t*);
bool_t xdr_GetInput_t(XDR*, GetInput_t*);
bool_t xdr_PlayTone_t(XDR*, PlayTone_t*);
bool_t xdr_SelectItem_t(XDR*, SelectItem_t*);
bool_t xdr_SendSs_t(XDR*, SendSs_t*);
bool_t xdr_SendUssd_t(XDR*, SendUssd_t*);
bool_t xdr_SetupCall_t(XDR*, SetupCall_t*);
bool_t xdr_Refresh_t(XDR*, Refresh_t*);
bool_t xdr_SendMOSMS_t(XDR*, SendMOSMS_t*);
bool_t xdr_SATK_EventData_t(XDR*, SATK_EventData_t*);
bool_t xdr_SendMOSMS_t(XDR*, SendMOSMS_t*);
bool_t xdr_SendStkDtmf_t(XDR*, SendStkDtmf_t*);
bool_t xdr_LaunchBrowserReq_t(XDR*, LaunchBrowserReq_t*);
bool_t xdr_IdleModeText_t(XDR*, IdleModeText_t*);
bool_t xdr_StkDataService_t(XDR*, StkDataService_t*);
bool_t xdr_StkRunAtCmd_t(XDR*, StkRunAtCmd_t*);
bool_t xdr_SATK_CallSetupFail_t(XDR*, StkCallSetupFail_t*);
bool_t xdr_SATK_CallControlDisplay_t(XDR*, StkCallControlDisplay_t*);
bool_t xdr_StkLangNotification_t(XDR* , StkLangNotification_t*);
bool_t xdr_StkRunAtReq_t(XDR*, RunAT_Request*);
bool_t xdr_Subaddress_t(XDR* xdrs, Subaddress_t *subAddr);

bool_t xdr_StkTermProfileInd_t(XDR* xdrs, StkTermProfileInd_t* res);
bool_t xdr_StkUiccCatInd_t(XDR* xdrs, StkUiccCatInd_t* res);
bool_t xdr_StkExtProactiveCmdInd_t(XDR* xdrs, StkExtProactiveCmdInd_t* res);
bool_t xdr_ProactiveCmdData_t(XDR* xdrs, ProactiveCmdData_t* res);
bool_t xdr_StkProactiveCmdFetchingOnOffRsp_t(void* xdrs, StkProactiveCmdFetchingOnOffRsp_t *rsp);
bool_t xdr_StkPollingIntervalRsp_t(void* xdrs, StkPollingIntervalRsp_t* rsp);
__END_DECLS

#endif

