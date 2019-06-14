/*
 * Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2019, AngeloGioacchino Del Regno <kholk11@gmail.com>
 * Highly based on qpnp-adc-tm
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

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/spmi.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/iio/consumer.h>
#include "adc-tm.h"
#include "adc-tm3-common.h"
#include "../thermal_core.h"

int adc_tm3_get_temp(struct adc_tm_sensor *sensor, int *temp)
{
	int ret, milli_celsius;

	if (!sensor || !sensor->adc)
		return -EINVAL;

	ret = iio_read_channel_processed(sensor->adc, &milli_celsius);
	if (ret < 0)
		return ret;

	*temp = milli_celsius;

	return 0;
}

int32_t adc_tm3_read_reg(struct adc_tm_chip *chip,
			int16_t reg, u8 *data, int len)
{
	int ret;

	ret = regmap_bulk_read(chip->regmap, (chip->base + reg), data, len);
	if (ret < 0)
		pr_err("adc-tm read reg %d failed with %d\n", reg, ret);

	return ret;
}

int32_t adc_tm3_write_reg(struct adc_tm_chip *chip,
			int16_t reg, u8 *data, int len)
{
	int ret;

	ret = regmap_bulk_write(chip->regmap, (chip->base + reg), data, len);
	if (ret < 0)
		pr_err("adc-tm write reg %d failed with %d\n", reg, ret);

	return ret;
}

int32_t adc_tm3_reg_update(struct adc_tm_chip *chip,
			uint16_t addr, u8 mask, bool state)
{
	u8 reg_value = 0;
	int ret;

	ret = adc_tm3_read_reg(chip, addr, &reg_value, 1);
	if (ret < 0) {
		pr_err("read failed for addr:0x%x\n", addr);
		return ret;
	}

	reg_value = reg_value & ~mask;
	if (state)
		reg_value |= mask;

	pr_debug("state:%d, reg:0x%x with bits:0x%x and mask:0x%x\n",
					state, addr, reg_value, ~mask);
	ret = adc_tm3_write_reg(chip, addr, &reg_value, 1);
	if (ret < 0) {
		pr_err("write failed for addr:%x\n", addr);
		return ret;
	}

	return ret;
}

int32_t adc_tm3_enable(struct adc_tm_chip *chip)
{
	int rc = 0;
	u8 data = 0;

	data = ADC_TM_EN_BIT;
	rc = adc_tm3_write_reg(chip, ADC_TM_EN_CTL1, &data, 1);
	if (rc < 0) {
		pr_err("adc-tm enable failed\n");
		return rc;
	}

	return rc;
}

int32_t adc_tm3_request_conversion(struct adc_tm_chip *chip)
{
	int rc = 0;
	u8 data = 0;

	data = ADC_TM_CONV_REQ_SET;
	rc = adc_tm3_write_reg(chip, ADC_TM_CONV_REQ, &data, 1);
	if (rc < 0) {
		pr_err("adc-tm request conversion failed\n");
		return rc;
	}

	return rc;
}

int32_t adc_tm3_get_btm_idx(struct adc_tm_chip *chip,
			    struct adc_tm_trip_reg_type *adc_tm_ch_data,
			    uint32_t btm_chan, uint32_t *btm_chan_idx)
{
	int i;

	for (i = 0; i < ADC_TM_CHAN_NONE; i++) {
		if (adc_tm_ch_data[i].btm_amux_ch == btm_chan) {
			*btm_chan_idx = i;
			return 0;
		}
	}

	return -EINVAL;
}

int32_t adc_tm3_check_revision(struct adc_tm_chip *chip,
			u8 adc1_sel_ctl, u8 adc4_sel_ctl,
			uint32_t btm_chan_num)
{
	u8 rev, perph_subtype;
	int rc = 0;

	rc = adc_tm3_read_reg(chip, ADC_TM_REVISION3, &rev, 1);
	if (rc) {
		pr_err("adc-tm revision read failed\n");
		return rc;
	}

	rc = adc_tm3_read_reg(chip, ADC_TM_PERPH_SUBTYPE, &perph_subtype, 1);
	if (rc) {
		pr_err("adc-tm perph_subtype read failed\n");
		return rc;
	}

	if (perph_subtype == ADC_TM_PERPH_TYPE2) {
		if ((rev < ADC_TM_REV_EIGHT_CHAN_SUPPORT) &&
			(btm_chan_num > adc4_sel_ctl)) {
			pr_debug("Version does not support more than 5 channels\n");
			return -EINVAL;
		}
	}

	if (perph_subtype == ADC_TM_PERPH_SUBTYPE_TWO_CHAN_SUPPORT) {
		if (btm_chan_num > adc1_sel_ctl) {
			pr_debug("Version does not support more than 2 channels\n");
			return -EINVAL;
		}
	}

	return rc;
}

