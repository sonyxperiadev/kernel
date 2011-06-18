//***************************************************************************
//
//	Copyright © 2001-2008 Broadcom Corporation
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
*   @file   simlock_api.h
*
*   @brief  This file contains definitions for SIM Lock module.
*
*			This file defines the interface for SIM Lock API. This file 
*           provides the function prototypes necessary to check/verify/
*           lock/unlock different types of SIM Locks, and verify/change 
*           passwords for SIM Locks.
*
****************************************************************************/
/**
*   @defgroup   SIMLOCKOLDGroup   Legacy SIM Security definitions
*   @ingroup    SIMLOCKGroup
*   @brief      This group defines the legacy interfaces to the SIM Lock module 
*
	\n Use the link below to navigate back to the Subscriber Identity Module Overview page. \n
    \li \if CAPI2
	\ref CAPI2SIMOverview
	\endif
	\if CAPI
	\ref SIMOverview
	\endif
*****************************************************************************/

#ifndef _SIMLOCK_H_
#define _SIMLOCK_H_

//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------

/*
	Maximum number of code entries for simlocks
*/
#define  MAX_NETWORK_LOCKS			50  /**< Maximum number of network lock code entries */
#define  MAX_NET_SUBSET_LOCKS		50  /**< Maximum number of network subset lock code entries */
#define  MAX_PROVIDER_LOCKS			50  /**< Maximum number of service provider lock code entries */
#define  MAX_CORPORATE_LOCKS		50  /**< Maximum number of corporate lock code entires */
#define  MAX_PHONE_LOCKS			50  /**< Maximum number of phone lock code entries */

/*
	Length of simlock elements
*/
#define  MAX_CONTROL_KEY_LENGTH		16	/**< Maximum length of control key per GSM 02.22 */
#define  MAX_GID_FIELD_LENGTH		10	/**< Maximum length of GID per GSM 02.22 */
#define  MAX_SIMLOCK_SIGNATURE_LEN	24  /**< Maximum length of simlock signature */
#define  BCD_IMEI_LEN                8  /**< length of IMEI bytes */

/*
	Memory address
*/
#define  SIMLOCK_IMAGE_OFFSET       0x3f80000   ///< Beginning address of SIMLOCK image in NAND flash

typedef enum
{
	PH_SIM_LOCK_OFF,	/* PH-SIM lock off */
	PH_SIM_LOCK_ON,		/* PH-SIM lock on, but do not need to enter password if the same SIM card */
	PH_SIM_FULL_LOCK_ON /* PH-SIM lock on, need to enter password every time upon power up */
} PH_SIM_Lock_Status_t;

//-------------------------------------------------
// Data Structure
//-------------------------------------------------

/// Structure of simlock indicators
typedef struct 
{
	UInt8	networkLockIndicator;		///< Indicator of network lock: 0=off, 1=on
	UInt8	netSubsetLockIndicator;		///< Indicator of network subset lock: 0=off, 1=on
	UInt8	providerLockIndicator;	    ///< Indicator of service provider lock: 0=off, 1=on
	UInt8	corpLockIndicator;          ///< Indicator of corporate lock: 0=off, 1=on
	UInt8	phoneLockIndicator;         ///< Indicator of phone lock: 0=off, 1=on
	
} SIMLock_Indicator_t;

/// Structure of network lock code
typedef struct{
	UInt8	mcc[3];		///< MCC
	UInt8	mnc[3];		///< MNC

} SIMLock_NetworkCode_t;


/// Structure of network subset lock code
typedef struct{
	UInt8	mcc[3];				///< MCC
	UInt8	mnc[3];				///< MNC
	UInt8	imsi_6_and_7[2];	///< IMSI digits 6 and 7

} SIMLock_NetSubSetCode_t;


/// Structure of service provider lock code
typedef struct{
	UInt8	mcc[3];						///< MCC
	UInt8	mnc[3];						///< MNC
	UInt8	gid1[MAX_GID_FIELD_LENGTH];	///< GID1 file

} SIMLock_ProviderCode_t;


/// Structure of corporate lock code
typedef struct{
	UInt8	mcc[3];						///< MCC
	UInt8	mnc[3];						///< MNC
	UInt8	gid1[MAX_GID_FIELD_LENGTH]; ///< GID1 file - GSM02.22 pp20
	UInt8	gid2[MAX_GID_FIELD_LENGTH];	///< GID2 file 

} SIMLock_CorporateCode_t;


/// Structure of phone lock code
typedef struct{
	UInt8	mcc[3];				///< MCC
	UInt8	mnc[3];				///< MNC
	UInt8	imsi_6_and_7[2];	///< IMSI digits 6 and 7
	UInt8	imsi_8_to_15[8];	///< IMSI digits 8 to 15

} SIMLock_PHONECode_t;



/* push the current packing value and align SIMLock_CodeFile_t to 1 byte boundary */
#pragma pack ( push, 1 )

/// SIM Lock data file (memory layout in flash)
typedef struct
{	
	/** SIMLock_Indicator_t is always in the first place since it is not encrypted */
	SIMLock_Indicator_t	indicator;

	UInt8		maxUnlockAttempt;   /**< Maximum number of unlock attempts allowed */
	Boolean		resetUnlockCounter; /**< 0: permanent locks, 1:semi-permanent locks */

	UInt8		simLockSign[MAX_SIMLOCK_SIGNATURE_LEN+1];  /**< Signature of SIM Lock data */

	/** Control keys are encrypted in ME (Acronym per GSM 02.22) */
	UInt8		nck[MAX_CONTROL_KEY_LENGTH+1];  /**< Network control keys */
	UInt8		nsck[MAX_CONTROL_KEY_LENGTH+1];	/**< Network subset control key */
	UInt8		spck[MAX_CONTROL_KEY_LENGTH+1];	/**< Provider control key	*/
	UInt8		cck[MAX_CONTROL_KEY_LENGTH+1];	/**< Corporate control key */
	UInt8		pck[MAX_CONTROL_KEY_LENGTH+1];	/**< Personalization control key */

    /* Number of SIM Lock code */
	UInt8		numNetworkLocks;    /**< network lock code number	*/
	UInt8		numProviderLocks;	/**< provider lock code number */	
	UInt8		numNetSubsetLocks;	/**< network subset lock code number */	
	UInt8		numCorpLocks;	    /**< corporate lock code number */	
	UInt8		numPhoneLocks;	    /**< SIM lock code number */

	/** Check sum - calculated using three dividends:
	 * 1. the data elements in this structure before "chksum". 
	 * 2. The Simlock code groups, i.e. detailed Simlock info (MCC, MNC, GID1, GID2 etc), 
	 *    which are stored immediately following this structure in the Flash.
	 * 3. BCD coded IMEI number (14 digits).
	 */
	UInt16	    chksum;

} SIMLock_CodeFile_t;

/* pop the packing value */
#pragma pack (pop)


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

/**
 * @addtogroup SIMLOCKOLDGroup
 * @{
 */

//***************************************************************************************
/**
    This function checks if a lock personalization (indicator) is turned on.. 

	@param		lockType (in) network lock, provider lock, and etc.
	@param		ps_full_lock_on (in) pointer to PH-SIM full lock status	
	@return		Boolean
	@note
	For the return value: TRUE - the lock is on; FALSE - lock is off.

**/

Boolean  SIMLockIsLockOn(SIMLockType_t lockType, Boolean *ps_full_lock_on);


//***************************************************************************************
/**
    This function is called during powerup to check if SIM is valid for all the SIMLOCK types. 

	@param		imsi (in) IMSI (MCC and MNC), pass NULL if SIM not inserted
	@param		gid1 (in) GID1 file, pass NULL if SIM not inserted
	@param		gid2 (in) GID2 file, pass NULL if SIM not inserted
	@return		Boolean
	@note
	1. For the return value: TRUE - SIM valid for all active locks; FALSE - invalid SIM.
	2. The passed IMSI data must have the following format: 
       MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits). 

**/

Boolean  SIMLockCheckAllLocks(UInt8* imsi, UInt8* gid1, UInt8* gid2);


//***************************************************************************************
/**
    This function locks/unlocks desired SIM Lock. 

	@param		lockType (in) network lock, provider lock, and etc.
	@param		key (in) unlock password	
	@return		SIMLock_Status_t
	@note
	The return value shows lock/unlock status.

**/

SIMLock_Status_t  SIMLockUnlockSIM(SIMLockType_t lockType, UInt8* key);


//***************************************************************************************
/**
    This function disables/enables the desired SIM lock setting whose indicator is saved in FFS 

	@param		action (in) 1: lock SIM; 0 = unlock SIM
	@param		ph_sim_full_lock_on (in) whether PH-SIM full lock should be set on
    @param		lockType (in) SIM Lock type: network, network subset, ...
    @param		key (in) control key
	@param		imsi (in) IMSI (MCC and MNC)
    @param		gid1 (in) GID1 in SIM
	@param		gid2 (in) GID2 in SIM
	@return		SIMLock_Status_t
	@note
	The return value shows lock/unlock status.

**/

SIMLock_Status_t  SIMLockSetLock(UInt8 action, Boolean ph_sim_full_lock_on, SIMLockType_t lockType,  
								 UInt8* key, UInt8* imsi, UInt8* gid1, UInt8* gid2);  



//***************************************************************************************
/**
    This function return one of the closed lock type. If no lock type is closed, 
    SIMLOCK_INVALID_LOCK is returned. 

	@param		lock_blocked (in) a pointer to indicate if the lock is blocked	
	@return		SIMLockType_t
	@note
	The return value is the current closed lock type.

**/

SIMLockType_t SIMLockGetCurrentClosedLock(Boolean *lock_blocked);



//***************************************************************************************
/**
    This function gets the current sim pin status according to simlock type.

	@return		SIM_PIN_Status_t
	@note
	The return value is the SIM PIN status corresponding to the current locked SIMLOCK type 
	if there is a locked SIMLOCK type. "PIN_READY_STATUS" if there is no locked SIMLOCK type.

**/

SIM_PIN_Status_t SIMGetCurrLockedSimlockType(void);


//***************************************************************************************
/**
    This function changes the PH-SIM lock password.

	@param		old_pwd (in) the old ASCII coded PH-SIM password (null terminated)
	@param		new_pwd (in) the new ASCII coded PH-SIM password (null terminated)	
	@return		SIMLock_Status_t
	@note
	The return value: TRUE - if password is successfully changed, FALSE - otherwise

**/

SIMLock_Status_t SIMLockChangePasswordPHSIM(UInt8 *old_pwd, UInt8 *new_pwd);


//***************************************************************************************
/**
    This function checks the validity of the password for the PH-SIM simlock..

	@param		pwd (in) the ASCII coded PH-SIM password (null terminated)	
	@return		Boolean
	@note
	The return value: TRUE - if password is correct, FALSE - otherwise

**/

Boolean	SIMLockCheckPasswordPHSIM(UInt8 *pwd);


//***************************************************************************************
/**
    This function sets the SIM type.

	@param		is_testsim (in) whether current SIM is test SIM: 1: test SIM; 0 = normal SIM	
	@return		
	@note	

**/

void SIMLockSetSIMType(Boolean is_testsim);


//***************************************************************************************
/**
    This function returns the SimLock signature.

	@param		simLockSign (in) returned SimLock signature
	@param		simLockSignSize (in) size of the "simLockSign" buffer	
	@return		
	@note	

**/

void SIMLockGetSignature(UInt8* simLockSign, UInt8 simLockSignSize);


//***************************************************************************************
/**
    This function gets IMEI in Secure Bootloader.

	@return		UInt8*
	@note
	Return:	NULL if there is no Secure Bootloader; Otherwise address of 8-byte 
			IMEI (BCD encoded, first and last nibbles not used and set to 0)

**/

const UInt8* SIMLockGetImeiSecboot(void);

//***************************************************************************************
/**
	This function returns SIMLOCK status by getting the status from the client. 

	@param		simlock_state (out) SIMLOCK status structure
	@param		sim_data (in) SIM data to perform SIMLOCK status check, IMSI, GID1 & GID2
	@param		is_testsim (in) TRUE if a test SIM is inserted; FALSE otherwise
**/	
void SIMLOCK_GetStatus(SIMLOCK_STATE_t *simlock_state, const SIMLOCK_SIM_DATA_t *sim_data, Boolean is_testsim);

/** @} */

#endif  // _SIMLOCK_H_

