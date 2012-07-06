/*
 * as3676.c - Led dimmer
 *
 * Version:
 * 2012-02-21: v1.6 : - small bug fix for DCDC handling
 * 2012-02-03: v1.5 : - avoid current spikes when turning on DCDC
 *                    - forgot to remove attributes on remove
 * 2012-01-12: v1.4 : - fixes for linux v3.x and minor code review issues
 *                    - fixing incorrect configuration of GPIO1/light pin
 * 2011-12-23: v1.3 : - group check is sometimes too strict, disable it
 * 2011-12-15: v1.2 : - fixed pattern starting/stopping
 *                    - added startup brightness
 *                    - fixed leds turning off
 * 2011-09-07: v1.1 : - adding some review comments
 *                    - fixing deadlock in suspend function existing since v1.0
 * 2010-12-15: v1.0 : - adding power management
 *                    - fixed ALS (potential oops, falling slopes not working)
 *                    - added locking
 * 2010-10-13: v0.9 : second milestone: all practical features,
 *                                      suspend/resume is pending
 * 2010-09-30: v0.8 : first milestone: all existing features from as3677
 *
 * Copyright (C) 2011 Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/leds-as3676.h>

/* Current group check function is too strict in some cases.
   Setting this define to 0 disables them */
#define AS3676_ENABLE_GROUP_CHECK 0

#define AS3676_NUM_LEDS  13

#define AS3676_REG_Control                  0x00
#define AS3676_REG_curr12_control           0x01
#define AS3676_REG_curr_rgb_control         0x02
#define AS3676_REG_curr3_control            0x03
#define AS3676_REG_curr4_control            0x04
#define AS3676_REG_GPIO_output_1            0x05
#define AS3676_REG_GPIO_signal_1            0x06
#define AS3676_REG_LDO_Voltage              0x07
#define AS3676_REG_Curr1_current            0x09
#define AS3676_REG_Curr2_current            0x0A
#define AS3676_REG_Rgb1_current             0x0B
#define AS3676_REG_Rgb2_current             0x0C
#define AS3676_REG_Rgb3_current             0x0D
#define AS3676_REG_Curr3x_strobe            0x0E
#define AS3676_REG_Curr3x_preview           0x0F
#define AS3676_REG_Curr3x_other             0x10
#define AS3676_REG_Curr3x_strobe_control    0x11
#define AS3676_REG_Curr3x_control           0x12
#define AS3676_REG_Curr41_current           0x13
#define AS3676_REG_Curr42_current           0x14
#define AS3676_REG_Curr43_current           0x15
#define AS3676_REG_Pwm_control              0x16
#define AS3676_REG_Pwm_code                 0x17
#define AS3676_REG_Pattern_control          0x18
#define AS3676_REG_Pattern_data0            0x19
#define AS3676_REG_Pattern_data1            0x1A
#define AS3676_REG_Pattern_data2            0x1B
#define AS3676_REG_Pattern_data3            0x1C
#define AS3676_REG_GPIO_control             0x1E
#define AS3676_REG_GPIO_driving_cap         0x20
#define AS3676_REG_DCDC_control1            0x21
#define AS3676_REG_DCDC_control2            0x22
#define AS3676_REG_CP_control               0x23
#define AS3676_REG_CP_mode_Switch1          0x24
#define AS3676_REG_CP_mode_Switch2          0x25
#define AS3676_REG_ADC_control              0x26
#define AS3676_REG_ADC_MSB_result           0x27
#define AS3676_REG_ADC_LSB_result           0x28
#define AS3676_REG_Overtemp_control         0x29
#define AS3676_REG_Curr_low_voltage_status1 0x2A
#define AS3676_REG_Curr_low_voltage_status2 0x2B
#define AS3676_REG_Gpio_current             0x2C
#define AS3676_REG_Curr6_current            0x2F
#define AS3676_REG_Adder_Current_1          0x30
#define AS3676_REG_Adder_Current_2          0x31
#define AS3676_REG_Adder_Current_3          0x32
#define AS3676_REG_Adder_Enable_1           0x33
#define AS3676_REG_Adder_Enable_2           0x34
#define AS3676_REG_Subtract_Enable          0x35
#define AS3676_REG_ASIC_ID1                 0x3E
#define AS3676_REG_ASIC_ID2                 0x3F
#define AS3676_REG_Curr30_current           0x40
#define AS3676_REG_Curr31_current           0x41
#define AS3676_REG_Curr32_current           0x42
#define AS3676_REG_Curr33_current           0x43
#define AS3676_REG_Audio_Control            0x46
#define AS3676_REG_Audio_Input              0x47
#define AS3676_REG_Audio_Output             0x48
#define AS3676_REG_GPIO_output_2            0x50
#define AS3676_REG_GPIO_signal_2            0x51
#define AS3676_REG_Adder_Current_4          0x52
#define AS3676_REG_CURR3x_audio_source      0x53
#define AS3676_REG_Pattern_End              0x54
#define AS3676_REG_Audio_Control_2          0x55
#define AS3676_REG_DLS_mode_control1        0x56
#define AS3676_REG_DLS_mode_control2        0x57
#define AS3676_REG_ALS_control              0x90
#define AS3676_REG_ALS_filter               0x91
#define AS3676_REG_ALS_offset               0x92
#define AS3676_REG_ALS_result               0x93
#define AS3676_REG_ALS_curr12_group         0x94
#define AS3676_REG_ALS_rgb_group            0x95
#define AS3676_REG_ALS_curr3x_group         0x96
#define AS3676_REG_ALS_curr4x_group         0x97
#define AS3676_REG_ALS_group_1_Y0           0x98
#define AS3676_REG_ALS_group_1_Y3           0x99
#define AS3676_REG_ALS_group_1_X1           0x9A
#define AS3676_REG_ALS_group_1_K1           0x9B
#define AS3676_REG_ALS_group_1_X2           0x9C
#define AS3676_REG_ALS_group_1_K2           0x9D
#define AS3676_REG_ALS_group_2_Y0           0x9E
#define AS3676_REG_ALS_group_2_Y3           0x9F
#define AS3676_REG_ALS_group_2_X1           0xA0
#define AS3676_REG_ALS_group_2_K1           0xA1
#define AS3676_REG_ALS_group_2_X2           0xA2
#define AS3676_REG_ALS_group_2_K2           0xA3
#define AS3676_REG_ALS_group_3_Y0           0xA4
#define AS3676_REG_ALS_group_3_Y3           0xA5
#define AS3676_REG_ALS_group_3_X1           0xA6
#define AS3676_REG_ALS_group_3_K1           0xA7
#define AS3676_REG_ALS_group_3_X2           0xA8
#define AS3676_REG_ALS_group_3_K2           0xA9

#define ldev_to_led(c)       container_of(c, struct as3676_led, ldev)

#define AS3676_WRITE_REG(a, b) as3676_write_reg(data, (a), (b))
#define AS3676_READ_REG(a) as3676_read_reg(data, (a))
#define AS3676_MODIFY_REG(a, b, c) as3676_modify_reg(data, (a), (b), (c))
#define AS3676_WRITE_PATTERN(a) do { \
	u32 __d = a; \
	AS3676_WRITE_REG(AS3676_REG_Pattern_data0, __d & 0xff); __d >>= 8; \
	AS3676_WRITE_REG(AS3676_REG_Pattern_data1, __d & 0xff); __d >>= 8; \
	AS3676_WRITE_REG(AS3676_REG_Pattern_data2, __d & 0xff); __d >>= 8; \
	AS3676_WRITE_REG(AS3676_REG_Pattern_data3, __d & 0xff); \
	} while (0)

#define AS3676_LOCK()   mutex_lock(&(data)->update_lock)
#define AS3676_UNLOCK() mutex_unlock(&(data)->update_lock)

struct as3676_reg {
	const char *name;
	u8 value;
};

struct as3676_led {
	u8 reg;
	u8 mode_reg; /* fixed offset 0x93 to amb_mode */
	u8 mode_shift; /* same offsets for amb_mode */
	u8 has_extended_mode;
	u8 adder_reg;
	u8 adder_on_reg;
	u8 adder_on_shift;
	u8 dls_mode_reg; /* fixed offset -0x32 to cp_mode reg */
	u8 dls_mode_shift; /* same for cp_mode_shift */
	u8 aud_reg;
	u8 aud_shift;
	struct device_attribute *aud_file;
	int dim_brightness;
	int dim_value;
	struct i2c_client *client;
	const char *name;
	struct led_classdev ldev;
	struct as3676_platform_led *pled;
	bool use_pattern;
	u8 marker;
};

struct as3676_data {
	struct i2c_client *client;
	struct as3676_led leds[AS3676_NUM_LEDS];
	struct mutex update_lock;
	struct delayed_work dim_work;
	struct as3676_reg regs[255];
	bool dimming_in_progress;
	int pattern_running;
	u32 pattern_data;
	int num_leds;
	int ldo_count;
	u8 als_control_backup;
	u8 als_result_backup;
};

struct as3676_als_group {
	u8 gn;
	u8 y0;
	u8 x1;
	s8 k1;
	u8 x2;
	s8 k2;
	u8 y3;
};

#define AS3676_ATTR(_name)  \
	__ATTR(_name, 0644, as3676_##_name##_show, as3676_##_name##_store)

#define AS3676_DEV_ATTR(_name)  \
	struct device_attribute as3676_##_name = AS3676_ATTR(_name);

static u8 as3676_read_reg(struct as3676_data *data, u8 reg);
static s32 as3676_write_reg(struct as3676_data *data, u8 reg, u8 value);
static s32 as3676_modify_reg(struct as3676_data *data, u8 reg, u8 reset,
		u8 set);
static void as3676_set_brightness(struct as3676_data *data,
		struct as3676_led *led, enum led_brightness value);
static void as3676_switch_als(struct as3676_data *data, int als_on);

static ssize_t as3676_curr4x_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr4x_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_rgbx_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_rgbx_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_curr126_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr126_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_curr30_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr30_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_curr31_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr31_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_curr32_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr32_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);
static ssize_t as3676_curr33_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t as3676_curr33_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);

AS3676_DEV_ATTR(curr4x_audio_on);
AS3676_DEV_ATTR(rgbx_audio_on);
AS3676_DEV_ATTR(curr126_audio_on);
AS3676_DEV_ATTR(curr30_audio_channel);
AS3676_DEV_ATTR(curr31_audio_channel);
AS3676_DEV_ATTR(curr32_audio_channel);
AS3676_DEV_ATTR(curr33_audio_channel);

#define AS3676_REG(NAME, VAL)[AS3676_REG_##NAME] = \
	{.name = __stringify(NAME), .value = (VAL)}

static const struct as3676_data as3676_default_data = {
	.client = NULL,
	.num_leds = AS3676_NUM_LEDS,
	.leds = {{
			.name = "as3676::curr1",
			.reg = AS3676_REG_Curr1_current,
			.mode_reg = AS3676_REG_curr12_control,
			.mode_shift = 0,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_1,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 0,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 0,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 4,
			.aud_file = &as3676_curr126_audio_on,
		},
		{
			.name = "as3676::curr2",
			.reg = AS3676_REG_Curr2_current,
			.mode_reg = AS3676_REG_curr12_control,
			.mode_shift = 2,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_2,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 1,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 1,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 4,
			.aud_file = &as3676_curr126_audio_on,
		},
		{
			.name = "as3676::curr6",
			.reg = AS3676_REG_Curr6_current,
			.mode_reg = AS3676_REG_curr_rgb_control,
			.mode_shift = 6,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_3,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 2,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 7,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 4,
			.aud_file = &as3676_curr126_audio_on,
		},
		{
			.name = "as3676::rgb1",
			.reg = AS3676_REG_Rgb1_current,
			.mode_reg = AS3676_REG_curr_rgb_control,
			.mode_shift = 0,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_1,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 0,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 4,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 5,
			.aud_file = &as3676_rgbx_audio_on,
		},
		{
			.name = "as3676::rgb2",
			.reg = AS3676_REG_Rgb2_current,
			.mode_reg = AS3676_REG_curr_rgb_control,
			.mode_shift = 2,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_2,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 1,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 5,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 5,
			.aud_file = &as3676_rgbx_audio_on,
		},
		{
			.name = "as3676::rgb3",
			.reg = AS3676_REG_Rgb3_current,
			.mode_reg = AS3676_REG_curr_rgb_control,
			.mode_shift = 4,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_3,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 2,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 6,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 5,
			.aud_file = &as3676_rgbx_audio_on,
		},
		{
			.name = "as3676::curr41",
			.reg = AS3676_REG_Curr41_current,
			.mode_reg = AS3676_REG_curr4_control,
			.mode_shift = 0,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_1,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 3,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 2,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 6,
			.aud_file = &as3676_curr4x_audio_on,
		},
		{
			.name = "as3676::curr42",
			.reg = AS3676_REG_Curr42_current,
			.mode_reg = AS3676_REG_curr4_control,
			.mode_shift = 2,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_2,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 4,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 3,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 6,
			.aud_file = &as3676_curr4x_audio_on,
		},
		{
			.name = "as3676::curr43",
			.reg = AS3676_REG_Curr43_current,
			.mode_reg = AS3676_REG_curr4_control,
			.mode_shift = 4,
			.has_extended_mode = 0,
			.adder_reg = AS3676_REG_Adder_Current_3,
			.adder_on_reg = AS3676_REG_Adder_Enable_1,
			.adder_on_shift = 5,
			.dls_mode_reg = AS3676_REG_DLS_mode_control2,
			.dls_mode_shift = 4,
			.aud_reg = AS3676_REG_Audio_Output,
			.aud_shift = 6,
			.aud_file = &as3676_curr4x_audio_on,
		},
		{
			.name = "as3676::curr30",
			.reg = AS3676_REG_Curr30_current,
			.mode_reg = AS3676_REG_curr3_control,
			.mode_shift = 0,
			.has_extended_mode = 1,
			.adder_reg = AS3676_REG_Adder_Current_1,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 3,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 0,
			.aud_reg = AS3676_REG_CURR3x_audio_source,
			.aud_shift = 0,
			.aud_file = &as3676_curr30_audio_channel,
		},
		{
			.name = "as3676::curr31",
			.reg = AS3676_REG_Curr31_current,
			.mode_reg = AS3676_REG_curr3_control,
			.mode_shift = 2,
			.has_extended_mode = 1,
			.adder_reg = AS3676_REG_Adder_Current_2,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 4,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 1,
			.aud_reg = AS3676_REG_CURR3x_audio_source,
			.aud_shift = 2,
			.aud_file = &as3676_curr31_audio_channel,
		},
		{
			.name = "as3676::curr32",
			.reg = AS3676_REG_Curr32_current,
			.mode_reg = AS3676_REG_curr3_control,
			.mode_shift = 4,
			.has_extended_mode = 1,
			.adder_reg = AS3676_REG_Adder_Current_3,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 5,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 2,
			.aud_reg = AS3676_REG_CURR3x_audio_source,
			.aud_shift = 4,
			.aud_file = &as3676_curr32_audio_channel,
		},
		{
			.name = "as3676::curr33",
			.reg = AS3676_REG_Curr33_current,
			.mode_reg = AS3676_REG_curr3_control,
			.mode_shift = 6,
			.has_extended_mode = 1,
			.adder_reg = AS3676_REG_Adder_Current_4,
			.adder_on_reg = AS3676_REG_Adder_Enable_2,
			.adder_on_shift = 6,
			.dls_mode_reg = AS3676_REG_DLS_mode_control1,
			.dls_mode_shift = 3,
			.aud_reg = AS3676_REG_CURR3x_audio_source,
			.aud_shift = 6,
			.aud_file = &as3676_curr33_audio_channel,
		},
	},
	.regs = {
		AS3676_REG(Control                 , 0),
		AS3676_REG(curr12_control          , 0),
		AS3676_REG(curr_rgb_control        , 0),
		AS3676_REG(curr3_control           , 0),
		AS3676_REG(curr4_control           , 0),
		AS3676_REG(GPIO_output_1           , 0),
		AS3676_REG(GPIO_signal_1           , 0),
		AS3676_REG(LDO_Voltage             , 0),
		AS3676_REG(Curr1_current           , 0),
		AS3676_REG(Curr2_current           , 0),
		AS3676_REG(Rgb1_current            , 0),
		AS3676_REG(Rgb2_current            , 0),
		AS3676_REG(Rgb3_current            , 0),
		AS3676_REG(Curr3x_strobe           , 0),
		AS3676_REG(Curr3x_preview          , 0),
		AS3676_REG(Curr3x_other            , 0),
		AS3676_REG(Curr3x_strobe_control   , 0),
		AS3676_REG(Curr3x_control          , 0),
		AS3676_REG(Curr41_current          , 0),
		AS3676_REG(Curr42_current          , 0),
		AS3676_REG(Curr43_current          , 0),
		AS3676_REG(Pwm_control             , 0),
		AS3676_REG(Pwm_code                , 0),
		AS3676_REG(Pattern_control         , 0),
		AS3676_REG(Pattern_data0           , 0),
		AS3676_REG(Pattern_data1           , 0),
		AS3676_REG(Pattern_data2           , 0),
		AS3676_REG(Pattern_data3           , 0),
		AS3676_REG(GPIO_control            , 0x44),
		AS3676_REG(GPIO_driving_cap        , 0),
		AS3676_REG(DCDC_control1           , 0),
		AS3676_REG(DCDC_control2           , 0x04),
		AS3676_REG(CP_control              , 0),
		AS3676_REG(CP_mode_Switch1         , 0),
		AS3676_REG(CP_mode_Switch2         , 0),
		AS3676_REG(ADC_control             , 0x03),
		AS3676_REG(ADC_MSB_result          , 0),
		AS3676_REG(ADC_LSB_result          , 0),
		AS3676_REG(Overtemp_control        , 0x01),
		AS3676_REG(Curr_low_voltage_status1, 0),
		AS3676_REG(Curr_low_voltage_status2, 0),
		AS3676_REG(Gpio_current            , 0),
		AS3676_REG(Curr6_current           , 0),
		AS3676_REG(Adder_Current_1         , 0),
		AS3676_REG(Adder_Current_2         , 0),
		AS3676_REG(Adder_Current_3         , 0),
		AS3676_REG(Adder_Enable_1          , 0),
		AS3676_REG(Adder_Enable_2          , 0),
		AS3676_REG(Subtract_Enable         , 0),
		AS3676_REG(ASIC_ID1                , 0),
		AS3676_REG(ASIC_ID2                , 0),
		AS3676_REG(Curr30_current          , 0),
		AS3676_REG(Curr31_current          , 0),
		AS3676_REG(Curr32_current          , 0),
		AS3676_REG(Curr33_current          , 0),
		AS3676_REG(Audio_Control           , 0),
		AS3676_REG(Audio_Input             , 0),
		AS3676_REG(Audio_Output            , 0),
		AS3676_REG(GPIO_output_2           , 0),
		AS3676_REG(GPIO_signal_2           , 0),
		AS3676_REG(Adder_Current_4         , 0),
		AS3676_REG(CURR3x_audio_source     , 0),
		AS3676_REG(Pattern_End             , 0),
		AS3676_REG(Audio_Control_2         , 0),
		AS3676_REG(DLS_mode_control1       , 0),
		AS3676_REG(DLS_mode_control2       , 0),
		AS3676_REG(ALS_control             , 0),
		AS3676_REG(ALS_filter              , 0),
		AS3676_REG(ALS_offset              , 0),
		AS3676_REG(ALS_result              , 0),
		AS3676_REG(ALS_curr12_group        , 0),
		AS3676_REG(ALS_rgb_group           , 0),
		AS3676_REG(ALS_curr3x_group        , 0),
		AS3676_REG(ALS_curr4x_group        , 0),
		AS3676_REG(ALS_group_1_Y0          , 0),
		AS3676_REG(ALS_group_1_Y3          , 0),
		AS3676_REG(ALS_group_1_X1          , 0),
		AS3676_REG(ALS_group_1_K1          , 0),
		AS3676_REG(ALS_group_1_X2          , 0),
		AS3676_REG(ALS_group_1_K2          , 0),
		AS3676_REG(ALS_group_2_Y0          , 0),
		AS3676_REG(ALS_group_2_Y3          , 0),
		AS3676_REG(ALS_group_2_X1          , 0),
		AS3676_REG(ALS_group_2_K1          , 0),
		AS3676_REG(ALS_group_2_X2          , 0),
		AS3676_REG(ALS_group_2_K2          , 0),
		AS3676_REG(ALS_group_3_Y0          , 0),
		AS3676_REG(ALS_group_3_Y3          , 0),
		AS3676_REG(ALS_group_3_X1          , 0),
		AS3676_REG(ALS_group_3_K1          , 0),
		AS3676_REG(ALS_group_3_X2          , 0),
		AS3676_REG(ALS_group_3_K2          , 0),
	},
};

static int as3676_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int as3676_remove(struct i2c_client *client);

static const struct i2c_device_id as3676_id[] = {
	{ "as3676", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, as3676_id);

#ifdef CONFIG_PM
static int as3676_suspend(struct device *dev)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	dev_info(dev, "Suspending AS3676\n");

	AS3676_LOCK();
	data->als_control_backup = AS3676_READ_REG(AS3676_REG_ALS_control);
	data->als_result_backup = i2c_smbus_read_byte_data(data->client,
			AS3676_REG_ALS_result);
	as3676_switch_als(data, 0);
	AS3676_UNLOCK();

	return 0;
}

static int as3676_resume(struct device *dev)
{
	struct as3676_data *data = dev_get_drvdata(dev);

	dev_info(dev, "Resuming AS3676\n");

	AS3676_LOCK();
	AS3676_WRITE_REG(AS3676_REG_ALS_result, data->als_result_backup);
	if (data->als_control_backup & 1)
		as3676_switch_als(data, 1);
	AS3676_UNLOCK();

	return 0;
}

static int as3676_dev_suspend(struct device *dev)
{
	return as3676_suspend(dev);
}

static int as3676_dev_resume(struct device *dev)
{
	return as3676_resume(dev);
}

static const struct dev_pm_ops as3676_pm = {
	.suspend  = as3676_dev_suspend,
	.resume   = as3676_dev_resume,
};
#endif


static void as3676_shutdown(struct i2c_client *client)
{
	struct as3676_data *data = i2c_get_clientdata(client);
	int i;

	dev_info(&client->dev, "Shutting down AS3676\n");

	for (i = 0; i < data->num_leds; i++)
		as3676_set_brightness(data, data->leds+i, 0);

	AS3676_LOCK();
	AS3676_WRITE_REG(AS3676_REG_Control, 0x00);
	AS3676_WRITE_REG(AS3676_REG_ALS_control, 0x00);
	AS3676_UNLOCK();
}

static struct i2c_driver as3676_driver = {
	.driver = {
		.name   = "as3676",
#ifdef CONFIG_PM
		.pm     = &as3676_pm,
#endif
	},
	.probe  = as3676_probe,
	.remove = as3676_remove,
	.shutdown = as3676_shutdown,
	.id_table = as3676_id,
};

static int device_add_attributes(struct device *dev,
				 struct device_attribute *attrs)
{
	int error = 0;
	int i;

	if (attrs) {
		for (i = 0; attr_name(attrs[i]); i++) {
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
		for (i = 0; attr_name(attrs[i]); i++)
			device_remove_file(dev, &attrs[i]);
}

static u8 as3676_read_reg(struct as3676_data *data, u8 reg)
{
	return data->regs[reg].value;
}

static s32 as3676_write_reg(struct as3676_data *data, u8 reg, u8 value)
{
	s32 ret = i2c_smbus_write_byte_data(data->client, reg, value);
	if (ret == 0)
		data->regs[reg].value = value;
	return ret;
}

static s32 as3676_modify_reg(struct as3676_data *data, u8 reg, u8 reset, u8 set)
{
	s32 ret;
	u8 val = (data->regs[reg].value & ~reset) | set;

	ret = i2c_smbus_write_byte_data(data->client, reg, val);

	if (ret == 0)
		data->regs[reg].value = val;
	return ret;
}

static void as3676_switch_led(struct as3676_data *data,
		struct as3676_led *led,
		int mode)
{
	int pattern_running = data->pattern_running;

	if (led->has_extended_mode) {
		u8 ext_pattern = (AS3676_READ_REG(AS3676_REG_Pattern_control)
				>> (led->dls_mode_shift+4)) & 1;
		if (ext_pattern && (mode != 1 || led->use_pattern == 0))
			pattern_running--;

		if (!ext_pattern && mode == 1 && led->use_pattern)
			pattern_running++;
	} else {
		u8 old_mode = (AS3676_READ_REG(led->mode_reg)
				>> led->mode_shift) & 0x3;
		if (old_mode == 0x3 && (mode != 1 || led->use_pattern == 0))
			pattern_running--;

		if (old_mode != 0x3 && mode == 1 && led->use_pattern)
			pattern_running++;
	}

	/* If the last led is going to leave the pattern generator stop it
	   avoiding artefacts */
	if (pattern_running == 0 && data->pattern_running == 1) {
		/* Stop pattern generator avoiding flashes at next start */
		AS3676_WRITE_PATTERN(0);
		/* Kickstart pattern generator */
		AS3676_WRITE_REG(AS3676_REG_Pattern_control,
				AS3676_READ_REG(AS3676_REG_Pattern_control));
	}

	switch (mode) {
	case 0:
		AS3676_MODIFY_REG(led->mode_reg,
				0x03 << led->mode_shift, 0);
		if (led->has_extended_mode) {
			AS3676_MODIFY_REG(AS3676_REG_Pattern_control,
					1 << (led->dls_mode_shift+4), 0);
		}
		break;
	case 1: {
			u8 on_val = (led->use_pattern ? 0x3 : 0x1);
			if (led->has_extended_mode) {
				on_val = 0x3;
				AS3676_MODIFY_REG(AS3676_REG_Pattern_control,
						1 << (led->dls_mode_shift+4),
						led->use_pattern
						<< (led->dls_mode_shift+4));
			}
			AS3676_MODIFY_REG(led->mode_reg,
					0x03 << led->mode_shift,
					on_val << led->mode_shift);
			break;
		}
	case 2:
		AS3676_MODIFY_REG(led->mode_reg, 0x3<<led->mode_shift,
				0x2<<led->mode_shift);
		break;

	}
	/* If we have now the first led running on a pattern,
	   make sure values are correct */
	if (pattern_running == 1 && data->pattern_running == 0) {
		AS3676_WRITE_PATTERN(data->pattern_data);
		/* Kickstart pattern generator to correctly use pattern_delay */
		AS3676_WRITE_REG(AS3676_REG_Pattern_control,
				AS3676_READ_REG(AS3676_REG_Pattern_control));
	}
	data->pattern_running = pattern_running;
}

static void as3676_check_DCDC(struct as3676_data *data)
{ /* Here we check if one or more leds are connected to DCDC.
     Doing the same for charge pump is not necessary due to cp_auto_on */
	int i, on_dcdc = 0;
	struct as3676_led *led;
	u8 value;

	for (i = 0; i < 3; i++) {
		led = data->leds + i;
		if (led->pled && led->pled->on_charge_pump)
			continue;
		if (AS3676_READ_REG(led->mode_reg) & (3<<led->mode_shift)) {
			on_dcdc++;
			continue;
		}
	}
	value = AS3676_READ_REG(AS3676_REG_Control);
	if (((value&0x8) && !on_dcdc) || (!(value&0x8) && on_dcdc)) {
		AS3676_MODIFY_REG(AS3676_REG_Control, 0x8, on_dcdc ? 8 : 0);
		/* alternative setup to avoid current spikes */
		/* see application note AN3676_DCDC_1v0.pdf */
		usleep_range(12000, 20000);
	}
}

static void as3676_set_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct device *dev = led_cdev->dev->parent;
	struct as3676_data *data = dev_get_drvdata(dev);
	struct as3676_led *led = ldev_to_led(led_cdev);
	as3676_set_brightness(data, led, value);
	if (led->dim_brightness != -1 && data->dimming_in_progress) {
		dev_warn(dev, "LED %s is currently being dimmed, changing"
				"brightness not possible!", led->name);
	}
}
static void as3676_set_brightness(struct as3676_data *data,
		struct as3676_led *led, enum led_brightness value)
{
	u8 prev_value = AS3676_READ_REG(led->reg);

	AS3676_LOCK();
	value = (value * led->pled->max_current_uA + 38250/2) / 38250;
	if ((prev_value == LED_OFF) != (value == LED_OFF)) {
		/* we must switch on/off */
		as3676_switch_led(data, led, value != LED_OFF);
		as3676_check_DCDC(data);
		AS3676_WRITE_REG(led->reg, value);
	} else {
		AS3676_WRITE_REG(led->reg, value);
	}
	AS3676_UNLOCK();
}

#define MSNPRINTF(...) do { act = snprintf(buf, rem, __VA_ARGS__);	\
				if (act > rem)				\
					goto exit;			\
				buf += act;				\
				rem -= act;				\
			} while (0)

static ssize_t as3676_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	size_t ps = PAGE_SIZE, cw = 0;
	u8 cr;
	int i = 0;
	struct as3676_reg *reg;

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

	cw = scnprintf(buf, ps, "pattern_running=%d\n",
				data->pattern_running);
	ps -= cw;
	buf += cw;

	return PAGE_SIZE - ps;
}

static ssize_t as3676_debug_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int i;
	u8 reg, val;
	i = sscanf(buf, "0x%3hhx=0x%3hhx", &reg, &val);
	if (i != 2)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_WRITE_REG(reg, val);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_use_pattern_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->use_pattern);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_use_pattern_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i, use_pattern;

	i = sscanf(buf, "%1d", &use_pattern);
	if (i != 1)
		return -EINVAL;

	AS3676_LOCK();

	led->use_pattern = use_pattern;

	if (AS3676_READ_REG(led->reg) == LED_OFF)
		goto exit;

	as3676_switch_led(data, led, 1);
exit:
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_dim_brightness_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));

	snprintf(buf, PAGE_SIZE, "%d\n", led->dim_brightness);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_dim_brightness_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	int i;
	int dim_brightness;
	i = sscanf(buf, "%10d", &dim_brightness);
	if (i != 1)
		return -EINVAL;
	if (dim_brightness > 255 || dim_brightness < -1)
		return -EINVAL;
	if (data->dimming_in_progress) {
		dev_warn(dev, "Cannot change dimming parameters while dimming"
				" is in progress\n");
		return -EAGAIN;
	}
	led->dim_value =
		(dim_brightness * led->pled->max_current_uA + 38250/2) / 38250;
	led->dim_brightness = dim_brightness;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_data_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pdata, p, i;
	ssize_t act, rem = PAGE_SIZE;
	AS3676_LOCK();
	pdata  = data->pattern_data;
	AS3676_UNLOCK();
	p = pdata;
	if (AS3676_READ_REG(AS3676_REG_Pattern_control) & 1) {
		int r = 0, g = 0, b = 0, i;
		/* 10 rgb bits in 32 bits */
		for (i = 0; i < 10; i++) {
			r |= (pdata & 1) << i;
			pdata >>= 1;
			g |= (pdata & 1) << i;
			pdata >>= 1;
			b |= (pdata & 1) << i;
			pdata >>= 1;
		}

		MSNPRINTF("use_color == 1: 30 bit pattern "
				"data %#010x, curr1/rgb1=%#05x, "
				"curr2/rgb2=%#05x, curr6/rgb3=%#05x\n"
				       , p, r, g, b);
		for (i = 0; i < 10; i++) {
			if (r & (1 << i))
				MSNPRINTF("-");
			else
				MSNPRINTF("_");
		}
		MSNPRINTF("\n");
		for (i = 0; i < 10; i++) {
			if (g & (1 << i))
				MSNPRINTF("-");
			else
				MSNPRINTF("_");
		}
		MSNPRINTF("\n");
		for (i = 0; i < 10; i++) {
			if (b & (1 << i))
				MSNPRINTF("-");
			else
				MSNPRINTF("_");
		}
	} else {
		MSNPRINTF("use_color == 0: 32 bit pattern "
				"data %#010x\n", p);
		for (i = 0; i < 32; i++) {
			if (p & (1 << i))
				MSNPRINTF("-");
			else
				MSNPRINTF("_");
		}
	}
	MSNPRINTF("\n");
exit:
	return PAGE_SIZE - rem;
}

static ssize_t as3676_pattern_data_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int r, g, b, pdata, i;
	i = sscanf(buf, "%10x,%10x,%10x", &pdata, &g, &b);
	if (i != 1 && i != 3)
		return -EINVAL;
	if (i == 3) {
		r = pdata;
		pdata = 0;
		/* Read in 10 rgb bits */
		for (i = 9; i >= 0 ; i--) {
			pdata <<= 1;
			pdata |= (b >> i) & 1;
			pdata <<= 1;
			pdata |= (g >> i) & 1;
			pdata <<= 1;
			pdata |= (r >> i) & 1;
		}

	}
	AS3676_LOCK();
	data->pattern_data = pdata;
	if (data->pattern_running)
		AS3676_WRITE_PATTERN(pdata);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_duration_secs_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_duration = 1, creg;
	creg = AS3676_READ_REG(AS3676_REG_Gpio_current);
	if (creg & 0x40)
		pattern_duration *= 8;

	snprintf(buf, PAGE_SIZE, "%d, possible 1,8\n", pattern_duration);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_duration_secs_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_duration, i;
	i = sscanf(buf, "%10u", &pattern_duration);
	if (i != 1)
		return -EINVAL;
	if ((pattern_duration != 8) && (pattern_duration != 1))
		return -EINVAL;
	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_Gpio_current, 0x40,
			  (pattern_duration == 8) ? 0x40 : 0);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_use_color_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_use_color = 42;
	pattern_use_color =  AS3676_READ_REG(AS3676_REG_Pattern_control) & 1;
	snprintf(buf, PAGE_SIZE, "%d\n", pattern_use_color);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_use_color_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_use_color, i;
	i = sscanf(buf, "%10u", &pattern_use_color);
	pattern_use_color = !!pattern_use_color;
	if (i != 1)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_Pattern_control, 1, pattern_use_color);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_delay, creg, greg, pdsecs;
	creg = AS3676_READ_REG(AS3676_REG_Pattern_control);
	greg = AS3676_READ_REG(AS3676_REG_Gpio_current);
	pdsecs = pattern_delay = ((creg & 0x6) >> 1) | ((greg & 0x10) >> 2);
	if (greg & 0x40)
		pdsecs *= 8;
	snprintf(buf, PAGE_SIZE, "%d pattern_durations = %d secs\n",
			pattern_delay, pdsecs);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_delay_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int pattern_delay, i;
	i = sscanf(buf, "%3u", &pattern_delay);
	if (i != 1)
		return -EINVAL;
	if (pattern_delay > 7)
		return -EINVAL;
	pattern_delay = pattern_delay << 1;
	pattern_delay = (pattern_delay & 0x6) | ((pattern_delay & 0x8) << 1);
	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_Pattern_control,
			0x06, pattern_delay & 0x6);
	AS3676_MODIFY_REG(AS3676_REG_Gpio_current, 0x10, pattern_delay & 0x10);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_filter_speed_mHz_show(struct device *dev,
				char *buf, int offset)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int filter_speed_mHz, filter_speed;
	filter_speed = (AS3676_READ_REG(AS3676_REG_ALS_filter) >> offset) & 0x7;
	filter_speed_mHz = 250 << filter_speed;
	snprintf(buf, PAGE_SIZE, "%d\n", filter_speed_mHz);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_filter_speed_mHz_store(struct device *dev,
				const char *buf, size_t size, int offset)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int filter_speed_mHz, filter_speed, i, highest_bit;
	i = sscanf(buf, "%10d", &filter_speed_mHz);
	if (i != 1)
		return -EINVAL;

	/* The following code rounds to the neares power of two */
	/* division while preserving round bit */
	filter_speed = (filter_speed_mHz << 1) / 250;
	highest_bit = fls(filter_speed);
	/* now check the second most significant bit */
	if (highest_bit < 2)
		filter_speed = filter_speed & 1;
	else if (filter_speed & (1 << (highest_bit - 2)))
		filter_speed = highest_bit - 1;
	else
		filter_speed = highest_bit - 2;

	if (filter_speed > 7)
		filter_speed = 7;

	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_ALS_filter, 0x7 << offset,
			filter_speed<<offset);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_filter_up_speed_mHz_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_als_filter_speed_mHz_show(dev, buf, 0);
}

static ssize_t as3676_als_filter_up_speed_mHz_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_als_filter_speed_mHz_store(dev, buf, size, 0);
}

static ssize_t as3676_als_filter_down_speed_mHz_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_als_filter_speed_mHz_show(dev, buf, 4);
}

static ssize_t as3676_als_filter_down_speed_mHz_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_als_filter_speed_mHz_store(dev, buf, size, 4);
}

static ssize_t as3676_als_offset_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int als_offset;
	als_offset = AS3676_READ_REG(AS3676_REG_ALS_offset);

	snprintf(buf, PAGE_SIZE, "%d\n", als_offset);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_offset_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	u8 als_offset;
	int i;

	i = sscanf(buf, "%3hhu", &als_offset);
	if (i != 1)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_WRITE_REG(AS3676_REG_ALS_offset, als_offset);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_gain_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int als_gain, creg;
	char *s;
	creg = AS3676_READ_REG(AS3676_REG_ALS_control);
	als_gain = (creg & 0x06) >> 1;

	switch (als_gain) {
	case 0:
		s = "1/4";
		break;
	case 1:
		s = "1/2";
		break;
	case 2:
		s = "1";
		break;
	case 3:
		s = "2";
		break;
	default:
		s = "will not happen";
		break;
	}
	snprintf(buf, PAGE_SIZE, "%d (corresponds to %s)\n", als_gain, s);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_gain_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	u8 als_gain;
	int i;

	i = sscanf(buf, "%3hhu", &als_gain);
	if (i != 1)
		return -EINVAL;
	if (als_gain > 3)
		return -EINVAL;

	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_ALS_control, 0x06, als_gain<<1);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_use_softdim_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int use_softdim = AS3676_READ_REG(AS3676_REG_Pattern_control) & 0x8;
	use_softdim >>= 3;
	snprintf(buf, PAGE_SIZE, "%d\n", use_softdim);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_pattern_use_softdim_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int use_softdim, i;
	i = sscanf(buf, "%10d", &use_softdim);
	if (i != 1)
		return -EINVAL;
	use_softdim = !!use_softdim;

	AS3676_LOCK();
	if (((AS3676_READ_REG(AS3676_REG_Pattern_control) & 0x8) >> 3)
		== use_softdim)
		goto exit;

	if (use_softdim && data->dimming_in_progress) {
		dev_warn(dev, "Cannot use softdim while dimming is in"
				" progress\n");
		AS3676_UNLOCK();
		return -EINVAL;
	}
	AS3676_MODIFY_REG(AS3676_REG_Pattern_control, 0x08, use_softdim<<3);
exit:
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_group_n_show(u8 gid, struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	u8 group_reg;
	s32 y0, y3, x1, x2, x, y;
	s32 k1, k2;
	const uint inc = 8;
	int i;
	ssize_t act, rem = PAGE_SIZE;

	AS3676_LOCK();
	group_reg = AS3676_REG_ALS_group_1_Y0 + (gid - 1) * 6;
	y0 = AS3676_READ_REG(group_reg);
	y3 = AS3676_READ_REG(group_reg + 1);
	x1 = AS3676_READ_REG(group_reg + 2);
	k1 = (s8) AS3676_READ_REG(group_reg + 3);
	x2 = AS3676_READ_REG(group_reg + 4);
	k2 = (s8) AS3676_READ_REG(group_reg + 5);
	AS3676_UNLOCK();
	MSNPRINTF("%d,%d,%d,%d,%d,%d"
			"(y0=%d, x1=%d, k1=%d, x2=%d, k2=%d, y3=%d)\n",
			y0, x1, k1, x2, k2, y3,
			y0, x1, k1, x2, k2, y3);

	MSNPRINTF("       ");
	for (i = 0; i < 255; i += inc)
		MSNPRINTF("y");
	MSNPRINTF("\n");
	/* Print first segment, straight line */
	for (x = 0; x < x1; x += inc) {
		MSNPRINTF("x0=%3d,y=%3d:", x, y0);
		for (i = 0; i < y0; i += inc)
			MSNPRINTF(" ");
		MSNPRINTF("|\n");
	}
	/* Second segment, slope k1 */
	for (; x < x2; x += inc) {
		y = ((k1 * (x - x1)) / 32) + y0;
		MSNPRINTF("x1=%3d,y=%3d:", x, y);
		for (i = 0; i < y; i += inc)
			MSNPRINTF(" ");
		if (k1 < 0)
			MSNPRINTF("/\n");
		else
			MSNPRINTF("\\\n");
	}
	/* Third segment, slope k2 */
	for (; x < 256; x += inc) {
		y = (((k1 * (x2 - x1))) / 32)
			+ ((k2 * (x - x2)) / 32)
			+ y0;
		if ((k2 >= 0 && y > y3) ||
			(k2 < 0 && y < y3))
			break;
		MSNPRINTF("x2=%3d,y=%3d:", x, y);
		for (i = 0; i < y; i += inc)
			MSNPRINTF(" ");
		if (k2 < 0)
			MSNPRINTF("/\n");
		else
			MSNPRINTF("\\\n");
	}
	/* Fourth and last segment straight line */
	for (; x < 256; x += inc) {
		MSNPRINTF("x3=%3d,y=%3d:", x, y3);
		for (i = 0; i < y3; i += inc)
			MSNPRINTF(" ");
		MSNPRINTF("|\n");
	}

exit:
	return PAGE_SIZE - rem;
}

#if AS3676_ENABLE_GROUP_CHECK
static int as3676_als_group_check(struct device *dev,
		struct as3676_als_group *g)
{
	s32 y2, x3;
	if (g->gn == 0 || g->gn > 3) {
			dev_warn(dev, "invalid gn must be 1-3, is %d\n",
					g->gn);
			return -EINVAL;
	}
	if (g->y0 > g->y3) {
		if (g->k1 > 0 || g->k2 > 0) {
			dev_warn(dev, "k1=%d and k2=%d must be negative with"
					"this y0,y3\n", g->k1, g->k2);
			return -EINVAL;
		}
	} else {
		if (g->k1 < 0 || g->k2 < 0) {
			dev_warn(dev, "k1=%d and k2=%d must be positive with"
					" this given y0,y3\n", g->k1, g->k2);
			return -EINVAL;
		}
	}
	if (g->x1 == 0 || g->k1 == 0) {
		dev_warn(dev, "k1=%d and x1=%d must not be zero\n",
				g->k1, g->x1);
		return -EINVAL;
	}
	if (g->x1 > g->x2) {
		dev_warn(dev, "x1=%d must be smaller than x2=%d\n",
				g->x1, g->x2);
		return -EINVAL;
	}

	/* Test y2 */
	y2 = g->y0 + (((s32)g->k1) * (g->x2 - g->x1)) / 32;

	if (y2 < 0 || y2 > 255 ||
		((g->k2 >= 0) && (g->y3 < y2)) ||
		((g->k2 < 0) && (g->y3 > y2))) {
		dev_warn(dev, "Invalid y2 would be %d\n", y2);
		return -EINVAL;
	}

	/* Test x3 */
	if (g->k2 == 0) {
		if (y2 != g->y3) {
			dev_warn(dev, "Invalid y3, must equal y2 for k2==0!\n");
			return -EINVAL;
		}
	} else {
		x3 = g->x2 + ((g->y3 - y2)) * 32 / g->k2;
		if (x3 < 0 || x3 > 255 || x3 < g->x2) {
			dev_warn(dev, "Invalid x3 would be %d, y2=%d\n",
					x3, y2);
			return -EINVAL;
		}
	}

	return 0;
}
#endif

static ssize_t as3676_als_group_n_store(u8 gid, struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int i;
	u8 group_reg = AS3676_REG_ALS_group_1_Y0 + (gid - 1) * 6;
	struct as3676_als_group g;

	g.gn = gid;
	i = sscanf(buf, "%3hhu,%3hhu,%3hhd,%3hhu,%3hhd,%3hhu",
			&g.y0, &g.x1, &g.k1, &g.x2, &g.k2, &g.y3);

	if (i < 6)
		return -EINVAL;

#if AS3676_ENABLE_GROUP_CHECK
	i = as3676_als_group_check(dev, &g);

	if (i != 0)
		return i;
#endif

	AS3676_LOCK();
	AS3676_WRITE_REG(group_reg + 0, g.y0);
	AS3676_WRITE_REG(group_reg + 1, g.y3);
	AS3676_WRITE_REG(group_reg + 2, g.x1);
	AS3676_WRITE_REG(group_reg + 3, g.k1);
	AS3676_WRITE_REG(group_reg + 4, g.x2);
	AS3676_WRITE_REG(group_reg + 5, g.k2);
	AS3676_UNLOCK();

	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_group1_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_als_group_n_show(1, dev, attr, buf);
}

static ssize_t as3676_als_group1_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_als_group_n_store(1, dev, attr, buf, size);
}
static ssize_t as3676_als_group2_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_als_group_n_show(2, dev, attr, buf);
}

static ssize_t as3676_als_group2_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_als_group_n_store(2, dev, attr, buf, size);
}
static ssize_t as3676_als_group3_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_als_group_n_show(3, dev, attr, buf);
}

static ssize_t as3676_als_group3_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_als_group_n_store(3, dev, attr, buf, size);
}

#define DELTA(A, B) (((A) < (B)) ? ((B)-(A)) : ((A)-(B)))
static u16 dim_speed2time[8] = { 1000, 1900, 500, 950, 100, 190, 50, 95 };

static ssize_t as3676_dim_time_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int dim_speed = (AS3676_READ_REG(AS3676_REG_Pwm_control) >> 3) & 0x7;
	u16 dim_time = dim_speed2time[dim_speed];
	snprintf(buf, PAGE_SIZE, "%d (50,95,190,500,950,1900)\n", dim_time);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_dim_time_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int dim_time, i, curr_best = 3, delta;
	i = sscanf(buf, "%10d", &dim_time);
	if (i != 1 || dim_time > 10000)
		return -EINVAL;
	delta = 65535;
	for (i = 0; i < ARRAY_SIZE(dim_speed2time) ; i++) {
		/* Omit settings where there exist better ones */
		if (i == 0 || i == 4)
			continue;
		if (DELTA(dim_time, dim_speed2time[i]) < delta) {
			delta = DELTA(dim_time, dim_speed2time[i]);
			curr_best = i;
		}
	}
	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x38, curr_best<<3);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_dim_start_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	if (data->dimming_in_progress)
		snprintf(buf, PAGE_SIZE, "1 (in progress)\n");
	else
		snprintf(buf, PAGE_SIZE, "0 (ready)\n");
	return strnlen(buf, PAGE_SIZE);
}

static void as3676_clear_dimming(struct as3676_data *data)
{
	int i;
	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = data->leds + i;
		led->dim_brightness = -1;
		led->dim_value = -1;
	}
	data->dimming_in_progress = 0;
}

static int as3676_get_dimming_direction(struct as3676_data *data)
{
	int i, dir = 0, err = 0;
	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = data->leds + i;
		if (led->dim_brightness == -1)
			continue;
		if (AS3676_READ_REG(led->reg) == led->dim_value)
			continue;
		switch (dir) {
		case 0:
			if (AS3676_READ_REG(led->reg)
					> led->dim_value)
				dir = -1;
			else
				dir = 1;
			break;
		case 1:
			if (AS3676_READ_REG(led->reg)
					> led->dim_value)
				err = -EINVAL;
			break;
		case -1:
			if (AS3676_READ_REG(led->reg)
					< led->dim_value)
				err = -EINVAL;
			break;
		}
	}
	if (err)
		return err;
	return dir;
}

static int as3676_check_cross_dimming(struct device *dev,
struct as3676_data *data, int dir)
{
	int i, j, use_group;
	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = data->leds + i;
		led->marker = 0;
	}
	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = data->leds + i;
		int adder1, adder2;
		if (led->marker)
			continue;
		if (led->dim_brightness == -1)
			continue;

		use_group = AS3676_READ_REG(led->mode_reg + 0x93)
			& (0x3<<led->mode_shift);
		if (dir == 1)
			adder1 = AS3676_READ_REG(led->reg);
		else
			adder1 = led->dim_value;
		led->marker = 1;
		for (j = i; j < data->num_leds; j++) {
			struct as3676_led *led2 = data->leds + j;
			if (led2->dim_brightness == -1)
				continue;
			if (led2->adder_reg != led->adder_reg)
				continue;
			if (led2->marker)
				continue;
			if (use_group) {
				dev_warn(dev, "Led %s uses ALS, cannot be "
					      "dimmed together with led %s\n",
					      led->name, led2->name);
				return -EINVAL;
			}
			if (AS3676_READ_REG(led->mode_reg + 0x93)
					& (0x3<<led->mode_shift)) {
				dev_warn(dev, "Led %s uses ALS, cannot be "
						"dimmed together with led %s\n",
						led2->name, led->name);
				return -EINVAL;
			}

			led2->marker = 1;
			if (dir == 1)
				adder2 = AS3676_READ_REG(led2->reg);
			else
				adder2 = led2->dim_value;
			if (adder1 != adder2) {
				dev_warn(dev, "Led %s would need adder value %d"
				       " and led %s would need adder value %d.",
					led->name, adder1, led2->name, adder2);
				return -EINVAL;
			}
		}
	}
	return 0;
}

static u8 as3676_als_corrected_adder(u8 value, struct as3676_data *data,
		struct as3676_led *led)
{
	int gid = AS3676_READ_REG(led->mode_reg + 0x93) &
		(0x3<<led->mode_shift);
	int amb_result;
	u8 group_reg = AS3676_REG_ALS_group_1_Y0 + (gid - 1) * 6;
	s32 y0, y3, x1, x2, x3, out;
	s32 k1, k2;

	if (gid == 0)
		return value;

	amb_result = i2c_smbus_read_byte_data(data->client,
			AS3676_REG_ALS_result);

	y0 = AS3676_READ_REG(group_reg);
	y3 = AS3676_READ_REG(group_reg + 1);
	x1 = AS3676_READ_REG(group_reg + 2);
	k1 = (s8) AS3676_READ_REG(group_reg + 3);
	x2 = AS3676_READ_REG(group_reg + 4);
	k2 = (s8) AS3676_READ_REG(group_reg + 5);
	x3 = x2 + (y3 - (x2 - x1) * k1 / 32 - y0) * 32 / k2;

	if (amb_result < x1) /* first part */
		out = y0;
	else if (amb_result < x2) /* second part */
		out = k1 * (amb_result - x1) / 32 + y0;
	else if (amb_result < x3) /* third part */
		out = k1 * (x2 - x1) / 32 + k2 * (amb_result - x2) / 32 + y0;
	else /* fourth part */
		out = y3;

	return value * out / 256;
}

static ssize_t as3676_dim_start_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int dim_start, i, dir, err = 0, dim_speed, use_softdim;
	u16 dim_time;

	use_softdim = (AS3676_READ_REG(AS3676_REG_Pattern_control) & 0x8)>>3;

	i = sscanf(buf, "%5d", &dim_start);
	if (i != 1)
		return -EINVAL;
	if (dim_start != 1 && dim_start != 0)
		return -EINVAL;

	if (!dim_start)
		goto exit_clr;

	if (use_softdim) {
		dev_warn(dev, "pattern_use_softdim is set,"
				" dimming not possible\n");
		err = -EBUSY;
		goto exit_clr;
	}

	if (dim_start && data->dimming_in_progress) {
		dev_warn(dev, "Dimming in progress, try again later\n");
		return -EAGAIN;
	}

	data->dimming_in_progress = 1;

	/* Before continuing remove dimming request which don't require
	   dimming */
	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = data->leds + i;
		if (AS3676_READ_REG(led->reg) == led->dim_value)
			led->dim_brightness = -1;
	}

	dir = as3676_get_dimming_direction(data);
	if (dir != 1 && dir != -1) {
		dev_warn(dev, "Inconsistent dimming directions, "
				"resetting dim values\n");
		goto exit_clr;
	}

	err = as3676_check_cross_dimming(dev, data, dir);
	if (err) {
		dev_warn(dev, "Inconsistent cross dimming, "
				"each subset of curr1/rgb1/curr30,curr41, "
				"curr2/rgb2/curr31/curr42 and "
				"curr6/rgb3/curr32/curr43 must either dim up "
				"from the same value or down to the same "
				"value!\n");
		goto exit_clr;
	}

	AS3676_LOCK();
	dim_speed = (AS3676_READ_REG(AS3676_REG_Pwm_control) >> 3) & 0x7;
	dim_time = dim_speed2time[dim_speed];
	if (dir == -1) { /* down dimming */
		/* To be safe disable adder and pwm, should be disabled already
		   anyway */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x0);
		AS3676_WRITE_REG(AS3676_REG_Pwm_code, 0xff);

		for (i = 0; i < data->num_leds; i++) {
			struct as3676_led *led = data->leds + i;
			if (led->dim_brightness == -1)
				continue;
			/* turn off adder */
			AS3676_MODIFY_REG(led->adder_on_reg,
					1<<led->adder_on_shift, 0);
			AS3676_WRITE_REG(led->adder_reg,
					as3676_als_corrected_adder(
					led->dim_value, data, led));
			AS3676_WRITE_REG(led->reg, AS3676_READ_REG(led->reg) -
					led->dim_value);
			/* FIXME Here the LED is at a lower value until the next
			   i2c command hits. Alternatively we could do something
			   like pumping the value over or having one i2c request
			   for the two registers and good i2c driver */
			AS3676_MODIFY_REG(led->adder_on_reg, 0,
					1<<led->adder_on_shift);
			/* Switch led to pwm */
			as3676_switch_led(data, led, 2);
		}

		/* Enable down dimming */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x4);
		/* Wait for dimming to be finished */
		schedule_delayed_work(&data->dim_work,
				msecs_to_jiffies(dim_time));
	} else { /* up dimming */
		/* To be safe disable adder and pwm, should be disabled already
		   anyway */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x0);
		AS3676_WRITE_REG(AS3676_REG_Pwm_code, 0x00);

		for (i = 0; i < data->num_leds; i++) {
			struct as3676_led *led = data->leds + i;
			if (led->dim_brightness == -1)
				continue;
			AS3676_MODIFY_REG(led->adder_on_reg,
					1<<led->adder_on_shift, 0);
			AS3676_WRITE_REG(led->adder_reg,
				as3676_als_corrected_adder(
				AS3676_READ_REG(led->reg), data, led));
			/* Switch to pwm */
			as3676_switch_led(data, led, 2);
			/* FIXME Here the LED is for a short while off until
			   the next i2c command hits */
			/* Alternatively we could do something like pumping the
			   value over */
			/* or having one i2c request for the two registers and
			   good i2c driver */
			AS3676_MODIFY_REG(led->adder_on_reg, 0,
					1<<led->adder_on_shift);
			as3676_check_DCDC(data);
			AS3676_WRITE_REG(led->reg, led->dim_value -
					AS3676_READ_REG(led->reg));
		}
		/* Now start the updimming */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x2);
		/* Wait for dimming to be finished */
		schedule_delayed_work(&data->dim_work,
				msecs_to_jiffies(dim_time));
	}
	AS3676_UNLOCK();
	goto exit;
exit_clr:
	as3676_clear_dimming(data);
exit:
	if (err)
		return err;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_use_dls_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	int use_dls = AS3676_READ_REG(led->dls_mode_reg);

	use_dls >>= led->dls_mode_shift;
	use_dls &= 1;
	snprintf(buf, PAGE_SIZE, "%d\n", use_dls);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_use_dls_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	u8 use_dls;
	i = sscanf(buf, "%3hhu", &use_dls);
	if (i != 1)
		return -EINVAL;
	use_dls = !!use_dls;
	AS3676_LOCK();
	AS3676_MODIFY_REG(led->dls_mode_reg,
			1 << led->dls_mode_shift,
			use_dls << led->dls_mode_shift);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_group_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	int als_group = AS3676_READ_REG(led->mode_reg + 0x93);

	als_group >>= led->mode_shift;
	als_group &= 0x3;
	snprintf(buf, PAGE_SIZE, "%d (0=no ambient control, N = groupN)\n"
				, als_group);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_group_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	int i;
	u8 als_group;
	i = sscanf(buf, "%3hhu", &als_group);
	if (i != 1 || als_group > 3)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_MODIFY_REG(led->mode_reg + 0x93,
			3 << led->mode_shift,
			als_group << led->mode_shift);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int als_on = AS3676_READ_REG(AS3676_REG_ALS_control) & 1;
	snprintf(buf, PAGE_SIZE, "%d\n", als_on);
	return strnlen(buf, PAGE_SIZE);
}

static void as3676_switch_als(struct as3676_data *data, int als_on)
{
	int i;
	u8 adc_result;

	if ((AS3676_READ_REG(AS3676_REG_ALS_control) & 1) == als_on)
		return;

	if (als_on)
		data->ldo_count++;
	else
		data->ldo_count--;

	/* Switch LDO on/off as required */
	AS3676_MODIFY_REG(AS3676_REG_Control, 1, (data->ldo_count >= 1));
	AS3676_MODIFY_REG(AS3676_REG_ALS_control, 1, als_on);
	/* It is important to always start a ADC conversion to prevent AS3676
	   from drawing too much power when switching off ALS. */
	AS3676_WRITE_REG(AS3676_REG_ADC_control, 0x80); /* GPIO2/LIGHT */
	for (i = 0; i < 10; i++) {
		adc_result = i2c_smbus_read_byte_data(data->client,
				AS3676_REG_ADC_MSB_result);
		if (!(adc_result & 0x80))
			break;
		udelay(10);
	}
}
static ssize_t as3676_als_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int als_on, i;
	i = sscanf(buf, "%10d", &als_on);
	if (i != 1)
		return -EINVAL;
	if (als_on != 1 && als_on != 0)
		return -EINVAL;
	AS3676_LOCK();
	as3676_switch_als(data, als_on);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_result_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int i;
	s32 als_result, amb_gain, offset;
	u32 adc_result;
	s32 hw_result = i2c_smbus_read_byte_data(data->client,
			AS3676_REG_ALS_result);

	/* Start measuring GPIO2/LIGHT */
	AS3676_LOCK();
	AS3676_WRITE_REG(AS3676_REG_ADC_control, 0x80);
	for (i = 0; i < 10; i++) {
		adc_result = i2c_smbus_read_byte_data(data->client,
				AS3676_REG_ADC_MSB_result);
		if (!(adc_result & 0x80))
			break;
		udelay(10);
	}
	adc_result <<= 3;
	adc_result |= i2c_smbus_read_byte_data(data->client,
			AS3676_REG_ADC_LSB_result);

	amb_gain = (AS3676_READ_REG(AS3676_REG_ALS_control) & 0x06) >> 1;
	amb_gain = 1 << amb_gain; /* Have gain ready for calculations */
	offset = AS3676_READ_REG(AS3676_REG_ALS_offset);
	AS3676_UNLOCK();

	/* multiply always before doing divisions to preserve precision.
	   Overflows should not happen with the values */
	als_result = (adc_result - 4 * offset) * amb_gain / 4;

	snprintf(buf, PAGE_SIZE, "adc=%d,offset=%d,amb_gain %d/4 => calc=%d,"
			" filtered=%d\n",
			adc_result, offset, amb_gain, als_result, hw_result);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_als_result_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return -EINVAL;
}

static void as3676_dim_work(struct work_struct *work)
{
	struct as3676_data *data =
		container_of(work, struct as3676_data, dim_work.work);

	int i, dir;

	dir = as3676_get_dimming_direction(data);

	AS3676_LOCK();
	if (dir == -1) { /* down dimming */
		for (i = 0; i < data->num_leds; i++) {
			struct as3676_led *led = data->leds + i;
			if (led->dim_brightness == -1)
				continue;
			AS3676_WRITE_REG(led->reg, led->dim_value);
			AS3676_MODIFY_REG(led->adder_on_reg,
					1<<led->adder_on_shift, 0);
			/* Here the LED is turned off until the next i2c command
			   hits. Alternatively we could do something like
			   pumping the value over. Switch from pwm to desired
			   mode */
			as3676_switch_led(data, led, led->dim_value != 0);
			led->ldev.brightness = led->dim_brightness;
		}
		/* Turn off Pwm */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x0);
		/* Possibly DCDC can be switched off now */
		as3676_check_DCDC(data);
	} else { /* up dimming */
		for (i = 0; i < data->num_leds; i++) {
			struct as3676_led *led = data->leds + i;
			if (led->dim_brightness == -1)
				continue;
			/* Switch off the adder */
			AS3676_MODIFY_REG(led->adder_on_reg,
					1<<led->adder_on_shift, 0);
			/* Here the LED is at a lower value until the next
			   command hits. Alternatively we could do something
			   like pumping the value over */
			AS3676_WRITE_REG(led->reg, led->dim_value);
			/* Switch to desired mode */
			as3676_switch_led(data, led, 1);
			led->ldev.brightness = led->dim_brightness;
		}
		/* Turn off Pwm */
		AS3676_MODIFY_REG(AS3676_REG_Pwm_control, 0x6, 0x0);
	}
	as3676_clear_dimming(data);
	AS3676_UNLOCK();
}

static ssize_t as3676_led_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	int val = AS3676_READ_REG(led->aud_reg) >> led->aud_shift;
	val &= 1;

	snprintf(buf, PAGE_SIZE, "%d\n", val);
	return strnlen(buf, PAGE_SIZE);
}
static ssize_t as3676_led_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	int audio_on, i;
	i = sscanf(buf, "%10d", &audio_on);
	if (i != 1)
		return -EINVAL;

	AS3676_LOCK();
	AS3676_MODIFY_REG(led->aud_reg, 1<<led->aud_shift,
			audio_on<<led->aud_shift);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}
static ssize_t as3676_curr4x_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_led_audio_on_show(dev, attr, buf);
}
static ssize_t as3676_curr4x_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_led_audio_on_store(dev, attr, buf, size);
}
static ssize_t as3676_rgbx_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_led_audio_on_show(dev, attr, buf);
}
static ssize_t as3676_rgbx_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_led_audio_on_store(dev, attr, buf, size);
}
static ssize_t as3676_curr126_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_led_audio_on_show(dev, attr, buf);
}
static ssize_t as3676_curr126_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_led_audio_on_store(dev, attr, buf, size);
}
static ssize_t as3676_curr3x_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	int val = AS3676_READ_REG(led->aud_reg) >> led->aud_shift;
	val &= 0x3;

	snprintf(buf, PAGE_SIZE, "%d\n", val);
	return strnlen(buf, PAGE_SIZE);
}
static ssize_t as3676_curr3x_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_led *led = ldev_to_led(dev_get_drvdata(dev));
	struct as3676_data *data = dev_get_drvdata(dev->parent);
	int val, i;
	i = sscanf(buf, "%10d", &val);
	if (i != 1)
		return -EINVAL;
	if (val < 0 || val > 3)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_MODIFY_REG(led->aud_reg, 0x3<<led->aud_shift,
			val<<led->aud_shift);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}
static ssize_t as3676_curr30_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_curr3x_audio_channel_show(dev, attr, buf);
}
static ssize_t as3676_curr30_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_curr3x_audio_channel_store(dev, attr, buf, size);
}
static ssize_t as3676_curr31_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_curr3x_audio_channel_show(dev, attr, buf);
}
static ssize_t as3676_curr31_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_curr3x_audio_channel_store(dev, attr, buf, size);
}
static ssize_t as3676_curr32_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_curr3x_audio_channel_show(dev, attr, buf);
}
static ssize_t as3676_curr32_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_curr3x_audio_channel_store(dev, attr, buf, size);
}
static ssize_t as3676_curr33_audio_channel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return as3676_curr3x_audio_channel_show(dev, attr, buf);
}
static ssize_t as3676_curr33_audio_channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	return as3676_curr3x_audio_channel_store(dev, attr, buf, size);
}
static ssize_t as3676_audio_on_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int val = AS3676_READ_REG(AS3676_REG_Audio_Control);
	val &= 1;
	snprintf(buf, PAGE_SIZE, "%d\n", val);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_audio_on_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int audio_on, i;
	i = sscanf(buf, "%10d", &audio_on);
	if (i != 1)
		return -EINVAL;
	if (audio_on != 1 && audio_on != 0)
		return -EINVAL;

	if ((AS3676_READ_REG(AS3676_REG_ALS_control) & 1) == audio_on)
		goto exit;

	AS3676_LOCK();
	if (audio_on)
		data->ldo_count++;
	else
		data->ldo_count--;

	/* Switch LDO on/off as required */
	AS3676_MODIFY_REG(AS3676_REG_Control, 1, (data->ldo_count >= 1));

	AS3676_MODIFY_REG(AS3676_REG_Audio_Control, 1, audio_on);
	AS3676_UNLOCK();

exit:
	return strnlen(buf, PAGE_SIZE);
}
static ssize_t as3676_audio_color_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int audio_color = AS3676_READ_REG(AS3676_REG_Audio_Control);
	audio_color >>= 1;
	audio_color &= 0x7;
	snprintf(buf, PAGE_SIZE, "%d\n", audio_color);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t as3676_audio_color_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct as3676_data *data = dev_get_drvdata(dev);
	int audio_color, i;
	i = sscanf(buf, "%10d", &audio_color);
	if (i < 0 || i > 7)
		return -EINVAL;
	AS3676_LOCK();
	AS3676_MODIFY_REG(AS3676_REG_Audio_Control, 0xe, audio_color << 1);
	AS3676_UNLOCK();
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute as3676_attributes[] = {
	AS3676_ATTR(debug),
	AS3676_ATTR(dim_start),
	AS3676_ATTR(dim_time),
	AS3676_ATTR(pattern_data),
	AS3676_ATTR(pattern_duration_secs),
	AS3676_ATTR(pattern_use_color),
	AS3676_ATTR(pattern_delay),
	AS3676_ATTR(pattern_use_softdim),
	AS3676_ATTR(als_filter_up_speed_mHz),
	AS3676_ATTR(als_filter_down_speed_mHz),
	AS3676_ATTR(als_group1),
	AS3676_ATTR(als_group2),
	AS3676_ATTR(als_group3),
	AS3676_ATTR(als_result),
	AS3676_ATTR(als_offset),
	AS3676_ATTR(als_gain),
	AS3676_ATTR(als_on),
	AS3676_ATTR(audio_on),
	AS3676_ATTR(audio_color),
	__ATTR_NULL
};

static struct device_attribute as3676_led_attributes[] = {
	AS3676_ATTR(als_group),
	AS3676_ATTR(dim_brightness),
	AS3676_ATTR(use_dls),
	AS3676_ATTR(use_pattern),
	__ATTR_NULL
};

static int as3676_configure(struct i2c_client *client,
		struct as3676_data *data, struct as3676_platform_data *pdata)
{
	int i, err = 0;

	if (pdata == NULL)
		return -EINVAL;

	INIT_DELAYED_WORK(&data->dim_work, as3676_dim_work);

	AS3676_MODIFY_REG(AS3676_REG_DCDC_control1, 0xf8,
			pdata->step_up_vtuning << 3);

	AS3676_MODIFY_REG(AS3676_REG_Audio_Input, 0x7,
			pdata->audio_gain);
	AS3676_MODIFY_REG(AS3676_REG_Audio_Input, 0x38,
			pdata->audio_agc_ctrl << 3);
	/* full amplitude, regulate using current regs: */
	AS3676_MODIFY_REG(AS3676_REG_Audio_Output, 0x07, 0x07);
	AS3676_MODIFY_REG(AS3676_REG_Audio_Control_2, 0x0e,
			pdata->audio_speed_up << 1);
	AS3676_MODIFY_REG(AS3676_REG_Audio_Control, 0xc0,
			pdata->audio_speed_down << 6);
	/* Set LDO to 2.8 volts: 20 * 0.05 + 1.8 = 2.8 Volts */
	AS3676_WRITE_REG(AS3676_REG_LDO_Voltage, 20);

	/* Configure GPIO1 and GPIO2 for DLS / Light using minimal power
	   GPIO1 : digital input, no pulls
	   GPIO2 : analog input, no pulls */
	AS3676_WRITE_REG(AS3676_REG_GPIO_control, 0xc0);

	AS3676_MODIFY_REG(AS3676_REG_Audio_Control_2, 0x30,
			pdata->audio_source << 4);

	/* Alternative Startup according to application note
	   AN3676_DCDC_v1.0.pdf: set auto feedback */
	AS3676_MODIFY_REG(AS3676_REG_DCDC_control2, 0x80, 0x80);
	if (pdata->step_up_lowcur)
		AS3676_MODIFY_REG(AS3676_REG_DCDC_control2, 0x08, 0x08);
	/* Auto feedback needs one of the currents selected as feedback: */
	AS3676_MODIFY_REG(AS3676_REG_DCDC_control1, 0x6, 0x2);

	/* Remove curr33 since it is used as audio input */
	if (pdata->audio_source == 0)
		data->num_leds--;

	for (i = 0; i < data->num_leds; i++) {
		struct as3676_led *led = &data->leds[i];
		struct as3676_platform_led *pled = &pdata->leds[i];
		led->pled = pled;
		if (pled->name)
			led->name = pled->name;
		if (pled->on_charge_pump) {
			AS3676_MODIFY_REG(led->dls_mode_reg - 0x32,
					1 << led->dls_mode_shift,
					1 << led->dls_mode_shift);
		}
		if (led->pled->max_current_uA > 38250)
			led->pled->max_current_uA = 38250;
		if (led->pled->startup_current_uA > led->pled->max_current_uA)
			led->pled->startup_current_uA =
				led->pled->max_current_uA;
		/* Round down max current to next step */
		led->pled->max_current_uA =
			led->pled->max_current_uA / 150 * 150;
		led->client = client;
		led->ldev.name = led->name;
		led->ldev.brightness = LED_OFF;
		led->ldev.brightness_set = as3676_set_led_brightness;
		led->ldev.blink_set = NULL;
		led->dim_brightness = -1;
		err = led_classdev_register(&client->dev, &led->ldev);
		if (err < 0) {
			dev_err(&client->dev,
					"couldn't register LED %s\n",
					led->name);
			goto exit;
		}
		err = device_add_attributes(led->ldev.dev,
				as3676_led_attributes);
		if (err < 0) {
			dev_err(&client->dev,
					"couldn't add attributes %s\n",
					led->name);
			goto exit;
		}
		err = device_create_file(led->ldev.dev, led->aud_file);
		if (err < 0) {
			dev_err(&client->dev,
					"couldn't add attributes %s\n",
					led->name);
			goto exit;
		}
	}

	err = device_add_attributes(&client->dev, as3676_attributes);

	if (err)
		goto exit;

	for (i = 0; i < 6; i++) {
		struct as3676_led *led = &data->leds[i];
		if (led->pled->startup_current_uA) {
			as3676_set_brightness(data, led,
				led->pled->startup_current_uA * 255 /
				led->pled->max_current_uA);

		}
	}
	return 0;

exit:
	device_remove_attributes(&client->dev, as3676_attributes);
	if (i > 0)
		for (i = i - 1; i >= 0; i--) {
			struct as3676_led *led = &data->leds[i];
			device_remove_attributes(led->ldev.dev,
				as3676_led_attributes);
			device_remove_file(led->ldev.dev, led->aud_file);
			led_classdev_unregister(&data->leds[i].ldev);
		}
	return err;
}

static int as3676_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct as3676_data *data;
	struct as3676_platform_data *as3676_pdata = client->dev.platform_data;
	int id1, id2, i;
	int err = 0;

	if (!as3676_pdata)
		return -EIO;

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	data = kzalloc(sizeof(*data), GFP_USER);
	if (!data)
		return -ENOMEM;

	/* initialize with meaningful data ( register names, etc.) */
	*data = as3676_default_data;

	dev_set_drvdata(&client->dev, data);

	data->client = client;
	mutex_init(&data->update_lock);

	id1 = i2c_smbus_read_byte_data(client, AS3676_REG_ASIC_ID1);
	id2 = i2c_smbus_read_byte_data(client, AS3676_REG_ASIC_ID2);
	if (id1 < 0) {
		err = id1;
		goto exit;
	}
	if (id2 < 0) {
		err = id2;
		goto exit;
	}
	if ((id1 != 0xae) || (id2&0xf0) != 0x50) {
		err = -ENXIO;
		dev_err(&client->dev, "wrong chip detected, ids %x %x",
				id1, id2);
		goto exit;
	}
	dev_info(&client->dev, "AS3676 driver v1.6: detected AS3676"
			"compatible chip with ids %x %x\n",
			id1, id2);
	/* all voltages on */
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

	AS3676_WRITE_REG(AS3676_REG_Control, 0x00);
	AS3676_WRITE_REG(AS3676_REG_CP_control, 0x40); /* cp_auto_on */

	i2c_set_clientdata(client, data);


	err = as3676_configure(client, data, as3676_pdata);
exit:
	if (err) {
		kfree(data);
		dev_err(&client->dev, "could not configure %x", err);
		i2c_set_clientdata(client, NULL);
	}

	return err;
}

static int as3676_remove(struct i2c_client *client)
{
	struct as3676_data *data = i2c_get_clientdata(client);
	int i;

	for (i = 0; i < AS3676_NUM_LEDS; i++) {
		struct as3676_led *led = &data->leds[i];
		device_remove_attributes(led->ldev.dev, as3676_led_attributes);
		device_remove_file(led->ldev.dev, led->aud_file);
		led_classdev_unregister(&led->ldev);
	}
	device_remove_attributes(&client->dev, as3676_attributes);

	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static int __init as3676_init(void)
{
	return i2c_add_driver(&as3676_driver);
}

static void __exit as3676_exit(void)
{
	i2c_del_driver(&as3676_driver);
}

MODULE_AUTHOR("Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AS3676 LED dimmer");

module_init(as3676_init);
module_exit(as3676_exit);

