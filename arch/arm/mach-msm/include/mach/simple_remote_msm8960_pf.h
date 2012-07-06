/* kernel/arch/arm/mach-msm/include/mach/simple_remote_msm8960_pf.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Takashi Shiina <takashi.shiina@sonyericsson.com>
 *          Tadashi Kubo <tadashi.kubo@sonyericsson.com>
 *          Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *          Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef LINUX_SIMPLE_REMOTE_MSM8960_PLATFORM
#define LINUX_SIMPLE_REMOTE_MSM8960_PLATFORM

#define SIMPLE_REMOTE_PF_NAME "simple_remote_pf"

struct regulator;

struct simple_remote_platform_regulators {
	const char *name;
	struct regulator *reg;
	int vol_min;
	int vol_max;
};

struct simple_remote_platform_data {
	unsigned int headset_detect_enable_pin;
	unsigned int headset_detect_read_pin;
	int button_detect_irq;
	uint32_t adc_channel;
	unsigned int hsed_base;
	int (*initialize)(struct simple_remote_platform_data *);
	void (*deinitialize)(struct simple_remote_platform_data *);
	int invert_plug_det; /* Default is 0 = plug inserted */
	struct simple_remote_platform_regulators *regs;
	u16 num_regs;
};

#endif /* LINUX_SIMPLE_REMOTE_MSM8960_PLATFORM */
