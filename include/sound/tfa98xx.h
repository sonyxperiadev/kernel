/* Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __TFA98XX_H
#define __TFA98XX_H

struct tfa98xx_param_data {
	unsigned int size;
	unsigned int type;
	unsigned char *data;
};

#define TFA98XX_IOCTL_MAGIC 't'

#define TFA98XX_PATCH_PARAM \
	_IOW(TFA98XX_IOCTL_MAGIC, 0, struct tfa98xx_param_data)
#define TFA98XX_CONFIG_PARAM \
	_IOW(TFA98XX_IOCTL_MAGIC, 1, struct tfa98xx_param_data)
#define TFA98XX_SPEAKER_PARAM \
	_IOW(TFA98XX_IOCTL_MAGIC, 2, struct tfa98xx_param_data)
#define TFA98XX_PRESET_PARAM \
	_IOW(TFA98XX_IOCTL_MAGIC, 3, struct tfa98xx_param_data)
#define TFA98XX_EQ_PARAM \
	_IOW(TFA98XX_IOCTL_MAGIC, 4, struct tfa98xx_param_data)

#define PARAM_SIZE_MAX (16 * 1024)

enum {
	PATCH_DSP = 0,
	PATCH_COLDBOOT,
	PATCH_MAX
};

enum {
	AMP_TOP = 0,
	AMP_BOTTOM,
	AMP_RECEIVER,
	AMP_MAX
};

enum {
	TYPE_HIFISPEAKER_TOP = 0,
	TYPE_HIFISPEAKER_BOTTOM,
	TYPE_HIFISPEAKER_RING_TOP,
	TYPE_HIFISPEAKER_RING_BOTTOM,
	TYPE_HIFISPEAKER_SFORCE_TOP,
	TYPE_HIFISPEAKER_SFORCE_BOTTOM,
	TYPE_VOICECALLSPEAKER_TOP,
	TYPE_VOICECALLSPEAKER_BOTTOM,
	TYPE_FMSPEAKER_TOP,
	TYPE_FMSPEAKER_BOTTOM,
	TYPE_VOICECALLEARPICE_TOP,
	TYPE_MAX
};

#endif
