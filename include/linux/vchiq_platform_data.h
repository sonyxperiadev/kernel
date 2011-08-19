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


#ifndef VCHIQ_PLATFORM_DATA_H_
#define VCHIQ_PLATFORM_DATA_H_

/*
 * A pointer to the following structure needs to be stored in the platform_device's
 * dev.platform_data member.
 *
 * The instance name is used to associate the vchiq instance with a vceb instance.
 *
 *  The instance_name can be NULL or the empty string. In this situation, the proc entries
 *  will be created as before
 *
 *      /proc/vc/vchiq_control
 *      /proc/vc/vchiq_queue
 *      /proc/vc/vchiq_xxx (etc.)
 *
 *  If an instance name is provided, then the proc entries will be created as:
 *
 *      /proc/vc/instance-name/vchiq_control
 *      /proc/vc/instance-name/vchiq_control
 *      /proc/vc/isntance-name/vchiq_xxx (etc.)
 */

typedef enum
{
    VCHIQ_DEVICE_TYPE_SHARED_MEM,
    VCHIQ_DEVICE_TYPE_HOST_PORT,

} VCHIQ_DEVICE_TYPE;

typedef struct
{
    const char         *instance_name;
    VCHIQ_DEVICE_TYPE   dev_type;

} VCHIQ_PLATFORM_DATA_T;

#endif  /* VCHIQ_PLATFORM_DATA_H_ */
