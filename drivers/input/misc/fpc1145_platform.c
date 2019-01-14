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
#include <linux/uaccess.h>
#include <linux/pm_wakeup.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/poll.h>

#define FPC1145_RESET_LOW_US 1000
#define FPC1145_RESET_HIGH1_US 100
#define FPC1145_RESET_HIGH2_US 1250
#define PWR_ON_STEP_SLEEP 100
#define PWR_ON_STEP_RANGE1 100
#define PWR_ON_STEP_RANGE2 900
#define NUM_PARAMS_REG_ENABLE_SET 2

#define FPC_IRQPOLL_TIMEOUT_MS 500
#define FPC_MAX_HAL_PROCESSING_TIME 400

#define FPC_IOC_MAGIC 0x1145
#define FPC_IOCWPREPARE _IOW(FPC_IOC_MAGIC, 0x01, int)
#define FPC_IOCWDEVWAKE _IOW(FPC_IOC_MAGIC, 0x02, int)
#define FPC_IOCWRESET _IOW(FPC_IOC_MAGIC, 0x03, int)
#define FPC_IOCWAWAKE _IOW(FPC_IOC_MAGIC, 0x04, int)
#define FPC_IOCRPREPARE _IOR(FPC_IOC_MAGIC, 0x81, int)
#define FPC_IOCRDEVWAKE _IOR(FPC_IOC_MAGIC, 0x82, int)
#define FPC_IOCRIRQ _IOR(FPC_IOC_MAGIC, 0x83, int)
#define FPC_IOCRIRQPOLL _IOR(FPC_IOC_MAGIC, 0x84, int)
#define FPC_IOCRHWTYPE _IOR(FPC_IOC_MAGIC, 0x85, int)

static const char *const pctl_names[] = {
	"fpc1145_reset_reset", "fpc1145_reset_active", "fpc1145_irq_active",
#ifdef CONFIG_ARCH_SONY_LOIRE
	"fpc1145_ldo_enable",  "fpc1145_ldo_disable",
#endif
};

typedef enum {
	VCC_SPI = 0,
#ifndef CONFIG_ARCH_SONY_LOIRE
	VDD_ANA,
	VDD_IO,
#endif
	FPC_VREG_MAX,
} fpc_rails_t;

struct vreg_config {
	char *name;
	unsigned long vmin;
	unsigned long vmax;
	int ua_load;
	bool is_optional;
};

static const struct vreg_config vreg_conf[] = {
	{ "vcc_spi", 1800000UL, 1800000UL, 10, true },
#ifndef CONFIG_ARCH_SONY_LOIRE
	{ "vdd_ana", 1800000UL, 1800000UL, 6000, false },
	{ "vdd_io", 1800000UL, 1800000UL, 6000, true },
#endif
};

struct fpc1145_data {
	struct miscdevice misc;
	struct device *dev;
	struct pinctrl *fingerprint_pinctrl;
	struct pinctrl_state *pinctrl_state[ARRAY_SIZE(pctl_names)];
	struct regulator *vreg[ARRAY_SIZE(vreg_conf)];

	int irq_gpio;
#ifdef CONFIG_ARCH_SONY_LOIRE
	int ldo_gpio;
#endif

	int irq;
	bool irq_fired;
	wait_queue_head_t irq_evt;

	struct mutex lock;
	bool prepared;
};

struct fpc1145_awake_args {
	int awake;
	unsigned int timeout;
};

struct fpc1145_data *to_fpc1145_data(struct file *fp)
{
	struct miscdevice *md = (struct miscdevice *)fp->private_data;

	return container_of(md, struct fpc1145_data, misc);
}

static int vreg_setup(struct fpc1145_data *fpc1145, fpc_rails_t fpc_rail,
		      bool enable)
{
	int rc;
	struct regulator *vreg = fpc1145->vreg[fpc_rail];
	struct device *dev = fpc1145->dev;

	if (!vreg)
		return -EINVAL;

	if (enable) {
		if (regulator_count_voltages(vreg) > 0) {
			rc = regulator_set_voltage(vreg,
						   vreg_conf[fpc_rail].vmin,
						   vreg_conf[fpc_rail].vmax);
			if (rc)
				dev_err(dev,
					"Unable to set voltage on %s, %d\n",
					vreg_conf[fpc_rail].name, rc);
		}
		rc = regulator_set_load(vreg, vreg_conf[fpc_rail].ua_load);
		if (rc < 0)
			dev_err(dev, "Unable to set current on %s, %d\n",
				vreg_conf[fpc_rail].name, rc);
		rc = regulator_enable(vreg);
		if (rc) {
			dev_err(dev, "error enabling %s: %d\n",
				vreg_conf[fpc_rail].name, rc);
		}
	} else {
		if (regulator_is_enabled(vreg)) {
			regulator_disable(vreg);
			dev_dbg(dev, "disabled %s\n", vreg_conf[fpc_rail].name);
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

		rc = vreg_setup(fpc1145, VCC_SPI, true);
		if (rc)
			goto exit;

#ifdef CONFIG_ARCH_SONY_LOIRE
		(void)select_pin_ctl(fpc1145, "fpc1145_ldo_enable");
#else
		rc = vreg_setup(fpc1145, VDD_IO, true);
		if (rc)
			goto exit_1;

		rc = vreg_setup(fpc1145, VDD_ANA, true);
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
		(void)vreg_setup(fpc1145, VDD_ANA, false);
	exit_2:
		(void)vreg_setup(fpc1145, VDD_IO, false);
	exit_1:
#endif
		(void)vreg_setup(fpc1145, VCC_SPI, false);
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
	struct fpc1145_data *fpc1145 = to_fpc1145_data(fp);
	pm_wakeup_event(fpc1145->dev, 1);
	return 0;
}

static int fpc1145_device_release(struct inode *inode, struct file *fp)
{
	struct fpc1145_data *fpc1145 = to_fpc1145_data(fp);
	pm_relax(fpc1145->dev);
	return 0;
}

static long fpc1145_device_ioctl(struct file *fp, unsigned int cmd,
				 unsigned long arg)
{
	int8_t val = 0;
	int rc = -EINVAL;
	unsigned int timeout = 0;
	struct fpc1145_awake_args awake_args;
	void __user *usr = (void __user *)arg;
	struct fpc1145_data *fpc1145 = to_fpc1145_data(fp);

	switch (cmd) {
	case FPC_IOCWPREPARE:
		dev_dbg(fpc1145->dev, "%s device\n",
			(arg == 0 ? "Unpreparing" : "Preparing"));
		rc = device_prepare(fpc1145, !!arg);
		break;
	case FPC_IOCWDEVWAKE:
		dev_dbg(fpc1145->dev, "Setting devwake %lu\n", arg);
		dev_dbg(fpc1145->dev, "WDEVWAKE Not implemented.\n");
		break;
	case FPC_IOCWRESET:
		dev_dbg(fpc1145->dev, "Resetting device\n");
		rc = hw_reset(fpc1145);
		break;
	case FPC_IOCWAWAKE:
		rc = copy_from_user(&awake_args,
				    (struct fpc1145_awake_args *)usr,
				    sizeof(awake_args));
		if (rc)
			break;
		timeout = min(awake_args.timeout,
			      (unsigned int)FPC_MAX_HAL_PROCESSING_TIME);
		if (awake_args.awake) {
			dev_dbg(fpc1145->dev, "Extending wakelock for %dms\n",
				timeout);
			pm_wakeup_event(fpc1145->dev, timeout);
		} else {
			dev_dbg(fpc1145->dev, "Relaxing wakelock\n");
			pm_relax(fpc1145->dev);
		}
		break;
	case FPC_IOCRPREPARE:
		rc = put_user((int8_t)fpc1145->prepared, (int *)usr);
		break;
	case FPC_IOCRDEVWAKE:
		dev_dbg(fpc1145->dev, "RDEVWAKE Not implemented.\n");
		break;
	case FPC_IOCRIRQ:
		val = gpio_get_value(fpc1145->irq_gpio);
		rc = put_user(val, (int *)usr);
		break;
	case FPC_IOCRIRQPOLL:
		val = gpio_get_value(fpc1145->irq_gpio);
		if (val) {
			/* We don't need to wait: IRQ has already fired */
			rc = put_user(val, (int *)usr);
			return rc;
		}

		if (fpc1145->irq_fired) {
			fpc1145->irq_fired = false;
			enable_irq(fpc1145->irq);
		}

		rc = wait_event_interruptible_timeout(
			fpc1145->irq_evt, fpc1145->irq_fired,
			msecs_to_jiffies(FPC_IRQPOLL_TIMEOUT_MS));
		if (rc == -ERESTARTSYS)
			return rc;

		val = gpio_get_value(fpc1145->irq_gpio);
		if (val)
			pm_wakeup_event(fpc1145->dev,
					FPC_MAX_HAL_PROCESSING_TIME);

		rc = put_user(val, (int *)usr);
		break;
#ifdef CONFIG_ARCH_SONY_NILE
	case FPC_IOCRHWTYPE:
		rc = put_user((int)FP_HW_TYPE_FPC, (int *)usr);
		break;
#endif
	default:
		rc = -ENOIOCTLCMD;
		dev_err(fpc1145->dev, "Unknown IOCTL 0x%x.\n", cmd);
		break;
	}

	return rc;
}

static unsigned int fpc1145_poll_interrupt(struct file *fp,
					   struct poll_table_struct *wait)
{
	int val = 0;
	struct fpc1145_data *fpc1145 = to_fpc1145_data(fp);
	struct device *dev = fpc1145->dev;

	val = gpio_get_value(fpc1145->irq_gpio);
	if (val) {
		/* Early out */
		dev_dbg(dev, "gpio already triggered\n");
		pm_wakeup_event(dev, FPC_MAX_HAL_PROCESSING_TIME);
		return POLLIN | POLLRDNORM;
	}

	/* Add current file to the waiting list  */
	poll_wait(fp, &fpc1145->irq_evt, wait);

	/* Enable the irq */
	if (fpc1145->irq_fired) {
		fpc1145->irq_fired = false;
		enable_irq(fpc1145->irq);
	}

	val = gpio_get_value(fpc1145->irq_gpio);
	if (val) {
		dev_dbg(dev, "gpio triggered after poll_wait\n");
		pm_wakeup_event(dev, FPC_MAX_HAL_PROCESSING_TIME);
		return POLLIN | POLLRDNORM;
	}

	/*
	 * Nothing happened yet; make the poll wait for irq_evt.
	 * The wakelock can be relaxed preemptively, as no processing has to
	 * be done until the next wake-enabled IRQ fires.
	 */

	pm_relax(dev);

	return 0;
}

static int fpc1145_device_suspend(struct device *dev)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);

	dev_dbg(dev, "Suspending device\n");

	/* Wakeup when finger detected */
	enable_irq_wake(fpc1145->irq);

	return 0;
}

static int fpc1145_device_resume(struct device *dev)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);

	dev_dbg(dev, "Resuming device\n");

	disable_irq_wake(fpc1145->irq);

	return 0;
}

static const struct file_operations fpc1145_device_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.open = fpc1145_device_open,
	.release = fpc1145_device_release,
	.unlocked_ioctl = fpc1145_device_ioctl,
	.compat_ioctl = fpc1145_device_ioctl,
	.poll = fpc1145_poll_interrupt,
};

static irqreturn_t fpc1145_irq_handler(int irq, void *handle)
{
	struct fpc1145_data *fpc1145 = handle;

	int val = gpio_get_value(fpc1145->irq_gpio);

	dev_dbg(fpc1145->dev, "%s: gpio=%d\n", __func__, val);

	fpc1145->irq_fired = true;

	pm_wakeup_event(fpc1145->dev, FPC_MAX_HAL_PROCESSING_TIME);
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

static int fpc1145_get_regulators(struct fpc1145_data *fpc1145)
{
	struct device *dev = fpc1145->dev;
	unsigned short i = 0;

	for (i = 0; i < FPC_VREG_MAX; i++) {
		if (!vreg_conf[i].is_optional)
			fpc1145->vreg[i] = devm_regulator_get_optional(
				dev, vreg_conf[i].name);
		else
			fpc1145->vreg[i] =
				devm_regulator_get(dev, vreg_conf[i].name);

		if (IS_ERR_OR_NULL(fpc1145->vreg[i])) {
			fpc1145->vreg[i] = NULL;
			dev_err(dev, "CRITICAL: Cannot get %s regulator.\n",
				vreg_conf[i].name);
			return -EINVAL;
		}
	}

	return 0;
}

static int fpc1145_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int rc = 0;
	size_t i;
	int irqf;
	struct device_node *np = dev->of_node;
#ifdef CONFIG_ARCH_SONY_NILE
	int hw_type;
#endif

	struct fpc1145_data *fpc1145 =
		devm_kzalloc(dev, sizeof(*fpc1145), GFP_KERNEL);
	if (!fpc1145) {
		rc = -ENOMEM;
		goto exit;
	}

	fpc1145->misc = (struct miscdevice){
		.minor = MISC_DYNAMIC_MINOR,
		.name = "fingerprint",
		.fops = &fpc1145_device_fops,
	};

	fpc1145->dev = dev;
	platform_set_drvdata(pdev, fpc1145);
	dev_set_drvdata(fpc1145->dev, fpc1145);

	if (!np) {
		dev_err(dev, "no of node found\n");
		rc = -EINVAL;
		goto exit;
	}

	rc = fpc1145_get_regulators(fpc1145);
	if (rc)
		goto exit;

#ifdef CONFIG_ARCH_SONY_NILE
	rc = vreg_setup(fpc1145, VDD_ANA, true);
	if (rc)
		goto exit;

	hw_type = cei_fp_module_detect();

	(void)vreg_setup(fpc1145, VDD_ANA, false);

	if (hw_type != FP_HW_TYPE_FPC) {
		dev_info(dev, "FPC sensor not found, bailing out\n");
		rc = -ENODEV;
		goto exit;
	}

	dev_info(dev, "Detected FPC sensor\n");
#endif

	rc = fpc1145_request_named_gpio(fpc1145, "fpc,gpio_irq",
					&fpc1145->irq_gpio);
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

	device_init_wakeup(dev, true);
	init_waitqueue_head(&fpc1145->irq_evt);
	fpc1145->irq_fired = false;

	fpc1145->irq = gpio_to_irq(fpc1145->irq_gpio);
	rc = devm_request_threaded_irq(dev, fpc1145->irq, NULL,
				       fpc1145_irq_handler, irqf, dev_name(dev),
				       fpc1145);
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

	rc = misc_register(&fpc1145->misc);
	if (!rc)
		goto exit;

	dev_info(dev, "%s: ok\n", __func__);

exit:
	return rc;
}

static int fpc1145_remove(struct platform_device *pdev)
{
	struct fpc1145_data *fpc1145 = platform_get_drvdata(pdev);

	device_init_wakeup(fpc1145->dev, false);
	mutex_destroy(&fpc1145->lock);
#ifdef CONFIG_ARCH_SONY_LOIRE
	(void)vreg_setup(fpc1145, VCC_SPI, false);
#else
	(void)vreg_setup(fpc1145, VDD_IO, false);
	(void)vreg_setup(fpc1145, VCC_SPI, false);
	(void)vreg_setup(fpc1145, VDD_ANA, false);
#endif

	dev_info(&pdev->dev, "%s\n", __func__);
	return 0;
}

static struct of_device_id fpc1145_of_match[] = {
	{
		.compatible = "fpc,fpc1020",
	},
	{
		.compatible = "fpc,fpc1145",
	},
	{}
};
MODULE_DEVICE_TABLE(of, fpc1145_of_match);

static SIMPLE_DEV_PM_OPS(fpc1145_pm_ops, fpc1145_device_suspend,
			 fpc1145_device_resume);

static struct platform_driver fpc1145_driver = {
	.driver = {
		.name = "fpc1145",
		.owner = THIS_MODULE,
		.of_match_table = fpc1145_of_match,
		.pm = &fpc1145_pm_ops,
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
