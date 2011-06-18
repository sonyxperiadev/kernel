//***************************************************************************
//
//	Copyright © 2009 Broadcom Corporation
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
*   @file   netreg_api_old.h
*
*   @brief  This file defines the interface for OLD Network Registration related API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _NETREG_API_OLD_H_
#define _NETREG_API_OLD_H_
//==================================OLD NETREG API PROTOTYPES ==================================//
// ALL THE API PROTOTYPES BELOW ARE THE OLD API PROTOTYPES AND ARE PRESENT FOR BACKWARD COMPATIBILITY //
/** @} */

/**
*   @defgroup   OldNetRegAPIGroup   Legacy Network Registration API
*   @ingroup    NetRegAPIGroup
*   @brief      This group is the legacy API prototypes and are present for backwards compatibility

*	Using the following functions, the user can	register, deregister, select a PLMN, band etc 
*
	Test code testing these API's can be found in at_plmn.c\n

	Click here to return to the Network Registration overview: \ref NetRegOverview
****************************************************************************/

/**
 * @addtogroup OldNetRegAPIGroup
 * @{
 */

void				PrintPCHDebugInfo(UInt8 *inString);

//*****************************************************************************
/**
* This function closes the Equivalent PLMN and Network Parameter
* files since we do not close them when calling MS_ReadEquivPlmnList(),
* MS_WriteEquivPlmnList(), MS_ReadNetParamData() & MS_WriteNetPar() functions. 
*
*/
void				MN_CloseFile(void);

//****************************************************************************
/**
* This function writes the last registered RAT to NVRAM/file system.
*
*/
void	MS_WriteLastRegisteredRAT(Rat_t rat);
/**
* This function reads the last registered RAT from NVRAM/file system.
*
*/
void				MS_ReadLastRegisteredRAT(Rat_t * rat);

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param		clientID (in) Client ID
	@param		SIMPresent (in) is SIM present
	@param		SIMType (in) SIM type
	@param		regType (in) registration type, gsm,gprs or both
	@param		plmn (in) plmn (mcc, mnc)
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		MS_SendCombinedAttachReq(UInt8 clientID,Boolean SIMPresent,SIMType_t SIMType,RegType_t regType, PLMN_t plmn);

//***************************************************************************************
/**
	Function to send request for gprs detach
	@param		clientID (in) Client ID
	@param		cause (in) cause of deregistration
	@param		regType (in) option to select gsm,gprs or both
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		MS_SendDetachReq(UInt8 clientID, DeRegCause_t cause, RegType_t regType);

//***************************************************************************************
/**
	Function to get gprs attach state
	@return		AttachState_t attach state
	@note
**/
AttachState_t	MS_GetGPRSAttachStatus(void);


//***************************************************************************************
/**
	Function to check if deregistration/detach is in progress
	@return		Boolean: TRUE or FALSE
	@note
**/
Boolean			MS_IsDeRegisterInProgress(void);

//***************************************************************************************
/**
	Function to check if registration/attach is in progress
	@return		Boolean: TRUE or FALSE
	@note
**/
Boolean			MS_IsRegisterInProgress(void);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param		forcedReadyState (in) force ready state set to true/false
	@return		void
	@note
**/
void			MS_ForcedReadyStateReq(Boolean forcedReadyState);

//***************************************************************************************
/**
	Client can use this API to release the PS immeidately in case of fast dormancy
	@return		void
	@note
**/
void			MS_ForcePsReleaseReq(void);

//***************************************************************************************
/**
	Function to set MS class
	@param		msClass (in) MS Class type
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		PDP_SetMSClass(MSClass_t msClass);

//***************************************************************************************
/**
	Function to get MS class
	@return		MSClass_t: MS Class type
	@note
**/
MSClass_t		PDP_GetMSClass(void);

//==========================PLMN SELECTION PART =========================================//
//***********************************************************************
/**
* This function perform the selection of PLMN network. 
*
*/
Result_t			MS_PlmnSelect(
						UInt8 clientID, 
						Boolean ucs2,
						PlmnSelectMode_t selectMode, 
						PlmnSelectFormat_t format, 
						char *plmnValue, 
						Boolean *netReqSent
						);

//************************************************************************************
/**
*  This function performs either: the abortion of current manual PLMN selection; revert back to 
*  previous PLMN selection mode before manual PLMN selection (typically used after manual PLMN selection fails). 
*
*	@return		RESULT_OK: <br>
*				RESULT_ERROR: invalid newSelectMode	 
*
*/
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

//******************************************************************************
/** 
* This function returns index for manual registration if it is found
* 
*/
UInt8				MS_FindPlmnForManualSel(Boolean ucs2, UInt8 *oper, PlmnSelectFormat_t format, PLMN_t *plmn, UInt8 *rat);

//************************************************************************************
/** 
* This function check if the plmn index matches the registered PLMN. 
*
*/
Boolean				MS_IsRegisteredPLMN(PLMN_t plmn);

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
*	-# CPHS ONS & ONSS (based on EF-ONS & EF-ONSS in CPHS-enabled 2G SIM)
*	-# NITZ network name in MM/GMM message received from network
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
* This function requests an immediate auto network search.
*
*/
void				MS_AutoSearchReq(void);


/**
	This function sets the RAT used in Manual PLMN Selection through MS_PlmnSelect(). 

	@param		manual_rat (in) RAT_NOT_AVAILABLE (no RAT specified); RAT_GSM; RAT_UMTS

	@return		RESULT_OK if RAT setting is OK; RESULT_ERROR otherwise (i.e. RAT not supported)
**/
Result_t MS_SetPlmnSelectRat(UInt8 manual_rat);

/**
	This function returns the RAT used in Manual PLMN Selection through MS_PlmnSelect().

	@return		RAT_NOT_AVAILABLE (no RAT specified); RAT_GSM; RAT_UMTS
**/
UInt8 MS_GetPlmnSelectRat(void);

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
* This function returns the system RAT setting supported in platform which is defined
* in System Parameter file.
*
*/
RATSelect_t		MS_GetSystemRAT(void);

//******************************************************************************
/**
* This function returns the current RAT setting supported in platform. The supported
* RAT is the same as system RAT by default upon powerup, but supported RAT can be
* changed to be a subset of system RAT through MS_SetSupportedRATandBand().
*
*/
RATSelect_t		MS_GetSupportedRAT(void);

//******************************************************************************
/**
* This function returns the system band setting supported in platform which is defined
* in System Parameter file.
*
*/
BandSelect_t	MS_GetSystemBand(void);


//******************************************************************************
/**
* This function returns the current band setting supported in platform. The supported
* band is the same as system band by default upon powerup, but supported band can be
* changed to be a subset of system band through MS_SetSupportedRATandBand().
*
*/
BandSelect_t	MS_GetSupportedBand(void);

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

//-----------------------------------------------------------
// MS_SetMEPowerClass():	set the preferred ME power class for 
//						each GSM frequency band supported. 
//----------------------------------------------------------
Result_t MS_SetMEPowerClass(UInt8 band, UInt8 pwrClass);

//======================Registration query API===================================//
//***************************************************************************************
/**
	Function to check if MS is GSM registered.
	@return		 Boolean TRUE if MS is registered.
	@note This function is used to check if the MS has camped on a cell and thereby ready
	to perform normal GSM functions.
	Note: Please use the MS_GetElement() API to check if gsm is registered
		  instead of this API. This API will be obsolete soon.
**/

Boolean				MS_IsGSMRegistered(void);

//***************************************************************************************
/**
	Function to check if MS is GPRS registered.
	@return		 Boolean TRUE if MS is GPRS registered.
	@note This function is used to check if the MS is GPRS attached. If true, the MS is ready 
	to perform normal GPRS operations.
 	Note: Please use the MS_GetElement() API to check if GPRS is registered
 		  instead of this API. This API will be obsolete soon.
**/

Boolean				MS_IsGPRSRegistered(void);


//***************************************************************************************
/**
	Function to get the GSM registration state of the MS
	@return		 MSRegState_t 
	@note 	This function is used to check the GSM registration state of the MS.
 	Note: Please use the MS_GetElement() API to retrieve the GSM RegState
 		  instead of this API. This API will be obsolete soon.
**/

MSRegState_t		MS_GetGSMRegState(void);

//***************************************************************************************
/**
	Function to get the GPRS registration state of the MS
	@return		 MSRegState_t 
	@note 	This function is used to check the GPRS registration state of the MS.
 	Note: Please use the MS_GetElement() API to retrieve the GPRS RegState
 		  instead of this API. This API will be obsolete soon.
**/

MSRegState_t		MS_GetGPRSRegState(void);

//***************************************************************************************
/**
	Function to get the GSM registration reject cause generated locally
	@return		 NetworkCause_t 
	@note 	This function returns the GSM registration reject cause generated locally.
 	Note: Please use the MS_GetElement() API to retrieve the GSM RegCause
 		  instead of this API. This API will be obsolete soon.
**/

NetworkCause_t	MS_GetGSMRegCause(void);

//***************************************************************************************
/**
	Function to get the GPRS registration reject cause generated locally
	@return		 NetworkCause_t 
	@note This function returns the GPRS registration reject cause generated locally.
 	Note: Please use the MS_GetElement() API to retrieve the GPRS RegCause
 		  instead of this API. This API will be obsolete soon.
**/

NetworkCause_t	MS_GetGPRSRegCause(void);

//***************************************************************************************
/**
	Function to get the GSM registration reject cause from network
	@return		 UInt8 
	@note This function returns the GSM registration reject cause from network.
 	Note: Please use the MS_GetElement() API to retrieve the GSM RegNetwork Cause
 		  instead of this API. This API will be obsolete soon.
**/

UInt8	MS_GetGSMRegNetworkCause(void);

//***************************************************************************************
/**
	Function to get the GPRS registration reject cause from network
	@return		 UInt8 
	@note This function returns the GPRS registration reject cause from network.
 	Note: Please use the MS_GetElement() API to retrieve the GPRS RegNetwork Cause
 		  instead of this API. This API will be obsolete soon.
**/

UInt8	MS_GetGPRSRegNetworkCause(void);

//***************************************************************************************
/**
	Function to get the MS' registration Location Area Code
	@return		 LACode_t 
 	Note: Please use the MS_GetElement() API to retrieve the Registered LAC
 		  instead of this API. This API will be obsolete soon.
**/

LACode_t	MS_GetRegisteredLAC(void);

//***************************************************************************************
/**
	Function to get the MS' registred cell information
	@return		 CellInfo_t 
	Note: 	Please use the MS_GetElement() API to retrieve the Registered cellinfo
			instead of this API. This API will be obsolete soon.
**/

CellInfo_t	MS_GetRegisteredCellInfo(void);

//***************************************************************************************
/**
	Function to get the MS' registred Mobile Country Code
	@return		 UInt16 The mobile country code of the MS 
	@note	This function returns the mobile country code of the network on which the 
	MS is registered. Once this is done, we need to use MS_PlmnConvertRawMcc() to retrieve 
	the MCC is correct format. e.g raw format: 0x1300 correct format: 0x310
**/

UInt16	MS_GetPlmnMCC(void);

//***************************************************************************************
/**
	Function to get the MS' registred Mobile Network Code
	@return		 UInt16 The mobile network code of the MS 
	@note	This function returns the mobile network code of the network on which the 
	MS is registered.Once this is done, we need to use MS_PlmnConvertRawMnc() to 
	retrieve the MCC is correct format.for example: the passed MCC is 0x1300 and passed MNC 
	is 0x0071 for Pacific Bell. This function returns the converted MNC value as 0x170
**/

UInt8	MS_GetPlmnMNC(void);

//***************************************************************************************
/**
	This is a utility to check if GPRS is allowed
	@return		 Boolean TRUE if GPRS is allowed
	@note	
	This function is used to check if GPRS is allowed. If the MS is a class B 
	device, GPRS should be allowed during GSM voice or data call.
**/

Boolean	MS_IsGprsAllowed(void);


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

UInt8	MS_GetCurrentRAT(void);

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

UInt8	MS_GetCurrentBand(void);


//***************************************************************************************
/**
	Function to return current registration information including GSM/GPRS registration state, 
	registered MCC, MNC, LAC, Cell_ID, RAT and band.

	@return		 MSRegStateInfo_t
 	Note: Please use the MS_GetElement() API to retrieve the RegistrationInfo
 		  instead of this API. This API will be obsolete soon.
**/
void	MS_GetRegistrationInfo(MSRegStateInfo_t *reg_info);

//**************************************************************************************
/**
	This function returns the GSM registration state
	@return		RegisterStatus_t
**/
RegisterStatus_t	SYS_GetGSMRegistrationStatus(void);

//**************************************************************************************
/**
	This function returns the GPRS registration state
	@return		RegisterStatus_t
**/
RegisterStatus_t	SYS_GetGPRSRegistrationStatus(void);

//**************************************************************************************
/**
	This function returns TRUE if we are registered to GPRS or GSM service.
	@return		Boolean
**/
Boolean	SYS_IsRegisteredGSMOrGPRS(void);

Boolean	MS_IsRegisteredHomePLMN(void);

Result_t SYS_CellLock(Boolean Enable, BandSelect_t lockband, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);

GANStatus_t MS_GetPrefNetStatus(void);


/** @} */
/** 
	@addtogroup TimeZoneGroup
	@{
*/

//***************************************************************************************
/**
    Function to update real time clock
	@param		updateFlag (in) Boolean value required to confirm RTC update
	@note
	This function is used by the client to confirm/denny update of real time clock.
**/	

void TIMEZONE_UpdateRTC(Boolean updateFlag);

//***************************************************************************************
/**
    Function to get time zone update mode
	@return		TimeZoneUpdateMode_t 
	@note
	Possible update modes are manual, auto, user_confirm, none.
**/	

TimeZoneUpdateMode_t TIMEZONE_GetTZUpdateMode(void);

//***************************************************************************************
/**
    Function to set time zone update mode
	@param		mode (in) TimeZoneUpdateMode_t 
**/	

void TIMEZONE_SetTZUpdateMode(TimeZoneUpdateMode_t mode);



void TIMEZONE_GetNetworkName(Boolean isLongName, PLMN_NAME_t *netName);

void TIMEZONE_DeleteNetworkName(void);
/** @} */


#endif  // _NETREG_API_OLD_H
