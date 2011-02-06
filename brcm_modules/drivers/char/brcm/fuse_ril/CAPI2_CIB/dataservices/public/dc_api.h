//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
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
*  @file   dc_api.h
*
*  @brief  API to the data connections.
*
*  @note   
*
****************************************************************************/

#ifndef _DC_API_H
#define _DC_API_H

// ---- Include Files -------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

// ---- Macro Declarations -------------------------------------------------------

// ---- Type Definitions -------------------------------------------------------

/// Description of Param1_t should be included here
/// the description can be multiple lines and is terminated when a blank
/// line is encountered.


/**
Data connection status
**/
typedef enum
{
	DC_STATUS_CONNECTING,			///< data connection is in connecting state
	DC_STATUS_DIALING,				///< 
	DC_STATUS_INITIALIZING,			///< 
	DC_STATUS_VERIFYING,			///< data connection enter this state when csd data call to isp just return CONNECT
	DC_STATUS_VERIFIED,				///< 
	DC_STATUS_CONNECTED,			///< data connection is connected
	DC_STATUS_DISCONNECTING,		///< data connection is connecting
	DC_STATUS_DISCONNECTED,			///< data connection is disconnected
	DC_STATUS_ERR_NETWORK_FAILURE,	///< 
	DC_STATUS_ERR_CALL_BARRED,		///< 
	DC_STATUS_ERR_NO_ACCOUNT,		///< data connection failed due to invalid data account id
	DC_STATUS_ERR_NO_CARRIER,		///< data connection failed due to no carrier
	DC_STATUS_ERR_DATA_LINK_BROKEN,	///< 
	DC_STATUS_ERR_NO_CONNECTION		///< 
}DC_ConnectionStatus_t;

/** @} */


/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
Data connection type
**/
typedef enum
{
	DC_MODEM_INITIATED,	///< if the phone is used as modem (ppp server)
	DC_MS_INITIATED		///< if the connection is setup for data over WEDGE/GPRS (such as WAP)
}DC_ConnectionType_t;

/**
Asynchronous message to notify clients data connection information such as connection status, 
source IP, PCH reject code etc.
**/
typedef struct
{
	UInt8					acctId;			///< Data account id
	DC_ConnectionStatus_t		status;			///< Data connection status
	Result_t					rejectCode; 		///< Rejection code
	UInt32					srcIP;			///< Source IP of the phone from PDP activation 
} DC_ReportCallStatus_t;

/** @} */

typedef struct
{
    UInt32 pri_addr;	// primary DNS server address
    UInt32 sec_addr;	//Secondary DNS server addres
} DcDNSServerAddr_t;

/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function handles data setup request. ATC/MMI must register their message posting callback function by using
	inClientId = SYS_RegisterForMSEvent(). ATC/MMI must store the ID for later purpose(like DeRegister).Data service 
	layer will notify application clients the data connection status with asynchronous events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@param inLinkType	(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnection(	UInt8							inClientID,
									UInt8							inAcctId, 
									DC_ConnectionType_t				inLinkType );

/**
	This function handles data setup requests for clients preferring to share a PDP context and APN. If no match is found, a new
    connection is established. Data service layer will notify application clients the data connection status with asynchronous 
    events (refer to DC_ReportCallStatus_t)
	
	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@param inLinkType		(in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server);								
								DC_MS_INITIATED=1, when use as GPRS phone application
    @param inApnCheck     (in)    APN associated with the connection client wants to share
    @param inAcctDCAcctId  (out)   Account id of the active connection that is shared
	@return Result_t, Return PDP_CIRCUIT_CALL_ACTIVE if there is a voice or CSD call active and it is not UMTS network
					  Otherwise return generic RESULT_OK or RESULT_ERROR depending on DC setup conditions
**/
Result_t DC_SetupDataConnectionEx(	UInt8					inClientID,
									UInt8					inAcctId, 
									DC_ConnectionType_t		inLinkType,
                                    UInt8 *inApnCheck, 
                                    UInt8 *inAcctDCAcctId );
/**
	This function handles data shutdown request

	@param inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param inAcctId		(in)	Data account id
	@return Result_t
**/
Result_t DC_ShutdownDataConnection(	UInt8							inClientID,								   
									UInt8							inAcctId );

/** @} */


/**
	This function initializes the data connections
**/
void DC_InitDataConnections(void);



/**
	This function cleans up the active data connections
**/
void DC_DeInitDataConnections(void);

/**
	This function is used to retrieve the DNS server addresses after data connection is setup.

	@param inAcctId		(in)	Data account id
	@return const UInt8 *
**/
DcDNSServerAddr_t DC_GetDnsServerAddr(UInt8 inAcctId);
/**
	This function is get the index of the tcp/ip stack interface for the data connection
	identified by the accountId. 

	@param inAcctId		(in)	Data account id
	@return -1 if error else the interface index/number in the tcp/ip stack interface list.
**/
int DC_GetNetIfaceIndex(UInt8 inAcctId);

/**
	This function gets the accountId from the interface index of the tcp/ip stack interface 

	@param inIface		(in)	TCP/IP stack based interface Id
	@return INVALID_ACCT_ID if error else the interface index/number in the tcp/ip stack interface list.
**/
UInt8 DC_GetAcctIdByNetIfaceIndex(int inIface);

/**
 * @addtogroup DATAConnectionGroup
 * @{
 */

/**
	This function retrieves the P-CSCF address received from the network upon successful PDP activation.

	@param inAcctId		(in)	Data account id
	@return const UInt8 *
**/
const UInt8 *DC_GetPCscfAddr(UInt8 inAcctId);


/** @} */

#ifdef __cplusplus
}
#endif

#endif

