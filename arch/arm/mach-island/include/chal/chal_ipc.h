/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _IPC_H_
#define _IPC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* include file declarations */
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/bcmerr.h>

/******************************************************************************/
/* macro declarations */
#define IPC_STATUS_SYNC_MASK   (( 1uL << 31 ) | ( 1uL << 15 ))
#define IPC_STATUS_SYNC_VAL    (( 1uL << 31 ) | ( 0uL << 15 ))

/******************************************************************************/
/* typedef declarations */

typedef void* CHAL_IPC_HANDLE;

typedef enum {
    IPC_MAILBOX_ID_0 = 0,
    IPC_MAILBOX_ID_1 = 1,
    IPC_MAILBOX_ID_2 = 2,
    IPC_MAILBOX_ID_3 = 3,
    IPC_MAILBOX_ID_MAX
} IPC_MAILBOX_ID;

typedef enum
{
   IPC_INTERRUPT_SOURCE_0 = 0,
   IPC_INTERRUPT_SOURCE_1,
   IPC_INTERRUPT_SOURCE_2,
   IPC_INTERRUPT_SOURCE_3,
   IPC_INTERRUPT_SOURCE_4,
   IPC_INTERRUPT_SOURCE_5,
   IPC_INTERRUPT_SOURCE_6,
   IPC_INTERRUPT_SOURCE_7,
   IPC_INTERRUPT_SOURCE_8,
   IPC_INTERRUPT_SOURCE_9,
   IPC_INTERRUPT_SOURCE_10,
   IPC_INTERRUPT_SOURCE_11,
   IPC_INTERRUPT_SOURCE_12,
   IPC_INTERRUPT_SOURCE_13,
   IPC_INTERRUPT_SOURCE_14,
   IPC_INTERRUPT_SOURCE_15,
   IPC_INTERRUPT_SOURCE_16,
   IPC_INTERRUPT_SOURCE_17,
   IPC_INTERRUPT_SOURCE_18,
   IPC_INTERRUPT_SOURCE_19,
   IPC_INTERRUPT_SOURCE_20,
   IPC_INTERRUPT_SOURCE_21,
   IPC_INTERRUPT_SOURCE_22,
   IPC_INTERRUPT_SOURCE_23,
   IPC_INTERRUPT_SOURCE_24,
   IPC_INTERRUPT_SOURCE_25,
   IPC_INTERRUPT_SOURCE_26,
   IPC_INTERRUPT_SOURCE_27,
   IPC_INTERRUPT_SOURCE_28,
   IPC_INTERRUPT_SOURCE_29,
   IPC_INTERRUPT_SOURCE_30,
   IPC_INTERRUPT_SOURCE_31,
   IPC_INTERRUPT_SOURCE_NULL = 32,
   IPC_INTERRUPT_SOURCE_MAX = 32
} IPC_INTERRUPT_SOURCE;


typedef enum {
    IPC_INTERRUPT_MODE_OPEN = 0,
    IPC_INTERRUPT_MODE_SECURE = 1
} IPC_INTERRUPT_MODE;

typedef enum {
    IPC_INTERRUPT_STATUS_DISABLED = 0,
    IPC_INTERRUPT_STATUS_ENABLED = 1
} IPC_INTERRUPT_STATUS;

typedef struct CHAL_IPC_CONFIG_T {
    void*     recieveCallback;
} CHAL_IPC_CONFIG_T;


/******************************************************************************/
/* function declarations */

CHAL_IPC_HANDLE chal_ipc_config (
   CHAL_IPC_CONFIG_T *pConfig
   );

BCM_ERR_CODE chal_ipc_write_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t value
    );

BCM_ERR_CODE chal_ipc_read_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t *value
    );

BCM_ERR_CODE chal_ipc_query_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t *result
    );

BCM_ERR_CODE chal_ipc_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t address
    );

BCM_ERR_CODE chal_ipc_sleep_vc (
    CHAL_IPC_HANDLE handle
    );

BCM_ERR_CODE chal_ipc_int_vcset (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    );

BCM_ERR_CODE chal_ipc_int_clr (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    );

BCM_ERR_CODE chal_ipc_int_secmode (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum,
    IPC_INTERRUPT_MODE intMode
    );

BCM_ERR_CODE chal_ipc_get_int_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    );

BCM_ERR_CODE chal_ipc_get_int_source (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE *source
    );

BCM_ERR_CODE chal_ipc_get_error_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    );


#ifdef __cplusplus
}
#endif

#endif /* _IPC_H_ */

