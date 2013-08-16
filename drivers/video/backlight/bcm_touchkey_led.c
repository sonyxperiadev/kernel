/*
 * linux/drivers/video/backlight/bcm_touchkey_led.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/broadcom/lcd.h>
/*#include <linux/device.h>*/
#include <linux/regulator/consumer.h>
#include <mach/gpio.h>

#define TOUCHKEYLED_DEBUG 1

#if TOUCHKEYLED_DEBUG
#define KLDBG(fmt, args...) printk(fmt, ## args)
#else
#define KLDBG(fmt, args...)
#endif

struct touchkeypad_led_data {
	struct platform_device *pdev;
	unsigned int ctrl_pin;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};

static int current_intensity;
#define TOUCHKEY_LED_MAX	100
#define TOUCHKEY_LED_MIN	0

#define TOUCHKEYLED_ON   1
#define TOUCHKEYLED_OFF 0

static struct regulator *touchkeyled_regulator;

void touchkeyled_power_ctrl(unsigned char on_off)
{
	int ret;
	if (touchkeyled_regulator == NULL) {
		KLDBG(" %s, %d\n", __func__, __LINE__);
		touchkeyled_regulator = regulator_get(NULL, "gpldo3_uc");

		if (IS_ERR(touchkeyled_regulator))
			printk("[KEYLED] can not get VKEYLED_3.3V\n");
	}
	if (on_off == TOUCHKEYLED_ON) {
		KLDBG("[KEYLED] %s, %d Keyled On\n", __func__, __LINE__);

		ret = regulator_set_voltage(
			touchkeyled_regulator, 3300000, 3300000);
		if (ret < 0)
			printk(KERN_INFO
				"[KEYLED] regulator_set_voltage : %d\n", ret);
			/*ret = regulator_enable(touchkeyled_regulator);*/
			ret = regulator_enable(touchkeyled_regulator);
			if (ret < 0)
				printk(KERN_INFO
					"[KEYLED] regulator_enable : %d\n",
					ret
				);
		} else {
			KLDBG(
				"[KEYLED] %s, %d Keyled Off\n",
				__func__, __LINE__
			);
			ret = regulator_disable(touchkeyled_regulator);
			/*ret = regulator_force_disable
			(touchkeyled_regulator);*/
			if (ret < 0)
				printk(KERN_INFO
					"[KEYLED] regulator_disable : %d\n",
					ret
				);
	}

}

/* input: intensity in percentage 0% - 100% */
int touchkeyled_set_intensity(struct backlight_device *bd)
{
	int user_intensity = bd->props.brightness;

	KLDBG(
		"[KEYLED] keyled_set_intensity = %d current_intensity = %d\n",
		user_intensity, current_intensity
	);

	if (user_intensity != 0 && current_intensity == 0)
		touchkeyled_power_ctrl(TOUCHKEYLED_ON);
	else if (user_intensity == 0 && current_intensity != 0)
		touchkeyled_power_ctrl(TOUCHKEYLED_OFF);

	current_intensity = user_intensity;

	return 0;
}


static int touchkeyled_set_brightness(struct backlight_device *bd)
{

	KLDBG("[KEYLED] %s, brightness=%d\n", __func__, bd->props.brightness);

	touchkeyled_set_intensity(bd);

	return 0;
}


static int touchkeyled_get_brightness(struct backlight_device *bl)
{
	KLDBG("[KEYLED] %s\n", __func__);

	return current_intensity;
}

static struct backlight_ops bcm_touchkey_led_ops = {
	.update_status	= touchkeyled_set_brightness,
	.get_brightness	= touchkeyled_get_brightness,
};


static int touchkeyled_suspend(struct platform_device *pdev,
					pm_message_t state)
{

	KLDBG("[KEYLED] %s, %d\n", __func__, __LINE__);

	return 0;
}

static int touchkeyled_resume(struct platform_device *pdev)
{

	KLDBG("[KEYLED] %s, %d\n", __func__, __LINE__);

	return 0;
}


static int touchkeyled_probe(struct platform_device *pdev)
{
	struct backlight_device *bl;
	struct touchkeypad_led_data *touchkeyled;
	struct backlight_properties props;
	int ret = 0;

	printk(KERN_INFO "[KEYLED] %s, %d\n", __func__, __LINE__);


	touchkeyled = kzalloc(sizeof(*touchkeyled), GFP_KERNEL);
	if (!touchkeyled) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_bl;
	}

	memset(&props, 0, sizeof(struct backlight_properties));

	props.brightness = TOUCHKEY_LED_MAX;
	props.max_brightness = TOUCHKEY_LED_MAX;
	props.type = BACKLIGHT_PLATFORM;

	bl = backlight_device_register(pdev->name, &pdev->dev,
			NULL, &bcm_touchkey_led_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		goto err_bl;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	touchkeyled->pdev = pdev;
	touchkeyled->early_suspend_desc.level =
		EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	touchkeyled->early_suspend_desc.suspend = touchkeyled_suspend;
	touchkeyled->early_suspend_desc.resume = touchkeyled_resume;
	register_early_suspend(&touchkeyled->early_suspend_desc);
#endif

	/*touchkeyled_power_ctrl(TOUCHKEYLED_ON);*/
	platform_set_drvdata(pdev, bl);

	bl->props.power = FB_BLANK_UNBLANK;
	bl->props.brightness = TOUCHKEY_LED_MAX;
	bl->props.max_brightness = TOUCHKEY_LED_MAX;

	KLDBG("[KEYLED] Probe done!");
	return 0;
err_bl:
	kfree(touchkeyled);
	return ret;

}

static int touchkeyled_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct touchkeypad_led_data *touchkey = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);


#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&touchkey->early_suspend_desc);
#endif

	kfree(touchkey);
	return 0;
}

static int touchkeyled_shutdown(struct platform_device *pdev)
{
	KLDBG("[KEYLED] touchkeyled_shutdown\n");
	touchkeyled_power_ctrl(TOUCHKEYLED_OFF);

	return 0;
}

static struct platform_driver touchkeyled_driver = {
	.driver		= {
		.name	= "touchkey-led",
		.owner	= THIS_MODULE,
	},
	.probe		= touchkeyled_probe,
	.remove		= touchkeyled_remove,
	.shutdown      = touchkeyled_shutdown,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend        = touchkeyled_suspend,
	.resume         = touchkeyled_resume,
#endif

};

static int __init touchkeyled_init(void)
{
	KLDBG("[KEYLED] touchkeyled_init\n");

	return platform_driver_register(&touchkeyled_driver);
}
module_init(touchkeyled_init);

static void __exit touchkeyled_exit(void)
{
	platform_driver_unregister(&touchkeyled_driver);
}
module_exit(touchkeyled_exit);

MODULE_DESCRIPTION("Touchkey LED Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:keypad-led");


