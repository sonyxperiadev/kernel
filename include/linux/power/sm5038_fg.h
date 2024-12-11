/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2021 Sony Corporation,
 * and licensed under the license of the file.
 */
/*
 * sm5038_fg.h
 *
 * This software is sm5038 under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SM5038_FUELGAUGE_H
#define __SM5038_FUELGAUGE_H

#include <linux/power_supply.h>
#include <linux/power/sm5038.h>

#include <linux/regulator/machine.h>

#define PRINT_COUNT	10

#define SW_RESET_CODE			0x00A6
#define SW_RESET_OTP_CODE		0x01A6
#define RS_MAN_CNTL				0x0800

#define FG_INIT_MARK			0xA000
#define FG_WRITE_UNLOCK_CODE 	0x8400
#define FG_WRITE_LOCK_CODE 		0x0000
#define FG_PARAM_UNLOCK_CODE	0x3700
#define FG_PARAM_LOCK_CODE	    0x0000
#define FG_TABLE_LEN			0x17 /* real table length -1 */
#define FG_ADD_TABLE_LEN		0xF /* real table length -1 */
#define FG_INIT_B_LEN		    0x7 /* real table length -1 */


/* control register value */
#define ENABLE_MIX_MODE         0x8200
#define ENABLE_TEMP_MEASURE     0x4000
#define ENABLE_TOPOFF_SOC       0x2000
#define ENABLE_RS_MAN_MODE      0x0800
#define ENABLE_MANUAL_OCV       0x0400
#define ENABLE_MODE_nENQ4       0x0200

#define ENABLE_VL_ALARM         0x0001

#define AUTO_RS_OFF             0x0000
#define AUTO_RS_100             0x0001
#define AUTO_RS_200             0x0002
#define AUTO_RS_300             0x0003

#define SM5038_FG_SOH_TH_MASK   0x0F00

#define CNTL_REG_DEFAULT_VALUE  0x2008
#define INIT_CHECK_MASK         0x1000
#define DISABLE_RE_INIT         0x1000
#define JIG_CONNECTED	0x0001
#define I2C_ERROR_REMAIN		0x0004
#define I2C_ERROR_CHECK	0x0008
#define DATA_VERSION	0x00F0


struct sm5038_fg_info {
	int temp_fg;
	int min_temp_fg;
	int max_temp_fg;
	u16 soc;
	u16 scaled_soc;
	u16 soc_scale_base;
	u16 vbat;
	u16 avg_vbat;
	u16 min_vbat;
	u16 max_vbat;
	u16 batt_ocv;
	u16 q_now;
	int fg_current;
	int avg_current;
	u16 q_max;
	u16 soh;
	u16 batt_soc_cycle;
	int init_complete;
	u16 chg_state;
	u16 last_chg_state;
};

struct sm5038_fg_init_data {
	u16 q_max;
	int battery_table[3][24];
	int rs_value[7];   /*spare min max factor chg_factor dischg_factor manvalue*/
	int i_dp_default;
	int dp_i_off;
	int dp_i_pslo;
	int dp_i_nslo;
	int i_alg_default;
	int alg_i_off;
	int alg_i_pslo;
	int alg_i_nslo;
	int v_default;
	int v_off;
	int v_slo;
	int vt_default;
	int vtt;
	int ivt;
	int ivv;
	int aux_ctrl[2];
	int min_vbat;
	int max_vbat;
	int top_off;
	int min_temp;
	int max_temp;
	int temp_std;
	int data_ver;
};

typedef struct sm5038_fuelgauge_platform_data {
	int fg_irq;
	unsigned long fg_irq_attr;
	/* fuel alert SOC (-1: not use) */
	int fuel_alert_soc;
	/* fuel alert can be repeated */
	bool repeated_fuelalert;
	/* soc should be soc x 10 (0.1% degree)
	 * only for scaling
	 */
	int capacity_max;
	int capacity_max_margin;
	int capacity_min;

} sm5038_fuelgauge_platform_data_t;

struct sm5038_fg_data {
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex fg_lock;
	struct power_supply *psy_fg;
	struct sm5038_platform_data *sm5038_pdata;
	sm5038_fuelgauge_platform_data_t *pdata;
	struct sm5038_fg_info	info;
	struct sm5038_fg_init_data	init_data;
	struct work_struct work;
	struct delayed_work isr_work;
	struct wakeup_source *fuel_alert_ws;
	bool is_fuel_alerted;
	int fg_irq;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* Soft charge */
	int			somc_batt_soc;

	/* Learning */
	int			qage;
	int			learning_counter;
	int			learning_range_max;
	int			learning_range_min;
	int			learned_capacity_raw;

	/* Misc */
	int			current_sense;

	/* debug */
	int			debug_mask;
	u8			dfs_reg_addr;
	bool			regdump_en;
#endif
};

#endif /* __SM5038_FUELGAUGE_H */
