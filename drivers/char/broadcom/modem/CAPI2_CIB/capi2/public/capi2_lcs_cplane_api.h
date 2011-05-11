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
*   @brief      This group defines the interfaces to the location service API.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CAPI2_LCS_CPLANE_API_H__
#define _CAPI2_LCS_CPLANE_API_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------

/**
* @addtogroup  CAPI2_LCS_CPLANE_APIGroup
* @{
*/

//***************************************************************************************
/**
	Request to send the provided RRLP data to control plane network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inData (in)  The buffer containing the RRLP data to be sent.
	@param		inDataLen (in)  The RRLP data length.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRLP_SEND_DATA_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen);

//***************************************************************************************
/**
	Register a RRLP message handler with the LCS API.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRLP_REG_HDL_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Register a RRC message handler with the LCS API.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRC_REG_HDL_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Send the measurement result UL_DCCH to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inData (in)  The measurement result data.
	@param		inDataLen (in)  The measurement result data length
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRC_SEND_DL_DCCH_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8 *inData, UInt32 inDataLen);

//***************************************************************************************
/**
	Send the measurement control failure to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTransactionId (in)  The transaction ID
	@param		inMcFailure (in)  The measurement control failure code.
	@param		inErrorCode (in)  The error code. This field is reserved.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode);

//***************************************************************************************
/**
	Send the RRC status to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inStatus (in)  The RRC status.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_RRC_STAT_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus);

//***************************************************************************************
/**
	Get the supported GPS capabilities
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_GET_GPS_CAP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::Result_t
**/
void CAPI2_LcsApi_GetGpsCapabilities(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Set the supported GPS capabilities. This method will detach the network first <br>set the new class mark and attach network again.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCapMask (in)  Bit mask indicates the supported GPS capabilities.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_SET_GPS_CAP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LcsApi_SetGpsCapabilities(ClientInfo_t* inClientInfoPtr, UInt16 inCapMask);

//***************************************************************************************
/**
	Function to L1_bb_isLocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		watch (in) Param is watch
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_L1_BB_ISLOCKED_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_LCS_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch);


/** @} */


#ifdef __cplusplus
}
#endif

#endif	// _CAPI2_LCS_CPLANE_API_H__
