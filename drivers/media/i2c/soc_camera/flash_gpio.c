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
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include <linux/init.h>
#include <linux/timer.h>

#include "flash_lamp.h"

/* board related gpio defines */
#if defined(CONFIG_VIDEO_FLASH_GPIO_EN) && \
	defined(CONFIG_VIDEO_FLASH_GPIO_MODE)
#define GPIO_FLASH_EN CONFIG_VIDEO_FLASH_GPIO_EN
#define GPIO_MODE_SEL CONFIG_VIDEO_FLASH_GPIO_MODE
#else
#error "ENABLE/MODE pins are not defined!" \
	"Please define them in Kernel Config"
#endif

struct gpio_flash_state_s {
	enum v4l2_flash_led_mode mode;
	int duration;
	int intensity;
};
static struct gpio_flash_state_s gpio_flash_state;

static void gpio_flash_timeout_func(unsigned long lparam)
{
	gpio_set_value(GPIO_MODE_SEL, 0);
	gpio_set_value(GPIO_FLASH_EN, 0);
	gpio_flash_state.mode = V4L2_FLASH_LED_MODE_NONE;
}

/* flash lamp interface implementation */
static int gpio_flash_setup(enum v4l2_flash_led_mode mode,
		int duration_in_us, int intensity)
{
	int ret = 0;

	gpio_flash_state.mode = mode;
	gpio_flash_state.duration = duration_in_us;
	/* gpio based flash doesn't support intensity */
	gpio_flash_state.intensity = intensity;

	return ret;
}

static int gpio_flash_enable(void)
{
	int ret = 0;

	switch (gpio_flash_state.mode) {
	case V4L2_FLASH_LED_MODE_NONE:
		gpio_set_value(GPIO_MODE_SEL, 0);
		gpio_set_value(GPIO_FLASH_EN, 0);
		break;
	case V4L2_FLASH_LED_MODE_FLASH:
	case V4L2_FLASH_LED_MODE_FLASH_AUTO:
		gpio_set_value(GPIO_MODE_SEL, 1);
		gpio_set_value(GPIO_FLASH_EN, 1);
		break;
	case V4L2_FLASH_LED_MODE_TORCH:
		gpio_set_value(GPIO_MODE_SEL, 0);
	    gpio_set_value(GPIO_FLASH_EN, 1);
	    break;
	default:
	    ret = -1;
	}

	return ret;
}

static int gpio_flash_disable(void)
{
	int ret = 0;

	gpio_set_value(GPIO_MODE_SEL, 0);
	gpio_set_value(GPIO_FLASH_EN, 0);

	return ret;
}

static enum v4l2_flash_led_mode gpio_flash_get_mode(void)
{
	return gpio_flash_state.mode;
}

/* get duration associated with current mode */
static int gpio_flash_get_duration(void)
{
	return gpio_flash_state.duration;
}

static int gpio_flash_get_intensity(void)
{
	return gpio_flash_state.intensity;
}

static int gpio_flash_reset(void)
{
	int ret = 0;

	gpio_flash_state.mode = V4L2_FLASH_LED_MODE_NONE;
	gpio_flash_state.duration = 0;
	gpio_flash_state.intensity = 0;
	ret += gpio_flash_setup(gpio_flash_state.mode,
			gpio_flash_state.duration,
			gpio_flash_state.intensity);
	ret += gpio_flash_disable();

	return ret;
}

int get_flash_lamp(struct flash_lamp_s *lamp)
{
	int ret = 0;

	/* validate input */
	if (NULL == lamp)
		return -1;

	/* initialize flash lamp instance */
	lamp->name = "gpio-flash";
	lamp->get_duration = gpio_flash_get_duration;
	lamp->get_mode = gpio_flash_get_mode;
	lamp->get_intensity = gpio_flash_get_intensity;
	lamp->reset = gpio_flash_reset;
	lamp->setup = gpio_flash_setup;
	lamp->enable = gpio_flash_enable;
	lamp->disable = gpio_flash_disable;

	return ret;
}

static int __init gpio_flash_mod_init(void)
{
	if (gpio_request_one(GPIO_FLASH_EN , GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		      "Flash-En")) {
		pr_err("gpio Flash-En failed\n");
		return -1;
	}
	if (gpio_request_one(GPIO_MODE_SEL , GPIOF_DIR_OUT | GPIOF_INIT_HIGH,
		      "Flash-SEL")) {
		pr_err(KERN_ERR "gpio Flash-SEL failed\n");
		return -1;
	}
	memset(&gpio_flash_state, sizeof(struct flash_lamp_s), 0);

	return 0;
}

static void __exit gpio_flash_mod_exit(void)
{
	gpio_free(GPIO_MODE_SEL);
	gpio_free(GPIO_FLASH_EN);
}

module_init(gpio_flash_mod_init);
module_exit(gpio_flash_mod_exit);

MODULE_DESCRIPTION("GPIO Flash driver");
MODULE_LICENSE("GPL v2");

