/*  Date: 2011/7/4 17:00:00
 *  Revision: 2.7
 */

/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
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

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define BMA2XX_SW_CALIBRATION 1

#define BMA2XXX_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define BMA2XXX_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

struct bma2xxacc {
	s16 x, y, z;
};

struct bma2xx_data {
	struct i2c_client *bma2xx_client;
	atomic_t delay;
	atomic_t enable;
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
	u32 orientation;
};

#ifdef BMA2XX_SW_CALIBRATION
static int bma2xx_offset[3];
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma2xx_early_suspend(struct early_suspend *h);
static void bma2xx_late_resume(struct early_suspend *h);
#endif

static int bma2xx_smbus_read_byte(struct i2c_client *client,
				  unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int bma2xx_smbus_write_byte(struct i2c_client *client,
				   unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma2xx_smbus_read_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
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

#ifdef BMA2XXX_ENABLE_INT1
static int bma2xx_set_int1_pad_sel(struct i2c_client *client,
				   unsigned char int1sel)
{
	int comres = 0;
	unsigned char data;
	unsigned char state;
	state = 0x01;

	switch (int1sel) {
	case 0:
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
	case 1:
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
	case 2:
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
	case 3:
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
	case 4:
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
	case 5:
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
	case 6:
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
	default:
		break;
	}

	return comres;
}
#endif /* BMA2XXX_ENABLE_INT1 */
#ifdef BMA2XXX_ENABLE_INT2
static int bma2xx_set_int2_pad_sel(struct i2c_client *client,
				   unsigned char int2sel)
{
	int comres = 0;
	unsigned char data;
	unsigned char state;
	state = 0x01;

	switch (int2sel) {
	case 0:
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
	case 1:
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
	case 2:
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
	case 3:
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
	case 4:
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
	case 5:
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
	case 6:
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
	default:
		break;
	}

	return comres;
}
#endif /* BMA2XXX_ENABLE_INT2 */

static int bma2xx_set_Int_Enable(struct i2c_client *client,
				 unsigned char InterruptType,
				 unsigned char value)
{
	int comres = 0;
	unsigned char data1, data2;

	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_INT_ENABLE1_REG, &data1);
	comres =
	    bma2xx_smbus_read_byte(client, BMA2XXX_INT_ENABLE2_REG, &data2);

	value = value & 1;
	switch (InterruptType) {
	case 0:
		/* Low G Interrupt  */
		data2 = BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_LOWG_INT, value);
		break;
	case 1:
		/* High G X Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_X_INT, value);
		break;
	case 2:
		/* High G Y Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_Y_INT, value);
		break;
	case 3:
		/* High G Z Interrupt */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_HIGHG_Z_INT, value);
		break;
	case 4:
		/* New Data Interrupt  */

		data2 =
		    BMA2XXX_SET_BITSLICE(data2, BMA2XXX_EN_NEW_DATA_INT, value);
		break;
	case 5:
		/* Slope X Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_X_INT, value);
		break;
	case 6:
		/* Slope Y Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_Y_INT, value);
		break;
	case 7:
		/* Slope Z Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SLOPE_Z_INT, value);
		break;
	case 8:
		/* Single Tap Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_SINGLE_TAP_INT,
					 value);
		break;
	case 9:
		/* Double Tap Interrupt */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_DOUBLE_TAP_INT,
					 value);
		break;
	case 10:
		/* Orient Interrupt  */

		data1 =
		    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_ORIENT_INT, value);
		break;
	case 11:
		/* Flat Interrupt */

		data1 = BMA2XXX_SET_BITSLICE(data1, BMA2XXX_EN_FLAT_INT, value);
		break;
	default:
		break;
	}
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_INT_ENABLE1_REG, &data1);
	comres =
	    bma2xx_smbus_write_byte(client, BMA2XXX_INT_ENABLE2_REG, &data2);

	return comres;
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
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL, 0);
			break;
		case 1:
			data1 =
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL, 5);
			break;
		case 2:
			data1 =
			    BMA2XXX_SET_BITSLICE(data1, BMA2XXX_RANGE_SEL, 8);
			break;
		case 3:
			data1 = BMA2XXX_SET_BITSLICE(data1,
						     BMA2XXX_RANGE_SEL, 12);
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

#if defined(BMA2XXX_ENABLE_INT1) || defined(BMA2XXX_ENABLE_INT2)
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
#endif /* defined(BMA2XXX_ENABLE_INT1)||defined(BMA2XXX_ENABLE_INT2) */
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
static int bma2xx_read_accel_x(struct i2c_client *client, short *a_x)
{
	int comres;
	unsigned char data[2];

	comres =
	    bma2xx_smbus_read_byte_block(client, BMA2XXX_ACC_X_LSB__REG, data,
					 2);
	*a_x =
	    BMA2XXX_GET_BITSLICE(data[0],
				 BMA2XXX_ACC_X_LSB) |
	    (BMA2XXX_GET_BITSLICE(data[1], BMA2XXX_ACC_X_MSB) <<
	     BMA2XXX_ACC_X_LSB__LEN);
	*a_x =
	    *a_x << (sizeof(short) * 8 -
		     (BMA2XXX_ACC_X_LSB__LEN + BMA2XXX_ACC_X_MSB__LEN));
	*a_x =
	    *a_x >> (sizeof(short) * 8 -
		     (BMA2XXX_ACC_X_LSB__LEN + BMA2XXX_ACC_X_MSB__LEN));

	return comres;
}
static int bma2xx_read_accel_y(struct i2c_client *client, short *a_y)
{
	int comres;
	unsigned char data[2];

	comres =
	    bma2xx_smbus_read_byte_block(client, BMA2XXX_ACC_Y_LSB__REG, data,
					 2);
	*a_y =
	    BMA2XXX_GET_BITSLICE(data[0],
				 BMA2XXX_ACC_Y_LSB) |
	    (BMA2XXX_GET_BITSLICE(data[1], BMA2XXX_ACC_Y_MSB) <<
	     BMA2XXX_ACC_Y_LSB__LEN);
	*a_y =
	    *a_y << (sizeof(short) * 8 -
		     (BMA2XXX_ACC_Y_LSB__LEN + BMA2XXX_ACC_Y_MSB__LEN));
	*a_y =
	    *a_y >> (sizeof(short) * 8 -
		     (BMA2XXX_ACC_Y_LSB__LEN + BMA2XXX_ACC_Y_MSB__LEN));

	return comres;
}

static int bma2xx_read_accel_z(struct i2c_client *client, short *a_z)
{
	int comres;
	unsigned char data[2];

	comres =
	    bma2xx_smbus_read_byte_block(client, BMA2XXX_ACC_Z_LSB__REG, data,
					 2);
	*a_z =
	    BMA2XXX_GET_BITSLICE(data[0],
				 BMA2XXX_ACC_Z_LSB) |
	    BMA2XXX_GET_BITSLICE(data[1], BMA2XXX_ACC_Z_MSB)
	    << BMA2XXX_ACC_Z_LSB__LEN;
	*a_z =
	    *a_z << (sizeof(short) * 8 -
		     (BMA2XXX_ACC_Z_LSB__LEN + BMA2XXX_ACC_Z_MSB__LEN));
	*a_z =
	    *a_z >> (sizeof(short) * 8 -
		     (BMA2XXX_ACC_Z_LSB__LEN + BMA2XXX_ACC_Z_MSB__LEN));

	return comres;
}

static int bma2xx_read_accel_xyz(struct i2c_client *client,
				 struct bma2xxacc *acc)
{
	int comres;
	unsigned char data[6];

	comres = bma2xx_smbus_read_byte_block(client,
					      BMA2XXX_ACC_X_LSB__REG, data, 6);

	acc->x = BMA2XXX_GET_BITSLICE(data[0], BMA2XXX_ACC_X_LSB)
	    | (BMA2XXX_GET_BITSLICE(data[1],
				    BMA2XXX_ACC_X_MSB) <<
	       BMA2XXX_ACC_X_LSB__LEN);
	acc->x =
	    acc->x << (sizeof(short) * 8 -
		       (BMA2XXX_ACC_X_LSB__LEN + BMA2XXX_ACC_X_MSB__LEN));
	acc->x =
	    acc->x >> (sizeof(short) * 8 -
		       (BMA2XXX_ACC_X_LSB__LEN + BMA2XXX_ACC_X_MSB__LEN));
	acc->y = BMA2XXX_GET_BITSLICE(data[2], BMA2XXX_ACC_Y_LSB)
	    | (BMA2XXX_GET_BITSLICE(data[3],
				    BMA2XXX_ACC_Y_MSB) <<
	       BMA2XXX_ACC_Y_LSB__LEN);
	acc->y =
	    acc->y << (sizeof(short) * 8 -
		       (BMA2XXX_ACC_Y_LSB__LEN + BMA2XXX_ACC_Y_MSB__LEN));
	acc->y =
	    acc->y >> (sizeof(short) * 8 -
		       (BMA2XXX_ACC_Y_LSB__LEN + BMA2XXX_ACC_Y_MSB__LEN));

	acc->z = BMA2XXX_GET_BITSLICE(data[4], BMA2XXX_ACC_Z_LSB)
	    | (BMA2XXX_GET_BITSLICE(data[5],
				    BMA2XXX_ACC_Z_MSB) <<
	       BMA2XXX_ACC_Z_LSB__LEN);
	acc->z =
	    acc->z << (sizeof(short) * 8 -
		       (BMA2XXX_ACC_Z_LSB__LEN + BMA2XXX_ACC_Z_MSB__LEN));
	acc->z =
	    acc->z >> (sizeof(short) * 8 -
		       (BMA2XXX_ACC_Z_LSB__LEN + BMA2XXX_ACC_Z_MSB__LEN));

	return comres;
}

static void bma2xx_work_func(struct work_struct *work)
{
	struct bma2xx_data *bma2xx = container_of((struct delayed_work *)work,
						  struct bma2xx_data, work);
	static struct bma2xxacc acc;
	int X, Y, Z;
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma2xx->delay));

	bma2xx_read_accel_xyz(bma2xx->bma2xx_client, &acc);
	switch (bma2xx->orientation) {
	case BMA_ORI_100_010_001:
		X = acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_010_100_001:
		X = acc.y;
		Y = acc.x;
		Z = acc.z;
		break;
	case BMA_ORI_f00_010_001:
		X = -acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_f00_0f0_001:
		X = -acc.x;
		Y = -acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_100_0f0_001:
		X = acc.x;
		Y = -acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_100_010_00f:
		X = acc.x;
		Y = acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_010_100_00f:
		X = acc.y;
		Y = acc.x;
		Z = -acc.z;
		break;
	case BMA_ORI_f00_010_00f:
		X = -acc.x;
		Y = acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_f00_0f0_00f:
		X = -acc.x;
		Y = -acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_100_0f0_00f:
		X = acc.x;
		Y = -acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_010_f00_00f:
		X = acc.y;
		Y = -acc.x;
		Z = -acc.z;
		break;
	case BMA_ORI_0f0_100_00f:
		X = -acc.y;
		Y = acc.x;
		Z = -acc.z;
		break;
	default:
		break;
	}
#ifdef BMA2XX_SW_CALIBRATION
	input_report_abs(bma2xx->input, ABS_X, X-bma2xx_offset[0]);
	input_report_abs(bma2xx->input, ABS_Y, Y-bma2xx_offset[1]);
	input_report_abs(bma2xx->input, ABS_Z, Z-bma2xx_offset[2]);
#else

	input_report_abs(bma2xx->input, ABS_X, X);
	input_report_abs(bma2xx->input, ABS_Y, Y);
	input_report_abs(bma2xx->input, ABS_Z, Z);
#endif
	input_sync(bma2xx->input);
	mutex_lock(&bma2xx->value_mutex);
	bma2xx->value = acc;
	mutex_unlock(&bma2xx->value_mutex);
	schedule_delayed_work(&bma2xx->work, delay);
}

static ssize_t bma2xx_register_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	int address, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	sscanf(buf, "%d%d", &address, &value);

	if (bma2xx_write_reg
	    (bma2xx->bma2xx_client, (unsigned char)address,
	     (unsigned char *)&value) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_register_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{

	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	size_t count = 0;
	u8 reg[0x3d];
	int i;

	for (i = 0; i < 0x3d; i++) {
		bma2xx_smbus_read_byte(bma2xx->bma2xx_client, i, reg + i);

		count += sprintf(&buf[count], "0x%x: %d\n", i, reg[i]);
	}
	return count;

}
static ssize_t bma2xx_range_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_range(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_range_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_range(bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_bandwidth_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_bandwidth(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_bandwidth_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_bandwidth(bma2xx->bma2xx_client,
				 (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_mode(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_mode_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_mode(bma2xx->bma2xx_client, (unsigned char)data) < 0)
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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma2xx->delay));

}

static ssize_t bma2xx_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma2xx->enable));

}

static void bma2xx_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&bma2xx->enable);

	mutex_lock(&bma2xx->enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			bma2xx_set_mode(bma2xx->bma2xx_client,
					BMA2XXX_MODE_NORMAL);
			schedule_delayed_work(&bma2xx->work,
					      msecs_to_jiffies(atomic_read
							       (&bma2xx->
								delay)));
			atomic_set(&bma2xx->enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			bma2xx_set_mode(bma2xx->bma2xx_client,
					BMA2XXX_MODE_SUSPEND);
			cancel_delayed_work_sync(&bma2xx->work);
			atomic_set(&bma2xx->enable, 0);
		}
	}
	mutex_unlock(&bma2xx->enable_mutex);

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
	if ((data == 0) || (data == 1))
		bma2xx_set_enable(dev, data);

	return count;
}

static ssize_t bma2xx_enable_int_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	int type, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	sscanf(buf, "%d%d", &type, &value);

	if (bma2xx_set_Int_Enable(bma2xx->bma2xx_client, type, value) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_int_mode_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_Int_Mode(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma2xx_int_mode_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	data = kstrtoul(buf, NULL, 10);

	if (bma2xx_set_Int_Mode(bma2xx->bma2xx_client, data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_slope_duration_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_slope_duration(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_slope_duration_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_slope_duration
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_slope_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_slope_threshold(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_slope_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_slope_threshold
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_high_g_duration_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_high_g_duration(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_high_g_duration_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_high_g_duration
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_high_g_threshold_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_high_g_threshold(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_high_g_threshold_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_high_g_threshold
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_low_g_duration_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_low_g_duration(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_low_g_duration_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_low_g_duration
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_low_g_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_low_g_threshold(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_low_g_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_low_g_threshold
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_threshold_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_tap_threshold(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_threshold_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma2xx_set_tap_threshold(bma2xx->bma2xx_client, (unsigned char)data)
	    < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_duration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_tap_duration(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_duration_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_duration(bma2xx->bma2xx_client, (unsigned char)data)
	    < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_tap_quiet_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_tap_quiet(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_quiet_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_quiet(bma2xx->bma2xx_client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_tap_shock_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_tap_shock(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_shock_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_shock(bma2xx->bma2xx_client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_tap_samp_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_tap_samp(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_tap_samp_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_tap_samp(bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_mode_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_orient_mode(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_mode_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_mode(bma2xx->bma2xx_client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_blocking_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_orient_blocking(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_blocking_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_blocking
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_orient_hyst_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_orient_hyst(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_hyst_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_orient_hyst(bma2xx->bma2xx_client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_orient_theta_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_theta_blocking(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_orient_theta_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_theta_blocking
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_flat_theta_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_theta_flat(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_flat_theta_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_theta_flat(bma2xx->bma2xx_client, (unsigned char)data) <
	    0)
		return -EINVAL;

	return count;
}
static ssize_t bma2xx_flat_hold_time_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_flat_hold_time(bma2xx->bma2xx_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma2xx_flat_hold_time_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_flat_hold_time
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma2xx_fast_calibration_x_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{

	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_offset_target_x(bma2xx->bma2xx_client, &data) < 0)
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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_x
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(bma2xx->bma2xx_client, 1) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(bma2xx->bma2xx_client, &tmp);

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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_offset_target_y(bma2xx->bma2xx_client, &data) < 0)
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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_y
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(bma2xx->bma2xx_client, 2) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(bma2xx->bma2xx_client, &tmp);

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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	if (bma2xx_get_offset_target_z(bma2xx->bma2xx_client, &data) < 0)
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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma2xx_set_offset_target_z
	    (bma2xx->bma2xx_client, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma2xx_set_cal_trigger(bma2xx->bma2xx_client, 3) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma2xx_get_cal_ready(bma2xx->bma2xx_client, &tmp);

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

	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

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
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2xx_data *bma2xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	if (data != 1)
		return -EINVAL;
	/* set to 2 G range */
	if (bma2xx_set_range(bma2xx->bma2xx_client, 0) < 0)
		return -EINVAL;

	bma2xx_write_reg(bma2xx->bma2xx_client, 0x32, &clear_value);

	bma2xx_set_selftest_st(bma2xx->bma2xx_client, 1);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 0);
	mdelay(10);
	bma2xx_read_accel_x(bma2xx->bma2xx_client, &value1);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 1);
	mdelay(10);
	bma2xx_read_accel_x(bma2xx->bma2xx_client, &value2);
	diff = value1 - value2;

	printk(KERN_INFO "diff x is %d,value1 is %d, value2 is %d\n",
		diff, value1, value2);

	if (abs(diff) < 204)
		result |= 1;

	bma2xx_set_selftest_st(bma2xx->bma2xx_client, 2);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 0);
	mdelay(10);
	bma2xx_read_accel_y(bma2xx->bma2xx_client, &value1);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 1);
	mdelay(10);
	bma2xx_read_accel_y(bma2xx->bma2xx_client, &value2);
	diff = value1 - value2;
	printk(KERN_INFO "diff y is %d,value1 is %d, value2 is %d\n",
		diff, value1, value2);
	if (abs(diff) < 204)
		result |= 2;

	bma2xx_set_selftest_st(bma2xx->bma2xx_client, 3);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 0);
	mdelay(10);
	bma2xx_read_accel_z(bma2xx->bma2xx_client, &value1);
	bma2xx_set_selftest_stn(bma2xx->bma2xx_client, 1);
	mdelay(10);
	bma2xx_read_accel_z(bma2xx->bma2xx_client, &value2);
	diff = value1 - value2;

	printk(KERN_INFO "diff z is %d,value1 is %d, value2 is %d\n", diff,
	       value1, value2);
	if (abs(diff) < 102)
		result |= 4;

	atomic_set(&bma2xx->selftest_result, (unsigned int)result);

	printk(KERN_INFO "self test finished\n");

	return count;
}
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
	struct input_dev *input_dev = dd->input;
	err = sscanf(buf, "%d %d %d", &x, &y, &z);
	if (err != 3) {
		pr_err("invalid parameter number: %d\n", err);
		return err;
	}
	mutex_lock(&input_dev->mutex);
	bma2xx_offset[0] = x;
	bma2xx_offset[1] = y;
	bma2xx_offset[2] = z;
	mutex_unlock(&input_dev->mutex);
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
	NULL
};

static struct attribute_group bma2xx_attribute_group = {
	.attrs = bma2xx_attributes
};

#if defined(BMA2XXX_ENABLE_INT1) || defined(BMA2XXX_ENABLE_INT2)
unsigned char *orient[] = { "upward looking portrait upright",
	"upward looking portrait upside-down",
	"upward looking landscape left",
	"upward looking landscape right",
	"downward looking portrait upright",
	"downward looking portrait upside-down",
	"downward looking landscape left",
	"downward looking landscape right"
};

static void bma2xx_irq_work_func(struct work_struct *work)
{
	struct bma2xx_data *bma2xx = container_of((struct work_struct *)work,
						  struct bma2xx_data, irq_work);

	unsigned char status = 0;
	unsigned char i;
	unsigned char first_value = 0;
	unsigned char sign_value = 0;

	bma2xx_get_interruptstatus1(bma2xx->bma2xx_client, &status);

	switch (status) {

	case 0x01:
		pr_info("Low G interrupt happened\n");
		input_report_rel(bma2xx->input, LOW_G_INTERRUPT,
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
						input_report_rel(bma2xx->input,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_X_NEGATIVE_HAPPENED);
					if (i == 1)
						input_report_rel(bma2xx->input,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_Y_NEGATIVE_HAPPENED);
					if (i == 2)
						input_report_rel(bma2xx->input,
					HIGH_G_INTERRUPT,
					HIGH_G_INTERRUPT_Z_NEGATIVE_HAPPENED);
				} else {
					if (i == 0)
						input_report_rel(bma2xx->input,
						HIGH_G_INTERRUPT,
						HIGH_G_INTERRUPT_X_HAPPENED);
					if (i == 1)
						input_report_rel(bma2xx->input,
						HIGH_G_INTERRUPT,
						HIGH_G_INTERRUPT_Y_HAPPENED);
					if (i == 2)
						input_report_rel(bma2xx->input,
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
						input_report_rel(bma2xx->input,
					SLOP_INTERRUPT,
					SLOPE_INTERRUPT_X_NEGATIVE_HAPPENED);
					else if (i == 1)
						input_report_rel(bma2xx->input,
					SLOP_INTERRUPT,
					SLOPE_INTERRUPT_Y_NEGATIVE_HAPPENED);
					else if (i == 2)
						input_report_rel(bma2xx->input,
					SLOP_INTERRUPT,
					SLOPE_INTERRUPT_Z_NEGATIVE_HAPPENED);
				} else {
					if (i == 0)
						input_report_rel(bma2xx->input,
						SLOP_INTERRUPT,
						SLOPE_INTERRUPT_X_HAPPENED);
					else if (i == 1)
						input_report_rel(bma2xx->input,
						SLOP_INTERRUPT,
						SLOPE_INTERRUPT_Y_HAPPENED);
					else if (i == 2)
						input_report_rel(bma2xx->input,
						SLOP_INTERRUPT,
						SLOPE_INTERRUPT_Z_HAPPENED);

				}
			}

		}
		break;

	case 0x10:
		input_report_rel(bma2xx->input, DOUBLE_TAP_INTERRUPT,
				 DOUBLE_TAP_INTERRUPT_HAPPENED);
		break;
	case 0x20:
		input_report_rel(bma2xx->input, SINGLE_TAP_INTERRUPT,
				 SINGLE_TAP_INTERRUPT_HAPPENED);
		break;
	case 0x40:
		bma2xx_get_orient_status(bma2xx->bma2xx_client, &first_value);
		if (first_value == 0)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			UPWARD_PORTRAIT_UP_INTERRUPT_HAPPENED);
		else if (first_value == 1)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			UPWARD_PORTRAIT_DOWN_INTERRUPT_HAPPENED);
		else if (first_value == 2)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			UPWARD_LANDSCAPE_LEFT_INTERRUPT_HAPPENED);
		else if (first_value == 3)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			UPWARD_LANDSCAPE_RIGHT_INTERRUPT_HAPPENED);
		else if (first_value == 4)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			DOWNWARD_PORTRAIT_UP_INTERRUPT_HAPPENED);
		else if (first_value == 5)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			DOWNWARD_PORTRAIT_DOWN_INTERRUPT_HAPPENED);
		else if (first_value == 6)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			DOWNWARD_LANDSCAPE_LEFT_INTERRUPT_HAPPENED);
		else if (first_value == 7)
			input_report_abs(bma2xx->input, ORIENT_INTERRUPT,
			DOWNWARD_LANDSCAPE_RIGHT_INTERRUPT_HAPPENED);
		break;
	case 0x80:
		bma2xx_get_orient_flat_status(bma2xx->bma2xx_client,
					      &sign_value);
		if (sign_value == 1)
			input_report_abs(bma2xx->input, FLAT_INTERRUPT,
					 FLAT_INTERRUPT_TURE_HAPPENED);
		else
			input_report_abs(bma2xx->input, FLAT_INTERRUPT,
					 FLAT_INTERRUPT_FALSE_HAPPENED);
		}
		break;
	default:
		break;
	}

}

static irqreturn_t bma2xx_irq_handler(int irq, void *handle)
{

	struct bma2xx_data *data = handle;

	if (data == NULL)
		return IRQ_HANDLED;
	if (data->bma2xx_client == NULL)
		return IRQ_HANDLED;

	schedule_work(&data->irq_work);

	return IRQ_HANDLED;

}
#endif /* defined(BMA2XXX_ENABLE_INT1)||defined(BMA2XXX_ENABLE_INT2) */


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
	/* read chip id */
	tempvalue = i2c_smbus_read_byte_data(client, BMA2XXX_CHIP_ID_REG);

	pr_info("bma2xx chip id is %d\n", tempvalue);
	i2c_set_clientdata(client, data);
	data->bma2xx_client = client;
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma2xx_set_bandwidth(client, BMA2XXX_BW_SET);
	bma2xx_set_range(client, BMA2XXX_RANGE_SET);

#if defined(BMA2XXX_ENABLE_INT1) || defined(BMA2XXX_ENABLE_INT2)
	bma2xx_set_Int_Mode(client, 1);	/*latch interrupt 250ms */
#endif
	/***************
	1,high g x
	2,high g y
	5,slope x
	8,single tap
	9,double tap
	10,orient
	11,flat
	***************/
	bma2xx_set_Int_Enable(client, 8, 1);
	bma2xx_set_Int_Enable(client, 10, 1);
	bma2xx_set_Int_Enable(client, 11, 1);

#ifdef BMA2XXX_ENABLE_INT1
	/* maps interrupt to INT1 pin */
	bma2xx_set_int1_pad_sel(client, PAD_LOWG);
	bma2xx_set_int1_pad_sel(client, PAD_HIGHG);
	bma2xx_set_int1_pad_sel(client, PAD_SLOP);
	bma2xx_set_int1_pad_sel(client, PAD_DOUBLE_TAP);
	bma2xx_set_int1_pad_sel(client, PAD_SINGLE_TAP);
	bma2xx_set_int1_pad_sel(client, PAD_ORIENT);
	bma2xx_set_int1_pad_sel(client, PAD_FLAT);
#endif

#ifdef BMA2XXX_ENABLE_INT2
	/* maps interrupt to INT2 pin */
	bma2xx_set_int2_pad_sel(client, PAD_LOWG);
	bma2xx_set_int2_pad_sel(client, PAD_HIGHG);
	bma2xx_set_int2_pad_sel(client, PAD_SLOP);
	bma2xx_set_int2_pad_sel(client, PAD_DOUBLE_TAP);
	bma2xx_set_int2_pad_sel(client, PAD_SINGLE_TAP);
	bma2xx_set_int2_pad_sel(client, PAD_ORIENT);
	bma2xx_set_int2_pad_sel(client, PAD_FLAT);
#endif
	if (client->dev.of_node) {
		np = client->dev.of_node;
		if (of_property_read_u32(np, "gpio-irq-pin", &val))
			goto err_read;
		client->irq = val;
		if (of_property_read_u32(np, "orientation", &val))
			data->orientation = 7;
		else
			data->orientation = val;
		client->irq = val;
	}
#if defined(BMA2XXX_ENABLE_INT1) || defined(BMA2XXX_ENABLE_INT2)
	data->IRQ = gpio_to_irq(client->irq);
	err =
	    request_irq(data->IRQ, bma2xx_irq_handler, IRQF_TRIGGER_RISING,
			"bma2xx", data);
	if (err)
		printk(KERN_ERR "could not request irq\n");
	INIT_WORK(&data->irq_work, bma2xx_irq_work_func);
#endif

	INIT_DELAYED_WORK(&data->work, bma2xx_work_func);
	atomic_set(&data->delay, BMA2XXX_MAX_DELAY);
	atomic_set(&data->enable, 0);

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_REL, LOW_G_INTERRUPT);
	input_set_capability(dev, EV_REL, HIGH_G_INTERRUPT);
	input_set_capability(dev, EV_REL, SLOP_INTERRUPT);
	input_set_capability(dev, EV_REL, DOUBLE_TAP_INTERRUPT);
	input_set_capability(dev, EV_REL, SINGLE_TAP_INTERRUPT);
	input_set_capability(dev, EV_ABS, ORIENT_INTERRUPT);
	input_set_capability(dev, EV_ABS, FLAT_INTERRUPT);
	input_set_abs_params(dev, ABS_X, ABSMIN, ABSMAX, 0, 0);
	input_set_abs_params(dev, ABS_Y, ABSMIN, ABSMAX, 0, 0);
	input_set_abs_params(dev, ABS_Z, ABSMIN, ABSMAX, 0, 0);
	set_bit(EV_REL, dev->evbit);
	set_bit(ABS_X, dev->relbit);
	set_bit(ABS_Y, dev->relbit);
	set_bit(ABS_Z, dev->relbit);

	input_set_drvdata(dev, data);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		goto kfree_exit;
	}

	data->input = dev;
	data->input->dev.parent = &data->bma2xx_client->dev;

	err = sysfs_create_group(&client->dev.kobj, &bma2xx_attribute_group);
	if (err < 0)
		goto error_sysfs;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = bma2xx_early_suspend;
	data->early_suspend.resume = bma2xx_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);

	return 0;
err_read:
error_sysfs:
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

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma2xx_set_mode(data->bma2xx_client, BMA2XXX_MODE_SUSPEND);
		cancel_delayed_work_sync(&data->work);
	}
	mutex_unlock(&data->enable_mutex);
}

static void bma2xx_late_resume(struct early_suspend *h)
{
	struct bma2xx_data *data =
	    container_of(h, struct bma2xx_data, early_suspend);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma2xx_set_mode(data->bma2xx_client, BMA2XXX_MODE_NORMAL);
		schedule_delayed_work(&data->work,
				      msecs_to_jiffies(atomic_read
						       (&data->delay)));
	}
	mutex_unlock(&data->enable_mutex);
}
#endif

static int __devexit bma2xx_remove(struct i2c_client *client)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);

	bma2xx_set_enable(&client->dev, 0);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	sysfs_remove_group(&data->input->dev.kobj, &bma2xx_attribute_group);
	input_unregister_device(data->input);
	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int bma2xx_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma2xx_set_mode(data->bma2xx_client, BMA2XXX_MODE_SUSPEND);
		cancel_delayed_work_sync(&data->work);
	}
	mutex_unlock(&data->enable_mutex);

	return 0;
}

static int bma2xx_resume(struct i2c_client *client)
{
	struct bma2xx_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma2xx_set_mode(data->bma2xx_client, BMA2XXX_MODE_NORMAL);
		schedule_delayed_work(&data->work,
				      msecs_to_jiffies(atomic_read
						       (&data->delay)));
	}
	mutex_unlock(&data->enable_mutex);

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
	.remove = __devexit_p(bma2xx_remove),

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
