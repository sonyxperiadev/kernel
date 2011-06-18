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
*   @file   sim_api_old.h
*
*   @brief  This file defines the interface for OLD SIM API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed  in later release
*
****************************************************************************/
/**

*   @defgroup   SIMOLDAPIGroup   Legacy SIM API
*   @ingroup    SIMAPIGroup
*
*   @brief      This group defines the legacy interfaces to the SIM system and provides
*				API documentation.

*	Using the following functions, the user can	read/update all the EFiles as defined
*	in GSM 11.11 and 3GPP 31.101/31.102 and the Common PCN Handset Specification (CPHS). 
*
	Test code testing these API's can be found in at_sim.c

	\n Use the link below to navigate back to the Subscriber Identity Module Overview page. \n
    \li \if CAPI2
	\ref CAPI2SIMOverview
	\endif
	\if CAPI
	\ref SIMOverview
	\endif
****************************************************************************/
/**
*   @defgroup   USIMOLDAPIGroup   Legacy 3G UMTS SIM (USIM) API
*   @ingroup    USIMAPIGroup
*	@brief		3G specific API's.
*
	\n Use the link below to navigate back to the Subscriber Identity Module Overview page. \n
    \li \if CAPI2
	\ref CAPI2SIMOverview
	\endif
	\if CAPI
	\ref SIMOverview
	\endif
****************************************************************************/

#ifndef _SIM_API_OLD_H_
#define _SIM_API_OLD_H_

extern const UInt16 Select_Mf_Path;
extern const UInt16 Sim_Df_Cingular_Path[2];
extern const UInt16 Usim_Df_Cingular_Path[3];
extern const UInt16 Sim_Global_Pbk_Df_Path[2];
extern const UInt16 Sim_Local_Pbk_Df_Path[2];

/**
 * @addtogroup SIMOLDAPIGroup
 * @{
 */

//***************************************************************************************
/**
	Macro to get the path of EF-MF
**/	
#define SIM_GetMfPath() (&Select_Mf_Path)


//***************************************************************************************
/**
	Macro to get the path length of EF-MF
**/								
#define SIM_GetMfPathLen() ( sizeof(Select_Mf_Path) / sizeof(UInt16) )	


//***************************************************************************************
/**
	Macro to get the path of AT&T Cingular DF: 3F00/7F66 (2G SIM) or 3F00/7FFF/7F66 (3G USIM)
**/	
#define SIM_GetDfCingularPath()		((SIM_GetApplicationType() == SIM_APPL_2G) ? Sim_Df_Cingular_Path : Usim_Df_Cingular_Path)


//***************************************************************************************
/**
	Macro to get the path length of AT&T Cingular DF
**/	
#define SIM_GetDfCingularPathLen()	( ((SIM_GetApplicationType() == SIM_APPL_2G) ? sizeof(Sim_Df_Cingular_Path) : sizeof(Usim_Df_Cingular_Path)) / sizeof(UInt16) )


//***************************************************************************************
/**
	Macro to get the path length of Global phonebook in 3G USIM
**/	
#define SIM_GetGlobalPbkDfPath()	Sim_Global_Pbk_Df_Path
#define SIM_GetGlobalPbkDfPathLen()	( sizeof(Sim_Global_Pbk_Df_Path) / sizeof(UInt16) )


//***************************************************************************************
/**
	Macro to get the path of Local phonebook in 3G USIM
**/	
#define SIM_GetLocalPbkDfPath()		Sim_Local_Pbk_Df_Path


//***************************************************************************************
/**
	Macro to get the path length of Local phonebook in 3G USIM
**/	
#define SIM_GetLocalPbkDfPathLen()	( sizeof(Sim_Local_Pbk_Df_Path) / sizeof(UInt16) )
/** @} */

/** @addtogroup SIMOLDAPIGroup
	@{
*/

//***************************************************************************************
/**
	This function returns TRUE if the data cached by the SIM module (e.g. SMS data, CPHS
	information data) is ready to be read.

	@return TRUE if cached data is ready; FALSE otherwise
**/	
Boolean SIM_IsCachedDataReady(void);

//***************************************************************************************
/**
	This function returns the service bit status in the Customer Service Profile (EF-CSP).	
	@param		service_code (in) Service Code
	@return		SERVICE_FLAG_STATUS_t
**/	
SERVICE_FLAG_STATUS_t SIM_GetServiceCodeStatus(SERVICE_CODE_STATUS_CPHS_t service_code);

//***************************************************************************************
/**
	This function returns whether a CPHS sevice in the Customer Service Profile (EF-CSP)
	is both activated and allocated.

	@param		sst_entry (in) CPHS service type
	@return		TRUE if service is both activated and allocated; FALSE otherwise
**/	
Boolean SIM_CheckCphsService(CPHS_SST_ENTRY_t sst_entry);

//***************************************************************************************
/**
	This function returns the CPHS phase.

	@return		CPHS phase. 0x01 for phase 1; 0x02 for phase 2.
**/	
UInt8 SIM_GetCphsPhase(void);

//***************************************************************************************
/**
	This function returns the Service Center Number in the SMSP EF in the SIM.
	@param		sca_data (in) Service Center address
	@param		rec_no (in) 0-based Record number
	@return		Result_t
	@note
	Possible return values are ::SMS_SIM_BUSY, ::SMS_SIM_NOT_INSERT, ::RESULT_OK,
	::RESULT_ERROR.
**/	
Result_t SIM_GetSmsSca(SIM_SCA_DATA_t sca_data, UInt8 rec_no);

//******************************************************************************
/**
	This function returns the number of SMS Service Parameter records in SIM.
	@param		sms_param_rec_no (IO) pointer to where the result will be written
	@return		Result_t
	@note
	Possible return values are ::RESULT_OK if success, ::RESULT_ERROR otherwise.
	
**/
Result_t SIM_GetSmsParamRecNum(UInt8* sms_param_rec_no);

//***************************************************************************************
/**
	This function returns TRUE if the SIM indicates that the user subscribes to Alternative Line
	Service (ALS). 

	@return	Boolean: TRUE if ALS is enabled; FALSE otherwise
**/	

Boolean SIM_IsALSEnabled(void);

//***************************************************************************************
/**
	This function returns the current selected default voice line (L1 or L2 of ALS).

	@return		UInt8 : 0 if L1 is selected, 1 if L2 is selected
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM only.
**/	
UInt8 SIM_GetAlsDefaultLine(void);

//***************************************************************************************
/**
	This function selectes the default voice line (L1 or L2 of ALS).

	@param		line (in) 0 for L1; 1 for L2.
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM only.
**/	
void SIM_SetAlsDefaultLine(UInt8 line);

//***************************************************************************************
/**
	This function sends SIM file update indication for EF-PLMNsel in 2G SIM or EF-PLMNwact in 3G USIM 
	to the protocol stack so that the stack will re-read the relevant file and perform PLMN selection accordingly. 

	@param		prefer_plmn_file (in) SIM_PLMN_SEL (for EF-PLMNsel in 2G SIM) or USIM_PLMN_WACT (for EF-PLMNwact in 3G USIM)
**/
void SIM_SendPreferredPlmnUpdateInd(SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
	This function returns the call forwarding flags in CPHS EF-6F13 file. 

	@param		sim_file_exist (in) Flag to indicate whether the EF-6F13 file exists in SIM

	@return		L1/L2/Data/Fax call forwarding flags.
**/
SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t SIM_GetCallForwardUnconditionalFlag(Boolean *sim_file_exist);

//***************************************************************************************
/**
	This function returns the type of SIM inserted. 

	@return		SIM_APPL_2G if 2G SIM inserted; SIM_APPL_3G if 3G USIM inserted; SIM_APPL_INVALID if no SIM inserted
**/
SIM_APPL_TYPE_t SIM_GetApplicationType(void);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is a test SIM

	@return		TRUE if test SIM inserted; FALSE otherwise.
**/
Boolean SIM_IsTestSIM(void);

//***************************************************************************************
/**
	This function returns SMS "Memory Exceeded Flag" in EF-SMSS

	@return		TRUE if "SMS memory exceeded flag" is set; FALSE otherwise
**/
Boolean SIM_GetSmsMemExceededFlag(void);

//***************************************************************************************
/**
	This function updates SMS "Memory Exceeded Flag" in EF-SMSS. The calling task is blocked
	until the SIM update is finished.

    @param cap_exceeded (in) TRUE for "Memory Exceeded" status; FALSE otherwise
**/
void SIM_UpdateSMSCapExceededFlag(Boolean cap_exceeded);

//***************************************************************************************
/**
	This function returns whether the passed APN name is allowed based on the APN Control 
	List (ACL) setting.

	@param apn_name (in) APN name,e.g. "wap.cingular"

	@return		TRUE if APN name is allowed; FALSE otherwise
**/
Boolean SIM_IsAllowedAPN(const char *apn_name);


//***************************************************************************************
/**
	This function returns the SIM chip ID in EF-ICCID.

	@param		icc_parm (IO) Pointer to ICC ID buffer.
	@return		RESULT_OK if ICCID is returned successfully; RESULT_ERROR otherwise.
**/	
Result_t SIM_GetIccid(ICCID_ASCII_t *icc_parm);

//***************************************************************************************
/**
  This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. 
  This function is different from SIM_GetPresentStatus(). SIM_GetPresentStatus() works in Flight or Normal mode, 
  not in Off and Charging mode. SIM_DetectSim() works in Flight, Noraml, Off and Charging mode.

  @return	TRUE if SIM is inserted; FALSE otherwise
**/
Boolean SIM_DetectSim(void);

//***************************************************************************************
/**
	This function sets SIM Protocol (T=0, T=1). This is typically used in BT-SAP application
	to set the protcol preference.

    @param      protocol (in) Protocol, 0 for T=0; 1 for T=1
	@return		RESULT_OK if success, RESULT_ERROR otherwise.  
**/	
Result_t SIM_Set_Protocol(UInt8 protocol);

//***************************************************************************************
/**
	This function returns SIM Protocol (T=0, T=1).

	@return	0 for T=0 protocol; 1 for T=1 protocol
**/	
UInt8 SIM_Get_Protocol(void);

//***************************************************************************************
/**
	This function  terminates a Generic APDU transfer session. This function is called
	so that USIMAP can exit the APDU transfer mode and change into another state 
	to process other SIM commands.

	@return		RESULT_OK
**/	
Result_t SIM_TerminateXferApdu(void);

//***************************************************************************************
/**
	This function returns TRUE if RUIM SIM card is inserted. 

	@return  TRUE if RUIM SIM card is inserted; FALSE otherwise
**/
Boolean SIM_GetRuimSuppFlag(void);

//***************************************************************************************
/**
	This function returns the file ID of the parent of the passed file ID.

    @param file_id (in) SIM file ID

	@return  File ID of the parent of the passed file ID
**/
APDUFileID_t simmi_GetMasterFileId(APDUFileID_t file_id);

//***************************************************************************************
/**
	This function allows the client to indicate the SIMLOCK status to CP so that
	CP stores the status.

    @param simlock_state (out) SIMLOCK status structure
**/
void SIMLOCK_SetStatus(const SIMLOCK_STATE_t *simlock_state);


//***************************************************************************************
/**
	This function returns the SIM type of the SIM inserted indicated in EF-AD, i.e. 
	whether it is a Normal SIM or test SIM.

    @return SIM type of the inserted SIM.
**/
SIMType_t SIM_GetSIMType(void);		

//***************************************************************************************
/**
	This function returns the phase of the SIM card indicated in EF-AD.

    @return SIM phase of the inserted SIM.
**/
SIMPhase_t SIM_GetCardPhase(void);	

//***************************************************************************************
/**
	This function returns the inserted/removed status of the SIM card.

    @return inserted/removed status of the SIM card.
**/
SIMPresent_t SIM_GetPresentStatus(void);	

//***************************************************************************************
/**
	This function returns whether FDN feature is enabled.

    @return TRUE if FDN is enabled; FALSE otherwise.
**/
Boolean SIM_IsOperationRestricted(void);///< TRUE, if SIM is inserted, but restricted

//***************************************************************************************
/**
	This function returns whether BDN feature is enabled

    @return TRUE if BDN feature is enabled; FALSE otherwise.
**/
Boolean SIM_IsBdnOperationRestricted(void);

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is blocked

    @param chv (in) Indicate whether PIN1 or PIN2

    @return TRUE if PIN1/PIN2 is blocked
**/
Boolean SIM_IsPINBlocked(CHV_t chv);	

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is permanently blocked, i.e. the PUK attempt
	maximum has been exceeded.

    @param chv (in) Indicate whether PIN1 or PIN2

    @return TRUE if PIN1/PIN2 is permamently blocked
**/
Boolean SIM_IsPUKBlocked(CHV_t chv);	

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. 

    A SIM/USIM is considered invalid if it is not possible to use it for normal operation, 
	e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during 
	FDN status checking in USIMAP. 

    @return TRUE if SIM is invalid; FALSE otherwise.
**/
Boolean SIM_IsInvalidSIM(void);	

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. 

    A SIM/USIM is considered invalid if it is not possible to use it for normal operation, 
	e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during 
	FDN status checking in USIMAP. 

    @return TRUE if SIM is invalid; FALSE otherwise.
**/
SIMServiceStatus_t SIM_GetServiceStatus(SIMService_t service); ///< Get Service Status

//***************************************************************************************
/**
	This function returns current SIM PIN status. The status returned is based on the 
	highest priority that the user needs to first unlock. The priority is from high to 
	low: SIM PIN1; Network Lock; Network Subset Lock; Service Provider Lock; Corporate
	Lock; Phone Lock; SIM PIN2. 

    @return Current SIM PIN status.
**/
SIM_PIN_Status_t SIM_GetPinStatus(void);

//***************************************************************************************
/**
	This function returns whether the current SIM PIN status allows the phone to obtain
	Normal Service. 

    @return TRUE if the current SIM PIN status allows the phone to obtain
			Normal Service; FALSE otherwise.
**/	
Boolean SIM_IsPinOK(void);

//***************************************************************************************
/**
	This function returns SIM IMSI in ASCII encoded NULL terminated string.

    @return Pointer to the SIM IMSI in ASCII encoded NULL terminated string.
**/	
IMSI_t* SIM_GetIMSI(void);

//***************************************************************************************
/**
	This function returns the EF-GID1 data. 

    @return Pointer to the data in EF-GID1.
**/	
GID_DIGIT_t* SIM_GetGID1(void);

//***************************************************************************************
/**
	This function returns the EF-GID2 data. 

    @return Pointer to the data in EF-GID2.
**/	
GID_DIGIT_t* SIM_GetGID2(void);

//***************************************************************************************
/**
	This function returns the PLMN structure for Home PLMN with the
	appropriate MCC and MNC values based upon the Acting-HPLMN and IMSI
	data in SIM. If Acting-HPLMN EF exists in SIM and is not set to (0xFF, 0xFF, 0xFF),  
	The Home PLMN is obtained from Acting-HPLMN, otherwise it is obtained from IMSI. 

	Complicated logic of using 2-digit versus 3-digit MNC is defined
	in Annex A of GSM 03.22. Specifically, North American networks (MCC is 
	in range 310-316) must use 3-digit MNC in IMSI. Non-North-American 
	networks can use either 2-digit or 3-digit MNC in IMSI. 

	Due to the above reason, the 6th digit of IMSI is always written 
	to the 'c' nibble in 0xabcd value of MCC returned by this function if 
	the PLMN is obtained from IMSI, not Act-HPLMN. The caller can use logic 
	similar to that in MS_IsMatchedPLMN() function to perform PLMN match.  

    @param home_plmn (out) Pointer to the returned HPLMN data.
**/	
void SIM_GetHomePlmn(PLMNId_t *home_plmn);
/** @} */

/**
 * @addtogroup USIMOLDAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function returns the USIM service status in EF-UST for 
	the passed service type.

	If the service is not defined in USIM EF-UST, the default value
	of Service Activated status is returned.  

    @param service (in) USIM service type

    @return USIM service status.
**/	
SIMServiceStatus_t USIM_GetServiceStatus(SIMService_t service);


//***************************************************************************************
/**
    Function to check if the passed APN is allowed by APN Control List (ACL) feature in USIM. 
	This function should be called only if a USIM is inserted. 

	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular". 
	@return		TRUE if the passed APN is allowed; FALSE otherwise. 
	@note		When matching the passed APN name with those in EF-ACL, the comparison is case-insensitive.
				For example, "isp.cingular", "Isp.Cingular" and "ISP.CINGULAR" are considered the same. 
**/	
Boolean USIM_IsAllowedAPN(const char *apn_name);

//***************************************************************************************
/**
    Function to return the number of APN's in EF-ACL in USIM. This function should be called
	only if a USIM is inserted. 

	@return		Number of APN's. 	
**/	
UInt8 USIM_GetNumOfAPN(void);

//***************************************************************************************
/**
    Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted. 

	@param		index (in) 0-based index. 
	@param		apn_name (out) Buffer to store the returned APN name whose labels are 
				separated by '.', e.g. "isp.cingular".
	
	@return		RESULT_OK if the APN name is returned successfully; RESULT_ERROR otherwise. 		
**/	
Result_t USIM_GetAPNEntry(UInt8 index, APN_NAME_t *apn_name);

//***************************************************************************************
/**
    Function to check the activated/deactivated status for a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. 

	@param		est_serv (in) Service type in EF-EST.  
	
	@return		TRUE if service is activated; FALSE otherwise. 
	@see		
**/	
Boolean USIM_IsEstServActivated(USIM_EST_SERVICE_t est_serv);

//***************************************************************************************
/**
    Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting, i.e. 
	whether the phone is Dual Mode, 2G only or 3G only. If EF-RAT does not exist in SIM, 
	USIM_RAT_MODE_INVALID is returned. 
	
	@return		RAT mode setting in AT&T EF-RAT.  				
**/	
USIM_RAT_MODE_t USIM_GetRatModeSetting(void);

/** @} */

/**
 * @addtogroup SIMOLDAPIGroup
 * @{
 */

//---------------------------------------------------------------------------------------
//						ASynchronous Function Prototype
//---------------------------------------------------------------------------------------

//***************************************************************************************
/**
    This function requests the parameter information of a MF/DF in the USIM, i.e. the response
	data to the Select MF/DF command.  

	@param		clientID (in) Client ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_DFILE_INFO_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendDFileInfoReq(UInt8 clientID, APDUFileID_t dfile_id, UInt8 path_len,
		const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the parameter information of a MF/DF in the SIM, i.e. the response
	data to the Select MF/DF command.

	@param		clientID (in) Client ID

	@param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().
	
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_DFILE_INFO_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SubmitDFileInfoReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len,
		const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic). It should be used for only 
	2G SIM or 3G USIM application. 

	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t

	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_INFO_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendEFileInfoReq(UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic).
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_INFO_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SubmitEFileInfoReq(UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests all the contents of a transparent EF in the SIM. It should be 
	used for only 2G SIM or 3G USIM application. 
	
	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendWholeBinaryEFileReadReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests all the contents of a transparent EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SubmitWholeBinaryEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests some data in a transparent EF in the SIM. It should be used for 
	only 2G SIM or 3G USIM application. 
	
	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in) offset of the first byte to be read in the EF.
	@param		length (in) number of bytes to read in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Read Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 
	
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendBinaryEFileReadReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests some data in a transparent EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in) offset of the first byte to be read in the EF.
	@param		length (in) number of bytes to read in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Read Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SubmitBinaryEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in) one-based record number.
	@param		length (in) length of a record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendRecordEFileReadReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in) one-based record number.
	@param		length (in) length of a record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SubmitRecordEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a transparent EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in) offset of the first byte to be written in the EF.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Update Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SendBinaryEFileUpdateReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
			const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a transparent EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in) offset of the first byte to be written in the EF.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Update Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SubmitBinaryEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							 const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a record in a linear fixed EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in) one-based record number.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SendLinearEFileUpdateReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a record in a linear fixed EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in) one-based record number.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SubmitLinearEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of the next record in a cyclic EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		clientID (in) Client ID
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) length of record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SendCyclicEFileUpdateReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to update the contents of the next record in a cyclic EF in the SIM.
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) length of record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SubmitCyclicEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to send the Search Record command (called Seek command in 2G SIM spec 11.11)
	to the SIM. For 3G USIM, it gets all the record indices of a linear fixed or cyclic file that matches a given pattern.
	For 2G SIM, it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.

	It should be used for only 2G SIM or 3G USIM application. 

    This function is very useful for speeding up SIM file cacheing on 3G USIM. For example in 3G phonebook in USIM, 
	typically a lot of the empty records exist in the phonebook files, e.g. EF-ADN, EF-PBC and EF-GRP. 

	This function can be called once to get indix list of the empty records for the above three exmplary files:
	EF-ADN: pass the pattern data of all 0xFF to get indices of empty records. 
	EF-PBC: pass the pattern data of all 0x00 to get indices of empty records.
	EF-GRP: pass the pattern data of all 0x00 to get indices of empty records. 
	
	Once the empty record list is obtained, no Read Record command is sent for the empty records but we can just 
	memset our data buffers to 0xFF or 0x00. Thus this can greatly reduce phonebook initialization time. 

	Besides phonebook files, this function can be used for any linear fixed or cyclic files, e.g. EONS EF-PNN, 
	EONS EF-OPL, EF-FDN & EF-SDN. 

	The above techniques shall be used for 3G USIM only. It is not useful for 2G SIM because the Seek command in 
	2G SIM returns the indix of the first matching record instead of the indices of all the matching records. 

	@param		clientID (in) Client ID
	@param		efile_id (in) Elementary File ID
	@param		dfile_id (in) Directory File ID
	@param		*ptr (in) pointer to data pattern
	@param		length (in) length of data pattern
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_SEEK_REC_RSP message by
	calling the passed callback function.
**/	
Result_t SIM_SendSeekRecordReq( UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to send the Search Record command (called Seek command in 2G SIM spec 11.11)
	to the SIM. For 3G USIM, it gets all the record indices of a linear fixed or cyclic file that matches a given pattern.
	For 2G SIM, it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.

    This function is very useful for speeding up SIM file cacheing on 3G USIM. For example in 3G phonebook in USIM, 
	typically a lot of the empty records exist in the phonebook files, e.g. EF-ADN, EF-PBC and EF-GRP. 

	This function can be called once to get indix list of the empty records for the above three exmplary files:
	EF-ADN: pass the pattern data of all 0xFF to get indices of empty records. 
	EF-PBC: pass the pattern data of all 0x00 to get indices of empty records.
	EF-GRP: pass the pattern data of all 0x00 to get indices of empty records. 
	
	Once the empty record list is obtained, no Read Record command is sent for the empty records but we can just 
	memset our data buffers to 0xFF or 0x00. Thus this can greatly reduce phonebook initialization time. 

	Besides phonebook files, this function can be used for any linear fixed or cyclic files, e.g. EONS EF-PNN, 
	EONS EF-OPL, EF-FDN & EF-SDN. 

	The above techniques shall be used for 3G USIM only. It is not useful for 2G SIM because the Seek command in 
	2G SIM returns the indix of the first matching record instead of the indices of all the matching records. 
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) Elementary File ID
	@param		dfile_id (in) Directory File ID
	@param		*ptr (in) pointer to data pattern
	@param		length (in) length of data pattern
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_SEEK_REC_RSP message by
	calling the passed callback function.
**/
Result_t SIM_SubmitSeekRecordReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests the number of remaining PIN1 and PIN2 attempts in the SIM.
	
	@param		clientID (in) Client ID
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_PIN_ATTEMPT_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendRemainingPinAttemptReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function requests the number of PLMN entries in the PLMN file. 

	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_PLMN_NUM_OF_ENTRY_RSP message by calling
	the passed callback function. The number of entries returned in that message is the 
	total number of PLMN storages provisioned in SIM, including empty entries. 
**/	
Result_t SIM_SendNumOfPLMNEntryReq(UInt8 clientID, SIM_PLMN_FILE_t plmn_file, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function requests PLMN entry contents in the PLMN file. 

	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@param		start_index (in) 0-based index of the first entry requested. 
	@param		end_index (in) 0-based index of the last entry requested. Pass 
				UNKNOWN_PLMN_INDEX to read all including the last entry starting from "start_index". 
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_PLMN_ENTRY_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SIM_SendReadPLMNEntryReq( UInt8 clientID, SIM_PLMN_FILE_t plmn_file, 
			UInt16 start_index, UInt16 end_index, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update multiple PLMN entries in the PLMN file.

	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access, SIM_PLMN_SEL for 2G SIM and USIM_PLMN_WACT for 3G USIM
    @param      first_idx (in) 0-based index of the first entry to write. If UNKNOWN_PLMN_INDEX
				is passed, the entries will be written to the free entry slots in SIM file. If 
				there are not enough free slots, error will be returned.
    @param      number_of_entry (in) Number of consecutive entries to write
    @param      plmn_entry[] (in) PLMN data entries to write
    @param		sim_access_cb (in) SIM access callback
    
	@note
	The SIM task returns the result in the ::MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP message by calling
	the passed callback function.

	If the update is successful, the client must call SIM_SendPreferredPlmnUpdateInd()
	function to send the preferred PLMN update indication to the protocol stack
	so that the stack can re-read the file and perform PLMN selection accordingly. 
**/	
Result_t SIM_SendWriteMulPLMNEntryReq( UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry,
    SIM_MUL_PLMN_ENTRY_t plmn_entry[], CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
	This function powers on/off SIM card. A MSG_SIM_POWER_ON_OFF_CARD_RSP message will be composed and passed
	to the callback function.

    @param		clientID (IN) Client ID
	@param		power_on (IN) True to power on, False to power off.
    @param      mode (IN) power on mode [0-Generic 1-Normal]. Applicable only if power_on is True.
    @param      sim_access_cb (IN) Registered event callback 
	@return		Result_t

**/	
Result_t SIM_PowerOnOffCard(UInt8 clientID, Boolean power_on, SIM_POWER_ON_MODE_t mode, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
	This function  retreives raw ATR from SIM driver. A MSG_SIM_GET_RAW_ATR_RSP message will be passed back
	to the callback function.

    @param		clientID (IN) Client ID
    @param      sim_access_cb (IN) Registered event callback 

	@return		Result_t
**/	
Result_t SIM_GetRawAtr(UInt8 clientID, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends a generic APDU command to the SIM. A MSG_SIM_SEND_GENERIC_APDU_CMD_RSP message
	will be passed back to the callback function.

    @param		clientID (IN) Client ID
    @param		apdu (IN) APDU command data
    @param		len (IN) APDU command length
    @param      sim_access_cb (IN) Registered event callback   
	@return		Result_t
**/
Result_t SIM_SendGenericApduCmd(UInt8 clientID, UInt8 *apdu, UInt16 len, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to USIM to open a logical channel. It is called before a 
	non-USIM application is activated. It will trigger a MSG_SIM_OPEN_SOCKET_RSP message to 
	be passed back to the callback function.

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SIM_SendOpenSocketReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);
//***************************************************************************************
/**
	This function sends the request to SIM to activate a non-USIM application and associate it
	with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP 
	message to be passed back to the callback function.

	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel
	@param		aid_data (IN) AID data in one EF-DIR record for the application to be activated
	@param		aid_len (IN) AID data length
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SIM_SendSelectAppiReq(UInt8 clientID, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to USIM to deactivate a non-USIM application associated
	with the passed socket ID representing a logical channel. The application must have been
	activated through SIM_SendSelectAppiReq() previously. It will trigger a MSG_SIM_DEACTIVATE_APPLI_RSP 
	message to be passed back to the callback function.

	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel and associated with the application
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SIM_SendDeactivateAppiReq(UInt8 clientID, UInt8 socket_id, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to close the logical channel previously opened through
	SIM_SendOpenSocketReq() function. The application associated with the logical channel must be 
	deactivated through SIM_SendDeactivateAppiReq() function before this function is called. It will 
	trigger a MSG_SIM_CLOSE_SOCKET_RSP message to be passed back to the callback function.

	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SIM_SendCloseSocketReq(UInt8 clientID, UInt8 socket_id, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to verify PIN1 or PIN2 password.

	@param		clientID (IN) Client ID
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		chv (IN) PIN1/PIN2 password in ASCII format
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_VERIFY_CHV_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendVerifyChvReq(UInt8 clientID, CHV_t chv_select,	CHVString_t chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to change PIN1 or PIN2 password.

	@param		clientID (IN) Client ID
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		old_chv (IN) Old PIN1/PIN2 password in ASCII format
	@param		new_chv (IN) New PIN1/PIN2 password in ASCII format
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_CHANGE_CHV_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendChangeChvReq(UInt8 clientID, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable the SIM PIN1 feature.

	@param		clientID (IN) Client ID
	@param		chv (IN) PIN1 password
	@param		enable_flag (IN) TRUE to enable SIM PIN1; FALSE to disable SIM PIN1
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ENABLE_CHV_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendSetChv1OnOffReq(UInt8 clientID, CHVString_t chv, Boolean enable_flag, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to unblock SIM PIN1 or PIN2.

	@param		clientID (IN) Client ID
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		puk (IN) PUK1 or PUK2 password
	@param		new_chv (IN) New PIN1 or PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_UNBLOCK_CHV_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendUnblockChvReq(UInt8 clientID, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable FDN feature.

	@param		clientID (IN) Client ID
	@param		oper_state (IN) SIMOPERSTATE_RESTRICTED_OPERATION to enable FDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable FDN
	@param		chv2 (IN) PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SET_FDN_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendSetOperStateReq(UInt8 clientID, SIMOperState_t oper_state, CHVString_t chv2, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable BDN feature.

	@param		clientID (IN) Client ID
	@param		oper_state (IN) SIMOPERSTATE_RESTRICTED_OPERATION to enable BDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable BDN
	@param		chv2 (IN) PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SET_BDN_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendSetBdnReq(UInt8 clientID, SIMBdnOperState_t oper_state, CHVString_t chv2, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to USIMAP to get the configuration information for 
	ADN, FDN, MSISDN, LND, BDN and SDN phonebooks. 

	@param		clientID (IN) Client ID
	@param		id (IN) Phonebook ID to identify the phonebook type
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PBK_INFO_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendPbkInfoReq(UInt8 clientID,	SIMPBK_ID_t id,	CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the maximum Accumulated Call Meter value. 

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_MAX_ACM_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendReadAcmMaxReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the maximum Accumulated Call Meter value. 

	@param		clientID (IN) Client ID
	@param		acm_max (IN) Maximum Accumulated Call Meter value
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_MAX_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendWriteAcmMaxReq(UInt8 clientID,	CallMeterUnit_t acm_max, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Accumulated Call Meter value. 

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_VALUE_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendReadAcmReq(UInt8 clientID,	CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Accumulated Call Meter value. 

	@param		clientID (IN) Client ID
	@param		acm (IN) Accumulated Call Meter value
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendWriteAcmReq(UInt8 clientID, CallMeterUnit_t acm, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to increase the Accumulated Call Meter value. 

	@param		clientID (IN) Client ID
	@param		acm (IN) Accumulated Call Meter value by which to increase the value in SIM
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_INCREASE_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendIncreaseAcmReq(UInt8 clientID,	CallMeterUnit_t acm, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Service Provider Name in EF-SPN

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SVC_PROV_NAME_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendReadSvcProvNameReq(UInt8 clientID,	CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Price Per Unit and Currency information.

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PUCT_DATA_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendReadPuctReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Price Per Unit and Currency information.

	@param		clientID (IN) Client ID
	@param		currency (IN) Currency name in string format
	@param		eppu (IN) Price Per Unit data
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PUCT_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendWritePuctReq(UInt8 clientID, CurrencyName_t currency, EPPU_t *eppu, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends a Generic SIM Access command to the SIM.

	@param		clientID (IN) Client ID
	@param		command (IN) APDU command byte
	@param		efile_id (IN) EF file ID of the file to be selected
	@param		dfile_id (IN) File ID of the parent of the EF to be selected
	@param		p1 (IN) APDU P1 byte
	@param		p2 (IN) APDU P2 byte
	@param		p3 (IN) APDU p3 byte
	@param		path_len (IN) Path length of the selection path on top of "dfile_id"
	@param		select_path (IN) Selection path on top of "dfile_id"
	@param		data (IN) APDU command data to be executed after file selection
	@param      sim_access_cb (IN) Registered event callback   
	
	@return		Result_t

    @note a MSG_SIM_RESTRICTED_ACCESS_RSP message will be returned to the passed callback function
**/
Result_t SIM_SendRestrictedAccessReq(UInt8 clientID, APDUCmd_t command,	APDUFileID_t efile_id, APDUFileID_t dfile_id,				
	UInt8 p1, UInt8 p2,	UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends a Generic SIM Access command to the SIM.

	@param		clientID (IN) Client ID

	@param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		command (IN) APDU command byte
	@param		efile_id (IN) EF file ID of the file to be selected
	@param		dfile_id (IN) File ID of the parent of the EF to be selected
	@param		p1 (IN) APDU P1 byte
	@param		p2 (IN) APDU P2 byte
	@param		p3 (IN) APDU p3 byte
	@param		path_len (IN) Path length of the selection path on top of "dfile_id"
	@param		select_path (IN) Selection path on top of "dfile_id"
	@param		data (IN) APDU command data to be executed after file selection
	@param      sim_access_cb (IN) Registered event callback   
	
	@return		Result_t

    @note a MSG_SIM_RESTRICTED_ACCESS_RSP message will be returned to the passed callback function
**/
Result_t SIM_SubmitRestrictedAccessReq(UInt8 clientID, UInt8 socket_id,	APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id,				
	UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data, CallbackFunc_t* sim_access_cb);
/** @} */

/**
 * @addtogroup USIMOLDAPIGroup
 * @{
 */

//***************************************************************************************
/**
    Function to send a request to USIM to activate/deactivate a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP 
	message will be passed back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_SET_EST_SERV_RSP message. 

	@param		clientID (in) Client ID of calling task.  
	@param		est_serv (in) Service type in EF-EST.  
	@param 		serv_on (in) TRUE to activate service; FALSE to deactivate service. 
	@param 		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  	
**/	
Result_t USIM_SendSetEstServReq(UInt8 clientID, USIM_EST_SERVICE_t est_serv, Boolean serv_on, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    Function to send a request to USIM to update an APN in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_UPDATE_ONE_APN_RSP message. 

	@param		clientID (in) Client ID of calling task.  
	@param		index (in) 0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.   
	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular".
							  Pass NULL to delete an APN from EF-ACL. 
	@param		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  			
**/	
Result_t USIM_SendWriteAPNReq(UInt8 clientID, UInt8 index, const char *apn_name, CallbackFunc_t* sim_access_cb);
//***************************************************************************************
/**
    Function to send a request to USIM to delete all APN's in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_DELETE_ALL_APN_RSP message. 

	@param		clientID (in) Client ID of calling task.  
	@param		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  				
**/	
Result_t USIM_SendDeleteAllAPNReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);
/** @} */

#endif  // _SIMAPI_OLD_H_
