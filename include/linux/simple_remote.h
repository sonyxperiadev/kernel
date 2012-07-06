/* kernel/include/linux/simple_remote.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Takashi Shiina <takashi.shiina@sonyericsson.com>
 *          Tadashi Kubo <tadashi.kubo@sonyericsson.com>
 *          Joachim Holst <joachim.holst@sonyericsson.com>
 *          Torbjorn Eklund <torbjorn.eklung@sonyericsson.com>
 *          Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef LINUX_SIMPLE_REMOTE
#define LINUX_SIMPLE_REMOTE

#include <linux/interrupt.h>

#define SIMPLE_REMOTE_NAME         "simple_remote"
#define SIMPLE_REMOTE_APPKEY_NAME  "simple_remote_appkey"

struct simple_remote_pf_interface {
	int (*read_hs_adc)		(unsigned int *adc_value);
	int (*enable_mic_bias)		(unsigned int enable);
	int (*get_current_plug_status)	(u8 *status);

	int (*enable_alternate_adc_mode)	(u8 enable);
	int (*enable_alternate_headset_mode)	(u8 enable);

	int (*set_period_freq)		(unsigned int value);
	int (*set_period_time)		(unsigned int value);
	int (*set_hysteresis_freq)	(unsigned int value);
	int (*set_hysteresis_time)	(unsigned int value);
	int (*set_trig_level)		(unsigned int value);

	int (*get_period_freq)		(unsigned int *value);
	int (*get_period_time)		(unsigned int *value);
	int (*get_hysteresis_freq)	(unsigned int *value);
	int (*get_hysteresis_time)	(unsigned int *value);
	int (*get_trig_level)		(unsigned int *value);

	int  (*register_plug_detect_interrupt)	(irq_handler_t func,
						 void *data);
	void (*unregister_plug_detect_interrupt)(void *data);

	int  (*register_hssd_button_interrupt)	(irq_handler_t func,
						 void *data);
	void (*unregister_hssd_button_interrupt)(void *data);
};

#endif /* LINUX_SIMPLE_REMOTE */
