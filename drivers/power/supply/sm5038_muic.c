/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2021 Sony Corporation,
 * and licensed under the license of the file.
 */
// SPDX-License-Identifier: GPL-2.0-only
/*
 * sm5038-muic.c
 *
 * Copyright (C) 2021 SiliconMitus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *
 */
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#include <linux/debugfs.h>
#endif

#include <linux/power/sm5038.h>
#include <linux/power/sm5038_muic.h>

static struct sm5038_muic_data *static_data;

static int com_to_open(struct sm5038_muic_data *muic_data);
static void sm5038_muic_handle_attach(struct sm5038_muic_data *muic_data,
			int new_dev, unsigned char vbvolt, int irq);
static void sm5038_muic_handle_detach(struct sm5038_muic_data *muic_data,
			int irq);
static void sm5038_muic_detect_dev(struct sm5038_muic_data *muic_data, int irq);

extern int sm5038_charger_oper_get_input_current_ua(void);
extern int sm5038_usbpd_get_otg_status(void);
extern int sm5038_charger_get_chg_mode_type(void);
extern int sm5038_charger_get_chg_on_status(void);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
extern int sm5038_cc_control_command(int enable);
extern int somc_sm5038_present_is_pd_apdapter(void);
#endif

char *SM5038_MUIC_INT_NAME[5] = {
	"DPDM_OVP",		/* 0 */
	"VBUS_DETACH",	/* 1 */
	"CHGTYPE",		/* 2 */
	"DCDTIMEOUT",	/* 3 */
	"HVDCP"			/* 4 */
};

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
enum print_reason {
	PR_INTERRUPT	= BIT(0),
	PR_CORE		= BIT(1),
	PR_MISC		= BIT(2),
	PR_REGISTER	= BIT(3),
};
#define somc_sm5038_muic_dbg(muic_data, reason, fmt, ...)			\
	do {								\
		if (muic_data->debug_mask & (reason))			\
			pr_info("[SOMC MUIC] %s: "fmt, __func__, ##__VA_ARGS__);	\
		else							\
			pr_debug("[SOMC MUIC] %s: "fmt, __func__, ##__VA_ARGS__);	\
	} while (0)
#endif

int sm5038_muic_i2c_read_byte(struct i2c_client *client, unsigned char command)
{
	unsigned char ret = 0;

	sm5038_read_reg(client, command, &ret);
	return ret;
}

int sm5038_muic_i2c_write_byte(struct i2c_client *client,
			unsigned char command, unsigned char value)
{
	int ret = 0;
	int retry = 0;
	unsigned char written = 0;

	ret = sm5038_write_reg(client, command, value);

	while (ret < 0) {
		pr_info("[%s:%s] reg(0x%x), retrying(%d)...\n",
			MUIC_DEV_NAME, __func__, command, retry);
		sm5038_read_reg(client, command, &written);
		pr_info("[%s:%s] reg(0x%x): written(0x%x), value(0x%x)\n",
			MUIC_DEV_NAME, __func__, command, written, value);
		if (written == value) {
			break;
		}
		if (retry > 5) {
			pr_err("[%s:%s] retry failed!!\n", MUIC_DEV_NAME,
					__func__);
			break;
		}
		msleep(100);
		retry++;
		ret = sm5038_write_reg(client, command, value);
	}

	return ret;
}

int sm5038_set_ctrl2_reg(struct sm5038_muic_data *muic_data, int shift, bool on)
{
	struct i2c_client *i2c = muic_data->i2c;
	unsigned char reg_val = 0;
	int ret = 0;

	//pr_info("[%s:%s] Register[%d], set [%d]\n", MUIC_DEV_NAME, __func__,
	//		shift, on);
	ret = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL2);
	if (ret < 0)
		pr_err("[%s:%s](%d)\n", MUIC_DEV_NAME, __func__, ret);
	if (on)
		reg_val = ret | (0x1 << shift);
	else
		reg_val = ret & ~(0x1 << shift);

	if (reg_val ^ ret) {
		//pr_info("[%s:%s] CTRL2 reg(0x%x) -> reg_val(0x%x) : update reg\n",
		//	MUIC_DEV_NAME, __func__, ret, reg_val);

		ret = sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_CNTL2, reg_val);
		if (ret < 0)
			pr_err("[%s:%s] err write CTRL2(%d)\n",
				MUIC_DEV_NAME, __func__, ret);
	} else {
		//pr_info("[%s:%s] (0x%x), just return\n",
		//	MUIC_DEV_NAME, __func__, ret);
		return 0;
	}

	ret = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL2);
	//if (ret < 0)
	//	pr_err("[%s:%s] err read CTRL2(0x%x)\n",
	//		MUIC_DEV_NAME, __func__, ret);
	//else
	//	pr_info("[%s:%s] CTRL2 reg after change(0x%x)\n",
	//		MUIC_DEV_NAME, __func__, ret);

	return ret;
}

static int sm5038_muic_get_vbus(void)
{
	struct sm5038_muic_data *muic_data = static_data;
	struct i2c_client *i2c = muic_data->i2c;
	int vbus_voltage = 0, voltage = 0;
	int irqvbus = 0, intmask2 = 0;
	int retry = 0;
	int vbus_valid = 0, reg_vbus = 0;

	if (static_data == NULL)
		return 0;

	reg_vbus = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS);
	vbus_valid = (reg_vbus&0x04)>>2;
	//pr_info("[%s:%s] REG_VBUS:0x%x, vbus_valid(%d)", MUIC_DEV_NAME,
	//	__func__, reg_vbus, vbus_valid);
	if (!vbus_valid) {
		pr_info("[%s:%s] skip : NO VBUS\n", MUIC_DEV_NAME, __func__);
		return 0;
	}

	intmask2 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INTMASK2);
	//pr_info("[%s:%s] REG_INTMASK2:0x%x\n", MUIC_DEV_NAME,
	//	__func__, intmask2);
	if (!(intmask2&INT2_VBUS_UPDATE_MASK)) {
		intmask2 = intmask2 | INT2_VBUS_UPDATE_MASK;
		sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_INTMASK2, intmask2);
	}

	sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_VBUS_READ, 1);

	for (retry = 0; retry < 5 ; retry++) {
		usleep_range(5000, 5100);
		irqvbus = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INT2);
		if (irqvbus & INT2_VBUS_UPDATE_MASK) {
			//pr_info("[%s:%s] VBUS update Success : irqvbus(0x%x), retry(%d)\n",
			//	MUIC_DEV_NAME, __func__, irqvbus, retry);
			break;
		}
		//pr_info("[%s:%s] VBUS update Fail : irqvbus(0x%x), retry(%d)\n",
		//		MUIC_DEV_NAME, __func__, irqvbus, retry);
	}

	sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_VBUS_READ, 0);

	if (retry >= 5) {
		pr_info("[%s:%s] VBUS update Failed(%d)\n", MUIC_DEV_NAME,
				__func__, retry);
		return 0;
	}

	voltage = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS_VOLTAGE);
	if (voltage < 0)
		pr_err("[%s:%s] err read VBUS VOLTAGE(0x%2x)\n", MUIC_DEV_NAME,
				__func__, voltage);

	vbus_voltage = voltage*100;

	//pr_info("[%s:%s] voltage=[0x%02x] vbus=%d mV, attached_dev(%d)\n",
	//	MUIC_DEV_NAME, __func__, voltage, vbus_voltage,
	//	muic_data->attached_dev);

	return vbus_voltage;
}

int sm5038_muic_get_vbus_voltage(void)
{
	struct sm5038_muic_data *muic_data = static_data;
	int vbus_voltage = 0;

	sm5038_irq_thread_lock();
	mutex_lock(&muic_data->muic_mutex);
	vbus_voltage = sm5038_muic_get_vbus();
	mutex_unlock(&muic_data->muic_mutex);
	sm5038_irq_thread_unlock();

	pr_info("[%s:%s] VBUS voltage:%d mV\n", MUIC_DEV_NAME, __func__, vbus_voltage);

	return vbus_voltage;
}
EXPORT_SYMBOL_GPL(sm5038_muic_get_vbus_voltage);

int sm5038_muic_get_device_type(int *dev_type1, int *dev_type2)
{
	struct sm5038_muic_data *muic_data = static_data;
	int dev1 = 0, dev2 = 0;
	
	dev1 = sm5038_muic_i2c_read_byte(muic_data->i2c, SM5038_MUIC_REG_DEVICETYPE1);
	dev2 = sm5038_muic_i2c_read_byte(muic_data->i2c, SM5038_MUIC_REG_DEVICETYPE2);
	if ( (dev1 < 0) || (dev2 < 0)) {
		return (-1);
	}

	*dev_type1 = dev1;
	*dev_type2 = dev2;

	pr_info("[%s:%s]Device Type[0x%02x,0x%02x]\n", MUIC_DEV_NAME, __func__, dev1, dev2);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_muic_get_device_type);

int sm5038_muic_hvdcp_voltage_control(struct sm5038_muic_data *muic_data, int voltage)
{
#if defined(CONFIG_SM5038_HVDCP_ENABLED)
	pr_info("[%s:%s] voltage: %d V attached_dev(%d)\n", MUIC_DEV_NAME, __func__, voltage, muic_data->attached_dev);

	if (!((muic_data->attached_dev == ATTACHED_DEV_DCP) ||
		(muic_data->attached_dev == ATTACHED_DEV_HVDCP))) {
		pr_info("[%s:%s] skip : It's not DCP or HVDCP\n", MUIC_DEV_NAME, __func__);
		return 0;
	}

	mutex_lock(&muic_data->muic_mutex);

	if (voltage == SM5038_HVDCP_5V) {

		pr_info("[%s:%s] 5V\n", MUIC_DEV_NAME, __func__);
		sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_DPDMCNTL, 0);
		muic_data->attached_dev = ATTACHED_DEV_DCP;
		sm5038_muic_cable_type_notification(muic_data, POWER_SUPPLY_TYPE_USB_DCP);

	} else if (voltage == SM5038_HVDCP_9V) {

		pr_info("[%s:%s] 9V\n", MUIC_DEV_NAME, __func__);
		sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_DPDMCNTL, 1);
		muic_data->attached_dev = ATTACHED_DEV_HVDCP;
		sm5038_muic_cable_type_notification(muic_data, POWER_SUPPLY_TYPE_USB_HVDCP);

	}

	mutex_unlock(&muic_data->muic_mutex);
#else
	pr_info("[%s:%s] not supported\n", MUIC_DEV_NAME, __func__);
#endif

	return 0;
}


int sm5038_muic_check_fled_state(int enable, int mode)
{
	struct sm5038_muic_data *muic_data = static_data;

	pr_info("[%s:%s] enable(%d), mode(%d)\n", MUIC_DEV_NAME, __func__,
			enable, mode);

	if (mode == FLEDEN_TORCH_ON) { /* torch */
		muic_data->fled_torch_enable = enable;
	} else if (mode == FLEDEN_FLASH_ON) { /* flash */
		muic_data->fled_flash_enable = enable;
	}

	pr_info("[%s:%s] fled_torch_enable(%d), fled_flash_enable(%d)\n",
			MUIC_DEV_NAME, __func__, muic_data->fled_torch_enable,
			muic_data->fled_flash_enable);

	if ((muic_data->fled_torch_enable == 1) ||
			(muic_data->fled_flash_enable == 1)) {
		sm5038_muic_hvdcp_voltage_control(muic_data, SM5038_HVDCP_5V);
	} else if ((muic_data->fled_torch_enable == false) &&
			(muic_data->fled_flash_enable == false)) {
		sm5038_muic_hvdcp_voltage_control(muic_data, SM5038_HVDCP_9V);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_muic_check_fled_state);


static int set_manual_sw(struct sm5038_muic_data *muic_data, bool on)
{
	struct i2c_client *i2c = muic_data->i2c;
	unsigned char reg_val = 0;
	int ret = 0;

	ret = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);
	if (ret < 0)
		pr_err("[%s:%s] err read MANUAL_SW(%d)\n",
			MUIC_DEV_NAME, __func__, ret);

	pr_info("[%s:%s] on:0x%x, reg_manual=0x%x\n", MUIC_DEV_NAME, __func__,
			on, ret);

	if (on) /* MANSW_AUTOMATIC:0  MANSW_MANUAL:1 */
		reg_val = ret | 0x80;
	else
		reg_val = ret & 0x7F;

	pr_info("[%s:%s] reg_val(0x%x) != ret(0x%x) update\n",
		MUIC_DEV_NAME, __func__, reg_val, ret);

	ret = sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_MANUAL_SW,
			reg_val);
	if (ret < 0)
		pr_err("[%s:%s] err write(%d)\n",
			MUIC_DEV_NAME, __func__, ret);

	ret = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);
	if (ret < 0)
		pr_err("[%s:%s] err read MANUAL_SW(%d)\n",
				MUIC_DEV_NAME, __func__, ret);
	else
		pr_info("[%s:%s] after change(0x%x)\n",
			MUIC_DEV_NAME, __func__, ret);

	return ret;
}

static int set_com_sw(struct sm5038_muic_data *muic_data,
			enum sm5038_reg_manual_sw_value reg_val)
{
	struct i2c_client *i2c = muic_data->i2c;
	int ret = 0;
	int temp = 0;

	/*  --- MANSW [5:3][2:0] : DM DP  --- */
	temp = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);
	if (temp < 0)
		pr_err("[%s:%s] err read MANSW(0x%x)\n",
			MUIC_DEV_NAME, __func__, temp);

	pr_info("[%s:%s]rev_val(0x%x) != temp(0x%x), update\n",
		MUIC_DEV_NAME, __func__, reg_val, temp);

	temp = (temp&0xC0)|reg_val;

	ret = sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_MANUAL_SW, temp);
	if (ret < 0)
		pr_err("[%s:%s] err write MANSW(0x%x)\n",
			MUIC_DEV_NAME, __func__, reg_val);

	return ret;
}

static int com_to_open(struct sm5038_muic_data *muic_data)
{
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	ret = set_com_sw(muic_data, MANSW_OPEN);
	if (ret)
		pr_err("[%s:%s] set_com_sw err\n", MUIC_DEV_NAME, __func__);

	set_manual_sw(muic_data, MANSW_MANUAL); /* auto:0  manual:1 */

	return ret;
}

static int com_to_usb(struct sm5038_muic_data *muic_data)
{
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	ret = set_com_sw(muic_data, MANSW_USB);
	if (ret)
		pr_err("[%s:%s]set_com_usb err\n", MUIC_DEV_NAME, __func__);

	set_manual_sw(muic_data, MANSW_MANUAL); /* auto:0  manual:1 */

	return ret;
}

/* sw_control : 0 : open */
/*              1 : USB  */
/*              2 : UART */
int sm5038_manual_sw_control(int sw_control)
{
	struct sm5038_muic_data *muic_data = static_data;
	int ret = 0;

	pr_info("[%s:%s] sw_control = %d\n", MUIC_DEV_NAME, __func__, sw_control);

	if (sw_control == 1){ // USB
		ret = set_com_sw(muic_data, MANSW_USB);
		if (ret)
			pr_err("[%s:%s]usb err\n", MUIC_DEV_NAME, __func__);
	} else if (sw_control == 2){ // UART
		ret = set_com_sw(muic_data, MANSW_UART);
		if (ret)
			pr_err("[%s:%s]uart err\n", MUIC_DEV_NAME, __func__);
	} else {
		ret = set_com_sw(muic_data, MANSW_OPEN);
		if (ret)
			pr_err("[%s:%s]open err\n", MUIC_DEV_NAME, __func__);
	}

	set_manual_sw(muic_data, MANSW_MANUAL); /* auto:0  manual:1 */	
	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_manual_sw_control);


#if 0
static int com_to_uart(struct sm5038_muic_data *muic_data)
{
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	ret = set_com_sw(muic_data, MANSW_UART);
	if (ret)
		pr_err("[%s:%s] set_com_uart err\n", MUIC_DEV_NAME, __func__);

	set_manual_sw(muic_data, MANSW_MANUAL); /* auto:0  manual:1 */

	return ret;
}
#endif

static int com_to_JTAG(struct sm5038_muic_data *muic_data)
{
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	ret = set_com_sw(muic_data, MANSW_JTAG);
	if (ret)
		pr_err("[%s:%s] com_to_JTag err\n", MUIC_DEV_NAME, __func__);

	set_manual_sw(muic_data, MANSW_MANUAL); /* auto:0  manual:1 */

	return ret;
}


static void sm5038_muic_set_bc12(struct sm5038_muic_data *muic_data,
		int enable)
{
	struct i2c_client *i2c = muic_data->i2c;
	int reg_value = 0;

	reg_value = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL);
	if (reg_value < 0) {
		pr_err("[%s:%s] failed, return\n", MUIC_DEV_NAME, __func__);
		return;
	}
	if (enable == 1) /* 0:bc12 disable, 1:bc12 enable*/
		reg_value = reg_value & 0xFD;
	else
		reg_value = reg_value | 0x02;

	sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_CNTL, reg_value);
	pr_info("[%s:%s] enable = %d, reg_value = 0x%x\n", MUIC_DEV_NAME,
			__func__, enable, reg_value);
}

static void sm5038_muic_cable_type_notification(struct sm5038_muic_data *muic_data, int cable_type)
{
	pr_info("[%s:%s] power_supply_cable_type:%d\n", MUIC_DEV_NAME, __func__, cable_type);

	muic_data->power_supply_cable_type = cable_type;
	schedule_delayed_work(&muic_data->muic_noti_work, 0);

}

static void sm5038_muic_handle_attach(struct sm5038_muic_data *muic_data,
			int new_dev, unsigned char vbvolt, int irq)
{
	int ret = 0;
	bool noti = (new_dev != muic_data->attached_dev) ? true : false;
	int power_supply_type = 0;

	if ( irq == SM5038_MUIC_IRQ_PROBE ) {
		noti = true;
	}

	pr_info("[%s:%s] attached_dev:%d, new_dev:%d noti:%d\n",
		MUIC_DEV_NAME, __func__, muic_data->attached_dev, new_dev, noti);

	switch (new_dev) {
	case ATTACHED_DEV_USB:
		ret = com_to_usb(muic_data);
		power_supply_type = POWER_SUPPLY_TYPE_USB;
		break;
	case ATTACHED_DEV_CDP:
		ret = com_to_usb(muic_data);
		power_supply_type = POWER_SUPPLY_TYPE_USB_CDP;
		break;
	case ATTACHED_DEV_DCDTIMEOUT:
		ret = com_to_usb(muic_data);
		power_supply_type = POWER_SUPPLY_TYPE_USB_FLOAT;
		break;
	case ATTACHED_DEV_DCP:
		ret = com_to_open(muic_data);
		power_supply_type = POWER_SUPPLY_TYPE_USB_DCP;
		break;
	case ATTACHED_DEV_LOTA:
		ret = com_to_open(muic_data);
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		power_supply_type = POWER_SUPPLY_TYPE_USB_DCP;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		power_supply_type = POWER_SUPPLY_TYPE_APPLE_BRICK_ID;
#endif
		break;
	case ATTACHED_DEV_JTAG:
		ret = com_to_JTAG(muic_data);
		noti = false;
		break;
	case ATTACHED_DEV_HVDCP:
		ret = com_to_open(muic_data);
#if defined(CONFIG_SM5038_HVDCP_ENABLED)
		power_supply_type = POWER_SUPPLY_TYPE_USB_HVDCP;
		sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_DPDMCNTL, 1);
#else
		power_supply_type = POWER_SUPPLY_TYPE_USB_DCP;
#endif
		break;
	default:
		noti = false;
		pr_info("[%s:%s] unsupported dev=%d, vbus=%c\n",
			MUIC_DEV_NAME, __func__, new_dev, (vbvolt ? 'O' : 'X'));
		break;
	}

	if (ret)
		pr_err("[%s:%s] something wrong %d (ERR=%d)\n",
			MUIC_DEV_NAME, __func__, new_dev, ret);

	pr_info("[%s:%s] done\n", MUIC_DEV_NAME, __func__);

	muic_data->attached_dev = new_dev;

	if (noti) {
		sm5038_muic_cable_type_notification(muic_data, power_supply_type);
	}

}

static void sm5038_muic_handle_detach(struct sm5038_muic_data *muic_data,
		int irq)
{
	int ret = 0;
	bool noti = true;

	pr_info("[%s:%s] attached_dev:%d\n", MUIC_DEV_NAME, __func__,
			muic_data->attached_dev);

	switch (muic_data->attached_dev) {
	case ATTACHED_DEV_USB:
	case ATTACHED_DEV_CDP:
	case ATTACHED_DEV_DCDTIMEOUT:
	case ATTACHED_DEV_DCP:
	case ATTACHED_DEV_LOTA:
	case ATTACHED_DEV_JTAG:
		ret = com_to_open(muic_data);
		break;
	case ATTACHED_DEV_HVDCP:
		ret = com_to_open(muic_data);
		sm5038_set_ctrl2_reg(muic_data, REG_CTRL2_DPDMCNTL, 0);
		break;
	default:
		noti = false;
		pr_info("[%s:%s] invalid type(%d)\n",
			MUIC_DEV_NAME, __func__, muic_data->attached_dev);
		break;
	}

	if (ret)
		pr_err("[%s:%s] something wrong %d (ERR=%d)\n",
			MUIC_DEV_NAME, __func__, muic_data->attached_dev, ret);

	muic_data->attached_dev = ATTACHED_DEV_NONE;

	if (noti) {
		sm5038_muic_cable_type_notification(muic_data, POWER_SUPPLY_TYPE_BATTERY);
	}
}

static void sm5038_muic_detect_dev(struct sm5038_muic_data *muic_data, int irq)
{
	struct i2c_client *i2c = muic_data->i2c;
	sm5038_muic_attached_dev_t new_dev = ATTACHED_DEV_NONE;
	int intr = MUIC_INTR_DETACH;
	int dev1 = 0, dev2 = 0, ctrl = 0, manualsw = 0, ctrl2 = 0, vbvolt = 0;

	dev1     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE1);
	dev2     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE2);
	ctrl     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL);
	manualsw = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);
	ctrl2    = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL2);
	vbvolt   = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS);
	vbvolt   = (vbvolt&0x04)>>2;

	pr_info("[%s:%s] dev1:0x%02x, dev2:0x%02x, ctrl:0x%02x, ma:0x%02x, ctrl2:0x%02x, vbvolt:0x%02x\n",
			MUIC_DEV_NAME, __func__, dev1, dev2, ctrl,
			manualsw, ctrl2, vbvolt);

	muic_data->dev1 = dev1;
	muic_data->dev2 = dev2;

	if (dev1 & DEV_TYPE1_LO_TA) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_LO_TA + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_LOTA;
		pr_info("[%s:%s] LO_TA\n", MUIC_DEV_NAME, __func__);

	} else if (dev1 & DEV_TYPE1_DCP2) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_DCP2 + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_DCP;
		pr_info("[%s:%s] DCP2\n", MUIC_DEV_NAME, __func__);

	} else if (dev1 & DEV_TYPE1_CDP) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_CDP + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_CDP;
		pr_info("[%s:%s] CDP\n", MUIC_DEV_NAME, __func__);

	} else if (dev1 & DEV_TYPE1_DCP) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_DCP + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_DCP;
		pr_info("[%s:%s] DCP\n", MUIC_DEV_NAME, __func__);

	} else if (dev1 & DEV_TYPE1_SDP) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_SDP(USB) + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_USB;
		pr_info("[%s:%s] USB(SDP)\n", MUIC_DEV_NAME, __func__);

	} else if (dev1 & DEV_TYPE1_DCD_OUT_SDP) {
		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE1_DCD_OUT_SDP + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_DCDTIMEOUT;
		pr_info("[%s:%s] DCD_OUT_SDP\n", MUIC_DEV_NAME, __func__);
	}

	if (dev2 & DEV_TYPE2_DEBUG_JTAG) {

		pr_info("[%s:%s] DEBUG_JTAG\n", MUIC_DEV_NAME, __func__);
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JTAG;

	} else if (dev2 & DEV_TYPE2_HVDCP) {

		if (!vbvolt) {
			pr_info("[%s:%s] DEV_TYPE2 HVDCP + NO VBUS\n",
					MUIC_DEV_NAME, __func__);
			return;
		}
		intr = MUIC_INTR_ATTACH;
#if defined(CONFIG_SM5038_HVDCP_ENABLED)
		new_dev = ATTACHED_DEV_HVDCP;
		pr_info("[%s:%s] HVDCP\n", MUIC_DEV_NAME, __func__);
#else
		new_dev = ATTACHED_DEV_DCP;
		pr_info("[%s:%s] .DCP.\n", MUIC_DEV_NAME, __func__);
#endif

	}

	if (intr == MUIC_INTR_ATTACH)
		sm5038_muic_handle_attach(muic_data, new_dev, vbvolt, irq);
	else
		sm5038_muic_handle_detach(muic_data, irq);
}

static int sm5038_muic_reg_init(struct sm5038_muic_data *muic_data)
{
	struct i2c_client *i2c = muic_data->i2c;
	int intmask1 = 0, intmask2 = 0, cntl = 0, manualsw = 0;
	int dev1 = 0, dev2 = 0, cntl2 = 0, vbus_voltage = 0;
	int cfg1 = 0, cfg2 = 0;
	int vbus = 0, bcd_state = 0, otp_if_sts = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_CNTL,  0x24);
	sm5038_muic_i2c_write_byte(i2c, SM5038_MUIC_REG_CNTL2, 0x00);

	intmask1 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INTMASK1);
	intmask2 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INTMASK2);
	cntl     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL);
	manualsw = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);

	dev1     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE1);
	dev2     = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE2);
	cntl2    = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL2);
	vbus_voltage = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS_VOLTAGE);

	cfg1 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CFG1);
	cfg2 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CFG2);
	vbus = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS);
	bcd_state = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_BCD_STATE);
	otp_if_sts = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_OTP_IF_STS);


	pr_info("[%s:%s] intmask1:0x%x, intmask2:0x%x, cntl:0x%x, mansw:0x%x\n",
		MUIC_DEV_NAME, __func__, intmask1, intmask2, cntl, manualsw);
	pr_info("[%s:%s] dev1:0x%x, dev2:0x%x, cntl2:0x%x, vbus_voltage:0x%x\n",
		MUIC_DEV_NAME, __func__, dev1, dev2, cntl2, vbus_voltage);
	pr_info("[%s:%s] cfg1:0x%x, cfg2=0x%x, vbus=0x%x, bcd_state=0x%x, otp_if_sts=0x%x\n",
		MUIC_DEV_NAME, __func__, cfg1, cfg2, vbus, bcd_state, otp_if_sts);

	return 0;
}

static irqreturn_t sm5038_muic_irq_thread(int irq, void *data)
{
	struct sm5038_muic_data *muic_data = data;
	int irq_num = irq - muic_data->mfd_pdata->irq_base;

	mutex_lock(&muic_data->muic_mutex);
	__pm_stay_awake(muic_data->wake_lock);

	if (irq != (SM5038_MUIC_IRQ_PROBE)) {
		pr_info("[%s:%s] irq_gpio(%d), irq (%d), irq_num(%d:%s)\n",
				MUIC_DEV_NAME, __func__,
				muic_data->mfd_pdata->irq_base, irq, irq_num,
				SM5038_MUIC_INT_NAME[irq_num]);
	} else {
		pr_info("[%s:%s] irq_gpio(%d), irq (%d), irq_num(%d)\n",
				MUIC_DEV_NAME, __func__,
				muic_data->mfd_pdata->irq_base, irq, irq_num);
	}

	if ((irq == (SM5038_MUIC_IRQ_PROBE)) ||
		(irq_num == SM5038_MUIC_IRQ_INT1_DPDM_OVP) ||
		(irq_num == SM5038_MUIC_IRQ_INT1_VBUS_DETACH) ||
		(irq_num == SM5038_MUIC_IRQ_INT1_CHGTYPE) ||
		(irq_num == SM5038_MUIC_IRQ_INT1_DCDTIMEOUT) ||
		(irq_num == SM5038_MUIC_IRQ_INT2_HVDCP)) {

		sm5038_muic_detect_dev(muic_data, irq);
	}

	__pm_relax(muic_data->wake_lock);
	mutex_unlock(&muic_data->muic_mutex);

	pr_info("[%s:%s] done\n", MUIC_DEV_NAME, __func__);
	return IRQ_HANDLED;
}

static void sm5038_muic_debug_reg_log(struct work_struct *work)
{
	struct sm5038_muic_data *muic_data = container_of(work,
			struct sm5038_muic_data, muic_debug_work.work);
	struct i2c_client *i2c = muic_data->i2c;
	unsigned char data[13] = {0, };

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (!muic_data->regdump_en)
		return;
#endif
	data[0]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INTMASK1);
	data[1]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_INTMASK2);
	data[2]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL);
	data[3]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_MANUAL_SW);
	data[4]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE1);
	data[5]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE2);

	data[6]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CNTL2);
	data[7]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS_VOLTAGE);
	data[8]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CFG1);
	data[9]  = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_CFG2);

	data[10] = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_VBUS);
	data[11] = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_BCD_STATE);
	data[12] = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_OTP_IF_STS);

	pr_info("[%s:%s]IM[0x%02x 0x%02x] CNTL:0x%02x MAN_SW:0x%02x DEV_TYPE[0x%02x 0x%02x]\n",
			MUIC_DEV_NAME, __func__, data[0], data[1], data[2],
			data[3], data[4], data[5]);

	pr_info("[%s:%s]CNTL2:0x%02x VBUS_VOL:%d CFG1:0x%02x CFG2:0x%02x",
			MUIC_DEV_NAME, __func__, data[6], data[7], data[8], data[9]);

	pr_info("[%s:%s]VBUS:0x%02x BCD_STATE:0x%02x OTP_IF_STS:0x%02x\n",
			MUIC_DEV_NAME, __func__, data[10], data[11], data[12]);

	schedule_delayed_work(&muic_data->muic_debug_work,
						msecs_to_jiffies(60000)); /* 60 sec */		

	return;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define CC_DISCONNECTION_PERIOD_MS	400
#define CC_RECONNECTION_CHECK_PERIOD_MS	10
#define CC_RECONNECTION_CHECK_MAX	100

bool somc_sm5038_is_cc_reconnection_running(void)
{
	struct sm5038_muic_data *muic_data = static_data;

	return muic_data->cc_reconnection_running;
}

#endif
static void sm5038_muic_init_detect(struct work_struct *work)
{
	struct sm5038_muic_data *muic_data = container_of(work,
			struct sm5038_muic_data, muic_init_detect_work.work);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	int i;
	struct power_supply *usb_psy;
	union power_supply_propval pval = {0, };
#endif

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	/* initial cable detection */
	sm5038_muic_irq_thread(SM5038_MUIC_IRQ_PROBE, muic_data);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (muic_data->attached_dev != ATTACHED_DEV_DCP) {
		pr_info("%s: DCP is not attached, CC reconnection is not needed\n",
								__func__);
		return;
	}

	if (somc_sm5038_present_is_pd_apdapter()) {
		pr_info("%s: PD charger is detected already, CC reconnection is not needed\n",
								__func__);
		return;
	}

	usb_psy = power_supply_get_by_name("usb");
	if (!usb_psy) {
		pr_info("%s: Failed to get usb_psy\n", __func__);
		return;
	}

	pr_info("%s: disconnecting CC\n", __func__);
	muic_data->cc_reconnection_running = true;
	sm5038_cc_control_command(1);

	msleep(CC_DISCONNECTION_PERIOD_MS);

	sm5038_cc_control_command(0);

	/* Wait until USB online is set after restoring CC */
	for (i = 0; i < CC_RECONNECTION_CHECK_MAX; i++) {
		power_supply_get_property(usb_psy, POWER_SUPPLY_PROP_ONLINE,
									&pval);
		if (pval.intval == 1)
			break;

		msleep(CC_RECONNECTION_CHECK_PERIOD_MS);
	}

	muic_data->cc_reconnection_running = false;
	pr_info("%s: re-connecting CC as Sink\n", __func__);
#endif
	return;
}

static void sm5038_muic_noti(struct work_struct *work)
{
	struct sm5038_muic_data *muic_data = container_of(work,
			struct sm5038_muic_data, muic_noti_work.work);
	union power_supply_propval val = {0, };

	pr_info("[%s:%s] power_supply_cable_type=%d\n", MUIC_DEV_NAME, __func__, muic_data->power_supply_cable_type);

	val.intval = muic_data->power_supply_cable_type;
	psy_do_property("battery", set,
		POWER_SUPPLY_PROP_ONLINE, val);

	return;
}

static int sm5038_init_rev_info(struct sm5038_muic_data *muic_data)
{
	int vendor_id = 0;
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	vendor_id = sm5038_muic_i2c_read_byte(muic_data->i2c, SM5038_MUIC_REG_DEVICEID);
	if (vendor_id < 0) {
		pr_err("[%s:%s] vendor_id(%d)\n", MUIC_DEV_NAME, __func__, vendor_id);
		ret = -ENODEV;
	} else {
		muic_data->muic_vendor = (vendor_id & 0x07);
		pr_info("[%s:%s] vendor_id=0x%x\n",
			MUIC_DEV_NAME, __func__, muic_data->muic_vendor);
	}
	return ret;
}
void sm5038_muic_register_reset(struct i2c_client *i2c, void *muic)
{
	struct sm5038_muic_data *muic_data = static_data;

	if (muic_data == NULL) {
		pr_info("[%s:%s] static muic fail", MUIC_DEV_NAME, __func__);
		return;
	}

	pr_info("[%s:%s] MUIC RESET\n", MUIC_DEV_NAME, __func__);

	sm5038_muic_i2c_write_byte(muic_data->i2c, SM5038_MUIC_REG_RESET, 0x01);

	/* reg init */
	sm5038_muic_i2c_write_byte(muic_data->i2c, SM5038_MUIC_REG_INTMASK1, 0xCC);
	sm5038_muic_i2c_write_byte(muic_data->i2c, SM5038_MUIC_REG_INTMASK2, 0xFE);

	sm5038_muic_reg_init(muic_data);

}
EXPORT_SYMBOL_GPL(sm5038_muic_register_reset);
#define REQUEST_IRQ(_irq, _dev_id, _name)				\
do {									\
	ret = request_threaded_irq(_irq, NULL, sm5038_muic_irq_thread,	\
				0, _name, _dev_id);	\
	if (ret < 0) {							\
		pr_err("[%s:%s] Failed to request IRQ #%d: %d\n",	\
				MUIC_DEV_NAME, __func__, _irq, ret);	\
		_irq = 0;						\
	}								\
} while (0)

static int sm5038_muic_irq_init(struct sm5038_muic_data *muic_data)
{
	int irq_base = muic_data->mfd_pdata->irq_base;
	int ret = 0;

	if (muic_data->mfd_pdata && (muic_data->mfd_pdata->irq_base > 0)) {
		/* request MUIC IRQ */
		muic_data->irqs.irq_dpdm_ovp =
			irq_base + SM5038_MUIC_IRQ_INT1_DPDM_OVP;
		REQUEST_IRQ(muic_data->irqs.irq_dpdm_ovp, muic_data,
				"muic-dpdm_ovp");

		muic_data->irqs.irq_vbus_detach =
			irq_base + SM5038_MUIC_IRQ_INT1_VBUS_DETACH;
		REQUEST_IRQ(muic_data->irqs.irq_vbus_detach, muic_data,
				"muic-vbus_detach");

		muic_data->irqs.irq_chgtype_attach =
			irq_base + SM5038_MUIC_IRQ_INT1_CHGTYPE;
		REQUEST_IRQ(muic_data->irqs.irq_chgtype_attach, muic_data,
				"muic-chgtype_attach");

		muic_data->irqs.irq_dcdtimeout =
			irq_base + SM5038_MUIC_IRQ_INT1_DCDTIMEOUT;
		REQUEST_IRQ(muic_data->irqs.irq_dcdtimeout, muic_data,
				"muic-dcdtimeout");

		muic_data->irqs.irq_hvdcp =
			irq_base + SM5038_MUIC_IRQ_INT2_HVDCP;
		REQUEST_IRQ(muic_data->irqs.irq_hvdcp, muic_data,
				"muic-hvdcp");
	}

	pr_info("[%s:%s] muic-dpdm_ovp(%d), muic-vbus_detach(%d)",
		MUIC_DEV_NAME, __func__, muic_data->irqs.irq_dpdm_ovp,
		muic_data->irqs.irq_vbus_detach);

	pr_info("[%s:%s] muic-chgtype_attach(%d), muic-dcdtimeout(%d)\n",
		MUIC_DEV_NAME, __func__, muic_data->irqs.irq_chgtype_attach,
		muic_data->irqs.irq_dcdtimeout);

	pr_info("[%s:%s] muic-hvdcp(%d)\n",
		MUIC_DEV_NAME, __func__, muic_data->irqs.irq_hvdcp);

	return ret;
}

#define FREE_IRQ(_irq, _dev_id, _name)					\
do {									\
	if (_irq) {							\
		free_irq(_irq, _dev_id);				\
		pr_info("[%s:%s] IRQ(%d):%s free done\n", MUIC_DEV_NAME,\
				__func__, _irq, _name);			\
	}								\
} while (0)

static void sm5038_muic_free_irqs(struct sm5038_muic_data *muic_data)
{
	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	/* free MUIC IRQ */
	FREE_IRQ(muic_data->irqs.irq_dpdm_ovp, muic_data, "muic-dpdm_ovp");
	FREE_IRQ(muic_data->irqs.irq_vbus_detach, muic_data, "muic-vbus_detach");
	FREE_IRQ(muic_data->irqs.irq_chgtype_attach, muic_data, "muic-chgtype_attach");
	FREE_IRQ(muic_data->irqs.irq_dcdtimeout, muic_data, "muic-dcdtimeout");

	FREE_IRQ(muic_data->irqs.irq_hvdcp, muic_data, "muic-hvdcp");

}

static enum power_supply_property sm5038_muic_usb_port_props[] = {
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TECHNOLOGY,
};
	
static int sm5038_muic_usb_port_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct sm5038_muic_data *muic_data = power_supply_get_drvdata(psy);
	struct i2c_client *i2c = muic_data->i2c;
	int dev1 = 0;
	int ret = 0;
	int ua = 0;
	int charge_mode = 0;
	int chgon_status = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = POWER_SUPPLY_TYPE_USB;
		pr_debug("[%s:%s] POWER_SUPPLY_PROP_TYPE : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		dev1 = sm5038_muic_i2c_read_byte(i2c, SM5038_MUIC_REG_DEVICETYPE1);
		//pr_debug("[%s:%s] dev1:0x%x\n", MUIC_DEV_NAME, __func__, dev1);
		if (dev1 & DEV_TYPE1_SDP) {
			charge_mode = sm5038_charger_get_chg_mode_type();
			chgon_status = sm5038_charger_get_chg_on_status();
			//pr_debug("[%s:%s] charge_mode:%d\n", MUIC_DEV_NAME, __func__, charge_mode);
			if (charge_mode < 0) {
				return charge_mode;
			}
			/* charge_mode 0 : SM5038_CHG_MODE_CHARGING_OFF */
			/*             1 : SM5038_CHG_MODE_CHARGING_ON  */
			/*             2 : SM5038_CHG_MODE_BUCK_OFF     */
			if (charge_mode == 2 || chgon_status == 0) {
				val->intval = 0;
			} else {
				val->intval = 1;
			}
		} else {
			val->intval = 0;
		}
		pr_debug("[%s:%s] POWER_SUPPLY_PROP_ONLINE : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = 5000000;
		pr_debug("[%s:%s] POWER_SUPPLY_PROP_VOLTAGE_MAX : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		ua = sm5038_charger_oper_get_input_current_ua();
		val->intval = ua * 1000;
		pr_debug("[%s:%s] POWER_SUPPLY_PROP_CURRENT_MAX : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = 0;
		pr_debug("[%s:%s] POWER_SUPPLY_PROP_TECHNOLOGY : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		break;		
	default:
		pr_err_ratelimited("Get prop %d is not supported in pc_port\n",
				psp);
		return -EINVAL;
	}

	if (ret < 0) {
		pr_debug("Couldn't get prop %d ret = %d\n", psp, ret);
		return -ENODATA;
	}

	return 0;
}

static int sm5038_muic_usb_port_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		pr_info("[%s:%s] POWER_SUPPLY_PROP_TECHNOLOGY : val->intval=%d\n", MUIC_DEV_NAME, __func__, val->intval);
		sm5038_manual_sw_control(val->intval);
		break;

	default:
		pr_err_ratelimited("Set prop %d is not supported in pc_port\n",
				psp);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int sm5038_muic_pc_port_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		return 1;
	default:
		break;
	}

	return 0;
}

static char *sm5038_muic_supplied_to[] = {
	"sm5038-muic",
};

static const struct power_supply_desc sm5038_muic_usb_port_psy_desc = {
	.name           = "pc_port",
	.type           = POWER_SUPPLY_TYPE_USB,
	.properties     = sm5038_muic_usb_port_props,
	.num_properties = ARRAY_SIZE(sm5038_muic_usb_port_props),
	.get_property   = sm5038_muic_usb_port_get_prop,
	.set_property   = sm5038_muic_usb_port_set_prop,
	.property_is_writeable = sm5038_muic_pc_port_prop_is_writeable,
};

static int sm5038_muic_init_usb_port_psy(struct sm5038_muic_data *muic_data)
{
	struct power_supply_config usb_port_cfg = {};

	usb_port_cfg.drv_data = muic_data;
	usb_port_cfg.supplied_to = sm5038_muic_supplied_to;
	usb_port_cfg.num_supplicants = ARRAY_SIZE(sm5038_muic_supplied_to);
	
	muic_data->usb_port_psy = devm_power_supply_register(muic_data->dev,
						  &sm5038_muic_usb_port_psy_desc,
						  &usb_port_cfg);
	if (IS_ERR(muic_data->usb_port_psy)) {
		pr_err("Couldn't register USB pc_port power supply\n");
		return PTR_ERR(muic_data->usb_port_psy);
	}

	return 0;
}

//static const struct of_device_id sm5038_muic_match_table[] = {
//	{ .compatible = "sm5038-muic",},
//	{},
//};

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#if defined(CONFIG_DEBUG_FS)
static int debug_mask_get(void *data, u64 *val)
{
	struct sm5038_muic_data *muic_data = data;

	*val = (u64)muic_data->debug_mask;

	return 0;
}
static int debug_mask_set(void *data, u64 val)
{
	struct sm5038_muic_data *muic_data = data;

	muic_data->debug_mask = (int)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(debug_mask_ops, debug_mask_get, debug_mask_set,
								"%d\n");

static int reg_addr_get(void *data, u64 *val)
{
	struct sm5038_muic_data *muic_data = data;

	*val = (u64)muic_data->dfs_reg_addr;

	return 0;
}
static int reg_addr_set(void *data, u64 val)
{
	struct sm5038_muic_data *muic_data = data;

	muic_data->dfs_reg_addr = (u8)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_addr_ops, reg_addr_get, reg_addr_set,
								"0x%02llx\n");

static int reg_data_get(void *data, u64 *val)
{
	struct sm5038_muic_data *muic_data = data;
	u8 reg_data;

	sm5038_read_reg(muic_data->i2c, muic_data->dfs_reg_addr, &reg_data);
	*val = (u64)reg_data;

	return 0;
}
static int reg_data_set(void *data, u64 val)
{
	struct sm5038_muic_data *muic_data = data;
	u8 reg_data;

	reg_data = (u8)val;
	sm5038_update_reg(muic_data->i2c, muic_data->dfs_reg_addr, reg_data,
									0xff);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_data_ops, reg_data_get, reg_data_set,
								"0x%02llx\n");

static int regdump_en_get(void *data, u64 *val)
{
	struct sm5038_muic_data *muic_data = data;

	*val = (u64)muic_data->regdump_en;

	return 0;
}
static int regdump_en_set(void *data, u64 val)
{
	struct sm5038_muic_data *muic_data = data;

	muic_data->regdump_en = (int)val;
	cancel_delayed_work_sync(&muic_data->muic_debug_work);
	schedule_delayed_work(&muic_data->muic_debug_work, 0);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(regdump_en_ops, regdump_en_get, regdump_en_set,
								"%d\n");

static void somc_sm5038_create_debugfs(struct sm5038_muic_data *muic_data)
{
	struct dentry *file;
	struct dentry *dir;

	dir = debugfs_create_dir("sm5038_muic", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		pr_err("Couldn't create sm5038_muic debugfs rc=%ld\n",
								(long)dir);
		return;
	}

	file = debugfs_create_file("debug_mask", S_IFREG | 0644,
					dir, muic_data, &debug_mask_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create debug_mask file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_addr", S_IFREG | 0644,
						dir, muic_data, &reg_addr_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_addr file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_data", S_IFREG | 0644,
						dir, muic_data, &reg_data_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_data file rc=%ld\n", (long)file);

	file = debugfs_create_file("regdump_en", S_IFREG | 0644,
					dir, muic_data, &regdump_en_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create regdump_en file rc=%ld\n", (long)file);
}
#else
static void somc_sm5038_create_debugfs(struct sm5038_muic_data *muic_data)
{
}

#endif
#endif
int sm5038_muic_probe(struct sm5038_dev *sm5038)
{
	//struct sm5038_dev *sm5038 = dev_get_drvdata(pdev->dev.parent);
	//struct sm5038_platform_data *mfd_pdata = dev_get_platdata(sm5038->dev);
	struct sm5038_muic_data *muic_data;
	int ret = 0;

	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

	muic_data = kzalloc(sizeof(struct sm5038_muic_data), GFP_KERNEL);
	if (!muic_data) {
		ret = -ENOMEM;
		goto err_return;
	}

	//if (!mfd_pdata) {
	//	pr_err("[%s:%s] failed to get sm5038 mfd platform data\n",
	//			MUIC_DEV_NAME, __func__);
	//	ret = -ENOMEM;
	//	goto err_kfree1;
	//}

	/* save platfom data for gpio control functions */
	static_data = muic_data;

	muic_data->dev = sm5038->dev;
	muic_data->i2c = sm5038->muic_i2c;
	muic_data->mfd_pdata = sm5038->pdata;

	muic_data->fled_torch_enable = 0;
	muic_data->fled_flash_enable = 0;

	muic_data->attached_dev = ATTACHED_DEV_NONE;
	sm5038->check_muic_reset = sm5038_muic_register_reset;
	//platform_set_drvdata(pdev, muic_data);

	if (muic_data->i2c == NULL) {
		pr_err("[%s:%s] i2c NULL\n", MUIC_DEV_NAME, __func__);
		goto err_kfree1;
	}

	mutex_init(&muic_data->muic_mutex);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	muic_data->wake_lock = wakeup_source_create("5038_muic_wake");	/* 4.19 */
	if (muic_data->wake_lock)
		wakeup_source_add(muic_data->wake_lock);
#else
	muic_data->wake_lock = wakeup_source_register(NULL, "5038_muic_wake"); /* 5.4  */
#endif


	ret = sm5038_init_rev_info(muic_data);
	if (ret) {
		pr_err("[%s:%s] failed to init muic(%d)\n",
				MUIC_DEV_NAME, __func__, ret);
		goto err_kfree1;
	}

	sm5038_muic_reg_init(muic_data);

	ret = sm5038_muic_irq_init(muic_data);
	if (ret) {
		pr_err("[%s:%s] failed to init irq(%d)\n",
				MUIC_DEV_NAME, __func__, ret);
		goto fail_init_irq;
	}


	INIT_DELAYED_WORK(&muic_data->muic_debug_work,
						sm5038_muic_debug_reg_log);
	schedule_delayed_work(&muic_data->muic_debug_work,
						msecs_to_jiffies(30000)); /* 30 sec */

	INIT_DELAYED_WORK(&muic_data->muic_init_detect_work,
						sm5038_muic_init_detect);
	schedule_delayed_work(&muic_data->muic_init_detect_work,
						msecs_to_jiffies(3000)); /* 3 sec */
	INIT_DELAYED_WORK(&muic_data->muic_noti_work,
						sm5038_muic_noti);
	ret = sm5038_muic_init_usb_port_psy(muic_data);
	if (ret < 0) {
		pr_err("[%s:%s]Couldn't initialize pc_port psy ret=%d\n", MUIC_DEV_NAME, __func__, ret);
		goto fail_init_irq;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_sm5038_create_debugfs(muic_data);
	muic_data->debug_mask = PR_CORE;
#endif
	pr_info("[%s:%s] Done.\n", MUIC_DEV_NAME, __func__);

	return 0;

fail_init_irq:
	sm5038_muic_free_irqs(muic_data);

	mutex_destroy(&muic_data->muic_mutex);

err_kfree1:
	kfree(muic_data);
err_return:
	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_muic_probe);


int sm5038_muic_remove(void)
{
	struct sm5038_muic_data *muic_data = static_data;

	if (muic_data) {
		pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);

		cancel_delayed_work_sync(&muic_data->muic_debug_work);
		cancel_delayed_work_sync(&muic_data->muic_init_detect_work);
		cancel_delayed_work_sync(&muic_data->muic_noti_work);
		wakeup_source_unregister(muic_data->wake_lock);
		disable_irq_wake(muic_data->i2c->irq);
		sm5038_muic_free_irqs(muic_data);
		mutex_destroy(&muic_data->muic_mutex);
		i2c_set_clientdata(muic_data->i2c, NULL);
		kfree(muic_data);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_muic_remove);

void sm5038_muic_shutdown(void)
{
	struct sm5038_muic_data *muic_data = static_data;
	int ret;

	cancel_delayed_work_sync(&muic_data->muic_debug_work);
	cancel_delayed_work_sync(&muic_data->muic_init_detect_work);
	cancel_delayed_work_sync(&muic_data->muic_noti_work);
	pr_info("[%s:%s]\n", MUIC_DEV_NAME, __func__);
	if (!muic_data->i2c) {
		pr_err("[%s:%s] no muic i2c client\n", MUIC_DEV_NAME, __func__);
		return;
	}

	pr_info("[%s:%s] open D+,D-,V_bus line\n", MUIC_DEV_NAME, __func__);
	ret = com_to_open(muic_data);
	if (ret < 0)
		pr_err("[%s:%s] fail to open mansw\n", MUIC_DEV_NAME, __func__);

	/* set auto sw mode before shutdown to make sure device goes into */
	/* LPM charging when TA or USB is connected during off state */
	pr_info("[%s:%s] muic auto detection enable\n",
			MUIC_DEV_NAME, __func__);

	ret = set_manual_sw(muic_data, MANSW_AUTOMATIC); /* auto:0  manual:1 */
	if (ret < 0) {
		pr_err("[%s:%s] fail to update reg\n", MUIC_DEV_NAME, __func__);
		return;
	}

	sm5038_muic_set_bc12(muic_data, BC12_ENABLE);
	return;
}
EXPORT_SYMBOL_GPL(sm5038_muic_shutdown);



//static struct platform_driver sm5038_muic_driver = {
//	.driver = {
//		.name = "sm5038-muic",
//		.owner	= THIS_MODULE,
//		.of_match_table = sm5038_muic_match_table,
//	},
//	.probe = sm5038_muic_probe,
//	.remove = sm5038_muic_remove,
//	.shutdown = sm5038_muic_shutdown,
//};

//static int __init sm5038_muic_init(void)
//{
//	return platform_driver_register(&sm5038_muic_driver);
//}
//module_init(sm5038_muic_init);

//static void __exit sm5038_muic_exit(void)
//{
//	platform_driver_unregister(&sm5038_muic_driver);
//}
//module_exit(sm5038_muic_exit);

//MODULE_DESCRIPTION("SM5038 MUIC driver");
MODULE_LICENSE("GPL");
