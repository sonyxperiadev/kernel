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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/
#include "mobcom_types.h"

#include "resultcode.h"

#include "capi2_reqrep.h"
#include "capi2_gen_apps_switch.h"

Result_t CAPI2_GenAppsMsgHnd(MsgType_t msgId, UInt32 tid, UInt8 clientID, CAPI2_ReqRep_t* reqRep)
{
	Result_t result = RESULT_OK;
	switch((UInt32)msgId)
	{


	case MSG_PMU_IS_SIM_READY_REQ:
		_DEF(CAPI2_PMU_IsSIMReady)(tid, clientID);
		break;
	case MSG_PMU_ACTIVATE_SIM_REQ:
		_DEF(CAPI2_PMU_ActivateSIM)(tid, clientID,reqRep->req_rep_u.CAPI2_PMU_ActivateSIM_Req.volt);
		break;
	case MSG_PMU_DEACTIVATE_SIM_REQ:
		_DEF(CAPI2_PMU_DeactivateSIM)(tid, clientID);
		break;
	case MSG_GPIO_SET_HIGH_64PIN_REQ:
		_DEF(CAPI2_GPIO_Set_High_64Pin)(tid, clientID,reqRep->req_rep_u.CAPI2_GPIO_Set_High_64Pin_Req.gpio_pin);
		break;
	case MSG_GPIO_SET_LOW_64PIN_REQ:
		_DEF(CAPI2_GPIO_Set_Low_64Pin)(tid, clientID,reqRep->req_rep_u.CAPI2_GPIO_Set_Low_64Pin_Req.gpio_pin);
		break;
	case MSG_PMU_START_CHARGING_REQ:
		_DEF(CAPI2_PMU_StartCharging)(tid, clientID);
		break;
	case MSG_PMU_STOP_CHARGING_REQ:
		_DEF(CAPI2_PMU_StopCharging)(tid, clientID);
		break;
	case MSG_SMS_ISMESTORAGEENABLED_REQ:
		_DEF(CAPI2_SMS_IsMeStorageEnabled)(tid, clientID);
		break;
	case MSG_SMS_GETMAXMECAPACITY_REQ:
		_DEF(CAPI2_SMS_GetMaxMeCapacity)(tid, clientID);
		break;
	case MSG_SMS_GETNEXTFREESLOT_REQ:
		_DEF(CAPI2_SMS_GetNextFreeSlot)(tid, clientID);
		break;
	case MSG_SMS_SETMESMSSTATUS_REQ:
		_DEF(CAPI2_SMS_SetMeSmsStatus)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.slotNumber,reqRep->req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.status);
		break;
	case MSG_SMS_GETMESMSSTATUS_REQ:
		_DEF(CAPI2_SMS_GetMeSmsStatus)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_GetMeSmsStatus_Req.slotNumber);
		break;
	case MSG_SMS_STORESMSTOME_REQ:
		_DEF(CAPI2_SMS_StoreSmsToMe)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inSms,reqRep->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inLength,reqRep->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.status,reqRep->req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.slotNumber);
		break;
	case MSG_SMS_RETRIEVESMSFROMME_REQ:
		_DEF(CAPI2_SMS_RetrieveSmsFromMe)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Req.slotNumber);
		break;
	case MSG_SMS_REMOVESMSFROMME_REQ:
		_DEF(CAPI2_SMS_RemoveSmsFromMe)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Req.slotNumber);
		break;
	case MSG_PMU_CLIENT_POWER_DOWN_REQ:
		_DEF(CAPI2_PMU_ClientPowerDown)(tid, clientID);
		break;
	case MSG_PMU_GET_POWERUP_CAUSE_REQ:
		_DEF(CAPI2_PMU_GetPowerupCause)(tid, clientID);
		break;
	case MSG_SOCKET_OPEN_REQ:
		_DEF(CAPI2_SOCKET_Open)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Open_Req.domain,reqRep->req_rep_u.CAPI2_SOCKET_Open_Req.type,reqRep->req_rep_u.CAPI2_SOCKET_Open_Req.protocol);
		break;
	case MSG_SOCKET_BIND_REQ:
		_DEF(CAPI2_SOCKET_Bind)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Bind_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_Bind_Req.addr);
		break;
	case MSG_SOCKET_LISTEN_REQ:
		_DEF(CAPI2_SOCKET_Listen)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Listen_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_Listen_Req.backlog);
		break;
	case MSG_SOCKET_ACCEPT_REQ:
		_DEF(CAPI2_SOCKET_Accept)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Accept_Req.descriptor);
		break;
	case MSG_SOCKET_CONNECT_REQ:
		_DEF(CAPI2_SOCKET_Connect)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Connect_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_Connect_Req.name);
		break;
	case MSG_SOCKET_GETPEERNAME_REQ:
		_DEF(CAPI2_SOCKET_GetPeerName)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_GetPeerName_Req.descriptor);
		break;
	case MSG_SOCKET_GETSOCKNAME_REQ:
		_DEF(CAPI2_SOCKET_GetSockName)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_GetSockName_Req.descriptor);
		break;
	case MSG_SOCKET_SETSOCKOPT_REQ:
		_DEF(CAPI2_SOCKET_SetSockOpt)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.optname,reqRep->req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.optval);
		break;
	case MSG_SOCKET_GETSOCKOPT_REQ:
		_DEF(CAPI2_SOCKET_GetSockOpt)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_GetSockOpt_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_GetSockOpt_Req.optname);
		break;
	case MSG_SOCKET_SEND_REQ:
		_DEF(CAPI2_SOCKET_Send)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Send_Req.sockSendReq);
		break;
	case MSG_SOCKET_SEND_TO_REQ:
		_DEF(CAPI2_SOCKET_SendTo)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_SendTo_Req.sockSendReq,reqRep->req_rep_u.CAPI2_SOCKET_SendTo_Req.to);
		break;
	case MSG_SOCKET_RECV_REQ:
		_DEF(CAPI2_SOCKET_Recv)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Recv_Req.sockRecvReq);
		break;
	case MSG_SOCKET_RECV_FROM_REQ:
		_DEF(CAPI2_SOCKET_RecvFrom)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_RecvFrom_Req.sockRecvReq,reqRep->req_rep_u.CAPI2_SOCKET_RecvFrom_Req.from);
		break;
	case MSG_SOCKET_CLOSE_REQ:
		_DEF(CAPI2_SOCKET_Close)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Close_Req.descriptor);
		break;
	case MSG_SOCKET_SHUTDOWN_REQ:
		_DEF(CAPI2_SOCKET_Shutdown)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Shutdown_Req.descriptor,reqRep->req_rep_u.CAPI2_SOCKET_Shutdown_Req.how);
		break;
	case MSG_SOCKET_ERRNO_REQ:
		_DEF(CAPI2_SOCKET_Errno)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_Errno_Req.descriptor);
		break;
	case MSG_SOCKET_SO2LONG_REQ:
		_DEF(CAPI2_SOCKET_SO2LONG)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_SO2LONG_Req.socket);
		break;
	case MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_REQ:
		_DEF(CAPI2_SOCKET_GetSocketSendBufferSpace)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_GetSocketSendBufferSpace_Req.bufferSpace);
		break;
	case MSG_SOCKET_PARSE_IPAD_REQ:
		_DEF(CAPI2_SOCKET_ParseIPAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_SOCKET_ParseIPAddr_Req.ipString);
		break;
	case MSG_SETUP_DATA_CONNECTION_REQ:
		_DEF(CAPI2_DC_SetupDataConnection)(tid, clientID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnection_Req.inClientID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnection_Req.acctID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnection_Req.linkType);
		break;
	case MSG_SETUP_DATA_CONNECTION_EX_REQ:
		_DEF(CAPI2_DC_SetupDataConnectionEx)(tid, clientID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.inClientID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.acctID,reqRep->req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.linkType,reqRep->req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.apnCheck,reqRep->req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.actDCAcctId);
		break;
	case MSG_DC_SHUTDOWN_DATA_CONNECTION_REQ:
		_DEF(CAPI2_DC_ShutdownDataConnection)(tid, clientID,reqRep->req_rep_u.CAPI2_DC_ShutdownDataConnection_Req.inClientID,reqRep->req_rep_u.CAPI2_DC_ShutdownDataConnection_Req.acctID);
		break;
	case MSG_DATA_IS_ACCT_ID_VALID_REQ:
		_DEF(CAPI2_DATA_IsAcctIDValid)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_IsAcctIDValid_Req.acctID);
		break;
	case MSG_DATA_CREATE_GPRS_ACCT_REQ:
		_DEF(CAPI2_DATA_CreateGPRSDataAcct)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_CreateGPRSDataAcct_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_CreateGPRSDataAcct_Req.pGprsSetting);
		break;
	case MSG_DATA_CREATE_GSM_ACCT_REQ:
		_DEF(CAPI2_DATA_CreateCSDDataAcct)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_CreateCSDDataAcct_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_CreateCSDDataAcct_Req.pCsdSetting);
		break;
	case MSG_DATA_DELETE_ACCT_REQ:
		_DEF(CAPI2_DATA_DeleteDataAcct)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_DeleteDataAcct_Req.acctID);
		break;
	case MSG_DATA_SET_USERNAME_REQ:
		_DEF(CAPI2_DATA_SetUsername)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetUsername_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetUsername_Req.username);
		break;
	case MSG_DATA_GET_USERNAME_REQ:
		_DEF(CAPI2_DATA_GetUsername)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetUsername_Req.acctID);
		break;
	case MSG_DATA_SET_PASSWORD_REQ:
		_DEF(CAPI2_DATA_SetPassword)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetPassword_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetPassword_Req.password);
		break;
	case MSG_DATA_GET_PASSWORD_REQ:
		_DEF(CAPI2_DATA_GetPassword)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetPassword_Req.acctID);
		break;
	case MSG_DATA_SET_STATIC_IP_REQ:
		_DEF(CAPI2_DATA_SetStaticIPAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetStaticIPAddr_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetStaticIPAddr_Req.staticIPAddr);
		break;
	case MSG_DATA_GET_STATIC_IP_REQ:
		_DEF(CAPI2_DATA_GetStaticIPAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetStaticIPAddr_Req.acctID);
		break;
	case MSG_DATA_SET_PRIMARY_DNS_ADDR_REQ:
		_DEF(CAPI2_DATA_SetPrimaryDnsAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetPrimaryDnsAddr_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetPrimaryDnsAddr_Req.priDnsAddr);
		break;
	case MSG_DATA_GET_PRIMARY_DNS_ADDR_REQ:
		_DEF(CAPI2_DATA_GetPrimaryDnsAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetPrimaryDnsAddr_Req.acctID);
		break;
	case MSG_DATA_SET_SECOND_DNS_ADDR_REQ:
		_DEF(CAPI2_DATA_SetSecondDnsAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetSecondDnsAddr_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetSecondDnsAddr_Req.sndDnsAddr);
		break;
	case MSG_DATA_GET_SECOND_DNS_ADDR_REQ:
		_DEF(CAPI2_DATA_GetSecondDnsAddr)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetSecondDnsAddr_Req.acctID);
		break;
	case MSG_DATA_SET_DATA_COMPRESSION_REQ:
		_DEF(CAPI2_DATA_SetDataCompression)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetDataCompression_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetDataCompression_Req.dataCompEnable);
		break;
	case MSG_DATA_GET_DATA_COMPRESSION_REQ:
		_DEF(CAPI2_DATA_GetDataCompression)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetDataCompression_Req.acctID);
		break;
	case MSG_DATA_GET_ACCT_TYPE_REQ:
		_DEF(CAPI2_DATA_GetAcctType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetAcctType_Req.acctID);
		break;
	case MSG_DATA_GET_EMPTY_ACCT_SLOT_REQ:
		_DEF(CAPI2_DATA_GetEmptyAcctSlot)(tid, clientID);
		break;
	case MSG_DATA_GET_CID_FROM_ACCTID_REQ:
		_DEF(CAPI2_DATA_GetCidFromDataAcctID)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCidFromDataAcctID_Req.acctID);
		break;
	case MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_REQ:
		_DEF(CAPI2_DATA_GetDataAcctIDFromCid)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetDataAcctIDFromCid_Req.contextID);
		break;
	case MSG_DATA_GET_PRI_FROM_ACCTID_REQ:
		_DEF(CAPI2_DATA_GetPrimaryCidFromDataAcctID)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req.acctID);
		break;
	case MSG_DATA_IS_SND_DATA_ACCT_REQ:
		_DEF(CAPI2_DATA_IsSecondaryDataAcct)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_IsSecondaryDataAcct_Req.acctID);
		break;
	case MSG_DATA_GET_ACCTID_FROM_CID_REQ:
		_DEF(CAPI2_DATA_GetDataSentSize)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetDataSentSize_Req.acctID);
		break;
	case MSG_DATA_GET_DATA_RCV_SIZE_REQ:
		_DEF(CAPI2_DATA_GetDataRcvSize)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetDataRcvSize_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_PDP_TYPE_REQ:
		_DEF(CAPI2_DATA_SetGPRSPdpType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSPdpType_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSPdpType_Req.pdpType);
		break;
	case MSG_DATA_GET_GPRS_PDP_TYPE_REQ:
		_DEF(CAPI2_DATA_GetGPRSPdpType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGPRSPdpType_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_APN_REQ:
		_DEF(CAPI2_DATA_SetGPRSApn)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSApn_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSApn_Req.apn);
		break;
	case MSG_DATA_GET_GPRS_APN_REQ:
		_DEF(CAPI2_DATA_GetGPRSApn)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGPRSApn_Req.acctID);
		break;
	case MSG_DATA_SET_AUTHEN_METHOD_REQ:
		_DEF(CAPI2_DATA_SetAuthenMethod)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetAuthenMethod_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetAuthenMethod_Req.authenMethod);
		break;
	case MSG_DATA_GET_AUTHEN_METHOD_REQ:
		_DEF(CAPI2_DATA_GetAuthenMethod)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetAuthenMethod_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_HEADER_COMPRESSION_REQ:
		_DEF(CAPI2_DATA_SetGPRSHeaderCompression)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSHeaderCompression_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSHeaderCompression_Req.headerCompEnable);
		break;
	case MSG_DATA_GET_GPRS_HEADER_COMPRESSION_REQ:
		_DEF(CAPI2_DATA_GetGPRSHeaderCompression)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGPRSHeaderCompression_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_QOS_REQ:
		_DEF(CAPI2_DATA_SetGPRSQos)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSQos_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSQos_Req.qos);
		break;
	case MSG_CAPI2_DATA_GET_GPRS_QOS_REQ:
		_DEF(CAPI2_DATA_GetGPRSQos)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGPRSQos_Req.acctID);
		break;
	case MSG_DATA_SET_ACCT_LOCK_REQ:
		_DEF(CAPI2_DATA_SetAcctLock)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetAcctLock_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetAcctLock_Req.acctLock);
		break;
	case MSG_DATA_GET_ACCT_LOCK_REQ:
		_DEF(CAPI2_DATA_GetAcctLock)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetAcctLock_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_ONLY_REQ:
		_DEF(CAPI2_DATA_SetGprsOnly)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGprsOnly_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGprsOnly_Req.gprsOnly);
		break;
	case MSG_DATA_GET_GPRS_ONLY_REQ:
		_DEF(CAPI2_DATA_GetGprsOnly)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGprsOnly_Req.acctID);
		break;
	case MSG_DATA_SET_GPRS_TFT_REQ:
		_DEF(CAPI2_DATA_SetGPRSTft)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSTft_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetGPRSTft_Req.pTft);
		break;
	case MSG_DATA_GET_GPRS_TFT_REQ:
		_DEF(CAPI2_DATA_GetGPRSTft)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetGPRSTft_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_DIAL_NUMBER_REQ:
		_DEF(CAPI2_DATA_SetCSDDialNumber)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDialNumber_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDialNumber_Req.dialNumber);
		break;
	case MSG_DATA_GET_CSD_DIAL_NUMBER_REQ:
		_DEF(CAPI2_DATA_GetCSDDialNumber)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDDialNumber_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_DIAL_TYPE_REQ:
		_DEF(CAPI2_DATA_SetCSDDialType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDialType_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDialType_Req.csdDialType);
		break;
	case MSG_DATA_GET_CSD_DIAL_TYPE_REQ:
		_DEF(CAPI2_DATA_GetCSDDialType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDDialType_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_BAUD_RATE_REQ:
		_DEF(CAPI2_DATA_SetCSDBaudRate)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDBaudRate_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDBaudRate_Req.csdBaudRate);
		break;
	case MSG_DATA_GET_CSD_BAUD_RATE_REQ:
		_DEF(CAPI2_DATA_GetCSDBaudRate)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDBaudRate_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_SYNC_TYPE_REQ:
		_DEF(CAPI2_DATA_SetCSDSyncType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDSyncType_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDSyncType_Req.csdSyncType);
		break;
	case MSG_DATA_GET_CSD_SYNC_TYPE_REQ:
		_DEF(CAPI2_DATA_GetCSDSyncType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDSyncType_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_ERROR_CORRECTION_REQ:
		_DEF(CAPI2_DATA_SetCSDErrorCorrection)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDErrorCorrection_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDErrorCorrection_Req.enable);
		break;
	case MSG_DATA_GET_CSD_ERROR_CORRECTION_REQ:
		_DEF(CAPI2_DATA_GetCSDErrorCorrection)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDErrorCorrection_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_REQ:
		_DEF(CAPI2_DATA_SetCSDErrCorrectionType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDErrCorrectionType_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDErrCorrectionType_Req.errCorrectionType);
		break;
	case MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_REQ:
		_DEF(CAPI2_DATA_GetCSDErrCorrectionType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDErrCorrectionType_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_DATA_COMP_TYPE_REQ:
		_DEF(CAPI2_DATA_SetCSDDataCompType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDataCompType_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDDataCompType_Req.dataCompType);
		break;
	case MSG_DATA_GET_CSD_DATA_COMP_TYPE_REQ:
		_DEF(CAPI2_DATA_GetCSDDataCompType)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDDataCompType_Req.acctID);
		break;
	case MSG_DATA_SET_CSD_CONN_ELEMENT_REQ:
		_DEF(CAPI2_DATA_SetCSDConnElement)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_SetCSDConnElement_Req.acctID,reqRep->req_rep_u.CAPI2_DATA_SetCSDConnElement_Req.connElement);
		break;
	case MSG_DATA_GET_CSD_CONN_ELEMENT_REQ:
		_DEF(CAPI2_DATA_GetCSDConnElement)(tid, clientID,reqRep->req_rep_u.CAPI2_DATA_GetCSDConnElement_Req.acctID);
		break;
	case MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_REQ:
		_DEF(CAPI2_DATA_UpdateAccountToFileSystem)(tid, clientID);
		break;
	case MSG_DATA_RESET_DATA_SIZE_REQ:
		_DEF(CAPI2_resetDataSize)(tid, clientID,reqRep->req_rep_u.CAPI2_resetDataSize_Req.cid);
		break;
	case MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_REQ:
		_DEF(CAPI2_addDataSentSizebyCid)(tid, clientID,reqRep->req_rep_u.CAPI2_addDataSentSizebyCid_Req.cid,reqRep->req_rep_u.CAPI2_addDataSentSizebyCid_Req.size);
		break;
	case MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_REQ:
		_DEF(CAPI2_addDataRcvSizebyCid)(tid, clientID,reqRep->req_rep_u.CAPI2_addDataRcvSizebyCid_Req.cid,reqRep->req_rep_u.CAPI2_addDataRcvSizebyCid_Req.size);
		break;
	case MSG_GPIO_CONFIG_OUTPUT_64PIN_REQ:
		_DEF(CAPI2_GPIO_ConfigOutput_64Pin)(tid, clientID,reqRep->req_rep_u.CAPI2_GPIO_ConfigOutput_64Pin_Req.pin);
		break;
	case MSG_GPS_CONTROL_REQ:
		_DEF(CAPI2_GPS_Control)(tid, clientID,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Cmnd,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Param0,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Param1,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Param2,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Param3,reqRep->req_rep_u.CAPI2_GPS_Control_Req.u32Param4);
		break;
	case MSG_FFS_CONTROL_REQ:
		_DEF(CAPI2_FFS_Control)(tid, clientID,reqRep->req_rep_u.CAPI2_FFS_Control_Req.cmd,reqRep->req_rep_u.CAPI2_FFS_Control_Req.address,reqRep->req_rep_u.CAPI2_FFS_Control_Req.offset,reqRep->req_rep_u.CAPI2_FFS_Control_Req.size);
		break;
	case MSG_CP2AP_PEDESTALMODE_CONTROL_REQ:
		_DEF(CAPI2_CP2AP_PedestalMode_Control)(tid, clientID,reqRep->req_rep_u.CAPI2_CP2AP_PedestalMode_Control_Req.enable);
		break;
	case MSG_RTC_SetTime_REQ:
		_DEF(CAPI2_RTC_SetTime)(tid, clientID,reqRep->req_rep_u.CAPI2_RTC_SetTime_Req.inTime);
		break;
	case MSG_RTC_SetDST_REQ:
		_DEF(CAPI2_RTC_SetDST)(tid, clientID,reqRep->req_rep_u.CAPI2_RTC_SetDST_Req.inDST);
		break;
	case MSG_RTC_SetTimeZone_REQ:
		_DEF(CAPI2_RTC_SetTimeZone)(tid, clientID,reqRep->req_rep_u.CAPI2_RTC_SetTimeZone_Req.inTimezone);
		break;
	case MSG_RTC_GetTime_REQ:
		_DEF(CAPI2_RTC_GetTime)(tid, clientID,reqRep->req_rep_u.CAPI2_RTC_GetTime_Req.time);
		break;
	case MSG_RTC_GetTimeZone_REQ:
		_DEF(CAPI2_RTC_GetTimeZone)(tid, clientID);
		break;
	case MSG_RTC_GetDST_REQ:
		_DEF(CAPI2_RTC_GetDST)(tid, clientID);
		break;
	case MSG_SMS_GETMESMS_BUF_STATUS_REQ:
		_DEF(CAPI2_SMS_GetMeSmsBufferStatus)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_Req.cmd);
		break;
	case MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ:
		_DEF(CAPI2_SMS_GetRecordNumberOfReplaceSMS)(tid, clientID,reqRep->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.storageType,reqRep->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.tp_pid,reqRep->req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.oaddress);
		break;
	case MSG_FLASH_SAVEIMAGE_REQ:
		_DEF(CAPI2_FLASH_SaveImage)(tid, clientID,reqRep->req_rep_u.CAPI2_FLASH_SaveImage_Req.flash_addr,reqRep->req_rep_u.CAPI2_FLASH_SaveImage_Req.length,reqRep->req_rep_u.CAPI2_FLASH_SaveImage_Req.shared_mem_addr);
		break;
	case MSG_SIMLOCK_GET_STATUS_REQ:
		_DEF(CAPI2_SIMLOCK_GetStatus)(tid, clientID,reqRep->req_rep_u.CAPI2_SIMLOCK_GetStatus_Req.sim_data);
		break;
	case MSG_FFS_READ_REQ:
		_DEF(CAPI2_FFS_Read)(tid, clientID,reqRep->req_rep_u.CAPI2_FFS_Read_Req.ffsReadReq);
		break;
	case MSG_CAPI2_AT_COMMAND_TO_AP_REQ:
		_DEF(CAPI2_AT_ProcessCmdToAP)(tid, clientID,reqRep->req_rep_u.CAPI2_AT_ProcessCmdToAP_Req.channel,reqRep->req_rep_u.CAPI2_AT_ProcessCmdToAP_Req.cmdStr);
		break;
	case MSG_INTERTASK_MSG_TO_AP_REQ:
		_DEF(CAPI2_InterTaskMsgToAP)(tid, clientID,reqRep->req_rep_u.CAPI2_InterTaskMsgToAP_Req.inPtrMsg);
		break;
	}
	return result;
}
