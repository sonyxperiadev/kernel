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

#ifndef SENSORS_BMA150_I2C_SETTINGS_H
#define SENSORS_BMA150_I2C_SETTINGS_H

/* 
 * The bma150 driver axis setting:
 *  For x as x
 *  For y reverse y
 *  For z reverse z
 */

#define BMA150_DRIVER_AXIS_SETTINGS \
{ \
   .x_change = x_dir, \
   .y_change = y_dir_rev, \
   .z_change = z_dir_rev, \
}

#endif /* SENSORS_BMA150_I2C_SETTINGS_H */
