/* kernel/include/linux/input/noa3402.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Johan Olson <johan.olson@sonyericsson.com>
 *         Louis Benoit <louis.benoit@onsemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __NOA3402_H__
#define __NOA3402_H__

#define NOA3402_NAME "noa3402"

#ifdef __KERNEL__

#define PWM_SENSITIVITY_STD	0
#define PWM_SENSITIVITY_HALF	(1 << 5)
#define PWM_SENSITIVITY_QUARTER	(1 << 6)
#define PWM_SENSITIVITY_EIGHTH	((1 << 5) | (1 << 6))

#define PWM_RES_7_BIT	0
#define PWM_RES_8_BIT	(1 << 3)
#define PWM_RES_9_BIT	(1 << 4)
#define PWM_RES_10_BIT	((1 << 3) | (1 << 4))

#define PWM_TYPE_LINEAR	0
#define PWM_TYPE_LOG	(1 << 2)

#define LED_CURRENT_MA_TO_REG(ma) ((ma - 5) / 5)	/* milliampere */

#define PS_INTEGRATION_150_US	0			/* 75us for Rev C */
#define PS_INTEGRATION_300_US	(1 << 0)		/* 150us for Rev C */
#define PS_INTEGRATION_600_US	(1 << 1)		/* 300us for Rev C */
#define PS_INTEGRATION_1200_US	(1 << 0 | 1 << 1)	/* 600us for Rev C */

#define PS_INTERVAL_MS_TO_REG(ms) ((ms - 5) / 5)

#define ALS_INTEGRATION_6_25_MS	0
#define ALS_INTEGRATION_12_5_MS	(1 << 0)
#define ALS_INTEGRATION_25_MS	(1 << 1)
#define ALS_INTEGRATION_50_MS	(1 << 0 | 1 << 1)
#define ALS_INTEGRATION_100_MS	(1 << 2)
#define ALS_INTEGRATION_200_MS	(1 << 0 | 1 << 2)
#define ALS_INTEGRATION_400_MS	(1 << 1 | 1 << 2)
#define ALS_INTEGRATION_800_MS	(1 << 0 | 1 << 1 | 1 << 2)

#define ALS_INTERVAL_MS_TO_REG(ms) (ms / 50)

struct noa3402_platform_data {
	u16 gpio;
	u8 pwm_sensitivity;
	u8 pwm_res;
	u8 pwm_type;
	u8 ps_led_current;
	u8 ps_filter_nbr_correct;
	u8 ps_filter_nbr_measurements;
	u8 ps_integration_time;
	u8 ps_interval;
	u8 als_integration_time;
	u8 als_interval;
	unsigned int is_irq_wakeup;
	char *phys_dev_path;
	int (*hw_config)(int enable);
	int (*gpio_setup)(int request);
};
#endif

#endif
