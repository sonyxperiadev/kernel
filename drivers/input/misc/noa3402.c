/* kernel/drivers/input/misc/noa3402.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Johan Olson <johan.olson@sonyericsson.com>
 *         Louis Benoit <louis.benoit@onsemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

/* #define DEBUG */

#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/earlysuspend.h>
#include <linux/ctype.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/input/noa3402.h>

#define NOA3402_PART			0x90
#define NOA3402_PART_MASK		0xf0
#define NOA3402_REV_MASK		0x0f

#define PS_INT_HI_MASK			(1 << 1)

#define INT_ACTIVE_LOW			0
#define INT_ALWAYS_UPDATE		(1 << 1)
#define PWM_ENABLE			(1 << 0)
#define PWM_POSITIVE_CYCLE		(1 << 1)
#define VALID_LED_CURRENT_BITS_MASK	0x1F
#define PS_CONFIG_HYST_ENABLE		(1 << 5)
#define PS_CONFIG_HYST_UPPER		(1 << 4)
#define ALS_CONTROL_OFF			0
#define ALS_CONTROL_ONE_SHOT		(1 << 0)
#define ALS_CONTROL_REPEAT		(1 << 1)
#define VALID_ALS_INTEG_TIME_BITS_MASK	0x07
#define PS_CONTROL_OFF			0
#define PS_CONTROL_REPEAT		(1 << 1)

enum active_status {
	ALS_POWER = (1 << 0),
	PS_POWER = (1 << 1),
};

enum noa_regs {
	PART_ID,
	RESET,
	INT_CONFIG,
	PWM_CONFIG,
	PS_LED_CURRENT,
	PS_TH_UP_MSB,
	PS_TH_UP_LSB,
	PS_TH_LO_MSB,
	PS_TH_LO_LSB,
	PS_FILTER_CONFIG,
	PS_CONFIG,
	PS_INTERVAL,
	PS_CONTROL,
	ALS_TH_UP_MSB,
	ALS_TH_UP_LSB,
	ALS_TH_LO_MSB,
	ALS_TH_LO_LSB,
	ALS_FILTER_CONFIG,
	ALS_CONFIG,
	ALS_INTERVAL,
	ALS_CONTROL,
	INTERRUPT,
	PS_DATA_MSB,
	PS_DATA_LSB,
	ALS_DATA_MSB,
	ALS_DATA_LSB,
	NBR_REGS
};

#define NBR_CONFIG_REGS		(ALS_CONTROL - INT_CONFIG + 1)
#define NBR_RESULT_REGS		(PS_DATA_LSB - INTERRUPT + 1)
#define NBR_PS_TH_REGS		4

static const char regmap[] = {
	[PART_ID] =		0x00,
	[RESET] =		0x01,
	[INT_CONFIG] =		0x02,
	[PWM_CONFIG] =		0x03,
	[PS_LED_CURRENT] =	0x0F,
	[PS_TH_UP_MSB] =	0x10,
	[PS_TH_UP_LSB] =	0x11,
	[PS_TH_LO_MSB] =	0x12,
	[PS_TH_LO_LSB] =	0x13,
	[PS_FILTER_CONFIG] =	0x14,
	[PS_CONFIG] =		0x15,
	[PS_INTERVAL] =		0x16,
	[PS_CONTROL] =		0x17,
	[ALS_TH_UP_MSB] =	0x20,
	[ALS_TH_UP_LSB] =	0x21,
	[ALS_TH_LO_MSB] =	0x22,
	[ALS_TH_LO_LSB] =	0x23,
	[ALS_FILTER_CONFIG] =	0x24,
	[ALS_CONFIG] =		0x25,
	[ALS_INTERVAL] =	0x26,
	[ALS_CONTROL] =		0x27,
	[INTERRUPT] =		0x40,
	[PS_DATA_MSB] =		0x41,
	[PS_DATA_LSB] =		0x42,
	[ALS_DATA_MSB] =	0x43,
	[ALS_DATA_LSB] =	0x44
};

struct noa3402_chip {
	struct i2c_client		*client;
	struct input_dev		*input_dev;
	struct dentry			*dir;
	struct mutex			mutex;
	u8				regs[NBR_REGS];
	int				irq;
	enum active_status		active;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend		early_suspend;
#endif
};

static void noa3402_init_register(struct noa3402_chip *chip)
{
	struct noa3402_platform_data *pdata = chip->client->dev.platform_data;

	chip->regs[INT_CONFIG] = INT_ALWAYS_UPDATE | INT_ACTIVE_LOW;
	chip->regs[PWM_CONFIG] = PWM_POSITIVE_CYCLE | PWM_ENABLE |
		pdata->pwm_sensitivity | pdata->pwm_res | pdata->pwm_type;
	chip->regs[PS_LED_CURRENT] =
			pdata->ps_led_current & VALID_LED_CURRENT_BITS_MASK;
	chip->regs[PS_TH_UP_MSB] = 0x02;	/* Configured via sysfs */
	chip->regs[PS_TH_UP_LSB] = 0x00;	/* Configured via sysfs */
	chip->regs[PS_TH_LO_MSB] = 0x01;	/* Configured via sysfs */
	chip->regs[PS_TH_LO_LSB] = 0x00;	/* Configured via sysfs */
	chip->regs[PS_FILTER_CONFIG] =
			((pdata->ps_filter_nbr_correct & 0x0F) << 4) |
			(pdata->ps_filter_nbr_measurements & 0x0F);
	chip->regs[PS_CONFIG] = PS_CONFIG_HYST_ENABLE | PS_CONFIG_HYST_UPPER |
						pdata->ps_integration_time;
	chip->regs[PS_INTERVAL] = pdata->ps_interval;
	chip->regs[PS_CONTROL] = PS_CONTROL_OFF;
	chip->regs[ALS_TH_UP_MSB] = 0xFF;	/* Disabled */
	chip->regs[ALS_TH_UP_LSB] = 0xFF;	/* Disabled */
	chip->regs[ALS_TH_LO_MSB] = 0x00;	/* Disabled */
	chip->regs[ALS_TH_LO_LSB] = 0x00;	/* Disabled */
	chip->regs[ALS_FILTER_CONFIG] = 0x11;	/* Obsolete in Rev D */
	chip->regs[ALS_CONFIG] =
		pdata->als_integration_time & VALID_ALS_INTEG_TIME_BITS_MASK;
	chip->regs[ALS_INTERVAL] = pdata->als_interval;
	chip->regs[ALS_CONTROL] = ALS_CONTROL_REPEAT;
}

static int noa3402_write_config_registers(struct noa3402_chip *chip)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(chip->client, regmap[INT_CONFIG],
				NBR_CONFIG_REGS, &chip->regs[INT_CONFIG]);
	if (ret)
		dev_err(&chip->client->dev, "%s: I2C block write failed "
						"(err = %d)\n", __func__, ret);
	return ret;
}

static int noa3402_power_ctrl(enum active_status a, bool on,
			      struct noa3402_chip *chip)
{
	struct noa3402_platform_data *pdata = chip->client->dev.platform_data;
	bool current_status = !!chip->active;
	int ret = 0;

	if (on)
		chip->active |= a;
	else
		chip->active &= ~a;

	if (current_status != !!chip->active)
		ret = pdata->hw_config(!!chip->active);
	if (ret)
		return ret;

	if (current_status)
		ret = noa3402_write_config_registers(chip);

	return ret;
}

static int noa3402_device_open(struct input_dev *dev)
{
	struct noa3402_chip *chip = input_get_drvdata(dev);
	int ret;

	dev_dbg(&chip->client->dev, "%s\n", __func__);

	mutex_lock(&chip->mutex);
	ret = noa3402_power_ctrl(PS_POWER, true, chip);
	if (ret)
		goto err_power;

	ret = i2c_smbus_write_byte_data(chip->client, regmap[PS_CONTROL],
							PS_CONTROL_REPEAT);
	if (ret)
		goto err_write;
	else
		chip->regs[PS_CONTROL] = PS_CONTROL_REPEAT;
	goto exit;

err_write:
	(void)noa3402_power_ctrl(PS_POWER, false, chip);
err_power:
	dev_err(&chip->client->dev, "%s. Failed to activate device, err = %d\n",
								__func__, ret);
exit:
	mutex_unlock(&chip->mutex);
	return ret;
}

static void noa3402_device_close(struct input_dev *dev)
{
	struct noa3402_chip *chip = input_get_drvdata(dev);
	int ret;

	dev_dbg(&chip->client->dev, "%s\n", __func__);
	mutex_lock(&chip->mutex);
	ret = i2c_smbus_write_byte_data(chip->client, regmap[PS_CONTROL],
								PS_CONTROL_OFF);
	if (ret)
		dev_err(&chip->client->dev, "%s. Failed to deactivate PS, ret ="
							" %d\n", __func__, ret);
	else
		chip->regs[PS_CONTROL] = PS_CONTROL_OFF;

	(void)noa3402_power_ctrl(PS_POWER, false, chip);
	mutex_unlock(&chip->mutex);
}

static ssize_t noa3402_ps_threshold_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t count)
{
	struct noa3402_chip *chip = dev_get_drvdata(dev);
	u16 hi = 0;
	u16 lo = 0;
	int ret;

	if (sscanf(buf, "%4hx %4hx", &hi, &lo) != 2) {
		dev_err(dev, "%s: Param error\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&chip->mutex);
	chip->regs[PS_TH_UP_MSB] = (hi & 0xFF00) >> 8;
	chip->regs[PS_TH_UP_LSB] = hi & 0xFF;
	chip->regs[PS_TH_LO_MSB] = (lo & 0xFF00) >> 8;
	chip->regs[PS_TH_LO_LSB] = lo & 0xFF;
	ret = i2c_smbus_write_i2c_block_data(chip->client, regmap[PS_TH_UP_MSB],
				NBR_PS_TH_REGS, &(chip->regs[PS_TH_UP_MSB]));
	mutex_unlock(&chip->mutex);
	ret = strnlen(buf, count);
	return ret;
}

static ssize_t noa3402_als_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct noa3402_chip *chip = dev_get_drvdata(dev);
	u8 data[2];
	int ret;

	mutex_lock(&chip->mutex);
	if (chip->active & ALS_POWER) {
		ret = i2c_smbus_read_i2c_block_data(chip->client,
				regmap[ALS_DATA_MSB], sizeof(data), data);
		if (ret == 2)
			ret = snprintf(buf, PAGE_SIZE, "%.2hhx%.2hhx\n",
							data[0], data[1]);
		else
			dev_err(&chip->client->dev, "%s: failed to read ALS "
					"data, ret = %d\n", __func__, ret);
	} else {
		ret = -EPERM;
	}

	mutex_unlock(&chip->mutex);
	return ret;
}

static struct device_attribute attributes[] = {
	__ATTR(ps_threshold, 0200, NULL, noa3402_ps_threshold_store),
	__ATTR(als, 0444, noa3402_als_show, NULL),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;
error:
	for (; i >= 0 ; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

static int noa3402_detect_device(struct i2c_client *client, u8 *revision)
{
	int ret = 0;

	ret = i2c_smbus_read_byte_data(client, regmap[PART_ID]);
	if (ret < 0) {
		dev_err(&client->dev, "NOA3402 not responding\n");
		goto out;
	}

	if ((ret & NOA3402_PART_MASK) == NOA3402_PART) {
		dev_dbg(&client->dev, "%s: NOA3402 detected and responding\n",
								__func__);
		*revision = ret & NOA3402_REV_MASK;
		ret = 0;
	} else {
		dev_err(&client->dev, "%s: NOA3402 not found (id = 0x%x)\n",
								__func__, ret);
		ret = -ENODEV;
	}
out:
	return ret;
}

static irqreturn_t noa3402_irq(int irq, void *data)
{
	struct noa3402_chip *chip = data;
	u8 reg_data[NBR_RESULT_REGS];
	int ret;

	mutex_lock(&chip->mutex);
	if (!(chip->active & PS_POWER)) {
		dev_err(&chip->client->dev,
			"%s: IRQ received, but chip not active\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_read_i2c_block_data(chip->client, regmap[INTERRUPT],
						NBR_RESULT_REGS, reg_data);
	if (ret != NBR_RESULT_REGS) {
		dev_err(&chip->client->dev,
			"%s: IRQ received, but failed to read regs, ret = %d\n",
			__func__, ret);
		goto exit;
	}

	input_report_abs(chip->input_dev, ABS_DISTANCE,
					reg_data[0] & PS_INT_HI_MASK ? 0 : 255);
	input_sync(chip->input_dev);

	if (reg_data[0] & PS_INT_HI_MASK)
		dev_dbg(&chip->client->dev, "%s: Proximity HIGH\n", __func__);
	else
		dev_dbg(&chip->client->dev, "%s: Proximity LOW\n", __func__);
exit:
	mutex_unlock(&chip->mutex);
	return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void noa3402_early_suspend(struct early_suspend *data)
{
	struct noa3402_chip *chip =
			container_of(data, struct noa3402_chip, early_suspend);

	dev_dbg(&chip->client->dev, "%s\n", __func__);
	mutex_lock(&chip->mutex);
	if (i2c_smbus_write_byte_data(chip->client, regmap[PWM_CONFIG],
				chip->regs[PWM_CONFIG] & ~PWM_ENABLE))
		dev_err(&chip->client->dev, "%s. Failed to suspend PWM\n",
								__func__);
	else
		chip->regs[PWM_CONFIG] &= ~PWM_ENABLE;

	/* Workaround due to PWM bug we need one extra (quick) measurement to
	make sure PWM goes low. */
	if (i2c_smbus_write_byte_data(chip->client, regmap[ALS_CONFIG],
						ALS_INTEGRATION_6_25_MS))
		dev_err(&chip->client->dev,
			"%s. Failed to lower ALS integration time\n", __func__);
	else
		chip->regs[ALS_CONFIG] = ALS_INTEGRATION_6_25_MS;

	if (i2c_smbus_write_byte_data(chip->client, regmap[ALS_CONTROL],
							ALS_CONTROL_ONE_SHOT))
		dev_err(&chip->client->dev, "%s. Failed to suspend ALS\n",
								__func__);
	else
		/* Register is automatically set to OFF when one shot
		measurement is done */
		chip->regs[ALS_CONTROL] = ALS_CONTROL_OFF;

	(void)noa3402_power_ctrl(ALS_POWER, false, chip);
	mutex_unlock(&chip->mutex);
}

static void noa3402_late_resume(struct early_suspend *data)
{
	struct noa3402_chip *chip =
			container_of(data, struct noa3402_chip, early_suspend);
	struct noa3402_platform_data *pdata = chip->client->dev.platform_data;

	dev_dbg(&chip->client->dev, "%s\n", __func__);
	mutex_lock(&chip->mutex);
	if (noa3402_power_ctrl(ALS_POWER, true, chip)) {
		dev_err(&chip->client->dev, "%s. Failed to power on\n",
								__func__);
	} else {
		/* PWM workaround needs to restore ALS integration time */
		if (i2c_smbus_write_byte_data(chip->client, regmap[ALS_CONFIG],
					pdata->als_integration_time &
					VALID_ALS_INTEG_TIME_BITS_MASK))
			dev_err(&chip->client->dev,
				"%s. Failed to lower ALS integration time\n",
				__func__);
		else
			chip->regs[ALS_CONTROL] = pdata->als_integration_time &
						VALID_ALS_INTEG_TIME_BITS_MASK;

		if (i2c_smbus_write_byte_data(chip->client, regmap[ALS_CONTROL],
							ALS_CONTROL_REPEAT))
			dev_err(&chip->client->dev, "%s. Failed to resume ALS"
								"\n", __func__);
		else
			chip->regs[ALS_CONTROL] = ALS_CONTROL_REPEAT;

		if (i2c_smbus_write_byte_data(chip->client, regmap[PWM_CONFIG],
					chip->regs[PWM_CONFIG] | PWM_ENABLE))
			dev_err(&chip->client->dev, "%s. Failed to resume PWM"
								"\n", __func__);
		else
			chip->regs[PWM_CONFIG] |= PWM_ENABLE;
	}
	mutex_unlock(&chip->mutex);
}
#endif

static int __devinit noa3402_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct noa3402_chip *chip;
	struct noa3402_platform_data *pdata = client->dev.platform_data;
	int ret;
	u8 revision = 0;

	dev_dbg(&client->dev, "%s\n", __func__);

	if (!pdata || !pdata->gpio_setup || !pdata->hw_config) {
		dev_err(&client->dev, "%s: platform data is not complete.\n",
			__func__);
		ret = -ENODEV;
		goto exit;
	}

	ret = pdata->gpio_setup(1);
	if (ret) {
		dev_err(&client->dev, "%s: gpio_setup failed\n", __func__);
		goto exit;
	}

	ret = pdata->hw_config(1);
	if (ret) {
		dev_err(&client->dev, "%s: power on failed\n", __func__);
		goto exit_free_gpio;
	}

	ret = noa3402_detect_device(client, &revision);
	if (ret) {
		dev_err(&client->dev, "%s: device not responding"
						" ret = %d\n", __func__, ret);
		ret = -ENODEV;
		goto exit_power_off;
	}

	chip = kzalloc(sizeof(struct noa3402_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto exit_power_off;
	}
	i2c_set_clientdata(client, chip);
	mutex_init(&chip->mutex);
	chip->irq = gpio_to_irq(pdata->gpio);
	if (chip->irq < 0) {
		dev_err(&client->dev, "%s: gpio_to_irq failed\n", __func__);
		ret = chip->irq;
		goto exit_free_data;
	}

	chip->client = client;
	noa3402_init_register(chip);
	ret = request_threaded_irq(chip->irq, NULL, noa3402_irq,
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, NOA3402_NAME, chip);
	if (ret < 0) {
		dev_err(&client->dev, "%s: request_irq failed\n", __func__);
		goto exit_free_data;
	}
	ret = irq_set_irq_wake(chip->irq, pdata->is_irq_wakeup);
	if (ret) {
		dev_err(&client->dev, "%s: irq_set_irq_wake failed\n", __func__);
		goto exit_free_irq;
	}
	ret = create_sysfs_interfaces(&client->dev);
	if (ret)
		goto exit_free_irq;

#ifdef CONFIG_HAS_EARLYSUSPEND
	chip->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	chip->early_suspend.suspend = noa3402_early_suspend;
	chip->early_suspend.resume = noa3402_late_resume;
	register_early_suspend(&chip->early_suspend);
#endif

	chip->input_dev = input_allocate_device();
	if (!chip->input_dev) {
		dev_err(&client->dev, "%s: input_allocate_device failed\n",
								__func__);
		goto exit_free_irq;
	}
	input_set_drvdata(chip->input_dev, chip);
	chip->input_dev->open = noa3402_device_open;
	chip->input_dev->close = noa3402_device_close;
	chip->input_dev->name = NOA3402_NAME;
	chip->input_dev->phys = pdata->phys_dev_path;
	chip->input_dev->id.product = NOA3402_PART;
	chip->input_dev->id.version = revision;
	chip->input_dev->id.bustype = BUS_I2C;
	set_bit(EV_ABS, chip->input_dev->evbit);
	set_bit(ABS_DISTANCE, chip->input_dev->absbit); /* For PS irq */
	input_set_abs_params(chip->input_dev, ABS_DISTANCE, 0, 255, 0, 0);
	ret = input_register_device(chip->input_dev);
	if (ret) {
		dev_err(&client->dev, "%s: input_register_device failed\n",
								__func__);
		input_free_device(chip->input_dev);
		goto exit_free_irq;
	}

	ret = noa3402_power_ctrl(ALS_POWER, true, chip);
	if (ret)
		goto exit_unregister;
	ret = noa3402_write_config_registers(chip);
	if (ret)
		goto exit_unregister;

	dev_info(&client->dev, "%s: NOA3402 probed successfully.\n", __func__);
	return 0;

exit_unregister:
	input_unregister_device(chip->input_dev);
exit_free_irq:
	free_irq(chip->irq, chip);
exit_free_data:
	i2c_set_clientdata(client, NULL);
	kfree(chip);
exit_power_off:
	(void)pdata->hw_config(0);
exit_free_gpio:
	(void)pdata->gpio_setup(0);
exit:
	dev_err(&client->dev, "%s: Probing NOA3402 failed\n", __func__);
	return ret;
}

static int noa3402_remove(struct i2c_client *client)
{
	struct noa3402_platform_data *pdata = client->dev.platform_data;
	struct noa3402_chip *chip = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&chip->early_suspend);
#endif
	remove_sysfs_interfaces(&client->dev);
	free_irq(chip->irq, chip);
	input_unregister_device(chip->input_dev);
	pdata->hw_config(0);
	pdata->gpio_setup(0);
	i2c_set_clientdata(client, NULL);
	kfree(chip);
	return 0;
}

static int __devinit noa3402_detect(struct i2c_client *client,
					struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;
	return 0;
}

static const struct i2c_device_id noa3402_id[] = {
	{NOA3402_NAME, 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, noa3402_id);

static struct i2c_driver noa3402_driver = {
	.driver	 = {
		.name  = NOA3402_NAME,
		.owner	= THIS_MODULE,
	},
	.class  = I2C_CLASS_HWMON,
	.probe  = noa3402_probe,
	.detect = noa3402_detect,
	.remove	= noa3402_remove,
	.id_table = noa3402_id,
};

static int __init noa3402_init(void)
{
	return i2c_add_driver(&noa3402_driver);
}

static void __exit noa3402_exit(void)
{
	i2c_del_driver(&noa3402_driver);
}

MODULE_DESCRIPTION("NOA3402 combined ALS and proximity sensor");
MODULE_AUTHOR("johan.olson@sonyericsson.com");
MODULE_LICENSE("GPL v2");

module_init(noa3402_init);
module_exit(noa3402_exit);
