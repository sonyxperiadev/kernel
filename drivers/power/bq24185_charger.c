/*
 * linux/drivers/power/bq24185_charger.c
 *
 * TI BQ24185 Switch-Mode One-Cell Li-Ion charger
 *
 * Copyright (C) 2010-2012 Sony Ericsson Mobile Communications AB.
 *
 * Authors: James Jacobsson <james.jacobsson@sonyericsson.com>
 *          Imre Sunyi <imre.sunyi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/i2c/bq24185_charger.h>

#include <asm/mach-types.h>

/* #define DEBUG_FS */

#define REG_STATUS	0x00
#define REG_CONTROL	0x01
#define REG_BR_VOLTAGE	0x02
#define REG_VENDOR	0x03
#define REG_TERMINATION	0x04
#define REG_DPM		0x05
#define REG_SAFETY_LIM	0x06
#define REG_NTC		0x07
#define REG_BOOST	0x08

#define WATCHDOG_TIMER		10 /* HW has 12s but have 2s as margin */

#define REG_STATUS_FAULT_MASK	0x07
#define REG_STATUS_BOOST_BIT	3
#define REG_STATUS_STAT_MASK	0x30
#define REG_STATUS_EN_STAT_BIT	6
#define REG_STATUS_TMR_RST_BIT	7

#define REG_CONTROL_HZ_MODE_BIT		1
#define REG_CONTROL_CE_BIT		2
#define REG_CONTROL_TE_BIT		3
#define REG_CONTROL_IIN_LIM_MASK	0xC0

#define REG_BR_VOLTAGE_MASK	0xFC

#define REG_VENDOR_REV_MASK	0x07
#define REG_VENDOR_CODE_MASK	0xE0
#define REG_VENDOR_CODE		0x02

#define REG_TERMINATION_VITERM_MASK	0x07
#define REG_TERMINATION_VICHRG_MASK	0x78
#define REG_TERMINATION_RESET_BIT	7

#define REG_DPM_VINDPM_MASK	0x07
#define REG_DPM_LOW_CHG_BIT	5

#define REG_SAFETY_LIM_VMREG_MASK	0x0F
#define REG_SAFETY_LIM_VMCHRG_MASK	0xF0

#define REG_NTC_TS_EN_BIT	3
#define REG_NTC_TMR_MASK	0x60

#define REG_BOOST_OPA_MODE_BIT	3

/* Values in mV */
#define MIN_CHARGE_VOLTAGE	3500
#define MAX_CHARGE_VOLTAGE	4440
#define MIN_MAX_CHARGE_VOLTAGE	4200
#define MAX_MAX_CHARGE_VOLTAGE	MAX_CHARGE_VOLTAGE
#define CHARGE_VOLTAGE_STEP	20

/* Values in mA */
#define LOW_CHARGE_CURRENT(x)	((350 * BQ24185_RSENS_REF) / (x))
#define MIN_CHARGE_CURRENT(x)	((550 * BQ24185_RSENS_REF) / (x))
#define MAX_CHARGE_CURRENT(x)	((1550 * BQ24185_RSENS_REF) / (x))
#define CHARGE_CURRENT_STEP(x)	((100 * BQ24185_RSENS_REF) / (x))

/* Values in mA */
#define MIN_CHARGE_TERM_CURRENT(x)	((25 * BQ24185_RSENS_REF) / (x))
#define MAX_CHARGE_TERM_CURRENT(x)	((200 * BQ24185_RSENS_REF) / (x))
#define CHARGE_TERM_CURRENT_STEP(x)	((25 * BQ24185_RSENS_REF) / (x))

#define SET_BIT(bit, val, data) ((val << bit) | ((data) & ~(1 << bit)))
#define CHK_BIT(bit, data) (((data) & (1 << bit)) >> bit)
#define SET_MASK(mask, val, data) (((data) & ~(mask)) | (val))
#define CHK_MASK(mask, data) ((data) & (mask))
#define DATA_MASK(mask, data) ((data) << (ffs(mask) - 1))

#ifdef DEBUG
#define MUTEX_LOCK(x) do {						\
	struct bq24185_data *_b = container_of(x, struct bq24185_data, lock);\
	dev_dbg(&_b->clientp->dev, "Locking mutex in %s\n", __func__);	\
	mutex_lock(x);							\
} while (0)
#define MUTEX_UNLOCK(x) do {						\
	struct bq24185_data *_b = container_of(x, struct bq24185_data, lock);\
	dev_dbg(&_b->clientp->dev, "Unlocking mutex in %s\n", __func__);\
	mutex_unlock(x);						\
} while (0)
#else
#define MUTEX_LOCK(x) mutex_lock(x)
#define MUTEX_UNLOCK(x) mutex_unlock(x)
#endif /* DEBUG */

enum bq24185_status {
	STAT_READY,
	STAT_CHARGE_IN_PROGRESS,
	STAT_CHARGE_DONE,
	STAT_FAULT,
};

enum bq24185_fault {
	FAULT_NORMAL,
	FAULT_VBUS_OVP,
	FAULT_SLEEP_MODE,
	FAULT_FAULTY_ADAPTER,
	FAULT_DCOUT_LIMIT,
	FAULT_THERMAL_SHUTDOWN,
	FAULT_TIMER_FAULT,
	FAULT_NO_BATTERY,
};

enum bq24185_iin_lim {
	IIN_LIM_100MA,
	IIN_LIM_500MA,
	IIN_LIM_800MA,
	IIN_LIM_NO,
};

enum bq247185_tmr {
	TMR_27MIN,
	TMR_3H,
	TMR_6H,
	TMR_OFF,
};

struct bq24185_status_data {
	enum bq24185_status stat;
	enum bq24185_fault fault;
};

static atomic_t bq24185_init_ok = ATOMIC_INIT(0);

struct bq24185_data {
	struct power_supply bat_ps;
	struct i2c_client *clientp;
	struct delayed_work work;
	struct workqueue_struct *wq;
	struct bq24185_status_data cached_status;
	struct mutex lock;
	struct wake_lock wake_lock;
	struct bq24185_platform_data *control;

	int ext_status;
	int chg_status;
	int bat_present;
	u8 rsens;
	u8 watchdog_enable_vote;
	u8 irq_wake_enabled;
	u8 chg_disable_vote;
	u8 usb_compliant_mode;
	u8 boot_initiated_charging;
	u16 charging_safety_timer;
	u8 chg_disabled_by_voltage;
	u8 chg_disabled_by_current;
	u8 chg_disabled_by_input_current;

	enum bq24185_opa_mode mode;
	u8 vindpm_usb_compliant;
	u8 vindpm_non_compliant;
};

static void bq24185_hz_enable(struct bq24185_data *bd, int enable);

#ifdef DEBUG_FS
static int read_sysfs_interface(const char *pbuf, s32 *pvalue, u8 base)
{
	long long val;
	int rc;

	rc = strict_strtoll(pbuf, base, &val);
	if (!rc)
		*pvalue = (s32)val;

	return rc;
}

static ssize_t store_power_charger(struct device *pdev,
				   struct device_attribute *attr,
				   const char *pbuf,
				   size_t count)
{
	int rc = count;
	s32 onoff;

	if (!read_sysfs_interface(pbuf, &onoff, 10) &&
	    onoff >= 0 && onoff <= 2) {
		int ret;

		if (!onoff)
			ret = bq24185_turn_off_charger();
		else
			ret = bq24185_turn_on_charger(onoff - 1);

		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
			"Expect [0..2] where:\n0: Turn off charger\n"
			"1: Turn on charger in non USB compatible mode\n"
			"2: Turn on charger in USB compatible mode\n");
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_opa(struct device *pdev,
			 struct device_attribute *attr,
			 const char *pbuf,
			 size_t count)
{
	int rc = count;
	s32 opa;

	if (!read_sysfs_interface(pbuf, &opa, 10) &&
	    opa >= 0 && opa <= CHARGER_BOOST_MODE) {
		int ret = bq24185_set_opa_mode(opa);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
			"Expect [0..1] where:\n0: Charger mode\n"
			"1: Boost mode\n");
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_chg_volt(struct device *pdev,
			      struct device_attribute *attr,
			      const char *pbuf,
			      size_t count)
{
	int rc = count;
	s32 mv;

	if (!read_sysfs_interface(pbuf, &mv, 10) &&
	    mv >= 0 && mv <= USHRT_MAX) {
		int ret = bq24185_set_charger_voltage(mv);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
			"Expect [0..%u] mV\n", USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_chg_curr(struct device *pdev,
			      struct device_attribute *attr,
			      const char *pbuf,
			      size_t count)
{
	int rc = count;
	s32 ma;

	if (!read_sysfs_interface(pbuf, &ma, 10) &&
	    ma >= 0 && ma <= USHRT_MAX) {
		int ret = bq24185_set_charger_current(ma);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
			"Expect [0..%u] mA\n", USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_chg_curr_term(struct device *pdev,
				   struct device_attribute *attr,
				   const char *pbuf,
				   size_t count)
{
	int rc = count;
	s32 ma;

	if (!read_sysfs_interface(pbuf, &ma, 10) &&
	    ma >= 0 && ma <= USHRT_MAX) {
		int ret = bq24185_set_charger_termination_current(ma);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. Expect [0..%u] mA\n",
			USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_input_curr_lim(struct device *pdev,
				    struct device_attribute *attr,
				    const char *pbuf,
				    size_t count)
{
	int rc = count;
	s32 ma;

	if (!read_sysfs_interface(pbuf, &ma, 10) &&
	    ma >= 0 && ma <= USHRT_MAX) {
		int ret = bq24185_set_input_current_limit(ma);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. Expect [0..%u] mA\n",
			USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_safety_timer(struct device *pdev,
				  struct device_attribute *attr,
				  const char *pbuf,
				  size_t count)
{
	int rc = count;
	s32 time;

	if (!read_sysfs_interface(pbuf, &time, 10) &&
	    time >= 0 && time <= USHRT_MAX) {
		int ret = bq24185_set_charger_safety_timer(time);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. Expect [0..%u] minutes\n",
			USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}

static struct device_attribute debug_attrs[] = {
	__ATTR(set_power_charger,	0200, NULL, store_power_charger),
	__ATTR(set_opa,			0200, NULL, store_opa),
	__ATTR(set_chg_volt,		0200, NULL, store_chg_volt),
	__ATTR(set_chg_curr,		0200, NULL, store_chg_curr),
	__ATTR(set_chg_curr_term,	0200, NULL, store_chg_curr_term),
	__ATTR(set_input_curr_lim,	0200, NULL, store_input_curr_lim),
	__ATTR(set_safety_timer,	0200, NULL, store_safety_timer),
};

static int debug_create_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(debug_attrs); i++)
		if (device_create_file(dev, &debug_attrs[i]))
			goto debug_create_attrs_failed;

	return 0;

debug_create_attrs_failed:
	dev_err(dev, "Failed creating semc battery attrs.\n");
	while (i--)
		device_remove_file(dev, &debug_attrs[i]);

	return -EIO;
}

static void debug_remove_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(debug_attrs); i++)
		(void)device_remove_file(dev, &debug_attrs[i]);
}
#endif /* DEBUG_FS */

#ifdef DEBUG
static void bq24185_dump_registers(struct bq24185_data *bd)
{
	u8 i;
	s32 data[9];

	for (i = 0; i < 9; i++) {
		data[i] = i2c_smbus_read_byte_data(bd->clientp, i);
		if (data[i] < 0)
			dev_err(&bd->clientp->dev, "Failed dumping reg %u\n",
				i);
	}

	dev_dbg(&bd->clientp->dev, "Regdump\n\t0: 0x%.2x, 1: 0x%.2x, 2: 0x%.2x,"
		" 3: 0x%.2x, 4: 0x%.2x, 5: 0x%.2x, 6: 0x%.2x, 7: 0x%.2x, "
		"8: 0x%.2x\n",
		data[0], data[1], data[2], data[3], data[4],
		data[5], data[6], data[7], data[8]);
}
#endif

static s32 bq24185_i2c_read_byte(struct i2c_client *client, u8 command)
{
	s32 data = i2c_smbus_read_byte_data(client, command);

	if (data < 0)
		dev_err(&client->dev, "I2C Read error  REG: %d "
			"return: %d\n", command, data);
	return data;
}

static s32 bq24185_i2c_write_byte(struct i2c_client *client,
				  u8 command, u8 value)
{
	s32 data = i2c_smbus_write_byte_data(client, command, value);
	if (data < 0)
		dev_err(&client->dev, "I2C Write error REG: %d DATA:"
			" 0x%.2x return: %d\n", command, value, data);
	return data;
}

static int bq24185_enable_charger(struct bq24185_data *bd)
{
	s32 rc;

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disable_vote && --bd->chg_disable_vote) {
		MUTEX_UNLOCK(&bd->lock);
		return 0;
	}

	if (bd->mode != CHARGER_BOOST_MODE)
		bq24185_hz_enable(bd, 0);

	MUTEX_UNLOCK(&bd->lock);

	dev_info(&bd->clientp->dev, "Enabling charger\n");

	rc = bq24185_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (rc < 0)
		return rc;

	return bq24185_i2c_write_byte(bd->clientp, REG_CONTROL,
		 SET_BIT(REG_CONTROL_CE_BIT, 0, rc));
}

static int bq24185_disable_charger(struct bq24185_data *bd)
{
	s32 rc;

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disable_vote++) {
		MUTEX_UNLOCK(&bd->lock);
		return 0;
	}

	if (bd->mode != CHARGER_BOOST_MODE)
		bq24185_hz_enable(bd, 1);

	MUTEX_UNLOCK(&bd->lock);

	dev_info(&bd->clientp->dev, "Disabling charger\n");

	rc = bq24185_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (rc < 0)
		return rc;

	return bq24185_i2c_write_byte(bd->clientp, REG_CONTROL,
		 SET_BIT(REG_CONTROL_CE_BIT, 1, rc));
}

static int bq24185_reset_charger(struct bq24185_data *bd)
{
	s32 data = bq24185_i2c_read_byte(bd->clientp, REG_TERMINATION);

	if (data < 0)
		return data;

	data = SET_BIT(REG_TERMINATION_RESET_BIT, 1, data);
	return bq24185_i2c_write_byte(bd->clientp, REG_TERMINATION, data);
}

static int bq24185_check_status(struct bq24185_data *bd)
{
	s32 status;
	const char *pzstat[] = {
		"Ready", "Charge in progress", "Charge done", "Fault" };
	const char *pzfault[] = {
		"Normal", "VBUS OVP", "Sleep mode",
		"Fault adapter or VBUS<Vuvlo", "DCOUT Current limit tripped",
		"Thermal shutdown or TS Fault",	"Timer fault", "No battery"
	};

	status = bq24185_i2c_read_byte(bd->clientp, REG_STATUS);
	if (status < 0)
		return status;

	MUTEX_LOCK(&bd->lock);
	bd->cached_status.stat = (status & REG_STATUS_STAT_MASK) >>
		(ffs(REG_STATUS_STAT_MASK) - 1);
	bd->cached_status.fault = (status & REG_STATUS_FAULT_MASK) >>
		(ffs(REG_STATUS_FAULT_MASK) - 1);
	if ((CHARGER_BOOST_MODE == bd->mode) &&
		(CHARGER_BOOST_MODE !=
		CHK_BIT(REG_STATUS_BOOST_BIT, status))) {
		dev_dbg(&bd->clientp->dev, "Detect vbus_drop\n");
		if (bd->control && bd->control->notify_vbus_drop)
			bd->control->notify_vbus_drop();
	}
	bd->mode = CHK_BIT(REG_STATUS_BOOST_BIT, status);
	MUTEX_UNLOCK(&bd->lock);

	dev_dbg(&bd->clientp->dev, "--[ Status of %s ]--\n", BQ24185_NAME);
	dev_dbg(&bd->clientp->dev, " Status/Control:\n   PSEL=%s\n"
		"   EN_STAT=%s\n   STAT=%s\n   BOOST=%s\n   FAULT=%s\n",
		 status & (1 << 7) ? "high" : "low",
		 status & (1 << 6) ? "Enabled" : "Disabled",
		 pzstat[bd->cached_status.stat],
		 status & (1 << 3) ? "Boost" : "Charger",
		 pzfault[bd->cached_status.fault]);

	return 0;
}

static void bq24185_update_power_supply(struct bq24185_data *bd)
{
	MUTEX_LOCK(&bd->lock);
	if (bd->ext_status < 0) {
		switch (bd->cached_status.stat) {
		case STAT_READY:
			bd->chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
			break;
		case STAT_CHARGE_IN_PROGRESS:
			bd->chg_status = POWER_SUPPLY_STATUS_CHARGING;
			break;
		case STAT_CHARGE_DONE:
			bd->chg_status = POWER_SUPPLY_STATUS_FULL;
			break;
		case STAT_FAULT:
		default:
			bd->chg_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		}
	} else {
		bd->chg_status = bd->ext_status;
	}

	switch (bd->cached_status.fault) {
	case FAULT_NO_BATTERY:
		bd->bat_present = 0;
		break;
	default:
		bd->bat_present = 1;
		break;
	}
	MUTEX_UNLOCK(&bd->lock);

	power_supply_changed(&bd->bat_ps);
}

static irqreturn_t bq24185_thread_irq(int irq, void *data)
{
	struct bq24185_data *bd = data;
	struct bq24185_status_data old_status = bd->cached_status;

	dev_dbg(&bd->clientp->dev, "Receiving threaded interrupt\n");
	/* Delay the interrupt handling since STATx in register '0' is not
	 * always updated when receiving this.
	 * 300 ms according to TI.
	 */
	msleep(300);

	if (!bq24185_check_status(bd) &&
	    memcmp(&bd->cached_status, &old_status, sizeof bd->cached_status)) {
		dev_info(&bd->clientp->dev, "Charger status: %d\n",
			bd->cached_status.stat);
		bq24185_update_power_supply(bd);
	}

	return IRQ_HANDLED;
}

static void bq24185_hz_enable(struct bq24185_data *bd, int enable)
{
	s32 data = bq24185_i2c_read_byte(bd->clientp, REG_CONTROL);

	if (data < 0)
		return;

	data = SET_BIT(REG_CONTROL_HZ_MODE_BIT, enable, data);
	bq24185_i2c_write_byte(bd->clientp, REG_CONTROL, data);
}

static void bq24185_start_watchdog_reset(struct bq24185_data *bd)
{
	MUTEX_LOCK(&bd->lock);
	if (bd->watchdog_enable_vote++) {
		MUTEX_UNLOCK(&bd->lock);
		return;
	}
	MUTEX_UNLOCK(&bd->lock);

	wake_lock(&bd->wake_lock);

	(void)queue_delayed_work(bd->wq, &bd->work, 0);
}

static void bq24185_stop_watchdog_reset(struct bq24185_data *bd)
{
	MUTEX_LOCK(&bd->lock);
	if (!bd->watchdog_enable_vote || --bd->watchdog_enable_vote) {
		MUTEX_UNLOCK(&bd->lock);
		return;
	}

	/* Clear the 'disabled_by_xxx' flags.
	 * If asic HW watchdog has been timed out, as it can from this
	 * function, then asic will be resetted to default mode.
	 * In default mode it will start charging as soon as VBUS goes high
	 * again. Clearing these flags will allow this driver to set initial
	 * settings that will put asic in non-charging mode when external
	 * turns on the charger again.
	 */
	bd->chg_disabled_by_voltage = 0;
	bd->chg_disabled_by_current = 0;
	bd->chg_disabled_by_input_current = 0;
	bd->chg_disable_vote = 0;
	MUTEX_UNLOCK(&bd->lock);

	cancel_delayed_work(&bd->work);

	wake_unlock(&bd->wake_lock);
}

static void bq24185_reset_watchdog_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct bq24185_data *bd =
		container_of(dwork, struct bq24185_data, work);
	s32 data = bq24185_i2c_read_byte(bd->clientp, REG_STATUS);

	if (data >= 0) {
		data = SET_BIT(REG_STATUS_TMR_RST_BIT, 1, data);
		bq24185_i2c_write_byte(bd->clientp, REG_STATUS, data);
	}

	/* bq24185_check_status(bd); */
#ifdef DEBUG
	bq24185_dump_registers(bd);
#endif

	(void)queue_delayed_work(bd->wq, &bd->work, HZ * WATCHDOG_TIMER);
}

static int bq24185_bat_get_property(struct power_supply *bat_ps,
				    enum power_supply_property psp,
				    union power_supply_propval *val)
{
	struct bq24185_data *bd =
		container_of(bat_ps, struct bq24185_data, bat_ps);

	MUTEX_LOCK(&bd->lock);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = bd->chg_status;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = bd->bat_present;
		break;
	default:
		MUTEX_UNLOCK(&bd->lock);
		return -EINVAL;
	}
	MUTEX_UNLOCK(&bd->lock);
	return 0;
}

static int bq24185_set_input_voltage_dpm(struct bq24185_data *bd)
{
	s32 data;

	data = bq24185_i2c_read_byte(bd->clientp, REG_DPM);
	if (data < 0)
		return data;

	if (bd->usb_compliant_mode)
		data = SET_MASK(REG_DPM_VINDPM_MASK,
				DATA_MASK(REG_DPM_VINDPM_MASK,
					  bd->vindpm_usb_compliant),
				data);
	else
		data = SET_MASK(REG_DPM_VINDPM_MASK,
				DATA_MASK(REG_DPM_VINDPM_MASK,
					  bd->vindpm_non_compliant),
				data);

	return bq24185_i2c_write_byte(bd->clientp, REG_DPM, data);
}

static int bq24185_set_init_values(struct bq24185_data *bd)
{
	s32 data;
	s32 rc;

	dev_info(&bd->clientp->dev, "Set init values\n");

	/* Enable status interrupts */
	data = bq24185_i2c_read_byte(bd->clientp, REG_STATUS);
	if (data < 0)
		return data;

	data = SET_BIT(REG_STATUS_EN_STAT_BIT, 1, data);
	rc = bq24185_i2c_write_byte(bd->clientp, REG_STATUS, data);
	if (rc < 0)
		return rc;

	/* Sets any charging relates registers to 'off' */
	(void)bq24185_set_charger_voltage(0);
	(void)bq24185_set_charger_current(0);
	(void)bq24185_set_charger_termination_current(0);
	(void)bq24185_set_charger_safety_timer(0);

	(void)bq24185_set_input_voltage_dpm(bd);

	return 0;
}

int bq24185_turn_on_charger(u8 usb_compliant)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	int rc;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	dev_info(&bd->clientp->dev, "Turning on charger. USB-%s mode\n",
		 usb_compliant ? "Host" : "Dedicated");

	bd->usb_compliant_mode = usb_compliant;

	rc = bq24185_set_init_values(bd);
	if (rc < 0)
		return rc;

	/* Need to start watchdog reset otherwise HW will reset itself.
	 * If boot has triggered charging the watchdog resetter is already
	 * started.
	 */
	if (!bd->boot_initiated_charging)
		bq24185_start_watchdog_reset(bd);
	else
		bd->boot_initiated_charging = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(bq24185_turn_on_charger);

int bq24185_turn_off_charger(void)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	dev_info(&bd->clientp->dev, "Turning off charger\n");

	bq24185_stop_watchdog_reset(bd);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24185_turn_off_charger);

int bq24185_set_opa_mode(enum bq24185_opa_mode mode)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	s32 data;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);
	data = bq24185_i2c_read_byte(bd->clientp, REG_BOOST);
	if (data < 0)
		return data;

	MUTEX_LOCK(&bd->lock);
	bd->mode = mode;
	MUTEX_UNLOCK(&bd->lock);

	switch (mode) {
	case CHARGER_CHARGER_MODE:
		if (!CHK_BIT(REG_BOOST_OPA_MODE_BIT, data))
			return 0;

		(void)bq24185_set_charger_safety_timer(
			bd->charging_safety_timer);
		bq24185_stop_watchdog_reset(bd);
		bq24185_hz_enable(bd, 1);
		data = SET_BIT(REG_BOOST_OPA_MODE_BIT, 0, data);
		dev_info(&bd->clientp->dev, "Disabling boost mode\n");
		return bq24185_i2c_write_byte(bd->clientp, REG_BOOST, data);
	case CHARGER_BOOST_MODE:
		if (CHK_BIT(REG_BOOST_OPA_MODE_BIT, data))
			return 0;

		(void)bq24185_set_charger_safety_timer(0);
		/* HZ_MODE overrides OPA_MODE. Set no HZ_MODE */
		bq24185_hz_enable(bd, 0);
		bq24185_start_watchdog_reset(bd);
		data = SET_BIT(REG_BOOST_OPA_MODE_BIT, 1, data);
		dev_info(&bd->clientp->dev, "Enabling boost mode\n");
		return bq24185_i2c_write_byte(bd->clientp, REG_BOOST, data);
	default:
		dev_err(&bd->clientp->dev, "Invalid charger mode\n");
	}

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(bq24185_set_opa_mode);

int bq24185_get_opa_mode(enum bq24185_opa_mode *mode)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;

	if (!mode)
		return -EPERM;

	if (!psy)
		return -EAGAIN;

	bd = container_of(psy, struct bq24185_data, bat_ps);

	MUTEX_LOCK(&bd->lock);
	*mode = bd->mode;
	MUTEX_UNLOCK(&bd->lock);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24185_get_opa_mode);

int bq24185_set_charger_voltage(u16 mv)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	u8 voreg;
	s32 data;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	if (mv < MIN_CHARGE_VOLTAGE) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_voltage) {
			bd->chg_disabled_by_voltage = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24185_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	voreg = (min_t(u16, mv, MAX_CHARGE_VOLTAGE) - MIN_CHARGE_VOLTAGE) /
		CHARGE_VOLTAGE_STEP;
	dev_info(&bd->clientp->dev, "Setting charger voltage to %u mV\n",
		 MIN_CHARGE_VOLTAGE + voreg * CHARGE_VOLTAGE_STEP);
	data = bq24185_i2c_read_byte(bd->clientp, REG_BR_VOLTAGE);
	if (data < 0)
		return data;
	if (CHK_MASK(REG_BR_VOLTAGE_MASK, data) !=
	    DATA_MASK(REG_BR_VOLTAGE_MASK, voreg)) {
		data = SET_MASK(REG_BR_VOLTAGE_MASK,
				DATA_MASK(REG_BR_VOLTAGE_MASK, voreg),
				data);
		rc = bq24185_i2c_write_byte(bd->clientp, REG_BR_VOLTAGE,
						data);
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_voltage) {
		bd->chg_disabled_by_voltage = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24185_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24185_set_charger_voltage);

int bq24185_set_charger_current(u16 ma)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	s32 data;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	if (ma < LOW_CHARGE_CURRENT(bd->rsens)) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_current) {
			bd->chg_disabled_by_current = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24185_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	data = bq24185_i2c_read_byte(bd->clientp, REG_DPM);
	if (data < 0)
		return data;

	if (ma < MIN_CHARGE_CURRENT(bd->rsens)) {
		dev_info(&bd->clientp->dev,
			 "Setting charger current to %u mA\n",
			 LOW_CHARGE_CURRENT(bd->rsens));
		if (!CHK_BIT(REG_DPM_LOW_CHG_BIT, data)) {
			data = SET_BIT(REG_DPM_LOW_CHG_BIT, 1, data);
			rc = bq24185_i2c_write_byte(bd->clientp, REG_DPM,
							data);
		}
	} else {
		u8 vichrg = (min_t(u16, ma, MAX_CHARGE_CURRENT(bd->rsens)) -
			     MIN_CHARGE_CURRENT(bd->rsens)) /
			CHARGE_CURRENT_STEP(bd->rsens);
		dev_info(&bd->clientp->dev,
			 "Setting charger current to %u mA\n",
			 MIN_CHARGE_CURRENT(bd->rsens) +
			 vichrg * CHARGE_CURRENT_STEP(bd->rsens));

		if (CHK_BIT(REG_DPM_LOW_CHG_BIT, data)) {
			data = SET_BIT(REG_DPM_LOW_CHG_BIT, 0, data);
			rc = bq24185_i2c_write_byte(bd->clientp, REG_DPM,
							data);
			if (rc < 0)
				return rc;
		}

		data = bq24185_i2c_read_byte(bd->clientp, REG_TERMINATION);
		if (data < 0)
			return data;

		if (CHK_MASK(REG_TERMINATION_VICHRG_MASK, data) !=
		    DATA_MASK(REG_TERMINATION_VICHRG_MASK, vichrg)) {
			data = SET_MASK(REG_TERMINATION_VICHRG_MASK,
					DATA_MASK(REG_TERMINATION_VICHRG_MASK,
						  vichrg), data);
			rc = bq24185_i2c_write_byte(bd->clientp,
							REG_TERMINATION, data);
		}
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_current) {
		bd->chg_disabled_by_current = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24185_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24185_set_charger_current);

int bq24185_set_charger_termination_current(u16 ma)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	s32 data;
	u8 viterm;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	data = bq24185_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return data;

	if (ma < MIN_CHARGE_TERM_CURRENT(bd->rsens)) {
		if (CHK_BIT(REG_CONTROL_TE_BIT, data)) {
			data = SET_BIT(REG_CONTROL_TE_BIT, 0, data);
			dev_info(&bd->clientp->dev,
				 "Disable charge current termination\n");
			rc = bq24185_i2c_write_byte(bd->clientp,
							REG_CONTROL, data);
		}
		return rc;
	}

	if (!CHK_BIT(REG_CONTROL_TE_BIT, data)) {
		data = SET_BIT(REG_CONTROL_TE_BIT, 1, data);
		dev_info(&bd->clientp->dev,
			 "Enable charge current termination\n");
		rc = bq24185_i2c_write_byte(bd->clientp, REG_CONTROL, data);
		if (rc < 0)
			return rc;
	}

	viterm = (clamp_val(ma, MIN_CHARGE_TERM_CURRENT(bd->rsens),
			    MAX_CHARGE_TERM_CURRENT(bd->rsens)) -
		  MIN_CHARGE_TERM_CURRENT(bd->rsens))
		/ CHARGE_TERM_CURRENT_STEP(bd->rsens);

	data = bq24185_i2c_read_byte(bd->clientp, REG_TERMINATION);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_TERMINATION_VITERM_MASK, data) ==
	    DATA_MASK(REG_TERMINATION_VITERM_MASK, viterm))
		return 0;

	data = SET_MASK(REG_TERMINATION_VITERM_MASK,
			DATA_MASK(REG_TERMINATION_VITERM_MASK, viterm), data);
	dev_info(&bd->clientp->dev, "Charge current termination set to %u mA\n",
		 25 + viterm * 25);
	return bq24185_i2c_write_byte(bd->clientp, REG_TERMINATION, data);
}
EXPORT_SYMBOL_GPL(bq24185_set_charger_termination_current);

int bq24185_set_charger_safety_timer(u16 minutes)
{
	const char *hwtime[] = {
		"27 minutes", "3 hours", "6 hours", "disable"
	};
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	enum bq247185_tmr safety_timer;
	s32 data;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	MUTEX_LOCK(&bd->lock);

	if (CHARGER_BOOST_MODE == bd->mode && minutes) {
		/* In boost mode safety timer must be disabled.
		 * Enable safety timer when leaving that mode.
		 */
		bd->charging_safety_timer = minutes;
		minutes = 0;
	} else if (CHARGER_BOOST_MODE != bd->mode) {
		bd->charging_safety_timer = minutes;
	}

	MUTEX_UNLOCK(&bd->lock);

	if (!minutes)
		safety_timer = TMR_OFF;
	else if (minutes <= 27)
		safety_timer = TMR_27MIN;
	else if (minutes <= 180)
		safety_timer = TMR_3H;
	else
		safety_timer = TMR_6H;

	data = bq24185_i2c_read_byte(bd->clientp, REG_NTC);
	if (data < 0)
		return data;
	if (CHK_MASK(REG_NTC_TMR_MASK, data) ==
	    DATA_MASK(REG_NTC_TMR_MASK, safety_timer))
		return 0;

	dev_info(&bd->clientp->dev, "Set safety timer to %s\n",
		 hwtime[safety_timer]);

	data = SET_MASK(REG_NTC_TMR_MASK,
			DATA_MASK(REG_NTC_TMR_MASK, safety_timer),
			data);
	return bq24185_i2c_write_byte(bd->clientp, REG_NTC, data);
}
EXPORT_SYMBOL_GPL(bq24185_set_charger_safety_timer);

int bq24185_set_input_current_limit(u16 ma)
{
	const char *hwlim[] = {
		"100 mA", "500 mA", "800 mA", "no limit"
	};
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;
	enum bq24185_iin_lim iin_lim;
	s32 data;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	if (ma < 100) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_input_current) {
			bd->chg_disabled_by_input_current = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24185_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	if (ma < 500)
		iin_lim = IIN_LIM_100MA;
	else if (ma < 800)
		iin_lim = IIN_LIM_500MA;
	else if (ma > 800)
		iin_lim = IIN_LIM_NO;
	else
		iin_lim = IIN_LIM_800MA;

	dev_info(&bd->clientp->dev, "Setting input charger current to %s\n",
		 hwlim[iin_lim]);

	data = bq24185_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_CONTROL_IIN_LIM_MASK, data) !=
	    DATA_MASK(REG_CONTROL_IIN_LIM_MASK, iin_lim)) {
		data = SET_MASK(REG_CONTROL_IIN_LIM_MASK,
				DATA_MASK(REG_CONTROL_IIN_LIM_MASK, iin_lim),
				data);
		rc = bq24185_i2c_write_byte(bd->clientp, REG_CONTROL, data);
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_input_current) {
		bd->chg_disabled_by_input_current = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24185_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24185_set_input_current_limit);

int bq24185_set_ext_charging_status(int status)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24185_NAME);
	struct bq24185_data *bd;

	if (!psy)
		return -EINVAL;
	bd = container_of(psy, struct bq24185_data, bat_ps);

	MUTEX_LOCK(&bd->lock);
	bd->ext_status = status;
	MUTEX_UNLOCK(&bd->lock);

	bq24185_update_power_supply(bd);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24185_set_ext_charging_status);

int bq24185_charger_initialized(void)
{
	return (int)atomic_read(&bq24185_init_ok);
}
EXPORT_SYMBOL_GPL(bq24185_charger_initialized);

static int __exit bq24185_remove(struct i2c_client *client)
{
	struct bq24185_data *bd = i2c_get_clientdata(client);

	if (bd->irq_wake_enabled)
		(void)disable_irq_wake(client->irq);

	free_irq(client->irq, bd);

	if (delayed_work_pending(&bd->work))
		cancel_delayed_work_sync(&bd->work);

	destroy_workqueue(bd->wq);

	wake_lock_destroy(&bd->wake_lock);

#ifdef DEBUG_FS
	debug_remove_attrs(bd->bat_ps.dev);
#endif
	power_supply_unregister(&bd->bat_ps);

	i2c_set_clientdata(client, NULL);

	kfree(bd);
	return 0;
}

static const struct i2c_device_id bq24185_id[] = {
	{BQ24185_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, bq24185_id);

static enum power_supply_property bq24185_bat_main_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
};

static int bq24185_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct bq24185_platform_data *pdata = client->dev.platform_data;
	struct bq24185_data *bd;
	s32 buf;
	int rc = 0;

	if (pdata && pdata->gpio_configure) {
		rc = pdata->gpio_configure(1);
		if (rc) {
			pr_err("%s: failed to gpio_configure\n", BQ24185_NAME);
			goto probe_exit;
		}
	}

	/* Make sure we have at least i2c functionality on the bus */
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "No i2c functionality available\n");
		rc = -EIO;
		goto probe_exit_hw_deinit;
	}

	buf = bq24185_i2c_read_byte(client, REG_VENDOR);
	if (buf <= 0) {
		dev_err(&client->dev, "Failed read vendor info\n");
		rc = -EIO;
		goto probe_exit_hw_deinit;
	}

	if (((buf & REG_VENDOR_CODE_MASK) >> 5) == REG_VENDOR_CODE) {
		dev_info(&client->dev, "Found BQ24185, rev 0x%.2x\n", buf & 7);
	} else {
		dev_err(&client->dev, "Invalid vendor code\n");
		rc = -ENODEV;
		goto probe_exit_hw_deinit;
	}

	if (pdata) {
		/* Set the safety limit register. Must be done before any other
		 * registers are written to.
		 */
		buf = bq24185_i2c_write_byte(client, REG_SAFETY_LIM,
						pdata->mbrv | pdata->mccsv);
		if (buf < 0) {
			rc = buf;
			goto probe_exit_hw_deinit;
		}

		buf = bq24185_i2c_read_byte(client, REG_SAFETY_LIM);
		if (buf < 0) {
			rc = buf;
			goto probe_exit_hw_deinit;
		}

		if (buf != (pdata->mbrv | pdata->mccsv)) {
			if (pdata->support_boot_charging) {
				dev_warn(&client->dev,
					 "*** Safety limit could not be "
					 "set!\nPlease check boot accessing "
					 "charger (want 0x%x, got 0x%x) ***",
					 pdata->mbrv | pdata->mccsv, buf);
			} else {
				dev_err(&client->dev,
					"Safety limit could not be set!\n");
				rc = -EPERM;
				goto probe_exit_hw_deinit;
			}
		}
	}

	bd = kzalloc(sizeof(struct bq24185_data), GFP_KERNEL);
	if (!bd) {
		rc = -ENOMEM;
		goto probe_exit_hw_deinit;
	}

	bd->bat_ps.name = BQ24185_NAME;
	bd->bat_ps.type = POWER_SUPPLY_TYPE_BATTERY;
	bd->bat_ps.properties = bq24185_bat_main_props;
	bd->bat_ps.num_properties = ARRAY_SIZE(bq24185_bat_main_props);
	bd->bat_ps.get_property = bq24185_bat_get_property;
	bd->bat_ps.use_for_apm = 1;
	bd->clientp = client;
	bd->rsens = BQ24185_RSENS_REF;
	bd->ext_status = -1;

	pdata = client->dev.platform_data;
	if (pdata) {
		bd->bat_ps.name = pdata->name;
		bd->rsens = pdata->rsens;
		bd->control = pdata;
		bd->vindpm_usb_compliant = pdata->vindpm_usb_compliant;
		bd->vindpm_non_compliant = pdata->vindpm_non_compliant;

		if (pdata->supplied_to) {
			bd->bat_ps.supplied_to = pdata->supplied_to;
			bd->bat_ps.num_supplicants = pdata->num_supplicants;
		}
	}

	if (!bd->rsens) {
		kfree(bd);
		dev_err(&client->dev, "Invalid sense resistance!\n");
		return -EINVAL;
	}

	mutex_init(&bd->lock);
	wake_lock_init(&bd->wake_lock, WAKE_LOCK_SUSPEND,
		       "bq24185_watchdog_lock");

	bd->wq = create_singlethread_workqueue("bq24185worker");
	if (!bd->wq) {
		dev_err(&client->dev, "Failed creating workqueue\n");
		rc = -ENOMEM;
		goto probe_exit_free;
	}

	INIT_DELAYED_WORK(&bd->work, bq24185_reset_watchdog_worker);

	rc = power_supply_register(&client->dev, &bd->bat_ps);
	if (rc) {
		dev_err(&client->dev,
			"Failed registering to power_supply class\n");
		goto probe_exit_work_queue;
	}

	i2c_set_clientdata(client, bd);

	bq24185_check_status(bd);
	bq24185_update_power_supply(bd);

	if (pdata && pdata->support_boot_charging &&
	    STAT_CHARGE_IN_PROGRESS == bd->cached_status.stat) {
		dev_info(&client->dev, "Charging started by boot\n");
		bd->boot_initiated_charging = 1;
		/* Kick the watchdog to not lose the boot setting */
		bq24185_start_watchdog_reset(bd);
	} else {
		dev_info(&client->dev, "Not initialized by boot\n");
		rc = bq24185_reset_charger(bd);
		if (rc)
			goto probe_exit_unregister;

	}

	rc = request_threaded_irq(client->irq, NULL, bq24185_thread_irq,
				  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				  IRQF_DISABLED | IRQF_ONESHOT,
				  "bq24185interrupt", bd);
	if (rc) {
		dev_err(&client->dev, "Failed requesting IRQ\n");
		goto probe_exit_unregister;
	}

	device_init_wakeup(&client->dev, 1);

	rc = enable_irq_wake(client->irq);
	if (rc)
		dev_err(&client->dev,
			"Failed to enable wakeup on IRQ request\n");
	else
		bd->irq_wake_enabled = 1;

#ifdef DEBUG_FS
	if (debug_create_attrs(bd->bat_ps.dev))
		dev_info(&client->dev, "Debug support failed\n");
#endif

	atomic_set(&bq24185_init_ok, 1);
	return 0;

probe_exit_unregister:
	power_supply_unregister(&bd->bat_ps);
probe_exit_work_queue:
	destroy_workqueue(bd->wq);
probe_exit_free:
	wake_lock_destroy(&bd->wake_lock);
	kfree(bd);
probe_exit_hw_deinit:
	if (pdata && pdata->gpio_configure)
		rc = pdata->gpio_configure(0);
probe_exit:
	return rc;
}

static struct i2c_driver bq24185_driver = {
	.driver = {
		   .name = BQ24185_NAME,
		   .owner = THIS_MODULE,
	},
	.probe = bq24185_probe,
	.remove = __exit_p(bq24185_remove),
	.id_table = bq24185_id,
};

static int __init bq24185_init(void)
{
	int rc;

	rc = i2c_add_driver(&bq24185_driver);
	if (rc) {
		pr_err("%s FAILED: i2c_add_driver rc=%d\n", __func__, rc);
		goto init_exit;
	}
	return 0;

init_exit:
	return rc;
}

static void __exit bq24185_exit(void)
{
	i2c_del_driver(&bq24185_driver);
}

module_init(bq24185_init);
module_exit(bq24185_exit);

MODULE_AUTHOR("James Jacobsson, Imre Sunyi");
MODULE_LICENSE("GPLv2");
