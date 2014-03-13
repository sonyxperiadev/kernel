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
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/slab.h>
#include <mach/pinmux.h>
#ifdef CONFIG_THERMAL
#include <linux/thermal.h>
#endif
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif
#include <mach/pinmux.h>

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
	struct delayed_work bl_delay_on_work;
	int pb_enable_adapt_bright;
#ifdef CONFIG_THERMAL
	struct thermal_cooling_device *bl_cdev;
	int thermal_max_brightness;
	int curr_state;
#endif
};

/* Forward declarations */
static inline void bl_validate_limits(struct backlight_device *bl,
				int *brightness);
static int __init bl_debugfs_init(struct backlight_device *bl);
static void __exit bl_debugfs_exit(struct backlight_device *bl);

/*
During soft reset, the PWM registers are reset but the pad
control registers are not.
So for a short duration (till loader sets 0x404 to PWM control
register) the PWM output remains high.

During hard reset this is not seen as both the pad control and
the PWM registers are in reset state.

Hence setting the function of pad cntrl register at 0x3500489C from
PWM2 to GPIO24 on soft reset.
*/
static int pwm_pin = -1;
static int pwm_pin_reboot_func = -1;


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

		bl_validate_limits(bl, &brightness);
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
	int brightness = bl->props.brightness;

	bl_validate_limits(bl, &brightness);

	return brightness;
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

#ifdef CONFIG_OF

static int pwm_backlight_parse_dt(struct device *dev,
				  struct platform_pwm_backlight_data *data)
{
	struct device_node *node = dev->of_node;
	struct property *prop;
	int length;
	u32 val;
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
					   &val);
		if (ret < 0)
			return ret;

		data->dft_brightness = val;
		data->max_brightness--;
	}

	ret = of_property_read_u32(node, "pwm-id", &val);
	if (ret < 0)
		return ret;
	data->pwm_id = val;

	ret = of_property_read_u32(node, "polarity", &val);
	if (ret < 0)
		return ret;
	data->polarity = val;

	ret = of_property_read_u32(node, "pwm-period-ns", &val);
	if (ret < 0)
		return ret;
	data->pwm_period_ns = val;

	if (of_property_read_u32(node, "pb_enable_adapt_bright", &val))
		data->pb_enable_adapt_bright = 0;
	else
		data->pb_enable_adapt_bright = val;

	ret = of_property_read_string(node, "pwm-request-label",
					&data->pwm_request_label);
	if (ret < 0)
		return ret;

	if (of_property_read_u32(node, "bl-on-delay", &val))
		data->bl_delay_on = 0;
	else
		data->bl_delay_on = val;


	if (of_property_read_u32(node,
				"pwm_pin_name", &val)) {
		pwm_pin = -1;
	} else
		pwm_pin = val;

	if (of_property_read_u32(node,
				"pwm_pin_reboot_func", &val)) {
		pwm_pin_reboot_func = -1;
	} else
		pwm_pin_reboot_func = val;

	return ret;
}

static struct of_device_id pwm_backlight_of_match[] = {
	{ .compatible = "bcm,pwm-backlight" },
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

#ifdef CONFIG_THERMAL
/* Thermal cooling device helper functions */
enum {
	GET_COOLING_LEVEL,
	GET_BACKLIGHT_LEVEL,
};

static int bl_get_property(struct thermal_cooling_device *cdev,
		int in, int *out, int property)
{
	struct backlight_device *bl = cdev->devdata;
	struct pwm_bl_data *pb = bl_get_data(bl);
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

static inline void bl_validate_limits(struct backlight_device *bl,
			int *brightness)
{
	struct pwm_bl_data *pb = bl_get_data(bl);

	if (*brightness > pb->thermal_max_brightness)
		*brightness = pb->thermal_max_brightness;
}

static int bl_cooling_get_max_state(struct thermal_cooling_device *cdev,
			unsigned long *state)
{
	struct backlight_device *bl = cdev->devdata;
	*state = bl->props.max_brightness;
	return 0;
}

static int bl_cooling_get_cur_state(struct thermal_cooling_device *cdev,
			unsigned long *state)
{
	struct backlight_device *bl = cdev->devdata;
	struct pwm_bl_data *pb = bl_get_data(bl);
	u32 cur_level;

	if (pb->curr_state >= 0) {
		*state = pb->curr_state;
	} else {
		cur_level = pwm_backlight_get_brightness(bl);
		bl_get_property(cdev, pb->levels[cur_level], (int *)state,
			GET_COOLING_LEVEL);
	}

	return 0;
}

static int bl_cooling_set_cur_state(struct thermal_cooling_device *cdev,
			unsigned long state)
{
	struct backlight_device *bl = cdev->devdata;
	struct pwm_bl_data *pb = bl_get_data(bl);
	int cur_level = pwm_backlight_get_brightness(bl);
	u32 bl_level;

	/* is adaptive brightness control enabled */
	if (!pb->pb_enable_adapt_bright)
		return 0;

	/* validate brightness state passed */
	if (state < 0 || state > bl->props.max_brightness)
		return -EINVAL;

	if (pb->curr_state == state)
		return 0;

	/* remap cooling device state to backlight device state */
	if (bl_get_property(cdev, state, &bl_level, GET_BACKLIGHT_LEVEL))
		return -EINVAL;

	/* if backlight is already at requested level, ignore */
	if (bl_level == cur_level)
		return 0;

	pb->thermal_max_brightness = bl_level;
	pr_info("ADB: BBIC, state %lu Action:BL : %u\n", state, bl_level);
	pb->curr_state = state;
	backlight_update_status(bl);

	return 0;
}

static struct thermal_cooling_device_ops bl_cooling_ops = {
	.get_max_state = bl_cooling_get_max_state,
	.set_cur_state = bl_cooling_set_cur_state,
	.get_cur_state = bl_cooling_get_cur_state,
};

static int bl_brightness_coolant_register(struct backlight_device *bl)
{
	struct pwm_bl_data *pb = bl_get_data(bl);

	pb->thermal_max_brightness = bl->props.max_brightness;
	pb->curr_state = -1;
	if (!pb->pb_enable_adapt_bright)
		return 0;

	pb->bl_cdev = thermal_cooling_device_register(BACKLIGHT_CDEV_NAME,
						bl, &bl_cooling_ops);
	if (IS_ERR(pb->bl_cdev)) {
		dev_err(pb->dev, "backlight cooling device registration failed\n");
		return PTR_ERR(pb->bl_cdev);
	}

	return 0;
}

static void bl_brightness_coolant_unregister(struct backlight_device *bl)
{
	struct pwm_bl_data *pb = bl_get_data(bl);

	if (IS_ERR_OR_NULL(pb->bl_cdev))
		return;

	thermal_cooling_device_unregister(pb->bl_cdev);
}
#else /* CONFIG_THERMAL */

static int bl_brightness_coolant_register(struct backlight_device *bl)
{
	return 0;
}

static void bl_brightness_coolant_unregister(struct backlight_device *bl)
{
}

static void bl_validate_limits(struct backlight_device *bl, int *brightness)
{
}
#endif

static int pwm_backlight_probe(struct platform_device *pdev)
{
	struct platform_pwm_backlight_data *data = dev_get_platdata(&pdev->dev);
	struct platform_pwm_backlight_data defdata;
	struct backlight_properties props;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
	unsigned int max;
	int ret;

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
	if (data->levels) {
		max = data->levels[data->max_brightness];
		pb->levels = data->levels;
	} else
		max = data->max_brightness;

	pb->notify = data->notify;
	pb->notify_after = data->notify_after;
	pb->check_fb = data->check_fb;
	pb->exit = data->exit;
	pb->dev = &pdev->dev;
	pb->pb_enable_adapt_bright = data->pb_enable_adapt_bright;
	pb->pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pb->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM, trying legacy API\n");
		pb->pwm = pwm_request(data->pwm_id, data->pwm_request_label);
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

	/* Register backlight as coolant */
	ret = bl_brightness_coolant_register(bl);
	if (ret) {
		backlight_device_unregister(bl);
		goto err_alloc;
	}

	if (data->bl_delay_on == 0)
		backlight_update_status(bl);
	else {
		INIT_DELAYED_WORK(&(pb->bl_delay_on_work), bl_delay_on_func);
		schedule_delayed_work(&(pb->bl_delay_on_work),
				msecs_to_jiffies(data->bl_delay_on));
	}

	ret = bl_debugfs_init(bl);
	if (ret)
		dev_warn(&pdev->dev, "failed to create debud fs\n");

	return 0;

err_alloc:
	if (data->exit)
		data->exit(&pdev->dev);
	return ret;
}

static int __exit pwm_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	bl_debugfs_exit(bl);
	bl_brightness_coolant_unregister(bl);
	backlight_device_unregister(bl);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
	if (pb->exit)
		pb->exit(&pdev->dev);
	return 0;
}

static void pwm_backlight_shutdown(struct platform_device *pdev)
{
	struct pin_config new_pin_config;

	/*reset the pwm pin to GPIO function if defined in the kernel-dtb*/
	if (pwm_pin >= 0 && pwm_pin_reboot_func >= 0) {
		pr_info("reset the pwm pin to GPIO function\r\n");
		new_pin_config.name = pwm_pin;
		pinmux_get_pin_config(&new_pin_config);
		new_pin_config.func = pwm_pin_reboot_func;
		pinmux_set_pin_config(&new_pin_config);
	}
}

#ifdef CONFIG_PM_SLEEP
static int pwm_backlight_suspend(struct device *dev)
{
	struct backlight_device *bl = dev_get_drvdata(dev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	if (pb->notify)
		pb->notify(pb->dev, 0);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
	if (pb->notify_after)
		pb->notify_after(pb->dev, 0);
	return 0;
}

static int pwm_backlight_resume(struct device *dev)
{
	struct backlight_device *bl = dev_get_drvdata(dev);

	backlight_update_status(bl);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(pwm_backlight_pm_ops, pwm_backlight_suspend,
			 pwm_backlight_resume);

static struct platform_driver pwm_backlight_driver = {
	.driver		= {
		.name		= "pwm-backlight",
		.owner		= THIS_MODULE,
		.pm		= &pwm_backlight_pm_ops,
		.of_match_table	= of_match_ptr(pwm_backlight_of_match),
	},
	.probe		= pwm_backlight_probe,
	.remove		= pwm_backlight_remove,
	.shutdown	= pwm_backlight_shutdown,
};

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_pb_root_dir;

static int pb_get_adapt_bright(void *data, u64 *enable)
{
	struct pwm_bl_data *pb = bl_get_data(data);

	*enable = !!pb->pb_enable_adapt_bright;

	return 0;
}

static int pb_set_adapt_bright(void *data, u64 enable)
{
	struct backlight_device *bl = data;
	struct pwm_bl_data *pb = bl_get_data(bl);

	if (pb->pb_enable_adapt_bright == enable)
		return 0;

	pb->pb_enable_adapt_bright = enable;
	if (enable)
		bl_brightness_coolant_register(bl);
	else
		bl_brightness_coolant_unregister(bl);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pb_adapt_bright_fops, pb_get_adapt_bright,
				pb_set_adapt_bright, "%llu\n");

static void __exit bl_debugfs_exit(struct backlight_device *bl)
{
	if (!IS_ERR_OR_NULL(dent_pb_root_dir))
		debugfs_remove_recursive(dent_pb_root_dir);
}

static int __init bl_debugfs_init(struct backlight_device *bl)
{
	struct dentry *dentry_file;
	int ret = 0;

	dent_pb_root_dir = debugfs_create_dir("pwm_bl", NULL);
	if (IS_ERR_OR_NULL(dent_pb_root_dir)) {
		ret = PTR_ERR(dent_pb_root_dir);
		goto err;
	}

	dentry_file = debugfs_create_file("enable_adapt_bright",
			S_IRUSR | S_IWUSR, dent_pb_root_dir, bl,
			&pb_adapt_bright_fops);

	if (IS_ERR_OR_NULL(dentry_file)) {
		ret = PTR_ERR(dentry_file);
		goto err;
	}

	return ret;

err:
	if (!IS_ERR_OR_NULL(dent_pb_root_dir))
		debugfs_remove_recursive(dent_pb_root_dir);

	return ret;
}

#else /* CONFIG_DEBUG_FS */

static int __init bl_debugfs_init(struct backlight_device *bl)
{
	return 0;
}

static void __exit bl_debugfs_exit(struct backlight_device *bl)
{
	return 0;
}
#endif

module_platform_driver(pwm_backlight_driver);

MODULE_DESCRIPTION("PWM based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-backlight");
