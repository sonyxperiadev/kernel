/*
 * This file is part of the AL3006 sensor driver.
 * Chip is combined proximity and ambient light sensor.
 *
 * Copyright (c) 2011 Liteon-semi Corporation
 *
 * Contact: YC Hou <yc_hou@liteon-semi.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 * Filename: al3006.c
 *
 * Summary:
 *	AL3006 sensor dirver for kernel version 2.6.36.
 *
 * Modification History:
 * Date     By       Summary
 * -------- -------- -------------------------------------------------------
 * 06/13/11 YC		 Original Creation (Test version:1.0)
 * 06/15/11 YC		 Add device attribute to open controled interface.
 * 06/22/11 YC       Add INT function and handler.
 * 06/28/11 YC		 Add proximity attributes.
 * 06/28/11 YC       Change dev name to dyna for demo purpose (ver 1.5).
 * 08/23/11 YC       Add engineer mode. Change version to 1.6.
 * 10/04/11 YC       Add calibration compensation function and add not power up
 *                   prompt. Change version to 1.7.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/string.h>

#ifdef CONFIG_BRCM_VIRTUAL_SENSOR
#include <linux/brvsens_driver.h>

static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);
#endif

#define AL3006_DRV_NAME	"al3006"
//#define AL3006_DRV_NAME		"dyna"
#define DRIVER_VERSION		"1.7"

#define AL3006_NUM_CACHABLE_REGS	7

#define AL3006_RES_COMMAND	0x05
#define AL3006_RES_SHIFT	(0)
#define AL3006_RES_MASK		0x3f

#define AL3006_MODE_COMMAND	0x00
#define AL3006_MODE_SHIFT	(0)
#define AL3006_MODE_MASK	0x03

#define AL3006_POW_MASK		0x0c
#define AL3006_POW_UP		0x00
#define AL3006_POW_DOWN		0x02
#define AL3006_POW_SHIFT	0x02

#define AL3006_OBJ_MASK		0x80
#define AL3006_OBJ_SHIFT	(7)

#define AL3006_INT_COMMAND	0x03
#define AL3006_INT_SHIFT	(0)
#define AL3006_INT_MASK		0x03

#define AL3006_TIME_CTRL_COMMAND	0x01
#define AL3006_TIME_CTRL_SHIFT	(0)
#define AL3006_TIME_CTRL_MASK	0x7F

#define LSC_DBG
#ifdef LSC_DBG
#define LDBG(s,args...)	{printk("LDBG: func [%s], line [%d], ",__func__,__LINE__); printk(s,## args);}
#else
#define LDBG(s,args...) {}
#endif

struct al3006_data {
	struct i2c_client *client;
	struct mutex lock;
	u8 reg_cache[AL3006_NUM_CACHABLE_REGS];
//	u8 power_state_before_suspend;
	int irq;
	struct input_dev *input;
};

static u8 al3006_reg[AL3006_NUM_CACHABLE_REGS] =
	{0x00,0x01,0x02,0x03,0x04,0x05,0x08};

static int lux_table[64]={1,1,1,2,2,2,3,4,4,5,6,7,9,11,13,16,19,22,27,32,39,46,56,67,80,96,
                          116,139,167,200,240,289,346,416,499,599,720,864,1037,1245,1495,
                          1795,2154,2586,3105,3728,4475,5372,6449,7743,9295,11159,13396,
                          16082,19307,23178,27826,33405,40103,48144,57797,69386,83298,
                          100000};

int cali = 0;

/*
 * register access helpers
 */

static int __al3006_read_reg(struct i2c_client *client,
			       u32 reg, u8 mask, u8 shift)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	return (data->reg_cache[reg] & mask) >> shift;
}

static int __al3006_write_reg(struct i2c_client *client,
				u32 reg, u8 mask, u8 shift, u8 val)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int ret = 0;
	u8 tmp;

	if (reg >= AL3006_NUM_CACHABLE_REGS)
		return -EINVAL;

	mutex_lock(&data->lock);

	tmp = data->reg_cache[reg];
	tmp &= ~mask;
	tmp |= val << shift;

	ret = i2c_smbus_write_byte_data(client, reg, tmp);
	if (!ret)
		data->reg_cache[reg] = tmp;

	mutex_unlock(&data->lock);
	return ret;
}

/*
 * internally used functions
 */

/* range */
static int al3006_get_range(struct i2c_client *client)
{
	return 100000;
}


/* mode */
static int al3006_get_mode(struct i2c_client *client)
{
	return __al3006_read_reg(client, AL3006_MODE_COMMAND,
		AL3006_MODE_MASK, AL3006_MODE_SHIFT);
}

static int al3006_set_mode(struct i2c_client *client, int mode)
{
	return __al3006_write_reg(client, AL3006_MODE_COMMAND,
		AL3006_MODE_MASK, AL3006_MODE_SHIFT, mode);
}

/* power_state */
static int al3006_set_power_state(struct i2c_client *client, int state)
{
	return __al3006_write_reg(client, AL3006_MODE_COMMAND,
				AL3006_POW_MASK, AL3006_POW_SHIFT,
				state ? AL3006_POW_UP : AL3006_POW_DOWN);
}

static int al3006_get_power_state(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	u8 cmdreg = data->reg_cache[AL3006_MODE_COMMAND];
	return (cmdreg & AL3006_POW_MASK) >> AL3006_POW_SHIFT;
}

static int al3006_get_adc_value(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_RES_COMMAND);
	val &= AL3006_RES_MASK;

	if (val < 0) {
		mutex_unlock(&data->lock);
		return val;
	}

	mutex_unlock(&data->lock);

	if ((val + cali) > 63 || ((val + cali) < 0))
		cali = 0;

	return lux_table[(val + cali)];
}

static int al3006_get_object(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_RES_COMMAND);
	val &= AL3006_OBJ_MASK;

	mutex_unlock(&data->lock);

	return val >> AL3006_OBJ_SHIFT;
}

static int al3006_get_intstat(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_INT_COMMAND);
	val &= AL3006_INT_MASK;

	mutex_unlock(&data->lock);

	return val >> AL3006_INT_SHIFT;
}

/*
 * sysfs layer
 */
static int al3006_input_init(struct al3006_data *data)
{
    struct input_dev *dev;
    int err;

    dev = input_allocate_device();
    if (!dev) {
        return -ENOMEM;
    }
    dev->name = "LSC_al3006";
    dev->id.bustype = BUS_I2C;

    input_set_capability(dev, EV_ABS, ABS_MISC);
    input_set_capability(dev, EV_ABS, ABS_RUDDER);
    input_set_drvdata(dev, data);

    err = input_register_device(dev);
    if (err < 0) {
        input_free_device(dev);
		LDBG("input device register error! ret = [%d]\n", err)
        return err;
    }
    data->input = dev;

    return 0;
}

static void al3006_input_fini(struct al3006_data *data)
{
    struct input_dev *dev = data->input;

    input_unregister_device(dev);
    input_free_device(dev);
}

/* range */
static ssize_t al3006_show_range(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%i\n", al3006_get_range(data->client));
}

static DEVICE_ATTR(range, S_IRUGO, al3006_show_range, NULL);


/* mode */
static ssize_t al3006_show_mode(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_mode(data->client));
}

static ssize_t al3006_store_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 2))
		return -EINVAL;

	ret = al3006_set_mode(data->client, val);
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(mode, S_IWUSR | S_IRUGO,
		   al3006_show_mode, al3006_store_mode);


/* power state */
static ssize_t al3006_show_power_state(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_power_state(data->client));
}

static ssize_t al3006_store_power_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 1))
		return -EINVAL;

	ret = al3006_set_power_state(data->client, val);
	return ret ? ret : count;
}

static DEVICE_ATTR(power_state, S_IWUSR | S_IRUGO,
		   al3006_show_power_state, al3006_store_power_state);


/* lux */
static ssize_t al3006_show_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	/* No LUX data if not operational */
	if (al3006_get_power_state(data->client) == 0x02)
		return -EBUSY;

	return sprintf(buf, "%d\n", al3006_get_adc_value(data->client));
}

static DEVICE_ATTR(lux, S_IRUGO, al3006_show_lux, NULL);


/* proximity object detect */
static ssize_t al3006_show_object(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_object(data->client));
}

static DEVICE_ATTR(object, S_IRUGO, al3006_show_object, NULL);


/* calibration */
static ssize_t al3006_show_calibration_state(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d\n", cali);
}

static ssize_t al3006_store_calibration_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);
	int stdls, val;
	char tmp[10];
	u8 i = 63;

	/* No LUX data if not operational */
	if (al3006_get_power_state(data->client) != 0x00)
	{
		printk("Please power up first!");
		return -EINVAL;
	}

	cali = 0;

	sscanf(buf, "%d %s", &stdls, tmp);
	if (!strncmp(tmp, "-setcv", 6))
	{
		cali = stdls;
		return -EBUSY;
	}

	if (stdls < 0)
	{
		printk("Std light source: [%d] < 0 !!! Check again, please.\n", stdls);
		printk("Set calibration factor to 0.\n");
		return -EBUSY;
	}

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	val &= AL3006_RES_MASK;

	if (val < 0) {
		mutex_unlock(&data->lock);
		cali = 0;
		return -EBUSY;
	}
	mutex_unlock(&data->lock);


	while((stdls < lux_table[i]) && (i >= 0))
	{
		LDBG("[%d] < lux_table[%d] = %d \n", stdls, i, lux_table[i])
		i--;
	};

	LDBG("std cnt[%d], real cnt[%d]\n", i, val)

	cali = ++i - val;
	LDBG("calibration factor [%d]\n", cali)
	return -EBUSY;
}

static DEVICE_ATTR(calibration, S_IWUSR | S_IRUGO,
		   al3006_show_calibration_state, al3006_store_calibration_state);


#ifdef LSC_DBG
/* engineer mode */
static ssize_t al3006_em_read(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	int i;
	u8 tmp;

	for (i = 0; i < ARRAY_SIZE(data->reg_cache); i++)
		{
		mutex_lock(&data->lock);
		tmp = i2c_smbus_read_byte_data(data->client, al3006_reg[i]);
		mutex_unlock(&data->lock);

		printk("Reg[0x%x] Val[0x%x]\n", al3006_reg[i], tmp);
	}
	return 0;
}

static ssize_t al3006_em_write(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	u32 addr,val;
	int ret = 0;

	sscanf(buf, "%x%x", &addr, &val);

	printk("Write [%x] to Reg[%x]...\n",val,addr);
	mutex_lock(&data->lock);

	ret = i2c_smbus_write_byte_data(client, addr, val);
	if (!ret)
		data->reg_cache[addr] = val;

	mutex_unlock(&data->lock);

	return count;
}
static DEVICE_ATTR(em, S_IWUSR |S_IRUGO,
				   al3006_em_read, al3006_em_write);
#endif

static struct attribute *al3006_attributes[] = {
	&dev_attr_range.attr,
	&dev_attr_mode.attr,
	&dev_attr_power_state.attr,
	&dev_attr_lux.attr,
	&dev_attr_object.attr,
	&dev_attr_calibration.attr,
#ifdef LSC_DBG
	&dev_attr_em.attr,
#endif
	NULL
};

static const struct attribute_group al3006_attr_group = {
	.attrs = al3006_attributes,
};

#ifdef CONFIG_BRCM_VIRTUAL_SENSOR
static int al3006_set_mode_no_lock(struct al3006_data *data, u8 mode)
{
	int ret;
	u8 val = data->reg_cache[AL3006_MODE_COMMAND];
	val &= ~AL3006_MODE_MASK;
	val |= mode << AL3006_MODE_SHIFT;

	ret = i2c_smbus_write_byte_data(data->client, AL3006_MODE_COMMAND, val);

	if(mod_debug)
		printk("%s() reg:0x%x mode:0x%x new_reg:0x%x return:%d\n", __func__, data->reg_cache[AL3006_MODE_COMMAND], mode, val, ret);

	if(!ret)
		data->reg_cache[AL3006_MODE_COMMAND] = val;

	return ret;
}

static int al3006_activate(struct al3006_data *data, u8 flag)
{
	u8 reg, pwr;
	int ret;

	if(mod_debug)
		printk("%s() %s reg:0x%x\n", __func__, flag?"UP":"DOWN", data->reg_cache[AL3006_MODE_COMMAND]);

	mutex_lock(&data->lock);
	reg = data->reg_cache[AL3006_MODE_COMMAND];
	pwr = (reg & AL3006_POW_MASK) >> AL3006_POW_SHIFT;

	if(flag) /* activate */
	{
		if(pwr != AL3006_POW_UP)
		{
			reg &= ~AL3006_POW_MASK;
			reg |= AL3006_POW_UP << AL3006_POW_SHIFT;
		}
		reg &= ~AL3006_MODE_MASK;
		reg |= 0x03 << AL3006_MODE_SHIFT;
	}
	else /* deactivate */
	{
		if(pwr != AL3006_POW_DOWN)
		{
			reg &= ~AL3006_POW_MASK;
			reg |= AL3006_POW_DOWN << AL3006_POW_SHIFT;
		}
		reg &= ~AL3006_MODE_MASK;
		reg |= 0x03 << AL3006_MODE_SHIFT;
	}

	ret = i2c_smbus_write_byte_data(data->client, AL3006_MODE_COMMAND, reg);
	if (!ret)
		data->reg_cache[AL3006_MODE_COMMAND] = reg;

	mutex_unlock(&data->lock);

	if(ret)
		printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, ret);

	if(mod_debug)
		printk("%s() new_reg:0x%x return=%d\n", __func__, reg, ret);

	return ret;
}

static int al3006_read_als(struct al3006_data *data, u16* value)
{
	int val;

	mutex_lock(&data->lock);
	if(((data->reg_cache[AL3006_MODE_COMMAND] & AL3006_MODE_MASK) >> AL3006_MODE_SHIFT) != 0x00)
	{
		if(val = al3006_set_mode_no_lock(data, 0x00))
		{
			mutex_unlock(&data->lock);
			printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, val);
			return val;
		}
	}

	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	mutex_unlock(&data->lock);

	if(val < 0)
	{
		printk("%s() I2C ERROR %d\n", __func__, val);
		return val;
	}

	val &= AL3006_RES_MASK;
	if((val + cali) > 63 || ((val + cali) < 0))
		cali = 0;

	*value = lux_table[(val + cali)];

	if(mod_debug)
		printk("%s() val=%d cali=%d value=%d\n", __func__, val, cali, *value);
	return 0;
}

static int al3006_read_ps(struct al3006_data *data, u32* value)
{
	int val;

	mutex_lock(&data->lock);
	if(((data->reg_cache[AL3006_MODE_COMMAND] & AL3006_MODE_MASK) >> AL3006_MODE_SHIFT) != 0x01)
	{
		if(val = al3006_set_mode_no_lock(data, 0x01))
		{
			mutex_unlock(&data->lock);
			printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, val);
			return val;
		}
	}

	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	mutex_unlock(&data->lock);

	if(val < 0)
	{
		printk("%s() I2C ERROR %d\n", __func__, val);
		return val;
	}

	*value = (val & AL3006_OBJ_MASK) >> AL3006_OBJ_SHIFT;

	if(mod_debug)
		printk("%s() value=%d %s\n", __func__, *value, *value ? "obj near":"obj far");
	return 0;
}
#endif

static int al3006_init_client(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int i;

	/* read all the registers once to fill the cache.
	 * if one of the reads fails, we consider the init failed */
	for (i = 0; i < ARRAY_SIZE(data->reg_cache); i++) {
		int v = i2c_smbus_read_byte_data(client, al3006_reg[i]);
		if (v < 0)
			return -ENODEV;

		data->reg_cache[i] = v;
	}

	/* set defaults */
	al3006_set_mode(client, 0);
	al3006_set_power_state(client, 0);

	/* set sensor responsiveness to fast (516 ms for the first read, 100ms afterward) */
	__al3006_write_reg(client, AL3006_TIME_CTRL_COMMAND, AL3006_TIME_CTRL_MASK, AL3006_TIME_CTRL_SHIFT, 0x10);

	return 0;
}

/*
 * I2C layer
 */

static irqreturn_t al3006_irq(int irq, void *data_)
{
	struct al3006_data *data = data_;
	u8 mode, int_stat;
	int Aval, Pval, tmp;

	mode = al3006_get_mode(data->client);
	int_stat = al3006_get_intstat(data->client);

	mutex_lock(&data->lock);

	tmp = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	Aval = tmp & AL3006_RES_MASK;
	Pval = (tmp & AL3006_OBJ_MASK) >> AL3006_OBJ_SHIFT;

	if ((int_stat == 1) || (mode == 0))
		printk("ALS lux value: %d\n", Aval);
	else
	{
		if (int_stat == 3)
			printk("ALS lux value: %d\n", Aval);

		printk("%s\n", Pval ? "obj near":"obj far");
	}

	mutex_unlock(&data->lock);
	return IRQ_HANDLED;
}

static int __devinit al3006_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct al3006_data *data;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	data = kzalloc(sizeof(struct al3006_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);
	data->irq = client->irq;

	/* initialize the AL3006 chip */
	err = al3006_init_client(client);
	if (err)
		goto exit_kfree;

	err = al3006_input_init(data);
	if (err)
		goto exit_kfree;

	/* register sysfs hooks */
	err = sysfs_create_group(&data->input->dev.kobj, &al3006_attr_group);
	if (err)
		goto exit_input;

	err = request_threaded_irq(client->irq, NULL, al3006_irq,
                               IRQF_TRIGGER_FALLING,
                               "al3006", data);
	if (err) {
		dev_err(&client->dev, "ret: %d, could not get IRQ %d\n",err,client->irq);
		goto exit_input;
	}

#ifdef CONFIG_BRCM_VIRTUAL_SENSOR
	brvsens_register(
		SENSOR_HANDLE_LIGHT,
		AL3006_DRV_NAME,
		(void*)data,
		(PFNACTIVATE)al3006_activate,
		(PFNREAD)al3006_read_als);

	brvsens_register(
		SENSOR_HANDLE_PROXIMITY,
		AL3006_DRV_NAME,
		(void*)data,
		(PFNACTIVATE)al3006_activate,
		(PFNREAD)al3006_read_ps);
#endif
	dev_info(&client->dev, "AL3006 driver version %s enabled\n", DRIVER_VERSION);
	return 0;

exit_input:
	al3006_input_fini(data);

exit_kfree:
	kfree(data);
	return err;
}

static int __devexit al3006_remove(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	free_irq(data->irq, data);

	sysfs_remove_group(&data->input->dev.kobj, &al3006_attr_group);
	al3006_set_power_state(client, 0);
	kfree(i2c_get_clientdata(client));
	return 0;
}

static int al3006_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	return al3006_activate(data, 0);
}

static int al3006_resume(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	return al3006_activate(data, 1);
}

static const struct i2c_device_id al3006_id[] = {
	{ "al3006", 0 },
	//{ "dyna", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, al3006_id);

static struct i2c_driver al3006_driver = {
	.driver = {
		.name	= AL3006_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = al3006_suspend,
	.resume	= al3006_resume,
	.probe	= al3006_probe,
	.remove	= __devexit_p(al3006_remove),
	.id_table = al3006_id,
};

static int __init al3006_init(void)
{
	return i2c_add_driver(&al3006_driver);
}

static void __exit al3006_exit(void)
{
	i2c_del_driver(&al3006_driver);
}

MODULE_AUTHOR("YC Hou, LiteOn-semi corporation.");
MODULE_DESCRIPTION("Test AP3006 driver on mini6410.");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);

module_init(al3006_init);
module_exit(al3006_exit);
