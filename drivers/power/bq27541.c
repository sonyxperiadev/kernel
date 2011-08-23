/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * TI BQ27541 I2C Battery Monitor Driver
 *
 * BQ27541 is a battery monitoring device that has built-in I2C interface for
 * monitoring voltage and temperature.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

#include <linux/hrtimer.h>
#include <asm/io.h>

#include <linux/broadcom/bq27541.h>


/************
 * CONSTANTS
 ************/
 
#define NUM_BYTES_TO_WR                 1
#define NUM_BYTES_TO_RD                 2


/*************
 * STRUCTURES
 *************/
 
struct i2c_priv_data
{
        struct i2c_client *p_i2c_client;
};


/********************
 * PRIVATE VARIABLES
 ********************/
 
static const __devinitconst char gBanner[] = 
        KERN_ERR "BQ27541 Battery Driver: 1.00\n";
static struct i2c_priv_data *gp_i2c_priv    = NULL;
static int mod_debug = 0;
module_param(mod_debug, int, 0644);


/************
 * FUNCTIONS
 ************/

/* 
 * Generic function for sending i2c command to read battery status 
 */
int i2c_bq27541_cmd(int cmd, int *value)
{
        int rc = 0;
        char buffer[NUM_BYTES_TO_RD];

        if (value == NULL) {
                printk(KERN_ERR "%s() value == NULL\n", __FUNCTION__);
                return -1;
        }

        if (gp_i2c_priv == NULL ||
            gp_i2c_priv->p_i2c_client == NULL)
        {
                printk(KERN_ERR "%s() gp_i2c_priv->p_i2c_client == NULL\n",
                       __FUNCTION__);
                return -1;
        }

        /* Have to do a write to set the register index. */
        buffer[0] = cmd;
        rc = i2c_master_send(gp_i2c_priv->p_i2c_client,
                             buffer, NUM_BYTES_TO_WR);

        if (rc < NUM_BYTES_TO_WR) {
                printk("%s i2c_master_send failed %d\n", 
                       I2C_BQ27541_DRIVER_NAME, rc);
                return -ENODEV;
        }

        msleep(1); /* the device requires some delays between write and read */
        rc = i2c_master_recv(gp_i2c_priv->p_i2c_client,
                             buffer, NUM_BYTES_TO_RD);

        if (rc < NUM_BYTES_TO_RD) {
                printk("%s i2c_master_recv failed %d\n", 
                       I2C_BQ27541_DRIVER_NAME, rc);
                return -ENODEV;
        }

        *value = (buffer[1] << 8) | buffer[0];
        if (mod_debug) {
                printk("%s 0x%4x: %5d (0x%4x)\n", 
                       __FUNCTION__, cmd, *value, *value);
        }

        return 0;
}

/*** Kernel registration routines ***/

static int __devinit bq27541_probe(struct i2c_client *p_i2c_client,
                                   const struct i2c_device_id *id)
{
        int rc = 0;
        struct device *dev;

        printk(gBanner);
        
        if (p_i2c_client == NULL) {
                printk(KERN_ERR "%s:%s p_i2c_client == NULL\n",
                       I2C_BQ27541_DRIVER_NAME, __FUNCTION__);
                return -1;
        }
        
        dev = &p_i2c_client->dev;
        if (dev == NULL) {
                printk(KERN_ERR "%s:%s p_i2c_client->dev == NULL\n",
                       I2C_BQ27541_DRIVER_NAME, __FUNCTION__);
                return -1;
        }


        gp_i2c_priv->p_i2c_client = p_i2c_client;
        i2c_set_clientdata(p_i2c_client, gp_i2c_priv);

        return rc;
}

static int bq27541_remove(struct i2c_client *client)
{
        /* Free all the memory that was allocated. */
        if (gp_i2c_priv != NULL) {
                kfree(gp_i2c_priv);
        }

        return 0;
}

static struct i2c_device_id bq27541_i2c_idtable[] = {
        { I2C_BQ27541_DRIVER_NAME, 0 },
        { }
};

static struct i2c_driver bq27541_i2c = {
        .driver = {
                .name   = I2C_BQ27541_DRIVER_NAME,
        },
        .id_table       = bq27541_i2c_idtable,
        .class          = I2C_CLASS_HWMON,
        .probe          = bq27541_probe,
        .remove         = bq27541_remove,
};

int __init i2c_bq27541_init(void)
{
        int rc;

        gp_i2c_priv = kmalloc(sizeof(struct i2c_priv_data), GFP_KERNEL);

        if (gp_i2c_priv == NULL) {
                printk(KERN_ERR "i2c_bq27541_init(): "
                       "memory allocation failed for gp_i2c_priv!\n");
                return -ENOMEM;
        }

        rc = i2c_add_driver(&bq27541_i2c);
        if (rc != 0) {
                printk(KERN_ERR "%s i2c_bq27541_init(): i2c_add_driver() failed, errno is %d\n", I2C_BQ27541_DRIVER_NAME, rc);
                return rc;
        }

        return rc;
}

static void __exit i2c_bq27541_exit(void)
{
        i2c_del_driver(&bq27541_i2c);
}

MODULE_DESCRIPTION("I2C bq27541 driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(i2c_bq27541_cmd);

module_init(i2c_bq27541_init);
module_exit(i2c_bq27541_exit);
