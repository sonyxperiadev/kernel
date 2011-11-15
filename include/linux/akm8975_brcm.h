/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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


/*
 * Definitions for akm8975 compass chip.
 */
#ifndef __AKM8975_H__
#define __AKM8975_H__


/* Axis switching stuff should be common header as same enum exists for accelerometer and gyro */
typedef enum
{
   compass_x_dir,
   compass_y_dir,
   compass_z_dir,
   compass_x_dir_rev,
   compass_y_dir_rev,
   compass_z_dir_rev
} akm8975_axis_change_enum;

struct t_akm8975_axis_change
{
   int x_change;
   int y_change;
   int z_change;
}; 


#define AKM8975_DEVICE_NAME   "compass"
#define AKM8975_DRV_NAME      "akm8975"
#define AKM8975_I2C_ADDR      0x0C

#endif

