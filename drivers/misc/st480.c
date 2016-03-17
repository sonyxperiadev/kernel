/************************************************************************
* Copyright (C) 2012 Senodia.
*
* Author: Tori Xu <tori.xz.xu@gmail.com>
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
**************************************************************************/
#include <linux/module.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/sensors_core.h>

#include <linux/st480.h>

struct st480_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct delayed_work work;
	u8 package;
	u8 orientation;
};

static struct st480_data *st480;

static atomic_t m_flag;
static atomic_t mv_flag;

static atomic_t open_count;
static atomic_t open_flag;
static atomic_t reserve_open_flag;

static int st480_delay = ST480_DEFAULT_DELAY;

struct mag_3 {
	s16 mag_x, mag_y, mag_z;
};
static struct mag_3 mag;

/*
 * i2c transfer
 * read/write
 */
#if ST480_DATA_TRANSFER
static int st480_i2c_read_data(struct i2c_client *client, int len, char *buf,
			       int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = len,
			.buf = buf,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buf,
		},
	};

	if (i2c_transfer(client->adapter, msgs, 2) < 0) {
		pr_err("megnetic_i2c_read_data: transfer error\n");
		return -EIO;
	} else
		return 0;
}
#endif

#if ST480_SMBUS_READ_BYTE
static int st480_smbus_read_byte(struct i2c_client *client,
				 unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -EPERM;
	*data = dummy & 0x000000ff;

	return 0;
}
#endif

#if ST480_SMBUS_WRITE_BYTE
static int st480_smbus_write_byte(struct i2c_client *client,
				  unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -EPERM;
	return 0;
}
#endif

#if ST480_SMBUS_READ_BYTE_BLOCK
static int st480_smbus_read_byte_block(struct i2c_client *client,
				       unsigned char reg_addr,
				       unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -EPERM;
	return 0;
}
#endif

#if ST480_SMBUS_WRITE_BYTE_BLOCK
static int st480_smbus_write_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_write_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -EPERM;
	return 0;
}
#endif

static int st480_setup(struct i2c_client *client)
{
	int ret;
	int err;
	unsigned char buf[5];
	unsigned char data[1];

	memset(buf, 0, sizeof(buf));
	data[0] = 0;

	buf[0] = READ_REGISTER_CMD;
	buf[1] = 0x00;
	ret = 0;
	err = 0;

	do {
		err = st480_i2c_read_data(client, 2, buf, 3);
		if (!err)
			break;
		else {
			ret++;
			msleep(20);
		}
		if (ret > MAX_FAILURE_COUNT)
			return -EIO;
	} while (err);

	if (buf[2] != ST480_DEVICE_ID) {
		pr_err("st480: device id is wrong\n");
		return -ENODEV;
	}
	buf[0] = WRITE_REGISTER_CMD;
	buf[1] = ONE_INIT_DATA_HIGH;
	buf[2] = ONE_INIT_DATA_LOW;
	buf[3] = ONE_INIT_REG;
	ret = 0;
	do {
		err = st480_i2c_read_data(client, 4, buf, 1);
		if (!err)
			break;
		else {
			ret++;
			msleep(20);
		}
		if (ret > MAX_FAILURE_COUNT)
			return -EIO;
	} while (err);

	buf[0] = WRITE_REGISTER_CMD;
	buf[1] = TWO_INIT_DATA_HIGH;
	buf[2] = TWO_INIT_DATA_LOW;
	buf[3] = TWO_INIT_REG;
	ret = 0;
	do {
		err = st480_i2c_read_data(client, 4, buf, 1);
		if (!err)
			break;
		else {
			ret++;
			msleep(20);
		}
		if (ret > MAX_FAILURE_COUNT)
			return -EIO;
	} while (err);

	/*set calibration register*/
	buf[0] = WRITE_REGISTER_CMD;
	buf[1] = CALIBRATION_DATA_HIGH;
	buf[2] = CALIBRATION_DATA_LOW;
	buf[3] = CALIBRATION_REG;
	ret = 0;
	do {
		err = st480_i2c_read_data(client, 4, buf, 1);
		if (!err)
			break;
		else {
			ret++;
			msleep(20);
		}
		if (ret > MAX_FAILURE_COUNT)
			return -EIO;
	} while (err);

	/*set mode config*/
	ret = 0;
	do  {
		err = st480_smbus_read_byte(client,
			SINGLE_MEASUREMENT_MODE_CMD, data);
		if (!err)
			break;
		else
			ret++;
		if (ret > MAX_FAILURE_COUNT)
			return -EIO;
	} while (err);

	return 0;
}

static void st480_work_func(void)
{
	char buffer[9];
	int ret;
	char data[1];
	int err;

	memset(data, 0, sizeof(data));
	memset(buffer, 0, sizeof(buffer));

	ret = 0;
	err = 0;
	do  {
		err = st480_smbus_read_byte_block(st480->client,
				READ_MEASUREMENT_CMD, buffer, 9);
		if (!err)
			break;
		else
			ret++;

		if (ret > MAX_FAILURE_COUNT)
			break;
	} while (err);

	if (!((buffer[0] >> 4) & 0X01)) {
		if (st480->package == ST480_SIZE_3X3_QFN) {
			switch (st480->orientation) {
			case ST480_BOARD_LOCATION_FRONT_DEGREE_0:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_90:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_180:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_270:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;

			case ST480_BOARD_LOCATION_BACK_DEGREE_0:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_90:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_180:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_270:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			default:
				break;
			}
		} else if (st480->package == ST480_SIZE_2X2_BGA) {
			switch (st480->orientation) {
			case ST480_BOARD_LOCATION_FRONT_DEGREE_0:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_90:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_180:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_270:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_0:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_90:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_180:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_270:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			default:
				break;
			}
		} else if (st480->package == ST480_SIZE_1_6X1_6_LGA) {
			switch (st480->orientation) {
			case ST480_BOARD_LOCATION_FRONT_DEGREE_0:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_90:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_180:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_270:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_0:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_90:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_180:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_270:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			default:
				break;
			}
		} else if (st480->package == ST480_SIZE_1_6X1_6_BGA) {
			switch (st480->orientation) {
			case ST480_BOARD_LOCATION_FRONT_DEGREE_0:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_90:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_180:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_FRONT_DEGREE_270:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (buffer[7] << 8)
						| buffer[8];
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_0:
				mag.mag_x = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_y = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_90:
				mag.mag_x = (buffer[3] << 8)
						| buffer[4];
				mag.mag_y = (-1) * ((buffer[5] << 8)
						| buffer[6]);
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_180:
				mag.mag_x = (buffer[5] << 8)
						| buffer[6];
				mag.mag_y = (buffer[3] << 8)
						| buffer[4];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			case ST480_BOARD_LOCATION_BACK_DEGREE_270:
				mag.mag_x = (-1) * ((buffer[3] << 8)
						| buffer[4]);
				mag.mag_y = (buffer[5] << 8)
						| buffer[6];
				mag.mag_z = (-1) * ((buffer[7] << 8)
						| buffer[8]);
				break;
			default:
				break;
			}
		}
		if (((buffer[1] << 8) | (buffer[2])) > 46244) {
			mag.mag_x =
			    mag.mag_x * (1 +
					 (70 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
			mag.mag_y =
			    mag.mag_y * (1 +
					 (70 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
			mag.mag_z =
			    mag.mag_z * (1 +
					 (70 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
		} else if (((buffer[1] << 8) | (buffer[2])) < 46244) {
			mag.mag_x =
			    mag.mag_x * (1 +
					 (60 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
			mag.mag_y =
			    mag.mag_y * (1 +
					 (60 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
			mag.mag_z =
			    mag.mag_z * (1 +
					 (60 / 128 / 4096) *
					 (((buffer[1] << 8) | (buffer[2])) -
					  46244));
		}

	}

	ret = 0;
	do {
		err = st480_smbus_read_byte(st480->client,
			SINGLE_MEASUREMENT_MODE_CMD, data);
		if (!err)
			break;
		else
			ret++;
		if (ret > MAX_FAILURE_COUNT)
			break;
	} while (err);
}

static void st480_input_func(struct work_struct *work)
{
	SENODIAFUNC("st480_input_func");
	st480_work_func();

	if (atomic_read(&m_flag) || atomic_read(&mv_flag)) {
		input_report_abs(st480->input_dev, ABS_RX, mag.mag_x);
		input_report_abs(st480->input_dev, ABS_RY, mag.mag_y);
		input_report_abs(st480->input_dev, ABS_RZ, mag.mag_z);
	}

	input_sync(st480->input_dev);

	schedule_delayed_work(&st480->work, msecs_to_jiffies(st480_delay));
}

static void ecs_closedone(void)
{
	SENODIAFUNC("ecs_closedone");
	atomic_set(&m_flag, 0);
	atomic_set(&mv_flag, 0);
}

/***** st480 functions ***************************************/
static int st480_open(struct inode *inode, struct file *file)
{
	int ret = -1;

	SENODIAFUNC("st480_open");

	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			ret = 0;
		}
	}

	if (atomic_read(&reserve_open_flag))
		schedule_delayed_work(&st480->work,
				      msecs_to_jiffies(st480_delay));
	return ret;
}

static int st480_release(struct inode *inode, struct file *file)
{
	SENODIAFUNC("st480_release");

	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);

	ecs_closedone();

	cancel_delayed_work(&st480->work);
	return 0;
}

static long st480_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;

	SENODIADBG("enter %s\n", __func__);

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
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

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
		atomic_set(&m_flag, flag);
		SENODIADBG("MFLAG is set to %d", flag);
		break;
	case ECS_IOCTL_APP_GET_MFLAG:
		flag = atomic_read(&m_flag);
		SENODIADBG("Mflag = %d\n", flag);
		break;
	case ECS_IOCTL_APP_SET_MVFLAG:
		atomic_set(&mv_flag, flag);
		SENODIADBG("MVFLAG is set to %d", flag);
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = atomic_read(&mv_flag);
		SENODIADBG("MVflag = %d\n", flag);
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		st480_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = st480_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MFLAG:
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static const struct file_operations st480_fops = {
	.owner = THIS_MODULE,
	.open = st480_open,
	.release = st480_release,
	.unlocked_ioctl = st480_ioctl,
};

static struct miscdevice st480_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "st480",
	.fops = &st480_fops,
};

/*********************************************/
#if ST480_AUTO_TEST
static int sensor_test_read(void)
{
	st480_work_func();
	return 0;
}

static int auto_test_read(void *unused)
{
	while (1) {
		sensor_test_read();
		msleep(200);
	}
	return 0;
}
#endif

int st480_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np;
	u32 val;
	int err = 0;

#if ST480_AUTO_TEST
	struct task_struct *thread;
#endif
	pr_info("st480_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("SENODIA st480_probe: check_functionality failed.\n");
		err = -ENODEV;
		goto exit0;
	}

	/* Allocate memory for driver data */
	st480 = kzalloc(sizeof(struct st480_data), GFP_KERNEL);
	if (!st480) {
		pr_err("st480_probe: memory allocation failed.\n");
		err = -ENOMEM;
		goto exit1;
	}

	st480->client = client;

	i2c_set_clientdata(client, st480);
	if (!client->dev.platform_data) {
		if (client->dev.of_node) {
			np = client->dev.of_node;
			err = of_property_read_u32(np, "package", &val);
			if (err)
				st480->package = 2;
			else
				st480->package = val;
			err = of_property_read_u32(np, "orientation", &val);
			if (err)
				st480->orientation = 0;
			else
				st480->orientation = val;
		} else {
			pr_err("st480: fail to get its setting\n");
			goto exit1;
		}
	} else {
		st480->package = 2;
		st480->orientation = 0;
	}
	INIT_DELAYED_WORK(&st480->work, st480_input_func);

	err = st480_setup(st480->client);
	if (err != 0) {
		pr_err("st480 setup error!\n");
		goto exit2;
	}

	/* Declare input device */
	st480->input_dev = input_allocate_device();
	if (!st480->input_dev) {
		err = -ENOMEM;
		pr_err("st480_probe: Failed to allocate input device\n");
		goto exit3;
	}

	/* Setup input device */
	__set_bit(EV_ABS, st480->input_dev->evbit);

	/* x-axis of raw magnetic vector (-32768, 32767) */
	input_set_abs_params(st480->input_dev, ABS_RX, ABSMIN_MAG,
			     ABSMAX_MAG, 0, 0);
	/* y-axis of raw magnetic vector (-32768, 32767) */
	input_set_abs_params(st480->input_dev, ABS_RY, ABSMIN_MAG,
			     ABSMAX_MAG, 0, 0);
	/* z-axis of raw magnetic vector (-32768, 32767) */
	input_set_abs_params(st480->input_dev, ABS_RZ, ABSMIN_MAG,
			     ABSMAX_MAG, 0, 0);

	/* Set name */
	st480->input_dev->name = "compass";

	/* Register */
	err = input_register_device(st480->input_dev);
	if (err) {
		pr_err("st480_probe: Unable to register input device\n");
		goto exit4;
	}

	err = misc_register(&st480_device);
	if (err) {
		pr_err("st480_probe: st480_device register failed\n");
		goto exit5;
	}

	/* As default, report all information */
	atomic_set(&m_flag, 1);
	atomic_set(&mv_flag, 1);

#if ST480_AUTO_TEST
	thread = kthread_run(auto_test_read, NULL, "st480_read_test");
#endif

	pr_info("st480 probe done\n");
	return 0;

exit5:
	misc_deregister(&st480_device);
	input_unregister_device(st480->input_dev);
exit4:
	input_free_device(st480->input_dev);
exit3:
exit2:
	kfree(st480);
exit1:
exit0:
	return err;

}

static int st480_remove(struct i2c_client *client)
{
	misc_deregister(&st480_device);
	input_unregister_device(st480->input_dev);
	input_free_device(st480->input_dev);
	cancel_delayed_work(&st480->work);
	kfree(st480);
	return 0;
}

static const struct of_device_id st480_of_match[] = {
	{.compatible = "bcm,st480",},
	{},
};

MODULE_DEVICE_TABLE(of, st480_of_match);

static const struct i2c_device_id st480_id[] = {
	{ST480_I2C_NAME, 0},
	{}
};

static struct i2c_driver st480_driver = {
	.probe = st480_probe,
	.remove = st480_remove,
	.id_table = st480_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = ST480_I2C_NAME,
		.of_match_table = st480_of_match,
	},
};

static int __init st480_init(void)
{
	return i2c_add_driver(&st480_driver);
}

static void __exit st480_exit(void)
{
	i2c_del_driver(&st480_driver);
}

module_init(st480_init);
module_exit(st480_exit);

MODULE_AUTHOR("Tori Xu <xuezhi_xu@senodia.com>");
MODULE_DESCRIPTION("senodia st480 linux driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("9.0");
