/*
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Shuhei Miyazaki <Shuhei.Miyazaki@sonyericsson.com>
 *          Naoki Miyazu <Naoki.X.Miyazu@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _PM8058_MIC_BIAS_H
#define _PM8058_MIC_BIAS_H

#define PM8058_MIC_BIAS_NAME "pmic8058-mic_bias"

extern struct pm8058_mic_bias_platform_data pm8058_mic_bias_pf_data;

struct pm8058_mic_bias_platform_data {
	unsigned int mic_bias_addr;
};

int pm8058_mic_bias_enable(bool enable);

#endif /* _PM8058_MIC_BIAS_H */
