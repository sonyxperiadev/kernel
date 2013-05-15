/* arch/arm/mach-msm/gyro-semc_common.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _GYRO_SEMC_COMMON_H_
#define _GYRO_SEMC_COMMON_H_

#ifdef CONFIG_SENSORS_MPU3050
extern int mpu3050_gpio_setup(struct device *dev, int enable);
extern int mpu3050_power_mode(struct device *dev, int enable);
extern struct mpu3050_platform_data mpu_data;

int bma250_read_axis_from_mpu3050(
	struct i2c_client *ic_dev, char *buf, int length);
void mpu3050_bypassmode(int rw, char *bypass);
int check_bma250_sleep_state(void);
void vote_bma250_sleep_state(int id, int vote);
#endif

#ifdef CONFIG_SENSORS_MPU6050
extern int mpu6050_gpio_setup(struct device *dev, int enable);
extern void mpu6050_hw_config(int enable);
extern void mpu6050_power_mode(int enable);
extern struct mpu_platform_data mpu6050_data;
extern struct ext_slave_platform_data mpu_compass_data;
#endif /* CONFIG_SENSORS_MPU6050 */

#endif
