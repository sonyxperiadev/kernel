/*****************************************************************************
 *  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
 *
 *  Unless you and Broadcom execute a separate written software license
 *  agreement governing use of this software, this software is licensed to you
 *  under the terms of the GNU General Public License version 2, available at
 *  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
 *
 *  Notwithstanding the above, under no circumstances may you combine this
 *  software in any way with any other Broadcom software provided under a
 *  license other than the GPL, without Broadcom's express prior written
 *  consent.
 *
 *****************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mfd/tpa2026d2.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

static int debug_mask = TPA2026D2_PRINT_ERROR \
	| TPA2026D2_PRINT_INIT;

#define pr_tpa_i2c(debug_level, args...) \
	do { \
		if (debug_mask & TPA2026D2_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

static struct platform_device tpa2026d2_pdev = {
	.name = "tpa2026d2_pdev",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct tpa2026d2 *tpa;

/* Return Error codes in i2c read/write interfaces
 *-ENODEV = if wrong register has been passed
 *-ENXIO = if register exists but no map found
 *-EIO = i2c read/write error
 *-EPERM = tpa is not initialized.
*/
static int tpa2026d2_read(u8 reg, u8 *val)
{
	int err;
	if (tpa == NULL)
		return -EPERM;

	mutex_lock(&tpa->bus->i2c_mutex);
	err = i2c_smbus_read_byte_data(tpa->bus->i2c, reg);
	mutex_unlock(&tpa->bus->i2c_mutex);
	if (err < 0)
		return err;
	*val = err;
	return 0;
}

static int tpa2026d2_write(u8 reg, u8 value)
{
	int err;
	if (tpa == NULL)
		return -EPERM;

	mutex_lock(&tpa->bus->i2c_mutex);
	err = i2c_smbus_write_byte_data(tpa->bus->i2c, reg, value);
	mutex_unlock(&tpa->bus->i2c_mutex);
	return (err < 0 ? err : 0);
}

static void tpa2026d2_gpio_output(char *name, int gpio_pin, int value)
{
	int rc = gpio_request(gpio_pin, name);

	if (!rc) {
		pr_tpa_i2c(FLOW, "tpa2026d2_gpio_output, pin %d -> %d\n",
		       gpio_pin, value);

		gpio_direction_output(gpio_pin, 0);
		gpio_set_value(gpio_pin, value);
		gpio_free(gpio_pin);
	} else {
		pr_tpa_i2c(ERROR, "tpa2026d2_gpio_output, error %s, pin %d\n",
			name, gpio_pin);
	}
}

void tpa2026d2_spk_power(int spk_l_on, int spk_r_on)
{
	int shutdown_gpio;
	u8 value = 0;
	int result;
	int reg_spk_l_on = 0;
	int reg_spk_r_on = 0;
	if (tpa == NULL)
		return;

	mutex_lock(&tpa->mutex);

	shutdown_gpio = tpa->pdata->shutdown_gpio;

	if (spk_l_on || spk_r_on) {
		if (!tpa->spk_l_on && !tpa->spk_r_on) {
			tpa2026d2_gpio_output("TPA2026D2_SDZ",
				shutdown_gpio, 1);
			mdelay(5);
		}
		result = tpa2026d2_read(TPA2026D2_REG_IC_FUNCTION, &value);
		if (result == 0) {
			pr_tpa_i2c(FLOW, "tpa2026d2 read = 0x%02X\n", value);
			if ((value & TPA2026D2_SPK_EN_L_MASK)
				== TPA2026D2_SPK_EN_L) {
				reg_spk_l_on = 1;
			}
			if ((value & TPA2026D2_SPK_EN_R_MASK)
				== TPA2026D2_SPK_EN_R) {
				reg_spk_r_on = 1;
			}
			if (spk_l_on && !reg_spk_l_on) {
				value |= TPA2026D2_SPK_EN_L;
				tpa->spk_l_on = 1;
			} else if (!spk_l_on && reg_spk_l_on) {
				value &= ~TPA2026D2_SPK_EN_L;
				tpa->spk_l_on = 0;
			}
			if (spk_r_on && !reg_spk_r_on) {
				value |= TPA2026D2_SPK_EN_R;
				tpa->spk_r_on = 1;
			} else if (!spk_r_on && reg_spk_r_on) {
				value &= ~TPA2026D2_SPK_EN_R;
				tpa->spk_r_on = 0;
			}
			pr_tpa_i2c(FLOW, "tpa2026d2 write 0x%02X\n", value);
			result = tpa2026d2_write(TPA2026D2_REG_IC_FUNCTION,
				value);
			if (result == 0) {
				pr_tpa_i2c(FLOW, "tpa2026d2 write success\n");
			} else {
				pr_tpa_i2c(ERROR, "tpa2026d2 write error %d\n",
					result);
			}
		} else {
			pr_tpa_i2c(ERROR, "tpa2026d2 read error %d\n", result);
		}
	} else {
		tpa2026d2_gpio_output("TPA2026D2_SDZ",
			shutdown_gpio, 0);
		tpa->spk_l_on = 0;
		tpa->spk_r_on = 0;
	}

	mutex_unlock(&tpa->mutex);
}
EXPORT_SYMBOL(tpa2026d2_spk_power);

static int __devinit tpa2026d2_i2c_probe(struct i2c_client *i2c,
				 const struct i2c_device_id *id)
{
	struct tpa2026d2_platform_data *pdata;
	struct tpa2026d2_bus *ic_bus = NULL;
	int ret = 0;

	pdata = (struct tpa2026d2_platform_data *)i2c->dev.platform_data;
	pr_tpa_i2c(INIT, "%s\n", __func__);
	pr_tpa_i2c(INIT, "%s called\n", __func__);
	ic_bus = kzalloc(sizeof(struct tpa2026d2_bus), GFP_KERNEL);
	tpa = kzalloc(sizeof(struct tpa2026d2), GFP_KERNEL);

	if ((tpa == NULL) || (ic_bus == NULL)) {
		pr_tpa_i2c(ERROR, "ERR: %s failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	tpa->bus = ic_bus;
	tpa->pdata = pdata;
	tpa->is_on = 0;
	pr_tpa_i2c(INIT, "%s i2c_bus_id = %d\n", __func__, pdata->i2c_bus_id);

	tpa->bus->i2c = i2c;
	i2c_set_clientdata(i2c, tpa);
	tpa->dev = &tpa->bus->i2c->dev;
	mutex_init(&tpa->bus->i2c_mutex);
	mutex_init(&tpa->mutex);
	dev_set_drvdata(tpa->dev, tpa);

	tpa2026d2_pdev.dev.platform_data = tpa;
	platform_device_register(&tpa2026d2_pdev);
#ifdef CONFIG_DEBUG_FS
	if (!tpa->dent_tpa2026d2) {
		tpa->dent_tpa2026d2 = debugfs_create_dir("tpa2026d2", NULL);
		if (!tpa->dent_tpa2026d2) {
			pr_err("Failed to initialize debugfs tpa2026d2\n");
			return;
		}
	}
	tpa->bus->dentry = debugfs_create_dir("i2c", tpa->dent_tpa2026d2);

	if (!tpa->bus->dentry)
		pr_err("Failed to setup i2c debugfs\n");

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
		tpa->bus->dentry , &debug_mask))
		debugfs_remove(tpa->bus->dentry);
#endif
	return ret;
err:
	kfree(tpa);
	tpa = NULL;
	kfree(ic_bus);
	return ret;
}

static int tpa2026d2_i2c_remove(struct i2c_client *i2c)
{
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(tpa->bus->dentry);
#endif
	platform_device_unregister(&tpa2026d2_pdev);
	kfree(tpa->bus);
	kfree(tpa);
	tpa = NULL;
	return 0;
}

static const struct i2c_device_id tpa2026d2_i2c_id[] = {
	{"tpa2026d2", 0}, {}
};

MODULE_DEVICE_TABLE(i2c, tpa2026d2_i2c_id);
static struct i2c_driver __refdata tpa2026d2_i2c_driver = {
	.driver = {
		   .name = "tpa2026d2",
		   .owner = THIS_MODULE,
		   },
	.probe = tpa2026d2_i2c_probe,
	.remove = tpa2026d2_i2c_remove,
	.id_table = tpa2026d2_i2c_id,
};

static int __init tpa2026d2_i2c_init(void)
{
	return i2c_add_driver(&tpa2026d2_i2c_driver);
}

/* init early so consumer devices can complete system boot */
subsys_initcall(tpa2026d2_i2c_init);

static void __exit tpa2026d2_i2c_exit(void)
{
	i2c_del_driver(&tpa2026d2_i2c_driver);
}
module_exit(tpa2026d2_i2c_exit);

MODULE_DESCRIPTION("I2C support for TPA2026D2");
MODULE_LICENSE("GPL");

