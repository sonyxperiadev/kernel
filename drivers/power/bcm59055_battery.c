/*
 *  linux/drivers/power/bcm59055_batter.c - Broadcom BCM59055 Battery driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/slab.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/broadcom/bcm59055-power.h>
#include <linux/broadcom/bcm59055-fuelgauge.h>
#include <linux/broadcom/bcm59055-adc.h>
#include <linux/workqueue.h>
#include <linux/mfd/bcm590xx/bcm590xx-usb.h>

#define USB_OFFLINE	4

const char *usb_model_name[]  = {
	[USB_CHARGER_UNKNOWN]	= "usb-unknown",
	[USB_CHARGER_SDP]	= "usb-sdp",
	[USB_CHARGER_CDP]	= "usb-cdp",
	[USB_CHARGER_DCP]	= "usb-dcp",
	[USB_OFFLINE]		= "usb-offline"
};

#define BYTE_COMBINE(msb,lsb)  (msb<<8 | lsb)
#define MV_FROM_REG(regval)  DIV_ROUND_CLOSEST(((regval) * 4800 ) , 1024)
#define PRE_ENUM_CURRENT	CURRENT_100_MA

struct bcm59055_power {
	struct bcm590xx *bcm590xx;
	struct power_supply battery;
	struct power_supply wall;
	struct power_supply usb;

	int usb_cc;
	int wac_cc;
	enum power_supply_type power_src;
	int usb_type;
	struct delayed_work charger_insert_wq;
	struct delayed_work batt_lvl_wq;

	unsigned short batt_min_volt;
	unsigned short batt_max_volt;
	unsigned short batt_technology;
	u8 *batt_vol;
	u8 *batt_adc;

	unsigned int batt_max_cap;
	// Display properties.
	unsigned int batt_percentage;
	unsigned int batt_prev_cap;
	unsigned int batt_status;
	unsigned int batt_voltage_now;
	struct notifier_block nb;
};

static struct bcm59055_power *pvt_data;
static enum power_supply_property bcm59055_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
};

static enum power_supply_property bcm59055_wall_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static enum power_supply_property bcm59055_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_MODEL_NAME
};


static enum power_supply_type bcm59055_get_power_supply_type(struct
							 bcm59055_power
							 *bcm59055_power)
{
	int regVal;
	struct bcm590xx *bcm59055 = bcm59055_power->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	regVal = bcm590xx_reg_read(bcm59055, BCM59055_REG_ENV2);
	if (P_CGPD_ENV & regVal)
		return POWER_SUPPLY_TYPE_MAINS;
	else if (P_UBPD_ENV & regVal)
		return POWER_SUPPLY_TYPE_USB;

	return POWER_SUPPLY_TYPE_BATTERY;
}

static int bcm59055_usb_get_property(struct power_supply *psy,
				 enum power_supply_property psp,
				 union power_supply_propval *val)
{
	int ret = 0;
	struct bcm59055_power *bcm59055_power =
		dev_get_drvdata(psy->dev->parent);
	if (unlikely(!bcm59055_power)) {
		pr_info("%s: invalid driver data !!!\n",
			__func__);
		return -EINVAL;
	}
	pr_debug("Inside %s\n", __func__);
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
		 (bcm59055_power->power_src ==
		 POWER_SUPPLY_TYPE_USB) ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = usb_model_name[bcm59055_power->usb_type];
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = -1;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcm59055_wall_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret = 0;
	struct bcm59055_power *bcm59055_power =
		dev_get_drvdata(psy->dev->parent);

	pr_debug("Inside %s\n", __func__);
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
			(bcm59055_power->power_src ==
			 POWER_SUPPLY_TYPE_MAINS) ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = -1;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bcm59055_battery_get_property(struct power_supply *psy,
				 enum power_supply_property psp,
				 union power_supply_propval *val)
{
	int ret = 0;
	struct bcm59055_power *battery_data = dev_get_drvdata(psy->dev->parent);
	pr_debug("Inside %s\n", __func__);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval =
			(battery_data->power_src ==
			 POWER_SUPPLY_TYPE_BATTERY) ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = battery_data->batt_status;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = battery_data->batt_technology;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = battery_data->batt_percentage;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		val->intval = battery_data->batt_min_volt;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = battery_data->batt_max_volt;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = battery_data->batt_voltage_now * 1000;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int get_batt_percentage(struct bcm59055_power *battery_data)
{
	int bat_capacity = 0;
	u16 count, slp_count;
	u32 accm;
	int ret;
	pr_debug("Inside %s\n", __func__);
	if (bcm59055_fg_offset_cal(FAST_CALIBRATION))
		pr_info("%s: FAST Calibration for Fuel Gauge failed\n", __func__);
	if (bcm59055_fg_init_read())
		pr_info("%s: Fuel Gauge Read init failed\n", __func__);
	ret = bcm59055_fg_read_soc(&accm, &count, &slp_count);
	/* TODO: Need to calculate Batt Capacity */
	pr_debug("%s: FG capacity %d\n", __func__, bat_capacity);
	if (bat_capacity < 0) {
		pr_debug("%s: Fuel Gauge Accumulator read failed\n", __func__);
		return bat_capacity;
	}
	if (battery_data->batt_prev_cap != bat_capacity) {
		battery_data->batt_percentage =
			(100 * (bat_capacity / battery_data->batt_max_cap));
		battery_data->batt_prev_cap = bat_capacity;
		return battery_data->batt_percentage;
	}

	return 0;
}

static int get_batt_voltage(struct bcm59055_power *battery_data)
{
	int adc_val;
	int voltage;
	int i;
	pr_debug("Inside %s\n", __func__);
	/* TODO: Need to build the ADC-Voltage table for current voltage reading */
	adc_val = bcm59055_saradc_read_data(ADC_VMBAT_CHANNEL);
	for (i = 0; i < VOLTAGE_ADC_MAX_SAMPLE; i++)
		if ((adc_val >= battery_data->batt_adc[i]) &&
				adc_val < battery_data->batt_adc[i+1])
			break;
	if (adc_val == battery_data->batt_adc[i])
		voltage = battery_data->batt_vol[i];
	else
		voltage = battery_data->batt_vol[i] +
			((battery_data->batt_vol[i+1] - battery_data->batt_vol[i]) *
			 ((adc_val - battery_data->batt_adc[i]) /
			  (battery_data->batt_adc[i+1] - battery_data->batt_adc[i])));
	battery_data->batt_voltage_now = voltage;
	pr_debug("%s: Adc Val %d, Battery voltage %d\n", __func__, adc_val,
			voltage);
	return voltage;
}

static void bcm59055_batt_lvl_wq(struct work_struct *work)
{
	struct bcm59055_power *battery_data = container_of(work,
			struct bcm59055_power, batt_lvl_wq.work);
/* For time being this has kept commented as battery management can be moved to
 * separate driver
 */
#if 0
	if (get_batt_percentage(battery_data))
		power_supply_changed(&battery_data->battery);
	if (battery_data->power_src != POWER_SUPPLY_TYPE_BATTERY)
		get_batt_voltage(battery_data);
	pr_debug("%s: Battery percentage %d\n", __func__,
			battery_data->batt_percentage);
#endif
	/* If charging is not happening bettery level can be measured
	 * with a period of 1 min else with 10 sec period
	*/
	if (battery_data->batt_status == POWER_SUPPLY_STATUS_DISCHARGING)
		schedule_delayed_work(&battery_data->batt_lvl_wq,
				msecs_to_jiffies(60000));
	else
		schedule_delayed_work(&battery_data->batt_lvl_wq,
				msecs_to_jiffies(10000));
}

static void bcm59055_start_charging(struct bcm590xx *bcm59055, int charger_type)
{
	int val = 0;
	pr_debug("Inside %s\n", __func__);

	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL3);

	if (charger_type == POWER_SUPPLY_TYPE_USB)
		val |= USB_HOSTEN;
	if (charger_type == POWER_SUPPLY_TYPE_MAINS)
		val |= WAC_HOSTEN;
	if (bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL3, val)) {
		pr_info("%s: Failed to start charging\n", __func__);
		return;
	}
	/* Update battery status */
	pvt_data->batt_status = POWER_SUPPLY_STATUS_CHARGING;
	power_supply_changed(&pvt_data->battery);
#if 0
	val = (BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256 <<
			BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256_SHIFT) |
		(BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT <<
		 	BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT_SHIFT);
	if (bcm590xx_reg_write(bcm59055, BCM59055_REG_PWMLEDCTRL1, val)) {
		pr_info("%s Configuring PMU LED failed \n", __func__ );
	}
#endif
}

static void bcm59055_stop_charging(struct bcm590xx *bcm59055, int charger_type)
{
	int val = 0;
	pr_debug("Inside %s\n", __func__);

	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL3);

	if (charger_type == POWER_SUPPLY_TYPE_USB)
		val &= ~USB_HOSTEN;
	if (charger_type == POWER_SUPPLY_TYPE_MAINS)
		val &= ~WAC_HOSTEN;
	if (bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL3, val)) {
		pr_info("%s: Failed to stop charging\n", __func__);
		return;
	}
	/* Update battery status */
	pvt_data->batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
	power_supply_changed(&pvt_data->battery);
#if 0
	val = (BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256 <<
			BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256_SHIFT) |
		(BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE <<
		 	BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE_SHIFT) |
		(BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT <<
		 	BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT_SHIFT);
	if (bcm590xx_reg_write(bcm59055, BCM59055_REG_PWMLEDCTRL1, val)) {
		pr_info("%s Configuring PMU LED failed \n", __func__ );
	}
#endif
}

/* USB driver call back once USB enumaration done */
void pmu_set_usb_enum_current(int curr)
{
	struct bcm590xx *bcm59055 = pvt_data->bcm590xx;
	struct bcm590xx_battery_pdata *pdata = bcm59055->pdata->battery_pdata;
	int val;
	pr_debug("Inside %s\n", __func__);
	/* set the current */
	if (curr)
		pvt_data->usb_cc = curr;
	else
		pvt_data->usb_cc = pdata->usb_cc;
	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL10);
	val &= ~USB_FC_CC_MASK;
	val |= (pvt_data->usb_cc & USB_FC_CC_MASK);
	if (bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL10, val))
		pr_info("%s: Failed to set USB FC Current\n", __func__);
}
EXPORT_SYMBOL(pmu_set_usb_enum_current);

static void bcm59055_power_isr(int intr, void *data)
{
	int val = 0;
	struct bcm59055_power *battery_data = data;
	struct bcm590xx *bcm59055 = battery_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);

	switch (intr) {
	case BCM59055_IRQID_INT2_CHGINS:
		pr_info("%s: WAC insert interrupt\n", __func__);
		battery_data->power_src = POWER_SUPPLY_TYPE_MAINS;
		val = bcm590xx_reg_read(bcm59055, BCM59055_REG_ENV3);
		if (!(val & P_CHGOV)) {
			val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL11);
			val &= ~WAC_FC_CC_MASK;
			val |= (pvt_data->wac_cc & WAC_FC_CC_MASK);
			if (!bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL11, val))
				bcm59055_start_charging(bcm59055, battery_data->power_src);
			else {
				pr_info("%s: Failed to set WAC FC Current\n", __func__);
			}
		} else
			pr_info("%s: WAC Over Voltage\n", __func__);
		/* Need to start NTC block on after detecting the charger */
		bcm59055_saradc_enable_ntc_block();
		break;
	case BCM59055_IRQID_INT2_CHGRM:
		pr_info("%s: WAC remove interrupt\n", __func__);
		battery_data->power_src = bcm59055_get_power_supply_type(battery_data);
		bcm59055_stop_charging(bcm59055, POWER_SUPPLY_TYPE_MAINS);
		/* Need to stop NTC block on after detecting the charger */
		bcm59055_saradc_disable_ntc_block();
		break;
	}
}

static int bcm59055_battery_cb(struct notifier_block *nb, unsigned long event,
		void *data)
{
	struct bcm590xx *bcm59055 = pvt_data->bcm590xx;
	u8 val;
	pr_info("Inside %s\n", __func__);
	printk("%s: Event %ld, Val = %d\n", __func__, event, *(int *)data);
	switch(event) {
	case BCMPMU_USB_EVENT_IN_RM:
		if ((*(int *)data) == 1) {
			/* USBINS notification: Need not to do anything. Wait for charger
			 * type notification.
			 */
			pvt_data->power_src = POWER_SUPPLY_TYPE_USB;
		} else {
			pr_info("%s: USB remove notification\n", __func__);
			pvt_data->power_src = bcm59055_get_power_supply_type(pvt_data);
			bcm59055_stop_charging(bcm59055, POWER_SUPPLY_TYPE_USB);
			pvt_data->usb_cc = PRE_ENUM_CURRENT;
			pvt_data->usb_type = USB_OFFLINE;
			/* Need to stop NTC block on after detecting the charger */
			bcm59055_saradc_disable_ntc_block();
		}
		break;
	case BCMPMU_USB_EVENT_CHGR_DETECTION:
		pvt_data->usb_type = *(int *)data;
		val = bcm590xx_reg_read(bcm59055, BCM59055_REG_ENV3);
		if (!(val & P_USBOV)) {		/* NO USB OV */
			val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL10);
			val &= ~USB_FC_CC_MASK;
			val |= (pvt_data->usb_cc & USB_FC_CC_MASK);
			if (!bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL10, val))
				bcm59055_start_charging(bcm59055, pvt_data->power_src);
			else {
				pr_info("%s: Failed to set USB FC Current\n", __func__);
			}
		} else
			pr_info("%s: USB Over Voltage\n", __func__);

		/* Need to start NTC block on after detecting the charger */
		bcm59055_saradc_enable_ntc_block();
		break;
	}
	return 0;
}

static int bcm59055_init_charger(struct bcm59055_power *battery_data)
{
	int val = 0, ret = 0 ;
	struct bcm590xx *bcm59055 = battery_data->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	/* Keep NTC block off at start */
	bcm59055_saradc_disable_ntc_block();

	/* Enable The Fuel Gauge and do a long calibration */
	ret = bcm59055_fg_enable();
	if (ret) {
		pr_info("%s: Failed to enable Fuel Gauge\n", __func__);
		return ret;
	}
	ret = bcm59055_fg_offset_cal(LONG_CALIBRATION);
	if (ret) {
		pr_info("%s: Failed to long calibrate Fuel Gauge\n", __func__);
		return ret;
	}

	/* Set the VFLOAT MAX..the max voltage for MBC */
	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL6);
	val &= ~VFLOAT_MASK;
	val |= (VFLOAT_4_2_V | VFLOATMAX_LOCK);
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL6, val);
	if (ret) {
		pr_info("%s Configuring MBCCTRL6 failed \n", __func__ );
		return ret;
	}
	/* Set VFLOAT..charging voltage */
	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL7);
	val &= ~VFLOAT_MASK;
	val |= VFLOAT_4_2_V;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL7, val);
	if (ret) {
		pr_info("%s Configuring MBCCTRL7 failed \n", __func__ );
		return ret;
	}
	/* set ICC Max the max current for MBC */
	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL8);
	val &= ~ICCMAX_MASK;
	val |= (CURRENT_900_MA | ICCMAX_LOCK);
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL8, val);
	if (ret) {
		pr_info("%s Configuring MBCCTRL8 failed \n", __func__ );
		return ret;
	}
	val = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL9);
	val |= SWUP;
	ret = bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL9, val);
	if (ret) {
		pr_info("%s Configuring MBCCTRL8 failed \n", __func__ );
		return ret;
	}
	battery_data->nb.notifier_call = bcm59055_battery_cb;
	ret = bcmpmu_usb_add_notifier(BCMPMU_USB_EVENT_IN_RM, &battery_data->nb);
	ret |= bcmpmu_usb_add_notifier(BCMPMU_USB_EVENT_CHGR_DETECTION,
			&battery_data->nb);
	ret = bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGINS, true,
			bcm59055_power_isr, battery_data);
	ret |= bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGRM, true,
			bcm59055_power_isr, battery_data);
	if (ret) {
		pr_info("%s request_irq for charger inserted condition failed \n",
				__func__ );
		return ret;
	}

	return 0;
}

static int bcm59055_battery_probe(struct platform_device *pdev)
{
	int ret, usb_typ;
	struct bcm59055_power *battery_data;
	struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);
	struct bcm590xx_battery_pdata *battery_pdata;

	pr_info("BCM59055 Battery Driver Probe\n");
	battery_pdata = bcm59055->pdata->battery_pdata;

	battery_data = kzalloc(sizeof(struct bcm59055_power), GFP_KERNEL);
	if (battery_data == NULL) {
		pr_info("%s : Failed to allocate memory for bcm59055_power\n", __func__);
		ret = -ENOMEM;
		goto err_data_alloc;
	}

	battery_data->bcm590xx = bcm59055;
	battery_data->batt_max_cap = battery_pdata->batt_max_capacity;
	battery_data->batt_min_volt = battery_pdata->batt_min_volt;
	battery_data->batt_max_volt = battery_pdata->batt_max_volt;
	battery_data->batt_technology = battery_pdata->batt_technology;
	battery_data->usb_cc = PRE_ENUM_CURRENT;
	battery_data->batt_vol = battery_pdata->batt_vol;
	battery_data->batt_adc = battery_pdata->batt_adc;
	battery_data->power_src = POWER_SUPPLY_TYPE_BATTERY;
	INIT_DELAYED_WORK(&battery_data->batt_lvl_wq, bcm59055_batt_lvl_wq);

	platform_set_drvdata(pdev, battery_data);
	pvt_data = battery_data;
	ret = bcm59055_init_charger(battery_data);
	if (ret < 0) {
		pr_info("%s Charger initialization failed. \n", __func__);
		goto err_ac_register;
	}

	if (get_batt_percentage(battery_data) < 0)
		battery_data->batt_percentage = 50;
	/* For now no FG and ADC is connected in rayboard so hardcoding the capacity */
	battery_data->batt_percentage = 50;
	battery_data->battery.name = "bcm59055-battery";
	battery_data->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	battery_data->battery.properties = bcm59055_battery_props;
	battery_data->battery.num_properties = ARRAY_SIZE(bcm59055_battery_props);
	battery_data->battery.get_property = bcm59055_battery_get_property;
	battery_data->batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;

	battery_data->wall.name = "bcm59055-usb";
	battery_data->wall.type = POWER_SUPPLY_TYPE_USB;
	battery_data->wall.properties = bcm59055_usb_props;
	battery_data->wall.num_properties = ARRAY_SIZE(bcm59055_usb_props);
	battery_data->wall.get_property = bcm59055_usb_get_property;

	battery_data->usb.name = "bcm59055-wall";
	battery_data->usb.type = POWER_SUPPLY_TYPE_MAINS;
	battery_data->usb.properties = bcm59055_wall_props;
	battery_data->usb.num_properties = ARRAY_SIZE(bcm59055_wall_props);
	battery_data->usb.get_property = bcm59055_wall_get_property;

	/* Register AC power supply */
	ret = power_supply_register(&pdev->dev, &battery_data->wall);
	if (ret) {
		pr_info("%s : Failed to register WALL power supply\n", __func__);
		goto err_ac_register;
	}

	/* Register battery power supply */
	ret = power_supply_register(&pdev->dev, &battery_data->usb);
	if (ret) {
		pr_info("%s : Failed to register battery power supply\n", __func__);
		goto err_battery_register;
	}

	/* Register USB power supply */
	ret = power_supply_register(&pdev->dev, &battery_data->battery);
	if (ret) {
		pr_info("%s : Failed to register battery power supply\n", __func__);
		goto err_usb_register;
	}
	battery_data->usb_type = USB_OFFLINE;
	// Check if charger is already connected
	ret = bcm59055_get_power_supply_type(battery_data);
	if (ret != POWER_SUPPLY_TYPE_BATTERY /*&& usb_driver_init*/) {
		pr_info("%s: Charger connected %s\n", __func__,
				(ret == POWER_SUPPLY_TYPE_MAINS ? "Wall" : "USB"));
		if (ret == POWER_SUPPLY_TYPE_MAINS)
			bcm59055_power_isr(BCM59055_IRQID_INT2_CHGINS, battery_data);
		else {
			usb_typ = bcmpmu_usb_get(BCMPMU_CTRL_GET_CHARGER_TYPE, bcm59055);
			battery_data->power_src = POWER_SUPPLY_TYPE_USB;
			printk("%s: USB Type %d\n", __func__, usb_typ);
			bcm59055_battery_cb(&battery_data->nb,
					BCMPMU_USB_EVENT_CHGR_DETECTION, &usb_typ);
		}
	}
	printk("%s: pwr_src %d, usb_typ %d\n", __func__, battery_data->power_src,
			battery_data->usb_type);
	schedule_delayed_work(&battery_data->batt_lvl_wq, 0 );
	printk("%s: Probe Sucess\n", __func__);
	return 0;

err_usb_register:
	power_supply_unregister(&battery_data->usb);
err_battery_register:
	power_supply_unregister(&battery_data->wall);
err_ac_register:
	kfree(battery_data);
err_data_alloc:
	return ret;
}

static int bcm59055_battery_remove(struct platform_device *pdev)
{
	struct bcm59055_power *data = platform_get_drvdata(pdev);
	struct bcm590xx *bcm59055 = data->bcm590xx;
	pr_debug("Inside %s\n", __func__);
	bcm59055_fg_disable();
	power_supply_unregister(&data->battery);
	power_supply_unregister(&data->wall);
	power_supply_unregister(&data->usb);

	cancel_delayed_work(&data->batt_lvl_wq);

	bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT2_CHGINS);
	bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT2_CHGRM);
	kfree(data);

	return 0;
}

static struct platform_driver bcm59055_battery_driver = {
	.probe		= bcm59055_battery_probe,
	.remove		= bcm59055_battery_remove,
	.driver		= {
		.name = "bcm590xx-power"
	}
};

static int __init bcm59055_battery_init(void)
{
	return platform_driver_register(&bcm59055_battery_driver);
}

static void __exit bcm59055_battery_exit(void)
{
	platform_driver_unregister(&bcm59055_battery_driver);
}

module_init(bcm59055_battery_init);
module_exit(bcm59055_battery_exit);
