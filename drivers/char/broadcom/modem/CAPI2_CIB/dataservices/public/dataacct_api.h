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
*   @file   dataacct_api.h
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

#ifndef _DATAACCT_API_H_
#define _DATAACCT_API_H_

/**
 * @addtogroup DATAAccountGroup
 * @{
 */

/**
	Initialize data accounts.
**/

void			DATA_InitAllDataAcct(void);

/**
	Check if an account ID is valid.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID to check.  ID should have been allocated by a call to 
						 DATA_GetEmptyAcctSlot
	@return Boolean	 Returns true if account is valid, false otherwie.
**/

Boolean			DsApi_IsAcctIDValid(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get the ClientInfo associated with the account.
	@param acctID			(in)	Account ID
	@return *ClientInfo_t	 Returns NULL if account is not valid, pointer to the ClientInfo otherwise
**/

ClientInfo_t*	DsApi_GetClientInfoPtr(UInt8 acctID);

/**
	Set the SimId for the account.
	@param acctID	(in) Account ID.  
	@param simId	(in) SIM ID.  
	
	@return *ClientInfo_t	 Returns NULL if account is not valid, pointer to the ClientInfo otherwie.
**/
Result_t DsApi_SetSimId(UInt8 acctID, SimNumber_t simId);

/**
	Get the SimId for the account.
	@param acctID	(in) Account ID.  
	
	@return SimNumber_t	 Returns SIM_ALL, if account is not valid.
**/
SimNumber_t DsApi_GetSimId(UInt8 acctID);

/**
	Get the SimId for the account from the Context ID.
	@param cid	(in) context ID.  
	
	@return SimNumber_t	 Returns SIM_ALL, if account is not valid.
**/
SimNumber_t DsApi_GetSimIdFromCid(UInt8 cid);

/**
	Create a Packet Switch Data Account.  The account ID should have been allocated by
	a call to DsApi_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *pGprsSetting	(in)	Pointer to GPRS context
	@return Result_t
**/

Result_t		DsApi_CreateGPRSDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID, GPRSContext_t *pGprsSetting);


/**
	Create a Circuit Switch Data Account.  The account ID should have been allocated by
	a call to DsApi_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *pCsdSetting		(in)	Pointer to CSD context
	@return Result_t
**/

Result_t		DsApi_CreateCSDDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDContext_t *pCsdSetting);



/**
	Delete a data account.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Result_t
**/

Result_t		DsApi_DeleteDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Retrieve the GPRS context associated with the passed account ID.  If the account ID is
	invalid or not associated with a packet switch account RESULT_ERROR is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *pGprsContext	(out)	Pointer to GPRS context
	@return Result_t
**/

Result_t	DsApi_GetGPRSContext(ClientInfo_t* inClientInfoPtr, UInt8 acctID, GPRSContext_t *pGprsContext);

/**
	Retrieve the CSD context associated with the passed account ID.  If the account ID is
	invalid or not associated with a circuit switch account RESULT_ERROR is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *pCsdContext		(out)	Pointer to CSD context
	@return Result_t
**/

Result_t	DsApi_GetCSDContext(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDContext_t *pCsdContext);



/**
	Set the user name associated with the passed account ID.  If the account ID
	is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *username		(in)	Pointer to null terminated user name
	@return Result_t

**/

Result_t		DsApi_SetUsername(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *username);


/**
	Get the user name associated with the passed account ID.  If the account
	ID is invalid a NULL pointer is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Pointer to NULL terminated user name
**/

UInt8			*DsApi_GetUsername(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Set the password associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param *password		(in)	Pointer to null terminated password
	@return Result_t
**/

Result_t		DsApi_SetPassword(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *password);


/**
	Get the password associated with a passed account ID.  If the account
	ID	is invalid a NULL pointer is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Pointer to NULL terminated password

**/

UInt8			*DsApi_GetPassword(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Set the static IP address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param staticIPAddr		(in)	An array of 4 bytes holding the static IP address
	@return Result_t
**/

Result_t		DsApi_SetStaticIPAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *staticIPAddr);

/**
	Get the static IP address associated with the passed account ID. If the
	account ID is invalid a NULL pointer is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return an array of 4 bytes holding an IP address
**/

const UInt8*	DsApi_GetStaticIPAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Set the primary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param priDnsAddr		(in)	An array of 4 bytes containing primary DNS address
	@return Result_t
**/

Result_t		DsApi_SetPrimaryDnsAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *priDnsAddr);

/**
	Get the primary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return an array of 4 bytes containing primary DNS address
**/

const UInt8*	DsApi_GetPrimaryDnsAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the secondary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param sndDnsAddr		(in)	An array of 4 bytes containing secondary DNS address
	@return Result_t
**/

Result_t		DsApi_SetSecondDnsAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *sndDnsAddr);

/**
	Get the secondary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return an array of 4 bytes containing secondary DNS address
**/

const UInt8*	DsApi_GetSecondDnsAddr(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Enable/Disable compression for the passed account ID.  If the account ID is invalid
	an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param dataCompEnable	(in)	TRUE = ENABLE, FALSE = DISABLE compression
	@return Result_t
**/

Result_t		DsApi_SetDataCompression(ClientInfo_t* inClientInfoPtr, UInt8 acctID, Boolean dataCompEnable);

/**
	Get current compression setting for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return TRUE = ENABLED, FALSE = DISABLED
**/

Boolean			DsApi_GetDataCompression(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get account type associated with the passed account ID.  If the account
	ID is invalid INVALID_ACCT_ID is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return DataAccount_t		Data account type
**/

DataAccountType_t			DsApi_GetAcctType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Allocate an account ID.  This function should be called first to allocate and account
	ID before calling any other function.  If the function fails INVALID_ACCT_ID is
	returnd.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@return Account ID
**/

UInt8			DsApi_GetEmptyAcctSlot(ClientInfo_t* inClientInfoPtr);


/**
	Get the packet switch context ID associated with the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Context ID
**/

UInt8			DsApi_GetCidFromDataAcctID(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Get the account ID associated with the passed packet switch context
	ID.  If an account doesn't exist with the passed context ID INVALID_ACCT_ID
	is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param cid				(in)	Context ID
**/

UInt8			DsApi_GetDataAcctIDFromCid(ClientInfo_t* inClientInfoPtr, UInt8 cid);


/**
	Get the primary cid associated with the passed account id.
	ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	account id
**/

UInt8			DsApi_GetPrimaryCidFromDataAcctID(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Check if the account is associated with secondary PDP.
    
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	account id
	Return TRUE if it is secondary PDP. Otherwise return FALSE
	False is also returned if the account is invalid. Applications are
	responsible for calling DsApi_IsAcctIDValid to check the validity of
	the account.
**/

Boolean DsApi_IsSecondaryDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get the data sent size associated with the passed in account ID
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctId			(in)	Context ID
	@return Sent Data size
**/

UInt32			DsApi_GetDataSentSize(ClientInfo_t* inClientInfoPtr, UInt8 acctId);

/**
	Get the data receive size associated with the passed in account ID
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctId			(in)	Context ID
	@return Receive Data size
**/

UInt32			DsApi_GetDataRcvSize(ClientInfo_t* inClientInfoPtr, UInt8 acctId);


//==============================================================================
// GPRS only Data Account APIs
//==============================================================================

/**
	Set the PDP type associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param pdpType			(in)	Pointer to PDP Type
	@return Result_t
**/

Result_t		DsApi_SetGPRSPdpType(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *pdpType);

/**
	Get the PDP type associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Pointer to PDP Type
**/

const UInt8*	DsApi_GetGPRSPdpType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the APN associated with the passed account ID.  If the ID is invalid
	an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param apn				(in)	NULL terminated string containing APN name
	@return Result_t
**/

Result_t		DsApi_SetGPRSApn(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *apn);

/**
	Get the APN associated with the passed account ID.  If the account ID is
	invalid NULL is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return NULL terminated string containing APN name
**/

const UInt8*	DsApi_GetGPRSApn(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set	authentication method associated with passed account ID.  If the ID
	is invalid an error is returned
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param authenMethod		(in)	Authentication method
	@return Result_t
**/

Result_t		DsApi_SetAuthenMethod(ClientInfo_t* inClientInfoPtr, UInt8 acctID, DataAuthenMethod_t authenMethod);

/**
	Get authentication method associated with the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return DataAuthenMethod_t
**/

DataAuthenMethod_t			DsApi_GetAuthenMethod(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Enable/Disable IP header compression for the passed account ID.  If the ID is
	invalid return an error.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param headerCompEnable	(in)	TRUE = Enabled, FALSE = disabled
	@return Result_t
**/

Result_t		DsApi_SetGPRSHeaderCompression(ClientInfo_t* inClientInfoPtr, UInt8 acctID, Boolean headerCompEnable);

/**
	Get the current IP header compression setting for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return TRUE = Enabled, FALSE = disabled
**/

Boolean			DsApi_GetGPRSHeaderCompression(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the QoS parameters associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param qos				(in)	QoS Profile
	@return Result_t
**/

Result_t		DsApi_SetGPRSQos(ClientInfo_t* inClientInfoPtr, UInt8 acctID, PCHQosProfile_t qos);

/**
	Get the QoS parameters associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return Pointer to QoS profile
**/

const PCHQosProfile_t*	DsApi_GetGPRSQos(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set data account lock for the passed account ID.  If the ID is
	invalid return an error.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param acctLock			(in)	TRUE(1) = Locked, FALSE(0) = UnLocked
	@return Result_t
**/

Result_t		DsApi_SetAcctLock(ClientInfo_t* inClientInfoPtr, UInt8 acctID, Boolean acctLock);

/**
	Get data account lock for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return TRUE(1) = Locked, FALSE(0) = UnLocked
**/

Boolean			DsApi_GetAcctLock(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Set data account gprsOnly for the passed account ID.  If the ID is
	invalid return an error.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param gprsOnly			(in)	TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
	@return Result_t
**/
Result_t		DsApi_SetGprsOnly(ClientInfo_t* inClientInfoPtr, UInt8 acctID, Boolean gprsOnly);

/**
	Get gprsOnly for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/
Boolean			DsApi_GetGprsOnly(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set data account TFT for the passed account ID.  If the ID is
	invalid return an error.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param pTft				(in)	
	@return Result_t
**/

Result_t DsApi_SetGPRSTft(ClientInfo_t* inClientInfoPtr, UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);

/**
	Get TFT for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@param pTft     (in/out) pointer to TFT that holds the result
	@return TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/

Result_t DsApi_GetGPRSTft(ClientInfo_t* inClientInfoPtr, UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);



/**
	DsApi_CheckTFT for the passed TFT.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param pTft				(in)	pointer to TTFT
	@param isSecondary		(in) indicate the type of context that the TFT is associated
	@param priCid			(in) the primary cid 
	@return TRUE(1) = TFT is valid, FALSE(0) = TFT is invalid
**/
Boolean  DsApi_CheckTft(ClientInfo_t* inClientInfoPtr, PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);

/**
	Get challenge for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return challenge for CHAP
**/
UInt8 *DsApi_GetChapChallenge(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get challenge length  for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return challenge len for CHAP
**/
UInt8 DsApi_GetChapChallengeLen(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get chap response for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return response for CHAP
**/
UInt8 *DsApi_GetChapResponse(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get chap response length for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return response len for CHAP
**/
UInt8 DsApi_GetChapResponseLen(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get chap user id for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return user id for CHAP
**/
UInt8 *DsApi_GetChapUserId(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Get chap user id length for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return user id len for CHAP
**/
UInt8 DsApi_GetChapUserIdLen(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


/**
	Get chap id for the passed account ID.
	@param *inClientInfoPtr	(in)	ClientInfo_t pointer
	@param acctID			(in)	Account ID
	@return chap id
**/
UInt8 DsApi_GetChapID(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

//==============================================================================
//	CSD Only Data Account APIs
//==============================================================================
/**
	Set the the dial string associated with the passed account
	ID.  If the account ID is invalid an error is returned.
**/

Result_t		DsApi_SetCSDDialNumber(ClientInfo_t* inClientInfoPtr, UInt8 acctID, UInt8 *dialNumber);

/** 
	Get the dial string associated with the passed account
	ID.  If the account ID is invalid then NULL is returned.
**/

const UInt8*	DsApi_GetCSDDialNumber(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the account for a digital or analog modem.  If the account ID
	or dial type is invalid an error is returned.
**/

Result_t		DsApi_SetCSDDialType(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDDialType_t dialType);

/**
	Get the modem type associated with the passed account ID. 
**/

CSDDialType_t			DsApi_GetCSDDialType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the user data rate for the passed account.
**/

Result_t		DsApi_SetCSDBaudRate(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDBaudRate_t baudRate);

/**
	Get the user data rate associated with the passed account ID.
**/

CSDBaudRate_t			DsApi_GetCSDBaudRate(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the synchronization type for the passed account.
**/

Result_t		DsApi_SetCSDSyncType(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDSyncType_t synctype);

/**
	Get the synchronization type associated with the passed account ID.
**/

CSDSyncType_t	DsApi_GetCSDSyncType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Enable/disable error correction for the passed account ID.
**/

Result_t		DsApi_SetCSDErrorCorrection(ClientInfo_t* inClientInfoPtr, UInt8 acctID, Boolean enable);

/**
	Get the error correction setting for the passed account ID.
**/

Boolean			DsApi_GetCSDErrorCorrection(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the error correction type for the passed account ID.  If the account ID
	is invalid an error is returned.
**/

Result_t		DsApi_SetCSDErrCorrectionType(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);

/**
	Get the error correction type associated with the passed account.
**/

CSDErrCorrectionType_t			DsApi_GetCSDErrCorrectionType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set the compression type associated with the passed account ID.  If the account ID
	is invalid an error is returned.
**/

Result_t		DsApi_SetCSDDataCompType(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDDataCompType_t dataCompType);

/**
	Get the Compression type associated with the passed account ID.
**/

CSDDataCompType_t			DsApi_GetCSDDataCompType(ClientInfo_t* inClientInfoPtr, UInt8 acctID);

/**
	Set connection element for the associated account ID.  If the account ID is
	invalid an error is returned.
**/

Result_t		DsApi_SetCSDConnElement(ClientInfo_t* inClientInfoPtr, UInt8 acctID, CSDConnElement_t connElement);

/**
	Get the connection element associated with the passed account ID.
**/

CSDConnElement_t			DsApi_GetCSDConnElement(ClientInfo_t* inClientInfoPtr, UInt8 acctID);


//*****************************************************************************
/**
*   DsApi_CreateWLANDataAcct() creates a WLAN Data Account.
*   The account ID should have been allocated by a call to DsApi_GetEmptyAcctSlot().  
*   If there are inconsistencies in the account settings, an error will be returned.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param acctID			(in)	Account ID.
*   @param *pWlanSetting	(in)	Pointer to WLAN Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_CreateWLANDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID, WLANData_t *pWlanSetting);


//*****************************************************************************
/**
*   DsApi_SetWLANDataAcct() updates WLAN Data Account settings.
*   The account ID of the data account already created.
*   Will update the settings.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param acctID			(in)	Account ID.
*   @param *pWlanSetting	(in)	Pointer to WLAN Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_SetWLANDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 acctID, WLANData_t *pWlanSetting);


//*****************************************************************************
/**
*   DsApi_GetWLANSetting() gets WLAN Data Account settings.
*   The account ID of the data account already created.
*   Will get the settings.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param accId			(in)	Account ID.
*   @param *pWlanSetting	(in)	Pointer to WLAN Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_GetWLANSetting(ClientInfo_t* inClientInfoPtr, UInt8 accId, WLANData_t *pWlanSetting);

/**
	Update accout infomation to file system..
**/

void			DATA_UpdateAccountToFileSystem(void);

/**
	ClientInfo initialization function for Data Account Service APIs
**/

//*****************************************************************************
/**
*   DsApi_CreateUSBEEMDataAcct() creates a USB EEM Data Account.
*   The account ID should have been allocated by a call to DsApi_GetEmptyAcctSlot().  
*   If there are inconsistencies in the account settings, an error will be returned.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param inAcctID			(in)	Account ID.
*   @param *inUSBEEMDataPtr	(in)	Pointer to USB EEM Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_CreateUSBEEMDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 inAcctID, USBEEMData_t *inUSBEEMDataPtr);


//*****************************************************************************
/**
*   DsApi_SetUSBEEMDataAcct() updates USB EEM Data Account settings.
*   The account ID of the data account already created.
*   Will update the settings.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param inAcctID			(in)	Account ID.
*   @param *inUSBEEMDataPtr	(in)	Pointer to USB EEM Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_SetUSBEEMDataAcct(ClientInfo_t* inClientInfoPtr, UInt8 inAcctID, USBEEMData_t *inUSBEEMDataPtr);


//*****************************************************************************
/**
*   DsApi_GetUSBEEMDataAcct() gets USB EEM Data Account settings.
*   The account ID of the data account already created.
*   Will get the settings.
*
*   @param *inClientInfoPtr	(in)	ClientInfo_t pointer
*   @param inAcctID			(in)	Account ID.
*   @param *inUSBEEMDataPtr	(in)	Pointer to USB EEM Settings data
*
*   @return Result_t
*
*******************************************************************************/
Result_t		DsApi_GetUSBEEMSetting(ClientInfo_t* inClientInfoPtr, UInt8 inAcctID, USBEEMData_t *inUSBEEMDataPtr);

/**
	Update accout infomation to file system..
**/

void			DATA_UpdateAccountToFileSystem(void);

/**
	ClientInfo initialization function for Data Account Service APIs
**/

void 			DATA_InitClientInfo(ClientInfo_t* inClientInfoPtr, UInt8 inClientId, UInt8 acctID);

UInt8 DsApi_GetIntAcctId(ClientInfo_t* inClientInfoPtr, UInt8 inAcctID);

UInt8 DsApi_GetExtAcctId(ClientInfo_t* inClientInfoPtr, UInt8 inAcctID);


/** @} */


#include "dataacct_old_api.h"

#endif // _DATAACCT_API_H_

