/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
/*
 * Device driver for monitoring ambient light intensity in (lux), RGB, and
 * color temperature (in kelvin) within the AMS-TAOS TCS family of devices.
 *
 * Copyright (c) 2016, AMS-TAOS USA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <linux/ktime.h>
#include "tcs3490.h"

//
// Debug options
//

// Create an ABI for dumping CHIP_ID, REV_ID, and colorbin register contents?
#define CHIP_ID_ABI    1

// Print info re lux calculations (printed per sample, potentially voluminous)?
//#define LUX_MESSAGES    1


//
// Configurational options
//

// Define this as nonzero, or not, depending on your system configuration.
// The color-bin correction factors need to be applied at a different
// point in the computation with inked glass vs. air or clear glass.
#define INKED_GLASS    0


//
// Constants
//

#define TCS3490_CMD_ALS_INT_CLR  0xE6
#define TCS3490_CMD_ALL_INT_CLR  0xE7
#define TCS3490_CHANNEL 0xC0

#define CENTI_MSEC_PER_ATIME_TICK  278
#define CENTI_MSEC_PER_MSEC        100

#define I2C_ADDR_OFFSET     0x80

#define GAIN1   0
#define GAIN4   1
#define GAIN16  2
#define GAIN64  3

#define TCS3490_MAX_INTEGRATION_CYCLES 256

//
// pertaining to the Color Bins register, used for improved color accuracy
//

#define _Q  16  // fixed point arithmetic shift

#define COLOR_BINS_MSK_COEFF    (1 << 15)
#define CONFIG_SPARE_2_MSK      (1 << 3)

#define COLOR_BINS_SHFT_RED     11
#define COLOR_BINS_MSK_RED      (((1 << 4) - 1) << COLOR_BINS_SHFT_RED)
#define COLOR_BINS_INT_MSK_RED  ((1 << (4-1)) - 1)

#define COLOR_BINS_SHFT_GRN     6
#define COLOR_BINS_MSK_GRN      (((1 << 5) - 1) << COLOR_BINS_SHFT_GRN)
#define COLOR_BINS_INT_MSK_GRN  ((1 << (5-1)) - 1)

#define COLOR_BINS_SHFT_BLU     0
#define COLOR_BINS_MSK_BLU      (((1 << 6) - 1) << COLOR_BINS_SHFT_BLU)
#define COLOR_BINS_INT_MSK_BLU  ((1 << (6-1)) - 1)

// These are processed only at compile time; there is no floating point
// arithmetic at runtime
#define SLOPE_RED               (-0.0111)
#define OFFSET_RED              (1.0174)
#define SLOPE_GRN               (-0.0110)
#define OFFSET_GRN              (1.0602)
#define SLOPE_BLU               (-0.0074)
#define OFFSET_BLU              (1.0631)

#define RGBCIR_SENSOR_SYSFS_LINK_NAME "rgbcir_sensor"
#define RGBCIR_SENSOR_PINCTRL_IRQ_ACTIVE "rgbcir_irq_active"
#define RGBCIR_SENSOR_PINCTRL_IRQ_SUSPEND "rgbcir_irq_suspend"

enum tcs3490_regs {
    TCS3490_CONTROL,
    TCS3490_ALS_TIME,              // 0x81
    TCS3490_RESV_1,
    TCS3490_WAIT_TIME,             // 0x83
    TCS3490_ALS_MINTHRESHLO,       // 0x84
    TCS3490_ALS_MINTHRESHHI,       // 0x85
    TCS3490_ALS_MAXTHRESHLO,       // 0x86
    TCS3490_ALS_MAXTHRESHHI,       // 0x87
    TCS3490_RESV_2,                // 0x88
    TCS3490_PRX_MINTHRESHLO,       // 0x89 -> Not used for TCS3490

    TCS3490_RESV_3,                // 0x8A
    TCS3490_PRX_MAXTHRESHHI,       // 0x8B  -> Not used for TCS3490
    TCS3490_PERSISTENCE,           // 0x8C
    TCS3490_CONFIG,                // 0x8D
    TCS3490_PRX_PULSE_COUNT,       // 0x8E  -> Not used for TCS3490
    TCS3490_GAIN,                  // 0x8F  : Gain Control Register
    TCS3490_AUX,                   // 0x90
    TCS3490_REVID,
    TCS3490_CHIPID,
    TCS3490_STATUS,                // 0x93

    TCS3490_CLR_CHANLO,            // 0x94
    TCS3490_CLR_CHANHI,            // 0x95
    TCS3490_RED_CHANLO,            // 0x96
    TCS3490_RED_CHANHI,            // 0x97
    TCS3490_GRN_CHANLO,            // 0x98
    TCS3490_GRN_CHANHI,            // 0x99
    TCS3490_BLU_CHANLO,            // 0x9A
    TCS3490_BLU_CHANHI,            // 0x9B
    TCS3490_PRX_HI,                // 0x9C
    TCS3490_PRX_LO,                // 0x9D

    TCS3490_PRX_OFFSET,            // 0x9E
    TCS3490_RESV_4,                // 0x9F
    TCS3490_IRBEAM_CFG,            // 0xA0
    TCS3490_IRBEAM_CARR,           // 0xA1
    TCS3490_IRBEAM_NS,             // 0xA2
    TCS3490_IRBEAM_ISD,            // 0xA3
    TCS3490_IRBEAM_NP,             // 0xA4
    TCS3490_IRBEAM_IPD,            // 0xA5
    TCS3490_IRBEAM_DIV,            // 0xA6
    TCS3490_IRBEAM_LEN,            // 0xA7

    TCS3490_IRBEAM_STAT,           // 0xA8

    TCS3490_REG_COLOR_BINLO=0x55,  // 0xD5
    TCS3490_REG_COLOR_BINHI=0x56,  // 0xD6

    TCS3490_REG_MAX,

};

enum tcs3490_en_reg {
    TCS3490_EN_PWR_ON   = (1 << 0),
    TCS3490_EN_ALS      = (1 << 1),
    TCS3490_EN_PRX      = (1 << 2),
    TCS3490_EN_WAIT     = (1 << 3),
    TCS3490_EN_ALS_IRQ  = (1 << 4),
    TCS3490_EN_PRX_IRQ  = (1 << 5),
    TCS3490_EN_IRQ_PWRDN = (1 << 6),
    TCS3490_EN_BEAM     = (1 << 7),
};

enum tcs3490_status {
    TCS3490_ST_ALS_VALID  = (1 << 0),
    TCS3490_ST_PRX_VALID  = (1 << 1),
    TCS3490_ST_BEAM_IRQ   = (1 << 3),
    TCS3490_ST_ALS_IRQ    = (1 << 4),
    TCS3490_ST_PRX_IRQ    = (1 << 5),
    TCS3490_ST_PRX_SAT    = (1 << 6),
};

enum {
    TCS3490_ALS_GAIN_MASK = (3 << 0),
    TCS3490_PRX_GAIN_MASK = (3 << 2),
    TCS3490_ALS_AGL_MASK  = (1 << 2),
    TCS3490_ALS_AGL_SHIFT = 2,
    TCS3490_ATIME_PER_100 = 273,
    TCS3490_ATIME_DEFAULT_MS = 50,
    SCALE_SHIFT = 11,
    RATIO_SHIFT = 10,
    MAX_ALS_VALUE = 0xffff,
    MIN_ALS_VALUE = 10,
    GAIN_SWITCH_LEVEL = 100,
    GAIN_AUTO_INIT_VALUE = AGAIN_16,
};

static u8 const restorable_regs[] = {
    TCS3490_ALS_TIME,
    TCS3490_PERSISTENCE,
    TCS3490_GAIN,
};

static u8 const als_gains[] = {
    1,
    4,
    16,
    64
};

struct tcs3490_als_info {
    u32 saturation;
	u16 clear_red_raw;
	u16 clear_green_raw;
	u16 clear_blue_raw;
    u16 clear_raw;
    u16 red_raw;
    u16 green_raw;
    u16 blue_raw;
	u16 ir_raw;
	uint64_t timestamp;
};

struct tcs3490_chip {
    struct mutex lock;
    struct i2c_client *client;
    struct tcs3490_als_info als_inf;
    struct tcs3490_parameters params;
    struct tcs3490_i2c_platform_data *pdata;
    u8 shadow[42];

	struct input_dev *a_idev;
    int in_suspend;
    int wake_irq;
    int irq_pending;
    bool unpowered;
    bool als_enabled;
	int als_thres_enabled;
	int als_switch_ch_enabled;

    bool als_gain_auto;
	u8 als_channel;
    u8 device_index;
	struct regulator *vdd;
	struct regulator *gpio_vdd;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
};

static int tcs3490_power_on(struct tcs3490_chip *chip);
static int tcs3490_pltf_power_off(struct tcs3490_chip *chip);

static int tcs3490_pinctrl_init(struct tcs3490_chip *data)
{
	data->pinctrl = devm_pinctrl_get(&data->client->dev);
	if (IS_ERR_OR_NULL(data->pinctrl)) {
		dev_err(&data->client->dev,
		"%s:%d Getting pinctrl handle failed\n",
		__func__, __LINE__);
		return -EINVAL;
	}
	data->gpio_state_active =
		pinctrl_lookup_state(data->pinctrl,
			RGBCIR_SENSOR_PINCTRL_IRQ_ACTIVE);
	if (IS_ERR_OR_NULL(data->gpio_state_active)) {
		dev_err(&data->client->dev,
		"%s:Failed to get the active state pinctrl handle\n",
		__func__);
		return -EINVAL;
	}
	data->gpio_state_suspend =
		pinctrl_lookup_state(data->pinctrl,
			RGBCIR_SENSOR_PINCTRL_IRQ_SUSPEND);
	if (IS_ERR_OR_NULL(data->gpio_state_suspend)) {
		dev_err(&data->client->dev,
		"%s:Failed to get the suspend state pinctrl handle\n",
		__func__);
		return -EINVAL;
	}
	return 0;
}

static int tcs3490_i2c_blk_read(struct tcs3490_chip *chip,
        u8 reg, u8 *val, int size)
{
    s32 ret;
    struct i2c_client *client = chip->client;

	uint8_t w_buf[1];
	uint8_t *r_buf;
	struct i2c_msg msg[2];

	reg |= I2C_ADDR_OFFSET;
	w_buf[0] = reg;
	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = 1;

	r_buf = kmalloc(size + 1, GFP_KERNEL);
	if (!r_buf)
		return -ENOMEM;
	memset(r_buf, 0, size);
	msg[1].addr = (client->addr) >> 1;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = r_buf;
	msg[1].len = size;
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2) {
		usleep_range(3000, 4000);
		ret = i2c_transfer(client->adapter, msg, 2);
		if (ret != 2) {
			dev_err(&client->dev,
				"%s: i2c ERROR !! reg=0x%x ret=%d\n",
				__func__, reg, ret);
			kfree(r_buf);
			return ret;
		}
	} else {
		ret = 0;
	}
	memcpy(val, r_buf, sizeof(u8) * size);
	kfree(r_buf);

    return ret;
}

static int tcs3490_i2c_read(struct tcs3490_chip *chip, u8 reg, u8 *val)
{
    int ret;
    struct i2c_client *client = chip->client;
	uint8_t w_buf[1];
	uint8_t r_buf[1];
	struct i2c_msg msg[2];

	reg |= I2C_ADDR_OFFSET;
	w_buf[0] = reg;
	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = 1;

	r_buf[0] = 0x0;
	msg[1].addr = (client->addr) >> 1;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = r_buf;
	msg[1].len = 1;
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2) {
		usleep_range(3000, 4000);
		ret = i2c_transfer(client->adapter, msg, 2);
		if (ret != 2) {
			dev_err(&client->dev,
				"%s: i2c ERROR !! reg=0x%x ret=%d\n",
				__func__, reg, ret);
			return ret;
		}
	} else {
		ret = 0;
	}
	*val = r_buf[0];

    return 0;
}

static int tcs3490_i2c_write(struct tcs3490_chip *chip, u8 reg, u8 val)
{
    int ret;
    struct i2c_client *client = chip->client;

	uint8_t w_buf[2];
	struct i2c_msg msg[1];

	if (reg == TCS3490_CONTROL) {
		dev_dbg(&client->dev, "%s: CONTRL addr[0x%x] data[0x%x]",
		__func__, reg, val);
	}

	reg |= I2C_ADDR_OFFSET;
	w_buf[0] = reg;
	w_buf[1] = val;
	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = 2;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret != 1) {
		usleep_range(3000, 4000);
		ret = i2c_transfer(client->adapter, msg, 1);
		if (ret != 1) {
			dev_err(&client->dev,
				"%s: i2c ERROR !! reg=0x%x ret=%d\n",
				__func__, reg, ret);
			ret = -1;
		}
	} else {
		ret = 0;
	}

    return ret;
}

static int tcs3490_i2c_reg_blk_write(struct tcs3490_chip *chip,
        u8 reg, u8 *val, int size)
{
    s32 ret;
    struct i2c_client *client = chip->client;

	uint8_t *w_buf;
	struct i2c_msg msg[2];

	reg |= I2C_ADDR_OFFSET;
	w_buf = kmalloc(size + 1, GFP_KERNEL);
	if (!w_buf) {
		dev_err(&client->dev, "%s: failed to allocate buffer",
		__func__);
		return -ENOMEM;
	}
	w_buf[0] = reg;
	memmove(&w_buf[1], val, size);
	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = size + 1;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret != 1) {
		usleep_range(3000, 4000);
		ret = i2c_transfer(client->adapter, msg, 1);
		if (ret != 1) {
			dev_err(&client->dev,
				"%s: i2c ERROR !! reg=0x%x ret=%d\n",
				__func__, reg, ret);
		}
	} else {
		ret = 0;
	}
	kfree(w_buf);

    return ret;
}

static int tcs3490_flush_regs(struct tcs3490_chip *chip)
{
    unsigned i;
    int rc;
    u8 reg;

    for (i = 0; i < ARRAY_SIZE(restorable_regs); i++) {
        reg = restorable_regs[i];
        rc = tcs3490_i2c_write(chip, reg, chip->shadow[reg]);
        if (rc) {
            dev_err(&chip->client->dev, "%s: err on reg 0x%02x\n",
                    __func__, reg);
            break;
        }
    }
    return rc;
}

static int tcs3490_update_enable_reg(struct tcs3490_chip *chip)
{
	dev_info(&chip->client->dev, "%s: Writing CONTROL, val[0x0%x]",
		__func__, chip->shadow[TCS3490_CONTROL]);
    return  tcs3490_i2c_write(chip, TCS3490_CONTROL,
            chip->shadow[TCS3490_CONTROL]);
}

static int tcs3490_set_als_gain(struct tcs3490_chip *chip, int gain)
{
    int rc;
    u8 ctrl_reg  = chip->shadow[TCS3490_GAIN] & ~TCS3490_ALS_GAIN_MASK;

    switch (gain) {
    case 1:
        ctrl_reg |= AGAIN_1;
        break;
    case 4:
        ctrl_reg |= AGAIN_4;
        break;
    case 16:
        ctrl_reg |= AGAIN_16;
        break;
    case 64:
        ctrl_reg |= AGAIN_64;
        break;
    default:
        dev_err(&chip->client->dev, "%s: wrong als gain %d\n",
                __func__, gain);
        return -EINVAL;
    }

    rc = tcs3490_i2c_write(chip, TCS3490_GAIN, ctrl_reg);
    if (!rc) {
        chip->shadow[TCS3490_GAIN] = ctrl_reg;
        chip->params.als_gain = gain;
        dev_info(&chip->client->dev, "%s: new als gain %d\n",
                __func__, gain);
    }
    return rc;
}


static int tcs3490_irq_clr(struct tcs3490_chip *chip, u8 int2clr)
{
	int ret;

	ret = tcs3490_i2c_write(chip, int2clr, 0);
	if (ret) {
		dev_err(&chip->client->dev, "%s: failed 2x, int to clr %02x\n",
		__func__, int2clr);
	}

	return ret;
}

static void tcs3490_get_als_setup_next(struct tcs3490_chip *chip)
{
	u8 *buf;
	u32 sat;
	int rc;
	u8 atime = 0;
	u8 cur_channel = 0xFF;
	ktime_t cur_ktime;

	mutex_lock(&chip->lock);
	buf = &chip->shadow[TCS3490_CLR_CHANLO];
	atime = chip->shadow[TCS3490_ALS_TIME];
	mutex_unlock(&chip->lock);

	tcs3490_i2c_read(chip, TCS3490_CHANNEL, &cur_channel);
	if ((chip->als_switch_ch_enabled == 1) && (cur_channel == 0x00)) {
		/*RGBC*/
		/*extract raw channel data*/
		mutex_lock(&chip->lock);
		chip->als_inf.clear_raw =
			le16_to_cpup((const __le16 *)&buf[0]);
		chip->als_inf.clear_red_raw =
			le16_to_cpup((const __le16 *)&buf[2]);
		chip->als_inf.clear_green_raw =
			le16_to_cpup((const __le16 *)&buf[4]);
		chip->als_inf.clear_blue_raw =
			le16_to_cpup((const __le16 *)&buf[6]);
		mutex_unlock(&chip->lock);

		/*Switch to RGB-IR*/
		rc = tcs3490_i2c_write(chip, TCS3490_CHANNEL, 0x80);
		if (!rc) {
			dev_dbg(&chip->client->dev,
				"%s: Channel: RGB-IR", __func__);
			mutex_lock(&chip->lock);
			chip->als_channel = 0x80;
			mutex_unlock(&chip->lock);
		}
		dev_dbg(&chip->client->dev,
			"%s: Changed ALS channel from RGBC to 0x%x\n",
			__func__, chip->als_channel);
	} else if ((chip->als_switch_ch_enabled == 1) &&
		(cur_channel == 0x80)) {
		/*RGBC-IR*/
		/*extract ir channel data*/
		mutex_lock(&chip->lock);
		chip->als_inf.ir_raw =
			le16_to_cpup((const __le16 *)&buf[0]);
		chip->als_inf.red_raw =
			le16_to_cpup((const __le16 *)&buf[2]);
		chip->als_inf.green_raw =
			le16_to_cpup((const __le16 *)&buf[4]);
		chip->als_inf.blue_raw =
			le16_to_cpup((const __le16 *)&buf[6]);
		mutex_unlock(&chip->lock);
		/*Switch to RGBC*/
		rc = tcs3490_i2c_write(chip, TCS3490_CHANNEL, 0x00);
		if (!rc) {
			dev_dbg(&chip->client->dev, "%s: Channel: RGBC",
				__func__);
			mutex_lock(&chip->lock);
			chip->als_channel = 0x00;
			mutex_unlock(&chip->lock);
		}
		cur_ktime = ktime_get();
		chip->als_inf.timestamp = (uint64_t)cur_ktime.tv64;
		dev_dbg(&chip->client->dev,
			"%s: Changed channel from RGBC-IR to 0x%x Time %llu\n",
			__func__, chip->als_channel, chip->als_inf.timestamp);
	} else {
		if (cur_channel == 0x00) {
			mutex_lock(&chip->lock);
			chip->als_inf.clear_raw =
				le16_to_cpup((const __le16 *)&buf[0]);
			mutex_unlock(&chip->lock);
		} else {
			mutex_lock(&chip->lock);
			chip->als_inf.ir_raw =
				le16_to_cpup((const __le16 *)&buf[0]);
			mutex_unlock(&chip->lock);
		}
		mutex_lock(&chip->lock);
		chip->als_inf.red_raw   =
			le16_to_cpup((const __le16 *)&buf[2]);
		chip->als_inf.green_raw =
			le16_to_cpup((const __le16 *)&buf[4]);
		chip->als_inf.blue_raw  =
			le16_to_cpup((const __le16 *)&buf[6]);
		mutex_unlock(&chip->lock);
		cur_ktime = ktime_get();
		chip->als_inf.timestamp = (uint64_t)cur_ktime.tv64;
	}

	sat = min_t(u32, MAX_ALS_VALUE,
		(u32)(TCS3490_MAX_INTEGRATION_CYCLES - atime) << 10);
	sat = sat * 8 / 10;
	chip->als_inf.saturation = sat;

	dev_dbg(&chip->client->dev,
		"%s: raw c/ir,r,g,b: %d, %d, %d, %d\n",
		__func__,
		(cur_channel == 0x00) ?
			chip->als_inf.clear_raw : chip->als_inf.ir_raw,
		chip->als_inf.red_raw,
		chip->als_inf.green_raw,
		chip->als_inf.blue_raw);
}

static int tcs3490_read_all(struct tcs3490_chip *chip)
{
    int ret = 0;

	mutex_lock(&chip->lock);
    tcs3490_i2c_read(chip, TCS3490_STATUS,
            &chip->shadow[TCS3490_STATUS]);

	ret = tcs3490_i2c_blk_read(chip, TCS3490_CLR_CHANLO,
		&chip->shadow[TCS3490_CLR_CHANLO], 2);
	ret = tcs3490_i2c_blk_read(chip, TCS3490_RED_CHANLO,
		&chip->shadow[TCS3490_RED_CHANLO], 2);
	ret = tcs3490_i2c_blk_read(chip, TCS3490_GRN_CHANLO,
		&chip->shadow[TCS3490_GRN_CHANLO], 2);
	ret = tcs3490_i2c_blk_read(chip, TCS3490_BLU_CHANLO,
		&chip->shadow[TCS3490_BLU_CHANLO], 2);
	mutex_unlock(&chip->lock);
    return (ret < 0) ? ret : 0;
}

static int tcs3490_update_als_thres(struct tcs3490_chip *chip, bool on_enable)
{
    s32 ret;
    u8 *buf = &chip->shadow[TCS3490_ALS_MINTHRESHLO];
    u16 deltaP = chip->params.als_deltaP;
    u16 from, to, cur;
    u16 saturation = chip->als_inf.saturation;

	mutex_lock(&chip->lock);
    cur = chip->als_inf.clear_raw;
	mutex_unlock(&chip->lock);

	if (!on_enable)
		/* move deltaP far away from
		 * current position to force an irq */
		from = to = cur > saturation / 2 ? 0 : saturation;
	else {
		deltaP = cur * deltaP / 100;
		if (!deltaP)
			deltaP = 1;
		if (cur > deltaP)
			from = cur - deltaP;
		else
			from = 0;
		if (cur < (saturation - deltaP))
			to = cur + deltaP;
		else
			to = saturation;
	}

    *buf++ = from & 0xff;
    *buf++ = from >> 8;
    *buf++ = to & 0xff;
    *buf++ = to >> 8;
	mutex_lock(&chip->lock);
    ret = tcs3490_i2c_reg_blk_write(chip, TCS3490_ALS_MINTHRESHLO,
            &chip->shadow[TCS3490_ALS_MINTHRESHLO],
            TCS3490_ALS_MAXTHRESHHI - TCS3490_ALS_MINTHRESHLO + 1);
	mutex_unlock(&chip->lock);

	dev_info(&chip->client->dev,
		"%s: on_enable[%s] cur[%u] deltaP[%u] from[%u] to[%u]",
		__func__, on_enable?"TRUE":"FALSE", cur, deltaP, from, to);

	return (ret < 0) ? ret : 0;
}

static int tcs3490_check_and_report(struct tcs3490_chip *chip)
{
    u8 status;
    u8 saturation;

    int ret = tcs3490_read_all(chip);
    if (ret)
        goto exit_clr;

	mutex_lock(&chip->lock);
    status = chip->shadow[TCS3490_STATUS];
	mutex_unlock(&chip->lock);

    saturation = chip->als_inf.saturation;

    if ((status & (TCS3490_ST_ALS_VALID | TCS3490_ST_ALS_IRQ)) ==
            (TCS3490_ST_ALS_VALID | TCS3490_ST_ALS_IRQ)) {
		tcs3490_get_als_setup_next(chip);
        tcs3490_irq_clr(chip, TCS3490_CMD_ALS_INT_CLR);
		if (chip->als_thres_enabled)
			tcs3490_update_als_thres(chip, 1);
    }

exit_clr:
    tcs3490_irq_clr(chip, TCS3490_CMD_ALL_INT_CLR);

    return ret;
}

static irqreturn_t tcs3490_irq(int irq, void *handle)
{
    struct tcs3490_chip *chip = handle;

    (void)tcs3490_check_and_report(chip);
	sysfs_notify(&chip->a_idev->dev.kobj, NULL, "notify");
    return IRQ_HANDLED;
}

static void tcs3490_set_defaults(struct tcs3490_chip *chip)
{
    u8 *sh = chip->shadow;
    struct device *dev = &chip->client->dev;

	dev_info(dev, "%s: use defaults\n", __func__);
	mutex_lock(&chip->lock);
	sh[TCS3490_ALS_TIME] = 0xFE;/*0xEE;*/ /* integration time : 254 ms */
	mutex_unlock(&chip->lock);
	sh[TCS3490_WAIT_TIME] = 0xFF; /* wait time : 2.4 ms */
	sh[TCS3490_PERSISTENCE] = ALS_PERSIST(0);
	sh[TCS3490_PRX_PULSE_COUNT] = 8;
	sh[TCS3490_GAIN] = AGAIN_1;/*AGAIN_16;*/
	sh[TCS3490_AUX] = 0x00; /* No saturation int */

    chip->als_gain_auto = false;
	mutex_lock(&chip->lock);
	chip->als_channel = 0x00; /* Clear channel */
	mutex_unlock(&chip->lock);
	chip->als_switch_ch_enabled = 1; /* Obtain RGBC and IR in turn */
}

static ssize_t tcs3490_chip_pow_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", chip->unpowered ? 0 : 1);
}

static ssize_t tcs3490_chip_pow_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct tcs3490_chip *chip = dev_get_drvdata(dev);
	int rc = 0;
	bool value;

	dev_info(&chip->client->dev, "CHK 1 %s\n", __func__);

	if (strtobool(buf, &value))
		return -EINVAL;

	dev_info(&chip->client->dev, "CHK 2 %s: value = %s\n",
		__func__, value?"TRUE":"FALSE");

	if (value) {
		if (!rc && chip->unpowered)
			tcs3490_power_on(chip);
		if (!rc && chip->pinctrl && chip->gpio_state_active) {
			rc = pinctrl_select_state(chip->pinctrl,
				chip->gpio_state_active);
			if (rc)
				dev_err(&chip->client->dev,
					"%s: cannot set pin to active state",
					__func__);
		}
		if (!rc) {
			dev_info(&chip->client->dev,
				"tcs3490: Request threaded IRQ\n");
			rc = request_threaded_irq(chip->client->irq, NULL,
				&tcs3490_irq,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				dev_name(&chip->client->dev), chip);
			if (rc) {
				dev_err(&chip->client->dev,
					"Failed to request irq %d\n",
					chip->client->irq);
				(void)pinctrl_select_state(chip->pinctrl,
					chip->gpio_state_suspend);
			}
		}
		if (rc)
			tcs3490_pltf_power_off(chip);
	} else {
		if (chip->pinctrl && chip->gpio_state_suspend) {
			free_irq(chip->client->irq, chip);
			rc = pinctrl_select_state(chip->pinctrl,
			chip->gpio_state_suspend);
			if (rc)
				dev_err(&chip->client->dev,
					"%s: cannot set pin to active state",
					__func__);
		}
		if (!chip->unpowered)
			tcs3490_pltf_power_off(chip);
	}

	return size;
}

static int tcs3490_als_enable(struct tcs3490_chip *chip, int on)
{
    int rc;

    dev_info(&chip->client->dev, "%s: on = %d\n", __func__, on);
    if (on) {
        tcs3490_irq_clr(chip, TCS3490_CMD_ALS_INT_CLR);
        tcs3490_update_als_thres(chip, 1);
        chip->shadow[TCS3490_CONTROL] |=
                (TCS3490_EN_PWR_ON | TCS3490_EN_ALS |
                TCS3490_EN_ALS_IRQ);

        rc = tcs3490_update_enable_reg(chip);
        if (rc)
            return rc;
        mdelay(3);
    } else {
        chip->shadow[TCS3490_CONTROL] &=
            ~(TCS3490_EN_ALS_IRQ);

        if (!(chip->shadow[TCS3490_CONTROL] & TCS3490_EN_PRX))
            chip->shadow[TCS3490_CONTROL] &= ~TCS3490_EN_PWR_ON;
        rc = tcs3490_update_enable_reg(chip);
        if (rc)
            return rc;
	chip->als_inf.timestamp = 0;
        tcs3490_irq_clr(chip, TCS3490_CMD_ALS_INT_CLR);
    }
    if (!rc)
        chip->als_enabled = on;

    return rc;
}

static ssize_t tcs3490_als_enable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    return snprintf(buf, PAGE_SIZE, "%d\n", chip->als_enabled);
}

static ssize_t tcs3490_als_enable_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    bool value;

    if (strtobool(buf, &value))
        return -EINVAL;

    if (value)
        tcs3490_als_enable(chip, 1);
    else
        tcs3490_als_enable(chip, 0);

    return size;
}

static ssize_t tcs3490_auto_gain_enable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    return snprintf(buf, PAGE_SIZE, "%s\n",
                chip->als_gain_auto ? "auto" : "manual");
}

static ssize_t tcs3490_auto_gain_enable_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    bool value;

    if (strtobool(buf, &value))
        return -EINVAL;

    if (value)
        chip->als_gain_auto = true;
    else
        chip->als_gain_auto = false;

    return size;
}

static ssize_t tcs3490_als_gain_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",
		chip->params.als_gain);
}

static ssize_t tcs3490_als_all_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%u,%u,%u,%u,%u,%u,%u,%u,%llu",
	chip->als_inf.clear_red_raw,
	chip->als_inf.clear_green_raw,
	chip->als_inf.clear_blue_raw,
	chip->als_inf.clear_raw,
	chip->als_inf.red_raw,
	chip->als_inf.green_raw,
	chip->als_inf.blue_raw,
	chip->als_inf.ir_raw,
	chip->als_inf.timestamp);
	mutex_unlock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_red_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%d", chip->als_inf.red_raw);
	mutex_lock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_green_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%d", chip->als_inf.green_raw);
	mutex_unlock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_blue_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%d", chip->als_inf.blue_raw);
	mutex_unlock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_clear_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	ssize_t ret;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%d", chip->als_inf.clear_raw);
	mutex_unlock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_gain_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    unsigned long gain;
    int rc;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);

    rc = kstrtoul(buf, 10, &gain);

    if (rc)
        return -EINVAL;
    if (gain != 0 && gain != 1 && gain != 4 && gain != 16 &&
            gain != 60 && gain != 64)
        return -EINVAL;

	mutex_lock(&chip->lock);
	if (gain) {
		chip->als_gain_auto = false;
		rc = tcs3490_set_als_gain(chip, gain);
	} else {
		chip->als_gain_auto = true;
	}
    tcs3490_flush_regs(chip);
    mutex_unlock(&chip->lock);
    return rc ? rc : size;
}

static ssize_t tcs3490_als_persist_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d",
		(((chip->params.persist) & 0x0f)));
}

static ssize_t tcs3490_als_persist_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    long persist;
    int rc;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);

    rc = kstrtoul(buf, 10, &persist);
    if (rc)
        return -EINVAL;

    mutex_lock(&chip->lock);
    chip->shadow[TCS3490_PERSISTENCE] &= 0xF0;
    chip->shadow[TCS3490_PERSISTENCE] |= ((u8)persist & 0x0F);

	rc = tcs3490_flush_regs(chip);
	if (!rc)
		chip->params.persist = chip->shadow[TCS3490_PERSISTENCE];
    mutex_unlock(&chip->lock);
    return size;
}

static ssize_t tcs3490_als_itime_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    int t;
	mutex_lock(&chip->lock);
	t = TCS3490_MAX_INTEGRATION_CYCLES - chip->params.als_time;
	mutex_unlock(&chip->lock);
	return snprintf(buf, PAGE_SIZE, "%d\n", t);
}

static ssize_t tcs3490_als_itime_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    long itime;
    int rc;
    struct tcs3490_chip *chip = dev_get_drvdata(dev);

    rc = kstrtoul(buf, 10, &itime);
    if (rc)
        return -EINVAL;

	mutex_lock(&chip->lock);
	chip->shadow[TCS3490_ALS_TIME] =
		TCS3490_MAX_INTEGRATION_CYCLES - (u8)itime;
	rc = tcs3490_flush_regs(chip);
	if (!rc)
		chip->params.als_time = chip->shadow[TCS3490_ALS_TIME];
    mutex_unlock(&chip->lock);
    return size;
}

static ssize_t tcs3490_als_thres_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d", chip->als_thres_enabled);
}

static ssize_t tcs3490_als_thres_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t size)
{
	struct tcs3490_chip *chip = dev_get_drvdata(dev);
	int rc;
	int on_enable;

	rc = kstrtoint(buf, 10, &on_enable);

	if (rc)
		return -EINVAL;

	if (on_enable == 1)
		tcs3490_update_als_thres(chip, 1);
	else
		tcs3490_update_als_thres(chip, 0);
	chip->als_thres_enabled = 1;
	return size;
}

static ssize_t tcs3490_als_deltaP_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
    return snprintf(buf, PAGE_SIZE,
		"%d (in %%)", chip->params.als_deltaP);
}

static ssize_t tcs3490_als_deltaP_store(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t size)
{
    unsigned long deltaP;
    int rc;
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

    rc = kstrtoul(buf, 10, &deltaP);
    if (rc || deltaP > 100)
        return -EINVAL;
    mutex_lock(&chip->lock);
    chip->params.als_deltaP = deltaP;
    mutex_unlock(&chip->lock);
	dev_info(&chip->client->dev,
		"%s: Changed ALS deltaP, deltaP[%lu]",
		__func__, deltaP);
    return size;
}

/*=========== Switch IR/Clear channel ==================*/
static ssize_t tcs3490_als_channel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

	mutex_lock(&chip->lock);
	ret = snprintf(buf, PAGE_SIZE, "%d",
		((chip->als_channel & 0x80) == 0x00) ? 0 : 1);
	mutex_unlock(&chip->lock);
	return ret;
}

static ssize_t tcs3490_als_channel_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t size)
{
	unsigned long channel;
	u8 ch_val = 0;
	int rc;
	struct tcs3490_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &channel);
	if (rc || channel > 2)
		return -EINVAL;

	ch_val = (channel == 0) ? 0x00 : 0x80;
	rc = tcs3490_i2c_write(chip, TCS3490_CHANNEL, ch_val);
	if (!rc) {
		dev_info(&chip->client->dev, "%s: new channel %s\n",
			__func__, ((ch_val & 0x80) == 0x00) ? "clear" : "ir");
		mutex_lock(&chip->lock);
		chip->als_channel = ch_val;
		mutex_unlock(&chip->lock);
	}
	return size;
}
/*======================================================*/

static ssize_t tcs3490_chip_id_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct tcs3490_chip *chip = dev_get_drvdata(dev);
	uint8_t id;

	tcs3490_i2c_read(chip, TCS3490_CHIPID, &id);
	chip->shadow[TCS3490_CHIPID] = id;

	return snprintf(buf,
			PAGE_SIZE,
			"0x%02x",
			chip->shadow[TCS3490_CHIPID]);
}

static ssize_t tcs3490_notify(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	dev_dbg(dev, "%s: Notify is called", __func__);
	return snprintf(buf, PAGE_SIZE, "%d", 1);
}

/*=========== DEFINE DEVICE_ATTR ==================*/
static DEVICE_ATTR(chip_pow, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_chip_pow_show, tcs3490_chip_pow_store);
static DEVICE_ATTR(als_Itime, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_itime_show, tcs3490_als_itime_store);
static DEVICE_ATTR(als_thres, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_thres_show, tcs3490_als_thres_store);
static DEVICE_ATTR(als_red, S_IRUGO, tcs3490_als_red_show, NULL);
static DEVICE_ATTR(als_green, S_IRUGO, tcs3490_als_green_show, NULL);
static DEVICE_ATTR(als_blue, S_IRUGO, tcs3490_als_blue_show, NULL);
static DEVICE_ATTR(als_clear, S_IRUGO, tcs3490_als_clear_show, NULL);
static DEVICE_ATTR(als_all, S_IRUGO, tcs3490_als_all_show, NULL);
static DEVICE_ATTR(als_gain, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_gain_show, tcs3490_als_gain_store);
static DEVICE_ATTR(als_thresh_deltaP, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_deltaP_show, tcs3490_als_deltaP_store);
static DEVICE_ATTR(als_auto_gain, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_auto_gain_enable_show,
			tcs3490_auto_gain_enable_store);
static DEVICE_ATTR(als_power_state, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_enable_show, tcs3490_als_enable_store);
static DEVICE_ATTR(als_persist, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_persist_show, tcs3490_als_persist_store);
static DEVICE_ATTR(als_channel, S_IRUGO | S_IWUSR | S_IWGRP,
			tcs3490_als_channel_show, tcs3490_als_channel_store);
static DEVICE_ATTR(chip_id, S_IRUGO, tcs3490_chip_id_show, NULL);
static DEVICE_ATTR(notify, S_IRUGO, tcs3490_notify, NULL);

static struct attribute *tcs3490_attributes[] = {
	&dev_attr_chip_pow.attr,
	&dev_attr_als_Itime.attr,
	&dev_attr_als_thres.attr,
	&dev_attr_als_red.attr,
	&dev_attr_als_green.attr,
	&dev_attr_als_blue.attr,
	&dev_attr_als_clear.attr,
	&dev_attr_als_all.attr,
	&dev_attr_als_gain.attr,
	&dev_attr_als_thresh_deltaP.attr,
	&dev_attr_als_auto_gain.attr,
	&dev_attr_als_power_state.attr,
	&dev_attr_als_persist.attr,
	&dev_attr_als_channel.attr,
	&dev_attr_chip_id.attr,
	&dev_attr_notify.attr,
	NULL
};

static const struct attribute_group tcs3490_attr_group = {
	.attrs = tcs3490_attributes,
};

static int tcs3490_pltf_power_on(struct tcs3490_chip *chip)
{
	int rc = 0;

	mutex_lock(&chip->lock);
	rc = regulator_enable(chip->vdd);
	if (rc) {
		dev_err(&chip->client->dev,
		"Regulator vdd enable failed rc=%d\n", rc);
		chip->unpowered = true;
		mutex_unlock(&chip->lock);
	}
	if (!rc) {
		if (chip->gpio_vdd)
			rc = regulator_enable(chip->gpio_vdd);
	}
	if (!rc) {
		dev_dbg(&chip->client->dev,
		"%s: rgbcir, power init, regulator enable OK\n", __func__);
		/* Enable Oscillator */
		tcs3490_i2c_write(chip, TCS3490_CONTROL, 0x01);
		mutex_unlock(&chip->lock);
		usleep_range(10000, 11000);
	}
	return rc;
}

static int tcs3490_power_on(struct tcs3490_chip *chip)
{
	int rc = 0;

    rc = tcs3490_pltf_power_on(chip);
    if (rc)
        return rc;
    dev_info(&chip->client->dev, "%s: chip was off, restoring regs\n",
            __func__);

	dev_info(&chip->client->dev, "tcs3490: Setting defaults\n");
	tcs3490_set_defaults(chip);
	rc = tcs3490_flush_regs(chip);

	if (!rc)
		chip->unpowered = false;

    return rc;
}

static int tcs3490_pltf_power_off(struct tcs3490_chip *chip)
{
	int rc = 0;

	mutex_lock(&chip->lock);
	/* Disable Oscillator */
	tcs3490_i2c_write(chip, TCS3490_CONTROL, 0x00);
	usleep_range(3000, 4000);
	if (chip->gpio_vdd) {
		(void)regulator_disable(chip->gpio_vdd);
	}
	rc = regulator_disable(chip->vdd);

	if (!rc)
		chip->unpowered = true;
	mutex_unlock(&chip->lock);
	return rc;
}

static int tcs3490_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct tcs3490_chip *chip;
	int rc = 0;

	dev_info(&client->dev, "start probing tcs3490\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
			"%s: check functionality failed", __func__);
		return -EIO;
	}
	dev_info(&client->dev, "tcs3490: Checking IRQ number %d\n",
		client->irq);
	if (client->irq < 0) {
		dev_err(&client->dev, "%s: no reason to run.\n", __func__);
		rc = -EINVAL;
		goto init_failed;
	} else {
		dev_info(&client->dev, "%s: client->irq = %d\n",
			__func__, client->irq);
	}
	chip = kzalloc(sizeof(struct tcs3490_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	i2c_set_clientdata(client, chip);
	chip->client = client;
	chip->device_index = 0; /*use default*/
	if (chip->client->dev.of_node) {
		rc = tcs3490_pinctrl_init(chip);
		if (rc) {
			dev_err(&client->dev,
				"%s: failed to pinctrl init\n", __func__);
			goto init_failed;
		}
	}
	dev_info(&chip->client->dev, "tcs3490: Initializing mutex\n");
	mutex_init(&chip->lock);
	chip->vdd = regulator_get(&chip->client->dev, "rgbcir_vdd");
	if (IS_ERR(chip->vdd)) {
		rc = PTR_ERR(chip->vdd);
		dev_err(&chip->client->dev,
			"Regulator get failed,avdd, rc = %d\n", rc);
		goto init_failed;
	}
	chip->gpio_vdd = NULL;
	if (chip->client->dev.of_node) {
		int count = 0;
		count = of_property_count_strings(chip->client->dev.of_node,
			"ams,rgbcir-gpio-vreg-name");
		if (count) {
			chip->gpio_vdd = regulator_get(&chip->client->dev,
				"rgbcir_gpio_vdd");
			if (IS_ERR(chip->gpio_vdd)) {
				rc = PTR_ERR(chip->vdd);
				dev_err(&chip->client->dev,
				"Regulator get failed,avdd, rc = %d\n", rc);
				goto init_failed;
			}
		}
	}
	dev_info(&chip->client->dev,
		"%s: rgbcir, power init, regulator get OK\n", __func__);
	chip->unpowered = true;
	chip->a_idev = input_allocate_device();
	if (!chip->a_idev) {
		rc = -ENOMEM;
		dev_err(&client->dev,
		"%s: failed to allocate input device", __func__);
		kfree(chip);
	}
	chip->a_idev->name = "AMS TCS3490 Sensor";

	dev_info(&chip->client->dev, "tcs3490: set bit EV_ABS\n");
	set_bit(EV_ABS, chip->a_idev->evbit);

	dev_info(&chip->client->dev, "tcs3490: set bit ABS_MISC\n");
	set_bit(ABS_MISC, chip->a_idev->absbit);

	dev_info(&chip->client->dev, "tcs3490: Set ABS params\n");
	input_set_abs_params(chip->a_idev, ABS_MISC, 0, 65535, 0, 0); /*check*/

	dev_info(&chip->client->dev,
		"tcs3490: assigning open/close functions\n");

	rc = input_register_device(chip->a_idev);
	if (rc) {
		dev_err(&client->dev,
		"failed to register input device");
		goto exit_free_dev_ps;
	}
	input_set_drvdata(chip->a_idev, chip);
	dev_info(&chip->client->dev, "tcs3490: i2c nr %d\n",
		client->adapter->nr);

	rc = sysfs_create_group(&chip->a_idev->dev.kobj,
		&tcs3490_attr_group);
	if (rc) {
		rc = -ENOMEM;
		dev_err(&client->dev,
			"%s: failed to create sysfs group", __func__);
		goto exit_unregister_dev_ps;
	}

	rc = sysfs_create_link(chip->a_idev->dev.kobj.parent,
		&chip->a_idev->dev.kobj, RGBCIR_SENSOR_SYSFS_LINK_NAME);
	if (rc) {
		rc = -ENOMEM;
		dev_err(&client->dev,
			"%s: failed to create sysfs link", __func__);
		goto exit_unregister_dev_ps;
	}

	dev_info(&client->dev, "Probe ok.\n");
	return 0;

init_failed:
	dev_err(&client->dev, "Probe failed.\n");
	return rc;

exit_unregister_dev_ps:
	input_unregister_device(chip->a_idev);
exit_free_dev_ps:
	input_free_device(chip->a_idev);
	kfree(chip);
	return rc;
}

static int tcs3490_remove(struct i2c_client *client)
{
    struct tcs3490_chip *chip = i2c_get_clientdata(client);
	sysfs_remove_link(&chip->a_idev->dev.kobj,
		RGBCIR_SENSOR_SYSFS_LINK_NAME);
    free_irq(client->irq, chip);
    if (chip->a_idev) {
        input_unregister_device(chip->a_idev);
    }

    i2c_set_clientdata(client, NULL);
    kfree(chip);
    return 0;
}

static const struct i2c_device_id tcs3490_idtable[] = {
    { "tcs3490", 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, tcs3490_idtable);

static const struct of_device_id tcs3490_dt_match[] = {
	{ .compatible = "ams,tcs3490", },
	{ },
};
MODULE_DEVICE_TABLE(of, tcs3490_dt_match);

static struct i2c_driver tcs3490_driver = {
    .driver = {
        .name = "tcs3490",
		.owner = THIS_MODULE,
		.of_match_table = tcs3490_dt_match,
    },
    .id_table = tcs3490_idtable,
    .probe = tcs3490_probe,
	.remove = tcs3490_remove,
};

static int __init tcs3490_init(void)
{
	int rc = 0;

	pr_info("Initialize TCS3490 driver");
	rc = i2c_add_driver(&tcs3490_driver);
	pr_info("TCS3490 added i2c driver rc = %d", rc);

	return rc;
}

static void __exit tcs3490_exit(void)
{
	pr_info("Delete TCS3490 driver");
    i2c_del_driver(&tcs3490_driver);
}

module_init(tcs3490_init);
module_exit(tcs3490_exit);

MODULE_DESCRIPTION("AMS tcs3490 driver");
MODULE_LICENSE("GPL v2");
