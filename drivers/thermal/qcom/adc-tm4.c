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

/* ADC TM (HC) definitions */
#define ADC_TM_ADC_DIG_PARAM		0x42
#define ADC_TM_FAST_AVG_CTL		0x43

#define ADC_TM_BTM_CONV_REQ		0x47
#define ADC_TM_BTM_CONV_REQ_EN		BIT(7)

#define ADC_TM_MEAS_INTERVAL_CTL	0x50
#define ADC_TM_MEAS_INTERVAL_TIME_SHIFT	0x3
#define ADC_TM_MEAS_INTERVAL_CTL2	0x51
#define ADC_TM_MEAS_INTERVAL_CTL2_SHIFT	0x4
#define ADC_TM_MEAS_INTERVAL_CTL2_MASK	0xf0
#define ADC_TM_MEAS_INTERVAL_CTL3_MASK	0xf

#define ADC_TM_Mn_ADC_CH_SEL_CTL(n)	((n * 8) + 0x60)
#define ADC_TM_Mn_LOW_THR0(n)		((n * 8) + 0x61)
#define ADC_TM_Mn_LOW_THR1(n)		((n * 8) + 0x62)
#define ADC_TM_Mn_HIGH_THR0(n)		((n * 8) + 0x63)
#define ADC_TM_Mn_HIGH_THR1(n)		((n * 8) + 0x64)
#define ADC_TM_Mn_MEAS_INTERVAL_CTL(n)	((n * 8) + 0x65)
#define ADC_TM_Mn_CTL(n)		((n * 8) + 0x66)
#define ADC_TM_Mn_EN(n)			((n * 8) + 0x67)
#define ADC_TM_CTL_HW_SETTLE_DELAY_MASK	0xf
#define ADC_TM_CTL_CAL_SEL		0x30
#define ADC_TM_CTL_CAL_SEL_MASK_SHIFT	4

#define ADC_TM_Mn_DATA0(n)		((n * 2) + 0xa0)
#define ADC_TM_Mn_DATA1(n)		((n * 2) + 0xa1)
#define ADC_TM_DATA_SHIFT		8
/*
 * On HC, this control register is present per-channel so, to enable
 * features, we set the feature bit on the channel specific EN register.
 */
#define ADC_TM_Mn_MEAS_EN_CTL(n)	ADC_TM_Mn_EN(n)
#define ADC_TM_Mn_LOW_THR_INT_EN_CTL(n)	ADC_TM_Mn_EN(n)
#define ADC_TM_Mn_HIGH_THR_INT_EN_CTL(n) ADC_TM_Mn_EN(n)
#define ADC_TM_Mn_LOW_THR_INT_EN(n)	BIT(0)
#define ADC_TM_Mn_HIGH_THR_INT_EN(n)	BIT(1)
#define ADC_TM_Mn_MEAS_EN(n)		BIT(7)

static struct adc_tm_linear_graph cal_graph[ADC_ABS_CAL + 1];
static struct adc_tm_reverse_scale_fn *adc_tm_rscale_fn;

static struct adc_tm_trip_reg_type adc_tm_ch_data[] = {
	[ADC_TM_CHAN0] = {ADC_TM_Mn_ADC_CH_SEL_CTL(0)},
	[ADC_TM_CHAN1] = {ADC_TM_Mn_ADC_CH_SEL_CTL(1)},
	[ADC_TM_CHAN2] = {ADC_TM_Mn_ADC_CH_SEL_CTL(2)},
	[ADC_TM_CHAN3] = {ADC_TM_Mn_ADC_CH_SEL_CTL(3)},
	[ADC_TM_CHAN4] = {ADC_TM_Mn_ADC_CH_SEL_CTL(4)},
	[ADC_TM_CHAN5] = {ADC_TM_Mn_ADC_CH_SEL_CTL(5)},
	[ADC_TM_CHAN6] = {ADC_TM_Mn_ADC_CH_SEL_CTL(6)},
	[ADC_TM_CHAN7] = {ADC_TM_Mn_ADC_CH_SEL_CTL(7)},
};

static int32_t adc_tm4_absolute_rthr(const struct adc_tm_data *data,
			struct adc_tm_config *tm_config)
{
	int64_t low_thr = 0, high_thr = 0;
	int sign = 0;

	pr_debug("high_volt:%llu, low_volt:%llu\n",
			tm_config->high_thr_voltage,
			tm_config->low_thr_voltage);

	/* Numerator is always 1, prescal is the denominator */
	low_thr = div_s64(tm_config->low_thr_voltage, tm_config->prescal);
	low_thr -= ADC_TM3_625_UV;
	low_thr *= cal_graph[ADC_ABS_CAL].dy;

	if (low_thr < 0) {
		sign = 1;
		low_thr = -low_thr;
	}

	low_thr = div64_s64(low_thr, ADC_TM3_625_UV);
	if (sign)
		low_thr = -low_thr;
	tm_config->low_thr_voltage = low_thr + cal_graph[ADC_ABS_CAL].gnd;

	sign = 0;
	high_thr = div_s64(tm_config->high_thr_voltage, tm_config->prescal);
	low_thr -= ADC_TM3_625_UV;
	low_thr *= cal_graph[ADC_ABS_CAL].dy;
	if (high_thr < 0) {
		sign = 1;
		high_thr = -high_thr;
	}
	high_thr = high_thr * tm_config->prescal;
	high_thr = div64_s64(high_thr, ADC_TM3_625_UV);
	if (sign)
		high_thr = -high_thr;
	tm_config->high_thr_voltage = high_thr + cal_graph[ADC_ABS_CAL].gnd;

	pr_debug("adc_code_high:%x, adc_code_low:%x\n",
			abs(tm_config->high_thr_voltage),
			abs(tm_config->low_thr_voltage));

	return 0;
}

static int32_t adc_tm4_enable(struct adc_tm_chip *chip)
{
	int rc = 0;
	u8 data = 0;

	rc = adc_tm3_enable(chip);
	if (rc < 0)
		return rc;

	/* Extra enable sequence for HC: enable channel conversion */
	data = ADC_TM_BTM_CONV_REQ_EN;
	rc = adc_tm3_write_reg(chip, ADC_TM_BTM_CONV_REQ, &data, 1);
	if (rc < 0) {
		pr_err("adc-tm request conversion failed\n");
		return rc;
	}

	return rc;
}

static int adc_tm4_configure(struct adc_tm_sensor *sensor,
					uint32_t btm_chan_idx)
{
	struct adc_tm_chip *chip = sensor->chip;
	u8 buf[8], tmp = 0, cal_sel = 0;
	int ret = 0;

	/* Read current fast average parameters */
	ret = adc_tm3_read_reg(chip, ADC_TM_FAST_AVG_CTL, &tmp, 1);
	if (ret < 0) {
		pr_err("adc-tm fast average read failed with %d\n", ret);
		return ret;
	}

	/* Setup fast average delay and enable */
	tmp |= sensor->hw_settle_time;
	ret = adc_tm3_write_reg(chip, ADC_TM_FAST_AVG_CTL, &tmp, 1);
	if (ret < 0) {
		pr_err("adc-tm fast average enable failed with %d\n", ret);
		return ret;
	}

	/* Read current amux selection registers for this channel */
	ret = adc_tm3_read_reg(chip,
			ADC_TM_Mn_ADC_CH_SEL_CTL(btm_chan_idx), buf, 8);
	if (ret < 0) {
		pr_err("adc-tm block read failed with %d\n", ret);
		return ret;
	}

	/* Update ADC channel select for the corresponding BTM channel */
	tmp = sensor->adc_ch;
	ret = adc_tm3_write_reg(chip,
			ADC_TM_Mn_ADC_CH_SEL_CTL(btm_chan_idx), &tmp, 1);
	if (ret < 0) {
		pr_err("adc-tm amux sel write failed with %d\n", ret);
		return ret;
	}

	/* Set calibration select, hw_settle delay */
	cal_sel = (u8) (sensor->cal_sel << ADC_TM_CTL_CAL_SEL_MASK_SHIFT);
	buf[6] &= (u8) ~ADC_TM_CTL_HW_SETTLE_DELAY_MASK;
	buf[6] |= (u8) sensor->hw_settle_time;
	buf[6] &= (u8) ~ADC_TM_CTL_CAL_SEL;
	buf[6] |= (u8) cal_sel;

	ret = adc_tm3_write_reg(chip, ADC_TM_Mn_CTL(btm_chan_idx), &buf[6], 1);
	if (ret < 0) {
		pr_err("adc-tm hw-settle, calib sel write failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static int32_t adc_tm_add_to_list(struct adc_tm_chip *chip,
				uint32_t dt_index,
				struct adc_tm_param *param)
{
	struct adc_tm_client_info *client_info = NULL;
	bool client_info_exists = false;

	list_for_each_entry(client_info,
			&chip->sensor[dt_index].thr_list, list) {
		if (client_info->param == param) {
			client_info->low_thr_requested = param->low_thr;
			client_info->high_thr_requested = param->high_thr;
			client_info->state_request = param->state_request;
			client_info->notify_low_thr = false;
			client_info->notify_high_thr = false;
			client_info_exists = true;
			pr_debug("client found\n");
		}
	}

	if (!client_info_exists) {
		client_info = devm_kzalloc(chip->dev,
			sizeof(struct adc_tm_client_info), GFP_KERNEL);
		if (!client_info)
			return -ENOMEM;

		pr_debug("new client\n");
		client_info->param = param;
		client_info->low_thr_requested = param->low_thr;
		client_info->high_thr_requested = param->high_thr;
		client_info->state_request = param->state_request;

		list_add_tail(&client_info->list,
					&chip->sensor[dt_index].thr_list);
	}
	return 0;
}

static int32_t adc_tm4_thr_update(struct adc_tm_sensor *sensor,
			int32_t high_thr, int32_t low_thr)
{
	int ret = 0;
	u8 trip_low_thr[2], trip_high_thr[2];
	uint16_t reg_low_thr_lsb, reg_high_thr_lsb;
	uint16_t reg_low_thr_msb, reg_high_thr_msb;
	uint32_t scale_type = 0, mask = 0, btm_chan_idx = 0;
	struct adc_tm_config tm_config;
	struct adc_tm_chip *chip = sensor->chip;

	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
		sensor->btm_ch, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx\n");
		return ret;
	}

	tm_config.high_thr_voltage = (int64_t)high_thr;
	tm_config.low_thr_voltage = (int64_t)low_thr;
	tm_config.prescal = sensor->prescaling;

	scale_type = sensor->adc_rscale_fn;
	if (scale_type >= SCALE_RSCALE_NONE) {
		ret = -EBADF;
		return ret;
	}

	adc_tm_rscale_fn[scale_type].chan(chip->data, &tm_config);

	mask = lower_32_bits(tm_config.high_thr_voltage);
	trip_high_thr[0] = ADC_TM_LOWER_MASK(mask);
	trip_high_thr[1] = ADC_TM_UPPER_MASK(mask);

	mask = lower_32_bits(tm_config.low_thr_voltage);
	trip_low_thr[0] = ADC_TM_LOWER_MASK(mask);
	trip_low_thr[1] = ADC_TM_UPPER_MASK(mask);

	pr_debug("high_thr:0x%llx, low_thr:0x%llx\n",
		tm_config.high_thr_voltage, tm_config.low_thr_voltage);

	reg_low_thr_lsb = ADC_TM_Mn_LOW_THR0(btm_chan_idx);
	reg_high_thr_lsb = ADC_TM_Mn_HIGH_THR0(btm_chan_idx);
	reg_low_thr_msb = ADC_TM_Mn_LOW_THR1(btm_chan_idx);
	reg_high_thr_msb = ADC_TM_Mn_HIGH_THR1(btm_chan_idx);

	if (low_thr != INT_MIN) {
		ret = adc_tm3_write_reg(chip, reg_low_thr_lsb,
						&trip_low_thr[0], 1);
		if (ret) {
			pr_err("Warm LSB set threshold err\n");
			goto fail;
		}

		ret = adc_tm3_write_reg(chip, reg_low_thr_msb,
						&trip_low_thr[1], 1);
		if (ret) {
			pr_err("Warm MSB set threshold err\n");
			goto fail;
		}
	}

	if (high_thr != INT_MAX) {
		ret = adc_tm3_write_reg(chip, reg_high_thr_lsb,
						&trip_high_thr[0], 1);
		if (ret) {
			pr_err("adc-tm cool temp set threshold err\n");
			goto fail;
		}

		ret = adc_tm3_write_reg(chip, reg_high_thr_msb,
						&trip_high_thr[1], 2);
		if (ret) {
			pr_err("adc-tm cool temp set threshold err\n");
			goto fail;
		}
	}
fail:
	return ret;
}

static int32_t adc_tm4_manage_thresholds(struct adc_tm_sensor *sensor)
{
	int ret = 0, high_thr = INT_MAX, low_thr = INT_MIN;
	struct adc_tm_client_info *client_info = NULL;
	struct list_head *thr_list;
	uint32_t btm_chan_idx = 0;
	struct adc_tm_chip *chip = sensor->chip;

	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
				  sensor->btm_ch, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx with %d\n", ret);
		return ret;
	}
	/*
	 * Reset the high_thr_set and low_thr_set of all
	 * clients since the thresholds will be recomputed.
	 */
	list_for_each(thr_list, &sensor->thr_list) {
		client_info = list_entry(thr_list,
					struct adc_tm_client_info, list);
		client_info->high_thr_set = false;
		client_info->low_thr_set = false;
	}

	/* Find the min of high_thr and max of low_thr */
	list_for_each(thr_list, &sensor->thr_list) {
		client_info = list_entry(thr_list,
					struct adc_tm_client_info, list);

		if ((client_info->state_request == ADC_TM_HIGH_THR_ENABLE) ||
			(client_info->state_request ==
				ADC_TM_HIGH_LOW_THR_ENABLE))
			if (client_info->high_thr_requested < high_thr)
				high_thr = client_info->high_thr_requested;

		if ((client_info->state_request == ADC_TM_LOW_THR_ENABLE) ||
			(client_info->state_request ==
				ADC_TM_HIGH_LOW_THR_ENABLE))
			if (client_info->low_thr_requested > low_thr)
				low_thr = client_info->low_thr_requested;

		pr_debug("threshold compared is high:%d and low:%d\n",
				client_info->high_thr_requested,
				client_info->low_thr_requested);
		pr_debug("current threshold is high:%d and low:%d\n",
							high_thr, low_thr);
	}

	/* Check which of the high_thr and low_thr got set */
	list_for_each(thr_list, &sensor->thr_list) {
		client_info = list_entry(thr_list,
					struct adc_tm_client_info, list);

		if ((client_info->state_request == ADC_TM_HIGH_THR_ENABLE) ||
			(client_info->state_request ==
				ADC_TM_HIGH_LOW_THR_ENABLE))
			if (high_thr == client_info->high_thr_requested)
				client_info->high_thr_set = true;

		if ((client_info->state_request == ADC_TM_LOW_THR_ENABLE) ||
			(client_info->state_request ==
				ADC_TM_HIGH_LOW_THR_ENABLE))
			if (low_thr == client_info->low_thr_requested)
				client_info->low_thr_set = true;
	}

	ret = adc_tm4_thr_update(sensor, high_thr, low_thr);
	if (ret < 0)
		pr_err("setting chan:%d threshold failed\n", btm_chan_idx);

	pr_debug("threshold written is high:%d and low:%d\n",
							high_thr, low_thr);

	return 0;
}

static void notify_adc_tm4_fn(struct work_struct *work)
{
	struct adc_tm_client_info *client_info = NULL;
	struct adc_tm_chip *chip;
	struct list_head *thr_list;
	uint32_t btm_chan_num = 0, btm_chan_idx = 0;
	int ret = 0;

	struct adc_tm_sensor *adc_tm = container_of(work,
		struct adc_tm_sensor, work);

	chip = adc_tm->chip;

	btm_chan_num = adc_tm->btm_ch;
	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
				  btm_chan_num, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx\n");
		return;
	}

	mutex_lock(&chip->adc_mutex_lock);

	if (adc_tm->low_thr_triggered) {
		/* adjust thr, calling manage_thr */
		list_for_each(thr_list, &adc_tm->thr_list) {
			client_info = list_entry(thr_list,
					struct adc_tm_client_info, list);
			if (client_info->low_thr_set) {
				client_info->low_thr_set = false;
				client_info->notify_low_thr = true;
				if (client_info->state_request ==
						ADC_TM_HIGH_LOW_THR_ENABLE)
					client_info->state_request =
							ADC_TM_HIGH_THR_ENABLE;
				else
					client_info->state_request =
							ADC_TM_LOW_THR_DISABLE;
			}
		}
		adc_tm4_manage_thresholds(adc_tm);

		adc_tm->low_thr_triggered = false;
	}

	if (adc_tm->high_thr_triggered) {
		/* adjust thr, calling manage_thr */
		list_for_each(thr_list, &adc_tm->thr_list) {
			client_info = list_entry(thr_list,
					struct adc_tm_client_info, list);
			if (client_info->high_thr_set) {
				client_info->high_thr_set = false;
				client_info->notify_high_thr = true;
				if (client_info->state_request ==
						ADC_TM_HIGH_LOW_THR_ENABLE)
					client_info->state_request =
							ADC_TM_LOW_THR_ENABLE;
				else
					client_info->state_request =
							ADC_TM_HIGH_THR_DISABLE;
			}
		}
		adc_tm4_manage_thresholds(adc_tm);

		adc_tm->high_thr_triggered = false;
	}
	mutex_unlock(&chip->adc_mutex_lock);

	list_for_each_entry(client_info, &adc_tm->thr_list, list) {
		if (client_info->notify_low_thr) {
			if (client_info->param->threshold_notification
								!= NULL) {
				pr_debug("notify kernel with low state\n");
				client_info->param->threshold_notification(
					ADC_TM_LOW_STATE,
					client_info->param->btm_ctx);
				client_info->notify_low_thr = false;
			}
		}

		if (client_info->notify_high_thr) {
			if (client_info->param->threshold_notification
								!= NULL) {
				pr_debug("notify kernel with high state\n");
				client_info->param->threshold_notification(
					ADC_TM_HIGH_STATE,
					client_info->param->btm_ctx);
				client_info->notify_high_thr = false;
			}
		}
	}
}

int32_t adc_tm4_channel_measure(struct adc_tm_chip *chip,
					struct adc_tm_param *param)

{
	int ret = 0, i = 0;
	uint32_t channel, dt_index = 0, btm_chan_idx = 0;
	bool chan_found = false, high_thr_set = false, low_thr_set = false;
	struct adc_tm_client_info *client_info = NULL;

	ret = adc_tm_is_valid(chip);
	if (ret || (param == NULL))
		return -EINVAL;

	if (param->threshold_notification == NULL) {
		pr_debug("No notification for high/low temp\n");
		return -EINVAL;
	}

	mutex_lock(&chip->adc_mutex_lock);

	channel = param->channel;

	while (i < chip->dt_channels) {
		if (chip->sensor[i].adc_ch == channel) {
			dt_index = i;
			chan_found = true;
			break;
		}
		i++;
	}

	if (!chan_found)  {
		pr_err("not a valid ADC_TM channel\n");
		ret = -EINVAL;
		goto fail_unlock;
	}

	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
		chip->sensor[dt_index].btm_ch, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx with %d\n", ret);
		goto fail_unlock;
	}

	/* add channel client to channel list */
	adc_tm_add_to_list(chip, dt_index, param);

	/* set right thresholds for the sensor */
	adc_tm4_manage_thresholds(&chip->sensor[dt_index]);

	/* enable low/high irqs */
	list_for_each_entry(client_info,
			&chip->sensor[dt_index].thr_list, list) {
		if (client_info->high_thr_set == true)
			high_thr_set = true;
		if (client_info->low_thr_set == true)
			low_thr_set = true;
	}

	if (low_thr_set) {
		/* Enable low threshold's interrupt */
		pr_debug("low sensor:%x with state:%d\n",
				dt_index, param->state_request);
		ret = adc_tm3_reg_update(chip,
			ADC_TM_Mn_LOW_THR_INT_EN_CTL(btm_chan_idx),
			ADC_TM_Mn_LOW_THR_INT_EN(btm_chan_idx), true);
		if (ret < 0) {
			pr_err("low thr enable err:%d\n",
				chip->sensor[dt_index].btm_ch);
			goto fail_unlock;
		}
	}

	if (high_thr_set) {
		/* Enable high threshold's interrupt */
		pr_debug("high sensor mask:%x with state:%d\n",
			dt_index, param->state_request);
		ret = adc_tm3_reg_update(chip,
			ADC_TM_Mn_HIGH_THR_INT_EN_CTL(btm_chan_idx),
			ADC_TM_Mn_HIGH_THR_INT_EN(btm_chan_idx), true);
		if (ret < 0) {
			pr_err("high thr enable err:%d\n",
				chip->sensor[dt_index].btm_ch);
			goto fail_unlock;
		}
	}

	/* configure channel */
	ret = adc_tm4_configure(&chip->sensor[dt_index], btm_chan_idx);
	if (ret < 0) {
		pr_err("Error during adc-tm configure:%d\n", ret);
		goto fail_unlock;
	}

	ret = adc_tm4_enable(chip);
	if (ret < 0)
		pr_err("Error enabling adc-tm with %d\n", ret);

	ret = adc_tm3_request_conversion(chip);
	if (ret < 0)
		pr_err("Error requesting conversion: %d\n", ret);

fail_unlock:
	mutex_unlock(&chip->adc_mutex_lock);
	return ret;
}
EXPORT_SYMBOL(adc_tm4_channel_measure);

int32_t adc_tm4_disable_chan_meas(struct adc_tm_chip *chip,
					struct adc_tm_param *param)
{
	int ret = 0, i = 0;
	uint32_t channel, dt_index = 0, btm_chan_idx = 0;
	unsigned long flags;

	ret = adc_tm_is_valid(chip);
	if (ret || (param == NULL))
		return -EINVAL;

	channel = param->channel;

	while (i < chip->dt_channels) {
		if (chip->sensor[i].adc_ch == channel) {
			dt_index = i;
			break;
		}
		i++;
	}

	if (i == chip->dt_channels)  {
		pr_err("not a valid ADC_TM channel\n");
		return -EINVAL;
	}

	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
		chip->sensor[dt_index].btm_ch, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx with %d\n", ret);
		return ret;
	}

	spin_lock_irqsave(&chip->adc_tm_lock, flags);

	ret = adc_tm3_reg_update(chip,
			ADC_TM_Mn_HIGH_THR_INT_EN_CTL(btm_chan_idx),
			ADC_TM_Mn_HIGH_THR_INT_EN(btm_chan_idx), false);
	if (ret < 0) {
		pr_err("high thr disable err\n");
		goto fail;
	}

	ret = adc_tm3_reg_update(chip,
			ADC_TM_Mn_LOW_THR_INT_EN_CTL(btm_chan_idx),
			ADC_TM_Mn_LOW_THR_INT_EN(btm_chan_idx), false);
	if (ret < 0) {
		pr_err("low thr disable err\n");
		goto fail;
	}

	ret = adc_tm3_reg_update(chip, ADC_TM_Mn_MEAS_EN_CTL(btm_chan_idx),
			ADC_TM_Mn_MEAS_EN(btm_chan_idx), false);
	if (ret < 0)
		pr_err("multi measurement disable failed\n");

fail:
	spin_unlock_irqrestore(&chip->adc_tm_lock, flags);
	return ret;
}
EXPORT_SYMBOL(adc_tm4_disable_chan_meas);

static int adc_tm4_set_mode(struct adc_tm_sensor *sensor,
			      enum thermal_device_mode mode)
{
	struct adc_tm_chip *chip = sensor->chip;
	int ret = 0;
	uint32_t btm_chan_idx = 0;

	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
				  sensor->btm_ch, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx with %d\n", ret);
		return ret;
	}

	if (mode == THERMAL_DEVICE_ENABLED) {
		ret = adc_tm4_configure(sensor, btm_chan_idx);
		if (ret < 0) {
			pr_err("Error during adc-tm configure:%d\n", ret);
			return ret;
		}

		ret = adc_tm4_enable(chip); {
		if (ret < 0)
			pr_err("Error enabling adc-tm with %d\n", ret);
			return ret;
		}

		ret = adc_tm3_request_conversion(chip);
		if (ret < 0)
			pr_err("Error requesting conversion: %d\n", ret);

	} else if (mode == THERMAL_DEVICE_DISABLED) {
		ret = adc_tm3_reg_update(chip,
				ADC_TM_Mn_MEAS_EN_CTL(btm_chan_idx),
				ADC_TM_Mn_MEAS_EN(btm_chan_idx), false);
		if (ret < 0) {
			pr_err("Disable failed for ch:%d\n", btm_chan_idx);
			return ret;
		}
	}

	return ret;
}

static int adc_tm4_activate_trip_type(struct adc_tm_sensor *adc_tm,
			int trip, enum thermal_device_mode mode)
{
	struct adc_tm_chip *chip = adc_tm->chip;
	int ret = 0;
	bool state = false;
	uint32_t btm_chan_idx = 0, btm_chan = 0;

	if (mode == THERMAL_DEVICE_ENABLED)
		state = true;

	btm_chan = adc_tm->btm_ch;
	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
				  btm_chan, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx\n");
		return ret;
	}

	switch (trip) {
	case THERMAL_TRIP_CONFIGURABLE_HI:
		/* low_thr (lower voltage) for higher temp */
		ret = adc_tm3_reg_update(chip,
				ADC_TM_Mn_LOW_THR_INT_EN_CTL(btm_chan_idx),
				ADC_TM_Mn_LOW_THR_INT_EN(btm_chan_idx),
				state);
		if (ret)
			pr_err("channel:%x failed\n", btm_chan);
	break;
	case THERMAL_TRIP_CONFIGURABLE_LOW:
		/* high_thr (higher voltage) for cooler temp */
		ret = adc_tm3_reg_update(chip,
				ADC_TM_Mn_HIGH_THR_INT_EN_CTL(btm_chan_idx),
				ADC_TM_Mn_HIGH_THR_INT_EN(btm_chan_idx),
				state);
		if (ret)
			pr_err("channel:%x failed\n", btm_chan);
	break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int adc_tm4_set_trip_temp(struct adc_tm_sensor *sensor,
					int low_temp, int high_temp)
{
	struct adc_tm_chip *chip;
	struct adc_tm_config tm_config;
	u8 trip_low_thr[2], trip_high_thr[2];
	uint16_t reg_low_thr_lsb, reg_high_thr_lsb;
	uint16_t reg_low_thr_msb, reg_high_thr_msb;
	int ret;
	uint32_t btm_chan = 0, btm_chan_idx = 0, mask = 0;
	unsigned long flags;

	if (!sensor)
		return -EINVAL;

	chip = sensor->chip;

	if (adc_tm3_check_revision(chip, ADC_TM_Mn_ADC_CH_SEL_CTL(1),
				ADC_TM_Mn_ADC_CH_SEL_CTL(4), sensor->btm_ch))
		return -EINVAL;

	pr_debug("%s:low_temp(mdegC):%d, high_temp(mdegC):%d\n", __func__,
							low_temp, high_temp);

	tm_config.channel = sensor->adc_ch;
	tm_config.high_thr_temp = tm_config.low_thr_temp = 0;
	if (high_temp != INT_MAX)
		tm_config.high_thr_temp = high_temp;
	if (low_temp != INT_MIN)
		tm_config.low_thr_temp = low_temp;

	if ((high_temp == INT_MAX) && (low_temp == INT_MIN)) {
		pr_err("No trips to set\n");
		return -EINVAL;
	}

	pr_debug("requested a low temp- %d and high temp- %d\n",
			tm_config.low_thr_temp, tm_config.high_thr_temp);

#ifdef CONFIG_ARCH_SONY_YOSHINO
	if (adc_tm->btm_ch == ADC_TM_M1_ADC_CH_SEL_CTL ||
	    adc_tm->btm_ch == ADC_TM_M2_ADC_CH_SEL_CTL) {
		/* Coming in DeciDegC, so let's scale it up */
		/* TODO: Is that correct? This is to avoid creating one more
		 *       table and one more function, which seems useless */
		tm_config.high_thr_temp *= 100;
		tm_config.low_thr_temp *= 100;
	}
#endif
	adc_tm_scale_therm_voltage_100k(&tm_config, chip->data);

	/* Cool temperature corresponds to high voltage threshold */
	mask = lower_32_bits(tm_config.high_thr_voltage);
	trip_high_thr[0] = ADC_TM_LOWER_MASK(mask);
	trip_high_thr[1] = ADC_TM_UPPER_MASK(mask);
	/* Warm temperature corresponds to low voltage threshold */
	mask = lower_32_bits(tm_config.low_thr_voltage);
	trip_low_thr[0] = ADC_TM_LOWER_MASK(mask);
	trip_low_thr[1] = ADC_TM_UPPER_MASK(mask);

	pr_debug("high_thr:0x%llx, low_thr:0x%llx\n",
		tm_config.high_thr_voltage, tm_config.low_thr_voltage);

	btm_chan = sensor->btm_ch;
	ret = adc_tm3_get_btm_idx(chip, adc_tm_ch_data,
				  btm_chan, &btm_chan_idx);
	if (ret < 0) {
		pr_err("Invalid btm channel idx\n");
		return ret;
	}

	spin_lock_irqsave(&chip->adc_tm_lock, flags);

	reg_low_thr_lsb = ADC_TM_Mn_LOW_THR0(btm_chan_idx);
	reg_high_thr_lsb = ADC_TM_Mn_HIGH_THR0(btm_chan_idx);
	reg_low_thr_msb = ADC_TM_Mn_LOW_THR1(btm_chan_idx);
	reg_high_thr_msb = ADC_TM_Mn_HIGH_THR1(btm_chan_idx);

	if (high_temp != INT_MAX) {
		ret = adc_tm3_write_reg(chip, reg_low_thr_lsb,
						&trip_low_thr[0], 1);
		if (ret) {
			pr_err("Warm LSB set threshold err\n");
			goto fail;
		}

		ret = adc_tm3_write_reg(chip, reg_low_thr_msb,
						&trip_low_thr[1], 1);
		if (ret) {
			pr_err("Warm MSB set threshold err\n");
			goto fail;
		}

		ret = adc_tm4_activate_trip_type(sensor,
				THERMAL_TRIP_CONFIGURABLE_HI,
				THERMAL_DEVICE_ENABLED);
		if (ret) {
			pr_err("adc-tm warm activation failed\n");
			goto fail;
		}
	} else {
		ret = adc_tm4_activate_trip_type(sensor,
				THERMAL_TRIP_CONFIGURABLE_HI,
				THERMAL_DEVICE_DISABLED);
		if (ret) {
			pr_err("adc-tm warm deactivation failed\n");
			goto fail;
		}
	}

	if (low_temp != INT_MIN) {
		ret = adc_tm3_write_reg(chip, reg_high_thr_lsb,
						&trip_high_thr[0], 1);
		if (ret) {
			pr_err("adc-tm cool temp set threshold err\n");
			goto fail;
		}

		ret = adc_tm3_write_reg(chip, reg_high_thr_msb,
						&trip_high_thr[1], 2);
		if (ret) {
			pr_err("adc-tm cool temp set threshold err\n");
			goto fail;
		}

		ret = adc_tm4_activate_trip_type(sensor,
				THERMAL_TRIP_CONFIGURABLE_LOW,
				THERMAL_DEVICE_ENABLED);
		if (ret) {
			pr_err("adc-tm cool activation failed\n");
			goto fail;
		}
	} else {
		ret = adc_tm4_activate_trip_type(sensor,
				THERMAL_TRIP_CONFIGURABLE_LOW,
				THERMAL_DEVICE_DISABLED);
		if (ret) {
			pr_err("adc-tm cool deactivation failed\n");
			goto fail;
		}
	}

	if ((high_temp != INT_MAX) || (low_temp != INT_MIN)) {
		ret = adc_tm4_set_mode(sensor, THERMAL_DEVICE_ENABLED);
		if (ret)
			pr_err("sensor enabled failed\n");
	} else {
		ret = adc_tm4_set_mode(sensor, THERMAL_DEVICE_DISABLED);
		if (ret)
			pr_err("sensor disable failed\n");
	}

fail:
	spin_unlock_irqrestore(&chip->adc_tm_lock, flags);

	return ret;
}

static irqreturn_t adc_tm4_handler(int irq, void *data)
{
	struct adc_tm_chip *chip = data;
	u8 status_low, status_high, ctl;
	int ret = 0, i = 0;
	unsigned long flags;

	ret = adc_tm3_read_reg(chip, ADC_TM_STATUS_LOW, &status_low, 1);
	if (ret < 0) {
		pr_err("adc-tm-tm read status low failed with %d\n", ret);
		return IRQ_HANDLED;
	}

	ret = adc_tm3_read_reg(chip, ADC_TM_STATUS_HIGH, &status_high, 1);
	if (ret < 0) {
		pr_err("adc-tm-tm read status high failed with %d\n", ret);
		return IRQ_HANDLED;
	}

	while (i < chip->dt_channels) {
		bool upper_set = false, lower_set = false;
		u8 data_low = 0, data_high = 0;
		u16 code = 0;
		int temp;

		if (!chip->sensor[i].non_thermal &&
				IS_ERR(chip->sensor[i].tzd)) {
			pr_err("thermal device not found\n");
			i++;
			continue;
		}

		if (!chip->sensor[i].non_thermal) {
			ret = adc_tm3_get_temp(&chip->sensor[i], &temp);
			if (ret < 0) {
				i++;
				continue;
			}
			ret = adc_tm3_read_reg(chip, ADC_TM_Mn_DATA0(i),
						&data_low, 1);
			if (ret)
				pr_err("adc_tm data_low read failed with %d\n",
							ret);
			ret = adc_tm3_read_reg(chip, ADC_TM_Mn_DATA1(i),
						&data_high, 1);
			if (ret)
				pr_err("adc_tm data_high read failed with %d\n",
							ret);
			code = ((data_high << ADC_TM_DATA_SHIFT) | data_low);
		}

		spin_lock_irqsave(&chip->adc_tm_lock, flags);

		ret = adc_tm3_read_reg(chip, ADC_TM_Mn_MEAS_EN_CTL(i), &ctl, 1);
		if (ret) {
			pr_err("ctl read failed with %d\n", ret);
			goto fail;
		}

		if ((status_low & 0x1) && (ctl & ADC_TM_Mn_MEAS_EN(i))
				&& (ctl & ADC_TM_Mn_LOW_THR_INT_EN(i)))
			lower_set = true;

		if ((status_high & 0x1) && (ctl & ADC_TM_Mn_MEAS_EN(i)) &&
					(ctl & ADC_TM_Mn_HIGH_THR_INT_EN(i)))
			upper_set = true;
fail:
		status_low >>= 1;
		status_high >>= 1;
		spin_unlock_irqrestore(&chip->adc_tm_lock, flags);
		if (!(upper_set || lower_set)) {
			i++;
			continue;
		}

		if (!chip->sensor[i].non_thermal) {
			/*
			 * Expected behavior is while notifying
			 * of_thermal, thermal core will call set_trips
			 * with new thresholds and activate/disable
			 * the appropriate trips.
			 */
			pr_debug("notifying of_thermal\n");
			temp = therm_fwd_scale((int64_t)code,
						ADC_HC_VDD_REF, chip->data);
			of_thermal_handle_trip_temp(chip->sensor[i].tzd,
						temp);
		} else {
			if (lower_set) {
				ret = adc_tm3_reg_update(chip,
					ADC_TM_Mn_LOW_THR_INT_EN_CTL(i),
					ADC_TM_Mn_LOW_THR_INT_EN(i),
					false);
				if (ret < 0) {
					pr_err("low thr disable failed\n");
					return IRQ_HANDLED;
				}

				chip->sensor[i].low_thr_triggered
				= true;

				queue_work(chip->sensor[i].req_wq,
						&chip->sensor[i].work);
			}

			if (upper_set) {
				ret = adc_tm3_reg_update(chip,
					ADC_TM_Mn_HIGH_THR_INT_EN_CTL(i),
					ADC_TM_Mn_HIGH_THR_INT_EN(i),
					false);
				if (ret < 0) {
					pr_err("high thr disable failed\n");
					return IRQ_HANDLED;
				}

				chip->sensor[i].high_thr_triggered = true;

				queue_work(chip->sensor[i].req_wq,
						&chip->sensor[i].work);
			}
		}
		i++;
	}
	return IRQ_HANDLED;
}

static int adc3_tm_read_raw_iio(struct adc_tm_chip *chip,
			const char* channel_name, int *result)
{
	struct iio_channel *vadc_chan;
	int value = -1, rc = -EACCES;

	vadc_chan = iio_channel_get(chip->dev, channel_name);
	if (IS_ERR_OR_NULL(vadc_chan))
		return -EACCES;

	rc = iio_read_channel_raw(vadc_chan, &value);
	if (rc < 0)
		dev_err(chip->dev, "Cannot read IIO channel %s.\n",
			channel_name);
	else
		*result = value;

	iio_channel_release(vadc_chan);
	return rc;
}

static int adc_tm4_get_calibration_params(struct adc_tm_chip *chip)
{
	int vref_calib = 0, gnd_calib = 0;
	struct adc_tm_linear_graph *graph;
	int rc = -EINVAL;

	/* READ VADC_REF_1250MV for NON-HC, or
	 * ADC_1P25VREF for HC (which is a VADC5 definition!) */

	/* ABSOLUTE (HC: ADC_1P25VREF ) */
	graph = &cal_graph[ADC_ABS_CAL];
	rc = adc3_tm_read_raw_iio(chip, "ref_1p25", &vref_calib);
	if (rc < 0)
		return rc;

	/* HC: always ADC_REF_GND (both ABS and RATIO) */
	rc = adc3_tm_read_raw_iio(chip, "ref_gnd", &gnd_calib);
	if (rc < 0)
		return rc;

	graph->dy = (vref_calib - gnd_calib);
	graph->dx = ADC_TM3_625_UV;
	graph->gnd = gnd_calib;
	graph->vref = vref_calib;

	/* RATIOMETRIC (HC: ADC_VREF_VADC) */
	graph = &cal_graph[ADC_RATIO_CAL];
	rc = adc3_tm_read_raw_iio(chip, "vref_vadc", &vref_calib);
	if (rc < 0)
		return rc;

	/* HC: always ADC_REF_GND (both ABS and RATIO) */
	rc = adc3_tm_read_raw_iio(chip, "ref_gnd", &gnd_calib);
	if (rc < 0) {
		rc = adc3_tm_read_raw_iio(chip, "ch_spare1", &gnd_calib);
		if (rc < 0)
			return rc;
	}

	graph->dy = (vref_calib - gnd_calib);
	graph->dx = ADC_TM3_RATIOMETRIC_RANGE;
	graph->gnd = gnd_calib;
	graph->vref = vref_calib;

	/* For these SoCs the ratiometric range is different */
	if (of_machine_is_compatible("qcom,msm8998") ||
	    of_machine_is_compatible("qcom,sdm630") ||
	    of_machine_is_compatible("qcom,sdm636") ||
	    of_machine_is_compatible("qcom,sdm660") ||
	    of_machine_is_compatible("qcom,sdm845"))
		graph->dx = ADC_TM3_RATIOMETRIC_RANGE_8998;

	return rc;
}

static int adc_tm4_register_intr(struct adc_tm_chip *chip, const char* name)
{
	struct platform_device *pdev;
	int ret, irq;
	pdev = to_platform_device(chip->dev);

	irq = platform_get_irq_byname(pdev, name);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get irq %s\n", name);
		return irq;
	}

	ret = devm_request_threaded_irq(&pdev->dev, irq, NULL,
			adc_tm4_handler,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			name, chip);
	if (ret) {
		dev_err(&pdev->dev, "failed to get irq %s\n", name);
		return ret;
	}

	enable_irq_wake(irq);

	return ret;
}

static int adc_tm4_register_interrupts(struct adc_tm_chip *chip)
{
	int ret;

	ret = adc_tm4_register_intr(chip, "eoc-int-en-set");

	return ret;
}

static int adc_tm4_init(struct adc_tm_chip *chip, uint32_t dt_chans)
{
	u8 buf, channels_available, meas_int_timer_2_3 = 0;
	int ret, sz;
	unsigned int i;

	/* If this fails, then the number of channels for TM4 and HC is 8 */
	ret = adc_tm3_read_reg(chip, ADC_TM_NUM_BTM, &channels_available, 1);
	if (ret < 0) {
		pr_err("read failed for BTM channels\n");
		channels_available = 8;
	}
	pr_info("ADC TM: %u channels available\n", channels_available);

	if (dt_chans > channels_available) {
		pr_err("Number of nodes greater than channels supported:%d\n",
							channels_available);
		return -EINVAL;
	}

	/* Select decimation */
	buf = chip->prop.decimation;
	ret = adc_tm3_write_reg(chip,
			ADC_TM_ADC_DIG_PARAM, &buf, 1);


	/* Read current fast average parameters */
	ret = adc_tm3_read_reg(chip, ADC_TM_FAST_AVG_CTL, &buf, 1);
	if (ret < 0) {
		pr_err("adc-tm fast average read failed with %d\n", ret);
		return ret;
	}

	/* Setup fast average delay and enable */
	buf |= chip->prop.fast_avg_samples;
	ret = adc_tm3_write_reg(chip, ADC_TM_FAST_AVG_CTL, &buf, 1);
	if (ret < 0) {
		pr_err("adc-tm fast average enable failed with %d\n", ret);
		return ret;
	}

	/* Select timer1 */
	buf = chip->prop.timer1;
	ret = adc_tm3_write_reg(chip,
			ADC_TM_MEAS_INTERVAL_CTL, &buf, 1);

	/* Select timer2 and timer3 */
	meas_int_timer_2_3 |= chip->prop.timer2 <<
				ADC_TM_MEAS_INTERVAL_CTL2_SHIFT;
	meas_int_timer_2_3 |= chip->prop.timer3;

	ret = adc_tm3_write_reg(chip,
			ADC_TM_MEAS_INTERVAL_CTL2, &meas_int_timer_2_3, 1);
	if (ret < 0)
		pr_err("adc-tm block write failed with %d\n", ret);

	spin_lock_init(&chip->adc_tm_lock);
	mutex_init(&chip->adc_mutex_lock);

	for (i = 0; i < dt_chans; i++)
		chip->sensor[i].btm_ch = adc_tm_ch_data[i].btm_amux_ch;

	/* And now let's wire up the Rscale functions */
	sz = sizeof(struct adc_tm_reverse_scale_fn) * SCALE_RSCALE_NONE;
	adc_tm_rscale_fn = devm_kzalloc(chip->dev, sz, GFP_KERNEL);
	if (!adc_tm_rscale_fn)
		return -ENOMEM;

	adc_tm_rscale_fn[SCALE_R_ABSOLUTE].chan = adc_tm4_absolute_rthr;

	ret = adc_tm4_get_calibration_params(chip);
	if (ret)
		dev_err(chip->dev, "Cannot get calibration parameters!!\n");

	return ret;
}

static const struct adc_tm_ops ops_adc_tm4 = {
	.init		= adc_tm4_init,
	.set_trips	= adc_tm4_set_trip_temp,
	.interrupts_reg = adc_tm4_register_interrupts,
	.get_temp	= adc_tm3_get_temp,
	.notify_adc	= notify_adc_tm4_fn,
};

const struct adc_tm_data data_adc_tm4 = {
	.ops			= &ops_adc_tm4,
	.full_scale_code_volt	= 0x4000,
	.decimation = (unsigned int []) {250, 420, 840},
	.hw_settle = (unsigned int []) {15, 100, 200, 300, 400, 500, 600, 700,
					1, 2, 4, 8, 16, 32, 64, 128},
};
