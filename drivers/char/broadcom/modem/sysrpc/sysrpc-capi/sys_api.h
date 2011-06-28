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

#ifndef _SYS_API_H 
#define _SYS_API_H

typedef UInt8* CAPI2_SYSPARM_IMEI_PTR_t;

//******************************************************************************
/**
	This function to retrieve IMEI from CP.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		imei_type (in) IMEI1 or IMEI2
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::char**
	@note
*/
void CAPI2_SYSPARM_GetImei( UInt32 tid, UInt8 clientID, IMEI_TYPE_t imei_type);


#endif

