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
*   @file   capi2_lcs_api.h
*
*   @brief  This header provides the interface description for the CAPI2 Location Service API.
*
****************************************************************************/
/**
*   @defgroup    CAPI2_LCSAPIGroup   Location Service
*   @ingroup     CAPI2_LCSGroup
*
*   @brief      This group defines the interfaces to the location service API.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CAPI2_LCS_API_H
#define CAPI2_LCS_API_H		///< Include guard.

// ---- Include Files -------------------------------------------------------
#include "capi2_types.h"
#include "capi2_resultcode.h"
#include "capi2_lcs_ds.h"
#include "capi2_lcs_cplane_ds.h"
#include "capi2_ss_ds.h"
/**
* @addtogroup  CAPI2_LCSAPIGroup
* @{
*/

//**************************************************************************************
/**
	Start the location service with the provided service type. When the LCS_SERVICE_OFF parameter is specified, 
	the location service is disabled. All the ongoing position requests will be terminated.
	Incoming position requests will be rejected until another service type is set.


	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inServiceType (in) A set of LCS services to be set. This is a bitmask, which
				may be one or more of the following flags: 			

					- LCS_SERVICE_AUTONOMOUS	
                    - LCS_SERVICE_E911		
					- LCS_SERVICE_SUPL		
					- LCS_SERVICE_LTO			
                    - LCS_SERVICE_ALL			
                    - LCS_SERVICE_OFF			
					.
					
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SERVICE_CONTROL_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsResult_t

	@sa			::LcsServiceType_t

**/
extern void CAPI2_LCS_ServiceControl(UInt32 tid, UInt8 clientID, LcsServiceType_t inServiceType);

//**************************************************************************************
/**
*	Retrieve the current location services. It is a bitmask, which may be one or more of the following flags. 			

					- LCS_SERVICE_AUTONOMOUS	
                    - LCS_SERVICE_E911		
					- LCS_SERVICE_SUPL		
					- LCS_SERVICE_LTO			
                    - LCS_SERVICE_ALL			
                    - LCS_SERVICE_OFF			
					.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b Responses 
	@n@b MsgType_t: ::MSG_LCS_SERVICE_QUERY_RSP
	@n@b Result_t: 
	@n@b ResultData: :::LcsServiceType_t

**/
extern void CAPI2_LCS_ServiceQuery(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	 Register a SUPL message handler with the LCS API. This method should 
	 be called only once after the phone is powered on. The registered client should 
	 be responsible to handle the following SUPL messages.
	 If no SUPL message handler is registered the SUPL messages will be broadcasted

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID	(in)	The ID of the client that will handle the SUPL related messages.
	
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_REGISTER_SUPL_MSG_HANDLER_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: n/a

	@sa 		The handler will receive following SUPL Messages:<br>
				::MSG_LCS_SUPL_INIT_HMAC_REQ<br>
				::MSG_LCS_SUPL_NOTIFICATION_IND<br>
				::MSG_LCS_SUPL_CONNECT_REQ<br>
				::MSG_LCS_SUPL_DISCONNECT_REQ<br>

**/
extern void CAPI2_LCS_RegisterSuplMsgHandler(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	This function starts a periodic position fixing request. 
	The client will receive a ::MSG_LCS_REQ_RESULT_IND with LcsPosReqResult_t payload.

	When LCS_OUTPUT_RAW_POS_DETAIL_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_POSITION_DETAIL_IND message 
	with LcsPosDetail_t payload whenever a position fix is available. The size of LcsPosDetail_t is much bigger than LcsPosInfo_t. 
	If it is not necessary to get all data in LcsPosDetail_t, LCS_OUTPUT_RAW_POS_INFO_MASK mask should be used.

	When LCS_OUTPUT_RAW_POS_INFO_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_POSITION_INFO_IND message 
	with LcsPosInfo_t payload whenever a position fix is available. After receiving this message, 
	the client can call CAPI2_LCS_GetPosition() to retrieve the detailed position fix data. (Use LCS_OUTPUT_RAW_POS_DETAIL_MASK for best performance)

	When LCS_OUTPUT_NEMA_XXX_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_NMEA_READY_IND message 

	LCS_StopPosReq() should be invoked to stop the position fixing request.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inPosOutputMask (in) The position output mask. It can be one or more of the LCS_OUTPUT_XXXX_MASKs.					
	@param		inPosFixPeriod (in) The desired period of position fixing in milliseconds. 
						Period of 0 indicates continuous position fixing at the native 
						measurement rate of the receiver. Period of -1 indicates single position fix.
	
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_START_POS_REQ_PERIODIC_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsHandle_t (The handle of created request)

	@sa 		Client may receive following messages accroding to inPosOutputMask<br>
				::MSG_LCS_REQ_RESULT_IND  ::LcsPosReqResult_t<br>
				::MSG_LCS_POSITION_DETAIL_IND ::LcsPosDetail_t<br>
				::MSG_LCS_POSITION_INFO_IND ::LcsPosInfo_t<br>
				::MSG_LCS_NMEA_READY_IND ::LcsNmeaData_t<br>

	@note 		For CAPI2, if you need detailed info, use ::MSG_LCS_POSITION_DETAIL_IND will be 
	more efficient than ::MSG_LCS_POSITION_INFO_IND / CAPI2_LCS_GetPosition() 
**/
extern void CAPI2_LCS_StartPosReqPeriodic(UInt32 tid, UInt8 clientID, UInt32 inPosOutputMask, UInt32 inPosFixPeriod );

//**************************************************************************************
/**
	This function starts a single position fixing request. 
	The caller will receive a ::MSG_LCS_REQ_RESULT_IND with LcsPosReqResult_t payload.

	When LCS_OUTPUT_RAW_POS_INFO_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_POSITION_INFO_IND message 
	with LcsPosInfo_t payload whenever a position fix is available. After receiving this message, 
	the client can call LCS_GetPosition() to retrieve the detailed position fix data.

	When LCS_OUTPUT_RAW_POS_DETAIL_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_POSITION_DETAIL_IND message 
	with LcsPosDetail_t payload whenever a position fix is available. The size of LcsPosDetail_t is much bigger than LcsPosInfo_t. 
	If it is not necessary to get all data in LcsPosDetail_t, LCS_OUTPUT_RAW_POS_INFO_MASK mask should be used.
	When LCS_OUTPUT_NEMA_XXX_MASK is specified in inPosOutputMask, the client will receive ::MSG_LCS_NMEA_READY_IND message.

	LCS_StopPosReq() should be invoked to stop the position fixing request.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inPosOutputMask (in) The position output mask. It can be one or more of the LCS_OUTPUT_XXXX_MASKs.						
	@param		inAccuracyInMeters (in) The position data accuracy in meters.
	@param		inPosFixTimeOut (in) The response timeout in seconds.
	
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_START_POS_REQ_SINGLE_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsHandle_t (The handle of created request)

	@sa 		Client may receive following messages accroding to inPosOutputMask<br>
				::MSG_LCS_REQ_RESULT_IND  ::LcsPosReqResult_t<br>
				::MSG_LCS_POSITION_DETAIL_IND ::LcsPosDetail_t<br>
				::MSG_LCS_POSITION_INFO_IND ::LcsPosInfo_t<br>
				::MSG_LCS_NMEA_READY_IND ::LcsNmeaData_t<br>

	@sa 		::CAPI2_LCS_StartPosReqPeriodic<br>
				::CAPI2_LCS_StopPosReq

	@note 		For CAPI2, if you need detailed info, use ::MSG_LCS_POSITION_DETAIL_IND will be more 
				efficient than ::MSG_LCS_POSITION_INFO_IND / CAPI2_LCS_GetPosition() 
**/
extern void CAPI2_LCS_StartPosReqSingle(UInt32 tid, UInt8 clientID, UInt32 inPosOutputMask, UInt32 inAccuracyInMeters, UInt32 inPosFixTimeOut);

//**************************************************************************************
/**
	Stop the running position fixing that is started by CAPI2_LCS_StartPosReqPeriodic() or CAPI2_LCS_StartPosReqSingle().

	@param		tid (in)Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inLcsHandle (in) The position fix handle to be stopped.
	
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_STOP_POS_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsResult_t

	@sa 		Client may receive following messages(s):<br>
				::MSG_LCS_REQ_RESULT_IND  ::LcsPosReqResult_t<br>

**/
extern void CAPI2_LCS_StopPosReq(UInt32 tid, UInt8 clientID, LcsHandle_t inLcsHandle);

//**************************************************************************************
/**
	Retrieve the detailed information of the available position fix.
    Call when ::MSG_LCS_POSITION_INFO_IND is received.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_GET_POSITION_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsPosData_t

	@note 		For CAPI2, if you need detailed info, use ::MSG_LCS_POSITION_DETAIL_IND will be more 
				efficient than ::MSG_LCS_POSITION_INFO_IND / CAPI2_LCS_GetPosition() 

**/
extern void CAPI2_LCS_GetPosition(UInt32 tid, UInt8 clientID);

//**************************************************************************************
/**
	This function should be invoked for respond to ::MSG_LCS_SUPL_NOTIFICATION_IND

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSessionHdl (in)	The SUPL session handle.
	@param		inIsAllowed (in) 1 - allow positioning request, 0 - deny

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SUPL_VERIFICATION_RSP_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: n/a

	@note		This function should be called as a respond to ::MSG_LCS_SUPL_NOTIFICATION_IND
				Application should response with LCS_SuplVerificationRsp only if notification type is
				LCS_SUPL_NOTIFICATION_AND_VERIFICATION_ALLOWED or LCS_SUPL_NOTIFICATION_AND_VERIFICATION_DENIED.

	@sa 		::MSG_LCS_SUPL_NOTIFICATION_IND ::LcsSuplNotificationData_t<br>

**/
extern void CAPI2_LCS_SuplVerificationRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, Boolean inIsAllowed);

//*******************************************************************************
/**
	This function should be invoked by LCS client in response to ::MSG_LCS_SUPL_CONNECT_REQ.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSessionHdl	(in)	The SUPL session handle.
	@param		inConnectHdl	(in)	The created SUPL connection handle. If the connection is failed, the handle should be NULL.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SUPL_CONNECT_RSP_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: ::LcsResult_t

	@sa			::MSG_LCS_SUPL_CONNECT_REQ LcsSuplConnection_t<br>
	
*******************************************************************************/
extern void CAPI2_LCS_SuplConnectRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl);

//**************************************************************************************
/**
	This function notifies the LCS module that there is data received from the SUPL IP connection.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSessionHdl (in)	The SUPL session handle.
	@param		inConnectHdl (in)	The SUPL connection handle. 
	@param		inData (in)	The received data buffer.
	@param		inDataLen (in)	The received data length.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SUPL_DATA_AVAILABLE_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a
	
**/
extern void CAPI2_LCS_SuplDataAvailable(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl, const UInt8 *inData, UInt32 inDataLen);

//**************************************************************************************
/**
	This function notify the LCS module that the SUPL IP connection is disconnected.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSessionHdl (in)	The SUPL session handle.
	@param		inConnectHdl (in)	The SUPL connection handle.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SUPL_DISCONNECTED_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@note		This function should be called during active SUPL session 
				to inform SUPL protocol stack that TCP/IP connection have been 
				terminated for any reason
**/
extern void CAPI2_LCS_SuplDisconnected(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl);

//**************************************************************************************
/**
*	This function should be called as a respond to ::MSG_LCS_SUPL_INIT_HMAC_REQ.
*
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSessionHdl (in)	The SUPL session handle.
	@param		inSuplInitData (in) This buffer should contain the same SUPL INIT data provided in the 
								payload (LcsSuplData_t) of ::MSG_LCS_SUPL_INIT_HMAC_REQ.
	@param		inSuplInitDataLen (in) This value should be the same SUPL INIT data length provided in the 
								payload (LcsSuplData_t) of ::MSG_LCS_SUPL_INIT_HMAC_REQ.
	@param		inSuplHmac (in) This result HMAC (8 bytes) data.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SUPL_INIT_HMAC_RSP_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_SUPL_INIT_HMAC_REQ  ::LcsSuplData_t<br>
			
**/
extern void CAPI2_LCS_SuplInitHmacRsp(UInt32 tid, UInt8 clientID, LcsSuplSessionHdl_t inSessionHdl, unsigned char *	inSuplInitData, UInt32 inSuplInitDataLen, SuplHmac_t inSuplHmac );

/** @} */

extern void CAPI2_LCS_ConfigSet(UInt32 tid, UInt8 clientID, UInt32 configId, UInt32 value);

extern void CAPI2_LCS_ConfigGet(UInt32 tid, UInt8 clientID, UInt32 configId);

extern void CAPI2_LCS_CmdData(UInt32 tid, UInt8 clientID, CAPI2_LcsCmdData_t inCmdData);

//**************************************************************************************
/**
	This function CAPI2_L1_bb_isLocked reports if the AFC algorithm in the L1 code is locked to
	a base station now, and over the recent history. The lock statistics are cleared when this is called.
	Statistics are kept of loss of lock, etc. until the next call to L1_bb_isLocked().
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		watch (in) FALSE - The L1 AFC returns the current state; clears the history; and starts watching the history.
					       TRUE  - The L1 AFC returns the current state ANDed with the history and stop watching the AFC history.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_L1_BB_ISLOCKED_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :Boolean
	
**/
extern void CAPI2_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch);

//*******************************************************************************
/**
*	Start Control Plane Mobile Originated Location Request (MO-LR).
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inClientInfo			(in) Pointer of the client information of the caller.
*	@param inCpMoLrReq	(in) The MO-LR request parameter.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_MNSS_CLIENT_LCS_SRV_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : LCS_SrvRsp_t
	
**/
extern void CAPI2_LCS_CpMoLrReq(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq)
;

//*******************************************************************************
/**
*	Abort the ongoing Control Plane Mobile Originated Location Request (MO-LR)
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inClientInfo			(in) Pointer of the client information of the caller.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMOLRABORT_RSP
	@n@b Result_t :		::RESULT_OK

**/
extern void CAPI2_LCS_CpMoLrAbort(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr);

//*******************************************************************************
/**
*	Respond the Control Plane Location Notification request.
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inClientInfo			(in) Pointer of the client information of the caller.
*	@param inVerificationRsp	(in) The verification response parameter.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMTLRVERIFICATIONRSP_RSP
	@n@b Result_t :		::RESULT_OK

**/

extern void CAPI2_LCS_CpMtLrVerificationRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp);

//*******************************************************************************
/**
*	Generic response of the MT-LR request.
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inClientInfo			(in) Pointer of the client information of the caller.
*	@param inOperation	(in) The operation of the original request.
*	@param inIsAccepted	(in) The Area Event Report parameter.
*

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMTLRRSP_RSP
	@n@b Result_t :	  ::RESULT_OK

**/

extern void CAPI2_LCS_CpMtLrRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted);

//*******************************************************************************
/**
*	Respond the Location Update request.
*	If inTerminationCause is not NULL, the current location procedure will be terminated.
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inClientInfo			(in) Pointer of the client information of the caller.
*	@param inTerminationCause	(in) The termination cause.
*

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPLOCUPDATERSP_RSP
	@n@b Result_t :	  ::RESULT_OK

**/


extern void CAPI2_LCS_CpLocUpdateRsp(UInt32 tid, UInt8 clientID, ClientInfo_t *inClientInfoPtr, const LCS_TermCause_t *inTerminationCause);

//*******************************************************************************
/**
*	Decode the location estimate data. (see 3GPP TS 23.032)
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inLocEstData	(in) The location estimate data.
*

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_DECODEPOSESTIMATE_RSP
	@n@b Result_t :	  ::RESULT_OK
	@n@b ResultData : LcsPosEstimateInfo_t

**/

extern void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData);


//*******************************************************************************
/**
*	Encode the assistance data from LcsAssistanceReq_t to LCS_GanssAssistData_t.
*	(see 3GPP TS 49.031 V5.3.0, Section 10.10)
*
	@param tid (in) Unique exchange/transaction id which is passed back in the response
*	@param clientID	(in) The client ID returned by SYS_RegisterForMSEvent().
*	@param inAssistReq		(in) The original assistance request.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_ENCODEASSISTANCEREQ_RSP
	@n@b Result_t :	  ::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : LCS_GanssAssistData_t
	
**/

extern void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq);



/** @} */

#ifdef __cplusplus
}
#endif

#endif	// CAPI2_LCS_API_H__
