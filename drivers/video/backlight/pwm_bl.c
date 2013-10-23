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
#include <mach/pinmux.h>
#ifdef CONFIG_KONA_TMON
#include <linux/broadcom/kona_tmon.h>
#define TMON_NB_INIT_WORK_DELAY 10000
#endif
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif


struct pwm_bl_data {
	struct pwm_device	*pwm;
	struct device		*dev;
	unsigned int		period;
	unsigned int		lth_brightness;
	int			(*notify)(struct device *,
					  int brightness);
	void			(*notify_after)(struct device *,
					int brightness);
	int			(*check_fb)(struct device *, struct fb_info *);
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend bd_early_suspend;
#endif
	struct delayed_work bl_delay_on_work;
#ifdef CONFIG_KONA_TMON
	struct delayed_work tmon_nb_init_work;
	struct notifier_block tmon_nb;
	int max_brightness;
#endif
};
static struct pwm_bl_data *pwm_bl_data;
#ifdef CONFIG_KONA_TMON
/* Adaptive brightness on by default */
static bool pb_enable_adapt_bright = true;
#endif
#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_pb_root_dir;
#endif

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
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);
	int brightness = bl->props.brightness;
	int max = bl->props.max_brightness;

	if (bl->props.power != FB_BLANK_UNBLANK)
		brightness = 0;

	if (bl->props.fb_blank != FB_BLANK_UNBLANK)
		brightness = 0;

	if (pb->notify)
		brightness = pb->notify(pb->dev, brightness);

	if (brightness == 0) {
		pwm_config(pb->pwm, 0, pb->period);
		pwm_disable(pb->pwm);
	} else {
#ifdef CONFIG_KONA_TMON
		if (brightness > pb->max_brightness)
			brightness = pb->max_brightness;
#endif
		brightness = pb->lth_brightness +
			(brightness * (pb->period - pb->lth_brightness) / max);
		pwm_config(pb->pwm, brightness, pb->period);
		pwm_enable(pb->pwm);
	}

	if (pb->notify_after)
		pb->notify_after(pb->dev, brightness);

	return 0;
}

static int pwm_backlight_get_brightness(struct backlight_device *bl)
{
#ifdef CONFIG_KONA_TMON
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);

	if (bl->props.brightness > pb->max_brightness)
		return pb->max_brightness;
#endif
	return bl->props.brightness;
}

static int pwm_backlight_check_fb(struct backlight_device *bl,
				  struct fb_info *info)
{
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);

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

#ifdef CONFIG_KONA_TMON
static void pb_update_max_brightness(unsigned long curr_temp,
				struct pwm_bl_data *pb)
{
	struct platform_device *pdev;
	struct platform_pwm_backlight_data *data;
	struct backlight_device *bl;
	int i;

	pdev = container_of(pb->dev, struct platform_device, dev);
	data = pdev->dev.platform_data;
	bl = platform_get_drvdata(pdev);

	pb->max_brightness = data->max_brightness;

	if (!pb_enable_adapt_bright)
		goto update_status;

	for (i = data->temp_comp_size - 1; i >= 0; i--) {
		if (curr_temp >= data->temp_comp_tbl[i].trigger_temp) {
			pb->max_brightness =
					data->temp_comp_tbl[i].max_brightness;
			break;
		}
	}
	dev_dbg(&pdev->dev,
		"pb_update_max_brightness: max_brightness=%d\n",
		pb->max_brightness);

update_status:
	backlight_update_status(bl);
}

static void pb_tmon_nb_init(struct pwm_bl_data *pb)
{
	unsigned long curr_temp;

	curr_temp = (unsigned long)tmon_get_current_temp(true, true);
	pb_update_max_brightness(curr_temp, pb);
	tmon_register_notifier(&pb->tmon_nb);
}

static void pb_tmon_nb_init_work(struct work_struct *ws)
{
	struct pwm_bl_data *pb = container_of((struct delayed_work *)ws,
				struct pwm_bl_data, tmon_nb_init_work);

	pb_tmon_nb_init(pb);
}

static void pb_tmon_nb_remove(struct pwm_bl_data *pb, bool update_bright)
{
	unsigned long curr_temp;

	tmon_unregister_notifier(&pb->tmon_nb);
	if (update_bright) {
		curr_temp = (unsigned long)tmon_get_current_temp(true, true);
		pb_update_max_brightness(curr_temp, pb);
	}
}

static int pb_tmon_notify_handler(struct notifier_block *nb,
				unsigned long curr_temp, void *dev)
{
	struct pwm_bl_data *pb = container_of(nb, struct pwm_bl_data, tmon_nb);
	pb_update_max_brightness(curr_temp, pb);

	return 0;
}
#endif /* CONFIG_KONA_TMON */

static int pwm_backlight_probe(struct platform_device *pdev)
{
	struct backlight_properties props;
	struct platform_pwm_backlight_data *data = NULL;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
#ifdef CONFIG_KONA_TMON
	int size, i;
	unsigned long *temp_comp_tbl;
	int *int_ptr;
#endif
	const char *pwm_request_label = NULL;
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

#ifdef CONFIG_KONA_TMON
		if (!of_property_read_u32(pdev->dev.of_node,
				"temp_comp_size", &val))
			data->temp_comp_size = val;

		if (data->temp_comp_size > 0) {
			temp_comp_tbl = (unsigned long *)of_get_property(
					pdev->dev.of_node,
					"temp_comp_tbl", &size);
			if (!temp_comp_tbl) {
				ret = -EINVAL;
				goto err_read;
			}
			data->temp_comp_tbl = kzalloc(sizeof(
					struct pb_temp_comp)*
					data->temp_comp_size, GFP_KERNEL);
			if (!data->temp_comp_tbl) {
				ret = -EINVAL;
				goto err_alloc;
			}

			for (i = 0; i < data->temp_comp_size; i++) {
				data->temp_comp_tbl[i].trigger_temp =
					be32_to_cpu(*temp_comp_tbl++);
				int_ptr = (int *) temp_comp_tbl;
				data->temp_comp_tbl[i].max_brightness =
					be32_to_cpu(*int_ptr++);
				temp_comp_tbl = (unsigned long *) int_ptr;
			}
		}
#endif

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

		if (of_property_read_u32(pdev->dev.of_node,
				"pwm_pin_name", &val)) {
			pwm_pin = -1;
		} else
			pwm_pin = val;

		if (of_property_read_u32(pdev->dev.of_node,
				"pwm_pin_reboot_func", &val)) {
			pwm_pin_reboot_func = -1;
		} else
			pwm_pin_reboot_func = val;

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

	pb->period = data->pwm_period_ns;

	pb->notify = data->notify;
	pb->notify_after = data->notify_after;
	pb->check_fb = data->check_fb;
	pb->lth_brightness = data->lth_brightness *
		(data->pwm_period_ns / data->max_brightness);
	pb->dev = &pdev->dev;
	if (pdev->dev.of_node)
		pb->pwm = pwm_request(data->pwm_id, pwm_request_label);
	else
		pb->pwm = pwm_request(data->pwm_id, "backlight");

	if (IS_ERR(pb->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM for backlight\n");
		ret = PTR_ERR(pb->pwm);
		goto err_alloc;
	} else
		dev_dbg(&pdev->dev, "got pwm for backlight\n");

	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = data->max_brightness;
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, pb,
				       &pwm_backlight_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_bl;
	}

	bl->props.brightness = data->dft_brightness;
	pwm_set_polarity(pb->pwm, data->polarity);

	pr_info("pwm_backlight_probe bl-delay-on %d\r\n", bl_delay_on);
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

#ifdef CONFIG_KONA_TMON
	pb->max_brightness = data->max_brightness;

	if (data->temp_comp_size > 0) {
		if (pb_enable_adapt_bright) {
			INIT_DELAYED_WORK(&pb->tmon_nb_init_work,
					pb_tmon_nb_init_work);
			schedule_delayed_work(&pb->tmon_nb_init_work,
				msecs_to_jiffies(TMON_NB_INIT_WORK_DELAY));
		}
		pb->tmon_nb.notifier_call = pb_tmon_notify_handler;
	}
#endif
	pwm_bl_data = pb;

	return 0;

err_bl:
	pwm_free(pb->pwm);
err_alloc:
	if (data->exit)
		data->exit(&pdev->dev);
#ifdef CONFIG_KONA_TMON
	kfree(data->temp_comp_tbl);
#endif
err_read:
	if (pdev->dev.of_node)
		kfree(data);
	return ret;
}

static int pwm_backlight_remove(struct platform_device *pdev)
{
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);

#ifdef CONFIG_KONA_TMON
	if (data->temp_comp_size > 0 && pb_enable_adapt_bright)
		pb_tmon_nb_remove(pb, false);
	kfree(data->temp_comp_tbl);
#endif
#ifdef CONFIG_DEBUG_FS
	if (dent_pb_root_dir)
		debugfs_remove_recursive(dent_pb_root_dir);
#endif

	backlight_device_unregister(bl);
	pwm_config(pb->pwm, 0, pb->period);
	pwm_disable(pb->pwm);
	pwm_free(pb->pwm);
	if (data && data->exit)
		data->exit(&pdev->dev);
	if (data && pdev->dev.of_node) {
		kfree(data);
		pdev->dev.platform_data = NULL;
	}
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

#ifdef CONFIG_PM
static int pwm_backlight_suspend(struct device *dev)
{
	struct backlight_device *bl = dev_get_drvdata(dev);
	struct pwm_bl_data *pb = dev_get_drvdata(&bl->dev);

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
		.name	= "pwm-backlight",
		.owner	= THIS_MODULE,
		.of_match_table = pwm_backlight_of_match,
#ifdef CONFIG_PM
		.pm	= &pwm_backlight_pm_ops,
#endif
	},
	.probe		= pwm_backlight_probe,
	.remove		= pwm_backlight_remove,
	.shutdown	= pwm_backlight_shutdown,
};

#ifdef CONFIG_DEBUG_FS
#ifdef CONFIG_KONA_TMON
static ssize_t pb_get_adapt_bright(struct file *file,
	char __user *user_buf, size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(user_buf, count, ppos,
			pb_enable_adapt_bright ? "1\n" : "0\n", 2);
}

static ssize_t pb_set_adapt_bright(struct file *file,
	char const __user *buf, size_t count, loff_t *offset)
{
	u32 len = 0;
	char input_str[5];
	int input;
	bool old, new;

	memset(input_str, 0, ARRAY_SIZE(input_str));
	if (count > ARRAY_SIZE(input_str))
		len = ARRAY_SIZE(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(&input_str[0], "%d", &input);

	/* Check if input is valid */
	if (input == 0)
		new = false;
	else if (input == 1)
		new = true;
	else
		goto exit;

	if (!pwm_bl_data)
		return -EINVAL;

	old = pb_enable_adapt_bright;

	if (!pwm_bl_data)
		return -EINVAL;

	/* Check if we are enabling or disabling */
	if (old == false && new == true) {
		pb_enable_adapt_bright = true;
		pb_tmon_nb_init(pwm_bl_data);
	} else if (old == true && new == false) {
		pb_enable_adapt_bright = false;
		pb_tmon_nb_remove(pwm_bl_data, true);
	}

	return count;
exit:
	pr_info("USAGE:\necho <enable> > enable_adapt_bright\n");
	pr_info("where <enable> is 0 or 1\n");
	return count;
}

static const struct file_operations pb_enable_adapt_bright_ops = {
	.write = pb_set_adapt_bright,
	.read = pb_get_adapt_bright,
};

static ssize_t pb_get_temp_tholds(struct file *file,
	char __user *user_buf, size_t count, loff_t *ppos)
{
	u32 len = 0;
	int i;
	char out_str[200];
	struct platform_device *pdev;
	struct platform_pwm_backlight_data *data;

	memset(out_str, 0, sizeof(out_str));
	len += snprintf(out_str + len, sizeof(out_str) - len,
			"Level\t\tLimit Brightness\tThreshold\n");
	if (!pwm_bl_data)
		return -EINVAL;
	pdev = container_of(pwm_bl_data->dev, struct platform_device, dev);
	data = pdev->dev.platform_data;
	for (i = 0; i < data->temp_comp_size; i++)
		len += snprintf(out_str + len, sizeof(out_str) - len,
		"Level%d:\t\t%d\t\t\t%lu\n", i,
		data->temp_comp_tbl[i].max_brightness,
		data->temp_comp_tbl[i].trigger_temp);

	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static ssize_t pb_set_temp_tholds(struct file *file,
	char const __user *buf, size_t count, loff_t *offset)
{
	u32 len = 0;
	char input_str[20];
	unsigned int num_bright;
	int next, prev, i, thold;
	struct platform_device *pdev;
	struct platform_pwm_backlight_data *data;

	memset(input_str, 0, ARRAY_SIZE(input_str));
	if (count > ARRAY_SIZE(input_str))
		len = ARRAY_SIZE(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(&input_str[0], "%d%d", &i, &thold);

	/* Check if index is valid */
	if (!pwm_bl_data)
		return -EINVAL;
	pdev = container_of(pwm_bl_data->dev, struct platform_device, dev);
	data = pdev->dev.platform_data;
	num_bright = data->temp_comp_size;
	if (i < 0 || i >= num_bright)
		goto exit;

	/* Check if thold is in ascending order */
	prev = i - 1;
	next = i + 1;

	if (i > 0)
		if (thold <= data->temp_comp_tbl[prev].trigger_temp)
			goto exit;
	if (i < num_bright - 1)
		if (thold >= data->temp_comp_tbl[next].trigger_temp)
			goto exit;

	data->temp_comp_tbl[i].trigger_temp = thold;
	return count;
exit:
	pr_info("USAGE:\necho index thold > temp_tholds\n");
	pr_info("index range: [0-%d]\ntholds should be in ascending order\n",
			num_bright - 1);
	return count;
}

static const struct file_operations pb_temp_tholds_ops = {
	.write = pb_set_temp_tholds,
	.read = pb_get_temp_tholds,
};
#endif /* CONFIG_KONA_TMON */

int __init bl_debug_init(void)
{
	dent_pb_root_dir = debugfs_create_dir("pwm_bl", NULL);
#ifdef CONFIG_KONA_TMON
	if (!debugfs_create_file("enable_adapt_bright",  S_IRUSR | S_IWUSR,
			dent_pb_root_dir, NULL, &pb_enable_adapt_bright_ops))
		return -ENOMEM;
	if (!debugfs_create_file("temp_tholds",  S_IRUSR | S_IWUSR,
			dent_pb_root_dir, NULL, &pb_temp_tholds_ops))
		return -ENOMEM;
#endif
	return 0;
}

late_initcall(bl_debug_init);

#endif /* CONFIG_DEBUG_FS */

module_platform_driver(pwm_backlight_driver);

MODULE_DESCRIPTION("PWM based Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-backlight");

