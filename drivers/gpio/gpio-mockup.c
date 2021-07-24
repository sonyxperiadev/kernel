// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO Testing Device Driver
 *
 * Copyright (C) 2014  Kamlakant Patel <kamlakant.patel@broadcom.com>
 * Copyright (C) 2015-2016  Bamvor Jian Zhang <bamv2005@gmail.com>
 * Copyright (C) 2017 Bartosz Golaszewski <brgl@bgdev.pl>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irq_sim.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#include "gpiolib.h"

#define GPIO_MOCKUP_NAME	"gpio-mockup"
#define GPIO_MOCKUP_MAX_GC	10
/*
 * We're storing two values per chip: the GPIO base and the number
 * of GPIO lines.
 */
#define GPIO_MOCKUP_MAX_RANGES	(GPIO_MOCKUP_MAX_GC * 2)

#define gpio_mockup_err(...)	pr_err(GPIO_MOCKUP_NAME ": " __VA_ARGS__)

enum {
	GPIO_MOCKUP_DIR_IN = 0,
	GPIO_MOCKUP_DIR_OUT = 1,
};

/*
 * struct gpio_pin_status - structure describing a GPIO status
 * @dir:       Configures direction of gpio as "in" or "out", 0=in, 1=out
 * @value:     Configures status of the gpio as 0(low) or 1(high)
 */
struct gpio_mockup_line_status {
	int dir;
	int value;
};

struct gpio_mockup_chip {
	struct gpio_chip gc;
	struct gpio_mockup_line_status *lines;
	struct irq_sim irqsim;
	struct dentry *dbg_dir;
};

struct gpio_mockup_dbgfs_private {
	struct gpio_mockup_chip *chip;
	struct gpio_desc *desc;
	int offset;
};

struct gpio_mockup_platform_data {
	int base;
	int ngpio;
	int index;
	bool named_lines;
};

static int gpio_mockup_ranges[GPIO_MOCKUP_MAX_RANGES];
static int gpio_mockup_num_ranges;
module_param_array(gpio_mockup_ranges, int, &gpio_mockup_num_ranges, 0400);

static bool gpio_mockup_named_lines;
module_param_named(gpio_mockup_named_lines,
		   gpio_mockup_named_lines, bool, 0400);

static struct dentry *gpio_mockup_dbg_dir;

static int gpio_mockup_range_base(unsigned int index)
{
	return gpio_mockup_ranges[index * 2];
}

static int gpio_mockup_range_ngpio(unsigned int index)
{
	return gpio_mockup_ranges[index * 2 + 1];
}

static int gpio_mockup_get(struct gpio_chip *gc, unsigned int offset)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	return chip->lines[offset].value;
}

static void gpio_mockup_set(struct gpio_chip *gc,
			    unsigned int offset, int value)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	chip->lines[offset].value = !!value;
}

static void gpio_mockup_set_multiple(struct gpio_chip *gc,
				     unsigned long *mask, unsigned long *bits)
{
	unsigned int bit;

	for_each_set_bit(bit, mask, gc->ngpio)
		gpio_mockup_set(gc, bit, test_bit(bit, bits));

}

static int gpio_mockup_dirout(struct gpio_chip *gc,
			      unsigned int offset, int value)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	gpio_mockup_set(gc, offset, value);
	chip->lines[offset].dir = GPIO_MOCKUP_DIR_OUT;

	return 0;
}

static int gpio_mockup_dirin(struct gpio_chip *gc, unsigned int offset)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	chip->lines[offset].dir = GPIO_MOCKUP_DIR_IN;

	return 0;
}

static int gpio_mockup_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	return !chip->lines[offset].dir;
}

static int gpio_mockup_to_irq(struct gpio_chip *gc, unsigned int offset)
{
	struct gpio_mockup_chip *chip = gpiochip_get_data(gc);

	return irq_sim_irqnum(&chip->irqsim, offset);
}

static ssize_t gpio_mockup_event_write(struct file *file,
				       const char __user *usr_buf,
				       size_t size, loff_t *ppos)
{
	struct gpio_mockup_dbgfs_private *priv;
	struct gpio_mockup_chip *chip;
	struct seq_file *sfile;
	struct gpio_desc *desc;
	int rv, val;

	rv = kstrtoint_from_user(usr_buf, size, 0, &val);
	if (rv)
		return rv;
	if (val != 0 && val != 1)
		return -EINVAL;

	sfile = file->private_data;
	priv = sfile->private;
	desc = priv->desc;
	chip = priv->chip;

	gpiod_set_value_cansleep(desc, val);
	irq_sim_fire(&chip->irqsim, priv->offset);

	return size;
}

static int gpio_mockup_event_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations gpio_mockup_event_ops = {
	.owner = THIS_MODULE,
	.open = gpio_mockup_event_open,
	.write = gpio_mockup_event_write,
	.llseek = no_llseek,
};

static void gpio_mockup_debugfs_setup(struct device *dev,
				      struct gpio_mockup_chip *chip)
{
	struct gpio_mockup_dbgfs_private *priv;
	struct dentry *evfile, *link;
	struct gpio_chip *gc;
	const char *devname;
	char *name;
	int i;

	gc = &chip->gc;
	devname = dev_name(&gc->gpiodev->dev);

	chip->dbg_dir = debugfs_create_dir(devname, gpio_mockup_dbg_dir);
	if (IS_ERR_OR_NULL(chip->dbg_dir))
		goto err;

	link = debugfs_create_symlink(gc->label, gpio_mockup_dbg_dir, devname);
	if (IS_ERR_OR_NULL(link))
		goto err;

	for (i = 0; i < gc->ngpio; i++) {
		name = devm_kasprintf(dev, GFP_KERNEL, "%d", i);
		if (!name)
			goto err;

		priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
		if (!priv)
			goto err;

		priv->chip = chip;
		priv->offset = i;
		priv->desc = &gc->gpiodev->descs[i];

		evfile = debugfs_create_file(name, 0200, chip->dbg_dir, priv,
					     &gpio_mockup_event_ops);
		if (IS_ERR_OR_NULL(evfile))
			goto err;
	}

	return;

err:
	dev_err(dev, "error creating debugfs event files\n");
}

static int gpio_mockup_name_lines(struct device *dev,
				  struct gpio_mockup_chip *chip)
{
	struct gpio_chip *gc = &chip->gc;
	char **names;
	int i;

	names = devm_kcalloc(dev, gc->ngpio, sizeof(char *), GFP_KERNEL);
	if (!names)
		return -ENOMEM;

	for (i = 0; i < gc->ngpio; i++) {
		names[i] = devm_kasprintf(dev, GFP_KERNEL,
					  "%s-%d", gc->label, i);
		if (!names[i])
			return -ENOMEM;
	}

	gc->names = (const char *const *)names;

	return 0;
}

static int gpio_mockup_probe(struct platform_device *pdev)
{
	struct gpio_mockup_platform_data *pdata;
	struct gpio_mockup_chip *chip;
	struct gpio_chip *gc;
	int rv, base, ngpio;
	struct device *dev;
	char *name;

	dev = &pdev->dev;
	pdata = dev_get_platdata(dev);
	base = pdata->base;
	ngpio = pdata->ngpio;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	name = devm_kasprintf(dev, GFP_KERNEL, "%s-%c",
			      pdev->name, pdata->index);
	if (!name)
		return -ENOMEM;

	gc = &chip->gc;
	gc->base = base;
	gc->ngpio = ngpio;
	gc->label = name;
	gc->owner = THIS_MODULE;
	gc->parent = dev;
	gc->get = gpio_mockup_get;
	gc->set = gpio_mockup_set;
	gc->set_multiple = gpio_mockup_set_multiple;
	gc->direction_output = gpio_mockup_dirout;
	gc->direction_input = gpio_mockup_dirin;
	gc->get_direction = gpio_mockup_get_direction;
	gc->to_irq = gpio_mockup_to_irq;

	chip->lines = devm_kcalloc(dev, gc->ngpio,
				   sizeof(*chip->lines), GFP_KERNEL);
	if (!chip->lines)
		return -ENOMEM;

	if (pdata->named_lines) {
		rv = gpio_mockup_name_lines(dev, chip);
		if (rv)
			return rv;
	}

	rv = devm_irq_sim_init(dev, &chip->irqsim, gc->ngpio);
	if (rv < 0)
		return rv;

	rv = devm_gpiochip_add_data(dev, &chip->gc, chip);
	if (rv)
		return rv;

	if (!IS_ERR_OR_NULL(gpio_mockup_dbg_dir))
		gpio_mockup_debugfs_setup(dev, chip);

	return 0;
}

static struct platform_driver gpio_mockup_driver = {
	.driver = {
		.name = GPIO_MOCKUP_NAME,
	},
	.probe = gpio_mockup_probe,
};

static struct platform_device *gpio_mockup_pdevs[GPIO_MOCKUP_MAX_GC];

static void gpio_mockup_unregister_pdevs(void)
{
	struct platform_device *pdev;
	int i;

	for (i = 0; i < GPIO_MOCKUP_MAX_GC; i++) {
		pdev = gpio_mockup_pdevs[i];

		if (pdev)
			platform_device_unregister(pdev);
	}
}

static int __init gpio_mockup_init(void)
{
	int i, num_chips, err = 0, index = 'A';
	struct gpio_mockup_platform_data pdata;
	struct platform_device *pdev;

	if ((gpio_mockup_num_ranges < 2) ||
	    (gpio_mockup_num_ranges % 2) ||
	    (gpio_mockup_num_ranges > GPIO_MOCKUP_MAX_RANGES))
		return -EINVAL;

	/* Each chip is described by two values. */
	num_chips = gpio_mockup_num_ranges / 2;

	/*
	 * The second value in the <base GPIO - number of GPIOS> pair must
	 * always be greater than 0.
	 */
	for (i = 0; i < num_chips; i++) {
		if (gpio_mockup_range_ngpio(i) < 0)
			return -EINVAL;
	}

	gpio_mockup_dbg_dir = debugfs_create_dir("gpio-mockup-event", NULL);
	if (IS_ERR_OR_NULL(gpio_mockup_dbg_dir))
		gpio_mockup_err("error creating debugfs directory\n");

	err = platform_driver_register(&gpio_mockup_driver);
	if (err) {
		gpio_mockup_err("error registering platform driver\n");
		debugfs_remove_recursive(gpio_mockup_dbg_dir);
		return err;
	}

	for (i = 0; i < num_chips; i++) {
		pdata.index = index++;
		pdata.base = gpio_mockup_range_base(i);
		pdata.ngpio = pdata.base < 0
				? gpio_mockup_range_ngpio(i)
				: gpio_mockup_range_ngpio(i) - pdata.base;
		pdata.named_lines = gpio_mockup_named_lines;

		pdev = platform_device_register_resndata(NULL,
							 GPIO_MOCKUP_NAME,
							 i, NULL, 0, &pdata,
							 sizeof(pdata));
		if (IS_ERR(pdev)) {
			gpio_mockup_err("error registering device");
			platform_driver_unregister(&gpio_mockup_driver);
			gpio_mockup_unregister_pdevs();
			debugfs_remove_recursive(gpio_mockup_dbg_dir);
			return PTR_ERR(pdev);
		}

		gpio_mockup_pdevs[i] = pdev;
	}

	return 0;
}

static void __exit gpio_mockup_exit(void)
{
	debugfs_remove_recursive(gpio_mockup_dbg_dir);
	platform_driver_unregister(&gpio_mockup_driver);
	gpio_mockup_unregister_pdevs();
}

module_init(gpio_mockup_init);
module_exit(gpio_mockup_exit);

MODULE_AUTHOR("Kamlakant Patel <kamlakant.patel@broadcom.com>");
MODULE_AUTHOR("Bamvor Jian Zhang <bamv2005@gmail.com>");
MODULE_AUTHOR("Bartosz Golaszewski <brgl@bgdev.pl>");
MODULE_DESCRIPTION("GPIO Testing driver");
MODULE_LICENSE("GPL v2");
