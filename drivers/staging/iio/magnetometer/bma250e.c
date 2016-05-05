/*
 * Copyright (c) 2013-2014 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/iio/buffer.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>

#include "yas.h"

#define YAS_RANGE_2G                                                         (0)
#define YAS_RANGE_4G                                                         (1)
#define YAS_RANGE_8G                                                         (2)
#define YAS_RANGE_16G                                                        (3)
#define YAS_RANGE                                                   YAS_RANGE_2G
#if YAS_RANGE == YAS_RANGE_2G
#define YAS_RESOLUTION                                                     (256)
#elif YAS_RANGE == YAS_RANGE_4G
#define YAS_RESOLUTION                                                     (128)
#elif YAS_RANGE == YAS_RANGE_8G
#define YAS_RESOLUTION                                                      (64)
#elif YAS_RANGE == YAS_RANGE_16G
#define YAS_RESOLUTION                                                      (32)
#else
#define YAS_RESOLUTION                                                     (256)
#endif
#define YAS_GRAVITY_EARTH                                              (9806550)
#define YAS_STARTUP_TIME                                                  (3000)
/*
   YAS_WRITE_IDLE_TIME

   Idle time t,idle wacc sum (max.) is required after write accesses,
   but t,idle wacc sum (max.) is unknown. So, t,idle wacc sum (max.) is assumed
   here as 900 [usec], twice the t,idle wacc sum typical: 450 [usec].
*/
#define YAS_WRITE_IDLE_TIME                                                (900)
#define YAS_SOFTRESET_WAIT_TIME                                           (1800)
#define YAS_DEFAULT_POSITION                                                 (0)
#define YAS_BGW_CHIPID                                                    (0x00)
#define YAS_BGW_CHIPID_VAL                                                (0xf9)
#define YAS_BGW_SOFTRESET                                                 (0x14)
#define YAS_BGW_SOFTRESET_VAL                                             (0xb6)
#define YAS_PMU_LPW                                                       (0x11)
#define YAS_PMU_LPW_LOW1                                                  (0x40)
#define YAS_PMU_LPW_DSUS                                                  (0x20)
#define YAS_PMU_RANGE                                                     (0x0f)
#define YAS_PMU_RANGE_2G                                                  (0x03)
#define YAS_PMU_RANGE_4G                                                  (0x05)
#define YAS_PMU_RANGE_8G                                                  (0x08)
#define YAS_PMU_RANGE_16G                                                 (0x0c)
#if YAS_RANGE == YAS_RANGE_2G
#define YAS_PMU_RANGE_VAL                                       YAS_PMU_RANGE_2G
#elif YAS_RANGE == YAS_RANGE_4G
#define YAS_PMU_RANGE_VAL                                       YAS_PMU_RANGE_4G
#elif YAS_RANGE == YAS_RANGE_8G
#define YAS_PMU_RANGE_VAL                                       YAS_PMU_RANGE_8G
#elif YAS_RANGE == YAS_RANGE_16G
#define YAS_PMU_RANGE_VAL                                      YAS_PMU_RANGE_16G
#else
#define YAS_PMU_RANGE_VAL                                       YAS_PMU_RANGE_2G
#endif
#define YAS_SLEEPDUR_0                                                    (0x0a)
#define YAS_SLEEPDUR_1                                                    (0x0c)
#define YAS_SLEEPDUR_2                                                    (0x0e)
#define YAS_SLEEPDUR_4                                                    (0x10)
#define YAS_SLEEPDUR_6                                                    (0x12)
#define YAS_SLEEPDUR_10                                                   (0x14)
#define YAS_SLEEPDUR_25                                                   (0x16)
#define YAS_SLEEPDUR_50                                                   (0x18)
#define YAS_SLEEPDUR_100                                                  (0x1a)
#define YAS_SLEEPDUR_500                                                  (0x1c)
#define YAS_SLEEPDUR_1000                                                 (0x1e)
#define YAS_ACCD_X_LSB                                                    (0x02)

struct yas_sd {
	int delay;
	uint8_t sd;
};

struct yas_module {
	int initialized;
	int enable;
	int delay;
	int position;
	uint8_t lpw;
	struct yas_driver_callback cbk;
};

static const struct yas_sd yas_sd_tbl[] = {
	{0,    YAS_SLEEPDUR_0},
	{1,    YAS_SLEEPDUR_1},
	{2,    YAS_SLEEPDUR_2},
	{4,    YAS_SLEEPDUR_4},
	{6,    YAS_SLEEPDUR_6},
	{10,   YAS_SLEEPDUR_10},
	{25,   YAS_SLEEPDUR_25},
	{50,   YAS_SLEEPDUR_50},
	{100,  YAS_SLEEPDUR_100},
	{500,  YAS_SLEEPDUR_500},
	{1000, YAS_SLEEPDUR_1000}
};

static const int8_t yas_position_map[][3][3] = {
	{ { 0, -1,  0}, { 1,  0,  0}, { 0,  0,  1} },/* top/upper-left */
	{ { 1,  0,  0}, { 0,  1,  0}, { 0,  0,  1} },/* top/upper-right */
	{ { 0,  1,  0}, {-1,  0,  0}, { 0,  0,  1} },/* top/lower-right */
	{ {-1,  0,  0}, { 0, -1,  0}, { 0,  0,  1} },/* top/lower-left */
	{ { 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1} },/* bottom/upper-right */
	{ {-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1} },/* bottom/upper-left */
	{ { 0, -1,  0}, {-1,  0,  0}, { 0,  0, -1} },/* bottom/lower-left */
	{ { 1,  0,  0}, { 0, -1,  0}, { 0,  0, -1} },/* bottom/lower-right */
};

static struct yas_module module;

static int yas_read_reg(uint8_t adr, uint8_t *val);
static int yas_write_reg(uint8_t adr, uint8_t val);
static void yas_set_sd(int delay);
static int yas_power_up(void);
static int yas_power_down(void);
static int yas_init(void);
static int yas_term(void);
static int yas_get_delay(void);
static int yas_set_delay(int delay);
static int yas_get_enable(void);
static int yas_set_enable(int enable);
static int yas_get_position(void);
static int yas_set_position(int position);
static int yas_measure(struct yas_data *raw, int num);
static int yas_ext(int32_t cmd, void *result);

static int
yas_read_reg(uint8_t adr, uint8_t *val)
{
	return module.cbk.device_read(YAS_TYPE_ACC, adr, val, 1);
}

static int
yas_write_reg(uint8_t adr, uint8_t val)
{
	int rt;
	rt = module.cbk.device_write(YAS_TYPE_ACC, adr, &val, 1);
	module.cbk.usleep(YAS_WRITE_IDLE_TIME);
	return rt;
}

static void yas_set_sd(int delay)
{
	int i;
	if (delay < 3) {
		module.lpw = 0;
		return;
	}
	for (i = 1; i < NELEMS(yas_sd_tbl) &&
		     (delay - 3) >= yas_sd_tbl[i].delay; i++)
		;
	module.lpw = YAS_PMU_LPW_LOW1 | yas_sd_tbl[i-1].sd;
}

static int
yas_power_up(void)
{
	if (yas_write_reg(YAS_BGW_SOFTRESET, YAS_BGW_SOFTRESET_VAL) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	module.cbk.usleep(YAS_SOFTRESET_WAIT_TIME);
	if (yas_write_reg(YAS_PMU_RANGE, YAS_PMU_RANGE_VAL) < 0) {
		yas_write_reg(YAS_PMU_LPW, YAS_PMU_LPW_DSUS);
		return YAS_ERROR_DEVICE_COMMUNICATION;
	}
	if (yas_write_reg(YAS_PMU_LPW, module.lpw) < 0) {
		yas_write_reg(YAS_PMU_LPW, YAS_PMU_LPW_DSUS);
		return YAS_ERROR_DEVICE_COMMUNICATION;
	}
	return YAS_NO_ERROR;
}

static int
yas_power_down(void)
{
	if (yas_write_reg(YAS_PMU_LPW, YAS_PMU_LPW_DSUS) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	return YAS_NO_ERROR;
}

static uint8_t accel_product_id=0;
static int
yas_init(void)
{
	uint8_t id = 0;

	if (module.initialized)
		return YAS_ERROR_INITIALIZE;
	module.cbk.usleep(YAS_STARTUP_TIME);
	if (module.cbk.device_open(YAS_TYPE_ACC) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (yas_read_reg(YAS_BGW_CHIPID, &id) < 0) {
		module.cbk.device_close(YAS_TYPE_ACC);
		return YAS_ERROR_DEVICE_COMMUNICATION;
	}
	printk("who_am_i_val:%02x\n", id);
	accel_product_id = id;

	if (id != YAS_BGW_CHIPID_VAL) {
		module.cbk.device_close(YAS_TYPE_ACC);
		return YAS_ERROR_CHIP_ID;
	}
	module.enable = 0;
	module.delay = YAS_DEFAULT_SENSOR_DELAY;
	module.position = YAS_DEFAULT_POSITION;
	yas_set_sd(module.delay);
	yas_power_down();
	module.cbk.device_close(YAS_TYPE_ACC);
	module.initialized = 1;
	return YAS_NO_ERROR;
}

static int
yas_term(void)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	yas_set_enable(0);
	module.initialized = 0;
	return YAS_NO_ERROR;
}

static int
yas_get_delay(void)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	return module.delay;
}

static int
yas_set_delay(int delay)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	if (delay < 0)
		return YAS_ERROR_ARG;
	module.delay = delay;
	yas_set_sd(delay);
	if (!module.enable)
		return YAS_NO_ERROR;
	if (yas_write_reg(YAS_PMU_LPW, module.lpw) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	return YAS_NO_ERROR;
}

static int
yas_get_enable(void)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	return module.enable;
}

static int
yas_set_enable(int enable)
{
	int rt;

	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	if (enable != 0)
		enable = 1;
	if (module.enable == enable)
		return YAS_NO_ERROR;
	if (enable) {
		module.cbk.usleep(YAS_STARTUP_TIME);
		if (module.cbk.device_open(YAS_TYPE_ACC))
			return YAS_ERROR_DEVICE_COMMUNICATION;
		rt = yas_power_up();
		if (rt < 0) {
			module.cbk.device_close(YAS_TYPE_ACC);
			return rt;
		}
	} else {
		yas_power_down();
		module.cbk.device_close(YAS_TYPE_ACC);
	}
	module.enable = enable;
	return YAS_NO_ERROR;
}

static int
yas_get_position(void)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	return module.position;
}

static int
yas_set_position(int position)
{
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	if (position < 0 || position > 7)
		return YAS_ERROR_ARG;
	module.position = position;
	return YAS_NO_ERROR;
}

static int
yas_measure(struct yas_data *raw, int num)
{
	uint8_t buf[6];
	int16_t dat[3];
	int i, j;

	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	if (raw == NULL || num < 0)
		return YAS_ERROR_ARG;
	if (num == 0 || module.enable == 0)
		return 0;
	if (module.cbk.device_read(YAS_TYPE_ACC
				   , YAS_ACCD_X_LSB
				   , buf
				   , 6) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	for (i = 0; i < 3; i++)
		dat[i] = (int16_t)(((int16_t)((buf[i*2+1] << 8))
				    | buf[i*2]) >> 6);
	for (i = 0; i < 3; i++) {
		raw->xyz.v[i] = 0;
		for (j = 0; j < 3; j++)
			raw->xyz.v[i] += dat[j] *
				yas_position_map[module.position][i][j];
		raw->xyz.v[i] *= (YAS_GRAVITY_EARTH / YAS_RESOLUTION);
	}
	raw->type = YAS_TYPE_ACC;
	if (module.cbk.current_time == NULL)
		raw->timestamp = 0;
	else
		raw->timestamp = module.cbk.current_time();
	raw->accuracy = 0;
	return 1;
}

static int
yas_ext(int32_t cmd, void *result)
{
	(void)cmd;
	(void)result;
	if (!module.initialized)
		return YAS_ERROR_INITIALIZE;
	return YAS_NO_ERROR;
}

static int
yas_acc_driver_init(struct yas_acc_driver *f)
{
	if (f == NULL
	    || f->callback.device_open == NULL
	    || f->callback.device_close == NULL
	    || f->callback.device_write == NULL
	    || f->callback.device_read == NULL
	    || f->callback.usleep == NULL)
		return YAS_ERROR_ARG;
	f->init = yas_init;
	f->term = yas_term;
	f->get_delay = yas_get_delay;
	f->set_delay = yas_set_delay;
	f->get_enable = yas_get_enable;
	f->set_enable = yas_set_enable;
	f->get_position = yas_get_position;
	f->set_position = yas_set_position;
	f->measure = yas_measure;
	f->ext = yas_ext;
	module.cbk = f->callback;
	yas_term();
	return YAS_NO_ERROR;
}

static struct i2c_client *this_client;

enum {
	YAS_SCAN_ACCEL_X,
	YAS_SCAN_ACCEL_Y,
	YAS_SCAN_ACCEL_Z,
	YAS_SCAN_TIMESTAMP,
};
struct yas_state {
	struct mutex lock;
	struct yas_acc_driver acc;
	struct i2c_client *client;
	struct iio_trigger  *trig;
	struct delayed_work work;
	int16_t sampling_frequency;
	atomic_t pseudo_irq_enable;
	int32_t compass_data[3];
	int32_t calib_bias[3];
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend sus;
#endif
};

static int yas_device_open(int32_t type)
{
	return 0;
}

static int yas_device_close(int32_t type)
{
	return 0;
}

static int yas_device_write(int32_t type, uint8_t addr, const uint8_t *buf,
		int len)
{
	uint8_t tmp[2];
	if (sizeof(tmp) - 1 < len)
		return -1;
	tmp[0] = addr;
	memcpy(&tmp[1], buf, len);
	if (i2c_master_send(this_client, tmp, len + 1) < 0)
		return -1;
	return 0;
}

static int yas_device_read(int32_t type, uint8_t addr, uint8_t *buf, int len)
{
	struct i2c_msg msg[2];
	int err;
	msg[0].addr = this_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;
	msg[1].addr = this_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf;
	err = i2c_transfer(this_client->adapter, msg, 2);
	if (err != 2) {
		dev_err(&this_client->dev,
				"i2c_transfer() read error: "
				"slave_addr=%02x, reg_addr=%02x, err=%d\n",
				this_client->addr, addr, err);
		return err;
	}
	return 0;
}

static void yas_usleep(int us)
{
	usleep_range(us, us + 1000);
}

static uint32_t yas_current_time(void)
{
	return jiffies_to_msecs(jiffies);
}

static int yas_pseudo_irq_enable(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	if (!atomic_cmpxchg(&st->pseudo_irq_enable, 0, 1))
		schedule_delayed_work(&st->work, 0);
	return 0;
}

static int yas_pseudo_irq_disable(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	if (atomic_cmpxchg(&st->pseudo_irq_enable, 1, 0))
		cancel_delayed_work_sync(&st->work);
	return 0;
}

static int yas_set_pseudo_irq(struct iio_dev *indio_dev, int enable)
{
	if (enable)
		yas_pseudo_irq_enable(indio_dev);
	else
		yas_pseudo_irq_disable(indio_dev);
	return 0;
}

static int yas_data_rdy_trig_poll(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	iio_trigger_poll(st->trig, iio_get_time_ns());
	return 0;
}

static irqreturn_t yas_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct iio_buffer *buffer = indio_dev->buffer;
	struct yas_state *st = iio_priv(indio_dev);
	int len = 0, i, j;
	size_t datasize = buffer->access->get_bytes_per_datum(buffer);
	int32_t *acc;

	acc = (int32_t *) kmalloc(datasize, GFP_KERNEL);
	if (acc == NULL) {
		dev_err(indio_dev->dev.parent,
				"memory alloc failed in buffer bh");
		return -ENOMEM;
	}
	if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
		j = 0;
		for (i = 0; i < 3; i++) {
			if (test_bit(i, indio_dev->active_scan_mask)) {
				acc[j] = st->compass_data[i];
				j++;
			}
		}
		len = j * 4;
	}

	/* Guaranteed to be aligned with 8 byte boundary */
	if (buffer->scan_timestamp)
		*(s64 *)(((phys_addr_t)acc + len
					+ sizeof(s64) - 1) & ~(sizeof(s64) - 1))
			= pf->timestamp;
	iio_push_to_buffers(indio_dev, (u8 *)acc);

	iio_trigger_notify_done(indio_dev->trig);
	kfree(acc);
	return IRQ_HANDLED;
}

static int yas_data_rdy_trigger_set_state(struct iio_trigger *trig,
		bool state)
{
	struct iio_dev *indio_dev = iio_trigger_get_drvdata(trig);
	yas_set_pseudo_irq(indio_dev, state);
	return 0;
}

static const struct iio_trigger_ops yas_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = &yas_data_rdy_trigger_set_state,
};

static int yas_probe_trigger(struct iio_dev *indio_dev)
{
	int ret;
	struct yas_state *st = iio_priv(indio_dev);
	indio_dev->pollfunc = iio_alloc_pollfunc(&iio_pollfunc_store_time,
			&yas_trigger_handler, IRQF_ONESHOT, indio_dev,
			"%s_consumer%d", indio_dev->name, indio_dev->id);
	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	st->trig = iio_trigger_alloc("%s-dev%d",
			indio_dev->name,
			indio_dev->id);
	if (!st->trig) {
		ret = -ENOMEM;
		goto error_dealloc_pollfunc;
	}
	st->trig->dev.parent = &st->client->dev;
	st->trig->ops = &yas_trigger_ops;
	iio_trigger_set_drvdata(st->trig, indio_dev);

	ret = iio_trigger_register(st->trig);
	if (ret)
		goto error_free_trig;
	return 0;

error_free_trig:
	iio_trigger_free(st->trig);
error_dealloc_pollfunc:
	iio_dealloc_pollfunc(indio_dev->pollfunc);
error_ret:
	return ret;
}

static void yas_remove_trigger(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	iio_trigger_unregister(st->trig);
	iio_trigger_free(st->trig);
	iio_dealloc_pollfunc(indio_dev->pollfunc);
}

static const struct iio_buffer_setup_ops yas_buffer_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};

static void yas_remove_buffer(struct iio_dev *indio_dev)
{
	iio_buffer_unregister(indio_dev);
	iio_kfifo_free(indio_dev->buffer);
};

static int yas_probe_buffer(struct iio_dev *indio_dev)
{
	int ret;
	struct iio_buffer *buffer;

	buffer = iio_kfifo_allocate(indio_dev);
	if (!buffer) {
		ret = -ENOMEM;
		goto error_ret;
	}
	buffer->scan_timestamp = true;
	indio_dev->buffer = buffer;
	indio_dev->setup_ops = &yas_buffer_setup_ops;
	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;
	ret = iio_buffer_register(indio_dev, indio_dev->channels,
			indio_dev->num_channels);
	if (ret)
		goto error_free_buf;
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_ACCEL_X);
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_ACCEL_Y);
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_ACCEL_Z);
	return 0;

error_free_buf:
	iio_kfifo_free(indio_dev->buffer);
error_ret:
	return ret;
}

static ssize_t yas_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret;
	mutex_lock(&st->lock);
	ret = st->acc.get_position();
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return sprintf(buf, "%d\n", ret);
}

static ssize_t yas_position_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret, position;
	sscanf(buf, "%d\n", &position);
	mutex_lock(&st->lock);
	ret = st->acc.set_position(position);
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return count;
}

static ssize_t yas_sampling_frequency_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	return sprintf(buf, "%d\n", st->sampling_frequency);
}

static ssize_t yas_sampling_frequency_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret, data;
	ret = kstrtoint(buf, 10, &data);
	if (ret)
		return ret;
	if (data <= 0)
		return -EINVAL;
	st->sampling_frequency = data;
	return count;
}

static ssize_t yas_ping(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "0x19:0x%02x\n", accel_product_id);
}

static int yas_write_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan,
		int val,
		int val2,
		long mask)
{
	struct yas_state  *st = iio_priv(indio_dev);

	mutex_lock(&st->lock);

	switch (mask) {
	case IIO_CHAN_INFO_CALIBBIAS:
		st->calib_bias[chan->channel2 - IIO_MOD_X] = val;
		break;
	}

	mutex_unlock(&st->lock);

	return 0;
}

static int yas_read_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan,
		int *val,
		int *val2,
		long mask) {
	struct yas_state  *st = iio_priv(indio_dev);
	int ret = -EINVAL;

	if (chan->type != IIO_ACCEL)
		return -EINVAL;

	mutex_lock(&st->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		*val = st->compass_data[chan->channel2 - IIO_MOD_X];
		ret = IIO_VAL_INT;
		break;
	case IIO_CHAN_INFO_SCALE:
	case IIO_CHAN_INFO_CALIBSCALE:
		/* Gain : counts / m/s^2 = 1000000 [um/s^2] */
		/* Scaling factor : 1000000 / Gain = 1 */
		*val = 0;
		*val2 = 1;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;
	case IIO_CHAN_INFO_CALIBBIAS:
		*val = st->calib_bias[chan->channel2 - IIO_MOD_X];
		ret = IIO_VAL_INT;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static void yas_work_func(struct work_struct *work)
{
	struct yas_data acc[1];
	struct yas_state *st =
		container_of((struct delayed_work *)work,
				struct yas_state, work);
	struct iio_dev *indio_dev = iio_priv_to_dev(st);
	uint32_t time_before, time_after;
	int32_t delay;
	int ret, i;

	time_before = jiffies_to_msecs(jiffies);
	mutex_lock(&st->lock);
	ret = st->acc.measure(acc, 1);
	if (ret == 1) {
		for (i = 0; i < 3; i++)
			st->compass_data[i]
				= acc[0].xyz.v[i] - st->calib_bias[i];
	}
	mutex_unlock(&st->lock);
	if (ret == 1)
		yas_data_rdy_trig_poll(indio_dev);
	time_after = jiffies_to_msecs(jiffies);
	delay = MSEC_PER_SEC / st->sampling_frequency
		- (time_after - time_before);
	if (delay <= 0)
		delay = 1;
	schedule_delayed_work(&st->work, msecs_to_jiffies(delay));
}

#define YAS_ACCELEROMETER_CHANNEL(axis)				\
{								\
	.type = IIO_ACCEL,					\
	.modified = 1,						\
	.channel2 = IIO_MOD_##axis,				\
	.info_mask_separate = BIT(IIO_CHAN_INFO_CALIBSCALE) |	\
				BIT(IIO_CHAN_INFO_CALIBBIAS),	\
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),	\
	.scan_index = YAS_SCAN_ACCEL_##axis,			\
	.scan_type = IIO_ST('s', 32, 32, 0)			\
}

static const struct iio_chan_spec yas_channels[] = {
	YAS_ACCELEROMETER_CHANNEL(X),
	YAS_ACCELEROMETER_CHANNEL(Y),
	YAS_ACCELEROMETER_CHANNEL(Z),
	IIO_CHAN_SOFT_TIMESTAMP(YAS_SCAN_TIMESTAMP)
};

static IIO_DEVICE_ATTR(sampling_frequency, S_IRUSR|S_IWUSR,
		yas_sampling_frequency_show,
		yas_sampling_frequency_store, 0);
static IIO_DEVICE_ATTR(position, S_IRUSR|S_IWUSR,
		yas_position_show, yas_position_store, 0);
static IIO_DEVICE_ATTR(ping, S_IRUGO|S_IWUSR|S_IWGRP,
		yas_ping, NULL, 0);

static struct attribute *yas_attributes[] = {
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_position.dev_attr.attr,
	&iio_dev_attr_ping.dev_attr.attr,
	NULL
};
static const struct attribute_group yas_attribute_group = {
	.attrs = yas_attributes,
};

static const struct iio_info yas_info = {
	.read_raw = &yas_read_raw,
	.write_raw = &yas_write_raw,
	.attrs = &yas_attribute_group,
	.driver_module = THIS_MODULE,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void yas_early_suspend(struct early_suspend *h)
{
	struct yas_state *st = container_of(h,
			struct yas_state, sus);
	if (atomic_read(&st->pseudo_irq_enable))
		cancel_delayed_work_sync(&st->work);
}


static void yas_late_resume(struct early_suspend *h)
{
	struct yas_state *st = container_of(h,
			struct yas_state, sus);
	if (atomic_read(&st->pseudo_irq_enable))
		schedule_delayed_work(&st->work, 0);
}
#endif

static int yas_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct yas_state *st;
	struct iio_dev *indio_dev;
	int ret, i;

	this_client = i2c;
	printk("[CCI]%s: yas_bosch_accel_probe start ---\n", __FUNCTION__);

	indio_dev = iio_device_alloc(sizeof(*st));
	if (!indio_dev) {
		ret = -ENOMEM;
		goto error_ret;
	}
	i2c_set_clientdata(i2c, indio_dev);

	indio_dev->name = id->name;
	indio_dev->dev.parent = &i2c->dev;
	indio_dev->info = &yas_info;
	indio_dev->channels = yas_channels;
	indio_dev->num_channels = ARRAY_SIZE(yas_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	st = iio_priv(indio_dev);
	st->client = i2c;
	st->sampling_frequency = 20;
	st->acc.callback.device_open = yas_device_open;
	st->acc.callback.device_close = yas_device_close;
	st->acc.callback.device_read = yas_device_read;
	st->acc.callback.device_write = yas_device_write;
	st->acc.callback.usleep = yas_usleep;
	st->acc.callback.current_time = yas_current_time;
	INIT_DELAYED_WORK(&st->work, yas_work_func);
	mutex_init(&st->lock);
#ifdef CONFIG_HAS_EARLYSUSPEND
	st->sus.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	st->sus.suspend = yas_early_suspend;
	st->sus.resume = yas_late_resume;
	register_early_suspend(&st->sus);
#endif
	for (i = 0; i < 3; i++) {
		st->compass_data[i] = 0;
		st->calib_bias[i] = 0;
	}

	ret = yas_probe_buffer(indio_dev);
	if (ret)
		goto error_free_dev;
	ret = yas_probe_trigger(indio_dev);
	if (ret)
		goto error_remove_buffer;
	ret = iio_device_register(indio_dev);
	if (ret)
		goto error_remove_trigger;
	ret = yas_acc_driver_init(&st->acc);
	if (ret < 0) {
		ret = -EFAULT;
		goto error_unregister_iio;
	}
	ret = st->acc.init();
	if (ret < 0) {
		ret = -EFAULT;
		goto error_unregister_iio;
	}
	ret = st->acc.set_enable(1);
	if (ret < 0) {
		ret = -EFAULT;
		goto error_driver_term;
	}
	printk("[CCI]%s: yas_bosch_accel_probe end ---\n", __FUNCTION__);

	return 0;

error_driver_term:
	st->acc.term();
error_unregister_iio:
	iio_device_unregister(indio_dev);
error_remove_trigger:
	yas_remove_trigger(indio_dev);
error_remove_buffer:
	yas_remove_buffer(indio_dev);
error_free_dev:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&st->sus);
#endif
	iio_device_free(indio_dev);
error_ret:
	i2c_set_clientdata(i2c, NULL);
	this_client = NULL;
	return ret;
}

static int yas_remove(struct i2c_client *i2c)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(i2c);
	struct yas_state *st;
	if (indio_dev) {
		st = iio_priv(indio_dev);
#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&st->sus);
#endif
		yas_pseudo_irq_disable(indio_dev);
		st->acc.term();
		iio_device_unregister(indio_dev);
		yas_remove_trigger(indio_dev);
		yas_remove_buffer(indio_dev);
		iio_device_free(indio_dev);
		this_client = NULL;
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int yas_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	if (atomic_read(&st->pseudo_irq_enable))
		cancel_delayed_work_sync(&st->work);
	st->acc.set_enable(0);
	return 0;
}

static int yas_resume(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	st->acc.set_enable(1);
	if (atomic_read(&st->pseudo_irq_enable))
		schedule_delayed_work(&st->work, 0);
	return 0;
}

static SIMPLE_DEV_PM_OPS(yas_pm_ops, yas_suspend, yas_resume);
#define YAS_PM_OPS (&yas_pm_ops)
#else
#define YAS_PM_OPS NULL
#endif

static const struct i2c_device_id yas_id[] = {
	{ "bma250e", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, yas_id);

static struct of_device_id bma250e_of_match[] = {
		{ .compatible  = "qcom,bma250e",},
		{ .compatible  = "bma250e",},
		{ },
};
MODULE_DEVICE_TABLE(of, bma250e_of_match);

static struct i2c_driver yas_driver = {
	.driver = {
		.name	= "bma250e",
		.owner	= THIS_MODULE,
		.pm	= YAS_PM_OPS,
		.of_match_table = bma250e_of_match,
	},
	.probe		= yas_probe,
	.remove		= yas_remove,
	.id_table	= yas_id,
};
static int __init yas_initialize(void)
{
	return i2c_add_driver(&yas_driver);
}

static void __exit yas_terminate(void)
{
	i2c_del_driver(&yas_driver);
}

module_init(yas_initialize);
module_exit(yas_terminate);

MODULE_DESCRIPTION("Bosch BMA250E I2C driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("5.1.1000c.c2");
