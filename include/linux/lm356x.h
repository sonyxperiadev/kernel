/* include/linux/lm356x.h
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 *
 * Author: Kazunari YOSHINO <Kazunari.X.Yoshino@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
*/

#ifndef __MISC_LM356X_H
#define __MISC_LM356X_H

#include <linux/types.h>

/* Trigger of strobe */
enum lm356x_strobe_trigger {
	LM356X_STROBE_TRIGGER_LEVEL,
	LM356X_STROBE_TRIGGER_EDGE
};

/* Flash H/W synchronization control */
enum lm356x_sync_state {
	LM356X_SYNC_OFF,
	LM356X_SYNC_ON
};

/* Strobe polarity */
enum lm356x_strobe_polarity {
	LM356X_STROBE_POLARITY_LOW,
	LM356X_STROBE_POLARITY_HIGH
};

/* Setting of LEDI/NTC pin */
enum lm356x_ledintc_pin_setting {
	LM356X_LEDINTC_INDICATOR_OUTPUT,
	LM356X_LEDINTC_NTC_THERMISTOR_INPUT
};

/* TX1 polarity */
enum lm356x_tx1_polarity {
	LM356X_TX1_POLARITY_LOW,
	LM356X_TX1_POLARITY_HIGH
};

/* TX2 polarity */
enum lm356x_tx2_polarity {
	LM356X_TX2_POLARITY_LOW,
	LM356X_TX2_POLARITY_HIGH
};

/* Enable/Disable H/W torch mode */
enum lm356x_hw_torch_mode {
	LM356X_HW_TORCH_MODE_DISABLE,
	LM356X_HW_TORCH_MODE_ENABLE
};

/*
 * Setting of the restore to privacy mode
 * at the end of the flash pulse
 */
enum lm356x_privacy_terminate {
	LM356X_PRIVACY_MODE_TURN_BACK,
	LM356X_PRIVACY_MODE_REMAIN_OFF
};

#define LM3560_DRV_NAME "lm3560"
#define LM3561_DRV_NAME "lm3561"

struct lm356x_platform_data {
	int (*hw_enable)(void);
	int (*hw_disable)(void);
	int led_nums; /* The number of the LED for Torch/Flash mode */
	enum lm356x_strobe_trigger strobe_trigger;
	unsigned long privacy_current; /* 3125uA..25000uA */
	enum lm356x_privacy_terminate privacy_terminate;
	int privacy_led_nums; /* The number of the LED for Privacy mode */
	unsigned long privacy_blink_period; /* 0us(No bliking)..512000us */
	unsigned long current_limit;
	enum lm356x_sync_state flash_sync;
	enum lm356x_strobe_polarity strobe_polarity;
	enum lm356x_ledintc_pin_setting ledintc_pin_setting;
	enum lm356x_tx1_polarity tx1_polarity;
	enum lm356x_tx2_polarity tx2_polarity;
	enum lm356x_hw_torch_mode hw_torch_mode;
};

#endif /* __LINUX_LEDS_LM356X_H */

