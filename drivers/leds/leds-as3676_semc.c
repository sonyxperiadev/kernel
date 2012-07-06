/*
 * Copyright (c) 2009-2012 Sony Ericsson Mobile Comm
 *
 * Author: Courtney Cavin <courtney.cavin@sonyericsson.com>
 *
 * This file is subject to the terms and conditions of version 2 of
 * the GNU General Public License.  See the file COPYING in the main
 * directory of this archive for more details.
 */

#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/leds-as3676_semc.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/delay.h>

#define AS3676_NAME "as3676"

enum as3676_register {
	AS3676_REG_CTRL,
	AS3676_LDO_VOLTAGE,
	AS3676_GPIO_CTRL,
	AS3676_MODE_SWITCH,
	AS3676_MODE_SWITCH_2,

	AS3676_REG_PWM_CTRL,
	AS3676_REG_PWM_CODE,

	AS3676_REG_GPIO_CURR,

	AS3676_ADC_CTRL,

	AS3676_DCDC_CTRL_1,
	AS3676_DCDC_CTRL_2,

	AS3676_DLS_CTRL_1,
	AS3676_DLS_CTRL_2,

	AS3676_SINK_1_2_CTRL,
	AS3676_SINK_06_CTRL,
	AS3676_SINK_3X_CTRL,
	AS3676_SINK_4X_CTRL,

#ifdef CONFIG_LEDS_AS3676_HW_BLINK
	/* AS3676_REG_PATTERN_XXX must go after AS3676_SINK_4X_CTRL */
	AS3676_REG_PATTERN_DATA_0,
	AS3676_REG_PATTERN_DATA_1,
	AS3676_REG_PATTERN_DATA_2,
	AS3676_REG_PATTERN_DATA_3,
#endif
	AS3676_REG_PATTERN_CTRL,

	AS3676_SINK_01_CURR,
	AS3676_SINK_02_CURR,

	AS3676_SINK_06_CURR,

	AS3676_SINK_30_CURR,
	AS3676_SINK_31_CURR,
	AS3676_SINK_32_CURR,
	AS3676_SINK_33_CURR,

	AS3676_SINK_41_CURR,
	AS3676_SINK_42_CURR,
	AS3676_SINK_43_CURR,

	AS3676_SINK_RGB1_CURR,
	AS3676_SINK_RGB2_CURR,
	AS3676_SINK_RGB3_CURR,

	AS3676_OVERTEMP_CTRL,

	AS3676_AMB_CTRL,
	AS3676_AMB_FILTER,
	AS3676_AMB_OFFSET,
	AS3676_SINK_1_2_AMB,
	AS3676_SINK_06_AMB,
	AS3676_SINK_3X_AMB,
	AS3676_SINK_4X_AMB,
	AS3676_AMB_RGB_GRP,
	AS3676_GROUP_1_Y0,
	AS3676_GROUP_1_Y3,
	AS3676_GROUP_1_X1,
	AS3676_GROUP_1_X2,
	AS3676_GROUP_1_K1,
	AS3676_GROUP_1_K2,
	AS3676_GROUP_2_Y0,
	AS3676_GROUP_2_Y3,
	AS3676_GROUP_2_X1,
	AS3676_GROUP_2_X2,
	AS3676_GROUP_2_K1,
	AS3676_GROUP_2_K2,
	AS3676_GROUP_3_Y0,
	AS3676_GROUP_3_Y3,
	AS3676_GROUP_3_X1,
	AS3676_GROUP_3_X2,
	AS3676_GROUP_3_K1,
	AS3676_GROUP_3_K2,

	AS3676_REG_MAX,

	AS3676_SINK_RGB_CTRL = AS3676_SINK_06_CTRL,
	AS3676_SINK_RGB_AMB = AS3676_SINK_06_AMB,
};

enum as3676_ctrl_value {
	AS3676_CTRL_OFF,
	AS3676_CTRL_ON,
	AS3676_CTRL_PWM, /* controlled via PWM */
	AS3676_CTRL_PATTERN, /* controlled via pattern */
	AS3676_CTRL_EXT_CURR = 0x03,

	AS3676_CTRL_MASK = 0x03
};

enum as3676_sink_flags {
	AS3676_FLAG_DCDC_CTRL  = (1 << 0), /* led controls step up ctrlr */
	AS3676_FLAG_EXT_CURR   = (1 << 1), /* powered via external current */
};

int as3676_sink_map[] = {
	[AS3676_SINK_01] = AS3676_SINK_01_CURR,
	[AS3676_SINK_02] = AS3676_SINK_02_CURR,
	[AS3676_SINK_06] = AS3676_SINK_06_CURR,
	[AS3676_SINK_30] = AS3676_SINK_30_CURR,
	[AS3676_SINK_31] = AS3676_SINK_31_CURR,
	[AS3676_SINK_32] = AS3676_SINK_32_CURR,
	[AS3676_SINK_33] = AS3676_SINK_33_CURR,
	[AS3676_SINK_41] = AS3676_SINK_41_CURR,
	[AS3676_SINK_42] = AS3676_SINK_42_CURR,
	[AS3676_SINK_43] = AS3676_SINK_43_CURR,
	[AS3676_SINK_RGB1] = AS3676_SINK_RGB1_CURR,
	[AS3676_SINK_RGB2] = AS3676_SINK_RGB2_CURR,
	[AS3676_SINK_RGB3] = AS3676_SINK_RGB3_CURR,

};

static const struct as3676_sink {
	enum as3676_register ctrl;
	enum as3676_register amb;
	enum as3676_register dls;
	enum as3676_register pattern;
	int lower_bit;
	int flags;
	int dls_bit;
	int pattern_bit;
} as3676_sink[] = {
	[AS3676_SINK_01_CURR] = {
		.ctrl = AS3676_SINK_1_2_CTRL,
		.amb = AS3676_SINK_1_2_AMB,
		.lower_bit = 0,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 0,
		.flags = AS3676_FLAG_DCDC_CTRL,
	},
	[AS3676_SINK_02_CURR] = {
		.ctrl = AS3676_SINK_1_2_CTRL,
		.amb = AS3676_SINK_1_2_AMB,
		.lower_bit = 2,
		.flags = AS3676_FLAG_DCDC_CTRL,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 1,
		.flags = AS3676_FLAG_DCDC_CTRL,
	},
	[AS3676_SINK_06_CURR] = {
		.ctrl = AS3676_SINK_06_CTRL,
		.amb = AS3676_SINK_06_AMB,
		.lower_bit = 6,
		.flags = AS3676_FLAG_DCDC_CTRL,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 7,
	},
	[AS3676_SINK_30_CURR] = {
		.ctrl = AS3676_SINK_3X_CTRL,
		.amb = AS3676_SINK_3X_AMB,
		.lower_bit = 0,
		.flags = AS3676_FLAG_EXT_CURR,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 0,
		.pattern = AS3676_REG_PATTERN_CTRL,
		.pattern_bit = 4,
	},
	[AS3676_SINK_31_CURR] = {
		.ctrl = AS3676_SINK_3X_CTRL,
		.amb = AS3676_SINK_3X_AMB,
		.lower_bit = 2,
		.flags = AS3676_FLAG_EXT_CURR,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 1,
		.pattern = AS3676_REG_PATTERN_CTRL,
		.pattern_bit = 5,
	},
	[AS3676_SINK_32_CURR] = {
		.ctrl = AS3676_SINK_3X_CTRL,
		.amb = AS3676_SINK_3X_AMB,
		.lower_bit = 4,
		.flags = AS3676_FLAG_EXT_CURR,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 2,
		.pattern = AS3676_REG_PATTERN_CTRL,
		.pattern_bit = 6,
	},
	[AS3676_SINK_33_CURR] = {
		.ctrl = AS3676_SINK_3X_CTRL,
		.amb = AS3676_SINK_3X_AMB,
		.lower_bit = 6,
		.flags = AS3676_FLAG_EXT_CURR,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 3,
		.pattern = AS3676_REG_PATTERN_CTRL,
		.pattern_bit = 7,
	},
	[AS3676_SINK_41_CURR] = {
		.ctrl = AS3676_SINK_4X_CTRL,
		.amb = AS3676_SINK_4X_AMB,
		.lower_bit = 0,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 2,
	},
	[AS3676_SINK_42_CURR] = {
		.ctrl = AS3676_SINK_4X_CTRL,
		.amb = AS3676_SINK_4X_AMB,
		.lower_bit = 2,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 3,
	},
	[AS3676_SINK_43_CURR] = {
		.ctrl = AS3676_SINK_4X_CTRL,
		.amb = AS3676_SINK_4X_AMB,
		.lower_bit = 4,
		.dls = AS3676_DLS_CTRL_2,
		.dls_bit = 4,
	},
	[AS3676_SINK_RGB1_CURR] = {
		.ctrl = AS3676_SINK_RGB_CTRL,
		.amb = AS3676_SINK_RGB_AMB,
		.lower_bit = 0,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 4,
	},
	[AS3676_SINK_RGB2_CURR] = {
		.ctrl = AS3676_SINK_RGB_CTRL,
		.amb = AS3676_SINK_RGB_AMB,
		.lower_bit = 2,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 5,
	},
	[AS3676_SINK_RGB3_CURR] = {
		.ctrl = AS3676_SINK_RGB_CTRL,
		.amb = AS3676_SINK_RGB_AMB,
		.lower_bit = 4,
		.dls = AS3676_DLS_CTRL_1,
		.dls_bit = 6,
	},
};

static const u8 as3676_i2c_registers[] = {
	[AS3676_SINK_01_CURR]   = 0x09,
	[AS3676_SINK_02_CURR]   = 0x0a,
	[AS3676_SINK_06_CURR]   = 0x2f,
	[AS3676_SINK_30_CURR]   = 0x40,
	[AS3676_SINK_31_CURR]   = 0x41,
	[AS3676_SINK_32_CURR]   = 0x42,
	[AS3676_SINK_33_CURR]   = 0x43,
	[AS3676_SINK_41_CURR]   = 0x13,
	[AS3676_SINK_42_CURR]   = 0x14,
	[AS3676_SINK_43_CURR]   = 0x15,
	[AS3676_SINK_RGB1_CURR] = 0x0b,
	[AS3676_SINK_RGB2_CURR] = 0x0c,
	[AS3676_SINK_RGB3_CURR] = 0x0d,
	[AS3676_SINK_3X_CTRL]   = 0x03,
	[AS3676_SINK_4X_CTRL]   = 0x04,
	[AS3676_SINK_1_2_CTRL]  = 0x01,
	[AS3676_SINK_RGB_CTRL]  = 0x02,
	[AS3676_REG_PWM_CTRL]   = 0x16,
	[AS3676_REG_PWM_CODE]   = 0x17,
	[AS3676_REG_CTRL]       = 0x00,
	[AS3676_LDO_VOLTAGE]    = 0x07,
	[AS3676_GPIO_CTRL]      = 0x1E,
	[AS3676_MODE_SWITCH]    = 0x24,
	[AS3676_MODE_SWITCH_2]  = 0x25,
	[AS3676_ADC_CTRL]       = 0x26,
	[AS3676_REG_GPIO_CURR]  = 0x2c,
	[AS3676_OVERTEMP_CTRL]  = 0x29,
#ifdef CONFIG_LEDS_AS3676_HW_BLINK
	[AS3676_REG_PATTERN_DATA_0] = 0x19,
	[AS3676_REG_PATTERN_DATA_1] = 0x1a,
	[AS3676_REG_PATTERN_DATA_2] = 0x1b,
	[AS3676_REG_PATTERN_DATA_3] = 0x1c,
#endif
	[AS3676_REG_PATTERN_CTRL] = 0x18,
	[AS3676_DLS_CTRL_1]     = 0x56,
	[AS3676_DLS_CTRL_2]     = 0x57,
	[AS3676_DCDC_CTRL_1]    = 0x21,
	[AS3676_DCDC_CTRL_2]    = 0x22,
	[AS3676_AMB_CTRL]       = 0x90,
	[AS3676_AMB_FILTER]     = 0x91,
	[AS3676_AMB_OFFSET]     = 0x92,
	[AS3676_SINK_1_2_AMB]   = 0x94,
	[AS3676_SINK_06_AMB]    = 0x95,
	[AS3676_SINK_3X_AMB]    = 0x96,
	[AS3676_SINK_4X_AMB]    = 0x97,
	[AS3676_GROUP_1_Y0]     = 0x98,
	[AS3676_GROUP_1_Y3]     = 0x99,
	[AS3676_GROUP_1_X1]     = 0x9a,
	[AS3676_GROUP_1_X2]     = 0x9c,
	[AS3676_GROUP_1_K1]     = 0x9b,
	[AS3676_GROUP_1_K2]     = 0x9d,
	[AS3676_GROUP_2_Y0]     = 0x9e,
	[AS3676_GROUP_2_Y3]     = 0x9f,
	[AS3676_GROUP_2_X1]     = 0xa0,
	[AS3676_GROUP_2_X2]     = 0xa2,
	[AS3676_GROUP_2_K1]     = 0xa1,
	[AS3676_GROUP_2_K2]     = 0xa3,
	[AS3676_GROUP_3_Y0]     = 0xa4,
	[AS3676_GROUP_3_Y3]     = 0xa5,
	[AS3676_GROUP_3_X1]     = 0xa6,
	[AS3676_GROUP_3_X2]     = 0xa8,
	[AS3676_GROUP_3_K1]     = 0xa7,
	[AS3676_GROUP_3_K2]     = 0xa9,
};

#define AS3676_MAX_CURRENT  38250

#define AS3676_SLOW_PATTERN_BIT_DURATION_MS  250
#define AS3676_FAST_PATTERN_BIT_DURATION_MS  31

#define AS3676_REG_CTRL_WAIT_US  (12 * 1000)

#define AS3676_INTERFACE_MAX AS3676_SINK_MAX

#define AS3676_ADC_READ_RETRY_NUM  10

static const struct as3676_als_config as3676_default_config = {
	.gain = AS3676_GAIN_1,
	.filter_up = AS3676_FILTER_1HZ,
	.filter_down = AS3676_FILTER_4HZ,
	.source = AS3676_ALS_SOURCE_GPIO2,
	.curve = {
		[AS3676_AMB_OFF] = {
			.y0 = 0,
			.y3 = 0,
			.k1 = 0,
			.k2 = 0,
			.x1 = 0,
			.x2 = 0,
		},
		[AS3676_AMB_GROUP_1] = {
			.y0 = 48,
			.y3 = 255,
			.k1 = 48,
			.k2 = 48,
			.x1 = 5,
			.x2 = 127,
		},
		[AS3676_AMB_GROUP_2] = {
			.y0 = 48,
			.y3 = 255,
			.k1 = 48,
			.k2 = 48,
			.x1 = 5,
			.x2 = 127,
		},
		[AS3676_AMB_GROUP_3] = {
			.y0 = 48,
			.y3 = 255,
			.k1 = 48,
			.k2 = 48,
			.x1 = 5,
			.x2 = 127,
		},
	},
};

struct as3676_interface {
	int index;
	u64 regs;
	int flags;
	int max_current;
	enum as3676_ctrl_value last_set_mode;
	enum as3676_ctrl_value previous_mode;
	struct led_classdev cdev;
	struct kobject kobj;
};

enum suspend_state {
	AS3676_NO_SUSPEND = 0,
	AS3676_SUSPENDED,
};

struct as3676_record {
	struct i2c_client *client;
	struct as3676_interface interfaces[AS3676_INTERFACE_MAX];
	int n_interfaces;
	struct delayed_work delayed_work;
	u8 registers[AS3676_REG_MAX];
	u64 dcdcbit;
	int is_dcdc;
	enum suspend_state suspend;
	int als_connected;
	int als_wait;
	int dls_connected;
	u64 softdim_limitation;
	struct as3676_als_config als;
	struct as3676_softdim_config softdim;
	struct mutex lock;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int ovp_high;
};

#define as3676_lock(rd) mutex_lock(&(rd)->lock)
#define as3676_unlock(rd) mutex_unlock(&(rd)->lock)

static void as3676_als_set_enable(struct as3676_record *rd, u8 enable);
static void as3676_als_set_params(struct as3676_record *rd,
				struct as3676_als_config *param);

static inline u8 reg_get(struct as3676_record *rd, enum as3676_register reg)
{
	u8 ret;

	ret = rd->registers[reg];

	return ret;
}

static inline void reg_set(struct as3676_record *rd,
		enum as3676_register reg, u8 val)
{
	struct as3676_data {
		u8 addr;
		u8 value;
	} __packed;
	struct as3676_data data;
	int ret;

	data.addr = as3676_i2c_registers[reg];
	data.value = val;
	ret = i2c_master_send(rd->client, (u8 *)&data, sizeof(data));
	if (ret != sizeof(data))
		dev_err(&rd->client->dev, "send i2c failed\n");
	else
		rd->registers[reg] = val;
}

static void as3676_als_delayed_worker(struct work_struct *work)
{
	struct as3676_record *rd;
	u8 val;

	rd = container_of(work, struct as3676_record, delayed_work.work);

	as3676_lock(rd);
	val = reg_get(rd, AS3676_REG_CTRL);

	if (val) {
		as3676_als_set_enable(rd, 1);
		reg_set(rd, AS3676_ADC_CTRL, rd->als.source);
	}
	as3676_unlock(rd);
}

static void as3676_set_amb(struct as3676_record *rd, enum as3676_register reg,
	int group)
{
	enum as3676_register amb_reg;
	int off_bits;
	int amb_val;

	amb_reg = as3676_sink[reg].amb;
	off_bits = as3676_sink[reg].lower_bit;


	amb_val = reg_get(rd, amb_reg) & ~(AS3676_AMB_MASK << off_bits);
	amb_val |= group << off_bits;

	reg_set(rd, amb_reg, amb_val);
}

static void as3676_set_interface_amb(struct as3676_record *rd,
		struct as3676_interface *intf, int group)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(as3676_sink); ++i) {
		if (intf->regs & ((u64)1 << i))
			as3676_set_amb(rd, i, group);
	}
}

static void as3676_set_als_config(struct as3676_record *rd,
		const struct as3676_als_config *config,
		struct as3676_interface *intf)
{
	enum as3676_amb_value group;

	switch (intf->flags & AS3676_FLAG_ALS_MASK) {
	case AS3676_FLAG_ALS_GROUP1:
		group = AS3676_AMB_GROUP_1;
		break;
	case AS3676_FLAG_ALS_GROUP2:
		group = AS3676_AMB_GROUP_2;
		break;
	case AS3676_FLAG_ALS_GROUP3:
		group = AS3676_AMB_GROUP_3;
		break;
	default:
		group = AS3676_AMB_OFF;
		break;
	}

	reg_set(rd, AS3676_ADC_CTRL, config->source);

	switch (group) {
	case AS3676_AMB_GROUP_1:
		reg_set(rd, AS3676_GROUP_1_Y0, config->curve[group].y0);
		reg_set(rd, AS3676_GROUP_1_Y3, config->curve[group].y3);
		reg_set(rd, AS3676_GROUP_1_X1, config->curve[group].x1);
		reg_set(rd, AS3676_GROUP_1_K1, config->curve[group].k1);
		reg_set(rd, AS3676_GROUP_1_X2, config->curve[group].x2);
		reg_set(rd, AS3676_GROUP_1_K2, config->curve[group].k2);
		break;
	case AS3676_AMB_GROUP_2:
		reg_set(rd, AS3676_GROUP_2_Y0, config->curve[group].y0);
		reg_set(rd, AS3676_GROUP_2_Y3, config->curve[group].y3);
		reg_set(rd, AS3676_GROUP_2_X1, config->curve[group].x1);
		reg_set(rd, AS3676_GROUP_2_K1, config->curve[group].k1);
		reg_set(rd, AS3676_GROUP_2_X2, config->curve[group].x2);
		reg_set(rd, AS3676_GROUP_2_K2, config->curve[group].k2);
		break;
	case AS3676_AMB_GROUP_3:
		reg_set(rd, AS3676_GROUP_3_Y0, config->curve[group].y0);
		reg_set(rd, AS3676_GROUP_3_Y3, config->curve[group].y3);
		reg_set(rd, AS3676_GROUP_3_X1, config->curve[group].x1);
		reg_set(rd, AS3676_GROUP_3_K1, config->curve[group].k1);
		reg_set(rd, AS3676_GROUP_3_X2, config->curve[group].x2);
		reg_set(rd, AS3676_GROUP_3_K2, config->curve[group].k2);
		break;
	default:
		break;
	}

	as3676_set_interface_amb(rd, intf, group);
}

static void as3676_get_als_config(struct as3676_record *rd,
		struct as3676_als_curve *curve, enum as3676_amb_value group)
{
	switch (group) {
	case AS3676_AMB_GROUP_1:
		curve->y0 = reg_get(rd, AS3676_GROUP_1_Y0);
		curve->y3 = reg_get(rd, AS3676_GROUP_1_Y3);
		curve->x1 = reg_get(rd, AS3676_GROUP_1_X1);
		curve->k1 = reg_get(rd, AS3676_GROUP_1_K1);
		curve->x2 = reg_get(rd, AS3676_GROUP_1_X2);
		curve->k2 = reg_get(rd, AS3676_GROUP_1_K2);
		break;
	case AS3676_AMB_GROUP_2:
		curve->y0 = reg_get(rd, AS3676_GROUP_2_Y0);
		curve->y3 = reg_get(rd, AS3676_GROUP_2_Y3);
		curve->x1 = reg_get(rd, AS3676_GROUP_2_X1);
		curve->k1 = reg_get(rd, AS3676_GROUP_2_K1);
		curve->x2 = reg_get(rd, AS3676_GROUP_2_X2);
		curve->k2 = reg_get(rd, AS3676_GROUP_2_K2);
		break;
	case AS3676_AMB_GROUP_3:
		curve->y0 = reg_get(rd, AS3676_GROUP_3_Y0);
		curve->y3 = reg_get(rd, AS3676_GROUP_3_Y3);
		curve->x1 = reg_get(rd, AS3676_GROUP_3_X1);
		curve->k1 = reg_get(rd, AS3676_GROUP_3_K1);
		curve->x2 = reg_get(rd, AS3676_GROUP_3_X2);
		curve->k2 = reg_get(rd, AS3676_GROUP_3_K2);
		break;
	default:
		break;
	}
}

static void as3676_set_curr_mode(struct as3676_record *rd,
		enum as3676_register reg, enum led_brightness value, int flags,
		int is_pattern)
{
	enum as3676_register ctrl_reg;
	int off_bits;
	u8 ctrl_val;

	ctrl_reg = as3676_sink[reg].ctrl;
	off_bits = as3676_sink[reg].lower_bit;

	ctrl_val = reg_get(rd, ctrl_reg);
	ctrl_val &= ~(AS3676_CTRL_MASK << off_bits);
	if (value == LED_OFF)
		ctrl_val |= (AS3676_CTRL_OFF << off_bits);
	else if (flags & AS3676_FLAG_PWM_CTRL)
		ctrl_val |= (AS3676_CTRL_PWM << off_bits);
	else if (as3676_sink[reg].flags & AS3676_FLAG_EXT_CURR)
		ctrl_val |= (AS3676_CTRL_EXT_CURR << off_bits);
	else if (is_pattern) /* don't skip pattern */
		ctrl_val |= (AS3676_CTRL_PATTERN << off_bits);
	else
		ctrl_val |= (AS3676_CTRL_ON << off_bits);
	reg_set(rd, ctrl_reg, ctrl_val);

}

static void as3676_set_brightness(struct as3676_record *rd,
		enum as3676_register reg, enum led_brightness value, int flags,
		int is_pattern)
{
	enum as3676_register ctrl_reg;
	int off_bits;
	u8 ctrl_val;

	if (as3676_sink[reg].flags & AS3676_FLAG_DCDC_CTRL &&
			rd->suspend == AS3676_SUSPENDED)
		return;

	if (value > LED_FULL)
		value = LED_FULL;
	if (value < LED_OFF)
		value = LED_OFF;

	if (rd->dls_connected && flags & AS3676_FLAG_DLS) {
		ctrl_reg = as3676_sink[reg].dls;
		off_bits = as3676_sink[reg].dls_bit;
		ctrl_val = reg_get(rd, ctrl_reg);
		ctrl_val |= (1 << off_bits);
		reg_set(rd, ctrl_reg, ctrl_val);
	}

#ifdef CONFIG_LEDS_AS3676_HW_BLINK
	if (as3676_sink[reg].flags & AS3676_FLAG_EXT_CURR &&
			value == LED_OFF) {
		ctrl_val = reg_get(rd, AS3676_REG_PATTERN_CTRL);
		ctrl_reg = as3676_sink[reg].pattern;
		off_bits = as3676_sink[reg].pattern_bit;

		ctrl_val &= ~(1 << off_bits);
		reg_set(rd, AS3676_REG_PATTERN_CTRL, ctrl_val);
	}
#endif

	if (value)
		reg_set(rd, reg, value);

	as3676_set_curr_mode(rd, reg, value, flags, is_pattern);

	if (!value)
		reg_set(rd, reg, value);
}

static enum as3676_ctrl_value get_mode(struct as3676_record *rd,
		enum as3676_register reg)
{
	enum as3676_register ctrl_reg = as3676_sink[reg].ctrl;
	int off_bits = as3676_sink[reg].lower_bit;
	u8 ctrl_val = reg_get(rd, ctrl_reg);
	return (ctrl_val >> off_bits) & AS3676_CTRL_MASK;
}

static int as3676_get_brightness(struct as3676_interface *intf,
		struct as3676_record *rd, enum as3676_register reg)
{
	int	value = reg_get(rd, reg);
	if (intf->max_current)
		value = value * AS3676_MAX_CURRENT / intf->max_current;
	return value;
}

static int as3676_brightness_to_current(struct as3676_interface *intf)
{
	int value = intf->cdev.brightness;
	if (intf->max_current)
		value = (value * intf->max_current) / AS3676_MAX_CURRENT;

	if ((value == 0) && (intf->cdev.brightness != LED_OFF))
		value = 1;
	return value;
}

static void as3676_set_interface_brightness(struct as3676_interface *intf,
		enum led_brightness value)
{
	int i;
	struct as3676_record *rd;
	int is_set_yet = 0;	/* avoid multiple set for buttons */
	int is_pattern = 0;

	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	if (intf->last_set_mode == AS3676_CTRL_PATTERN) {
		if (value == LED_OFF) {
			/*
			 * when blink off, brightness is rearranged
			 * with current brightness and pwm_code
			 */
			int brightness = 0;
			int pwm_code = 0;
			for (i = 0; i < ARRAY_SIZE(as3676_sink); ++i) {
				if (intf->regs & ((u64)1 << i))
					brightness
					 = as3676_get_brightness(intf, rd, i);
			}
			pwm_code = i2c_smbus_read_byte_data(rd->client, 0x17);
			brightness = brightness * pwm_code / 0xff;
			value = brightness & 0xff;
		} else {
			is_pattern = 1;
		}
	}

	intf->cdev.brightness = value;
	value = as3676_brightness_to_current(intf);

	for (i = 0; i < ARRAY_SIZE(as3676_sink); ++i) {
		if (intf->regs & ((u64)1 << i)) {
			if (!is_set_yet) {
				intf->previous_mode = intf->last_set_mode;
				is_set_yet = 1;
			}
			as3676_set_brightness(rd, i, value, intf->flags,
				is_pattern);
			intf->last_set_mode = get_mode(rd, i);
		}
	}
}

static void as3676_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;

	intf = container_of(led_cdev, struct as3676_interface, cdev);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);
	dev_dbg(led_cdev->dev, "brightness i=%d, on=%d\n", intf->index, value);
	as3676_lock(rd);
	as3676_set_interface_brightness(intf, value);
	as3676_unlock(rd);
}


#ifdef CONFIG_LEDS_AS3676_HW_BLINK
#define ON_TIME_SIZE	10
#define OFF_TIME_SIZE	18

static void as3676_set_blink(struct as3676_record *rd,
		enum as3676_register reg, unsigned long *on, unsigned long *off,
		int value, int is_set_pwm_code, int pwm_value)
{
	enum as3676_register ctrl_reg;
	int off_bits;
	u8 ctrl_val, curr_val;
	int i, y;

	/*
	 * Blinking period for as3676 consists of:
	 *  [delay value(led off)] [32bit pattern (0-led off, 1-led on)]
	 *   delay value is in range 0..7 seconds (x8 for slow clock)
	 *   each bit in pattern has duration 31ms (250ms for slow clock)
	 *
	 *  as3676_patterns table contains most suitable predefined settings for
	 *  all periods which can be configured
	 */
	static unsigned long on_time_range[ON_TIME_SIZE] = {
		100, 200, 500, 1000, 1500, 2000, 3000, 4000, 6000, 8000
	};

	static unsigned long off_time_range[OFF_TIME_SIZE] = {
		100, 200, 500, 1000, 1500, 2000, 3000, 4000, 6000, 8000,
		10000, 15000, 20000, 25000, 30000, 40000, 50000, 60000
	};

	static struct as3676_pattern {
		unsigned int pattern;
		int delay;
		int slow;
		int dim_speed;
	} as3676_patterns[OFF_TIME_SIZE][ON_TIME_SIZE] = {
		/* off_time_range 100 */
		{ {0x55555555, 0, 0, -1}, {0xFCFCFCFC, 0, 0, 6},
		  {0x1FFF1FFF, 0, 0, 6}, {0x1FFFFFFF, 0, 0, 6},
		  {0x7F7F7F7F, 0, 1, 7}, {0x7F7F7F7F, 0, 1, 7},
		  {0x7FFF7FFF, 0, 1, 7}, {0x7FFF7FFF, 0, 1, 7},
		  {0x7FFFFFFF, 0, 1, 7}, {0x7FFFFFFF, 0, 1, 7} },
		/* off_time_range 200 */
		{ {0x03030303, 0, 0, 6}, {0x00FF00FF, 0, 0, 7},
		  {0x000FFFFF, 0, 0, 7}, {0x03FFFFFF, 0, 0, 7},
		  {0x7F7F7F7F, 0, 1, 7}, {0x7F7F7F7F, 0, 1, 7},
		  {0x7FFF7FFF, 0, 1, 7}, {0x7FFF7FFF, 0, 1, 7},
		  {0x7FFFFFFF, 0, 1, 7}, {0x7FFFFFFF, 0, 1, 7} },
		/* off_time_range 500 */
		{ {0x00070007, 0, 0, 6}, {0x00000FFF, 0, 0, 7},
		  {0x0000FFFF, 0, 0, 5}, {0x1F1F1F1F, 0, 1, 2},
		  {0x3F3F3F3F, 0, 1, 5}, {0x3F3F3F3F, 0, 1, 5},
		  {0x3FFF3FFF, 0, 1, 5}, {0x3FFF3FFF, 0, 1, 5},
		  {0x1FFFFFFF, 0, 1, 2}, {0x3FFFFFFF, 0, 1, 5} },
		/* off_time_range 1000 */
		{ {0x00000007, 0, 0, 6}, {0x0000003F, 0, 0, 7},
		  {0x07070707, 0, 1, 2}, {0xFFFFFFFF, 1, 0, 2},
		  {0x1F1F1F1F, 0, 1, 2}, {0x03FF03FF, 0, 1, 3},
		  {0x0FFF0FFF, 0, 1, 2}, {0x1FFF1FFF, 0, 1, 2},
		  {0x07FFFFFF, 0, 1, 3}, {0x1FFFFFFF, 0, 1, 2} },
		/* off_time_range 1500 */
		{ {0x0000000F, 1, 0, 6}, {0x0000007F, 1, 0, 7},
		  {0x03030303, 0, 1, 5}, {0x07070707, 0, 1, 2},
		  {0x00FF00FF, 0, 1, 3}, {0x01FF01FF, 0, 1, 3},
		  {0x07FF07FF, 0, 1, 3}, {0x1FFF1FFF, 0, 1, 5},
		  {0x01FFFFFF, 0, 1, 3}, {0x0FFFFFFF, 0, 1, 2} },
		/* off_time_range 2000 */
		{ {0x0000000F, 1, 0, 6}, {0x0000007F, 1, 0, 7},
		  {0x03030303, 0, 1, 5}, {0xFFFFFFFF, 2, 0, 2},
		  {0x007F007F, 0, 1, 3}, {0x00FF00FF, 0, 1, 3},
		  {0x03FF03FF, 0, 1, 3}, {0x0003FFFF, 0, 1, 3},
		  {0x00FFFFFF, 0, 1, 3}, {0xFFFFFFFF, 1, 1, 1} },
		/* off_time_range 3000 */
		{ {0x0000000F, 2, 0, 6}, {0x0000007F, 2, 0, 5},
		  {0x00030003, 0, 1, 5}, {0xFFFFFFFF, 3, 0, 2},
		  {0x001F001F, 0, 1, 3}, {0x003F003F, 0, 1, 3},
		  {0x03FF03FF, 0, 1, 3}, {0x0003FFFF, 0, 1, 3},
		  {0x00FFFFFF, 0, 1, 3}, {0xFFFFFFFF, 1, 1, 1} },
		/* off_time_range 4000 */
		{ {0x0000000F, 3, 0, 6}, {0x0000007F, 3, 0, 7},
		  {0x0000FFFF, 4, 0, 5}, {0xFFFFFFFF, 4, 0, 2},
		  {0x000001FF, 0, 1, 3}, {0x000003FF, 0, 1, 3},
		  {0x00003FFF, 0, 1, 3}, {0x0000FFFF, 0, 1, 1},
		  {0x00FFFFFF, 0, 1, 3}, {0xFFFFFFFF, 1, 1, 1} },
		/* off_time_range 6000 */
		{ {0x0000000F, 5, 0, 6}, {0x0000007F, 5, 0, 7},
		  {0x0000FFFF, 6, 0, 5}, {0xFFFFFFFF, 6, 0, 2},
		  {0x0000003F, 0, 1, 3}, {0x000000FF, 0, 1, 3},
		  {0x00000FFF, 0, 1, 3}, {0x0000FFFF, 0, 1, 3},
		  {0x0FFFFFFF, 1, 1, 3}, {0xFFFFFFFF, 1, 1, 3} },
		/* off_time_range 8000 */
		{ {0x0000000F, 7, 0, 6}, {0x0000007F, 7, 0, 7},
		  {0x0000FFFF, 7, 0, 5}, {0xFFFFFFFF, 7, 0, 2},
		  {0x0000003F, 0, 1, 3}, {0x000000FF, 0, 1, 3},
		  {0x00000FFF, 0, 1, 3}, {0x0000FFFF, 0, 1, 1},
		  {0x00FFFFFF, 1, 1, 1}, {0xFFFFFFFF, 1, 1, 1} },
		/* off_time_range 10000 */
		{ {0x0000000F, 7, 0, 6}, {0x0000007F, 7, 0, 7},
		  {0x0000FFFF, 7, 0, 5}, {0xFFFFFFFF, 7, 0, 2},
		  {0x0000003F, 1, 1, 3}, {0x000000FF, 1, 1, 3},
		  {0x00000FFF, 1, 1, 3}, {0x0000FFFF, 1, 1, 1},
		  {0x00FFFFFF, 1, 1, 1}, {0xFFFFFFFF, 1, 1, 1} },
		/* off_time_range 15000 */
		{ {0x00000001, 1, 1, 7}, {0x00000001, 1, 1, 7},
		  {0x00000003, 1, 1, 5}, {0x0000000F, 1, 1, 2},
		  {0x0000003F, 1, 1, 3}, {0x000000FF, 1, 1, 3},
		  {0x00000FFF, 1, 1, 3}, {0x0000FFFF, 2, 1, 1},
		  {0x00FFFFFF, 2, 1, 1}, {0xFFFFFFFF, 2, 1, 1} },
		/* off_time_range 20000 */
		{ {0x00000001, 2, 1, 7}, {0x00000001, 2, 1, 7},
		  {0x00000003, 2, 1, 5}, {0x0000000F, 2, 1, 2},
		  {0x0000003F, 2, 1, 3}, {0x000000FF, 2, 1, 3},
		  {0x00000FFF, 2, 1, 3}, {0x0000FFFF, 2, 1, 1},
		  {0x00FFFFFF, 2, 1, 1}, {0xFFFFFFFF, 3, 1, 1} },
		/* off_time_range 25000 */
		{ {0x00000001, 2, 1, 7}, {0x00000001, 2, 1, 7},
		  {0x00000003, 2, 1, 5}, {0x0000000F, 2, 1, 2},
		  {0x0000003F, 2, 1, 3}, {0x000000FF, 2, 1, 3},
		  {0x00000FFF, 2, 1, 3}, {0x0000FFFF, 3, 1, 1},
		  {0x00FFFFFF, 3, 1, 1}, {0xFFFFFFFF, 3, 1, 1} },
		/* off_time_range 30000 */
		{ {0x00000001, 3, 1, 7}, {0x00000001, 3, 1, 7},
		  {0x00000003, 3, 1, 5}, {0x0000000F, 3, 1, 2},
		  {0x0000003F, 3, 1, 3}, {0x000000FF, 3, 1, 3},
		  {0x00000FFF, 3, 1, 3}, {0x0000FFFF, 3, 1, 1},
		  {0x00FFFFFF, 4, 1, 1}, {0xFFFFFFFF, 4, 1, 1} },
		/* off_time_range 40000 */
		{ {0x00000001, 4, 1, 7}, {0x00000001, 4, 1, 7},
		  {0x00000003, 4, 1, 5}, {0x0000000F, 4, 1, 2},
		  {0x0000003F, 4, 1, 3}, {0x000000FF, 4, 1, 3},
		  {0x00000FFF, 4, 1, 3}, {0x0000FFFF, 5, 1, 1},
		  {0x00FFFFFF, 5, 1, 1}, {0xFFFFFFFF, 5, 1, 1} },
		/* off_time_range 50000 */
		{ {0x00000001, 5, 1, 7}, {0x00000001, 5, 1, 7},
		  {0x00000003, 6, 1, 5}, {0x0000000F, 6, 1, 2},
		  {0x0000003F, 6, 1, 3}, {0x000000FF, 6, 1, 3},
		  {0x00000FFF, 6, 1, 3}, {0x0000FFFF, 6, 1, 1},
		  {0x00FFFFFF, 6, 1, 1}, {0xFFFFFFFF, 6, 1, 1} },
		/* off_time_range 60000 */
		{ {0x00000001, 7, 1, 7}, {0x00000001, 7, 1, 7},
		  {0x00000003, 7, 1, 5}, {0x0000000F, 7, 1, 2},
		  {0x0000003F, 7, 1, 3}, {0x000000FF, 7, 1, 3},
		  {0x00000FFF, 7, 1, 3}, {0x0000FFFF, 7, 1, 1},
		  {0x00FFFFFF, 7, 1, 1}, {0xFFFFFFFF, 7, 1, 1} }
	};
	struct as3676_pattern *patt_def;
	int pattern_ctrl_val = 0;

	if (as3676_sink[reg].flags & AS3676_FLAG_DCDC_CTRL) {
		dev_err(&rd->client->dev,
				"Request for blinking on DCDC ctrl led\n");
		return;
	}

	for (i = 0; i < ON_TIME_SIZE - 1; ++i) {
		if (*on <= on_time_range[i])
			break;
	}

	for (y = 0; y < OFF_TIME_SIZE - 1; ++y) {
		if (*off <= off_time_range[y])
			break;
	}

	patt_def = &as3676_patterns[y][i];

	curr_val = reg_get(rd, AS3676_REG_GPIO_CURR);
	curr_val &= (1 << 7);
	reg_set(rd, AS3676_REG_GPIO_CURR, curr_val |
			((patt_def->delay & 0x4) << 2) |
			(patt_def->slow << 6));

	reg_set(rd, AS3676_REG_PATTERN_DATA_0,
			patt_def->pattern & 0xff);
	reg_set(rd, AS3676_REG_PATTERN_DATA_1,
			(patt_def->pattern >>  8) & 0xff);
	reg_set(rd, AS3676_REG_PATTERN_DATA_2,
			(patt_def->pattern >> 16) & 0xff);
	reg_set(rd, AS3676_REG_PATTERN_DATA_3,
			(patt_def->pattern >> 24) & 0xff);

	/* clear pattern_delay and softdim_pattern bit */
	ctrl_val = reg_get(rd, AS3676_REG_PATTERN_CTRL)
					& ~(1 << 3 | 0x3 << 1);
	ctrl_val |= (patt_def->delay & 0x3) << 1;
	if (rd->softdim.enable && !rd->softdim_limitation &&
	    patt_def->dim_speed >= 0) {
		if (is_set_pwm_code)
			reg_set(rd, AS3676_REG_PWM_CODE, pwm_value);
		/* If PWM is used, do not use soft dim */
		ctrl_val |= 1 << 3;
		reg_set(rd, AS3676_REG_PWM_CTRL, (patt_def->dim_speed << 3));
	} else {
		is_set_pwm_code = 0;
		reg_set(rd, AS3676_REG_PWM_CTRL, 0 << 3 | 1 << 1);
	}
	rd->softdim.dim_speed = patt_def->dim_speed;
	pattern_ctrl_val = ctrl_val;

	if (as3676_sink[reg].flags & AS3676_FLAG_EXT_CURR) {
		ctrl_reg = as3676_sink[reg].pattern;
		off_bits = as3676_sink[reg].pattern_bit;

		ctrl_val = reg_get(rd, ctrl_reg);
		reg_set(rd, ctrl_reg, ctrl_val | (1 << off_bits));
	}

	ctrl_reg = as3676_sink[reg].ctrl;
	off_bits = as3676_sink[reg].lower_bit;

	ctrl_val = reg_get(rd, ctrl_reg) & ~(AS3676_CTRL_MASK << off_bits);
	reg_set(rd, ctrl_reg, ctrl_val | (AS3676_CTRL_PATTERN << off_bits));

	reg_set(rd, AS3676_REG_PATTERN_CTRL, pattern_ctrl_val);
	/* If we have now the first led running on a pattern,
	   make sure values are correct */
	if (is_set_pwm_code)
		/* Kickstart pattern generator to correctly use pattern_delay */
		reg_set(rd, AS3676_REG_PATTERN_CTRL,
			reg_get(rd, AS3676_REG_PATTERN_CTRL));

	reg_set(rd, reg, value);
}

static int is_need_set_pwm_code(struct as3676_interface *intf)
{
	return ((intf->previous_mode != AS3676_CTRL_PATTERN)
		&& (intf->last_set_mode != AS3676_CTRL_PATTERN)) ? 1 : 0;
}

static int isRGB(const struct led_classdev *cdev)
{
	if (cdev->name == NULL)
		return 0;
	else if ((strncmp(cdev->name, "red", 4) == 0)
		|| (strncmp(cdev->name, "green", 6) == 0)
		|| (strncmp(cdev->name, "blue", 5) == 0))
		return 1;
	else
		return 0;
}


static int is_RGB_off(const struct as3676_record *rd)
{
	int i;
	for (i = 0; i < AS3676_INTERFACE_MAX; ++i) {
		if (isRGB(&rd->interfaces[i].cdev)
			&& (rd->interfaces[i].cdev.brightness != LED_OFF))
			return 0;
	}
	return 1;
}

static void as3676_set_interface_blink(struct as3676_interface *intf,
		unsigned long *on, unsigned long *off)
{
	struct as3676_record *rd;
	int value;
	int i;
	int is_set_yet = 0;	/* avoid multiple set for buttons */
	int is_set_pwm_code = 0;
	int pwm_value = intf->cdev.brightness;

	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	if (*on == 0 && *off == 0)
		return;

	value = intf->cdev.brightness;

	if (intf->max_current)
		value = (value * intf->max_current) / AS3676_MAX_CURRENT;

	for (i = 0; i < ARRAY_SIZE(as3676_sink); ++i) {
		if (intf->regs & ((u64)1 << i)) {
			if (!is_set_yet) {
				is_set_pwm_code = is_need_set_pwm_code(intf);
				intf->previous_mode = intf->last_set_mode;
				intf->last_set_mode = AS3676_CTRL_PATTERN;
				is_set_yet = 1;
				if (isRGB(&intf->cdev))
					pwm_value = is_RGB_off(rd) ? 0 : 0xff;
			}
			as3676_set_blink(rd, i, on, off, value, is_set_pwm_code,
				pwm_value);
		}
	}
}

static int as3676_blink(struct led_classdev *led_cdev,
		unsigned long *on, unsigned long *off)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;

	intf = container_of(led_cdev, struct as3676_interface, cdev);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);
	dev_dbg(led_cdev->dev, "blink i=%d, on=%ld, off=%ld\n",
			intf->index, *on, *off);
	as3676_lock(rd);
	as3676_set_interface_blink(intf, on, off);
	as3676_unlock(rd);
	return 0;
}
#endif

static ssize_t as3676_als_value_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	ssize_t ret;
	u8 val;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	as3676_lock(rd);
	ret = i2c_smbus_read_i2c_block_data(rd->client, 0x93, 1, &val);
	if (ret < 0) {
		as3676_unlock(rd);
		return ret;
	}
	as3676_unlock(rd);

	snprintf(buf, PAGE_SIZE, "%u\n", val);
	ret = strnlen(buf, PAGE_SIZE) + 1;

	return ret;
}

static void as3676_als_set_adc_ctrl(struct as3676_record *rd)
{
	u8 val;
	int status;
	/* Sometimes AS3676 has increased 200uA current consumption in standby
	 * and need to handle it by sw. */
	reg_set(rd, AS3676_ADC_CTRL, 0x80);
	status = i2c_smbus_read_i2c_block_data(rd->client, AS3676_ADC_CTRL,
					       1, &val);
	if (status < 0)
		dev_err(&rd->client->dev, "%s:I2C read error:%d\n",
			 __func__, status);
}

static void as3676_als_set_enable(struct as3676_record *rd, u8 enable)
{
	if (enable)
		reg_set(rd, AS3676_AMB_CTRL, (rd->als.gain << 1) | 0x01);
	else
		reg_set(rd, AS3676_AMB_CTRL, 0x00);
}

static ssize_t as3676_als_enable_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	unsigned long enable;
	int ret;
	u8 val;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	ret = strict_strtoul(buf, 0, &enable);
	if (ret < 0 || (enable != 1 && enable != 0))
		return -EINVAL;

	as3676_lock(rd);
	val = reg_get(rd, AS3676_AMB_CTRL) & 0x01;

	if (enable != val) {
		as3676_als_set_enable(rd, enable);

		if (enable) {
			as3676_als_set_params(rd, &rd->als);
			reg_set(rd, AS3676_ADC_CTRL, rd->als.source);
		} else {
			struct as3676_als_config param;

			memset(&param, 0x00, sizeof(struct as3676_als_config));
			as3676_als_set_params(rd, &param);
			as3676_als_set_adc_ctrl(rd);
		}
	}

	as3676_unlock(rd);
	return count;
}


static ssize_t as3676_als_enable_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	u8 val;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	as3676_lock(rd);
	val = reg_get(rd, AS3676_AMB_CTRL);
	as3676_unlock(rd);

	return snprintf(buf, PAGE_SIZE, "%u\n", (val & 0x01) ? 1 : 0);
}

static ssize_t as3676_als_curve_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	int ret;
	struct as3676_als_curve curve;
	enum as3676_amb_value als_group;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	ret = sscanf(buf, "%10u,%10u,%10u,%10u,%10u,%10u,%10u", &als_group,
			&curve.y0, &curve.y3, &curve.k1,
			&curve.k2, &curve.x1, &curve.x2);
	if (ret != 7)
		return -EINVAL;

	if (als_group & ~AS3676_AMB_MASK)
		return -EINVAL;
	/* if any of the values are > 255 or < 0, error out */
	if ((curve.y0 | curve.y3) & ~0xff)
		return -EINVAL;
	if ((curve.k1 | curve.k2) & ~0xff)
		return -EINVAL;
	if ((curve.x1 | curve.x2) & ~0xff)
		return -EINVAL;

	as3676_lock(rd);
	intf->flags &= ~AS3676_FLAG_ALS_MASK;
	switch (als_group) {
	case AS3676_AMB_GROUP_1:
		intf->flags |= AS3676_FLAG_ALS_GROUP1;
		break;
	case AS3676_AMB_GROUP_2:
		intf->flags |= AS3676_FLAG_ALS_GROUP2;
		break;
	case AS3676_AMB_GROUP_3:
		intf->flags |= AS3676_FLAG_ALS_GROUP3;
		break;
	default:
		break;
	}

	memcpy(&rd->als.curve[als_group], &curve,
				sizeof(struct as3676_als_curve));

	as3676_set_als_config(rd, &rd->als, intf);
	as3676_unlock(rd);

	return count;
}

static ssize_t as3676_als_curve_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	struct as3676_als_curve curve;
	char *curve_str = buf;
	int length = 0;
	int i;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	as3676_lock(rd);
	length = snprintf(curve_str, PAGE_SIZE, "curve,y0,y3,k1,k2,x1,x2\n");
	curve_str += length;

	for (i = 1; i < AS3676_AMB_MAX; i++) {
		memset(&curve, 0x00, sizeof(struct as3676_als_curve));
		as3676_get_als_config(rd, &curve, i);

		length = snprintf(curve_str, PAGE_SIZE - (curve_str - buf),
			"group_%u,%u,%u,%u,%u,%u,%u\n",
			i, curve.y0, curve.y3, curve.k1,
			curve.k2, curve.x1, curve.x2);
		curve_str += length;
	}
	as3676_unlock(rd);

	return strnlen(buf, PAGE_SIZE);
}

static void as3676_als_set_params(struct as3676_record *rd,
				struct as3676_als_config *param)
{
	u8 ctrl = reg_get(rd, AS3676_AMB_CTRL);

	/* amb_gain 0x90 */
	ctrl &= ~0x06;
	reg_set(rd, AS3676_AMB_CTRL, ((param->gain << 1) | ctrl));
	/* als_filter 0x91 */
	reg_set(rd, AS3676_AMB_FILTER,
		(param->filter_up | (param->filter_down << 4)));
	/* als offs 0x92 */
	reg_set(rd, AS3676_AMB_OFFSET, param->offset);
}

static ssize_t as3676_als_params_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t size)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;
	struct as3676_als_config param;
	int count;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	count = sscanf(buf, "%10u,%10u,%10u,%10u", &param.gain,
		&param.filter_up, &param.filter_down, &param.offset);
	if (count != 4)
		return -EINVAL;

	if ((param.gain >= AS3676_GAIN_MAX) ||
		(param.filter_up >= AS3676_FILTER_MAX) ||
		(param.filter_down >= AS3676_FILTER_MAX) ||
		(param.offset >= 0xFF))
		return -EINVAL;

	as3676_lock(rd);
	rd->als.gain = param.gain;
	rd->als.filter_up = param.filter_up;
	rd->als.filter_down = param.filter_down;
	rd->als.offset = param.offset;

	as3676_als_set_params(rd, &rd->als);
	as3676_unlock(rd);

	return size;
}

static ssize_t as3676_als_params_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct as3676_interface *intf;
	struct as3676_record *rd;

	intf = container_of(kobj, struct as3676_interface, kobj);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	return snprintf(buf, PAGE_SIZE, "gain,filter_up,filter_down,offset\n"
		"%u,%u,%u,%u\n", rd->als.gain, rd->als.filter_up,
		rd->als.filter_down, rd->als.offset);
}

static struct kobj_attribute as3676_als_attr_value =
	__ATTR(value, 0444, as3676_als_value_show, NULL);
static struct kobj_attribute as3676_als_attr_enable =
	__ATTR(enable, 0644, as3676_als_enable_show, as3676_als_enable_store);
static struct kobj_attribute as3676_als_attr_curve =
	__ATTR(curve, 0644, as3676_als_curve_show, as3676_als_curve_store);
static struct kobj_attribute as3676_als_attr_params =
	__ATTR(params, 0644, as3676_als_params_show, as3676_als_params_store);

static ssize_t as3676_max_current_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct as3676_record *rd;
	struct as3676_interface *intf;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	intf = container_of(led_cdev, struct as3676_interface, cdev);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	return snprintf(buf, PAGE_SIZE, "%d\n", intf->max_current);
}

static ssize_t as3676_max_current_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	unsigned long curr_val;
	struct as3676_record *rd;
	struct as3676_interface *intf;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct as3676_platform_data *pdata;

	intf = container_of(led_cdev, struct as3676_interface, cdev);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	ret = strict_strtoul(buf, 10, &curr_val);

	if (ret != 0 || curr_val == 0)
		return -EINVAL;

	as3676_lock(rd);
	pdata = rd->client->dev.platform_data;

	if (curr_val > pdata->leds[intf->index].max_current)
		curr_val = pdata->leds[intf->index].max_current;

	intf->max_current = (int)curr_val;
	as3676_unlock(rd);

	return size;
}

static void as3676_set_softdim(struct as3676_record *rd)
{
	u8 ctrl_val;

	ctrl_val = reg_get(rd, AS3676_REG_PATTERN_CTRL);
	if (rd->softdim.enable && !rd->softdim_limitation &&
	    rd->softdim.dim_speed >= 0) {
		reg_set(rd, AS3676_REG_PWM_CTRL, (rd->softdim.dim_speed << 3));
		reg_set(rd, AS3676_REG_PATTERN_CTRL, ctrl_val | (1 << 3));
	} else {
		/* If PWM is used, do not use soft dim */
		reg_set(rd, AS3676_REG_PWM_CTRL, 0 << 3 | 1 << 1);
		reg_set(rd, AS3676_REG_PATTERN_CTRL, ctrl_val & ~(1 << 3));
	}
}

static void as3676_als_set_mode(struct as3676_record *rd,
			enum as3676_register reg, u8 mode)
{
	enum as3676_register ctrl_reg;
	int off_bits;
	u8 ctrl_val;

	ctrl_reg = as3676_sink[reg].ctrl;
	off_bits = as3676_sink[reg].lower_bit;
	ctrl_val = reg_get(rd, ctrl_reg) & ~(AS3676_CTRL_MASK << off_bits);

	reg_set(rd, ctrl_reg, ctrl_val | (mode << off_bits));
}

static ssize_t as3676_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret, i;
	unsigned long mode;
	struct as3676_record *rd;
	struct as3676_interface *intf;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	intf = container_of(led_cdev, struct as3676_interface, cdev);
	rd = container_of(intf, struct as3676_record, interfaces[intf->index]);

	ret = strict_strtoul(buf, 10, &mode);

	if ((ret != 0) || (mode > AS3676_CTRL_PWM))
		return -EINVAL;

	as3676_lock(rd);
	intf->flags &= ~AS3676_FLAG_PWM_CTRL;
	if (mode == AS3676_CTRL_PWM)
		intf->flags |= AS3676_FLAG_PWM_CTRL;

	for (i = 0; i < ARRAY_SIZE(as3676_sink); ++i) {
		if (intf->regs & ((u64)1 << i)) {
			as3676_als_set_mode(rd, i, (u8)mode);
			if (intf->flags & AS3676_FLAG_PWM_CTRL ||
			    as3676_sink[i].flags & AS3676_FLAG_EXT_CURR)
				rd->softdim_limitation |= ((u64)1 << i);
			else
				rd->softdim_limitation &= ~((u64)1 << i);
		}
	}
	as3676_set_softdim(rd);
	as3676_unlock(rd);

	return size;
}

static ssize_t as3676_adc_als_value_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct as3676_record *rd = dev_get_drvdata(dev);
	struct as3676_interface *intf;
	struct led_classdev *led_cdev = dev_get_drvdata(&rd->client->dev);
	u32 adc_result;
	u8 val;
	int i;

	intf = container_of(led_cdev, struct as3676_interface, cdev);

	as3676_lock(rd);
	val = reg_get(rd, AS3676_ADC_CTRL);
	as3676_unlock(rd);
	if ((val & 0x3F) != AS3676_ALS_SOURCE_GPIO2) {
		dev_err(&rd->client->dev, "als source failed\n");
		return -EFAULT;
	}

	for (i = 0; i < AS3676_ADC_READ_RETRY_NUM; i++) {
		adc_result = i2c_smbus_read_byte_data(rd->client, 0x27);
		if (!(adc_result & 0x80))
			break;
		/* Wait for the end of the next ADC conversion cycle */
		udelay(10);
	}
	if (i >= AS3676_ADC_READ_RETRY_NUM) {
		dev_err(&rd->client->dev, "adc_result failed\n");
		return -EFAULT;
	}

	adc_result = (adc_result & 0x7F) << 3;
	adc_result |= i2c_smbus_read_byte_data(rd->client, 0x28) & 0x07;

	return snprintf(buf, PAGE_SIZE, "%u\n", adc_result);
}

static DEVICE_ATTR(max_current, 0600, as3676_max_current_show,
					as3676_max_current_store);
static DEVICE_ATTR(mode, 0200, NULL, as3676_mode_store);
static DEVICE_ATTR(adc_als_value, 0444, as3676_adc_als_value_show, NULL);

static void dummy_kobj_release(struct kobject *kobj)
{ }

static struct kobj_type dummy_kobj_ktype = {
	.release	= dummy_kobj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};

static struct attribute *as3676_als_attrs[] = {
	&as3676_als_attr_value.attr,
	&as3676_als_attr_enable.attr,
	&as3676_als_attr_curve.attr,
	&as3676_als_attr_params.attr,
	NULL
};

static struct attribute_group as3676_als_attr_group = {
	.attrs = as3676_als_attrs,
};

static int as3676_create_als_tree(struct as3676_record *rd,
		struct as3676_interface *intf)
{
	int rc;

	rc = kobject_init_and_add(&intf->kobj, &dummy_kobj_ktype,
			&intf->cdev.dev->kobj, "als");
	if (rc)
		return rc;

	rc = sysfs_create_group(&intf->kobj, &as3676_als_attr_group);
	if (rc)
		kobject_put(&intf->kobj);

	return rc;
}

static void as3676_remove_als_tree(struct as3676_record *rd,
		struct as3676_interface *intf)
{
	sysfs_remove_group(&intf->kobj, &as3676_als_attr_group);
	kobject_put(&intf->kobj);
}

static void as3676_set_curr_ctrl(struct as3676_record *rd, int enable)
{
	int i, j;
	enum led_brightness value;
	for (i = 0; i < rd->n_interfaces; ++i) {
		struct as3676_interface *intf = &rd->interfaces[i];
		for (j = 0; j < ARRAY_SIZE(as3676_sink); ++j) {
			if (intf->regs & ((u64)1 << j) &&
			   as3676_sink[j].flags & AS3676_FLAG_DCDC_CTRL) {
				if (enable) {
					value =
					  as3676_brightness_to_current(intf);
					reg_set(rd, j, value);
					as3676_set_curr_mode(rd, j,
						value, intf->flags, 0);
				} else {
					as3676_set_curr_mode(rd, j,
						LED_OFF, intf->flags, 0);
					reg_set(rd, j, LED_OFF);
				}
			}
		}
	}
}

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static int as3676_pm_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as3676_record *rd = i2c_get_clientdata(client);

	dev_info(dev, "Suspending AS3676\n");

	as3676_lock(rd);
	if (rd->is_dcdc) {
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x0c);
		as3676_set_curr_ctrl(rd, 0);
		reg_set(rd, AS3676_REG_CTRL, 0x00);

		rd->suspend = AS3676_SUSPENDED;
	} else {
		reg_set(rd, AS3676_REG_CTRL, 0x00);
	}

	if (rd->als_connected) {
		as3676_als_set_enable(rd, 0);
		as3676_als_set_adc_ctrl(rd);
	}

	as3676_unlock(rd);

	return 0;
}

static int as3676_pm_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as3676_record *rd = i2c_get_clientdata(client);

	dev_info(dev, "Resuming AS3676\n");

	as3676_lock(rd);
	if (rd->is_dcdc) {
		reg_set(rd, AS3676_REG_CTRL, 0x0d);
		usleep(AS3676_REG_CTRL_WAIT_US);
		as3676_set_curr_ctrl(rd, 1);
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x8c);
		rd->suspend = AS3676_NO_SUSPEND;
	} else {
		reg_set(rd, AS3676_REG_CTRL, 0x05);
	}

	if (rd->als_connected) {
		if (rd->als_wait) {
			schedule_delayed_work(&rd->delayed_work,
					msecs_to_jiffies(rd->als_wait));
		} else {
			as3676_als_set_enable(rd, 1);
			reg_set(rd, AS3676_ADC_CTRL, rd->als.source);
		}
	}

	as3676_unlock(rd);

	return 0;
}
#else
#define as3676_pm_suspend	NULL
#define as3676_pm_resume	NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void as3676_early_suspend(struct early_suspend *handler)
{
	struct as3676_record *rd =
		container_of(handler, struct as3676_record, early_suspend);

	dev_info(&rd->client->dev, "%s\n", __func__);

	as3676_lock(rd);
	if (rd->is_dcdc) {
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x0c);
		as3676_set_curr_ctrl(rd, 0);
		reg_set(rd, AS3676_REG_CTRL, 0x00);

		rd->suspend = AS3676_SUSPENDED;
	} else {
		reg_set(rd, AS3676_REG_CTRL, 0x00);
	}

	if (rd->als_connected) {
		as3676_als_set_enable(rd, 0);
		as3676_als_set_adc_ctrl(rd);
	}

	as3676_unlock(rd);
}

static void as3676_late_resume(struct early_suspend *handler)
{
	struct as3676_record *rd =
		container_of(handler, struct as3676_record, early_suspend);

	dev_info(&rd->client->dev, "%s\n", __func__);

	as3676_lock(rd);

	if (rd->is_dcdc) {
		reg_set(rd, AS3676_REG_CTRL, 0x0d);
		usleep(AS3676_REG_CTRL_WAIT_US);
		as3676_set_curr_ctrl(rd, 1);
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x8c);
		rd->suspend = AS3676_NO_SUSPEND;
	} else {
		reg_set(rd, AS3676_REG_CTRL, 0x05);
	}

	if (rd->als_connected) {
		if (rd->als_wait) {
			schedule_delayed_work(&rd->delayed_work,
					msecs_to_jiffies(rd->als_wait));
		} else {
			as3676_als_set_enable(rd, 1);
			reg_set(rd, AS3676_ADC_CTRL, rd->als.source);
		}
	}

	as3676_unlock(rd);
}
#endif

static void as3676_shutdown(struct i2c_client *client)
{
	struct as3676_record *rd = i2c_get_clientdata(client);
	int i;

	dev_info(&client->dev, "Shutting down AS3676\n");

	as3676_lock(rd);

	for (i = 0; i < rd->n_interfaces; ++i)
		as3676_set_interface_brightness(&rd->interfaces[i], 0);

	reg_set(rd, AS3676_REG_CTRL, 0x00);
	reg_set(rd, AS3676_MODE_SWITCH, 0x00);
	reg_set(rd, AS3676_MODE_SWITCH_2, 0x00);
	if (rd->als_connected)
		reg_set(rd, AS3676_AMB_CTRL, 0x0);

	as3676_unlock(rd);
}

static int __devexit as3676_remove(struct i2c_client *client)
{
	struct as3676_record *rd = i2c_get_clientdata(client);
	int i;

	dev_info(&client->dev, "Removing AS3676 driver\n");

	for (i = 0; i < rd->n_interfaces; ++i) {
		struct as3676_interface *intf = &rd->interfaces[i];
		device_remove_file(intf->cdev.dev, &dev_attr_max_current);
		device_remove_file(intf->cdev.dev, &dev_attr_mode);
		if (rd->als_connected && (intf->flags & AS3676_FLAG_ALS_MASK))
			as3676_remove_als_tree(rd, intf);
	}
	device_remove_file(&client->dev, &dev_attr_adc_als_value);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&rd->early_suspend);
#endif

	cancel_delayed_work_sync(&rd->delayed_work);
	reg_set(rd, AS3676_REG_CTRL, 0x00);
	mutex_destroy(&rd->lock);
	dev_set_drvdata(&client->dev, NULL);
	i2c_set_clientdata(client, NULL);
	device_init_wakeup(&client->dev, 0);
	kfree(rd);

	return 0;
}

static int __devinit as3676_probe(struct i2c_client *client,
		const struct i2c_device_id *id);

static const struct i2c_device_id as3676_idtable[] = {
	{AS3676_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, as3676_idtable);

static const struct dev_pm_ops as3676_pm = {
	.suspend = as3676_pm_suspend,
	.resume = as3676_pm_resume,
};

static struct i2c_driver as3676_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = AS3676_NAME,
		.pm = &as3676_pm,
	},
	.probe   = as3676_probe,
	.remove  = __devexit_p(as3676_remove),
	.shutdown = as3676_shutdown,
	.id_table = as3676_idtable,
};

#define AS3676_DCDC_CTRL_1_DEFAULT 0x62

static int __devinit as3676_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct as3676_record *rd = 0;
	struct as3676_platform_data *pdata;
	int err;
	int i, j;
	int ldo_val;

	pdata = client->dev.platform_data;

	if (!pdata || pdata->num_leds == 0) {
		dev_err(&client->dev, "no/bad pdata provided\n");
		return -EFAULT;
	}

	if (pdata->num_leds > AS3676_INTERFACE_MAX) {
		dev_err(&client->dev, "pdata specifies too many leds\n");
		return -EFAULT;
	}

	rd = kzalloc(sizeof(struct as3676_record), GFP_KERNEL);
	if (!rd)
		return -ENOMEM;

	rd->als_connected = pdata->als_connected;
	rd->als_wait = pdata->als_wait;
	rd->dls_connected = pdata->dls_connected;
	if (pdata->ovp_high)
		rd->ovp_high = pdata->ovp_high;
	else
		rd->ovp_high = AS3676_DCDC_CTRL_1_DEFAULT;

	client->driver = &as3676_driver;

	mutex_init(&rd->lock);
	INIT_DELAYED_WORK(&rd->delayed_work, as3676_als_delayed_worker);

	/* We will need the i2c device later */
	rd->client = client;

	i2c_set_clientdata(client, rd);
	dev_set_drvdata(&client->dev, rd);

	rd->n_interfaces = pdata->num_leds;
	rd->softdim_limitation = 0;

	for (i = 0; i < rd->n_interfaces; ++i) {
		struct as3676_platform_led *led = &pdata->leds[i];
		struct as3676_interface *intf = &rd->interfaces[i];
		intf->cdev.name           = led->name;
		intf->cdev.brightness     = led->default_brightness;
		intf->cdev.brightness_set = as3676_brightness;
#ifdef CONFIG_LEDS_AS3676_HW_BLINK
		if (led->flags & AS3676_FLAG_BLINK)
			intf->cdev.blink_set = as3676_blink;
#endif
		intf->flags = led->flags;
		intf->max_current = led->max_current;
		intf->last_set_mode = AS3676_CTRL_OFF;
		intf->previous_mode = AS3676_CTRL_OFF;
		intf->index = i;

		for (j = 0; j < AS3676_SINK_MAX; ++j) {
			if (led->sinks & BIT(j)) {
				intf->regs |= ((u64)1 << as3676_sink_map[j]);
				if (as3676_sink[as3676_sink_map[j]].flags &
						AS3676_FLAG_DCDC_CTRL) {
					rd->is_dcdc = 1;
				}
				if ((intf->flags & AS3676_FLAG_PWM_CTRL) ||
				    (as3676_sink[as3676_sink_map[j]].flags &
							AS3676_FLAG_EXT_CURR)) {
					rd->softdim_limitation |=
						((u64)1 << as3676_sink_map[j]);
				}
			}
		}
		err = led_classdev_register(&client->dev, &intf->cdev);
		if (err < 0) {
			dev_info(&client->dev, "Failed to add %s\n",
					intf->cdev.name);
			goto error;
		}
		if (rd->als_connected && (intf->flags & AS3676_FLAG_ALS_MASK)) {
			err = as3676_create_als_tree(rd, intf);
			if (err) {
				dev_err(&client->dev,
					"create dev_attr_als_group failed\n");
				led_classdev_unregister(&intf->cdev);
				goto error;
			}
		}
		err = device_create_file(intf->cdev.dev, &dev_attr_max_current);
		if (err) {
			dev_err(&client->dev,
				"create dev_attr_max_current failed\n");
			as3676_remove_als_tree(rd, intf);
			led_classdev_unregister(&intf->cdev);
			goto error;
		}
		err = device_create_file(intf->cdev.dev, &dev_attr_mode);
		if (err) {
			dev_err(&client->dev,
				"create dev_attr_mode failed\n");
			device_remove_file(intf->cdev.dev,
					&dev_attr_max_current);
			as3676_remove_als_tree(rd, intf);
			led_classdev_unregister(&intf->cdev);
			goto error;
		}
	}

	err = device_create_file(&client->dev, &dev_attr_adc_als_value);
	if (err)
		dev_err(&client->dev,
			"create dev_attr_adc_als_value failed\n");

	/* Enable charge pump and connect all leds to it */
	if (rd->is_dcdc) {
		reg_set(rd, AS3676_DCDC_CTRL_1, rd->ovp_high);
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x0c);
		reg_set(rd, AS3676_REG_CTRL, 0x0d);
		usleep(AS3676_REG_CTRL_WAIT_US);
		reg_set(rd, AS3676_DCDC_CTRL_2, 0x8c);
	} else {
		reg_set(rd, AS3676_REG_CTRL, 0x05);
	}
	rd->suspend = AS3676_NO_SUSPEND;
	reg_set(rd, AS3676_OVERTEMP_CTRL, 0x10);

	if (pdata->ldo_mV <= 0) {
		ldo_val = 0x1f;
	} else if ((pdata->ldo_mV < AS3676_LDO_MIN) ||
			(pdata->ldo_mV > AS3676_LDO_MAX)) {
		dev_err(&client->dev, "ldo_mV in pdata is out-of-range\n");
		err = -EINVAL;
		goto error;
	} else {
		ldo_val = (pdata->ldo_mV - AS3676_LDO_MIN) / 50;
	}

	reg_set(rd, AS3676_LDO_VOLTAGE, ldo_val);

	reg_set(rd, AS3676_GPIO_CTRL, 0xC4);
	reg_set(rd, AS3676_MODE_SWITCH, 0x70);
	/* Allow dimming up */
	rd->softdim.enable = pdata->softdim_enable;

	reg_set(rd, AS3676_REG_PWM_CODE, 0);
	as3676_set_softdim(rd);

	if (pdata->als_config) {
		memcpy(&rd->als, pdata->als_config,
				sizeof(struct as3676_als_config));
	} else {
		memcpy(&rd->als, &as3676_default_config,
				sizeof(struct as3676_als_config));
	}

	if (rd->als_connected) {
		as3676_als_set_params(rd, &rd->als);
		as3676_als_set_enable(rd, 1);
	}

	if (rd->dls_connected)
		reg_set(rd, AS3676_REG_GPIO_CURR, 1 << 7);

	for (i = 0; i < rd->n_interfaces; ++i) {
		struct as3676_interface *intf = &rd->interfaces[i];
		as3676_set_interface_brightness(intf, intf->cdev.brightness);
		if ((rd->als_connected) && (intf->flags & AS3676_FLAG_ALS_MASK))
			as3676_set_als_config(rd, &rd->als, intf);
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	rd->early_suspend.suspend = as3676_early_suspend;
	rd->early_suspend.resume = as3676_late_resume;
	register_early_suspend(&rd->early_suspend);
#endif
	return 0;

error:
	for (--i; i >= 0; --i) {
		struct as3676_interface *intf = &rd->interfaces[i];
		device_remove_file(intf->cdev.dev, &dev_attr_max_current);
		device_remove_file(intf->cdev.dev, &dev_attr_mode);
		if (rd->als_connected && (intf->flags & AS3676_FLAG_ALS_MASK))
			as3676_remove_als_tree(rd, intf);
		led_classdev_unregister(&intf->cdev);
	}
	cancel_delayed_work_sync(&rd->delayed_work);
	reg_set(rd, AS3676_REG_CTRL, 0x00);
	mutex_destroy(&rd->lock);
	dev_set_drvdata(&client->dev, NULL);
	i2c_set_clientdata(client, NULL);
	device_init_wakeup(&client->dev, 0);
	kfree(rd);
	return err;
}

static int __init as3676_init(void)
{
	return i2c_add_driver(&as3676_driver);
}

static void __exit as3676_exit(void)
{
	i2c_del_driver(&as3676_driver);
}

module_init(as3676_init);
module_exit(as3676_exit);

MODULE_AUTHOR("Courtney Cavin <courtney.cavin@sonyericsson.com>");
MODULE_DESCRIPTION("AS3676 I2C LED driver");
MODULE_LICENSE("GPL v2");
