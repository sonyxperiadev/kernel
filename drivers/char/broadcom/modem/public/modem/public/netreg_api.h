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
*   @file   netreg_api.h
*   @brief  This file defines the interface for Network Registration related API functions.
*
****************************************************************************/

/**
*   @defgroup   NetRegAPIGroup   Network Registration API
*   @ingroup    TelephonyGroup
*
*   @brief      This group defines the interfaces to the NetReg system and provides
*				API documentation.

*	Using the following functions, the user can	register, deregister, select a PLMN, band etc 
*
	Test code testing these API's can be found in at_plmn.c\n

	Click here to return to the Network Registration overview: \ref NetRegOverview
****************************************************************************/
/**
*
*   @defgroup   TimeZoneGroup	Time Zone Group
*   @ingroup    TelephonyGroup				
*
*   @brief      This group contains the Time Zone APIs.
*
	\n Use the link below to navigate back to the PLMN Table and Time Zone page. \n
    \li \ref PLMNTimeZoneOverview
****************************************************************************/

#ifndef _NETREG_API_H_
#define _NETREG_API_H_

/**
 * @addtogroup NetRegAPIGroup
 * @{
 */

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param	clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param	SIMPresent		(in) is SIM present
	@param	SIMType			(in) SIM type
	@param	regType			(in) registration type, gsm,gprs or both
	@param	plmn			(in) plmn (mcc, mnc)
	@return	Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		NetRegApi_SendCombinedAttachReq(ClientInfo_t *clientInfoPtr,Boolean SIMPresent,SIMType_t SIMType,RegType_t regType, PLMN_t plmn);

//harini: simtype_t is defined in sim_def.h, do i need to include that in this file? so should we move simtype_t to sim_commmon.h

//        plmn_t is defined in mmregtypes.h , i don't see others including this file.
//***************************************************************************************
/**
	Function to send request for gprs detach
	@param	clientInfoPtr	(in) pointer to clientinfo of the calling client 
	@param	cause			(in) cause of deregistration
	@param	regType			(in) option to select gsm,gprs or both
	@return	Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		NetRegApi_SendDetachReq(ClientInfo_t *clientInfoPtr, DeRegCause_t cause, RegType_t regType);


//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param	clientInfoPtr	(in) pointer to clientinfo of the calling client 
	@param	forcedReadyState (in) force ready state set to true/false
	@return	void
	@note
**/
void			NetRegApi_ForcedReadyStateReq(ClientInfo_t *clientInfoPtr, Boolean forcedReadyState );

//***************************************************************************************
/**
	Client can use this API to notify stack to immeidately release ps for fast dormancy feature
	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@return		void
	@note
**/
void			NetRegApi_ForcePsReleaseReq(ClientInfo_t *clientInfoPtr);

//***************************************************************************************
/**
	Function to set MS class
	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		msClass (in) MS Class type
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
	@note
**/
Result_t		NetRegApi_SetMSClass(ClientInfo_t *clientInfoPtr, MSClass_t msClass);



//==========================PLMN SELECTION PART =========================================//
//***********************************************************************
/**
* This function perform the selection of PLMN network. 
*
	@param	clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param	ucs2			(in) TRUE if UCS2 format of long name and short name is perferred
	@param	selectMode		(in) Plmn select Mode
	@param	format			(in) Plmn format
	@param  plmnValue		(in) Plmn value
	@param  netReqSent		(out) Net Request sent
	@return		Result_t :		::RESULT_OK, ::RESULT_ERROR
*/
Result_t			NetRegApi_PlmnSelect(
						ClientInfo_t		*clientInfoPtr, 
						Boolean 			ucs2,
						PlmnSelectMode_t 	selectMode, 
						PlmnSelectFormat_t 	format, 
						char 				*plmnValue, 
						Boolean 			*netReqSent
						);

//************************************************************************************
/**
*  This function performs either: the abortion of current manual PLMN selection; revert back to 
*  previous PLMN selection mode before manual PLMN selection (typically used after manual PLMN selection fails). 
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*	@return		RESULT_OK: <br>
*				RESULT_ERROR: invalid newSelectMode	 
*
*/
Result_t			NetRegApi_AbortPlmnSelect(ClientInfo_t		*clientInfoPtr);

//************************************************************************************
/**
* This function get the current PLMN select mode. 
*
*/	//this is already present in msdatabase, clients should query that ms database element that corresponds to this.
//PlmnSelectMode_t	MS_GetPlmnMode(void);

//************************************************************************************
/** 
* This function set the current PLMN select mode. 
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*	@param		mode mode
*
*/
void				NetRegApi_SetPlmnMode(ClientInfo_t	*clientInfoPtr, PlmnSelectMode_t mode);

//************************************************************************************


//************************************************************************************
/** 
* This function set the current PLMN select format. 
*
*/
//this is already present in msdatabase, clients should query that ms database element that corresponds to this.
//void				MS_SetPlmnFormat(PlmnSelectFormat_t format);

//******************************************************************************
/** 
* This function returns index for manual registration if it is found
* 
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		ucs2			(in) TRUE if UCS2 format of long name and short name is perferred
	@param		oper			(in) operator name
	@param		format			(in) PLMN select format
	@param		plmn			(in) PLMN
	@param		rat				(out) rat
	@return		index for manual registration if it is found
*/
UInt8				NetRegApi_FindPlmnForManualSel
						(	ClientInfo_t	*clientInfoPtr, 
							Boolean ucs2, 
							UInt8 *oper, 
							PlmnSelectFormat_t format, 
							PLMN_t *plmn, 
							UInt8 *rat);

//************************************************************************************
/** 
* This function check if the plmn index matches the registered PLMN. 
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		plmn			(in) PLMN
*/
Boolean		NetRegApi_IsRegisteredPLMN(ClientInfo_t *clientInfoPtr, PLMN_t plmn);



//****************************************************************************
/**
* restarts PLMN Search
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*/
void				NetRegApi_SearchAvailablePLMN(ClientInfo_t *clientInfoPtr);

//****************************************************************************
/**
* abort searching available PLMN
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*/
void				NetRegApi_AbortSearchPLMN(ClientInfo_t *clientInfoPtr);

//*****************************************************************************
/**
* Get the network names (long and short) based upon the
* MCC/MNC/LAC tuple. The order of priority is as follows:
*	-# EONS (based on MCC, MNC, LAC and EF-OPL & EF-PNN in EONS-enabled SIM
*	-# CPHS ONS & ONSS (based on EF-ONS & EF-ONSS in CPHS-enabled 2G SIM)
*	-# NITZ network name in MM/GMM message received from network
*	-# Internal MCC/MNC lookup table (based on SE.13, NAPRD10 and carrier requirements)
*
* @param clientInfoPtr	(in) pointer to clientInfo of the calling client
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
Boolean				NetRegApi_GetPLMNNameByCode(	ClientInfo_t	*clientInfoPtr, 
													UInt16 			mcc, 
													UInt8 			mnc, 
													UInt16 			lac, 
													Boolean 		ucs2,
													PLMN_NAME_t 	*long_name, 

													PLMN_NAME_t 	*short_name );



//******************************************************************************
/**
* This function requests an immediate auto network search.
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*/
void				NetRegApi_AutoSearchReq(ClientInfo_t *clientInfoPtr);

/**
	This function sets the RAT used in Manual PLMN Selection through MS_PlmnSelect(). 

	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		manual_rat		(in) RAT_NOT_AVAILABLE (no RAT specified); RAT_GSM; RAT_UMTS

	@return		RESULT_OK if RAT setting is OK; RESULT_ERROR otherwise (i.e. RAT not supported)
**/
Result_t 			NetRegApi_SetPlmnSelectRat(ClientInfo_t *clientInfoPtr,UInt8 manual_rat);

//========================BAND SELECTION PART==============================================//

//******************************************************************************
/**
* Set band to 900/1800/1900/Dual
*
*	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
*   @param		bandSelect	    (in) band selection
*	@note This function shall be called in power-up mode only, i.e. not in off or flight mode.
*/
Result_t			NetRegApi_SelectBand( ClientInfo_t *clientInfoPtr, BandSelect_t bandSelect );

//******************************************************************************
/**
* This function sets the RATs (Radio Access Technologies) and bands to be
* supported by platform.
*
*	@param clientInfoPtr pointer to clientInfo of the calling client
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
*   @param RAT_cap_sim2 RAT setting for the 2nd VM
*	@param band_cap_sim2 Band setting for the 2nd VM

*	@return	RESULT_OK if the RATs and bands specified is allowed by sysparm and
*						they are consistent with each other
*			RESULT_ERROR otherwise
*
*	@note	The settings specified by this function won't be in effect until
*			PhoneCtrlApi_ProcessPowerUpReq() (in power-off state) or
*			SYS_SelectBand(BAND_AUTO) (in power-on state) is called.
*/
Result_t	NetRegApi_SetSupportedRATandBandEx( 	ClientInfo_t *clientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap, 
												RATSelect_t RAT_cap_sim2, BandSelect_t band_cap_sim2 );

Result_t	NetRegApi_SetSupportedRATandBand( 	ClientInfo_t *clientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap );


//==================================TIMEZONE=========================================
//***************************************************************************************
/**
    Function to update real time clock
	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		updateFlag		(in) Boolean value required to confirm RTC update
	@note
	This function is used by the client to confirm/denny update of real time clock.
**/	

void 				NetRegApi_UpdateRTC(ClientInfo_t *clientInfoPtr, Boolean updateFlag);

//***************************************************************************************
/**
    Function to get time zone update mode
	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@return		TimeZoneUpdateMode_t 
	@note
	Possible update modes are manual, auto, user_confirm, none.
**/	

TimeZoneUpdateMode_t NetRegApi_GetTZUpdateMode(ClientInfo_t *clientInfoPtr);

//***************************************************************************************
/**
    Function to set time zone update mode
	@param		clientInfoPtr	(in) pointer to clientInfo of the calling client
	@param		mode			(in) TimeZoneUpdateMode_t 
**/	

void 		NetRegApi_SetTZUpdateMode(ClientInfo_t *clientInfoPtr, TimeZoneUpdateMode_t mode);

void 		NetRegApi_GetNetworkName(ClientInfo_t *clientInfoPtr, Boolean isLongName, PLMN_NAME_t *netName);

void		NetRegApi_DeleteNetworkName(ClientInfo_t *clientInfoPtr);

//******************************************************************************
/**
* This function sets the RATs (Radio Access Technologies) and bands to be
* supported by platform.
*
*	@param clientInfoPtr pointer to the clientInfo that initiates this APi request
*	@param Enable		 TRUE: enable Cell lock on a particular frequency,if FALSE then rest of the parameters are ignored
*   @param lockband		 the band on which the UE has to be locked to
*   @param lockrat		 rat on whcih the UE has to be locked to  
*   @param lockuarfcn
*   @param lockpsc 
*	@return	RESULT_OK if CAPI is able to call the stack APIs to lock to the required arfcn etc.
*
*	@note	
*/

Result_t NetRegApi_CellLock(ClientInfo_t *clientInfoPtr, Boolean Enable, BandSelect_t lockband, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);

/** @} */

#endif  // _NETREG_API_H
