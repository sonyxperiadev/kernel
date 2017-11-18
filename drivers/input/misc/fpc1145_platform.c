/*
 * FPC1145 Fingerprint sensor device driver
 *
 * This driver will control the platform resources that the FPC fingerprint
 * sensor needs to operate. The major things are probing the sensor to check
 * that it is actually connected and let the Kernel know this and with that also
 * enabling and disabling of regulators, enabling and disabling of platform
 * clocks, controlling GPIOs such as SPI chip select, sensor reset line, sensor
 * IRQ line, MISO and MOSI lines.
 *
 * The sensor's IRQ events will be pushed to Kernel's event handling system and
 * are exposed in the drivers event node. This makes it possible for a user
 * space process to poll the input node and receive IRQ events easily. Usually
 * this node is available under /dev/input/eventX where 'X' is a number given by
 * the event system. A user space process will need to traverse all the event
 * nodes and ask for its parent's name (through EVIOCGNAME) which should match
 * the value in device tree named input-device-name.
 *
 * This driver will NOT send any SPI commands to the sensor it only controls the
 * electrical parts.
 *
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */
/*
 * IOCTL implementation for FPC1145 driver
 * Copyright (C) 2017 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */

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
#include <linux/uaccess.h>
#include <linux/wakelock.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>

#define FPC1145_RESET_LOW_US 1000
#define FPC1145_RESET_HIGH1_US 100
#define FPC1145_RESET_HIGH2_US 1250
#define PWR_ON_STEP_SLEEP 100
#define PWR_ON_STEP_RANGE1 100
#define PWR_ON_STEP_RANGE2 900
#define NUM_PARAMS_REG_ENABLE_SET 2

#define FPC_IRQPOLL_TIMEOUT_MS 250

#define FPC_IOC_MAGIC	0x1145
#define FPC_IOCWPREPARE	_IOW(FPC_IOC_MAGIC, 0x01, int)
#define FPC_IOCWDEVWAKE	_IOW(FPC_IOC_MAGIC, 0x02, int)
#define FPC_IOCWRESET	_IOW(FPC_IOC_MAGIC, 0x03, int)
#define FPC_IOCRPREPARE	_IOR(FPC_IOC_MAGIC, 0x81, int)
#define FPC_IOCRDEVWAKE	_IOR(FPC_IOC_MAGIC, 0x82, int)
#define FPC_IOCRIRQ	_IOR(FPC_IOC_MAGIC, 0x83, int)
#define FPC_IOCRIRQPOLL	_IOR(FPC_IOC_MAGIC, 0x84, int)

static const char * const pctl_names[] = {
	"fpc1145_reset_reset",
	"fpc1145_reset_active",
	"fpc1145_irq_active",
#ifdef CONFIG_ARCH_SONY_LOIRE
	"fpc1145_ldo_enable",
	"fpc1145_ldo_disable",
#endif
};

struct vreg_config {
	char *name;
	unsigned long vmin;
	unsigned long vmax;
	int ua_load;
};

static const struct vreg_config const vreg_conf[] = {
#ifdef CONFIG_ARCH_SONY_LOIRE
	{ "vcc_spi", 1800000UL, 1800000UL, 10, },
#else
	{ "vdd_ana", 1800000UL, 1800000UL, 6000, },
	{ "vcc_spi", 1800000UL, 1800000UL, 10, },
	{ "vdd_io", 1800000UL, 1800000UL, 6000, },
#endif
};

struct fpc1145_data {
	struct device *dev;
	struct pinctrl *fingerprint_pinctrl;
	struct pinctrl_state *pinctrl_state[ARRAY_SIZE(pctl_names)];
	struct regulator *vreg[ARRAY_SIZE(vreg_conf)];

	int irq_gpio;
	int rst_gpio;
#ifdef CONFIG_ARCH_SONY_LOIRE
	int ldo_gpio;
#endif

	int irq;
	bool irq_fired;
	wait_queue_head_t irq_evt;

	struct wake_lock wakelock;
	struct mutex lock;
	bool prepared;
};

static struct fpc1145_data *fpc1145_drvdata = NULL;

static int vreg_setup(struct fpc1145_data *fpc1145, const char *name,
	bool enable)
{
	size_t i;
	int rc;
	struct regulator *vreg;
	struct device *dev = fpc1145->dev;

	for (i = 0; i < ARRAY_SIZE(fpc1145->vreg); i++) {
		const char *n = vreg_conf[i].name;

		if (!strncmp(n, name, strlen(n)))
			goto found;
	}
	dev_err(dev, "Regulator %s not found\n", name);
	return -EINVAL;
found:
	vreg = fpc1145->vreg[i];
	if (enable) {
		if (!vreg) {
			vreg = regulator_get(dev, name);
			if (IS_ERR(vreg)) {
				dev_err(dev, "Unable to get %s\n", name);
				return PTR_ERR(vreg);
			}
		}
		if (regulator_count_voltages(vreg) > 0) {
			rc = regulator_set_voltage(vreg, vreg_conf[i].vmin,
					vreg_conf[i].vmax);
			if (rc)
				dev_err(dev,
					"Unable to set voltage on %s, %d\n",
					name, rc);
		}
		rc = regulator_set_load(vreg, vreg_conf[i].ua_load);
		if (rc < 0)
			dev_err(dev, "Unable to set current on %s, %d\n",
					name, rc);
		rc = regulator_enable(vreg);
		if (rc) {
			dev_err(dev, "error enabling %s: %d\n", name, rc);
			regulator_put(vreg);
			vreg = NULL;
		}
		fpc1145->vreg[i] = vreg;
	} else {
		if (vreg) {
			if (regulator_is_enabled(vreg)) {
				regulator_disable(vreg);
				dev_dbg(dev, "disabled %s\n", name);
			}
			regulator_put(vreg);
			fpc1145->vreg[i] = NULL;
		}
		rc = 0;
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
 * @see fpc1145_probe
 */
static int select_pin_ctl(struct fpc1145_data *fpc1145, const char *name)
{
	size_t i;
	int rc;
	struct device *dev = fpc1145->dev;

	for (i = 0; i < ARRAY_SIZE(fpc1145->pinctrl_state); i++) {
		const char *n = pctl_names[i];

		if (!strncmp(n, name, strlen(n))) {
			rc = pinctrl_select_state(fpc1145->fingerprint_pinctrl,
					fpc1145->pinctrl_state[i]);
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

static int hw_reset(struct fpc1145_data *fpc1145)
{
	int irq_gpio;
	struct device *dev = fpc1145->dev;
	int rc = select_pin_ctl(fpc1145, "fpc1145_reset_active");

	if (rc)
		goto exit;
	usleep_range(FPC1145_RESET_HIGH1_US, FPC1145_RESET_HIGH1_US + 100);

	rc = select_pin_ctl(fpc1145, "fpc1145_reset_reset");
	if (rc)
		goto exit;
	usleep_range(FPC1145_RESET_LOW_US, FPC1145_RESET_LOW_US + 100);

	rc = select_pin_ctl(fpc1145, "fpc1145_reset_active");
	if (rc)
		goto exit;
	usleep_range(FPC1145_RESET_HIGH1_US, FPC1145_RESET_HIGH1_US + 100);

	irq_gpio = gpio_get_value(fpc1145->irq_gpio);
	dev_info(dev, "IRQ after reset %d\n", irq_gpio);
exit:
	return rc;
}

/**
 * Will setup clocks, GPIOs, and regulators to correctly initialize the touch
 * sensor to be ready for work.
 *
 * In the correct order according to the sensor spec this function will
 * enable/disable regulators, SPI platform clocks, and reset line, all to set
 * the sensor in a correct power on or off state "electrical" wise.
 *
 * @see  spi_prepare_set
 * @note This function will not send any commands to the sensor it will only
 *       control it "electrically".
 */
static int device_prepare(struct fpc1145_data *fpc1145, bool enable)
{
	int rc;

	mutex_lock(&fpc1145->lock);
	if (enable && !fpc1145->prepared) {

		fpc1145->prepared = true;
		select_pin_ctl(fpc1145, "fpc1145_reset_reset");

		rc = vreg_setup(fpc1145, "vcc_spi", true);
		if (rc)
			goto exit;

#ifdef CONFIG_ARCH_SONY_LOIRE
		(void)select_pin_ctl(fpc1145, "fpc1145_ldo_enable");
#else
		rc = vreg_setup(fpc1145, "vdd_io", true);
		if (rc)
			goto exit_1;

		rc = vreg_setup(fpc1145, "vdd_ana", true);
		if (rc)
			goto exit_2;
#endif

		usleep_range(PWR_ON_STEP_SLEEP,
				PWR_ON_STEP_SLEEP + PWR_ON_STEP_RANGE2);

		(void)select_pin_ctl(fpc1145, "fpc1145_reset_active");
		usleep_range(PWR_ON_STEP_SLEEP,
				PWR_ON_STEP_SLEEP + PWR_ON_STEP_RANGE1);

	} else if (!enable && fpc1145->prepared) {
		rc = 0;

		(void)select_pin_ctl(fpc1145, "fpc1145_reset_reset");
		usleep_range(PWR_ON_STEP_SLEEP,
				PWR_ON_STEP_SLEEP + PWR_ON_STEP_RANGE2);

#ifdef CONFIG_ARCH_SONY_LOIRE
		(void)select_pin_ctl(fpc1145, "fpc1145_ldo_disable");
#else
		(void)vreg_setup(fpc1145, "vdd_ana", false);
exit_2:
		(void)vreg_setup(fpc1145, "vdd_io", false);
exit_1:
#endif
		(void)vreg_setup(fpc1145, "vcc_spi", false);
exit:
		fpc1145->prepared = false;
	} else {
		rc = 0;
	}
	mutex_unlock(&fpc1145->lock);
	return rc;
}

static int fpc1145_device_open(struct inode *inode, struct file *fp)
{
	wake_lock_timeout(&fpc1145_drvdata->wakelock,
				usecs_to_jiffies(250));
	return 0;
}

static int fpc1145_device_release(struct inode *inode, struct file *fp)
{
	if (wake_lock_active(&fpc1145_drvdata->wakelock))
		wake_unlock(&fpc1145_drvdata->wakelock);
	return 0;
}

static long fpc1145_device_ioctl(struct file *fp,
		unsigned int cmd, unsigned long arg)
{
	int8_t val = 0;
	int rc = -EINVAL;
	void __user *usr = (void __user*)arg;

	switch (cmd) {
	case FPC_IOCWPREPARE:
		dev_dbg(fpc1145_drvdata->dev, "%s device\n",
			(arg == 0 ? "Unpreparing" : "Preparing"));
		rc = device_prepare(fpc1145_drvdata, !!arg);
		break;
	case FPC_IOCWDEVWAKE:
		dev_dbg(fpc1145_drvdata->dev, "Setting devwake %lu\n", arg);
		dev_dbg(fpc1145_drvdata->dev, "WDEVWAKE Not implemented.\n");
		break;
	case FPC_IOCWRESET:
		dev_dbg(fpc1145_drvdata->dev, "Resetting device\n");
		rc = hw_reset(fpc1145_drvdata);
		break;
	case FPC_IOCRPREPARE:
		rc = put_user((int8_t)fpc1145_drvdata->prepared,
				(int*) usr);
		break;
	case FPC_IOCRDEVWAKE:
		dev_dbg(fpc1145_drvdata->dev, "RDEVWAKE Not implemented.\n");
		break;
	case FPC_IOCRIRQ:
		val = gpio_get_value(fpc1145_drvdata->irq_gpio);
		rc = put_user(val, (int*) usr);
		break;
	case FPC_IOCRIRQPOLL:
		val = gpio_get_value(fpc1145_drvdata->irq_gpio);
		if (val) {
			/* We don't need to wait: IRQ has already fired */
			rc = put_user(val, (int*) usr);
			return rc;
		}

		if (fpc1145_drvdata->irq_fired) {
			fpc1145_drvdata->irq_fired = false;
			enable_irq_wake(fpc1145_drvdata->irq);
		}

		rc = wait_event_interruptible_timeout(fpc1145_drvdata->irq_evt,
				fpc1145_drvdata->irq_fired,
				msecs_to_jiffies(FPC_IRQPOLL_TIMEOUT_MS));
		if (rc == -ERESTARTSYS)
			return rc;

		val = gpio_get_value(fpc1145_drvdata->irq_gpio);
		rc = put_user(val, (int*) usr);
		break;
	default:
		rc = -ENOIOCTLCMD;
		dev_err(fpc1145_drvdata->dev, "Unknown IOCTL 0x%x.\n", cmd);
		break;
	}

	return rc;
}

static const struct file_operations fpc1145_device_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.open = fpc1145_device_open,
	.release = fpc1145_device_release,
	.unlocked_ioctl = fpc1145_device_ioctl,
	.compat_ioctl = fpc1145_device_ioctl,
};

static struct miscdevice fpc1145_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fingerprint",
	.fops = &fpc1145_device_fops,
};

static irqreturn_t fpc1145_irq_handler(int irq, void *handle)
{
	struct fpc1145_data *fpc1145 = handle;

	dev_dbg(fpc1145->dev, "%s\n", __func__);

	fpc1145->irq_fired = true;

	wake_lock_timeout(&fpc1145->wakelock, usecs_to_jiffies(250));
	wake_up_interruptible(&fpc1145->irq_evt);
	disable_irq_nosync(fpc1145->irq);

	return IRQ_HANDLED;
}

static int fpc1145_request_named_gpio(struct fpc1145_data *fpc1145,
	const char *label, int *gpio)
{
	struct device *dev = fpc1145->dev;
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

static int fpc1145_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int rc = 0;
	size_t i;
	int irqf;
	struct device_node *np = dev->of_node;

	struct fpc1145_data *fpc1145 = devm_kzalloc(dev, sizeof(*fpc1145),
			GFP_KERNEL);
	if (!fpc1145) {
		rc = -ENOMEM;
		goto exit;
	}

	fpc1145->dev = dev;
	platform_set_drvdata(pdev, fpc1145);
	fpc1145_drvdata = fpc1145;

	if (!np) {
		dev_err(dev, "no of node found\n");
		rc = -EINVAL;
		goto exit;
	}

	rc = fpc1145_request_named_gpio(fpc1145, "fpc,gpio_irq",
			&fpc1145->irq_gpio);
	if (rc)
		goto exit;
	rc = fpc1145_request_named_gpio(fpc1145, "fpc,gpio_rst",
			&fpc1145->rst_gpio);
	if (rc)
		goto exit;
#ifdef CONFIG_ARCH_SONY_LOIRE
	rc = fpc1145_request_named_gpio(fpc1145, "fpc,gpio_ldo",
			&fpc1145->ldo_gpio);
	if (rc)
		goto exit;
#endif

	fpc1145->fingerprint_pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(fpc1145->fingerprint_pinctrl)) {
		if (PTR_ERR(fpc1145->fingerprint_pinctrl) == -EPROBE_DEFER) {
			dev_info(dev, "pinctrl not ready\n");
			rc = -EPROBE_DEFER;
			goto exit;
		}
		dev_err(dev, "Target does not use pinctrl\n");
		fpc1145->fingerprint_pinctrl = NULL;
		rc = -EINVAL;
		goto exit;
	}

	for (i = 0; i < ARRAY_SIZE(fpc1145->pinctrl_state); i++) {
		const char *n = pctl_names[i];
		struct pinctrl_state *state =
			pinctrl_lookup_state(fpc1145->fingerprint_pinctrl, n);
		if (IS_ERR(state)) {
			dev_err(dev, "cannot find '%s'\n", n);
			rc = -EINVAL;
			goto exit;
		}
		dev_info(dev, "found pin control %s\n", n);
		fpc1145->pinctrl_state[i] = state;
	}

	rc = select_pin_ctl(fpc1145, "fpc1145_reset_reset");
	if (rc)
		goto exit;
	rc = select_pin_ctl(fpc1145, "fpc1145_irq_active");
	if (rc)
		goto exit;

	irqf = IRQF_TRIGGER_HIGH | IRQF_ONESHOT;
	mutex_init(&fpc1145->lock);

	wake_lock_init(&fpc1145->wakelock, WAKE_LOCK_SUSPEND, "fpc_wake");
	init_waitqueue_head(&fpc1145->irq_evt);
	fpc1145->irq_fired = false;

	fpc1145->irq = gpio_to_irq(fpc1145->irq_gpio);
	rc = devm_request_threaded_irq(dev, fpc1145->irq,
			NULL, fpc1145_irq_handler, irqf,
			dev_name(dev), fpc1145);
	if (rc) {
		dev_err(dev, "could not request irq %d\n",
				gpio_to_irq(fpc1145->irq_gpio));
		goto exit;
	}
	dev_dbg(dev, "requested irq %d\n", gpio_to_irq(fpc1145->irq_gpio));
	enable_irq_wake(fpc1145->irq);

	if (of_property_read_bool(dev->of_node, "fpc,enable-on-boot")) {
		dev_info(dev, "Enabling hardware\n");
		(void)device_prepare(fpc1145, true);
	}

	rc = misc_register(&fpc1145_misc);
	if (!rc)
		goto exit;

	dev_info(dev, "%s: ok\n", __func__);
exit:
	return rc;
}

static int fpc1145_remove(struct platform_device *pdev)
{
	struct fpc1145_data *fpc1145 = platform_get_drvdata(pdev);

	wake_lock_destroy(&fpc1145->wakelock);
	mutex_destroy(&fpc1145->lock);
#ifdef CONFIG_ARCH_SONY_LOIRE
	(void)vreg_setup(fpc1145, "vcc_spi", false);
#else
	(void)vreg_setup(fpc1145, "vdd_io", false);
	(void)vreg_setup(fpc1145, "vcc_spi", false);
	(void)vreg_setup(fpc1145, "vdd_ana", false);
#endif

	dev_info(&pdev->dev, "%s\n", __func__);
	return 0;
}

static struct of_device_id fpc1145_of_match[] = {
	{ .compatible = "fpc,fpc1020", },
	{ .compatible = "fpc,fpc1145", },
	{}
};
MODULE_DEVICE_TABLE(of, fpc1145_of_match);

static struct platform_driver fpc1145_driver = {
	.driver = {
		.name = "fpc1145",
		.owner = THIS_MODULE,
		.of_match_table = fpc1145_of_match,
	},
	.probe = fpc1145_probe,
	.remove = fpc1145_remove,
};

static int __init fpc1145_init(void)
{
	int rc = platform_driver_register(&fpc1145_driver);

	if (!rc)
		pr_info("%s OK\n", __func__);
	else
		pr_err("%s %d\n", __func__, rc);
	return rc;
}

static void __exit fpc1145_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&fpc1145_driver);
}

module_init(fpc1145_init);
module_exit(fpc1145_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Aleksej Makarov");
MODULE_AUTHOR("Henrik Tillman <henrik.tillman@fingerprints.com>");
MODULE_DESCRIPTION("FPC1145 Fingerprint sensor device driver.");
