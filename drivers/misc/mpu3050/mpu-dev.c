/*
 $License:
    Copyright (C) 2011 InvenSense Corporation, All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  $
 */

/*
 * Modifications for Broadcom virtual sensor driver 2011/10/03
 */

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/signal.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/poll.h>

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include "mpuirq.h"
#include "slaveirq.h"
#include "mlsl.h"
#include "mpu-i2c.h"
#include "mldl_cfg.h"
#include "mpu.h"

#include <linux/brcm_axis_change.h>
#include <linux/brvsens_driver.h>

/* ---- Public Variables ------------------------------------------------- */
/* Debug flag settings:
#define SENSOR_HANDLE_ACCELEROMETER 0x01     // (1 << (SENSOR_TYPE_ACCELEROMETER - 1) )
#define SENSOR_HANDLE_COMPASS       0x02     // (1 << (SENSOR_TYPE_MAGNETIC_FIELD - 1) )
#define SENSOR_HANDLE_GYROSCOPE     0x08     // (1 << (SENSOR_TYPE_GYROSCOPE - 1) )
#define SENSOR_HANDLE_LIGHT         0x10     // (1 << (SENSOR_TYPE_LIGHT - 1) )
#define SENSOR_HANDLE_PRESSURE      0x20     // (1 << (SENSOR_TYPE_PRESSURE - 1) )
#define SENSOR_HANDLE_TEMPERATURE   0x40     // (1 << (SENSOR_TYPE_TEMPERATURE - 1) )
*/

static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);

static int compass_count = 0;
struct axis_data
{
	s16 x;
	s16 y;
	s16 z;
};

/* Platform data for the MPU */
struct mpu_private_data {
	struct miscdevice dev;
	struct i2c_client *client;
	struct mldl_cfg mldl_cfg;

	struct mutex mutex;
	wait_queue_head_t mpu_event_wait;
	struct completion completion;
	struct timer_list timeout;
	struct notifier_block nb;
	struct mpuirq_data mpu_pm_event;
	int response_timeout;	/* In seconds */
	unsigned long event;
	int pid;
};

/* Helper to switch values depending how sensor is mounted on the board */
static int mpu_switch_values(int               axis_dir,
                             struct axis_data *p_axis_old,
                             s16              *p_axis)
{
	switch (axis_dir)
	{
		case axis_x_dir:
			*p_axis = p_axis_old->x;
			break;
			
		case axis_y_dir:
			*p_axis = p_axis_old->y;
			break;
			
		case axis_z_dir:
			*p_axis = p_axis_old->z;
			break;
			
		case axis_x_dir_rev:
			*p_axis = -p_axis_old->x;
			break;
			
		case axis_y_dir_rev:
			*p_axis = -p_axis_old->y;
			break;
			
		case axis_z_dir_rev:
			*p_axis = -p_axis_old->z;
			break;
			
		default:
         printk("%s() axis_dir out of range: %d\n", __func__, axis_dir);
			return -1;
	}
	
	return 0;
}
static void mpu6050_change_orientation(struct axis_data* p_axis_orig_t,
                                       struct t_brcm_axis_change *p_axis_change)
{
	s16 temp_x = 0, temp_y = 0, temp_z = 0;

   if (p_axis_change == NULL)
   {
      return;
   }

	mpu_switch_values(p_axis_change->x_change, p_axis_orig_t, &temp_x);
	mpu_switch_values(p_axis_change->y_change, p_axis_orig_t, &temp_y);
	mpu_switch_values(p_axis_change->z_change, p_axis_orig_t, &temp_z);

	p_axis_orig_t->x = temp_x;
	p_axis_orig_t->y = temp_y;
	p_axis_orig_t->z = temp_z;
}

static s16 undo_twos_complement(u16 twos_comp)
{
   s16 temp;

   if (!(twos_comp & 0x8000))
   {  /* No change needed. */
      return (s16)twos_comp;
   }

   temp = ((~(twos_comp)) + 1)*-1;
   return temp;
}

static void mpu_pm_timeout(u_long data)
{
	struct mpu_private_data *mpu = (struct mpu_private_data *)data;
	struct i2c_client *client = mpu->client;
	dev_dbg(&client->adapter->dev, "%s\n", __func__);
	complete(&mpu->completion);
}

static int mpu_pm_notifier_callback(struct notifier_block *nb,
				    unsigned long event, void *unused)
{
	struct mpu_private_data *mpu =
	    container_of(nb, struct mpu_private_data, nb);
	struct i2c_client *client = mpu->client;
	struct timeval event_time;
	dev_dbg(&client->adapter->dev, "%s: %ld\n", __func__, event);

	/* Prevent the file handle from being closed before we initialize
	   the completion event */
	mutex_lock(&mpu->mutex);
	if (!(mpu->pid) ||
	    (event != PM_SUSPEND_PREPARE && event != PM_POST_SUSPEND)) {
		mutex_unlock(&mpu->mutex);
		return NOTIFY_OK;
	}

	do_gettimeofday(&event_time);
	mpu->mpu_pm_event.interruptcount++;
	mpu->mpu_pm_event.irqtime =
	    (((long long)event_time.tv_sec) << 32) + event_time.tv_usec;
	mpu->mpu_pm_event.data_type = MPUIRQ_DATA_TYPE_PM_EVENT;
	mpu->mpu_pm_event.data = mpu->event;

	if (event == PM_SUSPEND_PREPARE)
		mpu->event = MPU_PM_EVENT_SUSPEND_PREPARE;
	if (event == PM_POST_SUSPEND)
		mpu->event = MPU_PM_EVENT_POST_SUSPEND;

	if (mpu->response_timeout > 0) {
		mpu->timeout.expires = jiffies + mpu->response_timeout * HZ;
		add_timer(&mpu->timeout);
	}
	INIT_COMPLETION(mpu->completion);
	mutex_unlock(&mpu->mutex);

	wake_up_interruptible(&mpu->mpu_event_wait);
	wait_for_completion(&mpu->completion);
	del_timer_sync(&mpu->timeout);
	dev_dbg(&client->adapter->dev, "%s: %ld DONE\n", __func__, event);
	return NOTIFY_OK;
}

static int mpu_dev_open(struct inode *inode, struct file *file)
{
	struct mpu_private_data *mpu =
	    container_of(file->private_data, struct mpu_private_data, dev);
	struct i2c_client *client = mpu->client;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	int result;
	dev_dbg(&client->adapter->dev, "%s\n", __func__);
	dev_dbg(&client->adapter->dev, "current->pid %d\n", current->pid);
	mpu->pid = current->pid;
	/* we could do some checking on the flags supplied by "open" */
	/* i.e. O_NONBLOCK */
	/* -> set some flag to disable interruptible_sleep_on in mpu_read */

	/* Reset the sensors to the default */
	result = mutex_lock_interruptible(&mpu->mutex);
	if (result) {
		dev_err(&client->adapter->dev,
			"%s: mutex_lock_interruptible returned %d\n",
			__func__, result);
		return result;
	}
	mldl_cfg->requested_sensors = INV_THREE_AXIS_GYRO;
	if (mldl_cfg->accel && mldl_cfg->accel->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_ACCEL;

	if (mldl_cfg->compass && mldl_cfg->compass->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_COMPASS;

	if (mldl_cfg->pressure && mldl_cfg->pressure->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_PRESSURE;
	mutex_unlock(&mpu->mutex);
	return 0;
}

/* close function - called when the "file" /dev/mpu is closed in userspace   */
static int mpu_release(struct inode *inode, struct file *file)
{
	struct mpu_private_data *mpu =
	    container_of(file->private_data, struct mpu_private_data, dev);
	struct i2c_client *client = mpu->client;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;
	int result = 0;

	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	mutex_lock(&mpu->mutex);
	mldl_cfg->requested_sensors = 0;
	result = inv_mpu_suspend(mldl_cfg, client->adapter,
				 accel_adapter, compass_adapter,
				 pressure_adapter, INV_ALL_SENSORS);
	mpu->pid = 0;
	mutex_unlock(&mpu->mutex);
	complete(&mpu->completion);
	dev_dbg(&client->adapter->dev, "mpu_release\n");
	return result;
}

/* read function called when from /dev/mpu is read.  Read from the FIFO */
static ssize_t mpu_read(struct file *file,
			char __user *buf, size_t count, loff_t *offset)
{
	struct mpu_private_data *mpu =
	    container_of(file->private_data, struct mpu_private_data, dev);
	struct i2c_client *client = mpu->client;
	size_t len = sizeof(mpu->mpu_pm_event) + sizeof(unsigned long);
	int err;

	if (!mpu->event && (!(file->f_flags & O_NONBLOCK)))
		wait_event_interruptible(mpu->mpu_event_wait, mpu->event);

	if (!mpu->event || NULL == buf
	    || count < sizeof(mpu->mpu_pm_event) + sizeof(unsigned long))
		return 0;

	err = copy_to_user(buf, &mpu->mpu_pm_event, sizeof(mpu->mpu_pm_event));
	if (err != 0) {
		dev_err(&client->adapter->dev,
			"Copy to user returned %d\n", err);
		return -EFAULT;
	}
	mpu->event = 0;
	return len;
}

static unsigned int mpu_poll(struct file *file, struct poll_table_struct *poll)
{
	struct mpu_private_data *mpu =
	    container_of(file->private_data, struct mpu_private_data, dev);
	int mask = 0;

	poll_wait(file, &mpu->mpu_event_wait, poll);
	if (mpu->event)
		mask |= POLLIN | POLLRDNORM;
	return mask;
}

static int
mpu_dev_ioctl_set_mpu_pdata(struct i2c_client *client, unsigned long arg)
{
	int ii;
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mpu_platform_data *pdata = mpu->mldl_cfg.pdata;
	struct mpu_platform_data local_pdata;

	if (copy_from_user(&local_pdata, (unsigned char __user *)arg,
			   sizeof(local_pdata)))
		return -EFAULT;

	pdata->int_config = local_pdata.int_config;
	for (ii = 0; ii < DIM(pdata->orientation); ii++)
		pdata->orientation[ii] = local_pdata.orientation[ii];
	pdata->level_shifter = local_pdata.level_shifter;

	pdata->accel.address = local_pdata.accel.address;
	for (ii = 0; ii < DIM(pdata->accel.orientation); ii++)
		pdata->accel.orientation[ii] =
		    local_pdata.accel.orientation[ii];

	pdata->compass.address = local_pdata.compass.address;
	for (ii = 0; ii < DIM(pdata->compass.orientation); ii++)
		pdata->compass.orientation[ii] =
		    local_pdata.compass.orientation[ii];

	pdata->pressure.address = local_pdata.pressure.address;
	for (ii = 0; ii < DIM(pdata->pressure.orientation); ii++)
		pdata->pressure.orientation[ii] =
		    local_pdata.pressure.orientation[ii];

	dev_dbg(&client->adapter->dev, "%s\n", __func__);

	return INV_SUCCESS;
}

static int
mpu_dev_ioctl_set_mpu_config(struct i2c_client *client, unsigned long arg)
{
	int ii;
	int result = INV_SUCCESS;
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct mldl_cfg *temp_mldl_cfg;

	dev_dbg(&client->adapter->dev, "%s\n", __func__);

	temp_mldl_cfg = kzalloc(sizeof(struct mldl_cfg), GFP_KERNEL);
	if (NULL == temp_mldl_cfg)
		return -ENOMEM;

	/*
	 * User space is not allowed to modify accel compass pressure or
	 * pdata structs, as well as silicon_revision product_id or trim
	 */
	if (copy_from_user(temp_mldl_cfg, (struct mldl_cfg __user *)arg,
			   offsetof(struct mldl_cfg, silicon_revision))) {
		result = -EFAULT;
		goto out;
	}

	if (mldl_cfg->gyro_is_suspended) {
		if (mldl_cfg->addr != temp_mldl_cfg->addr)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->int_config != temp_mldl_cfg->int_config)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->ext_sync != temp_mldl_cfg->ext_sync)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->full_scale != temp_mldl_cfg->full_scale)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->lpf != temp_mldl_cfg->lpf)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->clk_src != temp_mldl_cfg->clk_src)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->divider != temp_mldl_cfg->divider)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->dmp_enable != temp_mldl_cfg->dmp_enable)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->fifo_enable != temp_mldl_cfg->fifo_enable)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->dmp_cfg1 != temp_mldl_cfg->dmp_cfg1)
			mldl_cfg->gyro_needs_reset = TRUE;

		if (mldl_cfg->dmp_cfg2 != temp_mldl_cfg->dmp_cfg2)
			mldl_cfg->gyro_needs_reset = TRUE;

		for (ii = 0; ii < MPU_NUM_AXES; ii++)
			if (mldl_cfg->offset_tc[ii] !=
			    temp_mldl_cfg->offset_tc[ii])
				mldl_cfg->gyro_needs_reset = TRUE;

		for (ii = 0; ii < MPU_NUM_AXES; ii++)
			if (mldl_cfg->offset[ii] != temp_mldl_cfg->offset[ii])
				mldl_cfg->gyro_needs_reset = TRUE;

		if (memcmp(mldl_cfg->ram, temp_mldl_cfg->ram,
			   MPU_MEM_NUM_RAM_BANKS * MPU_MEM_BANK_SIZE *
			   sizeof(unsigned char)))
			mldl_cfg->gyro_needs_reset = TRUE;
	}

	memcpy(mldl_cfg, temp_mldl_cfg,
	       offsetof(struct mldl_cfg, silicon_revision));

 out:
	kfree(temp_mldl_cfg);
	return result;
}

static int
mpu_dev_ioctl_get_mpu_config(struct i2c_client *client, unsigned long arg)
{
	/* Have to be careful as there are 3 pointers in the mldl_cfg
	 * structure */
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct mldl_cfg *local_mldl_cfg;
	int retval = 0;

	local_mldl_cfg = kzalloc(sizeof(struct mldl_cfg), GFP_KERNEL);
	if (NULL == local_mldl_cfg)
		return -ENOMEM;

	retval =
	    copy_from_user(local_mldl_cfg, (struct mldl_cfg __user *)arg,
			   sizeof(struct mldl_cfg));
	if (retval) {
		dev_err(&client->adapter->dev,
			"%s|%s:%d: EFAULT on arg\n",
			__FILE__, __func__, __LINE__);
		retval = -EFAULT;
		goto out;
	}

	/* Fill in the accel, compass, pressure and pdata pointers */
	if (mldl_cfg->accel) {
		retval = copy_to_user((void __user *)local_mldl_cfg->accel,
				      mldl_cfg->accel,
				      sizeof(*mldl_cfg->accel));
		if (retval) {
			dev_err(&client->adapter->dev,
				"%s|%s:%d: EFAULT on accel\n",
				__FILE__, __func__, __LINE__);
			retval = -EFAULT;
			goto out;
		}
	}

	if (mldl_cfg->compass) {
		retval = copy_to_user((void __user *)local_mldl_cfg->compass,
				      mldl_cfg->compass,
				      sizeof(*mldl_cfg->compass));
		if (retval) {
			dev_err(&client->adapter->dev,
				"%s|%s:%d: EFAULT on compass\n",
				__FILE__, __func__, __LINE__);
			retval = -EFAULT;
			goto out;
		}
	}

	if (mldl_cfg->pressure) {
		retval = copy_to_user((void __user *)local_mldl_cfg->pressure,
				      mldl_cfg->pressure,
				      sizeof(*mldl_cfg->pressure));
		if (retval) {
			dev_err(&client->adapter->dev,
				"%s|%s:%d: EFAULT on pressure\n",
				__FILE__, __func__, __LINE__);
			retval = -EFAULT;
			goto out;
		}
	}

	if (mldl_cfg->pdata) {
		retval = copy_to_user((void __user *)local_mldl_cfg->pdata,
				      mldl_cfg->pdata,
				      sizeof(*mldl_cfg->pdata));
		if (retval) {
			dev_err(&client->adapter->dev,
				"%s|%s:%d: EFAULT on pdata\n",
				__FILE__, __func__, __LINE__);
			retval = -EFAULT;
			goto out;
		}
	}

	/* Do not modify the accel, compass, pressure and pdata pointers */
	retval = copy_to_user((struct mldl_cfg __user *)arg,
			      mldl_cfg, offsetof(struct mldl_cfg, accel));

	if (retval)
		retval = -EFAULT;
 out:
	kfree(local_mldl_cfg);
	return retval;
}

/**
 * Pass a requested slave configuration to the slave sensor
 *
 * @param adapter the adaptor to use to communicate with the slave
 * @param mldl_cfg the mldl configuration structuer
 * @param slave pointer to the slave descriptor
 * @param usr_config The configuration to pass to the slave sensor
 *
 * @return 0 or non-zero error code
 */
static int slave_config(struct mldl_cfg *mldl_cfg,
			void *gyro_adapter,
			void *slave_adapter,
			struct ext_slave_descr *slave,
			struct ext_slave_platform_data *pdata,
			struct ext_slave_config __user *usr_config)
{
	int retval = INV_SUCCESS;
	struct ext_slave_config config;
	if ((!slave) || (!slave->config))
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;

	retval = copy_from_user(&config, usr_config, sizeof(config));
	if (retval)
		return -EFAULT;

	if (config.len && config.data) {
		int *data;
		data = kzalloc(config.len, GFP_KERNEL);
		if (!data)
			return INV_ERROR_MEMORY_EXAUSTED;

		retval = copy_from_user(data,
					(void __user *)config.data, config.len);
		if (retval) {
			retval = -EFAULT;
			kfree(data);
			return retval;
		}
		config.data = data;
	}
	retval = inv_mpu_slave_config(mldl_cfg, gyro_adapter, slave_adapter,
				      &config, slave, pdata);
	kfree(config.data);
	return retval;
}

/**
 * Get a requested slave configuration from the slave sensor
 *
 * @param adapter the adaptor to use to communicate with the slave
 * @param mldl_cfg the mldl configuration structuer
 * @param slave pointer to the slave descriptor
 * @param usr_config The configuration for the slave to fill out
 *
 * @return 0 or non-zero error code
 */
static int slave_get_config(struct mldl_cfg *mldl_cfg,
			    void *gyro_adapter,
			    void *slave_adapter,
			    struct ext_slave_descr *slave,
			    struct ext_slave_platform_data *pdata,
			    struct ext_slave_config __user *usr_config)
{
	int retval = INV_SUCCESS;
	struct ext_slave_config config;
	void *user_data;
	if (!(slave) || !(slave->get_config))
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;

	retval = copy_from_user(&config, usr_config, sizeof(config));
	if (retval)
		return -EFAULT;

	user_data = config.data;
	if (config.len && config.data) {
		int *data;
		data = kzalloc(config.len, GFP_KERNEL);
		if (!data)
			return INV_ERROR_MEMORY_EXAUSTED;

		retval = copy_from_user(data,
					(void __user *)config.data, config.len);
		if (retval) {
			retval = -EFAULT;
			kfree(data);
			return retval;
		}
		config.data = data;
	}
	retval = inv_mpu_get_slave_config(mldl_cfg, gyro_adapter,
					  slave_adapter, &config, slave, pdata);
	if (retval) {
		kfree(config.data);
		return retval;
	}
	retval = copy_to_user((unsigned char __user *)user_data,
			      config.data, config.len);
	kfree(config.data);
	return retval;
}

static int inv_slave_read(struct mldl_cfg *mldl_cfg,
			  void *gyro_adapter,
			  void *slave_adapter,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata,
			  void __user *usr_data)
{
	int retval;
	unsigned char *data;
	data = kzalloc(slave->read_len, GFP_KERNEL);
	if (!data)
		return -EFAULT;

	retval = inv_mpu_slave_read(mldl_cfg, gyro_adapter, slave_adapter,
				    slave, pdata, data);

	if ((INV_SUCCESS == retval) &&
	    (copy_to_user((unsigned char __user *)usr_data,
			  data, slave->read_len)))
		retval = -EFAULT;

	kfree(data);
	return retval;
}

static int mpu_handle_mlsl(void *sl_handle,
			   unsigned char addr,
			   unsigned int cmd,
			   struct mpu_read_write __user *usr_msg)
{
	int retval = INV_SUCCESS;
	struct mpu_read_write msg;
	unsigned char *user_data;
	retval = copy_from_user(&msg, usr_msg, sizeof(msg));
	if (retval)
		return -EFAULT;

	user_data = msg.data;
	if (msg.length && msg.data) {
		unsigned char *data;
		data = kzalloc(msg.length, GFP_KERNEL);
		if (!data)
			return INV_ERROR_MEMORY_EXAUSTED;

		retval = copy_from_user(data,
					(void __user *)msg.data, msg.length);
		if (retval) {
			retval = -EFAULT;
			kfree(data);
			return retval;
		}
		msg.data = data;
	} else {
		return INV_ERROR_INVALID_PARAMETER;
	}

	switch (cmd) {
	case MPU_READ:
		retval = inv_serial_read(sl_handle, addr,
					 msg.address, msg.length, msg.data);
		break;
	case MPU_WRITE:
		retval = inv_serial_write(sl_handle, addr,
					  msg.length, msg.data);
		break;
	case MPU_READ_MEM:
		retval = inv_serial_read_mem(sl_handle, addr,
					     msg.address, msg.length, msg.data);
		break;
	case MPU_WRITE_MEM:
		retval = inv_serial_write_mem(sl_handle, addr,
					      msg.address, msg.length,
					      msg.data);
		break;
	case MPU_READ_FIFO:
		retval = inv_serial_read_fifo(sl_handle, addr,
					      msg.length, msg.data);
		break;
	case MPU_WRITE_FIFO:
		retval = inv_serial_write_fifo(sl_handle, addr,
					       msg.length, msg.data);
		break;

	};
	retval = copy_to_user((unsigned char __user *)user_data,
			      msg.data, msg.length);
	kfree(msg.data);
	return retval;
}

/* ioctl - I/O control */
static long mpu_dev_ioctl(struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	struct mpu_private_data *mpu =
	    container_of(file->private_data, struct mpu_private_data, dev);
	struct i2c_client *client = mpu->client;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	int retval = 0;
	struct i2c_adapter *gyro_adapter;
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;

	gyro_adapter = client->adapter;
	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	retval = mutex_lock_interruptible(&mpu->mutex);
	if (retval) {
		dev_err(&client->adapter->dev,
			"%s: mutex_lock_interruptible returned %d\n",
			__func__, retval);
		return retval;
	}

	switch (cmd) {
	case MPU_SET_MPU_CONFIG:
		retval = mpu_dev_ioctl_set_mpu_config(client, arg);
		break;
	case MPU_SET_PLATFORM_DATA:
		retval = mpu_dev_ioctl_set_mpu_pdata(client, arg);
		break;
	case MPU_GET_MPU_CONFIG:
		retval = mpu_dev_ioctl_get_mpu_config(client, arg);
		break;
	case MPU_READ:
	case MPU_WRITE:
	case MPU_READ_MEM:
	case MPU_WRITE_MEM:
	case MPU_READ_FIFO:
	case MPU_WRITE_FIFO:
		retval = mpu_handle_mlsl(gyro_adapter, mldl_cfg->addr, cmd,
					 (struct mpu_read_write __user *)arg);
		break;
	case MPU_CONFIG_ACCEL:
		retval = slave_config(mldl_cfg,
				      gyro_adapter,
				      accel_adapter,
				      mldl_cfg->accel,
				      &mldl_cfg->pdata->accel,
				      (struct ext_slave_config __user *)arg);
		break;
	case MPU_CONFIG_COMPASS:
		retval = slave_config(mldl_cfg,
				      gyro_adapter,
				      compass_adapter,
				      mldl_cfg->compass,
				      &mldl_cfg->pdata->compass,
				      (struct ext_slave_config __user *)arg);
		break;
	case MPU_CONFIG_PRESSURE:
		retval = slave_config(mldl_cfg,
				      gyro_adapter,
				      pressure_adapter,
				      mldl_cfg->pressure,
				      &mldl_cfg->pdata->pressure,
				      (struct ext_slave_config __user *)arg);
		break;
	case MPU_GET_CONFIG_ACCEL:
		retval = slave_get_config(mldl_cfg,
					  gyro_adapter,
					  accel_adapter,
					  mldl_cfg->accel,
					  &mldl_cfg->pdata->accel,
					  (struct ext_slave_config __user *)
					  arg);
		break;
	case MPU_GET_CONFIG_COMPASS:
		retval = slave_get_config(mldl_cfg,
					  gyro_adapter,
					  compass_adapter,
					  mldl_cfg->compass,
					  &mldl_cfg->pdata->compass,
					  (struct ext_slave_config __user *)
					  arg);
		break;
	case MPU_GET_CONFIG_PRESSURE:
		retval = slave_get_config(mldl_cfg,
					  gyro_adapter,
					  pressure_adapter,
					  mldl_cfg->pressure,
					  &mldl_cfg->pdata->pressure,
					  (struct ext_slave_config __user *)
					  arg);
		break;
	case MPU_SUSPEND:
		retval = inv_mpu_suspend(mldl_cfg,
					 gyro_adapter,
					 accel_adapter,
					 compass_adapter,
					 pressure_adapter,
					 (~(mldl_cfg->requested_sensors))
					  & INV_ALL_SENSORS);
		break;
	case MPU_RESUME:
		retval = inv_mpu_resume(mldl_cfg,
					gyro_adapter,
					accel_adapter,
					compass_adapter,
					pressure_adapter,
					mldl_cfg->requested_sensors);
		break;
	case MPU_PM_EVENT_HANDLED:
		dev_dbg(&client->adapter->dev, "%s: %d\n", __func__, cmd);
		complete(&mpu->completion);
		break;
	case MPU_READ_ACCEL:
		{
			retval = inv_slave_read(mldl_cfg,
						gyro_adapter,
						accel_adapter,
						mldl_cfg->accel,
						&mldl_cfg->pdata->accel,
						(unsigned char __user *)arg);
		}
		break;
	case MPU_READ_COMPASS:
		{
			retval = inv_slave_read(mldl_cfg,
						gyro_adapter,
						compass_adapter,
						mldl_cfg->compass,
						&mldl_cfg->pdata->compass,
						(unsigned char __user *)arg);
		}
		break;
	case MPU_READ_PRESSURE:
		{
			retval = inv_slave_read(mldl_cfg,
						gyro_adapter,
						pressure_adapter,
						mldl_cfg->pressure,
						&mldl_cfg->pdata->pressure,
						(unsigned char __user *)arg);
		}
		break;
	default:
		dev_err(&client->adapter->dev,
			"%s: Unknown cmd %x, arg %lu: MIN %x MAX %x\n",
			__func__, cmd, arg,
			MPU_SET_MPU_CONFIG, MPU_SET_MPU_CONFIG);
		retval = -EINVAL;
	}

	mutex_unlock(&mpu->mutex);
	return retval;
}

void mpu_shutdown(struct i2c_client *client)
{
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;

	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	mutex_lock(&mpu->mutex);
	(void)inv_mpu_suspend(mldl_cfg, client->adapter,
			      accel_adapter, compass_adapter, pressure_adapter,
			      INV_ALL_SENSORS);
	mutex_unlock(&mpu->mutex);
	dev_dbg(&client->adapter->dev, "%s\n", __func__);
}

int mpu_dev_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;

	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	mutex_lock(&mpu->mutex);
	if (!mldl_cfg->ignore_system_suspend) {
		dev_dbg(&client->adapter->dev,
			"%s: suspending on event %d\n", __func__, mesg.event);
		(void)inv_mpu_suspend(mldl_cfg, client->adapter,
				      accel_adapter, compass_adapter,
				      pressure_adapter, INV_ALL_SENSORS);
	} else {
		dev_dbg(&client->adapter->dev,
			"%s: Already suspended %d\n", __func__, mesg.event);
	}
	mutex_unlock(&mpu->mutex);
	return 0;
}

int mpu_dev_resume(struct i2c_client *client)
{
	struct mpu_private_data *mpu =
	    (struct mpu_private_data *)i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;

	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	mutex_lock(&mpu->mutex);
	if (mpu->pid && !mldl_cfg->ignore_system_suspend) {
		(void)inv_mpu_resume(mldl_cfg, client->adapter,
				     accel_adapter,
				     compass_adapter,
				     pressure_adapter,
				     mldl_cfg->requested_sensors);
		dev_dbg(&client->adapter->dev,
			"%s for pid %d\n", __func__, mpu->pid);
	}
	mutex_unlock(&mpu->mutex);
	return 0;
}

/* define which file operations are supported */
static const struct file_operations mpu_fops = {
	.owner = THIS_MODULE,
	.read = mpu_read,
	.poll = mpu_poll,

#if HAVE_COMPAT_IOCTL
	.compat_ioctl = mpu_dev_ioctl,
#endif
#if HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = mpu_dev_ioctl,
#endif
	.open = mpu_dev_open,
	.release = mpu_release,
};

static unsigned short normal_i2c[] = { I2C_CLIENT_END };

static void mpu6050_set_power_mode(struct i2c_client* client, u8 val)
{
   int rc;

   struct pm_message mesg;
   if (val == 0)
   {  // Put the mpu6050 to sleep.
      mesg.event = PM_EVENT_SUSPEND;
      rc = mpu_dev_suspend(client, mesg);

      if (mod_debug)
      {
         printk("%s() putting %s to sleep\n", __func__, MPU_NAME);
      }
   }
   else
   {  // Wake the mpu6050.
      rc = mpu_dev_resume(client);
      if (mod_debug)
      {
         printk("%s() waking %s\n", __func__, MPU_NAME);
      }
   }

   if (mod_debug > 0)
      printk("%s() called, val %d rc %d\n", __func__, val, rc);

}

static void mpu6050_set_compass_power_mode(struct i2c_client* client, u8 val)
{
   if (mod_debug > 0)
      printk("%s() called, val %d\n", __func__, val);
}

static int mpu6050_xyz_read_reg(struct i2c_adapter *p_adapter,
                                unsigned char       slave_address,
                                int                 reg,
                                unsigned int        length,
                                unsigned char *     buffer)
{
   int rc;
   unsigned char       reg_name = (unsigned char)reg;

   rc = sensor_i2c_read(p_adapter,     /* struct i2c_adapter * */
                        slave_address, /* unsigned char address */
                        reg_name,      /* unsigned char reg */
                        length,        /* unsigned int len */
                        buffer);       /* unsigned char *data */
   if (mod_debug > 0xff)
      printk("%s() sensor_i2c_read() returned %d\n", __func__, rc);
   return rc;
}

static void mpu6050_read_gyro(struct i2c_client*  client,
                             struct axis_data*   coords)
{
	struct mpu_private_data *mpu = i2c_get_clientdata(client);
	struct i2c_adapter *gyro_adapter;
	struct i2c_adapter *accel_adapter;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
   int rc = 0;
   unsigned int  len     = 6;
	struct mpu_platform_data *p_mpu_pdata = mldl_cfg->pdata;

   struct t_brcm_sensors_axis_change *p_sensors_axis_change;

	u16 buffer[3] = { 0, 0, 0};

	gyro_adapter = client->adapter;
	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);

   mpu6050_xyz_read_reg(gyro_adapter,
                        client->addr,
                        MPUREG_GYRO_XOUT_H,
                        len,
                        (unsigned char *)buffer);

	coords->x = be16_to_cpu(buffer[0]);
	coords->y = be16_to_cpu(buffer[1]);
	coords->z = be16_to_cpu(buffer[2]);

   /* Undo the twos complement, 0xffff == -1! */
   coords->x = undo_twos_complement(coords->x);
   coords->y = undo_twos_complement(coords->y);
   coords->z = undo_twos_complement(coords->z);

   if (mod_debug & SENSOR_HANDLE_GYROSCOPE)
      printk("%s() called before,  x: %d, y: %d, z: %d, rc: %d\n",
             __func__, coords->x, coords->y, coords->z, rc);


   if (p_mpu_pdata->accel.private_data != NULL)
   {
      p_sensors_axis_change = (struct t_brcm_sensors_axis_change *)p_mpu_pdata->accel.private_data;
      mpu6050_change_orientation(coords,
                                 p_sensors_axis_change->p_gyro_axis_change);
   }

   if (mod_debug & SENSOR_HANDLE_GYROSCOPE)
      printk("%s() called after,  x: %d, y: %d, z: %d, rc: %d\n",
             __func__, coords->x, coords->y, coords->z, rc);
}  /* mpu6050_read_gyro(..) */

static void mpu6050_read_accel(struct i2c_client*  client,
                               struct axis_data*   coords)
{
	struct mpu_private_data *mpu = i2c_get_clientdata(client);
	struct i2c_adapter *gyro_adapter;
	struct i2c_adapter *accel_adapter;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct mpu_platform_data *p_mpu_pdata = mldl_cfg->pdata;

   struct t_brcm_sensors_axis_change *p_sensors_axis_change;

   int rc = 0;
   unsigned int  len     = 6;

	u16 buffer[3] = { 0, 0, 0};

	gyro_adapter = client->adapter;
	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);

   rc = mpu6050_xyz_read_reg(accel_adapter,
                             client->addr,
                             MPUREG_ACCEL_XOUT_H,
                             len,
                             (unsigned char *)buffer);

	coords->x = be16_to_cpu(buffer[0]);
	coords->y = be16_to_cpu(buffer[1]);
	coords->z = be16_to_cpu(buffer[2]);

   /* Undo the twos complement, 0xffff == -1! */
   coords->x = undo_twos_complement(coords->x);
   coords->y = undo_twos_complement(coords->y);
   coords->z = undo_twos_complement(coords->z);

   /* TODO: Tested on CapriStone and the x,y,z data are all 32 times larger than expected  */
   /* from Android ICS. Temporary divide by 32 to reduce the value here. In the future, we */
   /* need to find out how to config the accel sensor to reduce the data range. */
   coords->x >>= 5;
   coords->y >>= 5;
   coords->z >>= 5;
   

   if (p_mpu_pdata->accel.private_data != NULL)
   {
      p_sensors_axis_change = (struct t_brcm_sensors_axis_change *)p_mpu_pdata->accel.private_data;
      mpu6050_change_orientation(coords,
                                 p_sensors_axis_change->p_accel_axis_change);
   }

   if (mod_debug & SENSOR_HANDLE_ACCELEROMETER)
   {
      printk("%s() called, slave address: 0x%x, x: %d, y: %d, z: %d, rc: %d\n",
             __func__, client->addr, coords->x, coords->y, coords->z, rc);
   }
}  /* mpu6050_read_accel(..) */

void mpu6050_read_all_registers(struct i2c_client*  p_client)
{
	struct i2c_adapter *gyro_adapter;
   int i;

   unsigned int  len     = NUM_OF_MPU_REGISTERS;
   unsigned char data[NUM_OF_MPU_REGISTERS];
   int rc;

	gyro_adapter = p_client->adapter;

   rc = mpu6050_xyz_read_reg(gyro_adapter,
                             p_client->addr,
                             MPUREG_XG_OFFS_TC,
                             len,
                             data);
   if (rc != 0)
   {
      printk("%s() failed, mpu6050_xyz_read_reg() rc: %d\n", __func__, rc);
   }

   for (i = 0; i < NUM_OF_MPU_REGISTERS; i++)
   {
      printk("0x%2x %3d: 0x%x\n", i, i, data[i]);
   }
}

static int mpu6050_read_compass(struct i2c_client*  client,
                                struct axis_data*   coords)
{
   int rc = 0;
	struct mpu_private_data *mpu = i2c_get_clientdata(client);
	struct i2c_adapter *gyro_adapter;
	struct i2c_adapter *compass_adapter;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct mpu_platform_data *p_mpu_pdata = mldl_cfg->pdata;

   struct ext_slave_platform_data *p_slave_pdata;
   struct t_brcm_sensors_axis_change *p_sensors_axis_change;

   struct ext_slave_descr *p_slave_descr = mldl_cfg->compass;
   unsigned char *p_data;
	u16 buffer[3] = { 0, 0, 0};

   compass_count++;

   if (compass_count == 3 && mod_debug)
   {
      mpu6050_read_all_registers(client);
   }

	gyro_adapter    = client->adapter;
   compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
   p_slave_pdata = &p_mpu_pdata->compass;

   if (gyro_adapter == NULL)
   {
      printk("%s() gyro_adapter == NULL\n", __func__);
      return -1;
   }

   if (compass_adapter == NULL)
   {
      printk("%s() compass_adapter == NULL\n", __func__);
      return -1;
   }

	p_data = kzalloc(p_slave_descr->read_len, GFP_KERNEL);
	if (!p_data)
		return -EFAULT;

   rc = inv_mpu_slave_read(mldl_cfg,        /* struct mldl_cfg *mldl_cfg      */
                           gyro_adapter,    /* void *gyro_handle              */
                           compass_adapter, /* void *slave_handle             */
						         p_slave_descr,   /* struct ext_slave_descr         */
                           p_slave_pdata,   /* struct ext_slave_platform_data */
                           p_data);         /* unsigned char *data            */

   if (mod_debug & SENSOR_HANDLE_COMPASS)
   {
      printk("%s() called, count: %d length: %d, rc: %d x:0x%x 0x%x y:0x%x 0x%x z:0x%x 0x%x \n",
             __func__, compass_count, p_slave_descr->read_len, rc,
             p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5]);
   }

   if (rc == 0)
   {
      buffer[0] = (p_data[0] << 8)+p_data[1];
      buffer[1] = (p_data[2] << 8)+p_data[3];
      buffer[2] = (p_data[4] << 8)+p_data[5];

      buffer[0] = be16_to_cpu(buffer[0]);
      buffer[1] = be16_to_cpu(buffer[1]);
      buffer[2] = be16_to_cpu(buffer[2]);

      /* Undo the twos complement, 0xffff == -1! */
      coords->x = undo_twos_complement(buffer[0]);
      coords->y = undo_twos_complement(buffer[1]);
      coords->z = undo_twos_complement(buffer[2]);

      if (mod_debug & SENSOR_HANDLE_COMPASS)
      {
         printk("%s() called, before orientation, coords x:%d y:%d z:%d\n",
                __func__, coords->x, coords->y, coords->z);
      }

      if (p_mpu_pdata->accel.private_data != NULL)
      {
         p_sensors_axis_change = (struct t_brcm_sensors_axis_change *)p_mpu_pdata->accel.private_data;
         mpu6050_change_orientation(coords,
                                    p_sensors_axis_change->p_compass_axis_change);
      }
      else
      {
         if (mod_debug)
         {
            printk("%s() compass private data is NULL\n", __func__);
         }
      }
   }

   if (mod_debug & SENSOR_HANDLE_COMPASS)
   {
      printk("%s() called, coords x:%d y:%d z:%d\n",
             __func__, coords->x, coords->y, coords->z);
   }

   kfree(p_data);
   return rc;
}  /* mpu6050_read_compass(..) */

static int mpu6050_init(struct i2c_client*  client)
{
   int           i, rc;
   unsigned char address;
	struct i2c_adapter *p_i2c_adap;

   if (client == NULL)
   {
      return -1;
   }

   address = client->addr;

   /* Need to derefence the values that will be written into the registers. */
	struct mpu_private_data *mpu_priv_data = i2c_get_clientdata(client);
	struct mldl_cfg *mldl_cfg = &mpu_priv_data->mldl_cfg;
	struct mpu_platform_data *p_mpu_pdata = mldl_cfg->pdata;
   struct t_brcm_sensors_axis_change *p_sensors_axis_change;
   int    *p_data_reg_vals;

   unsigned char reg_values[7][2] =
   {
      //{register address, value to write}
      { MPUREG_PWR_MGMT_1,  0 }, /* 0x6b, 107 */
      { MPUREG_SMPLRT_DIV,  0 }, /* 0x19,  25 */
      { MPUREG_CONFIG,      0 }, /* 0x1a,  26 */
      { MPUREG_GYRO_CONFIG, 0 }, /* 0x1b,  27 */
      { MPUREG_ACCEL_CONFIG,0 }, /* 0x1c,  28 */
      { MPUREG_PWR_MGMT_1,  1 }, /* 0x6b, 107 */
      { MPUREG_PWR_MGMT_2,  0 }, /* 0x6C, 108 */
   };

   p_sensors_axis_change = (struct t_brcm_sensors_axis_change *)p_mpu_pdata->accel.private_data;
   p_data_reg_vals = (int *)p_sensors_axis_change->p_data;

   if (p_data_reg_vals != NULL)
   {
      for (i = 0; i < 7; i++)
      {
         reg_values[i][1] = p_data_reg_vals[i];
      }
   }

	p_i2c_adap = client->adapter;

   for (i = 0; i < 7; i++)
   {
      rc = sensor_i2c_write_register(p_i2c_adap,
                                     address,
			      //unsigned char reg, unsigned char value)
                                     reg_values[i][0],
                                     reg_values[i][1]);

      if (mod_debug)
      {
         printk("%s() i: %d sensor_i2c_write_register(0x%x, 0x%x) rc: %d\n",
                __func__, i, reg_values[i][0], reg_values[i][1], rc);
      }
   }

   return rc;
}

#ifdef I2C_CLIENT_INSMOD
I2C_CLIENT_INSMOD;
#endif

/*
 * In linux/mpu.h
 * struct ext_slave_descr {...};
 * struct ext_slave_platform_data {...};
 *
 * In kernel/drivers/misc/mpu3050/mldl_cfg.h
 * struct mldl_cfg {...};
 */

int mpu_probe(struct i2c_client *client, const struct i2c_device_id *devid)
{
	struct mpu_platform_data *pdata;
	struct mpu_private_data *mpu;
	struct mldl_cfg *mldl_cfg;
	int res = 0;
	struct i2c_adapter *accel_adapter = NULL;
	struct i2c_adapter *compass_adapter = NULL;
	struct i2c_adapter *pressure_adapter = NULL;

	dev_info(&client->adapter->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		res = -ENODEV;
		goto out_check_functionality_failed;
	}

	mpu = kzalloc(sizeof(struct mpu_private_data), GFP_KERNEL);
	if (!mpu) {
		res = -ENOMEM;
		goto out_alloc_data_failed;
	}

	i2c_set_clientdata(client, mpu);
	mpu->client = client;
	mldl_cfg = &mpu->mldl_cfg;

	init_waitqueue_head(&mpu->mpu_event_wait);

	mutex_init(&mpu->mutex);
	init_completion(&mpu->completion);

	mpu->response_timeout = 60;	/* Seconds */
	mpu->timeout.function = mpu_pm_timeout;
	mpu->timeout.data = (u_long) mpu;
	init_timer(&mpu->timeout);

	mpu->nb.notifier_call = mpu_pm_notifier_callback;
	mpu->nb.priority = 0;
	register_pm_notifier(&mpu->nb);

	pdata = (struct mpu_platform_data *)client->dev.platform_data;
	if (!pdata) {
		dev_WARN(&client->adapter->dev,
			 "Missing platform data for mpu\n");
	} else {
		mldl_cfg->pdata = pdata;

#if defined(CONFIG_INV_SENSORS_MODULE)
		pdata->accel.get_slave_descr = get_accel_slave_descr;
		pdata->compass.get_slave_descr = get_compass_slave_descr;
		pdata->pressure.get_slave_descr = get_pressure_slave_descr;
#endif

		if (pdata->accel.get_slave_descr) {
			mldl_cfg->accel = pdata->accel.get_slave_descr();
			dev_info(&client->adapter->dev,
				 "%s: +%s\n", MPU_NAME, mldl_cfg->accel->name);
			accel_adapter = i2c_get_adapter(pdata->accel.adapt_num);
			if (pdata->accel.irq > 0) {
				dev_info(&client->adapter->dev,
					 "Installing Accel irq using %d\n",
					 pdata->accel.irq);
				res = slaveirq_init(accel_adapter,
						    &pdata->accel, "accelirq");
				if (res)
					goto out_accelirq_failed;
			} else {
            /* Remove warning and trackeback that appears on console
				 * dev_WARN(&client->adapter->dev,
				 *	 "WARNING: Accel irq not assigned\n");
             */
            printk("%s() Warning,  Accel irq not assigned\n", __func__);
			}
		} else {
			dev_warn(&client->adapter->dev,
				 "%s: No Accel Present\n", MPU_NAME);
		}

		if (pdata->compass.get_slave_descr) {
			mldl_cfg->compass = pdata->compass.get_slave_descr();
			dev_info(&client->adapter->dev,
				 "%s: +%s\n", MPU_NAME,
				 mldl_cfg->compass->name);
			compass_adapter =
			    i2c_get_adapter(pdata->compass.adapt_num);

         if (compass_adapter == NULL)
         {
            printk("%s() compass_adapter == NULL\n", __func__);
      		goto out_compassirq_failed;
         }

			if (pdata->compass.irq > 0)
         {
				dev_info(&client->adapter->dev,
					 "Installing Compass irq using %d\n",
					 pdata->compass.irq);
				res = slaveirq_init(compass_adapter,
						    &pdata->compass,
						    "compassirq");
				if (res)
					goto out_compassirq_failed;
			} else {
				dev_warn(&client->adapter->dev,
					 "WARNING: Compass irq not assigned\n");
			}
		} else {
			dev_warn(&client->adapter->dev,
				 "%s: No Compass Present\n", MPU_NAME);
		}

		if (pdata->pressure.get_slave_descr) {
			mldl_cfg->pressure = pdata->pressure.get_slave_descr();
			dev_info(&client->adapter->dev,
				 "%s: +%s\n", MPU_NAME,
				 mldl_cfg->pressure->name);
			pressure_adapter =
			    i2c_get_adapter(pdata->pressure.adapt_num);

			if (pdata->pressure.irq > 0) {
				dev_info(&client->adapter->dev,
					 "Installing Pressure irq using %d\n",
					 pdata->pressure.irq);
				res = slaveirq_init(pressure_adapter,
						    &pdata->pressure,
						    "pressureirq");
				if (res)
					goto out_pressureirq_failed;
			} else {
				dev_warn(&client->adapter->dev,
					 "Pressure irq not assigned\n");
			}
		} else {
			dev_warn(&client->adapter->dev,
				 "%s: No Pressure Present\n", MPU_NAME);
		}
	}

	mldl_cfg->addr = client->addr;
	res = inv_mpu_open(&mpu->mldl_cfg, client->adapter,
			   accel_adapter, compass_adapter, pressure_adapter);

	if (res) {
		dev_err(&client->adapter->dev,
			"Unable to open %s %d\n", MPU_NAME, res);
		res = -ENODEV;
		goto out_whoami_failed;
	}

	mpu->dev.minor = MISC_DYNAMIC_MINOR;
	mpu->dev.name = "mpu";		/* Same for both 3050 and 6000 */
	mpu->dev.fops = &mpu_fops;
	res = misc_register(&mpu->dev);
	if (res < 0) {
		dev_err(&client->adapter->dev,
			"ERROR: misc_register returned %d\n", res);
		goto out_misc_register_failed;
	}

	if (client->irq > 0) {
		dev_info(&client->adapter->dev,
			 "Installing irq using %d\n", client->irq);
		res = mpuirq_init(client, mldl_cfg);
		if (res)
			goto out_mpuirq_failed;
	} else {
		dev_warn(&client->adapter->dev,
			 "Missing %s IRQ\n", MPU_NAME);
	}

	brvsens_register(SENSOR_HANDLE_ACCELEROMETER,   /* sensor UID */
			MPU_NAME,                      /* human readable name */
			(void*)client,                 /* context: passed back in activate/read callbacks */
			(PFNACTIVATE)NULL,
			(PFNREAD)mpu6050_read_accel);  /* read callback */
	/* Register with BRVSENS driver. */
	brvsens_register(SENSOR_HANDLE_GYROSCOPE,       /* sensor UID */
			MPU_NAME,                      /* human readable name */
			(void*)client,                 /* context: passed back in activate/read callbacks */
			(PFNACTIVATE)mpu6050_set_power_mode,
			(PFNREAD)mpu6050_read_gyro);   /* read callback */

#ifndef CONFIG_MPU_SENSORS_COMPASS_NONE
	brvsens_register(SENSOR_HANDLE_COMPASS,
			MPU_NAME,
			(void*)client,
			(PFNACTIVATE)mpu6050_set_compass_power_mode,
			(PFNREAD)mpu6050_read_compass);
#endif
   res  = mpu6050_init(client);
	return res;

 out_mpuirq_failed:
	misc_deregister(&mpu->dev);
 out_misc_register_failed:
	inv_mpu_close(&mpu->mldl_cfg, client->adapter,
		      accel_adapter, compass_adapter, pressure_adapter);
 out_whoami_failed:
	if (pdata && pdata->pressure.get_slave_descr && pdata->pressure.irq)
		slaveirq_exit(&pdata->pressure);
 out_pressureirq_failed:
	if (pdata && pdata->compass.get_slave_descr && pdata->compass.irq)
		slaveirq_exit(&pdata->compass);
 out_compassirq_failed:
	if (pdata && pdata->accel.get_slave_descr && pdata->accel.irq)
		slaveirq_exit(&pdata->accel);
 out_accelirq_failed:
	kfree(mpu);
 out_alloc_data_failed:
 out_check_functionality_failed:
	dev_err(&client->adapter->dev, "%s failed %d\n", __func__, res);
	return res;

}

static int mpu_remove(struct i2c_client *client)
{
	struct mpu_private_data *mpu = i2c_get_clientdata(client);
	struct i2c_adapter *accel_adapter;
	struct i2c_adapter *compass_adapter;
	struct i2c_adapter *pressure_adapter;
	struct mldl_cfg *mldl_cfg = &mpu->mldl_cfg;
	struct mpu_platform_data *pdata = mldl_cfg->pdata;

	accel_adapter = i2c_get_adapter(mldl_cfg->pdata->accel.adapt_num);
	compass_adapter = i2c_get_adapter(mldl_cfg->pdata->compass.adapt_num);
	pressure_adapter = i2c_get_adapter(mldl_cfg->pdata->pressure.adapt_num);

	dev_dbg(&client->adapter->dev, "%s\n", __func__);

	inv_mpu_close(mldl_cfg, client->adapter,
		      accel_adapter, compass_adapter, pressure_adapter);

	if (client->irq)
		mpuirq_exit();

	if (pdata && pdata->pressure.get_slave_descr && pdata->pressure.irq)
		slaveirq_exit(&pdata->pressure);

	if (pdata && pdata->compass.get_slave_descr && pdata->compass.irq)
		slaveirq_exit(&pdata->compass);

	if (pdata && pdata->accel.get_slave_descr && pdata->accel.irq)
		slaveirq_exit(&pdata->accel);

	misc_deregister(&mpu->dev);

   brvsens_deregister(SENSOR_HANDLE_COMPASS);
   brvsens_deregister(SENSOR_HANDLE_ACCELEROMETER);
   brvsens_deregister(SENSOR_HANDLE_GYROSCOPE);

	kfree(mpu);

	return 0;
}

static const struct i2c_device_id mpu_id[] = {
	{MPU_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, mpu_id);

static struct i2c_driver mpu_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = mpu_probe,
	.remove = mpu_remove,
	.id_table = mpu_id,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = MPU_NAME,
		   },
#ifdef I2C_CLIENT_INSMOD
	.address_data = &addr_data,
#else
	.address_list = normal_i2c,
#endif

	.shutdown = mpu_shutdown,	/* optional */
	.suspend = mpu_dev_suspend,	/* optional */
	.resume = mpu_dev_resume,	/* optional */

};

static int __init mpu_init(void)
{
	int res = i2c_add_driver(&mpu_driver);
	pr_info("%s: Probe name %s\n", __func__, MPU_NAME);
	if (res)
		pr_err("%s failed\n", __func__);
	return res;
}

static void __exit mpu_exit(void)
{
	pr_info("%s\n", __func__);
	i2c_del_driver(&mpu_driver);
}

module_init(mpu_init);
module_exit(mpu_exit);

MODULE_AUTHOR("Invensense Corporation");
MODULE_DESCRIPTION("User space character device interface for MPU");
MODULE_LICENSE("GPL");
MODULE_ALIAS(MPU_NAME);
