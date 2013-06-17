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
#include <linux/pwm/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

struct pwm_bl_data {
	struct pwm_device	*pwm;
	struct device		*dev;
	unsigned int		period;
	unsigned int		lth_brightness;
	unsigned int		*levels;
	int			(*notify)(struct device *,
					  int brightness);
	void			(*notify_after)(struct device *,
					int brightness);
	int			(*check_fb)(struct device *, struct fb_info *);
	void			(*exit)(struct device *);
};

static int pwm_backlight_update_status(struct backlight_device *bl)
{
	struct pwm_bl_data *pb = bl_get_data(bl);
	int brightness = bl->props.brightness;
	int max = bl->props.max_brightness;

	if (bl->props.power != FB_BLANK_UNBLANK ||
	    bl->props.fb_blank != FB_BLANK_UNBLANK ||
	    bl->props.state & BL_CORE_FBBLANK)
		brightness = 0;

	if (pb->notify)
		brightness = pb->notify(pb->dev, brightness);

	if (brightness == 0) {
		pwm_config(pb->pwm, 0, pb->period);
		pwm_disable(pb->pwm);
	} else {
		int duty_cycle;

		if (pb->levels) {
			duty_cycle = pb->levels[brightness];
			max = pb->levels[max];
		} else {
			duty_cycle = brightness;
		}

		duty_cycle = pb->lth_brightness +
		     (duty_cycle * (pb->period - pb->lth_brightness) / max);
		pwm_config(pb->pwm, duty_cycle, pb->period);
		pwm_enable(pb->pwm);
	}

	if (pb->notify_after)
		pb->notify_after(pb->dev, brightness);

	return 0;
}

static int pwm_backlight_get_brightness(struct backlight_device *bl)
{
	return bl->props.brightness;
}

static int pwm_backlight_check_fb(struct backlight_device *bl,
				  struct fb_info *info)
{
	struct pwm_bl_data *pb = bl_get_data(bl);

	return !pb->check_fb || pb->check_fb(pb->dev, info);
}

static void bl_delay_on_func(struct work_struct *work)
{
	struct pwm_bl_data *pb =
		container_of(work, struct pwm_bl_data, bl_delay_on_work.work);
	struct platform_device *pdev =
		container_of(pb->dev, struct platform_device, dev);
	struct backlight_device *bl = dev_get_drvdata(&pdev->dev);

	pr_info("bl_delay_on_func update brightness\r\n");
	backlight_update_status(bl);
}

static const struct backlight_ops pwm_backlight_ops = {
	.update_status	= pwm_backlight_update_status,
	.get_brightness	= pwm_backlight_get_brightness,
	.check_fb	= pwm_backlight_check_fb,
};
#ifdef CONFIG_HAS_EARLYSUSPEND
static void backlight_driver_early_suspend(struct early_suspend *h)
{
	struct pwm_bl_data *pb = container_of(h, struct pwm_bl_data, bd_early_suspend);
	struct platform_device *pdev = container_of(pb->dev, struct platform_device, dev);
	struct backlight_device *bl = dev_get_drvdata(&pdev->dev);

	if( bl->props.brightness) {
		pwm_config(pb->pwm, 0, pb->period);
		pwm_disable(pb->pwm);
	}
}

static void backlight_driver_late_resume(struct early_suspend *h)
{
	struct pwm_bl_data *pb = container_of(h, struct pwm_bl_data, bd_early_suspend);
	struct platform_device *pdev = container_of(pb->dev, struct platform_device, dev);
	struct backlight_device *bl = dev_get_drvdata(&pdev->dev);
	int brightness = bl->props.brightness;

	if (brightness) {
		brightness = pb->lth_brightness +
			(brightness * (pb->period - pb->lth_brightness) /
			bl->props.max_brightness);
		pwm_config(pb->pwm, brightness, pb->period);
		pwm_enable(pb->pwm);
	}
}
#endif

#ifdef CONFIG_OF
static int pwm_backlight_parse_dt(struct device *dev,
				  struct platform_pwm_backlight_data *data)
{
	struct device_node *node = dev->of_node;
	struct property *prop;
	int length;
	u32 value;
	int ret;

	if (!node)
		return -ENODEV;

	memset(data, 0, sizeof(*data));

	/* determine the number of brightness levels */
	prop = of_find_property(node, "brightness-levels", &length);
	if (!prop)
		return -EINVAL;

	data->max_brightness = length / sizeof(u32);

	/* read brightness levels from DT property */
	if (data->max_brightness > 0) {
		size_t size = sizeof(*data->levels) * data->max_brightness;

		data->levels = devm_kzalloc(dev, size, GFP_KERNEL);
		if (!data->levels)
			return -ENOMEM;

		ret = of_property_read_u32_array(node, "brightness-levels",
						 data->levels,
						 data->max_brightness);
		if (ret < 0)
			return ret;

		ret = of_property_read_u32(node, "default-brightness-level",
					   &value);
		if (ret < 0)
			return ret;

		data->dft_brightness = value;
		data->max_brightness--;
	}

	/*
	 * TODO: Most users of this driver use a number of GPIOs to control
	 *       backlight power. Support for specifying these needs to be
	 *       added.
	 */

	return 0;
}

static struct of_device_id pwm_backlight_of_match[] = {
	{ .compatible = "pwm-backlight" },
	{ }
};

MODULE_DEVICE_TABLE(of, pwm_backlight_of_match);
#else
static int pwm_backlight_parse_dt(struct device *dev,
				  struct platform_pwm_backlight_data *data)
{
	return -ENODEV;
}
#endif

static int pwm_backlight_probe(struct platform_device *pdev)
{
#if 0
<<<<<<< HEAD
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;
	struct platform_pwm_backlight_data defdata;
	struct backlight_properties props;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
	unsigned int max;
=======
#endif
	struct backlight_properties props;
	struct platform_pwm_backlight_data *data = NULL;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
	const char *pwm_request_label = NULL;
//>>>>>>> sdb-common-android-jb-4.2.2
	int ret;
	int bl_delay_on = 0;

	if (pdev->dev.platform_data)
		data = pdev->dev.platform_data;

	else if (pdev->dev.of_node) {
		u32 val;
		data = kzalloc(sizeof(struct platform_pwm_backlight_data),
				GFP_KERNEL);
		if (!data)
			return -ENOMEM;

		if (of_property_read_u32(pdev->dev.of_node, "pwm-id", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->pwm_id = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"max-brightness", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->max_brightness = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"dft-brightness", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->dft_brightness = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"polarity", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->polarity = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"pwm-period-ns", &val)) {
			ret = -EINVAL;
			goto err_read;
		}
		data->pwm_period_ns = val;

		if (of_property_read_string(pdev->dev.of_node,
			"pwm-request-label", &pwm_request_label)) {
			ret = -EINVAL;
			goto err_read;
		}

		if (of_property_read_u32(pdev->dev.of_node,
				"bl-on-delay", &val)) {
			bl_delay_on = 0;
		} else
			bl_delay_on = val;

		pdev->dev.platform_data = data;

	}
	if (!data) {
		ret = pwm_backlight_parse_dt(&pdev->dev, &defdata);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to find platform data\n");
			return ret;
		}

		data = &defdata;
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
#if 0
<<<<<<< HEAD
	if (data->levels) {
		max = data->levels[data->max_brightness];
		pb->levels = data->levels;
	} else
		max = data->max_brightness;
=======
#endif
	pb->period = data->pwm_period_ns;
#if 0
>>>>>>> sdb-common-android-jb-4.2.2
#endif
	pb->notify = data->notify;
	pb->notify_after = data->notify_after;
	pb->check_fb = data->check_fb;
	pb->exit = data->exit;
	pb->dev = &pdev->dev;

	pb->pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pb->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM, trying legacy API\n");

		if (pdev->dev.of_node)
			pb->pwm = pwm_request(data->pwm_id, pwm_request_label);
		else
			pb->pwm = pwm_request(data->pwm_id, "backlight");

//		pb->pwm = pwm_request(data->pwm_id, "pwm-backlight");
		if (IS_ERR(pb->pwm)) {
			dev_err(&pdev->dev, "unable to request legacy PWM\n");
			ret = PTR_ERR(pb->pwm);
			goto err_alloc;
		}
	}

	dev_dbg(&pdev->dev, "got pwm for backlight\n");

	/*
	 * The DT case will set the pwm_period_ns field to 0 and store the
	 * period, parsed from the DT, in the PWM device. For the non-DT case,
	 * set the period from platform data.
	 */
	if (data->pwm_period_ns > 0)
		pwm_set_period(pb->pwm, data->pwm_period_ns);

	pb->period = pwm_get_period(pb->pwm);
	pb->lth_brightness = data->lth_brightness * (pb->period / max);

	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = data->max_brightness;
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, pb,
				       &pwm_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_alloc;
	}

	if (data->dft_brightness > data->max_brightness) {
		dev_warn(&pdev->dev,
			 "invalid default brightness level: %u, using %u\n",
			 data->dft_brightness, data->max_brightness);
		data->dft_brightness = data->max_brightness;
	}

	bl->props.brightness = data->dft_brightness;
	pwm_set_polarity(pb->pwm, data->polarity);

	platform_set_drvdata(pdev, bl);
	return 0;

err_alloc:
	if (data->exit)
		data->exit(&pdev->dev);
err_read:
	if (pdev->dev.of_node)
		kfree(data);
	return ret;
}

static int pwm_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	backlight_device_unregister(bl);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
#if 0
<<<<<<< HEAD
	if (pb->exit)
		pb->exit(&pdev->dev);
=======
	pwm_free(pb->pwm);
	if (data && data->exit)
		data->exit(&pdev->dev);
	if (data && pdev->dev.of_node) {
		kfree(data);
		pdev->dev.platform_data = NULL;
	}
>>>>>>> sdb-common-android-jb-4.2.2
#endif
	return 0;
}

#ifdef CONFIG_PM
static int pwm_backlight_suspend(struct device *dev)
{
	struct backlight_device *bl = dev_get_drvdata(dev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	if (pb->notify)
		pb->notify(pb->dev, 0);

	if (pb->notify_after)
		pb->notify_after(pb->dev, 0);
	return 0;
}

static int pwm_backlight_resume(struct device *dev)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(pwm_backlight_pm_ops, pwm_backlight_suspend,
			 pwm_backlight_resume);

#endif
static const struct of_device_id pwm_backlight_of_match[] = {
	{ .compatible = "bcm,pwm-backlight", },
	{},
}
MODULE_DEVICE_TABLE(of, pwm_backlight_of_match);
static struct platform_driver pwm_backlight_driver = {
	.driver		= {
#if 0
<<<<<<< HEAD
		.name		= "pwm-backlight",
		.owner		= THIS_MODULE,
=======
#endif
		.name	= "pwm-backlight",
		.owner	= THIS_MODULE,
		.of_match_table = pwm_backlight_of_match,
//>>>>>>> sdb-common-android-jb-4.2.2
#ifdef CONFIG_PM
		.pm		= &pwm_backlight_pm_ops,
#endif
//		.of_match_table	= of_match_ptr(pwm_backlight_of_match), Arun
	},
	.probe		= pwm_backlight_probe,
	.remove		= pwm_backlight_remove,
};

module_platform_driver(pwm_backlight_driver);

MODULE_DESCRIPTION("PWM based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-backlight");

