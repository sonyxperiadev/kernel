/*
 * ADXL345/346 Three-Axis Digital Accelerometers
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (C) 2009 Michael Hennerich, Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/broadcom/gps.h>
#include <mach/pinmux.h>

#define GPS_VERSION	"1.00"

int hostwake_gpio;

struct gps_irq {
	wait_queue_head_t wait;
	int irq;
	int host_req_pin;
};

irqreturn_t gps_irq_handler(int irq, void *dev_id)
{
	struct gps_irq *ac_data = dev_id;

	wake_up_interruptible(&ac_data->wait);
	return IRQ_HANDLED;
}

static int gps_irq_open(struct inode *inode, struct file *filp)
{
	int ret;
	int irq;

	struct gps_irq *ac_data = kzalloc(sizeof(struct gps_irq), GFP_KERNEL);
	struct pin_config GPIOSetup[] = {PIN_BSC_CFG(GPIO16, BSC2CLK, 0x20),
		PIN_BSC_CFG(GPIO17, BSC2DAT, 0x20)};

	filp->private_data = ac_data;

	init_waitqueue_head(&ac_data->wait);

	/*
	 * Allocate the IRQ
	 */
	if (hostwake_gpio) {
		gpio_request(hostwake_gpio, "gps_irq");
		gpio_direction_input(hostwake_gpio);
		irq = gpio_to_irq(hostwake_gpio);
		if (irq < 0)
			return -1;
		ac_data->irq = irq;
		ac_data->host_req_pin = hostwake_gpio;

		ret = request_irq(irq, gps_irq_handler,
				  IRQF_TRIGGER_RISING, "gps_interrupt",
				  ac_data);
	} else
		return -1;

#if defined(CONFIG_MACH_RHEA_SS_AMAZING) || defined(CONFIG_MACH_RHEA_SS_LUCAS)
	pinmux_set_pin_config(&GPIOSetup[0]);
	pinmux_set_pin_config(&GPIOSetup[1]);
#endif
	return ret;
}

static int gps_irq_release(struct inode *inode, struct file *filp)
{
	struct gps_irq *ac_data = filp->private_data;
	struct pin_config GPIOSetup[] = {PIN_BSC_CFG(GPIO16, GPIO16, 0x20),
		PIN_BSC_CFG(GPIO17, GPIO17, 0x20)};

	/*
	 * Free the interrupt
	 */
	free_irq(ac_data->irq, ac_data);
	kfree(ac_data);
	filp->private_data = NULL;

#if defined(CONFIG_MACH_RHEA_SS_AMAZING) || defined(CONFIG_MACH_RHEA_SS_LUCAS)
	pinmux_set_pin_config(&GPIOSetup[0]);
	pinmux_set_pin_config(&GPIOSetup[1]);
#endif
	return 0;
}

static unsigned int gps_irq_poll(struct file *filp, poll_table * wait)
{
	struct gps_irq *ac_data = filp->private_data;

	poll_wait(filp, &ac_data->wait, wait);

	if (gpio_get_value(ac_data->host_req_pin))
		return POLLIN | POLLRDNORM;

	return 0;
}

static ssize_t gps_irq_read(struct file *filp,
			    char *buffer, size_t length, loff_t * offset)
{
	char gpio_value;
	struct gps_irq *ac_data = filp->private_data;

	gpio_value = gpio_get_value(ac_data->host_req_pin);
	put_user(gpio_value, buffer);
	return 1;
}

static const struct file_operations gps_irq_fops = {
	.open = gps_irq_open,
	.release = gps_irq_release,
	.poll = gps_irq_poll,
	.read = gps_irq_read
};

static struct miscdevice gps_irq_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gps_irq",
	.fops = &gps_irq_fops
};

static int gps_hostwake_probe(struct platform_device *pdev)
{
	struct gps_platform_data *pdata;

	pdata = pdev->dev.platform_data;
	hostwake_gpio = pdata->gpio_interrupt;
	printk(KERN_INFO "GPS I2C IRQ is %d\n", hostwake_gpio);
	return 0;
}

static struct platform_driver gps_hostwake_platform_driver = {
	.probe = gps_hostwake_probe,
	.driver = {
		   .name = "gps-hostwake",
		   .owner = THIS_MODULE,
		   },
};

static int gps_irq_init(void)
{
	printk(KERN_INFO "Generic GPS IRQ Driver v%s\n", GPS_VERSION);
	hostwake_gpio = 0;
	platform_driver_register(&gps_hostwake_platform_driver);

	return misc_register(&gps_irq_dev);
}

static void gps_irq_exit(void)
{
	platform_driver_unregister(&gps_hostwake_platform_driver);
	misc_deregister(&gps_irq_dev);
}

module_init(gps_irq_init);
module_exit(gps_irq_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for gps host wake interrupt");
