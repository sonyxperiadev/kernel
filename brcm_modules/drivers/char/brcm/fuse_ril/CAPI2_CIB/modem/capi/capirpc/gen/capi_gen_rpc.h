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
#ifndef CAPI_GEN_MSG_H
#define CAPI_GEN_MSG_H


//***************** < 1 > **********************






typedef struct
{
	Boolean	val;
}CAPI2_SMS_IsMeStorageEnabled_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SMS_GetMaxMeCapacity_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SMS_GetNextFreeSlot_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
	SIMSMSMesgStatus_t  status;
}CAPI2_SMS_SetMeSmsStatus_Req_t;

typedef struct
{
	Result_t	val;
}CAPI2_SMS_SetMeSmsStatus_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_GetMeSmsStatus_Req_t;

typedef struct
{
	SIMSMSMesgStatus_t	val;
}CAPI2_SMS_GetMeSmsStatus_Rsp_t;

typedef struct
{
	UInt8  *inSms;
	UInt16  inLength;
	SIMSMSMesgStatus_t  status;
	UInt16  slotNumber;
}CAPI2_SMS_StoreSmsToMe_Req_t;

typedef struct
{
	Result_t	val;
}CAPI2_SMS_StoreSmsToMe_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_RetrieveSmsFromMe_Req_t;

typedef struct
{
	Result_t	val;
}CAPI2_SMS_RetrieveSmsFromMe_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_RemoveSmsFromMe_Req_t;

typedef struct
{
	Result_t	val;
}CAPI2_SMS_RemoveSmsFromMe_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SMS_IsSmsReplSupported_Rsp_t;

typedef struct
{
	UInt16  cmd;
}CAPI2_SMS_GetMeSmsBufferStatus_Req_t;

typedef struct
{
	UInt16  bfree;
	UInt16  bused;
}CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt8  tp_pid;
	uchar_ptr_t  oaddress;
}CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t;

typedef struct
{
	UInt16	val;
}CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t;

//***************** < 2 > **********************





bool_t xdr_CAPI2_SMS_IsMeStorageEnabled_Rsp_t(void* xdrs, CAPI2_SMS_IsMeStorageEnabled_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMaxMeCapacity_Rsp_t(void* xdrs, CAPI2_SMS_GetMaxMeCapacity_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetNextFreeSlot_Rsp_t(void* xdrs, CAPI2_SMS_GetNextFreeSlot_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Rsp_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_StoreSmsToMe_Req_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_StoreSmsToMe_Rsp_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Rsp_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_IsSmsReplSupported_Rsp_t(void* xdrs, CAPI2_SMS_IsSmsReplSupported_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t *rsp);

//***************** < 3 > **********************





Result_t Handle_CAPI2_SMS_IsMeStorageEnabled(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_SMS_GetMaxMeCapacity(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_SMS_GetNextFreeSlot(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_SMS_SetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber, SIMSMSMesgStatus_t status);
Result_t Handle_CAPI2_SMS_GetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber);
Result_t Handle_CAPI2_SMS_StoreSmsToMe(RPC_Msg_t* pReqMsg, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber);
Result_t Handle_CAPI2_SMS_RetrieveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber);
Result_t Handle_CAPI2_SMS_RemoveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber);
Result_t Handle_CAPI2_SMS_IsSmsReplSupported(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_SMS_GetMeSmsBufferStatus(RPC_Msg_t* pReqMsg, UInt16 cmd);
Result_t Handle_CAPI2_SMS_GetRecordNumberOfReplaceSMS(RPC_Msg_t* pReqMsg, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress);

//***************** < 12 > **********************





//***************************************************************************************
/**
	Function response for the CAPI2_SMS_IsMeStorageEnabled
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SMS_ISMESTORAGEENABLED_RSP
**/
void CAPI2_SMS_IsMeStorageEnabled(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetMaxMeCapacity
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: UInt16
	@n Response to CP will be notified via ::MSG_SMS_GETMAXMECAPACITY_RSP
**/
void CAPI2_SMS_GetMaxMeCapacity(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetNextFreeSlot
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: UInt16
	@n Response to CP will be notified via ::MSG_SMS_GETNEXTFREESLOT_RSP
**/
void CAPI2_SMS_GetNextFreeSlot(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_SetMeSmsStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		slotNumber(in) param of type UInt16
	@param		status(in) param of type SIMSMSMesgStatus_t
	@return		Not Applicable
	@note
	Payload: Result_t
	@n Response to CP will be notified via ::MSG_SMS_SETMESMSSTATUS_RSP
**/
void CAPI2_SMS_SetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetMeSmsStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		slotNumber(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: SIMSMSMesgStatus_t
	@n Response to CP will be notified via ::MSG_SMS_GETMESMSSTATUS_RSP
**/
void CAPI2_SMS_GetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_StoreSmsToMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		inSms(in) param of type UInt8
	@param		inLength(in) param of type UInt16
	@param		status(in) param of type SIMSMSMesgStatus_t
	@param		slotNumber(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: Result_t
	@n Response to CP will be notified via ::MSG_SMS_STORESMSTOME_RSP
**/
void CAPI2_SMS_StoreSmsToMe(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_RetrieveSmsFromMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		slotNumber(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: Result_t
	@n Response to CP will be notified via ::MSG_SMS_RETRIEVESMSFROMME_RSP
**/
void CAPI2_SMS_RetrieveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_RemoveSmsFromMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		slotNumber(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: Result_t
	@n Response to CP will be notified via ::MSG_SMS_REMOVESMSFROMME_RSP
**/
void CAPI2_SMS_RemoveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_IsSmsReplSupported
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP
**/
void CAPI2_SMS_IsSmsReplSupported(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetMeSmsBufferStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		cmd(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_SMS_GETMESMS_BUF_STATUS_RSP
**/
void CAPI2_SMS_GetMeSmsBufferStatus(UInt32 tid, UInt8 clientID, UInt16 cmd);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetMeSmsBufferStatus_RSP
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		bfree(in) param of type UInt16
	@param		bused(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_SMS_GETMESMS_BUF_STATUS_RSP
**/
void CAPI2_SMS_GetMeSmsBufferStatus_RSP(UInt32 tid, UInt8 clientID, UInt16 bfree, UInt16 bused);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetRecordNumberOfReplaceSMS
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		storageType(in) param of type SmsStorage_t
	@param		tp_pid(in) param of type UInt8
	@param		oaddress(in) param of type uchar_ptr_t
	@return		Not Applicable
	@note
	Payload: UInt16
	@n Response to CP will be notified via ::MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP
**/
void CAPI2_SMS_GetRecordNumberOfReplaceSMS(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress);


//***************** < 16 > **********************



#endif
