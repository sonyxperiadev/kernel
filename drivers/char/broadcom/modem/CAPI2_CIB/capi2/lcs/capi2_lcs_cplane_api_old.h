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
*   @file   capi2_lcs_cplane_api_old.h
*
*   @brief  This header provides the interface description for the CAPI2 Location Service control plane API.
*
****************************************************************************/
/**
*   @defgroup    CAPI2_LCS_CPLANE_APIOLDGroup   Location Service Control Plane
*   @ingroup     CAPI2_LCSOLDGroup
*
*   @brief      This group defines the interfaces to the location service API.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CAPI2_LCS_CPLANE_API_OLD_H__
#define _CAPI2_LCS_CPLANE_API_OLD_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------

/**
* @addtogroup  CAPI2_LCS_CPLANE_APIOLDGroup
* @{
*/
//**************************************************************************************
/**
* Register a RRLP message handler with the LCS API. 
* This method should be called only once. The registered client should 
* be responsible to handle the following RRLP messages.
* If no RRLP message handler is registered the RRLP messages will not be sent.
*
*	- MSG_LCS_RRLP_DATA_IND                 Payload type {::LcsMsgData_t} The RRLP data received from network
*	- MSG_LCS_RRLP_RESET_STORED_INFO_IND    Payload type {::ClientInfo_t} The request to reset the store position information.
*	.
*
*	@param		inClientInfoPtr	(in) The info of the client that will handle the RRLP messages.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRLP_REG_HDL_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRLP_REG_HDL_REQ  ::ClientInfo_t<br>
			
**/
extern void CAPI2_LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
*	Request to send the provided RRLP data to control plane network.
*
*	@param		inClientInfoPtr	(in) 
*	@param		inData			(in) The buffer containing the RRLP data to be sent.
*	@param		inDataLen    	(in) The RRLP data length.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRLP_SEND_DATA_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRLP_SEND_DATA_REQ  ::CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t<br>
			
**/
extern void CAPI2_LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen);

//**************************************************************************************
/**
* Register a RRC message handler with the LCS API. 
* This method should be called only once. The registered client should 
* be responsible to handle the following RRC messages.
* If no RRC message handler is registered, the RRC messages will not be sent.
* The following messages will be sent to the registered handler.
*
*	- MSG_LCS_RRC_ASSISTANCE_DATA_IND			Payload type {::LcsMsgData_t} 
*	- MSG_LCS_RRC_MEASUREMENT_CTRL_IND		    Payload type {::LcsRrcMeasurement_t} 
*	- MSG_LCS_RRC_BROADCAST_SYS_INFO_IND		Payload type {::LcsRrcBroadcastSysInfo_t} 
*	- MSG_LCS_RRC_UE_STATE_IND				    Payload type {::LcsRrcUeState_t} 
*	- MSG_LCS_RRC_STOP_MEASUREMENT_IND		    Payload type {::ClientInfo_t} 
*	- MSG_LCS_RRC_RESET_POS_STORED_INFO_IND	    Payload type {::ClientInfo_t} 
*	.
*
	@param		inClientInfoPtr	(in)	The info of the client that will handle the RRC messages.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_REG_HDL_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_REG_HDL_REQ  default_proc<br>
			
**/
extern void CAPI2_LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//**************************************************************************************
/**
* Send UL_DCCH RRC data to network.
*
*

*	@param inClientInfoPtr 	(in) The registered RRC message handler 
*	@param inData			(in) The RRC data.
*	@param inDataLen		(in) The length of the RRC data 

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_SEND_DL_DCCH_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_SEND_DL_DCCH_REQ  ::CAPI2_LcsApi_RrcSendUlDcch_Req_t<br>
			
**/
extern void CAPI2_LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8 *inData, UInt32 inDataLen);

//**************************************************************************************
/**
* Send measurement contro failure to network.
*
*
*	@param inClientInfoPtr 	(in) The registered RRC message handler 
*	@param inTransactionId	(in) The transaction ID
*	@param inMcFailure	    (in) The measurement control failure code.
*	@param inErrorCode	    (in) The error code. 

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_MEAS_CTRL_FAILURE_REQ  ::CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t<br>
			
**/
extern void CAPI2_LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode);

//**************************************************************************************
/**
* Send RRC status to network.
*
*	@param		inClientInfoPtr 	(in) The registered RRC message handler 
*	@param		inStatus	        (in) The RRC status.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_STAT_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_STAT_REQ  ::CAPI2_LcsApi_RrcStatus_Req_t<br>
			
**/
extern void CAPI2_LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus);

//**************************************************************************************
/**
	This function CAPI2_LCS_L1_bb_isLocked reports if the AFC algorithm in the L1 code is locked to
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
extern void CAPI2_LCS_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch);

//*******************************************************************************
/**
*	Start a FTT synchronization request
*	The client will receive MSG_LCS_FTT_SYNC_RESULT_IND with LcsFttResult_t payload.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_FTT_SYNC_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :Result_t
*******************************************************************************/
extern void CAPI2_LCS_FttSyncReq( ClientInfo_t *inClientInfoPtr);

//*******************************************************************************
/**
*	Calculate the time difference in micro second between the provided two FTT parameters. 
*
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
*	@param inT1	(in) The first FTT parameter.
*	@param inT2	(in) The second FTT parameter.
*
	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_FTT_SYNC_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :UInt32
*
*******************************************************************************/
void CAPI2_LCS_FttCalcDeltaTime(UInt32 tid, UInt8 clientID, const LcsFttParams_t *inT1, const LcsFttParams_t *inT2);

/** @} */


#ifdef __cplusplus
}
#endif

#endif	// _CAPI2_LCS_CPLANE_API_H__
