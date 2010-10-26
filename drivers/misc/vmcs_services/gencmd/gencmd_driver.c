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
#include <linux/miscdevice.h>

#include "gencmd.h"
#include "gencmd_driver.h"

#define GENCMD_DRIVER_NAME  "gencmd"

struct gencmd_buffers {
	char cmd[GENCMD_CMD_SIZE];
	char resp[GENCMD_RESP_SIZE];
	int  resp_len;
	struct semaphore buf_sem;
};


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

    switch( cmd ) {
        case GENCMD_IOCTL_CMD_SEND_RESP:
            {
                GENCMD_IOCTL_SEND_RESP_T  request; 
                int count, resp_len, len;
                struct gencmd_buffers *g_buf;

                if( copy_from_user(&request,(void *) arg, sizeof(GENCMD_IOCTL_SEND_RESP_T)) ) {
                    ret = -EFAULT;
                    printk(KERN_ERR"[%s]error in copying cmd structure\n", __func__);
                    goto err_cmd;
                }

                count = request.cmd_len;
                resp_len = request.resp_len;

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
                if( copy_from_user(g_buf->cmd, request.cmd, count) ) {
                    ret = -EFAULT;
                    printk(KERN_ERR"[%s] copy from user failed user_buf=%p\n",
                            __func__, request.cmd);
                    goto done;
                }
                g_buf->cmd[count] = '\0';
                gencmd_print("cmd=%s\n", g_buf->cmd);

                /* call vc_gencmd */
                g_buf->resp_len = vc_gencmd( g_buf->resp, GENCMD_RESP_SIZE, g_buf->cmd);

                /* check the length */
                len = g_buf->resp_len;

                if( len > resp_len )
                    len = resp_len;

                if( !request.resp ) {
                    printk(KERN_ERR"[%s] no user buffer passed\n", __func__);
                    ret = 0;
                    goto done;
                }

                ret = copy_to_user(request.resp, g_buf->resp, len);
                if( ret < 0 ) {
                    printk(KERN_ERR"[%s] error in copying date to user buffer\n", __func__);
                    goto done;
                }

                gencmd_print("gencmd resp=%s\n", g_buf->resp);
                ret = len;
done:
                up(&g_buf->buf_sem);
                break;
err_lock:
err_len:
err_cmd:
                break;
            }

        default:
            printk(KERN_ERR"[%s]:unknown command cmd=%d\n", __func__, cmd);
            ret = -EIO;
            break;
    }

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

struct miscdevice gencmd_dev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     GENCMD_DRIVER_NAME,
    .fops =     &gencmd_fops
};


int __devinit gencmd_driver_init(void)
{
	int ret;

    ret = misc_register(&gencmd_dev);
    if (ret < 0) {
        printk(KERN_ERR "%s:failed to register device\n",
                GENCMD_DRIVER_NAME);
    }

	return ret;
}

void __devexit gencmd_driver_exit(void)
{
    misc_deregister(&gencmd_dev);
}
