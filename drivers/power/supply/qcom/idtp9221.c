/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#define pr_fmt(fmt) "IDTP: %s: " fmt, __func__

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/power_supply.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>

/* IDT P9221 Registers */
#define IDTP9221_REG_CHIP_ID_L			0x00
#define IDTP9221_REG_CHIP_ID_H			0x01
#define IDTP9221_REG_CHIP_REV			0x02
#define IDTP9221_REG_CHIP_REV_MASK		GENMASK(4, 7)
#define IDTP9221_REG_CHIP_MINOR_REV_MASK	GENMASK(0, 3)
#define IDTP9221_REG_CUSTOMER_ID		0x03
#define IDTP9221_REG_OTP_FW_MAJOR_REV_L		0x04
#define IDTP9221_REG_OTP_FW_MAJOR_REV_H		0x05
#define IDTP9221_REG_OTP_FW_MINOR_REV_L		0x06
#define IDTP9221_REG_OTP_FW_MINOR_REV_H		0x07
#define IDTP9221_REG_OTP_FW_DATE_CODE		0x08
#define IDTP9221_REG_OTP_FW_TIME_CODE		0x14
#define IDTP9221_REG_SRAM_FW_MAJOR_REV_L	0x1C
#define IDTP9221_REG_SRAM_FW_MAJOR_REV_H	0x1D
#define IDTP9221_REG_SRAM_FW_MINOR_REV_L	0x1E
#define IDTP9221_REG_SRAM_FW_MINOR_REV_H	0x1F
#define IDTP9221_REG_SRAM_FW_DATE_CODE		0x20
#define IDTP9221_REG_SRAM_FW_TIME_CODE		0x2C
#define IDTP9221_REG_STATUS_L			0x34
#define IDTP9221_REG_STAT_VOUT_BIT		BIT(7)
#define IDTP9221_REG_STAT_VRECT_BIT		BIT(6)
#define IDTP9221_REG_STAT_ACMISSING_BIT		BIT(5)
#define IDTP9221_REG_TX_DATA_RECEIVED_BIT	BIT(4)
#define IDTP9221_REG_STAT_THERM_BIT		BIT(3)
#define IDTP9221_REG_OVER_TEMPERATURE_BIT	BIT(2)
#define IDTP9221_REG_OVER_VOLTAGE_BIT		BIT(1)
#define IDTP9221_REG_OVER_CURRENT_BIT		BIT(0)
#define IDTP9221_REG_STATUS_H			0x35
#define IDTP9221_REG_AUTH_BIT			BIT(0)
#define IDTP9221_REG_INT_L			0x36
#define IDTP9221_REG_INT_H			0x37
#define IDTP9221_REG_INT_ENABLE_L		0x38
#define IDTP9221_REG_INT_ENABLE_H		0x39
#define IDTP9221_REG_CHARGE_STATUS		0x3A
#define IDTP9221_REG_E_REASON			0x3B
#define IDTP9221_REG_ADC_VOUT_L			0x3C
#define IDTP9221_REG_ADC_VOUT_H			0x3D
#define IDTP9221_REG_VOUT_SET			0x3E
#define IDTP9221_REG_VRECT_ADJ			0x3F
#define IDTP9221_REG_ADC_VRECT_L		0x40
#define IDTP9221_REG_ADC_VRECT_H		0x41
#define IDTP9221_REG_OVSET			0x42
#define IDTP9221_REG_MPRNREQ			0x43
#define IDTP9221_REG_RX_IOUT_L			0x44
#define IDTP9221_REG_RX_IOUT_H			0x45
#define IDTP9221_REG_ADC_DIE_TEMP_L		0x46
#define IDTP9221_REG_ADC_DIE_TEMP_H		0x47
#define IDTP9221_REG_OP_FREQ_L			0x48
#define IDTP9221_REG_OP_FREQ_H			0x49
#define IDTP9221_REG_ILIM_SET			0x4A
#define IDTP9221_REG_SYS_OP_MODE		0x4D
#define IDTP9221_REG_SYS_OP_MODE_MASK		GENMASK(0, 3)
#define IDTP9221_REG_SYS_OP_MODE_WPC_BIT	BIT(0)
#define IDTP9221_REG_SYS_OP_MODE_PMA_BIT	BIT(1)
#define IDTP9221_REG_SYS_OP_MODE_ACM_BIT	BIT(2)
#define IDTP9221_REG_SYS_OP_MODE_WPCEPP_BIT	BIT(3)
#define IDTP9221_REG_FW_SRC__MASK		GENMASK(6, 4)
#define IDTP9221_REG_FW_SRC_OTP_BIT		BIT(4)
#define IDTP9221_REG_FW_SRC_EEPROM_BIT		BIT(5)
#define IDTP9221_REG_FW_SRC_RAM_BIT		BIT(6)
#define IDTP9221_REG_COM			0x4E
#define IDTP9221_REG_ADC_VTHERM_L		0x64
#define IDTP9221_REG_ADC_VTHERM_H		0x65
#define IDTP9221_REG_VTHERM_SET_L		0x66
#define IDTP9221_REG_VTHERM_SET_H		0x67
#define IDTP9221_REG_FOD_0_A			0x68
#define IDTP9221_REG_FOD_0_B			0x69
#define IDTP9221_REG_FOD_1_A			0x6A
#define IDTP9221_REG_FOD_1_B			0x6B
#define IDTP9221_REG_FOD_2_A			0x6C
#define IDTP9221_REG_FOD_2_B			0x6D
#define IDTP9221_REG_FOD_3_A			0x6E
#define IDTP9221_REG_FOD_3_B			0x6F
#define IDTP9221_REG_FOD_4_A			0x70
#define IDTP9221_REG_FOD_4_B			0x71
#define IDTP9221_REG_FOD_5_A			0x72
#define IDTP9221_REG_FOD_5_B			0x73
#define IDTP9221_REG_FOD_EXPANSION_A		0x74
#define IDTP9221_REG_FOD_EXPANSION_B		0x75
#define IDTP9221_REG_FOD_EPP_MODE_1_A		0x76
#define IDTP9221_REG_FOD_EPP_MODE_1_B		0x77
#define IDTP9221_REG_RPP_L			0x88
#define IDTP9221_REG_RPP_H			0x89
#define IDTP9221_REG_SIGSTR			0x8A
#define IDTP9221_REG_TXID_L			0xF4
#define IDTP9221_REG_TXID_H			0xF5
#define IDTP9221_REG_PING_FREQ_L		0xFC
#define IDTP9221_REG_PING_FREQ_H		0xFD

#define FOD_REG_START_ADDR	IDTP9221_REG_FOD_0_A
#define FOD_REG_END_ADDR	IDTP9221_REG_FOD_EPP_MODE_1_B
#define FOD_REG_NUM		(FOD_REG_END_ADDR - FOD_REG_START_ADDR + 1)

#define IDTP9221_ADC_TO_VRECT_MV(adc)	(adc * 21000 / 4095)
#define IDTP9221_ADC_TO_VOUT_MV(adc)	(adc * 12600 / 4095)
#define IDTP9221_ADC_TO_MA(adc)		(adc * 1)
#define IDTP9221_ADC_TO_DIE_TEMP_C(adc)	((adc - 1350) * 83 / 444 - 273)
#define IDTP9221_ADC_TO_VTHERM_UV(adc)	(adc * (2100000 / 4095))
#define IDTP9221_ADC_TO_FREQ_KZ(adc)	(adc ? 64 * 6000 / adc : 0)
#define IDTP9221_ADC_TO_RPP_BPP_MW(adc)	(adc * 5000 / 32768)
#define IDTP9221_ADC_TO_RPP_EPP_MW(adc)	(adc * 10000 / 32768)
#define IDTP9221_ADC_TO_SIGSTR_PC(adc)	DIV_ROUND_CLOSEST(adc * 100, 256)

/* calc vtherm_temp_dC */
#define IDTP9221_LOG_CARRY               10000
#define IDTP9221_LOG_BORROW              100000
#define IDTP9221_LOG_2_10000             132877
#define IDTP9221_INVERSE_LOG_2_E         69315

#define IDTP9221_INVERSE_4250            23529
#define IDTP9221_INVERSE_298             335571
#define IDTP9221_INVERSE_CARRY           100000000
#define IDTP9221_INVERSE_BORROW          10000
#define IDTP9221_VTHERM_UV_MAX           1800000
#define IDTP9221_VTHERM_TEMP_MAX         2500
#define IDTP9221_OUTPUT_PRECISION        10
#define IDTP9221_ABSOLUTE_TEMPERATURE    (273 * IDTP9221_OUTPUT_PRECISION)
#define IDTP9221_BIT_ACCURACY            5

#define IDTP9221_REAL(antilog, msb)	\
		((antilog ^ (0x01 << msb)) >> (msb - IDTP9221_BIT_ACCURACY))

const int idtp9221_log_table_5bit[32] = {
	0, 439, 875, 1293, 1699, 2095, 2479, 2854,
	3219, 3576, 3923, 4263, 4594, 4919, 5236, 5549,
	5850, 6147, 6439, 6724, 7004, 7279, 7549, 7813,
	8074, 8329, 8580, 8826, 9069, 9307, 9542, 9773,
};

#define IDTP9221_INTERVAL_POWER_OK_DEFAULT_MS		3000
#define IDTP9221_INTERVAL_NEGOTIATION_DEFAULT_MS	50

enum {
	IDTP9221_ENABLE_CLIENT_DEBUG = 0,
	IDTP9221_ENABLE_CLIENT_USB,
	IDTP9221_ENABLE_CLIENT_DEV1,
	IDTP9221_ENABLE_CLIENT_THERM_WA,
	IDTP9221_ENABLE_CLIENT_MAX,
};

enum {
	IDTP9221_STATUS_PWR_OFF = 0,
	IDTP9221_STATUS_PWR_ON_NEGOTIATION,
	IDTP9221_STATUS_PWR_POWER_OK,
};

static const char * const str_status[] = {"POFF", "NEGO", "PWON"};

enum {
	IDTP9221_PSY_WIRELESS_MODE_OFF = 0,
	IDTP9221_PSY_WIRELESS_MODE_BPP,
	IDTP9221_PSY_WIRELESS_MODE_EPP,
	IDTP9221_PSY_WIRELESS_MODE_UNKNOWN,
	IDTP9221_PSY_WIRELESS_MODE_UNSETTLED,
};

enum {
	IDTP9221_OP_MODE_UNKNOWN = 0,
	IDTP9221_OP_MODE_QI_BPP,
	IDTP9221_OP_MODE_QI_EPP,
	IDTP9221_OP_MODE_PMA,
};

enum {
	IDTP9221_FW_SRC_NONE = 0,
	IDTP9221_FW_SRC_OTP,
	IDTP9221_FW_SRC_EEPROM,
	IDTP9221_FW_SRC_RAM,
};

struct idtp9221 {
	struct device		*dev;
	struct i2c_client	*i2c_handle;

	/* power supply */
	struct power_supply	*wireless_psy;
	struct power_supply	*dc_psy;
	struct power_supply	*usb_psy;
	struct power_supply	*batt_psy;
	int			usbin_valid;
	int			wireless_suspend;

	/* gpio/irq */
	int			wlc_en;
	int			wlc_en_rsvd;
	int			wlc_irq;
	int			wlc_gpio_irq;

	/* worker */
	struct delayed_work	detect_work;
	struct delayed_work	vout_change_work;
	struct mutex		reg_ctrl_lock;
	struct mutex		i2c_lock;
	bool			is_worker_active;
	bool			wake_lock_en;

	/* wlc_en control */
	bool			halt_client[IDTP9221_ENABLE_CLIENT_MAX];
	bool			halt_retry;
	bool			enabled;
	bool			halt_for_debug;

	/* read resister items */
	int			fw_rev_major_l;
	int			fw_rev_major_h;
	int			fw_rev_minor_l;
	int			fw_rev_minor_h;
	int			fw_rev_sram_major_l;
	int			fw_rev_sram_major_h;
	int			fw_rev_sram_minor_l;
	int			fw_rev_sram_minor_h;
	int			fw_src;
	int			op_mode;
	int			vrect_mv;
	int			vout_mv;
	int			iout_ma;
	int			die_temp_c;
	int			op_freq_khz;
	int			vtherm_temp_dc;
	long			vtherm_uv;
	int			rpp_raw;
	int			rpp_mw;
	int			sigstr_pc;
	int			txid;
	int			ping_freq_khz;

	/* write resister items */
	int			vout_set_mv;
	int			ilim_set_ma;

	/* misc ctrl */
	int			thermal_max_voltage;
	int			target_vout_set_mv;

	/* status */
	int			status;
	int			psy_wireless_mode;
	int			old_status;
	int			wireless_auth;
	bool			one_time_read_in_driver_done;
	bool			one_time_read_in_pwr_ok_done;

	/* debug */
	int			*debug_mask;
	u8			reg_shadow[256];
	u8			reg_ept;
	int			interval_power_ok_ms;
	int			interval_negotiation_ms;
	bool			disable_halt;
	int			monitor_for_debug;
	int			debug_vout_set_mv;
	int			debug_ilim_set_ma;
	u8			fod_adj[FOD_REG_NUM];
	bool			fod_adj_en;
	u8			cmd_reg_addr;
	u8			cmd_reg_data;

	/* configration from device tree */
	int			bpp_ilim_ma;
	int			epp_ilim_ma;
	int			epp_boost_ilim_ma;
	int			bpp_vout_mv;
	int			epp_vout_mv;
};

#define idtp9221_err(chip, fmt, ...)	pr_err(fmt, ##__VA_ARGS__)
#define idtp9221_dbg(chip, reason, fmt, ...)				\
	do {								\
		if (*chip->debug_mask & (reason))			\
			pr_err(fmt, ##__VA_ARGS__);		\
		else							\
			pr_debug(fmt, ##__VA_ARGS__);		\
	} while (0)

/* Debug flag definitions */
enum idtp9221_debug_flag {
	PR_IRQ		= BIT(0),
	PR_MONITOR	= BIT(1),
	PR_STATUS	= BIT(2),
	PR_REG_I2C	= BIT(3),
	PR_WORKER	= BIT(4),
	PR_PSY		= BIT(5),
	PR_MISC		= BIT(6),
	PR_MEASUREMENT	= BIT(7),
};

static int idtp9221_debug_mask = (PR_IRQ | PR_MONITOR | PR_STATUS |
				PR_WORKER | PR_MISC | PR_PSY | PR_MEASUREMENT);
static int idtp9221_debug_dump;
module_param_named(debug_mask, idtp9221_debug_mask, int, 0600);
module_param_named(reg_dump, idtp9221_debug_dump, int, 0600);

#define IDTP9221_I2C_RETRY_MAX 3
#define IDTP9221_I2C_RETRY_DELAY_MS 5
static int idtp9221_i2c_write_byte(struct idtp9221 *chip, u8 reg, u8 val)
{
	int rc = 0;
	int cnt = 0;
	struct i2c_client *client = chip->i2c_handle;
	unsigned char buf[3];
	int length = 3;

	buf[0] = 0x00;
	buf[1] = reg;
	buf[2] = val;

	mutex_lock(&chip->i2c_lock);
	while (cnt < IDTP9221_I2C_RETRY_MAX) {
		rc = i2c_master_send(client, buf, length);
		if (rc < length) {
			dev_err(chip->dev, "Can't write i2c %d\n", rc);
			cnt++;
			msleep(IDTP9221_I2C_RETRY_DELAY_MS);
		} else {
			idtp9221_dbg(chip, PR_REG_I2C,
					"[W] addr 0x%02x : 0x%02x\n", reg, val);
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static int idtp9221_i2c_read_byte(struct idtp9221 *chip, u8 reg, u8 *val)
{
	int rc = 0;
	int cnt = 0;
	struct i2c_msg msg[2];
	u8 wbuf[2];
	struct i2c_client *client = chip->i2c_handle;

	mutex_lock(&chip->i2c_lock);

	msg[0].addr = client->addr;
	msg[0].flags = client->flags & I2C_M_TEN;
	msg[0].len = 2;
	msg[0].buf = wbuf;

	wbuf[0] = 0;
	wbuf[1] = reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = val;

	while (cnt < IDTP9221_I2C_RETRY_MAX) {
		rc = i2c_transfer(client->adapter, msg, 2);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read i2c %d retry=%d\n",
								rc, cnt);
			cnt++;
			msleep(IDTP9221_I2C_RETRY_DELAY_MS);
		} else {
			idtp9221_dbg(chip, PR_REG_I2C,
					"[R] addr 0x%02x : 0x%02x\n",
					reg, *val);
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static int idtp9221_i2c_read_sequential(struct idtp9221 *chip, u8 reg, u8 *val,
								int length)
{
	int rc = 0;
	int cnt = 0;
	struct i2c_msg msg[2];
	u8 wbuf[2];
	struct i2c_client *client = chip->i2c_handle;

	if (length < 1 || reg + length > 256)
		return -ENOMEM;

	mutex_lock(&chip->i2c_lock);

	msg[0].addr = client->addr;
	msg[0].flags = client->flags & I2C_M_TEN;
	msg[0].len = 2;
	msg[0].buf = wbuf;

	wbuf[0] = 0;
	wbuf[1] = reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = val;

	while (cnt < IDTP9221_I2C_RETRY_MAX) {
		rc = i2c_transfer(client->adapter, msg, 2);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read i2c %d retry=%d\n",
								rc, cnt);
			cnt++;
			msleep(IDTP9221_I2C_RETRY_DELAY_MS);
		} else {
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static void idtp9221_dump_reg(struct idtp9221 *chip)
{
	u8 addr = 0;
	int rc = 0;
	char addr_1[24];
	char addr_2[24];

	rc = idtp9221_i2c_read_sequential(chip, 0, chip->reg_shadow, 256);
	if (rc < 0) {
		pr_err("Can't dump reg %d\n", rc);
		return;
	}

	pr_err("Addr|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	while (addr < 256) {
		snprintf(addr_1, 24, "%02X %02X %02X %02X %02X %02X %02X %02X",
				chip->reg_shadow[addr],
				chip->reg_shadow[addr+1],
				chip->reg_shadow[addr+2],
				chip->reg_shadow[addr+3],
				chip->reg_shadow[addr+4],
				chip->reg_shadow[addr+5],
				chip->reg_shadow[addr+6],
				chip->reg_shadow[addr+7]);
		snprintf(addr_2, 24, "%02X %02X %02X %02X %02X %02X %02X %02X",
				chip->reg_shadow[addr+8],
				chip->reg_shadow[addr+9],
				chip->reg_shadow[addr+10],
				chip->reg_shadow[addr+11],
				chip->reg_shadow[addr+12],
				chip->reg_shadow[addr+13],
				chip->reg_shadow[addr+14],
				chip->reg_shadow[addr+15]);

		pr_err("0x%02X|%s %s", addr, addr_1, addr_2);
		addr += 16;
	}
}

static long idtp9221_vtherm_calc_count_bit(long arg)
{
	long count = 0;

	while (arg > 0) {
		count += (arg & 1);
		arg >>= 1;
	}

	return count;
}

static long idtp9221_vtherm_calc_find_msb(long arg)
{
	int shift;
	long msb;

	if (arg == 0)
		return 0;
	for (shift = 1; shift < 32; shift <<= 1)
		arg |= (arg >> shift);

	msb = idtp9221_vtherm_calc_count_bit(arg) - 1;

	return msb;
}

static long idtp9221_vtherm_calc_ln(long antilog)
{
	long msb, ln, log2, log2_integer, log2_real;

	msb = idtp9221_vtherm_calc_find_msb(antilog);

	log2_integer = (long)(msb * IDTP9221_LOG_CARRY);
	log2_real = idtp9221_log_table_5bit[IDTP9221_REAL(antilog, msb)];
	log2 = log2_integer + log2_real - IDTP9221_LOG_2_10000;

	ln = IDTP9221_INVERSE_LOG_2_E * log2 / IDTP9221_LOG_BORROW;

	return ln;
}

static long idtp9221_conv_vtherm_to_celsius(long vadc)
{
	long vtherm_temp, numer, denom, ln, antilog;

	if (vadc < 0 || vadc >= IDTP9221_VTHERM_UV_MAX)
		return -EDOM;

	numer = IDTP9221_INVERSE_CARRY * IDTP9221_OUTPUT_PRECISION;

	antilog = vadc * IDTP9221_LOG_CARRY * 33 / 100 /
				(IDTP9221_VTHERM_UV_MAX - vadc);

	ln = idtp9221_vtherm_calc_ln(antilog);
	denom = IDTP9221_INVERSE_4250 * ln / IDTP9221_INVERSE_BORROW +
			IDTP9221_INVERSE_298;

	vtherm_temp = numer / denom - IDTP9221_ABSOLUTE_TEMPERATURE;

	return vtherm_temp;
}

static int idtp9221_get_op_mode(struct idtp9221 *chip)
{
	u8 reg;
	int rc;

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_SYS_OP_MODE, &reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (reg & IDTP9221_REG_SYS_OP_MODE_WPCEPP_BIT)
		chip->op_mode = IDTP9221_OP_MODE_QI_EPP;
	else if (reg & IDTP9221_REG_SYS_OP_MODE_WPC_BIT)
		chip->op_mode = IDTP9221_OP_MODE_QI_BPP;
	else if (reg & IDTP9221_REG_SYS_OP_MODE_PMA_BIT)
		chip->op_mode = IDTP9221_OP_MODE_PMA;
	else
		chip->op_mode = IDTP9221_OP_MODE_UNKNOWN;

	idtp9221_dbg(chip, PR_STATUS, "op_mode : %d\n", chip->op_mode);

	return 0;
}

#define VOUT_STEP 10
#define VOUT_WAIT_MS 200
static int idtp9221_set_vout_set(struct idtp9221 *chip, int val)
{
	if (chip->debug_vout_set_mv)
		val = chip->debug_vout_set_mv;

	if (val < 3500 || val > 12500)
		return -EINVAL;

	chip->target_vout_set_mv = val;
	schedule_delayed_work(&chip->vout_change_work,
					msecs_to_jiffies(VOUT_WAIT_MS));

	return 0;
}

static int idtp9221_handle_vout_set(struct idtp9221 *chip)
{
	int rc = 0;
	int vout_mv = 0;

	if (chip->status == IDTP9221_STATUS_PWR_POWER_OK) {
		if (chip->op_mode == IDTP9221_OP_MODE_QI_EPP) {
			if (chip->thermal_max_voltage > 0 &&
				chip->thermal_max_voltage < chip->epp_vout_mv)
				vout_mv = chip->thermal_max_voltage;
			else
				vout_mv = chip->epp_vout_mv;
		} else if (chip->op_mode == IDTP9221_OP_MODE_QI_BPP) {
			if (chip->thermal_max_voltage > 0 &&
				chip->thermal_max_voltage < chip->bpp_vout_mv)
				vout_mv = chip->thermal_max_voltage;
			else
				vout_mv = chip->bpp_vout_mv;
		}
		if (vout_mv > 0) {
			rc = idtp9221_set_vout_set(chip, vout_mv);
			if (rc)
				pr_err("Error in set_vout_set rc=%d\n", rc);
		}
	}
	return rc;
}

static void idtp9221_vout_change_work(struct work_struct *work)
{
	struct idtp9221 *chip = container_of(work,
				struct idtp9221,
				vout_change_work.work);
	int rc;
	u8 current_v_reg, target_v_reg;
	int current_v_mv, target_v_mv;
	int val;

	mutex_lock(&chip->reg_ctrl_lock);

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_VOUT_SET,
						&current_v_reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		goto error;
	}
	current_v_mv = current_v_reg * 100 + 3500;
	target_v_mv = (chip->target_vout_set_mv / 100) * 100;
	target_v_reg = (u8)((target_v_mv - 3500) / 100);
	idtp9221_dbg(chip, PR_MISC, "now:%dmV, target:%dmV\n",
					current_v_mv, target_v_mv);

	if (current_v_reg + VOUT_STEP < target_v_reg) {
		/* Increase Vout step by step */
		val = current_v_reg + VOUT_STEP;
		idtp9221_dbg(chip, PR_MISC, "Step Vout(Inc): 0x%02x\n", val);
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_VOUT_SET, val);
		if (rc < 0) {
			dev_err(chip->dev, "write error VOUT_SET %d\n", rc);
			goto error;
		}
		goto schedule_next;
	} else if (current_v_reg - VOUT_STEP > target_v_reg) {
		/* Decrease Vout step by step */
		val = current_v_reg - VOUT_STEP;
		idtp9221_dbg(chip, PR_MISC, "Step Vout(Dec): 0x%02x\n", val);
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_VOUT_SET, val);
		if (rc < 0) {
			dev_err(chip->dev, "write error VOUT_SET %d\n", rc);
			goto error;
		}
		goto schedule_next;
	} else if (current_v_reg != target_v_reg) {
		/* Finally set to targert Vout */
		val = target_v_reg;
		idtp9221_dbg(chip, PR_MISC, "Step Vout(Goal): 0x%02x\n", val);
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_VOUT_SET, val);
		if (rc < 0) {
			dev_err(chip->dev, "write error VOUT_SET %d\n", rc);
			goto error;
		}
		chip->vout_set_mv = target_v_mv;
		goto out;
	} else {
		/* Keep Vout */
		goto out;
	}
schedule_next:
	schedule_delayed_work(&chip->vout_change_work,
					msecs_to_jiffies(VOUT_WAIT_MS));
out:
error:
	mutex_unlock(&chip->reg_ctrl_lock);
}

static int idtp9221_set_ilim_set(struct idtp9221 *chip, int val)
{
	int rc;
	u8 current_ilim_reg, target_ilim_reg;

	if (chip->debug_ilim_set_ma)
		val = chip->debug_ilim_set_ma;

	if (val > 1300) {
		pr_err("Invalid ilim parameter val:%d\n", val);
		return -EINVAL;
	}
	target_ilim_reg = val / 100;

	if (chip->status == IDTP9221_STATUS_PWR_OFF)
		return 0;

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_ILIM_SET,
						&current_ilim_reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (current_ilim_reg != target_ilim_reg) {
		idtp9221_dbg(chip, PR_MISC, "set ilim_set to %dmA (from %d)\n",
				target_ilim_reg * 100, current_ilim_reg * 100);
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_ILIM_SET,
							target_ilim_reg);
		if (rc < 0) {
			dev_err(chip->dev, "read error ILIM_SET %d\n", rc);
			return rc;
		}
		chip->ilim_set_ma = val;
	}
	return 0;
}

static int idtp9221_set_fod(struct idtp9221 *chip, int reg_offset, u8 val)
{
	int rc = 0;
	u8 reg_addr;

	reg_addr = FOD_REG_START_ADDR + reg_offset;
	if (reg_addr > FOD_REG_END_ADDR) {
		dev_err(chip->dev,
			"invalid parameter reg_offset:%d\n", reg_offset);
		return rc;
	}

	if (!chip->fod_adj_en)
		return 0;

	idtp9221_dbg(chip, PR_MISC, "set FOD reg:0x%02x, val:0x%02x\n",
								reg_addr, val);

	if (chip->status != IDTP9221_STATUS_PWR_OFF) {
		rc = idtp9221_i2c_write_byte(chip, reg_addr, val);
		if (rc < 0) {
			dev_err(chip->dev, "read error FOD %d\n", rc);
			return rc;
		}
	}

	return rc;
}

static int idtp9221_set_fod_all(struct idtp9221 *chip)
{
	int rc = 0;
	u8 i;

	if (!chip->fod_adj_en)
		return 0;

	for (i = 0; i < FOD_REG_NUM; i++) {
		if (chip->status != IDTP9221_STATUS_PWR_OFF) {
			rc = idtp9221_set_fod(chip, i, chip->fod_adj[i]);
			if (rc < 0) {
				dev_err(chip->dev, "failed set FOD %d\n", rc);
				break;
			}
		}
	}
	return (rc < 0) ? rc : 0;
}

static int idtp9221_read_cmd_reg_byte(struct idtp9221 *chip, u8 addr, u8 *data)
{
	int rc = 0;

	if (chip->status != IDTP9221_STATUS_PWR_OFF) {
		rc = idtp9221_i2c_read_byte(chip, addr, data);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read register %d\n", rc);
			return rc;
		}
	}

	return rc;
}

static int idtp9221_write_cmd_reg_byte(struct idtp9221 *chip, u8 addr, u8 data)
{
	int rc = 0;

	if (chip->status != IDTP9221_STATUS_PWR_OFF) {
		rc = idtp9221_i2c_write_byte(chip, addr, data);
		if (rc < 0) {
			dev_err(chip->dev, "write error register %d\n", rc);
			return rc;
		}
	}

	return rc;
}

#define IDTP9221_EPT_MODE_UNKNOWN	0
#define IDTP9221_EPT_MODE_MAX		8
#define IDTP9221_EPT_SEND_COM		0x08
static int idtp9221_set_ept(struct idtp9221 *chip, int mode)
{
	int rc = 0;

	if (mode <= IDTP9221_EPT_MODE_UNKNOWN || mode > IDTP9221_EPT_MODE_MAX) {
		pr_err("Invalid mode %d\n", mode);
		return -EINVAL;
	}

	if (chip->status != IDTP9221_STATUS_PWR_OFF) {
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_E_REASON, mode);
		if (rc < 0) {
			dev_err(chip->dev, "write error of EPT mode %d\n", rc);
			return rc;
		}
		rc = idtp9221_i2c_write_byte(chip, IDTP9221_REG_COM,
							IDTP9221_EPT_SEND_COM);
		if (rc < 0) {
			dev_err(chip->dev, "write error of EPT send cmd %d\n",
									rc);
			return rc;
		}
		idtp9221_dbg(chip, PR_MISC, "Sent EPT command. mode:%d\n",
									mode);
	}
	return rc;
}

static int idtp9221_read_regs_oneshot_in_driver(struct idtp9221 *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[4];

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_SYS_OP_MODE,
								&reg_buf[0]);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (reg_buf[0] & IDTP9221_REG_FW_SRC_OTP_BIT)
		chip->fw_src = IDTP9221_FW_SRC_OTP;
	else if (reg_buf[0] & IDTP9221_REG_FW_SRC_EEPROM_BIT)
		chip->fw_src = IDTP9221_FW_SRC_EEPROM;
	else if (reg_buf[0] & IDTP9221_REG_FW_SRC_RAM_BIT)
		chip->fw_src = IDTP9221_FW_SRC_RAM;
	else
		chip->fw_src = IDTP9221_FW_SRC_NONE;
	idtp9221_dbg(chip, PR_MISC, "FW Source : %d\n", chip->fw_src);

	start_addr = IDTP9221_REG_OTP_FW_MAJOR_REV_L;
	length = 4;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->fw_rev_major_l = reg_buf[0];
	chip->fw_rev_major_h = reg_buf[1];
	chip->fw_rev_minor_l = reg_buf[2];
	chip->fw_rev_minor_h = reg_buf[3];
	idtp9221_dbg(chip, PR_MISC, "OTP FW Rev: %02x %02x %02x %02x\n",
				chip->fw_rev_major_l, chip->fw_rev_major_h,
				chip->fw_rev_minor_l, chip->fw_rev_minor_h);

	start_addr = IDTP9221_REG_SRAM_FW_MAJOR_REV_L;
	length = 4;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->fw_rev_sram_major_l = reg_buf[0];
	chip->fw_rev_sram_major_h = reg_buf[1];
	chip->fw_rev_sram_minor_l = reg_buf[2];
	chip->fw_rev_sram_minor_h = reg_buf[3];
	idtp9221_dbg(chip, PR_MISC, "EEPROM FW Rev: %02x %02x %02x %02x\n",
			chip->fw_rev_sram_major_l, chip->fw_rev_sram_major_h,
			chip->fw_rev_sram_minor_l, chip->fw_rev_sram_minor_h);

	idtp9221_dbg(chip, PR_MISC, "Wireless charger is activated\n");

	return 0;
}

static int idtp9221_read_regs_oneshot_in_pwr_ok(struct idtp9221 *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[4];

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_STATUS_H, &reg_buf[0]);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}
	if (reg_buf[0] & IDTP9221_REG_AUTH_BIT)
		chip->wireless_auth = 1;
	else
		chip->wireless_auth = 0;

	rc = idtp9221_i2c_read_byte(chip, IDTP9221_REG_SIGSTR, &reg_buf[0]);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}
	chip->sigstr_pc = IDTP9221_ADC_TO_SIGSTR_PC(reg_buf[0]);

	start_addr = IDTP9221_REG_TXID_L;
	length = 2;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf,
								length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
			start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->txid = reg_buf[0] | (reg_buf[1] << 8);

	start_addr = IDTP9221_REG_PING_FREQ_L;
	length = 2;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->ping_freq_khz = reg_buf[0] | (reg_buf[1] << 8);

	idtp9221_dbg(chip, PR_MONITOR,
		     "[%s] TxID:0x%04X(%d), Ping:%dkHz, Auth:%d, SigStr:%d%%\n",
		     str_status[chip->status], chip->txid, chip->txid,
		     chip->ping_freq_khz, chip->wireless_auth, chip->sigstr_pc);

	return 0;
}

static int idtp9221_read_regs_each_cycle(struct idtp9221 *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[2];
	int vrect_adc, vtherm_adc;

	start_addr = IDTP9221_REG_ADC_VRECT_L;
	length = 2;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	vrect_adc = reg_buf[0] | ((reg_buf[1] & 0x0f) << 8);
	chip->vrect_mv = IDTP9221_ADC_TO_VRECT_MV(vrect_adc);

	start_addr = IDTP9221_REG_ADC_VTHERM_L;
	length = 2;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	vtherm_adc = reg_buf[0] | ((reg_buf[1] & 0x0f) << 8);
	chip->vtherm_uv = IDTP9221_ADC_TO_VTHERM_UV(vtherm_adc);
	chip->vtherm_temp_dc = idtp9221_conv_vtherm_to_celsius(chip->vtherm_uv);

	idtp9221_dbg(chip, PR_MONITOR, "[%s] Vrect=%dmV VTherm=%ddC\n",
			str_status[chip->status],
			chip->vrect_mv, chip->vtherm_temp_dc);
	return 0;
}

static int idtp9221_read_regs_debug(struct idtp9221 *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[6];
	int vout_adc, iout_adc, die_temp_adc, op_freq, rpp_adc;

	if (!chip->monitor_for_debug)
		return 0;

	start_addr = IDTP9221_REG_E_REASON;
	length = 3;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->reg_ept = reg_buf[0];
	vout_adc = reg_buf[1] | ((reg_buf[2] & 0x0f) << 8);
	chip->vout_mv = IDTP9221_ADC_TO_VOUT_MV(vout_adc);

	start_addr = IDTP9221_REG_RX_IOUT_L;
	length = 6;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	iout_adc = reg_buf[0] | (reg_buf[1] << 8);
	chip->iout_ma = IDTP9221_ADC_TO_MA(iout_adc);

	die_temp_adc = reg_buf[2] | ((reg_buf[3] & 0x0f) << 8);
	chip->die_temp_c = IDTP9221_ADC_TO_DIE_TEMP_C(die_temp_adc);

	op_freq = reg_buf[4] | (reg_buf[5] << 8);
	chip->op_freq_khz = IDTP9221_ADC_TO_FREQ_KZ(op_freq);

	start_addr = IDTP9221_REG_RPP_L;
	length = 2;
	rc = idtp9221_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	rpp_adc = reg_buf[0] | (reg_buf[1] << 8);
	chip->rpp_raw = rpp_adc;
	if (chip->op_mode == IDTP9221_OP_MODE_QI_EPP)
		chip->rpp_mw = IDTP9221_ADC_TO_RPP_EPP_MW(rpp_adc);
	else
		chip->rpp_mw = IDTP9221_ADC_TO_RPP_BPP_MW(rpp_adc);

	idtp9221_dbg(chip, PR_MONITOR,
			"[%s] Vout=%dmv Iout=%dmA DieTemp=%dC op_freq=%dkHz RPP=%dmW (EPT/EOC/EOP Reason=0x%02X)\n",
			str_status[chip->status], chip->vout_mv,
			chip->iout_ma, chip->die_temp_c,
			chip->op_freq_khz, chip->rpp_mw, chip->reg_ept);
	return 0;
}

static void idtp9221_control_enable(struct idtp9221 *chip, bool enable)
{
	if (!gpio_is_valid(chip->wlc_en))
		return;

	if (enable) {
		gpio_direction_output(chip->wlc_en, 0);
		if (gpio_is_valid(chip->wlc_en_rsvd))
			gpio_direction_output(chip->wlc_en_rsvd, 0);
	} else {
		gpio_direction_output(chip->wlc_en, 1);
		if (gpio_is_valid(chip->wlc_en_rsvd))
			gpio_direction_output(chip->wlc_en_rsvd, 1);
	}

	chip->enabled = enable;
	idtp9221_dbg(chip, PR_MISC, "%d\n", enable);
}

static void idtp9221_handle_halt_proc(struct idtp9221 *chip)
{
	int i = 0;
	bool halt = false;

	chip->halt_retry = false;

	while (i < IDTP9221_ENABLE_CLIENT_MAX) {
		if (chip->halt_client[i]) {
			halt = true;
			break;
		}
		i++;
	}

	if (halt) {
		if (chip->is_worker_active) {
			/* wait for the end of worker */
			idtp9221_dbg(chip, PR_MISC,
					"wait for worker's end point\n");
			chip->halt_retry = true;
			return;
		}
		idtp9221_dbg(chip, PR_MISC, "Disable WLC_EN\n");
		idtp9221_control_enable(chip, false);
	} else {
		idtp9221_dbg(chip, PR_MISC,
					"Enable WLC_EN due to client:%d\n", i);
		idtp9221_control_enable(chip, true);
	}
}

static int idtp9221_handle_halt(struct idtp9221 *chip, int client, bool halt)
{
	if (client >= IDTP9221_ENABLE_CLIENT_MAX)
		return -EINVAL;

	if (chip->disable_halt)
		return 0;

	idtp9221_dbg(chip, PR_MISC, "client:%d halt:%d\n", client, (int)halt);
	chip->halt_client[client] = halt;

	idtp9221_handle_halt_proc(chip);

	return 0;
}

enum {
	ATTR_INTERVAL_POWER_OK_MS = 0,
	ATTR_INTERVAL_NEGOTIATION_MS,
	ATTR_MONITOR_FOR_DEBUG,
	ATTR_WLC_EN,
	ATTR_HALT_FOR_DEBUG,
	ATTR_DISABLE_HALT,
	ATTR_VOUT_SET_MV,
	ATTR_ILIM_SET_MA,
	ATTR_FOD_0_A,
	ATTR_FOD_0_B,
	ATTR_FOD_1_A,
	ATTR_FOD_1_B,
	ATTR_FOD_2_A,
	ATTR_FOD_2_B,
	ATTR_FOD_3_A,
	ATTR_FOD_3_B,
	ATTR_FOD_4_A,
	ATTR_FOD_4_B,
	ATTR_FOD_5_A,
	ATTR_FOD_5_B,
	ATTR_FOD_EXPANSION_A,
	ATTR_FOD_EXPANSION_B,
	ATTR_FOD_EPP_MODE_1_A,
	ATTR_FOD_EPP_MODE_1_B,
	ATTR_FOD_ADJUST_EN,
	ATTR_SEND_EPT,
	ATTR_CMD_REG_ADDR,
	ATTR_CMD_REG_DATA,
	ATTR_FW_REV,
	ATTR_VRECT_MV,
	ATTR_VOUT_MV,
	ATTR_IOUT_MA,
	ATTR_DIE_TEMP_C,
	ATTR_VTHERM_UV,
	ATTR_OP_FREQ_KHZ,
	ATTR_REG_EPT,
	ATTR_VTHERM_TEMP_DC,
	ATTR_SIGSTR_PC,
	ATTR_RPP_MW,
	ATTR_TXID,
	ATTR_PING_FREQ_KHZ,
};

static ssize_t idt9221_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);

static ssize_t idt9221_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);

static struct device_attribute wireless_attrs[] = {
	__ATTR(interval_power_ok_ms,	0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(interval_negotiation_ms,	0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(monitor_for_debug,	0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(wlc_en,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(halt_for_debug,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(disable_halt,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(vout_set_mv,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(ilim_set_ma,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_0_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_0_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_1_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_1_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_2_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_2_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_3_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_3_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_4_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_4_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_5_a,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_5_b,			0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_expansion_a,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_expansion_b,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_epp_mode_1_a,	0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_epp_mode_1_b,	0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fod_adjust_en,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(send_ept,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(cmd_reg_addr,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(cmd_reg_data,		0644, idt9221_param_show,
						idt9221_param_store),
	__ATTR(fw_rev,			0444, idt9221_param_show, NULL),
	__ATTR(vrect_mv,		0444, idt9221_param_show, NULL),
	__ATTR(vout_mv,			0444, idt9221_param_show, NULL),
	__ATTR(iout_ma,			0444, idt9221_param_show, NULL),
	__ATTR(die_temp_c,		0444, idt9221_param_show, NULL),
	__ATTR(vtherm_uv,		0444, idt9221_param_show, NULL),
	__ATTR(op_freq_khz,		0444, idt9221_param_show, NULL),
	__ATTR(reg_ept,			0444, idt9221_param_show, NULL),
	__ATTR(vtherm_temp_dc,		0444, idt9221_param_show, NULL),
	__ATTR(sigstr_pc,		0444, idt9221_param_show, NULL),
	__ATTR(rpp_mw,			0444, idt9221_param_show, NULL),
	__ATTR(txid,			0444, idt9221_param_show, NULL),
	__ATTR(ping_freq_khz,		0444, idt9221_param_show, NULL),
};

static ssize_t idt9221_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	const ptrdiff_t off = attr - wireless_attrs;
	struct idtp9221 *chip = dev_get_drvdata(dev);
	u8 reg_offset;
	int ret;

	switch (off) {
	case ATTR_INTERVAL_POWER_OK_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->interval_power_ok_ms);
		break;

	case ATTR_INTERVAL_NEGOTIATION_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->interval_negotiation_ms);
		break;

	case ATTR_MONITOR_FOR_DEBUG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->monitor_for_debug);
		break;

	case ATTR_WLC_EN:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->enabled);
		break;

	case ATTR_HALT_FOR_DEBUG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->halt_for_debug ? 1 : 0);
		break;

	case ATTR_DISABLE_HALT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->disable_halt ? 1 : 0);
		break;

	case ATTR_VOUT_SET_MV:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->vout_set_mv);
		break;

	case ATTR_ILIM_SET_MA:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->ilim_set_ma);
		break;

	case ATTR_FOD_0_A:
	case ATTR_FOD_0_B:
	case ATTR_FOD_1_A:
	case ATTR_FOD_1_B:
	case ATTR_FOD_2_A:
	case ATTR_FOD_2_B:
	case ATTR_FOD_3_A:
	case ATTR_FOD_3_B:
	case ATTR_FOD_4_A:
	case ATTR_FOD_4_B:
	case ATTR_FOD_5_A:
	case ATTR_FOD_5_B:
	case ATTR_FOD_EXPANSION_A:
	case ATTR_FOD_EXPANSION_B:
	case ATTR_FOD_EPP_MODE_1_A:
	case ATTR_FOD_EPP_MODE_1_B:
		reg_offset = off - ATTR_FOD_0_A;
		if (reg_offset < FOD_REG_NUM)
			size = scnprintf(buf, PAGE_SIZE, "0x%02x\n",
						chip->fod_adj[reg_offset]);
		break;

	case ATTR_FOD_ADJUST_EN:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->fod_adj_en);
		break;

	case ATTR_SEND_EPT:
		size = 0;
		break;

	case ATTR_CMD_REG_ADDR:
		size = scnprintf(buf, PAGE_SIZE, "0x%02X\n",
						chip->cmd_reg_addr);
		break;

	case ATTR_CMD_REG_DATA:
		ret = idtp9221_read_cmd_reg_byte(chip, chip->cmd_reg_addr,
						&chip->cmd_reg_data);
		if (ret < 0)
			break;

		size = scnprintf(buf, PAGE_SIZE,
				"reg_addr:0x%02X\nreg_data:0x%02X\n",
				chip->cmd_reg_addr, chip->cmd_reg_data);
		break;

	case ATTR_FW_REV:
		if (chip->fw_src == IDTP9221_FW_SRC_OTP)
			size = scnprintf(buf, PAGE_SIZE, "OTP:%02x%02x%02x\n",
							chip->fw_rev_major_l,
							chip->fw_rev_minor_h,
							chip->fw_rev_minor_l);
		else if (chip->fw_src == IDTP9221_FW_SRC_EEPROM ||
			 chip->fw_src == IDTP9221_FW_SRC_RAM)
			size = scnprintf(buf, PAGE_SIZE, "SRAM:%02x%02x%02x\n",
						chip->fw_rev_major_l,
						chip->fw_rev_sram_minor_h,
						chip->fw_rev_sram_minor_l);
		else
			size = scnprintf(buf, PAGE_SIZE, "Unknown\n");
		break;

	case ATTR_VRECT_MV:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->vrect_mv);
		break;

	case ATTR_VOUT_MV:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->vout_mv);
		break;

	case ATTR_IOUT_MA:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->iout_ma);
		break;

	case ATTR_DIE_TEMP_C:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->die_temp_c);
		break;

	case ATTR_VTHERM_UV:
		size = scnprintf(buf, PAGE_SIZE, "%ld\n", chip->vtherm_uv);
		break;

	case ATTR_OP_FREQ_KHZ:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->op_freq_khz);
		break;

	case ATTR_REG_EPT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->reg_ept);
		break;

	case ATTR_VTHERM_TEMP_DC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->vtherm_temp_dc);
		break;

	case ATTR_SIGSTR_PC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->sigstr_pc);
		break;

	case ATTR_RPP_MW:
		size = scnprintf(buf, PAGE_SIZE, "%d(raw:%d)\n",
					chip->rpp_mw, chip->rpp_raw);
		break;

	case ATTR_TXID:
		size = scnprintf(buf, PAGE_SIZE, "0x%04X(%d)\n",
							chip->txid, chip->txid);
		break;

	case ATTR_PING_FREQ_KHZ:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->ping_freq_khz);
		break;

	default:
		size = 0;
		break;
	}
	return size;
}

static ssize_t idt9221_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	const ptrdiff_t off = attr - wireless_attrs;
	struct idtp9221 *chip = dev_get_drvdata(dev);
	int ret;
	int val;
	u8 reg_offset;

	switch (off) {
	case ATTR_INTERVAL_POWER_OK_MS:
		ret = kstrtoint(buf, 10, &chip->interval_power_ok_ms);
		if (ret < 0)
			size = 0;
		break;

	case ATTR_INTERVAL_NEGOTIATION_MS:
		ret = kstrtoint(buf, 10, &chip->interval_negotiation_ms);
		if (ret < 0)
			size = 0;
		break;

	case ATTR_MONITOR_FOR_DEBUG:
		ret = kstrtoint(buf, 10, &chip->monitor_for_debug);
		if (ret < 0) {
			size = 0;
			break;
		}
		break;

	case ATTR_WLC_EN:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		idtp9221_dbg(chip, PR_MISC, "directry ctrl WLC_EN pin %d\n",
									val);
		idtp9221_control_enable(chip, val);
		break;

	case ATTR_HALT_FOR_DEBUG:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_DEBUG, val);
		chip->halt_for_debug = val;
		break;

	case ATTR_DISABLE_HALT:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		chip->disable_halt = val == 1 ? true : false;
		break;

	case ATTR_VOUT_SET_MV:
		ret = kstrtoint(buf, 10, &chip->debug_vout_set_mv);
		if (ret < 0) {
			size = 0;
			break;
		}
		ret = idtp9221_set_vout_set(chip, chip->debug_vout_set_mv);
		if (ret < 0)
			size = 0;
		break;

	case ATTR_ILIM_SET_MA:
		ret = kstrtoint(buf, 10, &chip->debug_ilim_set_ma);
		if (ret < 0) {
			size = 0;
			break;
		}
		ret = idtp9221_set_ilim_set(chip, chip->debug_ilim_set_ma);
		if (ret < 0)
			size = 0;
		break;

	case ATTR_FOD_0_A:
	case ATTR_FOD_0_B:
	case ATTR_FOD_1_A:
	case ATTR_FOD_1_B:
	case ATTR_FOD_2_A:
	case ATTR_FOD_2_B:
	case ATTR_FOD_3_A:
	case ATTR_FOD_3_B:
	case ATTR_FOD_4_A:
	case ATTR_FOD_4_B:
	case ATTR_FOD_5_A:
	case ATTR_FOD_5_B:
	case ATTR_FOD_EXPANSION_A:
	case ATTR_FOD_EXPANSION_B:
	case ATTR_FOD_EPP_MODE_1_A:
	case ATTR_FOD_EPP_MODE_1_B:
		ret = kstrtoint(buf, 0, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		reg_offset = off - ATTR_FOD_0_A;

		if (reg_offset < 0)
			break;

		chip->fod_adj[reg_offset] = (u8)val;
		if (chip->fod_adj_en) {
			ret = idtp9221_set_fod(chip, reg_offset,
				chip->fod_adj[reg_offset]);
			if (ret < 0) {
				size = 0;
				break;
			}
		}

		idtp9221_dbg(chip, PR_MISC, "fod_adj[%d] -> 0x%02x\n",
					(int)reg_offset,
					(int)chip->fod_adj[reg_offset]);
		break;

	case ATTR_FOD_ADJUST_EN:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		if (val == 1) {
			chip->fod_adj_en = true;
			if (chip->status == IDTP9221_STATUS_PWR_POWER_OK) {
				ret = idtp9221_set_fod_all(chip);
				if (ret < 0) {
					size = 0;
					break;
				}
			}

			idtp9221_dbg(chip, PR_MISC, "Enabled FOD Adjustment\n");
		} else {
			chip->fod_adj_en = false;
			idtp9221_dbg(chip, PR_MISC,
						"Disabled FOD Adjustment\n");
		}
		break;

	case ATTR_SEND_EPT:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		idtp9221_dbg(chip, PR_MISC, "Send EPT Packet. mode:%d\n", val);
		ret = idtp9221_set_ept(chip, val);
		if (ret < 0)
			size = 0;
		break;

	case ATTR_CMD_REG_ADDR:
		ret = kstrtoint(buf, 0, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		chip->cmd_reg_addr = (u8)val;
		break;

	case ATTR_CMD_REG_DATA:
		ret = kstrtoint(buf, 0, &val);
		if (ret < 0) {
			size = 0;
			break;
		}
		chip->cmd_reg_data = (u8)val;
		ret = idtp9221_write_cmd_reg_byte(chip, chip->cmd_reg_addr,
							chip->cmd_reg_data);
		break;

	default:
		break;
	}
	return size;
}


static int idtp9221_parse_dt(struct idtp9221 *chip)
{
	struct device_node *node = chip->dev->of_node;
	int rc;

	if (!node) {
		dev_err(chip->dev, "no dts data for idtp9221\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(node, "somc,ilim-bpp", &chip->bpp_ilim_ma);
	if (rc < 0)
		chip->bpp_ilim_ma = 500;

	rc = of_property_read_u32(node, "somc,ilim-epp-boost",
						&chip->epp_boost_ilim_ma);
	if (rc < 0)
		chip->epp_boost_ilim_ma = 1300;

	rc = of_property_read_u32(node, "somc,ilim-epp", &chip->epp_ilim_ma);
	if (rc < 0)
		chip->epp_ilim_ma = 700;

	rc = of_property_read_u32(node, "somc,vout-bpp", &chip->bpp_vout_mv);
	if (rc < 0)
		chip->bpp_vout_mv = 5000;

	rc = of_property_read_u32(node, "somc,vout-epp", &chip->epp_vout_mv);
	if (rc < 0)
		chip->epp_vout_mv = 9000;

	chip->wlc_en = of_get_named_gpio(node, "somc,wlc-en-gpio", 0);
	if (!gpio_is_valid(chip->wlc_en))
		dev_err(chip->dev, "Can't get wlc-en-gpio\n");

	chip->wlc_en_rsvd = of_get_named_gpio(node, "somc,wlc-en-rsvd-gpio", 0);

	chip->wlc_gpio_irq = of_get_named_gpio(node, "somc,wlc-irq-gpio", 0);
	if (!gpio_is_valid(chip->wlc_gpio_irq))
		dev_err(chip->dev, "Can't get wlc-irq-gpio\n");

	chip->disable_halt = of_property_read_bool(node,
						"somc,enable-wlc-during-usbin");

	chip->monitor_for_debug = of_property_read_bool(node,
					"somc,enable-monitor-for-debug");

	return 0;
}

static int idtp9221_get_status(struct idtp9221 *chip)
{
	int rc = 0;
	int wlc_irq_status;
	u8 val;

	wlc_irq_status = gpio_get_value(chip->wlc_gpio_irq);
	if (wlc_irq_status == 0) {
		rc = idtp9221_i2c_read_byte(chip,
						IDTP9221_REG_STATUS_L,
						&val);
		if (rc < 0) {
			rc = IDTP9221_STATUS_PWR_OFF;
			idtp9221_dbg(chip, PR_STATUS,
					"status:%d(%s) instead of I2C ERROR\n",
					rc, str_status[rc]);
			return rc;
		}

		if (val & IDTP9221_REG_STAT_VOUT_BIT)
			rc = IDTP9221_STATUS_PWR_POWER_OK;
		else
			rc = IDTP9221_STATUS_PWR_ON_NEGOTIATION;

	} else {
		rc = IDTP9221_STATUS_PWR_OFF;
	}

	idtp9221_dbg(chip, PR_STATUS, "status:%d(%s) wlc_irq_status:%d\n",
					rc, str_status[rc], wlc_irq_status);
	return rc;
}

static void idtp9221_wake_lock(struct idtp9221 *chip, bool en)
{
	if (en && !chip->wake_lock_en) {
		pm_stay_awake(chip->dev);
		idtp9221_dbg(chip, PR_MISC, "pm_stay_awake\n");
	} else if (!en && chip->wake_lock_en) {
		idtp9221_dbg(chip, PR_MISC, "pm_relax\n");
		pm_relax(chip->dev);
	}
	chip->wake_lock_en = en;
}

static void idtp9221_detect_work(struct work_struct *work)
{
	struct idtp9221 *chip = container_of(work,
				struct idtp9221,
				detect_work.work);
	int rc;
	int interval;
	ktime_t dbg_ktime_start, dbg_ktime_end;
	union power_supply_propval pval = {0, };

	idtp9221_dbg(chip, PR_WORKER, "worker is started\n");

	mutex_lock(&chip->reg_ctrl_lock);

	dbg_ktime_start = ktime_get_boottime();
	chip->is_worker_active = true;

	/* get status at first */
	chip->status = idtp9221_get_status(chip);
	if (chip->status == IDTP9221_STATUS_PWR_OFF) {
		chip->op_mode = 0;
		chip->one_time_read_in_pwr_ok_done = false;
		idtp9221_dbg(chip, PR_MISC, "skip monitoring due to POFF\n");
		goto reg_done;
	}

	/* read OP mode */
	if (!chip->op_mode) {
		rc = idtp9221_get_op_mode(chip);
		if (rc) {
			pr_err("Error in get_op_mode rc=%d\n", rc);
			goto reg_done;
		}
	}

	/* one time read parameter */
	if (!chip->one_time_read_in_driver_done) {
		rc = idtp9221_read_regs_oneshot_in_driver(chip);
		if (rc) {
			pr_err("Error in read_regs_oneshot rc=%d\n", rc);
			goto reg_done;
		}
		chip->one_time_read_in_driver_done = true;
	}

	/* one time read parameter after power on */
	if (chip->status == IDTP9221_STATUS_PWR_POWER_OK &&
		!chip->one_time_read_in_pwr_ok_done) {
		rc = idtp9221_read_regs_oneshot_in_pwr_ok(chip);
		if (rc) {
			pr_err("Error in read_regs one power_ok rc=%d\n", rc);
			goto reg_done;
		}
		chip->one_time_read_in_pwr_ok_done = true;
	}

	/* always read parameter */
	rc = idtp9221_read_regs_each_cycle(chip);
	if (rc) {
		pr_err("Error in read_regs_polling rc=%d\n", rc);
		goto reg_done;
	}

	/* always read parameter during debugging */
	rc = idtp9221_read_regs_debug(chip);
	if (rc) {
		pr_err("Error in read_regs_debug rc=%d\n", rc);
		goto reg_done;
	}

	dbg_ktime_end = ktime_get_boottime();
	idtp9221_dbg(chip, PR_MEASUREMENT, "It took %dms to execute i2c work\n",
			(int)ktime_to_ms(ktime_sub(dbg_ktime_end,
			dbg_ktime_start)));

	if (idtp9221_debug_dump)
		idtp9221_dump_reg(chip);

	rc = idtp9221_handle_vout_set(chip);
	if (chip->status == IDTP9221_STATUS_PWR_POWER_OK &&
			chip->old_status != IDTP9221_STATUS_PWR_POWER_OK) {
		rc = idtp9221_set_fod_all(chip);
		if (rc)
			pr_err("Error in set_fod_all rc=%d\n", rc);

		if (chip->op_mode == IDTP9221_OP_MODE_QI_EPP) {
			if (chip->wireless_auth)
				rc = idtp9221_set_ilim_set(chip,
						chip->epp_boost_ilim_ma);
			else
				rc = idtp9221_set_ilim_set(chip,
						chip->epp_ilim_ma);
			if (rc)
				pr_err("Error in set_ilim_set rc=%d\n", rc);
		}
		if (chip->op_mode == IDTP9221_OP_MODE_QI_BPP) {
			rc = idtp9221_set_ilim_set(chip, chip->bpp_ilim_ma);
			if (rc)
				pr_err("Error in set_ilim_set rc=%d\n", rc);
		}
	}

reg_done:
	if (chip->status == IDTP9221_STATUS_PWR_POWER_OK) {
		switch (chip->op_mode) {
		case IDTP9221_OP_MODE_QI_EPP:
			pval.intval = IDTP9221_PSY_WIRELESS_MODE_EPP;
			break;
		case IDTP9221_OP_MODE_QI_BPP:
			pval.intval = IDTP9221_PSY_WIRELESS_MODE_BPP;
			break;
		case IDTP9221_OP_MODE_PMA:
		case IDTP9221_OP_MODE_UNKNOWN:
		default:
			pval.intval = IDTP9221_PSY_WIRELESS_MODE_UNKNOWN;
			break;
		}
	} else {
		pval.intval = IDTP9221_PSY_WIRELESS_MODE_OFF;
	}

	if (pval.intval != chip->psy_wireless_mode) {
		if (!chip->dc_psy) {
			idtp9221_dbg(chip, PR_PSY,
					"re-call power_supply_get_by_name()\n");
			chip->dc_psy = power_supply_get_by_name("dc");
		}

		if (chip->dc_psy) {
			idtp9221_dbg(chip, PR_PSY,
					"call power_supply_set_property()\n");
			power_supply_set_property(chip->dc_psy,
				POWER_SUPPLY_PROP_WIRELESS_MODE, &pval);
		}
		chip->psy_wireless_mode = pval.intval;
	}

	chip->is_worker_active = false;
	if (chip->halt_retry)
		idtp9221_handle_halt_proc(chip);

	if (!chip->batt_psy) {
		idtp9221_dbg(chip, PR_PSY,
				"re-call power_supply_get_by_name()\n");
		chip->batt_psy = power_supply_get_by_name("battery");
	}

	if (chip->old_status != chip->status && chip->batt_psy)
		power_supply_changed(chip->batt_psy);

	chip->old_status = chip->status;
	switch (chip->status) {
	case IDTP9221_STATUS_PWR_ON_NEGOTIATION:
		if (idtp9221_get_status(chip) == IDTP9221_STATUS_PWR_OFF) {
			idtp9221_dbg(chip, PR_WORKER,
						"reschedule worker(OFF)\n");
			/*
			 * set interval to 0 to stop worker at next time
			 * if halt or disconnected during monitoring
			 */
			interval = 0;
		} else {
			idtp9221_dbg(chip, PR_WORKER,
						"reschedule worker(NEGO)\n");
			/* speed up monitoring during negotiation */
			interval = chip->interval_negotiation_ms;
		}
		schedule_delayed_work(&chip->detect_work,
						msecs_to_jiffies(interval));
		break;
	case IDTP9221_STATUS_PWR_POWER_OK:
		if (idtp9221_get_status(chip) == IDTP9221_STATUS_PWR_OFF) {
			idtp9221_dbg(chip, PR_WORKER,
						"reschedule worker(OFF)\n");
			/*
			 * set interval to 0 to stop worker at next time
			 * if halt or disconnected during monitoring
			 */
			interval = 0;
		} else {
			idtp9221_dbg(chip, PR_WORKER,
						"reschedule worker(PWON)\n");
			interval = chip->interval_power_ok_ms;
		}
		schedule_delayed_work(&chip->detect_work,
						msecs_to_jiffies(interval));
		break;
	case IDTP9221_STATUS_PWR_OFF:
	default:
		idtp9221_dbg(chip, PR_WORKER, "stop worker(POFF)\n");
		idtp9221_wake_lock(chip, false);
		break;
	}

	mutex_unlock(&chip->reg_ctrl_lock);
}

#define STATUS_PWR_OFF			"off"
#define STATUS_PWR_ON_NEGOTIATION	"negotiating"
#define STATUS_PWR_ON			"powered"
#define OP_MODE_QI_EPP_STR		"qi_epp"
#define OP_MODE_QI_BPP_STR		"qi_bpp"
#define OP_MODE_QI_PMA_STR		"pma"
#define OP_MODE_QI_UNKNOWN_STR		"unknown"
#define OP_MODE_QI_NONE_STR		"none"

static const char *idtp9221_get_wireless_charger_type(struct idtp9221 *chip)
{
	const char *charger_type = NULL;

	if (chip->status == IDTP9221_STATUS_PWR_POWER_OK) {
		switch (chip->op_mode) {
		case IDTP9221_OP_MODE_QI_EPP:
			charger_type = OP_MODE_QI_EPP_STR;
			break;
		case IDTP9221_OP_MODE_QI_BPP:
			charger_type = OP_MODE_QI_BPP_STR;
			break;
		case IDTP9221_OP_MODE_PMA:
			charger_type = OP_MODE_QI_PMA_STR;
			break;
		case IDTP9221_OP_MODE_UNKNOWN:
		default:
			charger_type = OP_MODE_QI_UNKNOWN_STR;
			break;
		}
	} else {
		charger_type = OP_MODE_QI_NONE_STR;
	}

	return charger_type;
}

static const char *idtp9221_get_wireless_status(int status)
{
	const char *wlc_status = NULL;

	switch (status) {
	case IDTP9221_STATUS_PWR_OFF:
		wlc_status = STATUS_PWR_OFF;
		break;
	case IDTP9221_STATUS_PWR_ON_NEGOTIATION:
		wlc_status = STATUS_PWR_ON_NEGOTIATION;
		break;
	case IDTP9221_STATUS_PWR_POWER_OK:
		wlc_status = STATUS_PWR_ON;
		break;
	default:
		break;
	}
	return wlc_status;
}

static enum power_supply_property wireless_props[] = {
	POWER_SUPPLY_PROP_DCIN_VALID,
	POWER_SUPPLY_PROP_USBIN_VALID,
	POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1,
	POWER_SUPPLY_PROP_WIRELESS_THERMAL_V_LIMIT,
	POWER_SUPPLY_PROP_AUTH,
	POWER_SUPPLY_PROP_CHARGER_TYPE,
	POWER_SUPPLY_PROP_WIRELESS_STATUS,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED,
};

static int idtp9221_psy_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	struct idtp9221 *chip = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
		break;

	case POWER_SUPPLY_PROP_USBIN_VALID:
		val->intval = chip->usbin_valid;
		break;

	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
		val->intval = chip->wireless_suspend;
		break;
	case POWER_SUPPLY_PROP_WIRELESS_THERMAL_V_LIMIT:
		val->intval = chip->thermal_max_voltage;
		break;
	case POWER_SUPPLY_PROP_AUTH:
		val->intval = chip->wireless_auth;
		break;
	case POWER_SUPPLY_PROP_CHARGER_TYPE:
		val->strval = idtp9221_get_wireless_charger_type(chip);
		break;
	case POWER_SUPPLY_PROP_WIRELESS_STATUS:
		val->strval = idtp9221_get_wireless_status(chip->status);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (chip->status == IDTP9221_STATUS_PWR_OFF) {
			return -ENODEV;
		} else if (chip->vtherm_temp_dc >= IDTP9221_VTHERM_TEMP_MAX) {
			pr_err("vtherm_temp_dc is abnomal value.\n");
			return -ENODEV;
		} else {
			val->intval = chip->vtherm_temp_dc;
		}
		break;
	case POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED:
		if (chip->status == IDTP9221_STATUS_PWR_POWER_OK)
			val->intval = true;
		else
			val->intval = false;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int idtp9221_psy_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	struct idtp9221 *chip = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
		break;

	case POWER_SUPPLY_PROP_USBIN_VALID:
		idtp9221_dbg(chip, PR_PSY,
				"POWER_SUPPLY_PROP_USBIN_VALID=%d\n",
				val->intval);
		if (val->intval)
			idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_USB,
									true);
		else
			idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_USB,
									false);

		chip->usbin_valid = val->intval;
		break;
	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
		idtp9221_dbg(chip, PR_PSY,
				"POWER_SUPPLY_PROP_WIRELESS_SUSPEND=%d\n",
				val->intval);
		if (val->intval)
			idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_DEV1,
									true);
		else
			idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_DEV1,
									false);

		chip->wireless_suspend = val->intval;
		break;
	case POWER_SUPPLY_PROP_WIRELESS_THERMAL_V_LIMIT:
		idtp9221_dbg(chip, PR_PSY, "WIRELESS_THERMAL_V_LIMIT=%d\n",
								val->intval);
		if (val->intval != chip->thermal_max_voltage) {
			chip->thermal_max_voltage = val->intval;
			if (val->intval == 0) {
				idtp9221_handle_halt(chip,
						IDTP9221_ENABLE_CLIENT_THERM_WA,
						true);
			} else {
				idtp9221_handle_halt(chip,
						IDTP9221_ENABLE_CLIENT_THERM_WA,
						false);
				idtp9221_handle_vout_set(chip);
			}
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int idtp9221_property_is_writeable(struct power_supply *psy,
					enum power_supply_property prop)
{
	int rc;

	switch (prop) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
	case POWER_SUPPLY_PROP_USBIN_VALID:
	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
	case POWER_SUPPLY_PROP_WIRELESS_THERMAL_V_LIMIT:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}

	return rc;
}

static const struct power_supply_desc wireless_psy_desc = {
	.name = "wireless",
	.type = POWER_SUPPLY_TYPE_WIRELESS,
	.properties = wireless_props,
	.num_properties = ARRAY_SIZE(wireless_props),
	.get_property = idtp9221_psy_get_property,
	.set_property = idtp9221_psy_set_property,
	.property_is_writeable = idtp9221_property_is_writeable,
};

static int idtp9221_init_wilreless_psy(struct idtp9221 *chip)
{
	struct power_supply_config wireless_cfg = {};
	int rc = 0;

	wireless_cfg.drv_data = chip;
	wireless_cfg.of_node = chip->dev->of_node;
	chip->wireless_psy = power_supply_register(chip->dev,
						   &wireless_psy_desc,
						   &wireless_cfg);
	if (IS_ERR(chip->wireless_psy)) {
		pr_err("Couldn't register wireless power supply\n");
		return PTR_ERR(chip->wireless_psy);
	}

	return rc;
}

static irqreturn_t idtp9221_irq_handler(int irq, void *data)
{
	struct idtp9221 *chip = data;
	int stat;

	idtp9221_wake_lock(chip, true);

	stat = gpio_get_value(chip->wlc_gpio_irq);
	idtp9221_dbg(chip, PR_IRQ, "irq_status=%d\n", stat);

	cancel_delayed_work_sync(&chip->detect_work);
	schedule_delayed_work(&chip->detect_work, msecs_to_jiffies(0));

	return IRQ_HANDLED;
}

static int idtp9221_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct idtp9221 *chip;
	int rc;
	int i;
	union power_supply_propval prop = {0, };
	bool wlc_en;

	pr_err("idtp9221_probe(start)\n");

	chip = devm_kzalloc(&client->dev, sizeof(struct idtp9221), GFP_KERNEL);
	if (!chip) {
		rc = -ENOMEM;
		goto err_kzalloc;
	}

	/* device name resistration */
	dev_set_name(&client->dev, "wireless-chg");

	/* i2c resistration */
	chip->i2c_handle = client;
	chip->dev = &client->dev;
	i2c_set_clientdata(client, chip);

	/* worker resistration */
	INIT_DELAYED_WORK(&chip->detect_work, idtp9221_detect_work);
	INIT_DELAYED_WORK(&chip->vout_change_work, idtp9221_vout_change_work);

	mutex_init(&chip->reg_ctrl_lock);
	mutex_init(&chip->i2c_lock);

	/* get device tree */
	rc = idtp9221_parse_dt(chip);
	if (rc < 0)
		goto err_parse_dt;

	chip->debug_mask = &idtp9221_debug_mask;
	chip->interval_power_ok_ms = IDTP9221_INTERVAL_POWER_OK_DEFAULT_MS;
	chip->interval_negotiation_ms =
				IDTP9221_INTERVAL_NEGOTIATION_DEFAULT_MS;
	chip->status = IDTP9221_STATUS_PWR_OFF;
	chip->old_status = IDTP9221_STATUS_PWR_OFF;
	chip->thermal_max_voltage = -1;
	chip->psy_wireless_mode = IDTP9221_PSY_WIRELESS_MODE_UNSETTLED;
	chip->dc_psy = power_supply_get_by_name("dc");
	chip->batt_psy = power_supply_get_by_name("battery");

	/* sysfs resistration */
	for (i = 0; i < ARRAY_SIZE(wireless_attrs); i++) {
		rc = device_create_file(&client->dev, &wireless_attrs[i]);
		if (rc < 0) {
			dev_err(chip->dev, "can't create sysfs (%d)\n", rc);
			goto err_sysfs;
		}
	}

	/* power supply resistration */
	rc = idtp9221_init_wilreless_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize wireless psy rc=%d\n", rc);
		goto err_psy;
	}

	/* GPIO initialize */
	if (gpio_is_valid(chip->wlc_en))
		gpio_request_one(chip->wlc_en, GPIOF_OUT_INIT_LOW, "wlc_en");

	if (gpio_is_valid(chip->wlc_en_rsvd))
		gpio_request_one(chip->wlc_en_rsvd, GPIOF_OUT_INIT_LOW,
								"wlc_en_rsvd");

	if (gpio_is_valid(chip->wlc_gpio_irq)) {
		rc = gpio_request(chip->wlc_gpio_irq, "idtp9221_irq");
		if (rc < 0) {
			dev_err(chip->dev, "can't request irq (%d)\n", rc);
			goto err_irq;
		}

		rc = gpio_direction_input(chip->wlc_gpio_irq);
		if (rc < 0) {
			dev_err(chip->dev, "can't set input gpio (%d)\n", rc);
			goto err_irq;
		}
	} else {
		pr_err("wlc_gpio_irq is invalid\n");
	}

	dev_set_drvdata(chip->dev, chip);
	device_init_wakeup(chip->dev, 1);

	/* determine initial wlc_en value depend on usbin */
	chip->usb_psy = power_supply_get_by_name("usb");
	if (chip->usb_psy) {
		rc = power_supply_get_property(chip->usb_psy,
					POWER_SUPPLY_PROP_PRESENT, &prop);
		wlc_en = !prop.intval;
	} else {
		wlc_en = true;
		pr_err("can't get usb_psy\n");
	}
	if (chip->disable_halt) {
		idtp9221_dbg(chip, PR_MISC, "Keep enabling WLC_EN for debug\n");
		idtp9221_control_enable(chip, true);
	} else {
		idtp9221_handle_halt(chip, IDTP9221_ENABLE_CLIENT_USB, !wlc_en);
	}

	idtp9221_irq_handler(0, chip);

	/* irq */
	if (gpio_is_valid(chip->wlc_gpio_irq)) {
		chip->wlc_irq = gpio_to_irq(chip->wlc_gpio_irq);
		if (chip->wlc_irq < 0) {
			rc = chip->wlc_irq;
			dev_err(chip->dev, "can't gpio_to_irq (%d)\n", rc);
			goto err_irq;
		}
		rc = devm_request_threaded_irq(chip->dev, chip->wlc_irq,
				NULL, idtp9221_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				IRQF_ONESHOT, "idtp9221", chip);
		if (rc) {
			dev_err(chip->dev,
				"can't devm_request_threaded_irq (%d)\n", rc);
			goto err_irq;
		}

		enable_irq_wake(chip->wlc_irq);
	}

	pr_err("idtp9221_probe(end)\n");
	return 0;

err_psy:
	for (i = 0; i < ARRAY_SIZE(wireless_attrs); i++)
		device_remove_file(&client->dev, &wireless_attrs[i]);
err_sysfs:
err_irq:
err_parse_dt:
	i2c_set_clientdata(client, NULL);
	kzfree(chip);
err_kzalloc:
	return rc;
}

static int idtp9221_remove(struct i2c_client *client)
{

	struct idtp9221 *chip = i2c_get_clientdata(client);
	int i;

	cancel_delayed_work_sync(&chip->detect_work);
	power_supply_unregister(chip->wireless_psy);
	idtp9221_wake_lock(chip, false);
	device_init_wakeup(chip->dev, 0);

	for (i = 0; i < ARRAY_SIZE(wireless_attrs); i++)
		device_remove_file(&client->dev, &wireless_attrs[i]);

	i2c_set_clientdata(client, NULL);
	kzfree(chip);
	return 0;
}

static const struct of_device_id idtp9221_dt_ids[] = {
	{ .compatible = "idtp,idtp9221", },
	{ }
};
MODULE_DEVICE_TABLE(of, idtp9221_dt_ids);

static const struct i2c_device_id idtp9221_ids[] = {
	{"idtp9221", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, idtp9221_ids);

static struct i2c_driver idtp9221_driver = {
	.driver = {
		   .name = "idtp9221",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(idtp9221_dt_ids),
		   },
	.probe = idtp9221_probe,
	.remove = idtp9221_remove,
	.id_table = idtp9221_ids,
};

module_i2c_driver(idtp9221_driver);

MODULE_LICENSE("GPL v2");
