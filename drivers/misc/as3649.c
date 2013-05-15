/*
 * as3649.c - Led flash driver
 *
 * Version:
 * 2012-09-04: v0.11 : changed rounding, prefere ROUND_UP
 * 2012-07-23: v0.10 : incorporate many review comments
 * 2012-07-10: v0.9 : added automatic shutdown, some review comments
 * 2012-06-11: v0.8 : incorporated reviews, better diag pulse handling
 * 2012-05-04: v0.7 : first version which is adapted to AS3649
 * code originating from AS3648 driver
 *
 * Copyright (C) 2012 Ulrich Herrmann <ulrich.herrmann@ams.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/as3649.h>

#define AS3649_CURR_STEP          3922 /* uA */
#define AS3649_CURR_STEP_BOOST    4902 /* uA */
#define AS3649_TXMASK_STEP       31400 /* uA */
#define AS3649_TXMASK_STEP_BOOST 39200 /* uA */
#define AS3649_NTC_CURR_STEP        40 /* uA */
#define AS3649_NUM_REGS             16
#define AS3649_MAX_PEAK_CURRENT   1250
#define AS3649_MIN_ILIMIT         2500 /* mA */

#define AS3649_MAX_TORCH_CURRENT     \
	DIV_ROUND_UP(AS3649_CURR_STEP * 0x3f, 1000)
#define AS3649_MAX_INDICATOR_CURRENT \
	DIV_ROUND_UP(AS3649_CURR_STEP * 0x3f, 1000)
#define AS3649_MAX_NTC_CURRENT       \
	(0xf * AS3649_NTC_CURR_STEP)

#define AS3649_REG_ChipID            0
#define AS3649_REG_Current_Set_LED1  1
#define AS3649_REG_Current_Set_LED2  2
#define AS3649_REG_TXMask            3
#define AS3649_REG_Low_Voltage       4
#define AS3649_REG_Flash_Timer       5
#define AS3649_REG_Control           6
#define AS3649_REG_Strobe_Signalling 7
#define AS3649_REG_Fault             8
#define AS3649_REG_PWM_and_Indicator 9
#define AS3649_REG_ADC_Result      0xa
#define AS3649_REG_ADC_Result_lsbs 0xb
#define AS3649_REG_min_LED_Current 0xe
#define AS3649_REG_act_LED_Current 0xf
#define AS3649_REG_Password       0x80
#define AS3649_REG_Current_Boost  0x81

/* Register bit defines */
#define AS3649_REG_ChipID_fixed_id_mask                0xf8
#define AS3649_REG_ChipID_fixed_id                     0xc0
#define AS3649_REG_ChipID_version_mask                 0x07
#define AS3649_REG_Control_mode_mask                   0x3
#define AS3649_REG_Control_mode_etorch                 0x0
#define AS3649_REG_Control_mode_ind                    0x1
#define AS3649_REG_Control_mode_itorch                 0x2
#define AS3649_REG_Control_mode_flash                  0x3
#define AS3649_REG_TXMask_mode_none                    0x0
#define AS3649_REG_TXMask_mode_txmask                  0x1
#define AS3649_REG_TXMask_mode_ext_torch               0x1
#define AS3649_REG_Flash_Timer_small_step              4
#define AS3649_REG_Flash_Timer_large_step              32
#define AS3649_REG_Flash_Timer_step_border             32
#define AS3649_REG_Flash_Timer_step_border_ms  \
	(AS3649_REG_Flash_Timer_step_border * \
	 AS3649_REG_Flash_Timer_small_step \
	 + AS3649_REG_Flash_Timer_small_step)
#define AS3649_REG_Strobe_Signalling_adc_convert       0x10
#define AS3649_REG_Strobe_Signalling_channel_tjunc     0x1
#define AS3649_REG_Strobe_Signalling_channel_ntc       0x0
#define AS3649_REG_Strobe_Signalling_strobe_on         0x80
#define AS3649_REG_Strobe_Signalling_strobe_type_level 0x80
#define AS3649_REG_Strobe_Signalling_dcdc_skip_enable  0x20
#define AS3649_REG_Low_Voltage_ntc_current_shift       4
#define AS3649_REG_Low_Voltage_ntc_on                  0x8
#define AS3649_REG_PWM_and_Indicator_freq_switch_on    0x8
#define AS3649_REG_PWM_and_Indicator_load_balance_on   0x10
#define AS3649_REG_PWM_and_Indicator_const_v_mode      0x20
#define AS3649_REG_Fault_status_uvlo                   0x01
#define AS3649_REG_Fault_fault_ntc                     0x04
#define AS3649_REG_Fault_fault_txmask                  0x08
#define AS3649_REG_Fault_fault_timeout                 0x10
#define AS3649_REG_Fault_fault_overtemp                0x20
#define AS3649_REG_Fault_fault_led_short               0x40
#define AS3649_REG_Fault_fault_ovp                     0x80
#define AS3649_REG_Password_pw                         0xa1


#define AS3649_WRITE_REG(a, b) i2c_smbus_write_byte_data(data->client, a, b)
#define AS3649_READ_REG(a) i2c_smbus_read_byte_data(data->client, a)

#define AS3649_LOCK()   mutex_lock(&(data)->update_lock)
#define AS3649_UNLOCK() mutex_unlock(&(data)->update_lock)

#define AS3649_ATTR(_name)  \
	__ATTR(_name, 0660, as3649_##_name##_show, as3649_##_name##_store)

#define AS3649_RO_ATTR(_name)  \
	__ATTR(_name, 0440, as3649_##_name##_show, NULL)

#define AS3649_WO_ATTR(_name)  \
	__ATTR(_name, 0220, NULL, as3649_##_name##_store)

struct as3649_reg {
	const char *name;
	u8 id;
	u8 value;
};

struct as3649_data {
	void (*enable)(struct device *dev, bool on);
	struct as3649_platform_data *pdata;
	struct i2c_client *client;
	struct mutex update_lock;
	bool flash_mode;
	u8 flash_curr;
	u8 flash_boost;
	u8 flash_time;
	u8 normal_curr;
	u8 normal_ctrl;
	u8 diag_ctrl;
	u8 normal_lv;
	u8 diag_lv;
	u8 strobe_reg;
	u8 pwm_reg;
	u8 pwm_mode;
	u8 led_mask;
	u8 diag_pulse_time;
	u8 fault; /* shadow of fault reg for later on read out */
	u8 vfi; /* calculated from vf_mV, index into as3649_tmax */
	s16 flash_tamb_celsius;
};

struct as3649_tflash_max {
	u16 tmax_65degC[3][4];
	u16 tmax_75degC[3][4];
	u16 tmax_75degC_boost[3][5];
	u16 tmax_40degC_boost[3][5];
};

/* Values taken from application note to calculate max flash time
   from T_amb, v_f and I_flash */
static const struct as3649_tflash_max as3649_tmax = {
	.tmax_75degC = { /* Starts at 1.7A ~ 0.85A */
		{1124, 1124, 1124, 1124}, /* vf 3.75 V*/
		{1124, 1124,  740,  352}, /* vf 4.10 V*/
		{1124, 1104,  352,  256}, /* vf 4.40 V*/
	},
	.tmax_65degC = { /* Starts at 1.7A ~ 0.85A */
		{1124, 1124, 1124, 1124 }, /* vf 3.75 V*/
		{1124, 1124, 1124, 1124 }, /* vf 4.10 V*/
		{1124, 1124, 1124,  476 }, /* vf 4.40 V*/
	},
	.tmax_75degC_boost = { /* Starts at 2.1A ~ 1.05A */
		{264, 216, 180, 156, 136 }, /* vf 3.75 V*/
		{204, 172, 148, 128, 112 }, /* vf 4.10 V*/
		{168, 144, 124, 108,  96 }, /* vf 4.40 V*/
	},
	.tmax_40degC_boost = { /* Starts at 2.1A ~ 1.05A */
		{516, 332, 260, 212, 180 }, /* vf 3.75 V*/
		{304, 240, 196, 164, 144 }, /* vf 4.10 V*/
		{236, 192, 160, 140, 120 }, /* vf 4.40 V*/
	}
};

static void as3649_set_leds(struct as3649_data *data, u8 ledMask,
		u8 ctrl, u8 curr);

#define AS3649_REG(NAME, VAL) \
	{.name = __stringify(NAME), .id = AS3649_REG_##NAME, .value = (VAL)}

static const struct as3649_data as3649_default_data = {
	.client = NULL,
};

static const struct as3649_reg as3649_regs[AS3649_NUM_REGS] = {
	AS3649_REG(ChipID,            0xB1),
	AS3649_REG(Current_Set_LED1,  0x9C),
	AS3649_REG(Current_Set_LED2,  0x9C),
	AS3649_REG(TXMask,            0x68),
	AS3649_REG(Low_Voltage,       0x2C),
	AS3649_REG(Flash_Timer,       0x23),
	AS3649_REG(Control,           0x00),
	AS3649_REG(Strobe_Signalling, 0xC0),
	AS3649_REG(Fault,             0x00),
	AS3649_REG(PWM_and_Indicator, 0x00),
	AS3649_REG(ADC_Result,        0x00),
	AS3649_REG(ADC_Result_lsbs,   0x00),
	AS3649_REG(min_LED_Current,   0x00),
	AS3649_REG(act_LED_Current,   0x00),
	AS3649_REG(Password,          0x00),
	AS3649_REG(Current_Boost,     0x00),
};

static int device_add_attributes(struct device *dev,
				 struct device_attribute *attrs)
{
	int error = 0;
	int i;

	if (attrs) {
		for (i = 0; attr_name(attrs[i]); i++) {
			error = device_create_file(dev, &attrs[i]);
			if (error) {
				dev_err(dev, "Failed creating %s",
						attrs[i].attr.name);
				break;
			}
		}
		if (error)
			while (--i >= 0)
				device_remove_file(dev, &attrs[i]);
	}
	return error;
}

static void device_remove_attributes(struct device *dev,
				     struct device_attribute *attrs)
{
	int i;

	if (attrs)
		for (i = 0; attr_name(attrs[i]); i++)
			device_remove_file(dev, &attrs[i]);
}

static void as3649_dummy_enable(struct device *dev, bool on)
{
}

static void as3649_set_leds(struct as3649_data *data,
			u8 ledMask, u8 ctrl, u8 curr)
{
	if (ledMask & 1)
		AS3649_WRITE_REG(AS3649_REG_Current_Set_LED1, curr);
	else
		AS3649_WRITE_REG(AS3649_REG_Current_Set_LED1, 0);
	if (ledMask & 2)
		AS3649_WRITE_REG(AS3649_REG_Current_Set_LED2, curr);
	else
		AS3649_WRITE_REG(AS3649_REG_Current_Set_LED2, 0);

	if (ledMask == 0 || curr == 0)
		AS3649_WRITE_REG(AS3649_REG_Control,
				AS3649_REG_Control_mode_etorch);
	else
		AS3649_WRITE_REG(AS3649_REG_Control, ctrl);
}

static void as3649_set_txmask(struct as3649_data *data)
{
	u8 tm;
	u32 limit = 0, txmask;

	tm = ((data->pdata->use_tx_mask) ? AS3649_REG_TXMask_mode_txmask
			: AS3649_REG_TXMask_mode_none);

	if (data->pdata->I_limit_mA > AS3649_MIN_ILIMIT)
		limit = (data->pdata->I_limit_mA - AS3649_MIN_ILIMIT) / 400;

	limit = clamp_t(u32, limit, 0, 3);
	tm |= limit<<2;

	txmask = data->pdata->txmasked_current_mA * 1000;

	if (data->flash_boost)
		txmask /= AS3649_TXMASK_STEP_BOOST;
	else
		txmask /= AS3649_TXMASK_STEP;

	txmask = clamp_t(u32, txmask, 0, 0xf);

	tm |= txmask<<4;

	AS3649_WRITE_REG(AS3649_REG_TXMask, tm);
}


static ssize_t as3649_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE, cw = 0;
	u8 cr;
	int i = 0;
	const struct as3649_reg *reg;
	ssize_t err;

	AS3649_LOCK();
	err = pm_runtime_get_sync(&data->client->dev);
	if (err < 0)
		goto exit;
	while (cw < ps && i < ARRAY_SIZE(as3649_regs)) {
		ps -= cw;
		buf += cw;
		cw = 0;
		reg = as3649_regs + i;
		if (reg->name) {
			if (reg->id == AS3649_REG_Current_Boost) {
				i2c_smbus_write_byte_data(data->client,
						AS3649_REG_Password,
						AS3649_REG_Password_pw);
			}
			cr = i2c_smbus_read_byte_data(data->client, reg->id);
			cw = scnprintf(buf, ps, "%34s %02x: %#04x\n",
					reg->name, reg->id, cr);
		}
		i++;
	}
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(dev);
	ps -= cw;
	buf += cw;
exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return PAGE_SIZE - ps;
}

static ssize_t as3649_debug_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	int i;
	u8 reg, val;
	ssize_t err;
	i = sscanf(buf, "0x%hhx=0x%hhx", &reg, &val);
	if (i != 2)
		return -EINVAL;

	AS3649_LOCK();
	err = pm_runtime_get_sync(&data->client->dev);
	if (err < 0)
		goto exit;
	AS3649_WRITE_REG(reg, val);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(dev);
exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_brightness_max_uA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", 1000 *
			data->pdata->max_sustained_current_mA);
}
static ssize_t as3649_brightness_uA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;
	u32 val;

	val = data->normal_curr * AS3649_CURR_STEP;
	if (val && (data->normal_ctrl & AS3649_REG_Control_mode_mask) ==
			AS3649_REG_Control_mode_ind)
		val = val * data->pwm_mode / 64;
	return scnprintf(buf, ps, "%d\n", val);
}

static ssize_t as3649_brightness_uA_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	ssize_t err = 0;
	int i;
	u32 val, curr;
	u8 ctrl = 0, pwm_reg = 0;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;

	if (val > (data->pdata->max_sustained_current_mA * 1000))
		val = data->pdata->max_sustained_current_mA * 1000;

	AS3649_LOCK();
	if (0 != val && 0 == data->normal_curr) {
		err = pm_runtime_get_sync(dev);
		if (err < 0)
			goto exit;
	}
	if (val == 0) {
		ctrl = 0;
		curr = 0;
	} else if (val > (data->pdata->min_current_mA * 1000)) {
		/* compute curr and write it */
		curr = DIV_ROUND_UP(val, AS3649_CURR_STEP);
		/* turn on torch mode */
		ctrl |= AS3649_REG_Control_mode_itorch;
	} else {
		u32 pwm_mode;
		u32 pwm_64 = (val * 64) / (data->pdata->min_current_mA * 1000);
		u8 inct_pwm = 0;
		/* turn on indicator mode */
		ctrl |= AS3649_REG_Control_mode_ind;
		/* compute curr and write it using pwm */
		if (pwm_64 >= 16)
			pwm_mode = 16;
		else if (pwm_64 >= 12)
			pwm_mode = 12;
		else if (pwm_64 >= 8)
			pwm_mode = 8;
		else if (pwm_64 >= 4)
			pwm_mode = 4;
		else if (pwm_64 >= 3)
			pwm_mode = 3;
		else if (pwm_64 >= 2)
			pwm_mode = 2;
		else
			pwm_mode = 1;
		data->pwm_mode = pwm_mode;
		if (pwm_mode >= 4) { /* Covers 1,2,3,4 /16 */
			inct_pwm = pwm_mode / 4 - 1;
			pwm_reg = data->pwm_reg | inct_pwm;
		} else { /* Covers the 1,2,3,4 /64 settings */
			inct_pwm = pwm_mode + 3;
			pwm_reg = data->pwm_reg | inct_pwm |
				  AS3649_REG_PWM_and_Indicator_const_v_mode;
		}

		curr = val * 64 / pwm_mode;

		if (curr < 1000*data->pdata->min_current_mA)
			curr = 1000*data->pdata->min_current_mA;

		curr = DIV_ROUND_UP(curr, AS3649_CURR_STEP);
	}
	/* Turn off leds for avoiding short flashes of bright light */
	if (!data->flash_mode)
		as3649_set_leds(data, data->led_mask, ctrl, 0);

	AS3649_WRITE_REG(AS3649_REG_PWM_and_Indicator, pwm_reg);

	if (!data->flash_mode)
		as3649_set_leds(data, data->led_mask, ctrl, curr);

	pm_runtime_mark_last_busy(&data->client->dev);
	if (0 == val && 0 != data->normal_curr)
		pm_runtime_put_autosuspend(dev);

	data->normal_curr = curr;
	data->normal_ctrl = ctrl;
exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_ntc_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t err;
	u16 result;
	struct as3649_data *data = dev_get_drvdata(dev);

	AS3649_LOCK();
	err = pm_runtime_get_sync(dev);
	if (err < 0)
		goto exit;
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling,
			data->strobe_reg |
			AS3649_REG_Strobe_Signalling_adc_convert |
			AS3649_REG_Strobe_Signalling_channel_ntc);
	usleep_range(250, 10000);
	result = AS3649_READ_REG(AS3649_REG_ADC_Result);
	result <<= 2;
	result |= AS3649_READ_REG(AS3649_REG_ADC_Result_lsbs);
	result *= 2200;
	result /= 1024;
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(dev);

	AS3649_UNLOCK();

exit:
	if (err < 0)
		return err;
	return scnprintf(buf, PAGE_SIZE, "%d mV\n", result);
}

static void as3649_get_T_junc(struct as3649_data *data, s16 *celsius, u16 *raw)
{
	u16 result;
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling,
			data->strobe_reg |
			AS3649_REG_Strobe_Signalling_adc_convert |
			AS3649_REG_Strobe_Signalling_channel_ntc);
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling,
			data->strobe_reg
			| AS3649_REG_Strobe_Signalling_adc_convert
			| AS3649_REG_Strobe_Signalling_channel_tjunc);
	usleep_range(250, 10000);
	result = AS3649_READ_REG(AS3649_REG_ADC_Result);
	result <<= 2;
	result |= AS3649_READ_REG(AS3649_REG_ADC_Result_lsbs);
	*raw = result;

	*celsius = 329 - result; /* This is only valid above 60degC */
	if (result < 268)
		*celsius -= 0;
	else if (result < 278)
		*celsius -= 1;
	else if (result < 297)
		*celsius -= 2;
	else if (result < 316)
		*celsius -= 3;
	else if (result < 325)
		*celsius += 4;
	else if (result < 334)
		*celsius -= 5;
	else if (result < 343)
		*celsius -= 6;
	else if (result < 352)
		*celsius -= 7;

}
static ssize_t as3649_T_junc_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u16 result;
	s16 celsius;
	ssize_t err = 0;

	AS3649_LOCK();
	err = pm_runtime_get_sync(dev);
	if (err < 0)
		goto exit;

	as3649_get_T_junc(data, &celsius, &result);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(dev);

exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return scnprintf(buf, PAGE_SIZE, "%d degC %d raw\n", celsius, result);
}

static ssize_t as3649_flash_diag_pulse_trigger_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	int i, val;
	ssize_t err = 0;
	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;

	AS3649_LOCK();
	if (!data->flash_mode) {
		err = -EINVAL;
		goto exit;
	}

	AS3649_WRITE_REG(AS3649_REG_Low_Voltage, data->diag_lv);
	AS3649_WRITE_REG(AS3649_REG_Flash_Timer, data->diag_pulse_time);
	AS3649_WRITE_REG(AS3649_REG_Control, data->diag_ctrl
			| AS3649_REG_Control_mode_flash);
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling,
			data->strobe_reg
			& ~AS3649_REG_Strobe_Signalling_strobe_on);

exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_flash_time_ms_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;
	u32 ft = data->flash_time;

	if (ft < AS3649_REG_Flash_Timer_step_border)
		ft = ft * AS3649_REG_Flash_Timer_small_step
			+ AS3649_REG_Flash_Timer_small_step; /* 4ms steps */
	else /* above 0x20 32ms steps */
		ft = AS3649_REG_Flash_Timer_step_border_ms
		   + (ft - AS3649_REG_Flash_Timer_step_border)
		     * AS3649_REG_Flash_Timer_large_step;


	return scnprintf(buf, ps, "%d\n", ft);
}

static ssize_t as3649_flash_time_ms_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	int i;
	u32 val;
	u32 reg;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if (val == 0)
		return -EINVAL;

	if (val <= AS3649_REG_Flash_Timer_step_border_ms)
		reg = DIV_ROUND_UP(val, AS3649_REG_Flash_Timer_small_step)
			- 1;
	else
		reg = AS3649_REG_Flash_Timer_step_border
		    + DIV_ROUND_UP(val
				       - AS3649_REG_Flash_Timer_step_border_ms
				       , AS3649_REG_Flash_Timer_large_step);

	reg = clamp_t(u32, reg, 0, 0x3f);

	AS3649_LOCK();
	data->flash_time = (u8)reg;
	AS3649_UNLOCK();

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_flash_trigger_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	int i;
	ssize_t err = 0;
	u32 val;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if (val != 1)
		return -EINVAL;

	AS3649_LOCK();
	if (!data->flash_mode) {
		err = -EINVAL;
		goto exit;
	}

	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling,
			data->strobe_reg
			& ~AS3649_REG_Strobe_Signalling_strobe_on);

exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_flash_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE;

	return scnprintf(buf, ps, "%d\n", data->flash_mode);
}

static ssize_t as3649_flash_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	int i;
	u32 val;
	ssize_t err = 0;

	i = sscanf(buf, "%d", &val);
	if (i != 1)
		return -EINVAL;
	if ((val != 0) && (val != 1))
		return -EINVAL;

	AS3649_LOCK();

	if (val == data->flash_mode)
		goto exit;

	if (val) {
		u16 dummy;
		data->flash_mode = val;
		err = pm_runtime_get_sync(dev);
		if (err < 0)
			goto exit;
		AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling, data->strobe_reg
				| AS3649_REG_Strobe_Signalling_strobe_on);
		as3649_get_T_junc(data, &data->flash_tamb_celsius, &dummy);
		as3649_set_txmask(data);
		/* reset fault reg */
		data->fault |= i2c_smbus_read_byte_data(data->client,
				AS3649_REG_Fault);
		AS3649_WRITE_REG(AS3649_REG_Flash_Timer, data->flash_time);
		as3649_set_leds(data, 3, AS3649_REG_Control_mode_flash,
				data->flash_curr);
		if (data->flash_boost) {
			AS3649_WRITE_REG(AS3649_REG_Password,
					AS3649_REG_Password_pw);
			AS3649_WRITE_REG(AS3649_REG_Current_Boost, 1);
		}
	} else {
		/* in most cases the following test will tell us if
		   flash is still in progress */
		if (AS3649_READ_REG(AS3649_REG_act_LED_Current)) {
			err = -EAGAIN;
			goto exit;
		}
		data->flash_mode = val;
		AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling, data->strobe_reg
				| AS3649_REG_Strobe_Signalling_strobe_on);
		AS3649_WRITE_REG(AS3649_REG_Low_Voltage, data->normal_lv);
		AS3649_WRITE_REG(AS3649_REG_Password, AS3649_REG_Password_pw);
		AS3649_WRITE_REG(AS3649_REG_Current_Boost, 0);
		as3649_set_leds(data, data->led_mask,
				data->normal_ctrl, data->normal_curr);
		pm_runtime_mark_last_busy(&data->client->dev);
		pm_runtime_put_autosuspend(dev);
	}
exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_flash_brightness_max_mA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
			data->pdata->max_peak_current_mA);
}

static ssize_t as3649_flash_brightness_mA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u32 val;

	if (data->flash_boost)
		val = DIV_ROUND_CLOSEST(data->flash_curr *
				AS3649_CURR_STEP_BOOST, 1000);
	else
		val = DIV_ROUND_CLOSEST(data->flash_curr *
				AS3649_CURR_STEP, 1000);

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t as3649_flash_brightness_mA_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u32 val, fb;
	int i;

	i = sscanf(buf, "%u", &val);
	if (i != 1)
		return -EINVAL;

	if (val > data->pdata->max_peak_current_mA)
		val = data->pdata->max_peak_current_mA;

	fb = DIV_ROUND_UP((val * 1000), AS3649_CURR_STEP);
	AS3649_LOCK();
	data->flash_boost = 0;
	if (fb > 255) {
		fb = DIV_ROUND_UP((val * 1000), AS3649_CURR_STEP_BOOST);
		data->flash_boost = 1;
	}
	data->flash_curr = fb;
	AS3649_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3649_led_usage_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", data->led_mask);
}

static ssize_t as3649_led_usage_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u32 val;
	int i;

	i = sscanf(buf, "%u", &val);
	if (i != 1)
		return -EINVAL;
	if (val > 3)
		return -EINVAL;

	AS3649_LOCK();
	if (val == data->led_mask)
		goto exit;

	data->led_mask = val;

	if (!data->flash_mode && data->normal_curr) {
		as3649_set_leds(data, data->led_mask,
				data->normal_ctrl, data->normal_curr);
	}
exit:
	AS3649_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static void as3649_get_last_min_current(struct as3649_data *data,
		u16 *min_current)
{
	u32 min_curr;

	min_curr = i2c_smbus_read_byte_data(data->client,
			AS3649_REG_min_LED_Current);

	if (data->flash_boost)
		min_curr *= AS3649_CURR_STEP_BOOST;
	else
		min_curr *= AS3649_CURR_STEP;

	min_curr = DIV_ROUND_CLOSEST(min_curr, 1000);

	*min_current = min_curr;
}

static ssize_t as3649_flash_last_min_current_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u16 min_curr;
	ssize_t err = 0;

	AS3649_LOCK();

	if (!data->flash_mode) {
		dev_warn(dev, "read out of flash_last_min_current while not "
				"in flash_mode\n");
		min_curr = 0;
		goto exit;
	}

	/* no need to pm_runtine_get_sync - in flash mode we have it already */

	if (AS3649_READ_REG(AS3649_REG_act_LED_Current)) {
		dev_warn(dev, "trying to read out flash_last_min_current while"
				" flash is ongoing\n");
		err = -EAGAIN;
		goto exit;
	}

	as3649_get_last_min_current(data, &min_curr);

exit:
	AS3649_UNLOCK();
	if (err < 0)
		return err;
	return scnprintf(buf, PAGE_SIZE, "%d\n", min_curr);
}

static u16 as3649_get_t_flash_max(s16 celsius, u16 mA, int vfi, u16 coil_peak)
{
	u8 idx;
	u16 t_flash_max = 1124;
	const u16 *tmax_table = NULL;

	if (coil_peak <= 2500)
		return t_flash_max;

	if (celsius >= 85)
		t_flash_max = 300;

	if (mA > 1000) { /* tables for boost mode */
		if (celsius >= 75)
			tmax_table = as3649_tmax.tmax_75degC_boost[vfi];
		else if (celsius >= 40)
			tmax_table = as3649_tmax.tmax_40degC_boost[vfi];
		idx = DIV_ROUND_UP(mA, 50);
		idx = clamp_t(s16, idx, 21, 25);
		idx -= 21; /* Table starts at 2.1V */
	} else {  /* non-boost tables */
		if (celsius >= 75)
			tmax_table = as3649_tmax.tmax_75degC[vfi];
		else if (celsius >= 65)
			tmax_table = as3649_tmax.tmax_65degC[vfi];
		idx = DIV_ROUND_UP(mA, 50);
		idx = clamp_t(s16, idx, 17, 20);
		idx -= 17; /* Table starts at 1.7V */
	}

	if (tmax_table && tmax_table[idx] < t_flash_max)
		t_flash_max = tmax_table[idx];

	return t_flash_max;
}

static ssize_t as3649_flash_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u16 mA, t_flash_max;
	ssize_t err = 0;

	AS3649_LOCK();

	if (err < 0)
		goto exit;
	if (!data->flash_mode) {
		err = scnprintf(buf, PAGE_SIZE, "0 ms (not in flash_mode)\n");
		goto exit;
	}
	/* no need to pm_runtine_get_sync - in flash mode we have it already */
	if (AS3649_READ_REG(AS3649_REG_act_LED_Current)) {
		dev_warn(dev, "trying to read out flash_t_max while flash is "
				"ongoing\n");
		err = -EAGAIN;
		goto exit;
	}
	as3649_get_last_min_current(data, &mA);

	t_flash_max = as3649_get_t_flash_max(
			data->flash_tamb_celsius, mA,
			data->vfi, data->pdata->I_limit_mA);

	err = scnprintf(buf, PAGE_SIZE, "t_flash_max=%d ms @%d degC @%d mA\n",
			t_flash_max, data->flash_tamb_celsius, mA);
exit:
	AS3649_UNLOCK();
	return err;
}

static ssize_t as3649_flash_t_max_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u16 mA, t_flash_max;
	ssize_t err = 0;

	AS3649_LOCK();
	if (!data->flash_mode) {
		err = scnprintf(buf, PAGE_SIZE, "0 ms (not in flash_mode)\n");
		goto exit;
	}
	/* no need to pm_runtine_get_sync - in flash mode we have it already */
	if (err < 0)
		goto exit;
	if (AS3649_READ_REG(AS3649_REG_act_LED_Current)) {
		dev_warn(dev, "trying to read out flash_t_max while flash is "
				"ongoing\n");
		err = -EAGAIN;
		pm_runtime_mark_last_busy(&data->client->dev);
		pm_runtime_put_autosuspend(dev);
		goto exit;
	}
	as3649_get_last_min_current(data, &mA);

	t_flash_max = as3649_get_t_flash_max(
			data->flash_tamb_celsius, mA,
			data->vfi, data->pdata->I_limit_mA);

	err = scnprintf(buf, PAGE_SIZE, "%d\n", t_flash_max);
exit:
	AS3649_UNLOCK();
	return err;
}

static ssize_t as3649_fault_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	u8 fault = 0;

	AS3649_LOCK();

	if (!pm_runtime_suspended(dev))
		fault = AS3649_READ_REG(AS3649_REG_Fault);

	fault |= data->fault;
	data->fault = 0;

	AS3649_UNLOCK();

	return scnprintf(buf, PAGE_SIZE, "0x%02x\n", fault);
}

static struct device_attribute as3649_attributes[] = {
	AS3649_ATTR(debug),
	AS3649_ATTR(brightness_uA),
	AS3649_RO_ATTR(brightness_max_uA),
	AS3649_ATTR(flash_time_ms),
	AS3649_WO_ATTR(flash_trigger),
	AS3649_ATTR(flash_brightness_mA),
	AS3649_RO_ATTR(flash_brightness_max_mA),
	AS3649_RO_ATTR(flash_last_min_current),
	AS3649_ATTR(flash_mode),
	AS3649_ATTR(led_usage),
	AS3649_RO_ATTR(ntc),
	AS3649_RO_ATTR(T_junc),
	AS3649_WO_ATTR(flash_diag_pulse_trigger),
	AS3649_RO_ATTR(flash_t_max),
	AS3649_RO_ATTR(flash_info),
	AS3649_RO_ATTR(fault),
	__ATTR_NULL
};

static const u16 v_in_low[] = {3070, 3140, 3220, 3300, 3380, 3470};
static u8 as3649_get_vin_index(u16 mV)
{
	s8 vin;
	if (mV == 0)
		return 0;
	for (vin = ARRAY_SIZE(v_in_low) - 1; vin >= 0; vin--) {
		if (mV >= v_in_low[vin])
			break;
	}
	vin += 2;

	return vin;
}

static int as3649_configure(struct i2c_client *client,
		struct as3649_data *data, struct as3649_platform_data *pdata)
{
	int err = 0;
	u8 lv, vin;

	data->pdata = pdata;

	as3649_set_txmask(data);

	/*************** Setting AS3649_REG_Low_Voltage *********************/
	if (pdata->ntc_current_uA > AS3649_MAX_NTC_CURRENT) {
		dev_warn(&client->dev,
				"ntc_current_mA of %d higher than possible,"
				" reducing to %d",
				data->pdata->ntc_current_uA,
				AS3649_MAX_NTC_CURRENT);
		lv = (AS3649_MAX_NTC_CURRENT / AS3649_NTC_CURR_STEP)
			<< AS3649_REG_Low_Voltage_ntc_current_shift;
	} else {
		lv = DIV_ROUND_UP(data->pdata->ntc_current_uA
				  , AS3649_NTC_CURR_STEP)
				  << AS3649_REG_Low_Voltage_ntc_current_shift;
	}
	lv |= (data->pdata->ntc_on) ? AS3649_REG_Low_Voltage_ntc_on : 0;

	vin = as3649_get_vin_index(pdata->vin_low_v_mV);
	data->normal_lv = lv | vin << 0;
	vin = as3649_get_vin_index(pdata->diag_vin_low_v_mV);
	data->diag_lv = lv | vin << 0;

	AS3649_WRITE_REG(AS3649_REG_Low_Voltage, data->normal_lv);

	/*************** Setting AS3649_REG_PWM_and_Indicator ***************/
	data->pwm_reg  = ((pdata->freq_switch_on)
			? AS3649_REG_PWM_and_Indicator_freq_switch_on : 0);
	data->pwm_reg |= ((pdata->load_balance_on)
			? AS3649_REG_PWM_and_Indicator_load_balance_on : 0);

	AS3649_WRITE_REG(AS3649_REG_PWM_and_Indicator, data->pwm_reg);

	/*************** Setting AS3649_REG_Strobe_Signalling ***************/
	data->strobe_reg = pdata->strobe_type
		? (AS3649_REG_Strobe_Signalling_strobe_type_level) : 0;
	data->strobe_reg |= AS3649_REG_Strobe_Signalling_strobe_on;
	data->strobe_reg |= ((pdata->dcdc_skip_enable)
			? AS3649_REG_Strobe_Signalling_dcdc_skip_enable : 0);
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling, data->strobe_reg);

	if (data->pdata->max_peak_current_mA > AS3649_MAX_PEAK_CURRENT) {
		dev_warn(&client->dev,
				"max_peak_current_mA of %d higher than"
				" possible, reducing to %d\n",
				data->pdata->max_peak_current_mA,
				AS3649_MAX_PEAK_CURRENT);
		data->pdata->max_peak_current_mA = AS3649_MAX_PEAK_CURRENT;
	}
	if (data->pdata->max_sustained_current_mA > AS3649_MAX_TORCH_CURRENT) {
		dev_warn(&client->dev,
				"max_sustained_current_mA of %d higher than"
				" possible, reducing to %d\n",
				data->pdata->max_sustained_current_mA,
				AS3649_MAX_TORCH_CURRENT);
		data->pdata->max_sustained_current_mA =
			AS3649_MAX_TORCH_CURRENT;
	}
	if ((1000*data->pdata->min_current_mA) < AS3649_CURR_STEP) {
		data->pdata->min_current_mA =
			DIV_ROUND_UP(AS3649_CURR_STEP, 1000);
		dev_warn(&client->dev,
				"min_current_mA lower than possible, icreasing"
				" to %d\n",
				data->pdata->min_current_mA);
	}
	if (data->pdata->min_current_mA > AS3649_MAX_INDICATOR_CURRENT) {
		dev_warn(&client->dev,
				"min_current_mA of %d higher than possible,"
				" reducing to %d",
				data->pdata->min_current_mA,
				AS3649_MAX_INDICATOR_CURRENT);
		data->pdata->min_current_mA =
			AS3649_MAX_INDICATOR_CURRENT;
	}

	data->diag_ctrl =
		  (pdata->diag_pulse_vcompl_adj << 5)
		| (pdata->diag_pulse_force_dcdc_on << 4)
		| (pdata->diag_pulse_min_on_increase << 3);

	data->diag_pulse_time = pdata->diag_pulse_time;
	if (data->diag_pulse_time > 63)
		data->diag_pulse_time = 63;

	/* Setting vf index for this LED */
	data->vfi = 0; /* 3750 mV */
	if (pdata->vf_mV  >= 4100)
		data->vfi = 1;
	if (pdata->vf_mV  >= 4400)
		data->vfi = 2;

	data->led_mask = 3;
	data->normal_ctrl = 0;
	data->normal_curr = 0;

	as3649_set_leds(data, data->led_mask,
			data->normal_ctrl, data->normal_curr);

	err = device_add_attributes(&client->dev, as3649_attributes);

	if (err < 0)
		goto exit;

	return 0;

exit:
	device_remove_attributes(&client->dev, as3649_attributes);
	return err;
}

static int as3649_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct as3649_data *data;
	struct as3649_platform_data *as3649_pdata = client->dev.platform_data;
	int id1, i;
	int err = 0;

	if (!as3649_pdata)
		return -EIO;

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct as3649_data), GFP_USER);
	if (!data)
		return -ENOMEM;

	/* initialize with meaningful data (register names, etc.) */
	*data = as3649_default_data;

	dev_set_drvdata(&client->dev, data);

	data->client = client;
	mutex_init(&data->update_lock);

	data->enable = as3649_dummy_enable;
	if (as3649_pdata->enable)
		data->enable = as3649_pdata->enable;
	if (as3649_pdata->init)
		as3649_pdata->init(&client->dev, true);
	data->enable(&client->dev, true);

	id1 = i2c_smbus_read_byte_data(client, AS3649_REG_ChipID);
	if (id1 < 0) {
		err = id1;
		data->enable(&client->dev, false);
		goto exit;
	}
	if ((id1 & AS3649_REG_ChipID_fixed_id_mask)
			!= AS3649_REG_ChipID_fixed_id) {
		err = -ENXIO;
		dev_err(&client->dev, "wrong chip detected, ids %x", id1);
		data->enable(&client->dev, false);
		goto exit;
	}
	dev_info(&client->dev, "AS3649 driver v0.11: detected AS3649 "
			"compatible chip with id %x\n", id1);

	/* AS3649 has no reset via I2C, only ON-pin which is maybe not
	   available to uC */
	for (i = 0; i < ARRAY_SIZE(as3649_regs); i++) {
		if (as3649_regs[i].name)
			i2c_smbus_write_byte_data(client,
					i, as3649_regs[i].value);
	}

	i2c_set_clientdata(client, data);

	err = as3649_configure(client, data, as3649_pdata);

	data->enable(&client->dev, false);

	if (err < 0)
		goto exit;

	pm_runtime_enable(&client->dev);
	pm_suspend_ignore_children(&client->dev, true);
	err = pm_runtime_get_sync(&data->client->dev);
	if (err < 0)
		goto exit;
	pm_runtime_set_autosuspend_delay(&client->dev,
			as3649_pdata->autosuspend_delay_ms);
	pm_runtime_use_autosuspend(&client->dev);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);

exit:
	if (err < 0) {
		if (as3649_pdata->init)
			as3649_pdata->init(&client->dev, false);
		dev_err(&client->dev, "could not configure %d", err);
		kfree(data);
		i2c_set_clientdata(client, NULL);
	}

	return err;
}

#ifdef CONFIG_PM
static int as3649_suspend(struct device *dev)
{
	struct as3649_data *data = dev_get_drvdata(dev);
	data->fault |= AS3649_READ_REG(AS3649_REG_Fault);
	data->enable(dev, false);

	return 0;
}

static int as3649_resume(struct device *dev)
{
	struct as3649_data *data = dev_get_drvdata(dev);

	data->enable(dev, true);
	as3649_set_txmask(data);
	AS3649_WRITE_REG(AS3649_REG_Low_Voltage, data->normal_lv);
	AS3649_WRITE_REG(AS3649_REG_PWM_and_Indicator, data->pwm_reg);
	AS3649_WRITE_REG(AS3649_REG_Strobe_Signalling, data->strobe_reg);

	return 0;
}
#endif

static int as3649_remove(struct i2c_client *client)
{
	struct as3649_data *data = i2c_get_clientdata(client);

	dev_info(&client->dev, "Removing AS3649 device\n");

	AS3649_LOCK();

	device_remove_attributes(&client->dev, as3649_attributes);
	pm_runtime_get_sync(&data->client->dev);
	as3649_set_leds(data, 3, AS3649_REG_Control_mode_etorch, 0);
	pm_runtime_put_sync(&data->client->dev);
	pm_runtime_suspend(&client->dev);
	pm_runtime_disable(&client->dev);
	if (data->pdata->init)
		data->pdata->init(&client->dev, false);
	i2c_set_clientdata(client, NULL);
	kfree(data);

	AS3649_UNLOCK();
	return 0;
}

#if CONFIG_PM
static const struct dev_pm_ops as3649_pm = {
	SET_RUNTIME_PM_OPS(as3649_suspend, as3649_resume, NULL)
};
#endif

static const struct i2c_device_id as3649_id[] = {
	{ "as3649", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, as3649_id);

static struct i2c_driver as3649_driver = {
	.driver = {
		.name   = "as3649",
#if CONFIG_PM
		.pm = &as3649_pm,
#endif
	},
	.probe  = as3649_probe,
	.remove = as3649_remove,
	.id_table = as3649_id,
};

static int __init as3649_init(void)
{
	return i2c_add_driver(&as3649_driver);
}

static void __exit as3649_exit(void)
{
	i2c_del_driver(&as3649_driver);
}

MODULE_AUTHOR("Ulrich Herrmann <ulrich.herrmann@ams.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AS3649 LED flash light");

module_init(as3649_init);
module_exit(as3649_exit);
