/* kernel/drivers/misc/felica.c
 *
 * Copyright (C) 2010-2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroaki Kuriyama <Hiroaki.Kuriyama@sonyericsson.com>
 * Author: Satomi Watanabe <Satomi.Watanabe@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <crypto/hash.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/felica.h>

#define DRV_VERSION "0.2"
#define AUTHENTICATION_LEN 1024
#define AUTH_HASH_LEN 32
#define HASH_ALG "sha256"
#define UDELAY_CEN_WRITE 100

static DEFINE_SPINLOCK(spinlock);

static ssize_t show_usbcon(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned int value;
	unsigned long flags;

	spin_lock_irqsave(&spinlock, flags);
	value = d->st_usbcon;
	spin_unlock_irqrestore(&spinlock, flags);

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t set_usbcon(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned long value;
	unsigned long flags;

	if (strict_strtoul(buf, 10, &value)) {
		dev_err(d->dev, "%s: Failed to parse integer\n",
				__func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&spinlock, flags);
	d->st_usbcon = value;
	dev_dbg(d->dev, "%u\n", d->st_usbcon);
	spin_unlock_irqrestore(&spinlock, flags);

	return count;
}

static ssize_t show_airplane(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned int value;
	unsigned long flags;

	spin_lock_irqsave(&spinlock, flags);
	value = d->st_airplane;
	spin_unlock_irqrestore(&spinlock, flags);

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t set_airplane(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned long value;
	unsigned long flags;

	if (strict_strtoul(buf, 10, &value)) {
		dev_err(d->dev, "%s: Failed to parse integer\n",
				__func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&spinlock, flags);
	d->st_airplane = value;
	dev_dbg(d->dev, "%u\n", d->st_airplane);
	spin_unlock_irqrestore(&spinlock, flags);

	return count;
}

static ssize_t show_rwusb(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned int value;
	unsigned long flags;

	spin_lock_irqsave(&spinlock, flags);
	value = d->ta_rwusb;
	spin_unlock_irqrestore(&spinlock, flags);

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t set_rwusb(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct felica_dev *d = dev_get_drvdata(dev);
	unsigned long value;
	unsigned long flags;

	if (strict_strtoul(buf, 10, &value)) {
		dev_err(d->dev, "%s: Failed to parse integer\n",
				__func__);
		return -EINVAL;
	}
	spin_lock_irqsave(&spinlock, flags);
	d->ta_rwusb = value;
	dev_dbg(d->dev, "%u\n", d->ta_rwusb);
	spin_unlock_irqrestore(&spinlock, flags);

	return count;
}

static struct device_attribute attrs[] = {
	__ATTR(st_usbcon, S_IRUGO | S_IWUSR, show_usbcon, set_usbcon),
	__ATTR(st_airplane, S_IRUGO | S_IWUSR, show_airplane, set_airplane),
	__ATTR(ta_rwusb, S_IRUGO | S_IWUSR, show_rwusb, set_rwusb),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attrs); i++)
		if (device_create_file(dev, attrs + i))
			goto error;
	return 0;
error:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, attrs + i);

	dev_err(dev, "Unable to create sysfs interfaces\n");

	return -EIO;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attrs); i++)
		device_remove_file(dev, attrs + i);
}

struct sdesc {
	struct shash_desc shash;
	char ctx[];
};

/** @ Authentication hash value*/
static const u8 auth_hash[AUTH_HASH_LEN] = {
	0xbf, 0x19, 0x2e, 0xaf, 0x67, 0x0c, 0x90, 0xc8,
	0x12, 0x5f, 0xbb, 0xbc, 0x9e, 0x17, 0x23, 0x39,
	0x69, 0x4a, 0xb7, 0xec, 0x3a, 0x9f, 0x91, 0x57,
	0x80, 0xec, 0xb3, 0xf5, 0xfe, 0x28, 0x8f, 0x33
};

static int calc_hash(const u8 *src, int src_len, u8 *out, struct device *dev)
{
	struct crypto_shash *shash;
	struct sdesc *desc;
	int size;
	int ret = -EFAULT;

	shash = crypto_alloc_shash(HASH_ALG, 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(shash)) {
		dev_err(dev, "%s: Error. crypto_alloc_shash.\n", __func__);
		goto err_shash;
	}

	size = sizeof(struct shash_desc) + crypto_shash_descsize(shash);
	desc = kmalloc(size, GFP_KERNEL);
	if (!desc) {
		dev_err(dev, "%s: Error. No enough mem for Desc.\n", __func__);
		ret = -ENOMEM;
		goto err_desc;
	}

	desc->shash.tfm = shash;
	desc->shash.flags = 0x00;

	if (crypto_shash_digest(&desc->shash, src, src_len, out)) {
		dev_err(dev, "%s: Error. generate hash.\n", __func__);
		goto err_generate;
	}

	ret = 0;

err_generate:
	kfree(desc);
err_desc:
	crypto_free_shash(shash);
err_shash:
	return ret;
}

static int felica_cen_open(struct inode *inode, struct file *file)
{
	int ret;
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_cen);

	dev_dbg(d->device_cen.this_device, "%s\n", __func__);

	/* Open CEN */
	ret = d->flcen->cen_init(d);
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Cannot enable CEN.\n", __func__);
		return ret;
	}
	dev_dbg(d->device_cen.this_device, "Successfully enabled CEN\n");

	return 0;
}

static int felica_cen_release(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_cen);

	dev_dbg(d->device_cen.this_device,
			"%s\n", __func__);

	return 0;
}

static ssize_t felica_cen_read(struct file *file, char __user *buf,
					size_t count, loff_t *offset)
{
	int ret;
	char kbuf;
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_cen);

	dev_dbg(d->device_cen.this_device, "%s\n", __func__);

	/* Read CEN value */
	ret = d->flcen->cen_read(&kbuf, d);
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Error. CEN access failed.\n", __func__);
		return -EIO;
	}

	/* Copy CEN value to user space */
	ret = copy_to_user(buf, &kbuf, sizeof(kbuf));
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Error. copy_to_user failure.\n", __func__);
		return -EFAULT;
	}

	/* 1 byte read */
	return sizeof(kbuf);
}

static ssize_t felica_cen_write(struct file *file, const char __user *buf,
					size_t count, loff_t *offset)
{
	int ret;
	char kbuf;
	u8 *src;
	u8 hash[AUTH_HASH_LEN];
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_cen);

	dev_dbg(d->device_cen.this_device, "%s\n", __func__);

	if ((AUTHENTICATION_LEN+1) != count || buf == NULL) {
		dev_err(d->device_cen.this_device,
				"%s: Error. Invalid arg @CEN write.\n",
					__func__);
		ret = -EINVAL;
		goto exit;
	}

	/* Carry out user authentication */
	src = kmalloc(AUTHENTICATION_LEN, GFP_KERNEL);
	if (!src) {
		dev_err(d->device_cen.this_device,
				"%s: Error. No enough mem for Auth.\n",
					__func__);
		ret = -ENOMEM;
		goto exit;
	}
	ret = copy_from_user(src, buf, AUTHENTICATION_LEN);
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Error. copy_from_user failure.\n",
					__func__);
		ret = -EFAULT;
		goto end_process;
	}

	if (calc_hash(src, AUTHENTICATION_LEN, hash,
					d->device_cen.this_device)) {
		dev_err(d->device_cen.this_device,
				"%s: Error. calc hash digest failure.\n",
					__func__);
		ret = -EACCES;
		goto end_process;
	}

	if (memcmp(auth_hash, hash, AUTH_HASH_LEN)) {
		dev_err(d->device_cen.this_device,
				"%s: Error. invalid authentication.\n",
					__func__);
		ret = -EACCES;
		goto end_process;
	}

	/* Copy value from user space */
	ret = copy_from_user(&kbuf, &buf[AUTHENTICATION_LEN], sizeof(kbuf));
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Error. copy_from_user failure.\n",
					__func__);
		ret = -EFAULT;
		goto end_process;
	}

	ret = d->flcen->cen_write(kbuf, d);
	if (ret) {
		dev_err(d->device_cen.this_device,
				"%s: Error. Cannot write CEN.\n", __func__);
		goto end_process;
	}

	/* usec delay*/
	udelay(UDELAY_CEN_WRITE);

	ret = AUTHENTICATION_LEN+1;

end_process:
	kfree(src);
exit:
	return ret;
}

/***************** CEN FOPS ****************************/
static const struct file_operations felica_cen_fops = {
	.owner		= THIS_MODULE,
	.read		= felica_cen_read,
	.write		= felica_cen_write,
	.open		= felica_cen_open,
	.release	= felica_cen_release,
	.fsync		= NULL,
};

static int felica_cen_probe_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	/* Create CEN character device (/dev/felica_cen) */
	if (misc_register(&dev->device_cen)) {
		dev_err(dev->dev, "%s: Error. Cannot register CEN.\n",
				__func__);
		return -ENODEV;
	}

	return 0;
}

static void felica_cen_remove_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	misc_deregister(&dev->device_cen);
}

static irqreturn_t felica_int_irq(int irq, void *dev)
{
	int state;
	struct felica_dev *d = dev;

	/* Read INT GPIO */
	state = d->flint->int_read(d);

	if (0 == state || 1 == state) {
		/* Update value of the switch device */
		switch_set_state(&d->swdev, state);
		dev_dbg(d->dev, "INT state = %d\n", state);
	} else {
		dev_err(d->dev, "%s: Error. Cannot read INT GPIO.\n", __func__);
	}

	return IRQ_HANDLED;
}

static int felica_int_probe_func(struct felica_dev *dev)
{
	int ret;
	int state;

	dev_dbg(dev->dev, "%s\n", __func__);

	/* Init INT GPIO */
	ret = dev->flint->int_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s: Error. INT GPIO init failed.\n",
				__func__);
		ret = -ENODEV;
		goto err_request_int_gpio;
	}

	/* Create INT switch device (felica_push) */
	dev->swdev.name = "felica_push";
	if (switch_dev_register(&dev->swdev)) {
		dev_err(dev->dev, "%s: Error. Cannot create switch dev\n",
					__func__);
		ret = -ENOMEM;
		goto err_create_switch_dev;
	}

	/* Read INT GPIO */
	state = dev->flint->int_read(dev);
	if (0 > state) {
		ret = -EIO;
		goto err_read_state;
	}
	/* Set state of the device */
	switch_set_state(&dev->swdev, state);

	/* Request IRQ for INT GPIO */
	ret = request_threaded_irq(dev->flint->irq_int, NULL, felica_int_irq,
	 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "felica_irq", dev);
	if (ret) {
		dev_err(dev->dev, "%s: Error. Request IRQ failed.\n", __func__);
		ret = -EIO;
		goto err_request_int_irq;
	}

	/* Enable IRQ wake */
	ret = enable_irq_wake(dev->flint->irq_int);
	if (ret) {
		dev_err(dev->dev, "%s: Error. Enabling IRQ wake failed.\n",
					__func__);
		ret = -EIO;
		goto err_enable_irq_wake;
	}

	return 0;

err_enable_irq_wake:
	free_irq(dev->flint->irq_int, NULL);
err_request_int_irq:
err_read_state:
	switch_dev_unregister(&dev->swdev);
err_create_switch_dev:
	dev->flint->int_release(dev);
err_request_int_gpio:
	return ret;
}

static void felica_int_remove_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	switch_dev_unregister(&dev->swdev);
	disable_irq_wake(dev->flint->irq_int);
	free_irq(dev->flint->irq_int, NULL);
	dev->flint->int_release(dev);
}

static int felica_pon_open(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_pon);

	dev_dbg(d->device_pon.this_device, "%s\n", __func__);

	return 0;
}

static int felica_pon_release(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_pon);

	dev_dbg(d->device_pon.this_device, "%s\n", __func__);

	/* Forcedly, write Low to PON GPIO */
	d->flpon->pon_write(0, d);

	/* Forcedly, turn off TVDD */
	d->flpon->tvdd_off(d);

	return 0;
}

static ssize_t felica_pon_write(struct file *file, const char __user *buf,
					size_t count, loff_t *offset)
{
	int ret;
	char kbuf;
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_pon);
	unsigned long flags;

	dev_dbg(d->device_pon.this_device, "%s\n", __func__);

	if (1 != count || buf == NULL) {
		dev_err(d->device_pon.this_device,
				"%s: Error. Invalid arg @PON write.\n",
					__func__);
		ret = -EINVAL;
		goto exit;
	}

	/* Copy value from user space */
	if (copy_from_user(&kbuf, buf, sizeof(kbuf))) {
		dev_err(d->device_pon.this_device,
			"%s: Error. copy_from_user failure.\n", __func__);
		ret = -EFAULT;
		goto exit;
	}

	if (1 == kbuf) {
		/* Write High to PON GPIO */
		d->flpon->pon_write(1, d);

		spin_lock_irqsave(&spinlock, flags);
		if (((0 == d->st_usbcon)
					&& (0 == d->st_airplane))
					|| (1 == d->ta_rwusb)) {
			spin_unlock_irqrestore(&spinlock, flags);
			dev_dbg(d->device_pon.this_device, "TVDD --> ON\n");
			/* Turn on TVDD */

			if (d->flpon->tvdd_on(d)) {
				dev_err(d->device_pon.this_device,
					"%s: Error. Cannot ctrl TVDD.\n",
					__func__);
				ret = -EIO;
				goto exit;
			}
		} else {
			spin_unlock_irqrestore(&spinlock, flags);
			dev_dbg(d->device_pon.this_device, "TVDD == OFF\n");
		}
	} else if (0 == kbuf) {
		/* Write LOW to PON GPIO */
		d->flpon->pon_write(0, d);
		/* Forcedly, turn off TVDD */
		d->flpon->tvdd_off(d);
	} else {
		dev_err(d->device_pon.this_device,
			"%s: Error. Invalid val @PON write.\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	/* 1 byte write */
	ret = 1;

exit:
	return ret;
}

/***************** PON FOPS ****************************/
static const struct file_operations felica_pon_fops = {
	.owner		= THIS_MODULE,
	.read		= NULL,
	.write		= felica_pon_write,
	.open		= felica_pon_open,
	.release	= felica_pon_release,
	.fsync		= NULL,
};

static int felica_pon_probe_func(struct felica_dev *dev)
{
	int ret;

	dev_dbg(dev->dev, "%s\n", __func__);

	/* Init PON GPIO */
	ret = dev->flpon->pon_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s: Error. PON GPIO init failed.\n",
				__func__);
		ret = -ENODEV;
		goto err_request_pon_gpio;
	}

	/* Disable TVDD vreg device */
	dev->flpon->tvdd_off(dev);

	/* Create PON character device (/dev/felica_pon) */
	if (misc_register(&dev->device_pon)) {
		dev_err(dev->dev, "%s: Error. Cannot register PON.\n",
				__func__);
		ret = -ENODEV;
		goto err_create_pon_dev;
	}

	return 0;

err_create_pon_dev:
	dev->flpon->pon_release(dev);
err_request_pon_gpio:
	return ret;
}

static void felica_pon_remove_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	misc_deregister(&dev->device_pon);
	dev->flpon->pon_release(dev);
}

static int felica_rfs_open(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rfs);

	dev_dbg(d->device_rfs.this_device, "%s\n", __func__);

	return 0;
}

static int felica_rfs_release(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rfs);

	dev_dbg(d->device_rfs.this_device, "%s\n", __func__);

	return 0;
}

static ssize_t felica_rfs_read(struct file *file, char __user *buf,
					size_t count, loff_t *offset)
{
	char kbuf;
	int gpio_val;
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rfs);

	dev_dbg(d->device_rfs.this_device, "%s\n", __func__);

	/* Read RFS GPIO value */
	gpio_val = d->flrfs->rfs_read(d);

	if (0 == gpio_val) {
		kbuf = 1;
	} else if (1 == gpio_val) {
		kbuf = 0;
	} else {
		dev_err(d->device_rfs.this_device,
				"%s: Error. Invalid GPIO value @RFS read.\n",
					__func__);
		return -EIO;
	}

	/* Copy the value to user space */
	if (copy_to_user(buf, &kbuf, sizeof(kbuf))) {
		dev_err(d->device_rfs.this_device,
				"%s: Error. copy_to_user failure.\n", __func__);
		return -EFAULT;
	}

	/* 1 byte read */
	return sizeof(kbuf);
}

/***************** RFS FOPS ****************************/
static const struct file_operations felica_rfs_fops = {
	.owner		= THIS_MODULE,
	.read		= felica_rfs_read,
	.write		= NULL,
	.open		= felica_rfs_open,
	.release	= felica_rfs_release,
	.fsync		= NULL,
};

static int felica_rfs_probe_func(struct felica_dev *dev)
{
	int ret;

	dev_dbg(dev->dev, "%s\n", __func__);

	/* Init RFS GPIO */
	ret = dev->flrfs->rfs_init(dev);
	if (ret) {
		dev_err(dev->dev, "%s: Error. RFS GPIO init failed.\n",
				__func__);
		ret = -ENODEV;
		goto err_request_rfs_gpio;
	}

	/* Create RFS character device (/dev/felica_rfs) */
	if (misc_register(&dev->device_rfs)) {
		dev_err(dev->dev, "%s: Error. Cannot register RFS.\n",
				__func__);
		ret = -ENODEV;
		goto err_create_rfs_dev;
	}

	return 0;

err_create_rfs_dev:
	dev->flrfs->rfs_release(dev);
err_request_rfs_gpio:
	return ret;
}

static void felica_rfs_remove_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	misc_deregister(&dev->device_rfs);
	dev->flrfs->rfs_release(dev);
}

static int felica_rws_open(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rws);

	dev_dbg(d->device_rws.this_device, "%s\n", __func__);

	return 0;
}

static int felica_rws_release(struct inode *inode, struct file *file)
{
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rws);

	dev_dbg(d->device_rws.this_device, "%s\n", __func__);

	return 0;
}

static ssize_t felica_rws_read(struct file *file, char __user *buf,
					size_t count, loff_t *offset)
{
	int ret;
	char kbuf;
	struct miscdevice *c = file->private_data;
	struct felica_dev *d = container_of(c, struct felica_dev, device_rws);
	unsigned long flags;

	dev_dbg(d->device_rws.this_device, "%s\n", __func__);

	spin_lock_irqsave(&spinlock, flags);
	dev_dbg(d->device_rws.this_device,
			"st_usbcon = 0x%x\n", d->st_usbcon);
	dev_dbg(d->device_rws.this_device,
			"st_airplane = 0x%x\n", d->st_airplane);
	dev_dbg(d->device_rws.this_device,
			"ta_rwusb = 0x%x\n", d->ta_rwusb);

	/* Check R/W module parameters */
	if (0 != d->st_usbcon && 1 != d->st_usbcon) {
		dev_err(d->device_rws.this_device,
				"%s: Error. Invalid st_usbcon value.\n",
					__func__);
		ret = -EIO;
		goto exit;
	}
	if (0 != d->st_airplane && 1 != d->st_airplane) {
		dev_err(d->device_rws.this_device,
				"%s: Error. Invalid st_airplane value.\n",
					__func__);
		ret = -EIO;
		goto exit;
	}
	if (0 != d->ta_rwusb && 1 != d->ta_rwusb) {
		dev_err(d->device_rws.this_device,
				"%s: Error. Invalid ta_rwusb value.\n",
					__func__);
		ret = -EIO;
		goto exit;
	}

	/* Determine RWS value */
	if ((1 == d->st_usbcon || 1 == d->st_airplane)
		&& 0 == d->ta_rwusb)
		kbuf = 1;
	else
		kbuf = 0;

	dev_dbg(d->device_rws.this_device, "kbuf = 0x%x\n", kbuf);

	/* Copy the value to user space */
	if (copy_to_user(buf, &kbuf, sizeof(kbuf))) {
		dev_err(d->device_rws.this_device,
				"%s: Error. copy_to_user failure.\n", __func__);
		ret = -EFAULT;
		goto exit;
	}

	/* 1 byte read */
	ret = sizeof(kbuf);
exit:
	spin_unlock_irqrestore(&spinlock, flags);
	return ret;
}

/***************** RWS FOPS ****************************/
static const struct file_operations felica_rws_fops = {
	.owner		= THIS_MODULE,
	.read		= felica_rws_read,
	.write		= NULL,
	.open		= felica_rws_open,
	.release	= felica_rws_release,
};

static int felica_rws_probe_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	/* Create RWS character device (/dev/felica_rws) */
	if (misc_register(&dev->device_rws)) {
		dev_err(dev->dev, "%s: Error. Cannot register RWS.\n",
				__func__);
		return -ENODEV;
	}

	return 0;
}

static void felica_rws_remove_func(struct felica_dev *dev)
{
	dev_dbg(dev->dev, "%s\n", __func__);

	misc_deregister(&dev->device_rws);
}

static int felica_probe(struct platform_device *pdev)
{
	int	ret;
	struct felica_dev *dev;
	struct felica_platform_data	*flc_pfdata;

	dev_info(&pdev->dev, "FeliCa driver ver %s being loaded\n",
							DRV_VERSION);

	flc_pfdata = pdev->dev.platform_data;
	if (NULL == flc_pfdata) {
		dev_err(&pdev->dev, "%s: No platform data\n", __func__);
		ret = -EINVAL;
		goto err_get_platform_data;
	}

	dev = kzalloc(sizeof(struct felica_dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&pdev->dev, "%s: no memory\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_data;
	}

	dev->dev = &pdev->dev;
	platform_set_drvdata(pdev, dev);

	dev->device_cen.minor = MISC_DYNAMIC_MINOR;
	dev->device_cen.name = "felica_cen";
	dev->device_cen.fops = &felica_cen_fops;

	dev->device_pon.minor = MISC_DYNAMIC_MINOR;
	dev->device_pon.name = "felica_pon";
	dev->device_pon.fops = &felica_pon_fops;

	dev->device_rfs.minor = MISC_DYNAMIC_MINOR;
	dev->device_rfs.name = "felica_rfs";
	dev->device_rfs.fops = &felica_rfs_fops;

	dev->device_rws.minor = MISC_DYNAMIC_MINOR;
	dev->device_rws.name = "felica_rws";
	dev->device_rws.fops = &felica_rws_fops;

	dev->flcen = &flc_pfdata->cen_pfdata;
	dev->flpon = &flc_pfdata->pon_pfdata;
	dev->flrfs = &flc_pfdata->rfs_pfdata;
	dev->flint = &flc_pfdata->int_pfdata;

	/* GPIO setting of MSM & PM */
	ret = flc_pfdata->gpio_init(dev);
	if (ret && -EBUSY != ret) {
		dev_err(&pdev->dev, "%s: GPIO init failed\n", __func__);
		goto error_gpio_init;
	}

	/* Call initialization functions of each controller */
	ret = felica_cen_probe_func(dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: CEN probe failure\n", __func__);
		goto err_cen_probe;
	}

	ret = felica_pon_probe_func(dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: PON probe failure\n", __func__);
		goto err_pon_probe;
	}

	ret = felica_rfs_probe_func(dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: RFS probe failure\n", __func__);
		goto err_rfs_probe;
	}

	ret = felica_int_probe_func(dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: INT probe failure\n", __func__);
		goto err_int_probe;
	}

	ret = felica_rws_probe_func(dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: RWS probe failure\n", __func__);
		goto err_rws_probe;
	}

	/* Create dev. attrs for Reader/Writer mode */
	ret = create_sysfs_interfaces(&pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "%s: failed to create dev. attrs\n",
				__func__);
		goto err_create_sysfs;
	}

	return 0;

/* Error handling */
err_create_sysfs:
	felica_rws_remove_func(dev);
err_rws_probe:
	felica_int_remove_func(dev);
err_int_probe:
	felica_rfs_remove_func(dev);
err_rfs_probe:
	felica_pon_remove_func(dev);
err_pon_probe:
	felica_cen_remove_func(dev);
err_cen_probe:
	flc_pfdata->reg_release(dev);
error_gpio_init:
	kfree(dev);
err_alloc_data:
err_get_platform_data:

	return ret;
}

static int felica_remove(struct platform_device *pdev)
{
	struct felica_dev *dev = platform_get_drvdata(pdev);
	struct felica_platform_data	*flc_pfdata;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	remove_sysfs_interfaces(&pdev->dev);

	felica_rws_remove_func(dev);
	felica_int_remove_func(dev);
	felica_rfs_remove_func(dev);
	felica_pon_remove_func(dev);
	felica_cen_remove_func(dev);

	flc_pfdata = pdev->dev.platform_data;
	flc_pfdata->reg_release(dev);

	kfree(dev);

	return 0;
}

static struct platform_driver semc_felica_driver = {
	.probe		= felica_probe,
	.remove		= felica_remove,
	.driver		= {
		.name		= FELICA_DEV_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init felica_init(void)
{
	int ret;

	ret = platform_driver_register(&semc_felica_driver);
	return ret;
}

static void __exit felica_exit(void)
{
	platform_driver_unregister(&semc_felica_driver);
}

module_init(felica_init);
module_exit(felica_exit);

MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("SEMC");
MODULE_DESCRIPTION("FeliCa driver");
MODULE_LICENSE("GPL");
