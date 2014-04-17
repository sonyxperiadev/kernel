/* ami306_km.c - AMI-Sensor driver
 *
 * Copyright (C) 2011 AICHI STEEL CORPORATION
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
/**
 * @file	ami306_km.c
 * @brief	Linux Kernel Module Interface
 */
/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Includes depend on platform
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "linux/ami306_def.h"
#include "linux/ami306_hw.h"
#include "linux/ami306_cmd.h"
#include "linux/ami_sensor.h"
#include "linux/ami_sensor_pif.h"

/*-===========================================================================
 *	Device Driver Platform Interface
 *-=========================================================================-*/
/**
 * suspend execution for microsecond intervals
 *
 * @param usec	microsecond
 */
void AMI_udelay(u32 usec)
{
	udelay(usec);
}

/**
 * suspend execution for millisecond intervals
 *
 * @param usec	millisecond
 */
void AMI_mdelay(u32 msec)
{
	msleep(msec);
}

/**
 * get DRDY value
 *
 * @return DRDY value
 */
#ifdef USE_DRDY_PIN
int AMI_DRDY_Value(void)
{
	return gpio_get_value(AMI_GPIO_DRDY);
}
#endif

/**
 * i2c write
 *
 * @param i2c	i2c handle
 * @param adr	register address
 * @param len	data length
 * @param buf	data buffer
 * @return		result
 */
int AMI_i2c_send(void *i2c, u8 adr, u8 len, u8 *buf)
{
	int res = 0;
	res = i2c_smbus_write_i2c_block_data(i2c, adr, len, buf);
	return (res < 0 ? AMI_COMM_ERR : 0);
}

/**
 * i2c read
 *
 * @param i2c	i2c handle
 * @param adr	register address
 * @param len	data length
 * @param buf	data buffer
 * @return		result
 */
int AMI_i2c_recv(void *i2c, u8 adr, u8 len, u8 *buf)
{
	int res = 0;
	res = i2c_smbus_read_i2c_block_data(i2c, adr, len, buf);
	return (res < 0 ? AMI_COMM_ERR : 0);
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Trace
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/**
 * Trace
 *
 * @param fmt	trace format
 * @param ...	value
 */
void AMI_LOG(const char *fmt, ...)
{
	va_list argp;
	char buf[256];
	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	printk(buf);
	printk("\n");
}

/**
 * Trace for debug
 *
 * @param fmt	trace format
 * @param ...	value
 */
void AMI_DLOG(const char *fmt, ...)
{
#ifdef DEBUG_LOG
	va_list argp;
	char buf[256];
	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	printk(buf);
	printk("\n");
#endif
}

/*-===========================================================================
 *	Linux Device Driver API
 *-=========================================================================-*/
static int ami_open(struct inode *inode, struct file *file)
{
	AMI_DLOG("AMI : Open device ");
	return 0;
}

/*---------------------------------------------------------------------------*/
static int ami_release(struct inode *inode, struct file *file)
{
	AMI_DLOG("AMI : Release device");
	return 0;
}

/*---------------------------------------------------------------------------*/
static struct ami_sensor_parameter k_param;
static struct ami_sensor_value k_val;
static u8 k_offset[3];
static s16 k_si[9];
static s16 k_dir[2];
static struct ami_driverinfo k_drv;
static struct ami_register k_reg;
static char *k_mem = NULL;

struct i2c_client *g_client;
void *g_handle = NULL;
static struct semaphore g_mutex;

static int ami_cmd(unsigned int cmd, unsigned long arg)
{
	int res = 0;
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case AMI_IOCTL_START_SENSOR:
		res = AMI_StartSensor(g_handle);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_STOP_SENSOR:
		res = AMI_StopSensor(g_handle);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_VALUE:
		res = AMI_GetValue(g_handle, &k_val);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_val, sizeof k_val))
			return -EFAULT;
		break;
	case AMI_IOCTL_SEARCH_OFFSET:
		res = AMI_SearchOffset(g_handle);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_WRITE_OFFSET:
		if (copy_from_user(k_offset, argp, sizeof(k_offset)))
			return -EFAULT;
		res = AMI_WriteOffset(g_handle, k_offset);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_READ_OFFSET:
		res = AMI_ReadOffset(g_handle, k_offset);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_offset, sizeof k_offset))
			return -EFAULT;
		break;
	case AMI_IOCTL_SET_SOFTIRON:
		if (copy_from_user(k_si, argp, sizeof(k_si)))
			return -EFAULT;
		res = AMI_SetSoftIron(g_handle, k_si);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_SOFTIRON:
		res = AMI_GetSoftIron(g_handle, k_si);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_si, sizeof k_si))
			return -EFAULT;
		break;
	case AMI_IOCTL_SET_DIR:
		AMI_DLOG("Set Direction");
		if (copy_from_user(k_dir, argp, sizeof(k_dir)))
			return -EFAULT;
		res = AMI_SetDirection(g_handle, k_dir[0], k_dir[1]);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_DIR:
		AMI_DLOG("Get Direction");
		res = AMI_GetDirection(g_handle, &k_dir[0], &k_dir[1]);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_dir, sizeof k_dir))
			return -EFAULT;
		break;
	case AMI_IOCTL_READ_PARAMS:
		res = AMI_ReadParameter(g_handle, &k_param);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_param, sizeof k_param))
			return -EFAULT;
		break;
	case AMI_IOCTL_DRIVERINFO:
		res = AMI_DriverInformation(g_handle, &k_drv);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_drv, sizeof k_drv))
			return -EFAULT;
		break;

	/* Self Test */
	case AMI_IOCTL_SELF_TEST:
		AMI_DLOG("Self Test");
		res = AMI_SelfTest(g_client);
		if (copy_to_user(argp, &res, sizeof(res)))
			return -EFAULT;
		break;
	/* i2c debug  */
	case AMI_IOCTL_DBG_READ:
	case AMI_IOCTL_DBG_READ_W:
		if (copy_from_user(&k_reg, argp, sizeof(k_reg)))
			return -EFAULT;
		if (cmd == AMI_IOCTL_DBG_READ)
			AMI_i2c_recv_b(g_client, k_reg.adr, &k_reg.dat.byte);
		if (cmd == AMI_IOCTL_DBG_READ_W)
			AMI_i2c_recv_w(g_client, k_reg.adr, &k_reg.dat.word);
		if (copy_to_user(argp, &k_reg, sizeof k_reg))
			return -EFAULT;
		break;
	case AMI_IOCTL_DBG_WRITE:
	case AMI_IOCTL_DBG_WRITE_W:
		if (copy_from_user(&k_reg, argp, sizeof(k_reg)))
			return -EFAULT;
		if (cmd == AMI_IOCTL_DBG_WRITE)
			AMI_i2c_send_b(g_client, k_reg.adr, k_reg.dat.byte);
		if (cmd == AMI_IOCTL_DBG_WRITE_W)
			AMI_i2c_send_w(g_client, k_reg.adr, k_reg.dat.word);
		break;
	case AMI_IOCTL_GET_RAW:
		res = AMI_GetRawValue(g_handle, &k_val);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_val, sizeof k_val))
			return -EFAULT;
		break;
	default:
		AMI_LOG("%s not supported = 0x%08x", __FUNCTION__, cmd);
		return -ENOIOCTLCMD;
	}
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long ami_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ami_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
#endif
{
	int ret = 0;
	if(g_handle == NULL) {
		AMI_LOG("not initialized");
		return -EFAULT;
	}

	if (down_interruptible(&g_mutex)) {
		AMI_LOG("down_interruptible");
		return -ERESTARTSYS;
	}
	ret = ami_cmd(cmd, arg);
	up(&g_mutex);
	return ret;
}

/*-===========================================================================
 *	Linux Kernel Module
 *-=========================================================================-*/
static struct file_operations ami_fops = {
	.owner = THIS_MODULE,
	.open = ami_open,
	.release = ami_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ami_ioctl,
#else
	.ioctl = ami_ioctl,
#endif
};

/*---------------------------------------------------------------------------*/
static struct miscdevice ami_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AMI_DRV_NAME,
	.fops = &ami_fops,
};

/*---------------------------------------------------------------------------*/
static int __devinit ami_probe(struct i2c_client *client,
			       const struct i2c_device_id *devid)
{
	int size = 0;

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "start");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		AMI_LOG("AMI : adapter can NOT support I2C_FUNC_I2C.");
		return -ENODEV;
	}

#ifdef USER_MEMORY
	/* Initialize driver command */
	size = AMI_GetMemSize();
	k_mem = kmalloc(size, GFP_KERNEL);
	if (k_mem == NULL) {
		AMI_LOG("AMI : kmalloc error.");
		return -ENODEV;
	}
	g_handle = AMI_InitDriver(k_mem, client);
	if (g_handle == NULL) {
		AMI_LOG("AMI : AMI_InitDriver error.");
		return -ENODEV;
	}
#else
	g_handle = AMI_InitDriver(client);
	if (g_handle == NULL) {
		AMI_LOG("AMI : AMI_InitDriver error.");
		return -ENODEV;
	}
#endif

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "end");

	return 0;
}

/*---------------------------------------------------------------------------*/
static int __devexit ami_remove(struct i2c_client *client)
{
	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "start");
	if (k_mem != NULL)
		kfree(k_mem);
	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "end");
	return 0;
}

/*---------------------------------------------------------------------------*/
static const struct i2c_device_id ami_idtable[] = {
	{AMI_DRV_NAME, 0},
	{}
};

/*---------------------------------------------------------------------------*/
static struct i2c_driver ami_i2c_driver = {
	.driver = {
		   .name = AMI_DRV_NAME,
		   },
	.probe = ami_probe,
	.remove = __devexit_p(ami_remove),
	.id_table = ami_idtable,
};

/*---------------------------------------------------------------------------*/
static int __devinit ami_init(void)
{
	int res = 0;
	struct i2c_board_info info;
	struct i2c_adapter *adapter;

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "start");
	AMI_LOG("%s compiled %s %s", AMI_DRV_NAME, __DATE__, __TIME__);

	res = misc_register(&ami_device);
	if (0 > res) {
		AMI_LOG("AMI : register failed");
		return res;
	}
	res = i2c_add_driver(&ami_i2c_driver);
	if (0 > res) {
		AMI_LOG("AMI : i2c_add_driver failed");
		misc_deregister(&ami_device);
		return res;
	}
	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = AMI_I2C_ADDRESS;
	strlcpy(info.type, AMI_DRV_NAME, I2C_NAME_SIZE);
	adapter = i2c_get_adapter(AMI_I2C_BUS_NUM);
	g_client = i2c_new_device(adapter, &info);
	i2c_put_adapter(adapter);

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "end");

	/* Init gpio */
	gpio_request(AMI_GPIO_DRDY, "DRDY");
	gpio_direction_input(AMI_GPIO_DRDY);

	sema_init(&g_mutex, 1);

	AMI_DLOG("AMI_IOCTL_START_SENSOR : 0x%08x", AMI_IOCTL_START_SENSOR);
	AMI_DLOG("AMI_IOCTL_GET_VALUE : 0x%08x", AMI_IOCTL_GET_VALUE);
	AMI_DLOG("AMI_IOCTL_STOP_SENSOR : 0x%08x", AMI_IOCTL_STOP_SENSOR);

	return res;
}

module_init(ami_init);
/*---------------------------------------------------------------------------*/
static void __exit ami_exit(void)
{
	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "start");
	/* Unregist i2c slave address, or next i2c_new_device will fail. */
	i2c_unregister_device(g_client);
	i2c_del_driver(&ami_i2c_driver);	/* delete driver */
	misc_deregister(&ami_device);

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __FUNCTION__, "end");
}

module_exit(ami_exit);

MODULE_AUTHOR("AICHI STEEL");
MODULE_DESCRIPTION("AMI MI sensor");
MODULE_LICENSE("GPL");
