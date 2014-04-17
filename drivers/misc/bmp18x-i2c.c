/*  Copyright (c) 2011  Bosch Sensortec GmbH
    Copyright (c) 2011  Unixphere

    Based on:
    BMP085 driver, bmp085.c
    Copyright (c) 2010  Christoph Mair <christoph.mair@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include "linux/bmp18x.h"

#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#ifdef CONFIG_ARCH_KONA
#include <linux/regulator/consumer.h>
static struct regulator *regulator;
#endif

#define BMP_ERROR(fmt, args...)   printk(KERN_ERR "%s, " fmt, __func__, ## args)
#define BMP_INFO(fmt, args...)   printk(KERN_INFO "%s, " fmt, __func__, ## args)
#define BMP_DEBUG(fmt, args...) \
do { if (mod_debug) printk(KERN_WARNING "%s, " fmt, __func__, ## args); } \
while (0)

/* module parameter that enables tracing */
static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);

static int bmp18x_i2c_read_block(void *client, u8 reg, int len, char *buf)
{
	return i2c_smbus_read_i2c_block_data(client, reg, len, buf);
}

static int bmp18x_i2c_read_byte(void *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static int bmp18x_i2c_write_byte(void *client, u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(client, reg, value);
}

static const struct bmp18x_bus_ops bmp18x_i2c_bus_ops =
{
	.bus_type	= BUS_I2C,
	.read_block	= bmp18x_i2c_read_block,
	.read_byte	= bmp18x_i2c_read_byte,
	.write_byte	= bmp18x_i2c_write_byte
};

static int bmp18x_i2c_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	int err = 0;
	struct bmp18x_platform_data *pdata = NULL;
	struct bmp18x_data_bus data_bus =
	{
		.bops = &bmp18x_i2c_bus_ops,
		.client = client
	};

        printk(KERN_ALERT "****************** calling bmp probe*************************"); 
	err = bmp18x_probe(&client->dev, &data_bus);
	if (err) {
		BMP_ERROR("bmp18x_probe failed with status: %d\n", err);
		return err;
	}
	if (client->dev.platform_data)
		pdata = client->dev.platform_data;

	else if (client->dev.of_node) {
		const char *regulator_name;

		pdata = kzalloc(sizeof(struct bmp18x_platform_data),
			GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		if (of_property_read_string(client->dev.of_node,
			"regulator-name", &regulator_name)) {
			BMP_ERROR("%s: can't get regulator name from DT!\n",
				BMP18X_NAME);
			goto err_read;
		}

		pdata->supply_name = (char *) regulator_name;
	}
#ifdef CONFIG_ARCH_KONA

	if (pdata) {
		regulator = regulator_get(&client->dev, pdata->supply_name);
		err = IS_ERR_OR_NULL(regulator);
		if (err) {
			regulator = NULL;
			BMP_ERROR("%s: can't get vdd regulator!\n",
				BMP18X_NAME);
			return -EIO;
		}

		/* make sure that regulator is enabled if device
		 is successfully bound */
		err = regulator_enable(regulator);
		BMP_INFO("called regulator_enable for vdd regulator. "
			"Status: %d\n", err);
		if (err) {
			BMP_ERROR("regulator_enable for vdd regulator "
				 "failed with status: %d\n", err);
			regulator_put(regulator);
		}
		return err;
	}
err_read:
	if (client->dev.of_node)
		kfree(pdata);
	return -EIO;
#endif
}

static void bmp18x_i2c_shutdown(struct i2c_client *client)
{
	int ret;
	ret = bmp18x_disable(&client->dev);
	BMP_DEBUG("set low power state. Status: %d\n", ret);

#ifdef CONFIG_ARCH_KONA
	if (regulator) {
		ret = regulator_disable(regulator);
		BMP_DEBUG("called regulator_disable. Status: %d\n", ret);
		if (ret) {
			BMP_ERROR("regulator_disable failed with status: %d\n",
				  ret);
			return;
		}
		regulator_put(regulator);
	}

#endif
}

static int bmp18x_i2c_remove(struct i2c_client *client)
{
	int ret;

	ret = bmp18x_remove(&client->dev);
	if (ret)
		BMP_ERROR("bmp18x_remove failed with status: %d\n", ret);

#ifdef CONFIG_ARCH_KONA
	if (regulator) {
		ret = regulator_disable(regulator);
		BMP_DEBUG("called regulator_disable. Status: %d\n", ret);
		if (ret) {
			BMP_ERROR("regulator_disable failed with status: %d\n",
				  ret);
			return ret;
		}
		regulator_put(regulator);
	}

#endif
	return ret;
}

#ifdef CONFIG_PM
static int bmp18x_i2c_suspend(struct device *dev)
{
	int ret = 0;

	BMP_DEBUG("called\n");

	/* set low power state */
	ret = bmp18x_disable(dev);
	BMP_DEBUG("set low power state. Status: %d\n", ret);
	if (ret) {
		BMP_ERROR("set low power state failed with status: %d\n",
				 ret);
		return ret;
	}

#ifdef CONFIG_ARCH_KONA
	if (regulator) {
		ret = regulator_disable(regulator);
		BMP_DEBUG("called regulator_disable. Status: %d\n", ret);
		if (ret)
			BMP_ERROR("regulator_disable failed with status: %d\n",
				 ret);
	}
#endif
	return ret;
}

static int bmp18x_i2c_resume(struct device *dev)
{
	int ret = 0;

	BMP_DEBUG("called\n");

	ret = bmp18x_enable(dev);
	BMP_DEBUG("set high power state. Status: %d\n", ret);
	if (ret) {
		BMP_ERROR("set high power state failed with "
			 "status: %d\n", ret);
		return ret;
	}

#ifdef CONFIG_ARCH_KONA
	if (regulator) {
		ret = regulator_enable(regulator);
		BMP_DEBUG("called regulator_enable. Status: %d\n", ret);
		if (ret)
			BMP_ERROR("regulator_enable failed with status: %d\n",
				 ret);
	}
#endif
	return ret;
}

static const struct dev_pm_ops bmp18x_i2c_pm_ops = {
	.suspend	= bmp18x_i2c_suspend,
	.resume		= bmp18x_i2c_resume
};
#endif

static const struct i2c_device_id bmp18x_id[] = {
	{ BMP18X_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, bmp18x_id);

static const struct of_device_id bmp18x_of_match[] = {
	{ .compatible = "bcm,bmp18x", },
	{},
}
MODULE_DEVICE_TABLE(of, bmp18x_of_match);

static struct i2c_driver bmp18x_i2c_driver =
{
	.driver =
	{
		.owner	= THIS_MODULE,
		.name	= BMP18X_NAME,
#ifdef CONFIG_PM
		.pm	= &bmp18x_i2c_pm_ops,
		.of_match_table = bmp18x_of_match,
#endif
	},
	.id_table	= bmp18x_id,
	.probe		= bmp18x_i2c_probe,
	.remove		= bmp18x_i2c_remove,
	.shutdown	= bmp18x_i2c_shutdown,
};

static int __init bmp18x_i2c_init(void)
{
        printk(KERN_ALERT "inside bmp18x_i2c_init\n");
	return i2c_add_driver(&bmp18x_i2c_driver);
}

static void __exit bmp18x_i2c_exit(void)
{
	i2c_del_driver(&bmp18x_i2c_driver);
}


MODULE_AUTHOR("Eric Andersson <eric.andersson@unixphere.com>");
MODULE_DESCRIPTION("BMP18X I2C bus driver");
MODULE_LICENSE("GPL");

module_init(bmp18x_i2c_init);
module_exit(bmp18x_i2c_exit);
