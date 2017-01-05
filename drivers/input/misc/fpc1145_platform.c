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
 * The driver will expose most of its available functionality in sysfs which
 * enables dynamic control of these features from eg. a user space process.
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

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>

#define FPC1145_RESET_LOW_US 1000
#define FPC1145_RESET_HIGH1_US 100
#define FPC1145_RESET_HIGH2_US 1250
#define PWR_ON_STEP_SLEEP 100
#define PWR_ON_STEP_RANGE1 100
#define PWR_ON_STEP_RANGE2 900
#define NUM_PARAMS_REG_ENABLE_SET 2
#define FPC_SYMLINK "fpc1145_device"

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
	struct mutex lock;
	bool prepared;
};

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
		rc = regulator_set_optimum_mode(vreg, vreg_conf[i].ua_load);
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

static ssize_t pinctl_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);
	int rc = select_pin_ctl(fpc1145, buf);

	return rc ? rc : count;
}
static DEVICE_ATTR(pinctl_set, S_IWUSR, NULL, pinctl_set);

static ssize_t regulator_enable_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);
	char op;
	char name[16];
	int rc;
	bool enable;

	if (NUM_PARAMS_REG_ENABLE_SET != sscanf(buf, "%15s,%c", name, &op))
		return -EINVAL;
	if (op == 'e')
		enable = true;
	else if (op == 'd')
		enable = false;
	else
		return -EINVAL;
	rc = vreg_setup(fpc1145, name, enable);
	return rc ? rc : count;
}
static DEVICE_ATTR(regulator_enable, S_IWUSR, NULL, regulator_enable_set);

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

static ssize_t hw_reset_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);

	if (!strncmp(buf, "reset", strlen("reset")))
		rc = hw_reset(fpc1145);
	else
		return -EINVAL;
	return rc ? rc : count;
}
static DEVICE_ATTR(hw_reset, S_IWUSR, NULL, hw_reset_set);

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

static ssize_t device_prepare_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;
	struct fpc1145_data *fpc1145 = dev_get_drvdata(dev);

	if (!strncmp(buf, "enable", strlen("enable")))
		rc = device_prepare(fpc1145, true);
	else if (!strncmp(buf, "disable", strlen("disable")))
		rc = device_prepare(fpc1145, false);
	else
		return -EINVAL;
	return rc ? rc : count;
}
static DEVICE_ATTR(spi_prepare, S_IWUSR, NULL, device_prepare_set);

/**
 * sysfs node for controlling whether the driver is allowed
 * to wake up the platform on interrupt.
 */
static ssize_t wakeup_enable_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}
static DEVICE_ATTR(wakeup_enable, S_IWUSR, NULL, wakeup_enable_set);


/**
 * sysf node to check the interrupt status of the sensor, the interrupt
 * handler should perform sysf_notify to allow userland to poll the node.
 */
static ssize_t irq_get(struct device *device,
	struct device_attribute *attribute,
	char *buffer)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(device);
	int irq = gpio_get_value(fpc1145->irq_gpio);

	return scnprintf(buffer, PAGE_SIZE, "%i\n", irq);
}


/**
 * writing to the irq node will just drop a printk message
 * and return success, used for latency measurement.
 */
static ssize_t irq_ack(struct device *device,
	struct device_attribute *attribute,
	const char *buffer, size_t count)
{
	struct fpc1145_data *fpc1145 = dev_get_drvdata(device);

	dev_dbg(fpc1145->dev, "%s\n", __func__);
	return count;
}

static DEVICE_ATTR(irq, S_IRUSR | S_IWUSR, irq_get, irq_ack);

static struct attribute *attributes[] = {
	&dev_attr_pinctl_set.attr,
	&dev_attr_spi_prepare.attr,
	&dev_attr_regulator_enable.attr,
	&dev_attr_hw_reset.attr,
	&dev_attr_wakeup_enable.attr,
	&dev_attr_irq.attr,
	NULL
};

static const struct attribute_group attribute_group = {
	.attrs = attributes,
};

static irqreturn_t fpc1145_irq_handler(int irq, void *handle)
{
	struct fpc1145_data *fpc1145 = handle;

	dev_dbg(fpc1145->dev, "%s\n", __func__);

	sysfs_notify(&fpc1145->dev->kobj, NULL, dev_attr_irq.attr.name);

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

	irqf = IRQF_TRIGGER_RISING | IRQF_ONESHOT;
	mutex_init(&fpc1145->lock);
	rc = devm_request_threaded_irq(dev, gpio_to_irq(fpc1145->irq_gpio),
			NULL, fpc1145_irq_handler, irqf,
			dev_name(dev), fpc1145);
	if (rc) {
		dev_err(dev, "could not request irq %d\n",
				gpio_to_irq(fpc1145->irq_gpio));
		goto exit;
	}
	dev_dbg(dev, "requested irq %d\n", gpio_to_irq(fpc1145->irq_gpio));

	rc = sysfs_create_group(&dev->kobj, &attribute_group);
	if (rc) {
		dev_err(dev, "could not create sysfs\n");
		goto exit;
	}

	rc = sysfs_create_link(&dev->parent->kobj, &dev->kobj, FPC_SYMLINK);
	if (rc) {
		dev_err(dev, "could not add symlink\n");
		goto exit;
	}

	if (of_property_read_bool(dev->of_node, "fpc,enable-on-boot")) {
		dev_info(dev, "Enabling hardware\n");
		(void)device_prepare(fpc1145, true);
	}

	dev_info(dev, "%s: ok\n", __func__);
exit:
	return rc;
}

static int fpc1145_remove(struct platform_device *pdev)
{
	struct fpc1145_data *fpc1145 = platform_get_drvdata(pdev);

	sysfs_remove_link(&pdev->dev.parent->kobj, FPC_SYMLINK);
	sysfs_remove_group(&pdev->dev.kobj, &attribute_group);
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
