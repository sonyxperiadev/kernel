/*
 * Copyright 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
#include <linux/slab.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>


struct bcmpmu59xxx_led_param {
	struct bcmpmu59xxx *bcmpmu;
	struct led_classdev led_dev;
	struct mutex lock;
};


static uint8_t delay_on_to_reg(unsigned long *delay_on)
{
	if (*delay_on <= 100) {
		*delay_on = 100;
		return 0;
	} else if (*delay_on <= 200) {
		*delay_on = 200;
		return 1;
	} else if (*delay_on <= 500) {
		*delay_on = 500;
		return 2;
	} else if (*delay_on <= 1000) {
		*delay_on = 1000;
		return 3;
	} else {
		*delay_on = 2000;
		return 4;
	}
}


static uint8_t delay_off_to_reg(unsigned long *delay_off)
{
	if (*delay_off <= 500) {
		*delay_off = 500;
		return 0 << 3;
	} else if (*delay_off <= 1000) {
		*delay_off = 1000;
		return 1 << 3;
	} else if (*delay_off <= 2000) {
		*delay_off = 2000;
		return 2 << 3;
	} else if (*delay_off <= 5000) {
		*delay_off = 5000;
		return 3 << 3;
	} else if (*delay_off <= 10000) {
		*delay_off = 10000;
		return 4 << 3;
	} else {
		*delay_off = 20000;
		return 5 << 3;
	}
}


static void bcmpmu_led_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness brightness)
{
	struct bcmpmu59xxx_led_param *leddata =
		container_of(led_cdev, struct bcmpmu59xxx_led_param, led_dev);
	struct bcmpmu59xxx *bcmpmu = leddata->bcmpmu;

	mutex_lock(&leddata->lock);

	if (brightness) {	/* turning on led and set brightness */
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL1, 0x0);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL4, brightness >> 1);
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL6, brightness >> 1);
	} else {		/* turning off led */
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL1, 0x2);
	}

	mutex_unlock(&leddata->lock);
}


static int bcmpmu_led_blink_set(struct led_classdev *led_cdev,
		unsigned long *delay_on, unsigned long *delay_off)
{
	struct bcmpmu59xxx_led_param *leddata =
		container_of(led_cdev, struct bcmpmu59xxx_led_param, led_dev);
	struct bcmpmu59xxx *bcmpmu = leddata->bcmpmu;

	mutex_lock(&leddata->lock);

	if (0 == *delay_on)		/* turning off led */
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL1, 0x2);
	else if (0 == *delay_off)	/* constant brightness */
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL3, 0x7);
	else				/* blink */
		bcmpmu->write_dev(bcmpmu, PMU_REG_PWMLEDCTRL3,
				delay_on_to_reg(delay_on) |
				delay_off_to_reg(delay_off));

	mutex_unlock(&leddata->lock);

	return 0;
}


static int bcmpmu_led_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_led_pdata *pdata;
	struct bcmpmu59xxx_led_param *leddata;

	pdata = (struct bcmpmu59xxx_led_pdata *)pdev->dev.platform_data;

	leddata = kzalloc(sizeof(struct bcmpmu59xxx_led_param), GFP_KERNEL);
	if (NULL == leddata) {
		pr_err("failed to alloc mem\n");
		return -ENOMEM;
	}

	mutex_init(&leddata->lock);
	leddata->bcmpmu = bcmpmu;
	leddata->led_dev.name = pdata->led_name;
	leddata->led_dev.brightness = LED_OFF;
	leddata->led_dev.max_brightness = LED_FULL;
	leddata->led_dev.default_trigger = "timer";
	leddata->led_dev.brightness_set = bcmpmu_led_brightness_set;
	leddata->led_dev.blink_set = bcmpmu_led_blink_set;

	platform_set_drvdata(pdev, leddata);

	ret = led_classdev_register(&pdev->dev, &leddata->led_dev);
	if (ret) {
		kfree(leddata);
		pr_err("failed to register led class!\n");
		return -ENODEV;
	}

	return ret;
}


static int bcmpmu_led_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx_led_param *leddata;

	leddata = platform_get_drvdata(pdev);

	if (NULL != leddata) {
		led_classdev_unregister(&leddata->led_dev);
		kfree(leddata);
	}

	return 0;
}


static struct platform_driver bcmpmu_led_driver = {
	.driver = {
		.name = "bcmpmu59xxx-led",
	},
	.probe = bcmpmu_led_probe,
	.remove = bcmpmu_led_remove,
};


static int __init bcmpmu_led_init(void)
{
	return platform_driver_register(&bcmpmu_led_driver);
}
module_init(bcmpmu_led_init);


static void __exit bcmpmu_led_exit(void)
{
	platform_driver_unregister(&bcmpmu_led_driver);
}
module_exit(bcmpmu_led_exit);


MODULE_DESCRIPTION("BCM PMIC59xxx PWM_LED driver");
MODULE_LICENSE("GPL");

