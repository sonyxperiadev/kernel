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

#ifdef HISTORY_LOGGING
#define	AllocInterTaskMsgFromHeap(msgType, dataLength)	\
		AllocInterTaskMsgFromHeapDbg(msgType, dataLength, __FILE__, __LINE__)
extern InterTaskMsg_t*	AllocInterTaskMsgFromHeapDbg(MsgType_t msgType, UInt16 dataLength, char* fileName, UInt32 lineNumber);
#else // #ifdef HISTORY_LOGGING
extern InterTaskMsg_t*	AllocInterTaskMsgFromHeap(MsgType_t msgType, UInt16 dataLength);
#endif // #ifdef HISTORY_LOGGING

#ifdef HISTORY_LOGGING
#define	FreeInterTaskMsg(inMsg)	\
		FreeInterTaskMsgDbg(inMsg, __FILE__, __LINE__)
extern void	FreeInterTaskMsgDbg(InterTaskMsg_t* inMsg, char* fileName, UInt32 lineNumber);
#else // #ifdef HISTORY_LOGGING
extern void	FreeInterTaskMsg(InterTaskMsg_t* inMsg);
#endif // #ifdef HISTORY_LOGGING

/* Maximum number of secondary clients */
#define	MAX_NOOF_CAPI2_CLIENTS	3


/* Secondary client list */
extern UInt8 capi2ClientIDs[MAX_NOOF_CAPI2_CLIENTS];


Boolean				MS_IsGSMRegistered(void);

//***************************************************************************************
/**
	Function to check if MS is GPRS registered.
	@return		 Boolean TRUE if MS is GPRS registered.
	@note This function is used to check if the MS is GPRS attached. If true, the MS is ready 
	to perform normal GPRS operations.
**/

Boolean				MS_IsGPRSRegistered(void);



//***************************************************************************************
/**
	Function to get the GSM registration reject cause generated locally
	@return		 NetworkCause_t 
	@note 	This function returns the GSM registration reject cause generated locally.
**/

NetworkCause_t	MS_GetGSMRegCause(void);

//***************************************************************************************
/**
	Function to get the GPRS registration reject cause generated locally
	@return		 NetworkCause_t 
	@note This function returns the GPRS registration reject cause generated locally.
**/

NetworkCause_t	MS_GetGPRSRegCause(void);

//***************************************************************************************
/**
	Function to get the GSM registration reject cause from network
	@return		 UInt8 
	@note This function returns the GSM registration reject cause from network.
**/

UInt8	MS_GetGSMRegNetworkCause(void);

//***************************************************************************************
/**
	Function to get the GPRS registration reject cause from network
	@return		 UInt8 
	@note This function returns the GPRS registration reject cause from network.
**/

UInt8	MS_GetGPRSRegNetworkCause(void);

//***************************************************************************************
/**
	Function to get the MS' registration Location Area Code
	@return		 LACode_t 
**/

LACode_t			MS_GetRegisteredLAC(void);

//***************************************************************************************
/**
	Function to get the MS' registred cell information
	@return		 CellInfo_t 
**/

CellInfo_t			MS_GetRegisteredCellInfo(void);

//***************************************************************************************
/**
	Function to get the MS' registred Mobile Country Code
	@return		 UInt16 The mobile country code of the MS 
	@note	This function returns the mobile country code of the network on which the 
	MS is registered.
**/

UInt16				MS_GetPlmnMCC(void);

//***************************************************************************************
/**
	Function to get the MS' registred Mobile Network Code
	@return		 UInt16 The mobile network code of the MS 
	@note	This function returns the mobile network code of the network on which the 
	MS is registered.
**/

UInt8				MS_GetPlmnMNC(void);


//***************************************************************************************
/**
	This function de-registers the mobile from the network and
	powers down the system.
**/

void				SYS_ProcessPowerDownReq(void);

//***************************************************************************************
/**
	This function powers-up the platform with No RF activity.  In this state
	the system is functional but can not access the network.  It is typically
	used to allow access to run the mobile in a restricted environment such as
	an airplane.
**/

void				SYS_ProcessNoRfReq(void);

//***************************************************************************************
/**
	This function powers-up the platform.  The mobile will start to search for
	a network on which to camp and will broadcast events to registered clients.
**/

void				SYS_ProcessPowerUpReq(void);

//***************************************************************************************
/**
	Clients can use this function to query the reason for power-on.
	@return		PowerOnCause_t	Reason for Power-On	
**/

PowerOnCause_t		SYS_GetMSPowerOnCause(void);

//***************************************************************************************
/**
	This is a utility to check if GPRS is allowed
	@return		 Boolean TRUE if GPRS is allowed
	@note	
	This function is used to check if GPRS is allowed. If the MS is a class B 
	device, GPRS should be allowed during GSM voice or data call.
**/

Boolean				MS_IsGprsAllowed(void);


//***************************************************************************************
/**
	Function to return the current RAT
	@return		 UInt8 Current RAT
	@note	
	This function is used to check the current Radio Access Technology(RAT).
	Possible return values are :<br>
	RAT_NOT_AVAILABLE = 0<br>
	RAT_GSM			  = 1<br>
	RAT_UMTS		  = 2
**/

UInt8				MS_GetCurrentRAT(void);

//***************************************************************************************
/**
	Function to return the current frequency band.
	@return		 UInt8 Current frequency band
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

UInt8				MS_GetCurrentBand(void);

void			MS_SetStartBand(UInt8 startBand);

//******************************************************************************
/** 
* For MMI/ATC to query the status of good band info
*					If mobile registeres to network, it would store the last
*					registered band info. Otherwise, it would show the last band
*					info set by MS_SetStartBand().
*
*	@return START_BAND_GSM900_DCS1800_UMTS2100<br>
*			START_BAND_PCS1900_GSM850_UMTS1900_UMTS850
* 
*/
UInt8			MS_GetStartBand(void);

//******************************************************************************
/** 
* Set band to 900/1800/1900/Dual
*
*/
Result_t		SYS_SelectBand( BandSelect_t bandSelect );

//******************************************************************************
/**
* This function sets the RATs (Radio Access Technologies) and bands to be 
* supported by platform.
*
*	@param RAT_cap	GSM_ONLY(0), DUAL_MODE_GSM_PREF(1), DUAL_MODE_UMTS_PREF(2), or UMTS_ONLY(3)
*	@param band_cap	combination of GSM and/or UMTS bands (no need to set BAND_AUTO bit)<br>
*					e.g., BAND_GSM900_DCS1800 | BAND_PCS1900_ONLY (triband GSM, no UMTS)<br>
*						  BAND_UMTS2100_ONLY (single band UMTS, no GSM)<br>
*						  BAND_GSM900_DCS1800 | BAND_PCS1900_ONLY | BAND_UMTS2100_ONLY (triband GSM + single band UMTS)<br>
*						  BAND_GSM900_DSC1800 | BAND_PCS1900_GSM850 | BAND_UMTS2100_ONLY (quadband GSM + single band UMTS)<br>
*						(if RAT is GSM_ONLY, the UMTS band setting will be ignored;
*						 if RAT is UMTS_ONLY, the GSM band setting will be ignored)
*						or
*						BAND_NULL
*						(this resumes supported band setting to system setting from sysparm)
*	@return	RESULT_OK if the RATs and bands specified is allowed by sysparm and
*						they are consistent with each other
*			RESULT_ERROR otherwise
*
*	@note	The settings specified by this function won't be in effect until
*			SYS_ProcessPowerUpReq() (in power-off state) or 
*			SYS_SelectBand(BAND_AUTO) (in power-on state) is called.
*/
Result_t		MS_SetSupportedRATandBand(RATSelect_t RAT_cap, BandSelect_t band_cap);

//******************************************************************************
/**
* This function returns the current RAT setting supported in platform.
*
*/
RATSelect_t		MS_GetSupportedRAT(void);

//******************************************************************************
/** 
* This function returns the current band setting supported in platform.
*
*/
BandSelect_t	MS_GetSupportedBand(void);

const char* PLMN_GetCountryByMcc(UInt16 mcc);

UInt16 MS_GetPLMNListSize(void);

Boolean MS_GetPLMNEntryByIndex(
					UInt16	index,
					Boolean	ucs2,
					UInt16	*plmn_mcc,
					UInt16	*plmn_mnc,
				 	PLMN_NAME_t *long_name, 
					PLMN_NAME_t *short_name );

Boolean	MS_GetCountryInitialByMcc(UInt16 mcc, const char** country_initial);

Boolean MS_GetPLMNByCode(
					Boolean ucs2,
					UInt16	plmn_mcc,
					UInt16	plmn_mnc,
					PLMN_NAME_t *long_name, 
					PLMN_NAME_t *short_name,
					const char	**country_name ); 

Boolean MS_GetPLMNCodeByName(
					Boolean		ucs2,
					Boolean		long_format,
					const char	*plmn_name,
				 	UInt16		*plmn_mcc,
					UInt16		*plmn_mnc );

Result_t			MS_PlmnSelect(
						UInt8 clientID, 
						Boolean ucs2,
						PlmnSelectMode_t selectMode, 
						PlmnSelectFormat_t format, 
						char *plmnValue, 
						Boolean *netReqSent
						);

Result_t			MS_AbortPlmnSelect(void);

//************************************************************************************
/**
* This function get the current PLMN select mode. 
*
*/
PlmnSelectMode_t	MS_GetPlmnMode(void);

//************************************************************************************
/** 
* This function set the current PLMN select mode. 
*
*/
void				MS_SetPlmnMode(PlmnSelectMode_t mode);

//************************************************************************************
/** 
* This function get the current PLMN select format. 
*
*/
PlmnSelectFormat_t	MS_GetPlmnFormat(void);

//************************************************************************************
/** 
* This function set the current PLMN select format. 
*
*/
void				MS_SetPlmnFormat(PlmnSelectFormat_t format);



//****************************************************************************
/**
 * This function converts the passed PLMN name to a string to be output to
 * ATC/MMI. 
 * 
 * -# If the PLMN name string is ASCII encoded, the returned string is ASCII 
 *	  encoded and NULL terminated. 
 *
 * -# If the PLMN name string is UCS2 encoded, the returned string is Hex 
 *	  string encoded. 
 *
 * @note the returned string is dynamically allocated. The caller of this 
 *		 function needs to de-allocate the memory if the returned string is 
 *		 not NULL.
 *
 */
char*				MS_ConvertPLMNNameStr(Boolean ucs2, PLMN_NAME_t *plmn_name);

//****************************************************************************
/**
 * This function determines if the passed MCC-MNC matches the MCC-MNC returned
 * from the network. 
 *
 * @note the passed MCC-MNC supports the use of "wild-carding" in MCC and MNC: 
 *       if a digit is set to Hex 'D', it matches any digit value in network
 *		 MCC-MNC.
 *
 */
Boolean				MS_IsMatchedPLMN(UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc);

//****************************************************************************
/**
* restarts PLMN Search
*
*/
void				MS_SearchAvailablePLMN(void);

//****************************************************************************
/**
* abort searching available PLMN
*
*/
void				MS_AbortSearchPLMN(void);

//*****************************************************************************
/**
* Get the network names (long and short) based upon the
* MCC/MNC/LAC tuple. The order of priority is as follows:
*	-# EONS (based on MCC, MNC, LAC and EF-OPL & EF-PNN in EONS-enabled SIM
*	-# CPHS ONS & ONSS (based on EF-ONS & EF-ONSS in CPHS-enabled SIM)
*	-# Internal MCC/MNC lookup table (based on SE.13, NAPRD10 and carrier requirements)
*
* @param mcc registered MCC in raw format, e.g. 0x13F0 for Cingular Wireless
* @param mnc registered MNC in raw format, e.g. 0x71 for Cingular Wireless
* @param lac registered LAC in raw format
* @param ucs2 TRUE if UCS2 format of long name and short name is perferred
* @param long_name Pointer to the buffer to store the PLMN long name
* @param short_name Pointer to the buffer to store the PLMN short name
*
* @return Boolean TRUE if a match is found; FALSE otherwise.
*
*/
Boolean				MS_GetPLMNNameByCode(	UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2,
											PLMN_NAME_t *long_name, PLMN_NAME_t *short_name );

//******************************************************************************
/**
* This function decodes the Network Name 
* (long name or short name) IE defined in Section 
* 10.5.3.5a of GSM 04.08.
*
* @param ptr point to the length field of the Network Name IE TLV 
*				(the byte after Network Name IEI).
* @param mcc MCC of the registered PLMN in raw format (i.e. 13F0 for USA).
* @param plmn_name Buffer to store the returned PLMN name. 
*
*/
void				MS_DecodeNetworkName(UInt8 *ptr, UInt16 mcc, PLMN_NAME_t *plmn_name); 

//*****************************************************************************
/**
* This function closes the Equivalent PLMN and Network Parameter
* files since we do not close them when calling MS_ReadEquivPlmnList(),
* MS_WriteEquivPlmnList(), MS_ReadNetParamData() & MS_WriteNetPar() functions. 
*
*/
void				MN_CloseFile(void);

//******************************************************************************
/**
* This function requests an immediate auto network search.
*
*/
void				MS_AutoSearchReq(void);

//*****************************************************************************
/**
* This function reads the last registered RAT from NVRAM/file system.
*
*/
void				MS_ReadLastRegisteredRAT(UInt8* rat);

//****************************************************************************
/**
* This function writes the last registered RAT to NVRAM/file system.
*
*/
void				MS_WriteLastRegisteredRAT(UInt8 rat);

/**
* This function performs a PLMN detach-then-reattach procedure for STK Refresh.
* It should be called if the Refresh type is "Init With Full File Change 
* Notification" because the stack does not perform an IMSI detach-then-reattach
* procedure for the new IMSI. 
*
* @return Boolean TRUE if re-attach procedure is initiated; FALSE otherwise. 
*
*/
Boolean				MS_RefreshReAttach(void);

void				SYS_ProcessReset(void);
void				SYS_ProcessPowerDownReq(void);
void				SYS_ProcessNoRfReq(void);
void				SYS_ProcessPowerUpReq(void);
PowerOnCause_t		SYS_GetMSPowerOnCause(void);
void				SYS_SetMSPowerOnCause(UInt8);
void				SYS_SetSoftResetCause(void);
Boolean				SYS_IsResetCausedByAssert(void);
void				MsDatabase_Init(void);


SystemState_t		SYS_GetSystemState(void);
Result_t			SYS_SetSystemState(SystemState_t state);

void				SYS_GetRxSignalInfo(UInt8 *RxLev, UInt8 *RxQual);
void				SYS_SetRxSignalInfo(UInt8 RxLev, Boolean RxQualValid, UInt8 RxQual);


//GSM & GPRS status

RegisterStatus_t	SYS_GetGSMRegistrationStatus(void);
RegisterStatus_t	SYS_GetGPRSRegistrationStatus(void);
void				SYS_SetGSMRegistrationStatus(RegisterStatus_t status);
void				SYS_SetGPRSRegistrationStatus(RegisterStatus_t status);
Boolean				SYS_IsRegisteredGSMOrGPRS(void);

void				SYS_SetGSMRegistrationCause( PCHRejectCause_t cause );
void				SYS_SetGPRSRegistrationCause( PCHRejectCause_t cause );
PCHRejectCause_t	SYS_GetGSMRegistrationCause(void);
PCHRejectCause_t	SYS_GetGPRSRegistrationCause(void);

void				MS_SetGSMRegCause( NetworkCause_t cause );
void				MS_SetGPRSRegCause( NetworkCause_t cause );


//LAC
void				MS_SetRegisteredLAC(LACode_t lac);

//Cell Info
void				SYS_SetRegisteredCellInfo(CellInfo_t cell_info);

//MCC & MNC
Boolean				MS_IsPlmnForbidden(void);
void				MS_SetPlmnIsForbidden(Boolean is_forbidden);

void				MS_SetPlmnMCC(UInt16 mcc);
void				MS_SetPlmnMNC(UInt8 mnc);

void				MS_SetCurrentPlmn(PLMNId_t curr_plmn);

UInt8				MS_GetDefaultSMSParamRecNum(void);
void				MS_SetDefaultSMSParamRecNum(UInt8 rec_num);

UInt16				MS_PlmnConvertRawMcc(UInt16 mcc);						//Internal Use
UInt16				MS_PlmnConvertRawMnc(UInt16 mcc, UInt16 mnc);			//Internal Use
void				MS_ConvertRawPlmnToHer(UInt16 mcc, UInt16 mnc, UInt8 *plmn_str, UInt8 plmn_str_sz);
Boolean				MS_ConvertStringToRawPlmnId(UInt8 *oper, PLMNId_t *plmn_id);
void				MS_ConvertMccMncToRawPlmnid(UInt16 mcc, UInt16 mnc, PLMNId_t *plmn_id);
Boolean				MS_IsRegisteredHomePLMN(void);

Result_t			MS_SetPowerDownTimer(UInt8 powerDownTimer);
void				MS_SetAttachMode(UInt8 mode);
UInt8				MS_GetAttachMode(void);


//=========================================================================
// MS Internal Functions
//=========================================================================
Boolean				MS_IsGprsCallActive(UInt8 chan);
void				MS_SetChanGprsCallActive(UInt8 chan, Boolean active);
void				MS_SetCidForGprsActiveChan(UInt8 chan, UInt8 cid);
UInt8				MS_GetGprsActiveChanFromCid(UInt8 cid);
UInt8				MS_GetCidFromGprsActiveChan(UInt8 chan);
void				MS_StatisticInfoReport(void);


void				SetPowerUpRegStatus(Boolean powerUpReq);
Boolean				IsPowerUpRegStatus(void);
Boolean				IsPowerDownInProgress(void);			// TRUE: at+cfun=0 power down is in progress
void				stopPowerDownDetachTimer(void);
void				startPowerDownDetachTimer(void);
void				startPowerDownStorageTimer(void);

extern	UInt16	pppDropDL;
extern	UInt16	pppDropUL;
extern	UInt16	pppDLPacket;
extern	UInt16	ifpppDropDL;

//MsConfig_t*			MS_GetCfg( void ) ;

void				MS_SetProductionTestMode(UInt8 mode);
UInt8				MS_GetProductionTestMode(void);

void				MS_SetInvalidSimMsMe(Boolean invalid_sim_ms_me);
Boolean				MS_GetInvalidSimMsMe(void);

void				MS_SetPowerDownToReset(Boolean is_soft_reset);
Boolean				MS_GetPowerDownToReset(void);

typedef void (*ReturnValue_t)(			// Callback to return BADR value
	UInt16 badr_value					// Returned BADR value
	);

void ADCMGR_Start(                      // Start ADC measurement
    UInt16 init_value,                  // BMR value to start measurement
    ReturnValue_t return_value_cb       // Callback function to return BADR value
    );                                   // Return: TRUE, if request submission succeeded

Result_t AT_ProcessCmd( 
	UInt8			ch, 
	const UInt8*	atCmdStr );


RATSelect_t		MS_GetSystemRAT(void);

RATSelect_t		MS_GetSupportedRAT(void);

BandSelect_t	MS_GetSystemBand(void);


BandSelect_t	MS_GetSupportedBand(void);

void MS_GetRegistrationInfo(MSRegStateInfo_t *reg_info);
UInt16 SYSPARM_GetMSClass( void );
UInt8 AUDIO_GetMicrophoneGainSetting (void);
Result_t AUDIO_SetSpeakerVol(UInt8 vol);
UInt8 AUDIO_GetSpeakerVol(void);
Result_t AUDIO_SetMicrophoneGain (UInt8 gain);
UInt8 *SYSPARM_GetManufacturerName(void);
UInt8 *SYSPARM_GetModelName(void);
UInt8 *SYSPARM_GetSWVersion(void);
void UTIL_ExtractImei(UInt8* inImeiStrPtr);

//Boolean SYS_SetFilteredEventMask(UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);
//Boolean SYS_SetRegisteredEventMask(UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);
UInt16 SYSPARM_GetEGPRSMSClass( void );
void SYS_SetRssiThreshold(UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);


UInt16	SYSPARM_GetSysparmIndPartFileVersion(void);
void 			SYS_GetBootLoaderVersion(UInt8* BootLoaderVersion, UInt8 BootLoaderVersion_sz);
void			SYS_GetDSFVersion(UInt8* DSFVersion, UInt8 DSFVersion_sz);
UInt16 SYSPARM_GetChanMode( void );		// Return the MS channel mode
#define SYSPARM_UMTS_BAND_LIST_LENGTH	10

void TIMEZONE_DeleteNetworkName(void);

void TIMEZONE_UpdateRTC(Boolean updateFlag);

TimeZoneUpdateMode_t TIMEZONE_GetTZUpdateMode(void);

void TIMEZONE_SetTZUpdateMode(TimeZoneUpdateMode_t mode);

typedef Int16 (AtRespCallbackFunc_t) (
									  UInt8 inChnlID, ///< The internal channel ID that is return by AT_RegisterInterChannel.
									  UInt8* inBuff,  ///< The buffer contains the Tx data.
									  Int16 inBuffLen, ///< The length of the Tx data buffer.
									  Boolean isUnsolicited ///< The length of the Tx data buffer.
									  );


/** Register an internal AT channel. The caller can use the returned internal channel ID to call AT_ProcessCmd()
* for processing an AT command string.
 */
UInt8 AT_RegisterInterChannel(
							  AtRespCallbackFunc_t * inAtRespFunc ///< The internal channel callback function provided by the caller.
							  );

Int16 Capi2GetAtChannel(void);

Result_t MS_SetElement(	UInt8			inClientId,
						MS_Element_t	inElemType,
						void*			inElemPtr);

Result_t MS_GetElement(	UInt8			inClientId,
						MS_Element_t	inElemType,
						void*			inElemPtr);

Boolean MS_IsDeRegisterInProgress(void);
Boolean MS_IsRegisterInProgress(void);
Result_t MS_SetPlmnSelectRat(UInt8 manual_rat);


void HAL_EM_BATTMGR_BattChargingNotification(HAL_EM_BATTMGR_Charger_t chargeType, HAL_EM_BATTMGR_Charger_InOut_t inOut, UInt8 status);
void HAL_EM_BATTMGR_BattADCReq(void);
HAL_EM_BATTMGR_Result_en_t HAL_EM_BATTMGR_RegisterEventCB(
	HAL_EM_BATTMGR_Event_en_t event,			///< (in) Event type
	void *callback								///< (in) Callback routine
	);

//Result_t MS_InitCallCfgAmpF( UInt8 inClientId ); 
//Result_t MS_InitVideoCallCfg( UInt8 inClientId ); 
//Result_t MS_InitFaxConfig( UInt8 inClientId );
//Result_t MS_InitCallCfg( UInt8 inClientId );

UInt8* PATCH_GetRevision( void );
void DIAG_ApiMeasurmentReportReq(Boolean inPeriodicReport, UInt32 inTimeInterval);

//Audio related
void VOLUMECTRL_SetBasebandVolume(		
   	UInt8 level,
   	UInt8 chnl,
	UInt16 *audio_atten,
	UInt8 extid
);
void RIPCMDQ_Connect_Downlink(
	Boolean Downlink
	);
void RIPCMDQ_Connect_Uplink(
	Boolean	Uplink
	);
void ECHO_SetDigitalTxGain(
	Int16 digital_gain_step
	);

void AUDIO_Turn_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off);

void SPEAKER_StopTone( void );			// Stop a supervisory tone
void SPEAKER_StartGenericTone(
	Boolean	superimpose,		// flag to enable superimposed tone
	UInt16	tone_duration,		// tone duration in ms
	UInt16	f1,					// first frequency
	UInt16	f2,					// second frequency, if f2==0 not frequency component output
	UInt16	f3					// third frequency,	 if f3==0 not frequency component output
	);
void SPEAKER_StartTone(					// Generate a supervisory tone
	SpeakerTone_t tone,					// Selected tone
	UInt8 duration						// Tone duration (MS)
	);	 

void AUDIO_ASIC_SetAudioMode(AudioMode_t audioMode);

#include "capi2_lcs_ds.h"
#include "capi2_sms_ds.h"
#include "capi2_stk_ds.h"
//LCS API
void LCS_CmdData(CAPI2_LcsCmdData_t inCmdData);
extern void LCS_SuplDisconnected(LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl);
extern LcsResult_t LCS_SuplConnectRsp(LcsSuplSessionHdl_t inSessionHdl, LcsSuplConnectHdl_t inConnectHdl);
extern void LCS_SuplVerificationRsp(LcsSuplSessionHdl_t inSessionHdl, Boolean inIsAllowed);
extern UInt32 LCS_ConfigGet(UInt32 configId);
extern Result_t LCS_ConfigSet(UInt32 configId, UInt32 value);
extern LcsResult_t LCS_StopPosReq(LcsHandle_t inLcsHandle);
extern LcsServiceType_t LCS_ServiceQuery(void);
extern LcsResult_t LCS_ServiceControl(LcsServiceType_t inServiceType);


//SMS
UInt16 SMS_GetSmsMaxCapacity(SmsStorage_t storageType);
Result_t SMS_ConfigureMEStorage(Boolean flag);
void SMS_SendMemAvailInd(void);

//SIM
SIMServiceStatus_t USIM_GetServiceStatus(SIMService_t service);

Result_t MS_SetMEPowerClass(UInt8 band, UInt8 pwrClass);
MSRegState_t		MS_GetGSMRegState(void);
MSRegState_t		MS_GetGPRSRegState(void);

//PBK
Boolean PBK_GetFdnCheck(void);
void PBK_SetFdnCheck(Boolean fdn_chk_on);


//STK
void SATK_SendSetupCallRes(SATK_ResultCode_t result);
void SATK_SendPlayToneRes(SATK_ResultCode_t result);

typedef enum
{
	EM_BATTMGR_ACTION_RUN_BATTMGR,					///< Run battmgr. Replacement for BATTMGR_Run
	EM_BATTMGR_ACTION_CONFIG_BATTMGR,				///< Configure battery mgr's battery monitor with values from sysparms. 
													///< Replacement for BATTMGR_Config
	EM_BATTMGR_ACTION_GET_BATTLEVEL_PERCENT,		///< Get battery level in percentage 
	EM_BATTMGR_ACTION_GET_BATTLEVEL,				///< Get battery level in discrete units 
	EM_BATTMGR_ACTION_IS_USB_CHARGER_PRESENT,		///< Is USB cable present?  
	EM_BATTMGR_ACTION_IS_WALL_CHARGER_PRESENT,		///< Is Wall charger present? 
	EM_BATTMGR_ACTION_GET_BATT_TEMP,				///< Get current reading of battery temperature
	EM_BATTMGR_ACTION_GET_CHARGING_STATUS,			///< Get charging status
  	EM_BATTMGR_ACTION_MAX_ACTIONS					///< Max supported action
} HAL_EM_BATTMGR_Action_en_t;

typedef struct 
{
	UInt16 battLevel;						   		///< (out) battery level
} HAL_EM_BATTMGR_Action_BattLevel_st_t;	   	
