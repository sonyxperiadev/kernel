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

struct bcm590xx *g_bcm590xx = NULL ;
static DEFINE_MUTEX(io_mutex);
unsigned int g_alloc = 0 ;

static s32 bcm590xx_i2c_read_device(struct bcm590xx *bcm590xx, enum SLAVE_ID slave, u8 reg)
{
    s32 retval = 0 ; 

    if ( slave == SLAVE_ID0 )
        retval = i2c_smbus_read_byte_data(bcm590xx->i2c_client_0, reg);
    else if ( slave == SLAVE_ID1 )
        retval = i2c_smbus_read_byte_data(bcm590xx->i2c_client_1, reg);
    return retval ;
}

static s32 bcm590xx_i2c_write_device(struct bcm590xx *bcm590xx, enum SLAVE_ID slave, u8 reg, u8 value)
{
    s32 retval = 0 ; 
    
    if ( slave == SLAVE_ID0 )
        retval = i2c_smbus_write_byte_data(bcm590xx->i2c_client_0, reg, value);
    else if ( slave == SLAVE_ID1 )
        retval = i2c_smbus_write_byte_data(bcm590xx->i2c_client_1, reg, value);
    return retval ;
}

s32 bcm590xx_reg_read(enum SLAVE_ID slave, u8 reg) 
{
    s32 retval = 0 ;
    mutex_lock(&io_mutex);
    retval = g_bcm590xx->read_dev(g_bcm590xx, slave, reg);    
    mutex_unlock(&io_mutex);
    return retval ; 
}
EXPORT_SYMBOL_GPL(bcm590xx_reg_read);

s32 bcm590xx_reg_write(enum SLAVE_ID slave, u8 reg, u8 val)
{
    s32 retval = 0 ;
    mutex_lock(&io_mutex);
    retval = g_bcm590xx->write_dev(g_bcm590xx, slave, reg, val);    
    mutex_unlock(&io_mutex);
    return retval ; 
}

static int bcm590xx_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
    int ret = 0;
    struct bcm590xx_platform_data *pdata = i2c->dev.platform_data;

    printk(KERN_INFO "REG : bcm590xx_i2c_probe called \n"); 

    if ( g_alloc == 0 ) 
    {
        g_bcm590xx = kzalloc(sizeof(struct bcm590xx), GFP_KERNEL);
        if (g_bcm590xx == NULL) {
            kfree(i2c);
            return -ENOMEM;
        }
        g_alloc = 1 ; 
    }

    if ( pdata->slave == SLAVE_ID1)
    {
        g_bcm590xx->i2c_client_1 = i2c;
    }
    else if ( pdata->slave == SLAVE_ID0 )
    {
        i2c_set_clientdata(i2c, g_bcm590xx);
        g_bcm590xx->dev = &i2c->dev;
        g_bcm590xx->i2c_client_0 = i2c;
        g_bcm590xx->irq = i2c->irq;
        g_bcm590xx->read_dev = bcm590xx_i2c_read_device;
        g_bcm590xx->write_dev = bcm590xx_i2c_write_device;
        g_bcm590xx->pdata = pdata ;

        ret = bcm590xx_device_init(g_bcm590xx, i2c->irq, i2c->dev.platform_data);
        if (ret < 0)
            goto err;
    }
    return ret;


err:
    kfree(g_bcm590xx);
    return ret;
}

static int bcm590xx_i2c_remove(struct i2c_client *i2c)
{
    struct bcm590xx *bcm590xx = i2c_get_clientdata(i2c);
    bcm590xx_device_exit(bcm590xx);
    kfree(bcm590xx);
    return 0;
}

static const struct i2c_device_id bcm590xx_i2c_id[] = {
    { "bcm590xx", 0 },
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

MODULE_DESCRIPTION("I2C support for the WM8350 AudioPlus PMIC");
MODULE_LICENSE("GPL");
