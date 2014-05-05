/*  Date: 2011/7/4 17:00:00
 *  Revision: 2.7
 */

/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
 * (C) Copyright 2014 Sony Mobile Communications
 * All Rights Reserved
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/bma2xx.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/spinlock.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define BMA2X2_MAX_FIFO_LEVEL 32
#define BMA2XX_SW_CALIBRATION 1

#define BMA2XXX_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define BMA2XXX_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

enum bma2xx_orientation {
	ORI_X_Y_Z,
	ORI_Y_NX_Z,
	ORI_NX_NY_Z,
	ORI_NY_X_Z,
	ORI_Y_X_NZ,
	ORI_X_NY_NZ,
	ORI_NY_NX_NZ,
	ORI_NX_Y_NZ,
};

static const char * const orientation_id[] = {
	"ORI_X_Y_Z",
	"ORI_Y_NX_Z",
	"ORI_NX_NY_Z",
	"ORI_NY_X_Z",
	"ORI_Y_X_NZ",
	"ORI_X_NY_NZ",
	"ORI_NY_NX_NZ",
	"ORI_NX_Y_NZ",
};

struct bma2xxacc {
	s16 x, y, z;
};

enum bma2xx_func {
	/* wakeups first */
	BMA_WAKE_LOWG,
	BMA_WAKE_HIGHG_X,
	BMA_WAKE_HIGHG_Y,
	BMA_WAKE_HIGHG_Z,
	BMA_WAKE_DRDY,
	BMA_WAKE_SLOPE_X,
	BMA_WAKE_SLOPE_Y,
	BMA_WAKE_SLOPE_Z,
	BMA_WAKE_STAP,
	BMA_WAKE_DTAP,
	BMA_WAKE_ORIENT,
	BMA_WAKE_FLAT,
	BMA_WAKE_NOMOTION_XYZ,
	BMA_WAKE_SLOWMOTION_XYZ,
	BMA_LAST_REAL_WAKE = BMA_WAKE_SLOWMOTION_XYZ,
	/* not really wakeups below */
	BMA_POLL_DATA,
	BMA_ENABLE_WAKE,
	BMA_EN_WAKE_ON_EARLY_SUSPEND,
	BMA_FIFO_WM,
};

enum bma2xx_state {
	DEV_SUSPENDED = 1 << 0,
	IRQ1_PENDING  = 1 << 1,
	IRQ2_PENDING  = 1 << 2,
};

enum bma2xx_range {
	BOSCH_ACCEL_SENSOR_RANGE_2G = 3,
	BOSCH_ACCEL_SENSOR_RANGE_4G = 5,
	BOSCH_ACCEL_SENSOR_RANGE_8G = 8,
	BOSCH_ACCEL_SENSOR_RANGE_16G = 12,
};

enum bma2xx_chip_id {
	ID_BMA250 = 3,
	ID_BMA255 = 250,
};

#define BMA_FUNC(bit) (1 << (bit))
#define WAKE_SET(enable) \
	((enable & (BMA_FUNC(BMA_LAST_REAL_WAKE + 1) - 1)) != 0)
#define WAKE_ENABLED(enable) ((enable & BMA_FUNC(BMA_ENABLE_WAKE)) != 0)
#define DATA_ENABLED(enable) ((enable & \
	(BMA_FUNC(BMA_POLL_DATA) | BMA_FUNC(BMA_FIFO_WM))) != 0)

struct bma2xx_data {
	struct i2c_client *bma2xx_client;
	atomic_t delay;
	int enable;
	atomic_t selftest_result;
	unsigned char mode;
	struct input_dev *input;
	struct bma2xxacc value;
	struct mutex value_mutex;
	struct mutex enable_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	struct work_struct irq_work;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int IRQ;
	enum bma2xx_orientation orientation;
	struct regulator *regulator;
	struct input_dev *wake_idev;
	spinlock_t lock;
	int state;
	struct work_struct fifo_work;
	struct work_struct flush_work;
	int data_irq;
	u8 r_shift;
	bool fflush;
	int axis_num_map[3];
	bool nomotion;
	bool wuff;
};

#ifdef BMA2XX_SW_CALIBRATION
static int bma2xx_offset[3];
#endif

static const struct bma2xxacc axis_num = {1, 2, 3};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma2xx_early_suspend(struct early_suspend *h);
static void bma2xx_late_resume(struct early_suspend *h);
#endif

static int bma2xx_smbus_read_byte(struct i2c_client *client,
				  unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	*data = dummy & 0x000000ff;

	return 0;
}

static int bma2xx_smbus_write_byte(struct i2c_client *client,
				   unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	dev_dbg(&client->dev, "%s: 0x%02x -> [0x%02x] = %d\n",
			__func__, reg_addr, *data, dummy);
	return 0;
}

static int bma2xx_smbus_read_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	return 0;
}

static int bma2xx_big_block_read(struct i2c_client *cl, u8 addr,
	int len, char *rec_buf)
{
	int rc;
	struct i2c_msg msg[2] = {
		[0] =  {
			.addr = cl->addr,
			.flags = cl->flags & I2C_M_TEN,
			.len = 1,
			.buf = &addr,
		},
		[1] =  {
			.addr = cl->addr,
			.flags = (cl->flags & I2C_M_TEN) | I2C_M_RD,
			.len = len,
			.buf = rec_buf,
		},
	};
	rc = i2c_transfer(cl->adapter, msg, ARRAY_SIZE(msg));
	if (rc == ARRAY_SIZE(msg))
		return 0;
	dev_err(&cl->dev, "%s: rc %d\n", __func__, rc);
	return rc < 0 ? rc : -EIO;
}

static int bma2xx_set_mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data1;

	if (Mode < 3) {
		comres = bma2xx_smbus_read_byte(client,
						BMA2XXX_EN_LOW_POWER__REG,
						&data1);
		switch (Mode) {
		case BMA2XXX_MODE_NORMAL:
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_LOW_POWER, 0);
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_SUSPEND, 0);
			break;
		case BMA2XXX_MODE_LOWPOWER:
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_LOW_POWER, 1);
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_SUSPEND, 0);
			break;
		case BMA2XXX_MODE_SUSPEND:
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_LOW_POWER, 0);
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_EN_SUSPEND, 1);
			break;
		default:
			break;
		}

		comres += bma2xx_smbus_write_byte(client,
						  BMA2XXX_EN_LOW_POWER__REG,
						  &data1);
	} else {
		comres = -1;
	}

	return comres;
}

static int bma2xx_set_int1_pad_sel(struct i2c_client *client,
				   enum bma2xx_func int1sel, int state)
{
	int comres = 0;
	unsigned char data;
	state &= 0x01;

	switch (int1sel) {
	case BMA_WAKE_LOWG:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_LOWG__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_LOWG, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_LOWG__REG,
					    &data);
		break;
	case BMA_WAKE_HIGHG_X:
	case BMA_WAKE_HIGHG_Y:
	case BMA_WAKE_HIGHG_Z:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_HIGHG__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_HIGHG,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_HIGHG__REG,
					    &data);
		break;
	case BMA_WAKE_SLOPE_X:
	case BMA_WAKE_SLOPE_Y:
	case BMA_WAKE_SLOPE_Z:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_SLOPE__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_SLOPE,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_SLOPE__REG,
					    &data);
		break;

	case BMA_WAKE_NOMOTION_XYZ:
	case BMA_WAKE_SLOWMOTION_XYZ:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_NOMOT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_NOMOT,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_NOMOT__REG,
					    &data);
		break;

	case BMA_WAKE_DTAP:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_DB_TAP__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_DB_TAP,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_DB_TAP__REG,
					    &data);
		break;
	case BMA_WAKE_STAP:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_SNG_TAP__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_SNG_TAP,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_SNG_TAP__REG,
					    &data);
		break;
	case BMA_WAKE_ORIENT:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_ORIENT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_ORIENT,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_ORIENT__REG,
					    &data);
		break;
	case BMA_WAKE_FLAT:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_FLAT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT1_PAD_FLAT, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_FLAT__REG,
					    &data);
		break;
	case BMA_FIFO_WM:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT1_PAD_FIFOWM__REG,
					   &data);
		data = BMA2XXX_SET_BITSLICE(data,
				BMA2XXX_EN_INT1_PAD_FIFOWM, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT1_PAD_FIFOWM__REG,
					    &data);
		break;
	default:
		break;
	}
	return comres;
}

static int bma2xx_set_int2_pad_sel(struct i2c_client *client,
				   enum bma2xx_func int2sel, int state)
{
	int comres = 0;
	unsigned char data;
	state &= 0x01;

	switch (int2sel) {
	case BMA_WAKE_LOWG:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_LOWG__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_LOWG, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_LOWG__REG,
					    &data);
		break;
	case BMA_WAKE_HIGHG_X:
	case BMA_WAKE_HIGHG_Y:
	case BMA_WAKE_HIGHG_Z:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_HIGHG__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_HIGHG,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_HIGHG__REG,
					    &data);
		break;
	case BMA_WAKE_SLOPE_X:
	case BMA_WAKE_SLOPE_Y:
	case BMA_WAKE_SLOPE_Z:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_SLOPE__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_SLOPE,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_SLOPE__REG,
					    &data);
		break;

	case BMA_WAKE_NOMOTION_XYZ:
	case BMA_WAKE_SLOWMOTION_XYZ:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_NOMOT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_NOMOT,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_NOMOT__REG,
					    &data);
		break;

	case BMA_WAKE_DTAP:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_DB_TAP__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_DB_TAP,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_DB_TAP__REG,
					    &data);
		break;
	case BMA_WAKE_STAP:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_SNG_TAP__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_SNG_TAP,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_SNG_TAP__REG,
					    &data);
		break;
	case BMA_WAKE_ORIENT:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_ORIENT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_ORIENT,
					 state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_ORIENT__REG,
					    &data);
		break;
	case BMA_WAKE_FLAT:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_FLAT__REG,
					   &data);
		data =
		    BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_INT2_PAD_FLAT, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_FLAT__REG,
					    &data);
		break;
	case BMA_FIFO_WM:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_EN_INT2_PAD_FIFOWM__REG,
					   &data);
		data = BMA2XXX_SET_BITSLICE(data,
				BMA2XXX_EN_INT2_PAD_FIFOWM, state);
		comres =
		    bma2xx_smbus_write_byte(client,
					    BMA2XXX_EN_INT2_PAD_FIFOWM__REG,
					    &data);
		break;
	default:
		break;
	}
	return comres;
}

static int bma2xx_set_pad_sel(struct i2c_client *client,
	enum bma2xx_func int2sel, int state, int pad)
{
	if (pad == 1)
		return bma2xx_set_int1_pad_sel(client, int2sel, state);
	if (pad == 2)
		return bma2xx_set_int2_pad_sel(client, int2sel, state);
	return -EINVAL;
}

struct bma_func_map {
	union {
		struct {
			int x, y, z;
		};
		enum bma2xx_func slope_xyz[3];
	};
};

static struct bma_func_map bma_func_map[] = {
	[ORI_X_Y_Z] = { .x = BMA_WAKE_SLOPE_X, .y = BMA_WAKE_SLOPE_Y,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_NX_NY_Z] = { .x = BMA_WAKE_SLOPE_X, .y = BMA_WAKE_SLOPE_Y,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_X_NY_NZ] = { .x = BMA_WAKE_SLOPE_X, .y = BMA_WAKE_SLOPE_Y,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_NX_Y_NZ] = { .x = BMA_WAKE_SLOPE_X, .y = BMA_WAKE_SLOPE_Y,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_Y_NX_Z] = { .x = BMA_WAKE_SLOPE_Y, .y = BMA_WAKE_SLOPE_X,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_NY_X_Z] = { .x = BMA_WAKE_SLOPE_Y, .y = BMA_WAKE_SLOPE_X,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_Y_X_NZ] = { .x = BMA_WAKE_SLOPE_Y, .y = BMA_WAKE_SLOPE_X,
			.z = BMA_WAKE_SLOPE_Z },
	[ORI_NY_NX_NZ] = { .x = BMA_WAKE_SLOPE_Y, .y = BMA_WAKE_SLOPE_X,
			.z = BMA_WAKE_SLOPE_Z },
};

static enum bma2xx_func bma2xx_map_interrupt_type(
		struct bma2xx_data *bma2xx,
		enum bma2xx_func irq_type)
{
	int n;

	switch (irq_type) {
	case BMA_WAKE_SLOPE_X:
	case BMA_WAKE_SLOPE_Y:
	case BMA_WAKE_SLOPE_Z:
		n = irq_type - BMA_WAKE_SLOPE_X;
		return bma_func_map[bma2xx->orientation].slope_xyz[n];
	default:
		return irq_type;
	}
}

static int bma2xx_set_Int_Enable(struct i2c_client *client,
				 enum bma2xx_func interrupt_type,
				 unsigned char value)
{
	int comres = 0;
	unsigned char data1, data2, data3;
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_INT_ENABLE1_REG, &data1);
	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_INT_ENABLE2_REG, &data2);
	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_INT_ENABLE3_REG, &data3);

	value = value & 1;
	interrupt_type = bma2xx_map_interrupt_type(bma2xx, interrupt_type);

	switch (interrupt_type) {
	case BMA_WAKE_LOWG:
		/* Low G Interrupt  */
		data2 = BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_LOWG_INT, value);
		break;
	case BMA_WAKE_HIGHG_X:
		/* High G X Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_X_INT, value);
		break;
	case BMA_WAKE_HIGHG_Y:
		/* High G Y Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_Y_INT, value);
		break;
	case BMA_WAKE_HIGHG_Z:
		/* High G Z Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_Z_INT, value);
		break;
	case BMA_WAKE_DRDY:
		/* New Data Interrupt  */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_NEW_DATA_INT, value);
		break;
	case BMA_WAKE_SLOPE_X:
		/* Slope X Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_X_INT, value);
		break;
	case BMA_WAKE_SLOPE_Y:
		/* Slope Y Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_Y_INT, value);
		break;
	case BMA_WAKE_SLOPE_Z:
		/* Slope Z Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_Z_INT, value);
		break;

	case BMA_WAKE_NOMOTION_XYZ:
		/* No motion on all axis */
		bma2xx->nomotion = true;
		data3 =
		    BMA2XXX_SET_BITSLICE(data3, BMA2XXX_EN_NOMOT_XYZ_INT,
		    value ? 0xf : 0);
		break;

	case BMA_WAKE_SLOWMOTION_XYZ:
		/* slow motion on all axis */
		bma2xx->nomotion = false;
		data3 =
		    BMA2XXX_SET_BITSLICE(data3, BMA2XXX_EN_NOMOT_XYZ_INT,
		    value ? 0x7 : 0);
		break;

	case BMA_WAKE_STAP:
		/* Single Tap Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SINGLE_TAP_INT,
					 value);
		break;
	case BMA_WAKE_DTAP:
		/* Double Tap Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_DOUBLE_TAP_INT,
					 value);
		break;
	case BMA_WAKE_ORIENT:
		/* Orient Interrupt  */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_ORIENT_INT, value);
		break;
	case BMA_WAKE_FLAT:
		/* Flat Interrupt */

		data1 = BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_FLAT_INT, value);
		break;
	case BMA_FIFO_WM:
		/* FIFO watermark Interrupt */
		data2 = BMA2XXX_SET_BITSLICE(data2,
				BMA2XXX_EN_FIFOWM_INT, value);
		break;
	default:
		break;
	}
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_INT_ENABLE1_REG, &data1);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_INT_ENABLE2_REG, &data2);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_INT_ENABLE3_REG, &data3);

	return comres;
}

static int bma2xx_enable_func_locked(struct bma2xx_data *bma2xx,
		enum bma2xx_func sel, int state)
{
	int rc;
	bma2xx->enable = state ? bma2xx->enable | BMA_FUNC(sel) :
			bma2xx->enable & ~BMA_FUNC(sel);
	if (sel <= BMA_LAST_REAL_WAKE) {
		struct i2c_client *client = bma2xx->bma2xx_client;

		rc = bma2xx_set_Int_Enable(client, sel, state);
		if (rc)
			goto err;
		switch (state) {
		case 1:
			rc = bma2xx_set_int1_pad_sel(client, sel, 1);
			break;
		case 2:
			rc = bma2xx_set_int2_pad_sel(client, sel, 1);
			break;
		case 0:
			rc = bma2xx_set_int1_pad_sel(client, sel, 0);
			rc = rc ? rc :
				bma2xx_set_int2_pad_sel(client, sel, 0);
			break;
		default:
			rc = -EINVAL;
			break;
		}
	} else {
		rc = 0;
	}
err:
	return rc;
}

static int bma2xx_get_mode(struct i2c_client *client, unsigned char *Mode)
{
	int comres = 0;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_EN_LOW_POWER__REG, Mode);
	*Mode = (*Mode) >> 6;

	return comres;
}

static int bma2xx_set_range(struct i2c_client *client, unsigned char Range)
{
	int comres = 0;
	unsigned char data1;

	if (Range < 4) {
		comres = bma2xx_smbus_read_byte(client,
						BMA2XXX_RANGE_SEL_REG, &data1);
		switch (Range) {
		case 0:
			data1 =
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL,
					BOSCH_ACCEL_SENSOR_RANGE_2G);
			break;
		case 1:
			data1 =
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL,
					BOSCH_ACCEL_SENSOR_RANGE_4G);
			break;
		case 2:
			data1 =
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL,
					BOSCH_ACCEL_SENSOR_RANGE_8G);
			break;
		case 3:
			data1 = BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL,
					BOSCH_ACCEL_SENSOR_RANGE_16G);
			break;
		default:
			break;
		}
		comres += bma2xx_smbus_write_byte(client,
						  BMA2XXX_RANGE_SEL_REG,
						  &data1);
	} else {
		comres = -1;
	}

	return comres;
}

static int bma2xx_get_range(struct i2c_client *client, unsigned char *Range)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_RANGE_SEL__REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_RANGE_SEL);
	*Range = data;

	return comres;
}

static int bma2xx_set_bandwidth(struct i2c_client *client, unsigned char BW)
{
	int comres = 0;
	unsigned char data;
	int Bandwidth = 0;

	if (BW < 8) {
		switch (BW) {
		case 0:
			Bandwidth = BMA2XXX_BW_7_81HZ;
			break;
		case 1:
			Bandwidth = BMA2XXX_BW_15_63HZ;
			break;
		case 2:
			Bandwidth = BMA2XXX_BW_31_25HZ;
			break;
		case 3:
			Bandwidth = BMA2XXX_BW_62_50HZ;
			break;
		case 4:
			Bandwidth = BMA2XXX_BW_125HZ;
			break;
		case 5:
			Bandwidth = BMA2XXX_BW_250HZ;
			break;
		case 6:
			Bandwidth = BMA2XXX_BW_500HZ;
			break;
		case 7:
			Bandwidth = BMA2XXX_BW_1000HZ;
			break;
		default:
			break;
		}
		comres = bma2xx_smbus_read_byte(client,
						BMA2XXX_BANDWIDTH__REG, &data);
		data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_BANDWIDTH, Bandwidth);
		comres += bma2xx_smbus_write_byte(client,
						  BMA2XXX_BANDWIDTH__REG,
						  &data);
	} else {
		comres = -1;
	}

	return comres;
}

static int bma2xx_get_bandwidth(struct i2c_client *client, unsigned char *BW)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_BANDWIDTH__REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_BANDWIDTH);
	if (data <= 8) {
		*BW = 0;
	} else {
		if (data >= 0x0F)
			*BW = 7;
		else
			*BW = data - 8;

	}

	return comres;
}

static int bma2xx_get_interruptstatus1(struct i2c_client *client,
				       unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_STATUS1_REG, &data);
	*intstatus = data;

	return comres;
}

static int bma2xx_get_HIGH_first(struct i2c_client *client, unsigned char param,
				 unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	switch (param) {
	case 0:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_STATUS_ORIENT_HIGH_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_FIRST_X);
		*intstatus = data;
		break;
	case 1:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_STATUS_ORIENT_HIGH_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_FIRST_Y);
		*intstatus = data;
		break;
	case 2:
		comres =
		    bma2xx_smbus_read_byte(client,
					   BMA2XXX_STATUS_ORIENT_HIGH_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_FIRST_Z);
		*intstatus = data;
		break;
	default:
		break;
	}

	return comres;
}

static int bma2xx_get_HIGH_sign(struct i2c_client *client,
				unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_ORIENT_HIGH_REG,
				   &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_SIGN_S);
	*intstatus = data;

	return comres;
}

static int bma2xx_get_slope_first(struct i2c_client *client,
				  unsigned char param, unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	switch (param) {
	case 0:
		comres =
		    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_TAP_SLOPE_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_FIRST_X);
		*intstatus = data;
		break;
	case 1:
		comres =
		    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_TAP_SLOPE_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_FIRST_Y);
		*intstatus = data;
		break;
	case 2:
		comres =
		    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_TAP_SLOPE_REG,
					   &data);
		data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_FIRST_Z);
		*intstatus = data;
		break;
	default:
		break;
	}

	return comres;
}

static int bma2xx_get_slope_sign(struct i2c_client *client,
				 unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_TAP_SLOPE_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_SIGN_S);
	*intstatus = data;

	return comres;
}

static int bma2xx_get_orient_status(struct i2c_client *client,
				    unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_ORIENT_HIGH_REG,
				   &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_ORIENT_S);
	*intstatus = data;

	return comres;
}

static int bma2xx_get_orient_flat_status(struct i2c_client *client,
					 unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_STATUS_ORIENT_HIGH_REG,
				   &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_FLAT_S);
	*intstatus = data;

	return comres;
}

static int bma2xx_set_Int_Mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client,
					BMA2XXX_INT_MODE_SEL__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_INT_MODE_SEL, Mode);
	comres = bma2xx_smbus_write_byte(client,
					 BMA2XXX_INT_MODE_SEL__REG, &data);

	return comres;
}

static int bma2xx_get_Int_Mode(struct i2c_client *client, unsigned char *Mode)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client,
					BMA2XXX_INT_MODE_SEL__REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_INT_MODE_SEL);
	*Mode = data;

	return comres;
}
static int bma2xx_set_slope_duration(struct i2c_client *client,
				     unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_SLOPE_DUR__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_SLOPE_DUR, duration);
	comres = bma2xx_smbus_write_byte(client, BMA2XXX_SLOPE_DUR__REG, &data);

	return comres;
}

static int bma2xx_get_slope_duration(struct i2c_client *client,
				     unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_SLOPE_DURN_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_DUR);
	*status = data;

	return comres;
}

static int bma2xx_set_slope_threshold(struct i2c_client *client,
				      unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_SLOPE_THRES__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_SLOPE_THRES, threshold);
	comres = bma2xx_smbus_write_byte(client,
					 BMA2XXX_SLOPE_THRES__REG, &data);

	return comres;
}

static int bma2xx_get_slope_threshold(struct i2c_client *client,
				      unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_SLOPE_THRES_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_SLOPE_THRES);
	*status = data;

	return comres;
}
static int bma2xx_set_low_g_duration(struct i2c_client *client,
				     unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_LOWG_DUR__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_LOWG_DUR, duration);
	comres = bma2xx_smbus_write_byte(client, BMA2XXX_LOWG_DUR__REG, &data);

	return comres;
}

static int bma2xx_get_low_g_duration(struct i2c_client *client,
				     unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_LOW_DURN_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_LOWG_DUR);
	*status = data;

	return comres;
}

static int bma2xx_set_low_g_threshold(struct i2c_client *client,
				      unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_LOWG_THRES__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_LOWG_THRES, threshold);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_LOWG_THRES__REG, &data);

	return comres;
}

static int bma2xx_get_low_g_threshold(struct i2c_client *client,
				      unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_LOW_THRES_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_LOWG_THRES);
	*status = data;

	return comres;
}

static int bma2xx_set_high_g_duration(struct i2c_client *client,
				      unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_HIGHG_DUR__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_HIGHG_DUR, duration);
	comres = bma2xx_smbus_write_byte(client, BMA2XXX_HIGHG_DUR__REG, &data);

	return comres;
}

static int bma2xx_get_high_g_duration(struct i2c_client *client,
				      unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_HIGH_DURN_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_DUR);
	*status = data;

	return comres;
}

static int bma2xx_set_high_g_threshold(struct i2c_client *client,
				       unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_HIGHG_THRES__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_HIGHG_THRES, threshold);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_HIGHG_THRES__REG, &data);

	return comres;
}

static int bma2xx_get_high_g_threshold(struct i2c_client *client,
				       unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_HIGH_THRES_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_HIGHG_THRES);
	*status = data;

	return comres;
}

static int bma2xx_set_tap_duration(struct i2c_client *client,
				   unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_DUR__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_TAP_DUR, duration);
	comres = bma2xx_smbus_write_byte(client, BMA2XXX_TAP_DUR__REG, &data);

	return comres;
}

static int bma2xx_get_tap_duration(struct i2c_client *client,
				   unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_TAP_DUR);
	*status = data;

	return comres;
}

static int bma2xx_set_tap_shock(struct i2c_client *client, unsigned char setval)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_TAP_SHOCK_DURN__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_TAP_SHOCK_DURN, setval);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_TAP_SHOCK_DURN__REG, &data);

	return comres;
}

static int bma2xx_get_tap_shock(struct i2c_client *client,
				unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_TAP_SHOCK_DURN);
	*status = data;

	return comres;
}

static int bma2xx_set_tap_quiet(struct i2c_client *client,
				unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_TAP_QUIET_DURN__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_TAP_QUIET_DURN, duration);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_TAP_QUIET_DURN__REG, &data);

	return comres;
}

static int bma2xx_get_tap_quiet(struct i2c_client *client,
				unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_TAP_QUIET_DURN);
	*status = data;

	return comres;
}

static int bma2xx_set_tap_threshold(struct i2c_client *client,
				    unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_THRES__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_TAP_THRES, threshold);
	comres = bma2xx_smbus_write_byte(client, BMA2XXX_TAP_THRES__REG, &data);

	return comres;
}

static int bma2xx_get_tap_threshold(struct i2c_client *client,
				    unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_THRES_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_TAP_THRES);
	*status = data;

	return comres;
}

static int bma2xx_set_tap_samp(struct i2c_client *client, unsigned char samp)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_TAP_SAMPLES__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_TAP_SAMPLES, samp);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_TAP_SAMPLES__REG, &data);

	return comres;
}

static int bma2xx_get_tap_samp(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_TAP_THRES_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_TAP_SAMPLES);
	*status = data;

	return comres;
}

static int bma2xx_set_orient_mode(struct i2c_client *client, unsigned char mode)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_MODE__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_ORIENT_MODE, mode);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_ORIENT_MODE__REG, &data);

	return comres;
}

static int bma2xx_get_orient_mode(struct i2c_client *client,
				  unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_ORIENT_MODE);
	*status = data;

	return comres;
}

static int bma2xx_set_orient_blocking(struct i2c_client *client,
				      unsigned char samp)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_BLOCK__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_ORIENT_BLOCK, samp);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_ORIENT_BLOCK__REG, &data);

	return comres;
}

static int bma2xx_get_orient_blocking(struct i2c_client *client,
				      unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_ORIENT_BLOCK);
	*status = data;

	return comres;
}

static int bma2xx_set_orient_hyst(struct i2c_client *client,
				  unsigned char orienthyst)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_HYST__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_ORIENT_HYST, orienthyst);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_ORIENT_HYST__REG, &data);

	return comres;
}

static int bma2xx_get_orient_hyst(struct i2c_client *client,
				  unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_ORIENT_PARAM_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_ORIENT_HYST);
	*status = data;

	return comres;
}
static int bma2xx_set_theta_blocking(struct i2c_client *client,
				     unsigned char thetablk)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_THETA_BLOCK__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_THETA_BLOCK, thetablk);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_THETA_BLOCK__REG, &data);

	return comres;
}

static int bma2xx_get_theta_blocking(struct i2c_client *client,
				     unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_THETA_BLOCK_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_THETA_BLOCK);
	*status = data;

	return comres;
}

static int bma2xx_set_theta_flat(struct i2c_client *client,
				 unsigned char thetaflat)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_THETA_FLAT__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_THETA_FLAT, thetaflat);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_THETA_FLAT__REG, &data);

	return comres;
}

static int bma2xx_get_theta_flat(struct i2c_client *client,
				 unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_THETA_FLAT_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_THETA_FLAT);
	*status = data;

	return comres;
}

static int bma2xx_set_flat_hold_time(struct i2c_client *client,
				     unsigned char holdtime)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_FLAT_HOLD_TIME__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_FLAT_HOLD_TIME, holdtime);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_FLAT_HOLD_TIME__REG, &data);

	return comres;
}

static int bma2xx_get_flat_hold_time(struct i2c_client *client,
				     unsigned char *holdtime)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_FLAT_HOLD_TIME_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_FLAT_HOLD_TIME);
	*holdtime = data;

	return comres;
}

static int bma2xx_write_reg(struct i2c_client *client, unsigned char addr,
			    unsigned char *data)
{
	int comres = 0;
	comres = bma2xx_smbus_write_byte(client, addr, data);

	return comres;
}

static int bma2xx_set_offset_target_x(struct i2c_client *client,
				      unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_COMP_TARGET_OFFSET_X__REG,
				   &data);
	data =
	    BMA2XXX_SET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_X,
				 offsettarget);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_COMP_TARGET_OFFSET_X__REG,
				    &data);

	return comres;
}

static int bma2xx_get_offset_target_x(struct i2c_client *client,
				      unsigned char *offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_OFFSET_PARAMS_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_X);
	*offsettarget = data;

	return comres;
}

static int bma2xx_set_offset_target_y(struct i2c_client *client,
				      unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_COMP_TARGET_OFFSET_Y__REG,
				   &data);
	data =
	    BMA2XXX_SET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_Y,
				 offsettarget);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_COMP_TARGET_OFFSET_Y__REG,
				    &data);

	return comres;
}

static int bma2xx_get_offset_target_y(struct i2c_client *client,
				      unsigned char *offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_OFFSET_PARAMS_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_Y);
	*offsettarget = data;

	return comres;
}

static int bma2xx_set_offset_target_z(struct i2c_client *client,
				      unsigned char offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_COMP_TARGET_OFFSET_Z__REG,
				   &data);
	data =
	    BMA2XXX_SET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_Z,
				 offsettarget);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_COMP_TARGET_OFFSET_Z__REG,
				    &data);

	return comres;
}

static int bma2xx_get_offset_target_z(struct i2c_client *client,
				      unsigned char *offsettarget)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_OFFSET_PARAMS_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_COMP_TARGET_OFFSET_Z);
	*offsettarget = data;

	return comres;
}

static int bma2xx_get_cal_ready(struct i2c_client *client,
				unsigned char *calrdy)
{
	int comres = 0;
	unsigned char data;

	comres = bma2xx_smbus_read_byte(client, BMA2XXX_OFFSET_CTRL_REG, &data);
	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_FAST_COMP_RDY_S);
	*calrdy = data;

	return comres;
}

static int bma2xx_set_cal_trigger(struct i2c_client *client,
				  unsigned char caltrigger)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_EN_FAST_COMP__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_FAST_COMP, caltrigger);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_EN_FAST_COMP__REG, &data);

	return comres;
}

static int bma2xx_set_selftest_st(struct i2c_client *client,
				  unsigned char selftest)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_EN_SELF_TEST__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_EN_SELF_TEST, selftest);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_EN_SELF_TEST__REG, &data);

	return comres;
}

static int bma2xx_set_selftest_stn(struct i2c_client *client, unsigned char stn)
{
	int comres = 0;
	unsigned char data;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_NEG_SELF_TEST__REG, &data);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_NEG_SELF_TEST, stn);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_NEG_SELF_TEST__REG, &data);

	return comres;
}

static int bma2xx_read_axis(struct i2c_client *client, short *acc, int axis)
{
	struct bma2xx_data *bma = i2c_get_clientdata(client);
	int comres = bma2xx_smbus_read_byte_block(client,
			BMA2XXX_ACC_X_LSB__REG + axis * 2, (char *)acc,
			sizeof(*acc));

#ifdef __BIG_ENDIAN
	*acc = swab16(*acc);
#endif
	*acc >>= bma->r_shift;
	return comres;
}

static int bma2xx_read_accel_x(struct i2c_client *client, short *a_x)
{
	return bma2xx_read_axis(client, a_x, 0);
}

static int bma2xx_read_accel_y(struct i2c_client *client, short *a_y)
{
	return bma2xx_read_axis(client, a_y, 1);
}

static int bma2xx_read_accel_z(struct i2c_client *client, short *a_z)
{
	return bma2xx_read_axis(client, a_z, 2);
}

static int bma2xx_read_accel_xyz(struct bma2xx_data *bma, struct bma2xxacc *acc)
{
	int comres = bma2xx_smbus_read_byte_block(bma->bma2xx_client,
			BMA2XXX_ACC_X_LSB__REG, (char *)acc, sizeof(*acc));

#ifdef __BIG_ENDIAN
	acc->x = swab16(acc->x);
	acc->y = swab16(acc->y);
	acc->z = swab16(acc->z);
#endif
	acc->x >>= bma->r_shift;
	acc->y >>= bma->r_shift;
	acc->z >>= bma->r_shift;
	return comres;
}

static enum bma2xx_orientation bma2xx_get_orientation(const char *name)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(orientation_id); i++)
		if (!strcmp(name, orientation_id[i]))
			return i;
	return -1;
}

static void bma2xx_map_axes(struct bma2xx_data *bma2xx,
	const struct bma2xxacc *acc, int *xyz)
{
	switch (bma2xx->orientation) {
	case ORI_X_Y_Z:
		xyz[0] = acc->x;
		xyz[1] = acc->y;
		xyz[2] = acc->z;
		break;
	case ORI_Y_NX_Z:
		xyz[0] = acc->y;
		xyz[1] = -acc->x;
		xyz[2] = acc->z;
		break;
	case ORI_NX_NY_Z:
		xyz[0] = -acc->x;
		xyz[1] = -acc->y;
		xyz[2] = acc->z;
		break;
	case ORI_NY_X_Z:
		xyz[0] = -acc->y;
		xyz[1] = acc->x;
		xyz[2] = acc->z;
		break;
	case ORI_Y_X_NZ:
		xyz[0] = acc->y;
		xyz[1] = acc->x;
		xyz[2] = -acc->z;
		break;
	case ORI_X_NY_NZ:
		xyz[0] = acc->x;
		xyz[1] = -acc->y;
		xyz[2] = -acc->z;
		break;
	case ORI_NY_NX_NZ:
		xyz[0] = -acc->y;
		xyz[1] = -acc->x;
		xyz[2] = -acc->z;
		break;
	default:
	case ORI_NX_Y_NZ:
		xyz[0] = -acc->x;
		xyz[1] = acc->y;
		xyz[2] = -acc->z;
		break;
	}
}

static void bma2xx_report_data(struct bma2xx_data *bma2xx,
		struct input_dev *dev)
{
	static struct bma2xxacc acc;
	int xyz[3];

	bma2xx_read_accel_xyz(bma2xx, &acc);
	bma2xx_map_axes(bma2xx, &acc, xyz);

#ifdef BMA2XX_SW_CALIBRATION
	xyz[0] -= bma2xx_offset[0];
	xyz[1] -= bma2xx_offset[1];
	xyz[2] -= bma2xx_offset[2];
#endif
	input_event(dev, EV_MSC, MSC_SERIAL, xyz[0]);
	input_event(dev, EV_MSC, MSC_PULSELED, xyz[1]);
	input_event(dev, EV_MSC, MSC_GESTURE, xyz[2]);
	mutex_lock(&bma2xx->value_mutex);
	bma2xx->value = acc;
	mutex_unlock(&bma2xx->value_mutex);
}

static void bma2xx_work_func(struct work_struct *work)
{
	struct bma2xx_data *bma2xx = container_of((struct delayed_work *)work,
						  struct bma2xx_data, work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma2xx->delay));

	bma2xx_report_data(bma2xx, bma2xx->input);
	input_sync(bma2xx->input);
	schedule_delayed_work(&bma2xx->work, delay);
}

static int bma2xx_fifo_config(struct bma2xx_data *bma, int wm, int pad)
{
	int rc;
	unsigned char data = 0;
	struct i2c_client *cl = bma->bma2xx_client;

	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_FIFO_DATA_SEL, FIFO_DATA_XYZ);
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_FIFO_MODE, FIFO_MODE_STREAM);
	rc = bma2xx_smbus_write_byte(cl, BMA2XXX_FIFO_DATA_SEL__REG, &data);
	if (rc)
		goto err;
	data = 0;
	data = BMA2XXX_SET_BITSLICE(data, BMA2XXX_FIFO_WM, wm);
	rc = bma2xx_smbus_write_byte(cl, BMA2XXX_FIFO_WM__REG, &data);
	if (rc)
		goto err;
	if (wm) {
		rc = bma2xx_set_pad_sel(cl, BMA_FIFO_WM, 1, pad);
		rc = rc ? rc : bma2xx_set_Int_Enable(cl, BMA_FIFO_WM, 1);
	} else {
		rc = bma2xx_set_Int_Enable(cl, BMA_FIFO_WM, 0);
		rc = rc ? rc : bma2xx_set_pad_sel(cl, BMA_FIFO_WM, 0, 1);
		rc = rc ? rc : bma2xx_set_pad_sel(cl, BMA_FIFO_WM, 0, 2);
	}
err:
	return rc;
}

static int bma2xx_fifo_read(struct bma2xx_data *bma)
{
	struct i2c_client *cl = bma->bma2xx_client;
	unsigned char frame_cnt;
	struct bma2xxacc acc[BMA2X2_MAX_FIFO_LEVEL];
	int xyz[3];
	unsigned i;
	int rc;

	mutex_lock(&bma->value_mutex);
	rc = bma2xx_smbus_read_byte(cl,
			BMA2XXX_FIFO_STATUS_REG, &frame_cnt);
	if (rc)
		goto err;
	frame_cnt = BMA2XXX_GET_BITSLICE(frame_cnt, BMA2XXX_FIFO_FRAME);

	dev_dbg(&cl->dev, "%s: %d frames\n", __func__, frame_cnt);

	if (!frame_cnt)
		goto err;

	rc = bma2xx_big_block_read(cl, BMA2XXX_FIFO_DATA_REG,
			frame_cnt * 6, (char *)acc);
	if (rc)
		goto err;

	for (i = 0; i < frame_cnt; i++) {
#ifdef __BIG_ENDIAN
		acc[i].x = swab16(acc[i].x);
		acc[i].y = swab16(acc[i].y);
		acc[i].z = swab16(acc[i].z);
#endif
		acc[i].x >>= bma->r_shift;
		acc[i].y >>= bma->r_shift;
		acc[i].z >>= bma->r_shift;
		bma2xx_map_axes(bma, &acc[i], xyz);
#ifdef BMA2XX_SW_CALIBRATION
		xyz[0] -= bma2xx_offset[0];
		xyz[1] -= bma2xx_offset[1];
		xyz[2] -= bma2xx_offset[2];
#endif
		input_event(bma->input, EV_MSC, MSC_SERIAL, xyz[0]);
		input_event(bma->input, EV_MSC, MSC_PULSELED, xyz[1]);
		input_event(bma->input, EV_MSC, MSC_GESTURE, xyz[2]);
	}
	input_sync(bma->input);
err:
	mutex_unlock(&bma->value_mutex);
	return rc;
}

static int bma2xx_set_optimal_mode_locked(struct bma2xx_data *data,
	bool in_suspend)
{
	int power_mode;

	power_mode = (!in_suspend || data->wuff) && DATA_ENABLED(data->enable) ?
			BMA2XXX_MODE_NORMAL :
			WAKE_SET(data->enable) && WAKE_ENABLED(data->enable) ?
			BMA2XXX_MODE_LOWPOWER :
			BMA2XXX_MODE_SUSPEND;

	dev_dbg(&data->bma2xx_client->dev,
		"%s: in suspend %d wuff %d enable 0x%x, power mode %d\n",
		__func__, in_suspend, data->wuff, data->enable, power_mode);
	return bma2xx_set_mode(data->bma2xx_client, power_mode);
}

static int bma2xx_allow_wake(struct bma2xx_data *bma, bool allow)
{
	int rc;
	mutex_lock(&bma->enable_mutex);
	if (allow)
		bma->enable |= BMA_FUNC(BMA_ENABLE_WAKE);
	else
		bma->enable &= ~BMA_FUNC(BMA_ENABLE_WAKE);
	rc = bma2xx_set_optimal_mode_locked(bma, false);
	mutex_unlock(&bma->enable_mutex);
	return rc;
}

#define attr_to_bma2xx(dev) \
	((struct bma2xx_data *)input_get_drvdata(to_input_dev(dev)))

static ssize_t bma2xx_register_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	int address, value;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	sscanf(buf, "%d%d", &address, &value);

	if (bma2xx_write_reg(client, (unsigned char)address,
			(unsigned char *)&value) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_register_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	size_t count = 0;
	u8 reg[0x40];
	int i;

	for (i = 0; i < sizeof(reg); i++) {
		bma2xx_smbus_read_byte(client, i, reg + i);

		count += sprintf(&buf[count], "0x%x: %d\n", i, reg[i]);
	}
	return count;

}
static ssize_t bma2xx_range_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_range(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_range_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_range(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_bandwidth_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_bandwidth(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_bandwidth_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_bandwidth(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_mode(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_mode_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_mode(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_value_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bma2xx_data *bma2xx = input_get_drvdata(input);
	struct bma2xxacc acc_value;

	mutex_lock(&bma2xx->value_mutex);
	acc_value = bma2xx->value;
	mutex_unlock(&bma2xx->value_mutex);

	return sprintf(buf, "%d %d %d\n", acc_value.x, acc_value.y,
		       acc_value.z);
}

static ssize_t bma2xx_delay_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);

	return sprintf(buf, "%d\n", atomic_read(&bma2xx->delay));

}

static ssize_t bma2xx_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > BMA2XXX_MAX_DELAY)
		data = BMA2XXX_MAX_DELAY;
	atomic_set(&bma2xx->delay, (unsigned int)data);

	return count;
}

static ssize_t bma2xx_enable_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{

	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);

	return sprintf(buf, "0x%x\n", bma2xx->enable);

}

/*
 * enable = 0 - no data produced
 * enable = 1 - timer polling mode
 * enable > 0  && < 32 - FIFO mode
 * enable >= 32 - not allowed value
 */
static int bma2xx_set_enable(struct device *dev, int enable)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	int rc;

	mutex_lock(&bma2xx->enable_mutex);
	if (!enable) {
		bma2xx_enable_func_locked(bma2xx, BMA_FIFO_WM, 0);
		bma2xx_enable_func_locked(bma2xx, BMA_POLL_DATA, 0);
	} else if (enable == 1) {
		/* timer polling mode */
		bma2xx_enable_func_locked(bma2xx, BMA_POLL_DATA, 1);
		bma2xx_enable_func_locked(bma2xx, BMA_FIFO_WM, 0);
	} else {
		/* enable < BMA2X2_MAX_FIFO_LEVEL : FIFO mode */
		bma2xx_enable_func_locked(bma2xx, BMA_POLL_DATA, 0);
		bma2xx_enable_func_locked(bma2xx, BMA_FIFO_WM, 1);
	}
	if (enable != 1) {
		dev_dbg(&bma2xx->bma2xx_client->dev,
				"%s: stop polling\n", __func__);
		cancel_delayed_work_sync(&bma2xx->work);
	}
	rc = bma2xx_set_optimal_mode_locked(bma2xx, false);
	if (rc)
		goto err;
	if (enable == 1) {
		dev_dbg(&bma2xx->bma2xx_client->dev,
				"%s: start polling\n", __func__);
		schedule_delayed_work(&bma2xx->work,
				msecs_to_jiffies(atomic_read(&bma2xx->delay)));
	}
	rc = bma2xx_fifo_config(bma2xx, enable > 1 ? enable : 0, 2);
err:
	mutex_unlock(&bma2xx->enable_mutex);
	return rc;
}

static ssize_t bma2xx_enable_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (data < BMA2X2_MAX_FIFO_LEVEL)
		error = bma2xx_set_enable(dev, data);
	else
		error = -EINVAL;
	return error ? error : count;
}

static void bma2xx_fifo_flush_func(struct work_struct *w)
{
	struct bma2xx_data *bma2xx = container_of(w,
			struct bma2xx_data, flush_work);
	bma2xx_fifo_read(bma2xx);
	input_event(bma2xx->input, EV_SYN, SYN_CONFIG, 0);
	input_sync(bma2xx->input);
}

static ssize_t bma2xx_fflush_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	unsigned long data;
	int error;

	error = kstrtoul(buf, 10, &data);
	if (error)
		goto err;
	if (data == 1)
		schedule_work(&bma2xx->flush_work);
	else
		error = -EINVAL;
err:
	return error ? error : count;
}


static ssize_t bma2xx_enable_int_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	int type, value;
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	int rc;

	rc = sscanf(buf, "%d%d", &type, &value);
	if (rc == 2 && type <= BMA_LAST_REAL_WAKE) {
		mutex_lock(&bma2xx->enable_mutex);
		rc = bma2xx_enable_func_locked(bma2xx, type, value);
		mutex_unlock(&bma2xx->enable_mutex);
	} else {
		dev_err(&bma2xx->bma2xx_client->dev, "%s Invalid argument\n",
				__func__);
		rc = -EINVAL;
	}
	return rc ? rc : count;
}

static ssize_t bma2xx_int_mode_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_Int_Mode(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_int_mode_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned long data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	data = kstrtoul(buf, 10, NULL);

	if (bma2xx_set_Int_Mode(client, data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_slope_duration_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_slope_duration(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_slope_duration_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_slope_duration(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_slope_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_slope_threshold(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_slope_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_slope_threshold(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_nomot_duration_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;
	int rc = bma2xx_smbus_read_byte(client, BMA2XXX_NOMOT_DUR__REG, &data);

	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_NOMOT_DUR);
	if (rc < 0)
		return rc;
	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_nomot_duration_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;
	unsigned char d;
	int rc;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	rc = bma2xx_smbus_read_byte(client, BMA2XXX_NOMOT_DUR__REG, &d);
	d = BMA2XXX_SET_BITSLICE(d, BMA2XXX_NOMOT_DUR, data);
	rc = rc < 0 ? rc :
		bma2xx_smbus_write_byte(client, BMA2XXX_NOMOT_DUR__REG, &d);
	if (rc < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_nomot_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;
	int rc = bma2xx_smbus_read_byte(client, BMA2XXX_NOMOT_THRES_REG, &data);

	data = BMA2XXX_GET_BITSLICE(data, BMA2XXX_NOMOT_THRES);
	if (rc < 0)
		return rc;
	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_nomot_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;
	unsigned char d;
	int rc;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	rc = bma2xx_smbus_read_byte(client, BMA2XXX_NOMOT_THRES__REG, &d);
	d = BMA2XXX_SET_BITSLICE(d, BMA2XXX_NOMOT_THRES, data);
	rc = rc < 0 ? rc :
		bma2xx_smbus_write_byte(client, BMA2XXX_NOMOT_THRES__REG, &d);
	if (rc < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_high_g_duration_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_high_g_duration(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_high_g_duration_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;
	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_high_g_duration(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_high_g_threshold_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_high_g_threshold(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_high_g_threshold_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_high_g_threshold(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_low_g_duration_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_low_g_duration(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_low_g_duration_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_low_g_duration(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_low_g_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_low_g_threshold(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_low_g_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_low_g_threshold(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_threshold_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_tap_threshold(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_threshold_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_tap_threshold(client, (unsigned char)data)
	    < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_duration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_tap_duration(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_duration_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_duration(client, (unsigned char)data)
	    < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_quiet_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_tap_quiet(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_quiet_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_quiet(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_tap_shock_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_tap_shock(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_shock_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_shock(client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_tap_samp_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_tap_samp(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_samp_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_samp(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_mode_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_orient_mode(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_mode_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_mode(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_blocking_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_orient_blocking(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_blocking_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_blocking(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_orient_hyst_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_orient_hyst(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_hyst_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_hyst(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_theta_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_theta_blocking(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_theta_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_theta_blocking(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_flat_theta_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_theta_flat(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_flat_theta_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_theta_flat(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_flat_hold_time_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_flat_hold_time(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_flat_hold_time_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_flat_hold_time(client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_fast_calibration_x_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{

	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_offset_target_x(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_fast_calibration_x_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_x(client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(client, 1) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(client, &tmp);

		printk(KERN_INFO "wait 2ms and got cal ready flag is %d\n",
		       tmp);
		timeout++;
		if (timeout == 50) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

	printk(KERN_INFO "x axis fast calibration finished\n");
	return count;
}

static ssize_t bma2xx_fast_calibration_y_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{

	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_offset_target_y(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_fast_calibration_y_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_y(client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(client, 2) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(client, &tmp);

		printk(KERN_INFO "wait 2ms and got cal ready flag is %d\n",
		       tmp);
		timeout++;
		if (timeout == 50) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

	printk(KERN_INFO "y axis fast calibration finished\n");
	return count;
}

static ssize_t bma2xx_fast_calibration_z_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{

	unsigned char data;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	if (bma2xx_get_offset_target_z(client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_fast_calibration_z_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	int error;
	struct i2c_client *client = attr_to_bma2xx(dev)->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_z(client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(client, 3) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(client, &tmp);

		printk(KERN_INFO "wait 2ms and got cal ready flag is %d\n",
		       tmp);
		timeout++;
		if (timeout == 50) {
			printk(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		};

	} while (tmp == 0);

	printk(KERN_INFO "z axis fast calibration finished\n");
	return count;
}

static ssize_t bma2xx_selftest_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);

	return sprintf(buf, "%d\n", atomic_read(&bma2xx->selftest_result));

}

static ssize_t bma2xx_selftest_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{

	unsigned long data;
	unsigned char clear_value = 0;
	int error;
	short value1 = 0;
	short value2 = 0;
	short diff = 0;
	unsigned long result = 0;
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	struct i2c_client *client = bma2xx->bma2xx_client;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (data != 1)
		return -EINVAL;
	/* set to 2 G range */
	if (bma2xx_set_range(client, 0) < 0)
		return -EINVAL;

	bma2xx_write_reg(client, 0x32, &clear_value);

	bma2xx_set_selftest_st(client, 1);
	bma2xx_set_selftest_stn(client, 0);
	mdelay(10);
	bma2xx_read_accel_x(client, &value1);
	bma2xx_set_selftest_stn(client, 1);
	mdelay(10);
	bma2xx_read_accel_x(client, &value2);
	diff = value1 - value2;

	printk(KERN_INFO "diff x is %d,value1 is %d, value2 is %d\n",
		diff, value1, value2);

	if (abs(diff) < 204)
		result |= 1;

	bma2xx_set_selftest_st(client, 2);
	bma2xx_set_selftest_stn(client, 0);
	mdelay(10);
	bma2xx_read_accel_y(client, &value1);
	bma2xx_set_selftest_stn(client, 1);
	mdelay(10);
	bma2xx_read_accel_y(client, &value2);
	diff = value1 - value2;
	printk(KERN_INFO "diff y is %d,value1 is %d, value2 is %d\n",
		diff, value1, value2);
	if (abs(diff) < 204)
		result |= 2;

	bma2xx_set_selftest_st(client, 3);
	bma2xx_set_selftest_stn(client, 0);
	mdelay(10);
	bma2xx_read_accel_z(client, &value1);
	bma2xx_set_selftest_stn(client, 1);
	mdelay(10);
	bma2xx_read_accel_z(client, &value2);
	diff = value1 - value2;

	printk(KERN_INFO "diff z is %d,value1 is %d, value2 is %d\n", diff,
	       value1, value2);
	if (abs(diff) < 102)
		result |= 4;

	atomic_set(&bma2xx->selftest_result, (unsigned int)result);

	printk(KERN_INFO "self test finished\n");

	return count;
}


static ssize_t bma2xx_get_allow_wake(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	return scnprintf(buf, PAGE_SIZE, "allow %d, suspend_allow %d\n",
			!!(bma2xx->enable & BMA_FUNC(BMA_ENABLE_WAKE)),
			!!(bma2xx->enable &
			BMA_FUNC(BMA_EN_WAKE_ON_EARLY_SUSPEND)));
}

static ssize_t bma2xx_set_allow_wake(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	int error = 0;

	if (!strncmp(buf, "allow", 5)) {
		error = bma2xx_allow_wake(bma2xx, true);
	} else if (!strncmp(buf, "deny", 4)) {
		error = bma2xx_allow_wake(bma2xx, false);
	} else if (!strncmp(buf, "suspend_allow", 13)) {
		mutex_lock(&bma2xx->enable_mutex);
		bma2xx->enable |= BMA_FUNC(BMA_EN_WAKE_ON_EARLY_SUSPEND);
		mutex_unlock(&bma2xx->enable_mutex);
	} else if (!strncmp(buf, "suspend_deny", 12)) {
		mutex_lock(&bma2xx->enable_mutex);
		bma2xx->enable &= ~BMA_FUNC(BMA_EN_WAKE_ON_EARLY_SUSPEND);
		mutex_unlock(&bma2xx->enable_mutex);
	} else {
		error = -EINVAL;
	}
	return error ? error : count;
}
static DEVICE_ATTR(wake_allow,  S_IRUGO | S_IWUSR | S_IWGRP,
		bma2xx_get_allow_wake, bma2xx_set_allow_wake);


static ssize_t bma2xx_get_fifo_wake(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", bma2xx->wuff);
}

static ssize_t bma2xx_set_fifo_wake(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bma2xx_data *bma2xx = attr_to_bma2xx(dev);
	unsigned long wuff;

	int error = kstrtoul(buf, 10, &wuff);
	if (error)
		return error;
	bma2xx->wuff = !!wuff;
	return count;
}
static DEVICE_ATTR(fifo_wake, S_IRUGO | S_IWUSR | S_IWGRP,
		bma2xx_get_fifo_wake, bma2xx_set_fifo_wake);


#ifdef BMA2XX_SW_CALIBRATION
static ssize_t bma2xx_get_offset(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d, %d, %d\n", bma2xx_offset[0],
				bma2xx_offset[1], bma2xx_offset[2]);
}

static ssize_t bma2xx_set_offset(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int x, y, z;
	int err = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *dd = i2c_get_clientdata(client);

	err = sscanf(buf, "%d %d %d", &x, &y, &z);
	if (err != 3) {
		pr_err("invalid parameter number: %d\n", err);
		return err;
	}
	mutex_lock(&dd->value_mutex);
	bma2xx_offset[0] = x;
	bma2xx_offset[1] = y;
	bma2xx_offset[2] = z;
	mutex_unlock(&dd->value_mutex);
	return count;
}
static DEVICE_ATTR(offset,  S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
			bma2xx_get_offset, bma2xx_set_offset);
#endif

static DEVICE_ATTR(range, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_range_show, bma2xx_range_store);
static DEVICE_ATTR(bandwidth, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_bandwidth_show, bma2xx_bandwidth_store);
static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_mode_show, bma2xx_mode_store);
static DEVICE_ATTR(value, S_IRUGO, bma2xx_value_show, NULL);
static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_delay_show, bma2xx_delay_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_enable_show, bma2xx_enable_store);
static DEVICE_ATTR(enable_int, S_IWUSR | S_IWGRP | S_IWOTH,
		   NULL, bma2xx_enable_int_store);
static DEVICE_ATTR(int_mode, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_int_mode_show, bma2xx_int_mode_store);
static DEVICE_ATTR(slope_duration, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_slope_duration_show, bma2xx_slope_duration_store);
static DEVICE_ATTR(slope_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_slope_threshold_show, bma2xx_slope_threshold_store);
static DEVICE_ATTR(high_g_duration, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_high_g_duration_show, bma2xx_high_g_duration_store);
static DEVICE_ATTR(high_g_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_high_g_threshold_show, bma2xx_high_g_threshold_store);
static DEVICE_ATTR(low_g_duration, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_low_g_duration_show, bma2xx_low_g_duration_store);
static DEVICE_ATTR(low_g_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_low_g_threshold_show, bma2xx_low_g_threshold_store);
static DEVICE_ATTR(tap_duration, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_tap_duration_show, bma2xx_tap_duration_store);
static DEVICE_ATTR(tap_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_tap_threshold_show, bma2xx_tap_threshold_store);
static DEVICE_ATTR(tap_quiet, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_tap_quiet_show, bma2xx_tap_quiet_store);
static DEVICE_ATTR(tap_shock, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_tap_shock_show, bma2xx_tap_shock_store);
static DEVICE_ATTR(tap_samp, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_tap_samp_show, bma2xx_tap_samp_store);
static DEVICE_ATTR(orient_mode, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_orient_mode_show, bma2xx_orient_mode_store);
static DEVICE_ATTR(orient_blocking, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_orient_blocking_show, bma2xx_orient_blocking_store);
static DEVICE_ATTR(orient_hyst, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_orient_hyst_show, bma2xx_orient_hyst_store);
static DEVICE_ATTR(orient_theta, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_orient_theta_show, bma2xx_orient_theta_store);
static DEVICE_ATTR(flat_theta, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_flat_theta_show, bma2xx_flat_theta_store);
static DEVICE_ATTR(flat_hold_time, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_flat_hold_time_show, bma2xx_flat_hold_time_store);
static DEVICE_ATTR(reg, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_register_show, bma2xx_register_store);
static DEVICE_ATTR(fast_calibration_x, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_fast_calibration_x_show,
		   bma2xx_fast_calibration_x_store);
static DEVICE_ATTR(fast_calibration_y, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_fast_calibration_y_show,
		   bma2xx_fast_calibration_y_store);
static DEVICE_ATTR(fast_calibration_z, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_fast_calibration_z_show,
		   bma2xx_fast_calibration_z_store);
static DEVICE_ATTR(selftest, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_selftest_show, bma2xx_selftest_store);
static DEVICE_ATTR(fflush, S_IWUSR | S_IWGRP,
		   NULL, bma2xx_fflush_store);
static DEVICE_ATTR(nomot_duration, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_nomot_duration_show, bma2xx_nomot_duration_store);
static DEVICE_ATTR(nomot_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   bma2xx_nomot_threshold_show, bma2xx_nomot_threshold_store);

static struct attribute *bma2xx_attributes[] = {
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_mode.attr,
	&dev_attr_value.attr,
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_enable_int.attr,
	&dev_attr_int_mode.attr,
	&dev_attr_slope_duration.attr,
	&dev_attr_slope_threshold.attr,
	&dev_attr_high_g_duration.attr,
	&dev_attr_high_g_threshold.attr,
	&dev_attr_low_g_duration.attr,
	&dev_attr_low_g_threshold.attr,
	&dev_attr_tap_threshold.attr,
	&dev_attr_tap_duration.attr,
	&dev_attr_tap_quiet.attr,
	&dev_attr_tap_shock.attr,
	&dev_attr_tap_samp.attr,
	&dev_attr_orient_mode.attr,
	&dev_attr_orient_blocking.attr,
	&dev_attr_orient_hyst.attr,
	&dev_attr_orient_theta.attr,
	&dev_attr_flat_theta.attr,
	&dev_attr_flat_hold_time.attr,
	&dev_attr_reg.attr,
	&dev_attr_fast_calibration_x.attr,
	&dev_attr_fast_calibration_y.attr,
	&dev_attr_fast_calibration_z.attr,
	&dev_attr_selftest.attr,
#ifdef BMA2XX_SW_CALIBRATION
	&dev_attr_offset.attr,
#endif
	&dev_attr_wake_allow.attr,
	&dev_attr_fflush.attr,
	&dev_attr_nomot_duration.attr,
	&dev_attr_nomot_threshold.attr,
	&dev_attr_fifo_wake.attr,
	NULL
};

static struct attribute_group bma2xx_attribute_group = {
	.attrs = bma2xx_attributes
};

unsigned char *orient[] = { "upward looking portrait upright",
	"upward looking portrait upside-down",
	"upward looking landscape left",
	"upward looking landscape right",
	"downward looking portrait upright",
	"downward looking portrait upside-down",
	"downward looking landscape left",
	"downward looking landscape right"
};

static void bma2xx_process_statusbit(struct bma2xx_data *bma2xx, int bit)
{
	unsigned char i;
	unsigned char first_value = 0;
	unsigned char sign_value = 0;
	struct input_dev *idev = bma2xx->wake_idev;

	switch (bit) {

	case 0x01:
		pr_info("Low G interrupt happened\n");
		input_report_rel(idev, LOW_G_INTERRUPT,
				 LOW_G_INTERRUPT_HAPPENED);
		break;
	case 0x02:
		for (i = 0; i < 3; i++) {
			bma2xx_get_HIGH_first(bma2xx->bma2xx_client, i,
					      &first_value);
			if (first_value == 1) {

				bma2xx_get_HIGH_sign(bma2xx->bma2xx_client,
						     &sign_value);

				if (sign_value == 1) {
					if (i == 0)
						input_report_rel(idev,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_X_NEGATIVE_HAPPENED);
					if (i == 1)
						input_report_rel(idev,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_Y_NEGATIVE_HAPPENED);
					if (i == 2)
						input_report_rel(idev,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_Z_NEGATIVE_HAPPENED);
				} else {
					if (i == 0)
						input_report_rel(idev,
						HIGH_G_INTERRUPT,
						HIGH_G_INTERRUPT_X_HAPPENED);
					if (i == 1)
						input_report_rel(idev,
						HIGH_G_INTERRUPT,
						HIGH_G_INTERRUPT_Y_HAPPENED);
					if (i == 2)
						input_report_rel(idev,
						HIGH_G_INTERRUPT,
						HIGH_G_INTERRUPT_Z_HAPPENED);

				}
			}

		}
		break;
	case 0x04:
		for (i = 0; i < 3; i++) {
			bma2xx_get_slope_first(bma2xx->bma2xx_client, i,
					       &first_value);
			if (first_value == 1) {

				bma2xx_get_slope_sign(bma2xx->bma2xx_client,
						      &sign_value);

				if (sign_value == 1) {
					if (i == 0)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[0]);
					else if (i == 1)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[1]);
					else if (i == 2)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[2]);
				} else {
					if (i == 0)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[0]);
					else if (i == 1)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[1]);
					else if (i == 2)
						input_report_rel(idev,
						SLOP_INTERRUPT,
						-bma2xx->axis_num_map[2]);

				}
			}

		}
		break;
	case 0x08:
		input_report_rel(idev, NOMOTION_INTERRUPT, bma2xx->nomotion ?
				NOMOTION_INTERRUPT_HAPPENED :
				SLOWMOTION_INTERRUPT_HAPPENED);
		break;
	case 0x10:
		input_report_rel(idev, DOUBLE_TAP_INTERRUPT,
				 DOUBLE_TAP_INTERRUPT_HAPPENED);
		break;
	case 0x20:
		input_report_rel(idev, SINGLE_TAP_INTERRUPT,
				 SINGLE_TAP_INTERRUPT_HAPPENED);
		break;
	case 0x40:
		bma2xx_get_orient_status(bma2xx->bma2xx_client, &first_value);
		if (first_value == 0)
			input_report_abs(idev, ORIENT_INTERRUPT,
			UPWARD_PORTRAIT_UP_INTERRUPT_HAPPENED);
		else if (first_value == 1)
			input_report_abs(idev, ORIENT_INTERRUPT,
			UPWARD_PORTRAIT_DOWN_INTERRUPT_HAPPENED);
		else if (first_value == 2)
			input_report_abs(idev, ORIENT_INTERRUPT,
			UPWARD_LANDSCAPE_LEFT_INTERRUPT_HAPPENED);
		else if (first_value == 3)
			input_report_abs(idev, ORIENT_INTERRUPT,
			UPWARD_LANDSCAPE_RIGHT_INTERRUPT_HAPPENED);
		else if (first_value == 4)
			input_report_abs(idev, ORIENT_INTERRUPT,
			DOWNWARD_PORTRAIT_UP_INTERRUPT_HAPPENED);
		else if (first_value == 5)
			input_report_abs(idev, ORIENT_INTERRUPT,
			DOWNWARD_PORTRAIT_DOWN_INTERRUPT_HAPPENED);
		else if (first_value == 6)
			input_report_abs(idev, ORIENT_INTERRUPT,
			DOWNWARD_LANDSCAPE_LEFT_INTERRUPT_HAPPENED);
		else if (first_value == 7)
			input_report_abs(idev, ORIENT_INTERRUPT,
			DOWNWARD_LANDSCAPE_RIGHT_INTERRUPT_HAPPENED);
		break;
	case 0x80:
		bma2xx_get_orient_flat_status(bma2xx->bma2xx_client,
					      &sign_value);
		if (sign_value == 1)
			input_report_abs(idev, FLAT_INTERRUPT,
					 FLAT_INTERRUPT_TURE_HAPPENED);
		else
			input_report_abs(idev, FLAT_INTERRUPT,
					 FLAT_INTERRUPT_FALSE_HAPPENED);
		break;
	default:
		break;
	}
	input_sync(idev);

}

static void bma2xx_irq_work_func(struct work_struct *work)
{
	struct bma2xx_data *bma2xx = container_of((struct work_struct *)work,
						  struct bma2xx_data, irq_work);

	unsigned char status;
	struct input_dev *idev = bma2xx->wake_idev;
	unsigned i;

	bma2xx_report_data(bma2xx, idev);
	bma2xx_get_interruptstatus1(bma2xx->bma2xx_client, &status);
	dev_dbg(&bma2xx->bma2xx_client->dev, "%s status 0x%02x\n", __func__,
			status);
	for (i = 0; i < 8; i++) {
		if (status & (1 << i))
			bma2xx_process_statusbit(bma2xx, 1 << i);
	}
}

static void bma2xx_fifo_work_func(struct work_struct *work)
{
	struct bma2xx_data *bma2xx = container_of(work, struct bma2xx_data,
			fifo_work);
	unsigned char data;
	int rc;

	while (true) {
		rc = bma2xx_smbus_read_byte(bma2xx->bma2xx_client,
				BMA2XXX_FIFOWM_S__REG, &data);
		if (rc)
			break;
		if (!BMA2XXX_GET_BITSLICE(data, BMA2XXX_FIFOWM_S))
			break;
		(void)bma2xx_fifo_read(bma2xx);
		data = BMA2XXX_SET_BITSLICE(0, BMA2XXX_INT_RESET_LATCHED, 1);
		rc = bma2xx_smbus_write_byte(bma2xx->bma2xx_client,
				BMA2XXX_INT_RESET_LATCHED__REG, &data);
		if (rc)
			break;
	}
}

static irqreturn_t bma2xx_irq_handler(int irq, void *handle)
{

	struct bma2xx_data *data = handle;
	unsigned long f;

	if (data == NULL)
		return IRQ_HANDLED;
	if (data->bma2xx_client == NULL)
		return IRQ_HANDLED;

	spin_lock_irqsave(&data->lock, f);
	if (data->state & DEV_SUSPENDED) {
		data->state |= IRQ1_PENDING;
	} else {
		data->state &= ~IRQ1_PENDING;
		schedule_work(&data->irq_work);
	}
	spin_unlock_irqrestore(&data->lock, f);
	return IRQ_HANDLED;
}

static irqreturn_t bma2xx_fifo_irq_handler(int irq, void *handle)
{
	struct bma2xx_data *data = handle;
	unsigned long f;

	if (data && data->bma2xx_client) {
		spin_lock_irqsave(&data->lock, f);
		if (data->state & DEV_SUSPENDED) {
			data->state |= IRQ2_PENDING;
		} else {
			data->state &= ~IRQ2_PENDING;
			schedule_work(&data->fifo_work);
		}
		spin_unlock_irqrestore(&data->lock, f);
	}
	return IRQ_HANDLED;
}

static int bma2xx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err = 0;
	unsigned char tempvalue;
	struct bma2xx_data *data;
	struct input_dev *dev;
	struct device_node *np;
	u32 val = 0;
#ifdef BMA2XX_SW_CALIBRATION
	bma2xx_offset[0] = 0;
	bma2xx_offset[1] = 0;
	bma2xx_offset[2] = 0;
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		printk(KERN_INFO "[bma2xxx]i2c_check_functionality error\n");
	data = kzalloc(sizeof(struct bma2xx_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	/* Turn on regulator (if it is configured in dt) */
	if (client->dev.of_node) {
		char *regname;
		int rc;

		np = client->dev.of_node;
		if (of_property_read_string(np,"regulator",
			(const char**)&regname)) {
			data->regulator = NULL;
			printk("Failed to get regulator from OF DT\n");
		} else {
			msleep(2000);
			data->regulator = regulator_get(NULL, regname);
			if ( data->regulator != NULL ) {
				rc = regulator_set_voltage(data->regulator, 2800000, 2800000);
				if (rc) {
					printk("bma2xx: Set_Voltage failed (rc=%i)\n",rc);
				} else {
					printk("bma2xx: Set_Voltage done\n");
				}
				rc = regulator_enable(data->regulator);
				if (rc) {
					printk("Failed to enable regulator\n");
				}
			}
		}
		printk("bma2xx: Regulator=%p\n",data->regulator);
	}

	/* read chip id */
	tempvalue = i2c_smbus_read_byte_data(client, BMA2XXX_CHIP_ID_REG);
	switch (tempvalue) {
	case ID_BMA250:
		dev_info(&client->dev, "bma250 chip found\n");
		data->r_shift = 6;
		break;
	case ID_BMA255:
		dev_info(&client->dev, "bma255 chip found\n");
		data->r_shift = 4;
		break;
	default:
		dev_info(&client->dev, "bma2xx chip id is %d\n", tempvalue);
		data->r_shift = 4;
		break;
	}
	i2c_set_clientdata(client, data);
	data->bma2xx_client = client;
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma2xx_set_bandwidth(client, BMA2XXX_BW_SET);
	bma2xx_set_range(client, BMA2XXX_RANGE_SET);

	bma2xx_set_Int_Mode(client, 1);	/*latch interrupt 250ms */
	/***************
	1,high g x
	2,high g y
	5,slope x
	8,single tap
	9,double tap
	10,orient
	11,flat
	bma2xx_set_Int_Enable(client, 8, 1);
	bma2xx_set_Int_Enable(client, 10, 1);
	bma2xx_set_Int_Enable(client, 11, 1);
	***************/

	if (client->dev.of_node) {
		const char *str;

		np = client->dev.of_node;
		if (of_property_read_u32(np, "gpio-irq-pin", &val))
			client->irq = -EINVAL;
		else
			client->irq = val;

		if (of_property_read_string(np, "orientation", &str))
			data->orientation =
				bma2xx_get_orientation("ORI_X_Y_Z");
		else
			data->orientation = bma2xx_get_orientation(str);

		if (of_property_read_u32(np, "gpio-data-irq-pin", &val))
			data->data_irq = -EINVAL;
		else
			data->data_irq = val;
	}
	if (client->irq != -EINVAL) {
		data->IRQ = gpio_to_irq(client->irq);
		err = request_irq(data->IRQ, bma2xx_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
			"	bma2xx", data);
		if (err)
			dev_err(&client->dev, "could not request irq\n");
	}
	if (data->data_irq != -EINVAL) {
		data->data_irq = gpio_to_irq(data->data_irq);
		err = request_irq(data->data_irq, bma2xx_fifo_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				"bma2xx_fifo", data);
		if (err)
			dev_err(&client->dev, "could not request fifo irq\n");
		else
			device_init_wakeup(&client->dev, 1);
	}
	INIT_WORK(&data->irq_work, bma2xx_irq_work_func);
	INIT_WORK(&data->fifo_work, bma2xx_fifo_work_func);
	INIT_WORK(&data->flush_work, bma2xx_fifo_flush_func);

	INIT_DELAYED_WORK(&data->work, bma2xx_work_func);
	atomic_set(&data->delay, BMA2XXX_MAX_DELAY);

	bma2xx_map_axes(data, &axis_num, data->axis_num_map);

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_MSC, MSC_SERIAL);
	input_set_capability(dev, EV_MSC, MSC_PULSELED);
	input_set_capability(dev, EV_MSC, MSC_GESTURE);
	input_set_events_per_packet(dev, BMA2X2_MAX_FIFO_LEVEL * 3);

	input_set_drvdata(dev, data);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		err = -ENOMEM;
		goto kfree_exit;
	}
	data->input = dev;
	data->input->dev.parent = &data->bma2xx_client->dev;

	data->wake_idev = input_allocate_device();
	if (!data->wake_idev) {
		err = -ENOMEM;
		goto kfree_exit_unregister;
	}

	data->wake_idev->name = "gesture_wake";
	data->wake_idev->id.bustype = BUS_I2C;

	input_set_capability(data->wake_idev, EV_REL, LOW_G_INTERRUPT);
	input_set_capability(data->wake_idev, EV_REL, HIGH_G_INTERRUPT);
	input_set_capability(data->wake_idev, EV_REL, SLOP_INTERRUPT);
	input_set_capability(data->wake_idev, EV_REL, DOUBLE_TAP_INTERRUPT);
	input_set_capability(data->wake_idev, EV_REL, SINGLE_TAP_INTERRUPT);
	input_set_capability(data->wake_idev, EV_ABS, ORIENT_INTERRUPT);
	input_set_capability(data->wake_idev, EV_ABS, FLAT_INTERRUPT);
	input_set_capability(data->wake_idev, EV_KEY, KEY_POWER);
	input_set_capability(data->wake_idev, EV_MSC, MSC_SERIAL);
	input_set_capability(data->wake_idev, EV_MSC, MSC_PULSELED);
	input_set_capability(data->wake_idev, EV_MSC, MSC_GESTURE);
	input_set_capability(data->wake_idev, EV_REL, NOMOTION_INTERRUPT);
	input_set_drvdata(data->wake_idev, data);

	err = input_register_device(data->wake_idev);
	if (err < 0) {
		input_free_device(data->wake_idev);
		goto kfree_exit_unregister;
	}
	data->wake_idev->dev.parent = &data->bma2xx_client->dev;


	err = sysfs_create_group(&dev->dev.kobj, &bma2xx_attribute_group);
	if (err < 0)
		goto error_sysfs;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = bma2xx_early_suspend;
	data->early_suspend.resume = bma2xx_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	spin_lock_init(&data->lock);
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma2xx_set_mode(data->bma2xx_client, BMA2XXX_MODE_SUSPEND);

	return 0;

error_sysfs:
	input_unregister_device(data->wake_idev);
kfree_exit_unregister:
	input_unregister_device(data->input);

kfree_exit:
	kfree(data);
exit:
	return err;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma2xx_early_suspend(struct early_suspend *h)
{
	struct bma2xx_data *data =
	    container_of(h, struct bma2xx_data, early_suspend);

	if (data->enable & BMA_FUNC(BMA_EN_WAKE_ON_EARLY_SUSPEND))
		(void)bma2xx_allow_wake(data, true);
}

static void bma2xx_late_resume(struct early_suspend *h)
{
	struct bma2xx_data *data =
	    container_of(h, struct bma2xx_data, early_suspend);

	if (data->enable & BMA_FUNC(BMA_EN_WAKE_ON_EARLY_SUSPEND))
		(void)bma2xx_allow_wake(data, false);
}
#endif

static int bma2xx_remove(struct i2c_client *client)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);

	bma2xx_set_enable(&client->dev, 0);
	device_init_wakeup(&client->dev, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	sysfs_remove_group(&data->input->dev.kobj, &bma2xx_attribute_group);
	input_unregister_device(data->wake_idev);
	input_unregister_device(data->input);
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int bma2xx_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);
	unsigned long f;

	spin_lock_irqsave(&data->lock, f);
	data->state |= DEV_SUSPENDED;
	spin_unlock_irqrestore(&data->lock, f);
	flush_work(&data->irq_work);
	flush_work(&data->fifo_work);

	mutex_lock(&data->enable_mutex);
	bma2xx_set_optimal_mode_locked(data, true);

	if (device_may_wakeup(&data->bma2xx_client->dev)) {
		if (WAKE_SET(data->enable) && WAKE_ENABLED(data->enable)) {
			int rc = irq_set_irq_wake(data->IRQ, 1);
			dev_dbg(&data->bma2xx_client->dev,
				"set IRQ %d wake = %d\n", data->IRQ, rc);
		}
		if (data->wuff) {
			int rc = irq_set_irq_wake(data->data_irq, 1);
			dev_dbg(&data->bma2xx_client->dev,
					"set fifo irq %d wake = %d\n",
					data->data_irq, rc);
		}
	}
	mutex_unlock(&data->enable_mutex);
	return 0;
}

static int bma2xx_resume(struct i2c_client *client)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);
	unsigned long f;

	mutex_lock(&data->enable_mutex);
	bma2xx_set_optimal_mode_locked(data, false);
	if (device_may_wakeup(&data->bma2xx_client->dev)) {
		if (WAKE_SET(data->enable) && WAKE_ENABLED(data->enable))
			(void)irq_set_irq_wake(data->IRQ, 0);
		if (data->wuff)
			(void)irq_set_irq_wake(data->data_irq, 0);
	}
	mutex_unlock(&data->enable_mutex);

	spin_lock_irqsave(&data->lock, f);
	data->state &= ~DEV_SUSPENDED;
	if (data->state & IRQ1_PENDING) {
		dev_dbg(&data->bma2xx_client->dev, "gesture irq pending\n");
		data->state &= ~IRQ1_PENDING;
		schedule_work(&data->irq_work);
	}
	if (data->state & IRQ2_PENDING) {
		dev_dbg(&data->bma2xx_client->dev, "fifo irq pending\n");
		data->state &= ~IRQ2_PENDING;
		schedule_work(&data->fifo_work);
	}
	spin_unlock_irqrestore(&data->lock, f);
	return 0;
}

#else

#define bma2xx_suspend		NULL
#define bma2xx_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id bma2xx_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bma2xx_id);

static const struct of_device_id bma2xx_of_match[] = {
	{.compatible = "bcm,bma2xx",},
	{},
}

MODULE_DEVICE_TABLE(of, bma2xx_of_match);

static struct i2c_driver bma2xx_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = SENSOR_NAME,
		   .of_match_table = bma2xx_of_match,
		   },
	.suspend = bma2xx_suspend,
	.resume = bma2xx_resume,
	.id_table = bma2xx_id,
	.probe = bma2xx_probe,
	.remove = bma2xx_remove,

};

static int __init BMA2XXX_init(void)
{
	return i2c_add_driver(&bma2xx_driver);
}

static void __exit BMA2XXX_exit(void)
{
	i2c_del_driver(&bma2xx_driver);
}

MODULE_AUTHOR("Albert Zhang <xu.zhang@bosch-sensortec.com>");
MODULE_DESCRIPTION("BMA2XXX accelerometer sensor driver");
MODULE_LICENSE("GPL");

module_init(BMA2XXX_init);
module_exit(BMA2XXX_exit);
