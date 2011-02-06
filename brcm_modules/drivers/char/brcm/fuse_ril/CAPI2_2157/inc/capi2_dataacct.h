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

/// @cond CAPI2_DATAConnectionGroup

/**
*
*   @file   capi2_dataacct.h
*
*   @brief  This file contains the interface to the Data Account 
			Services.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_DATAAccountGroup   Data Account Management
*   @ingroup    CAPI2_DATAAccountGroup
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

#ifndef _CAPI2_DATAACCT_H_
#define _CAPI2_DATAACCT_H_

/**
 * @addtogroup CAPI2_DATAAccountGroup
 * @{
 */

#include "capi2_types.h"
#include "capi2_pchtypes.h"
#include "capi2_resultcode.h"

//There is same define MAX_DATA_ACCOUNT_ID in dataacct.h, Please keep accordance.
//[GCF] - TC 27.22.4.27.2 Seq.2.1 ,to keep defined data accounts to a max of 9
#define		MAX_DATA_ACCOUNT_ID		10     //increased from 10 to 11 since one slot is reserved for STK
#define		DATA_ACCOUNT_ID_FOR_ATC	0x80
#define		INVALID_ACCT_ID			0xFF
/*The last Data Account slot corresponding to MAX_DATA_ACCOUNT_ID is reserved for STK-BIP and ATC/MMI clients should not use it*/
#define     DATA_ACCT_FOR_STK       MAX_DATA_ACCOUNT_ID


#define		MAX_UNERNAME_LEN		32
#define		MAX_PASSWORD_LEN		32
#define		MAX_PDPTYPE_LEN			8
#define		MAX_APN_LEN				32
#define		MAX_STATIC_IP_LEN		4
#define		MAX_DNS_ADDR_LEN		4
#define		MAX_DIAL_NUMBER_LEN		32

#define		DATA_DEFAULT_DATA_COMPRESSION	FALSE
#define		DATA_DEFAULT_HEADER_COMPRESSION	FALSE
#define		DATA_DEFAULT_EMPTY_IP_ADDR		0x00000000

//CHAP
#define		MAX_CHAP_CHALLENGE_LEN	128
#define		MAX_CHAP_RESPONSE_LEN	128
#define		MAX_CHAP_USERID_LEN		128

/**
Type of data account, it can either be a Circuit Switch Data account 
or packet switch data account.
**/
typedef enum
{
	DATA_ACCOUNT_NOT_USED,
	DATA_ACCOUNT_GPRS,
	DATA_ACCOUNT_CSD
}DataAccountType_t;

/**
Data authentication method. This data type sepecifies the authentication method
when establishing a connection. It can either be PAP or CHAP
**/
typedef enum
{
	DATA_PPP_PAP,	///< PAP authentication mode
	DATA_PPP_CHAP,	///< CHAP authentication mode 
	DATA_PPP_PASS_THROUGH,	///< PPP pass through mode
	DATA_PPP_CHAP_CR  //CHAP with Challenge and Response
}DataAuthenMethod_t;
#define	DATA_DEFAULT_AUTHEN_TYPE	DATA_PPP_CHAP

/**
This is used to specify if the Circuit switch connection is for a analog or
digital modem.
**/
typedef enum
{
	DATA_CSD_ANALOG,
	DATA_CSD_ISDN
}CSDDialType_t;
#define	DATA_DEFAULT_DIAL_TYPE		DATA_CSD_ANALOG

/**
Baud rate of circuit switched connection in bits per second.
**/
typedef enum
{
	DATA_CSD_BAUD_AUTOBAUDING	= 0,  // Autobauding
	DATA_CSD_BAUD_9600			= 7,  // 9600bps  V.32
	DATA_CSD_BAUD_14400			= 14, // 14400bps V.34
	DATA_CSD_BAUD_19200			= 15, // 19200bps V.34
	DATA_CSD_BAUD_28800			= 16, // 28800bps V.34
	DATA_CSD_BAUD_38400			= 49, // 38400bps V.120
	DATA_CSD_BAUD_48000			= 50, // 48000bps V.120
	DATA_CSD_BAUD_56000			= 51, // 56000bps V.120
	DATA_CSD_BAUD_9600_V_110	= 71, // 9600bps V.110 or X.31
	DATA_CSD_BAUD_48000_V_110	= 82, // 48000bps V.110 or X.31
	DATA_CSD_BAUD_56000_V_110	= 83, // 56000bps V.110 or X.31 to be used with async NT UDI or RDI
	DATA_CSD_BAUD_64000_V_110	= 83, // 64000bps X.31 to be used with async NT UDI.
	DATA_CSD_BAUD_56000_Tran	= 115,// 56000bps Transparent
	DATA_CSD_BAUD_64000_Tran	= 116,// 64000bps Transparent
	DATA_CSD_BAUD_32000_PIAFS	= 120,// 32000bps PIAFS32K
	DATA_CSD_BAUD_64000_PIAFS	= 121,// 32000bps PIAFS64K
	DATA_CSD_BAUD_28800_MM	    = 130,// 28800bps MultiMedia
	DATA_CSD_BAUD_32000_MM	    = 131,// 32000bps MultiMedia
	DATA_CSD_BAUD_33600_MM	    = 132,// 33600bps MultiMedia
	DATA_CSD_BAUD_56000_MM	    = 133,// 56000bps MultiMedia
	DATA_CSD_BAUD_64000_MM	    = 134 // 64000bps MultiMedia

}CSDBaudRate_t;
#define	DATA_DEFAULT_BAUDRATE		DATA_CSD_BAUD_9600

/**
This type specifies the synchronization that is to be used
for the connection.
**/

typedef enum
{
  DATA_CSD_ASYNC = 0,			// Data ckt Async (UDI or 3.1Khz)
  DATA_CSD_SYNC,				// Data ckt Sync  (UDI or 3.1Khz)
  DATA_CSD_PAD_ACCESS_ASYNC,	// PAD Access Async(UDI)
  DATA_CSD_PAD_ACCESS_SYNC,		// PAD Access Sync (UDI)
  DATA_CSD_RDI_ASYNC,			// RDI Data ckt Async		
  DATA_CSD_RDI_SYNC,			// RDI Data ckt Sync
  DATA_CSD_PAD_RDI_ASYNC,		// RDI PAD Access Async
  DATA_CSD_PAD_RDI_SYNC			// RDI PAD Access Sync
}CSDSyncType_t; 
#define DATA_DEFAULT_SYNCTYPE		DATA_CSD_ASYNC	

/**
Error correction types for circuit switch data. The two modes are
V.42 and MNP. This is for end to end transparent connection element
**/
typedef enum
{
	DATA_CSD_EC_V42,
	DATA_CSD_EC_MNP
}CSDErrCorrectionType_t;
#define	DATA_DEFAULT_EC_TYPE		DATA_CSD_EC_V42

/**
Data compression type. This can be V42bis or MNP for circuit switched connections.
For GPRS, only V.42bis is supported. Selecting a particular compression type also selects the
appropriate error correction method.  Error correction is always required if compression is enabled.
**/
typedef enum
{
	DATA_CSD_DC_V42BIS,
	DATA_CSD_DC_MNP
}CSDDataCompType_t;
#define	DATA_DEFAULT_DC_TYPE		DATA_CSD_DC_V42BIS

/**
Connection element data type. For circuit switched calls, it can either be 
transparent or non-trnasparent. For NT mode an RLP link is used and for tranparent
mode, error correction has to be negotiated by the peers.
**/
typedef enum
{
	DATA_CSD_CE_TRANSPARENT,			// 0 -transparent
	DATA_CSD_CE_NON_TRANSPARENT,		// 1 - non - transparent
	DATA_CSD_CE_BOTH_TRAN_PREFERRED,	// 2 - both - transparent preferred
	DATA_CSD_CE_BOTH_NON_TRAN_PREFERRED	// 3 - both - non transparent preferred
}CSDConnElement_t;
#define	DATA_DEFAULT_CONN_ELEMENT	DATA_CSD_CE_NON_TRANSPARENT

/**
Packet switched context parameters.
**/
typedef struct 
{	
	// Common fields for primary and secondary accounts
	UInt8			acctID;				///< Account ID
	Boolean			dataCompression;	///< Data Compression enabled/disabled	
	Boolean			headerCompression;	///< Header Compression enabled/disabled
	PCHQosProfile_t	qos;				///< Quality of Service profile

	//Fields for primary account only
	UInt8			*username;			///< Username for the account
	UInt8			*password;			///< Password associated with the account
	//CHAP
	UInt8			chapID;
	UInt8			chapChallengeLen;
	UInt8			chapResponseLen;
	UInt8			chapUserIdLen;
	UInt8			*chapChallenge;
	UInt8			*chapResponse;
	UInt8			*chapUserId;
	
	UInt8			*pdpType;			///< PDP type
	UInt8			*apn;				///< Access Point Name
	UInt8			*staticIPAddr;		///< Static IP address
	UInt8			*priDnsAddr;		///< primary DNS
	UInt8			*sndDnsAddr;		///< Secondary DNS
	UInt8			authenType;			///< Authentication type( PAP/CHAP)
	Boolean			gprsOnly;			///< gprsOnly flag, set to TRUE to disable voice call during DUN
	
	// Fields for secondary account only
	Boolean         isSecondaryPDP;		/// For Secondary PDP context
	UInt8			primaryAcctId;		/// account ID associated with the primary context, applicable for secondary PDPs only
	PCHTrafficFlowTemplate_t tft;		/// Traffic Flow Template
}GPRSContext_t;

/**
Circuit Switched Data account parameters
**/
typedef struct 
{	
	UInt8			acctID;					///< Account ID		
	UInt8			*username;				///< Username for the account
	UInt8			*password;				///< Password associated with the account
	UInt8			*staticIPAddr;			///< Static IP address
	UInt8			*priDnsAddr;			///< primary DNS
	UInt8			*sndDnsAddr;			///< Secondary DNS
	UInt8			*dialNumber;			///< Pointer to the dial number
	CSDDialType_t	dialType;				///< Dial type( Analog/ISDN)
	CSDBaudRate_t	baudRate;				///< Connection Baud rate
	CSDErrCorrectionType_t	errCorrectionType;	///< Error Correction type(V.42/MNP)
	Boolean			errorCorrection;		///< Error correction enabled/disabled
	CSDDataCompType_t	dataCompressionType;	///< Data compression type( V42bis/MNP)	
	Boolean			dataCompression;		///< Data Compression enabled/disabled
	CSDConnElement_t	connElement;		///< Connection element(Transparent/Non Transparent)
	UInt8			authenType;				///< PPP authentication type
	CSDSyncType_t	synctype;				///< Synchronization type ( async vs sync )
}CSDContext_t;


/**
GPRS Data type.
**/

typedef struct
{
	UInt8				cid;		///< Context ID
}DataGPRS_t;

/**
CSD Data type
**/
typedef struct
{
	UInt8				dialNumber[MAX_DIAL_NUMBER_LEN];///< Dial Number
	UInt8				dialType;						///< Analog vs ISDN call 
	UInt8				baudRate;						///< Baud Rate
	UInt8				errCorrectionType;				///< Type of Error Correction to use
	Boolean				errorCorrection;				///< Error correction enabled/disabled
	UInt8				dataCompressionType;			///< Type of Data Compression to use
	Boolean				dataCompression;				///< TRUE means enable data compression
	UInt8				connElement;					///< Transparent or Non-Transparent bearer
	CSDSyncType_t		synctype;						///< Synchronization type ( async vs sync )
}DataCSD_t;

/**
Response data for CAPI2_Data_GetGPRSTft().
**/
typedef struct
{
	PCHTrafficFlowTemplate_t* pTft;
	Result_t result;

}CAPI2_DATA_GetGPRSTft_Result_t;

//=============================================================================
// MS Data Account API Functions
//=============================================================================

//***************************************************************************************
/**
	Check if an account ID is valid.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID	(in) Account ID to check.  ID should have been allocated by a call to 
						 DATA_GetEmptyAcctSlot
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_IS_ACCT_ID_VALID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	true if account is valid, false otherwise.
**/
//***************************************************************************************
void CAPI2_DATA_IsAcctIDValid(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Create a Packet Switch Data Account.  The account ID should have been allocated by
	a call to DATA_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@param		*pGprsSetting	(in)	Pointer to GPRS context
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_CREATE_GPRS_ACCT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_CreateGPRSDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID, GPRSContext_t *pGprsSetting);


//***************************************************************************************
/**
	Create a Circuit Switch Data Account.  The account ID should have been allocated by
	a call to DATA_GetEmptyAcctSlot().  If there are inconsistencies in the account settings
	an error will be returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@param		*pCsdSetting	(in)	Pointer to CSD context
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_CREATE_GSM_ACCT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_CreateCSDDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdSetting);

//***************************************************************************************
/**
	Delete a data account.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_DELETE_ACCT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_DeleteDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Retrieve the GPRS context associated with the passed account ID.  If the account ID is
	invalid or not associated with a packet switch account ::RESULT_ERROR is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_GPRS_CONTEXT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Pointer to GPRS context
**/
//void CAPI2_DATA_GetGPRSContext(UInt32 tid, UInt8 clientID, UInt8 acctID);

//******************************************************************************
/**
	Retrieve the CSD context associated with the passed account ID.  If the account ID is
	invalid or not associated with a circuit switch account ::RESULT_ERROR is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_CONTEXT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Pointer to CSD context
**/
//******************************************************************************
//void CAPI2_DATA_GetCSDContext(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdContext);

//***************************************************************************************
/**
	Set the user name associated with the passed account ID.  If the account ID
	is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@param		*username	(in)	Pointer to null terminated user name
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_USERNAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetUsername(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *username);


//***************************************************************************************
/**
	Get the user name associated with the passed account ID.  If the account
	ID is invalid a NULL pointer is returned in ResultData
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_USERNAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	UInt8*, user name string OR NULL
**/	
void CAPI2_DATA_GetUsername(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set the password associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@param		*password	(in)	Pointer to null terminated password
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_PASSWORD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetPassword(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *password);


//***************************************************************************************
/**
	Get the password associated with a passed account ID.  If the account
	ID	is invalid a NULL pointer is returned in ResultData
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_PASSWORD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Pointer to NULL terminated password 
**/	
void CAPI2_DATA_GetPassword(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set the static IP address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@param	staticIPAddr	(in)	An array of 4 bytes holding the static IP address
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_STATIC_IPADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetStaticIPAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *staticIPAddr);


//***************************************************************************************
/**
	Get the static IP address associated with the passed account ID. If the
	account ID is invalid a NULL pointer is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_STATIC_IPADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	an array of 4 bytes holding an IP address
**/	
void CAPI2_DATA_GetStaticIPAddr(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set the primary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@param		priDnsAddr	(in)	An array of 4 bytes containing primary DNS address
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetPrimaryDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *priDnsAddr);


//***************************************************************************************
/**
	Get the primary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	an array of 4 bytes containing primary DNS address
**/	
void CAPI2_DATA_GetPrimaryDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID);

/**
	Set the secondary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param acctID		(in)	Account ID
	@param sndDnsAddr	(in)	An array of 4 bytes containing secondary DNS address
	@return Result_t
**/

//***************************************************************************************
/**
	Set the secondary DNS address associated with the passed account ID.  If the
	account ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param	sndDnsAddr	(in)	An array of 4 bytes containing secondary DNS address
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_SECONDARY_DNS_ADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetSecondDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *sndDnsAddr);

//***************************************************************************************
/**
	Get the secondary DSN address associated with the passed account ID.  If the
	accout ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID		(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_SECONDARY_DNS_ADDR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	an array of 4 bytes containing secondary DNS address
**/	
void CAPI2_DATA_GetSecondDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Enable/Disable compression for the passed account ID.  If the account ID is invalid
	an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@param		dataCompEnable	(in)	TRUE = ENABLE, FALSE = DISABLE compression
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_DATA_COMPRESSION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetDataCompression(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean dataCompEnable);


//***************************************************************************************
/**
	Get current compression setting for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_DATA_COMPRESSION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean, TRUE = ENABLED, FALSE = DISABLED
**/	
void CAPI2_DATA_GetDataCompression(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Get account type associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_ACCT_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	DataAccount_t,	Data account type, If the account ID is invalid INVALID_ACCT_ID is returned.
**/	
void CAPI2_DATA_GetAcctType(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Allocate an account ID.  This function should be called first to allocate and account
	ID before calling any other function.  If the function fails INVALID_ACCT_ID is
	returnd.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Account ID
**/	
void CAPI2_DATA_GetEmptyAcctSlot(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CID_FROM_ACCTID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Context ID
**/	
void CAPI2_DATA_GetCidFromDataAcctID(UInt32 tid, UInt8 clientID, UInt8 acctID);


//******************************************************************************
/**
	Get the account ID associated with the passed packet switch context
	ID.  If an account doesn't exist with the passed context ID INVALID_ACCT_ID
	is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param cid		(in)	Context ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_ACCTID_FROM_CID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The data account ID, or INVALID_ACCT_ID if the account doesn't exist.
**/
//******************************************************************************
void CAPI2_DATA_GetDataAcctIDFromCid(UInt32 tid, UInt8 clientID, UInt8 cid);

//******************************************************************************
/**
	Get the primary cid associated with the passed account id.
	ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID		(in)	account id
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_PRI_FROM_ACCTID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The data account ID, or INVALID_ACCT_ID if the account doesn't exist.
**/
//******************************************************************************
void CAPI2_DATA_GetPrimaryCidFromDataAcctID(UInt32 tid, UInt8 clientID, UInt8 acctID);

//******************************************************************************
/**
	Check if the account is associated with secondary PDP.
    
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID		(in)	account id
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_IS_SND_DATA_ACCT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Return TRUE if it is secondary PDP. Otherwise return FALSE
	False is also returned if the account is invalid. Applications are
	responsible for calling DATA_IsAcctIDValid to check the validity of
	the account.
**/
void CAPI2_DATA_IsSecondaryDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Get the data sent size associated with the passed in account ID
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_DATA_SENT_SIZE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	UInt32, Sent Data size
**/	
void CAPI2_DATA_GetDataSentSize(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Get the data receive size associated with the passed in account ID
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_DATA_RECV_SIZE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	UInt32, Receive Data size
**/	
void CAPI2_DATA_GetDataRcvSize(UInt32 tid, UInt8 clientID, UInt8 acctID);


//==============================================================================
// GPRS only Data Account APIs
//==============================================================================

//***************************************************************************************
/**
	Set the PDP type associated with the passed account ID.  If the account
	ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID		(in)	Account ID
	@param pdpType		(in)	Pointer to PDP Type
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_DATA_RECV_SIZE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_DATA_SetGPRSPdpType(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *pdpType);

//***************************************************************************************
/**
	Get the PDP type associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID		(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_DATA_RECV_SIZE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Pointer to PDP Type
**/
void CAPI2_DATA_GetGPRSPdpType(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set the APN associated with the passed account ID.  If the ID is invalid
	an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		apn			(in)	NULL terminated string containing APN name
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_GPRS_APN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetGPRSApn(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *apn);


//***************************************************************************************
/**
	Get the APN associated with the passed account ID.  If the account ID is
	invalid NULL is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID			(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_GPRS_APN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	NULL terminated string containing APN name
**/	
void CAPI2_DATA_GetGPRSApn(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set	authentication method associated with passed account ID.  If the ID
	is invalid an error is returned
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		authenMethod	(in)	Authentication method
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_AUTHEN_METHOD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetAuthenMethod(UInt32 tid, UInt8 clientID, UInt8 acctID, DataAuthenMethod_t authenMethod);


//***************************************************************************************
/**
	Get authentication method associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_AUTHEN_METHOD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	DataAuthenMethod_t
**/	
void CAPI2_DATA_GetAuthenMethod(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Enable/Disable IP header compression for the passed account ID.  If the ID is
	invalid return an error.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param headerCompEnable	(in)	TRUE = Enabled, FALSE = disabled
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_HDR_COMPRESSION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetGPRSHeaderCompression(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean headerCompEnable);


//***************************************************************************************
/**
	Get the current IP header compression setting for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_HDR_COMPRESSION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean, TRUE = Enabled, FALSE = disabled
**/	
void CAPI2_DATA_GetGPRSHeaderCompression(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set the QoS parameters associated with the passed account ID.  If the account
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		qos		(in)	QoS Profile
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_GPRS_QOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetGPRSQos(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHQosProfile_t qos);


//***************************************************************************************
/**
	Get the QoS parameters associated with the passed account ID.  If the account
	ID is invalid NULL is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_GPRS_QOS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Pointer to QoS profile
**/	
void CAPI2_DATA_GetGPRSQos(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set data account lock for the passed account ID.  If the ID is
	invalid return an error.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param	acctLock	(in)	TRUE(1) = Locked, FALSE(0) = UnLocked
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_ACCT_LOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetAcctLock(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean acctLock);


//***************************************************************************************
/**
	Get data account lock for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_ACCT_LOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean, TRUE(1) = Locked, FALSE(0) = UnLocked
**/	
void CAPI2_DATA_GetAcctLock(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set data account gprsOnly for the passed account ID.  If the ID is
	invalid return an error.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		gprsOnly	(in)	TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_GPRS_ONLY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetGprsOnly(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean gprsOnly);

//***************************************************************************************
/**
	Get gprsOnly for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_GPRS_ONLY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean, TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/	
void CAPI2_DATA_GetGprsOnly(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set data account TFT for the passed account ID.  If the ID is
	invalid return an error.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID	(in)	Account ID
	@param pTft	(in) TFT	
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_GPRS_TFT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_DATA_SetGPRSTft(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHTrafficFlowTemplate_t* pTft);

//***************************************************************************************
/**
	Get TFT for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param acctID	(in)	Account ID
	@param pTft     (in/out) 
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_GPRS_TFT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Structure that contains:
							pointer to TFT that holds the result, and
							TRUE(1) = GPRS-only, FALSE(0) = not GPRS-only
**/
void CAPI2_DATA_GetGPRSTft(UInt32 tid, UInt8 clientID, UInt8 acctID);

//==============================================================================
//	CSD Only Data Account APIs
//==============================================================================

//***************************************************************************************
/**
	Set the the dial string associated with the passed account
	ID.  If the account ID is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		*dialNumber	(in)	Dial string
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_DIAL_STR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetCSDDialNumber(UInt32 tid, UInt8 clientID, UInt8 acctID, UInt8 *dialNumber);


//***************************************************************************************
/**
	Get the dial string associated with the passed account
	ID.  If the account ID is invalid then NULL is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_DIAL_STR_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	NULL terminated csd dial string or NULL
**/	
void CAPI2_DATA_GetCSDDialNumber(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set the account for a digital or analog modem.  If the account ID
	or dial type is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		dialType	(in)	CSD Dial Type
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_DIAL_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetCSDDialType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDialType_t dialType);


//***************************************************************************************
/**
	Get the modem type associated with the passed account ID. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_DIAL_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	CSDDialType_t
**/	
void CAPI2_DATA_GetCSDDialType(UInt32 tid, UInt8 clientID, UInt8 acctID);


//***************************************************************************************
/**
	Set the user data rate for the passed account.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		baudRate	(in)	CSD Baud Rate
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_BAUD_RATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_SetCSDBaudRate(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDBaudRate_t baudRate);


//***************************************************************************************
/**
	Get the user data rate associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_BAUD_RATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	CSDBaudRate_t
**/	
void CAPI2_DATA_GetCSDBaudRate(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set the synchronization type for the passed account.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		synctype	(in)	CSD sync type
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_SYNC_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_DATA_SetCSDSyncType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDSyncType_t synctype);

//***************************************************************************************
/**
	Get the synchronization type associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_SYNC_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The CSD sync type.
**/
void CAPI2_DATA_GetCSDSyncType(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Enable/disable error correction for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_SYNC_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The CSD sync type.
**/
void CAPI2_DATA_SetCSDErrorCorrection(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean enable);

//***************************************************************************************
/**
	Get the error correction setting for the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The CSD error correction.
**/
void CAPI2_DATA_GetCSDErrorCorrection(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set the error correction type for the passed account ID.  If the account ID
	is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		errCorrectionType	(in) CSD error correction type.
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None.
**/
void CAPI2_DATA_SetCSDErrCorrectionType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);

//***************************************************************************************
/**
	Get the error correction type associated with the passed account.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	CSD error correction type.
**/
void CAPI2_DATA_GetCSDErrCorrectionType(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set the compression type associated with the passed account ID.  If the account ID
	is invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		dataCompType	(in)	CSD Data compression type.
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None.
**/
void CAPI2_DATA_SetCSDDataCompType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDataCompType_t dataCompType);

//***************************************************************************************
/**
	Get the Compression type associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	CSD Data compression type..
**/
void CAPI2_DATA_GetCSDDataCompType(UInt32 tid, UInt8 clientID, UInt8 acctID);

//***************************************************************************************
/**
	Set connection element for the associated account ID.  If the account ID is
	invalid an error is returned.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@param		connElement	(in)	Connection element.
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_SET_CSD_CONN_ELEMENT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None.
**/
void CAPI2_DATA_SetCSDConnElement(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDConnElement_t connElement);

//***************************************************************************************
/**
	Get the connection element associated with the passed account ID.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acctID	(in)	Account ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_GET_CSD_CONN_ELEMENT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Connection element.
**/
void CAPI2_DATA_GetCSDConnElement(UInt32 tid, UInt8 clientID, UInt8 acctID);

//==============================================================================
//	File System - Data Account APIs
//==============================================================================

//***************************************************************************************
/**
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_DATA_UpdateAccountToFileSystem(UInt32 tid, UInt8 clientID);

//==============================================================================
//	Internal functions
//==============================================================================

//***************************************************************************************
/**
	This functions resets the data send/recv size counter
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid	(in)	Context ID
	@return		None
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_RESET_DATA_SIZE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/	
void CAPI2_DATA_resetDataSize(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID.
	@param		size (in) Data sent size.
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_addDataSentSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size);

//***************************************************************************************
/**
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in) Context ID.
	@param		size (in) Data received size.
	@note
	The Async response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_addDataRcvSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size);

/** @} */

/// @endcond 

#endif // _CAPI2_DATAACCT_H_

