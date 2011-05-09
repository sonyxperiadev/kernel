/*
 *  camera_driver.c - camera driver interface
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

#include "camera.h"
#include "camera_regs.h"
#include "camera_driver.h"

#define CAMERA_DRIVER_NAME  "vc_camera"

struct camera_buffers {
	char cmd[CAMERA_CMD_SIZE];
	char resp[CAMERA_RESP_SIZE];
	int  resp_len;
	struct semaphore buf_sem;
};


/*driver functions  */
ssize_t camera_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{

	struct camera_buffers *g_buf;
	int ret = 0;
	int len;

	camera_print("-IN buf=%p count=%d\n", buf, count);
	/* get the buffer */
	g_buf = (struct camera_buffers *) filp->private_data;

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

	camera_print("camera resp=%s\n", g_buf->resp);
	ret = len;

done:
	up(&g_buf->buf_sem);

err_lock:
	camera_print("-OUT ret=%d\n", ret);
	return ret;
}


ssize_t camera_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct camera_buffers *g_buf;
	int ret = 0;

	camera_print("-IN buf=%p count=%d\n", buf, count);

	/* check the length */
	if( count > (CAMERA_CMD_SIZE - 1) ) {
		printk(KERN_ERR"[%s] max cmd size=%d\n", __func__, CAMERA_CMD_SIZE);
		ret = -ENOMEM;
		goto err_len;
	}

	/* get the buffer */
	g_buf = (struct camera_buffers *) filp->private_data;

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
	camera_print("cmd=%s\n", g_buf->cmd);

	/* call vc_camera */
	g_buf->resp_len = vc_camera( g_buf->resp, CAMERA_RESP_SIZE, g_buf->cmd);
	ret = count;

done:
	up(&g_buf->buf_sem);
err_lock:
err_len:
	camera_print("-OUT ret=%d\n", ret);
	return ret;
}

int camera_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = -1;
	camera_print("-IN cmd=%d arg=0x%lx\n", cmd, arg);

	switch( cmd ) {
		case CAMERA_IOCTL_VIEWFINDER_SETUP:
			{
				CAMERA_IOCTL_SETUP_CAMERA_T  request;

				if( copy_from_user(&request,(void *) arg, sizeof(CAMERA_IOCTL_SETUP_CAMERA_T)) ) {
					ret = -EFAULT;
					printk(KERN_ERR"[%s]error in copying cmd structure\n", __func__);
					goto err_cmd;
				}

				vc_camera_control( VIEWFINDER_ENABLE == request.state );
				break;
			}

		case CAMERA_IOCTL_TAKE_PICTURE:
            {
				CAMERA_CAPTURE_S  request;

				if( copy_from_user(&request,(void *) arg, sizeof(CAMERA_CAPTURE_S)) ) {
					ret = -EFAULT;
					printk(KERN_ERR"[%s]error in copying cmd structure\n", __func__);
					goto err_cmd;
				}
                ret = vc_camera_take_picture(request.width, request.height, request.max_size, request.buffer);
				break;
            }

		default:
			printk(KERN_ERR"[%s]:unknown command cmd=%d\n", __func__, cmd);
			ret = -EIO;
			break;
	}

err_cmd:
	camera_print("-OUT ret=%d\n", ret);
	return ret;
}

int camera_open (struct inode *inode, struct file *filp)
{
	struct camera_buffers *g_buf;
	int ret = 0;

	camera_print("-IN\n");
	g_buf = kmalloc(sizeof(struct camera_buffers), GFP_KERNEL);
	if( !g_buf ) {
		printk(KERN_ERR"[%s]:failed to allocate camera buffers\n", __func__);
		ret = -ENOMEM;
	}
	else {
		filp->private_data = g_buf;
		g_buf->resp_len = 0;
		sema_init(&g_buf->buf_sem, 1);
	}
	camera_print("-OUT ret=%d\n", ret);
	return ret;
}

int camera_release(struct inode *inode, struct file *filp)
{
	int ret;
	struct camera_buffers *g_buf;

	camera_print("-IN");
	vc_camera_control(VIREFINDER_DISABLE);
	g_buf = (struct camera_buffers *) filp->private_data;
	ret = down_interruptible(&g_buf->buf_sem );
	if( ret < 0 ) {
		printk(KERN_ERR"[%s]:failed to get buffer semaphore\n", __func__);
		goto done;
	}
	kfree(g_buf);
	ret = 0;
done:
	camera_print("-OUT ret=%d\n", ret);
	return ret;
}

struct file_operations camera_fops = {
	.owner =	THIS_MODULE,
	.read =		camera_read,
	.write =	camera_write,
	.ioctl =	camera_ioctl,
	.open =		camera_open,
	.release =	camera_release,
};

struct miscdevice camera_dev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     CAMERA_DRIVER_NAME,
	.fops =     &camera_fops
};


int __devinit camera_driver_init(void)
{
	int ret;

	ret = misc_register(&camera_dev);
	if (ret < 0) {
		printk(KERN_ERR "%s:failed to register device\n",
				CAMERA_DRIVER_NAME);
	}

	return ret;
}

void __devexit camera_driver_exit(void)
{
	misc_deregister(&camera_dev);
}
