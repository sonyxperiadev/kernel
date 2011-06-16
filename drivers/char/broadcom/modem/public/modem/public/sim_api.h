//
//	Copyright ?2004-2008 Broadcom Corporation
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
*   @file   sim_api.h
*
*   @brief  This file defines the interface for SIM API functions.
*
****************************************************************************/

/**

*   @defgroup   SIMAPIGroup   SIM API
*   @ingroup    SIMGroup
*
*   @brief      This group defines the interfaces to the SIM system and provides
*				API documentation.

*	Using the following functions, the user can	read/update all the EFiles as defined
*	in GSM 11.11 and 3GPP 31.101/31.102 and the Common PCN Handset Specification (CPHS). 
*
	Test code testing these API's can be found in at_sim.c
****************************************************************************/
/**
*   @defgroup   USIMAPIGroup   3G UMTS SIM (USIM) API
*   @ingroup    SIMSTK
	@brief		3G specific API's.

	Here is some sample code demonstrating the API's.

	\dontinclude at_sim_test.c
	\skip * Function Name:	testUsimAdnPbk
	\until return AT_STATUS_PENDING;	
****************************************************************************/


#ifndef _SIM_API_H_
#define _SIM_API_H_


//---------------------------------------------------------------------------------------
//							Synchronous Function Prototype
//---------------------------------------------------------------------------------------

/**
 * @addtogroup SIMAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function returns TRUE if the data cached by the SIM module (e.g. SMS data, CPHS
	information data) is ready to be read.

    @param		inClientInfoPtr (in) pointer to the client Info

	@return TRUE if cached data is ready; FALSE otherwise
**/	
Boolean SimApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the service bit status in the Customer Service Profile (EF-CSP).	

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		service_code (in) Service Code
	@return		SERVICE_FLAG_STATUS_t
**/
SERVICE_FLAG_STATUS_t SimApi_GetServiceCodeStatus(ClientInfo_t* inClientInfoPtr, SERVICE_CODE_STATUS_CPHS_t service_code);


//******************************************************************************
/**
	This function returns the number of SMS Service Parameter records in SIM.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		sms_param_rec_no (IO) pointer to where the result will be written
	@return		Result_t
	@note
	Possible return values are ::RESULT_OK if success, ::RESULT_ERROR otherwise.
	
**/
Result_t SimApi_GetSmsParamRecNum(ClientInfo_t* inClientInfoPtr, UInt8* sms_param_rec_no);

//***************************************************************************************
/**
	This function returns TRUE if the SIM indicates that the user subscribes to Alternative Line
	Service (ALS). 

    @param		inClientInfoPtr (in) pointer to the client Info
	@return	Boolean: TRUE if ALS is enabled; FALSE otherwise
**/	
Boolean SimApi_IsALSEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the current selected default voice line (L1 or L2 of ALS).

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		UInt8 : 0 if L1 is selected, 1 if L2 is selected
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM only.
**/	
UInt8 SimApi_GetAlsDefaultLine(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	This function sends SIM file update indication for EF-PLMNsel in 2G SIM or EF-PLMNwact in 3G USIM 
	to the protocol stack so that the stack will re-read the relevant file and perform PLMN selection accordingly. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		prefer_plmn_file (in) SIM_PLMN_SEL (for EF-PLMNsel in 2G SIM) or USIM_PLMN_WACT (for EF-PLMNwact in 3G USIM)
**/
void SimApi_SendPreferredPlmnUpdateInd(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
	This function returns the call forwarding flags in CPHS EF-6F13 file. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		sim_file_exist (in) Flag to indicate whether the EF-6F13 file exists in SIM

	@return		L1/L2/Data/Fax call forwarding flags.
**/
SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t SimApi_GetCallForwardUnconditionalFlag(ClientInfo_t* inClientInfoPtr, Boolean *sim_file_exist);


//***************************************************************************************
/**
	This function returns the type of SIM inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		SIM_APPL_2G if 2G SIM inserted; SIM_APPL_3G if 3G USIM inserted; SIM_APPL_INVALID if no SIM inserted
**/
SIM_APPL_TYPE_t SimApi_GetApplicationType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is a test SIM

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		TRUE if test SIM inserted; FALSE otherwise.
**/
Boolean SimApi_IsTestSIM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM PIN needs to be entered.

    @return		TRUE if SIM PIN needs to be entered; FALSE otherwise.
**/
Boolean SIM_IsPINRequired(void);		


//***************************************************************************************
/**
	This function sets SIM Protocol (T=0, T=1). This is typically used in BT-SAP application
	to set the protcol preference.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param      protocol (in) Protocol, 0 for T=0; 1 for T=1
	@return		RESULT_OK if success, RESULT_ERROR otherwise.  
**/	
Result_t SimApi_Set_Protocol(ClientInfo_t* inClientInfoPtr, UInt8 protocol);

//***************************************************************************************
/**
	This function returns SIM Protocol (T=0, T=1).

    @param		inClientInfoPtr (in) pointer to the client Info
	@return	0 for T=0 protocol; 1 for T=1 protocol
**/	
UInt8 SimApi_Get_Protocol(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function  terminates a Generic APDU transfer session. This function is called
	so that USIMAP can exit the APDU transfer mode and change into another state 
	to process other SIM commands.

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		RESULT_OK
**/	
Result_t SimApi_TerminateXferApdu(ClientInfo_t* inClientInfoPtr);


/** @} */

/**
 * @addtogroup SIMLOCKGroup
 * @{
 */

//***************************************************************************************
/**
	This function allows the client to indicate the SIMLOCK status to CP so that
	CP stores the status.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param simlock_state (out) SIMLOCK status structure
**/
void SIMLOCKApi_SetStatus(ClientInfo_t* inClientInfoPtr, const SIMLOCK_STATE_t *simlock_state);
/** @} */

/**
 * @addtogroup SIMAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function returns the SIM type of the SIM inserted indicated in EF-AD, i.e. 
	whether it is a Normal SIM or test SIM.

    @param		inClientInfoPtr (in) pointer to the client Info
    @return SIM type of the inserted SIM.
**/
SIMType_t SimApi_GetSIMType( ClientInfo_t* inClientInfoPtr );	///< Get SIM type

//***************************************************************************************
/**
	This function returns the phase of the SIM card indicated in EF-AD.

    @param		inClientInfoPtr (in) pointer to the client Info
    @return SIM phase of the inserted SIM.
**/
SIMPhase_t SimApi_GetCardPhase(ClientInfo_t* inClientInfoPtr);	///< Return the SIM card phase

//***************************************************************************************
/**
	This function returns the inserted/removed status of the SIM card.

    @param		inClientInfoPtr (in) pointer to the client Info
    @return inserted/removed status of the SIM card.
**/
SIMPresent_t SimApi_GetPresentStatus(ClientInfo_t* inClientInfoPtr); ///< SIM present status

//***************************************************************************************
/**
	This function returns whether FDN feature is enabled.

    @param		inClientInfoPtr (in) pointer to the client Info
    @return TRUE if FDN is enabled; FALSE otherwise.
**/
Boolean SimApi_IsOperationRestricted(ClientInfo_t* inClientInfoPtr);///< TRUE, if SIM is inserted, but restricted

//***************************************************************************************
/**
	This function returns whether BDN feature is enabled

    @param		inClientInfoPtr (in) pointer to the client Info
    @return TRUE if BDN feature is enabled; FALSE otherwise.
**/
Boolean SimApi_IsBdnOperationRestricted(ClientInfo_t* inClientInfoPtr);///< TRUE, if SIM is inserted, but restricted

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is blocked

    @param		inClientInfoPtr (in) pointer to the client Info
    @param chv (in) Indicate whether PIN1 or PIN2

    @return TRUE if PIN1/PIN2 is blocked
**/
Boolean SimApi_IsPINBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv); ///< Check if CHV's PIN blocked

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is permanently blocked, i.e. the PUK attempt
	maximum has been exceeded.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param chv (in) Indicate whether PIN1 or PIN2

    @return TRUE if PIN1/PIN2 is permamently blocked
**/
Boolean SimApi_IsPUKBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv);	///< Check if PUK blocked (unblock attempts exceeded

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. 

    A SIM/USIM is considered invalid if it is not possible to use it for normal operation, 
	e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during 
	FDN status checking in USIMAP. 

    @param		inClientInfoPtr (in) pointer to the client Info
    @return TRUE if SIM is invalid; FALSE otherwise.
**/

Boolean SimApi_IsInvalidSIM(ClientInfo_t* inClientInfoPtr);  ///< Check if the inserted SIM/USIM is invalid


//***************************************************************************************
/**
	This function returns the data in EF-UST. It shall be called only after 
	MSG_SIM_CACHED_DATA_READY_IND inter-task message has been sent to client



     @param		inClientInfoPtr (in) pointer to the client Info
     @param		outUstPtr (out) buffer to store EF-UST data

    @return		None
**/	
void USimApi_GetUstData(ClientInfo_t* inClientInfoPtr, UST_DATA_t* outUstPtr);


//***************************************************************************************
/**
	This function returns the data in EF-AD. It shall be called only after 
	MSG_SIM_DETECTION_IND inter-task message has been sent to client and it indicates
	SIM is present.

     @param		inClientInfoPtr (in) pointer to the client Info
     @param		outAdPtr (out) buffer to store EF-AD data

    @return		None
**/	
void SimApi_GetAdData(ClientInfo_t* inClientInfoPtr, AD_DATA_t* outAdPtr);


//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. 

    A SIM/USIM is considered invalid if it is not possible to use it for normal operation, 
	e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during 
	FDN status checking in USIMAP. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param service (in) USIM service type
    @return TRUE if SIM is invalid; FALSE otherwise.
**/
SIMServiceStatus_t SimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service); ///< Get Service Status

//***************************************************************************************
/**
	This function returns current SIM PIN status. The status returned is based on the 
	highest priority that the user needs to first unlock. The priority is from high to 
	low: SIM PIN1; Network Lock; Network Subset Lock; Service Provider Lock; Corporate
	Lock; Phone Lock; SIM PIN2. 

    @param		inClientInfoPtr (in) pointer to the client Info
    @return Current SIM PIN status.
**/
SIM_PIN_Status_t SimApi_GetPinStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the current SIM PIN status allows the phone to obtain
	Normal Service. 

    @param		inClientInfoPtr (in) pointer to the client Info
    @return TRUE if the current SIM PIN status allows the phone to obtain
			Normal Service; FALSE otherwise.
**/	
Boolean SimApi_IsPinOK(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns SIM IMSI in ASCII encoded NULL terminated string.

    @param		inClientInfoPtr (in) pointer to the client Info
    @return Pointer to the SIM IMSI in ASCII encoded NULL terminated string.
**/	
IMSI_t* SimApi_GetIMSI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID1 data. 

    @param		inClientInfoPtr (in) pointer to the client Info
    @return Pointer to the data in EF-GID1.
**/	
GID_DIGIT_t* SimApi_GetGID1(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID2 data. 

    @param		inClientInfoPtr (in) pointer to the client Info
    @return Pointer to the data in EF-GID2.
**/	
GID_DIGIT_t* SimApi_GetGID2(ClientInfo_t* inClientInfoPtr);

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

    @param		inClientInfoPtr (in) pointer to the client Info
    @param home_plmn (out) Pointer to the returned HPLMN data.
**/	
void SimApi_GetHomePlmn(ClientInfo_t* inClientInfoPtr, PLMNId_t *home_plmn);
/** @} */

/**
 * @addtogroup USIMAPIGroup
 * @{
 */

//***************************************************************************************
/**
	This function returns the USIM service status in EF-UST for 
	the passed service type.

	If the service is not defined in USIM EF-UST, the default value
	of Service Activated status is returned.  

     @param		inClientInfoPtr (in) pointer to the client Info
     @param service (in) USIM service type

    @return USIM service status.
**/	
SIMServiceStatus_t USimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service);


//***************************************************************************************
/**
    Function to return the number of APN's in EF-ACL in USIM. This function should be called
	only if a USIM is inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		Number of APN's. 	
**/	
UInt8 USimApi_GetNumOfAPN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
    Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		index (in) 0-based index. 
	@param		apn_name (out) Buffer to store the returned APN name whose labels are 
				separated by '.', e.g. "isp.cingular".
	
	@return		RESULT_OK if the APN name is returned successfully; RESULT_ERROR otherwise. 		
**/	
Result_t USimApi_GetAPNEntry(ClientInfo_t* inClientInfoPtr, UInt8 index, APN_NAME_t *apn_name);

//***************************************************************************************
/**
    Function to check the activated/deactivated status for a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		est_serv (in) Service type in EF-EST.  
	
	@return		TRUE if service is activated; FALSE otherwise. 
	@see		
**/	
Boolean USimApi_IsEstServActivated(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv);

/** @} */

/**
 * @addtogroup SIMAPIGroup
 * @{
 */

//---------------------------------------------------------------------------------------
//						ASynchronous Function Prototype
//---------------------------------------------------------------------------------------

//***************************************************************************************
/**
    This function requests the parameter information of a MF/DF in the USIM, i.e. the response
	data to the Select MF/DF command.  

	@param		client_info_ptr (in) Pointer to client info
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_DFILE_INFO_RSP message by calling
	the passed callback function.
**/	
Result_t SimApi_SendDFileInfoReq(ClientInfo_t* client_info_ptr, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the parameter information of a MF/DF in the SIM, i.e. the response
	data to the Select MF/DF command.

	@param		client_info_ptr (in) Pointer to client info

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
Result_t SimApi_SubmitDFileInfoReq(ClientInfo_t* client_info_ptr, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic). It should be used for only 
	2G SIM or 3G USIM application. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendEFileInfoReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic).

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SubmitEFileInfoReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests some data in a transparent EF in the SIM. It should be used for 
	only 2G SIM or 3G USIM application. 
	
	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendBinaryEFileReadReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests some data in a transparent EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitBinaryEFileReadReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendRecordEFileReadReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitRecordEFileReadReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function sends the request to SIM module to read multiple records of a linear-fixed 
	or cyclic SIM file.
	
	@param		inClientInfoPtr (in) pointer to the client Info

    @param		socketId (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efileId (in) EF ID
	@param		dfileId (in) DF ID
	@param		firstRecNum (in) one-based record number of the first record
	@param		numOfRec (in) number of records to read
	
	@param		recLen (in) length of a record in the EF which is obtained through SimApi_SubmitEFileInfoReq() function. 
				If set to 0, this function will internally send the "Select" command to obtain the record length info.

	@param		pathLen (in) Path length 
	@param		*pSelectPath (in) Pointer to path to be selected
	
	@param		optimizationFlag1 (in) Whether to optimize the reading by skipping the empty records with all 0xFF found through 
				Search Record command. Generally this flag should be set TRUE only if the number of records 
				of the EF is larger than 2 and it is likely that some records are empty. This flag only has significance for 3G USIM 
				and is ignored for 2G SIM. 

	@param		optimizationFlag2 (in) Whether to optimize the reading by skipping the empty records with all 0x00 found through 
				Search Record command. Generally this flag should be set TRUE only if the number of records 
				of the EF is larger than 2 and it is likely that some records are empty. This flag only has significance for 3G USIM 
				and is ignored for 2G SIM. 

	@param		*pAccessCb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_MUL_REC_DATA_RSP message by calling the passed callback function.

    The total number of bytes requested shall not be larger than MAX_MUL_REC_DATA_SIZE otherwise SIMACCESS_NO_ACCESS will be returuned
    in MSG_SIM_MUL_REC_DATA_RSP message. 

    It is strongly recommended that client limits the number of records appropriately so that it does not block other SIM access and 
	IPC traffic.
**/	
Result_t SimApi_SubmitMulRecordEFileReq( ClientInfo_t* inClientInfoPtr, UInt8 socketId, APDUFileID_t efileId, APDUFileID_t dfileId, UInt8 firstRecNum, 
						UInt8 numOfRec, UInt8 recLen, UInt8 pathLen, const UInt16 *pSelectPath, Boolean optimizationFlag1, 
						Boolean optimizationFlag2, CallbackFunc_t* pAccessCb );


//***************************************************************************************
/**
    This function requests to update the contents of a transparent EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendBinaryEFileUpdateReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
			const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a transparent EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitBinaryEFileUpdateReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							 const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a record in a linear fixed EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendLinearEFileUpdateReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of a record in a linear fixed EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitLinearEFileUpdateReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of the next record in a cyclic EF in the SIM.
	It should be used for only 2G SIM or 3G USIM application. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendCyclicEFileUpdateReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update the contents of the next record in a cyclic EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitCyclicEFileUpdateReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
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

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendSeekRecordReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
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
	
	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitSeekRecordReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
	    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests the number of remaining PIN1 and PIN2 attempts in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_PIN_ATTEMPT_RSP message by calling
	the passed callback function.
**/	
Result_t SimApi_SendRemainingPinAttemptReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests the number of PLMN entries in the PLMN file. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		plmn_file (in) PLMN file to access
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_PLMN_NUM_OF_ENTRY_RSP message by calling
	the passed callback function. The number of entries returned in that message is the 
	total number of PLMN storages provisioned in SIM, including empty entries. 
**/	
Result_t SimApi_SendNumOfPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests PLMN entry contents in the PLMN file. 

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendReadPLMNEntryReq( ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, 
			UInt16 start_index, UInt16 end_index, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests to update multiple PLMN entries in the PLMN file.

	@param		inClientInfoPtr (in) pointer to the client Info
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
Result_t SimApi_SendWriteMulPLMNEntryReq( ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry,
    SIM_MUL_PLMN_ENTRY_t plmn_entry[], CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
	This function powers on/off SIM card. A MSG_SIM_POWER_ON_OFF_CARD_RSP message will be composed and passed
	to the callback function.

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		power_on (IN) True to power on, False to power off.
    @param      mode (IN) power on mode [0-Generic 1-Normal]. Applicable only if power_on is True.
    @param      sim_access_cb (IN) Registered event callback 
	@return		Result_t

**/	
Result_t SimApi_PowerOnOffCard(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function  retreives raw ATR from SIM driver. A MSG_SIM_GET_RAW_ATR_RSP message will be passed back
	to the callback function.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param      sim_access_cb (IN) Registered event callback 

	@return		Result_t
**/	
Result_t SimApi_GetRawAtr(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    Get the ATR data from SIMIO driver. This function is different from SIM_GetRawAtr() 
	in that it returns the ATR data synchronously.

	@param atr_data (out) Buffer to store the returned ATR data

	@return RESULT_OK if ATR data is returned successfully; SIMACCESS_NO_ACCESS or 
			SIMACCESS_NO_SIM otherwise
**/
SIMAccess_t SIM_GetAtrData(APDU_t *atr_data);


//***************************************************************************************
/**
	This function sends a generic APDU command to the SIM. A MSG_SIM_SEND_GENERIC_APDU_CMD_RSP message
	will be passed back to the callback function.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param		apdu (IN) APDU command data
    @param		len (IN) APDU command length
    @param      sim_access_cb (IN) Registered event callback   
	@return		Result_t
**/
Result_t SimApi_SendGenericApduCmd(ClientInfo_t* inClientInfoPtr, UInt8 *apdu, UInt16 len, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function powers on/off the SIM card. The calling task is blocked until SIM 
	power on/off operation is finished. 

    @param inClientInfoPtr (in) pointer to the client Info
	@param power_on (in) TRUE for power on; FALSE for power off
	@param mode (in) Mode for power on request; Ignored for power off request.

	@return
	SIMACCESS_ALREADY_POWERED_OFF - SIM is already powered off and a power off request is received.
	SIMACCESS_ALREADY_POWERED_ON - SIM is already powered on and a power on request is received.
	SIMACCESS_NO_ACCESS - Can not perform the request, e.g. a power off request is received during voice call.
	SIMACCESS_SUCCESS - SIM power on/off is successfully performed.
**/
SIMAccess_t SimApi_PowerOnOffSim(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
    Function to return whether a USIM application is supported in the inserted USIM. If 
	the application is supported, the AID data for the application is written to the 
	passed AID data buffer. The returned AID data can be passed to the SimApi_SendSelectAppiReq()
	function to activate the supported application.

    @param inClientInfoPtr (in) pointer to the client Info
    @param appli_type (in) USIM Application type
	@param pOutAidData (out) Pointer to AID data buffer

	@return		TRUE if the passed application type is supported; FALSE otherwise.
**/	
Boolean USimApi_IsApplicationSupported(ClientInfo_t* inClientInfoPtr, USIM_APPLICATION_TYPE appli_type, USIM_AID_DATA_t *pOutAidData);


//***************************************************************************************
/**
	This function sends the request to USIM to open a logical channel. It is called before a 
	non-USIM application is activated. It will trigger a MSG_SIM_OPEN_SOCKET_RSP message to 
	be passed back to the callback function.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SimApi_SendOpenSocketReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to activate a non-USIM application and associate it
	with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP 
	message to be passed back to the callback function.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		socket_id (IN) Socket ID representing a logical channel
	@param		aid_data (IN) AID data in one EF-DIR record for the application to be activated
	@param		aid_len (IN) AID data length
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SimApi_SendSelectAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to USIM to deactivate a non-USIM application associated
	with the passed socket ID representing a logical channel. The application must have been
	activated through SIM_SendSelectAppiReq() previously. It will trigger a MSG_SIM_DEACTIVATE_APPLI_RSP 
	message to be passed back to the callback function.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		socket_id (IN) Socket ID representing a logical channel and associated with the application
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SimApi_SendDeactivateAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to close the logical channel previously opened through
	SIM_SendOpenSocketReq() function. The application associated with the logical channel must be 
	deactivated through SIM_SendDeactivateAppiReq() function before this function is called. It will 
	trigger a MSG_SIM_CLOSE_SOCKET_RSP message to be passed back to the callback function.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		socket_id (IN) Socket ID representing a logical channel
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SimApi_SendCloseSocketReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to verify PIN1 or PIN2 password.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		chv (IN) PIN1/PIN2 password in ASCII format
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_VERIFY_CHV_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendVerifyChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select,	CHVString_t chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to change PIN1 or PIN2 password.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		old_chv (IN) Old PIN1/PIN2 password in ASCII format
	@param		new_chv (IN) New PIN1/PIN2 password in ASCII format
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_CHANGE_CHV_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendChangeChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable the SIM PIN1 feature.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		chv (IN) PIN1 password
	@param		enable_flag (IN) TRUE to enable SIM PIN1; FALSE to disable SIM PIN1
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ENABLE_CHV_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendSetChv1OnOffReq(ClientInfo_t* inClientInfoPtr, CHVString_t chv, Boolean enable_flag, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to unblock SIM PIN1 or PIN2.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		chv_select (IN) Select PIN1 or PIN2
	@param		puk (IN) PUK1 or PUK2 password
	@param		new_chv (IN) New PIN1 or PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_UNBLOCK_CHV_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendUnblockChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable FDN feature.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		oper_state (IN) SIMOPERSTATE_RESTRICTED_OPERATION to enable FDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable FDN
	@param		chv2 (IN) PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SET_FDN_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendSetOperStateReq(ClientInfo_t* inClientInfoPtr, SIMOperState_t oper_state, CHVString_t chv2, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the maximum Accumulated Call Meter value. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_MAX_ACM_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendReadAcmMaxReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the maximum Accumulated Call Meter value. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		acm_max (IN) Maximum Accumulated Call Meter value
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_MAX_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendWriteAcmMaxReq(ClientInfo_t* inClientInfoPtr,	CallMeterUnit_t acm_max, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Accumulated Call Meter value. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_VALUE_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendReadAcmReq(ClientInfo_t* inClientInfoPtr,	CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Accumulated Call Meter value. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		acm (IN) Accumulated Call Meter value
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendWriteAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to increase the Accumulated Call Meter value. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		acm (IN) Accumulated Call Meter value by which to increase the value in SIM
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_ACM_INCREASE_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendIncreaseAcmReq(ClientInfo_t* inClientInfoPtr,	CallMeterUnit_t acm, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Service Provider Name in EF-SPN

	@param		inClientInfoPtr (in) pointer to the client Info
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SVC_PROV_NAME_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendReadSvcProvNameReq(ClientInfo_t* inClientInfoPtr,	CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Price Per Unit and Currency information.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PUCT_DATA_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendReadPuctReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Price Per Unit and Currency information.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		currency (IN) Currency name in string format
	@param		eppu (IN) Price Per Unit data
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PUCT_UPDATE_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendWritePuctReq(ClientInfo_t* inClientInfoPtr, CurrencyName_t currency, EPPU_t *eppu, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends a Generic SIM Access command to the SIM.

	@param		inClientInfoPtr (in) pointer to the client Info

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
Result_t SimApi_SubmitRestrictedAccessReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id,	APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id,				
	UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data, CallbackFunc_t* sim_access_cb);
/** @} */

/**
 * @addtogroup USIMAPIGroup
 * @{
 */

//***************************************************************************************
/**
    Function to send a request to USIM to activate/deactivate a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP 
	message will be passed back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_SET_EST_SERV_RSP message. 

	@param		inClientInfoPtr (in) pointer to the client Info 
	@param		est_serv (in) Service type in EF-EST.  
	@param 		serv_on (in) TRUE to activate service; FALSE to deactivate service. 
	@param 		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  	
**/	
Result_t USimApi_SendSetEstServReq(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv, Boolean serv_on, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function first selects a file and then sends a APDU 
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		socketId  	(in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().
    
	@param		dfileId     (in) DF ID
	@param		efileId     (in) EF ID
	@param		pathLen     (in) Path length 
	@param		pPath       (in) Pointer to path to be selected
	@param		apduLen     (in) length of the APDU
	@param		pApdu       (in) Pointer to APDU
    @param		simAccessCb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_SEND_APDU_RSP message by calling
	the passed callback function.
**/	
//***************************************************************************************
Result_t SimApi_SubmitSelectFileSendApduReq(ClientInfo_t*   inClientInfoPtr,
                                            UInt8           socketId,
                                            APDUFileID_t    dfileId,
                                            APDUFileID_t    efileId, 
                                            UInt8           pathLen, 
                                            const UInt16*   pPath,  
                                            UInt16          apduLen,
                                            const UInt8*    pApdu,
                                            CallbackFunc_t* simAccessCb);

//***************************************************************************************
/**
	This function sends the request to SIM to activate a non-USIM application and associate it
	with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP 
	message to be passed back to the callback function.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		socket_id (IN) Socket ID representing a logical channel
	@param		aid_data (IN) AID data in one EF-DIR record for the application to be activated
	@param		aid_len (IN) AID data length
	@param		app_occur (IN) Occurence of application.
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SimApi_SendSelectApplicationReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, 
                                         const UInt8 *aid_data, UInt8 aid_len, 
                                         SIM_APP_OCCURRENCE_t app_occur, 
                                         CallbackFunc_t* sim_access_cb);


//************************************************************************************
/**
    Function Name:	NetRegApi_PerformSteeringOfRoaming

    @param inClientInfoPtr	(in) pointer to clientInfo of the calling client
	@param inPlmnListPtr    (in) PLMNwACT list
	@param inListLen        (in) PLMNwACT list length
	@param inRefreshType    (in) Refresh Type (Steering of Roaming=6, Steering of Roaming (WLAN)=7

    @return		Result_t
**/
Result_t SimApi_PerformSteeringOfRoaming(ClientInfo_t* inClientInfoPtr, 
										 SIM_MUL_PLMN_ENTRY_t* inPlmnListPtr,
										 UInt8 inListLen,
										 UInt8 inRefreshType);

//***************************************************************************************
/**
    This function returns whether SIM PIN needs to be entered to unlock the SIM.
    @param inClientInfoPtr	(in) pointer to clientInfo of the calling client

	@return TRUE if SIM PIN needs to be entered; FALSE otherwise
**/	
Boolean SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr); ///< TRUE if SIM PIN needs to be entered;


//***************************************************************************************
/**
    This function returns whether SIM PBK access is allowed?
    @param inClientInfoPtr	(in) pointer to clientInfo of the calling client
    @param id				(in) SIM Pbk ID

	@return TRUE if SIM PIN needs to be entered; FALSE otherwise
**/	
Boolean SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
    Get the ATR data from SIMIO driver. This function is different from SIM_GetRawAtr() 
	in that it returns the ATR data synchronously.

    @param inClientInfoPtr	(in) pointer to clientInfo of the calling client
	@param atr_data (out) Buffer to store the returned ATR data

	@return RESULT_OK if ATR data is returned successfully; SIMACCESS_NO_ACCESS or 
			SIMACCESS_NO_SIM otherwise
**/
SIMAccess_t SimApi_GetAtrData(ClientInfo_t* inClientInfoPtr, APDU_t *atr_data);


//***************************************************************************************
/**
    This function performs cold/warm SIM reset
    @param inClientInfoPtr	(in) pointer to clientInfo of the calling client
    @param resetMode		(in) reset mode (cold/warm: 0/1)

	@return TRUE if reset allowed; FALSE otherwise
**/	
Result_t SimApi_ResetSIM(ClientInfo_t* inClientInfoPtr, Boolean resetMode);

/** @} */

#endif  // _SIMAPI_H_
