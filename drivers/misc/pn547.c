/* drivers/misc/pn547.c
 *
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/async.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pn547.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wakelock.h>

#include <mach/board-nfc.h>
#include <mach/gpiomux.h>

#define PN547_WAIT_VENENABLE	10
#define PN547_WAIT_VENDISABLE	60

#define PN547_WAKE_LOCK_TIMEOUT	(HZ)
#define MAX_I2C_RETRY_COUNT	5
#define MAX_NORMAL_FRAME_SIZE	(255 + 3)
#define MAX_FIRMDL_FRAME_SIZE	(1023 + 5)

#define PN547_RES_READY_TIMEOUT_NORMAL	(HZ * 2)
#define PN547_RES_READY_TIMEOUT_FWDL	(HZ * 15)
#define PN547_RES_READY_SIZE	6
#define PN547_RES_READY	"ready"
#define PN547_RES_NOT_READY	"not_ready"

#ifdef DEBUG_DETAIL
#define DETAIL_LOG_SIZE	3
#endif

static DEFINE_MUTEX(lock);

static const char * const init_deinit_cmds[] = {
	[PN547_INIT]	= "pn547_init",
	[PN547_DEINIT]	= "pn547_deinit",
};

static const char * const set_pwr_cmds[] = {
	[PN547_SET_PWR_OFF]	= "pn547_set_pwr_off",
	[PN547_SET_PWR_ON]	= "pn547_set_pwr_on",
	[PN547_SET_PWR_FWDL]	= "pn547_set_pwr_fwdl",
};

struct pn547_dev {
	wait_queue_head_t		wq;
	struct wake_lock		wake_lock;
	struct device			*dev;
	struct i2c_client		*i2c_client;
	struct pn547_i2c_platform_data	*pdata;
	enum pn547_state		state;
	bool				busy;
	u16				req_size;
	atomic_t			res_ready;
};

static void dump_buf(struct pn547_dev *d, u8 *buf, int size)
{
#ifdef DEBUG_DETAIL
	if (d->state == PN547_STATE_ON || d->state == PN547_STATE_FWDL) {
		int i, size, p = 0;
		bool fwdl;
		char *s;

		fwdl = d->state == PN547_STATE_FWDL;
		size = fwdl ? MAX_FIRMDL_FRAME_SIZE : MAX_NORMAL_FRAME_SIZE;
		s = kzalloc(size * DETAIL_LOG_SIZE + 1);
		for (i = 0; i < size; i++)
			p += snprintf(s + p, sizeof(s) - p, "%02x ", buf[i]);
		dev_dbg(d->dev, "%s\n", s);
		kzfree(s);
	}
#endif
}

static irqreturn_t pn547_dev_irq_handler(int irq, void *dev_info)
{
	struct pn547_dev *d = dev_info;

	dev_dbg(d->dev, "%s: interruption\n", __func__);
	mutex_lock(&lock);
	wake_lock_timeout(&d->wake_lock, PN547_WAKE_LOCK_TIMEOUT);
	atomic_set(&d->res_ready, 1);
	wake_up_interruptible(&d->wq);
	mutex_unlock(&lock);
	return IRQ_HANDLED;
}

static int pn547_dev_chip_config(enum pn547_state state, struct pn547_dev *dev)
{
	switch (state) {
	case PN547_STATE_OFF:
		gpio_set_value_cansleep(dev->pdata->fwdl_en_gpio, 0);
		gpio_set_value_cansleep(dev->pdata->ven_gpio, 0);
		msleep(PN547_WAIT_VENDISABLE);
		break;
	case PN547_STATE_ON:
		gpio_set_value_cansleep(dev->pdata->fwdl_en_gpio, 0);
		gpio_set_value_cansleep(dev->pdata->ven_gpio, 1);
		msleep(PN547_WAIT_VENENABLE);
		break;
	case PN547_STATE_FWDL:
		gpio_set_value_cansleep(dev->pdata->ven_gpio, 1);
		gpio_set_value_cansleep(dev->pdata->fwdl_en_gpio, 1);
		msleep(PN547_WAIT_VENENABLE);
		gpio_set_value_cansleep(dev->pdata->ven_gpio, 0);
		msleep(PN547_WAIT_VENENABLE);
		gpio_set_value_cansleep(dev->pdata->ven_gpio, 1);
		msleep(PN547_WAIT_VENENABLE);
		break;
	default:
		dev_err(dev->dev, "%s: undefined state %d\n", __func__, state);
		return -EINVAL;
	}
	return 0;
}

static int pn547_parse_dt(struct device *dev,
			 struct pn547_i2c_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	int ret = 0;

	ret = of_get_named_gpio(np, "nxp,irq_gpio", 0);
	if (ret < 0) {
		dev_err(dev, "failed to get \"nxp,irq_gpio\"\n");
		goto err;
	}
	pdata->irq_gpio = ret;

	ret = of_get_named_gpio(np, "nxp,dwld_en", 0);
	if (ret < 0) {
		dev_err(dev, "failed to get \"nxp,dwld_en\"\n");
		goto err;
	}
	pdata->fwdl_en_gpio = ret;

	ret = of_get_named_gpio(np, "nxp,pvdd_en", 0);
	if (ret < 0) {
		dev_err(dev, "failed to get \"nxp,pvdd_en\"\n");
		goto err;
	}
	pdata->pvdd_en_gpio = ret;

	ret = of_get_named_gpio(np, "nxp,ven", 0);
	if (ret < 0) {
		dev_err(dev, "failed to get \"nxp,ven\"\n");
		goto err;
	}
	pdata->ven_gpio = ret;

err:
	return ret;
}

static int pn547_gpio_request(struct device *dev,
				struct pn547_i2c_platform_data *pdata)
{
	int ret;

	ret = gpio_request(pdata->pvdd_en_gpio, "pn547_pvdd_en");
	if (ret)
		goto err_pvdd;
	ret = gpio_request(pdata->irq_gpio, "pn547_irq");
	if (ret)
		goto err_irq;
	ret = gpio_request(pdata->fwdl_en_gpio, "pn547_fw");
	if (ret)
		goto err_fwdl_en;
	ret = gpio_request(pdata->ven_gpio, "pn547_ven");
	if (ret)
		goto err_ven;
	return 0;

err_ven:
	gpio_free(pdata->fwdl_en_gpio);
err_fwdl_en:
	gpio_free(pdata->irq_gpio);
err_irq:
	gpio_free(pdata->pvdd_en_gpio);
err_pvdd:
	dev_err(dev, "%s: gpio request err %d\n", __func__, ret);
	return ret;
}

static void pn547_gpio_release(struct pn547_i2c_platform_data *pdata)
{
	gpio_free(pdata->ven_gpio);
	gpio_free(pdata->irq_gpio);
	gpio_free(pdata->fwdl_en_gpio);
	gpio_free(pdata->pvdd_en_gpio);
}

static ssize_t pn547_dev_recv_resp_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pn547_dev *d = dev_get_drvdata(dev);
	int ret = 0, retry_count = 0, maxlen;
	bool fwdl;

	if (!d)
		return -ENODEV;

	mutex_lock(&lock);
	fwdl = d->state == PN547_STATE_FWDL;
	maxlen = fwdl ? MAX_FIRMDL_FRAME_SIZE : MAX_NORMAL_FRAME_SIZE;
	if (d->req_size > maxlen)
		d->req_size = maxlen;
	atomic_set(&d->res_ready, 0);
retry:
	ret = i2c_master_recv(d->i2c_client, buf, d->req_size);
	if (ret == -ENODEV || ret == -ENOTCONN || ret == -EIO) {
		retry_count++;
		if (retry_count > MAX_I2C_RETRY_COUNT) {
			dev_err(d->dev, "%s: i2c err %d, retry count expired\n",
				__func__, ret);
			goto exit;
		}
		usleep_range(10000, 11000);
		goto retry;
	}

	if (ret < 0) {
		dev_err(d->dev, "%s: i2c err %d\n", __func__, ret);
		goto exit;
	}

	if (ret > d->req_size) {
		dev_err(d->dev, "%s: i2c err %d\n", __func__, ret);
		ret = -EIO;
		goto exit;
	}
	dump_buf(d, buf, ret);
exit:
	mutex_unlock(&lock);
	return ret;
}

static ssize_t pn547_dev_recv_resp_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct pn547_dev *d = dev_get_drvdata(dev);

	if (!d)
		return -ENODEV;

	mutex_lock(&lock);
	memcpy(&d->req_size, buf, size);
	dev_dbg(d->dev, "%s: req_size = %d\n", __func__, d->req_size);
	mutex_unlock(&lock);
	return 0;
}

static ssize_t pn547_dev_res_ready_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pn547_dev *d = dev_get_drvdata(dev);
	bool state = false;

	if (!d)
		return -ENODEV;

	dev_dbg(d->dev, "%s: res_ready_show enter\n", __func__);

	wait_event_interruptible_timeout(d->wq, atomic_read(&d->res_ready),
		d->state == PN547_STATE_FWDL ?
		PN547_RES_READY_TIMEOUT_FWDL : PN547_RES_READY_TIMEOUT_NORMAL);
	if (atomic_read(&d->res_ready))
		state = true;

	dev_dbg(d->dev, "%s: res_ready_show exit\n", __func__);
	return snprintf(buf, PN547_RES_READY_SIZE, "%s",
		state ? PN547_RES_READY : PN547_RES_NOT_READY);
}

static ssize_t pn547_dev_send_cmd_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct pn547_dev *d = dev_get_drvdata(dev);
	int ret = 0, retry_count = 0, maxlen;
	bool fwdl;

	if (!d)
		return -ENODEV;

	mutex_lock(&lock);
	if (!size)
		goto exit;

	fwdl = d->state == PN547_STATE_FWDL;
	maxlen = fwdl ? MAX_FIRMDL_FRAME_SIZE : MAX_NORMAL_FRAME_SIZE;
	if (size > maxlen)
		size = maxlen;
retry:
	ret = i2c_master_send(d->i2c_client, buf, size);
	if (ret == -ENODEV || ret == -ENOTCONN) {
		retry_count++;
		if (retry_count > MAX_I2C_RETRY_COUNT) {
			dev_err(d->dev, "%s: i2c err %d, retry count expired\n",
				__func__, ret);
			goto exit;
		}
		usleep_range(10000, 11000);
		goto retry;
	}

	if (ret < 0) {
		dev_err(d->dev, "%s: i2c err %d\n", __func__, ret);
		goto exit;
	}
	dev_dbg(d->dev, "%s: %d bytes write\n", __func__, ret);
	dump_buf(d, (u8 *)buf, ret);
exit:
	mutex_unlock(&lock);
	return ret;
}

static ssize_t pn547_dev_init_deinit_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct pn547_dev *d = dev_get_drvdata(dev);

	if (!d)
		return -ENODEV;

	mutex_lock(&lock);
	if (!strncmp(buf, init_deinit_cmds[PN547_INIT], PAGE_SIZE)) {
		dev_dbg(d->dev, "%s: PN547_INIT\n", __func__);
		if (d->busy) {
			mutex_unlock(&lock);
			return -EBUSY;
		}
		d->busy = true;
	} else if (!strncmp(buf, init_deinit_cmds[PN547_DEINIT], PAGE_SIZE)) {
		dev_dbg(d->dev, "%s: PN547_DEINIT\n", __func__);
		if (atomic_read(&d->res_ready))
			dev_dbg(d->dev, "%s: interruption is ignored\n",
				__func__);
		atomic_set(&d->res_ready, 0);
		d->busy = false;
	}
	mutex_unlock(&lock);
	return 0;
}

static ssize_t pn547_dev_set_pwr_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct pn547_dev *d = dev_get_drvdata(dev);
	int state, ret = 0;

	disable_irq(d->i2c_client->irq);
	mutex_lock(&lock);
	if (!strncmp(buf, set_pwr_cmds[PN547_SET_PWR_OFF], PAGE_SIZE)) {
		dev_dbg(d->dev, "%s: PN547_SET_PWR_OFF\n", __func__);
		state = PN547_STATE_OFF;
	} else if (!strncmp(buf, set_pwr_cmds[PN547_SET_PWR_ON], PAGE_SIZE)) {
		dev_dbg(d->dev, "%s: PN547_SET_PWR_ON\n", __func__);
		state = PN547_STATE_ON;
	} else if (!strncmp(buf, set_pwr_cmds[PN547_SET_PWR_FWDL], PAGE_SIZE)) {
		dev_dbg(d->dev, "%s: PN547_SET_PWR_FWDL\n", __func__);
		state = PN547_STATE_FWDL;
	} else {
		dev_err(d->dev, "%s: illegal command\n", __func__);
		ret = -EINVAL;
	}

	if (!ret) {
		ret = pn547_dev_chip_config(state, d);
	    if (IS_ERR_VALUE(ret)) {
			dev_err(d->dev, "%s: chip config err %d\n",
				__func__, ret);
			goto exit;
		}
		if (d->state != state) {
			d->state = state;
			atomic_set(&d->res_ready, 0);
	    }
	} else {
		dev_err(d->dev, "%s failed\n", __func__);
	}
exit:
	mutex_unlock(&lock);
	enable_irq(d->i2c_client->irq);
	return strnlen(buf, PAGE_SIZE);
}

static struct device_attribute pn547_sysfs_attrs[] = {
	__ATTR(init_deinit, S_IWUSR, NULL, pn547_dev_init_deinit_store),
	__ATTR(set_pwr, S_IWUSR, NULL, pn547_dev_set_pwr_store),
	__ATTR(res_ready, S_IRUSR, pn547_dev_res_ready_show, NULL),
	__ATTR(send_cmd, S_IWUSR, NULL, pn547_dev_send_cmd_store),
	__ATTR(recv_rsp, S_IRUSR | S_IWUSR, pn547_dev_recv_resp_show,
		pn547_dev_recv_resp_store),
};

static int pn547_dev_create_sysfs_entries(struct i2c_client *dev)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(pn547_sysfs_attrs); i++) {
		ret = device_create_file(&dev->dev, &pn547_sysfs_attrs[i]);
		if (ret) {
			for (; i >= 0; i--)
				device_remove_file(&dev->dev,
						&pn547_sysfs_attrs[i]);
			goto exit;
		}
	}
exit:
	return ret;
}

static void pn547_dev_remove_sysfs_entries(struct i2c_client *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pn547_sysfs_attrs); i++)
		device_remove_file(&dev->dev, &pn547_sysfs_attrs[i]);
}

static int pn547_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct pn547_dev *dev;
	struct pn547_i2c_platform_data *pdata;
	struct clk *nfc_clk = NULL;
	int ret = 0;

	pdata = kzalloc(sizeof(struct pn547_i2c_platform_data),
			GFP_KERNEL);
	if (!pdata) {
		dev_err(&client->dev, "failed to get allocate memory\n");
		ret = -ENOMEM;
		goto probe_pdata;
	}
	ret = pn547_parse_dt(&client->dev, pdata);
	if (ret < 0) {
		dev_err(&client->dev, "failed to parse device tree: %d\n", ret);
		goto probe_parse_dt;
	}

	ret = board_nfc_parse_dt(&client->dev, pdata);
	if (ret < 0) {
		dev_err(&client->dev, "failed to parse device tree: %d\n", ret);
		goto probe_parse_dt;
	}

	if (pdata->dynamic_config) {
		ret = board_nfc_hw_lag_check(client, pdata);
		if (ret < 0)
			goto probe_hw_lag_check;
	}

	ret = pn547_gpio_request(&client->dev, pdata);
	if (ret) {
		dev_err(&client->dev, "failed to request gpio\n");
		goto probe_gpio_request;
	}
	dev_dbg(&client->dev, "%s:\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "%s: i2c check failed\n", __func__);
		ret = -ENODEV;
		goto probe_i2c;
	}

	dev = kzalloc(sizeof(struct pn547_dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&client->dev, "%s: no memory\n", __func__);
		ret = -ENOMEM;
		goto probe_mem;
	}
	dev->i2c_client = client;
	dev->dev = &client->dev;
	dev->pdata = pdata;
	init_waitqueue_head(&dev->wq);
	wake_lock_init(&dev->wake_lock, WAKE_LOCK_SUSPEND, "pn547");
	i2c_set_clientdata(client, dev);
	dev->state = PN547_STATE_UNKNOWN;

	ret = request_threaded_irq(client->irq, NULL, pn547_dev_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT, client->name, dev);
	if (IS_ERR_VALUE(ret)) {
		dev_err(&client->dev, "%s: irq request err %d\n",
			__func__, ret);
		goto probe_irq;
	}

	nfc_clk = clk_get(&client->dev, "nfc_clk");
	if (IS_ERR(nfc_clk)) {
		dev_err(&client->dev, "Couldn't get nfc_clk\n");
		goto probe_clk;
	}
	ret = clk_prepare_enable(nfc_clk);
	if (ret) {
		dev_err(&client->dev, "nfc_clk enable is failed\n");
		goto probe_clk_enable;
	}

	ret = pn547_dev_create_sysfs_entries(dev->i2c_client);
	if (IS_ERR_VALUE(ret)) {
		dev_err(&client->dev, "%s: create sysfs entries err %d\n",
			__func__, ret);
		goto probe_sysfs;
	}
	return ret;

probe_sysfs:
	free_irq(client->irq, dev);
probe_clk_enable:
	clk_put(nfc_clk);
probe_clk:
probe_irq:
	i2c_set_clientdata(client, NULL);
	wake_lock_destroy(&dev->wake_lock);
	kzfree(dev);
probe_mem:
probe_i2c:
	pn547_gpio_release(pdata);
probe_gpio_request:
probe_hw_lag_check:
probe_parse_dt:
	kzfree(pdata);
probe_pdata:
	dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	return ret;
}

static int pn547_remove(struct i2c_client *client)
{
	struct pn547_dev *d = i2c_get_clientdata(client);

	pn547_dev_remove_sysfs_entries(client);
	free_irq(client->irq, d);
	i2c_set_clientdata(client, NULL);
	wake_lock_destroy(&d->wake_lock);
	pn547_gpio_release(d->pdata);
	kzfree(d->pdata);
	kzfree(d);
	return 0;
}

static int pn547_pm_suspend(struct device *dev)
{
	int ret;
	struct pn547_dev *d = dev_get_drvdata(dev);

	dev_dbg(d->dev, "%s:\n", __func__);
	if (!mutex_trylock(&lock))
		return -EAGAIN;
	if (d->busy) {
		if (atomic_read(&d->res_ready)) {
			wake_lock_timeout(&d->wake_lock,
				PN547_WAKE_LOCK_TIMEOUT);
			mutex_unlock(&lock);
			return -EAGAIN;
		}
		ret = irq_set_irq_wake(d->i2c_client->irq, 1);
		if (IS_ERR_VALUE(ret))
			dev_err(dev, "%s: irq wake err %d\n", __func__, ret);
	}
	mutex_unlock(&lock);
	return 0;
}

static int pn547_pm_resume(struct device *dev)
{
	struct pn547_dev *d = dev_get_drvdata(dev);
	int ret;

	dev_dbg(d->dev, "%s:\n", __func__);
	mutex_lock(&lock);
	if (d->busy) {
		ret = irq_set_irq_wake(d->i2c_client->irq, 0);
		if (IS_ERR_VALUE(ret))
			dev_err(dev, "%s: irq wake err %d\n", __func__, ret);
	}
	mutex_unlock(&lock);
	return 0;
}

static const struct i2c_device_id pn547_id[] = {
	{ PN547_DEVICE_NAME, 0 },
	{ },
};

static struct of_device_id pn547_match_table[] = {
	{ .compatible = "nxp,pn547", },
	{ },
};

static const struct dev_pm_ops pn547_pm_ops = {
	.suspend	= pn547_pm_suspend,
	.resume		= pn547_pm_resume,
};

static struct i2c_driver pn547_driver = {
	.id_table	= pn547_id,
	.probe		= pn547_probe,
	.remove		= pn547_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= PN547_DEVICE_NAME,
		.pm		= &pn547_pm_ops,
		.of_match_table	= pn547_match_table,
	},
};

#ifndef MODULE
static void __init pn547_dev_init_async(void *unused, async_cookie_t cookie)
{
	int rc;

	rc = i2c_add_driver(&pn547_driver);
	if (rc != 0)
		pr_err("Maxim I2C registration failed rc = %d\n", rc);
}
#endif

static int __init pn547_dev_init(void)
{
#ifdef MODULE
	return i2c_add_driver(&pn547_driver);
#else
	async_schedule(pn547_dev_init_async, NULL);
	return 0;
#endif
}

static void __exit pn547_dev_exit(void)
{
	i2c_del_driver(&pn547_driver);
}

module_init(pn547_dev_init);
module_exit(pn547_dev_exit);

MODULE_DESCRIPTION("NFC PN547 driver");
MODULE_LICENSE("GPL v2");
