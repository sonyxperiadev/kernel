/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
#ifndef _BCM_KRIL_SIMLOCK_H
#define _BCM_KRIL_SIMLOCK_H
   

//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------
#define  MAX_NETWORK_LOCKS      50
#define  MAX_NET_SUBSET_LOCKS   50
#define  MAX_PROVIDER_LOCKS     50
#define  MAX_CORPORATE_LOCKS    50
#define  MAX_PHONE_LOCKS        50

#define  MAX_CONTROL_KEY_LENGTH   16  // per GSM 02.22
#define  MAX_GID_FIELD_LENGTH     1 // per GSM02.22

#define  MAX_SIMLOCK_SIGNATURE_LEN  24


//-------------------------------------------------
// Data Structure
//-------------------------------------------------
typedef struct 
{
  UInt8 networkLockIndicator;   // 0=off; 1=on
  UInt8 netSubsetLockIndicator;   
  UInt8 providerLockIndicator;  
  UInt8 corpLockIndicator;
  UInt8 phoneLockIndicator;
  
} SIMLock_Indicator_t;


// Network personalization
typedef struct{
  UInt8 mcc[3];   // MCC
  UInt8 mnc[3];   // MNC

} SIMLock_NetworkCode_t;


// Network subset personalization
typedef struct{
  UInt8 mcc[3];       // MCC
  UInt8 mnc[3];       // MNC
  UInt8 imsi_6_and_7[2];  // IMSI digits 6 and 7

} SIMLock_NetSubSetCode_t;


// Service provider personalization
typedef struct{
  UInt8 mcc[3];           // MCC
  UInt8 mnc[3];           // MNC
  UInt8 gid1[MAX_GID_FIELD_LENGTH]; // GID1 file

} SIMLock_ProviderCode_t;


// Corporate personalization
typedef struct{
  UInt8 mcc[3];     // MCC
  UInt8 mnc[3];     // MNC
  UInt8 gid1[MAX_GID_FIELD_LENGTH]; // GID1 file - GSM02.22 pp20
  UInt8 gid2[MAX_GID_FIELD_LENGTH]; // GID2 file 

} SIMLock_CorporateCode_t;


// SIM personalization
typedef struct{
  UInt8 mcc[3];       // MCC
  UInt8 mnc[3];       // MNC
  UInt8 imsi_6_and_7[2];  // IMSI digits 6 and 7
  UInt8 imsi_8_to_15[8];  // IMSI digits 8 to 15

} SIMLock_PHONECode_t;


// SIM Lock data file (memory layout in flash)
typedef struct
{ 
  //SIMLock_Indicator_t should be always in the first place since
  //it is not encrypted
  SIMLock_Indicator_t indicator;

  UInt8   maxUnlockAttempt;
  Boolean   resetUnlockCounter;

  UInt8   simLockSign[MAX_SIMLOCK_SIGNATURE_LEN+1];
  // control keys are encrypted in ME (Acronym per GSM 02.22)
  UInt8   nck[MAX_CONTROL_KEY_LENGTH+1];  // network control keys
  UInt8   nsck[MAX_CONTROL_KEY_LENGTH+1]; // network subset control key
  UInt8   spck[MAX_CONTROL_KEY_LENGTH+1]; // provider control key 
  UInt8   cck[MAX_CONTROL_KEY_LENGTH+1];  // corporate control key
  UInt8   pck[MAX_CONTROL_KEY_LENGTH+1];  // personalization control key

  //network lock code num
  UInt8           numNetworkLocks;    
  //provider lock code num
  UInt8           numProviderLocks; 
  //network subset lock code num
  UInt8           numNetSubsetLocks;  
  //corporate lock code num
  UInt8           numCorpLocks; 
  //SIM lock code num
  UInt8           numPhoneLocks;  

  /* Check sum - calculated using three dividends:
   * 1. the data elements in this structure before "chksum". 
   * 2. The Simlock code groups, i.e. detailed Simlock info (MCC, MNC, GID1, GID2 etc), 
   *    which are stored immediately following this structure in the Flash.
   * 3. BCD coded IMEI number (14 digits).
   */
  UInt16            chksum;

} SIMLock_CodeFile_t;

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------

typedef enum
{
	PH_SIM_LOCK_OFF,	/* PH-SIM lock off */
	PH_SIM_LOCK_ON,		/* PH-SIM lock on, but do not need to enter password if the same SIM card */
	PH_SIM_FULL_LOCK_ON /* PH-SIM lock on, need to enter password every time upon power up */
} PH_SIM_Lock_Status_t;

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

// SIM lock indication
Boolean  SIMLockIsLockOn(SIMLockType_t lockType, Boolean *ps_full_lock_on);

// SIM lock verification
Boolean  SIMLockCheckAllLocks(UInt8* imsi, UInt8* gid1, UInt8* gid2);

// SIM lock unlock
SIMLock_Status_t  SIMLockUnlockSIM(SIMLockType_t lockType, UInt8* key);

// SIM lock set lock/unlock (indicator on/off)
SIMLock_Status_t  SIMLockSetLock(UInt8 action, Boolean ph_sim_full_lock_on, SIMLockType_t lockType,  
                 UInt8* key, UInt8* imsi, UInt8* gid1, UInt8* gid2);  

// Get the current closed lock type
SIMLockType_t SIMLockGetCurrentClosedLock(Boolean *lock_blocked);

// Change the password of the PH-SIM simlock.
SIMLock_Status_t SIMLockChangePasswordPHSIM(UInt8 *old_pwd, UInt8 *new_pwd);

// Check the validity of the password for the PH-SIM simlock.
Boolean SIMLockCheckPasswordPHSIM(UInt8 *pwd);

// Return the sim lock signature
void SIMLockGetSignature(UInt8* simLockSign, UInt8 simLockSignSize);

// Get Imei in Secure Bootloader
const UInt8* SIMLockGetImeiSecboot(void);

// Update the current SIM lock state
void SIMLockUpdateSIMLockState(void);

// Get the current SIM lock state
void SIMLockGetSIMLockState(SIMLOCK_STATE_t* simlock_state);

// Get the remain attempt
UInt8 SIMLockGetRemainAttempt(void);

#endif  // _BCM_KRIL_SIMLOCK_H

