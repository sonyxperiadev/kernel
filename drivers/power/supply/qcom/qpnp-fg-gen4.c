// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
 */

#define pr_fmt(fmt)	"FG: %s: " fmt, __func__

#include <linux/alarmtimer.h>
#include <linux/irq.h>
#include <linux/ktime.h>
#include <linux/nvmem-consumer.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_batterydata.h>
#include <linux/platform_device.h>
#include <linux/iio/consumer.h>
#include <linux/qpnp/qpnp-pbs.h>
#include <linux/qpnp/qpnp-revid.h>
#include <linux/thermal.h>
#include "fg-core.h"
#include "fg-reg.h"
#include "fg-alg.h"

#define FG_GEN4_DEV_NAME	"qcom,fg-gen4"
#define TTF_AWAKE_VOTER		"fg_ttf_awake"

#define PERPH_SUBTYPE_REG		0x05
#define FG_BATT_SOC_PM8150B		0x10
#define FG_BATT_INFO_PM8150B		0x11
#define FG_MEM_IF_PM8150B		0x0D
#define FG_ADC_RR_PM8150B		0x13

#define SDAM_COOKIE_OFFSET		0x80
#define SDAM_CYCLE_COUNT_OFFSET		0x81
#define SDAM_CAP_LEARN_OFFSET		0x91
#define SDAM_COOKIE			0xA5
#define SDAM_FG_PARAM_LENGTH		20

#define FG_SRAM_LEN			972
#define PROFILE_LEN			416
#define PROFILE_COMP_LEN		24
#define KI_COEFF_SOC_LEVELS		3
#define ESR_CAL_LEVELS			2
#define KI_COEFF_MAX			15564
#define SLOPE_LIMIT_NUM_COEFFS		4
#define SLOPE_LIMIT_COEFF_MAX		31128
#define BATT_THERM_NUM_COEFFS		5
#define RSLOW_NUM_COEFFS		4

/* SRAM address/offset definitions in ascending order */
#define BATT_THERM_CONFIG_WORD		3
#define RATIO_CENTER_OFFSET		1
#define BATT_THERM_COEFF_WORD		4
#define BATT_THERM_COEFF_OFFSET		0
#define BATT_TEMP_CONFIG_WORD		9
#define BATT_TEMP_HOT_OFFSET		0
#define BATT_TEMP_COLD_OFFSET		1
#define BATT_TEMP_CONFIG2_WORD		10
#define BATT_TEMP_HYST_DELTA_OFFSET	0
#define ESR_CAL_SOC_MIN_OFFSET		1
#define ESR_CAL_THRESH_WORD		11
#define ESR_CAL_SOC_MAX_OFFSET		0
#define ESR_CAL_TEMP_MIN_OFFSET		1
#define ESR_PULSE_THRESH_WORD		12
#define ESR_CAL_TEMP_MAX_OFFSET		0
#define ESR_PULSE_THRESH_OFFSET		1
#define DELTA_ESR_THR_WORD		14
#define DELTA_ESR_THR_OFFSET		0
#define ESR_TIMER_DISCHG_MAX_WORD	17
#define ESR_TIMER_DISCHG_MAX_OFFSET	0
#define ESR_TIMER_DISCHG_INIT_WORD	17
#define ESR_TIMER_DISCHG_INIT_OFFSET	1
#define ESR_TIMER_CHG_MAX_WORD		18
#define ESR_TIMER_CHG_MAX_OFFSET	0
#define ESR_TIMER_CHG_INIT_WORD		18
#define ESR_TIMER_CHG_INIT_OFFSET	1
#define CUTOFF_CURR_WORD		19
#define CUTOFF_CURR_OFFSET		0
#define CUTOFF_VOLT_WORD		20
#define CUTOFF_VOLT_OFFSET		0
#define KI_COEFF_CUTOFF_WORD		21
#define KI_COEFF_CUTOFF_OFFSET		0
#define SYS_TERM_CURR_WORD		22
#define SYS_TERM_CURR_OFFSET		0
#define VBATT_FULL_WORD			23
#define VBATT_FULL_OFFSET		0
#define KI_COEFF_FULL_SOC_NORM_WORD	24
#define KI_COEFF_FULL_SOC_NORM_OFFSET	1
#define KI_COEFF_LOW_DISCHG_WORD	25
#define KI_COEFF_FULL_SOC_LOW_OFFSET	0
#define KI_COEFF_LOW_DISCHG_OFFSET	1
#define KI_COEFF_MED_DISCHG_WORD	26
#define KI_COEFF_MED_DISCHG_OFFSET	0
#define KI_COEFF_HI_DISCHG_WORD		26
#define KI_COEFF_HI_DISCHG_OFFSET	1
#define KI_COEFF_LO_MED_DCHG_THR_WORD	27
#define KI_COEFF_LO_MED_DCHG_THR_OFFSET	0
#define KI_COEFF_MED_HI_DCHG_THR_WORD	27
#define KI_COEFF_MED_HI_DCHG_THR_OFFSET	1
#define KI_COEFF_LOW_CHG_WORD		28
#define KI_COEFF_LOW_CHG_OFFSET		0
#define KI_COEFF_MED_CHG_WORD		28
#define KI_COEFF_MED_CHG_OFFSET		1
#define KI_COEFF_HI_CHG_WORD		29
#define KI_COEFF_HI_CHG_OFFSET		0
#define KI_COEFF_LO_MED_CHG_THR_WORD	29
#define KI_COEFF_LO_MED_CHG_THR_OFFSET	1
#define KI_COEFF_MED_HI_CHG_THR_WORD	30
#define KI_COEFF_MED_HI_CHG_THR_OFFSET	0
#define DELTA_BSOC_THR_WORD		30
#define DELTA_BSOC_THR_OFFSET		1
#define SLOPE_LIMIT_WORD		32
#define SLOPE_LIMIT_OFFSET		0
#define DELTA_MSOC_THR_WORD		32
#define DELTA_MSOC_THR_OFFSET		1
#define VBATT_LOW_WORD			35
#define VBATT_LOW_OFFSET		1
#define SYS_CONFIG_WORD			60
#define SYS_CONFIG_OFFSET		0
#define SYS_CONFIG2_OFFSET		1
#define PROFILE_LOAD_WORD		65
#define PROFILE_LOAD_OFFSET		0
#define RSLOW_COEFF_DISCHG_WORD		78
#define RSLOW_COEFF_LOW_OFFSET		0
#define RSLOW_CONFIG_WORD		241
#define RSLOW_CONFIG_OFFSET		0
#define NOM_CAP_WORD			271
#define NOM_CAP_OFFSET			0
#define RCONN_WORD			275
#define RCONN_OFFSET			0
#define ACT_BATT_CAP_WORD		285
#define ACT_BATT_CAP_OFFSET		0
#define BATT_AGE_LEVEL_WORD		288
#define BATT_AGE_LEVEL_OFFSET		0
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define CYCLE_COUNT_WORD		291
#define CYCLE_COUNT_OFFSET		0
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
/*
 * The qualcomm cycle count function is disabled.
 * SRAM address 0d291~0d298 for its function is used by SOMC functions.
 */
#define SOMC_AGING_LEVEL_WORD		291
#define SOMC_AGING_LEVEL_OFFSET		0
#endif
#define PROFILE_INTEGRITY_WORD		299
#define PROFILE_INTEGRITY_OFFSET	0
#define IBAT_FINAL_WORD			320
#define IBAT_FINAL_OFFSET		0
#define VBAT_FINAL_WORD			321
#define VBAT_FINAL_OFFSET		0
#define BATT_TEMP_WORD			328
#define BATT_TEMP_OFFSET		0
#define ESR_WORD			331
#define ESR_OFFSET			0
#define ESR_MDL_WORD			335
#define ESR_MDL_OFFSET			0
#define ESR_CHAR_WORD			336
#define ESR_CHAR_OFFSET			0
#define ESR_DELTA_DISCHG_WORD		340
#define ESR_DELTA_DISCHG_OFFSET		0
#define ESR_DELTA_CHG_WORD		341
#define ESR_DELTA_CHG_OFFSET		0
#define ESR_ACT_WORD			342
#define ESR_ACT_OFFSET			0
#define RSLOW_WORD			368
#define RSLOW_OFFSET			0
#define OCV_WORD			417
#define OCV_OFFSET			0
#define VOLTAGE_PRED_WORD		432
#define VOLTAGE_PRED_OFFSET		0
#define BATT_SOC_WORD			449
#define BATT_SOC_OFFSET			0
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define CUTOFF_SOC_WORD			453
#define CUTOFF_SOC_OFFSET		0
#endif
#define FULL_SOC_WORD			455
#define FULL_SOC_OFFSET			0
#define CC_SOC_SW_WORD			458
#define CC_SOC_SW_OFFSET		0
#define CC_SOC_WORD			460
#define CC_SOC_OFFSET			0
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define SYSTEM_SOC_WORD			462
#define SYSTEM_SOC_OFFSET		0
#endif
#define MONOTONIC_SOC_WORD		463
#define MONOTONIC_SOC_OFFSET		0

/* v2 SRAM address and offset in ascending order */
#define LOW_PASS_VBATT_WORD		3
#define LOW_PASS_VBATT_OFFSET		0
#define RSLOW_SCALE_FN_DISCHG_V2_WORD	281
#define RSLOW_SCALE_FN_DISCHG_V2_OFFSET	0
#define RSLOW_SCALE_FN_CHG_V2_WORD	285
#define RSLOW_SCALE_FN_CHG_V2_OFFSET	0
#define ACT_BATT_CAP_v2_WORD		287
#define ACT_BATT_CAP_v2_OFFSET		0
#define IBAT_FLT_WORD			322
#define IBAT_FLT_OFFSET			0
#define VBAT_FLT_WORD			326
#define VBAT_FLT_OFFSET			0
#define RSLOW_v2_WORD			371
#define RSLOW_v2_OFFSET			0
#define OCV_v2_WORD			425
#define OCV_v2_OFFSET			0
#define VOLTAGE_PRED_v2_WORD		440
#define VOLTAGE_PRED_v2_OFFSET		0
#define BATT_SOC_v2_WORD		455
#define BATT_SOC_v2_OFFSET		0
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define CUTOFF_SOC_v2_WORD		459
#define CUTOFF_SOC_v2_OFFSET		0
#endif
#define FULL_SOC_v2_WORD		461
#define FULL_SOC_v2_OFFSET		0
#define CC_SOC_SW_v2_WORD		464
#define CC_SOC_SW_v2_OFFSET		0
#define CC_SOC_v2_WORD			466
#define CC_SOC_v2_OFFSET		0
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define SYSTEM_SOC_v2_WORD		468
#define SYSTEM_SOC_v2_OFFSET		0
#endif
#define MONOTONIC_SOC_v2_WORD		469
#define MONOTONIC_SOC_v2_OFFSET		0
#define FIRST_LOG_CURRENT_v2_WORD	471
#define FIRST_LOG_CURRENT_v2_OFFSET	0

static struct fg_irq_info fg_irqs[FG_GEN4_IRQ_MAX];

/* DT parameters for FG device */
struct fg_dt_props {
	bool	force_load_profile;
	bool	hold_soc_while_full;
	bool	linearize_soc;
	bool	rapid_soc_dec_en;
	bool	five_pin_battery;
	bool	multi_profile_load;
	bool	esr_calib_dischg;
	bool	soc_hi_res;
	bool	soc_scale_mode;
	int	cutoff_volt_mv;
	int	empty_volt_mv;
	int	sys_min_volt_mv;
	int	cutoff_curr_ma;
	int	sys_term_curr_ma;
	int	delta_soc_thr;
	int	vbatt_scale_thr_mv;
	int	scale_timer_ms;
	int	force_calib_level;
	int	esr_timer_chg_fast[NUM_ESR_TIMERS];
	int	esr_timer_chg_slow[NUM_ESR_TIMERS];
	int	esr_timer_dischg_fast[NUM_ESR_TIMERS];
	int	esr_timer_dischg_slow[NUM_ESR_TIMERS];
	u32	esr_cal_soc_thresh[ESR_CAL_LEVELS];
	int	esr_cal_temp_thresh[ESR_CAL_LEVELS];
	int	esr_filter_factor;
	int	delta_esr_disable_count;
	int	delta_esr_thr_uohms;
	int	rconn_uohms;
	int	batt_id_pullup_kohms;
	int	batt_temp_cold_thresh;
	int	batt_temp_hot_thresh;
	int	batt_temp_hyst;
	int	batt_temp_delta;
	u32	batt_therm_freq;
	int	esr_pulse_thresh_ma;
	int	esr_meas_curr_ma;
	int	slope_limit_temp;
	int	ki_coeff_low_chg;
	int	ki_coeff_med_chg;
	int	ki_coeff_hi_chg;
	int	ki_coeff_lo_med_chg_thr_ma;
	int	ki_coeff_med_hi_chg_thr_ma;
	int	ki_coeff_cutoff_gain;
	int	ki_coeff_full_soc_dischg[2];
	int	ki_coeff_soc[KI_COEFF_SOC_LEVELS];
	int	ki_coeff_low_dischg[KI_COEFF_SOC_LEVELS];
	int	ki_coeff_med_dischg[KI_COEFF_SOC_LEVELS];
	int	ki_coeff_hi_dischg[KI_COEFF_SOC_LEVELS];
	int	ki_coeff_lo_med_dchg_thr_ma;
	int	ki_coeff_med_hi_dchg_thr_ma;
	int	slope_limit_coeffs[SLOPE_LIMIT_NUM_COEFFS];
};

struct fg_gen4_chip {
	struct fg_dev		fg;
	struct fg_dt_props	dt;
	struct iio_channel	*batt_id_chan;
	struct cycle_counter	*counter;
	struct cap_learning	*cl;
	struct ttf		*ttf;
	struct soh_profile	*sp;
	struct device_node	*pbs_dev;
	struct nvmem_device	*fg_nvmem;
	struct votable		*delta_esr_irq_en_votable;
	struct votable		*pl_disable_votable;
	struct votable		*cp_disable_votable;
	struct votable		*parallel_current_en_votable;
	struct votable		*mem_attn_irq_en_votable;
	struct votable		*fv_votable;
	struct work_struct	esr_calib_work;
	struct work_struct	soc_scale_work;
	struct alarm		esr_fast_cal_timer;
	struct alarm		soc_scale_alarm_timer;
	struct delayed_work	pl_enable_work;
	struct work_struct	pl_current_en_work;
	struct completion	mem_attn;
	struct mutex		soc_scale_lock;
	struct mutex		esr_calib_lock;
	ktime_t			last_restart_time;
	char			batt_profile[PROFILE_LEN];
	enum slope_limit_status	slope_limit_sts;
	int			ki_coeff_full_soc[2];
	int			delta_esr_count;
	int			recharge_soc_thr;
	int			esr_actual;
	int			esr_nominal;
	int			soh;
	int			esr_soh_cycle_count;
	int			batt_age_level;
	int			last_batt_age_level;
	int			soc_scale_msoc;
	int			prev_soc_scale_msoc;
	int			soc_scale_slope;
	int			msoc_actual;
	int			vbatt_avg;
	int			vbatt_now;
	int			vbatt_res;
	int			scale_timer;
	int			current_now;
	int			calib_level;
	bool			first_profile_load;
	bool			ki_coeff_dischg_en;
	bool			slope_limit_en;
	bool			esr_fast_calib;
	bool			esr_fast_calib_done;
	bool			esr_fast_cal_timer_expired;
	bool			esr_fast_calib_retry;
	bool			esr_fcc_ctrl_en;
	bool			esr_soh_notified;
	bool			rslow_low;
	bool			rapid_soc_dec_en;
	bool			vbatt_low;
	bool			chg_term_good;
	bool			soc_scale_mode;
};

struct bias_config {
	u8	status_reg;
	u8	lsb_reg;
	int	bias_kohms;
};

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_debug_mask;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_debug_mask = FG_SOMC;
#endif

static bool fg_profile_dump;
static ssize_t profile_dump_show(struct device *dev, struct device_attribute
		*attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%c\n", fg_profile_dump ? 'Y' : 'N');
}

static ssize_t profile_dump_store(struct device *dev, struct device_attribute
		*attr, const char *buf, size_t count)
{
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	fg_profile_dump = val;

	return count;
}
static DEVICE_ATTR_RW(profile_dump);

static int fg_sram_dump_period_ms = 20000;
static ssize_t sram_dump_period_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", fg_sram_dump_period_ms);
}

static ssize_t sram_dump_period_ms_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int val;

	if (kstrtos32(buf, 0, &val))
		return -EINVAL;

	fg_sram_dump_period_ms = val;

	return count;
}
static DEVICE_ATTR_RW(sram_dump_period_ms);

static int fg_restart_mp;
static bool fg_sram_dump;
static bool fg_esr_fast_cal_en;

static int fg_gen4_validate_soc_scale_mode(struct fg_gen4_chip *chip);
static int fg_gen4_esr_fast_calib_config(struct fg_gen4_chip *chip, bool en);

static struct fg_sram_param pm8150b_v1_sram_params[] = {
	PARAM(BATT_SOC, BATT_SOC_WORD, BATT_SOC_OFFSET, 4, 1, 1, 0, NULL,
		fg_decode_default),
	PARAM(FULL_SOC, FULL_SOC_WORD, FULL_SOC_OFFSET, 2, 1, 1, 0,
		fg_encode_default, fg_decode_default),
	PARAM(MONOTONIC_SOC, MONOTONIC_SOC_WORD, MONOTONIC_SOC_OFFSET, 2, 1, 1,
		0, NULL, fg_decode_default),
	PARAM(VOLTAGE_PRED, VOLTAGE_PRED_WORD, VOLTAGE_PRED_OFFSET, 2, 1000,
		244141, 0, NULL, fg_decode_voltage_15b),
	PARAM(OCV, OCV_WORD, OCV_OFFSET, 2, 1000, 244141, 0, NULL,
		fg_decode_voltage_15b),
	PARAM(VBAT_FINAL, VBAT_FINAL_WORD, VBAT_FINAL_OFFSET, 2, 1000, 244141,
		0, NULL, fg_decode_voltage_15b),
	PARAM(IBAT_FINAL, IBAT_FINAL_WORD, IBAT_FINAL_OFFSET, 2, 1000, 488282,
		0, NULL, fg_decode_current_16b),
	PARAM(RCONN, RCONN_WORD, RCONN_OFFSET, 2, 1000, 122070, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(ESR, ESR_WORD, ESR_OFFSET, 2, 1000, 244141, 0, fg_encode_default,
		fg_decode_value_16b),
	PARAM(ESR_MDL, ESR_MDL_WORD, ESR_MDL_OFFSET, 2, 1000, 244141, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(ESR_ACT, ESR_ACT_WORD, ESR_ACT_OFFSET, 2, 1000, 244141, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(RSLOW, RSLOW_WORD, RSLOW_OFFSET, 2, 1000, 244141, 0, NULL,
		fg_decode_value_16b),
	PARAM(CC_SOC, CC_SOC_WORD, CC_SOC_OFFSET, 4, 1, 1, 0, NULL,
		fg_decode_cc_soc),
	PARAM(CC_SOC_SW, CC_SOC_SW_WORD, CC_SOC_SW_OFFSET, 4, 1, 1, 0, NULL,
		fg_decode_cc_soc),
	PARAM(ACT_BATT_CAP, ACT_BATT_CAP_WORD, ACT_BATT_CAP_OFFSET, 2,
		1, 1, 0, NULL, fg_decode_default),
	/* Entries below here are configurable during initialization */
	PARAM(CUTOFF_VOLT, CUTOFF_VOLT_WORD, CUTOFF_VOLT_OFFSET, 2, 1000000,
		244141, 0, fg_encode_voltage, NULL),
	PARAM(VBATT_LOW, VBATT_LOW_WORD, VBATT_LOW_OFFSET, 1, 1000,
		15625, -2000, fg_encode_voltage, NULL),
	PARAM(VBATT_FULL, VBATT_FULL_WORD, VBATT_FULL_OFFSET, 2, 1000,
		244141, 0, fg_encode_voltage, fg_decode_voltage_15b),
	PARAM(CUTOFF_CURR, CUTOFF_CURR_WORD, CUTOFF_CURR_OFFSET, 2,
		100000, 48828, 0, fg_encode_current, NULL),
	PARAM(SYS_TERM_CURR, SYS_TERM_CURR_WORD, SYS_TERM_CURR_OFFSET, 2,
		100000, 48828, 0, fg_encode_current, NULL),
	PARAM(DELTA_MSOC_THR, DELTA_MSOC_THR_WORD, DELTA_MSOC_THR_OFFSET,
		1, 2048, 1000, 0, fg_encode_default, NULL),
	PARAM(DELTA_BSOC_THR, DELTA_BSOC_THR_WORD, DELTA_BSOC_THR_OFFSET,
		1, 2048, 1000, 0, fg_encode_default, NULL),
	PARAM(ESR_TIMER_DISCHG_MAX, ESR_TIMER_DISCHG_MAX_WORD,
		ESR_TIMER_DISCHG_MAX_OFFSET, 1, 1, 1, 0, fg_encode_default,
		NULL),
	PARAM(ESR_TIMER_DISCHG_INIT, ESR_TIMER_DISCHG_INIT_WORD,
		ESR_TIMER_DISCHG_INIT_OFFSET, 1, 1, 1, 0, fg_encode_default,
		NULL),
	PARAM(ESR_TIMER_CHG_MAX, ESR_TIMER_CHG_MAX_WORD,
		ESR_TIMER_CHG_MAX_OFFSET, 1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_TIMER_CHG_INIT, ESR_TIMER_CHG_INIT_WORD,
		ESR_TIMER_CHG_INIT_OFFSET, 1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_PULSE_THRESH, ESR_PULSE_THRESH_WORD, ESR_PULSE_THRESH_OFFSET,
		1, 1000, 15625, 0, fg_encode_default, NULL),
	PARAM(DELTA_ESR_THR, DELTA_ESR_THR_WORD, DELTA_ESR_THR_OFFSET, 2, 1000,
		61036, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_CUTOFF, KI_COEFF_CUTOFF_WORD, KI_COEFF_CUTOFF_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_FULL_SOC, KI_COEFF_FULL_SOC_NORM_WORD,
		KI_COEFF_FULL_SOC_NORM_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_LOW_DISCHG, KI_COEFF_LOW_DISCHG_WORD,
		KI_COEFF_LOW_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_DISCHG, KI_COEFF_MED_DISCHG_WORD,
		KI_COEFF_MED_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_HI_DISCHG, KI_COEFF_HI_DISCHG_WORD,
		KI_COEFF_HI_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_LOW_CHG, KI_COEFF_LOW_CHG_WORD, KI_COEFF_LOW_CHG_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_CHG, KI_COEFF_MED_CHG_WORD, KI_COEFF_MED_CHG_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_HI_CHG, KI_COEFF_HI_CHG_WORD, KI_COEFF_HI_CHG_OFFSET, 1,
		1000, 61035, 0, fg_encode_default, NULL),
	PARAM(SLOPE_LIMIT, SLOPE_LIMIT_WORD, SLOPE_LIMIT_OFFSET, 1, 8192,
		1000000, 0, fg_encode_default, NULL),
	PARAM(BATT_TEMP_COLD, BATT_TEMP_CONFIG_WORD, BATT_TEMP_COLD_OFFSET, 1,
		1, 1, 0, fg_encode_default, NULL),
	PARAM(BATT_TEMP_HOT, BATT_TEMP_CONFIG_WORD, BATT_TEMP_HOT_OFFSET, 1,
		1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_SOC_MIN, BATT_TEMP_CONFIG2_WORD, ESR_CAL_SOC_MIN_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_SOC_MAX, ESR_CAL_THRESH_WORD, ESR_CAL_SOC_MAX_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_TEMP_MIN, ESR_CAL_THRESH_WORD, ESR_CAL_TEMP_MIN_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_TEMP_MAX, ESR_PULSE_THRESH_WORD, ESR_CAL_TEMP_MAX_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	PARAM(CUTOFF_SOC, CUTOFF_SOC_WORD, CUTOFF_SOC_OFFSET, 2, 1, 1,
		0, NULL, fg_decode_default),
	PARAM(SYSTEM_SOC, SYSTEM_SOC_WORD, SYSTEM_SOC_OFFSET, 2, 1, 1,
		0, NULL, fg_decode_default),
#endif
};

static struct fg_sram_param pm8150b_v2_sram_params[] = {
	PARAM(VBAT_TAU, LOW_PASS_VBATT_WORD, LOW_PASS_VBATT_OFFSET, 1, 1, 1, 0,
		NULL, NULL),
	PARAM(BATT_SOC, BATT_SOC_v2_WORD, BATT_SOC_v2_OFFSET, 4, 1, 1, 0, NULL,
		fg_decode_default),
	PARAM(FULL_SOC, FULL_SOC_v2_WORD, FULL_SOC_v2_OFFSET, 2, 1, 1, 0,
		fg_encode_default, fg_decode_default),
	PARAM(MONOTONIC_SOC, MONOTONIC_SOC_v2_WORD, MONOTONIC_SOC_v2_OFFSET, 2,
		1, 1, 0, NULL, fg_decode_default),
	PARAM(VOLTAGE_PRED, VOLTAGE_PRED_v2_WORD, VOLTAGE_PRED_v2_OFFSET, 2,
		1000, 244141, 0, NULL, fg_decode_voltage_15b),
	PARAM(OCV, OCV_v2_WORD, OCV_v2_OFFSET, 2, 1000, 244141, 0, NULL,
		fg_decode_voltage_15b),
	PARAM(VBAT_FLT, VBAT_FLT_WORD, VBAT_FLT_OFFSET, 4, 10000, 19073, 0,
		NULL, fg_decode_voltage_24b),
	PARAM(VBAT_FINAL, VBAT_FINAL_WORD, VBAT_FINAL_OFFSET, 2, 1000, 244141,
		0, NULL, fg_decode_voltage_15b),
	PARAM(IBAT_FINAL, IBAT_FINAL_WORD, IBAT_FINAL_OFFSET, 2, 1000, 488282,
		0, NULL, fg_decode_current_16b),
	PARAM(IBAT_FLT, IBAT_FLT_WORD, IBAT_FLT_OFFSET, 4, 10000, 19073, 0,
		NULL, fg_decode_current_24b),
	PARAM(RCONN, RCONN_WORD, RCONN_OFFSET, 2, 1000, 122070, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(ESR, ESR_WORD, ESR_OFFSET, 2, 1000, 244141, 0, fg_encode_default,
		fg_decode_value_16b),
	PARAM(ESR_MDL, ESR_MDL_WORD, ESR_MDL_OFFSET, 2, 1000, 244141, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(ESR_ACT, ESR_ACT_WORD, ESR_ACT_OFFSET, 2, 1000, 244141, 0,
		fg_encode_default, fg_decode_value_16b),
	PARAM(RSLOW, RSLOW_v2_WORD, RSLOW_v2_OFFSET, 2, 1000, 244141, 0, NULL,
		fg_decode_value_16b),
	PARAM(CC_SOC, CC_SOC_v2_WORD, CC_SOC_v2_OFFSET, 4, 1, 1, 0, NULL,
		fg_decode_cc_soc),
	PARAM(CC_SOC_SW, CC_SOC_SW_v2_WORD, CC_SOC_SW_v2_OFFSET, 4, 1, 1, 0,
		NULL, fg_decode_cc_soc),
	PARAM(ACT_BATT_CAP, ACT_BATT_CAP_v2_WORD, ACT_BATT_CAP_v2_OFFSET, 2,
		1, 1, 0, NULL, fg_decode_default),
	/* Entries below here are configurable during initialization */
	PARAM(CUTOFF_VOLT, CUTOFF_VOLT_WORD, CUTOFF_VOLT_OFFSET, 2, 1000000,
		244141, 0, fg_encode_voltage, NULL),
	PARAM(VBATT_LOW, VBATT_LOW_WORD, VBATT_LOW_OFFSET, 1, 1000,
		15625, -2000, fg_encode_voltage, NULL),
	PARAM(VBATT_FULL, VBATT_FULL_WORD, VBATT_FULL_OFFSET, 2, 1000,
		244141, 0, fg_encode_voltage, fg_decode_voltage_15b),
	PARAM(CUTOFF_CURR, CUTOFF_CURR_WORD, CUTOFF_CURR_OFFSET, 2,
		100000, 48828, 0, fg_encode_current, NULL),
	PARAM(SYS_TERM_CURR, SYS_TERM_CURR_WORD, SYS_TERM_CURR_OFFSET, 2,
		100000, 48828, 0, fg_encode_current, NULL),
	PARAM(DELTA_MSOC_THR, DELTA_MSOC_THR_WORD, DELTA_MSOC_THR_OFFSET,
		1, 2048, 1000, 0, fg_encode_default, NULL),
	PARAM(DELTA_BSOC_THR, DELTA_BSOC_THR_WORD, DELTA_BSOC_THR_OFFSET,
		1, 2048, 1000, 0, fg_encode_default, NULL),
	PARAM(ESR_TIMER_DISCHG_MAX, ESR_TIMER_DISCHG_MAX_WORD,
		ESR_TIMER_DISCHG_MAX_OFFSET, 1, 1, 1, 0, fg_encode_default,
		NULL),
	PARAM(ESR_TIMER_DISCHG_INIT, ESR_TIMER_DISCHG_INIT_WORD,
		ESR_TIMER_DISCHG_INIT_OFFSET, 1, 1, 1, 0, fg_encode_default,
		NULL),
	PARAM(ESR_TIMER_CHG_MAX, ESR_TIMER_CHG_MAX_WORD,
		ESR_TIMER_CHG_MAX_OFFSET, 1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_TIMER_CHG_INIT, ESR_TIMER_CHG_INIT_WORD,
		ESR_TIMER_CHG_INIT_OFFSET, 1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_PULSE_THRESH, ESR_PULSE_THRESH_WORD, ESR_PULSE_THRESH_OFFSET,
		1, 1000, 15625, 0, fg_encode_default, NULL),
	PARAM(DELTA_ESR_THR, DELTA_ESR_THR_WORD, DELTA_ESR_THR_OFFSET, 2, 1000,
		61036, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_CUTOFF, KI_COEFF_CUTOFF_WORD, KI_COEFF_CUTOFF_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_FULL_SOC, KI_COEFF_FULL_SOC_NORM_WORD,
		KI_COEFF_FULL_SOC_NORM_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_LOW_DISCHG, KI_COEFF_LOW_DISCHG_WORD,
		KI_COEFF_LOW_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_DISCHG, KI_COEFF_MED_DISCHG_WORD,
		KI_COEFF_MED_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_HI_DISCHG, KI_COEFF_HI_DISCHG_WORD,
		KI_COEFF_HI_DISCHG_OFFSET, 1, 1000, 61035, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_LO_MED_DCHG_THR, KI_COEFF_LO_MED_DCHG_THR_WORD,
		KI_COEFF_LO_MED_DCHG_THR_OFFSET, 1, 1000, 15625, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_HI_DCHG_THR, KI_COEFF_MED_HI_DCHG_THR_WORD,
		KI_COEFF_MED_HI_DCHG_THR_OFFSET, 1, 1000, 15625, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_LOW_CHG, KI_COEFF_LOW_CHG_WORD, KI_COEFF_LOW_CHG_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_CHG, KI_COEFF_MED_CHG_WORD, KI_COEFF_MED_CHG_OFFSET,
		1, 1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_HI_CHG, KI_COEFF_HI_CHG_WORD, KI_COEFF_HI_CHG_OFFSET, 1,
		1000, 61035, 0, fg_encode_default, NULL),
	PARAM(KI_COEFF_LO_MED_CHG_THR, KI_COEFF_LO_MED_CHG_THR_WORD,
		KI_COEFF_LO_MED_CHG_THR_OFFSET, 1, 1000, 15625, 0,
		fg_encode_default, NULL),
	PARAM(KI_COEFF_MED_HI_CHG_THR, KI_COEFF_MED_HI_CHG_THR_WORD,
		KI_COEFF_MED_HI_CHG_THR_OFFSET, 1, 1000, 15625, 0,
		fg_encode_default, NULL),
	PARAM(SLOPE_LIMIT, SLOPE_LIMIT_WORD, SLOPE_LIMIT_OFFSET, 1, 8192,
		1000000, 0, fg_encode_default, NULL),
	PARAM(BATT_TEMP_COLD, BATT_TEMP_CONFIG_WORD, BATT_TEMP_COLD_OFFSET, 1,
		1, 1, 0, fg_encode_default, NULL),
	PARAM(BATT_TEMP_HOT, BATT_TEMP_CONFIG_WORD, BATT_TEMP_HOT_OFFSET, 1,
		1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_SOC_MIN, BATT_TEMP_CONFIG2_WORD, ESR_CAL_SOC_MIN_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_SOC_MAX, ESR_CAL_THRESH_WORD, ESR_CAL_SOC_MAX_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_TEMP_MIN, ESR_CAL_THRESH_WORD, ESR_CAL_TEMP_MIN_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
	PARAM(ESR_CAL_TEMP_MAX, ESR_PULSE_THRESH_WORD, ESR_CAL_TEMP_MAX_OFFSET,
		1, 1, 1, 0, fg_encode_default, NULL),
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	PARAM(CUTOFF_SOC, CUTOFF_SOC_v2_WORD, CUTOFF_SOC_v2_OFFSET, 2, 1, 1,
		0, NULL, fg_decode_default),
	PARAM(SYSTEM_SOC, SYSTEM_SOC_v2_WORD, SYSTEM_SOC_v2_OFFSET, 2, 1, 1,
		0, NULL, fg_decode_default),
#endif
};

/* All get functions below */

struct bias_config id_table[3] = {
	{0x65, 0x66, 400},
	{0x6D, 0x6E, 100},
	{0x75, 0x76, 30},
};

#define BID_VREF_MV	1875
static int fg_get_batt_id_adc(struct fg_gen4_chip *chip, u32 *batt_id_ohms)
{
	int rc, batt_id_mv;
	int64_t denom;

	rc = iio_read_channel_processed(chip->batt_id_chan, &batt_id_mv);
	if (rc < 0) {
		pr_err("Error in reading batt_id channel, rc=%d\n", rc);
		return rc;
	}

	batt_id_mv = div_s64(batt_id_mv, 1000);
	if (batt_id_mv == 0) {
		pr_debug("batt_id_mv = 0 from ADC\n");
		return 0;
	}

	denom = div64_s64(BID_VREF_MV * 1000, batt_id_mv) - 1000;
	if (denom <= 0) {
		/* batt id connector might be open, return 0 kohms */
		return 0;
	}

	*batt_id_ohms = div64_u64(chip->dt.batt_id_pullup_kohms * 1000 * 1000
					+ denom / 2, denom);

	pr_debug("batt_id_mv=%d, batt_id_ohms=%d\n", batt_id_mv, *batt_id_ohms);

	return 0;
}

#define MAX_BIAS_CODE	0x70E4
static int fg_gen4_get_batt_id(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int i, rc, batt_id_kohms;
	u16 tmp = 0, bias_code = 0, delta = 0;
	u8 val, bias_id = 0;

	if (chip->batt_id_chan)
		return fg_get_batt_id_adc(chip, &fg->batt_id_ohms);

	for (i = 0; i < ARRAY_SIZE(id_table); i++)  {
		rc = fg_read(fg, fg->rradc_base + id_table[i].status_reg, &val,
				1);
		if (rc < 0) {
			pr_err("Failed to read bias_sts, rc=%d\n", rc);
			return rc;
		}

		if (val & BIAS_STS_READY) {
			rc = fg_read(fg, fg->rradc_base + id_table[i].lsb_reg,
					(u8 *)&tmp, 2);
			if (rc < 0) {
				pr_err("Failed to read bias_lsb_reg, rc=%d\n",
					rc);
				return rc;
			}
		}

		pr_debug("bias_code[%d]: 0x%04x\n", i, tmp);

		/*
		 * Bias code closer to MAX_BIAS_CODE/2 is the one which should
		 * be used for calculating battery id.
		 */
		if (!delta || abs(tmp - MAX_BIAS_CODE / 2) < delta) {
			bias_id = i;
			bias_code = tmp;
			delta = abs(tmp - MAX_BIAS_CODE / 2);
		}
	}

	pr_debug("bias_id: %d bias_code: 0x%04x\n", bias_id, bias_code);

	/*
	 * Following equation is used for calculating battery id.
	 * batt_id(KOhms) = bias_id(KOhms) / ((MAX_BIAS_CODE / bias_code) - 1)
	 */
	batt_id_kohms = (id_table[bias_id].bias_kohms * bias_code) * 10 /
			(MAX_BIAS_CODE - bias_code);
	fg->batt_id_ohms = (batt_id_kohms * 1000) / 10;
	return 0;
}

static int fg_gen4_get_nominal_capacity(struct fg_gen4_chip *chip,
					int64_t *nom_cap_uah)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	u8 buf[2];

	rc = fg_sram_read(fg, NOM_CAP_WORD, NOM_CAP_OFFSET, buf, 2,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading %04x[%d] rc=%d\n", NOM_CAP_WORD,
			NOM_CAP_OFFSET, rc);
		return rc;
	}

	*nom_cap_uah = (int)(buf[0] | buf[1] << 8) * 1000;
	fg_dbg(fg, FG_CAP_LEARN, "nom_cap_uah: %lld\n", *nom_cap_uah);
	return 0;
}

static int fg_gen4_get_learned_capacity(void *data, int64_t *learned_cap_uah)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc, act_cap_mah;
	u8 buf[2];

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	if (chip->fg_nvmem)
		rc = nvmem_device_read(chip->fg_nvmem, SDAM_CAP_LEARN_OFFSET, 2,
					buf);
	else
		rc = fg_get_sram_prop(fg, FG_SRAM_ACT_BATT_CAP, &act_cap_mah);
	if (rc < 0) {
		pr_err("Error in getting learned capacity, rc=%d\n", rc);
		return rc;
	}

	if (chip->fg_nvmem)
		*learned_cap_uah = (buf[0] | buf[1] << 8) * 1000;
	else
		*learned_cap_uah = act_cap_mah * 1000;

	fg_dbg(fg, FG_CAP_LEARN, "learned_cap_uah:%lld\n", *learned_cap_uah);
	return 0;
}

#define CC_SOC_30BIT	GENMASK(29, 0)
#define BATT_SOC_32BIT	GENMASK(31, 0)
static int fg_gen4_get_charge_raw(struct fg_gen4_chip *chip, int *val)
{
	int rc, cc_soc;
	int64_t nom_cap_uah;

	rc = fg_get_sram_prop(&chip->fg, FG_SRAM_CC_SOC, &cc_soc);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC, rc=%d\n", rc);
		return rc;
	}

	rc = fg_gen4_get_nominal_capacity(chip, &nom_cap_uah);
	if (rc < 0) {
		pr_err("Error in getting nominal capacity, rc=%d\n", rc);
		return rc;
	}

	*val = div_s64(cc_soc * nom_cap_uah, CC_SOC_30BIT);
	return 0;
}

static int fg_gen4_get_charge_counter(struct fg_gen4_chip *chip, int *val)
{
	int rc, cc_soc;
	int64_t learned_cap_uah;

	rc = fg_get_sram_prop(&chip->fg, FG_SRAM_CC_SOC_SW, &cc_soc);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
		return rc;
	}

	rc = fg_gen4_get_learned_capacity(chip, &learned_cap_uah);
	if (rc < 0) {
		pr_err("Error in getting learned capacity, rc=%d\n", rc);
		return rc;
	}

	*val = div_s64(cc_soc * learned_cap_uah, CC_SOC_30BIT);
	return 0;
}

static int fg_gen4_get_charge_counter_shadow(struct fg_gen4_chip *chip,
						int *val)
{
	int rc, batt_soc;
	int64_t learned_cap_uah;

	rc = fg_get_sram_prop(&chip->fg, FG_SRAM_BATT_SOC, &batt_soc);
	if (rc < 0) {
		pr_err("Error in getting BATT_SOC, rc=%d\n", rc);
		return rc;
	}

	rc = fg_gen4_get_learned_capacity(chip, &learned_cap_uah);
	if (rc < 0) {
		pr_err("Error in getting learned capacity, rc=%d\n", rc);
		return rc;
	}

	*val = div_u64((u32)batt_soc * learned_cap_uah, BATT_SOC_32BIT);
	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define SRAM_BATT_TEMP_UPDATE_TIME_THRESH_MS 3000
#endif
static int fg_gen4_get_battery_temp(struct fg_dev *fg, int *val)
{
	int rc = 0;
	u16 buf;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	s64 current_ktime_ms, ktime_diff_ms;

	current_ktime_ms = ktime_to_ms(ktime_get_boottime());
	ktime_diff_ms = current_ktime_ms - fg->last_update_batt_temp_ktime_ms;

	if (ktime_diff_ms >= 0 &&
		ktime_diff_ms < SRAM_BATT_TEMP_UPDATE_TIME_THRESH_MS) {
		*val = fg->last_update_batt_temp;
		fg_dbg(fg, FG_STATUS, "Skip to read batt_temp from sram to use last data(%d)\n",
									*val);
	} else {
		rc = fg_sram_read(fg, BATT_TEMP_WORD, BATT_TEMP_OFFSET,
					(u8 *)&buf, 2, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Failed to read BATT_TEMP_WORD rc=%d\n", rc);
			return rc;
		}
		*val = sign_extend32(buf, 9) * 100 / 40;

		fg->last_update_batt_temp_ktime_ms = current_ktime_ms;
		fg->last_update_batt_temp = *val;
		fg_dbg(fg, FG_STATUS, "Read batt_temp(%d) from sram since enough time has passed\n",
									*val);
	}
#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = fg_sram_read(fg, BATT_TEMP_WORD, BATT_TEMP_OFFSET, (u8 *)&buf,
			2, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Failed to read BATT_TEMP_WORD rc=%d\n", rc);
		return rc;
	}

	/*
	 * 10 bits representing temperature from -128 to 127 and each LSB is
	 * 0.25 C. Multiply by 10 to convert it to deci degrees C.
	 */
	*val = sign_extend32(buf, 9) * 100 / 40;
#endif

	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define ECELSIUS_DEGREE (-2730)
static int fg_gen4_somc_get_real_temp(struct fg_dev *fg, int *val)
{
	union power_supply_propval pval = {0, };
	int rc;
	int batt_temp;
	int aux_temp = ECELSIUS_DEGREE;
	int corrected_batt_temp = ECELSIUS_DEGREE;
	int corrected_aux_temp = ECELSIUS_DEGREE;

	if (fg->use_real_temp && (fg->real_temp_debug != -EINVAL)) {
		*val = fg->real_temp_debug;
		return 0;
	}

	if (fg->temp_corr_en) {
		if (fg->cell_temp_avtive) {
			*val = fg->cell_temp;
			fg_dbg(fg, FG_TEMP_CORR, "batt:%d -> real battery temp:%d\n",
							batt_temp, *val);
			return 0;
		} else {
			fg_dbg(fg, FG_TEMP_CORR, "cell_temp is not active yet. So use batt_temp or mixed temp.\n");
		}
	}

	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("failed to read batt_temp rc=%d\n", rc);
		return rc;
	}

	if (fg->use_real_temp) {
		corrected_batt_temp = batt_temp + fg->batt_temp_correctton;
	} else {
		*val = batt_temp;
		fg_dbg(fg, FG_STATUS,
			"Real Temp(mixed temp) is not supported. So, batt_temp is used\n");
		return 0;
	}

	if (fg->real_temp_use_aux) {
		if (!fg->batt_psy) {
			pr_err("failed to get batt_psy\n");
			return rc;
		}
		rc = power_supply_get_property(fg->batt_psy,
						POWER_SUPPLY_PROP_AUX_TEMP,
						&pval);
		if (rc) {
			pr_err("Couldn't get aux_temp rc = %d\n", rc);
		} else {
			aux_temp = pval.intval;
			corrected_aux_temp = aux_temp + fg->aux_temp_correctton;
		}
	}

	*val = max(corrected_batt_temp, corrected_aux_temp);
	fg_dbg(fg, FG_STATUS, "batt:%d aux:%d -> real battery temp(mixed temp):%d\n",
						batt_temp, aux_temp, *val);
	return 0;
}

static int fg_gen4_somc_set_real_temp_debug(struct fg_dev *fg, int val)
{
	int rc = 0;

	if (val < ECELSIUS_DEGREE) {
		fg->real_temp_debug = -EINVAL;
		rc = -EINVAL;
	} else {
		fg->real_temp_debug = val;
	}

	if (fg->batt_psy)
		power_supply_changed(fg->batt_psy);

	return 0;
}

#endif
static int fg_gen4_tz_get_temp(void *data, int *temperature)
{
	struct fg_dev *fg = (struct fg_dev *)data;
	int rc, temp;

	if (!temperature)
		return -EINVAL;

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = fg_gen4_get_battery_temp(fg, &temp);
	if (rc < 0)
		return rc;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->use_real_temp) {
		rc = fg_gen4_somc_get_real_temp(fg, &temp);
		if (rc < 0)
			return rc;
	} else {
		rc = fg_gen4_get_battery_temp(fg, &temp);
		if (rc < 0)
			return rc;
	}
#endif

	/* Convert deciDegC to milliDegC */
	*temperature = temp * 100;
	return rc;
}

static struct thermal_zone_of_device_ops fg_gen4_tz_ops = {
	.get_temp = fg_gen4_tz_get_temp,
};

static int fg_gen4_get_debug_batt_id(struct fg_dev *fg, int *batt_id)
{
	int rc;
	u16 tmp;

	rc = fg_read(fg, ADC_RR_FAKE_BATT_LOW_LSB(fg), (u8 *)&tmp, 2);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			ADC_RR_FAKE_BATT_LOW_LSB(fg), rc);
		return rc;
	}

	/*
	 * Following equation is used for calculating battery id.
	 * batt_id(KOhms) = 30000 / ((MAX_BIAS_CODE / bias_code) - 1)
	 */

	batt_id[0] = (30000 * tmp) / (MAX_BIAS_CODE - tmp);

	rc = fg_read(fg, ADC_RR_FAKE_BATT_HIGH_LSB(fg), (u8 *)&tmp, 2);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			ADC_RR_FAKE_BATT_HIGH_LSB(fg), rc);
		return rc;
	}

	batt_id[1] = (30000 * tmp) / (MAX_BIAS_CODE - tmp);
	pr_debug("debug batt_id range: [%d %d]\n", batt_id[0], batt_id[1]);
	return 0;
}

static bool is_debug_batt_id(struct fg_dev *fg)
{
	int debug_batt_id[2], rc;

	if (fg->batt_id_ohms < 0)
		return false;

	rc = fg_gen4_get_debug_batt_id(fg, debug_batt_id);
	if (rc < 0) {
		pr_err("Failed to get debug batt_id, rc=%d\n", rc);
		return false;
	}

	if (is_between(debug_batt_id[0], debug_batt_id[1],
		fg->batt_id_ohms)) {
		fg_dbg(fg, FG_POWER_SUPPLY, "Debug battery id: %dohms\n",
			fg->batt_id_ohms);
		return true;
	}

	return false;
}

static int fg_gen4_get_cell_impedance(struct fg_gen4_chip *chip, int *val)
{
	struct fg_dev *fg = &chip->fg;
	int rc, esr_uohms, temp, vbat_term_mv, v_delta, rprot_uohms = 0;
	int rslow_uohms, fv_uv = fg->bp.float_volt_uv;

	rc = fg_get_sram_prop(fg, FG_SRAM_ESR_ACT, &esr_uohms);
	if (rc < 0) {
		pr_err("failed to get ESR_ACT, rc=%d\n", rc);
		return rc;
	}

	rc = fg_get_sram_prop(fg, FG_SRAM_RSLOW, &rslow_uohms);
	if (rc < 0) {
		pr_err("failed to get Rslow, rc=%d\n", rc);
		return rc;
	}

	esr_uohms += rslow_uohms;

	if (!chip->dt.five_pin_battery)
		goto out;

	if (fg->charge_type != POWER_SUPPLY_CHARGE_TYPE_TAPER)
		goto out;

	if ((fg->charge_type == POWER_SUPPLY_CHARGE_TYPE_TAPER) &&
		(fg->health != POWER_SUPPLY_HEALTH_GOOD)) {
		if (!chip->fv_votable)
			chip->fv_votable = find_votable("FV");

		if (!chip->fv_votable)
			goto out;

		fv_uv = get_effective_result(chip->fv_votable);
	}

	if (fv_uv <= 0)
		goto out;

	rc = fg_get_battery_voltage(fg, &vbat_term_mv);
	if (rc < 0)
		goto out;

	rc = fg_get_sram_prop(fg, FG_SRAM_VBAT_FINAL, &temp);
	if (rc < 0) {
		pr_err("Error in getting VBAT_FINAL rc:%d\n", rc);
		goto out;
	}

	v_delta = abs(temp - fv_uv);

	rc = fg_get_sram_prop(fg, FG_SRAM_IBAT_FINAL, &temp);
	if (rc < 0) {
		pr_err("Error in getting IBAT_FINAL rc:%d\n", rc);
		goto out;
	}

	if (!temp)
		goto out;

	rprot_uohms = div64_u64((u64)v_delta * 1000000, abs(temp));
	pr_debug("v_delta: %d ibat_final: %d rprot_uohms: %d\n", v_delta, temp,
		rprot_uohms);
out:
	*val = esr_uohms - rprot_uohms;
	return rc;
}

static int fg_gen4_get_prop_capacity(struct fg_dev *fg, int *val)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, msoc;

	if (is_debug_batt_id(fg)) {
		*val = DEBUG_BATT_SOC;
		return 0;
	}

	if (fg->fg_restarting) {
		*val = fg->last_soc;
		return 0;
	}

	if (fg->battery_missing || !fg->soc_reporting_ready) {
		*val = BATT_MISS_SOC;
		return 0;
	}

	if (chip->vbatt_low) {
		*val = EMPTY_SOC;
		return 0;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->profile_load_status == PROFILE_MISSING) {
		*val = UNKNOWN_BATT_SOC;
		return 0;
	}

#endif
	if (fg->charge_full) {
		*val = FULL_CAPACITY;
		return 0;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->charge_full_releasing) {
		rc = fg_get_msoc(fg, &msoc);
		fg_dbg(fg, FG_SOMC,
			"Now working charge_full_releasing at %d\n", msoc);
		*val = FULL_CAPACITY;
		return 0;
	}

#endif
	if (chip->soc_scale_mode) {
		mutex_lock(&chip->soc_scale_lock);
		*val = chip->soc_scale_msoc;
		mutex_unlock(&chip->soc_scale_lock);
	} else {
		rc = fg_get_msoc(fg, &msoc);
		if (rc < 0)
			return rc;
		if (chip->dt.linearize_soc && fg->delta_soc > 0)
			*val = fg->maint_soc;
		else
			*val = msoc;
	}

	return 0;
}

static int fg_gen4_get_prop_real_capacity(struct fg_dev *fg, int *val)
{
	return fg_get_msoc(fg, val);
}

static int fg_gen4_get_prop_capacity_raw(struct fg_gen4_chip *chip, int *val)
{
	struct fg_dev *fg = &chip->fg;
	int rc;

	if (!chip->dt.soc_hi_res) {
		rc = fg_get_msoc_raw(fg, val);
		return rc;
	}

	if (!is_input_present(fg)) {
		rc = fg_gen4_get_prop_capacity(fg, val);
		if (!rc)
			*val = *val * 100;
		return rc;
	}

	rc = fg_get_sram_prop(&chip->fg, FG_SRAM_MONOTONIC_SOC, val);
	if (rc < 0) {
		pr_err("Error in getting MONOTONIC_SOC, rc=%d\n", rc);
		return rc;
	}

	/* Show it in centi-percentage */
	*val = (*val * 10000) / 0xFFFF;

	return 0;
}

static inline void get_esr_meas_current(int curr_ma, u8 *val)
{
	switch (curr_ma) {
	case 60:
		*val = ESR_MEAS_CUR_60MA;
		break;
	case 120:
		*val = ESR_MEAS_CUR_120MA;
		break;
	case 180:
		*val = ESR_MEAS_CUR_180MA;
		break;
	case 240:
		*val = ESR_MEAS_CUR_240MA;
		break;
	default:
		*val = ESR_MEAS_CUR_120MA;
		break;
	};

	*val <<= ESR_PULL_DOWN_IVAL_SHIFT;
}

static int fg_gen4_get_power(struct fg_gen4_chip *chip, int *val, bool average)
{
	struct fg_dev *fg = &chip->fg;
	int rc, v_min, v_pred, esr_uohms, rslow_uohms;
	s64 power;

	rc = fg_get_sram_prop(fg, FG_SRAM_VOLTAGE_PRED, &v_pred);
	if (rc < 0)
		return rc;

	v_min = chip->dt.sys_min_volt_mv * 1000;
	power = (s64)v_min * (v_pred - v_min);

	rc = fg_get_sram_prop(fg, FG_SRAM_ESR_ACT, &esr_uohms);
	if (rc < 0) {
		pr_err("failed to get ESR_ACT, rc=%d\n", rc);
		return rc;
	}

	rc = fg_get_sram_prop(fg, FG_SRAM_RSLOW, &rslow_uohms);
	if (rc < 0) {
		pr_err("failed to get Rslow, rc=%d\n", rc);
		return rc;
	}

	if (average)
		power = div_s64(power, esr_uohms + rslow_uohms);
	else
		power = div_s64(power, esr_uohms);

	pr_debug("V_min: %d V_pred: %d ESR: %d Rslow: %d power: %lld\n", v_min,
		v_pred, esr_uohms, rslow_uohms, power);

	*val = power;
	return 0;
}

static int fg_gen4_get_prop_soc_scale(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc;

	rc = fg_get_sram_prop(fg, FG_SRAM_VBAT_FLT, &chip->vbatt_avg);
	if (rc < 0) {
		pr_err("Failed to get filtered battery voltage, rc = %d\n",
			rc);
		return rc;
	}

	rc = fg_get_battery_voltage(fg, &chip->vbatt_now);
	if (rc < 0) {
		pr_err("Failed to get battery voltage, rc =%d\n", rc);
		return rc;
	}

	rc = fg_get_battery_current(fg, &chip->current_now);
	if (rc < 0) {
		pr_err("Failed to get battery current rc=%d\n", rc);
		return rc;
	}

	chip->vbatt_now = DIV_ROUND_CLOSEST(chip->vbatt_now, 1000);
	chip->vbatt_avg = DIV_ROUND_CLOSEST(chip->vbatt_avg, 1000);
	chip->vbatt_res = chip->vbatt_avg - chip->dt.cutoff_volt_mv;
	fg_dbg(fg, FG_FVSS, "Vbatt now=%d Vbatt avg=%d Vbatt res=%d\n",
		chip->vbatt_now, chip->vbatt_avg, chip->vbatt_res);

	return rc;
}

#define SDAM1_MEM_124_REG	0xB0BC
static int fg_gen4_set_calibrate_level(struct fg_gen4_chip *chip, int val)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	u8 buf;

	if (!chip->pbs_dev)
		return -ENODEV;

	if (is_debug_batt_id(fg))
		return 0;

	if (val < 0 || val > 0x83) {
		pr_err("Incorrect calibration level %d\n", val);
		return -EINVAL;
	}

	if (val == chip->calib_level)
		return 0;

	if (chip->dt.force_calib_level != -EINVAL)
		val = chip->dt.force_calib_level;

	buf = (u8)val;
	rc = fg_write(fg, SDAM1_MEM_124_REG, &buf, 1);
	if (rc < 0) {
		pr_err("Error in writing to 0x%04X, rc=%d\n",
			SDAM1_MEM_124_REG, rc);
		return rc;
	}

	buf = 0x1;
	rc = qpnp_pbs_trigger_event(chip->pbs_dev, buf);
	if (rc < 0) {
		pr_err("Error in triggering PBS rc=%d\n", rc);
		return rc;
	}

	rc = fg_read(fg, SDAM1_MEM_124_REG, &buf, 1);
	if (rc < 0) {
		pr_err("Error in reading from 0x%04X, rc=%d\n",
			SDAM1_MEM_124_REG, rc);
		return rc;
	}

	if (buf) {
		pr_err("Incorrect return value: %x\n", buf);
		return -EINVAL;
	}

	if (is_parallel_charger_available(fg)) {
		cancel_work_sync(&chip->pl_current_en_work);
		schedule_work(&chip->pl_current_en_work);
	}

	chip->calib_level = val;
	fg_dbg(fg, FG_POWER_SUPPLY, "Set calib_level to %x\n", val);
	return 0;
}

/* ALG callback functions below */

static int fg_gen4_get_ttf_param(void *data, enum ttf_param param, int *val)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc = 0, act_cap_mah, full_soc;
	u8 buf[2];

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;

	switch (param) {
	case TTF_TTE_VALID:
		*val = 1;
		if (fg->battery_missing || is_debug_batt_id(fg))
			*val = 0;
		break;
	case TTF_MSOC:
		rc = fg_gen4_get_prop_capacity(fg, val);
		break;
	case TTF_VBAT:
		rc = fg_get_battery_voltage(fg, val);
		break;
	case TTF_OCV:
		rc = fg_get_sram_prop(fg, FG_SRAM_OCV, val);
		if (rc < 0)
			pr_err("Failed to get battery OCV, rc=%d\n", rc);
		break;
	case TTF_IBAT:
		rc = fg_get_battery_current(fg, val);
		break;
	case TTF_FCC:
		if (chip->fg_nvmem)
			rc = nvmem_device_read(chip->fg_nvmem,
					SDAM_CAP_LEARN_OFFSET, 2, buf);
		else
			rc = fg_get_sram_prop(fg, FG_SRAM_ACT_BATT_CAP,
					&act_cap_mah);
		if (rc < 0) {
			pr_err("Failed to get ACT_BATT_CAP rc=%d\n", rc);
			break;
		}

		if (chip->fg_nvmem)
			act_cap_mah = buf[0] | buf[1] << 8;

		rc = fg_get_sram_prop(fg, FG_SRAM_FULL_SOC, &full_soc);
		if (rc < 0) {
			pr_err("Failed to get FULL_SOC rc=%d\n", rc);
			break;
		}

		full_soc = DIV_ROUND_CLOSEST(((u16)full_soc >> 8) *
						FULL_CAPACITY, FULL_SOC_RAW);
		*val = full_soc * act_cap_mah / FULL_CAPACITY;
		break;
	case TTF_MODE:
		if (is_qnovo_en(fg))
			*val = TTF_MODE_QNOVO;
		else if (chip->ttf->step_chg_cfg_valid)
			*val = TTF_MODE_VBAT_STEP_CHG;
		else if (chip->ttf->ocv_step_chg_cfg_valid)
			*val = TTF_MODE_OCV_STEP_CHG;
		else
			*val = TTF_MODE_NORMAL;
		break;
	case TTF_ITERM:
		*val = chip->dt.sys_term_curr_ma;
		break;
	case TTF_RBATT:
		rc = fg_gen4_get_cell_impedance(chip, val);
		break;
	case TTF_VFLOAT:
		*val = fg->bp.float_volt_uv;
		break;
	case TTF_CHG_TYPE:
		*val = fg->charge_type;
		break;
	case TTF_CHG_STATUS:
		*val = fg->charge_status;
		break;
	case TTF_CHG_DONE:
		*val = fg->charge_done;
		break;
	default:
		pr_err_ratelimited("Unsupported parameter %d\n", param);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int fg_gen4_store_learned_capacity(void *data, int64_t learned_cap_uah)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int16_t cc_mah;
	int rc;
	u8 cookie = SDAM_COOKIE;

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	if (fg->battery_missing || !learned_cap_uah)
		return -EPERM;

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cc_mah = div64_s64(learned_cap_uah, 1000);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cc_mah = DIV_ROUND_CLOSEST(learned_cap_uah, 1000);
#endif
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ACT_BATT_CAP].addr_word,
			fg->sp[FG_SRAM_ACT_BATT_CAP].addr_byte, (u8 *)&cc_mah,
			fg->sp[FG_SRAM_ACT_BATT_CAP].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing act_batt_cap_bkup, rc=%d\n", rc);
		return rc;
	}

	if (chip->fg_nvmem) {
		rc = nvmem_device_write(chip->fg_nvmem, SDAM_CAP_LEARN_OFFSET,
					2, (u8 *)&cc_mah);
		if (rc < 0) {
			pr_err("Error in writing learned capacity to SDAM, rc=%d\n",
				rc);
			return rc;
		}

		rc = nvmem_device_write(chip->fg_nvmem, SDAM_COOKIE_OFFSET, 1,
					&cookie);
		if (rc < 0) {
			pr_err("Error in writing cookie to SDAM, rc=%d\n", rc);
			return rc;
		}
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_CAP_LEARN, "learned capacity %llduah/%dmah stored\n",
		chip->cl->learned_cap_uah, cc_mah);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_SOMC, "learned capacity %llduah/%dmah stored\n",
		chip->cl->learned_cap_uah, cc_mah);
#endif
	return 0;
}

static int fg_gen4_prime_cc_soc_sw(void *data, u32 batt_soc)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc, cc_soc_sw;

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	if (batt_soc == CC_SOC_30BIT)
		cc_soc_sw = batt_soc;
	else
		cc_soc_sw = div64_s64((int64_t)batt_soc * CC_SOC_30BIT,
				BATT_SOC_32BIT);

	rc = fg_sram_write(fg, fg->sp[FG_SRAM_CC_SOC_SW].addr_word,
		fg->sp[FG_SRAM_CC_SOC_SW].addr_byte, (u8 *)&cc_soc_sw,
		fg->sp[FG_SRAM_CC_SOC_SW].len, FG_IMA_ATOMIC);
	if (rc < 0)
		pr_err("Error in writing cc_soc_sw, rc=%d\n", rc);
	else
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_STATUS, "cc_soc_sw: %x\n", cc_soc_sw);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_SOMC, "prime cc_soc_sw: %x\n", cc_soc_sw);
#endif

	return rc;
}

static bool fg_gen4_cl_ok_to_begin(void *data)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc, val = 0;

	if (!chip)
		return false;

	fg = &chip->fg;

	if (chip->cl->dt.ibat_flt_thr_ma <= 0)
		return true;

	rc = fg_get_sram_prop(fg, FG_SRAM_IBAT_FLT, &val);
	if (rc < 0) {
		pr_err("Failed to get filtered battery current, rc = %d\n",
			rc);
		return true;
	}

	/* convert to milli-units */
	val = DIV_ROUND_CLOSEST(val, 1000);

	pr_debug("IBAT_FLT thr: %d val: %d\n", chip->cl->dt.ibat_flt_thr_ma,
		val);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* disable the check for false */
	fg_dbg(fg, FG_SOMC, "IBAT_FLT thr: %d val: %d but ignore this function\n",
		chip->cl->dt.ibat_flt_thr_ma, val);
	return true;
#endif
	if (abs(val) > chip->cl->dt.ibat_flt_thr_ma)
		return false;

	return true;
}

static int fg_gen4_get_cc_soc_sw(void *data, int *cc_soc_sw)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc, temp;

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	rc = fg_get_sram_prop(fg, FG_SRAM_CC_SOC_SW, &temp);
	if (rc < 0) {
		pr_err("Error in getting CC_SOC_SW, rc=%d\n", rc);
		return rc;
	}

	*cc_soc_sw = temp;
	return rc;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_somc_get_monotonic_soc(void *data, int *msoc)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg;
	int rc;

	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	rc = fg_get_msoc(fg, msoc);
	if (rc < 0) {
		pr_err("Error in getting msoc, rc=%d\n", rc);
		return rc;
	}

	return rc;
}

#endif
static int fg_gen4_restore_count(void *data, u16 *buf, int length)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct fg_gen4_chip *chip = data;
	int id, rc = 0;
	u8 tmp[2];

	if (!chip)
		return -ENODEV;

	if (!buf || length > BUCKET_COUNT)
		return -EINVAL;

	for (id = 0; id < length; id++) {
		if (chip->fg_nvmem)
			rc = nvmem_device_read(chip->fg_nvmem,
				SDAM_CYCLE_COUNT_OFFSET + (id * 2), 2, tmp);
		else
			rc = fg_sram_read(&chip->fg, CYCLE_COUNT_WORD + id,
					CYCLE_COUNT_OFFSET, (u8 *)tmp, 2,
					FG_IMA_DEFAULT);
		if (rc < 0)
			pr_err("failed to read bucket %d rc=%d\n", id, rc);
		else
			*buf++ = tmp[0] | tmp[1] << 8;
	}

	return rc;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	return 0;
#endif
}

static int fg_gen4_store_count(void *data, u16 *buf, int id, int length)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct fg_gen4_chip *chip = data;
	int rc;

	if (!chip)
		return -ENODEV;

	if (!buf || length > BUCKET_COUNT * 2 || id < 0 ||
		id > BUCKET_COUNT - 1 || ((id * 2) + length) > BUCKET_COUNT * 2)
		return -EINVAL;

	if (chip->fg_nvmem)
		rc = nvmem_device_write(chip->fg_nvmem,
			SDAM_CYCLE_COUNT_OFFSET + (id * 2), length, (u8 *)buf);
	else
		rc = fg_sram_write(&chip->fg, CYCLE_COUNT_WORD + id,
				CYCLE_COUNT_OFFSET, (u8 *)buf, length,
				FG_IMA_DEFAULT);
	if (rc < 0)
		pr_err("failed to write bucket %d rc=%d\n", id, rc);

	return rc;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	return 0;
#endif
}

/* All worker and helper functions below */

static int fg_parse_dt_property_u32_array(struct device_node *node,
				const char *prop_name, int *buf, int len)
{
	int rc;

	rc = of_property_count_elems_of_size(node, prop_name, sizeof(u32));
	if (rc < 0) {
		if (rc == -EINVAL)
			return 0;
		else
			return rc;
	} else if (rc != len) {
		pr_err("Incorrect length %d for %s, rc=%d\n", len, prop_name,
			rc);
		return -EINVAL;
	}

	rc = of_property_read_u32_array(node, prop_name, buf, len);
	if (rc < 0) {
		pr_err("Error in reading %s, rc=%d\n", prop_name, rc);
		return rc;
	}

	return 0;
}

static void fg_gen4_update_rslow_coeff(struct fg_dev *fg, int batt_temp)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, i;
	bool rslow_low = false;
	u8 buf[RSLOW_NUM_COEFFS];

	if (!fg->bp.rslow_normal_coeffs || !fg->bp.rslow_low_coeffs)
		return;

	/* Update Rslow low coefficients when Tbatt is < 0 C */
	if (batt_temp < 0)
		rslow_low = true;

	if (chip->rslow_low == rslow_low)
		return;

	for (i = 0; i < RSLOW_NUM_COEFFS; i++) {
		if (rslow_low)
			buf[i] = fg->bp.rslow_low_coeffs[i] & 0xFF;
		else
			buf[i] = fg->bp.rslow_normal_coeffs[i] & 0xFF;
	}

	rc = fg_sram_write(fg, RSLOW_COEFF_DISCHG_WORD, RSLOW_COEFF_LOW_OFFSET,
			buf, RSLOW_NUM_COEFFS, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Failed to write RLOW_COEFF_DISCHG_WORD rc=%d\n", rc);
	} else {
		chip->rslow_low = rslow_low;
		fg_dbg(fg, FG_STATUS, "Updated Rslow %s coefficients\n",
			rslow_low ? "low" : "normal");
	}
}

#define KI_COEFF_FULL_SOC_NORM_DEFAULT	2442
#define KI_COEFF_FULL_SOC_LOW_DEFAULT	2442
static int fg_gen4_adjust_ki_coeff_full_soc(struct fg_gen4_chip *chip,
						int batt_temp)
{
	struct fg_dev *fg = &chip->fg;
	int rc, ki_coeff_full_soc_norm, ki_coeff_full_soc_low;
	u8 val;

	if ((batt_temp < 0) ||
		(fg->charge_status == POWER_SUPPLY_STATUS_DISCHARGING)) {
		ki_coeff_full_soc_norm = 0;
		ki_coeff_full_soc_low = 0;
	} else if (fg->charge_status == POWER_SUPPLY_STATUS_CHARGING) {
		ki_coeff_full_soc_norm = chip->dt.ki_coeff_full_soc_dischg[0];
		ki_coeff_full_soc_low = chip->dt.ki_coeff_full_soc_dischg[1];
	}

	if (chip->ki_coeff_full_soc[0] == ki_coeff_full_soc_norm &&
		chip->ki_coeff_full_soc[1] == ki_coeff_full_soc_low)
		return 0;

	fg_encode(fg->sp, FG_SRAM_KI_COEFF_FULL_SOC, ki_coeff_full_soc_norm,
		&val);
	rc = fg_sram_write(fg, KI_COEFF_FULL_SOC_NORM_WORD,
			KI_COEFF_FULL_SOC_NORM_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ki_coeff_full_soc_norm, rc=%d\n", rc);
		return rc;
	}

	fg_encode(fg->sp, FG_SRAM_KI_COEFF_FULL_SOC, ki_coeff_full_soc_low,
		&val);
	rc = fg_sram_write(fg, KI_COEFF_LOW_DISCHG_WORD,
			KI_COEFF_FULL_SOC_LOW_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ki_coeff_full_soc_low, rc=%d\n", rc);
		return rc;
	}

	chip->ki_coeff_full_soc[0] = ki_coeff_full_soc_norm;
	chip->ki_coeff_full_soc[1] = ki_coeff_full_soc_low;
	fg_dbg(fg, FG_STATUS, "Wrote ki_coeff_full_soc [%d %d]\n",
		ki_coeff_full_soc_norm, ki_coeff_full_soc_low);
	return 0;
}

static int fg_gen4_set_ki_coeff_dischg(struct fg_dev *fg, int ki_coeff_low,
		int ki_coeff_med, int ki_coeff_hi)
{
	int rc;
	u8 val;

	if (ki_coeff_low >= 0) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_LOW_DISCHG, ki_coeff_low,
			&val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_LOW_DISCHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_LOW_DISCHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_LOW_DISCHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_low, rc=%d\n", rc);
			return rc;
		}
		fg_dbg(fg, FG_STATUS, "Wrote ki_coeff_low %d\n", ki_coeff_low);
	}

	if (ki_coeff_med >= 0) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_MED_DISCHG, ki_coeff_med,
			&val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_MED_DISCHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_MED_DISCHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_MED_DISCHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_med, rc=%d\n", rc);
			return rc;
		}
		fg_dbg(fg, FG_STATUS, "Wrote ki_coeff_med %d\n", ki_coeff_med);
	}

	if (ki_coeff_hi >= 0) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_HI_DISCHG, ki_coeff_hi,
			&val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_HI_DISCHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_HI_DISCHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_HI_DISCHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_hi, rc=%d\n", rc);
			return rc;
		}
		fg_dbg(fg, FG_STATUS, "Wrote ki_coeff_hi %d\n", ki_coeff_hi);
	}

	return 0;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define KI_COEFF_LOW_DISCHG_DEFAULT	367
#define KI_COEFF_MED_DISCHG_DEFAULT	62
#define KI_COEFF_HI_DISCHG_DEFAULT	0
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define KI_COEFF_LOW_DISCHG_DEFAULT	184
#define KI_COEFF_MED_DISCHG_DEFAULT	123
#define KI_COEFF_HI_DISCHG_DEFAULT	62
#endif
static int fg_gen4_adjust_ki_coeff_dischg(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, i, msoc;
	int ki_coeff_low = KI_COEFF_LOW_DISCHG_DEFAULT;
	int ki_coeff_med = KI_COEFF_MED_DISCHG_DEFAULT;
	int ki_coeff_hi = KI_COEFF_HI_DISCHG_DEFAULT;

	if (!chip->ki_coeff_dischg_en)
		return 0;

	rc = fg_gen4_get_prop_capacity(fg, &msoc);
	if (rc < 0) {
		pr_err("Error in getting capacity, rc=%d\n", rc);
		return rc;
	}

	if (fg->charge_status == POWER_SUPPLY_STATUS_DISCHARGING) {
		for (i = KI_COEFF_SOC_LEVELS - 1; i >= 0; i--) {
			if (msoc < chip->dt.ki_coeff_soc[i]) {
				ki_coeff_low = chip->dt.ki_coeff_low_dischg[i];
				ki_coeff_med = chip->dt.ki_coeff_med_dischg[i];
				ki_coeff_hi = chip->dt.ki_coeff_hi_dischg[i];
			}
		}
	}

	rc = fg_gen4_set_ki_coeff_dischg(fg,
			ki_coeff_low, ki_coeff_med, ki_coeff_hi);
	if (rc < 0)
		return rc;

	return 0;
}

static int fg_gen4_slope_limit_config(struct fg_gen4_chip *chip, int batt_temp)
{
	struct fg_dev *fg = &chip->fg;
	enum slope_limit_status status;
	int rc;
	u8 buf;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u8 read_buf;
#endif

	if (!chip->slope_limit_en || chip->rapid_soc_dec_en)
		return 0;

	if (fg->charge_status == POWER_SUPPLY_STATUS_CHARGING ||
		fg->charge_status == POWER_SUPPLY_STATUS_FULL) {
		if (batt_temp < chip->dt.slope_limit_temp)
			status = LOW_TEMP_CHARGE;
		else
			status = HIGH_TEMP_CHARGE;
	} else {
		if (batt_temp < chip->dt.slope_limit_temp)
			status = LOW_TEMP_DISCHARGE;
		else
			status = HIGH_TEMP_DISCHARGE;
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (chip->slope_limit_sts == status)
		return 0;

	fg_encode(fg->sp, FG_SRAM_SLOPE_LIMIT,
		chip->dt.slope_limit_coeffs[status], &buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_SLOPE_LIMIT].addr_word,
			fg->sp[FG_SRAM_SLOPE_LIMIT].addr_byte, &buf,
			fg->sp[FG_SRAM_SLOPE_LIMIT].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in configuring slope_limit coefficient, rc=%d\n",
			rc);
		return rc;
	}
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = fg_sram_read(fg, fg->sp[FG_SRAM_SLOPE_LIMIT].addr_word,
				fg->sp[FG_SRAM_SLOPE_LIMIT].addr_byte,
				&read_buf,
				fg->sp[FG_SRAM_SLOPE_LIMIT].len,
				FG_IMA_DEFAULT);

	if (fg->slope_limit_suspended)
		fg_encode(fg->sp, FG_SRAM_SLOPE_LIMIT, 0, &buf);
	else
		fg_encode(fg->sp, FG_SRAM_SLOPE_LIMIT,
				chip->dt.slope_limit_coeffs[status], &buf);

	if (read_buf != buf) {
		rc = fg_sram_write(fg, fg->sp[FG_SRAM_SLOPE_LIMIT].addr_word,
					fg->sp[FG_SRAM_SLOPE_LIMIT].addr_byte,
					&buf, fg->sp[FG_SRAM_SLOPE_LIMIT].len,
					FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in configuring slope_limit coefficient, rc=%d\n",
				rc);
			return rc;
		}
		fg_dbg(fg, FG_SOMC, "slope_limit: %d->%d\n", read_buf, buf);
	}
#endif

	chip->slope_limit_sts = status;
	fg_dbg(fg, FG_STATUS, "Slope limit status: %d value: %x\n", status,
		buf);
	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_somc_suspend_slope_limit(struct fg_gen4_chip *chip,
								bool suspend)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	int batt_temp;

	fg->slope_limit_suspended = suspend;
	fg_dbg(fg, FG_SOMC, "Slope limit set to %s\n",
						suspend ? "zero" : "normal");

	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("Error in getting batt_temp\n");
		return rc;
	}

	rc = fg_gen4_slope_limit_config(chip, batt_temp);
	if (rc < 0) {
		pr_err("Error in configuring slope limiter rc:%d\n",
								rc);
		return rc;
	}

	return 0;
}

#endif
static int fg_gen4_configure_cutoff_current(struct fg_dev *fg, int current_ma)
{
	int rc;
	u8 buf[2];

	fg_encode(fg->sp, FG_SRAM_CUTOFF_CURR, current_ma, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_CUTOFF_CURR].addr_word,
			fg->sp[FG_SRAM_CUTOFF_CURR].addr_byte, buf,
			fg->sp[FG_SRAM_CUTOFF_CURR].len, FG_IMA_DEFAULT);
	if (rc < 0)
		pr_err("Error in writing cutoff_curr, rc=%d\n", rc);

	return rc;
}

#define SLOPE_LIMIT_DEFAULT	5738
#define CUTOFF_CURRENT_MAX	15999
static int fg_gen4_rapid_soc_config(struct fg_gen4_chip *chip, bool en)
{
	struct fg_dev *fg = &chip->fg;
	int rc, slope_limit_coeff, cutoff_curr_ma;
	u8 buf;

	slope_limit_coeff = en ? SLOPE_LIMIT_COEFF_MAX : SLOPE_LIMIT_DEFAULT;
	fg_encode(fg->sp, FG_SRAM_SLOPE_LIMIT, slope_limit_coeff, &buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_SLOPE_LIMIT].addr_word,
			fg->sp[FG_SRAM_SLOPE_LIMIT].addr_byte, &buf,
			fg->sp[FG_SRAM_SLOPE_LIMIT].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in configuring slope_limit coefficient, rc=%d\n",
			rc);
		return rc;
	}

	cutoff_curr_ma = en ? CUTOFF_CURRENT_MAX : chip->dt.cutoff_curr_ma;
	rc = fg_gen4_configure_cutoff_current(fg, cutoff_curr_ma);
	if (rc < 0)
		return rc;

	fg_dbg(fg, FG_STATUS, "Configured slope limit coeff %d cutoff current %d mA\n",
		slope_limit_coeff, cutoff_curr_ma);
	return 0;
}

static int qpnp_fg_gen4_get_step_charging_params(struct fg_gen4_chip *chip,
					struct device_node *profile_node)
{
	struct fg_dev *fg = &chip->fg;
	int rc, len, i, tuple_len;

	/*
	 * Currently step charging thresholds should be read only for Vbatt
	 * based and not for SOC based.
	 */
	if (!of_property_read_bool(profile_node, "qcom,soc-based-step-chg") &&
		of_find_property(profile_node, "qcom,step-chg-ranges", &len) &&
		fg->bp.float_volt_uv > 0 && fg->bp.fastchg_curr_ma > 0) {
		len /= sizeof(u32);
		tuple_len = len / (sizeof(struct range_data) / sizeof(u32));
		if (tuple_len <= 0 || tuple_len > MAX_STEP_CHG_ENTRIES)
			return -EINVAL;

		mutex_lock(&chip->ttf->lock);
		chip->ttf->step_chg_cfg =
			kcalloc(len, sizeof(*chip->ttf->step_chg_cfg),
				GFP_KERNEL);
		if (!chip->ttf->step_chg_cfg) {
			mutex_unlock(&chip->ttf->lock);
			return -ENOMEM;
		}

		chip->ttf->step_chg_data =
			kcalloc(tuple_len, sizeof(*chip->ttf->step_chg_data),
				GFP_KERNEL);
		if (!chip->ttf->step_chg_data) {
			kfree(chip->ttf->step_chg_cfg);
			mutex_unlock(&chip->ttf->lock);
			return -ENOMEM;
		}

		rc = read_range_data_from_node(profile_node,
				"qcom,step-chg-ranges",
				chip->ttf->step_chg_cfg,
				fg->bp.float_volt_uv,
				fg->bp.fastchg_curr_ma * 1000);
		if (rc < 0) {
			pr_err("Error in reading qcom,step-chg-ranges from battery profile, rc=%d\n",
				rc);
			kfree(chip->ttf->step_chg_data);
			kfree(chip->ttf->step_chg_cfg);
			chip->ttf->step_chg_cfg = NULL;
			mutex_unlock(&chip->ttf->lock);
			return rc;
		}

		chip->ttf->step_chg_num_params = tuple_len;
		chip->ttf->step_chg_cfg_valid = true;
		if (of_property_read_bool(profile_node,
					   "qcom,ocv-based-step-chg")) {
			chip->ttf->step_chg_cfg_valid = false;
			chip->ttf->ocv_step_chg_cfg_valid = true;
		}

		mutex_unlock(&chip->ttf->lock);

		if (chip->ttf->step_chg_cfg_valid) {
			for (i = 0; i < tuple_len; i++)
				pr_debug("Vbatt_low: %d Vbatt_high: %d FCC: %d\n",
				chip->ttf->step_chg_cfg[i].low_threshold,
				chip->ttf->step_chg_cfg[i].high_threshold,
				chip->ttf->step_chg_cfg[i].value);
		}
	}

	return 0;
}

static int fg_gen4_get_batt_profile_dt_props(struct fg_gen4_chip *chip,
					struct device_node *profile_node)
{
	struct fg_dev *fg = &chip->fg;
	int rc, len;

	rc = of_property_read_string(profile_node, "qcom,battery-type",
			&fg->bp.batt_type_str);
	if (rc < 0) {
		pr_err("battery type unavailable, rc:%d\n", rc);
		return rc;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = of_property_read_u32(profile_node, "somc,initial-capacity-uah",
			&fg->initial_capacity);
	if (rc < 0) {
		pr_err("battery initial capacity unavailable, rc:%d\n", rc);
		fg->initial_capacity = -EINVAL;
	}

#endif
	rc = of_property_read_u32(profile_node, "qcom,max-voltage-uv",
			&fg->bp.float_volt_uv);
	if (rc < 0) {
		pr_err("battery float voltage unavailable, rc:%d\n", rc);
		fg->bp.float_volt_uv = -EINVAL;
	}

	rc = of_property_read_u32(profile_node, "qcom,fastchg-current-ma",
			&fg->bp.fastchg_curr_ma);
	if (rc < 0) {
		pr_err("battery fastchg current unavailable, rc:%d\n", rc);
		fg->bp.fastchg_curr_ma = -EINVAL;
	}

	rc = of_property_read_u32(profile_node, "qcom,fg-cc-cv-threshold-mv",
			&fg->bp.vbatt_full_mv);
	if (rc < 0) {
		pr_err("battery cc_cv threshold unavailable, rc:%d\n", rc);
		fg->bp.vbatt_full_mv = -EINVAL;
	}

	if (of_find_property(profile_node, "qcom,therm-coefficients", &len)) {
		len /= sizeof(u32);
		if (len == BATT_THERM_NUM_COEFFS) {
			if (!fg->bp.therm_coeffs) {
				fg->bp.therm_coeffs = devm_kcalloc(fg->dev,
					BATT_THERM_NUM_COEFFS, sizeof(u32),
					GFP_KERNEL);
				if (!fg->bp.therm_coeffs)
					return -ENOMEM;
			}
		}

		rc = of_property_read_u32_array(profile_node,
			"qcom,therm-coefficients", fg->bp.therm_coeffs, len);
		if (rc < 0) {
			pr_err("Couldn't read therm coefficients, rc:%d\n", rc);
			devm_kfree(fg->dev, fg->bp.therm_coeffs);
			fg->bp.therm_coeffs = NULL;
		}

		rc = of_property_read_u32(profile_node,
			"qcom,therm-center-offset", &fg->bp.therm_ctr_offset);
		if (rc < 0) {
			pr_err("battery therm-center-offset unavailable, rc:%d\n",
				rc);
			fg->bp.therm_ctr_offset = -EINVAL;
		}
	}

	if (of_find_property(profile_node, "qcom,therm-pull-up", NULL)) {
		rc = of_property_read_u32(profile_node, "qcom,therm-pull-up",
				&fg->bp.therm_pull_up_kohms);
		if (rc < 0) {
			pr_err("Couldn't read therm-pull-up, rc:%d\n", rc);
			fg->bp.therm_pull_up_kohms = -EINVAL;
		}
	}

	if (of_find_property(profile_node, "qcom,rslow-normal-coeffs", NULL) &&
		of_find_property(profile_node, "qcom,rslow-low-coeffs", NULL)) {
		if (!fg->bp.rslow_normal_coeffs) {
			fg->bp.rslow_normal_coeffs = devm_kcalloc(fg->dev,
						RSLOW_NUM_COEFFS, sizeof(u32),
						GFP_KERNEL);
			if (!fg->bp.rslow_normal_coeffs)
				return -ENOMEM;
		}

		if (!fg->bp.rslow_low_coeffs) {
			fg->bp.rslow_low_coeffs = devm_kcalloc(fg->dev,
						RSLOW_NUM_COEFFS, sizeof(u32),
						GFP_KERNEL);
			if (!fg->bp.rslow_low_coeffs) {
				devm_kfree(fg->dev, fg->bp.rslow_normal_coeffs);
				fg->bp.rslow_normal_coeffs = NULL;
				return -ENOMEM;
			}
		}

		rc = fg_parse_dt_property_u32_array(profile_node,
			"qcom,rslow-normal-coeffs",
			fg->bp.rslow_normal_coeffs, RSLOW_NUM_COEFFS);
		if (rc < 0) {
			devm_kfree(fg->dev, fg->bp.rslow_normal_coeffs);
			fg->bp.rslow_normal_coeffs = NULL;
			devm_kfree(fg->dev, fg->bp.rslow_low_coeffs);
			fg->bp.rslow_low_coeffs = NULL;
			return rc;
		}

		rc = fg_parse_dt_property_u32_array(profile_node,
			"qcom,rslow-low-coeffs",
			fg->bp.rslow_low_coeffs, RSLOW_NUM_COEFFS);
		if (rc < 0) {
			devm_kfree(fg->dev, fg->bp.rslow_normal_coeffs);
			fg->bp.rslow_normal_coeffs = NULL;
			devm_kfree(fg->dev, fg->bp.rslow_low_coeffs);
			fg->bp.rslow_low_coeffs = NULL;
			return rc;
		}
	}

	rc = qpnp_fg_gen4_get_step_charging_params(chip, profile_node);
	if (rc < 0)
		return rc;

	return 0;
}

static int fg_gen4_get_batt_profile(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	struct device_node *node = fg->dev->of_node;
	struct device_node *batt_node, *profile_node;
	const char *data;
	int rc, len, avail_age_level = 0;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int num;
	int step_len;
	int i;
	u32 step_buf[STEP_DATA_DT_MAX_NUM];
	int batt_aging_level;
	char batt_node_name[MAX_BATT_NODE_NAME_SIZE];
	ssize_t size = 0;

	memset(step_buf, 0, sizeof(step_buf));
#endif

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	batt_node = of_find_node_by_name(node, "qcom,battery-data");
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->requested_batt_aging_level != NO_REQUESTED_LEVEL)
		batt_aging_level = fg->requested_batt_aging_level;
	else
		batt_aging_level = fg->batt_aging_level;

	if (fg->max_batt_aging_level != BATT_AGING_LEVEL_NONE) {
		size = scnprintf(batt_node_name, sizeof(batt_node_name),
				"somc,battery-data-al%d", batt_aging_level);
		if (size < 0) {
			pr_err("Cannot create the Batterydata node name.\n");
			return -ENXIO;
		}
		fg_dbg(fg, FG_SOMC, "Batterydata node: %s\n", batt_node_name);
		batt_node = of_find_node_by_name(node, batt_node_name);
	} else {
		fg_dbg(fg, FG_SOMC, "Batterydata node: Qualcomm default\n");
		batt_node = of_find_node_by_name(node, "qcom,battery-data");
	}

#endif
	if (!batt_node) {
		pr_err("Batterydata not available\n");
		return -ENXIO;
	}

	if (chip->dt.multi_profile_load)
		profile_node = of_batterydata_get_best_aged_profile(batt_node,
					fg->batt_id_ohms / 1000,
					chip->batt_age_level, &avail_age_level);
	else
		profile_node = of_batterydata_get_best_profile(batt_node,
					fg->batt_id_ohms / 1000, NULL);
	if (IS_ERR(profile_node))
		return PTR_ERR(profile_node);

	if (!profile_node) {
		pr_err("couldn't find profile handle\n");
		return -ENODATA;
	}

	if (chip->dt.multi_profile_load) {
		if (chip->batt_age_level != avail_age_level) {
			fg_dbg(fg, FG_STATUS, "Batt_age_level %d doesn't exist, using %d\n",
				chip->batt_age_level, avail_age_level);
			chip->batt_age_level = avail_age_level;
		}

		if (!chip->sp)
			chip->sp = devm_kzalloc(fg->dev, sizeof(*chip->sp),
						GFP_KERNEL);
		if (!chip->sp)
			return -ENOMEM;

		if (!chip->sp->initialized) {
			chip->sp->batt_id_kohms = fg->batt_id_ohms / 1000;
			chip->sp->last_batt_age_level = chip->batt_age_level;
			chip->sp->bp_node = batt_node;
			chip->sp->bms_psy = fg->fg_psy;
			rc = soh_profile_init(fg->dev, chip->sp);
			if (rc < 0) {
				devm_kfree(fg->dev, chip->sp);
				chip->sp = NULL;
			} else {
				fg_dbg(fg, FG_STATUS, "SOH profile count: %d\n",
					chip->sp->profile_count);
			}
		}
	}

	data = of_get_property(profile_node, "qcom,fg-profile-data", &len);
	if (!data) {
		pr_err("No profile data available\n");
		return -ENODATA;
	}

	if (len != PROFILE_LEN) {
		pr_err("battery profile incorrect size: %d\n", len);
		return -EINVAL;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	mutex_lock(&fg->step_lock);
	fg->step_en = false;
	rc = of_property_count_elems_of_size(profile_node,
					"somc,step-cfg", sizeof(u32));
	if (rc < 0) {
		pr_err("Can't get size of somc,step-cfg %d\n", rc);
		goto step_err;
	} else {
		step_len = rc;
		fg_dbg(fg, FG_STEP, "size of somc,step-cfg: %d\n", step_len);
	}
	if (step_len > STEP_DATA_DT_MAX_NUM || (step_len % STEP_DATA_RAW)) {
		pr_err("step config table size is invalid\n");
		goto step_err;
	} else {
		rc = of_property_read_u32_array(profile_node, "somc,step-cfg",
							&step_buf[0], step_len);
		if (rc < 0) {
			pr_err("Can't get somc,step-cfg %d\n", rc);
			goto step_err;
		}
		num = 0;
		for (i = 0; i < step_len; i += STEP_DATA_RAW) {
			fg->step_data.temp_low[num] = step_buf[i];
			fg->step_data.temp_high[num] = step_buf[i + 1];
			fg->step_data.voltage_low[num] = step_buf[i + 2];
			fg->step_data.voltage_high[num] = step_buf[i + 3];
			fg->step_data.target_current[num] = step_buf[i + 4];
			fg->step_data.target_voltage[num] = step_buf[i + 5];
			fg->step_data.condition[num] = step_buf[i + 6];
			num++;
		}
		fg->step_data.data_num = num;

		/* Check Step Data */
		fg_dbg(fg, FG_SOMC, "No |  Range of Temp  | Range of VBATT  | Target FCC/FV Condition\n");
		for (i = 0; i < fg->step_data.data_num; i++)
			fg_dbg(fg, FG_SOMC,
				"%2d | %6d - %6d | %6d - %6d | %6d %4d %d\n",
				i + 1,
				fg->step_data.temp_low[i],
				fg->step_data.temp_high[i],
				fg->step_data.voltage_low[i],
				fg->step_data.voltage_high[i],
				fg->step_data.target_current[i],
				fg->step_data.target_voltage[i],
				fg->step_data.condition[i]);

		fg->step_en = true;
	}
	rc = of_property_read_u32(profile_node, "somc,step-cell-impedance-mohm",
						&fg->cell_impedance_mohm);
	if (rc < 0) {
		pr_err("Can't get battery cell impedance %d\n", rc);
		fg->cell_impedance_mohm = -EINVAL;
	}
step_err:
	mutex_unlock(&fg->step_lock);

#endif
	fg->profile_available = true;
	memcpy(chip->batt_profile, data, len);

	/* Read all other DT properties after getting profile */
	rc = fg_gen4_get_batt_profile_dt_props(chip, profile_node);
	if (rc < 0)
		return rc;

	return 0;
}

static int fg_gen4_bp_params_config(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, i;
	u8 buf, therm_coeffs[BATT_THERM_NUM_COEFFS * 2];
	u8 rslow_coeffs[RSLOW_NUM_COEFFS], val, mask;
	u16 rslow_scalefn;

	if (fg->bp.vbatt_full_mv > 0) {
		rc = fg_set_constant_chg_voltage(fg,
				fg->bp.vbatt_full_mv * 1000);
		if (rc < 0)
			return rc;
	}

	if (fg->bp.therm_coeffs) {
		/* Each coefficient is a 16-bit value */
		for (i = 0; i < BATT_THERM_NUM_COEFFS; i++) {
			therm_coeffs[i*2] = fg->bp.therm_coeffs[i] & 0xFF;
			therm_coeffs[i*2 + 1] = fg->bp.therm_coeffs[i] >> 8;
		}
		rc = fg_sram_write(fg, BATT_THERM_COEFF_WORD,
			BATT_THERM_COEFF_OFFSET, therm_coeffs,
			BATT_THERM_NUM_COEFFS * 2, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing therm-coeffs, rc=%d\n", rc);
			return rc;
		}

		buf = fg->bp.therm_ctr_offset;
		rc = fg_sram_write(fg, BATT_THERM_CONFIG_WORD,
			RATIO_CENTER_OFFSET, &buf, 1, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing therm-ctr-offset, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (fg->bp.rslow_normal_coeffs && fg->bp.rslow_low_coeffs) {
		rc = fg_sram_read(fg, RSLOW_COEFF_DISCHG_WORD,
				RSLOW_COEFF_LOW_OFFSET, rslow_coeffs,
				RSLOW_NUM_COEFFS, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Failed to read RLOW_COEFF_DISCHG_WORD rc=%d\n",
				rc);
			return rc;
		}

		/* Read Rslow coefficients back and set the status */
		for (i = 0; i < RSLOW_NUM_COEFFS; i++) {
			buf = fg->bp.rslow_low_coeffs[i] & 0xFF;
			if (rslow_coeffs[i] == buf) {
				chip->rslow_low = true;
			} else {
				chip->rslow_low = false;
				break;
			}
		}
		fg_dbg(fg, FG_STATUS, "Rslow_low: %d\n", chip->rslow_low);
	}

	/*
	 * Since this SRAM word falls inside profile region, configure it after
	 * the profile is loaded. This parameter doesn't come from battery
	 * profile DT property.
	 */
	if (fg->wa_flags & PM8150B_V1_RSLOW_COMP_WA) {
		val = 0;
		mask = BIT(1);
		rc = fg_sram_masked_write(fg, RSLOW_CONFIG_WORD,
				RSLOW_CONFIG_OFFSET, mask, val, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing RSLOW_CONFIG_WORD, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (fg->wa_flags & PM8150B_V2_RSLOW_SCALE_FN_WA) {
		rslow_scalefn = 0x4000;
		rc = fg_sram_write(fg, RSLOW_SCALE_FN_CHG_V2_WORD,
				RSLOW_SCALE_FN_CHG_V2_OFFSET,
				(u8 *)&rslow_scalefn, 2, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing RSLOW_SCALE_FN_CHG_WORD rc=%d\n",
				rc);
			return rc;
		}

		rc = fg_sram_write(fg, RSLOW_SCALE_FN_DISCHG_V2_WORD,
				RSLOW_SCALE_FN_DISCHG_V2_OFFSET,
				(u8 *)&rslow_scalefn, 2, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing RSLOW_SCALE_FN_DISCHG_WORD rc=%d\n",
				rc);
			return rc;
		}
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	val = fg->batt_aging_level;
	rc = fg_sram_write(fg, SOMC_AGING_LEVEL_WORD, SOMC_AGING_LEVEL_OFFSET,
					&val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing batt_aging_level, rc=%d\n", rc);
		return rc;
	}

#endif
	if (fg->bp.therm_pull_up_kohms > 0) {
		switch (fg->bp.therm_pull_up_kohms) {
		case 30:
			buf = BATT_THERM_PULL_UP_30K;
			break;
		case 100:
			buf = BATT_THERM_PULL_UP_100K;
			break;
		case 400:
			buf = BATT_THERM_PULL_UP_400K;
			break;
		default:
			return -EINVAL;
		}

		rc = fg_masked_write(fg, ADC_RR_BATT_THERM_BASE_CFG1(fg),
					BATT_THERM_PULL_UP_MASK, buf);
		if (rc < 0) {
			pr_err("failed to write to 0x%04X, rc=%d\n",
				ADC_RR_BATT_THERM_BASE_CFG1(fg), rc);
			return rc;
		}
	}

	return 0;
}

static void clear_battery_profile(struct fg_dev *fg)
{
	u8 val = 0;
	int rc;

	rc = fg_sram_write(fg, PROFILE_INTEGRITY_WORD,
			PROFILE_INTEGRITY_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (rc < 0)
		pr_err("failed to write profile integrity rc=%d\n", rc);
}

#define PROFILE_LOAD_BIT	BIT(0)
#define BOOTLOADER_LOAD_BIT	BIT(1)
#define BOOTLOADER_RESTART_BIT	BIT(2)
#define HLOS_RESTART_BIT	BIT(3)
#define FIRST_PROFILE_LOAD_BIT	BIT(4)
static bool is_profile_load_required(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	u8 buf[PROFILE_COMP_LEN], val;
	bool profiles_same = false, valid_integrity = false;
	int rc, i;
	u8 white_list_values[] = {
		HLOS_RESTART_BIT,
		BOOTLOADER_LOAD_BIT,
		BOOTLOADER_LOAD_BIT | BOOTLOADER_RESTART_BIT,
		BOOTLOADER_RESTART_BIT | HLOS_RESTART_BIT,
		BOOTLOADER_LOAD_BIT | FIRST_PROFILE_LOAD_BIT,
		BOOTLOADER_LOAD_BIT | BOOTLOADER_RESTART_BIT |
			FIRST_PROFILE_LOAD_BIT,
		HLOS_RESTART_BIT | BOOTLOADER_RESTART_BIT |
			FIRST_PROFILE_LOAD_BIT,
	};

	if (chip->dt.multi_profile_load &&
		(chip->batt_age_level != chip->last_batt_age_level))
		return true;

	rc = fg_sram_read(fg, PROFILE_INTEGRITY_WORD,
			PROFILE_INTEGRITY_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("failed to read profile integrity rc=%d\n", rc);
		return false;
	}

	/* Check if integrity bit is set */
	if (val & PROFILE_LOAD_BIT) {
		fg_dbg(fg, FG_STATUS, "Battery profile integrity bit is set\n");

		/* Whitelist the values */
		val &= ~PROFILE_LOAD_BIT;
		for (i = 0; i < ARRAY_SIZE(white_list_values); i++)  {
			if (val == white_list_values[i]) {
				valid_integrity = true;
				break;
			}
		}

		if (!valid_integrity) {
			val |= PROFILE_LOAD_BIT;
			pr_warn("Garbage value in profile integrity word: 0x%x\n",
				val);
			return true;
		}

		rc = fg_sram_read(fg, PROFILE_LOAD_WORD, PROFILE_LOAD_OFFSET,
				buf, PROFILE_COMP_LEN, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in reading battery profile, rc:%d\n", rc);
			fg->profile_load_status = PROFILE_SKIPPED;
			return false;
		}
		profiles_same = memcmp(chip->batt_profile, buf,
					PROFILE_COMP_LEN) == 0;
		if (profiles_same) {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
			fg_dbg(fg, FG_STATUS, "Battery profile is same, not loading it\n");
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
			fg_dbg(fg, FG_SOMC, "Battery profile is same, not loading it\n");
#endif
			fg->profile_load_status = PROFILE_LOADED;
			return false;
		}

		if (!chip->dt.force_load_profile) {
			pr_warn("Profiles doesn't match, skipping loading it since force_load_profile is disabled\n");
			if (fg_profile_dump) {
				pr_info("FG: loaded profile:\n");
				dump_sram(fg, buf, PROFILE_LOAD_WORD,
					PROFILE_COMP_LEN);
				pr_info("FG: available profile:\n");
				dump_sram(fg, chip->batt_profile,
					PROFILE_LOAD_WORD, PROFILE_LEN);
			}
			fg->profile_load_status = PROFILE_SKIPPED;
			return false;
		}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_STATUS, "Profiles are different, loading the correct one\n");
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_SOMC, "Profiles are different, loading the correct one\n");
#endif
	} else {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_STATUS, "Profile integrity bit is not set\n");
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_SOMC, "Profile integrity bit is not set\n");
#endif
		if (fg_profile_dump) {
			pr_info("FG: profile to be loaded:\n");
			dump_sram(fg, chip->batt_profile, PROFILE_LOAD_WORD,
				PROFILE_LEN);
		}
	}
	return true;
}

#define SOC_READY_WAIT_TIME_MS	1000
static int qpnp_fg_gen4_load_profile(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	u8 val, mask, buf[2];
	int rc;
	bool normal_profile_load = false;

	/*
	 * This is used to determine if the profile is loaded normally i.e.
	 * either multi profile loading is disabled OR if it is enabled, then
	 * only loading the profile with battery age level 0 is considered.
	 */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	normal_profile_load = !chip->dt.multi_profile_load ||
				(chip->dt.multi_profile_load &&
					chip->batt_age_level == 0);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->requested_batt_aging_level != NO_REQUESTED_LEVEL) {
		normal_profile_load = false;
		fg_dbg(fg, FG_SOMC, "Skip side loading and soc restart %d\n",
						fg->requested_batt_aging_level);
	} else {
		normal_profile_load = true;
		fg_dbg(fg, FG_SOMC, "Execute side loading and soc restart\n");
	}
#endif
	if (normal_profile_load) {
		rc = fg_masked_write(fg, BATT_SOC_RESTART(fg), RESTART_GO_BIT,
					0);
		if (rc < 0) {
			pr_err("Error in writing to %04x, rc=%d\n",
				BATT_SOC_RESTART(fg), rc);
			return rc;
		}
	}

	/* load battery profile */
	rc = fg_sram_write(fg, PROFILE_LOAD_WORD, PROFILE_LOAD_OFFSET,
			chip->batt_profile, PROFILE_LEN, FG_IMA_ATOMIC);
	if (rc < 0) {
		pr_err("Error in writing battery profile, rc:%d\n", rc);
		return rc;
	}

	if (normal_profile_load) {
		/* Enable side loading for voltage and current */
		val = mask = BIT(0);
		rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
				SYS_CONFIG_OFFSET, mask, val, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in setting SYS_CONFIG_WORD[0], rc=%d\n",
				rc);
			return rc;
		}

		/* Clear first logged main current ADC values */
		buf[0] = buf[1] = 0;
		rc = fg_sram_write(fg, FIRST_LOG_CURRENT_v2_WORD,
				FIRST_LOG_CURRENT_v2_OFFSET, buf, 2,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in clearing FIRST_LOG_CURRENT rc=%d\n",
				rc);
			return rc;
		}
	}

	/* Set the profile integrity bit */
	val = HLOS_RESTART_BIT | PROFILE_LOAD_BIT;
	rc = fg_sram_write(fg, PROFILE_INTEGRITY_WORD,
			PROFILE_INTEGRITY_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("failed to write profile integrity rc=%d\n", rc);
		return rc;
	}

	if (chip->dt.multi_profile_load && chip->batt_age_level >= 0) {
		val = (u8)chip->batt_age_level;
		rc = fg_sram_write(fg, BATT_AGE_LEVEL_WORD,
				BATT_AGE_LEVEL_OFFSET, &val, 1, FG_IMA_ATOMIC);
		if (rc < 0) {
			pr_err("Error in writing batt_age_level, rc:%d\n", rc);
			return rc;
		}
	}

	if (normal_profile_load) {
		chip->last_restart_time = ktime_get();
		rc = fg_restart(fg, SOC_READY_WAIT_TIME_MS);
		if (rc < 0) {
			pr_err("Error in restarting FG, rc=%d\n", rc);
			return rc;
		}

		/* Clear side loading for voltage and current */
		val = 0;
		mask = BIT(0);
		rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
				SYS_CONFIG_OFFSET, mask, val, FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in clearing SYS_CONFIG_WORD[0], rc=%d\n",
				rc);
			return rc;
		}
	}

	return 0;
}

static bool is_sdam_cookie_set(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	u8 cookie;

	rc = nvmem_device_read(chip->fg_nvmem, SDAM_COOKIE_OFFSET, 1,
				&cookie);
	if (rc < 0) {
		pr_err("Error in reading SDAM_COOKIE rc=%d\n", rc);
		return false;
	}

	fg_dbg(fg, FG_STATUS, "cookie: %x\n", cookie);
	return (cookie == SDAM_COOKIE);
}

static void fg_gen4_clear_sdam(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	u8 buf[SDAM_FG_PARAM_LENGTH] = { 0 };
	int rc;

	/*
	 * Clear all bytes of SDAM used to store FG parameters when it is first
	 * profile load so that the junk values would not be used.
	 */
	rc = nvmem_device_write(chip->fg_nvmem, SDAM_CYCLE_COUNT_OFFSET,
			SDAM_FG_PARAM_LENGTH, buf);
	if (rc < 0)
		pr_err("Error in clearing SDAM rc=%d\n", rc);
	else
		fg_dbg(fg, FG_STATUS, "Cleared SDAM\n");
}

static void fg_gen4_post_profile_load(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc, act_cap_mah;
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u8 buf[16] = {0};

	if (chip->dt.multi_profile_load &&
		chip->batt_age_level != chip->last_batt_age_level) {

		/* Keep ESR fast calib config disabled */
		fg_gen4_esr_fast_calib_config(chip, false);
		chip->esr_fast_calib = false;

		mutex_lock(&chip->esr_calib_lock);

		rc = fg_sram_write(fg, ESR_DELTA_DISCHG_WORD,
				ESR_DELTA_DISCHG_OFFSET, buf, 2,
				FG_IMA_DEFAULT);
		if (rc < 0)
			pr_err("Error in writing ESR_DELTA_DISCHG, rc=%d\n",
				rc);

		rc = fg_sram_write(fg, ESR_DELTA_CHG_WORD, ESR_DELTA_CHG_OFFSET,
				buf, 2, FG_IMA_DEFAULT);
		if (rc < 0)
			pr_err("Error in writing ESR_DELTA_CHG, rc=%d\n", rc);

		mutex_unlock(&chip->esr_calib_lock);
	}
#endif

	/* If SDAM cookie is not set, read back from SRAM and load it in SDAM */
	if (chip->fg_nvmem && !is_sdam_cookie_set(chip)) {
		fg_gen4_clear_sdam(chip);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		rc = fg_sram_read(&chip->fg, CYCLE_COUNT_WORD,
					CYCLE_COUNT_OFFSET, buf, 16,
					FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in reading cycle counters from SRAM rc=%d\n",
				rc);
		} else {
			rc = nvmem_device_write(chip->fg_nvmem,
				SDAM_CYCLE_COUNT_OFFSET, 16, (u8 *)buf);
			if (rc < 0)
				pr_err("Error in writing cycle counters to SDAM rc=%d\n",
					rc);
		}

#endif
		rc = fg_get_sram_prop(fg, FG_SRAM_ACT_BATT_CAP, &act_cap_mah);
		if (rc < 0) {
			pr_err("Error in getting learned capacity, rc=%d\n",
				rc);
		} else {
			rc = nvmem_device_write(chip->fg_nvmem,
				SDAM_CAP_LEARN_OFFSET, 2, (u8 *)&act_cap_mah);
			if (rc < 0)
				pr_err("Error in writing learned capacity to SDAM, rc=%d\n",
					rc);
		}
	}

	/* Restore the cycle counters so that it would be valid at this point */
	rc = restore_cycle_count(chip->counter);
	if (rc < 0)
		pr_err("Error in restoring cycle_count, rc=%d\n", rc);

}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_configure_full_soc(struct fg_dev *fg, int bsoc);
#define SOC_16BIT		GENMASK(15, 0)
#endif
#if defined(CONFIG_SOMC_CHARGER_XFL)
#define DUMMY_CELL_IMPEDANCE_MOHM 0
#define DUMMY_STEP_DATA_MAX_NUM 5
#define DUMMY_FASTCHG_CURRENT_MA 1500
#define DUMMY_FLOAT_VOLTAGE_UV 4400000
static void set_dummy_profile_data(struct fg_dev *fg)
{
	int temp_low[DUMMY_STEP_DATA_MAX_NUM] = {-2550, 50, 120, 450, 550};
	int temp_high[DUMMY_STEP_DATA_MAX_NUM] = {50, 120, 450, 550, 2550};
	int voltage_low[DUMMY_STEP_DATA_MAX_NUM] = {0, 0, 0, 0, 0};
	int voltage_high[DUMMY_STEP_DATA_MAX_NUM] = {4500, 4500, 4500, 4500, 4500};
	int target_current[DUMMY_STEP_DATA_MAX_NUM] = {0, 500, 1500, 500, 0};
	int target_voltage[DUMMY_STEP_DATA_MAX_NUM] = {4400, 4400, 4400, 4150, 4150};
	int condition[DUMMY_STEP_DATA_MAX_NUM] = {1, 2, 3, 4, 5};
	int i;

	fg->bp.float_volt_uv = DUMMY_FLOAT_VOLTAGE_UV;
	fg->bp.fastchg_curr_ma = DUMMY_FASTCHG_CURRENT_MA;
	fg->cell_impedance_mohm = DUMMY_CELL_IMPEDANCE_MOHM;
	fg->step_data.data_num = DUMMY_STEP_DATA_MAX_NUM;
	for (i = 0; i < DUMMY_STEP_DATA_MAX_NUM; i++) {
		fg->step_data.temp_low[i] = temp_low[i];
		fg->step_data.temp_high[i] = temp_high[i];
		fg->step_data.voltage_low[i] = voltage_low[i];
		fg->step_data.voltage_high[i] = voltage_high[i];
		fg->step_data.target_current[i] = target_current[i];
		fg->step_data.target_voltage[i] = target_voltage[i];
		fg->step_data.condition[i] = condition[i];
	}

	fg_dbg(fg, FG_SOMC, "No |  Range of Temp  | Range of VBATT  | Target FCC/FV Condition\n");
	for (i = 0; i < DUMMY_STEP_DATA_MAX_NUM; i++) {
		fg_dbg(fg, FG_SOMC,
			"%2d | %6d - %6d | %6d - %6d | %6d %4d %d\n",
			i + 1,
			fg->step_data.temp_low[i],
			fg->step_data.temp_high[i],
			fg->step_data.voltage_low[i],
			fg->step_data.voltage_high[i],
			fg->step_data.target_current[i],
			fg->step_data.target_voltage[i],
			fg->step_data.condition[i]);
	}
}
#endif
static void profile_load_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work,
				struct fg_dev,
				profile_load_work.work);
	struct fg_gen4_chip *chip = container_of(fg,
				struct fg_gen4_chip, fg);
	int64_t nom_cap_uah, learned_cap_uah = 0;
	u8 val, buf[2];
	int rc;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u64 temp;
	int nom_cap_temp_mah = 0, nom_cap_mah = 0;
	int charge_done = 0;
	int vbatt_uv = 0;
#endif

	vote(fg->awake_votable, PROFILE_LOAD, true, 0);

	rc = fg_gen4_get_batt_id(chip);
	if (rc < 0) {
		pr_err("Error in getting battery id, rc:%d\n", rc);
		goto out;
	}

#if defined(CONFIG_SOMC_CHARGER_XFL)
	pr_warn("Skip profile load by CONFIG_SOMC_CHARGER_XFL=y\n");
	fg->profile_load_status = PROFILE_LOADED;
	set_dummy_profile_data(fg);
	fg->step_en = true;
	fg->profile_available = true;
	batt_psy_initialized(fg);
	goto out;

#endif
	rc = fg_gen4_get_batt_profile(fg);
	if (rc < 0) {
		fg->profile_load_status = PROFILE_MISSING;
		pr_warn("profile for batt_id=%dKOhms not found..using OTP, rc:%d\n",
			fg->batt_id_ohms / 1000, rc);
		goto out;
	}

	if (!fg->profile_available)
		goto out;

	if (!is_profile_load_required(chip))
		goto done;

	if (!chip->dt.multi_profile_load) {
		clear_cycle_count(chip->counter);
		if (chip->fg_nvmem && !is_sdam_cookie_set(chip))
			fg_gen4_clear_sdam(chip);
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	charge_done = fg->charge_done;

	rc = fg_gen4_get_nominal_capacity(chip, &temp);
	if (rc < 0) {
		pr_err("Error in getting nominal_capacity, rc:%d\n", rc);
		goto out;
	}
	nom_cap_temp_mah = temp / 1000;
#endif

	fg_dbg(fg, FG_STATUS, "profile loading started\n");

	if (chip->dt.multi_profile_load &&
		chip->batt_age_level != chip->last_batt_age_level) {
		rc = fg_gen4_get_learned_capacity(chip, &learned_cap_uah);
		if (rc < 0)
			pr_err("Error in getting learned capacity rc=%d\n", rc);
		else
			fg_dbg(fg, FG_STATUS, "learned capacity: %lld uAh\n",
				learned_cap_uah);
	}

	rc = qpnp_fg_gen4_load_profile(chip);
	if (rc < 0)
		goto out;

	fg_dbg(fg, FG_STATUS, "SOC is ready\n");
	fg->profile_load_status = PROFILE_LOADED;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->requested_batt_aging_level != NO_REQUESTED_LEVEL) {
		fg->batt_aging_level = fg->requested_batt_aging_level;

		rc = fg_gen4_get_learned_capacity(chip, &learned_cap_uah);
		if (rc < 0)
			pr_err("Error in getting learned capacity rc=%d\n", rc);
		else
			fg_dbg(fg, FG_STATUS, "learned capacity: %lld uAh\n",
				learned_cap_uah);
	}
#endif
	if (fg->wa_flags & PM8150B_V1_DMA_WA)
		msleep(1000);

	if (learned_cap_uah == 0) {
		/*
		 * Whenever battery profile is loaded, read nominal capacity and
		 * write it to actual (or aged) capacity as it is outside the
		 * profile region and might contain OTP values. learned_cap_uah
		 * would have non-zero value if multiple profile loading is
		 * enabled and a profile got loaded already.
		 */
		rc = fg_sram_read(fg, NOM_CAP_WORD, NOM_CAP_OFFSET, buf, 2,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in reading %04x[%d] rc=%d\n",
				NOM_CAP_WORD, NOM_CAP_OFFSET, rc);
		} else {
			nom_cap_uah = (buf[0] | buf[1] << 8) * 1000;
			rc = fg_gen4_store_learned_capacity(chip, nom_cap_uah);
			if (rc < 0)
				pr_err("Error in writing to ACT_BATT_CAP rc=%d\n",
					rc);
		}
	}
done:
	rc = fg_sram_read(fg, PROFILE_INTEGRITY_WORD,
			PROFILE_INTEGRITY_OFFSET, &val, 1, FG_IMA_DEFAULT);
	if (!rc && (val & FIRST_PROFILE_LOAD_BIT)) {
		fg_dbg(fg, FG_STATUS, "First profile load bit is set\n");
		chip->first_profile_load = true;
	}

	fg_gen4_post_profile_load(chip);

	rc = fg_gen4_bp_params_config(fg);
	if (rc < 0)
		pr_err("Error in configuring battery profile params, rc:%d\n",
			rc);

	rc = fg_gen4_get_nominal_capacity(chip, &nom_cap_uah);
	if (!rc) {
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		nom_cap_mah = nom_cap_uah / 1000;
#endif
		rc = cap_learning_post_profile_init(chip->cl, nom_cap_uah);
		if (rc < 0)
			pr_err("Error in cap_learning_post_profile_init rc=%d\n",
				rc);
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (fg->requested_batt_aging_level != NO_REQUESTED_LEVEL) {
		rc = fg_get_battery_voltage(fg, &vbatt_uv);
		if (rc < 0)
			pr_err("Error in get_battery_voltage rc=%d\n", rc);

		if (vbatt_uv > fg->bp.float_volt_uv) {
			fg->fv_rb_en = 1;
			fg_dbg(fg, FG_SOMC, "Start FV_RB, vbatt:%d, fv:%d\n",
				vbatt_uv / 1000, fg->bp.float_volt_uv / 1000);
		} else {
			fg->fv_rb_en = 0;
			fg_dbg(fg, FG_SOMC, "Don't start FV_RB, vbatt:%d, fv:%d\n",
				vbatt_uv / 1000, fg->bp.float_volt_uv / 1000);
		}
		alarm_start_relative(&fg->somc_fv_rb_alarm_timer,
							ms_to_ktime(0));
	}
#endif
	batt_psy_initialized(fg);
	fg_notify_charger(fg);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (chip->cl->active)
		cap_learning_abort(chip->cl);

	if (fg->temp_corr_en)
		schedule_delayed_work(&fg->somc_temp_corr_work,
						msecs_to_jiffies(10000));
#endif
	schedule_delayed_work(&chip->ttf->ttf_work, msecs_to_jiffies(10000));
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_STATUS, "profile loaded successfully");
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_SOMC, "profile loaded successfully");
#endif
out:
	if (!chip->esr_fast_calib || is_debug_batt_id(fg)) {
		/* If it is debug battery, then disable ESR fast calibration */
		fg_gen4_esr_fast_calib_config(chip, false);
		chip->esr_fast_calib = false;
	}

	if (chip->dt.multi_profile_load && rc < 0)
		chip->batt_age_level = chip->last_batt_age_level;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg->requested_batt_aging_level = NO_REQUESTED_LEVEL;
#endif
	fg->soc_reporting_ready = true;
	vote(fg->awake_votable, ESR_FCC_VOTER, true, 0);
	schedule_delayed_work(&chip->pl_enable_work, msecs_to_jiffies(5000));
	vote(fg->awake_votable, PROFILE_LOAD, false, 0);
	if (!work_pending(&fg->status_change_work)) {
		pm_stay_awake(fg->dev);
		schedule_work(&fg->status_change_work);
	}

	rc = fg_gen4_validate_soc_scale_mode(chip);
	if (rc < 0)
		pr_err("Failed to validate SOC scale mode, rc=%d\n", rc);
}

static void get_batt_psy_props(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	union power_supply_propval prop = {0, };
	int rc;

	if (!batt_psy_initialized(fg))
		return;

	rc = power_supply_get_property(fg->batt_psy, POWER_SUPPLY_PROP_STATUS,
			&prop);
	if (rc < 0) {
		pr_err("Error in getting charging status, rc=%d\n", rc);
		return;
	}

	fg->charge_status = prop.intval;
	rc = power_supply_get_property(fg->batt_psy,
			POWER_SUPPLY_PROP_CHARGE_TYPE, &prop);
	if (rc < 0) {
		pr_err("Error in getting charge type, rc=%d\n", rc);
		return;
	}

	fg->charge_type = prop.intval;
	rc = power_supply_get_property(fg->batt_psy,
			POWER_SUPPLY_PROP_CHARGE_DONE, &prop);
	if (rc < 0) {
		pr_err("Error in getting charge_done, rc=%d\n", rc);
		return;
	}

	fg->charge_done = prop.intval;
	rc = power_supply_get_property(fg->batt_psy, POWER_SUPPLY_PROP_HEALTH,
		&prop);
	if (rc < 0) {
		pr_err("Error in getting battery health, rc=%d\n", rc);
		return;
	}

	fg->health = prop.intval;

	if (!chip->recharge_soc_thr) {
		rc = power_supply_get_property(fg->batt_psy,
			POWER_SUPPLY_PROP_RECHARGE_SOC, &prop);
		if (rc < 0) {
			pr_err("Error in getting recharge SOC, rc=%d\n", rc);
			return;
		}

		if (prop.intval < 0)
			pr_debug("Recharge SOC not configured %d\n",
				prop.intval);
		else
			chip->recharge_soc_thr = prop.intval;
	}
}

static int fg_gen4_esr_soh_update(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, msoc, esr_uohms, tmp;

	if (!fg->soc_reporting_ready || fg->battery_missing) {
		chip->esr_actual = -EINVAL;
		chip->esr_nominal = -EINVAL;
		return 0;
	}

	rc = get_cycle_count(chip->counter, &tmp);
	if (rc < 0)
		pr_err("Couldn't get cycle count rc=%d\n", rc);
	else if (tmp != chip->esr_soh_cycle_count)
		chip->esr_soh_notified = false;

	if (fg->charge_status != POWER_SUPPLY_STATUS_CHARGING ||
			chip->esr_soh_notified)
		return 0;

	rc = fg_get_msoc(fg, &msoc);
	if (rc < 0) {
		pr_err("Error in getting msoc, rc=%d\n", rc);
		return rc;
	}

	if (msoc != ESR_SOH_SOC) {
		fg_dbg(fg, FG_STATUS, "msoc: %d, not publishing ESR params\n",
			msoc);
		return 0;
	}

	rc = fg_get_sram_prop(fg, FG_SRAM_ESR_ACT, &esr_uohms);
	if (rc < 0) {
		pr_err("Error in getting esr_actual, rc=%d\n",
			rc);
		return rc;
	}
	chip->esr_actual = esr_uohms;

	rc = fg_get_sram_prop(fg, FG_SRAM_ESR_MDL, &esr_uohms);
	if (rc < 0) {
		pr_err("Error in getting esr_nominal, rc=%d\n",
			rc);
		chip->esr_actual = -EINVAL;
		return rc;
	}
	chip->esr_nominal = esr_uohms;

	fg_dbg(fg, FG_STATUS, "esr_actual: %d esr_nominal: %d\n",
		chip->esr_actual, chip->esr_nominal);

	if (fg->batt_psy)
		power_supply_changed(fg->batt_psy);

	rc = get_cycle_count(chip->counter, &chip->esr_soh_cycle_count);
	if (rc < 0)
		pr_err("Couldn't get cycle count rc=%d\n", rc);

	chip->esr_soh_notified = true;

	return 0;
}

static int fg_gen4_update_maint_soc(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc = 0, msoc;

	if (!chip->dt.linearize_soc)
		return 0;

	mutex_lock(&fg->charge_full_lock);
	if (fg->delta_soc <= 0)
		goto out;

	rc = fg_get_msoc(fg, &msoc);
	if (rc < 0) {
		pr_err("Error in getting msoc, rc=%d\n", rc);
		goto out;
	}

	if (msoc >= fg->maint_soc) {
		/*
		 * When the monotonic SOC goes above maintenance SOC, we should
		 * stop showing the maintenance SOC.
		 */
		fg->delta_soc = 0;
		fg->maint_soc = 0;
	} else if (fg->maint_soc && msoc < fg->last_msoc) {
		/* MSOC is decreasing. Decrease maintenance SOC as well */
		fg->maint_soc -= 1;
		if (!(msoc % 10)) {
			/*
			 * Reduce the maintenance SOC additionally by 1 whenever
			 * it crosses a SOC multiple of 10.
			 */
			fg->maint_soc -= 1;
			fg->delta_soc -= 1;
		}
	}

	fg_dbg(fg, FG_STATUS, "msoc: %d last_msoc: %d maint_soc: %d delta_soc: %d\n",
		msoc, fg->last_msoc, fg->maint_soc, fg->delta_soc);
	fg->last_msoc = msoc;
out:
	mutex_unlock(&fg->charge_full_lock);
	return rc;
}

static int fg_gen4_configure_full_soc(struct fg_dev *fg, int bsoc)
{
	int rc;
	u8 full_soc[2] = {0xFF, 0xFF}, buf[2];

	/*
	 * Once SOC masking condition is cleared, FULL_SOC and MONOTONIC_SOC
	 * needs to be updated to reflect the same. Write battery SOC to
	 * FULL_SOC and write a full value to MONOTONIC_SOC.
	 */
	fg_encode(fg->sp, FG_SRAM_FULL_SOC, bsoc, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_FULL_SOC].addr_word,
			fg->sp[FG_SRAM_FULL_SOC].addr_byte, buf,
			fg->sp[FG_SRAM_FULL_SOC].len, FG_IMA_ATOMIC);
	if (rc < 0) {
		pr_err("failed to write full_soc rc=%d\n", rc);
		return rc;
	}

	rc = fg_sram_write(fg, fg->sp[FG_SRAM_MONOTONIC_SOC].addr_word,
			fg->sp[FG_SRAM_MONOTONIC_SOC].addr_byte, full_soc,
			fg->sp[FG_SRAM_MONOTONIC_SOC].len, FG_IMA_ATOMIC);
	if (rc < 0) {
		pr_err("failed to write monotonic_soc rc=%d\n", rc);
		return rc;
	}

	return 0;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int fg_gen4_set_recharge_soc(struct fg_dev *fg, int recharge_soc)
{
	union power_supply_propval prop = {0, };
	int rc;

	if (recharge_soc < 0 || recharge_soc > FULL_CAPACITY || !fg->batt_psy)
		return 0;

	prop.intval = recharge_soc;
	rc = power_supply_set_property(fg->batt_psy,
		POWER_SUPPLY_PROP_RECHARGE_SOC, &prop);
	if (rc < 0) {
		pr_err("Error in setting recharge SOC, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

#endif
static int fg_gen4_adjust_recharge_soc(struct fg_gen4_chip *chip)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct fg_dev *fg = &chip->fg;
	int rc, msoc, recharge_soc, new_recharge_soc = 0;
	bool recharge_soc_status;

	if (!chip->recharge_soc_thr)
		return 0;

	recharge_soc = chip->recharge_soc_thr;
	recharge_soc_status = fg->recharge_soc_adjusted;

	/*
	 * If the input is present and charging had been terminated, adjust
	 * the recharge SOC threshold based on the monotonic SOC at which
	 * the charge termination had happened.
	 */
	if (is_input_present(fg)) {
		if (fg->charge_done) {
			if (!fg->recharge_soc_adjusted) {
				/* Get raw monotonic SOC for calculation */
				rc = fg_get_msoc(fg, &msoc);
				if (rc < 0) {
					pr_err("Error in getting msoc, rc=%d\n",
						rc);
					return rc;
				}

				/* Adjust the recharge_soc threshold */
				new_recharge_soc = msoc - (FULL_CAPACITY -
								recharge_soc);
				fg->recharge_soc_adjusted = true;
				if (fg->health == POWER_SUPPLY_HEALTH_GOOD)
					chip->chg_term_good = true;
			} else {
				/*
				 * If charge termination happened properly then
				 * do nothing.
				 */
				if (chip->chg_term_good)
					return 0;

				if (fg->health != POWER_SUPPLY_HEALTH_GOOD)
					return 0;

				/*
				 * Device is out of JEITA. Restore back default
				 * threshold.
				 */

				new_recharge_soc = recharge_soc;
				fg->recharge_soc_adjusted = false;
				chip->chg_term_good = false;
			}
		} else {
			if (!fg->recharge_soc_adjusted)
				return 0;

			/*
			 * If we are here, then it means that recharge SOC
			 * had been adjusted already and it could be probably
			 * because of early termination. We shouldn't restore
			 * the original recharge SOC threshold if the health is
			 * not good, which means battery is in JEITA zone.
			 */
			if (fg->health != POWER_SUPPLY_HEALTH_GOOD)
				return 0;

			/* Restore the default value */
			new_recharge_soc = recharge_soc;
			fg->recharge_soc_adjusted = false;
			chip->chg_term_good = false;
		}
	} else {
		/* Restore the default value */
		new_recharge_soc = recharge_soc;
		fg->recharge_soc_adjusted = false;
		chip->chg_term_good = false;
	}

	if (recharge_soc_status == fg->recharge_soc_adjusted)
		return 0;

	rc = fg_gen4_set_recharge_soc(fg, new_recharge_soc);
	if (rc < 0) {
		fg->recharge_soc_adjusted = recharge_soc_status;
		pr_err("Couldn't set recharge SOC, rc=%d\n", rc);
		return rc;
	}

	fg_dbg(fg, FG_STATUS, "recharge soc set to %d\n", new_recharge_soc);
#endif
	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void fg_gen4_somc_charge_full_releasing_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev,
					somc_charge_full_releasing_work.work);
	fg_dbg(fg, FG_SOMC, "charge_full_releasing set to false\n");
	fg->charge_full_releasing = false;
}

#define CHARGE_FULL_REL_TIME 1000 /* 1 sec */
#endif
static int fg_gen4_charge_full_update(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	union power_supply_propval prop = {0, };
	int rc, msoc, bsoc, recharge_soc, msoc_raw;

	if (!chip->dt.hold_soc_while_full)
		return 0;

	if (!batt_psy_initialized(fg))
		return 0;

	mutex_lock(&fg->charge_full_lock);
	vote(fg->delta_bsoc_irq_en_votable, DELTA_BSOC_IRQ_VOTER,
		fg->charge_done, 0);
	rc = power_supply_get_property(fg->batt_psy,
		POWER_SUPPLY_PROP_RECHARGE_SOC, &prop);
	if (rc < 0) {
		pr_err("Error in getting recharge_soc, rc=%d\n", rc);
		goto out;
	}

	recharge_soc = prop.intval;
	recharge_soc = DIV_ROUND_CLOSEST(recharge_soc * FULL_SOC_RAW,
				FULL_CAPACITY);
	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &bsoc);
	if (rc < 0) {
		pr_err("Error in getting BATT_SOC, rc=%d\n", rc);
		goto out;
	}

	/* We need 2 most significant bytes here */
	bsoc = (u32)bsoc >> 16;
	rc = fg_get_msoc_raw(fg, &msoc_raw);
	if (rc < 0) {
		pr_err("Error in getting msoc_raw, rc=%d\n", rc);
		goto out;
	}
	msoc = DIV_ROUND_CLOSEST(msoc_raw * FULL_CAPACITY, FULL_SOC_RAW);

	fg_dbg(fg, FG_STATUS, "msoc: %d bsoc: %x health: %d status: %d full: %d\n",
		msoc, bsoc, fg->health, fg->charge_status,
		fg->charge_full);
	if (fg->charge_done && !fg->charge_full) {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (msoc >= 99 && fg->health == POWER_SUPPLY_HEALTH_GOOD) {
			fg_dbg(fg, FG_STATUS, "Setting charge_full to true\n");
			fg->charge_full = true;
		} else {
			fg_dbg(fg, FG_STATUS, "Terminated charging @ SOC%d\n",
				msoc);
		}
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (msoc > 99 && fg->health == POWER_SUPPLY_HEALTH_GOOD) {
			fg_dbg(fg, FG_SOMC, "Setting charge_full to true\n");
			fg->charge_full = true;
			fg->full_counter++;
		} else {
			fg_dbg(fg, FG_SOMC, "Charge terminated but msoc is enough %d\n",
				msoc);
		}
#endif
	} else if ((msoc_raw <= recharge_soc || !fg->charge_done)
			&& fg->charge_full) {
		if (chip->dt.linearize_soc) {
			fg->delta_soc = FULL_CAPACITY - msoc;

			/*
			 * We're spreading out the delta SOC over every 10%
			 * change in monotonic SOC. We cannot spread more than
			 * 9% in the range of 0-100 skipping the first 10%.
			 */
			if (fg->delta_soc > 9) {
				fg->delta_soc = 0;
				fg->maint_soc = 0;
			} else {
				fg->maint_soc = FULL_CAPACITY;
				fg->last_msoc = msoc;
			}
		}

		/*
		 * If charge_done is still set, wait for recharging or
		 * discharging to happen.
		 */
		if (fg->charge_done)
			goto out;

		rc = fg_gen4_configure_full_soc(fg, bsoc);
		if (rc < 0)
			goto out;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg_dbg(fg, FG_SOMC, "charge_full_releasing set to true\n");
		fg->charge_full_releasing = true;
		schedule_delayed_work(&fg->somc_charge_full_releasing_work,
					msecs_to_jiffies(CHARGE_FULL_REL_TIME));
		if (is_input_present(fg))
			fg->recharge_counter++;
#endif
		fg->charge_full = false;
		fg_dbg(fg, FG_STATUS, "msoc_raw = %d bsoc: %d recharge_soc: %d delta_soc: %d\n",
			msoc_raw, bsoc >> 8, recharge_soc, fg->delta_soc);
	}

out:
	mutex_unlock(&fg->charge_full_lock);
	return rc;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define FV_RB_REL_VBATT_THRESHOLD_UV	(100 * 1000) /* 100 mV */
#define FV_RB_REL_TIME_THRESHOLD_MS	(10 * 60 * 1000) /* 10 min */
static int fg_gen4_somc_fv_rb_update(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	union power_supply_propval prop = {0, };
	int rc = 0;
	int vbatt_uv;
	bool suspend;
	int current_ktime_ms, ktime_diff_ms;

	if (!fg->fv_rb_en && !fg->fv_rb_suspened)
		return 0;

	rc = fg_get_battery_voltage(fg, &vbatt_uv);
	if (rc < 0)
		pr_err("Error in get_battery_voltage rc=%d\n", rc);

	current_ktime_ms = ktime_to_ms(ktime_get_boottime());
	ktime_diff_ms = current_ktime_ms - fg->fv_rb_suspened_ktime_ms;

	fg_dbg(fg, FG_STATUS, "vbatt_uv:%d float_volt_uv:%d ktime_diff_ms:%d\n",
				vbatt_uv, fg->bp.float_volt_uv, ktime_diff_ms);

	if (vbatt_uv < fg->bp.float_volt_uv - FV_RB_REL_VBATT_THRESHOLD_UV &&
		fg->fv_rb_suspened_ktime_ms &&
		ktime_diff_ms > FV_RB_REL_TIME_THRESHOLD_MS) {
		fg->fv_rb_en = 0;
		fg_dbg(fg, FG_SOMC, "Set fg->fv_rb_en to 0. vbatt_uv:%d float_volt_uv:%d ktime_diff_ms:%d\n",
				vbatt_uv, fg->bp.float_volt_uv, ktime_diff_ms);
	}

	if (fg->fv_rb_en &&
		(fg->charge_status == POWER_SUPPLY_STATUS_CHARGING ||
		fg->charge_status == POWER_SUPPLY_STATUS_FULL)) {
		suspend = true;
	} else {
		suspend = false;
	}

	if (suspend != fg->fv_rb_suspened) {
		fg_dbg(fg, FG_SOMC, "suspend status is changed. suspend:%d, fg->fv_rb_en:%d fg->charge_status:%d\n",
				suspend, fg->fv_rb_en, fg->charge_status);
		fg_gen4_somc_suspend_slope_limit(chip, suspend);

		/* request icl to smb driver */
		if (batt_psy_initialized(fg)) {
			prop.intval = (int)suspend;
			rc = power_supply_set_property(fg->batt_psy,
					POWER_SUPPLY_PROP_PROFILE_FV_RB_EN,
					&prop);
			if (rc < 0)
				pr_err("Error in profile_fv_rb_en on batt_psy rc=%d\n",
									rc);

			fg_notify_charger(fg);
		}
		fg->fv_rb_suspened = suspend;

		if (suspend)
			fg->fv_rb_suspened_ktime_ms =
					ktime_to_ms(ktime_get_boottime());
	}
	return rc;
}

static enum alarmtimer_restart fg_somc_fv_rb_alarm_timer(struct alarm *alarm,
							ktime_t time)
{
	struct fg_dev *fg = container_of(alarm, struct fg_dev,
						somc_fv_rb_alarm_timer);

	fg_dbg(fg, FG_STATUS, "FV_RB timer expired\n");
	/*
	 * We cannot vote for awake votable here as that takes
	 * a mutex lock and this is executed in an atomic context.
	 */
	fg->fv_rb_awake = true;
	pm_stay_awake(fg->dev);
	schedule_work(&fg->somc_fv_rb_soc_work);

	return ALARMTIMER_NORESTART;
}

#define FV_RB_WORK_POL_TIME_MS	3000
#define STEP_UP_MSOC_RAW	32	/* about +0.05% */
#define STEP_UP_THR_MSOC_RAW	65207	/* about 99.5% */
static void fg_gen4_somc_fv_rb_soc_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev,
						somc_fv_rb_soc_work);
	int rc;
	int msoc_raw, inc_msoc_raw;
	bool reschedule = false;

	if (fg->fv_rb_awake)
		vote(fg->awake_votable, FV_RB_SOC, true, 0);

	fg_gen4_somc_fv_rb_update(fg);
	if (fg->fv_rb_en) {
		reschedule = true;
		fg_dbg(fg, FG_SOMC, "FV_RB is enabled. So, will re-schedule work\n");
	}

	rc = fg_get_sram_prop(fg, FG_SRAM_MONOTONIC_SOC, &msoc_raw);
	if (rc < 0) {
		pr_err("Error in getting MONOTONIC_SOC, rc=%d\n", rc);
		goto out;
	}
	fg_dbg(fg, FG_STATUS, "msoc_raw=%d, fg->charge_status=%d\n",
						msoc_raw, fg->charge_status);

	if (!fg->charge_full && fg->charge_status == POWER_SUPPLY_STATUS_FULL &&
					msoc_raw < STEP_UP_THR_MSOC_RAW) {
		if (fg->fv_rb_full_step_up_soc_raw)
			inc_msoc_raw = fg->fv_rb_full_step_up_soc_raw;
		else
			inc_msoc_raw = msoc_raw;

		inc_msoc_raw += STEP_UP_MSOC_RAW;
		if (inc_msoc_raw > 65535)
			inc_msoc_raw = 65535;

		rc = fg_sram_write(fg, fg->sp[FG_SRAM_MONOTONIC_SOC].addr_word,
				fg->sp[FG_SRAM_MONOTONIC_SOC].addr_byte,
				(u8 *)&inc_msoc_raw,
				fg->sp[FG_SRAM_MONOTONIC_SOC].len,
				FG_IMA_ATOMIC);
		if (rc < 0) {
			pr_err("failed to write monotonic_soc rc=%d\n", rc);
			goto out;
		}
		fg_dbg(fg, FG_SOMC, "msoc is increased to %d. So, will re-schedule work\n",
								inc_msoc_raw);
		reschedule = true;
		fg->fv_rb_full_step_up_soc_raw = inc_msoc_raw;
	} else {
		fg->fv_rb_full_step_up_soc_raw = 0;
	}

	if (reschedule)
		alarm_start_relative(&fg->somc_fv_rb_alarm_timer,
					ms_to_ktime(FV_RB_WORK_POL_TIME_MS));
out:
	vote(fg->awake_votable, FV_RB_SOC, false, 0);
}

#endif
static int fg_gen4_esr_fcc_config(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	union power_supply_propval prop = {0, };
	int rc;
	bool parallel_en = false, cp_en = false, qnovo_en, esr_fcc_ctrl_en;
	u8 val, mask;

	if (is_parallel_charger_available(fg)) {
		rc = power_supply_get_property(fg->parallel_psy,
			POWER_SUPPLY_PROP_CHARGING_ENABLED, &prop);
		if (rc < 0)
			pr_err_ratelimited("Error in reading charging_enabled from parallel_psy, rc=%d\n",
				rc);
		else
			parallel_en = prop.intval;
	}

	if (chip->cp_disable_votable)
		cp_en = !get_effective_result(chip->cp_disable_votable);

	qnovo_en = is_qnovo_en(fg);

	fg_dbg(fg, FG_POWER_SUPPLY, "chg_sts: %d par_en: %d cp_en: %d qnov_en: %d esr_fcc_ctrl_en: %d\n",
		fg->charge_status, parallel_en, cp_en, qnovo_en,
		chip->esr_fcc_ctrl_en);

	if (fg->charge_status == POWER_SUPPLY_STATUS_CHARGING &&
			(parallel_en || qnovo_en || cp_en)) {
		if (chip->esr_fcc_ctrl_en)
			return 0;

		/*
		 * When parallel charging or Qnovo or Charge pump is enabled,
		 * configure ESR FCC to 300mA to trigger an ESR pulse. Without
		 * this, FG can request the main charger to increase FCC when it
		 * is supposed to decrease it.
		 */
		val = GEN4_ESR_FCC_300MA << GEN4_ESR_FAST_CRG_IVAL_SHIFT |
			ESR_FAST_CRG_CTL_EN_BIT;
		esr_fcc_ctrl_en = true;
	} else {
		if (!chip->esr_fcc_ctrl_en)
			return 0;

		/*
		 * If we're here, then it means either the device is not in
		 * charging state or parallel charging / Qnovo / Charge pump is
		 * disabled. Disable ESR fast charge current control in SW.
		 */
		val = GEN4_ESR_FCC_1A << GEN4_ESR_FAST_CRG_IVAL_SHIFT;
		esr_fcc_ctrl_en = false;
	}

	mask = GEN4_ESR_FAST_CRG_IVAL_MASK | ESR_FAST_CRG_CTL_EN_BIT;
	rc = fg_masked_write(fg, BATT_INFO_ESR_FAST_CRG_CFG(fg), mask, val);
	if (rc < 0) {
		pr_err("Error in writing to %04x, rc=%d\n",
			BATT_INFO_ESR_FAST_CRG_CFG(fg), rc);
		return rc;
	}

	chip->esr_fcc_ctrl_en = esr_fcc_ctrl_en;
	fg_dbg(fg, FG_STATUS, "esr_fcc_ctrl_en set to %d\n",
		chip->esr_fcc_ctrl_en);
	return 0;
}

static int fg_gen4_configure_esr_cal_soc(struct fg_dev *fg, int soc_min,
					int soc_max)
{
	int rc;
	u8 buf[2];

	fg_encode(fg->sp, FG_SRAM_ESR_CAL_SOC_MIN, soc_min, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ESR_CAL_SOC_MIN].addr_word,
			fg->sp[FG_SRAM_ESR_CAL_SOC_MIN].addr_byte, buf,
			fg->sp[FG_SRAM_ESR_CAL_SOC_MIN].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_CAL_SOC_MIN, rc=%d\n", rc);
		return rc;
	}

	fg_encode(fg->sp, FG_SRAM_ESR_CAL_SOC_MAX, soc_max, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ESR_CAL_SOC_MAX].addr_word,
			fg->sp[FG_SRAM_ESR_CAL_SOC_MAX].addr_byte, buf,
			fg->sp[FG_SRAM_ESR_CAL_SOC_MAX].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_CAL_SOC_MAX, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static int fg_gen4_configure_esr_cal_temp(struct fg_dev *fg, int temp_min,
					int temp_max)
{
	int rc;
	u8 buf[2];

	fg_encode(fg->sp, FG_SRAM_ESR_CAL_TEMP_MIN, temp_min, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ESR_CAL_TEMP_MIN].addr_word,
			fg->sp[FG_SRAM_ESR_CAL_TEMP_MIN].addr_byte, buf,
			fg->sp[FG_SRAM_ESR_CAL_TEMP_MIN].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_CAL_TEMP_MIN, rc=%d\n", rc);
		return rc;
	}

	fg_encode(fg->sp, FG_SRAM_ESR_CAL_TEMP_MAX, temp_max, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ESR_CAL_TEMP_MAX].addr_word,
			fg->sp[FG_SRAM_ESR_CAL_TEMP_MAX].addr_byte, buf,
			fg->sp[FG_SRAM_ESR_CAL_TEMP_MAX].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_CAL_TEMP_MAX, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

#define ESR_CAL_TEMP_MIN	-127
#define ESR_CAL_TEMP_MAX	127
static int fg_gen4_esr_fast_calib_config(struct fg_gen4_chip *chip, bool en)
{
	struct fg_dev *fg = &chip->fg;
	int rc, esr_timer_chg_init, esr_timer_chg_max, esr_timer_dischg_init,
		esr_timer_dischg_max, esr_fast_cal_ms, esr_cal_soc_min,
		esr_cal_soc_max, esr_cal_temp_min, esr_cal_temp_max;
	u8 val, mask;

	esr_timer_chg_init = esr_timer_chg_max = -EINVAL;
	esr_timer_dischg_init = esr_timer_dischg_max = -EINVAL;
	if (en) {
		esr_timer_chg_init = chip->dt.esr_timer_chg_fast[TIMER_RETRY];
		esr_timer_chg_max = chip->dt.esr_timer_chg_fast[TIMER_MAX];
		esr_timer_dischg_init =
				chip->dt.esr_timer_dischg_fast[TIMER_RETRY];
		esr_timer_dischg_max =
				chip->dt.esr_timer_dischg_fast[TIMER_MAX];

		esr_cal_soc_min = 0;
		esr_cal_soc_max = FULL_SOC_RAW;
		esr_cal_temp_min = ESR_CAL_TEMP_MIN;
		esr_cal_temp_max = ESR_CAL_TEMP_MAX;

		vote(chip->delta_esr_irq_en_votable, DELTA_ESR_IRQ_VOTER,
			true, 0);
		chip->esr_fast_calib_done = false;
	} else {
		chip->esr_fast_calib_done = true;

		esr_timer_chg_init = chip->dt.esr_timer_chg_slow[TIMER_RETRY];
		esr_timer_chg_max = chip->dt.esr_timer_chg_slow[TIMER_MAX];
		esr_timer_dischg_init =
				chip->dt.esr_timer_dischg_slow[TIMER_RETRY];
		esr_timer_dischg_max =
				chip->dt.esr_timer_dischg_slow[TIMER_MAX];

		esr_cal_soc_min = chip->dt.esr_cal_soc_thresh[0];
		esr_cal_soc_max = chip->dt.esr_cal_soc_thresh[1];
		esr_cal_temp_min = chip->dt.esr_cal_temp_thresh[0];
		esr_cal_temp_max = chip->dt.esr_cal_temp_thresh[1];

		vote(chip->delta_esr_irq_en_votable, DELTA_ESR_IRQ_VOTER,
			false, 0);
	}

	rc = fg_set_esr_timer(fg, esr_timer_chg_init, esr_timer_chg_max, true,
				FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in setting ESR charge timer, rc=%d\n",
			rc);
		return rc;
	}

	rc = fg_set_esr_timer(fg, esr_timer_dischg_init, esr_timer_dischg_max,
				false, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in setting ESR discharge timer, rc=%d\n",
			rc);
		return rc;
	}

	rc = fg_gen4_configure_esr_cal_soc(fg, esr_cal_soc_min,
			esr_cal_soc_max);
	if (rc < 0) {
		pr_err("Error in configuring SOC thresholds, rc=%d\n",
			rc);
		return rc;
	}

	rc = fg_gen4_configure_esr_cal_temp(fg, esr_cal_temp_min,
			esr_cal_temp_max);
	if (rc < 0) {
		pr_err("Error in configuring temperature thresholds, rc=%d\n",
			rc);
		return rc;
	}

	/*
	 * Disable ESR discharging timer and ESR pulsing during
	 * discharging when ESR fast calibration is disabled. Otherwise, keep
	 * it enabled so that ESR pulses can happen during discharging.
	 */
	val = (en || chip->dt.esr_calib_dischg) ? BIT(6) | BIT(7) : 0;
	mask = BIT(6) | BIT(7);
	rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
			SYS_CONFIG_OFFSET, mask, val, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing SYS_CONFIG_WORD, rc=%d\n", rc);
		return rc;
	}

	/*
	 * esr_fast_cal_timer won't be initialized if esr_fast_calib is
	 * not enabled. Hence don't start/cancel the timer.
	 */
	if (!chip->esr_fast_calib)
		goto out;

	if (en) {
		/* Set ESR fast calibration timer to 50 seconds as default */
		esr_fast_cal_ms = 50000;
		if (chip->dt.esr_timer_chg_fast > 0 &&
			chip->dt.delta_esr_disable_count > 0)
			esr_fast_cal_ms = 3 * chip->dt.delta_esr_disable_count *
				chip->dt.esr_timer_chg_fast[TIMER_MAX] * 1000;

		alarm_start_relative(&chip->esr_fast_cal_timer,
					ms_to_ktime(esr_fast_cal_ms));
	} else {
		alarm_cancel(&chip->esr_fast_cal_timer);
	}

out:
	fg_dbg(fg, FG_STATUS, "%sabling ESR fast calibration\n",
		en ? "En" : "Dis");
	return 0;
}

#define IBATT_TAU_MASK	GENMASK(3, 0)
static int fg_gen4_set_vbatt_tau(struct fg_gen4_chip *chip, u8 vbatt_tau)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	u8 buf;

	rc = fg_sram_read(fg, fg->sp[FG_SRAM_VBAT_TAU].addr_word,
			fg->sp[FG_SRAM_VBAT_TAU].addr_byte,
			&buf, fg->sp[FG_SRAM_VBAT_TAU].len,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading Vbatt_tau, rc=%d\n", rc);
		return rc;
	}

	buf &= IBATT_TAU_MASK;
	buf |= vbatt_tau << 4;
	rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_VBAT_TAU].addr_word,
			fg->sp[FG_SRAM_VBAT_TAU].addr_byte,
			&buf, fg->sp[FG_SRAM_VBAT_TAU].len,
			FG_IMA_DEFAULT);
	if (rc < 0)
		pr_err("Error in writing Vbatt_tau, rc=%d\n", rc);

	return rc;
}

static int fg_gen4_enter_soc_scale(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc, soc;

	rc = fg_gen4_get_prop_capacity(fg, &soc);
	if (rc < 0) {
		pr_err("Failed to get capacity, rc =%d\n", rc);
		return rc;
	}

	/* Set entry FVS SOC equal to current H/W reported SOC */
	chip->soc_scale_msoc = chip->prev_soc_scale_msoc = soc;
	chip->scale_timer = chip->dt.scale_timer_ms;
	/*
	 * Calculate the FVS slope to linearly calculate SOC
	 * based on filtered battery voltage.
	 */
	chip->soc_scale_slope =
			DIV_ROUND_CLOSEST(chip->vbatt_res,
					chip->soc_scale_msoc);
	if (chip->soc_scale_slope <= 0) {
		pr_err("Error in slope calculated = %d\n",
			chip->soc_scale_slope);
		return -EINVAL;
	}

	chip->soc_scale_mode = true;
	fg_dbg(fg, FG_FVSS, "Enter FVSS mode, SOC=%d slope=%d timer=%d\n", soc,
		chip->soc_scale_slope, chip->scale_timer);
	alarm_start_relative(&chip->soc_scale_alarm_timer,
				ms_to_ktime(chip->scale_timer));

	return 0;
}

static void fg_gen4_write_scale_msoc(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int soc_raw, rc;

	if (!fg->charge_full) {
		soc_raw = DIV_ROUND_CLOSEST(chip->soc_scale_msoc * 0xFFFF,
						100);
		rc = fg_sram_write(fg, fg->sp[FG_SRAM_MONOTONIC_SOC].addr_word,
				fg->sp[FG_SRAM_MONOTONIC_SOC].addr_byte,
				(u8 *)&soc_raw,
				fg->sp[FG_SRAM_MONOTONIC_SOC].len,
				FG_IMA_ATOMIC);
		if (rc < 0) {
			pr_err("failed to write monotonic_soc rc=%d\n", rc);
			chip->soc_scale_mode = false;
		}
	}
}

static void fg_gen4_exit_soc_scale(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;

	if (chip->soc_scale_mode) {
		alarm_cancel(&chip->soc_scale_alarm_timer);
		if (work_busy(&chip->soc_scale_work) != WORK_BUSY_RUNNING)
			cancel_work_sync(&chip->soc_scale_work);

		/* While exiting soc_scale_mode, Update MSOC register */
		fg_gen4_write_scale_msoc(chip);
	}

	chip->soc_scale_mode = false;
	fg_dbg(fg, FG_FVSS, "Exit FVSS mode, work_status=%d\n",
				work_busy(&chip->soc_scale_work));
}

static int fg_gen4_validate_soc_scale_mode(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc;

	if (!chip->dt.soc_scale_mode)
		return 0;

	rc = fg_gen4_get_prop_soc_scale(chip);
	if (rc < 0) {
		pr_err("Failed to get soc scale props\n");
		goto fail_soc_scale;
	}

	rc = fg_get_msoc(fg, &chip->msoc_actual);
	if (rc < 0) {
		pr_err("Failed to get msoc rc=%d\n", rc);
		goto fail_soc_scale;
	}

	if (!chip->soc_scale_mode && fg->charge_status ==
		POWER_SUPPLY_STATUS_DISCHARGING &&
		chip->vbatt_avg < chip->dt.vbatt_scale_thr_mv) {
		rc = fg_gen4_enter_soc_scale(chip);
		if (rc < 0) {
			pr_err("Failed to enter SOC scale mode\n");
			goto fail_soc_scale;
		}
	} else if (chip->soc_scale_mode && chip->current_now < 0) {
		/*
		 * Stay in SOC scale mode till H/W SOC catch scaled SOC
		 * while charging.
		 */
		if (chip->msoc_actual >= chip->soc_scale_msoc)
			fg_gen4_exit_soc_scale(chip);
	}

	return 0;
fail_soc_scale:
	fg_gen4_exit_soc_scale(chip);
	return rc;
}

static int fg_gen4_set_vbatt_low(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc, vbatt_flt;

	if (chip->soc_scale_mode) {
		rc = fg_get_sram_prop(fg, FG_SRAM_VBAT_FLT,
					&vbatt_flt);
		if (rc < 0) {
			pr_err("failed to get filtered battery voltage, rc=%d\n",
				rc);
			/*
			 * If we fail here, exit FVSS mode
			 * and set Vbatt low flag true to report
			 * 0 SOC
			 */
			fg_gen4_exit_soc_scale(chip);
			chip->vbatt_low = true;
			return 0;
		}

		vbatt_flt /= 1000;
		if (vbatt_flt < chip->dt.empty_volt_mv ||
		    vbatt_flt > (fg->bp.float_volt_uv/1000)) {
			pr_err("Filtered Vbatt is not in range %d\n",
			       vbatt_flt);
			/*
			 * If we fail here, exit FVSS mode
			 * and set Vbatt low flag true to report
			 * 0 SOC
			 */
			fg_gen4_exit_soc_scale(chip);
			chip->vbatt_low = true;
			return 0;
		}

		if (vbatt_flt <= chip->dt.cutoff_volt_mv)
			chip->vbatt_low = true;
	} else {
		/* Set the flag to show 0% */
		chip->vbatt_low = true;
	}

	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int somc_fg_gen4_set_batt_aging_level(struct fg_dev *fg, int aging_level)
{
	if (fg->max_batt_aging_level == BATT_AGING_LEVEL_NONE) {
		pr_err("cannot set batt aging level because somc batterydata node does not exists\n");
		return -EINVAL;
	}

	if (aging_level < 0 || aging_level > fg->max_batt_aging_level) {
		pr_err("setting value is out of range\n");
		return -EINVAL;
	}

	if (aging_level == fg->batt_aging_level)
		return 0;

	fg->profile_load_status = PROFILE_NOT_LOADED;
	fg->requested_batt_aging_level = aging_level;
	schedule_delayed_work(&fg->profile_load_work, 0);
	return 0;
}

#endif
/* All irq handlers below this */

static irqreturn_t fg_mem_attn_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	complete_all(&chip->mem_attn);

	return IRQ_HANDLED;
}

static irqreturn_t fg_mem_xcp_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	u8 status;
	int rc;

	rc = fg_read(fg, MEM_IF_INT_RT_STS(fg), &status, 1);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			MEM_IF_INT_RT_STS(fg), rc);
		return IRQ_HANDLED;
	}

	fg_dbg(fg, FG_IRQ, "irq %d triggered, status:%d\n", irq, status);

	mutex_lock(&fg->sram_rw_lock);
	rc = fg_clear_dma_errors_if_any(fg);
	if (rc < 0)
		pr_err("Error in clearing DMA error, rc=%d\n", rc);

	if (status & MEM_XCP_BIT) {
		rc = fg_clear_ima_errors_if_any(fg, true);
		if (rc < 0 && rc != -EAGAIN)
			pr_err("Error in checking IMA errors rc:%d\n", rc);
	}

	mutex_unlock(&fg->sram_rw_lock);
	return IRQ_HANDLED;
}

static irqreturn_t fg_delta_esr_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int esr_uohms, rc;

	rc = fg_get_battery_resistance(fg, &esr_uohms);
	if (rc < 0)
		return IRQ_HANDLED;

	fg_dbg(fg, FG_IRQ, "irq %d triggered esr_uohms: %d\n", irq, esr_uohms);

	if (chip->esr_fast_calib) {
		vote(fg->awake_votable, ESR_CALIB, true, 0);
		schedule_work(&chip->esr_calib_work);
	}

	return IRQ_HANDLED;
}

static irqreturn_t fg_vbatt_low_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, vbatt_mv, msoc_raw;
	s64 time_us;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u8 status;

	rc = fg_read(fg, BATT_INFO_INT_RT_STS(fg), &status, 1);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			BATT_INFO_INT_RT_STS(fg), rc);
		return IRQ_HANDLED;
	}

#endif
	rc = fg_get_battery_voltage(fg, &vbatt_mv);
	if (rc < 0)
		return IRQ_HANDLED;

	vbatt_mv /= 1000;
	rc = fg_get_msoc_raw(fg, &msoc_raw);
	if (rc < 0)
		return IRQ_HANDLED;

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_IRQ, "irq %d triggered vbatt_mv: %d msoc_raw:%d\n", irq,
		vbatt_mv, msoc_raw);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_SOMC,
		"irq %d triggered sts:0x%02x vbatt_mv: %d msoc_raw:%d\n",
		irq, status, vbatt_mv, msoc_raw);
#endif

	if (!fg->soc_reporting_ready) {
		fg_dbg(fg, FG_IRQ, "SOC reporting is not ready\n");
		return IRQ_HANDLED;
	}

	if (chip->last_restart_time) {
		time_us = ktime_us_delta(ktime_get(), chip->last_restart_time);
		if (time_us < 10000000) {
			fg_dbg(fg, FG_IRQ, "FG restarted before %lld us\n",
				time_us);
			return IRQ_HANDLED;
		}
	}

	if (vbatt_mv < chip->dt.cutoff_volt_mv) {
		if (chip->dt.rapid_soc_dec_en) {
			/*
			 * Set this flag so that slope limiter coefficient
			 * cannot be configured during rapid SOC decrease.
			 */
			chip->rapid_soc_dec_en = true;

			rc = fg_gen4_rapid_soc_config(chip, true);
			if (rc < 0)
				pr_err("Error in configuring for rapid SOC reduction rc:%d\n",
					rc);
		} else {
			fg_gen4_set_vbatt_low(chip);
		}
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (chip->vbatt_low && (vbatt_mv >= chip->dt.cutoff_volt_mv ||
						!(status & GEN4_VBT_LOW_BIT))) {
		fg_dbg(fg, FG_SOMC, "vbat_low is deasserted\n");
		chip->vbatt_low = false;
	}
#endif

	if (batt_psy_initialized(fg))
		power_supply_changed(fg->batt_psy);

	return IRQ_HANDLED;
}

static irqreturn_t fg_batt_missing_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	u8 status;
	int rc;

	rc = fg_read(fg, ADC_RR_INT_RT_STS(fg), &status, 1);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			ADC_RR_INT_RT_STS(fg), rc);
		return IRQ_HANDLED;
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_IRQ, "irq %d triggered sts:%d\n", irq, status);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_dbg(fg, FG_SOMC, "irq %d triggered sts:%d\n", irq, status);
#endif
	fg->battery_missing = (status & ADC_RR_BT_MISS_BIT);

	if (fg->battery_missing) {
		fg->profile_available = false;
		fg->profile_load_status = PROFILE_NOT_LOADED;
		fg->soc_reporting_ready = false;
		fg->batt_id_ohms = -EINVAL;

		mutex_lock(&chip->ttf->lock);
		chip->ttf->step_chg_cfg_valid = false;
		chip->ttf->step_chg_num_params = 0;
		kfree(chip->ttf->step_chg_cfg);
		chip->ttf->step_chg_cfg = NULL;
		kfree(chip->ttf->step_chg_data);
		chip->ttf->step_chg_data = NULL;
		mutex_unlock(&chip->ttf->lock);
		cancel_delayed_work_sync(&chip->pl_enable_work);
		vote(fg->awake_votable, ESR_FCC_VOTER, false, 0);
		if (chip->pl_disable_votable)
			vote(chip->pl_disable_votable, ESR_FCC_VOTER, true, 0);
		if (chip->cp_disable_votable)
			vote(chip->cp_disable_votable, ESR_FCC_VOTER, true, 0);
		return IRQ_HANDLED;
	}

	clear_battery_profile(fg);
	schedule_delayed_work(&fg->profile_load_work, 0);

	if (fg->fg_psy)
		power_supply_changed(fg->fg_psy);

	return IRQ_HANDLED;
}

static irqreturn_t fg_batt_temp_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	return IRQ_HANDLED;
}

static irqreturn_t fg_delta_batt_temp_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, batt_temp;

	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("Error in getting batt_temp\n");
		return IRQ_HANDLED;
	}

	fg_dbg(fg, FG_IRQ, "irq %d triggered batt_temp:%d\n", irq, batt_temp);

	rc = fg_gen4_slope_limit_config(chip, batt_temp);
	if (rc < 0)
		pr_err("Error in configuring slope limiter rc:%d\n", rc);

	rc = fg_gen4_adjust_ki_coeff_full_soc(chip, batt_temp);
	if (rc < 0)
		pr_err("Error in configuring ki_coeff_full_soc rc:%d\n", rc);

	if (abs(fg->last_batt_temp - batt_temp) > 30)
		pr_warn("Battery temperature last:%d current: %d\n",
			fg->last_batt_temp, batt_temp);

	if (fg->last_batt_temp != batt_temp)
		fg->last_batt_temp = batt_temp;

	if (batt_psy_initialized(fg))
		power_supply_changed(fg->batt_psy);

	fg_gen4_update_rslow_coeff(fg, batt_temp);
	return IRQ_HANDLED;
}

static irqreturn_t fg_soc_ready_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	complete_all(&fg->soc_ready);
	return IRQ_HANDLED;
}

static irqreturn_t fg_soc_update_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	complete_all(&fg->soc_update);
	return IRQ_HANDLED;
}

static irqreturn_t fg_delta_bsoc_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	int rc;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);

	rc = fg_gen4_charge_full_update(fg);
	if (rc < 0)
		pr_err("Error in charge_full_update, rc=%d\n", rc);

	return IRQ_HANDLED;
}

#define CENTI_FULL_SOC		10000
static irqreturn_t fg_delta_msoc_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, batt_soc, batt_temp, msoc_raw;
	bool input_present = is_input_present(fg);
	u32 batt_soc_cp;

	rc = fg_get_msoc_raw(fg, &msoc_raw);
	if (!rc)
		fg_dbg(fg, FG_IRQ, "irq %d triggered msoc_raw: %d\n", irq,
			msoc_raw);

	get_batt_psy_props(fg);

	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &batt_soc);
	if (rc < 0)
		pr_err("Failed to read battery soc rc: %d\n", rc);
	else
		cycle_count_update(chip->counter, (u32)batt_soc >> 24,
			fg->charge_status, fg->charge_done, input_present);

	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("Failed to read battery temp rc: %d\n", rc);
	} else {
		if (chip->cl->active) {
			batt_soc_cp = div64_u64(
					(u64)(u32)batt_soc * CENTI_FULL_SOC,
					BATT_SOC_32BIT);
			cap_learning_update(chip->cl, batt_temp, batt_soc_cp,
				fg->charge_status, fg->charge_done,
				input_present, is_qnovo_en(fg));
		}

		rc = fg_gen4_slope_limit_config(chip, batt_temp);
		if (rc < 0)
			pr_err("Error in configuring slope limiter rc:%d\n",
				rc);
	}

	rc = fg_gen4_charge_full_update(fg);
	if (rc < 0)
		pr_err("Error in charge_full_update, rc=%d\n", rc);

	rc = fg_gen4_esr_soh_update(fg);
	if (rc < 0)
		pr_err("Error in updating ESR for SOH, rc=%d\n", rc);

	rc = fg_gen4_update_maint_soc(fg);
	if (rc < 0)
		pr_err("Error in updating maint_soc, rc=%d\n", rc);

	rc = fg_gen4_adjust_ki_coeff_dischg(fg);
	if (rc < 0)
		pr_err("Error in adjusting ki_coeff_dischg, rc=%d\n", rc);

	/*
	 * If ESR fast calibration is done even before 3 delta ESR interrupts
	 * had fired, then it is possibly a failed attempt. In such cases,
	 * retry ESR fast calibration once again. This will get restored to
	 * normal config once the timer expires or delta ESR interrupt count
	 * reaches the threshold.
	 */
	if (chip->esr_fast_calib && chip->esr_fast_calib_done &&
		(chip->delta_esr_count < 3) && !chip->esr_fast_calib_retry) {
		rc = fg_gen4_esr_fast_calib_config(chip, true);
		if (rc < 0)
			pr_err("Error in configuring esr_fast_calib, rc=%d\n",
				rc);
		else
			chip->esr_fast_calib_retry = true;
	}

	rc = fg_gen4_validate_soc_scale_mode(chip);
	if (rc < 0)
		pr_err("Failed to validate SOC scale mode, rc=%d\n", rc);

	if (batt_psy_initialized(fg))
		power_supply_changed(fg->batt_psy);

	return IRQ_HANDLED;
}

static irqreturn_t fg_empty_soc_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	if (batt_psy_initialized(fg))
		power_supply_changed(fg->batt_psy);

	return IRQ_HANDLED;
}

static irqreturn_t fg_soc_irq_handler(int irq, void *data)
{
	struct fg_dev *fg = data;

	fg_dbg(fg, FG_IRQ, "irq %d triggered\n", irq);
	return IRQ_HANDLED;
}

static irqreturn_t fg_dummy_irq_handler(int irq, void *data)
{
	pr_debug("irq %d triggered\n", irq);
	return IRQ_HANDLED;
}

static struct fg_irq_info fg_irqs[FG_GEN4_IRQ_MAX] = {
	/* BATT_SOC irqs */
	[MSOC_FULL_IRQ] = {
		.name		= "msoc-full",
		.handler	= fg_soc_irq_handler,
	},
	[MSOC_HIGH_IRQ] = {
		.name		= "msoc-high",
		.handler	= fg_soc_irq_handler,
		.wakeable	= true,
	},
	[MSOC_EMPTY_IRQ] = {
		.name		= "msoc-empty",
		.handler	= fg_empty_soc_irq_handler,
		.wakeable	= true,
	},
	[MSOC_LOW_IRQ] = {
		.name		= "msoc-low",
		.handler	= fg_soc_irq_handler,
		.wakeable	= true,
	},
	[MSOC_DELTA_IRQ] = {
		.name		= "msoc-delta",
		.handler	= fg_delta_msoc_irq_handler,
		.wakeable	= true,
	},
	[BSOC_DELTA_IRQ] = {
		.name		= "bsoc-delta",
		.handler	= fg_delta_bsoc_irq_handler,
		.wakeable	= true,
	},
	[SOC_READY_IRQ] = {
		.name		= "soc-ready",
		.handler	= fg_soc_ready_irq_handler,
		.wakeable	= true,
	},
	[SOC_UPDATE_IRQ] = {
		.name		= "soc-update",
		.handler	= fg_soc_update_irq_handler,
	},
	/* BATT_INFO irqs */
	[ESR_DELTA_IRQ] = {
		.name		= "esr-delta",
		.handler	= fg_delta_esr_irq_handler,
		.wakeable	= true,
	},
	[VBATT_LOW_IRQ] = {
		.name		= "vbatt-low",
		.handler	= fg_vbatt_low_irq_handler,
		.wakeable	= true,
	},
	[VBATT_PRED_DELTA_IRQ] = {
		.name		= "vbatt-pred-delta",
		.handler	= fg_dummy_irq_handler,
	},
	/* MEM_IF irqs */
	[MEM_ATTN_IRQ] = {
		.name		= "mem-attn",
		.handler	= fg_mem_attn_irq_handler,
		.wakeable	= true,
	},
	[DMA_GRANT_IRQ] = {
		.name		= "dma-grant",
		.handler	= fg_dummy_irq_handler,
		.wakeable	= true,
	},
	[MEM_XCP_IRQ] = {
		.name		= "ima-xcp",
		.handler	= fg_mem_xcp_irq_handler,
		.wakeable	= true,
	},
	[DMA_XCP_IRQ] = {
		.name		= "dma-xcp",
		.handler	= fg_dummy_irq_handler,
		.wakeable	= true,
	},
	[IMA_RDY_IRQ] = {
		.name		= "ima-rdy",
		.handler	= fg_dummy_irq_handler,
	},
	/* ADC_RR irqs */
	[BATT_TEMP_COLD_IRQ] = {
		.name		= "batt-temp-cold",
		.handler	= fg_batt_temp_irq_handler,
		.wakeable	= true,
	},
	[BATT_TEMP_HOT_IRQ] = {
		.name		= "batt-temp-hot",
		.handler	= fg_batt_temp_irq_handler,
		.wakeable	= true,
	},
	[BATT_TEMP_DELTA_IRQ] = {
		.name		= "batt-temp-delta",
		.handler	= fg_delta_batt_temp_irq_handler,
		.wakeable	= true,
	},
	[BATT_ID_IRQ] = {
		.name		= "batt-id",
		.handler	= fg_dummy_irq_handler,
	},
	[BATT_MISSING_IRQ] = {
		.name		= "batt-missing",
		.handler	= fg_batt_missing_irq_handler,
		.wakeable	= true,
	},
};

static enum alarmtimer_restart fg_esr_fast_cal_timer(struct alarm *alarm,
							ktime_t time)
{
	struct fg_gen4_chip *chip = container_of(alarm, struct fg_gen4_chip,
					esr_fast_cal_timer);
	struct fg_dev *fg = &chip->fg;

	if (!chip->esr_fast_calib_done) {
		fg_dbg(fg, FG_STATUS, "ESR fast calibration timer expired\n");

		/*
		 * We cannot vote for awake votable here as that takes
		 * a mutex lock and this is executed in an atomic context.
		 */
		pm_stay_awake(fg->dev);
		chip->esr_fast_cal_timer_expired = true;
		schedule_work(&chip->esr_calib_work);
	}

	return ALARMTIMER_NORESTART;
}

static void esr_calib_work(struct work_struct *work)
{
	struct fg_gen4_chip *chip = container_of(work, struct fg_gen4_chip,
				    esr_calib_work);
	struct fg_dev *fg = &chip->fg;
	int rc, fg_esr_meas_diff;
	s16 esr_raw, esr_char_raw, esr_delta, esr_meas_diff, esr_filtered;
	u8 buf[2];

	mutex_lock(&chip->esr_calib_lock);

	if (chip->delta_esr_count > chip->dt.delta_esr_disable_count ||
		chip->esr_fast_calib_done) {
		fg_dbg(fg, FG_STATUS, "delta_esr_count: %d esr_fast_calib_done:%d\n",
			chip->delta_esr_count, chip->esr_fast_calib_done);
		goto out;
	}

	/*
	 * If the number of delta ESR interrupts fired is more than the count
	 * to disable the interrupt OR ESR fast calibration timer is expired
	 * OR after one retry, disable ESR fast calibration.
	 */
	if (chip->delta_esr_count >= chip->dt.delta_esr_disable_count ||
		chip->esr_fast_cal_timer_expired) {
		rc = fg_gen4_esr_fast_calib_config(chip, false);
		if (rc < 0)
			pr_err("Error in configuring esr_fast_calib, rc=%d\n",
				rc);

		if (chip->esr_fast_cal_timer_expired) {
			pm_relax(fg->dev);
			chip->esr_fast_cal_timer_expired = false;
		}

		if (chip->esr_fast_calib_retry)
			chip->esr_fast_calib_retry = false;

		goto out;
	}

	rc = fg_sram_read(fg, ESR_WORD, ESR_OFFSET, buf, 2,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading ESR, rc=%d\n", rc);
		goto out;
	}
	esr_raw = buf[1] << 8 | buf[0];

	rc = fg_sram_read(fg, ESR_CHAR_WORD, ESR_CHAR_OFFSET, buf, 2,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading ESR_CHAR, rc=%d\n", rc);
		goto out;
	}
	esr_char_raw = buf[1] << 8 | buf[0];

	esr_meas_diff = esr_raw - esr_char_raw;

	rc = fg_sram_read(fg, ESR_DELTA_DISCHG_WORD, ESR_DELTA_DISCHG_OFFSET,
			buf, 2, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading ESR_DELTA_DISCHG, rc=%d\n", rc);
		goto out;
	}
	esr_delta = buf[1] << 8 | buf[0];
	fg_dbg(fg, FG_STATUS, "esr_raw: 0x%x esr_char_raw: 0x%x esr_meas_diff: 0x%x esr_delta: 0x%x\n",
		esr_raw, esr_char_raw, esr_meas_diff, esr_delta);

	fg_esr_meas_diff = esr_meas_diff - (esr_delta / 32);

	/* Don't filter for the first attempt so that ESR can converge faster */
	if (!chip->delta_esr_count)
		esr_filtered = fg_esr_meas_diff;
	else
		esr_filtered = fg_esr_meas_diff >> chip->dt.esr_filter_factor;

	esr_delta = esr_delta + (esr_filtered * 32);

	/* Bound the limits */
	if (esr_delta > SHRT_MAX)
		esr_delta = SHRT_MAX;
	else if (esr_delta < SHRT_MIN)
		esr_delta = SHRT_MIN;

	fg_dbg(fg, FG_STATUS, "fg_esr_meas_diff: 0x%x esr_filt: 0x%x esr_delta_new: 0x%x\n",
		fg_esr_meas_diff, esr_filtered, esr_delta);

	buf[0] = esr_delta & 0xff;
	buf[1] = (esr_delta >> 8) & 0xff;
	rc = fg_sram_write(fg, ESR_DELTA_DISCHG_WORD, ESR_DELTA_DISCHG_OFFSET,
			buf, 2, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_DELTA_DISCHG, rc=%d\n", rc);
		goto out;
	}

	rc = fg_sram_write(fg, ESR_DELTA_CHG_WORD, ESR_DELTA_CHG_OFFSET,
			buf, 2, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing ESR_DELTA_CHG, rc=%d\n", rc);
		goto out;
	}

	chip->delta_esr_count++;
	fg_dbg(fg, FG_STATUS, "Wrote ESR delta [0x%x 0x%x]\n", buf[0], buf[1]);
out:
	mutex_unlock(&chip->esr_calib_lock);
	vote(fg->awake_votable, ESR_CALIB, false, 0);
}

static enum alarmtimer_restart fg_soc_scale_timer(struct alarm *alarm,
							ktime_t time)
{
	struct fg_gen4_chip *chip = container_of(alarm, struct fg_gen4_chip,
							soc_scale_alarm_timer);

	schedule_work(&chip->soc_scale_work);
	return ALARMTIMER_NORESTART;
}

static void soc_scale_work(struct work_struct *work)
{
	struct fg_gen4_chip *chip = container_of(work, struct fg_gen4_chip,
						soc_scale_work);
	struct fg_dev *fg = &chip->fg;
	int soc, soc_thr_percent, rc;

	if (!chip->soc_scale_mode)
		return;

	soc_thr_percent = chip->dt.delta_soc_thr / 10;
	if (soc_thr_percent == 0) {
		/* Set minimum SOC change that can be reported = 1% */
		soc_thr_percent = 1;
	}

	rc = fg_gen4_validate_soc_scale_mode(chip);
	if (rc < 0)
		pr_err("Failed to validate SOC scale mode, rc=%d\n", rc);

	/* re-validate soc scale mode as we may have exited FVSS */
	if (!chip->soc_scale_mode) {
		fg_dbg(fg, FG_FVSS, "exit soc scale mode\n");
		return;
	}

	if (chip->vbatt_res <= 0)
		chip->vbatt_res = 0;

	mutex_lock(&chip->soc_scale_lock);
	soc = DIV_ROUND_CLOSEST(chip->vbatt_res,
				chip->soc_scale_slope);
	chip->soc_scale_msoc = soc;
	chip->scale_timer = chip->dt.scale_timer_ms;

	fg_dbg(fg, FG_FVSS, "soc: %d last soc: %d msoc_actual: %d\n", soc,
			chip->prev_soc_scale_msoc, chip->msoc_actual);
	if ((chip->prev_soc_scale_msoc - chip->msoc_actual) > soc_thr_percent) {
		/*
		 * If difference between previous SW calculated SOC and HW SOC
		 * is higher than SOC threshold, then handle this by
		 * showing previous SW SOC - SOC threshold.
		 */
		chip->soc_scale_msoc = chip->prev_soc_scale_msoc -
					soc_thr_percent;
	} else if (soc > chip->prev_soc_scale_msoc) {
		/*
		 * If calculated SOC is higher than current SOC, report current
		 * SOC
		 */
		chip->soc_scale_msoc = chip->prev_soc_scale_msoc;
		chip->scale_timer = chip->dt.scale_timer_ms;
	} else if ((chip->prev_soc_scale_msoc - soc) > soc_thr_percent) {
		/*
		 * If difference b/w current SOC and calculated SOC
		 * is higher than SOC threshold then handle this by
		 * showing current SOC - SOC threshold and decrease
		 * timer resolution to catch up the rate of decrement
		 * of SOC.
		 */
		chip->soc_scale_msoc = chip->prev_soc_scale_msoc -
					soc_thr_percent;
		chip->scale_timer = chip->dt.scale_timer_ms /
				(chip->prev_soc_scale_msoc - soc);
	}

	if (chip->soc_scale_msoc < 0)
		chip->soc_scale_msoc = 0;

	mutex_unlock(&chip->soc_scale_lock);
	if (chip->prev_soc_scale_msoc != chip->soc_scale_msoc) {
		if (batt_psy_initialized(fg))
			power_supply_changed(fg->batt_psy);
	}

	chip->prev_soc_scale_msoc = chip->soc_scale_msoc;
	fg_dbg(fg, FG_FVSS, "Calculated SOC=%d SOC reported=%d timer resolution=%d\n",
		soc, chip->soc_scale_msoc, chip->scale_timer);
	alarm_start_relative(&chip->soc_scale_alarm_timer,
				ms_to_ktime(chip->scale_timer));
}

static void pl_current_en_work(struct work_struct *work)
{
	struct fg_gen4_chip *chip = container_of(work,
				struct fg_gen4_chip,
				pl_current_en_work);
	struct fg_dev *fg = &chip->fg;
	bool input_present = is_input_present(fg), en;

	en = fg->charge_done ? false : input_present;

	if (get_effective_result(chip->parallel_current_en_votable) == en)
		return;

	vote(chip->parallel_current_en_votable, FG_PARALLEL_EN_VOTER, en, 0);
}

static void pl_enable_work(struct work_struct *work)
{
	struct fg_gen4_chip *chip = container_of(work,
				struct fg_gen4_chip,
				pl_enable_work.work);
	struct fg_dev *fg = &chip->fg;

	if (chip->pl_disable_votable)
		vote(chip->pl_disable_votable, ESR_FCC_VOTER, false, 0);
	if (chip->cp_disable_votable)
		vote(chip->cp_disable_votable, ESR_FCC_VOTER, false, 0);
	vote(fg->awake_votable, ESR_FCC_VOTER, false, 0);
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void fg_gen4_somc_jeita_step_wakelock(struct fg_dev *fg, bool en)
{
	if (en)
		__pm_stay_awake(fg->step_ws);
	else
		__pm_relax(fg->step_ws);

	fg_dbg(fg, FG_STEP, "wake lock for JEITA/Step: %d\n", (int)en);
	fg->step_lock_en = en;
}

static void fg_gen4_somc_jeita_step_update(struct fg_dev *fg)
{
	int rc;
	bool input_present;
	int bsoc;

	if (!fg->step_en)
		return;

	input_present = is_input_present(fg);

	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &bsoc);
	bsoc = 10000 * ((u32)bsoc >> 16) / 65535;

	fg_dbg(fg, FG_STEP, "step_lock_en=%d input_present=%d bsoc=%d\n",
					fg->step_lock_en, input_present, bsoc);

	if (!fg->step_lock_en && input_present) {
		mutex_lock(&fg->step_lock);
		fg_gen4_somc_jeita_step_wakelock(fg, true);
		fg_dbg(fg, FG_STATUS, "schedule JEITA/Step worker\n");
		schedule_delayed_work(&fg->somc_jeita_step_charge_work,
							msecs_to_jiffies(0));
		mutex_unlock(&fg->step_lock);
	}
}

#define CELL_IMPEDANCE_MOHM 30
static void fg_gen4_somc_jeita_step_charge_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev,
					somc_jeita_step_charge_work.work);
	union power_supply_propval prop = {0, };
	int rc;
	int temp, vbatt_uv = 0, current_now = 0;
	int temp_sum = 0, current_now_sum = 0, vbatt_uv_sum = 0;
	int temp_avg = 0, current_now_avg = 0, vbatt_uv_avg = 0;
	int sum_cnt = 0;
	int vcell_mv, target_current_ma = 0, target_voltage_mv = 0;
	int condition = 0;
	bool target_is_found = false;
	int i;
	int target_idx = -1;
	bool input_present;
	int msoc, bsoc;
	int ocv_uv;

	static const char * const str_status[] = {"-", "Charging",
				"Discharging", "NotCharging", "FULL", "-"};

	if (!fg->step_en)
		return;

	mutex_lock(&fg->step_lock);

	if (fg->profile_load_status != PROFILE_LOADED)
		goto retry;

	/* read input present status */
	input_present = is_input_present(fg);

	/* read msoc for log */
	rc = fg_get_sram_prop(fg, FG_SRAM_MONOTONIC_SOC, &msoc);
	if (rc < 0)
		pr_err("failed to get msoc\n");
	else
		msoc  = 10000 * msoc / 65535;

	/* read bsoc for log */
	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &bsoc);
	if (rc < 0)
		pr_err("failed to get bsoc\n");
	else
		bsoc = 10000 * ((u32)bsoc >> 16) / 65535;

	/* read real temp */
	if (fg->use_real_temp)
		rc = fg_gen4_somc_get_real_temp(fg, &temp);
	else
		rc = fg_gen4_get_battery_temp(fg, &temp);

	if (rc < 0) {
		pr_err("failed to get temp\n");
		temp = 250;
	}

	/* read current_now */
	rc = fg_get_battery_current(fg, &current_now);
	if (rc < 0) {
		pr_err("failed to get battery current, rc=%d\n", rc);
		goto out;
	}

	/* read vbat */
	rc = fg_get_battery_voltage(fg, &vbatt_uv);
	if (rc < 0) {
		pr_err("failed to get battery voltage, rc=%d\n", rc);
		goto out;
	}

	/* read OCV */
	rc = fg_get_sram_prop(fg, FG_SRAM_OCV, &ocv_uv);
	if (rc < 0) {
		pr_err("failed to get battery ocv, rc=%d\n", rc);
		ocv_uv = 0;
	}

	/* calc avarage value of temp, current and voltage */
	for (i = STEP_INPUT_BUF_NUM - 1; i > 0; i--)
		fg->step_input_data[i] = fg->step_input_data[i - 1];
	fg->step_input_data[0].temp = temp;
	fg->step_input_data[0].current_now = current_now;
	fg->step_input_data[0].voltage_now = vbatt_uv;
	fg->step_input_data[0].stored_ktime_ms =
					ktime_to_ms(ktime_get_boottime());

	for (i = 0; i < STEP_INPUT_BUF_NUM; i++) {
		if (fg->step_input_data[i].stored_ktime_ms) {
			temp_sum += fg->step_input_data[i].temp;
			current_now_sum += fg->step_input_data[i].current_now;
			vbatt_uv_sum += fg->step_input_data[i].voltage_now;
			sum_cnt++;
			fg_dbg(fg, FG_STEP,
				"[%d] temp:%d current:%d voltage:%d\n",
				i,
				fg->step_input_data[i].temp,
				fg->step_input_data[i].current_now,
				fg->step_input_data[i].voltage_now);
		}
	}
	if (sum_cnt > 0) {
		temp_avg = temp_sum / sum_cnt;
		current_now_avg = current_now_sum / sum_cnt;
		vbatt_uv_avg = vbatt_uv_sum / sum_cnt;
	}
	fg_dbg(fg, FG_STEP, "[T] temp:%d current:%d voltage:%d\n",
				temp_avg, current_now_avg, vbatt_uv_avg);

	/* calculate cell voltage */
	if (current_now_avg < 0 && fg->cell_impedance_mohm)
		vcell_mv = (vbatt_uv_avg - (fg->cell_impedance_mohm * (-1) *
						current_now_avg / 1000)) / 1000;
	else
		vcell_mv = vbatt_uv_avg / 1000;

	/* update the highest value of cell voltage */
	if (fg->charge_status != POWER_SUPPLY_STATUS_CHARGING) {
		fg_dbg(fg, FG_STEP, "Clear vcell max.\n");
		fg->vcell_max_mv = 0;
	} else if (fg->charge_status != fg->prev_charge_status) {
		fg_dbg(fg, FG_STEP, "Init vcell max as current vcell.\n");
		fg->vcell_max_mv = vcell_mv;
	} else if (vcell_mv > fg->vcell_max_mv) {
		fg_dbg(fg, FG_STEP, "Update vcell max\n");
		fg->vcell_max_mv = vcell_mv;
	} else if (current_now_avg > 0) {
		fg_dbg(fg, FG_STEP, "Init vcell max due to minus charge\n");
		fg->vcell_max_mv = vcell_mv;
	}

	fg_dbg(fg, FG_STEP,
		"[%s] BSOC:%d MSOC:%d RTEMP:%d IBATT:%dmA VBATT:%dmV VCELL:%dmV VCELL_MAX:%dmV OCV=%dmV\n",
		str_status[fg->charge_status], bsoc, msoc, temp_avg,
		(current_now_avg / 1000), (vbatt_uv_avg / 1000),
		vcell_mv, fg->vcell_max_mv, (ocv_uv / 1000));

	/* determine step current target */
	fg_dbg(fg, FG_STEP,
		"+-------------------------------------------------------+\n");
	fg_dbg(fg, FG_STEP,
		"|No |  Range of Temp  | Range of VCELL  | Target fcc/fv |\n");
	fg_dbg(fg, FG_STEP,
		"|---+-----------------+-----------------+---------------|\n");
	for (i = 0; i < fg->step_data.data_num; i++) {
		if (!target_is_found &&
		    temp_avg >= fg->step_data.temp_low[i] &&
		    temp_avg < fg->step_data.temp_high[i] &&
		    fg->vcell_max_mv >= fg->step_data.voltage_low[i] &&
		    fg->vcell_max_mv <= fg->step_data.voltage_high[i]) {
			target_current_ma = fg->step_data.target_current[i];
			target_voltage_mv = fg->step_data.target_voltage[i];
			condition = fg->step_data.condition[i];
			target_is_found = true;
			target_idx = i;
		}
		fg_dbg(fg, FG_STEP,
			"|%2d | %6d - %6d | %6d - %6d | %2s%5d%6d |\n",
			i + 1,
			fg->step_data.temp_low[i],
			fg->step_data.temp_high[i],
			fg->step_data.voltage_low[i],
			fg->step_data.voltage_high[i],
			i == target_idx ? "->" : "  ",
			fg->step_data.target_current[i],
			fg->step_data.target_voltage[i]);
	}
	fg_dbg(fg, FG_STEP,
		"+-------------------------------------------------------+\n");

	if (!target_is_found) {
		fg_dbg(fg, FG_STEP, "Couldn't find step target in table.\n");
		goto retry;
	}

	if (target_idx != fg->target_idx)
		fg_dbg(fg, FG_SOMC,
			"[%s] BSOC:%d MSOC:%d RTEMP:%d IBATT:%dmA VBATT:%dmV VCELL:%dmV Target FCC:%dmA FV:%dmV\n",
			str_status[fg->charge_status], bsoc, msoc, temp_avg,
			(current_now_avg / 1000), (vbatt_uv_avg / 1000),
			vcell_mv, target_current_ma, target_voltage_mv);
	fg->target_idx = target_idx;

	/* request vote to smb driver */
	prop.intval = condition;
	rc = power_supply_set_property(fg->batt_psy,
					POWER_SUPPLY_PROP_JEITA_CONDITION,
					&prop);
	if (rc < 0)
		pr_err("Error in step jeita fv set on batt_psy, rc=%d\n", rc);

	prop.intval = target_current_ma * 1000;
	rc = power_supply_set_property(fg->batt_psy,
					POWER_SUPPLY_PROP_JEITA_STEP_FCC,
					&prop);
	if (rc < 0)
		pr_err("Error in step current set on batt_psy, rc=%d\n", rc);

	prop.intval = target_voltage_mv * 1000;
	rc = power_supply_set_property(fg->batt_psy,
					POWER_SUPPLY_PROP_JEITA_STEP_FV,
					&prop);
	if (rc < 0)
		pr_err("Error in step jeita fv set on batt_psy, rc=%d\n", rc);

retry:
	/* re-schedule work */
	if (input_present && (fg->charge_status != POWER_SUPPLY_STATUS_FULL)) {
		schedule_delayed_work(&fg->somc_jeita_step_charge_work,
							msecs_to_jiffies(2000));
	} else {
		fg_dbg(fg, FG_SOMC,
			"exit step_work input_present:%d status:%d\n",
					input_present, fg->charge_status);
		memset(fg->step_input_data, 0, sizeof(fg->step_input_data));
		fg_gen4_somc_jeita_step_wakelock(fg, false);
	}
	mutex_unlock(&fg->step_lock);
	return;
out:
	memset(fg->step_input_data, 0, sizeof(fg->step_input_data));
	fg->target_idx = -1;
	fg_gen4_somc_jeita_step_wakelock(fg, false);
	mutex_unlock(&fg->step_lock);
}

static void fg_gen4_somc_set_temp_corr_sus_time(struct fg_dev *fg, int ktime_ms)
{
	if (!fg->temp_corr_en)
		return;

	mutex_lock(&fg->temp_corr_lock);
	fg->temp_data.suspended_ktime_ms = ktime_ms;
	mutex_unlock(&fg->temp_corr_lock);
}

enum {
	TEMP_CORR_SAMPLE_WORK = 0,
	TEMP_CORR_SAMPLE_SUSRES,
};
static void fg_gen4_somc_set_temp_corr_data(struct fg_dev *fg, int mode)
{
	int rc;
	int i, j;
	int batt_temp;
	int batt_current_ma;
	int cnt;
	int current_ktime_ms, store_start_time_ms;
	struct fg_temp_data *td;

	if (!fg->temp_corr_en)
		return;

	mutex_lock(&fg->temp_corr_lock);

	td = &fg->temp_data;
	current_ktime_ms = ktime_to_ms(ktime_get_boottime());

	/*
	 * store 0mA data during suspend to buffer just after resumed.
	 * data[0] is last suspended data.
	 */
	if (mode == TEMP_CORR_SAMPLE_SUSRES && td->suspended_ktime_ms) {
		cnt = (current_ktime_ms - td->suspended_ktime_ms) / 1000;
		if (cnt > TEMP_DATA_BUF_NUM)
			store_start_time_ms = current_ktime_ms -
						TEMP_DATA_BUF_NUM * 1000;
		else
			store_start_time_ms = td->suspended_ktime_ms;
		fg_dbg(fg, FG_TEMP_CORR, "fill with 0mA data. suspended_ktime_ms:%d, current_ktime_ms:%d, store_start_time_ms:%d\n",
			td->suspended_ktime_ms, current_ktime_ms,
			store_start_time_ms);
		for (j = 0; j < cnt && j < TEMP_DATA_BUF_NUM; j++) {
			batt_temp = td->data[0].batt_temp;
			for (i = TEMP_DATA_BUF_NUM - 1; i > 0; i--)
				td->data[i] = td->data[i - 1];
			td->data[0].id = td->last_id++;
			td->data[0].batt_current_ma = 0;
			td->data[0].batt_temp = batt_temp;
			td->data[0].ktime_ms = store_start_time_ms + j * 1000;
			fg_dbg(fg, FG_TEMP_CORR, "(%d) Sample ID:%d, time:%d, batt_current:%dmA, batt_temp:%3d\n",
				mode, td->last_id, td->data[0].ktime_ms,
				td->data[0].batt_current_ma,
				td->data[0].batt_temp);
		}
	}

	/* read batt temp */
	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("failed to get batt temp\n");
		batt_temp = 250;
	}

	/* read current_now */
	rc = fg_get_battery_current(fg, &batt_current_ma);
	if (rc < 0) {
		pr_err("failed to get battery current, rc=%d\n", rc);
		batt_current_ma = 0;
	} else {
		batt_current_ma /= 1000;
	}

	/* store data to buffer. data[0] is the latest */
	for (i = TEMP_DATA_BUF_NUM - 1; i > 0; i--)
		td->data[i] = td->data[i - 1];
	td->data[0].id = td->last_id++;
	td->data[0].batt_current_ma = batt_current_ma;
	td->data[0].batt_temp = batt_temp;
	td->data[0].ktime_ms = current_ktime_ms;
	fg_dbg(fg, FG_TEMP_CORR, "(%d) Sample ID:%d, time:%d, batt_current:%dmA, batt_temp:%3d\n",
			mode, td->last_id, td->data[0].ktime_ms,
			td->data[0].batt_current_ma, td->data[0].batt_temp);

	if (td->last_id == 1)
		td->first_ktime_ms = td->data[0].ktime_ms;

	mutex_unlock(&fg->temp_corr_lock);
}

static void fg_gen4_somc_temp_corr_on_resume(struct fg_dev *fg)
{
	if (!fg->temp_corr_en)
		return;

	fg_gen4_somc_set_temp_corr_data(fg, TEMP_CORR_SAMPLE_SUSRES);
	schedule_delayed_work(&fg->somc_temp_corr_work, msecs_to_jiffies(0));
}

static void fg_gen4_somc_temp_corr_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev,
						somc_temp_corr_work.work);
	int i;
	int active_time_ms;
	int cur_a, cur_b, integral_cur, cur_diff_sum = 0, cur_avg = 0;
	int time_latest, time_a, time_b, time_total_ms = 0, time_diff_ms;
	int avg_ms, coef_a;
	int ctemp;
	int delay;
	struct fg_temp_data *td;

	if (!fg->temp_corr_en)
		return;

	td = &fg->temp_data;
	coef_a = fg->temp_corr_coef_a;
	avg_ms = fg->temp_corr_avg_sec * 1000;

	if (avg_ms <= 0) {
		fg_dbg(fg, FG_SOMC,
			"Don't calculate Iavg. avg_ms:%d\n", avg_ms);
		return;
	}

	/* take a sample of ibat data */
	fg_gen4_somc_set_temp_corr_data(fg, TEMP_CORR_SAMPLE_WORK);

	if (td->last_id == 1)
		td->first_ktime_ms = td->data[0].ktime_ms;

	active_time_ms = td->data[0].ktime_ms - td->first_ktime_ms;
	if (active_time_ms < avg_ms) {
		fg_dbg(fg, FG_TEMP_CORR, "Sample data is not enough. (%d ms)\n",
								active_time_ms);
		goto reschedule;
	}
	/* calc moving average from buffer */
	time_latest = td->data[0].ktime_ms;
	for (i = 0; i < TEMP_DATA_BUF_NUM - 1; i++) {
		time_a = td->data[i].ktime_ms;
		time_b = td->data[i + 1].ktime_ms;

		time_total_ms = time_latest - time_b;
		time_diff_ms = time_a - time_b;
		cur_a = abs(td->data[i].batt_current_ma);
		cur_b = abs(td->data[i + 1].batt_current_ma);
		integral_cur = (cur_a + cur_b) * time_diff_ms / 2;
		cur_diff_sum = cur_diff_sum + integral_cur;
		if (td->data[i].id)
			fg_dbg(fg, FG_TEMP_CORR,
				"Absolute current:%4d->%4d, time:%d->%d(diff:%6d(ms)), integral:%d(sum:%d)\n",
				cur_a, cur_b, time_a, time_b, time_diff_ms,
				integral_cur, cur_diff_sum);

		if (td->data[i + 1].id && time_latest - time_b > avg_ms)
			break;
	}
	if (time_total_ms > 0) {
		cur_avg = (cur_diff_sum / time_total_ms);
		fg_dbg(fg, FG_TEMP_CORR, "Iavg:%d(mA), cur_diff_sum:%d, time_total_ms:%d\n",
					cur_avg, cur_diff_sum, time_total_ms);
	}

	/* calculate cell temp from battery temp and moving average current */
	if (cur_avg > 0) {
		fg->cell_temp_avtive = true;
		ctemp = ((u64)cur_avg * (u64)cur_avg * coef_a) / (100 * 100000);
		fg->cell_temp = td->data[0].batt_temp - ctemp;
		fg_dbg(fg, FG_TEMP_CORR, "latest batt_temp:%d, cell_temp:%d\n",
					td->data[0].batt_temp, fg->cell_temp);
	} else {
		fg->cell_temp_avtive = false;
		fg->cell_temp = td->data[0].batt_temp;
		fg_dbg(fg, FG_TEMP_CORR, "latest batt_temp:%d, cell_temp:%d(no correction)\n",
					td->data[0].batt_temp, fg->cell_temp);
	}
	fg->iavg = cur_avg;

reschedule:
	if (td->data[0].batt_current_ma > 1000 ||
		td->data[0].batt_current_ma < -50)
		delay = 1000;
	else
		delay = 5000;

	schedule_delayed_work(&fg->somc_temp_corr_work,
						msecs_to_jiffies(delay));
}

static enum alarmtimer_restart fg_gen4_somc_psy_chg_alarm_timer(
					struct alarm *alarm, ktime_t time)
{
	struct fg_dev *fg = container_of(alarm, struct fg_dev,
							psy_chg_alarm_timer);

	fg->psy_chg_awake = true;
	pm_stay_awake(fg->dev);

	fg_dbg(fg, FG_SOMC, "PSY CHANGE timer expired\n");
	schedule_work(&fg->psy_chg_work);

	return ALARMTIMER_NORESTART;
}

#define PSY_CHG_ALARM_ONLINE_HOT_TIME_MS	(10 * 60 * 1000)
#define PSY_CHG_ALARM_ONLINE_OTHER_TIME_MS	(60 * 60 * 1000)
#define PSY_CHG_ALARM_OFFLINE_HOT_TIME_MS	(30 * 60 * 1000)
#define PSY_CHG_ALARM_OFFLINE_OTHER_TIME_MS	(24 * 60 * 60 * 1000)
#define PSY_CHG_ALARM_FIRST_TIME_MS		(10 * 60 * 1000)
#define PSY_CHG_HOT_THRESHOLD			(400)
#define PSY_CHG_WAKE_HOLD_TIME_MS		(20)
static void fg_gen4_somc_psy_chg_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev, psy_chg_work);
	int rc;
	bool input_present;
	int temp, next_time = 0;

	if (fg->psy_chg_awake)
		vote(fg->awake_votable, PSY_CHG_VOTER, true, 0);

	/* read real temp */
	if (fg->use_real_temp)
		rc = fg_gen4_somc_get_real_temp(fg, &temp);
	else
		rc = fg_gen4_get_battery_temp(fg, &temp);

	if (rc < 0) {
		pr_err("failed to get temp\n");
		temp = 250;
	}

	/* read input present status */
	input_present = is_input_present(fg);

	if (input_present) {
		if (temp > PSY_CHG_HOT_THRESHOLD)
			next_time = PSY_CHG_ALARM_ONLINE_HOT_TIME_MS;
		else
			next_time = PSY_CHG_ALARM_ONLINE_OTHER_TIME_MS;
	} else {
		if (temp > PSY_CHG_HOT_THRESHOLD)
			next_time = PSY_CHG_ALARM_OFFLINE_HOT_TIME_MS;
		else
			next_time = PSY_CHG_ALARM_OFFLINE_OTHER_TIME_MS;
	}

	if (fg->batt_psy) {
		fg->psy_chg_counter++;
		power_supply_changed(fg->batt_psy);
		msleep(PSY_CHG_WAKE_HOLD_TIME_MS);
	}

	fg_dbg(fg, FG_SOMC, "Sent power_supply_changed. input_present:%d, temp=%d, next_time:%d\n",
					input_present, temp, next_time);
	if (next_time)
		alarm_start_relative(&fg->psy_chg_alarm_timer,
						ms_to_ktime(next_time));

	fg->psy_chg_awake = false;
	vote(fg->awake_votable, PSY_CHG_VOTER, false, 0);
}
#endif
static void status_change_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work,
			struct fg_dev, status_change_work);
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc, batt_soc, batt_temp;
	bool input_present, qnovo_en;
	u32 batt_soc_cp;

	if (fg->battery_missing) {
		pm_relax(fg->dev);
		return;
	}

	if (!chip->pl_disable_votable)
		chip->pl_disable_votable = find_votable("PL_DISABLE");

	if (!chip->cp_disable_votable)
		chip->cp_disable_votable = find_votable("CP_DISABLE");

	if (!chip->fv_votable)
		chip->fv_votable = find_votable("FV");

	if (!batt_psy_initialized(fg)) {
		fg_dbg(fg, FG_STATUS, "Charger not available?!\n");
		goto out;
	}

	if (!fg->soc_reporting_ready) {
		fg_dbg(fg, FG_STATUS, "Profile load is not complete yet\n");
		goto out;
	}

	get_batt_psy_props(fg);

	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &batt_soc);
	if (rc < 0) {
		pr_err("Failed to read battery soc rc: %d\n", rc);
		goto out;
	}

	rc = fg_gen4_get_battery_temp(fg, &batt_temp);
	if (rc < 0) {
		pr_err("Failed to read battery temp rc: %d\n", rc);
		goto out;
	}

	input_present = is_input_present(fg);
	qnovo_en = is_qnovo_en(fg);
	cycle_count_update(chip->counter, (u32)batt_soc >> 24,
		fg->charge_status, fg->charge_done, input_present);

	batt_soc_cp = div64_u64((u64)(u32)batt_soc * CENTI_FULL_SOC,
				BATT_SOC_32BIT);
	cap_learning_update(chip->cl, batt_temp, batt_soc_cp,
			fg->charge_status, fg->charge_done, input_present,
			qnovo_en);

	rc = fg_gen4_charge_full_update(fg);
	if (rc < 0)
		pr_err("Error in charge_full_update, rc=%d\n", rc);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	alarm_start_relative(&fg->somc_fv_rb_alarm_timer, ms_to_ktime(0));

#endif
	rc = fg_gen4_slope_limit_config(chip, batt_temp);
	if (rc < 0)
		pr_err("Error in configuring slope limiter rc:%d\n", rc);

	rc = fg_gen4_adjust_ki_coeff_dischg(fg);
	if (rc < 0)
		pr_err("Error in adjusting ki_coeff_dischg, rc=%d\n", rc);

	rc = fg_gen4_adjust_ki_coeff_full_soc(chip, batt_temp);
	if (rc < 0)
		pr_err("Error in configuring ki_coeff_full_soc rc:%d\n", rc);

	rc = fg_gen4_adjust_recharge_soc(chip);
	if (rc < 0)
		pr_err("Error in adjusting recharge SOC, rc=%d\n", rc);

	rc = fg_gen4_esr_fcc_config(chip);
	if (rc < 0)
		pr_err("Error in adjusting FCC for ESR, rc=%d\n", rc);

	if (is_parallel_charger_available(fg)) {
		cancel_work_sync(&chip->pl_current_en_work);
		schedule_work(&chip->pl_current_en_work);
	}

	rc = fg_gen4_validate_soc_scale_mode(chip);
	if (rc < 0)
		pr_err("Failed to validate SOC scale mode, rc=%d\n", rc);

	ttf_update(chip->ttf, input_present);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_gen4_somc_jeita_step_update(fg);
#endif

out:
	fg_dbg(fg, FG_STATUS, "charge_status:%d charge_type:%d charge_done:%d\n",
		fg->charge_status, fg->charge_type, fg->charge_done);
	pm_relax(fg->dev);
}

static void sram_dump_work(struct work_struct *work)
{
	struct fg_dev *fg = container_of(work, struct fg_dev,
				    sram_dump_work.work);
	u8 *buf;
	int rc;
	s64 timestamp_ms, quotient;
	s32 remainder;

	buf = kcalloc(FG_SRAM_LEN, sizeof(u8), GFP_KERNEL);
	if (!buf)
		goto resched;

	rc = fg_sram_read(fg, 0, 0, buf, FG_SRAM_LEN, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in reading FG SRAM, rc:%d\n", rc);
		kfree(buf);
		goto resched;
	}

	timestamp_ms = ktime_to_ms(ktime_get_boottime());
	quotient = div_s64_rem(timestamp_ms, 1000, &remainder);
	fg_dbg(fg, FG_STATUS, "SRAM Dump Started at %lld.%d\n",
		quotient, remainder);
	dump_sram(fg, buf, 0, FG_SRAM_LEN);
	kfree(buf);
	timestamp_ms = ktime_to_ms(ktime_get_boottime());
	quotient = div_s64_rem(timestamp_ms, 1000, &remainder);
	fg_dbg(fg, FG_STATUS, "SRAM Dump done at %lld.%d\n",
		quotient, remainder);
resched:
	schedule_delayed_work(&fg->sram_dump_work,
			msecs_to_jiffies(fg_sram_dump_period_ms));
}

static ssize_t sram_dump_en_store(struct device *dev, struct device_attribute
		*attr, const char *buf, size_t count)
{
	int rc;
	struct power_supply *bms_psy;
	struct fg_gen4_chip *chip;
	struct fg_dev *fg;
	bool old_val = fg_sram_dump;
	bool store_val;

	if (kstrtobool(buf, &store_val)) {
		pr_err("Unable to set fg_sram_dump\n");
		return -EINVAL;
	}
	fg_sram_dump = store_val;

	if (fg_sram_dump == old_val)
		goto exit;

	bms_psy = power_supply_get_by_name("bms");
	if (!bms_psy) {
		pr_err("bms psy not found\n");
		return -ENODEV;
	}

	chip = power_supply_get_drvdata(bms_psy);
	fg = &chip->fg;

	power_supply_put(bms_psy);
	if (fg->battery_missing) {
		pr_warn("Battery is missing\n");
		goto exit;
	}

	if (fg_sram_dump)
		schedule_delayed_work(&fg->sram_dump_work,
				msecs_to_jiffies(fg_sram_dump_period_ms));
	else
		cancel_delayed_work_sync(&fg->sram_dump_work);
exit:
	rc = count;
	return rc;
}

static ssize_t sram_dump_en_show(struct device *dev, struct device_attribute
		*attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%c\n", fg_sram_dump ? 'Y' : 'N');
}
static DEVICE_ATTR_RW(sram_dump_en);

static ssize_t restart_store(struct device *dev, struct device_attribute
		*attr, const char *buf, size_t count)
{
	int rc;
	struct power_supply *bms_psy;
	struct fg_gen4_chip *chip;
	struct fg_dev *fg;
	int val;

	if (kstrtos32(buf, 10, &val)) {
		pr_err("Unable to set fg_restart_mp\n");
		return -EINVAL;
	}
	fg_restart_mp = val;

	if (fg_restart_mp != 1) {
		pr_err("Bad value %d\n", fg_restart_mp);
		return -EINVAL;
	}

	bms_psy = power_supply_get_by_name("bms");
	if (!bms_psy) {
		pr_err("bms psy not found\n");
		goto exit;
	}

	chip = power_supply_get_drvdata(bms_psy);
	if (!chip)
		return -ENODEV;

	fg = &chip->fg;
	power_supply_put(bms_psy);
	rc = fg_restart(fg, SOC_READY_WAIT_TIME_MS);
	if (rc < 0) {
		pr_err("Error in restarting FG, rc=%d\n", rc);
		return rc;
	}

	pr_info("FG restart done\n");
exit:
	rc = count;
	return rc;
}

static ssize_t restart_show(struct device *dev, struct device_attribute
		*attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", fg_restart_mp);
}
static DEVICE_ATTR_RW(restart);

static ssize_t esr_fast_cal_en_store(struct device *dev, struct device_attribute
		*attr, const char *buf, size_t count)
{
	int rc;
	struct power_supply *bms_psy;
	struct fg_gen4_chip *chip;
	bool old_val = fg_esr_fast_cal_en;
	bool store_val;

	if (kstrtobool(buf, &store_val)) {
		pr_err("Unable to set fg_esr_fast_cal_en\n");
		return -EINVAL;
	}
	fg_esr_fast_cal_en = store_val;

	if (fg_esr_fast_cal_en == old_val)
		goto exit;

	bms_psy = power_supply_get_by_name("bms");
	if (!bms_psy) {
		pr_err("bms psy not found\n");
		return -ENODEV;
	}

	chip = power_supply_get_drvdata(bms_psy);
	power_supply_put(bms_psy);

	if (!chip)
		return -ENODEV;

	if (fg_esr_fast_cal_en)
		chip->delta_esr_count = 0;

	rc = fg_gen4_esr_fast_calib_config(chip, fg_esr_fast_cal_en);
	if (rc < 0)
		return rc;
exit:
	rc = count;
	return rc;
}

static ssize_t esr_fast_cal_en_show(struct device *dev, struct device_attribute
		*attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%c\n", fg_esr_fast_cal_en ? 'Y' : 'N');
}
static DEVICE_ATTR_RW(esr_fast_cal_en);

static struct attribute *fg_attrs[] = {
	&dev_attr_profile_dump.attr,
	&dev_attr_sram_dump_period_ms.attr,
	&dev_attr_sram_dump_en.attr,
	&dev_attr_restart.attr,
	&dev_attr_esr_fast_cal_en.attr,
	NULL,
};
ATTRIBUTE_GROUPS(fg);

/* All power supply functions here */

static int fg_psy_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *pval)
{
	struct fg_gen4_chip *chip = power_supply_get_drvdata(psy);
	struct fg_dev *fg = &chip->fg;
	int rc = 0, val;
	int64_t temp;

	switch (psp) {
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = fg_gen4_get_prop_capacity(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_REAL_CAPACITY:
		rc = fg_gen4_get_prop_real_capacity(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_RAW:
		rc = fg_gen4_get_prop_capacity_raw(chip, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CC_SOC:
		rc = fg_get_sram_prop(&chip->fg, FG_SRAM_CC_SOC, &val);
		if (rc < 0) {
			pr_err("Error in getting CC_SOC, rc=%d\n", rc);
			return rc;
		}
		/* Show it in centi-percentage */
		pval->intval = div_s64((int64_t)val * 10000,  CC_SOC_30BIT);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (fg->battery_missing)
			pval->intval = 3700000;
		else
			rc = fg_get_battery_voltage(fg, &pval->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		rc = fg_get_battery_voltage(fg, &pval->intval);
#endif
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = fg_get_battery_current(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		rc = fg_get_sram_prop(fg, FG_SRAM_IBAT_FLT, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		rc = fg_gen4_get_battery_temp(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_RESISTANCE:
		rc = fg_get_battery_resistance(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_ESR_ACTUAL:
		pval->intval = chip->esr_actual;
		break;
	case POWER_SUPPLY_PROP_ESR_NOMINAL:
		pval->intval = chip->esr_nominal;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		rc = fg_get_sram_prop(fg, FG_SRAM_OCV, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		rc = fg_get_sram_prop(fg, FG_SRAM_VBAT_FLT, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_RESISTANCE_ID:
		pval->intval = fg->batt_id_ohms;
		break;
	case POWER_SUPPLY_PROP_BATTERY_TYPE:
		pval->strval = fg_get_battery_type(fg);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		pval->intval = fg->bp.float_volt_uv;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW_RAW:
		rc = fg_gen4_get_charge_raw(chip, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		pval->intval = chip->cl->init_cap_uah;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		rc = fg_gen4_get_learned_capacity(chip, &temp);
		if (!rc)
			pval->intval = (int)temp;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		rc = fg_gen4_get_nominal_capacity(chip, &temp);
		if (!rc)
			pval->intval = (int)temp;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (fg->initial_capacity >= 0) {
			pval->intval = fg->initial_capacity;
		} else {
			rc = fg_gen4_get_nominal_capacity(chip, &temp);
			if (!rc)
				pval->intval = (int)temp;
		}
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		rc = fg_gen4_get_charge_counter(chip, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER_SHADOW:
		rc = fg_gen4_get_charge_counter_shadow(chip, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		rc = get_cycle_count(chip->counter, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNTS:
		rc = get_cycle_counts(chip->counter, &pval->strval);
		if (rc < 0)
			pval->strval = NULL;
		break;
	case POWER_SUPPLY_PROP_SOC_REPORTING_READY:
		pval->intval = fg->soc_reporting_ready;
		break;
	case POWER_SUPPLY_PROP_CLEAR_SOH:
		pval->intval = chip->first_profile_load;
		break;
	case POWER_SUPPLY_PROP_SOH:
		pval->intval = chip->soh;
		break;
	case POWER_SUPPLY_PROP_DEBUG_BATTERY:
		pval->intval = is_debug_batt_id(fg);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
		rc = fg_get_sram_prop(fg, FG_SRAM_VBATT_FULL, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_TIME_TO_FULL_AVG:
		rc = ttf_get_time_to_full(chip->ttf, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		rc = ttf_get_time_to_full(chip->ttf, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG:
		rc = ttf_get_time_to_empty(chip->ttf, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_CC_STEP:
		if ((chip->ttf->cc_step.sel >= 0) &&
				(chip->ttf->cc_step.sel < MAX_CC_STEPS)) {
			pval->intval =
				chip->ttf->cc_step.arr[chip->ttf->cc_step.sel];
		} else {
			pr_err("cc_step_sel is out of bounds [0, %d]\n",
				chip->ttf->cc_step.sel);
			return -EINVAL;
		}
		break;
	case POWER_SUPPLY_PROP_CC_STEP_SEL:
		pval->intval = chip->ttf->cc_step.sel;
		break;
	case POWER_SUPPLY_PROP_BATT_AGE_LEVEL:
		pval->intval = chip->batt_age_level;
		break;
	case POWER_SUPPLY_PROP_SCALE_MODE_EN:
		pval->intval = chip->soc_scale_mode;
		break;
	case POWER_SUPPLY_PROP_POWER_NOW:
		rc = fg_gen4_get_power(chip, &pval->intval, false);
		break;
	case POWER_SUPPLY_PROP_POWER_AVG:
		rc = fg_gen4_get_power(chip, &pval->intval, true);
		break;
	case POWER_SUPPLY_PROP_CALIBRATE:
		pval->intval = chip->calib_level;
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGE_FULL_RAW:
		pval->intval = chip->cl->final_cap_uah;
		break;
	case POWER_SUPPLY_PROP_LEARNING_COUNTER:
		pval->intval = chip->cl->learning_counter;
		break;
	case POWER_SUPPLY_PROP_LEARNING_TRIAL_COUNTER:
		pval->intval = chip->cl->learning_trial_counter;
		break;
	case POWER_SUPPLY_PROP_RECHARGE_COUNTER:
		pval->intval = fg->recharge_counter;
		break;
	case POWER_SUPPLY_PROP_FULL_COUNTER:
		pval->intval = fg->full_counter;
		break;
	case POWER_SUPPLY_PROP_REAL_TEMP:
		rc = fg_gen4_somc_get_real_temp(fg, &pval->intval);
		break;
	case POWER_SUPPLY_PROP_BATT_AGING_LEVEL:
		pval->intval = fg->batt_aging_level;
		break;
	case POWER_SUPPLY_PROP_REAL_NOM_CAP:
		rc = fg_gen4_get_nominal_capacity(chip, &temp);
		if (!rc)
			pval->intval = (int)temp;
		break;
	case POWER_SUPPLY_PROP_BATTERY_RAW_SOC:
		rc = fg_get_sram_prop(&chip->fg, FG_SRAM_BATT_SOC, &val);
		if (rc < 0) {
			pr_err("Error in getting BATT_SOC, rc=%d\n", rc);
			return rc;
		}
		/* Show it in centi-percentage */
		pval->intval = (int)((u64)((u32)val) * 10000 / BATT_SOC_32BIT);
		break;
#endif
	default:
		pr_err("unsupported property %d\n", psp);
		rc = -EINVAL;
		break;
	}

	if (rc < 0)
		return -ENODATA;

	return 0;
}

static int fg_psy_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *pval)
{
	struct fg_gen4_chip *chip = power_supply_get_drvdata(psy);
	struct fg_dev *fg = &chip->fg;
	int rc = 0;
	u8 val, mask;

	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_FULL:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (chip->cl->active) {
			pr_warn("Capacity learning active!\n");
			return 0;
		}
		if (pval->intval <= 0 || pval->intval > chip->cl->nom_cap_uah) {
			pr_err("charge_full is out of bounds\n");
			return -EINVAL;
		}
		mutex_lock(&chip->cl->lock);
		rc = fg_gen4_store_learned_capacity(chip, pval->intval);
		if (!rc)
			chip->cl->learned_cap_uah = pval->intval;
		mutex_unlock(&chip->cl->lock);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		mutex_lock(&chip->cl->lock);
		chip->cl->learned_cap_uah = pval->intval;
		cap_learning_somc_limit_learned_cap(chip->cl);
		rc = fg_gen4_store_learned_capacity(chip,
				chip->cl->learned_cap_uah);
		mutex_unlock(&chip->cl->lock);
		if (chip->cl->learned_cap_uah == chip->cl->nom_cap_uah)
			fg_restart(fg, SOC_READY_WAIT_TIME_MS);
		if (chip->cl->active)
			cap_learning_abort(chip->cl);
#endif
		break;
	case POWER_SUPPLY_PROP_CC_STEP:
		if ((chip->ttf->cc_step.sel >= 0) &&
				(chip->ttf->cc_step.sel < MAX_CC_STEPS)) {
			chip->ttf->cc_step.arr[chip->ttf->cc_step.sel] =
								pval->intval;
		} else {
			pr_err("cc_step_sel is out of bounds [0, %d]\n",
				chip->ttf->cc_step.sel);
			return -EINVAL;
		}
		break;
	case POWER_SUPPLY_PROP_CC_STEP_SEL:
		if ((pval->intval >= 0) && (pval->intval < MAX_CC_STEPS)) {
			chip->ttf->cc_step.sel = pval->intval;
		} else {
			pr_err("cc_step_sel is out of bounds [0, %d]\n",
				pval->intval);
			return -EINVAL;
		}
		break;
	case POWER_SUPPLY_PROP_ESR_ACTUAL:
		chip->esr_actual = pval->intval;
		break;
	case POWER_SUPPLY_PROP_ESR_NOMINAL:
		chip->esr_nominal = pval->intval;
		break;
	case POWER_SUPPLY_PROP_SOH:
		chip->soh = pval->intval;
		if (chip->sp)
			soh_profile_update(chip->sp, chip->soh);
		break;
	case POWER_SUPPLY_PROP_CLEAR_SOH:
		if (chip->first_profile_load && !pval->intval) {
			fg_dbg(fg, FG_STATUS, "Clearing first profile load bit\n");
			val = 0;
			mask = FIRST_PROFILE_LOAD_BIT;
			rc = fg_sram_masked_write(fg, PROFILE_INTEGRITY_WORD,
					PROFILE_INTEGRITY_OFFSET, mask, val,
					FG_IMA_DEFAULT);
			if (rc < 0)
				pr_err("Error in writing to profile integrity word rc=%d\n",
					rc);
			else
				chip->first_profile_load = false;
		}
		break;
	case POWER_SUPPLY_PROP_BATT_AGE_LEVEL:
		if (!chip->dt.multi_profile_load || pval->intval < 0 ||
			chip->batt_age_level == pval->intval)
			return -EINVAL;
		chip->last_batt_age_level = chip->batt_age_level;
		chip->batt_age_level = pval->intval;
		schedule_delayed_work(&fg->profile_load_work, 0);
		break;
	case POWER_SUPPLY_PROP_CALIBRATE:
		rc = fg_gen4_set_calibrate_level(chip, pval->intval);
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_REAL_TEMP:
		rc = fg_gen4_somc_set_real_temp_debug(&chip->fg, pval->intval);
		if (rc < 0) {
			pr_err("Error in writing real_temp_debug, rc=%d\n", rc);
			return rc;
		}
		break;
	case POWER_SUPPLY_PROP_BATT_AGING_LEVEL:
		rc = somc_fg_gen4_set_batt_aging_level(&chip->fg, pval->intval);
		if (rc < 0)
			return rc;
		break;
#endif
	default:
		break;
	}

	return rc;
}

static int fg_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_FULL:
	case POWER_SUPPLY_PROP_CC_STEP:
	case POWER_SUPPLY_PROP_CC_STEP_SEL:
	case POWER_SUPPLY_PROP_ESR_ACTUAL:
	case POWER_SUPPLY_PROP_ESR_NOMINAL:
	case POWER_SUPPLY_PROP_SOH:
	case POWER_SUPPLY_PROP_CLEAR_SOH:
	case POWER_SUPPLY_PROP_BATT_AGE_LEVEL:
	case POWER_SUPPLY_PROP_CALIBRATE:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_REAL_TEMP:
	case POWER_SUPPLY_PROP_BATT_AGING_LEVEL:
#endif
		return 1;
	default:
		break;
	}

	return 0;
}

static enum power_supply_property fg_psy_props[] = {
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_REAL_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_RAW,
	POWER_SUPPLY_PROP_CC_SOC,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_RESISTANCE_ID,
	POWER_SUPPLY_PROP_RESISTANCE,
	POWER_SUPPLY_PROP_ESR_ACTUAL,
	POWER_SUPPLY_PROP_ESR_NOMINAL,
	POWER_SUPPLY_PROP_BATTERY_TYPE,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_NOW_RAW,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_CHARGE_COUNTER_SHADOW,
	POWER_SUPPLY_PROP_CYCLE_COUNTS,
	POWER_SUPPLY_PROP_SOC_REPORTING_READY,
	POWER_SUPPLY_PROP_CLEAR_SOH,
	POWER_SUPPLY_PROP_SOH,
	POWER_SUPPLY_PROP_DEBUG_BATTERY,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
	POWER_SUPPLY_PROP_TIME_TO_FULL_AVG,
	POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG,
	POWER_SUPPLY_PROP_CC_STEP,
	POWER_SUPPLY_PROP_CC_STEP_SEL,
	POWER_SUPPLY_PROP_BATT_AGE_LEVEL,
	POWER_SUPPLY_PROP_POWER_NOW,
	POWER_SUPPLY_PROP_POWER_AVG,
	POWER_SUPPLY_PROP_SCALE_MODE_EN,
	POWER_SUPPLY_PROP_CALIBRATE,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	POWER_SUPPLY_PROP_CHARGE_FULL_RAW,
	POWER_SUPPLY_PROP_LEARNING_COUNTER,
	POWER_SUPPLY_PROP_LEARNING_TRIAL_COUNTER,
	POWER_SUPPLY_PROP_RECHARGE_COUNTER,
	POWER_SUPPLY_PROP_FULL_COUNTER,
	POWER_SUPPLY_PROP_REAL_TEMP,
	POWER_SUPPLY_PROP_BATT_AGING_LEVEL,
	POWER_SUPPLY_PROP_REAL_NOM_CAP,
	POWER_SUPPLY_PROP_BATTERY_RAW_SOC,
#endif
};

static const struct power_supply_desc fg_psy_desc = {
	.name = "bms",
	.type = POWER_SUPPLY_TYPE_BMS,
	.properties = fg_psy_props,
	.num_properties = ARRAY_SIZE(fg_psy_props),
	.get_property = fg_psy_get_property,
	.set_property = fg_psy_set_property,
	.property_is_writeable = fg_property_is_writeable,
};

/* All callback functions below */

static int fg_notifier_cb(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct power_supply *psy = data;
	struct fg_dev *fg = container_of(nb, struct fg_dev, nb);

	if (event != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (work_pending(&fg->status_change_work))
		return NOTIFY_OK;

	if ((strcmp(psy->desc->name, "battery") == 0)
		|| (strcmp(psy->desc->name, "parallel") == 0)
		|| (strcmp(psy->desc->name, "usb") == 0)) {
		/*
		 * We cannot vote for awake votable here as that takes
		 * a mutex lock and this is executed in an atomic context.
		 */
		pm_stay_awake(fg->dev);
		schedule_work(&fg->status_change_work);
	}

	return NOTIFY_OK;
}

static int fg_awake_cb(struct votable *votable, void *data, int awake,
			const char *client)
{
	struct fg_dev *fg = data;

	if (awake)
		pm_stay_awake(fg->dev);
	else
		pm_relax(fg->dev);

	pr_debug("client: %s awake: %d\n", client, awake);
	return 0;
}

static int fg_gen4_ttf_awake_voter(void *data, bool val)
{
	struct fg_gen4_chip *chip = data;
	struct fg_dev *fg = &chip->fg;

	if (!chip)
		return -ENODEV;

	if (fg->battery_missing ||
		fg->profile_load_status == PROFILE_NOT_LOADED)
		return -EPERM;

	vote(fg->awake_votable, TTF_AWAKE_VOTER, val, 0);
	return 0;
}

static int fg_wait_for_mem_attn(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc, retries = 2;
	ktime_t now;
	s64 time_us;

	reinit_completion(&chip->mem_attn);
	now = ktime_get();

	while (retries--) {
		/* Wait for MEM_ATTN completion */
		rc = wait_for_completion_interruptible_timeout(
			&chip->mem_attn, msecs_to_jiffies(1000));
		if (rc > 0) {
			rc = 0;
			break;
		} else if (!rc) {
			rc = -ETIMEDOUT;
		}
	}

	time_us = ktime_us_delta(ktime_get(), now);
	if (rc < 0)
		pr_err("wait for mem_attn timed out rc=%d\n", rc);

	fg_dbg(fg, FG_STATUS, "mem_attn wait time: %lld us\n", time_us);
	return rc;
}

static int fg_parallel_current_en_cb(struct votable *votable, void *data,
					int enable, const char *client)
{
	struct fg_dev *fg = data;
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	int rc;
	u8 val, mask;

	vote(chip->mem_attn_irq_en_votable, MEM_ATTN_IRQ_VOTER, true, 0);

	/* Wait for MEM_ATTN interrupt */
	rc = fg_wait_for_mem_attn(chip);
	if (rc < 0)
		return rc;

	val = enable ? SMB_MEASURE_EN_BIT : 0;
	mask = SMB_MEASURE_EN_BIT;
	rc = fg_masked_write(fg, BATT_INFO_FG_CNV_CHAR_CFG(fg), mask, val);
	if (rc < 0)
		pr_err("Error in writing to 0x%04x, rc=%d\n",
			BATT_INFO_FG_CNV_CHAR_CFG(fg), rc);

	vote(chip->mem_attn_irq_en_votable, MEM_ATTN_IRQ_VOTER, false, 0);
	fg_dbg(fg, FG_STATUS, "Parallel current summing: %d\n", enable);

	return rc;
}

static int fg_delta_bsoc_irq_en_cb(struct votable *votable, void *data,
					int enable, const char *client)
{
	struct fg_dev *fg = data;

	if (!fg->irqs[BSOC_DELTA_IRQ].irq)
		return 0;

	if (enable) {
		enable_irq(fg->irqs[BSOC_DELTA_IRQ].irq);
		enable_irq_wake(fg->irqs[BSOC_DELTA_IRQ].irq);
	} else {
		disable_irq_wake(fg->irqs[BSOC_DELTA_IRQ].irq);
		disable_irq_nosync(fg->irqs[BSOC_DELTA_IRQ].irq);
	}

	return 0;
}

static int fg_gen4_delta_esr_irq_en_cb(struct votable *votable, void *data,
					int enable, const char *client)
{
	struct fg_dev *fg = data;

	if (!fg->irqs[ESR_DELTA_IRQ].irq)
		return 0;

	if (enable) {
		enable_irq(fg->irqs[ESR_DELTA_IRQ].irq);
		enable_irq_wake(fg->irqs[ESR_DELTA_IRQ].irq);
	} else {
		disable_irq_wake(fg->irqs[ESR_DELTA_IRQ].irq);
		disable_irq_nosync(fg->irqs[ESR_DELTA_IRQ].irq);
	}

	return 0;
}

static int fg_gen4_mem_attn_irq_en_cb(struct votable *votable, void *data,
					int enable, const char *client)
{
	struct fg_dev *fg = data;

	if (!fg->irqs[MEM_ATTN_IRQ].irq)
		return 0;

	if (enable) {
		enable_irq(fg->irqs[MEM_ATTN_IRQ].irq);
		enable_irq_wake(fg->irqs[MEM_ATTN_IRQ].irq);
	} else {
		disable_irq_wake(fg->irqs[MEM_ATTN_IRQ].irq);
		disable_irq_nosync(fg->irqs[MEM_ATTN_IRQ].irq);
	}

	fg_dbg(fg, FG_STATUS, "%sabled mem_attn irq\n", enable ? "en" : "dis");
	return 0;
}

/* All init functions below this */

static int fg_alg_init(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct cycle_counter *counter;
	struct cap_learning *cl;
	struct ttf *ttf;
	int rc;

	counter = devm_kzalloc(fg->dev, sizeof(*counter), GFP_KERNEL);
	if (!counter)
		return -ENOMEM;

	counter->restore_count = fg_gen4_restore_count;
	counter->store_count = fg_gen4_store_count;
	counter->data = chip;

	rc = cycle_count_init(counter);
	if (rc < 0) {
		dev_err(fg->dev, "Error in initializing cycle counter, rc:%d\n",
			rc);
		counter->data = NULL;
		devm_kfree(fg->dev, counter);
		return rc;
	}

	chip->counter = counter;

	cl = devm_kzalloc(fg->dev, sizeof(*cl), GFP_KERNEL);
	if (!cl)
		return -ENOMEM;

	cl->cc_soc_max = CC_SOC_30BIT;
	cl->get_cc_soc = fg_gen4_get_cc_soc_sw;
	cl->prime_cc_soc = fg_gen4_prime_cc_soc_sw;
	cl->get_learned_capacity = fg_gen4_get_learned_capacity;
	cl->store_learned_capacity = fg_gen4_store_learned_capacity;
	cl->ok_to_begin = fg_gen4_cl_ok_to_begin;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cl->get_monotonic_soc = fg_gen4_somc_get_monotonic_soc;
#endif
	cl->data = chip;

	rc = cap_learning_init(cl);
	if (rc < 0) {
		dev_err(fg->dev, "Error in initializing capacity learning, rc:%d\n",
			rc);
		counter->data = NULL;
		cl->data = NULL;
		devm_kfree(fg->dev, counter);
		devm_kfree(fg->dev, cl);
		return rc;
	}

	chip->cl = cl;

	ttf = devm_kzalloc(fg->dev, sizeof(*ttf), GFP_KERNEL);
	if (!ttf)
		return -ENOMEM;

	ttf->get_ttf_param = fg_gen4_get_ttf_param;
	ttf->awake_voter = fg_gen4_ttf_awake_voter;
	ttf->iterm_delta = 0;
	ttf->data = chip;

	rc = ttf_tte_init(ttf);
	if (rc < 0) {
		dev_err(fg->dev, "Error in initializing ttf, rc:%d\n", rc);
		ttf->data = NULL;
		counter->data = NULL;
		cl->data = NULL;
		devm_kfree(fg->dev, ttf);
		devm_kfree(fg->dev, counter);
		devm_kfree(fg->dev, cl);
		return rc;
	}

	chip->ttf = ttf;

	return 0;
}

static int fg_gen4_esr_calib_config(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	u8 buf[2], val, mask;
	int rc;

	if (chip->esr_fast_calib) {
		rc = fg_gen4_esr_fast_calib_config(chip, true);
		if (rc < 0)
			return rc;
	} else {
		if (chip->dt.esr_timer_chg_slow[TIMER_RETRY] >= 0 &&
			chip->dt.esr_timer_chg_slow[TIMER_MAX] >= 0) {
			rc = fg_set_esr_timer(fg,
				chip->dt.esr_timer_chg_slow[TIMER_RETRY],
				chip->dt.esr_timer_chg_slow[TIMER_MAX], true,
				FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error in setting ESR charge timer, rc=%d\n",
					rc);
				return rc;
			}
		}

		if (chip->dt.esr_timer_dischg_slow[TIMER_RETRY] >= 0 &&
			chip->dt.esr_timer_dischg_slow[TIMER_MAX] >= 0) {
			rc = fg_set_esr_timer(fg,
				chip->dt.esr_timer_dischg_slow[TIMER_RETRY],
				chip->dt.esr_timer_dischg_slow[TIMER_MAX],
				false, FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error in setting ESR discharge timer, rc=%d\n",
					rc);
				return rc;
			}
		}

		if (chip->dt.esr_calib_dischg) {
			/* Allow ESR calibration only during discharging */
			val = BIT(6) | BIT(7);
			mask = BIT(1) | BIT(6) | BIT(7);
			rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
					SYS_CONFIG_OFFSET, mask, val,
					FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error in writing SYS_CONFIG_WORD, rc=%d\n",
					rc);
				return rc;
			}

			/* Disable ESR charging timer */
			val = 0;
			mask = BIT(0);
			rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
					SYS_CONFIG2_OFFSET, mask, val,
					FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error in writing SYS_CONFIG2_OFFSET, rc=%d\n",
					rc);
				return rc;
			}
		} else {
			/*
			 * Disable ESR discharging timer and ESR pulsing during
			 * discharging when ESR fast calibration is disabled.
			 */
			val = 0;
			mask = BIT(6) | BIT(7);
			rc = fg_sram_masked_write(fg, SYS_CONFIG_WORD,
					SYS_CONFIG_OFFSET, mask, val,
					FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error in writing SYS_CONFIG_WORD, rc=%d\n",
					rc);
				return rc;
			}
		}
	}

	/*
	 * Delta ESR interrupt threshold should be configured as specified if
	 * ESR fast calibration is disabled. Else, set it to max (4000 mOhms).
	 */
	fg_encode(fg->sp, FG_SRAM_DELTA_ESR_THR,
		chip->esr_fast_calib ? 4000000 : chip->dt.delta_esr_thr_uohms,
		buf);
	rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_DELTA_ESR_THR].addr_word,
			fg->sp[FG_SRAM_DELTA_ESR_THR].addr_byte, buf,
			fg->sp[FG_SRAM_DELTA_ESR_THR].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing DELTA_ESR_THR, rc=%d\n", rc);
		return rc;
	}

	return rc;
}

#define BATT_TEMP_HYST_MASK	GENMASK(3, 0)
#define BATT_TEMP_DELTA_MASK	GENMASK(7, 4)
#define BATT_TEMP_DELTA_SHIFT	4
static int fg_gen4_batt_temp_config(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc;
	u8 buf, val, mask;

	if (chip->dt.batt_temp_cold_thresh != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_BATT_TEMP_COLD,
			chip->dt.batt_temp_cold_thresh, &buf);
		rc = fg_sram_write(fg, fg->sp[FG_SRAM_BATT_TEMP_COLD].addr_word,
				fg->sp[FG_SRAM_BATT_TEMP_COLD].addr_byte, &buf,
				fg->sp[FG_SRAM_BATT_TEMP_COLD].len,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing batt_temp_cold_thresh, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.batt_temp_hot_thresh != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_BATT_TEMP_HOT,
			chip->dt.batt_temp_hot_thresh, &buf);
		rc = fg_sram_write(fg, fg->sp[FG_SRAM_BATT_TEMP_HOT].addr_word,
				fg->sp[FG_SRAM_BATT_TEMP_HOT].addr_byte, &buf,
				fg->sp[FG_SRAM_BATT_TEMP_HOT].len,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing batt_temp_hot_thresh, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.batt_temp_hyst != -EINVAL) {
		val = chip->dt.batt_temp_hyst & BATT_TEMP_HYST_MASK;
		mask = BATT_TEMP_HYST_MASK;
		rc = fg_sram_masked_write(fg, BATT_TEMP_CONFIG2_WORD,
				BATT_TEMP_HYST_DELTA_OFFSET, mask, val,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing batt_temp_hyst, rc=%d\n", rc);
			return rc;
		}
	}

	if (chip->dt.batt_temp_delta != -EINVAL) {
		val = (chip->dt.batt_temp_delta << BATT_TEMP_DELTA_SHIFT)
				& BATT_TEMP_DELTA_MASK;
		mask = BATT_TEMP_DELTA_MASK;
		rc = fg_sram_masked_write(fg, BATT_TEMP_CONFIG2_WORD,
				BATT_TEMP_HYST_DELTA_OFFSET, mask, val,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing batt_temp_delta, rc=%d\n", rc);
			return rc;
		}
	}

	val = (u8)chip->dt.batt_therm_freq;
	rc = fg_write(fg, ADC_RR_BATT_THERM_FREQ(fg), &val, 1);
	if (rc < 0) {
		pr_err("failed to write to 0x%04X, rc=%d\n",
			 ADC_RR_BATT_THERM_FREQ(fg), rc);
		return rc;
	}

	return rc;
}

static int fg_gen4_init_ki_coeffts(struct fg_gen4_chip *chip)
{
	int rc;
	u8 val;
	struct fg_dev *fg = &chip->fg;

	if (chip->dt.ki_coeff_low_chg != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_LOW_CHG,
			chip->dt.ki_coeff_low_chg, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_LOW_CHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_LOW_CHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_LOW_CHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_low_chg, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_med_chg != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_MED_CHG,
			chip->dt.ki_coeff_med_chg, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_MED_CHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_MED_CHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_MED_CHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_med_chg, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_hi_chg != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_HI_CHG,
			chip->dt.ki_coeff_hi_chg, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_HI_CHG].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_HI_CHG].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_HI_CHG].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_hi_chg, rc=%d\n", rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_lo_med_chg_thr_ma != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_LO_MED_CHG_THR,
			chip->dt.ki_coeff_lo_med_chg_thr_ma, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_LO_MED_CHG_THR].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_LO_MED_CHG_THR].addr_byte,
			&val, fg->sp[FG_SRAM_KI_COEFF_LO_MED_CHG_THR].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_lo_med_chg_thr_ma, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_med_hi_chg_thr_ma != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_MED_HI_CHG_THR,
			chip->dt.ki_coeff_med_hi_chg_thr_ma, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_MED_HI_CHG_THR].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_MED_HI_CHG_THR].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_MED_HI_CHG_THR].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_med_hi_chg_thr_ma, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_lo_med_dchg_thr_ma != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_LO_MED_DCHG_THR,
			chip->dt.ki_coeff_lo_med_dchg_thr_ma, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_LO_MED_DCHG_THR].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_LO_MED_DCHG_THR].addr_byte,
			&val, fg->sp[FG_SRAM_KI_COEFF_LO_MED_DCHG_THR].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_lo_med_dchg_thr_ma, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_med_hi_dchg_thr_ma != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_MED_HI_DCHG_THR,
			chip->dt.ki_coeff_med_hi_dchg_thr_ma, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_MED_HI_DCHG_THR].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_MED_HI_DCHG_THR].addr_byte,
			&val, fg->sp[FG_SRAM_KI_COEFF_MED_HI_DCHG_THR].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_med_hi_dchg_thr_ma, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (chip->dt.ki_coeff_cutoff_gain != -EINVAL) {
		fg_encode(fg->sp, FG_SRAM_KI_COEFF_CUTOFF,
			  chip->dt.ki_coeff_cutoff_gain, &val);
		rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_KI_COEFF_CUTOFF].addr_word,
			fg->sp[FG_SRAM_KI_COEFF_CUTOFF].addr_byte, &val,
			fg->sp[FG_SRAM_KI_COEFF_CUTOFF].len,
			FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing ki_coeff_cutoff_gain, rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = fg_gen4_set_ki_coeff_dischg(fg, KI_COEFF_LOW_DISCHG_DEFAULT,
		KI_COEFF_MED_DISCHG_DEFAULT, KI_COEFF_HI_DISCHG_DEFAULT);
	if (rc < 0)
		return rc;

	return 0;
}

#define VBATT_TAU_DEFAULT	3
static int fg_gen4_hw_init(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	u8 buf[4], val;
	int rc;

	rc = fg_read(fg, ADC_RR_INT_RT_STS(fg), &val, 1);
	if (rc < 0) {
		pr_err("failed to read addr=0x%04x, rc=%d\n",
			ADC_RR_INT_RT_STS(fg), rc);
		return rc;
	}
	fg->battery_missing = (val & ADC_RR_BT_MISS_BIT);

	if (fg->battery_missing) {
		pr_warn("Not initializing FG because of battery missing\n");
		return 0;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	val = BATT_THERM_FREQ_1S;
	rc = fg_write(fg, ADC_RR_BATT_THERM_FREQ(fg), &val, 1);
	if (rc < 0) {
		pr_err("failed to write to 0x%04X, rc=%d\n",
					ADC_RR_BATT_THERM_FREQ(fg), rc);
		return rc;
	}

#endif
	fg_encode(fg->sp, FG_SRAM_CUTOFF_VOLT, chip->dt.cutoff_volt_mv, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_CUTOFF_VOLT].addr_word,
			fg->sp[FG_SRAM_CUTOFF_VOLT].addr_byte, buf,
			fg->sp[FG_SRAM_CUTOFF_VOLT].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing cutoff_volt, rc=%d\n", rc);
		return rc;
	}

	rc = fg_gen4_configure_cutoff_current(fg, chip->dt.cutoff_curr_ma);
	if (rc < 0)
		return rc;

	fg_encode(fg->sp, FG_SRAM_SYS_TERM_CURR, chip->dt.sys_term_curr_ma,
		buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_SYS_TERM_CURR].addr_word,
			fg->sp[FG_SRAM_SYS_TERM_CURR].addr_byte, buf,
			fg->sp[FG_SRAM_SYS_TERM_CURR].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing sys_term_curr, rc=%d\n", rc);
		return rc;
	}

	if (chip->dt.empty_volt_mv > 0) {
		fg_encode(fg->sp, FG_SRAM_VBATT_LOW,
			chip->dt.empty_volt_mv, buf);
		rc = fg_sram_write(fg, fg->sp[FG_SRAM_VBATT_LOW].addr_word,
				fg->sp[FG_SRAM_VBATT_LOW].addr_byte, buf,
				fg->sp[FG_SRAM_VBATT_LOW].len,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in writing empty_volt_mv, rc=%d\n", rc);
			return rc;
		}
	}

	fg_encode(fg->sp, FG_SRAM_DELTA_MSOC_THR,
		chip->dt.delta_soc_thr, buf);
	rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_DELTA_MSOC_THR].addr_word,
			fg->sp[FG_SRAM_DELTA_MSOC_THR].addr_byte,
			buf, fg->sp[FG_SRAM_DELTA_MSOC_THR].len,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing delta_msoc_thr, rc=%d\n", rc);
		return rc;
	}

	fg_encode(fg->sp, FG_SRAM_DELTA_BSOC_THR,
		chip->dt.delta_soc_thr, buf);
	rc = fg_sram_write(fg,
			fg->sp[FG_SRAM_DELTA_BSOC_THR].addr_word,
			fg->sp[FG_SRAM_DELTA_BSOC_THR].addr_byte,
			buf, fg->sp[FG_SRAM_DELTA_BSOC_THR].len,
			FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing delta_bsoc_thr, rc=%d\n", rc);
		return rc;
	}

	rc = fg_gen4_batt_temp_config(chip);
	if (rc < 0)
		return rc;

	fg_encode(fg->sp, FG_SRAM_ESR_PULSE_THRESH,
		chip->dt.esr_pulse_thresh_ma, buf);
	rc = fg_sram_write(fg, fg->sp[FG_SRAM_ESR_PULSE_THRESH].addr_word,
			fg->sp[FG_SRAM_ESR_PULSE_THRESH].addr_byte, buf,
			fg->sp[FG_SRAM_ESR_PULSE_THRESH].len, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("Error in writing esr_pulse_thresh_ma, rc=%d\n", rc);
		return rc;
	}

	get_esr_meas_current(chip->dt.esr_meas_curr_ma, &val);
	rc = fg_masked_write(fg, BATT_INFO_ESR_PULL_DN_CFG(fg),
			ESR_PULL_DOWN_IVAL_MASK, val);
	if (rc < 0) {
		pr_err("Error in writing esr_meas_curr_ma, rc=%d\n", rc);
		return rc;
	}

	if (is_debug_batt_id(fg)) {
		val = ESR_NO_PULL_DOWN;
		rc = fg_masked_write(fg, BATT_INFO_ESR_PULL_DN_CFG(fg),
			ESR_PULL_DOWN_MODE_MASK, val);
		if (rc < 0) {
			pr_err("Error in writing esr_pull_down, rc=%d\n", rc);
			return rc;
		}
	}

	if (chip->dt.rconn_uohms) {
		/*
		 * Read back Rconn to see if it's already configured. If it is
		 * a non-zero value, then skip configuring it.
		 */
		rc = fg_sram_read(fg, RCONN_WORD, RCONN_OFFSET, buf, 2,
				FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error reading Rconn, rc=%d\n", rc);
			return rc;
		}

		if (!buf[0] && !buf[1]) {
			fg_encode(fg->sp, FG_SRAM_RCONN, chip->dt.rconn_uohms,
				buf);
			rc = fg_sram_write(fg, RCONN_WORD, RCONN_OFFSET, buf, 2,
					FG_IMA_DEFAULT);
			if (rc < 0) {
				pr_err("Error writing Rconn, rc=%d\n", rc);
				return rc;
			}
		} else {
			pr_debug("Skipping configuring Rconn [0x%x 0x%x]\n",
				buf[0], buf[1]);
		}
	}

	rc = fg_gen4_init_ki_coeffts(chip);
	if (rc < 0)
		return rc;

	rc = fg_gen4_esr_calib_config(chip);
	if (rc < 0)
		return rc;

	chip->batt_age_level = chip->last_batt_age_level = -EINVAL;
	if (chip->dt.multi_profile_load) {
		rc = fg_sram_read(fg, BATT_AGE_LEVEL_WORD,
			BATT_AGE_LEVEL_OFFSET, &val, 1, FG_IMA_DEFAULT);
		if (!rc)
			chip->batt_age_level = chip->last_batt_age_level = val;
	}

	if (chip->dt.soc_scale_mode) {
		rc = fg_gen4_set_vbatt_tau(chip, VBATT_TAU_DEFAULT);
		if (rc < 0) {
			fg_gen4_exit_soc_scale(chip);
			return rc;
		}
	}

	return 0;
}

static int fg_parse_slope_limit_coefficients(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	struct device_node *node = fg->dev->of_node;
	int rc, i;

	if (!of_find_property(node, "qcom,slope-limit-coeffs", NULL))
		return 0;

	rc = of_property_read_u32(node, "qcom,slope-limit-temp-threshold",
			&chip->dt.slope_limit_temp);
	if (rc < 0)
		return 0;

	rc = fg_parse_dt_property_u32_array(node, "qcom,slope-limit-coeffs",
		chip->dt.slope_limit_coeffs, SLOPE_LIMIT_NUM_COEFFS);
	if (rc < 0)
		return rc;

	for (i = 0; i < SLOPE_LIMIT_NUM_COEFFS; i++) {
		if (chip->dt.slope_limit_coeffs[i] > SLOPE_LIMIT_COEFF_MAX ||
			chip->dt.slope_limit_coeffs[i] < 0) {
			pr_err("Incorrect slope limit coefficient\n");
			return -EINVAL;
		}
	}

	chip->slope_limit_en = true;
	return 0;
}

static int fg_parse_ki_coefficients(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	struct device_node *node = fg->dev->of_node;
	int rc, i;

	chip->dt.ki_coeff_full_soc_dischg[0] = KI_COEFF_FULL_SOC_NORM_DEFAULT;
	chip->dt.ki_coeff_full_soc_dischg[1] = KI_COEFF_FULL_SOC_LOW_DEFAULT;

	if (of_find_property(node, "qcom,ki-coeff-full-dischg", NULL)) {
		rc = fg_parse_dt_property_u32_array(node,
			"qcom,ki-coeff-full-dischg",
			chip->dt.ki_coeff_full_soc_dischg, 2);
		if (rc < 0)
			return rc;

		if (chip->dt.ki_coeff_full_soc_dischg[0] < 62 ||
			chip->dt.ki_coeff_full_soc_dischg[0] > 15564 ||
			chip->dt.ki_coeff_full_soc_dischg[1] < 62 ||
			chip->dt.ki_coeff_full_soc_dischg[1] > 15564) {
			pr_err("Error in ki_coeff_full_soc_dischg values\n");
			return -EINVAL;
		}
	}

	chip->dt.ki_coeff_low_chg = 184;
	of_property_read_u32(node, "qcom,ki-coeff-low-chg",
		&chip->dt.ki_coeff_low_chg);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_chg = 62;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_chg = 123;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-med-chg",
		&chip->dt.ki_coeff_med_chg);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_hi_chg = 0;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_hi_chg = 62;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-hi-chg",
		&chip->dt.ki_coeff_hi_chg);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_lo_med_chg_thr_ma = 500;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_lo_med_chg_thr_ma = 63;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-chg-low-med-thresh-ma",
		&chip->dt.ki_coeff_lo_med_chg_thr_ma);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_hi_chg_thr_ma = 1000;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_hi_chg_thr_ma = 500;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-chg-med-hi-thresh-ma",
		&chip->dt.ki_coeff_med_hi_chg_thr_ma);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_lo_med_dchg_thr_ma = 50;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_lo_med_dchg_thr_ma = 63;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-dischg-low-med-thresh-ma",
		&chip->dt.ki_coeff_lo_med_dchg_thr_ma);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_hi_dchg_thr_ma = 100;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chip->dt.ki_coeff_med_hi_dchg_thr_ma = 500;
#endif
	of_property_read_u32(node, "qcom,ki-coeff-dischg-med-hi-thresh-ma",
		&chip->dt.ki_coeff_med_hi_dchg_thr_ma);

	chip->dt.ki_coeff_cutoff_gain = -EINVAL;
	of_property_read_u32(node, "qcom,ki-coeff-cutoff",
		&chip->dt.ki_coeff_cutoff_gain);

	for (i = 0; i < KI_COEFF_SOC_LEVELS; i++) {
		chip->dt.ki_coeff_low_dischg[i] = KI_COEFF_LOW_DISCHG_DEFAULT;
		chip->dt.ki_coeff_med_dischg[i] = KI_COEFF_MED_DISCHG_DEFAULT;
		chip->dt.ki_coeff_hi_dischg[i] = KI_COEFF_HI_DISCHG_DEFAULT;
	}

	if (!of_find_property(node, "qcom,ki-coeff-soc-dischg", NULL) ||
		(!of_find_property(node, "qcom,ki-coeff-low-dischg", NULL) &&
		!of_find_property(node, "qcom,ki-coeff-med-dischg", NULL) &&
		!of_find_property(node, "qcom,ki-coeff-hi-dischg", NULL)))
		return 0;

	rc = fg_parse_dt_property_u32_array(node, "qcom,ki-coeff-soc-dischg",
		chip->dt.ki_coeff_soc, KI_COEFF_SOC_LEVELS);
	if (rc < 0)
		return rc;

	rc = fg_parse_dt_property_u32_array(node, "qcom,ki-coeff-low-dischg",
		chip->dt.ki_coeff_low_dischg, KI_COEFF_SOC_LEVELS);
	if (rc < 0)
		return rc;

	rc = fg_parse_dt_property_u32_array(node, "qcom,ki-coeff-med-dischg",
		chip->dt.ki_coeff_med_dischg, KI_COEFF_SOC_LEVELS);
	if (rc < 0)
		return rc;

	rc = fg_parse_dt_property_u32_array(node, "qcom,ki-coeff-hi-dischg",
		chip->dt.ki_coeff_hi_dischg, KI_COEFF_SOC_LEVELS);
	if (rc < 0)
		return rc;

	for (i = 0; i < KI_COEFF_SOC_LEVELS; i++) {
		if (chip->dt.ki_coeff_soc[i] < 0 ||
			chip->dt.ki_coeff_soc[i] > FULL_CAPACITY) {
			pr_err("Error in ki_coeff_soc_dischg values\n");
			return -EINVAL;
		}

		if (chip->dt.ki_coeff_low_dischg[i] < 0 ||
			chip->dt.ki_coeff_low_dischg[i] > KI_COEFF_MAX) {
			pr_err("Error in ki_coeff_low_dischg values\n");
			return -EINVAL;
		}

		if (chip->dt.ki_coeff_med_dischg[i] < 0 ||
			chip->dt.ki_coeff_med_dischg[i] > KI_COEFF_MAX) {
			pr_err("Error in ki_coeff_med_dischg values\n");
			return -EINVAL;
		}

		if (chip->dt.ki_coeff_hi_dischg[i] < 0 ||
			chip->dt.ki_coeff_hi_dischg[i] > KI_COEFF_MAX) {
			pr_err("Error in ki_coeff_hi_dischg values\n");
			return -EINVAL;
		}
	}
	chip->ki_coeff_dischg_en = true;
	return 0;
}

#define DEFAULT_ESR_DISABLE_COUNT	5
#define DEFAULT_ESR_FILTER_FACTOR	2
#define DEFAULT_DELTA_ESR_THR		1832
static int fg_parse_esr_cal_params(struct fg_dev *fg)
{
	struct fg_gen4_chip *chip = container_of(fg, struct fg_gen4_chip, fg);
	struct device_node *node = fg->dev->of_node;
	int rc, i, temp;

	if (!of_find_property(node, "qcom,fg-esr-cal-soc-thresh", NULL) ||
		!of_find_property(node, "qcom,fg-esr-cal-temp-thresh", NULL))
		return 0;

	rc = fg_parse_dt_property_u32_array(node, "qcom,fg-esr-cal-soc-thresh",
		chip->dt.esr_cal_soc_thresh, ESR_CAL_LEVELS);
	if (rc < 0) {
		pr_err("Invalid SOC thresholds for ESR fast cal, rc=%d\n", rc);
		return rc;
	}

	rc = fg_parse_dt_property_u32_array(node, "qcom,fg-esr-cal-temp-thresh",
		chip->dt.esr_cal_temp_thresh, ESR_CAL_LEVELS);
	if (rc < 0) {
		pr_err("Invalid temperature thresholds for ESR fast cal, rc=%d\n",
			rc);
		return rc;
	}

	for (i = 0; i < ESR_CAL_LEVELS; i++) {
		if (chip->dt.esr_cal_soc_thresh[i] > FULL_SOC_RAW) {
			pr_err("esr_cal_soc_thresh value shouldn't exceed %d\n",
				FULL_SOC_RAW);
			return -EINVAL;
		}

		if (chip->dt.esr_cal_temp_thresh[i] < ESR_CAL_TEMP_MIN ||
			chip->dt.esr_cal_temp_thresh[i] > ESR_CAL_TEMP_MAX) {
			pr_err("esr_cal_temp_thresh value should be within [%d %d]\n",
				ESR_CAL_TEMP_MIN, ESR_CAL_TEMP_MAX);
			return -EINVAL;
		}
	}

	if (chip->dt.esr_timer_dischg_slow[TIMER_RETRY] >= 0 &&
			chip->dt.esr_timer_dischg_slow[TIMER_MAX] >= 0) {
		/* ESR calibration only during discharging */
		chip->dt.esr_calib_dischg = of_property_read_bool(node,
						"qcom,fg-esr-calib-dischg");
		if (chip->dt.esr_calib_dischg)
			return 0;
	}

	chip->dt.delta_esr_disable_count = DEFAULT_ESR_DISABLE_COUNT;
	rc = of_property_read_u32(node, "qcom,fg-delta-esr-disable-count",
		&temp);
	if (!rc)
		chip->dt.delta_esr_disable_count = temp;

	chip->dt.esr_filter_factor = DEFAULT_ESR_FILTER_FACTOR;
	rc = of_property_read_u32(node, "qcom,fg-esr-filter-factor",
		&temp);
	if (!rc)
		chip->dt.esr_filter_factor = temp;

	chip->dt.delta_esr_thr_uohms = DEFAULT_DELTA_ESR_THR;
	rc = of_property_read_u32(node, "qcom,fg-delta-esr-thr", &temp);
	if (!rc)
		chip->dt.delta_esr_thr_uohms = temp;

	chip->esr_fast_calib = true;
	return 0;
}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)

#define MAX_AGING_LEVEL_NODE 10
static void somc_fg_gen4_check_max_batt_aging_level(struct fg_dev *fg)
{
	struct device_node *node = fg->dev->of_node;
	char batt_node_name[MAX_BATT_NODE_NAME_SIZE];
	ssize_t size;
	int i;

	for (i = 0; i < MAX_AGING_LEVEL_NODE; i++) {
		size = scnprintf(batt_node_name, sizeof(batt_node_name),
						"somc,battery-data-al%d", i);
		if (size < 0)
			break;

		if (!of_find_node_by_name(node, batt_node_name))
			break;
	}

	if (i > 0) {
		fg->max_batt_aging_level = i - 1;
	} else {
		fg_dbg(fg, FG_SOMC, "There is no one somc batterydata node\n");
		fg->max_batt_aging_level = BATT_AGING_LEVEL_NONE;
	}
}
#endif

#define BTEMP_DELTA_LOW			0
#define BTEMP_DELTA_HIGH		3

static void fg_gen4_parse_batt_temp_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct device_node *node = fg->dev->of_node;
	int rc, temp;

	rc = of_property_read_u32(node, "qcom,fg-batt-temp-hot", &temp);
	if (rc < 0)
		chip->dt.batt_temp_hot_thresh = -EINVAL;
	else
		chip->dt.batt_temp_hot_thresh = temp;

	rc = of_property_read_u32(node, "qcom,fg-batt-temp-cold", &temp);
	if (rc < 0)
		chip->dt.batt_temp_cold_thresh = -EINVAL;
	else
		chip->dt.batt_temp_cold_thresh = temp;

	rc = of_property_read_u32(node, "qcom,fg-batt-temp-hyst", &temp);
	if (rc < 0)
		chip->dt.batt_temp_hyst = -EINVAL;
	else if (temp >= BTEMP_DELTA_LOW && temp <= BTEMP_DELTA_HIGH)
		chip->dt.batt_temp_hyst = temp;

	rc = of_property_read_u32(node, "qcom,fg-batt-temp-delta", &temp);
	if (rc < 0)
		chip->dt.batt_temp_delta = -EINVAL;
	else if (temp >= BTEMP_DELTA_LOW && temp <= BTEMP_DELTA_HIGH)
		chip->dt.batt_temp_delta = temp;

	chip->dt.batt_therm_freq = 8;
	rc = of_property_read_u32(node, "qcom,fg-batt-therm-freq", &temp);
	if (temp > 0 && temp <= 255)
		chip->dt.batt_therm_freq = temp;
}

static int fg_gen4_parse_nvmem_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	int rc;

	if (of_find_property(fg->dev->of_node, "nvmem", NULL)) {
		chip->fg_nvmem = devm_nvmem_device_get(fg->dev, "fg_sdam");
		if (IS_ERR_OR_NULL(chip->fg_nvmem)) {
			rc = PTR_ERR(chip->fg_nvmem);
			if (rc != -EPROBE_DEFER) {
				dev_err(fg->dev, "Couldn't get nvmem device, rc=%d\n",
					rc);
				return -ENODEV;
			}
			chip->fg_nvmem = NULL;
			return rc;
		}
	}

	return 0;
}

#define DEFAULT_CL_START_SOC		15
#define DEFAULT_CL_MIN_TEMP_DECIDEGC	150
#define DEFAULT_CL_MAX_TEMP_DECIDEGC	500
#define DEFAULT_CL_MAX_INC_DECIPERC	5
#define DEFAULT_CL_MAX_DEC_DECIPERC	100
#define DEFAULT_CL_MIN_LIM_DECIPERC	0
#define DEFAULT_CL_MAX_LIM_DECIPERC	0
#define DEFAULT_CL_DELTA_BATT_SOC	10

static void fg_gen4_parse_cl_params_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct device_node *node = fg->dev->of_node;

	chip->cl->dt.max_start_soc = DEFAULT_CL_START_SOC;
	of_property_read_u32(node, "qcom,cl-start-capacity",
				&chip->cl->dt.max_start_soc);

	chip->cl->dt.min_delta_batt_soc = DEFAULT_CL_DELTA_BATT_SOC;
	/* read from DT property and update, if value exists */
	of_property_read_u32(node, "qcom,cl-min-delta-batt-soc",
					&chip->cl->dt.min_delta_batt_soc);

	chip->cl->dt.min_temp = DEFAULT_CL_MIN_TEMP_DECIDEGC;
	of_property_read_u32(node, "qcom,cl-min-temp", &chip->cl->dt.min_temp);

	chip->cl->dt.max_temp = DEFAULT_CL_MAX_TEMP_DECIDEGC;
	of_property_read_u32(node, "qcom,cl-max-temp", &chip->cl->dt.max_temp);

	chip->cl->dt.max_cap_inc = DEFAULT_CL_MAX_INC_DECIPERC;
	of_property_read_u32(node, "qcom,cl-max-increment",
				&chip->cl->dt.max_cap_inc);

	chip->cl->dt.max_cap_dec = DEFAULT_CL_MAX_DEC_DECIPERC;
	of_property_read_u32(node, "qcom,cl-max-decrement",
				&chip->cl->dt.max_cap_dec);

	chip->cl->dt.min_cap_limit = DEFAULT_CL_MIN_LIM_DECIPERC;
	of_property_read_u32(node, "qcom,cl-min-limit",
				&chip->cl->dt.min_cap_limit);

	chip->cl->dt.max_cap_limit = DEFAULT_CL_MAX_LIM_DECIPERC;
	of_property_read_u32(node, "qcom,cl-max-limit",
				&chip->cl->dt.max_cap_limit);

	of_property_read_u32(node, "qcom,cl-skew", &chip->cl->dt.skew_decipct);

	if (of_property_read_bool(node, "qcom,cl-wt-enable")) {
		chip->cl->dt.cl_wt_enable = true;
		chip->cl->dt.max_start_soc = -EINVAL;
		chip->cl->dt.min_start_soc = -EINVAL;
	}

	chip->cl->dt.ibat_flt_thr_ma = 100;
	of_property_read_u32(node, "qcom,cl-ibat-flt-thresh-ma",
		&chip->cl->dt.ibat_flt_thr_ma);
}

static int fg_gen4_parse_revid_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct device_node *revid_node, *node = fg->dev->of_node;

	revid_node = of_parse_phandle(node, "qcom,pmic-revid", 0);
	if (!revid_node) {
		pr_err("Missing qcom,pmic-revid property - driver failed\n");
		return -EINVAL;
	}

	fg->pmic_rev_id = get_revid_data(revid_node);
	of_node_put(revid_node);
	if (IS_ERR_OR_NULL(fg->pmic_rev_id)) {
		pr_err("Unable to get pmic_revid rc=%ld\n",
			PTR_ERR(fg->pmic_rev_id));
		/*
		 * the revid peripheral must be registered, any failure
		 * here only indicates that the rev-id module has not
		 * probed yet.
		 */
		return -EPROBE_DEFER;
	}

	pr_debug("PMIC subtype %d Digital major %d\n",
		fg->pmic_rev_id->pmic_subtype, fg->pmic_rev_id->rev4);

	switch (fg->pmic_rev_id->pmic_subtype) {
	case PM8150B_SUBTYPE:
		fg->version = GEN4_FG;
		fg->use_dma = true;
		fg->sp = pm8150b_v2_sram_params;
		if (fg->pmic_rev_id->rev4 == PM8150B_V1P0_REV4) {
			fg->sp = pm8150b_v1_sram_params;
			fg->wa_flags |= PM8150B_V1_DMA_WA;
			fg->wa_flags |= PM8150B_V1_RSLOW_COMP_WA;
		} else if (fg->pmic_rev_id->rev4 == PM8150B_V2P0_REV4) {
			fg->wa_flags |= PM8150B_V2_RSLOW_SCALE_FN_WA;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int fg_gen4_parse_child_nodes_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct device_node *child, *node = fg->dev->of_node;
	u32 base;
	u8 subtype;
	int rc;

	if (of_get_available_child_count(node) == 0) {
		dev_err(fg->dev, "No child nodes specified!\n");
		return -ENXIO;
	}

	for_each_available_child_of_node(node, child) {
		rc = of_property_read_u32(child, "reg", &base);
		if (rc < 0) {
			dev_err(fg->dev, "reg not specified in node %s, rc=%d\n",
				child->full_name, rc);
			return rc;
		}

		rc = fg_read(fg, base + PERPH_SUBTYPE_REG, &subtype, 1);
		if (rc < 0) {
			dev_err(fg->dev, "Couldn't read subtype for base %d, rc=%d\n",
				base, rc);
			return rc;
		}

		switch (subtype) {
		case FG_BATT_SOC_PM8150B:
			fg->batt_soc_base = base;
			break;
		case FG_BATT_INFO_PM8150B:
			fg->batt_info_base = base;
			break;
		case FG_MEM_IF_PM8150B:
			fg->mem_if_base = base;
			break;
		case FG_ADC_RR_PM8150B:
			fg->rradc_base = base;
			break;
		default:
			dev_err(fg->dev, "Invalid peripheral subtype 0x%x\n",
				subtype);
			return -ENXIO;
		}
	}

	return 0;
}

#define DEFAULT_CUTOFF_VOLT_MV		3100
#define DEFAULT_EMPTY_VOLT_MV		2812
#define DEFAULT_SYS_MIN_VOLT_MV		2800
#define DEFAULT_SYS_TERM_CURR_MA	-125
#define DEFAULT_CUTOFF_CURR_MA		200
#define DEFAULT_DELTA_SOC_THR		5	/* 0.5 % */
#define DEFAULT_ESR_PULSE_THRESH_MA	47
#define DEFAULT_ESR_MEAS_CURR_MA	120
#define DEFAULT_SCALE_VBATT_THR_MV	3400
#define DEFAULT_SCALE_ALARM_TIMER_MS	10000
#define DEFAULT_BATT_ID_PULLUP_KOHMS	100
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define DEFAULT_TEMP_CORR_COEF_A	70
#define DEFAULT_TEMP_CORR_AGV_SEC	60
#define SOC_16BIT		GENMASK(15, 0)
#endif

static int fg_gen4_parse_dt(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;
	struct device_node *node = fg->dev->of_node;
	u32 temp;
	int rc;

	if (!node)  {
		dev_err(fg->dev, "device tree node missing\n");
		return -ENXIO;
	}

	rc = fg_gen4_parse_revid_dt(chip);
	if (rc < 0)
		return rc;

	if (of_find_property(node, "qcom,pmic-pbs", NULL)) {
		chip->pbs_dev = of_parse_phandle(node, "qcom,pmic-pbs", 0);
		if (!chip->pbs_dev) {
			pr_err("Missing qcom,pmic-pbs property\n");
			return -ENODEV;
		}
	}

	rc = fg_gen4_parse_nvmem_dt(chip);
	if (rc < 0)
		return rc;

	rc = of_property_match_string(fg->dev->of_node, "io-channel-names",
					"batt_id");
	if (rc >= 0) {
		chip->batt_id_chan = devm_iio_channel_get(fg->dev, "batt_id");
		if (IS_ERR(chip->batt_id_chan)) {
			rc = PTR_ERR(chip->batt_id_chan);
			if (rc != -EPROBE_DEFER)
				pr_err("Couldn't get batt_id_chan rc=%d\n", rc);
			chip->batt_id_chan = NULL;
			return rc;
		}
	}

	rc = fg_gen4_parse_child_nodes_dt(chip);
	if (rc < 0)
		return rc;

	/* Read all the optional properties below */
	chip->dt.cutoff_volt_mv = DEFAULT_CUTOFF_VOLT_MV;
	of_property_read_u32(node, "qcom,fg-cutoff-voltage",
				&chip->dt.cutoff_volt_mv);

	chip->dt.cutoff_curr_ma = DEFAULT_CUTOFF_CURR_MA;
	of_property_read_u32(node, "qcom,fg-cutoff-current",
				&chip->dt.cutoff_curr_ma);

	chip->dt.empty_volt_mv = DEFAULT_EMPTY_VOLT_MV;
	of_property_read_u32(node, "qcom,fg-empty-voltage",
				&chip->dt.empty_volt_mv);

	chip->dt.sys_term_curr_ma = DEFAULT_SYS_TERM_CURR_MA;
	of_property_read_u32(node, "qcom,fg-sys-term-current",
				&chip->dt.sys_term_curr_ma);

	chip->dt.delta_soc_thr = DEFAULT_DELTA_SOC_THR;
	of_property_read_u32(node, "qcom,fg-delta-soc-thr",
				&chip->dt.delta_soc_thr);

	if (chip->dt.delta_soc_thr < 0 || chip->dt.delta_soc_thr >= 125) {
		pr_err("Invalid delta SOC threshold=%d\n",
		       chip->dt.delta_soc_thr);
		return -EINVAL;
	}

	chip->dt.esr_timer_chg_fast[TIMER_RETRY] = -EINVAL;
	chip->dt.esr_timer_chg_fast[TIMER_MAX] = -EINVAL;
	rc = fg_parse_dt_property_u32_array(node, "qcom,fg-esr-timer-chg-fast",
		chip->dt.esr_timer_chg_fast, NUM_ESR_TIMERS);
	if (rc < 0)
		return rc;

	chip->dt.esr_timer_dischg_fast[TIMER_RETRY] = -EINVAL;
	chip->dt.esr_timer_dischg_fast[TIMER_MAX] = -EINVAL;
	rc = fg_parse_dt_property_u32_array(node,
		"qcom,fg-esr-timer-dischg-fast", chip->dt.esr_timer_dischg_fast,
		NUM_ESR_TIMERS);
	if (rc < 0)
		return rc;

	chip->dt.esr_timer_chg_slow[TIMER_RETRY] = -EINVAL;
	chip->dt.esr_timer_chg_slow[TIMER_MAX] = -EINVAL;
	rc = fg_parse_dt_property_u32_array(node, "qcom,fg-esr-timer-chg-slow",
		chip->dt.esr_timer_chg_slow, NUM_ESR_TIMERS);
	if (rc < 0)
		return rc;

	chip->dt.esr_timer_dischg_slow[TIMER_RETRY] = -EINVAL;
	chip->dt.esr_timer_dischg_slow[TIMER_MAX] = -EINVAL;
	rc = fg_parse_dt_property_u32_array(node,
		"qcom,fg-esr-timer-dischg-slow", chip->dt.esr_timer_dischg_slow,
		NUM_ESR_TIMERS);
	if (rc < 0)
		return rc;

	chip->dt.force_load_profile = of_property_read_bool(node,
					"qcom,fg-force-load-profile");

	fg_gen4_parse_cl_params_dt(chip);
	fg_gen4_parse_batt_temp_dt(chip);

	chip->dt.hold_soc_while_full = of_property_read_bool(node,
					"qcom,hold-soc-while-full");

	chip->dt.linearize_soc = of_property_read_bool(node,
					"qcom,linearize-soc");

	chip->dt.soc_scale_mode = of_property_read_bool(node,
						"qcom,soc-scale-mode-en");
	if (chip->dt.soc_scale_mode) {
		chip->dt.vbatt_scale_thr_mv = DEFAULT_SCALE_VBATT_THR_MV;
		of_property_read_u32(node, "qcom,soc-scale-vbatt-mv",
					&chip->dt.vbatt_scale_thr_mv);
		chip->dt.scale_timer_ms = DEFAULT_SCALE_ALARM_TIMER_MS;
		of_property_read_u32(node, "qcom,soc-scale-time-ms",
					&chip->dt.scale_timer_ms);
	}

	chip->dt.force_calib_level = -EINVAL;
	of_property_read_u32(node, "qcom,force-calib-level",
					&chip->dt.force_calib_level);

	rc = fg_parse_ki_coefficients(fg);
	if (rc < 0)
		pr_err("Error in parsing Ki coefficients, rc=%d\n", rc);

	rc = of_property_read_u32(node, "qcom,fg-rconn-uohms", &temp);
	if (!rc)
		chip->dt.rconn_uohms = temp;

	rc = fg_parse_slope_limit_coefficients(fg);
	if (rc < 0)
		pr_err("Error in parsing slope limit coeffs, rc=%d\n", rc);

	chip->dt.esr_pulse_thresh_ma = DEFAULT_ESR_PULSE_THRESH_MA;
	rc = of_property_read_u32(node, "qcom,fg-esr-pulse-thresh-ma", &temp);
	if (!rc) {
		if (temp > 0 && temp < 1000)
			chip->dt.esr_pulse_thresh_ma = temp;
	}

	chip->dt.esr_meas_curr_ma = DEFAULT_ESR_MEAS_CURR_MA;
	rc = of_property_read_u32(node, "qcom,fg-esr-meas-curr-ma", &temp);
	if (!rc) {
		/* ESR measurement current range is 60-240 mA */
		if (temp >= 60 || temp <= 240)
			chip->dt.esr_meas_curr_ma = temp;
	}

	rc = fg_parse_esr_cal_params(fg);
	if (rc < 0)
		return rc;

	chip->dt.rapid_soc_dec_en = of_property_read_bool(node,
					"qcom,rapid-soc-dec-en");

	chip->dt.five_pin_battery = of_property_read_bool(node,
					"qcom,five-pin-battery");
	chip->dt.multi_profile_load = of_property_read_bool(node,
					"qcom,multi-profile-load");
	chip->dt.soc_hi_res = of_property_read_bool(node, "qcom,soc-hi-res");

	chip->dt.sys_min_volt_mv = DEFAULT_SYS_MIN_VOLT_MV;
	of_property_read_u32(node, "qcom,fg-sys-min-voltage",
				&chip->dt.sys_min_volt_mv);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		fg->temp_corr_en = of_property_read_bool(node,
							"somc,temp-corr-en");
		if (fg->temp_corr_en) {
			rc = of_property_read_u32(node, "somc,temp-corr-coef-a",
									&temp);
			if (rc < 0)
				fg->temp_corr_coef_a = DEFAULT_TEMP_CORR_COEF_A;
			else
				fg->temp_corr_coef_a = temp;

			rc = of_property_read_u32(node,
						"somc,temp-corr-avg-sec",
						&temp);
			if (rc < 0)
				fg->temp_corr_avg_sec =
						DEFAULT_TEMP_CORR_AGV_SEC;
			else
				fg->temp_corr_avg_sec = temp;
		}
#endif

	chip->dt.batt_id_pullup_kohms = DEFAULT_BATT_ID_PULLUP_KOHMS;
	of_property_read_u32(node, "qcom,batt-id-pullup-kohms",
				&chip->dt.batt_id_pullup_kohms);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg->use_real_temp = of_property_read_bool(node,
					"somc,jeita-step-use-real-temp");
	if (fg->use_real_temp) {
		rc = of_property_read_u32(node,
					  "somc,jeita-aux-temp-correction",
					  &temp);
		if (rc < 0)
			fg->aux_temp_correctton = 0;
		else
			fg->aux_temp_correctton = temp;

		fg->real_temp_use_aux = (rc < 0) ? false : true;

		rc = of_property_read_u32(node,
					  "somc,jeita-batt-temp-correction",
					  &temp);
		if (rc < 0)
			fg->batt_temp_correctton = 0;
		else
			fg->batt_temp_correctton = temp;
	}

	rc = of_property_read_u32(node, "somc,msoc-tune-a", &temp);
	if (!rc && temp >= 0 && temp <= SOC_16BIT)
		fg->msoc_tune_a = temp;
	else
		fg->msoc_tune_a = SOC_16BIT / 2;

	rc = of_property_read_u32(node, "somc,msoc-tune-b", &temp);
	if (!rc && temp >= 0 && temp <= SOC_16BIT && temp > fg->msoc_tune_a)
		fg->msoc_tune_b = temp;
	else
		fg->msoc_tune_b = SOC_16BIT;

	somc_fg_gen4_check_max_batt_aging_level(fg);
#endif
	return 0;
}

static void fg_gen4_cleanup(struct fg_gen4_chip *chip)
{
	struct fg_dev *fg = &chip->fg;

	fg_unregister_interrupts(fg, chip, FG_GEN4_IRQ_MAX);

	cancel_work_sync(&fg->status_change_work);
	if (chip->soc_scale_mode)
		fg_gen4_exit_soc_scale(chip);

	cancel_delayed_work_sync(&fg->profile_load_work);
	cancel_delayed_work_sync(&fg->sram_dump_work);
	cancel_work_sync(&chip->pl_current_en_work);

	power_supply_unreg_notifier(&fg->nb);
	debugfs_remove_recursive(fg->dfs_root);
	sysfs_remove_groups(&fg->dev->kobj, fg_groups);

	if (fg->awake_votable)
		destroy_votable(fg->awake_votable);

	if (fg->delta_bsoc_irq_en_votable)
		destroy_votable(fg->delta_bsoc_irq_en_votable);

	if (chip->delta_esr_irq_en_votable)
		destroy_votable(chip->delta_esr_irq_en_votable);

	if (chip->parallel_current_en_votable)
		destroy_votable(chip->parallel_current_en_votable);

	if (chip->mem_attn_irq_en_votable)
		destroy_votable(chip->mem_attn_irq_en_votable);

	dev_set_drvdata(fg->dev, NULL);
}

static void fg_gen4_post_init(struct fg_gen4_chip *chip)
{
	int i;
	struct fg_dev *fg = &chip->fg;

	if (!is_debug_batt_id(fg))
		return;

	/* Disable all wakeable IRQs for a debug battery */
	vote(fg->delta_bsoc_irq_en_votable, DEBUG_BOARD_VOTER, false, 0);
	vote(chip->delta_esr_irq_en_votable, DEBUG_BOARD_VOTER, false, 0);
	vote(chip->mem_attn_irq_en_votable, DEBUG_BOARD_VOTER, false, 0);

	for (i = 0; i < FG_GEN4_IRQ_MAX; i++) {
		if (fg->irqs[i].irq && fg->irqs[i].wakeable) {
			if (i == BSOC_DELTA_IRQ || i == ESR_DELTA_IRQ ||
					i == MEM_ATTN_IRQ) {
				continue;
			} else {
				disable_irq_wake(fg->irqs[i].irq);
				disable_irq_nosync(fg->irqs[i].irq);
			}
		}
	}

	fg_dbg(fg, FG_STATUS, "Disabled wakeable irqs for debug board\n");
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void somc_fg_gen4_restore_batt_aging_level_from_sram(struct fg_dev *fg)
{
	int rc;
	u8 val;

	fg->batt_aging_level = 0;
	if (fg->max_batt_aging_level == BATT_AGING_LEVEL_NONE) {
		fg_dbg(fg, FG_SOMC, "batt aging level is not restored\n");
		return;
	}

	rc = fg_sram_read(fg, SOMC_AGING_LEVEL_WORD, SOMC_AGING_LEVEL_OFFSET,
						&val, 1, FG_IMA_DEFAULT);
	if (rc < 0) {
		pr_err("failed to read batt aging level rc=%d\n", rc);
	} else if (val > fg->max_batt_aging_level) {
		pr_err("saved batt aging level is abnormal val=%d\n", val);
	} else {
		fg->batt_aging_level = val;
	}
	fg_dbg(fg, FG_SOMC, "batt aging level = %d\n", fg->batt_aging_level);

	fg->requested_batt_aging_level = NO_REQUESTED_LEVEL;
}

/*****************************
 * somc sysfs implementation *
 *****************************/
enum fg_somc_sysfs {
	ATTR_REG_BATT_SOC_INT_RT_STS = 0,
	ATTR_REG_BATT_INFO_INT_RT_STS,
	ATTR_SRAM_REG_ESR_MDL,
	ATTR_SRAM_REG_ESR_ACT,
	ATTR_SRAM_REG_RSLOW,
	ATTR_SRAM_REG_BATTERY_SOC,
	ATTR_SRAM_REG_CC_SOC,
	ATTR_SRAM_REG_MONOTONIC_SOC,
	ATTR_SRAM_REG_FULL_SOC,
	ATTR_SRAM_REG_CC_SOC_SW,
	ATTR_SRAM_REG_CUTOFF_SOC,
	ATTR_SRAM_REG_SYSTEM_SOC,
	ATTR_SRAM_REG_OCV_CALCULATED,
	ATTR_SRAM_REG_LEARNED_CAPACITY,
	ATTR_SRAM_REG_NOMINAL_CAPACITY,
	ATTR_SRAM_REG_SLOPE_LIMIT,
	ATTR_VCELL_MAX,
	ATTR_CL_ACTIVE,
	ATTR_CL_BSOC_DROP,
	ATTR_CL_CCSOC_DROP,
	ATTR_CL_HOLD_TIME,
	ATTR_CL_TOTAL_TIME,
	ATTR_MATIC_SOC,
	ATTR_MSOC_TUNE_A,
	ATTR_MSOC_TUNE_B,
	ATTR_PROFILE_LOAD_STATUS,
	ATTR_SOC_RESTART_COUNTER,
	ATTR_IAVG,
	ATTR_CELL_TEMP,
	ATTR_CELL_TEMP_COEF_A,
	ATTR_CELL_TEMP_AVG_SEC,
	ATTR_FV_RB_EN,
	ATTR_FV_RB_SUSPEND,
	ATTR_FV_RB_SLOPE_LIMIT_SUSPENDED,
	ATTR_FV_RB_FULL_STEP_UP_SOC,
	ATTR_PSY_CHG_COUNTER,
};

static ssize_t fg_somc_param_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fg_somc_param_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count);

static struct device_attribute fg_somc_attrs[] = {
	__ATTR(somc_reg_batt_soc_int_rt_sts, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_reg_batt_info_int_rt_sts, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_esr_mdl, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_esr_act, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_rslow, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_batt_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_cc_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_monotonic_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_full_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_cc_soc_software, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_cutoff_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_system_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_ocv_calculated, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_learned_capacity, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_nominal_capacity, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_sram_reg_slope_limit, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_vcell_max, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cl_active, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cl_bsoc_drop, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cl_ccsoc_drop, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cl_hold_time, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cl_total_time, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_maint_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_msoc_tune_a, 0644, fg_somc_param_show, fg_somc_param_store),
	__ATTR(somc_msoc_tune_b, 0644, fg_somc_param_show, fg_somc_param_store),
	__ATTR(somc_profile_load_status, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_soc_restart_counter, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_iavg, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cell_temp, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_cell_temp_coef_a, 0644,
				fg_somc_param_show, fg_somc_param_store),
	__ATTR(somc_cell_temp_avg_sec, 0644,
				fg_somc_param_show, fg_somc_param_store),
	__ATTR(somc_fv_rb_en, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_fv_rb_suspend, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_fv_rb_slope_limit_suspended, 0444,
				fg_somc_param_show, NULL),
	__ATTR(somc_fv_rb_full_step_up_soc, 0444, fg_somc_param_show, NULL),
	__ATTR(somc_psy_chg_counter, 0444, fg_somc_param_show, NULL),
};

#define DECIMAL_CELL		100
#define DECIMAL_MAG		10000LL

static ssize_t fg_somc_get_sram_soc_str(struct fg_gen4_chip *chip,
				enum fg_sram_param_id id, char *buf, int size)
{
	int rc = 0;
	int value, capacity, high_cap, low_cap;
	struct fg_dev *fg = &chip->fg;

	rc = fg_get_sram_prop(fg, id, &value);
	if (rc < 0) {
		pr_err("Error reading address rc=%d\n", rc);
		return 0;
	}

	switch (id) {
	case FG_SRAM_BATT_SOC:
		capacity = (int)((u64)((u32)value) * DECIMAL_MAG /
							BATT_SOC_32BIT);
		break;
	case FG_SRAM_CC_SOC:
	case FG_SRAM_CC_SOC_SW:
		capacity = (int)((s64)value * DECIMAL_MAG / CC_SOC_30BIT);
		break;
	case FG_SRAM_MONOTONIC_SOC:
	case FG_SRAM_FULL_SOC:
	case FG_SRAM_CUTOFF_SOC:
	case FG_SRAM_SYSTEM_SOC:
		capacity = (int)((u64)((u32)value) * DECIMAL_MAG / SOC_16BIT);
		break;
	default:
		break;
	}

	high_cap = abs(capacity / DECIMAL_CELL);
	low_cap = abs(capacity % DECIMAL_CELL);
	if (capacity >= 0)
		size = scnprintf(buf, size, "%d.%02d\n", high_cap, low_cap);
	else
		size = scnprintf(buf, size, "-%d.%02d\n", high_cap, low_cap);
	return size;
}

#define PROFILE_LOAD_STATUS_STR_MISSING		"PROFILE_MISSING"
#define PROFILE_LOAD_STATUS_STR_LOADED		"PROFILE_LOADED"
#define PROFILE_LOAD_STATUS_STR_SKIPPED		"PROFILE_SKIPPED"
#define PROFILE_LOAD_STATUS_STR_NOT_LOADED	"PROFILE_NOT_LOADED"
#define PROFILE_LOAD_STATUS_STR_ERROR		"N/A"
static const char *fg_somc_get_profile_load_status(struct fg_dev *fg)
{
	char *status;

	switch (fg->profile_load_status) {
	case PROFILE_MISSING:
		status = PROFILE_LOAD_STATUS_STR_MISSING;
		break;
	case PROFILE_LOADED:
		status = PROFILE_LOAD_STATUS_STR_LOADED;
		break;
	case PROFILE_SKIPPED:
		status = PROFILE_LOAD_STATUS_STR_SKIPPED;
		break;
	case PROFILE_NOT_LOADED:
		status = PROFILE_LOAD_STATUS_STR_NOT_LOADED;
		break;
	default:
		status = PROFILE_LOAD_STATUS_STR_ERROR;
		break;
	};

	return status;
}

static ssize_t fg_somc_param_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(dev);
	struct fg_dev *fg = &chip->fg;
	ssize_t size = 0;
	const ptrdiff_t off = attr - fg_somc_attrs;
	int rc = 0;
	u8 reg[2];
	int64_t nom_cap_uah;
	int act_cap_mah;
	int ocv_uv;
	int val;

	switch (off) {
	case ATTR_REG_BATT_SOC_INT_RT_STS:
		rc = fg_read(fg, BATT_SOC_INT_RT_STS(fg), reg, 1);
		if (rc < 0)
			pr_err("failed to read addr=0x%04x, rc=%d\n",
						BATT_SOC_INT_RT_STS(fg), rc);
		size = scnprintf(buf, PAGE_SIZE, "%02X\n", reg[0]);
		break;
	case ATTR_REG_BATT_INFO_INT_RT_STS:
		rc = fg_read(fg, BATT_INFO_INT_RT_STS(fg), reg, 1);
		if (rc < 0)
			pr_err("failed to read addr=0x%04x, rc=%d\n",
						BATT_INFO_INT_RT_STS(fg), rc);
		size = scnprintf(buf, PAGE_SIZE, "%02X\n", reg[0]);
		break;
	case ATTR_SRAM_REG_ESR_MDL:
		rc = fg_get_sram_prop(fg, FG_SRAM_ESR_MDL, &val);
		if (rc < 0)
			pr_err("Error reading address rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case ATTR_SRAM_REG_ESR_ACT:
		rc = fg_get_sram_prop(fg, FG_SRAM_ESR_ACT, &val);
		if (rc < 0)
			pr_err("Error reading address rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case ATTR_SRAM_REG_RSLOW:
		rc = fg_get_sram_prop(fg, FG_SRAM_RSLOW, &val);
		if (rc < 0)
			pr_err("Error reading address rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case ATTR_SRAM_REG_BATTERY_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_BATT_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_CC_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_CC_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_MONOTONIC_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_MONOTONIC_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_FULL_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_FULL_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_CC_SOC_SW:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_CC_SOC_SW,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_CUTOFF_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_CUTOFF_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_SYSTEM_SOC:
		size = fg_somc_get_sram_soc_str(chip, FG_SRAM_SYSTEM_SOC,
							buf, PAGE_SIZE);
		break;
	case ATTR_SRAM_REG_OCV_CALCULATED:
		rc = fg_get_sram_prop(fg, FG_SRAM_OCV, &ocv_uv);
		if (rc < 0)
			pr_err("failed to get battery ocv, rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", ocv_uv);
		break;
	case ATTR_SRAM_REG_LEARNED_CAPACITY:
		rc = fg_get_sram_prop(fg, FG_SRAM_ACT_BATT_CAP, &act_cap_mah);
		if (rc < 0)
			pr_err("Error in getting ACT_BATT_CAP, rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", act_cap_mah);
		break;
	case ATTR_SRAM_REG_NOMINAL_CAPACITY:
		rc = fg_sram_read(fg, NOM_CAP_WORD, NOM_CAP_OFFSET, reg, 2,
								FG_IMA_DEFAULT);
		if (rc < 0) {
			pr_err("Error in reading %04x[%d] rc=%d\n",
					NOM_CAP_WORD, NOM_CAP_OFFSET, rc);
		} else {
			nom_cap_uah = (int)(reg[0] | reg[1] << 8) * 1000;
			size = scnprintf(buf, PAGE_SIZE, "%d\n", nom_cap_uah);
		}
		break;
	case ATTR_SRAM_REG_SLOPE_LIMIT:
		rc = fg_sram_read(fg, SLOPE_LIMIT_WORD, SLOPE_LIMIT_OFFSET,
							reg, 1, FG_IMA_DEFAULT);
		if (rc < 0)
			pr_err("Error reading address rc=%d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", reg[0]);
		break;
	case ATTR_VCELL_MAX:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->vcell_max_mv);
		break;
	case ATTR_CL_ACTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", (int)chip->cl->active);
		break;
	case ATTR_CL_BSOC_DROP:
		val = chip->cl->batt_soc_cp_drop;
		if (val >= 0) {
			size = scnprintf(buf, PAGE_SIZE, "%d.%02d\n",
							val / 100, val % 100);
		} else {
			size = scnprintf(buf, PAGE_SIZE, "-%d.%02d\n",
					abs(val / 100), abs(val % 100));
		}
		break;
	case ATTR_CL_CCSOC_DROP:
		val = (int)(div64_s64((int64_t)chip->cl->cc_soc_drop * 10000,
							CC_SOC_30BIT));
		if (val >= 0) {
			size = scnprintf(buf, PAGE_SIZE, "%d.%02d\n",
							val / 100, val % 100);
		} else {
			size = scnprintf(buf, PAGE_SIZE, "-%d.%02d\n",
					abs(val / 100), abs(val % 100));
		}
		break;
	case ATTR_CL_HOLD_TIME:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					(int)(chip->cl->hold_time / 1000));
		break;
	case ATTR_CL_TOTAL_TIME:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					(int)(chip->cl->total_time / 1000));
		break;
	case ATTR_MATIC_SOC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->maint_soc);
		break;
	case ATTR_MSOC_TUNE_A:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->msoc_tune_a);
		break;
	case ATTR_MSOC_TUNE_B:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->msoc_tune_b);
		break;
	case ATTR_PROFILE_LOAD_STATUS:
		size = scnprintf(buf, PAGE_SIZE, "%s\n",
					fg_somc_get_profile_load_status(fg));
		break;
	case ATTR_SOC_RESTART_COUNTER:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						fg->soc_restart_counter);
		break;
	case ATTR_IAVG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->iavg);
		break;
	case ATTR_CELL_TEMP:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->cell_temp);
		break;
	case ATTR_CELL_TEMP_COEF_A:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->temp_corr_coef_a);
		break;
	case ATTR_CELL_TEMP_AVG_SEC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->temp_corr_avg_sec);
		break;
	case ATTR_FV_RB_EN:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->fv_rb_en);
		break;
	case ATTR_FV_RB_SUSPEND:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", fg->fv_rb_suspened);
		break;
	case ATTR_FV_RB_SLOPE_LIMIT_SUSPENDED:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						fg->slope_limit_suspended);
		break;
	case ATTR_FV_RB_FULL_STEP_UP_SOC:
		val = 10000 * fg->fv_rb_full_step_up_soc_raw / 65535;
		if (val >= 0)
			size = scnprintf(buf, PAGE_SIZE, "%d.%02d\n",
							val / 100, val % 100);
		else
			size = scnprintf(buf, PAGE_SIZE, "0\n");
		break;
	case ATTR_PSY_CHG_COUNTER:
		size = scnprintf(buf, PAGE_SIZE, "%lu\n", fg->psy_chg_counter);
		break;
	default:
		size = 0;
		break;
	}
	return size;
}

static ssize_t fg_somc_param_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(dev);
	struct fg_dev *fg = &chip->fg;
	const ptrdiff_t off = attr - fg_somc_attrs;
	int ret;

	switch (off) {
	case ATTR_MSOC_TUNE_A:
		ret = kstrtoint(buf, 10, &fg->msoc_tune_a);
		if (ret < 0)
			return ret;
		break;
	case ATTR_MSOC_TUNE_B:
		ret = kstrtoint(buf, 10, &fg->msoc_tune_b);
		if (ret < 0)
			return ret;
		break;
	case ATTR_CELL_TEMP_COEF_A:
		ret = kstrtoint(buf, 10, &fg->temp_corr_coef_a);
		if (ret < 0)
			return ret;
		break;
	case ATTR_CELL_TEMP_AVG_SEC:
		ret = kstrtoint(buf, 10, &fg->temp_corr_avg_sec);
		if (ret < 0)
			return ret;
		break;
	default:
		break;
	}
	return count;
}

static int fg_somc_create_sysfs_entries(struct device *dev)
{
	int i;
	int rc = 0;

	for (i = 0; i < ARRAY_SIZE(fg_somc_attrs); i++) {
		rc = device_create_file(dev, &fg_somc_attrs[i]);
		if (rc < 0) {
			dev_err(dev, "device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;
revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &fg_somc_attrs[i]);
	return rc;
}

static void fg_somc_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fg_somc_attrs); i++)
		device_remove_file(dev, &fg_somc_attrs[i]);
}

#endif
static int fg_gen4_probe(struct platform_device *pdev)
{
	struct fg_gen4_chip *chip;
	struct fg_dev *fg;
	struct power_supply_config fg_psy_cfg = {};
	int rc, msoc, volt_uv, batt_temp;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	fg = &chip->fg;
	fg->dev = &pdev->dev;
	fg->debug_mask = &fg_gen4_debug_mask;
	fg->irqs = fg_irqs;
	fg->charge_status = -EINVAL;
	fg->online_status = -EINVAL;
	fg->batt_id_ohms = -EINVAL;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg->real_temp_debug = -EINVAL;
#endif
	chip->ki_coeff_full_soc[0] = -EINVAL;
	chip->ki_coeff_full_soc[1] = -EINVAL;
	chip->esr_soh_cycle_count = -EINVAL;
	chip->calib_level = -EINVAL;
	chip->soh = -EINVAL;
	fg->regmap = dev_get_regmap(fg->dev->parent, NULL);
	if (!fg->regmap) {
		dev_err(fg->dev, "Parent regmap is unavailable\n");
		return -ENXIO;
	}

	mutex_init(&fg->bus_lock);
	mutex_init(&fg->sram_rw_lock);
	mutex_init(&fg->charge_full_lock);
	mutex_init(&chip->soc_scale_lock);
	mutex_init(&chip->esr_calib_lock);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	mutex_init(&fg->step_lock);
	mutex_init(&fg->temp_corr_lock);
#endif
	init_completion(&fg->soc_update);
	init_completion(&fg->soc_ready);
	init_completion(&chip->mem_attn);
	INIT_WORK(&fg->status_change_work, status_change_work);
	INIT_WORK(&chip->esr_calib_work, esr_calib_work);
	INIT_WORK(&chip->soc_scale_work, soc_scale_work);
	INIT_DELAYED_WORK(&fg->profile_load_work, profile_load_work);
	INIT_DELAYED_WORK(&fg->sram_dump_work, sram_dump_work);
	INIT_DELAYED_WORK(&chip->pl_enable_work, pl_enable_work);
	INIT_WORK(&chip->pl_current_en_work, pl_current_en_work);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	INIT_DELAYED_WORK(&fg->somc_jeita_step_charge_work,
					fg_gen4_somc_jeita_step_charge_work);

	fg->step_ws = wakeup_source_register(fg->dev, "somc_jeita_step");
	if (!fg->step_ws)
		return -EINVAL;

	INIT_DELAYED_WORK(&fg->somc_temp_corr_work,
					fg_gen4_somc_temp_corr_work);
	INIT_DELAYED_WORK(&fg->somc_charge_full_releasing_work,
				fg_gen4_somc_charge_full_releasing_work);
	INIT_WORK(&fg->somc_fv_rb_soc_work, fg_gen4_somc_fv_rb_soc_work);
	INIT_WORK(&fg->psy_chg_work, fg_gen4_somc_psy_chg_work);
	if (alarmtimer_get_rtcdev()) {
		alarm_init(&fg->psy_chg_alarm_timer,
			ALARM_BOOTTIME, fg_gen4_somc_psy_chg_alarm_timer);
	} else {
		pr_err("Error in alarm_init, rc:%d\n", rc);
		return rc;
	}
#endif

	fg->awake_votable = create_votable("FG_WS", VOTE_SET_ANY,
					fg_awake_cb, fg);
	if (IS_ERR(fg->awake_votable)) {
		rc = PTR_ERR(fg->awake_votable);
		fg->awake_votable = NULL;
		goto exit;
	}

	fg->delta_bsoc_irq_en_votable = create_votable("FG_DELTA_BSOC_IRQ",
						VOTE_SET_ANY,
						fg_delta_bsoc_irq_en_cb, fg);
	if (IS_ERR(fg->delta_bsoc_irq_en_votable)) {
		rc = PTR_ERR(fg->delta_bsoc_irq_en_votable);
		fg->delta_bsoc_irq_en_votable = NULL;
		goto exit;
	}

	chip->delta_esr_irq_en_votable = create_votable("FG_DELTA_ESR_IRQ",
						VOTE_SET_ANY,
						fg_gen4_delta_esr_irq_en_cb,
						chip);
	if (IS_ERR(chip->delta_esr_irq_en_votable)) {
		rc = PTR_ERR(chip->delta_esr_irq_en_votable);
		chip->delta_esr_irq_en_votable = NULL;
		goto exit;
	}

	chip->mem_attn_irq_en_votable = create_votable("FG_MEM_ATTN_IRQ",
						VOTE_SET_ANY,
						fg_gen4_mem_attn_irq_en_cb, fg);
	if (IS_ERR(chip->mem_attn_irq_en_votable)) {
		rc = PTR_ERR(chip->mem_attn_irq_en_votable);
		chip->mem_attn_irq_en_votable = NULL;
		goto exit;
	}

	chip->parallel_current_en_votable = create_votable("FG_SMB_MEAS_EN",
						VOTE_SET_ANY,
						fg_parallel_current_en_cb, fg);
	if (IS_ERR(chip->parallel_current_en_votable)) {
		rc = PTR_ERR(chip->parallel_current_en_votable);
		chip->parallel_current_en_votable = NULL;
		goto exit;
	}

	rc = fg_alg_init(chip);
	if (rc < 0) {
		dev_err(fg->dev, "Error in alg_init, rc:%d\n",
			rc);
		goto exit;
	}

	rc = fg_gen4_parse_dt(chip);
	if (rc < 0) {
		dev_err(fg->dev, "Error in reading DT parameters, rc:%d\n",
			rc);
		goto exit;
	}

	if (chip->esr_fast_calib) {
		if (alarmtimer_get_rtcdev()) {
			alarm_init(&chip->esr_fast_cal_timer, ALARM_BOOTTIME,
				fg_esr_fast_cal_timer);
		} else {
			dev_err(fg->dev, "Failed to initialize esr_fast_cal timer\n");
			rc = -EPROBE_DEFER;
			goto exit;
		}
	}

	if (chip->dt.soc_scale_mode) {
		if (alarmtimer_get_rtcdev()) {
			alarm_init(&chip->soc_scale_alarm_timer,
				ALARM_BOOTTIME, fg_soc_scale_timer);
		} else {
			dev_err(fg->dev, "Failed to initialize SOC scale timer\n");
			rc = -EPROBE_DEFER;
			goto exit;
		}
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (alarmtimer_get_rtcdev()) {
		alarm_init(&fg->somc_fv_rb_alarm_timer,
			ALARM_BOOTTIME, fg_somc_fv_rb_alarm_timer);
	} else {
		dev_err(fg->dev, "Failed to initialize FV_RB SOC timer\n");
		rc = -EPROBE_DEFER;
		goto exit;
	}

#endif
	rc = fg_memif_init(fg);
	if (rc < 0) {
		dev_err(fg->dev, "Error in initializing FG_MEMIF, rc:%d\n",
			rc);
		goto exit;
	}

	platform_set_drvdata(pdev, chip);
	rc = fg_gen4_hw_init(chip);
	if (rc < 0) {
		dev_err(fg->dev, "Error in initializing FG hardware, rc:%d\n",
			rc);
		goto exit;
	}

	/* Register the power supply */
	fg_psy_cfg.drv_data = fg;
	fg_psy_cfg.of_node = fg->dev->of_node;
	fg->fg_psy = devm_power_supply_register(fg->dev, &fg_psy_desc,
			&fg_psy_cfg);
	if (IS_ERR(fg->fg_psy)) {
		pr_err("failed to register fg_psy rc = %ld\n",
				PTR_ERR(fg->fg_psy));
		goto exit;
	}

	fg->nb.notifier_call = fg_notifier_cb;
	rc = power_supply_reg_notifier(&fg->nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		goto exit;
	}

	rc = fg_register_interrupts(&chip->fg, FG_GEN4_IRQ_MAX);
	if (rc < 0) {
		dev_err(fg->dev, "Error in registering interrupts, rc:%d\n",
			rc);
		goto exit;
	}

	if (fg->irqs[MEM_ATTN_IRQ].irq)
		irq_set_status_flags(fg->irqs[MEM_ATTN_IRQ].irq,
					IRQ_DISABLE_UNLAZY);

	/* Keep SOC_UPDATE irq disabled until we require it */
	if (fg->irqs[SOC_UPDATE_IRQ].irq)
		disable_irq_nosync(fg->irqs[SOC_UPDATE_IRQ].irq);

	/* Keep BSOC_DELTA_IRQ disabled until we require it */
	vote(fg->delta_bsoc_irq_en_votable, DELTA_BSOC_IRQ_VOTER, false, 0);

	/* Keep MEM_ATTN_IRQ disabled until we require it */
	vote(chip->mem_attn_irq_en_votable, MEM_ATTN_IRQ_VOTER, false, 0);

	fg_debugfs_create(fg);

	rc = sysfs_create_groups(&fg->dev->kobj, fg_groups);
	if (rc < 0) {
		dev_err(fg->dev, "Error in creating sysfs entries, rc:%d\n",
			rc);
		goto exit;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = fg_somc_create_sysfs_entries(fg->dev);
	if (rc < 0) {
		dev_err(fg->dev,
			"Error in creating fg_somc_sysfs entries, rc:%d\n",
			rc);
		goto exit;
	}

	alarm_start_relative(&fg->psy_chg_alarm_timer,
				ms_to_ktime(PSY_CHG_ALARM_FIRST_TIME_MS));
#endif
	rc = fg_get_battery_voltage(fg, &volt_uv);
	if (!rc)
		rc = fg_get_msoc(fg, &msoc);

	if (!rc)
		rc = fg_gen4_get_battery_temp(fg, &batt_temp);

	if (!rc)
		rc = fg_gen4_get_batt_id(chip);

	if (!rc) {
		fg->last_batt_temp = batt_temp;
		pr_info("battery SOC:%d voltage: %duV temp: %d id: %d ohms\n",
			msoc, volt_uv, batt_temp, fg->batt_id_ohms);
	}

	fg->tz_dev = thermal_zone_of_sensor_register(fg->dev, 0, fg,
							&fg_gen4_tz_ops);
	if (IS_ERR_OR_NULL(fg->tz_dev)) {
		rc = PTR_ERR(fg->tz_dev);
		fg->tz_dev = NULL;
		dev_dbg(fg->dev, "Couldn't register with thermal framework rc:%d\n",
			rc);
	}

	device_init_wakeup(fg->dev, true);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_fg_gen4_restore_batt_aging_level_from_sram(fg);
#endif
	if (!fg->battery_missing)
		schedule_delayed_work(&fg->profile_load_work, 0);

	fg_gen4_post_init(chip);

	pr_debug("FG GEN4 driver probed successfully\n");
	return 0;
exit:
	fg_gen4_cleanup(chip);
	return rc;
}

static int fg_gen4_remove(struct platform_device *pdev)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(&pdev->dev);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_somc_remove_sysfs_entries(chip->fg.dev);
#endif
	fg_gen4_cleanup(chip);
	return 0;
}

static void fg_gen4_shutdown(struct platform_device *pdev)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(&pdev->dev);
	struct fg_dev *fg = &chip->fg;
	int rc, bsoc;

	fg_unregister_interrupts(fg, chip, FG_GEN4_IRQ_MAX);

	if (chip->soc_scale_mode)
		fg_gen4_exit_soc_scale(chip);

	if (chip->rapid_soc_dec_en) {
		rc = fg_gen4_rapid_soc_config(chip, false);
		if (rc < 0)
			pr_err("Error in reverting rapid SOC decrease config rc:%d\n",
				rc);
	}

	rc = fg_get_sram_prop(fg, FG_SRAM_BATT_SOC, &bsoc);
	if (rc < 0) {
		pr_err("Error in getting BATT_SOC, rc=%d\n", rc);
		return;
	}

	if (fg->charge_full) {
		/* We need 2 most significant bytes here */
		bsoc = (u32)bsoc >> 16;

		rc = fg_gen4_configure_full_soc(fg, bsoc);
		if (rc < 0) {
			pr_err("Error in configuring full_soc, rc=%d\n", rc);
			return;
		}
	}

	/*
	 * Charging status doesn't matter when the device shuts down and we
	 * have to treat this as charge done. Hence pass charge_done as true.
	 */
	cycle_count_update(chip->counter, (u32)bsoc >> 24,
		POWER_SUPPLY_STATUS_NOT_CHARGING, true, is_input_present(fg));
}

static int fg_gen4_suspend(struct device *dev)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(dev);
	struct fg_dev *fg = &chip->fg;

	cancel_delayed_work_sync(&chip->ttf->ttf_work);
	if (fg_sram_dump)
		cancel_delayed_work_sync(&fg->sram_dump_work);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cancel_delayed_work_sync(&fg->somc_temp_corr_work);
	fg_gen4_somc_set_temp_corr_sus_time(fg,
					ktime_to_ms(ktime_get_boottime()));
#endif
	return 0;
}

static int fg_gen4_resume(struct device *dev)
{
	struct fg_gen4_chip *chip = dev_get_drvdata(dev);
	struct fg_dev *fg = &chip->fg;

	schedule_delayed_work(&chip->ttf->ttf_work, 0);
	if (fg_sram_dump)
		schedule_delayed_work(&fg->sram_dump_work,
				msecs_to_jiffies(fg_sram_dump_period_ms));
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	fg_gen4_somc_temp_corr_on_resume(fg);
	fg_gen4_somc_set_temp_corr_sus_time(fg, 0);
#endif
	return 0;
}

static const struct dev_pm_ops fg_gen4_pm_ops = {
	.suspend	= fg_gen4_suspend,
	.resume		= fg_gen4_resume,
};

static const struct of_device_id fg_gen4_match_table[] = {
	{.compatible = FG_GEN4_DEV_NAME},
	{},
};

static struct platform_driver fg_gen4_driver = {
	.driver = {
		.name = FG_GEN4_DEV_NAME,
		.of_match_table = fg_gen4_match_table,
		.pm		= &fg_gen4_pm_ops,
	},
	.probe		= fg_gen4_probe,
	.remove		= fg_gen4_remove,
	.shutdown	= fg_gen4_shutdown,
};

module_platform_driver(fg_gen4_driver);

MODULE_DESCRIPTION("QPNP Fuel gauge GEN4 driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" FG_GEN4_DEV_NAME);
