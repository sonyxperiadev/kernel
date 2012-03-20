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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/bcmi2cnfc.h>
#include <linux/poll.h>
#include <linux/version.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#define     TRUE                1
#define     FALSE               0
#define     STATE_HIGH          1
#define     STATE_LOW           0

#define     NFC_REQ_ACTIVE_STATE    STATE_HIGH
#define     NFC_WAKE_ACTIVE_STATE   STATE_HIGH
#define     NFC_REQ_INACTIVE_STATE  STATE_LOW

#define     CHANGE_CLIENT_ADDR  TRUE

#define     READ_MULTIPLE_PACKETS   0

/* end of compile options, do not change below */

#ifdef CONFIG_DEBUG_FS

#define DBG(a...) \
	do { \
	    if (debug_en) \
			a;  \
	} while (0)

#define DBG2(a...)  \
	do {	\
		if (debug_en > 1)  {\
			 a; \
		}	\
	} while (0)

static int debug_en;

#endif

#define     MAX_BUFFER_SIZE     780

struct bcmi2cnfc_dev {
	wait_queue_head_t read_wq;
	struct mutex read_mutex;
	struct i2c_client *client;
	struct miscdevice bcmi2cnfc_device;
	unsigned int wake_gpio;
	unsigned int en_gpio;
	unsigned int irq_gpio;
	bool irq_enabled;
	spinlock_t irq_enabled_lock;
	unsigned int packet_size;
	unsigned int wake_active_state;
	unsigned int read_multiple_packets;
};

#define INTERRUPT_TRIGGER_TYPE  IRQF_TRIGGER_RISING

#if CHANGE_CLIENT_ADDR
/*
 The alias address 0x79, when sent as a 7-bit address from the host
 processor will match the first byte (highest 2 bits) of the default
 client address (0x1FA) that is programmed in bcm20791.
 When used together with the first byte (0xFA) of the byte sequence below,
 it can be used to address the bcm20791
 in a system that does not support 10-bit address and change the default
 address to 0x38. the new address can be changed by changing the
 CLIENT_ADDRESS below if 0x38 conflicts with other device on the same i2c bus.
*/

#define ALIAS_ADDRESS       0x79
static char addr_data[] = {
	0xFA, 0xF2, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x2A
};

static void change_client_addr(struct bcmi2cnfc_dev *bcmi2cnfc_dev, int addr)
{
	struct i2c_client *client;
	int ret;
	int i;

	client = bcmi2cnfc_dev->client;

	client->addr = ALIAS_ADDRESS;
	client->flags &= ~I2C_CLIENT_TEN;

	addr_data[5] = addr & 0xFF;
	ret = 0;
	for (i = 1; i < sizeof(addr_data) - 1; ++i)
		ret += addr_data[i];
	addr_data[sizeof(addr_data) - 1] = (ret & 0xFF);
	DBG(dev_info(&client->dev,
		     "Change client device at (0x%04X) flag = %04x, addr_data[%d] = %02x\n",
		     client->addr, client->flags, sizeof(addr_data) - 1,
		     addr_data[sizeof(addr_data) - 1]));
	ret = i2c_master_send(client, addr_data, sizeof(addr_data));
	client->addr = addr_data[5];

	DBG(dev_info(&client->dev,
		     "Change client device changed to (0x%04X) flag = %04x, ret = %d\n",
		     client->addr, client->flags, ret));
}
#endif

static irqreturn_t bcmi2cnfc_dev_irq_handler(int irq, void *dev_id)
{
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = dev_id;

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		"irq %llu\n", get_jiffies_64()));

	if (gpio_get_value(bcmi2cnfc_dev->irq_gpio) != NFC_REQ_ACTIVE_STATE)
		return IRQ_HANDLED;

	/* Wake up waiting readers */
	wake_up(&bcmi2cnfc_dev->read_wq);

	return IRQ_HANDLED;
}

static unsigned int bcmi2cnfc_dev_poll(struct file *filp, poll_table * wait)
{
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = filp->private_data;
	unsigned int mask = 0;

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_poll enter %llu\n", get_jiffies_64()));

	poll_wait(filp, &bcmi2cnfc_dev->read_wq, wait);

	if (gpio_get_value(bcmi2cnfc_dev->irq_gpio) == NFC_REQ_ACTIVE_STATE)
		mask |= POLLIN | POLLRDNORM;

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_poll leave %llu\n", get_jiffies_64()));

	return mask;
}

static ssize_t bcmi2cnfc_dev_read(struct file *filp, char __user *buf,
				  size_t count, loff_t *offset)
{
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = filp->private_data;
	unsigned char tmp[MAX_BUFFER_SIZE];
	int ret, len, total, packets, prev_total;

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_read %llu\n", get_jiffies_64()));

	ret = 0;
	len = 1;
	total = 0;
	packets = 0;
	prev_total = 0;
	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (bcmi2cnfc_dev->packet_size > 0) {
		count = bcmi2cnfc_dev->packet_size;
		len = count;
	}

	/* Read data */

#define PACKET_HEADER_SIZE_NCI      (4)
#define PACKET_HEADER_SIZE_HCI      (3)
#define PACKET_TYPE_NCI             (16)
#define PACKET_TYPE_HCIEV           (4)
#define MAX_PACKET_SIZE             (PACKET_HEADER_SIZE_NCI + 255)

	do {
		ret = i2c_master_recv(bcmi2cnfc_dev->client, tmp + total, len);

		if (ret != len) {
			if (total == 0)
				dev_err(&bcmi2cnfc_dev->client->dev,
					"(type byte)IO Error ret=%d !!!!\n",
					ret);
			break;
		}

		if (len == count) {
			total = ret;
			++packets;
			break;
		}

		if (tmp[total] == PACKET_TYPE_NCI)
			len = PACKET_HEADER_SIZE_NCI - 1;
		else if (tmp[total] == PACKET_TYPE_HCIEV)
			len = PACKET_HEADER_SIZE_HCI - 1;
		else {
			dev_err(&bcmi2cnfc_dev->client->dev,
				"unknown type header tmp[%d]=%d !!!!\n", total,
				tmp[total]);
			break;
		}
		if (len > 0) {
			/* read the remainder of the packet header */
			ret =
			    i2c_master_recv(bcmi2cnfc_dev->client,
					    tmp + total + 1, len);
			if (ret != len) {
				dev_err(&bcmi2cnfc_dev->client->dev,
					"(type header) IO Error ret = %d!!!!\n",
					ret);
				total = prev_total;
				break;
			}
			/* get the packet payload length */
			len = tmp[total + ret];
			total += ret + 1;
		}
		if (len > 0) {
			/* read the packet payload */
			ret =
			    i2c_master_recv(bcmi2cnfc_dev->client, tmp + total,
					    len);
			if (ret > 0) {
				total += len;
			} else {
				dev_err(&bcmi2cnfc_dev->client->dev,
					"packet IO Error ret = %d!!!!\n", ret);
				total = prev_total;
				break;
			}
		}
		++packets;
		len = 1;
		prev_total = total;
	} while (bcmi2cnfc_dev->read_multiple_packets > 0
		 && total + MAX_PACKET_SIZE <= count);

	if (total > 0)
		if (total > count || copy_to_user(buf, tmp, total)) {
			dev_err(&bcmi2cnfc_dev->client->dev,
				"failed to copy to user space, total = %d\n",
				total);
			total = -EFAULT;
		}

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_read leave %llu\n", get_jiffies_64()));

	/*if (packets > 1) */
	DBG(dev_info(&bcmi2cnfc_dev->client->dev,
		     "read %d packets total = %d bytes\n", packets, total));
	return total > 0 ? total : len;
}

static ssize_t bcmi2cnfc_dev_write(struct file *filp, const char __user *buf,
				   size_t count, loff_t *offset)
{
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret;

	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_write enter %llu\n", get_jiffies_64()));

	DBG(dev_info(&bcmi2cnfc_dev->client->dev, "%s, %d\n", __func__, count));

	if (count > MAX_BUFFER_SIZE) {
		dev_err(&bcmi2cnfc_dev->client->dev, "out of memory\n");
		return -ENOMEM;
	}

	if (copy_from_user(tmp, buf, count)) {
		dev_err(&bcmi2cnfc_dev->client->dev,
			"failed to copy from user space\n");
		return -EFAULT;
	}

	/* Write data */
	ret = i2c_master_send(bcmi2cnfc_dev->client, tmp, count);
	if (ret != count) {
		dev_err(&bcmi2cnfc_dev->client->dev,
			"failed to write %d\n", ret);
		ret = -EIO;
	}
	DBG2(dev_info(&bcmi2cnfc_dev->client->dev,
		      "bcmi2cnfc_dev_write leave %llu\n", get_jiffies_64()));

	return ret;
}

static int bcmi2cnfc_dev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = container_of(filp->private_data,
							   struct bcmi2cnfc_dev,
							   bcmi2cnfc_device);

	filp->private_data = bcmi2cnfc_dev;
	bcmi2cnfc_dev->wake_active_state = NFC_WAKE_ACTIVE_STATE;
	bcmi2cnfc_dev->read_multiple_packets = READ_MULTIPLE_PACKETS;

	DBG(dev_info(&bcmi2cnfc_dev->client->dev,
		     "%d,%d\n", imajor(inode), iminor(inode)));

	return ret;
}

static long bcmi2cnfc_dev_unlocked_ioctl(struct file *filp,
					 unsigned int cmd, unsigned long arg)
{
	struct bcmi2cnfc_dev *bcmi2cnfc_dev = filp->private_data;

	switch (cmd) {
	case BCMNFC_SET_WAKE_ACTIVE_STATE:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_SET_WAKE_ACTIVE_STATE (%x, %lx):\n",
			     __func__, cmd, arg));
		bcmi2cnfc_dev->wake_active_state = arg;
		break;
	case BCMNFC_READ_FULL_PACKET:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_READ_FULL_PACKET (%x, %lx):\n",
			     __func__, cmd, arg));
		bcmi2cnfc_dev->packet_size = arg;
		break;
	case BCMNFC_READ_MULTI_PACKETS:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_READ_MULTI_PACKETS (%x, %lx):\n",
			     __func__, cmd, arg));
		bcmi2cnfc_dev->read_multiple_packets = arg;
		break;
#if CHANGE_CLIENT_ADDR
	case BCMNFC_CHANGE_ADDR:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_CHANGE_ADDR (%x, %lx):\n", __func__,
			     cmd, arg));
		change_client_addr(bcmi2cnfc_dev, arg);
		break;
#endif
	case BCMNFC_POWER_CTL:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_POWER_CTL (%x, %lx):\n", __func__, cmd,
			     arg));
		if (arg == 1) {	/* Power On */
			gpio_set_value(bcmi2cnfc_dev->en_gpio, 1);
			enable_irq(bcmi2cnfc_dev->client->irq);
		} else {
			disable_irq_nosync(bcmi2cnfc_dev->client->irq);
			gpio_set_value(bcmi2cnfc_dev->en_gpio, 0);
		}
		break;
	case BCMNFC_WAKE_CTL:
		DBG(dev_info(&bcmi2cnfc_dev->client->dev,
			     "%s, BCMNFC_WAKE_CTL (%x, %lx):\n", __func__, cmd,
			     arg));
		gpio_set_value(bcmi2cnfc_dev->wake_gpio, arg);
		break;
	default:
		dev_err(&bcmi2cnfc_dev->client->dev,
			"%s, unknown cmd (%x, %lx)\n", __func__, cmd, arg);
		return 0;
	}

	return 0;
}

static int bcmi2cnfc_dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations bcmi2cnfc_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.poll = bcmi2cnfc_dev_poll,
	.read = bcmi2cnfc_dev_read,
	.write = bcmi2cnfc_dev_write,
	.open = bcmi2cnfc_dev_open,
	.release = bcmi2cnfc_dev_release,
	.unlocked_ioctl = bcmi2cnfc_dev_unlocked_ioctl
};

static int bcmi2cnfc_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	int ret;
	struct bcmi2cnfc_i2c_platform_data *platform_data;
	struct bcmi2cnfc_dev *bcmi2cnfc_dev;
	platform_data = client->dev.platform_data;

	DBG(dev_info(&client->dev, "%s, probing bcmi2cnfc driver\n", __func__));
	if (platform_data == NULL) {
		dev_err(&client->dev, "nfc probe fail\n");
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "need I2C_FUNC_I2C\n");
		return -ENODEV;
	}

	bcmi2cnfc_dev = kzalloc(sizeof(*bcmi2cnfc_dev), GFP_KERNEL);
	if (bcmi2cnfc_dev == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	bcmi2cnfc_dev->wake_gpio = platform_data->wake_gpio;
	bcmi2cnfc_dev->irq_gpio = platform_data->irq_gpio;
	bcmi2cnfc_dev->en_gpio = platform_data->en_gpio;
	bcmi2cnfc_dev->client = client;

	platform_data->init(platform_data);

	/* init mutex and queues */
	init_waitqueue_head(&bcmi2cnfc_dev->read_wq);
	mutex_init(&bcmi2cnfc_dev->read_mutex);
	spin_lock_init(&bcmi2cnfc_dev->irq_enabled_lock);

	bcmi2cnfc_dev->bcmi2cnfc_device.minor = MISC_DYNAMIC_MINOR;
	bcmi2cnfc_dev->bcmi2cnfc_device.name = "bcmi2cnfc";
	bcmi2cnfc_dev->bcmi2cnfc_device.fops = &bcmi2cnfc_dev_fops;

	ret = misc_register(&bcmi2cnfc_dev->bcmi2cnfc_device);
	if (ret) {
		dev_err(&client->dev, "misc_register failed\n");
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	DBG(dev_info(&client->dev, "requesting IRQ %d\n", client->irq));
	bcmi2cnfc_dev->irq_enabled = true;

	ret = request_irq(client->irq, bcmi2cnfc_dev_irq_handler,
			  INTERRUPT_TRIGGER_TYPE, client->name, bcmi2cnfc_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}

	disable_irq_nosync(client->irq);

	i2c_set_clientdata(client, bcmi2cnfc_dev);
	bcmi2cnfc_dev->packet_size = 0;
	DBG(dev_info(&client->dev,
		     "%s, probing bcmi2cnfc driver exited successfully\n",
		     __func__));
	return 0;

err_request_irq_failed:
	misc_deregister(&bcmi2cnfc_dev->bcmi2cnfc_device);
err_misc_register:
	mutex_destroy(&bcmi2cnfc_dev->read_mutex);
	kfree(bcmi2cnfc_dev);
err_exit:
	gpio_free(platform_data->wake_gpio);
	gpio_free(platform_data->en_gpio);
	gpio_free(platform_data->irq_gpio);
	return ret;
}

static int bcmi2cnfc_remove(struct i2c_client *client)
{
	struct bcmi2cnfc_i2c_platform_data *platform_data;
	struct bcmi2cnfc_dev *bcmi2cnfc_dev;
	DBG(dev_info
	    (&client->dev, "%s, Removing bcmi2cnfc driver\n", __func__));

	platform_data = client->dev.platform_data;

	bcmi2cnfc_dev = i2c_get_clientdata(client);
	free_irq(client->irq, bcmi2cnfc_dev);
	misc_deregister(&bcmi2cnfc_dev->bcmi2cnfc_device);
	mutex_destroy(&bcmi2cnfc_dev->read_mutex);
	kfree(bcmi2cnfc_dev);
	platform_data->reset(platform_data);

	return 0;
}

static const struct i2c_device_id bcmi2cnfc_id[] = {
	{"bcmi2cnfc", 0},
	{}
};

static struct i2c_driver bcmi2cnfc_driver = {
	.id_table = bcmi2cnfc_id,
	.probe = bcmi2cnfc_probe,
	.remove = bcmi2cnfc_remove,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "bcmi2cnfc",
		   },
};

/*
 * module load/unload record keeping
 */

static int __init bcmi2cnfc_dev_init(void)
{
	pr_info("Loading bcmi2cnfc driver\n");
	return i2c_add_driver(&bcmi2cnfc_driver);
}

module_init(bcmi2cnfc_dev_init);

static void __exit bcmi2cnfc_dev_exit(void)
{
	pr_info("Unloading bcmi2cnfc driver\n");
	i2c_del_driver(&bcmi2cnfc_driver);
}

module_exit(bcmi2cnfc_dev_exit);

#ifdef CONFIG_DEBUG_FS

int __init bcmi2cnfc_debug_init(void)
{
	struct dentry *root_dir;
	root_dir = debugfs_create_dir("bcmi2cnfc", 0);
	if (!root_dir)
		return -ENOMEM;
	if (!debugfs_create_u32
	    ("debug", S_IRUSR | S_IWUSR, root_dir, &debug_en))
		return -ENOMEM;

	return 0;

}

late_initcall(bcmi2cnfc_debug_init);

#endif

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("NFC bcmi2cnfc driver");
MODULE_LICENSE("GPL");
