/*
 * ET51X Fingerprint sensor device driver
 *
 * This driver will control the platform resources that the Egistec fingerprint
 * sensor needs to operate. The major things are probing the sensor to check
 * that it is actually connected and let the Kernel know this and with that also
 * enabling and disabling of regulators. controlling the GPIO reset line and
 * handling GPIO IRQ's.
 *
 * IRQ events can be received by polling on /dev/fingerprint or through the
 * IOCTL interface. This interface is used at the same time to control the
 * (power) state of the sensor and reset it if necessary.
 *
 * This driver will NOT send any SPI commands to the sensor, it only controls
 * the electrical parts.
 *
 * The driver has been based on the FPC driver from the Sony Open Devices
 * Project, and it's original cpcopyright notices have been included below to
 * respect that.
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */
/*
 * IOCTL implementation for FPC driver
 * Copyright (C) 2017 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */

#include "cei_fp_detect.h"

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/uaccess.h>


#define PWR_ON_STEP_SLEEP 100
#define PWR_ON_STEP_RANGE1 100
#define PWR_ON_STEP_RANGE2 900

#define ET51X_RESET_LOW_US 1000
#define ET51X_RESET_HIGH1_US 100

#define ET51X_IRQPOLL_TIMEOUT_MS 500
#define ET51X_MAX_HAL_PROCESSING_TIME 400

#define ET51X_IOC_MAGIC 0x1145
#define ET51X_IOC_R_BASE 0x80
#define ET51X_IOCWPREPARE _IOW(ET51X_IOC_MAGIC, 0x01, int)
#define ET51X_IOCWDEVWAKE _IOW(ET51X_IOC_MAGIC, 0x02, int)
#define ET51X_IOCWRESET _IOW(ET51X_IOC_MAGIC, 0x03, int)
#define ET51X_IOCWAWAKE _IOW(ET51X_IOC_MAGIC, 0x04, int)
#define ET51X_IOCRPREPARE _IOR(ET51X_IOC_MAGIC, ET51X_IOC_R_BASE + 0x01, int)
#define ET51X_IOCRDEVWAKE _IOR(ET51X_IOC_MAGIC, ET51X_IOC_R_BASE + 0x02, int)
#define ET51X_IOCRIRQ _IOR(ET51X_IOC_MAGIC, ET51X_IOC_R_BASE + 0x03, int)
#define ET51X_IOCRIRQPOLL _IOR(ET51X_IOC_MAGIC, ET51X_IOC_R_BASE + 0x04, int)
#define ET51X_IOCRHWTYPE _IOR(ET51X_IOC_MAGIC, ET51X_IOC_R_BASE + 0x05, int)

#define ET51X_REGULATOR_VDD_ANA "vdd_ana"

static const char *const pctl_names[] = {
	"et51x_reset_reset",
	"et51x_reset_active",
	"et51x_irq_active",
};

struct et51x_data {
	struct miscdevice misc;
	struct device *dev;
	struct pinctrl *fingerprint_pinctrl;
	struct pinctrl_state *pinctrl_state[ARRAY_SIZE(pctl_names)];
	struct regulator *vdd_ana;

	int irq_gpio;

	int irq;
	bool irq_fired;
	wait_queue_head_t irq_evt;

	struct mutex intrpoll_lock;
	struct mutex lock;
	bool prepared;
	bool check_sensor_type;
	bool low_voltage_probe;
};

struct et51x_awake_args {
	int awake;
	unsigned int timeout;
};

struct et51x_data *to_et51x_data(struct file *fp)
{
	struct miscdevice *md = (struct miscdevice *)fp->private_data;

	return container_of(md, struct et51x_data, misc);
}

static int et51x_vreg_set_voltage(struct device *dev, struct regulator *vreg,
		int voltage)
{
	int rc = 0;

	if (!dev || !vreg)
		return -EINVAL;

	dev_dbg(dev, "Setting regulator %s to %d volts\n",
			ET51X_REGULATOR_VDD_ANA,
			voltage);

	if (regulator_count_voltages(vreg) > 0) {
		rc = regulator_set_voltage(vreg, voltage, voltage);
		if (rc)
			dev_err(dev, "Unable to set voltage to %d: %d\n",
					voltage, rc);
	} else {
		dev_warn(dev, "No voltages available");
	}

	rc = regulator_enable(vreg);
	if (rc)
		dev_err(dev, "Unable to enable: %d\n", rc);

	return rc;
}

static int vreg_setup(struct et51x_data *et51x, bool enable)
{
	int rc = 0;
	struct regulator *vreg = et51x->vdd_ana;
	struct device *dev = et51x->dev;

	if (!vreg)
		return -EINVAL;

	dev_dbg(dev, "%s regulator %s\n", enable ? "enabling" : "disabling",
			ET51X_REGULATOR_VDD_ANA);

	if (enable) {
		rc = et51x_vreg_set_voltage(dev, vreg, 3300000);
	} else {
		if (regulator_is_enabled(vreg)) {
			regulator_disable(vreg);
			dev_dbg(dev, "disabled %s\n", ET51X_REGULATOR_VDD_ANA);
		}
	}

	return rc;
}

/**
 * Will try to select the set of pins (GPIOS) defined in a pin control node of
 * the device tree named @p name.
 *
 * The node can contain several eg. GPIOs that is controlled when selecting it.
 * The node may activate or deactivate the pins it contains, the action is
 * defined in the device tree node itself and not here. The states used
 * internally is fetched at probe time.
 *
 * @see pctl_names
 * @see et51x_probe
 */
static int select_pin_ctl(struct et51x_data *et51x, const char *name)
{
	size_t i;
	int rc;
	struct device *dev = et51x->dev;

	for (i = 0; i < ARRAY_SIZE(et51x->pinctrl_state); i++) {
		const char *n = pctl_names[i];

		if (!strncmp(n, name, strlen(n))) {
			rc = pinctrl_select_state(et51x->fingerprint_pinctrl,
						  et51x->pinctrl_state[i]);
			if (rc)
				dev_err(dev, "cannot select '%s'\n", name);
			else
				dev_dbg(dev, "Selected '%s'\n", name);
			goto exit;
		}
	}
	rc = -EINVAL;
	dev_err(dev, "%s:'%s' not found\n", __func__, name);
exit:
	return rc;
}

static bool et51x_get_gpio_triggered(const struct et51x_data *drvdata)
{
	return !gpio_get_value(drvdata->irq_gpio);
}

static int hw_reset(struct et51x_data *et51x)
{
	int irq_gpio;
	struct device *dev = et51x->dev;
	int rc = select_pin_ctl(et51x, "et51x_reset_active");
	if (rc)
		goto exit;
	usleep_range(ET51X_RESET_HIGH1_US, ET51X_RESET_HIGH1_US + 100);

	rc = select_pin_ctl(et51x, "et51x_reset_reset");
	if (rc)
		goto exit;
	usleep_range(ET51X_RESET_LOW_US, ET51X_RESET_LOW_US + 100);

	rc = select_pin_ctl(et51x, "et51x_reset_active");
	if (rc)
		goto exit;
	usleep_range(ET51X_RESET_HIGH1_US, ET51X_RESET_HIGH1_US + 100);

	irq_gpio = et51x_get_gpio_triggered(et51x);
	dev_dbg(dev, "IRQ after reset %d\n", irq_gpio);
exit:
	return rc;
}

static int device_prepare(struct et51x_data *et51x, bool enable)
{
	int rc = 0;

	mutex_lock(&et51x->lock);
	if (enable && !et51x->prepared) {
		et51x->prepared = true;

		rc = vreg_setup(et51x, true);
		if (rc)
			goto exit;
	} else if (!enable && et51x->prepared) {
		(void)vreg_setup(et51x, false);
	exit:
		et51x->prepared = false;
	} else {
		rc = 0;
	}
	mutex_unlock(&et51x->lock);
	return rc;
}

static int et51x_device_open(struct inode *inode, struct file *fp)
{
	struct et51x_data *et51x = to_et51x_data(fp);
	pm_wakeup_event(et51x->dev, 1);
	return 0;
}

static int et51x_device_release(struct inode *inode, struct file *fp)
{
	struct et51x_data *et51x = to_et51x_data(fp);
	pm_relax(et51x->dev);
	return 0;
}

static inline void et51x_enable_irq_if_disabled(struct et51x_data *et51x)
{
	mutex_lock(&et51x->intrpoll_lock);

	/* Enable the irq if not enabled: */
	if (et51x->irq_fired) {
		et51x->irq_fired = false;
		dev_dbg(et51x->dev, "%s: enabling irq\n", __func__);
		enable_irq(et51x->irq);
	}

	mutex_unlock(&et51x->intrpoll_lock);
}

static long et51x_device_ioctl(struct file *fp, unsigned int cmd,
			       unsigned long arg)
{
	int8_t val = 0;
	int rc = -EINVAL;
	unsigned int timeout = 0;
	struct et51x_awake_args awake_args;
	void __user *usr = (void __user *)arg;
	struct et51x_data *et51x = to_et51x_data(fp);

	switch (cmd) {
	case ET51X_IOCWPREPARE:
		dev_dbg(et51x->dev, "%s device\n",
			(arg == 0 ? "Unpreparing" : "Preparing"));
		rc = device_prepare(et51x, !!arg);
		break;
	case ET51X_IOCWDEVWAKE:
		dev_dbg(et51x->dev, "Setting devwake %lu\n", arg);
		dev_dbg(et51x->dev, "WDEVWAKE Not implemented.\n");
		break;
	case ET51X_IOCWRESET:
		dev_dbg(et51x->dev, "Resetting device\n");
		rc = hw_reset(et51x);
		break;
	case ET51X_IOCWAWAKE:
		rc = copy_from_user(&awake_args, (struct et51x_awake_args *)usr,
				    sizeof(awake_args));
		if (rc)
			break;
		timeout = min(awake_args.timeout,
			      (unsigned int)ET51X_MAX_HAL_PROCESSING_TIME);
		if (awake_args.awake) {
			dev_dbg(et51x->dev, "Extending wakelock for %dms\n",
				timeout);
			pm_wakeup_event(et51x->dev, timeout);
		} else {
			dev_dbg(et51x->dev, "Relaxing wakelock\n");
			pm_relax(et51x->dev);
		}
		break;
	case ET51X_IOCRPREPARE:
		rc = put_user((int8_t)et51x->prepared, (int *)usr);
		break;
	case ET51X_IOCRDEVWAKE:
		dev_dbg(et51x->dev, "RDEVWAKE Not implemented.\n");
		break;
	case ET51X_IOCRIRQ:
		val = et51x_get_gpio_triggered(et51x);
		rc = put_user(val, (int *)usr);
		break;
	case ET51X_IOCRIRQPOLL:
		val = et51x_get_gpio_triggered(et51x);
		if (val) {
			/* We don't need to wait: IRQ has already fired */
			rc = put_user(val, (int *)usr);
			return rc;
		}

		et51x_enable_irq_if_disabled(et51x);

		rc = wait_event_interruptible_timeout(
			et51x->irq_evt, et51x->irq_fired,
			msecs_to_jiffies(ET51X_IRQPOLL_TIMEOUT_MS));
		if (rc == -ERESTARTSYS)
			return rc;

		val = et51x_get_gpio_triggered(et51x);
		if (val)
			pm_wakeup_event(et51x->dev,
					ET51X_MAX_HAL_PROCESSING_TIME);

		rc = put_user(val, (int *)usr);
		break;
	case ET51X_IOCRHWTYPE:
		if (et51x->check_sensor_type)
			rc = put_user((int)FP_HW_TYPE_EGISTEC, (int *)usr);
		else
			rc = -ENOSYS;
		break;
	default:
		rc = -ENOIOCTLCMD;
		dev_err(et51x->dev, "Unknown IOCTL 0x%x.\n", cmd);
		break;
	}

	return rc;
}

static unsigned int et51x_poll_interrupt(struct file *fp,
					 struct poll_table_struct *wait)
{
	int val = 0;
	struct et51x_data *et51x = to_et51x_data(fp);
	struct device *dev = et51x->dev;

	/* Add current file to the waiting list */
	poll_wait(fp, &et51x->irq_evt, wait);

	val = et51x_get_gpio_triggered(et51x);
	if (val) {
		dev_dbg(dev, "gpio triggered\n");
		pm_wakeup_event(dev, ET51X_MAX_HAL_PROCESSING_TIME);
		return POLLIN | POLLRDNORM;
	}

	/*
	 * Nothing happened yet; make the poll wait for irq_evt.
	 * The wakelock can be relaxed preemptively, as no processing has to
	 * be done until the next wake-enabled IRQ fires.
	 */
	et51x_enable_irq_if_disabled(et51x);
	pm_relax(dev);

	return 0;
}

static int et51x_device_suspend(struct device *dev)
{
	struct et51x_data *et51x = dev_get_drvdata(dev);

	dev_dbg(dev, "Suspending device\n");

	/* Wakeup when finger detected */
	enable_irq_wake(et51x->irq);

	return 0;
}

static int et51x_device_resume(struct device *dev)
{
	struct et51x_data *et51x = dev_get_drvdata(dev);

	dev_dbg(dev, "Resuming device\n");

	disable_irq_wake(et51x->irq);

	return 0;
}

static const struct file_operations et51x_device_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.open = et51x_device_open,
	.release = et51x_device_release,
	.unlocked_ioctl = et51x_device_ioctl,
	.compat_ioctl = et51x_device_ioctl,
	.poll = et51x_poll_interrupt,
};

static irqreturn_t et51x_irq_handler(int irq, void *handle)
{
	struct et51x_data *et51x = handle;

	mutex_lock(&et51x->intrpoll_lock);

	dev_dbg(et51x->dev, "%s: gpio=%d\n", __func__,
			et51x_get_gpio_triggered(et51x));

	et51x->irq_fired = true;

	disable_irq_nosync(et51x->irq);
	pm_wakeup_event(et51x->dev, ET51X_MAX_HAL_PROCESSING_TIME);
	wake_up_interruptible(&et51x->irq_evt);

	mutex_unlock(&et51x->intrpoll_lock);

	return IRQ_HANDLED;
}

static int et51x_request_named_gpio(struct et51x_data *et51x, const char *label,
				    int *gpio)
{
	struct device *dev = et51x->dev;
	struct device_node *np = dev->of_node;
	int rc = of_get_named_gpio(np, label, 0);

	if (rc < 0) {
		dev_err(dev, "failed to get '%s'\n", label);
		return rc;
	}
	*gpio = rc;
	rc = devm_gpio_request(dev, *gpio, label);
	if (rc) {
		dev_err(dev, "failed to request gpio %d\n", *gpio);
		return rc;
	}
	dev_dbg(dev, "%s %d\n", label, *gpio);
	return 0;
}

static int et51x_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int rc = 0;
	size_t i;
	int irqf;
	struct device_node *np = dev->of_node;
	int hw_type = 0;

	struct et51x_data *et51x =
		devm_kzalloc(dev, sizeof(*et51x), GFP_KERNEL);
	if (!et51x) {
		rc = -ENOMEM;
		goto exit;
	}

	et51x->misc = (struct miscdevice){
		.minor = MISC_DYNAMIC_MINOR,
		.name = "fingerprint",
		.fops = &et51x_device_fops,
	};

	et51x->dev = dev;
	platform_set_drvdata(pdev, et51x);
	dev_set_drvdata(et51x->dev, et51x);

	if (!np) {
		dev_err(dev, "no of node found\n");
		rc = -EINVAL;
		goto exit;
	}

	et51x->vdd_ana = devm_regulator_get(dev, ET51X_REGULATOR_VDD_ANA);
	if (IS_ERR_OR_NULL(et51x->vdd_ana)) {
		dev_err(dev, "CRITICAL: Cannot get %s regulator: %ld\n",
			ET51X_REGULATOR_VDD_ANA, PTR_ERR(et51x->vdd_ana));
		if (IS_ERR(et51x->vdd_ana))
			rc = PTR_ERR(et51x->vdd_ana);
		else
			rc = -EINVAL;
		et51x->vdd_ana = NULL;
		goto exit;
	}

	et51x->check_sensor_type = of_property_read_bool(dev->of_node,
			"et51x,check-sensor-type");
	/* Use low-voltage probe by default to prevent HW damage */
	et51x->low_voltage_probe = !of_property_read_bool(dev->of_node,
			"et51x,no-low-voltage-probe");

	dev_dbg(dev, "check_sensor_type: %d, low_voltage_probe: %d\n",
			et51x->check_sensor_type, et51x->low_voltage_probe);

	if (et51x->low_voltage_probe)
		/* Start at 1.8v which is the voltage used for FPC,
		 * to err on the side of caution:
		 */
		rc = et51x_vreg_set_voltage(et51x->dev, et51x->vdd_ana,
				1800000);
	else
		rc = vreg_setup(et51x, true);

	if (rc)
		goto exit_powerdown;

	if (et51x->check_sensor_type) {
		hw_type = cei_fp_module_detect();

		if (hw_type != FP_HW_TYPE_EGISTEC) {
			dev_info(dev,
				"Egistec sensor not found, bailing out\n");
			rc = -ENODEV;
			goto exit_powerdown;
		}

		dev_info(dev, "Detected Egistec sensor\n");

		/* Scale up to 3.3v */
		rc = vreg_setup(et51x, true);
		if (rc)
			goto exit_powerdown;
	}

	rc = et51x_request_named_gpio(et51x, "et51x,gpio_irq",
				      &et51x->irq_gpio);
	if (rc)
		goto exit_powerdown;

	et51x->fingerprint_pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(et51x->fingerprint_pinctrl)) {
		if (PTR_ERR(et51x->fingerprint_pinctrl) == -EPROBE_DEFER) {
			dev_info(dev, "pinctrl not ready\n");
			rc = -EPROBE_DEFER;
			goto exit_powerdown;
		}
		dev_err(dev, "Target does not use pinctrl\n");
		et51x->fingerprint_pinctrl = NULL;
		rc = -EINVAL;
		goto exit_powerdown;
	}

	for (i = 0; i < ARRAY_SIZE(et51x->pinctrl_state); i++) {
		const char *n = pctl_names[i];
		struct pinctrl_state *state =
			pinctrl_lookup_state(et51x->fingerprint_pinctrl, n);
		if (IS_ERR(state)) {
			dev_err(dev, "cannot find '%s'\n", n);
			rc = -EINVAL;
			goto exit_powerdown;
		}
		dev_info(dev, "found pin control %s\n", n);
		et51x->pinctrl_state[i] = state;
	}

	rc = select_pin_ctl(et51x, "et51x_reset_reset");
	if (rc)
		goto exit;
	rc = select_pin_ctl(et51x, "et51x_irq_active");
	if (rc)
		goto exit;

	irqf = IRQF_TRIGGER_LOW | IRQF_ONESHOT;
	mutex_init(&et51x->lock);
	mutex_init(&et51x->intrpoll_lock);

	device_init_wakeup(dev, true);
	init_waitqueue_head(&et51x->irq_evt);
	et51x->irq_fired = false;

	et51x->irq = gpio_to_irq(et51x->irq_gpio);
	rc = devm_request_threaded_irq(dev, et51x->irq, NULL, et51x_irq_handler,
				       irqf, dev_name(dev), et51x);
	if (rc) {
		dev_err(dev, "could not request irq %d\n", et51x->irq);
		goto exit_powerdown;
	}
	dev_dbg(dev, "requested irq %d\n", et51x->irq);
	enable_irq_wake(et51x->irq);

	/* Reset the device once: */
	hw_reset(et51x);

	rc = misc_register(&et51x->misc);
	if (rc)
		goto exit_powerdown;

	dev_info(dev, "%s: ok\n", __func__);

	if (of_property_read_bool(dev->of_node, "et51x,enable-on-boot")) {
		dev_info(dev, "Leaving power enabled\n");
		goto exit;
	}

exit_powerdown:
	(void)vreg_setup(et51x, false);
exit:
	return rc;
}

static int et51x_remove(struct platform_device *pdev)
{
	struct et51x_data *et51x = platform_get_drvdata(pdev);

	device_init_wakeup(et51x->dev, false);
	mutex_destroy(&et51x->lock);
	(void)device_prepare(et51x, false);

	dev_info(&pdev->dev, "%s\n", __func__);
	return 0;
}

/* clang-format off */
static struct of_device_id et51x_of_match[] = {
	{ .compatible = "etspi,et510", },
	{}
};
/* clang-format on */
MODULE_DEVICE_TABLE(of, et51x_of_match);

static SIMPLE_DEV_PM_OPS(et51x_pm_ops, et51x_device_suspend,
			 et51x_device_resume);

static struct platform_driver et51x_driver = {
	.driver = {
		.name = "et51x",
		.owner = THIS_MODULE,
		.of_match_table = et51x_of_match,
		.pm = &et51x_pm_ops,
	},
	.probe = et51x_probe,
	.remove = et51x_remove,
};

static int __init et51x_init(void)
{
	int rc = platform_driver_register(&et51x_driver);

	if (!rc)
		pr_info("%s OK\n", __func__);
	else
		pr_err("%s %d\n", __func__, rc);
	return rc;
}

static void __exit et51x_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&et51x_driver);
}

module_init(et51x_init);
module_exit(et51x_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("et51x Fingerprint sensor device driver.");
