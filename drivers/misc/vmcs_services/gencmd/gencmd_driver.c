/*
 *  gencmd_driver.c - gencmd driver interface
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

#include "gencmd.h"
#include "gencmd_driver.h"


#define GENCMD_CMD_SIZE		512
#define GENCMD_RESP_SIZE	512

struct gencmd_dev {
	dev_t number;
	struct cdev cdev;
};

struct gencmd_buffers {
	char cmd[GENCMD_CMD_SIZE];
	char resp[GENCMD_RESP_SIZE];
	int  resp_len;
	struct semaphore buf_sem;
};

static struct gencmd_dev *gencmd_device;

/*driver functions  */
ssize_t gencmd_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{

	struct gencmd_buffers *g_buf;
	int ret = 0; 
	int len;

	gencmd_print("-IN buf=%p count=%d\n", buf, count);
	/* get the buffer */
	g_buf = (struct gencmd_buffers *) filp->private_data;

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
	
	gencmd_print("gencmd resp=%s\n", g_buf->resp);
	ret = len;
	
done:
	up(&g_buf->buf_sem);

err_lock:	
	gencmd_print("-OUT ret=%d\n", ret);
	return ret;
}


ssize_t gencmd_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct gencmd_buffers *g_buf;
	int ret = 0;
	
	gencmd_print("-IN buf=%p count=%d\n", buf, count);

	/* check the length */
	if( count > (GENCMD_CMD_SIZE - 1) ) {
		printk(KERN_ERR"[%s] max cmd size=%d\n", __func__, GENCMD_CMD_SIZE);
		ret = -ENOMEM;
		goto err_len;
	}

	/* get the buffer */
	g_buf = (struct gencmd_buffers *) filp->private_data;

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
	gencmd_print("cmd=%s\n", g_buf->cmd);

	/* call vc_gencmd */
	g_buf->resp_len = vc_gencmd( g_buf->resp, GENCMD_RESP_SIZE, g_buf->cmd);
	ret = count;

done:
	up(&g_buf->buf_sem);
err_lock:
err_len:	
	gencmd_print("-OUT ret=%d\n", ret);
	return ret;
}

int gencmd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = -1;
	gencmd_print("-IN cmd=%d arg=0x%lx\n", cmd, arg);

	gencmd_print("-OUT ret=%d\n", ret);
	return ret;
}

int gencmd_open (struct inode *inode, struct file *filp)
{
	struct gencmd_buffers *g_buf;
	int ret = 0;

	gencmd_print("-IN\n");
	g_buf = kmalloc(sizeof(struct gencmd_buffers), GFP_KERNEL);
	if( !g_buf ) {
		printk(KERN_ERR"[%s]:failed to allocate gencmd buffers\n", __func__);
		ret = -ENOMEM;
	}
	else {
		filp->private_data = g_buf;
		g_buf->resp_len = 0;
		sema_init(&g_buf->buf_sem, 1);
	}
	gencmd_print("-OUT ret=%d\n", ret);
	return ret;
}

int gencmd_release(struct inode *inode, struct file *filp)
{
	int ret;
	struct gencmd_buffers *g_buf;

	gencmd_print("-IN");
	g_buf = (struct gencmd_buffers *) filp->private_data;
	ret = down_interruptible(&g_buf->buf_sem );
	if( ret < 0 ) {
		printk(KERN_ERR"[%s]:failed to get buffer semaphore\n", __func__);
		goto done;
	}
	kfree(g_buf);
	ret = 0;
done:
	gencmd_print("-OUT ret=%d\n", ret);
	return ret;
}

struct file_operations gencmd_fops = {
	.owner =	THIS_MODULE,
	.read =		gencmd_read,
	.write =	gencmd_write,
	.ioctl =	gencmd_ioctl,
	.open =		gencmd_open,
	.release =	gencmd_release,
};

int __devinit gencmd_driver_init(void)
{
	int ret;

	gencmd_device = NULL;

	/* allocate internal structure */
	gencmd_device = kmalloc(sizeof(struct gencmd_dev), GFP_KERNEL);
	if( !gencmd_device ) {
		ret = -ENOMEM;
		printk(KERN_ERR"[%s] failed to allocate mem for gencmd\n", __func__);
		goto err_gencmd_device;
	}

	/* allocate major and minor number */
	gencmd_device->number = 0;
	ret = alloc_chrdev_region(&gencmd_device->number, 0, 1, "VC Gencmd device");
	if( ret < 0 ) {
		printk(KERN_ERR"[%s]failed to allocate major/minor num for gencmd\n",
				__func__);
		goto err_number;
	}
	
	gencmd_print("Gencmd major=%d minor=%d\n", MAJOR(gencmd_device->number),
			MINOR(gencmd_device->number));

	/*init cdev */
	cdev_init(&gencmd_device->cdev, &gencmd_fops);

	/* register character driver */
	ret = cdev_add(&gencmd_device->cdev, gencmd_device->number, 1);
	if( ret < 0 ) {
		printk(KERN_ERR"[%s]failed to register gencmd character driver\n",
				__func__);
		goto err_cdev;
	}


	return 0;
err_cdev:
	/* free up major/minor number */
	unregister_chrdev_region(gencmd_device->number, 1);

err_number:
	/* free gencmd structure */
	kfree(gencmd_device);
	gencmd_device = NULL;
err_gencmd_device:
	return ret;
}

void __devexit gencmd_driver_exit(void)
{
	if(gencmd_device) {
		/* remove char dev */
		cdev_del(&gencmd_device->cdev);
		
		/* free up major/minor number */
		unregister_chrdev_region(gencmd_device->number, 1);
		
		/* free gencmd structure */
		kfree(gencmd_device);
		gencmd_device = NULL;
	}
}
