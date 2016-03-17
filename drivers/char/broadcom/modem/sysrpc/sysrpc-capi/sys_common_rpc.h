/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/****************************************************************************
*
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )
*
****************************************************************************/
#ifndef SYS_COMMON_RPC_H
#define SYS_COMMON_RPC_H

#define _DBG_(a) a		/* by default logs are enabled */

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif

#ifdef WIN32
/* Windows */
#define SYS_TRACE	printf
extern UInt32 g_dwLogLEVEL;

#else
/* Target */
extern int Log_DebugPrintf(UInt16 logID, char *fmt, ...);
extern void Log_DebugOutputString(UInt16 logID, char *dbgString);
extern Boolean Log_IsLoggingEnable(UInt16 logID);

#define SYS_TRACE(...) Log_DebugPrintf(LOGID_MISC, __VA_ARGS__)

#endif

void SYS_GenGetPayloadInfo(void *dataBuf, MsgType_t msgType, void **ppBuf,
			   UInt32 *len);

UInt8 SYS_GetClientId(void);
void sysGetXdrStruct(RPC_XdrInfo_t **ptr, UInt16 *size);

typedef enum {
	SIMLDO1,
	SIMLDO2
} PMU_SIMLDO_t;

typedef enum {
	PMU_SIM3P0Volt = 0,
	PMU_SIM1P8Volt,
	PMU_SIM0P0Volt
} PMU_SIMVolt_t;

struct MtestOutput_t {
	Int32 res;
	UInt8 *data;
	UInt32 len;
};

typedef enum {
	/* SIMLOCK/PhoneLock disabled or password verified */
	SYS_SIM_SECURITY_OPEN,

	/* SIMLOCK/PhoneLock enabled & pending on password verification */
	SYS_SIM_SECURITY_LOCKED,

	/* SIMLOCK/PhoneLock blocked & may be unblocked if semi-permanent */
	SYS_SIM_SECURITY_BLOCKED,

	/* SIMLOCK/PhoneLock password has been verified */
	SYS_SIM_SECURITY_VERIFIED,

	/* SIMLOCK status not initialized yet */
	SYS_SIM_SECURITY_NOT_INIT
} SYS_SIM_SECURITY_STATE_t;

/* SIMLOCK status for all SIMLOCK types */
typedef struct {
	/* TRUE if network lock is enabled */
	UInt8 network_lock_enabled;

	/* TRUE if network subset lock is enabled */
	UInt8 network_subset_lock_enabled;

	/* TRUE if service provider lock is enabled */
	UInt8 service_provider_lock_enabled;

	/* TRUE if corporate lock is enabled */
	UInt8 corporate_lock_enabled;

	/* TRUE if phone lock is enabled */
	UInt8 phone_lock_enabled;

	/* Network Lock status */
	SYS_SIM_SECURITY_STATE_t network_lock;

	/* Network Subset Lock status */
	SYS_SIM_SECURITY_STATE_t network_subset_lock;

	/* Service Provider Lock status */
	SYS_SIM_SECURITY_STATE_t service_provider_lock;

	/* Corporate Lock status */
	SYS_SIM_SECURITY_STATE_t corporate_lock;

	/* Phone Lock status */
	SYS_SIM_SECURITY_STATE_t phone_lock;
} SYS_SIMLOCK_STATE_t;		/* SIMLOCK status structure */

#define SYS_GID_DIGITS	10	/* Max. GID1/GID2 file length */
#define SYS_IMSI_DIGITS	15	/* Max. IMSI digits */

/* GID1/GID2 file length (not null terminated) */
typedef UInt8 SYS_GID_DIGIT_t[SYS_GID_DIGITS];

/* NULL terminated IMSI string in ASCII format */
typedef UInt8 SYS_IMSI_t[SYS_IMSI_DIGITS + 1];

typedef struct {
	/* NULL terminated IMSI string */
	SYS_IMSI_t imsi_string;

	/* GID1 data */
	SYS_GID_DIGIT_t gid1;

	/* Number of bytes in "gid1" element, i.e. number of bytes in EF-GID1 */
	UInt8 gid1_len;

	/* GID2 data */
	SYS_GID_DIGIT_t gid2;

	/* Number of bytes in "gid2" element, i.e. number of bytes in EF-GID2 */
	UInt8 gid2_len;
} SYS_SIMLOCK_SIM_DATA_t;

/**
	AP System command types, for use with MSG_AP_SYS_CMD_REQ
**/
enum SYS_APSysCmd_t {
	AP_SYS_CMD_RFLDO,	/* Set RF LDO */
	AP_SYS_CMD_SIMLDO,	/* Set SIM LDO's */
	AP_SYS_CMD_SIM1LDO,	/* Set SIM1 LDO */
	AP_SYS_CMD_SIM2LDO,	/* Set SIM2 LDO */
	AP_SYS_CMD_TOTAL	/* total number of AP system commands */
};

enum SYS_LDO_Cmd_Type_t {
	SYS_LDO_OFF,
	SYS_LDO_ON
};

void SIMLOCKApi_SetStatus(ClientInfo_t *inClientInfoPtr,
			  SYS_SIMLOCK_STATE_t *simlock_state);
void SIMLOCKApi_SetStatusEx(UInt8 simId, SYS_SIMLOCK_STATE_t *simlock_state);
UInt32 SimApi_GetCurrLockedSimlockType(ClientInfo_t *inClientInfoPtr);

/***************** < 1 > **********************/

#endif
