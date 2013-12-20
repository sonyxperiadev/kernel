/*
 * bmg160.h -  Linux kernel  for BOSCH bmg160 gyroscope
 *
 * Copyright (c) 2012 Sony Mobile Communications AB
 *
 * Authors: Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */
#ifndef LINUX_BMG160_MODULE_H
#define LINUX_BMG160_MODULE_H

#include <linux/device.h>

#define BMG160_NAME                      "bmg160"

#define BMG160_CHIP_ID_REG               0x00
#define BMG160_CHIP_ID_BMG160            0x0f

#define BMG160_X_AXIS_LSB_REG            0x02
#define BMG160_Y_AXIS_LSB_REG            0x04
#define BMG160_Z_AXIS_LSB_REG            0x06

#define BMG160_RANGE_REG                 0x0F
#define BMG160_RANGE_MASK                0x07
#define BMG160_RANGE_2000                0x00
#define BMG160_RANGE_1000                0x01
#define BMG160_RANGE_500                 0x02
#define BMG160_RANGE_250                 0x03
#define BMG160_RANGE_125                 0x04

#define BMG160_UR_BW_SEL_REG             0x10
#define BMG160_UR_BW_100HZ_32HZ          0x07
#define BMG160_UR_BW_200HZ_64HZ          0x06
#define BMG160_UR_BW_100HZ_12HZ          0x05
#define BMG160_UR_BW_200HZ_23HZ          0x04
#define BMG160_UR_BW_400HZ_47HZ          0x03
#define BMG160_UR_BW_1000HZ_116HZ        0x02
#define BMG160_UR_BW_2000HZ_230HZ        0x01
#define BMG160_UR_BW_2000HZ_583HZ        0x00

#define BMG160_MODE_CTRL_REG             0x11
#define BMG160_MODE_SUSPEND              0x80
#define BMG160_MODE_DEEPSUSPEND          0x20

#define BMG160_RESET_REG                 0x14
#define BMG160_RESET                     0xB6

#define BMG160_SOC                       0x31
#define BMG160_OFC1_REG                  0x36
#define BMG160_OFC2_REG                  0x37
#define BMG160_OFC3_REG                  0x38
#define BMG160_OFC4_REG                  0x39
#define BMG160_TRIMGP0_REG               0x3A

#define BMG160_LAST_REG 0x3F

/**
 * struct bmg160_platform_data - data to set up bmg160 driver
 *
 * @setup: optional callback to activate the driver.
 * @teardown: optional callback to invalidate the driver.
 * @power_up: optional callback to turn power on, it is assumed
 *            that Vdd and/or VddIO is turned on. Note that this
 *            must include sleeping to wait for capacitors to get
 *            changed and curcuit wakeup (BMA255 3ms, BMA250 2ms)
 *            You shouldn't sleep more than say 25ms for driver
 *            to work.
 * @power_down: optional callback to turn power off, it is assumed
 *             that Vdd and/or VddIO is turned off
 * @range: specifies range of captured data
**/

struct bmg160_platform_data {
	int (*setup)(struct device *);
	void (*teardown)(struct device *);
	int (*power_up)(struct device *);
	int (*power_down)(struct device *);
	int range;
};

#endif /* LINUX_BMG160_MODULE_H */
