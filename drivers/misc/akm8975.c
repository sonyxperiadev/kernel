/*****************************************************************************
* Copyright (c) 2011 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/


#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/input.h>
<<<<<<< HEAD
#include <linux/delay.h>

=======
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/freezer.h>
>>>>>>> mps-lmp
#include <linux/akm8975.h>

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

<<<<<<< HEAD
=======
#define AK8975DRV_DATA_DBG 0
#define MAX_FAILURE_COUNT 10
#define AK8975_MAX_CONVERSION_TIMEOUT	500

struct akm8975_data {
	struct i2c_client *this_client;
	struct akm8975_platform_data *pdata;
	struct input_dev *input_dev;
	struct completion data_ready;
	struct mutex flags_lock;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

/*
* Because misc devices can not carry a pointer from driver register to
* open, we keep this global. This limits the driver to a single instance.
*/
struct akm8975_data *akmd_data;
>>>>>>> mps-lmp

/* Data Buffer Structure definition */
struct akm8975_measure
{
	s16	x,
		y,
		z;
};


<<<<<<< HEAD
/* Client Data */
struct akm8975_data 
{
	struct i2c_client*  this_client;
	struct input_dev*   input;
};
=======
static short mv_flag;
>>>>>>> mps-lmp


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

<<<<<<< HEAD
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

=======

/*
 * Helper function to write to the I2C device's registers.
 */
static int akm8975_i2c_txdata(struct akm8975_data *akm, u8 reg, u8 val)
{
	struct i2c_msg msg;
	u8 w_data[2];
	int ret = 0;
 
	w_data[0] = reg;
	w_data[1] = val;

	msg.addr = akm->this_client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = w_data;

	FUNCDBG("called");

	ret = i2c_transfer(akm->this_client->adapter, &msg, 1);

	if (ret < 0) {
		dev_err(&akm->this_client->dev, "Write to device fails status %x\n", ret);
 	}
 
	return ret;
}
 
/*
 * Helper function to read a contiguous set of the I2C device's registers.
 */
static int akm8975_i2c_rxdata(struct akm8975_data *akm,
			    u8 reg, u8 length, u8 *buffer)
{
	struct i2c_msg msg[2];
	u8 w_data[2];
	int ret;

	w_data[0] = reg;

	msg[0].addr = akm->this_client->addr;
	msg[0].flags = 0;	 
	msg[0].len = 1;
	msg[0].buf = w_data;

	msg[1].addr = akm->this_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = buffer;

	ret = i2c_transfer(akm->this_client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&akm->this_client->dev, "Read from device fails\n");
		return ret;
	}

	return 0;
}


static int akm8975_ecs_get_data(struct akm8975_data *akm, u8 length, char *buffer)
{
	memset(buffer, 0, length);
	
	return akm8975_i2c_rxdata(akm, AK8975_REG_ST1, length, buffer);
}

static int akm8975_ecs_set_mode(struct akm8975_data *akm, char mode)
{
	/* guarantee some delay between each mode switch
	 */
	msleep(1);
 	return akm8975_i2c_txdata(akm, AK8975_REG_CNTL, mode);
}


static void akm8975_ecs_report_value(struct akm8975_data *akm, int *rbuf)
{
	struct akm8975_data *data = i2c_get_clientdata(akm->this_client);

	FUNCDBG("called");

#if AK8975DRV_DATA_DBG
	pr_info("Magnetic:	 x = %d , y = %d , z = %d \n\n",
				 rbuf[0], rbuf[1], rbuf[2]);
#endif

	mutex_lock(&akm->flags_lock);

	if (mv_flag) {
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[0]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[1]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[2]);
	}
	mutex_unlock(&akm->flags_lock);

	input_sync(data->input_dev);
}

static void akm8975_ecs_close_done(struct akm8975_data *akm)
{
	FUNCDBG("called");
	mutex_lock(&akm->flags_lock);
	mv_flag = 1;
	mutex_unlock(&akm->flags_lock);
}
>>>>>>> mps-lmp

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
<<<<<<< HEAD
	struct input_dev* input = to_input_dev(dev);
	struct akm8975_data* akm = input_get_drvdata(input);
	struct akm8975_measure measure;
	
    int i = akm8975_measure_cbk(akm, &measure);
    if (i != 0)
        return sprintf(buf, "AKM8975 Error\n");
    
    // can not adjust sensitivity -- no float point math in Kernel
    return sprintf(buf, "X:%d Y:%d Z:%d\n",
      measure.x, measure.y, measure.z);
=======
	void __user *argp = (void __user *) arg;
	short flag;
	struct akm8975_data *akm = file->private_data;

 	FUNCDBG("called");

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MVFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	mutex_lock(&akm->flags_lock);
	switch (cmd) {
	case ECS_IOCTL_APP_SET_MVFLAG:
		mv_flag = flag;
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = mv_flag;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = akmd_delay;
		break;
	default:
		return -ENOTTY;
	}
	mutex_unlock(&akm->flags_lock);

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
>>>>>>> mps-lmp
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
<<<<<<< HEAD
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
=======
	struct akm8975_data *akm = file->private_data;

	FUNCDBG("called");
	akm8975_ecs_close_done(akm);
	return 0;
}

static int akmd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		      unsigned long arg)
{
	void __user *argp = (void __user *) arg;

	char rwbuf[16];
	char databuf[RBUFF_SIZE];
	int ret = -1;
	int status;
	int value[3];
	short delay;
	short mode;
	struct akm8975_data *akm = file->private_data;
	
	FUNCDBG("called");

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_MODE:
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;

	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if ((rwbuf[0] > AK8975_FUSE_ASAZ) || (rwbuf[1] < 1) ||
			(rwbuf[1] > 3)) {
			pr_err("AKM8975 IOCTL_READ invalid argument: %d, %d\n", 
				(int)rwbuf[0], (int)rwbuf[1]);
			return -EINVAL;
		}

		ret = akm8975_i2c_rxdata(akm, rwbuf[0], rwbuf[1], &rwbuf[2]);
		if (ret < 0)
			return ret;
		break;

	case ECS_IOCTL_WRITE:
		if (rwbuf[0] < 2)
			return -EINVAL;

		ret = akm8975_i2c_txdata(akm, rwbuf[0], rwbuf[1]);
		if (ret < 0)
			return ret;
		break;

	case ECS_IOCTL_SET_MODE:
		ret = akm8975_ecs_set_mode(akm, (char) mode);
		if (ret < 0)
			return ret;

		if (mode == AK8975_MODE_SNG_MEASURE) {
			/* wait for data to become ready */
			ret = wait_for_completion_interruptible_timeout(&akm->data_ready, 
						msecs_to_jiffies(AK8975_MAX_CONVERSION_TIMEOUT));
			if (ret < 0) {
				pr_err("AKM8975 conversion timeout happened\n");
				return -EINVAL;
			}
		}
		break;

	case ECS_IOCTL_GETDATA:
		ret = akm8975_ecs_get_data(akm, RBUFF_SIZE, databuf);
		if (ret < 0)
			return ret;

	case ECS_IOCTL_SET_YPR:
		akm8975_ecs_report_value(akm, value);
		break;

	case ECS_IOCTL_GET_OPEN_STATUS:
		wait_event_interruptible(open_wq,
					 (atomic_read(&open_flag) != 0));
		status = atomic_read(&open_flag);
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
		wait_event_interruptible(open_wq,
					 (atomic_read(&open_flag) == 0));
		status = atomic_read(&open_flag);
		break;

	case ECS_IOCTL_GET_DELAY:
		delay = akmd_delay;
		break;

	default:
		FUNCDBG("Unknown cmd\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if (copy_to_user(argp, &rwbuf[2], rwbuf[1]))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		if (copy_to_user(argp, &databuf, RBUFF_SIZE))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_DELAY:
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static irqreturn_t akm8975_interrupt(int irq, void *dev_id)
{
	struct akm8975_data *akm = dev_id;
	FUNCDBG("called");

	complete(&akm->data_ready);

	return IRQ_HANDLED;
}
>>>>>>> mps-lmp

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
<<<<<<< HEAD
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
=======
	struct akm8975_data *data;
	int gpio_drdy; 
	int ret;

	data = i2c_get_clientdata(client);

	ret = request_irq(client->irq, akm8975_interrupt, IRQF_TRIGGER_RISING,
				"akm8975", data);
	if (ret < 0) {
		pr_err("akm8975_init_client: request irq failed\n");
		goto err;
	}

	/* set up the Data Ready GPIO pin */
	gpio_drdy = irq_to_gpio(client->irq);
	if (!gpio_drdy) {
		pr_err("akm8975_init_client: no valid GPIO for data ready\n");
		ret = -EINVAL;
		goto err;
	}
	
	ret = gpio_request(gpio_drdy, "akm_8975_drdy");
	if (ret < 0) {
		pr_err("akm8975_init_client: request GPIO %d failed, err %d\n",
			gpio_drdy, ret);
		goto err;
	}
	
	ret = gpio_direction_input(gpio_drdy);
	if (ret < 0) {
		pr_err("akm8975_init_client: set GPIO %d as input failed, err %d\n",
			gpio_drdy, ret);
		gpio_free(gpio_drdy);
		goto err;
	}
	
	init_waitqueue_head(&open_wq);

	mutex_lock(&data->flags_lock);
	mv_flag = 1;
	mutex_unlock(&data->flags_lock);

	return 0;
err:
  return ret;
}

static const struct file_operations akmd_fops = {
	.owner = THIS_MODULE,
	.open = akmd_open,
	.release = akmd_release,
	.ioctl = akmd_ioctl,
};

static const struct file_operations akm_aot_fops = {
	.owner = THIS_MODULE,
	.open = akm_aot_open,
	.release = akm_aot_release,
	.ioctl = akm_aot_ioctl,
};

static struct miscdevice akm_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8975_aot",
	.fops = &akm_aot_fops,
};

static struct miscdevice akmd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8975_dev",
	.fops = &akmd_fops,
};

int akm8975_probe(struct i2c_client *client,
		  const struct i2c_device_id *devid)
{
	struct akm8975_data *akm;
	int err;
	FUNCDBG("called");

	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		err = -ENODEV;
		goto exit_platform_data_null;
>>>>>>> mps-lmp
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
<<<<<<< HEAD
	
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
=======

	akm->pdata = client->dev.platform_data;

	mutex_init(&akm->flags_lock);
	init_completion(&akm->data_ready);

	i2c_set_clientdata(client, akm);

	err = akm8975_power_on(akm);
	if (err < 0)
		goto exit_power_on_failed;

	akm8975_init_client(client);
	akm->this_client = client;
	akmd_data = akm;

	akm->input_dev = input_allocate_device();
	if (!akm->input_dev) {
		err = -ENOMEM;
		dev_err(&akm->this_client->dev,
			"input device allocate failed\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, akm->input_dev->evbit);

	/* x-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0X, -4096, 4095, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0Y, -4096, 4095, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_BRAKE, -4096, 4095, 0, 0);

	akm->input_dev->name = "compass";

	err = input_register_device(akm->input_dev);
	if (err) {
		pr_err("akm8975_probe: Unable to register input device: %s\n",
					 akm->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&akmd_device);
	if (err) {
		pr_err("akm8975_probe: akmd_device register failed\n");
		goto exit_misc_device_register_failed;
>>>>>>> mps-lmp
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
<<<<<<< HEAD
	struct akm8975_data* akm = i2c_get_clientdata(client);
	FUNCDBG("Enter\n");
	
	sysfs_remove_group(&akm->input->dev.kobj, &akm8975_attribute_group);
	akm8975_input_delete(akm);
	
=======
	struct akm8975_data *akm = i2c_get_clientdata(client);
	FUNCDBG("called");

	gpio_free(irq_to_gpio(client->irq));	
	free_irq(client->irq, NULL);
	input_unregister_device(akm->input_dev);
	misc_deregister(&akmd_device);
	misc_deregister(&akm_aot_device);
	akm8975_power_off(akm);
>>>>>>> mps-lmp
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
