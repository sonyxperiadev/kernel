/* drivers/misc/akm8975.c - akm8975 compass driver
 *
 * Copyright (C) 2007-2008 HTC Corporation.
 * Author: Hou-Kun Chen <houkun.chen@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * Revised by AKM 2009/04/02
 * Revised by Motorola 2010/05/27
 * Modifications for Broadcom virtual sensor driver 2011/08/18
 */


#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/delay.h>

#include <linux/akm8975_brcm.h>

#include <linux/brvsens_driver.h>

/* AKM8975 Operation Modes */
#define AK8975_MODE_SNG_MEASURE   0x01
#define	AK8975_MODE_SELF_TEST     0x08
#define	AK8975_MODE_FUSE_ACCESS   0x0F
#define	AK8975_MODE_POWER_DOWN    0x00

/* AK8975 Register Addresses */
#define AK8975_REG_WIA		0x00
#define AK8975_REG_INFO		0x01
#define AK8975_REG_ST1		0x02
#define AK8975_REG_HXL		0x03
#define AK8975_REG_HXH		0x04
#define AK8975_REG_HYL		0x05
#define AK8975_REG_HYH		0x06
#define AK8975_REG_HZL		0x07
#define AK8975_REG_HZH		0x08
#define AK8975_REG_ST2		0x09
#define AK8975_REG_CNTL		0x0A
#define AK8975_REG_RSV		0x0B
#define AK8975_REG_ASTC		0x0C
#define AK8975_REG_TS1		0x0D
#define AK8975_REG_TS2		0x0E
#define AK8975_REG_I2CDIS	0x0F

// AK8975 fuse-rom address
#define AK8975_FUSE_ASAX	0x10
#define AK8975_FUSE_ASAY	0x11
#define AK8975_FUSE_ASAZ	0x12

// read interval between measurement request and data read
// future implementations should poll data ready 
// (DRDY bit of ST1 register on Asahi Kasei)
#define AKM8975_READ_INTERVAL  10     // miliseconds

#define AK8975_DEBUG 0

#if AK8975_DEBUG
   #define FUNCDBG(msg)	printk("++++ %s:%s ++++\n", __func__, msg);
#else
   #define FUNCDBG(msg)
#endif


/* Data Buffer Structure definition */
struct akm8975_measure
{
	s16	x,
		y,
		z;
};


/* Client Data */
struct akm8975_data 
{
	struct i2c_client*  this_client;
	struct input_dev*   input;
};


/* Common algorithm for switching axis values. It belongs to shared library, instead of every
   driver having to reimplement same code */
static int switch_values
(
   akm8975_axis_change_enum     axis_source_enum, 
   struct akm8975_measure*      p_akm_old, 
   s16*                         p_axis
)
{
	switch (axis_source_enum)
	{
		case compass_x_dir:
			*p_axis = p_akm_old->x;
			break;
			
		case compass_y_dir:
			*p_axis = p_akm_old->y;
			break;
			
		case compass_z_dir:
			*p_axis = p_akm_old->z;
			break;
			
		case compass_x_dir_rev:
			*p_axis = -p_akm_old->x;
			break;
			
		case compass_y_dir_rev:
			*p_axis = -p_akm_old->y;
			break;
			
		case compass_z_dir_rev:
			*p_axis = -p_akm_old->z;
			break;
			
		default:
			return -1;
	}
	
	return 0;
}

/* The compass IC has not been mounted as expected and the values
 *  reported need to be swapped and/or reversed.
 */  
static void akm8975_change_orientation
(
   struct akm8975_measure*         p_akm_orig_t, 
   struct t_akm8975_axis_change*   axis_change
)
{
	s16 temp_x = 0, temp_y = 0, temp_z = 0;

	switch_values(axis_change->x_change, p_akm_orig_t, &temp_x);
	switch_values(axis_change->y_change, p_akm_orig_t, &temp_y);
	switch_values(axis_change->z_change, p_akm_orig_t, &temp_z);

	p_akm_orig_t->x = temp_x;
	p_akm_orig_t->y = temp_y;
	p_akm_orig_t->z = temp_z;
}

/* IO Functionality */
static int akm8975_i2c_txdata
(
   struct akm8975_data* akm, 
   char*                buf,
   int                  length  
)
{
	struct i2c_msg msgs[] = 
	{
		{
			.addr = akm->this_client->addr,
			.flags = 0,
			.len = length,
			.buf = buf,
		},
	};

	if (i2c_transfer(akm->this_client->adapter, msgs, 1) < 0) 
	{
		printk(KERN_ERR "akm8975_i2c_txdata: transfer error\n");
		return -EIO;
	} 
	else
		return 0;
}

static int akm8975_i2c_rxdata
(
   struct akm8975_data* akm,
   char*                buf,
   int                  length
)
{
	struct i2c_msg msgs[] = 
	{
		{
			.addr = akm->this_client->addr,
			.flags = 0,
			.len = 1,
			.buf = buf,
		},
		{
			.addr = akm->this_client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buf,
		},
	};

	if (i2c_transfer(akm->this_client->adapter, msgs, 2) < 0) 
	{
		printk(KERN_ERR "akm8975_i2c_rxdata: transfer error\n");
		return -EIO;
	} 
	else
		return 0;
}

/* Measurement function */
static int akm8975_measure_cbk
(
   struct akm8975_data*      data,
   struct akm8975_measure*   measure
)
{    
   char buf[6];
   
   FUNCDBG("Enter\n");
   
   /* Implementation needs to:
      -- Perform single write operation to AK8975_REG_CNTL, requesting AK8975_MODE_SNG_MEASURE
      -- Perform read operation to AK8975_REG_HXL requesting 6 bytes  */
   
   // write 
   buf[0] = AK8975_REG_CNTL;
   buf[1] = AK8975_MODE_SNG_MEASURE;
   
   if (akm8975_i2c_txdata(data, buf, 2) != 0)
      return -1;
   
   // relax tiny bit
   msleep_interruptible(AKM8975_READ_INTERVAL);
   
   // now read
   buf[0] = AK8975_REG_HXL;
   if (akm8975_i2c_rxdata(data, buf, sizeof(buf) ) != 0)
      return -1;
      
   memcpy(measure, buf, sizeof(buf) );
   
   if (data->this_client->dev.platform_data != NULL)
   {  
      /* Need to modify the values reported. */
	  akm8975_change_orientation(measure, 
	     (struct t_akm8975_axis_change*) data->this_client->dev.platform_data);
   }
   
   /* NOTE: Not accounting for axis sensitivity. For Asahi Kasei Compass Driver 
            this is calculated by rather complicated formula: 
             
            Hadj = H x ( (ASA - 128) x 0.5 / 128 + 1)
            
            Although values are not the same for each axis, difference is small
            Initial version will simply have to compensate in user space. Future 
            releases might decide to send adjustments in last 2 bytes of event data
            structure; Sensor User Library can then decide to fine tune using the
            formula above */
            
   FUNCDBG("Exit\n");
   return 0;
}


/* Axis Sensitivity retrieval */
static int akm8975_retrieve_sensitivity
(
   struct akm8975_data* data,
   char*                buffer
)
{
   char buf[2];
    
   /* Implementation needs to:
      -- Perform single write operation to AK8975_REG_CNTL, requesting AK8975_MODE_FUSE_ACCESS
      -- Perform 3 consecutive read operations to AK8975_FUSE_ASA(X)(Y)(Z) */
      
   // write 
   buf[0] = AK8975_REG_CNTL;
   buf[1] = AK8975_MODE_FUSE_ACCESS;
   
   if (akm8975_i2c_txdata(data, buf, 2) != 0)
      return -1;
   
   // relax tiny bit
   msleep_interruptible(AKM8975_READ_INTERVAL);
   
   // we perform 3 successive reads requesting a byte 
   buf[0] = AK8975_FUSE_ASAX; 
   if (akm8975_i2c_rxdata(data, buf, 1 ) != 0)
      return -1;
      
   *(buffer + 0) = buf[0];   // store x value
      
   buf[0] = AK8975_FUSE_ASAY; 
   if (akm8975_i2c_rxdata(data, buf, 1 ) != 0)
      return -1;
      
   *(buffer + 1) = buf[0];   // store y value
      
   buf[0] = AK8975_FUSE_ASAZ; 
   if (akm8975_i2c_rxdata(data, buf, 1 ) != 0)
      return -1;
      
   *(buffer + 2) = buf[0];   // store z value
   
   // flip axes if necessary
   if (data->this_client->dev.platform_data != NULL)
   { 
       struct akm8975_measure measure;
       measure.x = *(buffer + 0);
       measure.y = *(buffer + 1);
       measure.z = *(buffer + 2);
       
       akm8975_change_orientation(&measure, 
	     (struct t_akm8975_axis_change*) data->this_client->dev.platform_data);
	     
	   memcpy(buffer + 0, &(measure.x), 1);
	   memcpy(buffer + 1, &(measure.y), 1);
	   memcpy(buffer + 2, &(measure.z), 1);
   }
    
   return 0;
}

/* Self Test function */
static int    akm8975_selftest
(
   struct akm8975_data*      data,
   struct akm8975_measure*   measure
)
{
   char buf[6];
   
   FUNCDBG("Enter\n");
   
   /* Implementation needs to:
      -- Perform single write operation to AK8975_REG_CNTL, requesting AK8975_MODE_SELF_TEST
      -- Perform read operation to AK8975_REG_HXL requesting 6 bytes  */
   
   // write 
   buf[0] = AK8975_REG_CNTL;
   buf[1] = AK8975_MODE_SELF_TEST;
   
   if (akm8975_i2c_txdata(data, buf, 2) != 0)
      return -1;
   
   // relax tiny bit
   msleep_interruptible(AKM8975_READ_INTERVAL);
   
   // now read
   buf[0] = AK8975_REG_HXL; 
   if (akm8975_i2c_rxdata(data, buf, sizeof(buf) ) != 0)
      return -1;
   
   // flip the axes if necessary
   memcpy(measure, buf, sizeof(buf) );
   
   if (data->this_client->dev.platform_data != NULL)
   {  
      /* Need to modify the values reported. */
	  akm8975_change_orientation(measure, 
	     (struct t_akm8975_axis_change*) data->this_client->dev.platform_data);
   }
   
   // TODO: Validate measured data against the criteria, which is yet TBD
     
   FUNCDBG("Exit\n");
   return 0;
}

static ssize_t akm8975_value_show
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	struct input_dev* input = to_input_dev(dev);
	struct akm8975_data* akm = input_get_drvdata(input);
	struct akm8975_measure measure;
	
    int i = akm8975_measure_cbk(akm, &measure);
    if (i != 0)
        return sprintf(buf, "AKM8975 Error\n");
    
    // can not adjust sensitivity -- no float point math in Kernel
    return sprintf(buf, "X:%d Y:%d Z:%d\n",
      measure.x, measure.y, measure.z);
}

static ssize_t akm8975_sensitivity_show
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	struct input_dev* input = to_input_dev(dev);
	struct akm8975_data* akm = input_get_drvdata(input);
	char buffer[4];
	
	int i = akm8975_retrieve_sensitivity(akm, buffer);
	if (i != 0)
	    return sprintf(buf, "AKM8975 Error\n");
	
	return sprintf(buf, "X:%d Y:%d Z:%d\n",
      buffer[0], buffer[1], buffer[2] );
}

static ssize_t akm8975_selftest_show
(
   struct device*           dev,
   struct device_attribute* attr,
   char*                    buf
)
{
	struct input_dev* input = to_input_dev(dev);
	struct akm8975_data* akm = input_get_drvdata(input);
	struct akm8975_measure measure;
	
    int i = akm8975_selftest(akm, &measure);
    if (i != 0)
        return sprintf(buf, "AKM8975 Error\n");
    
    return sprintf(buf, "X:%d Y:%d Z:%d\n",
      measure.x, measure.y, measure.z);
}

// Linux shell access	
static DEVICE_ATTR(values, S_IRUGO|S_IWUSR|S_IWGRP,
		akm8975_value_show, NULL);


static DEVICE_ATTR(sensitivity, S_IRUGO|S_IWUSR|S_IWGRP,
		akm8975_sensitivity_show, NULL);
		
static DEVICE_ATTR(selftest, S_IRUGO|S_IWUSR|S_IWGRP,
        akm8975_selftest_show, NULL);
		
static struct attribute* akm8975_attributes[] = 
{
	&dev_attr_values.attr,
	&dev_attr_sensitivity.attr,
	&dev_attr_selftest.attr,
	NULL
};

static struct attribute_group akm8975_attribute_group = 
{
	.attrs = akm8975_attributes
};


/* Internal Helper that initializes sys interface */
static int akm8975_input_init(struct akm8975_data* akm)
{
	// allocate input device
	struct input_dev* dev = input_allocate_device();
	if (!dev) 
	{
	    printk(KERN_ERR "%s:Input device allocate failed\n", __FUNCTION__);
		return -ENOMEM;
	}
	
	// register input device
	dev->name = AKM8975_DEVICE_NAME;

	if (input_register_device(dev) )
	{
	    printk(KERN_ERR "%s: Unable to register input device: [%s]\n",
	       __FUNCTION__, dev->name);
					 
	    input_free_device(dev);
		return -ENODEV;
	}
	
	set_bit(EV_ABS, dev->evbit);
	
	/* x-axis of raw magnetic vector */
	input_set_abs_params(dev, ABS_HAT0X, -20480, 20479, 0, 0);
	
	/* y-axis of raw magnetic vector */
	input_set_abs_params(dev, ABS_HAT0Y, -20480, 20479, 0, 0);
	
	/* z-axis of raw magnetic vector */
	input_set_abs_params(dev, ABS_BRAKE, -20480, 20479, 0, 0);
	
	// all ok
	akm->input = dev;
	input_set_drvdata(dev, akm);
	
	return 0;
}

static void akm8975_input_delete(struct akm8975_data* akm)
{
	struct input_dev* dev = akm->input;

	input_unregister_device(dev);
	input_free_device(dev);
}

/* Standard module "probe" implementation */
static int akm8975_probe
(
   struct i2c_client*          client,
   const struct i2c_device_id* devid
)
{
	struct akm8975_data* akm = 0;
	struct akm8975_measure measure; 
	
	FUNCDBG("Enter\n");
	
	// check i2c adapter
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk(KERN_ERR "%s: i2c_check_functionality error\n", __FUNCTION__);
		return -ENXIO;
	}
	
	// dump axis change for debugging
    if (client->dev.platform_data == NULL)
	{  
	    /* No axis values need to be changed. */
	    FUNCDBG("No axis values need to be changed for AKM Driver\n");
	}
	else
	{
		struct t_akm8975_axis_change* axis_change = (struct t_akm8975_axis_change*) client->dev.platform_data;
		
		printk(KERN_INFO "%s() x_change: %d y_change: %d z_change: %d\n", __FUNCTION__,
			axis_change->x_change, axis_change->y_change, axis_change->z_change);
	}
	
	// allocate client data
    akm = kzalloc(sizeof(struct akm8975_data), GFP_KERNEL);
	if (!akm) 
	{
		printk(KERN_ERR "%s:Failed to allocate memory for Module Data\n", __FUNCTION__);
			
		return -ENOMEM;
	}
	
	// connect the dots
	akm->this_client = client;
	i2c_set_clientdata(client, akm);
	
	// perform  initial read to verify sensor works
	// (NOTE: Seems that self-test creates its own magnetic field which further
	//        impacts sensor calibration)
	if (akm8975_measure_cbk(akm, &measure) != 0)
	{
	   printk(KERN_ERR "%s: AKM8975 Measure Failed\n", __FUNCTION__);
	   kfree (akm);
	   
	   return -ENXIO;
	}
	
	
	// register input device
	if (akm8975_input_init(akm) )
	{
	   printk(KERN_ERR "%s: Error registering Input Device\n", __FUNCTION__);
	   
	   kfree (akm);
	   return -ENODEV;
	}
	
	if (sysfs_create_group(&akm->input->dev.kobj,
			&akm8975_attribute_group) < 0)
    {
       printk(KERN_ERR "%s: Error registering Input Device\n", __FUNCTION__);
	   
	   akm8975_input_delete(akm);
	   kfree (akm);
	   
	   return -ENODEV;
    }
	
	// register with BRVSENS driver 
    brvsens_register(
       SENSOR_HANDLE_COMPASS,                 // sensor UID
       AKM8975_DRV_NAME,                      // human readable name
       (void*)akm,                            // context: passed back in activate/read callbacks
       0,                                     // NO activate callback -- sensor puts itself in low power mode automatically after each read
       (PFNREAD)akm8975_measure_cbk);         // read callback
       
    printk(KERN_DEBUG "akm8975 driver register OK\n");
    
    // and we are done!
	FUNCDBG("Exit\n");
	return 0;
}

/* Standard Module "remove" implementation */
static int __devexit akm8975_remove(struct i2c_client* client)
{
	struct akm8975_data* akm = i2c_get_clientdata(client);
	FUNCDBG("Enter\n");
	
	sysfs_remove_group(&akm->input->dev.kobj, &akm8975_attribute_group);
	akm8975_input_delete(akm);
	
	kfree(akm);
	
	FUNCDBG("Exit\n");
	return 0;
}


static const struct i2c_device_id akm8975_id [] =
{
	{ AKM8975_DRV_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, akm8975_id);

static struct i2c_driver akm8975_driver = 
{
	.probe    = akm8975_probe,
	.remove   = akm8975_remove,
	.id_table = akm8975_id,
	
	.driver = 
	{
	    .owner = THIS_MODULE,
		.name  = AKM8975_DRV_NAME,
	},
};

static int __init akm8975_init(void)
{
    int err = 0;
    
	FUNCDBG("Enter\n");
	err = i2c_add_driver(&akm8975_driver);
	
	if (err != 0)
	   printk(KERN_ERR "%s: i2c_add_driver failure [%d]\n", __FUNCTION__, err);
	   
	FUNCDBG("Exit\n");
	return err;
}

static void __exit akm8975_exit(void)
{
	FUNCDBG("Enter\n");
	i2c_del_driver(&akm8975_driver);
	FUNCDBG("Exit\n");
}

module_init(akm8975_init);
module_exit(akm8975_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("AK8975 compass driver");
MODULE_LICENSE("GPL");
