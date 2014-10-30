/*
 * Copyright (c) 2011 Synaptics Incorporated
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/clearpad.h>

#define CHAR_DEVICE_NAME "rmi"
#define DEVICE_CLASS_NAME "rmidev"

#define RMI_CHAR_DEV_TMPBUF_SZ (1024 * 8)
#define RMI_REG_ADDR_PAGE_SELECT 0xFF
#define REG_ADDR_LIMIT 0xFFFF

struct rmidev_data {
	/* mutex for file operation*/
	struct mutex file_mutex;
	/* main char dev structure */
	struct cdev main_dev;

	/* pointer to the corresponding RMI4 device.  We use this to do */
	/* read, write, etc. */

	struct platform_device *pdev;
	struct clearpad_platform_data_t *pdata;
	struct clearpad_bus_data_t *bdata;
	/* reference count */
	int ref_count;

	struct class *device_class;

	unsigned char tmpbuf[RMI_CHAR_DEV_TMPBUF_SZ];
};

/*store dynamically allocated major number of char device*/
static int rmidev_major_num;


static struct class *rmidev_device_class;

static int rmi_read_block(struct rmidev_data *data, u16 addr, u8 *buf, int len)
{
	return data->bdata->read_block(data->bdata->dev, addr, buf, len);
}

static int rmi_write_block(struct rmidev_data *data, u16 addr, const u8 *buf,
		int len)
{
	return data->bdata->write_block(data->bdata->dev, addr, buf, len);
}

/* file operations for RMI char device */

/*
 * rmidev_llseek: - use to setup register address
 *
 * @filp: file structure for seek
 * @off: offset
 *       if whence == SEEK_SET,
 *       high 16 bits: page address
 *       low 16 bits: register address
 *
 *       if whence == SEEK_CUR,
 *       offset from current position
 *
 *       if whence == SEEK_END,
 *       offset from END(0xFFFF)
 *
 * @whence: SEEK_SET , SEEK_CUR or SEEK_END
 */
static loff_t rmidev_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t newpos;
	struct rmidev_data *data = filp->private_data;

	if (IS_ERR(data)) {
		pr_err("%s: pointer of char device is invalid", __func__);
		newpos = -EBADF;
		goto exit;
	}

	mutex_lock(&(data->file_mutex));

	switch (whence) {
	case SEEK_SET:
		newpos = off;
		break;

	case SEEK_CUR:
		newpos = filp->f_pos + off;
		break;

	case SEEK_END:
		newpos = REG_ADDR_LIMIT + off;
		break;

	default:		/* can't happen */
		newpos = -EINVAL;
		goto clean_up;
	}

	if (newpos < 0 || newpos > REG_ADDR_LIMIT) {
		dev_err(&data->pdev->dev, "newpos 0x%04x is invalid.\n",
			(unsigned int)newpos);
		newpos = -EINVAL;
		goto clean_up;
	}

	filp->f_pos = newpos;

clean_up:
	mutex_unlock(&(data->file_mutex));
exit:
	return newpos;
}

/*
 *  rmidev_read: - use to read data from RMI stream
 *
 *  @filp: file structure for read
 *  @buf: user-level buffer pointer
 *
 *  @count: number of byte read
 *  @f_pos: offset (starting register address)
 *
 *	@return number of bytes read into user buffer (buf) if succeeds
 *          negative number if error occurs.
 */
static ssize_t rmidev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct rmidev_data *data = filp->private_data;
	ssize_t retval  = 0;

	if (*f_pos > REG_ADDR_LIMIT) {
		retval = -EINVAL;
		goto exit;
	}

	/* limit offset to REG_ADDR_LIMIT-1 */
	if (count > (REG_ADDR_LIMIT - *f_pos))
		count = REG_ADDR_LIMIT - *f_pos;
	if (count > RMI_CHAR_DEV_TMPBUF_SZ)
		count = RMI_CHAR_DEV_TMPBUF_SZ;

	if (count == 0)
		goto exit;

	if (IS_ERR(data)) {
		pr_err("%s: pointer of char device is invalid", __func__);
		retval = -EBADF;
		goto exit;
	}

	mutex_lock(&(data->file_mutex));

	retval = rmi_read_block(data, *f_pos, data->tmpbuf, count);

	if (retval < 0)
		goto clean_up;
	else
		*f_pos += retval;

	if (copy_to_user(buf, data->tmpbuf, count))
		retval = -EFAULT;

clean_up:

	mutex_unlock(&(data->file_mutex));
exit:
	return retval;
}

/*
 * rmidev_write: - use to write data into RMI stream
 *
 * @filep : file structure for write
 * @buf: user-level buffer pointer contains data to be written
 * @count: number of byte be be written
 * @f_pos: offset (starting register address)
 *
 * @return number of bytes written from user buffer (buf) if succeeds
 *         negative number if error occurs.
 */
static ssize_t rmidev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct rmidev_data *data = filp->private_data;
	ssize_t retval  = 0;

	if (*f_pos > REG_ADDR_LIMIT) {
		retval = -EINVAL;
		goto exit;
	}

	/* limit offset to REG_ADDR_LIMIT-1 */
	if (count > (REG_ADDR_LIMIT - *f_pos))
		count = REG_ADDR_LIMIT - *f_pos;
	if (count > RMI_CHAR_DEV_TMPBUF_SZ)
		count = RMI_CHAR_DEV_TMPBUF_SZ;

	if (count == 0)
		goto exit;

	if (IS_ERR(data)) {
		pr_err("%s: pointer of char device is invalid", __func__);
		retval = -EBADF;
		goto exit;
	}

	if (copy_from_user(data->tmpbuf, buf, count)) {
		retval = -EFAULT;
		goto exit;
	}

	mutex_lock(&(data->file_mutex));

	retval = rmi_write_block(data, *f_pos, data->tmpbuf, (int)count);

	if (retval >= 0)
		*f_pos += count;

	mutex_unlock(&(data->file_mutex));
exit:
	return retval;
}

/*
 * rmidev_open: - get a new handle for from RMI stream
 * @inp : inode struture
 * @filp: file structure for read/write
 *
 * @return 0 if succeeds
 */
static int rmidev_open(struct inode *inp, struct file *filp)
{
	struct rmidev_data *data = container_of(inp->i_cdev,
			struct rmidev_data, main_dev);
	int retval = 0;

	filp->private_data = data;

	if (!data->pdata) {
		retval = -EACCES;
		goto exit;
	}

	mutex_lock(&(data->file_mutex));
	if (data->ref_count < 1)
		data->ref_count++;
	else
		retval = -EACCES;

	mutex_unlock(&(data->file_mutex));
exit:
	return retval;
}

/*
 *  rmidev_release: - release an existing handle
 *  @inp: inode structure
 *  @filp: file structure for read/write
 *
 *  @return 0 if succeeds
 */
static int rmidev_release(struct inode *inp, struct file *filp)
{
	struct rmidev_data *data = container_of(inp->i_cdev,
			struct rmidev_data, main_dev);
	int retval = 0;

	if (!data->pdev) {
		retval = -EACCES;
		goto exit;
	}

	mutex_lock(&(data->file_mutex));

	data->ref_count--;
	if (data->ref_count < 0)
		data->ref_count = 0;

	mutex_unlock(&(data->file_mutex));
exit:
	return retval;
}

static const struct file_operations rmidev_fops = {
	.owner =    THIS_MODULE,
	.llseek =   rmidev_llseek,
	.read =     rmidev_read,
	.write =    rmidev_write,
	.open =     rmidev_open,
	.release =  rmidev_release,
};

/*
 * rmi_char_dev_clean_up - release memory or unregister driver
 * @rmi_char_dev: rmi_char_dev structure
 *
 */
static void rmidev_device_cleanup(struct rmidev_data *data)
{
	dev_t devno;

	/* Get rid of our char dev entries */
	if (data) {
		devno = data->main_dev.dev;

		if (data->device_class)
			device_destroy(data->device_class, devno);

		cdev_del(&data->main_dev);

		/* cleanup_module is never called if registering failed */
		unregister_chrdev_region(devno, 1);
		pr_debug("%s: rmidev device is removed\n", __func__);
	}
}

/*
 * rmi_char_devnode - return device permission
 *
 * @dev: char device structure
 * @mode: file permission
 *
 */
static char *rmi_char_devnode(struct device *dev, mode_t *mode)
{
	char *pret = NULL;

	if (mode) {
		/**mode = 0600*/
		*mode = S_IRUSR|S_IWUSR;
		pret = kasprintf(GFP_KERNEL, "rmi/%s", dev_name(dev));
	}
	return pret;
}

static int rmidev_init_device(struct rmidev_data *data)
{
	dev_t dev_no;
	int retval;
	struct device *device_ptr;

	if (rmidev_major_num) {
		dev_no = MKDEV(rmidev_major_num, 0);
		retval = register_chrdev_region(dev_no, 1, CHAR_DEVICE_NAME);
	} else {
		retval = alloc_chrdev_region(&dev_no, 0, 1, CHAR_DEVICE_NAME);
		/* let kernel allocate a major for us */
		rmidev_major_num = MAJOR(dev_no);
		dev_info(&data->pdev->dev, "Major number of rmidev: %d\n",
				 rmidev_major_num);
	}
	if (retval < 0) {
		dev_err(&data->pdev->dev,
			"Failed to register or allocate char dev, code %d.\n",
				retval);
		goto exit;
	} else {
		dev_info(&data->pdev->dev, "Allocated rmidev %d %d.\n",
			 MAJOR(dev_no), MINOR(dev_no));
	}

	mutex_init(&data->file_mutex);

	cdev_init(&data->main_dev, &rmidev_fops);

	retval = cdev_add(&data->main_dev, dev_no, 1);
	if (retval) {
		dev_err(&data->pdev->dev, "Error %d adding rmi_char_dev.\n",
			retval);
		rmidev_device_cleanup(data);
		goto exit;
	}

	dev_set_name(&data->pdev->dev, "rmidev%d", MINOR(dev_no));
	data->device_class = rmidev_device_class;
	device_ptr = device_create(
			data->device_class,
			NULL, dev_no, NULL,
			CHAR_DEVICE_NAME"%d",
			MINOR(dev_no));

	if (IS_ERR(device_ptr)) {
		dev_err(&data->pdev->dev,
			"Failed to create rmi device.\n");
		rmidev_device_cleanup(data);
		retval = -ENODEV;
	}
exit:
	return retval;
}

static int __devinit rmi_dev_probe(struct platform_device *pdev)
{

	struct clearpad_data_t *cdata = pdev->dev.platform_data;
	struct rmidev_data *data;
	int retval = 0;

	data = kzalloc(sizeof(struct rmidev_data), GFP_KERNEL);
	if (!data) {
		retval = -ENOMEM;
		goto exit;
	}

	dev_set_drvdata(&pdev->dev, data);
	data->pdev = pdev;
	data->pdata = cdata->pdata;
	if (!data->pdata) {
		dev_err(&data->pdev->dev, "no platform data\n");
		retval = -EINVAL;
		goto err_free;
	}
	data->bdata = cdata->bdata;
	if (!data->bdata) {
		dev_err(&data->pdev->dev, "no bus data\n");
		retval = -EINVAL;
		goto err_free;
	}

	retval = rmidev_init_device(data);
	if (!retval)
		goto exit;

err_free:
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(data);
exit:
	return retval;
}

static int __devexit rmi_dev_remove(struct platform_device *pdev)
{
	struct rmidev_data *data = dev_get_drvdata(&pdev->dev);
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(data);
	return 0;
}

static struct platform_driver rmidev_driver = {
	.driver = {
		.name = CLEARPAD_RMI_DEV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= rmi_dev_probe,
	.remove		= __devexit_p(rmi_dev_remove),
};

static int __init rmidev_init(void)
{
	int retval = 0;

	pr_debug("%s: rmi_dev initialization.\n", __func__);

	/* create device node */
	rmidev_device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);

	if (IS_ERR(rmidev_device_class)) {
		pr_err("%s: ERROR - Failed to create /dev/%s.\n", __func__,
			CHAR_DEVICE_NAME);
		retval = -ENODEV;
		goto exit;
	}
	/* setup permission */
	rmidev_device_class->devnode = rmi_char_devnode;

	retval = platform_driver_register(&rmidev_driver);
exit:
	return retval;
}

static void __exit rmidev_exit(void)
{
	pr_debug("%s: exiting.\n", __func__);
	platform_driver_unregister(&rmidev_driver);
	class_unregister(rmidev_device_class);
	class_destroy(rmidev_device_class);
}

module_init(rmidev_init);
module_exit(rmidev_exit);

MODULE_AUTHOR("Christopher Heiny <cheiny@synaptics.com>");
MODULE_DESCRIPTION("RMI4 Char Device");
MODULE_LICENSE("GPL");
