//*********************************************************************
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
*   @file   capi2_old_ds.h
*
*   @brief  This file contains CAPI2 related definitions for DS.
*
****************************************************************************/

void			DATA_InitAllDataAcct(void);

/**
	Check if an account ID is valid.
	@param acctID	(in) Account ID to check.  ID should have been allocated by a call to 
						 DATA_GetEmptyAcctSlot
	@return Boolean	 Returns true if account is valid, false otherwie.
**/

Boolean			DATA_IsAcctIDValid(UInt8 acctID);

/**
	Create a Packet Switch Data Account.  The account ID should have been allocated by
	a call to DATA_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param acctID			(in)	Account ID
	@param *pGprsSetting	(in)	Pointer to GPRS context
	@return Result_t
**/

Result_t		DATA_CreateGPRSDataAcct(UInt8 acctID, GPRSContext_t *pGprsSetting);

/**
	Create a Circuit Switch Data Account.  The account ID should have been allocated by
	a call to DATA_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param acctID			(in)	Account ID
	@param *pCsdSetting		(in)	Pointer to CSD context
	@return Result_t
**/

Result_t		DATA_CreateCSDDataAcct(UInt8 acctID, CSDContext_t *pCsdSetting);

/**
	Delete a data account.
	@param acctID	(in)	Account ID
	@return Result_t
**/

Result_t		DATA_DeleteDataAcct(UInt8 acctID);

/**
	Retrieve the GPRS context associated with the passed account ID.  If the account ID is
	invalid or not associated with a packet switch account RESULT_ERROR is returned.
	@param acctID			(in)	Account ID
	@param *pGprsContext	(out)	Pointer to GPRS context
	@return Result_t
**/

Result_t	DATA_GetGPRSContext(UInt8 acctID, GPRSContext_t *pGprsContext);

/**
	Retrieve the CSD context associated with the passed account ID.  If the account ID is
	invalid or not associated with a circuit switch account RESULT_ERROR is returned.
	@param acctID			(in)	Account ID
	@param *pCsdContext		(out)	Pointer to CSD context
	@return Result_t
**/

Result_t	DATA_GetCSDContext(UInt8 acctID, CSDContext_t *pCsdContext);

/**
	Set the user name associated with the passed account ID.  If the account ID
	is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param *username	(in)	Pointer to null terminated user name
	@return Result_t

**/

Result_t		DATA_SetUsername(UInt8 acctID, UInt8 *username);

/**
	Get the user name associated with the passed account ID.  If the account
	ID is invalid a NULL pointer is returned.
	@param acctID		(in)	Account ID
	@return Pointer to NULL terminated user name
**/

UInt8			*DATA_GetUsername(UInt8 acctID);

/**
	Set the password associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param *password	(in)	Pointer to null terminated password
	@return Result_t
**/

Result_t		DATA_SetPassword(UInt8 acctID, UInt8 *password);

/**
	Get the password associated with a passed account ID.  If the account
	ID	is invalid a NULL pointer is returned.
	@param acctID		(in)	Account ID
	@return Pointer to NULL terminated password

**/

UInt8			*DATA_GetPassword(UInt8 acctID);

/**
	Set the static IP address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param staticIPAddr	(in)	An array of 4 bytes holding the static IP address
	@return Result_t
**/

Result_t		DATA_SetStaticIPAddr(UInt8 acctID, UInt8 *staticIPAddr);

/**
	Get the static IP address associated with the passed account ID. If the
	account ID is invalid a NULL pointer is returned.
	@param acctID		(in)	Account ID
	@return an array of 4 bytes holding an IP address
**/

const UInt8*	DATA_GetStaticIPAddr(UInt8 acctID);

/**
	Set the primary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param priDnsAddr	(in)	An array of 4 bytes containing primary DNS address
	@return Result_t
**/

Result_t		DATA_SetPrimaryDnsAddr(UInt8 acctID, UInt8 *priDnsAddr);

/**
	Get the primary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@return an array of 4 bytes containing primary DNS address
**/

const UInt8*	DATA_GetPrimaryDnsAddr(UInt8 acctID);

/**
	Set the secondary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param sndDnsAddr	(in)	An array of 4 bytes containing secondary DNS address
	@return Result_t
**/

Result_t		DATA_SetSecondDnsAddr(UInt8 acctID, UInt8 *sndDnsAddr);

/**
	Get the secondary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@return an array of 4 bytes containing secondary DNS address
**/

const UInt8*	DATA_GetSecondDnsAddr(UInt8 acctID);

/**
	Enable/Disable compression for the passed account ID.  If the account ID is invalid
	an error is returned.
	@param acctID			(in)	Account ID
	@param dataCompEnable	(in)	TRUE = ENABLE, FALSE = DISABLE compression
	@return Result_t
**/

Result_t		DATA_SetDataCompression(UInt8 acctID, Boolean dataCompEnable);

/**
	Get current compression setting for the passed account ID.
	@param acctID		(in)	Account ID
	@return TRUE = ENABLED, FALSE = DISABLED
**/

Boolean			DATA_GetDataCompression(UInt8 acctID);

/**
	Get account type associated with the passed account ID.  If the account
	ID is invalid INVALID_ACCT_ID is returned.
	@param acctID		(in)	Account ID
	@return DataAccount_t		Data account type
**/

DataAccountType_t			DATA_GetAcctType(UInt8 acctID);

/**
	Allocate an account ID.  This function should be called first to allocate and account
	ID before calling any other function.  If the function fails INVALID_ACCT_ID is
	returnd.
	@return Account ID
**/

UInt8			DATA_GetEmptyAcctSlot(void);

/**
	Get the packet switch context ID associated with the passed account ID.
	@param acctID		(in)	Account ID
	@return Context ID
**/

UInt8			DATA_GetCidFromDataAcctID(UInt8 acctID);

/**
	Get the account ID associated with the passed packet switch context
	ID.  If an account doesn't exist with the passed context ID INVALID_ACCT_ID
	is returned.
	@param cid		(in)	Context ID
**/

UInt8			DATA_GetDataAcctIDFromCid(UInt8 cid);

/**
	Get the primary cid associated with the passed account id.
	ID.
	@param acctID		(in)	account id
**/

UInt8			DATA_GetPrimaryCidFromDataAcctID(UInt8 acctID);

/**
	Check if the account is associated with secondary PDP.
    
	@param acctID		(in)	account id
	Return TRUE if it is secondary PDP. Otherwise return FALSE
	False is also returned if the account is invalid. Applications are
	responsible for calling DATA_IsAcctIDValid to check the validity of
	the account.
**/

Boolean DATA_IsSecondaryDataAcct(UInt8 acctID);

/**
	Get the data sent size associated with the passed in account ID
	@param acctId		(in)	Context ID
	@return Sent Data size
**/

UInt32			DATA_GetDataSentSize(UInt8 acctId);

/**
	Get the data receive size associated with the passed in account ID
	@param acctId		(in)	Context ID
	@return Receive Data size
**/

UInt32			DATA_GetDataRcvSize(UInt8 acctId);
//==============================================================================
// GPRS only Data Account APIs
//==============================================================================

/**
	Set the PDP type associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param pdpType		(in)	Pointer to PDP Type
	@return Result_t
**/

Result_t		DATA_SetGPRSPdpType(UInt8 acctID, UInt8 *pdpType);

/**
	Get the PDP type associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param acctID		(in)	Account ID
	@return Pointer to PDP Type
**/

const UInt8*	DATA_GetGPRSPdpType(UInt8 acctID);

/**
	Set the APN associated with the passed account ID.  If the ID is invalid
	an error is returned.
	@param acctID		(in)	Account ID
	@param apn			(in)	NULL terminated string containing APN name
	@return Result_t
**/

Result_t		DATA_SetGPRSApn(UInt8 acctID, UInt8 *apn);

/**
	Get the APN associated with the passed account ID.  If the account ID is
	invalid NULL is returned.
	@param acctID		(in)	Account ID
	@return NULL terminated string containing APN name
**/

const UInt8*	DATA_GetGPRSApn(UInt8 acctID);

/**
	Set	authentication method associated with passed account ID.  If the ID
	is invalid an error is returned
	@param acctID		(in)	Account ID
	@param authenMethod	(in)	Authentication method
	@return Result_t
**/

Result_t		DATA_SetAuthenMethod(UInt8 acctID, DataAuthenMethod_t authenMethod);

/**
	Get authentication method associated with the passed account ID.
	@param acctID		(in)	Account ID
	@return DataAuthenMethod_t
**/

DataAuthenMethod_t			DATA_GetAuthenMethod(UInt8 acctID);

/**
	Enable/Disable IP header compression for the passed account ID.  If the ID is
	invalid return an error.
	@param acctID		(in)	Account ID
	@param headerCompEnable	(in)	TRUE = Enabled, FALSE = disabled
	@return Result_t
**/

Result_t		DATA_SetGPRSHeaderCompression(UInt8 acctID, Boolean headerCompEnable);

/**
	Get the current IP header compression setting for the passed account ID.
	@param acctID		(in)	Account ID
	@return TRUE = Enabled, FALSE = disabled
**/

Boolean			DATA_GetGPRSHeaderCompression(UInt8 acctID);

/**
	Set the QoS parameters associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param qos			(in)	QoS Profile
	@return Result_t
**/

Result_t		DATA_SetGPRSQos(UInt8 acctID, PCHQosProfile_t qos);

/**
	Get the QoS parameters associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param acctID		(in)	Account ID
	@return Pointer to QoS profile
**/

const PCHQosProfile_t*	DATA_GetGPRSQos(UInt8 acctID);

/**
	Set data account lock for the passed account ID.  If the ID is
	invalid return an error.
	@param acctID	(in)	Account ID
	@param acctLock	(in)	TRUE(1) = Locked, FALSE(0) = UnLocked
	@return Result_t
**/

Result_t		DATA_SetAcctLock(UInt8 acctID, Boolean acctLock);

/**
	Get data account lock for the passed account ID.
	@param acctID	(in)	Account ID
	@return TRUE(1) = Locked, FALSE(0) = UnLocked
**/

Boolean			DATA_GetAcctLock(UInt8 acctID);


/**
	Set data account gprsOnly for the passed account ID.  If the ID is
	invalid return an error.
	@param acctID	(in)	Account ID
	@param gprsOnly	(in)	TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
	@return Result_t
**/
Result_t		DATA_SetGprsOnly(UInt8 acctID, Boolean gprsOnly);

/**
	Get gprsOnly for the passed account ID.
	@param acctID	(in)	Account ID
	@return TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/
Boolean			DATA_GetGprsOnly(UInt8 acctID);

/**
	Set data account TFT for the passed account ID.  If the ID is
	invalid return an error.
	@param acctID	(in)	Account ID
	@param pTft	(in)	
	@return Result_t
**/

Result_t DATA_SetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);

/**
	Get TFT for the passed account ID.
	@param acctID	(in)	Account ID
	@param pTft     (in/out) pointer to TFT that holds the result
	@return TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/

Result_t DATA_GetGPRSTft(UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);



/**
	DATA_CheckTFT for the passed TFT.
	@param pTft	(in)	pointer to TTFT
	@param isSecondary     (in) indicate the type of context that the TFT is associated
	@param priCid		(in) the primary cid 
	@return TRUE(1) = TFT is valid, FALSE(0) = TFT is invalid
**/
 Boolean  DATA_CheckTft(PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);

//==============================================================================
//	CSD Only Data Account APIs
//==============================================================================
/**
	Set the the dial string associated with the passed account
	ID.  If the account ID is invalid an error is returned.
**/

Result_t		DATA_SetCSDDialNumber(UInt8 acctID, UInt8 *dialNumber);

/** 
	Get the dial string associated with the passed account
	ID.  If the account ID is invalid then NULL is returned.
**/

const UInt8*	DATA_GetCSDDialNumber(UInt8 acctID);

/**
	Set the account for a digital or analog modem.  If the account ID
	or dial type is invalid an error is returned.
**/

Result_t		DATA_SetCSDDialType(UInt8 acctID, CSDDialType_t dialType);

/**
	Get the modem type associated with the passed account ID. 
**/

CSDDialType_t			DATA_GetCSDDialType(UInt8 acctID);

/**
	Set the user data rate for the passed account.
**/

Result_t		DATA_SetCSDBaudRate(UInt8 acctID, CSDBaudRate_t baudRate);

/**
	Get the user data rate associated with the passed account ID.
**/

CSDBaudRate_t			DATA_GetCSDBaudRate(UInt8 acctID);

/**
	Set the synchronization type for the passed account.
**/

Result_t		DATA_SetCSDSyncType(UInt8 acctID, CSDSyncType_t synctype);

/**
	Get the synchronization type associated with the passed account ID.
**/

CSDSyncType_t	DATA_GetCSDSyncType(UInt8 acctID);

/**
	Enable/disable error correction for the passed account ID.
**/

Result_t		DATA_SetCSDErrorCorrection(UInt8 acctID, Boolean enable);

/**
	Get the error correction setting for the passed account ID.
**/

Boolean			DATA_GetCSDErrorCorrection(UInt8 acctID);

/**
	Set the error correction type for the passed account ID.  If the account ID
	is invalid an error is returned.
**/

Result_t		DATA_SetCSDErrCorrectionType(UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);

/**
	Get the error correction type associated with the passed account.
**/

CSDErrCorrectionType_t			DATA_GetCSDErrCorrectionType(UInt8 acctID);

/**
	Set the compression type associated with the passed account ID.  If the account ID
	is invalid an error is returned.
**/

Result_t		DATA_SetCSDDataCompType(UInt8 acctID, CSDDataCompType_t dataCompType);

/**
	Get the Compression type associated with the passed account ID.
**/

CSDDataCompType_t			DATA_GetCSDDataCompType(UInt8 acctID);

/**
	Set connection element for the associated account ID.  If the account ID is
	invalid an error is returned.
**/

Result_t		DATA_SetCSDConnElement(UInt8 acctID, CSDConnElement_t connElement);

/**
	Get the connection element associated with the passed account ID.
**/

CSDConnElement_t			DATA_GetCSDConnElement(UInt8 acctID);

//==============================================================================
//	File System - Data Account APIs
//==============================================================================
void			DATA_UpdateAccountToFileSystem(void);

//==============================================================================
//	Internal functions
//==============================================================================
Result_t		resetDataSize(UInt8 cid);
Result_t		addDataSentSizebyCid(UInt8 cid, UInt32 size);
Result_t		addDataRcvSizebyCid(UInt8 cid, UInt32 size);

Result_t DC_SetupDataConnection(	UInt8							inClientID,
									UInt8							acctId, 
									DC_ConnectionType_t				linkType );

/**
	This function handles data shutdown request

	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param acctId		(in)	Data account id
	@return Result_t
**/
Result_t DC_ShutdownDataConnection(	UInt8							inClientID,								   
									UInt8							acctId );
