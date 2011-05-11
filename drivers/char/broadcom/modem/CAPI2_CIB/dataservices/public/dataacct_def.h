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
*   @file   dataacct_def.h
*
*   @brief  This file contains the definitions for the Data Account 
			Services.
*
****************************************************************************/

#ifndef _DATAACCT_DEF_H_
#define _DATAACCT_DEF_H_


#define		MAX_DATA_ACCOUNT_ID		11     //increased from 10 to 11 since one slot is reserved for STK
#define		DATA_ACCOUNT_ID_FOR_ATC	0x80
#define		INVALID_ACCT_ID			0xFF
/*The last Data Account slot corresponding to MAX_DATA_ACCOUNT_ID is reserved for STK-BIP and ATC/MMI clients should not use it*/
#define     DATA_ACCT_FOR_STK       MAX_DATA_ACCOUNT_ID


#define		MAX_UNERNAME_LEN		32
#define		MAX_PASSWORD_LEN		32
#define		MAX_PDPTYPE_LEN			8
#define		MAX_APN_LEN				32
#define		MAX_STATIC_IP_LEN		20
#define		MAX_DNS_ADDR_LEN		20
#define		MAX_DIAL_NUMBER_LEN		32

#define		DATA_DEFAULT_DATA_COMPRESSION	FALSE
#define		DATA_DEFAULT_HEADER_COMPRESSION	FALSE
#define		DATA_DEFAULT_EMPTY_IP_ADDR		0x00000000

//CHAP
#define		MAX_CHAP_CHALLENGE_LEN	128
#define		MAX_CHAP_RESPONSE_LEN	128
#define		MAX_CHAP_USERID_LEN		128

#define		MAX_NAT_FORWARDS		1


/// Type of data account.  It can either be a Circuit Switch Data account 
/// or a packet switch data account.
typedef enum
{
	DATA_ACCOUNT_NOT_USED,
	DATA_ACCOUNT_GPRS,
	DATA_ACCOUNT_CSD,
	DATA_ACCOUNT_WLAN,
	DATA_ACCOUNT_USB_EEM
}DataAccountType_t;


/// Data authentication method. This data type sepecifies the authentication method
/// when establishing a connection. It can either be PAP or CHAP
typedef enum
{
	DATA_PPP_PAP,			///< PAP authentication mode
	DATA_PPP_CHAP,			///< CHAP authentication mode 
	DATA_PPP_PASS_THROUGH,	///< PPP pass through mode
	DATA_PPP_CHAP_CR		///< CHAP with Challenge and Response
}DataAuthenMethod_t;
#define	DATA_DEFAULT_AUTHEN_TYPE	DATA_PPP_CHAP

/// This is used to specify if the Circuit switch connection is for a analog or digital modem.
typedef enum
{
	DATA_CSD_ANALOG,
	DATA_CSD_ISDN
}CSDDialType_t;
#define	DATA_DEFAULT_DIAL_TYPE		DATA_CSD_ANALOG

/// Baud rate of circuit switched connection in bits per second.
typedef enum
{
	DATA_CSD_BAUD_AUTOBAUDING	= 0,  ///< Autobauding
	DATA_CSD_BAUD_9600			= 7,  ///< 9600bps  V.32
	DATA_CSD_BAUD_14400			= 14, ///< 14400bps V.34
	DATA_CSD_BAUD_19200			= 15, ///< 19200bps V.34
	DATA_CSD_BAUD_28800			= 16, ///< 28800bps V.34
	DATA_CSD_BAUD_38400			= 49, ///< 38400bps V.120
	DATA_CSD_BAUD_48000			= 50, ///< 48000bps V.120
	DATA_CSD_BAUD_56000			= 51, ///< 56000bps V.120
	DATA_CSD_BAUD_9600_V_110	= 71, ///< 9600bps V.110 or X.31
	DATA_CSD_BAUD_48000_V_110	= 82, ///< 48000bps V.110 or X.31
	DATA_CSD_BAUD_56000_V_110	= 83, ///< 56000bps V.110 or X.31 to be used with async NT UDI or RDI
	DATA_CSD_BAUD_64000_V_110	= 83, ///< 64000bps X.31 to be used with async NT UDI.
	DATA_CSD_BAUD_56000_Tran	= 115,///< 56000bps Transparent
	DATA_CSD_BAUD_64000_Tran	= 116,///< 64000bps Transparent
	DATA_CSD_BAUD_32000_PIAFS	= 120,///< 32000bps PIAFS32K
	DATA_CSD_BAUD_64000_PIAFS	= 121,///< 32000bps PIAFS64K
	DATA_CSD_BAUD_28800_MM	    = 130,///< 28800bps MultiMedia
	DATA_CSD_BAUD_32000_MM	    = 131,///< 32000bps MultiMedia
	DATA_CSD_BAUD_33600_MM	    = 132,///< 33600bps MultiMedia
	DATA_CSD_BAUD_56000_MM	    = 133,///< 56000bps MultiMedia
	DATA_CSD_BAUD_64000_MM	    = 134 ///< 64000bps MultiMedia

}CSDBaudRate_t;
#define	DATA_DEFAULT_BAUDRATE		DATA_CSD_BAUD_9600

/// This type specifies the synchronization that is to be used for the connection.
typedef enum
{
  DATA_CSD_ASYNC = 0,			///< Data ckt Async (UDI or 3.1Khz)
  DATA_CSD_SYNC,				///< Data ckt Sync  (UDI or 3.1Khz)
  DATA_CSD_PAD_ACCESS_ASYNC,	///< PAD Access Async(UDI)
  DATA_CSD_PAD_ACCESS_SYNC,		///< PAD Access Sync (UDI)
  DATA_CSD_RDI_ASYNC,			///< RDI Data ckt Async		
  DATA_CSD_RDI_SYNC,			///< RDI Data ckt Sync
  DATA_CSD_PAD_RDI_ASYNC,		///< RDI PAD Access Async
  DATA_CSD_PAD_RDI_SYNC			///< RDI PAD Access Sync
}CSDSyncType_t; 
#define DATA_DEFAULT_SYNCTYPE		DATA_CSD_ASYNC	

/// Error correction types for circuit switch data. The two modes are
/// V.42 and MNP. This is for end to end transparent connection element
typedef enum
{
	DATA_CSD_EC_V42,
	DATA_CSD_EC_MNP
}CSDErrCorrectionType_t;
#define	DATA_DEFAULT_EC_TYPE		DATA_CSD_EC_V42

/// Data compression type. This can be V42bis or MNP for circuit switched connections.
/// For GPRS, only V.42bis is supported. Selecting a particular compression type also selects the
/// appropriate error correction method.  Error correction is always required if compression is enabled.
typedef enum
{
	DATA_CSD_DC_V42BIS,
	DATA_CSD_DC_MNP
}CSDDataCompType_t;
#define	DATA_DEFAULT_DC_TYPE		DATA_CSD_DC_V42BIS

/// Connection element data type. For circuit switched calls, it can either be 
/// transparent or non-trnasparent. For NT mode an RLP link is used and for tranparent
/// mode, error correction has to be negotiated by the peers.
typedef enum
{
	DATA_CSD_CE_TRANSPARENT,			///< 0 -transparent
	DATA_CSD_CE_NON_TRANSPARENT,		///< 1 - non - transparent
	DATA_CSD_CE_BOTH_TRAN_PREFERRED,	///< 2 - both - transparent preferred
	DATA_CSD_CE_BOTH_NON_TRAN_PREFERRED	///< 3 - both - non transparent preferred
}CSDConnElement_t;
#define	DATA_DEFAULT_CONN_ELEMENT	DATA_CSD_CE_NON_TRANSPARENT

/// Packet switched context parameters.
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
	Boolean         isSecondaryPDP;		///< For Secondary PDP context
	UInt8			primaryAcctId;		///< account ID associated with the primary context, applicable for secondary PDPs only
	PCHTrafficFlowTemplate_t tft;		///< Traffic Flow Template
	PCHProtConfig_t	protConfig;         ///< Protocol Configuration Options
    Boolean         bMTPdp;             ///< TRUE for MT when activation is initiated by the Network
    ///<FALSE for MO when activation is initiated by the MS.
    Int32           secTiPd;            ///< transaction id for secondary PDP. 
    ///<for MT, secTiPd is received from the Network; 
    ///<for MO, secTiPd is to be created by the MS
}GPRSContext_t;

/// Circuit Switched Data account parameters
typedef struct 
{	
	UInt8			acctID;						///< Account ID		
	UInt8			*username;					///< Username for the account
	UInt8			*password;					///< Password associated with the account
	UInt8			*staticIPAddr;				///< Static IP address
	UInt8			*priDnsAddr;				///< primary DNS
	UInt8			*sndDnsAddr;				///< Secondary DNS
	UInt8			*dialNumber;				///< Pointer to the dial number
	CSDDialType_t	dialType;					///< Dial type( Analog/ISDN)
	CSDBaudRate_t	baudRate;					///< Connection Baud rate
	CSDErrCorrectionType_t	errCorrectionType;	///< Error Correction type(V.42/MNP)
	Boolean			errorCorrection;			///< Error correction enabled/disabled
	CSDDataCompType_t	dataCompressionType;	///< Data compression type( V42bis/MNP)	
	Boolean			dataCompression;			///< Data Compression enabled/disabled
	CSDConnElement_t	connElement;			///< Connection element(Transparent/Non Transparent)
	UInt8			authenType;					///< PPP authentication type
	CSDSyncType_t	synctype;					///< Synchronization type ( async vs sync )
}CSDContext_t;


/// IPv4 address acquisition modes
typedef enum
{
    // legacy modes available not using universal plug'n'play
    ADDR_MODE_STATIC_IP,                    ///< fixed (static) address
    ADDR_MODE_DHCP,                         ///< dhcp, with no fallback, trying forever
    ADDR_MODE_AUTOIP,                       ///< auto-ip , with no fallback, trying forever

    // new modes available under lightweight universal plug'n'play
    ADDR_MODE_DHCP_AUTO_STATIC,             ///< dhcp, fall back to auto-ip, fall back to static
    ADDR_MODE_DHCP_STATIC,                  ///< dhcp, fall back to static
    ADDR_MODE_AUTO_STATIC                   ///< auto-ip, fall back to static
} AddressMode_t;


/// WLAN data
typedef struct
{
    AddressMode_t       addrMode;           ///< ipv4 address mode
	UInt8			    staticIPAddr[MAX_STATIC_IP_LEN];
	UInt8			    defGw[MAX_STATIC_IP_LEN];
	UInt8			    snMask[MAX_STATIC_IP_LEN];
	UInt8			    priDns[MAX_STATIC_IP_LEN];
	UInt8			    secDns[MAX_STATIC_IP_LEN];
	UInt8			    macAddr[6];
}WLANData_t;

/// GPRS Data type.
typedef struct
{
	UInt8				cid;		///< Context ID
	UInt8			internalAcctId;						///< Internal account Id
	UInt8			useIntAcctIdForDataConnection;		///< Data connection would use the internal acctId if not 0	
}DataGPRS_t;

/// CSD Data type
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

/// WLAN Data type
typedef struct
{
    AddressMode_t       addrMode;                        ///< address mode           
	UInt8	            macAddr[6];
	UInt8	            defGw[MAX_STATIC_IP_LEN];
	UInt8	            snMask[MAX_STATIC_IP_LEN];
}DataWLAN_t;


/// USB EEM Definitions

typedef enum
{
	DS_IP_PROTO_TCP = 6,
	DS_IP_PROTO_UDP = 17
} DS_IpProtoId_t;
/// NAT forwards
typedef struct 
{
	DS_IpProtoId_t	protocolId;							///< UDP/TCP 
	UInt16	internalPortNo;								///< The Internal server port No
	UInt8	internalIpAddr[MAX_STATIC_IP_LEN];			///< The internal server ip address
	UInt16	externalPortNo;								///< The known external port no. for the server
} DS_NatForwards_t;
/// USB EEM Data type
typedef struct
{
    AddressMode_t       addrMode;                        ///< address mode           
	UInt8	            macAddr[6];						 ///< Mac address
	UInt8	            defGw[MAX_STATIC_IP_LEN];		 ///< Gateway address for the interface
	UInt8	            snMask[MAX_STATIC_IP_LEN];		 ///< Subnet mask
	DS_NatForwards_t	natForwards[MAX_NAT_FORWARDS];	 ///< List of NAT forwards for the servers running on internal side
	UInt8				extAcctId;						 ///< Account Id of the external/internet side
}DataUSBEEM_t;


///
typedef struct
{
	DataUSBEEM_t		dataUsbEem;
	UInt8			    staticIPAddr[MAX_STATIC_IP_LEN];
	UInt8			    priDns[MAX_STATIC_IP_LEN];
	UInt8			    secDns[MAX_STATIC_IP_LEN];
	Boolean				bUseIntAcctIdAlways;	///< For internal testing using the mmi browser
} USBEEMData_t;

/// User Account
typedef struct 
{	
	UInt8				acctID;							///< Account ID
	DataAccountType_t	acctType;						///< Account type

	UInt8				username[MAX_UNERNAME_LEN];		///< Account user name
	UInt8				password[MAX_PASSWORD_LEN];		///< Password for the account
	UInt8				staticIPAddr[MAX_STATIC_IP_LEN];///< Static IP address
	UInt8				priDnsAddr[MAX_DNS_ADDR_LEN];	///< Primary DNS address
	UInt8				sndDnsAddr[MAX_DNS_ADDR_LEN];	///< Secondary DNS address
	UInt32				dataSentSize;					///< Data sent size
	UInt32				dataRcvSize;					///< Data Receive Size
	UInt8				authenType;						///< PPP authentication type
	Boolean				acctLock;
	//CHAP
	UInt8			chapID;
	UInt8			chapChallengeLen;
	UInt8			chapResponseLen;
	UInt8			chapUserIdLen;
	UInt8			chapChallenge[MAX_CHAP_CHALLENGE_LEN];
	UInt8			chapResponse[MAX_CHAP_RESPONSE_LEN];
	UInt8			chapUserId[MAX_CHAP_USERID_LEN];
	
	union {
		DataGPRS_t		gprsData;						///< GPRS Data type
		DataCSD_t		csdData;						///< CSD data type
		DataWLAN_t		wlanData;	
		DataUSBEEM_t	usbEemData;
	}userData;


}UserDataAccount_t;

typedef struct
{
	UserDataAccount_t	*dataAccountPtr;
	ClientInfo_t		clientInfo;
}UserDataAccountEx_t;

#endif // _DATAACCT_DEF_H_

