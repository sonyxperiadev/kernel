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

//#define BH1715_DEBUG
//#define PROC_DEBUG

#ifdef PROC_DEBUG
#include <linux/proc_fs.h>
#endif

static struct i2c_client* bh1715_client = NULL;

/* Each client has this additional data */
struct bh1715_data
{
	atomic_t pollrate;      /* poll rate [msec] -- sensor library multiplexing */    
};

/* Helper to manage poll rate setting */
static inline void SetPollRate
(
   struct bh1715_data* bh1715data,  // client data structure pointer
   u32                 msec          // poll rate in milliseconds
)
{
   atomic_set(&(bh1715data->pollrate), msec);
}

/*	i2c write routine for bh1715	*/
static inline char bh1715_i2c_write(unsigned char cmd)
{
	s32 res;
#ifdef BH1715_DEBUG
	printk(KERN_DEBUG "%s: write cmd 0x%x\n", __FUNCTION__, cmd);
#endif
	if (bh1715_client == NULL)	/*	No global client pointer?	*/
	{
		printk(KERN_ERR "%s: client is not set\n", __FUNCTION__);
		return -EFAULT;
	}

	res = i2c_master_send(bh1715_client, &cmd, 1);
	if(res < 0)
	{
		printk(KERN_ERR "%s: i2c_master_send failed\n", __FUNCTION__);
		return -EFAULT;
	}
	return 0;
}

static inline char bh1715_i2c_measure(u16* res)
{
	u32 tmp;
	unsigned char buff[2];

	if (bh1715_client == NULL)	/*	No global client pointer?	*/
	{
		printk(KERN_ERR "%s: client is not set\n", __FUNCTION__);
		return -EFAULT;
	}
	
	if (i2c_master_recv(bh1715_client, buff, 2) < 0)
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

static ssize_t bh1715_read(struct file* f, char __user* buf, size_t sz, loff_t* off)
{
	u16 res;
	
	// check global client pointer
	if (bh1715_client == NULL)
	{
		printk(KERN_ERR "%s: I2C driver not installed\n", __FUNCTION__); 
		return -1;
	}

	// set power on
	if (bh1715_i2c_write(BH1715_PWR_ON))
	{
		printk(KERN_ERR "%s: bh1715_i2c_write ERROR\n", __FUNCTION__); 
		return -1;
	}

	if (bh1715_i2c_measure(&res))
	{
		printk(KERN_ERR "%s: bh1715_i2c_measure ERROR\n", __FUNCTION__); 
		return -1;
	}

	copy_to_user(buf, &res, sizeof(u16));

	return sizeof(u16);
}

static unsigned int bh1715_poll(struct file* file, struct poll_table_struct* tab)
{
	struct bh1715_data* data = 0;

	if (bh1715_client == NULL)
	{ 
		msleep_interruptible(BH1715_POLL_RATE_MSEC);
		return POLLERR;
	}
	
	data = i2c_get_clientdata(bh1715_client);
	msleep_interruptible(atomic_read(&(data->pollrate)  ) );
	
	return POLLIN;
}

static long bh1715_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 int_val = 0;
	unsigned char char_val;
	unsigned char data[4];
	int err = 0;

	/* check cmd */
	if (_IOC_TYPE(cmd) != BH1715_IOC_MAGIC)	
	{
		printk(KERN_ERR "%s: cmd magic type error\n", __FUNCTION__);
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
		
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
		
	if (err)
	{
		printk(KERN_ERR "%s: cmd access error\n", __FUNCTION__);
		return -EFAULT;
	}

	if (bh1715_client == NULL)
	{
		printk(KERN_ERR "%s: I2C driver not installed\n", __FUNCTION__); 
		return -EFAULT;
	}

    /* cmd mapping */
	switch (cmd) 
	{
	case BH1715_IOC_PWR_OFF:
		return bh1715_i2c_write(BH1715_PWR_OFF);

	case BH1715_IOC_PWR_ON:
		return bh1715_i2c_write(BH1715_PWR_ON);

	case BH1715_IOC_SET_MODE:
		if (copy_from_user(&char_val, (unsigned char*)arg, 1) != 0)
		{
			printk(KERN_ERR "%s: copy_from_user error\n", __FUNCTION__);
			return -EFAULT;
		}
		
#ifdef BH1715_DEBUG			
		printk(KERN_INFO "BH1715_IOC_SET_MODE 0x%x\n", char_val);
#endif
		return bh1715_i2c_write(char_val);

	/* Set poll rate */	 
	case BH1715_SET_POLL_RATE:
		if (copy_from_user((unsigned int*)data, (unsigned int*)arg, sizeof(unsigned int) ) != 0)
		{
			printk(KERN_ERR "bmp085_ioctl::BH1715_SET_POLL_RATE -- copy_from_user error\n");
			return -EFAULT;
		}
		
		// user delay in usec
		memcpy(&int_val, data, 4);
		SetPollRate(i2c_get_clientdata(bh1715_client), int_val);


#ifdef BH1715_DEBUG	
		printk("++++ BH1715_SET_POLL_RATE -- OK. Delay set to: [%d] microseconds ++++\n", int_val);
#endif
		return 0;
		
    case BH1715_SET_ENABLE:
	   {
		   /* Note: According to State Transition Diagram on RohM Datasheet, each read (single or continuous)
	                requires turning the power on, then doing the read. After the read, power is turned off
	                Thus flipping the power on/off as part of Enable has no effect? */

#ifdef BH1715_DEBUG	
		   if (copy_from_user(data, (unsigned char*)arg, sizeof(unsigned char) ) != 0)
		   {
			  printk(KERN_ERR "[%s]::copy_from_user error\n", __FUNCTION__);
			  return -EFAULT;
		   }
		   
           printk("++++ BH1715_SET_ENABLE -- OK. Set to: {%d} ++++\n", *data);
#endif
		   return 0;
		}	 
		
	default:
	    printk(KERN_ERR "[%s]::Cmd number error", __FUNCTION__); 
		return -ENOTTY;
	}
	
	// never reached
}

static struct file_operations bh1715_fops = 
{
	.owner          = THIS_MODULE,
	.read           = bh1715_read,
	.poll           = bh1715_poll,
	.unlocked_ioctl = bh1715_ioctl,
};

static struct miscdevice bh1715_device = 
{
	.minor = MISC_DYNAMIC_MINOR,
	.name  = BH1715_DRV_NAME,
	.fops  = &bh1715_fops,
};

#ifdef PROC_DEBUG
#define MAX_PROC_BUF_SIZE  32
#define PROC_ENTRY_BH1715  "bh1715"
static struct proc_dir_entry *proc_bh1715;

static int proc_debug_bh1715(struct file *file, const char __user *buffer, unsigned long count, void *data)
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

	if (bh1715_i2c_write((unsigned char) cmd))
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
	
		if (bh1715_i2c_measure(&val))
			printk(KERN_ERR "%s: measurement read error\n", __FUNCTION__);
		else
			printk(KERN_DEBUG "%s: measurement %u\n", __FUNCTION__, val);
			
		break;
	}
	
	printk(KERN_DEBUG "%s: OK\n", __FUNCTION__);
	return count;
}
#endif

static int bh1715_probe(struct i2c_client* client, const struct i2c_device_id* id)
{
	int ret = 0;
	struct bh1715_data* data = 0;
		
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

	ret = misc_register(&bh1715_device);
	if (ret) 
	{
		printk(KERN_ERR "%s: bh1715_device register failed\n", __FUNCTION__);
		goto on_error;
	}
	

#ifdef PROC_DEBUG
	proc_bh1715 = create_proc_entry(PROC_ENTRY_BH1715, 0644, NULL);
	if (proc_bh1715 == NULL) 
	{
		printk(KERN_ERR "bh1715/debug driver procfs failed\n");
		ret = -ENOMEM;
		goto on_error_with_deregister;
	}
	
	proc_bh1715->write_proc = proc_debug_bh1715;
	printk(KERN_DEBUG "bh1715/debug driver procfs OK\n");
#endif

	/* allocate and register client data */
	if (!(data = kmalloc(sizeof(struct bh1715_data), GFP_KERNEL)))
	{
		ret = -ENOMEM;
		printk(KERN_INFO "kmalloc error\n");
		
		goto on_error_with_deregister;
	}
	
	memset(data, 0, sizeof(struct bh1715_data));
	i2c_set_clientdata(client, data);
	
	/* set global client pointer */
	bh1715_client = client;
	
	 /* Initial default poll rate */
    SetPollRate(data, BH1715_POLL_RATE_MSEC);
    
	return 0;

on_error_with_deregister:
	misc_deregister(&bh1715_device);
	
on_error:
	printk(KERN_ERR "%s failed\n", __FUNCTION__);
	return ret;
}

static int bh1715_remove(struct i2c_client *client)
{
	misc_deregister(&bh1715_device);
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
	.remove 	= bh1715_remove,
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
