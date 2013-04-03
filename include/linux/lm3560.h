/* include/linux/lm3560.h
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Kazunari YOSHINO <Kazunari.X.Yoshino@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
*/

#ifndef __MISC_LM3560_H
#define __MISC_LM3560_H

#include <linux/types.h>

/* Trigger of strobe */
enum lm3560_strobe_trigger {
	LM3560_STROBE_TRIGGER_LEVEL,
	LM3560_STROBE_TRIGGER_EDGE
};

/* Flash H/W synchronization control */
enum lm3560_sync_state {
	LM3560_SYNC_OFF,
	LM3560_SYNC_ON
};

/* Strobe polarity */
enum lm3560_strobe_polarity {
	LM3560_STROBE_POLARITY_LOW,
	LM3560_STROBE_POLARITY_HIGH
};

/* Setting of LEDI/NTC pin */
enum lm3560_ledintc_pin_setting {
	LM3560_LEDINTC_INDICATOR_OUTPUT,
	LM3560_LEDINTC_NTC_THERMISTOR_INPUT
};

/* TX1 polarity */
enum lm3560_tx1_polarity {
	LM3560_TX1_POLARITY_LOW,
	LM3560_TX1_POLARITY_HIGH
};

/* TX2 polarity */
enum lm3560_tx2_polarity {
	LM3560_TX2_POLARITY_LOW,
	LM3560_TX2_POLARITY_HIGH
};

/* Enable/Disable H/W torch mode */
enum lm3560_hw_torch_mode {
	LM3560_HW_TORCH_MODE_DISABLE,
	LM3560_HW_TORCH_MODE_ENABLE
};

/*
 * Setting of the restore to privacy mode
 * at the end of the flash pulse
 */
enum lm3560_privacy_terminate {
	LM3560_PRIVACY_MODE_TURN_BACK,
	LM3560_PRIVACY_MODE_REMAIN_OFF
};

#define LM3560_DRV_NAME "lm3560"
struct device;
struct lm3560_platform_data {
	int (*power)(struct device *dev, bool request);
	int (*platform_init)(struct device *dev, bool request);
	int led_nums; /* The number of the LED for Torch/Flash mode */
	enum lm3560_strobe_trigger strobe_trigger;
	unsigned long privacy_current; /* 3125uA..25000uA */
	enum lm3560_privacy_terminate privacy_terminate;
	int privacy_led_nums; /* The number of the LED for Privacy mode */
	unsigned long privacy_blink_period; /* 0us(No bliking)..512000us */
	unsigned long current_limit;
	enum lm3560_sync_state flash_sync;
	enum lm3560_strobe_polarity strobe_polarity;
	enum lm3560_ledintc_pin_setting ledintc_pin_setting;
	enum lm3560_tx1_polarity tx1_polarity;
	enum lm3560_tx2_polarity tx2_polarity;
	enum lm3560_hw_torch_mode hw_torch_mode;
};
#endif /* __LINUX_LEDS_LM3560_H */

