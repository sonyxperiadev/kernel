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

#ifndef MPU6050_SETTINGS_H
#define MPU6050_SETTINGS_H

#define MPU6050_I2C_BUS_ID	2
#define MPU6050_SLAVE_ADDR	0x68

#define MPU6050_INIT_CFG	0x10

#define MPU6050_IRQ_GPIO	33

#ifdef CONFIG_MPU_SENSORS_AK8975
#define MPU6050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_AMI30x
#define MPU6050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_AMI306
#define MPU6050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_YAS529
#define MPU6050_COMPASS_SLAVE_ADDR   0x2E
#elif defined CONFIG_MPU_SENSORS_MMC314X
#define MPU6050_COMPASS_SLAVE_ADDR   0x30
#elif defined CONFIG_MPU_SENSORS_HSCDTD00XX
#define MPU6050_COMPASS_SLAVE_ADDR   0x0C
#endif

/*
 * Correction for the mount position of MPU6050 sensor on RheaStone.
 * For x reverse x
 * For y y
 * For z reverse z
 */
#define MPU6050_DRIVER_ACCEL_GYRO_ORIENTATION \
{ \
	        -1, 0, 0, \
	         0, 1, 0, \
	         0, 0, -1, \
}

#define MPU6050_DRIVER_COMPASS_ORIENTATION \
{ \
	 1, 0, 0, \
	 0, 1, 0, \
	 0, 0, 1, \
}

#endif // MPU6050_SETTINGS_H
