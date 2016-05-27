/*
 * leds-as3668.h - platform data structure for as3668 led controller
 *
 * Copyright (C) 2012 Florian Lobmaier <florian.lobmaier@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#ifndef __LINUX_AS3668_H
#define __LINUX_AS3668_H

#define AS3668_LED_COLOR_RED 1
#define AS3668_LED_COLOR_GREEN 2
#define AS3668_LED_COLOR_BLUE 3

#define AS3668_PATTERN_START_SOURCE_SW 0
#define AS3668_PATTERN_START_SOURCE_GPIO 1

#define AS3668_PWM_SOURCE_INTERNAL 0
#define AS3668_PWM_SOURCE_EXTERNAL 1

#define AS3668_GPIO_INPUT_MODE_ANALOG 0
#define AS3668_GPIO_INPUT_MODE_DIGITAL 1

#define AS3668_GPIO_MODE_INPUT_ONLY 0
#define AS3668_GPIO_MODE_OUTPUT 1

#define AS3668_AUDIO_INPUT_GPIO 0
#define AS3668_AUDIO_INPUT_CURR4 1
#define AS3668_AUDIO_INPUT_CAP_PRECHARGE 0
#define AS3668_AUDIO_INPUT_CAP_NO_PRECHARGE 1
#define AS3668_AUDIO_AUTO_PRECHARGE 0
#define AS3668_AUDIO_MANUAL_PRECHARGE 1

struct as3668_platform_led {
	const char *name; /* if NULL the default name is used */
	u16 max_current_uA; /* This leds maximum current in mA */
};

struct as3668_platform_data {
	struct as3668_platform_led leds[4]; /* curr1,2,3,4 */
	u16 vbat_monitor_voltage_mV; /* device enters shutdown if Vbat drops
				      * below the given voltage */
	u8 pattern_start_source;
	u8 pwm_source;
	u8 gpio_input_invert;
	u8 gpio_input_mode;
	u8 gpio_mode;
	u8 audio_input_pin;
	u8 audio_pulldown_off;
	u8 audio_dis_start;
	u8 audio_man_start;
	u8 audio_adc_characteristic;
};
#endif /* __LINUX_AS3668_H */
