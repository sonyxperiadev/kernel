// SPDX-License-Identifier: GPL-2.0-only
/*
 * Fuel gauge driver for sm5038
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This driver is based on sm5038_fg.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/mod_devicetable.h>
#include <linux/power_supply.h>
#include <linux/power/sm5038_fg.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

static struct sm5038_fg_data *static_fg_data;


#define fg_abs(x) (((x) < 0) ? -(x):(x))
#define MINVAL(a, b) ((a <= b) ? a : b)
#define MAXVAL(a, b) ((a > b) ? a : b)

#define SM5038_MAX_VBAT_MARGIN		50 /* 50 mV */

#define LIMIT_N_CURR_MIXFACTOR -2000
#define TABLE_READ_COUNT 2
#define FG_ABNORMAL_RESET -1
#define IGNORE_N_I_OFFSET 1
#define SM5038_FG_FULL_DEBUG 1
#define I2C_ERROR_COUNT_MAX 5
#define USE_UNIT_MICRO 1
//#define USE_AGING_RATE_FILT 1
#define MIN_RECHARGE_SOC 950
#define CHECK_BATT_ID 1
#ifdef CHECK_BATT_ID
#define BATT_ID_OHM_MIN 1400
#define BATT_ID_OHM_MAX 28700
#define BATT_ID_INVALID_SOC 200
#endif
#define USE_CC_DISCHG_CAPACITY
#ifdef USE_CC_DISCHG_CAPACITY
#define CC_DISCHG_CAPACITY_LIMIT_3_0 95
#define CC_DISCHG_CAPACITY_DENOM_3_0 107
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define CURRENT_SENSE_5MOHM 0
#define CURRENT_SENSE_2MOHM 1
#endif

static int sm5038_fg_debug_print;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
enum print_reason {
	PR_INTERRUPT	= BIT(0),
	PR_CORE		= BIT(1),
	PR_MISC		= BIT(2),
	PR_REGISTER	= BIT(3),
	PR_LEARN	= BIT(4),
};
#define somc_sm5038_fg_dbg(fuelgauge, reason, fmt, ...)			\
	do {								\
		if (fuelgauge->debug_mask & (reason))			\
			pr_info("[SOMC FG] %s: "fmt, __func__, ##__VA_ARGS__);	\
		else							\
			pr_debug("[SOMC FG] %s: "fmt, __func__, ##__VA_ARGS__);	\
	} while (0)
#endif

static int sm5038_regs[] = {
	0x0000, 0x0001, 0x0003, 0x0004, 0x0010, 0x0013, 0x0020, 0x0021, 0x0022, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F
};

static int sm5038_srams[] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A,
	0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048,
	0x0059, 0x005A, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x0087, 0x008A, 0x008B,
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
	0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D8, 0x00DF, 0x00ED, 0x00EE
};

static int sm5038_abnormal_reset_check(struct sm5038_fg_data *fuelgauge);
static bool sm5038_fg_init(struct sm5038_fg_data *fuelgauge, bool is_surge);
static unsigned int sm5038_get_vbat(struct sm5038_fg_data *fuelgauge);
static unsigned int sm5038_get_soc(struct sm5038_fg_data *fuelgauge);
static unsigned int sm5038_get_scaled_soc(struct sm5038_fg_data *fuelgauge);
static unsigned int sm5038_get_ocv(struct sm5038_fg_data *fuelgauge);
static unsigned int sm5038_get_qage(struct sm5038_fg_data *fuelgauge);
static int sm5038_get_current(struct sm5038_fg_data *fuelgauge);
#ifdef USE_AGING_RATE_FILT
static int sm5038_get_soh(struct sm5038_fg_data *fuelgauge);
#endif
static unsigned int  sm5038_get_cycle(struct sm5038_fg_data *fuelgauge);
#ifdef CHECK_BATT_ID
extern int sm5038_get_batt_id_ohm(unsigned int *batt_id_ohm);
#endif
void sm5038_fg_set_proc_cycle(int cycle);


static int sm5038_fg_read_sram(struct sm5038_fg_data *fuelgauge, int sram_addr)
{
	int data;

	mutex_lock(&fuelgauge->fg_lock);

	if (sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_SRAM_RADDR, sram_addr)) {
		pr_info("%s: SM5038_FG_REG_SRAM_RADDR write ERROR!!!!!\n", __func__);
		mutex_unlock(&fuelgauge->fg_lock);
		return -EPERM;
	}

	data = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_SRAM_RDATA);

	mutex_unlock(&fuelgauge->fg_lock);

	return data;
}

static int sm5038_fg_write_sram(struct sm5038_fg_data *fuelgauge, int sram_addr, int data)
{
	mutex_lock(&fuelgauge->fg_lock);

	if (sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_SRAM_WADDR, sram_addr)) {
		pr_info("%s: SM5038_FG_REG_SRAM_WADDR write ERROR!!!!!\n", __func__);
		mutex_unlock(&fuelgauge->fg_lock);
		return -EPERM;
	}
	if (sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_SRAM_WDATA, data)) {
		pr_info("%s: SM5038_FG_REG_SRAM_WDATA write ERROR!!!!!\n", __func__);
		mutex_unlock(&fuelgauge->fg_lock);
		return -EPERM;
	}

	mutex_unlock(&fuelgauge->fg_lock);

	return 1;
}

static int sm5038_fg_verified_write_word(struct i2c_client *client,
		u8 reg_addr, u16 data)
{
	int ret;

	ret = sm5038_write_word(client, reg_addr, data);
	if (ret < 0) {
		msleep(50);
		pr_info("1st fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n",
				__func__, ret, reg_addr, data);
		ret = sm5038_write_word(client, reg_addr, data);
		if (ret < 0) {
			msleep(50);
			pr_info("2nd fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n",
					__func__, ret, reg_addr, data);
			ret = sm5038_write_word(client, reg_addr, data);
			if (ret < 0) {
				pr_info("3rd fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n",
						__func__, ret, reg_addr, data);
			}
		}
	}

	return ret;
}

static void sm5038_dump_all(struct sm5038_fg_data *fuelgauge)
{
	int val;
	int i;
	int r_size = (int)ARRAY_SIZE(sm5038_srams)/4;
	char temp_buf[700] = {0,};

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (!fuelgauge->regdump_en)
		return;
#endif
	switch (sm5038_fg_debug_print%5) {
	case 0:
		for (i = 0; i < (int)ARRAY_SIZE(sm5038_regs); i++) {
			val = -1;
			val = sm5038_read_word(fuelgauge->i2c, sm5038_regs[i]);
			sprintf(temp_buf+strlen(temp_buf), "%02x:%04x,", sm5038_regs[i], val);
		}
		pr_info("[sm5038_fg_all_regs] %s\n", temp_buf);
		memset(temp_buf, 0x0, sizeof(temp_buf));
		sm5038_fg_debug_print++;
		break;
	case 1:
		for (i = 0;  i < r_size; i++) {
			val = -1;
			val = sm5038_fg_read_sram(fuelgauge, sm5038_srams[i]);
			sprintf(temp_buf+strlen(temp_buf), "%02x:%04x,", sm5038_srams[i], val);
		}
		pr_info("[sm5038_fg_all_srams_1] %s\n", temp_buf);
		memset(temp_buf, 0x0, sizeof(temp_buf));
		sm5038_fg_debug_print++;
		break;
	case 2:
		for (i = r_size; i < r_size*2; i++) {
			val = -1;
			val = sm5038_fg_read_sram(fuelgauge, sm5038_srams[i]);
			sprintf(temp_buf+strlen(temp_buf), "%02x:%04x,", sm5038_srams[i], val);
		}
		pr_info("[sm5038_fg_all_srams_2] %s\n", temp_buf);
		memset(temp_buf, 0x0, sizeof(temp_buf));
		sm5038_fg_debug_print++;
		break;
	case 3:
		for (i = r_size*2; i < r_size*3; i++) {
			val = -1;
			val = sm5038_fg_read_sram(fuelgauge, sm5038_srams[i]);
			sprintf(temp_buf+strlen(temp_buf), "%02x:%04x,", sm5038_srams[i], val);
		}
		pr_info("[sm5038_fg_all_srams_3] %s\n", temp_buf);
		memset(temp_buf, 0x0, sizeof(temp_buf));
		sm5038_fg_debug_print++;
		break;
	case 4:
		for (i = r_size*3; i < r_size*4; i++) {
			val = -1;
			val = sm5038_fg_read_sram(fuelgauge, sm5038_srams[i]);
			sprintf(temp_buf+strlen(temp_buf), "%02x:%04x,", sm5038_srams[i], val);
		}
		pr_info("[sm5038_fg_all_srams_4] %s\n", temp_buf);
		memset(temp_buf, 0x0, sizeof(temp_buf));
		sm5038_fg_debug_print = 0;
		break;
	}
}

static int sm5038_get_temperature(struct sm5038_fg_data *fuelgauge)
{
	int ret;
	int temp; /* = 250; 250 means 25.0oC*/

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_TEMPERATURE);
	if (ret < 0) {
		pr_err("%s: read temp reg fail", __func__);
		temp = 0;
	} else {
		temp = (((ret&0x7FFF)*10) * 2989) >> 11 >> 8;
		if (ret&0x8000) {
			temp *= -1;
		}
	}
	fuelgauge->info.temp_fg = temp;

	if (fuelgauge->info.min_temp_fg > temp) {
		fuelgauge->info.min_temp_fg = temp;
	}
	if (fuelgauge->info.max_temp_fg < temp) {
		fuelgauge->info.max_temp_fg = temp;
	}

	return temp;
}

#if 0
static int sm5038_get_charge_status(struct sm5038_fg_data *fuelgauge)
{
	int ret = -1;

	if (sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT) & 0x0020) {
		if (fuelgauge->info.avg_current > 0) {
			ret = POWER_SUPPLY_STATUS_CHARGING;
		} else {
			ret = POWER_SUPPLY_STATUS_DISCHARGING;
		}
	} else {
		// POWER_SUPPLY_STATUS_FULL????
		ret = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	return ret;
}

static int sm5038_get_battery_health(struct sm5038_fg_data *fuelgauge)
{
	// todo return charging health
	int temp, vavg, vbat, ret;
//	u32 val;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VBAT_AVG);
	if (ret < 0) {
		goto health_error;
	}

	if (ret & 0x8000) {
		vavg = 2700 - (((ret & 0x7fff) * 10) / 109);
	} else {
		vavg = ((ret * 10) / 109) + 2700;
	}

	sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VBAT);

	if (ret < 0) {
		goto health_error;
	}

	if (ret & 0x8000) {
		vbat = 2700 - (((ret & 0x7fff) * 10) / 109);
	} else {
		vbat = ((ret * 10) / 109) + 2700;
	}

	if (vavg < fuelgauge->init_data.min_vbat) {
		ret = POWER_SUPPLY_HEALTH_DEAD;
		goto out;
	}

	if (vbat > fuelgauge->init_data.max_vbat + SM5038_MAX_VBAT_MARGIN) {
		ret = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		goto out;
	}

	// is not battery temp, this is charger IC skin temp
	temp = sm5038_get_temperature(fuelgauge);

	if (temp < fuelgauge->init_data.min_temp) {
		ret = POWER_SUPPLY_HEALTH_COLD;
		goto out;
	}

	if (temp > fuelgauge->init_data.max_temp) {
		ret = POWER_SUPPLY_HEALTH_OVERHEAT;
		goto out;
	}

	ret = POWER_SUPPLY_HEALTH_GOOD;

out:
	return ret;

health_error:
	return ret;
}
#endif

int sm5038_fg_get_prop_cycle(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	int cycle = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	cycle = sm5038_get_cycle(fuelgauge);

	fuelgauge->info.batt_soc_cycle = cycle;

	return cycle;
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_cycle);

int sm5038_fg_get_prop_vbat_max(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}
#ifdef USE_UNIT_MICRO
	return (fuelgauge->info.max_vbat*1000);
#else
	return (fuelgauge->info.max_vbat);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_vbat_max);

int sm5038_fg_get_prop_vbat_min(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

#ifdef USE_UNIT_MICRO
	return (fuelgauge->info.min_vbat*1000);
#else
	return (fuelgauge->info.min_vbat);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_vbat_min);

int sm5038_fg_get_prop_init_vbat_max(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

#ifdef USE_UNIT_MICRO
	return (fuelgauge->init_data.max_vbat*1000);
#else
	return (fuelgauge->init_data.max_vbat);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_init_vbat_max);

int sm5038_fg_get_prop_init_vbat_min(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

#ifdef USE_UNIT_MICRO
	return (fuelgauge->init_data.min_vbat*1000);
#else
	return (fuelgauge->init_data.min_vbat);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_init_vbat_min);

int sm5038_fg_get_prop_vbat_now(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_vbat(fuelgauge);
	if (ret < 0)
		return ret;
	else
#ifdef USE_UNIT_MICRO
		return (fuelgauge->info.vbat*1000);
#else
		return (fuelgauge->info.vbat);
#endif

}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_vbat_now);

int sm5038_fg_get_prop_vbat_avg(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_vbat(fuelgauge);
	if (ret < 0)
		return ret;
	else
#ifdef USE_UNIT_MICRO
		return (fuelgauge->info.avg_vbat*1000);
#else
		return (fuelgauge->info.avg_vbat);
#endif

}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_vbat_avg);

int sm5038_fg_get_prop_ocv(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_ocv(fuelgauge);
	if (ret < 0)
		return ret;
	else
#ifdef USE_UNIT_MICRO
		return (fuelgauge->info.batt_ocv*1000);
#else
		return (fuelgauge->info.batt_ocv);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_ocv);


int sm5038_fg_get_prop_soc(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_scaled_soc(fuelgauge);

	/* for_debug */
	sm5038_dump_all(fuelgauge);

	if (ret < 0)
		return ret;
	else
		return (ret/10);
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_soc);

int sm5038_fg_get_prop_rsoc(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_soc(fuelgauge);

	if (ret < 0)
		return ret;
	else
		return (ret/10);
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_rsoc);

int sm5038_fg_get_prop_q_max_design(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}
#ifdef USE_UNIT_MICRO
	return ((fuelgauge->init_data.q_max * 1000) >> 11) * 1000;
#else
	return ((fuelgauge->init_data.q_max * 1000) >> 11);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_q_max_design);

int sm5038_fg_get_prop_q_max(void)
{
	int q_max;
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}
	q_max = fuelgauge->info.q_max * 1000 >> 11;
#ifdef USE_UNIT_MICRO
	return q_max * 1000;
#else
	return q_max;
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_q_max);

int sm5038_fg_get_prop_q_now(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

#ifdef USE_UNIT_MICRO
	return (fuelgauge->info.q_now * 1000);
#else
	return fuelgauge->info.q_now;
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_q_now);


int sm5038_fg_get_prop_soc_cycle(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	return (fuelgauge->info.batt_soc_cycle);
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_soc_cycle);


int sm5038_fg_get_prop_soh(void)
{
#ifdef USE_AGING_RATE_FILT	
    struct sm5038_fg_data *fuelgauge = static_fg_data;
#endif	
    int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

#ifdef USE_AGING_RATE_FILT
	ret = sm5038_get_soh(fuelgauge);
#else
	ret = 100;
#endif
	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_soh);


int sm5038_fg_get_prop_temp(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret = 0;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_get_temperature(fuelgauge);
	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_temp);

int sm5038_fg_get_prop_temp_min(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	return (fuelgauge->info.min_temp_fg);
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_temp_min);

int sm5038_fg_get_prop_temp_max(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	return (fuelgauge->info.max_temp_fg);
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_temp_max);

int sm5038_fg_get_prop_current_now(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}
	sm5038_get_current(fuelgauge);

#ifdef USE_UNIT_MICRO
		return (fuelgauge->info.fg_current*1000);
#else
		return (fuelgauge->info.fg_current);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_current_now);


int sm5038_fg_get_prop_current_avg(void)
{
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	return 0;
#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}
	sm5038_get_current(fuelgauge);

#ifdef USE_UNIT_MICRO
	return (fuelgauge->info.avg_current*1000);
#else
	return (fuelgauge->info.avg_current);
#endif
#endif
}
EXPORT_SYMBOL_GPL(sm5038_fg_get_prop_current_avg);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
int somc_sm5038_fg_learn_update(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	u16 qage;
	int ret, limit_gap, high_add, low_add, initial_qage;

	initial_qage = (fuelgauge->init_data.q_max * 1000) >> 11;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_QAGE);
	somc_sm5038_fg_dbg(fuelgauge, PR_LEARN, "QAGE:0x%04x", ret);
	if (ret == 0) {
		qage = initial_qage;
		if (fuelgauge->info.q_max != qage)
			fuelgauge->info.q_max = qage;
	} else {
		qage = ((ret * 1000) >> 11);
		if (qage != fuelgauge->qage) {
			fuelgauge->learning_counter++;
			somc_sm5038_fg_dbg(fuelgauge, PR_LEARN, "QAGE changed:%d->%d, learning_counter:%d",
					fuelgauge->qage,
					qage, fuelgauge->learning_counter);
			fuelgauge->qage = qage;
		}
		ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_CTRL1);
		high_add = (ret & 0x3000) >> 12;
		low_add = (ret & 0x0C00) >> 10;
		limit_gap = 40 - (low_add * 10) + (high_add * 5);
		if (high_add == 3)
			limit_gap = limit_gap - 2;
		qage = (qage * 100) / limit_gap;

		fuelgauge->learned_capacity_raw = qage;
		fuelgauge->info.q_max = qage;
		somc_sm5038_fg_dbg(fuelgauge, PR_LEARN, "learned_capacity_raw:%d, limit_gap:%d, aux_ctrl1:0x%04x\n",
					fuelgauge->learned_capacity_raw,
					limit_gap, ret);
	}
	return 0;
}

int somc_sm5038_fg_learn_get_counter(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	return fuelgauge->learning_counter;
}

int somc_sm5038_fg_learn_set_range(int min, int max)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret;
	u16 read_data, write_data, reg_min, reg_max;

	if (min >= 70) {
		fuelgauge->learning_range_min = 70;
		reg_min = 0x0C00;
	} else if (min >= 60) {
		fuelgauge->learning_range_min = 60;
		reg_min = 0x0800;
	}  else if (min >= 50) {
		fuelgauge->learning_range_min = 50;
		reg_min = 0x0400;
	} else {
		fuelgauge->learning_range_min = 40;
		reg_min = 0x0000;
	}

	if (max >= 93) {
		fuelgauge->learning_range_max = 93;
		reg_max = 0x3000;
	} else if (max >= 90) {
		fuelgauge->learning_range_max = 90;
		reg_max = 0x2000;
	} else if (max >= 85) {
		fuelgauge->learning_range_max = 85;
		reg_max = 0x1000;
	} else {
		fuelgauge->learning_range_max = 80;
		reg_max = 0x0000;
	}

	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_CTRL1);
	if (ret < 0) {
		pr_err("%s: Can't read reg_aux_ctrl1.\n", __func__);
		return 0;
	}
	read_data = ret;
	write_data = read_data & ~0x3C00;
	write_data |= (reg_min | reg_max);
	somc_sm5038_fg_dbg(fuelgauge, PR_LEARN, "AUX_CTRL1:0x%04x->0x%04x", read_data, write_data);
	if (read_data != write_data) {
		ret = sm5038_write_word(fuelgauge->i2c,
					SM5038_FG_REG_AUX_CTRL1, write_data);
		if (ret < 0) {
			pr_err("%s: Can't write reg_aux_ctrl1.\n", __func__);
			return 0;
		}
	}

	return 0;
}

int somc_sm5038_fg_learn_get_learned_capacity_raw(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	return fuelgauge->learned_capacity_raw;
}

int somc_sm5038_fg_get_prop_batt_soc(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;
	int ret;
	u16 soc_reg;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return -EINVAL;
	}

	/* Temporary */
	(void)sm5038_abnormal_reset_check(fuelgauge);

	/* Test */
	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_QAGE);
	somc_sm5038_fg_dbg(fuelgauge, PR_LEARN, "QAGE:0x%04x", ret);

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC);
	if (ret < 0) {
		pr_err("%s: Can't read sram soc. Keep previous soc:%d\n",
					__func__, fuelgauge->somc_batt_soc);
	} else {
		soc_reg = ret & 0x7FFF;
		fuelgauge->somc_batt_soc = soc_reg * 100 / 256;
	}
	return (fuelgauge->somc_batt_soc);
}
EXPORT_SYMBOL(somc_sm5038_fg_get_prop_batt_soc);

#endif
static unsigned int sm5038_get_vbat(struct sm5038_fg_data *fuelgauge)
{
	int ret1 = 0, ret2 = 0;
	unsigned int vbat = 0, vbat_avg = 0; /* = 3500; 3500 means 3500mV*/

	ret1 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VBAT);
    ret2 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VBAT_AVG);

	if (ret1 < 0) {
		pr_err("%s: read vbat reg fail", __func__);
		vbat = 4000;
	} else {
		if (ret1 & 0x8000)
			vbat = 2700 - (((ret1 & 0x7fff) * 10) / 109);
		else
			vbat = ((ret1 * 10) / 109) + 2700;
	}
	fuelgauge->info.vbat = vbat;

	if (ret2 < 0) {
		pr_err("%s: read vbat_avg reg fail", __func__);
		vbat_avg = vbat;
	} else {
		if (ret2 & 0x8000)
			vbat_avg = 2700 - (((ret2 & 0x7fff) * 10) / 109);
		else
			vbat_avg = ((ret2 * 10) / 109) + 2700;
	}
    fuelgauge->info.avg_vbat = vbat_avg;

	if (fuelgauge->info.min_vbat > vbat) {
		fuelgauge->info.min_vbat = vbat;
	}
	if (fuelgauge->info.max_vbat < vbat) {
		fuelgauge->info.max_vbat = vbat;
	}

	return vbat;
}

static int sm5038_get_current(struct sm5038_fg_data *fuelgauge)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int ret, s_stat, a_stat;
	int curr = 0, curr_avg = 0; /* = 1000; 1000 means 1000mA*/

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_CURRENT);
    s_stat = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_SYSTEM_STATUS);
    a_stat = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT);

	if (ret < 0) {
		pr_err("%s: read curr reg fail", __func__);
		curr = 0;
	} else {
		curr = ((ret&0x7fff)*1000)/2044;
		if (ret&0x8000) {
			curr *= -1;
		} else if (!(a_stat & 0x0020)) {
			curr = 0;
		}
	}

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_CURRENT_AVG);
	if (ret < 0) {
		pr_err("%s: read curr_avg reg fail", __func__);
		curr_avg = 0;
	} else {
		curr_avg = ((ret&0x7fff)*1000)/2044;
		if (ret&0x8000) {
			curr_avg *= -1;
		} else if (!(a_stat & 0x0020)) {
			curr_avg = 0;
		}
	}

	if (a_stat & 0x0400) {
		curr = curr * 25 / 10;
		curr_avg = curr_avg * 25 / 10;
	}

	if ((s_stat & 0x8000) || ((a_stat & 0x0060) == 0x0040)) {
		pr_info("%s: AUX stat = 0x%x\n", __func__, ret);
		curr = 0;
		curr_avg = 0;
	}

	fuelgauge->info.fg_current = curr;
    fuelgauge->info.avg_current = curr_avg;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int ret;
	int curr;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_CURRENT);
	if (ret < 0) {
		pr_err("%s: read curr reg fail", __func__);
		curr = 0;
	} else {
		curr = ((ret & 0x7fff) * 1000) / 2044;
		if (fuelgauge->current_sense == CURRENT_SENSE_2MOHM)
			curr = curr * 25 / 10;
		if (ret & 0x8000)
			curr *= -1;
	}
	fuelgauge->info.fg_current = curr;
#endif
	return curr;
}

static unsigned int sm5038_get_ocv(struct sm5038_fg_data *fuelgauge)
{
	int ret;
	unsigned int ocv; /* = 3500; *//*3500 means 3500mV*/

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_OCV);
	if (ret < 0) {
		pr_err("%s: read ocv reg fail\n", __func__);
		ocv = 4000;
	} else {
		ocv = (ret * 1000) >> 11;
	}

	fuelgauge->info.batt_ocv = ocv;

	return ocv;
}

static unsigned int sm5038_get_qest(struct sm5038_fg_data *fuelgauge)
{
	int ret;
	unsigned int qest;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_QEST);
	if (ret < 0) {
		pr_err("%s: read qest reg fail\n", __func__);
		qest = 1000;
	} else {
		qest = (ret * 1000) >> 11;
	}

	fuelgauge->info.q_now = qest;

	return qest;
}


static bool sm5038_fg_check_reg_init_need(struct sm5038_fg_data *fuelgauge)
{
	int ret;

	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_SYSTEM_STATUS);

	if ((ret & INIT_CHECK_MASK) == DISABLE_RE_INIT) {
		pr_info("%s: SM5038_REG_FG_SYSTEM_STATUS : 0x%x , return FALSE NO init need\n", __func__, ret);
		return 0;
	} else {
		pr_info("%s: SM5038_REG_FG_SYSTEM_STATUS : 0x%x , return TRUE init need!!!!\n", __func__, ret);
		return 1;
	}
}

unsigned int sm5038_get_soc(struct sm5038_fg_data *fuelgauge)
{
	int ret;
	unsigned int soc, ocv, qest;
#ifdef CHECK_BATT_ID
	unsigned int batt_id_ohm;
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	soc = somc_sm5038_fg_get_prop_batt_soc() / 10;

	return soc;
#endif
	/* vbat update */
	sm5038_get_vbat(fuelgauge);

	/* ocv update */
	ocv = sm5038_get_ocv(fuelgauge);

	/* qest update */
	qest = sm5038_get_qest(fuelgauge);

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC);
	if (ret < 0) {
		pr_err("%s: Warning!!!! read soc reg fail\n", __func__);
		soc = 500;
	} else {
		soc = (ret * 10) >> 8;
		if (fg_abs(fuelgauge->info.soc - soc) > 50) {
			pr_info("%s: Warning!!!! info_batt_soc = %d, read_soc = %d, ret = 0x%x, soc_gap_high\n", __func__, fuelgauge->info.soc, soc, ret);
			ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC);
			if (ret < 0) {
				pr_err("%s: Warning!!!! read soc reg fail\n", __func__);
				soc = 500;
			} else {
				soc = (ret * 10) >> 8;
			}
			pr_info("%s: Warning!!!! second read soc = %d, ret = 0x%x\n", __func__, soc, ret);
		}
	}

	if (sm5038_abnormal_reset_check(fuelgauge) < 0 || fuelgauge->info.init_complete == 0)	{
		pr_info("%s: FG init ERROR!! pre_SOC returned!!, read_SOC = %d, pre_SOC = %d\n", 
			__func__, soc, fuelgauge->info.soc);
		return fuelgauge->info.soc;
	}

    ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT);
	if ((ret & 0x0060) == 0x0040) {
		pr_info("%s: AUX stat = 0x%x\n", __func__, ret);
		/** update qage **/
		if (soc > 950) {
			sm5038_get_qage(fuelgauge);
		}

		/** for soc scale todo **/
		
	}

	fuelgauge->info.soc = soc;
	pr_info("%s: soc = %d, avg_v = %d, avg_i = %d, ocv = %d, qest = %d, qage = %d\n", __func__,
		soc, fuelgauge->info.avg_vbat, fuelgauge->info.avg_current, ocv, qest, fuelgauge->info.q_max);

#ifdef CHECK_BATT_ID
	/** check batt_ID, if batt_ID is abnormal return 20% SOC **/
	sm5038_get_batt_id_ohm(&batt_id_ohm);
	if (!((batt_id_ohm > BATT_ID_OHM_MIN) && (batt_id_ohm < BATT_ID_OHM_MAX))) {
		fuelgauge->info.soc = BATT_ID_INVALID_SOC;
		pr_info("%s: Abnormal batt_ID!!!! soc = %d\n", __func__, fuelgauge->info.soc);
	}
#endif
	return fuelgauge->info.soc;
}

unsigned int sm5038_get_scaled_soc(struct sm5038_fg_data *fuelgauge)
{
	unsigned int scaled_soc = 0;
	unsigned int soc = 0;

	soc = sm5038_get_soc(fuelgauge);

	scaled_soc = soc * 1000 / fuelgauge->info.soc_scale_base;

	pr_info("%s: scaled_soc=%d, soc=%d, base=%d\n", __func__, scaled_soc, soc, fuelgauge->info.soc_scale_base);

	fuelgauge->info.scaled_soc = scaled_soc;

	return scaled_soc;
}

enum sm5038_battery_table_type {
	DISCHARGE_TABLE = 0,
	SOC_TABLE,
	TABLE_MAX,
};

int sm5038_fg_calculate_iocv(struct sm5038_fg_data *fuelgauge)
{
	bool only_lb = false, sign_i_offset = 0; /*valid_cb=false, */
	int roop_start = 0, roop_max = 0, i = 0, cb_last_index = 0, cb_pre_last_index = 0;
	int lb_v_buffer[FG_INIT_B_LEN+1] = {0, 0, 0, 0, 0, 0, 0, 0};
	int lb_i_buffer[FG_INIT_B_LEN+1] = {0, 0, 0, 0, 0, 0, 0, 0};
	int cb_v_buffer[FG_INIT_B_LEN+1] = {0, 0, 0, 0, 0, 0, 0, 0};
	int cb_i_buffer[FG_INIT_B_LEN+1] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i_offset_margin = 0x14, i_vset_margin = 0x67;
	int v_max = 0, v_min = 0, v_sum = 0, lb_v_avg = 0, cb_v_avg = 0, lb_v_set = 0, lb_i_set = 0, i_offset = 0; /* lb_v_minmax_offset=0, */
	int i_max = 0, i_min = 0, i_sum = 0, lb_i_avg = 0, cb_i_avg = 0, cb_v_set = 0, cb_i_set = 0; /* lb_i_minmax_offset=0, */
	int lb_i_p_v_min = 0, lb_i_n_v_max = 0, cb_i_p_v_min = 0, cb_i_n_v_max = 0;

	int v_ret = 0, i_ret = 0, ret = 0;

	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT);
	pr_info("%s: iocv_status_read = addr : 0x%x , data : 0x%x\n", __func__, SM5038_FG_REG_AUX_STAT, ret);

	/* init start */
	if ((ret & 0x0800) == 0x0000) {
		only_lb = true;
	}

/*
	if ((ret & 0x3000) == 0x3000) {
		valid_cb = true;
	}
*/
/* init end */

	/* lb get start */
	roop_max = (ret & 0xF000)>>12;
	if (roop_max > FG_INIT_B_LEN+1)
		roop_max = FG_INIT_B_LEN+1;

	roop_start = SM5038_FG_ADDR_SRAM_START_LB_V;
	for (i = roop_start; i < roop_start + roop_max; i++) {
		v_ret = sm5038_fg_read_sram(fuelgauge, i);
		i_ret = sm5038_fg_read_sram(fuelgauge, i+0x10);

		if ((i_ret&0x8000) == 0x8000) {
			i_ret = -(i_ret&0x7FFF);
		}

		lb_v_buffer[i-roop_start] = v_ret;
		lb_i_buffer[i-roop_start] = i_ret;

		if (i == roop_start) {
			v_max = v_ret;
			v_min = v_ret;
			v_sum = v_ret;
			i_max = i_ret;
			i_min = i_ret;
			i_sum = i_ret;
		} else {
			if (v_ret > v_max)
				v_max = v_ret;
			else if (v_ret < v_min)
				v_min = v_ret;
			v_sum = v_sum + v_ret;

			if (i_ret > i_max)
				i_max = i_ret;
			else if (i_ret < i_min)
				i_min = i_ret;
			i_sum = i_sum + i_ret;
		}

		if (fg_abs(i_ret) > i_vset_margin) {
			if (i_ret > 0) {
				if (lb_i_p_v_min == 0) {
					lb_i_p_v_min = v_ret;
				} else {
					if (v_ret < lb_i_p_v_min)
						lb_i_p_v_min = v_ret;
				}
			} else {
				if (lb_i_n_v_max == 0) {
					lb_i_n_v_max = v_ret;
				} else {
					if (v_ret > lb_i_n_v_max)
						lb_i_n_v_max = v_ret;
				}
			}
		}
	}
	v_sum = v_sum - v_max - v_min;
	i_sum = i_sum - i_max - i_min;

    /*
	lb_v_minmax_offset = v_max - v_min;
	lb_i_minmax_offset = i_max - i_min;
	*/

	lb_v_avg = v_sum / (roop_max-2);
	lb_i_avg = i_sum / (roop_max-2);
	/* lb get end */

	/* lb_vset start */
	if (fg_abs(lb_i_buffer[roop_max-1]) < i_vset_margin) {
		if (fg_abs(lb_i_buffer[roop_max-2]) < i_vset_margin) {
			lb_v_set = MAXVAL(lb_v_buffer[roop_max-2], lb_v_buffer[roop_max-1]);
			if (fg_abs(lb_i_buffer[roop_max-3]) < i_vset_margin) {
				lb_v_set = MAXVAL(lb_v_buffer[roop_max-3], lb_v_set);
			}
		} else {
			lb_v_set = lb_v_buffer[roop_max-1];
		}
	} else {
		lb_v_set = lb_v_avg;
	}

	if (lb_i_n_v_max > 0) {
		lb_v_set = MAXVAL(lb_i_n_v_max, lb_v_set);
	}
/*
	else if (lb_i_p_v_min > 0) {
		lb_v_set = MINVAL(lb_i_p_v_min, lb_v_set);
	}
	lb_vset end

	lb offset make start
*/
	if (roop_max > 3) {
		lb_i_set = (lb_i_buffer[2] + lb_i_buffer[3]) / 2;
	}

	if ((fg_abs(lb_i_buffer[roop_max-1]) < i_offset_margin) && (fg_abs(lb_i_set) < i_offset_margin)) {
		lb_i_set = MAXVAL(lb_i_buffer[roop_max-1], lb_i_set);
	} else if (fg_abs(lb_i_buffer[roop_max-1]) < i_offset_margin) {
		lb_i_set = lb_i_buffer[roop_max-1];
	} else if (fg_abs(lb_i_set) < i_offset_margin) {
		i_offset = lb_i_set;
	} else {
		lb_i_set = 0;
	}

	i_offset = lb_i_set;

	i_offset = i_offset + 4;	/* add extra offset */

	if (i_offset <= 0) {
		sign_i_offset = 1;
#ifdef IGNORE_N_I_OFFSET
		i_offset = 0;
#else
		i_offset = -i_offset;
#endif
	}

	i_offset = i_offset>>1;

	if (sign_i_offset == 0) {
		i_offset = i_offset|0x0080;
	}
	i_offset = i_offset | i_offset<<8;

	pr_info("%s: iocv_l_max=0x%x, iocv_l_min=0x%x, iocv_l_avg=0x%x, lb_v_set=0x%x, roop_max=%d \n",
			__func__, v_max, v_min, lb_v_avg, lb_v_set, roop_max);
	pr_info("%s: ioci_l_max=0x%x, ioci_l_min=0x%x, ioci_l_avg=0x%x, lb_i_set=0x%x, i_offset=0x%x, sign_i_offset=%d\n",
			__func__, i_max, i_min, lb_i_avg, lb_i_set, i_offset, sign_i_offset);

	if (!only_lb) {
		/* cb get start */
		roop_start = SM5038_FG_ADDR_SRAM_START_CB_V;
		roop_max = FG_INIT_B_LEN+1;
		for (i = roop_start; i < roop_start + roop_max; i++) {
			v_ret = sm5038_fg_read_sram(fuelgauge, i);
			i_ret = sm5038_fg_read_sram(fuelgauge, i+0x10);

			if ((i_ret&0x8000) == 0x8000) {
				i_ret = -(i_ret&0x7FFF);
			}

			cb_v_buffer[i-roop_start] = v_ret;
			cb_i_buffer[i-roop_start] = i_ret;

			if (i == roop_start) {
				v_max = v_ret;
				v_min = v_ret;
				v_sum = v_ret;
				i_max = i_ret;
				i_min = i_ret;
				i_sum = i_ret;
			} else {
				if (v_ret > v_max)
					v_max = v_ret;
				else if (v_ret < v_min)
					v_min = v_ret;
				v_sum = v_sum + v_ret;

				if (i_ret > i_max)
					i_max = i_ret;
				else if (i_ret < i_min)
					i_min = i_ret;
				i_sum = i_sum + i_ret;
			}

			if (fg_abs(i_ret) > i_vset_margin) {
				if (i_ret > 0) {
					if (cb_i_p_v_min == 0) {
						cb_i_p_v_min = v_ret;
					} else {
						if (v_ret < cb_i_p_v_min)
							cb_i_p_v_min = v_ret;
					}
				} else {
					if (cb_i_n_v_max == 0) {
						cb_i_n_v_max = v_ret;
					} else {
						if (v_ret > cb_i_n_v_max)
							cb_i_n_v_max = v_ret;
					}
				}
			}
		}
		v_sum = v_sum - v_max - v_min;
		i_sum = i_sum - i_max - i_min;

		cb_v_avg = v_sum / (roop_max-2);
		cb_i_avg = i_sum / (roop_max-2);
		/* cb get end */

		/* cb_vset start */
		cb_last_index = ((ret & 0xF000)>>12)-(FG_INIT_B_LEN+1)-1; /*-8-1 */
		if (cb_last_index < 0) {
			cb_last_index = FG_INIT_B_LEN;
		}

		for (i = roop_max; i > 0; i--) {
			if (fg_abs(cb_i_buffer[cb_last_index]) < i_vset_margin) {
				cb_v_set = cb_v_buffer[cb_last_index];
				if (fg_abs(cb_i_buffer[cb_last_index]) < i_offset_margin) {
					cb_i_set = cb_i_buffer[cb_last_index];
				}

				cb_pre_last_index = cb_last_index - 1;
				if (cb_pre_last_index < 0) {
					cb_pre_last_index = FG_INIT_B_LEN;
				}

				if (fg_abs(cb_i_buffer[cb_pre_last_index]) < i_vset_margin) {
					cb_v_set = MAXVAL(cb_v_buffer[cb_pre_last_index], cb_v_set);
					if (fg_abs(cb_i_buffer[cb_pre_last_index]) < i_offset_margin) {
						cb_i_set = MAXVAL(cb_i_buffer[cb_pre_last_index], cb_i_set);
					}
				}
			} else {
				cb_last_index--;
				if (cb_last_index < 0) {
					cb_last_index = FG_INIT_B_LEN;
				}
			}
		}

		if (cb_v_set == 0) {
			cb_v_set = cb_v_avg;
			if (cb_i_set == 0) {
				cb_i_set = cb_i_avg;
			}
		}

		if (cb_i_n_v_max > 0) {
			cb_v_set = MAXVAL(cb_i_n_v_max, cb_v_set);
		}
/*
		else if(cb_i_p_v_min > 0) {
			cb_v_set = MINVAL(cb_i_p_v_min, cb_v_set);
		}
		cb_vset end

		cb offset make start
*/
		if (fg_abs(cb_i_set) < i_offset_margin) {
			if (cb_i_set > lb_i_set) {
				i_offset = cb_i_set;
				i_offset = i_offset + 4;	/* add extra offset */

				if (i_offset <= 0) {
					sign_i_offset = 1;
#ifdef IGNORE_N_I_OFFSET
					i_offset = 0;
#else
					i_offset = -i_offset;
#endif
				}

				i_offset = i_offset>>1;

				if (sign_i_offset == 0) {
					i_offset = i_offset|0x0080;
				}
				i_offset = i_offset | i_offset<<8;

			}
		}
		/* cb offset make end */

		pr_info("%s: iocv_c_max=0x%x, iocv_c_min=0x%x, iocv_c_avg=0x%x, cb_v_set=0x%x, cb_last_index=%d\n",
				__func__, v_max, v_min, cb_v_avg, cb_v_set, cb_last_index);
		pr_info("%s: ioci_c_max=0x%x, ioci_c_min=0x%x, ioci_c_avg=0x%x, cb_i_set=0x%x, i_offset=0x%x, sign_i_offset=%d\n",
				__func__, i_max, i_min, cb_i_avg, cb_i_set, i_offset, sign_i_offset);

	}

	/* final set */
	if ((fg_abs(cb_i_set) > i_vset_margin) || only_lb) {
		ret = MAXVAL(lb_v_set, cb_i_n_v_max);
		cb_i_set = lb_i_avg;
	} else {
		ret = cb_v_set;
		cb_i_set = cb_i_avg;
	}

	/* for HQ first boot vbat-soc adjust */
	if (((ret > 0x1CCC) && (ret < 0x1D99)) && ((cb_i_set < 0x3000) && (cb_i_set > 0xCC))) {
		cb_v_set = ret;
		ret = ret - (((cb_i_set * 19) + 283624) / 1000);
		pr_info("%s: first boot vbat-soc adjust 1st_v=0x%x, 2nd_v=0x%x, all_i=0x%x\n", __func__, cb_v_set, ret, cb_i_set);
	}

	if (ret > fuelgauge->init_data.battery_table[DISCHARGE_TABLE][FG_TABLE_LEN-1]) {
		pr_info("%s: iocv ret change 0x%x -> 0x%x \n", __func__, ret, fuelgauge->init_data.battery_table[DISCHARGE_TABLE][FG_TABLE_LEN-1]);
		ret = fuelgauge->init_data.battery_table[DISCHARGE_TABLE][FG_TABLE_LEN-1];
	} else if (ret < fuelgauge->init_data.battery_table[DISCHARGE_TABLE][0]) {
		pr_info("%s: iocv ret change 0x%x -> 0x%x \n", __func__, ret, (fuelgauge->init_data.battery_table[DISCHARGE_TABLE][0] + 0x10));
		ret = fuelgauge->init_data.battery_table[DISCHARGE_TABLE][0] + 0x10;
	}

	return ret;
}

bool sm5038_fg_reset(struct sm5038_fg_data *fuelgauge, bool is_quickstart)
{
	if (fuelgauge->info.init_complete == 0) {
		pr_info("%s: Not work reset! prev init working! return! \n", __func__);
		return true;
	}

	pr_info("%s: Start fg reset\n", __func__);
	/* SW reset code */
    fuelgauge->info.init_complete = 0;

	/* cycle backup */
	sm5038_get_cycle(fuelgauge);

	sm5038_fg_verified_write_word(fuelgauge->i2c, SM5038_FG_REG_RESET, SW_RESET_CODE);
	/* delay 600ms */
	msleep(600);

	if (is_quickstart) {
		if (sm5038_fg_init(fuelgauge, false)) {
			pr_info("%s: Quick Start !!\n", __func__);
		} else {
			pr_info("%s: sm5038_fg_init ERROR!!!!\n", __func__);
			return false;
		}
	}
#ifdef ENABLE_BATT_CYCLE_STEP
	else {
		if (sm5038_fg_init(fuelgauge, true)) {
			pr_info("%s: BATT_CYCLE_STEP reset !!\n", __func__);
		} else {
			pr_info("%s: sm5038_fg_init ERROR!!!!\n", __func__);
			return false;
		}
	}
#endif

	pr_info("%s: End fg reset\n", __func__);

	return true;
}
static bool sm5038_fg_reg_init(struct sm5038_fg_data *fuelgauge, bool is_surge)
{
	int i, j, k, value, ret = 0;
	uint8_t table_reg;
	int write_table[TABLE_MAX][FG_TABLE_LEN+1];

	pr_info("%s: sm5038_fg_reg_init START!!\n", __func__);

	/* start write ctrl unlock */
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_CTRL, FG_WRITE_UNLOCK_CODE);

	/* start first param_ctrl unlock */
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_TABLE_UNLOCK, FG_PARAM_UNLOCK_CODE);

	/* CAP write */
    sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_Q_MAX, fuelgauge->init_data.q_max);
	pr_info("%s: SM5038_FG_ADDR_SRAM_Q_MAX 0x%x\n",
		__func__, fuelgauge->init_data.q_max);

	for (i = 0; i < TABLE_MAX; i++) {
		for (j = 0; j <= FG_TABLE_LEN; j++) {
			write_table[i][j] = fuelgauge->init_data.battery_table[i][j];
		}
	}

	for (i = 0; i < TABLE_MAX; i++)	{
		table_reg = SM5038_FG_ADDR_TABLE0_0 + (i*(FG_TABLE_LEN+1));
		pr_info("table_%d write : ", i);
		for (j = 0; j <= FG_TABLE_LEN; j++) {
			sm5038_fg_write_sram(fuelgauge, (table_reg + j), write_table[i][j]);
			//pr_info("%s: TABLE write [%d][%d] = 0x%x : 0x%x\n", __func__, i, j, (table_reg + j), write_table[i][j]);
			pr_info("[0x%x]0x%x ", (table_reg + j), write_table[i][j]);
		}
		pr_info("\n");
	}

	/*for verify table data write*/
	for (i = 0; i < TABLE_MAX; i++) {
		table_reg = SM5038_FG_ADDR_TABLE0_0 + (i*(FG_TABLE_LEN+1));
		for (j = 0; j <= FG_TABLE_LEN; j++) {
			if (write_table[i][j] == sm5038_fg_read_sram(fuelgauge, (table_reg + j))) {
				pr_debug("%s: TABLE data verify OK [%d][%d] = 0x%x : 0x%x\n",
				__func__, i, j, (table_reg + j), write_table[i][j]);
			} else {
				ret |= I2C_ERROR_CHECK;
				for (k = 1; k <= I2C_ERROR_COUNT_MAX; k++) {
					pr_info("%s: TABLE write data ERROR!!!! rewrite [%d][%d] = 0x%x : 0x%x, count=%d\n",
						__func__, i, j, (table_reg + j), write_table[i][j], k);
					sm5038_fg_write_sram(fuelgauge, (table_reg + j), write_table[i][j]);
					msleep(30);
					if (write_table[i][j] == sm5038_fg_read_sram(fuelgauge, (table_reg + j))) {
						pr_debug("%s: TABLE rewrite OK [%d][%d] = 0x%x : 0x%x, count=%d\n",
						__func__, i, j, (table_reg + j), write_table[i][j], k);
						break;
					}
					if (k == I2C_ERROR_COUNT_MAX)
						ret |= I2C_ERROR_REMAIN;
				}
			}
		}
	}

	table_reg = SM5038_FG_ADDR_TABLE2_0;
	pr_info("table_%d write : ", i);
	for (j = 0; j <= FG_ADD_TABLE_LEN; j++) {
		sm5038_fg_write_sram(fuelgauge, (table_reg + j), fuelgauge->init_data.battery_table[i][j]);
		//pr_info("%s: TABLE write OK [%d][%d] = 0x%x : 0x%x\n", __func__, i, j, (table_reg + j), fuelgauge->init_data.battery_table[i][j]);
		pr_info("[0x%x]0x%x ", (table_reg + j), fuelgauge->init_data.battery_table[i][j]);
	}
	pr_info("\n");

	/* RS write */
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_MIN, fuelgauge->init_data.rs_value[1]);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_MAX, fuelgauge->init_data.rs_value[2]);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_FACTOR, fuelgauge->init_data.rs_value[3]);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_CHG_FACTOR, fuelgauge->init_data.rs_value[4]);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_DISCHG_FACTOR, fuelgauge->init_data.rs_value[5]);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_AUTO_MAN_VALUE, fuelgauge->init_data.rs_value[6]);

	pr_info("%s: spare=0x%x, MIN=0x%x, MAX=0x%x, FACTOR=0x%x, C_FACT=0x%x, D_FACT=0x%x, MAN_VALUE=0x%x\n", __func__,
		fuelgauge->init_data.rs_value[0], fuelgauge->init_data.rs_value[1], fuelgauge->init_data.rs_value[2], fuelgauge->init_data.rs_value[3],
		fuelgauge->init_data.rs_value[4], fuelgauge->init_data.rs_value[5], fuelgauge->init_data.rs_value[6]);

	/* surge reset defence */
	if (is_surge) {
		/* cycle restore */
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC_CYCLE, fuelgauge->info.batt_soc_cycle);
		pr_info("%s: SM5038 batt_soc_cycle restore %d\n", __func__, fuelgauge->info.batt_soc_cycle);
		value = ((fuelgauge->info.batt_ocv<<8)/125);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2, 0);
	} else {
		pr_info("%s: iocv is buffer value\n", __func__);
		value = sm5038_fg_calculate_iocv(fuelgauge);
	}
	msleep(10);
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_INIT_OCV, value);
	pr_info("%s: IOCV_MAN_WRITE = %d : 0x%x\n", __func__, SM5038_FG_ADDR_SRAM_INIT_OCV, value);

	/* LOCK */
	value = FG_PARAM_LOCK_CODE;
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_TABLE_UNLOCK, value);
	pr_info("%s: LAST PARAM CTRL VALUE = 0x%x : 0x%x\n", __func__, SM5038_FG_REG_TABLE_UNLOCK, value);

	/* init delay */
	msleep(20);

	/* write batt data version */
	// ret |= (fuelgauge->init_data.data_ver << 4) & DATA_VERSION;
	ret = (fuelgauge->init_data.data_ver << 4) & DATA_VERSION;
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_1, ret);
	pr_info("%s: RESERVED = 0x%x : 0x%x\n", __func__, SM5038_FG_ADDR_SRAM_USER_RESERV_1, ret);

	/* soh first write for init*/
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2, 0);

	return 1;
}

static int sm5038_abnormal_reset_check(struct sm5038_fg_data *fuelgauge)
{
	int cntl_read;

	/* abnormal case process */
	if (sm5038_fg_check_reg_init_need(fuelgauge)) {
		cntl_read = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_CTRL);
		pr_info("%s: SM5038 FG abnormal case!!!! SM5038_FG_REG_CTRL : 0x%x, init_complete : %d\n", __func__,
		cntl_read, fuelgauge->info.init_complete);

		if (fuelgauge->info.init_complete == 1) {
			/* SW reset code */
			fuelgauge->info.init_complete = 0;
			if (sm5038_fg_verified_write_word(fuelgauge->i2c, SM5038_FG_REG_RESET, SW_RESET_OTP_CODE) < 0) {
				pr_info("%s: Warning!!!! SM5038 FG abnormal case.... SW reset FAIL \n", __func__);
			} else {
				pr_info("%s: SM5038 FG abnormal case.... SW reset OK\n", __func__);
			}
			/* delay 100ms */
			msleep(100);
			/* init code */
			sm5038_fg_init(fuelgauge, true);
		}
		return FG_ABNORMAL_RESET;
	}
	return 0;
}

static void sm5038_ical_set (struct sm5038_fg_data *fuelgauge)
{
	if (!fuelgauge->init_data.i_dp_default) {
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_DP_IOFFSET, fuelgauge->init_data.dp_i_off);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_DP_IPSLOPE, fuelgauge->init_data.dp_i_pslo);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_DP_INSLOPE, fuelgauge->init_data.dp_i_nslo);

    }
	if (!fuelgauge->init_data.i_alg_default) {
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_ALG_IOFFSET, fuelgauge->init_data.alg_i_off);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_ALG_IPSLOPE, fuelgauge->init_data.alg_i_pslo);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_ALG_INSLOPE, fuelgauge->init_data.alg_i_nslo);
    }

	pr_info("%s: dp %d: <0x%x 0x%x 0x%x> alg %d: <0x%x 0x%x 0x%x>\n",
		__func__,
	fuelgauge->init_data.i_dp_default, fuelgauge->init_data.dp_i_off, fuelgauge->init_data.dp_i_pslo, fuelgauge->init_data.dp_i_nslo,
	fuelgauge->init_data.i_alg_default, fuelgauge->init_data.alg_i_off, fuelgauge->init_data.alg_i_pslo, fuelgauge->init_data.alg_i_nslo);
}

static void sm5038_vcal_set(struct sm5038_fg_data *fuelgauge)
{
	if (!fuelgauge->init_data.v_default)	{
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VOFFSET, fuelgauge->init_data.v_off);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VSLOPE, fuelgauge->init_data.v_slo);
	}

	if (!fuelgauge->init_data.vt_default) {
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_VVT, fuelgauge->init_data.vtt);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_IVT, fuelgauge->init_data.ivt);
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_IVV, fuelgauge->init_data.ivv);
	}

    pr_info("%s: vcal %d: <0x%x 0x%x> T %d: <0x%x 0x%x 0x%x>\n",
			__func__,
		fuelgauge->init_data.v_default, fuelgauge->init_data.v_off, fuelgauge->init_data.v_slo,
		fuelgauge->init_data.vt_default, fuelgauge->init_data.vtt, fuelgauge->init_data.ivt, fuelgauge->init_data.ivv);
}

void sm5038_aux_set(struct sm5038_fg_data *fuelgauge)
{
    int rsmanvalue;

	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_AUX_CTRL1, fuelgauge->init_data.aux_ctrl[0]);
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_AUX_CTRL2, fuelgauge->init_data.aux_ctrl[1]);

    rsmanvalue = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_RS_AUTO_MAN_VALUE);

	pr_info("%s: aux_ctrl1 = 0x%x, aux_ctrl2 = 0x%x, rsmanvalue = 0x%x\n",
		__func__, fuelgauge->init_data.aux_ctrl[0], fuelgauge->init_data.aux_ctrl[1], rsmanvalue);

    return;
}

#ifdef USE_AGING_RATE_FILT
static void sm5038_soh_init(struct sm5038_fg_data *fuelgauge)
{
	int ret, temp;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2);
	temp = ret;
	ret = ret & 0x7F;
	if (ret == 0) {
		fuelgauge->info.soh = 100;
		temp = temp & 0x80;
		temp = temp | fuelgauge->info.soh;
		sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2, temp);
		pr_info("%s : soh reset %d, write 0x%x\n", __func__, fuelgauge->info.soh, temp);
	} else {
		fuelgauge->info.soh = ret;
		pr_info("%s asoc restore : %d\n", __func__, fuelgauge->info.soh);
	}
}

static int sm5038_get_soh(struct sm5038_fg_data *fuelgauge)
{
	int soh, pre_soh, h_flag, c_flag, delta_t, temp;

	soh = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_AGING_RATE_FILT);
	pre_soh = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2);
	c_flag = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC_CYCLE);

	pr_info("%s : asoc = %d, ic_soh = 0x%x, pre = 0x%x, cycle = %d, t = %d \n", __func__, fuelgauge->info.soh, soh, pre_soh, c_flag, fuelgauge->info.temp_fg);

    soh = soh * 100 / 2048;

	h_flag = (pre_soh & 0x80)>>7;
	pre_soh = pre_soh & 0x7F;
	c_flag = (c_flag >> 4) % 2;

	// need batt_temp, don't use IC skin temp.
	delta_t = fuelgauge->info.temp_fg/10 - fuelgauge->init_data.temp_std;
	if (delta_t >= 0) {
		if (soh < pre_soh) {
			if (c_flag != h_flag) {
				pre_soh = pre_soh-1;
				temp = (c_flag<<7) | pre_soh;
				sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2, temp);
				pr_info("%s : pre_soh update to %d, write 0x%x\n", __func__, pre_soh, temp);
			}
		}
	}
	soh = pre_soh;

	fuelgauge->info.soh = soh;

	return fuelgauge->info.soh;
}
#endif

static unsigned int sm5038_get_qage(struct sm5038_fg_data *fuelgauge)
{
	int ret, limit_gap, high_add, low_add;
	u16 qage = 0;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_QAGE);

	if (ret == 0) {
#ifndef USE_AGING_RATE_FILT
		ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2);
		ret = ret & 0x3FFF;
		if ((ret == 0) || (ret > fuelgauge->init_data.q_max)) {
			qage = fuelgauge->init_data.q_max;
		} else {
			qage = ret;
		}
#else
		qage = fuelgauge->init_data.q_max;
#endif
		pr_info("%s: capacity has not yet been clearly calculated, full_chg = 0x%x, full_chg designed = 0x%x\n", __func__, qage, fuelgauge->init_data);
	} else {
		qage = ret;
		ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_CTRL1);
		high_add = (ret & 0x3000) >> 12;
		low_add = (ret & 0x0C00) >> 10;
		limit_gap = 40 - (low_add * 10) + (high_add * 5);
		if (high_add == 3)
			limit_gap = limit_gap - 2;
		qage = (qage * 100) / limit_gap;
#ifdef USE_CC_DISCHG_CAPACITY
		if (qage*100/fuelgauge->init_data.q_max > CC_DISCHG_CAPACITY_LIMIT_3_0) {
			qage = fuelgauge->init_data.q_max;
		} else {
			qage = qage * 100 / CC_DISCHG_CAPACITY_DENOM_3_0;
		}
#endif
		if (qage > fuelgauge->init_data.q_max) {
			qage = fuelgauge->init_data.q_max;
		}
		pr_info("%s: capacity has been calculated, last full_chg = 0x%x, full_chg = 0x%x, full_chg designed = 0x%x, aux = 0x%x\n",
			__func__, fuelgauge->info.q_max, qage, fuelgauge->init_data.q_max, ret);
	}

	fuelgauge->info.q_max = qage;
#ifndef USE_AGING_RATE_FILT
	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2);
	ret = ret & ~0x3FFF;
	ret = ret & qage;
	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2, qage);
#endif

	return qage;
}

static unsigned int sm5038_get_cycle(struct sm5038_fg_data *fuelgauge)
{
	int ret = 0, cycle = 0;

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC_CYCLE);
	if (ret < 0) {
		pr_err("%s: read cycle reg fail", __func__);
		cycle = 0;
	} else {
		cycle = ret&0x00FF;
	}
	fuelgauge->info.batt_soc_cycle = cycle;

	pr_info("%s: SM5038 batt_soc_cycle = %d\n", __func__, cycle);

	return cycle;
}

void sm5038_fg_set_proc_cycle(int cycle)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	if (static_fg_data == NULL) {
		pr_err("%s: static fg fail", __func__);
		return ;
	}

	/* start first param_ctrl unlock */
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_TABLE_UNLOCK, FG_PARAM_UNLOCK_CODE);

	sm5038_fg_write_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC_CYCLE, cycle);
	pr_info("%s: SM5038 manual batt_soc_cycle write %d\n", __func__, cycle);

	/* end param_ctrl lock */
	sm5038_write_word(fuelgauge->i2c, SM5038_FG_REG_TABLE_UNLOCK, FG_PARAM_LOCK_CODE);

	return;
}
EXPORT_SYMBOL_GPL(sm5038_fg_set_proc_cycle);

static int sm5038_device_id = -1;

static unsigned int sm5038_get_device_id(struct sm5038_fg_data *fuelgauge)
{
	int ret;
	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_DEVICE_ID);
	sm5038_device_id = ret;
	pr_info("%s: SM5038 vender_id = 0x%x\n", __func__, ret);

	return ret;
}

static void sm5038_fg_iocv_buffer_read(struct sm5038_fg_data *fuelgauge)
{
	int ret0, ret1, ret2, ret3, ret4, ret5, ret6, ret7;

	ret0 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V);
	ret1 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+1);
	ret2 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+2);
	ret3 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+3);
	ret4 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+4);
	ret5 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+5);
	ret6 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+6);
	ret7 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_V+7);
	pr_info("%s: sm5038 FG buffer lb_V = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n",
		__func__, ret0, ret1, ret2, ret3, ret4, ret5, ret6, ret7);

	ret0 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V);
	ret1 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+1);
	ret2 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+2);
	ret3 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+3);
	ret4 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+4);
	ret5 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+5);
	ret6 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+6);
	ret7 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_V+7);
	pr_info("%s: sm5038 FG buffer cb_V = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n",
		__func__, ret0, ret1, ret2, ret3, ret4, ret5, ret6, ret7);

	ret0 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I);
	ret1 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+1);
	ret2 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+2);
	ret3 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+3);
	ret4 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+4);
	ret5 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+5);
	ret6 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+6);
	ret7 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_LB_I+7);
	pr_info("%s: sm5038 FG buffer lb_I = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n",
		__func__, ret0, ret1, ret2, ret3, ret4, ret5, ret6, ret7);

	ret0 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I);
	ret1 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+1);
	ret2 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+2);
	ret3 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+3);
	ret4 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+4);
	ret5 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+5);
	ret6 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+6);
	ret7 = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_START_CB_I+7);
	pr_info("%s: sm5038 FG buffer cb_I = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n",
		__func__, ret0, ret1, ret2, ret3, ret4, ret5, ret6, ret7);

	return;
}

static bool sm5038_fg_init(struct sm5038_fg_data *fuelgauge, bool is_surge)
{
	int ret;
	fuelgauge->info.init_complete = 0;

	if (sm5038_get_device_id(fuelgauge) < 0) {
		return false;
	}

	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_1);
	pr_info("%s: reserv_1 = 0x%x\n", __func__, ret);

	if ((ret & DATA_VERSION) != ((fuelgauge->init_data.data_ver << 4) & DATA_VERSION)) {
		pr_info("%s: data ver check diff FG will reset!!!! ret = 0x%x, data = 0x%x\n", __func__, ret, fuelgauge->init_data.data_ver << 4);
		/* FG reset for reinit*/
		sm5038_fg_verified_write_word(fuelgauge->i2c, SM5038_FG_REG_RESET, SW_RESET_CODE);
		/* reset delay 1000ms */
		msleep(1000);
	}

	if (sm5038_fg_check_reg_init_need(fuelgauge)) {
		if (sm5038_fg_reg_init(fuelgauge, is_surge))
			pr_info("%s: boot time kernel init DONE!\n", __func__);
		else
			pr_info("%s: ERROR!! boot time kernel init ERROR!!\n", __func__);
	}

    // curr_cal setup
	sm5038_ical_set(fuelgauge);
    sm5038_vcal_set(fuelgauge);

	/* write aux_ctrl cfg */
	sm5038_aux_set(fuelgauge);

#ifdef USE_AGING_RATE_FILT
	sm5038_soh_init(fuelgauge);
	fuelgauge->info.q_max = fuelgauge->init_data.q_max;
#else
	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_USER_RESERV_2);
	ret = ret & 0x3FFF;
	if ((ret == 0) || (ret > fuelgauge->init_data.q_max)) {
		fuelgauge->info.q_max = fuelgauge->init_data.q_max;
	} else {
		fuelgauge->info.q_max = ret;
	}
#endif

	fuelgauge->info.init_complete = 1;

	/* set init value */
	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_SOC);
	fuelgauge->info.soc = (ret * 10) >> 8;
	fuelgauge->info.soc_scale_base = 1000;
	fuelgauge->info.max_vbat = sm5038_get_vbat(fuelgauge);
	fuelgauge->info.min_vbat = fuelgauge->info.max_vbat;
	pr_info("%s: probe stage init soc = %d, ret = 0x%x, vbat = %d\n", __func__, fuelgauge->info.soc, ret, fuelgauge->info.max_vbat);

	/* first chg_state_update */
	sm5038_get_current(fuelgauge);
	if ((sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT) & 0x0020) && (fuelgauge->info.avg_current > 0)) {
		fuelgauge->info.last_chg_state = 1;
	} else {
		fuelgauge->info.last_chg_state = 0;
	}

	/** for IOCV debug **/
	sm5038_fg_iocv_buffer_read(fuelgauge);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	ret = sm5038_fg_read_sram(fuelgauge, SM5038_FG_ADDR_SRAM_QAGE);
	fuelgauge->qage = ((ret * 1000) >> 11);

	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_AUX_STAT);
	if (ret & 0x0400)
		fuelgauge->current_sense = CURRENT_SENSE_2MOHM;
	else
		fuelgauge->current_sense = CURRENT_SENSE_5MOHM;
#endif
	return true;
}

int sm5038_fg_alert_init(struct sm5038_fg_data *fuelgauge, int soc)
{
	int ret;
	int value_soc_alarm;

	fuelgauge->is_fuel_alerted = false;

	/* check status */
	ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_STATUS);
	if (ret < 0) {
		pr_err("%s: Failed to read SM5038_FG_REG_STATUS\n", __func__);
		return -EPERM;
	}

    value_soc_alarm = soc;

	pr_info("%s: fg_irq= 0x%x, INT_STATUS=0x%x, SOC_ALARM=0x%x \n",
		__func__, fuelgauge->pdata->fg_irq, ret, value_soc_alarm);

	return 1;
}

bool sm5038_fg_fuelalert_init(struct sm5038_fg_data *fuelgauge,
				int soc)
{
	/* Set sm5038 alert configuration. */
	if (sm5038_fg_alert_init(fuelgauge, soc) > 0)
		return true;
	else
		return false;
}

void sm5038_fg_fuelalert_set(struct sm5038_fg_data *fuelgauge,
				int enable)
{
	// do more action..
	u16 ret;

	if (enable) {
		ret = sm5038_read_word(fuelgauge->i2c, SM5038_FG_REG_STATUS);
		pr_info("%s: SM5038_FG_REG_STATUS(0x%x)\n", __func__, ret);
	}

	return;
}

bool sm5038_fg_fuelalert_process(void *irq_data)
{
	struct sm5038_fg_data *fuelgauge =
		(struct sm5038_fg_data *)irq_data;

	sm5038_fg_fuelalert_set(fuelgauge, 0);

	return true;
}

#if 0
static void sm5038_fg_isr_work(struct work_struct *work)
{
	struct sm5038_fg_data *fuelgauge =
		container_of(work, struct sm5038_fg_data, isr_work.work);

	/* process for fuel gauge chip */
	sm5038_fg_fuelalert_process(fuelgauge);

	__pm_relax(fuelgauge->fuel_alert_ws);
}

static irqreturn_t sm5038_fg_irq_thread(int irq, void *irq_data)
{
	struct sm5038_fg_data *fuelgauge = irq_data;

	pr_info("%s\n", __func__);

	if (fuelgauge->is_fuel_alerted) {
		return IRQ_HANDLED;
	} else {
		__pm_stay_awake(fuelgauge->fuel_alert_ws);
		fuelgauge->is_fuel_alerted = true;
		schedule_delayed_work(&fuelgauge->isr_work, 0);
	}

	return IRQ_HANDLED;
}
#endif

static int sm5038_fuelgauge_debugfs_show(struct seq_file *s, void *data)
{
	struct sm5038_fg_data *fuelgauge = s->private;
	int i;
	u8 reg;
	u8 reg_data;

	seq_printf(s, "SM5038 FUELGAUGE IC :\n");
	seq_printf(s, "===================\n");
	for (i = 0; i < 16; i++) {
		if (i == 12)
			continue;
		for (reg = 0; reg < 0x10; reg++) {
			reg_data = sm5038_read_word(fuelgauge->i2c, reg + i * 0x10);
			seq_printf(s, "0x%02x:\t0x%04x\n", reg + i * 0x10, reg_data);
		}
		if (i == 4)
			i = 10;
	}
	seq_printf(s, "\n");
	return 0;
}

static int sm5038_fuelgauge_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, sm5038_fuelgauge_debugfs_show, inode->i_private);
}

static const struct file_operations sm5038_fuelgauge_debugfs_fops = {
	.open			= sm5038_fuelgauge_debugfs_open,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};

#ifdef CONFIG_OF
#define PROPERTY_NAME_SIZE 128

#define PINFO(format, args...) \
	printk(KERN_INFO "%s() line-%d: " format, \
		__func__, __LINE__, ## args)

static int sm5038_fuelgauge_parse_dt(struct sm5038_fg_data *fuelgauge)
{
	char prop_name[PROPERTY_NAME_SIZE];
	int battery_id = -1;
	int table[24];
	int rs_value[7];
	int battery_type[3];
//	int v_alarm[2];
//	int set_temp_poff[4];
	int i_cal[8];
	int v_cal[7];
	int aux_ctrl[2];
	char dt_msg[512];


	int ret;
	int i, j;
//	const u32 *p;
//	int len;

	struct device_node *np = of_find_node_by_name(NULL, "sm5038-fuelgauge");

	/* reset, irq gpio info */
	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
	} else {
		ret = of_property_read_u32(np, "fuelgauge,capacity_max",
				&fuelgauge->pdata->capacity_max);
		if (ret < 0)
			pr_err("%s error reading capacity_max %d\n", __func__, ret);

		ret = of_property_read_u32(np, "fuelgauge,capacity_min",
				&fuelgauge->pdata->capacity_min);
		if (ret < 0)
			pr_err("%s error reading capacity_min %d\n", __func__, ret);

		pr_info("%s: capacity_max: %d, "
				"capacity_min: %d\n", __func__, fuelgauge->pdata->capacity_max,
				fuelgauge->pdata->capacity_min);
	}

	/* get battery_params node for reg init */
	np = of_find_node_by_name(of_node_get(np), "battery_params");
	if (np == NULL) {
		PINFO("Cannot find child node \"battery_params\"\n");
		return -EINVAL;
	}

	/* get battery_id */
	if (of_property_read_u32(np, "battery,id", &battery_id) < 0)
		PINFO("not battery,id property\n");
	PINFO("battery id = %d\n", battery_id);

	/* get battery_table */
	for (i = DISCHARGE_TABLE; i < TABLE_MAX; i++) {
		  snprintf(prop_name, PROPERTY_NAME_SIZE,
					 "battery%d,%s%d", battery_id, "battery_table", i);

		  ret = of_property_read_u32_array(np, prop_name, table, 24);
		  if (ret < 0) {
					 PINFO("Can get prop %s (%d)\n", prop_name, ret);
		  }
		  for (j = 0; j <= FG_TABLE_LEN; j++) {
					 fuelgauge->init_data.battery_table[i][j] = table[j];
					 //PINFO("%s = <table[%d][%d] 0x%x>\n", prop_name, i, j, table[j]);
		  }
		  sprintf(dt_msg,"%s = table ",prop_name);
		  for (j = 0; j <= FG_TABLE_LEN; j++) {
			sprintf(dt_msg+strlen(dt_msg),"[%d]0x%x,",j,table[j]); 
		  }
		  PINFO("%s\n", dt_msg);
	}

	memset(dt_msg, 0x0, sizeof(dt_msg));
	snprintf(prop_name, PROPERTY_NAME_SIZE,
			"battery%d,%s%d", battery_id, "battery_table", i);

	ret = of_property_read_u32_array(np, prop_name, table, 16);
	if (ret < 0) {
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	}
	sprintf(dt_msg,"%s = table ",prop_name);
	
	for (j = 0; j <= FG_ADD_TABLE_LEN; j++) {
		fuelgauge->init_data.battery_table[i][j] = table[j];
		sprintf(dt_msg+strlen(dt_msg),"[%d]0x%x,",j,table[j]); 
		// PINFO("%s = <table[%d][%d] 0x%x>\n", prop_name, i, j, table[j]);
	}
	PINFO("%s\n", dt_msg);

	/* get rs_value */
	for (i = 0; i < 7; i++) {
		snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "rs_value");
		ret = of_property_read_u32_array(np, prop_name, rs_value, 7);
		if (ret < 0) {
			PINFO("Can get prop %s (%d)\n", prop_name, ret);
		}
		fuelgauge->init_data.rs_value[i] = rs_value[i];
	}
	/*spare min max factor chg_factor dischg_factor manvalue*/
	PINFO("%s = <0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x>\n", prop_name,
		rs_value[0], rs_value[1], rs_value[2], rs_value[3], rs_value[4], rs_value[5], rs_value[6]);

	/* battery_type */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "battery_type");
	ret = of_property_read_u32_array(np, prop_name, battery_type, 3);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->init_data.min_vbat = battery_type[0];
	fuelgauge->init_data.max_vbat = battery_type[1];
	fuelgauge->init_data.q_max = battery_type[2];

	PINFO("%s = <min_vbat=%d max_vbat=%d 0x%x>\n", prop_name,
		fuelgauge->init_data.min_vbat, fuelgauge->init_data.max_vbat, fuelgauge->init_data.q_max);

	/* V_ALARM
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "v_alarm");
	ret = of_property_read_u32_array(np, prop_name, v_alarm, 2);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->init_data.value_v_alarm = v_alarm[0];
	fuelgauge->init_data.value_v_alarm_hys = v_alarm[1];
	PINFO("%s = <%d %d>\n", prop_name, fuelgauge->info.value_v_alarm, fuelgauge->info.value_v_alarm_hys);
	*/

    /* TOP OFF */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "topoff");
    ret = of_property_read_u32_array(np, prop_name, &fuelgauge->init_data.top_off, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
    PINFO("%s = <%d>\n", prop_name, fuelgauge->init_data.top_off);

	/* i CAL */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "i_cal");
	ret = of_property_read_u32_array(np, prop_name, i_cal, 8);
	if (ret < 0) {
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	} else {
		fuelgauge->init_data.i_dp_default = i_cal[0];
		fuelgauge->init_data.dp_i_off = i_cal[1];
		fuelgauge->init_data.dp_i_pslo = i_cal[2];
		fuelgauge->init_data.dp_i_nslo = i_cal[3];
		fuelgauge->init_data.i_alg_default = i_cal[4];
		fuelgauge->init_data.alg_i_off = i_cal[5];
		fuelgauge->init_data.alg_i_pslo = i_cal[6];
		fuelgauge->init_data.alg_i_nslo = i_cal[7];
	}
	PINFO("%s = dp : [%d]<0x%x 0x%x 0x%x> alg : [%d]<0x%x 0x%x 0x%x>\n", prop_name,
		fuelgauge->init_data.i_dp_default, fuelgauge->init_data.dp_i_off, fuelgauge->init_data.dp_i_pslo, fuelgauge->init_data.dp_i_nslo
		, fuelgauge->init_data.i_alg_default, fuelgauge->init_data.alg_i_off, fuelgauge->init_data.alg_i_pslo, fuelgauge->init_data.alg_i_nslo);

	/* v CAL */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "v_cal");
	ret = of_property_read_u32_array(np, prop_name, v_cal, 7);
	if (ret < 0) {
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	} else {
	    fuelgauge->init_data.v_default = v_cal[0];
		fuelgauge->init_data.v_off = v_cal[1];
		fuelgauge->init_data.v_slo = v_cal[2];
		fuelgauge->init_data.vt_default = v_cal[3];
		fuelgauge->init_data.vtt = v_cal[4];
		fuelgauge->init_data.ivt = v_cal[5];
		fuelgauge->init_data.ivv = v_cal[6];
	}
	PINFO("%s = v : [%d]<0x%x 0x%x> vt : [%d]<0x%x 0x%x 0x%x>\n", prop_name,
		fuelgauge->init_data.v_default, fuelgauge->init_data.v_off, fuelgauge->init_data.v_slo
		, fuelgauge->init_data.vt_default, fuelgauge->init_data.vtt, fuelgauge->init_data.ivt, fuelgauge->init_data.ivv);

	/* temp_std */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "temp_std");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->init_data.temp_std, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <%d>\n", prop_name, fuelgauge->init_data.temp_std);

	/* tem poff level
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "tem_poff");
	ret = of_property_read_u32_array(np, prop_name, set_temp_poff, 4);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->init_data.n_tem_poff = set_temp_poff[0];
	fuelgauge->init_data.n_tem_poff_offset = set_temp_poff[1];
	fuelgauge->init_data.l_tem_poff = set_temp_poff[2];
	fuelgauge->init_data.l_tem_poff_offset = set_temp_poff[3];

	PINFO("%s = <%d, %d, %d, %d>\n",
		prop_name,
		fuelgauge->init_data.n_tem_poff, fuelgauge->init_data.n_tem_poff_offset,
		fuelgauge->init_data.l_tem_poff, fuelgauge->init_data.l_tem_poff_offset);
	*/

	/* aux_ctrl setting */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "aux_ctrl");
	ret = of_property_read_u32_array(np, prop_name, aux_ctrl, 2);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->init_data.aux_ctrl[0] = aux_ctrl[0];
	fuelgauge->init_data.aux_ctrl[1] = aux_ctrl[1];

	/* batt data version */
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "data_ver");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->init_data.data_ver, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <%d>\n", prop_name, fuelgauge->init_data.data_ver);

	return 0;
}
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#if defined(CONFIG_DEBUG_FS)
static int debug_mask_get(void *data, u64 *val)
{
	struct sm5038_fg_data *fuelgauge = data;

	*val = (u64)fuelgauge->debug_mask;

	return 0;
}
static int debug_mask_set(void *data, u64 val)
{
	struct sm5038_fg_data *fuelgauge = data;

	fuelgauge->debug_mask = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(debug_mask_ops, debug_mask_get, debug_mask_set,
								"%d\n");

static int reg_addr_get(void *data, u64 *val)
{
	struct sm5038_fg_data *fuelgauge = data;

	*val = (u64)fuelgauge->dfs_reg_addr;

	return 0;
}
static int reg_addr_set(void *data, u64 val)
{
	struct sm5038_fg_data *fuelgauge = data;

	fuelgauge->dfs_reg_addr = (u8)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_addr_ops, reg_addr_get, reg_addr_set,
								"0x%02llx\n");

static int reg_data_get(void *data, u64 *val)
{
	struct sm5038_fg_data *fuelgauge = data;
	u16 reg_data;

	reg_data = sm5038_read_word(fuelgauge->i2c,
						fuelgauge->dfs_reg_addr);
	*val = (u64)reg_data;

	return 0;
}
static int reg_data_set(void *data, u64 val)
{
	struct sm5038_fg_data *fuelgauge = data;
	u16 reg_data;

	reg_data = (u16)val;
	sm5038_write_word(fuelgauge->i2c, fuelgauge->dfs_reg_addr, reg_data);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_data_ops, reg_data_get, reg_data_set,
								"0x%04llx\n");

static int regdump_en_get(void *data, u64 *val)
{
	struct sm5038_fg_data *fuelgauge = data;

	*val = (u64)fuelgauge->regdump_en;

	return 0;
}
static int regdump_en_set(void *data, u64 val)
{
	struct sm5038_fg_data *fuelgauge = data;

	fuelgauge->regdump_en = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(regdump_en_ops, regdump_en_get, regdump_en_set,
								"%d\n");

static void somc_sm5038_create_debugfs(struct sm5038_fg_data *fuelgauge)
{
	struct dentry *file;
	struct dentry *dir;

	dir = debugfs_create_dir("sm5038_fg", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		pr_err("Couldn't create sm5038_fg debugfs rc=%ld\n",
								(long)dir);
		return;
	}

	file = debugfs_create_file("debug_mask", S_IFREG | 0644,
					dir, fuelgauge, &debug_mask_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create debug_mask file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_addr", S_IFREG | 0644,
						dir, fuelgauge, &reg_addr_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_addr file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_data", S_IFREG | 0644,
						dir, fuelgauge, &reg_data_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_data file rc=%ld\n", (long)file);

	file = debugfs_create_file("regdump_en", S_IFREG | 0644,
					dir, fuelgauge, &regdump_en_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create regdump_en file rc=%ld\n", (long)file);
}
#else
static void somc_sm5038_create_debugfs(struct sm5038_fg_data *fuelgauge)
{
}

#endif
#endif
int sm5038_fuelgauge_probe(struct sm5038_dev *sm5038)
{
	//struct sm5038_dev *sm5038 = dev_get_drvdata(pdev->dev.parent);
	//struct sm5038_platform_data *pdata = dev_get_platdata(sm5038->dev);
	struct sm5038_fg_data *fuelgauge;
	sm5038_fuelgauge_platform_data_t *fuelgauge_data;
	int ret = 0;

	pr_info("%s: SM5038 Fuelgauge Driver Loading probe start\n", __func__);

	fuelgauge = kzalloc(sizeof(*fuelgauge), GFP_KERNEL);
	if (!fuelgauge)
		return -ENOMEM;

	fuelgauge_data = kzalloc(sizeof(sm5038_fuelgauge_platform_data_t), GFP_KERNEL);
	if (!fuelgauge_data) {
		ret = -ENOMEM;
		goto err_free;
	}

	/* save platfom data for fg */
	static_fg_data = NULL;

	mutex_init(&fuelgauge->fg_lock);

	fuelgauge->dev = sm5038->dev;
	fuelgauge->pdata = fuelgauge_data;
	fuelgauge->i2c = sm5038->fuelgauge_i2c;
	fuelgauge->sm5038_pdata = sm5038->pdata;

#if defined(CONFIG_OF)
	ret = sm5038_fuelgauge_parse_dt(fuelgauge);
	if (ret < 0) {
		pr_err("%s not found fuelgauge dt! ret[%d]\n",
				__func__, ret);
	}
#endif

	//platform_set_drvdata(pdev, fuelgauge);

	if (fuelgauge->i2c == NULL) {
		pr_err("%s : i2c NULL\n", __func__);
		goto err_data_free;
	}

	(void) debugfs_create_file("sm5038-fuelgauge-regs",
		S_IRUGO, NULL, (void *)fuelgauge, &sm5038_fuelgauge_debugfs_fops);

	/* cycle init */
	fuelgauge->info.batt_soc_cycle = 0;
	if (!sm5038_fg_init(fuelgauge, false)) {
		pr_err("%s: Failed to Initialize Fuelgauge\n", __func__);
		goto err_data_free;
	}

	fuelgauge->fg_irq = fuelgauge->sm5038_pdata->irq_base + SM5038_FG_IRQ_INT_LOW_VOLTAGE;
	pr_info("[%s]IRQ_BASE(%d) FG_IRQ(%d)\n",
		__func__, fuelgauge->sm5038_pdata->irq_base, fuelgauge->fg_irq);

	fuelgauge->is_fuel_alerted = false;
	if (fuelgauge->pdata->fuel_alert_soc >= 0) {
		if (sm5038_fg_fuelalert_init(fuelgauge,
					fuelgauge->pdata->fuel_alert_soc)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
			//wakeup_source_init(fuelgauge->fuel_alert_ws, "fuel_alerted");
			if (!(fuelgauge->fuel_alert_ws)) {
				fuelgauge->fuel_alert_ws = wakeup_source_create("fuel_alerted"); // 4.19 Q
				if (fuelgauge->fuel_alert_ws)
					wakeup_source_add(fuelgauge->fuel_alert_ws);
			}
#else
			fuelgauge->fuel_alert_ws = wakeup_source_register(NULL, "fuel_alerted"); // 5.4 R
#endif
#if 0
			if (fuelgauge->fg_irq) {
				INIT_DELAYED_WORK(&fuelgauge->isr_work, sm5038_fg_isr_work);

				ret = request_threaded_irq(fuelgauge->fg_irq,
						NULL, sm5038_fg_irq_thread,
						0,
						"fuelgauge-irq", fuelgauge);
				if (ret) {
					pr_err("%s: Failed to Request IRQ\n", __func__);
					goto err_supply_unreg;
				}
			}
#endif			
		} else {
			pr_err("%s: Failed to Initialize Fuel-alert\n",
					__func__);
			goto err_supply_unreg;
		}
	}

	static_fg_data = fuelgauge;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_sm5038_create_debugfs(fuelgauge);
	fuelgauge->debug_mask = PR_CORE;
#endif
	pr_info("%s: SM5038 Fuelgauge Driver Loading probe done\n", __func__);

	return 0;

err_supply_unreg:
err_data_free:
#if defined(CONFIG_OF)
#endif
//err_pdata_free:
	kfree(fuelgauge_data);
	mutex_destroy(&fuelgauge->fg_lock);
err_free:
	kfree(fuelgauge);

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_fuelgauge_probe);



int sm5038_fuelgauge_remove(void)
{
	struct sm5038_fg_data *fuelgauge = static_fg_data;

	mutex_destroy(&fuelgauge->fg_lock);

	kfree(fuelgauge);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_fuelgauge_remove);

//static int sm5038_fuelgauge_suspend(struct device *dev)
//{
//	return 0;
//}

//static int sm5038_fuelgauge_resume(struct device *dev)
//{
//	struct sm5038_fg_data *fuelgauge = dev_get_drvdata(dev);
//
//	return 0;
//}

//static void sm5038_fuelgauge_shutdown(struct platform_device *pdev)
//{
////	struct sm5038_fg_data *fuelgauge = platform_get_drvdata(pdev);
//
//	pr_info("%s: ++\n", __func__);
//
//	pr_info("%s: --\n", __func__);
//}

//static SIMPLE_DEV_PM_OPS(sm5038_fuelgauge_pm_ops, sm5038_fuelgauge_suspend,
//			sm5038_fuelgauge_resume);

//static struct platform_driver sm5038_fuelgauge_driver = {
//	.driver = {
//			.name = "sm5038-fuelgauge",
//			.owner = THIS_MODULE,
//#ifdef CONFIG_PM
//			.pm = &sm5038_fuelgauge_pm_ops,
//#endif
//	},
//	.probe  = sm5038_fuelgauge_probe,
//	.remove = sm5038_fuelgauge_remove,
//	.shutdown = sm5038_fuelgauge_shutdown,
//};

//static int __init sm5038_fuelgauge_init(void)
//{
//	pr_info("%s: \n", __func__);
//	return platform_driver_register(&sm5038_fuelgauge_driver);
//}

//static void __exit sm5038_fuelgauge_exit(void)
//{
//	platform_driver_unregister(&sm5038_fuelgauge_driver);
//}
//module_init(sm5038_fuelgauge_init);
//module_exit(sm5038_fuelgauge_exit);


//MODULE_AUTHOR("Jake JUNG");
//MODULE_DESCRIPTION("SM5038 Fuel Gauge");

MODULE_LICENSE("GPL");
