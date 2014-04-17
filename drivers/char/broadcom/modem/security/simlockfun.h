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
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*	consent.
*
****************************************************************************/
#ifndef _BCM_KRIL_SIMLOCK_H
#define _BCM_KRIL_SIMLOCK_H

/*------------------------------*/
/* Constant Definitions			*/
/*------------------------------*/
#define  MAX_NETWORK_LOCKS      50
#define  MAX_NET_SUBSET_LOCKS   50
#define  MAX_PROVIDER_LOCKS     50
#define  MAX_CORPORATE_LOCKS    50
#define  MAX_PHONE_LOCKS        50

#define  MAX_CONTROL_KEY_LENGTH   16	/* per GSM 02.22 */
#define  MAX_GID_FIELD_LENGTH     1	/* per GSM02.22 */

#define  MAX_SIMLOCK_SIGNATURE_LEN  24

#define  MCC_MNC_CODE_SIZE  6

#define DUAL_SIM_SIZE  (SIM_DUAL_SECOND + 1)

/* Number of IMEI digits (14 + check digit)*/
#define IMEI_DIGITS	15

/* Number of IMEI MMAC digits */
#define IMEI_MAC_DIGITS	8

/* BCD coded IMEI length: 7 bytes for 14 digits,
 *but first nibble and last nibble are not used (set to 0)
 * in our system parameter convention. So total 8 bytes. */
#define BCD_IMEI_LEN  8

/*------------------------------*/
/* Data Structure				*/
/*------------------------------*/
struct _SIMLock_Indicator_t {
	UInt8 networkLockIndicator;	/* 0=off; 1=on */
	UInt8 netSubsetLockIndicator;
	UInt8 providerLockIndicator;
	UInt8 corpLockIndicator;
	UInt8 phoneLockIndicator;
};
#define SIMLock_Indicator_t struct _SIMLock_Indicator_t

/* Network personalization */
struct _SIMLock_NetworkCode_t {
	UInt8 mcc[3];		/* MCC */
	UInt8 mnc[3];		/* MNC */
};
#define SIMLock_NetworkCode_t struct _SIMLock_NetworkCode_t

/* Network subset personalization */
struct _SIMLock_NetSubSetCode_t {
	UInt8 mcc[3];		/* MCC */
	UInt8 mnc[3];		/* MNC */
	UInt8 imsi_6_and_7[2];	/* IMSI digits 6 and 7 */
};
#define SIMLock_NetSubSetCode_t struct _SIMLock_NetSubSetCode_t

/* Service provider personalization */
struct _SIMLock_ProviderCode_t {
	UInt8 mcc[3];		/* MCC */
	UInt8 mnc[3];		/* MNC */
	UInt8 gid1[MAX_GID_FIELD_LENGTH];	/* GID1 file */
};
#define SIMLock_ProviderCode_t struct _SIMLock_ProviderCode_t

/* Corporate personalization */
struct _SIMLock_CorporateCode_t {
	UInt8 mcc[3];		/* MCC */
	UInt8 mnc[3];		/* MNC */
	UInt8 gid1[MAX_GID_FIELD_LENGTH];	/* GID1 file - GSM02.22 pp20 */
	UInt8 gid2[MAX_GID_FIELD_LENGTH];	/* GID2 file  */
};
#define SIMLock_CorporateCode_t struct _SIMLock_CorporateCode_t

/* SIM personalization */
struct _SIMLock_PHONECode_t {
	UInt8 mcc[3];		/* MCC */
	UInt8 mnc[3];		/* MNC */
	UInt8 imsi_6_and_7[2];	/* IMSI digits 6 and 7 */
	UInt8 imsi_8_to_15[8];	/* IMSI digits 8 to 15 */

} ;
#define SIMLock_PHONECode_t struct _SIMLock_PHONECode_t

/* SIM Lock data file (memory layout in flash) */
struct _SIMLock_CodeFile_t {
	/*SIMLock_Indicator_t should be always in the first place since */
	/*it is not encrypted */
	SIMLock_Indicator_t indicator;

	UInt8 maxUnlockAttempt;
	Boolean resetUnlockCounter;

	UInt8 simLockSign[MAX_SIMLOCK_SIGNATURE_LEN + 1];
	/* control keys are encrypted in ME (Acronym per GSM 02.22) */
	/* network control keys */
	UInt8 nck[MAX_CONTROL_KEY_LENGTH + 1];
	/* network subset control key */
	UInt8 nsck[MAX_CONTROL_KEY_LENGTH + 1];
	/* provider control key  */
	UInt8 spck[MAX_CONTROL_KEY_LENGTH + 1];
	/* corporate control key */
	UInt8 cck[MAX_CONTROL_KEY_LENGTH + 1];
	/* personalization control key */
	UInt8 pck[MAX_CONTROL_KEY_LENGTH + 1];

	/*network lock code num */
	UInt8 numNetworkLocks;
	/*provider lock code num */
	UInt8 numProviderLocks;
	/*network subset lock code num */
	UInt8 numNetSubsetLocks;
	/*corporate lock code num */
	UInt8 numCorpLocks;
	/*SIM lock code num */
	UInt8 numPhoneLocks;

	/* Check sum - calculated using three dividends:
	 * 1. the data elements in this structure before "chksum".
	 * 2. The Simlock code groups, i.e. detailed Simlock info
	 *    (MCC, MNC, GID1, GID2 etc), which are stored immediately
	 *    following this structure in the Flash.
	 * 3. BCD coded IMEI number (14 digits).
	 */
	UInt16 chksum;

};
#define SIMLock_CodeFile_t struct _SIMLock_CodeFile_t

/*--------------------------*/
/* enum						*/
/*--------------------------*/

enum _PH_SIM_Lock_Status_t {
	PH_SIM_LOCK_OFF,	/* PH-SIM lock off */
	PH_SIM_LOCK_ON,		/* PH-SIM lock on, but do not need to */
				/* enter password if the same SIM card */
	PH_SIM_FULL_LOCK_ON	/* PH-SIM lock on, need to enter password */
				/* every time upon power up */
};
#define PH_SIM_Lock_Status_t enum _PH_SIM_Lock_Status_t

/*--------------------------*/
/* Function Prototype		*/
/*--------------------------*/

/* SIM lock indication */
Boolean SIMLockIsLockOn(SEC_SimLock_LockType_t lockType,
			Boolean *ps_full_lock_on);

/* SIM lock verification */
Boolean SIMLockCheckAllLocks(SimNumber_t SimId, UInt8 *imsi, UInt8 *gid1,
			     UInt8 *gid2);

/* SIM lock unlock */
SEC_SimLock_Status_t SIMLockUnlockSIM(SimNumber_t SimId,
				      SEC_SimLock_LockType_t lockType,
				      UInt8 *key);

/* SIM lock set lock/unlock (indicator on/off) */
SEC_SimLock_Status_t SIMLockSetLock(SimNumber_t SimId,
				    UInt8 action,
				    Boolean ph_sim_full_lock_on,
				    SEC_SimLock_LockType_t lockType,
				    UInt8 *key);

/* Get the current closed lock type */
SEC_SimLock_LockType_t SIMLockGetCurrentClosedLock(SimNumber_t SimId,
						   Boolean *lock_blocked);

/* Change the password of the PH-SIM simlock. */
SEC_SimLock_Status_t SIMLockChangePasswordPHSIM(UInt8 *old_pwd,
						UInt8 *new_pwd);

/* Check the validity of the password for the PH-SIM simlock. */
Boolean SIMLockCheckPasswordPHSIM(UInt8 *pwd);

/* Return the sim lock signature */
void SIMLockGetSignature(UInt8 *simLockSign, UInt8 simLockSignSize);

/* Get Imei in Secure Bootloader */
const UInt8 *SIMLockGetImeiSecboot(void);

/* Update the current SIM lock state */
void SIMLockUpdateSIMLockState(SimNumber_t SimId);

/* Get the current SIM lock state */
void SIMLockGetSIMLockState(SimNumber_t SimId,
			    sec_simlock_state_t *simlock_state);

/* Get the remain attempt */
UInt8 SIMLockGetRemainAttempt(SimNumber_t SimId);

/* Get SIM Lock inforamtion */
UInt16 SIMLockGetLockCodeInform(SEC_SimLock_LockType_t lockType,
				UInt8 *lockinfo, UInt16 len);

/* set current SIM data, based on info received from CP */
void SIMLockSetSimData(sec_simlock_sim_data_t *simData, SimNumber_t simID);

/*store IMEI*/
Boolean SetImeiData(SimNumber_t SimId, UInt8 *imeiStr);

#endif /* _BCM_KRIL_SIMLOCK_H */
