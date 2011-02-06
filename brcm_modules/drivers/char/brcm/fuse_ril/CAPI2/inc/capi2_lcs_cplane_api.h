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
*   @file   capi2_lcs_cplane_api.h
*
*   @brief  This header provides the interface description for the CAPI2 Location Service control plane API.
*
****************************************************************************/
/**
*   @defgroup    CAPI2_LCS_CPLANE_APIGroup   Location Service Control Plane
*   @ingroup     CAPI2_LCSGroup
*
*   @brief      This group defines the interfaces to the location service control plane APIs.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CAPI2_LCS_CPLANE_API_H
#define CAPI2_LCS_CPLANE_API_H		///< Include guard.

// ---- Include Files -------------------------------------------------------
#include "capi2_types.h"
#include "capi2_resultcode.h"
#include "capi2_lcs_cplane_ds.h"
/**
* @addtogroup  CAPI2_LCS_CPLANE_APIGroup
* @{
*/

//**************************************************************************************
/**
*	Request to send the provided RRLP data to control plane network.
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
*	@param		inRrlpHandler	(in) The registered RRLP message handler 
*	@param		inData			(in) The buffer containing the RRLP data to be sent.
*	@param		inDataLen    	(in) The RRLP data length.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_SEND_RRLP_DATA_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_SEND_RRLP_DATA_REQ  ::CAPI2_LCS_RrlpData_t<br>
			
**/
extern void CAPI2_LCS_SendRrlpDataToNetwork(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrlpHandler, const UInt8 *inData, UInt32 inDataLen);

//**************************************************************************************
/**
* Register a RRLP message handler with the LCS API. 
* This method should be called only once. The registered client should 
* be responsible to handle the following RRLP messages.
* If no RRLP message handler is registered the RRLP messages will not be sent.
*
*	- MSG_LCS_RRLP_DATA_IND                 Payload type {::LcsMsgData_t} The RRLP data received from network
*	- MSG_LCS_RRLP_RESET_STORED_INFO_IND    Payload type {::LcsClientInfo_t} The request to reset the store position information.
*	.
*
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
*	@param		inRrlpHandler	(in) The info of the client that will handle the RRLP messages.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_REG_RRLP_HDL_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_REG_RRLP_HDL_REQ  ::LcsClientInfo_t<br>
			
**/
extern void CAPI2_LCS_RegisterRrlpDataHandler(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrlpHandler);

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
*	- MSG_LCS_RRC_STOP_MEASUREMENT_IND		    Payload type {::LcsClientInfo_t} 
*	- MSG_LCS_RRC_RESET_POS_STORED_INFO_IND	    Payload type {::LcsClientInfo_t} 
*	.
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
	@param		inRrcHandler	(in)	The info of the client that will handle the RRC messages.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_REG_RRC_HDL_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_REG_RRC_HDL_REQ  ::LcsClientInfo_t<br>
			
**/
extern void CAPI2_LCS_RegisterRrcDataHandler(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrcHandler);

//**************************************************************************************
/**
* Send measurement report to network.
*
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
*	@param inRrcHandler 	(in) The registered RRC message handler 
*	@param inMeasurementId	(in) The measurement ID
*	@param inMeasData	    (in) The measurement result data.
*	@param inMeasDataLen	(in) The measurement result data length
*	@param inEventData	    (in) The event result data
*	@param inEventDataLen	(in) The event result data length
*	@param inAddMeasResult	(in) Add measurement result flag

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_MEAS_REPORT_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_MEAS_REPORT_REQ  ::CAPI2_LCS_RrcMeasReport_t<br>
			
**/
extern void CAPI2_LCS_RrcMeasurementReport(UInt32 tid, UInt8 clientID, 
										   LcsClientInfo_t inRrcHandler, 
											UInt16 inMeasurementId, 
                                           UInt8* inMeasData, 
                                           UInt32 inMeasDataLen, 
                                           UInt8* inEventData,
                                           UInt32 inEventDataLen,
                                           Boolean inAddMeasResult);

//**************************************************************************************
/**
* Send measurement contro failure to network.
*
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
*	@param inRrcHandler 	(in) The registered RRC message handler 
*	@param inTransactionId	(in) The transaction ID
*	@param inMcFailure	    (in) The measurement control failure code.
*	@param inErrorCode	    (in) The error code. This filed is reserved for future use.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_MEAS_FAILURE_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_MEAS_FAILURE_REQ  ::CAPI2_LCS_RrcMeasFailure_t<br>
			
**/
extern void CAPI2_LCS_RrcMeasurementControlFailure(UInt32 tid, UInt8 clientID, 
												   LcsClientInfo_t inRrcHandler, 
												   UInt16 inTransactionId, 
												   LcsRrcMcFailure_t inMcFailure, 
												   UInt32 inErrorCode);

//**************************************************************************************
/**
* Send RRC status to network.
*
*
	@param		tid				(in) Unique exchange/transaction id which is passed back in the response
	@param		clientID		(in) Client ID
*	@param		inRrcHandler 	(in) The registered RRC message handler 
*	@param		inStatus	        (in) The RRC status.

	@n@b 		Responses 
	@n@b 		MsgType_t: ::MSG_LCS_RRC_STATUS_RSP
	@n@b 		Result_t: 
	@n@b 		ResultData: :n/a

	@sa			::MSG_LCS_RRC_STATUS_REQ  ::CAPI2_LCS_RrcStatus_t<br>
			
**/
extern void CAPI2_LCS_RrcStatus(UInt32 tid, UInt8 clientID, LcsClientInfo_t inRrcHandler, LcsRrcMcStatus_t inStatus);

/** @} */



#ifdef __cplusplus
}
#endif

#endif	// CAPI2_LCS_API_H__
