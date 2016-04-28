/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/ktime.h>
#include <linux/wakelock.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/power/bcmpmu-fg.h>
#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#else
#include <linux/alarmtimer.h>
#endif
#include <mach/kona_timer.h>
#include <linux/fb.h>

#ifndef CONFIG_WD_TAPPER
/* 300 seconds are based on wd_tapper count which is configured in dts file */
#define ALARM_DEFAULT_TIMEOUT		300
#endif

#define FG_CAPACITY_SAVE_REG		(PMU_REG_FGGNRL1)
#define FG_CAP_FULL_CHARGE_REG		(PMU_REG_FGGNRL2)
#define FG_CAP_FULL_CHARGE_REG1		(PMU_REG_FGGNRL3)
#define FG_LOW_CAL_STATUS_REG		(PMU_REG_FGGNRL3)
#define FG_FC_SAVE_CAP_MASK	0x80
#define FG_SAVE_CAP_DELTA_NEGTV 0x40
#define FG_SAVE_CAP_DELTA_MASK 0x3F
#define FG_FC_CAP_MASK	0x8000
#define FG_CAP_MASK 0xFF
#define FG_FC_MAX_SAMPLES	2
#define FG_FC_MAX_DEV	7
#define FG_QF_DELTA	5

#define PMU_FG_CURR_SMPL_SIGN_BIT_MASK		0x8000
#define PMU_FG_CURR_SMPL_SIGN_BIT_SHIFT		15

#define FG_CAP_DELTA_THRLD		10
#define FG_FLAT_CAP_DELTA_THRLD		20
#define ADC_VBAT_AVG_SAMPLES		8
#define ADC_NTC_AVG_SAMPLES		8
#define FG_INIT_CAPACITY_AVG_SAMPLES	8
#define FG_CAL_CAPACITY_AVG_SAMPLES	8
#define FG_INIT_CAPACITY_SAMPLE_DELAY	150
#define FG_INIT_CAP_CHARGING_DELAY	500

#define ADC_READ_TRIES			10
#define ADC_RETRY_DELAY		100 /* 100ms */
#define AVG_SAMPLES			5
#define NTC_TEMP_OFFSET			100
#define FAKE_IBAT_INTIAL_AVG		0xFFF
#define FG_MIN_DIS_VOLT_FOR_OCV		3550

#define FG_WORK_POLL_TIME_MS		(5000)
#define CHARG_ALGO_POLL_TIME_MS		(30000)
#define DISCHARGE_ALGO_POLL_TIME_MS	(60000)
#define LOW_BATT_POLL_TIME_MS		(5000)
#define CRIT_BATT_POLL_TIME_MS		(2000)
#define FAKE_BATT_POLL_TIME_MS		(60000)

#define FG_SETTLING_TIME (bcmpmu_fg_sample_rate_to_time\
			(fg->sample_rate) * 5)

#define INTERPOLATE_LINEAR(X, Xa, Ya, Xb, Yb) \
	(Ya + (((Yb - Ya) * (X - Xa))/(Xb - Xa)))

#define CAPACITY_PERCENTAGE_FULL	100
#define CAPACITY_PERCENTAGE_EMPTY	0
#define CAPACITY_ZERO_ALIAS		0xFF
#define CAPACITY_INIT_CAP_FLAT		0x80

/* Low Battery Calibration Macros */
#define FG_LOW_BAT_CAP_DELTA_LIMIT	30

/* EOC error correction calibration Macros*/
#define FG_CAL_EOC_SPLIT_CNT		5
#define FG_CAL_EOC_POINT		80

/**
 * FG scaling factor = 1024LSB = 1000mA
 *			1LSB = 0.977
 */
#define FG_CURR_SCALING_FACTOR		977
#define FG_SLEEP_SAMPLE_RATE		32000
#define FG_SLEEP_CURR_UA		1000
#define FG_EOC_CURRENT			75
#define FG_CURR_SAMPLE_MAX		2000
#define FG_EOC_CNT_THRLD		5
#define FG_EOC_CAP_THRLD		98
#define EOC_CAP_INC_CNT_THRLD		3
#define FG_MAX_ADJ_FACTOR		50
#define MAX_EOC_ADJ_FACTOR		100
#define MIN_EOC_ADJ_FACTOR		-100

#define BATT_LI_ION_MIN_VOLT		2100
#define BATT_LI_ION_MAX_VOLT		4200
#define BATT_VFLOAT_DEFAULT		0xC
#define BATT_MIN_EOC_VOLT		4116
#define CAL_CNT_THRESHOLD		3
#define GUARD_BAND_LOW_THRLD		10
#define CRIT_CUTOFF_CNT_THRD		3
#define LOW_VOLT_CNT_THRD		2
#define FG_DUMMY_BAT_CAP		30

#define ESR_VL_PER_MARGIN		105
#define ESR_VF_PER_MARGIN		95
/**
 * FG should go to synchronous mode (modulator turned off)
 * during system deep sleep condition i.e. PC1, PC2 & PC3
 * = 0, 0, 0
 */
#define FG_OPMOD_CTRL_SETTING		0x1


#define NTC_ROOM_TEMP			250 /* 25C */
/**
 * Helper macros
 */
#define to_bcmpmu_fg_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_fg_data, mem)

#define capacity_delta_in_percent(fg, c1, c2) \
	((((c1 > c2) ? (c1 - c2) : (c2 - c1)) * 100) / \
	  fg->capacity_info.max_design)

#define FG_LOCK(fg)		{ \
	fg->lock_cnt++;\
	mutex_lock(&fg->mutex);\
}

#define FG_UNLOCK(fg)		{ \
	fg->lock_cnt--;\
	mutex_unlock(&fg->mutex);\
}

enum bcmpmu_fg_cal_state {
	CAL_STATE_IDLE,
	CAL_STATE_START,
	CAL_STATE_WAIT,
	CAL_STATE_DONE
};

enum bcmpmu_fg_cal_mode {
	CAL_MODE_NONE,
	CAL_MODE_FORCE,
	CAL_MODE_LOW_BATT,
	CAL_MODE_HI_BATT,
	CAL_MODE_TEMP,
	CAL_MODE_EOC_ADJ,
};

enum bcmpmu_mbc_state {
	MBC_STATE_IDLE,
	MBC_STATE_QC,
	MBC_STATE_FC,
	MBC_STATE_MC,
	MBC_STATE_PAUSE,
	MBC_STATE_THSD,
};

enum bcmpmu_fg_sample_rate {
	SAMPLE_RATE_2HZ = 0,
	SAMPLE_RATE_4HZ,
	SAMPLE_RATE_8HZ,
	SAMPLE_RATE_16HZ,
};

enum bcmpmu_fg_offset_cal_mode {
	FG_HW_CAL_MODE_FORCE, /* force calibration */
	FG_HW_CAL_MODE_1PT,   /* 1 point calibration */
	FG_HW_CAL_MODE_LONGCAL, /* calibrate FG: offset using slow output */
	FG_HW_CAL_MODE_FAST, /* calibrate FG: offset using fast output */
};

enum bcmpmu_fg_opmode {
	FG_OPMODE_SYNC,
	FG_OPMODE_CONTINUOUS,
};

enum bcmpmu_fg_mode {
	FG_MODE_ACTIVE,
	FG_MODE_FREEZE,
};

enum maintenance_chrgr_mode {
	SW_MAINTENANCE_CHARGING,
	HW_MAINTENANCE_CHARGING,
};

enum bcmpmu_batt_charging_state {
	CHARG_STATE_IDLE,
	CHARG_STATE_FC,
	CHARG_STATE_MC,
};

static const char * const charging_state_dbg[] = {
	"chrgr_idle",
	"chrgr_fc",
	"chrgr_mc"
};

enum bcmpmu_batt_discharging_state {
	DISCHARG_STATE_HIGH_BATT,
	DISCHARG_STATE_LOW_BATT,
	DISCHARG_STATE_CRIT_BATT,
	DISCHARG_STATE_CRIT_VOLT,
};

static const char * const discharge_state_dbg[] = {
	"high_batt",
	"low_bat",
	"crit_batt",
	"crit_volt",
};

static enum power_supply_property bcmpmu_fg_props[] = {
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_FULL_BAT,
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
};

struct bcmpmu_fg_status_flags {
	int batt_status;
	int prev_batt_status;
	bool chrgr_connected;
	bool charging_enabled;
	bool batt_present;
	bool init_capacity;
	bool fg_eoc;
	bool reschedule_work;
	bool eoc_chargr_en;
	bool calibration;
	bool low_bat_cal;
	bool low_cal_status;
	bool high_bat_cal;
	bool fully_charged;
	bool coulb_dis;
	bool init_ocv;
	bool cv_entered;
	bool cal_eoc_adj;
};

#define BATTERY_STATUS_UNKNOWN(flag)	\
	((flag.batt_status == POWER_SUPPLY_STATUS_UNKNOWN) || \
	  (flag.init_capacity))

struct bcmpmu_batt_cap_info {
	int max_design; /* maximum desing capacity of battery mAS*/
	int initial;	/* Initial capacity calculation */
	int capacity;	/* current capacity in mAs */
	int full_charge; /* full charged capacity of battery in percentage */
	int percentage; /* current capacity in percentage */
	int prev_percentage; /* previous capacity percentage */
	int prev_level; /* previous capacity level */
	int ocv_cap; /* open circult voltage capacity */
	int uuc; /* unusable capacity in percent */
	bool first_boot_init_cap_flat; /* capacity in flat area upon 1st boot */
	int cap_at_eoc;
};

struct batt_adc_data {
	int temp;
	int volt;
	int curr_inst;
	int esr;
};

struct avg_sample_buff {
	int curr[AVG_SAMPLES];
	int volt[AVG_SAMPLES];
	int idx;
	bool dirty;
};
struct fg_fc_cap_d {
	u16 cap[FG_FC_MAX_SAMPLES];
	u16 fg_fc_cap;
	u16 fc_samples;
};
#ifdef CONFIG_DEBUG_FS
struct fg_probes {
	int volt_avg;
	int adj_factor;
	int temp_factor;
	int capacity_delta;
};
#endif

/**
 * FG private data
 */
struct bcmpmu_fg_data {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex mutex;
	struct bcmpmu_fg_pdata *pdata;
	struct power_supply psy;

	/* works and workQ */
	struct workqueue_struct *fg_wq;
	struct delayed_work fg_periodic_work;
	struct work_struct low_batt_irq_work;

	/* Notifier blocks */
	struct notifier_block accy_nb;
	struct notifier_block usb_det_nb;
	struct notifier_block chrgr_status_nb;
	struct notifier_block chrgr_current_nb;
	struct notifier_block acld_nb;
	struct notifier_block display_nb;

	struct bcmpmu_batt_cap_info capacity_info;
	struct bcmpmu_fg_status_flags flags;
	struct batt_adc_data adc_data;
	struct avg_sample_buff avg_sample;
#ifdef CONFIG_WD_TAPPER
	struct wd_tapper_node wd_tap_node;
#else
	struct alarm alarm;
	struct wake_lock fg_alarm_wake_lock;
	int alarm_timeout;
#endif /*CONFIG_WD_TAPPER*/
	u64 last_sample_tm;
	u64 last_sample_ocv_tm;
	u64 last_curr_sample_tm;
	int last_curr_sample;

	enum bcmpmu_fg_cal_state cal_state;
	enum bcmpmu_fg_cal_mode cal_mode;
	enum bcmpmu_batt_charging_state charging_state;
	enum bcmpmu_batt_discharging_state discharge_state;
	enum bcmpmu_fg_sample_rate sample_rate;

	enum bcmpmu_chrgr_type_t chrgr_type;
	struct bcmpmu_battery_data *bdata;
	int eoc_adj_fct;
	int eoc_cap_delta;
	int eoc_current;
	int vfloat_lvl;
	int vf_zone;
	int vfloat_eoc;
	int low_cal_adj_fct;
	int high_cal_adj_fct;
	int cal_eoc_adj_fct;
	int low_volt_cnt;
	int cutoff_cap_cnt;
	int crit_cutoff_cap_prev;
	int crit_cutoff_cap;
	int crit_cutoff_delta;
	int cal_low_bat_cnt;
	int cal_high_bat_cnt;
	int cal_low_clr_cnt;
	int cal_high_clr_cnt;
	int cal_eoc_adj_cal_cnt;
	int eoc_cnt;
	int cap_inc_dec_cnt;
	int accumulator;
	int dummy_bat_cap_lmt;
	int prev_cap_delta;
	int max_discharge_current;
	int sleep_current_ua[2]; /* 0 = in use, 1 = in pending */
	bool used_init_cap;
	bool acld_enabled;
	int init_notifier;
	int prev_ocv;
	int ibat_avg;
	long int delta_volt;
	int delta_cap_mas;
	int cal_eoc_point;

	/* for debugging only */
	int lock_cnt;

#ifdef CONFIG_DEBUG_FS
	struct fg_probes probes;
#endif
	struct fg_fc_cap_d fcd;
};

#ifdef CONFIG_DEBUG_FS
#define DEBUG_FS_PERMISSIONS	(S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP)
#endif

#ifdef DEBUG
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |\
			BCMPMU_PRINT_FLOW | BCMPMU_PRINT_VERBOSE)
#else
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT)
#endif

static u32 debug_mask = DEBUG_MASK;
#define pr_fg(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[FG]:"args); \
		} \
	} while (0)

static bool ntc_disable;
core_param(ntc_disable, ntc_disable, bool, 0644);

/**
 * internal function prototypes
 */
static void bcmpmu_fg_batt_cal_algo(struct bcmpmu_fg_data *fg);
static void bcmpmu_fg_reset_adj_factors(struct bcmpmu_fg_data *fg);
static void bcmpmu_fg_update_psy(struct bcmpmu_fg_data *fg, bool force_update);
static int bcmpmu_fg_get_curr_inst(struct bcmpmu_fg_data *fg);
static int bcmpmu_fg_freeze_read(struct bcmpmu_fg_data *fg);
static inline int bcmpmu_fg_sample_rate_to_time(
		enum bcmpmu_fg_sample_rate rate);
static int bcmpmu_fg_register_notifiers(struct bcmpmu_fg_data *fg);
static int bcmpmu_fg_get_temp_factor(struct bcmpmu_fg_data *fg);

/**
 * Math - Utility functions
 */
static unsigned int factorial(unsigned int n)
{
	unsigned int res = 1;
	unsigned int prev;

	while (n > 1) {
		prev = res;
		res *= n;
		BUG_ON(prev >= res);
		n--;
	}

	return res;
}

long long power(long int num, int pow)
{
	long long res = 1;

	while (pow--)
		res *= num;

	return res;

}

/* Using Taylors Theorem */
long long exponential(int x)
{
	long long sum = 1000;
	long long den = 1;
	int i;

	for (i = 1; i < 9; i++) {
		sum += div64_s64(power(x, i), (factorial(i)*den));
		den *= 1000;
	}

	return sum;
}

/**
 * inline function
 */

static inline int percentage_to_capacity(struct bcmpmu_fg_data *fg, int per)
{
	u64 capacity, full_cap_tfact, tfact;

	tfact = bcmpmu_fg_get_temp_factor(fg);
	full_cap_tfact = fg->capacity_info.full_charge;
	full_cap_tfact = full_cap_tfact * tfact;
	full_cap_tfact = div64_u64(full_cap_tfact, 1000);
	capacity = per;
	capacity = capacity * full_cap_tfact;
	capacity = div64_u64(capacity, 100);
	pr_fg(FLOW, "cap %lld\n", capacity);
	return capacity;
}

static inline int capacity_to_percentage(struct bcmpmu_fg_data *fg, int cap)
{
	u64 capacity = 0, full_cap_tfact, tfact;
	int capdelta;


	tfact = bcmpmu_fg_get_temp_factor(fg);
	full_cap_tfact = fg->capacity_info.full_charge;
	full_cap_tfact = (full_cap_tfact * tfact);
	full_cap_tfact = div64_u64(full_cap_tfact, 1000);
	capdelta = fg->capacity_info.full_charge - cap;
	if (full_cap_tfact > capdelta) {
		capacity = ((full_cap_tfact - capdelta) * 100) +
			fg->capacity_info.full_charge / 2;
		capacity = div64_u64(capacity, full_cap_tfact);
	}
	return capacity;
}

static inline void fill_avg_sample_buff(struct bcmpmu_fg_data *fg)
{
	int i;
	fg->avg_sample.idx = 0;

	for (i = 0; i < AVG_SAMPLES; i++) {
		fg->avg_sample.curr[i] = bcmpmu_fg_get_curr_inst(fg);
		fg->avg_sample.volt[i] = bcmpmu_fg_get_batt_volt(fg->bcmpmu);
		fg->avg_sample.idx++;
	}
}

static inline void update_avg_sample_buff(struct bcmpmu_fg_data *fg)
{
	if (fg->avg_sample.dirty) {
		fill_avg_sample_buff(fg);
		fg->avg_sample.dirty = false;
	}

	if (fg->avg_sample.idx == AVG_SAMPLES)
		fg->avg_sample.idx = 0;
	fg->avg_sample.volt[fg->avg_sample.idx] = fg->adc_data.volt;
	fg->avg_sample.curr[fg->avg_sample.idx] = fg->adc_data.curr_inst;
	fg->avg_sample.idx++;
}

static inline void clear_avg_sample_buff(struct bcmpmu_fg_data *fg)
{
	fg->avg_sample.idx = 0;
	fg->avg_sample.dirty = true;
}

#ifndef CONFIG_WD_TAPPER
static void bcmpmu_fg_program_alarm(struct bcmpmu_fg_data *fg,
		long seconds)
{
	ktime_t interval = ktime_set(seconds, 0);
	ktime_t next;

	pr_fg(VERBOSE, "set timeout %ld s.\n", seconds);
	next = ktime_add(ktime_get_real(), interval);

	alarm_start(&fg->alarm, next);
}

static enum alarmtimer_restart bcmpmu_fg_alarm_callback(
		struct alarm *alarm, ktime_t now)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(alarm, alarm);
	/*wanna do something here?*/
	pr_fg(VERBOSE, "FG wakeup!\n");
	wake_lock(&fg->fg_alarm_wake_lock);
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
	return ALARMTIMER_NORESTART;
}
#endif


/**
 * bcmpmu_fgsmpl_to_curr - Converts current sample to current
 *
 * @smpl_7_0:	lower 8-bits of FGSMPL_CAL register
 * @smple_15_8 : Upper 8-bits of FGSMPL_CAL register
 *
 * Converts FG current sample register to current in mA. This function
 * can be overiden in board file
 */

__weak int bcmpmu_fgsmpl_to_curr(struct bcmpmu_fg_data *fg,
		u8 smpl_7_0, u8 smpl_15_8)
{
	int sample = 0;
	/*
	 * Current sample is represented in 2's complement format
	 * in PMU. if FGSMPL[15] is sign bit which represents current
	 * flow (positive current or negative current)
	 */

	sample = smpl_7_0 | (smpl_15_8 << 8);
	if (smpl_15_8 & 0x80)
		sample |= 0xFFFF0000; /* extend sign bit for int */
	if (sample > 0)
		sample += 2;
	else
		sample -= 2;
	return ((sample / 4) * fg->pdata->fg_factor) / 1000;
}

static inline int bcmpmu_fg_sample_rate_to_actual(
		enum bcmpmu_fg_sample_rate rate)
{
	int actual;
	switch (rate) {
	case SAMPLE_RATE_2HZ:
		actual = 2083;
		break;
	case SAMPLE_RATE_4HZ:
		actual = 4083;
		break;
	case SAMPLE_RATE_8HZ:
		actual = 8083;
		break;
	case SAMPLE_RATE_16HZ:
		actual = 1683;
		break;
	default:
		BUG_ON(1);
	}
	return actual;
}

static inline int bcmpmu_fg_sample_rate_to_time(
		enum bcmpmu_fg_sample_rate rate)
{
	int ms;
	switch (rate) {
	case SAMPLE_RATE_2HZ:
		ms = 500;
		break;
	case SAMPLE_RATE_4HZ:
		ms = 250;
		break;
	case SAMPLE_RATE_8HZ:
		ms = 125;
		break;
	case SAMPLE_RATE_16HZ:
		ms = 62;
		break;
	default:
		BUG_ON(1);
	}
	return ms;
}

__weak int bcmpmu_fg_accumulator_to_capacity(struct bcmpmu_fg_data *fg,
		int accm, int sample_cnt, int sleep_cnt)
{
	int accm_act;
	int accm_sleep;
	int cap_mas;
	s64 ibat_avg;
	s64 temp1;
	s64 temp2;
	int sample_rate;

	sample_rate = bcmpmu_fg_sample_rate_to_actual(fg->sample_rate);

	accm_act = ((accm * fg->pdata->fg_factor) / sample_rate);
	accm_sleep = ((sleep_cnt * fg->sleep_current_ua[0]) /
			fg->pdata->sleep_sample_rate);

	cap_mas = accm_act - accm_sleep;

	pr_fg(VERBOSE, "accm_act: %d accm_sleep: %d cap_mas: %d\n",
			accm_act, accm_sleep, cap_mas);

	if (sample_cnt || sleep_cnt) {
		temp1 = ((s64)sample_cnt * 1000000);
		do_div(temp1, sample_rate);
		temp2 = ((s64)sleep_cnt * 1000000);
		do_div(temp2, fg->pdata->sleep_sample_rate);
		pr_fg(VERBOSE, "temp1 = %lld temp2 = %lld\n", temp1, temp2);
		ibat_avg = ((cap_mas * 1000) / ((s32)temp1  + (s32)temp2));
		fg->ibat_avg = ibat_avg;
		pr_fg(FLOW, "AVG Current %lld\n", ibat_avg);
	}
	return cap_mas;
}

static bool bcmpmu_fg_is_batt_present(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;
	bool present = false;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_ENV5, &reg);

	if (!ret)
		present = !!(reg & ENV5_P_MBPD);
	return present;
}


static bool bcmpmu_fg_is_charger_present(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;
	bool present = false;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_ENV2, &reg);

	if (!ret)
		present = !!(reg & ENV2_USB_VALID);
	return present;
}

static int bcmpmu_fg_calibrate_offset(struct bcmpmu_fg_data *fg,
		enum bcmpmu_fg_offset_cal_mode mode)
{
	u8 reg;
	int ret;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	if (ret)
		return ret;

	switch (mode) {
	case FG_HW_CAL_MODE_FORCE:
		reg |= FGCTRL2_FGFORCECAL_MASK;
		break;
	case FG_HW_CAL_MODE_1PT:
		reg |= FGCTRL2_FG1PTCAL_MASK;
		break;
	case FG_HW_CAL_MODE_LONGCAL:
		reg |= FGCTRL2_LONGCAL_MASK;
		break;
	case FG_HW_CAL_MODE_FAST:
		reg |= FGCTRL2_FGCAL_MASK;
		break;
	default:
		BUG_ON(1);
	}
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);
	return ret;
}

static int bcmpmu_fg_set_sample_rate(struct bcmpmu_fg_data *fg,
		enum bcmpmu_fg_sample_rate rate)
{
	int ret;
	u8 reg;

	if ((rate < SAMPLE_RATE_2HZ) || (rate > SAMPLE_RATE_16HZ))
		return -EINVAL;
	fg->sample_rate = rate;
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, &reg);
	if (ret)
		return ret;
	reg &= ~FGOCICCTRL_FGCOMBRATE_MASK;
	reg |= (rate << FGOCICCTRL_FGCOMBRATE_SHIFT);

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, reg);

	pr_fg(INIT, "set CIC sample rate to %d\n", rate);

	return ret;
}

static int bcmpmu_fg_enable(struct bcmpmu_fg_data *fg, bool enable)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL1, &reg);
	if (!ret) {
		if (enable)
			reg |= FGCTRL1_FGHOSTEN_MASK;
		else
			reg &= ~FGCTRL1_FGHOSTEN_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL1, reg);
	}
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL1, &reg);
	return ret;
}
/**
 * bcmpmu_fg_reset - Reset FG HW and coulomb counters
 *
 * @bcmpmu_fg_data:		Pointer to bcmpmu_fg_data struct
 *
 * Reset the PMU Fuel Gauage and clear the counters
 */
static int bcmpmu_fg_reset(struct bcmpmu_fg_data *fg)
{
	u8 reg;
	int ret;
	u8 accm[4];
	u8 act_cnt[2];
	u8 sleep_cnt[2];

	pr_fg(INIT, "Reset Fuel Gauge HW\n");
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	if (!ret) {
		reg |= FGCTRL2_FGRESET_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);
	}

	/** dummy read to clear the ACCMx and CNTx registers
	 */
	ret = bcmpmu_fg_freeze_read(fg);
	if (ret)
		return ret;

	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGACCM1, accm, 4);
	if (ret)
		return ret;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGCNT1, act_cnt, 2);
	if (ret)
		return ret;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGSLEEPCNT1,
			sleep_cnt, 2);
	if (ret)
		return ret;

	fg->last_sample_tm = kona_hubtimer_get_counter();

	return 0;
}

static int bcmpmu_fg_enable_coulb_counter(struct bcmpmu_fg_data *fg,
		bool enable)
{
	if (fg->flags.coulb_dis == enable) {
		bcmpmu_fg_reset(fg);
		if (enable) {
			bcmpmu_fg_enable(fg, true);
			fg->flags.coulb_dis = false;
		} else {
			bcmpmu_fg_enable(fg, false);
			fg->flags.coulb_dis = true;
		}
	}
	return 0;
}

/**
 * bcmpmu_fg_freeze_read - Freeze FG coulomb counter for read
 *
 * @bcmpmu_fg_data:		Pointer to bcmpmu_fg_data struct
 *
 * Writes to FGFRZREAD bit which will freeze the coulomb counter
 * and latch the accumulator registers and sample counter register
 */

static int bcmpmu_fg_freeze_read(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	if (!ret) {
		reg |= FGCTRL2_FGFRZREAD_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);
	}
	return ret;
}

static int bcmpmu_fg_volt_to_cap(struct bcmpmu_fg_data *fg, int volt)
{
	struct batt_volt_cap_cpt_map *lut;
	u32 lut_sz;
	int cap_percentage = 0;
	int idx;

	lut = fg->bdata->batt_prop->volt_cap_cpt_lut;
	lut_sz = fg->bdata->batt_prop->volt_cap_cpt_lut_sz;

	for (idx = 0; idx < lut_sz; idx++) {
		if (volt >= lut[idx].volt)
			break;
	}
	if ((idx > 0) && (idx < lut_sz)) {
		cap_percentage = INTERPOLATE_LINEAR(volt,
				lut[idx].volt,
				lut[idx].cap,
				lut[idx - 1].volt,
				lut[idx - 1].cap);

	} else if (idx == 0)
		cap_percentage = 100; /* full capacity */

	return cap_percentage;
}

static int bcmpmu_fg_get_temp_factor(struct bcmpmu_fg_data *fg)
{
	struct batt_esr_temp_lut *lut =	fg->bdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->bdata->batt_prop->esr_temp_lut_sz;
	int idx;
	int temp;
	int temp_fact;

	BUG_ON(!lut);

	temp = fg->adc_data.temp;
	if (temp <= lut[0].temp)
		temp_fact = lut[0].fct;
	else if (temp >= lut[lut_sz - 1].temp)
		temp_fact = lut[lut_sz - 1].fct;
	else {
		/*find esr zone */
		for (idx = 0; idx < lut_sz; idx++) {
			if ((temp >= lut[idx].temp) &&
					(temp < lut[idx + 1].temp))
				break;
		}

		if (idx == (lut_sz - 1))
			BUG_ON(1);
		temp_fact = INTERPOLATE_LINEAR(temp,
						lut[idx].temp,
						lut[idx].fct,
						lut[idx + 1].temp,
						lut[idx + 1].fct);

	}

	pr_fg(FLOW, "temperature factor = %d\n", temp_fact);
	return temp_fact;
}
static int bcmpmu_fg_get_esr_guardband(struct bcmpmu_fg_data *fg)
{
	struct batt_esr_temp_lut *lut =	fg->bdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->bdata->batt_prop->esr_temp_lut_sz;
	int idx;
	int temp;

	BUG_ON(!lut);
	temp = fg->adc_data.temp;

	/*find esr zone */
	for (idx = 0; idx < lut_sz; idx++) {
		if (temp <= lut[idx].temp)
			break;
	}
	if (idx == lut_sz)
		idx = lut_sz - 1;
	return lut[idx].guardband;
}

static int bcmpmu_fg_get_cutoff_capacity(struct bcmpmu_fg_data *fg, int volt)
{
	struct batt_cutoff_cap_map *lut = fg->bdata->batt_prop->cutoff_cap_lut;
	int lut_sz = fg->bdata->batt_prop->cutoff_cap_lut_sz;
	int idx;
	int cutoff_cap;

	BUG_ON(!lut);

	if (volt > lut[0].volt)
		return -1;

	for (idx = 0; idx < lut_sz - 1; idx++) {
		if ((volt <= lut[idx].volt) &&
				(volt >= lut[idx + 1].volt))
			break;
	}

	cutoff_cap = lut[idx].cap;
	pr_fg(VERBOSE, "volt: %d cutoff cap: %d\n", volt, cutoff_cap);
	return cutoff_cap;
}

static int bcmpmu_fg_eoc_curr_to_capacity(struct bcmpmu_fg_data *fg, int curr)
{
	struct batt_eoc_curr_cap_map *lut = fg->bdata->batt_prop->eoc_cap_lut;
	u32 lut_sz = fg->bdata->batt_prop->eoc_cap_lut_sz;
	int idx;

	if (!lut)
		return -EINVAL;

	for (idx = 0; idx < lut_sz - 1; idx++)
		if ((curr < lut[idx].eoc_curr) &&
				(curr >= lut[idx + 1].eoc_curr))
			break;
	return lut[idx].capacity;
}

static int bcmpmu_fg_get_batt_esr(struct bcmpmu_fg_data *fg, int volt, int temp)
{
	struct batt_esr_temp_lut *lut = fg->bdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->bdata->batt_prop->esr_temp_lut_sz;
	int slope, offset, esr;
	int idx;

	if (!lut) {
		pr_fg(ERROR, "ESR<->TEMP table is not defined\n");
		return 0;
	}

	/*find esr zone */
	for (idx = 0; idx < lut_sz; idx++) {
		if (temp <= lut[idx].temp)
			break;
	}
	if (idx == lut_sz)
		idx = lut_sz - 1;
	if (volt < lut[idx].esr_vl_lvl) {
		slope = lut[idx].esr_vl_slope;
		offset = lut[idx].esr_vl_offset;
	} else if (volt < lut[idx].esr_vm_lvl) {
		slope = lut[idx].esr_vm_slope;
		offset = lut[idx].esr_vm_offset;
	} else if (volt < lut[idx].esr_vh_lvl) {
		slope = lut[idx].esr_vh_slope;
		offset = lut[idx].esr_vh_offset;
	} else {
		slope = lut[idx].esr_vf_slope;
		offset = lut[idx].esr_vf_offset;
	}
	esr = (volt * slope) / 1000;
	esr += offset;

	return esr;
}

static int bcmpmu_fg_get_esr_to_ocv(int volt, int curr, int offset,
		int slope)
{
	int ocv = 0;
	s64 temp, temp1;
	temp = 1000000 + (s64)(slope * curr);
	temp1 = 1000 * ((s64)(1000 * volt) - (s64)(offset * curr));

	if (temp != 0)
		ocv = div64_s64(temp1, temp);

	return ocv;
}

static int bcmpmu_fg_get_esr(int volt, int curr, int offset,
		int slope)
{
	int esr = 0;
	s64 temp, temp1;
	temp = ((s64)(curr * slope) / 1000) + 1000;
	temp1 = (s64)(1000 * offset) + (s64)(volt * slope);

	if (temp != 0)
		esr = div64_s64(temp1, temp);
	return esr;
}

static int bcmpmu_fg_get_esr_from_ocv(int ocv, int offset, int slope,
		int per_margin)
{
	s64 temp, temp1;
	/**
	 *ESR := (offset + (ocv * slope /1000)) * per_margin
	 */
	temp = per_margin * ((1000 * offset) + (s64)(ocv * slope));
	temp1 = 100000;

	return div64_s64(temp, temp1);
}

static int bcmpmu_fg_get_capacitance(struct bcmpmu_fg_data *fg, int volt)
{
	struct batt_volt_cap_cpt_map *lut;
	u32 lut_sz;
	int capacitance = 0;
	int idx;

	lut = fg->bdata->batt_prop->volt_cap_cpt_lut;
	lut_sz = fg->bdata->batt_prop->volt_cap_cpt_lut_sz;

	for (idx = 0; idx < lut_sz; idx++) {
		if (volt >= lut[idx].volt)
			break;
	}
	if ((idx > 0) && (idx < lut_sz)) {
		capacitance = INTERPOLATE_LINEAR(volt,
			(int)lut[idx].volt,
			(int)lut[idx].cpt,
			(int)lut[idx - 1].volt,
			(int)lut[idx - 1].cpt);


	} else if (idx == 0)
		capacitance = lut[idx].cpt; /* capacitance at Max Voltage */

	return capacitance;
}

static int bcmpmu_fg_get_esr_to_ocv_with_cap
	(struct bcmpmu_fg_data *fg, int volt, int curr, int esr)
{
	u64 t_now;
	int t_ms;
	long int cap_farad;
	long int exp;
	long long temp1;
	int ocv;

	ocv = fg->prev_ocv;

	t_now = kona_hubtimer_get_counter();
	t_ms = ((t_now - fg->last_sample_ocv_tm) * 1000)/CLOCK_TICK_RATE;

	cap_farad = bcmpmu_fg_get_capacitance(fg, fg->prev_ocv);
	exp = -1 * (t_ms*1000)/(cap_farad * esr);
	temp1 = exponential(exp);

	ocv = (volt*1000) + (temp1 * (fg->prev_ocv - volt));
	ocv = DIV_ROUND_CLOSEST(ocv, 1000);

	pr_fg(VERBOSE, "[NEWMATH] t_ms: %d esr: %d, cap_farad: %ld, exp: %ld\n",
		t_ms, esr, cap_farad, exp);
	pr_fg(VERBOSE, "[NEWMATH] exponential(exp): %lld, p_ocv: %d, ocv: %d\n",
		temp1, fg->prev_ocv, ocv);

	return ocv;
}

static int bcmpmu_fg_is_cv_entered(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;
	bool entered = false;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_ENV3, &reg);

	if (!ret)
		entered = !!(reg & ENV3_P_MBC_CV);

	fg->flags.cv_entered = entered ? true : false;

	return entered;
}

static int bcmpmu_fg_get_batt_ocv(struct bcmpmu_fg_data *fg, int volt, int curr,
		int temp)
{
	struct batt_esr_temp_lut *lut = fg->bdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->bdata->batt_prop->esr_temp_lut_sz;
	int slope = 0, offset = 0, ocv;
	int idx;
	int min_volt;
	int max_volt;
	int esr, esr_vl, esr_vf;
	u64 t_now;
	int t_ms;
	long int delta_q;
	long int cap_farad;
	int delta_volt = 0;

	if (!lut) {
		pr_fg(ERROR, "ESR<->TEMP table is not defined\n");
		return 0;
	}

	min_volt = fg->bdata->batt_prop->min_volt;
	max_volt = fg->bdata->batt_prop->max_volt;
	ocv = 0;

	/*find esr zone */
	for (idx = 0; idx < lut_sz; idx++) {
		if (temp <= lut[idx].temp)
			break;
	}
	if (idx == lut_sz)
		idx = lut_sz - 1;

	/**
	* discharging mode:
	*	calculate ocv for each zone and qualify it for right
	*	zone.
	*	OCV = ((vbat + esr_offset(ESR voltage zone) x ibat) /
	*		(1 - esr_slope(ESR voltage zone))x ibat))
	*	if (ocv < esr_vx_lvl) - This OCV qualifies this zone
	*	and its right OCV
	*
	*Charging mode:
	*	In case of charging, calculate esr for each zone and
	*	qualify it for right zone
	*/

	if ((!fg->flags.init_ocv) ||
		((curr < 0) && !fg->flags.chrgr_connected)) {

		slope = lut[idx].esr_vl_slope;
		offset = lut[idx].esr_vl_offset;
		ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
		if ((ocv > min_volt) && (ocv <= lut[idx].esr_vl_lvl))
			goto exit;

		slope = lut[idx].esr_vm_slope;
		offset = lut[idx].esr_vm_offset;
		ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
		if ((ocv > lut[idx].esr_vl_lvl) && (ocv <= lut[idx].esr_vm_lvl))
			goto exit;

		slope = lut[idx].esr_vh_slope;
		offset = lut[idx].esr_vh_offset;
		ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
		if ((ocv > lut[idx].esr_vm_lvl) && (ocv <= lut[idx].esr_vh_lvl))
			goto exit;

		slope = lut[idx].esr_vf_slope;
		offset = lut[idx].esr_vf_offset;
		ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
	} else if (!bcmpmu_fg_is_cv_entered(fg)) {
		curr = (fg->ibat_avg
			== FAKE_IBAT_INTIAL_AVG) ? curr : fg->ibat_avg;
		ocv = fg->prev_ocv;
		t_now = kona_hubtimer_get_counter();
		t_ms =
		((t_now - fg->last_sample_ocv_tm) * 1000)/CLOCK_TICK_RATE;

		/* Div by 1000 for seconds */
		delta_q =
			(curr * t_ms)/1000;
		cap_farad = bcmpmu_fg_get_capacitance(fg, fg->prev_ocv);
		fg->delta_volt += (delta_q * 1000)/cap_farad; /* In mV */

		if (abs(fg->delta_volt) > 1000) {
			delta_volt = fg->delta_volt/1000;
			fg->delta_volt -= delta_volt * 1000;
			ocv = fg->prev_ocv + delta_volt;
		}

		pr_fg(VERBOSE,
			"[NEWMATH] delta_q: %ld, cap_farad: %ld, t_ms: %d\n",
			delta_q, cap_farad, t_ms);
		pr_fg(VERBOSE,
			"[NEWMATH] delta_volt: %d, p_ocv: %d, ocv: %d\n",
			 delta_volt, fg->prev_ocv, ocv);
	} else {
		curr = (fg->ibat_avg
			== FAKE_IBAT_INTIAL_AVG) ? curr : fg->ibat_avg;
		esr_vl = bcmpmu_fg_get_esr_from_ocv(lut[idx].esr_vl_lvl,
			lut[idx].esr_vl_offset,
			lut[idx].esr_vl_slope,
			ESR_VL_PER_MARGIN);

		esr_vf = bcmpmu_fg_get_esr_from_ocv(max_volt,
			lut[idx].esr_vf_offset,
			lut[idx].esr_vf_slope,
			ESR_VF_PER_MARGIN);

		pr_fg(VERBOSE, "esr_vf: %d esr_vl: %d\n", esr_vf, esr_vl);

		slope = lut[idx].esr_vl_slope;
		offset = lut[idx].esr_vl_offset;
		esr = bcmpmu_fg_get_esr(volt, curr, offset, slope);
		if ((esr > esr_vf) && (esr <= esr_vl)) {
			ocv =
			bcmpmu_fg_get_esr_to_ocv_with_cap(fg, volt, curr, esr);
			if ((ocv <= lut[idx].esr_vl_lvl) && (ocv >= min_volt))
				goto exit;
		}

		slope = lut[idx].esr_vm_slope;
		offset = lut[idx].esr_vm_offset;
		esr = bcmpmu_fg_get_esr(volt, curr, offset, slope);
		if ((esr > esr_vf) && (esr <= esr_vl)) {
			ocv =
			bcmpmu_fg_get_esr_to_ocv_with_cap(fg, volt, curr, esr);
			if ((ocv <= lut[idx].esr_vm_lvl))
				goto exit;
		}

		slope = lut[idx].esr_vh_slope;
		offset = lut[idx].esr_vh_offset;
		esr = bcmpmu_fg_get_esr(volt, curr, offset, slope);
		if ((esr > esr_vf) && (esr <= esr_vl)) {
			ocv =
			bcmpmu_fg_get_esr_to_ocv_with_cap(fg, volt, curr, esr);
			if ((ocv <= lut[idx].esr_vh_lvl))
				goto exit;
		}

		slope = lut[idx].esr_vf_slope;
		offset = lut[idx].esr_vf_offset;
		esr = bcmpmu_fg_get_esr(volt, curr, offset, slope);
		if ((esr > esr_vf) && (esr <= esr_vl))
			ocv =
			bcmpmu_fg_get_esr_to_ocv_with_cap(fg, volt, curr, esr);
			if ((ocv <= max_volt))
				goto exit;
	}

	if (ocv < min_volt) {
		pr_fg(ERROR, "OCV below min voltage!!\n");
		ocv = min_volt;
	} else if (ocv > max_volt) {
		pr_fg(ERROR, "OCV above max voltage!!\n");
		ocv = max_volt;
	}
exit:
	pr_fg(VERBOSE, "fg_zone:%d volt: %d curr: %d temp: %d\n",
			idx, volt, curr, temp);
	pr_fg(VERBOSE, "FG_ZONE: slope: %d offset: %d ocv: %d\n",
			slope, offset, ocv);

	fg->prev_ocv = ocv;
	fg->last_sample_ocv_tm = kona_hubtimer_get_counter();

	return ocv;
}

int bcmpmu_fg_get_batt_volt(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_adc_result result;
	int ret;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(bcmpmu, PMU_ADC_CHANN_VMBATT,
				PMU_ADC_REQ_RTM_MODE, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}
int bcmpmu_fg_get_avg_volt(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_fg_data *fg = (struct bcmpmu_fg_data *)bcmpmu->fg;
	int i = 0;
	int volt_sum = 0;
	int volt_samples[ADC_VBAT_AVG_SAMPLES];


	do {
		volt_samples[i] = bcmpmu_fg_get_batt_volt(bcmpmu);

		/**
		 * voltage too high?? may be wrong ADC
		 * sample or connected to power supply > 4.2V
		 */
		if (volt_samples[i] > fg->bdata->batt_prop->max_volt) {
			pr_fg(ERROR, "VBAT > MAX_VOLT");
			volt_samples[i] = fg->bdata->batt_prop->max_volt;
		}

		volt_sum += volt_samples[i];
		i++;
		msleep(25);
	} while (i < ADC_VBAT_AVG_SAMPLES);

	return interquartile_mean(volt_samples, ADC_VBAT_AVG_SAMPLES);
}

static inline int bcmpmu_fg_get_batt_temp(struct bcmpmu_fg_data *fg)
{
	int temp_samples[ADC_NTC_AVG_SAMPLES] = {0};
	struct bcmpmu_adc_result result;
	int retries;
	static int temp_prev = 0xffff;
	int ret = 0, i = 0;
	bool mean = true;

	if (ntc_disable)
		return NTC_ROOM_TEMP;

	do {
		retries = ADC_READ_TRIES;
		while (retries--) {
			ret = bcmpmu_adc_read(fg->bcmpmu, PMU_ADC_CHANN_NTC,
					PMU_ADC_REQ_RTM_MODE, &result);
			if (!ret)
				break;
			msleep(ADC_RETRY_DELAY);
		}

		BUG_ON(retries <= 0);

		if ((temp_prev == 0xffff) ||
			((result.conv < (temp_prev + NTC_TEMP_OFFSET)) &&
			(result.conv > (temp_prev - NTC_TEMP_OFFSET)))) {
			temp_prev = result.conv;
			mean = false;
			break;
		}
		temp_samples[i] = result.conv;
		msleep(ADC_RETRY_DELAY);
		i++;

	} while (i < ADC_NTC_AVG_SAMPLES);

	if (mean)
		temp_prev = interquartile_mean(temp_samples, i);

	return temp_prev;
}

static int bcmpmu_fg_get_curr_inst(struct bcmpmu_fg_data *fg)
{
	u64 t_now;
	u64 t_ms;
	int ret = 0;
	u8 reg;
	u8 smpl_cal[2];

	FG_LOCK(fg);
	/**
	 * Avoid reading instant current register earlier than
	 * next sample available
	 */
	t_now = kona_hubtimer_get_counter();
	t_ms = ((t_now - fg->last_curr_sample_tm) * 1000)/CLOCK_TICK_RATE;

	if (t_ms < bcmpmu_fg_sample_rate_to_time(fg->sample_rate))
		msleep(bcmpmu_fg_sample_rate_to_time(fg->sample_rate) - t_ms);


	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	/**
	 * Write 1  to FGFRZSMPL bit to latch the current sample to
	 * FGSMPL_CAL registers
	 */
	reg |= FGCTRL2_FGFRZSMPL_MASK;
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);

	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGSMPL3, smpl_cal,
			2);
	BUG_ON(ret != 0);

	fg->last_curr_sample_tm = kona_hubtimer_get_counter();
	fg->last_curr_sample =
		bcmpmu_fgsmpl_to_curr(fg, smpl_cal[1], smpl_cal[0]);
	FG_UNLOCK(fg);
	pr_fg(FLOW, "%s:fg->last_curr_sample=%d\n",
		__func__, fg->last_curr_sample);
	return fg->last_curr_sample;
}

int bcmpmu_fg_get_one_c_rate(struct bcmpmu59xxx *bcmpmu, int *one_c_rate)
{
	struct bcmpmu_fg_data *fg = (struct bcmpmu_fg_data *)bcmpmu->fg;

	if (!fg)
		return -EAGAIN;

	*one_c_rate = fg->bdata->batt_prop->one_c_rate;

	return 0;
}
/* bcmpmu_fg_get_batt_curr - Gives instantaneous battery current through FG
 * Returns 0 on Success, -EAGAIN on failure
 *
 * */
int bcmpmu_fg_get_batt_curr(struct bcmpmu59xxx *bcmpmu, int *curr)
{
	struct bcmpmu_fg_data *fg = (struct bcmpmu_fg_data *)bcmpmu->fg;
	int retries = ADC_READ_TRIES;

	if (!fg)
		return -EAGAIN;

	if (fg->flags.coulb_dis) {
		pr_fg(INIT, "%s: Get Ibat directly from FG\n", __func__);
		bcmpmu_fg_enable_coulb_counter(fg, true);
		while (retries--) {
			*curr = bcmpmu_fg_get_curr_inst(fg);
			pr_fg(FLOW, "curr = %d retrie = %d\n", *curr, retries);
			if ((*curr < FG_CURR_SAMPLE_MAX) &&
					(*curr > -FG_CURR_SAMPLE_MAX))
				break;
			msleep(bcmpmu_fg_sample_rate_to_time(
						fg->sample_rate));
		}
		bcmpmu_fg_enable_coulb_counter(fg, false);
		if (retries <= 0)
			return -EAGAIN;
	} else
		*curr = bcmpmu_fg_get_curr_inst(fg);

	return 0;
}


int bcmpmu_fg_get_cur(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_fg_data *fg = (struct bcmpmu_fg_data *)bcmpmu->fg;

	if (!fg)
		return 0;

	return fg->adc_data.curr_inst;
}
EXPORT_SYMBOL(bcmpmu_fg_get_cur);

/**
 * bcmpmu_fg_can_battery_be_full - Can battery be full?
 * @bcmpmu_fg_data:	Pointer to bcmpmu_fg_data struct
 *
 * Determine if battery can be fully charged i.e. to its
 * maximum allowed voltage.
 */
static bool bcmpmu_fg_can_battery_be_full(struct bcmpmu_fg_data *fg)
{
	return fg->vfloat_lvl == fg->bdata->volt_levels->vfloat_lvl;
}

/**
 * bcmpmu_fg_get_load_comp_capacity - Get Battery open circuit capacity
 *
 * @bcmpmu_fg_data:		Pointer to bcmpmu_fg_data struct
 *
 * Determine load compensated battery capacity (open circuit voltage to
 * capacity)
 */

static int bcmpmu_fg_get_load_comp_capacity(struct bcmpmu_fg_data *fg,
		bool load_comp)
{
	int vbat_oc = 0;
	int capacity_percentage = 0;

	fg->adc_data.curr_inst = bcmpmu_fg_get_curr_inst(fg);
	fg->adc_data.volt = bcmpmu_fg_get_batt_volt(fg->bcmpmu);
	fg->adc_data.temp = bcmpmu_fg_get_batt_temp(fg);

	if (abs(fg->adc_data.curr_inst) > FG_CURR_SAMPLE_MAX)
		fg->adc_data.curr_inst = 0;

	if (load_comp) {
		vbat_oc = bcmpmu_fg_get_batt_ocv(fg,
				fg->adc_data.volt,
				fg->adc_data.curr_inst,
				fg->adc_data.temp);
		capacity_percentage = bcmpmu_fg_volt_to_cap(fg, vbat_oc);
	} else
		capacity_percentage = bcmpmu_fg_volt_to_cap(fg,
				fg->adc_data.volt);

	if (!fg->flags.init_capacity)
		update_avg_sample_buff(fg);

	pr_fg(FLOW,
			"vbat: %d, vbat_comp: %d, ocv_cap: %d, Tntc: %d, Ic: %d, "
			"Iavg: %d, Cap: %d\n",
			fg->adc_data.volt, vbat_oc, capacity_percentage,
			fg->adc_data.temp, fg->adc_data.curr_inst,
			fg->ibat_avg, fg->capacity_info.percentage);

	BUG_ON(capacity_percentage > 100);
	return capacity_percentage;
}

static int bcmpmu_fg_get_adj_factor(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_battery_data *bdata = fg->bdata;
	struct batt_adc_data *adc = &fg->adc_data;
	struct batt_eoc_curr_cap_map *lut = bdata->batt_prop->eoc_cap_lut;
	int capacity = fg->capacity_info.percentage;
	int capacity_eoc;
	int adj_factor = 0;
	bool charging;

	charging = ((fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING) ?
			true : false);

	if (lut && charging && !fg->flags.fg_eoc) {
		if ((capacity >= lut[0].capacity) &&
				(capacity < CAPACITY_PERCENTAGE_FULL) &&
				(adc->curr_inst > 0) &&
				(adc->curr_inst <= lut[0].eoc_curr)) {
			capacity_eoc =
				bcmpmu_fg_eoc_curr_to_capacity(fg,
						adc->curr_inst);
			if (bcmpmu_fg_can_battery_be_full(fg) &&
				capacity < CAPACITY_PERCENTAGE_FULL)
				adj_factor = (((capacity_eoc - capacity) *
							100) /
						(capacity - 100));
			/*If not in limits then set to Max/Min adj
			 * factor allowed
			 */
			if (adj_factor > MAX_EOC_ADJ_FACTOR)
				adj_factor = MAX_EOC_ADJ_FACTOR;
			else if (adj_factor < MIN_EOC_ADJ_FACTOR)
				adj_factor = MIN_EOC_ADJ_FACTOR;

			fg->eoc_adj_fct = adj_factor;

			pr_fg(FLOW, "EOC capacity: %d factor: %d\n",
					capacity_eoc, adj_factor);
		}
	} else if (!charging) {
		if (fg->high_cal_adj_fct)
			adj_factor = fg->high_cal_adj_fct;
		else
			adj_factor = fg->low_cal_adj_fct;
	}

#ifdef CONFIG_DEBUG_FS
	fg->probes.adj_factor = adj_factor;
#endif

	return adj_factor;
}

static void bcmpmu_fg_reset_adj_factors(struct bcmpmu_fg_data *fg)
{
	fg->flags.high_bat_cal = false;
	fg->flags.low_bat_cal = false;
	fg->flags.calibration = false;
	fg->cal_high_bat_cnt = 0;
	fg->cal_low_bat_cnt = 0;
	fg->low_cal_adj_fct = 0;
	fg->high_cal_adj_fct = 0;
	fg->cal_eoc_adj_fct = 0;
	fg->cal_eoc_adj_cal_cnt = 0;
	fg->cal_high_clr_cnt = 0;
	fg->cal_low_clr_cnt = 0;
	fg->prev_cap_delta = 0;
}

static void bcmpmu_fg_reset_cutoff_cnts(struct bcmpmu_fg_data *fg)
{
	fg->crit_cutoff_cap = -1;
	fg->crit_cutoff_cap_prev = fg->crit_cutoff_cap;
	fg->crit_cutoff_delta = 0;
	fg->cutoff_cap_cnt = 0;
}

static int bcmpmu_fg_get_uuc(struct bcmpmu_fg_data *fg)
{
	int curr_avg;
	int unusable_volt;
	int cutoff;
	int esr;

	update_avg_sample_buff(fg);
	curr_avg = interquartile_mean(fg->avg_sample.curr, AVG_SAMPLES);

	/* Store the maximum average current during a discharge cycle.
	 * This will be used to track the unusable capacity due to the
	 * battery voltage under load will go faster to the cutoff voltage.
	 * When charging this get reset.
	 */
	if (curr_avg < 0) {
		/* Sanity check on the current. If OCV is higher than the
		   maximum allowed voltage on the battery either the
		   voltage or/and current measurement has gone wrong.
		*/
		esr = bcmpmu_fg_get_batt_esr(fg, fg->adc_data.volt,
					fg->adc_data.temp);
		if ((fg->adc_data.volt - (esr * curr_avg) / 1000) >
			fg->bdata->batt_prop->max_volt) {
			clear_avg_sample_buff(fg);
		} else {
			fg->max_discharge_current =
				min(fg->max_discharge_current, curr_avg);
		}
	} else if (fg->max_discharge_current < 0 && curr_avg > 0) {
		fg->max_discharge_current = 0;
	}

	if (!fg->max_discharge_current)
		return 0;

	unusable_volt = fg->bdata->batt_prop->min_volt;
	do {
		unusable_volt += 25;
		esr = bcmpmu_fg_get_batt_esr(fg, unusable_volt,
					fg->adc_data.temp);
		cutoff = unusable_volt +
			(esr * fg->max_discharge_current) / 1000;
	} while (cutoff <= fg->bdata->batt_prop->min_volt &&
		unusable_volt < fg->bdata->batt_prop->max_volt);

	return bcmpmu_fg_volt_to_cap(fg, unusable_volt);
}

static int bcmpmu_fg_get_usable_cap_from_ocv_cap(int ocv_cap, int uuc)
{
	return max(ocv_cap - uuc, CAPACITY_PERCENTAGE_EMPTY);
}

static void bcmpmu_fg_update_adj_factor(struct bcmpmu_fg_data *fg)
{
	int capacity_delta;
	int cal_volt_low;
	int cal_cap_low;
	int guardband;
	int usable_ocv_cap;
	bool charging;

	charging = ((fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING) ?
			true : false);
	if (charging) {
		/**
		 * These factors are cleared in the bcmpmu_fg_event_handler when
		 * PMU_ACCY_EVT_OUT_CHRGR_TYPE event is received but
		 * there is a possiblity that when
		 * PMU_ACCY_EVT_OUT_CHRGR_TYPE event is recieved,
		 * calibration algo is already running and high/low calibration
		 * factor is updated by the calibration algo after being cleared
		 * from event_handler.
		 */
		if (fg->high_cal_adj_fct || fg->low_cal_adj_fct) {
			fg->high_cal_adj_fct = 0;
			fg->low_cal_adj_fct = 0;
			fg->cal_high_clr_cnt = 0;
			fg->cal_low_clr_cnt = 0;
			fg->prev_cap_delta = 0;
		}
		return;
	}

	usable_ocv_cap =
		bcmpmu_fg_get_usable_cap_from_ocv_cap(fg->capacity_info.ocv_cap,
						fg->capacity_info.uuc);
	capacity_delta = abs(fg->capacity_info.percentage - usable_ocv_cap);
	cal_volt_low = fg->bdata->cal_data->volt_low;
	cal_cap_low = fg->bdata->cal_data->cap_low;
	guardband = bcmpmu_fg_get_esr_guardband(fg);

	if (fg->eoc_adj_fct) {
		pr_fg(FLOW, "clear eoc_adj_fct\n");
		fg->eoc_adj_fct = 0;
	}

	/**
	 * low battery calibration:
	 * if there is no other calibration running
	 * and battery voltage is below calibration low
	 * voltage and capacity is GREATER than low calibration
	 * capacity and debounced  3 rounds -> calibrated the battery
	 * in next round of algo
	 */
	if (!fg->flags.calibration &&
		(!fg->flags.low_bat_cal) &&
		(fg->adc_data.volt <= cal_volt_low) &&
		(fg->cal_low_bat_cnt++ > CAL_CNT_THRESHOLD)) {
		/**
		 * Perfect : Lets do low battery calibration
		 * now!!
		 */
		fg->cal_mode = CAL_MODE_LOW_BATT;
		fg->flags.calibration = true;
		fg->cal_low_bat_cnt = 0;
		fg->flags.low_bat_cal = true;
		return;
	} else if ((fg->cal_low_bat_cnt > 0) &&
			(fg->adc_data.volt > cal_volt_low)) {
		fg->cal_low_bat_cnt = 0;
		pr_fg(FLOW, "cal_low_bat_cnt cleared\n");
	} else if (!fg->flags.calibration &&
			!fg->flags.high_bat_cal &&
			(capacity_delta >= guardband) &&
			(fg->cal_high_bat_cnt++ > CAL_CNT_THRESHOLD)) {
		fg->cal_mode = CAL_MODE_HI_BATT;
		fg->flags.calibration = true;
		fg->cal_high_bat_cnt = 0;
		fg->flags.high_bat_cal = true;
		fg->flags.cal_eoc_adj = false;
		fg->flags.fully_charged = false;
	} else if (fg->flags.cal_eoc_adj &&
			(fg->delta_cap_mas > 0) &&
			(fg->capacity_info.percentage < fg->cal_eoc_point)) {
		fg->cal_mode = CAL_MODE_EOC_ADJ;
		fg->flags.calibration = true;
	} else if ((fg->cal_high_bat_cnt > 0) &&
			(capacity_delta < guardband)) {
		fg->cal_high_bat_cnt = 0;
		pr_fg(FLOW, "cal_high_bat_cnt cleared\n");
	}

	if ((fg->low_cal_adj_fct) &&
			(capacity_delta <= 1) &&
			(fg->cal_low_clr_cnt++ > CAL_CNT_THRESHOLD)) {
		fg->flags.low_bat_cal = false;
		fg->low_cal_adj_fct = 0;
		fg->cal_low_clr_cnt = 0;
		pr_fg(FLOW, "clear low_cal_adj_fct\n");
	} else if ((fg->cal_low_clr_cnt > 0) &&
			(capacity_delta > 1)) {
		fg->cal_low_clr_cnt = 0;
		pr_fg(FLOW, "cal_low_clr_cnt cleared\n");
	} else if (fg->flags.low_bat_cal &&
		fg->prev_cap_delta > capacity_delta) {
		fg->flags.calibration = true;
	}

	if (fg->high_cal_adj_fct &&
			(capacity_delta < GUARD_BAND_LOW_THRLD) &&
			(fg->cal_high_clr_cnt++ > CAL_CNT_THRESHOLD)) {
		fg->flags.high_bat_cal = false;
		fg->high_cal_adj_fct = 0;
		fg->cal_high_clr_cnt = 0;
		pr_fg(FLOW, "clear high_cal_adj_fct\n");
	} else if ((fg->cal_high_clr_cnt > 0) &&
			(capacity_delta > GUARD_BAND_LOW_THRLD)) {
		fg->cal_high_clr_cnt = 0;
		pr_fg(FLOW, "clear cal_high_clr_cnt\n");
	}

	fg->prev_cap_delta = capacity_delta;
}

static void bcmpmu_fg_get_coulomb_counter(struct bcmpmu_fg_data *fg)
{
	u64 t_now;
	u64 t_ms;
	int ret;
	int sample_count;
	int sleep_count;
	int capacity_delta;
	int capacity_adj;
	int adj_factor;
	int temp_factor;

	u8 accm[4];
	u8 act_cnt[2];
	u8 sleep_cnt[2];

	fg->capacity_info.ocv_cap = bcmpmu_fg_get_load_comp_capacity(fg, true);
	fg->capacity_info.uuc = bcmpmu_fg_get_uuc(fg);
	pr_fg(FLOW, "ocv_cap %d uuc %d\n",
		fg->capacity_info.ocv_cap, fg->capacity_info.uuc);

	/**
	 * Avoid reading accumulator register earlier than
	 * next sample available
	 */

	t_now = kona_hubtimer_get_counter();
	t_ms = ((t_now - fg->last_sample_tm) * 1000)/CLOCK_TICK_RATE;

	if (t_ms < bcmpmu_fg_sample_rate_to_time(fg->sample_rate)) {
		pr_fg(FLOW,
			"CC Read time(%llu) < FG samp time(%d)\n",
			t_ms, bcmpmu_fg_sample_rate_to_time(fg->sample_rate));
		return;
	}

	fg->last_sample_tm = t_now;

	/**
	 * if coulomb couting is disabled, just return
	 */
	if (fg->flags.coulb_dis)
		return;

	adj_factor = bcmpmu_fg_get_adj_factor(fg);

	if (adj_factor != 0)
		pr_fg(FLOW, "cap_adj_factor: %d\n", adj_factor);

	/**
	 * latch accumulator, active counter and sleep counter
	 * registers and then read them
	 */
	ret = bcmpmu_fg_freeze_read(fg);

	if (ret)
		return;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGACCM1, accm, 4);
	if (ret)
		return;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGCNT1, act_cnt, 2);
	if (ret)
		return;
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGSLEEPCNT1,
			sleep_cnt, 2);
	if (ret)
		return;

	pr_fg(VERBOSE, "FG ACCMx : %x %x %x %x", accm[0], accm[1], accm[2],
			accm[3]);
	pr_fg(VERBOSE, "FG CNTx: %x %x SLEEPCNTx: %x %x\n", act_cnt[0],
			act_cnt[1], sleep_cnt[0], sleep_cnt[1]);

	/**
	 * Invalid sample ?
	 * if PMU_FG_FGACCM1[7] = 1 => sample valid
	 */
	if (!(accm[0] & FGACCM1_FGRDVALID_MASK))
		return;

	/**
	 * check for sign bit: PMU_FG_FGACCM1[25] represents
	 * sign bit
	 */
	accm[0] &= FGACCM1_FGACCM_25_24_MASK;
	if (accm[0] >= 0x2)
		accm[0] |= 0xFC; /* Sign bit extention */

	fg->accumulator = ((accm[0] << 24) | (accm[1] << 16) |
			(accm[2] << 8) | (accm[3]));
	sample_count = (((act_cnt[0] & FGCNT1_FGCNT_11_8_MASK) << 8) |
			(act_cnt[1] & FGCNT2_FGCNT_7_0_MASK));
	sleep_count = (((sleep_cnt[0] & FGSLEEPCNT1_FGSLEEPCNT_15_8_MASK) << 8)
			| (sleep_cnt[1] & FGSLEEPCNT2_FGSLEEPCNT_7_0_MASK));

	/**
	 * update the current battery capacity
	 */
	capacity_delta = bcmpmu_fg_accumulator_to_capacity(fg, fg->accumulator,
			sample_count, sleep_count);
#ifdef CONFIG_DEBUG_FS
	fg->probes.capacity_delta = capacity_delta;
#endif

	if (fg->flags.cal_eoc_adj && fg->cal_eoc_adj_fct)
		capacity_delta = capacity_delta - fg->cal_eoc_adj_fct;

	capacity_adj = (capacity_delta -
			(capacity_delta * adj_factor / 100));

#ifdef CONFIG_DEBUG_FS
	fg->probes.temp_factor = temp_factor;
#endif

	fg->capacity_info.capacity += capacity_adj;

	if (fg->flags.fully_charged &&
			fg->flags.chrgr_connected) {
		fg->capacity_info.cap_at_eoc =
			clamp(fg->capacity_info.cap_at_eoc +
				capacity_adj, 0, fg->capacity_info.full_charge);
		pr_fg(FLOW, "cap_at_eoc: %d\n", fg->capacity_info.cap_at_eoc);
	}

	if (fg->capacity_info.capacity > fg->capacity_info.full_charge)
		fg->capacity_info.capacity = fg->capacity_info.full_charge;
	else if (fg->capacity_info.capacity < 0)
		fg->capacity_info.capacity = 0;

	fg->capacity_info.percentage = capacity_to_percentage(fg,
			fg->capacity_info.capacity);
	bcmpmu_fg_update_adj_factor(fg);

	pr_fg(VERBOSE, "accm: %d accm_adj: %d cal_eoc_adj_fct: %d\n",
			capacity_delta,
			capacity_adj,
			fg->cal_eoc_adj_fct);

	pr_fg(FLOW, "cap_mAs: %d percentage: %d\n",
			fg->capacity_info.capacity,
			fg->capacity_info.percentage);
}

int bcmpmu_fg_get_current_capacity(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_fg_data *fg;
	if (!bcmpmu)
		return -EINVAL;

	fg = bcmpmu->fg;

	BUG_ON(!fg);

	pr_fg(FLOW, "%s : capacity %d\n",
			__func__, fg->capacity_info.percentage);

	return fg->capacity_info.percentage;
}
EXPORT_SYMBOL(bcmpmu_fg_get_current_capacity);

static int bcmpmu_fg_save_cap(struct bcmpmu_fg_data *fg, int cap_percentage)
{
	int ret;
	/* if cap_percentage=0, then ignore BUG_ON since we writing 0xff */
	if (cap_percentage != CAPACITY_ZERO_ALIAS) {
		BUG_ON((cap_percentage < CAPACITY_PERCENTAGE_EMPTY) ||
			(cap_percentage > CAPACITY_PERCENTAGE_FULL));
		cap_percentage = clamp(cap_percentage,
				CAPACITY_PERCENTAGE_EMPTY,
				CAPACITY_PERCENTAGE_FULL);
		if (fg->capacity_info.first_boot_init_cap_flat)
			cap_percentage |= CAPACITY_INIT_CAP_FLAT;
	}

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, FG_CAPACITY_SAVE_REG,
			cap_percentage);
	return ret;
}

static int bcmpmu_fg_get_saved_cap(struct bcmpmu_fg_data *fg)
{
	int ret;
	int cap;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, FG_CAPACITY_SAVE_REG, &reg);

	if (!ret) {
		if (reg != CAPACITY_ZERO_ALIAS &&
			reg & CAPACITY_INIT_CAP_FLAT) {
			reg &= ~CAPACITY_INIT_CAP_FLAT;
			pr_fg(INIT, "First boot was is flat OCV\n");
			fg->capacity_info.first_boot_init_cap_flat = true;
		}
		cap = reg;
	} else {
		cap = -1;
	}

	return cap;
}


static void bcmpmu_fg_update_fc_sample(struct bcmpmu_fg_data *fg,
					int cap, u8 index)
{
	u8 last_cap;
	char cap_delta;

	cap_delta = abs(cap - 100);
	if (cap <= 100)
		cap_delta |= FG_SAVE_CAP_DELTA_NEGTV;
	pr_fg(VERBOSE, "%s saved %d delta abs %ld\n",
			__func__, cap_delta,  abs(cap - 100));
	if (fg->pdata->saved_fc_samples > 1) {
		if (index) {
			fg->bcmpmu->write_dev(fg->bcmpmu,
				FG_CAP_FULL_CHARGE_REG1, cap_delta);
		} else {
			/* put sample to 1 to  0 and new in 1
			 * so we will have always old sample in 0
			 * which to be replaced
			 */
			fg->bcmpmu->read_dev(fg->bcmpmu,
					FG_CAP_FULL_CHARGE_REG1, &last_cap);
			fg->bcmpmu->write_dev(fg->bcmpmu,
					FG_CAP_FULL_CHARGE_REG, last_cap);
			fg->bcmpmu->write_dev(fg->bcmpmu,
					FG_CAP_FULL_CHARGE_REG1, cap_delta);
		}
	} else if (!index) {
		/* If we have define no ofsaved sample 1 in design then if
		 * validation fails replace the same
		 */
		fg->bcmpmu->write_dev(fg->bcmpmu,
					FG_CAP_FULL_CHARGE_REG, cap_delta);
	}

}

/* check whether FC sample meeting Max and Min deviation */
static bool bcmpmu_fg_qf_fc_cap(struct bcmpmu_fg_data *fg, int runing_fc_cap)
{
	int max, min, i;
	bool fc_cap = false;
	char cap_delta;

	if (fg->fcd.fc_samples != (fg->pdata->saved_fc_samples))
		return fc_cap;

	max = min = runing_fc_cap;
	for (i = 0; i < fg->fcd.fc_samples; i++) {
		if (max <  (fg->fcd.cap[i] & FG_CAP_MASK))
			max = (fg->fcd.cap[i] & FG_CAP_MASK);
		if (min >  (fg->fcd.cap[i] & FG_CAP_MASK))
			min = (fg->fcd.cap[i] & FG_CAP_MASK);
	}

	pr_fg(VERBOSE, "%s max %d min %d FG_QF_DELTA %d\n",
		__func__, max, min, FG_QF_DELTA);
	fc_cap = max - min <= FG_QF_DELTA ? true : false;
	/* if Qualified QF sample is mean if all samples */
	if (fc_cap) {
		fg->fcd.fg_fc_cap = runing_fc_cap;
		for (i = 0; i < fg->fcd.fc_samples; i++)
			fg->fcd.fg_fc_cap +=  fg->fcd.cap[i] & FG_CAP_MASK;

		fg->fcd.fg_fc_cap = DIV_ROUND_CLOSEST(fg->fcd.fg_fc_cap, ++i);

		cap_delta = abs(fg->fcd.fg_fc_cap - 100);
		if (fg->fcd.fg_fc_cap <= 100)
			cap_delta |= FG_SAVE_CAP_DELTA_NEGTV;
		cap_delta |= FG_FC_SAVE_CAP_MASK;
		pr_fg(FLOW, "%s fg fc_cap %d\n", __func__,  cap_delta);
		/* update FC Sample in reg : mean of latch samples with QF
		 * Flag set */
		fg->bcmpmu->write_dev(fg->bcmpmu,
				FG_CAP_FULL_CHARGE_REG, cap_delta);
	}

	return fc_cap;
}
/* Check whether FC sample still meeting Max deviation criteria */
static bool bcmpmu_fg_validate_fc_cap(struct bcmpmu_fg_data *fg,
					int runing_fc_cap)
{
	int dev, fc_cap, i;
	u8 cnt = 0;

	fc_cap = fg->fcd.cap[0] & FG_CAP_MASK;

	for (i = 1; i < fg->fcd.fc_samples; i++) {
		dev = fc_cap - (fg->fcd.cap[i] & FG_CAP_MASK);
		if (abs(dev) >  FG_FC_MAX_DEV)
			cnt++;
	}

	dev = fc_cap - runing_fc_cap;
	if (abs(dev) >  FG_FC_MAX_DEV)
		cnt++;

	pr_fg(VERBOSE, "%s fg sample %d max dev %d\n"
		, __func__, cnt, dev);
	/* Check for  max FG_CAP max dev allowed */
	return cnt >= fg->fcd.fc_samples ? 0 : 1;

}

/* Count the no. of samples present*/
static void bcmpmu_fg_get_saved_fc_caps(struct bcmpmu_fg_data *fg)
{
	int i, c = 0;
	int ret;
	short cap = 0;
	char saved_cap[FG_FC_MAX_SAMPLES];

	memset(fg->fcd.cap, 0, sizeof(struct fg_fc_cap_d));
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu,
			FG_CAP_FULL_CHARGE_REG, saved_cap,
			fg->pdata->saved_fc_samples);
	if (ret) {
		pr_fg(ERROR, "%s bulk read failed\n", __func__);
		return;
	}
	/* cap is saved in 8 bits bit 7 is for Qualified FC cap
	 * bit 5-0 is delta of cap , cap is 100 + delta
	 * bit 6 is sign of delta. When restoring cap is saved
	 * in 16 bit bit 15 is  for Qualified FC cap and rest bits
	 * is for actual cap
	 */
	for (i = 0; i < fg->pdata->saved_fc_samples; i++) {
		if (saved_cap[i]) {
			cap = saved_cap[i] & FG_SAVE_CAP_DELTA_MASK;
			if (saved_cap[i] & FG_SAVE_CAP_DELTA_NEGTV)
				cap = 100 - cap;
			else
				cap = 100 + cap;
			fg->fcd.cap[i] = cap;
			if (saved_cap[i] & FG_FC_SAVE_CAP_MASK)
				fg->fcd.cap[i] |= FG_FC_CAP_MASK;
			pr_fg(FLOW, "%s fc[%d] fcd_cap %d cap %d\n",
				__func__, i, fg->fcd.cap[i], cap);
			c++;
		}
	}
	fg->fcd.fc_samples = c;

}

/* read latch value and check for FG_FC flasg */
static int bcmpmu_fg_get_fc_cap(struct bcmpmu_fg_data *fg)
{
	int fc_cap = 0;

	bcmpmu_fg_get_saved_fc_caps(fg);
	if (fg->fcd.cap[0] & FG_FC_CAP_MASK)
		fc_cap = fg->fcd.cap[0] & FG_CAP_MASK;
	pr_fg(FLOW, "%s qfc_cap %d\n", __func__, fc_cap);
	return fc_cap;

}

static int bcmpmu_fg_save_full_charge_cap(struct bcmpmu_fg_data *fg, int cap)
{
	int ret = 0;
	bool qf_cap;
	u8 update_index = 0;
	/* old way */
	if (!fg->pdata->full_cap_qf_sample)
		ret = fg->bcmpmu->write_dev(fg->bcmpmu,
				FG_CAP_FULL_CHARGE_REG, cap);
	/* newbie */
	else {
		pr_fg(FLOW, "%s cap %d\n", __func__, cap);
		/* check for latch QF FC CAP */
		if (bcmpmu_fg_get_fc_cap(fg)) {

			if (bcmpmu_fg_validate_fc_cap(fg, cap))
				/* replace FC with Cap */
				update_index = 1;

			bcmpmu_fg_update_fc_sample(fg, cap, update_index);

		} else  {
			/* if No QF FC check for no. of latch samples
			 * If we have max samples check for QF of FC*/
			qf_cap  = bcmpmu_fg_qf_fc_cap(fg, cap);
			/* if we got qf_cap update running cap with
			 * last sample storein reg1 else place last sample
			 * from reg1 to reg0 and running sample to reg1*/
			if (qf_cap)
				update_index = 1;
			bcmpmu_fg_update_fc_sample(fg, cap, update_index);
		}
	}
	return ret;
}

static int bcmpmu_fg_get_saved_cap_full_charge(struct bcmpmu_fg_data *fg)
{
	int ret;
	int cap;
	u8 reg;

	if (!fg->pdata->full_cap_qf_sample) {
		ret = fg->bcmpmu->read_dev(fg->bcmpmu,
			FG_CAP_FULL_CHARGE_REG, &reg);
		if (ret)
			cap = 0;
		else
			cap = reg;
	} else {
		cap = bcmpmu_fg_get_fc_cap(fg);

	}
	return cap;
}

static int bcmpmu_fg_set_sync_mode(struct bcmpmu_fg_data *fg, bool sync)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL1, &reg);
	if (ret)
		return ret;
	if (sync)
		reg |= FGCTRL1_FGSYNCMODE_MASK;
	else
		reg &= FGCTRL1_FGSYNCMODE_MASK;

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL1, reg);

	if (ret)
		return ret;

	/**
	 * turn of FG PLL during sync mode to save power
	 */
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, &reg);
	if (ret)
		return ret;
	reg |= FGOCICCTRL_FGSYNC_PLLOFF_MASK;
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, reg);

	return ret;
}

static int bcmpmu_fg_set_opmod(struct bcmpmu_fg_data *fg, int opvalue)
{
	int ret;
	u8 reg;
	u8 opmode_mask = (FGOPMODCTRL_OPMODCTRL0_MASK |
					FGOPMODCTRL_OPMODCTRL1_MASK |
					FGOPMODCTRL_OPMODCTRL2_MASK |
					FGOPMODCTRL_OPMODCTRL3_MASK);

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGOPMODCTRL, &reg);

	if (ret)
		return ret;
	reg &= ~opmode_mask;
	reg |= opvalue & opmode_mask;

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGOPMODCTRL, reg);

	return ret;
}

static int bcmpmu_fg_set_vfloat_level(struct bcmpmu_fg_data *fg, int vfloat)
{
	return fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL7,
				vfloat);
}

static int bcmpmu_fg_set_vfloat_max_level(struct bcmpmu_fg_data *fg,
		int vfloat_max)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_MBCCTRL6, &reg);
	if (ret)
		return ret;

	if (reg & MBCCTRL6_VFLOATMAX_LOCK_MASK) {
		pr_fg(ERROR, "VFLOATMAX register is locked\n");
		return -ENOMEM;
	}

	reg &= ~(MBCCTRL6_VFLOATMAX_MASK | MBCCTRL6_RESERVED_MASK);
	reg |= ((vfloat_max & MBCCTRL6_VFLOATMAX_MASK) <<
			MBCCTRL6_VFLOATMAX_SHIFT);

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL6, reg);

	return ret;
}

static int bcmpmu_fg_set_eoc_thrd(struct bcmpmu_fg_data *fg, int curr)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FG_EOC_TH, &reg);
	if (ret)
		return ret;
	reg &= (u8)~FG_EOC_TH_7_0_MASK;
	reg |= curr & FG_EOC_TH_7_0_MASK;

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FG_EOC_TH, reg);
	return ret;
}

static int bcmpmu_fg_set_sw_eoc_condition(struct bcmpmu_fg_data *fg,
					  bool sw_eoc)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, &reg);
	if (ret)
		return ret;
	if (sw_eoc)
		reg |= MBCCTRL9_SW_EOC_MASK;
	else
		reg &= ~MBCCTRL9_SW_EOC_MASK;

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, reg);
	/**
	 * Post EOC event to all registered notifiers
	 */
	bcmpmu_call_notifier(fg->bcmpmu, PMU_FG_EVT_EOC, &fg->flags.fg_eoc);

	return ret;
}

static int bcmpmu_fg_set_maintenance_chrgr_mode(struct bcmpmu_fg_data *fg,
		enum maintenance_chrgr_mode mode)
{
	int ret;
	u8 reg1;
	u8 reg2;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, &reg1);
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_MBCCTRL3, &reg2);

	if (ret)
		return ret;

	if (mode == SW_MAINTENANCE_CHARGING) {
		reg1 |= MBCCTRL9_EOC_DET_MODE_MASK;
		reg2 &= ~MBCCTRL3_HW_MC_ST_EN_MASK;
	} else {
		reg1 &= ~MBCCTRL9_EOC_DET_MODE_MASK;
		reg2 |= MBCCTRL3_HW_MC_ST_EN_MASK;
	}

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, reg1);
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL3, reg2);

	return ret;
}
/*
 * called in WQ context (not in IRQ context)
 */
static void  bcmpmu_fg_irq_handler(u32 irq, void *data)
{
	struct bcmpmu_fg_data *fg = data;

	FG_LOCK(fg);

	if ((irq == PMU_IRQ_EOC) && (fg->pdata->hw_maintenance_charging)) {
		pr_fg(FLOW, "PMU_IRQ_EOC\n");
		fg->charging_state = CHARG_STATE_MC;
		fg->flags.fg_eoc = true;
		fg->flags.prev_batt_status = fg->flags.batt_status;
		fg->flags.batt_status = POWER_SUPPLY_STATUS_FULL;
	}

	FG_UNLOCK(fg);

	if ((irq == PMU_IRQ_MBTEMPHIGH) || (irq == PMU_IRQ_MBTEMPLOW)) {
		pr_fg(FLOW, "%s: PMU NTC ISR Triggered,Charging Disabled: %x\n",
			__func__, irq);
		bcmpmu_chrgr_usb_en(fg->bcmpmu, 0);
	}

	if (irq == PMU_IRQ_LOWBAT) {
		pr_fg(FLOW, "%s: Low batt IRQ triggered\n", __func__);
		if (fg->discharge_state == DISCHARG_STATE_HIGH_BATT)
			clear_avg_sample_buff(fg);
		else
			fg->bcmpmu->mask_irq(fg->bcmpmu, PMU_IRQ_LOWBAT);

		queue_work(fg->fg_wq, &fg->low_batt_irq_work);
	}
}


static int display_event_handler(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(nb, display_nb);
	struct fb_event *evt = (struct fb_event *)data;

	switch (event) {
	case FB_EVENT_BLANK:
		if (*(int *)(evt->data) == FB_BLANK_UNBLANK)
			fg->sleep_current_ua[1] =
			fg->pdata->sleep_current_ua[SLEEP_DISPLAY_AMBIENT];
		else
			fg->sleep_current_ua[1] =
				fg->pdata->sleep_current_ua[SLEEP_DEEP];
		break;

	default:
		break;
	}

	return 0;
}

static int bcmpmu_fg_event_handler(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct bcmpmu_fg_data *fg;
	int enable;
	int chrgr_curr;
	bool cancel_n_resch_work = false;
	int poll_time = 0;

	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS:
		fg = to_bcmpmu_fg_data(nb, accy_nb);
		pr_fg(VERBOSE, "PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS\n");
		FG_LOCK(fg);
		if (fg->pdata->hw_maintenance_charging && fg->flags.fg_eoc) {
			pr_fg(FLOW, "maintenance charging: resume charging\n");
			fg->flags.fg_eoc = false;
			fg->eoc_cap_delta = 0;
			fg->charging_state = CHARG_STATE_FC;
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
		}
		FG_UNLOCK(fg);
		break;
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		fg = to_bcmpmu_fg_data(nb, usb_det_nb);
		fg->chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;

		pr_fg(VERBOSE, "PMU_ACCY_EVT_OUT_CHRGR_TYPE\n");
		pr_fg(VERBOSE, "chrgr type = %d\n", fg->chrgr_type);
		FG_LOCK(fg);
		if (fg->chrgr_type == PMU_CHRGR_TYPE_NONE) {
			pr_fg(FLOW, "charger disconnected!!\n");
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;

			if (fg->flags.fully_charged) {
				fg->delta_cap_mas =
					fg->capacity_info.full_charge -
					fg->capacity_info.cap_at_eoc;
				pr_fg(FLOW, "%s: fg->delta_cap_mas : %d\n",
					__func__,
					fg->delta_cap_mas);

				BUG_ON(fg->delta_cap_mas < 0);
				if (fg->delta_cap_mas > 0)
					fg->flags.cal_eoc_adj = true;
			}
			fg->flags.fg_eoc = false;
			fg->capacity_info.cap_at_eoc = 0;
			fg->eoc_cap_delta = 0;
			fg->flags.chrgr_connected = false;
			poll_time = DISCHARGE_ALGO_POLL_TIME_MS;
			cancel_n_resch_work = true;
			if (fg->bcmpmu->flags & BCMPMU_FG_VF_CTRL)
				fg->vf_zone = -1;
		} else if (fg->chrgr_type > PMU_CHRGR_TYPE_NONE &&
				fg->chrgr_type < PMU_CHRGR_TYPE_MAX) {
			pr_fg(FLOW, "charger connected!!\n");
			fg->flags.chrgr_connected = true;
			fg->flags.fully_charged = false;
			fg->flags.cal_eoc_adj = false;
			fg->capacity_info.cap_at_eoc = 0;
			bcmpmu_fg_reset_adj_factors(fg);
			bcmpmu_fg_reset_cutoff_cnts(fg);
			bcmpmu_fg_enable_coulb_counter(fg, true);
			clear_avg_sample_buff(fg);
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
		} else
			BUG_ON(1);

		FG_UNLOCK(fg);
		bcmpmu_fg_update_psy(fg, true);
		break;
	case PMU_CHRGR_EVT_CHRG_STATUS:
		fg = to_bcmpmu_fg_data(nb, chrgr_status_nb);
		enable = *(int *)data;
		pr_fg(VERBOSE, "PMU_CHRGR_EVENT_CHRG_STATUS\n");
		FG_LOCK(fg);
		if (fg->acld_enabled && !enable) {
			pr_fg(VERBOSE, "ACLD temporary disabling charging\n");
		} else if (enable && fg->flags.chrgr_connected) {
			fg->flags.charging_enabled = true;
			fg->flags.fully_charged = false;
			if ((fg->bcmpmu->flags & BCMPMU_SPA_EN)
					&& fg->flags.fg_eoc) {
				fg->flags.eoc_chargr_en = true;
			}
			bcmpmu_fg_reset_cutoff_cnts(fg);
			bcmpmu_fg_reset_adj_factors(fg);
			bcmpmu_fg_enable_coulb_counter(fg, true);
			clear_avg_sample_buff(fg);
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
			pr_fg(FLOW, "charging enabled\n");
			cancel_n_resch_work = true;
		} else if (!fg->flags.fg_eoc) {
			fg->flags.charging_enabled = false;
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
			fg->eoc_cap_delta = 0;
			pr_fg(FLOW, "charging disabled\n");
		}
		FG_UNLOCK(fg);

		break;
	case PMU_ACCY_EVT_OUT_CHRG_CURR:
		fg = to_bcmpmu_fg_data(nb, chrgr_current_nb);
		chrgr_curr = *(int *)data;
		pr_fg(VERBOSE, "PMU_ACCY_EVT_OUT_CHRG_CURR\n");
		FG_LOCK(fg);
		if (fg->flags.chrgr_connected &&
			fg->flags.charging_enabled &&
			chrgr_curr > 0) {
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
			pr_fg(FLOW, "charging current enabled\n");
		}
		FG_UNLOCK(fg);
		break;

	case PMU_ACLD_EVT_ACLD_STATUS:
		fg = to_bcmpmu_fg_data(nb, acld_nb);
		fg->acld_enabled = *(bool *)data;
		break;

	default:
		BUG_ON(1);
	}

	if (cancel_n_resch_work ||
		(fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING &&
			fg->flags.prev_batt_status != fg->flags.batt_status)) {
		cancel_delayed_work_sync(&fg->fg_periodic_work);
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
			msecs_to_jiffies(poll_time));
	}

	return 0;
}

static int bcmpmu_fg_hw_maint_charging_init(struct bcmpmu_fg_data *fg)
{
	int ret;

	ret = fg->bcmpmu->register_irq(fg->bcmpmu, PMU_IRQ_EOC,
			bcmpmu_fg_irq_handler, fg);
	if (ret) {
		pr_fg(ERROR, "Failed to register PMU_IRQ_EOC\n");
		return ret;
	}

	ret = bcmpmu_fg_set_eoc_thrd(fg, fg->eoc_current);
	ret = bcmpmu_fg_set_maintenance_chrgr_mode(fg,
			HW_MAINTENANCE_CHARGING);
	ret = fg->bcmpmu->unmask_irq(fg->bcmpmu, PMU_IRQ_EOC);

	return ret;
}

static int bcmpmu_fg_sw_maint_charging_init(struct bcmpmu_fg_data *fg)
{
	int ret;

	ret = bcmpmu_fg_set_maintenance_chrgr_mode(fg, SW_MAINTENANCE_CHARGING);

	return ret;
}

static void bcmpmu_fg_hw_init(struct bcmpmu_fg_data *fg)
{
	/**
	 * Enable synchronous mode to save battery current
	 * during deep sleep condition
	 */
	bcmpmu_fg_set_opmod(fg, FG_OPMOD_CTRL_SETTING);
	bcmpmu_fg_set_sync_mode(fg, true);
	bcmpmu_fg_enable(fg, true);
	bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
	bcmpmu_fg_reset(fg);
}

static int bcmpmu_fg_get_ocv_capacity(struct bcmpmu_fg_data *fg)
{
	int cap_percentage;

	pr_fg(VERBOSE, "%s\n", __func__);

	cap_percentage = bcmpmu_fg_get_load_comp_capacity(fg, true);

	BUG_ON((cap_percentage > 100) || (cap_percentage < 0));

	return cap_percentage;
}

static int bcmpmu_fg_get_ocv_avg_capacity(struct bcmpmu_fg_data *fg,
		int samples)
{
	int i = 0;
	int cap_percentage;
	int cap_samples[50];

	pr_fg(VERBOSE, "%s\n", __func__);

	do {
		msleep(FG_INIT_CAPACITY_SAMPLE_DELAY);
		cap_samples[i] = bcmpmu_fg_get_load_comp_capacity(fg, true);
		i++;
	} while (i < samples);

	cap_percentage = interquartile_mean(cap_samples, samples);

	BUG_ON((cap_percentage > 100) || (cap_percentage < 0));

	return cap_percentage;
}


static void bcmpmu_fg_update_psy(struct bcmpmu_fg_data *fg,
		bool force_update)
{
	bool update_psy = false;

	if (ntc_disable) {
		if (fg->capacity_info.percentage == 0) {
			fg->capacity_info.percentage = 1;
			pr_fg(FLOW, "change to percentage=1 for test\n");
		}
	}

	if (fg->capacity_info.prev_percentage != fg->capacity_info.percentage) {
		pr_fg(VERBOSE, "Change in capacity.. Update power supply\n");
		fg->capacity_info.prev_percentage =
			fg->capacity_info.percentage;
		update_psy = true;
	}

	if (fg->flags.batt_status != fg->flags.prev_batt_status) {
		fg->flags.prev_batt_status = fg->flags.batt_status;
		update_psy = true;
	}

	if (update_psy || force_update) {
		if (fg->capacity_info.percentage == 0) {
			bcmpmu_fg_save_cap(fg, CAPACITY_ZERO_ALIAS);
		} else {
			/* save cap w.r.t fully_charge*/
			int cap = fg->capacity_info.capacity * 100 +
				fg->capacity_info.full_charge / 2;
			cap = div64_s64(cap, fg->capacity_info.full_charge);
			bcmpmu_fg_save_cap(fg, cap);
		}
		if (fg->bcmpmu->flags & BCMPMU_SPA_EN)
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
					PMU_FG_EVT_CAPACITY,
					fg->capacity_info.percentage);
		else
			power_supply_changed(&fg->psy);
	}
}

static bool bcmpmu_fg_is_cap_in_flat(struct bcmpmu_fg_data *fg, int cap)
{
	bool ret = false;

	if (fg->bdata->batt_prop->enable_flat_ocv_soc &&
		cap <= fg->bdata->batt_prop->flat_ocv_soc_high &&
		cap >= fg->bdata->batt_prop->flat_ocv_soc_low)
		ret = true;

	return ret;
}

static int bcmpmu_fg_get_init_cap(struct bcmpmu_fg_data *fg)
{
	int saved_cap;
	int full_charge_cap;
	int init_cap = 0;
	int cap_percentage;
	bool init_cap_flat;


	saved_cap = bcmpmu_fg_get_saved_cap(fg);
	full_charge_cap = bcmpmu_fg_get_saved_cap_full_charge(fg);

	if (bcmpmu_fg_is_charger_present(fg) &&
			(bcmpmu_fg_get_batt_volt(fg->bcmpmu) >=
			 FG_MIN_DIS_VOLT_FOR_OCV)) {
		pr_fg(FLOW, "Disable charging before Init OCV calc\n");
		bcmpmu_chrgr_usb_en(fg->bcmpmu, 0);
		msleep(FG_INIT_CAP_CHARGING_DELAY);
		/* Use only  one samples,
		 * as the dead battery might cause an issue if discharging
		 */
		init_cap = bcmpmu_fg_get_ocv_capacity(fg);
		pr_fg(FLOW, "Enable charging after Init OCV calc\n");
		bcmpmu_chrgr_usb_en(fg->bcmpmu, 1);
	} else
		init_cap = bcmpmu_fg_get_ocv_avg_capacity(fg,
				FG_INIT_CAPACITY_AVG_SAMPLES);

	fg->flags.init_ocv = true;


	BUG_ON(init_cap > 100);
	BUG_ON(init_cap < 0);

	pr_fg(INIT, "saved capacity: %d open circuit cap: %d, fg_factor: %d\n",
			saved_cap, init_cap, fg->pdata->fg_factor);

	if (!full_charge_cap) {
		fg->capacity_info.full_charge = fg->capacity_info.max_design;
	} else {
		if (fg->pdata->disable_full_charge_learning &&
			full_charge_cap != 100) {
			pr_fg(INIT,
				"Battery learning disabled. Restoring data\n");
			bcmpmu_fg_save_full_charge_cap(fg, 100);
			full_charge_cap = 100;
		}
		fg->capacity_info.full_charge =
			((fg->capacity_info.max_design * full_charge_cap) /
			 100);
	}

	pr_fg(FLOW, "saved full_charge: %d full_charge_cap: %d\n",
			full_charge_cap,
			fg->capacity_info.full_charge);

	init_cap_flat = bcmpmu_fg_is_cap_in_flat(fg, init_cap);

	if (saved_cap > 0 || saved_cap == CAPACITY_ZERO_ALIAS) {
		bool force_saved = false;

		if (saved_cap == CAPACITY_ZERO_ALIAS)
			saved_cap = 0;

		if (bcmpmu_fg_get_curr_inst(fg) > 0 &&
			init_cap > saved_cap) {
			/* Assume charged from boot.
			 * Calculate the amount of average current flow into
			 * the battery since boot based from the newly
			 * estimated capacity compared to the saved one.
			 * If average current is higher than the charge
			 * current set, then the saved capacity is used.
			 */
			int volt = bcmpmu_fg_get_batt_volt(fg->bcmpmu);

			if (volt < fg->vfloat_eoc) {
				/* In CC mode */
				struct timespec ts;
				int iavg;
				int imax = bcmpmu_get_icc_fc(fg->bcmpmu);

				get_monotonic_boottime(&ts);
				iavg = ((init_cap - saved_cap) *
					fg->capacity_info.full_charge) /
					(100 * ts.tv_sec);

				if (iavg > imax) {
					pr_fg(FLOW, "Iavg > Imax (%d > %d)\n",
						iavg, imax);
					force_saved = true;
				}
			}
		}

		if (fg->used_init_cap && !force_saved) {
			cap_percentage = init_cap;
		} else if ((init_cap_flat &&
				!fg->capacity_info.first_boot_init_cap_flat) ||
			(init_cap_flat &&
				fg->capacity_info.first_boot_init_cap_flat &&
				abs(saved_cap - init_cap) <
				fg->pdata->flat_cap_delta_thrld) ||
			force_saved ||
			(abs(saved_cap - init_cap) <
				fg->pdata->cap_delta_thrld)) {
			pr_fg(INIT, "Limiting to saved cap\n");
			cap_percentage = saved_cap;
		} else {
			cap_percentage = init_cap;
			fg->used_init_cap = true;
		}
	} else {
		cap_percentage = init_cap;
		fg->capacity_info.first_boot_init_cap_flat = init_cap_flat;
		pr_fg(INIT, "First boot, no saved capacity\n");
	}

	/**
	 * Very low initial capacity because VBAT is very low. And at this
	 * low voltage, we can not rely on OCV capacity (because of voltage
	 * fluctuation on Battery terminal). So we will hold the capacity to 1%
	 * and wait for discharging algorithm to run which calculates critrical
	 * cutoff capacity based on the terminal voltage
	 */
	if (!cap_percentage) {
		pr_fg(FLOW, "capacity below crit cutoff\n");
		cap_percentage = 1;
	}

	return percentage_to_capacity(fg, cap_percentage);
}

static int bcmpmu_fg_get_vf_zone_idx(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_fg_vf_data *vfd;
	int temp;
	u8 vfd_sz, zone = 0;
	bool allow_new_zone = false;

	vfd = fg->bdata->vfd;
	vfd_sz = fg->bdata->vfd_sz;

	temp = fg->adc_data.temp;
	if (temp < 0 || temp < vfd[0].temp) {
		zone = 0;
	} else if (temp >= vfd[vfd_sz - 1].temp) {
		zone = vfd_sz - 1;
	} else {
		for (zone = 0; zone < vfd_sz; zone++) {
			if (temp >= vfd[zone].temp && temp < vfd[zone + 1].temp)
				break;
		}
	}

	if (vfd[zone].vfloat_lvl > fg->vfloat_lvl) {
		if ((zone > fg->vf_zone &&
			temp >= (vfd[zone].temp + fg->pdata->hysteresis)) ||
			(temp + fg->pdata->hysteresis) <= vfd[fg->vf_zone].temp)
			allow_new_zone = true;
	} else {
		allow_new_zone = true;
	}

	return allow_new_zone ? zone : fg->vf_zone;
}

static void bcmpmu_fg_update_vf_zone(struct bcmpmu_fg_data *fg)
{
	int zone_idx;

	if (fg->flags.batt_status != POWER_SUPPLY_STATUS_CHARGING)
		return;

	zone_idx = bcmpmu_fg_get_vf_zone_idx(fg);
	if (zone_idx != fg->vf_zone) {
		u8 vfloat_lvl = min_t(u8, fg->bdata->vfd[zone_idx].vfloat_lvl,
				fg->bdata->volt_levels->vfloat_lvl);
		u16 vfloat_eoc = min_t(u16, fg->bdata->vfd[zone_idx].vfloat_eoc,
				fg->bdata->volt_levels->high);
		u16 eoc = max_t(u16, fg->bdata->vfd[zone_idx].eoc_curr,
				fg->bdata->eoc_current);

		if (fg->vfloat_lvl != vfloat_lvl ||
				fg->vfloat_eoc != vfloat_eoc ||
				fg->eoc_current != eoc) {
			fg->vfloat_lvl = vfloat_lvl;
			fg->vfloat_eoc = vfloat_eoc;
			fg->eoc_current = eoc;
			fg->eoc_cnt = 0;

			pr_fg(FLOW, "--- Vfloat_lvl 0x%02x, EOC %u mA\n",
					fg->vfloat_lvl, fg->eoc_current);

			bcmpmu_fg_set_vfloat_level(fg, fg->vfloat_lvl);
		}
		fg->vf_zone = zone_idx;
	}
}

static int bcmpmu_fg_sw_maint_charging_algo(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_batt_volt_levels *volt_levels = fg->bdata->volt_levels;
	struct bcmpmu_fg_status_flags *flags = &fg->flags;
	int vfloat_volt;
	int volt_thrld;
	int cap_percentage;
	int volt;
	int curr;
	bool eoc_condition = false;
	if ((fg->bcmpmu->flags & BCMPMU_FG_VF_CTRL) && fg->bdata->vfd_sz)
		bcmpmu_fg_update_vf_zone(fg);
	vfloat_volt = fg->vfloat_eoc;
	volt_thrld = vfloat_volt - volt_levels->vfloat_gap;
	cap_percentage = fg->capacity_info.percentage;
	volt = fg->adc_data.volt;
	curr = fg->adc_data.curr_inst;

	if ((flags->fg_eoc) &&
			(flags->batt_status == POWER_SUPPLY_STATUS_DISCHARGING))
		flags->fg_eoc = false;

	if (flags->fg_eoc) {
		pr_fg(FLOW, "fg_eoc = %d\n", flags->fg_eoc);
		if ((fg->bcmpmu->flags & BCMPMU_SPA_EN) &&
				(flags->eoc_chargr_en)) {
			pr_fg(FLOW, "sw_maint_chrgr: SPA SW EOC cleared\n");
			bcmpmu_fg_set_sw_eoc_condition(fg, false);
			flags->fg_eoc = false;
		} else if (!(fg->bcmpmu->flags & BCMPMU_SPA_EN) &&
				(volt < volt_thrld)) {
			pr_fg(FLOW, "sw_maint_chrgr: SW EOC cleared\n");
			flags->prev_batt_status = flags->batt_status;
			flags->batt_status = POWER_SUPPLY_STATUS_CHARGING;
			flags->fg_eoc = false;
			bcmpmu_fg_set_sw_eoc_condition(fg, false);
			bcmpmu_chrgr_usb_en(fg->bcmpmu, 1);
		}
	} else if ((!flags->fg_eoc) &&
			(!fg->eoc_cap_delta) &&
			(volt >= vfloat_volt) &&
			((curr > 0) &&
			 (curr <= fg->eoc_current)) &&
			(fg->eoc_cnt++ > FG_EOC_CNT_THRLD)) {
		pr_fg(FLOW, "eoc_cnt hit\n");
		fg->eoc_cnt = 0;
		if (cap_percentage != CAPACITY_PERCENTAGE_FULL &&
		    bcmpmu_fg_can_battery_be_full(fg)) {
			fg->eoc_cap_delta =
				CAPACITY_PERCENTAGE_FULL - cap_percentage;
			pr_fg(FLOW, "eoc_cap_delta: %d\n", fg->eoc_cap_delta);
			if (fg->eoc_cap_delta > 1)
				goto exit;
			else
				eoc_condition = true;
		} else
			eoc_condition = true;
	} else if ((!flags->fg_eoc) && fg->eoc_cnt &&
			((volt < vfloat_volt) ||
			 (curr > fg->eoc_current))) {
		pr_fg(FLOW, "clear EOC counter\n");
		fg->eoc_cnt = 0;
	} else if (!flags->fg_eoc && (fg->eoc_cap_delta > 0) &&
			(fg->cap_inc_dec_cnt++ > EOC_CAP_INC_CNT_THRLD)) {
		pr_fg(FLOW, "cap_inc_dec_cnt hit\n");
		fg->cap_inc_dec_cnt = 0;
		cap_percentage += 1;
		fg->capacity_info.percentage = cap_percentage;
		fg->capacity_info.capacity =
			percentage_to_capacity(fg,
					cap_percentage);
		fg->eoc_cap_delta -= 1;
		if ((fg->eoc_cap_delta <= 0) ||
				(cap_percentage == CAPACITY_PERCENTAGE_FULL))
			eoc_condition = true;
	}

	if (eoc_condition) {
		pr_fg(FLOW, "sw_maint_chrgr: SW EOC tripped\n");
		fg->eoc_cap_delta = 0;
		flags->fg_eoc = true;
		flags->prev_batt_status = flags->batt_status;
		if (bcmpmu_fg_can_battery_be_full(fg)) {
			pr_fg(FLOW, "sw_maint_chrgr: Fully Charged\n");
			flags->fully_charged = true;
			fg->capacity_info.cap_at_eoc =
				fg->capacity_info.capacity =
					fg->capacity_info.full_charge;
			flags->batt_status = POWER_SUPPLY_STATUS_FULL;
			/* Any error in the first estimation is eliminated when
			 * battery is fully charged.
			 */
			if (fg->capacity_info.first_boot_init_cap_flat)
				fg->capacity_info.first_boot_init_cap_flat =
					false;
		} else {
			flags->batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		}
		/**
		 * Tell PMU that EOC condition has happened
		 * so that safetly timers can be cleared
		 */
		bcmpmu_fg_set_sw_eoc_condition(fg, true);

		if (fg->bcmpmu->flags & BCMPMU_SPA_EN) {
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
					PMU_CHRGR_EVT_EOC, 0);
			flags->eoc_chargr_en = false;
		} else {
			pr_fg(FLOW, "sw_maint_chrgr: disable charging\n");
			bcmpmu_chrgr_usb_en(fg->bcmpmu, 0);
			bcmpmu_fg_update_psy(fg, false);
		}
	}
exit:
	return 0;
}

static void bcmpmu_fg_cal_force_algo(struct bcmpmu_fg_data *fg)
{
	int cap_percentage;

	cap_percentage = bcmpmu_fg_get_ocv_avg_capacity(fg,
			FG_CAL_CAPACITY_AVG_SAMPLES);

	fg->capacity_info.capacity = percentage_to_capacity(fg, cap_percentage);
	fg->capacity_info.percentage = capacity_to_percentage(fg,
			fg->capacity_info.capacity);

	pr_fg(FLOW, "force_cal_algo: prev capacity: %d new capacity: %d\n",
			fg->capacity_info.prev_percentage,
			fg->capacity_info.percentage);

	/**
	 * clear all other calibration factors
	 */
	bcmpmu_fg_reset_adj_factors(fg);
	bcmpmu_fg_update_psy(fg, true);
}

static void bcmpmu_fg_cal_low_batt_algo(struct bcmpmu_fg_data *fg)
{
	int ocv_cap;
	int cap_delta;
	u64 cap_full_charge = 0;

	pr_fg(FLOW, "%s..\n", __func__);

	ocv_cap = bcmpmu_fg_get_ocv_avg_capacity(fg,
			FG_CAL_CAPACITY_AVG_SAMPLES);
	ocv_cap = bcmpmu_fg_get_usable_cap_from_ocv_cap(ocv_cap,
							fg->capacity_info.uuc);
	cap_delta = ocv_cap - fg->capacity_info.percentage;

	fg->low_cal_adj_fct = cap_delta * 100 / fg->capacity_info.percentage;
	if ((fg->low_cal_adj_fct > 0) &&
			(fg->low_cal_adj_fct > FG_MAX_ADJ_FACTOR))
		fg->low_cal_adj_fct = FG_MAX_ADJ_FACTOR;
	else if ((fg->low_cal_adj_fct < 0) &&
			(fg->low_cal_adj_fct < -FG_MAX_ADJ_FACTOR))
		fg->low_cal_adj_fct = -FG_MAX_ADJ_FACTOR;

	/**
	 * full charge -> full discharge cycle
	 * This is the new full capacity of the battery
	 * which can be lower than design capacity of the battery
	 * due to aging or mutilple charge/discharge
	 * we save this to FG general purpose register. On next
	 * boot this will be new capacity of the battery
	 */

	/* delta will be negative: current capacity should be higher
	 * than OCV capacity
	 */
	/* Also delta should be in the range of -30 to -1 */

	if (!fg->pdata->disable_full_charge_learning &&
			(abs(cap_delta) <= FG_LOW_BAT_CAP_DELTA_LIMIT) &&
			fg->flags.fully_charged) {
		u64 temp_cap_delta;
		int tfact = bcmpmu_fg_get_temp_factor(fg);
		pr_fg(FLOW, "%s: temperature factor = %d\n", __func__, tfact);

		/* cap_full_charge = (((fg->capacity_info.full_charge *
		 *		tfact) / 1000) *
		 *			(100 + cap_delta));
		 */
		cap_full_charge = fg->capacity_info.full_charge * tfact;
		cap_full_charge = div64_u64(cap_full_charge, 1000);
		cap_full_charge = cap_full_charge * (100 + cap_delta);

		pr_fg(FLOW, "%s..cap_fc %llu\n", __func__, cap_full_charge);

		/* cap_full_charge = (cap_full_charge * 1000) / tfact; */
		cap_full_charge = (cap_full_charge * 1000);
		cap_full_charge = div64_u64(cap_full_charge, tfact);

		pr_fg(FLOW, "tfact %d cap_fc %llu\n", tfact, cap_full_charge);
		cap_full_charge = div64_u64(cap_full_charge, 100);


		/* cap_delta = ((100 * cap_full_charge) /
		 *	fg->capacity_info.max_design) - 100;
		 */
		temp_cap_delta = (100 * cap_full_charge);
		temp_cap_delta = div64_u64(temp_cap_delta,
			fg->capacity_info.max_design);
		cap_delta = (int)(temp_cap_delta - 100);

		pr_fg(FLOW, "cap_delta %d cap_fc %llu\n",
				cap_delta, cap_full_charge);
		pr_fg(FLOW, "Before full_charge update, cap_mAs: %d per: %d\n",
				fg->capacity_info.capacity,
				fg->capacity_info.percentage);
		fg->capacity_info.full_charge = cap_full_charge;
		fg->capacity_info.capacity = percentage_to_capacity(fg,
				fg->capacity_info.percentage);
		pr_fg(FLOW, "After full_charge update, cap_mAs: %d per: %d\n",
				fg->capacity_info.capacity,
				fg->capacity_info.percentage);
		bcmpmu_fg_save_full_charge_cap(fg, (100 + cap_delta));
		fg->flags.fully_charged = false;
	} else if (fg->flags.fully_charged) {
		fg->flags.fully_charged = false;
		if (!fg->pdata->disable_full_charge_learning)
			pr_fg(FLOW,
				"Low Battery Calib: cap_delta Limit Exceeds\n");
	}
	pr_fg(FLOW, "cap_delta: %d low_cal_fct: %d full_charge_cap: %d\n",
			cap_delta, fg->low_cal_adj_fct,
			fg->capacity_info.full_charge);
}

static void bcmpmu_fg_cal_high_batt_algo(struct bcmpmu_fg_data *fg)
{
	int ocv_cap;
	int cap_delta;

	pr_fg(FLOW, "%s..\n", __func__);

	ocv_cap = bcmpmu_fg_get_ocv_avg_capacity(fg,
			FG_CAL_CAPACITY_AVG_SAMPLES);
	ocv_cap = bcmpmu_fg_get_usable_cap_from_ocv_cap(ocv_cap,
							fg->capacity_info.uuc);
	cap_delta = ocv_cap - fg->capacity_info.percentage;

	fg->high_cal_adj_fct = cap_delta * 100 / fg->capacity_info.percentage;
	if ((fg->high_cal_adj_fct > 0) &&
			(fg->high_cal_adj_fct > FG_MAX_ADJ_FACTOR))
		fg->high_cal_adj_fct = FG_MAX_ADJ_FACTOR;
	else if ((fg->high_cal_adj_fct < 0) &&
			(fg->high_cal_adj_fct < -FG_MAX_ADJ_FACTOR))
		fg->high_cal_adj_fct = -FG_MAX_ADJ_FACTOR;
	/**
	 * high battery calibration is done becasue capacity diff between
	 * OCV capacity and coloumb capacity is greater than guardband defined
	 * for this temperature zone. In this case we will use high battery
	 * calibration factor to adjust coulomb counter ( clear low battery
	 * calibration factor)
	 */
	fg->low_cal_adj_fct = 0;
	pr_fg(FLOW, "cap delta: %d high cal factor: %d\n", cap_delta,
			fg->high_cal_adj_fct);
}

static void bcmpmu_fg_cal_eoc_adj_batt_algo(struct bcmpmu_fg_data *fg)
{
	pr_fg(FLOW, "%s: fg->cal_eoc_adj_cal_cnt:%d\n",
		__func__, fg->cal_eoc_adj_cal_cnt);

	if (fg->cal_eoc_adj_cal_cnt == FG_CAL_EOC_SPLIT_CNT) {
		fg->flags.cal_eoc_adj = false;
		fg->cal_eoc_adj_cal_cnt = 0;
		fg->cal_eoc_adj_fct = 0;
		fg->delta_cap_mas = 0;
		fg->cal_mode = CAL_MODE_NONE;
	} else {
		fg->cal_eoc_adj_fct = fg->delta_cap_mas/FG_CAL_EOC_SPLIT_CNT;
		fg->cal_eoc_adj_cal_cnt++;
	}
}

static void bcmpmu_fg_batt_cal_algo(struct bcmpmu_fg_data *fg)
{
	switch (fg->cal_mode) {
	case CAL_MODE_FORCE:
		bcmpmu_fg_cal_force_algo(fg);
		break;
	case CAL_MODE_LOW_BATT:
		bcmpmu_fg_cal_low_batt_algo(fg);
		break;
	case CAL_MODE_HI_BATT:
		bcmpmu_fg_cal_high_batt_algo(fg);
		break;
	case CAL_MODE_EOC_ADJ:
		bcmpmu_fg_cal_eoc_adj_batt_algo(fg);
		break;

	default:
		break;
	}
	fg->flags.calibration = false;
	fg->flags.reschedule_work = true;
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
}

static bool bcmpmu_fg_ntc_update(struct bcmpmu_fg_data *fg)
{
	bool update_psy = false;
	struct bcmpmu_fg_pdata *pdata = fg->pdata;

	if (fg->adc_data.temp >= pdata->ntc_high_temp)
			update_psy = true;
	return	update_psy;
}

static void bcmpmu_fg_charging_algo(struct bcmpmu_fg_data *fg)
{
	int poll_time = CHARG_ALGO_POLL_TIME_MS;
	bool psy_update = false;

	pr_fg(FLOW, "%s\n", __func__);


	if (fg->discharge_state != DISCHARG_STATE_HIGH_BATT) {
#ifdef CONFIG_WD_TAPPER
		wd_tapper_update_timeout_req(&fg->wd_tap_node,
				TAPPER_DEFAULT_TIMEOUT);
#endif
		fg->discharge_state = DISCHARG_STATE_HIGH_BATT;
		fg->bcmpmu->unmask_irq(fg->bcmpmu, PMU_IRQ_LOWBAT);
	}

	if (fg->flags.coulb_dis) {
		pr_fg(FLOW, "%s: enable coulomb counter\n",
			__func__);
		bcmpmu_fg_enable_coulb_counter(fg, true);
	}

	bcmpmu_fg_get_coulomb_counter(fg);
	if (!fg->pdata->hw_maintenance_charging)
		bcmpmu_fg_sw_maint_charging_algo(fg);

	/**
	 * always report 100% when EOC is reached and charger
	 * is connected (coloumb capacity will reduce during MC
	 * state as charging is disable (either by SW or HW),
	 * but we will always show 100%
	 */
	if (fg->flags.fg_eoc) {
		if (bcmpmu_fg_can_battery_be_full(fg)) {
			fg->capacity_info.percentage = CAPACITY_PERCENTAGE_FULL;
			fg->capacity_info.capacity =
				fg->capacity_info.full_charge;
		}
	} else if ((fg->capacity_info.prev_percentage ==
				CAPACITY_PERCENTAGE_FULL - 1) &&
			(fg->capacity_info.percentage ==
			 CAPACITY_PERCENTAGE_FULL)) {
		pr_fg(FLOW, "Waiting EOC condition. Hold cap to 99\n");
		/**
		 * Until EOC codition is reached, no matter what coulomb
		 * capacity is, we hold is to 99%
		 */
		fg->capacity_info.percentage = CAPACITY_PERCENTAGE_FULL - 1;
		fg->capacity_info.capacity = percentage_to_capacity(fg,
				fg->capacity_info.percentage);
	}

	psy_update = bcmpmu_fg_ntc_update(fg);
	bcmpmu_fg_update_psy(fg, psy_update);

	FG_LOCK(fg);
	if (fg->flags.reschedule_work)
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(poll_time));
	FG_UNLOCK(fg);
}

static void bcmpmu_fg_discharging_algo(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_batt_volt_levels *volt_levels;
	struct bcmpmu_batt_cap_levels *cap_levels;
	struct bcmpmu_batt_cap_info *cap_info;
	struct batt_cutoff_cap_map *cutoff_lut;
	int volt;
	int volt_avg;
	int cap_per;
	int cap_cutoff;
	int poll_time = DISCHARGE_ALGO_POLL_TIME_MS;
#ifdef CONFIG_WD_TAPPER
	int ret = 0;
#endif
	bool force_update_psy = false;
	bool config_tapper = false;

	pr_fg(FLOW, "discharging_algo: state %s\n",
			discharge_state_dbg[fg->discharge_state]);

	volt_levels = fg->bdata->volt_levels;
	cap_levels = fg->bdata->cap_levels;
	cap_info = &fg->capacity_info;
	cutoff_lut = fg->bdata->batt_prop->cutoff_cap_lut;

	bcmpmu_fg_get_coulomb_counter(fg);

	volt = fg->adc_data.volt;
	cap_per = cap_info->percentage;

	volt_avg = interquartile_mean(fg->avg_sample.volt, AVG_SAMPLES);
#ifdef CONFIG_DEBUG_FS
	fg->probes.volt_avg = volt_avg;
#endif
	pr_fg(FLOW, "vbat_avg: %d\n", volt_avg);

	switch (fg->discharge_state) {
	case DISCHARG_STATE_HIGH_BATT:
		if ((volt_avg <= volt_levels->low) ||
				(cap_per <= cap_levels->low)) {
				fg->discharge_state = DISCHARG_STATE_LOW_BATT;
				fg->bcmpmu->mask_irq(fg->bcmpmu,
						PMU_IRQ_LOWBAT);
				/* fall through */
		} else {
			break;
		}
	case DISCHARG_STATE_LOW_BATT:
		cap_cutoff = bcmpmu_fg_get_cutoff_capacity(fg, volt_avg);
		if ((cap_cutoff >= 0) ||
				(cap_per <= cutoff_lut[0].cap)) {
			/**
			 * disable coulomb counter. Now we will rely on
			 * cutoff voltage table for capacity
			 */
			bcmpmu_fg_enable_coulb_counter(fg, false);
			fg->discharge_state = DISCHARG_STATE_CRIT_BATT;
			poll_time = 0;
			config_tapper = true;
		} else if (volt_avg <= volt_levels->low) {
			int usable_cap =
				bcmpmu_fg_get_usable_cap_from_ocv_cap(
					fg->capacity_info.ocv_cap,
					fg->capacity_info.uuc);
			if (usable_cap < cap_info->percentage) {
				poll_time = fg->pdata->poll_rate_low_batt;
				config_tapper = true;
			}
		}
		break;
	case DISCHARG_STATE_CRIT_BATT:
		poll_time = fg->pdata->poll_rate_crit_batt;
		config_tapper = true;
		cap_cutoff = bcmpmu_fg_get_cutoff_capacity(fg, volt_avg);
		pr_fg(FLOW, "cutoff_cap: %d cutoff_delta: %d cutoff_prev: %d\n",
				fg->crit_cutoff_cap,
				fg->crit_cutoff_delta,
				fg->crit_cutoff_cap_prev);
		pr_fg(FLOW, "cutoff_cap_cnt: %d\n", fg->cutoff_cap_cnt);

		if ((fg->crit_cutoff_delta > 0) &&
				(cap_info->percentage > 0)) {
			cap_info->percentage--;
			cap_info->capacity = percentage_to_capacity(fg,
					cap_info->percentage);
			if (--fg->crit_cutoff_delta == 0)
				fg->crit_cutoff_cap = -1;
			break;
		}
		if ((cap_cutoff >= 0) && (fg->crit_cutoff_cap < 0))
			fg->crit_cutoff_cap = cap_cutoff;

		if ((cap_cutoff >= 0) &&
			(cap_cutoff == fg->crit_cutoff_cap)) {
			if (fg->crit_cutoff_cap != fg->crit_cutoff_cap_prev)
				poll_time = min(500,
						fg->pdata->poll_rate_crit_batt);
			if (++fg->cutoff_cap_cnt > CRIT_CUTOFF_CNT_THRD) {
				if ((fg->crit_cutoff_cap_prev < 0) ||
					(fg->crit_cutoff_cap <
						fg->crit_cutoff_cap_prev)) {
					pr_fg(FLOW,
						"crit_cutoff threshold: %d\n",
						fg->crit_cutoff_cap);
					fg->crit_cutoff_cap_prev =
						fg->crit_cutoff_cap;
					fg->crit_cutoff_delta =
						cap_info->percentage -
						cap_cutoff;
					fg->cutoff_cap_cnt = 0;
					poll_time = 0;
				}
				if (fg->crit_cutoff_delta <= 0) {
					fg->crit_cutoff_delta = 0;
					fg->crit_cutoff_cap = -1;
					fg->cutoff_cap_cnt = 0;
				}
			}
		} else if (((cap_cutoff < 0) &&
					(fg->cutoff_cap_cnt > 0)) ||
				((cap_cutoff != fg->crit_cutoff_cap) &&
				 (fg->cutoff_cap_cnt > 0))) {
			pr_fg(FLOW, "clear cutoff_cap_cnt\n");
			fg->cutoff_cap_cnt = 0;
			fg->crit_cutoff_cap = -1;
			fg->crit_cutoff_delta = 0;
		}
		break;
	default:
		BUG();
	}

	if (fg->capacity_info.first_boot_init_cap_flat) {
		int usable_cap = bcmpmu_fg_get_usable_cap_from_ocv_cap(
			fg->capacity_info.ocv_cap,
			fg->capacity_info.uuc);

		/* Clear first boot flat capacity area when empty battery
		 * or the estimated capacity is within the margin of ocv
		 * based capacity. At these two modes we know that any error
		 * in the first estimation is eliminated.
		 */
		if (!cap_info->percentage ||
			(!bcmpmu_fg_is_cap_in_flat(fg, usable_cap) &&
			!bcmpmu_fg_is_cap_in_flat(fg, cap_info->percentage) &&
			abs(usable_cap - cap_info->percentage) <
				fg->pdata->cap_delta_thrld))
			fg->capacity_info.first_boot_init_cap_flat = false;
	}

#ifdef CONFIG_WD_TAPPER
	/**
	 * reconfigure wakeup time in case of low battery
	 * or critical battery
	 */
	if (config_tapper) {
		pr_fg(VERBOSE, "set tapper timeout to %d\n",
				(poll_time / 1000));
		ret = wd_tapper_update_timeout_req(&fg->wd_tap_node,
				(poll_time / 1000));
		BUG_ON(ret);
	}
#else
	if (config_tapper)
		fg->alarm_timeout = poll_time / 1000;
	else
		fg->alarm_timeout = ALARM_DEFAULT_TIMEOUT;
	pr_fg(VERBOSE, "set poll(wakeup) timeout to %d\n",
				fg->alarm_timeout);

	bcmpmu_fg_program_alarm(fg, fg->alarm_timeout);
#endif /*CONFIG_WD_TAPPER*/

	force_update_psy = bcmpmu_fg_ntc_update(fg);
	bcmpmu_fg_update_psy(fg, force_update_psy);

	FG_LOCK(fg);
	if (fg->flags.reschedule_work)
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(poll_time));
	FG_UNLOCK(fg);
}
static void bcmpmu_fg_periodic_work(struct work_struct *work)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(work,
			fg_periodic_work.work);
	struct bcmpmu_fg_status_flags flags;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int data;
	int ret;

	FG_LOCK(fg);
	flags = fg->flags;
	FG_UNLOCK(fg);

	/**
	 * No battery Case (BSI/BCL pin is not connected)
	 * if battery is not present (booting with power supply
	 * and BCL pin floating, we will not do coloumb counting
	 */
	if (!fg->flags.batt_present && !fg->flags.init_capacity) {
		fg->capacity_info.initial = fg->capacity_info.max_design;
		fg->capacity_info.percentage =
			bcmpmu_fg_get_load_comp_capacity(fg, false);

		pr_fg(ERROR, "no batt; vbat=%d fg_capty=%d\n",
			fg->adc_data.volt, fg->capacity_info.percentage);

		/* compensate the volt line dropping
		   QA camera app still can be run @ low batt  */
		if (fg->capacity_info.percentage < fg->dummy_bat_cap_lmt) {
			pr_fg(FLOW, "actual capty=%d, limit %d\n",
				fg->capacity_info.percentage,
						fg->dummy_bat_cap_lmt);
			fg->capacity_info.percentage = fg->dummy_bat_cap_lmt;
		}

		if (fg->bcmpmu->flags & BCMPMU_SPA_EN) {
			fg->capacity_info.prev_percentage
				= fg->capacity_info.percentage;
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
				PMU_FG_EVT_CAPACITY,
				fg->capacity_info.prev_percentage);
		} else {
			bcmpmu_fg_update_psy(fg, false);
		}
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(FAKE_BATT_POLL_TIME_MS));
#ifndef CONFIG_WD_TAPPER
		if (wake_lock_active(&fg->fg_alarm_wake_lock))
			wake_unlock(&fg->fg_alarm_wake_lock);
#endif
		return;
	}

	if (fg->flags.init_capacity) {
		fg->capacity_info.initial = bcmpmu_fg_get_init_cap(fg);
		fg->capacity_info.capacity = fg->capacity_info.initial;
		fg->capacity_info.percentage = capacity_to_percentage(fg,
				fg->capacity_info.capacity);

		fg->flags.init_capacity = false;
		pr_fg(FLOW, "Initial battery capacity %d capmas %d\n",
				fg->capacity_info.percentage,
				fg->capacity_info.capacity);
		if (fg->bcmpmu->flags & BCMPMU_SPA_EN) {
			fg->capacity_info.prev_percentage
				= fg->capacity_info.percentage;
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
				PMU_FG_EVT_CAPACITY,
				fg->capacity_info.prev_percentage);
		} else {
			/*
			 * fg->psy.external_power_changed(&fg->psy);
			 * */
			bcmpmu_usb_get(fg->bcmpmu,
					BCMPMU_USB_CTRL_GET_CHRGR_TYPE,
					&data);
			chrgr_type = data;
			pr_fg(FLOW, "%s chrgr_type = %d\n",
							__func__, chrgr_type);
			if (chrgr_type > PMU_CHRGR_TYPE_NONE &&
					chrgr_type < PMU_CHRGR_TYPE_MAX) {
				fg->chrgr_type = chrgr_type;
				fg->flags.chrgr_connected = true;
				if (bcmpmu_is_usb_host_enabled(fg->bcmpmu))
					fg->flags.charging_enabled = true;
			}

			fg->flags.prev_batt_status = fg->flags.batt_status;

			if (fg->flags.chrgr_connected &&
				fg->flags.charging_enabled)
				fg->flags.batt_status =
					POWER_SUPPLY_STATUS_CHARGING;
			else
				fg->flags.batt_status =
					POWER_SUPPLY_STATUS_DISCHARGING;
		}
		if (!fg->init_notifier) {
			ret = bcmpmu_fg_register_notifiers(fg);
			WARN_ON(ret);
		}
		bcmpmu_fg_update_psy(fg, true);
		fg->flags.reschedule_work = true;
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
	} else if ((fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING) ||
			(fg->flags.batt_status == POWER_SUPPLY_STATUS_FULL) ||
			(fg->flags.batt_status ==
				POWER_SUPPLY_STATUS_NOT_CHARGING &&
			!bcmpmu_fg_can_battery_be_full(fg)))
		bcmpmu_fg_charging_algo(fg);
	else
		bcmpmu_fg_discharging_algo(fg);

	if (fg->flags.calibration)
		bcmpmu_fg_batt_cal_algo(fg);

	if (fg->sleep_current_ua[0] != fg->sleep_current_ua[1]) {
		fg->sleep_current_ua[0] = fg->sleep_current_ua[1];
		pr_fg(FLOW, "Sleep current changed to %d uA\n",
			fg->sleep_current_ua[0]);
	}

	pr_fg(VERBOSE, "flags: %d %d %d %d %d %d %d %d %d %d %d %d\n",
			flags.batt_status,
			flags.prev_batt_status,
			flags.chrgr_connected,
			flags.charging_enabled,
			flags.init_capacity,
			flags.fg_eoc,
			flags.reschedule_work,
			flags.eoc_chargr_en,
			flags.calibration,
			fg->flags.cv_entered,
			flags.fully_charged,
			flags.cal_eoc_adj);

#ifndef CONFIG_WD_TAPPER
	if (wake_lock_active(&fg->fg_alarm_wake_lock))
		wake_unlock(&fg->fg_alarm_wake_lock);
#endif
}

static void bcmpmu_fg_low_batt_irq_work(struct work_struct *work)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(work,
						low_batt_irq_work);

	cancel_delayed_work_sync(&fg->fg_periodic_work);
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
}

static int bcmpmu_fg_get_capacity_level(struct bcmpmu_fg_data *fg)
{
	int cap_percentage = fg->capacity_info.prev_percentage;
	int level;

	if (cap_percentage <= fg->bdata->cap_levels->critical)
		level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if (cap_percentage <= fg->bdata->cap_levels->low)
		level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (cap_percentage < fg->bdata->cap_levels->normal)
		level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	else if (cap_percentage <= fg->bdata->cap_levels->high)
		level = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else
		level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	return level;
}

static int bcmpmu_fg_get_properties(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(psy, psy);
	int ret = 0;
	struct bcmpmu_fg_status_flags flags = fg->flags;

	switch (psp) {
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		if (fg->capacity_info.percentage == 100 &&
			flags.batt_status == POWER_SUPPLY_STATUS_CHARGING)
			val->intval = POWER_SUPPLY_STATUS_FULL;
		else
			val->intval = flags.batt_status;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (BATTERY_STATUS_UNKNOWN(flags)) {
			if (flags.init_capacity) {
				val->intval = fg->capacity_info.percentage;
				if (!fg->capacity_info.percentage)
					ret = -EBUSY;
			} else {
				val->intval = CAPACITY_PERCENTAGE_FULL;
			}
		} else {
			val->intval = fg->capacity_info.percentage;
		}
		pr_fg(FLOW, "POWER_SUPPLY_PROP_CAPACITY = %d\n",
				val->intval);
		BUG_ON(val->intval < 0);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		else
			val->intval = bcmpmu_fg_get_capacity_level(fg);
		pr_fg(FLOW, "POWER_SUPPLY_PROP_CAPACITY_LEVEL = %d\n",
				val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = 3700;
		else
			val->intval = bcmpmu_fg_get_batt_volt(fg->bcmpmu);
		val->intval = val->intval * 1000;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = fg->adc_data.temp;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
		else
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_FULL_BAT:
		val->intval =  fg->bdata->batt_prop->one_c_rate;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		if (fg->bdata->batt_prop->model)
			val->strval = fg->bdata->batt_prop->model;
		else
			val->strval = "Li-Ion Battery";
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = (fg->capacity_info.full_charge * 10) / 36;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = (fg->capacity_info.max_design * 10) / 36;
		break;
	default:
		BUG_ON(1);
		break;
	}

	return ret;
}
#if 0
static int bcmpmu_fg_power_supply_class_data(struct device *dev, void *data)
{
	struct power_supply *psy, *ext_psy;
	struct bcmpmu_fg_data *fg;
	union power_supply_propval online;
	union power_supply_propval curr;
	int i, ret = 0;
	bool found_supply = false;
	bool is_charger = false;

	psy = data;
	ext_psy = (struct power_supply *)dev_get_drvdata(dev);
	fg = to_bcmpmu_fg_data(psy, psy);

	for (i = 0; i < ext_psy->num_supplicants; i++) {
		if (strcmp(ext_psy->supplied_to[i], psy->name) == 0) {
			found_supply = true;
			pr_fg(VERBOSE, "Found matching power supplier %s\n",
					ext_psy->name);
			break;
		}
	}
	/**
	 * This is not the power supply which is supplying to me
	 * return success here so that power supply class iterator
	 * goes to the next devices and gives a callback again
	 */
	if (!found_supply)
		return 0;

	/**
	 * Found a power supply.
	 * lets find out the props which we are interested in
	 */
	if ((ext_psy->type == POWER_SUPPLY_TYPE_MAINS) ||
			(ext_psy->type == POWER_SUPPLY_TYPE_USB) ||
			(ext_psy->type == POWER_SUPPLY_TYPE_USB_DCP) ||
			(ext_psy->type == POWER_SUPPLY_TYPE_USB_CDP) ||
			(ext_psy->type == POWER_SUPPLY_TYPE_USB_ACA))
		is_charger = true;

	if (!is_charger)
		return 0;

	if (ext_psy->get_property) {
		ret = ext_psy->get_property(ext_psy,
				POWER_SUPPLY_PROP_ONLINE,
				&online);
		ret = ext_psy->get_property(ext_psy,
				POWER_SUPPLY_PROP_CURRENT_NOW,
				&curr);
	}
	if (ret) {
		pr_fg(FLOW, "Failed to get supply property\n");
		return 0;
	}

	if (online.intval) {
		pr_fg(VERBOSE,
			"Power supply %s is online & and charging\n",
			ext_psy->name);
		FG_LOCK(fg);
		fg->flags.prev_batt_status = fg->flags.batt_status;
		fg->flags.batt_status =
			POWER_SUPPLY_STATUS_CHARGING;
		bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
		FG_UNLOCK(fg);
	} else if (!online.intval) {
		pr_fg(VERBOSE, "Power Supply removed\n");
		FG_LOCK(fg);
		fg->flags.prev_batt_status = fg->flags.batt_status;
		fg->flags.batt_status =
			POWER_SUPPLY_STATUS_DISCHARGING;
		fg->flags.fg_eoc = false;
		bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
		FG_UNLOCK(fg);
	}
	return 0;
}

static void bcmpmu_fg_external_power_changed(struct power_supply *psy)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(psy, psy);

	pr_fg(VERBOSE, "%s\n", __func__);

	/**
	 * iterate over list of the power supply devices and find
	 * out which power device is supplying to me
	 */
	class_for_each_device(power_supply_class, NULL, &fg->psy,
			bcmpmu_fg_power_supply_class_data);
}
#endif

static int bcmpmu_fg_register_notifiers(struct bcmpmu_fg_data *fg)
{
	int ret = 0;

	fg->usb_det_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&fg->usb_det_nb);
	if (ret)
		return ret;

	fg->chrgr_status_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
			&fg->chrgr_status_nb);
	if (ret)
		goto unreg_usb_det_nb;

	fg->accy_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS,
			&fg->accy_nb);
	if (ret)
		goto unreg_chrg_status_nb;

	fg->chrgr_current_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRG_CURR,
			&fg->chrgr_current_nb);
	if (ret)
		goto unreg_chrgr_current_nb;

	fg->acld_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACLD_EVT_ACLD_STATUS,
			&fg->acld_nb);
	if (ret)
		goto unreg_acld_nb;

	if (fg->pdata->enable_selective_sleep_current) {
		fg->display_nb.notifier_call = display_event_handler;
		ret = fb_register_client(&fg->display_nb);
		if (ret)
			goto unreg_display_nb;
	}

	fg->init_notifier = 1;

	return 0;

unreg_display_nb:
	bcmpmu_remove_notifier(PMU_ACLD_EVT_ACLD_STATUS,
			&fg->acld_nb);
unreg_acld_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRG_CURR,
			&fg->chrgr_current_nb);
unreg_chrgr_current_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS,
			&fg->accy_nb);
unreg_chrg_status_nb:
	bcmpmu_remove_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
			&fg->chrgr_status_nb);
unreg_usb_det_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&fg->usb_det_nb);
	return ret;
}

static int bcmpmu_fg_set_platform_data(struct bcmpmu_fg_data *fg,
		struct bcmpmu_fg_pdata *pdata)
{
	struct bcmpmu_batt_property *prop;
	struct bcmpmu_battery_data *bdata;
	enum battery_type batt_type;

	if (!pdata || !pdata->batt_data)
		return -EINVAL;

	fg->pdata = pdata;

	batt_type = get_battery_type();
	bdata = &pdata->batt_data[batt_type];

	if (!bdata->batt_prop || !bdata->cap_levels || !bdata->volt_levels)
		return -EINVAL;

	fg->bdata = bdata;
	prop = bdata->batt_prop;

	/**
	 * verify mandatory fields from platform data
	 */
	if ((prop->full_cap == 0) || (!prop->volt_cap_cpt_lut) ||
			(!prop->esr_temp_lut))
		return -EINVAL;
	if (!bdata->eoc_current)
		bdata->eoc_current = FG_EOC_CURRENT;
	fg->eoc_current = bdata->eoc_current;
	if (!pdata->sleep_current_ua[SLEEP_DEEP]) {
		pdata->sleep_current_ua[SLEEP_DEEP] = FG_SLEEP_CURR_UA;
		pdata->enable_selective_sleep_current = false;
	}
	fg->sleep_current_ua[0] = pdata->sleep_current_ua[SLEEP_DEEP];
	fg->sleep_current_ua[1] = fg->sleep_current_ua[0];
	if (!pdata->sleep_sample_rate)
		pdata->sleep_sample_rate = SAMPLE_RATE_2HZ;
	if (!pdata->fg_factor)
		pdata->fg_factor = FG_CURR_SCALING_FACTOR;
	if (prop->min_volt == 0)
		prop->min_volt = BATT_LI_ION_MIN_VOLT;
	if (prop->max_volt == 0)
		prop->max_volt = BATT_LI_ION_MAX_VOLT;

	fg->capacity_info.max_design = bdata->batt_prop->full_cap;

	if (bdata->volt_levels->vfloat_max)
		bcmpmu_fg_set_vfloat_max_level(fg,
				bdata->volt_levels->vfloat_max);
	else
		bcmpmu_fg_set_vfloat_max_level(fg, BATT_VFLOAT_DEFAULT);


	/**
	 * set VFLOAT levels
	 */
	if (!bdata->volt_levels->vfloat_lvl)
		bdata->volt_levels->vfloat_lvl = BATT_VFLOAT_DEFAULT;
	fg->vfloat_lvl = bdata->volt_levels->vfloat_lvl;
	bcmpmu_fg_set_vfloat_level(fg, bdata->volt_levels->vfloat_lvl);

	if (!bdata->volt_levels->high)
		bdata->volt_levels->high = BATT_MIN_EOC_VOLT;
	fg->vfloat_eoc = bdata->volt_levels->high;

	if (!pdata->cap_delta_thrld)
		pdata->cap_delta_thrld = FG_CAP_DELTA_THRLD;
	if (!pdata->flat_cap_delta_thrld)
		pdata->flat_cap_delta_thrld = FG_FLAT_CAP_DELTA_THRLD;
	return 0;
}

__weak enum battery_type get_battery_type(void)
{
	return BATT_0;
}

int bcmpmu_fg_set_sw_eoc_current(struct bcmpmu59xxx *bcmpmu, int eoc_current)
{
	struct bcmpmu_fg_data *fg;
	if ((eoc_current <= 0) || !bcmpmu)
		return -EINVAL;

	fg = bcmpmu->fg;

	BUG_ON(!fg);

	pr_fg(FLOW, "set sw eoc current level to: %d\n", eoc_current);
	FG_LOCK(fg);
	fg->eoc_current = eoc_current;
	FG_UNLOCK(fg);
	return 0;
}
EXPORT_SYMBOL(bcmpmu_fg_set_sw_eoc_current);

int bcmpmu_fg_calibrate_battery(struct bcmpmu59xxx *bcmpmu)
{
	struct bcmpmu_fg_data *fg;
	if (!bcmpmu)
		return -EINVAL;

	fg = bcmpmu->fg;
	if (!fg)
		return -EINVAL;

	pr_fg(FLOW, "%s\n", __func__);

	FG_LOCK(fg);
	if (fg->flags.calibration) {
		pr_fg(ERROR, "calibration is running!!\n");
		FG_UNLOCK(fg);
		return -EAGAIN;
	}

	fg->flags.reschedule_work = false;
	fg->flags.calibration = true;
	fg->cal_mode = CAL_MODE_FORCE;
	FG_UNLOCK(fg);

	cancel_delayed_work_sync(&fg->fg_periodic_work);
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);

	return 0;
}
EXPORT_SYMBOL(bcmpmu_fg_calibrate_battery);

#ifdef CONFIG_DEBUG_FS

int debugfs_fg_flags_init(struct bcmpmu_fg_data *fg, struct dentry *flags_dir)
{
	struct dentry *file;

	file = debugfs_create_u32("batt_status", DEBUG_FS_PERMISSIONS,
			flags_dir, &fg->flags.batt_status);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("fg_eoc", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.fg_eoc);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("calibration", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.calibration);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("low_bat_cal", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.low_bat_cal);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("high_bat_cal", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.high_bat_cal);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("fully_charged", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.fully_charged);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("coulb_dis", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.coulb_dis);
	if (IS_ERR_OR_NULL(file))
		return -1;

	file = debugfs_create_u8("charging_enabled", DEBUG_FS_PERMISSIONS,
			flags_dir, (u8 *)&fg->flags.charging_enabled);
	if (IS_ERR_OR_NULL(file))
		return -1;

	return 0;
}

int debugfs_fg_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

int debugfs_get_curr_read(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct bcmpmu_fg_data *fg = file->private_data;
	int curr;
	int count;
	char buff[50];

	curr = bcmpmu_fg_get_curr_inst(fg);

	count = snprintf(buff, 50, "%s=%d\n", "current", curr);

	return simple_read_from_buffer(buf, len, ppos, buff,
		strlen(buff));
}

static const struct file_operations fg_current_fops = {
	.open = debugfs_fg_open,
	.read = debugfs_get_curr_read,
};

static int debugfs_get_batt_volt(void *data, u64 *volt)
{
	struct bcmpmu_fg_data *fg = data;
	*volt = bcmpmu_fg_get_batt_volt(fg->bcmpmu);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_volt_fops,
	debugfs_get_batt_volt, NULL, "%llu\n");

int debugfs_get_batt_temp(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct bcmpmu_fg_data *fg = file->private_data;
	int temp;
	int count;
	char buff[50];

	temp = bcmpmu_fg_get_batt_temp(fg);

	count = snprintf(buff, 50, "%s=%d\n", "ntc", temp);

	return simple_read_from_buffer(buf, len, ppos, buff,
		strlen(buff));
}

static const struct file_operations fg_temp_fops = {
	.open = debugfs_fg_open,
	.read = debugfs_get_batt_temp,
};

static int debugfs_get_fg_factor(void *data, u64 *factor)
{
	struct bcmpmu_fg_data *fg = data;
	*factor = fg->pdata->fg_factor;
	return 0;
}

static int debugfs_set_fg_factor(void *data, u64 factor)
{
	struct bcmpmu_fg_data *fg = data;
	fg->pdata->fg_factor = factor;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fg_factor_fops,
	debugfs_get_fg_factor, debugfs_set_fg_factor, "%llu\n");

static int debugfs_get_fg_sample_rate(void *data, u64 *rate)
{
	struct bcmpmu_fg_data *fg = data;
	*rate = fg->sample_rate;
	return 0;
}

static int debugfs_set_fg_sample_rate(void *data, u64 rate)
{
	struct bcmpmu_fg_data *fg = data;
	return bcmpmu_fg_set_sample_rate(fg, rate);
}

DEFINE_SIMPLE_ATTRIBUTE(fg_sample_rate_fops,
	debugfs_get_fg_sample_rate, debugfs_set_fg_sample_rate, "%llu\n");

static int debugfs_fg_cal_battery(void *data, u64 cal)
{
	struct bcmpmu_fg_data *fg = data;

	if (cal == 1)
		bcmpmu_fg_calibrate_battery(fg->bcmpmu);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_cal_fops,
		NULL, debugfs_fg_cal_battery, "%llu\n");

static int debugfs_get_capacity(void *data, u64 *capacity)
{
	struct bcmpmu_fg_data *fg = data;
	*capacity = fg->capacity_info.percentage;
	return 0;
}

static int debugfs_set_capacity(void *data, u64 capacity)
{
	struct bcmpmu_fg_data *fg = data;
	FG_LOCK(fg);
	fg->capacity_info.percentage = capacity;
	fg->capacity_info.capacity = percentage_to_capacity(fg,
			(u32)capacity);
	FG_UNLOCK(fg);
	bcmpmu_fg_update_psy(fg, true);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_capacity_fops,
	debugfs_get_capacity, debugfs_set_capacity, "%llu\n");

static int debugfs_get_dummy_cap(void *data, u64 *capacity)
{
	struct bcmpmu_fg_data *fg = data;
	*capacity = fg->dummy_bat_cap_lmt;
	return 0;
}

static int debugfs_set_dummy_cap(void *data, u64 capacity)
{
	struct bcmpmu_fg_data *fg = data;
	fg->dummy_bat_cap_lmt = capacity;
	cancel_delayed_work_sync(&fg->fg_periodic_work);
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
	bcmpmu_fg_update_psy(fg, true);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_dummy_bat_fops,
	debugfs_get_dummy_cap, debugfs_set_dummy_cap, "%llu\n");

static int debugfs_get_adj_factor(struct file *file, char __user *buf,
		size_t len, loff_t *ppos)
{
	struct bcmpmu_fg_data *fg = file->private_data;
	char buff[6];

	snprintf(buff, sizeof(buff), "%d\n", fg->probes.adj_factor);

	return simple_read_from_buffer(buf, len, ppos, buff, strlen(buff));
}
static const struct file_operations fg_adj_factor_fops = {
	.open = debugfs_fg_open,
	.read = debugfs_get_adj_factor,
};

static int debugfs_get_temp_factor(struct file *file, char __user *buf,
		size_t len, loff_t *ppos)
{
	struct bcmpmu_fg_data *fg = file->private_data;
	char buff[6];

	snprintf(buff, sizeof(buff), "%d\n", fg->probes.temp_factor);

	return simple_read_from_buffer(buf, len, ppos, buff, strlen(buff));
}
static const struct file_operations fg_temp_factor_fops = {
	.open = debugfs_fg_open,
	.read = debugfs_get_temp_factor,
};

static int debugfs_get_capacity_delta(struct file *file, char __user *buf,
		size_t len, loff_t *ppos)
{
	struct bcmpmu_fg_data *fg = file->private_data;
	char buff[16];

	snprintf(buff, sizeof(buff), "%d\n", fg->probes.capacity_delta);

	return simple_read_from_buffer(buf, len, ppos, buff, strlen(buff));
}
static const struct file_operations fg_capacity_delta_fops = {
	.open = debugfs_fg_open,
	.read = debugfs_get_capacity_delta,
};

static int debugfs_get_qfc(void *data, u64 *capacity)
{
	struct bcmpmu_fg_data *fg = data;
	*capacity = bcmpmu_fg_get_saved_cap_full_charge(fg);
	return 0;
}

static int debugfs_save_qfc(void *data, u64 capacity)
{
	struct bcmpmu_fg_data *fg = data;
	bcmpmu_fg_save_full_charge_cap(fg, capacity);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_qfc_fops,
	debugfs_get_qfc, debugfs_save_qfc, "%llu\n");

static void bcmpmu_fg_debugfs_init(struct bcmpmu_fg_data *fg)
{
	struct dentry *dentry_fg_dir;
	struct dentry *dentry_fg_flags_dir;
	struct dentry *dentry_fg_file;
	struct bcmpmu59xxx *bcmpmu = fg->bcmpmu;

	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_fg(ERROR, "%s: dentry_bcmpmu is NULL", __func__);
		return;
	}

	dentry_fg_dir = debugfs_create_dir("fuelgauge", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	dentry_fg_flags_dir = debugfs_create_dir("flags", dentry_fg_dir);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	if (debugfs_fg_flags_init(fg, dentry_fg_flags_dir))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("capacity", DEBUG_FS_PERMISSIONS,
				dentry_fg_dir, fg,
				&fg_capacity_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("eoc_curr", DEBUG_FS_PERMISSIONS,
				dentry_fg_dir,
				&fg->eoc_current);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("vfloat", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir,
			&fg->vfloat_lvl);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("cal_mode", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir,
			&fg->cal_mode);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("current", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, fg, &fg_current_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("voltage", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, fg, &fg_volt_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("ntc", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, fg, &fg_temp_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("fg_factor", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, fg, &fg_factor_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("sample_rate",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir, fg,
			&fg_sample_rate_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;
	dentry_fg_file = debugfs_create_file("calibration",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir, fg,
			&fg_cal_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;
	dentry_fg_file = debugfs_create_u32("debug_mask", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, &debug_mask);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;
	dentry_fg_file = debugfs_create_file("dummy_bat_cap_lmt",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir, fg,
				&fg_dummy_bat_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("initial", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, (u32 *)&fg->capacity_info.initial);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("capacity_mas",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir,
			(u32 *)&fg->capacity_info.capacity);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("full_charge", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, (u32 *)&fg->capacity_info.full_charge);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u8("ocv_cap", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, (u8 *)&fg->capacity_info.ocv_cap);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u8("uuc", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, (u8 *)&fg->capacity_info.uuc);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u8("discharge_state",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir,
			(u8 *)&fg->discharge_state);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u16("volt_avg", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, (u16 *)&fg->probes.volt_avg);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("adj_factor", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, fg, &fg_adj_factor_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("temp_factor",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir,
			fg, &fg_temp_factor_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("capacity_delta",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir,
			fg, &fg_capacity_delta_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_file("qfc_cap",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir, fg,
				&fg_qfc_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("cal_eoc_point",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir,
			(u32 *)&fg->cal_eoc_point);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;


	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_fg_dir))
		debugfs_remove_recursive(dentry_fg_dir);
}
#endif

static ssize_t show_fg_factor(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bcmpmu_fg_data *fg =
		container_of(psy, struct bcmpmu_fg_data, psy);

	return scnprintf(buf, PAGE_SIZE, "%d", fg->pdata->fg_factor);
}

static ssize_t store_fg_factor(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct bcmpmu_fg_data *fg =
		container_of(psy, struct bcmpmu_fg_data, psy);
	int factor;
	int ret;

	ret = sscanf(buf, "%4d", &factor);
	if (!ret) {
		pr_fg(ERROR, "Can not calibrate fg_factor\n");
		return -EBADMSG;
	}

	if (factor <= 0) {
		pr_fg(ERROR, "Bad fg_factor value. Defaulting to %d\n",
			FG_CURR_SCALING_FACTOR);
		factor = FG_CURR_SCALING_FACTOR;
	}

	if (fg->pdata->fg_factor != factor) {
		fg->max_discharge_current =
			DIV_ROUND_CLOSEST((fg->max_discharge_current * factor),
					fg->pdata->fg_factor);
		fg->pdata->fg_factor = factor;
		pr_fg(INIT, "fg_factor updated to %d\n", factor);
		if (!fg->flags.init_capacity) {
			fg->flags.init_capacity = true;
			cancel_delayed_work_sync(&fg->fg_periodic_work);
			queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
		}
	}

	return strlen(buf);
}

static struct device_attribute sysfs_attrs[] = {
	__ATTR(fg_factor, S_IRUGO|S_IWUSR, show_fg_factor, store_fg_factor),
};

static int sysfs_create_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(sysfs_attrs); i++)
		if (device_create_file(dev, &sysfs_attrs[i]))
			goto sysfs_create_attrs_failed;

	return 0;

sysfs_create_attrs_failed:
	pr_fg(ERROR, "Failed creating sysfs attrs.\n");
	while (i--)
		(void)device_remove_file(dev, &sysfs_attrs[i]);

	return -EIO;
}

static void sysfs_remove_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(sysfs_attrs); i++)
		(void)device_remove_file(dev, &sysfs_attrs[i]);
}

#if CONFIG_PM
static int bcmpmu_fg_resume(struct platform_device *pdev)
{
	struct bcmpmu_fg_data *fg = platform_get_drvdata(pdev);

	pr_fg(FLOW, "%s\n", __func__);

	FG_LOCK(fg);
	fg->flags.reschedule_work = true;
	FG_UNLOCK(fg);

	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
	return 0;
}

static int bcmpmu_fg_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bcmpmu_fg_data *fg = platform_get_drvdata(pdev);

	pr_fg(FLOW, "%s\n", __func__);

	FG_LOCK(fg);
	fg->flags.reschedule_work = false;
	FG_UNLOCK(fg);

	cancel_delayed_work_sync(&fg->fg_periodic_work);
	return 0;
}
#else
#define bcmpmu_fg_resume	NULL
#define bcmpmu_fg_suspend	NULL
#endif

static int bcmpmu_fg_remove(struct platform_device *pdev)
{
	struct bcmpmu_fg_data *fg = platform_get_drvdata(pdev);

	fg->bcmpmu->unregister_irq(fg->bcmpmu, PMU_IRQ_MBTEMPHIGH);
	fg->bcmpmu->unregister_irq(fg->bcmpmu, PMU_IRQ_MBTEMPLOW);

	if (fg->pdata->enable_selective_sleep_current)
		fb_unregister_client(&fg->display_nb);

	sysfs_remove_attrs(fg->psy.dev);

	/* Disable FG */
	bcmpmu_fg_enable(fg, false);
	if (!(fg->bcmpmu->flags & BCMPMU_SPA_EN))
		power_supply_unregister(&fg->psy);
	kfree(fg);
	return 0;
}

static int bcmpmu_fg_probe(struct platform_device *pdev)
{
	struct bcmpmu_fg_data *fg;
	struct bcmpmu_fg_pdata *pdata;
	struct bcmpmu59xxx *bcmpmu;
	int ret = 0;

	bcmpmu = dev_get_drvdata(pdev->dev.parent);
	pdata = pdev->dev.platform_data;

	pr_fg(INIT, "%s\n", __func__);

	if (!pdata) {
		pr_fg(ERROR, "%s: pdata is NULL\n", __func__);
		return -EINVAL;
	}

	fg = kzalloc(sizeof(struct bcmpmu_fg_data), GFP_KERNEL);
	if (!fg) {
		pr_fg(ERROR, "%s: kzalloc failed!!\n", __func__);
		return -ENOMEM;
	}
	fg->bcmpmu = bcmpmu;
	bcmpmu->fg = fg;

	ret = bcmpmu_fg_set_platform_data(fg, pdata);
	if (ret)
		goto free_dev;
	if (!(bcmpmu->flags & BCMPMU_SPA_EN)) {
		fg->psy.name = "battery";
		fg->psy.type = POWER_SUPPLY_TYPE_BATTERY;
		fg->psy.properties = bcmpmu_fg_props;
		fg->psy.num_properties = ARRAY_SIZE(bcmpmu_fg_props);
		fg->psy.get_property = bcmpmu_fg_get_properties;
		fg->psy.external_power_changed = NULL;
		/*
		fg->psy.external_power_changed =
			bcmpmu_fg_external_power_changed;
		*/
	}

	fg->fg_wq = create_singlethread_workqueue("bcmpmu_fg_wq");
	if (IS_ERR_OR_NULL(fg->fg_wq)) {
		ret = PTR_ERR(fg->fg_wq);
		goto free_dev;
	}

	/**
	 * Dont want to keep CPU busy with this work when CPU is idle
	 */
	INIT_DELAYED_WORK(&fg->fg_periodic_work, bcmpmu_fg_periodic_work);
	INIT_WORK(&fg->low_batt_irq_work, bcmpmu_fg_low_batt_irq_work);

	mutex_init(&fg->mutex);
#ifdef CONFIG_WD_TAPPER
	ret = wd_tapper_add_timeout_req(&fg->wd_tap_node, "fg",
			TAPPER_DEFAULT_TIMEOUT);
	if (ret) {
		pr_fg(ERROR, "failed to register with wd-tapper\n");
		goto destroy_workq;
	}
#else
	alarm_init(&fg->alarm, ALARM_REALTIME, bcmpmu_fg_alarm_callback);
	fg->alarm_timeout = ALARM_DEFAULT_TIMEOUT;
	wake_lock_init(&fg->fg_alarm_wake_lock,
			WAKE_LOCK_SUSPEND, "fg_alarm_wakelock");
#endif /*CONFIG_WD_TAPPER*/

	fg->flags.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	fg->flags.prev_batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	fg->discharge_state = DISCHARG_STATE_HIGH_BATT;
	fg->dummy_bat_cap_lmt = FG_DUMMY_BAT_CAP;

	fg->flags.init_capacity = true;
	fg->crit_cutoff_cap = -1;
	fg->crit_cutoff_cap_prev = -1;
	fg->crit_cutoff_delta = 0;
	fg->delta_volt = 0;
	fg->flags.init_ocv = false;
	fg->ibat_avg = FAKE_IBAT_INTIAL_AVG;
	fg->flags.cv_entered = false;
	fg->cal_eoc_point = FG_CAL_EOC_POINT;

	if (bcmpmu_fg_is_batt_present(fg)) {
		pr_fg(INIT, "main battery present\n");
		fg->flags.batt_present = true;
	} else
		pr_fg(INIT, "booting with power supply\n");

	bcmpmu_fg_set_sample_rate(fg, SAMPLE_RATE_4HZ);
	bcmpmu_fg_hw_init(fg);

	if (!(bcmpmu->flags & BCMPMU_SPA_EN)) {
		ret = power_supply_register(&pdev->dev, &fg->psy);
		if (ret) {
			pr_fg(ERROR, "%s: Failed to register power supply\n",
					__func__);
			goto destroy_workq;
		}
	}
	platform_set_drvdata(pdev, fg);

	if (fg->pdata->hw_maintenance_charging) {
		pr_fg(INIT, "Enable HW maintenace charging\n");
		ret = bcmpmu_fg_hw_maint_charging_init(fg);
	} else {
		pr_fg(INIT, "Using SW maintenance charging mode\n");
		ret = bcmpmu_fg_sw_maint_charging_init(fg);
	}
	clear_avg_sample_buff(fg);

	ret = fg->bcmpmu->register_irq(fg->bcmpmu, PMU_IRQ_MBTEMPHIGH,
			bcmpmu_fg_irq_handler, fg);
	if (ret) {
		pr_fg(ERROR, "Failed to register PMU_IRQ_MBTEMPHIGH\n");
		goto destroy_workq;
	}

	ret = fg->bcmpmu->register_irq(fg->bcmpmu, PMU_IRQ_MBTEMPLOW,
			bcmpmu_fg_irq_handler, fg);
	if (ret) {
		pr_fg(ERROR, "Failed to register PMU_IRQ_MBTEMPLOW\n");
		goto destroy_workq;
	}

	ret = fg->bcmpmu->register_irq(fg->bcmpmu, PMU_IRQ_LOWBAT,
			bcmpmu_fg_irq_handler, fg);
	if (ret) {
		pr_fg(ERROR, "Failed to register PMU_IRQ_LOWBAT\n");
		goto destroy_workq;
	}

	ret = sysfs_create_attrs(fg->psy.dev);
	if (ret) {
		pr_fg(ERROR, "Complete sysfs support failed\n");
		goto destroy_workq;
	}

	ret = fg->bcmpmu->unmask_irq(fg->bcmpmu, PMU_IRQ_MBTEMPHIGH);
	ret = fg->bcmpmu->unmask_irq(fg->bcmpmu, PMU_IRQ_MBTEMPLOW);
	ret = fg->bcmpmu->unmask_irq(fg->bcmpmu, PMU_IRQ_LOWBAT);

	if ((fg->bcmpmu->flags & BCMPMU_FG_VF_CTRL) &&
			fg->bdata->vfd_sz)
		fg->vf_zone = -1;

	if (fg->pdata->full_cap_qf_sample) {
		if (!fg->pdata->saved_fc_samples ||
				fg->pdata->saved_fc_samples >
					FG_FC_MAX_SAMPLES) {
			WARN_ON(1);
			pr_fg(ERROR, "FG Plat data not correct\n");
			fg->pdata->saved_fc_samples = FG_FC_MAX_SAMPLES;
		}
	}

	/**
	 * Run FG algorithm now
	 */
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(FG_SETTLING_TIME));
#ifdef CONFIG_DEBUG_FS
	bcmpmu_fg_debugfs_init(fg);
#endif
	return 0;

destroy_workq:
	destroy_workqueue(fg->fg_wq);
free_dev:
	kfree(fg);
	pr_fg(ERROR, "%s: failed!!\n", __func__);
	return ret;
}

static struct platform_driver bcmpmu_fg_driver = {
	.driver = {
		.name = "bcmpmu_fg",
	},
	.probe = bcmpmu_fg_probe,
	.remove = bcmpmu_fg_remove,
	.suspend = bcmpmu_fg_suspend,
	.resume = bcmpmu_fg_resume,

};

static int __init bcmpmu_fg_init(void)
{
	return platform_driver_register(&bcmpmu_fg_driver);
}
module_init(bcmpmu_fg_init);

static void __exit bcmpmu_fg_exit(void)
{
	platform_driver_unregister(&bcmpmu_fg_driver);
}
module_exit(bcmpmu_fg_exit);

MODULE_DESCRIPTION("Broadcom PMU Fuel Gauge Driver");
MODULE_LICENSE("GPL");
