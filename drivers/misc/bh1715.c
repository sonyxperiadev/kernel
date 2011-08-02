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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <asm/uaccess.h>
#include <linux/poll.h>

#include <linux/bh1715.h>
#include <linux/brvsens_driver.h>

//#define BH1715_DEBUG
//#define PROC_DEBUG

#ifdef PROC_DEBUG
#include <linux/proc_fs.h>
#endif


/*	i2c write routine for bh1715	*/
static inline char bh1715_i2c_write(struct i2c_client* client, unsigned char cmd)
{
	s32 res;
#ifdef BH1715_DEBUG
	printk(KERN_DEBUG "%s: write cmd 0x%x\n", __FUNCTION__, cmd);
#endif

	res = i2c_master_send(client, &cmd, 1);
	if (res < 0)
	{
		printk(KERN_ERR "%s: i2c_master_send failed\n", __FUNCTION__);
		return -EFAULT;
	}
	
	return 0;
}

/* i2c measure. Power must be set ON first; after measurement it is
   automatically turned OFF */
static inline char bh1715_i2c_measure(struct i2c_client* client, u16* res)
{
	u32 tmp;
	unsigned char buff[2];
	
	if (i2c_master_recv(client, buff, 2) < 0)
	{
		printk(KERN_ERR "%s: i2c_master_recv failed\n", __FUNCTION__);
		return -EFAULT;
	}
	
	tmp = ((buff[0] << 8) | buff[1]) * 10 / 12;
	*res = (u16) tmp;
	
#ifdef BH1715_DEBUG
	printk(KERN_DEBUG "%s: measure %u\n", __FUNCTION__, *res);
#endif

	return 0;
}

// read callback -- turn power on, then measure
// returns 0 on success, -1 on failure
static int bh1715_read_cbk
(
   struct i2c_client* client,    // context -- i2c client
   u16*               buffer     // data buffer pointer -- single short
)
{
    // set power on
	if (bh1715_i2c_write(client, BH1715_PWR_ON))
	{
		printk(KERN_ERR "%s: bh1715_i2c_write ERROR\n", __FUNCTION__); 
		return -1;
	}

	if (bh1715_i2c_measure(client, buffer))
	{
		printk(KERN_ERR "%s: bh1715_i2c_measure ERROR\n", __FUNCTION__); 
		return -1;
	}

	return 0;
}  
   
// PROCFS entry for Debug / Power management purposes
#ifdef PROC_DEBUG

#define MAX_PROC_BUF_SIZE  32
#define PROC_ENTRY_BH1715  BH1715_DRV_NAME
static struct proc_dir_entry* proc_bh1715;

static int proc_debug_bh1715
(
   struct file*         file, 
   const char __user*   buffer,
   unsigned long        count, 
   void*                data     // user data -- i2c_client pointer
)
{
	u16 val;
	int rc, num_args, cmd;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE) 
	{
		count = MAX_PROC_BUF_SIZE;
	}

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) 
	{
		printk(KERN_ERR "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	num_args = sscanf(kbuf, "%i", &cmd);
	if (num_args != 1)
	{
		printk(KERN_ERR "echo <cmd> > /proc/%s\n", PROC_ENTRY_BH1715);
		return count;
	}

	if (bh1715_i2c_write( (struct i2c_client*)data, (unsigned char) cmd))
	{
		printk(KERN_ERR "%s: write error\n", __FUNCTION__);
		return count;
	}

	switch(cmd) 
	{
	case BH1715_CONT_HRES:
	case BH1715_CONT_LRES:
	case BH1715_ONET_HRES:
	case BH1715_ONET_LRES:
	
		if (bh1715_i2c_measure((struct i2c_client*)data, &val))
			printk(KERN_ERR "%s: measurement read error\n", __FUNCTION__);
		else
			printk(KERN_DEBUG "%s: measurement %u\n", __FUNCTION__, val);
			
		break;
	}
	
	printk(KERN_DEBUG "%s: OK\n", __FUNCTION__);
	return count;
}
#endif

static int bh1715_probe
(
   struct i2c_client*          client, 
   const struct i2c_device_id* id
)
{
	int ret = 0;
		
#ifdef BH1715_DEBUG
	printk(KERN_INFO "%s: entry\n", __FUNCTION__);
#endif

    /* Check i2c and register bh1715 device */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk(KERN_ERR "%s: functionality check failed\n", __FUNCTION__);
		ret = -ENODEV;
		goto on_error;
	}
	

#ifdef PROC_DEBUG
	proc_bh1715 = create_proc_entry(PROC_ENTRY_BH1715, 0644, NULL);
	if (proc_bh1715 == NULL) 
	{
		printk(KERN_ERR "bh1715/debug driver procfs failed\n");
		ret = -ENOMEM;
		
		goto on_error;
	}
	
	proc_bh1715->write_proc = proc_debug_bh1715;
	proc_bh1715->data       = client;
	
	printk(KERN_DEBUG "bh1715/debug driver procfs OK\n");
#endif


	/* Register with "brvsens" driver */
    brvsens_register(SENSOR_HANDLE_LIGHT,              // sensor UID
                     BH1715_DRV_NAME,                  // human readable name
                     (void*)client,                    // context; passed back in read/activate callbacks
                     0,                                // activate callback -- no need as chip is turned off after each measurement
                     (PFNREAD)bh1715_read_cbk);        // read callback
                     
	return 0;

	
on_error:
	printk(KERN_ERR "%s failed\n", __FUNCTION__);
	return ret;
}


static const struct i2c_device_id bh1715_id[] = 
{
	{ BH1715_DRV_NAME, 0 },
	{ }
};

static struct i2c_driver bh1715_driver = 
{
	.probe 		= bh1715_probe,
	.id_table	= bh1715_id,
	.driver 	= 
	{
		.owner	= THIS_MODULE,
		.name	= BH1715_DRV_NAME,
	},
};

static int __init bh1715_init(void)
{
	return i2c_add_driver(&bh1715_driver);
}

static void __exit bh1715_exit(void)
{
	i2c_del_driver(&bh1715_driver);
}

module_init(bh1715_init);
module_exit(bh1715_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BH1715 Ambient Light Sensor Driver");
MODULE_LICENSE("GPL");
