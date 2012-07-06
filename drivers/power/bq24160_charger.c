/*
 * linux/drivers/power/bq24160_charger.c
 *
 * TI Bq24160 Dual-Input, Single Cell Switch-Mode Li-Ion charger
 *
 * Copyright (C) 2011-2012 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroyuki Namba <hiroyuki.namba@sonyericsson.com>
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

#include <linux/i2c/bq24160_charger.h>

#include <asm/mach-types.h>

#define REG_STATUS		0x00
#define REG_BR_STATUS		0x01
#define REG_CONTROL		0x02
#define REG_BR_VOLTAGE		0x03
#define REG_VENDOR		0x04
#define REG_TERMINATION		0x05
#define REG_DPM			0x06
#define REG_NTC			0x07

#define WATCHDOG_TIMER		10 /* HW has 12s but have 2s as margin */

#define REG_STATUS_TMR_RST_BIT		7
#define REG_STATUS_STAT_MASK		0x70
#define REG_STATUS_SUPPLY_SEL_BIT	3
#define REG_STATUS_FAULT_MASK		0x07

#define REG_BR_STATUS_INSTAT_MASK	0xC0
#define REG_BR_STATUS_USBSTAT_MASK	0x30
#define REG_BR_STATUS_OTG_LOCK_BIT	3
#define REG_BR_STATUS_BATSTAT_MASK	0x06

#define REG_CONTROL_RESET_BIT		7
#define REG_CONTROL_IUSB_LIM_MASK	0x70
#define REG_CONTROL_EN_STAT_BIT		3
#define REG_CONTROL_TE_BIT		2
#define REG_CONTROL_CE_BIT		1
#define REG_CONTROL_HZ_MODE_BIT		0

#define REG_BR_VOLTAGE_MASK		0xFC
#define REG_BR_VOLTAGE_IIN_LIM_BIT	1

#define REG_VENDOR_REV_MASK		0x07
#define REG_VENDOR_CODE_MASK		0xE0
#define REG_VENDOR_CODE			0x02

#define REG_TERMINATION_ICHRG_MASK	0xF8
#define REG_TERMINATION_ITERM_MASK	0x07

#define REG_DPM_MINSYS_STATUS_BIT	7
#define REG_DPM_DPM_STATUS_BIT		6
#define REG_DPM_VINDPM_USB_MASK		0x38
#define REG_DPM_VINDPM_IN_MASK		0x07

#define REG_NTC_2XTMR_EN_BIT		7
#define REG_NTC_TMR_MASK		0x60
#define REG_NTC_BATGD_EN_BIT		4
#define REG_NTC_TS_EN_BIT		3
#define REG_NTC_TS_FAULT_MASK		0x06
#define REG_NTC_LOW_CHG_BIT		0

/* Values in mV */
#define MIN_CHARGE_VOLTAGE	3500
#define MAX_CHARGE_VOLTAGE	4440
#define MIN_MAX_CHARGE_VOLTAGE	4200
#define MAX_MAX_CHARGE_VOLTAGE	MAX_CHARGE_VOLTAGE
#define CHARGE_VOLTAGE_STEP	20

/* Values in mA */
#define MIN_CHARGE_CURRENT 275
#define LOW_CHARGE_CURRENT 550
#define MAX_CHARGE_CURRENT 2500
#define CHARGE_CURRENT_STEP 75

/* Values in mA */
#define MIN_CHARGE_TERM_CURRENT 50
#define MAX_CHARGE_TERM_CURRENT 400
#define CHARGE_TERM_CURRENT_STEP 50

/* Values in mA */
#define MIN_CHARGE_CURRENT_LIMIT_USB 100
#define MIN_CHARGE_CURRENT_LIMIT_IN 1500

/* Vender Part Revision  */
#define VENDOR_REV_23 0x05

#define STOP_CHARGE_VOLTAGE_REV_23 4000
#define START_CHARGE_VOLTAGE_REV_23 3900


#define CURRENT_DEPEND_ON_PRODUCT USHRT_MAX

/*
 * Delay time until enabling charge to prevent back boost.
 * Add a margin of 20ms from I2C timing analyzing.
 */
#define DELAYED_CHARGE_ENABLE_TIME_MS ((HZ / 5) + (HZ / 50)) /* 200ms + 20ms */
#define DELAYED_HI_Z_TIME_MS (HZ / 20) /* 50ms */
#define PHASE_HZ_MODE_HI 1
#define PHASE_HZ_MODE_LO 2
#define PHASE_CEZ_LO 3

/* Power supply connection mask */
#define NO_CHG 0x00
#define USB_CHG 0x01
#define WALL_CHG 0x02
#define CRADLE_CHG 0x04

#define OTG_UNLOCK 0
#define OTG_LOCK 1

#define SET_BIT(bit, val, data) ((val << bit) | ((data) & ~(1 << bit)))
#define CHK_BIT(bit, data) (((data) & (1 << bit)) >> bit)
#define SET_MASK(mask, val, data) (((data) & ~(mask)) | (val))
#define CHK_MASK(mask, data) ((data) & (mask))
#define DATA_MASK(mask, data) ((data) << (ffs(mask) - 1))

#ifdef DEBUG
#define MUTEX_LOCK(x) do {						\
	struct bq24160_data *_b = container_of(x, struct bq24160_data, lock);\
	dev_dbg(&_b->clientp->dev, "Locking mutex in %s\n", __func__);	\
	mutex_lock(x);							\
} while (0)
#define MUTEX_UNLOCK(x) do {						\
	struct bq24160_data *_b = container_of(x, struct bq24160_data, lock);\
	dev_dbg(&_b->clientp->dev, "Unlocking mutex in %s\n", __func__);\
	mutex_unlock(x);						\
} while (0)
#else
#define MUTEX_LOCK(x) mutex_lock(x)
#define MUTEX_UNLOCK(x) mutex_unlock(x)
#endif /* DEBUG */

enum bq24160_status {
	STAT_NO_VALID_SOURCE,
	STAT_IN_READY,
	STAT_USB_READY,
	STAT_CHARGING_FROM_IN,
	STAT_CHARGING_FROM_USB,
	STAT_CHARGE_DONE,
	STAT_NA,
	STAT_FAULT,
};

enum bq24160_fault {
	FAULT_NORMAL,
	FAULT_THERMAL_SHUTDOWN,
	FAULT_BATT_TEMP_FAULT,
	FAULT_WDOG_TIMER_EXPIRED,
	FAULT_NA,
	FAULT_SAFETY_TIMER_EXPIRED,
	FAULT_SUPPLY_FAULT,
	FAULT_BATTERY_FAULT,
};

enum bq24160_in_status {
	INSTAT_NORMAL,
	INSTAT_SUPPLY_OVP,
	INSTAT_WEAK_SOURCE_CONNECTED,
	INSTAT_FAULTY_ADAPTER,
};
enum bq24160_usb_status {
	USBSTAT_NORMAL,
	USBSTAT_SUPPLY_OVP,
	USBSTAT_WEAK_SOURCE_CONNECTED,
	USBSTAT_FAULTY_ADAPTER,
};
enum bq24160_bat_status {
	BATSTAT_BATTERY_PRESENT,
	BATSTAT_BATTERY_OVP,
	BATSTAT_BATTERY_NOT_PRESENT,
	BATSTAT_BATTERY_NA,
};

enum bq24160_iusb_lim {
	IUSB_LIM_100MA,
	IUSB_LIM_150MA,
	IUSB_LIM_500MA,
	IUSB_LIM_800MA,
	IUSB_LIM_900MA,
	IUSB_LIM_1500MA,
};

enum bq24160_iin_lim {
	IIN_LIM_1500MA,
	IIN_LIM_2500MA,
};

enum bq24160_vindpm_usb {
	VINDPM_USB_4200MV,
	VINDPM_USB_4280MV,
	VINDPM_USB_4360MV,
	VINDPM_USB_4440MV,
	VINDPM_USB_4520MV,
	VINDPM_USB_4600MV,
	VINDPM_USB_4680MV,
	VINDPM_USB_4760MV,
};

enum bq24160_vindpm_in {
	VINDPM_IN_4200MV,
	VINDPM_IN_4280MV,
	VINDPM_IN_4360MV,
	VINDPM_IN_4440MV,
	VINDPM_IN_4520MV,
	VINDPM_IN_4600MV,
	VINDPM_IN_4680MV,
	VINDPM_IN_4760MV,
};

enum bq24160_tmr {
	TMR_27MIN,
	TMR_6H,
	TMR_9H,
	TMR_OFF,
};

enum bq24160_otg {
	OTG_NO_SUPPLY_PRESENT,
	OTG_SUPPLY_PRESENT,
};

enum bq24160_supply_sel {
	SUPPLY_SEL_IN,
	SUPPLY_SEL_USB,
};

struct bq24160_status_data {
	enum bq24160_status stat;
	enum bq24160_fault fault;
	enum bq24160_in_status in_stat;
	enum bq24160_usb_status usb_stat;
	enum bq24160_bat_status batt_stat;
};

static atomic_t bq24160_init_ok = ATOMIC_INIT(0);

struct bq24160_data {
	struct power_supply bat_ps;
	struct i2c_client *clientp;
	struct delayed_work work;
	struct delayed_work enable_work;
	struct workqueue_struct *wq;
	struct bq24160_status_data cached_status;
	struct mutex lock;
	struct wake_lock wake_lock;
	struct bq24160_platform_data *control;

	int ext_status;
	int chg_status;
	int bat_present;
	int otg_lock;
	int vendor_rev;
	u8 watchdog_enable_vote;
	u8 irq_wake_enabled;
	u8 usb_compliant_mode;
	u8 boot_initiated_charging;
	u16 charging_safety_timer;
	u8 chg_disabled_by_voltage;
	u8 chg_disabled_by_current;
	u8 chg_disabled_by_input_current;
	u8 restricted_enable_charger;
	u8 status_update_disregard;
};

static void bq24160_hz_enable(struct bq24160_data *bd, int enable);
static void bq24160_start_delayed_enable(struct bq24160_data *bd,
					unsigned long delay);
static void bq24160_start_watchdog_reset(struct bq24160_data *bd);
static void bq24160_stop_watchdog_reset(struct bq24160_data *bd);

#ifdef DEBUG_FS

static int bq24160_set_input_current_limit_usb(struct bq24160_data *bd, u16 ma);
static int bq24160_set_input_current_limit_in(struct bq24160_data *bd, u16 ma);

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
			ret = bq24160_turn_off_charger();
		else
			ret = bq24160_turn_on_charger(onoff - 1);

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

static ssize_t store_chg_volt(struct device *pdev,
			      struct device_attribute *attr,
			      const char *pbuf,
			      size_t count)
{
	int rc = count;
	s32 mv;

	if (!read_sysfs_interface(pbuf, &mv, 10) &&
	    mv >= 0 && mv <= USHRT_MAX) {
		int ret = bq24160_set_charger_voltage(mv);
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
		int ret = bq24160_set_charger_current(ma);
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
		int ret = bq24160_set_charger_termination_current(ma);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. Expect [0..%u] mA\n",
			USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_input_curr_lim_usb(struct device *pdev,
				    struct device_attribute *attr,
				    const char *pbuf,
				    size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq24160_data *bd =
		container_of(psy, struct bq24160_data, bat_ps);
	int rc = count;
	s32 ma;

	if (!read_sysfs_interface(pbuf, &ma, 10) &&
	    ma >= 0 && ma <= USHRT_MAX) {
		int ret = bq24160_set_input_current_limit_usb(bd, ma);
		if (ret < 0)
			rc = ret;
	} else {
		dev_err(pdev, "Wrong input to sysfs. Expect [0..%u] mA\n",
			USHRT_MAX);
		rc = -EINVAL;
	}

	return rc;
}
static ssize_t store_input_curr_lim_in(struct device *pdev,
				    struct device_attribute *attr,
				    const char *pbuf,
				    size_t count)
{
	struct power_supply *psy = dev_get_drvdata(pdev);
	struct bq24160_data *bd =
		container_of(psy, struct bq24160_data, bat_ps);
	int rc = count;
	s32 ma;

	if (!read_sysfs_interface(pbuf, &ma, 10) &&
	    ma >= 0 && ma <= USHRT_MAX) {
		int ret = bq24160_set_input_current_limit_in(bd, ma);
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
		int ret = bq24160_set_charger_safety_timer(time);
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
	__ATTR(set_chg_volt,		0200, NULL, store_chg_volt),
	__ATTR(set_chg_curr,		0200, NULL, store_chg_curr),
	__ATTR(set_chg_curr_term,	0200, NULL, store_chg_curr_term),
	__ATTR(set_input_curr_lim_usb,	0200, NULL, store_input_curr_lim_usb),
	__ATTR(set_input_curr_lim_in,	0200, NULL, store_input_curr_lim_in),
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
static void bq24160_dump_registers(struct bq24160_data *bd)
{
	u8 i;
	s32 data[8];

	for (i = 0; i < 8; i++) {
		data[i] = i2c_smbus_read_byte_data(bd->clientp, i);
		if (data[i] < 0)
			dev_err(&bd->clientp->dev, "Failed dumping reg %u\n",
				i);
	}

	dev_dbg(&bd->clientp->dev, "Regdump\t0: 0x%.2x, 1: 0x%.2x, 2: 0x%.2x,"
		" 3: 0x%.2x, 4: 0x%.2x, 5: 0x%.2x, 6: 0x%.2x, 7: 0x%.2x\n",
		data[0], data[1], data[2], data[3], data[4],
		data[5], data[6], data[7]);
}
#endif

static s32 bq24160_i2c_read_byte(struct i2c_client *client, u8 command)
{
	s32 value = i2c_smbus_read_byte_data(client, command);
	if (value < 0)
		dev_err(&client->dev, "I2C Read error  REG: %d "
			"return: %d\n", command, value);
	return value;
}

static s32 bq24160_i2c_write_byte(struct i2c_client *client,
				  u8 command, u8 value)
{
	s32 data;

	if (command == REG_CONTROL)
		value = SET_BIT(REG_CONTROL_RESET_BIT, 0, value);

	data = i2c_smbus_write_byte_data(client, command, value);
	if (data < 0)
		dev_err(&client->dev, "I2C Write error REG: %d DATA:"
			" 0x%.2x return: %d\n", command, value, data);
	return data;
}

static void bq24160_set_ce(struct bq24160_data *bd, int disable)
{
	s32 data;

	data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return;

	data = SET_BIT(REG_CONTROL_CE_BIT, disable, data);
	bq24160_i2c_write_byte(bd->clientp, REG_CONTROL, data);
}

static int bq24160_enable_charger(struct bq24160_data *bd)
{
	dev_dbg(&bd->clientp->dev, "%s()\n", __func__);

	MUTEX_LOCK(&bd->lock);
	if (bd->restricted_enable_charger) {
		dev_dbg(&bd->clientp->dev,
		"Cancel enable charger\n");
		MUTEX_UNLOCK(&bd->lock);
		return 0;
	}

	/* Set HZ_MODE=0 before delay about 200ms
	 * following workaround flow chart.
	 */
	bq24160_hz_enable(bd, 0);

	bd->restricted_enable_charger = 1;
	MUTEX_UNLOCK(&bd->lock);
	bq24160_start_delayed_enable(bd, DELAYED_CHARGE_ENABLE_TIME_MS);
	return 0;
}

static int bq24160_disable_charger(struct bq24160_data *bd)
{
	dev_dbg(&bd->clientp->dev, "%s()\n", __func__);

	MUTEX_LOCK(&bd->lock);
	if (bd->restricted_enable_charger) {
		dev_dbg(&bd->clientp->dev,
		"Cancel disable charger\n");
		MUTEX_UNLOCK(&bd->lock);
		return 0;
	}

	MUTEX_UNLOCK(&bd->lock);
	bq24160_start_delayed_enable(bd, 0);
	return 0;
}

static int bq24160_reset_charger(struct bq24160_data *bd)
{
	s32 data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);

	if (data < 0)
		return data;

	data = SET_BIT(REG_CONTROL_RESET_BIT, 1, data);
	return i2c_smbus_write_byte_data(bd->clientp, REG_CONTROL, data);
}

static int bq24160_check_status(struct bq24160_data *bd)
{
	s32 status;
	const char *pzstat[] = {
		"No Valid Source",
		"IN Ready", "USB Ready",
		"Charging from IN", "Charging from USB",
		"Charge done", "NA", "Fault" };
	const char *pzfault[] = {
		"Normal", "Thermal shutdown", "Battery temp fault",
		"Watchdog timer expired", "Safety timer expired",
		"IN Supply Fault", "USB Supply Fault", "Battery fault"
	};

	status = bq24160_i2c_read_byte(bd->clientp, REG_STATUS);
	if (status < 0)
		return status;

	MUTEX_LOCK(&bd->lock);
	bd->cached_status.stat = (status & REG_STATUS_STAT_MASK) >>
		(ffs(REG_STATUS_STAT_MASK) - 1);
	bd->cached_status.fault = (status & REG_STATUS_FAULT_MASK) >>
		(ffs(REG_STATUS_FAULT_MASK) - 1);

	/* If status changes to discharing during disabling charger interval
	 * of workaround, not update the status that shown to other driver.
	 */
	if (bd->restricted_enable_charger) {
		dev_info(&bd->clientp->dev,
			"Charger status: %d (disabling charge interval)\n",
			bd->cached_status.stat);
		bd->status_update_disregard = 1;
		MUTEX_UNLOCK(&bd->lock);
		return -EAGAIN;
	}
	MUTEX_UNLOCK(&bd->lock);

	dev_dbg(&bd->clientp->dev, "--[ Status of %s ]--\n", BQ24160_NAME);
	dev_dbg(&bd->clientp->dev, " Status/Control:\n"
		"   SUPPLY_SEL=%s\n   STAT=%s\n   FAULT=%s\n",
		 status & (1 << 3) ? "USB" : "IN",
		 pzstat[bd->cached_status.stat],
		 pzfault[bd->cached_status.fault]);

	return 0;
}

static void bq24160_update_power_supply(struct bq24160_data *bd)
{
	MUTEX_LOCK(&bd->lock);
	if (bd->ext_status < 0) {
		switch (bd->cached_status.stat) {
		case STAT_NO_VALID_SOURCE:
		case STAT_USB_READY:
		case STAT_IN_READY:
			bd->chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
			break;
		case STAT_CHARGING_FROM_USB:
		case STAT_CHARGING_FROM_IN:
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

	switch (bd->cached_status.batt_stat) {
	case BATSTAT_BATTERY_NOT_PRESENT:
		bd->bat_present = 0;
		break;
	default:
		bd->bat_present = 1;
		break;
	}
	MUTEX_UNLOCK(&bd->lock);

	power_supply_changed(&bd->bat_ps);
}

static irqreturn_t bq24160_thread_irq(int irq, void *data)
{
	struct bq24160_data *bd = data;
	struct bq24160_status_data old_status = bd->cached_status;

	dev_dbg(&bd->clientp->dev, "Receiving threaded interrupt\n");
	/* Delay the interrupt handling since STATx in register '0' is not
	 * always updated when receiving this.
	 * 300 ms according to TI.
	 */
	msleep(300);

	if (!bq24160_check_status(bd) &&
	    memcmp(&bd->cached_status, &old_status, sizeof bd->cached_status)) {
		dev_info(&bd->clientp->dev, "Charger status: %d\n",
			bd->cached_status.stat);
		bq24160_update_power_supply(bd);
	}

	return IRQ_HANDLED;
}

static void bq24160_hz_enable(struct bq24160_data *bd, int enable)
{
	s32 data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return;

	data = SET_BIT(REG_CONTROL_HZ_MODE_BIT, enable, data);
	bq24160_i2c_write_byte(bd->clientp, REG_CONTROL, data);
}

static void bq24160_set_supply_sel(struct bq24160_data *bd, int sel)
{
	s32 data;

	data = bq24160_i2c_read_byte(bd->clientp, REG_STATUS);
	if (data < 0)
		return;

	data = SET_BIT(REG_STATUS_SUPPLY_SEL_BIT, sel, data);
	bq24160_i2c_write_byte(bd->clientp, REG_STATUS, data);
	return;
}

static int bq24160_otg_lock(struct bq24160_data *bd, int lock)
{
	s32 data;

	data = bq24160_i2c_read_byte(bd->clientp, REG_BR_STATUS);
	if (data < 0)
		return data;

	data = SET_BIT(REG_BR_STATUS_OTG_LOCK_BIT, lock, data);
	return bq24160_i2c_write_byte(bd->clientp, REG_BR_STATUS, data);
}

static void bq24160_low_chg_enable(struct bq24160_data *bd, int enable)
{
	s32 data;

	data = bq24160_i2c_read_byte(bd->clientp, REG_NTC);
	if (data < 0)
		return;

	data = SET_BIT(REG_NTC_LOW_CHG_BIT, enable, data);
	bq24160_i2c_write_byte(bd->clientp, REG_NTC, data);
	return;
}

static void bq24160_start_watchdog_reset(struct bq24160_data *bd)
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

static void bq24160_stop_watchdog_reset(struct bq24160_data *bd)
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
	MUTEX_UNLOCK(&bd->lock);

	cancel_delayed_work(&bd->work);

	wake_unlock(&bd->wake_lock);
}

static void bq24160_reset_watchdog_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct bq24160_data *bd =
		container_of(dwork, struct bq24160_data, work);
	s32 data = bq24160_i2c_read_byte(bd->clientp, REG_STATUS);

	if (data >= 0) {
		data = SET_BIT(REG_STATUS_TMR_RST_BIT, 1, data);
		bq24160_i2c_write_byte(bd->clientp, REG_STATUS, data);
	}

	/* bq24160_check_status(bd); */
#ifdef DEBUG
	bq24160_dump_registers(bd);
#endif

	(void)queue_delayed_work(bd->wq, &bd->work, HZ * WATCHDOG_TIMER);
}

static bool bq24160_is_disabled_charger(struct bq24160_data *bd)
{
	dev_dbg(&bd->clientp->dev, "dis_v=%d dis_c=%d dis_i=%d\n",
		bd->chg_disabled_by_voltage,
		bd->chg_disabled_by_current,
		bd->chg_disabled_by_input_current);

	return bd->chg_disabled_by_voltage ||
	       bd->chg_disabled_by_current ||
	       bd->chg_disabled_by_input_current;
}

static void bq24160_start_delayed_enable(struct bq24160_data *bd,
						unsigned long delay)
{
	if (delayed_work_pending(&bd->enable_work))
		cancel_delayed_work(&bd->enable_work);
	queue_delayed_work(bd->wq, &bd->enable_work, delay);
}

static void bq24160_delayed_enable_worker(struct work_struct *work)
{
	s32 data;
	int disabled;
	int update = 0;
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct bq24160_data *bd =
		container_of(dwork, struct bq24160_data, enable_work);

	MUTEX_LOCK(&bd->lock);

	data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0) {
		MUTEX_UNLOCK(&bd->lock);
		return;
	}
	disabled = CHK_BIT(REG_CONTROL_CE_BIT, data);

	if (bq24160_is_disabled_charger(bd)) {
		if (!disabled) {
			dev_info(&bd->clientp->dev, "Disabling charger\n");
			bq24160_hz_enable(bd, 1);
			bq24160_set_ce(bd, 1);
		} else {
			dev_dbg(&bd->clientp->dev,
			"Already charger is Disabled\n");
			if (bd->status_update_disregard)
				update = 1;
		}
	} else {
		if (disabled) {
			if (bd->restricted_enable_charger ==
						PHASE_HZ_MODE_HI) {
				dev_dbg(&bd->clientp->dev,
					"Temporary HZ_MODE=Hi\n");
				bq24160_hz_enable(bd, 1);
				bd->restricted_enable_charger++;
				MUTEX_UNLOCK(&bd->lock);
				bq24160_start_delayed_enable(bd,
					DELAYED_HI_Z_TIME_MS);
				return;
			} else if (bd->restricted_enable_charger ==
						PHASE_HZ_MODE_LO) {
				dev_dbg(&bd->clientp->dev,
					"Temporary HZ_MODE=Lo\n");
				bq24160_hz_enable(bd, 0);
				bd->restricted_enable_charger++;
				MUTEX_UNLOCK(&bd->lock);
				bq24160_start_delayed_enable(bd,
					DELAYED_CHARGE_ENABLE_TIME_MS);
				return;
			} else if (bd->restricted_enable_charger ==
						PHASE_CEZ_LO) {
				dev_info(&bd->clientp->dev,
					"Enabling charger\n");
				bq24160_hz_enable(bd, 0);
				bq24160_set_ce(bd, 0);
			}
		} else {
			dev_dbg(&bd->clientp->dev,
			"Already charger is Enabled\n");
			if (bd->status_update_disregard)
				update = 1;
		}
	}
	bd->restricted_enable_charger = 0;
	bd->status_update_disregard = 0;

	MUTEX_UNLOCK(&bd->lock);

	if (update)
		bq24160_update_power_supply(bd);
}

static int bq24160_bat_get_property(struct power_supply *bat_ps,
				    enum power_supply_property psp,
				    union power_supply_propval *val)
{
	struct bq24160_data *bd =
		container_of(bat_ps, struct bq24160_data, bat_ps);

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


int bq24160_set_input_voltage_dpm_usb(u8 usb_compliant)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 data;

	if (!psy)
		return -EAGAIN;

	bd = container_of(psy, struct bq24160_data, bat_ps);

	data = bq24160_i2c_read_byte(bd->clientp, REG_DPM);
	if (data < 0)
		return data;

	if (bd->vendor_rev == VENDOR_REV_23) {
		data = SET_MASK(REG_DPM_VINDPM_USB_MASK,
				DATA_MASK(REG_DPM_VINDPM_USB_MASK,
					  VINDPM_USB_4200MV),
				data);
	} else {
		if (usb_compliant)
			data = SET_MASK(REG_DPM_VINDPM_USB_MASK,
					DATA_MASK(REG_DPM_VINDPM_USB_MASK,
						  VINDPM_USB_4600MV),
					data);
		else
			data = SET_MASK(REG_DPM_VINDPM_USB_MASK,
					DATA_MASK(REG_DPM_VINDPM_USB_MASK,
						  VINDPM_USB_4280MV),
					data);
	}

	dev_dbg(&bd->clientp->dev,
		"Setting input voltage based DPM(USB)\n");

	return bq24160_i2c_write_byte(bd->clientp, REG_DPM, data);
}
EXPORT_SYMBOL_GPL(bq24160_set_input_voltage_dpm_usb);

int bq24160_set_input_voltage_dpm_in(void)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 data;

	if (!psy)
		return -EAGAIN;

	bd = container_of(psy, struct bq24160_data, bat_ps);

	data = bq24160_i2c_read_byte(bd->clientp, REG_DPM);
	if (data < 0)
		return data;

	if (bd->vendor_rev == VENDOR_REV_23) {
		data = SET_MASK(REG_DPM_VINDPM_IN_MASK,
				DATA_MASK(REG_DPM_VINDPM_IN_MASK,
					  VINDPM_IN_4200MV),
				data);
	} else {
		data = SET_MASK(REG_DPM_VINDPM_IN_MASK,
				DATA_MASK(REG_DPM_VINDPM_IN_MASK,
					  VINDPM_IN_4280MV),
				data);
	}
	dev_dbg(&bd->clientp->dev,
		"Setting input voltage based DPM(IN)\n");

	return bq24160_i2c_write_byte(bd->clientp, REG_DPM, data);
}
EXPORT_SYMBOL_GPL(bq24160_set_input_voltage_dpm_in);

int bq24160_setup_exchanged_power_supply(u8 connection)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	MUTEX_LOCK(&bd->lock);

	bq24160_hz_enable(bd, 1);
	bq24160_set_ce(bd, 1);

	if (connection == USB_CHG || connection == WALL_CHG) {
		dev_info(&bd->clientp->dev,
		"Setting USB: OTG_Lock OFF, SUPPLY_SEL=USB\n");
		(void)bq24160_otg_lock(bd, 0);
		bq24160_set_supply_sel(bd, SUPPLY_SEL_USB);
	} else {
		dev_info(&bd->clientp->dev,
		"Setting IN: OTG_Lock ON, SUPPLY_SEL=IN\n");
		(void)bq24160_otg_lock(bd, 1);
		bq24160_set_supply_sel(bd, SUPPLY_SEL_IN);
	}

	bq24160_hz_enable(bd, 0);

	bd->restricted_enable_charger = 1;

	MUTEX_UNLOCK(&bd->lock);

	bq24160_start_delayed_enable(bd, DELAYED_CHARGE_ENABLE_TIME_MS);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24160_setup_exchanged_power_supply);

static int bq24160_set_init_values(struct bq24160_data *bd)
{
	s32 data;
	s32 rc;

	dev_info(&bd->clientp->dev, "Set init values\n");

	/* Enable status interrupts */
	data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return data;

	data = SET_BIT(REG_CONTROL_EN_STAT_BIT, 1, data);
	rc = bq24160_i2c_write_byte(bd->clientp, REG_CONTROL, data);
	if (rc < 0)
		return rc;

	/* Sets any charging relates registers to 'off' */
	(void)bq24160_set_charger_voltage(0);
	(void)bq24160_set_charger_current(0);
	(void)bq24160_set_charger_termination_current(0);
	(void)bq24160_set_charger_safety_timer(0);

	(void)bq24160_set_input_voltage_dpm_usb(bd->usb_compliant_mode);

	return 0;
}

int bq24160_turn_on_charger(u8 usb_compliant)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	int rc;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	dev_info(&bd->clientp->dev, "Turning on charger. USB-%s mode\n",
		 usb_compliant ? "Host" : "Dedicated");

	bd->usb_compliant_mode = usb_compliant;

	rc = bq24160_set_init_values(bd);
	if (rc < 0)
		return rc;

	/* Need to start watchdog reset otherwise HW will reset itself.
	 * If boot has triggered charging the watchdog resetter is already
	 * started.
	 */
	if (!bd->boot_initiated_charging)
		bq24160_start_watchdog_reset(bd);
	else
		bd->boot_initiated_charging = 0;

	return rc;
}
EXPORT_SYMBOL_GPL(bq24160_turn_on_charger);

int bq24160_turn_off_charger(void)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	dev_info(&bd->clientp->dev, "Turning off charger\n");

	if (delayed_work_pending(&bd->enable_work))
		cancel_delayed_work(&bd->enable_work);

	MUTEX_LOCK(&bd->lock);

	bd->restricted_enable_charger = 0;

	/* Release OTG_LOCK to can detect USB connection after turning off.
	 * except when OTG locked from USB driver.
	 */
	if (!bd->otg_lock)
		(void)bq24160_otg_lock(bd, 0);

	bq24160_hz_enable(bd, 1);
	bq24160_set_ce(bd, 1);

	MUTEX_UNLOCK(&bd->lock);

	bq24160_stop_watchdog_reset(bd);

	/* Turning off when charging status is "Discharging" by disabling
	 * charger interval of workaround, call power_supply changed
	 * function again to update the charging status that shown to other.
	 * Because the status isn't updated in bq24160_check_status()
	 * during disabling charge interval.
	 */
	bq24160_update_power_supply(bd);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24160_turn_off_charger);

int bq24160_set_otg_lock(int lock)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (OTG_UNLOCK != lock && OTG_LOCK != lock)
		return -EINVAL;

	if (bd->otg_lock == lock) {
		dev_dbg(&bd->clientp->dev,
			"Ignore the same operation\n");
		return 0;
	}

	dev_info(&bd->clientp->dev, "OTG lock request: %d\n", lock);

	if (lock)
		bq24160_start_watchdog_reset(bd);
	else
		bq24160_stop_watchdog_reset(bd);

	bd->otg_lock = lock;

	return bq24160_otg_lock(bd, lock);
}
EXPORT_SYMBOL_GPL(bq24160_set_otg_lock);

int bq24160_set_charger_voltage(u16 mv)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	u8 voreg;
	s32 data;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (mv < MIN_CHARGE_VOLTAGE) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_voltage) {
			bd->chg_disabled_by_voltage = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24160_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	voreg = (min_t(u16, mv, MAX_CHARGE_VOLTAGE) - MIN_CHARGE_VOLTAGE) /
		CHARGE_VOLTAGE_STEP;
	dev_info(&bd->clientp->dev, "Setting charger voltage to %u mV\n",
		 MIN_CHARGE_VOLTAGE + voreg * CHARGE_VOLTAGE_STEP);
	data = bq24160_i2c_read_byte(bd->clientp, REG_BR_VOLTAGE);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_BR_VOLTAGE_MASK, data) !=
	    DATA_MASK(REG_BR_VOLTAGE_MASK, voreg)) {
		data = SET_MASK(REG_BR_VOLTAGE_MASK,
				DATA_MASK(REG_BR_VOLTAGE_MASK, voreg),
				data);
		rc = bq24160_i2c_write_byte(bd->clientp, REG_BR_VOLTAGE, data);
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_voltage) {
		bd->chg_disabled_by_voltage = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24160_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24160_set_charger_voltage);

int bq24160_set_charger_current(u16 ma)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 data;
	u8 vichrg;
	u16 ma_set;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (ma < MIN_CHARGE_CURRENT) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_current) {
			bd->chg_disabled_by_current = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24160_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	vichrg = (min_t(u16, ma, MAX_CHARGE_CURRENT) * 2 -
		LOW_CHARGE_CURRENT) / CHARGE_CURRENT_STEP;
	ma_set = (LOW_CHARGE_CURRENT + vichrg * CHARGE_CURRENT_STEP) / 2;
	if (ma < LOW_CHARGE_CURRENT ||
	    (ma <= MAX_CHARGE_CURRENT / 2 && ma == ma_set)) {
		bq24160_low_chg_enable(bd, 1);
		dev_info(&bd->clientp->dev,
			"Setting charger current to %u mA\n", ma_set);
	} else {
		vichrg = (min_t(u16, ma, MAX_CHARGE_CURRENT) -
			LOW_CHARGE_CURRENT) / CHARGE_CURRENT_STEP;
		ma_set = LOW_CHARGE_CURRENT + vichrg * CHARGE_CURRENT_STEP;
		bq24160_low_chg_enable(bd, 0);
		dev_info(&bd->clientp->dev,
			 "Setting charger current to %u mA\n", ma_set);
	}

	data = bq24160_i2c_read_byte(bd->clientp, REG_TERMINATION);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_TERMINATION_ICHRG_MASK, data) !=
	    DATA_MASK(REG_TERMINATION_ICHRG_MASK, vichrg)) {
		data = SET_MASK(REG_TERMINATION_ICHRG_MASK,
				DATA_MASK(REG_TERMINATION_ICHRG_MASK,
					  vichrg), data);
		rc = bq24160_i2c_write_byte(bd->clientp,
						REG_TERMINATION, data);
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_current) {
		bd->chg_disabled_by_current = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24160_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24160_set_charger_current);

int bq24160_set_charger_termination_current(u16 ma)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 data;
	u8 viterm;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return data;

	if (ma < MIN_CHARGE_TERM_CURRENT) {
		if (CHK_BIT(REG_CONTROL_TE_BIT, data)) {
			data = SET_BIT(REG_CONTROL_TE_BIT, 0, data);
			dev_info(&bd->clientp->dev,
				 "Disable charge current termination\n");
			rc = bq24160_i2c_write_byte(bd->clientp,
							REG_CONTROL, data);
		}
		return rc;
	}

	if (!CHK_BIT(REG_CONTROL_TE_BIT, data)) {
		data = SET_BIT(REG_CONTROL_TE_BIT, 1, data);
		dev_info(&bd->clientp->dev,
			 "Enable charge current termination\n");
		rc = bq24160_i2c_write_byte(bd->clientp, REG_CONTROL, data);
		if (rc < 0)
			return rc;
	}

	viterm = (clamp_val(ma, MIN_CHARGE_TERM_CURRENT,
			    MAX_CHARGE_TERM_CURRENT) -
		  MIN_CHARGE_TERM_CURRENT)
		/ CHARGE_TERM_CURRENT_STEP;

	data = bq24160_i2c_read_byte(bd->clientp, REG_TERMINATION);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_TERMINATION_ITERM_MASK, data) ==
	    DATA_MASK(REG_TERMINATION_ITERM_MASK, viterm))
		return 0;

	data = SET_MASK(REG_TERMINATION_ITERM_MASK,
			DATA_MASK(REG_TERMINATION_ITERM_MASK, viterm), data);
	dev_info(&bd->clientp->dev, "Charge current termination set to %u mA\n",
		 25 + viterm * 25);
	return bq24160_i2c_write_byte(bd->clientp, REG_TERMINATION, data);
}
EXPORT_SYMBOL_GPL(bq24160_set_charger_termination_current);

int bq24160_set_charger_safety_timer(u16 minutes)
{
	const char *hwtime[] = {
		"27 minutes", "6 hours", "9 hours", "disable"
	};
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	enum bq24160_tmr safety_timer;
	s32 data;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (!minutes)
		safety_timer = TMR_OFF;
	else if (minutes <= 27)
		safety_timer = TMR_27MIN;
	else if (minutes <= 360)
		safety_timer = TMR_6H;
	else
		safety_timer = TMR_9H;

	data = bq24160_i2c_read_byte(bd->clientp, REG_NTC);
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
	return bq24160_i2c_write_byte(bd->clientp, REG_NTC, data);
}
EXPORT_SYMBOL_GPL(bq24160_set_charger_safety_timer);

static int bq24160_set_input_current_limit_usb(struct bq24160_data *bd, u16 ma)
{
	const char *hwlim[] = {
		"100 mA", "150 mA", "500 mA", "800 mA",
		"900 mA", "1500 mA"
	};
	enum bq24160_iusb_lim iusb_lim;
	s32 data;
	s32 rc = 0;

	if (ma < 150)
		iusb_lim = IUSB_LIM_100MA;
	else if (ma < 500)
		iusb_lim = IUSB_LIM_150MA;
	else if (ma < 800)
		iusb_lim = IUSB_LIM_500MA;
	else if (ma < 900)
		iusb_lim = IUSB_LIM_800MA;
	else if (ma < 1500)
		iusb_lim = IUSB_LIM_900MA;
	else
		iusb_lim = IUSB_LIM_1500MA;

	dev_info(&bd->clientp->dev,
		 "Setting input charger current(USB) to %s\n",
		 hwlim[iusb_lim]);

	data = bq24160_i2c_read_byte(bd->clientp, REG_CONTROL);
	if (data < 0)
		return data;

	if (CHK_MASK(REG_CONTROL_IUSB_LIM_MASK, data) !=
	    DATA_MASK(REG_CONTROL_IUSB_LIM_MASK, iusb_lim)) {
		data = SET_MASK(REG_CONTROL_IUSB_LIM_MASK,
				DATA_MASK(REG_CONTROL_IUSB_LIM_MASK, iusb_lim),
				data);
		rc = bq24160_i2c_write_byte(bd->clientp, REG_CONTROL, data);
	}
	return rc;
}

static int bq24160_set_input_current_limit_in(struct bq24160_data *bd, u16 ma)
{
	const char *hwlim[] = {
		"1500 mA", "2500 mA"
	};
	enum bq24160_iin_lim iin_lim;
	s32 data;
	s32 rc = 0;

	if (ma < 2500)
		iin_lim = IIN_LIM_1500MA;
	else
		iin_lim = IIN_LIM_2500MA;

	dev_info(&bd->clientp->dev, "Setting input charger current(IN) to %s\n",
		 hwlim[iin_lim]);

	data = bq24160_i2c_read_byte(bd->clientp, REG_BR_VOLTAGE);
	if (data < 0)
		return data;

	if (CHK_BIT(REG_BR_VOLTAGE_IIN_LIM_BIT, data) != iin_lim) {
		data = SET_BIT(REG_BR_VOLTAGE_IIN_LIM_BIT, iin_lim, data);
		rc = bq24160_i2c_write_byte(bd->clientp, REG_BR_VOLTAGE, data);
	}
	return rc;
}

/* This function is common using variable chg_disabled_by_input_current with
 * function bq24160_set_input_current_limit_dual().
 * Because one or other will be used.
 * This function is used by the product using only USB port.
 */
int bq24160_set_input_current_limit(u16 ma)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (ma < MIN_CHARGE_CURRENT_LIMIT_USB) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_input_current) {
			bd->chg_disabled_by_input_current = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24160_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	} else {
		rc = bq24160_set_input_current_limit_usb(bd, ma);
	}

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_input_current) {
		bd->chg_disabled_by_input_current = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24160_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(bq24160_set_input_current_limit);

/* This function is common using variable chg_disabled_by_input_current with
 * function bq24160_set_input_current_limit().
 * Because one or other will be used.
 * This function is used by the product using USB and IN port.
 */
int bq24160_set_input_current_limit_dual(u16 ma_usb, u16 ma_in)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;
	s32 rc = 0;

	if (!psy)
		return -EAGAIN;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	if (ma_usb < MIN_CHARGE_CURRENT_LIMIT_USB &&
		ma_in < MIN_CHARGE_CURRENT_LIMIT_IN) {
		MUTEX_LOCK(&bd->lock);
		if (!bd->chg_disabled_by_input_current) {
			bd->chg_disabled_by_input_current = 1;
			MUTEX_UNLOCK(&bd->lock);
			bq24160_disable_charger(bd);
		} else {
			MUTEX_UNLOCK(&bd->lock);
		}
		return 0;
	}

	if (ma_usb >= MIN_CHARGE_CURRENT_LIMIT_USB)
		rc = bq24160_set_input_current_limit_usb(bd, ma_usb);

	if (ma_in >= MIN_CHARGE_CURRENT_LIMIT_IN)
		rc = bq24160_set_input_current_limit_in(bd, ma_in);

	MUTEX_LOCK(&bd->lock);
	if (bd->chg_disabled_by_input_current) {
		bd->chg_disabled_by_input_current = 0;
		MUTEX_UNLOCK(&bd->lock);
		bq24160_enable_charger(bd);
	} else {
		MUTEX_UNLOCK(&bd->lock);
	}
	return rc;
}
EXPORT_SYMBOL_GPL(bq24160_set_input_current_limit_dual);

int bq24160_set_ext_charging_status(int status)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return -EINVAL;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	MUTEX_LOCK(&bd->lock);
	bd->ext_status = status;
	MUTEX_UNLOCK(&bd->lock);

	bq24160_update_power_supply(bd);

	return 0;
}
EXPORT_SYMBOL_GPL(bq24160_set_ext_charging_status);

bool bq24160_is_restricted_by_charger_revision(int batt_voltage,
				u16 chg_voltage_now, u16 chg_current_now)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return true;
	bd = container_of(psy, struct bq24160_data, bat_ps);

	dev_dbg(&bd->clientp->dev,
		"Get vendor_revision 0x%.2x\n", bd->vendor_rev);

	if (bd->vendor_rev == VENDOR_REV_23) {
		if (batt_voltage >= STOP_CHARGE_VOLTAGE_REV_23 ||
			(batt_voltage >= START_CHARGE_VOLTAGE_REV_23 &&
			!chg_voltage_now && !chg_current_now)) {
			return true;
		}
	}
	return false;
}
EXPORT_SYMBOL_GPL(bq24160_is_restricted_by_charger_revision);

void bq24160_get_restricted_setting(u16 *chg_voltage, u16 *chg_current)
{
	struct power_supply *psy = power_supply_get_by_name(BQ24160_NAME);
	struct bq24160_data *bd;

	if (!psy)
		return;

	bd = container_of(psy, struct bq24160_data, bat_ps);

	dev_info(&bd->clientp->dev,
		"Get restricted charge voltage and current "
		"depending on vendor_revision\n");

	if (bd->vendor_rev == VENDOR_REV_23) {
		*chg_voltage = 0;
		*chg_current = 0;
	}
}
EXPORT_SYMBOL_GPL(bq24160_get_restricted_setting);

int bq24160_charger_initialized(void)
{
	return (int)atomic_read(&bq24160_init_ok);
}
EXPORT_SYMBOL_GPL(bq24160_charger_initialized);

static int __exit bq24160_remove(struct i2c_client *client)
{
	struct bq24160_data *bd = i2c_get_clientdata(client);

	if (bd->irq_wake_enabled)
		(void)disable_irq_wake(client->irq);

	free_irq(client->irq, bd);

	if (delayed_work_pending(&bd->work))
		cancel_delayed_work_sync(&bd->work);

	if (delayed_work_pending(&bd->enable_work))
		cancel_delayed_work_sync(&bd->enable_work);

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

static const struct i2c_device_id bq24160_id[] = {
	{BQ24160_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, bq24160_id);

static enum power_supply_property bq24160_bat_main_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
};

static int bq24160_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct bq24160_platform_data *pdata = client->dev.platform_data;
	struct bq24160_data *bd;
	s32 buf;
	int rc = 0;

	if (pdata && pdata->gpio_configure) {
		rc = pdata->gpio_configure(1);
		if (rc) {
			pr_err("%s: failed to gpio_configure\n", BQ24160_NAME);
			goto probe_exit;
		}
	}

	/* Make sure we have at least i2c functionality on the bus */
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "No i2c functionality available\n");
		rc = -EIO;
		goto probe_exit_hw_deinit;
	}

	buf = bq24160_i2c_read_byte(client, REG_VENDOR);
	if (buf <= 0) {
		dev_err(&client->dev, "Failed read vendor info\n");
		rc = -EIO;
		goto probe_exit_hw_deinit;
	}

	if (((buf & REG_VENDOR_CODE_MASK) >> 5) == REG_VENDOR_CODE) {
		dev_info(&client->dev, "Found bq24160, rev 0x%.2x\n",
			 buf & REG_VENDOR_REV_MASK);
	} else {
		dev_err(&client->dev, "Invalid vendor code\n");
		rc = -ENODEV;
		goto probe_exit_hw_deinit;
	}

	bd = kzalloc(sizeof(struct bq24160_data), GFP_KERNEL);
	if (!bd) {
		rc = -ENOMEM;
		goto probe_exit_hw_deinit;
	}

	bd->bat_ps.name = BQ24160_NAME;
	bd->bat_ps.type = POWER_SUPPLY_TYPE_BATTERY;
	bd->bat_ps.properties = bq24160_bat_main_props;
	bd->bat_ps.num_properties = ARRAY_SIZE(bq24160_bat_main_props);
	bd->bat_ps.get_property = bq24160_bat_get_property;
	bd->bat_ps.use_for_apm = 1;
	bd->clientp = client;
	bd->ext_status = -1;
	bd->vendor_rev = buf & REG_VENDOR_REV_MASK;

	if (pdata) {
		bd->bat_ps.name = pdata->name;
		bd->control = pdata;

		if (pdata->supplied_to) {
			bd->bat_ps.supplied_to = pdata->supplied_to;
			bd->bat_ps.num_supplicants = pdata->num_supplicants;
		}
	}

	mutex_init(&bd->lock);
	wake_lock_init(&bd->wake_lock, WAKE_LOCK_SUSPEND,
		       "bq24160_watchdog_lock");

	bd->wq = create_singlethread_workqueue("bq24160worker");
	if (!bd->wq) {
		dev_err(&client->dev, "Failed creating workqueue\n");
		rc = -ENOMEM;
		goto probe_exit_free;
	}

	INIT_DELAYED_WORK(&bd->work, bq24160_reset_watchdog_worker);
	INIT_DELAYED_WORK(&bd->enable_work,
				bq24160_delayed_enable_worker);

	rc = power_supply_register(&client->dev, &bd->bat_ps);
	if (rc) {
		dev_err(&client->dev,
			"Failed registering to power_supply class\n");
		goto probe_exit_work_queue;
	}

	i2c_set_clientdata(client, bd);

	bq24160_check_status(bd);
	bq24160_update_power_supply(bd);

	if (pdata && pdata->support_boot_charging &&
	    (STAT_CHARGING_FROM_USB == bd->cached_status.stat ||
	     STAT_CHARGING_FROM_IN == bd->cached_status.stat ||
	     STAT_USB_READY == bd->cached_status.stat ||
	     STAT_IN_READY == bd->cached_status.stat)) {
		dev_info(&client->dev, "Charging started by boot\n");
		bd->boot_initiated_charging = 1;
		/* Kick the watchdog to not lose the boot setting */
		bq24160_start_watchdog_reset(bd);
	} else {
		dev_info(&client->dev, "Not initialized by boot\n");
		rc = bq24160_reset_charger(bd);
		if (rc)
			goto probe_exit_unregister;

	}

	rc = request_threaded_irq(client->irq, NULL, bq24160_thread_irq,
				  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				  IRQF_DISABLED | IRQF_ONESHOT,
				  "bq24160interrupt", bd);
	if (rc) {
		dev_err(&client->dev, "Failed requesting IRQ\n");
		goto probe_exit_unregister;
	}

	device_init_wakeup(&client->dev, 1);

	rc = enable_irq_wake(client->irq);
	if (rc) {
		dev_err(&client->dev,
			"Failed to enable wakeup on IRQ request\n");
		goto probe_exit_unregister;
	} else {
		bd->irq_wake_enabled = 1;
	}

#ifdef DEBUG_FS
	if (debug_create_attrs(bd->bat_ps.dev))
		dev_info(&client->dev, "Debug support failed\n");
#endif

	atomic_set(&bq24160_init_ok, 1);
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

static struct i2c_driver bq24160_driver = {
	.driver = {
		   .name = BQ24160_NAME,
		   .owner = THIS_MODULE,
	},
	.probe = bq24160_probe,
	.remove = __exit_p(bq24160_remove),
	.id_table = bq24160_id,
};

static int __init bq24160_init(void)
{
	int rc;

	rc = i2c_add_driver(&bq24160_driver);
	if (rc) {
		pr_err("%s FAILED: i2c_add_driver rc=%d\n", __func__, rc);
		goto init_exit;
	}
	return 0;

init_exit:
	return rc;
}

static void __exit bq24160_exit(void)
{
	i2c_del_driver(&bq24160_driver);
}

module_init(bq24160_init);
module_exit(bq24160_exit);

MODULE_AUTHOR("Hiroyuki Namba");
MODULE_LICENSE("GPL v2");
