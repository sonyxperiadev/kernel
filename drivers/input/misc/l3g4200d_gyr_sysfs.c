/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
*
* File Name		: l3g4200d_gyr_sysfs.c
* Authors		: MH - C&I BU - Application Team
*			: Carmine Iascone (carmine.iascone@st.com)
*			: Matteo Dameno (matteo.dameno@st.com)
* Version		: V 1.0 sysfs
* Date			: 19/11/2010
* Description		: L3G4200D digital output gyroscope sensor API
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
********************************************************************************
* REVISON HISTORY
*
* VERSION | DATE        | AUTHORS            | DESCRIPTION
*
* 1.0     | 19/11/2010  | Carmine Iascone    | First Release
*
*******************************************************************************/
/*#define DEBUG*/
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/input.h>

#include <linux/l3g4200d_gyr.h>

/** Maximum input-device-reported rot speed value value in dps*/
#define FS_MAX			32768

/* l3g4200d gyroscope registers */
#define WHO_AM_I        0x0F

#define CTRL_REG1       0x20    /* CTRL REG1 */
#define CTRL_REG2       0x21    /* CTRL REG2 */
#define CTRL_REG3       0x22    /* CTRL_REG3 */
#define CTRL_REG4       0x23    /* CTRL_REG4 */
#define CTRL_REG5       0x24    /* CTRL_REG5 */
#define MAX_CTRL_REG_NR 5

/* CTRL_REG1 */
#define PM_OFF		0x00
#define PM_NORMAL	0x08
#define ENABLE_ALL_AXES	0x07
#define BW00		0x00
#define BW01		0x10
#define BW10		0x20
#define BW11		0x30
#define ODR100		0x00  /* ODR = 100Hz */
#define ODR200		0x40  /* ODR = 200Hz */
#define ODR400		0x80  /* ODR = 400Hz */
#define ODR800		0xC0  /* ODR = 800Hz */


#define AXISDATA_REG    0x28

#define FUZZ			0
#define FLAT			0
#define AUTO_INCREMENT		0x80

/*#define SENSITIVITY_250DPS	8.75
#define SENSITIVITY_500DPS	17.50
#define SENSITIVITY_2000DPS	70*/


/** Registers Contents */
#define WHOAMI_L3G4200D		0x00D3	/* Expected content for WAI register*/

/*
 * L3G4200D gyroscope data
 * brief structure containing gyroscope values for yaw, pitch and roll in
 * signed short
 */

struct l3g4200d_xyz {
	short	x,	/* x-axis angular rate data. */
		y,	/* y-axis angluar rate data. */
		z;	/* z-axis angular rate data. */
};

struct output_rate {
	int poll_rate_ms;
	u8 mask;
};

static const struct output_rate odr_table[] = {

	{	2,	ODR800|BW10},
	{	3,	ODR400|BW01},
	{	5,	ODR200|BW00},
	{	10,	ODR100|BW00},
};

struct l3g4200d_data {
	struct i2c_client *client;
	struct l3g4200d_gyr_platform_data *pdata;
	struct mutex lock;
	struct delayed_work input_work;
	struct input_dev *input_dev;
	bool enabled;
	u8 reg_addr;
	u8 resume_state[MAX_CTRL_REG_NR];
	unsigned int poll_interval;
};

static int l3g4200d_i2c_read(struct l3g4200d_data *gyro,
				  u8 *buf, int len)
{
	int err;

	struct i2c_msg msgs[] = {
		{
		 .addr = gyro->client->addr,
		 .flags = gyro->client->flags & I2C_M_TEN,
		 .len = 1,
		 .buf = buf,
		 },
		{
		 .addr = gyro->client->addr,
		 .flags = (gyro->client->flags & I2C_M_TEN) | I2C_M_RD,
		 .len = len,
		 .buf = buf,
		 },
	};

	err = i2c_transfer(gyro->client->adapter, msgs, 2);

	if (err != 2) {
		dev_err(&gyro->client->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int l3g4200d_i2c_write(struct l3g4200d_data *gyro,
						u8 *buf,
						u8 len)
{
	int err;

	struct i2c_msg msgs[] = {
		{
		 .addr = gyro->client->addr,
		 .flags = gyro->client->flags & I2C_M_TEN,
		 .len = len,
		 .buf = buf,
		 },
	};

	err = i2c_transfer(gyro->client->adapter, msgs, 1);

	if (err != 1) {
		dev_err(&gyro->client->dev, "write transfer error\n");
		return -EIO;
	}

	return 0;
}

static int l3g4200d_update_fs_range(struct l3g4200d_data *gyro, u8 new_fs)
{
	int err = 0;
	if (gyro->enabled) {
		u8 buf[2];

		buf[0] = CTRL_REG4;
		err = l3g4200d_i2c_read(gyro, buf, 1);
		if (err)
			return err;

		buf[0] = buf[0] & 0x00CF;
		buf[1] = new_fs|buf[0];
		buf[0] = CTRL_REG4;
		err = l3g4200d_i2c_write(gyro, buf, sizeof(buf));
		if (err)
			return err;
	}
	gyro->resume_state[3] = new_fs;
	return err;
}

static int l3g4200d_update_odr(struct l3g4200d_data *gyro,
				int poll_interval)
{
	int err = 0;
	int i;
	u8 config[2];

	for (i = ARRAY_SIZE(odr_table) - 1; i >= 0; i--) {
		if (odr_table[i].poll_rate_ms <= poll_interval)
			break;
	}

	config[1] = odr_table[i].mask;
	config[1] |= (ENABLE_ALL_AXES + PM_NORMAL);

	if (gyro->enabled) {
		config[0] = CTRL_REG1;
		err = l3g4200d_i2c_write(gyro, config, sizeof(config));
		if (err)
			return err;
	}
	gyro->resume_state[0] = config[1];
	return err;
}

/* gyroscope data readout */
static int l3g4200d_get_data(struct l3g4200d_data *gyro,
			     struct l3g4200d_xyz *data)
{
	int err;
	unsigned char gyro_out[6];
	/* y,p,r hardware data */
	s16 hw_d[3] = { 0 , 0, 0};

	gyro_out[0] = AUTO_INCREMENT | AXISDATA_REG;

	err = l3g4200d_i2c_read(gyro, gyro_out, sizeof(gyro_out));
	if (err)
		return err;

	hw_d[0] = (s16) (((gyro_out[1]) << 8) | gyro_out[0]);
	hw_d[1] = (s16) (((gyro_out[3]) << 8) | gyro_out[2]);
	hw_d[2] = (s16) (((gyro_out[5]) << 8) | gyro_out[4]);

	data->x = gyro->pdata->negate_x ? -hw_d[gyro->pdata->axis_map_x]
		   : hw_d[gyro->pdata->axis_map_x];
	data->y = gyro->pdata->negate_y ? -hw_d[gyro->pdata->axis_map_y]
		   : hw_d[gyro->pdata->axis_map_y];
	data->z = gyro->pdata->negate_z ? -hw_d[gyro->pdata->axis_map_z]
		   : hw_d[gyro->pdata->axis_map_z];

	dev_dbg(&gyro->client->dev, "gyro_out: x = %d y = %d z= %d\n",
		data->x, data->y, data->z);
	return err;
}

static void l3g4200d_report_values(struct l3g4200d_data *gyro,
					struct l3g4200d_xyz *data)
{
	input_report_abs(gyro->input_dev, ABS_X, data->x);
	input_report_abs(gyro->input_dev, ABS_Y, data->y);
	input_report_abs(gyro->input_dev, ABS_Z, data->z);
	input_sync(gyro->input_dev);
}

static void l3g4200d_device_power_off(struct l3g4200d_data *dev_data)
{
	int err;
	u8 buf[2];

	if (!dev_data->enabled)
		return ;
	dev_dbg(&dev_data->client->dev, "%s\n", __func__);

	buf[0] = CTRL_REG1;
	buf[1] = PM_OFF;
	err = l3g4200d_i2c_write(dev_data, buf, sizeof(buf));
	if (err < 0) {
		dev_err(&dev_data->client->dev, "power off failed\n");
		return ;
	}
	if (dev_data->pdata->power_off)
		dev_data->pdata->power_off();

	dev_data->enabled = 0;
}

static int l3g4200d_device_power_on(struct l3g4200d_data *dev_data)
{
	int err = 0;
	u8 buf[6];

	if (dev_data->enabled)
		return err;
	dev_dbg(&dev_data->client->dev, "%s\n", __func__);

	if (dev_data->pdata->power_on) {
		err = dev_data->pdata->power_on();
		if (err < 0) {
			dev_err(&dev_data->client->dev,
				"pdata->power_on() failed\n");
			return err;
		}
	}

	buf[0] = AUTO_INCREMENT | CTRL_REG1;
	buf[1] = dev_data->resume_state[0];
	buf[2] = dev_data->resume_state[1];
	buf[3] = dev_data->resume_state[2];
	buf[4] = dev_data->resume_state[3];
	buf[5] = dev_data->resume_state[4];

	err = l3g4200d_i2c_write(dev_data, buf, sizeof(buf));
	if (err < 0) {
		dev_err(&dev_data->client->dev, "power on failed\n");
		l3g4200d_device_power_off(dev_data);
		return err;
	}
	dev_data->enabled = 1;
	return err;
}

static ssize_t attr_get_polling_rate(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int val;
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);

	mutex_lock(&gyro->lock);
	val = gyro->poll_interval;
	mutex_unlock(&gyro->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t attr_set_polling_rate(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t size)
{
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	unsigned long interval_ms;

	if (strict_strtoul(buf, 10, &interval_ms))
		return -EINVAL;
	if (!interval_ms)
		return -EINVAL;

	mutex_lock(&gyro->lock);
	gyro->poll_interval = interval_ms;
	l3g4200d_update_odr(gyro, interval_ms);
	mutex_unlock(&gyro->lock);

	return size;
}

static ssize_t attr_get_range(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	int range = 0;
	char val;

	mutex_lock(&gyro->lock);
	val = gyro->pdata->fs_range;
	mutex_unlock(&gyro->lock);

	switch (val) {
	case L3G4200D_FS_250DPS:
		range = 250;
		break;
	case L3G4200D_FS_500DPS:
		range = 500;
		break;
	case L3G4200D_FS_2000DPS:
		range = 2000;
		break;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", range);
}

static ssize_t attr_set_range(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t size)
{
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	unsigned long val;
	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	mutex_lock(&gyro->lock);
	gyro->pdata->fs_range = val;
	l3g4200d_update_fs_range(gyro, val);
	mutex_unlock(&gyro->lock);

	return size;
}

static ssize_t attr_reg_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{
	int rc;
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	u8 x[2];
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	mutex_lock(&gyro->lock);
	x[0] = gyro->reg_addr;
	x[1] = val;
	rc = l3g4200d_i2c_write(gyro, x, sizeof(x));
	mutex_unlock(&gyro->lock);

	return size;
}

static ssize_t attr_reg_get(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t ret;
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	int rc;
	u8 data;

	mutex_lock(&gyro->lock);
	data = gyro->reg_addr;
	rc = l3g4200d_i2c_read(gyro, &data, 1);
	mutex_unlock(&gyro->lock);

	ret = snprintf(buf, PAGE_SIZE, "0x%02x\n", data);
	return ret;
}

static ssize_t attr_addr_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct l3g4200d_data *gyro = dev_get_drvdata(dev);
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	mutex_lock(&gyro->lock);
	gyro->reg_addr = val;
	mutex_unlock(&gyro->lock);

	return size;
}

static struct device_attribute attributes[] = {
	__ATTR(pollrate_ms, 0666, attr_get_polling_rate, attr_set_polling_rate),
	__ATTR(range, 0666, attr_get_range, attr_set_range),
	__ATTR(reg_value, 0600, attr_reg_get, attr_reg_set),
	__ATTR(reg_addr, 0200, NULL, attr_addr_set),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;

error:
	for ( ; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s:Unable to create interface\n", __func__);
	return -EIO;
}

static int remove_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
	return 0;
}

static void l3g4200d_input_work_func(struct work_struct *work)
{
	struct l3g4200d_data *gyro = container_of(
			(struct delayed_work *)work,
			struct l3g4200d_data,
			input_work);

	struct l3g4200d_xyz data_out = { 0, 0, 0 };
	int err;

	err = l3g4200d_get_data(gyro, &data_out);
	if (err < 0)
		dev_err(&gyro->client->dev, "get_gyroscope_data failed\n");
	else
		l3g4200d_report_values(gyro, &data_out);

	schedule_delayed_work(&gyro->input_work,
				msecs_to_jiffies(gyro->poll_interval));
}

static int __devinit l3g4200d_validate_pdata(struct l3g4200d_data *gyro)
{
	gyro->pdata->poll_interval = max(gyro->pdata->poll_interval,
			gyro->pdata->min_interval);

	if (gyro->pdata->axis_map_x > 2 ||
	    gyro->pdata->axis_map_y > 2 ||
	    gyro->pdata->axis_map_z > 2) {
		dev_err(&gyro->client->dev,
			"invalid axis_map value x:%u y:%u z%u\n",
			gyro->pdata->axis_map_x,
			gyro->pdata->axis_map_y,
			gyro->pdata->axis_map_z);
		return -EINVAL;
	}

	/* Only allow 0 and 1 for negation boolean flag */
	if (gyro->pdata->negate_x > 1 ||
	    gyro->pdata->negate_y > 1 ||
	    gyro->pdata->negate_z > 1) {
		dev_err(&gyro->client->dev,
			"invalid negate value x:%u y:%u z:%u\n",
			gyro->pdata->negate_x,
			gyro->pdata->negate_y,
			gyro->pdata->negate_z);
		return -EINVAL;
	}

	/* Enforce minimum polling interval */
	if (gyro->pdata->poll_interval < gyro->pdata->min_interval) {
		dev_err(&gyro->client->dev,
			"minimum poll interval violated\n");
		return -EINVAL;
	}
	return 0;
}

int l3g4200d_input_open(struct input_dev *input)
{
	struct l3g4200d_data *gyro = input_get_drvdata(input);
	int err;
	dev_dbg(&gyro->client->dev, "%s\n", __func__);

	mutex_lock(&gyro->lock);
	err = l3g4200d_device_power_on(gyro);
	if (!err)
		schedule_delayed_work(&gyro->input_work,
			msecs_to_jiffies(gyro->poll_interval));
	mutex_unlock(&gyro->lock);
	return 0;
}

void l3g4200d_input_close(struct input_dev *dev)
{
	struct l3g4200d_data *gyro = input_get_drvdata(dev);
	dev_dbg(&gyro->client->dev, "%s\n", __func__);

	mutex_lock(&gyro->lock);
	cancel_delayed_work_sync(&gyro->input_work);
	l3g4200d_device_power_off(gyro);
	mutex_unlock(&gyro->lock);
}

static int __devinit l3g4200d_input_init(struct l3g4200d_data *gyro)
{
	int err;

	INIT_DELAYED_WORK(&gyro->input_work, l3g4200d_input_work_func);
	gyro->input_dev = input_allocate_device();
	if (!gyro->input_dev) {
		dev_err(&gyro->client->dev, "input device allocate failed\n");
		return -ENOMEM;
	}

	gyro->input_dev->open = l3g4200d_input_open;
	gyro->input_dev->close = l3g4200d_input_close;
	gyro->input_dev->name = L3G4200D_DEV_NAME;
	gyro->input_dev->id.bustype = BUS_I2C;
	gyro->input_dev->dev.parent = &gyro->client->dev;

	input_set_drvdata(gyro->input_dev, gyro);

	set_bit(EV_ABS, gyro->input_dev->evbit);

	input_set_abs_params(gyro->input_dev,
		ABS_X, -FS_MAX, FS_MAX, FUZZ, FLAT);
	input_set_abs_params(gyro->input_dev,
		ABS_Y, -FS_MAX, FS_MAX, FUZZ, FLAT);
	input_set_abs_params(gyro->input_dev,
		ABS_Z, -FS_MAX, FS_MAX, FUZZ, FLAT);

	err = input_register_device(gyro->input_dev);
	if (err) {
		dev_err(&gyro->client->dev,
			"unable to register input device %s\n",
			gyro->input_dev->name);
		input_free_device(gyro->input_dev);
		return err;
	}
	return 0;
}

static int __devinit l3g4200d_probe(struct i2c_client *client,
					const struct i2c_device_id *devid)
{
	struct l3g4200d_data *gyro;
	int err = -ENODEV;

	dev_info(&client->dev, "%s\n", __func__);

	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		return err;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "client not i2c capable:1\n");
		return err;
	}

	gyro = kzalloc(sizeof(*gyro), GFP_KERNEL);
	if (gyro == NULL) {
		dev_err(&client->dev, "failed to allocate memory\n");
		return err;
	}

	mutex_init(&gyro->lock);
	gyro->client = client;

	gyro->pdata = kmalloc(sizeof(*gyro->pdata), GFP_KERNEL);
	if (gyro->pdata == NULL) {
		dev_err(&client->dev, "failed to allocate memory\n");
		goto err_gyro_free;
	}
	memcpy(gyro->pdata, client->dev.platform_data, sizeof(*gyro->pdata));

	err = l3g4200d_validate_pdata(gyro);
	if (err < 0) {
		dev_err(&client->dev, "failed to validate platform data\n");
		goto err_pdata_free;
	}

	i2c_set_clientdata(client, gyro);

	if (gyro->pdata->init) {
		err = gyro->pdata->init();
		if (err < 0) {
			dev_err(&client->dev, "init failed: %d\n", err);
			goto err_pdata_free;
		}
	}

	memset(gyro->resume_state, 0, ARRAY_SIZE(gyro->resume_state));

	gyro->resume_state[0] = 0x07;
	gyro->resume_state[1] = 0x00;
	gyro->resume_state[2] = 0x00;
	gyro->resume_state[3] = 0x00;
	gyro->resume_state[4] = 0x00;

	err = l3g4200d_device_power_on(gyro);
	if (err < 0) {
		dev_err(&client->dev, "power on failed: %d\n", err);
		goto err_pdata_exit;
	}

	err = l3g4200d_update_fs_range(gyro, gyro->pdata->fs_range);
	if (err < 0) {
		dev_err(&client->dev, "update_fs_range failed\n");
		goto err_power_off;
	}

	err = l3g4200d_update_odr(gyro, gyro->pdata->poll_interval);
	if (err < 0) {
		dev_err(&client->dev, "update_odr failed\n");
		goto err_power_off;
	}

	err = l3g4200d_input_init(gyro);
	if (err < 0)
		goto err_power_off;

	err = create_sysfs_interfaces(&client->dev);
	if (err < 0) {
		dev_err(&client->dev,
			"%s device register failed\n", L3G4200D_DEV_NAME);
		goto err_unregister_dev;
	}

	l3g4200d_device_power_off(gyro);

	dev_info(&client->dev, "%s completed.\n", __func__);
	return 0;

err_unregister_dev:
	input_unregister_device(gyro->input_dev);
err_power_off:
	l3g4200d_device_power_off(gyro);
err_pdata_exit:
	if (gyro->pdata->exit)
		gyro->pdata->exit();
err_pdata_free:
	kfree(gyro->pdata);
err_gyro_free:
	kfree(gyro);
	return err;
}

static int  __devexit l3g4200d_remove(struct i2c_client *client)
{
	struct l3g4200d_data *gyro = i2c_get_clientdata(client);
	dev_dbg(&client->dev, "%s\n", __func__);

	input_unregister_device(gyro->input_dev);
	l3g4200d_device_power_off(gyro);
	remove_sysfs_interfaces(&client->dev);

	if (gyro->pdata->exit)
		gyro->pdata->exit();
	kfree(gyro->pdata);
	kfree(gyro);
	return 0;
}

#ifdef CONFIG_PM
static int l3g4200d_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct l3g4200d_data *gyro = i2c_get_clientdata(client);
	dev_dbg(&client->dev, "%s\n", __func__);

	if (!mutex_trylock(&gyro->lock))
		return -EAGAIN;

	cancel_delayed_work_sync(&gyro->input_work);
	l3g4200d_device_power_off(gyro);
	mutex_unlock(&gyro->lock);
	return 0;
}

static int l3g4200d_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct l3g4200d_data *gyro = i2c_get_clientdata(client);
	int err;
	dev_dbg(&client->dev, "%s\n", __func__);

	mutex_lock(&gyro->lock);

	if (gyro->input_dev->users) {
		err = l3g4200d_device_power_on(gyro);
		if (err)
			return err;

		schedule_delayed_work(&gyro->input_work,
			msecs_to_jiffies(gyro->poll_interval));
	}
	mutex_unlock(&gyro->lock);
	return 0;
}
#else
#define l3g4200d_suspend NULL
#define l3g4200d_resume NULL
#endif

static const struct i2c_device_id l3g4200d_id[] = {
	{ L3G4200D_DEV_NAME , 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, l3g4200d_id);

static const struct dev_pm_ops l3g4200d_pm = {
	.suspend = l3g4200d_suspend,
	.resume = l3g4200d_resume,
};

static struct i2c_driver l3g4200d_driver = {
	.driver = {
			.owner = THIS_MODULE,
			.name = L3G4200D_DEV_NAME,
#ifdef CONFIG_PM
			.pm = &l3g4200d_pm,
#endif
	},
	.probe = l3g4200d_probe,
	.remove = __devexit_p(l3g4200d_remove),
	.id_table = l3g4200d_id,
};

static int __init l3g4200d_init(void)
{
	return i2c_add_driver(&l3g4200d_driver);
}

static void __exit l3g4200d_exit(void)
{
	i2c_del_driver(&l3g4200d_driver);
}

module_init(l3g4200d_init);
module_exit(l3g4200d_exit);

MODULE_DESCRIPTION("l3g4200d digital gyroscope sysfs driver");
MODULE_AUTHOR("STMicroelectronics");
MODULE_LICENSE("GPL");
