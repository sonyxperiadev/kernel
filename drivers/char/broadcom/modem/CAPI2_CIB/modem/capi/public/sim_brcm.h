//***************************************************************************
//
//	Copyright © 2007-2008 Broadcom Corporation
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
*   @file   sim_brcm.h
*
*   @brief  This file contains the function prototypes for the SIM/USIM API.
*
****************************************************************************/

#ifndef _SIM_BRCM_H_
#define _SIM_BRCM_H_



//******************************************************************************
//							Constant Definitions
//******************************************************************************

#define SIM_PBK_BDN_CMP_SZ 1

#define SIM_EXT_REC_NOT_EXIST 0xFF
#define SIM_CMP_REC_NOT_EXIST 0xFF
#define SIM_CMI_REC_NOT_EXIST 0xFF

/* Number of bytes used for storing EXT digits */
#define SIM_PBK_EXT_BCD_DIGIT_SIZE (SIM_EXT_DIGIT_NUM_OF_BYTES - 3)

/* Bit mask for EF-EXT1 and EF-EXT2 etc: see Section 10.5.10 of GSM 11.11 */
#define SIM_EXT_SUB_ADDR_BIT_MASK			0x01
#define SIM_EXT_ADDITIONAL_DATA_BIT_MASK	0x02


/* SW1 status byte value in SIM response: see Section 9.4.1 of GSM 11.11 */
#define SW1_NORMAL_END_OF_CMD	0x90	/**< Normal end of command */
#define SW1_GET_RESP_IND		0x9F	/**< SIM wants ME to send Get Response command to get response data */
#define SW1_PROACT_CMD_PENDING	0x91	/**< Normal end of command and proactive command pending */
#define SW1_SIM_BUSY            0x93    /**< SIM Busy */

/// GID type
typedef enum
{
	GID_NONE,
	GID_1,
	GID_2
} GID_t;

/// SIM file access
typedef enum
{
	SIMFILEACCESS_CHV1	= SI_CHV1,
	SIMFILEACCESS_CHV2	= SI_CHV2,
	SIMFILEACCESS_ALW	= SI_ALW,
	SIMFILEACCESS_RFU	= SI_RFU,
	SIMFILEACCESS_ADM	= SI_ADM,
	SIMFILEACCESS_NEV	= SI_NEV
} SIMFileAccess_t;


//******************************************************************************
//							Type Definitions
//******************************************************************************

#define SIMPBK_NAME			40	///< Max. phone book name
#define SIMPBK_DIGITS		40	///< Max. phone book number


typedef UInt8 SIMPBK_Name_t[SIMPBK_NAME]; ///< Alpha string associated with number
										
typedef UInt8 SIMPBK_Number_t[SIMPBK_DIGITS + 1]; ///< Phonebook number, NULL terminated


/// SIM phonebook entry 
typedef struct
{
	gsm_TON_t ton;						///< Type of number
	gsm_NPI_t npi;						///< Numbering plan identifier
	SIMPBK_Name_t name;					///< Alpha string associated with number
	SIMPBK_Number_t digits;				///< Phonebook number
	UInt8	ext_rec_index;				///< 0-based index of record in EF-EXT1, EF-EXT2 etc.
    UInt8 cmp;                 			///< Comparison Method Pointer
} SIMPBK_Entry_t;						///< SIM Phonebook entry


/// SIM Phonebook write result
typedef struct
{
	SIMAccess_t result;					///< SIM access result
	SIMPBK_ID_t pbk;					///< Phonebook
	UInt16		index;					///< Write index
	SIMPBK_Entry_t	simpbk_entry;		///< Phonebook contents written
	void		*additional_pbk_cb;		///< Additional phonebook callback function
} SIM_PBK_WRITE_RESULT_t;

/// SIM Phonebook Data
typedef struct
{
	SIMAccess_t		result;				///< SIM access result
	SIMPBK_ID_t		pbk;				///< Phonebook
	UInt16			index;				///< Current record index
	UInt16			end_index;			///< Index of the last record to read
	SIMPBK_Entry_t	buffer;				///< Phonebook contents
	void			*additional_pbk_cb;	///< Additional phonebook callback function
} SIM_PBK_DATA_t;


 
typedef UInt16  CBMesgId_t;   ///< Cell Broadcast Message Id.


#define SMSPARAM_PAR_IND_SZ		1
#define SMSPARAM_TP_DA_SZ		12

#define SMSPARAM_TP_PROTO_SZ	1
#define SMSPARAM_TP_DCS_SZ		1
#define SMSPARAM_TP_VP_SZ		1
#define	SMSPARAM_DATA_SZ (SMSPARAM_PAR_IND_SZ + SMSPARAM_TP_DA_SZ + SMSPARAM_SCA_SZ + SMSPARAM_TP_PROTO_SZ + SMSPARAM_TP_DCS_SZ + SMSPARAM_TP_VP_SZ)


/* Macros to get remaining PIN1/PIN2/PUK1/PUK2 attempts in the response of 
 * Selecting a MF/DF SIM file. "ptr" points to the response data.
 */
#define GET_PIN1_ATTEMPT(ptr)  ((ptr)[18] & 0x0F)	
#define GET_PUK1_ATTEMPT(ptr)  ((ptr)[19] & 0x0F)
#define GET_PIN2_ATTEMPT(ptr)  ((ptr)[20] & 0x0F)
#define GET_PUK2_ATTEMPT(ptr)  ((ptr)[21] & 0x0F)


#define PAR_INDICATOR 0
#define PAR_TPDA      (PAR_INDICATOR + SMSPARAM_PAR_IND_SZ)
#define PAR_TPSCA     (PAR_TPDA + SMSPARAM_TP_DA_SZ)
#define PAR_PID       (PAR_TPSCA + SMSPARAM_SCA_SZ)
#define PAR_DCS       (PAR_PID + SMSPARAM_TP_PROTO_SZ)
#define PAR_VP        (PAR_DCS + SMSPARAM_TP_DCS_SZ)


#define PAR_TPDA_MSK  1
#define PAR_TPSCA_MSK 2
#define PAR_PID_MSK   4
#define PAR_DCS_MSK   8
#define PAR_VP_MSK    0x10



/// SIM SMS Parameters
typedef struct
{	
	// See GSM 11.11, Section 10.3.6
	SIMPBK_Name_t		name;							///< SMS Parameter Name
	UInt8				param_data[SMSPARAM_DATA_SZ];	///< SMS Parameter data
} SIMSMSParam_t;


typedef enum
{
	SIMFILETYPE_TRANSPARENT = SI_TRANSPARENT,
	SIMFILETYPE_LINEAR = SI_LINEAR_FIXED,
	SIMFILETYPE_CYCLIC = SI_CYCLIC
} SIMFileType_t;


/* Note: Do not change the order of the following enum. The order is associated with
 *       the coding in function SIM_GetServiceStatus() in sim_mi.c.
 */

/// SIM general service status
typedef struct
{
	Boolean sim_general_service_ready;	///< TRUE if SIM General Services Ready
	Boolean first_time_powerup;			///< TRUE if we power up first time
} SIMGeneralServiceStatus_t;

/// SIM status
typedef struct
{
	Boolean chv1_verified;		///< TRUE if CHV1 has just been verified
	Boolean first_time_powerup; ///< TRUE if we power up first time 
} SIMStatus_t;

/// SIM SMS TP MR
typedef struct 
{
	SIMAccess_t result;	///< SIM access result
	UInt8		tp_mr;	///< SMS reference number
} SIM_SMS_TP_MR_t;

/// SIM SMS parameter
typedef struct
{
	SIMAccess_t result;			///< SIM access result
	UInt16		rec_no;			///< Record number found, range 0 to n-1
	SIMSMSParam_t sms_param;	///< SMS Parameter
} SIM_SMS_PARAM_t;

/// Selection path information for a SIM/USIM file
typedef struct
{
	APDUFileID_t	file_id;
	UInt16			parent_id;
	UInt8			path_len;
	const UInt16	*select_path;
} SIM_FILE_PATH_INFO_t;

/// SIM PIN1 info
typedef struct
{
    Boolean chv_reverify_req;      ///< TRUE if PIN1 re-verification required
    Boolean generic_apdu_history;  ///< TRUE if Generic APDU mode has been entered before
} CHV1Info_t;


//******************************************************************************
//							GSM File Access
//******************************************************************************

//***************************************************************************************
/**
    This function sends the request to read a SMS record in SIM. A MSG_SIM_SMS_DATA_RSP  
	message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param rec_no (in) 0-based index
	@param sim_access_cb (in) Callback function.  

	@return		RESULT_OK.  
**/	
Result_t SIM_SendReadSmsReq(UInt8 clientID,	UInt16 rec_no, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the request to read a record in EF-SMSP. A MSG_SIM_SMS_PARAM_DATA_RSP
	message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param rec_no (in) 0-based index
	@param sim_access_cb (in) Callback function.  

	@return		RESULT_OK.  
**/	
Result_t SIM_SendReadSmsParamReq(UInt8 clientID, UInt16 rec_no, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the request to get a new SMS reference number from the SIM. A 
	MSG_SIM_SMS_TP_MR_RSP message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param sim_access_cb (in) Callback function.  

	@return		RESULT_OK.  
**/
Result_t SIM_SendSmsRefNumReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the request to write a SMS record in the SIM. A MSG_SIM_SMS_WRITE_RSP 
	message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param rec_no (in) 0-based index
	@param p_sms_mesg (in) SMS message contents
	@param sim_access_cb (in) Callback function.  

	@return		RESULT_OK.  
**/
Result_t SIM_SendWriteSmsReq(UInt8 clientID, UInt16 rec_no, SIMSMSMesg_t *p_sms_mesg, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the request to update the status of a SMS record in the SIM. 
	A MSG_SIM_SMS_STATUS_UPD_RSP message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param rec_no (in) 0-based index
	@param status_value (in) SMS status byte
	@param sim_access_cb (in) Callback function.  

	@return		RESULT_OK.  
**/
Result_t SIM_SendWriteSmsStatusReq(UInt8 clientID, UInt16 rec_no, SIMSMSMesgStatus_t status_value, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the request to update the Service Center Number in EF-SMSP record. 
	A MSG_SIM_SMS_SCA_UPD_RSP message will be returned to the passed callback function.

	@param clientID (in) Client ID of the calling task
	@param rec_no (in) 0-based index
	@param sca_data (in) SMS Service Center Number
	@param sim_access_cb (in) Callback function.  

	@return	RESULT_OK.  
**/
Result_t SIM_SendWriteSmsScaReq(UInt8 clientID, UInt8 rec_no, SIM_SCA_DATA_t *sca_data, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function posts the MSG_SIM_STATUS_IND message to upper layers in platform to 
	indicate the SIM inserted/removed status with the lauguage information in SIM.

	@param chv1_verified (in) TRUE if CHV1 has been verified
	@param first_time_powerup (in) TRUE if we are powering up for the first SIM detection 
		   and need to send GMMREG_ATTACH_REQ to stack
**/
void SIM_PostStatusInd(Boolean chv1_verified, Boolean first_time_powerup);


//***************************************************************************************
/**
    This function posts the MSG_PBK_INIT_SIM_DATA_REQ message to phonebk task so that we 
	start initializing SMS data in the phonebook task. 
**/
void SIM_PostInitSmsInd(void);


//***************************************************************************************
/**
    This function posts the MSG_SIM_PROC_GENERAL_SERVICE_IND message to SIM initialization 
	task so that we start to read generic SIM data in platform. 

	@param sim_general_service_ready (in) TRUE if SIM PIN is not locked or blocked; FALSE otherwise
	@param first_time_powerup (in) TRUE if we are powering up for the first SIM detection 
		   and need to send GMMREG_ATTACH_REQ to stack
**/
void SIM_PostGeneralServiceInd(Boolean sim_general_service_ready, Boolean first_time_powerup);


//***************************************************************************************
/**
    This function posts the MSG_SIM_ACM_UPDATE_IND message to MSC task (then relayed to MN task)
	so that the MN task uses the new ACM value for AOC checking. 
**/
void SIM_PostAcmUpdateInd(void);


//***************************************************************************************
/**
    This function posts the MSG_SIM_SMS_MEM_AVAIL_IND message to MSC task so that 
	the SMS memory available/unavailable status is updated with the network. 

    @param avail (in) TRUE if SMS memory is available in SIM; FALSE otherwise
**/
void SIM_PostSmsMemAvailInd(Boolean avail);


//***************************************************************************************
/**
    This function posts the MSG_SIM_CACHED_DATA_READY_IND message to upper layer so that
	the client can begin reading SIM data.
**/
void SIM_PostCachedDataReadyInd(void);


#endif

