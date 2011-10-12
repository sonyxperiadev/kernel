/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#if !defined __AKM8975_I2C_SETTINGS_H__
#define      __AKM8975_I2C_SETTINGS_H__

// compass i2c settings
#define AKM8975_I2C_BUS_ID    2
#define AKM8975_I2C_ADDR      0x0C


/* 
 * Correction for the mount position of AKM8975 sensor on daughter card PCB.
 *  For x x
 *  For y y
 *  For z z
 */

#define AKM8975_DRIVER_AXIS_SETTINGS \
{ \
   .x_change = compass_x_dir_rev, \
   .y_change = compass_y_dir_rev, \
   .z_change = compass_z_dir, \
}



#endif     // __AKM8975_I2C_SETTINGS_H__

