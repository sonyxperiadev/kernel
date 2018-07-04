/*
 * siw_touch_gpio.c - SiW touch gpio driver
 *
 * Copyright (C) 2016 Silicon Works - http://www.siliconworks.co.kr
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 * Author: Hyunho Kim <kimhh@siliconworks.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#ifndef __SIW_TOUCH_GPIO_H
#define __SIW_TOUCH_GPIO_H

#if 1
enum {
	GPIO_PULL_DOWN  = 0,
	GPIO_PULL_UP,
	GPIO_NO_PULL,
};

enum {
	GPIO_OUT_ZERO = 0,
	GPIO_OUT_ONE,
};
#else
/******************************************************************************
* Enumeration for GPIO pin
******************************************************************************/
/* GPIO MODE CONTROL VALUE*/
typedef enum {
	GPIO_MODE_UNSUPPORTED = -1,
	GPIO_MODE_GPIO = 0,
	GPIO_MODE_00 = 0,
	GPIO_MODE_01 = 1,
	GPIO_MODE_02 = 2,
	GPIO_MODE_03 = 3,
	GPIO_MODE_04 = 4,
	GPIO_MODE_05 = 5,
	GPIO_MODE_06 = 6,
	GPIO_MODE_07 = 7,
	GPIO_MODE_MAX,
	GPIO_MODE_DEFAULT = GPIO_MODE_01,
} GPIO_MODE;

/*----------------------------------------------------------------------------*/
/* GPIO DIRECTION */
typedef enum {
	GPIO_DIR_UNSUPPORTED = -1,
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT = 1,
	GPIO_DIR_MAX,
	GPIO_DIR_DEFAULT = GPIO_DIR_IN,
} GPIO_DIR;

/*----------------------------------------------------------------------------*/
/* GPIO PULL ENABLE*/

typedef enum {
	GPIO_PULL_EN_UNSUPPORTED = -1,
	GPIO_NOPULLUP		= -4,
	GPIO_NOPULLDOWN		= -5,
	GPIO_PULL_DISABLE = 0,
	GPIO_PULL_ENABLE  = 1,
	GPIO_PULL_EN_MAX,
	GPIO_PULL_EN_DEFAULT = GPIO_PULL_ENABLE,
} GPIO_PULL_EN;

/*----------------------------------------------------------------------------*/
/* GPIO SMT*/
typedef enum {
	GPIO_SMT_UNSUPPORTED = -1,
	GPIO_SMT_DISABLE = 0,
	GPIO_SMT_ENABLE  = 1,

	GPIO_SMT_MAX,
	GPIO_SMT_DEFAULT = GPIO_SMT_ENABLE,
} GPIO_SMT;

/*----------------------------------------------------------------------------*/
/* GPIO IES*/
typedef enum {
	GPIO_IES_UNSUPPORTED = -1,
	GPIO_IES_DISABLE = 0,
	GPIO_IES_ENABLE = 1,

	GPIO_IES_MAX,
	GPIO_IES_DEFAULT = GPIO_IES_ENABLE,
} GPIO_IES;

/*----------------------------------------------------------------------------*/
/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
	GPIO_PULL_UNSUPPORTED = -1,
	GPIO_PULL_DOWN  = 0,
	GPIO_PULL_UP    = 1,
	GPIO_NO_PULL = 2,
	GPIO_PULL_MAX,
	GPIO_PULL_DEFAULT = GPIO_PULL_DOWN
} GPIO_PULL;

/*----------------------------------------------------------------------------*/
/* GPIO INVERSION */
typedef enum {
	GPIO_DATA_INV_UNSUPPORTED = -1,
	GPIO_DATA_UNINV = 0,
	GPIO_DATA_INV = 1,

	GPIO_DATA_INV_MAX,
	GPIO_DATA_INV_DEFAULT = GPIO_DATA_UNINV
} GPIO_INVERSION;

/*----------------------------------------------------------------------------*/
/* GPIO OUTPUT */
typedef enum {
	GPIO_OUT_UNSUPPORTED = -1,
	GPIO_OUT_ZERO = 0,
	GPIO_OUT_ONE = 1,

	GPIO_OUT_MAX,
	GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
	GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,	/*compatible with DCT */
} GPIO_OUT;

/*----------------------------------------------------------------------------*/
/* GPIO INPUT */
typedef enum {
	GPIO_IN_UNSUPPORTED = -1,
	GPIO_IN_ZERO = 0,
	GPIO_IN_ONE = 1,

	GPIO_IN_MAX,
} GPIO_IN;
#endif

extern int siw_touch_gpio_init(struct device *dev, int pin, const char *name);
extern void siw_touch_gpio_free(struct device *dev, int pin);

extern void siw_touch_gpio_direction_input(struct device *dev, int pin);
extern void siw_touch_gpio_direction_output(struct device *dev, int pin, int value);
extern void siw_touch_gpio_set_pull(struct device *dev, int pin, int value);

extern int siw_touch_power_init(struct device *dev);
extern int siw_touch_power_free(struct device *dev);
extern void siw_touch_power_vdd(struct device *dev, int value);
extern void siw_touch_power_vio(struct device *dev, int value);

#endif	/* __SIW_TOUCH_GPIO_H */

