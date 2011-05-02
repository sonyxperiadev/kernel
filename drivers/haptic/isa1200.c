/*
 *  isa1200.c - Haptic Motor
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/haptic.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/ctype.h>
#include <linux/workqueue.h>
#include <linux/i2c/isa1200.h>
#include "haptic.h"

struct isa1200_chip {
	struct i2c_client *client;
	struct pwm_device *pwm;
	struct haptic_classdev cdev;
	struct work_struct work;
	struct timer_list timer;

	unsigned int	len;		/* LDO enable */
	unsigned int	hen;		/* Haptic haptic enable */

	int enable;
	int powered;

	int level;
	int level_max;

	int ldo_level;
};

static inline struct isa1200_chip *cdev_to_isa1200_chip(
		struct haptic_classdev *haptic_cdev)
{
	return container_of(haptic_cdev, struct isa1200_chip, cdev);
}

static int isa1200_chip_set_pwm_cycle(struct isa1200_chip *haptic)
{
	int duty = PWM_HAPTIC_PERIOD * haptic->level / 100;
	return pwm_config(haptic->pwm, duty, PWM_HAPTIC_PERIOD);
}

static int isa1200_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int isa1200_write_reg(struct i2c_client *client, int reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void isa1200_chip_power_on(struct isa1200_chip *haptic)
{
	if (haptic->powered)
		return;
	haptic->powered = 1;
	/* Use smart mode enable control */
	pwm_enable(haptic->pwm);
}

static void isa1200_chip_power_off(struct isa1200_chip *haptic)
{
	if (!haptic->powered)
		return;
	haptic->powered = 0;
	/* Use smart mode enable control */
	pwm_disable(haptic->pwm);
}

static void isa1200_chip_work(struct work_struct *work)
{
	struct isa1200_chip *haptic;
	int r;

	haptic = container_of(work, struct isa1200_chip, work);
	if (haptic->enable) {
		r = isa1200_chip_set_pwm_cycle(haptic);
		if (r) {
			dev_dbg(haptic->cdev.dev, "set_pwm_cycle failed\n");
			return;
		}
		isa1200_chip_power_on(haptic);
	} else {
		isa1200_chip_power_off(haptic);
	}
}

static void isa1200_chip_timer(unsigned long data)
{
	struct isa1200_chip *haptic = (struct isa1200_chip *)data;

	haptic->enable = 0;
	isa1200_chip_power_off(haptic);
}

static void isa1200_chip_set(struct haptic_classdev *haptic_cdev,
				enum haptic_value value)
{
	struct isa1200_chip *haptic =
		cdev_to_isa1200_chip(haptic_cdev);

	switch (value) {
	case HAPTIC_OFF:
		haptic->enable = 0;
		break;
	case HAPTIC_HALF:
	case HAPTIC_FULL:
	default:
		haptic->enable = 1;
		break;
	}

	schedule_work(&haptic->work);
}

static enum haptic_value isa1200_chip_get(struct haptic_classdev *haptic_cdev)
{
	struct isa1200_chip *haptic =
		cdev_to_isa1200_chip(haptic_cdev);

	if (haptic->enable)
		return HAPTIC_FULL;

	return HAPTIC_OFF;
}

#define ATTR_DEF_SHOW(name) \
static ssize_t isa1200_chip_show_##name(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev); \
	struct isa1200_chip *haptic = cdev_to_isa1200_chip(haptic_cdev); \
 \
	return sprintf(buf, "%u\n", haptic->name) + 1; \
}

#define ATTR_DEF_STORE(name) \
static ssize_t isa1200_chip_store_##name(struct device *dev, \
		struct device_attribute *attr, \
		const char *buf, size_t size) \
{ \
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev); \
	struct isa1200_chip *haptic = cdev_to_isa1200_chip(haptic_cdev); \
	ssize_t ret = -EINVAL; \
	unsigned long val; \
 \
	ret = strict_strtoul(buf, 10, &val); \
	if (ret == 0) { \
		ret = size; \
		haptic->name = val; \
		schedule_work(&haptic->work); \
	} \
 \
	return ret; \
}

ATTR_DEF_SHOW(enable);
ATTR_DEF_STORE(enable);
static DEVICE_ATTR(enable, 0644, isa1200_chip_show_enable,
		isa1200_chip_store_enable);

static ssize_t isa1200_chip_store_level(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev);
	struct isa1200_chip *haptic = cdev_to_isa1200_chip(haptic_cdev);
	ssize_t ret = -EINVAL;
	unsigned long val;

	ret = strict_strtoul(buf, 10, &val);
	if (ret == 0) {
		ret = size;
		if (haptic->level_max < val)
			val = haptic->level_max;
		haptic->level = val;
		schedule_work(&haptic->work);
	}

	return ret;
}
ATTR_DEF_SHOW(level);
static DEVICE_ATTR(level, 0644, isa1200_chip_show_level,
		isa1200_chip_store_level);

ATTR_DEF_SHOW(level_max);
static DEVICE_ATTR(level_max, 0444, isa1200_chip_show_level_max, NULL);

static ssize_t isa1200_chip_store_oneshot(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev);
	struct isa1200_chip *haptic = cdev_to_isa1200_chip(haptic_cdev);
	ssize_t ret = -EINVAL;
	unsigned long val;

	ret = strict_strtoul(buf, 10, &val);
	if (ret == 0) {
		ret = size;
		haptic->enable = 1;
		mod_timer(&haptic->timer, jiffies + val * HZ / 1000);
		schedule_work(&haptic->work);
	}

	return ret;
}
static DEVICE_ATTR(oneshot, 0200, NULL, isa1200_chip_store_oneshot);

static struct attribute *haptic_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_level.attr,
	&dev_attr_level_max.attr,
	&dev_attr_oneshot.attr,
	NULL,
};

static const struct attribute_group haptic_group = {
	.attrs = haptic_attributes,
};

static void isa1200_setup(struct i2c_client *client)
{
	struct isa1200_chip *chip = i2c_get_clientdata(client);
	int value;

	gpio_set_value(chip->len, 1);
	udelay(250);
	gpio_set_value(chip->len, 1);

	value = isa1200_read_reg(client, ISA1200_SCTRL0);
	value &= ~ISA1200_LDOADJ_MASK;
	value |= chip->ldo_level;
	isa1200_write_reg(client, ISA1200_SCTRL0, value);

	value = ISA1200_HAPDREN | ISA1200_OVERHL | ISA1200_HAPDIGMOD_PWM_IN |
		ISA1200_PWMMOD_DIVIDER_128;
	isa1200_write_reg(client, ISA1200_HCTRL0, value);

	value = ISA1200_EXTCLKSEL | ISA1200_BIT6_ON | ISA1200_MOTTYP_LRA |
		ISA1200_SMARTEN | ISA1200_SMARTOFFT_64;
	isa1200_write_reg(client, ISA1200_HCTRL1, value);

	value = isa1200_read_reg(client, ISA1200_HCTRL2);
	value |= ISA1200_SEEN;
	isa1200_write_reg(client, ISA1200_HCTRL2, value);
	isa1200_chip_power_off(chip);
	isa1200_chip_power_on(chip);
}

static int __devinit isa1200_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct isa1200_chip *chip;
	struct haptic_platform_data *pdata;
	int ret;

	pdata = client->dev.platform_data;
	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct isa1200_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	chip->cdev.set = isa1200_chip_set;
	chip->cdev.get = isa1200_chip_get;
	chip->cdev.show_enable = isa1200_chip_show_enable;
	chip->cdev.store_enable = isa1200_chip_store_enable;
	chip->cdev.store_oneshot = isa1200_chip_store_oneshot;
	chip->cdev.show_level = isa1200_chip_show_level;
	chip->cdev.store_level = isa1200_chip_store_level;
	chip->cdev.show_level_max = isa1200_chip_show_level_max;
	chip->cdev.name = pdata->name;
	chip->enable = 0;
	chip->level = PWM_HAPTIC_DEFAULT_LEVEL;
	chip->level_max = PWM_HAPTIC_DEFAULT_LEVEL;
	chip->ldo_level = pdata->ldo_level;

	if (pdata->setup_pin)
		pdata->setup_pin();
	chip->len = pdata->gpio;
	chip->hen = pdata->gpio;
	chip->pwm = pwm_request(pdata->pwm_timer, "haptic");
	if (IS_ERR(chip->pwm)) {
		dev_err(&client->dev, "unable to request PWM for haptic.\n");
		ret = PTR_ERR(chip->pwm);
		goto error_pwm;
	}

	INIT_WORK(&chip->work, isa1200_chip_work);

	/* register our new haptic device */
	ret = haptic_classdev_register(&client->dev, &chip->cdev);
	if (ret < 0) {
		dev_err(&client->dev, "haptic_classdev_register failed\n");
		goto error_classdev;
	}

	ret = sysfs_create_group(&chip->cdev.dev->kobj, &haptic_group);
	if (ret)
		goto error_enable;

	init_timer(&chip->timer);
	chip->timer.data = (unsigned long)chip;
	chip->timer.function = &isa1200_chip_timer;

	i2c_set_clientdata(client, chip);

	if (gpio_is_valid(pdata->gpio)) {
		ret = gpio_request(pdata->gpio, "haptic enable");
		if (ret)
			goto error_gpio;
		gpio_direction_output(pdata->gpio, 1);
	}

	isa1200_setup(client);

	printk(KERN_INFO "isa1200 %s registered\n", pdata->name);
	return 0;

error_gpio:
	gpio_free(pdata->gpio);
error_enable:
	sysfs_remove_group(&chip->cdev.dev->kobj, &haptic_group);
error_classdev:
	haptic_classdev_unregister(&chip->cdev);
error_pwm:
	pwm_free(chip->pwm);
	kfree(chip);
	return ret;
}

static int __devexit isa1200_remove(struct i2c_client *client)
{
	return 0;
}

static int isa1200_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct isa1200_chip *chip = i2c_get_clientdata(client);
	isa1200_chip_power_off(chip);
	return 0;
}

static int isa1200_resume(struct i2c_client *client)
{
	isa1200_setup(client);
	return 0;
}

static const struct i2c_device_id isa1200_id[] = {
	{ "isa1200", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, isa1200_id);

static struct i2c_driver isa1200_driver = {
	.driver	= {
		.name	= "isa1200",
	},
	.probe		= isa1200_probe,
	.remove		= __devexit_p(isa1200_remove),
	.suspend	= isa1200_suspend,
	.resume		= isa1200_resume,
	.id_table	= isa1200_id,
};

static int __init isa1200_init(void)
{
	return i2c_add_driver(&isa1200_driver);
}

static void __exit isa1200_exit(void)
{
	i2c_del_driver(&isa1200_driver);
}

module_init(isa1200_init);
module_exit(isa1200_exit);

MODULE_AUTHOR("Kyungmin Park <kyungmin.park@samsung.com>");
MODULE_DESCRIPTION("ISA1200 Haptic Motor driver");
MODULE_LICENSE("GPL");
