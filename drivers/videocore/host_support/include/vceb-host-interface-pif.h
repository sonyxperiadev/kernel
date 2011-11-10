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
*  vceb-host-interface-pif.h
*
*  PURPOSE:
*
*       This file defines the vceb_host_interface_pif structure which contains
*       information passed into the vceb_host_interface_create_instance
*       function. This header needs to compile under linux and under the
*       bootloaders.
*
*****************************************************************************/

#if !defined( VCEB_HOST_INTERFACE_PIF_H )
#define VCEB_HOST_INTERFACE_PIF_H

typedef struct opaque_vceb_host_param_t
{
    unsigned    target;
    unsigned    pad_current;
    unsigned    run_pin;    /* gpio number of the run pin for the 1st videocore */

    /* The following pins are for the second videocore, and should be set to 
     * zero if there is no second videocore
     */
    unsigned    run2_pin;
    unsigned    hat2_pin;
    unsigned    extra_delay;

} vceb_host_interface_pif_t;

#endif /* VCEB_HOST_INTERFACE_PIF_H */

