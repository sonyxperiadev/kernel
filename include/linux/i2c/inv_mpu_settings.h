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

#ifndef INV_MPU_SETTINGS_H
#define INV_MPU_SETTINGS_H

#define INV_MPU_I2C_BUS_ID	2
#define INV_MPU_SLAVE_ADDR	0x68

#define INV_MPU_INIT_CFG	0x10
#if defined(CONFIG_MACH_HAWAII_GARNET) || defined(CONFIG_MACH_JAVA_GARNET)
#define INV_MPU_DRIVER_NAME	"mpu3050"
#define INV_MPU_IRQ_GPIO	33
#else
#define INV_MPU_DRIVER_NAME	"mpu6050"
#define INV_MPU_IRQ_GPIO	33
#endif

/*
 * Correction for the mount position of INV_MPU sensor.
 *  For x reverse x
 *  For y y
 *  For z reverse z
 */
#if defined(CONFIG_MACH_HAWAII_GARNET)  || defined(CONFIG_MACH_JAVA_GARNET)
#define INV_MPU_DRIVER_GYRO_ORIENTATION \
{ \
	0, -1, 0, \
	 -1, 0, 0, \
	 0, 0, -1, \
}

#define INV_MPU_DRIVER_COMPASS_ORIENTATION \
{ \
	 0, -1, 0, \
	 -1, 0, 0, \
	 0, 0, -1, \
}
#else
#define INV_MPU_DRIVER_GYRO_ORIENTATION \
{ \
	-1, 0, 0, \
	 0, 1, 0, \
	 0, 0, -1, \
}

#define INV_MPU_DRIVER_COMPASS_ORIENTATION \
{ \
	 -1, 0, 0, \
	 0, 1, 0, \
	 0, 0, -1, \
}
#endif
#ifdef CONFIG_MPU_SENSORS_KXTF9
#define MPU_SENSORS_KXTF9_IRQ_GPIO   104
#define MPU_SENSORS_KXTF9_SLAVE_ADDR	0x0F

#define INV_MPU_DRIVER_ACCEL_KXTF9_ORIENTATION \
{ \
	-1, 0, 0,\
	0, 1, 0,\
	0, 0, -1,\
}
#endif

#endif /* INV_MPU_SETTINGS_H */
