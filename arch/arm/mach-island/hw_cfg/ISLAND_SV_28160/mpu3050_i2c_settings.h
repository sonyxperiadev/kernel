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

#ifndef MPU3050_I2C_SETTINGS_H
#define MPU3050_I2C_SETTINGS_H

#define MPU3050_I2C_BUS_ID    2
#define MPU3050_GPIO_IRQ_PIN 11

// Application programmable full-scale range of +- 250, +-500,
// +- 1000 or +- 2000 degrees/second. Calibration for BCMHANA_TABLET 
// identified following pre-configured value
#define MPU3050_SCALE        250	

/* 
 * Correction for the mount position of MPU3050 sensor on daughter card PCB.
 *  For x x
 *  For y y
 *  For z reverse z
 */

#define MPU3050_DRIVER_AXIS_SETTINGS \
{ \
   .x_change = gyro_x_dir_rev, \
   .y_change = gyro_y_dir, \
   .z_change = gyro_z_dir_rev, \
}

#endif /* MPU3050_I2C_SETTINGS_H */
