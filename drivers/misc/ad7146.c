/**
* \file ad7146.c
* This file is the core driver part of AD7146 with Event interface.
 It also has routines for interrupt handling for
* Sensor active and Convertion complete interrupt modes,
* suspend, resume, initialization routines etc.
* AD7146 Controller Driver
* Copyright 2013 Analog Devices Inc.
* Copyright (C) 2013 Sony Mobile Communications Inc.
* Licensed under the GPL version 2 or later.
*/

#include <asm/system.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/ad7146.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/switch.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

typedef int (*ad7146_read_t)(struct device *, unsigned short, unsigned short *);
typedef int (*ad7146_write_t)(struct device *, unsigned short, unsigned short);

/* Power control Register */
#define PWR_CTRL_REG				0x0000
#define PWR_MODE_FULL_PWR			0x0000
#define PWR_MODE_LOWPWR				0x0002
#define PWR_MODE_SHUTDOWN			0x0003
#define PWR_MODE_MASK				0xFFFC

/* Calibration and Control Register */
#define STG_CAL_EN_REG				0x0001
#define STG0_CAL_EN					0x0001
#define STG1_CAL_EN					0x0002
#define ALL_STG_CAL_EN_OFF			0x0000

/* Device Control Registers */
#define AMB_COMP_CTRL0_REG			0x0002
#define AMB_COMP_CTRL1_REG			0x0003
#define AMB_COMP_CTRL2_REG			0x0004
#define AMB_COMP_FORCE_CAL			0x4000

/* Interrupt Enable registers */
#define STG_LOW_INT_EN_REG			0x0005
#define STG_HIGH_INT_EN_REG			0x0006
#define STG_COM_INT_EN_REG			0x0007
#define STG0_INT_EN					0x0001
#define STG1_INT_EN					0x0002
#define ALL_STG_INT_EN_OFF			0x0000

/* Interrupt Status registers */
#define STG_LOW_INT_STA_REG			0x0008
#define STG_HIGH_INT_STA_REG		0x0009
#define STG_COM_INT_STA_REG			0x000A
#define STG0_INT_STA				0x0001
#define STG1_INT_STA				0x0002

/* Register address of Stage 1 CDC result */
#define CDC_RESULT_S0_REG			0x000B
#define STG0_CDC_RESULT_S0_REG		0x000B
#define STG1_CDC_RESULT_S0_REG		0x000C

/* Device ID Register Address */
#define AD7146_PARTID_REG			0x0017

/* Register address of MOD_FREQ_CTL_REG register */
#define MOD_FREQ_CTL_REG			0x0045

/* lowest/Highest write accessible register */
#define LOWER_WR_REG				0x0008
#define HIGHER_WR_REG				0x007f

/* STAGE0 Configuration register */
#define STG0_CONNECTION_6_0_REG		0x0080
#define STG0_CONNECTION_12_7_REG	0x0081
#define STG0_AFE_OFFSET_REG			0x0082
#define STG0_SENSITIVITY_REG		0x0083
#define STG0_OFFSET_LOW_REG			0x0084
#define STG0_OFFSET_HIGH_REG		0x0085
#define STG0_OFFSET_HIGH_CLAMP_REG	0x0086
#define STG0_OFFSET_LOW_CLAMP_REG	0x0087

/* STAGE1 Configuration register */
#define STG1_CONNECTION_6_0_REG		0x0088
#define STG1_CONNECTION_12_7_REG	0x0089
#define STG1_AFE_OFFSET_REG			0x008A
#define STG1_SENSITIVITY_REG		0x008B
#define STG1_OFFSET_LOW_REG			0x008C
#define STG1_OFFSET_HIGH_REG		0x008D
#define STG1_OFFSET_HIGH_CLAMP_REG	0x008E
#define STG1_OFFSET_LOW_CLAMP_REG	0x008F

/* STAGEx Common Configuration */
#define STGx_CONNECT_OFF1			0x3FFF
#define STGx_CONNECT_OFF2			0x0FFF

/* STAGEx Configuration register */
#define STG2_CONNECTION_6_0_REG		0x0090
#define STG2_CONNECTION_12_7_REG	0x0091
#define STG3_CONNECTION_6_0_REG		0x0098
#define STG3_CONNECTION_12_7_REG	0x0099
#define STG4_CONNECTION_6_0_REG		0x00A0
#define STG4_CONNECTION_12_7_REG	0x00A1
#define STG5_CONNECTION_6_0_REG		0x00A8
#define STG5_CONNECTION_12_7_REG	0x00A9
#define STG6_CONNECTION_6_0_REG		0x00B0
#define STG6_CONNECTION_12_7_REG	0x00B1
#define STG7_CONNECTION_6_0_REG		0x00B8
#define STG7_CONNECTION_12_7_REG	0x00B9
#define STG8_CONNECTION_6_0_REG		0x00C0
#define STG8_CONNECTION_12_7_REG	0x00C1
#define STG9_CONNECTION_6_0_REG		0x00C8
#define STG9_CONNECTION_12_7_REG	0x00C9
#define STG10_CONNECTION_6_0_REG	0x00D0
#define STG10_CONNECTION_12_7_REG	0x00D1
#define STG11_CONNECTION_6_0_REG	0x00D8
#define STG11_CONNECTION_12_7_REG	0x00D9

/* STAGE0 slow FIFO ambient value Register Address */
#define STG0_SF_AMBIENT				0x00F1

/* STAGEx Threshold Register Adress */
#define STG0_HIGH_THRESHOLD			0x00FA
#define STG0_LOW_THRESHOLD			0x0101
#define STG1_HIGH_THRESHOLD			0x011E
#define STG1_LOW_THRESHOLD			0x0125

/* Defines */
#define DRIVER_NAME				"ad7146"
#define SWITCH_NAME_DEF			"ad7146"
#define SWITCH_NAME_1			"ad7146_1"
#define SWITCH_NAME_2			"ad7146_2"
#define SWITCH_UPDATA_NAME		"ad7146_updata"
#define AD7146_PRODUCT_ID		0x7146
#define AD7146_PARTID			0x1490
#define REGULATOR_MAX_VOLTAGE	2850000
#define REGULATOR_UA_LOAD		15000
#define REGULATOR_ON_DELAY		3
#define STAGE_NUM				12
#define DISABLE_AD7146			0
#define ENABLE_AD7146			1
#define MIN_FORCED_CAL_SLEEP	20
#define MAX_FORCED_CAL_SLEEP	50
#define DECIMAL_BASE			10
#define I2C_READ_LEN			2
#define I2C_WRITE_LEN			4
#define I2C_MSG_LEN				1
#define DEFAULT_DAC_MID_VALUE	0x2710
#define DAC_DIFF_VAL			0xA00
#define MIN_DAC_MID_VAL			0x2000
#define MAX_DAC_MID_VAL			0xA000
#define FT_CALIB_DELAY			10
#define DECI_MAX				0x3
#define DECIMATION_MASK			0x300
#define CDC_ZERO_VALUE			0x0
#define FULL_SCALE_VALUE		0xFFFF
#define FT_CALIB_T_OUT			8000
#define DEFAULT_MOD_FREQ_VALUE	0xD00
#define MAX_AFE_VALUE			0x3F
#define POS_SWAP_MASK			0x8000
#define NEG_SWAP_MASK			0x080
#define MIN_AFE_STEP			0x06
#define CAL_ROW_SIZE			0x2
#define ACT_SYSFS				1
#define BYTE_SIZE				8
#define LOW_O_STAGE_SFT			14
#define STG_CON_MASK			0x3FFF
#define STG_CONN_CHK_CNT		14
#define STG_CONN_CNT			2
#define STG_JUMP_CNT			0x24
#define SKIP_UNUSED_EN			0xF000
#define INT_EN_MASK				0x0FFF
#define GPIO_INT_EN_MASK		0x1000
#define POS_STG_CON_MASK		0x2
#define LOW_NIBBLE				0x0F
#define HW_DET_MASK				0xFFF0
#define ADD_FACTOR				0x1
#define I2C_ACCESS_RETRY_MAX	5
#define I2C_ACCESS_RETRY_WAIT	10
#define SW_INIT_STATUS			2
#define STAGE_REG_NUM			8
#define PAD_NUM_MAX				2
#define TEMP_BUFER_MAX_LEN		100
#define PAD_INIT_NUM			0
#define PAD_NUMBER_MIN			1
#define PAD_NUMBER_MAX			12
#define STG0_EN_FLG				0x0001
#define STG1_EN_FLG				0x0002
#define STGX_ALL_OFF			0x0000
#define PAD1					0
#define PAD2					1
#define GET_DATA_REG_MAX		3
#define AMB_COMP_CTRL_NUM_MAX	3
#define SWITCH_WORK_NO_JITTER	0
#define SWITCH_WORK_JITTER		50
#define SWITCH_WORK_FORCE_CAL_JITTER	50
#define RET_ZERO_VAL			0

#define AD7146_BIT_MASK(s)		(1 << (s))
#define AD7146_FORCED_CAL_MASK	AD7146_BIT_MASK(14)
#define OPEN_AIR_HIGH_VALUE		(DEFAULT_DAC_MID_VALUE + DAC_DIFF_VAL)
#define OPEN_AIR_LOW_VALUE		(DEFAULT_DAC_MID_VALUE - DAC_DIFF_VAL)
#define GET_AFE_REG(cur_stg)	((stg_conn_register[((cur_stg) * 2)]) + 2)
#define GET_POS_AFE(cur_afe)	(((cur_afe) & (0x3f00))>>8)
#define GET_NEG_AFE(cur_afe)	((cur_afe) & (0x3f))
#define SET_POS_AFE(cur_afe, pos_afe)	(((cur_afe) & 0xC0FF) | ((pos_afe)<<8))
#define SET_NEG_AFE(cur_afe, neg_afe)	(((cur_afe) & (0xFFC0)) | (neg_afe))
#define POWER_SLEEP_TIME(x)		((((x & 0x0C) >> 2) + 1) * 200)
#define HYS(S, T, P)		((T) - ((((T)-(S)) * P)/100))
#define HYS_POS(S, T, P)	((T) + ((((T)-(S)) * P) / (100 - P)))
#define GET_AMB_REG(x)		(((x) * STG_JUMP_CNT) + STG0_SF_AMBIENT)
#define GET_HT_TH_REG(x)	(((x) * STG_JUMP_CNT) + STG0_HIGH_THRESHOLD)

enum ad7146_device_state {
	IDLE,
	ACTIVE
};

enum dac_status {
	INIT_DAC,
	PENDING,
	DONE_SUCCESS,
	DONE_FAILED
};

enum ad7146_stage_num {
	STG_ZERO,
	STG_ONE
};

enum ad7146_sens_state  {
	AD7146_SENS_NOT_DET,
	AD7146_SENS_DET
};

enum ad7146_i2c_err_state {
	AD7146_I2C_RW_NO_ERR,
	AD7146_I2C_RW_ERR
};

enum ad7146_data_point {
	AD7146_NOT_SET,
	AD7146_LOW,
	AD7146_CORRECT,
	AD7146_HIGH,
};

enum ad7146_reg_amb_comp_ctrlx {
	AMB_COMP_CTRL_NUM0,
	AMB_COMP_CTRL_NUM1,
	AMB_COMP_CTRL_NUM2,
};

enum ad7146_register_setup_order {
	STG0_CONNECTION_6_0,
	STG0_CONNECTION_12_7,
	STG1_CONNECTION_6_0,
	STG1_CONNECTION_12_7,
	MOD_FREQ_CTL,
	AMB_COMP_CTRL0,
	AMB_COMP_CTRL1,
	AMB_COMP_CTRL2,
	STG_LOW_INT_EN,
	STG_HIGH_INT_EN,
	STG_COM_INT_EN,
	PWR_CONTROL,
	STG_CAL_EN,
	AMB_COMP_CTRL0_2,
	AD7146_REG_MAX,
};

/* cdc data, threshold register address table */
static unsigned short data_reg_addr[PAD_NUM_MAX][GET_DATA_REG_MAX] = {
	{STG0_CDC_RESULT_S0_REG, STG0_HIGH_THRESHOLD, STG0_LOW_THRESHOLD},
	{STG1_CDC_RESULT_S0_REG, STG1_HIGH_THRESHOLD, STG1_LOW_THRESHOLD},
};

/* stage register table */
const unsigned short stg_conn_register[STAGE_NUM * STG_CONN_CNT] = {
	STG0_CONNECTION_6_0_REG,
	STG0_CONNECTION_12_7_REG,
	STG1_CONNECTION_6_0_REG,
	STG1_CONNECTION_12_7_REG,
	STG2_CONNECTION_6_0_REG,
	STG2_CONNECTION_12_7_REG,
	STG3_CONNECTION_6_0_REG,
	STG3_CONNECTION_12_7_REG,
	STG4_CONNECTION_6_0_REG,
	STG4_CONNECTION_12_7_REG,
	STG5_CONNECTION_6_0_REG,
	STG5_CONNECTION_12_7_REG,
	STG6_CONNECTION_6_0_REG,
	STG6_CONNECTION_12_7_REG,
	STG7_CONNECTION_6_0_REG,
	STG7_CONNECTION_12_7_REG,
	STG8_CONNECTION_6_0_REG,
	STG8_CONNECTION_12_7_REG,
	STG9_CONNECTION_6_0_REG,
	STG9_CONNECTION_12_7_REG,
	STG10_CONNECTION_6_0_REG,
	STG10_CONNECTION_12_7_REG,
	STG11_CONNECTION_6_0_REG,
	STG11_CONNECTION_12_7_REG,
};

struct ad7146_shadow_reg {
	unsigned short addr;
	unsigned short data;
};

static struct ad7146_shadow_reg shadow_reg[] = {
	/* STAGE0 Configuration */
	{STG0_CONNECTION_6_0_REG,	0x3FBF},
	{STG0_CONNECTION_12_7_REG,	0x1FDF},

	/* STAGE1 Configuration */
	{STG1_CONNECTION_6_0_REG,	0x37FF},
	{STG1_CONNECTION_12_7_REG,	0x1BFF},

	/* Modulator Control */
	{MOD_FREQ_CTL_REG,		0x0D01},

	/* Setup  */
	{AMB_COMP_CTRL0_REG,	0x30FB}, /* FORCE_CAL disabled */
	{AMB_COMP_CTRL1_REG,	0x01FF},
	{AMB_COMP_CTRL2_REG,	0x05FF},

	/* Interrupt */
	{STG_LOW_INT_EN_REG,	0x0000}, /* All INT disabled */
	{STG_HIGH_INT_EN_REG,	0x0000}, /* All INT disabled */
	{STG_COM_INT_EN_REG,	0x0000},

	/* Setup  */
	{PWR_CTRL_REG,			0xC01E}, /* PWR_MODE_LOWPOWER */

	/* Calibration */
	{STG_CAL_EN_REG,		0x0003}, /* All CAL_EN disabled */

	/* Setup  */
	{AMB_COMP_CTRL0_REG,	0x30FB}, /* FORCE_CAL disabled */
};

struct ad7146_product_stgx_data {
	unsigned int stgx_conn_6_0;
	unsigned int stgx_conn_12_7;
	unsigned int stgx_afe_offset;
	unsigned int stgx_sensitivity;
	unsigned int stgx_offset_low;
	unsigned int stgx_offset_high;
	unsigned int stgx_offset_high_clamp;
	unsigned int stgx_offset_low_clamp;
	unsigned int stgx_hysterisis;
};

static struct ad7146_product_stgx_data default_stgx_data = {
	STGx_CONNECT_OFF1, STGx_CONNECT_OFF2,
	RET_ZERO_VAL, RET_ZERO_VAL,
	RET_ZERO_VAL, RET_ZERO_VAL,
	RET_ZERO_VAL, RET_ZERO_VAL,
	RET_ZERO_VAL
};

struct ad7146_product_data {
	unsigned int amb_comp_ctrlx[AMB_COMP_CTRL_NUM_MAX];
	unsigned int mod_freq_ctrl;
	struct ad7146_product_stgx_data stgx_data[PAD_NUM_MAX];
};

struct ad7146_driver_data {
	enum ad7146_device_state state;
	unsigned char index;
};

struct cap_sensor_dev {
	const char *name;
	struct device *dev;
	void *data;
};

struct ad7146_chip {
	unsigned short high_status;
	unsigned short low_status;
	unsigned short sensor_int_enable;
	unsigned short power_reg_value;
	unsigned short no_of_stages;
	unsigned short no_of_sens_stages;
	unsigned short stg_calib_enable;
	unsigned short stg_connection;
	unsigned short open_air_low;
	unsigned short open_air_high;
	unsigned char last_stg_en;
	struct ad7146_driver_data *sw;
	int irq;
	struct device *dev;
	struct delayed_work work;
	struct work_struct calib_work;
	struct work_struct resume_work;
	ad7146_read_t read;
	ad7146_write_t write;
	unsigned product;
	struct mutex mutex;
	struct mutex mutex_shadow;
	unsigned version;
	enum dac_status curr_dac_status;
	unsigned short *dac_cal_buffer;
	struct switch_dev sw_state;
	struct switch_dev sw_stg0;
	struct switch_dev sw_stg1;
	struct cap_sensor_dev cap_dev;
	unsigned short sw_updata;
	const char *vdd_supply_name;
	struct regulator *vreg_prox_vdd;
	int irq_gpio;
	unsigned short i2c_err_flag;
	unsigned short pad_enable_state;
	int current_pad_no;
	struct ad7146_product_data product_data;
	unsigned short save_data_point[PAD_NUM_MAX];
	unsigned short fc_flag;
	unsigned short keep_detect_flag;
	unsigned short sv_threshold[PAD_NUM_MAX];
};

struct class *cap_sensor_class;


static void ad7146_set_switch_status(struct ad7146_chip *ad7146,
	unsigned short val)
{
	static unsigned short prv_sw_status = SW_INIT_STATUS;

	if (AD7146_I2C_RW_NO_ERR != ad7146->i2c_err_flag)
		val = AD7146_SENS_DET;
	if (prv_sw_status != val) {
		switch_set_state(&ad7146->sw_state, val);
		dev_dbg(ad7146->dev, "%s: set switch state=%x\n",
			__func__, val);
	}
	prv_sw_status = switch_get_state(&ad7146->sw_state);
	return;
}

static int ad7146_i2c_write(struct device *dev, unsigned short reg,
		unsigned short data)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ad7146_chip *ad7146 = i2c_get_clientdata(client);
	char device_addr = client->addr;
	unsigned short tx[I2C_READ_LEN];
	int ret = 0;
	int rty_cnt;
	struct i2c_msg ad7146_wr_msg = {
			.addr = device_addr,
			.buf = (u8 *)tx,
			.len = I2C_WRITE_LEN,
			.flags = 0,
	};

	tx[0] = cpu_to_be16(reg);
	tx[1] = cpu_to_be16(data);
	for (rty_cnt = 0; rty_cnt < I2C_ACCESS_RETRY_MAX; rty_cnt++) {
		ret = i2c_transfer(client->adapter,
			&ad7146_wr_msg, I2C_MSG_LEN);
		if (ret < 0)
			msleep(I2C_ACCESS_RETRY_WAIT);
		else
			break;
	}

	if (rty_cnt < I2C_ACCESS_RETRY_MAX) {
		ad7146->i2c_err_flag = AD7146_I2C_RW_NO_ERR;
	} else {
		dev_err(&client->dev, "I2C WRITE error\n");
		ad7146->i2c_err_flag = AD7146_I2C_RW_ERR;
		ad7146_set_switch_status(ad7146, AD7146_SENS_DET);
	}
	return ret;
}

static int ad7146_i2c_read(struct device *dev, unsigned short reg,
					unsigned short *data)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ad7146_chip *ad7146 = i2c_get_clientdata(client);
	unsigned short tx = cpu_to_be16(reg);
	unsigned short rx = 0;
	char device_addr = client->addr;
	int ret = 0;
	int rty_cnt;
	struct i2c_msg ad7146_rd_msg[I2C_READ_LEN] = {
			{
				.addr = device_addr,
				.buf = (u8 *)&tx,
				.len = I2C_READ_LEN,
				.flags = 0,
			},
			{
				.addr = device_addr,
				.buf = (u8 *)&rx,
				.len = I2C_READ_LEN,
				.flags = I2C_M_RD,
			}
	};

	for (rty_cnt = 0; rty_cnt < I2C_ACCESS_RETRY_MAX; rty_cnt++) {
		ret = i2c_transfer(client->adapter,
			ad7146_rd_msg, I2C_READ_LEN);
		if (ret < 0)
			msleep(I2C_ACCESS_RETRY_WAIT);
		else
			break;
	}

	if (rty_cnt < I2C_ACCESS_RETRY_MAX) {
		*data = be16_to_cpu(rx);
		ad7146->i2c_err_flag = AD7146_I2C_RW_NO_ERR;
	} else {
		dev_err(dev, "I2C READ error %d\n", ret);
		ad7146->i2c_err_flag = AD7146_I2C_RW_ERR;
		ad7146_set_switch_status(ad7146, AD7146_SENS_DET);
	}
	return ret;
}

static ssize_t do_dac_calibrate(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned short val = 0;
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);

	ret = kstrtou16(buf, DECIMAL_BASE, &val);
	if (ret || val != ACT_SYSFS) {
		dev_err(ad7146->dev, "[AD7146]: %s INVALID CMD", __func__);
		return count;
	}
	if (!work_pending(&ad7146->calib_work)) {
		ad7146->curr_dac_status = INIT_DAC;
		schedule_work(&ad7146->calib_work);
	} else {
		dev_err(ad7146->dev, "[AD7146]: WORK PENDING!\n");
	}
	return count;
}

static ssize_t show_dac_status(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	void  *dac_temp_buf = NULL;
	unsigned short dac_length;
	char temp_buf[TEMP_BUFER_MAX_LEN];
	int ret;
	if (DONE_SUCCESS != ad7146->curr_dac_status) {
		ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			"%d", ad7146->curr_dac_status);
		memcpy(buf, temp_buf, ret);
	} else {
		ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			"%d", ad7146->curr_dac_status);
		dac_temp_buf = buf;
		memcpy(buf, temp_buf, ret);
		dac_temp_buf = dac_temp_buf + ret;
		dac_length = (*ad7146->dac_cal_buffer) * sizeof(short) *
			sizeof(short) + sizeof(short);
		dac_temp_buf = memcpy(dac_temp_buf, ad7146->dac_cal_buffer,
			dac_length);
		ret += dac_length;
	}
	return ret;
}

static inline unsigned short decimation_calc(unsigned short cal_pwr_reg)
{
	unsigned short deci_factor = 0;
	deci_factor = ((cal_pwr_reg & DECIMATION_MASK) >> BYTE_SIZE) +
		ADD_FACTOR;
	if (deci_factor > DECI_MAX)
		deci_factor  = DECI_MAX;
	return deci_factor;
}

static ssize_t store_obj_detect(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned short val = 0;
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);

	ret = kstrtou16(buf, DECIMAL_BASE, &val);
	if (ret || (AD7146_SENS_DET != val &&
				AD7146_SENS_NOT_DET != val)) {
		dev_err(ad7146->dev, "%s INVALID CMD %d",
			__func__, val);
		return count;
	}
	if (!ad7146->pad_enable_state) {
		dev_err(ad7146->dev, "%s not enable pad !", __func__);
		return count;
	}

	ad7146_set_switch_status(ad7146, val);
	return count;
}

static ssize_t show_obj_detect(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int ret = 0;
	char temp_buf[TEMP_BUFER_MAX_LEN];

	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"%d", switch_get_state(&ad7146->sw_state));
	memcpy(buf, temp_buf, ret);
	return ret;
}

static void dac_calibration_work(struct work_struct *work)
{
	struct ad7146_chip *ad7146 = container_of(work,
						struct ad7146_chip,
						calib_work);
	unsigned short dac_curr_cdc = 0;
	unsigned short dac_cur_afe = 0;
	unsigned short dac_pos_afe = 0;
	unsigned short dac_neg_afe = 0;
	unsigned short dac_power_reg = 0;
	unsigned short dac_curr_stg = 0;
	unsigned short dac_afe_reg = 0;
	unsigned short try_neg_swap = 0;
	unsigned short try_pos_swap = 0;
	unsigned short dac_sleeptime = 0;
	unsigned short dac_tot_caldata = 0;
	unsigned short deci_factor = 0;
	unsigned short *dac_temp_buffer = NULL;
	unsigned long dac_start_time = 0;
	unsigned char stage_cal_status[STAGE_NUM] = {[0 ... (STAGE_NUM-1)] = 0};
	unsigned char afe_written[STAGE_NUM] = {[0 ... (STAGE_NUM-1)] = 0};
	unsigned short calib_pending = ad7146->stg_connection;
	struct ad7146_product_stgx_data *pt;

	if (!ad7146->pad_enable_state) {
		dev_info(ad7146->dev,
			"All pads are not enabled. Close calibration...\n");
		ad7146->curr_dac_status = DONE_FAILED;
		return;
	}

	mutex_lock(&ad7146->mutex);
	ad7146->write(ad7146->dev, STG_COM_INT_EN_REG, DISABLE_AD7146);
	ad7146->write(ad7146->dev, STG_HIGH_INT_EN_REG, DISABLE_AD7146);
	ad7146->write(ad7146->dev, STG_LOW_INT_EN_REG, DISABLE_AD7146);

	disable_irq(ad7146->irq);

	ad7146->curr_dac_status = PENDING;

	/* Change to Full power mode if needed */
	dac_power_reg = shadow_reg[PWR_CONTROL].data;
	ad7146->write(ad7146->dev, PWR_CTRL_REG,
		(dac_power_reg & PWR_MODE_MASK));
	dac_sleeptime =  POWER_SLEEP_TIME(dac_power_reg);
	dev_err(ad7146->dev, "changing to full power with delay %d",
		dac_sleeptime);
	msleep(dac_sleeptime);

	/* Change Modulation frequency if needed */
	if (shadow_reg[MOD_FREQ_CTL].data & MAX_AFE_VALUE)
		ad7146->write(ad7146->dev, MOD_FREQ_CTL_REG,
			DEFAULT_MOD_FREQ_VALUE);
	ad7146->read(ad7146->dev, MOD_FREQ_CTL_REG, &dac_sleeptime);

	if (!ad7146->stg_connection) {
		dev_info(ad7146->dev, "NO STAGE connection!!!\n");
		goto er_cal_no_buf;
	}
	/* Read decimation and calculate the sleep time for conversion */
	deci_factor = decimation_calc(dac_power_reg);

	dev_dbg(ad7146->dev, "DECIMATION factor %d!!!\n", deci_factor);
	dac_sleeptime = ((ad7146->last_stg_en + ADD_FACTOR) * FT_CALIB_DELAY *
		((dac_sleeptime & MAX_AFE_VALUE) + ADD_FACTOR)) /
		deci_factor;
	dev_dbg(ad7146->dev, "Calc CONV Time %dms!\n", dac_sleeptime);
	kzfree(ad7146->dac_cal_buffer);
	ad7146->dac_cal_buffer = kzalloc(
			(sizeof(short) + (sizeof(short) *
			CAL_ROW_SIZE * ad7146->no_of_stages)),
			GFP_KERNEL);
	if (!ad7146->dac_cal_buffer) {
		dev_err(ad7146->dev, "%s: NO MEM\n", __func__);
		goto er_cal_f;
	}
	/* Store the data in the data field by using this temp pointer */
	dac_temp_buffer =
		(unsigned short *)((char *)ad7146->dac_cal_buffer
		+ sizeof(short));
	dac_start_time = jiffies;

	while ((jiffies_to_msecs(jiffies - dac_start_time) < FT_CALIB_T_OUT) &&
			calib_pending) {
		/* Loop till the enabled stages */
		for (dac_curr_stg = 0; dac_curr_stg <= ad7146->last_stg_en;
				dac_curr_stg++) {
			dev_info(ad7146->dev, "CURRENT STAGE = %d!!!\n",
				 dac_curr_stg);
			if ((!(ad7146->stg_connection &
				AD7146_BIT_MASK(dac_curr_stg))) ||
				stage_cal_status[dac_curr_stg]) {
				/*
				  Stage not connected or
				  calibration already done
				 */
				continue;
			}
			/* Reading AFE OFFSET Register */
			dac_afe_reg = GET_AFE_REG(dac_curr_stg);
			dac_cur_afe = CDC_ZERO_VALUE;
			ad7146->read(ad7146->dev,
				(CDC_RESULT_S0_REG + dac_curr_stg),
				&dac_curr_cdc);

			if ((CDC_ZERO_VALUE == dac_curr_cdc ||
				FULL_SCALE_VALUE == dac_curr_cdc) &&
				!afe_written[dac_curr_stg]) {
				ad7146->write(ad7146->dev, dac_afe_reg,
					DISABLE_AD7146);
				afe_written[dac_curr_stg] = ENABLE_AD7146;
			} else {
				ad7146->read(ad7146->dev, dac_afe_reg,
					&dac_cur_afe);
				try_pos_swap = dac_cur_afe & POS_SWAP_MASK;
				try_neg_swap = dac_cur_afe & NEG_SWAP_MASK;
			}

			dev_dbg(ad7146->dev, "CURSTG %x R %x D %x CDC %x\n",
				(dac_curr_stg), dac_afe_reg,
				dac_cur_afe, dac_curr_cdc);

			dac_neg_afe = GET_NEG_AFE(dac_cur_afe);
			dac_pos_afe = GET_POS_AFE(dac_cur_afe);
			/* Saturation Exception flow */
			if (CDC_ZERO_VALUE == dac_curr_cdc ||
				FULL_SCALE_VALUE == dac_curr_cdc) {
				dev_dbg(ad7146->dev, "PS%d NS%d\n",
					try_pos_swap,
					try_neg_swap);
				if (!try_neg_swap && !try_pos_swap) {
					/* As both of the afe */
					dac_pos_afe += MIN_AFE_STEP;
					if (dac_pos_afe > MAX_AFE_VALUE) {
						try_neg_swap = ENABLE_AD7146;
						dac_cur_afe |= NEG_SWAP_MASK;
					} else {
						dac_cur_afe =
							SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
					}
				} else if (try_neg_swap && !try_pos_swap) {
					dac_neg_afe += MIN_AFE_STEP;
					if (dac_neg_afe > MAX_AFE_VALUE) {
						try_pos_swap = ENABLE_AD7146;
						try_neg_swap = DISABLE_AD7146;
						dac_cur_afe = (dac_cur_afe |
							POS_SWAP_MASK) &
							(~NEG_SWAP_MASK);
					} else {
						dac_cur_afe =
							SET_NEG_AFE(dac_cur_afe,
								dac_neg_afe);
					}
				} else if (try_pos_swap && !try_neg_swap) {
					if (dac_pos_afe > MIN_AFE_STEP)
						dac_pos_afe -= MIN_AFE_STEP;
					else if (dac_neg_afe > MIN_AFE_STEP)
						dac_neg_afe -= MIN_AFE_STEP;
					else
						goto er_cal_f;
					dac_cur_afe =
						SET_NEG_AFE(dac_cur_afe,
							dac_neg_afe);
					dac_cur_afe =
						SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
				} else {
					/* Both of the swap is set */
					goto er_cal_unkwn_f;
				}
			} else if (dac_curr_cdc > ad7146->open_air_high) {
				if (dac_pos_afe < MAX_AFE_VALUE &&
					!try_pos_swap) {
					dac_pos_afe++;
					dac_cur_afe =
						SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
				} else if (dac_neg_afe > 0 && !try_neg_swap) {
					dac_neg_afe--;
					dac_cur_afe =
						SET_NEG_AFE(dac_cur_afe,
							dac_neg_afe);
				} else if (dac_pos_afe > 0 && try_pos_swap) {
					dac_pos_afe--;
					dac_cur_afe =
						SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
				} else if (dac_neg_afe < MAX_AFE_VALUE &&
					try_neg_swap) {
					dac_neg_afe++;
					dac_cur_afe =
						SET_NEG_AFE(dac_cur_afe,
							dac_neg_afe);
				} else {
					dev_err(ad7146->dev, "CDC can't be reduced\n");
					goto er_cal_f;
				}
			} else if (dac_curr_cdc < ad7146->open_air_low) {
				if (dac_pos_afe > 0 && !try_pos_swap) {
					dac_pos_afe--;
					dac_cur_afe =
						SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
				} else if (dac_neg_afe < MAX_AFE_VALUE &&
					!try_neg_swap) {
					dac_neg_afe++;
					dac_cur_afe =
						SET_NEG_AFE(dac_cur_afe,
							dac_neg_afe);
				} else if (dac_pos_afe < MAX_AFE_VALUE &&
					try_pos_swap) {
					dac_pos_afe++;
					dac_cur_afe =
						SET_POS_AFE(dac_cur_afe,
							dac_pos_afe);
				} else if (dac_neg_afe > 0 && try_neg_swap) {
					dac_neg_afe--;
					dac_cur_afe =
						SET_NEG_AFE(dac_cur_afe,
							dac_neg_afe);
				} else {
					dev_err(ad7146->dev,
						"CDC can't be increased\n");
					goto er_cal_f;
				}
			} else {
				/* CDC is in RANGE */
				*dac_temp_buffer = dac_afe_reg;
				dac_temp_buffer++;
				stage_cal_status[dac_curr_stg] = ENABLE_AD7146;
				*dac_temp_buffer = dac_cur_afe;
				dac_temp_buffer++;
				calib_pending &= (unsigned short)
					~AD7146_BIT_MASK(dac_curr_stg);
				dac_tot_caldata++;
				continue;
			}
			/* Update the AFE for the CURRENT STAGE */
			ad7146->write(ad7146->dev, dac_afe_reg, dac_cur_afe);
			if (PAD_NUM_MAX > dac_curr_stg) {
				pt = &ad7146->product_data.
					stgx_data[dac_curr_stg];
				if (!pt) {
					dev_err(ad7146->dev, "%s: pt is NULL!",
						__func__);
					goto er_cal_f;
				}
				pt->stgx_afe_offset = dac_cur_afe;
			}
		}
		/* Sleep after one complete cycle of AFE changes */
		msleep(dac_sleeptime);
	}
	dev_info(ad7146->dev, "Factory Calibration Complete !\nStatus:");
	if (!calib_pending && dac_tot_caldata == ad7146->no_of_stages) {
		for (dac_curr_stg = 0; dac_curr_stg <= ad7146->last_stg_en;
				dac_curr_stg++) {
			dac_afe_reg = GET_AFE_REG(dac_curr_stg);
			ad7146->read(ad7146->dev, dac_afe_reg, &dac_cur_afe);
			if (PAD_NUM_MAX > dac_curr_stg) {
				pt = &ad7146->product_data.
					stgx_data[dac_curr_stg];
				if (!pt) {
					dev_err(ad7146->dev, "%s: pt is NULL!",
						__func__);
					goto er_cal_f;
				}
				pt->stgx_afe_offset = dac_cur_afe;
			}
		}
		*ad7146->dac_cal_buffer = dac_tot_caldata;
		dev_err(ad7146->dev, "SUCCESS\t TOTAL_STG = %d!!!\n",
			ad7146->no_of_stages);
		ad7146->curr_dac_status = DONE_SUCCESS;
	} else {
		dev_err(ad7146->dev, "FAILED\t TOTAL_STG=%d STAGES_DONE=%d\n",
			ad7146->no_of_stages, dac_tot_caldata);
		ad7146->curr_dac_status = DONE_FAILED;
	}
	ad7146->write(ad7146->dev, MOD_FREQ_CTL_REG,
		shadow_reg[MOD_FREQ_CTL].data);
	enable_irq(ad7146->irq);
	ad7146->write(ad7146->dev, STG_HIGH_INT_EN_REG,
		shadow_reg[STG_HIGH_INT_EN].data);
	ad7146->write(ad7146->dev, STG_LOW_INT_EN_REG,
		shadow_reg[STG_LOW_INT_EN].data);
	ad7146->write(ad7146->dev, PWR_CTRL_REG,
		shadow_reg[PWR_CONTROL].data);
	dev_info(ad7146->dev, "POWER MODE restored!!!\n");
	/* Do force recalibration of all the stages */
	ad7146->write(ad7146->dev, AMB_COMP_CTRL0_REG,
		shadow_reg[PWR_CONTROL].data | AMB_COMP_FORCE_CAL);
	msleep(MIN_FORCED_CAL_SLEEP);
	/* Clear Interruput */
	ad7146->read(ad7146->dev, STG_LOW_INT_STA_REG, &ad7146->low_status);
	ad7146->read(ad7146->dev, STG_HIGH_INT_STA_REG, &ad7146->high_status);
	mutex_unlock(&ad7146->mutex);
	return;

er_cal_unkwn_f:
	dev_err(ad7146->dev, "UNKNOWN error\n");
er_cal_f:
	kzfree(ad7146->dac_cal_buffer);
	ad7146->dac_cal_buffer = NULL;
er_cal_no_buf:
	ad7146->curr_dac_status = DONE_FAILED;
	ad7146->write(ad7146->dev, MOD_FREQ_CTL_REG,
		shadow_reg[MOD_FREQ_CTL].data);
	enable_irq(ad7146->irq);
	ad7146->write(ad7146->dev, STG_HIGH_INT_EN_REG,
		shadow_reg[STG_HIGH_INT_EN].data);
	ad7146->write(ad7146->dev, STG_LOW_INT_EN_REG,
		shadow_reg[STG_LOW_INT_EN].data);
	ad7146->write(ad7146->dev, PWR_CTRL_REG,
		shadow_reg[PWR_CONTROL].data);
	mutex_unlock(&ad7146->mutex);
	dev_err(ad7146->dev, "POWER MODE restored!!!\n");
	dev_err(ad7146->dev, "Factory calibration FAILED\n");
	dev_err(ad7146->dev, "MAXIMUX RETRY REACHED\n");
	return;
}

static void getStageInfo(struct ad7146_chip *ad7146)
{
	unsigned short intr_en = 0;
	unsigned short num_of_stages = 0;
	unsigned short last_stage_num = 0;
	unsigned short sens_stages = 0;
	unsigned short sens_active = 0;
	unsigned short temp_1, temp_2, count;
	unsigned char stage_index = 0;
	unsigned char curnt_stg = 0;
	unsigned int stg_conn;

	intr_en = 0;
	ad7146->sensor_int_enable = 0;
	ad7146->last_stg_en = 0;
	num_of_stages = 0;
	ad7146->read(ad7146->dev, STG_HIGH_INT_EN_REG, &sens_active);
	while (stage_index < (STAGE_NUM * STG_CONN_CNT)) {
		ad7146->read(ad7146->dev,
			stg_conn_register[stage_index], &temp_1);
		stage_index++;

		ad7146->read(ad7146->dev,
			stg_conn_register[stage_index], &temp_2);
		stage_index++;
		/* temp_1 -> Lower order, while temp_2 -> Higher order */
		stg_conn =
			temp_2 << LOW_O_STAGE_SFT | (temp_1 & STG_CON_MASK);
		curnt_stg = (stage_index / STG_CONN_CNT) - ADD_FACTOR;
		dev_dbg(ad7146->dev, "STAGE %d CON_REG = %x\n",
			curnt_stg, stg_conn);
		/* Check the 13 CIN's and the internal Sensor connectivity */
		for (count = 0; count < STG_CONN_CHK_CNT; count++) {
			if (POS_STG_CON_MASK == (stg_conn & DECI_MAX)) {
				if (sens_active & AD7146_BIT_MASK(curnt_stg)) {
					ad7146->sw[sens_stages].index =
						curnt_stg;
					sens_stages++;
				}
				intr_en = (intr_en |
					(AD7146_BIT_MASK(curnt_stg)));
				last_stage_num = curnt_stg;
				num_of_stages++;
				break;
			}
			stg_conn = (stg_conn >> STG_CONN_CNT);
		}
	}
	intr_en = intr_en & INT_EN_MASK;
	ad7146->sensor_int_enable = (intr_en & sens_active);
	ad7146->stg_calib_enable =  ad7146->sensor_int_enable;
	ad7146->last_stg_en = last_stage_num;
	ad7146->stg_connection = intr_en;
	ad7146->no_of_sens_stages = sens_stages;
	ad7146->no_of_stages = num_of_stages;

	dev_dbg(ad7146->dev,
		"Stage Calibration Enable = %d No_of_Sens_stg = %d ",
		intr_en, sens_stages);
	dev_dbg(ad7146->dev, "last_stg_en = %d\n",
		last_stage_num);
	dev_dbg(ad7146->dev, "sensor_int_enable  = %d\n",
		ad7146->sensor_int_enable);
	return;
}

static ssize_t store_dac_mid_value(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int err;
	unsigned short val;
	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(ad7146->dev, "%s: INVALID CMD", __func__);
		return err;
	}
	mutex_lock(&ad7146->mutex);

	dev_dbg(ad7146->dev, "%s: val = 0x%04x\n", __func__, val);
	if (val >= MIN_DAC_MID_VAL && val <= MAX_DAC_MID_VAL) {
		ad7146->open_air_low = val - DAC_DIFF_VAL;
		ad7146->open_air_high = val + DAC_DIFF_VAL;
	} else {
		dev_err(ad7146->dev, "%s: Invalid val = 0x%04x\n",
			__func__, val);
	}

	mutex_unlock(&ad7146->mutex);
	return count;
}

static ssize_t show_dac_mid_value(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ad7146_chip  *ad7146 = dev_get_drvdata(dev);
	int ret;
	char temp_buf[TEMP_BUFER_MAX_LEN];
	int mid_val = (ad7146->open_air_low + ad7146->open_air_high) /
				STG_CONN_CNT;

	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"%d %d %d\n", ad7146->open_air_low, mid_val,
			ad7146->open_air_high);
	memcpy(buf, temp_buf, ret);
	return ret;
}

static ssize_t do_force_calibrate(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int err;
	unsigned short val;
	unsigned short temp;
	int cnt;
	struct ad7146_driver_data *sw = NULL;
	unsigned short temp_th[PAD_NUM_MAX];

	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(ad7146->dev,
			"%s: INVALID CMD (%d)", __func__, err);
		goto force_calib_end;
	}
	if (ENABLE_AD7146 != val) {
		err = -EINVAL;
		dev_err(ad7146->dev,
			"%s: INVALID VAL(%u)", __func__, val);
		goto force_calib_end;
	}
	if (!(ad7146->pad_enable_state & STG0_EN_FLG) ||
		!(ad7146->pad_enable_state & STG1_EN_FLG)) {
		ad7146->fc_flag = DISABLE_AD7146;
		ad7146->keep_detect_flag = DISABLE_AD7146;
		err = -EINVAL;
		dev_err(ad7146->dev,
			"%s: INVALID CMD", __func__);
		goto force_calib_end;
	}

	mutex_lock(&ad7146->mutex);
	if (!ad7146->fc_flag) {
		ad7146->read(ad7146->dev, STG0_HIGH_THRESHOLD,
			&ad7146->sv_threshold[STG_ZERO]);
		ad7146->read(ad7146->dev, STG1_HIGH_THRESHOLD,
			&ad7146->sv_threshold[STG_ONE]);
		dev_dbg(ad7146->dev,
			"%s:save threshold: PAD1:%04x, PAD2:%04x\n",
			__func__, ad7146->sv_threshold[STG_ZERO],
			ad7146->sv_threshold[STG_ONE]);
	}
	ad7146->keep_detect_flag = DISABLE_AD7146;
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		sw = &ad7146->sw[cnt];
		sw->state = IDLE;
		if (STG_ZERO == sw->index)
			switch_set_state(&ad7146->sw_stg0, AD7146_SENS_NOT_DET);
		else if (STG_ONE == sw->index)
			switch_set_state(&ad7146->sw_stg1, AD7146_SENS_NOT_DET);
		else
			dev_err(ad7146->dev,
				"Failed to set state of the switch device %u.\n",
				sw->index);
	}
	ad7146_set_switch_status(ad7146, AD7146_SENS_NOT_DET);
	ad7146->sw_updata = AD7146_SENS_NOT_DET;
	sysfs_notify(&ad7146->cap_dev.dev->kobj, NULL, "sw_updata");
	ad7146->read(ad7146->dev, AMB_COMP_CTRL0_REG, &temp);
	temp = temp | AD7146_FORCED_CAL_MASK;
	ad7146->write(ad7146->dev, AMB_COMP_CTRL0_REG, temp);
	msleep(MIN_FORCED_CAL_SLEEP);
	ad7146->read(ad7146->dev, STG0_HIGH_THRESHOLD, &temp_th[STG_ZERO]);
	ad7146->read(ad7146->dev, STG1_HIGH_THRESHOLD, &temp_th[STG_ONE]);
	if (!ad7146->fc_flag &&
		(temp_th[STG_ZERO] > ad7146->sv_threshold[STG_ZERO] ||
		temp_th[STG_ONE] > ad7146->sv_threshold[STG_ONE]))
		ad7146->fc_flag = ENABLE_AD7146;
	if (ad7146->fc_flag &&
		(temp_th[STG_ZERO] <= ad7146->sv_threshold[STG_ZERO] ||
		temp_th[STG_ONE] <= ad7146->sv_threshold[STG_ONE])) {
		ad7146->fc_flag = DISABLE_AD7146;
		dev_dbg(ad7146->dev,
			"%s:fc flag off: PAD1:%04x < %04x and PAD2:%04x < %04x\n",
			__func__,
			temp_th[STG_ZERO], ad7146->sv_threshold[STG_ZERO],
			temp_th[STG_ONE], ad7146->sv_threshold[STG_ONE]);
	}
	ad7146->read(ad7146->dev, STG_LOW_INT_STA_REG, &temp);
	ad7146->read(ad7146->dev, STG_HIGH_INT_STA_REG, &temp);
	mutex_unlock(&ad7146->mutex);
	if (ad7146->i2c_err_flag) {
		err = -EIO;
		dev_err(ad7146->dev,
			"%s: Failed to force calibration. (%d)\n",
			__func__, err);
		goto force_calib_end;
	}
	dev_dbg(ad7146->dev,
		"%s: Force calibration done. fc_flag=%u keep_detect_flag=%u\n",
		__func__, ad7146->fc_flag, ad7146->keep_detect_flag);
	err = count;

force_calib_end:
	return err;
}

static ssize_t store_pad_num(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int err;
	unsigned short val;
	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(ad7146->dev,
			"%s: INVALID CMD (%d)", __func__, err);
		return err;
	}
	if (PAD_NUMBER_MIN > val || PAD_NUMBER_MAX < val) {
		dev_err(ad7146->dev,
			"%s: INVALID VAL (%u)", __func__, val);
		return -EINVAL;
	}
	if (PAD_NUM_MAX < val) {
		dev_err(ad7146->dev,
			"%s: Not support pad (%u)", __func__, val);
		return -EINVAL;
	}

	ad7146->current_pad_no = val - PAD_NUMBER_MIN;
	return count;
}

static ssize_t show_pad_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int ret;
	unsigned short rd_data;
	char temp_buf[TEMP_BUFER_MAX_LEN];

	if (!ad7146->pad_enable_state) {
		dev_err(ad7146->dev, "%s not enable pad !", __func__);
		return -EINVAL;
	}
	if (PAD1 > ad7146->current_pad_no ||
		PAD_NUMBER_MAX <= ad7146->current_pad_no) {
		dev_err(ad7146->dev, "%s: Invalid current_pad_no (%u)",
			__func__, ad7146->current_pad_no);
		return -EINVAL;
	}

	ad7146->read(ad7146->dev,
		(CDC_RESULT_S0_REG + ad7146->current_pad_no), &rd_data);
	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"0x%04X\n", rd_data);
	memcpy(buf, temp_buf, ret);
	return ret;
}

static ssize_t store_pad_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int err;
	unsigned short val;
	struct ad7146_product_stgx_data *pt;

	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(ad7146->dev,
			"%s: INVALID CMD (%d)", __func__, err);
		return err;
	}
	if (PAD1 > ad7146->current_pad_no ||
		PAD2 < ad7146->current_pad_no) {
		dev_err(ad7146->dev, "%s: Invalid current_pad_no (%u)",
			__func__, ad7146->current_pad_no);
		return -EINVAL;
	}

	dev_dbg(ad7146->dev, "%s: val = 0x%04x\n", __func__, val);
	pt = &ad7146->product_data.stgx_data[ad7146->current_pad_no];
	if (!pt) {
		dev_err(ad7146->dev, "%s: pt is NULL!", __func__);
		return -ENOMEM;
	}
	pt->stgx_afe_offset = val;
	return count;
}

static ssize_t show_pad_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int ret;
	unsigned short val;
	char temp_buf[TEMP_BUFER_MAX_LEN];
	struct ad7146_product_stgx_data *pt;

	if (PAD1 > ad7146->current_pad_no ||
		PAD2 < ad7146->current_pad_no) {
		dev_err(ad7146->dev, "%s: Invalid current_pad_no (%u)",
			__func__, ad7146->current_pad_no);
		return -EINVAL;
	}

	pt = &ad7146->product_data.stgx_data[ad7146->current_pad_no];
	if (!pt) {
		dev_err(ad7146->dev, "%s: pt is NULL!", __func__);
		return -ENOMEM;
	}
	val = pt->stgx_afe_offset;
	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"0x%04X\n", val);
	memcpy(buf, temp_buf, ret);
	dev_dbg(ad7146->dev, "%s: val = 0x%04x\n", __func__, val);
	return ret;
}

static ssize_t show_sw_updata(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int ret;
	char temp_buf[TEMP_BUFER_MAX_LEN];

	if (!ad7146->pad_enable_state) {
		dev_err(ad7146->dev,
			"%s: INVALID CMD", __func__);
		return -EINVAL;
	}

	if (!ad7146->sw_updata) {
		dev_dbg(ad7146->dev, "%s: sw_updata not detect.\n",
			__func__);
		return RET_ZERO_VAL;
	}

	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"%u\n", AD7146_SENS_DET);
	memcpy(buf, temp_buf, ret);
	dev_dbg(ad7146->dev, "%s: sw_updata detected. : %u\n",
		__func__, ad7146->sw_updata);
	return RET_ZERO_VAL;
}

static int ad7146_setup_defaults(struct ad7146_chip *ad7146)
{
	int rc = 0;
	int stage;
	struct ad7146_product_stgx_data *pt;

	for (stage = STG_ZERO; stage < STAGE_NUM; stage++) {
		ad7146->write(ad7146->dev,
			STG0_CONNECTION_6_0_REG + stage * STAGE_REG_NUM,
			STGx_CONNECT_OFF1);
		ad7146->write(ad7146->dev,
			STG0_CONNECTION_12_7_REG + stage * STAGE_REG_NUM,
			STGx_CONNECT_OFF2);
		if (PAD_NUM_MAX > stage) {
			pt = &ad7146->product_data.stgx_data[stage];
			if (!pt) {
				dev_err(ad7146->dev, "%s: pt is NULL!",
					__func__);
				rc = -ENOMEM;
				pt = &default_stgx_data;
			}
			ad7146->write(ad7146->dev,
				(STG0_AFE_OFFSET_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_afe_offset);
			ad7146->write(ad7146->dev,
				(STG0_SENSITIVITY_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_sensitivity);
			ad7146->write(ad7146->dev,
				(STG0_OFFSET_LOW_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_offset_low);
			ad7146->write(ad7146->dev,
				(STG0_OFFSET_HIGH_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_offset_high);
			ad7146->write(ad7146->dev,
				(STG0_OFFSET_HIGH_CLAMP_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_offset_high_clamp);
			ad7146->write(ad7146->dev,
				(STG0_OFFSET_LOW_CLAMP_REG +
				stage * STAGE_REG_NUM),
				(unsigned short)pt->stgx_offset_low_clamp);
		}
	}
	return rc;
}

static int ad7146_pad_setting(struct ad7146_chip *ad7146,
		unsigned short val)
{
	int rc = 0;
	int i;
	unsigned short pwr_data, data;
	struct ad7146_product_data *dt;
	struct ad7146_product_stgx_data *pt0, *pt1;

	dev_dbg(ad7146->dev, "pat setting: before-%x, after-%x\n",
		ad7146->pad_enable_state, val);

	dt = &ad7146->product_data;
	if (!dt) {
		dev_err(ad7146->dev, "%s: dt is NULL!", __func__);
		return -ENOMEM;
	}
	pt0 = &ad7146->product_data.stgx_data[PAD1];
	pt1 = &ad7146->product_data.stgx_data[PAD2];
	if ((!pt0 && (val & STG0_EN_FLG)) ||
		(!pt1 && (val & STG1_EN_FLG))) {
		dev_err(ad7146->dev, "%s: pt is NULL!", __func__);
		return -ENOMEM;
	}

	mutex_lock(&ad7146->mutex_shadow);
	/* Set product initial data */
	shadow_reg[MOD_FREQ_CTL].data =
		(unsigned short)dt->mod_freq_ctrl;
	shadow_reg[AMB_COMP_CTRL0].data =
		(unsigned short)dt->amb_comp_ctrlx[AMB_COMP_CTRL_NUM0];
	shadow_reg[AMB_COMP_CTRL0_2].data =
		(unsigned short)dt->amb_comp_ctrlx[AMB_COMP_CTRL_NUM0];
	shadow_reg[AMB_COMP_CTRL1].data =
		(unsigned short)dt->amb_comp_ctrlx[AMB_COMP_CTRL_NUM1];
	shadow_reg[AMB_COMP_CTRL2].data =
		(unsigned short)dt->amb_comp_ctrlx[AMB_COMP_CTRL_NUM2];

	/* Clear all int enable flag */
	shadow_reg[STG_LOW_INT_EN].data = ALL_STG_INT_EN_OFF;
	shadow_reg[STG_HIGH_INT_EN].data = ALL_STG_INT_EN_OFF;
	shadow_reg[STG_CAL_EN].data = ALL_STG_CAL_EN_OFF;
	shadow_reg[AMB_COMP_CTRL0_2].data &= ~AMB_COMP_FORCE_CAL;
	shadow_reg[PWR_CONTROL].data |= PWR_MODE_SHUTDOWN;
	pwr_data = shadow_reg[PWR_CONTROL].data;

	/* Update shadow register */
	if (val)
		shadow_reg[PWR_CONTROL].data =
			(shadow_reg[PWR_CONTROL].data & PWR_MODE_MASK) |
			PWR_MODE_LOWPWR;

	if (val & STG0_EN_FLG) {
		shadow_reg[STG_LOW_INT_EN].data |= STG0_INT_EN;
		shadow_reg[STG_HIGH_INT_EN].data |= STG0_INT_EN;
		shadow_reg[STG_CAL_EN].data |= STG0_CAL_EN;
		shadow_reg[STG0_CONNECTION_6_0].data =
			(unsigned short)pt0->stgx_conn_6_0;
		shadow_reg[STG0_CONNECTION_12_7].data =
			(unsigned short)pt0->stgx_conn_12_7;
		shadow_reg[AMB_COMP_CTRL0_2].data |= AMB_COMP_FORCE_CAL;
	} else {
		shadow_reg[STG0_CONNECTION_6_0].data = STGx_CONNECT_OFF1;
		shadow_reg[STG0_CONNECTION_12_7].data = STGx_CONNECT_OFF2;
	}
	if (val & STG1_EN_FLG) {
		shadow_reg[STG_LOW_INT_EN].data |= STG1_INT_EN;
		shadow_reg[STG_HIGH_INT_EN].data |= STG1_INT_EN;
		shadow_reg[STG_CAL_EN].data |= STG1_CAL_EN;
		shadow_reg[STG1_CONNECTION_6_0].data =
			(unsigned short)pt1->stgx_conn_6_0;
		shadow_reg[STG1_CONNECTION_12_7].data =
			(unsigned short)pt1->stgx_conn_12_7;
		shadow_reg[AMB_COMP_CTRL0_2].data |= AMB_COMP_FORCE_CAL;
	} else {
		shadow_reg[STG1_CONNECTION_6_0].data = STGx_CONNECT_OFF1;
		shadow_reg[STG1_CONNECTION_12_7].data = STGx_CONNECT_OFF2;
	}
	mutex_unlock(&ad7146->mutex_shadow);

	mutex_lock(&ad7146->mutex);
	/* Change PowerMode to Shutdown mode */
	ad7146->write(ad7146->dev, shadow_reg[PWR_CONTROL].addr, pwr_data);
	/* Setup register */
	for (i = 0; i < AD7146_REG_MAX; i++) {
		ad7146->write(ad7146->dev, shadow_reg[i].addr,
			shadow_reg[i].data);
		dev_dbg(ad7146->dev, "%s: pat set: addr 0x%04x, data 0x%04x\n",
			__func__, shadow_reg[i].addr, shadow_reg[i].data);
	}
	msleep(MAX_FORCED_CAL_SLEEP);
	/* Clear INT status */
	ad7146->read(ad7146->dev, STG_LOW_INT_STA_REG, &data);
	ad7146->read(ad7146->dev, STG_HIGH_INT_STA_REG, &data);
	mutex_unlock(&ad7146->mutex);
	return rc;
}

static ssize_t store_pad_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct ad7146_chip *ad7146 = dev_get_drvdata(dev);
	int err, cnt;
	unsigned short val, prev_mode;
	struct ad7146_driver_data *sw = NULL;

	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(ad7146->dev, "%s: pat set error [%d]\n",
			__func__, err);
		return err;
	}

	if (val > (STG0_EN_FLG | STG1_EN_FLG)) {
		dev_err(ad7146->dev, "%s: pat set error [%d]\n",
			__func__, -EINVAL);
		return -EINVAL;
	}

	if (val != ad7146->pad_enable_state) {
		cancel_delayed_work(&ad7146->work);
		if (!(ad7146->pad_enable_state & STG0_EN_FLG) ||
			!(ad7146->pad_enable_state & STG1_EN_FLG)) {
			ad7146->fc_flag = DISABLE_AD7146;
			ad7146->keep_detect_flag = DISABLE_AD7146;
		}
		if (!ad7146->pad_enable_state) {
			mutex_lock(&ad7146->mutex);
			err = ad7146_setup_defaults(ad7146);
			mutex_unlock(&ad7146->mutex);
			if (err) {
				dev_err(ad7146->dev,
					"%s: pat default set error [%d]\n",
					__func__, err);
				return err;
			}
		}
		err = ad7146_pad_setting(ad7146, val);
		if (err) {
			dev_err(ad7146->dev,
				"%s: pat setting error [%d]\n",
				__func__, err);
			return err;
		}
		prev_mode = ad7146->pad_enable_state;
		ad7146->pad_enable_state = val;

		for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
			if ((prev_mode & AD7146_BIT_MASK(cnt)) &&
				!(val & AD7146_BIT_MASK(cnt))) {
				sw = &ad7146->sw[cnt];
				sw->state = IDLE;
				if (STG_ZERO == sw->index)
					switch_set_state(&ad7146->sw_stg0,
						AD7146_SENS_NOT_DET);
				else if (STG_ONE == sw->index)
					switch_set_state(&ad7146->sw_stg1,
						AD7146_SENS_NOT_DET);
				else
					dev_err(ad7146->dev,
						"Failed to set state of the switch device %u.\n",
						sw->index);
			}
		}
		if (!val) {
			ad7146_set_switch_status(ad7146,
				AD7146_SENS_NOT_DET);
			ad7146->sw_updata = AD7146_SENS_NOT_DET;
			sysfs_notify(&ad7146->cap_dev.dev->kobj, NULL,
				"sw_updata");
		}
	}
	return count;
}

static ssize_t show_cap_dev_name(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	char temp_buf[TEMP_BUFER_MAX_LEN];
	ret = snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
		"%s\n", DRIVER_NAME);
	memcpy(buf, temp_buf, ret);
	return ret;
}

/* sysfs table */
static struct device_attribute ad7146_sysfs_entries[] = {
	__ATTR(dac_calibrate,
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
		show_dac_status, do_dac_calibrate),
	__ATTR(obj_detect,
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
		show_obj_detect, store_obj_detect),
	__ATTR(dac_mid_val,
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
		show_dac_mid_value, store_dac_mid_value),
	__ATTR(force_calib,
		(S_IWUSR | S_IWGRP), NULL, do_force_calibrate),
	__ATTR(pad_set,
		(S_IWUSR | S_IWGRP), NULL, store_pad_set),
	__ATTR(pad_num,
		(S_IWUSR | S_IWGRP), NULL, store_pad_num),
	__ATTR(pad_data,
		(S_IRUSR | S_IRGRP), show_pad_data, NULL),
	__ATTR(pad_offset,
		(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP),
		show_pad_offset, store_pad_offset),
	__ATTR(sw_updata,
		(S_IRUSR | S_IRGRP), show_sw_updata, NULL),
	__ATTR(name,
		(S_IRUSR | S_IRGRP), show_cap_dev_name, NULL),
};

static int ad7146_sysfs_create(struct device *dev)
{
	int cnt;

	for (cnt = 0; cnt < ARRAY_SIZE(ad7146_sysfs_entries);
		cnt++)
		if (device_create_file(dev, ad7146_sysfs_entries + cnt))
			goto error_dereg_sys;

	return 0;

error_dereg_sys:
	for (--cnt; cnt >= 0 ; cnt--)
		device_remove_file(dev, ad7146_sysfs_entries + cnt);
	dev_err(dev, "%s: Unable to create sys interface\n", __func__);
	return -ENODEV;
}

static void ad7146_sysfs_remove(struct device *dev)
{
	int cnt;

	for (cnt = 0; cnt < ARRAY_SIZE(ad7146_sysfs_entries);
		cnt++)
		device_remove_file(dev, ad7146_sysfs_entries + cnt);
	return;
}

static int create_cap_sensor_class(void)
{
	cap_sensor_class = class_create(THIS_MODULE, "cap_sensor");
	if (IS_ERR(cap_sensor_class))
		return PTR_ERR(cap_sensor_class);
	return 0;
}

static int cap_sensor_dev_register(struct cap_sensor_dev *cdev)
{
	int ret;

	cdev->dev = device_create(cap_sensor_class, NULL,
		MKDEV(0, 1), NULL, "%s", DRIVER_NAME);
	if (IS_ERR(cdev->dev))
		return PTR_ERR(cdev->dev);

	ret = ad7146_sysfs_create(cdev->dev);
	if (ret < 0)
		goto err_create_dev;

	dev_set_drvdata(cdev->dev, cdev->data);
	return 0;

err_create_dev:
	device_destroy(cap_sensor_class, MKDEV(0, 1));
	printk(KERN_ERR "cap_sensor: Failed to register driver %s\n",
		cdev->name);

	return ret;
}

static void cap_sensor_dev_unregister(struct cap_sensor_dev *cdev)
{
	ad7146_sysfs_remove(cdev->dev);
	dev_set_drvdata(cdev->dev, NULL);
	device_destroy(cap_sensor_class, MKDEV(0, 1));
	return;
}

static void ad7146_hys_comp_neg(struct ad7146_chip *ad7146,
	unsigned short index)
{
	unsigned short high_threshold = 0;
	unsigned short sf_ambient = 0;
	unsigned short result = 0;

	ad7146->read(ad7146->dev, GET_HT_TH_REG(index),
		&high_threshold);
	ad7146->read(ad7146->dev, GET_AMB_REG(index),
		&sf_ambient);
	result = HYS(sf_ambient, high_threshold,
		ad7146->product_data.stgx_data[index].stgx_hysterisis);
	ad7146->write(ad7146->dev, GET_HT_TH_REG(index),
		result);
	dev_dbg(ad7146->dev,
		"N STG%d S:AMB 0x%x, T:HT 0x%x -> HYS:0x%x\n",
		index, sf_ambient, high_threshold, result);
	return;
}

static void ad7146_hys_comp_pos(struct ad7146_chip *ad7146,
	unsigned short index)
{
	unsigned short high_threshold = 0;
	unsigned short sf_ambient = 0;
	unsigned short result = 0;

	ad7146->read(ad7146->dev, GET_HT_TH_REG(index),
		&high_threshold);
	ad7146->read(ad7146->dev, GET_AMB_REG(index),
		&sf_ambient);
	result = HYS_POS(sf_ambient, high_threshold,
		ad7146->product_data.stgx_data[index].stgx_hysterisis);
	ad7146->write(ad7146->dev, GET_HT_TH_REG(index),
		result);
	dev_dbg(ad7146->dev,
		"P STG%d S:AMB 0x%x, T:HT 0x%x -> HYS_POS:0x%x\n",
		index, sf_ambient, high_threshold, result);
	return;
}

/* set switch event routine */
static void switch_set_work(struct work_struct *work)
{
	struct ad7146_chip *ad7146 = container_of(
		(struct delayed_work *)work, struct ad7146_chip, work);
	struct ad7146_driver_data *sw = NULL;
	int cnt, index, force_calib, tm_val;
	unsigned short sw_data = STGX_ALL_OFF;
	unsigned short prev_sw_data = ad7146->sw_updata;
	unsigned short data;
	unsigned short pwr_data;
	static unsigned short sv_data_point[PAD_NUM_MAX];
	static unsigned short stg_data[PAD_NUM_MAX];
	static unsigned short stg_h_threshold[PAD_NUM_MAX];
	static unsigned short stg_l_threshold[PAD_NUM_MAX];
	static unsigned short stg_data_point[PAD_NUM_MAX];
	static unsigned short switch_flag[PAD_NUM_MAX];

	if (!ad7146->pad_enable_state)
		return;

	force_calib = DISABLE_AD7146;
	mutex_lock(&ad7146->mutex);
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		if (ad7146->pad_enable_state & AD7146_BIT_MASK(cnt)) {
			/* get cdc value and threshold */
			sv_data_point[cnt] = ad7146->save_data_point[cnt];
			index = CDC_ZERO_VALUE;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
				&stg_data[cnt]);
			index++;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
				&stg_h_threshold[cnt]);
			index++;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
				&stg_l_threshold[cnt]);

			/* check cdc : threshold */
			if (stg_data[cnt] >= stg_h_threshold[cnt]) {
				stg_data_point[cnt] = AD7146_HIGH;
				switch_flag[cnt] = ACTIVE;
			} else if (stg_data[cnt] >= stg_l_threshold[cnt]) {
				stg_data_point[cnt] = AD7146_CORRECT;
				switch_flag[cnt] = IDLE;
			} else if (CDC_ZERO_VALUE == stg_data[cnt]) {
				stg_data_point[cnt] = AD7146_CORRECT;
				switch_flag[cnt] = IDLE;
			} else {
				stg_data_point[cnt] = AD7146_LOW;
				switch_flag[cnt] = IDLE;
				force_calib = ENABLE_AD7146;
			}

			dev_dbg(ad7146->dev,
				"%s:SENSOR%d:DATA %04x, HIGH %04x, LOW %04x, POINT %u\n",
				__func__, (cnt+1), stg_data[cnt],
				stg_h_threshold[cnt], stg_l_threshold[cnt],
				stg_data_point[cnt]);
		} else {
			stg_data_point[cnt] = AD7146_NOT_SET;
			dev_dbg(ad7146->dev,
				"%s:SENSOR%d:POINT %u\n", __func__, (cnt+1),
				stg_data_point[cnt]);
		}
	}
	mutex_unlock(&ad7146->mutex);

	if (ad7146->fc_flag &&
		(stg_h_threshold[STG_ZERO] <= ad7146->sv_threshold[STG_ZERO] ||
		stg_h_threshold[STG_ONE] <= ad7146->sv_threshold[STG_ONE])) {
		ad7146->fc_flag = DISABLE_AD7146;
		ad7146->keep_detect_flag = DISABLE_AD7146;
		dev_dbg(ad7146->dev,
			"%s:fc flag off: PAD1:%04x < %04x and PAD2:%04x < %04x\n",
			__func__,
			stg_h_threshold[STG_ZERO],
			ad7146->sv_threshold[STG_ZERO],
			stg_h_threshold[STG_ONE],
			ad7146->sv_threshold[STG_ONE]);
	}

	if (ad7146->fc_flag && !ad7146->keep_detect_flag) {
		for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
			if (stg_data_point[cnt] == sv_data_point[cnt] &&
				AD7146_HIGH == stg_data_point[cnt]) {
				ad7146->keep_detect_flag = ENABLE_AD7146;
				dev_dbg(ad7146->dev, "%s:keep_detect_flag on\n",
					__func__);
				break;
			}
		}
	}

	tm_val = SWITCH_WORK_NO_JITTER;
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		if (AD7146_NOT_SET == stg_data_point[cnt])
			continue;
		/* check data point - save : current */
		if (stg_data_point[cnt] != sv_data_point[cnt]) {
			mutex_lock(&ad7146->mutex);
			ad7146->save_data_point[cnt] = stg_data_point[cnt];
			mutex_unlock(&ad7146->mutex);
			tm_val = SWITCH_WORK_JITTER;
			dev_dbg(ad7146->dev,
				"%s:schedule SENSOR%d: SAVE %u != CURRENT %u\n",
				__func__, (cnt+1),
				stg_data_point[cnt], sv_data_point[cnt]);
			continue;
		}

		dev_dbg(ad7146->dev,
			"%s: stage%d: state=%u: keep_detect_flag=%u!\n",
			__func__, cnt, switch_flag[cnt],
			ad7146->keep_detect_flag);
		switch_flag[cnt] |= ad7146->keep_detect_flag;

		/* update switch state */
		sw = &ad7146->sw[cnt];
		switch (sw->state) {
		case IDLE:
			/* Sensor went to active */
			if (ACTIVE == switch_flag[cnt]) {
				dev_dbg(ad7146->dev,
					"%s: stage%d: proximity touched!\n",
					__func__, sw->index);
				sw->state = switch_flag[cnt];
				if (STG_ZERO == sw->index)
					switch_set_state(&ad7146->sw_stg0,
						AD7146_SENS_DET);
				else if (STG_ONE == sw->index)
					switch_set_state(&ad7146->sw_stg1,
						AD7146_SENS_DET);
				else
					dev_err(ad7146->dev,
						"Failed to set state of the switch device %u.\n",
						sw->index);
				sw_data |= AD7146_BIT_MASK(sw->index);
				mutex_lock(&ad7146->mutex);
				ad7146_hys_comp_neg(ad7146, sw->index);
				mutex_unlock(&ad7146->mutex);
			} else {
				dev_dbg(ad7146->dev,
					"%s: stage%d: old state=IDLE: current state=IDLE!\n",
					__func__, sw->index);
			}
			break;
		case ACTIVE:
			/* Sensor went to inactive */
			if (IDLE == switch_flag[cnt]) {
				dev_dbg(ad7146->dev,
					"%s: stage%d: proximity released!\n",
					__func__, sw->index);
				sw->state = switch_flag[cnt];
				if (STG_ZERO == sw->index)
					switch_set_state(&ad7146->sw_stg0,
						AD7146_SENS_NOT_DET);
				else if (STG_ONE == sw->index)
					switch_set_state(&ad7146->sw_stg1,
						AD7146_SENS_NOT_DET);
				else
					dev_err(ad7146->dev,
						"Failed to set state of the switch device %u.\n",
						sw->index);
				mutex_lock(&ad7146->mutex);
				ad7146_hys_comp_pos(ad7146, sw->index);
				mutex_unlock(&ad7146->mutex);
			} else {
				dev_dbg(ad7146->dev,
					"%s: stage%d: old state=ACTIVE: current state=ACTIVE!\n",
					__func__, sw->index);
			}
			break;
		default:
			break;
		}
	}

	if (prev_sw_data != sw_data) {
		ad7146->sw_updata = sw_data;
		if (!prev_sw_data && sw_data)
			sysfs_notify(&ad7146->cap_dev.dev->kobj, NULL,
				"sw_updata");
	}

	if (ENABLE_AD7146 == force_calib) {
		/* force calibration */
		mutex_lock(&ad7146->mutex);
		ad7146->read(ad7146->dev, AMB_COMP_CTRL0_REG, &data);
		data = (data | AD7146_FORCED_CAL_MASK);
		ad7146->write(ad7146->dev, AMB_COMP_CTRL0_REG, data);
		mutex_unlock(&ad7146->mutex);
		dev_dbg(ad7146->dev, "%s: force calibration!\n", __func__);
		tm_val = SWITCH_WORK_FORCE_CAL_JITTER;
	}

	/* set scheduller */
	ad7146->read(ad7146->dev, shadow_reg[PWR_CONTROL].addr, &pwr_data);
	if (tm_val && !ad7146->i2c_err_flag &&
		((pwr_data & PWR_MODE_SHUTDOWN) != PWR_MODE_SHUTDOWN))
		schedule_delayed_work(&ad7146->work, msecs_to_jiffies(tm_val));
	return;
}

static irqreturn_t ad7146_isr(int irq, void *handle)
{

	struct ad7146_chip *ad7146 = handle;
	int cnt, index, tm_val;
	unsigned short pwr_data;
	static unsigned short stg_data[PAD_NUM_MAX];
	static unsigned short stg_h_threshold[PAD_NUM_MAX];
	static unsigned short stg_l_threshold[PAD_NUM_MAX];

	if (!ad7146->pad_enable_state)
		return IRQ_HANDLED;

	cancel_delayed_work(&ad7146->work);

	mutex_lock(&ad7146->mutex);
	/* get int-status */
	ad7146->read(ad7146->dev, STG_LOW_INT_STA_REG, &ad7146->low_status);
	ad7146->read(ad7146->dev, STG_HIGH_INT_STA_REG, &ad7146->high_status);
	dev_dbg(ad7146->dev, "%s:INT-STATUS:LOW-0x%04x, HIGH:0x%04x\n",
		__func__, ad7146->low_status, ad7146->high_status);

	tm_val = SWITCH_WORK_JITTER;
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		if (ad7146->pad_enable_state & AD7146_BIT_MASK(cnt)) {
			/* get cdc value and threshold */
			index = CDC_ZERO_VALUE;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
			&stg_data[cnt]);
			index++;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
				&stg_h_threshold[cnt]);
			index++;
			ad7146->read(ad7146->dev, data_reg_addr[cnt][index],
				&stg_l_threshold[cnt]);

			/* check cdc : threshold */
			if (stg_data[cnt] >= stg_h_threshold[cnt]) {
				ad7146->save_data_point[cnt] = AD7146_HIGH;
			} else if (stg_data[cnt] >= stg_l_threshold[cnt]) {
				ad7146->save_data_point[cnt] = AD7146_CORRECT;
			} else if (CDC_ZERO_VALUE == stg_data[cnt]) {
				ad7146->save_data_point[cnt] = AD7146_CORRECT;
			} else {
				ad7146->save_data_point[cnt] = AD7146_LOW;
				tm_val = SWITCH_WORK_FORCE_CAL_JITTER;
			}
			dev_dbg(ad7146->dev,
				"%s:SENSOR%d:DATA %04x, HIGH %04x, LOW %04x, POINT %u\n",
				__func__, (cnt+1), stg_data[cnt],
				stg_h_threshold[cnt], stg_l_threshold[cnt],
				ad7146->save_data_point[cnt]);
		} else {
			ad7146->save_data_point[cnt] = AD7146_NOT_SET;
			dev_dbg(ad7146->dev,
				"%s:SENSOR%d:POINT %u\n", __func__, (cnt+1),
				ad7146->save_data_point[cnt]);
		}
	}
	mutex_unlock(&ad7146->mutex);

	/* set scheduller */
	ad7146->read(ad7146->dev, shadow_reg[PWR_CONTROL].addr, &pwr_data);
	if (!ad7146->i2c_err_flag &&
		((pwr_data & PWR_MODE_SHUTDOWN) != PWR_MODE_SHUTDOWN))
		schedule_delayed_work(&ad7146->work, msecs_to_jiffies(tm_val));
	return IRQ_HANDLED;
}

static void resume_set_work(struct work_struct *work)
{
	struct ad7146_chip *ad7146 = container_of(work,
						struct ad7146_chip,
						resume_work);
	struct ad7146_driver_data *sw = NULL;
	int cnt, rc;

	dev_dbg(ad7146->dev, "%s call: pad_enable_state = %x\n",
		__func__, ad7146->pad_enable_state);
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		sw = &ad7146->sw[cnt];
		sw->state = IDLE;
		if (STG_ZERO == sw->index)
			switch_set_state(&ad7146->sw_stg0, AD7146_SENS_NOT_DET);
		else if (STG_ONE == sw->index)
			switch_set_state(&ad7146->sw_stg1, AD7146_SENS_NOT_DET);
		else
			dev_err(ad7146->dev,
				"Failed to set state of the switch device %u.\n",
				sw->index);
	}
	ad7146_set_switch_status(ad7146, AD7146_SENS_NOT_DET);
	ad7146->sw_updata = AD7146_SENS_NOT_DET;
	ad7146->i2c_err_flag = AD7146_I2C_RW_NO_ERR;
	ad7146->fc_flag = DISABLE_AD7146;
	ad7146->keep_detect_flag = DISABLE_AD7146;

	enable_irq(ad7146->irq);
	if (ad7146->pad_enable_state) {
		rc = ad7146_setup_defaults(ad7146);
		if (!rc)
			ad7146_pad_setting(ad7146, ad7146->pad_enable_state);
	}
	return;
}

static int ad7146_hw_detect(struct ad7146_chip *ad7146)
{
	unsigned short data;

	ad7146->read(ad7146->dev, AD7146_PARTID_REG, &data);
	switch (data & HW_DET_MASK) {
	case AD7146_PARTID:
		ad7146->product = AD7146_PRODUCT_ID;
		ad7146->version = data & LOW_NIBBLE;
		dev_info(ad7146->dev, "found AD7146 , rev:%d\n",
			ad7146->version);
		return 0;

	default:
		dev_err(ad7146->dev,
			"ad7146 Not Found, read ID is %04x\n", data);
		return -ENODEV;
	}
}

static int read_data_from_device_tree(struct ad7146_chip *ad7146)
{
	struct device *dev = ad7146->dev;
	const char * const err_format = "failed to get \"%s\"\n";
	const char * const prop_vdd_supply_name = "prox_vdd-supply_name";
	const char * const prop_irq_gpio = "AD,irq_gpio";
	const char * const prop_stagex = "pad,stage";
	const char * const prop_stagex_connect1 = "%s%d-connect1";
	const char * const prop_stagex_connect2 = "%s%d-connect2";
	const char * const prop_stagex_afe = "%s%d-afe";
	const char * const prop_stagex_sensitivity = "%s%d-sensitivity";
	const char * const prop_stagex_offset_l = "%s%d-offset_l";
	const char * const prop_stagex_offset_h = "%s%d-offset_h";
	const char * const prop_stagex_offset_l_clamp = "%s%d-offset_l_clamp";
	const char * const prop_stagex_offset_h_clamp = "%s%d-offset_h_clamp";
	const char * const prop_stagex_hysterisis = "%s%d-hysterisis";
	const char * const prop_amb_comp_ctrlx = "pad,amb_comp_ctrl%d";
	const char * const prop_mod_freq_ctrl = "pad,mod_freq_ctrl";
	char temp_buf[TEMP_BUFER_MAX_LEN];
	int rc, cnt;
	struct ad7146_product_stgx_data *pt;

	rc = of_property_read_string(dev->of_node, prop_vdd_supply_name,
		&ad7146->vdd_supply_name);
	if (rc) {
		dev_err(dev, err_format, prop_vdd_supply_name);
		return rc;
	}

	rc = of_get_named_gpio(dev->of_node, prop_irq_gpio, 0);
	if (rc < 0) {
		dev_err(dev, err_format, prop_irq_gpio);
		return rc;
	}
	ad7146->irq_gpio = rc;

	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		pt = &ad7146->product_data.stgx_data[cnt];
		if (!pt) {
			dev_err(ad7146->dev, "%s: pt is NULL!", __func__);
			return -ENOMEM;
		}
		/* Stagex Connection1 Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_connect1, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_conn_6_0);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex Connection2 Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_connect2, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_conn_12_7);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex AFE Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_afe, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_afe_offset);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex SENSITIVITY Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_sensitivity, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_sensitivity);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex OFFSET_LOW Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_offset_l, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_offset_low);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex OFFSET_HIGH Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_offset_h, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_offset_high);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex OFFSET_HIGH_CLAMP Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_offset_l_clamp, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_offset_high_clamp);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex OFFSET_HIGH_CLAMP Register value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_offset_h_clamp, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_offset_low_clamp);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
		/* Stagex Hysterisis percentage value */
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_stagex_hysterisis, prop_stagex, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&pt->stgx_hysterisis);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
	}

	/* MOD_FREQ_CTRL Register value */
	rc = of_property_read_u32(dev->of_node,
		prop_mod_freq_ctrl, &ad7146->product_data.mod_freq_ctrl);
	if (rc < 0) {
		dev_err(dev, err_format, prop_mod_freq_ctrl);
		return rc;
	}

	/* AMB_COMP_CTRLx Register value */
	for (cnt = 0; cnt < AMB_COMP_CTRL_NUM_MAX; cnt++) {
		snprintf(temp_buf, TEMP_BUFER_MAX_LEN,
			prop_amb_comp_ctrlx, cnt);
		rc = of_property_read_u32(dev->of_node, temp_buf,
			&ad7146->product_data.amb_comp_ctrlx[cnt]);
		if (rc < 0) {
			dev_err(dev, err_format, temp_buf);
			return rc;
		}
	}
	return 0;
}

static int irq_gpio_request(struct ad7146_chip *ad7146)
{
	int rc = gpio_request(ad7146->irq_gpio, "ad7146_irq");
	if (rc)
		dev_err(ad7146->dev, "%s: failed to request gpio %d, rc=%d",
			__func__, ad7146->irq_gpio, rc);
	return rc;
}

static int vreg_turn_on(struct ad7146_chip *ad7146)
{
	const char * const vdd_supply_name = ad7146->vdd_supply_name;
	struct device *dev = ad7146->dev;
	const int max_voltage = REGULATOR_MAX_VOLTAGE;
	const int ua_load = REGULATOR_UA_LOAD;
	const int delay_ms = REGULATOR_ON_DELAY;
	int rc, enabled;

	ad7146->vreg_prox_vdd = regulator_get(dev, vdd_supply_name);
	if (IS_ERR(ad7146->vreg_prox_vdd)) {
		dev_err(dev, "%s: regulator_get failed, rc=%lu",
			vdd_supply_name, PTR_ERR(ad7146->vreg_prox_vdd));
		rc = PTR_ERR(ad7146->vreg_prox_vdd);
		goto err;
	}

	rc = regulator_set_voltage(ad7146->vreg_prox_vdd,
		max_voltage, max_voltage);
	if (rc) {
		dev_err(dev, "%s: set voltage failed, rc=%d\n",
				vdd_supply_name, rc);
		goto err_reg_put;
	}

	rc = regulator_enable(ad7146->vreg_prox_vdd);
	if (rc) {
		dev_err(dev, "%s: enabling regulator failed, rc=%d",
			vdd_supply_name, rc);
		goto err_reg_put;
	}

	rc = regulator_set_optimum_mode(ad7146->vreg_prox_vdd, ua_load);
	if (rc < 0) {
		dev_err(dev, "%s: setting optimum mode failed, rc = %d",
			vdd_supply_name, rc);
		goto err_reg_disable;
	}

	enabled = regulator_is_enabled(ad7146->vreg_prox_vdd);
	if (enabled < 0)
		dev_warn(dev, "%s: regulator_is_enabled failed, rc=%d",
			vdd_supply_name, enabled);

	msleep(delay_ms);

	dev_info(dev, "regulator '%s' (0x%lx) is%s enabled and the mode is %d",
		vdd_supply_name, PTR_ERR(ad7146->vreg_prox_vdd),
		enabled > 0 ? "" : " NOT", rc);
	return 0;

err_reg_disable:
	regulator_disable(ad7146->vreg_prox_vdd);
err_reg_put:
	regulator_put(ad7146->vreg_prox_vdd);
err:
	return rc;
}

static int vreg_turn_off(struct ad7146_chip *ad7146)
{
	const char * const vdd_supply_name = ad7146->vdd_supply_name;
	struct device *dev = ad7146->dev;
	const int rc = regulator_disable(ad7146->vreg_prox_vdd);

	if (rc)
		dev_warn(dev, "failed to disable regulator '%s', rc=%d",
			vdd_supply_name, rc);

	regulator_put(ad7146->vreg_prox_vdd);
	return rc;
}

static int ad7146_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct device *dev;
	struct ad7146_chip *ad7146 = NULL;
	int irq;
	int error;
	int cnt;
	struct ad7146_driver_data *driver_data;

	if (NULL == client) {
		printk(KERN_ERR "I2C Client doesn't exist\n");
		error = -EINVAL;
		goto err_out;
	}

	dev = &client->dev;
	irq = client->irq;
	dev_info(dev, "%s called IRQ = %d\n", __func__, irq);

	ad7146 = kzalloc(sizeof(*ad7146), GFP_KERNEL);
	if (!ad7146) {
		error = -ENOMEM;
		goto err_out;
	}
	ad7146->sw = kzalloc((sizeof(*driver_data) * STAGE_NUM),
				  GFP_KERNEL);
	if (!ad7146->sw) {
		error = -ENOMEM;
		goto err_free_chip;
	}

	ad7146->read = ad7146_i2c_read;
	ad7146->write = ad7146_i2c_write;
	ad7146->irq = irq;
	ad7146->dev = dev;

	ad7146->open_air_low = OPEN_AIR_LOW_VALUE;
	ad7146->open_air_high = OPEN_AIR_HIGH_VALUE;

	mutex_init(&ad7146->mutex);
	mutex_init(&ad7146->mutex_shadow);

	ad7146->pad_enable_state = DISABLE_AD7146;
	ad7146->i2c_err_flag = AD7146_I2C_RW_NO_ERR;
	ad7146->current_pad_no = PAD_INIT_NUM;
	ad7146->save_data_point[STG_ZERO] = AD7146_NOT_SET;
	ad7146->save_data_point[STG_ONE] = AD7146_NOT_SET;
	ad7146->sw_updata = DISABLE_AD7146;
	ad7146->fc_flag = DISABLE_AD7146;
	ad7146->keep_detect_flag = DISABLE_AD7146;

	error = read_data_from_device_tree(ad7146);
	if (error)
		goto err_free_mem;

	error = irq_gpio_request(ad7146);
	if (error)
		goto err_free_mem;

	error = vreg_turn_on(ad7146);
	if (error)
		goto err_free_mem;

	i2c_set_clientdata(client, ad7146);

	ad7146->sw_state.name = SWITCH_NAME_DEF;
	ad7146->sw_state.state = AD7146_SENS_DET;
	error = switch_dev_register(&ad7146->sw_state);
	if (error < 0)
		goto err_free_mem;

	ad7146->sw_stg0.name = SWITCH_NAME_1;
	ad7146->sw_stg0.state = AD7146_SENS_NOT_DET;
	error = switch_dev_register(&ad7146->sw_stg0);
	if (error < 0)
		goto err_switch_reg_1;

	ad7146->sw_stg1.name = SWITCH_NAME_2;
	ad7146->sw_stg1.state = AD7146_SENS_NOT_DET;
	error = switch_dev_register(&ad7146->sw_stg1);
	if (error < 0)
		goto err_switch_reg_2;

	/* check if the device is existing by reading device id of AD7146 */
	error = ad7146_hw_detect(ad7146);
	if (error)
		goto err_switch_reg;

	/* initialize and request sw/hw resources */
	INIT_WORK(&ad7146->calib_work, dac_calibration_work);
	INIT_DELAYED_WORK(&ad7146->work, switch_set_work);
	INIT_WORK(&ad7146->resume_work, resume_set_work);

	error = create_cap_sensor_class();
	if (error < 0)
		goto err_switch_reg;

	ad7146->cap_dev.name = DRIVER_NAME;
	ad7146->cap_dev.data = ad7146;
	error = cap_sensor_dev_register(&ad7146->cap_dev);
	if (error < 0)
		goto err_cap_class;

	if (irq <= 0) {
		dev_err(dev, "IRQ not configured!\n");
		error = -EINVAL;
		goto err_cap_reg;
	}
	error = request_threaded_irq(ad7146->irq, NULL, ad7146_isr,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			dev_name(dev), ad7146);
	if (error) {
		dev_err(dev, "irq %d busy?\nDriver init Failed", ad7146->irq);
		goto err_cap_reg;
	}

	error = ad7146_setup_defaults(ad7146);
	if (error) {
		dev_err(dev, "Failed to setup defaults %d.\n", error);
		goto err_cap_reg;
	}
	error = ad7146_pad_setting(ad7146, (STG0_EN_FLG | STG1_EN_FLG));
	if (error) {
		dev_err(dev, "Failed to set all pad-on %d.\n", error);
		goto err_cap_reg;
	}
	getStageInfo(ad7146);
	error = ad7146_pad_setting(ad7146, STGX_ALL_OFF);
	if (error) {
		dev_err(dev, "Failed to set all pad-off %d.\n", error);
		goto err_cap_reg;
	}
	for (cnt = 0; cnt < PAD_NUM_MAX; cnt++) {
		driver_data = &ad7146->sw[cnt];
		driver_data->state = IDLE;
		if (STG_ZERO == driver_data->index)
			switch_set_state(&ad7146->sw_stg0, AD7146_SENS_NOT_DET);
		else if (STG_ONE == driver_data->index)
			switch_set_state(&ad7146->sw_stg1, AD7146_SENS_NOT_DET);
		else
			dev_err(ad7146->dev,
				"Failed to set state of the switch device %u.\n",
				driver_data->index);
	}
	ad7146_set_switch_status(ad7146, AD7146_SENS_NOT_DET);
	ad7146->sw_updata = AD7146_SENS_NOT_DET;
	return 0;

err_cap_reg:
	cap_sensor_dev_unregister(&ad7146->cap_dev);
err_cap_class:
	class_destroy(cap_sensor_class);
err_switch_reg:
	switch_dev_unregister(&ad7146->sw_stg1);
err_switch_reg_2:
	switch_dev_unregister(&ad7146->sw_stg0);
err_switch_reg_1:
	switch_dev_unregister(&ad7146->sw_state);
err_free_mem:
	kzfree(ad7146->sw);
err_free_chip:
	kzfree(ad7146);
err_out:
	dev_err(dev, "Failed to setup ad7146 device\n");
	return -ENODEV;
}

static void ad7146_shutdown(struct i2c_client *client)
{
	struct ad7146_chip *ad7146 = i2c_get_clientdata(client);

	free_irq(ad7146->irq, ad7146);
	cancel_delayed_work(&ad7146->work);
	cancel_work_sync(&ad7146->calib_work);
	cancel_work_sync(&ad7146->resume_work);
	cap_sensor_dev_unregister(&ad7146->cap_dev);
	class_destroy(cap_sensor_class);
	switch_dev_unregister(&ad7146->sw_stg1);
	switch_dev_unregister(&ad7146->sw_stg0);
	switch_dev_unregister(&ad7146->sw_state);
	vreg_turn_off(ad7146);
	kzfree(ad7146->dac_cal_buffer);
	kzfree(ad7146->sw);
	kzfree(ad7146);
	return;
}

#ifdef CONFIG_PM
int ad7146_i2c_suspend(struct device *dev)
{
	struct ad7146_chip *ad7146 = i2c_get_clientdata(to_i2c_client(dev));
	unsigned short pwr_data;

	cancel_work_sync(&ad7146->resume_work);
	if (ad7146->pad_enable_state) {
		pwr_data = shadow_reg[PWR_CONTROL].data | PWR_MODE_SHUTDOWN;
		ad7146->write(ad7146->dev,
			shadow_reg[PWR_CONTROL].addr, pwr_data);
	}
	disable_irq(ad7146->irq);
	cancel_delayed_work(&ad7146->work);
	dev_dbg(ad7146->dev, "%s end: pad_enable_state = %x\n",
		__func__, ad7146->pad_enable_state);
	return 0;
}

int ad7146_i2c_resume(struct device *dev)
{
	struct ad7146_chip *ad7146 = i2c_get_clientdata(to_i2c_client(dev));

	dev_dbg(ad7146->dev, "%s: schedule resume work thread.\n", __func__);
	schedule_work(&ad7146->resume_work);
	return 0;
}

static SIMPLE_DEV_PM_OPS(ad7146_pm, ad7146_i2c_suspend, ad7146_i2c_resume);
#endif

static const struct i2c_device_id ad7146_id[] = {
	{ "ad7146_NORM", 0 },
	{ "ad7146_PROX", 1 },
	{ "ad7146", 2 }, {},
};
MODULE_DEVICE_TABLE(i2c, ad7146_id);

struct i2c_driver ad7146_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
#ifdef CONFIG_PM
		.pm   = &ad7146_pm,
#endif
	},
	.probe    = ad7146_probe,
	.shutdown = ad7146_shutdown,
	.id_table = ad7146_id,
};

static __init int ad7146_i2c_init(void)
{
	return i2c_add_driver(&ad7146_i2c_driver);
}
module_init(ad7146_i2c_init);

static __exit void ad7146_i2c_exit(void)
{
	i2c_del_driver(&ad7146_i2c_driver);
	return;
}
module_exit(ad7146_i2c_exit);
MODULE_DESCRIPTION("Analog Devices ad7146 Sensor Driver");
MODULE_AUTHOR("Analog Devices Inc.");
MODULE_LICENSE("GPL v2");
