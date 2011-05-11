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
*   @file   capi2_phonectrl_api_old.h
*
*   @brief  This file defines the capi2 api's related to phone control
*
****************************************************************************/
#ifndef CAPI2_PHONECTRL_API_OLD_H
#define CAPI2_PHONECTRL_API_OLD_H

#ifdef __cplusplus
extern "C" {
#endif



/**
Rx Level
**/
typedef struct {
	UInt8				RxLev;		///< RxLev
	UInt8				RxQual;		///< RxQual
}MsRxLevelData_t;


/**
AT Command response
**/
typedef struct {
	Int16				len;		///< At Response string len
	UInt8				*buffer;	///< Response Buffer
	UInt8				chan; ///< AT channel on AP
}AtResponse_t;



/**
CAPI2 Response callback result data buffer handle.
**/
typedef void* ResultDataBufHandle_t;

typedef UInt8* CAPI2_Patch_Revision_Ptr_t;
//***************************************************************************************
/**
    Function to set a registered client's Registered Event Mask List.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in) List of event masks.
	@param		maskLen (in) Number of event masks passed in "maskList".
	@return		None
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_SET_REGISTERED_EVENT_MASK_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean ( TRUE for success; FALSE for failure. )

	@note
	This function is called to set the Registered Event Mask list for a registered client.
	For an unsolicited event to be broadcast to a specific client, the event's message type must
	be included in the client's registered event mask list. Specifically, there must be at least
	one registered event mask in the list whose logical AND operation with the event's message type
	is equal to the event's message type itself.

    Typically the client can just use the default setting created by SYS_RegisterForMSEvent()
	which is: one registered event mask of 0xFFFF to receive all unsolicited events.
	If the default setting is used, there is no need to call this function.

    This function can be called for more flexibility with the event mask set to the XOR value of
	the message types of all unsolicited events the client wants to receive.

**/

void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);

//***************************************************************************************
/**
    Function to set a registered client's Filtered Event Mask List.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in) List of event masks.
	@param		maskLen (in) Number of event masks passed in "maskList".
	@param		enableFlag (in) Flag to control the filter.
	@return		None
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_SET_FILTERED_EVENT_MASK_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean ( TRUE for success; FALSE for failure. )

	@note
	This function is called to set the Filtered Event Mask list for a registered client.
	For an unsolicited event to be broadcast to a specific client, the event's message type must
	NOT be included in the client's filtered event mask list. Specifically, there must not be
	one filtered event mask in the list whose logical AND operation with the event's message type
	is equal to the event's message type itself.

	Typically the client can just use the default setting created by SYS_RegisterForMSEvent()
	which is: one registered event mask of 0xFFFF to receive all unsolicited events.
	If the default setting is used, there is no need to call this function.

    This function can be called for more flexibility with the event mask set to the XOR value of
	the message types of all unsolicited events the client do not want to receive.
**/

void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);



//***************************************************************************************
/**
    Function to set CP (Modem processor) to Sleep mode
	@param	bSleepMode (in) TRUE will set the Sleep mode, FALSE to set to normal mode
	@return	TRUE on success or FALSE
	@note
	This function is used to stop receiving call registration status , signal strength etc when AP goes to deep sleep.
**/
Boolean CAPI2_SYS_SetSleepMode(Boolean bSleepMode);

//***************************************************************************************
/**
    Called by Open OS layer when interrupt is triggered from CP to AP. The CAPI will internally determine which
	interface needs processing and consequently trigggers callback
	for either Ack, response, packet data or CSD Data.

	@return	none
	@note
	This function should not be called in ISR directly but in high level task.
**/
void CAPI2_SYS_CP_NotificationHandler(void);



/** @} */

/**
MS PLMN Info
**/
typedef struct {
	Boolean					matchResult;	///< Match result
	UInt16					mcc;			///< mcc
	UInt16					mnc;			///< mnc
	PLMN_NAME_t				longName;		///< Long plmn name
	PLMN_NAME_t				shortName;		///< Short plmn name
	PLMN_NAME_t				countryName;	///< Country Name
}MsPlmnInfo_t;


#define MAX_IMEI_STR	20 ///< Should be atleast 16

/**
IMEI string
**/
typedef struct
{
	UInt8	imei_str[MAX_IMEI_STR];	///< imei string
}MSImeiStr_t;


//=========================================================================
// MS State/Information Functions
//=========================================================================
//GSM & GPRS status

//***************************************************************************************
/**
	Function to check if MS is GSM registered.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GSM_REGISTERED_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
	@note This function is used to check if the MS has camped on a cell and thereby ready
	to perform normal GSM functions.
**/
void CAPI2_MS_IsGSMRegistered(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to check if MS is GPRS registered.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GPRS_REGISTERED_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
	@note This function is used to check if the MS is GPRS attached. If true, the MS is ready
	to perform normal GPRS operations.
**/
void CAPI2_MS_IsGPRSRegistered(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	Function to get the GSM registration reject cause generated locally
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GSM_CAUSE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	NetworkCause_t
	@note 	This function returns the GSM registration reject cause generated locally.
**/
void CAPI2_MS_GetGSMRegCause(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	Function to get the GPRS registration reject cause generated locally
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GPRS_CAUSE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	NetworkCause_t
	@note This function returns the GPRS registration reject cause generated locally.
**/
void CAPI2_MS_GetGPRSRegCause(UInt32 tid, UInt8 clientID);



//***************************************************************************************
/**
	Function to get the MS' registration Location Area Code
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_REGISTERED_LAC_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	LACode_t
**/
void CAPI2_MS_GetRegisteredLAC(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	Function to get the MS' registred Mobile Country Code
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_PLMN_MCC_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt16 The mobile country code of the MS
	@note	This function returns the mobile country code of the network on which the
	MS is registered.
**/
void CAPI2_MS_GetPlmnMCC(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	Function to get the MS' registred Mobile Network Code
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_PLMN_MNC_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt8 The mobile network code of the MS
	@note	This function returns the mobile network code of the network on which the
	MS is registered.
**/
void CAPI2_MS_GetPlmnMNC(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function de-registers the mobile from the network and
	powers down the system.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_POWERDOWN_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SYS_ProcessPowerDownReq(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function powers-up the platform with No RF activity.  In this state
	the system is functional but can not access the network.  It is typically
	used to allow access to run the mobile in a restricted environment such as
	an airplane.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_POWERUP_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SYS_ProcessNoRfReq(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function powers-up the platform.  The mobile will start to search for
	a network on which to camp and will broadcast events to registered clients.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_POWERUP_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SYS_ProcessPowerUpReq(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This is a utility to check if GPRS is allowed
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GPRS_ALLOWED_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean TRUE if GPRS is allowed
	@note
	This function is used to check if GPRS is allowed. If the MS is a class B
	device, GPRS should be allowed during GSM voice or data call.
**/
void CAPI2_MS_IsGprsAllowed(UInt32 tid, UInt8 clientID);



//***************************************************************************************
/**
	Function to request the current RAT
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_CURRENT_RAT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt8 Current RAT
	@note
	This function is used to check the current Radio Access Technology(RAT).
	Possible return values are :<br>
	RAT_NOT_AVAILABLE = 0<br>
	RAT_GSM			  = 1<br>
	RAT_UMTS		  = 2
**/
void CAPI2_MS_GetCurrentRAT(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to request the current frequency band.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_CURRENT_BAND_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt8 Current frequency band
	@note
	This function is used to return the current frequency band the MS is registered on.
	Possible return values are :<br>
		BAND_GSM_900	<br>
		BAND_GSM_900_P<br>
		BAND_GSM_900_E	<br>
		BAND_GSM_900_R	<br>
		BAND_GSM_1800	<br>
		BAND_GSM_1900	<br>
		BAND_GSM_850	<br>
		BAND_GSM_450	<br>
		BAND_GSM_480	<br>
		BAND_GSM750		<br>
		BAND_GSM_T_GSM_380	<br>
		BAND_GSM_T_GSM_410	<br>
		BAND_GSM_T_GSM_900	<br>
		BAND_UMTS_BAND_I	(UMTS 2100 MHz band)<br>
		BAND_UMTS_BAND_II	(UMTS 1900 MHz band)<br>
		BAND_UMTS_BAND_III	<br>
		BAND_UMTS_BAND_IV	(UMTS 1700 MHz band)<br>
		BAND_UMTS_BAND_V	(UMTS 850 MHz band)<br>
		BAND_UMTS_BAND_VI
**/
void CAPI2_MS_GetCurrentBand(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function perform the selection of PLMN network.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ucs2 (in) Set if the plmn is in ucs2 format
	@param		selectMode (in) PLMN selection mode
	@param		format (in) Format of plmn
	@param		plmnValue (in) PLMN value
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_SELECT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt16 ( One of the PCHRejectCause_t values )
**/
void CAPI2_MS_PlmnSelect(
						UInt32 tid,
						UInt8 clientID,
						Boolean ucs2,
						PlmnSelectMode_t selectMode,
						PlmnSelectFormat_t format,
						char *plmnValue);

//***************************************************************************************
/**
    This function performs either: the abortion of current manual PLMN selection; revert back to 
	previous PLMN selection mode before manual PLMN selection (typically used after manual PLMN selection fails). 
	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_ABORT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_AbortPlmnSelect(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function get the current PLMN select mode.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_PLMN_MODE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PlmnSelectMode_t
**/
void CAPI2_MS_GetPlmnMode(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function set the current PLMN select mode.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) PLMN select mode
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_SET_PLMN_MODE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetPlmnMode(UInt32 tid, UInt8 clientID, PlmnSelectMode_t mode);

//***************************************************************************************
/**
    This function get the current PLMN select format.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_PLMN_FORMAT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PlmnSelectFormat_t
**/
void CAPI2_MS_GetPlmnFormat(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function set the current PLMN select format.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		format (in) PLMN format
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_SET_PLMN_FORMAT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetPlmnFormat(UInt32 tid, UInt8 clientID, PlmnSelectFormat_t format);



//***************************************************************************************
/**
	This function determines if the passed MCC-MNC matches the MCC-MNC returned from the network.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		net_mcc (in) mcc
	@param		net_mnc (in) mnc
	@param		mcc (in) mcc
	@param		mnc (in) mnc
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_MATCH_PLMN_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean

	@note
	The passed MCC-MNC supports the use of "wild-carding" in MCC and MNC:
	if a digit is set to Hex 'D', it matches any digit value in network MCC-MNC.
**/
void CAPI2_MS_IsMatchedPLMN(UInt32 tid, UInt8 clientID, UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc);

//***************************************************************************************
/**
    This function perform the search of available PLMN.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_SEARCH_PLMN_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SearchAvailablePLMN(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function aborts the search of available PLMN.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_ABORT_PLMN_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_AbortSearchPLMN(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function get the network names (long and short) based upon the  MCC/MNC/LAC tuple.
	The order of priority is as follows:
	-# EONS (based on MCC, MNC, LAC and EF-OPL & EF-PNN in EONS-enabled SIM
	-# CPHS ONS & ONSS (based on EF-ONS & EF-ONSS in CPHS-enabled 2G SIM)
	-# NITZ network name in MM/GMM message received from network
	-# Internal MCC/MNC lookup table (based on SE.13, NAPRD10 and carrier requirements)

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mcc registered MCC in raw format, e.g. 0x13F0 for Cingular Wireless
	@param		mnc registered MNC in raw format, e.g. 0x71 for Cingular Wireless
	@param		lac registered LAC in raw format
	@param		ucs2 TRUE if UCS2 format of long name and short name is perferred
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_NAME_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	MsPlmnName_t
**/
void CAPI2_MS_GetPLMNNameByCode(UInt32 tid, UInt8 clientID,	UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2);

//***************************************************************************************
/**
    This function requests an immediate auto network search.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_AUTO_SEARCH_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_AutoSearchReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param	inClientInfoPtr	(in) pointer to clientinfo of the calling client 
	@param	bandSelect (in) 
	@return	void
	@note
**/
void CAPI2_NetRegApi_SelectBand(ClientInfo_t* inClientInfoPtr, BandSelect_t bandSelect);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param	inClientInfoPtr	(in) pointer to clientinfo of the calling client 
	@param	RAT_cap (in) 
	@param	band_cap (in) 
	@return	void
	@note
**/
void CAPI2_NetRegApi_SetSupportedRATandBand(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param	inClientInfoPtr	(in) pointer to clientinfo of the calling client 
	@param	Enable (in)
	@param	lockBand (in)
	@param	lockrat (in)
	@param	lockuarfcn (in)
	@param	lockpsc (in)
	@return	void
	@note
**/
void CAPI2_NetRegApi_CellLock(ClientInfo_t* inClientInfoPtr, Boolean Enable, BandSelect_t lockBand, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);


//***************************************************************************************
/**
	Function to get the country code string by mcc
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mcc (in) country code
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_GET_MCC_COUNTRY_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Null terminated country string
**/
void CAPI2_PLMN_GetCountryByMcc(UInt32 tid, UInt8 clientID, UInt16 mcc);

//***************************************************************************************
/**
	Function to query the plmn list size
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_LIST_SIZE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	UInt16 ( size of plmn list )
**/
void CAPI2_MS_GetPLMNListSize(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to gets the plmn info based in the input index
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		index (in) index from the table to retrieve record
	@param		ucs2 TRUE if UCS2 format of long name and short name is perferred
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_INFO_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	MsPlmnInfo_t
**/
void CAPI2_MS_GetPLMNEntryByIndex(
					UInt32 tid,
					UInt8 clientID,
					UInt16	index,
					Boolean	ucs2);


//***************************************************************************************
/**
	Function to gets the plmn info based plmn codes
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ucs2 (in) TRUE if UCS2 format of long name and short name is perferred
	@param		plmn_mcc (in) mcc
	@param		plmn_mnc (in) mnc
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_PLMN_INFO_BY_INDEX_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	MsPlmnInfo_t
**/
void CAPI2_MS_GetPLMNByCode(
					UInt32 tid,
					UInt8 clientID,
					Boolean ucs2,
					UInt16	plmn_mcc,
					UInt16	plmn_mnc);


//***************************************************************************************
/**
    This function queries system state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_SYSTEM_STATE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	SystemState_t
**/
void CAPI2_SYS_GetSystemState(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function sends request to set system state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		state (in) System state
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_SET_SYSTEM_STATE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SYS_SetSystemState(UInt32 tid, UInt8 clientID, SystemState_t state);


//***************************************************************************************
/**
    This function queries the Rx Signal level
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_RX_LEVEL_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	MsRxLevelData_t
**/
void CAPI2_SYS_GetRxSignalInfo(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function set the flag to enable/disable broadcasting the cell info message.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param      inEnableCellInfoMsg (in) flag to update
	@return		None
	@note
	@n@b Responses 

**/
void CAPI2_SYS_EnableCellInfoMsg(UInt32 tid, UInt8 clientID, Boolean inEnableCellInfoMsg);

//GSM & GPRS status

//***************************************************************************************
/**
    This function checks the GSM registration status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_GSMREG_STATUS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	RegisterStatus_t
**/
void CAPI2_SYS_GetGSMRegistrationStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function checks the GPRS registration status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_GPRSREG_STATUS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	RegisterStatus_t
**/
void CAPI2_SYS_GetGPRSRegistrationStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function checks if either GSM or GPRS is registered
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_REG_STATUS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_SYS_IsRegisteredGSMOrGPRS(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function checks the GSM registration status cause
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_GET_GSMREG_CAUSE_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	PCHRejectCause_t
**/
void CAPI2_SYS_GetGSMRegistrationCause(UInt32 tid, UInt8 clientID);



//***************************************************************************************
/**
    This function checks if the PLMN is forbid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SYS_IS_PLMN_FORBIDDEN_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_MS_IsPlmnForbidden(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function checks if home plmn is registered
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_HOME_PLMN_REG_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_MS_IsRegisteredHomePLMN(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sets the power down timer
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		powerDownTimer (in) timer value
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SET_POWER_DOWN_TIMER_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetPowerDownTimer(UInt32 tid, UInt8 clientID, UInt8 powerDownTimer);


//***************************************************************************************
/**
	Function request to set the pre-power-cycle frequency band.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		startBand (in) starting band
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_START_BAND_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
	@note
	The valid band should be:
		START_BAND_GSM900_DCS1800_UMTS2100<br>
		START_BAND_PCS1900_GSM850_UMTS1900_UMTS850

**/
void CAPI2_MS_SetStartBand(UInt32 tid, UInt8 clientID, UInt8 startBand);

//***************************************************************************************
/**
	Function to select band to 900/1800/1900/Dual
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		bandSelect (in) band to select
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_SELECT_BAND_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_SYS_SelectBand( UInt32 tid, UInt8 clientID, BandSelect_t bandSelect );

//***************************************************************************************
/**
	This function sets the RATs (Radio Access Technologies) and bands to be supported by platform.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param RAT_cap	GSM_ONLY(0), DUAL_MODE_GSM_PREF(1), DUAL_MODE_UMTS_PREF(2), or UMTS_ONLY(3)
	@param band_cap	combination of GSM and/or UMTS bands (no need to set BAND_AUTO bit)<br>
					e.g., BAND_GSM900_DCS1800 | BAND_PCS1900_ONLY (triband GSM, no UMTS)<br>
						  BAND_UMTS2100_ONLY (single band UMTS, no GSM)<br>
						  BAND_GSM900_DCS1800 | BAND_PCS1900_ONLY | BAND_UMTS2100_ONLY (triband GSM + single band UMTS)<br>
						  BAND_GSM900_DSC1800 | BAND_PCS1900_GSM850 | BAND_UMTS2100_ONLY (quadband GSM + single band UMTS)<br>
						(if RAT is GSM_ONLY, the UMTS band setting will be ignored;
						 if RAT is UMTS_ONLY, the GSM band setting will be ignored)
						or
						BAND_NULL
						(this resumes supported band setting to system setting from sysparm)
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_SET_RAT_BAND_RSP
	@n@b Result_t :		RESULT_OK if the RATs and bands specified is allowed by sysparm and
						they are consistent with each other
	@n@b ResultData :	None
	@note
	The settings specified by this function won't be in effect until
	SYS_ProcessPowerUpReq() (in power-off state) or
	SYS_SelectBand(BAND_AUTO) (in power-on state) is called.
**/
void CAPI2_MS_SetSupportedRATandBand(UInt32 tid, UInt8 clientID, RATSelect_t RAT_cap, BandSelect_t band_cap);


//******************************************************************************
/**
	This function returns the system RAT setting supported in platform which is defined
	in System Parameter file.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_GET_SYSTEM_RAT_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::RATSelect_t

**/
void CAPI2_MS_GetSystemRAT(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function returns the current RAT setting supported in platform. The supported
	RAT is the same as system RAT by default upon powerup, but supported RAT can be
	changed to be a subset of system RAT through CAPI2_MS_SetSupportedRATandBand().
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_GET_SUPPORTED_RAT_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::RATSelect_t

*/
void CAPI2_MS_GetSupportedRAT(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function returns the system band setting supported in platform which is defined
	in System Parameter file.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_GET_SYSTEM_BAND_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::BandSelect_t
*/
void CAPI2_MS_GetSystemBand(UInt32 tid, UInt8 clientID);


//******************************************************************************
/**
	This function returns the current band setting supported in platform. The supported
	band is the same as system band by default upon powerup, but supported band can be
	changed to be a subset of system band through CAPI2_MS_SetSupportedRATandBand().
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_GET_SUPPORTED_BAND_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::BandSelect_t
*/
void CAPI2_MS_GetSupportedBand(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function returns the current GPRS multi-slot class. The multi-slot classes
	we support are 1~12 and 33. See Annex B of 3GPP 45.002 for the multi-slot class definition.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYSPARAM_GET_MSCLASS_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::UInt16

**/
void CAPI2_SYSPARM_GetMSClass( UInt32 tid, UInt8 clientID );

//******************************************************************************
/**
	This function returns the manufacture name
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYSPARAM_GET_MNF_NAME_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: Null terminated UInt8*

**/
void CAPI2_SYSPARM_GetManufacturerName(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function returns the MS model name
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYSPARAM_GET_MODEL_NAME_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: Null terminated UInt8*

**/
void CAPI2_SYSPARM_GetModelName(UInt32 tid, UInt8 clientID) ;

//******************************************************************************
/**
	This function returns the software version
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYSPARAM_GET_SW_VERSION_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: Null terminated UInt8*

**/
void CAPI2_SYSPARM_GetSWVersion(UInt32 tid, UInt8 clientID);


//******************************************************************************
/**
	Get GPRS multislot class: Support for 1~12 and 33. See Section Annex B of 3GPP 45.002.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYSPARAM_GET_EGPRS_CLASS_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: UInt16

**/
void CAPI2_SYSPARM_GetEGPRSMSClass( UInt32 tid, UInt8 clientID );


//******************************************************************************
/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is
	posted to clients. Once this function is called, the difference between the new and old RXLEV/RXQUAL
	values (if current RAT is GSM) or RSCP/Ec/Io values (if current RAT is UMTS) must be larger or equal
	to the threshold in order for the MSG_RSSI_IND message to be sent.

    The caller can pass 0 to a threshold to disable the threshold checking.

	@param tid (in) Unique exchange/transaction id which is passed back in the response
	@param clientID (in) Client ID
	@param gsm_rxlev_thresold (in) GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values.
	@param gsm_rxqual_thresold (in) GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values.
	@param umts_rscp_thresold (in) UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param umts_ecio_thresold (in) UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.

	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_SYS_SET_RSSI_THRESHOLD_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: None

**/
void CAPI2_SYS_SetRssiThreshold(UInt32 tid, UInt8 clientID, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);


/**
MS Element
**/
typedef struct
{
	MS_Element_t  inElemType;

	union
	{
		UInt8				u8Data;
		UInt16				u16Data;
		Boolean				bData;
		MSNetAccess_t		netAccess;
		MSNwOperationMode_t	netOper;
		MSNwType_t			netMsType;
		MS_TestChan_t		testChannel;
		PLMN_t				plmn;
		PlmnSelectMode_t	mode;
		PlmnSelectFormat_t	format;
		UInt8				u3Bytes[3];
		UInt8				u10Bytes[10];
        ECC_REC_LIST_t      ecc_list;
		NVRAMClassmark_t	stackClassmark;
		UInt8               imeidata[15];
		Fax_Param_t			faxParam;
	}data_u;
}CAPI2_MS_Element_t;

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to update the MS Database elements. This function will copy the element
	passed in the third argument in to the database.
	Note: After proper range checks in neccessary the value will be updated.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID, used to verify whether the client is allowed to modify
				the element. 
	@param		data (in) The database element type and data

	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_SET_ELEMENT_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: None
**/
void CAPI2_MS_SetElement(UInt32 tid, UInt8 clientID, CAPI2_MS_Element_t *data);
void CAPI2_MsDbApi_SetElement(ClientInfo_t* client_info, CAPI2_MS_Element_t *inElemData);

//**************************************************************************************
/**
	This function is a generic interface that will be used by any clients (external/
	internal) to read any MS Database elements. This function will copy the contents of
	the database value to the memory location passed in by the last argument.
	The calling entity should know what will be the typecast used to retreive the element.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID, used to identify that the client is allowd to
				access the element or not.
	@param		inElemType (in) The database element type.

	@n@b Responses
	@n@b MsgType_t: ::MSG_MS_GET_ELEMENT_RSP
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: CAPI2_MS_Element_t
**/
void CAPI2_MS_GetElement(UInt32 tid, UInt8 clientID, MS_Element_t inElemType);
void CAPI2_MsDbApi_GetElement(ClientInfo_t* client_info, MS_Element_t inElemType);


/** @} */

/****************************************************************************/
/**

*   @defgroup   CAPI2_AuxAdcDrvGroup   Aux ADC Group
*
*   @brief      This group defines the interfaces to access ADC functions
*
*
****************************************************************************/
/**
 * @addtogroup CAPI2_AuxAdcDrvGroup
 * @{
 */



/** @} */

/************************************************ INTERNAL CAPI2 Functions for CSS APPS ONLY ****************************************************/
 /** @cond */

typedef struct
{
	UInt16			pgsm_supported;
	UInt16			egsm_supported;
	UInt16			dcs_supported;
	UInt16			pcs_supported;
	UInt16			gsm850_supported;
	UInt16			pgsm_pwr_class;
	UInt16			egsm_pwr_class;
	UInt16			dcs_pwr_class;
	UInt16			pcs_pwr_class;
	UInt16			gsm850_pwr_class;
	UInt16			ms_class_hscsd;
	
	UInt16			a5_1;
	UInt16			a5_2;
	UInt16			a5_3;
	UInt16			a5_4;
	UInt16			a5_5;
	UInt16			a5_6;
	UInt16			a5_7;
	
	UInt16			ms_class;
	UInt16			ms_class_egprs;


	UInt16			gea_1;
	UInt16			gea_2;
	UInt16			gea_3;
	UInt16			gea_4;
	UInt16			gea_5;
	UInt16			gea_6;
	UInt16			gea_7;

} CAPI2_Class_t;


//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::UInt16
	@note
*/
void CAPI2_SYSPARM_GetSysparmIndPartFileVersion(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::char*
	@note
*/
void CAPI2_SYS_GetBootLoaderVersion(UInt32 tid, UInt8 clientID, UInt8 BootLoaderVersion_sz);

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::char*
	@note
*/
void CAPI2_SYS_GetDSFVersion(UInt32 tid, UInt8 clientID, UInt8 DSFVersion_sz);

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::UInt16
	@note
*/
void CAPI2_SYSPARM_GetChanMode( UInt32 tid, UInt8 clientID );

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::CAPI2_Class_t
	@note
*/
void CAPI2_SYSPARM_GetClassmark( UInt32 tid, UInt8 clientID );


//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::
	@note
*/
void CAPI2_SYSPARM_SetDARPCfg(UInt32 tid, UInt8 clientID, UInt8 darp_cfg);

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::
	@note
*/
void CAPI2_SYSPARM_SetEGPRSMSClass( UInt32 tid, UInt8 clientID, UInt16 egprs_class );

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::
	@note
*/
void CAPI2_SYSPARM_SetGPRSMSClass( UInt32 tid, UInt8 clientID, UInt16 gprs_class );


//******************************************************************************
/**
	This function This function erases the NITZ Network Name saved in RAM &
	file system. This is required to pass NITZ GCF TC 44.2.9.1.2.

	After calling this function, MMI may call MS_GetPLMNNameByCode()
	again to refresh the registered PLMN name displayed.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: None
	@note
*/
void CAPI2_TIMEZONE_DeleteNetworkName(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	This function
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: None
	@note
*/
void CAPI2_TIMEZONE_UpdateRTC(UInt32 tid, UInt8 clientID, Boolean updateFlag);


//******************************************************************************
/**
    Function to get time zone update mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: ::TimeZoneUpdateMode_t
	@note
	Possible update modes are manual, auto, user_confirm, none.
*/
void CAPI2_TIMEZONE_GetTZUpdateMode(UInt32 tid, UInt8 clientID);


//******************************************************************************
/**
    Function to set time zone update mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) TimeZoneUpdateMode_t
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t: ::MSG_
	@n@b Result_t: ::RESULT_OK.
	@n@b ResultData: None
	@note
*/
void CAPI2_TIMEZONE_SetTZUpdateMode(UInt32 tid, UInt8 clientID, TimeZoneUpdateMode_t mode);


//***************************************************************************************
/**
	Function to
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_xx_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	??
**/
void CAPI2_MS_GetGPRSRegState(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_xx_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	??
**/
void CAPI2_MS_GetGSMRegState(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_xx_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	??
**/
void CAPI2_MS_GetRegisteredCellInfo(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_xx_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	??
**/
void CAPI2_MS_SetMEPowerClass(UInt32 tid, UInt8 clientID, UInt8 band, UInt8 pwrClass);

//***************************************************************************************
/**
	Function to check if de-register is in progress
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_IS_DEREGISTER_IN_PROGRESS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_MS_IsDeRegisterInProgress(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to check if register is in progress
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_IS_REGISTER_IN_PROGRESS_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_MS_IsRegisterInProgress(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to select plmn rat
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		manual_rat (in) RAT_NOT_AVAILABLE (no RAT specified); RAT_GSM; RAT_UMTS
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_SET_PLMN_SELECT_RAT_RSP
	@n@b Result_t :		RESULT_OK, RESULT_ERROR
	@n@b ResultData :	None
**/
void CAPI2_MS_SetPlmnSelectRat(UInt32 tid, UInt8 clientID, UInt8 manual_rat);

//******************************************************************************
/** 
	Function to request the protocol stack to return test parameters (e.g. measurement report)
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inPeriodicReport, True to request stack to report TestParam periodically
	@param      inTimeInterval		The time interval between peiodic reports
       @return		None
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_DIAG_MEASURE_REPORT_RSP
	@n@b Result_t :		RESULT_OK
	@n@b ResultData :	None

**/
void CAPI2_DIAG_ApiMeasurmentReportReq(UInt32 tid, UInt8 clientID, Boolean inPeriodicReport, UInt32 inTimeInterval);

//******************************************************************************
/**
	Function to enable or disable Cell Lock Feature
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cell_lockEnable, TRUE if to enable Cell Lock , FALSE otherwise
      @return		None
      	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_DIAG_CELLLOCK_RSP
	@n@b Result_t :		RESULT_OK
	@n@b ResultData :	None

**/
void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable);

//******************************************************************************
/** 
	Function to query the Cell Lock Feature staus.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
       @return		None
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_DIAG_CELLLOCK_STATUS_RSP
	@n@b Result_t :		RESULT_OK
	@n@b ResultData :	Boolean

**/
void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID);

//******************************************************************************
/**
	Function to force the GPRS stack to stay in the ready-state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		forcedReadyState	 (in) forced Ready State	
	@return		None
	@note
	The Async SYS response is as follows
	@n@b Responses
	@n@b MsgType_t :	MSG_MS_FORCEDREADYSTATE_RSP
	@n@b Result_t :		RESULT_OK
	@n@b ResultData :	None

**/
void CAPI2_MS_ForcedReadyStateReq(UInt32 tid, UInt8 clientID, Boolean forcedReadyState);

	
/** @endcond */

void CAPI2_MS_InitCallCfg(UInt32 tid, UInt8 clientID);
void CAPI2_MS_InitFaxConfig(UInt32 tid, UInt8 clientID);
void CAPI2_MS_InitVideoCallCfg(UInt32 tid, UInt8 clientID);
void CAPI2_MS_InitCallCfgAmpF(UInt32 tid, UInt8 clientID);

void CAPI2_PATCH_GetRevision(UInt32 tid, UInt8 clientID);
void CAPI2_TestCmds(UInt32 tid, UInt8 clientID, UInt32 testId, UInt32 param1, UInt32 param2, unsigned char* buffer);
void CAPI2_MS_SetRuaReadyTimer(UInt32 tid, UInt8 clientID, UInt32 inRuaReadyTimer);
Int32 GetCapi2MSElementSize(MS_Element_t inElemType);//Utility function and not really capi2 req/resp

void CAPI2_BCD_CheckString2PBCD( UInt32 tid, UInt8 clientID, char* str);
void CAPI2_DIALSTR_IsValidString( UInt32 tid, UInt8 clientID, const UInt8* str);
void CAPI2_UTIL_Cause2NetworkCause( UInt32 tid, UInt8 clientID, Cause_t inCause);
void CAPI2_UTIL_ErrCodeToNetCause( UInt32 tid, UInt8 clientID, SS_ErrorCode_t inErrCode);
void CAPI2_IsGprsDialStr( UInt32 tid, UInt8 clientID, const UInt8* inDialStr);
void CAPI2_UTIL_GetNumOffsetInSsStr( UInt32 tid, UInt8 clientID, const char* ssStr);
void CAPI2_IsPppLoopbackDialStr( UInt32 tid, UInt8 clientID, const UInt8* str);
void CAPI2_SYS_GetRIPPROCVersion(UInt32 tid, UInt8 clientID);
void CAPI2_SYSPARM_SetHSDPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat);
void CAPI2_MS_GetPrefNetStatus(UInt32 tid, UInt8 clientID);
void CAPI2_SYSPARM_GetHSDPAPHYCategory(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_SetHSUPAPHYCategory(UInt32 tid, UInt8 clientID, int hsupa_phy_cat);
void CAPI2_SYSPARM_GetHSUPAPHYCategory(UInt32 tid, UInt8 clientID);
void CAPI2_InterTaskMsgToAP_RSP(UInt32 tid, UInt8 clientID);
void CAPI2_SYSPARM_GetActualLowVoltReading(UInt32 tid, UInt8 clientID);
void CAPI2_SYSPARM_GetActual4p2VoltReading(UInt32 tid, UInt8 clientID);
void CAPI2_DIALSTR_ParseGetCallType(UInt32 tid, UInt8 clientID, const UInt8* dialStr);

void CAPI2_SYSPARM_GetHSUPASupported(UInt32 tid, UInt8 clientID);
void CAPI2_SYSPARM_GetHSDPASupported(UInt32 tid, UInt8 clientID);
void CAPI2_MS_ForcePsReleaseReq(UInt32 tid, UInt8 clientID);

typedef struct
{
#define SOCKET_PACKETFILTER_WHITELIST                0x00
#define SOCKET_PACKETFILTER_BLACKLIST                0x01
        UInt16        filterType;            // SOCKET_PACKETFILTER_WHITELIST is used only
        Boolean         bAutomaticRrcRelease; // determine to 'RRC Release' or not in 3 sec after getting unsolicited packet
#define SOCKET_PACKETFILTER_UDP_MAX           24  // number of supported UDP port
#define SOCKET_PACKETFILTER_TCP_MAX           48  // number of supported TCP port
#define SOCKET_PACKETFILTER_PORT_NOTSET      0
        UInt16  udpList[SOCKET_PACKETFILTER_UDP_MAX];
        UInt16  tcpList[SOCKET_PACKETFILTER_TCP_MAX];
} WL_SocketFilterList_t;

Result_t WL_PsSetFilterList(ClientInfo_t* inClientInfoPtr, UInt8 inCid, WL_SocketFilterList_t* inDataPtr);


#ifdef __cplusplus
}
#endif

#endif

