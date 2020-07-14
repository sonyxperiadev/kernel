/*
 * Copyright (C) 2019 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
// SPDX-License-Identifier: GPL-2.0-only

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

/* IDT P9382A Registers */

#define IDTP9382A_REG_CHIP_ID_L			0x00
#define IDTP9382A_REG_CHIP_ID_H			0x01
#define IDTP9382A_REG_CHIP_REV			0x02
#define IDTP9382A_REG_CHIP_REV_MASK		GENMASK(4, 7)
#define IDTP9382A_REG_CHIP_MINOR_REV_MASK	GENMASK(0, 3)
#define IDTP9382A_REG_CUSTOMER_ID		0x03
#define IDTP9382A_REG_OTP_FW_MAJOR_REV_L	0x04
#define IDTP9382A_REG_OTP_FW_MAJOR_REV_H	0x05
#define IDTP9382A_REG_OTP_FW_MINOR_REV_L	0x06
#define IDTP9382A_REG_OTP_FW_MINOR_REV_H	0x07
#define IDTP9382A_REG_OTP_FW_DATE_CODE		0x08
#define IDTP9382A_REG_OTP_FW_TIME_CODE		0x14
#define IDTP9382A_REG_SRAM_FW_MAJOR_REV_L	0x1C
#define IDTP9382A_REG_SRAM_FW_MAJOR_REV_H	0x1D
#define IDTP9382A_REG_SRAM_FW_MINOR_REV_L	0x1E
#define IDTP9382A_REG_SRAM_FW_MINOR_REV_H	0x1F
#define IDTP9382A_REG_SRAM_FW_DATE_CODE		0x20
#define IDTP9382A_REG_SRAM_FW_TIME_CODE		0x2C
#define IDTP9382A_REG_STATUS_L			0x34
#define IDTP9382A_REG_STAT_VOUT_BIT		BIT(7)
#define IDTP9382A_REG_STAT_VRECT_BIT		BIT(6)
#define IDTP9382A_REG_STAT_ACMISSING_BIT	BIT(5)
#define IDTP9382A_REG_TX_DATA_RECEIVED_BIT	BIT(4)
#define IDTP9382A_REG_STAT_THERM_BIT		BIT(3)
#define IDTP9382A_REG_OVER_TEMPERATURE_BIT	BIT(2)
#define IDTP9382A_REG_OVER_VOLTAGE_BIT		BIT(1)
#define IDTP9382A_REG_OVER_CURRENT_BIT		BIT(0)
#define IDTP9382A_REG_STATUS_H			0x35
#define IDTP9382A_REG_INT_L			0x36
#define IDTP9382A_REG_INT_H			0x37
#define IDTP9382A_REG_INT_ENABLE_L		0x38
#define IDTP9382A_REG_INT_ENABLE_H		0x39
#define IDTP9382A_REG_INT_CLEAR			0x3A
#define IDTP9382A_REG_VOUT_SET			0x3C
#define IDTP9382A_REG_ILIM_SET			0x3D
#define IDTP9382A_REG_CHARGE_STATUS		0x3E
#define IDTP9382A_REG_EPT_CODE			0x3F
#define IDTP9382A_REG_VRECT			0x40
#define IDTP9382A_REG_VOUT			0x42
#define IDTP9382A_REG_RX_IOUT			0x44
#define IDTP9382A_REG_SIGSTR			0x46
#define IDTP9382A_REG_OP_FREQ_L			0x48
#define IDTP9382A_REG_OP_FREQ_H			0x49

#define IDTP9382A_REG_SYS_OP_MODE		0x4A
#define IDTP9382A_REG_SYS_OP_MODE_MASK		GENMASK(0, 3)
#define IDTP9382A_REG_SYS_OP_MODE_WPC_BIT	BIT(0)
#define IDTP9382A_REG_SYS_OP_MODE_PMA_BIT	BIT(1)
#define IDTP9382A_REG_SYS_OP_MODE_TX_BIT	BIT(2)
#define IDTP9382A_REG_SYS_OP_MODE_WPCEPP_BIT	BIT(3)
#define IDTP9382A_REG_FW_SRC_MASK		GENMASK(6, 4)
#define IDTP9382A_REG_FW_SRC_OTP_BIT		BIT(4)
#define IDTP9382A_REG_FW_SRC_EEPROM_BIT		BIT(5)
#define IDTP9382A_REG_FW_SRC_RAM_BIT		BIT(6)
#define IDTP9382A_REG_COM			0x4C
#define IDTP9382A_REG_FW_SWITCH_KEY		0x4D
#define IDTP9382A_REG_NFC_STATUS		0x4F

#define IDTP9382A_REG_RPP			0x50
#define IDTP9382A_REG_CEP_VALUE			0x58
#define IDTP9382A_REG_ADC_VRECT_L		0x60
#define IDTP9382A_REG_ADC_VRECT_H		0x61
#define IDTP9382A_REG_ADC_VOUT_L		0x62
#define IDTP9382A_REG_ADC_VOUT_H		0x63
#define IDTP9382A_REG_RX_IOUT_L			0x64
#define IDTP9382A_REG_RX_IOUT_H			0x65
#define IDTP9382A_REG_ADC_DIE_TEMP_L		0x66
#define IDTP9382A_REG_ADC_DIE_TEMP_H		0x67
#define IDTP9382A_REG_RX_AC_PERIOD		0x68
#define IDTP9382A_REG_PING_FREQ_L		0x6A
#define IDTP9382A_REG_PING_FREQ_H		0x6B
#define IDTP9382A_REG_ADC_VTHERM_L		0x6C
#define IDTP9382A_REG_ADC_VTHERM_H		0x6D
#define IDTP9382A_REG_FOD_0_A			0x70
#define IDTP9382A_REG_FOD_0_B			0x71
#define IDTP9382A_REG_FOD_1_A			0x72
#define IDTP9382A_REG_FOD_1_B			0x73
#define IDTP9382A_REG_FOD_2_A			0x74
#define IDTP9382A_REG_FOD_2_B			0x75
#define IDTP9382A_REG_FOD_3_A			0x76
#define IDTP9382A_REG_FOD_3_B			0x77
#define IDTP9382A_REG_FOD_4_A			0x78
#define IDTP9382A_REG_FOD_4_B			0x79
#define IDTP9382A_REG_FOD_5_A			0x7A
#define IDTP9382A_REG_FOD_5_B			0x7B
#define IDTP9382A_REG_FOD_EXPANSION_A		0x7C
#define IDTP9382A_REG_FOD_EXPANSION_B		0x7D
#define IDTP9382A_REG_FOD_EPP_MODE_1_A		0x7E
#define IDTP9382A_REG_FOD_EPP_MODE_1_B		0x7F

#define IDTP9382A_REG_DEBUG_L			0x80
#define IDTP9382A_REG_DEBUG_H			0x81

#define IDTP9382A_REG_EPP_Q_FACTOR		0x83
#define IDTP9382A_REG_EPP_GUARANTEED		0x84
#define IDTP9382A_REG_EPP_POTENTIAL		0x85
#define IDTP9382A_REG_EPP_CAPABILITY		0x86
#define IDTP9382A_REG_EPP_RENAGOTIATION_STATUS	0x87
#define IDTP9382A_REG_EPP_CURRENT_RPP		0x88
#define IDTP9382A_REG_EPP_CURRENT_NEGO_PWR	0x89
#define IDTP9382A_REG_EPP_MAX_PWR		0x8A
#define IDTP9382A_REG_EPP_CURRENT_FSK_MOD	0x8B
#define IDTP9382A_REG_EPP_REQUEST_RPP		0x8C
#define IDTP9382A_REG_EPP_REQUEST_NEGO_PWR	0x8D
#define IDTP9382A_REG_EPP_REQUEST_MAX_PWR	0x8E
#define IDTP9382A_REG_EPP_REQUEST_FSK_MOD	0x8F

#define IDTP9382A_REG_VRECT_TARGET		0x90
#define IDTP9382A_REG_VRECT_KNEE		0x92
#define IDTP9382A_REG_VRECT_CORRECTION_FACTOR	0x93
#define IDTP9382A_REG_VRECT_MAX_CORRECTION	0x94
#define IDTP9382A_REG_VRECT_MIN_CORRECTION	0x96

#define IDTP9382A_REG_FOD_STATE			0x9C
#define IDTP9382A_REG_FOD_SECTION		0x9D
#define IDTP9382A_REG_VRECT_HISTORY		0x9E
#define IDTP9382A_REG_IRECT_HISTORY		0xA0

#define IDTP9382A_REG_ALIGNMENT_X_AXIS		0xB0
#define IDTP9382A_REG_ALIGNMENT_Y_AXIS		0xB1
#define IDTP9382A_REG_ASK_MODULATION_DEPTH	0xB2
#define IDTP9382A_REG_OVSET			0xB3

#define IDTP9382A_REG_EPP_SPECIFICATION_REV	0xB9
#define IDTP9382A_REG_EPP_MANUFACTURER_CODE	0xBA

#define IDTP9382A_REG_PIN_1_RESET_VOLTAGE	0xBC

#define IDTP9382A_REG_CONFIG_TABLE_SIZE		0xC8
#define IDTP9382A_REG_CONFIG_TABLE_CMD		0xCA

#define IDTP9382A_REG_EXT_TEMP_ALARM		0xDA

#define FOD_REG_START_ADDR	IDTP9382A_REG_FOD_0_A
#define FOD_REG_END_ADDR	IDTP9382A_REG_FOD_EPP_MODE_1_B
#define FOD_REG_NUM		(FOD_REG_END_ADDR - FOD_REG_START_ADDR + 1)

#define IDTP9382A_ADC_TO_VRECT_MV(adc)	(adc * 21000 / 4095)
#define IDTP9382A_ADC_TO_VOUT_MV(adc)	(adc * 12600 / 4095)
#define IDTP9382A_ADC_TO_MA(adc)		(adc * 1)
#define IDTP9382A_ADC_TO_DIE_TEMP_C(adc)	((adc - 1350) * 83 / 444 - 273)
#define IDTP9382A_ADC_TO_RPP_BPP_MW(adc)	(adc * 5000 / 32768)
#define IDTP9382A_ADC_TO_RPP_EPP_MW(adc)	(adc * 10000 / 32768)
#define IDTP9382A_ADC_TO_SIGSTR_PC(adc)	DIV_ROUND_CLOSEST(adc * 100, 256)

#define IDTP9382A_PWR_TO_MW(val)		(val * 500)

#define IDTP9382A_INTERVAL_POWER_OK_DEFAULT_MS		3000
#define IDTP9382A_INTERVAL_NEGOTIATION_DEFAULT_MS	50

#define IDTP9382A_TXID_CURRENT_LIMITED_EPP		0x28
#define IDTP9382A_TXID_AICL_FAST_STEP_EPP		0x00
#define IDTP9382A_TXID_UNSTABLE_EPP			0x27

const u8 idtp9382a_bpp_fod_param[FOD_REG_NUM] = {
	0x98, 0x32, 0x8F, 0x30, 0x8E, 0x30, 0x98, 0x15,
	0x97, 0x1E, 0xA4, 0xF7, 0x14, 0x00, 0x01, 0x50,
};

const u8 idtp9382a_epp_fod_param[FOD_REG_NUM] = {
	0x96, 0x3C, 0x87, 0x53, 0x93, 0x2F, 0x8F, 0x38,
	0x94, 0x17, 0x95, 0x19, 0x14, 0x00, 0x01, 0x50,
};

enum {
	IDTP9382A_ENABLE_CLIENT_DEBUG = 0,
	IDTP9382A_ENABLE_CLIENT_USB,
	IDTP9382A_ENABLE_CLIENT_DEV1,
	IDTP9382A_ENABLE_CLIENT_THERM_WA,
	IDTP9382A_ENABLE_CLIENT_RECONNECTION,
	IDTP9382A_ENABLE_CLIENT_MAX,
};

enum {
	IDTP9382A_STATUS_PWR_OFF = 0,
	IDTP9382A_STATUS_PWR_ON_NEGOTIATION,
	IDTP9382A_STATUS_PWR_POWER_OK,
};

static const char * const str_status[] = {"POFF", "NEGO", "PWON"};

enum {
	IDTP9382A_PSY_WIRELESS_MODE_OFF = 0,
	IDTP9382A_PSY_WIRELESS_MODE_BPP,
	IDTP9382A_PSY_WIRELESS_MODE_EPP,
	IDTP9382A_PSY_WIRELESS_MODE_LIM_EPP,
	IDTP9382A_PSY_WIRELESS_MODE_FAST_STEP_EPP,
	IDTP9382A_PSY_WIRELESS_MODE_UNKNOWN,
	IDTP9382A_PSY_WIRELESS_MODE_UNSETTLED,
};

enum {
	IDTP9382A_OP_MODE_UNKNOWN = 0,
	IDTP9382A_OP_MODE_QI_BPP,
	IDTP9382A_OP_MODE_QI_EPP,
	IDTP9382A_OP_MODE_PMA,
};

enum {
	IDTP9382A_FW_SRC_NONE = 0,
	IDTP9382A_FW_SRC_OTP,
	IDTP9382A_FW_SRC_EEPROM,
	IDTP9382A_FW_SRC_RAM,
};

struct idtp9382a {
	struct device		*dev;
	struct i2c_client	*i2c_handle;

	/* virtual_device */
	struct device		*vdev;
	struct class		*vcls;

	/* power supply */
	struct power_supply	*wireless_psy;
	struct power_supply	*dc_psy;
	struct power_supply	*usb_psy;
	struct power_supply	*batt_psy;
	int			usbin_valid;
	int			wireless_suspend;
	int			mitigation_vout_set_mv;

	/* gpio/irq */
	int			wlc_en;
	int			wlc_en_rsvd;
	int			wlc_irq;
	int			wlc_gpio_irq;

	/* worker */
	struct delayed_work	detect_work;
	struct delayed_work	vout_change_work;
	struct delayed_work	wlc_reconnection_enable_work;
	struct mutex		reg_ctrl_lock;
	struct mutex		i2c_lock;
	struct mutex		vout_set_lock;
	bool			is_worker_active;
	bool			wake_lock_en;

	/* wlc_en control */
	bool			halt_client[IDTP9382A_ENABLE_CLIENT_MAX];
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
	int			wlc_therm_temp_dc;
	int			rpp_raw;
	int			rpp_mw;
	int			sigstr_pc;
	int			txid;
	int			ping_freq_khz;
	int			guaranteed_pwr_mw;
	int			potential_pwr_mw;

	/* write resister items */
	int			vout_set_mv;
	int			ilim_set_ma;

	/* misc ctrl */
	int			target_vout_set_mv;

	/* status */
	int			status;
	int			psy_wireless_mode;
	int			old_status;
	bool			one_time_read_in_driver_done;
	bool			one_time_read_in_pwr_ok_done;
	int			wireless_chg_negotiation;
	bool			wlc_reconnection_running;

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
	ktime_t			dumped_time;

	/* configration from device tree */
	int			bpp_ilim_ma;
	int			epp_ilim_ma;
	int			bpp_vout_mv;
	int			epp_vout_mv;

	/* power_ok delay WA */
	bool			vout_check_done;
	int			vout_ok_counter;
	int			vout_check_counter;

	/* dcin_en status for abnormal negotiation detection */
	int			abnormal_negotiation;
};

#define idtp9382a_err(chip, fmt, ...)	pr_err(fmt, ##__VA_ARGS__)
#define idtp9382a_dbg(chip, reason, fmt, ...)				\
	do {								\
		if (*chip->debug_mask & (reason))			\
			pr_err(fmt, ##__VA_ARGS__);			\
		else							\
			pr_debug(fmt, ##__VA_ARGS__);			\
	} while (0)

/* Debug flag definitions */
enum idtp9382a_debug_flag {
	PR_MAJOR	= BIT(0),
	PR_MONITOR	= BIT(1),
	PR_STATUS	= BIT(2),
	PR_REG_I2C	= BIT(3),
	PR_WORKER	= BIT(4),
	PR_PSY		= BIT(5),
	PR_MISC		= BIT(6),
	PR_MEASUREMENT	= BIT(7),
};

static int idtp9382a_debug_mask = PR_MAJOR | PR_MONITOR;
static int idtp9382a_debug_dump;
module_param_named(debug_mask, idtp9382a_debug_mask, int, 0600);
module_param_named(reg_dump, idtp9382a_debug_dump, int, 0600);

#define IDTP9382A_I2C_RETRY_MAX 3
#define IDTP9382A_I2C_RETRY_DELAY_MS 5
static int idtp9382a_i2c_write_byte(struct idtp9382a *chip, u8 reg, u8 val)
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
	while (cnt < IDTP9382A_I2C_RETRY_MAX) {
		rc = i2c_master_send(client, buf, length);
		if (rc < length) {
			dev_err(chip->dev, "Can't write i2c %d\n", rc);
			cnt++;
			msleep(IDTP9382A_I2C_RETRY_DELAY_MS);
		} else {
			idtp9382a_dbg(chip, PR_REG_I2C,
				"[W] addr 0x%02x : 0x%02x\n", reg, val);
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static int idtp9382a_i2c_write_sequential(struct idtp9382a *chip, u8 reg,
							const u8 *val, int len)
{
	int rc = 0;
	int cnt = 0;
	struct i2c_client *client = chip->i2c_handle;
	u8 *buf;
	int buf_len = len + 2;

	mutex_lock(&chip->i2c_lock);

	buf = kmalloc(buf_len, GFP_KERNEL);
	if (!buf) {
		rc = -ENOMEM;
		goto err;
	}

	buf[0] = 0x00;
	buf[1] = reg;
	memcpy(&buf[2], val, len);

	while (cnt < IDTP9382A_I2C_RETRY_MAX) {
		rc = i2c_master_send(client, buf, buf_len);
		if (rc < buf_len) {
			dev_err(chip->dev, "Can't write i2c %d\n", rc);
			cnt++;
			msleep(IDTP9382A_I2C_RETRY_DELAY_MS);
		} else {
			idtp9382a_dbg(chip, PR_REG_I2C,
					"[W] addr 0x%02x to 0x%02x\n", reg,
								reg + len - 1);
			break;
		}
	}
	kfree(buf);
err:
	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static int idtp9382a_i2c_read_byte(struct idtp9382a *chip, u8 reg, u8 *val)
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

	while (cnt < IDTP9382A_I2C_RETRY_MAX) {
		rc = i2c_transfer(client->adapter, msg, 2);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read i2c %d retry=%d\n",
								rc, cnt);
			cnt++;
			msleep(IDTP9382A_I2C_RETRY_DELAY_MS);
		} else {
			idtp9382a_dbg(chip, PR_REG_I2C,
					"[R] addr 0x%02x : 0x%02x\n",
					reg, *val);
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static int idtp9382a_i2c_read_sequential(struct idtp9382a *chip, u8 reg,
							u8 *val, int length)
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

	while (cnt < IDTP9382A_I2C_RETRY_MAX) {
		rc = i2c_transfer(client->adapter, msg, 2);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read i2c %d retry=%d\n",
								rc, cnt);
			cnt++;
			msleep(IDTP9382A_I2C_RETRY_DELAY_MS);
		} else {
			break;
		}
	}

	mutex_unlock(&chip->i2c_lock);
	return rc;
}

static void idtp9382a_dump_reg(struct idtp9382a *chip)
{
	int addr = 0;
	int rc = 0;
	char addr_1[24];
	char addr_2[24];
	ktime_t ktime_now;
	s64 time_diff_ms;

	ktime_now = ktime_get_boottime();
	time_diff_ms = ktime_to_ms(ktime_sub(ktime_now, chip->dumped_time));
	if (time_diff_ms < idtp9382a_debug_dump * 1000)
		return;

	chip->dumped_time = ktime_now;

	rc = idtp9382a_i2c_read_sequential(chip, 0, chip->reg_shadow, 256);
	if (rc < 0) {
		pr_err("Can't dump reg %d\n", rc);
		return;
	}
	pr_err("----------------------------------------------------\n");
	pr_err("Addr|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	pr_err("----------------------------------------------------\n");
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

		pr_err("0x%02X|%s %s\n", addr, addr_1, addr_2);
		addr += 16;
	}
	pr_err("----------------------------------------------------\n");
}

static int idtp9382a_get_op_mode(struct idtp9382a *chip)
{
	u8 reg;
	int rc;

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_SYS_OP_MODE, &reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (reg & IDTP9382A_REG_SYS_OP_MODE_WPCEPP_BIT)
		chip->op_mode = IDTP9382A_OP_MODE_QI_EPP;
	else
		chip->op_mode = IDTP9382A_OP_MODE_QI_BPP;
	idtp9382a_dbg(chip, PR_STATUS, "op_mode : %d\n", chip->op_mode);

	return 0;
}

#define VOUT_STEP 10
#define VOUT_WAIT_MS 200
static int idtp9382a_set_vout_set(struct idtp9382a *chip, int val)
{
	if (chip->debug_vout_set_mv)
		val = chip->debug_vout_set_mv;

	if (val < 3500 || val > 12500)
		return -EINVAL;

	mutex_lock(&chip->vout_set_lock);
	chip->target_vout_set_mv = val;
	mutex_unlock(&chip->vout_set_lock);
	schedule_delayed_work(&chip->vout_change_work,
					msecs_to_jiffies(VOUT_WAIT_MS));

	return 0;
}

static int idtp9382a_handle_vout_set(struct idtp9382a *chip)
{
	int rc = 0;
	int vout_mv = 0;

	if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK) {
		if (chip->op_mode == IDTP9382A_OP_MODE_QI_EPP) {
			if (chip->mitigation_vout_set_mv > 0 &&
						chip->mitigation_vout_set_mv <
							chip->epp_vout_mv)
				vout_mv = chip->mitigation_vout_set_mv;
			else
				vout_mv = chip->epp_vout_mv;

		} else if (chip->op_mode == IDTP9382A_OP_MODE_QI_BPP) {
			vout_mv = chip->bpp_vout_mv;
		}
		idtp9382a_dbg(chip, PR_MISC, "vout:%dmV\n", vout_mv);
		if (vout_mv > 0) {
			rc = idtp9382a_set_vout_set(chip, vout_mv);
			if (rc)
				pr_err("Error in set_vout_set rc=%d\n", rc);
		}
	}
	return rc;
}

static void idtp9382a_vout_change_work(struct work_struct *work)
{
	struct idtp9382a *chip = container_of(work,
				struct idtp9382a,
				vout_change_work.work);
	int rc;
	u8 current_v_reg, target_v_reg;
	int current_v_mv, target_v_mv;
	int val;

	mutex_lock(&chip->reg_ctrl_lock);

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_VOUT_SET,
						&current_v_reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		goto error;
	}
	current_v_mv = current_v_reg * 100;
	target_v_mv = (chip->target_vout_set_mv / 100) * 100;
	target_v_reg = (u8)(target_v_mv / 100);
	idtp9382a_dbg(chip, PR_MISC, "now:%dmV, target:%dmV\n",
					current_v_mv, target_v_mv);

	if (current_v_reg + VOUT_STEP < target_v_reg) {
		/* Increase Vout step by step */
		val = current_v_reg + VOUT_STEP;
		idtp9382a_dbg(chip, PR_MISC, "Step Vout(Inc): 0x%02x\n", val);
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_VOUT_SET,
								val);
		if (rc < 0) {
			dev_err(chip->dev, "write error VOUT_SET %d\n", rc);
			goto error;
		}
		goto schedule_next;
	} else if (current_v_reg - VOUT_STEP > target_v_reg) {
		/* Decrease Vout step by step */
		val = current_v_reg - VOUT_STEP;
		idtp9382a_dbg(chip, PR_MISC, "Step Vout(Dec): 0x%02x\n", val);
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_VOUT_SET,
								val);
		if (rc < 0) {
			dev_err(chip->dev, "write error VOUT_SET %d\n", rc);
			goto error;
		}
		goto schedule_next;
	} else if (current_v_reg != target_v_reg) {
		/* Finally set to targert Vout */
		val = target_v_reg;
		idtp9382a_dbg(chip, PR_MAJOR, "Step Vout(Goal): 0x%02x\n",
								val);
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_VOUT_SET,
								val);
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

static int idtp9382a_set_ilim_set(struct idtp9382a *chip, int val)
{
	int rc;
	u8 current_ilim_reg, target_ilim_reg;

	if (chip->debug_ilim_set_ma)
		val = chip->debug_ilim_set_ma;

	if (val > 1300) {
		pr_err("Invalid ilim parameter val:%d\n", val);
		return -EINVAL;
	}
	target_ilim_reg = val / 50 - 1;

	if (chip->status == IDTP9382A_STATUS_PWR_OFF)
		return 0;

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_ILIM_SET,
						&current_ilim_reg);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (current_ilim_reg != target_ilim_reg) {
		idtp9382a_dbg(chip, PR_MAJOR,
			"set ilim_set to %dmA (from %d)\n",
			target_ilim_reg * 50 + 50, current_ilim_reg * 50 + 50);
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_ILIM_SET,
							target_ilim_reg);
		if (rc < 0) {
			dev_err(chip->dev, "read error ILIM_SET %d\n", rc);
			return rc;
		}
		chip->ilim_set_ma = val;
	}

	return 0;
}

static int idtp9382a_set_fod(struct idtp9382a *chip, int reg_offset, u8 val)
{
	int rc = 0;
	u8 reg_addr;

	reg_addr = FOD_REG_START_ADDR + reg_offset;
	if (reg_addr > FOD_REG_END_ADDR) {
		dev_err(chip->dev,
			"invalid parameter reg_offset:%d\n", reg_offset);
		return rc;
	}

	idtp9382a_dbg(chip, PR_MAJOR, "set FOD reg:0x%02x, val:0x%02x\n",
								reg_addr, val);

	if (chip->status != IDTP9382A_STATUS_PWR_OFF) {
		rc = idtp9382a_i2c_write_byte(chip, reg_addr, val);
		if (rc < 0) {
			dev_err(chip->dev, "read error FOD %d\n", rc);
			return rc;
		}
	}

	return rc;
}

static int idtp9382a_set_fod_all(struct idtp9382a *chip)
{
	int rc = 0;

	if (chip->status == IDTP9382A_STATUS_PWR_OFF)
		return 0;

	if (chip->fod_adj_en)
		rc = idtp9382a_i2c_write_sequential(chip, FOD_REG_START_ADDR,
						chip->fod_adj, FOD_REG_NUM);
	else if (chip->op_mode == IDTP9382A_OP_MODE_QI_BPP)
		rc = idtp9382a_i2c_write_sequential(chip, FOD_REG_START_ADDR,
					idtp9382a_bpp_fod_param, FOD_REG_NUM);
	else if (chip->op_mode == IDTP9382A_OP_MODE_QI_EPP)
		rc = idtp9382a_i2c_write_sequential(chip, FOD_REG_START_ADDR,
					idtp9382a_epp_fod_param, FOD_REG_NUM);

	if (rc < 0)
		dev_err(chip->dev, "failed set FOD %d\n", rc);

	return (rc < 0) ? rc : 0;
}

static int idtp9382a_read_cmd_reg_byte(struct idtp9382a *chip, u8 addr,
								u8 *data)
{
	int rc = 0;

	if (chip->status != IDTP9382A_STATUS_PWR_OFF) {
		rc = idtp9382a_i2c_read_byte(chip, addr, data);
		if (rc < 0) {
			dev_err(chip->dev, "Can't read register %d\n", rc);
			return rc;
		}
	}

	return rc;
}

static int idtp9382a_write_cmd_reg_byte(struct idtp9382a *chip, u8 addr,
								u8 data)
{
	int rc = 0;

	if (chip->status != IDTP9382A_STATUS_PWR_OFF) {
		rc = idtp9382a_i2c_write_byte(chip, addr, data);
		if (rc < 0) {
			dev_err(chip->dev, "write error register %d\n", rc);
			return rc;
		}
	}

	return rc;
}

#define IDTP9382A_EPT_MODE_UNKNOWN	0
#define IDTP9382A_EPT_MODE_MAX		8
#define IDTP9382A_EPT_SEND_COM		0x08
static int idtp9382a_set_ept(struct idtp9382a *chip, int mode)
{
	int rc = 0;

	if (mode < IDTP9382A_EPT_MODE_UNKNOWN ||
			mode > IDTP9382A_EPT_MODE_MAX) {
		pr_err("Invalid mode %d\n", mode);
		return -EINVAL;
	}

	if (chip->status != IDTP9382A_STATUS_PWR_OFF) {
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_EPT_CODE,
								mode);
		if (rc < 0) {
			dev_err(chip->dev, "write error of EPT mode %d\n", rc);
			return rc;
		}
		rc = idtp9382a_i2c_write_byte(chip, IDTP9382A_REG_COM,
						IDTP9382A_EPT_SEND_COM);
		if (rc < 0) {
			dev_err(chip->dev, "write error of EPT send cmd %d\n",
									rc);
			return rc;
		}
		idtp9382a_dbg(chip, PR_MAJOR, "Sent EPT command. mode:%d\n",
									mode);
	}
	return rc;
}

static int idtp9382a_read_regs_oneshot_in_driver(struct idtp9382a *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[12];

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_SYS_OP_MODE,
								&reg_buf[0]);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}

	if (reg_buf[0] & IDTP9382A_REG_FW_SRC_OTP_BIT)
		chip->fw_src = IDTP9382A_FW_SRC_OTP;
	else if (reg_buf[0] & IDTP9382A_REG_FW_SRC_EEPROM_BIT)
		chip->fw_src = IDTP9382A_FW_SRC_EEPROM;
	else if (reg_buf[0] & IDTP9382A_REG_FW_SRC_RAM_BIT)
		chip->fw_src = IDTP9382A_FW_SRC_RAM;
	else
		chip->fw_src = IDTP9382A_FW_SRC_NONE;
	idtp9382a_dbg(chip, PR_MAJOR, "FW Source : %d\n", chip->fw_src);

	start_addr = IDTP9382A_REG_OTP_FW_MAJOR_REV_L;
	length = 4;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->fw_rev_major_l = reg_buf[0];
	chip->fw_rev_major_h = reg_buf[1];
	chip->fw_rev_minor_l = reg_buf[2];
	chip->fw_rev_minor_h = reg_buf[3];
	idtp9382a_dbg(chip, PR_MAJOR, "OTP FW Rev: %02x %02x %02x %02x\n",
				chip->fw_rev_major_l, chip->fw_rev_major_h,
				chip->fw_rev_minor_l, chip->fw_rev_minor_h);

	start_addr = IDTP9382A_REG_OTP_FW_DATE_CODE;
	length = 12;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	idtp9382a_dbg(chip, PR_MAJOR, "FW date: %s\n",
				reg_buf);

	start_addr = IDTP9382A_REG_SRAM_FW_MAJOR_REV_L;
	length = 4;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->fw_rev_sram_major_l = reg_buf[0];
	chip->fw_rev_sram_major_h = reg_buf[1];
	chip->fw_rev_sram_minor_l = reg_buf[2];
	chip->fw_rev_sram_minor_h = reg_buf[3];
	idtp9382a_dbg(chip, PR_MAJOR, "EEPROM FW Rev: %02x %02x %02x %02x\n",
			chip->fw_rev_sram_major_l, chip->fw_rev_sram_major_h,
			chip->fw_rev_sram_minor_l, chip->fw_rev_sram_minor_h);

	idtp9382a_dbg(chip, PR_MAJOR, "Wireless charger is activated\n");

	return 0;
}

static int idtp9382a_read_regs_oneshot_in_pwr_ok(struct idtp9382a *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[4];

	/* read OP mode */
	rc = idtp9382a_get_op_mode(chip);
	if (rc) {
		pr_err("Error in get_op_mode rc=%d\n", rc);
		return rc;
	}

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_SIGSTR, &reg_buf[0]);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}
	chip->sigstr_pc = IDTP9382A_ADC_TO_SIGSTR_PC(reg_buf[0]);

	start_addr = IDTP9382A_REG_EPP_MANUFACTURER_CODE;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->txid = reg_buf[0] | (reg_buf[1] << 8);

	start_addr = IDTP9382A_REG_PING_FREQ_L;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->ping_freq_khz = reg_buf[0] | (reg_buf[1] << 8);

	start_addr = IDTP9382A_REG_EPP_GUARANTEED;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->guaranteed_pwr_mw = IDTP9382A_PWR_TO_MW(reg_buf[0]);
	chip->potential_pwr_mw = IDTP9382A_PWR_TO_MW(reg_buf[1]);

	idtp9382a_dbg(chip, PR_MONITOR,
			"[%s] Ping:%dkHz, SigStr:%d%%, TxID:0x%04x(%d), guaranteed_pwr:%dmW, potential_pwr:%dmW\n",
			str_status[chip->status],
			chip->ping_freq_khz, chip->sigstr_pc,
			chip->txid, chip->txid, chip->guaranteed_pwr_mw,
			chip->potential_pwr_mw);
	return 0;
}

static int idtp9382a_read_regs_each_cycle(struct idtp9382a *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[2];
	int vrect;

	start_addr = IDTP9382A_REG_VRECT;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	vrect = reg_buf[0] | (reg_buf[1] << 8);
	chip->vrect_mv = vrect;
	idtp9382a_dbg(chip, PR_MONITOR, "[%s] Vrect=%dmV\n",
				str_status[chip->status], chip->vrect_mv);

	return 0;
}

static int idtp9382a_read_regs_debug(struct idtp9382a *chip)
{
	int rc;
	u8 start_addr;
	int length;
	u8 reg_buf[6];
	int vout, iout, die_temp_adc, rpp_adc;
	u8 val;

	if (!chip->monitor_for_debug)
		return 0;

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_EPT_CODE, &val);
	if (rc < 0) {
		pr_err("Can't read reg %d\n", rc);
		return rc;
	}
	chip->reg_ept = val;

	start_addr = IDTP9382A_REG_VOUT;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	vout = reg_buf[0] | (reg_buf[1] << 8);
	chip->vout_mv = vout;

	start_addr = IDTP9382A_REG_RX_IOUT;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	iout = reg_buf[0] | (reg_buf[1] << 8);
	chip->iout_ma = iout;

	start_addr = IDTP9382A_REG_ADC_DIE_TEMP_L;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	die_temp_adc = reg_buf[0] | (reg_buf[1] << 8);
	chip->die_temp_c = IDTP9382A_ADC_TO_DIE_TEMP_C(die_temp_adc);

	start_addr = IDTP9382A_REG_OP_FREQ_L;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	chip->op_freq_khz = reg_buf[0] | (reg_buf[1] << 8);

	start_addr = IDTP9382A_REG_RPP;
	length = 4;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return rc;
	}
	rpp_adc = reg_buf[0] | (reg_buf[1] << 8);
	rpp_adc |= (reg_buf[2] << 16) | (reg_buf[3] << 24);
	chip->rpp_raw = rpp_adc;
	if (chip->op_mode == IDTP9382A_OP_MODE_QI_EPP)
		chip->rpp_mw = IDTP9382A_ADC_TO_RPP_EPP_MW(rpp_adc);
	else
		chip->rpp_mw = IDTP9382A_ADC_TO_RPP_BPP_MW(rpp_adc);

	idtp9382a_dbg(chip, PR_MONITOR,
			"[%s] Vout=%dmv Iout=%dmA DieTemp=%dC op_freq=%dkHz RPP=%dmW (EPT/EOC/EOP Reason=0x%02X)\n",
			str_status[chip->status], chip->vout_mv,
			chip->iout_ma, chip->die_temp_c,
			chip->op_freq_khz, chip->rpp_mw, chip->reg_ept);

	return 0;
}

static void idtp9382a_control_enable(struct idtp9382a *chip, bool enable)
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
	idtp9382a_dbg(chip, PR_MISC, "%d\n", enable);
}

static void idtp9382a_handle_halt_proc(struct idtp9382a *chip)
{
	int i = 0;
	bool halt = false;

	chip->halt_retry = false;

	while (i < IDTP9382A_ENABLE_CLIENT_MAX) {
		if (chip->halt_client[i]) {
			halt = true;
			break;
		}
		i++;
	}

	if (halt) {
		if (chip->is_worker_active) {
			/* wait for the end of worker */
			idtp9382a_dbg(chip, PR_MAJOR,
					"wait for worker's end point\n");
			chip->halt_retry = true;
			return;
		}
		idtp9382a_dbg(chip, PR_MAJOR, "Disable WLC_EN\n");
		idtp9382a_control_enable(chip, false);
	} else {
		idtp9382a_dbg(chip, PR_MAJOR,
					"Enable WLC_EN due to client:%d\n", i);
		idtp9382a_control_enable(chip, true);
	}
}

static int idtp9382a_handle_halt(struct idtp9382a *chip, int client, bool halt)
{
	if (client >= IDTP9382A_ENABLE_CLIENT_MAX)
		return -EINVAL;

	if (chip->disable_halt)
		return 0;

	idtp9382a_dbg(chip, PR_MISC, "client:%d halt:%d\n", client, (int)halt);
	chip->halt_client[client] = halt;

	idtp9382a_handle_halt_proc(chip);

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
	ATTR_OP_FREQ_KHZ,
	ATTR_REG_EPT,
	ATTR_SIGSTR_PC,
	ATTR_TXID,
	ATTR_RPP_MW,
	ATTR_PING_FREQ_KHZ,
	ATTR_GUARANTEED_PWR_MW,
	ATTR_POTENTIAL_PWR_MW,
};

static ssize_t idtp9382a_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);

static ssize_t idtp9382a_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size);

static struct device_attribute wireless_attrs[] = {
	__ATTR(somc_interval_power_ok_ms,	0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_interval_negotiation_ms,	0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_monitor_for_debug,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_wlc_en,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_halt_for_debug,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_disable_halt,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_vout_set_mv,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_ilim_set_ma,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_0_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_0_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_1_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_1_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_2_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_2_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_3_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_3_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_4_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_4_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_5_a,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_5_b,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_expansion_a,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_expansion_b,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_epp_mode_1_a,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_epp_mode_1_b,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fod_adjust_en,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_send_ept,			0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_cmd_reg_addr,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_cmd_reg_data,		0644, idtp9382a_param_show,
						idtp9382a_param_store),
	__ATTR(somc_fw_rev,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_vrect_mv,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_vout_mv,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_iout_ma,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_die_temp_c,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_op_freq_khz,	0444, idtp9382a_param_show, NULL),
	__ATTR(somc_reg_ept,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_sigstr_pc,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_txid,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_rpp_mw,		0444, idtp9382a_param_show, NULL),
	__ATTR(somc_ping_freq_khz,	0444, idtp9382a_param_show, NULL),
	__ATTR(somc_guaranteed_pwr_mw,	0444, idtp9382a_param_show, NULL),
	__ATTR(somc_potential_pwr_mw,	0444, idtp9382a_param_show, NULL),
};

static ssize_t idtp9382a_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	const ptrdiff_t off = attr - wireless_attrs;
	struct idtp9382a *chip = dev_get_drvdata(dev);
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
		ret = idtp9382a_read_cmd_reg_byte(chip, chip->cmd_reg_addr,
						&chip->cmd_reg_data);
		if (ret < 0)
			return ret;

		size = scnprintf(buf, PAGE_SIZE,
				"reg_addr:0x%02X\nreg_data:0x%02X\n",
				chip->cmd_reg_addr, chip->cmd_reg_data);
		break;

	case ATTR_FW_REV:
		if (chip->fw_src == IDTP9382A_FW_SRC_OTP)
			size = scnprintf(buf, PAGE_SIZE, "OTP:%02x%02x%02x\n",
							chip->fw_rev_major_l,
							chip->fw_rev_minor_h,
							chip->fw_rev_minor_l);
		else if (chip->fw_src == IDTP9382A_FW_SRC_EEPROM ||
			 chip->fw_src == IDTP9382A_FW_SRC_RAM)
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
	case ATTR_OP_FREQ_KHZ:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->op_freq_khz);
		break;

	case ATTR_REG_EPT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->reg_ept);
		break;

	case ATTR_SIGSTR_PC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->sigstr_pc);
		break;

	case ATTR_TXID:
		size = scnprintf(buf, PAGE_SIZE, "0x%04x(%d)\n",
						chip->txid, chip->txid);
		break;

	case ATTR_RPP_MW:
		size = scnprintf(buf, PAGE_SIZE, "%d(raw:%d)\n",
					chip->rpp_mw, chip->rpp_raw);
		break;

	case ATTR_PING_FREQ_KHZ:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->ping_freq_khz);
		break;

	case ATTR_GUARANTEED_PWR_MW:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->guaranteed_pwr_mw);
		break;

	case ATTR_POTENTIAL_PWR_MW:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chip->potential_pwr_mw);
		break;

	default:
		size = 0;
		break;
	}
	return size;
}

static ssize_t idtp9382a_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	const ptrdiff_t off = attr - wireless_attrs;
	struct idtp9382a *chip = dev_get_drvdata(dev);
	int ret;
	int val;
	u8 reg_offset;

	switch (off) {
	case ATTR_INTERVAL_POWER_OK_MS:
		ret = kstrtoint(buf, 10, &chip->interval_power_ok_ms);
		if (ret < 0)
			return ret;
		break;

	case ATTR_INTERVAL_NEGOTIATION_MS:
		ret = kstrtoint(buf, 10, &chip->interval_negotiation_ms);
		if (ret < 0)
			return ret;
		break;

	case ATTR_MONITOR_FOR_DEBUG:
		ret = kstrtoint(buf, 10, &chip->monitor_for_debug);
		if (ret < 0)
			return ret;
		break;

	case ATTR_WLC_EN:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0)
			return ret;
		idtp9382a_dbg(chip, PR_MISC, "directry ctrl WLC_EN pin %d\n",
									val);
		idtp9382a_control_enable(chip, val);
		break;

	case ATTR_HALT_FOR_DEBUG:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0)
			return ret;
		ret = idtp9382a_handle_halt(chip, IDTP9382A_ENABLE_CLIENT_DEBUG,
									val);
		if (ret < 0)
			return ret;
		chip->halt_for_debug = val;
		break;

	case ATTR_DISABLE_HALT:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0)
			return ret;
		chip->disable_halt = val == 1 ? true : false;
		break;

	case ATTR_VOUT_SET_MV:
		ret = kstrtoint(buf, 10, &chip->debug_vout_set_mv);
		if (ret < 0)
			return ret;
		ret = idtp9382a_set_vout_set(chip, chip->debug_vout_set_mv);
		if (ret < 0)
			return ret;
		break;

	case ATTR_ILIM_SET_MA:
		ret = kstrtoint(buf, 10, &chip->debug_ilim_set_ma);
		if (ret < 0)
			return ret;
		ret = idtp9382a_set_ilim_set(chip, chip->debug_ilim_set_ma);
		if (ret < 0)
			return ret;
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
		if (ret < 0)
			return ret;
		reg_offset = off - ATTR_FOD_0_A;

		if (reg_offset < 0)
			break;

		chip->fod_adj[reg_offset] = (u8)val;
		if (chip->fod_adj_en) {
			ret = idtp9382a_set_fod(chip, reg_offset,
				chip->fod_adj[reg_offset]);
			if (ret < 0)
				return ret;
		}

		idtp9382a_dbg(chip, PR_MISC, "fod_adj[%d] -> 0x%02x\n",
					(int)reg_offset,
					(int)chip->fod_adj[reg_offset]);
		break;

	case ATTR_FOD_ADJUST_EN:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0)
			return ret;
		if (val == 1) {
			chip->fod_adj_en = true;
			if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK) {
				ret = idtp9382a_set_fod_all(chip);
				if (ret < 0)
					return ret;
			}

			idtp9382a_dbg(chip, PR_MAJOR,
						"Enabled FOD Adjustment\n");
		} else {
			chip->fod_adj_en = false;
			idtp9382a_dbg(chip, PR_MAJOR,
						"Disabled FOD Adjustment\n");
		}
		break;

	case ATTR_SEND_EPT:
		ret = kstrtoint(buf, 10, &val);
		if (ret < 0)
			return ret;
		idtp9382a_dbg(chip, PR_MAJOR, "Send EPT Packet. mode:%d\n",
								val);
		ret = idtp9382a_set_ept(chip, val);
		if (ret < 0)
			return ret;
		break;

	case ATTR_CMD_REG_ADDR:
		ret = kstrtoint(buf, 0, &val);
		if (ret < 0)
			return ret;
		chip->cmd_reg_addr = (u8)val;
		break;

	case ATTR_CMD_REG_DATA:
		ret = kstrtoint(buf, 0, &val);
		if (ret < 0)
			return ret;
		chip->cmd_reg_data = (u8)val;
		ret = idtp9382a_write_cmd_reg_byte(chip, chip->cmd_reg_addr,
							chip->cmd_reg_data);
		if (ret < 0)
			return ret;
		break;

	default:
		break;
	}
	return size;
}

static ssize_t wireless_chg_negotiation_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct idtp9382a *chip = dev_get_drvdata(dev);
	int size;

	size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chip->wireless_chg_negotiation);

	return size;
}
static DEVICE_ATTR_RO(wireless_chg_negotiation);

static struct attribute *wlc_attrs[] = {
	&dev_attr_wireless_chg_negotiation.attr,
	NULL,
};
ATTRIBUTE_GROUPS(wlc);

static int idtp9382a_send_uevent(struct idtp9382a *chip)
{
	int ret;
	char *envp[] = {NULL, NULL};

	if (chip->wireless_chg_negotiation)
		envp[0] = "SWITCH_STATE=1";
	else
		envp[0] = "SWITCH_STATE=0";

	ret = kobject_uevent_env(&chip->vdev->kobj, KOBJ_CHANGE, envp);

	return ret;
}

static int idtp9382a_parse_dt(struct idtp9382a *chip)
{
	struct device_node *node = chip->dev->of_node;
	int rc;

	if (!node) {
		dev_err(chip->dev, "no dts data for idtp9382a\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(node, "somc,ilim-bpp", &chip->bpp_ilim_ma);
	if (rc < 0)
		chip->bpp_ilim_ma = 500;

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

	chip->wlc_en_rsvd = of_get_named_gpio(node,
					"somc,wlc-en-rsvd-gpio", 0);

	chip->wlc_gpio_irq = of_get_named_gpio(node, "somc,wlc-irq-gpio", 0);
	if (!gpio_is_valid(chip->wlc_gpio_irq))
		dev_err(chip->dev, "Can't get wlc-irq-gpio\n");

	chip->disable_halt = of_property_read_bool(node,
					"somc,enable-wlc-during-usbin");

	chip->monitor_for_debug = of_property_read_bool(node,
					"somc,enable-monitor-for-debug");
	return 0;
}

#define IDTP9382A_POWER_OK_VOUT_THR 4300
#define IDTP9382A_POWER_OK_VOUT_CNT 6
#define IDTP9382A_VOUT_CNT_MAX 30
static int idtp9382a_get_status(struct idtp9382a *chip, bool vout_check)
{
	int rc = 0;
	int wlc_irq_status;
	u8 val;
	u8 reg_buf[2];
	int vout = 0;

	wlc_irq_status = gpio_get_value(chip->wlc_gpio_irq);
	if (wlc_irq_status != 0) {
		rc = IDTP9382A_STATUS_PWR_OFF;
		goto pwr_off;
	}

	rc = idtp9382a_i2c_read_byte(chip, IDTP9382A_REG_STATUS_L, &val);
	if (rc < 0) {
		pr_err("Can't read Status reg. %d\n", rc);
		rc = IDTP9382A_STATUS_PWR_OFF;
		goto i2c_err;
	}

	if (!(val & IDTP9382A_REG_STAT_VOUT_BIT)) {
		rc = IDTP9382A_STATUS_PWR_ON_NEGOTIATION;
		goto vout_low;
	}

	if (vout_check && !chip->vout_check_done) {
		rc = idtp9382a_i2c_read_sequential(chip, IDTP9382A_REG_VOUT,
								reg_buf, 2);
		if (rc < 0) {
			pr_err("Can't read Vout reg. %d\n", rc);
			rc = IDTP9382A_STATUS_PWR_OFF;
			goto i2c_err;
		}
		vout = reg_buf[0] | (reg_buf[1] << 8);
		if (vout >= IDTP9382A_POWER_OK_VOUT_THR)
			chip->vout_ok_counter++;

		chip->vout_check_counter++;
		if (chip->vout_ok_counter >= IDTP9382A_POWER_OK_VOUT_CNT ||
			chip->vout_check_counter >= IDTP9382A_VOUT_CNT_MAX) {
			chip->vout_check_done = true;
			idtp9382a_dbg(chip, PR_MAJOR,
						"vout:%dmv vout_ok:%d/%d\n",
						vout,
						chip->vout_ok_counter,
						chip->vout_check_counter);
		} else {
			idtp9382a_dbg(chip, PR_STATUS,
					"vout:%dmv vout_ok:%d/%d\n", vout,
					chip->vout_ok_counter,
					chip->vout_check_counter);
		}
	}
	if (chip->vout_check_done)
		rc = IDTP9382A_STATUS_PWR_POWER_OK;
	else
		rc = IDTP9382A_STATUS_PWR_ON_NEGOTIATION;

i2c_err:
pwr_off:
	if (rc == IDTP9382A_STATUS_PWR_OFF) {
		chip->vout_ok_counter = 0;
		chip->vout_check_counter = 0;
		chip->vout_check_done = false;
	}
vout_low:
	idtp9382a_dbg(chip, PR_STATUS, "status:%d(%s) wlc_irq_status:%d\n",
					rc, str_status[rc], wlc_irq_status);
	return rc;
}

static void idtp9382a_wake_lock(struct idtp9382a *chip, bool en)
{
	if (en && !chip->wake_lock_en) {
		pm_stay_awake(chip->dev);
		idtp9382a_dbg(chip, PR_MISC, "pm_stay_awake\n");
	} else if (!en && chip->wake_lock_en) {
		idtp9382a_dbg(chip, PR_MISC, "pm_relax\n");
		pm_relax(chip->dev);
	}
	chip->wake_lock_en = en;
}

static void idtp9382a_detect_work(struct work_struct *work)
{
	struct idtp9382a *chip = container_of(work,
				struct idtp9382a,
				detect_work.work);
	int rc;
	int interval;
	ktime_t dbg_ktime_start, dbg_ktime_end;
	union power_supply_propval pval = {0, };

	idtp9382a_dbg(chip, PR_WORKER, "worker is started\n");

	mutex_lock(&chip->reg_ctrl_lock);

	dbg_ktime_start = ktime_get_boottime();
	chip->is_worker_active = true;

	/* get status at first */
	chip->status = idtp9382a_get_status(chip, true);
	if (chip->status == IDTP9382A_STATUS_PWR_OFF) {
		chip->mitigation_vout_set_mv = 0;
		chip->op_mode = 0;
		chip->one_time_read_in_pwr_ok_done = false;
		power_supply_changed(chip->wireless_psy);
		idtp9382a_dbg(chip, PR_MAJOR, "skip monitoring due to POFF\n");
		goto reg_done;
	} else if (chip->status == IDTP9382A_STATUS_PWR_ON_NEGOTIATION &&
				chip->old_status == IDTP9382A_STATUS_PWR_OFF) {
		if (!chip->dc_psy) {
			idtp9382a_dbg(chip, PR_PSY,
				"re-call power_supply_get_by_name()\n");
			chip->dc_psy = power_supply_get_by_name("dc");
		}

		if (chip->dc_psy) {
			idtp9382a_dbg(chip, PR_PSY,
					"call power_supply_set_property()\n");
			rc = power_supply_get_property(chip->dc_psy,
				POWER_SUPPLY_PROP_DCIN_EN_STS, &pval);
			if (rc < 0) {
				pr_err("Couldn't get dcin_en_sts rc=%d\n", rc);
				chip->abnormal_negotiation = 0;
			} else {
				chip->abnormal_negotiation = pval.intval;
			}
			pval.intval = 1;
			power_supply_set_property(chip->dc_psy,
				POWER_SUPPLY_PROP_RESET_DCIN_EN, &pval);
		} else {
			idtp9382a_dbg(chip, PR_MAJOR,
					"Could not get dc_power_supply\n");
		}
	}

	/* one time read parameter */
	if (!chip->one_time_read_in_driver_done) {
		rc = idtp9382a_read_regs_oneshot_in_driver(chip);
		if (rc) {
			pr_err("Error in read_regs_oneshot rc=%d\n", rc);
			goto reg_done;
		}
		chip->one_time_read_in_driver_done = true;
	}

	/* one time read parameter after power on */
	if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK &&
		!chip->one_time_read_in_pwr_ok_done) {
		rc = idtp9382a_read_regs_oneshot_in_pwr_ok(chip);
		if (rc) {
			pr_err("Error in read_regs one power_ok rc=%d\n", rc);
			goto reg_done;
		}
		chip->one_time_read_in_pwr_ok_done = true;
	}

	/* always read parameter */
	rc = idtp9382a_read_regs_each_cycle(chip);
	if (rc) {
		pr_err("Error in read_regs_polling rc=%d\n", rc);
		goto reg_done;
	}

	/* always read parameter during debugging */
	rc = idtp9382a_read_regs_debug(chip);
	if (rc) {
		pr_err("Error in read_regs_debug rc=%d\n", rc);
		goto reg_done;
	}

	dbg_ktime_end = ktime_get_boottime();
	idtp9382a_dbg(chip, PR_MEASUREMENT,
			"It took %dms to execute i2c work\n",
			(int)ktime_to_ms(ktime_sub(dbg_ktime_end,
			dbg_ktime_start)));

	if (idtp9382a_debug_dump)
		idtp9382a_dump_reg(chip);

	if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK &&
			chip->old_status != IDTP9382A_STATUS_PWR_POWER_OK) {
		rc = idtp9382a_set_fod_all(chip);
		if (rc)
			pr_err("Error in set_fod_all rc=%d\n", rc);

		if (chip->op_mode == IDTP9382A_OP_MODE_QI_EPP) {
			rc = idtp9382a_set_ilim_set(chip, chip->epp_ilim_ma);
			if (rc)
				pr_err("Error in set_ilim_set rc=%d\n", rc);
		}
		if (chip->op_mode == IDTP9382A_OP_MODE_QI_BPP) {
			rc = idtp9382a_set_ilim_set(chip, chip->bpp_ilim_ma);
			if (rc)
				pr_err("Error in set_ilim_set rc=%d\n", rc);
		}
		power_supply_changed(chip->wireless_psy);
	}

	rc = idtp9382a_handle_vout_set(chip);
reg_done:
	if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK) {
		switch (chip->op_mode) {
		case IDTP9382A_OP_MODE_QI_EPP:
			if (chip->txid == IDTP9382A_TXID_CURRENT_LIMITED_EPP ||
				chip->txid == IDTP9382A_TXID_UNSTABLE_EPP)
				pval.intval =
					IDTP9382A_PSY_WIRELESS_MODE_LIM_EPP;
			else if (chip->txid ==
					IDTP9382A_TXID_AICL_FAST_STEP_EPP)
				pval.intval =
				IDTP9382A_PSY_WIRELESS_MODE_FAST_STEP_EPP;
			else
				pval.intval = IDTP9382A_PSY_WIRELESS_MODE_EPP;
			break;
		case IDTP9382A_OP_MODE_QI_BPP:
			pval.intval = IDTP9382A_PSY_WIRELESS_MODE_BPP;
			break;
		case IDTP9382A_OP_MODE_PMA:
		case IDTP9382A_OP_MODE_UNKNOWN:
		default:
			pval.intval = IDTP9382A_PSY_WIRELESS_MODE_UNKNOWN;
			break;
		}
	} else {
		pval.intval = IDTP9382A_PSY_WIRELESS_MODE_OFF;
	}

	if (pval.intval != chip->psy_wireless_mode) {
		if (!chip->dc_psy) {
			idtp9382a_dbg(chip, PR_PSY,
				"re-call power_supply_get_by_name()\n");
			chip->dc_psy = power_supply_get_by_name("dc");
		}

		if (chip->dc_psy) {
			idtp9382a_dbg(chip, PR_PSY,
					"call power_supply_set_property()\n");
			power_supply_set_property(chip->dc_psy,
				POWER_SUPPLY_PROP_WIRELESS_MODE, &pval);
			chip->psy_wireless_mode = pval.intval;
		} else {
			idtp9382a_dbg(chip, PR_MAJOR,
					"Could not get dc_power_supply\n");
		}
	}

	/* status notification */
	if ((chip->status == IDTP9382A_STATUS_PWR_ON_NEGOTIATION) &&
		(chip->old_status != IDTP9382A_STATUS_PWR_ON_NEGOTIATION)) {
		if (!chip->abnormal_negotiation) {
			chip->wireless_chg_negotiation = 1;
			idtp9382a_send_uevent(chip);
		}
	} else if ((chip->status != IDTP9382A_STATUS_PWR_ON_NEGOTIATION) &&
		(chip->old_status == IDTP9382A_STATUS_PWR_ON_NEGOTIATION)) {
		chip->wireless_chg_negotiation = 0;
		idtp9382a_send_uevent(chip);
	}

	chip->is_worker_active = false;
	if (chip->halt_retry)
		idtp9382a_handle_halt_proc(chip);

	if (!chip->batt_psy) {
		idtp9382a_dbg(chip, PR_PSY,
				"re-call power_supply_get_by_name()\n");
		chip->batt_psy = power_supply_get_by_name("battery");
	}

	if (chip->old_status != chip->status && chip->batt_psy)
		power_supply_changed(chip->batt_psy);

	chip->old_status = chip->status;
	switch (chip->status) {
	case IDTP9382A_STATUS_PWR_ON_NEGOTIATION:
		if (idtp9382a_get_status(chip, false) ==
						IDTP9382A_STATUS_PWR_OFF) {
			idtp9382a_dbg(chip, PR_WORKER,
						"reschedule worker(OFF)\n");
			/*
			 * set interval to 0 to stop worker at next time
			 * if halt or disconnected during monitoring
			 */
			interval = 0;
			power_supply_changed(chip->wireless_psy);
		} else {
			idtp9382a_dbg(chip, PR_WORKER,
						"reschedule worker(NEGO)\n");
			/* speed up monitoring during negotiation */
			interval = chip->interval_negotiation_ms;
		}
		schedule_delayed_work(&chip->detect_work,
						msecs_to_jiffies(interval));
		break;
	case IDTP9382A_STATUS_PWR_POWER_OK:
		if (idtp9382a_get_status(chip, false) ==
						IDTP9382A_STATUS_PWR_OFF) {
			idtp9382a_dbg(chip, PR_WORKER,
						"reschedule worker(OFF)\n");
			/*
			 * set interval to 0 to stop worker at next time
			 * if halt or disconnected during monitoring
			 */
			interval = 0;
			power_supply_changed(chip->wireless_psy);
		} else {
			idtp9382a_dbg(chip, PR_WORKER,
						"reschedule worker(PWON)\n");
			interval = chip->interval_power_ok_ms;
		}
		schedule_delayed_work(&chip->detect_work,
						msecs_to_jiffies(interval));
		break;
	case IDTP9382A_STATUS_PWR_OFF:
	default:
		idtp9382a_dbg(chip, PR_WORKER, "stop worker(POFF)\n");
		idtp9382a_wake_lock(chip, false);
		break;
	}

	mutex_unlock(&chip->reg_ctrl_lock);
}

#define IDTP9382A_REDETECTION_WAIT_MS	2000
static void idtp9382a_wlc_reconnection_enable_work(struct work_struct *work)
{
	struct idtp9382a *chip = container_of(work,
				struct idtp9382a,
				wlc_reconnection_enable_work.work);

	idtp9382a_dbg(chip, PR_MAJOR, "Enable WLC_EN, start reconnection\n");
	idtp9382a_handle_halt(chip, IDTP9382A_ENABLE_CLIENT_RECONNECTION,
									false);
	/*
	 * Wireless charger cannot be re-detected immediately even though
	 * enable controller, so delay indication of completing re-connection
	 * process slightly.
	 */
	msleep(IDTP9382A_REDETECTION_WAIT_MS);
	chip->wlc_reconnection_running = false;
}

#define IDTP9382A_DELAY_WLC_ENABLE_MS	2000
static void idtp9382a_wlc_reconnection_on_boot(struct idtp9382a *chip)
{
	int rc, length;
	u8 start_addr, reg_buf[2];

	start_addr = IDTP9382A_REG_EPP_MANUFACTURER_CODE;
	length = 2;
	rc = idtp9382a_i2c_read_sequential(chip, start_addr, reg_buf, length);
	if (rc < 0) {
		pr_err("Can't read reg %d to %d. %d\n",
				start_addr, start_addr + length - 1, rc);
		return;
	}
	chip->txid = reg_buf[0] | (reg_buf[1] << 8);
	if (chip->txid == IDTP9382A_TXID_UNSTABLE_EPP) {
		idtp9382a_dbg(chip, PR_MAJOR,
			"Unstable EPP on boot was detected, disable WLC_EN to reconnect\n");
		chip->wlc_reconnection_running = true;
		idtp9382a_handle_halt(chip,
				IDTP9382A_ENABLE_CLIENT_RECONNECTION, true);
		schedule_delayed_work(&chip->wlc_reconnection_enable_work,
			msecs_to_jiffies(IDTP9382A_DELAY_WLC_ENABLE_MS));
	}
}

#define STATUS_PWR_OFF			"off"
#define STATUS_PWR_ON_NEGOTIATION	"negotiating"
#define STATUS_PWR_ON			"powered"
#define OP_MODE_QI_EPP_STR		"qi_epp"
#define OP_MODE_QI_BPP_STR		"qi_bpp"
#define OP_MODE_QI_PMA_STR		"pma"
#define OP_MODE_QI_UNKNOWN_STR		"unknown"
#define OP_MODE_QI_NONE_STR		"none"

static const char *idtp9382a_get_wireless_charger_type(struct idtp9382a *chip)
{
	const char *charger_type = NULL;

	if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK) {
		switch (chip->op_mode) {
		case IDTP9382A_OP_MODE_QI_EPP:
			charger_type = OP_MODE_QI_EPP_STR;
			break;
		case IDTP9382A_OP_MODE_QI_BPP:
			charger_type = OP_MODE_QI_BPP_STR;
			break;
		case IDTP9382A_OP_MODE_PMA:
			charger_type = OP_MODE_QI_PMA_STR;
			break;
		case IDTP9382A_OP_MODE_UNKNOWN:
		default:
			charger_type = OP_MODE_QI_UNKNOWN_STR;
			break;
		}
	} else {
		charger_type = OP_MODE_QI_NONE_STR;
	}

	return charger_type;
}

static const char *idtp9382a_get_wireless_status(int status)
{
	const char *wlc_status = NULL;

	switch (status) {
	case IDTP9382A_STATUS_PWR_OFF:
		wlc_status = STATUS_PWR_OFF;
		break;
	case IDTP9382A_STATUS_PWR_ON_NEGOTIATION:
		wlc_status = STATUS_PWR_ON_NEGOTIATION;
		break;
	case IDTP9382A_STATUS_PWR_POWER_OK:
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
	POWER_SUPPLY_PROP_CHARGER_TYPE,
	POWER_SUPPLY_PROP_WIRELESS_STATUS,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED,
	POWER_SUPPLY_PROP_WLC_VOUT_SET,
	POWER_SUPPLY_PROP_WLC_RECONNECTION_RUNNING,
};

static int idtp9382a_psy_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	struct idtp9382a *chip = power_supply_get_drvdata(psy);
	union power_supply_propval pval = {0, };
	int rc;

	switch (psp) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
		break;

	case POWER_SUPPLY_PROP_USBIN_VALID:
		val->intval = chip->usbin_valid;
		break;

	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
		val->intval = chip->wireless_suspend;
		break;
	case POWER_SUPPLY_PROP_CHARGER_TYPE:
		val->strval = idtp9382a_get_wireless_charger_type(chip);
		break;
	case POWER_SUPPLY_PROP_WIRELESS_STATUS:
		val->strval = idtp9382a_get_wireless_status(chip->status);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = chip->wlc_therm_temp_dc;
		if (chip->status == IDTP9382A_STATUS_PWR_OFF)
			return -ENODEV;
		if (!chip->batt_psy) {
			idtp9382a_dbg(chip, PR_PSY,
				"re-call power_supply_get_by_name()\n");
			chip->batt_psy = power_supply_get_by_name("battery");
		}
		if (chip->batt_psy) {
			rc = power_supply_get_property(chip->batt_psy,
						POWER_SUPPLY_PROP_TEMP, &pval);
			if (rc < 0) {
				pr_err("Error in getting battery temp, rc=%d\n",
									rc);
				return rc;
			}
			chip->wlc_therm_temp_dc = pval.intval;
			val->intval = chip->wlc_therm_temp_dc;
		} else {
			pr_err("Could not get battery_psy, rc=%d\n", rc);
			return -ENODEV;
		}
		break;
	case POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED:
		if (chip->status == IDTP9382A_STATUS_PWR_POWER_OK)
			val->intval = true;
		else
			val->intval = false;
		break;
	case POWER_SUPPLY_PROP_WLC_VOUT_SET:
		val->intval = chip->mitigation_vout_set_mv;
		break;
	case POWER_SUPPLY_PROP_WLC_RECONNECTION_RUNNING:
		val->intval = chip->wlc_reconnection_running;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int idtp9382a_psy_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	struct idtp9382a *chip = power_supply_get_drvdata(psy);
	int rc;

	switch (psp) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
		break;

	case POWER_SUPPLY_PROP_USBIN_VALID:
		idtp9382a_dbg(chip, PR_PSY,
				"POWER_SUPPLY_PROP_USBIN_VALID=%d\n",
				val->intval);
		if (val->intval)
			idtp9382a_handle_halt(chip,
					IDTP9382A_ENABLE_CLIENT_USB, true);
		else
			idtp9382a_handle_halt(chip,
					IDTP9382A_ENABLE_CLIENT_USB, false);

		chip->usbin_valid = val->intval;
		break;
	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
		idtp9382a_dbg(chip, PR_PSY,
				"POWER_SUPPLY_PROP_WIRELESS_SUSPEND=%d\n",
				val->intval);
		if (val->intval)
			idtp9382a_handle_halt(chip,
					IDTP9382A_ENABLE_CLIENT_DEV1, true);
		else
			idtp9382a_handle_halt(chip,
					IDTP9382A_ENABLE_CLIENT_DEV1, false);

		chip->wireless_suspend = val->intval;
		break;
	case POWER_SUPPLY_PROP_WLC_VOUT_SET:
		if (chip->status != IDTP9382A_STATUS_PWR_OFF &&
				val->intval != chip->mitigation_vout_set_mv) {
			chip->mitigation_vout_set_mv = val->intval;
			rc = idtp9382a_handle_vout_set(chip);
			if (rc < 0)
				return rc;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int idtp9382a_property_is_writeable(struct power_supply *psy,
					enum power_supply_property prop)
{
	int rc;

	switch (prop) {
	case POWER_SUPPLY_PROP_DCIN_VALID:
	case POWER_SUPPLY_PROP_USBIN_VALID:
	case POWER_SUPPLY_PROP_WIRELESS_SUSPEND_FOR_DEV1:
	case POWER_SUPPLY_PROP_WLC_VOUT_SET:
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
	.get_property = idtp9382a_psy_get_property,
	.set_property = idtp9382a_psy_set_property,
	.property_is_writeable = idtp9382a_property_is_writeable,
};

static int idtp9382a_init_wireless_psy(struct idtp9382a *chip)
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

static irqreturn_t idtp9382a_irq_handler(int irq, void *data)
{
	struct idtp9382a *chip = data;
	int stat;

	idtp9382a_wake_lock(chip, true);

	stat = gpio_get_value(chip->wlc_gpio_irq);
	idtp9382a_dbg(chip, PR_MAJOR, "irq_status=%d\n", stat);

	cancel_delayed_work_sync(&chip->detect_work);
	schedule_delayed_work(&chip->detect_work, msecs_to_jiffies(0));

	return IRQ_HANDLED;
}

static int idtp9382a_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct idtp9382a *chip;
	int rc;
	int i;
	union power_supply_propval prop = {0, };
	bool wlc_en;

	pr_err("idtp9382a_probe(start)\n");

	chip = devm_kzalloc(&client->dev, sizeof(struct idtp9382a),
							GFP_KERNEL);
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
	INIT_DELAYED_WORK(&chip->detect_work, idtp9382a_detect_work);
	INIT_DELAYED_WORK(&chip->vout_change_work, idtp9382a_vout_change_work);
	INIT_DELAYED_WORK(&chip->wlc_reconnection_enable_work,
				idtp9382a_wlc_reconnection_enable_work);

	mutex_init(&chip->reg_ctrl_lock);
	mutex_init(&chip->i2c_lock);
	mutex_init(&chip->vout_set_lock);

	/* get device tree */
	rc = idtp9382a_parse_dt(chip);
	if (rc < 0)
		goto err_parse_dt;

	/* virtual device resistration */
	chip->vcls = class_create(THIS_MODULE, "wlc_switch");
	if (IS_ERR(chip->vcls)) {
		rc = PTR_ERR(chip->vcls);
		pr_err("Could not create class rc=%d\n", rc);
		goto err_class;
	}
	chip->vcls->dev_groups = wlc_groups;

	chip->vdev = device_create(chip->vcls, NULL, MKDEV(0, 0),
					NULL, "wireless_chg");
	if (IS_ERR(chip->vdev)) {
		dev_err(chip->vdev, "%s: Could not create device\n", __func__);
		rc = -ENODEV;
		goto err_device;
	}
	dev_set_drvdata(chip->vdev, chip);

	chip->debug_mask = &idtp9382a_debug_mask;
	chip->interval_power_ok_ms = IDTP9382A_INTERVAL_POWER_OK_DEFAULT_MS;
	chip->interval_negotiation_ms =
				IDTP9382A_INTERVAL_NEGOTIATION_DEFAULT_MS;
	chip->status = IDTP9382A_STATUS_PWR_OFF;
	chip->old_status = IDTP9382A_STATUS_PWR_OFF;
	chip->psy_wireless_mode = IDTP9382A_PSY_WIRELESS_MODE_UNSETTLED;
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
	rc = idtp9382a_init_wireless_psy(chip);
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
		rc = gpio_request(chip->wlc_gpio_irq, "idtp9382a_irq");
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
		idtp9382a_dbg(chip, PR_MAJOR,
					"Keep enabling WLC_EN for debug\n");
		idtp9382a_control_enable(chip, true);
	} else {
		idtp9382a_handle_halt(chip, IDTP9382A_ENABLE_CLIENT_USB,
								!wlc_en);
	}

	idtp9382a_irq_handler(0, chip);

	if (!gpio_get_value(chip->wlc_gpio_irq))
		idtp9382a_wlc_reconnection_on_boot(chip);

	/* irq */
	if (gpio_is_valid(chip->wlc_gpio_irq)) {
		chip->wlc_irq = gpio_to_irq(chip->wlc_gpio_irq);
		if (chip->wlc_irq < 0) {
			rc = chip->wlc_irq;
			dev_err(chip->dev, "can't gpio_to_irq (%d)\n", rc);
			goto err_irq;
		}
		rc = devm_request_threaded_irq(chip->dev, chip->wlc_irq,
				NULL, idtp9382a_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				IRQF_ONESHOT, "idtp9382a", chip);
		if (rc) {
			dev_err(chip->dev,
				"can't devm_request_threaded_irq (%d)\n", rc);
			goto err_irq;
		}

		enable_irq_wake(chip->wlc_irq);
	}

	pr_err("idtp9382a_probe(end)\n");
	return 0;

err_psy:
	for (i = 0; i < ARRAY_SIZE(wireless_attrs); i++)
		device_remove_file(&client->dev, &wireless_attrs[i]);
err_sysfs:
err_irq:
err_device:
	class_destroy(chip->vcls);
err_class:
err_parse_dt:
	i2c_set_clientdata(client, NULL);
	kzfree(chip);
err_kzalloc:
	return rc;
}

static int idtp9382a_remove(struct i2c_client *client)
{

	struct idtp9382a *chip = i2c_get_clientdata(client);
	int i;

	cancel_delayed_work_sync(&chip->detect_work);
	power_supply_unregister(chip->wireless_psy);
	idtp9382a_wake_lock(chip, false);
	device_init_wakeup(chip->dev, 0);

	for (i = 0; i < ARRAY_SIZE(wireless_attrs); i++)
		device_remove_file(&client->dev, &wireless_attrs[i]);

	class_destroy(chip->vcls);
	i2c_set_clientdata(client, NULL);
	kzfree(chip);
	return 0;
}

static const struct of_device_id idtp9382a_dt_ids[] = {
	{ .compatible = "idtp,idtp9382a", },
	{ }
};
MODULE_DEVICE_TABLE(of, idtp9382a_dt_ids);

static const struct i2c_device_id idtp9382a_ids[] = {
	{"idtp9382a", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, idtp9382a_ids);

static struct i2c_driver idtp9382a_driver = {
	.driver = {
		   .name = "idtp9382a",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(idtp9382a_dt_ids),
		   },
	.probe = idtp9382a_probe,
	.remove = idtp9382a_remove,
	.id_table = idtp9382a_ids,
};

module_i2c_driver(idtp9382a_driver);

MODULE_DESCRIPTION("P9382A driver");
MODULE_LICENSE("GPL v2");
