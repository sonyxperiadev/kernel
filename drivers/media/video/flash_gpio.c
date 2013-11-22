/*
 * ocp8111 flash driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include <linux/gpio_keys.h>
#include <linux/gpio.h>
#include "flash_gpio.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>

#if defined(CONFIG_MACH_JAVA_C_LC2)
#define GPIO_FLASH_EN 11
#define GPIO_TORCH_SEL 34
#elif defined(CONFIG_MACH_JAVA_C_5606)
#define GPIO_FLASH_EN 11
#define GPIO_TORCH_SEL 10
#endif

/*
void gpio_flash_torch_on(void);
void gpio_flash_torch_off(void);
void gpio_flash_flash_on(unsigned long timeout);
void gpio_flash_flash_off(void);
*/
static struct timer_list timer;
static char *msg = "hello world";
void gpio_flash_torch_on(void)
{
	gpio_set_value(GPIO_TORCH_SEL, 0);
	gpio_set_value(GPIO_FLASH_EN, 1);
	pr_debug(KERN_ERR "gpio_flash torch on\n");
}

void gpio_flash_torch_off(void)
{
	gpio_set_value(GPIO_TORCH_SEL, 0);
	gpio_set_value(GPIO_FLASH_EN, 0);
	del_timer(&timer);
	pr_debug(KERN_ERR "gpio flash torch off\n");
}

void gpio_flash_flash_off(void)
{
	gpio_set_value(GPIO_TORCH_SEL, 0);
	gpio_set_value(GPIO_FLASH_EN, 0);
	pr_debug(KERN_ERR "gpio flash flash off\n");
}


static void timeout_func(unsigned long lparam)
{
	gpio_flash_torch_off();
}

void gpio_flash_flash_on(unsigned long timeout)
{
	timer.data = (unsigned long) msg;
	timer.expires = jiffies + (timeout * HZ)/1000000;
	timer.function = timeout_func;
	add_timer(&timer);

	gpio_set_value(GPIO_TORCH_SEL, 1);
	gpio_set_value(GPIO_FLASH_EN, 1);
	pr_debug(KERN_ERR "gpio flash flash on timeout %ld\n", timeout);
}

static int __init gpio_flash_mod_init(void)
{
	if (gpio_request_one(GPIO_FLASH_EN , GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		      "Flash-En")) {
		printk(KERN_ERR "gpio Flash-En failed\n");
		return -1;
	}
#if defined(CONFIG_MACH_JAVA_C_LC2)
	if (gpio_request_one(GPIO_TORCH_SEL , GPIOF_DIR_OUT | GPIOF_INIT_HIGH,
		      "Flash-SEL")) {
		printk(KERN_ERR "gpio Flash-SEL failed\n");
		return -1;
	}
#elif defined(CONFIG_MACH_JAVA_C_5606)
	if (gpio_request_one(GPIO_TORCH_SEL , GPIOF_DIR_OUT | GPIOF_INIT_LOW,
				  "Flash-SEL")) {
			printk(KERN_ERR "gpio Flash-SEL failed\n");
			return -1;
		}
#endif
	init_timer(&timer);
	return 0;
}

static void __exit gpio_flash_mod_exit(void)
{
	gpio_free(GPIO_TORCH_SEL);
	gpio_free(GPIO_FLASH_EN);
}

module_init(gpio_flash_mod_init);
module_exit(gpio_flash_mod_exit);

MODULE_DESCRIPTION("GPIO Flash driver");
MODULE_LICENSE("GPL v2");

