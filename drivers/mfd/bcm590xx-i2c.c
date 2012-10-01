/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/core.h>

static int bcm590xx_i2c_read_device(struct bcm590xx *bcm590xx, char reg, int i)
{
	return i2c_smbus_read_byte_data(bcm590xx->i2c_client[i].client, reg);
}

static int bcm590xx_i2c_mul_read_device(struct bcm590xx *bcm590xx, char reg, int len, u8 *val, int i)
{
	int ret;
	ret = i2c_smbus_read_i2c_block_data(bcm590xx->i2c_client[i].client, reg, len, val);
	if (ret < len)
	    return -EIO;
	return ret;
}

// static int bcm590xx_i2c_write_device(struct bcm590xx *bcm590xx, char reg, int bytes, void *value)
static int bcm590xx_i2c_write_device(struct bcm590xx *bcm590xx, char reg, int bytes, u8 value, int i)
{
	return i2c_smbus_write_byte_data(bcm590xx->i2c_client[i].client, reg, value);
}

static int bcm590xx_i2c_mul_write_device(struct bcm590xx *bcm590xx, char reg, int len, u8 *val, int i)
{
	int ret;
	ret = i2c_smbus_write_i2c_block_data(bcm590xx->i2c_client[i].client, reg, len, val);
	return ret;
}

static int bcm590xx_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct bcm590xx *bcm590xx;
	int ret = 0;
	struct bcm590xx_platform_data *pdata = i2c->dev.platform_data;

	printk("REG : bcm590xx_i2c_probe called \n");

	bcm590xx = kzalloc(sizeof(struct bcm590xx), GFP_KERNEL);
	if (bcm590xx == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c, bcm590xx);
	bcm590xx->dev = &i2c->dev;
	bcm590xx->i2c_client[0].client = i2c;
	bcm590xx->i2c_client[0].addr = i2c->addr;
	bcm590xx->irq = i2c->irq;
	bcm590xx->read_dev = bcm590xx_i2c_read_device;
	bcm590xx->mul_read_dev = bcm590xx_i2c_mul_read_device;
	bcm590xx->write_dev = bcm590xx_i2c_write_device;
	bcm590xx->mul_write_dev = bcm590xx_i2c_mul_write_device;
	bcm590xx->pdata = pdata ;

	ret = bcm590xx_device_init(bcm590xx, i2c->irq, i2c->dev.platform_data);
	if (ret < 0)
		goto err;
    return ret;


err:
    return ret;
}

static int bcm590xx_i2c_remove(struct i2c_client *i2c)
{
	struct bcm590xx *bcm590xx = i2c_get_clientdata(i2c);

	bcm590xx_device_exit(bcm590xx);

	return 0;
}

static const struct i2c_device_id bcm590xx_i2c_id[] = {
       { "bcm59035", 0 },
       { "bcm59038", 0 },
       { "bcm59055", 0 },
       /* for new supported chip add a entry here */
       { }
};
MODULE_DEVICE_TABLE(i2c, bcm590xx_i2c_id);


static struct i2c_driver bcm590xx_i2c_driver = {
	.driver = {
		.name = "bcm590xx",
		.owner = THIS_MODULE,
	},
	.probe = bcm590xx_i2c_probe,
	.remove = bcm590xx_i2c_remove,
	.id_table = bcm590xx_i2c_id,
};

static int __init bcm590xx_i2c_init(void)
{
	return i2c_add_driver(&bcm590xx_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(bcm590xx_i2c_init);

static void __exit bcm590xx_i2c_exit(void)
{
	i2c_del_driver(&bcm590xx_i2c_driver);
}
module_exit(bcm590xx_i2c_exit);

MODULE_DESCRIPTION("I2C support for the BCM590xx PMIC");
MODULE_LICENSE("GPL");
