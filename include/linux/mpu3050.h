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

/* user commands */
#define MPU3050_IOC_MAGIC        'B'

/* Default delay between poll events in non-interrupt mode */
#define MPU3050_POLL_RATE_MSEC    200   // 0.2 seconds

/* Uniform IOCTL Interface for User control from Sensor Library */
#define MPU3050_SET_POLL_RATE           _IOW(MPU3050_IOC_MAGIC, 100, unsigned int)
#define MPU3050_SET_ENABLE			    _IOW(MPU3050_IOC_MAGIC, 101, unsigned char)

// gyro scale; SET can be added once driver implements functionality
#define MPU3050_GET_SCALE				_IOW(MPU3050_IOC_MAGIC, 102, unsigned int)

#endif /* _MPU3050_H_ */
