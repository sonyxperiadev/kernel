/*
 * mpu3050.c - MPU3050 Tri-axis gyroscope driver
 *
 * Copyright (C) 2011 Wistron Co.Ltd
 * Joseph Lai <joseph_lai@wistron.com>
 *
 * Trimmed down by Alan Cox <alan@linux.intel.com> to produce this version
 *
 * This is a 'lite' version of the driver, while we consider the right way
 * to present the other features to user space. In particular it requires the
 * device has an IRQ, and it only provides an input interface, so is not much
 * use for device orientation. A fuller version is available from the Meego
 * tree.
 *
 * This program is based on bma023.c.
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/mpu3050.h>
#include <linux/gpio.h>

#include <linux/miscdevice.h>
#include <linux/poll.h>

#include <linux/brvsens_driver.h>

// #define PROC_DEBUG   // comment in/out to enable cmd line interface via /proc/mpu3050

#ifdef PROC_DEBUG
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#endif

#define MPU3050_CHIP_ID_REG	    0x00
#define MPU3050_CHIP_ID		    0x69
#define MPU3050_XOUT_H		    0x1D
#define MPU3050_PWR_MGM		    0x3E
#define MPU3050_PWR_MGM_POS	    6
#define MPU3050_PWR_MGM_MASK	0x40

#define MPU3050_AUTO_DELAY	    1000

#define MPU3050_MIN_VALUE	    -32768
#define MPU3050_MAX_VALUE	    32767

#define SENSOR_NAME 			MPU3050_DRV_NAME

struct axis_data 
{
	s16 x;
	s16 y;
	s16 z;
};

/* User Data */
struct mpu3050_data 
{
	struct device*     dev;
	struct input_dev*  idev;
	struct mutex       lock;
};


/* Helper to switch values depending how sensor is mounted on the board */
static int mpu_switch_values
(
   mpu3050_axis_change_enum       axis_source_enum, 
   struct axis_data*              p_axis_old,
   s16*                           p_axis
)
{
	switch (axis_source_enum)
	{
		case gyro_x_dir:
			*p_axis = p_axis_old->x;
			break;
			
		case gyro_y_dir:
			*p_axis = p_axis_old->y;
			break;
			
		case gyro_z_dir:
			*p_axis = p_axis_old->z;
			break;
			
		case gyro_x_dir_rev:
			*p_axis = -p_axis_old->x;
			break;
			
		case gyro_y_dir_rev:
			*p_axis = -p_axis_old->y;
			break;
			
		case gyro_z_dir_rev:
			*p_axis = -p_axis_old->z;
			break;
			
		default:
			return -1;
	}
	
	return 0;
}

/* Correction of data depending how gyro chip has been mounted on the board */
static void mpu3050_change_orientation
(
   struct axis_data*             p_axis_orig_t,
   struct t_mpu3050_axis_change* axis_change
)
{
	s16 temp_x = 0, temp_y = 0, temp_z = 0;

	mpu_switch_values(axis_change->x_change, p_axis_orig_t, &temp_x);
	mpu_switch_values(axis_change->y_change, p_axis_orig_t, &temp_y);
	mpu_switch_values(axis_change->z_change, p_axis_orig_t, &temp_z);

	p_axis_orig_t->x = temp_x;
	p_axis_orig_t->y = temp_y;
	p_axis_orig_t->z = temp_z;
}

/**
 *	mpu3050_xyz_read_reg	-	read the axes values
 *	@buffer: provide register addr and get register
 *	@length: length of register
 *
 *	Reads the register values in one transaction or returns a negative
 *	error code on failure.
 */
static int mpu3050_xyz_read_reg
(
   struct i2c_client* client,
   u8*                buffer,
   int                length
)
{
	/* Annoying we can't make this const because the i2c layer doesn't
	   declare input buffers const */
	char cmd = MPU3050_XOUT_H;
	struct i2c_msg msg[] = 
	{
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &cmd,
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
 *	mpu3050_read_xyz	-	get co-ordinates from device
 *	@client: i2c address of sensor
 *	@coords: co-ordinates to update
 *
 *	Return the converted X Y and Z co-ordinates from the sensor device
 */
static void mpu3050_read_xyz
(
   struct i2c_client*  client,
   struct axis_data*   coords
)
{
	u16 buffer[3];
	struct mpu3050_platform_data* mpd = (struct mpu3050_platform_data*) client->dev.platform_data;
	
	mpu3050_xyz_read_reg(client, (u8 *)buffer, 6);
	
	coords->x = be16_to_cpu(buffer[0]);
	coords->y = be16_to_cpu(buffer[1]);
	coords->z = be16_to_cpu(buffer[2]);
	
	/* Adjust axis data if required */
    if (mpd->p_axis_change != 0)
	{  
	   mpu3050_change_orientation(coords, mpd->p_axis_change);
	}
	
	
	dev_dbg(&client->dev, "%s: x %d, y %d, z %d\n", __FUNCTION__,
					coords->x, coords->y, coords->z);
}

/**
 *	mpu3050_set_power_mode	-	set the power mode
 *	@client: i2c client for the sensor
 *	@val: value to switch on/off of power, 1: normal power, 0: low power
 *
 *	Put device to normal-power mode or low-power mode.
 */
static void mpu3050_set_power_mode(struct i2c_client* client, u8 val)
{
	u8 value;

        printk(KERN_INFO "%s() requested power mode - %d\n",
               __FUNCTION__, val); 

	value = i2c_smbus_read_byte_data(client, MPU3050_PWR_MGM);
	value = (value & ~MPU3050_PWR_MGM_MASK) |
		(((val << MPU3050_PWR_MGM_POS) & MPU3050_PWR_MGM_MASK) ^
		 MPU3050_PWR_MGM_MASK);
		 
	i2c_smbus_write_byte_data(client, MPU3050_PWR_MGM, value);
}

/**
 *	mpu3050_input_open	-	called on input event open
 *	@input: input dev of opened device
 *
 *	The input layer calls this function when input event is opened. The
 *	function will push the device to resume. Then, the device is ready
 *	to provide data.
 */
static int mpu3050_input_open(struct input_dev* input)
{
	struct mpu3050_data* sensor = input_get_drvdata(input);
	pm_runtime_get(sensor->dev);
	
	return 0;
}

/**
 *	mpu3050_input_close	-	called on input event close
 *	@input: input dev of closed device
 *
 *	The input layer calls this function when input event is closed. The
 *	function will push the device to suspend.
 */
static void mpu3050_input_close(struct input_dev* input)
{
	struct mpu3050_data *sensor = input_get_drvdata(input);
	pm_runtime_put(sensor->dev);
}

/**
 *	mpu3050_interrupt_thread	-	handle an IRQ
 *	@irq: interrupt numner
 *	@data: the sensor
 *
 *	Called by the kernel single threaded after an interrupt occurs. Read
 *	the sensor data and generate an input event for it.
 */
static irqreturn_t mpu3050_interrupt_thread(int irq, void* data)
{
    struct i2c_client*   client = (struct i2c_client*)data;
    struct mpu3050_data* sensor = i2c_get_clientdata(client);
	struct axis_data axis;
    
	mutex_lock(&sensor->lock);
	mpu3050_read_xyz(client, &axis);
	mutex_unlock(&sensor->lock);

	input_report_abs(sensor->idev, ABS_X, axis.x);
	input_report_abs(sensor->idev, ABS_Y, axis.y);
	input_report_abs(sensor->idev, ABS_Z, axis.z);
	
	input_sync(sensor->idev);

	return IRQ_HANDLED;
}



/**
 *	mpu3050_register_input_device	-	register input dev
 *	@sensor: sensor to remove from input
 *
 *	Add an input device to the sensor. This will be used to report
 *	events from the sensor itself.
 */
static int mpu3050_register_input_device(struct i2c_client* client)
{
	struct input_dev* idev;
	int ret;
	struct mpu3050_data* sensor = i2c_get_clientdata(client);
	
	sensor->idev = input_allocate_device();
	idev = sensor->idev;
	
	if (!idev) 
	{
		dev_err(&client->dev, "failed to allocate input device\n");
		ret = -ENOMEM;
		goto failed_alloc;
	}
	
	idev->name = MPU3050_DRV_NAME;
	
	idev->open       = mpu3050_input_open;
	idev->close      = mpu3050_input_close;
	idev->id.bustype = BUS_I2C;
	idev->dev.parent = &client->dev;
	idev->evbit[0]   = BIT_MASK(EV_ABS);
	
	input_set_abs_params(idev, ABS_X, MPU3050_MIN_VALUE,
			MPU3050_MAX_VALUE, 0, 0);
			
	input_set_abs_params(idev, ABS_Y, MPU3050_MIN_VALUE,
			MPU3050_MAX_VALUE, 0, 0);
			
	input_set_abs_params(idev, ABS_Z, MPU3050_MIN_VALUE,
			MPU3050_MAX_VALUE, 0, 0);
			
	input_set_drvdata(idev, sensor);
	ret = input_register_device(idev);
	
	if (ret) 
	{
		dev_err(&client->dev, "failed to register input device\n");
		goto failed_reg;
	}
	
	ret = request_irq(client->irq, mpu3050_interrupt_thread, IRQF_TRIGGER_RISING,
    				MPU3050_DRV_NAME, client);
						
	if (ret)
	{
		dev_err(&client->dev, "can't get IRQ %d, ret %d\n",
				client->irq, ret);
				
		goto failed_irq;
	}
	
	printk("++++ mpu3050 Interrupt Registered!. IRQ line: %d++++\n", client->irq);
	return 0;
	
failed_irq:
	input_unregister_device(idev);
	return ret;
	
failed_reg:
	if (idev)
		input_free_device(idev);
		
failed_alloc:
	return ret;
}


/* cmd line interface for purpose of power management */
#ifdef PROC_DEBUG
   #define MAX_PROC_BUF_SIZE  32
   #define PROC_ENTRY_MPU3050  MPU3050_DRV_NAME
static struct proc_dir_entry* proc_mpu;


static int proc_debug_mpu(struct file* file, const char __user* buffer, unsigned long count, void* data)
{
	int rc, num_args, cmd;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];
	struct axis_data coords;
	struct i2c_client* client = (struct i2c_client*)data;
	struct mpu3050_data* sensor = i2c_get_clientdata(client);

	if (count > MAX_PROC_BUF_SIZE) 
	{
		count = MAX_PROC_BUF_SIZE;
	}

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) 
	{
		printk(KERN_ERR "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	num_args = sscanf(kbuf, "%i", &cmd);
	if (num_args != 1) 
	{
		printk(KERN_ERR "echo <cmd> > %s\n", PROC_ENTRY_MPU3050);
		return count;
	}
	
	switch (cmd)
	{
	case 0: 
		printk(KERN_ERR "CMD MODE_SLEEP\n");
		mpu3050_set_power_mode(client, 0); 
		 
		break;
		
	case 1:
		printk(KERN_ERR "CMD MODE_NORMAL\n");
		mpu3050_set_power_mode(client, 1);
		msleep_interruptible(100);  /* wait for gyro chip resume */
		
		break;
		
	case 2:
		coords.x = coords.y = coords.z = 0;
		mpu3050_read_xyz(client, &coords);
		printk(KERN_INFO "CMD MODE READ x: %d, y: %d, z: %d\n", coords.x, coords.y, coords.z);
		
		break;
		
	default:
		printk(KERN_ERR "CMD MODE %d UNSUPPORTED\n", cmd);
		return -EFAULT;
	}
	return count;
}
#endif


/**
 *	mpu3050_probe	-	device detection callback
 *	@client: i2c client of found device
 *	@id: id match information
 *
 *	The I2C layer calls us when it believes a sensor is present at this
 *	address. Probe to see if this is correct and to validate the device.
 *
 *	If present install the relevant sysfs interfaces and input device.
 */
static int __devinit mpu3050_probe
(
   struct i2c_client*          client,
   const struct i2c_device_id* id
)
{
	struct mpu3050_data* sensor = 0;
	int ret = 0;
	
	sensor = kzalloc(sizeof(struct mpu3050_data), GFP_KERNEL);
	if (!sensor) 
	{
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}
	
	sensor->dev = &client->dev;
	i2c_set_clientdata(client, sensor);

    
	/* maps GPIO to IRQ */
	if (client->dev.platform_data != NULL)
	{
		struct mpu3050_platform_data* mpd = (struct mpu3050_platform_data*) client->dev.platform_data;
		client->irq = gpio_to_irq(mpd->gpio_irq_pin);
		
		ret = gpio_request(mpd->gpio_irq_pin, client->name);
		if (ret)
			dev_err(&client->dev, "failed to request GPIO\n");
		else
			dev_info(&client->dev, "GPIO %d <-> IRQ %d\n", mpd->gpio_irq_pin, client->irq);
	}

	mpu3050_set_power_mode(client, 1);
	msleep_interruptible(10);

	ret = i2c_smbus_read_byte_data(client, MPU3050_CHIP_ID_REG);
	if (ret < 0) 
	{
		dev_err(&client->dev, "failed to detect device\n");
		goto failed_free;
	}
	
	if (ret != MPU3050_CHIP_ID)
	{
		dev_err(&client->dev, "unsupported chip id\n");
		goto failed_free;
	}
	
	mutex_init(&sensor->lock);

	ret = mpu3050_register_input_device(client);
	if (ret)
		dev_err(&client->dev, "only provide sysfs\n");

	pm_runtime_enable(&client->dev);
	pm_runtime_set_active(&client->dev);
	
// TODO uncomment next line
//	pm_runtime_set_autosuspend_delay(&client->dev, MPU3050_AUTO_DELAY);

	dev_info(&client->dev, "%s registered\n", id->name); 

#ifdef PROC_DEBUG
	proc_mpu = create_proc_entry(PROC_ENTRY_MPU3050, 0644, NULL);
	if (proc_mpu == NULL)
	{
		printk(KERN_ERR "mpu3050/debug driver procfs failed\n");
		ret = -ENOMEM;
		
		goto failed_free;
	}
	
	proc_mpu->write_proc = proc_debug_mpu;
	proc_mpu->data       = client;
	
	printk(KERN_INFO "mpu3050/debug driver procfs OK\n");
#endif
    
    // register with BRVSENS driver 
    brvsens_register(SENSOR_HANDLE_GYROSCOPE, 
                MPU3050_DRV_NAME, 
                (void*)client, 
                (PFNACTIVATE)mpu3050_set_power_mode, 
                (PFNREAD)mpu3050_read_xyz);
	return 0;

failed_free:
	kfree(sensor);
	return ret;
}

/**
 *	mpu3050_remove	-	remove a sensor
 *	@client: i2c client of sensor being removed
 *
 *	Our sensor is going away, clean up the resources.
 */
static int __devexit mpu3050_remove(struct i2c_client* client)
{
	struct mpu3050_data* data = i2c_get_clientdata(client);

	pm_runtime_disable(&(client->dev) );
	pm_runtime_set_suspended(&(client->dev) );

	if (data->idev)
	{
	   free_irq(client->irq, data);
	   input_unregister_device(data->idev);
	   data->idev = NULL;	
    }
		
	kfree(data);
	return 0;
}

#ifdef CONFIG_PM
/**
 *	mpu3050_suspend		-	called on device suspend
 *	@client: i2c client of sensor
 *	@mesg: actual suspend type
 *
 *	Put the device into sleep mode before we suspend the machine.
 */
static int mpu3050_suspend(struct i2c_client* client, pm_message_t mesg)
{
	mpu3050_set_power_mode(client, 0);
	return 0;
}

/**
 *	mpu3050_resume		-	called on device resume
 *	@client: i2c client of sensor
 *
 *	Put the device into powered mode on resume.
 */
static int mpu3050_resume(struct i2c_client* client)
{
	mpu3050_set_power_mode(client, 1);
	msleep_interruptible(100);  /* wait for gyro chip resume */
	
	return 0;
}
#else
   #define mpu3050_suspend NULL
   #define mpu3050_resume NULL
#endif


static const struct i2c_device_id mpu3050_ids[] = 
{
	{ SENSOR_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mpu3050_ids);

static struct i2c_driver mpu3050_i2c_driver = 
{
	.driver	= 
	{
		.name	= SENSOR_NAME, 
	},
	.probe		= mpu3050_probe,
	.remove		= __devexit_p(mpu3050_remove),
	.suspend	= mpu3050_suspend,
	.resume		= mpu3050_resume,
	.id_table	= mpu3050_ids,
};

static int __init mpu3050_init(void)
{
	return i2c_add_driver(&mpu3050_i2c_driver);
}


static void __exit mpu3050_exit(void)
{
	i2c_del_driver(&mpu3050_i2c_driver);
}


module_init(mpu3050_init);
module_exit(mpu3050_exit);

MODULE_AUTHOR("Wistron Corp.");
MODULE_DESCRIPTION("MPU3050 Tri-axis gyroscope driver");
MODULE_LICENSE("GPL");
