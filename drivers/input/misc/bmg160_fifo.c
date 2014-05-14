/*
 * bmg160_fifo.c -  Linux kernel driver for BOSCH bmg160 gyroscope
 *
 * Copyright (c) 2012-2014 Sony Mobile Communications AB
 *
 * Authors: Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
 * Authors: Aleksej Makarov <aleksej.makarov@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>

#define BMG160_NAME                      "bmg160"

#define BMG160_CHIP_ID_REG               0x00
#define BMG160_CHIP_ID_BMG160            0x0f

#define BMG160_RANGE_REG                 0x0F
#define BMG160_RANGE_MASK                0x07
#define BMG160_RANGE_2000                0x00
#define BMG160_RANGE_1000                0x01
#define BMG160_RANGE_500                 0x02
#define BMG160_RANGE_250                 0x03
#define BMG160_RANGE_125                 0x04

#define BMG160_UR_BW_SEL_REG             0x10
#define BMG160_UR_BW_100HZ_32HZ          0x07
#define BMG160_UR_BW_200HZ_64HZ          0x06
#define BMG160_UR_BW_100HZ_12HZ          0x05
#define BMG160_UR_BW_200HZ_23HZ          0x04
#define BMG160_UR_BW_400HZ_47HZ          0x03
#define BMG160_UR_BW_1000HZ_116HZ        0x02
#define BMG160_UR_BW_2000HZ_230HZ        0x01
#define BMG160_UR_BW_2000HZ_583HZ        0x00

#define BMG160_MODE_CTRL_REG             0x11
#define BMG160_MODE_SUSPEND              0x80
#define BMG160_MODE_DEEPSUSPEND          0x20

#define BMG160_FIFO_ENA_REG              0x15
#define BMG160_FIFO_ENA_BIT          (1 << 6)

#define BMG160_FIFO_WM_ENA_REG           0x1E
#define BMG160_FIFO_WM_ENA_BIT       (1 << 7)

#define BMG160_FIFO_CFG0_REG             0x3D
#define BMG160_FIFO_WM_MASK              0x7F

#define BMG160_FIFO_CFG1_REG             0x3E
#define BMG160_FIFO_STREAM            (2 << 6)

#define BMG160_FRAME_CNT_REG             0x0E
#define BMG160_FRAME_CNT_MASK            0x7F
#define BMG160_FIFO_OVERRUN           (1 << 7)

#define BMG160_RESET_REG                 0x14
#define BMG160_RESET                     0xB6

#define BMG160_SOC                       0x31
#define BMG160_OFC1_REG                  0x36
#define BMG160_OFC2_REG                  0x37
#define BMG160_OFC3_REG                  0x38
#define BMG160_OFC4_REG                  0x39
#define BMG160_TRIMGP0_REG               0x3A
#define BMG160_FIFO_DATA_REG             0x3F

#define BMG160_LAST_REG 0x3F
#define BMG160_VENDORID 0x0001
#define FIFO_MAX 100

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("bmg160");

static const int min_wm[] = {
	[BMG160_UR_BW_2000HZ_583HZ] = 10,
	[BMG160_UR_BW_2000HZ_230HZ] = 10,
	[BMG160_UR_BW_1000HZ_116HZ] = 5,
	[BMG160_UR_BW_400HZ_47HZ] = 2,
	[BMG160_UR_BW_200HZ_23HZ] = 1,
	[BMG160_UR_BW_100HZ_12HZ] = 1,
	[BMG160_UR_BW_200HZ_64HZ] = 1,
	[BMG160_UR_BW_100HZ_32HZ] = 1,
};

static const int data_rates_us[] = {
	[BMG160_UR_BW_2000HZ_583HZ] = 1000000 / 2000,
	[BMG160_UR_BW_2000HZ_230HZ] = 1000000 / 2000,
	[BMG160_UR_BW_1000HZ_116HZ] = 1000000 / 1000,
	[BMG160_UR_BW_400HZ_47HZ] = 1000000 / 400,
	[BMG160_UR_BW_200HZ_23HZ] = 1000000 / 200,
	[BMG160_UR_BW_100HZ_12HZ] = 1000000 / 100,
	[BMG160_UR_BW_200HZ_64HZ] = 1000000 / 200,
	[BMG160_UR_BW_100HZ_32HZ] = 1000000 / 100,
};

struct frame {
	short x;
	short y;
	short z;
};

struct header {
	short cnt;
	struct frame data[0];
}  __packed;

#define IO_BUF_SIZE (FIFO_MAX * (sizeof(struct frame) + sizeof(struct header)))

enum bmg160_orientation {
	ORI_X_Y_Z,
	ORI_Y_NX_Z,
	ORI_NX_NY_Z,
	ORI_NY_X_Z,
	ORI_Y_X_NZ,
	ORI_X_NY_NZ,
	ORI_NY_NX_NZ,
	ORI_NX_Y_NZ,
};

static const char * const orientation_id[] = {
	"ORI_X_Y_Z",
	"ORI_Y_NX_Z",
	"ORI_NX_NY_Z",
	"ORI_NY_X_Z",
	"ORI_Y_X_NZ",
	"ORI_X_NY_NZ",
	"ORI_NY_NX_NZ",
	"ORI_NX_Y_NZ",
};

struct bmg160_data {
	struct i2c_client           *ic_dev;
	u8                           chip_id;
	struct workqueue_struct     *workqueue;
	struct work_struct           work;
	bool                         enabled;
	struct regulator            *regulator;
	u8                           wm_level;
	u8                           data_rate_reg;
	struct mutex                 lock;
	u8                           rx_buf[FIFO_MAX * 6];
	u8                           range;
	struct miscdevice            cdev;
	wait_queue_head_t            wq;
	u8                          *out_buf;
	u8                          *read_buf;
	int                          w_idx;
	spinlock_t                   data_lock;
	struct hrtimer               hr_timer;
	ktime_t                      ktime;
	bool                         flush_complete_req;
	enum bmg160_orientation      orientation;
};

static int bmg160_ic_read(struct i2c_client *ic_dev, u8 reg, u8 *buf, int len)
{
	int rc;
	struct i2c_msg msg[2] = {
		[0] =  {
			.addr = ic_dev->addr,
			.flags = ic_dev->flags & I2C_M_TEN,
			.len = 1,
			.buf = &reg,
		},
		[1] =  {
			.addr = ic_dev->addr,
			.flags = (ic_dev->flags & I2C_M_TEN) | I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

	rc = i2c_transfer(ic_dev->adapter, msg, ARRAY_SIZE(msg));
	if (rc != ARRAY_SIZE(msg)) {
		dev_err(&ic_dev->dev, "%s: reg 0x%02x, rc = %d\n",
				__func__, reg, rc);
		return -ENXIO;
	}
	return rc < 0 ? rc : 0;
}

static inline int bmg160_ic_write(struct i2c_client *ic_dev, u8 reg, u8 val)
{
	int rc = i2c_smbus_write_byte_data(ic_dev, reg, val);
	if (rc)
		dev_err(&ic_dev->dev, "%s: reg 0x%02x val 0x%02x, rc = %d\n",
				__func__, reg, val, rc);
	return rc;
}

static int bmg160_power_down(struct bmg160_data *bd)
{
	int rc;

	(void)bmg160_ic_write(bd->ic_dev, BMG160_MODE_CTRL_REG,
			BMG160_MODE_DEEPSUSPEND);
	if (!IS_ERR_OR_NULL(bd->regulator)) {
		dev_dbg(&bd->ic_dev->dev, "%s: disable regulator\n", __func__);
		rc = regulator_disable(bd->regulator);
		if (rc)
			dev_err(&bd->ic_dev->dev,
				"%s: regulator_disable failed %i\n",
				__func__, rc);
	}
	return rc;
}

static int bmg160_power_up(struct bmg160_data *bd)
{
	int rc;

	if (!IS_ERR_OR_NULL(bd->regulator)) {
		dev_dbg(&bd->ic_dev->dev, "%s: enable regulator\n", __func__);
		rc = regulator_enable(bd->regulator);
		if (rc) {
			dev_err(&bd->ic_dev->dev,
				"%s: regulator_enable failed %i\n",
				__func__, rc);
			return rc;
		}
	}
	usleep_range(450, 700);
	rc = bmg160_ic_write(bd->ic_dev, BMG160_RESET_REG, BMG160_RESET);
	if (!rc)
		msleep(30);
	return rc;
}

static int bmg160_read_chip_id(struct bmg160_data *bd)
{
	int rc;

	rc = bmg160_ic_read(bd->ic_dev, BMG160_CHIP_ID_REG,
						&bd->chip_id, 1);
	if (rc)
		dev_err(&bd->ic_dev->dev, "%s: unable to read chip id\n",
								__func__);
	else
		dev_info(&bd->ic_dev->dev, "bmg160: detected chip id %d\n",
							bd->chip_id);
	return rc;
}

static void bmg160_report_flush_complete(struct bmg160_data *bd)
{
	struct header *hdr;

	if (bd->w_idx + sizeof(*hdr) > IO_BUF_SIZE) {
		dev_warn(&bd->ic_dev->dev, "packet lost %d\n", bd->w_idx);
		bd->w_idx = 0;
	}
	hdr = (struct header *)(bd->out_buf + bd->w_idx);
	bd->w_idx += sizeof(*hdr);
	hdr->cnt = 0;
}

static void bmg160_report_data(struct work_struct *work)
{
	struct bmg160_data *bd =
			container_of(work, struct bmg160_data, work);
	int rc;
	u8 fcnt;
	struct header *hdr;
	int len;
	unsigned long flags;

	rc = bmg160_ic_read(bd->ic_dev, BMG160_FRAME_CNT_REG, &fcnt,
			sizeof(fcnt));
	if (rc)
		goto exit;

	if (fcnt & BMG160_FIFO_OVERRUN) {
		dev_warn(&bd->ic_dev->dev, "%s: FIFO overrun\n", __func__);
		(void)bmg160_ic_write(bd->ic_dev, BMG160_FIFO_CFG1_REG,
				BMG160_FIFO_STREAM);
	}
	fcnt &= BMG160_FRAME_CNT_MASK;

	if (fcnt) {
		rc = bmg160_ic_read(bd->ic_dev, BMG160_FIFO_DATA_REG,
				bd->rx_buf, (int)fcnt * sizeof(struct frame));
		if (rc)
			goto exit;
	}

	spin_lock_irqsave(&bd->data_lock, flags);
	if (!bd->out_buf)
		goto skip_report;
	if (!fcnt)
		goto skip_data;

	len = (int)fcnt * sizeof(struct frame) + sizeof(*hdr);
	if (bd->w_idx + len > IO_BUF_SIZE) {
		dev_warn(&bd->ic_dev->dev, "packet lost %d\n", bd->w_idx);
		bd->w_idx = 0;
	}
	hdr = (struct header *)(bd->w_idx + bd->out_buf);
	hdr->cnt = fcnt;
	memcpy(hdr->data, bd->rx_buf, (int)fcnt * sizeof(struct frame));
	bd->w_idx += len;

skip_data:
	if (bd->flush_complete_req) {
		bmg160_report_flush_complete(bd);
		bd->flush_complete_req = false;
	}

skip_report:
	spin_unlock_irqrestore(&bd->data_lock, flags);
	wake_up_interruptible(&bd->wq);
exit:
	return;
}

static int bmg160_update_wm(struct bmg160_data *bd, int wm)
{
	bd->wm_level = wm < min_wm[bd->data_rate_reg] ?
			min_wm[bd->data_rate_reg] : wm;
	bd->ktime = ktime_set(0, data_rates_us[bd->data_rate_reg] *
			bd->wm_level * 1000);
	dev_dbg(&bd->ic_dev->dev, "%s: %d\n", __func__, bd->wm_level);
	return bmg160_ic_write(bd->ic_dev, BMG160_FIFO_CFG0_REG,
			BMG160_FIFO_WM_MASK & bd->wm_level);
}

static int bmg160_write_settings(struct bmg160_data *bd)
{
	int rc = bmg160_ic_write(bd->ic_dev, BMG160_RANGE_REG, bd->range &
			BMG160_RANGE_MASK);

	rc = rc ? rc : bmg160_ic_write(bd->ic_dev, BMG160_UR_BW_SEL_REG,
			bd->data_rate_reg);

	rc = rc ? rc : bmg160_ic_write(bd->ic_dev, BMG160_FIFO_CFG1_REG,
			BMG160_FIFO_STREAM);

	rc = rc ? rc : bmg160_update_wm(bd, bd->wm_level);
	if (rc)
		dev_err(&bd->ic_dev->dev, "%s: error %d\n", __func__, rc);
	return rc;
}

static int bmg160_set_enable(struct bmg160_data *bd, int wm)
{
	int rc;

	mutex_lock(&bd->lock);
	if (wm) {
		if (bd->enabled) {
			rc = bmg160_update_wm(bd, wm);
			goto exit;
		}

		rc = bmg160_power_up(bd);
		if (rc)
			goto exit;

		rc = bmg160_write_settings(bd);
		if (rc)
			goto err_power_down;

		rc = bmg160_update_wm(bd, wm);
		if (rc)
			goto err_power_down;

		hrtimer_start(&bd->hr_timer, bd->ktime, HRTIMER_MODE_REL);
		bd->enabled = true;
	} else if (!wm && bd->enabled) {
		hrtimer_cancel(&bd->hr_timer);
		cancel_work_sync(&bd->work);
		bd->enabled = false;
err_power_down:
		rc = bmg160_power_down(bd);
	} else {
		rc = 0;
	}
exit:
	mutex_unlock(&bd->lock);
	return rc;
}

static enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer)
{
	struct bmg160_data *bd =
		container_of(timer, struct bmg160_data, hr_timer);

	queue_work(bd->workqueue, &bd->work);
	hrtimer_start(&bd->hr_timer, bd->ktime, HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static ssize_t bmg160_rate_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", bd->data_rate_reg);
}

static ssize_t bmg160_rate_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int rc = 0;
	struct bmg160_data *bd = dev_get_drvdata(dev);
	unsigned long val;

	rc = kstrtoul(buf, 10, &val);
	if (rc)
		return rc;
	if (val >= ARRAY_SIZE(data_rates_us))
		return -EINVAL;
	bd->data_rate_reg = val;
	dev_dbg(&bd->ic_dev->dev, "%s: rate %d ms\n", __func__,
			data_rates_us[bd->data_rate_reg] / 1000);
	return count;
}

static ssize_t bmg160_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", bd->enabled ?
			bd->wm_level : 0);
}

static ssize_t bmg160_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int rc = 0;
	struct bmg160_data *bd = dev_get_drvdata(dev);
	unsigned long val;

	rc = kstrtoul(buf, 10, &val);
	if (rc)
		return rc;
	if (val >= FIFO_MAX)
		return -EINVAL;
	bmg160_set_enable(bd, val);
	dev_dbg(&bd->ic_dev->dev, "%s: WM %d\n", __func__, bd->wm_level);
	return rc ? rc : count;
}

static ssize_t bmg160_chip_id_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", bd->chip_id);
}

static ssize_t bmg160_register_store(struct device *dev,
	     struct device_attribute *attr, const char *buf, size_t count)
{
	int address, value;
	int rc;
	struct bmg160_data *bd = dev_get_drvdata(dev);

	if (2 == sscanf(buf, "%x,%x", &address, &value))
		rc = bmg160_ic_write(bd->ic_dev, address, value);
	else
		rc = -EINVAL;
	return rc ? rc : count;
}

static ssize_t bmg160_register_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	size_t count = 0;
	u8 reg[BMG160_LAST_REG];
	unsigned i;
	int rc = bmg160_ic_read(bd->ic_dev, 0, reg, sizeof(reg));

	if (!rc) {
		for (i = 0; i < sizeof(reg); i++)
			count += scnprintf(buf + count, PAGE_SIZE - count,
				"0x%02x: %3d (%02x)\n", i, reg[i], reg[i]);
	}
	return rc ? rc : count;
}

static ssize_t bmg160_fflush_store(struct device *dev,
	     struct device_attribute *attr, const char *buf, size_t count)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	unsigned long val;
	int rc = kstrtoul(buf, 10, &val);

	if (!rc && val == 1) {
		spin_lock_irqsave(&bd->data_lock, val);
		bd->flush_complete_req = true;
		spin_unlock_irqrestore(&bd->data_lock, val);
		queue_work(bd->workqueue, &bd->work);
		return count;
	}
	return -EINVAL;
}

static struct device_attribute attributes[] = {
	__ATTR(delay, 0644, bmg160_rate_show, bmg160_rate_store),
	__ATTR(chip_id, 0444, bmg160_chip_id_show, NULL),
	__ATTR(enable, 0644, bmg160_enable_show, bmg160_enable_store),
	__ATTR(fflush, 0200, NULL, bmg160_fflush_store),
	__ATTR(reg, 0644, bmg160_register_show, bmg160_register_store),
};

static int add_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto undo;
	return 0;
undo:
	for (i--; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: failed to create sysfs interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

static int bmg160_cdev_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *c = filp->private_data;
	struct bmg160_data *bd = container_of(c, struct bmg160_data, cdev);
	unsigned long flags;
	char *p;

	spin_lock_irqsave(&bd->data_lock, flags);
	if (!bd->out_buf) {
		spin_unlock_irqrestore(&bd->data_lock, flags);
		p = kmalloc(2 * IO_BUF_SIZE, GFP_KERNEL);
		if (!p) {
			dev_err(&bd->ic_dev->dev, "%s: no memory\n", __func__);
			return -ENOMEM;
		}
		spin_lock_irqsave(&bd->data_lock, flags);
		bd->out_buf = p;
		bd->read_buf = p + IO_BUF_SIZE;
		bd->w_idx = 0;
		spin_unlock_irqrestore(&bd->data_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&bd->data_lock, flags);
	return -EBUSY;
}

static int bmg160_cdev_close(struct inode *inode, struct file *filp)
{
	struct miscdevice *c = filp->private_data;
	struct bmg160_data *bd = container_of(c, struct bmg160_data, cdev);
	unsigned long flags;

	spin_lock_irqsave(&bd->data_lock, flags);
	kfree(bd->out_buf);
	bd->out_buf = NULL;
	spin_unlock_irqrestore(&bd->data_lock, flags);
	return 0;
}

static unsigned int bmg160_cdev_poll(struct file *filp,
	struct poll_table_struct *pt)
{
	unsigned int mask;
	unsigned long flags;
	struct miscdevice *c = filp->private_data;
	struct bmg160_data *bd = container_of(c, struct bmg160_data, cdev);

	poll_wait(filp, &bd->wq, pt);
	spin_lock_irqsave(&bd->data_lock, flags);
	if (bd->w_idx)
		mask = POLLIN | POLLRDNORM;
	else
		mask = 0;
	spin_unlock_irqrestore(&bd->data_lock, flags);
	return mask;
}

static enum bmg160_orientation bmg160_get_orientation(const char *name)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(orientation_id); i++)
		if (!strcmp(name, orientation_id[i]))
			return i;
	return -1;
}

static void bmg160_map_axis(struct bmg160_data *bd, struct frame *p)
{
	switch (bd->orientation) {
	case ORI_X_Y_Z:
		p->x = le16_to_cpu(p->x);
		p->y = le16_to_cpu(p->y);
		p->z = le16_to_cpu(p->z);
		break;
	case ORI_Y_NX_Z:
		p->x = le16_to_cpu(p->y);
		p->y = -le16_to_cpu(p->x);
		p->z = le16_to_cpu(p->z);
		break;
	case ORI_NX_NY_Z:
		p->x = -le16_to_cpu(p->x);
		p->y = -le16_to_cpu(p->y);
		p->z = le16_to_cpu(p->z);
		break;
	case ORI_NY_X_Z:
		p->x = -le16_to_cpu(p->y);
		p->y = le16_to_cpu(p->x);
		p->z = le16_to_cpu(p->z);
		break;
	case ORI_Y_X_NZ:
		p->x = le16_to_cpu(p->y);
		p->y = le16_to_cpu(p->x);
		p->z = -le16_to_cpu(p->z);
		break;
	case ORI_X_NY_NZ:
		p->x = le16_to_cpu(p->x);
		p->y = -le16_to_cpu(p->y);
		p->z = -le16_to_cpu(p->z);
		break;
	case ORI_NY_NX_NZ:
		p->x = -le16_to_cpu(p->y);
		p->y = -le16_to_cpu(p->x);
		p->z = -le16_to_cpu(p->z);
		break;
	default:
	case ORI_NX_Y_NZ:
		p->x = -le16_to_cpu(p->x);
		p->y = le16_to_cpu(p->y);
		p->z = -le16_to_cpu(p->z);
		break;
	}
}

static ssize_t bmg160_cdev_read(struct file *filp, char __user *buf,
		size_t size, loff_t *offs)
{
	ssize_t rc;
	int len;
	unsigned long flags;
	struct frame *p;
	unsigned i;
	struct miscdevice *c = filp->private_data;
	struct bmg160_data *bd = container_of(c, struct bmg160_data, cdev);
	struct header *header;

	spin_lock_irqsave(&bd->data_lock, flags);
	while (!bd->w_idx) {
		spin_unlock_irqrestore(&bd->data_lock, flags);

		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(bd->wq, bd->w_idx))
			return -ERESTARTSYS;

		spin_lock_irqsave(&bd->data_lock, flags);
	}

	memcpy(bd->read_buf, bd->out_buf, bd->w_idx);
	rc = len = bd->w_idx;
	bd->w_idx = 0;
	spin_unlock_irqrestore(&bd->data_lock, flags);

	for (header = (struct header *)bd->read_buf; len; ) {

		dev_dbg(&bd->ic_dev->dev, "%s: %d frames\n", __func__,
				header->cnt);

		for (i = 0, p = header->data; i < header->cnt; i++, p++)
			bmg160_map_axis(bd, p);

		len -= sizeof(*header) + header->cnt * sizeof(struct frame);
		header = (struct header *)p;
	}
	if (copy_to_user(buf, bd->read_buf, rc))
		rc = -EFAULT;
	return rc;
}

static const struct file_operations bmg160_fops = {
	.owner = THIS_MODULE,
	.open = bmg160_cdev_open,
	.release = bmg160_cdev_close,
	.read = bmg160_cdev_read,
	.poll = bmg160_cdev_poll,
};

static int bmg160_setup_cdev(struct bmg160_data *bd)
{
	static int dev_index;
	int rc;
	char name[32];

	scnprintf(name, sizeof(name), "bmg160-%d", dev_index);

	bd->cdev.minor = MISC_DYNAMIC_MINOR;
	bd->cdev.name = name;
	bd->cdev.fops = &bmg160_fops;
	rc = misc_register(&bd->cdev);
	if (rc) {
		dev_err(&bd->ic_dev->dev, "%s: misc_registererror %d\n",
				__func__, rc);
	} else {
		dev_info(&bd->ic_dev->dev, "%s: misc_device %s] added\n",
				__func__, name);
		dev_index++;
	}
	return rc;
}

static int bmg160_probe(struct i2c_client *ic_dev,
		const struct i2c_device_id *id)
{
	struct bmg160_data *bd;
	int                 rc;
	struct device *dev = &ic_dev->dev;
	struct device_node *np;

	bd = kzalloc(sizeof(struct bmg160_data), GFP_KERNEL);
	if (!bd) {
		rc = -ENOMEM;
		goto probe_exit;
	}

	if (dev->of_node) {
		const char *regname;
		int rc;
		u32 val;

		np = dev->of_node;
		if (of_property_read_string(np, "regulator", &regname)) {
			dev_info(dev, "Failed to get regulator from OF DT\n");
			goto skip_regulator;
		}
		bd->regulator = regulator_get(NULL, regname);
		if (IS_ERR_OR_NULL(bd->regulator)) {
			dev_err(dev, "regulator_get '%s'failed\n", regname);
			goto probe_err_free;
		}
		rc = regulator_set_voltage(bd->regulator, 2800000, 2800000);
		if (rc) {
			dev_err(dev, "regulator_set_voltage rc=%i\n", rc);
			goto probe_err_reg_put;
		}
skip_regulator:
		if (of_property_read_u32(np, "range", &val))
			bd->range = BMG160_RANGE_2000;
		else
			bd->range = val;

		if (of_property_read_string(np, "orientation", &regname))
			bd->orientation =
				bmg160_get_orientation("ORI_X_Y_Z");
		else
			bd->orientation = bmg160_get_orientation(regname);

	} else {
		bd->range = BMG160_RANGE_2000;
		bd->orientation = bmg160_get_orientation("ORI_X_Y_Z");
	}
	bd->ic_dev = ic_dev;
	INIT_WORK(&bd->work, bmg160_report_data);
	mutex_init(&bd->lock);
	hrtimer_init(&bd->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	bd->hr_timer.function = my_hrtimer_callback;
	init_waitqueue_head(&bd->wq);
	spin_lock_init(&bd->data_lock);
	bd->data_rate_reg = BMG160_UR_BW_100HZ_32HZ;
	i2c_set_clientdata(ic_dev, bd);

	rc = bmg160_power_up(bd);
	if (rc)
		goto probe_err_reg_put;

	rc = bmg160_read_chip_id(bd);
	if (rc) {
		bmg160_power_down(bd);
		goto probe_err_config;
	}

	rc = bmg160_power_down(bd);
	if (rc)
		goto probe_err_config;

	bd->workqueue = create_singlethread_workqueue(dev_name(
			&bd->ic_dev->dev));
	if (!bd->workqueue) {
		dev_err(&ic_dev->dev, "%s, failed to allocate workqueue\n",
				__func__);
		rc = -ENOMEM;
		goto probe_err_config;
	}

	rc = add_sysfs_interfaces(&bd->ic_dev->dev);
	if (rc)
		goto probe_err_reg;

	rc = bmg160_setup_cdev(bd);
	if (rc)
		goto probe_err_cdev;
	return rc;

probe_err_cdev:
	remove_sysfs_interfaces(&bd->ic_dev->dev);
probe_err_reg:
	destroy_workqueue(bd->workqueue);
probe_err_config:
	(void)bmg160_power_down(bd);
probe_err_reg_put:
	if (bd->regulator)
		regulator_put(bd->regulator);
	i2c_set_clientdata(ic_dev, NULL);
probe_err_free:
	kfree(bd);
probe_exit:
	return rc;
}

static int bmg160_remove(struct i2c_client *ic_dev)
{
	struct bmg160_data *bd;

	bd = i2c_get_clientdata(ic_dev);

	bmg160_set_enable(bd, false);
	misc_deregister(&bd->cdev);
	remove_sysfs_interfaces(&bd->ic_dev->dev);
	destroy_workqueue(bd->workqueue);
	i2c_set_clientdata(ic_dev, NULL);
	if (bd->regulator) {
		regulator_disable(bd->regulator);
		regulator_put(bd->regulator);
	}
	kfree(bd->out_buf);
	kfree(bd);
	return 0;
}

static int bmg160_resume(struct device *dev)
{
	return 0;
}

static int bmg160_suspend(struct device *dev)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	bmg160_set_enable(bd, false);
	return 0;
}

static const struct dev_pm_ops bmg160_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(bmg160_suspend, bmg160_resume)
};

static const struct i2c_device_id bmg160_i2c_id[] = {
	{BMG160_NAME, 0},
	{}
};

static const struct of_device_id bmg160_of_match[] = {
	{.compatible = "bmg160",},
	{},
};

static struct i2c_driver bmg160_driver = {
	.driver = {
		.name  = BMG160_NAME,
		.owner = THIS_MODULE,
		.pm    = &bmg160_pm_ops,
		.of_match_table = bmg160_of_match,
	},
	.probe         = bmg160_probe,
	.remove        = bmg160_remove,
	.id_table      = bmg160_i2c_id,
};

static int __init bmg160_init(void)
{
	return i2c_add_driver(&bmg160_driver);
}
module_init(bmg160_init);

static void __exit bmg160_exit(void)
{
	i2c_del_driver(&bmg160_driver);
}
module_exit(bmg160_exit);
