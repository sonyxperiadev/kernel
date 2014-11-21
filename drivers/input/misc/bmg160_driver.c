/*
 * Last modified: Jul 27th, 2012
 * Revision: V1.4
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2012 Bosch Sensortec GmbH
 * Copyright(C) 2013 Foxconn International Holdings, Ltd. All rights reserved.
 * All Rights Reserved
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/bmg160.h>
#include <linux/bs_log.h>
#include <linux/regulator/consumer.h>	/* PERI-FG-REGULATOR_SET-00+ */

/* sensor specific */
#define SENSOR_NAME "bmg160"

#define SENSOR_CHIP_ID_BMG (0x0f)

#define BMG_REG_NAME(name) BMG160_##name
#define BMG_VAL_NAME(name) BMG160_##name
#define BMG_CALL_API(name) bmg160_##name

#define BMG_I2C_WRITE_DELAY_TIME 1

/* generic */
#define BMG_MAX_RETRY_I2C_XFER (10)//(100)
#define BMG_MAX_RETRY_WAKEUP (5)
#define BMG_MAX_RETRY_WAIT_DRDY (100)

#define BMG_DELAY_MIN (1)
#define BMG_DELAY_DEFAULT (200)

#define MAG_VALUE_MAX (32767)
#define MAG_VALUE_MIN (-32768)

#define BYTES_PER_LINE (16)

#define BMG_SELF_TEST 0

#ifdef BMG_USE_FIFO
#define MAX_FIFO_F_LEVEL 100
#define MAX_FIFO_F_BYTES 8
#endif


struct op_mode_map {
	char *op_mode_name;
	long op_mode;
};

static const struct op_mode_map op_mode_maps[] = {
	{"normal", BMG_VAL_NAME(MODE_NORMAL)},
	{"deepsuspend", BMG_VAL_NAME(MODE_DEEPSUSPEND)},
	{"suspend", BMG_VAL_NAME(MODE_SUSPEND)},
	{"fastpowerup", BMG_VAL_NAME(MODE_FASTPOWERUP)},
	{"advancedpowersav", BMG_VAL_NAME(MODE_ADVANCEDPOWERSAVING)},
};

struct bmg_client_data {
	struct bmg160_t device;
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_handler;
#endif

	atomic_t delay;

	struct bmg160_data_t value;
	u8 enable:1;

	int layout;
	/* controls not only reg, but also workqueue */
	struct mutex mutex_op_mode;
	struct mutex mutex_enable;
	struct mutex mutex_value;
	struct mutex bmg160_io_ctrl_mutex;
	/* PERI-FG-REGULATOR_SET-00+[ */
	struct regulator *reg;
	unsigned int lpm_uA;
	unsigned int hpm_uA;
	/* PERI-FG-REGULATOR_SET-00+] */
};

static struct i2c_client *bmg_client;
/* i2c operation for API */
static void bmg_i2c_delay(BMG160_S32 msec);
static char bmg_i2c_read(struct i2c_client *client, u8 reg_addr,
		u8 *data, u8 len);
static char bmg_i2c_write(struct i2c_client *client, u8 reg_addr,
		u8 *data, u8 len);

static void bmg_dump_reg(struct i2c_client *client);
static int bmg_check_chip_id(struct i2c_client *client);

#ifdef CONFIG_HAS_EARLYSUSPEND
static int bmg_pre_suspend(struct i2c_client *client);
static int bmg_post_resume(struct i2c_client *client);

static void bmg_early_suspend(struct early_suspend *handler);
static void bmg_late_resume(struct early_suspend *handler);
#endif

static int bmg_check_chip_id(struct i2c_client *client)
{
	int err = 0;
	u8 chip_id = 0;

	bmg_i2c_read(client, BMG_REG_NAME(CHIP_ID_ADDR), &chip_id, 1);
	PINFO("read chip id result: %#x", chip_id);

	if ((chip_id & 0xff) != SENSOR_CHIP_ID_BMG)
		err = -1;

	return err;
}

static void bmg_i2c_delay(BMG160_S32 msec)
{
	mdelay(msec);
}

static void bmg_dump_reg(struct i2c_client *client)
{
	int i;
	u8 dbg_buf[64];
	u8 dbg_buf_str[64 * 3 + 1] = "";

	for (i = 0; i < BYTES_PER_LINE; i++) {
		dbg_buf[i] = i;
		sprintf(dbg_buf_str + i * 3, "%02x%c",
				dbg_buf[i],
				(((i + 1) % BYTES_PER_LINE == 0) ? '\n' : ' '));
	}
	PDEBUG("%s\n", dbg_buf_str);

	bmg_i2c_read(client, BMG_REG_NAME(CHIP_ID_ADDR), dbg_buf, 64);
	for (i = 0; i < 64; i++) {
		sprintf(dbg_buf_str + i * 3, "%02x%c",
				dbg_buf[i],
				(((i + 1) % BYTES_PER_LINE == 0) ? '\n' : ' '));
	}
	PDEBUG("%s\n", dbg_buf_str);
}

/*	i2c read routine for API*/
static char bmg_i2c_read(struct i2c_client *client, u8 reg_addr,
		u8 *data, u8 len)
{

	int retry;
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)i2c_get_clientdata(client);
	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = &reg_addr,
		},

		{
		 .addr = client->addr,
		 .flags = I2C_M_RD,
		 .len = len,
		 .buf = data,
		 },
	};
	mutex_lock(&client_data->bmg160_io_ctrl_mutex);
	for (retry = 0; retry < BMG_MAX_RETRY_I2C_XFER; retry++) {
		if (i2c_transfer(client->adapter, msg, 2) ==2)
			break;
		else
			mdelay(BMG_I2C_WRITE_DELAY_TIME);
	}

	if (BMG_MAX_RETRY_I2C_XFER <= retry) {
		PERR("I2C xfer error");
		mutex_unlock(&client_data->bmg160_io_ctrl_mutex);
		return -EIO;
	}
	mutex_unlock(&client_data->bmg160_io_ctrl_mutex);
	return 0;

}

#ifdef BMG_USE_FIFO
static char bmg_i2c_burst_read(struct i2c_client *client, u8 reg_addr,
		u8 *data, u16 len)
{
	int retry;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = &reg_addr,
		},

		{
		 .addr = client->addr,
		 .flags = I2C_M_RD,
		 .len = len,
		 .buf = data,
		 },
	};

	for (retry = 0; retry < BMG_MAX_RETRY_I2C_XFER; retry++) {
		if (i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg)) > 0)
			break;
		else
			mdelay(BMG_I2C_WRITE_DELAY_TIME);
	}

	if (BMG_MAX_RETRY_I2C_XFER <= retry) {
		PERR("I2C xfer error");
		return -EIO;
	}

	return 0;
}
#endif

/*	i2c write routine for */
static char bmg_i2c_write(struct i2c_client *client, u8 reg_addr,
		u8 *data, u8 len)
{
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)i2c_get_clientdata(client);
	u8 buffer[2];
	int retry;
	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = 2,
		 .buf = buffer,
		 },
	};
	mutex_lock(&client_data->bmg160_io_ctrl_mutex);
	while (0 != len--) {
		buffer[0] = reg_addr;
		buffer[1] = *data;
		for (retry = 0; retry < BMG_MAX_RETRY_I2C_XFER; retry++) {
			if (i2c_transfer(client->adapter, msg,
						ARRAY_SIZE(msg)) > 0) {
				break;
			} else {
				mdelay(BMG_I2C_WRITE_DELAY_TIME);
			}
		}
		if (BMG_MAX_RETRY_I2C_XFER <= retry) {
			PERR("I2C xfer error");
			mutex_unlock(&client_data->bmg160_io_ctrl_mutex);
			return -EIO;
		}
		reg_addr++;
		data++;
	}
	mutex_unlock(&client_data->bmg160_io_ctrl_mutex);
	return 0;

}

static char bmg_i2c_read_wrapper(u8 dev_addr, u8 reg_addr, u8 *data, u8 len)
{
	char err = 0;
	err = bmg_i2c_read(bmg_client, reg_addr, data, len);
	return err;
}

static char bmg_i2c_write_wrapper(u8 dev_addr, u8 reg_addr, u8 *data, u8 len)
{
	char err = 0;
	err = bmg_i2c_write(bmg_client, reg_addr, data, len);
	return err;
}


static void bmg_work_func(struct work_struct *work)
{
	struct bmg_client_data *client_data =
		container_of((struct delayed_work *)work,
			struct bmg_client_data, work);

	unsigned long delay =
		msecs_to_jiffies(atomic_read(&client_data->delay));

	mutex_lock(&client_data->mutex_value);

	BMG_CALL_API(get_dataXYZ)(&client_data->value);

	input_report_abs(client_data->input, ABS_X, client_data->value.datax);
	input_report_abs(client_data->input, ABS_Y, client_data->value.datay);
	input_report_abs(client_data->input, ABS_Z, client_data->value.dataz);
	mutex_unlock(&client_data->mutex_value);

	input_sync(client_data->input);

	schedule_delayed_work(&client_data->work, delay);
}

/* PERI-FG-REGULATOR_SET-00+[ */
int bmg_set_power(int Mode)
{
    int rc = 0;
    struct bmg_client_data *client_data = i2c_get_clientdata(bmg_client);

    if (IS_ERR(client_data->reg))
    {
        PERR("Failed.");
        return -ENODEV;
    }

    if (Mode != BMG160_MODE_SUSPEND)
    {
        rc = regulator_set_optimum_mode(client_data->reg, client_data->hpm_uA);
        if (rc < 0)
        {
            PERR("enable failed.");
            return rc;
        }
        printk("bmg_set_power(): Mode(%d), %duA", Mode, client_data->hpm_uA);
    }
    else
    {
        rc = regulator_set_optimum_mode(client_data->reg, client_data->lpm_uA);
        if (rc < 0)
        {
            PERR("disable failed.");
            return rc;
        }
        printk("bmg_set_power(): Mode(%d), %duA", Mode, client_data->lpm_uA);
    }
    return 0;
}
EXPORT_SYMBOL_GPL(bmg_set_power);
/* PERI-FG-REGULATOR_SET-00+] */

static ssize_t bmg_show_chip_id(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    int ret;
    struct input_dev *input = to_input_dev(dev);
    struct bmg_client_data *client_data = input_get_drvdata(input);

    ret = bmg_check_chip_id(client_data->client);
    if(ret!=0)
        return snprintf(buf, PAGE_SIZE,"%d\n",0);
    else
        return snprintf(buf, PAGE_SIZE,"%d\n", SENSOR_CHIP_ID_BMG);
}

static ssize_t bmg_show_op_mode(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);
	u8 op_mode = 0xff;

	mutex_lock(&client_data->mutex_op_mode);
	BMG_CALL_API(get_mode)(&op_mode);
	mutex_unlock(&client_data->mutex_op_mode);

	ret = sprintf(buf, "%d\n", op_mode);

	return ret;
}

static ssize_t bmg_store_op_mode(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err = 0;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	long op_mode;

	err = strict_strtoul(buf, 10, &op_mode);
	if (err)
		return count;

	mutex_lock(&client_data->mutex_op_mode);

	err = BMG_CALL_API(set_mode)(op_mode);

	mutex_unlock(&client_data->mutex_op_mode);

	if (err)
		return count;
	else
		return count;
}



static ssize_t bmg_show_value(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);
	int count;

	BMG_CALL_API(get_dataXYZ)(&client_data->value);

	count = snprintf(buf, PAGE_SIZE, "%hd %hd %hd\n",
			client_data->value.datax,
			client_data->value.datay,
			client_data->value.dataz);

	return count;
}

static ssize_t bmg_show_range(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char range;
	BMG_CALL_API(get_range_reg)(&range);
	err = snprintf(buf, PAGE_SIZE, "%d\n", range);
	return err;
}

static ssize_t bmg_store_range(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long range;
	err = strict_strtoul(buf, 10, &range);
	if (err)
		return count;
	BMG_CALL_API(set_range_reg)(range);
	return count;
}

static ssize_t bmg_show_bandwidth(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char bandwidth;
	BMG_CALL_API(get_bw)(&bandwidth);
	err = snprintf(buf, PAGE_SIZE,"%d\n", bandwidth);
	return err;
}

static ssize_t bmg_store_bandwidth(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long bandwidth;
	err = strict_strtoul(buf, 10, &bandwidth);
	if (err)
		return count;
	BMG_CALL_API(set_bw)(bandwidth);
	return count;
}


static ssize_t bmg_show_enable(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);
	int err;

	mutex_lock(&client_data->mutex_enable);
	err = snprintf(buf, PAGE_SIZE,"%d\n", client_data->enable);
	mutex_unlock(&client_data->mutex_enable);
	return err;
}

static ssize_t bmg_store_enable(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int err;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	err = strict_strtoul(buf, 10, &data);
	if (err)
		return count;

	data = data ? 1 : 0;
	mutex_lock(&client_data->mutex_enable);
	if (data != client_data->enable) {
		if (data) {
			schedule_delayed_work(
					&client_data->work,
					msecs_to_jiffies(atomic_read(
							&client_data->delay)));
		} else {
			cancel_delayed_work_sync(&client_data->work);
		}

		client_data->enable = data;
	}
	mutex_unlock(&client_data->mutex_enable);

	return count;
}

static ssize_t bmg_show_delay(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&client_data->delay));

}

static ssize_t bmg_store_delay(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int err;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	err = strict_strtoul(buf, 10, &data);
	if (err)
		return count;

	if (data <= 0) {
		err = -EINVAL;
		return count;
	}

	if (data < BMG_DELAY_MIN)
		data = BMG_DELAY_MIN;

	atomic_set(&client_data->delay, data);

	return count;
}


static ssize_t bmg_store_fastoffset_en(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long fastoffset_en;
	err = strict_strtoul(buf, 10, &fastoffset_en);
	if (err)
		return count;
	if (fastoffset_en) {
		BMG_CALL_API(set_fast_offset_en_ch)(BMG160_X_AXIS, 1);
		BMG_CALL_API(set_fast_offset_en_ch)(BMG160_Y_AXIS, 1);
		BMG_CALL_API(set_fast_offset_en_ch)(BMG160_Z_AXIS, 1);
		BMG_CALL_API(enable_fast_offset)();
	}
	return count;
}

static ssize_t bmg_store_slowoffset_en(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long slowoffset_en;
	err = strict_strtoul(buf, 10, &slowoffset_en);
	if (err)
		return count;
	if (slowoffset_en) {
		BMG_CALL_API(set_slow_offset_th)(3);
		BMG_CALL_API(set_slow_offset_dur)(0);

		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_X_AXIS, 1);
		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_Y_AXIS, 1);
		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_Z_AXIS, 1);
	} else {
		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_X_AXIS, 0);
		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_Y_AXIS, 0);
		BMG_CALL_API(set_slow_offset_en_ch)(BMG160_Z_AXIS, 0);
	}

	return count;
}

static ssize_t bmg_show_selftest(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char selftest;
	BMG_CALL_API(selftest)(&selftest);
	err = snprintf(buf, PAGE_SIZE, "%d\n", selftest);
	return err;
}

static ssize_t bmg_show_sleepdur(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char sleepdur;
	BMG_CALL_API(get_sleepdur)(&sleepdur);
	err = snprintf(buf, PAGE_SIZE, "%d\n", sleepdur);
	return err;
}

static ssize_t bmg_store_sleepdur(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long sleepdur;
	err = strict_strtoul(buf, 10, &sleepdur);
	if (err)
		return count;
	BMG_CALL_API(set_sleepdur)(sleepdur);
	return count;
}

static ssize_t bmg_show_autosleepdur(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char autosleepdur;
	BMG_CALL_API(get_autosleepdur)(&autosleepdur);
	err = snprintf(buf, PAGE_SIZE, "%d\n", autosleepdur);
	return err;
}

static ssize_t bmg_store_autosleepdur(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long autosleepdur;
	unsigned char bandwidth;
	err = strict_strtoul(buf, 10, &autosleepdur);
	if (err)
		return count;
	BMG_CALL_API(get_bw)(&bandwidth);
	BMG_CALL_API(set_autosleepdur)(autosleepdur, bandwidth);
	return count;
}

#ifdef BMG_DEBUG
static ssize_t bmg_store_softreset(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long softreset;
	err = strict_strtoul(buf, 10, &softreset);
	if (err)
		return count;
	BMG_CALL_API(set_soft_reset)();
	return count;
}

static ssize_t bmg_show_dumpreg(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	size_t count = 0;
	u8 reg[0x40];
	int i;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	for (i = 0; i < 0x40; i++) {
		bmg_i2c_read(client_data->client, i, reg+i, 1);

		count += snprintf(&buf[count],PAGE_SIZE, "0x%x: %d\n", i, reg[i]);
	}
	return count;
}
#endif

#ifdef BMG_USE_FIFO
static ssize_t bmg_show_fifo_mode(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char fifo_mode;
	BMG_CALL_API(get_fifo_mode)(&fifo_mode);
	err = snprintf(buf, PAGE_SIZE, "%d\n", fifo_mode);
	return err;
}

static ssize_t bmg_store_fifo_mode(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	unsigned long fifo_mode;
	err = strict_strtoul(buf, 10, &fifo_mode);
	if (err)
		return count;
	BMG_CALL_API(set_fifo_mode)(fifo_mode);
	return count;
}

static ssize_t bmg_show_fifo_framecount(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char fifo_framecount;
	BMG_CALL_API(get_fifo_framecount)(&fifo_framecount);
	err = snprintf(buf, PAGE_SIZE, "%d\n", fifo_framecount);
	return err;
}

static ssize_t bmg_show_fifo_overrun(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char fifo_overrun;
	BMG_CALL_API(get_fifo_overrun)(&fifo_overrun);
	err = snprintf(buf, PAGE_SIZE, "%d\n", fifo_overrun);
	return err;
}

static ssize_t bmg_show_fifo_data_frame(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err, i, len;
	signed char fifo_data_out[MAX_FIFO_F_LEVEL * MAX_FIFO_F_BYTES] = {0};
	unsigned char f_count, f_len = 0;
	unsigned char fifo_datasel = 0;
	unsigned char fifo_tag = 0;

	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);

	BMG_CALL_API(get_fifo_data_sel)(&fifo_datasel);
	BMG_CALL_API(get_fifo_tag)(&fifo_tag);

	if (fifo_datasel)
		f_len = 2;
	else
		f_len = 6;

	if (fifo_tag)
		f_len += 2;

	BMG_CALL_API(get_fifo_framecount)(&f_count);

	bmg_i2c_burst_read(client_data->client, BMG160_FIFO_DATA_ADDR,
						fifo_data_out, f_count * f_len);
	err = 0;

	len = snprintf(buf, PAGE_SIZE, "%lu ", jiffies);
	buf += len;
	err += len;

	len = snprintf(buf, PAGE_SIZE,"%u ", f_count);
	buf += len;
	err += len;

	len = snprintf(buf, PAGE_SIZE, "%u ", f_len);
	buf += len;
	err += len;

	for (i = 0; i < f_count * f_len; i++)	{
		len = snprintf(buf, PAGE_SIZE,"%d ", fifo_data_out[i]);
		buf += len;
		err += len;
	}

	return err;
}

static ssize_t bmg_show_fifo_data_sel(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char fifo_data_sel;
	BMG_CALL_API(get_fifo_data_sel)(&fifo_data_sel);
	err = snprintf(buf, PAGE_SIZE, "%d\n", fifo_data_sel);
	return err;
}

static ssize_t bmg_store_fifo_data_sel(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)

{
	int err;
	unsigned long fifo_data_sel;
	err = strict_strtoul(buf, 10, &fifo_data_sel);
	if (err)
		return count;
	BMG_CALL_API(set_fifo_data_sel)(fifo_data_sel);
	return count;
}

static ssize_t bmg_show_fifo_tag(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	unsigned char fifo_tag;
	BMG_CALL_API(get_fifo_tag)(&fifo_tag);
	err = snprintf(buf, PAGE_SIZE, "%d\n", fifo_tag);
	return err;
}

static ssize_t bmg_store_fifo_tag(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)

{
	int err;
	unsigned long fifo_tag;
	err = strict_strtoul(buf, 10, &fifo_tag);
	if (err)
		return count;
	BMG_CALL_API(set_fifo_tag)(fifo_tag);
	return count;
}
#endif

static ssize_t bmg_show_layout(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int err;
	struct input_dev *input = to_input_dev(dev);
	struct bmg_client_data *client_data = input_get_drvdata(input);
	err = snprintf(buf, PAGE_SIZE, "%d\n", client_data->layout);
	return err;
}

static ssize_t bmg_store_layout(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
    unsigned long data;
    int err;
    struct input_dev *input = to_input_dev(dev);
    struct bmg_client_data *client_data = input_get_drvdata(input);

    err = strict_strtoul(buf, 10, &data);
    if (err)
        return count;


    if (data <= 0) {
        err = -EINVAL;
        return count;
    }

    if (data >= 7)
        data = 7;
    client_data->layout = data;
    return count;
}

static DEVICE_ATTR(chip_id, S_IRUGO,
		bmg_show_chip_id, NULL);
static DEVICE_ATTR(op_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_op_mode, bmg_store_op_mode);
static DEVICE_ATTR(value, S_IRUGO,
		bmg_show_value, NULL);
static DEVICE_ATTR(range, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_range, bmg_store_range);
static DEVICE_ATTR(bandwidth, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_bandwidth, bmg_store_bandwidth);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_enable, bmg_store_enable);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_delay, bmg_store_delay);
static DEVICE_ATTR(fastoffset_en, S_IRUGO|S_IWUSR|S_IWGRP,
		NULL, bmg_store_fastoffset_en);
static DEVICE_ATTR(slowoffset_en, S_IRUGO|S_IWUSR|S_IWGRP,
		NULL, bmg_store_slowoffset_en);
static DEVICE_ATTR(selftest, S_IRUGO,
		bmg_show_selftest, NULL);
static DEVICE_ATTR(sleepdur, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_sleepdur, bmg_store_sleepdur);
static DEVICE_ATTR(autosleepdur, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_autosleepdur, bmg_store_autosleepdur);
#ifdef BMG_DEBUG
static DEVICE_ATTR(softreset, S_IRUGO|S_IWUSR|S_IWGRP,
		NULL, bmg_store_softreset);
static DEVICE_ATTR(regdump, S_IRUGO,
		bmg_show_dumpreg, NULL);
#endif
#ifdef BMG_USE_FIFO
static DEVICE_ATTR(fifo_mode, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_mode, bmg_store_fifo_mode);
static DEVICE_ATTR(fifo_framecount, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_framecount, NULL);
static DEVICE_ATTR(fifo_overrun, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_overrun, NULL);
static DEVICE_ATTR(fifo_data_frame, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_data_frame, NULL);
static DEVICE_ATTR(fifo_data_sel, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_data_sel, bmg_store_fifo_data_sel);
static DEVICE_ATTR(fifo_tag, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_fifo_tag, bmg_store_fifo_tag);
#endif
static DEVICE_ATTR(layout, S_IRUGO|S_IWUSR|S_IWGRP,
		bmg_show_layout, bmg_store_layout);

static struct attribute *bmg_attributes[] = {
	&dev_attr_chip_id.attr,
	&dev_attr_op_mode.attr,
	&dev_attr_value.attr,
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	&dev_attr_fastoffset_en.attr,
	&dev_attr_slowoffset_en.attr,
	&dev_attr_selftest.attr,
	&dev_attr_sleepdur.attr,
	&dev_attr_autosleepdur.attr,
#ifdef DEBUG
	&dev_attr_softreset.attr,
	&dev_attr_regdump.attr,
#endif
#ifdef BMG_USE_FIFO
	&dev_attr_fifo_mode.attr,
	&dev_attr_fifo_framecount.attr,
	&dev_attr_fifo_overrun.attr,
	&dev_attr_fifo_data_frame.attr,
	&dev_attr_fifo_data_sel.attr,
	&dev_attr_fifo_tag.attr,
#endif
	&dev_attr_layout.attr,
	NULL
};

static struct attribute_group bmg_attribute_group = {
	.attrs = bmg_attributes
};


static int bmg_input_init(struct bmg_client_data *client_data)
{
	struct input_dev *dev;
	int err = 0;

	dev = input_allocate_device();
	if (NULL == dev)
		return -ENOMEM;

	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_X, MAG_VALUE_MIN, MAG_VALUE_MAX, 0, 0);
	input_set_abs_params(dev, ABS_Y, MAG_VALUE_MIN, MAG_VALUE_MAX, 0, 0);
	input_set_abs_params(dev, ABS_Z, MAG_VALUE_MIN, MAG_VALUE_MAX, 0, 0);
	input_set_drvdata(dev, client_data);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	client_data->input = dev;

	return 0;
}

static void bmg_input_destroy(struct bmg_client_data *client_data)
{
	struct input_dev *dev = client_data->input;

	input_unregister_device(dev);
	input_free_device(dev);
}

static int bmg_parse_dt(struct device *dev,
				struct bmg_client_data *pdata)
{
    int len = 0;	/* PERI-FG-REGULATOR_SET-00+ */
    const __be32 *prop;	/* PERI-FG-REGULATOR_SET-00+ */
    unsigned int temp_val = 0;
    struct device_node *np = dev->of_node;

    of_property_read_u32(np, "bosch,layout",&temp_val);
    pdata->layout = temp_val;

    /* PERI-FG-REGULATOR_SET-00+[ */
    /* Get the regulator handle */
    pdata->reg = regulator_get(dev, "bmgvdd");
    if (IS_ERR(pdata->reg))
    {
        PERR("Get regulator(bmgvdd) failed.\n");
        return -ENODEV;
    }

    prop = of_get_property(np, "bosch,vdd-current-level", &len);
    if (!prop || (len != (2 * sizeof(__be32))))
    {
        PERR("%s bosch,vdd-current-level property.\n", prop ? "invalid format" : "no");
        return -ENODEV;
    }
    else
    {
        pdata->lpm_uA = be32_to_cpup(&prop[0]);
        pdata->hpm_uA = be32_to_cpup(&prop[1]);
    }
    /* PERI-FG-REGULATOR_SET-00+] */

    return 0;
}

static int bmg_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;
    struct bmg_client_data *client_data = NULL;

    PINFO("function entrance");

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        PERR("i2c_check_functionality error!");
        err = -EIO;
        goto exit_err_clean;
    }

    if (NULL == bmg_client) {
        bmg_client = client;
    } else {
        PERR("this driver does not support multiple clients");
        err = -EINVAL;
        goto exit_err_clean;
    }

    client_data = kzalloc(sizeof(struct bmg_client_data), GFP_KERNEL);
    if (NULL == client_data) {
        PERR("no memory available");
        err = -ENOMEM;
        goto exit_err_clean;
    }

    if (client->dev.of_node)
    {
        bmg_parse_dt(&client->dev,client_data);
    }
    PERR("%s:layout =%d",__FUNCTION__,client_data->layout);

    i2c_set_clientdata(client, client_data);
    client_data->client = client;
    mutex_init(&client_data->mutex_op_mode);
    mutex_init(&client_data->mutex_enable);
    mutex_init(&client_data->mutex_value);
    mutex_init(&client_data->bmg160_io_ctrl_mutex);
    /* check chip id */
    err = bmg_check_chip_id(client);
    if (!err) {
        PNOTICE("Bosch Sensortec Device %s detected", SENSOR_NAME);
    } else {
        PERR("Bosch Sensortec Device not found, chip id mismatch");
        err = -1;
        goto exit_err_clean;
    }

    /* input device init */
    err = bmg_input_init(client_data);
    if (err < 0)
        goto exit_err_clean;

    /* sysfs node creation */
    err = sysfs_create_group(&client_data->input->dev.kobj,
    &bmg_attribute_group);

    if (err < 0)
        goto exit_err_sysfs;

    /* workqueue init */
    INIT_DELAYED_WORK(&client_data->work, bmg_work_func);
    atomic_set(&client_data->delay, BMG_DELAY_DEFAULT);

    /* h/w init */
    client_data->device.bus_read = bmg_i2c_read_wrapper;
    client_data->device.bus_write = bmg_i2c_write_wrapper;
    client_data->device.delay_msec = bmg_i2c_delay;
    BMG_CALL_API(init)(&client_data->device);

    bmg_dump_reg(client);

    client_data->enable = 0;
    /* now it's power on which is considered as resuming from suspend */
    err = BMG_CALL_API(set_mode)(
        BMG_VAL_NAME(MODE_SUSPEND));

    if (err < 0)
        goto exit_err_sysfs;


#ifdef CONFIG_HAS_EARLYSUSPEND
    client_data->early_suspend_handler.suspend = bmg_early_suspend;
    client_data->early_suspend_handler.resume = bmg_late_resume;
    register_early_suspend(&client_data->early_suspend_handler);
#endif

    PNOTICE("sensor %s probed successfully", SENSOR_NAME);

    PDEBUG("i2c_client: %p client_data: %p i2c_device: %p input: %p",
    		client, client_data, &client->dev, client_data->input);

    return 0;

exit_err_sysfs:
    if (err)
        bmg_input_destroy(client_data);

exit_err_clean:
    if (err) {
        if (client_data != NULL) {
            kfree(client_data);
            client_data = NULL;
        }

        bmg_client = NULL;
    }

    return err;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int bmg_pre_suspend(struct i2c_client *client)
{
	int err = 0;
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)i2c_get_clientdata(client);
	PINFO("function entrance");

	mutex_lock(&client_data->mutex_enable);
	if (client_data->enable) {
		cancel_delayed_work_sync(&client_data->work);
		PINFO("cancel work");
	}
	mutex_unlock(&client_data->mutex_enable);

	return err;
}

static int bmg_post_resume(struct i2c_client *client)
{
	int err = 0;
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)i2c_get_clientdata(client);

	PINFO("function entrance");
	mutex_lock(&client_data->mutex_enable);
	if (client_data->enable) {
		schedule_delayed_work(&client_data->work,
				msecs_to_jiffies(
					atomic_read(&client_data->delay)));
	}
	mutex_unlock(&client_data->mutex_enable);

	return err;
}

static void bmg_early_suspend(struct early_suspend *handler)
{
	int err = 0;
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)container_of(handler,
			struct bmg_client_data, early_suspend_handler);
	struct i2c_client *client = client_data->client;
	u8 op_mode;

	PINFO("function entrance");

	mutex_lock(&client_data->mutex_op_mode);
	BMG_CALL_API(get_mode)(&op_mode);
	if (op_mode == BMG_VAL_NAME(MODE_NORMAL)) {
		err = bmg_pre_suspend(client);
		err = BMG_CALL_API(set_mode)(
				BMG_VAL_NAME(MODE_SUSPEND));
	}
	mutex_unlock(&client_data->mutex_op_mode);
}

static void bmg_late_resume(struct early_suspend *handler)
{
	int err = 0;
	struct bmg_client_data *client_data =
		(struct bmg_client_data *)container_of(handler,
			struct bmg_client_data, early_suspend_handler);
	struct i2c_client *client = client_data->client;

	PINFO("function entrance");

	mutex_lock(&client_data->mutex_op_mode);

	if (client_data->enable)
	err = BMG_CALL_API(set_mode)(BMG_VAL_NAME(MODE_NORMAL));

	/* post resume operation */
	bmg_post_resume(client);

	mutex_unlock(&client_data->mutex_op_mode);
}
#endif

static int bmg_remove(struct i2c_client *client)
{
	int err = 0;
	u8 op_mode;

	struct bmg_client_data *client_data =
		(struct bmg_client_data *)i2c_get_clientdata(client);

	if (NULL != client_data) {
#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&client_data->early_suspend_handler);
#endif
		mutex_lock(&client_data->mutex_op_mode);
		BMG_CALL_API(get_mode)(&op_mode);
		if (BMG_VAL_NAME(MODE_NORMAL) == op_mode) {
			cancel_delayed_work_sync(&client_data->work);
			PINFO("cancel work");
		}
		mutex_unlock(&client_data->mutex_op_mode);

		err = BMG_CALL_API(set_mode)(
				BMG_VAL_NAME(MODE_SUSPEND));
		mdelay(BMG_I2C_WRITE_DELAY_TIME);

		sysfs_remove_group(&client_data->input->dev.kobj,
				&bmg_attribute_group);
		bmg_input_destroy(client_data);
		kfree(client_data);

		bmg_client = NULL;
	}

	return err;
}

static const struct i2c_device_id bmg_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bmg_id);

#ifdef CONFIG_OF
static struct of_device_id bmg160_match_table[] = {
	{ .compatible = "bosch,bmg160",},
	{ },
};
#else
#define rmi4_match_table NULL
#endif

static struct i2c_driver bmg_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
		.of_match_table = bmg160_match_table,
	},
	.class = I2C_CLASS_HWMON,
	.id_table = bmg_id,
	.probe = bmg_probe,
	.remove = bmg_remove,
};

static int __init BMG_init(void)
{
	return i2c_add_driver(&bmg_driver);
}

static void __exit BMG_exit(void)
{
	i2c_del_driver(&bmg_driver);
}

MODULE_AUTHOR("Ji.Chen <ji.chen@bosch-sensortec.com>");
MODULE_DESCRIPTION("driver for " SENSOR_NAME);
MODULE_LICENSE("GPL");

module_init(BMG_init);
module_exit(BMG_exit);
