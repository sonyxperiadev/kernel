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
*   @file   capi2_cp_api.h
*
*   @brief  This file defines the capi2 api's for Communication processor
*
****************************************************************************/
/**

*   @defgroup   CAPI2_CP_APIGroup   Communication Processor API
*
*   @brief      This group defines the interfaces from Communication processor
*               to Application processor.
****************************************************************************/

#ifndef CAPI2_CP_API_H
#define CAPI2_CP_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */


//***************************************************************************************
/**
	Function response for the CAPI2_SMS_IsMeStorageEnabled
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_ISMESTORAGEENABLED_RSP
**/
void CAPI2_SMS_IsMeStorageEnabled_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetMaxMeCapacity
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		capacity (in) Maximum messages
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_GETMAXMECAPACITY_RSP
**/
void CAPI2_SMS_GetMaxMeCapacity_RSP(UInt32 tid, UInt8 clientID, UInt16 capacity);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_GetNextFreeSlot
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		free_slot (in) free slot
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_GETNEXTFREESLOT_RSP
**/
void CAPI2_SMS_GetNextFreeSlot_RSP(UInt32 tid, UInt8 clientID, UInt16 free_slot);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_SetMeSmsStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flag (in) flag
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_SETMESMSSTATUS_RSP
**/
void CAPI2_SMS_SetMeSmsStatus_RSP(UInt32 tid, UInt8 clientID, Boolean flag);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_SetMeSmsStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) status
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_GETMESMSSTATUS_RSP
**/
void CAPI2_SMS_GetMeSmsStatus_RSP(UInt32 tid, UInt8 clientID, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_StoreSmsToMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flag (in) flag
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_STORESMSTOME_RSP
**/
void CAPI2_SMS_StoreSmsToMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_RetrieveSmsFromMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flag (in) flag
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_RETRIEVESMSFROMME_RSP
**/
void CAPI2_SMS_RetrieveSmsFromMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag);

//***************************************************************************************
/**
	Function response for the CAPI2_SMS_RemoveSmsFromMe
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flag (in) flag
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SMS_REMOVESMSFROMME_RSP
**/
void CAPI2_SMS_RemoveSmsFromMe_RSP(UInt32 tid, UInt8 clientID, Boolean flag);

/** @} */

void CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP(UInt32 tid, UInt8 clientID, UInt16 recordNumber);
void CAPI2_SMS_GetMeSmsBufferStatus_RSP(UInt32 tid, UInt8 clientID, UInt32 bfree, UInt32 bused);

#ifdef __cplusplus
}
#endif



#endif

