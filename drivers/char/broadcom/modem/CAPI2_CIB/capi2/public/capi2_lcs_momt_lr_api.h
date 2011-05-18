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
*   @file   capi2_lcs_momt_lr_api.h
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

#ifndef _CAPI2_LCS_MOMT_LR_API_H__
#define _CAPI2_LCS_MOMT_LR_API_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------

/**
* @addtogroup  CAPI2_LCSAPIGroup
* @{
*/

//***************************************************************************************
/**
	Start Control Plane Mobile Originated Location Request MO-LR.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCpMoLrReq (in)  The MO-LR request parameter.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_MNSS_CLIENT_LCS_SRV_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::LCS_SrvRsp_t
**/
void CAPI2_LCS_CpMoLrReq(ClientInfo_t* inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq);

//***************************************************************************************
/**
	Abort the ongoing Control Plane Mobile Originated Location Request MO-LR
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_CPMOLRABORT_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LCS_CpMoLrAbort(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Respond the Control Plane Location Notification request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inVerificationRsp (in)  The verification response parameter.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_CPMTLRVERIFICATIONRSP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LCS_CpMtLrVerificationRsp(ClientInfo_t* inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp);

//***************************************************************************************
/**
	Generic response of the MT-LR request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inOperation (in)  The operation of the original request.
	@param		inIsAccepted (in)  The Area Event Report parameter.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_CPMTLRRSP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LCS_CpMtLrRsp(ClientInfo_t* inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted);

//***************************************************************************************
/**
	Respond the Location Update request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTerminationCause (in)  The termination cause.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_CPLOCUPDATERSP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LCS_CpLocUpdateRsp(ClientInfo_t* inClientInfoPtr, const LCS_TermCause_t *inTerminationCause);

//***************************************************************************************
/**
	Decode the location estimate data. see 3GPP TS 23.032
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inLocEstData (in)  The location estimate data.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_DECODEPOSESTIMATE_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : void
**/
void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData);

//***************************************************************************************
/**
	Encode the assistance data from LcsAssistanceReq_t to LCS_GanssAssistData_t.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inAssistReq (in)  The original assistance request.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_ENCODEASSISTANCEREQ_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : int
**/
void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq);

//***************************************************************************************
/**
	Start a FTT synchronization request
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_LCS_FTT_SYNC_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : N/A
**/
void CAPI2_LCS_FttSyncReq(ClientInfo_t* inClientInfoPtr);


/** @} */

#ifdef __cplusplus
}
#endif

#endif	// _CAPI2_LCS_MOMT_LR_API_H__
