/*
 * Bluetooth Broadcomm  and low power control via GPIO
 *
 *  Copyright (C) 2011 Google, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/slab.h>
#ifdef CONFIG_BT_MSM_SLEEP
#include <net/bluetooth/bluesleep.h>
#endif

#define D_BCM_BLUETOOTH_CONFIG_MATCH_TABLE   "bcm,bcm43xx"

struct bcm43xx_data {
	struct device *dev;
	struct platform_device *pdev;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
	unsigned int reg_on_gpio;
	bool has_pinctl;
};

static struct bcm43xx_data *bcm43xx_my_data;
static struct rfkill *bt_rfkill;
static bool bt_enabled;

static int bcm43xx_bt_rfkill_set_power(void *data, bool blocked)
{
	int regOnGpio;

	pr_debug("Bluetooth device set power\n");

	regOnGpio = gpio_get_value(bcm43xx_my_data->reg_on_gpio);

	/* rfkill_ops callback. Turn transmitter on when blocked is false */
	if (!blocked) {
		if (regOnGpio) {
			pr_debug("Bluetooth device is already power on:%d\n",
				regOnGpio);
			return 0;
		}
		gpio_set_value(bcm43xx_my_data->reg_on_gpio, 1);

#if defined(CONFIG_BT_MSM_SLEEP) && !defined(CONFIG_LINE_DISCIPLINE_DRIVER)
		bluesleep_start(1);
#endif
	} else {
		if (!regOnGpio) {
			pr_debug("Bluetooth device is already power off:%d\n",
				regOnGpio);
			return 0;
		}
		gpio_set_value(bcm43xx_my_data->reg_on_gpio, 0);

#if defined(CONFIG_BT_MSM_SLEEP) && !defined(CONFIG_LINE_DISCIPLINE_DRIVER)
		bluesleep_stop();
#endif
	}
	bt_enabled = !blocked;

	return 0;
}

static const struct rfkill_ops bcm43xx_bt_rfkill_ops = {
	.set_block = bcm43xx_bt_rfkill_set_power,
};

static int bcm43xx_bluetooth_dev_init(struct platform_device *pdev,
				struct bcm43xx_data *my_data)
{
	int ret;
	struct device_node *of_node = pdev->dev.of_node;

	my_data->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(my_data->pinctrl)) {
		dev_err(&pdev->dev, "%s: pinctrl not defined\n",
			__func__);
		ret = PTR_ERR(my_data->pinctrl);
	} else {
		my_data->has_pinctl = true;
	}

	if (my_data->has_pinctl) {
		my_data->gpio_state_active =
			pinctrl_lookup_state(my_data->pinctrl, PINCTRL_STATE_DEFAULT);
		if (IS_ERR_OR_NULL(my_data->gpio_state_active)) {
			dev_err(&pdev->dev, "%s(): pinctrl lookup failed for default\n",
				__func__);
			ret = PTR_ERR(my_data->gpio_state_active);
			goto error_pinctrl;
		}

		my_data->gpio_state_suspend =
			pinctrl_lookup_state(my_data->pinctrl, PINCTRL_STATE_SLEEP);
		if (IS_ERR_OR_NULL(my_data->gpio_state_suspend)) {
			dev_err(&pdev->dev, "%s(): pinctrl lookup failed for sleep\n",
				__func__);
			ret = PTR_ERR(my_data->gpio_state_suspend);
			goto error_pinctrl;
		}

		ret = pinctrl_select_state(bcm43xx_my_data->pinctrl,
			bcm43xx_my_data->gpio_state_active);
		if (ret) {
			dev_err(&pdev->dev, "%s(): failed to select active state\n",
				__func__);
			goto error_pinctrl;
		}
	}

	my_data->reg_on_gpio = of_get_named_gpio(of_node, "bcm,reg-on-gpio", 0);
	if (my_data->reg_on_gpio < 0) {
		dev_err(&pdev->dev, "%s(): couldn't find bt reg on gpio\n",
			__func__);
		ret = -ENODEV;
		goto error_gpio;
	}

	return 0;

error_gpio:
	if (my_data->has_pinctl)
		pinctrl_select_state(my_data->pinctrl, my_data->gpio_state_suspend);
error_pinctrl:
	return ret;

}

static int bcm43xx_bluetooth_probe(struct platform_device *pdev)
{
	int ret;

	struct device_node *of_node = pdev->dev.of_node;
	dev_dbg(&pdev->dev, "bcm43xx bluetooth driver being loaded\n");

	if (!of_node) {
		dev_err(&pdev->dev, "%s(): of_node is null\n", __func__);
		ret = -EPERM;
		goto error_of_node;
	}

	bcm43xx_my_data = kzalloc(sizeof(*bcm43xx_my_data), GFP_KERNEL);
	if (!bcm43xx_my_data) {
		dev_err(&pdev->dev, "%s(): no memory\n", __func__);
		ret = -ENOMEM;
		goto error_alloc_mydata;
	}
	bcm43xx_my_data->pdev = pdev;

	ret = bcm43xx_bluetooth_dev_init(pdev, bcm43xx_my_data);
	if (ret) {
		dev_err(&pdev->dev, "%s(): dev init failed\n", __func__);
		goto error_dev_init;
	}

	bt_rfkill = rfkill_alloc("bcm43xx Bluetooth", &pdev->dev,
				RFKILL_TYPE_BLUETOOTH, &bcm43xx_bt_rfkill_ops,
				NULL);

	if (unlikely(!bt_rfkill)) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "%s(): rfkill_alloc fail\n",  __func__);
		goto  error_free_gpio;
	}

	rfkill_set_states(bt_rfkill, true, false);
	ret = rfkill_register(bt_rfkill);

	if (unlikely(ret)) {
		rfkill_destroy(bt_rfkill);
		ret = -ENOMEM;
		dev_err(&pdev->dev, "%s(): rfkill_register fail\n", __func__);
		goto  error_free_gpio;
	}

	return 0;

error_free_gpio:
	if (bcm43xx_my_data->has_pinctl)
		pinctrl_select_state(bcm43xx_my_data->pinctrl,
			bcm43xx_my_data->gpio_state_suspend);
error_dev_init:
	kzfree(bcm43xx_my_data);
error_alloc_mydata:
error_of_node:
	return ret;
}

static int bcm43xx_bluetooth_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "bcm43xx_bluetooth_remove\n");
	rfkill_unregister(bt_rfkill);
	rfkill_destroy(bt_rfkill);

	if (bcm43xx_my_data->has_pinctl)
		if (!IS_ERR_OR_NULL(bcm43xx_my_data) &&
				!IS_ERR_OR_NULL(bcm43xx_my_data->pinctrl) &&
				!IS_ERR_OR_NULL(bcm43xx_my_data->gpio_state_suspend))
			pinctrl_select_state(
					bcm43xx_my_data->pinctrl,
					bcm43xx_my_data->gpio_state_suspend);

	kzfree(bcm43xx_my_data);

	return 0;
}

static struct of_device_id bcm43xx_match_table[] = {
	{.compatible = D_BCM_BLUETOOTH_CONFIG_MATCH_TABLE },
	{}
};

static struct platform_driver bcm43xx_bluetooth_platform_driver = {
	.probe = bcm43xx_bluetooth_probe,
	.remove = bcm43xx_bluetooth_remove,
	.driver = {
		.name = "bcm43xx_bluetooth",
		.owner = THIS_MODULE,
		.of_match_table = bcm43xx_match_table,
	},
};

static int __init bcm43xx_bluetooth_init(void)
{
	bt_enabled = false;
	return platform_driver_register(&bcm43xx_bluetooth_platform_driver);
}

static void __exit bcm43xx_bluetooth_exit(void)
{
	platform_driver_unregister(&bcm43xx_bluetooth_platform_driver);
}


module_init(bcm43xx_bluetooth_init);
module_exit(bcm43xx_bluetooth_exit);

MODULE_ALIAS("platform:bcm43xx");
MODULE_DESCRIPTION("bcm43xx_bluetooth");
MODULE_AUTHOR("Jaikumar Ganesh <jaikumar@google.com>");
MODULE_LICENSE("GPL v2");
