// SPDX-License-Identifier: GPL-2.0-only
/*
 * sm5038-charger.c - SM5038 Charger device driver
 *
 */

#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
//#include <linux/power_supply.h>
#include <linux/version.h>
#include <linux/math64.h>

#include <linux/power/sm5038_charger.h>
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
#include <linux/power/sm5038_step_chg_n_jeita.h>

#endif
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>

#include <linux/hardware_info.h>
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#include <linux/pmic-voter.h>
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
enum print_reason {
	PR_INTERRUPT	= BIT(0),
	PR_CORE		= BIT(1),
	PR_MISC		= BIT(2),
	PR_REGISTER	= BIT(3),
	PR_LEARN	= BIT(4),
	PR_STEP		= BIT(5),
};
#define somc_sm5038_charger_dbg(charger, reason, fmt, ...)			\
	do {								\
		if (charger->debug_mask & (reason))			\
			pr_info("[SOMC Charger] %s: "fmt, __func__, ##__VA_ARGS__);	\
		else							\
			pr_debug("[SOMC Charger] %s: "fmt, __func__, ##__VA_ARGS__);	\
	} while (0)
#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define NONE_CABLE_CURRENT  100
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define NONE_CABLE_CURRENT  0
#endif
#define MICRO_CURR_0P45A	450
#define MICRO_CURR_0P5A		500
#define MICRO_CURR_0P55A	550
#define MICRO_CURR_1P0A		1000
#define MICRO_CURR_1P5A		1500
#define MICRO_CURR_2P0A		2000
#define MICRO_CURR_2P5A		2500
#define MICRO_CURR_3P0A		3000

enum thermal_status_levels {
	TEMP_SHUT_DOWN_SM5038_BUCK_OFF = 0,
	TEMP_SHUT_DOWN_SM5038_CHG_OFF,
	TEMP_ALERT_LEVEL,
	TEMP_ABOVE_RANGE,
	TEMP_WITHIN_RANGE,
	TEMP_BELOW_RANGE,
};

#define CHARGE_SCREEN_ON_OFF /* debug screen_on/screen_off */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int screen_state = 1;
#endif

static struct sm5038_charger_data *static_charger_data;

char *charge_mode_str[] = {
	"Charging-Off",
	"Charging-On",
	"Buck-Off",
};

static struct device_attribute sm5038_charger_attrs[] = {
	SM5038_CHARGER_ATTR(chip_id),
	SM5038_CHARGER_ATTR(set_ship_mode),
	SM5038_CHARGER_ATTR(battery_charging_enabled),	
};

static char *sm5038_supplied_to[] = {
	"sm5038-charger",
};

static enum power_supply_property sm5038_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT,
	POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
	/* sm5038-fuelgauge */
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_SCOPE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	POWER_SUPPLY_PROP_MODEL_NAME,
#endif
};

static enum power_supply_property sm5038_otg_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

/* fuelgauge */
extern int sm5038_fg_get_prop_cycle(void);
extern int sm5038_fg_get_prop_vbat_max(void);
extern int sm5038_fg_get_prop_vbat_min(void);
extern int sm5038_fg_get_prop_init_vbat_max(void);
extern int sm5038_fg_get_prop_init_vbat_min(void);
extern int sm5038_fg_get_prop_vbat_now(void);
extern int sm5038_fg_get_prop_vbat_avg(void);
extern int sm5038_fg_get_prop_ocv(void);
extern int sm5038_fg_get_prop_rsoc(void);
extern int sm5038_fg_get_prop_soc(void);
extern int sm5038_fg_get_prop_q_max(void);
extern int sm5038_fg_get_prop_q_now(void);
extern int sm5038_fg_get_prop_soc_cycle(void);
extern int sm5038_fg_get_prop_soh(void);
extern int sm5038_fg_get_prop_temp(void);
extern int sm5038_fg_get_prop_temp_min(void);
extern int sm5038_fg_get_prop_temp_max(void);
extern int sm5038_fg_get_prop_current_now(void);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
extern int sm5038_fg_get_prop_current_avg(void);
#endif
extern int sm5038_fg_get_prop_q_max_design(void);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
extern int somc_sm5038_fg_get_prop_batt_soc(void);
extern int somc_sm5038_fg_learn_update(void);
extern int somc_sm5038_fg_learn_get_counter(void);
extern int somc_sm5038_fg_learn_set_range(int min, int max);
extern int somc_sm5038_fg_learn_get_learned_capacity_raw(void);

/* muic */
extern bool somc_sm5038_is_cc_reconnection_running(void);
#endif

/* typec */
extern int sm5038_cc_control_command(int enable);
extern int sm5038_charger_cable_type_update(int cable_type);
extern int sm5038_charger_charge_mode_update(int charge_mode);
extern int sm5038_charger_chg_on_status_update(int chg_on_status);
extern int sm5038_charger_input_current_update(int input_current);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
/* jeita */
extern int update_step_chg_n_jeita_work(void);

static void sm5038_chg_set_fast_charging_current_by_type(struct sm5038_charger_data *charger, int cable_type);
#endif
static void sm5038_chg_set_vbuslimit_current_by_type(struct sm5038_charger_data *charger, int cable_type);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void sm5038_chg_buck_control(struct sm5038_charger_data *charger, bool buck_on);
static void sm5038_chg_charging(struct sm5038_charger_data *charger, int chg_en);
static void psy_chg_set_charging_enable(struct sm5038_charger_data *charger, int charge_mode);
#endif
static void sm5038_charger_enable_aicl_irq(struct sm5038_charger_data *charger);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_thermal_status(struct sm5038_charger_data *charger, int thermal);
#endif

extern int sm5038_step_chg_n_jeita_init(struct device *dev, bool step_chg_enable, bool sw_jeita_enable);
extern void sm5038_step_chg_n_jeita_deinit(void);

extern int sm5038_charger_oper_push_event(int event_type, bool enable);

/* add adc begin */
int sm5038_parse_dt_adc_channels(struct sm5038_charger_data *charger);
int sm5038_get_batt_id_ohm(unsigned int *batt_id_ohm);
int sm5038_get_batt_therm(int *batt_therm);
int sm5038_get_usb_conn_therm(int *usb_conn_therm);
int sm5038_get_charger_skin_therm(int *charger_skin_therm);
/* add adc end */

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void sm5038_get_capacity(struct sm5038_charger_data *charger, int *val)
{
	if (charger->fake_capacity >= 0 && charger->fake_capacity <= 100) {
		*val = charger->fake_capacity;

		return;
	}
	charger->batt_soc = (int)somc_sm5038_fg_get_prop_batt_soc();
	if (charger->full_soc_hold_en)
		*val = 10000;
	else if (charger->full_soc_adjust_en)
		*val = charger->full_adjust_soc;
	else if (charger->soc_adjust_en)
		*val = charger->adjust_soc;
	else
		*val = charger->batt_soc;

	somc_sm5038_charger_dbg(charger, PR_MISC, "Capacity:%d, full_soc_hold_en:%d, full_adjust_soc:%d, soc_adjust_en:%d, adjust_soc:%d, batt_soc:%d\n",
			*val, charger->full_soc_hold_en,
			charger->full_adjust_soc, charger->full_soc_adjust_en,
			charger->adjust_soc, charger->batt_soc);

	if (*val > 10000)
		*val = 100;
	else
		*val = (*val + 50) / 100;
}

static void somc_sm5038_timed_fake_chg_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work,
			struct sm5038_charger_data, timed_fake_chg_work.work);

	vote(charger->fake_chg_votable, TIMED_FAKE_CHG_VOTER, false, 0);
}

static void somc_sm5038_start_timed_fake_charging(
		struct sm5038_charger_data *charger, unsigned int delay_msecs)
{
	unsigned long delay;
	bool delay_scheduled = false;

	if (!delay_msecs)
		return;

	delay = msecs_to_jiffies(delay_msecs);
	vote(charger->fake_chg_votable, TIMED_FAKE_CHG_VOTER, true, 0);

	if (delayed_work_pending(&charger->timed_fake_chg_work)) {
		if (charger->timed_fake_chg_expire > jiffies + delay)
			delay_scheduled = true;
		else
			cancel_delayed_work_sync(
					&charger->timed_fake_chg_work);
	}
	if (!delay_scheduled) {
		schedule_delayed_work(&charger->timed_fake_chg_work, delay);
		charger->timed_fake_chg_expire = jiffies + delay;
	}
}

static void somc_sm5038_start_fake_charging(struct sm5038_charger_data *charger,
							const char *voter)
{
	vote(charger->fake_chg_votable, voter, true, 0);
}

static void somc_sm5038_stop_fake_charging(struct sm5038_charger_data *charger,
				const char *voter, unsigned int delay_msecs)
{
	int rc;

	rc = get_client_vote(charger->fake_chg_votable, voter);
	if (!rc || rc == EINVAL)
		return;

	if (delay_msecs)
		somc_sm5038_start_timed_fake_charging(charger, delay_msecs);

	vote(charger->fake_chg_votable, voter, false, 0);
}

static int somc_sm5038_get_charge_full_design(struct sm5038_charger_data *charger)
{
	return sm5038_fg_get_prop_q_max_design();
}

static int somc_sm5038_get_charge_full(struct sm5038_charger_data *charger)
{
	int val, charge_full_design;

	charge_full_design = somc_sm5038_get_charge_full_design(charger);

	if (charger->learned_capacity)
		val = charger->learned_capacity;
	else if (charger->restored_charge_full)
		val = charger->restored_charge_full;
	else
		val = charge_full_design;

	somc_sm5038_charger_dbg(charger, PR_LEARN, "charger->learned_capacity:%d, restored_charge_full:%d, charge_full_design:%d, val:%d",
				charger->learned_capacity, charger->restored_charge_full,
				charge_full_design, val);
	return val;
}

#define SOMC_LEARN_FILTER_UPPER_WIDTH	20 /* 2% */
#define SOMC_LEARN_FILTER_LOWER_WIDTH	20 /* 2% */
#define SOMC_LEARN_LOWER_LIMIT		400 /* 40% */
#define SOMC_LEARN_VENDOR_ADJUST_THR	950 /* 95% */
#define SOMC_LEARN_VENDOR_ADJUST_DENOM	107 /* decrease 7% */
static void somc_sm5038_update_charge_full(struct sm5038_charger_data *charger)
{
	int charge_full_design, charge_full_temp;
	int learned_capacity = 0, learned_capacity_temp = 0;
	int learned_capacity_raw;
	int filt_up_width, filt_lo_width, upper_limit, lower_limit;
	int learning_counter = 0;

	charge_full_design = somc_sm5038_get_charge_full_design(charger);
	charge_full_temp = somc_sm5038_get_charge_full(charger);

	/* update learned_capacity_raw in FG */
	somc_sm5038_fg_learn_update();

	learning_counter = somc_sm5038_fg_learn_get_counter();
	if (learning_counter == charger->learning_counter) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Do not update charge_full because counter is not incremented. learning_counter:%d",
					learning_counter);
		return;
	}
	charger->learning_counter = learning_counter;

	learned_capacity_raw =
			somc_sm5038_fg_learn_get_learned_capacity_raw() * 1000;
	charger->learned_capacity_raw = learned_capacity_raw;
	/* Filter(1): Keep initial value until batt_aging_level is changed for the first time */
	if (charger->batt_unaged) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Keep initial value until batt_aging_level is changed for the first time. learned_capacity_raw:%d, charge_full_temp:%d",
				learned_capacity_raw, charge_full_temp);
		return;
	}

	learned_capacity = learned_capacity_raw;
	/* Filter(2): Adjust for aged battery by vendor spec */
	if ((u64)learned_capacity * 1000 / charge_full_design <
						SOMC_LEARN_VENDOR_ADJUST_THR) {
		learned_capacity_temp = learned_capacity;
		learned_capacity = (int)((u64)learned_capacity_temp * 100 /
									107);
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(2): Adjust by Vendor algorithm. learned_capacity:%d->%d, learned_capacity_raw:%d",
					learned_capacity_temp,
					learned_capacity,
					learned_capacity_raw);
	}

	/* Filter(3): Adjust by Aging Level */
	if (charger->target_soc_adj_coef) {
		learned_capacity_temp = learned_capacity;
		learned_capacity = (int)((u64)learned_capacity_temp * 10000 /
						charger->target_soc_adj_coef);
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(3): Adjust by Aging Level. learned_capacity:%d->%d, learned_capacity_raw:%d, target_soc_adj_coef:%d",
					learned_capacity_temp,
					learned_capacity,
					learned_capacity_raw,
					charger->target_soc_adj_coef);
	}

	/* Filter(4): Suppress fluctuations by 2% */
	filt_up_width = charge_full_design * SOMC_LEARN_FILTER_UPPER_WIDTH /
									1000;
	filt_lo_width = charge_full_design * SOMC_LEARN_FILTER_LOWER_WIDTH /
									1000;
	if (learned_capacity - charge_full_temp > filt_up_width) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(4)-1: Suppress upper fluctuations. learned_capacity:%d->%d, filt_up_width:%d",
					learned_capacity,
					charge_full_temp + filt_up_width,
					filt_up_width);
		learned_capacity = charge_full_temp + filt_up_width;
	} else if (charge_full_temp - learned_capacity > filt_lo_width) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(4)-2: Suppress lower fluctuations. learned_capacity:%d->%d, filt_lo_width:%d",
					learned_capacity,
					charge_full_temp - filt_lo_width,
					filt_lo_width);
		learned_capacity = charge_full_temp - filt_lo_width;
	}

	/* Filter(5): Limit upper or lower */
	upper_limit = charge_full_design;
	lower_limit = charge_full_design * SOMC_LEARN_LOWER_LIMIT / 1000;
	if (learned_capacity > upper_limit) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(5)-1: Limit Max. learned_capacity:%d->%d",
						learned_capacity, upper_limit);
		learned_capacity = upper_limit;
	} else if (learned_capacity < lower_limit) {
		somc_sm5038_charger_dbg(charger, PR_LEARN, "Filter(5)-2: Limit Min. learned_capacity:%d->%d",
						learned_capacity, lower_limit);
		learned_capacity = lower_limit;
	}
	charger->learned_capacity = learned_capacity;

	somc_sm5038_charger_dbg(charger, PR_LEARN, "learned_capacity_raw:%d, charge_full_temp:%d, learned_capacity%d, charge_full_design:%d",
				learned_capacity_raw, charge_full_temp,
				learned_capacity, charge_full_design);
}

int somc_sm5038_present_is_pd_apdapter(void)
{
	struct sm5038_charger_data *charger = static_charger_data;

	return charger->present_is_pd_apdapter;
}

#endif
int sm5038_charger_get_chg_mode_type(void)
{
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}

	return (charger->charge_mode);
}
EXPORT_SYMBOL_GPL(sm5038_charger_get_chg_mode_type);

int sm5038_charger_get_chg_on_status(void)
{
	struct sm5038_charger_data *charger = static_charger_data;
	u8 reg;
	int chgon = false;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}
	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg);

	if (reg&0x1)
		chgon = true;
		
	return chgon;
}
EXPORT_SYMBOL_GPL(sm5038_charger_get_chg_on_status);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static inline u8 _cal_floating_voltage_mv_to_offset(u16 mV)
{
	u8 offset;

	if (mV <= 3700)
		offset = 0x0;
	else if (mV < 3900)
		offset = ((mV - 3700) / 50);    /* BATREG = 3.75 ~ 3.85V in 0.05V steps */
	else if (mV < 4050)
		offset = (((mV - 3900) / 100) + 4);    /* BATREG = 3.90, 4.0V in 0.1V steps */
	else if (mV < 4630)
		offset = (((mV - 4050) / 10) + 6);    /* BATREG = 4.05 ~ 4.62V in 0.01V steps */
	else {
		offset = 0x15;    /* default Offset : 4.2V */
	}
	return offset;
}



static void chg_set_batreg(struct sm5038_charger_data *charger, u16 float_voltage)
{
	u8 offset;

	charger->float_voltage = float_voltage;

	offset = _cal_floating_voltage_mv_to_offset(float_voltage);

	pr_info("sm5038-charger: %s: set as  (mV=%d) batreg Control\n", __func__, float_voltage);

	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL4, ((offset & 0x3F) << 0), (0x3F << 0));
}
#endif


/* AUTO ship mode condition : [SHIP_FORCED = 0] & [VSYS < SHIP_AUTO] */
static void chg_set_shipmode_auto_th(struct sm5038_charger_data *charger, u8 vref_offset)
{
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL11, (vref_offset << 1), (0x3 << 1));
	pr_info("sm5038-charger: %s: %d mV \n", __func__, (2600 + (vref_offset * 200)));
}


/* Forced ship mode condition : [SHIP_FORCED = 1] & set shipmode in time */
static void chg_set_shipmode_enter_time(struct sm5038_charger_data *charger, u8 in_time_offset)
{
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL11, (in_time_offset << 3), (0x3 << 3));	/*	SHIP_IN_TIME	*/
	pr_info("sm5038-charger: %s: %d s \n", __func__, ((in_time_offset + 1) * 8));
}

static int chg_get_shipmode_status(struct sm5038_charger_data *charger)
{
	u8 reg;
	int mode;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL11, &reg);
	mode = (reg & (0x1 << 5)) ? 1 : 0;

//	pr_info("sm5038-charger: %s: ship_mode[%s] \n", __func__, mode ? "forced" : "auto");

	return mode;
}

static void chg_set_en_shipmode(struct sm5038_charger_data *charger, int mode)
{
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL11, (mode << 5), (0x1 << 5));

	pr_info("sm5038-charger: %s: ship_mode[%s] \n", __func__, mode ? "forced" : "auto");
}

static void chg_set_aiclth(struct sm5038_charger_data *charger, u8 aicl_offset)
{
	int aicl_th = 4600;

    sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL11, (aicl_offset << 6), (0x3 << 6));

	if (aicl_offset) {
		aicl_th -= ((0x3 - aicl_offset) * 100);
	}

	pr_info("sm5038-charger: set aich_th(%d mV) \n", aicl_th);
}

static void chg_set_aiclen_vbus(struct sm5038_charger_data *charger, bool enable)
{
    sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1, (enable << 6), (0x1 << 6));

	pr_info("sm5038-charger: aiclen_vbus(%s) \n", enable ? "Enable" : "Disable");
}

static void chg_set_dischg_limit(struct sm5038_charger_data *charger, u8 dischg_offset)
{
    sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL6, (dischg_offset << 1), (0x7 << 1));

	pr_info("sm5038-charger: discharge current = %d mA \n",
		(dischg_offset == DISCHG_LIMIT_DISABLE) ? 0:(5400 + (dischg_offset * 600)));
}

static void chg_set_ocp_current(struct sm5038_charger_data *charger, u32 ocp_current)
{
	u8 dischg = DISCHG_LIMIT_C_5_4;

	if (ocp_current >= 9000)
		dischg = DISCHG_LIMIT_C_9_0;
	else if (ocp_current >= 8400)
		dischg = DISCHG_LIMIT_C_8_4;
	else if (ocp_current >= 7800)
		dischg = DISCHG_LIMIT_C_7_8;
	else if (ocp_current >= 7200)
		dischg = DISCHG_LIMIT_C_7_2;
	else if (ocp_current >= 6600)
		dischg = DISCHG_LIMIT_C_6_6;
	else if (ocp_current >= 6000)
		dischg = DISCHG_LIMIT_C_6_0;
	else if (ocp_current >= 5400)
		dischg = DISCHG_LIMIT_C_5_4;
	else
		dischg = DISCHG_LIMIT_DISABLE;

	chg_set_dischg_limit(charger, dischg);
}


static void chg_set_wdt_timer(struct sm5038_charger_data *charger, u8 wdt_timer)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
    sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WDTCNTL, (wdt_timer << 1), (0x3 << 1));
#endif
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_wdt_tmr_reset(struct sm5038_charger_data *charger)
{
	pr_debug("sm5038-charger: %s: wdt kick\n", __func__);
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WDTCNTL, (0x1 << 3), (0x1 << 3));
}

static void chg_set_wdt_enable(struct sm5038_charger_data *charger, bool enable)
{
	pr_info("sm5038-charger: %s: wdt enable(%d)\n", __func__, enable);
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WDTCNTL, (enable << 0), (0x1 << 0));
	if (enable)
		chg_set_wdt_tmr_reset(charger);
}

static void chg_set_wdtcntl_reset(struct sm5038_charger_data *charger)
{
	pr_info("sm5038-charger: %s: clear wdt expired\n", __func__);
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WDTCNTL, (0x1 << 6), (0x1 << 6));
}
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void somc_sm5038_disable_watchdog(struct sm5038_charger_data *charger)
{
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WDTCNTL, (0x0 << 0),
								(0x1 << 0));
}

#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_enq4fet(struct sm5038_charger_data *charger, bool enable)
{

	pr_info("sm5038-charger: %s: ENQ4FET(%d)\n", __func__, enable);
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1, (enable << 3), (0x1 << 3));
}
#endif

static void chg_set_fast_charging_step(struct sm5038_charger_data *charger, int step)
{
	pr_info("sm5038-charger: %s: SS_FAST  = %d mA/ms \n", __func__, (step ? 5 : 10));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL1, (step << 1), (0x1 << 0));

	return;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_input_current_limit(struct sm5038_charger_data *charger, int mA)
{
	u8 offset;


	if (mA > 3275 || mA < 0) {
		pr_err("sm5038-charger: %s: %d mA greater than max current, so keep current (%d mA)\n", __func__, mA, charger->input_current);
		return;
	}

	mutex_lock(&charger->charger_mutex);
	if (mA < 100) {
		offset = 0x00;
	} else {
		offset = ((mA - 100) / 25) & 0x7F;
	}
	if (is_wireless_type(charger->cable_type)) {
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WPCINCNTL2, (offset << 0), (0x7F << 0));
	} else {
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_VBUSCNTL, (offset << 0), (0x7F << 0));
	}
	charger->input_current = mA;
	sm5038_charger_input_current_update(charger->input_current);

	mutex_unlock(&charger->charger_mutex);
}
#endif

static int chg_get_input_current_limit(struct sm5038_charger_data *charger)
{
	u8 reg;

	if (is_wireless_type(charger->cable_type)) {
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_WPCINCNTL2, &reg);
	} else {
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_VBUSCNTL, &reg);
	}

	return ((reg & 0x7F) * 25) + 100;
}


static void chg_set_topoff_timer(struct sm5038_charger_data *charger, u8 tmr_offset)
{
	pr_info("sm5038-charger: %s: %d min \n", __func__, (tmr_offset == 0x3 ? 45 : 10*(tmr_offset+1)));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL7, (tmr_offset << 3), (0x3 << 3));
}

static void chg_set_enfastchgtimer(struct sm5038_charger_data *charger, bool enable)
{
	pr_info("sm5038-charger: %s: %s \n", __func__, (enable ? "ENABLE" : "DISABLE"));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL7, (enable << 0), (0x1 << 0));
}

static void chg_set_fastchg_timer(struct sm5038_charger_data *charger, u8 tmr_offset)
{
	pr_info("sm5038-charger: %s: %d hours \n", __func__, (4 + (tmr_offset * 2)) );
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL7, (tmr_offset << 1), (0x3 << 1));
}

#define PRINT_CHG_1ST_GROUP_REG_NUM    35	/* REG_0x00 ~ 0x29 */
#define PRINT_CHG_2ND_GROUP_REG_NUM    1	/* REG_0x30 */
#define PRINT_CHG_3TH_GROUP_REG_NUM    17	/* REG_0x40 ~ 0x50 */

static void chg_print_regmap(struct sm5038_charger_data *charger)
{

	u8 reg1s[PRINT_CHG_1ST_GROUP_REG_NUM] = {0x0, };
	u8 reg2 = 0x0;
	u8 reg3s[PRINT_CHG_3TH_GROUP_REG_NUM] = {0x0, };
	char temp_buf[500] = {0,};
	int i;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (!charger->regdump_en)
		return;
#endif
	// READ REG_0x00 ~ 0x29
	sm5038_bulk_read(charger->i2c, SM5038_CHG_REG_INTMSK1, PRINT_CHG_1ST_GROUP_REG_NUM, reg1s);
	for (i = 0; i < PRINT_CHG_1ST_GROUP_REG_NUM; ++i) {
		sprintf(temp_buf+strlen(temp_buf), "0x%02X[0x%02X],", SM5038_CHG_REG_INTMSK1 + i, reg1s[i]);
		if (i % 12 == 11) {
			pr_info("sm5038-charger: regmap: %s\n", temp_buf);
			memset(temp_buf, 0x0, sizeof(temp_buf));
		}
	}

	// READ REG_0x30
	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_WPC_VOLTAGE, &reg2);
	sprintf(temp_buf+strlen(temp_buf), "0x%02X[0x%02X],", SM5038_CHG_REG_WPC_VOLTAGE, reg2);
	pr_info("sm5038-charger: regmap: %s\n", temp_buf);
	memset(temp_buf, 0x0, sizeof(temp_buf));


	// READ REG_0x40 ~ 0x50
	sm5038_bulk_read(charger->i2c, SM5038_CHG_REG_SINKADJ, PRINT_CHG_3TH_GROUP_REG_NUM, reg3s);
	for (i = 0; i < PRINT_CHG_3TH_GROUP_REG_NUM; ++i) {
		sprintf(temp_buf+strlen(temp_buf), "0x%02X[0x%02X],", SM5038_CHG_REG_SINKADJ + i, reg3s[i]);
		if (i % 12 == 11 || i == (PRINT_CHG_3TH_GROUP_REG_NUM - 1)) {
			pr_info("sm5038-charger: regmap: %s\n", temp_buf);
			memset(temp_buf, 0x0, sizeof(temp_buf));
		}
	}

}

static int sm5038_chg_create_attrs(struct device *dev)
{
	unsigned long i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(sm5038_charger_attrs); i++) {
		rc = device_create_file(dev, &sm5038_charger_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	return rc;

create_attrs_failed:
	pr_err("%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &sm5038_charger_attrs[i]);
	return rc;
}

ssize_t sm5038_chg_show_attrs(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sm5038_charger_data *charger =	power_supply_get_drvdata(psy);
	const ptrdiff_t offset = attr - sm5038_charger_attrs;
	int i = 0;
	int value = 0;

	if (charger == NULL) {
		pr_err("%s: charger is NULL \n", __func__);
		return -ENODEV;
	}

	switch (offset) {
	case CHIP_ID:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", "SM5038");
		break;
	case SET_SHIP_MODE:
		value = chg_get_shipmode_status(charger);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", value);
		pr_info("manual ship mode get as %s\n", value ? "enable" : "disable");
		break;
	case BATTERY_CHARGING_ENABLED:
		value = charger->charge_mode;
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", value);
		pr_info("charge_mode get is %s \n", value ? "CHG_ON" : "CHG_OFF");
		break;
	default:
		return -EINVAL;
	}
	return i;
}

ssize_t sm5038_chg_store_attrs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sm5038_charger_data *charger =	power_supply_get_drvdata(psy);
	const ptrdiff_t offset = attr - sm5038_charger_attrs;
	int ret = 0;
	u32 store_value;

	if (charger == NULL) {
		pr_err("%s: charger is NULL \n", __func__);
		return -ENODEV;
	}

	if (buf == NULL || kstrtouint(buf, 10, &store_value)) {
		return -ENXIO;
	}

	pr_info("sm5038-charger: %s:  store_value = %d \n", __func__, store_value);
	if (charger->i2c == NULL) {
		pr_err("%s: Charger i2c is NULL \n", __func__);
	}
	ret = count;

	switch (offset) {
	case CHIP_ID:
		break;
	case SET_SHIP_MODE:
		pr_info("SHIP_MODE is set [%d]\n", store_value);
		/*if val.intval = 1, immediately enter ship_mode */
		chg_set_en_shipmode(charger, store_value);
		break;
	case BATTERY_CHARGING_ENABLED:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		pr_info("BATTERY_CHARGING_ENABLED is set [%d]\n", store_value);
		psy_chg_set_charging_enable(charger, store_value);
		chg_print_regmap(charger);
#endif
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void sm5038_chg_print_status(struct sm5038_charger_data *charger)
{
	u8 reg_st[5] = {0, };
	
	sm5038_bulk_read(charger->i2c, SM5038_CHG_REG_STATUS1, 5, reg_st);

	pr_info("sm5038-charger: %s: STATUS1(0x%02x), STATUS2(0x%02x), STATUS3(0x%02x) , STATUS4(0x%02x) , STATUS5(0x%02x)\n",
		__func__, reg_st[0], reg_st[1], reg_st[2], reg_st[3], reg_st[4]);

	return ;
}
#endif
static int psy_chg_get_status(struct sm5038_charger_data *charger)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	int chg_on_status = 0;
	u8 reg_st[2] = {0, 0};

	if (charger->is_charging)
		chg_set_wdt_tmr_reset(charger);

	sm5038_bulk_read(charger->i2c, SM5038_CHG_REG_STATUS1, 2, reg_st);

	chg_on_status = (reg_st[0] & 0x1);	/* 1 : vbuspok, 0: vbusuvlo */
	sm5038_charger_chg_on_status_update(chg_on_status);
	
	if (reg_st[1] & (0x1 << 5)) { /* check: Top-off */
		status = POWER_SUPPLY_STATUS_FULL;
	} else if (reg_st[1] & (0x1 << 3)) {  /* check: Charging ON */
		status = POWER_SUPPLY_STATUS_CHARGING;
	} else {
		if ((reg_st[0] & (0x1 << 0)) || (reg_st[0] & (0x1 << 4))) { /* check: VBUS_POK, WPC_POK */
			status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		} else {
			status = POWER_SUPPLY_STATUS_DISCHARGING;
		}
	}
	charger->charging_status = status;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u8 reg[2];
	int chg_on_status = 0;

	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	union power_supply_propval pval = {0, };

	sm5038_bulk_read(charger->i2c, SM5038_CHG_REG_STATUS1, 2, reg);
	chg_on_status = (reg[0] & 0x1); /* 1 : vbuspok, 0: vbusuvlo */
	sm5038_charger_chg_on_status_update(chg_on_status);

	if (reg[1] & (0x1 << 3))
		status = POWER_SUPPLY_STATUS_CHARGING;
	else if (reg[0] & (0x1 << 0))
		status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	else
		status = POWER_SUPPLY_STATUS_DISCHARGING;

	if (charger->is_full)
		status = POWER_SUPPLY_STATUS_FULL;

	charger->charging_status = status;

	if (status == POWER_SUPPLY_STATUS_NOT_CHARGING &&
		!get_effective_result(charger->fake_chg_disallow_votable) &&
		get_effective_result(charger->fake_chg_votable))
		status = POWER_SUPPLY_STATUS_CHARGING;

	if (status == POWER_SUPPLY_STATUS_FULL &&
		charger->jeita_step_condition == JEITA_STEP_COND_WARM)
		status = POWER_SUPPLY_STATUS_CHARGING;

	if (charger->bootup_shutdown_phase != DURING_POWER_OFF_CHARGE) {
		if (charger->psy_chg && !power_supply_get_property(
			charger->psy_chg, POWER_SUPPLY_PROP_CAPACITY, &pval)) {
			if (pval.intval == 0)
				status = POWER_SUPPLY_STATUS_DISCHARGING;
		}
	}
#endif
	return status;
}


static int sm5038_read_iio_channel(struct sm5038_charger_data *charger,
		struct iio_channel *chan, int div, int *data)
{
	int ret = 0;
	*data = -ENODATA;

	if (chan) {
		ret = iio_read_channel_processed(chan, data);
		if (ret < 0) {
			pr_err("sm5038-charger: %s: Error in reading IIO channel data, ret=%d\n", __func__, ret);
			return ret;
		}

		if (div != 0)
			*data /= div;
	}

	return ret;
}

/******************************************************/

/*  add get batt_id  begin*/
#define BID_RPULL_OHM		100000
#define BID_VREF_MV			1600

int sm5038_get_batt_id_ohm(unsigned int *batt_id_ohm)
{
	int ret, batt_id_mv;
	static int batt_id_adc;
	int64_t denom;
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}

	/* Read battery-id */
	ret = iio_read_channel_processed(charger->pmk8350_batt_id, &batt_id_mv);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Failed to read BATT_ID over ADC, ret=%d\n", __func__, ret);
		return ret;
	}

	batt_id_adc = batt_id_mv;

	get_hardware_info_data(HWID_BATERY_ID, "SNYSCA6");
	get_hardware_info_data(HWID_BATERY_ID_ADC, &batt_id_adc);
//	pr_info("sm5038-charger: %s: read BATT_ID_ADC = %d\n", __func__, batt_id_adc);

	batt_id_mv = div_s64(batt_id_mv, 1000);
	if (batt_id_mv == 0) {
		pr_info("sm5038-charger: %s: batt_id_mv = 0 from ADC\n", __func__);
		return 0;
	}

	denom = div64_s64(BID_VREF_MV * 1000, batt_id_mv) - 1000;
	if (denom <= 0) {
		/* batt id connector might be open, return 0 kohms */
		return 0;
	}

	*batt_id_ohm = div64_u64(BID_RPULL_OHM * 1000 + denom / 2, denom);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_get_batt_id_ohm);

/*  add get batt_id  end */

#define DIV_FACTOR_MICRO_V_I	1
#define DIV_FACTOR_DECIDEGC	100
/*   add get batt_therm begin */
int sm5038_get_batt_therm(int *batt_therm)
{
	int ret = 0;
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_read_iio_channel(charger, charger->pm6125_batt_therm, DIV_FACTOR_MICRO_V_I, batt_therm);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: get batt_therm is error,ret = %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_get_batt_therm);

/*   add get batt_therm end */

/*   add get charger_skin_therm begin */
int sm5038_get_charger_skin_therm(int *charger_skin_therm)
{
	int ret = 0;
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_read_iio_channel(charger, charger->pmr735a_charger_skin_therm, DIV_FACTOR_DECIDEGC, charger_skin_therm);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: get charger_skin_therm is error,ret = %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_get_charger_skin_therm);

/*   add get charger_skin_therm end */

/*   add get usb_conn_therm begin */
int sm5038_get_usb_conn_therm(int *usb_conn_therm)
{
	int ret = 0;
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return -EINVAL;
	}

	ret = sm5038_read_iio_channel(charger, charger->pmk8350_usb_conn_therm, DIV_FACTOR_DECIDEGC, usb_conn_therm);
	if (ret < 0)	{
		pr_err("sm5038-charger: get usb_conn_therm is error,ret = %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_get_usb_conn_therm);

/*   add get usb_conn_therm end */

int sm5308_get_iio_channel(struct sm5038_charger_data *charger, const char *propname,
					struct iio_channel **chan)
{
	int ret = 0;

	ret = of_property_match_string(charger->dev->of_node,
					"io-channel-names", propname);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: iio chan pase dtsi error  ret = %d\n", __func__, ret);
		return 0;
	}

	*chan = iio_channel_get(charger->dev, propname);
	if (IS_ERR(*chan)) {
		ret = PTR_ERR(*chan);
		if (ret != -EPROBE_DEFER)
			pr_err("sm5038-charger: %s: %s channel unavailable, %d\n", __func__, propname, ret);
		*chan = NULL;
	}

	return ret;
}

/* add adc begin */
int sm5038_parse_dt_adc_channels(struct sm5038_charger_data *charger)
{
	int ret = 0;

	ret = sm5308_get_iio_channel(charger, "pm6125_batt_therm", &charger->pm6125_batt_therm);
	if (ret < 0) {
		pr_err("sm5038-charger: ERROR:%s:Failed to parse adc channel %s;rc = %d\n", __func__, "pm6125_batt_therm", ret);
		return ret;
	} else {
		pr_info("sm5038-charger: %s: pm6125_batt_therm = %x\n", __func__, charger->pm6125_batt_therm);
	}

	ret = sm5308_get_iio_channel(charger, "pmr735a_charger_skin_therm", &charger->pmr735a_charger_skin_therm);
	if (ret < 0) {
		pr_err("sm5038-charger: ERROR:%s:Failed to parse adc channel %s;rc = %d\n", __func__, "pmr735a_charger_skin_therm", ret);
		return ret;
	} else {
		pr_info("sm5038-charger: %s: pmr735a_charger_skin_therm = %x\n", __func__, charger->pmr735a_charger_skin_therm);
	}

	ret = sm5308_get_iio_channel(charger, "pmk8350_batt_id", &charger->pmk8350_batt_id);
	if (ret < 0) {
		pr_err("sm5038-charger: ERROR:%s:Failed to parse adc channel %s;rc = %d\n", __func__, "pmk8350_batt_id", ret);
		return ret;
	} else {
		pr_info("sm5038-charger: %s: pmk8350_batt_id = %x\n", __func__, charger->pmk8350_batt_id);
	}

	ret = sm5308_get_iio_channel(charger, "pmk8350_usb_conn_therm", &charger->pmk8350_usb_conn_therm);
	if (ret < 0) {
		pr_err("sm5038-charger: ERROR:%s:Failed to parse adc channel %s;rc = %d\n", __func__, "pmk8350_usb_conn_therm", ret);
		return ret;
	} else {
		pr_info("sm5038-charger: %s: pmk8350_usb_conn_therm = %x\n", __func__, charger->pmk8350_usb_conn_therm);
	}

	return 0;
}
/* add adc end */

void psy_chg_set_regulation_voltage(u16 mV)
{
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return ;
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chg_set_batreg(charger, mV);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->fv_votable, DEFAULT_VOTER, true, mV);
#endif

	if(charger->psy_chg)
		power_supply_changed(charger->psy_chg);

}
EXPORT_SYMBOL_GPL(psy_chg_set_regulation_voltage);

int psy_chg_get_charge_type(struct sm5038_charger_data *charger)
{
	int charge_type;
	u8 reg;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (charger->is_charging) {
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (charger->charging_status == POWER_SUPPLY_STATUS_CHARGING ||
		charger->charging_status == POWER_SUPPLY_STATUS_FULL) {
#endif
		if (charger->slow_rate_chg_mode) {
			pr_info("sm5038-charger: %s: slow rate charge mode\n", __func__);
			charge_type = POWER_SUPPLY_CHARGE_TYPE_ADAPTIVE;
		} else {
			if (reg & (0x1 << 4)) { /* CHECK Q4FULLON Status */
				charge_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
			} else {
				charge_type = POWER_SUPPLY_CHARGE_TYPE_TRICKLE; 	/* Linear-Charge mode */
			}
		}
	} else {
		charge_type = POWER_SUPPLY_CHARGE_TYPE_NONE;
	}

	return charge_type;
}

static int psy_chg_get_present(struct sm5038_charger_data *charger)
{
	u8 reg;
	int val;
	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg);

	if (reg & (0x1 << 0) || /* check: VBUS_POK */
		reg & (0x1 << 4)) { /* check: WPC_POK */
		val = 1;
	} else {
		val = 0;
	}

	return val;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_charging_current(struct sm5038_charger_data *charger, int mA)
{
	u8 offset;
	int uA;

	uA = mA * 1000;

	mutex_lock(&charger->charger_mutex);
	if (uA < 109375) {			// 109.375 mA
		offset = 0x07;
	} else if (uA > 3500000) {	//	3500.000 mA
		offset = 0xE0;
	} else {
		offset = (7 + ((uA - 109375) / 15625)) & 0xFF;
	}

	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL2, (offset << 0), (0xFF << 0));
	charger->charging_current = mA;

	mutex_unlock(&charger->charger_mutex);
}
#endif

static int chg_get_otg_current(struct sm5038_charger_data *charger)
{
	u8 reg, offset;
	int otg_curr_ma;


	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_BSTCNTL1, &reg);
	offset = (reg & 0xC0) >> 6;

	if (offset > OTG_CURRENT_1500mA)
		offset = OTG_CURRENT_1500mA;

	otg_curr_ma = (offset == OTG_CURRENT_500mA) ? 500 : (600 + (offset * 300));

	return otg_curr_ma;
}

static int chg_get_charging_current(struct sm5038_charger_data *charger)
{
	u8 reg;
	int fast_curr_ua;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL2, &reg);

	if ((reg & 0xFF) <= 0x07) {
		fast_curr_ua = 109000;
	} else if ((reg & 0xFF) >= 0xE0) {
		fast_curr_ua = 3500000;
	} else {
		fast_curr_ua = 109375 + ((reg & 0xFF) - 7) * 15625;
	}

	return (fast_curr_ua / 1000);
}



static void psy_chg_set_online(struct sm5038_charger_data *charger, int cable_type)
{
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	charger->proprietary_charger_detected = false;

	switch (cable_type) {
	case POWER_SUPPLY_TYPE_APPLE_BRICK_ID:
		charger->proprietary_charger_detected = true;
		cable_type = POWER_SUPPLY_TYPE_USB_DCP;
		break;
	case SOMC_POWER_SUPPLY_EXT_TYPE_RP_NONE:
	case SOMC_POWER_SUPPLY_EXT_TYPE_RP_DEFAULT:
		charger->typec_current = TYPEC_CURRENT_DEFAULT;
		cable_type = charger->cable_type;
		break;
	case SOMC_POWER_SUPPLY_EXT_TYPE_RP_1_5A:
		charger->typec_current = TYPEC_CURRENT_1_5A;
		cable_type = charger->cable_type;
		break;
	case SOMC_POWER_SUPPLY_EXT_TYPE_RP_3_0A:
		charger->typec_current = TYPEC_CURRENT_3_0A;
		cable_type = charger->cable_type;
		break;
	}
#endif
	pr_info("sm5038-charger: [start] cable_type(%d->%d), op_mode(%d), op_status(0x%x)",
			charger->cable_type, cable_type, sm5038_charger_oper_get_current_op_mode(),
			sm5038_charger_oper_get_current_status());

	charger->slow_rate_chg_mode = false;
	charger->cable_type = cable_type;
	sm5038_charger_cable_type_update(charger->cable_type);

	if (charger->cable_type == POWER_SUPPLY_TYPE_UNKNOWN ||
			charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_VBUSIN, 0);
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_WPCIN, 0);
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_USB_OTG, 0);

		/* set default input current */
		chg_set_input_current_limit(charger, NONE_CABLE_CURRENT);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		vote(charger->usb_icl_votable, CABLE_TYPE_VOTER, true,
							NONE_CABLE_CURRENT);
		vote(charger->usb_icl_votable, USB_PSY_VOTER, false, 0);
		vote(charger->usb_icl_votable, PD_VOTER, false, 0);
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_USB_OTG,
									false);

		if (is_client_vote_enabled(charger->fcc_votable,
							SAFETY_TIMER_VOTER))
			vote(charger->fcc_votable, SAFETY_TIMER_VOTER, false,
									0);
#endif

		if (charger->irq_aicl_enabled == 0) {
			u8 reg_data;
			charger->irq_aicl_enabled = 1;
			enable_irq(charger->irq_aicl);
			sm5038_read_reg(charger->i2c, SM5038_CHG_REG_INTMSK2, &reg_data);
			pr_info("sm5038-charger: %s: enable aicl(0x%X)\n", __func__, reg_data);
		}

		charger->is_input_control_used = false;
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		charger->fasttimer_expired = false;

		cancel_delayed_work_sync(&charger->rechg_work);
		if (charger->sw_ovp_enable)
			charger->sw_ovp_enable = false;
		cancel_delayed_work_sync(&charger->wa_sw_ovp_work);
		chg_set_thermal_status(charger, TEMP_SHUT_DOWN_SM5038_CHG_OFF);
		psy_chg_set_charging_enable(charger, SM5038_CHG_MODE_CHARGING_OFF);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		cancel_delayed_work_sync(&charger->somc_full_work);
		schedule_delayed_work(&charger->somc_full_work, 0);
#endif
	} else {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (is_wireless_type(charger->cable_type)) {
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_WPCIN, 1);
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_VBUSIN, 0);
		} else  {
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_VBUSIN, 1);
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_WPCIN, 0);
		}
#endif


		if (charger->cable_type == POWER_SUPPLY_TYPE_USB_PD)
			charger->present_is_pd_apdapter = 1;
		else
			charger->present_is_pd_apdapter = 0;

		/* set vbuslimit and fast charging current according to cable_type */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_chg_set_fast_charging_current_by_type(charger, charger->cable_type);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (charger->cable_type == POWER_SUPPLY_TYPE_USB) {
			if (!is_client_vote_enabled(charger->usb_icl_votable,
							USB_PSY_VOTER)) {
				vote(charger->usb_icl_votable, USB_PSY_VOTER,
						true, NONE_CABLE_CURRENT);
				vote(charger->usb_icl_votable, PD_VOTER,
								false, 0);
			}
		} else if (charger->cable_type == POWER_SUPPLY_TYPE_USB_PD) {
			if (!is_client_vote_enabled(charger->usb_icl_votable,
								PD_VOTER)) {
				vote(charger->usb_icl_votable, PD_VOTER,
							true, MICRO_CURR_0P5A);
				vote(charger->usb_icl_votable, USB_PSY_VOTER,
								false, 0);
			}
		}
#endif
		sm5038_chg_set_vbuslimit_current_by_type(charger, charger->cable_type);

		if (is_hv_wire_type(charger->cable_type)) {
			if (charger->irq_aicl_enabled == 1) {
				u8 reg_data;

				charger->irq_aicl_enabled = 0;
				disable_irq_nosync(charger->irq_aicl);
				cancel_delayed_work_sync(&charger->aicl_work);
				__pm_relax(charger->aicl_wake_lock);
				sm5038_read_reg(charger->i2c, SM5038_CHG_REG_INTMSK2, &reg_data);
				pr_info("sm5038-charger: %s: disable aicl(0x%X)\n", __func__, reg_data);
				charger->slow_rate_chg_mode = false;
			}
		}
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		cancel_delayed_work_sync(&charger->thermal_regulation_work);
		schedule_delayed_work(&charger->thermal_regulation_work, 0);
#endif

		schedule_delayed_work(&charger->wa_sw_ovp_work, 0);
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if(charger->psy_chg)
		power_supply_changed(charger->psy_chg);
#endif

	pr_info("sm5038-charger: [end] op_mode(%d), op_status(0x%x), pd_apdapter = %d\n",
			sm5038_charger_oper_get_current_op_mode(),
			sm5038_charger_oper_get_current_status(), charger->present_is_pd_apdapter);
}


static int psy_chg_get_vbuslimit_status(struct sm5038_charger_data *charger)
{
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	u8 reg_st1;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg_st1);

	pr_info("sm5038-charger: %s: STATUS1(0x%02x)\n", __func__, reg_st1);

	if ((reg_st1 & (0x1 << 7)) || (reg_st1 & (0x1 << 4))) { /* wpcinlimit or vbuslimit */
		status = 1;
	} else {
		status = 0;
	}

	return status;
}

static int psy_chg_get_health(struct sm5038_charger_data *charger)
{
	u8 reg;
	int offset;
	int health = POWER_SUPPLY_HEALTH_UNKNOWN;

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
/*
	if (charger->is_charging) {
		chg_set_wdt_tmr_reset(charger);
	}
*/
#endif	
	// chg_print_regmap(charger);  /* please keep this log message */

	/* when sw ocp ouccred, charging UI(lightning sign) disable */
	if (charger->sw_ovp_enable) {
		return POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	}

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg);

	if (is_wireless_type(charger->cable_type)) {
		offset = 4;
	} else {
		offset = 0;
	}

	if (reg & (0x1 << (0 + offset))) {
		charger->unhealth_cnt = 0;
		health = POWER_SUPPLY_HEALTH_GOOD;
	} else {
		if (charger->unhealth_cnt < HEALTH_DEBOUNCE_CNT) {
			health = POWER_SUPPLY_HEALTH_GOOD;
			charger->unhealth_cnt++;
		} else {
			if (reg & (0x1 << (2 + offset))) {
				health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			}
		}
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (health != POWER_SUPPLY_HEALTH_OVERVOLTAGE) {
		switch (charger->jeita_step_condition) {
		case JEITA_STEP_COND_COLD:
			health = POWER_SUPPLY_HEALTH_COLD;
			break;
		case JEITA_STEP_COND_COOL:
			health = POWER_SUPPLY_HEALTH_COOL;
			break;
		case JEITA_STEP_COND_NORMAL:
			health = POWER_SUPPLY_HEALTH_GOOD;
			break;
		case JEITA_STEP_COND_WARM:
			health = POWER_SUPPLY_HEALTH_WARM;
			break;
		case JEITA_STEP_COND_HOT:
			health = POWER_SUPPLY_HEALTH_OVERHEAT;
			break;
		}
	}

	if (charger->is_invalid_batt_id)
		health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
#endif

	return health;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void sm5038_chg_set_fast_charging_current_by_type(struct sm5038_charger_data *charger, int cable_type)
{
	int fast_charging_current_ma = 0;

	switch (cable_type) {
	case POWER_SUPPLY_TYPE_USB:
		fast_charging_current_ma = MICRO_CURR_2P0A;
		break;
	case POWER_SUPPLY_TYPE_USB_DCP:
		fast_charging_current_ma = MICRO_CURR_2P0A;
		break;
	case POWER_SUPPLY_TYPE_USB_CDP:
		fast_charging_current_ma = MICRO_CURR_2P0A;
		break;
	case POWER_SUPPLY_TYPE_APPLE_BRICK_ID:
		fast_charging_current_ma = MICRO_CURR_2P5A;
		break;
	case POWER_SUPPLY_TYPE_USB_HVDCP:
	case POWER_SUPPLY_TYPE_USB_HVDCP_3:
	case POWER_SUPPLY_TYPE_USB_HVDCP_3P5:
	case POWER_SUPPLY_TYPE_USB_PD:
		fast_charging_current_ma = MICRO_CURR_3P0A;
		break;
	case POWER_SUPPLY_TYPE_USB_FLOAT:
		fast_charging_current_ma = MICRO_CURR_1P0A;
		break;
	case POWER_SUPPLY_TYPE_UNKNOWN:
	case POWER_SUPPLY_TYPE_BATTERY:
	default:
		fast_charging_current_ma = MICRO_CURR_1P5A;
		break;
	}

	chg_set_charging_current(charger, fast_charging_current_ma);
	pr_info("sm5038-charger: %s: charging_current = %d mA\n", __func__, fast_charging_current_ma);

}
#endif
static void sm5038_chg_set_vbuslimit_current_by_type(struct sm5038_charger_data *charger, int cable_type)
{
	int input_current_ma = 0;

	switch (cable_type) {
	case POWER_SUPPLY_TYPE_USB:
		input_current_ma = MICRO_CURR_0P5A;
		break;
	case POWER_SUPPLY_TYPE_USB_DCP:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		input_current_ma = MICRO_CURR_1P5A;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (charger->typec_current == TYPEC_CURRENT_3_0A)
			input_current_ma = MICRO_CURR_1P5A;
		else if (charger->typec_current == TYPEC_CURRENT_1_5A)
			input_current_ma = MICRO_CURR_1P5A;
		else if (charger->proprietary_charger_detected)
			input_current_ma = MICRO_CURR_1P0A;
		else
			input_current_ma = MICRO_CURR_1P5A;
#endif
		break;

	case POWER_SUPPLY_TYPE_USB_CDP:
		input_current_ma = MICRO_CURR_1P5A;
		break;

	case POWER_SUPPLY_TYPE_APPLE_BRICK_ID:
		input_current_ma = MICRO_CURR_1P5A;
		break;

	case POWER_SUPPLY_TYPE_USB_HVDCP:
	case POWER_SUPPLY_TYPE_USB_HVDCP_3:
	case POWER_SUPPLY_TYPE_USB_HVDCP_3P5:
		input_current_ma = MICRO_CURR_1P5A;
		break;
	case POWER_SUPPLY_TYPE_USB_PD:
		input_current_ma = MICRO_CURR_3P0A;
		break;

	case POWER_SUPPLY_TYPE_USB_FLOAT:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		input_current_ma = MICRO_CURR_1P0A;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (charger->typec_current == TYPEC_CURRENT_3_0A)
			input_current_ma = MICRO_CURR_1P5A;
		else if (charger->typec_current == TYPEC_CURRENT_1_5A)
			input_current_ma = MICRO_CURR_1P5A;
		else
			input_current_ma = MICRO_CURR_0P5A;
#endif
		break;

	case POWER_SUPPLY_TYPE_UNKNOWN:
	case POWER_SUPPLY_TYPE_BATTERY:
	default:
		input_current_ma = NONE_CABLE_CURRENT;
		break;

	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chg_set_input_current_limit(charger, input_current_ma);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->usb_icl_votable, CABLE_TYPE_VOTER, true,
							input_current_ma);
#endif
	pr_info("sm5038-charger: %s: input_current = %d mA\n", __func__, input_current_ma);
}

static int sm5038_chg_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct sm5038_charger_data *charger =
		power_supply_get_drvdata(psy);
	int ma = 0;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int capacity = 0, charge_full = 0;
#endif
	char temp_buf[500] = {0, };
	enum power_supply_ext_property ext_psp = (enum power_supply_ext_property) psp;

	sprintf(temp_buf, "sm5038-charger: %s: psp=%d, ", __func__, psp);

	switch ((int)psp) {
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = psy_chg_get_charge_type(charger);
		pr_debug("%s POWER_SUPPLY_CHARGE_TYPE is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = psy_chg_get_status(charger);
//		pr_info("%s POWER_SUPPLY_STATUS get is [%d] \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = psy_chg_get_health(charger);
//		pr_info("%s  POWER_SUPPLY_HEALTH get is [%d] \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX: /* get input current which was set */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		ma = charger->input_current;
		val->intval = (ma * 1000);
		pr_debug("%s input current get as %d mA \n", temp_buf, ma);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (is_client_vote_enabled(charger->usb_icl_votable,
								USB_PSY_VOTER))
			val->intval = get_client_vote(
				charger->usb_icl_votable, USB_PSY_VOTER) * 1000;
		else
			val->intval = 0;
#endif
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT: /* get charge current which was read */
		ma = chg_get_charging_current(charger);
		val->intval = (ma * 1000);	// uA
		pr_debug("%s fast charging current get as %d mA\n", temp_buf, ma);
		break;
	case POWER_SUPPLY_PROP_PRESENT: /* USB PART */
		val->intval = psy_chg_get_present(charger);
		pr_debug("%s  Valid_VBUS is %d \n", temp_buf, val->intval);
		val->intval = 1;
		break;

	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = charger->system_temp_level;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = 0;
#endif
		pr_debug("%s system_temp_level get is [%d] \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = charger->thermal_levels;
		pr_debug("%s thermal_levels get max is [%d] \n", temp_buf, val->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = 0;
#endif
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 1;
//		pr_info("%s ONLINE get is [%d] \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		if (charger->charge_mode == SM5038_CHG_MODE_BUCK_OFF)
			val->intval = 1;
		else if (charger->charge_mode == SM5038_CHG_MODE_CHARGING_ON)
			val->intval = 0;
		pr_debug("%s input_current_limit get is [%s] \n", temp_buf,
			(val->intval ? "SUSPEND" : "CHG_ON"));
		break;
	case POWER_SUPPLY_PROP_TEMP:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (charger->real_temp_debug) {
			val->intval = charger->real_temp_debug;
			break;
		}
#endif
		sm5038_get_batt_therm(&charger->batt_therm);
		val->intval = charger->batt_therm;
		pr_debug("%s POWER_SUPPLY_PROP_TEMP get is [%d] \n", temp_buf, val->intval);
		break;
	/* ================= fuel gague ========================= */
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
/*		pr_info("%s POWER_SUPPLY_TECHNOLOGY_LION is %d \n", temp_buf, val->intval);	*/
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = sm5038_fg_get_prop_cycle();
		pr_debug("%s POWER_SUPPLY_PROP_CYCLE_COUNT is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = sm5038_fg_get_prop_vbat_max();
//		pr_info("%s POWER_SUPPLY_PROP_VOLTAGE_MAX is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		val->intval = sm5038_fg_get_prop_vbat_min();
//		pr_info("%s POWER_SUPPLY_PROP_VOLTAGE_MIN is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = sm5038_fg_get_prop_init_vbat_max();
		pr_debug("%s POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = sm5038_fg_get_prop_init_vbat_min();
		pr_debug("%s POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = sm5038_fg_get_prop_vbat_now();
//		pr_info("%s POWER_SUPPLY_PROP_VOLTAGE_NOW is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		val->intval = sm5038_fg_get_prop_vbat_avg();
		pr_debug("%s POWER_SUPPLY_PROP_VOLTAGE_AVG is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		val->intval = sm5038_fg_get_prop_ocv();
		pr_info("%s POWER_SUPPLY_PROP_VOLTAGE_OCV is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = sm5038_fg_get_prop_soc();
//		pr_debug("%s POWER_SUPPLY_PROP_CAPACITY is %d \n", temp_buf, val->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_get_capacity(charger, &val->intval);
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = sm5038_fg_get_prop_q_max_design();
		pr_debug("%s POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN is %d \n", temp_buf, val->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = somc_sm5038_get_charge_full_design(charger);
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = sm5038_fg_get_prop_q_max();
		pr_debug("%s POWER_SUPPLY_PROP_CHARGE_FULL is %d \n", temp_buf, val->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = somc_sm5038_get_charge_full(charger);
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = sm5038_fg_get_prop_q_now();
		pr_debug("%s POWER_SUPPLY_PROP_CHARGE_NOW is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		val->intval = sm5038_fg_get_prop_soc_cycle();
//		pr_info("%s POWER_SUPPLY_PROP_CHARGE_COUNTER is %d \n", temp_buf, val->intval);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_get_capacity(charger, &capacity);
		charge_full = somc_sm5038_get_charge_full(charger);
		val->intval = charge_full * capacity / 100;
#endif
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		val->intval = 100;
//		pr_info("%s POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		val->intval = sm5038_fg_get_prop_soh();
		pr_debug("%s POWER_SUPPLY_PROP_ENERGY_FULL is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP_MIN:
		val->intval = sm5038_fg_get_prop_temp_min();
//		pr_info("%s POWER_SUPPLY_PROP_TEMP_MIN is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP_MAX:
		val->intval = sm5038_fg_get_prop_temp_max();
//		pr_info("%s POWER_SUPPLY_PROP_TEMP_MAX is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = POWER_SUPPLY_SCOPE_SYSTEM;
/*		pr_info("%s POWER_SUPPLY_PROP_SCOPE is %d \n", temp_buf, val->intval);	*/
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = sm5038_fg_get_prop_current_now();
//		pr_info("%s POWER_SUPPLY_PROP_CURRENT_NOW is %d \n", temp_buf, val->intval);
		break;
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = sm5038_fg_get_prop_current_avg();
		pr_debug("%s POWER_SUPPLY_PROP_CURRENT_AVG is %d \n", temp_buf, val->intval);
		break;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = "MURRAY_BATTERY";
		break;
#endif
	case POWER_SUPPLY_EXT_PROP_MIN ... POWER_SUPPLY_EXT_PROP_MAX:
		switch (ext_psp) {
		case POWER_SUPPLY_EXT_PROP_SW_JEITA_ENABLED:
			val->intval = charger->sw_jeita_enabled;
			pr_debug("%s sw_jeita_enabled is %d \n", temp_buf, val->intval);
			break;
		case POWER_SUPPLY_EXT_PROP_BOOST_CURRENT:
			ma = chg_get_otg_current(charger);
			val->intval = (ma * 1000);
			pr_debug("%s otg current is %d mA\n", temp_buf, ma);
			break;
		case POWER_SUPPLY_EXT_PROP_INPUT_CURRENT_SETTLED:
			val->intval = psy_chg_get_vbuslimit_status(charger);
			pr_debug("%s vbuslimit status is %d mA\n", temp_buf, val->intval);
			break;
		case POWER_SUPPLY_EXT_PROP_FCC_STEPPER_ENABLE:
			val->intval = charger->fcc_stepper_enable;
			pr_debug("%s fcc_stepper_enable is %d mA\n", temp_buf, val->intval);
			break;
		case POWER_SUPPLY_EXT_PROP_PD_CURRENT_MAX:
			/* TODO */
		break;
		default:
			return -EINVAL;
		}

	default:
		return -EINVAL;
	}

	return 0;
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void sm5038_chg_buck_control(struct sm5038_charger_data *charger, bool buck_on)
{

	if (buck_on) {
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_SUSPEND, 0);
	} else {
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_SUSPEND, 1);
	}

	pr_info("sm5038-charger: %s: buck status(%d)\n", __func__, buck_on);

	return;
}

static void sm5038_chg_charging(struct sm5038_charger_data *charger, int chg_en)
{
	u8 reg;

	if (chg_en) {
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);
		if (reg & 0x80) { /* reset wdt expired status and re-init wdt */
			chg_set_wdtcntl_reset(charger);
			chg_set_wdt_timer(charger, WDT_TIME_S_90);
		}
	}

	chg_set_enq4fet(charger, chg_en);
	chg_set_wdt_enable(charger, chg_en);

	pr_info("sm5038-charger: %s: charging en(%d)\n", __func__, chg_en);
	return;

}

static void psy_chg_set_charging_enable(struct sm5038_charger_data *charger, int charge_mode)
{
	int buck_off = false;
	bool buck_off_status = (sm5038_charger_oper_get_current_status() & (0x1 << SM5038_CHARGER_OP_EVENT_SUSPEND)) ? 1 : 0;

	pr_info("sm5038-charger: charge_mode[%s -> %s]\n", charge_mode_str[charger->charge_mode], charge_mode_str[charge_mode]);

//	pr_info("sm5038-charger: charger_mode changed [%d] -> [%d]\n", charger->charge_mode, charge_mode);
	charger->charge_mode = charge_mode;
	sm5038_charger_charge_mode_update(charger->charge_mode);

	switch (charger->charge_mode) {
	case SM5038_CHG_MODE_BUCK_OFF:	/* BUCK-OFF */
		buck_off = true;
	case SM5038_CHG_MODE_CHARGING_OFF:	/* CHG-OFF */
		charger->is_charging = false;
		break;
	case SM5038_CHG_MODE_CHARGING_ON: /* CHG-ON */
		charger->is_charging = true;
		break;
	}

	sm5038_chg_charging(charger, charger->is_charging);

	if (buck_off != buck_off_status) {
		sm5038_chg_buck_control(charger, (!buck_off));
	}

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);
}
#endif

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
#ifdef CHARGE_SCREEN_ON_OFF /* debug screen_on/screen_off */
#define BACKLIGHT_NAME "/sys/class/backlight/panel0-backlight/brightness"
int sm5038_get_screen_state(void)
{
	struct file *pfile = NULL;
	//mm_segment_t old_fs;
	loff_t pos;

	ssize_t ret = 0;
	char brightness[10] = {0,};
	memset(brightness, 0, sizeof(brightness));

	pfile = filp_open(BACKLIGHT_NAME, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_info("open BACKLIGHT_NAME  file failed!\n");
		goto ERR_0;
	}

	//old_fs = get_fs();
	//set_fs(KERNEL_DS);
	pos = 0;

	ret = kernel_read(pfile, brightness, 10, &pos);
	if(ret <= 0) {
		pr_info("read BACKLIGHT_NAME  file failed!\n");
		goto ERR_1;
	}

	if(brightness[0] == '0') {
		screen_state = 0;
	} else {
		screen_state = 1;
	}
	pr_info("brightness = %s\n", brightness);

ERR_1:
	filp_close(pfile, NULL);
	//set_fs(old_fs);
	return 0;

ERR_0:
	return -1;
}
#endif
#endif

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
int psy_chg_set_system_temp_level(struct sm5038_charger_data *charger,
				const union power_supply_propval *val)
{
	int ma_by_temp_level;

	if (val->intval < 0)
		return -EINVAL;

	if (charger->thermal_levels <= 0)
		return -EINVAL;

	if (val->intval > charger->thermal_levels)
		return -EINVAL;

	charger->system_temp_level = val->intval;

	/* if system_Temp_level is max then charging off */
	if (charger->system_temp_level == charger->thermal_levels) {
		chg_set_thermal_status(charger, TEMP_SHUT_DOWN_SM5038_CHG_OFF);
		psy_chg_set_charging_enable(charger, SM5038_CHG_MODE_CHARGING_OFF);
		return 0;
	} 


	if (charger->charge_mode != SM5038_CHG_MODE_CHARGING_ON) {
		schedule_delayed_work(&charger->thermal_regulation_work, 0);	
/*
		chg_set_thermal_status(charger, TEMP_WITHIN_RANGE);
		psy_chg_set_charging_enable(charger, SM5038_CHG_MODE_CHARGING_ON);		
*/
	}

#ifdef CHARGE_SCREEN_ON_OFF /* debug screen_on/screen_off */
	sm5038_get_screen_state();
#endif

	if (screen_state == 1) {
		ma_by_temp_level = (charger->thermal_mitigation[charger->system_temp_level] / 1000);
	} else {
		ma_by_temp_level = (charger->thermal_mitigation_sleep[charger->system_temp_level] / 1000);
	}
	pr_info("sm5038-charger: %s: screen_state = %d, set fcc [%d -> %d] mA\n", __func__, screen_state, charger->charging_current, ma_by_temp_level);
	chg_set_charging_current(charger, ma_by_temp_level);

	return 0;
}
#endif

static void psy_chg_set_otg_control(struct sm5038_charger_data *charger, bool enable)
{

	if (enable == charger->otg_on) {
		return;
	}

	sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_USB_OTG, enable);
	charger->otg_on = enable;

	if (charger->psy_otg)
		power_supply_changed(charger->psy_otg);
}

/* according to the setting to
 * stop  charging when set to 1 : buck_off mode,
 * and
 * start charging when set to 0 : chg_on & buck_on mode.
 * water relese   when set to 2 : chg_off & buck_on mode.
 */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void psy_chg_set_input_control(struct sm5038_charger_data *charger, int input_control)
{
	int chg_mode = 0, thermal = 0;
	if (input_control == 1) {
		chg_mode = SM5038_CHG_MODE_BUCK_OFF;
		thermal = TEMP_SHUT_DOWN_SM5038_BUCK_OFF;
	} else if (input_control == 0) {
		chg_mode = SM5038_CHG_MODE_CHARGING_ON;
		thermal = TEMP_WITHIN_RANGE;
	} else {
		chg_mode = SM5038_CHG_MODE_CHARGING_OFF;
		thermal = TEMP_SHUT_DOWN_SM5038_CHG_OFF;
	}
	chg_set_thermal_status(charger, thermal);
	psy_chg_set_charging_enable(charger, chg_mode);

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);
}
#endif
static void psy_chg_set_charging_current(struct sm5038_charger_data *charger, int mA)
{
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int system_temp_fcc_ma = 0;
#endif

	if (mA > 3500 || mA < 0) {
		pr_err("sm5038-charger: %s: invaild current = %d mA , so keep current (%d mA)\n", __func__, mA, charger->charging_current);
		return;
	}
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (charger->thermal_mitigation != NULL) {
		if (screen_state == 1) {
			system_temp_fcc_ma = (charger->thermal_mitigation[charger->system_temp_level] / 1000);
		} else {
			system_temp_fcc_ma = (charger->thermal_mitigation_sleep[charger->system_temp_level] / 1000);
		}
//		pr_info("sm5038-charger: %s: fcc = [%dmA] by system_temp_level \n", __func__, system_temp_fcc_ma, charger->system_temp_level);

		/* compare system_temp_fcc to input fcc, set lower then current */
		if (system_temp_fcc_ma < mA) {
			pr_info("sm5038-charger: %s: becuase system_temp_level, fcc changed [%dmA]\n", __func__, system_temp_fcc_ma);		
			mA = system_temp_fcc_ma;
		}
	}

	if (mA != charger->charging_current) {
		chg_set_charging_current(charger, mA);
	} 
	/*
	else {
		pr_info("sm5038-charger: %s: charging_current_not_changed, becuase same [%dmA]\n", __func__,
			charger->charging_current);
	}
	*/
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->fcc_votable, DEFAULT_VOTER, true, mA);
#endif

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);
}

static int sm5038_chg_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct sm5038_charger_data *charger =
		power_supply_get_drvdata(psy);
	char temp_buf[500] = {0, };
	enum power_supply_ext_property ext_psp = (enum power_supply_ext_property) psp;
	int ma = 0;

	sprintf(temp_buf, "sm5038-charger: %s: psp=%d, ", __func__, psp);

	switch ((int)psp) {
	case POWER_SUPPLY_PROP_STATUS:
		// charger->status = val->intval;
		pr_info("%s POWER_SUPPLY_PROP_STATUS is %d \n", temp_buf, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		ma = (val->intval * 1000);
		pr_info("%s: input limit changed [%dmA] -> [%dmA]\n", temp_buf,
			charger->input_current, ma);
		chg_set_input_current_limit(charger, ma);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (is_client_vote_enabled(charger->usb_icl_votable,
							USB_PSY_VOTER)) {
			ma = val->intval / 1000;
			vote(charger->usb_icl_votable, USB_PSY_VOTER, true, ma);
		}
#endif
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		pr_info("%s: charging_current_changed [%dmA] -> [%dmA]\n", temp_buf,
				charger->charging_current, val->intval);
		psy_chg_set_charging_current(charger, val->intval);

		break;
/*
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE_MAX:
		mv = val->intval;
		if (mv <= 0) {
			pr_info("%s: float voltage no changed [%dmV]\n", temp_buf, mv);
			break;
		}
		pr_info("%s: float voltage changed [%dmV] -> [%dmV]\n", temp_buf,
			charger->pdata->chg_float_voltage, mv);
		charger->float_voltage = mv;
		chg_set_batreg(charger, charger->float_voltage);
		break;
*/

	case POWER_SUPPLY_PROP_ONLINE:
		if (charger->cable_type == POWER_SUPPLY_TYPE_USB_PD) {	/* current cable_type */
			if (val->intval == POWER_SUPPLY_TYPE_USB_DCP) {		/* insert  cable_type */
				pr_info("sm5038-charger:  %s: already PD connected, so DCP online skip\n", __func__);
				break;
			}
		}
	
		if (charger->otg_on == true) {
			pr_info("sm5038-charger: %s: OTG_ON states, so No changed CHG_MODE \n", __func__);
			break;
		}

		if (val->intval == 0) {
			pr_info("sm5038-charger: %s: invalid int value \n", __func__);
			break;
		}
			
		pr_info("%s: POWER_SUPPLY_PROP_ONLINE is [%d -> %d]\n", temp_buf, charger->cable_type, val->intval);
		psy_chg_set_online(charger, val->intval);
		break;

	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		pr_info("%s: thermal_levels(%d), system_temp_level(%d -> %d) \n", temp_buf,
			charger->thermal_levels, charger->system_temp_level, val->intval);
		psy_chg_set_system_temp_level(charger, val);
#endif
		break;

	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		pr_info("%s Charge mode is set [%d] \n", temp_buf, val->intval);
		if (val->intval == 1) {
			charger->is_input_control_used = true;
		} else {
			charger->is_input_control_used = false;
		}
		psy_chg_set_input_control(charger, val->intval);
#endif
		break;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		charger->restored_charge_full = val->intval;
		break;

#endif
	case POWER_SUPPLY_EXT_PROP_MIN ... POWER_SUPPLY_EXT_PROP_MAX:
		switch (ext_psp) {
		case POWER_SUPPLY_EXT_PROP_BOOST_CURRENT:
		{
			int otg_curr_ma = (val->intval / 1000);
			u8 otg_curr_offset;
			if (otg_curr_ma < 900)
				otg_curr_offset = OTG_CURRENT_500mA;
			else if (otg_curr_ma < 1200)
				otg_curr_offset = OTG_CURRENT_900mA;
			else if (otg_curr_ma < 1500)
				otg_curr_offset = OTG_CURRENT_1200mA;
			else
				otg_curr_offset = OTG_CURRENT_1500mA;

			sm5038_update_reg(charger->i2c, SM5038_CHG_REG_BSTCNTL1, (otg_curr_offset << 6), (0x3 << 6));
			pr_info("%s: otg current changed [%dmA]\n", temp_buf, otg_curr_ma);
		}
		break;

		case POWER_SUPPLY_EXT_PROP_FCC_STEPPER_ENABLE:
			pr_info("%s: fcc_stepper_enable is %d \n", temp_buf, val->intval);
			charger->fcc_stepper_enable = val->intval;
			break;

		case POWER_SUPPLY_EXT_PROP_PD_CURRENT_MAX:
			/* TODO */
			break;

		case POWER_SUPPLY_EXT_PROP_RERUN_AICL:
		{
			u8 reg;
			sm5038_charger_enable_aicl_irq(charger);
			sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);
			if (reg & (0x1 << 0))
				queue_delayed_work(charger->wqueue, &charger->aicl_work, msecs_to_jiffies(AICL_WORK_DELAY));
		}
			break;

		case POWER_SUPPLY_EXT_PROP_FORCE_RECHARGE:
			/* Done : Toggle ENQ4FET for Re-cycling charger loop */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
			chg_set_enq4fet(charger, 0);
			msleep(10);
			chg_set_enq4fet(charger, 1);
#endif

			break;

		default:
			return -EINVAL;

		}

	default:
		return -EINVAL;
	}

	return 0;
}

static int sm5038_otg_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sm5038_charger_data *charger =
		power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = charger->otg_on;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sm5038_otg_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct sm5038_charger_data *charger =
		power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		pr_info("%s: OTG %s\n", __func__,
			val->intval ? "ON" : "OFF");
		psy_chg_set_otg_control(charger, val->intval);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static inline u8 _calc_limit_current_offset_to_mA(unsigned short mA)
{
	unsigned char offset;

	if (mA < 100) {
		offset = 0x00;
	} else {
		offset = ((mA - 100) / 25) & 0x7F;
	}

	return offset;
}

static inline int _reduce_input_limit_current(struct sm5038_charger_data *charger)
{
	int input_limit = chg_get_input_current_limit(charger);
	u8 offset = _calc_limit_current_offset_to_mA(input_limit - REDUCE_CURRENT_STEP);

	if (is_wireless_type(charger->cable_type)) {
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WPCINCNTL2, ((offset & 0x7F) << 0), (0x7F << 0));
	} else {
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_VBUSCNTL, ((offset & 0x7F) << 0), (0x7F << 0));
	}
	charger->input_current = chg_get_input_current_limit(charger);

	pr_info("sm5038-charger: reduce input-limit: [%dmA] to [%dmA]\n",
			input_limit, charger->input_current);

	return charger->input_current;
}

static inline void _check_slow_rate_charging(struct sm5038_charger_data *charger)
{
	union power_supply_propval value;

	if (charger->input_current <= SLOW_CHARGING_CURRENT_STANDARD &&
			charger->cable_type != POWER_SUPPLY_TYPE_UNKNOWN) {

		pr_info("sm5038-charger: slow-rate charging on : input current(%dmA), cable-type(%d)\n",
			charger->input_current, charger->cable_type);

		charger->slow_rate_chg_mode = true;
		value.intval = POWER_SUPPLY_CHARGE_TYPE_ADAPTIVE;
//		psy_do_property("battery", set, POWER_SUPPLY_PROP_CHARGE_TYPE, value);
	} else {
		charger->slow_rate_chg_mode = false;
	}
	pr_info("sm5038-charger: slow-rate charging done\n");
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void auto_rechg_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work, struct sm5038_charger_data, rechg_work.work);

	int raw_soc = 0, charging_status = 0;
	bool rechg_ready_ok = false;

	/* 1. check charger status */
//	charging_status = psy_chg_get_status(charger);
	if (charger->charging_status != POWER_SUPPLY_STATUS_FULL) {
		pr_info("sm5038-charger: %s: not full-charging status(%d) \n", __func__, charging_status);
		goto reschedule;
	}

	/* 2. check cable_type */
	if (charger->cable_type == POWER_SUPPLY_TYPE_UNKNOWN || charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
		pr_info("sm5038-charger: %s: not attach cable_type, currently(%d) \n", __func__, charger->cable_type);
		goto reschedule;
	}

	/* 3. check charge mode status */
	if (charger->charge_mode != SM5038_CHG_MODE_CHARGING_OFF) {
		pr_info("sm5038-charger: %s: not charging-off STATUS currently(%d) \n", __func__, charger->charge_mode);
		goto reschedule;
	}

	/* 4. check fg available */
	raw_soc = (sm5038_fg_get_prop_rsoc() * 10);
	pr_info("sm5038-charger: %s: fg raw_soc = %d \n", __func__, raw_soc);

	/* 5. check soc value : soc < 95.0% */
	if (raw_soc <= charger->auto_rechg_soc) {
		rechg_ready_ok = true;
		pr_info("sm5038-charger: %s: All conditions are satisfied.\n", __func__, raw_soc);
	}

	if (rechg_ready_ok) {
		cancel_delayed_work(&charger->thermal_regulation_work);
		schedule_delayed_work(&charger->thermal_regulation_work, 0);
	}

	pr_info("sm5038-charger: %s: re-charging %s \n", __func__, (rechg_ready_ok ? "start":"already"));

	return;

reschedule:
//	chg_print_regmap(charger);
	schedule_delayed_work(&charger->rechg_work, msecs_to_jiffies(RECHG_WORK_DELAY));

}

#endif
static void aicl_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work, struct sm5038_charger_data, aicl_work.work);
	int input_limit;
	bool aicl_on = false;
	u8 reg, aicl_cnt = 0;

	pr_info("sm5038-charger: %s - start\n", __func__);

	mutex_lock(&charger->charger_mutex);
	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);
	while ((reg & (0x1 << 0)) && charger->cable_type != POWER_SUPPLY_TYPE_UNKNOWN) {
		if (++aicl_cnt >= 2) {
			input_limit = _reduce_input_limit_current(charger);
			aicl_on = true;
			if (input_limit <= MINIMUM_INPUT_CURRENT) {
				break;
			}
			aicl_cnt = 0;
		}
		msleep(50);
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);
	}

	mutex_unlock(&charger->charger_mutex);

	pr_info("sm5038-charger: %s - aicl_on(%d)\n", __func__, aicl_on);
#if 1
	if (aicl_on) {
		if (charger->psy_chg)
			power_supply_changed(charger->psy_chg);
	}
#endif
	_check_slow_rate_charging(charger);
	__pm_relax(charger->aicl_wake_lock);

	pr_info("sm5038-charger: %s - done\n", __func__);
}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int sm5038_update_thermal_readings(struct sm5038_charger_data *charger)
{
	int ret = 0;

	ret = sm5038_get_batt_id_ohm(&charger->batt_id_ohm);
	if (ret < 0)
		pr_err("sm5038-charger: [ ADC ] get batt_id_ohm error ,ret = %d\n", ret);

	ret = sm5038_get_batt_therm(&charger->batt_therm);
	if (ret < 0)
		pr_err("sm5038-charger: [ ADC ] get batt_therm error , ret = %d\n", ret);

	ret = sm5038_get_charger_skin_therm(&charger->charger_skin_therm);
	if (ret < 0)
		pr_err("sm5038-charger: [ ADC ] get charger_skin_therm error , ret = %d\n", ret);

	ret = sm5038_get_usb_conn_therm(&charger->usb_conn_therm);
	if (ret < 0)
		pr_err("sm5038-charger: [ ADC ] get usb_conn_therm error , ret = %d\n", ret);

	pr_info("sm5038-charger: Current_temperatures: BATT_ID: %d, USB_CONN_TEMP: %d, BATT_TEMP: %d, SKIN_TEMP: %d\n",
			charger->batt_id_ohm, charger->usb_conn_therm, charger->batt_therm,	charger->charger_skin_therm);

	return ret;

}
#endif


/* SW thermal regulation thresholds in deciDegC */
#define USB_CONN_TEMP_SHDN_THRESH		700
#define USB_CONN_TEMP_SHDN_ALERT_THRESH	600

#define USB_CONN_TEMP_RANGE_L_THRESH	200
#define USB_CONN_TEMP_RANGE_H_THRESH	300


#define BATT_TEMP_CHGOFF_H_THRESH		550
#define BATT_TEMP_CHGOFF_H_ALERT_THRESH	450

#define BATT_TEMP_CHGOFF_L_THRESH		50
#define BATT_TEMP_CHGOFF_L_ALERT_THRESH	100


#define BATT_TEMP_RANGE_L_THRESH	200
#define BATT_TEMP_RANGE_H_THRESH	290


#define THERM_REG_RECHECK_DELAY_1S		1000	/* 1s */
#define THERM_REG_RECHECK_DELAY_5S		5000	/* 5s */


#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void chg_set_thermal_status(struct sm5038_charger_data *charger, int thermal)
{
	pr_info("sm5038-charger: %s: Changed [%d -> %d]\n", __func__, charger->thermal_status, thermal);
	charger->thermal_status = thermal;
}

static int sm5038_process_thermal_readings(struct sm5038_charger_data *charger)
{
	u32 thermal_status = TEMP_BELOW_RANGE;
	int chg_mode = SM5038_CHG_MODE_CHARGING_ON;
	/* 
	 * If real detach status, 
	 *          skip therm condition.
	 * If fake detach status, becuase thermal is higher than USB_CONN_TEMP_SHDN_THRESH,
	 *          check therm condition.
	 */
	if ((charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) || (charger->cable_type == POWER_SUPPLY_TYPE_UNKNOWN)) {
		if (charger->thermal_detach == false)
			goto skip_temp_check;
	}

	/* If input_control is used, already buck-off status. so return */
	if (charger->is_input_control_used == true) {
		goto skip_temp_check;
	}

	/* if vbus is higher then 10V, already buck-off status. so return */
	if (charger->sw_ovp_enable) {
		goto skip_temp_check;		
	}

	/* ### buck off condition ####
	 *
	 * 1. USB_CONN_TEMP > USB_CONN_TEMP_SHDN_THRESH and charging on
	 *
	 */
	if (charger->usb_conn_therm > USB_CONN_TEMP_SHDN_THRESH) {
		if (charger->thermal_detach == false) {
			thermal_status = TEMP_SHUT_DOWN_SM5038_BUCK_OFF;
			chg_mode = SM5038_CHG_MODE_BUCK_OFF;
			goto out;
		}
	}


	/* ### charging off condition ####
	 *
	 * 1. USB_CONN_THERM >= SHDN_THRESH and thermal_shutdown(thermal_detach is true) status.
	 * 2. ALERT_THRESH < USB_CONN_THERM < SHDN_THRESH and thermal_shutdown(thermal_detach is true) status.
	 * 3. L_THRESH > BATT_TEMP  or BATT_TEMP > H_THRESH 
	 * 4. TOPOFF interrupt occured, charging-off
	 * 5. fasttimer_expired, charging-off
	 * 6. sw_ovp enabled, charging-off
	 *
	 */
	if( ((charger->usb_conn_therm >= USB_CONN_TEMP_SHDN_THRESH) && (charger->thermal_detach == true)) ||
		((charger->usb_conn_therm > USB_CONN_TEMP_SHDN_ALERT_THRESH) && (charger->usb_conn_therm < USB_CONN_TEMP_SHDN_THRESH) && (charger->thermal_detach == true)) ||		
		((charger->batt_therm > BATT_TEMP_CHGOFF_H_THRESH) || (charger->batt_therm < BATT_TEMP_CHGOFF_L_THRESH)) ||
		(charger->charging_status == POWER_SUPPLY_STATUS_FULL) ||
		(charger->fasttimer_expired == true) ||
		(charger->sw_ovp_enable == true)) {
		/* thermal_detach, so valid vbus is present */
		thermal_status = TEMP_SHUT_DOWN_SM5038_CHG_OFF;
		chg_mode = SM5038_CHG_MODE_CHARGING_OFF;
		goto out;
	}
		
	/* ### charging on condition ####
	* 
	* 1. USB_CONN_TEMP or BATT_TEMP Alert 
	*
	*/
	if ( ((charger->usb_conn_therm > USB_CONN_TEMP_SHDN_ALERT_THRESH) && (charger->thermal_detach == false)) ||
		(charger->batt_therm > BATT_TEMP_CHGOFF_H_ALERT_THRESH) ||
		(charger->batt_therm < BATT_TEMP_CHGOFF_L_ALERT_THRESH) ) {
		thermal_status = TEMP_ALERT_LEVEL;
		goto out;
	}

	if (charger->usb_conn_therm > USB_CONN_TEMP_RANGE_L_THRESH ||
		charger->batt_therm > BATT_TEMP_RANGE_L_THRESH) {
		thermal_status = TEMP_WITHIN_RANGE;
		goto out;
	}

out:
//	pr_info("sm5038-charger: %s: debug_location[%d], charger->thermal_detach[%d]\n", __func__, debug_location, charger->thermal_detach);
//	pr_info("sm5038-charger: %s: thermal_status[%d -> %d]\n", __func__, charger->thermal_status, thermal_status);
	if (thermal_status != charger->thermal_status) {
		chg_set_thermal_status(charger, thermal_status);

		if (charger->charge_mode != chg_mode)
			psy_chg_set_charging_enable(charger, chg_mode);

		if (chg_mode == SM5038_CHG_MODE_BUCK_OFF) {
			sm5038_cc_control_command(1); /* CC open */
			charger->thermal_detach = true;
		} 

	}
skip_temp_check:
	chg_print_regmap(charger);
	schedule_delayed_work(&charger->thermal_regulation_work, msecs_to_jiffies(THERM_REG_RECHECK_DELAY_5S));

	return 0;
}


static void sm5038_thermal_regulation_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work, struct sm5038_charger_data,
		thermal_regulation_work.work);

	int ret = 0;
/*
	if (charger->is_charging)
		chg_set_wdt_tmr_reset(charger);
*/
	sm5038_chg_print_status(charger);

	ret = sm5038_update_thermal_readings(charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Couldn't read current thermal values %d\n", __func__, ret);
	}

	ret = sm5038_process_thermal_readings(charger);
	if (ret < 0)
		pr_err("sm5038-charger: %s: Couldn't run sw thermal regulation %d\n", __func__, ret);

}
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static void somc_sm5038_main_wake_lock(struct sm5038_charger_data *charger,
								bool en)
{
	if (!charger->somc_main_wake_lock_en && en)
		__pm_stay_awake(charger->somc_main_wake_lock);
	else if (charger->somc_main_wake_lock_en && !en)
		__pm_relax(charger->somc_main_wake_lock);

	somc_sm5038_charger_dbg(charger, PR_MISC, "wake lock for SOMC: %d\n", (int)en);
	charger->somc_main_wake_lock_en = en;
}

#define VALID_BATT_ID_MIN_OHM 1400
#define VALID_BATT_ID_MAX_OHM 28700
#define CELL_IMPEDANCE_MOHM 30
#define SOMC_SM5038_WORK_CHG_POL_TIME_MS 1000
#define SOMC_SM5038_WORK_DISCHG_POL_TIME_MS 5000
#define SOMC_SM5038_TOPOFF_CURENT_TEMP_THRESH 350
#define SOMC_SM5038_TOPOFF_CURENT_HI_MA 394
#define SOMC_SM5038_TOPOFF_CURENT_LO_MA 246
#define SOMC_SM5038_PSY_CHANGE_TIME_MS 30000
#define SOC_ADJUST_SPEED_MAX 10
#define SOC_ADJUST_SPEED_NORMAL 5
#define SOC_ADJUST_SPEED_MIN 2
static void somc_sm5038_main_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work,
						struct sm5038_charger_data,
						somc_main_work.work);
	int rc;
	int temp, vbatt_uv = 0, current_now = 0;
	int temp_sum = 0, current_now_sum = 0, vbatt_uv_sum = 0;
	int temp_avg = 0, current_now_avg = 0, vbatt_uv_avg = 0;
	int sum_cnt = 0;
	int vcell_mv, target_current_ma = 0, target_voltage_mv = 0;
	int condition = 0;
	bool target_is_found = false;
	int target_idx = -1;
	int i;
	int lv = 0;
	int topoff_ma;
	u8 offset;
	int batt_id = 0;
	int adjust_soc, full_adjsut_target_soc;
	bool vbusok = false;
	u8 reg;
	struct dt_step_data step_data;
	bool psy_changed = false;
	s64 current_time;
	static const char * const str_status[] = {"-", "Charging",
				"Discharging", "NotCharging", "FULL", "-"};

	/* read batt_id (first time only), and judge whether valid or inbalid */
	if (!charger->batt_id_ohm) {
		rc = sm5038_get_batt_id_ohm(&batt_id);
		if (rc < 0) {
			pr_err("sm5038-charger: get batt_id_ohm error ,rc = %d\n", rc);
		} else {
			if (batt_id < VALID_BATT_ID_MIN_OHM ||
				batt_id > VALID_BATT_ID_MAX_OHM)
				charger->is_invalid_batt_id = true;

			charger->batt_id_ohm = batt_id;
		}
	}

	/* read VBUS status */
	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg);
	if (reg & (0x1 << 0))
		vbusok = true;

	/* read batt_temp 0.01C */
	if (charger->real_temp_debug) {
		temp = charger->real_temp_debug;
	} else {
		rc = sm5038_get_batt_therm(&temp);
		if (rc < 0)
			pr_err("Couldn't get batt_therm, ret=%d\n", rc);
	}

	/* read vbat uV */
	vbatt_uv = sm5038_fg_get_prop_vbat_now();

	/* read ibat uA (Positive value means charging) */
	current_now = sm5038_fg_get_prop_current_now();

	/* read soc % */
	charger->batt_soc = (int)somc_sm5038_fg_get_prop_batt_soc();
	somc_sm5038_charger_dbg(charger, PR_MISC, "status:%d, bsoc:%d, ibat:%d, vbat:%d, batt_therm:%d\n",
				charger->charging_status, charger->batt_soc,
				current_now, vbatt_uv, temp);

	if (!charger->step_en)
		goto SKIP_STEP;

	if (charger->batt_aging_level < AGING_LEVEL_NUM)
		lv = charger->batt_aging_level;
	step_data = charger->step_data[lv];

	/* calc avarage value of temp, current and voltage */
	current_time = ktime_to_ms(ktime_get_boottime());
	for (i = STEP_INPUT_BUF_NUM - 1; i > 0; i--)
		charger->step_input_data[i] = charger->step_input_data[i - 1];
	charger->step_input_data[0].temp = temp;
	charger->step_input_data[0].current_now = current_now;
	charger->step_input_data[0].voltage_now = vbatt_uv;
	charger->step_input_data[0].stored_ktime_ms = current_time;

	for (i = 0; i < STEP_INPUT_BUF_NUM; i++) {
		if (charger->step_input_data[i].stored_ktime_ms) {
			temp_sum += charger->step_input_data[i].temp;
			current_now_sum +=
				charger->step_input_data[i].current_now;
			vbatt_uv_sum +=
				charger->step_input_data[i].voltage_now;
			sum_cnt++;
			somc_sm5038_charger_dbg(charger, PR_STEP, "[%d] temp:%d current:%d voltage:%d\n",
				i,
				charger->step_input_data[i].temp,
				charger->step_input_data[i].current_now,
				charger->step_input_data[i].voltage_now);
		}
	}
	if (sum_cnt > 0) {
		temp_avg = temp_sum / sum_cnt;
		current_now_avg = current_now_sum / sum_cnt;
		vbatt_uv_avg = vbatt_uv_sum / sum_cnt;
	}
	somc_sm5038_charger_dbg(charger, PR_STEP, "[T] temp:%d current:%d voltage:%d\n",
				temp_avg, current_now_avg, vbatt_uv_avg);

	/* calculate cell voltage */
	if (current_now_avg > 0 && charger->cell_impedance_mohm)
		vcell_mv = (vbatt_uv_avg - (charger->cell_impedance_mohm *
					current_now_avg / 1000)) / 1000;
	else
		vcell_mv = vbatt_uv_avg / 1000;

	/* update the highest value of cell voltage */
	if (charger->charging_status != POWER_SUPPLY_STATUS_CHARGING) {
		somc_sm5038_charger_dbg(charger, PR_STEP, "Clear vcell max.\n");
		charger->vcell_max_mv = 0;
	} else if (charger->charging_status != charger->prev_charging_status) {
		somc_sm5038_charger_dbg(charger, PR_STEP, "Init vcell max as current vcell.\n");
		charger->vcell_max_mv = vcell_mv;
	} else if (vcell_mv > charger->vcell_max_mv) {
		somc_sm5038_charger_dbg(charger, PR_STEP, "Update vcell max\n");
		charger->vcell_max_mv = vcell_mv;
	} else if (current_now_avg < 0) {
		somc_sm5038_charger_dbg(charger, PR_STEP, "Init vcell max due to minus charge\n");
		charger->vcell_max_mv = vcell_mv;
	}

	somc_sm5038_charger_dbg(charger, PR_STEP, "[%s] BSOC:%d BTEMP:%d IBATT:%dmA VBATT:%dmV VCELL:%dmV VCELL_MAX:%dmV\n",
			str_status[charger->charging_status],
			charger->batt_soc, temp_avg,
			(current_now_avg / 1000), (vbatt_uv_avg / 1000),
			vcell_mv, charger->vcell_max_mv);

	/* determine step current target */
	somc_sm5038_charger_dbg(charger, PR_STEP, "+-------------------------------------------------------+\n");
	somc_sm5038_charger_dbg(charger, PR_STEP, "|No |  Range of Temp  | Range of VCELL  | Target fcc/fv |\n");
	somc_sm5038_charger_dbg(charger, PR_STEP, "|---+-----------------+-----------------+---------------|\n");
	for (i = 0; i < step_data.data_num; i++) {
		if (!target_is_found &&
		    temp_avg >= step_data.temp_low[i] &&
		    temp_avg < step_data.temp_high[i] &&
		    charger->vcell_max_mv >= step_data.voltage_low[i] &&
		    charger->vcell_max_mv <= step_data.voltage_high[i]) {
			target_current_ma = step_data.target_current[i];
			target_voltage_mv = step_data.target_voltage[i];
			condition = step_data.condition[i];
			target_is_found = true;
			target_idx = i;
		}
		somc_sm5038_charger_dbg(charger, PR_STEP, "|%2d | %6d - %6d | %6d - %6d | %2s%5d%6d |\n",
			i + 1,
			step_data.temp_low[i],
			step_data.temp_high[i],
			step_data.voltage_low[i],
			step_data.voltage_high[i],
			i == target_idx ? "->" : "  ",
			step_data.target_current[i],
			step_data.target_voltage[i]);
	}
	somc_sm5038_charger_dbg(charger, PR_STEP, "+-------------------------------------------------------+\n");

	if (!target_is_found)
		somc_sm5038_charger_dbg(charger, PR_CORE, "Couldn't find step target in table.\n");

	if (target_idx != charger->target_idx)
		somc_sm5038_charger_dbg(charger, PR_STEP, "[%s] BSOC:%d RTEMP:%d IBATT:%dmA VBATT:%dmV VCELL:%dmV Target FCC:%dmA FV:%dmV\n",
			str_status[charger->charging_status],
			charger->batt_soc, temp_avg,
			(current_now_avg / 1000), (vbatt_uv_avg / 1000),
			vcell_mv, target_current_ma, target_voltage_mv);

	/* request vote of fcc and fv */
	vote(charger->fcc_votable, JEITA_STEP_VOTER, true, target_current_ma);
	vote(charger->fv_votable, JEITA_STEP_VOTER, true, target_voltage_mv);

	/* Update termination current depending on batt_temp */
	if (temp_avg > SOMC_SM5038_TOPOFF_CURENT_TEMP_THRESH)
		topoff_ma = SOMC_SM5038_TOPOFF_CURENT_HI_MA;
	else
		topoff_ma = SOMC_SM5038_TOPOFF_CURENT_LO_MA;

	if (topoff_ma != charger->topoff_ma) {
		if (topoff_ma < 100)
			offset = 0x00;
		else if (topoff_ma < 800)
			offset = ((topoff_ma + 24 - 100) / 25) & 0x1f;
		else
			offset = 0x1c;

		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL5,
						(offset << 0), (0x1f << 0));

		charger->topoff_ma = topoff_ma;
		somc_sm5038_charger_dbg(charger, PR_STEP, "topoff current is changed to %dmA\n",
								topoff_ma);
	}

	/* Adjust SoC */
	charger->target_soc_adj_coef = charger->adj_soc_coef[lv];
	if (!charger->soc_adjust_en && charger->is_batt_aging_level_set) {
		charger->cur_soc_adj_coef = charger->target_soc_adj_coef;
		charger->soc_adjust_en = true;
		somc_sm5038_charger_dbg(charger, PR_CORE, "soc adjust enabled\n");
	}
	if (charger->soc_adjust_en && vbusok &&
		charger->cur_soc_adj_coef < charger->target_soc_adj_coef) {
		if (current_now_avg > 250000) {
			charger->cur_soc_adj_coef += SOC_ADJUST_SPEED_MAX;
			somc_sm5038_charger_dbg(charger, PR_MISC, "soc adjust: UP speedily");
		} else {
			charger->cur_soc_adj_coef += SOC_ADJUST_SPEED_MIN;
			somc_sm5038_charger_dbg(charger, PR_MISC, "soc adjust: UP slowly");
		}
		if (charger->cur_soc_adj_coef > charger->target_soc_adj_coef) {
			charger->cur_soc_adj_coef =
						charger->target_soc_adj_coef;
		}
	} else if (charger->soc_adjust_en && !vbusok &&
		charger->cur_soc_adj_coef > charger->target_soc_adj_coef) {
		charger->cur_soc_adj_coef -= SOC_ADJUST_SPEED_NORMAL;
		if (charger->cur_soc_adj_coef < charger->target_soc_adj_coef) {
			charger->cur_soc_adj_coef =
						charger->target_soc_adj_coef;
			somc_sm5038_charger_dbg(charger, PR_MISC, "soc adjust: DOWN normaly");
		}
	}
	if (charger->soc_adjust_en) {
		adjust_soc = charger->batt_soc * charger->cur_soc_adj_coef /
									10000;
		somc_sm5038_charger_dbg(charger, PR_MISC, "adjust_soc:%d->%d, cur_soc_adj_coef:%d, target_soc_adj_coef:%d\n",
				charger->adjust_soc,
				adjust_soc,
				charger->cur_soc_adj_coef,
				charger->target_soc_adj_coef);
		charger->adjust_soc = adjust_soc;
	}

	/* Full Adjust SoC */
	if (charger->soc_adjust_en)
		full_adjsut_target_soc = charger->adjust_soc;
	else
		full_adjsut_target_soc = charger->batt_soc;
	if (charger->full_soc_adjust_en) {
		if (charger->full_adjust_soc - full_adjsut_target_soc > 500 ||
			current_now_avg < -500000)
			charger->full_adjust_soc -= SOC_ADJUST_SPEED_MAX;
		else if (current_now_avg < -100000)
			charger->full_adjust_soc -= SOC_ADJUST_SPEED_NORMAL;
		else if (current_now_avg < 50000)
			charger->full_adjust_soc -= SOC_ADJUST_SPEED_MIN;
		else
			; /* Don't adjust during charging */

		somc_sm5038_charger_dbg(charger, PR_MISC, "Adjust full_adjust_soc. full_adjust_soc:%d, full_adjsut_target_soc:%d, current_now_avg:%d\n",
				charger->full_adjust_soc,
				full_adjsut_target_soc,
				current_now_avg);
		if (full_adjsut_target_soc >= charger->full_adjust_soc) {
			charger->full_adjust_soc = 0;
			charger->full_soc_adjust_en = false;
			somc_sm5038_charger_dbg(charger, PR_CORE, "Full SoC Adjust completed!!\n");
		}
		psy_changed = true;
	}

	/* Update learning range */
	if (charger->charging_status != POWER_SUPPLY_STATUS_CHARGING ||
						charger->batt_soc < 3500) {
		if (charger->target_soc_adj_coef > 11625)
			somc_sm5038_fg_learn_set_range(40, 80);
		else if (charger->target_soc_adj_coef > 10941)
			somc_sm5038_fg_learn_set_range(40, 85);
		else if (charger->target_soc_adj_coef > 10333)
			somc_sm5038_fg_learn_set_range(40, 90);
		else
			somc_sm5038_fg_learn_set_range(40, 93);
	}

	if (charger->charging_status == POWER_SUPPLY_STATUS_CHARGING &&
		current_time - charger->last_psy_changed_time_ms >
					SOMC_SM5038_PSY_CHANGE_TIME_MS) {
		psy_changed = true;
		charger->last_psy_changed_time_ms = current_time;
	}

	if ((charger->jeita_step_condition != condition ||
		charger->target_current_ma != target_current_ma ||
		charger->target_voltage_mv != target_voltage_mv))
		psy_changed = true;

	charger->jeita_step_condition = condition;
	charger->target_current_ma = target_current_ma;
	charger->target_voltage_mv = target_voltage_mv;
	if (charger->charging_status != POWER_SUPPLY_STATUS_CHARGING)
		charger->target_idx = -1;
SKIP_STEP:
	charger->prev_charging_status = charger->charging_status;

	if (charger->soc_adjust_en &&
		charger->last_psy_changed_adjust_soc / 10 !=
						charger->adjust_soc / 10) {
		somc_sm5038_charger_dbg(charger, PR_MISC, "adjust_soc changed %d->%d\n",
					charger->last_psy_changed_adjust_soc,
					charger->adjust_soc);
		psy_changed = true;
		charger->last_psy_changed_adjust_soc = charger->adjust_soc;
	}
	if (charger->last_psy_changed_batt_soc / 10 !=
						charger->batt_soc / 10) {
		somc_sm5038_charger_dbg(charger, PR_MISC, "batt_soc changed %d->%d\n",
					charger->last_psy_changed_batt_soc,
					charger->batt_soc);
		psy_changed = true;
		charger->last_psy_changed_batt_soc = charger->batt_soc;
	}

	if (charger->psy_chg && psy_changed)
		power_supply_changed(charger->psy_chg);

	/* re-schedule work and wake lock */
	if (charger->charging_status == POWER_SUPPLY_STATUS_CHARGING) {
		somc_sm5038_main_wake_lock(charger, true);
		schedule_delayed_work(&charger->somc_main_work,
			msecs_to_jiffies(SOMC_SM5038_WORK_CHG_POL_TIME_MS));
	} else if (charger->charging_status == POWER_SUPPLY_STATUS_FULL) {
		schedule_delayed_work(&charger->somc_main_work,
			msecs_to_jiffies(SOMC_SM5038_WORK_CHG_POL_TIME_MS));
		somc_sm5038_main_wake_lock(charger, false);
	} else {
		schedule_delayed_work(&charger->somc_main_work,
			msecs_to_jiffies(SOMC_SM5038_WORK_DISCHG_POL_TIME_MS));
		somc_sm5038_main_wake_lock(charger, false);
	}
}

#define SOMC_SM5038_FULL_WORK_POL_TIME_MS 10000
static void somc_sm5038_full_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work,
						struct sm5038_charger_data,
						somc_full_work.work);
	u8 reg;
	bool topoff = false, done = false, vbusok = false, is_warm = false;
	int soc, effective_icl;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS2, &reg);
	if (reg & (0x1 << 5))
		topoff = true;
	if (reg & (0x1 << 6))
		done = true;

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS1, &reg);
	if (reg & (0x1 << 0))
		vbusok = true;

	effective_icl = get_effective_result(charger->usb_icl_votable);
	is_warm = (charger->jeita_step_condition == JEITA_STEP_COND_WARM);

	if (!charger->is_full && (topoff || done)) {
		if (is_warm) {
			vote(charger->fcc_votable,
					WARM_TERMINATION_VOTER, true, 0);
			somc_sm5038_charger_dbg(charger, PR_CORE, "Terminated during WARM!!!\n");
		} else {
			/* Update Learning */
			somc_sm5038_update_charge_full(charger);

			vote(charger->fcc_votable, FULL_VOTER, true, 0);
			if (charger->full_recharge_debug_en)
				vote(charger->usb_icl_votable,
						FULL_DEBUG_VOTER, true, 100);
			charger->is_full = true;
			charger->full_soc_hold_en = true;
			charger->full_counter++;
			somc_sm5038_charger_dbg(charger, PR_CORE, "Fully charged!!! full_soc_hold_en:%d\n",
						charger->full_soc_hold_en);
		}
		if (charger->psy_chg)
			power_supply_changed(charger->psy_chg);
	}
	if (charger->is_full && (!vbusok || !effective_icl)) {
		vote(charger->fcc_votable, FULL_VOTER, false, 0);
		if (is_client_vote_enabled(charger->usb_icl_votable,
							FULL_DEBUG_VOTER))
			vote(charger->usb_icl_votable, FULL_DEBUG_VOTER,
								false, 0);
		charger->is_full = false;
		if (charger->psy_chg)
			power_supply_changed(charger->psy_chg);
	}
	if ((!vbusok || !effective_icl || !is_warm) &&
		is_client_vote_enabled(charger->fcc_votable,
						WARM_TERMINATION_VOTER)) {
		vote(charger->fcc_votable, WARM_TERMINATION_VOTER, false, 0);
		somc_sm5038_charger_dbg(charger, PR_CORE, "Release Termination during WARM!!! jeita_step_condition:%d\n",
						charger->jeita_step_condition);
	}

	if (charger->soc_adjust_en)
		soc = charger->adjust_soc;
	else
		soc = charger->batt_soc;
	if (charger->is_full && soc < 9500) {
		somc_sm5038_charger_dbg(charger, PR_CORE, "Recharged!!! soc:%d\n", soc);
		vote(charger->fcc_votable, FULL_VOTER, false, 0);
		if (charger->full_recharge_debug_en)
			vote(charger->usb_icl_votable, FULL_DEBUG_VOTER,
								false, 0);
		charger->is_full = false;
		charger->recharge_counter++;
		if (charger->psy_chg)
			power_supply_changed(charger->psy_chg);
	}

	if (charger->full_soc_hold_en && (!vbusok || !effective_icl)) {
		charger->full_soc_hold_en = false;
		charger->full_soc_adjust_en = true;
		charger->full_adjust_soc = 10000;
		somc_sm5038_charger_dbg(charger, PR_CORE, "Full SoC adjust is started by charger removed!! full_soc_hold_en:%d, full_adjust_soc:%d\n",
						charger->full_soc_hold_en,
						charger->full_adjust_soc);
		if (charger->psy_chg)
			power_supply_changed(charger->psy_chg);
	}

	if (is_client_vote_enabled(charger->fcc_votable, FULL_VOTER) ||
		is_client_vote_enabled(charger->fcc_votable,
						WARM_TERMINATION_VOTER))
		schedule_delayed_work(&charger->somc_full_work,
			msecs_to_jiffies(SOMC_SM5038_FULL_WORK_POL_TIME_MS));
}

#endif

extern int sm5038_muic_get_vbus_voltage(void);

#define SW_VBUS_OVP_RECHECK_DELAY_5S		5000	/* 5s */
#define WA_SW_VBUS_OVP_THRESH 		10000	/* 10V */

static void sm5038_sw_ovp_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work, struct sm5038_charger_data,
		wa_sw_ovp_work.work);
	int read_vbus = 0;
	bool en_sw_ovp_thresh = false;

	read_vbus = sm5038_muic_get_vbus_voltage();
	if (read_vbus > WA_SW_VBUS_OVP_THRESH)
		en_sw_ovp_thresh = true;

	if (charger->sw_ovp_enable != en_sw_ovp_thresh) {
		pr_info("sm5038-charger: %s: sw ovp %s\n", __func__, en_sw_ovp_thresh ? "ENABLE" : "DISABLE");
		charger->sw_ovp_enable = en_sw_ovp_thresh;

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		/* 0: chg_on, 1: buck_off */
		psy_chg_set_input_control(charger, charger->sw_ovp_enable);
#endif
	}

	schedule_delayed_work(&charger->wa_sw_ovp_work, msecs_to_jiffies(SW_VBUS_OVP_RECHECK_DELAY_5S));
}

static irqreturn_t chg_vbuspok_isr(int irq, void *data)
{
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct sm5038_charger_data *charger = data;
#endif

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);
#endif
	return IRQ_HANDLED;
}

static irqreturn_t chg_wpcinpok_isr(int irq, void *data)
{

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

	return IRQ_HANDLED;
}


static irqreturn_t chg_aicl_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;
	u8 reg;

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_CNTL1, &reg);
	pr_info("sm5038-charger: %s: check aiclen[%d:%d]\n", __func__, ((reg & 0x40) ? 1:0), ((reg & 0x20) ? 1:0));


	__pm_stay_awake(charger->aicl_wake_lock);
	queue_delayed_work(charger->wqueue, &charger->aicl_work, msecs_to_jiffies(50));

	return IRQ_HANDLED;
}

static void sm5038_charger_enable_aicl_irq(struct sm5038_charger_data *charger)
{
	int ret;
	u8 reg_data;

	ret = request_threaded_irq(charger->irq_aicl, NULL,
			chg_aicl_isr, 0, "aicl-irq", charger);
	if (ret < 0) {
		charger->irq_aicl_enabled = -1;
		pr_err("sm5038-charger: %s: fail to request aicl-irq:%d (ret=%d)\n",
					__func__, charger->irq_aicl, ret);
	} else {
		charger->irq_aicl_enabled = 1;
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_INTMSK2, &reg_data);
		pr_info("sm5038-charger: %s: enable aicl : 0x%X\n", __func__, reg_data);
	}
}

static irqreturn_t chg_done_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* Toggle ENQ4FET for Re-cycling charger loop */
	chg_set_enq4fet(charger, 0);
	msleep(10);
	chg_set_enq4fet(charger, 1);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cancel_delayed_work_sync(&charger->somc_full_work);
	schedule_delayed_work(&charger->somc_full_work, 0);
#endif

	return IRQ_HANDLED;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static irqreturn_t chg_chgon_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;

	somc_sm5038_charger_dbg(charger, PR_INTERRUPT, "sm5038-charger: %s: sm5038_irq[%d]\n",
							__func__, irq);

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);

	return IRQ_HANDLED;
}

#endif
static irqreturn_t chg_vsysovp_isr(int irq, void *data)
{

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

	return IRQ_HANDLED;
}

static irqreturn_t chg_vbusuvlo_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;
	u8 reg;

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

//	sm5038_cc_control_command(0); /* CC restore */
//	charger->thermal_detach = false;


	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_FACTORY1, &reg);
	if (reg & 0x02) {
		pr_info("sm5038-charger: %s: bypass mode enabled\n",
			__func__);
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);

	if (charger->bootup_shutdown_phase == DURING_POWER_OFF_CHARGE) {
		__pm_wakeup_event(charger->unplug_wakelock, UNPLUG_WAKE_PERIOD);
		schedule_delayed_work(&charger->offchg_termination_work,
				msecs_to_jiffies(OFFCHG_TERMINATION_DELAY_MS));
	}
#endif
	return IRQ_HANDLED;
}

static irqreturn_t chg_otgfail_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;
	u8 reg;

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

	sm5038_read_reg(charger->i2c, SM5038_CHG_REG_STATUS3, &reg);
	if (reg & 0x04) {
		pr_info("sm5038-charger: %s: otg overcurrent limit\n",
			__func__);

		psy_chg_set_otg_control(charger, false);
	}

	return IRQ_HANDLED;
}


static irqreturn_t chg_fasttmroff_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;

	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	charger->fasttimer_expired = true;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->fcc_votable, SAFETY_TIMER_VOTER, true, 0);
	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);
#endif
	return IRQ_HANDLED;
}


static irqreturn_t chg_topoff_isr(int irq, void *data)
{
	struct sm5038_charger_data *charger = data;
	pr_info("sm5038-charger: %s: sm5038_irq[%d]\n", __func__, irq);

	if (charger == NULL) {
		pr_err("%s: static charger fail", __func__);
		return IRQ_HANDLED;
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	// charger status is POWER_SUPPLY_STATUS FULL
	psy_chg_get_status(charger);

	psy_chg_set_charging_enable(charger, SM5038_CHG_MODE_CHARGING_OFF);

	cancel_delayed_work_sync(&charger->thermal_regulation_work);
	schedule_delayed_work(&charger->thermal_regulation_work, 0);

	schedule_delayed_work(&charger->rechg_work, msecs_to_jiffies(RECHG_WORK_DELAY));
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cancel_delayed_work_sync(&charger->somc_full_work);
	schedule_delayed_work(&charger->somc_full_work, 0);
#endif

	return IRQ_HANDLED;
}


static inline void sm5038_chg_init(struct sm5038_charger_data *charger)
{
	chg_set_aiclth(charger, AICL_TH_V_4_6);
	chg_set_aiclen_vbus(charger, 1);
	chg_set_ocp_current(charger, charger->pdata->chg_ocp_current);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chg_set_batreg(charger, charger->pdata->chg_float_voltage);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->fv_votable, DEFAULT_VOTER, true,
					charger->pdata->chg_float_voltage);
	somc_sm5038_disable_watchdog(charger);
#endif
	chg_set_wdt_timer(charger, WDT_TIME_S_90);
	chg_set_topoff_timer(charger, TOPOFF_TIME_M_45);
	chg_set_shipmode_auto_th(charger, AUTO_SHIP_MODE_VREF_V_2_6);
	chg_set_shipmode_enter_time(charger, SHIP_MODE_IN_TIME_S_8);
	chg_set_en_shipmode(charger, SHIP_MODE_AUTO);
	chg_set_fast_charging_step(charger, 1);
	chg_set_enfastchgtimer(charger, 1);
	chg_set_fastchg_timer(charger, FASTCHG_TIME_HRS_10);	
	chg_print_regmap(charger);

	pr_info("sm5038-charger: %s: init done.\n", __func__);
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define PROPERTY_NAME_SIZE 128
static int somc_sm5038_parse_dt_step(struct sm5038_charger_data *charger,
						struct device_node *np)
{
	int ret = 0;
	int num, i, j, step_len;
	u32 step_buf[STEP_DATA_DT_MAX_NUM];
	struct dt_step_data step_data;
	char prop_name[PROPERTY_NAME_SIZE];

	memset(step_buf, 0, sizeof(step_buf));

	for (i = 0; i < AGING_LEVEL_NUM; i++) {
		charger->step_en = true;

		snprintf(prop_name, PROPERTY_NAME_SIZE,
						"somc,step-cfg-lv%d", i);
		ret = of_property_count_elems_of_size(np,
						prop_name, sizeof(u32));
		if (ret < 0) {
			pr_err("Can't get size of %s %d\n", prop_name, ret);
			charger->step_en = false;
			goto step_err;
		} else {
			step_len = ret;
			somc_sm5038_charger_dbg(charger, PR_STEP, "size %s: %d\n", prop_name, step_len);
		}
		if (step_len > STEP_DATA_DT_MAX_NUM ||
						(step_len % STEP_DATA_RAW)) {
			pr_err("step config table size is invalid\n");
			charger->step_en = false;
			goto step_err;
		}
		ret = of_property_read_u32_array(np, prop_name,
						&step_buf[0], step_len);
		if (ret < 0) {
			pr_err("Can't get %s %d\n", prop_name, ret);
			goto step_err;
		}
		num = 0;
		for (j = 0; j < step_len; j += STEP_DATA_RAW) {
			step_data.temp_low[num] = step_buf[j];
			step_data.temp_high[num] = step_buf[j + 1];
			step_data.voltage_low[num] = step_buf[j + 2];
			step_data.voltage_high[num] = step_buf[j + 3];
			step_data.target_current[num] = step_buf[j + 4];
			step_data.target_voltage[num] = step_buf[j + 5];
			step_data.condition[num] = step_buf[j + 6];
			num++;
		}
		step_data.data_num = num;
		charger->step_data[i] = step_data;

		/* Check Step Data */
		somc_sm5038_charger_dbg(charger, PR_STEP, "AgingLv%d Jeita/Step table\n", i);
		somc_sm5038_charger_dbg(charger, PR_STEP, "No |  Range of Temp  | Range of VBATT  | Target FCC/FV Condition\n");
		for (j = 0; j < step_data.data_num; j++)
			pr_info("%2d | %6d - %6d | %6d - %6d | %6d %4d %d\n",
				j + 1,
				step_data.temp_low[j],
				step_data.temp_high[j],
				step_data.voltage_low[j],
				step_data.voltage_high[j],
				step_data.target_current[j],
				step_data.target_voltage[j],
				step_data.condition[j]);
	}
	ret = of_property_read_u32(np, "somc,step-cell-impedance-mohm",
						&charger->cell_impedance_mohm);
	if (ret < 0) {
		pr_err("Can't get battery cell impedance %d\n", ret);
		charger->cell_impedance_mohm = -EINVAL;
	}
step_err:
	return ret;
}

#endif

void sm5038_chg_register_reset(struct i2c_client *i2c, void *charger_data)
{
	struct sm5038_charger_data *charger = static_charger_data;

	if (charger == NULL) {
		pr_info("%s: static charger fail", __func__);
		return ;
	}

	pr_info("sm5038-charger: %s: register reset all\n", __func__);

	/* reset bit */ 
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1, (0x1 << 7), (0x1 << 7));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_INTMSK1, (0xEC << 0), (0xFF << 0));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_INTMSK2, (0x9F << 0), (0xFF << 0));
	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_INTMSK3, (0x5B << 0), (0xFF << 0));
	sm5038_chg_init(charger);

}
EXPORT_SYMBOL_GPL(sm5038_chg_register_reset);

static int sm5038_charger_parse_dt(struct sm5038_charger_data *charger)
{
	struct device_node *np;
	struct sm5038_charger_platform_data *pdata = charger->pdata;
	int ret = 0, byte_len = 0, dt_rechg_soc = 0;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	u32 therm_mitig_params[NUM_THERM_MITIG_STEPS];
	int i;
#endif

	np = of_find_node_by_name(NULL, "sm5038-charger");
	if (!np) {
		pr_err("sm5038-charger: %s: can't find battery node\n", __func__);
	} else {
		charger->dev->of_node = np;
/* float voltage */
		ret = of_property_read_u32(np, "battery,chg_float_voltage",
					   &pdata->chg_float_voltage);
		if (ret) {
			pr_info("sm5038-charger: %s: battery,chg_float_voltage is Empty\n", __func__);
			pdata->chg_float_voltage = 4480;
		}
		pr_info("sm5038-charger: %s: battery,chg_float_voltage is %d\n",
			__func__, pdata->chg_float_voltage);
		charger->float_voltage = pdata->chg_float_voltage;

/* ocp current */
		ret = of_property_read_u32(np, "battery,chg_ocp_current",
					   &pdata->chg_ocp_current);
		if (ret) {
			pr_info("sm5038-charger: %s: battery,chg_ocp_current is Empty\n", __func__);
			pdata->chg_ocp_current = 5400; /* mA */
		}
		pr_info("sm5038-charger: %s: battery,chg_ocp_current is %d\n", __func__,
			pdata->chg_ocp_current);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		ret = somc_sm5038_parse_dt_step(charger, np);
		if (ret < 0)	{
			pr_err("sm5038-charger: somc_sm5038_parse_dt_step is error. ret = %d\n",
									ret);
			return ret;
		}

		if (of_find_property(np,
				"somc,batt-aging-soc-adjust", &byte_len) &&
			(byte_len / sizeof(u32) == AGING_LEVEL_NUM)) {
			ret = of_property_read_u32_array(np,
						"somc,batt-aging-soc-adjust",
						&charger->adj_soc_coef[0],
						AGING_LEVEL_NUM);
			if (ret < 0)
				pr_err("Can't get somc,batt-aging-soc-adjust %d\n",
									ret);
		} else {
			somc_sm5038_charger_dbg(charger, PR_CORE, "size somc,batt-aging-soc-adjust: %d\n",
								byte_len);
		}
#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
/* sw_jeita_enabled */
		charger->sw_jeita_enabled = of_property_read_bool(np, "battery,sw-jeita-enable");
		pr_info("sm5038-charger: %s: battery,sw_jeita is %d\n",
			__func__, charger->sw_jeita_enabled);

/* step_chg_enabled */
		charger->step_chg_enabled = of_property_read_bool(np, "battery,step-charging-enable");
		pr_info("sm5038-charger: %s: battery,step_chg_jeita is %d\n",
			__func__, charger->step_chg_enabled);

/* thermal-mitigation */
		if (of_find_property(np, "battery,thermal-mitigation", &byte_len)) {
			charger->thermal_mitigation = devm_kzalloc(charger->dev, byte_len, GFP_KERNEL);
			if (charger->thermal_mitigation == NULL)
				return -ENOMEM;

			charger->thermal_levels = byte_len / sizeof(u32);
			ret = of_property_read_u32_array(np, "battery,thermal-mitigation", charger->thermal_mitigation, charger->thermal_levels);
			if (ret < 0) {
				pr_info("sm5038-charger: %s: battery,thermal_levels is Empty\n", __func__);
				charger->thermal_levels = 14;
			}
			pr_info("sm5038-charger: %s: battery,thermal_levels is %d\n", __func__, charger->thermal_levels);
		}

/* thermal-mitigation-sleep */
		if (of_find_property(np, "battery,thermal-mitigation-sleep", &byte_len)) {
			charger->thermal_mitigation_sleep = devm_kzalloc(charger->dev, byte_len, GFP_KERNEL);
			if (charger->thermal_mitigation_sleep == NULL)
				return -ENOMEM;

			charger->thermal_levels = byte_len / sizeof(u32);
			ret = of_property_read_u32_array(np, "battery,thermal-mitigation-sleep", charger->thermal_mitigation_sleep, charger->thermal_levels);
			if (ret < 0) {
				pr_info("sm5038-charger: %s: battery,thermal_levels is Empty\n", __func__);
				charger->thermal_levels = 14;
			}
			pr_info("sm5038-charger: %s: battery,thermal_levels is %d\n", __func__, charger->thermal_levels);
		}
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (of_find_property(
			np, "somc,thermal-fcc-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-fcc-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-fcc-ma ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[0].fcc_ma =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-usb-icl-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-usb-icl-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-usb-icl-ma ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[0].usb_icl_ma =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-charging-indication", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-charging-indication",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-charging-indication ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[0].
						charging_indication =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-fcc-sleep-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-fcc-sleep-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-fcc-ma ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[1].fcc_ma =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-usb-icl-sleep-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-usb-icl-sleep-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-usb-icl-ma ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[1].usb_icl_ma =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-charging-indication-sleep",
								&byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-charging-indication-sleep",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-charging-indication ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].cond[1].
						charging_indication =
							therm_mitig_params[i];
		}

		if (of_find_property(
			np, "somc,thermal-usb-cc-open", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
			ret = of_property_read_u32_array(np,
				"somc,thermal-usb-cc-open",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
			if (ret < 0) {
				pr_err("Couldn't read thermal-usb-cc-open ret = %d\n",
									ret);
				return ret;
			}
			for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
				charger->therm_mitig[i].usb_cc_open =
							therm_mitig_params[i];
		}
#endif

/* fcc-stepping-enable */
		charger->fcc_stepper_enable = of_property_read_bool(np, "battery,fcc-stepping-enable");
		pr_info("sm5038-charger: %s: battery,fcc-stepping-enable is %d\n", __func__, charger->fcc_stepper_enable);

/* re-charging-soc */
		charger->auto_rechg_soc = -EINVAL;
		ret = of_property_read_u32(np, "battery,auto-recharge-soc", &dt_rechg_soc);
		if (ret) {
			pr_info("sm5038-charger: %s: battery,auto-recharge-soc is Empty\n", __func__);
			dt_rechg_soc = 950;
		}
		charger->auto_rechg_soc = dt_rechg_soc;
		pr_info("sm5038-charger: %s: battery,auto-recharge-soc is %d\n", __func__, charger->auto_rechg_soc);
/*
		ret = of_property_read_string(np, "battery,wireless_charger_name", (char const **)&pdata->wireless_charger_name);
		if (ret)
			pr_info("sm5038-charger: %s: Wireless charger name is Empty\n", __func__);
*/
		ret = sm5038_parse_dt_adc_channels(charger);
		if (ret < 0)	{
			pr_err("sm5038-charger: sm5038_parse_dt_adc_channels is error. ret = %d\n", ret);
			return ret;
		}
	}


	pr_info("sm5038-charger: %s: parse dt done.\n", __func__);
	return 0;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)

static void somc_sm5038_get_active_panel(struct sm5038_charger_data *charger)
{
	int i;
	int count;
	struct device_node *node;
	struct drm_panel *panel;

	charger->active_panel = NULL;

	count = of_count_phandle_with_args(charger->dev->of_node, "panel",
									NULL);

	for (i = 0; i < count; i++) {
		node = of_parse_phandle(charger->dev->of_node, "panel", i);
		panel = of_drm_find_panel(node);
		of_node_put(node);
		if (!IS_ERR(panel)) {
			pr_debug("sm5038-charger: %s: Get active panel\n",
								__func__);
			charger->active_panel = panel;
		} else {
			pr_err("sm5038-charger: %s: Failed to Get active panel\n",
								__func__);
			break;
		}
	}
}

static int somc_sm5038_apply_thermal_mitigation(
					struct sm5038_charger_data *charger)
{
	struct somc_thermal_mitigation *current_therm_mitig =
			&charger->therm_mitig[charger->system_temp_level];
	int cond_idx;
	int rc = 0;

	if (charger->screen_state == 1)
		cond_idx = 0;
	else
		cond_idx = 1;

	rc |= vote(charger->fcc_votable, THERMAL_VOTER, true,
				current_therm_mitig->cond[cond_idx].fcc_ma);
	if (current_therm_mitig->cond[cond_idx].fcc_ma == 0) {
		cancel_delayed_work_sync(&charger->somc_full_work);
		schedule_delayed_work(&charger->somc_full_work, 0);
	}

	rc |= vote(charger->usb_icl_votable, THERMAL_VOTER, true,
				current_therm_mitig->cond[cond_idx].usb_icl_ma);

	switch (current_therm_mitig->cond[cond_idx].charging_indication) {
	case CHG_IND_DEFAULT:
		rc |= vote(charger->fake_chg_votable, THERMAL_VOTER,
								false, 0);
		rc |= vote(charger->fake_chg_disallow_votable, THERMAL_VOTER,
								false, 0);
		break;
	case CHG_IND_FAKE_CHARGING:
		rc |= vote(charger->fake_chg_votable, THERMAL_VOTER,
								true, 0);
		rc |= vote(charger->fake_chg_disallow_votable, THERMAL_VOTER,
								false, 0);
		break;
	case CHG_IND_FAKE_CHARGING_DISALLOW:
		rc |= vote(charger->fake_chg_votable, THERMAL_VOTER,
								false, 0);
		rc |= vote(charger->fake_chg_disallow_votable, THERMAL_VOTER,
								true, 0);
		break;
	default:
		break;
	}

	if (rc < 0)
		pr_err("Couldn't vote for thermal mitigation values, rc=%d\n",
									rc);

	sm5038_cc_control_command(current_therm_mitig->usb_cc_open);

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);

	return rc;
}

static int somc_sm5038_apply_thermal_mitigation_cb(struct notifier_block *self,
					unsigned long event, void *data)
{
	int transition;
	struct drm_panel_notifier *evdata = data;
	struct sm5038_charger_data *charger =
		container_of(self, struct sm5038_charger_data, fb_notifier);

	if (!evdata)
		return 0;

	if (evdata && evdata->data) {
		if (event == DRM_PANEL_EARLY_EVENT_BLANK) {
			transition = *(int *)evdata->data;
			if (transition == DRM_PANEL_BLANK_POWERDOWN)
				charger->screen_state = 0;
		}
		if (event == DRM_PANEL_EVENT_BLANK) {
			transition = *(int *)evdata->data;
			if (transition == DRM_PANEL_BLANK_UNBLANK)
				charger->screen_state = 1;
		}
	}

	return somc_sm5038_apply_thermal_mitigation(charger);
}

static int somc_sm5038_set_batt_aging_level(
				struct sm5038_charger_data *charger, int lv)
{
	int rc = 0;

	charger->batt_aging_level = lv;
	charger->is_batt_aging_level_set = true;

	cancel_delayed_work_sync(&charger->somc_main_work);
	schedule_delayed_work(&charger->somc_main_work, 0);

	return rc;
}

static void somc_sm5038_smart_charge_wdog_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work,
		struct sm5038_charger_data, smart_charge_wdog_work.work);

	pr_debug("Smart Charge Watchdog timer has expired.\n");

	vote(charger->fcc_votable, SMART_EN_VOTER, false, 0);
	somc_sm5038_stop_fake_charging(charger, SMART_EN_VOTER, 0);
}

static int somc_sm5038_smart_set_suspend(struct sm5038_charger_data *charger,
						bool smart_charge_suspended)
{
	int rc = 0;

	if (!charger->smart_charge_enabled) {
		pr_err("Couldn't set smart charge voter due to unactivated\n");
		return rc;
	}

	if (smart_charge_suspended)
		somc_sm5038_start_fake_charging(charger, SMART_EN_VOTER);
	else
		somc_sm5038_stop_fake_charging(charger, SMART_EN_VOTER,
						CHARGE_START_DELAY_TIME);

	rc = vote(charger->fcc_votable, SMART_EN_VOTER,
						smart_charge_suspended, 0);
	if (rc < 0) {
		pr_err("Couldn't vote en, rc=%d\n", rc);
		return rc;
	}
	if (smart_charge_suspended) {
		cancel_delayed_work_sync(&charger->somc_full_work);
		schedule_delayed_work(&charger->somc_full_work, 0);
	}

	pr_debug("voted for smart charging (%d).\n", smart_charge_suspended);
	cancel_delayed_work_sync(&charger->smart_charge_wdog_work);
	if (smart_charge_suspended)
		schedule_delayed_work(&charger->smart_charge_wdog_work,
				msecs_to_jiffies(SMART_CHARGE_WDOG_DELAY_MS));

	return rc;
}

static int somc_sm5038_usb_icl_vote_cb(struct votable *votable,
				void *data, int icl_ma, const char *client)
{
	struct sm5038_charger_data *charger = data;
	u8 offset;

	if (icl_ma >= 100)
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_VBUSIN,
									true);

	if (icl_ma < 100)
		offset = 0x00;
	else if (icl_ma <= 3275)
		offset = ((icl_ma - 100) / 25) & 0x7f;
	else
		offset = 0x7f;

	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_VBUSCNTL, offset, 0x7f);

	if (icl_ma < 100)
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_VBUSIN,
									false);

	charger->input_current = icl_ma;
	charger->charge_mode = (icl_ma >= 100) ?
		SM5038_CHG_MODE_CHARGING_ON : SM5038_CHG_MODE_CHARGING_OFF;
	sm5038_charger_charge_mode_update(charger->charge_mode);

	return 0;
}

static int somc_sm5038_fcc_vote_cb(struct votable *votable,
				void *data, int fcc_ma, const char *client)
{
	struct sm5038_charger_data *charger = data;
	int fcc_ua = fcc_ma * 1000;
	u8 offset;

	if (fcc_ua > 0)
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1,
							(0x1 << 3), (0x1 << 3));

	if (fcc_ua < 109375)		// 109.375 mA
		offset = 0x07;
	else if (fcc_ua > 3500000)	// 3500.000 mA
		offset = 0xe0;
	else
		offset = (7 + ((fcc_ua - 109375) / 15625)) & 0xff;

	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL2, offset, 0xff);

	if (fcc_ua <= 0)
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1,
							(0x0 << 3), (0x1 << 3));

	charger->charging_current = fcc_ma;

	return 0;
}

static int somc_sm5038_fv_vote_cb(struct votable *votable,
				void *data, int fv_mv, const char *client)
{
	struct sm5038_charger_data *charger = data;
	u8 offset;

	if (fv_mv <= 3700)
		/* mininum : 3.7V */
		offset = 0x0;
	else if (fv_mv < 3900)
		/* BATREG = 3.75 ~ 3.85V in 0.05V steps */
		offset = ((fv_mv - 3700) / 50);
	else if (fv_mv < 4050)
		/* BATREG = 3.90, 4.0V in 0.1V steps */
		offset = (((fv_mv - 3900) / 100) + 4);
	else if (fv_mv < 4630)
		/* BATREG = 4.05 ~ 4.62V in 0.01V steps */
		offset = (((fv_mv - 4050) / 10) + 6);
	else
		/* maximum : 4.62V */
		offset = 0x3f;

	pr_info("sm5038-charger: %s: set as  (mV=%d) batreg Control\n",
							__func__, fv_mv);

	sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CHGCNTL4,
							(offset & 0x3f), 0x3f);

	charger->float_voltage = fv_mv;

	return 0;
}

static int somc_sm5038_create_votables(struct sm5038_charger_data *charger)
{
	int rc = 0;

	charger->usb_icl_votable = create_votable("USB_ICL", VOTE_MIN,
						somc_sm5038_usb_icl_vote_cb,
						charger);
	if (IS_ERR(charger->usb_icl_votable)) {
		rc = PTR_ERR(charger->usb_icl_votable);
		charger->usb_icl_votable = NULL;
		goto out;
	}

	charger->fcc_votable = create_votable("FCC", VOTE_MIN,
						somc_sm5038_fcc_vote_cb,
						charger);
	if (IS_ERR(charger->fcc_votable)) {
		rc = PTR_ERR(charger->fcc_votable);
		charger->fcc_votable = NULL;
		goto out;
	}

	charger->fv_votable = create_votable("FV", VOTE_MIN,
						somc_sm5038_fv_vote_cb,
						charger);
	if (IS_ERR(charger->fv_votable)) {
		rc = PTR_ERR(charger->fv_votable);
		charger->fv_votable = NULL;
		goto out;
	}

	charger->fake_chg_votable = create_votable("FAKE_CHG",
						VOTE_SET_ANY, NULL, NULL);
	if (IS_ERR(charger->fake_chg_votable)) {
		rc = PTR_ERR(charger->fake_chg_votable);
		charger->fake_chg_votable = NULL;
		return rc;
	}

	charger->fake_chg_disallow_votable = create_votable("FAKE_CHG_DISALLOW",
						VOTE_SET_ANY, NULL, NULL);
	if (IS_ERR(charger->fake_chg_disallow_votable)) {
		rc = PTR_ERR(charger->fake_chg_disallow_votable);
		charger->fake_chg_disallow_votable = NULL;
		return rc;
	}

out:
	return rc;
}

static void somc_sm5038_destroy_votables(struct sm5038_charger_data *charger)
{
	if (charger->usb_icl_votable) {
		destroy_votable(charger->usb_icl_votable);
		charger->usb_icl_votable = NULL;
	}
	if (charger->fcc_votable) {
		destroy_votable(charger->fcc_votable);
		charger->fcc_votable = NULL;
	}
	if (charger->fv_votable) {
		destroy_votable(charger->fv_votable);
		charger->fv_votable = NULL;
	}
	if (charger->fake_chg_votable) {
		destroy_votable(charger->fake_chg_votable);
		charger->fake_chg_votable = NULL;
	}
	if (charger->fake_chg_disallow_votable) {
		destroy_votable(charger->fake_chg_disallow_votable);
		charger->fake_chg_disallow_votable = NULL;
	}
}

static void somc_sm5038_offchg_termination_work(struct work_struct *work)
{
	struct sm5038_charger_data *charger = container_of(work,
		struct sm5038_charger_data, offchg_termination_work.work);

	/* key event for power off charge */
	pr_err("input_report_key KEY_F24\n");
	input_report_key(charger->unplug_key, KEY_F24, 1);
	input_sync(charger->unplug_key);
	input_report_key(charger->unplug_key, KEY_F24, 0);
	input_sync(charger->unplug_key);
}

static ssize_t bootup_shutdown_phase_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	if (kstrtoint(buf, 10, &charger->bootup_shutdown_phase))
		return -EINVAL;

	return count;
}
static ssize_t bootup_shutdown_phase_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					charger->bootup_shutdown_phase);
}
static CLASS_ATTR_RW(bootup_shutdown_phase);

static ssize_t smart_charging_interruption_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	bool smart_charge_suspended;
	int val;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		smart_charge_suspended = true;
	else if (val == 0)
		smart_charge_suspended = false;
	else
		return -EINVAL;

	rc = somc_sm5038_smart_set_suspend(charger, smart_charge_suspended);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t smart_charging_interruption_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *sm5038_dev = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (is_client_vote_enabled(sm5038_dev->fcc_votable, SMART_EN_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(smart_charging_interruption);

static ssize_t smart_charging_status_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *sm5038_dev = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (is_client_vote_enabled(sm5038_dev->fcc_votable, SMART_EN_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RO(smart_charging_status);

static ssize_t smart_charging_activation_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *sm5038_dev = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		sm5038_dev->smart_charge_enabled = true;
	else if (val == 0)
		sm5038_dev->smart_charge_enabled = false;
	else
		return -EINVAL;

	return count;
}
static ssize_t smart_charging_activation_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *sm5038_dev = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					sm5038_dev->smart_charge_enabled);
}
static CLASS_ATTR_RW(smart_charging_activation);

static ssize_t usb_input_suspend_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;
	bool enabled;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		enabled = true;
	else if (val == 0)
		enabled = false;
	else
		return -EINVAL;

	rc = vote(charger->usb_icl_votable, USER_SUSPEND_VOTER, enabled, 0);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t usb_input_suspend_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (is_client_vote_enabled(charger->usb_icl_votable,
							USER_SUSPEND_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(usb_input_suspend);

static ssize_t system_temp_level_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	if (val < 0 || val > MAX_THERM_LEVEL)
		return -EINVAL;

	charger->system_temp_level = val;
	somc_sm5038_apply_thermal_mitigation(charger);

	return count;
}
static ssize_t system_temp_level_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					charger->system_temp_level);
}
static CLASS_ATTR_RW(system_temp_level);

static ssize_t system_temp_level_max_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", MAX_THERM_LEVEL);
}
static CLASS_ATTR_RO(system_temp_level_max);

static ssize_t batt_aging_level_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	if (val < 0 || val >= AGING_LEVEL_NUM)
		return -EINVAL;

	somc_sm5038_set_batt_aging_level(charger, val);

	return count;
}
static ssize_t batt_aging_level_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					charger->batt_aging_level);
}
static CLASS_ATTR_RW(batt_aging_level);

static ssize_t batt_soc_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	int val;

	val = somc_sm5038_fg_get_prop_batt_soc();

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RO(batt_soc);

static ssize_t batt_id_show(struct class *c,
				struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->batt_id_ohm);
}
static CLASS_ATTR_RO(batt_id);

static ssize_t is_invalid_batt_id_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->is_invalid_batt_id);
}
static CLASS_ATTR_RO(is_invalid_batt_id);

static ssize_t adjust_soc_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->adjust_soc);
}
static CLASS_ATTR_RO(adjust_soc);

static ssize_t soc_adjust_factor_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->cur_soc_adj_coef);
}
static CLASS_ATTR_RO(soc_adjust_factor);

static ssize_t full_soc_hold_en_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->full_soc_hold_en);
}
static CLASS_ATTR_RO(full_soc_hold_en);

static ssize_t full_soc_adjust_en_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->full_soc_adjust_en);
}
static CLASS_ATTR_RO(full_soc_adjust_en);

static ssize_t full_adjust_soc_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->full_adjust_soc);
}
static CLASS_ATTR_RO(full_adjust_soc);

static ssize_t pd_icl_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	if (val < 0)
		return -EINVAL;

	if (is_client_vote_enabled(charger->usb_icl_votable, PD_VOTER))
		vote(charger->usb_icl_votable, PD_VOTER, true, val);

	return count;
}
static ssize_t pd_icl_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (is_client_vote_enabled(charger->usb_icl_votable, PD_VOTER))
		val = get_client_vote(charger->usb_icl_votable, PD_VOTER);
	else
		val = -1;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(pd_icl);

static ssize_t proprietary_charger_detected_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
				(int)charger->proprietary_charger_detected);
}
static CLASS_ATTR_RO(proprietary_charger_detected);

static ssize_t typec_current_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	char *typec_current_str[] = {"default", "1.5A", "3.0A"};

	return scnprintf(buf, PAGE_SIZE, "%s\n",
				typec_current_str[charger->typec_current]);
}
static CLASS_ATTR_RO(typec_current);

static ssize_t vcell_max_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->vcell_max_mv);
}
static CLASS_ATTR_RO(vcell_max);

static ssize_t step_target_current_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->target_current_ma);
}
static CLASS_ATTR_RO(step_target_current);

static ssize_t step_target_voltage_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->target_voltage_mv);
}
static CLASS_ATTR_RO(step_target_voltage);

static ssize_t topoff_current_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->topoff_ma);
}
static CLASS_ATTR_RO(topoff_current);

static ssize_t learning_counter_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	int val;

	val = somc_sm5038_fg_learn_get_counter();

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RO(learning_counter);

static ssize_t learned_capacity_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->learned_capacity);
}
static CLASS_ATTR_RO(learned_capacity);

static ssize_t learned_capacity_raw_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					charger->learned_capacity_raw);
}
static CLASS_ATTR_RO(learned_capacity_raw);

static ssize_t full_counter_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->full_counter);
}
static CLASS_ATTR_RO(full_counter);

static ssize_t recharge_counter_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", charger->recharge_counter);
}
static CLASS_ATTR_RO(recharge_counter);

static ssize_t batt_unaged_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	if (val == 1)
		charger->batt_unaged = 1;
	else
		charger->batt_unaged = 0;

	return count;
}
static ssize_t batt_unaged_show(struct class *c,
				struct class_attribute *attr, char *buf)
{
	struct sm5038_charger_data *charger = container_of(c,
				struct sm5038_charger_data, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					charger->batt_unaged);
}
static CLASS_ATTR_RW(batt_unaged);

static ssize_t cc_reconnection_running_show(struct class *c,
					struct class_attribute *attr,
					char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n",
			somc_sm5038_is_cc_reconnection_running() ? 1 : 0);
}
static CLASS_ATTR_RO(cc_reconnection_running);

static struct attribute *somc_bcext_class_attrs[] = {
	&class_attr_bootup_shutdown_phase.attr,
	&class_attr_smart_charging_interruption.attr,
	&class_attr_smart_charging_status.attr,
	&class_attr_smart_charging_activation.attr,
	&class_attr_usb_input_suspend.attr,
	&class_attr_system_temp_level.attr,
	&class_attr_system_temp_level_max.attr,
	&class_attr_batt_aging_level.attr,
	&class_attr_batt_soc.attr,
	&class_attr_batt_id.attr,
	&class_attr_is_invalid_batt_id.attr,
	&class_attr_adjust_soc.attr,
	&class_attr_soc_adjust_factor.attr,
	&class_attr_full_soc_hold_en.attr,
	&class_attr_full_soc_adjust_en.attr,
	&class_attr_full_adjust_soc.attr,
	&class_attr_pd_icl.attr,
	&class_attr_proprietary_charger_detected.attr,
	&class_attr_typec_current.attr,
	&class_attr_vcell_max.attr,
	&class_attr_step_target_current.attr,
	&class_attr_step_target_voltage.attr,
	&class_attr_topoff_current.attr,
	&class_attr_learning_counter.attr,
	&class_attr_learned_capacity.attr,
	&class_attr_learned_capacity_raw.attr,
	&class_attr_full_counter.attr,
	&class_attr_recharge_counter.attr,
	&class_attr_batt_unaged.attr,
	&class_attr_cc_reconnection_running.attr,
	NULL,
};
ATTRIBUTE_GROUPS(somc_bcext_class);

#if defined(CONFIG_DEBUG_FS)
static int debug_mask_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->debug_mask;

	return 0;
}
static int debug_mask_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->debug_mask = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(debug_mask_ops, debug_mask_get, debug_mask_set,
								"%d\n");

static int reg_addr_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->dfs_reg_addr;

	return 0;
}
static int reg_addr_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->dfs_reg_addr = (u8)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_addr_ops, reg_addr_get, reg_addr_set,
								"0x%02llx\n");

static int reg_data_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;
	u8 reg_data;

	sm5038_read_reg(charger->i2c, charger->dfs_reg_addr, &reg_data);
	*val = (u64)reg_data;

	return 0;
}
static int reg_data_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;
	u8 reg_data;

	reg_data = (u8)val;
	sm5038_update_reg(charger->i2c, charger->dfs_reg_addr, reg_data, 0xff);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_data_ops, reg_data_get, reg_data_set,
								"0x%02llx\n");

static int real_temp_debug_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->real_temp_debug;

	return 0;
}
static int real_temp_debug_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->real_temp_debug = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(real_temp_debug_ops, real_temp_debug_get,
						real_temp_debug_set, "%d\n");

static int fake_capacity_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->fake_capacity;

	return 0;
}
static int fake_capacity_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->fake_capacity = (int)val;

	if (charger->psy_chg)
		power_supply_changed(charger->psy_chg);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(fake_capacity_ops, fake_capacity_get,
						fake_capacity_set, "%llu\n");

static int regdump_en_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->regdump_en;

	return 0;
}
static int regdump_en_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->regdump_en = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(regdump_en_ops, regdump_en_get, regdump_en_set,
								"%llu\n");

static int full_recharge_debug_en_get(void *data, u64 *val)
{
	struct sm5038_charger_data *charger = data;

	*val = (u64)charger->full_recharge_debug_en;

	return 0;
}
static int full_recharge_debug_en_set(void *data, u64 val)
{
	struct sm5038_charger_data *charger = data;

	charger->full_recharge_debug_en = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(full_recharge_debug_en_ops,
			full_recharge_debug_en_get,
			full_recharge_debug_en_set, "%llu\n");

static void somc_sm5038_create_debugfs(struct sm5038_charger_data *charger)
{
	struct dentry *file;
	struct dentry *dir;

	dir = debugfs_create_dir("sm5038_charger", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		pr_err("Couldn't create sm5038_charger debugfs rc=%ld\n",
								(long)dir);
		return;
	}

	file = debugfs_create_file("debug_mask", S_IFREG | 0644,
					dir, charger, &debug_mask_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create debug_mask file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_addr", S_IFREG | 0644,
						dir, charger, &reg_addr_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_addr file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_data", S_IFREG | 0644,
						dir, charger, &reg_data_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_data file rc=%ld\n", (long)file);

	file = debugfs_create_file("real_temp_debug", S_IFREG | 0644,
					dir, charger, &real_temp_debug_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create real_temp_debug file rc=%ld\n",
								(long)file);

	file = debugfs_create_file("fake_capacity", S_IFREG | 0644,
					dir, charger, &fake_capacity_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create fake_capacity file rc=%ld\n",
								(long)file);

	file = debugfs_create_file("regdump_en", S_IFREG | 0644,
						dir, charger, &regdump_en_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create regdump_en file rc=%ld\n", (long)file);

	file = debugfs_create_file("full_recharge_debug_en", S_IFREG | 0644,
				dir, charger, &full_recharge_debug_en_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create full_recharge_debug_en file rc=%ld\n",
								(long)file);
}
#else
static void somc_sm5038_create_debugfs(struct sm5038_charger_data *charger)
{
}
#endif

#endif
/* if need to set sm5038 pdata */
//static struct of_device_id sm5038_charger_match_table[] = {
//	{ .compatible = "siliconmitus,sm5038-charger",},
//	{},
//};

static int sm5038_battery_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGE_FULL:
#endif
		return 1;
	default:
		break;
	}

	return 0;
}


static const struct power_supply_desc sm5038_charger_power_supply_desc = {
	.name           = "battery",
	.type           = POWER_SUPPLY_TYPE_BATTERY,
	.get_property   = sm5038_chg_get_property,
	.set_property   = sm5038_chg_set_property,
	.properties     = sm5038_charger_props,
	.num_properties = ARRAY_SIZE(sm5038_charger_props),
	.property_is_writeable = sm5038_battery_prop_is_writeable,
};

static char *sm5038_otg_supply_list[] = {
	"otg",
};

static const struct power_supply_desc otg_power_supply_desc = {
	.name			= "sm5038-otg",
	.type			= POWER_SUPPLY_TYPE_UNKNOWN,
	.get_property	= sm5038_otg_get_property,
	.set_property	= sm5038_otg_set_property,
	.properties		= sm5038_otg_props,
	.num_properties	= ARRAY_SIZE(sm5038_otg_props),
};

int sm5038_charger_probe(struct sm5038_dev *sm5038)
{
	//struct sm5038_dev *sm5038 = dev_get_drvdata(pdev->dev.parent);
	//struct sm5038_platform_data *pdata = dev_get_platdata(sm5038->dev);
	struct sm5038_charger_data *charger;
	struct power_supply_config psy_cfg = {};
	int ret = 0;

	pr_info("sm5038-charger: %s: probe start\n", __func__);


	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

	charger->pdata = kzalloc(sizeof(struct  sm5038_charger_platform_data),
			GFP_KERNEL);
	if (!charger->pdata) {
		pr_err("sm5038-charger: %s: failed to allocate memory\n", __func__);
		ret = -ENOMEM;
		goto err_parse_dt_nomem;
	}

	charger->dev = sm5038->dev;
	charger->i2c = sm5038->charger_i2c;
	charger->otg_on = false;
	charger->irq_aicl_enabled = -1;
	charger->is_input_control_used = false;
	charger->thermal_status = TEMP_BELOW_RANGE;
	charger->thermal_detach = false;
	charger->sw_ovp_enable = false;
	charger->fasttimer_expired = false;

	sm5038->check_chg_reset = sm5038_chg_register_reset;

	mutex_init(&charger->charger_mutex);

	ret = sm5038_charger_parse_dt(charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: failed to find charger dt \n", __func__);
		goto err_parse_dt;
	}
	//platform_set_drvdata(pdev, charger);

	static_charger_data = charger;

	if (charger->i2c == NULL) {
		pr_err("sm5038-charger: %s: i2c NULL\n", SM5038_DEV_NAME, __func__);
		goto err_parse_dt;
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)

	ret = somc_sm5038_create_votables(charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: failed to create votables (ret=%d)\n",
								__func__, ret);
		goto err_create_votables;
	}
#endif

	sm5038_chg_init(charger);
	sm5038_charger_oper_table_init(sm5038);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(charger->usb_icl_votable, CABLE_TYPE_VOTER, true,
							NONE_CABLE_CURRENT);
	vote(charger->usb_icl_votable, PRODUCT_VOTER, true, MICRO_CURR_3P0A);
#endif
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	ret = sm5038_step_chg_n_jeita_init(charger->dev, charger->step_chg_enabled,  charger->sw_jeita_enabled);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Couldn't init sm5038_jeita_init ret =%d\n", __func__, ret);
		return ret;
	}
#endif
	/* Init work_queue, wake_lock for Slow-rate-charging */
	charger->wqueue = create_singlethread_workqueue(dev_name(charger->dev));
	if (!charger->wqueue) {
		pr_err("sm5038-charger: %s: fail to create workqueue\n", __func__);
		return -ENOMEM;
	}
	charger->slow_rate_chg_mode = false;


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
       //wakeup_source_init(charger->aicl_wake_lock, "charger-aicl");   // 4.19 R
       if (!(charger->aicl_wake_lock)) {
               charger->aicl_wake_lock = wakeup_source_create("charger-aicl"); // 4.19 Q
               if (charger->aicl_wake_lock)
                       wakeup_source_add(charger->aicl_wake_lock);
       }
#else
	charger->aicl_wake_lock = wakeup_source_register(NULL, "charger-aicl"); // 5.4 R
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	charger->somc_main_wake_lock = wakeup_source_register(NULL,
							"sm5038-somc-main");

	charger->unplug_wakelock = wakeup_source_register(NULL,
							"unplug_wakelock");
#endif

	INIT_DELAYED_WORK(&charger->aicl_work, aicl_work);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	INIT_DELAYED_WORK(&charger->rechg_work, auto_rechg_work);
	INIT_DELAYED_WORK(&charger->thermal_regulation_work, sm5038_thermal_regulation_work);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	INIT_DELAYED_WORK(&charger->timed_fake_chg_work,
					somc_sm5038_timed_fake_chg_work);
	INIT_DELAYED_WORK(&charger->smart_charge_wdog_work,
					somc_sm5038_smart_charge_wdog_work);
	INIT_DELAYED_WORK(&charger->somc_main_work, somc_sm5038_main_work);
	INIT_DELAYED_WORK(&charger->somc_full_work, somc_sm5038_full_work);
	INIT_DELAYED_WORK(&charger->offchg_termination_work,
					somc_sm5038_offchg_termination_work);
#endif

	INIT_DELAYED_WORK(&charger->wa_sw_ovp_work, sm5038_sw_ovp_work);
	psy_cfg.drv_data = charger;
	psy_cfg.supplied_to = sm5038_supplied_to;
	psy_cfg.num_supplicants = ARRAY_SIZE(sm5038_supplied_to);

	charger->psy_chg = devm_power_supply_register(sm5038->dev, &sm5038_charger_power_supply_desc, &psy_cfg);
	if (!charger->psy_chg) {
		pr_err("sm5038-charger: %s: failed to power supply charger register", __func__);
		goto err_power_supply_register;
	}

	charger->psy_otg = power_supply_register(sm5038->dev, &otg_power_supply_desc, &psy_cfg);
	if (!charger->psy_otg) {
		pr_err("sm5038-charger: %s: failed to power supply otg register ", __func__);
		goto err_power_supply_register_otg;
	}
	charger->psy_otg->supplied_to = sm5038_otg_supply_list;
	charger->psy_otg->num_supplicants = ARRAY_SIZE(sm5038_otg_supply_list);

	ret = sm5038_chg_create_attrs(&charger->psy_chg->dev);
	if (ret) {
		pr_err("sm5038-charger: %s : Failed to create_attrs\n", __func__);
		goto err_reg_irq;
	}

	/* Request IRQs */
	charger->irq_vbuspok = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT1_VBUSPOK;
	ret = request_threaded_irq(charger->irq_vbuspok, NULL,
			chg_vbuspok_isr, 0, "vbuspok-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request vbuspok-irq:%d (ret=%d)\n",
					__func__, charger->irq_vbuspok, ret);
		goto err_reg_irq;
	}

	charger->irq_wpcinpok = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT1_WPCINPOK;
	ret = request_threaded_irq(charger->irq_wpcinpok, NULL,
			chg_wpcinpok_isr, 0, "wpcinpok-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request wpcinpok-irq:%d (ret=%d)\n",
					__func__, charger->irq_wpcinpok, ret);
		goto err_reg_irq;
	}

	charger->irq_aicl = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT2_AICL;

	charger->irq_done = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT2_DONE;
	ret = request_threaded_irq(charger->irq_done, NULL,
			chg_done_isr, 0, "done-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request done-irq:%d (ret=%d)\n",
			__func__, charger->irq_done, ret);
		goto err_reg_irq;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	charger->irq_chgon = sm5038->pdata->irq_base +
						SM5038_CHG_IRQ_INT2_CHGON;
	ret = request_threaded_irq(charger->irq_chgon, NULL,
			chg_chgon_isr, 0, "chgon-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request chgon-irq:%d (ret=%d)\n",
			__func__, charger->irq_chgon, ret);
		goto err_reg_irq;
	}

#endif
	charger->irq_vsysovp = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT3_VSYSOVP;
	ret = request_threaded_irq(charger->irq_vsysovp, NULL,
			chg_vsysovp_isr, 0, "vsysovp-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request vsysovp-irq:%d (ret=%d)\n",
			__func__, charger->irq_vsysovp, ret);
		goto err_reg_irq;
	}

	charger->irq_vbusuvlo = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT1_VBUSUVLO;
	ret = request_threaded_irq(charger->irq_vbusuvlo, NULL,
			chg_vbusuvlo_isr, 0, "vbusuvlo-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request vbusuvlo-irq:%d (ret=%d)\n",
			__func__, charger->irq_vbusuvlo, ret);
		goto err_reg_irq;
	}

	charger->irq_fasttmroff = sm5038->irq_base + SM5038_CHG_IRQ_INT3_FASTTMROFF;
	ret = request_threaded_irq(charger->irq_fasttmroff, NULL,
			chg_fasttmroff_isr, 0, "fasttmroff-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request fasttmroff-irq:%d (ret=%d)\n",
			__func__, charger->irq_fasttmroff, ret);
		goto err_reg_irq;
	}

	charger->irq_otgfail = sm5038->irq_base + SM5038_CHG_IRQ_INT3_OTGFAIL;
	ret = request_threaded_irq(charger->irq_otgfail, NULL,
			chg_otgfail_isr, 0, "otgfail-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request otgfail-irq:%d (ret=%d)\n",
			__func__, charger->irq_otgfail, ret);
		goto err_reg_irq;
	}

	charger->irq_topoff = sm5038->pdata->irq_base + SM5038_CHG_IRQ_INT2_TOPOFF;
	ret = request_threaded_irq(charger->irq_topoff, NULL,
			chg_topoff_isr, 0, "topoff-irq", charger);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: fail to request topoff-irq:%d (ret=%d)\n",
			__func__, charger->irq_topoff, ret);
		goto err_reg_irq;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_sm5038_get_active_panel(charger);

	charger->fb_notifier.notifier_call =
				somc_sm5038_apply_thermal_mitigation_cb;
	if (charger->active_panel) {
		ret = drm_panel_notifier_register(charger->active_panel,
							&charger->fb_notifier);
		if (ret < 0) {
			pr_err("sm5038-charger: %s: failed to register panel notifier (ret=%d)\n",
								__func__, ret);
			goto err_panel_register;
		}
	}

	/* register input device */
	charger->unplug_key = input_allocate_device();
	if (!charger->unplug_key) {
		pr_err("sm5038-charger: %s: can't allocate unplug virtual button\n",
								__func__);
		ret = -ENOMEM;
		goto err_input_device_register;
	}

	input_set_capability(charger->unplug_key, EV_KEY, KEY_F24);
	charger->unplug_key->name = "SOMC Charger Removal";
	charger->unplug_key->dev.parent = charger->dev;
	ret = input_register_device(charger->unplug_key);
	if (ret) {
		pr_err("sm5038-charger: %s: can't register power key: %d\n",
								__func__, ret);
		ret = -ENOMEM;
		goto err_input_device_register;
	}

	charger->bcext_class.name = "battchg_ext";
	charger->bcext_class.class_groups = somc_bcext_class_groups;
	ret = class_register(&charger->bcext_class);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: failed to create bcext_class rc=%d\n",
								__func__, ret);
		goto err_class_register;
	}

	charger->fake_capacity = 999;
	somc_sm5038_create_debugfs(charger);
	charger->debug_mask = PR_CORE;

	somc_sm5038_main_wake_lock(charger, true);
	schedule_delayed_work(&charger->somc_main_work,
			msecs_to_jiffies(SOMC_SM5038_WORK_CHG_POL_TIME_MS));
#endif
	pr_info("sm5038-charger: %s: probe done.\n", __func__);

	return 0;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
err_class_register:
	class_unregister(&charger->bcext_class);
err_panel_register:
err_input_device_register:
	if (charger->unplug_key) {
		input_free_device(charger->unplug_key);
		charger->unplug_key = NULL;
	}
#endif
err_reg_irq:
//	power_supply_unregister(charger->psy_chg);
err_power_supply_register_otg:
	power_supply_unregister(charger->psy_otg);
err_power_supply_register:
	destroy_workqueue(charger->wqueue);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
err_create_votables:
	somc_sm5038_destroy_votables(charger);
#endif
err_parse_dt:
err_parse_dt_nomem:
	mutex_destroy(&charger->charger_mutex);
	kfree(charger);

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_charger_probe);

int sm5038_charger_remove(void)
{
	struct sm5038_charger_data *charger = static_charger_data;

	pr_info("sm5038-charger: %s: ++\n", __func__);

	destroy_workqueue(charger->wqueue);

	if (charger->i2c) {
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		chg_set_enq4fet(charger, false);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1,
							(0x0 << 3), (0x1 << 3));
#endif
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL2, OP_MODE_CHG_ON_VBUS, 0x0F);
	} else {
		pr_err("sm5038-charger: %s: no sm5038_charger i2c client\n", __func__);
	}
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	class_unregister(&charger->bcext_class);
	if (charger->unplug_key) {
		input_free_device(charger->unplug_key);
		charger->unplug_key = NULL;
	}
#endif
/*
	if (charger->psy_chg)
		power_supply_unregister(charger->psy_chg);
*/
	if (charger->psy_otg)
		power_supply_unregister(charger->psy_otg);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_sm5038_destroy_votables(charger);
#endif
	mutex_destroy(&charger->charger_mutex);

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	sm5038_step_chg_n_jeita_deinit();

#endif
	kfree(charger);

	pr_info("sm5038-charger: %s: --\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_remove);

//static int sm5038_charger_suspend(struct device *dev)
//{
//	return 0;
//}

//static int sm5038_charger_resume(struct device *dev)
//{
//	return 0;
//}

void sm5038_charger_shutdown(void)
{
	struct sm5038_charger_data *charger = static_charger_data;

	pr_info("sm5038-charger: %s: ++\n", __func__);

	if (charger->i2c) {
		u8 reg;

		/* disable charger */
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		chg_set_enq4fet(charger, false);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL1,
							(0x0 << 3), (0x1 << 3));
#endif
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_CNTL2, OP_MODE_CHG_ON_VBUS, 0x0F);
		/* set input current 500mA*/
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_WPCINCNTL2, (0x0C << 0), (0x7F << 0)); /* 400mA */
		sm5038_update_reg(charger->i2c, SM5038_CHG_REG_VBUSCNTL, (0x10 << 0), (0x7F << 0));	/* 500mA */
		/* disable bypass mode */
		sm5038_read_reg(charger->i2c, SM5038_CHG_REG_FACTORY1, &reg);
		if (reg & 0x02) {
			pr_info("sm5038-charger: %s: bypass mode is enabled\n", __func__);
			//chg_set_en_bypass_mode(charger, false);
		}
	} else {
		pr_err("sm5038-charger: %s: not sm5038 i2c client", __func__);
	}

	charger->thermal_detach = false;
	charger->sw_ovp_enable = false;
	charger->fasttimer_expired = false;
	
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cancel_delayed_work_sync(&charger->rechg_work);
	cancel_delayed_work_sync(&charger->thermal_regulation_work);
#endif
	cancel_delayed_work_sync(&charger->wa_sw_ovp_work);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	cancel_delayed_work_sync(&charger->somc_main_work);
#endif

	pr_info("sm5038-charger: %s: --\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_charger_shutdown);

//static SIMPLE_DEV_PM_OPS(sm5038_charger_pm_ops, sm5038_charger_suspend,
//		sm5038_charger_resume);

//static struct platform_driver sm5038_charger_driver = {
//	.driver = {
//		.name	        = "sm5038-charger",
//		.owner	        = THIS_MODULE,
//		.of_match_table = sm5038_charger_match_table,
//		.pm		        = &sm5038_charger_pm_ops,
//	},
//	.probe		= sm5038_charger_probe,
//	.remove		= sm5038_charger_remove,
//	.shutdown	= sm5038_charger_shutdown,
//};

//static int __init sm5038_charger_init(void)
//{
//	pr_info("sm5038-charger: %s:\n", __func__);
//
//	return platform_driver_register(&sm5038_charger_driver);
//}
//module_init(sm5038_charger_init);
//
//static void __exit sm5038_charger_exit(void)
//{
//	platform_driver_unregister(&sm5038_charger_driver);
//}
//module_exit(sm5038_charger_exit);

//MODULE_LICENSE("GPL");
//MODULE_DESCRIPTION("Charger driver for SM5038");

MODULE_LICENSE("GPL");
