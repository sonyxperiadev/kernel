/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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


#ifndef VCEB_PLATFORM_DATA_HANA_H_
#define VCEB_PLATFORM_DATA_HANA_H_

#include "common/vceb_host_interface.h"

/*
 * A pointer to the following structure needs to be stored in the platform_device's
 * dev.platform_data member.
 *
 * The instance name and parameters are passed to vceb_get_or_create_instance.
 *
 *  The instance_name can be NULL or the empty string. In this situation, the proc entries
 *  will be created as before
 *
 *      /proc/vceb_init
 *      /proc/vceb_download
 *      /proc/vceb_xxx (etc.)
 *
 *  If an instance name is provided, then the proc entries will be created as:
 *
 *      /proc/vceb/instance-name/vceb_init
 *      /proc/vceb/instance-name/vceb_download
 *      /proc/vceb/instance-name/vceb_xxx (etc.)
 *
 *  videocore_param is a character string which will be passed on the command line to the
 *  the final videocore image. This can be used for configuring platform specific gpios
 *  lcd type, camera type, etc.
 *
 *  host_param is an opaque structure which is passed into the 
 *  vceb_host_interface_create_instance function. It would contain information specific
 *  to that driver (i.e. gpio number of run pin).
 */

typedef struct vceb_platform_data_hana_t
{
    VCEB_HOST_INTERFACE_CREATE_PARAMS_T  create_params;

    uint32_t        bootFromKernel;

    /*
     * vcMemAddr is the main videocore memory address, typically LPDDR2
     */
    uint32_t        vcMemAddr;

    /*
     * vcSramAddr is the address of the SRAM used for communicating between the ARM 
     * and the videocore. 
     */
    uint32_t        vcSramAddr;
#if 0
    gpiomux_group_e gpiomux_lcd_group;
    int             gpiomux_lcd_id;
    const char     *gpiomux_lcd_label;

    gpiomux_group_e gpiomux_jtag_group;
    int             gpiomux_jtag_id;
    const char     *gpiomux_jtag_label;
#endif

} VCEB_PLATFORM_DATA_HANA_T;

#endif  /* VCEB_PLATFORM_DATA_HANA_H_ */

