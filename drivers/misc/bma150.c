/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
 * All Rights Reserved
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/delay.h>

#include <linux/bma150.h>

#if CONFIG_SENSORS_BRCM_VIRTUAL
#include <linux/brvsens_driver.h>
#endif

#define SENSOR_NAME 			BMA150_DRIVER_NAME
#define GRAVITY_EARTH                   9806550
#define ABSMIN_2G                       (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                       (GRAVITY_EARTH * 2)
#define BMA150_MAX_DELAY		200
#define BMA150_CHIP_ID			2
#define BMA150_RANGE_SET		0
#define BMA150_BW_SET			4

#define BMA150_CHIP_ID_REG		0x00
#define BMA150_X_AXIS_LSB_REG		0x02
#define BMA150_X_AXIS_MSB_REG		0x03
#define BMA150_Y_AXIS_LSB_REG		0x04
#define BMA150_Y_AXIS_MSB_REG		0x05
#define BMA150_Z_AXIS_LSB_REG		0x06
#define BMA150_Z_AXIS_MSB_REG		0x07
#define BMA150_STATUS_REG	        0x09
#define BMA150_CTRL_REG		        0x0a
#define BMA150_CONF1_REG	        0x0b

#define BMA150_CUSTOMER1_REG		0x12
#define BMA150_CUSTOMER2_REG		0x13
#define BMA150_RANGE_BWIDTH_REG	        0x14
#define BMA150_CONF2_REG	        0x15

#define BMA150_OFFS_GAIN_X_REG		0x16
#define BMA150_OFFS_GAIN_Y_REG		0x17
#define BMA150_OFFS_GAIN_Z_REG		0x18
#define BMA150_OFFS_GAIN_T_REG		0x19
#define BMA150_OFFSET_X_REG		0x1a
#define BMA150_OFFSET_Y_REG		0x1b
#define BMA150_OFFSET_Z_REG		0x1c
#define BMA150_OFFSET_T_REG	        0x1d

#define BMA150_CHIP_ID__POS		0
#define BMA150_CHIP_ID__MSK		0x07
#define BMA150_CHIP_ID__LEN		3
#define BMA150_CHIP_ID__REG		BMA150_CHIP_ID_REG

/* DATA REGISTERS */
#define BMA150_NEW_DATA_X__POS  	0
#define BMA150_NEW_DATA_X__LEN  	1
#define BMA150_NEW_DATA_X__MSK  	0x01
#define BMA150_NEW_DATA_X__REG		BMA150_X_AXIS_LSB_REG

#define BMA150_ACC_X_LSB__POS   	6
#define BMA150_ACC_X_LSB__LEN   	2
#define BMA150_ACC_X_LSB__MSK		0xC0
#define BMA150_ACC_X_LSB__REG		BMA150_X_AXIS_LSB_REG

#define BMA150_ACC_X_MSB__POS   	0
#define BMA150_ACC_X_MSB__LEN   	8
#define BMA150_ACC_X_MSB__MSK		0xFF
#define BMA150_ACC_X_MSB__REG		BMA150_X_AXIS_MSB_REG

#define BMA150_ACC_Y_LSB__POS   	6
#define BMA150_ACC_Y_LSB__LEN   	2
#define BMA150_ACC_Y_LSB__MSK   	0xC0
#define BMA150_ACC_Y_LSB__REG		BMA150_Y_AXIS_LSB_REG

#define BMA150_ACC_Y_MSB__POS   	0
#define BMA150_ACC_Y_MSB__LEN   	8
#define BMA150_ACC_Y_MSB__MSK   	0xFF
#define BMA150_ACC_Y_MSB__REG		BMA150_Y_AXIS_MSB_REG

#define BMA150_ACC_Z_LSB__POS   	6
#define BMA150_ACC_Z_LSB__LEN   	2
#define BMA150_ACC_Z_LSB__MSK		0xC0
#define BMA150_ACC_Z_LSB__REG		BMA150_Z_AXIS_LSB_REG

#define BMA150_ACC_Z_MSB__POS   	0
#define BMA150_ACC_Z_MSB__LEN   	8
#define BMA150_ACC_Z_MSB__MSK		0xFF
#define BMA150_ACC_Z_MSB__REG		BMA150_Z_AXIS_MSB_REG

/* CONTROL BITS */
#define BMA150_SLEEP__POS		0
#define BMA150_SLEEP__LEN		1
#define BMA150_SLEEP__MSK		0x01
#define BMA150_SLEEP__REG		BMA150_CTRL_REG

#define BMA150_SOFT_RESET__POS		1
#define BMA150_SOFT_RESET__LEN		1
#define BMA150_SOFT_RESET__MSK		0x02
#define BMA150_SOFT_RESET__REG		BMA150_CTRL_REG

#define BMA150_EE_W__POS		4
#define BMA150_EE_W__LEN		1
#define BMA150_EE_W__MSK		0x10
#define BMA150_EE_W__REG		BMA150_CTRL_REG

#define BMA150_UPDATE_IMAGE__POS	5
#define BMA150_UPDATE_IMAGE__LEN	1
#define BMA150_UPDATE_IMAGE__MSK	0x20
#define BMA150_UPDATE_IMAGE__REG	BMA150_CTRL_REG

#define BMA150_RESET_INT__POS		6
#define BMA150_RESET_INT__LEN		1
#define BMA150_RESET_INT__MSK		0x40
#define BMA150_RESET_INT__REG		BMA150_CTRL_REG

/* BANDWIDTH dependend definitions */
#define BMA150_BANDWIDTH__POS		0
#define BMA150_BANDWIDTH__LEN		3
#define BMA150_BANDWIDTH__MSK		0x07
#define BMA150_BANDWIDTH__REG		BMA150_RANGE_BWIDTH_REG

/* RANGE */
#define BMA150_RANGE__POS		3
#define BMA150_RANGE__LEN		2
#define BMA150_RANGE__MSK		0x18
#define BMA150_RANGE__REG		BMA150_RANGE_BWIDTH_REG
   
/* WAKE UP */

#define BMA150_WAKE_UP__POS		0
#define BMA150_WAKE_UP__LEN		1
#define BMA150_WAKE_UP__MSK		0x01
#define BMA150_WAKE_UP__REG		BMA150_CONF2_REG
   
#define BMA150_WAKE_UP_PAUSE__POS	1
#define BMA150_WAKE_UP_PAUSE__LEN	2
#define BMA150_WAKE_UP_PAUSE__MSK	0x06
#define BMA150_WAKE_UP_PAUSE__REG	BMA150_CONF2_REG
  
#define BMA150_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)
                 

#define BMA150_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))


/* range and bandwidth */
#define BMA150_RANGE_2G		0
#define BMA150_RANGE_4G		1
#define BMA150_RANGE_8G		2

#define BMA150_BW_25HZ		0
#define BMA150_BW_50HZ		1
#define BMA150_BW_100HZ		2
#define BMA150_BW_190HZ		3
#define BMA150_BW_375HZ		4
#define BMA150_BW_750HZ		5
#define BMA150_BW_1500HZ	6

/* debug I2C */
#define BMA150_DEBUG_I2C 1

/* Data Buffer Structure */
struct bma150acc
{
	s16	x,
		y,
		z;
} ;

/* User Data */
struct bma150_data
{
	struct i2c_client*           bma150_client;
	int                          IRQ;
	atomic_t                     delay;
	struct input_dev*            input;
	struct bma150acc             value;
	struct mutex                 value_mutex;
	struct delayed_work          work;
	struct work_struct           irq_work;
};


static int switch_values(bma150_axis_change_enum axis_source_enum, 
                         struct bma150acc *p_acc_old, s16 *p_axis)
{
	switch(axis_source_enum)
	{
		case x_dir:
			*p_axis = p_acc_old->x;
			break;
			
		case y_dir:
			*p_axis = p_acc_old->y;
			break;
			
		case z_dir:
			*p_axis = p_acc_old->z;
			break;
			
		case x_dir_rev:
			*p_axis = -p_acc_old->x;
			break;
			
		case y_dir_rev:
			*p_axis = -p_acc_old->y;
			break;
			
		case z_dir_rev:
			*p_axis = -p_acc_old->z;
			break;
			
		default:
			return -1;
	}
	return 0;
}

/* The accelerometer IC has not been mounted as expected and the values
 * reported need to be swapped and/or reversed.
 */  
static void bma150_change_orientation(struct bma150acc *p_acc_orig_t, 
                                      struct t_bma150_axis_change *axis_change)
{
	s16 temp_x = 0, temp_y = 0, temp_z = 0;

	switch_values(axis_change->x_change, p_acc_orig_t, &temp_x);
	switch_values(axis_change->y_change, p_acc_orig_t, &temp_y);
	switch_values(axis_change->z_change, p_acc_orig_t, &temp_z);

	p_acc_orig_t->x = temp_x;
	p_acc_orig_t->y = temp_y;
	p_acc_orig_t->z = temp_z;
}

static int bma150_smbus_read_byte(struct i2c_client *client, 
                                  unsigned char reg_addr, 
                                  unsigned char *data)
{
	s32 rc = i2c_smbus_read_byte_data(client, reg_addr);
	if (rc < 0)
	{
#if BMA150_DEBUG_I2C
		printk(KERN_ERR "%s: i2c_smbus_read_byte_data failed (rc=%d)\n",
                       __FUNCTION__, rc);
#endif
		return rc;
	}
	*data = rc & 0x000000ff;
	return 0;
}   

static int bma150_smbus_write_byte(struct i2c_client *client, 
                                   unsigned char reg_addr, 
                                   unsigned char *data)
{
	s32 rc = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (rc < 0)
	{
#if BMA150_DEBUG_I2C
		printk(KERN_ERR "%s: i2c_smbus_write_byte_data failed (rc=%d)\n",
                       __FUNCTION__, rc);
#endif
		return rc;
	}
	return 0;
}

static int bma150_smbus_read_byte_block(struct i2c_client *client, 
                                        unsigned char reg_addr, 
                                        unsigned char *data, 
                                        unsigned char len)
{
	s32 rc = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (rc < 0)
	{
#if BMA150_DEBUG_I2C
		printk(KERN_ERR "%s: i2c_smbus_read_i2c_block_data failed (rc=%d)\n",
                       __FUNCTION__, rc);
#endif
		return rc;
	}
	return 0;
}


/* brvsens activate callback; we want to set BMA150_SLEEP for deactivation and
   clear it for activation  */
static int activate(struct i2c_client *client, unsigned char flag)
{
	unsigned char data  = 0;
	unsigned char sleep_bit  = flag ? 0:1;

        printk(KERN_INFO "BMA150 %s() requested activation - %d\n",
	       __FUNCTION__, flag);

	if (client == NULL)
		return -1;
	
	struct bma150_data* bma150 = i2c_get_clientdata(client);

#ifndef CONFIG_SENSORS_BRCM_VIRTUAL
	/* if we are activated */
	if(flag)
	{
		schedule_delayed_work(&bma150->work,
			      msecs_to_jiffies(atomic_read(&bma150->delay)));
	}
	else
	{
		flush_scheduled_work();
		cancel_delayed_work_sync(&bma150->work);
	}
#endif

	/* read the value of the control register where SLEEP bit is defined,
	   set or clear SLEEP bit and write the value back */
        if(bma150_smbus_read_byte(client, BMA150_SLEEP__REG, &data))
		return -1;
        data = BMA150_SET_BITSLICE(data, BMA150_SLEEP, sleep_bit);
        return bma150_smbus_write_byte(client, BMA150_SLEEP__REG, &data);
}

static int bma150_set_range(struct i2c_client *client, unsigned char Range)
{
	unsigned char data = 0;

	if (client == NULL)
		return -1;

	if (Range < 3) {
		if(bma150_smbus_read_byte(client, BMA150_RANGE__REG, &data))
			return -1;

		data = BMA150_SET_BITSLICE(data, BMA150_RANGE, Range);

		return bma150_smbus_write_byte(client, BMA150_RANGE__REG, &data);
	}

	return -1;
}

static int bma150_get_range(struct i2c_client *client, unsigned char *Range)
{
	unsigned char data;
	
	if (client == NULL)
		return -1;

	if(bma150_smbus_read_byte(client, BMA150_RANGE__REG, &data))
		return -1;

	*Range = BMA150_GET_BITSLICE(data, BMA150_RANGE);

	return 0;
}

static int bma150_set_bandwidth(struct i2c_client *client, unsigned char BW)
{
	unsigned char data = 0;

	if (client == NULL)
		return -1;

	if (BW < 8) {
		if(bma150_smbus_read_byte(client, BMA150_BANDWIDTH__REG, &data))
			return -1;

		data = BMA150_SET_BITSLICE(data, BMA150_BANDWIDTH, BW);

		return bma150_smbus_write_byte(client, BMA150_BANDWIDTH__REG, &data);

	}
	return -1;
}

static int bma150_get_bandwidth(struct i2c_client *client, unsigned char *BW)
{
	unsigned char data;

	if (client == NULL)
		return -1;

	if(bma150_smbus_read_byte(client, BMA150_BANDWIDTH__REG, &data))
		return -1;

	*BW = BMA150_GET_BITSLICE(data, BMA150_BANDWIDTH);

	return 0;
}

static int bma150_read_accel_xyz(struct i2c_client *client, struct bma150acc *acc)
{
	unsigned char data[6];

	if (client == NULL)
		return -1;

	if(bma150_smbus_read_byte_block(client, BMA150_ACC_X_LSB__REG, &data[0], 6))
		return -1;

	acc->x = BMA150_GET_BITSLICE(data[0], BMA150_ACC_X_LSB) |
		(BMA150_GET_BITSLICE(data[1], BMA150_ACC_X_MSB)<<
						BMA150_ACC_X_LSB__LEN);
	acc->x = acc->x << (sizeof(short)*8-(BMA150_ACC_X_LSB__LEN+
						BMA150_ACC_X_MSB__LEN));
	acc->x = acc->x >> (sizeof(short)*8-(BMA150_ACC_X_LSB__LEN+
						BMA150_ACC_X_MSB__LEN));

	acc->y = BMA150_GET_BITSLICE(data[2], BMA150_ACC_Y_LSB) |
		(BMA150_GET_BITSLICE(data[3], BMA150_ACC_Y_MSB)<<
						BMA150_ACC_Y_LSB__LEN);
	acc->y = acc->y << (sizeof(short)*8-(BMA150_ACC_Y_LSB__LEN +
						BMA150_ACC_Y_MSB__LEN));
	acc->y = acc->y >> (sizeof(short)*8-(BMA150_ACC_Y_LSB__LEN +
						BMA150_ACC_Y_MSB__LEN));


	acc->z = BMA150_GET_BITSLICE(data[4], BMA150_ACC_Z_LSB);
	acc->z |= (BMA150_GET_BITSLICE(data[5], BMA150_ACC_Z_MSB)<<
						BMA150_ACC_Z_LSB__LEN);
	acc->z = acc->z << (sizeof(short)*8-(BMA150_ACC_Z_LSB__LEN+
						BMA150_ACC_Z_MSB__LEN));
	acc->z = acc->z >> (sizeof(short)*8-(BMA150_ACC_Z_LSB__LEN+
						BMA150_ACC_Z_MSB__LEN));

	if (client->dev.platform_data != NULL)
	{  
		/* Need to modify the values reported. */
		bma150_change_orientation(acc, (struct t_bma150_axis_change *) 
                                          client->dev.platform_data);
	}

	return 0;
}

#ifndef CONFIG_SENSORS_BRCM_VIRTUAL
static void bma150_work_func(struct work_struct* work)
{
	struct bma150_data* bma150 = container_of((struct delayed_work*)work,
			struct bma150_data, work);
			
	static struct bma150acc acc;
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma150->delay));

	if(!bma150_read_accel_xyz(bma150->bma150_client, &acc))
	{
		input_report_abs(bma150->input, ABS_X, acc.x);
		input_report_abs(bma150->input, ABS_Y, acc.y);
		input_report_abs(bma150->input, ABS_Z, acc.z);
		input_sync(bma150->input);
		mutex_lock(&bma150->value_mutex);
		bma150->value = acc;
		mutex_unlock(&bma150->value_mutex);
	}
	schedule_delayed_work(&bma150->work, delay);
}
#endif

static ssize_t bma150_range_show(struct device *dev, struct device_attribute *attr, 
                                 char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma150_data *bma150 = i2c_get_clientdata(client);

	if (bma150_get_range(bma150->bma150_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma150_range_store(struct device *dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma150_data *bma150 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma150_set_range(bma150->bma150_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma150_bandwidth_show(struct device *dev, 
                                     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma150_data *bma150 = i2c_get_clientdata(client);

	if (bma150_get_bandwidth(bma150->bma150_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma150_bandwidth_store(struct device *dev, 
                                      struct device_attribute *attr, 
                                      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma150_data *bma150 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma150_set_bandwidth(bma150->bma150_client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma150_value_show(struct device* dev, struct device_attribute* attr, 
                                 char* buf)
{
	struct input_dev* input = to_input_dev(dev);
	struct bma150_data* bma150 = input_get_drvdata(input);
	struct bma150acc acc_value;

	mutex_lock(&bma150->value_mutex);
	acc_value = bma150->value;
	mutex_unlock(&bma150->value_mutex);

	return sprintf(buf, "X:%d Y:%d Z:%d\n", acc_value.x, acc_value.y, acc_value.z);
}

static ssize_t bma150_get_data(struct device* dev, struct device_attribute* attr, 
                               char* buf)
{
	struct input_dev* input = to_input_dev(dev);
	struct bma150_data* bma150 = input_get_drvdata(input);
	struct bma150acc acc_value;
	
	mutex_lock(&bma150->value_mutex);
	acc_value = bma150->value;
	mutex_unlock(&bma150->value_mutex);

	memcpy(buf, &acc_value, sizeof(acc_value) );
	return sizeof(acc_value);
}

static ssize_t bma150_delay_show(struct device* dev, struct device_attribute* attr, 
                                 char* buf)
{
	struct i2c_client*  client = to_i2c_client(dev);
	struct bma150_data* bma150 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma150->delay));
}

static ssize_t bma150_delay_store(struct device* dev, struct device_attribute* attr, 
                                  const char* buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client* client  = to_i2c_client(dev);
	struct bma150_data* bma150 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (data > BMA150_MAX_DELAY)
		data = BMA150_MAX_DELAY;

	atomic_set(&bma150->delay, (unsigned int) data);

	return count;
}


static DEVICE_ATTR(range, S_IRUGO|S_IWUSR|S_IWGRP,
		bma150_range_show, bma150_range_store);
		
static DEVICE_ATTR(bandwidth, S_IRUGO|S_IWUSR|S_IWGRP,
		bma150_bandwidth_show, bma150_bandwidth_store);
		
// for command line: Human readable		
static DEVICE_ATTR(values, S_IRUGO|S_IWUSR|S_IWGRP,
		bma150_value_show, NULL);
		
// programatic interface without need for parsing
static DEVICE_ATTR(data, S_IRUGO|S_IWUSR|S_IWGRP,
		bma150_get_data, NULL);
		
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bma150_delay_show, bma150_delay_store);


static struct attribute* bma150_attributes[] = 
{
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_values.attr,
	&dev_attr_data.attr,
	&dev_attr_delay.attr,
	NULL
};

static struct attribute_group bma150_attribute_group = 
{
	.attrs = bma150_attributes
};

static int bma150_detect(struct i2c_client* client, struct i2c_board_info* info)
{
	struct i2c_adapter* adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	strlcpy(info->type, SENSOR_NAME, I2C_NAME_SIZE);

	return 0;
}

static int bma150_input_init(struct bma150_data* bma150)
{
	struct input_dev* dev;
	int err;

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
		
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_X, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_abs_params(dev, ABS_Y, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_abs_params(dev, ABS_Z, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_drvdata(dev, bma150);

	err = input_register_device(dev);
	if (err < 0) 
	{
		input_free_device(dev);
		return err;
	}
	bma150->input = dev;

	return 0;
}

static void bma150_input_delete(struct bma150_data* bma150)
{
	struct input_dev* dev = bma150->input;

	input_unregister_device(dev);
	input_free_device(dev);
}


static int bma150_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue = 0;
	struct bma150_data* data = 0;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk(KERN_ERR "%s: i2c_check_functionality error\n", __FUNCTION__);
		err = -ENODEV;
		goto exit;
	}
	
	if (client->dev.platform_data == NULL)
	{  
		/* No axis values need to be changed. */
		printk(KERN_INFO "%s: No axis values need to be changed\n", 
                       __FUNCTION__);
	}
	else
	{
		struct t_bma150_axis_change* axis_change = 
			(struct t_bma150_axis_change*) client->dev.platform_data;
		
		printk(KERN_INFO "%s: x_change: %d y_change: %d z_change: %d\n", 
                       __FUNCTION__,
                       axis_change->x_change, axis_change->y_change, 
                       axis_change->z_change);
	}

	data = kzalloc(sizeof(struct bma150_data), GFP_KERNEL);
	if (!data)
	{
		err = -ENOMEM;
		goto exit;
	}
		
	tempvalue = i2c_smbus_read_word_data(client, BMA150_CHIP_ID_REG);

	if ( (tempvalue & 0x00FF) == BMA150_CHIP_ID)
	{
		printk(KERN_INFO "Bosch Sensortec Device detected!\n" \
		       "BMA150 registered I2C driver!\n");
	} 
	else
	{
		printk(KERN_INFO "Bosch Sensortec Device not found" \
		       "i2c error %d \n", tempvalue);
				
		err = -1;
		goto kfree_exit;
	}

	i2c_set_clientdata(client, data);
	data->bma150_client = client;
	mutex_init(&data->value_mutex);
	if(bma150_set_bandwidth(client, BMA150_BW_SET))
	{
		printk(KERN_ERR "%s: read/write error\n", __FUNCTION__);
		err = -1;
		goto kfree_exit;
	}
	if(bma150_set_range(client, BMA150_RANGE_SET))
	{
		printk(KERN_ERR "%s: read/write error\n", __FUNCTION__);
		err = -1;
		goto kfree_exit;
	}

#ifndef CONFIG_SENSORS_BRCM_VIRTUAL
	INIT_DELAYED_WORK(&data->work, bma150_work_func);
	atomic_set(&data->delay, BMA150_MAX_DELAY);
#endif

	err = bma150_input_init(data);
	if (err < 0)
		goto kfree_exit;

	err = sysfs_create_group(&data->input->dev.kobj,
				 &bma150_attribute_group);
			
	if (err < 0)
		goto error_sysfs;

#if CONFIG_SENSORS_BRCM_VIRTUAL
	// register accelerometer with BRVSENS device
	brvsens_register(SENSOR_HANDLE_ACCELEROMETER,      // sensor UID
			 BMA150_DRIVER_NAME,               // human readable name
			 (void*)client,                    // context; passed back in read/activate callbacks
			 (PFNACTIVATE)activate,     // activate callback
			 (PFNREAD)bma150_read_accel_xyz);  // read callback
#endif
	
	return 0;

error_sysfs:
	bma150_input_delete(data);

kfree_exit:
	kfree(data);
	
exit:
	printk(KERN_ERR "%s FAILED\n", __FUNCTION__);
	return err;
}

static int bma150_suspend(struct i2c_client* client, pm_message_t mesg)
{
	return activate(client, 0);
}

static int bma150_resume(struct i2c_client* client)
{
	return activate(client, 1);
}

static int bma150_remove(struct i2c_client* client)
{
	struct bma150_data* data = i2c_get_clientdata(client);

	sysfs_remove_group(&data->input->dev.kobj, &bma150_attribute_group);
	bma150_input_delete(data);
	kfree(data);
	
	return 0;
}

static const struct i2c_device_id bma150_id[] = 
{
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma150_id);

static struct i2c_driver bma150_driver =
{	
	.driver = 
	{
		.owner    = THIS_MODULE,	
		.name	  = SENSOR_NAME,
	},
	.class		  = I2C_CLASS_HWMON,
	.id_table	  = bma150_id,
	.probe		  = bma150_probe,
	.remove		  = bma150_remove,
	.detect		  = bma150_detect,
	.suspend      = bma150_suspend,
	.resume       = bma150_resume,
};

static int __init BMA150_init(void)
{
	return i2c_add_driver(&bma150_driver);
}

static void __exit BMA150_exit(void)
{
	i2c_del_driver(&bma150_driver);
}


MODULE_AUTHOR("Albert Zhang <xu.zhang@bosch-sensortec.com>");
MODULE_DESCRIPTION("BMA150 driver");
MODULE_LICENSE("GPL");

module_init(BMA150_init);
module_exit(BMA150_exit);
