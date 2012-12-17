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

#ifndef MPU3050_SETTINGS_H
#define MPU3050_SETTINGS_H

#define MPU3050_I2C_BUS_ID	0
#define MPU3050_SLAVE_ADDR	0x68

#define MPU3050_INIT_CFG	0x10
#if defined(CONFIG_MACH_HAWAII_GARNET) && defined(CONFIG_MPU_SENSORS_MPU3050)
#define MPU3050_IRQ_GPIO	33
#else
#define MPU3050_IRQ_GPIO	64
#endif
#ifdef CONFIG_MPU_SENSORS_AK8975
#define MPU3050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_AMI30x
#define MPU3050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_AMI306
#define MPU3050_COMPASS_SLAVE_ADDR   0x0E
#elif defined CONFIG_MPU_SENSORS_YAS529
#define MPU3050_COMPASS_SLAVE_ADDR   0x2E
#elif defined CONFIG_MPU_SENSORS_MMC314X
#define MPU3050_COMPASS_SLAVE_ADDR   0x30
#elif defined CONFIG_MPU_SENSORS_HSCDTD00XX
#define MPU3050_COMPASS_SLAVE_ADDR   0x0C
#endif

/*
 * Correction for the mount position of MPU3050 sensor on CapriStone.
 *  For x reverse x
 *  For y y
 *  For z reverse z
 */
#if defined(CONFIG_MACH_HAWAII_GARNET) && defined(CONFIG_MPU_SENSORS_MPU3050)
#define MPU3050_DRIVER_GYRO_ORIENTATION \
{ \
	0, -1, 0, \
	 -1, 0, 0, \
	 0, 0, -1, \
}

#define MPU3050_DRIVER_COMPASS_ORIENTATION \
{ \
	 0, -1, 0, \
	 -1, 0, 0, \
	 0, 0, -1, \
}
#else
#define MPU3050_DRIVER_GYRO_ORIENTATION \
{ \
	-1, 0, 0, \
	 0, 1, 0, \
	 0, 0, -1, \
}

#define MPU3050_DRIVER_COMPASS_ORIENTATION \
{ \
	 -1, 0, 0, \
	 0, 1, 0, \
	 0, 0, -1, \
}
#endif
#ifdef CONFIG_MPU_SENSORS_KXTF9
#define MPU_SENSORS_KXTF9_IRQ_GPIO   104
#define MPU_SENSORS_KXTF9_SLAVE_ADDR	0x0F

#define MPU3050_DRIVER_ACCEL_KXTF9_ORIENTATION \
{ \
	-1, 0, 0,\
	0, 1, 0,\
	0, 0, -1,\
}
#endif

#endif /* MPU3050_SETTINGS_H */
