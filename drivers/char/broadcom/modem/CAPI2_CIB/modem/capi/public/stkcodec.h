//***************************************************************************
//
//	Copyright © 2001-2008 Broadcom Corporation
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
*   @file   stkcodec.h
*
*   @brief  This file contains definitions for STKCODEC.
*
* 	@note	Modified from Marie1 project to work on GPRS project
*
*****************************************************************************/

#ifndef _STK_CODEC_H_
#define _STK_CODEC_H_


#define	STKDEC_LENGTH_TWOBYTE_FLAG		0x81

/**
    STK decode result codes
**/
typedef enum{
	SDR_SUCCESS,                ///< Success
	SDR_CMDTYPE_NOT_UNDERSTOOD, ///< Command type not understood. Stack handle this
	SDR_CMDDATA_NOT_UNDERSTOOD, ///< Command data not understood. This layer and stack handle this
	SDR_REQUIRED_VALUE_MISSING  ///< Required value missing. This layer and stack handle this but currently incomplete
} STKDECResult_t;

/**
    Decode Display Text according to 11.14/6.6.1
**/
void STKDEC_DispTxtReq(
	SIMParmDisplayTextReq_t	*pDispTxtReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Setup Idle according to 11.14/6.6.22
**/
void STKDEC_SetupIdleTxtReq(
	SIMParmSetupIdlemodeTextReq_t	*pSetupIdleTxtReq,
	STKMsg_t						*pStkMsg
	);

/**
    Decode Get Input according to 11.14/6.6.3
**/
void STKDEC_GetInputReq(
	SIMParmGetInputReq_t	*pGetInputReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Get Inkey according to 11.14/6.6.2
**/
void STKDEC_GetInkeyReq(
	SIMParmGetInkeyReq_t	*pGetInkeyReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Play Tone according to 11.14/6.6.5
**/
void STKDEC_PlayToneReq(
	SIMParmPlayToneReq_t	*pPlayToneReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Select Item according to 11.14/6.6.8
**/
void STKDEC_SelItemReq(
	SIMParmSelectItemReq_t	*pSelItemReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Setup Menu according to 11.14/6.6.7
**/
Boolean STKDEC_SetupMenuReq(
	SIMParmSetupMenuReq_t	*pSetupMenuReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Refresh according to 11.14/6.6.13
**/
void STKDEC_StkRefreshReq(
	SIMParmSimtoolkitRefreshReq_t	*pStkRefreshReq,
	STKMsg_t						*pStkMsg
	);

/**
    Decode Refresh according to 11.14/6.6.13
**/
void STKDEC_SIAT_StkRefreshReq(
	SIMParmSiatSimtoolkitRefreshReq_t	*pSiatStkRefreshReq,
	STKMsg_t						*pStkMsg
	);

/**
    Decode Send SS according to 11.14/6.6.10
**/
void STKDEC_SendSSReq(
	SIMParmSendSsReq_t		*pSendSSReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode MO SMS according to 11.14/6.6.9
**/
void STKDEC_SendMoSMSReq(
	SIMParmMoSMSAlphaInd_t	*pSendMoSMSAplhaInd,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Setup Call according to 11.14/6.6.12
**/
void STKDEC_SetupCallReq(
	SIMParmSetupCallReq_t	*pSetupCallReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode Send DTMF according to 11.14/6.6.24
**/
void STKDEC_SendDtmfReq(
	SIMParmSendDtmfReq_t	*pSendDtmfReq,
	STKMsg_t				*pStkMsg
	);

/**
    Decode SMS Coding scheme according to 11.14/6.6.9
**/
void STKENC_SMSCodingScheme(
	Unicode_t		UnicodeType,
	Boolean			IsPacked,
	UInt8			*pCodingSchemeByte	//output argument
	);

/**
    Converts octet to septet
    @param  p_src (in) source of octets
    @param  p_dest (out) destination of septets
    @param  num_of_octets (in) Number of input octets
**/
UInt16 STKENC_Octet2Septet(
	const UInt8	*p_src,
	UInt8		*p_dest,
	UInt16		num_of_octets
	);

/**   
    This is a special version of the strncpy, where remainder
    of the string is filled with 0xFF and converts Default GSM
    value for '@', which is 0x00 to CODE_PAGE_MTI_AT_SIGN
    @param  pTxt (in) input text
    @param  Length (in) input text length
    @param  pStkTxt (out) output text

    @note: refer to GSM 11.14/12.2
**/
void STKDEC_AlphaIdentifier(
	UInt8					*pTxt,
	UInt16					Length,
	STKAlphaIdentifier_t	*pStkTxt
	);

/**
    Decodes the passed "si_icon_id" to the "STKIconId_t" structure. 
    See Section 12.31 of GSM 11.14 for Icon Identifier TLV format.
    @param  si_icon_id (in)  input icon id structure
    @param  stk_icon_id (out) output icon id structure
**/
void STKDEC_IconIdentifier(const T_SI_ICON_ID *si_icon_id, STKIconId_t *stk_icon_id);

/**
    Decode Duration TLV. refer to GSM 11.14/12.8
    @param  TimeUnitByte (in) Time unit
    @param  TimeIntervalByte (in) Time interval
    @return Time in msec
**/
UInt32 STKDEC_Duration(UInt8 TimeUnitByte, UInt8 TimeIntervalByte);

/**
    Decode text string TLV. refer to GSM 11.14/12.15
    @param pTxt (in) Pointer to raw text
    @param CodingByte (in)  Coding byte
    @param Length (in) Length of raw text
    @param pStkTxt (out)  Pointer to STK formatted text structure
    @param event (in) Event assoicated with the text
**/
void STKDEC_TxtString(UInt8	*pTxt, UInt8 CodingByte, UInt16 Length,	STKTxt_t *pStkTxt,  SATK_EVENTS_t event);

/**
    Decodes a SIM Toolkit TLV according to Annex D of GSM 11.14. 
    @param tlv (in) Pointer to the STK TLV to decode
    @param tag_value (out)  Tag value to return
    @param value_offset (out) Offset to the value field
    @param value_len (out) Number of bytes of the value field

    @return Number of bytes of the whole STK TLV
**/
UInt16 STKDEC_TLV(const UInt8 *tlv, UInt8 *tag_value, UInt8 *value_offset, UInt8 *value_len);

#endif	//_STK_CODEC_H_

