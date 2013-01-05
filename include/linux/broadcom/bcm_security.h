/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/****************************************************************************/
/**
*
*  @file   bcm_security.h
*
*  @brief  Interface to the Broadcom security kernel driver.
*
*  @Note   The file must match with bcm_security.h in user space
*
****************************************************************************/

#ifndef __BCM_SECURITY_H
#define __BCM_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

/* an 8-bit integer selected to be specific to this driver */
#define BCM_SEC_IOC_MAGIC   0xF1
#define BCM_SEC_NAME  "bcm_security"

/**
 *
 *  ioctl commands
 *
 **/
#define SEC_SIMLOCK_IS_LOCK_ON_IOC     _IOWR(BCM_SEC_IOC_MAGIC, 1, \
			sec_simlock_islockon_t)
#define SEC_SIMLOCK_SET_LOCK_IOC       _IOWR(BCM_SEC_IOC_MAGIC, 2, \
			sec_simlock_set_lock_t)
#define SEC_SIMLOCK_UNLOCK_SIM_IOC     _IOWR(BCM_SEC_IOC_MAGIC, 3, \
			sec_simlock_unlock_t)
#define SEC_SIMLOCK_GET_LOCK_STATE_IOC     _IOR(BCM_SEC_IOC_MAGIC, 4, \
			sec_simlock_state_t)
#define SEC_SIMLOCK_GET_REMAIN_ATTMPT_IOC  _IOR(BCM_SEC_IOC_MAGIC, 5, \
			sec_simlock_remain_t)
#define SEC_GET_IMEI_IOC	           _IOR(BCM_SEC_IOC_MAGIC, 6, \
			sec_get_imei_data_t)
#define SEC_SIMLOCK_SET_LOCK_STATE_IOC	_IOR(BCM_SEC_IOC_MAGIC, 7, \
			sec_simlock_state_t)

#define SEC_HDCP_AES_IOC    _IOWR(BCM_SEC_IOC_MAGIC, 8, \
						sec_hdcp_aes_data_t)

#define SEC_MEM_ALLOC       _IOW(BCM_SEC_IOC_MAGIC, 9, unsigned long)

#define SEC_SIMLOCK_IOC_MAXNR			    10

#define MAX_IMEI_DIGITS 15
#define MAX_IMSI_DIGITS 15
#define MAX_GID_DIGITS 10
#define CK_MAX_LENGTH  16
#define MAX_HDCP_DIGITS 320

enum _SEC_SimLock_SimNumber_t {
	SEC_SimLock_SIM_SINGLE,
	SEC_SimLock_SIM_DUAL_FIRST,
	SEC_SimLock_SIM_DUAL_SECOND,
	SEC_SimLock_SIM_ALL = 0xFF
};
#define SEC_SimLock_SimNumber_t enum _SEC_SimLock_SimNumber_t

enum _SEC_SimLock_LockType_t {
	SEC_SIMLOCK_NETWORK_LOCK = 0,
	SEC_SIMLOCK_NET_SUBSET_LOCK,
	SEC_SIMLOCK_PROVIDER_LOCK,
	SEC_SIMLOCK_CORP_LOCK,
	SEC_SIMLOCK_PHONE_LOCK,

	SEC_SIMLOCK_INVALID_LOCK
};
#define SEC_SimLock_LockType_t enum _SEC_SimLock_LockType_t

enum _SEC_SimLock_Status_t {
	SEC_SIMLOCK_SUCCESS,	/* Lock successfully unlocked */
	SEC_SIMLOCK_FAILURE,	/* Lock not unlocked, but can try again */
	SEC_SIMLOCK_PERMANENTLY_LOCKED,	/* Permanent locked     */
	SEC_SIMLOCK_WRONG_KEY,	/* Unlock password is wrong, can retry */
	SEC_SIMLOCK_NOT_ALLOWED,	/* Unlock not allowed */
	SEC_SIMLOCK_DISABLED,	/* Unlock disabled */

	SEC_SIMLOCK_INVALID_STATUS	/* Invalid status of SIMLOCK */
};
#define SEC_SimLock_Status_t enum _SEC_SimLock_Status_t

enum _SEC_SimLock_Security_State_t {
	SEC_SIMLOCK_SECURITY_OPEN,	/* disabled or password verified */
	SEC_SIMLOCK_SECURITY_LOCKED,	/* enabled/pending on pwd verif */
	SEC_SIMLOCK_SECURITY_BLOCKED,	/* blocked & may be unblocked  */
	SEC_SIMLOCK_SECURITY_VERIFIED,	/* password has been verified */
	SEC_SIMLOCK_SECURITY_NOT_INIT	/* SIMLOCK status not initialized yet */
};
#define SEC_SimLock_Security_State_t enum _SEC_SimLock_Security_State_t

struct _sec_simlock_state_t {
	SEC_SimLock_SimNumber_t sim_id;
	int network_lock_enabled;
	int network_subset_lock_enabled;
	int service_provider_lock_enabled;
	int corporate_lock_enabled;
	int phone_lock_enabled;

	SEC_SimLock_Security_State_t network_lock;
	SEC_SimLock_Security_State_t network_subset_lock;
	SEC_SimLock_Security_State_t service_provider_lock;
	SEC_SimLock_Security_State_t corporate_lock;
	SEC_SimLock_Security_State_t phone_lock;
};
#define sec_simlock_state_t struct _sec_simlock_state_t

struct _sec_simlock_unlock_t {
	SEC_SimLock_SimNumber_t sim_id;
	SEC_SimLock_LockType_t lock_type;   /* type of lock to be unlocked */
	char password[CK_MAX_LENGTH + 1];  /* unlock password */
	SEC_SimLock_Status_t unlock_status; /* result of unlock operation */
	int remain_attempt;
};
#define sec_simlock_unlock_t struct _sec_simlock_unlock_t

struct _sec_simlock_islockon_t {
	SEC_SimLock_SimNumber_t sim_id;
	SEC_SimLock_LockType_t lock_type; /* type of lock to be checked */
	int lock_enabled;	/* 1 if lock is enabled, 0 otherwise */
	int full_lock_enabled;	/* 1 if full lock is enabled, 0 otherwise */
	/* (only valid when checking SEC_SIMLOCK_PHONE_LOCK */
};
#define sec_simlock_islockon_t struct _sec_simlock_islockon_t

struct _sec_simlock_set_lock_t {
	SEC_SimLock_SimNumber_t sim_id;
	int action;	/* 1: lock SIM; 0 = unlock SIM */
	int full_lock_on; /* 1:PH-SIM full lock should */
			  /* be enabled 0 otherwise */
	SEC_SimLock_LockType_t lock_type;	/* type of lock to be set */
	char key[CK_MAX_LENGTH + 1];	/* control key */
	SEC_SimLock_Status_t set_lock_status;	/* result of set lock op */
	int remain_attempt;
};
#define sec_simlock_set_lock_t struct _sec_simlock_set_lock_t

struct _sec_simlock_remain_t {
	SEC_SimLock_SimNumber_t sim_id;
	int remain_attempt;
};
#define sec_simlock_remain_t struct _sec_simlock_remain_t

struct _sec_simlock_sim_data_t {
	char imsi_string[MAX_IMSI_DIGITS + 1];	/* NULL term'd IMSI string */
	char gid1[MAX_GID_DIGITS];	/* GID1 data */
	int gid1_len;	/* bytes in "gid1" element */
	char gid2[MAX_GID_DIGITS];	/* GID2 data */
	int gid2_len;	/* bytes in "gid2" element */
};
#define sec_simlock_sim_data_t struct _sec_simlock_sim_data_t

struct _sec_get_imei_data_t {
	char imei1_string[MAX_IMEI_DIGITS + 1]; /* NULL term'd IMEI string */
	char imei2_string[MAX_IMEI_DIGITS + 1]; /* NULL term'd IMEI string */
};

#define sec_get_imei_data_t struct _sec_get_imei_data_t

struct _sec_hdcp_aes_data_t {
	char hdcp_in_string[MAX_HDCP_DIGITS];
	char hdcp_out_string[MAX_HDCP_DIGITS];
	int hdcp_string_len;	/* bytes in HDCP element */
	int hdcp_enc_dec;	/* 1: encryption; 0: decryption */
	int hdcp_wrapping_key;	/* 1: wrapping key; 0: random key */
};
#define sec_hdcp_aes_data_t struct _sec_hdcp_aes_data_t

int sec_simlock_get_status(sec_simlock_sim_data_t *sim_data,
				SEC_SimLock_SimNumber_t simID,
				int is_test_sim,
				sec_simlock_state_t *sim_lock_state);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* __BCM_SECURITY_H */
