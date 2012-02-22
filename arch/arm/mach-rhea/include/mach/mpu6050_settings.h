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

#define MPU6050_I2C_BUS_ID	1
#define MPU6050_SLAVE_ADDR	0x68

#define MPU6050_INIT_CFG	0x10

#define MPU_INT_GPIO_PIN		77
#define ACCE_INT_GPIO_PIN		77
#define COMP_INT_GPIO_PIN		78

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
 * Correction for the mount position of MPU6050 sensor on RheaBerri EDN40
 * NOTE: If you are using this on other boards, you may have to change these
 * according to the MPU6050 mount position on it. This setting is specifically
 * for RheaBerri EDN40
 *
 *  For x y
 *  For y reverse x
 *  For z z
 */

/*   .y_change = axis_x_dir_rev, \ */
#define MPU6050_DRIVER_ACCEL_GYRO_SETTINGS \
{ \
   .x_change = axis_y_dir, \
   .y_change = axis_x_dir_rev, \
   .z_change = axis_z_dir, \
}

#define MPU6050_DRIVER_COMPASS_SETTINGS \
{ \
   .x_change = axis_y_dir, \
   .y_change = axis_x_dir, \
   .z_change = axis_z_dir_rev, \
}

#define MPU6050_DRIVER_REG_VALUES \
{ \
    0x0, /* MPUREG_PWR_MGMT_1   0x6C, 108 */ \
    0x4, /* MPUREG_SMPLRT_DIV   0x19,  25 */ \
    0xb, /* MPUREG_CONFIG       0x1a,  26 */ \
    0x8, /* MPUREG_GYRO_CONFIG  0x1b,  27 */ \
    0x8, /* MPUREG_ACCEL_CONFIG 0x1c,  28 */ \
    0x1, /* MPUREG_PWR_MGMT_1   0x6b, 107 */ \
    0x0, /* MPUREG_PWR_MGMT_2   0x6C, 108 */ \
}

#endif
