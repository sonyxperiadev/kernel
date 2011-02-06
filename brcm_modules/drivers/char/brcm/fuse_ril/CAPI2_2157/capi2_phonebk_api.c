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
*   @file   capi2_phonebk_api.c
*
*   @brief  This file implements the CAPI2 Phonebook API functions.
*
****************************************************************************/



//******************************************************************************
//	 			include block
//******************************************************************************
#include	"capi2_phonebk_ds.h"
#include	"capi2_reqrep.h"
/**
 * @addtogroup CAPI2_PBKAPIGroup
 * @{
 */

//**************************************************************************************
/**
	Request to find a alpha name within a phonebook.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		number (in) pointer to phone number 
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_GETALPHA_RSP
	@n@b Result_t : 
	@n@b ResultData : PBK_API_Name_t
**/
void CAPI2_PBK_GetAlpha(UInt32 tid, UInt8 clientID, char* number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_GetAlpha_Req.number = number;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_GETALPHA_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	This function returns TRUE if the passed ASCII-coded phone number is one of the 
	emergency numbers which consist of 

	1. When SIM/USIM is present: 
	   The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". 
 
	2. When SIM/USIM is not present:
 	   Number "000", "08", "112", "110", "118", "119", "911" and "999". 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*phone_number (in) pointer to phone number to be checked
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISEMERGENCYCALLNUMBER_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean ///< TRUE if the call is an emergency call	
**/

void CAPI2_PBK_IsEmergencyCallNumber(UInt32 tid, UInt8 clientID, char *phone_number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_IsEmergencyNumber_Req.number = phone_number;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_ISEMERGENCYCALLNUMBER_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	This function returns TRUE if the digits provided in a passed ASCII-coded phone
	number are a subset (starting at the first character) of the emergency call numbers. 
	For example, "9", "91" and "911" are considered to partially match "911" and this 
	function returns TRUE. 

    The emergency numbers consist of the following:
	
	1. When SIM/USIM is present: 
	   The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". 
 
	2. When SIM/USIM is not present:
 	   Number "000", "08", "112", "110", "118", "119", "911" and "999". 
   
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		*phone_number (in) pointer to phone number to be checked
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean ///< TRUE if the number is a partial match			
**/

void CAPI2_PBK_IsPartialEmergencyCallNumber(UInt32 tid, UInt8 clientID, char *phone_number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_IsPartialEmergencyNumber_Req.number = phone_number;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	This function requests the information of a phonebook type, including alpha size, digit size,
	number of total entries, number of free entries, the index of the first free entry and the index
	of the first used entry.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_GET_PBK_INFO_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_INFO_RSP_t

**/

void CAPI2_PBK_SendInfoReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.Send_PBK_Info_Req = pbk_id;
	req.respId = MSG_GET_PBK_INFO_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_SENDINFOREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This functions requests all the phonebook entries that match the passed alpha pattern
	(matching is case-insensitive for GSM Default Alphabet encoding).

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in “alpha”.
	@param		*alpha (in) pointer to actual phonebook alpha data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/

void CAPI2_PBK_SendFindAlphaMatchMultipleReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.PBK_SendFindAlphaMatchMultipleReq_Req.pbk_id = pbk_id;
	req.req_rep_u.PBK_SendFindAlphaMatchMultipleReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.PBK_SendFindAlphaMatchMultipleReq_Req.alpha_size = alpha_size;
	req.req_rep_u.PBK_SendFindAlphaMatchMultipleReq_Req.alpha = alpha;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This function requests the first phonebook entry that matches the passed alpha pattern
	(matching is case-insensitive for GSM default alphabet encoding). The matching is performed
	in the order of the passed phonebook types “pbkId”.

  	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		numOfPbk (in) number of phonebook types pointed by pbkId.
	@param		*pbkId (in) Phone book ID
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in “alpha”.
	@param		*alpha (in) pointer to actual phonebook alpha data
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/

void CAPI2_PBK_SendFindAlphaMatchOneReq( UInt32 tid, UInt8 clientID, UInt8 numOfPbk, PBK_Id_t *pbkId, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_SendFindAlphaMatchOneReq_Req.pbkId = *pbkId;
	req.req_rep_u.CAPI2_PBK_SendFindAlphaMatchOneReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PBK_SendFindAlphaMatchOneReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PBK_SendFindAlphaMatchOneReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_PBK_SendFindAlphaMatchOneReq_Req.numOfPbk = numOfPbk;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SENDFINDALPHAMATCHONEREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	Function to check if the phone book is ready for operation
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISREADY_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean ///< TRUE if ready	 
**/
void CAPI2_PBK_IsReady(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	/* message specific */
	/* NON */

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_ISREADY_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This function requests the phonebook entries whose indices are in the range specified by the
	“startIndex” and “endIndex” arguments.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		start_index (in) Index to start reading from
	@param		end_index (in) Last index to be read from
	@note
	The phonebook task returns the phonebook entries in the MSG_PBK_ENTRY_DATA_RSP
	messages by calling the passed callback function.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_ENTRY_DATA_RSP_t
**/


void CAPI2_PBK_SendReadEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, 
					UInt16 start_index, UInt16 end_index )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_SendReadEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PBK_SendReadEntryReq_Req.end_index = end_index;
	req.req_rep_u.CAPI2_PBK_SendReadEntryReq_Req.start_index = start_index;
	req.respId = MSG_PBK_ENTRY_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SENDREADENTRYREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This function requests to update a phonebook entry.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in) Index to be written to
	@param		type_of_number (in) type of number: one of 128, 129 and 145.
	@param		*number (in) Pointer to number to be written
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in “alpha”.
	@param		*alpha (in) pointer to actual phonebook alpha data

	@note
	If FIRST_PHONEBK_FREE_INDEX is passed in “index”, the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in “number”, the phonebook entry is deleted.
	@n@b Responses 
	@n@b MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_WRITE_ENTRY_RSP_t
**/

void CAPI2_PBK_SendWriteEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
				char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.number.len= (number)?(strlen(number)+1):0;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.number.str= number;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.special_fax_num = special_fax_num;
	req.req_rep_u.CAPI2_PBK_SendWriteEntryReq_Req.type_of_number = type_of_number;
	req.respId = MSG_WRT_PBK_ENTRY_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_SENDWRITEENTRYREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

//**************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PBK_SendWriteEntryReq()
	in that this function supports the extended 3G USIM ADN phonebook data (e.g. additional number and 
	email address). 

	@param		clientID (in) Client ID	to identify response messages
	@param		pbk_id (in) Phone book ID
	@param		special_fax_num (in) set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		0-based index (in) Index to be written to
	@param		type_of_number (in) type of number: one of 128, 129 and 145.
	@param		*number (in) Pointer to number to be written
	@param		alpha_coding (in) Type of coding employed(GSM/UCS..)
	@param		alpha_size (in) number of bytes passed in “alpha”.
	@param		*alpha (in) pointer to actual phonebook alpha data
	@param		*USIM_PBK_EXT_DATA_t (in) extended 3G phonebook data (pass NULL for 2G ADN). The client
				can also pass NULL so that the phonebook module skips updating the extended 3G phonebook files in USIM, i.e. 
				only EF-ADN and EF-EXT1 are updated and other files (e.g. EF-ANR and EF-EMAIL) are not updated at all.
	@param		*pbk_access_cb (in) Phonebook access callback

	@return		Result_t 
	@note
	The phonebook task returns the update result in the MSG_WRT_PBK_ENTRY_RSP message
	by calling the passed callback function.
	If FIRST_PHONEBK_FREE_INDEX is passed in “index”, the entry is written into the first free
	entry found (this is applicable only for PB_ME, PB_ADN and PB_FDN phonebooks).
	If NULL is passed in “number”, the phonebook entry is deleted.
**/
void CAPI2_PBK_SendUpdateEntryReq( UInt32 tid, UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
	char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, USIM_PBK_EXT_DATA_t *usim_adn_ext_data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.pbk_id = pbk_id;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.alpha = alpha;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.alpha_coding = alpha_coding;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.alpha_size = alpha_size;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.index = index;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.number.len= (number)?(strlen(number)+1):0;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.number.str= number;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.special_fax_num = special_fax_num;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.type_of_number = type_of_number;
	req.req_rep_u.CAPI2_PBK_SendUpdateEntryReq_Req.usim_adn_ext_data = usim_adn_ext_data;
	req.respId = MSG_WRT_PBK_ENTRY_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_SENDUPDATEENTRYREQ_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}
//**************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number, use PBK_IsUssdDiallable() API instead. 

  	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		*number (in) Pointer to NULL terminated number to be checked. 
	@param		*pbk_access_cb (in) Phonebook access callback

	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_CHK_PBK_DIALLED_NUM_RSP
	@n@b Result_t : 
	@n@b ResultData : ::PBK_CHK_NUM_DIALLABLE_RSP_t
**/
void CAPI2_PBK_SendIsNumDiallableReq(UInt32 tid, UInt8 clientID, char *number)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_SendIsNumDiallableReq_Req.number = number;
    req.respId = MSG_CHK_PBK_DIALLED_NUM_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_SENDISNUMDIALLABLEREQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This function checks whether the passed non-USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	For USSD number checking, use 
	PBK_IsUssdDiallable() API instead. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		number (in) Buffer that stores the NULL terminated number
	@param		is_voice_call (in) TRUE if the number is for voice call
	 
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISNUMDIALABLE_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean. TRUE if number is diallable; FALSE otherwise. 
**/
void CAPI2_PBK_IsNumDiallable(UInt32 tid, UInt8 clientID, const char *number, Boolean is_voice_call)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_IsNumDiallable_Req.is_voice_call = is_voice_call;
	req.req_rep_u.CAPI2_PBK_IsNumDiallable_Req.number = (char*)number;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_ISNUMDIALLABLE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


//**************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the
	FDN setting and the FDN phonebook contents.	
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID	to identify response messages
	@param		data (in) Buffer that stores the USSD number
	@param		dcs (in) USSD DCS byte in CB DCS format
	@param		len (in) Number of bytes passed in "data". 
	
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_PBK_ISUSSDDIALLABLE_RSP
	@n@b Result_t : 
	@n@b ResultData : Boolean ///< TRUE if USSD is diallable; FALSE otherwise.
**/
void CAPI2_PBK_IsUssdDiallable(UInt32 tid, UInt8 clientID, const UInt8 *data, UInt8 dcs, UInt8 len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 reqrep_len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_PBK_IsUssdDiallable_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_PBK_IsUssdDiallable_Req.dcs = dcs;
	req.req_rep_u.CAPI2_PBK_IsUssdDiallable_Req.len = len;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_PBK_ISUSSDDIALLABLE_REQ, clientID, result, &stream, &reqrep_len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, reqrep_len);
#endif
}




/** @} */

