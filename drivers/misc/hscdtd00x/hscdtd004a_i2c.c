/* drivers/misc/hscdtd00x/hscdtd004a_i2c.c
 *
 * GeoMagneticField device driver for I2C (HSCDTD004/HSCDTD006)
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

#ifdef ALPS_MAG_DEBUG
#define DEBUG 1
#endif

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define I2C_RETRIES		5

#define HSCDTD_DRIVER_NAME	"hscdtd004a"
#define HSCDTD_LOG_TAG		"[HSCDTD], "

#define HSCDTD_CHIP_ID		0x5445

#define HSCDTD_STBA		0x0B
#define HSCDTD_STBB		0x0C
#define HSCDTD_INFO		0x0D
#define HSCDTD_XOUT		0x10
#define HSCDTD_YOUT		0x12
#define HSCDTD_ZOUT		0x14
#define HSCDTD_XOUT_H		0x11
#define HSCDTD_XOUT_L		0x10
#define HSCDTD_YOUT_H		0x13
#define HSCDTD_YOUT_L		0x12
#define HSCDTD_ZOUT_H		0x15
#define HSCDTD_ZOUT_L		0x14

#define HSCDTD_STATUS		0x18
#define HSCDTD_CTRL1		0x1B
#define HSCDTD_CTRL2		0x1C
#define HSCDTD_CTRL3		0x1D

#define HSCDTD_TCS_TIME		10000	/* Measure temp. of every 10 sec */
#define HSCDTD_DATA_ACCESS_NUM	6
#define HSCDTD_3AXIS_NUM	3
#define HSCDTD_INITIALL_DELAY	20

#define HSCDTD_DELAY(us)	msleep(us)

/* Self-test resiter value */
#define HSCDTD_ST_REG_DEF	0x55
#define HSCDTD_ST_REG_PASS	0xAA
#define HSCDTD_ST_REG_X		0x01
#define HSCDTD_ST_REG_Y		0x02
#define HSCDTD_ST_REG_Z		0x04
#define HSCDTD_ST_REG_XYZ	0x07

/* Self-test error number */
#define HSCDTD_ST_OK		0x00
#define HSCDTD_ST_ERR_I2C	0x01
#define HSCDTD_ST_ERR_INIT	0x02
#define HSCDTD_ST_ERR_1ST	0x03
#define HSCDTD_ST_ERR_2ND	0x04
#define HSCDTD_ST_ERR_VAL	0x10
#define HSCDTD_ST_ERR_VAL_X	(HSCDTD_ST_REG_X | HSCDTD_ST_ERR_VAL)
#define HSCDTD_ST_ERR_VAL_Y	(HSCDTD_ST_REG_Y | HSCDTD_ST_ERR_VAL)
#define HSCDTD_ST_ERR_VAL_Z	(HSCDTD_ST_REG_Z | HSCDTD_ST_ERR_VAL)

static struct i2c_client *client_hscdtd;

static atomic_t flgena;
static atomic_t delay;
static atomic_t flgsuspend;
static int tcs_thr;
static int tcs_cnt;

/*--------------------------------------------------------------------------
 * i2c read/write function
 *--------------------------------------------------------------------------*/
static int hscdtd_i2c_read(char *rxdata, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_hscdtd->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxdata,
		 },
		{
		 .addr = client_hscdtd->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxdata,
		 },
	};

	do {
		err = i2c_transfer(client_hscdtd->adapter,
				   msgs, ARRAY_SIZE(msgs));
	} while ((err != ARRAY_SIZE(msgs)) && (++tries < I2C_RETRIES));

	if (err != ARRAY_SIZE(msgs)) {
		dev_err(&client_hscdtd->adapter->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int hscdtd_i2c_write(char *txdata, int length)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
		 .addr = client_hscdtd->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txdata,
		 },
	};

	do {
		err = i2c_transfer(client_hscdtd->adapter,
				   msgs, ARRAY_SIZE(msgs));
	} while ((err != ARRAY_SIZE(msgs)) && (++tries < I2C_RETRIES));

	if (err != ARRAY_SIZE(msgs)) {
		dev_err(&client_hscdtd->adapter->dev, "write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

/*--------------------------------------------------------------------------
 * hscdtd function
 *--------------------------------------------------------------------------*/
static int hscdtd_soft_reset(void)
{
	int rc;
	u8 buf[2];

	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "Software Reset\n");

	buf[0] = HSCDTD_CTRL3;
	buf[1] = 0x80;
	rc = hscdtd_i2c_write(buf, 2);
	HSCDTD_DELAY(5000);
	return rc;
}

static int hscdtd_tcs_setup(void)
{
	int rc;
	u8 buf[2];

	buf[0] = HSCDTD_CTRL3;
	buf[1] = 0x02;
	rc = hscdtd_i2c_write(buf, 2);
	HSCDTD_DELAY(1700);
	tcs_thr = HSCDTD_TCS_TIME / atomic_read(&delay);
	tcs_cnt = 0;

	return rc;
}

static int hscdtd_force_setup(void)
{
	u8 buf[2];

	buf[0] = HSCDTD_CTRL3;
	buf[1] = 0x40;
	return hscdtd_i2c_write(buf, 2);
}

int hscdtd_get_magnetic_field_data(int *xyz)
{
	int err = -1;
	int i;
	u8 sx[HSCDTD_DATA_ACCESS_NUM];

	if (atomic_read(&flgsuspend) == 1)
		return err;

	sx[0] = HSCDTD_XOUT;
	err = hscdtd_i2c_read(sx, HSCDTD_DATA_ACCESS_NUM);
	if (err < 0)
		return err;
	for (i = 0; i < HSCDTD_3AXIS_NUM; i++)
		xyz[i] = (int)((short)((sx[2 * i + 1] << 8) | (sx[2 * i])));

	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "x:%d,y:%d,z:%d\n", xyz[0], xyz[1], xyz[2]);

	if (++tcs_cnt > tcs_thr)
		hscdtd_tcs_setup();

	hscdtd_force_setup();

	return err;
}
EXPORT_SYMBOL(hscdtd_get_magnetic_field_data);

void hscdtd_activate(int flgatm, int flg, int dtime)
{
	u8 buf[2];
	int ena = atomic_read(&flgena);

	if (flg != 0)
		flg = 1;

	if ((!flg) && (ena)) {
		hscdtd_soft_reset();
	} else if ((flg) && (!ena)) {
		tcs_cnt = tcs_cnt * atomic_read(&delay) / dtime;
		tcs_thr = HSCDTD_TCS_TIME / dtime;

		buf[0] = HSCDTD_CTRL1;
		buf[1] = 0xE2;
		hscdtd_i2c_write(buf, 2);
		HSCDTD_DELAY(100);

		hscdtd_tcs_setup();
		hscdtd_force_setup();
	}

	if (flgatm) {
		atomic_set(&flgena, flg);
		atomic_set(&delay, dtime);
	}
}
EXPORT_SYMBOL(hscdtd_activate);

int hscdtd_get_hardware_data(int *xyz)
{
	int ret = 0;

	if (atomic_read(&flgsuspend) == 1)
		return -1;
	hscdtd_activate(0, 1, 10);
	HSCDTD_DELAY(5000);
	ret = hscdtd_get_magnetic_field_data(xyz);
	hscdtd_activate(0, atomic_read(&flgena), atomic_read(&delay));
	return ret;
}
EXPORT_SYMBOL(hscdtd_get_hardware_data);

int hscdtd_self_test_a(void)
{
	int rc = HSCDTD_ST_OK;
	u8 sx[2], cr1[1];

	if (atomic_read(&flgsuspend) == 1)
		return -1;

	/* Control resister1 backup  */
	cr1[0] = HSCDTD_CTRL1;
	if (hscdtd_i2c_read(cr1, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "Control resister1 value, %02X\n", cr1[0]);

	/* Move active mode (force state)  */
	sx[0] = HSCDTD_CTRL1;
	sx[1] = 0xE2;
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(40);

	/* Get inital value of self-test-A register  */
	sx[0] = HSCDTD_STBA;
	hscdtd_i2c_read(sx, 1);
	sx[0] = HSCDTD_STBA;
	if (hscdtd_i2c_read(sx, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBA reg. initial value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_DEF) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"Err: Initial value of STBA reg. is %02X\n", sx[0]);
		rc = HSCDTD_ST_ERR_INIT;
		goto err_stba;
	}

	/* do self-test-A  */
	sx[0] = HSCDTD_CTRL3;
	sx[1] = 0x20;
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(3000);

	/* Get 1st value of self-test-A register  */
	sx[0] = HSCDTD_STBA;
	if (hscdtd_i2c_read(sx, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBA reg. 1st value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_PASS) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"Err: 1st value of STBA reg. is %02X\n", sx[0]);
		rc = HSCDTD_ST_ERR_1ST;
		goto err_stba;
	}
	HSCDTD_DELAY(3000);

	/* Get 2nd value of self-test-A register  */
	sx[0] = HSCDTD_STBA;
	if (hscdtd_i2c_read(sx, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBA reg. 2nd value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_DEF) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"Err: 2nd value of STBA reg. is %02X\n", sx[0]);
		rc = HSCDTD_ST_ERR_2ND;
	}

err_stba:
	/* Resume */
	sx[0] = HSCDTD_CTRL1;
	sx[1] = cr1[0];
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(40);

	return rc;
}
EXPORT_SYMBOL(hscdtd_self_test_a);

int hscdtd_self_test_b(void)
{
	int rc = HSCDTD_ST_OK;
	u8 sx[2], cr1[1];

	if (atomic_read(&flgsuspend) == 1)
		return -1;

	/* Control resister1 backup  */
	cr1[0] = HSCDTD_CTRL1;
	if (hscdtd_i2c_read(cr1, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "Control resister1 value, %02X\n", cr1[0]);

	/* Move active mode (force state)  */
	sx[0] = HSCDTD_CTRL1;
	sx[1] = 0xE2;
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(40);

	/* Get inital value of self-test-B register  */
	sx[0] = HSCDTD_STBB;
	hscdtd_i2c_read(cr1, 1);
	sx[0] = HSCDTD_STBB;
	hscdtd_i2c_read(cr1, 1);
	sx[0] = HSCDTD_STBB;
	if (hscdtd_i2c_read(cr1, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBA reg. initial value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_DEF) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"Err: Initial value of STBB reg. is %02X\n", sx[0]);
		rc = HSCDTD_ST_ERR_INIT;
		goto err_stbb;
	}

	/* do self-test-B  */
	sx[0] = HSCDTD_CTRL3;
	sx[1] = 0x10;
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(6000);

	/* Get 1st value of self-test-A register  */
	sx[0] = HSCDTD_STBB;
	if (hscdtd_i2c_read(sx, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBB reg. 1st value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_PASS) {
		if ((sx[0] < HSCDTD_ST_REG_X) || (sx[0] > HSCDTD_ST_REG_XYZ)) {
			dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
				"Err: 1st value of STBB reg. is %02X\n", sx[0]);
			rc = HSCDTD_ST_ERR_1ST;
			goto err_stbb;
		} else {
			dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
				"Err: 1st value of STBB reg. is %02X\n", sx[0]);
			rc = (int)(sx[0] | HSCDTD_ST_ERR_VAL);
			goto err_stbb;
		}
	}
	HSCDTD_DELAY(3000);

	/* Get 2nd value of self-test-B register  */
	sx[0] = HSCDTD_STBB;
	if (hscdtd_i2c_read(sx, 1))
		return HSCDTD_ST_ERR_I2C;
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "STBA reg. 2nd value, %02X\n", sx[0]);
	if (sx[0] != HSCDTD_ST_REG_DEF) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"Err: 2nd value of STBB reg. is %02X\n", sx[0]);
		rc = HSCDTD_ST_ERR_2ND;
	}
err_stbb:
	/* Resume */
	sx[0] = HSCDTD_CTRL1;
	sx[1] = cr1[0];
	if (hscdtd_i2c_write(sx, 2))
		return HSCDTD_ST_ERR_I2C;
	HSCDTD_DELAY(40);

	return rc;
}
EXPORT_SYMBOL(hscdtd_self_test_b);

static int hscdtd_register_init(void)
{
	int v[HSCDTD_3AXIS_NUM], ret = 0;
	u8 buf[2];
	u16 chip_info;

	dev_dbg(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG "%s\n", __func__);

	buf[0] = HSCDTD_INFO;
	ret = hscdtd_i2c_read(buf, 2);
	if (ret < 0)
		return ret;

	chip_info = (u16) ((buf[1] << 8) | buf[0]);
	dev_dbg(&client_hscdtd->adapter->dev,
		HSCDTD_LOG_TAG "chip_info, 0x%04X\n", chip_info);
	if (chip_info != HSCDTD_CHIP_ID) {
		dev_err(&client_hscdtd->adapter->dev, HSCDTD_LOG_TAG
			"chipID error(0x%04X).\n", chip_info);
		return -1;
	}

	hscdtd_soft_reset();

	hscdtd_activate(0, 1, atomic_read(&delay));
	HSCDTD_DELAY(5000);
	ret = hscdtd_get_magnetic_field_data(v);
	hscdtd_activate(0, 0, atomic_read(&delay));
	dev_info(&client_hscdtd->adapter->dev,
		 HSCDTD_LOG_TAG "x:%d y:%d z:%d\n", v[0], v[1], v[2]);

	return ret;
}

/*--------------------------------------------------------------------------
 * suspend/resume function
 *--------------------------------------------------------------------------*/
static int hscdtd_suspend(struct i2c_client *client, pm_message_t mesg)
{
	dev_dbg(&client->adapter->dev, HSCDTD_LOG_TAG "%s\n", __func__);
	atomic_set(&flgsuspend, 1);
	hscdtd_activate(0, 0, atomic_read(&delay));
	return 0;
}

static int hscdtd_resume(struct i2c_client *client)
{
	dev_dbg(&client->adapter->dev, HSCDTD_LOG_TAG "%s\n", __func__);
	atomic_set(&flgsuspend, 0);
	hscdtd_activate(0, atomic_read(&flgena), atomic_read(&delay));
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void hscdtd_early_suspend(struct early_suspend *handler)
{
	hscdtd_suspend(client_hscdtd, PMSG_SUSPEND);
}

static void hscdtd_early_resume(struct early_suspend *handler)
{
	hscdtd_resume(client_hscdtd);
}

static struct early_suspend hscdtd_early_suspend_handler = {
	.suspend = hscdtd_early_suspend,
	.resume = hscdtd_early_resume,
};
#endif

/*--------------------------------------------------------------------------
 * i2c device
 *--------------------------------------------------------------------------*/
static int hscdtd_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	dev_dbg(&client->adapter->dev, HSCDTD_LOG_TAG "%s\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->adapter->dev, "client not i2c capable\n");
		return -ENODEV;
	}

	client_hscdtd = client;

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&hscdtd_early_suspend_handler);
#endif

	atomic_set(&flgena, 0);
	atomic_set(&delay, HSCDTD_INITIALL_DELAY);
	atomic_set(&flgsuspend, 0);
	tcs_cnt = 0;
	tcs_thr = HSCDTD_TCS_TIME / atomic_read(&delay);

	if (hscdtd_register_init()) {
		dev_err(&client->adapter->dev, "failed to initialize sensor\n");
		return -EIO;
	}

	dev_info(&client->adapter->dev,
		 "detected HSCDTD004/006 geomagnetic field sensor\n");

	return 0;
}

static int hscdtd_remove(struct i2c_client *client)
{
	dev_dbg(&client->adapter->dev, HSCDTD_LOG_TAG "%s\n", __func__);
	hscdtd_activate(0, 0, atomic_read(&delay));
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&hscdtd_early_suspend_handler);
#endif
	client_hscdtd = NULL;
	return 0;
}

/*--------------------------------------------------------------------------
 * module
 *--------------------------------------------------------------------------*/
static const struct i2c_device_id alps_id[] = {
	{HSCDTD_DRIVER_NAME, 0},
	{}
};

static struct i2c_driver hscdtd_driver = {
	.probe = hscdtd_probe,
	.remove = hscdtd_remove,
	.id_table = alps_id,
	.driver = {
		   .name = HSCDTD_DRIVER_NAME,
		   },
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = hscdtd_suspend,
	.resume = hscdtd_resume,
#endif
};

static int __init hscdtd_init(void)
{
	pr_debug(HSCDTD_LOG_TAG "%s\n", __func__);
	return i2c_add_driver(&hscdtd_driver);
}

static void __exit hscdtd_exit(void)
{
	pr_debug(HSCDTD_LOG_TAG "%s\n", __func__);
	i2c_del_driver(&hscdtd_driver);
}

module_init(hscdtd_init);
module_exit(hscdtd_exit);

MODULE_DESCRIPTION("Alps HSCDTD Device");
MODULE_AUTHOR("ALPS ELECTRIC CO., LTD.");
MODULE_LICENSE("GPL v2");
