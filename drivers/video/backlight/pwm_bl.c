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
	unsigned int		*levels;
	int			(*notify)(struct device *,
					  int brightness);
	void			(*notify_after)(struct device *,
					int brightness);
	int			(*check_fb)(struct device *, struct fb_info *);
	void			(*exit)(struct device *);
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

#ifdef CONFIG_KONA_TMON
		if (brightness > pb->max_brightness)
			brightness = pb->max_brightness;
#endif

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

int bl_delay_on = 0;
const char *pwm_request_label = NULL;
#ifdef CONFIG_OF
static int pwm_backlight_parse_dt(struct device *dev,
				  struct platform_pwm_backlight_data *data)
{
	struct device_node *node = dev->of_node;
	struct property *prop;
	int length;
	u32 val;
	int ret;
#ifdef CONFIG_KONA_TMON
	int size, i;
	unsigned long *temp_comp_tbl;
	int *int_ptr;
#endif
	const char *pwm_request_label = NULL;

	if (!node)
		return -ENODEV;

#if 0
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
#endif
	ret = of_property_read_u32(node, "pwm-id", &val);
	if (ret < 0)
		return ret;
	data->pwm_id = val;
	pr_info("pwm_id = %d\n", data->pwm_id);

	ret = of_property_read_u32(node,
			"max-brightness", &val);
	if (ret < 0)
		return ret;

	data->max_brightness = val;

	ret = of_property_read_u32(node,
			"dft-brightness", &val);
	if (ret < 0)
		return ret;

	data->dft_brightness = val;

	ret = of_property_read_u32(node,
			"polarity", &val);
	if (ret < 0)
		return ret;
	data->polarity = val;

	ret = of_property_read_u32(node,
			"pwm-period-ns", &val);
	if (ret < 0)
		return ret;
	data->pwm_period_ns = val;

#ifdef CONFIG_KONA_TMON
		if (!of_property_read_u32(node,
				"temp_comp_size", &val))
			data->temp_comp_size = val;

		if (data->temp_comp_size > 0) {
			temp_comp_tbl = (unsigned long *)of_get_property(
					node,
					"temp_comp_tbl", &size);
			if (!temp_comp_tbl) {
				ret = -EINVAL;
				return ret;
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

	ret = of_property_read_string(node,
		"pwm-request-label", &pwm_request_label);
	if (ret < 0)
		return ret;
	
	if (of_property_read_u32(node,
			"bl-on-delay", &val)) {
		bl_delay_on = 0;
	} else
		bl_delay_on = val;

	return ret;

err_alloc:
	if (data->exit)
		data->exit(dev);
#ifdef CONFIG_KONA_TMON
	kfree(data->temp_comp_tbl);
#endif
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

#ifdef CONFIG_KONA_TMON
static void pb_update_max_brightness(unsigned long curr_temp,
				struct pwm_bl_data *pb)
{
	struct platform_device *pdev;
	struct platform_pwm_backlight_data *data;
	struct backlight_device *bl;
	int i, prev_max_brightness;

	pdev = container_of(pb->dev, struct platform_device, dev);
	data = pdev->dev.platform_data;
	bl = platform_get_drvdata(pdev);

	prev_max_brightness = pb->max_brightness;
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
	if (prev_max_brightness != pb->max_brightness) {
		pr_info("[BACKLIGHT_DRIVER] Sensor: [BBIC], Threshold Crossed: [%lu], Threshold Crossing Direction: [%s], Throttling Action: [Max_Brightness = %d]"
		, pb->max_brightness > prev_max_brightness ?
		data->temp_comp_tbl[i+1].trigger_temp :
		data->temp_comp_tbl[i].trigger_temp,
		pb->max_brightness > prev_max_brightness ? "Fall" :
		"Rise", pb->max_brightness);
	}

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
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;
	struct backlight_properties props;
	struct backlight_device *bl;
	struct pwm_bl_data *pb;
	unsigned int max;
	int ret;

	if (!data) {
		data = kzalloc(sizeof(struct platform_pwm_backlight_data),
				GFP_KERNEL);
		if (!data)
			return -ENOMEM;

		ret = pwm_backlight_parse_dt(&pdev->dev, data);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to find platform data\n");
			goto err_init;
		}
		pdev->dev.platform_data = data;
	} else {
			bl_delay_on = data->bl_delay_on;
			pwm_request_label = data->pwm_request_label;
	}

	if (data->init) {
		ret = data->init(&pdev->dev);
		if (ret < 0)
			goto err_init;
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
	pb->pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pb->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM, trying legacy API\n");
		pb->pwm = pwm_request(data->pwm_id, "pwm-backlight");
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
		goto err_bl;
	}

	if (data->dft_brightness > data->max_brightness) {
		dev_warn(&pdev->dev,
			 "invalid default brightness level: %u, using %u\n",
			 data->dft_brightness, data->max_brightness);
		data->dft_brightness = data->max_brightness;
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
err_init:
	if (pdev->dev.of_node)
		kfree(data);
	return ret;
}

static int pwm_backlight_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = bl_get_data(bl);
	struct platform_pwm_backlight_data *data = pdev->dev.platform_data;

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
	if (pb->exit)
		pb->exit(&pdev->dev);
	return 0;
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

