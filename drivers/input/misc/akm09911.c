/*
 * AKM09911 eCompass driver
 *
 * Based on mpu3050.c by:
 *  Joseph Lai <joseph_lai@wistron.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>

#define AKM09911_NAME		"akm09911"
#define AKM09911_CHIP_ID	0x05

#define AKM09911_WHO_AM_I_2_REG	0x01
#define AKM09911_CNTL2_REG	0x31
#define AKM09911_ST1_REG	0x10
#define AKM09911_ST2_REG	0x18
#define AKM09911_DATA_REG	0x11
#define AKM09911_FUSE_ROM_REG	0x60

#define AKM09911_POWER_DOWN_MODE	0x00
#define AKM09911_SINGLE_MEASURE		0x01
#define AKM09911_FUSE_ACCESS		0x1F

#define AKM09911_DRDY_BIT	1

#define AKM09911_AUTO_DELAY	1000

struct axis_data {
	s16 x;
	s16 y;
	s16 z;

	/* status registers */
	u8 st1;
	u8 st2;
};

struct akm09911_sensor {
	struct i2c_client *client;
	struct device *dev;
	struct input_dev *idev;
	struct delayed_work work;

	unsigned long delay;
	u8 rom[3];
};

static ssize_t akm09911_show_rom(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct akm09911_sensor *sensor = i2c_get_clientdata(to_i2c_client(dev));

	return sprintf(buf, "%d %d %d\n", sensor->rom[0], sensor->rom[1],
			sensor->rom[2]);
}

static ssize_t akm09911_show_delay(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct akm09911_sensor *sensor = i2c_get_clientdata(to_i2c_client(dev));

	return sprintf(buf, "%lu\n", sensor->delay);
}

static ssize_t akm09911_store_delay(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct akm09911_sensor *sensor = i2c_get_clientdata(to_i2c_client(dev));
	unsigned long val;
	int error;

	error = kstrtoul(buf, 10, &val);
	if (error)
		return error;

	sensor->delay = val;

	return count;
}

static DEVICE_ATTR(delay, S_IWUSR | S_IRUGO,
		akm09911_show_delay, akm09911_store_delay);
static DEVICE_ATTR(rom, S_IRUGO,
		akm09911_show_rom, NULL);

static struct attribute *akm09911_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_rom.attr,
	NULL
};

static const struct attribute_group akm09911_attr_group = {
	.attrs = akm09911_attributes,
};

/**
 *	akm09911_read_reg	-	read the specified register
 *	@reg: register to read
 *	@buffer: provide register addr and get register
 *	@length: length of register
 *
 *	Reads the register values in one transaction or returns a negative
 *	error code on failure.
 */
static int akm09911_read_reg(struct i2c_client *client,
			     char reg, u8 *buffer, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buffer,
		},
	};

	return i2c_transfer(client->adapter, msg, 2);
}

/**
 *	akm09911_set_power_mode	-	set the power mode
 *	@client: i2c client for the sensor
 *	@val: power mode register value
 *
 *	Put device to normal-power mode or low-power mode.
 */
static int akm09911_set_power_mode(struct i2c_client *client, u8 val)
{
	return i2c_smbus_write_byte_data(client,
					 AKM09911_CNTL2_REG, val & 0x1F);
}

/**
 *	akm09911_read_xyz	-	get co-ordinates from device
 *	@client: i2c address of sensor
 *	@coords: co-ordinates to update
 *
 *	Return the converted X Y and Z co-ordinates from the sensor device
 */
static int akm09911_read_xyz(struct akm09911_sensor *sensor,
			     struct axis_data *coords)
{
	u16 buffer[3];
	int ret;

	ret = akm09911_set_power_mode(sensor->client, AKM09911_SINGLE_MEASURE);
	if (ret < 0)
		return ret;

	msleep(2); /* Measurement period */

	ret = akm09911_read_reg(sensor->client, AKM09911_ST1_REG,
				&coords->st1, 1);
	if (ret < 0)
		return ret;

	if ((coords->st1 & AKM09911_DRDY_BIT) == 0)
		return -EAGAIN;

	ret = akm09911_read_reg(sensor->client, AKM09911_ST2_REG,
				&coords->st2, 1);
	if (ret < 0)
		return ret;

	ret = akm09911_read_reg(sensor->client, AKM09911_DATA_REG,
				(u8 *)buffer, 6);
	if (ret < 0)
		return ret;

	coords->x = le16_to_cpu(buffer[0]);
	coords->y = le16_to_cpu(buffer[1]);
	coords->z = le16_to_cpu(buffer[2]);

	dev_dbg(&sensor->client->dev, "%s: x %d, y %d, z %d\n", __func__,
					coords->x, coords->y, coords->z);

	return 0;
}

/**
 *	akm09911_input_open	-	called on input event open
 *	@input: input dev of opened device
 *
 *	The input layer calls this function when input event is opened. The
 *	function will start the internal worker thread to provide data.
 */
static int akm09911_input_open(struct input_dev *input)
{
	struct akm09911_sensor *sensor = input_get_drvdata(input);

	/* Enable workqueue */
	schedule_delayed_work(&sensor->work, msecs_to_jiffies(sensor->delay));

	pm_runtime_get(sensor->dev);

	return 0;
}

/**
 *	akm09911_input_close	-	called on input event close
 *	@input: input dev of closed device
 *
 *	The input layer calls this function when input event is closed. The
 *	function will cancel the thread and push it to suspend.
 */
static void akm09911_input_close(struct input_dev *input)
{
	struct akm09911_sensor *sensor = input_get_drvdata(input);

	/* Cancel workqueue */
	cancel_delayed_work_sync(&sensor->work);

	pm_runtime_put(sensor->dev);
}

/**
 *	akm09911_work_thread	-	do work
 *	@data: the sensor
 *
 *	Called by the kernel at the set data rate. Read the sensor data and
 *	generate an input event for it.
 */
static void akm09911_work_thread(struct work_struct *work)
{
	struct akm09911_sensor *sensor =
		container_of(work, struct akm09911_sensor, work.work);
	struct axis_data axis;

	if (akm09911_read_xyz(sensor, &axis) < 0)
		return;

	input_event(sensor->idev, EV_MSC, MSC_SERIAL, axis.x);
	input_event(sensor->idev, EV_MSC, MSC_PULSELED, axis.y);
	input_event(sensor->idev, EV_MSC, MSC_GESTURE, axis.z);
	input_event(sensor->idev, EV_MSC, MSC_RAW, axis.st1);
	input_event(sensor->idev, EV_MSC, MSC_SCAN, axis.st2);
	input_sync(sensor->idev);

	schedule_delayed_work(&sensor->work, msecs_to_jiffies(sensor->delay));
}

static int akm09911_init_chip(struct i2c_client *client,
			      struct akm09911_sensor *sensor)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, AKM09911_WHO_AM_I_2_REG);
	if (ret < 0)
		return ret;

	if (ret != AKM09911_CHIP_ID)
		return -ENXIO;

	/* get FUSE rom content */
	ret = akm09911_set_power_mode(client, AKM09911_FUSE_ACCESS);
	if (ret < 0)
		return ret;

	ret = akm09911_read_reg(sensor->client, AKM09911_FUSE_ROM_REG,
				sensor->rom, sizeof(sensor->rom));
	if (ret < 0)
		return ret;

	return akm09911_set_power_mode(client, AKM09911_POWER_DOWN_MODE);
}

/**
 *	akm09911_probe - device detection callback
 *	@client: i2c client of found device
 *	@id: id match information
 *
 *	The I2C layer calls us when it believes a sensor is present at this
 *	address. Probe to see if this is correct and to validate the device.
 *
 *	If present install the relevant sysfs interfaces and input device.
 */
static int akm09911_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct akm09911_sensor *sensor;
	struct input_dev *idev;
	int ret;
	int error;

	sensor = kzalloc(sizeof(struct akm09911_sensor), GFP_KERNEL);
	idev = input_allocate_device();
	if (!sensor || !idev) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	sensor->client = client;
	sensor->dev = &client->dev;
	sensor->idev = idev;

	ret = akm09911_init_chip(client, sensor);
	if (ret < 0) {
		dev_err(&client->dev, "failed to detect device\n");
		error = -ENXIO;
		goto err_free_mem;
	}

	idev->name = AKM09911_NAME;
	/*
	 * Intentionally not setting bus type and parent here
	 * we need this device in /devices/virtual.
	 */
	idev->open = akm09911_input_open;
	idev->close = akm09911_input_close;

	/* EV_MSC to prevent input.c data filtering */
	input_set_capability(idev, EV_MSC, MSC_SERIAL);
	input_set_capability(idev, EV_MSC, MSC_PULSELED);
	input_set_capability(idev, EV_MSC, MSC_GESTURE);

	input_set_drvdata(idev, sensor);

	pm_runtime_set_active(&client->dev);

	error = input_register_device(idev);
	if (error) {
		dev_err(&client->dev, "failed to register input device\n");
		goto err_pm_set_suspended;
	}

	INIT_DELAYED_WORK(&sensor->work, akm09911_work_thread);

	error = sysfs_create_group(&idev->dev.kobj, &akm09911_attr_group);
	if (error)
		goto err_pm_set_suspended;

	pm_runtime_enable(&client->dev);
	pm_runtime_set_autosuspend_delay(&client->dev, AKM09911_AUTO_DELAY);

	return 0;

err_pm_set_suspended:
	pm_runtime_set_suspended(&client->dev);
err_free_mem:
	input_free_device(idev);
	kfree(sensor);
	return error;
}

/**
 *	akm09911_remove	-	remove a sensor
 *	@client: i2c client of sensor being removed
 *
 *	Our sensor is going away, clean up the resources.
 */
static int akm09911_remove(struct i2c_client *client)
{
	struct akm09911_sensor *sensor = i2c_get_clientdata(client);

	pm_runtime_disable(&client->dev);
	pm_runtime_set_suspended(&client->dev);

	input_unregister_device(sensor->idev);
	kfree(sensor);

	return 0;
}

#ifdef CONFIG_PM
/**
 *	akm09911_suspend		-	called on device suspend
 *	@dev: device being suspended
 *
 *	Put the device into sleep mode before we suspend the machine.
 */
static int akm09911_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	(void) akm09911_set_power_mode(client, AKM09911_POWER_DOWN_MODE);

	return 0;
}

/**
 *	akm09911_resume		-	called on device resume
 *	@dev: device being resumed
 *
 *	Put the device into powered mode on resume.
 */
static int akm09911_resume(struct device *dev)
{
	return 0;
}
#endif

static UNIVERSAL_DEV_PM_OPS(akm09911_pm, akm09911_suspend,
			    akm09911_resume, NULL);

static const struct i2c_device_id akm09911_ids[] = {
	{ AKM09911_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, akm09911_ids);

static const struct of_device_id akm09911_of_match[] = {
	{ .compatible = "akm,akm09911", },
	{ },
};
MODULE_DEVICE_TABLE(of, akm09911_of_match);

static struct i2c_driver akm09911_i2c_driver = {
	.driver	= {
		.name	= AKM09911_NAME,
		.owner	= THIS_MODULE,
		.pm	= &akm09911_pm,
		.of_match_table = akm09911_of_match,
	},
	.probe		= akm09911_probe,
	.remove		= akm09911_remove,
	.id_table	= akm09911_ids,
};

module_i2c_driver(akm09911_i2c_driver);

MODULE_AUTHOR("Oskar Andero <oskar.andero@sonymobile.com>");
MODULE_DESCRIPTION("AKM09911 eCompass driver");
MODULE_LICENSE("GPLv2");
