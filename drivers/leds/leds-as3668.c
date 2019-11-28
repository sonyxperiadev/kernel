/*
 * leds-as3668.c - Led flash driver
 *
 * Copyright (C) 2012 Florian Lobmaier <florian.lobmaier@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/leds-as3668.h>

#define AS3668_USE_PREDEFINED_PATTERNS 1

#if (AS3668_USE_PREDEFINED_PATTERNS == 1)
#include "leds-as3668_pattern.h"
#endif


#define AS3668_MAX_OUTPUT_CURRENT 25500 /* uA */
#define AS3668_VMON_MAX_VOLTAGE 3300 /* mA */
#define AS3668_VMON_MIN_VOLTAGE 3000 /* mA */
#define AS3668_VMON_VOLTAGE_STEP 150 /* mA */

#ifdef CONFIG_LEDS_AS3668_EXTENSION
#define AS3668_NUM_LEDS 3
#define AS3668_CURRENT_MAX_PATTERN 4
#else /* CONFIG_LEDS_AS3668_EXTENSION */
#define AS3668_NUM_LEDS 4
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

/* AS3668 registers */
#define AS3668_REG_ChipID1	0x3E
#define AS3668_REG_ChipID2	0x3F
#define AS3668_REG_CurrX_low_voltage_status 0x2B
#define AS3668_REG_Reg_Ctrl	0x00
#define AS3668_REG_CurrX_Ctrl	0x01
#define AS3668_REG_Curr1_Current 0x02
#define AS3668_REG_Curr2_Current 0x03
#define AS3668_REG_Curr3_Current 0x04
#define AS3668_REG_Curr4_Current 0x05
#define AS3668_REG_Gpio_Ctrl	0x06
#define AS3668_REG_Gpio_Output	0x07
#define AS3668_REG_Gpio_Signal	0x08
#define AS3668_REG_Pwm_Ctrl	0x15
#define AS3668_REG_Pwm_Timing	0x16
#define AS3668_REG_Pwm_Trigger	0x17
#define AS3668_REG_Pattern_Timing 0x18
#define AS3668_REG_Multiple_Pulse 0x19
#define AS3668_REG_Frame_Mask	0x1A
#define AS3668_REG_Start_Ctrl	0x1B
#define AS3668_REG_Pattern_Frame_Start_Delay 0x1C
#define AS3668_REG_Gpio_Toggle_Ctrl 0x1D
#define AS3668_REG_Adc_Ctrl	0x26
#define AS3668_REG_Adc_Result	0x27
#define AS3668_REG_CP_Ctrl	0x23
#define AS3668_REG_Overtemp_Ctrl 0x29
#define AS3668_REG_Audio_Agc	0x40
#define AS3668_REG_Audio_Input_Buffer 0x41
#define AS3668_REG_Audio_Ctrl	0x42
#define AS3668_REG_Audio_Output	0x43

#ifdef CONFIG_LEDS_AS3668_EXTENSION
#define AS3668_RGB_BR_MAX	255
#define AS3668_DEFAULT_MONITOR_MV	3000
#define AS3668_DEFAULT_SRC_MASK	0x7
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

#define ldev_to_led(c)       container_of(c, struct as3668_led, ldev)

#define AS3668_WRITE_REG(a, b) as3668_write_reg(data, (a), (b))
#define AS3668_READ_REG(a) as3668_read_reg(data, (a))
#define AS3668_MODIFY_REG(addr, mask, val) \
	as3668_modify_reg(data, (addr), (mask), (val))

#define AS3668_LOCK()   mutex_lock(&(data)->update_lock)
#define AS3668_UNLOCK() mutex_unlock(&(data)->update_lock)

#ifdef CONFIG_LEDS_AS3668_EXTENSION
enum {
	MODE_OFF = 0,
	MODE_ON,
	MODE_BLINK_PWM,
	MODE_BLINK_PATTERN,
};
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

struct as3668_reg {
	const char *name;
	u8 value;
};

struct as3668_led {
	u8 reg;
	u8 mode_reg;
	u8 mode_shift;
	u8 aud_reg;
	u8 aud_shift;
	u8 low_v_status_shift;
	u8 pwm_direction_shift;
	u8 pattern_frame_mask_shift;
	u8 pattern_frame_delay_shift;
	struct i2c_client *client;
	const char *name;
	struct led_classdev ldev;
	struct as3668_platform_led *pled;
	struct work_struct brightness_work;
	struct work_struct blink_work;
	bool use_pattern;
	u8 mode;
	u8 pwm_brightness;
	u8 pattern_frame_mask;
	u8 pattern_frame_delay;
	u8 pattern_brightness;
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	u8 max_single_brightness;
	u8 max_mix_brightness;
	u8 fade_max_brightness;
	u8 fade_min_brightness;
	u8 fade_brightness;
	struct work_struct fade_in_work;
	struct delayed_work delayed_fade_out_work;
	int led_off_delay;
	bool delay_enable;
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	u8 audio_enable;
	u8 audio_brightness;
};

struct as3668_data {
	struct as3668_platform_data *pdata;
	struct i2c_client *client;
	struct as3668_led leds[AS3668_NUM_LEDS];
	struct as3668_reg regs[255];
	struct delayed_work pwm_finished_work;
	int num_leds;
	int pattern_running;
	int dimming_in_progress;
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	unsigned long pattern_on_time;
	unsigned long pattern_off_time;
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	u8 pwm_dim_speed_up;
	u8 pwm_dim_speed_down;
	u8 pwm_dim_shape;
	u8 pwm_dim_mask;
	u8 pwm_dim_direction;
	u8 pattern_ton;
	u8 pattern_toff;
	u8 pattern_source_mask;
	u8 pattern_multiple_pulse;
	u8 pattern_tp_led;
	u8 pattern_fade_out;
	u8 pattern_run;
	int cp_clock;
	u8 cp_on;
	u8 cp_mode_switching;
	u8 cp_mode;
	u8 cp_auto_on;
	u8 gpio_output_state;
	u8 gpio_toggle_enable;
	u8 gpio_toggle_frame_nr;
	u8 audio_agc_on;
	u8 audio_agc_decay_up;
	u8 audio_agc_decay_down;
	u8 audio_agc_up_level;
	u8 audio_agc_down_level;
	u8 audio_decay;
	u8 audio_sync_mode;
	u8 audio_buffer_on;
	u8 audio_adc_on;
	u8 audio_adc_mode;
	u8 audio_on;
	u8 audio_src_mask;
	u8 pattern_predef_run_num;
	int audio_gain;
	struct mutex update_lock;
};

#define AS3668_ATTR(_name)  \
	__ATTR(_name, 0644, as3668_##_name##_show, as3668_##_name##_store)

#define AS3668_DEV_ATTR(_name)  \
	struct device_attribute as3668_##_name = AS3668_ATTR(_name);

#define AS3668_REG(NAME, VAL)[AS3668_REG_##NAME] = \
	{.name = __stringify(NAME), .value = (VAL)}

#define DELTA(A, B) (((A) < (B)) ? ((B)-(A)) : ((A)-(B)))
static u16 dim_speed2time[16] = {0,  120,  250,  380,  510,  770, 1000, 1600,
		2100, 2600, 3100, 4200, 5200, 6200, 7300, 8300};
static u16 pattern_toff_time[8] = {80, 150, 280, 540, 1100, 2100, 4200, 8400};
static u16 pattern_ton_time[8] = {40, 70, 140, 270, 530, 1100, 2100, 4200};
static u16 agc_decay_down_time[4] = {131, 262, 393, 524};
static u16 agc_decay_up_time[4] = {262, 524, 786, 1049};
static u16 audio_decay_time[4] = {10, 20, 40, 80 };
static u16 vbat_monitor_voltage[4] = {2000, 3000, 3150, 3300};
static u16 tp_led_time[4] = {0, 150, 280, 540};

static const struct as3668_data as3668_default_data = {
	.pdata = NULL,
	.client = NULL,
	.num_leds = AS3668_NUM_LEDS,
	.pattern_running = 0,
	.dimming_in_progress = 0,
	.pwm_dim_speed_up = 8,
	.pwm_dim_speed_down = 8,
	.pwm_dim_shape = 0,
	.pwm_dim_mask = 0,
	.pwm_dim_direction = 1,
	.pattern_ton = 0,
	.pattern_toff = 3,
	.pattern_source_mask = 0,
	.pattern_multiple_pulse = 0,
	.pattern_tp_led = 0,
	.pattern_fade_out = 0,
	.cp_clock = 1000,
	.cp_on = 0,
	.cp_mode_switching = 0,
	.cp_mode = 0,
	.cp_auto_on = 0,
	.gpio_output_state = 0,
	.gpio_toggle_enable = 0,
	.gpio_toggle_frame_nr = 0,
	.audio_agc_on = 0,
	.audio_agc_decay_up = 0,
	.audio_agc_decay_down = 0,
	.audio_agc_up_level = 0,
	.audio_agc_down_level = 0,
	.audio_decay = 0,
	.audio_sync_mode = 0,
	.audio_buffer_on = 0,
	.audio_gain = -6,
	.audio_adc_on = 0,
	.audio_adc_mode = 0,
	.audio_on = 0,
	.audio_src_mask = 0,
	.leds = {
		{
			.name = "as3668::curr1",
			.reg = AS3668_REG_Curr1_Current,
			.mode_reg = AS3668_REG_CurrX_Ctrl,
			.mode_shift = 0,
			.aud_reg = AS3668_REG_Audio_Output,
			.aud_shift = 0,
			.low_v_status_shift = 0,
			.pwm_direction_shift = 0,
			.pattern_frame_mask_shift = 0,
			.pattern_frame_delay_shift = 0,
			.mode = 0,
			.pattern_frame_mask = 0,
			.pattern_frame_delay = 0,
			.audio_enable = 0,
		},
		{
			.name = "as3668::curr2",
			.reg = AS3668_REG_Curr2_Current,
			.mode_reg = AS3668_REG_CurrX_Ctrl,
			.mode_shift = 2,
			.aud_reg = AS3668_REG_Audio_Output,
			.aud_shift = 1,
			.low_v_status_shift = 1,
			.pwm_direction_shift = 1,
			.pattern_frame_mask_shift = 2,
			.pattern_frame_delay_shift = 2,
			.mode = 0,
			.pattern_frame_mask = 0,
			.pattern_frame_delay = 0,
			.audio_enable = 0,
		},
		{
			.name = "as3668::curr3",
			.reg = AS3668_REG_Curr3_Current,
			.mode_reg = AS3668_REG_CurrX_Ctrl,
			.mode_shift = 4,
			.aud_reg = AS3668_REG_Audio_Output,
			.aud_shift = 2,
			.low_v_status_shift = 2,
			.pwm_direction_shift = 2,
			.pattern_frame_mask_shift = 4,
			.pattern_frame_delay_shift = 4,
			.mode = 0,
			.pattern_frame_mask = 0,
			.pattern_frame_delay = 0,
			.audio_enable = 0,
		},
#ifndef CONFIG_LEDS_AS3668_EXTENSION
		{
			.name = "as3668::curr4",
			.reg = AS3668_REG_Curr4_Current,
			.mode_reg = AS3668_REG_CurrX_Ctrl,
			.mode_shift = 6,
			.aud_reg = AS3668_REG_Audio_Output,
			.aud_shift = 3,
			.low_v_status_shift = 3,
			.pwm_direction_shift = 3,
			.pattern_frame_mask_shift = 6,
			.pattern_frame_delay_shift = 6,
			.mode = 0,
			.pattern_frame_mask = 0,
			.pattern_frame_delay = 0,
			.audio_enable = 0,
		},
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	},
	.regs = {
		AS3668_REG(ChipID1, 0xA5),
		AS3668_REG(ChipID2, 0x51),
		AS3668_REG(CurrX_Ctrl, 0x00),
		AS3668_REG(Curr1_Current, 0x00),
		AS3668_REG(Curr2_Current, 0x00),
		AS3668_REG(Curr3_Current, 0x00),
		AS3668_REG(Curr4_Current, 0x00),
		AS3668_REG(Gpio_Ctrl, 0x02),
		AS3668_REG(Gpio_Signal, 0x00),
		AS3668_REG(Pwm_Ctrl, 0x00),
		AS3668_REG(Pwm_Timing, 0x88),
		AS3668_REG(Pwm_Trigger, 0x00),
		AS3668_REG(Pattern_Timing, 0x18),
		AS3668_REG(Multiple_Pulse, 0x00),
		AS3668_REG(Frame_Mask, 0x00),
		AS3668_REG(Start_Ctrl, 0x01),
		AS3668_REG(Pattern_Frame_Start_Delay, 0x00),
		AS3668_REG(Gpio_Toggle_Ctrl, 0x00),
		AS3668_REG(Adc_Ctrl, 0x00),
		AS3668_REG(Adc_Result, 0x00),
		AS3668_REG(Reg_Ctrl, 0x00),
		AS3668_REG(CP_Ctrl, 0x40),
		AS3668_REG(Overtemp_Ctrl, 0x11),
		AS3668_REG(Audio_Agc, 0x00),
		AS3668_REG(Audio_Input_Buffer, 0x00),
		AS3668_REG(Audio_Ctrl, 0x00),
		AS3668_REG(Audio_Output, 0x00),
	},
};

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static struct as3668_platform_data as3668_ext_data = {
	.vbat_monitor_voltage_mV = 3000,
	.pattern_start_source = AS3668_PATTERN_START_SOURCE_SW,
	.pwm_source = AS3668_PWM_SOURCE_INTERNAL,
	.gpio_input_invert = 0,
	.gpio_input_mode = AS3668_GPIO_INPUT_MODE_ANALOG,
	.gpio_mode = AS3668_GPIO_MODE_INPUT_ONLY,
	.audio_input_pin = AS3668_AUDIO_INPUT_CURR4,
	.audio_pulldown_off = 0,
	.audio_man_start = AS3668_AUDIO_AUTO_PRECHARGE,
	.audio_dis_start = AS3668_AUDIO_INPUT_CAP_PRECHARGE,
	.audio_adc_characteristic = 0,
	.leds[0] = {
		.name = "blue",
		.max_current_uA = 25500,
	},
	.leds[1] = {
		.name = "red",
		.max_current_uA = 25500,
	},
	.leds[2] = {
		.name = "green",
		.max_current_uA = 25500,
	},
};

static int rgb_current_index;
static int __init rgb_current_setup(char *str)
{
	unsigned int res;

	if (!*str)
		return 0;
	if (!kstrtouint(str, 16, &res))
		rgb_current_index = res;
	return 1;
}
__setup("oemandroidboot.babe137e=", rgb_current_setup);
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static s32 as3668_write_reg(struct as3668_data *data, u8 addr, u8 val)
{
	s32 ret;

	ret = i2c_smbus_write_byte_data(data->client, addr, val);
	if (ret == 0)
		data->regs[addr].value = val;
	return ret;
}

static u8 as3668_read_reg(struct as3668_data *data, u8 addr)
{
	return data->regs[addr].value;
}

static s32 as3668_modify_reg(struct as3668_data *data, u8 addr, u8 mask, u8 val)
{
	u8 value;

	value = as3668_read_reg(data, addr);
	value &= ~mask;
	value |= (val & mask);
	return as3668_write_reg(data, addr, value);
}

#ifdef CONFIG_PM
static void as3668_shutdown(struct i2c_client *client)
{
	struct as3668_data *data = i2c_get_clientdata(client);
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	unsigned i;
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

	dev_info(&client->dev, "Shutting down AS3668\n");

	AS3668_LOCK();
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	for (i = 0; i < data->num_leds; i++) {
		AS3668_WRITE_REG(data->leds[i].reg, LED_OFF);
		AS3668_WRITE_REG(data->leds[i].mode_reg, MODE_OFF);
	}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	AS3668_UNLOCK();
}
#endif

static int device_add_attributes(struct device *dev,
		struct device_attribute *attrs)
{
	int error = 0;
	int i;

	if (attrs) {
		for (i = 0; attrs[i].attr.name; i++) {
			error = device_create_file(dev, &attrs[i]);
			if (error)
				break;
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
		for (i = 0; attrs[i].attr.name; i++)
			device_remove_file(dev, &attrs[i]);
}

static int as3668_set_pattern_ton(struct as3668_data *data, u16 ton_time)
{
	int i, curr_best = 0, delta;

	if (ton_time > 5000)
		return -EINVAL;
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(pattern_ton_time) ; i++) {
		if (DELTA(ton_time, pattern_ton_time[i]) < delta) {
			delta = DELTA(ton_time, pattern_ton_time[i]);
			curr_best = i;
		}
	}
	data->pattern_ton = curr_best;
	AS3668_MODIFY_REG(AS3668_REG_Pattern_Timing, 0x07, curr_best);
	return 0;
}

static int as3668_set_pattern_toff(struct as3668_data *data, u16 toff_time)
{
	int i, curr_best = 0, delta;

	if (toff_time > 10000)
		return -EINVAL;
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(pattern_toff_time) ; i++) {
		if (DELTA(toff_time, pattern_toff_time[i]) < delta) {
			delta = DELTA(toff_time, pattern_toff_time[i]);
			curr_best = i;
		}
	}
	data->pattern_toff = curr_best;
	AS3668_MODIFY_REG(AS3668_REG_Pattern_Timing, 0x38, (curr_best << 3));
	return 0;
}

static void as3668_set_vbat_monitor_voltage(struct as3668_data *data)
{
	int i, curr_best = 0, delta;

	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(vbat_monitor_voltage) ; i++) {
		if (DELTA(data->pdata->vbat_monitor_voltage_mV,
					vbat_monitor_voltage[i]) < delta) {
			delta = DELTA(data->pdata->vbat_monitor_voltage_mV,
					vbat_monitor_voltage[i]);
			curr_best = i;
		}
	}
	AS3668_MODIFY_REG(AS3668_REG_Overtemp_Ctrl, 0x60, (curr_best << 5));
}

void as3668_switch_led(struct as3668_data *data,
		struct as3668_led *led,
		int mode)
{
	AS3668_MODIFY_REG(led->mode_reg,
			(0x03 << led->mode_shift),
			(mode << led->mode_shift));
	led->mode = mode;
}

static bool as3668_pattern_in_use(struct as3668_data *data)
{
	int i;

	for (i = 0; i < data->num_leds; i++) {
		if (data->leds[i].mode == 0x03)
			return true;
	}
	return false;
}

static void as3668_pwm_finished_work(struct work_struct  *work)
{
	struct as3668_data *data = container_of(work, struct as3668_data,
						pwm_finished_work.work);
	struct i2c_client *client = data->client;
	struct device *dev = &client->dev;

	dev_info(dev, "pwm dimming finished\n");
	data->pattern_running = 0;
}

/** device driver files *****************************************************/
static ssize_t as3668_debug_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE, cw = 0;
	u8 cr;
	int i = 0;
	struct as3668_reg *reg;

	while (cw < ps && i < ARRAY_SIZE(data->regs)) {
		reg = data->regs + i;
		if (reg->name) {
			cr = i2c_smbus_read_byte_data(data->client, i);
			if (cr == reg->value)
				cw = scnprintf(buf, ps, "%24s %02x: %#04x\n",
					reg->name, i, reg->value);
			else
				cw = scnprintf(buf, ps,
						"%24s %02x: %#04x -> %#04x\n",
						reg->name, i, reg->value, cr);
			ps -= cw;
			buf += cw;
		}
		i++;
	}
	ps -= cw;
	buf += cw;
	return PAGE_SIZE - ps;
}

static ssize_t as3668_debug_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	u8 reg, val;

	i = sscanf(buf, "0x%hhx=0x%hhx", &reg, &val);
	if (i != 2)
		return -EINVAL;
	AS3668_LOCK();
	AS3668_WRITE_REG(reg, val);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_pwm_dim_speed_down_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n",	dim_speed2time[data->pwm_dim_speed_down]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_pwm_dim_speed_down_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int dim_time, i, curr_best = 0, delta;

	i = sscanf(buf, "%d", &dim_time);
	if (i != 1 || dim_time > 10000)
		return -EINVAL;
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(dim_speed2time) ; i++) {
		if (DELTA(dim_time, dim_speed2time[i]) < delta) {
			delta = DELTA(dim_time, dim_speed2time[i]);
			curr_best = i;
		}
	}
	AS3668_LOCK();
	data->pwm_dim_speed_down = curr_best;
	AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0x0F, curr_best);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_pwm_dim_speed_up_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
		"%d\n",	dim_speed2time[data->pwm_dim_speed_up]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_pwm_dim_speed_up_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int dim_time, i, curr_best = 0, delta;

	i = sscanf(buf, "%d", &dim_time);
	if (i != 1 || dim_time > 10000)
		return -EINVAL;
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(dim_speed2time) ; i++) {
		if (DELTA(dim_time, dim_speed2time[i]) < delta) {
			delta = DELTA(dim_time, dim_speed2time[i]);
			curr_best = i;
		}
	}
	AS3668_LOCK();
	data->pwm_dim_speed_up = curr_best;
	AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0xF0, curr_best << 4);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_pattern_time_on_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", (int)data->pattern_on_time);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_time_on_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_ton;

	i = sscanf(buf, "%d", &pattern_ton);
	if (pattern_ton < 0)
		return -EINVAL;

	data->pattern_on_time = pattern_ton;

	return strnlen(buf, PAGE_SIZE);
}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
static ssize_t as3668_pattern_time_on_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", pattern_ton_time[data->pattern_ton]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_time_on_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, err;
	int pattern_ton;

	i = sscanf(buf, "%d", &pattern_ton);
	if (i != 1)
		return -EINVAL;
	err = as3668_set_pattern_ton(data, (u16)pattern_ton);
	if (err)
		return -EINVAL;
	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_pattern_time_off_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", (int)data->pattern_off_time);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_time_off_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_toff;

	i = sscanf(buf, "%d", &pattern_toff);
	if (pattern_toff < 0)
		return -EINVAL;

	data->pattern_off_time = pattern_toff;

	return strnlen(buf, PAGE_SIZE);
}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
static ssize_t as3668_pattern_time_off_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", pattern_toff_time[data->pattern_toff]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_time_off_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, err;
	int pattern_toff;

	i = sscanf(buf, "%d", &pattern_toff);
	if (i != 1)
		return -EINVAL;
	err = as3668_set_pattern_toff(data, (u16)pattern_toff);
	if (err)
		return -EINVAL;
	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static ssize_t as3668_pattern_source_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "0x%x\n", data->pattern_source_mask);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_source_mask_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_mask;

	i = sscanf(buf, "0x%x", &pattern_mask);
	if (i != 1) {
		dev_info(dev, "define pattern source mask in "
			"format 0x01 - 0x0F (bit 0 - curr1, "
			"bit 1 - curr 2,...)\n");
		return -EINVAL;
	}
	data->pattern_source_mask = pattern_mask;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_multiple_pulse_number_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->pattern_multiple_pulse + 1);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_multiple_pulse_number_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_pulse;

	i = sscanf(buf, "%d", &pattern_pulse);
	if ((i != 1) || (pattern_pulse > 4) || (pattern_pulse < 1)) {
		dev_info(dev, "pattern number of multiple pulses must be in range 1-4\n");
		return -EINVAL;
	}
	data->pattern_multiple_pulse = pattern_pulse - 1;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_tp_led_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", tp_led_time[data->pattern_tp_led]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_tp_led_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, curr_best = 0, delta;
	int value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 540) || (value < 0)) {
		dev_info(dev, "pattern tp_led defines pause time for multiple"
			" pulse mode. Please specify ms in range 0-540\n");
		return -EINVAL;
	}
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(tp_led_time) ; i++) {
		if (DELTA(value, tp_led_time[i]) < delta) {
			delta = DELTA(value, tp_led_time[i]);
			curr_best = i;
		}
	}
	data->pattern_tp_led = curr_best;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_fade_out_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->pattern_fade_out);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_fade_out_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_fadeout;

	i = sscanf(buf, "%d", &pattern_fadeout);
	if ((i != 1) || (pattern_fadeout > 1) || (pattern_fadeout < 0)) {
		dev_info(dev, "enable pattern fade out before starting pattern by writing 1, disable by writing 0\n");
		return -EINVAL;
	}
	data->pattern_fade_out = pattern_fadeout;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_run_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->pattern_run);
	return strnlen(buf, PAGE_SIZE);
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_pattern_run_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	struct as3668_led *led;
	int i;
	int pattern_run;
	u8 mode_mask = 0, mode_ctrl = 0;

	i = sscanf(buf, "%d", &pattern_run);
	if ((i != 1) || (pattern_run > 1) || (pattern_run < 0)) {
		dev_info(dev,
			"define pattern run as 0 or 1 (1.. start pattern, 0.. stop pattern)\n");
		return -EINVAL;
	}
	if ((pattern_run == 1) && (as3668_pattern_in_use(data)) &&
			(data->pattern_running == 1)) {
		dev_info(dev, "pattern generator already in use\n");
		return -EBUSY;
	}

	data->pattern_run = pattern_run;

	if (pattern_run == 1) {
		AS3668_LOCK();
		as3668_set_pattern_ton(data, (u16)data->pattern_on_time);
		as3668_set_pattern_toff(data, (u16)data->pattern_off_time);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0x0F,
				data->pwm_dim_speed_down);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0xF0,
				data->pwm_dim_speed_up << 4);
		AS3668_MODIFY_REG(AS3668_REG_Multiple_Pulse, 0xC0,
			data->pattern_multiple_pulse << 6);
		AS3668_MODIFY_REG(AS3668_REG_Multiple_Pulse, 0x03,
				data->pattern_tp_led);
		data->pattern_running = 1;
		for (i = 0; i < data->num_leds; i++) {
			led = &data->leds[i];
			mode_mask |= (0x03 << led->mode_shift);
			if (led->pattern_brightness) {
				AS3668_MODIFY_REG(AS3668_REG_Frame_Mask,
					0x03 << led->pattern_frame_mask_shift,
					led->pattern_frame_mask << led->pattern_frame_mask_shift);
				mode_ctrl |= (MODE_BLINK_PATTERN << led->mode_shift);
			} else {
				AS3668_MODIFY_REG(AS3668_REG_Frame_Mask,
					0x03 << led->pattern_frame_mask_shift,
					0 << led->pattern_frame_mask_shift);
				mode_ctrl |= (MODE_OFF << led->mode_shift);
			}
			AS3668_WRITE_REG(led->reg, led->pattern_brightness);
		}
		AS3668_MODIFY_REG(AS3668_REG_CurrX_Ctrl, mode_mask, mode_ctrl);
		AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x06,
			((data->pattern_fade_out << 2) | (pattern_run << 1)));
		AS3668_UNLOCK();
	} else {
		AS3668_LOCK();
		data->pattern_running = 0;
		for (i = 0; i < data->num_leds; i++) {
			led = &data->leds[i];
			as3668_switch_led(data, led, MODE_OFF);
			AS3668_MODIFY_REG(AS3668_REG_Frame_Mask,
				0x03 << led->pattern_frame_mask_shift,
				0 << led->pattern_frame_mask_shift);
			AS3668_WRITE_REG(led->reg, LED_OFF);
		}
		AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x02, (pattern_run << 1));
		AS3668_UNLOCK();
	}
	return strnlen(buf, PAGE_SIZE);
}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
static ssize_t as3668_pattern_run_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pattern_run;

	i = sscanf(buf, "%d", &pattern_run);
	if ((i != 1) || (pattern_run > 1) || (pattern_run < 0)) {
		dev_info(dev, "define pattern run as 0 or 1 (1.. start pattern, 0.. stop pattern)\n");
		return -EINVAL;
	}
	if ((pattern_run == 1) && (as3668_pattern_in_use(data)) && (data->pattern_running == 1)) {
		dev_info(dev, "pattern generator already in use\n");
		return -EBUSY;
	}

	data->pattern_run = pattern_run;

	if (pattern_run == 1) {
		AS3668_LOCK();
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0x0F, data->pwm_dim_speed_down);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0xF0, data->pwm_dim_speed_up << 4);
		AS3668_MODIFY_REG(AS3668_REG_Multiple_Pulse, 0xC0, data->pattern_multiple_pulse << 6);
		AS3668_MODIFY_REG(AS3668_REG_Multiple_Pulse, 0x03, data->pattern_tp_led);
		if (data->pattern_source_mask & 0x01) {
			as3668_switch_led(data, &data->leds[0], 0x03);
			AS3668_WRITE_REG(AS3668_REG_Curr1_Current, data->leds[0].pattern_brightness);
		}
		if (data->pattern_source_mask & 0x02) {
			as3668_switch_led(data, &data->leds[1], 0x03);
			AS3668_WRITE_REG(AS3668_REG_Curr2_Current, data->leds[1].pattern_brightness);
		}
		if (data->pattern_source_mask & 0x04) {
			as3668_switch_led(data, &data->leds[2], 0x03);
			AS3668_WRITE_REG(AS3668_REG_Curr3_Current, data->leds[2].pattern_brightness);
		}
		if (data->pattern_source_mask & 0x08) {
			as3668_switch_led(data, &data->leds[3], 0x03);
			AS3668_WRITE_REG(AS3668_REG_Curr4_Current, data->leds[3].pattern_brightness);
		}
		AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x07,
			((data->pattern_fade_out << 2) | (pattern_run << 1) |
			 data->pdata->pattern_start_source));
		data->pattern_running = 1;
		AS3668_UNLOCK();
	} else {
		AS3668_LOCK();
		data->pattern_running = 0;
		AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x02,
				(pattern_run << 1));
		AS3668_UNLOCK();
	}
	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static ssize_t as3668_pwm_dim_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "0x%x\n", data->pwm_dim_mask);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_mask_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pwm_mask;

	i = sscanf(buf, "0x%x", &pwm_mask);
	if (i != 1) {
		dev_info(dev, "define pwm mask in "
			"format 0x01 - 0x0F (bit 0 - curr1, "
			"bit 1 - curr 2,...)\n");
		return -EINVAL;
	}
	data->pwm_dim_mask = pwm_mask;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_shape_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d (0.. logarithmic ramp, 1.. linear ramp)\n",
			data->pwm_dim_shape);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_shape_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pwm_shape;

	i = sscanf(buf, "%d", &pwm_shape);
	if ((i != 1) || (pwm_shape > 1) || (pwm_shape < 0)) {
		dev_info(dev, "define pwm dim shape as 0 or 1 (0.. logarithmic ramp, 1.. linear ramp)\n");
		return -EINVAL;
	}
	data->pwm_dim_shape = pwm_shape;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_direction_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d (0.. dim_down, 1.. dim_up)\n",
			data->pwm_dim_direction);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_direction_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pwm_dir;

	i = sscanf(buf, "%d", &pwm_dir);
	if ((i != 1) || (pwm_dir > 1) || (pwm_dir < 0)) {
		dev_info(dev, "define pwm dim direction as 0 or 1 (0.. dim_down, 1.. dim_up)\n");
		return -EINVAL;
	}
	data->pwm_dim_direction = pwm_dir;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_start_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->pattern_running);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_start_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int pwm_start;
	u8 direction;

	i = sscanf(buf, "%d", &pwm_start);
	if ((i != 1) || (pwm_start > 1) || (pwm_start < 0)) {
		dev_info(dev, "define pwm start as 1 to start PWM dimming\n");
		return -EINVAL;
	}
	if (as3668_pattern_in_use(data)) {
		dev_info(dev, "pattern generator already in use\n");
		return -EBUSY;
	}

	if (pwm_start == 1) {
		AS3668_LOCK();
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Ctrl, 0x01,
				data->pdata->pwm_source);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Ctrl, 0x02,
				data->pwm_dim_shape << 1);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0x0F,
				data->pwm_dim_speed_down);
		AS3668_MODIFY_REG(AS3668_REG_Pwm_Timing, 0xF0,
				data->pwm_dim_speed_up << 4);
		direction = 0;
#ifdef CONFIG_LEDS_AS3668_EXTENSION
		for (i = 0; i < data->num_leds; i++) {
			if (data->pwm_dim_mask & (1 << i)) {
				as3668_switch_led(data, &data->leds[i], 0x02);
				AS3668_WRITE_REG(data->leds[i].reg,
					data->leds[i].pwm_brightness);
			}
		}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
		if (data->pwm_dim_mask & 0x01) {
			as3668_switch_led(data, &data->leds[0], 0x02);
			AS3668_WRITE_REG(AS3668_REG_Curr1_Current, data->leds[0].pwm_brightness);
		}
		if (data->pwm_dim_mask & 0x02) {
			as3668_switch_led(data, &data->leds[1], 0x02);
			AS3668_WRITE_REG(AS3668_REG_Curr2_Current, data->leds[1].pwm_brightness);
		}
		if (data->pwm_dim_mask & 0x04) {
			as3668_switch_led(data, &data->leds[2], 0x02);
			AS3668_WRITE_REG(AS3668_REG_Curr3_Current, data->leds[2].pwm_brightness);
		}
		if (data->pwm_dim_mask & 0x08) {
			as3668_switch_led(data, &data->leds[3], 0x02);
			AS3668_WRITE_REG(AS3668_REG_Curr4_Current, data->leds[3].pwm_brightness);
		}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
		data->pattern_running = 1;
		if (data->pwm_dim_direction == 1) {
			AS3668_MODIFY_REG(AS3668_REG_Pwm_Trigger,
					(0x10 | data->pwm_dim_mask), 0x1F);
			schedule_delayed_work(&data->pwm_finished_work,
				msecs_to_jiffies(
					dim_speed2time[data->pwm_dim_speed_up]
					));
		} else {
			AS3668_MODIFY_REG(AS3668_REG_Pwm_Trigger,
					(0x10 | data->pwm_dim_mask), 0x10);
			schedule_delayed_work(&data->pwm_finished_work,
				msecs_to_jiffies(
					dim_speed2time[data->pwm_dim_speed_down]
					));
		}
		AS3668_UNLOCK();
	}
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_clock_khz_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->cp_clock);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_clock_khz_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, cp_clock;
	u8 cp_clock_bit;

	i = sscanf(buf, "%d", &cp_clock);
	if (i != 1 || cp_clock > 1000) {
		dev_info(dev, "specify charge pump clock in kHz (500 or 1000kHz allowed)\n");
		return -EINVAL;
	}
	if (cp_clock > 750) {
		cp_clock = 1000;
		cp_clock_bit = 0;
	} else {
		cp_clock = 500;
		cp_clock_bit = 1;
	}
	AS3668_LOCK();
	data->cp_clock = cp_clock;
	AS3668_MODIFY_REG(AS3668_REG_CP_Ctrl, 0x01, cp_clock_bit);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->cp_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, cp_on;

	i = sscanf(buf, "%d", &cp_on);
	if ((i != 1) || (cp_on > 1) || (cp_on < 0)) {
		dev_info(dev, "specify charge pump on as 0.. off or 1.. on\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->cp_on = cp_on;
	AS3668_MODIFY_REG(AS3668_REG_Reg_Ctrl, 0x04, cp_on << 2);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->cp_mode);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, cp_mode;

	i = sscanf(buf, "%d", &cp_mode);
	if ((i != 1) || (cp_mode > 1) || (cp_mode < 0)) {
		dev_info(dev, "specify charge pump mode as 0.. 1:1 mode or 1.. 1:2 mode\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->cp_mode = cp_mode;
	AS3668_MODIFY_REG(AS3668_REG_CP_Ctrl, 0x04, cp_mode << 2);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_mode_switching_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->cp_mode_switching);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_mode_switching_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, cp_mode_sw;

	i = sscanf(buf, "%d", &cp_mode_sw);
	if ((i != 1) || (cp_mode_sw > 1) || (cp_mode_sw < 0)) {
		dev_info(dev, "specify charge pump mode switching as 0.. Automatic Mode or 1.. Manual Mode\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->cp_mode_switching = cp_mode_sw;
	AS3668_MODIFY_REG(AS3668_REG_CP_Ctrl, 0x10, cp_mode_sw << 4);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_auto_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->cp_auto_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_cp_auto_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, cp_auto_on;

	i = sscanf(buf, "%d", &cp_auto_on);
	if ((i != 1) || (cp_auto_on > 1) || (cp_auto_on < 0)) {
		dev_info(dev, "specify charge pump auto on as 0.. Manual CP mode or 1.. Automatic CP mode\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->cp_auto_on = cp_auto_on;
	AS3668_MODIFY_REG(AS3668_REG_CP_Ctrl, 0x40, cp_auto_on << 6);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_input_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	u8 gpio_in;

	gpio_in = i2c_smbus_read_byte_data(data->client,
			AS3668_REG_Gpio_Signal);
	snprintf(buf, PAGE_SIZE,
	"%d\n", (gpio_in & 0x01));
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_input_state_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_output_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->gpio_output_state);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_output_state_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, gpio_out;

	i = sscanf(buf, "%d", &gpio_out);
	if ((i != 1) || (gpio_out > 1) || (gpio_out < 0)) {
		dev_info(dev, "specify GPIO output as 0.. GPIO pin low or 1.. GPIO pin high\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->gpio_output_state = gpio_out;
	AS3668_MODIFY_REG(AS3668_REG_Gpio_Output, 0x01, gpio_out);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_toggle_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->gpio_toggle_enable);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_toggle_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify GPIO toggle enable as 0.. disabled or 1.. enabled\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->gpio_toggle_enable = value;
	AS3668_MODIFY_REG(AS3668_REG_Gpio_Toggle_Ctrl, 0x04, (value << 2));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_toggle_frame_nr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE,
	"%d\n", data->gpio_toggle_frame_nr+1);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_gpio_toggle_frame_nr_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 4) || (value < 1)) {
		dev_info(dev, "specify GPIO toggle frame number as\n"
			"1.. 1 frame\n"
			"2.. 2 frames\n"
			"3.. 3 frames\n"
			"4.. 4 frames\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->gpio_toggle_frame_nr = value-1;
	AS3668_MODIFY_REG(AS3668_REG_Gpio_Toggle_Ctrl, 0x03, (value-1));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_agc_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	 int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio AGC on as 0.. AGC off or 1.. AGC on\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_agc_on = value;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x01, (value & 0x01));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_down_level_inc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_agc_down_level);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_down_level_inc_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	 int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio AGC down level increase as"
			" 0.. AGC normal threshold or"
			" 1.. AGC down switching threshold increased\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_agc_down_level = value;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x20, (value & 0x01) << 5);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_up_level_inc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_agc_up_level);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_up_level_inc_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	 int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio AGC up level increase as"
			" 0.. AGC normal threshold or"
			" 1.. AGC up switching threshold increased\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_agc_up_level = value;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x40, (value & 0x01) << 6);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_decay_down_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n",
			agc_decay_down_time[data->audio_agc_decay_down]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_decay_down_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value, curr_best = 0, delta;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 2000) || (value < 0)) {
		dev_info(dev, "specify audio AGC decay down in ms < 2000 ms\n");
		return -EINVAL;
	}
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(agc_decay_down_time) ; i++) {
		if (DELTA(value, agc_decay_down_time[i]) < delta) {
			delta = DELTA(value, agc_decay_down_time[i]);
			curr_best = i;
		}
	}

	AS3668_LOCK();
	data->audio_agc_decay_down = curr_best & 0x03;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x18, (curr_best & 0x03) << 3);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_decay_up_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n",
			agc_decay_up_time[data->audio_agc_decay_up]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_agc_decay_up_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value, curr_best = 0, delta;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 2000) || (value < 0)) {
		dev_info(dev, "specify audio AGC decay up in ms < 2000 ms\n");
		return -EINVAL;
	}
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(agc_decay_up_time) ; i++) {
		if (DELTA(value, agc_decay_up_time[i]) < delta) {
			delta = DELTA(value, agc_decay_up_time[i]);
			curr_best = i;
		}
	}
	AS3668_LOCK();
	data->audio_agc_decay_up = curr_best & 0x03;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x06, (curr_best & 0x03) << 1);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_decay_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", audio_decay_time[data->audio_decay]);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_decay_ms_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value, curr_best = 0, delta;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 100) || (value < 0)) {
		dev_info(dev, "specify audio decay in ms < 100 ms\n");
		return -EINVAL;
	}
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(audio_decay_time) ; i++) {
		if (DELTA(value, audio_decay_time[i]) < delta) {
			delta = DELTA(value, audio_decay_time[i]);
			curr_best = i;
		}
	}
	AS3668_LOCK();
	data->audio_decay = curr_best & 0x03;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Ctrl, 0x18, (curr_best & 0x03) << 3);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_sync_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_sync_mode);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_sync_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	 int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 6) || (value < 0)) {
		dev_info(dev, "specify audio sync mode as follows:\n"
			"0.. 4 LED bar code\n"
			"1.. 4 LED bar code with dimming\n"
			"2.. running LED bar code\n"
			"3.. running LED bar code with dimming\n"
			"4.. RGB\n"
			"5.. RGB with dimming\n"
			"6.. LED parallel with dimming\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_sync_mode = value;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Ctrl, 0x07, (value & 0x07));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_gain_dB_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_gain);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_gain_dB_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 25) || (value < -6)) {
		dev_info(dev, "specify audio gain in dB from -6 to 25dB\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_gain = value;
	value = value + 6; /* add offset for register bit value */
	AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x3E,
			(value & 0x1F) << 1);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_buffer_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_buffer_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_buffer_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio buffer on as 0.. off or 1.. on\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_buffer_on = value;
	AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x01, value);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_adc_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_adc_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_adc_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio adc on as 0.. off or 1.. on\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_adc_on = value;
	AS3668_MODIFY_REG(AS3668_REG_Adc_Ctrl, 0x01, value);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_adc_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_adc_mode);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_adc_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio adc mode as 0.. linear ADC or 1.. logarithmic ADC\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_adc_mode = value;
	AS3668_MODIFY_REG(AS3668_REG_Adc_Ctrl, 0x02, (value << 1));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_src_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "0x%x\n", data->audio_src_mask);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_src_mask_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i;
	int mask;

	i = sscanf(buf, "0x%x", &mask);
	if (i != 1) {
		dev_info(dev, "define audio source mask in "
		"format 0x01 - 0x0F (bit 0 - curr1, "
		"bit 1 - curr2,...)\n");
		return -EINVAL;
	}
	data->audio_src_mask = mask;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->audio_on);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_on_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value;

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > 1) || (value < 0)) {
		dev_info(dev, "specify audio on as 0.. off or 1.. on\n");
		return -EINVAL;
	}
	AS3668_LOCK();
	data->audio_on = value;
	if (value) {
		for (i = 0; i < AS3668_NUM_LEDS; i++) {
			if (data->audio_src_mask & (1 << i)) {
				AS3668_WRITE_REG(data->leds[i].reg,
						data->leds[i].audio_brightness);
				as3668_switch_led(data, &data->leds[i], 0x01);
				data->leds[i].audio_enable = 1;
			}
			AS3668_MODIFY_REG(AS3668_REG_Audio_Output, 0x0F,
					data->audio_src_mask);
		}
		AS3668_MODIFY_REG(AS3668_REG_Adc_Ctrl, 0x01, 0x01);
		data->audio_adc_on = 1;
		AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x01, 0x01);
		data->audio_agc_on = 1;
		AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x01, 0x01);
		data->audio_buffer_on = 1;
	} else {
		for (i = 0; i < AS3668_NUM_LEDS; i++) {
			if (data->audio_src_mask & (1 << i)) {
				AS3668_WRITE_REG(data->leds[i].reg, 0x00);
				as3668_switch_led(data, &data->leds[i], 0x00);
				data->leds[i].audio_enable = 0;
			}
			AS3668_MODIFY_REG(AS3668_REG_Audio_Output, 0x0F, 0x00);
		}
		AS3668_MODIFY_REG(AS3668_REG_Adc_Ctrl, 0x01, 0x00);
		data->audio_adc_on = 0;
		AS3668_MODIFY_REG(AS3668_REG_Audio_Agc, 0x01, 0x00);
		data->audio_agc_on = 0;
		AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x01, 0x00);
		data->audio_buffer_on = 0;
	}
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

#if (AS3668_USE_PREDEFINED_PATTERNS == 1)
static ssize_t as3668_pattern_predef_run_num_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_data *data = dev_get_drvdata(dev);

	snprintf(buf, PAGE_SIZE, "%d\n", data->pattern_predef_run_num);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_predef_run_num_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_data *data = dev_get_drvdata(dev);
	int i, value, max_pattern_num;
	u8 pattern_size;

	const as3668_pattern *pattern2run;

	max_pattern_num =
		sizeof(as3668_pattern_array) / sizeof(as3668_pattern *);

	i = sscanf(buf, "%d", &value);
	if ((i != 1) || (value > max_pattern_num) || (value < 0)) {
		dev_info(dev,
			"specify pre-defined pattern number from 0 to %d\n",
			max_pattern_num);
		return -EINVAL;
	}
	AS3668_LOCK();
	data->pattern_predef_run_num = value;
	pattern2run = as3668_pattern_array[value];
	pattern_size = as3668_pattern_size[value];
	for (i = 0; i < pattern_size; i++)
		AS3668_WRITE_REG(pattern2run[i].reg_addr, pattern2run[i].value);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}
#endif
#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_fade_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->fade_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_fade_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int fade_brightness;

	i = sscanf(buf, "%d", &fade_brightness);
	if (i != 1)
		return -EINVAL;

	led->fade_brightness = (u8)fade_brightness;
	if (led->fade_brightness != LED_OFF) {
		led->delay_enable = false;
		cancel_work_sync(&led->fade_in_work);
		cancel_delayed_work_sync(&led->delayed_fade_out_work);

		schedule_work(&led->fade_in_work);
		schedule_delayed_work(&led->delayed_fade_out_work,
				msecs_to_jiffies((u16)led->led_off_delay));
	} else {
		if (delayed_work_pending(&led->delayed_fade_out_work)) {
			led->delay_enable = true;
		} else {
			led->delay_enable = true;
			schedule_delayed_work(&led->delayed_fade_out_work, 0);
		}
	}

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_fade_max_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->fade_max_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_fade_max_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int fade_max_brightness;

	i = sscanf(buf, "%d", &fade_max_brightness);
	if (i != 1)
		return -EINVAL;
	led->fade_max_brightness = (u8)fade_max_brightness;

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_fade_min_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->fade_min_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_fade_min_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int fade_min_brightness;

	i = sscanf(buf, "%d", &fade_min_brightness);
	if (i != 1)
		return -EINVAL;
	led->fade_min_brightness = (u8)fade_min_brightness;

	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

/** led class device files **************************************************/
#ifdef CONFIG_LEDS_AS3668_EXTENSION
static void as3668_set_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct as3668_led *led = ldev_to_led(led_cdev);

	if (value < LED_OFF) {
		dev_err(&led->client->dev, "Invalid brightness value\n");
		return;
	}

	if (value > led_cdev->max_brightness)
		value = led_cdev->max_brightness;

	led->ldev.brightness = (u8)value;
	schedule_work(&led->brightness_work);
}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
static void as3668_set_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct as3668_led *led = ldev_to_led(led_cdev);

	led->ldev.brightness = (u8)value;
	schedule_work(&led->brightness_work);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static void as3668_set_brightness_work(struct work_struct  *work)
{
	struct as3668_led *led = container_of(work, struct as3668_led,
			brightness_work);
	struct i2c_client *client = led->client;
	struct device *dev = &client->dev;
	struct as3668_data *data = dev_get_drvdata(dev);

	if (led->ldev.brightness == LED_OFF)
		led->mode = 0x00;
	else
		led->mode = 0x01;
	AS3668_LOCK();
	as3668_switch_led(data, led, led->mode);
	AS3668_WRITE_REG(led->reg, led->ldev.brightness);
	AS3668_UNLOCK();
}
#ifdef CONFIG_LEDS_AS3668_EXTENSION
static void as3668_set_fade_in_work(struct work_struct  *work)
{
	struct as3668_led *led = container_of(work, struct as3668_led,
			fade_in_work);
	struct i2c_client *client = led->client;
	struct device *dev = &client->dev;
	struct as3668_data *data = dev_get_drvdata(dev);
	u8 brightness;
	u8 current_brightness;

	led->mode = 0x01;
	AS3668_LOCK();
	current_brightness = AS3668_READ_REG(led->reg);
	as3668_switch_led(data, led, led->mode);
	for (brightness = current_brightness;
			brightness <= led->fade_max_brightness; brightness++) {
		msleep(10);
		AS3668_WRITE_REG(led->reg, brightness);
	}
	AS3668_UNLOCK();
}
static void as3668_set_delayed_fade_out_work(struct work_struct  *work)
{
	struct as3668_led *led = container_of(work, struct as3668_led,
			delayed_fade_out_work.work);
	struct i2c_client *client = led->client;
	struct device *dev = &client->dev;
	struct as3668_data *data = dev_get_drvdata(dev);
	u8 brightness;
	u8 current_brightness;

	if (!led->delay_enable) return;
	led->mode = 0x01;
	AS3668_LOCK();
	current_brightness = AS3668_READ_REG(led->reg);
	as3668_switch_led(data, led, led->mode);
	for (brightness = current_brightness;
			brightness >= led->fade_min_brightness; brightness--) {
		msleep(10);
		if (brightness == 0) {
			led->mode = 0x00;
			as3668_switch_led(data, led, led->mode);
		}
		AS3668_WRITE_REG(led->reg, brightness);
	}

	AS3668_UNLOCK();
}

#endif /* CONFIG_LEDS_AS3668_EXTENSION */
static int as3668_set_led_blink(struct led_classdev *led_cdev,
		unsigned long *delay_on,
		unsigned long *delay_off) {
	struct device *dev = led_cdev->dev->parent;
	struct as3668_data *data = dev_get_drvdata(dev);
	struct as3668_led *led = ldev_to_led(led_cdev);

	if (as3668_pattern_in_use(data)) {
		dev_info(dev, "pattern generator already in use\n");
		return -EBUSY;
	} else {
		if (*delay_on == 0 || *delay_off == 0)
			return -EINVAL;
		led->ldev.blink_delay_on = *delay_on;
		led->ldev.blink_delay_off = *delay_off;
	}
	schedule_work(&led->blink_work);
	return 0;
}

static void as3668_set_blink_work(struct work_struct *work)
{
	struct as3668_led *led = container_of(work, 
			struct as3668_led, blink_work);
	struct i2c_client *client = led->client;
	struct device *dev = &client->dev;
	struct as3668_data *data = dev_get_drvdata(dev);

	if (!as3668_pattern_in_use(data)) {
		as3668_set_pattern_ton(data, led->ldev.blink_delay_on);
		as3668_set_pattern_toff(data, led->ldev.blink_delay_off);
	}
	led->ldev.blink_delay_on = pattern_ton_time[data->pattern_ton];
	led->ldev.blink_delay_off = pattern_toff_time[data->pattern_toff];
	AS3668_WRITE_REG(led->reg, led->ldev.blink_brightness);
	AS3668_LOCK();
	as3668_switch_led(data, led, 0x03);
	AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x06, 0x06);
	AS3668_UNLOCK();
}

static ssize_t as3668_pwm_dim_brightness_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->pwm_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pwm_dim_brightness_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int pwm_brightness;

	i = sscanf(buf, "%d", &pwm_brightness);
	if (i != 1)
		return -EINVAL;
	if ((pwm_brightness > (led->pled->max_current_uA/100)) ||
			(pwm_brightness < 0))
		return -EINVAL;
	led->pwm_brightness = pwm_brightness;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->mode);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_mode_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	int i;
	int mode;

	i = sscanf(buf, "%d", &mode);
	if (i != 1)
		return -EINVAL;
	if (mode > 3 || mode < 0)
		return -EINVAL;
	led->mode = mode;
	as3668_switch_led(data, led, mode);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_low_voltage_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	u8 low_v_stat;

	low_v_stat = i2c_smbus_read_byte_data(data->client,
			AS3668_REG_CurrX_low_voltage_status);
	snprintf(buf, PAGE_SIZE, "%d\n",
			low_v_stat & (0x01 << led->low_v_status_shift));
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_low_voltage_status_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_frame_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->pattern_frame_mask);
	return strnlen(buf, PAGE_SIZE);
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_pattern_frame_mask_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int frame_mask;

	i = sscanf(buf, "%d", &frame_mask);
	if (i != 1)
		return -EINVAL;
	if (frame_mask > 3 || frame_mask < 0)
		return -EINVAL;
	led->pattern_frame_mask = frame_mask;
	return strnlen(buf, PAGE_SIZE);
}
#else /* CONFIG_LEDS_AS3668_EXTENSION */
static ssize_t as3668_pattern_frame_mask_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	int i;
	int frame_mask;

	i = sscanf(buf, "%d", &frame_mask);
	if (i != 1)
		return -EINVAL;
	if (frame_mask > 3 || frame_mask < 0)
		return -EINVAL;
	led->pattern_frame_mask = frame_mask;
	AS3668_MODIFY_REG(AS3668_REG_Frame_Mask,
		0x03 << led->pattern_frame_mask_shift,
		frame_mask << led->pattern_frame_mask_shift);
	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static ssize_t as3668_pattern_frame_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->pattern_frame_delay);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_frame_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	int i;
	int frame_delay;

	i = sscanf(buf, "%d", &frame_delay);
	if (i != 1)
		return -EINVAL;
	if (frame_delay > 3 || frame_delay < 0)
		return -EINVAL;
	led->pattern_frame_delay = frame_delay;
	AS3668_MODIFY_REG(AS3668_REG_Pattern_Frame_Start_Delay,
		0x03 << led->pattern_frame_delay_shift,
		frame_delay << led->pattern_frame_delay_shift);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_pattern_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->pattern_brightness);
	return strnlen(buf, PAGE_SIZE);
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static ssize_t as3668_pattern_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int pattern_brightness;

	i = sscanf(buf, "%d", &pattern_brightness);
	if (i != 1)
		return -EINVAL;

	if (pattern_brightness < LED_OFF) {
		dev_err(dev, "Invalid pattern_brightness value\n");
		return -EINVAL;
	}

	if (pattern_brightness > (led->pled->max_current_uA / 100))
		pattern_brightness = (led->pled->max_current_uA / 100);

	led->pattern_brightness = pattern_brightness;

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_max_mix_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->max_mix_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_max_mix_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int max_mix_brightness;

	i = sscanf(buf, "%d", &max_mix_brightness);
	if (i != 1)
		return -EINVAL;
	led->max_mix_brightness = max_mix_brightness;
	if (led->max_mix_brightness > led->ldev.max_brightness) {
		led->ldev.max_brightness = led->max_mix_brightness;
		led->pled->max_current_uA = led->max_mix_brightness * 100;
	}
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_max_single_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->max_single_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_max_single_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int max_single_brightness;

	i = sscanf(buf, "%d", &max_single_brightness);
	if (i != 1)
		return -EINVAL;
	led->max_single_brightness = max_single_brightness;
	if (led->max_single_brightness > led->ldev.max_brightness) {
		led->ldev.max_brightness = led->max_single_brightness;
		led->pled->max_current_uA = led->max_single_brightness * 100;
	}
	return strnlen(buf, PAGE_SIZE);
}

#else /* CONFIG_LEDS_AS3668_EXTENSION */
static ssize_t as3668_pattern_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int pattern_brightness;

	i = sscanf(buf, "%d", &pattern_brightness);
	if (i != 1)
		return -EINVAL;
	if ((pattern_brightness > (led->pled->max_current_uA / 100)) || (pattern_brightness < 0))
		return -EINVAL;
	led->pattern_brightness = pattern_brightness;
	return strnlen(buf, PAGE_SIZE);
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static ssize_t as3668_audio_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->audio_enable);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	int i;
	int mode;

	i = sscanf(buf, "%d", &mode);
	if (i != 1)
		return -EINVAL;
	if (mode > 1 || mode < 0)
		return -EINVAL;
	AS3668_LOCK();
	led->audio_enable = mode;
	as3668_switch_led(data, led, mode);
	AS3668_MODIFY_REG(led->aud_reg, (1 << led->aud_shift),
			(mode << led->aud_shift));
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->audio_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_audio_brightness_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	int value;

	i = sscanf(buf, "%d", &value);
	if (i != 1)
		return -EINVAL;
	if ((value > (led->pled->max_current_uA/100)) || (value < 0))
		return -EINVAL;
	led->audio_brightness = value;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_breath_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3668_data *data = dev_get_drvdata(dev->parent);
	int enable, ret = 0;

	ret = sscanf(buf, "%d", &enable);

	if (enable > 0) {
		led->mode = MODE_BLINK_PATTERN;
		led->pattern_brightness = (led->pled->max_current_uA / 100);
		led->ldev.brightness = LED_FULL;
	} else {
		led->mode = MODE_OFF;
		led->pattern_brightness = 0;
		led->ldev.brightness = LED_OFF;
	}

	AS3668_LOCK();
	as3668_switch_led(data, led, led->mode);
	AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x06, 0x06);
	AS3668_WRITE_REG(led->reg, led->ldev.brightness);
	AS3668_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3668_breath_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct as3668_led *led = ldev_to_led(dev_get_drvdata(dev));
	int blinking = 1;

	if (led->mode == MODE_OFF)
		blinking = 0;

	snprintf(buf, PAGE_SIZE, "%d\n", blinking);
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute as3668_attributes[] = {
	AS3668_ATTR(debug),
	AS3668_ATTR(pattern_pwm_dim_speed_down_ms),
	AS3668_ATTR(pattern_pwm_dim_speed_up_ms),
	AS3668_ATTR(pattern_time_on_ms),
	AS3668_ATTR(pattern_time_off_ms),
	AS3668_ATTR(pattern_source_mask),
	AS3668_ATTR(pattern_multiple_pulse_number),
	AS3668_ATTR(pattern_tp_led_ms),
	AS3668_ATTR(pattern_fade_out),
	AS3668_ATTR(pattern_run),
	AS3668_ATTR(pwm_dim_mask),
	AS3668_ATTR(pwm_dim_shape),
	AS3668_ATTR(pwm_dim_direction),
	AS3668_ATTR(pwm_dim_start),
	AS3668_ATTR(cp_clock_khz),
	AS3668_ATTR(cp_on),
	AS3668_ATTR(cp_mode),
	AS3668_ATTR(cp_mode_switching),
	AS3668_ATTR(cp_auto_on),
	AS3668_ATTR(gpio_input_state),
	AS3668_ATTR(gpio_output_state),
	AS3668_ATTR(gpio_toggle_enable),
	AS3668_ATTR(gpio_toggle_frame_nr),
	AS3668_ATTR(audio_agc_on),
	AS3668_ATTR(audio_agc_decay_up_ms),
	AS3668_ATTR(audio_agc_decay_down_ms),
	AS3668_ATTR(audio_agc_up_level_inc),
	AS3668_ATTR(audio_agc_down_level_inc),
	AS3668_ATTR(audio_decay_ms),
	AS3668_ATTR(audio_sync_mode),
	AS3668_ATTR(audio_buffer_on),
	AS3668_ATTR(audio_gain_dB),
	AS3668_ATTR(audio_adc_on),
	AS3668_ATTR(audio_adc_mode),
	AS3668_ATTR(audio_src_mask),
	AS3668_ATTR(audio_on),
#if (AS3668_USE_PREDEFINED_PATTERNS == 1)
	AS3668_ATTR(pattern_predef_run_num),
#endif
	__ATTR_NULL
};

static struct device_attribute as3668_led_attributes[] = {
	AS3668_ATTR(pwm_dim_brightness),
	AS3668_ATTR(mode),
	AS3668_ATTR(low_voltage_status),
	AS3668_ATTR(pattern_frame_mask),
	AS3668_ATTR(pattern_frame_delay),
	AS3668_ATTR(pattern_brightness),
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	AS3668_ATTR(max_mix_brightness),
	AS3668_ATTR(max_single_brightness),
	AS3668_ATTR(fade_brightness),
	AS3668_ATTR(fade_max_brightness),
	AS3668_ATTR(fade_min_brightness),
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	AS3668_ATTR(audio_enable),
	AS3668_ATTR(audio_brightness),
	AS3668_ATTR(breath),
	__ATTR_NULL
};

/** init and configuration **************************************************/
static int as3668_configure(struct i2c_client *client,
		struct as3668_data *data, struct as3668_platform_data *pdata)
{
	int err = 0;
	int i;
	u8 reg_data;

	data->pdata = pdata;
	data->num_leds = AS3668_NUM_LEDS;

	AS3668_MODIFY_REG(AS3668_REG_Start_Ctrl, 0x01,
			(data->pdata->pattern_start_source & 0x01));
	AS3668_MODIFY_REG(AS3668_REG_Pwm_Ctrl, 0x01,
			(data->pdata->pwm_source & 0x01));
	AS3668_MODIFY_REG(AS3668_REG_Gpio_Ctrl, 0x07,
		((data->pdata->gpio_input_invert << 2)
		| (data->pdata->gpio_input_mode << 1)
		| (data->pdata->gpio_mode)));
	AS3668_MODIFY_REG(AS3668_REG_Audio_Ctrl, 0x40,
			(data->pdata->audio_pulldown_off << 6));
	AS3668_MODIFY_REG(AS3668_REG_Audio_Ctrl, 0x80,
			(data->pdata->audio_input_pin << 7));
	AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x40,
			(data->pdata->audio_man_start << 6));
	AS3668_MODIFY_REG(AS3668_REG_Audio_Input_Buffer, 0x80,
			(data->pdata->audio_dis_start << 7));
	AS3668_MODIFY_REG(AS3668_REG_Audio_Ctrl, 0x20,
			(data->pdata->audio_adc_characteristic << 5));

	AS3668_MODIFY_REG(AS3668_REG_CP_Ctrl, 0x40, data->cp_auto_on << 6);
	reg_data = AS3668_READ_REG(AS3668_REG_CP_Ctrl);
	dev_info(&client->dev, "cp_auto_on = 0x%02X\n", reg_data);

	AS3668_MODIFY_REG(AS3668_REG_Reg_Ctrl, 0x04, data->cp_on << 2);
	reg_data = AS3668_READ_REG(AS3668_REG_Reg_Ctrl);
	dev_info(&client->dev, "cp_on = 0x%02X\n", reg_data);

	if (data->pdata->vbat_monitor_voltage_mV < AS3668_VMON_MIN_VOLTAGE) {
		dev_warn(&client->dev,
				"vbat_monitor_voltage_mV of %d lower than"
				" possible, increasing to %d\n",
				data->pdata->vbat_monitor_voltage_mV,
				AS3668_VMON_MIN_VOLTAGE);
		data->pdata->vbat_monitor_voltage_mV  = AS3668_VMON_MIN_VOLTAGE;
	}

	as3668_set_vbat_monitor_voltage(data);

	for (i = 0; i < data->num_leds; i++) {
		struct as3668_led *led = &data->leds[i];
		struct as3668_platform_led *pled = &pdata->leds[i];

		led->pled = pled;
		if (pled->name)
			led->name = pled->name;
		if (led->pled->max_current_uA > AS3668_MAX_OUTPUT_CURRENT)
			led->pled->max_current_uA = AS3668_MAX_OUTPUT_CURRENT;
		led->pled->max_current_uA =
			led->pled->max_current_uA / 100 * 100;
		led->client = client;
		led->ldev.name = led->name;
		led->ldev.max_brightness = led->pled->max_current_uA / 100;
		led->ldev.brightness = LED_OFF;
		led->ldev.brightness_set = as3668_set_led_brightness;
		led->ldev.blink_brightness = LED_HALF;
		led->ldev.blink_set = as3668_set_led_blink;
		led->ldev.blink_delay_on = 0;
		led->ldev.blink_delay_off = 0;
		led->pwm_brightness = LED_OFF;
		INIT_WORK(&led->brightness_work, as3668_set_brightness_work);
#ifdef CONFIG_LEDS_AS3668_EXTENSION
		INIT_WORK(&led->fade_in_work, as3668_set_fade_in_work);
		INIT_DELAYED_WORK(&led->delayed_fade_out_work,
				as3668_set_delayed_fade_out_work);
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
		INIT_WORK(&led->blink_work, as3668_set_blink_work);
		err = led_classdev_register(&client->dev, &led->ldev);
		if (err < 0) {
			dev_err(&client->dev,
					"couldn't register LED %s\n",
					led->name);
			goto exit;
		}
		err = device_add_attributes(led->ldev.dev,
				as3668_led_attributes);
		if (err < 0) {
			dev_err(&client->dev,
					"couldn't add attributes %s\n",
					led->name);
			goto exit_ledreg;
		}
	}

	INIT_DELAYED_WORK(&data->pwm_finished_work, as3668_pwm_finished_work);

	err = device_add_attributes(&client->dev, as3668_attributes);

	if (err)
		goto exit_ledreg;

	return 0;

exit_ledreg:
	for (i = 0; i < AS3668_NUM_LEDS; i++)
		led_classdev_unregister(&data->leds[i].ldev);
exit:
	return err;
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static int as3668_parse_dt(struct device *dev,
		struct as3668_data *data,
		struct as3668_platform_data *pdata)
{
	struct device_node *node, *temp = NULL;
	int rc = 0, num_leds = 0, parsed_leds = 0, i, current_index;
	int ext_rc = 0;
	struct as3668_platform_led *pled;
	struct as3668_led *led = NULL;
	const char *led_label;
	u32 *rgb_current;
	u32 tmp_data, color_variation_max_num;

	node = dev->of_node;
	while ((temp = of_get_next_child(node, temp)))
		num_leds++;
	if (!num_leds) {
		dev_err(dev, "Unable to get rgb parameter\n");
		rc = -ECHILD;
		goto exit;
	}
	rc = of_property_read_u32(dev->of_node, "somc,led_num", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read num_leds\n");
		data->num_leds = AS3668_NUM_LEDS;
	} else {
		data->num_leds = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_pwm_dim_speed_down_ms", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_pwm_dim_speed_down_ms\n");
		data->pwm_dim_speed_down = 0;
	} else {
		dev_err(dev, "pwm_dim_speed_down = %d\n", tmp_data);
		data->pwm_dim_speed_down = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_pwm_dim_speed_up_ms", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_pwm_dim_speed_up_ms\n");
		data->pwm_dim_speed_up = 0;
	} else {
		dev_err(dev, "pwm_dim_speed_up = %d\n", tmp_data);
		data->pwm_dim_speed_up = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_source_mask", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_source_mask\n");
		data->pattern_source_mask = AS3668_DEFAULT_SRC_MASK;
	} else {
		dev_err(dev, "pattern_source_mask = %d\n", tmp_data);
		data->pattern_source_mask = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_multiple_pulse", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_multiple_pulse\n");
		data->pattern_multiple_pulse = 1;
	} else {
		data->pattern_multiple_pulse = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_tp_led", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_tp_led\n");
		data->pattern_tp_led = 0;
	} else {
		data->pattern_tp_led = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,pattern_fade_out", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_fade_out\n");
		data->pattern_fade_out = 0;
	} else {
		data->pattern_fade_out = tmp_data;
	}

	rc = of_property_read_u32(dev->of_node,
			"somc,color_variation_max_num", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_fade_out\n");
		color_variation_max_num = AS3668_CURRENT_MAX_PATTERN;
	} else {
		color_variation_max_num = tmp_data;
	}
	dev_info(dev, "color_variation_max_num[%d] rgb_current_index[%d]\n",
		color_variation_max_num, rgb_current_index);

	rc = of_property_read_u32(node,
			"somc,pattern_start_source", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pattern_start_source\n");
		pdata->pattern_start_source = AS3668_PATTERN_START_SOURCE_SW;
	} else {
		pdata->pattern_start_source = tmp_data;
	}
	rc = of_property_read_u32(node,
			"somc,pwm_source", &tmp_data);
	if (rc < 0) {
		dev_err(dev, "Unable to read pwm_source\n");
		pdata->pwm_source = AS3668_PWM_SOURCE_INTERNAL;
	} else {
		pdata->pwm_source = tmp_data;
	}

	rgb_current = devm_kzalloc(dev,
		sizeof(u32) * color_variation_max_num * (AS3668_NUM_LEDS * 2 + 1), GFP_KERNEL);
	if (!rgb_current) {
		dev_err(dev, "Failed to alloc rgb_current\n");
		led->max_mix_brightness = AS3668_MAX_OUTPUT_CURRENT / 100;
		led->max_single_brightness = AS3668_MAX_OUTPUT_CURRENT / 100;
	} else {
		rc = of_property_read_u32_array(node, "somc,max_current_uA",
				rgb_current, color_variation_max_num * (AS3668_NUM_LEDS * 2 + 1));
		if (rc < 0) {
			dev_err(dev, "Unable to read max_mix_current_uA\n");
			for (i = 0; i < color_variation_max_num * (AS3668_NUM_LEDS * 2 + 1); i++) {
				current_index = i % (AS3668_NUM_LEDS * 2 + 1);
				if (!current_index)
					rgb_current[i] =
						i / (AS3668_NUM_LEDS * 2 + 1);
				else
					rgb_current[i] = AS3668_MAX_OUTPUT_CURRENT;
			}
		}
	}

	for_each_child_of_node(node, temp) {
		pled = &pdata->leds[parsed_leds];
		led = &data->leds[parsed_leds];
		rc = of_property_read_string(temp, "label", &led_label);
		if (rc < 0) {
			dev_err(dev, "Unable to read label, rc = %d\n", rc);
			goto exit;
		}
		if (strncmp(led_label, "rgb", sizeof("rgb")) == 0) {
			rc = of_property_read_string(temp,
					"linux,name", &pled->name);
			if (rc < 0) {
				dev_err(dev, "Unable to read linux,name\n");
				goto exit;
			}
			current_index = (color_variation_max_num - 1) * (AS3668_NUM_LEDS * 2 + 1);
			if (rgb_current_index < rgb_current[0]
				|| rgb_current_index > rgb_current[current_index]) {
				current_index = (color_variation_max_num - 1) * (AS3668_NUM_LEDS * 2 + 1) + 1;
				led->max_single_brightness = rgb_current[current_index + parsed_leds * 2] / 100;
				led->max_mix_brightness = rgb_current[current_index + parsed_leds * 2 + 1] / 100;
			} else {
				for (i = 0; i < color_variation_max_num; i++) {
					current_index = i * (AS3668_NUM_LEDS * 2 + 1);
					if (rgb_current_index == rgb_current[current_index]) {
						break;
					}
				}
				led->max_single_brightness = rgb_current[current_index + 1 + parsed_leds * 2] / 100;
				led->max_mix_brightness = rgb_current[current_index + 1 + parsed_leds * 2 + 1] / 100;
			}

			if (led->max_single_brightness > led->max_mix_brightness)
				pled->max_current_uA = led->max_single_brightness * 100;
			else
				pled->max_current_uA = led->max_mix_brightness * 100;
			dev_info(dev, "%s max_single_brightness[%d] max_mix_brightness[%d]\n",
				(parsed_leds < 1 ? "blue" : (parsed_leds < 2 ? "red" : "green")),
				led->max_single_brightness, led->max_mix_brightness);

			rc = of_property_read_u32(temp,
				"somc,pattern_frame_mask", &tmp_data);
			if (rc < 0) {
				dev_err(dev,
					"Unable to read pattern_frame_mask %d\n",
					tmp_data);
				led->pattern_frame_mask = 0;
			} else {
				led->pattern_frame_mask = tmp_data;
			}
			rc = of_property_read_u32(temp,
				"somc,pattern_frame_delay", &tmp_data);
			if (rc < 0) {
				dev_err(dev,
					"Unable to read pattern_frame_delay %d\n",
					tmp_data);
				led->pattern_frame_delay = 0;
			} else {
				led->pattern_frame_delay = tmp_data;
			}
			ext_rc = of_property_read_u32(temp,
				"somc,fade_max_brightness", &tmp_data);
			if (ext_rc < 0) {
				dev_err(dev,
					"Unable to read fade_max_brightness %d\n",
					tmp_data);
				led->fade_max_brightness = 0;
			} else {
				led->fade_max_brightness = tmp_data;
			}
			ext_rc = of_property_read_u32(temp,
				"somc,fade_min_brightness", &tmp_data);
			if (ext_rc < 0) {
				dev_err(dev,
					"Unable to read fade_min_brightness %d\n",
					tmp_data);
				led->fade_min_brightness = 0;
			} else {
				led->fade_min_brightness = tmp_data;
			}
			ext_rc = of_property_read_u32(temp,
				"somc,fade_led_off_delay", &tmp_data);
			if (ext_rc < 0) {
				dev_err(dev,
					"Unable to read led_off_delay %d\n",
					tmp_data);
				led->led_off_delay = 0;
			} else {
				led->led_off_delay = tmp_data;
			}
		} else {
			dev_err(dev, "No LED matching label\n");
			rc = -EINVAL;
			goto exit;
		}
		parsed_leds++;
	}

exit:
	return rc;
}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static int as3668_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct as3668_data *data;
#ifdef CONFIG_LEDS_AS3668_EXTENSION
	struct as3668_platform_data *as3668_pdata = &as3668_ext_data;
#else /* CONFIG_LEDS_AS3668_EXTENSION */
	struct as3668_platform_data *as3668_pdata = client->dev.platform_data;
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	int id1, id2, i;
	int err = 0;

	if (!as3668_pdata)
		return -EIO;

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	data = kzalloc(sizeof(*data), GFP_USER);
	if (!data)
		return -ENOMEM;

	/* initialize with meaningful data (register names, etc.) */
	*data = as3668_default_data;

#ifdef CONFIG_LEDS_AS3668_EXTENSION
	err = as3668_parse_dt(&client->dev, data, as3668_pdata);
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to parse pdata\n", __func__);
		goto exit;
	}
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

	dev_set_drvdata(&client->dev, data);

	data->client = client;
	mutex_init(&data->update_lock);

	id1 = i2c_smbus_read_byte_data(client, AS3668_REG_ChipID1);
	if (id1 < 0) {
		err = id1;
		goto exit;
	}
	if (id1 != 0xA5) {
		err = -ENXIO;
		dev_err(&client->dev, "wrong chip detected, ids %x", id1);
		goto exit;
	}
	id2 = i2c_smbus_read_byte_data(client, AS3668_REG_ChipID2);
	dev_info(&client->dev, "AS3668 driver v1.0.0: detected AS3668 "
			"chip with rev. %dV0\n", ((id2 & 0x01) + 1));
	data->client = client;

	for (i = 0; i < ARRAY_SIZE(data->regs); i++) {
		if (data->regs[i].name)
			i2c_smbus_write_byte_data(client,
					i, data->regs[i].value);
	}

	for (i = 0; i < ARRAY_SIZE(data->regs); i++) {
		if (!data->regs[i].name)
			continue;
		data->regs[i].value = i2c_smbus_read_byte_data(client, i);
	}

	i2c_set_clientdata(client, data);

	err = as3668_configure(client, data, as3668_pdata);

exit:
	if (err) {
		dev_err(&client->dev, "could not configure %x", err);
		flush_scheduled_work();
		kfree(data);
		i2c_set_clientdata(client, NULL);
	}

	return err;
}

static int as3668_remove(struct i2c_client *client)
{
	struct as3668_data *data = i2c_get_clientdata(client);
	int i;

	dev_info(&data->client->dev, "remove as3668\n");

	for (i = 0; i < AS3668_NUM_LEDS; i++)
		led_classdev_unregister(&data->leds[i].ldev);
	device_remove_attributes(&client->dev, as3668_attributes);
	flush_scheduled_work();
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

#ifdef CONFIG_LEDS_AS3668_EXTENSION
static const struct of_device_id as3668_dt_ids[] = {
	{ .compatible = "as3668", },
	{ }
};
#endif /* CONFIG_LEDS_AS3668_EXTENSION */

static const struct i2c_device_id as3668_id[] = {
	{ "as3668", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, as3668_id);

static struct i2c_driver as3668_driver = {
	.driver = {
#ifdef CONFIG_LEDS_AS3668_EXTENSION
		.owner	= THIS_MODULE,
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
		.name   = "as3668",
#ifdef CONFIG_LEDS_AS3668_EXTENSION
		.of_match_table = of_match_ptr(as3668_dt_ids),
#endif /* CONFIG_LEDS_AS3668_EXTENSION */
	},
	.probe  = as3668_probe,
	.remove = as3668_remove,
#ifdef CONFIG_PM
	.shutdown = as3668_shutdown,
#endif
	.id_table = as3668_id,
};

static int __init as3668_init(void)
{
	return i2c_add_driver(&as3668_driver);
}

static void __exit as3668_exit(void)
{
	i2c_del_driver(&as3668_driver);
}


MODULE_AUTHOR("Florian Lobmaier <florian.lobmaier@austriamicrosystems.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AS3668 LED light");

module_init(as3668_init);
module_exit(as3668_exit);

