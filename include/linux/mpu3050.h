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
 * Definitions for MPU3050 gyro sensor chip.
 */

#ifndef _MPU3050_H_
#define _MPU3050_H_

/* Could be common header as same enum exists for accelerometer (bma150.h)
   However keeping separate copy as we might be switching to 6-axis gyro
   (Sensor Fusion with 3rd Party accelerometer */
typedef enum
{
   gyro_x_dir,
   gyro_y_dir,
   gyro_z_dir,
   gyro_x_dir_rev,
   gyro_y_dir_rev,
   gyro_z_dir_rev
} mpu3050_axis_change_enum;

struct t_mpu3050_axis_change
{
   int x_change;
   int y_change;
   int z_change;
}; 

struct mpu3050_platform_data
{
   int                           gpio_irq_pin;
   int                           scale;           // angular rate [deg/sec]
   struct t_mpu3050_axis_change* p_axis_change;
};

#define MPU3050_DRV_NAME		"mpu3050"
#define MPU3050_I2C_ADDR		0x68

#endif /* _MPU3050_H_ */
