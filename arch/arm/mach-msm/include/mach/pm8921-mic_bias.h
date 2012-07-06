/* kernel/arch/arm/mach-msm/include/mach/pm8921-mic_bias.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Shuhei Miyazaki <Shuhei.Miyazaki@sonyericsson.com>
 *          Hiroaki Hayashi <Hiroaki.Hayashi@sonyericsson.com>
 *          Daiki Yanagihara <Daiki.X.Yanagihara@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _PM8921_MIC_BIAS_H
#define _PM8921_MIC_BIAS_H

#define PM8921_MIC_BIAS_NAME "pmic8921-mic_bias"

extern struct pm8921_mic_bias_platform_data pm8921_mic_bias_pf_data;

struct pm8921_mic_bias_platform_data {
	unsigned int mic_bias_addr;
};

int pm8921_mic_bias_enable(bool enable);

#endif /* _PM8921_MIC_BIAS_H */

