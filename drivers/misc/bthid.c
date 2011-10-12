/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/hid.h>
#include <linux/version.h>


MODULE_AUTHOR("Daniel McDowell <mcdowell@broadcom.com>");
MODULE_DESCRIPTION("User level driver support for Bluetooth HID input");
MODULE_SUPPORTED_DEVICE("bthid");
MODULE_LICENSE("GPL");


#define BTHID_NAME              "bthid"
#define BTHID_MINOR             224
#define BTHID_IOCTL_RPT_DSCP    1
#define BTHID_MAX_CTRL_BUF_LEN  508


struct bthid_ctrl {
    int   size;
    char  buf[BTHID_MAX_CTRL_BUF_LEN];
};

struct bthid_device {
    struct input_dev   *dev;
    struct hid_device  *hid;
    int                dscp_set;
};


static int bthid_ll_start(struct hid_device *hid)
{
    printk("######## bthid_ll_start: hid = %p ########\n", hid);
    return 0;
}

static void bthid_ll_stop(struct hid_device *hid)
{
    printk("######## bthid_ll_stop: hid = %p ########\n", hid);
}

static int bthid_ll_open(struct hid_device *hid)
{
    printk("######## bthid_ll_open: hid = %p ########\n", hid);
    return 0;
}

static void bthid_ll_close(struct hid_device *hid)
{
    printk("######## bthid_ll_close: hid = %p ########\n", hid);
}

static int bthid_ll_hidinput_event(struct input_dev *dev, unsigned int type, 
                                   unsigned int code, int value)
{
    /*
    printk("######## bthid_ll_hidinput_event: dev = %p, type = %d, code = %d, value = %d ########\n",
           dev, type, code, value);
    */
    return 0;
}

static int bthid_ll_parse(struct hid_device *hid)
{
    int ret;
    unsigned char *buf;
    struct bthid_ctrl *p_ctrl = hid->driver_data;

    printk("######## bthid_ll_parse: hid = %p ########\n", hid);
    
    buf = kmalloc(p_ctrl->size, GFP_KERNEL);
    if (!buf)
    {
        return -ENOMEM;
    }

    memcpy(buf, p_ctrl->buf, p_ctrl->size);

    ret = hid_parse_report(hid, buf, p_ctrl->size);
    kfree(buf);

    printk("######## bthid_ll_parse: hid_parse_report() = %d ########\n", ret);
    return ret;
}

static struct hid_ll_driver bthid_ll_driver = {
    .start                = bthid_ll_start,
    .stop                 = bthid_ll_stop,
    .open                 = bthid_ll_open,
    .close                = bthid_ll_close,
    .hidinput_input_event = bthid_ll_hidinput_event,
    .parse                = bthid_ll_parse,
};


static int bthid_open(struct inode *inode, struct file *file)
{
    struct bthid_device *p_dev;

    printk("######## bthid_open: ########\n");

    p_dev = kzalloc(sizeof(struct bthid_device), GFP_KERNEL);
    if (!p_dev)
    {
        return -ENOMEM;
    }

    file->private_data = p_dev;
    
    printk("######## bthid_open: done ########\n");
    return 0;
}

static int bthid_release(struct inode *inode, struct file *file)
{
    struct bthid_device *p_dev = file->private_data;

    printk("######## bthid_release: ########\n");
    
    if (p_dev->hid) 
    {
        if (p_dev->hid->driver_data != NULL)
        {
            kfree(p_dev->hid->driver_data);
        }

        hidinput_disconnect(p_dev->hid);
        hid_destroy_device(p_dev->hid);
        p_dev->hid = NULL;
    }

    kfree(p_dev);
    file->private_data = NULL;

    printk("######## bthid_release: done ########\n");
    return 0;
}

static ssize_t bthid_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    unsigned char *buf;
    struct bthid_device *p_dev = file->private_data;
    int result;

    printk("######## bthid_write: count = %d ########\n", count);

    if (p_dev->dscp_set == 0)
    {
        printk("bthid_write: Oops, HID report descriptor not configured\n");
        return 0;
    }

    buf = kmalloc(count + 1, GFP_KERNEL);
    if (!buf)
    {
        return -ENOMEM;
    }

    if (copy_from_user(buf, buffer, count))
    {
        kfree(buf);
        return -EFAULT;
    }

    if (p_dev->hid) 
    {
        result = hid_input_report(p_dev->hid, HID_INPUT_REPORT, buf, count, 1);
	printk("hid_input_report = %d\n",result);
    }

    kfree(buf);

    /*
    printk("######## bthid_write: done ########\n");
    */

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int bthid_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long bthid_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    struct bthid_ctrl *p_ctrl;
    struct bthid_device *p_dev = file->private_data;

    printk("######## bthid_ioctl: cmd = %d ########\n", cmd);

    if (cmd != BTHID_IOCTL_RPT_DSCP || p_dev == NULL)
    {
        return -EINVAL;
    }

    p_ctrl = kmalloc(sizeof(struct bthid_ctrl), GFP_KERNEL);
    if (p_ctrl == NULL)
    {
        return -ENOMEM;
    }

    if (copy_from_user(p_ctrl, (void __user *) arg, sizeof(struct bthid_ctrl)) != 0)
    {
        kfree(p_ctrl);
        return -EFAULT;
    }

    if (p_ctrl->size <= 0) 
    {
        printk("Oops: Invalid BT HID report descriptor size %d\n", p_ctrl->size); 

        kfree(p_ctrl);
        return -EINVAL;
    }
    
    p_dev->hid = hid_allocate_device();
    if (p_dev->hid == NULL)
    {
        printk("Oops: Failed to allocation HID device.\n");

        kfree(p_ctrl);
        return -ENOMEM;
    }
    
    p_dev->hid->bus         = BUS_BLUETOOTH;
    p_dev->hid->vendor      = 0;
    p_dev->hid->product     = 0;
    p_dev->hid->version     = 0;
    p_dev->hid->country     = 0;
    p_dev->hid->ll_driver   = &bthid_ll_driver;
    p_dev->hid->driver_data = p_ctrl;

    strcpy(p_dev->hid->name, "Broadcom Bluetooth HID");

    if (hid_add_device(p_dev->hid) != 0)
    {
        printk("Oops: Failed to add HID device\n");

        kfree(p_ctrl);
        hid_destroy_device(p_dev->hid);
        p_dev->hid = NULL;
        return -EINVAL;
    }

    // TBD: don't know whether it is required
    if (hidinput_connect(p_dev->hid, 0) == 0) 
    {
        printk("bthid_ioctl: HID Interface Claimed...\n");
        p_dev->hid->claimed |= HID_CLAIMED_INPUT;
    }
    p_dev->dscp_set = 1;

    printk("######## bthid_ioctl: done ########\n");
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
#   ifdef CONFIG_COMPAT
static long bthid_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return bthid_compat_ioctl( file,
	                           cmd,
	                           (unsigned long)compat_ptr(arg) );
}
#   endif
#endif


#define BTHID_NAME0 "BtHid"

static const struct hid_device_id bthid_table[] = {
    { HID_BLUETOOTH_DEVICE(HID_ANY_ID, HID_ANY_ID) },
    { }
};

static struct hid_driver bthid_driver = {
    .name     = BTHID_NAME0,
    .id_table = bthid_table,
};


static const struct file_operations bthid_fops = {
    .owner   = THIS_MODULE,
    .open    = bthid_open,
    .release = bthid_release,
    .write   = bthid_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl   = bthid_ioctl,
#else
    .unlocked_ioctl = bthid_ioctl,
#   ifdef CONFIG_COMPAT
    .compat_ioctl = bthid_compat_ioctl,
#   endif
#endif
};

static struct miscdevice bthid_misc = {
    .name  = BTHID_NAME,
    .minor = BTHID_MINOR,
    .fops  = &bthid_fops,
};


static int __init bthid_init(void)
{
    int ret;

    printk("######## bthid_init: ########\n");

    ret = misc_register(&bthid_misc);
    if (ret != 0)
    {
        printk("Oops, failed to register Misc driver, ret = %d\n", ret);
        return ret;
    }

    ret = hid_register_driver(&bthid_driver);
    if (ret != 0)
    {
        printk("Oops, failed to register HID driver, ret = %d\n", ret);
        return ret;
    }

    printk("######## bthid_init: done ########\n");
    
    return ret;
}

static void __exit bthid_exit(void)
{
    printk("bthid_exit:\n");
    hid_unregister_driver(&bthid_driver);
    misc_deregister(&bthid_misc);
    printk("bthid_exit: done\n");
}

late_initcall(bthid_init);
module_exit(bthid_exit);
