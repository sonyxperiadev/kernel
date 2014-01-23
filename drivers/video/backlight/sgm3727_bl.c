/*
 * linux/drivers/video/backlight/pwm_bl.c
 *
 * simple PWM based backlight control, board code has to setup
 * 1) pin configuration so PWM waveforms can output
 * 2) platform_data being correctly configured
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/sgm3727_backlight.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spinlock_types.h>
#ifdef CONFIG_THERMAL
#include <linux/thermal.h>
#endif

#define IC_MAX_BR 32
struct sgm_bl_data {
	struct device		*dev;
	unsigned int bl_gpio;
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int		*levels;
	unsigned int shutdown_time;
	unsigned int ready_time;
	unsigned int interval_time;
	unsigned int last_brightness;
	int			(*notify)(struct device *,
					  int brightness);
	void			(*notify_after)(struct device *,
					int brightness);
	int			(*check_fb)(struct device *, struct fb_info *);
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend bd_early_suspend;
#endif
	struct delayed_work bl_delay_on_work;
	struct spinlock sgm_spin_lock;
};

/* Thermal cooling device helper functions */
enum {
	GET_COOLING_LEVEL,
	GET_BACKLIGHT_LEVEL,
};
static int bl_get_property(struct thermal_cooling_device *cdev,
		int in, int *out, int property)
{
	struct backlight_device *bl = cdev->devdata;
	struct sgm_bl_data *pb = bl_get_data(bl);
	int idx, ret = 0, ascending = 0;

	if (pb->levels[0] < pb->levels[1])
		ascending = 1;

	switch (property) {
	case GET_COOLING_LEVEL:
		for (idx = 0; idx <= bl->props.max_brightness; idx++) {
			if (pb->levels[idx] == in) {
				*out = ascending ?
					(bl->props.max_brightness - idx) : idx;
				break;
			}
		}
		break;
	case GET_BACKLIGHT_LEVEL:
		*out = ascending ? (bl->props.max_brightness - in) : in;
		break;
	default:
		dev_err(pb->dev, "invalid backlight cooling property\n");
		ret = -EINVAL;
		break;
	}

	return ret;
}

u32 backlight_cooling_get_level(struct thermal_cooling_device *cdev,
			u32 brightness)
{
	u32 level;

	if (bl_get_property(cdev, brightness, &level, GET_COOLING_LEVEL))
		return THERMAL_CSTATE_INVALID;

	return level;
}
EXPORT_SYMBOL_GPL(backlight_cooling_get_level);
static int sgm3727_backlight_update_status(struct backlight_device *bl)
{
	struct sgm_bl_data *pb = dev_get_drvdata(&bl->dev);
	int brightness = bl->props.brightness;
	int max = bl->props.max_brightness;
	unsigned long flags = 0;

	if (bl->props.power != FB_BLANK_UNBLANK)
		brightness = 0;

	if (bl->props.fb_blank != FB_BLANK_UNBLANK)
		brightness = 0;

	if (pb->notify)
		brightness = pb->notify(pb->dev, brightness);

	if (brightness > max)
		brightness = max;

	pr_debug("%s, brightness=%d, last_brightness=%d\n",
		__func__, brightness, pb->last_brightness);

	if (brightness == 0) {
		gpio_direction_output(pb->bl_gpio, 0);
		gpio_set_value(pb->bl_gpio, 0);
		udelay(pb->shutdown_time);
	} else {
		int loop = 0;
		int i = 0;

		if (brightness <= pb->last_brightness)
			loop = pb->last_brightness - brightness;
		else
			loop = IC_MAX_BR - (brightness - pb->last_brightness);

		pr_debug("%s, loop=%d\n", __func__, loop);

		spin_lock_irqsave(&pb->sgm_spin_lock, flags);
		if ((pb->last_brightness == 0) && (brightness != 0)) {
			gpio_direction_output(pb->bl_gpio, 1);
			gpio_set_value(pb->bl_gpio, 1);
			udelay(pb->ready_time);
			loop = IC_MAX_BR - brightness;
		}

		for (i = 0; i < loop; i++) {
			gpio_direction_output(pb->bl_gpio, 0);
			gpio_set_value(pb->bl_gpio, 0);
			ndelay(pb->interval_time);
			gpio_set_value(pb->bl_gpio, 1);
			ndelay(pb->interval_time);
		}
		spin_unlock_irqrestore(&pb->sgm_spin_lock, flags);
	}

	pb->last_brightness = brightness;


	if (pb->notify_after)
		pb->notify_after(pb->dev, brightness);

	return 0;
}

static int sgm3727_backlight_get_brightness(struct backlight_device *bl)
{
	return bl->props.brightness;
}

static int sgm3727_backlight_check_fb(struct backlight_device *bl,
				  struct fb_info *info)
{
	struct sgm_bl_data *pb = dev_get_drvdata(&bl->dev);

	return !pb->check_fb || pb->check_fb(pb->dev, info);
}

static void bl_delay_on_func(struct work_struct *work)
{
	struct sgm_bl_data *pb =
		container_of(work, struct sgm_bl_data, bl_delay_on_work.work);
	struct platform_device *pdev =
		container_of(pb->dev, struct platform_device, dev);
	struct backlight_device *bl = dev_get_drvdata(&pdev->dev);

	pr_info("bl_delay_on_func update brightness\r\n");
	backlight_update_status(bl);
}

static const struct backlight_ops sgm3727_backlight_ops = {
	.update_status	= sgm3727_backlight_update_status,
	.get_brightness	= sgm3727_backlight_get_brightness,
	.check_fb	= sgm3727_backlight_check_fb,
};
#ifdef CONFIG_HAS_EARLYSUSPEND
static void backlight_driver_early_suspend(struct early_suspend *h)
{
	struct sgm_bl_data *pb =
		container_of(h, struct sgm_bl_data, bd_early_suspend);

	gpio_direction_output(pb->bl_gpio, 0);
	gpio_set_value(pb->bl_gpio, 0);
	udelay(pb->shutdown_time);
	pb->last_brightness = 0;
}

static void backlight_driver_late_resume(struct early_suspend *h)
{
	struct sgm_bl_data *pb =
		container_of(h, struct sgm_bl_data, bd_early_suspend);
	struct platform_device *pdev =
		container_of(pb->dev, struct platform_device, dev);
	struct backlight_device *bl = dev_get_drvdata(&pdev->dev);

	backlight_update_status(bl);
}
#endif

static int sgm3727_backlight_probe(struct platform_device *pdev)
{
	struct backlight_properties props;
	struct platform_sgm3727_backlight_data *data = NULL;
	struct backlight_device *bl;
	struct sgm_bl_data *pb;
	struct property *prop;
	int ret;
	int length;
	int bl_delay_on = 0;

	if (pdev->dev.platform_data)
		data = pdev->dev.platform_data;

	else if (pdev->dev.of_node) {
		u32 val;
		data = kzalloc(sizeof(struct platform_sgm3727_backlight_data),
				GFP_KERNEL);
		if (!data)
			return -ENOMEM;

			/* determine the number of brightness levels */
		prop = of_find_property(pdev->dev.of_node, "brightness-levels", &length);
		if (!prop)
			return -EINVAL;

		data->max_brightness = length / sizeof(u32);

		/* read brightness levels from DT property */
		if (data->max_brightness > 0) {
			size_t size = sizeof(*data->levels) * data->max_brightness;

			data->levels = devm_kzalloc(&pdev->dev, size, GFP_KERNEL);
			if (!data->levels)
				return -ENOMEM;

			ret = of_property_read_u32_array(pdev->dev.of_node, "brightness-levels",
							 data->levels,
							 data->max_brightness);
			if (ret < 0)
				return ret;

			ret = of_property_read_u32(pdev->dev.of_node, "dft-brightness",
						   &val);
			if (ret < 0)
				return ret;

			data->dft_brightness = val;
			data->max_brightness--;
		}

		if (of_property_read_u32(pdev->dev.of_node,
				"backlight-gpio", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->bl_gpio = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"max-brightness", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->max_brightness = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"shutdown-time", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->shutdown_time = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"ready-time", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->ready_time = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"interval-time", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->interval_time = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"bl-on-delay", &val)) {
			bl_delay_on = 0;
		} else
			bl_delay_on = val;

		pdev->dev.platform_data = data;

	}
	if (!data) {
		dev_err(&pdev->dev, "failed to find platform data\n");
		return -EINVAL;
	}

	if (data->init) {
		ret = data->init(&pdev->dev);
		if (ret < 0)
			return ret;
	}
	pb = devm_kzalloc(&pdev->dev, sizeof(*pb), GFP_KERNEL);
	if (!pb) {
		dev_err(&pdev->dev, "no memory for state\n");
		ret = -ENOMEM;
		goto err_alloc;
	}
	pb->bl_gpio = data->bl_gpio;
	pb->max_brightness = data->max_brightness;
	pb->dft_brightness = data->dft_brightness;
	pb->shutdown_time = data->shutdown_time;
	pb->ready_time = data->ready_time;
	pb->interval_time = data->interval_time;
	pb->levels = data->levels;
	pb->notify = data->notify;
	pb->notify_after = data->notify_after;
	pb->check_fb = data->check_fb;

	pb->dev = &pdev->dev;

	ret = gpio_request(pb->bl_gpio, "gpio_bl");
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request gpio %d\n", pb->bl_gpio);
		goto err_alloc;
	}

	spin_lock_init(&pb->sgm_spin_lock);

	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = data->max_brightness;
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, pb,
				       &sgm3727_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

	bl->props.brightness = data->dft_brightness;

	/*brightness set in uboot is 17*/
	pb->last_brightness = 17;

	pr_info("sgm3727_backlight_probe bl-delay-on %d\r\n", bl_delay_on);
	if (bl_delay_on == 0)
		backlight_update_status(bl);
	else {
		INIT_DELAYED_WORK(&(pb->bl_delay_on_work), bl_delay_on_func);
		schedule_delayed_work(&(pb->bl_delay_on_work),
			msecs_to_jiffies(bl_delay_on));
	}

	platform_set_drvdata(pdev, bl);
#ifdef CONFIG_HAS_EARLYSUSPEND
	pb->bd_early_suspend.suspend = backlight_driver_early_suspend;
	pb->bd_early_suspend.resume = backlight_driver_late_resume;
	pb->bd_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&pb->bd_early_suspend);
#endif
	return 0;

err_bl:
	gpio_free(pb->bl_gpio);
err_alloc:
	if (data->exit)
		data->exit(&pdev->dev);
err_read:
	if (pdev->dev.of_node)
		kfree(data);
	return ret;
}

static int sgm3727_backlight_remove(struct platform_device *pdev)
{
	struct platform_sgm3727_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct sgm_bl_data *pb = dev_get_drvdata(&bl->dev);

	backlight_device_unregister(bl);
	gpio_direction_output(pb->bl_gpio, 0);
	gpio_set_value(pb->bl_gpio, 0);
	gpio_free(pb->bl_gpio);

	if (data && data->exit)
		data->exit(&pdev->dev);
	if (data && pdev->dev.of_node) {
		kfree(data);
		pdev->dev.platform_data = NULL;
	}
	return 0;
}


static const struct of_device_id sgm3727_backlight_of_match[] = {
	{ .compatible = "bcm,pwm-backlight", },
	{},
}
MODULE_DEVICE_TABLE(of, sgm3727_backlight_of_match);
static struct platform_driver sgm3727_backlight_driver = {
	.driver		= {
		.name	= "pwm-backlight",
		.owner	= THIS_MODULE,
		.of_match_table = sgm3727_backlight_of_match,
	},
	.probe		= sgm3727_backlight_probe,
	.remove		= sgm3727_backlight_remove,
};

module_platform_driver(sgm3727_backlight_driver);

MODULE_DESCRIPTION("SGM3727 Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sgm3727-backlight");

