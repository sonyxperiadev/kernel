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

#include <asm/atomic.h>

#include <linux/bh1715.h>
#include <linux/brvsens_driver.h>

//#define BH1715_DEBUG
#define PROC_DEBUG

#ifdef PROC_DEBUG
#include <linux/proc_fs.h>
#endif

/* Client Data */
struct bh1715_data 
{
	struct i2c_client*  this_client;    // i2c client pointer
	atomic_t            mode;           // continuous or single measurement mode
};


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

/* i2c measure. Power must be set ON first & Start measurement command sent.
   after measurement power is automatically turned OFF */
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

	return 0;
}

/* Activate sensor in continuous mode
   After this is invoked, measurements can be performed without flipping
   the power on each time */
static int bh1715_activate
(
   struct bh1715_data* bh1715,    // context -- client data
   u32                 mode       // on/off: In context of bh175 driver this means single vs continuous mode
)
{
    int err = 0;
    
    if (mode > 0)
    {
       err = bh1715_i2c_write(bh1715->this_client, BH1715_PWR_ON);
       if (err == 0)
          err = bh1715_i2c_write(bh1715->this_client, BH1715_CONT_HRES);
    }
    else
    {
       err = bh1715_i2c_write(bh1715->this_client, BH1715_PWR_OFF);
    }
   
    if (err) 
    {
       printk(KERN_ERR "%s: bh1715_i2c_write error\n", __FUNCTION__);
       atomic_set(&(bh1715->mode), BH1715_PWR_OFF);
       
       return err;
    }
  
    // remember mode
    atomic_set(&(bh1715->mode), mode > 0 ? BH1715_CONT_HRES : BH1715_PWR_OFF);
    
    return 0;   // all ok
}

/* read callback:

    1. In single measurement mode:
               turn power on, issue measure command then measure
               
    2. In continuous mode 
               only measure
               
 returns 0 on success, -1 on failure
 
*/
static int bh1715_read_cbk
(
   struct bh1715_data* bh1715,     // context -- client data pointer
   u16*                buffer      // data buffer pointer -- single short
)
{
    if (atomic_read(&(bh1715->mode) ) == BH1715_PWR_OFF)
    {
	    // set power on
		if (bh1715_i2c_write(bh1715->this_client, BH1715_PWR_ON))
		{
			printk(KERN_ERR "%s: bh1715_i2c_write ERROR\n", __FUNCTION__); 
			return -1;
		}
		
	 	// issue measure command
	 	if (bh1715_i2c_write(bh1715->this_client, BH1715_CONT_HRES) )
		{
			printk(KERN_ERR "%s: bh1715_i2c_write error\n", __FUNCTION__);
			return -1;
		} 
    }
 	
    // now measure data
	if (bh1715_i2c_measure(bh1715->this_client, buffer))
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
   void*                data     // user data
)
{
	u16 val;
	int rc, num_args, cmd;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];
	struct bh1715_data* bh1715 = (struct bh1715_data*)data;

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

	if (bh1715_i2c_write(bh1715->this_client, (unsigned char) cmd))
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
	
		if (bh1715_i2c_measure(bh1715->this_client, &val))
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
	u16 val = 0;   
	struct bh1715_data* bh1715 = 0;   // client data
		
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
	
	// allocate client data 
    bh1715 = kzalloc(sizeof(struct bh1715_data), GFP_KERNEL);
	if (!bh1715) 
	{
		printk(KERN_ERR "%s:Failed to allocate memory for Module Data\n", __FUNCTION__);
		return -ENOMEM;
	}
	
	// connect the dots
	bh1715->this_client = client;
	atomic_set(&(bh1715->mode), BH1715_PWR_OFF);   
	
	i2c_set_clientdata(client, bh1715);
	
	// perform single measurement to verify hardware works
	ret = bh1715_read_cbk(bh1715, &val);
	if (ret != 0)
	{
	   printk(KERN_ERR "%s: functionality check failed\n", __FUNCTION__);
	   ret = -ENODEV;
	   
	   kfree (bh1715);
	   goto on_error;
	}

#ifdef PROC_DEBUG
	proc_bh1715 = create_proc_entry(PROC_ENTRY_BH1715, 0644, NULL);
	if (proc_bh1715 == NULL) 
	{
		printk(KERN_ERR "bh1715/debug driver procfs failed\n");
		ret = -ENOMEM;
		
		kfree (bh1715);
		goto on_error;
	}
	
	proc_bh1715->write_proc = proc_debug_bh1715;
	proc_bh1715->data       = bh1715;
	
	printk(KERN_DEBUG "bh1715/debug driver procfs OK\n");
#endif


	/* Register with "brvsens" driver 
	   Note: Initially, after consultation with Blair I am implementing
	   this in "non-continuous" mode, in which case power needs to be
	   turned on/off for each measurement. In the future if you desire different configuration
	   simply pass bh1715_activate callback (recommend: per-board setting in board_template.c)
	*/
    brvsens_register(SENSOR_HANDLE_LIGHT,              // sensor UID
                     BH1715_DRV_NAME,                  // human readable name
                     (void*)bh1715,                    // context; passed back in read/activate callbacks
                     //(PFNACTIVATE)bh1715_activate,   // activate callback 
                     0,                         
                     (PFNREAD)bh1715_read_cbk);        // read callback
        
    printk(KERN_DEBUG "bh1715 driver register OK\n");             
	return 0;
  
	
on_error:
	printk(KERN_ERR "%s failed\n", __FUNCTION__);
	return ret;
}

/* Standard Module "remove" implementation */
static int __devexit bh1715_remove(struct i2c_client* client)
{
	struct bh1715_data* bh1715 = i2c_get_clientdata(client);
	
	kfree(bh1715);
	
	return 0;
}

static const struct i2c_device_id bh1715_id[] = 
{
	{ BH1715_DRV_NAME, 0 },
	{ }
};

static struct i2c_driver bh1715_driver = 
{
	.probe 		= bh1715_probe,
	.remove     = bh1715_remove,
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
