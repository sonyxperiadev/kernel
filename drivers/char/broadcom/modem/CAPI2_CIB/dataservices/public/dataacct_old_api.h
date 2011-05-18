//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   dataacct_old_api.h
*
*   @brief  This file contains the interface function prototypes to the Data Account 
			Services.
*
****************************************************************************/
/**

*   @defgroup   DATAAccountGroup   Data Account Management
*   @ingroup    DATAGroup
*
*   @brief      This group defines the interfaces to create and manage 
				data accounts.
*
*	The Data Account Management API provides services to create and 
	manage circuit switch and packet switch accounts.  A data account
	contains all the information that is needed to establish a connection
	with the network.
*
****************************************************************************/

#ifndef _DATAACCT_OLD_API_H_
#define _DATAACCT_OLD_API_H_
/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old Data Account Service API functions
//
Boolean			DATA_IsAcctIDValid(UInt8 acctID);
Result_t		DATA_CreateGPRSDataAcct(UInt8 acctID, GPRSContext_t *pGprsSetting);
Result_t		DATA_CreateCSDDataAcct(UInt8 acctID, CSDContext_t *pCsdSetting);
Result_t		DATA_DeleteDataAcct(UInt8 acctID);
Result_t		DATA_GetGPRSContext(UInt8 acctID, GPRSContext_t *pGprsContext);
Result_t		DATA_GetCSDContext(UInt8 acctID, CSDContext_t *pCsdContext);
Result_t		DATA_SetUsername(UInt8 acctID, UInt8 *username);
UInt8*			DATA_GetUsername(UInt8 acctID);
Result_t		DATA_SetPassword(UInt8 acctID, UInt8 *password);
UInt8*			DATA_GetPassword(UInt8 acctID);
Result_t		DATA_SetStaticIPAddr(UInt8 acctID, UInt8 *staticIPAddr);
const UInt8*	DATA_GetStaticIPAddr(UInt8 acctID);
Result_t		DATA_SetPrimaryDnsAddr(UInt8 acctID, UInt8 *priDnsAddr);
const UInt8*	DATA_GetPrimaryDnsAddr(UInt8 acctID);
Result_t		DATA_SetSecondDnsAddr(UInt8 acctID, UInt8 *sndDnsAddr);
const UInt8*	DATA_GetSecondDnsAddr(UInt8 acctID);
Result_t		DATA_SetDataCompression(UInt8 acctID, Boolean dataCompEnable);
Boolean			DATA_GetDataCompression(UInt8 acctID);
DataAccountType_t		DATA_GetAcctType(UInt8 acctID);
UInt8			DATA_GetEmptyAcctSlot(void);
UInt8			DATA_GetCidFromDataAcctID(UInt8 acctID);
UInt8			DATA_GetDataAcctIDFromCid(UInt8 cid);
UInt8			DATA_GetPrimaryCidFromDataAcctID(UInt8 acctID);
Boolean 		DATA_IsSecondaryDataAcct(UInt8 acctID);
UInt32			DATA_GetDataSentSize(UInt8 acctId);
UInt32			DATA_GetDataRcvSize(UInt8 acctId);
Result_t		DATA_SetGPRSPdpType(UInt8 acctID, UInt8 *pdpType);
const UInt8*	DATA_GetGPRSPdpType(UInt8 acctID);
Result_t		DATA_SetGPRSApn(UInt8 acctID, UInt8 *apn);
const UInt8*	DATA_GetGPRSApn(UInt8 acctID);
Result_t		DATA_SetAuthenMethod(UInt8 acctID, DataAuthenMethod_t authenMethod);
DataAuthenMethod_t		DATA_GetAuthenMethod(UInt8 acctID);
Result_t		DATA_SetGPRSHeaderCompression(UInt8 acctID, Boolean headerCompEnable);
Boolean			DATA_GetGPRSHeaderCompression(UInt8 acctID);
Result_t		DATA_SetGPRSQos(UInt8 acctID, PCHQosProfile_t qos);
const PCHQosProfile_t*	DATA_GetGPRSQos(UInt8 acctID);
Result_t		DATA_SetAcctLock(UInt8 acctID, Boolean acctLock);
Boolean			DATA_GetAcctLock(UInt8 acctID);
Result_t		DATA_SetGprsOnly(UInt8 acctID, Boolean gprsOnly);
Boolean			DATA_GetGprsOnly(UInt8 acctID);
Result_t		DATA_SetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);
Result_t		DATA_GetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);
Boolean			DATA_CheckTft(PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);
UInt8*			DATA_GetChapChallenge(UInt8 acctID);
UInt8 			DATA_GetChapChallengeLen(UInt8 acctID);
UInt8*			DATA_GetChapResponse(UInt8 acctID);
UInt8 			DATA_GetChapResponseLen(UInt8 acctID);
UInt8*			DATA_GetChapUserId(UInt8 acctID);
UInt8 			DATA_GetChapUserIdLen(UInt8 acctID);
UInt8 			DATA_GetChapID(UInt8 acctID);
Result_t		DATA_SetCSDDialNumber(UInt8 acctID, UInt8 *dialNumber);
const UInt8*	DATA_GetCSDDialNumber(UInt8 acctID);
Result_t		DATA_SetCSDDialType(UInt8 acctID, CSDDialType_t dialType);
CSDDialType_t	DATA_GetCSDDialType(UInt8 acctID);
Result_t		DATA_SetCSDBaudRate(UInt8 acctID, CSDBaudRate_t baudRate);
CSDBaudRate_t	DATA_GetCSDBaudRate(UInt8 acctID);
Result_t		DATA_SetCSDSyncType(UInt8 acctID, CSDSyncType_t synctype);
CSDSyncType_t	DATA_GetCSDSyncType(UInt8 acctID);
Result_t		DATA_SetCSDErrorCorrection(UInt8 acctID, Boolean enable);
Boolean			DATA_GetCSDErrorCorrection(UInt8 acctID);
Result_t		DATA_SetCSDErrCorrectionType(UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);
CSDErrCorrectionType_t	DATA_GetCSDErrCorrectionType(UInt8 acctID);
Result_t		DATA_SetCSDDataCompType(UInt8 acctID, CSDDataCompType_t dataCompType);
CSDDataCompType_t		DATA_GetCSDDataCompType(UInt8 acctID);
Result_t		DATA_SetCSDConnElement(UInt8 acctID, CSDConnElement_t connElement);
CSDConnElement_t		DATA_GetCSDConnElement(UInt8 acctID);
Result_t		DATA_CreateWLANDataAcct(UInt8 acctID, WLANData_t *pWlanSetting);
Result_t		DATA_SetWLANDataAcct(UInt8 acctID, WLANData_t *pWlanSetting);
Result_t		DATA_GetWLANSetting(UInt8 accId, WLANData_t *pWlanSetting);

// End of Old API definitions
/////////////////////////////////////////////////////////////////////////////////////////

#endif // _DATAACCT_OLD_API_H_
