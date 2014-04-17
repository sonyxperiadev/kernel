/* drivers/misc/hscdtd00x/hscd_kxtf9.c
 *
 * Accelerometer device driver for I2C
 *
 * Copyright (C) 2011-2012 ALPS ELECTRIC CO., LTD. All Rights Reserved.
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

#ifdef ALPS_ACC_DEBUG
#define DEBUG 1
#endif

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define I2C_RETRIES		5

#define ACCSNS_DRIVER_NAME	"kxtf9"
#define ACCSNS_LOG_TAG		"[KXTF9], "

/* Register Name for accsns */
#define ACCSNS_XOUT		0x06
#define ACCSNS_YOUT		0x08
#define ACCSNS_ZOUT		0x0A
#define ACCSNS_XOUT_L		0x06
#define ACCSNS_XOUT_H		0x07
#define ACCSNS_YOUT_L		0x08
#define ACCSNS_YOUT_H		0x09
#define ACCSNS_ZOUT_L		0x0A
#define ACCSNS_ZOUT_H		0x0B
#define ACCSNS_CTRL_REG1	0x1B
#define ACCSNS_CTRL_REG2	0x1C
#define ACCSNS_CTRL_REG3	0x1D
#define ACCSNS_INT_CTRL_REG1	0x1E
#define ACCSNS_DATA_CTRL_REG	0x21

#define ACCSNS_DATA_ACCESS_NUM	6
#define ACCSNS_3AXIS_NUM	3

#define ACCSNS_DELAY(us)	msleep(us)

static struct i2c_client *client_accsns;
static int acc_ori;
static atomic_t flgena;
static atomic_t flgsuspend;

/*--------------------------------------------------------------------------
 * i2c read/write function
 *--------------------------------------------------------------------------*/
static int accsns_i2c_read(u8 *rxdata, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_accsns->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxdata,
		 },
		{
		 .addr = client_accsns->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxdata,
		 },
	};

	do {
		err = i2c_transfer(client_accsns->adapter,
				   msgs, ARRAY_SIZE(msgs));
	} while ((err != ARRAY_SIZE(msgs)) && (++tries < I2C_RETRIES));

	if (err != ARRAY_SIZE(msgs)) {
		dev_err(&client_accsns->adapter->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int accsns_i2c_write(u8 *txdata, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_accsns->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txdata,
		 },
	};

	do {
		err = i2c_transfer(client_accsns->adapter,
				   msgs, ARRAY_SIZE(msgs));
	} while ((err != ARRAY_SIZE(msgs)) && (++tries < I2C_RETRIES));

	if (err != ARRAY_SIZE(msgs)) {
		dev_err(&client_accsns->adapter->dev, "write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

/** exchange sensor axis for mount direction.
 *
 * @mnt        : exchnge value (0-7)
 * @px, py, pz : pointer of sensor value
 */
static void acc_exchange_mount(int mnt, int *px, int *py, int *pz)
{
	int x, y, z;
	switch (mnt) {
	case 0:
		x = *px;
		y = *py;
		z = *pz;
		break;
	case 1:
		x = *py;
		y = -*px;
		z = *pz;
		break;
	case 2:
		x = -*px;
		y = -*py;
		z = *pz;
		break;
	case 3:
		x = -*py;
		y = *px;
		z = *pz;
		break;
	case 4:
		x = -*px;
		y = *py;
		z = -*pz;
		break;
	case 5:
		x = -*py;
		y = -*px;
		z = -*pz;
		break;
	case 6:
		x = *px;
		y = -*py;
		z = -*pz;
		break;
	case 7:
		x = *py;
		y = *px;
		z = -*pz;
		break;
	default:
		x = *px;
		y = *py;
		z = *pz;
		break;
	}
	*px = x;
	*py = y;
	*pz = z;
}

/*--------------------------------------------------------------------------
 * accsns function
 *--------------------------------------------------------------------------*/
int accsns_get_acceleration_data(int *xyz)
{
	int err = -1;
	int i;
	u8 sx[ACCSNS_DATA_ACCESS_NUM];
	short s[3];

	if (atomic_read(&flgsuspend) == 1)
		return err;

	sx[0] = ACCSNS_XOUT;
	err = accsns_i2c_read(sx, ACCSNS_DATA_ACCESS_NUM);
	if (err < 0)
		return err;
	memcpy(s, sx, sizeof(s));
	for (i = 0; i < ACCSNS_3AXIS_NUM; i++)
		xyz[i] = s[i] / 16;
	acc_exchange_mount(acc_ori, &xyz[0], &xyz[1], &xyz[2]);
	dev_dbg(&client_accsns->adapter->dev,
		ACCSNS_LOG_TAG "x:%d,y:%d,z:%d\n", xyz[0], xyz[1], xyz[2]);

	return err;
}
EXPORT_SYMBOL(accsns_get_acceleration_data);

void accsns_activate(int flgatm, int flg, int dtime)
{
	u8 buf[2];

	if (flg != 0)
		flg = 1;

	buf[0] = ACCSNS_DATA_CTRL_REG;
	buf[1] = 0x04;
	accsns_i2c_write(buf, 2);

	buf[0] = ACCSNS_INT_CTRL_REG1;
	buf[1] = 0x30;
	accsns_i2c_write(buf, 2);

	buf[0] = ACCSNS_CTRL_REG1;
	buf[1] = 0x60;
	accsns_i2c_write(buf, 2);

	if (flg) {
		buf[0] = ACCSNS_CTRL_REG1;
		buf[1] = 0xE0;
		accsns_i2c_write(buf, 2);
		ACCSNS_DELAY(10000);
	}
	if (flgatm)
		atomic_set(&flgena, flg);
}
EXPORT_SYMBOL(accsns_activate);

static int accsns_register_init(void)
{
	int d[ACCSNS_3AXIS_NUM], ret = 0;
	u8 buf[2];

	dev_dbg(&client_accsns->adapter->dev, ACCSNS_LOG_TAG "%s\n", __func__);

	buf[0] = ACCSNS_CTRL_REG3;
	buf[1] = 0x80;
	ret = accsns_i2c_write(buf, 2);
	if (ret < 0)
		return ret;
	ACCSNS_DELAY(20000);

	accsns_activate(0, 1, 0);
	ret = accsns_get_acceleration_data(d);
	accsns_activate(0, 0, 0);
	dev_info(&client_accsns->adapter->dev,
		 ACCSNS_LOG_TAG "x:%d y:%d z:%d\n", d[0], d[1], d[2]);
	return ret;
}

/*--------------------------------------------------------------------------
 * suspend/resume function
 *--------------------------------------------------------------------------*/
static int accsns_suspend(struct i2c_client *client, pm_message_t mesg)
{
	dev_dbg(&client->adapter->dev, ACCSNS_LOG_TAG "%s\n", __func__);
	atomic_set(&flgsuspend, 1);
	accsns_activate(0, 0, 0);
	return 0;
}

static int accsns_resume(struct i2c_client *client)
{
	dev_dbg(&client->adapter->dev, ACCSNS_LOG_TAG "%s\n", __func__);
	atomic_set(&flgsuspend, 0);
	accsns_activate(0, atomic_read(&flgena), 0);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void accsns_early_suspend(struct early_suspend *handler)
{
	accsns_suspend(client_accsns, PMSG_SUSPEND);
}

static void accsns_early_resume(struct early_suspend *handler)
{
	accsns_resume(client_accsns);
}

static struct early_suspend accsns_early_suspend_handler = {
	.suspend = accsns_early_suspend,
	.resume = accsns_early_resume,
};
#endif

/*--------------------------------------------------------------------------
 * i2c device
 *--------------------------------------------------------------------------*/
static int accsns_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device_node *np;
	u32 val;
	int ret;
	ret = 0;
	val = 0;
	dev_dbg(&client->adapter->dev, ACCSNS_LOG_TAG "%s\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->adapter->dev, "client not i2c capable\n");
		return -ENODEV;
	}

	client_accsns = client;

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&accsns_early_suspend_handler);
#endif

	atomic_set(&flgena, 0);
	atomic_set(&flgsuspend, 0);
	if (client->dev.of_node) {
		np = client->dev.of_node;
		ret = of_property_read_u32(np, "orientation", &val);
		if (!ret)
			acc_ori = val;
		else
			acc_ori = 0;
	} else
		acc_ori = 0;
	if (accsns_register_init()) {
		dev_err(&client->adapter->dev, "failed to initialize sensor\n");
		return -EIO;
	}

	dev_info(&client->adapter->dev,
		 "detected " ACCSNS_DRIVER_NAME "accelerometer\n");

	return 0;
}

static int accsns_remove(struct i2c_client *client)
{
	dev_dbg(&client->adapter->dev, ACCSNS_LOG_TAG "%s\n", __func__);
	accsns_activate(0, 0, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&accsns_early_suspend_handler);
#endif
	client_accsns = NULL;
	return 0;
}

static const struct i2c_device_id accsns_id[] = {
	{ACCSNS_DRIVER_NAME, 0},
	{}
};

static const struct of_device_id hscd_kxtf9_of_match[] = {
	{.compatible = "bcm,hscd_kxtf9",},
	{},
};

MODULE_DEVICE_TABLE(of, hscd_kxtf9_of_match);

static struct i2c_driver accsns_driver = {
	.probe = accsns_probe,
	.remove = accsns_remove,
	.id_table = accsns_id,
	.driver = {
		   .name = ACCSNS_DRIVER_NAME,
		   .of_match_table = hscd_kxtf9_of_match,
		   },
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = accsns_suspend,
	.resume = accsns_resume,
#endif
};

static int __init accsns_init(void)
{
	pr_debug(ACCSNS_LOG_TAG "%s\n", __func__);
	return i2c_add_driver(&accsns_driver);
}

static void __exit accsns_exit(void)
{
	pr_debug(ACCSNS_LOG_TAG "%s\n", __func__);
	i2c_del_driver(&accsns_driver);
}

module_init(accsns_init);
module_exit(accsns_exit);

MODULE_DESCRIPTION("Alps Accelerometer Device");
MODULE_AUTHOR("ALPS ELECTRIC CO., LTD.");
MODULE_LICENSE("GPL v2");
