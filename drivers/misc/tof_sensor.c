/*
 * Tof sensor driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/uaccess.h>
#include <linux/kobject.h>

#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include <linux/init.h>
#include <linux/input.h>
#include <linux/regulator/consumer.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/tof_sensor.h>

#define ENABLE_LOGE

#ifdef ENABLE_LOGE
#define LOGE(dev, f, a...)	dev_err(dev, "%s: " f, __func__, ##a)
#else
#define LOGE(dev, f, a...)
#endif

#ifdef ENABLE_LOGI
#define LOGI(dev, f, a...)	dev_info(dev, "%s: " f, __func__, ##a)
#else
#define LOGI(dev, f, a...)
#endif

#ifdef ENABLE_LOGD
#define LOGD(dev, f, a...)	dev_dbg(dev, "%s: " f, __func__, ##a)
#else
#define LOGD(dev, f, a...)
#endif

#define TOF_SENSOR_SYSFS_LINK_NAME "tof_sensor"
#define TOF_SENSOR_PINCTRL_IRQ_ACTIVE "tof_irq_active"
#define TOF_SENSOR_PINCTRL_IRQ_SUSPEND "tof_irq_suspend"

struct tof_sensor_info {
	char name[8];
	uint32_t i2c_client_id;
	uint32_t need_camera_on;
	uint32_t facing;
};

struct tof_sensor_data {
	struct i2c_client *client;
	struct regulator *avdd;
	uint32_t min_voltage;
	uint32_t max_voltage;
	uint8_t power_up;
	struct input_dev *input;
	const char *dev_name;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
	struct tof_sensor_info info;
	uint32_t ref_cnt;
};

static int tof_sensor_power_init(struct tof_sensor_data *data)
{
	int rc = 0;

	if (data->avdd == NULL) {
		data->avdd = regulator_get(&data->client->dev, "tof_avdd");
		if (IS_ERR(data->avdd)) {
			rc = PTR_ERR(data->avdd);
			LOGE(&data->client->dev,
				"Regulator get failed, avdd, rc = %d\n", rc);
			return rc;
		}
		LOGI(&data->client->dev, "%s: power init, regulator get OK",
			__func__);
	}

	return rc;
}

static int tof_sensor_pinctrl_init(struct tof_sensor_data *data)
{

	data->pinctrl = devm_pinctrl_get(&data->client->dev);
	if (IS_ERR_OR_NULL(data->pinctrl)) {
		LOGE(&data->client->dev,
			"%s:%d Getting pinctrl handle failed\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	data->gpio_state_active =
		pinctrl_lookup_state(data->pinctrl,
				TOF_SENSOR_PINCTRL_IRQ_ACTIVE);
	if (IS_ERR_OR_NULL(data->gpio_state_active)) {
		LOGE(&data->client->dev,
			"%s:Failed to get the active state pinctrl handle\n",
			__func__);
		return -EINVAL;
	}
	data->gpio_state_suspend
		= pinctrl_lookup_state(data->pinctrl,
				TOF_SENSOR_PINCTRL_IRQ_SUSPEND);
	if (IS_ERR_OR_NULL(data->gpio_state_suspend)) {
		LOGE(&data->client->dev,
			"%s:Failed to get the suspend state pinctrl handle\n",
			__func__);
		return -EINVAL;
	}
	return 0;
}

static int tof_sensor_parse_dt(struct tof_sensor_data *data)
{
	int rc = 0;
	const char *name = NULL;
	uint32_t val_u32 = 0;

	rc = of_property_read_string_index(data->client->dev.of_node,
		"sony,tof-sensor-name", 0, (const char **)(&name));
	if (rc < 0) {
		LOGE(&data->client->dev, "%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	memcpy(data->info.name, name, 8);

	rc = of_property_read_u32(data->client->dev.of_node,
		"sony,tof-need-cam-on", &val_u32);
	if (rc < 0) {
		LOGE(&data->client->dev, "%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	data->info.need_camera_on = val_u32;

	rc = of_property_read_u32(data->client->dev.of_node,
		"sony,tof-sensor-facing", &val_u32);
	if (rc < 0) {
		LOGE(&data->client->dev, "%s failed %d\n", __func__, __LINE__);
		goto fail;
	}
	data->info.facing = val_u32;

	LOGD(&data->client->dev, "%s name %s, need_camera_on %d, facing %d\n",
		__func__, data->info.name, data->info.need_camera_on,
		data->info.facing);

fail:
	return rc;
}

static irqreturn_t tof_sensor_irq(int irq, void *handle)
{
	struct tof_sensor_data *data = handle;

	LOGI(&data->client->dev, "Tof sensor irq");
	sysfs_notify(&data->input->dev.kobj, NULL, "tof_ranging_notify");

	return IRQ_HANDLED;
}

static ssize_t tof_sensor_power_ctl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_data *data = dev_get_drvdata(dev);
	int rc = 0;
	unsigned long value = 0;

	LOGD(&data->client->dev,
		"%s: Power control data", __func__);
	rc = kstrtoul(buf, sizeof(value), &value);
	if (rc) {
		LOGE(&data->client->dev,
			"%s: covert error. rc = %d", __func__, rc);
		return count;
	}
	if (value == TOF_SENSOR_POWER_CONTROL_ON) {
		/* power on */
		if (!rc && !data->ref_cnt &&
			data->pinctrl && data->gpio_state_active) {
			rc = pinctrl_select_state(data->pinctrl,
				data->gpio_state_active);
			if (rc)
				LOGE(&data->client->dev,
					"%s: cannot set pin to active state",
					__func__);
		}
		if (!rc && !data->ref_cnt) {
			rc = request_threaded_irq(data->client->irq, NULL,
				&tof_sensor_irq,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				dev_name(&data->client->dev), data);
			if (rc) {
				LOGE(&data->client->dev,
					"Failed to request irq %d\n",
					data->client->irq);
				(void)pinctrl_select_state(data->pinctrl,
					data->gpio_state_suspend);
			}
		}
		if (!rc && !data->ref_cnt && !data->info.need_camera_on) {
			rc = regulator_enable(data->avdd);
			if (rc) {
				LOGE(&data->client->dev,
					"Regulator avdd enable failed rc=%d",
					rc);
				return count;
			}
			LOGD(&data->client->dev,
				"%s: power up regulator enable OK",
				__func__);
			usleep_range(3000, 4000);
			data->power_up = 1;
		}
		data->ref_cnt++;
	} else if (value == TOF_SENSOR_POWER_CONTROL_OFF) {
		/* power off */
		if (data->ref_cnt) {
			data->ref_cnt--;
		}
		if (!data->ref_cnt && !data->info.need_camera_on) {
			usleep_range(3000, 4000);
			rc = regulator_disable(data->avdd);
			if (rc) {
				LOGE(&data->client->dev,
					"%s: Regulator avdd disable failed ",
					__func__);
			} else {
				data->power_up = 0;
				LOGD(&data->client->dev,
					"%s: power up regulator disable OK",
					__func__);
			}
		}
		if (!data->ref_cnt &&
			data->pinctrl && data->gpio_state_suspend) {
			free_irq(data->client->irq, data);
			rc = pinctrl_select_state(data->pinctrl,
				data->gpio_state_suspend);
			if (rc)
				LOGE(&data->client->dev,
					"%s: cannot set pin to suspend state",
					__func__);
		}
	} else
		LOGE(&data->client->dev,
			"%s: Power control data error [%ld]", __func__,
			value);

	return count;
}

static ssize_t tof_sensor_show_power_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_data *data = dev_get_drvdata(dev);

	LOGI(dev, "Get power stat is called");

	return snprintf(buf, PAGE_SIZE, "%d\n", data->power_up);
}

static ssize_t tof_sensor_ranging_notify(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	LOGI(dev, "Ranging notify is called");

	return snprintf(buf, PAGE_SIZE, "%d", 1);
}

static ssize_t tof_sensor_get_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_data *data = dev_get_drvdata(dev);

	LOGI(dev, "Get information is called");

	return snprintf(buf, PAGE_SIZE, "%s,%u,%u,%u", data->info.name,
		data->info.i2c_client_id, data->info.need_camera_on,
		data->info.facing);
}

static DEVICE_ATTR(tof_get_info, S_IRUGO, tof_sensor_get_info, NULL);
static DEVICE_ATTR(tof_ranging_notify, S_IRUGO, tof_sensor_ranging_notify,
	NULL);
static DEVICE_ATTR(tof_power_ctl, S_IRUGO | S_IWUSR | S_IWGRP,
	tof_sensor_show_power_status, tof_sensor_power_ctl);
static struct attribute *tof_sensor_attributes[] = {
	&dev_attr_tof_get_info.attr,
	&dev_attr_tof_ranging_notify.attr,
	&dev_attr_tof_power_ctl.attr,
	NULL
};

static const struct attribute_group tof_sensor_attr_group = {
	.attrs = tof_sensor_attributes,
};

static int tof_sensor_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct tof_sensor_data *data;
	int rc = 0;

	LOGI(&client->dev, "start probing stmdata\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LOGE(&client->dev,
			"%s: check_functionality failed.", __func__);
		return -EIO;
	}

	data = kzalloc(sizeof(struct tof_sensor_data), GFP_KERNEL);
	if (!data) {
		LOGE(&client->dev, "failed: no memory to alloc data %p", data);
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);
	data->client = client;
	if (data->client->dev.of_node) {
		rc = tof_sensor_pinctrl_init(data);
		if (rc) {
			LOGE(&client->dev,
				"%s: failed to pinctrl init", __func__);
			goto exit_free_i2c;
		}
		rc = tof_sensor_parse_dt(data);
		if (rc) {
			LOGE(&client->dev,
				"%s: failed to parse dt", __func__);
			goto exit_free_i2c;
		}
	}
	tof_sensor_power_init(data);
	data->ref_cnt = 0;

	data->dev_name = dev_name(&client->dev);
	data->input = input_allocate_device();
	if (!data->input) {
		rc = -ENOMEM;
		LOGE(&client->dev,
			"%s: failed to allocate input device", __func__);
		goto exit_free_i2c;
	}
	data->input->name = "ToF Sensor";
	rc = input_register_device(data->input);
	if (rc) {
		LOGE(&client->dev, "Failed to register input device");
		goto exit_free_dev;
	}
	input_set_drvdata(data->input, data);

	LOGI(&client->dev, "tof sensor: i2c nr%d\n", client->adapter->nr);
	data->info.i2c_client_id = client->adapter->nr;
	data->power_up = 0;

	/* sysfs */
	rc = sysfs_create_group(&data->input->dev.kobj,
		&tof_sensor_attr_group);
	if (rc) {
		rc = -ENOMEM;
		LOGE(&client->dev,
			"%s: failed to create sysfs group", __func__);
		goto exit_unregister_sysfs;
	}

	rc = sysfs_create_link(data->input->dev.kobj.parent,
		&data->input->dev.kobj,
		TOF_SENSOR_SYSFS_LINK_NAME);
	if (rc) {
		rc = -ENOMEM;
		LOGE(&client->dev,
			"%s: failed to create sysfs link", __func__);
		goto exit_unregister_sysfs;
	}
	return rc;

exit_unregister_sysfs:
	input_unregister_device(data->input);
exit_free_dev:
	input_free_device(data->input);
exit_free_i2c:
	i2c_set_clientdata(data->client, NULL);
	kfree(data);

	return rc;
}

static int tof_sensor_remove(struct i2c_client *client)
{
	struct tof_sensor_data *data = i2c_get_clientdata(client);

	sysfs_remove_link(&data->input->dev.kobj,
		TOF_SENSOR_SYSFS_LINK_NAME);
	input_unregister_device(data->input);
	input_free_device(data->input);
	LOGI(&data->client->dev, "Removed.");
	i2c_set_clientdata(data->client, NULL);
	kfree(data);

	return 0;
}

static struct i2c_device_id tof_sensor_id[] = {
	{ "tof_sensor", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, tof_sensor_id);

static struct of_device_id st_tof_sensor_dt_match[] = {
	{ .compatible = "tof_sensor", },
	{ },
};

static struct i2c_driver tof_sensor_i2c_driver = {
	.driver = {
		.name = "tof_sensor",
		.owner = THIS_MODULE,
		.of_match_table = st_tof_sensor_dt_match,
	},
	.probe = tof_sensor_probe,
	.remove = tof_sensor_remove,
	.id_table = tof_sensor_id,
};

static int __init tof_sensor_init(void)
{
	int rc = 0;

	pr_info("%s: Initialize i2c driver", __func__);
	rc = i2c_add_driver(&tof_sensor_i2c_driver);
	pr_info("%s: Added i2c driver rc = %d", __func__, rc);

	return rc;
}

static void __exit tof_sensor_exit(void)
{
	pr_info("Delete i2c driver");
	i2c_del_driver(&tof_sensor_i2c_driver);
}

module_init(tof_sensor_init);
module_exit(tof_sensor_exit);

MODULE_DESCRIPTION("Tof sensor driver");
MODULE_LICENSE("GPL v2");
