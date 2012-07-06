/* drivers/input/misc/lsm303dlh_acc.c
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/lsm303dlh_acc.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/input.h>

/*#define DEBUG_ATTR */

#define SHIFT_ADJ_2G    4
#define SHIFT_ADJ_4G    3
#define SHIFT_ADJ_8G    2

/* CTRL_REG1 */
#define PM_OFF            0x00
#define PM_NORMAL         0x20
#define ENABLE_ALL_AXES   0x07
#define ODRHALF           0x40  /* 0.5Hz output data rate */
#define ODR1              0x60  /* 1Hz output data rate */
#define ODR2              0x80  /* 2Hz output data rate */
#define ODR5              0xA0  /* 5Hz output data rate */
#define ODR10             0xC0  /* 10Hz output data rate */
#define ODR50             0x00  /* 50Hz output data rate */
#define ODR100            0x08  /* 100Hz output data rate */
#define ODR400            0x10  /* 400Hz output data rate */
#define ODR1000           0x18  /* 1000Hz output data rate */

/* CTRL_REG2 */
#define REBOOT              0x80
#define HI_PASS_NORMAL      0x00
#define HI_PASS_REF         0x20
#define HI_PASS_BYPASS      0x00
#define HI_PASS_OUT         0x10
#define HI_PASS_INT1        0x04
#define HI_PASS_INT2        0x08
#define HI_PASS_CF8         0x00
#define HI_PASS_CF16        0x01
#define HI_PASS_CF32        0x02
#define HI_PASS_CF64        0x03

/* CTRL_REG3 */
#define INT_HI_ACTIVE       0x00
#define INT_LO_ACTIVE       0x80
#define PUSHPULL_OPENDRAIN  0x40
#define INT2_LATCH          0x20
#define INT2_SRC            0x00
#define INT2_OR_INT1        0x08
#define INT2_DATA_RDY       0x10
#define INT2_BOOT           0x18
#define INT1_LATCH          0x04
#define INT1_SRC            0x00
#define INT1_OR_INT2        0x01
#define INT1_DATA_RDY       0x02
#define INT1_BOOT           0x03

/* CTRL_REG4 */
#define BLOCK_DATA_UPDATE   0x80
#define BIG_ENDIAN          0x40
#define LITTLE_ENDIAN       0x00
#define FSC_2G              0x00
#define FSC_4G              0x10
#define FSC_8G              0x30
#define FSC_MASK            0x30

/* CTRL_REG5 */
#define SLEEP_TO_WAKE       0x03

/* INTx_CFG */
#define OR_EVENTS           0x00
#define MOV_6D              0x40
#define AND_EVENTS          0x80
#define POS_6D              0xc0
#define ENABLE_LO_XYZ       0x15
#define ENABLE_HI_XYZ       0x2a
#define MOTION_INT_CFG      (MOV_6D | ENABLE_HI_XYZ)
#define POSITION_INT_CFG    (POS_6D | ENABLE_LO_XYZ)

#define WAKE_INT_CFG        (OR_EVENTS | ENABLE_HI_XYZ)
#define STILL_INT_CFG       (AND_EVENTS | ENABLE_LO_XYZ)
#define WAKE_DURATION       2
#define STILL_DURATION      5
#define WAKE_THRESHOLD      2
#define STILL_THRESHOLD     3

#define AUTO_INCREMENT      0x80
#define ACC_RANGE_MG        8000
#define CHIP_ID             0x32

struct output_rate {
	int poll_rate_ms;
	u8 mask;
};

static const struct output_rate odr_table[] = {
	{   1, PM_NORMAL | ODR1000},
	{   3, PM_NORMAL | ODR400 },
	{  10, PM_NORMAL | ODR100 },
	{  20, PM_NORMAL | ODR50  },
	{ 100,   ODR1000 | ODR10  },
	{ 200,   ODR1000 | ODR5   },
	{ 500,   ODR1000 | ODR2   },
	{1000,   ODR1000 | ODR1   },
	{2000,   ODR1000 | ODRHALF},
};

enum lsm303dlh_acc_irq_engine {
	IRQ_WAKE,
	IRQ_STILL,
};

enum lsm303dlh_acc_regs {
	REG_WHO_I_AM,
	REG_CTL1,
	REG_CTL2,
	REG_CTL3,
	REG_CTL4,
	REG_CTL5,
	REG_HP_F_RESET,
	REG_REF,
	REG_STATUS,
	REG_XL,
	REG_XH,
	REG_YL,
	REG_YH,
	REG_ZL,
	REG_ZH,
	REG_INT1_CFG,
	REG_INT1_SRC,
	REG_INT1_THS,
	REG_INT1_DUR,
	REG_INT2_CFG,
	REG_INT2_SRC,
	REG_INT2_THS,
	REG_INT2_DUR,
	REG_COUNT
};

#define AXISDATA_REG REG_XL

static const u8 lsm303dlh_acc_reg_map[] = {
	[REG_WHO_I_AM]   = 0x0f,
	[REG_CTL1]       = 0x20,
	[REG_CTL2]       = 0x21,
	[REG_CTL3]       = 0x22,
	[REG_CTL4]       = 0x23,
	[REG_CTL5]       = 0x24,
	[REG_HP_F_RESET] = 0x25,
	[REG_REF]        = 0x26,
	[REG_STATUS]     = 0x27,
	[REG_XL]         = 0x28,
	[REG_XH]         = 0x29,
	[REG_YL]         = 0x2a,
	[REG_YH]         = 0x2b,
	[REG_ZL]         = 0x2c,
	[REG_ZH]         = 0x2d,
	[REG_INT1_CFG]   = 0x30,
	[REG_INT1_SRC]   = 0x31,
	[REG_INT1_THS]   = 0x32,
	[REG_INT1_DUR]   = 0x33,
	[REG_INT2_CFG]   = 0x34,
	[REG_INT2_SRC]   = 0x35,
	[REG_INT2_THS]   = 0x36,
	[REG_INT2_DUR]   = 0x37,
};

struct lsm303dlh_acc_data {
	int irq;
	int irq_pad;
	int poll_interval;
	u8 regs[REG_COUNT];
#ifdef DEBUG_ATTR
	u8 reg_addr;
#endif
	u8 fsc;
	unsigned shift:3;
	unsigned powered:1;
	unsigned polling:1;
	struct i2c_client *client;
	struct delayed_work work;
	struct mutex lock;
	struct input_dev *input_dev;
	enum lsm303dlh_acc_range range;
	int poll_interval_ms;
	char *phys_dev_path;
	int (*power_on)(void);
	int (*power_off)(void);
};

#define LOCK(p) do { \
	dev_dbg(&(p)->client->dev, "%s: lock\n", __func__); \
	mutex_lock(&p->lock); \
} while (0)

#define UNLOCK(p) do { \
	dev_dbg(&(p)->client->dev, "%s: unlock\n", __func__); \
	mutex_unlock(&p->lock); \
} while (0)

#define INIT_LOCK(p) mutex_init(&p->lock);

/**
 * lsm303dlh_acc_i2c_read - read data from chip
 * @acc: Pointer to driver data structure
 * @buf: Byte array into which data will be read; On input first byte
 *       holds chip register offset
 * @len: number of bytes to be read from chip
*/
static int lsm303dlh_acc_i2c_read(struct lsm303dlh_acc_data *acc,
				  void *buf, int len)
{
	int err;
	u8 reg = *(u8 *)buf & ~AUTO_INCREMENT;
	struct i2c_msg msgs[] = {
		{
			.addr = acc->client->addr,
			.flags = acc->client->flags & I2C_M_TEN,
			.len = 1,
			.buf = buf,
		},
		{
			.addr = acc->client->addr,
			.flags = (acc->client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

	err = i2c_transfer(acc->client->adapter, msgs, 2);
	if (err != 2) {
		dev_err(&acc->client->dev, "%s: i2c read error %d (reg 0x%x)\n",
		       __func__, err, reg);
		return -EIO;
	}
	return 0;
}

/**
 * lsm303dlh_acc_i2c_write - write data to chip
 * @acc: Pointer to driver data structure
 * @buf: Byte array which will be written including register offset
 *       in the first byte;
 * @len: number of bytes to be written including register offset
*/
static int lsm303dlh_acc_i2c_write(struct lsm303dlh_acc_data *acc,
				   void *buf, int len)
{
	int err;
	struct i2c_msg msg = {
		.addr = acc->client->addr,
		.flags = acc->client->flags & I2C_M_TEN,
		.len = len,
		.buf = buf,
	};

	err = i2c_transfer(acc->client->adapter, &msg, 1);
	if (err != 1) {
		dev_err(&acc->client->dev, "%s: i2c write error"
			" %d (reg 0x%x)\n",
		       __func__, err, *(u8 *)buf & ~AUTO_INCREMENT);
		return -EIO;
	}
	return 0;
}

static int lsm303dlh_acc_wr_reg(struct lsm303dlh_acc_data *acc,
				   enum lsm303dlh_acc_regs reg, u8 data)
{
	int rc = 0;
	if (data != acc->regs[reg]) {
		struct  {
			u8 reg;
			u8 data;
		} __attribute__((packed)) x =
		{lsm303dlh_acc_reg_map[reg], data};

		rc = lsm303dlh_acc_i2c_write(acc, &x, sizeof(x));
		if (!rc) {
			acc->regs[reg] = data;
			dev_dbg(&acc->client->dev, "%s: 0x%02x -> reg[%02x]\n",
				   __func__, data, x.reg);
		}
	}
	return rc;
}

static int lsm303dlh_acc_flush_shadow(struct lsm303dlh_acc_data *acc,
				   enum lsm303dlh_acc_regs start_reg,
				   u8 nregs)
{
	struct  {
		u8 reg;
		u8 buf[REG_COUNT];
	} __attribute__((packed)) x;

	x.reg = lsm303dlh_acc_reg_map[start_reg] | AUTO_INCREMENT;
	memcpy(x.buf, &acc->regs[start_reg], nregs);
	return lsm303dlh_acc_i2c_write(acc, &x, nregs + sizeof(x.reg));
}


static int lsm303dlh_acc_rd_reg(struct lsm303dlh_acc_data *acc,
				   enum lsm303dlh_acc_regs reg, u8 *data)
{
	int rc = 0;

	*data = lsm303dlh_acc_reg_map[reg];
	rc = lsm303dlh_acc_i2c_read(acc, data, 1);
	if (!rc) {
		acc->regs[reg] = *data;
		dev_dbg(&acc->client->dev, "%s: 0x%02x -> reg[%02x]\n",
			   __func__, *data, lsm303dlh_acc_reg_map[reg]);
	}
	return rc;
}

static int lsm303dlh_acc_update_shadow(struct lsm303dlh_acc_data *acc,
				   enum lsm303dlh_acc_regs start_reg,
				   u8 nregs)
{
	int rc;

	acc->regs[start_reg] = lsm303dlh_acc_reg_map[start_reg] |
		AUTO_INCREMENT;
	rc = lsm303dlh_acc_i2c_read(acc, &acc->regs[start_reg], nregs);
	return rc;
}

static int lsm303dlh_acc_set_range(struct lsm303dlh_acc_data *acc)
{
	switch (acc->range) {
	case LSM303_RANGE_2G:
		acc->fsc = FSC_2G;
		acc->shift = SHIFT_ADJ_2G;
		break;
	case LSM303_RANGE_4G:
		acc->fsc = FSC_4G;
		acc->shift = SHIFT_ADJ_4G;
		break;
	case LSM303_RANGE_8G:
		acc->fsc = FSC_8G;
		acc->shift = SHIFT_ADJ_8G;
		break;
	default:
		dev_err(&acc->client->dev, "%s: range %d is not supported\n",
		       __func__, acc->range);
		return -EINVAL;
	}
	return 0;
}

static int lsm303dlh_acc_set_sampling_rate(struct lsm303dlh_acc_data *acc)
{
	int i;
	int err;
	u8 config;
	int poll_interval = acc->poll_interval_ms;

	for (i = ARRAY_SIZE(odr_table) - 1; i >= 0; i--) {
		if (odr_table[i].poll_rate_ms <= poll_interval)
			break;
	}
	config = odr_table[i].mask;
	acc->regs[REG_CTL1] = config | ENABLE_ALL_AXES;
	err = lsm303dlh_acc_flush_shadow(acc, REG_CTL1, 1);
	if (!err) {
		dev_dbg(&acc->client->dev, "%s sampling %d ms\n",
			   __func__, odr_table[i].poll_rate_ms);
	}
	lsm303dlh_acc_rd_reg(acc, REG_HP_F_RESET, &config);
	return err;
}

static void lsm303dlh_acc_set_interrupt(struct lsm303dlh_acc_data *acc,
					enum lsm303dlh_acc_irq_engine e)
{
	u8 reg;

	if (e == IRQ_WAKE) {
		acc->regs[REG_INT2_CFG] = WAKE_INT_CFG;
		acc->regs[REG_INT2_THS] = WAKE_THRESHOLD;
		acc->regs[REG_INT2_DUR] = WAKE_DURATION;
		acc->regs[REG_CTL4] = (acc->regs[REG_CTL4] & ~FSC_MASK) |
					FSC_2G;
	} else {
		acc->regs[REG_INT2_CFG] = STILL_INT_CFG;
		acc->regs[REG_INT2_THS] = STILL_THRESHOLD;
		acc->regs[REG_INT2_DUR] = STILL_DURATION;
		acc->regs[REG_CTL4] = (acc->regs[REG_CTL4] & ~FSC_MASK) |
					acc->fsc;
	}
	lsm303dlh_acc_flush_shadow(acc, REG_CTL4, 1);
	lsm303dlh_acc_flush_shadow(acc, REG_INT2_THS, 2);
	lsm303dlh_acc_flush_shadow(acc, REG_INT2_CFG, 1);
	lsm303dlh_acc_rd_reg(acc, REG_HP_F_RESET, &reg);

}

static void lsm303dlh_acc_setup(struct lsm303dlh_acc_data *acc)
{
	acc->regs[REG_CTL2] = HI_PASS_NORMAL | HI_PASS_INT2 | HI_PASS_CF64;
	acc->regs[REG_CTL3] = INT_HI_ACTIVE | INT2_LATCH | INT2_OR_INT1 |
				INT1_LATCH | INT1_OR_INT2;
	acc->regs[REG_CTL4] = BLOCK_DATA_UPDATE | LITTLE_ENDIAN;
	acc->regs[REG_CTL5] = 0;
	lsm303dlh_acc_flush_shadow(acc, REG_CTL2, REG_CTL5 - REG_CTL2 + 1);

	acc->regs[REG_INT1_CFG] = 0;
	lsm303dlh_acc_flush_shadow(acc, REG_INT1_CFG, 1);

	lsm303dlh_acc_set_range(acc);
	if (acc->irq >= 0)
		lsm303dlh_acc_set_interrupt(acc, IRQ_WAKE);
	lsm303dlh_acc_set_sampling_rate(acc);
}

static void lsm303dlh_acc_device_power_off(struct lsm303dlh_acc_data *acc)
{
	if (acc->powered) {
		acc->powered = 0;
		if (acc->irq >= 0)
			disable_irq(acc->irq);
		lsm303dlh_acc_wr_reg(acc, REG_CTL1, PM_OFF);

		if (acc->irq > -1)
			disable_irq(acc->irq);

		acc->power_off();
		dev_dbg(&acc->client->dev, "%s: power-off\n", __func__);
	}
}

static void lsm303dlh_acc_device_power_on(struct lsm303dlh_acc_data *acc)
{
	if (!acc->powered) {
		acc->power_on();
		lsm303dlh_acc_setup(acc);
		dev_dbg(&acc->client->dev, "%s: power-on\n", __func__);
		acc->powered = 1;
		if (acc->irq < 0) {
			acc->polling = 1;
			schedule_delayed_work(&acc->work,
				msecs_to_jiffies(acc->poll_interval_ms));
		} else {
			acc->polling = 0;
			enable_irq(acc->irq);
		}
	}
}

static void lsm303dlh_acc_poll_func(struct work_struct *work)
{
	struct lsm303dlh_acc_data *acc =
			container_of((struct delayed_work *)work,
			struct lsm303dlh_acc_data, work);

	LOCK(acc);
	if (acc->powered && acc->polling) {
		s16 *accel = (s16 *)&acc->regs[AXISDATA_REG];
		u8 shift = acc->shift;

		lsm303dlh_acc_update_shadow(acc, REG_XL, REG_ZH - REG_XL + 1);
		dev_dbg(&acc->client->dev, "%s: ACCEL: %6d, %6d, %6d\n",
				__func__,  accel[0] >> shift,
				accel[1] >> shift,
				accel[2] >> shift);
		input_report_abs(acc->input_dev, ABS_X, accel[0] >> shift);
		input_report_abs(acc->input_dev, ABS_Y, accel[1] >> shift);
		input_report_abs(acc->input_dev, ABS_Z, accel[2] >> shift);
		input_sync(acc->input_dev);
		schedule_delayed_work(&acc->work,
				msecs_to_jiffies(acc->poll_interval_ms));
	}
	UNLOCK(acc);
}

static irqreturn_t lsm303dlh_acc_isr(int irq, void *dev_id)
{
	struct lsm303dlh_acc_data *acc = dev_id;
	enum {
		DO_NOTHING,
		START_POLLING,
		STOP_POLLING,
	} action = DO_NOTHING;

	dev_dbg(&acc->client->dev, "%s\n", __func__);
	LOCK(acc);
	if (acc->powered) {
		u8 int_src;
		if (acc->regs[REG_INT2_CFG] == WAKE_INT_CFG) {
			dev_dbg(&acc->client->dev, "%s: motion interrupt\n",
				   __func__);
			lsm303dlh_acc_set_interrupt(acc, IRQ_STILL);
			action = START_POLLING;
			acc->polling = 1;
		} else {
			dev_dbg(&acc->client->dev, "%s: position interrupt\n",
				   __func__);
			lsm303dlh_acc_set_interrupt(acc, IRQ_WAKE);
			action = STOP_POLLING;
			acc->polling = 0;
		}
		lsm303dlh_acc_rd_reg(acc, REG_INT2_SRC, &int_src);
		dev_dbg(&acc->client->dev, "%s: irq src 0x%02x\n",
			   __func__, int_src);
	}
	UNLOCK(acc);
	if (action == START_POLLING)
		schedule_delayed_work(&acc->work,
				msecs_to_jiffies(acc->poll_interval_ms));
	else if (action == STOP_POLLING)
		cancel_delayed_work_sync(&acc->work);
	dev_dbg(&acc->client->dev, "%s, finished\n", __func__);
	return IRQ_HANDLED;
}

#ifdef CONFIG_SUSPEND
static int lsm303dlh_acc_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lsm303dlh_acc_data *acc = i2c_get_clientdata(client);

	dev_dbg(dev, "%s\n", __func__);
	LOCK(acc);
	lsm303dlh_acc_device_power_off(acc);
	UNLOCK(acc);
	cancel_delayed_work_sync(&acc->work);
	return 0;
}

static int lsm303dlh_acc_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lsm303dlh_acc_data *acc = i2c_get_clientdata(client);

	dev_dbg(dev, "%s\n", __func__);
	LOCK(acc);
	if (acc->input_dev->users) {
		dev_dbg(dev, "%s: resuming active"
			   " operation.\n", __func__);
		lsm303dlh_acc_device_power_on(acc);
	}
	UNLOCK(acc);
	return 0;
}
#else
#define lsm303dlh_acc_suspend NULL
#define lsm303dlh_acc_resume NULL
#endif

static int lsm303dlh_acc_open(struct input_dev *dev)
{
	struct lsm303dlh_acc_data *acc = input_get_drvdata(dev);

	LOCK(acc);
	lsm303dlh_acc_device_power_on(acc);
	UNLOCK(acc);
	return 0;
}

static void lsm303dlh_acc_close(struct input_dev *dev)
{
	struct lsm303dlh_acc_data *acc = input_get_drvdata(dev);

	LOCK(acc);
	lsm303dlh_acc_device_power_off(acc);
	UNLOCK(acc);
	cancel_delayed_work_sync(&acc->work);
}

static ssize_t attr_set_poll_rate(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	unsigned long interval_ms;

	if (strict_strtoul(buf, 10, &interval_ms))
		return -EINVAL;
	if (!interval_ms)
		return -EINVAL;
	LOCK(acc);
	acc->poll_interval_ms = interval_ms;
	if (acc->powered)
		lsm303dlh_acc_set_sampling_rate(acc);
	UNLOCK(acc);
	return size;
}

static ssize_t attr_get_poll_rate(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", acc->poll_interval_ms);
}

static ssize_t attr_set_range(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	unsigned long val;
	enum lsm303dlh_acc_range r;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;
	switch (val) {
	case 2:
		r = LSM303_RANGE_2G;
		break;
	case 4:
		r = LSM303_RANGE_4G;
		break;
	case 8:
		r = LSM303_RANGE_8G;
		break;
	default:
		return -EINVAL;
	}
	LOCK(acc);
	acc->range = r;
	if (acc->powered)
		lsm303dlh_acc_set_range(acc);
	UNLOCK(acc);
	return size;
}

static ssize_t attr_get_range(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	enum lsm303dlh_acc_range r;
	int range;

	LOCK(acc);
	r = acc->range;
	UNLOCK(acc);
	switch (r) {
	case LSM303_RANGE_2G:
		range = 2;
		break;
	case LSM303_RANGE_4G:
		range = 4;
		break;
	case LSM303_RANGE_8G:
		range = 8;
		break;
	default:
		range = 0;
	}
	return sprintf(buf, "%d\n", range);
}

#ifdef DEBUG_ATTR
static ssize_t attr_reg_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	int rc;
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	struct  {
		u8 reg;
		u8 data;
	} __attribute__((packed)) x;
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;
	LOCK(acc);
	x.reg = acc->reg_addr;
	UNLOCK(acc);
	x.data = val;
	rc = lsm303dlh_acc_i2c_write(acc, &x, sizeof(x));
	dev_dbg(&acc->client->dev, "%s: 0x%02x -> reg[%02x], rc = %d\n",
		__func__, x.data, x.reg, rc);
	return size;
}

static ssize_t attr_reg_get(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	int rc;
	u8 data ;

	LOCK(acc);
	data = acc->reg_addr;
	UNLOCK(acc);
	rc = lsm303dlh_acc_i2c_read(acc, &data, 1);
	dev_dbg(&acc->client->dev, "%s: reg[%02x] <- 0x%02x, rc = %d\n",
		__func__, acc->reg_addr, data, rc);
	ret = sprintf(buf, "0x%02x\n", data);
	return ret;
}


static ssize_t attr_addr_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	struct lsm303dlh_acc_data *acc = dev_get_drvdata(dev);
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;
	LOCK(acc);
	acc->reg_addr = val;
	UNLOCK(acc);
	return size;
}
#endif /* DEBUG_ATTR */

static struct device_attribute attributes[] = {
	__ATTR(pollrate_ms, 0666, attr_get_poll_rate, attr_set_poll_rate),
	__ATTR(range, 0666, attr_get_range, attr_set_range),
#ifdef DEBUG_ATTR
	__ATTR(reg_value, 0600, attr_reg_get, attr_reg_set),
	__ATTR(reg_addr, 0200, NULL, attr_addr_set),
#endif
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;
error:
	for (; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	return -EIO;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

static int lsm303dlh_acc_setup_irq(struct lsm303dlh_acc_data *acc,
				   struct lsm303dlh_acc_platform_data *pdata)
{
	int err;

	acc->irq_pad = pdata->irq_pad;
	if (acc->irq_pad < 0) {
		dev_info(&acc->client->dev, "%s: no IRQ configured\n", __func__);
		acc->irq = -1;
		return 0;
	}
	err = gpio_request(acc->irq_pad, "LSM303DLH_ACC");
	if (err) {
		dev_err(&acc->client->dev, "%s: unable to request GPIO %d\n",
			__func__, acc->irq_pad);
		return err;
	}
	acc->irq = gpio_to_irq(acc->irq_pad);
	err = request_threaded_irq(acc->irq, NULL, lsm303dlh_acc_isr,
				   IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				   "LSM303DLH_ACC", acc);
	if (err) {
		dev_err(&acc->client->dev, "%s: unable to request IRQ %d\n",
			__func__, acc->irq);
		goto err_exit1;
	}
	disable_irq(acc->irq);
	return 0;
	gpio_free(acc->irq_pad);
err_exit1:
	free_irq(acc->irq, acc);
	return err;
}

static int lsm303dlh_acc_power_stub(void)
{
	return 0;
}

static int __devinit lsm303dlh_acc_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lsm303dlh_acc_platform_data *pdata = client->dev.platform_data;
	struct lsm303dlh_acc_data *acc;
	int result = 0;
	u8 chip_id;

	dev_info(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		result = -EIO;
		goto err_check_functionality;
	}
	if (!pdata) {
		result = -EINVAL;
		dev_err(&client->dev, "%s: platform data required.\n",
			__func__);
		goto err_no_platform_data;
	}
	acc = kzalloc(sizeof(*acc), GFP_KERNEL);
	if (!acc) {
		result = -ENOMEM;
		goto err_alloc_data_failed;
	}
	acc->client = client;
	acc->range = pdata->range ? pdata->range : LSM303_RANGE_8G;
	acc->poll_interval_ms = pdata->poll_interval_ms ?
			pdata->poll_interval_ms : 100;
	INIT_DELAYED_WORK(&acc->work, lsm303dlh_acc_poll_func);
	INIT_LOCK(acc);

	if (pdata->power_on)
		acc->power_on = pdata->power_on;
	else
		acc->power_on = lsm303dlh_acc_power_stub;

	if (pdata->power_off)
		acc->power_off = pdata->power_off;
	else
		acc->power_off = lsm303dlh_acc_power_stub;
	i2c_set_clientdata(client, acc);

	acc->power_on();
	result = lsm303dlh_acc_rd_reg(acc, REG_WHO_I_AM, &chip_id);
	acc->power_off();
	if (result) {
		dev_err(&client->dev, "%s: device not responding\n", __func__);
		goto err_not_responding;
	}
	dev_info(&client->dev, "%s: chip ID 0x%02x\n", __func__, chip_id);
	if (CHIP_ID != chip_id) {
		dev_err(&client->dev, "%s: wrong chip ID\n", __func__);
		result = -ENODEV;
		goto err_wrong_id;
	}
	result = create_sysfs_interfaces(&client->dev);
	if (result)
		goto sys_attr_err;

	acc->input_dev = input_allocate_device();
	if (!acc->input_dev) {
		dev_err(&client->dev, "%s: input_allocate_device failed\n",
			__func__);
		result = -ENOMEM;
		goto err_allocate_device;
	}
	input_set_drvdata(acc->input_dev, acc);

	acc->input_dev->open = lsm303dlh_acc_open;
	acc->input_dev->close = lsm303dlh_acc_close;
	acc->input_dev->name = LSM303DLH_ACC_DEV_NAME;
	set_bit(EV_ABS, acc->input_dev->evbit);
	set_bit(ABS_X, acc->input_dev->absbit);
	set_bit(ABS_Y, acc->input_dev->absbit);
	set_bit(ABS_Z, acc->input_dev->absbit);
	input_set_abs_params(acc->input_dev, ABS_X, -ACC_RANGE_MG,
			ACC_RANGE_MG - 1, 0, 0);
	input_set_abs_params(acc->input_dev, ABS_Y, -ACC_RANGE_MG,
			ACC_RANGE_MG - 1, 0, 0);
	input_set_abs_params(acc->input_dev, ABS_Z, -ACC_RANGE_MG,
			ACC_RANGE_MG - 1, 0, 0);

	result = input_register_device(acc->input_dev);
	if (result) {
		dev_err(&client->dev, "%s: input_register_device failed!",
			__func__);
		input_free_device(acc->input_dev);
		goto err_register_device;
	}

	result = lsm303dlh_acc_setup_irq(acc, pdata);
	if (result)
		goto request_irq_err;

	dev_info(&client->dev, "%s completed.\n", __func__);
	return 0;

request_irq_err:
	input_unregister_device(acc->input_dev);
err_register_device:
err_allocate_device:
	remove_sysfs_interfaces(&client->dev);
sys_attr_err:
err_wrong_id:
err_not_responding:
	kfree(acc);
err_alloc_data_failed:
err_no_platform_data:
err_check_functionality:
	printk(KERN_ERR "%s failed.\n", __func__);
	return result;
}

static int __devexit lsm303dlh_acc_remove(struct i2c_client *client)
{
	struct lsm303dlh_acc_data *acc = i2c_get_clientdata(client);

	input_unregister_device(acc->input_dev);
	free_irq(acc->irq, acc);
	gpio_free(acc->irq_pad);
	remove_sysfs_interfaces(&client->dev);
	lsm303dlh_acc_device_power_off(acc);
	kfree(acc);
	return 0;
}

static const struct i2c_device_id lsm303dlh_acc_id[] = {
	{LSM303DLH_ACC_DEV_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, lsm303dlh_acc_id);

static const struct dev_pm_ops lsm303dlh_acc_pm = {
	.suspend = lsm303dlh_acc_suspend,
	.resume = lsm303dlh_acc_resume,
};

static struct i2c_driver lsm303dlh_acc_driver = {
	.driver = {
		.name = LSM303DLH_ACC_DEV_NAME,
		.owner = THIS_MODULE,
		.pm = &lsm303dlh_acc_pm,
	},
	.probe = lsm303dlh_acc_probe,
	.remove = __devexit_p(lsm303dlh_acc_remove),
	.id_table = lsm303dlh_acc_id,
};

static int __init lsm303dlh_acc_init(void)
{
	int err = i2c_add_driver(&lsm303dlh_acc_driver);
	return err;
}

static void __exit lsm303dlh_acc_exit(void)
{
	i2c_del_driver(&lsm303dlh_acc_driver);
}

module_init(lsm303dlh_acc_init);
module_exit(lsm303dlh_acc_exit);

MODULE_AUTHOR("Aleksej Makarov <aleksej.makarov@sonyericsson.com>");
MODULE_LICENSE("GPLv2");
MODULE_DESCRIPTION("lsm303dlh accelerometer driver");
