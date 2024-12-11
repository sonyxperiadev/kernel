/*
 * Copyright 2021 Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
// SPDX-License-Identifier: GPL-2.0-only
/*
 * sm5038-fled.c - SM5038 Flash-LEDs device driver
 *
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/power/sm5038.h>

#include <linux/power/sm5038_charger.h>


#define SM5038_FLED_VERSION		"UF1"
static struct sm5038_fled_data *g_sm5038_fled;
static struct class *camera_class; /*leds/class/ */

typedef enum {
	PM6125_FLASH_GPIO_STATE_ACTIVE,
	PM6125_FLASH_GPIO_STATE_SUSPEND,
	PM6125_FLASH_GPIO_STATE_MAX,	/* for array size */
} PM6125_FLASH_GPIO_STATE;

extern void sm5038_request_default_power_src(void);
extern int sm5038_muic_get_vbus_voltage(void);
extern void pm6125_flash_gpio_select_state(PM6125_FLASH_GPIO_STATE s);

extern int sm5038_muic_check_fled_state(int enable, int mode);   /* FLEDEN_TORCH_ON  = 0x1, FLEDEN_FLASH_ON  = 0x2 */
extern int sm5038_usbpd_check_fled_state(bool enable, u8 mode);

extern int sm5038_charger_oper_push_event(int event_type, bool enable);

struct sm5038_fled_platform_data {
	const char *name;
	u8 flash_brightness;
	u8 torch_brightness;
	u8 preflash_brightness;
	u8 timeout;

	int sysfs_input_data;

	bool en_preset_fled;	/* prepare_flash function */

	bool activated_fled;	/* flash used */
	bool activated_mled;	/* torch used */

	bool used_gpio_ctrl;

	
};

struct sm5038_fled_data {
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex fled_mutex;

	struct sm5038_fled_platform_data *pdata;

	struct device *rear_fled_dev;

	int input_voltage;
};


static void fled_set_mode(struct sm5038_fled_data *fled, u8 mode)
{
	sm5038_update_reg(fled->i2c, SM5038_CHG_REG_FLEDCNTL1, (mode << 0), (0x3 << 0));
}

static void fled_set_flash_current(struct sm5038_fled_data *fled, u8 offset)
{
	sm5038_update_reg(fled->i2c, SM5038_CHG_REG_FLEDCNTL2, (offset << 0), (0xf << 0));
}

static void fled_set_torch_current(struct sm5038_fled_data *fled, u8 offset)
{
	sm5038_update_reg(fled->i2c, SM5038_CHG_REG_FLEDCNTL2, (offset << 4), (0x7 << 4));
}

static int sm5038_fled_check_vbus(struct sm5038_fled_data *fled)
{
	fled->input_voltage = sm5038_muic_get_vbus_voltage();
	if (fled->input_voltage > 5500) {
		sm5038_request_default_power_src();
	}
	return 0;
}

static int sm5038_fled_control(u8 fled_mode)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038-fled: %s: not probe fled yet\n", __func__);
		return -ENXIO;
	}

	if (fled_mode == FLEDEN_FLASH_ON) {
        fled_set_mode(fled,  FLEDEN_EXTERNAL);
	    fled->pdata->used_gpio_ctrl = true;
		pr_info("sm5038-fled: %s: FLASH_MODE ON (%s) \n", __func__, fled->pdata->used_gpio_ctrl ? "GPIO_Control" : "I2C_Control");
		
	} else if (fled_mode == FLEDEN_TORCH_ON) {
        fled_set_mode(fled,  FLEDEN_EXTERNAL);
		fled->pdata->used_gpio_ctrl = true;
		pr_info("sm5038-fled: %s: TORCH_MODE ON (%s) \n", __func__, fled->pdata->used_gpio_ctrl ? "GPIO_Control" : "I2C_Control");

	} else if (fled_mode == FLEDEN_DISABLE) {
		pr_info("sm5038-fled: %s: i2c_control OFF_MODE & used gpio(%)\n", __func__, fled->pdata->used_gpio_ctrl);
		fled_set_mode(fled, FLEDEN_DISABLE);

		fled->pdata->used_gpio_ctrl = false;

		/* torch off */
		if (fled->pdata->activated_mled == true) {
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_TORCH, 0);
			/* 5V -> 9V */
			sm5038_fled_check_vbus(fled);
			sm5038_muic_check_fled_state(0, FLEDEN_TORCH_ON);
			sm5038_usbpd_check_fled_state(0, FLEDEN_TORCH_ON);

			fled->pdata->activated_mled = false;
		}

		/* flash off */
		if (fled->pdata->activated_fled == true) {
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_FLASH, 0);
			/* flash case, only vbus control, in prepare_flash & close_flash function  */
			fled->pdata->activated_fled = false;
		}
	} else {
		pr_err("sm5038-fled: %s: invalid mode = %d, used gpio(%)\n", __func__, fled_mode, fled->pdata->used_gpio_ctrl);
		return -EINVAL;
	}

	return 0;
}

static int sm5038_fled_torch_on(u8 brightness)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	pr_info("sm5038-fled: %s: start.\n", __func__);

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038-fled: %s: not probe fled yet\n", __func__);
		return -ENXIO;
	}

	mutex_lock(&fled->fled_mutex);

	fled_set_torch_current(fled, brightness);

	if (fled->pdata->activated_mled == false) {
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_TORCH, 1);
		fled->pdata->activated_mled = true;
	}

	sm5038_fled_control(FLEDEN_TORCH_ON);

	mutex_unlock(&fled->fled_mutex);

	pr_info("sm5038-fled: %s: done.\n", __func__);

	return 0;
}

static int sm5038_fled_flash_on(u8 brightness)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;
	pr_info("sm5038-fled: %s: start.\n", __func__);

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038-fled: %s: not probe fled yet\n", __func__);
		return -ENXIO;
	}

	mutex_lock(&fled->fled_mutex);

	fled_set_flash_current(fled, brightness);

	if (fled->pdata->activated_fled == false) {
		sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_FLASH, 1);
		fled->pdata->activated_fled = true;
	}
	sm5038_fled_control(FLEDEN_FLASH_ON);

	mutex_unlock(&fled->fled_mutex);

	pr_info("sm5038-fled: %s: done.\n", __func__);

	return 0;
}

static int sm5038_fled_prepare_flash(void)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	pr_info("sm5038-fled: %s: start.\n", __func__);

	if (fled == NULL) {
		pr_err("sm5038-fled: %s: not probe fled yet\n", __func__);
		return -ENXIO;
	}


	if (fled->pdata->en_preset_fled == true) {
		pr_info("sm5038-fled: %s: already prepared\n", __func__);
		return 0;
	}

	mutex_lock(&fled->fled_mutex);


	if (fled->pdata->en_preset_fled == false) {
		sm5038_fled_check_vbus(fled);
		sm5038_muic_check_fled_state(1, FLEDEN_FLASH_ON);
		sm5038_usbpd_check_fled_state(1, FLEDEN_FLASH_ON);
	}
	fled_set_flash_current(fled, fled->pdata->flash_brightness);
	fled_set_torch_current(fled, fled->pdata->torch_brightness);

	fled->pdata->en_preset_fled = true;

	mutex_unlock(&fled->fled_mutex);

	pr_info("sm5038-fled: %s: done.\n", __func__);

	return 0;
}

static int sm5038_fled_close_flash(void)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	pr_info("sm5038-fled: %s: start.\n", __func__);

	if (fled == NULL) {
		pr_err("sm5038-fled: %s: not probe fled yet\n", __func__);
		return -ENXIO;
	}

	if (fled->pdata->en_preset_fled == false) {
		pr_info("sm5038-fled: %s: already closed\n", __func__);
		return 0;
	}

	mutex_lock(&fled->fled_mutex);

	fled_set_mode(fled, FLEDEN_DISABLE);

	sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_TORCH, 0);
	sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_FLASH, 0);

	sm5038_fled_check_vbus(fled);
	sm5038_muic_check_fled_state(0, FLEDEN_FLASH_ON);
	sm5038_usbpd_check_fled_state(0, FLEDEN_FLASH_ON);

	fled->pdata->en_preset_fled = false;

	mutex_unlock(&fled->fled_mutex);

	pr_info("sm5038-fled: %s: done.\n", __func__);

	return 0;
}

/**
 * For export Camera flash control support
 *
 */

int32_t sm5038_fled_mode_ctrl(int state, uint32_t brightness)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;
	int ret = 0;
	u8 iq_cur = 0;

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038_fled: %s: g_sm5038_fled is not initialized.\n", __func__);
		return -EFAULT;
	}

	/* torch brigthness : 50mA ~ 225mA, 25mA step */
	if (brightness > 0 && (state == SM5038_FLED_MODE_TORCH_FLASH || state == SM5038_FLED_MODE_PRE_FLASH)) {
		if (brightness < 50)
			iq_cur = 0x0;
		else if (brightness > 225)
			iq_cur = 0x7;
		else
			iq_cur = (brightness - 50) / 25;
	}
	/* flash brigthness : 300mA,700mA, 800mA 900mA ~ 1500mA, 50mA step */
	else if (brightness > 0 && state == SM5038_FLED_MODE_MAIN_FLASH) {
		if (brightness < 700)
			iq_cur = 0x0;
		else if (brightness < 800)
			iq_cur = 0x1;
		else if (brightness < 900)
			iq_cur = 0x2;
		else
			iq_cur = 3 + (brightness - 900) / 50;
	}

	pr_info("sm5038-fled: %s: iq_cur=0x%x brightness=%u\n", __func__, iq_cur, brightness);

	switch (state) {

	case SM5038_FLED_MODE_OFF:
		/* FlashLight Mode OFF */
		ret = sm5038_fled_control(FLEDEN_DISABLE);
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_OFF(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_OFF(%d) done\n", __func__, state);
		break;

	case SM5038_FLED_MODE_MAIN_FLASH:
		/* FlashLight Mode Flash */
		if (brightness > 0)
			ret = sm5038_fled_flash_on(iq_cur);
		else
			ret = sm5038_fled_flash_on(fled->pdata->flash_brightness);
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_MAIN_FLASH(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_MAIN_FLASH(%d) done\n", __func__, state);
		break;

	case SM5038_FLED_MODE_TORCH_FLASH: /* TORCH FLASH */
		/* FlashLight Mode TORCH */
		if (brightness > 0)
			ret = sm5038_fled_torch_on(iq_cur);
		else
			ret = sm5038_fled_torch_on(fled->pdata->torch_brightness);
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_TORCH_FLASH(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_TORCH_FLASH(%d) done\n", __func__, state);
		break;

	case SM5038_FLED_MODE_PRE_FLASH: /* TORCH FLASH */
		/* FlashLight Mode TORCH */
		if (brightness > 0)
			ret = sm5038_fled_torch_on(iq_cur);
		else
			ret = sm5038_fled_torch_on(fled->pdata->preflash_brightness);
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_PRE_FLASH(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_PRE_FLASH(%d) done\n", __func__, state);
		break;

	case SM5038_FLED_MODE_PREPARE_FLASH:
		/* 9V -> 5V VBUS change */
		ret = sm5038_fled_prepare_flash();
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_PREPARE_FLASH(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_PREPARE_FLASH(%d) done\n", __func__, state);
		break;

	case SM5038_FLED_MODE_CLOSE_FLASH:
		/* 5V -> 9V VBUS change */
		ret = sm5038_fled_close_flash();
		if (ret < 0)
			pr_err("sm5038-fled: %s: SM5038_FLED_MODE_CLOSE_FLASH(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: SM5038_FLED_MODE_CLOSE_FLASH(%d) done\n", __func__, state);
		break;

	default:
		/* FlashLight Mode OFF */
		ret = sm5038_fled_control(FLEDEN_DISABLE);
		if (ret < 0)
			pr_err("sm5038-fled: %s: FLED_MODE_OFF(%d) failed\n", __func__, state);
		else
			pr_info("sm5038-fled: %s: FLED_MODE_OFF(%d) done\n", __func__, state);
		break;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(sm5038_fled_mode_ctrl);

/**
 *  For leds device file control (Torch-LED)
 */

static ssize_t sm5038_rear_flash_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 store_value;
	int ret;
	struct sm5038_fled_data *fled = g_sm5038_fled;

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038-fled: %s: g_sm5038_fled NULL \n", __func__);
		return -ENODEV;
	}

	if ((buf == NULL) || kstrtouint(buf, 10, &store_value)) {
		return -ENXIO;
	}

	fled->pdata->sysfs_input_data = store_value;

	pr_info("sm5038-fled: %s: value=%d\n", __func__, store_value);

	mutex_lock(&fled->fled_mutex);

	if (store_value == 0) { /* 0: Torch or Flash OFF */
		if (fled->pdata->activated_mled == false &&
				fled->pdata->activated_fled == false) {
			goto out_skip;
		}
		sm5038_fled_control(FLEDEN_DISABLE);

	} else if (store_value == 200) { /* 200 : Flash ON */
		fled_set_flash_current(fled, fled->pdata->torch_brightness);

		if (fled->pdata->activated_fled == true) {
			goto out_skip;
		}
		if (fled->pdata->activated_fled == false) {
			sm5038_fled_check_vbus(fled);
			sm5038_muic_check_fled_state(1, FLEDEN_FLASH_ON);
			sm5038_usbpd_check_fled_state(1, FLEDEN_FLASH_ON);
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_FLASH, 1);
		}
		sm5038_fled_control(FLEDEN_FLASH_ON);

		fled->pdata->activated_fled = true;

	} else { /* 1, 100, 1001~1010, : Torch ON */
		/* Main Torch on */
		if (store_value == 1) {
			fled_set_torch_current(fled, 0x1); /* Set mled = 75mA(0x1) */
		/* Factory Torch on */
		} else if (store_value == 100) {
			fled_set_torch_current(fled, 0x7);    /* Set mled=225mA */
		} else if (store_value >= 1001 && store_value <= 1010) {
			/* Torch on (Normal) */
			if (store_value-1001 > 7)
				fled_set_torch_current(fled, 0x07); /* Max 225mA(0x7)  */
			else
				fled_set_torch_current(fled, (store_value-1001)); /* 50mA(0x0) ~ 225mA(0x7) at 25mA step */
		} else {
			pr_err("sm5038-fled: %s: failed store cmd\n", __func__);
			ret = -EINVAL;
			goto out_p;
		}
		pr_info("sm5038-fled: %s: en_mled=%d \n", __func__, fled->pdata->activated_mled);

		if (fled->pdata->activated_mled == true) {
			goto out_skip;
		}

		if (fled->pdata->activated_mled == false) {
			sm5038_fled_check_vbus(fled);
			sm5038_muic_check_fled_state(1, FLEDEN_TORCH_ON);
			sm5038_usbpd_check_fled_state(1, FLEDEN_TORCH_ON);
			sm5038_charger_oper_push_event(SM5038_CHARGER_OP_EVENT_TORCH, 1);
		}
		sm5038_fled_control(FLEDEN_TORCH_ON);

		fled->pdata->activated_mled = true;
	}

out_skip:
	ret = count;

out_p:
	mutex_unlock(&fled->fled_mutex);

	return count;
}

static ssize_t sm5038_rear_flash_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	if (g_sm5038_fled == NULL) {
		pr_err("sm5038-fled: %s: g_sm5038_fled is NULL\n", __func__);
		return -ENODEV;
	}

	return sprintf(buf, "%d\n", fled->pdata->sysfs_input_data);
}

static DEVICE_ATTR(rear_flash, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, sm5038_rear_flash_show, sm5038_rear_flash_store);

static int sm5038_fled_parse_dt(struct device *dev, struct sm5038_fled_platform_data *pdata)
{
	struct device_node *np;
	u32 temp;
	pr_info("sm5038-fled: %s: parse from dtsi file\n", __func__);

	np = of_find_node_by_name(NULL, "sm5038-fled");
	if (!np) {
		pr_err("sm5038-fled: %s: can't find sm5038-fled np\n", __func__);
		return -EINVAL;
	}

	of_property_read_u32(np, "flash-brightness", &temp);
	pdata->flash_brightness = (temp & 0xff);
	of_property_read_u32(np, "preflash-brightness", &temp);
	pdata->preflash_brightness = (temp & 0xff);
	of_property_read_u32(np, "torch-brightness", &temp);
	pdata->torch_brightness = (temp & 0xff);
	of_property_read_u32(np, "timeout", &temp);
	pdata->timeout = (temp & 0xff);

	pr_info("sm5038-fled: %s: f_cur=0x%x, pre_f_cur=0x%x, cur=0x%x, tout=%d\n", __func__,
		pdata->flash_brightness, pdata->preflash_brightness,
		pdata->torch_brightness, pdata->timeout);
	pr_info("sm5038-fled: %s: parse dt done.\n", __func__);

	return 0;
}

static void sm5038_fled_init(struct sm5038_fled_data *fled)
{
	fled_set_mode(fled, FLEDEN_DISABLE);
	fled->pdata->activated_mled = false;
	fled->pdata->activated_fled = false;
	fled->pdata->en_preset_fled = false;

	fled->pdata->used_gpio_ctrl = false;

	mutex_init(&fled->fled_mutex);
	pr_info("sm5038-fled: %s: flash init done\n", __func__);
}

int sm5038_fled_probe(struct sm5038_dev *sm5038)
{
	//struct sm5038_dev *sm5038 = dev_get_drvdata(pdev->dev.parent);
	struct sm5038_fled_data *fled;
	int ret = 0;

	pr_info("sm5038-fled: sm5038 fled probe start (rev=%d)\n", sm5038->pmic_rev);

	fled = kzalloc(sizeof(struct sm5038_fled_data), GFP_KERNEL);
	if (unlikely(!fled)) {
		pr_err("sm5038-fled: %s: fail to alloc_devm\n", __func__);
		return -ENOMEM;
	}
	fled->dev = sm5038->dev;
	fled->i2c = sm5038->charger_i2c;

	fled->pdata = kzalloc(sizeof(struct sm5038_fled_platform_data), GFP_KERNEL);
	if (unlikely(!fled->pdata)) {
		pr_err("sm5038-fled: %s: fail to alloc_pdata\n", __func__);
		ret = -ENOMEM;
		goto free_dev;
	}
	ret = sm5038_fled_parse_dt(fled->rear_fled_dev, fled->pdata);
	if (ret < 0) {
		goto free_pdata;
	}

	if (fled->i2c == NULL) {
		pr_err("sm5038-fled: %s: i2c NULL\n", __func__);
		goto free_pdata;
	}

	sm5038_fled_init(fled);
	g_sm5038_fled = fled;

	camera_class = class_create(THIS_MODULE, "sm5038_fled");
	if (IS_ERR_OR_NULL(camera_class)) {
		pr_err("couldn't create class");
		return PTR_ERR(camera_class);
	}

	fled->rear_fled_dev = device_create(camera_class, NULL, 0, NULL, "flash");
	if (IS_ERR(fled->rear_fled_dev)) {
		pr_err("sm5038-fled: %s failed create device for rear_flash\n", __func__);
		goto free_pdata;
	}

	fled->rear_fled_dev->parent = fled->dev;

	ret = device_create_file(fled->rear_fled_dev, &dev_attr_rear_flash);
	if (IS_ERR_VALUE((unsigned long)ret)) {
		pr_err("sm5038-fled: %s failed create device file for rear_flash\n", __func__);
		goto free_device;
	}

	pr_info("sm5038-fled: sm5038 fled probe done[%s].\n", SM5038_FLED_VERSION);

	return 0;

free_device:
	device_remove_file(fled->rear_fled_dev, &dev_attr_rear_flash);
free_pdata:
	kfree(fled->pdata);
free_dev:
	kfree(fled);

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_fled_probe);

int sm5038_fled_remove(void)
{
	struct sm5038_fled_data *fled = g_sm5038_fled;

	device_remove_file(fled->rear_fled_dev, &dev_attr_rear_flash);

	fled_set_mode(fled, FLEDEN_DISABLE);

	//platform_set_drvdata(pdev, NULL);

	//devm_kfree(&pdev->dev, fled->pdata);
	//devm_kfree(&pdev->dev, fled);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_fled_remove);

//static struct of_device_id sm5038_fled_match_table[] = {
//	{ .compatible = "siliconmitus,sm5038-fled",},
//	{},
//};

//static const struct platform_device_id sm5038_fled_id[] = {
//	{"sm5038-fled", 0},
//	{},
//};

//static struct platform_driver sm5038_led_driver = {
//	.driver = {
//		.name  = "sm5038-fled",
//		.owner = THIS_MODULE,
//		.of_match_table = sm5038_fled_match_table,
//		},
//	.probe  = sm5038_fled_probe,
//	.remove = sm5038_fled_remove,
//	.id_table = sm5038_fled_id,
//};

//static int __init sm5038_led_driver_init(void)
//{
//	return platform_driver_register(&sm5038_led_driver);
//}
//late_initcall(sm5038_led_driver_init);
//
//static void __exit sm5038_led_driver_exit(void)
//{
//	platform_driver_unregister(&sm5038_led_driver);
//}
//module_exit(sm5038_led_driver_exit);

//MODULE_LICENSE("GPL");
//MODULE_DESCRIPTION("Flash-LED device driver for SM5038");
//MODULE_VERSION(SM5038_FLED_VERSION);

MODULE_LICENSE("GPL");
