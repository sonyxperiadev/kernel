/* [kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform_base.c]
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Hirokuni Kawasaki <hirokuni.kawaaki@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/delay.h>

#include "mhl_common.h"
#include "mhl_platform.h"

#define MHL_DRIVER_NAME "sii8620"
#define COMPATIBLE_NAME "qcom,mhl-sii8620"

/*gpio*/
static int int_gpio;
static int pwr_gpio;
static int rst_gpio;
static int switch_sel_1_gpio;
static int switch_sel_2_gpio;
static int fw_wake_gpio;

/*irq*/
static int irq_number;

static struct usb_ext_notification *mhl_info;
static bool isDiscoveryCalled;
static void (*notify_usb_online)(void *ctx, int on);
static void *context_cb;
static int (*device_discovery_cb)(void *context_cb);
static void *usb_ctx;
static struct i2c_client *mhl_i2c_client;
static struct clk *mhl_clk;

enum gpio_direction_types {
	GPIO_OUTPUT,
	GPIO_INPUT
};

extern int qpnp_chg_notify_mhl_state(int state);

static bool mhl_pf_is_switch_to_usb(void)
{
	if ((gpio_get_value(GPIO_MHL_SWITCH_SEL_1) == 0) &&
		(gpio_get_value(GPIO_MHL_SWITCH_SEL_2) == 0))
		return true;
	else
		return false;
}

int mhl_pf_get_gpio_num_int(void)
{
	return int_gpio;
}
EXPORT_SYMBOL(mhl_pf_get_gpio_num_int);

int mhl_pf_get_gpio_num_pwr(void)
{
	return pwr_gpio;
}
EXPORT_SYMBOL(mhl_pf_get_gpio_num_pwr);

int mhl_pf_get_gpio_num_rst(void)
{
	return rst_gpio;
}
EXPORT_SYMBOL(mhl_pf_get_gpio_num_rst);

int mhl_pf_get_gpio_num_fw_wake(void)
{
	return fw_wake_gpio;
}
EXPORT_SYMBOL(mhl_pf_get_gpio_num_fw_wake);

int mhl_pf_get_irq_number(void)
{
	pr_debug("%s:irq_number:%d\n", __func__, irq_number);
	return irq_number;
}
EXPORT_SYMBOL(mhl_pf_get_irq_number);

struct i2c_client *mhl_pf_get_i2c_client(void)
{
	return mhl_i2c_client;
}
EXPORT_SYMBOL(mhl_pf_get_i2c_client);

struct clk *mhl_pf_get_mhl_clk(void)
{
	return mhl_clk;
}
EXPORT_SYMBOL(mhl_pf_get_mhl_clk);


/**
 * mhl_pf_switch_to_usb: switch to usb.
 * This API must be called after the INT by PMIC.
 */
int mhl_pf_switch_to_usb(void)
{
	pr_debug("%s:", __func__);
	isDiscoveryCalled = false;

	if (mhl_pf_is_switch_to_usb())
		return MHL_SUCCESS;

	/* switch gpio to USB from MHL */
	/* todo : must use dtsi for gpio */
	gpio_set_value(GPIO_MHL_SWITCH_SEL_1, 0);
	gpio_set_value(GPIO_MHL_SWITCH_SEL_2, 0);
	pr_debug("%s: gpio(%d) : %d", __func__,
			 GPIO_MHL_SWITCH_SEL_1,
			 gpio_get_value(GPIO_MHL_SWITCH_SEL_1));
	pr_debug("%s: gpio(%d) : %d", __func__,
			 GPIO_MHL_SWITCH_SEL_2,
			 gpio_get_value(GPIO_MHL_SWITCH_SEL_2));

	msleep(20);

	if (!notify_usb_online) {
		pr_warn("%s: no notify_usb_online registration\n", __func__);
		/*
		 * This API may be called after mhl_pf_switch_device_discovery
		 * is called by USB, otherwise the "online" notification can't
		 * be sent to USB via the "notify_usb_online".
		 * Thought the notification doesn't occur, the GPIO will be
		 * somehow switched to USB.
		 * (USB is default. So, the system will try to switch to USB.)
		 */
		/*return MHL_FAIL;*/
	} else {
		notify_usb_online(usb_ctx, 0);
	}

	return MHL_SUCCESS;
}
EXPORT_SYMBOL(mhl_pf_switch_to_usb);

/**
 * mhl_pf_switch_register_cb: register
 * a call back for notifying device discovery.
 * The client should start device discovery
 * when call the callback and must notify the
 * result. Only one registration is acceptable.
 * When the register API is called before unregister,
 * the previous call back and context will be
 * replace with new registration.
 *
 * @device_discovery - call back API. Must not be NULL.
 * @context - it can be NULL. It is notified through the call back API.
 *
 */
void mhl_pf_switch_register_cb(int (*device_discovery)(void *context),
								void *context)
{
	int rc = MHL_USB_NON_INUSE;

	pr_debug("%s:\n", __func__);
	device_discovery_cb = device_discovery;
	context_cb = context;
	if (isDiscoveryCalled) {
		rc = device_discovery_cb(context_cb);
		if (rc == MHL_USB_INUSE) {
			if (notify_usb_online) {
				/* Found MHL device */
				qpnp_chg_notify_mhl_state(1);
				notify_usb_online(usb_ctx, 1);
			}
		}
	}

}
EXPORT_SYMBOL(mhl_pf_switch_register_cb);

/**
 * mhl_pf_switch_unregister_cb: unregister
 * the registerred call back and stored context is
 * unregisterred either.
 */
void mhl_pf_switch_unregister_cb(void)
{
	device_discovery_cb = NULL;
	context_cb = NULL;
}
EXPORT_SYMBOL(mhl_pf_switch_unregister_cb);

static void mhl_pf_swtich_resource_free(void)
{
	kfree(mhl_info);
}

static int mhl_pf_switch_device_discovery(void *data,
					int id,
					void (*usb_notify_cb)(void *, int),
					void *ctx)
{
	int rc = MHL_USB_NON_INUSE;
	pr_info("%s()\n", __func__);

	if (id) {
		/* todo : this logic can be reused in 8620? */
		pr_warn("%s: USB ID pin high. id=%d\n", __func__, id);
		return id;
	}

	if (!usb_notify_cb) {
		pr_warn("%s: cb || ctrl is NULL\n", __func__);
		return -EINVAL;
	}

	isDiscoveryCalled = true;
	usb_ctx = ctx;

	/* switch gpio to MHL from USB */
	/* todo : must use dtsi for gpio */
	gpio_set_value(GPIO_MHL_SWITCH_SEL_1, 1);
	gpio_set_value(GPIO_MHL_SWITCH_SEL_2, 1);
	pr_debug("%s: gpio(%d) : %d", __func__,
			 GPIO_MHL_SWITCH_SEL_1,
			 gpio_get_value(GPIO_MHL_SWITCH_SEL_1));
	pr_debug("%s: gpio(%d) : %d", __func__,
			 GPIO_MHL_SWITCH_SEL_2,
			 gpio_get_value(GPIO_MHL_SWITCH_SEL_2));

	msleep(20);
	if (!notify_usb_online)
		notify_usb_online = usb_notify_cb;
	if (device_discovery_cb) {
		rc = device_discovery_cb(context_cb);
		if (rc == MHL_USB_INUSE) {
			if (notify_usb_online) {
				/* Found MHL device */
				qpnp_chg_notify_mhl_state(1);
				notify_usb_online(usb_ctx, 1);
			}
		}
	} else {
		/*
		 * Even if there is no registerred call back,
		 * MHL_USB_INUSE must be returned since the
		 * MHL ko object is supposed to be installed
		 * at boot timing and immediately registerring
		 * its call back. So this API must return INUSE
		 * to usb.
		 */
		pr_warn("%s: no registerred cb.\n", __func__);
		rc = MHL_USB_INUSE;
	}

	pr_debug("%s: mhl is inuse ? : %d\n", __func__, (rc == MHL_USB_INUSE));


	return rc;
}

static int mhl_set_gpio(const char *gpio_name, int gpio_number,
		enum gpio_direction_types direction, int out_val)
{
	int ret = -EBUSY;
	pr_debug("%s()\n", __func__);
	pr_debug("%s:%s=[%d]\n", __func__, gpio_name, gpio_number);

	if (gpio_number < 0)
		return -EINVAL;


	if (gpio_is_valid(gpio_number)) {
		ret = gpio_request((unsigned int)gpio_number, gpio_name);
		if (ret < 0) {
			pr_err("%s:%s=[%d] req failed:%d\n",
				__func__, gpio_name, gpio_number, ret);
			return -EBUSY;
		}
		if (direction == GPIO_OUTPUT) {
			pr_debug("%s:gpio output\n", __func__);
			ret = gpio_direction_output(
				(unsigned int)gpio_number, out_val);
		} else if (direction == GPIO_INPUT) {
			pr_debug("%s:gpio input\n", __func__);
			ret = gpio_direction_input((unsigned int)gpio_number);
		} else {
			pr_err("%s:%s=[%d] invalid direction type :%d\n",
				__func__, gpio_name, gpio_number, ret);
			return -EINVAL;
		}
		if (ret < 0) {
			pr_err("%s: set dirn %s failed: %d\n",
				__func__, gpio_name, ret);
			return -EBUSY;
		}
	}

	return 0;
}

static void mhl_pf_gpio_config_release(void)
{
	if (int_gpio >= 0)
		gpio_free((unsigned int)int_gpio);
	if (pwr_gpio >= 0)
		gpio_free((unsigned int)pwr_gpio);
	if (rst_gpio >= 0)
		gpio_free((unsigned int)rst_gpio);
	if (switch_sel_1_gpio >= 0)
		gpio_free((unsigned int)switch_sel_1_gpio);
	if (switch_sel_2_gpio >= 0)
		gpio_free((unsigned int)switch_sel_2_gpio);
	if (fw_wake_gpio >= 0)
		gpio_free((unsigned int)fw_wake_gpio);
}

static int mhl_pf_gpio_config_init(void)
{
	int res = -1;
	pr_debug("%s()\n", __func__);

	/* reset */
	res = mhl_set_gpio("mhl-rst-gpio", rst_gpio, GPIO_OUTPUT, 0);
	if (res)
		goto error;

	/* interrupt */
	res = mhl_set_gpio("mhl-intr-gpio", int_gpio, GPIO_INPUT, 0);
	if (res)
		goto error;
	irq_number = gpio_to_irq(int_gpio);
	pr_debug("%s:irq_number:%d\n", __func__, irq_number);

	/* fw wake */
	res = mhl_set_gpio("mhl-fw-wake-gpio", fw_wake_gpio, GPIO_OUTPUT, 0);
	if (res)
		goto error;

	/* power */
	res = mhl_set_gpio("mhl-pwr-gpio", pwr_gpio, GPIO_OUTPUT, 0);
	if (res)
		goto error;

	/* switch sel 1 */
	res = mhl_set_gpio("mhl-switch-sel-1-gpio",
						switch_sel_1_gpio,
						GPIO_OUTPUT, 0);
	if (res)
		goto error;

	/* switch sel 2 */
	res = mhl_set_gpio("mhl-switch-sel-2-gpio",
			switch_sel_2_gpio, GPIO_OUTPUT, 0);
	if (res)
		goto error;

	return 0;

error:
	mhl_pf_gpio_config_release();
	return res;
}

static int get_gpios_from_device_tree(struct device_node *of_node)
{
	pr_debug("%s()\n", __func__);

	/* INTERRUPT */
	int_gpio = of_get_named_gpio(of_node, "mhl-intr-gpio", 0);
	if (int_gpio < 0) {
		pr_err("%s: Can't get mhl-intr-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():int_gpio:%d\n", __func__, int_gpio);

	/* 1.0V Power */
	pwr_gpio = of_get_named_gpio(of_node, "mhl-pwr-gpio", 0);
	if (pwr_gpio < 0) {
		pr_err("%s: Can't get mhl-pwr-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():pwr_gpio:%d\n", __func__, pwr_gpio);

	/* RESET */
	rst_gpio = of_get_named_gpio(of_node, "mhl-rst-gpio", 0);
	if (rst_gpio < 0) {
		pr_err("%s: Can't get mhl-rst-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():rst_gpio:%d\n", __func__, rst_gpio);

	/* USB/MHL switch (SEL1) */
	switch_sel_1_gpio = of_get_named_gpio(of_node,
				 "mhl-switch-sel-1-gpio", 0);
	if (switch_sel_1_gpio < 0) {
		pr_err("%s: Can't get mhl-switch-sel-1-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():switch_sel_1_gpio:%d\n", __func__, switch_sel_1_gpio);

	/* USB/MHL switch (SEL2) */
	switch_sel_2_gpio = of_get_named_gpio(of_node,
				 "mhl-switch-sel-2-gpio", 0);
	if (switch_sel_2_gpio < 0) {
		pr_err("%s: Can't get mhl-switch-sel-2-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():switch_sel_2_gpio:%d\n", __func__, switch_sel_2_gpio);

	/* FW WAKE */
	fw_wake_gpio = of_get_named_gpio(of_node, "mhl-fw-wake-gpio", 0);
	if (fw_wake_gpio < 0) {
		pr_err("%s: Can't get mhl-fw-wake-gpio\n", __func__);
		goto error;
	}
	pr_debug("%s():fw_wake_gpio:%d\n", __func__, fw_wake_gpio);
	return 0;

error:
	pr_err("%s: ret due to err\n", __func__);
	return -EBUSY;
}

static int mhl_tx_get_dt_data(struct device *dev_)
{
	int rc = 0;
	struct device_node *of_node = NULL;
	struct platform_device *op = NULL;

	of_node = dev_->of_node;
	if (!of_node) {
		pr_err("%s: invalid of_node\n", __func__);
		goto error;
	}

	op = of_find_device_by_node(of_node->parent);
	if (!op) {
		pr_err("%s: invalid op\n", __func__);
		goto error;
	}

	mhl_clk = clk_get(&op->dev, "");
	if (!mhl_clk) {
		pr_err("%s: invalid clk\n", __func__);
		goto error;
	}

	rc = get_gpios_from_device_tree(of_node);
	if (rc) {
		pr_err("%s: error gpio init\n", __func__);
		goto error;
	}

	return 0;
error:
	pr_err("%s: ret due to err\n", __func__);
	return rc;
} /* mhl_tx_get_dt_data */

static int mhl_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int rc = 0;

	pr_debug("%s:\n", __func__);

	mhl_info = NULL;
	isDiscoveryCalled = false;
	notify_usb_online = NULL;

	device_discovery_cb = NULL;
	context_cb = NULL;

	mhl_i2c_client = client;

	mhl_info = kzalloc(sizeof(*mhl_info), GFP_KERNEL);
	if (!mhl_info) {
		pr_err("%s: FAILED: cannot alloc platform_switch\n", __func__);
		rc = -ENOMEM;
		goto failed_probe;
	}

	if (client->dev.of_node) {
		rc = mhl_tx_get_dt_data(&client->dev);
		if (rc) {
			pr_err("%s:device tree error\n", __func__);
			goto failed_probe;
		}
		rc = mhl_pf_gpio_config_init();
		if (rc) {
			pr_err("%s:device tree error\n", __func__);
			goto failed_probe;
		}
	} else
		pr_err("%s: dev.of_node is null\n", __func__);

	mhl_info->notify = mhl_pf_switch_device_discovery;

	if (msm_register_usb_ext_notification(mhl_info) != 0) {
		pr_err("%s: register for usb notifcn failed\n", __func__);
		rc = -EPROBE_DEFER;
		goto failed_probe;
	}

	return 0;

failed_probe:
	mhl_pf_swtich_resource_free();

	return rc;
}

static int mhl_i2c_remove(struct i2c_client *client)
{
	pr_debug("%s:\n", __func__);

	/*
	 * All gpio will be released. All release needing gpio
	 * must be released before this API call
	 */
	mhl_pf_gpio_config_release();

	mhl_pf_swtich_resource_free();

	return 0;
}

static struct i2c_device_id mhl_sii_i2c_id[] = {
	{ MHL_DRIVER_NAME, 0 },
	{}
};

static const struct of_device_id mhl_match_table[] = {
	{.compatible = COMPATIBLE_NAME},
	{}
};

static struct i2c_driver mhl_sii_i2c_driver = {
	.driver = {
		.name = MHL_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = mhl_match_table,
	},
	.probe = mhl_i2c_probe,
	.remove =  mhl_i2c_remove,
	.id_table = mhl_sii_i2c_id,
};

module_i2c_driver(mhl_sii_i2c_driver);
MODULE_LICENSE("GPL");

