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
#include <linux/sort.h>
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


#ifndef CONFIG_WD_TAPPER
/* 300 seconds are based on wd_tapper count which is configured in dts file */
#define ALARM_DEFAULT_TIMEOUT		300
#endif

#define FG_CAPACITY_SAVE_REG		(PMU_REG_FGGNRL1)
#define FG_CAP_FULL_CHARGE_REG		(PMU_REG_FGGNRL2)

#define PMU_FG_CURR_SMPL_SIGN_BIT_MASK		0x8000
#define PMU_FG_CURR_SMPL_SIGN_BIT_SHIFT		15

#define FG_CAP_DELTA_THRLD		30
#define ADC_VBAT_AVG_SAMPLES		8
#define ADC_VBUS_AVG_SAMPLES		8
#define FG_INIT_CAPACITY_AVG_SAMPLES	4
#define FG_INIT_CAPACITY_SAMPLE_DELAY	100
#define ACLD_DELAY_500			500
#define ACLD_DELAY_1000			1000
#define ACLD_RETRIES			10
#define ACLD_VBUS_MARGIN		200 /* 200mV */
#define ACLD_CC_LIMIT			1800 /* mA */
#define ACLD_VBUS_ON_LOW_THRLD		4400
#define ADC_READ_TRIES			10
#define ADC_RETRY_DELAY		20 /* 20ms */
#define AVG_SAMPLES			5

#define FG_WORK_POLL_TIME_MS		(5000)
#define ACLD_WORK_POLL_TIME		(500)
#define CHARG_ALGO_POLL_TIME_MS		(5000)
#define DISCHARGE_ALGO_POLL_TIME_MS	(60000)
#define LOW_BATT_POLL_TIME_MS		(5000)
#define CRIT_BATT_POLL_TIME_MS		(2000)
#define FAKE_BATT_POLL_TIME_MS		(60000)

#define INTERPOLATE_LINEAR(X, Xa, Ya, Xb, Yb) \
	(Ya + (((Yb - Ya) * (X - Xa))/(Xb - Xa)))

#define CAPACITY_PERCENTAGE_FULL	100

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
#define FG_MAX_ADJ_FACTOR		50

#define BATT_LI_ION_MIN_VOLT		2100
#define BATT_LI_ION_MAX_VOLT		4200
#define BATT_VFLOAT_DEFAULT		0xC
#define BATT_ONE_C_RATE_DEF		1500
#define CAL_CNT_THRESHOLD		3
#define GUARD_BAND_LOW_THRLD		10
#define CRIT_CUTOFF_CNT_THRD		3
#define LOW_VOLT_CNT_THRD		2

/**
 * FG should go to synchronous mode (modulator turned off)
 * during system deep sleep condition i.e. PC1, PC2 & PC3
 * = 0, 0, 0
 */
#define FG_OPMOD_CTRL_SETTING		0x1

#define ACLD_ERR_CNT_THRLD	3
#define CHRGR_EFFICIENCY 85

#define NTC_ROOM_TEMP			250 /* 25C */
/**
 * Helper macros
 */
#define to_bcmpmu_fg_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_fg_data, mem)

#define percentage_to_capacity(fg, percentage)	\
	((fg->capacity_info.full_charge * percentage) / 100)

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
	CAL_MODE_FORCE,
	CAL_MODE_LOW_BATT,
	CAL_MODE_HI_BATT,
	CAL_MODE_TEMP,
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
	POWER_SUPPLY_PROP_MODEL_NAME,
};

struct bcmpmu_fg_status_flags {
	int batt_status;
	int prev_batt_status;
	bool chrgr_connected;
	bool charging_enabled;
	bool batt_present;
	bool chrgr_pause;
	bool init_capacity;
	bool fg_eoc;
	bool reschedule_work;
	bool eoc_chargr_en;
	bool calibration;
	bool low_bat_cal;
	bool high_bat_cal;
	bool fully_charged;
	bool coulb_dis;
	bool acld_en;
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
	struct delayed_work fg_acld_work;

	/* Notifier blocks */
	struct notifier_block accy_nb;
	struct notifier_block usb_det_nb;
	struct notifier_block chrgr_status_nb;
	struct notifier_block chrgr_current_nb;

	struct bcmpmu_batt_cap_info capacity_info;
	struct bcmpmu_fg_status_flags flags;
	struct batt_adc_data adc_data;
	struct avg_sample_buff avg_sample;
#ifdef CONFIG_WD_TAPPER
	struct wd_tapper_node wd_tap_node;
#else
	struct alarm alarm;
	int alarm_timeout;
#endif /*CONFIG_WD_TAPPER*/
	ktime_t last_sample_tm;
	ktime_t last_curr_sample_tm;

	enum bcmpmu_fg_cal_state cal_state;
	enum bcmpmu_fg_cal_mode cal_mode;
	enum bcmpmu_batt_charging_state charging_state;
	enum bcmpmu_batt_discharging_state discharge_state;
	enum bcmpmu_fg_sample_rate sample_rate;

	enum bcmpmu_chrgr_type_t chrgr_type;
	int eoc_adj_fct;
	int low_cal_adj_fct;
	int high_cal_adj_fct;
	int low_volt_cnt;
	int cutoff_cap_cnt;
	int crit_cutoff_cap_prev;
	int crit_cutoff_cap;
	int crit_cutoff_delta;
	int cal_low_bat_cnt;
	int cal_high_bat_cnt;
	int cal_low_clr_cnt;
	int cal_high_clr_cnt;
	int eoc_cnt;
	int acld_err_cnt;
	int acld_rtry_cnt;
	int accumulator;

	/* for debugging only */
	int lock_cnt;
};

#ifdef CONFIG_DEBUG_FS
#define DEBUG_FS_PERMISSIONS	(S_IRUSR | S_IWUSR)
#endif

static u32 debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW;

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
static int bcmpmu_fg_get_batt_volt(struct bcmpmu_fg_data *fg);
static int bcmpmu_fg_get_curr_inst(struct bcmpmu_fg_data *fg);

/**
 * inline function
 */
static inline int capacity_to_percentage(struct bcmpmu_fg_data *fg, int cap)
{
	u64 capacity;

	capacity = cap * 100 + fg->capacity_info.full_charge / 2;
	capacity = div64_s64(capacity, fg->capacity_info.full_charge);

	return capacity;
}

static int cmp(const void *a, const void *b)
{
	if (*((int *)a) < *((int *)b))
		return -1;
	if (*((int *)a) > *((int *)b))
		return 1;
	return 0;
}

static inline int average(int *data, int samples)
{
	int i;
	int sum = 0;

	for (i = 0; i < samples; i++)
		sum += data[i];

	return sum/i;
}

/**
 * calculates interquartile mean of the integer data set @data
 * @size is the number of samples. It is assumed that
 * @size is divisible by 4 to ease the calculations
 */

static int interquartile_mean(int *data, int num)
{
	int i, j;
	int avg = 0;

	sort(data, num, sizeof(int), cmp, NULL);

	i = num / 4;
	j = num - i;

	for ( ; i < j; i++)
		avg += data[i];

	avg = avg / (j - (num / 4));

	return avg;
}

static inline void fill_avg_sample_buff(struct bcmpmu_fg_data *fg)
{
	int i;
	fg->avg_sample.idx = 0;

	for (i = 0; i < AVG_SAMPLES; i++) {
		fg->avg_sample.volt[i] = bcmpmu_fg_get_batt_volt(fg);
		fg->avg_sample.curr[i] = bcmpmu_fg_get_curr_inst(fg);
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
	/*wanna do something here?*/
	pr_fg(VERBOSE, "FG wakeup!\n");

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
	int ibat_avg;
	int sample_rate;

	sample_rate = bcmpmu_fg_sample_rate_to_actual(fg->sample_rate);

	accm_act = ((accm * fg->pdata->fg_factor) / sample_rate);
	accm_sleep = ((sleep_cnt * fg->pdata->sleep_current_ua) /
			fg->pdata->sleep_sample_rate);

	cap_mas = accm_act - accm_sleep;

	pr_fg(VERBOSE, "accm_act: %d accm_sleep: %d cap_mas: %d\n",
			accm_act, accm_sleep, cap_mas);

	if (sample_cnt || sleep_cnt) {
		ibat_avg = (cap_mas * 1000) /
			((sample_cnt * 1000000 / sample_rate) +
			(sleep_cnt * 1000000 / fg->pdata->sleep_sample_rate));
		pr_fg(FLOW, "AVG Current %d\n", ibat_avg);
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

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, &reg);
	if (ret)
		return ret;
	reg &= ~FGOCICCTRL_FGCOMBRATE_MASK;
	reg |= (rate << FGOCICCTRL_FGCOMBRATE_SHIFT);

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGOCICCTRL, reg);

	pr_fg(INIT, "set CIC sample rate to %d\n", rate);

	/**
	 * since we have changed the CIC filter rate, its
	 * better to calibrate the offset for better accuracy
	 */
	bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
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
	pr_fg(INIT, "Reset Fuel Gauge HW\n");
	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_FGCTRL2, &reg);
	if(!ret) {
		reg |= FGCTRL2_FGRESET_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_FGCTRL2, reg);
	}
	return ret;
}

static int bcmpmu_fg_enable_coulb_counter(struct bcmpmu_fg_data *fg,
		bool enable)
{
	bcmpmu_fg_reset(fg);
	if (enable) {
		bcmpmu_fg_enable(fg, true);
		fg->flags.coulb_dis = false;
	} else {
		bcmpmu_fg_enable(fg, false);
		fg->flags.coulb_dis = true;
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
	struct batt_volt_cap_map *lut;
	u32 lut_sz;
	int cap_percentage = 0;
	int idx;

	lut = fg->pdata->batt_prop->volt_cap_lut;
	lut_sz = fg->pdata->batt_prop->volt_cap_lut_sz;

	for (idx = 0; idx < lut_sz; idx++) {
		if (volt > lut[idx].volt)
			break;
	}

	if (idx > 0) {
		cap_percentage = INTERPOLATE_LINEAR(volt,
				lut[idx].volt,
				lut[idx].cap,
				lut[idx - 1].volt,
				lut[idx - 1].cap);

	}
	else if (idx == 0)
		cap_percentage = 100; /* full capacity */

	return cap_percentage;
}

static int bcmpmu_fg_get_esr_guardband(struct bcmpmu_fg_data *fg)
{
	struct batt_esr_temp_lut *lut = fg->pdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->pdata->batt_prop->esr_temp_lut_sz;
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
	struct batt_cutoff_cap_map *lut = fg->pdata->batt_prop->cutoff_cap_lut;
	int lut_sz = fg->pdata->batt_prop->cutoff_cap_lut_sz;
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
	struct batt_eoc_curr_cap_map *lut = fg->pdata->batt_prop->eoc_cap_lut;
	u32 lut_sz = fg->pdata->batt_prop->eoc_cap_lut_sz;
	int idx;

	if (!lut)
		return -EINVAL;

	for (idx = 0; idx < lut_sz - 1; idx++)
		if ((curr < lut[idx].eoc_curr) &&
				(curr >= lut[idx + 1].eoc_curr))
			break;
	return lut[idx].capacity;
}

#if 0
static int bcmpmu_fg_get_batt_esr(struct bcmpmu_fg_data *fg, int volt, int temp)
{
	struct batt_esr_temp_lut *lut = fg->pdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->pdata->batt_prop->esr_temp_lut_sz;
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
#endif

static int bcmpmu_fg_get_esr_to_ocv(int volt, int curr, int offset,
		int slope)
{
	int ocv = 0;
	s64 temp, temp1;
	temp = 1000 + ((s64)(slope * curr) / 1000);
	temp1 = (s64)(1000 * volt) - (s64)(offset * curr);

	if (temp != 0)
		ocv = div64_s64(temp1, temp);
	return ocv;
}

static int bcmpmu_fg_get_batt_ocv(struct bcmpmu_fg_data *fg, int volt, int curr,
		int temp)
{
	struct batt_esr_temp_lut *lut = fg->pdata->batt_prop->esr_temp_lut;
	int lut_sz = fg->pdata->batt_prop->esr_temp_lut_sz;
	int slope, offset, ocv;
	int idx;
	int min_volt;
	int max_volt;

	if (!lut) {
		pr_fg(ERROR, "ESR<->TEMP table is not defined\n");
		return 0;
	}

	min_volt = fg->pdata->batt_prop->min_volt;
	max_volt = fg->pdata->batt_prop->max_volt;
	ocv = 0;

	/*find esr zone */
	for (idx = 0; idx < lut_sz; idx++) {
		if (temp <= lut[idx].temp)
			break;
	}
	if (idx == lut_sz)
		idx = lut_sz - 1;

	slope = lut[idx].esr_vl_slope;
	offset = lut[idx].esr_vl_offset;
	ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
	if ((ocv > min_volt) && (ocv < lut[idx].esr_vl_lvl))
		goto exit;

	slope = lut[idx].esr_vm_slope;
	offset = lut[idx].esr_vm_offset;
	ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
	if ((ocv > lut[idx].esr_vl_lvl) && (ocv < lut[idx].esr_vm_lvl))
		goto exit;

	slope = lut[idx].esr_vh_slope;
	offset = lut[idx].esr_vh_offset;
	ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
	if ((ocv > lut[idx].esr_vm_lvl) && (ocv < lut[idx].esr_vh_lvl))
		goto exit;

	slope = lut[idx].esr_vf_slope;
	offset = lut[idx].esr_vf_offset;
	ocv = bcmpmu_fg_get_esr_to_ocv(volt, curr, offset, slope);
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
	return ocv;
}

static int bcmpmu_fg_get_batt_volt(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_adc_result result;
	int ret;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(fg->bcmpmu, PMU_ADC_CHANN_VMBATT,
				PMU_ADC_REQ_SAR_MODE, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}

static int bcmpmu_fg_get_avg_volt(struct bcmpmu_fg_data *fg)
{
	int i = 0;
	int volt_sum = 0;
	int volt_samples[ADC_VBAT_AVG_SAMPLES];

	do {
		volt_samples[i] = bcmpmu_fg_get_batt_volt(fg);

		/**
		 * voltage too high?? may be wrong ADC
		 * sample or connected to power supply > 4.2V
		 */
		if (volt_samples[i] > fg->pdata->batt_prop->max_volt) {
			pr_fg(ERROR, "VBAT > MAX_VOLT");
			volt_samples[i] = fg->pdata->batt_prop->max_volt;
		}

		volt_sum += volt_samples[i];
		i++;
		msleep(25);
	} while (i < ADC_VBAT_AVG_SAMPLES);

	return interquartile_mean(volt_samples, ADC_VBAT_AVG_SAMPLES);
}

static inline int bcmpmu_fg_get_vbus(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_adc_result result;
	int ret;
	int retries = ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(fg->bcmpmu, PMU_ADC_CHANN_VBUS,
				PMU_ADC_REQ_SAR_MODE, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}
static int bcmpmu_fg_get_avg_vbus(struct bcmpmu_fg_data *fg)
{
	int i = 0;
	int vbus_samples[ADC_VBUS_AVG_SAMPLES];

	do {
		vbus_samples[i] = bcmpmu_fg_get_vbus(fg);
		i++;
		msleep(25);
	} while (i < ADC_VBUS_AVG_SAMPLES);

	return interquartile_mean(vbus_samples, ADC_VBUS_AVG_SAMPLES);
}
static int bcmpmu_get_inductor_current(struct bcmpmu_fg_data *fg)
{
	int i_ind;
	int vbus;
	int vbat;
	int usb_fc_cc;

	vbus = bcmpmu_fg_get_vbus(fg);
	vbat = bcmpmu_fg_get_batt_volt(fg);
	usb_fc_cc = bcmpmu_get_icc_fc(fg->bcmpmu);

	pr_fg(VERBOSE, "vbus = %d vbat = %d usb_fc_cc = %d\n",
			vbus, vbat, usb_fc_cc);
	i_ind = (usb_fc_cc * vbus * CHRGR_EFFICIENCY) / (vbat * 100);

	return i_ind;
}
static inline int bcmpmu_fg_get_batt_temp(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_adc_result result;
	int ret = 0;
	int retries = ADC_READ_TRIES;

	if (ntc_disable)
		return NTC_ROOM_TEMP;

	while (retries--) {
		ret = bcmpmu_adc_read(fg->bcmpmu, PMU_ADC_CHANN_NTC,
				PMU_ADC_REQ_SAR_MODE, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}

static int bcmpmu_acld_enable(struct bcmpmu_fg_data *fg, bool enable)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_OTG_BOOSTCTRL3, &reg);
	if (!ret) {
		if (enable)
			reg |= ACLD_ENABLE_MASK;
		else
			reg &= ~ACLD_ENABLE_MASK;
		ret = fg->bcmpmu->write_dev(fg->bcmpmu,
					PMU_REG_OTG_BOOSTCTRL3, reg);
	} else
		BUG_ON(1);

	return ret;
}

static bool bcmpmu_get_mbc_cv_status(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_ENV3, &reg);
	if (reg & ENV3_P_MBC_CV)
		return true;

	return false;
}
void bcmpmu_restore_cc_trim_otp(struct bcmpmu_fg_data *fg)
{
	int ret = 0;
	u8 reg;

	reg = fg->pdata->otp_cc_trim;
	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL18, reg);
	if (ret)
		BUG_ON(1);

}
static int bcmpmu_fg_get_curr_inst(struct bcmpmu_fg_data *fg)
{
	int ret = 0;
	u8 reg;
	u8 smpl_cal[2];

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

	return bcmpmu_fgsmpl_to_curr(fg, smpl_cal[1], smpl_cal[0]);
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
		bool load_comp, bool avg)
{
	int vbat;
	int vbat_oc = 0;
	int capacity_percentage = 0;

	fg->adc_data.temp = bcmpmu_fg_get_batt_temp(fg);

	if (avg) {
		vbat = bcmpmu_fg_get_avg_volt(fg);
		fg->adc_data.volt = vbat;
	} else {
		fg->adc_data.volt = bcmpmu_fg_get_batt_volt(fg);
		vbat = fg->adc_data.volt;
	}

	fg->adc_data.curr_inst = bcmpmu_fg_get_curr_inst(fg);

	if (abs(fg->adc_data.curr_inst) > FG_CURR_SAMPLE_MAX)
		fg->adc_data.curr_inst = 0;

	if (load_comp) {
		vbat_oc = bcmpmu_fg_get_batt_ocv(fg, vbat,
				fg->adc_data.curr_inst,
				fg->adc_data.temp);
		capacity_percentage = bcmpmu_fg_volt_to_cap(fg, vbat_oc);
	} else
		capacity_percentage = bcmpmu_fg_volt_to_cap(fg, vbat);

	pr_fg(FLOW, "vbat: %d, vbat_comp: %d ocv_cap: %d\n",
		fg->adc_data.volt, vbat_oc, capacity_percentage);

	update_avg_sample_buff(fg);

	BUG_ON(capacity_percentage > 100);

	return capacity_percentage;
}

static int bcmpmu_fg_get_adj_factor(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_fg_pdata *pdata = fg->pdata;
	struct batt_adc_data *adc = &fg->adc_data;
	struct batt_eoc_curr_cap_map *lut = pdata->batt_prop->eoc_cap_lut;
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
			if (capacity < CAPACITY_PERCENTAGE_FULL)
				fg->eoc_adj_fct = (((capacity_eoc - capacity) *
							100) /
						(capacity - 100));
			pr_fg(FLOW, "EOC capacity: %d factor: %d\n",
					capacity_eoc, fg->eoc_adj_fct);
			adj_factor = fg->eoc_adj_fct;
		}
	} else if (!charging) {
		if (fg->high_cal_adj_fct)
			adj_factor = fg->high_cal_adj_fct;
		else
			adj_factor = fg->low_cal_adj_fct;
	}

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
	fg->cal_high_clr_cnt = 0;
	fg->cal_low_clr_cnt = 0;
}

static void bcmpmu_fg_reset_cutoff_cnts(struct bcmpmu_fg_data *fg)
{
	fg->crit_cutoff_cap = -1;
	fg->crit_cutoff_cap_prev = fg->crit_cutoff_cap;
	fg->crit_cutoff_delta = 0;
	fg->cutoff_cap_cnt = 0;
}

static void bcmpmu_fg_update_adj_factor(struct bcmpmu_fg_data *fg)
{
	int capacity_delta;
	int volt_oc;
	int cal_volt_low;
	int cal_cap_low;
	int guardband;
	bool charging;

	charging = ((fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING) ?
			true : false);
	if (charging) {
		/**
		 * These factors are cleared in the bcmpmu_fg_event_handler when
		 * BCMPMU_CHRGR_EVENT_CHGR_DETECTION event is received but
		 * there is a possiblity that when
		 * BCMPMU_CHRGR_EVENT_CHGR_DETECTION event is recieved,
		 * calibration algo is already running and high/low calibration
		 * factor is updated by the calibration algo after being cleared
		 * from event_handler.
		 */
		if (fg->high_cal_adj_fct || fg->low_cal_adj_fct) {
			fg->high_cal_adj_fct = 0;
			fg->low_cal_adj_fct = 0;
			fg->cal_high_clr_cnt = 0;
			fg->cal_low_clr_cnt = 0;
		}
		return;
	}

	capacity_delta = abs(fg->capacity_info.percentage -
			fg->capacity_info.ocv_cap);
	volt_oc = bcmpmu_fg_get_batt_ocv(fg, fg->adc_data.volt,
			fg->adc_data.curr_inst, fg->adc_data.temp);

	cal_volt_low = fg->pdata->cal_data->volt_low;
	cal_cap_low = fg->pdata->cal_data->cap_low;
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
		(volt_oc <= cal_volt_low) &&
		(fg->capacity_info.percentage > cal_cap_low) &&
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
	} else if ((fg->cal_low_bat_cnt > 0) && (volt_oc > cal_volt_low)) {
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
			(capacity_delta >= 1)) {
		fg->cal_low_clr_cnt = 0;
		pr_fg(FLOW, "cal_low_clr_cnt cleared\n");
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
}


static void bcmpmu_fg_get_coulomb_counter(struct bcmpmu_fg_data *fg)
{
	ktime_t t_now;
	ktime_t t_diff;
	u64 t_ms;
	int ret;
	int sample_count;
	int sleep_count;
	int capacity_delta;
	int capacity_adj;
	int adj_factor;
	u8 accm[4];
	u8 act_cnt[2];
	u8 sleep_cnt[2];

	fg->capacity_info.ocv_cap = bcmpmu_fg_get_load_comp_capacity(fg, true,
			false);

	/**
	 * Avoid reading accumulator register earlier than
	 * next sample available
	 */
	t_now = ktime_get();
	t_diff = ktime_sub(t_now, fg->last_sample_tm);
	t_ms = ktime_to_ms(t_diff);

	if (t_ms < bcmpmu_fg_sample_rate_to_time(fg->sample_rate))
		return;

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

	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGACCM1, accm, 4);
	ret = fg->bcmpmu->read_dev_bulk(fg->bcmpmu, PMU_REG_FGCNT1, act_cnt, 2);
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
	capacity_adj = (capacity_delta -
			(capacity_delta * adj_factor / 100));

	fg->capacity_info.capacity += capacity_adj;

	if (fg->capacity_info.capacity > fg->capacity_info.full_charge)
		fg->capacity_info.capacity = fg->capacity_info.full_charge;
	else if (fg->capacity_info.capacity < 0)
		fg->capacity_info.capacity = 0;

	fg->capacity_info.percentage = capacity_to_percentage(fg,
			fg->capacity_info.capacity);
	bcmpmu_fg_update_adj_factor(fg);

	pr_fg(VERBOSE, "accm: %d accm_adj: %d\n",
			capacity_delta,
			capacity_adj);

	pr_fg(FLOW, "cap_mAs: %d percentage: %d\n",
			fg->capacity_info.capacity,
			fg->capacity_info.percentage);
}

static int bcmpmu_fg_save_cap(struct bcmpmu_fg_data *fg, int cap_percentage)
{
	int ret;

	BUG_ON((cap_percentage < 0) || (cap_percentage > 100));

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

	if (!ret)
		cap = reg;
	else
		cap = -1;
	return cap;
}

static int bcmpmu_fg_save_full_charge_cap(struct bcmpmu_fg_data *fg, int cap)
{
	BUG_ON(cap >= 100);
	return fg->bcmpmu->write_dev(fg->bcmpmu, FG_CAP_FULL_CHARGE_REG,
			cap);
}

static int bcmpmu_fg_get_saved_cap_full_charge(struct bcmpmu_fg_data *fg)
{
	int ret;
	int cap;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, FG_CAP_FULL_CHARGE_REG, &reg);

	if (ret)
		cap = 0;
	else
		cap = reg;
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

#if 0
static int bcmpmu_fg_set_sw_eoc_condition(struct bcmpmu_fg_data *fg)
{
	int ret;
	u8 reg;

	ret = fg->bcmpmu->read_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, &reg);
	if (ret)
		return ret;
	reg |= MBCCTRL9_SW_EOC_MASK;

	ret = fg->bcmpmu->write_dev(fg->bcmpmu, PMU_REG_MBCCTRL9, reg);
	return ret;
}
#endif

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
}

static int bcmpmu_fg_event_handler(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct bcmpmu_fg_data *fg;
	int enable;
	int chrgr_curr;
	bool cancel_n_resch_work = false;

	switch (event) {
	case BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS:
		fg = to_bcmpmu_fg_data(nb, accy_nb);
		pr_fg(VERBOSE, "BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS\n");
		FG_LOCK(fg);
		if (fg->pdata->hw_maintenance_charging && fg->flags.fg_eoc) {
			pr_fg(FLOW, "maintenance charging: resume charging\n");
			fg->flags.fg_eoc = false;
			fg->charging_state = CHARG_STATE_FC;
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
		}
		FG_UNLOCK(fg);
		break;
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		fg = to_bcmpmu_fg_data(nb, usb_det_nb);
		fg->chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;

		pr_fg(VERBOSE, "BCMPMU_CHRGR_EVENT_CHGR_DETECTION\n");
		pr_fg(VERBOSE, "chrgr type = %d\n", fg->chrgr_type);
		FG_LOCK(fg);
		if (fg->chrgr_type == PMU_CHRGR_TYPE_NONE) {
			pr_fg(FLOW, "charger disconnected!!\n");
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
			fg->flags.fg_eoc = false;
			fg->flags.chrgr_connected = false;
			bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
			if (fg->bcmpmu->flags & BCMPMU_ACLD_EN) {
				cancel_delayed_work_sync(&fg->fg_acld_work);
				bcmpmu_acld_enable(fg, false);
				bcmpmu_restore_cc_trim_otp(fg);

			}
		} else if (fg->chrgr_type > PMU_CHRGR_TYPE_NONE &&
				fg->chrgr_type < PMU_CHRGR_TYPE_MAX) {
			pr_fg(FLOW, "charger connected!!\n");
			fg->flags.chrgr_connected = true;
			bcmpmu_fg_reset_adj_factors(fg);
			bcmpmu_fg_reset_cutoff_cnts(fg);
			bcmpmu_fg_enable_coulb_counter(fg, true);
			clear_avg_sample_buff(fg);
			bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
			if ((fg->bcmpmu->flags & BCMPMU_ACLD_EN) &&
					(fg->chrgr_type == PMU_CHRGR_TYPE_DCP))
				queue_delayed_work(fg->fg_wq, &fg->fg_acld_work,
					msecs_to_jiffies(ACLD_WORK_POLL_TIME));
		} else
			BUG_ON(1);

		FG_UNLOCK(fg);
		bcmpmu_fg_update_psy(fg, true);
		break;
	case BCMPMU_CHRGR_EVENT_CHRG_STATUS:
		fg = to_bcmpmu_fg_data(nb, chrgr_status_nb);
		enable = *(int *)data;
		pr_fg(VERBOSE, "BCMPMU_CHRGR_EVENT_CHRG_STATUS\n");
		FG_LOCK(fg);
		if (enable && fg->flags.chrgr_connected) {
			fg->flags.charging_enabled = true;
			if ((fg->bcmpmu->flags & BCMPMU_SPA_EN)
					&& fg->flags.fg_eoc) {
				fg->flags.eoc_chargr_en = true;
			}
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_CHARGING;
			pr_fg(FLOW, "charging enabled\n");
			cancel_n_resch_work = true;
		} else if (!fg->flags.fg_eoc) {
			fg->flags.charging_enabled = false;
			fg->flags.prev_batt_status = fg->flags.batt_status;
			fg->flags.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
			pr_fg(FLOW, "charging disabled\n");
		}
		FG_UNLOCK(fg);
		if (cancel_n_resch_work) {
			cancel_delayed_work_sync(&fg->fg_periodic_work);
			queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
		}
		break;
	case BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT:
		fg = to_bcmpmu_fg_data(nb, chrgr_current_nb);
		chrgr_curr = *(int *)data;
		pr_fg(VERBOSE, "BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT\n");
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

	default:
		BUG_ON(1);
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

	ret = bcmpmu_fg_set_eoc_thrd(fg, fg->pdata->eoc_current);
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
	bcmpmu_fg_reset(fg);
	bcmpmu_fg_set_sample_rate(fg, SAMPLE_RATE_8HZ);
	/**
	 * Enable synchronous mode to save battery current
	 * during deep sleep condition
	 */
	bcmpmu_fg_set_opmod(fg, FG_OPMOD_CTRL_SETTING);
	bcmpmu_fg_set_sync_mode(fg, true);
	bcmpmu_fg_enable(fg, true);
	bcmpmu_fg_calibrate_offset(fg, FG_HW_CAL_MODE_FAST);
}

static int bcmpmu_fg_get_ocv_avg_capacity(struct bcmpmu_fg_data *fg)
{
	int i = 0;
	int cap_percentage;
	int cap_samples[FG_INIT_CAPACITY_AVG_SAMPLES];

	pr_fg(VERBOSE, "%s\n", __func__);
	bcmpmu_fg_enable(fg, false);
	bcmpmu_fg_set_sample_rate(fg, SAMPLE_RATE_8HZ);
	bcmpmu_fg_enable(fg, true);
	bcmpmu_fg_reset(fg);

	do {
		msleep(FG_INIT_CAPACITY_SAMPLE_DELAY);
		cap_samples[i] = bcmpmu_fg_get_load_comp_capacity(fg, true,
				true);
		i++;
	} while (i < FG_INIT_CAPACITY_AVG_SAMPLES);

	cap_percentage = interquartile_mean(cap_samples,
			FG_INIT_CAPACITY_AVG_SAMPLES);
	BUG_ON((cap_percentage > 100) || (cap_percentage < 0));

	bcmpmu_fg_enable(fg, false);
	bcmpmu_fg_set_sample_rate(fg, fg->sample_rate);
	bcmpmu_fg_enable(fg, true);
	bcmpmu_fg_reset(fg);

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
		bcmpmu_fg_save_cap(fg, fg->capacity_info.percentage);
		update_psy = true;
	}

	if (fg->flags.batt_status != fg->flags.prev_batt_status) {
		fg->flags.prev_batt_status = fg->flags.batt_status;
		update_psy = true;
	}

	if (update_psy || force_update) {
		if (fg->bcmpmu->flags & BCMPMU_SPA_EN)
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
					BCMPMU_CHRGR_EVENT_CAPACITY,
					fg->capacity_info.percentage);
		else
			power_supply_changed(&fg->psy);
	}
}


static int bcmpmu_fg_get_init_cap(struct bcmpmu_fg_data *fg)
{
	int saved_cap;
	int full_charge_cap;
	int init_cap = 0;
	int cap_percentage;
	int i = 0;
	int cutoff_cap;
	int cap_samples[FG_INIT_CAPACITY_AVG_SAMPLES];

	saved_cap = bcmpmu_fg_get_saved_cap(fg);
	full_charge_cap = bcmpmu_fg_get_saved_cap_full_charge(fg);

	do {
		msleep(FG_INIT_CAPACITY_SAMPLE_DELAY);
		cap_samples[i] = bcmpmu_fg_get_load_comp_capacity(fg, true,
				true);
		i++;
	} while (i < FG_INIT_CAPACITY_AVG_SAMPLES);

	init_cap = interquartile_mean(cap_samples,
			FG_INIT_CAPACITY_AVG_SAMPLES);
	BUG_ON(init_cap > 100);
	BUG_ON(init_cap < 0);

	pr_fg(INIT, "saved capacity: %d open circuit cap: %d\n",
			saved_cap, init_cap);
	pr_fg(VERBOSE, "full charge capacity: %d\n", full_charge_cap);

	if (!full_charge_cap)
		fg->capacity_info.full_charge = fg->capacity_info.max_design;
	else
		fg->capacity_info.full_charge = full_charge_cap;

	if (saved_cap > 0) {
		if (abs(saved_cap - init_cap) < FG_CAP_DELTA_THRLD)
			cap_percentage = saved_cap;
		else
			cap_percentage = init_cap;
	} else
		cap_percentage = init_cap;

	/**
	 * Very low initial capacity because VBAT is very low. And at this
	 * low voltage, we can not rely on OCV capacity (because of voltage
	 * fluctuation on Battery terminal). So we will hold the capacity to 1%
	 * and wait for discharging algorithm to run which calculates critrical
	 * cutoff capacity based on the terminal voltage
	 */
	cutoff_cap = fg->pdata->batt_prop->cutoff_cap_lut[0].cap;

	if (cap_percentage <= cutoff_cap) {
		pr_fg(FLOW, "capacity below crit cutoff\n");
		cap_percentage = cutoff_cap;
	}

	return percentage_to_capacity(fg, cap_percentage);
}

static int bcmpmu_fg_sw_maint_charging_algo(struct bcmpmu_fg_data *fg)
{
	struct bcmpmu_batt_volt_levels *volt_levels = fg->pdata->volt_levels;
	struct bcmpmu_fg_status_flags *flags = &fg->flags;
	int vfloat_volt;
	int volt_thrld;
	int cap_percentage;
	int volt;
	int curr;

	vfloat_volt = fg->pdata->volt_levels->high;
	volt_thrld = vfloat_volt - volt_levels->vfloat_gap;
	cap_percentage = fg->capacity_info.percentage;
	volt = fg->adc_data.volt;
	curr = fg->adc_data.curr_inst;

	if ((flags->fg_eoc) &&
			(flags->batt_status == POWER_SUPPLY_STATUS_DISCHARGING))
		flags->fg_eoc = false;

	if (flags->fg_eoc) {
		if ((fg->bcmpmu->flags & BCMPMU_SPA_EN) &&
				(flags->eoc_chargr_en)) {
			pr_fg(FLOW, "sw_maint_chrgr: SPA SW EOC cleared\n");
			flags->fg_eoc = false;
		} else if (!(fg->bcmpmu->flags & BCMPMU_SPA_EN) &&
				(volt < volt_thrld)) {
			pr_fg(FLOW, "sw_maint_chrgr: SW EOC cleared\n");
			flags->prev_batt_status = flags->batt_status;
			flags->batt_status = POWER_SUPPLY_STATUS_CHARGING;
			flags->fg_eoc = false;
			bcmpmu_chrgr_usb_en(fg->bcmpmu, 1);
		}
	} else if ((!flags->fg_eoc) &&
			(volt >= vfloat_volt) &&
			((curr > 0) &&
			 (curr <= fg->pdata->eoc_current)) &&
			(fg->eoc_cnt++ > FG_EOC_CNT_THRLD)) {
		if (cap_percentage != CAPACITY_PERCENTAGE_FULL) {
			cap_percentage += 1;
			fg->capacity_info.percentage = cap_percentage;
			fg->capacity_info.capacity =
				percentage_to_capacity(fg, cap_percentage);
			goto exit;
		}
		pr_fg(FLOW, "sw_maint_chrgr: SW EOC tripped\n");
		flags->fg_eoc = true;
		fg->eoc_cnt = 0;
		flags->fully_charged = true;
		flags->prev_batt_status = flags->batt_status;
		flags->batt_status = POWER_SUPPLY_STATUS_FULL;
		if (fg->bcmpmu->flags & BCMPMU_SPA_EN) {
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
					BCMPMU_CHRGR_EVENT_EOC, 0);
			flags->eoc_chargr_en = false;
		} else {
			pr_fg(FLOW, "sw_maint_chrgr: disable charging\n");
			bcmpmu_chrgr_usb_en(fg->bcmpmu, 0);
			bcmpmu_fg_update_psy(fg, false);
		}
	} else if ((!flags->fg_eoc) && fg->eoc_cnt &&
			((volt < vfloat_volt) ||
				(curr > fg->pdata->eoc_current))) {
		pr_fg(FLOW, "clear EOC counter\n");
		fg->eoc_cnt = 0;
	}
exit:
	return 0;
}

static void bcmpmu_fg_cal_force_algo(struct bcmpmu_fg_data *fg)
{
	int cap_percentage;

	cap_percentage = bcmpmu_fg_get_ocv_avg_capacity(fg);

	fg->capacity_info.capacity = percentage_to_capacity(fg, cap_percentage);
	fg->capacity_info.prev_percentage = capacity_to_percentage(fg,
			fg->capacity_info.capacity);
	fg->capacity_info.percentage =
		fg->capacity_info.prev_percentage;
	bcmpmu_fg_save_cap(fg, fg->capacity_info.prev_percentage);

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

	ocv_cap = bcmpmu_fg_get_ocv_avg_capacity(fg);
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
	if (fg->flags.fully_charged) {
		cap_full_charge = (fg->capacity_info.max_design *
				(100 + cap_delta));
		cap_full_charge = div_s64(cap_full_charge, 100);
		fg->capacity_info.full_charge = cap_full_charge;
		bcmpmu_fg_save_full_charge_cap(fg, (100 + cap_delta));
		fg->flags.fully_charged = false;
	}

	pr_fg(FLOW, "cap_delta: %d low_cal_fct: %d full_charge_cap: %llu\n",
			cap_delta, fg->low_cal_adj_fct, cap_full_charge);
}

static void bcmpmu_fg_cal_high_batt_algo(struct bcmpmu_fg_data *fg)
{
	int ocv_cap;
	int cap_delta;

	pr_fg(FLOW, "%s..\n", __func__);

	ocv_cap = bcmpmu_fg_get_ocv_avg_capacity(fg);
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
	default:
		break;
	}
	fg->flags.calibration = false;
	fg->flags.reschedule_work = true;
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
}

static void bcmpmu_check_battery_current_limit(struct bcmpmu_fg_data *fg)
{
	ktime_t t_now;
	ktime_t t_diff;
	u64 t_ms;
	int i_inst;/* Instantaneous current */
	int i_ind; /* inductor current */

	if (fg->chrgr_type != PMU_CHRGR_TYPE_DCP)
		return;
	/**
	 * Avoid reading accumulator register earlier than
	 * next sample available
	 */
	t_now = ktime_get();
	t_diff = ktime_sub(t_now, fg->last_curr_sample_tm);
	t_ms = ktime_to_ms(t_diff);

	if (t_ms < bcmpmu_fg_sample_rate_to_time(fg->sample_rate))
		return;

	fg->last_curr_sample_tm = t_now;


	i_inst = bcmpmu_fg_get_curr_inst(fg);
	i_ind = bcmpmu_get_inductor_current(fg);

	pr_fg(VERBOSE, "i_inst = %d, i_ind = %d\n", i_inst, i_ind);

	/* check if the battery current is above 1C limit or
	 * inductor current  is above iSat
	 * */
	if (((i_inst > fg->pdata->batt_prop->one_c_rate) ||
			(i_ind > fg->pdata->i_sat)) &&
			(fg->acld_err_cnt++ > ACLD_ERR_CNT_THRLD)) {
		bcmpmu_acld_enable(fg, false);
		fg->flags.acld_en = false;
		bcmpmu_set_icc_fc(fg->bcmpmu, fg->pdata->i_def_dcp);
		pr_fg(ERROR, "ACLD disabled: i_inst = %d, i_ind = %d\n",
				i_inst, i_ind);

	} else if (((i_inst < fg->pdata->batt_prop->one_c_rate) ||
			(i_ind < fg->pdata->i_sat)) &&
			(fg->flags.acld_en) &&
			(fg->acld_err_cnt > 0)) {
		fg->acld_err_cnt = 0;
		pr_fg(ERROR, "ACLD Error Count restarted\n");
	}
}

static void bcmpmu_fg_charging_algo(struct bcmpmu_fg_data *fg)
{
	int poll_time = CHARG_ALGO_POLL_TIME_MS;

	pr_fg(VERBOSE, "%s\n", __func__);

	if ((fg->bcmpmu->flags & BCMPMU_ACLD_EN) &&
			(fg->chrgr_type == PMU_CHRGR_TYPE_DCP))
		bcmpmu_check_battery_current_limit(fg);

	if (fg->discharge_state != DISCHARG_STATE_HIGH_BATT) {
#ifdef CONFIG_WD_TAPPER
		wd_tapper_update_timeout_req(&fg->wd_tap_node,
				TAPPER_DEFAULT_TIMEOUT);
#endif
		fg->discharge_state = DISCHARG_STATE_HIGH_BATT;
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
		fg->capacity_info.capacity = fg->capacity_info.full_charge;
		fg->capacity_info.percentage = CAPACITY_PERCENTAGE_FULL;
		fg->capacity_info.prev_percentage = CAPACITY_PERCENTAGE_FULL;
	}

	bcmpmu_fg_update_psy(fg, false);

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

	volt_levels = fg->pdata->volt_levels;
	cap_levels = fg->pdata->cap_levels;
	cap_info = &fg->capacity_info;
	cutoff_lut = fg->pdata->batt_prop->cutoff_cap_lut;

	bcmpmu_fg_get_coulomb_counter(fg);

	volt = fg->adc_data.volt;
	cap_per = cap_info->percentage;

	volt_avg = interquartile_mean(fg->avg_sample.volt, AVG_SAMPLES);
	pr_fg(FLOW, "vbat_avg: %d\n", volt_avg);

	switch (fg->discharge_state) {
	case DISCHARG_STATE_HIGH_BATT:
		if ((volt_avg <= volt_levels->low) ||
				(cap_per <= cap_levels->low))
				fg->discharge_state = DISCHARG_STATE_LOW_BATT;
		else
			break;
	case DISCHARG_STATE_LOW_BATT:
		cap_cutoff = bcmpmu_fg_get_cutoff_capacity(fg, volt_avg);
		if (cap_cutoff >= 0) {
			/**
			 * disable coulomb counter. Now we will rely on
			 * cutoff voltage table for capacity
			 */
			bcmpmu_fg_enable_coulb_counter(fg, false);
			fg->discharge_state = DISCHARG_STATE_CRIT_BATT;
			poll_time = fg->pdata->poll_rate_crit_batt;
			config_tapper = true;
		} else if (volt_avg <= volt_levels->low) {
			poll_time = fg->pdata->poll_rate_low_batt;
			config_tapper = true;
		}
		break;
	case DISCHARG_STATE_CRIT_BATT:
		cap_cutoff = bcmpmu_fg_get_cutoff_capacity(fg, volt_avg);
		pr_fg(FLOW, "cutoff_cap: %d cutoff_delta: %d cutoff_prev: %d\n",
				fg->crit_cutoff_cap,
				fg->crit_cutoff_delta,
				fg->crit_cutoff_cap_prev);
		pr_fg(FLOW, "cutoff_cap_cnt: %d\n", fg->cutoff_cap_cnt);

		if ((fg->crit_cutoff_delta > 0) &&
				(cap_info->percentage > 0)) {
			cap_info->prev_percentage =
				cap_info->percentage--;
			cap_info->capacity = percentage_to_capacity(fg,
					cap_info->percentage);
			if (--fg->crit_cutoff_delta == 0)
				fg->crit_cutoff_cap = -1;
			poll_time = fg->pdata->poll_rate_crit_batt;
			config_tapper = true;
			break;
		}
		if ((cap_cutoff >= 0) && (fg->crit_cutoff_cap < 0))
			fg->crit_cutoff_cap = cap_cutoff;

		if ((cap_cutoff >= 0) &&
				(cap_cutoff == fg->crit_cutoff_cap) &&
				(fg->cutoff_cap_cnt++ >
				 CRIT_CUTOFF_CNT_THRD)) {
			if ((fg->crit_cutoff_cap_prev < 0) ||
					(fg->crit_cutoff_cap <
					 fg->crit_cutoff_cap_prev)) {
				pr_fg(FLOW, "crit_cutoff threshold: %d\n",
						fg->crit_cutoff_cap);
				fg->crit_cutoff_cap_prev = fg->crit_cutoff_cap;
				fg->crit_cutoff_delta =
					cap_info->percentage - cap_cutoff;
				fg->cutoff_cap_cnt = 0;
			}
			if (fg->crit_cutoff_delta <= 0) {
				fg->crit_cutoff_delta = 0;
				fg->crit_cutoff_cap = -1;
				fg->cutoff_cap_cnt = 0;
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
		poll_time = fg->pdata->poll_rate_crit_batt;
		config_tapper = true;
		break;
	default:
		BUG();
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

	bcmpmu_fg_update_psy(fg, force_update_psy);

	FG_LOCK(fg);
	if (fg->flags.reschedule_work)
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(poll_time));
	FG_UNLOCK(fg);
}

static int bcmpmu_get_vbus_resistance(struct bcmpmu_fg_data *fg,
					int vbus_chrg_off, int vbus_chrg_on)
{
	int vbus_res;
	int icc_fc = bcmpmu_get_icc_fc(fg->bcmpmu);

	pr_fg(INIT, "%s: vbus_chrgr_off: %d vbus_chrgr_on: %d icc_fc = %d\n",
			__func__,
			vbus_chrg_off,
			vbus_chrg_on,
			icc_fc);

	vbus_res = ((vbus_chrg_off - vbus_chrg_on) * 1000) / icc_fc;

	if (vbus_res < 0) {
		pr_fg(INIT, "vbus_off = %d vbus_chrg_on = %d vbus_res = %d\n",
				vbus_chrg_off, vbus_chrg_on, vbus_res);
		vbus_res = 0;
		WARN_ON(1);
	}

	return vbus_res;
}

static int bcmpmu_get_vbus_load(struct bcmpmu_fg_data *fg, int vbus_chrg_off,
		int vbus_res)
{
	int icc_fc = bcmpmu_get_icc_fc(fg->bcmpmu);
	int vbus_load;

	pr_fg(INIT, "icc_fc = %d\n", icc_fc);

	vbus_load = (vbus_chrg_off - ((icc_fc * vbus_res) / 1000))
						- fg->pdata->acld_vbus_margin;
	if (vbus_load < 0) {
		pr_fg(INIT, "vbus_off = %d vbus_res = %d\n",
				vbus_chrg_off, vbus_res);
		vbus_load = vbus_chrg_off - fg->pdata->acld_vbus_margin;
		WARN_ON(1);
	}

	return vbus_load;
}

static bool bcmpmu_get_ubpd_int(struct bcmpmu_fg_data *fg)
{
	int ret;

	bcmpmu_usb_get(fg->bcmpmu, BCMPMU_USB_CTRL_GET_UBPD_INT, &ret);
	if (ret != 1)
		return false;

	return true;

}
static bool bcmpmu_is_usb_valid(struct bcmpmu_fg_data *fg)
{
	int ret;

	bcmpmu_usb_get(fg->bcmpmu, BCMPMU_USB_CTRL_GET_USB_VALID, &ret);
	if (ret != 1)
		return false;

	return true;

}
static void bcmpmu_fg_acld_algo(struct work_struct *work)
{

	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(work, fg_acld_work.work);
	struct bcmpmu_fg_status_flags *flags = &fg->flags;
	bool cc_lmt_hit = false;
	bool chrgr_fault = false;
	bool mbc_in_cv = false;
	int vbus_chrg_off;
	int vbus_chrg_on;
	int vbus_res;
	int vbus_load;
	int ret;
	int usb_fc_cc_reached;
	int usb_fc_cc_prev;
	int usb_fc_cc_next;

	pr_fg(INIT, "%s\n", __func__);


	ret = bcmpmu_chrgr_usb_en(fg->bcmpmu, 0);
	if (ret) {
		pr_fg(ERROR, "Rescheduling ACLD\n");
		if (fg->acld_rtry_cnt++ > ACLD_RETRIES)
			BUG_ON(1);
		goto exit;
	}
	fg->acld_rtry_cnt = 0;

	/* Return from ACLD if,
	 * DCP is quickly removed after insertion.
	 * */
	if (!bcmpmu_get_ubpd_int(fg)) {
		chrgr_fault = true;
		goto chrgr_pre_chk;
	}

	msleep(ACLD_DELAY_1000);

	if (!bcmpmu_get_ubpd_int(fg)) {
		chrgr_fault = true;
		goto chrgr_pre_chk;
	}

	vbus_chrg_off = bcmpmu_fg_get_avg_vbus(fg);
	pr_fg(INIT, "vbus_chrg_off = %d\n", vbus_chrg_off);

	bcmpmu_acld_enable(fg, true);
	/* set USB_FC_CC at OTP and USB_CC_TRIM at 0*/
	bcmpmu_set_cc_trim(fg->bcmpmu, PMU_USB_CC_ZERO_TRIM);
	bcmpmu_set_icc_fc(fg->bcmpmu, PMU_USB_FC_CC_OTP);
	bcmpmu_chrgr_usb_en(fg->bcmpmu, 1);

	msleep(ACLD_DELAY_500);

	if (bcmpmu_get_mbc_cv_status(fg)) {
		pr_fg(INIT, "MBC in CV. Regulate charging on Output\n");
		bcmpmu_acld_enable(fg, false);
		bcmpmu_restore_cc_trim_otp(fg);
		bcmpmu_set_icc_fc(fg->bcmpmu, fg->pdata->i_def_dcp);
		return;
	}

	vbus_chrg_on = bcmpmu_fg_get_avg_vbus(fg);
	vbus_res = bcmpmu_get_vbus_resistance(fg, vbus_chrg_off, vbus_chrg_on);
	pr_fg(INIT, "vbus_res(uohm) = %d\n", vbus_res);
	vbus_load = bcmpmu_get_vbus_load(fg, vbus_chrg_off, vbus_res);

	pr_fg(INIT, "Tuning USB_FC_CC\n");
	do {
		pr_fg(INIT, "vbus_chrg_on = %d vbus_res = %d vbus_load = %d\n",
				vbus_chrg_on, vbus_res, vbus_load);

		/* Check for Charger Errors and battery CV mode */
		if (!bcmpmu_get_ubpd_int(fg)) {
			chrgr_fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(fg)) {
			mbc_in_cv = true;
			break;
		}

		usb_fc_cc_prev = bcmpmu_get_icc_fc(fg->bcmpmu);

		if ((vbus_chrg_on < vbus_load) ||
				(vbus_chrg_on <= ACLD_VBUS_ON_LOW_THRLD)) {
			if (vbus_chrg_on < vbus_load)
				pr_fg(INIT, "vbus_chrg_on < vbus_load\n");
			else
				pr_fg(INIT, "VBUS Low Threshold hit\n");
			break;
		} else {
			/* Increase CC  */
			pr_fg(INIT, "Icreasing CC by one step\n");
			ret = bcmpmu_icc_fc_step_up(fg->bcmpmu);
			if (ret) {
				pr_fg(INIT, "Reached Maximum CC\n");
				break;
			}

			if (bcmpmu_get_icc_fc(fg->bcmpmu) >
					fg->pdata->acld_cc_lmt) {
				pr_fg(INIT, "Recommended DCP CC lmt hit\n");
				cc_lmt_hit = true;
				break;
			}

		}
		msleep(ACLD_DELAY_500);

		/*If the current is limited at input and if ACLD tries to
		 * draw more, USB_FC_CC is automatically decreased by PMU
		 * though we increase USB_FC_CC
		 * */
		usb_fc_cc_next = bcmpmu_get_icc_fc(fg->bcmpmu);
		if (usb_fc_cc_next <= usb_fc_cc_prev) {
			pr_fg(INIT, "PMU CC lmt hit: prev = %d next = %d\n",
					usb_fc_cc_prev, usb_fc_cc_next);
			break;
		}

		vbus_chrg_on = bcmpmu_fg_get_avg_vbus(fg);
		vbus_load = bcmpmu_get_vbus_load(fg, vbus_chrg_off,
				vbus_res);

	} while (true);

	bcmpmu_icc_fc_step_down(fg->bcmpmu);
	usb_fc_cc_reached = bcmpmu_get_icc_fc(fg->bcmpmu);

	if (mbc_in_cv) {
		pr_fg(INIT, "MBC in CV Mode\n");
		goto chrgr_pre_chk;
	}
	if (cc_lmt_hit)
		goto chrgr_pre_chk;

	pr_fg(INIT, "Tuning USB_CC_TRIM\n");
	bcmpmu_cc_trim_up(fg->bcmpmu);
	msleep(ACLD_DELAY_500);
	vbus_chrg_on = bcmpmu_fg_get_avg_vbus(fg);
	vbus_load = bcmpmu_get_vbus_load(fg, vbus_chrg_off, vbus_res);

	do {
		pr_fg(INIT, "vbus_chrg_on = %d vbus_res = %d vbus_load = %d\n",
				vbus_chrg_on, vbus_res, vbus_load);

		if (!bcmpmu_get_ubpd_int(fg)) {
			chrgr_fault = true;
			goto chrgr_pre_chk;
		}
		if (bcmpmu_get_mbc_cv_status(fg)) {
			pr_fg(INIT, "Exiting Trim Tuning, MBC in CV\n");
			break;
		}

		if ((vbus_chrg_on < vbus_load) ||
				(vbus_chrg_on < ACLD_VBUS_ON_LOW_THRLD)) {
			if (vbus_chrg_on < vbus_load)
				pr_fg(INIT, "vbus_chrg_on < vbus_load\n");
			else
				pr_fg(INIT, "VBUS Low Threshold hit\n");
			break;
		} else {
			/* Increase Trim code */
			pr_fg(INIT, "Increasing Trim code by one step\n");
			ret = bcmpmu_cc_trim_up(fg->bcmpmu);
			if (ret) {
				pr_fg(INIT, "Reached Maximum Trim code\n");
				break;
			}
		}
		msleep(ACLD_DELAY_500);
		vbus_chrg_on = bcmpmu_fg_get_avg_vbus(fg);
		vbus_load = bcmpmu_get_vbus_load(fg, vbus_chrg_off,
				vbus_res);
	} while (true);

	bcmpmu_cc_trim_down(fg->bcmpmu);
	bcmpmu_cc_trim_down(fg->bcmpmu);
	bcmpmu_set_icc_fc(fg->bcmpmu, PMU_USB_FC_CC_OTP);
	msleep(ACLD_DELAY_500);
	bcmpmu_set_icc_fc(fg->bcmpmu, usb_fc_cc_reached);
chrgr_pre_chk:
	/* charger presence check */
	if ((!bcmpmu_is_usb_valid(fg)) || chrgr_fault) {
		pr_fg(INIT, "Charger Error, restoring cc trim\n");
		bcmpmu_restore_cc_trim_otp(fg);
		return;
	}
	flags->acld_en = true;
	return;
exit:
	queue_delayed_work(fg->fg_wq, &fg->fg_acld_work,
			msecs_to_jiffies(ACLD_WORK_POLL_TIME));
	return;

}
static void bcmpmu_fg_periodic_work(struct work_struct *work)
{
	struct bcmpmu_fg_data *fg = to_bcmpmu_fg_data(work,
			fg_periodic_work.work);
	struct bcmpmu_fg_status_flags flags;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int data;

	FG_LOCK(fg);
	flags = fg->flags;
	FG_UNLOCK(fg);

	/**
	 * No battery Case (BSI/BCL pin is not connected)
	 * if battery is not present (booting with power supply
	 * and BCL pin floating, we will not do coloumb counting
	 */
	if (!fg->flags.batt_present && !fg->flags.init_capacity) {
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work,
				msecs_to_jiffies(FAKE_BATT_POLL_TIME_MS));
		return;
	}

	if (fg->flags.init_capacity) {
		fg->capacity_info.initial = bcmpmu_fg_get_init_cap(fg);
		/**
		 * During initial capacity calculation, sample rate is set
		 * to 8HZ for faster capacity calculation.
		 * Now we set to default sampling rate (2 HZ)
		 */
		bcmpmu_fg_enable(fg, false);
		bcmpmu_fg_set_sample_rate(fg, fg->sample_rate);
		bcmpmu_fg_enable(fg, true);

		fg->capacity_info.capacity = fg->capacity_info.initial;
		fg->capacity_info.prev_percentage = capacity_to_percentage(fg,
				fg->capacity_info.capacity);
		fg->capacity_info.percentage =
			fg->capacity_info.prev_percentage;
		bcmpmu_fg_save_cap(fg, fg->capacity_info.prev_percentage);
		fg->flags.init_capacity = false;
		pr_fg(FLOW, "Initial battery capacity %d\n",
				fg->capacity_info.percentage);
		if (fg->bcmpmu->flags & BCMPMU_SPA_EN)
			bcmpmu_post_spa_event_to_queue(fg->bcmpmu,
				BCMPMU_CHRGR_EVENT_CAPACITY,
				fg->capacity_info.prev_percentage);
		else {
			/*
			 * fg->psy.external_power_changed(&fg->psy);
			 * */
			bcmpmu_usb_get(fg->bcmpmu,
					BCMPMU_USB_CTRL_GET_CHRGR_TYPE,
					&data);
			chrgr_type = data;
			pr_fg(FLOW, "%s chrgr_type = %d\n",
							__func__, chrgr_type);
			if ((chrgr_type > PMU_CHRGR_TYPE_NONE &&
				chrgr_type < PMU_CHRGR_TYPE_MAX) &&
				!bcmpmu_is_usb_host_enabled(fg->bcmpmu) &&
				bcmpmu_get_icc_fc(fg->bcmpmu)) {

				fg->flags.prev_batt_status =
							fg->flags.batt_status;
				fg->flags.batt_status =
						POWER_SUPPLY_STATUS_CHARGING;
			} else {
				fg->flags.prev_batt_status =
							fg->flags.batt_status;
				fg->flags.batt_status =
						POWER_SUPPLY_STATUS_DISCHARGING;
			}

		}
		bcmpmu_fg_update_psy(fg, true);
		fg->flags.reschedule_work = true;
		queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);
	} else if ((fg->flags.batt_status == POWER_SUPPLY_STATUS_CHARGING) ||
			(fg->flags.batt_status == POWER_SUPPLY_STATUS_FULL))
		bcmpmu_fg_charging_algo(fg);
	else
		bcmpmu_fg_discharging_algo(fg);

	if (fg->flags.calibration)
		bcmpmu_fg_batt_cal_algo(fg);

	pr_fg(VERBOSE, "flags: %d %d %d %d %d %d %d %d %d %d\n",
			flags.batt_status,
			flags.prev_batt_status,
			flags.chrgr_connected,
			flags.charging_enabled,
			flags.chrgr_pause,
			flags.init_capacity,
			flags.fg_eoc,
			flags.reschedule_work,
			flags.eoc_chargr_en,
			flags.calibration);
}

static int bcmpmu_fg_get_capacity_level(struct bcmpmu_fg_data *fg)
{
	int cap_percentage = fg->capacity_info.prev_percentage;
	int level;

	if (cap_percentage <= fg->pdata->cap_levels->critical)
		level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if (cap_percentage <= fg->pdata->cap_levels->low)
		level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (cap_percentage < fg->pdata->cap_levels->normal)
		level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	else if (cap_percentage <= fg->pdata->cap_levels->high)
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
		val->intval = flags.batt_status;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = CAPACITY_PERCENTAGE_FULL;
		else
			val->intval = fg->capacity_info.percentage;
		pr_fg(VERBOSE, "POWER_SUPPLY_PROP_CAPACITY = %d\n",
				val->intval);
		BUG_ON(val->intval < 0);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		else
			val->intval = bcmpmu_fg_get_capacity_level(fg);
		pr_fg(VERBOSE, "POWER_SUPPLY_PROP_CAPACITY_LEVEL = %d\n",
				val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		if (BATTERY_STATUS_UNKNOWN(flags))
			val->intval = 3700;
		else
			val->intval = bcmpmu_fg_get_batt_volt(fg);
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
	case POWER_SUPPLY_PROP_MODEL_NAME:
		if (fg->pdata->batt_prop->model)
			val->strval = fg->pdata->batt_prop->model;
		else
			val->strval = "Li-Ion Battery";
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
		pr_fg(VERBOSE, "Power supply %s is online & "
				"and charging\n", ext_psy->name);
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
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
			&fg->usb_det_nb);
	if (ret)
		return ret;

	fg->chrgr_status_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_STATUS,
			&fg->chrgr_status_nb);
	if (ret)
		goto unreg_usb_det_nb;

	fg->accy_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
			&fg->accy_nb);
	if (ret)
		goto unreg_chrg_status_nb;

	fg->chrgr_current_nb.notifier_call = bcmpmu_fg_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
			&fg->chrgr_current_nb);
	if (ret)
		goto unreg_chrgr_current_nb;


	return 0;
unreg_chrgr_current_nb:
	bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
			&fg->accy_nb);
unreg_chrg_status_nb:
	bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_STATUS,
			&fg->chrgr_status_nb);
unreg_usb_det_nb:
	bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
			&fg->usb_det_nb);
	return ret;
}

static int bcmpmu_fg_set_platform_data(struct bcmpmu_fg_data *fg,
		struct bcmpmu_fg_pdata *pdata)
{
	struct bcmpmu_batt_property *prop;

	if (!pdata)
		return -EINVAL;

	fg->pdata = pdata;

	if ((!pdata->batt_prop) || (!pdata->cap_levels) ||
			!(pdata->volt_levels))
		return -EINVAL;

	prop = pdata->batt_prop;

	/**
	 * verify mandatory fields from platform data
	 */
	if ((prop->full_cap == 0) || (!prop->volt_cap_lut) ||
			(!prop->esr_temp_lut))
		return -EINVAL;
	if (!pdata->eoc_current)
		pdata->eoc_current = FG_EOC_CURRENT;
	if (!pdata->sleep_current_ua)
		pdata->sleep_current_ua = FG_SLEEP_CURR_UA;
	if (!pdata->sleep_sample_rate)
		pdata->sleep_sample_rate = SAMPLE_RATE_2HZ;
	if (!pdata->fg_factor)
		pdata->fg_factor = FG_CURR_SCALING_FACTOR;
	if (prop->min_volt == 0)
		prop->min_volt = BATT_LI_ION_MIN_VOLT;
	if (prop->max_volt == 0)
		prop->max_volt = BATT_LI_ION_MAX_VOLT;

	fg->capacity_info.max_design = pdata->batt_prop->full_cap;

	if (fg->pdata->volt_levels->vfloat_max)
		bcmpmu_fg_set_vfloat_max_level(fg,
				pdata->volt_levels->vfloat_max);
	else
		bcmpmu_fg_set_vfloat_max_level(fg, BATT_VFLOAT_DEFAULT);


	/**
	 * set VFLOAT levels
	 */
	if (fg->pdata->volt_levels->vfloat_lvl)
		bcmpmu_fg_set_vfloat_level(fg, pdata->volt_levels->vfloat_lvl);
	else
		bcmpmu_fg_set_vfloat_level(fg, BATT_VFLOAT_DEFAULT);

	if (!fg->pdata->batt_prop->one_c_rate)
		fg->pdata->batt_prop->one_c_rate = BATT_ONE_C_RATE_DEF;
	if (!fg->pdata->i_def_dcp)
		fg->pdata->i_def_dcp = PMU_DCP_DEF_CURR_LMT;
	if (!fg->pdata->i_sat)
		fg->pdata->i_sat = PMU_TYP_SAT_CURR;
	if (!fg->pdata->acld_vbus_margin)
		fg->pdata->acld_vbus_margin = ACLD_VBUS_MARGIN;
	if (!fg->pdata->acld_cc_lmt)
		fg->pdata->acld_cc_lmt = ACLD_CC_LIMIT;
	if (!fg->pdata->otp_cc_trim)
		fg->pdata->otp_cc_trim = PMU_OTP_CC_TRIM;

	return 0;
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
	fg->pdata->eoc_current = eoc_current;
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
int debugfs_get_curr_open(struct inode *inode, struct file *file)
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
	.open = debugfs_get_curr_open,
	.read = debugfs_get_curr_read,
};

static int debugfs_get_batt_volt(void *data, u64 *volt)
{
	struct bcmpmu_fg_data *fg = data;
	*volt = bcmpmu_fg_get_batt_volt(fg);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_volt_fops,
	debugfs_get_batt_volt, NULL, "%llu\n");

static int debugfs_get_batt_temp(void *data, u64 *temp)
{
	struct bcmpmu_fg_data *fg = data;
	*temp = bcmpmu_fg_get_batt_temp(fg);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(fg_temp_fops,
	debugfs_get_batt_temp, NULL, "%llu\n");


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

static int debug_pmu_acld_ctrl(void *data, u64 acld_ctrl)
{
	struct bcmpmu_fg_data *fg = data;
	struct bcmpmu59xxx *bcmpmu = fg->bcmpmu;

	if ((bcmpmu->rev_info.prj_id == BCMPMU_59054_ID) &&
			(bcmpmu->rev_info.ana_rev >= BCMPMU_59054A1_ANA_REV)) {

		if (acld_ctrl) {
			bcmpmu->flags |= BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(fg, true);
			pr_fg(INIT, "ACLD Enabled\n");
		} else {
			bcmpmu->flags &= ~BCMPMU_ACLD_EN;
			bcmpmu_acld_enable(fg, false);
			pr_fg(INIT, "ACLD Disabled\n");
		}


	} else
		pr_fg(INIT, "ACLD not supported on this PMU Revision\n");

	pr_fg(INIT, "bcmpmu->flags = 0x%x\n", bcmpmu->flags);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(debug_pmu_acld_fops,
		NULL, debug_pmu_acld_ctrl, "%llu\n");

static void bcmpmu_fg_debugfs_init(struct bcmpmu_fg_data *fg)
{
	struct dentry *dentry_fg_dir;
	struct dentry *dentry_fg_file;
	struct bcmpmu59xxx *bcmpmu = fg->bcmpmu;

	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_fg(ERROR, "%s: dentry_bcmpmu is NULL", __func__);
		return;
	}

	dentry_fg_dir = debugfs_create_dir("fuelgauge", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_fg_dir))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("capacity", DEBUG_FS_PERMISSIONS,
				dentry_fg_dir,
				&fg->capacity_info.prev_percentage);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("eoc_curr", DEBUG_FS_PERMISSIONS,
				dentry_fg_dir,
				&fg->pdata->eoc_current);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("vfloat", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir,
			&fg->pdata->volt_levels->vfloat_lvl);
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

	dentry_fg_file = debugfs_create_file("acld_ctrl",
			DEBUG_FS_PERMISSIONS, dentry_fg_dir, fg,
			&debug_pmu_acld_fops);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;

	dentry_fg_file = debugfs_create_u32("debug_mask", DEBUG_FS_PERMISSIONS,
			dentry_fg_dir, &debug_mask);
	if (IS_ERR_OR_NULL(dentry_fg_file))
		goto debugfs_clean;
	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_fg_dir))
		debugfs_remove_recursive(dentry_fg_dir);
}
#endif

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

	flush_delayed_work_sync(&fg->fg_periodic_work);
	return 0;
}
#else
#define bcmpmu_fg_resume 	NULL
#define bcmpmu_fg_suspend	NULL
#endif

static int __devexit bcmpmu_fg_remove(struct platform_device *pdev)
{
	struct bcmpmu_fg_data *fg = platform_get_drvdata(pdev);

	/* Disable FG */
	bcmpmu_fg_enable(fg, false);
	if (!(fg->bcmpmu->flags & BCMPMU_SPA_EN))
		power_supply_unregister(&fg->psy);
	kfree(fg);
	return 0;
}

static int __devinit bcmpmu_fg_probe(struct platform_device *pdev)
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
	INIT_DELAYED_WORK(&fg->fg_acld_work, bcmpmu_fg_acld_algo);

	mutex_init(&fg->mutex);
	ret = bcmpmu_fg_register_notifiers(fg);
	if (ret)
		goto destroy_workq;
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
#endif /*CONFIG_WD_TAPPER*/

	fg->flags.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	fg->flags.prev_batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	fg->discharge_state = DISCHARG_STATE_HIGH_BATT;

	fg->flags.init_capacity = true;
	fg->crit_cutoff_cap = -1;
	fg->crit_cutoff_cap_prev = -1;
	fg->crit_cutoff_delta = 0;

	if (bcmpmu_fg_is_batt_present(fg)) {
		pr_fg(INIT, "main battery present\n");
		fg->flags.batt_present = true;
	} else
		pr_fg(INIT, "booting with power supply\n");

	fg->sample_rate = SAMPLE_RATE_2HZ;
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

	/**
	 * Run FG algorithm now
	 */
	queue_delayed_work(fg->fg_wq, &fg->fg_periodic_work, 0);

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
	.remove = __devexit_p(bcmpmu_fg_remove),
	.suspend = bcmpmu_fg_suspend,
	.resume = bcmpmu_fg_resume,

};

static int __init bcmpmu_fg_init(void)
{
	return platform_driver_register(&bcmpmu_fg_driver);
}
subsys_initcall_sync(bcmpmu_fg_init);

static void __exit bcmpmu_fg_exit(void)
{
	platform_driver_unregister(&bcmpmu_fg_driver);
}
module_exit(bcmpmu_fg_exit);

MODULE_DESCRIPTION("Broadcom PMU Fuel Gauge Driver");
MODULE_LICENSE("GPL");
