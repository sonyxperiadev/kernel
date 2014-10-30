/* linux/drivers/input/touchscreen/clearpad_i2c.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (c) 2011 Synaptics Incorporated
 * Copyright (c) 2011 Unixphere
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Yusuke Yoshimura <Yusuke.Yoshimura@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/async.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/clearpad.h>

#define CLEARPAD_PAGE_SELECT_REGISTER 0xff
#define CLEARPAD_REG(addr) ((addr) & 0xff)
#define CLEARPAD_PAGE(addr) (((addr) >> 8) & 0xff)

struct clearpad_i2c_t {
	struct platform_device *pdev;
	unsigned int page;
	struct mutex page_mutex;
};

static int clearpad_i2c_set_page(struct device *dev, u8 page)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(dev);
	char txbuf[2] = {CLEARPAD_PAGE_SELECT_REGISTER, page};
	int rc = 0;

	rc = i2c_master_send(to_i2c_client(dev), txbuf, sizeof(txbuf));
	if (rc != sizeof(txbuf)) {
		dev_err(dev,
			"%s: set page failed: %d.", __func__, rc);
		rc = (rc < 0) ? rc : -EIO;
		goto exit;
	}
	this->page = page;
	rc = 0;
exit:
	return rc;
}

static int clearpad_i2c_read(struct device *dev, u16 addr, u8 *buf, u8 len)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(dev);
	s32 rc = 0;
	u8 page = CLEARPAD_PAGE(addr);
	u8 reg = CLEARPAD_REG(addr);
	int rsize = I2C_SMBUS_BLOCK_MAX;
	int off;

	mutex_lock(&this->page_mutex);

	if (page != this->page) {
		rc = clearpad_i2c_set_page(dev, page);
		if (rc < 0)
			goto exit;
	}

	for (off = 0; off < len; off += rsize) {
		if (len < off + I2C_SMBUS_BLOCK_MAX)
			rsize = len - off;
		rc = i2c_smbus_read_i2c_block_data(to_i2c_client(dev),
				reg + off, rsize, &buf[off]);
		if (rc != rsize) {
			dev_err(dev, "%s: rc = %d\n", __func__, rc);
			if (rc > 0) {
				off += rc;
				break;
			}
			goto exit;
		}
	}
	rc = off;
exit:
	mutex_unlock(&this->page_mutex);
	return rc;
}

static int clearpad_i2c_write(struct device *dev, u16 addr,
			      const u8 *buf, u8 len)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(dev);
	int rc = 0;
	u8 reg = CLEARPAD_REG(addr);
	u8 i;

	mutex_lock(&this->page_mutex);

	if (CLEARPAD_PAGE(addr) != this->page) {
		rc = clearpad_i2c_set_page(dev, CLEARPAD_PAGE(addr));
		if (rc < 0)
			goto exit;
	}

	for (i = 0; i < len; i++) {
		rc = i2c_smbus_write_byte_data(to_i2c_client(dev),
				reg + i, buf[i]);
		if (rc)
			goto exit;
	}
	rc = i;
exit:
	mutex_unlock(&this->page_mutex);
	return rc;
}

static int clearpad_i2c_read_block(struct device *dev, u16 addr, u8 *buf,
		int len)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(dev);
	u8 txbuf[1] = {addr & 0xff};
	int rc = 0;

	mutex_lock(&this->page_mutex);

	if (CLEARPAD_PAGE(addr) != this->page) {
		rc = clearpad_i2c_set_page(dev, CLEARPAD_PAGE(addr));
		if (rc < 0)
			goto exit;
	}

	rc = i2c_master_send(to_i2c_client(dev), txbuf, sizeof(txbuf));
	if (rc != sizeof(txbuf)) {
		rc = (rc < 0) ? rc : -EIO;
		goto exit;
	}

	rc = i2c_master_recv(to_i2c_client(dev), buf, len);
	if (rc < 0)
		dev_err(dev, "%s: rc = %d\n", __func__, rc);
exit:
	mutex_unlock(&this->page_mutex);
	return rc;
}

static int clearpad_i2c_write_block(struct device *dev, u16 addr, const u8 *buf,
		int len)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(dev);
	u8 txbuf[len + 1];
	int rc = 0;

	txbuf[0] = addr & 0xff;
	memcpy(txbuf + 1, buf, len);

	mutex_lock(&this->page_mutex);

	if (CLEARPAD_PAGE(addr) != this->page) {
		rc = clearpad_i2c_set_page(dev, CLEARPAD_PAGE(addr));
		if (rc < 0)
			goto exit;
	}

	rc = i2c_master_send(to_i2c_client(dev), txbuf, sizeof(txbuf));
	if (rc < 0)
		dev_err(dev, "%s: rc = %d\n", __func__, rc);
	else
		rc -= 1;
exit:
	mutex_unlock(&this->page_mutex);
	return rc;
}

static struct clearpad_bus_data_t clearpad_i2c_bus_data = {
	.bustype	= BUS_I2C,
	.set_page	= clearpad_i2c_set_page,
	.read		= clearpad_i2c_read,
	.write		= clearpad_i2c_write,
	.read_block	= clearpad_i2c_read_block,
	.write_block	= clearpad_i2c_write_block,
};

#ifdef CONFIG_OF
static int clearpad_parse_dt(struct device *dev,
		struct clearpad_platform_data_t *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->irq_gpio = of_get_named_gpio_flags(np, "synaptics,irq_gpio",
			0, &pdata->irq_gpio_flags);
	return 0;
}
#else
static int clearpad_parse_dt(struct device *dev,
		struct clearpad_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int __devinit clearpad_i2c_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	struct clearpad_data_t clearpad_data = {
		.pdata = NULL,
		.bdata = &clearpad_i2c_bus_data,
		.probe_retry = 0,
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD_RMI_DEV
		.rmi_dev = NULL,
#endif
	};
	struct clearpad_i2c_t *this;
	int rc;

	if (client->dev.of_node) {
		clearpad_data.pdata = devm_kzalloc(&client->dev,
				sizeof(struct clearpad_platform_data_t),
				GFP_KERNEL);
		if (!clearpad_data.pdata) {
			dev_err(&client->dev, "failed to allocate memory\n");
			rc = -ENOMEM;
			goto exit;
		}
		rc = clearpad_parse_dt(&client->dev, clearpad_data.pdata);
		if (rc) {
			dev_err(&client->dev, "failed to parse device tree\n");
			goto exit;
		}
	} else {
		clearpad_data.pdata = client->dev.platform_data;
	}

	this = kzalloc(sizeof(struct clearpad_i2c_t), GFP_KERNEL);
	if (!this) {
		rc = -ENOMEM;
		goto exit;
	}

	dev_set_drvdata(&client->dev, this);

	mutex_init(&this->page_mutex);

	this->pdev = platform_device_alloc(CLEARPAD_NAME, -1);
	if (!this->pdev) {
		rc = -ENOMEM;
		goto err_free;
	}
	clearpad_data.bdata->dev = &client->dev;
	clearpad_data.bdata->of_node = client->dev.of_node;
	this->pdev->dev.parent = &client->dev;
	rc = platform_device_add_data(this->pdev,
			&clearpad_data, sizeof(clearpad_data));
	if (rc)
		goto err_device_put;

	rc = platform_device_add(this->pdev);
	if (rc)
		goto err_device_put;

	dev_info(&client->dev, "%s: sucess\n", __func__);
	goto exit;

err_device_put:
	platform_device_put(this->pdev);
err_free:
	dev_set_drvdata(&client->dev, NULL);
	kfree(this);
exit:
	return rc;
}

static int __devexit clearpad_i2c_remove(struct i2c_client *client)
{
	struct clearpad_i2c_t *this = dev_get_drvdata(&client->dev);
	platform_device_unregister(this->pdev);
	dev_set_drvdata(&client->dev, NULL);
	kfree(this);
	return 0;
}

static const struct i2c_device_id clearpad_id[] = {
	{ CLEARPADI2C_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, clearpad_id);

#ifdef CONFIG_OF
static struct of_device_id clearpad_match_table[] = {
	{ .compatible = "synaptics,clearpad", },
	{ },
};
#else
#define clearpad_match_table NULL
#endif

static struct i2c_driver clearpad_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= CLEARPADI2C_NAME,
		.of_match_table = clearpad_match_table,
	},
	.id_table	= clearpad_id,
	.probe		= clearpad_i2c_probe,
	.remove		= __devexit_p(clearpad_i2c_remove),
};

#ifndef MODULE
void __devinit clearpad_i2c_init_async(void *unused, async_cookie_t cookie)
{
	int rc;

	rc = i2c_add_driver(&clearpad_i2c_driver);
	if (rc != 0)
		pr_err("Clearpad I2C registration failed rc = %d\n", rc);
}
#endif

static int __init clearpad_i2c_init(void)
{
#ifdef MODULE
	return i2c_add_driver(&clearpad_i2c_driver);
#else
	async_schedule(clearpad_i2c_init_async, NULL);
	return 0;
#endif
}

static void __exit clearpad_i2c_exit(void)
{
	i2c_del_driver(&clearpad_i2c_driver);
}

MODULE_DESCRIPTION(CLEARPADI2C_NAME "ClearPad I2C Driver");
MODULE_LICENSE("GPL v2");

module_init(clearpad_i2c_init);
module_exit(clearpad_i2c_exit);
