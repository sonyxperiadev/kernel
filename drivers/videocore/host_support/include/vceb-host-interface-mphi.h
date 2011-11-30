/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************
*
*  vceb-host-interface-mphi.h
*
*  PURPOSE:
*
*       This file defines the vceb_host_interface_mphi structure which contains
*       information passed into the vceb_host_interface_create_instance
*       function. This header needs to compile under linux and under the
*       bootloaders.
*
*****************************************************************************/

#if !defined( VCEB_HOST_INTERFACE_MPHI_H )
#define VCEB_HOST_INTERFACE_MPHI_H

#include <csp/gpiomux.h>
#include <csp/chal_ccu_inline.h>

typedef struct
{
    chal_ccu_kps_policy_freq_e  freqId;
    uint32_t                    freqMHz;

    uint8_t                     readLow;
    uint8_t                     readHigh;
    uint8_t                     writeLow;
    uint8_t                     writeHigh;
    uint8_t                     writeMax;

} vceb_host_interface_mphi_timing_t;

typedef struct opaque_vceb_host_param_t
{
    uint32_t        target;
    uint32_t        busWidth;
    uint32_t        baseAddr;
    uint32_t        bootFromKernel;
    int             runPin;         /* 0 = Use MPHI RUN pin, != 0 is gpio pin */
    gpiomux_group_e gpiomux_group;
    int             gpiomux_id;
    const char     *gpiomux_label;

    vceb_host_interface_mphi_timing_t  *timing;
    int                                 numTimings;

} vceb_host_interface_mphi_t;

#endif /* VCEB_HOST_INTERFACE_MPHI_H */

