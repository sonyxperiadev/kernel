/*
 * smb380.c - SMB380 Tri-axis accelerometer driver
 *
 * Copyright (C) 2010 Samsung Eletronics Co.Ltd
 * Kim Kyuwon <q1.kim@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Donggeun Kim <dg77.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#define DEBUG

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/smb380.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/rdb/brcm_rdb_chipreg.h>


#define SMB380_CHIP_ID_REG	0x00
#define SMB380_X_LSB_REG	0x02
#define SMB380_X_MSB_REG	0x03
#define SMB380_Y_LSB_REG	0x04
#define SMB380_Y_MSB_REG	0x05
#define SMB380_Z_LSB_REG	0x06
#define SMB380_Z_MSB_REG	0x07
#define SMB380_TEMP_REG		0x08
#define SMB380_CTRL1_REG	0x0a
#define SMB380_CTRL2_REG	0x0b
#define SMB380_SETTINGS1_REG	0x0c
#define SMB380_SETTINGS2_REG	0x0d
#define SMB380_SETTINGS3_REG	0x0e
#define SMB380_SETTINGS4_REG	0x0f
#define SMB380_SETTINGS5_REG	0x10
#define SMB380_SETTINGS6_REG	0x11
#define SMB380_RANGE_BW_REG	0x14
#define SMB380_CONF2_REG	0x15

#define SMB380_CHIP_ID		0x2

#define SMB380_ENABLE_ADV_INT_SHIFT	6
#define SMB380_ENABLE_ADV_INT_MASK	(0x1 << 6)

#define SMB380_NEW_DATA_INT_SHIFT	5
#define SMB380_NEW_DATA_INT_MASK	(0x1 << 5)

#define SMB380_RANGE_SHIFT		3
#define SMB380_RANGE_MASK		(0x3 << 3)
#define SMB380_BANDWIDTH_SHIFT		0
#define SMB380_BANDWIDTH_MASK		(0x7)

#define SMB380_ANY_MOTION_DUR_SHIFT		6
#define SMB380_ANY_MOTION_DUR_MASK		(0x3 << 6)
#define SMB380_HG_HYST_SHIFT		3
#define SMB380_HG_HYST_MASK		(0x7 << 3)
#define SMB380_LG_HYST_SHIFT		0
#define SMB380_LG_HYST_MASK		(0x7)

#define SMB380_ANY_MOTION_THRES_SHIFT		0
#define SMB380_ANY_MOTION_THRES_MASK		(0xFF << 0)

#define SMB380_HG_DUR_SHIFT		(0x0)
#define SMB380_HG_DUR_MASK		(0xff)
#define SMB380_HG_THRES_SHIFT		(0x0)
#define SMB380_HG_THRES_MASK		(0xff)
#define SMB380_LG_DUR_SHIFT		(0x0)
#define SMB380_LG_DUR_MASK		(0xff)
#define SMB380_LG_THRES_SHIFT		(0x0)
#define SMB380_LG_THRES_MASK		(0xff)

#define SMB380_ENABLE_ANY_MOTION_SHIFT		6
#define SMB380_ENABLE_ANY_MOTION_MASK		(0x1 << 6)
#define SMB380_ENABLE_HG_SHIFT		1
#define SMB380_ENABLE_HG_MASK		(0x1 << 1)
#define SMB380_ENABLE_LG_SHIFT		0
#define SMB380_ENABLE_LG_MASK		(0x1)

#define SMB380_SLEEP_SHIFT		0
#define SMB380_SLEEP_MASK		(0x1)

#define SMB380_ACCEL_BITS		10
#define SMB380_MAX_VALUE		((1 << ((SMB380_ACCEL_BITS) - 1)) - 1)
#define SMB380_MIN_VALUE		(-(1 << ((SMB380_ACCEL_BITS) - 1)))

#define SMB380_DEFAULT_RANGE		  RANGE_2G
#define SMB380_DEFAULT_BANDWIDTH	  BW_25HZ
#define SMB380_DEFAULT_ENABLE_ADV_INT	0
#define SMB380_DEFAULT_NEW_DATA_INT	  0
#define SMB380_DEFAULT_ANY_MOTION_INT 0
#define SMB380_DEFAULT_HG_INT		  1
#define SMB380_DEFAULT_LG_INT		  1
#define SMB380_DEFAULT_HG_DURATION	  0x96
#define SMB380_DEFAULT_HG_THRESHOLD	  0xa0
#define SMB380_DEFAULT_HG_HYST		  0
#define SMB380_DEFAULT_ANY_MOTION_DUR 0
#define SMB380_DEFAULT_ANY_MOTION_THRES 0
#define SMB380_DEFAULT_LG_DURATION	  0x96
#define SMB380_DEFAULT_LG_THRESHOLD	  0x14
#define SMB380_DEFAULT_LG_HYST		  0

struct smb380_data {
	s16 x;
	s16 y;
	s16 z;
	u8 temp;
};

struct smb380_sensor {
	struct i2c_client	*client;
	struct device		*dev;
	struct input_dev	*idev;
	struct work_struct	work;
	struct mutex		lock;

	struct smb380_data data;
	enum scale_range range;
	enum filter_bw bandwidth;
	u8 enable_adv_int;
	u8 new_data_int;
	u8 hg_int;
	u8 lg_int;
	u8 lg_dur;
	u8 lg_thres;
	u8 lg_hyst;
	u8 hg_dur;
	u8 hg_thres;
	u8 hg_hyst;
	u8 any_motion_dur;
	u8 any_motion_thres;
	u8 any_motion_int;
};

static int smb380_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;

	/*
	 * According to the datasheet, the interrupt should be deactivated
	 * on the host side when write sequences operate
	 */
	disable_irq_nosync(client->irq);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	enable_irq(client->irq);

	if (ret < 0)
		dev_err(&client->dev, "%s: reg 0x%x, val 0x%x, err %d\n",
			__func__, reg, val, ret);
	return ret;
}

static int smb380_read_reg(struct i2c_client *client, u8 reg)
{
	int ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
		dev_err(&client->dev, "%s: reg 0x%x, err %d\n",
			__func__, reg, ret);
	return ret;
}

static int smb380_xyz_read_reg(struct i2c_client *client,
			       u8 *buffer, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = buffer,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buffer,
		},
	};
	return i2c_transfer(client->adapter, msg, 2);
}

static int smb380_set_reg_bits(struct i2c_client *client,
					int val, int shift, u8 mask, u8 reg)
{
	u8 data = smb380_read_reg(client, reg);

	data = (data & ~mask) | ((val << shift) & mask);
	return smb380_write_reg(client, reg, data);
}

static u8 smb380_get_reg_bits(struct i2c_client *client, int shift,
					u8 mask, u8 reg)
{
	u8 data = smb380_read_reg(client, reg);

	data = (data & mask) >> shift;
	return data;
}

/* range */
static int smb380_set_range(struct i2c_client *client, enum scale_range range)
{
	return smb380_set_reg_bits(client, range, SMB380_RANGE_SHIFT,
				   SMB380_RANGE_MASK, SMB380_RANGE_BW_REG);
}

static u8 smb380_get_range(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_RANGE_SHIFT,
				   SMB380_RANGE_MASK, SMB380_RANGE_BW_REG);
}

/* bandwidth */
static int smb380_set_bandwidth(struct i2c_client *client, enum filter_bw bw)
{
	return smb380_set_reg_bits(client, bw, SMB380_BANDWIDTH_SHIFT,
				   SMB380_BANDWIDTH_MASK, SMB380_RANGE_BW_REG);
}

static u8 smb380_get_bandwidth(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_BANDWIDTH_SHIFT,
				   SMB380_BANDWIDTH_MASK, SMB380_RANGE_BW_REG);
}

/* enable_adv_int interrupt */
static int smb380_set_enable_adv_int(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val, SMB380_ENABLE_ADV_INT_SHIFT,
				   SMB380_ENABLE_ADV_INT_MASK, SMB380_CONF2_REG);
}

static u8 smb380_get_enable_adv_int(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ENABLE_ADV_INT_SHIFT,
				   SMB380_ENABLE_ADV_INT_MASK, SMB380_CONF2_REG);
}

/* new data interrupt */
static int smb380_set_new_data_int(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val, SMB380_NEW_DATA_INT_SHIFT,
				   SMB380_NEW_DATA_INT_MASK, SMB380_CONF2_REG);
}

static u8 smb380_get_new_data_int(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_NEW_DATA_INT_SHIFT,
				   SMB380_NEW_DATA_INT_MASK, SMB380_CONF2_REG);
}

/* any_motion interrupt */
static int smb380_set_any_motion_int(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val, SMB380_ENABLE_ANY_MOTION_SHIFT,
				   SMB380_ENABLE_ANY_MOTION_MASK, SMB380_CTRL2_REG);
}

static u8 smb380_get_any_motion_int(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ENABLE_ANY_MOTION_SHIFT,
				   SMB380_ENABLE_ANY_MOTION_MASK, SMB380_CTRL2_REG);
}

/* high-g interrupt */
static int smb380_set_hg_int(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val, SMB380_ENABLE_HG_SHIFT,
				   SMB380_ENABLE_HG_MASK, SMB380_CTRL2_REG);
}

static u8 smb380_get_hg_int(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ENABLE_HG_SHIFT,
				   SMB380_ENABLE_HG_MASK, SMB380_CTRL2_REG);
}

/* low-g interrupt */
static int smb380_set_lg_int(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val,	SMB380_ENABLE_LG_SHIFT,
				   SMB380_ENABLE_LG_MASK, SMB380_CTRL2_REG);
}

static u8 smb380_get_lg_int(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ENABLE_LG_SHIFT,
				   SMB380_ENABLE_LG_MASK, SMB380_CTRL2_REG);
}

/* low-g interrupt criterion for duration */
static int smb380_set_lg_dur(struct i2c_client *client, u8 dur)
{
	return smb380_set_reg_bits(client, dur,	SMB380_LG_DUR_SHIFT,
				   SMB380_LG_DUR_MASK, SMB380_SETTINGS2_REG);
}

static u8 smb380_get_lg_dur(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_LG_DUR_SHIFT,
				   SMB380_LG_DUR_MASK, SMB380_SETTINGS2_REG);
}

/* low-g interrupt criterion for threshold */
static int smb380_set_lg_thres(struct i2c_client *client, u8 thres)
{
	return smb380_set_reg_bits(client, thres, SMB380_LG_THRES_SHIFT,
				   SMB380_LG_THRES_MASK, SMB380_SETTINGS1_REG);
}

static u8 smb380_get_lg_thres(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_LG_THRES_SHIFT,
				   SMB380_LG_THRES_MASK, SMB380_SETTINGS1_REG);
}

/* low-g interrupt criterion for hysteresis */
static int smb380_set_lg_hyst(struct i2c_client *client, u8 hyst)
{
	return smb380_set_reg_bits(client, hyst, SMB380_LG_HYST_SHIFT,
				   SMB380_LG_HYST_MASK, SMB380_SETTINGS6_REG);
}

static u8 smb380_get_lg_hyst(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_LG_HYST_SHIFT,
				   SMB380_LG_HYST_MASK,	SMB380_SETTINGS6_REG);
}

/* high-g interrupt criterion for duration */
static int smb380_set_hg_dur(struct i2c_client *client, u8 dur)
{
	return smb380_set_reg_bits(client, dur,	SMB380_HG_DUR_SHIFT,
				   SMB380_HG_DUR_MASK, SMB380_SETTINGS4_REG);
}

static u8 smb380_get_hg_dur(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_HG_DUR_SHIFT,
				   SMB380_HG_DUR_MASK, SMB380_SETTINGS4_REG);
}

/* high-g interrupt criterion for threshold */
static int smb380_set_hg_thres(struct i2c_client *client, u8 thres)
{
	return smb380_set_reg_bits(client, thres, SMB380_HG_THRES_SHIFT,
			   SMB380_HG_THRES_MASK, SMB380_SETTINGS3_REG);
}

static u8 smb380_get_hg_thres(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_HG_THRES_SHIFT,
				   SMB380_HG_THRES_MASK, SMB380_SETTINGS3_REG);
}

/* any_motion_threshold value */
static int smb380_set_any_motion_thres(struct i2c_client *client, u8 any_motion_thres)
{
	return smb380_set_reg_bits(client, any_motion_thres, SMB380_ANY_MOTION_THRES_SHIFT,
				   SMB380_ANY_MOTION_THRES_MASK,	SMB380_SETTINGS5_REG);
}

static u8 smb380_get_any_motion_thres(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ANY_MOTION_THRES_SHIFT,
				   SMB380_ANY_MOTION_THRES_MASK,	SMB380_SETTINGS5_REG);
}


/* any_motion_dur criterion */
static int smb380_set_any_motion_dur(struct i2c_client *client, u8 any_motion_dur)
{
	return smb380_set_reg_bits(client, any_motion_dur, SMB380_ANY_MOTION_DUR_SHIFT,
				   SMB380_ANY_MOTION_DUR_MASK,	SMB380_SETTINGS6_REG);
}

static u8 smb380_get_any_motion_dur(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_ANY_MOTION_DUR_SHIFT,
				   SMB380_ANY_MOTION_DUR_MASK,	SMB380_SETTINGS6_REG);
}

/* high-g interrupt criterion for hysteresis */
static int smb380_set_hg_hyst(struct i2c_client *client, u8 hyst)
{
	return smb380_set_reg_bits(client, hyst, SMB380_HG_HYST_SHIFT,
				   SMB380_HG_HYST_MASK,	SMB380_SETTINGS6_REG);
}

static u8 smb380_get_hg_hyst(struct i2c_client *client)
{
	return smb380_get_reg_bits(client, SMB380_HG_HYST_SHIFT,
				   SMB380_HG_HYST_MASK,	SMB380_SETTINGS6_REG);
}

static int smb380_set_sleep(struct i2c_client *client, u8 val)
{
	return smb380_set_reg_bits(client, val,	SMB380_SLEEP_SHIFT,
				   SMB380_SLEEP_MASK, SMB380_CTRL1_REG);
}

/*
 * The description of the digital signals x, y and z is "2' complement".
 * So we need to correct the sign of data read by i2c.
 */
static void smb380_correct_accel_sign(s16 *val)
{
	*val <<= (sizeof(s16) * BITS_PER_BYTE - SMB380_ACCEL_BITS);
	*val >>= (sizeof(s16) * BITS_PER_BYTE - SMB380_ACCEL_BITS);
}

static void smb380_merge_register_values(struct i2c_client *client, s16 *val,
					 u8 lsb, u8 msb)
{
	*val = (msb << 2) | (lsb >> 6);
	smb380_correct_accel_sign(val);
}

/*
 * Read 8 bit temperature.
 * An output of 0 equals -30C, 1 LSB equals 0.5C
 */
static void smb380_read_temperature(struct i2c_client *client, u8 *temper)
{
	*temper = smb380_read_reg(client, SMB380_TEMP_REG);

	dev_dbg(&client->dev, "%s: %d\n", __func__, *temper);
}

static void smb380_read_xyz(struct i2c_client *client,
			    s16 *x, s16 *y, s16 *z)
{
	u8 buffer[6];
	buffer[0] = SMB380_X_LSB_REG;
	smb380_xyz_read_reg(client, buffer, 6);

	smb380_merge_register_values(client, x, buffer[0], buffer[1]);
	smb380_merge_register_values(client, y, buffer[2], buffer[3]);
	smb380_merge_register_values(client, z, buffer[4], buffer[5]);

	dev_dbg(&client->dev, "%s: x %d, y %d, z %d\n", __func__, *x, *y, *z);
}

static ssize_t smb380_show_xyz(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct smb380_sensor *sensor = dev_get_drvdata(dev);

	mutex_lock(&sensor->lock);
	smb380_read_xyz(sensor->client,
		&sensor->data.x, &sensor->data.y, &sensor->data.z);
	mutex_unlock(&sensor->lock);

	return sprintf(buf, "%d, %d, %d\n",
		sensor->data.x, sensor->data.y, sensor->data.z);
}
static DEVICE_ATTR(xyz, S_IRUGO, smb380_show_xyz, NULL);

static ssize_t smb380_show_temper(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct smb380_sensor *sensor = dev_get_drvdata(dev);

	mutex_lock(&sensor->lock);
	smb380_read_temperature(sensor->client, &sensor->data.temp);
	mutex_unlock(&sensor->lock);
	return sprintf(buf, "%d\n", sensor->data.temp);
}
static DEVICE_ATTR(temperature, S_IRUGO, smb380_show_temper, NULL);

#define SMB380_ADJUST(name) \
static ssize_t smb380_show_##name(struct device *dev, \
		struct device_attribute *att, char *buf) \
{ \
	struct smb380_sensor *sensor = dev_get_drvdata(dev); \
 \
	return sprintf(buf, "%d\n", sensor->name); \
} \
static ssize_t smb380_store_##name(struct device *dev, \
		struct device_attribute *attr, const char *buf, size_t count) \
{ \
	struct smb380_sensor *sensor = dev_get_drvdata(dev); \
	unsigned long val; \
	int ret; \
	u8 result; \
 \
	ret = strict_strtoul(buf, 10, &val); \
	if (!ret) { \
		smb380_set_##name(sensor->client, val); \
		result = smb380_get_##name(sensor->client); \
		mutex_lock(&sensor->lock); \
		sensor->name = result; \
		mutex_unlock(&sensor->lock); \
		return count; \
	} \
	else \
		return ret; \
} \
static DEVICE_ATTR(name, S_IRUGO | S_IWUSR, \
		smb380_show_##name, smb380_store_##name);

SMB380_ADJUST(range);
SMB380_ADJUST(bandwidth);
SMB380_ADJUST(enable_adv_int);
SMB380_ADJUST(new_data_int);
SMB380_ADJUST(hg_int);
SMB380_ADJUST(any_motion_int);
SMB380_ADJUST(lg_int);
SMB380_ADJUST(lg_dur);
SMB380_ADJUST(lg_thres);
SMB380_ADJUST(lg_hyst);
SMB380_ADJUST(hg_dur);
SMB380_ADJUST(hg_thres);
SMB380_ADJUST(hg_hyst);
SMB380_ADJUST(any_motion_dur);
SMB380_ADJUST(any_motion_thres);

static struct attribute *smb380_attributes[] = {
	&dev_attr_xyz.attr,
	&dev_attr_temperature.attr,
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_enable_adv_int.attr,
	&dev_attr_new_data_int.attr,
	&dev_attr_hg_int.attr,
	&dev_attr_any_motion_int.attr,
	&dev_attr_lg_int.attr,
	&dev_attr_lg_dur.attr,
	&dev_attr_lg_thres.attr,
	&dev_attr_lg_hyst.attr,
	&dev_attr_hg_dur.attr,
	&dev_attr_hg_thres.attr,
	&dev_attr_hg_hyst.attr,
	&dev_attr_any_motion_dur.attr,
	&dev_attr_any_motion_thres.attr,
	NULL
};

static const struct attribute_group smb380_group = {
	.attrs	= smb380_attributes,
};

static void smb380_work(struct work_struct *work)
{
	struct smb380_sensor *sensor =
			container_of(work, struct smb380_sensor, work);

	smb380_read_xyz(sensor->client,
		&sensor->data.x, &sensor->data.y, &sensor->data.z);
#if 1
	smb380_read_temperature(sensor->client, &sensor->data.temp);
#endif
	mutex_lock(&sensor->lock);
	input_report_abs(sensor->idev, ABS_X, sensor->data.x);
	input_report_abs(sensor->idev, ABS_Y, sensor->data.y);
	input_report_abs(sensor->idev, ABS_Z, sensor->data.z);
	input_sync(sensor->idev);
	mutex_unlock(&sensor->lock);

	enable_irq(sensor->client->irq);
}

static irqreturn_t smb380_irq(int irq, void *dev_id)
{
	struct smb380_sensor *sensor = dev_id;

	if (!work_pending(&sensor->work)) {
		disable_irq_nosync(irq);
		schedule_work(&sensor->work);
	}

	return IRQ_HANDLED;
}

static void smb380_initialize(struct smb380_sensor *sensor)
{
	smb380_set_range(sensor->client, sensor->range);
	smb380_set_bandwidth(sensor->client, sensor->bandwidth);
	smb380_set_enable_adv_int(sensor->client, sensor->enable_adv_int);
	smb380_set_new_data_int(sensor->client, sensor->new_data_int);
	smb380_set_hg_dur(sensor->client, sensor->hg_dur);
	smb380_set_hg_thres(sensor->client, sensor->hg_thres);
	smb380_set_hg_hyst(sensor->client, sensor->hg_hyst);
	smb380_set_any_motion_dur(sensor->client, sensor->any_motion_dur);
	smb380_set_any_motion_thres(sensor->client, sensor->any_motion_thres);
	smb380_set_lg_dur(sensor->client, sensor->lg_dur);
	smb380_set_lg_thres(sensor->client, sensor->lg_thres);
	smb380_set_lg_hyst(sensor->client, sensor->lg_hyst);
	smb380_set_any_motion_int(sensor->client, sensor->any_motion_int);
	smb380_set_hg_int(sensor->client, sensor->hg_int);
	smb380_set_lg_int(sensor->client, sensor->lg_int);
}

static void smb380_unregister_input_device(struct smb380_sensor *sensor)
{
	struct i2c_client *client = sensor->client;

	if (client->irq > 0)
		free_irq(client->irq, sensor);

	input_unregister_device(sensor->idev);
	sensor->idev = NULL;
}

static int smb380_register_input_device(struct smb380_sensor *sensor)
{
	struct i2c_client *client = sensor->client;
	struct input_dev *idev;
	int ret;

	idev = sensor->idev = input_allocate_device();
	if (!idev) {
		dev_err(&client->dev, "failed to allocate input device\n");
		ret = -ENOMEM;
		goto failed_alloc;
	}

	idev->name = "SMB380 Sensor";
	idev->id.bustype = BUS_I2C;
	idev->dev.parent = &client->dev;
	idev->evbit[0] = BIT_MASK(EV_ABS);

	input_set_abs_params(idev, ABS_X, SMB380_MIN_VALUE,
			SMB380_MAX_VALUE, 0, 0);
	input_set_abs_params(idev, ABS_Y, SMB380_MIN_VALUE,
			SMB380_MAX_VALUE, 0, 0);
	input_set_abs_params(idev, ABS_Z, SMB380_MIN_VALUE,
			SMB380_MAX_VALUE, 0, 0);

	input_set_drvdata(idev, sensor);

	ret = input_register_device(idev);
	if (ret) {
		dev_err(&client->dev, "failed to register input device\n");
		goto failed_reg;
	}

	/* Hack:
	 * Since an unknown source is changing this pin mux after the Uboot, 
	 * without knowing where it came from, we will set up the right pin mux 
	 * right before the usage.
	 */
	writel(0x303, KONA_CHIPREG_VA + CHIPREG_ULPI1_DATA_6_OFFSET);

	if (client->irq > 0) {

		/* 
		 * Setup GPIO properties for interrupt from sensor.
		 */
		ret = set_irq_type(client->irq, IRQ_TYPE_EDGE_FALLING);
		if (ret < 0) {
			printk(KERN_ERR "set_irq_type failed with irq %d\n", client->irq);
			goto failed_reg;
		}
		ret = gpio_request(irq_to_gpio(client->irq), "bma150 sensor int");
		if (ret < 0) {
			printk(KERN_ERR "unable to request GPIO pin %d\n", irq_to_gpio(client->irq));
			goto failed_reg;
		}
		gpio_direction_input(irq_to_gpio(client->irq));

		ret = request_irq(client->irq, smb380_irq, IRQF_TRIGGER_FALLING,
				"smb380 accelerometer", sensor);
		if (ret) {
			dev_err(&client->dev, "can't get IRQ %d, ret %d\n",
					client->irq, ret);
			goto failed_irq;
		}
	}

	return 0;

failed_irq:
	input_unregister_device(idev);
	idev = NULL;
failed_reg:
	if (idev)
		input_free_device(idev);
failed_alloc:
	return ret;
}

static int __devinit smb380_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct smb380_sensor *sensor;
	struct smb380_platform_data *pdata;
	int ret;

	sensor = kzalloc(sizeof(struct smb380_sensor), GFP_KERNEL);
	if (!sensor) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	pdata = client->dev.platform_data;

	sensor->client = client;
	i2c_set_clientdata(client, sensor);

	ret = smb380_read_reg(client, SMB380_CHIP_ID_REG);
	if (ret < 0) {
		dev_err(&client->dev, "failed to detect device\n");
		goto failed_free;
	}
	if (ret != SMB380_CHIP_ID) {
		dev_err(&client->dev, "unsupported chip id\n");
		goto failed_free;
	}

	INIT_WORK(&sensor->work, smb380_work);
	mutex_init(&sensor->lock);

	ret = sysfs_create_group(&client->dev.kobj, &smb380_group);
	if (ret) {
		dev_err(&client->dev, "failed to create attribute group\n");
		goto failed_free;
	}

	ret = smb380_register_input_device(sensor);
	if (ret)
		goto failed_remove_sysfs;

	if (pdata) {
		sensor->range = pdata->range;
		sensor->bandwidth = pdata->bandwidth;
		sensor->enable_adv_int = pdata->enable_adv_int;
		sensor->new_data_int = pdata->new_data_int;
		sensor->any_motion_int = pdata->any_motion_int;
		sensor->hg_int = pdata->hg_int;
		sensor->lg_int = pdata->lg_int;
		sensor->hg_dur = pdata->hg_dur;
		sensor->hg_thres = pdata->hg_thres;
		sensor->hg_hyst = pdata->hg_hyst;
		sensor->any_motion_dur = pdata->any_motion_dur;
		sensor->any_motion_thres = pdata->any_motion_thres;
		sensor->lg_dur = pdata->lg_dur;
		sensor->lg_thres = pdata->lg_thres;
		sensor->lg_hyst = pdata->lg_hyst;
	} else {
		sensor->range = SMB380_DEFAULT_RANGE;
		sensor->bandwidth = SMB380_DEFAULT_BANDWIDTH;
		sensor->enable_adv_int = SMB380_DEFAULT_ENABLE_ADV_INT;
		sensor->new_data_int = SMB380_DEFAULT_NEW_DATA_INT;
		sensor->any_motion_int = SMB380_DEFAULT_ANY_MOTION_INT;
		sensor->hg_int = SMB380_DEFAULT_HG_INT;
		sensor->lg_int = SMB380_DEFAULT_LG_INT;
		sensor->hg_dur = SMB380_DEFAULT_HG_DURATION;
		sensor->hg_thres = SMB380_DEFAULT_HG_THRESHOLD;
		sensor->hg_hyst = SMB380_DEFAULT_HG_HYST;
		sensor->any_motion_dur = SMB380_DEFAULT_ANY_MOTION_DUR;
		sensor->any_motion_thres = SMB380_DEFAULT_ANY_MOTION_THRES;
		sensor->lg_dur = SMB380_DEFAULT_LG_DURATION;
		sensor->lg_thres = SMB380_DEFAULT_LG_THRESHOLD;
		sensor->lg_hyst = SMB380_DEFAULT_LG_HYST;
	}

	smb380_initialize(sensor);

	dev_info(&client->dev, "%s registered\n", id->name);
	return 0;

failed_remove_sysfs:
	sysfs_remove_group(&client->dev.kobj, &smb380_group);
failed_free:
	kfree(sensor);
	return ret;
}

static int __devexit smb380_remove(struct i2c_client *client)
{
	struct smb380_sensor *sensor = i2c_get_clientdata(client);

	smb380_unregister_input_device(sensor);
	sysfs_remove_group(&client->dev.kobj, &smb380_group);
	kfree(sensor);
	return 0;
}

#ifdef CONFIG_PM
static int smb380_suspend(struct i2c_client *client, pm_message_t mesg)
{
	smb380_set_sleep(client, 1);
	return 0;
}

static int smb380_resume(struct i2c_client *client)
{
	smb380_set_sleep(client, 0);
	return 0;
}

#else
#define smb380_suspend NULL
#define smb380_resume NULL
#endif

static const struct i2c_device_id smb380_ids[] = {
	{ "smb380", 0 },
	{ "bma023", 1 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, smb380_ids);

static struct i2c_driver smb380_i2c_driver = {
	.driver	= {
		.name	= "smb380",
	},
	.probe		= smb380_probe,
	.remove		= __devexit_p(smb380_remove),
	.suspend	= smb380_suspend,
	.resume		= smb380_resume,
	.id_table	= smb380_ids,
};

static int __init smb380_init(void)
{
	return i2c_add_driver(&smb380_i2c_driver);
}
module_init(smb380_init);

static void __exit smb380_exit(void)
{
	i2c_del_driver(&smb380_i2c_driver);
}
module_exit(smb380_exit);

MODULE_AUTHOR("Kim Kyuwon <q1.kim@samsung.com>");
MODULE_DESCRIPTION("SMB380/BMA023 Tri-axis accelerometer driver");
MODULE_LICENSE("GPL");
