/*
     SEMC GPIO test driver

     Added in order to bypass the missing GPIO Support due to a conflict
     between MSM gpio_chip and GPIOLIB structs

*/


#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define stamp(fmt, args...) pr_debug("%s:%i: " fmt "\n", __func__, __LINE__, ## args)
#define stampit() stamp("here i am")
#define pr_devinit(fmt, args...) ({ static const __devinitdata char __fmt[] = fmt; printk(__fmt, ## args); })
#define pr_init(fmt, args...) ({ static const __initdata char __fmt[] = fmt; printk(__fmt, ## args); })

#define DRIVER_NAME "semc-atp-gpio"
#define PFX DRIVER_NAME ": "

struct gpio_data {
	atomic_t open_count;
};

struct group_data {
	dev_t dev_node;
	struct cdev cdev;
	struct resource *gpio_range;
	struct gpio_data *gpios;
};

/**
 *	semc_gpio_read :: read the value of the specified GPIO
 */
static ssize_t semc_gpio_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	unsigned int gpio = iminor(file->f_path.dentry->d_inode);
	ssize_t ret;

	stampit();

	for (ret = 0; ret < count; ++ret) {
		char byte = '0' + gpio_get_value(gpio);
		if (put_user(byte, buf + ret))
			break;
	}

	return ret;
}

/**
 *	semc_gpio_write :: modify the state of the specified GPIO
 */
static ssize_t semc_gpio_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	struct group_data *group_data = container_of(file->f_path.dentry->d_inode->i_cdev, struct group_data, cdev);
	unsigned int gpio = iminor(file->f_path.dentry->d_inode);
	struct gpio_data *gpio_data = &group_data->gpios[gpio - group_data->gpio_range->start];

	ssize_t ret;
	char dir = '?', uvalue = '?';
	int value;
	int rtn;

	stampit();

	rtn = gpio_request(gpio, DRIVER_NAME);
	if (rtn) {
		gpio_free(gpio);
		gpio_request(gpio, DRIVER_NAME);
//		return -1;
	}
	atomic_inc(&gpio_data->open_count);

	ret = 0;
	while (ret < count) {
		char byte;
		int user_ret = get_user(byte, buf + ret++);
		if (user_ret)
		{
			/* do not free until last consumer has closed */
			if (atomic_dec_and_test(&gpio_data->open_count)) gpio_free(gpio);
			return user_ret;
		}

		switch (byte) {
		case '\r':
		case '\n': continue;
		case 'I':
		case 'O': dir = byte; break;
		case 'T':
		case '0':
		case '1': uvalue = byte; break;
		default:
			/* do not free until last consumer has closed */
			if (atomic_dec_and_test(&gpio_data->open_count)) gpio_free(gpio);
			return -EINVAL;
		}
		stamp("processed byte '%c'", byte);
	}

	switch (uvalue) {
	case '?': value = gpio_get_value(gpio); break;
	case 'T': value = !gpio_get_value(gpio); break;
	default:  value = uvalue - '0'; break;
	}

	switch (dir) {
	case 'I': gpio_direction_input(gpio); break;
	case 'O': gpio_direction_output(gpio, value); break;
	}

	if (uvalue != '?')
		gpio_set_value(gpio, value);

	/* do not free until last consumer has closed */
	if (atomic_dec_and_test(&gpio_data->open_count)) gpio_free(gpio);

	return ret;
}

/**
 *	semc_gpio_open :: claim the specified GPIO
 *
 */
static int semc_gpio_open(struct inode *ino, struct file *file)
{
	struct group_data *group_data = container_of(ino->i_cdev, struct group_data, cdev);
	unsigned int gpio = iminor(ino);
/* SEMC PATCH BEGIN (sun for warning) */
//	struct gpio_data *gpio_data = &group_data->gpios[gpio - group_data->gpio_range->start];
/* SEMC PATCH END (sun for warning) */

	stampit();

	if (gpio < group_data->gpio_range->start || gpio > group_data->gpio_range->end)
		return -ENXIO;


	return 0;
}

/**
 *	semc_gpio_release - release the specified GPIO
 */
static int semc_gpio_release(struct inode *ino, struct file *file)
{
/* SEMC PATCH BEGIN (sun for warning) */
//	struct group_data *group_data = container_of(ino->i_cdev, struct group_data, cdev);
//	unsigned int gpio = iminor(ino);
//	struct gpio_data *gpio_data = &group_data->gpios[gpio - group_data->gpio_range->start];
/* SEMC PATCH END (sun for warning) */

	stampit();


	return 0;
}

static struct class *semc_gpio_class;

static struct file_operations semc_gpio_fops = {
	.owner    = THIS_MODULE,
	.read     = semc_gpio_read,
	.write    = semc_gpio_write,
	.open     = semc_gpio_open,
	.release  = semc_gpio_release,
};

/**
 *	semc_gpio_probe - setup the range of GPIOs
 */
static int __devinit semc_gpio_probe(struct platform_device *pdev)
{
	int ret;
	struct group_data *group_data;
	struct resource *gpio_range = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	int gpio, gpio_max = gpio_range->end - gpio_range->start + 1;

	stampit();

	group_data = kzalloc(sizeof(*group_data) + sizeof(struct gpio_data) * gpio_max, GFP_KERNEL);
	if (!group_data)
		return -ENOMEM;
	group_data->gpio_range = gpio_range;
	group_data->gpios = (void *)group_data + sizeof(*group_data);
	platform_set_drvdata(pdev, group_data);

	ret = alloc_chrdev_region(&group_data->dev_node, gpio_range->start, gpio_max, "gpio");
	if (ret) {
		pr_devinit(KERN_ERR PFX "unable to get a char device\n");
		kfree(group_data);
		return ret;
	}
	cdev_init(&group_data->cdev, &semc_gpio_fops);
	group_data->cdev.owner = THIS_MODULE;

	ret = cdev_add(&group_data->cdev, group_data->dev_node, gpio_max);
	if (ret) {
		pr_devinit(KERN_ERR PFX "unable to register char device\n");
		kfree(group_data);
		unregister_chrdev_region(group_data->dev_node, gpio_max);
		return ret;
	}

	for (gpio = gpio_range->start; gpio <= gpio_range->end; ++gpio)
		device_create(semc_gpio_class, &pdev->dev, group_data->dev_node + gpio,
		              NULL, "gpio%i", gpio);

	device_init_wakeup(&pdev->dev, 1);

	pr_devinit(KERN_INFO PFX "now handling %i GPIOs: %i - %i\n",
		gpio_max, gpio_range->start, gpio_range->end);

	return 0;
}

/**
 *	semc_gpio_remove - break down the range of GPIOs
 */
static int __devexit semc_gpio_remove(struct platform_device *pdev)
{
	struct group_data *group_data = platform_get_drvdata(pdev);
	struct resource *gpio_range = group_data->gpio_range;
	int gpio, gpio_max = gpio_range->end - gpio_range->start + 1;

	stampit();

	for (gpio = gpio_range->start; gpio <= gpio_range->end; ++gpio)
		device_destroy(semc_gpio_class, group_data->dev_node + gpio);

	cdev_del(&group_data->cdev);
	unregister_chrdev_region(group_data->dev_node, gpio_max);

	kfree(group_data);

	return 0;
}

struct platform_driver semc_gpio_device_driver = {
	.probe   = semc_gpio_probe,
	.remove  = __devexit_p(semc_gpio_remove),
	.driver  = {
		.name = DRIVER_NAME,
	}
};

/**
 *	semc_gpio_init - setup our GPIO device driver
 */
static int __init semc_gpio_init(void)
{
	semc_gpio_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(semc_gpio_class)) {
		pr_init(KERN_ERR PFX "unable to create gpio class\n");
		return PTR_ERR(semc_gpio_class);
	}

	return platform_driver_register(&semc_gpio_device_driver);
}
module_init(semc_gpio_init);

/**
 *	semc_gpio_exit - break down our GPIO device driver
 */
static void __exit semc_gpio_exit(void)
{
	class_destroy(semc_gpio_class);

	platform_driver_unregister(&semc_gpio_device_driver);
}
module_exit(semc_gpio_exit);

MODULE_AUTHOR("SEMC");
MODULE_DESCRIPTION("ETS GPIO test driver");
MODULE_LICENSE("GPL");
