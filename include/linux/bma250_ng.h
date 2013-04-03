/* include/linux/bma250_ng.h
 *
 * Bosh BMA 250. Digital, triaxial acceleration sensor.
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Marcus Bauer <marcus.bauer@sonyericsson.com>
 *         Tadashi Kubo <tadashi.kubo@sonyericsson.com>
 *         Takashi Shiina <takashi.shiina@sonyericsson.com>
 *         Hisakazu Furuie <hisakazu.x.furuie@sonyericsson.com>
 *         Chikaharu Gonnokami <Chikaharu.X.Gonnokami (at) sonyericsson.com>
 *         Stefan Karlsson <stefan4.karlsson@sonymobile.com>
 *
 * NOTE: This file has been created by Sony Ericsson Mobile Communications AB.
 *       This file has been modified by Sony Mobile Communications AB.
 *       This file contains code from: bma150.c
 *       The orginal bma150.c header is included below:
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef LINUX_BMA250_MODULE_H
#define LINUX_BMA250_MODULE_H

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Registers */
#define BMA250_CHIP_ID_REG               0x00
#define BMA250_VERSION_REG               0x01
#define BMA250_X_AXIS_LSB_REG            0x02

#define BMA250_RANGE_REG                 0x0F
#define BMA250_RANGE_MASK                0x0F
#define BMA250_RANGE_16G                 0x0C
#define BMA250_RANGE_8G                  0x08
#define BMA250_RANGE_4G                  0x05
#define BMA250_RANGE_2G                  0x03

#define BMA250_BW_SEL_REG                0x10
#define BMA250_BW_7_81HZ                 0x08
#define BMA250_BW_15_63HZ                0x09
#define BMA250_BW_31_25HZ                0x0A
#define BMA250_BW_62_50HZ                0x0B
#define BMA250_BW_125HZ                  0x0C
#define BMA250_BW_250HZ                  0x0D
#define BMA250_BW_500HZ                  0x0E
#define BMA250_BW_1000HZ                 0x0F

#define BMA250_MODE_CTRL_REG             0x11
#define BMA250_MODE_NOSLEEP              0x00
#define BMA250_MODE_SLEEP_0_5MS          0x4A
#define BMA250_MODE_SLEEP_1MS            0x4C
#define BMA250_MODE_SLEEP_2MS            0x4E
#define BMA250_MODE_SLEEP_4MS            0x50
#define BMA250_MODE_SLEEP_6MS            0x52
#define BMA250_MODE_SLEEP_10MS           0x54
#define BMA250_MODE_SLEEP_25MS           0x56
#define BMA250_MODE_SLEEP_50MS           0x58
#define BMA250_MODE_SLEEP_100MS          0x5A
#define BMA250_MODE_SLEEP_500MS          0x5C
#define BMA250_MODE_SLEEP_1000MS         0x5E
#define BMA250_MODE_SUSPEND              0x80

#define BMA250_RESET_REG                 0x14
#define BMA250_RESET                     0xB6

#define BMA250_INT_ENABLE1_REG           0x16
#define BMA250_INT_FLAT                  0x80
#define BMA250_INT_ORIENT                0x40
#define BMA250_INT_S_TAP                 0x20
#define BMA250_INT_D_TAP                 0x10
#define BMA250_INT_SLOPE_Z               0x04
#define BMA250_INT_SLOPE_Y               0x02
#define BMA250_INT_SLOPE_X               0x01

#define BMA250_INT_ENABLE2_REG           0x17
#define BMA250_INT_NEW_DATA              0x10
#define BMA250_INT_LOW_G                 0x08
#define BMA250_INT_HIGH_G_Z              0x04
#define BMA250_INT_HIGH_G_Y              0x02
#define BMA250_INT_HIGH_G_X              0x01

#define BMA250_INT_PIN1_REG              0x19
#define BMA250_INT_PIN1_FLAT             0x80
#define BMA250_INT_PIN1_ORIENT           0x40
#define BMA250_INT_PIN1_S_TAP            0x20
#define BMA250_INT_PIN1_D_TAP            0x10
#define BMA250_INT_PIN1_SLOPE            0x04
#define BMA250_INT_PIN1_HIGH_G           0x02
#define BMA250_INT_PIN1_LOW_G            0x01

#define BMA250_INT_NEW_DATA_REG          0x1A
#define BMA250_INT_PIN2                  0x80
#define BMA250_INT_PIN1                  0x01

#define BMA250_INT_PIN2_REG              0x1B
#define BMA250_INT_PIN2_FLAT             0x80
#define BMA250_INT_PIN2_ORIENT           0x40
#define BMA250_INT_PIN2_S_TAP            0x20
#define BMA250_INT_PIN2_D_TAP            0x10
#define BMA250_INT_PIN2_SLOPE            0x04
#define BMA250_INT_PIN2_HIGH_G           0x02
#define BMA250_INT_PIN2_LOW_G            0x01

#define BMA250_INT_CTRL_REG              0x21
#define BMA250_INT_RESET                 0x80

#define BMA250_SLOPE_DUR                 0x27
#define BMA250_SLOPE_THR                 0x28

#define BMA250_INTERRUPT_RESOLUTION   0
#define BMA250_TIMER_RESOLUTION       1
#define BMA250_INVALID   0xFFFFFFFF

#define BMA250_EEPROM_CTRL_REG           0x33
#define BMA250_EEPROM_RDY_MODE           0x00
#define BMA250_EEPROM_PROG_MODE          0x01
#define BMA250_EEPROM_WRITE_MODE         0x03
#define BMA250_EEPROM_NOT_RDY_MODE       0x04

#define BMA250_OFFSET_X_AXIS_REG         0x38
#define BMA250_OFFSET_Y_AXIS_REG         0x39
#define BMA250_OFFSET_Z_AXIS_REG         0x3A

struct registers {
	int range;
	int bw_sel;
};

/**
 * struct bma250_platform_data - data to set up bma250 driver
 *
 * @setup: optional callback to activate the driver.
 * @teardown: optional callback to invalidate the driver.
 *
**/

struct bma250_platform_data {
	int (*hw_config)(struct device *, int);
	struct registers *reg;
	int (*read_axis_data)(struct i2c_client *, char*, int);
	void(*bypass_state)(int, char*);
	int (*check_sleep_status)(void);
	void (*vote_sleep_status)(int, int);
	unsigned int rate;
};

#endif /* LINUX_BMA250_MODULE_H */
