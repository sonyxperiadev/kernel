/*  Copyright (c) 2011  Bosch Sensortec GmbH
    Copyright (c) 2011  Unixphere

    Based on:
    BMP085 driver, bmp085.c
    Copyright (c) 2010  Christoph Mair <christoph.mair@gmail.com>

    This driver supports the bmp18x digital barometric pressure
    and temperature sensors from Bosch Sensortec.

    A pressure measurement is issued by reading from pressure0_input.
    The return value ranges from 30000 to 110000 pascal with a resulution
    of 1 pascal (0.01 millibar) which enables measurements from 9000m above
    to 500m below sea level.

    The temperature can be read from temp0_input. Values range from
    -400 to 850 representing the ambient temperature in degree celsius
    multiplied by 10.The resolution is 0.1 celsius.

    Because ambient pressure is temperature dependent, a temperature
    measurement will be executed automatically even if the user is reading
    from pressure0_input. This happens if the last temperature measurement
    has been executed more then one second ago.

    To decrease RMS noise from pressure measurements, the bmp18x can
    autonomously calculate the average of up to eight samples. This is
    set up by writing to the oversampling sysfs file. Accepted values
    are 0, 1, 2 and 3. 2^x when x is the value written to this file
    specifies the number of samples used to calculate the ambient pressure.
    RMS noise is specified with six pascal (without averaging) and decreases
    down to 3 pascal when using an oversampling setting of 3.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "linux/bmp18x.h"
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>

#define BMP18X_CHIP_ID			          0x55

#define BMP18X_CALIBRATION_DATA_START	  0xAA
#define BMP18X_CALIBRATION_DATA_LENGTH	  11	/* 16 bit values */
#define BMP18X_CHIP_ID_REG		          0xD0
#define BMP18X_CTRL_REG			          0xF4
#define BMP18X_TEMP_MEASUREMENT		      0x2E
#define BMP18X_PRESSURE_MEASUREMENT	      0x34
#define BMP18X_CONVERSION_REGISTER_MSB	  0xF6
#define BMP18X_CONVERSION_REGISTER_LSB	  0xF7
#define BMP18X_CONVERSION_REGISTER_XLSB	  0xF8
#define BMP18X_TEMP_CONVERSION_TIME	      5

/* 50 msec */
#define PRESSURE_SENSOR_START_TIME_DELAY	50000000
#define TEMP_SENSOR_START_TIME_DELAY		50000000

static int mod_debug = 0;
module_param(mod_debug, int, 0644);

enum {
	pressure = 0,
	temp,
	numFds
};
struct bmp18x_calibration_data
{
	s16 AC1, AC2, AC3;
	u16 AC4, AC5, AC6;
	s16 B1, B2;
	s16 MB, MC, MD;
};


/* Each client has this additional data */
struct bmp18x_data
{
	struct	bmp18x_data_bus         data_bus;
	struct	device*                 dev;
	struct input_dev 				*input_dev;
	struct	mutex                   lock;
	struct	bmp18x_calibration_data calibration;
	u8	                            oversampling_setting;
	u32	                            raw_temperature;
	u32	                            raw_pressure;
	u32	                            temp_measurement_period;
	u32	                            last_temp_measurement;
	s32	                            b6; /* calculated temperature correction coefficient */
	struct work_struct 		work_pressure;
	struct work_struct 		work_temp;
	struct hrtimer 			timer_pressure;
	struct hrtimer 			timer_temp;
	ktime_t 			poll_delay[numFds];
	struct workqueue_struct 	*wq[numFds];
};


static s32 bmp18x_read_calibration_data(struct bmp18x_data* data)
{
	u16 tmp[BMP18X_CALIBRATION_DATA_LENGTH];
	struct bmp18x_calibration_data* cali = &(data->calibration);

	s32 status = data->data_bus.bops->read_block(data->data_bus.client,
				BMP18X_CALIBRATION_DATA_START,
				BMP18X_CALIBRATION_DATA_LENGTH * sizeof(u16),
				(u8*)tmp);

	if (status < 0)
		return status;

	if (status != BMP18X_CALIBRATION_DATA_LENGTH*sizeof(u16))
		return -EIO;

	cali->AC1 =  be16_to_cpu(tmp[0]);
	cali->AC2 =  be16_to_cpu(tmp[1]);
	cali->AC3 =  be16_to_cpu(tmp[2]);
	cali->AC4 =  be16_to_cpu(tmp[3]);
	cali->AC5 =  be16_to_cpu(tmp[4]);
	cali->AC6 =  be16_to_cpu(tmp[5]);
	cali->B1  =  be16_to_cpu(tmp[6]);
	cali->B2  =  be16_to_cpu(tmp[7]);
	cali->MB  =  be16_to_cpu(tmp[8]);
	cali->MC  =  be16_to_cpu(tmp[9]);
	cali->MD  =  be16_to_cpu(tmp[10]);

	return 0;
}


static s32 bmp18x_update_raw_temperature(struct bmp18x_data* data)
{
	u16 tmp;
	s32 status;

	if (!mutex_trylock(&data->lock))
		return -EAGAIN;
	status = data->data_bus.bops->write_byte(data->data_bus.client,
				BMP18X_CTRL_REG, BMP18X_TEMP_MEASUREMENT);

	if (status != 0)
	{
		dev_err(data->dev,
			"Error while requesting temperature measurement.\n");
		goto exit;
	}

	msleep(BMP18X_TEMP_CONVERSION_TIME);

	status = data->data_bus.bops->read_block(data->data_bus.client,
		BMP18X_CONVERSION_REGISTER_MSB, sizeof(tmp), (u8 *)&tmp);

	if (status < 0)
		goto exit;

	if (status != sizeof(tmp))
	{
		dev_err(data->dev,
			"Error while reading temperature measurement result\n");
		status = -EIO;
		goto exit;
	}

	data->raw_temperature = be16_to_cpu(tmp);
	data->last_temp_measurement = jiffies;

	status = 0;	/* everything ok, return 0 */

exit:
	mutex_unlock(&data->lock);
	return status;
}

static s32 bmp18x_update_raw_pressure(struct bmp18x_data* data)
{
	u32 tmp = 0;
	s32 status;

	if (!mutex_trylock(&data->lock))
		return -EAGAIN;
	status = data->data_bus.bops->write_byte(data->data_bus.client,
		BMP18X_CTRL_REG, BMP18X_PRESSURE_MEASUREMENT +
		(data->oversampling_setting<<6));

	if (status != 0)
	{
		dev_err(data->dev,
			"Error while requesting pressure measurement.\n");
		goto exit;
	}

	/* wait for the end of conversion */
	msleep(2+(3 << data->oversampling_setting));

	/* copy data into a u32 (4 bytes), but skip the first byte. */
	status = data->data_bus.bops->read_block(data->data_bus.client,
			BMP18X_CONVERSION_REGISTER_MSB, 3, ((u8*)&tmp)+1);

	if (status < 0)
		goto exit;

	if (status != 3)
	{
		dev_err(data->dev,
			"Error while reading pressure measurement results\n");

		status = -EIO;
		goto exit;
	}

	data->raw_pressure = be32_to_cpu((tmp));
	data->raw_pressure >>= (8-data->oversampling_setting);
	status = 0;	/* everything ok, return 0 */


exit:
	mutex_unlock(&data->lock);
	return status;
}


/*
 * This function starts the temperature measurement and returns the value
 * in tenth of a degree celsius.
 */
static s32 bmp18x_get_temperature(struct bmp18x_data* data, int* temperature)
{
	struct bmp18x_calibration_data *cali = &data->calibration;
	long x1, x2;
	int status;

	status = bmp18x_update_raw_temperature(data);
	if (status != 0)
		goto exit;

	x1 = ((data->raw_temperature - cali->AC6) * cali->AC5) >> 15;
	x2 = (cali->MC << 11) / (x1 + cali->MD);
	data->b6 = x1 + x2 - 4000;

	/* if NULL just update b6. Used for pressure only measurements */
	if (temperature != NULL)
		*temperature = (x1+x2+8) >> 4;

exit:
	return status;
}

/*
 * This function starts the pressure measurement and returns the value
 * in millibar. Since the pressure depends on the ambient temperature,
 * a temperature measurement is executed according to the given temperature
 * measurememt period (default is 1 sec boundary). This period could vary
 * and needs to be adjusted accoring to the sensor environment, i.e. if big
 * temperature variations then the temperature needs to be read out often.
 */
static s32 bmp18x_get_pressure(struct bmp18x_data* data, int* pressure)
{
	struct bmp18x_calibration_data* cali = &data->calibration;
	s32 x1, x2, x3, b3;
	u32 b4, b7;
	s32 p;
	int status;

	/* update the ambient temperature according to the given meas. period */
	if (data->last_temp_measurement +
			data->temp_measurement_period < jiffies)
	{
		status = bmp18x_get_temperature(data, NULL);
		if (status != 0)
			goto exit;
	}

	status = bmp18x_update_raw_pressure(data);
	if (status != 0)
		goto exit;

	x1 = (data->b6 * data->b6) >> 12;
	x1 *= cali->B2;
	x1 >>= 11;

	x2 = cali->AC2 * data->b6;
	x2 >>= 11;

	x3 = x1 + x2;

	b3 = (((((s32)cali->AC1) * 4 + x3) << data->oversampling_setting) + 2);
	b3 >>= 2;

	x1 = (cali->AC3 * data->b6) >> 13;
	x2 = (cali->B1 * ((data->b6 * data->b6) >> 12)) >> 16;
	x3 = (x1 + x2 + 2) >> 2;
	b4 = (cali->AC4 * (u32)(x3 + 32768)) >> 15;

	b7 = ((u32)data->raw_pressure - b3) *
					(50000 >> data->oversampling_setting);

	p = ((b7 < 0x80000000) ? ((b7 << 1) / b4) : ((b7 / b4) * 2));

	x1 = p >> 8;
	x1 *= x1;
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p += (x1 + x2 + 3791) >> 4;

	*pressure = p;

exit:
	return status;
}

/*
 * This function sets the chip-internal oversampling. Valid values are 0..3.
 * The chip will use 2^oversampling samples for internal averaging.
 * This influences the measurement time and the accuracy; larger values
 * increase both. The datasheet gives on overview on how measurement time,
 * accuracy and noise correlate.
 */
static void bmp18x_set_oversampling
(
   struct bmp18x_data* data,
   unsigned char       oversampling
)
{
	if (oversampling > 3)
		oversampling = 3;

	data->oversampling_setting = oversampling;
}

/*
 * Returns the currently selected oversampling. Range: 0..3
 */
static unsigned char bmp18x_get_oversampling(struct bmp18x_data* data)
{
	return data->oversampling_setting;
}

/* sysfs callbacks */
static ssize_t set_oversampling
(
   struct device*            dev,
   struct device_attribute*  attr,
   const  char*              buf,
   size_t                    count
)
{
	struct bmp18x_data* data = dev_get_drvdata(dev);
	unsigned long oversampling;
	int success = strict_strtoul(buf, 10, &oversampling);

	if (success == 0)
	{
		mutex_lock(&data->lock);
		bmp18x_set_oversampling(data, oversampling);
		mutex_unlock(&data->lock);
		return count;
	}

	return success;
}

static ssize_t show_oversampling
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	struct bmp18x_data* data = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", bmp18x_get_oversampling(data));
}

static DEVICE_ATTR(oversampling, S_IWUSR | S_IRUGO,
					show_oversampling, set_oversampling);


static ssize_t show_temperature
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf)
{
	int temperature;
	int status;
	struct bmp18x_data* data = dev_get_drvdata(dev);

	status = bmp18x_get_temperature(data, &temperature);

	if (status != 0)
		return status;
	else
	    return sprintf(buf, "%d\n", temperature);

}

static DEVICE_ATTR(temp0_input, S_IRUGO, show_temperature, NULL);


static ssize_t show_pressure
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	int pressure;
	int status;
	struct bmp18x_data* data = dev_get_drvdata(dev);

	status = bmp18x_get_pressure(data, &pressure);

	if (status != 0)
		return status;
	else
		return sprintf(buf, "%d\n", pressure);
}


static DEVICE_ATTR(pressure0_input, S_IRUGO, show_pressure, NULL);

static ssize_t bmp18x_get_data
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	int pressure = 0; int temperature = 0;
	int status = 0;

	struct bmp18x_data* data = dev_get_drvdata(dev);

	status = bmp18x_get_pressure(data, &pressure);
	if (status == 0)
		status = bmp18x_get_temperature(data, &temperature);

	if (status != 0)
		return status;
	else
	{
	    memcpy(buf, &pressure, sizeof(pressure) );
	    memcpy(buf + sizeof(pressure), &temperature, sizeof(temperature) );

	    return sizeof(pressure) + sizeof(temperature);
	}
}

static DEVICE_ATTR(data, S_IRUGO, bmp18x_get_data, NULL);

static ssize_t bmp18x_set_temp_enable(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	unsigned long enable = 0;
	int ret = strict_strtoul(buf, 10, &enable);

	if (ret < 0)
		return ret;

#ifdef CONFIG_PM
	if (enable)
		bmp18x_enable(dev);
	else
		bmp18x_disable(dev);
#endif
	/* we need to reset the input abs value */
    input_report_abs(data->input_dev, ABS_MISC, 0);
    input_sync(data->input_dev);

	if (enable) {
		hrtimer_start(&data->timer_temp,
			ktime_set(0, TEMP_SENSOR_START_TIME_DELAY), HRTIMER_MODE_REL);
	}
	else {
		hrtimer_cancel(&data->timer_temp);
		cancel_work_sync(&data->work_temp);
	}
	return count;
}
static DEVICE_ATTR(temp_enable, 0664, NULL, bmp18x_set_temp_enable);

static ssize_t bmp18x_set_pressure_enable(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	unsigned long enable = 0;
	int ret = strict_strtoul(buf, 10, &enable);

	if (ret < 0)
		return ret;

#ifdef CONFIG_PM
	if (enable)
		bmp18x_enable(dev);
	else
		bmp18x_disable(dev);
#endif
	/* we need to reset the input abs value */
    input_report_abs(data->input_dev, ABS_PRESSURE, 0);
    input_sync(data->input_dev);

	if (enable) {
		hrtimer_start(&data->timer_pressure,
			ktime_set(0, PRESSURE_SENSOR_START_TIME_DELAY), HRTIMER_MODE_REL);
	}
	else {
		hrtimer_cancel(&data->timer_pressure);
		cancel_work_sync(&data->work_pressure);
	}
	return count;
}
static DEVICE_ATTR(pressure_enable, 0664, NULL, bmp18x_set_pressure_enable);

static ssize_t bmp18x_set_temp_poll_delay(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	unsigned long poll_interval = 0;
	int ret = strict_strtoul(buf, 10, &poll_interval);

	if (ret < 0)
		return ret;

	data->poll_delay[temp] = ns_to_ktime(poll_interval);
	return count;
}

static ssize_t bmp18x_set_pressure_poll_delay(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	unsigned long poll_interval = 0;
	int ret = strict_strtoul(buf, 10, &poll_interval);

	if (ret < 0)
		return ret;

	data->poll_delay[pressure] = ns_to_ktime(poll_interval);

	return count;
}

static ssize_t bmp18x_get_temp_poll_delay( struct device* dev, struct device_attribute* attr, char* buf)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%lld ns\n",ktime_to_ns(data->poll_delay[temp]));
}

static ssize_t bmp18x_get_pressure_poll_delay( struct device* dev, struct device_attribute* attr, char* buf)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%lld ns\n",ktime_to_ns(data->poll_delay[pressure]));
}

static DEVICE_ATTR(temp_poll_delay, 0664,
	bmp18x_get_temp_poll_delay, bmp18x_set_temp_poll_delay);
static DEVICE_ATTR(pressure_poll_delay, 0664,
	bmp18x_get_pressure_poll_delay, bmp18x_set_pressure_poll_delay);

static struct attribute* bmp18x_attributes[] =
{
	&dev_attr_temp0_input.attr,
	&dev_attr_pressure0_input.attr,
	&dev_attr_data.attr,
	&dev_attr_oversampling.attr,
	NULL
};

static const struct attribute_group bmp18x_attr_group =
{
	.attrs = bmp18x_attributes,
};

static struct attribute* bmp18x_android_attributes[] =
{
	&dev_attr_pressure_enable.attr,
	&dev_attr_pressure_poll_delay.attr,
	&dev_attr_temp_enable.attr,
	&dev_attr_temp_poll_delay.attr,
	NULL
};

static const struct attribute_group bmp18x_android_attr_group =
{
	.attrs = bmp18x_android_attributes,
};

static int bmp18x_init_client
(
   struct bmp18x_data*          data,
   struct bmp18x_platform_data* pdata
)
{
	int status = bmp18x_read_calibration_data(data);
	if (status != 0)
		goto exit;

	data->last_temp_measurement = 0;
	data->temp_measurement_period =
		pdata ? (pdata->temp_measurement_period/1000) * HZ : 1 * HZ;

	data->oversampling_setting = pdata ? pdata->default_oversampling : 3;
	mutex_init(&data->lock);

exit:
	return status;
}

/* work func */
static void bmp18x_work_func_pressure(struct work_struct *work)
{
	struct bmp18x_data *data = container_of(work, struct bmp18x_data, work_pressure);
	int pressure = 0;
	int status;

	status = bmp18x_get_pressure(data, &pressure);
	if (status != 0)
		return;

	if (mod_debug > 0)
		printk("BMP180x pressure: report %d\n", pressure);

	input_report_abs(data->input_dev, ABS_PRESSURE, pressure);
	input_sync(data->input_dev);
}

static void bmp18x_work_func_temp(struct work_struct *work)
{
	struct bmp18x_data *data = container_of(work, struct bmp18x_data, work_temp);
	int temp = 0;
	int	status;

	status = bmp18x_get_temperature(data, &temp);
	if (status != 0)
		return;

	if (mod_debug > 0)
		printk("BMP180x Temp: report %d\n", temp);

	input_report_abs(data->input_dev, ABS_MISC, temp);
	input_sync(data->input_dev);
}

/* Pressure function */
static enum hrtimer_restart bmp18x_pressure_timer_func(struct hrtimer *timer)
{
	struct bmp18x_data *data = container_of(timer, struct bmp18x_data, timer_pressure);

	queue_work(data->wq[pressure], &data->work_pressure);
	hrtimer_forward_now(&data->timer_pressure, data->poll_delay[pressure]);
	return HRTIMER_RESTART;
}

static enum hrtimer_restart bmp18x_temp_timer_func(struct hrtimer *timer)
{
	struct bmp18x_data *data = container_of(timer, struct bmp18x_data, timer_temp);
	queue_work(data->wq[temp], &data->work_temp);
	hrtimer_forward_now(&data->timer_temp, data->poll_delay[temp]);
	return HRTIMER_RESTART;
}

static int bmp18x_work_init(struct bmp18x_data *data)
{
	/* Pressure device */
	/* hrtimer settings.  we poll for pressure values using a timer. */
	hrtimer_init(&data->timer_pressure, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->poll_delay[pressure] = ns_to_ktime(PRESSURE_SENSOR_START_TIME_DELAY);
	data->timer_pressure.function = bmp18x_pressure_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	* to read the i2c (can be slow and blocking)
	*/
	data->wq[pressure] = create_singlethread_workqueue("bmp18x_pressure_wq");
	if (!data->wq[pressure]) {
		pr_err("%s: could not create workqueue\n", __func__);
		return -ENOMEM;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&data->work_pressure, bmp18x_work_func_pressure);

	/* Temperature Sensor device */
	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&data->timer_temp, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->poll_delay[temp] = ns_to_ktime(TEMP_SENSOR_START_TIME_DELAY);
	data->timer_temp.function = bmp18x_temp_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	* to read the i2c (can be slow and blocking)
	*/
	data->wq[temp] = create_singlethread_workqueue("bmp18x_temp_wq");
	if (!data->wq[temp]) {
		pr_err("%s: could not create workqueue\n", __func__);
		destroy_workqueue(data->wq[pressure]);
		hrtimer_cancel(&data->timer_pressure);
		cancel_work_sync(&data->work_pressure);
		hrtimer_cancel(&data->timer_temp);
		return -ENOMEM;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&data->work_temp, bmp18x_work_func_temp);

	return 0;
}

static int bmp18x_input_init(struct device* dev, struct bmp18x_data *data)
{
	int err = -1;
	struct input_dev *input_dev;

	/* Allocate input device for pressure sensor */
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		printk("BMP18x: input device allocated failed\n");
		goto err0;
	}

	data->input_dev = input_dev;
	input_set_drvdata(input_dev, data);
	input_dev->name = "BMP18X";
	input_dev->phys = BMP18X_NAME;
	input_dev->id.bustype = data->data_bus.bops->bus_type;
	input_dev->dev.parent = dev;
	/* Pressure */
	input_set_capability(input_dev, EV_ABS, ABS_PRESSURE);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 300, 0, 0);
	/* Temp */
	input_set_capability(input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(input_dev, ABS_MISC, 0, 10000, 0, 0);

	err = input_register_device(data->input_dev);
	if (err) {
		printk("input register device (%s) failed with err %d\n", input_dev->name, err);
		goto err1;
	}
	printk("bmp18x_probe: install sensor success\n");

	err = bmp18x_work_init(data);
	if (err) {
		printk("bmp18x_work_init failed with err %d\n", err);
		goto err2;
	}

	/* sysfs create */
	err = sysfs_create_group(&data->input_dev->dev.kobj, &bmp18x_android_attr_group);

	return 0;

err2:
	input_unregister_device(data->input_dev);
err1:
	input_free_device(data->input_dev);
err0:
	return err;
}
int bmp18x_probe(struct device* dev, struct bmp18x_data_bus* data_bus)
{
	struct bmp18x_data* data;
	struct bmp18x_platform_data* pdata = dev->platform_data;

	u8 chip_id = pdata && pdata->chip_id ? pdata->chip_id : BMP18X_CHIP_ID;
	int err = 0;

	printk("bmp18x_probe: platform_data=0x%x\n", (u32)pdata);
	if (pdata)
		printk("bmp18x_probe: chipID = 0x%x\n", pdata->chip_id);

	if (data_bus->bops->read_byte(data_bus->client,
			BMP18X_CHIP_ID_REG) != chip_id)
	{
		printk(KERN_ERR "%s: chip_id failed!\n", BMP18X_NAME);
		err = -ENODEV;
		goto exit;
	}

	data = kzalloc(sizeof(struct bmp18x_data), GFP_KERNEL);
	if (!data)
	{
		err = -ENOMEM;
		goto exit;
	}


	data->data_bus = *data_bus;
	data->dev = dev;
	/* no need to call dev_set_drvdata as it is handle in bmp18x_input_init(input_set_drvdata) */

	/* Initialize the BMP18X chip */
	err = bmp18x_init_client(data, pdata);
	if (err != 0)
		goto exit_free;

	/* Register input polled device */
	err = bmp18x_input_init(dev, data);

	dev_info(dev, "Succesfully initialized bmp18x!\n");
	return 0;

exit_free:
	kfree(data);

exit:

	printk(KERN_ERR "%s FAILED\n", __FUNCTION__);
	return err;
}
EXPORT_SYMBOL(bmp18x_probe);

int bmp18x_remove(struct device *dev)
{
	struct bmp18x_data *data = dev_get_drvdata(dev);
	sysfs_remove_group(&data->input_dev->dev.kobj, &bmp18x_android_attr_group);
	kfree(data);
	return 0;
}
EXPORT_SYMBOL(bmp18x_remove);

#ifdef CONFIG_PM
int bmp18x_disable(struct device *dev)
{
	return 0;
}
EXPORT_SYMBOL(bmp18x_disable);

int bmp18x_enable(struct device *dev)
{
	return 0;
}
EXPORT_SYMBOL(bmp18x_enable);
#endif


MODULE_AUTHOR("Eric Andersson <eric.andersson@unixphere.com>");
MODULE_DESCRIPTION("BMP18X driver");
MODULE_LICENSE("GPL");
