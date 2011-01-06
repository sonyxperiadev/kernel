/*
 *  tv_intf_driver.c - tv_intf driver interface
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This device provides a shared mechanism for writing to the mailboxes,
 * semaphores, doorbells etc. that are shared between the ARM and the VideoCore
 * processor
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>

#include "tv_intf.h"
#include "tv_intf_driver.h"

#define TV_INTF_DRIVER_NAME  "tv_intf"

struct tv_intf_buffers {
	char cmd[TV_INTF_CMD_SIZE];
	char resp[TV_INTF_RESP_SIZE];
	int  resp_len;
	struct semaphore buf_sem;
};


/*driver functions  */
ssize_t tv_intf_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{

	struct tv_intf_buffers *g_buf;
	int ret = 0; 
	int len;

	tv_intf_print("-IN buf=%p count=%d\n", buf, count);
	/* get the buffer */
	g_buf = (struct tv_intf_buffers *) filp->private_data;

	ret = down_interruptible(&g_buf->buf_sem);
	if( ret < 0 ) {
		printk(KERN_ERR"[%s] error in getting buffer semaphore\n", __func__ );
		goto err_lock;
	}

	/* check the length */
	len = g_buf->resp_len;

	if( len > count )
		len = count;

	ret = copy_to_user(buf, g_buf->resp, len);
	if( ret < 0 ) {
		printk(KERN_ERR"[%s] error in copying date to user buffer\n", __func__);
		goto done;
	}
	
	tv_intf_print("tv_intf resp=%s\n", g_buf->resp);
	ret = len;
	
done:
	up(&g_buf->buf_sem);

err_lock:	
	tv_intf_print("-OUT ret=%d\n", ret);
	return ret;
}


ssize_t tv_intf_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct tv_intf_buffers *g_buf;
	int ret = 0;
	
	tv_intf_print("-IN buf=%p count=%d\n", buf, count);

	/* check the length */
	if( count > (TV_INTF_CMD_SIZE - 1) ) {
		printk(KERN_ERR"[%s] max cmd size=%d\n", __func__, TV_INTF_CMD_SIZE);
		ret = -ENOMEM;
		goto err_len;
	}

	/* get the buffer */
	g_buf = (struct tv_intf_buffers *) filp->private_data;

	ret = down_interruptible(&g_buf->buf_sem);
	if( ret < 0 ) {
		printk(KERN_ERR"[%s] error in getting buffer semaphore\n", __func__ );
		goto err_lock;
	}

	/* copy from user */
	if( copy_from_user(g_buf->cmd, buf, count) ) {
		ret = -EFAULT;
		printk(KERN_ERR"[%s] copy from user failed user_buf=%p\n",
				__func__, buf);
		goto done;
	}
	g_buf->cmd[count] = '\0';
	tv_intf_print("cmd=%s\n", g_buf->cmd);

	/* call bcm2835_tv_intf */
	g_buf->resp_len = bcm2835_tv_intf( g_buf->resp, TV_INTF_RESP_SIZE, g_buf->cmd);
	ret = count;

done:
	up(&g_buf->buf_sem);
err_lock:
err_len:	
	tv_intf_print("-OUT ret=%d\n", ret);
	return ret;
}

int tv_intf_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
	 unsigned long uncopied;
	 u8 *ioctl_cmd_buf;

    tv_intf_print("cmd = %x, size=%d\n", cmd, _IOC_SIZE(cmd));

	 BUG_ON(TV_INTF_MAX_IOCTL_CMD_SIZE < _IOC_SIZE(cmd));

	 ioctl_cmd_buf = (u8 *)kmalloc(TV_INTF_MAX_IOCTL_CMD_SIZE, GFP_KERNEL);
	 if (!ioctl_cmd_buf)
        return -ENOMEM;	

    tv_intf_print("Got buffer.\n");
    if (0 != _IOC_SIZE(cmd)) {
        uncopied = copy_from_user(ioctl_cmd_buf, (void *)arg, _IOC_SIZE(cmd));
        if (uncopied != 0) {
            kfree(ioctl_cmd_buf);
            return -EFAULT;
        }
        tv_intf_print("Copied from user space\n");

        switch (cmd) {
            case TV_INTF_IOCTL_GET_CTRLS:
                ret = bcm2835_tv_ioctl_get((TV_INTF_IOCTL_CTRLS_T *)ioctl_cmd_buf);
                uncopied = copy_to_user((void *)arg, ioctl_cmd_buf, _IOC_SIZE(cmd));
                if (uncopied != 0)
                   ret = -EFAULT;
                tv_intf_print("Copied to user space\n");
                break;

            case TV_INTF_IOCTL_SET_CTRLS:
                ret = bcm2835_tv_ioctl_set((TV_INTF_IOCTL_CTRLS_T *)ioctl_cmd_buf);
                break;

            case TV_INTF_IOCTL_GET_EDID:
                ret = bcm2835_tv_ioctl_edid_get((TV_INTF_IOCTL_EDID_T *)ioctl_cmd_buf);
                uncopied = copy_to_user((void *)arg, ioctl_cmd_buf, _IOC_SIZE(cmd));
                if (uncopied != 0)
                    ret = -EFAULT;
                tv_intf_print("Copied EDID to user space\n");
                break;

            default: 
                tv_intf_print("Wrong IOCTL cmd. Expect %x, %x or %x\n",
                              TV_INTF_IOCTL_GET_CTRLS,
                              TV_INTF_IOCTL_SET_CTRLS,
                              TV_INTF_IOCTL_GET_EDID);
                ret = -EFAULT;
                break;
        }
    }

    kfree(ioctl_cmd_buf);
    return ret;
}

int tv_intf_open (struct inode *inode, struct file *filp)
{
	struct tv_intf_buffers *g_buf;
	int ret = 0;

	tv_intf_print("-IN\n");
	g_buf = kmalloc(sizeof(struct tv_intf_buffers), GFP_KERNEL);
	if( !g_buf ) {
		printk(KERN_ERR"[%s]:failed to allocate tv_intf buffers\n", __func__);
		ret = -ENOMEM;
	}
	else {
		filp->private_data = g_buf;
		g_buf->resp_len = 0;
		sema_init(&g_buf->buf_sem, 1);
	}
	tv_intf_print("-OUT ret=%d\n", ret);
	return ret;
}

int tv_intf_release(struct inode *inode, struct file *filp)
{
	int ret;
	struct tv_intf_buffers *g_buf;

	tv_intf_print("-IN");
	g_buf = (struct tv_intf_buffers *) filp->private_data;
	ret = down_interruptible(&g_buf->buf_sem );
	if( ret < 0 ) {
		printk(KERN_ERR"[%s]:failed to get buffer semaphore\n", __func__);
		goto done;
	}
	kfree(g_buf);
	ret = 0;
done:
	tv_intf_print("-OUT ret=%d\n", ret);
	return ret;
}

struct file_operations tv_intf_fops = {
	.owner =	THIS_MODULE,
	.read =		tv_intf_read,
	.write =	tv_intf_write,
	.ioctl =	tv_intf_ioctl,
	.open =		tv_intf_open,
	.release =	tv_intf_release,
};

struct miscdevice tv_intf_dev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     TV_INTF_DRIVER_NAME,
    .fops =     &tv_intf_fops
};


int __devinit tv_intf_driver_init(void)
{
	int ret;

    ret = misc_register(&tv_intf_dev);
    if (ret < 0) {
        printk(KERN_ERR "%s:failed to register device\n",
                TV_INTF_DRIVER_NAME);
    }

	return ret;
}

void __devexit tv_intf_driver_exit(void)
{
    misc_deregister(&tv_intf_dev);
}
