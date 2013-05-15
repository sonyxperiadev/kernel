/* include/linux/lm3561.h
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Angela Fox <angela.fox@sonymobile.com>
 * Author: Aleksej Makarov <aleksej.makarov@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
*/

#ifndef __MISC_LM3561_H
#define __MISC_LM3561_H

#include <linux/types.h>

/* Trigger of strobe */
enum lm3561_strobe_trigger {
	LM3561_STROBE_TRIGGER_LEVEL,
	LM3561_STROBE_TRIGGER_EDGE
};

/* Flash H/W synchronization control */
enum lm3561_sync_state {
	LM3561_SYNC_OFF,
	LM3561_SYNC_ON
};

/* Strobe polarity */
enum lm3561_strobe_polarity {
	LM3561_STROBE_POLARITY_LOW,
	LM3561_STROBE_POLARITY_HIGH
};

/* Setting of LEDI/NTC pin */
enum lm3561_ledintc_pin_setting {
	LM3561_LEDINTC_INDICATOR_OUTPUT,
	LM3561_LEDINTC_NTC_THERMISTOR_INPUT
};

/* TX1 polarity */
enum lm3561_tx1_polarity {
	LM3561_TX1_POLARITY_LOW,
	LM3561_TX1_POLARITY_HIGH
};

/* TX2 polarity */
enum lm3561_tx2_polarity {
	LM3561_TX2_POLARITY_LOW,
	LM3561_TX2_POLARITY_HIGH
};

/* Enable/Disable H/W torch mode */
enum lm3561_hw_torch_mode {
	LM3561_HW_TORCH_MODE_DISABLE,
	LM3561_HW_TORCH_MODE_ENABLE
};

/*
 * Setting of the restore to privacy mode
 * at the end of the flash pulse
 */
enum lm3561_privacy_terminate {
	LM3561_PRIVACY_MODE_TURN_BACK,
	LM3561_PRIVACY_MODE_REMAIN_OFF
};

#define LM3561_DRV_NAME "lm3561"
struct lm3561_platform_data {
	int (*power)(struct device *dev, bool request);
	int (*platform_init)(struct device *dev, bool request);
	int led_nums; /* The number of the LED for Torch/Flash mode */
	enum lm3561_strobe_trigger strobe_trigger;
	unsigned long current_limit;
	enum lm3561_sync_state flash_sync;
	enum lm3561_strobe_polarity strobe_polarity;
	enum lm3561_ledintc_pin_setting ledintc_pin_setting;
	enum lm3561_tx1_polarity tx1_polarity;
	enum lm3561_tx2_polarity tx2_polarity;
	enum lm3561_hw_torch_mode hw_torch_mode;
};
#endif /* __LINUX_LEDS_LM3561_H */

