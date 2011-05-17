//***************************************************************************
//
//	Copyright © 2004-2010 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   phonebk_api_old.h
*
*   @brief  This file defines prototypes for the OLD Phonebook API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _PHONEBK_API_OLD_H_
#define _PHONEBK_API_OLD_H_


/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old Phonebook API functions which are obsolete, will be removed
//
void PBK_GetAlpha(char* number,	PBK_API_Name_t* alpha);
Boolean PBK_IsEmergencyCallNumber(char *phone_number);
Boolean PBK_IsPartialEmergencyCallNumber(char *phone_number);
Result_t PBK_SendInfoReq( UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendFindAlphaMatchMultipleReq( UInt8 clientID, PBK_Id_t pbk_id, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendFindAlphaMatchOneReq( UInt8 clientID, UInt8 numOfPbk, PBK_Id_t *pbkId, 
	ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbkAccessCb );
Boolean PBK_IsReady(void);
Result_t PBK_SendReadEntryReq( UInt8 clientID, PBK_Id_t pbk_id, 
					UInt16 start_index, UInt16 end_index, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendWriteEntryReq( UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
				char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUpdateEntryReq( UInt8 clientID, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, 
	char *number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, 
	const USIM_PBK_EXT_DATA_t *usim_adn_ext_data, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendIsNumDiallableReq(UInt8 clientID, char *number, CallbackFunc_t* pbk_access_cb);
Boolean PBK_IsNumDiallable(const char *number, Boolean is_voice_call);
Boolean PBK_IsUssdDiallable(const UInt8 *data, UInt8 dcs, UInt8 len);
Result_t PBK_SendUsimAasInfoReq(UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimAasReadReq(UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimAasUpdateReq( UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUsimGasInfoReq(UInt8 clientID, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimGasReadReq(UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimGasUpdateReq( UInt8 clientID, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, 
								   UInt8 alpha_size, const UInt8 *alpha, CallbackFunc_t* pbk_access_cb );
Result_t PBK_SendUsimHdkReadReq(UInt8 clientID, CallbackFunc_t* pbk_access_cb);
Result_t PBK_SendUsimHdkUpdateReq( UInt8 clientID, const HDKString_t *hidden_key,
								    CallbackFunc_t* pbk_access_cb );
void PBK_SetFdnCheck(Boolean fdn_chk_on);
Boolean PBK_GetFdnCheck(void);
Boolean PBK_IsNumBarred(const char *number, Boolean is_voice_call);
// End of Old API definitions



#endif

