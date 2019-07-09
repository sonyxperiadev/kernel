/*
 * Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2019, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include "adc-tm.h"

/* Misc definitions */
#define ADC_TM3_625_UV			625000
#define ADC_TM3_RATIOMETRIC_RANGE	1800
#define ADC_TM3_RATIOMETRIC_RANGE_8998	1875

/* Common registers */
#define ADC_TM_REVISION3		0x02
#define ADC_TM_PERPH_SUBTYPE		0x5
#define ADC_TM_PERPH_SUBTYPE_TWO_CHAN_SUPPORT 0x22
#define ADC_TM_PERPH_TYPE2		0x2
#define ADC_TM_REV_EIGHT_CHAN_SUPPORT	2

#define ADC_TM_STATUS1			0x08
#define ADC_TM_STATUS_LOW		0x0a
#define ADC_TM_STATUS_HIGH		0x0b
#define ADC_TM_NUM_BTM			0x0f

#define ADC_TM_EN_CTL1			0x46
#define ADC_TM_EN_BIT			BIT(7)

#define ADC_TM_CONV_REQ			0x52
#define ADC_TM_CONV_REQ_SET		BIT(7)

#define ADC_TM_LOWER_MASK(n)		((n) & 0x000000ff)
#define ADC_TM_UPPER_MASK(n)		(((n) & 0xffffff00) >> 8)

/* Common functions */
int adc_tm3_get_temp(struct adc_tm_sensor *sensor, int *temp);
int32_t adc_tm3_read_reg(struct adc_tm_chip *chip,
			int16_t reg, u8 *data, int len);
int32_t adc_tm3_write_reg(struct adc_tm_chip *chip,
			int16_t reg, u8 *data, int len);
int32_t adc_tm3_reg_update(struct adc_tm_chip *chip,
			uint16_t addr, u8 mask, bool state);

int32_t adc_tm3_enable(struct adc_tm_chip *chip);
int32_t adc_tm3_request_conversion(struct adc_tm_chip *chip);

int32_t adc_tm3_get_btm_idx(struct adc_tm_chip *chip,
			    struct adc_tm_trip_reg_type *adc_tm_ch_data,
			    uint32_t btm_chan, uint32_t *btm_chan_idx);

int32_t adc_tm3_check_revision(struct adc_tm_chip *chip,
			u8 adc1_sel_ctl, u8 adc4_sel_ctl,
			uint32_t btm_chan_num);
