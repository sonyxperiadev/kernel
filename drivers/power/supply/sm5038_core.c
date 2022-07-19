// SPDX-License-Identifier: GPL-2.0-only
/*
 *  sm5038_core.c - mfd core driver for the SM5038.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/power/sm5038.h>
#include <linux/regulator/machine.h>

#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>

static struct sm5038_dev *static_sm5038_data;

//static struct mfd_cell sm5038_devs[] = {
//	{ .name = "sm5038-muic", },
//	{ .name = "sm5038-fuelgauge", },
//	{ .name = "sm5038-charger", },
//	{ .name = "sm5038-fled", },
//};

extern int sm5038_irq_init(struct sm5038_dev *sm5038);
extern void sm5038_irq_exit(struct sm5038_dev *sm5038);

extern int sm5038_muic_probe(struct sm5038_dev *sm5038);
extern int sm5038_muic_remove(void);
extern void sm5038_muic_shutdown(void);

extern int sm5038_fuelgauge_probe(struct sm5038_dev *sm5038);
extern int sm5038_fuelgauge_remove(void);

extern int sm5038_charger_probe(struct sm5038_dev *sm5038);
extern int sm5038_charger_remove(void);
extern void sm5038_charger_shutdown(void);

extern int sm5038_fled_probe(struct sm5038_dev *sm5038);
extern int sm5038_fled_remove(void);


int sm5038_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {
		pr_info("%s:%s reg(0x%x), ret(%d)\n", SM5038_DEV_NAME, __func__, reg, ret);
		return ret;
	}
	*dest = (ret & 0xff);

    return 0;
}
EXPORT_SYMBOL_GPL(sm5038_read_reg);

int sm5038_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_bulk_read);

int sm5038_read_word(struct i2c_client *i2c, u8 reg)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_read_word_data(i2c, reg);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {

		pr_info("%s:%s reg(0x%x), ret(%d)\n", SM5038_DEV_NAME, __func__, reg, ret);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_read_word);

int sm5038_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {
		pr_info("%s:%s reg(0x%x), ret(%d)\n", SM5038_DEV_NAME, __func__, reg, ret);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_write_reg);

int sm5038_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_bulk_write);

int sm5038_write_word(struct i2c_client *i2c, u8 reg, u16 value)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_write_word_data(i2c, reg, value);
	mutex_unlock(&sm5038->i2c_lock);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_write_word);

int sm5038_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;
	int ret;

	mutex_lock(&sm5038->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		u8 old_val = ret & 0xff;
		u8 new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&sm5038->i2c_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(sm5038_update_reg);

void sm5038_irq_thread_lock(void)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;

	pr_info("%s: %s\n", SM5038_DEV_NAME, __func__);
	mutex_lock(&sm5038->irq_thread_lock);
	
	return;
}
EXPORT_SYMBOL_GPL(sm5038_irq_thread_lock);

void sm5038_irq_thread_unlock(void)
{
	struct sm5038_dev *sm5038 = static_sm5038_data;

	pr_info("%s: %s\n", SM5038_DEV_NAME, __func__);
	mutex_unlock(&sm5038->irq_thread_lock);

	return;
}
EXPORT_SYMBOL_GPL(sm5038_irq_thread_unlock);


static int of_sm5038_dt(struct device *dev, struct sm5038_platform_data *pdata)
{
	struct device_node *np_sm5038 = dev->of_node;

	if (!np_sm5038)
		return -EINVAL;

	pdata->irq_gpio = of_get_named_gpio(np_sm5038, "sm5038,irq-gpio", 0);
	pdata->wakeup = of_property_read_bool(np_sm5038, "sm5038,wakeup");

	pr_info("%s: irq-gpio: %u \n", __func__, pdata->irq_gpio);

	return 0;
}

static int sm5038_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *dev_id)
{
	struct sm5038_dev *sm5038;
	struct sm5038_platform_data *pdata = i2c->dev.platform_data;

	int ret = 0;
	u8 temp = 0;

	pr_info("%s: %s start\n", SM5038_DEV_NAME, __func__);

	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		pr_err("%s:%s i2c functionality check error\n",
				SM5038_DEV_NAME, __func__);		
		ret = -EIO;
		goto err;
	}

	sm5038 = kzalloc(sizeof(struct sm5038_dev), GFP_KERNEL);
	if (!sm5038) {
		pr_err("%s: Failed to alloc mem for sm5038\n",
								__func__);
		return -ENOMEM;
	}

	if (i2c->dev.of_node) {
		pdata = devm_kzalloc(&i2c->dev, sizeof(struct sm5038_platform_data), GFP_KERNEL);
		if (!pdata) {
			pr_err("Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err;

		}

		ret = of_sm5038_dt(&i2c->dev, pdata);
		if (ret < 0) {
			pr_err("Failed to get device of_node\n");
			goto err;
		}

		i2c->dev.platform_data = pdata;
	} else {
		pdata = i2c->dev.platform_data;
	}

	sm5038->dev = &i2c->dev;
	sm5038->charger_i2c = i2c;
	sm5038->irq = i2c->irq;

	if (pdata) {
		sm5038->pdata = pdata;

		pdata->irq_base = irq_alloc_descs(-1, 0, SM5038_IRQ_NR, -1);
		if (pdata->irq_base < 0) {
			pr_err("%s:%s irq_alloc_descs Fail! ret(%d)\n",
				SM5038_DEV_NAME, __func__, pdata->irq_base);
			ret = -EINVAL;
			goto err;
		} else {
			sm5038->irq_base = pdata->irq_base;
		}

		sm5038->irq_gpio = pdata->irq_gpio;
		sm5038->wakeup = pdata->wakeup;
	} else {
		ret = -EINVAL;
		pr_err("%s:%s pdata error\n", SM5038_DEV_NAME, __func__);
		goto err;
	}

	mutex_init(&sm5038->i2c_lock);
	mutex_init(&sm5038->irq_thread_lock);

	static_sm5038_data = sm5038;

	//i2c_set_clientdata(i2c, sm5038);
	
	pr_info("%s: %s : sm5038->dev=%p, sm5038->irq=%d\n", SM5038_DEV_NAME, __func__, sm5038->dev, sm5038->irq);

    /* Get Device ID & Check Charger I2C transmission */
    ret = sm5038_read_reg(i2c, SM5038_CHG_REG_DEVICEID, &temp);
	if (ret < 0 || (temp & 0x7) != 0x1) {
		pr_err("%s:%s device not found on this channel (reg_data=0x%x)\n", SM5038_DEV_NAME, __func__, temp);
		ret = -ENODEV;
		goto err_w_lock;
	}
    sm5038->vender_id = (temp & 0x7);
    sm5038->pmic_rev = ((temp >> 3) & 0x1F);

    pr_info("%s:%s v_id=0x%x, rev=0x%x\n",
		SM5038_DEV_NAME, __func__, sm5038->vender_id, sm5038->pmic_rev);

	sm5038->fuelgauge_i2c = i2c_new_dummy(i2c->adapter, SM5038_I2C_SADR_FG);
	//i2c_set_clientdata(sm5038->fuelgauge_i2c, sm5038);
    /* Check FG I2C transmission */
    ret = sm5038_read_word(sm5038->fuelgauge_i2c, SM5038_FG_REG_DEVICE_ID);
    if ((unsigned int)ret > 0xFF) {
		pr_err("%s:%s fail to setup FG I2C transmission (ret=0x%x)\n", SM5038_DEV_NAME, __func__, ret);
		ret = -ENODEV;
		goto err_w_lock;
    }

	sm5038->muic_i2c = i2c_new_dummy(i2c->adapter, SM5038_I2C_SADR_MUIC);
	//i2c_set_clientdata(sm5038->muic_i2c, sm5038);
    /* Check MUIC I2C transmission */
    ret = sm5038_read_reg(sm5038->muic_i2c, SM5038_MUIC_REG_DEVICEID, &temp);
	if (ret < 0 || temp != 0x1) {
		pr_err("%s:%s fail to setup MUIC I2C transmission (reg_data=0x%x)\n", SM5038_DEV_NAME, __func__, temp);
		ret = -ENODEV;
		goto err_w_lock;
	}

	ret = sm5038_irq_init(sm5038);
	if (ret < 0) {
		pr_err("%s:%s irq init error\n", SM5038_DEV_NAME, __func__);
		goto err_irq_init;
	}

	//ret = mfd_add_devices(sm5038->dev, -1, sm5038_devs, ARRAY_SIZE(sm5038_devs), NULL, 0, NULL);
	//if (ret < 0) {
	//	pr_err("%s:%s mfd_add_devices error\n", SM5038_DEV_NAME, __func__);
	//	goto err_mfd;
	//}

	device_init_wakeup(sm5038->dev, pdata->wakeup);

	//////////////////////
	// muic
	//////////////////////
	ret = sm5038_muic_probe(sm5038);
	if (ret < 0) {
		pr_err("%s:%s sm5038_muic_probe error\n", SM5038_DEV_NAME, __func__);
		goto err;
	}

	//////////////////////
	// fg
	//////////////////////
	ret = sm5038_fuelgauge_probe(sm5038);
	if (ret < 0) {
		pr_err("%s:%s sm5038_fuelgauge_probe error\n", SM5038_DEV_NAME, __func__);
		goto err;
	}

	//////////////////////
	// charger
	//////////////////////
	ret = sm5038_charger_probe(sm5038);
	if (ret < 0) {
		pr_err("%s:%s sm5038_charger_probe error\n", SM5038_DEV_NAME, __func__);
		goto err;
	}	
	//////////////////////
	// fled
	//////////////////////
	ret = sm5038_fled_probe(sm5038);
	if (ret < 0) {
		pr_err("%s:%s sm5038_fled_probe error\n", SM5038_DEV_NAME, __func__);
		goto err;
	}

    pr_info("%s: %s done\n", SM5038_DEV_NAME, __func__);

	return ret;

//err_mfd:
//	mfd_remove_devices(sm5038->dev);
//    sm5038_irq_exit(sm5038);
err_irq_init:
	i2c_unregister_device(sm5038->fuelgauge_i2c);
    i2c_unregister_device(sm5038->muic_i2c);
err_w_lock:
	mutex_destroy(&sm5038->i2c_lock);
	mutex_destroy(&sm5038->irq_thread_lock);
err:
	kfree(sm5038);

    return ret;
}

static int sm5038_i2c_remove(struct i2c_client *i2c)
{
	struct sm5038_dev *sm5038 = i2c_get_clientdata(i2c);

	//////////////////////
	// muic
	//////////////////////
	sm5038_muic_remove();


	//////////////////////
	// fg
	//////////////////////
	sm5038_fuelgauge_remove();

	//////////////////////
	// charger
	//////////////////////
	sm5038_charger_remove();
	
	//////////////////////
	// fled
	//////////////////////
	sm5038_fled_remove();


	//mfd_remove_devices(sm5038->dev);
    sm5038_irq_exit(sm5038);

	i2c_unregister_device(sm5038->muic_i2c);
    i2c_unregister_device(sm5038->fuelgauge_i2c);

    mutex_destroy(&sm5038->i2c_lock);
	mutex_destroy(&sm5038->irq_thread_lock);

    kfree(sm5038);

	return 0;
}

static void sm5038_i2c_shutdown(struct i2c_client *i2c)
{
	//////////////////////
	// muic
	//////////////////////
	sm5038_muic_shutdown();

	//////////////////////
	// fg
	//////////////////////

	//////////////////////
	// charger
	//////////////////////
	sm5038_charger_shutdown();
	
	//////////////////////
	// fled
	//////////////////////

	return;
}

static const struct i2c_device_id sm5038_i2c_id[] = {
	{ SM5038_DEV_NAME, TYPE_SM5038 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sm5038_i2c_id);

static struct of_device_id sm5038_i2c_dt_ids[] = {
	{ .compatible = "siliconmitus,sm5038mfd" },
	{ },
};
MODULE_DEVICE_TABLE(of, sm5038_i2c_dt_ids);

static int sm5038_suspend(struct device *dev)
{
	//struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	//struct sm5038_dev *sm5038 = i2c_get_clientdata(i2c);
	struct sm5038_dev *sm5038 = static_sm5038_data;

	pr_info("%s: %s : sm5038->dev=%p\n", SM5038_DEV_NAME, __func__, sm5038->dev);
	pr_info("%s:%s : disable_irq skip\n", SM5038_DEV_NAME, __func__);
/*
	if (device_may_wakeup(sm5038->dev))
		enable_irq_wake(sm5038->irq);

	disable_irq(sm5038->irq);
*/
	return 0;
}

static int sm5038_resume(struct device *dev)
{
	//struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	//struct sm5038_dev *sm5038 = i2c_get_clientdata(i2c);
	struct sm5038_dev *sm5038 = static_sm5038_data;

	pr_info("%s: %s : sm5038->dev=%p\n", SM5038_DEV_NAME, __func__, sm5038->dev);
	pr_info("%s:%s : enable_irq skip\n", SM5038_DEV_NAME, __func__);
/*
	if (device_may_wakeup(sm5038->dev))
		disable_irq_wake(sm5038->irq);

	enable_irq(sm5038->irq);
*/

	return 0;
}

const struct dev_pm_ops sm5038_pm = {
	.suspend    = sm5038_suspend,
	.resume     = sm5038_resume,
};

static struct i2c_driver sm5038_i2c_driver = {
	.driver		= {
		.name	= SM5038_DEV_NAME,
		.owner	= THIS_MODULE,
		.pm	    = &sm5038_pm,
		.of_match_table	= sm5038_i2c_dt_ids,
	},
	.probe		= sm5038_i2c_probe,
	.remove		= sm5038_i2c_remove,
	.shutdown	= sm5038_i2c_shutdown,
	.id_table	= sm5038_i2c_id,
};

static int __init sm5038_i2c_init(void)
{
	pr_info("%s:%s\n", SM5038_DEV_NAME, __func__);
	return i2c_add_driver(&sm5038_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(sm5038_i2c_init);

static void __exit sm5038_i2c_exit(void)
{
	i2c_del_driver(&sm5038_i2c_driver);
}
module_exit(sm5038_i2c_exit);

MODULE_DESCRIPTION("SM5038 multi-function core driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
