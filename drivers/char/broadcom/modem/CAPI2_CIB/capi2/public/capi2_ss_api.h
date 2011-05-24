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
*   @file   capi2_ss_api.h
*
*   @brief  This file defines the interface for Supplementary Services API for the CAPI2 interface
*
*			This file defines the interface for supplementary services API.  This file provides
*			the function prototypes necessary to activate, register, interrogate, enable & disable
*			supplementary services such as call forwarding, call barring, calling line presentation
*			for speech, data, fax and other call types. 
********************************************************************************************/
/**

*   @defgroup    CAPI2_SSAPIGroup   Supplementary Services
*   @ingroup     CAPI2_SSGroup
*
*   @brief      This group defines the interfaces to the supplementary services.
*
********************************************************************************************/

#ifndef _CAPI2_SS_API_H_
#define _CAPI2_SS_API_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup  CAPI2_SSAPIGroup
 * @{
 */

//***************************************************************************************
/**
	Function pars the SS String then it initiats a new SS request which causes the <br>MNSS state machine to establish a SS session.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDialStrSrvReqPtr (in)  Dial Str Service Request Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_DIALSTRSRVREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_DialStrSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr);

//***************************************************************************************
/**
	Function to initiate a SS or USSD session with the network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in)  Pointer to the SS API service request object.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_SSSRVREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_SsSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);

//***************************************************************************************
/**
	Function to initiate a new USSD request. which causes the MNSS state machine to <br>establish a USSD session.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inUssdSrvReqPtr (in) Param is inUssdSrvReqPtr
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_USSDSRVREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_UssdSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdSrvReq_t *inUssdSrvReqPtr);

//***************************************************************************************
/**
	Function to continues the current USSD session with the network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inUssdDataReqPtr (in) Param is inUssdDataReqPtr
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_USSDDATAREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_UssdDataReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdDataReq_t *inUssdDataReqPtr);

//***************************************************************************************
/**
	Function to initiate a release request which causes the MNSS state machine to <br>send a release complete message to the network. There is no response for this request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in)  Pointer to the SS API service request object.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_SSRELEASEREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_SsReleaseReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);

//***************************************************************************************
/**
	Function to continues the current SS session with the network. The response <br>primitive which caries the network respone back to the client;
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDataReqPtr (in)  Pointer to the SS API data request object.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SSAPI_DATAREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_DataReq(ClientInfo_t* inClientInfoPtr, SsApi_DataReq_t *inDataReqPtr);

//***************************************************************************************
/**
	Function to ResetSsAlsFlag
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SS_RESETSSALSFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SsApi_ResetSsAlsFlag(UInt32 tid, UInt8 clientID);




/** @} */



#ifdef __cplusplus
}
#endif

#endif // _CAPI2_SS_API_H_

