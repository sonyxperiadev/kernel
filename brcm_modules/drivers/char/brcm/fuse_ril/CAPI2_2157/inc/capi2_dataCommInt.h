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
*   @file   capi2_dataCommInt.h
*
*   @brief  This file defines the interface for dlink(data connection) API 
			Services.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_DATAConnectionGroup   Data Connection Management
*   @ingroup    CAPI2_DATAServiceGroup
*
*   @brief      This group defines the interfaces to intialize/setup/shutdown  
				data connections
*
*
****************************************************************************/
#ifndef _CAPI2_DATACOMMINT_H
#define _CAPI2_DATACOMMINT_H

/**
 * @addtogroup CAPI2_DATAConnectionGroup
 * @{
 */

#include "capi2_types.h"

#ifndef _DATACOMMINT_H

	typedef Int32	DCId_t;

	/**
	Data connection status
	**/
	typedef enum
	{
		DC_STATUS_CONNECTING,			///< data connection is in connecting state
		DC_STATUS_DIALING,				///< TBD
		DC_STATUS_INITIALIZING,			///< TBD
		DC_STATUS_VERIFYING,			///< data connection enter this state when csd data call to isp just return CONNECT
		DC_STATUS_VERIFIED,				///< TBD
		DC_STATUS_CONNECTED,			///< data connection is connected
		DC_STATUS_DISCONNECTING,		///< data connection is connecting
		DC_STATUS_DISCONNECTED,			///< data connection is disconnected
		DC_STATUS_ERR_NETWORK_FAILURE,	///< TBD
		DC_STATUS_ERR_CALL_BARRED,		///< TBD
		DC_STATUS_ERR_NO_ACCOUNT,		///< data connection failed due to invalid data account id
		DC_STATUS_ERR_NO_CARRIER,		///< data connection failed due to no carrier
		DC_STATUS_ERR_DATA_LINK_BROKEN,	///< TBD
		DC_STATUS_ERR_NO_CONNECTION,	///< TBD
        DC_STATUS_ERR_DHCPCLIENT_FAILURE,	///< WLAN - DHCP client failed
	    DC_STATUS_ERR_AUTOIP_FAILURE	///< WLAN - AUTOIP failed.
	}DC_ConnectionStatus_t;

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
		UInt8					acctId;		///< Data account id
		DC_ConnectionStatus_t	status;		///< Data connection status
		Result_t				rejectCode; ///< PCH rejection code PCHRejectCause_t 
		UInt32					srcIP;		///< Source IP of the phone from PDP activation 
	} DC_ReportCallStatus_t;


#endif	//#ifndef _DATACOMMINT_H


//***************************************************************************************
/**
	This function handles data setup request. 
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param		acctId (in)	Data account id
	@param		linkType (in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server); DC_MS_INITIATED=1, when use as GPRS phone application
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DC_REPORT_CALL_STATUS
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	DC_ReportCallStatus_t
**/
void CAPI2_DC_SetupDataConnection(	UInt32							tid,
									UInt8							clientID,
									UInt8							inClientID,
									UInt8							acctId, 
									DC_ConnectionType_t				linkType );

//***************************************************************************************
/**
	This function handles data setup request for client preferring to share an existing
    active Data Connection. 
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param		acctId (in)	Data account id
	@param		linkType (in)	DC_MODEM_INITIATED=0, when use the phone as a modem (ppp server); DC_MS_INITIATED=1, when use as GPRS phone application
    @param      apnCheck (in) APN associated with the connection client wants to share
    @param      actDCAcctId (out) Account id of the active connection that is shared
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DC_REPORT_CALL_STATUS
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	DC_ReportCallStatus_t
**/
void CAPI2_DC_SetupDataConnectionEx(UInt32							tid,
									UInt8							clientID,
									UInt8							inClientID,
									UInt8							acctId, 
									DC_ConnectionType_t				linkType,
                                    UInt8                           *apnCheck,
                                    UInt8                           *actDCAcctId);

//***************************************************************************************
/**
	This function handles data shutdown request
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inClientID	(in)	Client id that registered to receive the DC connection status messages
	@param		acctId		(in)	Data account id
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_DC_REPORT_CALL_STATUS
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	DC_ReportCallStatus_t
**/
void CAPI2_DC_ShutdownDataConnection(	UInt32							tid,
										UInt8							clientID,
										UInt8							inClientID,								   
										UInt8							acctId );




/** @} */

/// @endcond 

#endif

