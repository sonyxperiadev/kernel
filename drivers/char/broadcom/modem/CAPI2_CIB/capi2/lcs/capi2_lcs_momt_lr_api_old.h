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
*   @file   capi2_lcs_momt_lr_api_old.h
*
*   @brief  This header provides the interface description for the CAPI2 Location Service API.
*
****************************************************************************/
/**
*   @defgroup    CAPI2_LCSAPIOLDGroup   Location Service
*   @ingroup     CAPI2_LCSOLDGroup
*
*   @brief      This group defines the interfaces to the location service API.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CAPI2_LCS_MOMT_LR_API_OLD_H__
#define _CAPI2_LCS_MOMT_LR_API_OLD_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------

/**
* @addtogroup  CAPI2_LCSAPIOLDGroup
* @{
*/

//*******************************************************************************
/**
*	Start Control Plane Mobile Originated Location Request (MO-LR).
*
*	@param inClientInfoPtr	(in) Pointer of the client information of the caller.
*	@param inCpMoLrReq	(in) The MO-LR request parameter.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_MNSS_CLIENT_LCS_SRV_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : LCS_SrvRsp_t
	
**/
extern void CAPI2_LCS_CpMoLrReq( ClientInfo_t *inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq)
;

//*******************************************************************************
/**
*	Abort the ongoing Control Plane Mobile Originated Location Request (MO-LR)
*
*	@param inClientInfoPtr	(in) Pointer of the client information of the caller.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMOLRABORT_RSP
	@n@b Result_t :		::RESULT_OK

**/
extern void CAPI2_LCS_CpMoLrAbort(  ClientInfo_t *inClientInfoPtr);

//*******************************************************************************
/**
*	Respond the Control Plane Location Notification request.
*
*	@param inClientInfoPtr 		(in) Pointer of the client information of the caller.
*	@param inVerificationRsp	(in) The verification response parameter.

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMTLRVERIFICATIONRSP_RSP
	@n@b Result_t :		::RESULT_OK

**/

extern void CAPI2_LCS_CpMtLrVerificationRsp(  ClientInfo_t *inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp);

//*******************************************************************************
/**
*	Generic response of the MT-LR request.
*
*	@param inClientInfoPtr 	(in) Pointer of the client information of the caller.
*	@param inOperation	(in) The operation of the original request.
*	@param inIsAccepted	(in) The Area Event Report parameter.
*

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPMTLRRSP_RSP
	@n@b Result_t :	  ::RESULT_OK

**/

extern void CAPI2_LCS_CpMtLrRsp( ClientInfo_t *inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted);

//*******************************************************************************
/**
*	Respond the Location Update request.
*	If inTerminationCause is not NULL, the current location procedure will be terminated.
*
*	@param inClientInfoPtr		(in) Pointer of the client information of the caller.
*	@param inTerminationCause	(in) The termination cause.
*

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_LCS_CPLOCUPDATERSP_RSP
	@n@b Result_t :	  ::RESULT_OK

**/


extern void CAPI2_LCS_CpLocUpdateRsp(ClientInfo_t *inClientInfoPtr, const LCS_TermCause_t *inTerminationCause);

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

#endif	// _CAPI2_LCS_MOMT_LR_API_H__
