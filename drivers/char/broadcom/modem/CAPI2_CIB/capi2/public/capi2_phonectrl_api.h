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
*   @file   capi2_phonectrl_api.h
*
*   @brief  This file defines the capi2 api's related to phone control
*
****************************************************************************/
/**

*   @defgroup   CAPI2_PhoneControlAPIGroup   Phone Control API
*   @ingroup    CAPI2_PhoneControlGroup
*
*   @brief      This group defines the interfaces to the phone control system and provides
*				API documentation.  
****************************************************************************/


#ifndef CAPI2_PHONECTRL_API_H
#define CAPI2_PHONECTRL_API_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup CAPI2_PhoneControlAPIGroup
 * @{
 */



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

/**
CAPI2 Response callback result data buffer handle.
**/
//typedef void* ResultDataBufHandle_t;

typedef UInt8* CAPI2_Patch_Revision_Ptr_t;

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

Result_t WL_PsSetFilterList(ClientInfo_t* inClientInfoPtr, UInt8 inCid, WL_SocketFilterList_t* inDataPtr);

//***************************************************************************************
/**
	This function de-registers the mobile from the network and powers down the system.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_POWERDOWN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_ProcessPowerDownReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up the platform with No RF activity. In this state the system is functional but can not access the network.  It is typically used to allow access to run the mobile in a restricted environment such as an airplane.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_POWERUP_NORF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_ProcessNoRfReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up from No RF to calibration mode or move from No RF to calibration mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_NORF_CALIB_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_ProcessNoRfToCalib(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up the platform. The mobile will start to search for a network on which to camp and will broadcast events to registered clients.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_POWERUP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_ProcessPowerUpReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function is used to check the system state value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_GET_SYSTEM_STATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: SystemState_t
**/
void CAPI2_PhoneCtrlApi_GetSystemState(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function is used to set the system state value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		state (in) Param is state
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_SET_SYSTEM_STATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_SetSystemState(ClientInfo_t* inClientInfoPtr, SystemState_t state);

//***************************************************************************************
/**
	This function returns the recieved signal level and signal quality 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_GET_RX_LEVEL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: void
**/
void CAPI2_PhoneCtrlApi_GetRxSignalInfo(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to SetPowerDownTimer
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		powerDownTimer (in) Param is powerDownTimer
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SET_POWER_DOWN_TIMER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_SetPowerDownTimer(ClientInfo_t* inClientInfoPtr, UInt8 powerDownTimer);

//***************************************************************************************
/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is posted to clients. Once this function is called the difference between the new and old RXLEV/RXQUAL values if current RAT is GSM or RSCP/Ec/Io values if current RAT is UMTS must be larger or equal to the threshold in order for the MSG_RSSI_IND message to be sent.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		gsm_rxlev_thresold (in)  GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values. 
	@param		gsm_rxqual_thresold (in)  GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values. 
	@param		umts_rscp_thresold (in)  UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param		umts_ecio_thresold (in)  UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_SET_RSSI_THRESHOLD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_SetRssiThreshold(ClientInfo_t* inClientInfoPtr, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);

//***************************************************************************************
/**
	This function is used to enable/disable paging status 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		status (in)  value
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_SET_PAGING_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_PhoneCtrlApi_SetPagingStatus(ClientInfo_t* inClientInfoPtr, UInt8 status);

//***************************************************************************************
/**
	Set band to 900/1800/1900/Dual
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		bandSelect (in) Param is bandSelect
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SELECT_BAND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SelectBand(ClientInfo_t* inClientInfoPtr, BandSelect_t bandSelect);

//***************************************************************************************
/**
	This function sets the RATs Radio Access Technologies and bands to be<br> supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		RAT_cap (in) "RAT setting for the 2nd VM"
	@param		band_cap (in) "Band setting for the 2nd VM"
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SET_RAT_BAND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SetSupportedRATandBand(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap);

//***************************************************************************************
/**
	 This function sets the RATs Radio Access Technologies and bands to be supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		Enable (in) Param is Enable
	@param		lockBand (in) Param is lockBand
	@param		lockrat (in) Param is lockrat
	@param		lockuarfcn (in) Param is lockuarfcn
	@param		lockpsc (in) Param is lockpsc
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_CELL_LOCK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_CellLock(ClientInfo_t* inClientInfoPtr, Boolean Enable, BandSelect_t lockBand, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);

//***************************************************************************************
/**
	This function perform the selection of PLMN network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ucs2 (in) Param is ucs2
	@param		selectMode (in) Param is selectMode
	@param		format (in) Param is format
	@param		plmnValue (in) Param is plmnValue
	
	 Responses 
	 @n@b MsgType_t: ::MSG_PLMN_SELECT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_NetRegApi_PlmnSelect(ClientInfo_t* inClientInfoPtr, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char* plmnValue);

//***************************************************************************************
/**
	This function performs either: the abortion of current manual PLMN selection; revert back to previous PLMN selection mode before manual PLMN selection typically used after manual PLMN selection fails. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_PLMN_ABORT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_AbortPlmnSelect(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function set the current PLMN select mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in) Param is mode
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SET_PLMN_MODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SetPlmnMode(ClientInfo_t* inClientInfoPtr, PlmnSelectMode_t mode);

//***************************************************************************************
/**
	restarts PLMN Search
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SEARCH_PLMN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SearchAvailablePLMN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	abort searching available PLMN
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_ABORT_PLMN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_AbortSearchPLMN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function requests an immediate auto network search.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_AUTO_SEARCH_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_AutoSearchReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Get the network names long and short based upon the MCC/MNC/LAC tuple. The order of priority is as follows: -# EONS based on MCC MNC LAC and EF-OPL & EF-PNN in EONS-enabled SIM -# CPHS ONS & ONSS based on EF-ONS & EF-ONSS in CPHS-enabled 2G SIM -# NITZ network name in MM/GMM message received from network -# Internal MCC/MNC lookup table based on SE.13 NAPRD10 and carrier requirements
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mcc (in)  registered MCC in raw format
	@param		mnc (in)  registered MNC in raw format
	@param		lac (in)  registered LAC in raw format
	@param		ucs2 (in)  TRUE if UCS2 format of long name and short name is perferred
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_PLMN_NAME_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_NetRegApi_GetPLMNNameByCode(ClientInfo_t* inClientInfoPtr, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2);

//***************************************************************************************
/**
	Function to DeleteNetworkName
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_TIMEZONE_DELETE_NW_NAME_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_DeleteNetworkName(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sets the RAT used in Manual PLMN Selection through MS_PlmnSelect.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		manual_rat (in)  RAT_NOT_AVAILABLE no RAT specified; RAT_GSM; RAT_UMTS
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SET_PLMN_SELECT_RAT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SetPlmnSelectRat(ClientInfo_t* inClientInfoPtr, UInt8 manual_rat);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		forcedReadyState (in)  force ready state set to true/false
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_FORCEDREADYSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_ForcedReadyStateReq(ClientInfo_t* inClientInfoPtr, Boolean forcedReadyState);

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		SIMPresent (in)  is SIM present
	@param		SIMType (in)  SIM type
	@param		regType (in)  registration type
	@param		plmn (in)  plmn mcc
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SENDCOMBINEDATTACHREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SendCombinedAttachReq(ClientInfo_t* inClientInfoPtr, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn);

//***************************************************************************************
/**
	Function to send request for gprs detach
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cause (in)  cause of deregistration
	@param		regType (in)  option to select gsm
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SENDDETACHREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SendDetachReq(ClientInfo_t* inClientInfoPtr, DeRegCause_t cause, RegType_t regType);

//***************************************************************************************
/**
	Function to set MS class
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		msClass (in)  MS Class type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_PDP_SETMSCLASS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SetMSClass(ClientInfo_t* inClientInfoPtr, MSClass_t msClass);

//***************************************************************************************
/**
	Client can use this API to notify stack to immeidately release ps for fast dormancy feature
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_FORCE_PS_REL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_ForcePsReleaseReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sets the RATs Radio Access Technologies and bands to be supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		RAT_cap (in)  rat cap
	@param		band_cap (in)  bad cap selected
	@param		RAT_cap2 (in)  sim2 rat
	@param		band_cap2 (in)  sim2 band
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SET_RAT_BAND_EX_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_NetRegApi_SetSupportedRATandBandEx(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap, RATSelect_t RAT_cap2, BandSelect_t band_cap2);

//***************************************************************************************
/**
	This function is a generic interface that will be used by any clients external/internal to update the MS Database elements. This function will copy the element passed in the third argument in to the database.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElemData (in)  The database element type.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_SET_ELEMENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_MsDbApi_SetElement(ClientInfo_t* inClientInfoPtr, CAPI2_MS_Element_t *inElemData);

//***************************************************************************************
/**
	This function is a generic interface that will be used by any clients <br> external/internal to read any MS Database elements. This function will copy the <br>contents of the database value to the memory location passed in by the last argument. <br>The calling entity should know what will be the typecast used to retreive the element.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElemType (in) Param is inElemType
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_GET_ELEMENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: CAPI2_MS_Element_t
**/
void CAPI2_MsDbApi_GetElement(ClientInfo_t* inClientInfoPtr, MS_Element_t inElemType);

//***************************************************************************************
/**
	Function to InitCallCfg
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_MS_INITCALLCFG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_MsDbApi_InitCallCfg(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to SetRegisteredEventMask
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in) Param is maskList
	@param		maskLen (in) Param is maskLen
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_SET_REG_EVENT_MASK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);

//***************************************************************************************
/**
	Function to SetFilteredEventMask
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in) Param is maskList
	@param		maskLen (in) Param is maskLen
	@param		enableFlag (in) Param is enableFlag
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SYS_SET_REG_FILTER_MASK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);

//***************************************************************************************
/**
	Function to ApiCellLockReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cell_lockEnable (in) Param is cell_lockEnable
	
	 Responses 
	 @n@b MsgType_t: ::MSG_DIAG_CELLLOCK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable);

//***************************************************************************************
/**
	Function to ApiCellLockStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_DIAG_CELLLOCK_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function requests that the protocol stack to return test <br>parameters e.g. measurement report
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inPeriodicReport (in)  True to request stack to report TestParam periodically
	@param		inTimeInterval (in)  The time interval between peiodic reports.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_DIAG_MEASURE_REPORT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_DiagApi_MeasurmentReportReq(ClientInfo_t* inClientInfoPtr, Boolean inPeriodicReport, UInt32 inTimeInterval);


/** @} */


#ifdef __cplusplus
}
#endif

#endif

