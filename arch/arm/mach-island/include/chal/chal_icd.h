/*****************************************************************************
* Copyright (c) 2003-2008 Broadcom Corporation.  All rights reserved.
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
#ifndef CHAL_ICD_H__
#define CHAL_ICD_H__

#include <plat/chal/chal_types.h>
#include <chal/bcmerr.h>
#include <chal/chal_int.h>
#include <plat/chal/chal_common.h>
//#include <mach/csp/mm_io.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *  Use these macros to convert interrup id to ICDIPTR and bit offsets.
 *  each ICDIPTR contails 4 fields, 8 bits each, lower INTID at LSB side
 **************************************************************************/
#define ICDIPTR_CPU0            (0x1)
#define ICDIPTR_CPU1            (0x2)
#define ICDIPTR_REG             (MM_IO_BASE_GICDIST + 0x800)
#define ADDR_ICDIPTR(id)	((id) >> 2)
#define ICDIPTR_SHIFT(id)	(((id) & 0x3) * 8)
#define ICDIPTR_MASK            (0xFF)

typedef struct chal_icd_priv_context *chal_icd_handle;

BCM_ERR_CODE chal_icd_disable
(
    chal_icd_handle hIcd    /* [in] Interrupt Controller Dist Handle */
);

BCM_ERR_CODE chal_icd_enable
(
    chal_icd_handle hIcd    /* [in] Interrupt Controller Dist Handle */
);

BCM_ERR_CODE chal_icd_set_enable
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
);

BCM_ERR_CODE chal_icd_clear_enable
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
);

BCM_ERR_CODE chal_icd_set_spi_target
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         ulTarget /* [in] select cpu target */
);

BCM_ERR_CODE chal_icd_get_enable
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulMask /* [out] 1 == on, 0 == off */
);

/* should be used for testing only.  use this to trigger interrupt */
BCM_ERR_CODE chal_icd_set_pending
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
);

BCM_ERR_CODE chal_icd_get_pending
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
);

/* should be used for testing only.  use this to trigger interrupt */
BCM_ERR_CODE chal_icd_clear_pending
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
);

BCM_ERR_CODE chal_icd_get_active_status
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
);

BCM_ERR_CODE chal_icd_set_security
(
    chal_icd_handle     hIcd,        /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,     /* [in] Interrupt ID */
    int_secure_state_t  eSecureState /* [in] secure state */
);

BCM_ERR_CODE chal_icd_get_security
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_secure_state_t  *peSecureState /* [out] secure state */
);

BCM_ERR_CODE chal_icd_set_priority_level
(
    chal_icd_handle hIcd,      /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,   /* [in] Interrupt ID */
    int_priority_t  ePriority  /* [in] priority level */
);

BCM_ERR_CODE chal_icd_get_priority_level
(
    chal_icd_handle hIcd,       /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,    /* [in] Interrupt ID */
    int_priority_t  *pePriority /* [out] Priority Level */
);

BCM_ERR_CODE chal_icd_set_trigger_type
(
    chal_icd_handle hIcd,        /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,     /* [in] Interrupt ID */
    int_trigger_t   eTriggerType /* [in] trigger type */
);

BCM_ERR_CODE chal_icd_get_trigger_type
(
    chal_icd_handle hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,       /* [in] Interrupt ID */
    int_trigger_t   *peTriggerType /* [out] trigger type */
);

BCM_ERR_CODE chal_icd_get_signal_status
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
);

BCM_ERR_CODE chal_icd_trigger_secure_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuInterface  /* [in] target cpu bit mask */
);

BCM_ERR_CODE chal_icd_trigger_nonsecure_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuInterface  /* [in] target cpu bit mask */
);

/* Get information about ICD */
BCM_ERR_CODE chal_icd_get_info
(
    chal_icd_handle     hIcd,         /* [in] Interrupt Controller Dist Handle */
    uint32_t            *num_lspi,    /* [out] Number of Lockable SPI */
    uint32_t            *num_sd,      /* [out] Number of security domains */
    uint32_t            *num_cpus,    /* [out] Number of Cortex-A9 processors */
    uint32_t            *num_it       /* [out] Number of IT (interrupt lines) */
);

#ifdef __cplusplus
}
#endif

#endif /* CHAL_ICD_H__ */

