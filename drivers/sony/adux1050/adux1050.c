/*
 * ADUX1050 Generic Controller Driver
 *
 * @copyright 2016 Analog Devices Inc.
 *
 * Licensed under the GPL version 2 or later.
 * date      OCT-2016
 * version   Driver 1.3.1
 * version   Linux 3.18.20 and above
 * version   Android 6.0.1 [Marshmallow]
 */

/*
 * adux1050.c
 * This file is the core driver part of ADUX1050 Capacitive sensor.
 * It also has routines for interrupt handling,
 * suspend, resume, initialization routines etc.
 *
 * ADUX1050 Generic Controller Driver
 *
 * Copyright 2016 Analog Devices Inc.
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * Licensed under the GPL version 2 or later.
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 */

#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#ifdef CONFIG_ADUX1050_POLL
#include <linux/kthread.h>
#endif
#include "adux1050.h"

#ifdef CONFIG_ADUX1050_DEBUG
#define ADUX1050_LOG(format, arg...) pr_info("[adux1050]:"format, ## arg)
#define ADUX1050_DEV_LOG(dev, format, arg...) dev_info(dev, format, ## arg)
#else
#define ADUX1050_LOG(format, arg...) pr_debug("[adux1050]:"format, ## arg)
#define ADUX1050_DEV_LOG(dev, format, arg...) dev_dbg(dev, format, ## arg)
#endif


#define CHK_AUTO_TH_ENABLED(bs_reg)	(bs_reg & AUTO_TH_MASK)


/*
 * Local platform data used if no platform data is found in board file or
 * Device tree.
 */
static struct adux1050_platform_data local_plat_data = {
	.init_regs = {
		0x0001010F,	0x00020529,	0x00030000,	0x00050F55,
		0x00069999,	0x000700e8,	0x00080200,	0x00090000,
		0x000a000C,	0x00798000,
		0x000b9999,	0x000c03e8,	0x000d0200,	0x000e0000,
		0x000f000C,	0x007a8000,
		0x00109999,	0x001105e8,	0x00120200,	0x00130000,
		0x0014000C,	0x007b8000,
		0x00159999,	0x001607e8,	0x00170200,	0x00180000,
		0x0019000C,	0x007c8000,
	},
	.req_stg0_base = 10000,
	.req_stg1_base = 20000,
	.req_stg2_base = 30000,
	.req_stg3_base = 40000,
	.is_set_irq_gpio_no = false,
	.proxy_enable = false,
	.allowed_proxy_time = 20,
	.max_proxy_count = 10,
	.intr_err = 0,
	.device_name = {"adux1050_idXX"},
};

/*
 * int adux1050_i2c_write(struct device *dev, u8 reg, u16 *data, u16 data_cnt)
 * Writes to the device register through I2C interface.
 * Used to write the data to the I2C client's Register through the i2c protocol
 * Used i2c_transfer api's for the bus transfer
 * @param dev The i2c client's device structure
 * @param reg The register address to be written
 * @param data The data buffer which holds the data to be written to the device
 * @param data_cnt The number of data to be written to the device from buffer.
 * @return Number of messages transferred, default 2
 *
 * @see adux1050_i2c_read
 */
static int adux1050_i2c_write(struct device *dev, u8 reg,
		u16 *data, u16 data_cnt)
{

	struct i2c_client *client = to_i2c_client(dev);
	u8 device_addr = client->addr;
	u16 loop_cnt = 0;
	u8 tx[(MAX_ADUX1050_WR_LEN*sizeof(short)) + 1] = {0};
	u16 *head;
	s32 ret = -EIO;
	struct i2c_msg adux1050_wr_msg = {
		.addr = device_addr,
		.buf = (u8 *)tx,
		.len = ((data_cnt*sizeof(data_cnt))+1),
		.flags = 0,
	};
	if (!data)
		return -EINVAL;
	tx[0] = reg;
	head = (unsigned short *)&tx[1];
	for (loop_cnt = 0; loop_cnt < data_cnt; loop_cnt++)
		*(head++) = cpu_to_be16(*(data++));

	for (loop_cnt = 0; loop_cnt < I2C_RETRY_CNT; loop_cnt++) {
		ret = i2c_transfer(client->adapter, &adux1050_wr_msg, 1);
		if (unlikely(ret < 1))
			dev_err(&client->dev, "I2C write error %d\n", ret);
		else
			break;
	}
	return ret;
}

/*
 * int adux1050_i2c_read(struct device *dev, u8 reg,u16 *data, u16 data_cnt)
 * This is used to read the data from the ADUX1050's register through
 * I2C interface
 * This function uses i2c protocol and its api's to read data from register
 * @param dev The i2c client device Structure.
 * @param reg The register address to be read.
 * @param data The buffer's pointer to store the register's value.
 * @param data_cnt The number of registers to be read.
 * @return The number of messages transferred as an integer
 *
 * @see adux1050_i2c_write
 */
static int adux1050_i2c_read(struct device *dev, u8 reg,
			     u16 *data, u16 data_cnt)
{
	struct i2c_client *client = to_i2c_client(dev);
	u16 loop_cnt = 0;
	u16 rx[MAX_ADUX1050_WR_LEN] = {};
	s8  device_addr = client->addr;
	s32 ret = 0;
	struct i2c_msg adux1050_rd_msg[I2C_WRMSG_LEN] = {
			{
				.addr = device_addr,
				.buf = (u8 *)&reg,
				.len = sizeof(reg),
				.flags = 0,
			},
			{
				.addr = device_addr,
				.buf = (u8 *)rx,
				.len = data_cnt * sizeof(short),
				.flags = I2C_M_RD,
			}
	};
	for (loop_cnt = 0; loop_cnt < I2C_RETRY_CNT; loop_cnt++) {
		ret = i2c_transfer(client->adapter, adux1050_rd_msg,
				   I2C_WRMSG_LEN);
		if (unlikely(ret < I2C_WRMSG_LEN)) {
			dev_err(dev, "[ADUX1050]: I2C READ error %d\n", ret);
			if (loop_cnt >= (I2C_RETRY_CNT - 1))
				memset(data, 0, data_cnt * sizeof(short));
		} else {
			for (loop_cnt = 0; loop_cnt < data_cnt; loop_cnt++)
				data[loop_cnt] = be16_to_cpu(rx[loop_cnt]);
			break;
		}
	}
	return ret;
}

/*
 * inline u16 set_dac_offset(s16 new_offset_value)
 * Function to set the DAC positive and negative offset based on given offset
 * @param new_offset_value value to be set as the offset.
 * @return Combined +ve and -ve value to be set to the DAC_OFFSET_STGx register
 */
static inline u16 set_dac_offset(s16 new_offset_value)
{
	u16 offset_val = 0;

	if (new_offset_value >= 0)
		offset_val = ST_POS_DAC_OFFSET(new_offset_value);
	else
		offset_val = ST_NEG_DAC_OFFSET(new_offset_value);

	return offset_val;
}

/*
 * static inline s16 set_swap_state(struct adux1050_chip *adux1050, u16 stg_num,
 *	u16 *swap_state, s16 curr_val)
 * Function to set the swap bits based on the calibration DAC offset
 * @param adux1050 chip structure of ADUX1050 driver.
 * @param stg_num The stage to which swap is to be done for DAC offset value.
 * @param swap_state The swap state as set in control register
 *	[ctrl reg holds the swap state]
 * @param curr_val Current value of DAC_offest for the stage provided [stg_num]
 * @return Zero on success.
 */
static inline s16 set_swap_state(struct adux1050_chip *adux1050, u16 stg_num,
		   u16 *swap_state, s16 curr_val) {
	s16 err = 0;

	if (!swap_state)
		return -EINVAL;
	if (curr_val > (MAX_OFFSET/2)) {
		*swap_state = ((*swap_state & CLR_POS_SWAP) | SET_NEG_SWAP);
		err = adux1050->write(adux1050->dev, GET_CONFIG_REG(stg_num),
				swap_state, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Error in FC %d\n", err);
			return err;
		}
		dev_dbg(adux1050->dev, "%s - Swap set %x",
			__func__, *swap_state);
	} else if (curr_val < -(MAX_OFFSET/2)) {
		*swap_state = ((*swap_state & CLR_NEG_SWAP) | SET_POS_SWAP);
		err = adux1050->write(adux1050->dev, GET_CONFIG_REG(stg_num),
				      swap_state, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Error in FC %d\n", err);
			return err;
		}
		dev_dbg(adux1050->dev, "%s - Swap set %x",
			__func__, *swap_state);
	} else {
		dev_dbg(adux1050->dev, "%s - No Swap to be set", __func__);
		*swap_state = ((*swap_state & CLR_NEG_SWAP) & CLR_POS_SWAP);
		err = adux1050->write(adux1050->dev, GET_CONFIG_REG(stg_num),
				swap_state, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Error in FC %d\n", err);
			return err;
		}
	}
	return err;
}

/*
 * short adux1050_force_cal(struct adux1050_chip *adux1050,int cal_time)
 * Internal function to perform force calibration of the Chip.
 * @param adux1050 The chip structure of adux1050 driver
 * @param cal_time Sleep time required after the force calibration.
 * @return 0 on success and -1 on error
 */
static inline s16 adux1050_force_cal(struct adux1050_chip *adux1050,
				      int cal_time)
{
	u16 data = 0;
	s16 err = 0;

	err = adux1050->read(adux1050->dev, BASELINE_CTRL_REG, &data, DEF_WR);
	if (err <  I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Error in FC %d\n", err);
		return -EIO;
	}
	data = data | FORCE_CAL_MASK;
	err = adux1050->write(adux1050->dev, BASELINE_CTRL_REG, &data, DEF_WR);
	if (err < DEF_WR) {
		dev_err(adux1050->dev, "I2C WR Error in FC %d\n", err);
		return -EIO;
	}
	if (cal_time != 0) {
		dev_dbg(adux1050->dev, " sleep time in FC:%d\n", cal_time);
		msleep(cal_time);
	}
	return err;
}

/*
 * inline s16 get_conv_time(struct adux1050_chip *adux1050, int mul_flag)
 * To get the required conversion time for the current seting
 * @param adux1050 Chip structure.
 * @param mul_flag Multiplier flag
 * @return Returns the conversion time required for an updated configuration
 */
static inline s16 get_conv_time(struct adux1050_chip *adux1050, int mul_flag)
{
	u16 pwr_ctrl_reg = 0;
	u16 cv_time_ctrl = 0;
	u16 pwr_mode = 0;
	u16 stg_num = 0;
	u16 delay_in_ctoc = 0;
	u16 avg = 0;
	u16 osr = 0;
	u16 phase = 0;
	u16 base_time = 0;
	u16 temp_base_time = 0;
	u16 stg_cfg = 0;
	u16 lp_cnt = 0;
	s16 err = 0;

	err = adux1050->read(adux1050->dev, CTRL_REG, &pwr_ctrl_reg, DEF_WR);
	if (err < I2C_WRMSG_LEN)
		dev_err(adux1050->dev, "i2c RD Err %d\n", err);
	err = adux1050->read(adux1050->dev, CONV_TIME_CTRL_REG, &cv_time_ctrl,
			DEF_WR);
	if (err < I2C_WRMSG_LEN)
		dev_err(adux1050->dev, "i2c RD Err %d\n", err);
	pwr_mode = GET_PWR_MODE(pwr_ctrl_reg);
	stg_num = GET_NUM_STG(pwr_ctrl_reg);
	adux1050->tot_stg = stg_num;

	if (pwr_mode == PWR_STAND_BY) {
		dev_dbg(adux1050->dev, "Device in Standby mode\n");
		return 1;
	} else if (pwr_mode == PWR_FULL_POWER) {
		delay_in_ctoc = ZERO_VAL;
	} else if (pwr_mode == PWR_TIMED_CONV) {
		delay_in_ctoc = GET_TIMED_CONV_TIME(pwr_ctrl_reg);
	} else if (pwr_mode == PWR_AUTO_WAKE) {
		delay_in_ctoc = GET_TIMED_CONV_TIME(pwr_ctrl_reg);
		if (delay_in_ctoc < GET_AUTO_WAKE_TIME(pwr_ctrl_reg))
			delay_in_ctoc = GET_AUTO_WAKE_TIME(pwr_ctrl_reg);
	}
	avg = GET_AVG_CONV(cv_time_ctrl);
	avg = CALC_AVG_CONV(avg); /*AVG based multiplication factor*/
	osr = GET_OSR_CONV(cv_time_ctrl);
	osr = CALC_OSR_CONV(osr); /*OSR multipling factor*/
	phase = GET_CONV_TIME(cv_time_ctrl); /*Phase timing factor*/
	/* Calculate base time as a factor of phase */
	temp_base_time = ((phase + (phase / DECIMAL_BASE)) - 3) / 2;
	dev_dbg(adux1050->dev,	"%s, temp_Basetime(%d), avg (%d), OSR (%d)\n",
		__func__, temp_base_time, avg, osr);
	temp_base_time *= (avg * osr); /*Set the base_time*/
	for (; lp_cnt < stg_num; lp_cnt++) {
		err = adux1050->read(adux1050->dev, GET_CONFIG_REG(lp_cnt),
				     &stg_cfg, DEF_WR);
		if (err < I2C_WRMSG_LEN)
			dev_err(adux1050->dev, "i2c RD Err %d\n", err);
		if (IS_NOISE_MEASURE_EN(stg_cfg))
			base_time += (temp_base_time *
				      GET_NOISE_SAMPLE(cv_time_ctrl));
		else
			base_time += temp_base_time;
	}
	if (mul_flag == TWICE_CONV_DELAY_TIME) {
		base_time += base_time; /*Return twice the conv time*/
		base_time += delay_in_ctoc; /*Add the timed conv delay*/
	} else if (mul_flag == CONV_DELAY_TIME) {
		base_time += delay_in_ctoc;
	}
	dev_dbg(adux1050->dev,	"%s, Basetime(%d), delay_in_ctoc (%d)\n",
		 __func__, base_time, delay_in_ctoc);

	return base_time;
}

/*
 * int get_intr_mask_info(struct adux1050_chip *adux1050)
 * This function is used to get interrupt mask information.
 * @param adux1050 The chip structure of ADUX1050 driver
 * @return 0 on success.
 */
static int get_intr_mask_info(struct adux1050_chip *adux1050)
{
	u16 temp_reg_val = 0;
	s16 err = 0;

	/* Checking whether Conversion complete interrupt is enabled or not */
	err = adux1050->read(adux1050->dev, INT_CTRL_REG,
			     &temp_reg_val, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		return err;
	}
	adux1050->conv_enable = CHECK_CONV_EN(temp_reg_val);
	ADUX1050_LOG("%s - Checking conv_enable %d temp_reg_val %x\n",
			    __func__, adux1050->conv_enable, temp_reg_val);
	/* Checking whether High threshold interrupt is enabled or not */
	adux1050->high_thresh_enable = CHECK_THRESH_HIGH_EN(temp_reg_val);

	/* Checking whether Low threshold interrupt is enabled or not */
	adux1050->low_thresh_enable = CHECK_THRESH_LOW_EN(temp_reg_val);
	return 0;
}

/*
 * int getstageinfo(struct adux1050_chip *adux1050)
 * This function is used to get the current stage information.
 * @param adux1050 The chip structure of ADUX1050 driver
 * @return 0 on success.
 */
static int getstageinfo(struct adux1050_chip *adux1050)
{
	u16 temp_reg_val = 0;
	s16 err = 0;
	u8 stg_cnt = 0;
	u8 cin_cnt = 0;
	u8 temp_cin;

	adux1050->conn_stg_cnt = 0;

	/* How many stages to measure CDC */
	err = adux1050->read(adux1050->dev, CTRL_REG, &temp_reg_val, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		return err;
	}
	adux1050->num_stages = GET_NUM_STG(temp_reg_val);
	/* Find whether stage is connected(either +ve or -ve) or not */
	for (stg_cnt = 0 ; stg_cnt < TOTAL_STG ; stg_cnt++) {
		if (stg_cnt < adux1050->num_stages) {
			err = adux1050->read(adux1050->dev,
					     GET_CONFIG_REG(stg_cnt),
					     &temp_reg_val, DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d in %s\n",
					err, __FILE__);
				return err;
			}

			adux1050->stg_info[stg_cnt].status = CIN_NOT_CONNECTED;
			for (cin_cnt = 0; cin_cnt < TOTAL_CIN ; cin_cnt++) {
				temp_cin = (temp_reg_val) & 3;
				if ((temp_cin == CIN_NEG_INPUT) ||
				    (temp_cin == CIN_POS_INPUT)) {
					adux1050->stg_info[stg_cnt].status =
							CIN_CONNECTED;
					adux1050->conn_stg_cnt++;
					dev_dbg(adux1050->dev,
						 "STG CONNECTED %d,tot=%d\n",
						 stg_cnt,
						 adux1050->conn_stg_cnt);
					break;
				}
				temp_reg_val = temp_reg_val >> 2;
			}
		} else {
			adux1050->stg_info[stg_cnt].status = CIN_NOT_CONNECTED;
			pr_debug("%s - CDC not configured for STG_%d\n",
				__func__, stg_cnt);
		}
	}
	return 0;
}

/*
 * int update_calib_settings(struct adux1050_chip *adux1050, u16 total_stg,
				u16 *data, bool write_to_reg, u8 file_exist)
 * This function updates the calibration output to local register array and
	registers of ADUX1050.
 * @param adux1050 The chip structure of ADUX1050 driver
 * @param total_stg Number of stages for which calib settings to be updated
 * @param *data Pointer to buffer which contains the calib output
 * @param write_to_reg A Flag to specify whether to write to registers or not
 * @param file_exist A Flag to update the calib status
 * @return 0 on success
 */
inline int update_calib_settings(struct adux1050_chip *adux1050, u16 total_stg,
				u16 *data, bool write_to_reg, u8 file_exist)
{
	u16 temp_baseline_ctrl = 0;
	u16 int_ctrl_reg = 0;
	u16 stg_cnt = 0;
	u16 stg_num = 0;
	u8 fp = 0;
	u16 cal_base_fail_flag = 0;
	u16 value = 0;
	u16 hys_reg = 0;
	s16 err = 0;


	/* Disable the interrupt	*/
	err = adux1050->read(adux1050->dev, INT_CTRL_REG,
			     &int_ctrl_reg, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		goto err_calib;
	}
	value = int_ctrl_reg | DISABLE_DEV_INT;
	err = adux1050->write(adux1050->dev, INT_CTRL_REG, &value, DEF_WR);
	if (err < DEF_WR) {
		dev_err(adux1050->dev, "I2C WR Err %d in %s\n", err, __FILE__);
		goto err_calib;
	}

	/* Disabling the Auto threhold & Force calib to */
	/* update baseline registers */
	err = adux1050->read(adux1050->dev, BASELINE_CTRL_REG,
		       &temp_baseline_ctrl, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		goto err_calib;
	}
	temp_baseline_ctrl = temp_baseline_ctrl & ANTI_FORCE_CAL_MASK;
	if (temp_baseline_ctrl & AUTO_TH_MASK) {
		value = temp_baseline_ctrl;
		value = value & (~AUTO_TH_MASK);
		err = adux1050->write(adux1050->dev, BASELINE_CTRL_REG,
				&value, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			goto err_calib;
		}
	}
	msleep(adux1050->slp_time_conv_complete);

	for (stg_cnt = 0; stg_cnt < total_stg; stg_cnt++) {
		stg_num = data[fp++];
		if ((stg_num >= STG_ZERO) && (stg_num <= STG_THREE)) {

			adux1050->pdata->cal_fact_base[stg_num] = data[fp++];
			adux1050->pdata->cal_offset[stg_num] = data[fp++];
			adux1050->pdata->digi_offset[stg_num] = data[fp++];
			adux1050->pdata->stg_cfg[stg_num] = data[fp++];

			adux1050->bs_reg[stg_num].wr_flag = ADUX1050_ENABLE;
			adux1050->bs_reg[stg_num].value =
				adux1050->pdata->cal_fact_base[stg_num];

			adux1050->reg[GET_OFFSET_REG(stg_num)].wr_flag =
							ADUX1050_ENABLE;
			adux1050->reg[GET_OFFSET_REG(stg_num)].value =
				adux1050->pdata->cal_offset[stg_num];

			adux1050->reg[GET_HYS_REG(stg_num)].wr_flag =
							ADUX1050_ENABLE;
			hys_reg =
			((adux1050->reg[GET_HYS_REG(stg_num)].value) &
			 HYS_BYTE_MASK) |
			 (adux1050->pdata->digi_offset[stg_num] << 8);
			adux1050->reg[GET_HYS_REG(stg_num)].value = hys_reg;

			adux1050->reg[GET_CONFIG_REG(stg_num)].wr_flag =
							ADUX1050_ENABLE;
			adux1050->reg[GET_CONFIG_REG(stg_num)].value =
				adux1050->pdata->stg_cfg[stg_num];
			cal_base_fail_flag |=
				adux1050->pdata->cal_fact_base[stg_num];

			if (write_to_reg == ADUX1050_ENABLE) {
				err = adux1050->write(
				adux1050->dev, GET_BASE_LINE_REG(stg_cnt),
				&adux1050->pdata->cal_fact_base[stg_cnt],
				DEF_WR);
				if (err < DEF_WR) {
					dev_err(adux1050->dev,
						"I2C WR Err %d in %s\n",
						err, __FILE__);
					goto err_calib;
				}

				err = adux1050->write(
				adux1050->dev, GET_OFFSET_REG(stg_cnt),
				&adux1050->pdata->cal_offset[stg_cnt], DEF_WR);
				if (err < DEF_WR) {
					dev_err(adux1050->dev,
						"I2C WR Err %d in %s\n",
						err, __FILE__);
					goto err_calib;
				}

				err = adux1050->read(adux1050->dev,
					       GET_HYS_REG(stg_cnt),
					       &hys_reg, DEF_WR);
				if (err < I2C_WRMSG_LEN) {
					dev_err(adux1050->dev,
						"I2C RD Err %d in %s\n",
						err, __FILE__);
					goto err_calib;
				}

				hys_reg =
				((hys_reg & HYS_BYTE_MASK) |
				 (adux1050->pdata->digi_offset[stg_cnt] << 8));
				err = adux1050->write(adux1050->dev,
						GET_HYS_REG(stg_cnt),
						&hys_reg, DEF_WR);
				if (err < DEF_WR) {
					dev_err(adux1050->dev,
						"I2C WR Err %d in %s\n",
						err, __FILE__);
					goto err_calib;
				}

				err = adux1050->write(
				adux1050->dev, GET_CONFIG_REG(stg_cnt),
				&adux1050->pdata->stg_cfg[stg_cnt], DEF_WR);
				if (err < DEF_WR) {
					dev_err(adux1050->dev,
						"I2C WR Err %d in %s\n",
						err, __FILE__);
						goto err_calib;
				}
			}

		} else {
			dev_err(adux1050->dev,
				"%s Invalid Stg num in FILP or SYSFS input\n",
				__func__);
			break;
		}
	}

	pr_debug("Calib status = %d\n", adux1050->dac_calib.cal_flags);
	/*Restoring the Auto threshold mode if enabled previously*/
	if (temp_baseline_ctrl & AUTO_TH_MASK) {
		err = adux1050->write(adux1050->dev, BASELINE_CTRL_REG,
				&temp_baseline_ctrl, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			goto err_calib;
		}
	}
	/* Reenable the interrupt */
	err = adux1050->write(adux1050->dev, INT_CTRL_REG,
			      &int_ctrl_reg, DEF_WR);
	if (err < DEF_WR) {
		dev_err(adux1050->dev, "I2C WR Err %d in %s\n", err, __FILE__);
		goto err_calib;
	}

	if (cal_base_fail_flag != 0) {
		adux1050->dac_calib.cal_flags = CAL_RET_SUCCESS;
	#ifdef CONFIG_ADUX1050_EVAL
		err = get_intr_mask_info(adux1050);
		/* Getting the stage info */
		err = getstageinfo(adux1050);
		if (err < ZERO_VAL) {
			dev_err(adux1050->dev, "Error in getstageinfo %d\n",
				err);
			goto err_calib;
		}
	#endif
	} else {
		if (file_exist)
			adux1050->dac_calib.cal_flags = CAL_RET_EXIST;
		else
			adux1050->dac_calib.cal_flags = CAL_RET_NONE;
		err = -EIO;
	}
	return err;
err_calib:
	if (file_exist)
		adux1050->dac_calib.cal_flags = CAL_RET_EXIST;
	else
		adux1050->dac_calib.cal_flags = CAL_RET_NONE;
	err = -EIO;
	return err;

}


/*
 * int adux1050_store_register_values(struct adux1050_chip *adux1050)
 * This is to retrieve the register values from either device tree/local
 * platform data and store it in local array
 * @param  adux1050 The Device structure
 * @return Zero on success
 */
static int adux1050_store_register_values(struct adux1050_chip *adux1050)
{
	u32 lcnt = 0;
	u32 data_cnt = 0;
	const u32 *init_buffer = NULL;
	const __be32 *df_regs = NULL;
	u32 df_prop_length = 0;
#ifdef	CONFIG_OF

	u32 len;
	const __be32 *property = NULL;
#endif
	u8 of_reg_found = false;

#ifdef	CONFIG_OF

	/* Fetching data from Device tree */
	if (adux1050->dt_device_node) {
		df_regs = of_get_property(adux1050->dt_device_node,
					  "adi,adux1050_reg", &df_prop_length);
		/* Fetching required baseline value for STG 0 */
		property = of_get_property(adux1050->dt_device_node,
					   "adi,adux1050_stg0_base", &len);
		if (property && len == sizeof(int)) {
			adux1050->pdata->req_stg0_base = be32_to_cpu(*property);
			dev_dbg(adux1050->dev, "valid req_base on %s\n",
				 adux1050->dt_device_node->full_name);
		}
		/* Fetching required baseline value for STG 1 */
		property = of_get_property(adux1050->dt_device_node,
					   "adi,adux1050_stg1_base", &len);
		if (property && len == sizeof(int)) {
			adux1050->pdata->req_stg1_base = be32_to_cpu(*property);
			dev_dbg(adux1050->dev, "valid req_base1 on %s\n",
				 adux1050->dt_device_node->full_name);
		}
		/* Fetching required baseline value for STG 2 */
		property = of_get_property(adux1050->dt_device_node,
					   "adi,adux1050_stg2_base", &len);
		if (property && len == sizeof(int)) {
			adux1050->pdata->req_stg2_base = be32_to_cpu(*property);
			dev_dbg(adux1050->dev, "valid req_base2 on %s\n",
				 adux1050->dt_device_node->full_name);
		}
		/* Fetching required baseline value for STG 3 */
		property = of_get_property(adux1050->dt_device_node,
					   "adi,adux1050_stg3_base", &len);
		if (property && len == sizeof(int)) {
			adux1050->pdata->req_stg3_base = be32_to_cpu(*property);
			dev_dbg(adux1050->dev, "valid req_base3 on %s\n",
				 adux1050->dt_device_node->full_name);
		}
	}
#endif
	if ((adux1050->pdata->req_stg0_base > MAX_CALIB_TARGET) ||
	    (adux1050->pdata->req_stg0_base < MIN_CALIB_TARGET))
		adux1050->pdata->req_stg0_base = HALF_SCALE_VAL;

	if ((adux1050->pdata->req_stg1_base > MAX_CALIB_TARGET) ||
	    (adux1050->pdata->req_stg1_base < MIN_CALIB_TARGET))
		adux1050->pdata->req_stg1_base = HALF_SCALE_VAL;

	if ((adux1050->pdata->req_stg2_base > MAX_CALIB_TARGET) ||
	    (adux1050->pdata->req_stg2_base < MIN_CALIB_TARGET))
		adux1050->pdata->req_stg2_base = HALF_SCALE_VAL;

	if ((adux1050->pdata->req_stg3_base > MAX_CALIB_TARGET) ||
	    (adux1050->pdata->req_stg3_base < MIN_CALIB_TARGET))
		adux1050->pdata->req_stg3_base = HALF_SCALE_VAL;

	/* Data from either DT or initial platform data */
	if ((!df_regs) || (df_prop_length % sizeof(u32))) {
		if (df_prop_length % sizeof(u32))
			dev_err(adux1050->dev, "[ADUX1050]: Malformed prop regs\n");
		init_buffer = adux1050->pdata->init_regs;
		data_cnt = sizeof(adux1050->pdata->init_regs)/sizeof(int);
	} else {
		init_buffer = df_regs;
		data_cnt = df_prop_length / sizeof(u32);
		of_reg_found = true;
	}
	/* Setting enable for INT_CTRL register */
	adux1050->reg[INT_CTRL_REG].wr_flag = ADUX1050_ENABLE;

	for (lcnt = 0; lcnt < data_cnt; lcnt++) {
		u8 addr;
		u16 value;
		/* getting the address and the value to be written */
		if (likely(of_reg_found)) {
			addr = (u8)((be32_to_cpu(init_buffer[lcnt])
						 & ADDR_MASK) >> HEX_BASE);
			value = (u16)(be32_to_cpu(init_buffer[lcnt])
						 & DATA_MASK);
		} else {
			addr = (u8)((init_buffer[lcnt] & ADDR_MASK)
						>> HEX_BASE);
			value = (u16)(init_buffer[lcnt] & DATA_MASK);
		}
		/* Having a copy of device tree values in driver */
		if ((addr >= DEV_ID_REG) &&
		    (addr <= HIGHEST_WR_ACCESS)) {
			adux1050->reg[addr].wr_flag = ADUX1050_ENABLE;
			adux1050->reg[addr].value = value;
		/*	pr_debug("!!!!!!! ADDR - %x ; VALUE - %x !!!!\n", */
		/*		addr, adux1050->reg[addr].value);*/
		}
		/* Copy of Baseline registers */
		if ((addr >= BASELINE_STG0_REG) &&
		    (addr <= BASELINE_STG3_REG)) {
			switch (addr) {
			case BASELINE_STG0_REG:
				adux1050->bs_reg[STG_ZERO].wr_flag =
							ADUX1050_ENABLE;
				adux1050->bs_reg[STG_ZERO].value = value;
				break;
			case BASELINE_STG1_REG:
				adux1050->bs_reg[STG_ONE].wr_flag =
							ADUX1050_ENABLE;
				adux1050->bs_reg[STG_ONE].value = value;
				break;
			case BASELINE_STG2_REG:
				adux1050->bs_reg[STG_TWO].wr_flag =
							ADUX1050_ENABLE;
				adux1050->bs_reg[STG_TWO].value = value;
				break;
			case BASELINE_STG3_REG:
				adux1050->bs_reg[STG_THREE].wr_flag =
							ADUX1050_ENABLE;
				adux1050->bs_reg[STG_THREE].value = value;
			}
		}
	}

	return 0;
}

static void high_threshold_int_check(struct adux1050_chip *adux1050,
					    u16 high_status_change);

/* if all RESULT_STGx is under BASELINE_STGx, do FORCE_CAL */
static void adjust_baseline_after_enabled(struct adux1050_chip *adux1050)
{
	u8 stg_cnt = 0;
	u16 result_cdc;
	u16 baseline_cdc;
	s16 err = 0;
	bool do_force_cal = true;

	for (stg_cnt = 0; stg_cnt < adux1050->num_stages; stg_cnt++) {
		/* Fetch the CDC only if that stage is connected */
		if (adux1050->stg_info[stg_cnt].status != CIN_CONNECTED)
			continue;
		err = adux1050->read(adux1050->dev, GET_RESULT_REG(stg_cnt),
					&result_cdc, DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C Read Error %d in %s",
					err, __func__);
		ADUX1050_LOG("RESULT_STG%d: 0x%04x\n", stg_cnt, result_cdc);
		err = adux1050->read(adux1050->dev, GET_BASE_LINE_REG(stg_cnt),
					&baseline_cdc, DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C Read Error %d in %s",
					err, __func__);
		if (result_cdc > baseline_cdc)
			do_force_cal = false;
	}
	if (do_force_cal)
		adux1050_force_cal(adux1050, adux1050->slp_time_conv_complete);
}

/*
 * int adux1050_hw_init(struct adux1050_chip *adux1050)
 * To initialize the ADUX1050 device with register set defined in
 *	platform file or device tree
 * @param  adux1050 The Device structure
 * @return Zero on success
 */
static int adux1050_hw_init(struct adux1050_chip *adux1050)
{
	u32 lcnt = 0;
	u16 addr;
	u16 slp_time = 0;
	u16 temp_baseline_ctrl = 0;
	u16 pwr_ctrl_buff = 0;
	u16 temp_reg_value = 0;
	u16 value = 0;
	s16 err = 0;

	for (lcnt = 0; lcnt < (GLOBAL_REG_CNT + STG_CNF_CNT); lcnt++) {
		addr = lcnt;
		if (adux1050->reg[addr].wr_flag == ADUX1050_ENABLE) {
			value = adux1050->reg[addr].value;
			if (addr == BASELINE_CTRL_REG) {
				value = value & ANTI_FORCE_CAL_MASK;
				if (value & AUTO_TH_MASK) {
					temp_baseline_ctrl = value;
					value = value & (~AUTO_TH_MASK);
				}
			} else if (addr == CTRL_REG) {
				value = value & ~RESET_MASK;
				pwr_ctrl_buff = value;
				value = SET_PWR_MODE(value, PWR_STAND_BY);
			} else if (addr == INT_CTRL_REG) {
				const u16 mask_cdc = 0x0100;

				if (adux1050->pdata->mask_cdc_interrupt)
					value |= mask_cdc;
				else
					value &= ~mask_cdc;
				if (adux1050->int_pol == ACTIVE_HIGH)
					adux1050->int_ctrl =
						(value | ACTIVE_HIGH);
				else
					adux1050->int_ctrl =
						(value & ~ACTIVE_HIGH);
				value = adux1050->int_ctrl | DISABLE_DEV_INT;
			}
			ADUX1050_LOG("Addr 0x%02x Val 0x%04x\n", addr, value);
			err = adux1050->write(adux1050->dev, addr,
					      &value, DEF_WR);
			if (err < DEF_WR) {
				dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
					err, __FILE__);
				return err;
			}
		}
	}
	/* Baseline registers update */
	if (adux1050->bs_reg[STG_ZERO].wr_flag == ADUX1050_ENABLE) {
		err = adux1050->write(adux1050->dev, BASELINE_STG0_REG,
				&adux1050->bs_reg[STG_ZERO].value,
				DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
	}
	if (adux1050->bs_reg[STG_ONE].wr_flag == ADUX1050_ENABLE) {
		err = adux1050->write(adux1050->dev, BASELINE_STG1_REG,
				&adux1050->bs_reg[STG_ONE].value,
				DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
	}
	if (adux1050->bs_reg[STG_TWO].wr_flag == ADUX1050_ENABLE) {
		err = adux1050->write(adux1050->dev, BASELINE_STG2_REG,
				&adux1050->bs_reg[STG_TWO].value,
				DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
	}
	if (adux1050->bs_reg[STG_THREE].wr_flag == ADUX1050_ENABLE) {
		err = adux1050->write(adux1050->dev, BASELINE_STG3_REG,
				&adux1050->bs_reg[STG_THREE].value,
				DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
	}
	/* Restoring the power mode given in configuration */
	if (pwr_ctrl_buff) {
		err = adux1050->write(adux1050->dev, CTRL_REG,
				&pwr_ctrl_buff, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
		slp_time = get_conv_time(adux1050, CONV_TIME);
		if (slp_time > 0)
			msleep(slp_time);
		ADUX1050_LOG("Addr %x New Val %x\n", CTRL_REG, pwr_ctrl_buff);
	}
	/* Auto threshold enable  */
	if (temp_baseline_ctrl) {
		err = adux1050->write(adux1050->dev, BASELINE_CTRL_REG,
				&temp_baseline_ctrl, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			return err;
		}
		ADUX1050_LOG("Addr %x New Val %x\n",
				    BASELINE_CTRL_REG,
				    temp_baseline_ctrl);
	}
	/* Clearing the device interrupt STATUS register */
	err = adux1050->read(adux1050->dev, INT_STATUS_REG,
			     &temp_reg_value, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		return err;
	}
	adux1050->prev_low_status = GET_LOW_STATUS(temp_reg_value);
	adux1050->high_status =
	adux1050->prev_high_status = GET_HIGH_STATUS(temp_reg_value);

	/* Report initial state before enabling interrupt */
	/* Note: high_thresh_enable is not set until we call getstageinfo() */
	adux1050->high_thresh_enable = CHECK_THRESH_HIGH_EN(adux1050->int_ctrl);
	if (adux1050->high_thresh_enable) {
		const u16 high_status_change = adux1050->high_thresh_enable;

		high_threshold_int_check(adux1050, high_status_change);
		ADUX1050_LOG("Report Initial High State\n");
	}

	/* Getting the stage info */
	err = getstageinfo(adux1050);
	if (err < ZERO_VAL) {
		dev_err(adux1050->dev, "Err in getstageinfo %d in %s\n",
			err, __FILE__);
		return err;
	}

	/* Storing the sleeping time required for this configuration */
	adux1050->slp_time_conv_complete = get_conv_time(adux1050,
							TWICE_CONV_DELAY_TIME);
	dev_dbg(adux1050->dev, " CONV TIME: %d\n",
		 adux1050->slp_time_conv_complete);

	/* if all RESULT_STGx is under BASELINE_STGx, do FORCE_CAL */
	adjust_baseline_after_enabled(adux1050);

	/* Enabling the device interrupt */
	err = adux1050->write(adux1050->dev, INT_CTRL_REG,
			&adux1050->int_ctrl, DEF_WR);
	if (err < DEF_WR) {
		dev_err(adux1050->dev, "I2C WR Err %d in %s\n", err, __FILE__);
		return err;
	}
	ADUX1050_LOG("Addr 0x%02x New Val 0x%04x\n",
			    INT_CTRL_REG, adux1050->int_ctrl);

	err = get_intr_mask_info(adux1050);
	if (err < ZERO_VAL) {
		dev_err(adux1050->dev, "Err in get_intr_mask_info %d in %s\n",
			err, __FILE__);
		return err;
	}

	return 0;
}

/*
 * offset_write(struct adux1050_chip *adux1050, u16 stg_num, u16 data,
			u16 slp_time)
 * Internal function used to write the offset of the Stages
		 with a predeterminded delay
 * @param adux1050 ADUX chip structure
 * @param stg_num The stage to which offset has to be written
 * @param data The Value to be written to the offset register
 * @param slp_time Sleep time to given after writing the offset register
 * @return write status is returned
 */
static int offset_write(struct adux1050_chip *adux1050, u16 stg_num, u16 data,
			u16 slp_time)
{
	s32 ret = 0;

	ret = adux1050->write(adux1050->dev, GET_OFFSET_REG(stg_num), &data,
				DEF_WR);
	if (ret < DEF_WR) {
		dev_err(adux1050->dev, "I2C WR Err %d in %s\n", ret, __FILE__);
		return ret;
	}
	if (likely(slp_time))
		msleep(slp_time);
	return ret;
}
/*
 * inline s16 get_calc_dac_offset(u16 offset, u16 stg_cfg_reg)
 * To calculate effective DAC offset value from pos & neg dac offset
 * @param offset The current offset available.
 * @param stg_cfg_reg The current stage configuration.
 * @return the equivalant offset based on the swap bits.
 */
static inline s16 get_calc_dac_offset(u16 offset, u16 stg_cfg_reg)
{
	s16 cal_offset = 0;

	cal_offset = LD_POS_DAC_OFFSET(offset, stg_cfg_reg) +
		     LD_NEG_DAC_OFFSET(offset, stg_cfg_reg);
	return cal_offset;
}

/*
 * inline int set_calc_dac_offset(struct adux1050_chip *adux1050,
			      s16 cal_offset, u16 stg_num, u16 *stg_cfg_reg,
			      u16 *offset, u16 sleep_time)
 * Helper function to calculate the DAC offset for a stage.
 * @param adux1050 Chip structure.
 * @param cal_offset Calculated equalized offset
 * @param stg_num Stage number
 * @param *stg_cfg_reg Stage configuration register value.
 * @param *offset Current offset set.
 * @param sleep_time Sleep time required for register result cdc.
 * @return Offset to be set to the register.
 */
static inline int set_calc_dac_offset(struct adux1050_chip *adux1050,
			      s16 cal_offset, u16 stg_num, u16 *stg_cfg_reg,
			      u16 *offset, u16 sleep_time)
{
	s16 err = 0;

	if ((-MAX_OFFSET > cal_offset) || (cal_offset > MAX_OFFSET)) {
		dev_err(adux1050->dev, "[ADUX1050]: %s, offset ERROR(%d)\n",
			__func__, cal_offset);
		return -EINVAL;
	}
	*offset = set_dac_offset(cal_offset);
	err = set_swap_state(adux1050, stg_num, stg_cfg_reg, cal_offset);
	if (err < 0)
		return -EIO;
	dev_dbg(adux1050->dev, "--->> %s, cal_off(%d)offset(%x) swap(%x)\n",
			__func__, cal_offset, *offset, *stg_cfg_reg);
	return offset_write(adux1050, stg_num, *offset, sleep_time);
}

/*
 * static int adux1050_offset_check(struct adux1050_chip *adux1050,
 *				u16 max_cnt, s16 dir_flag, u16 slp_time)
 * ADUX1050 Saturation routine for bringing the device out of saturation using
		directional hopping method
 * @param adux1050 The device structure to be calibrated
 * @param max_cnt	Maximum count
 * @param dir_flag	Direction flag
 * @param slp_time	Sleep time to be given after offset check
 * @return 0 if success or error on failure.
 */
static int adux1050_offset_check(struct adux1050_chip *adux1050, u16 max_cnt,
					s16 dir_flag, u16 slp_time)
{
	s16 cal_offset;
	u16 data;
	u16 power_ctrl;
	u16 cin_range;
	u16 lp_count;
	u16 stg_num;
	u16 *offset;
	u16 *stg_cfg_reg;
	s16 err = 0;

	err = adux1050->read(adux1050->dev, CTRL_REG, &power_ctrl, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C WR Err %d in %s\n", err, __FILE__);
		return err;
	}
	cin_range = GET_CIN_RANGE(power_ctrl);
	adux1050->tot_stg = adux1050->num_stages;
	for (lp_count = 0; lp_count < max_cnt ; lp_count++) {
		for (stg_num = 0; stg_num < adux1050->tot_stg; stg_num++) {
			if ((!CHECK_CAL_STATE(
			    adux1050->dac_calib.sat_comp_stat, stg_num)) ||
			    (adux1050->stg_info[stg_num].status ==
			    CIN_NOT_CONNECTED))
				continue;
			offset = &adux1050->cur_dac_offset[stg_num];
			stg_cfg_reg = &adux1050->cur_swap_state[stg_num];
			err = adux1050->read(adux1050->dev,
					     GET_RESULT_REG(stg_num),
					     &data, DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev,
					"I2C WR Err %d in %s\n",
					err, __FILE__);
				return err;
			}
			if ((data <= ZERO_SCALE_VALUE) ||
			    (data >= FULL_SCALE_VALUE)) {
				if (lp_count != 0)
					cal_offset = get_calc_dac_offset(
							*offset, *stg_cfg_reg);
				else
					cal_offset = 0;
				cal_offset +=
				(GET_DAC_STEP_SIZE(DAC_CODEOUT_SAT,
						   cin_range) * dir_flag);
				pr_debug("[%d]Curr offset set (%d)\n",
					stg_num, cal_offset);
				err = set_calc_dac_offset(adux1050, cal_offset,
						    stg_num, stg_cfg_reg,
						    offset, 0);
				if (err < ZERO_VAL) {
					dev_err(adux1050->dev,
						"set_calc_dac_offset failed %d\n",
						err);
					return err;
				}
			} else {
				CLR_CAL_STATUS(
				adux1050->dac_calib.sat_comp_stat, stg_num);
				pr_debug("%s - SATUR state(%d)\n", __func__,
					adux1050->dac_calib.sat_comp_stat);
				if (!adux1050->dac_calib.sat_comp_stat)
					goto adux_offset_ok;
			}
			ADUX1050_LOG("SATURATION STATUS - %x\n",
					    adux1050->dac_calib.sat_comp_stat);
		}
		msleep(slp_time);
	}
	return -EIO;
adux_offset_ok:
	ADUX1050_LOG("%s, offset ok (%d)\n", __func__, data);
	return 0;
}

static inline void reset_stgcal_flag(struct adux1050_chip *adux1050, u16 *count)
{
	u16 lp_cnt = 0;

	for (; lp_cnt < adux1050->tot_stg; lp_cnt++) {
		if (adux1050->stg_info[lp_cnt].status == CIN_CONNECTED) {
			adux1050->dac_calib.stg_cal_stat |= (1 << lp_cnt);
			count[lp_cnt] = 1;
		}
	}

}
/*
 * int adux1050_binary_offset_check(struct adux1050_chip *adux1050,
 *				       u16 slp_time)
 * ADUX1050 Saturation routine for bringing the device out of saturation using
 * binary search method
 * @param adux1050 The device structure to be calibrated
 * @param slp_time	Sleep time to be given after offset check
 * @return Returns 0 on success and -EIO on error.
 */
static int adux1050_binary_offset_check(struct adux1050_chip *adux1050,
					u16 slp_time)
{
	u16 data;
	u16 lp_count;
	u16 stg_num;
	u16 *offset;
	u16 *stg_cfg_reg;
	s16 err = 0;
	s16 low_dac_offset[TOTAL_STG] = {-MAX_OFFSET,
					     -MAX_OFFSET,
					     -MAX_OFFSET,
					     -MAX_OFFSET};
	s16 curr_dac_offset[TOTAL_STG] = {ZERO_VAL,
					      ZERO_VAL,
					      ZERO_VAL,
					      ZERO_VAL};
	s16 high_dac_offset[TOTAL_STG] = {MAX_OFFSET,
					      MAX_OFFSET,
					      MAX_OFFSET,
					      MAX_OFFSET};
	adux1050->tot_stg = adux1050->num_stages;

	for (lp_count = 0; lp_count < MAX_SEARCH_DEPTH ; lp_count++) {
		for (stg_num = 0; stg_num < adux1050->tot_stg; stg_num++) {
			if ((!CHECK_CAL_STATE(
			    adux1050->dac_calib.sat_comp_stat, stg_num)) ||
			    (adux1050->stg_info[stg_num].status ==
			    CIN_NOT_CONNECTED))
				continue;
			offset = &adux1050->cur_dac_offset[stg_num];
			stg_cfg_reg = &adux1050->cur_swap_state[stg_num];
			err = adux1050->read(adux1050->dev,
					GET_RESULT_REG(stg_num), &data, DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C Read Err %d in %s\n",
					err, __func__);
				return err;
			}
			if (data >= FULL_SCALE_VALUE) {
				low_dac_offset[stg_num] =
					curr_dac_offset[stg_num];
			} else if (data <= ZERO_SCALE_VALUE) {
				high_dac_offset[stg_num] =
					curr_dac_offset[stg_num];
			} else {
				CLR_CAL_STATUS(
					adux1050->dac_calib.sat_comp_stat,
					stg_num);
				dev_dbg(adux1050->dev,
					 " SATUR state(%d)\n",
				adux1050->dac_calib.sat_comp_stat);
				if (!adux1050->dac_calib.sat_comp_stat)
					goto adux_offset_ok;

			}
			curr_dac_offset[stg_num] = (low_dac_offset[stg_num] +
				 high_dac_offset[stg_num]) / 2;
			err = set_calc_dac_offset(adux1050,
						  curr_dac_offset[stg_num],
						  stg_num, stg_cfg_reg,
						  offset, 0);
			if (err < ZERO_VAL) {
				dev_err(adux1050->dev,
					"set_calc_dac_offset failed %d in %s\n",
					err, __func__);
				return err;
			}
			pr_debug("Offset Set for stg[%d]", stg_num);
			pr_debug("Set to L[%d] C[%d] H[%d]",
				low_dac_offset[stg_num],
				curr_dac_offset[stg_num],
				high_dac_offset[stg_num]);
		}
		msleep(slp_time);
	}
	err = adux1050->read(adux1050->dev,
						 GET_RESULT_REG(stg_num),
						 &data, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __func__);
		return err;
	}
	if (data <= ZERO_SCALE_VALUE || data >= FULL_SCALE_VALUE) {
		pr_debug("offset_check- EIO");
		return -EIO;
	}
adux_offset_ok:
	pr_debug("offset ok (%d)", data);
	return 0;
}

/*
 * do_dac_compensation(struct adux1050_chip *adux1050, u16 power_ctrl,
				u16 cin_range, u16 *slp_time)
 * Non saturation CDC compensation routine.
 * @param adux1050 The device structure to be calibrated
 * @param power_ctrl The power control register read
 * @param cin_range  The current configuration's CIN range value
 * @param slp_time	  The sleep time to be given
 */
static int do_dac_compensation(struct adux1050_chip *adux1050, u16 power_ctrl,
				u16 cin_range, u16 *slp_time)
{
	u16 cdc_diff;
	u16 lp_count = 0;
	u16 stg_num = 0;
	u16 count[TOTAL_STG] = {0};
	s16 cal_offset = 0;
	u16 flr_cnt = 0;
	u16 data[TOTAL_STG] = {0};
	u16 hys_reg[TOTAL_STG] = {0};
	u16 offset = 0;
	u16 stg_cfg_reg = 0;
	s16 digi_offset = 0;
	u16 use_digi_offset = 0;
	s32 err = -EIO;
	u16 u16_div = GET_ARB_DAC_STEP_SIZE(cin_range);
	u16 dac_step = u16_div;
	u16 trgt[TOTAL_STG] = {0};
	u16 init_swap_state[TOTAL_STG];
	u16 init_dac_offset[TOTAL_STG];
	struct adux1050_platform_data *pdata = adux1050->pdata;
	u16 org_trgt[TOTAL_STG] = {pdata->req_stg0_base,
				   pdata->req_stg1_base,
				   pdata->req_stg2_base,
				   pdata->req_stg3_base};

	*slp_time = get_conv_time(adux1050, TWICE_CONV_DELAY_TIME);
	adux1050->dac_calib.sat_comp_stat = ZERO_VAL;
	if (adux1050->metal_id >= MET_VER1) {
		for (lp_count = 0; lp_count < TOTAL_STG; lp_count++)
			trgt[lp_count] = org_trgt[lp_count];
		use_digi_offset = 1;
		ADUX1050_LOG("%s Binary Search defined\n", __func__);
	}

	/*Writing initial values to zero*/
	for (lp_count = 0; lp_count < adux1050->tot_stg; lp_count++) {
		err = adux1050->read(adux1050->dev, GET_OFFSET_REG(lp_count),
			       &offset, DEF_WR);
		if (err < I2C_WRMSG_LEN) {
			dev_err(adux1050->dev, "I2C RD Err %d in %s\n",
				err, __FILE__);
			goto calib_failed_break;
		}
		err = adux1050->read(adux1050->dev, GET_CONFIG_REG(lp_count),
			       &stg_cfg_reg, DEF_WR);
		if (err < I2C_WRMSG_LEN) {
			dev_err(adux1050->dev, "I2C RD Err %d in %s\n",
				err, __FILE__);
			goto calib_failed_break;
		}
		init_swap_state[lp_count] = stg_cfg_reg;
		init_dac_offset[lp_count] = offset;
		/*Set initial step count to 1 WARNING: DO NOT set it to zero */
		count[lp_count] = 1;
		cal_offset = get_calc_dac_offset(offset, stg_cfg_reg);
		if (cal_offset != ZERO_VAL)
			cal_offset = 0;
		err = set_calc_dac_offset(adux1050, cal_offset, lp_count,
				    &stg_cfg_reg, &offset, 0);
		if (err < ZERO_VAL) {
			dev_err(adux1050->dev, "CALC OFF ERR %d in %s\n",
				err, __FILE__);
			goto calib_failed_break;
		}
		adux1050->cur_swap_state[lp_count] = stg_cfg_reg;
		adux1050->cur_dac_offset[lp_count] = offset;
		if (adux1050->metal_id < MET_VER1)
			trgt[lp_count] = HALF_SCALE_VAL;
		/** Clear the Digital offset if set already*/
		err = adux1050->read(adux1050->dev, GET_HYS_REG(lp_count),
			       &hys_reg[lp_count], DEF_WR);
		if (err < I2C_WRMSG_LEN) {
			dev_err(adux1050->dev, "I2C RD Err %d in %s\n",
				err, __FILE__);
			goto calib_failed_break;
		}
		hys_reg[lp_count] &= HYS_BYTE_MASK;
		err = adux1050->write(adux1050->dev, GET_HYS_REG(lp_count),
				&hys_reg[lp_count], DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
			goto calib_failed_break;
		}
		if (adux1050->stg_info[lp_count].status == CIN_CONNECTED) {
			adux1050->dac_calib.sat_comp_stat |= (1 << lp_count);
			adux1050->dac_calib.stg_cal_stat |= (1 << lp_count);
		}
	}
	msleep(*slp_time);
	u16_div = GET_ARB_DAC_STEP_SIZE(GET_CIN_RANGE(power_ctrl));
	dac_step = u16_div;
	/* Clear the device from saturation*/
	if (adux1050->metal_id < MET_VER1) {
		flr_cnt = (MAX_OFFSET / (GET_DAC_STEP_SIZE(DAC_CODEOUT_SAT,
						GET_CIN_RANGE(power_ctrl))));
		ADUX1050_LOG("%s - POSITIVE SAT ROUTINE\n", __func__);
		err = adux1050_offset_check(adux1050, flr_cnt, 1, *slp_time);
		if (err < 0) {
			ADUX1050_LOG("%s - NEGATIVE SAT ROUTINE\n", __func__);
			err = adux1050_offset_check(adux1050, flr_cnt,
						    MINUS_VAL, *slp_time);
			if (err < 0) {
				dev_err(adux1050->dev,
					"adux1050_offset_chk fail %d in %s\n",
					err, __func__);
				goto calib_failed_break;
			}
		}
	} else if (adux1050->metal_id >= MET_VER1) {
		ADUX1050_LOG("%s - Binary SAT ROUTINE begins\n", __func__);
		err = adux1050_binary_offset_check(adux1050, *slp_time);
		if (err < 0) {
			dev_err(adux1050->dev, "%s - Fail for both directions",
				__func__);
			goto calib_failed_break;
		}
		dev_dbg(adux1050->dev, "%s - Binary SAT ROUTINE ends",
			 __func__);
	}
	/* Set Stage cal status to uncalibrated initial value*/

	/* Make the stages to their corresponding target value*/
	for (lp_count = 0; lp_count < CALIB_LOOP_CNT; lp_count++) {
		u16 prv_data;

		for (stg_num = 0; stg_num < adux1050->tot_stg; stg_num++) {
			/*Skip the stage if already calibrated*/
			if ((!CHECK_CAL_STATE(
			    adux1050->dac_calib.stg_cal_stat, stg_num)) ||
			    (adux1050->stg_info[stg_num].status ==
			    CIN_NOT_CONNECTED))
				continue;
			prv_data = data[stg_num];
			err = adux1050->read(adux1050->dev,
					     GET_RESULT_REG(stg_num),
					     &data[stg_num], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev,
					"I2C RD Err %d in %s\n",
					err, __FILE__);
				goto calib_failed_break;
			}
			pr_debug("DATA %d,OFF %x,SWAP %x,STG %d\n",
				data[stg_num],
				adux1050->cur_dac_offset[stg_num],
				adux1050->cur_swap_state[stg_num],
				stg_num);
			/*Device not in saturation*/
			cdc_diff = abs(data[stg_num] - trgt[stg_num]);
			if (use_digi_offset && (cdc_diff < DIGI_OFFSET_SIZE)) {
				count[stg_num] = 0;
			} else {
				u16_div = abs(prv_data - data[stg_num]);
				u16_div = u16_div / count[stg_num];
				if ((u16_div < (dac_step/2)) ||
				    (u16_div > (dac_step + dac_step)))
					u16_div = dac_step;
				dev_dbg(adux1050->dev, "%s,p_data(%d)",
					__func__, prv_data);
				flr_cnt = cdc_diff / u16_div;
				count[stg_num] = ((cdc_diff % u16_div) >
					 GET_60_PERCENT(u16_div)) ?
					 (flr_cnt + 1) : flr_cnt;
				pr_debug("FLC(%d),CNT(%d),DIV(%d),DIF(%d)\n",
					flr_cnt, count[stg_num],
					u16_div, cdc_diff);
			}
			if (count[stg_num] != 0) {
				/* DAC step can be used to minimize the */
				/* difference in current and required CDC*/
				cal_offset = get_calc_dac_offset(
					adux1050->cur_dac_offset[stg_num],
					adux1050->cur_swap_state[stg_num]);
				dev_dbg(adux1050->dev, "%s,cal_off %d\n",
					__func__, cal_offset);
				if (data[stg_num] > trgt[stg_num])
					cal_offset += count[stg_num];
				else
					cal_offset -= count[stg_num];
				err = set_calc_dac_offset(adux1050, cal_offset,
				stg_num, &adux1050->cur_swap_state[stg_num],
				&adux1050->cur_dac_offset[stg_num], 0);
				if (err < ZERO_VAL) {
					dev_err(adux1050->dev, "%s S_Off err\n",
						__func__);
					goto calib_failed_break;
				}
			} else {
				CLR_CAL_STATUS(
				adux1050->dac_calib.stg_cal_stat, stg_num);
				/*When all the stages are in halfscale change*/
				/*  to the original target and CIN_range*/
				if ((!adux1050->dac_calib.stg_cal_stat) &&
				    (!use_digi_offset) &&
				    (adux1050->metal_id < MET_VER1)) {
					trgt[STG_ZERO] = pdata->req_stg0_base;
					trgt[STG_ONE] = pdata->req_stg1_base;
					trgt[STG_TWO] = pdata->req_stg2_base;
					trgt[STG_THREE] = pdata->req_stg3_base;
					pr_debug("R TRGT %x,%x,%x,%x\n",
						trgt[STG_ZERO], trgt[STG_ONE],
						trgt[STG_TWO],
						trgt[STG_THREE]);
					power_ctrl = SET_CIN_RANGE(power_ctrl,
								   cin_range);
					err = adux1050->write(adux1050->dev,
						CTRL_REG, &power_ctrl, DEF_WR);
					if (err < DEF_WR) {
						dev_err(adux1050->dev,
							" I2C WR Err %d in %s\n",
							err, __func__);
						goto calib_failed_break;
					}
					u16_div = GET_ARB_DAC_STEP_SIZE(
						    GET_CIN_RANGE(power_ctrl));
					dac_step = u16_div;
					reset_stgcal_flag(adux1050, count);

					use_digi_offset++;
					/* msleep(*slp_time);*/
					break;
				}
				/*Not in saturation and DAC minimal step size*/
				/* is higher than the required correction*/
				digi_offset = GET_DIGI_OFFSET(trgt[stg_num],
							      data[stg_num]);
				digi_offset = CLAMP_DIGI_OFFSET(digi_offset);

				adux1050->pdata->cal_offset[stg_num] =
					adux1050->cur_dac_offset[stg_num];
				adux1050->pdata->digi_offset[stg_num] =
					(u8)digi_offset;

				hys_reg[stg_num] =
				((hys_reg[stg_num] & HYS_BYTE_MASK) |
				 (adux1050->pdata->digi_offset[stg_num] << 8));
				err = adux1050->write(adux1050->dev,
						GET_HYS_REG(stg_num),
						&hys_reg[stg_num], DEF_WR);
				if (err < DEF_WR) {
					dev_err(adux1050->dev,
						"I2C WR Err %d in %s\n",
						err, __func__);
					goto calib_failed_break;
				}
				ADUX1050_LOG("Hys value = %x\n",
						    hys_reg[stg_num]);
				msleep(*slp_time);
				err = adux1050->read(adux1050->dev,
					       GET_RESULT_REG(stg_num),
					       &data[stg_num], DEF_WR);
				if (err < I2C_WRMSG_LEN) {
					dev_err(adux1050->dev,
						"I2C RD Err %d in %s\n",
						err, __func__);
					goto calib_failed_break;
				}
				adux1050->pdata->cal_fact_base[stg_num] =
						data[stg_num];
				adux1050->pdata->stg_cfg[stg_num] =
					adux1050->cur_swap_state[stg_num];
				dev_dbg(adux1050->dev,
					 "bas(%d)off(%x)st_con(%x)dioff(%d)\n",
					 data[stg_num], offset,
					 stg_cfg_reg, digi_offset);
				err = ZERO_VAL;
				pr_debug("CAL STATUS - %x\n",
					adux1050->dac_calib.stg_cal_stat);
				if (!adux1050->dac_calib.stg_cal_stat)
					goto calib_success_break;
				else
					continue;
			}
		}	/*End of stage based loop*/
		msleep(*slp_time);
	}	/*End of for*/

calib_failed_break:
	/*Failed to complete the compensation, return to the original values*/
	for (lp_count = 0; lp_count < adux1050->tot_stg; lp_count++) {
		err = adux1050->write(adux1050->dev, GET_CONFIG_REG(lp_count),
				&init_swap_state[lp_count], DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
		err = adux1050->write(adux1050->dev, GET_OFFSET_REG(lp_count),
			       &init_dac_offset[lp_count], DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __FILE__);
		adux1050->pdata->cal_fact_base[lp_count] = 0;
		adux1050->pdata->cal_offset[lp_count] = 0;
		adux1050->pdata->digi_offset[lp_count] = 0;
		adux1050->pdata->stg_cfg[lp_count] = 0;
		adux1050->dac_calib.cal_flags = CAL_RET_FAIL;
	}
	msleep(*slp_time);
	return err;
calib_success_break:
	adux1050->dac_calib.cal_flags = CAL_RET_SUCCESS;
	return CAL_RET_SUCCESS;
}

/*
 * static inline int adux1050_disable(struct adux1050_chip *adux1050)
 * Routine to set the power mode to standby in the ADUX1050 chip
 * @param adux1050 Chip structure to set the power mode to shutdown
 * @return Zero on success.
 */
static inline int adux1050_disable(struct adux1050_chip *adux1050)
{
	u16 data = 0;
	s16 err = 0;

	mutex_lock(&adux1050->mutex);
	err = adux1050->read(adux1050->dev, CTRL_REG, &data, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __func__);
		mutex_unlock(&adux1050->mutex);
		return err;
	}
	if (GET_PWR_MODE(data) != PWR_STAND_BY) {
		data = SET_PWR_MODE(data, PWR_STAND_BY);
		err = adux1050->write(adux1050->dev, CTRL_REG, &data, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __func__);
			mutex_unlock(&adux1050->mutex);
			return err;
		}
	}

	mutex_unlock(&adux1050->mutex);
	return 0;
}

/*
 * static int adux1050_enable(struct adux1050_chip *adux1050)
 * Routine to set the driver to enable state in the ADUX1050 chip
 * @param adux1050 Chip structure to set the power mode to shutdown
 * @return Zero on success.
 */
static int adux1050_enable(struct adux1050_chip *adux1050)
{
	s16 err = 0;

	mutex_lock(&adux1050->mutex);
	err = adux1050_hw_init(adux1050);
	if (err < ZERO_VAL) {
		dev_err(adux1050->dev, "Failed in adux1050_hw_init %d\n", err);
		mutex_unlock(&adux1050->mutex);
		return err;
	}

#ifdef CONFIG_ADUX1050_EVAL
	if (adux1050->power_mode_flag == ADUX1050_ENABLE) {
		err = adux1050->write(adux1050->dev, CTRL_REG,
				&adux1050->ctrl_reg, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d in %s\n",
				err, __func__);
			mutex_unlock(&adux1050->mutex);
			return err;
		}
		adux1050->power_mode_flag = ADUX1050_DISABLE;
	}
#endif

	mutex_unlock(&adux1050->mutex);
	return 0;
}

/*
 * static void adux1050_calibration(struct work_struct *cal_work)
 * ADUX1050 Calibration routine
 * @param cal_work Pointer to "calib_work", member of ADUX1050 chip structure
 * @return int
 */
static void adux1050_calibration(struct work_struct *cal_work)
{
	struct adux1050_chip *adux1050 =
		container_of(cal_work, struct adux1050_chip, calib_work);
	u8 slp_time_flag = 0;
	s32 err = 0;
	u16 data;
	u16 power_ctrl;
	u16 lp_cnt;
	u16 slp_time = 0;
	u16 s_t = 0;
	u16 cv_time_ctrl = 0;
	u16 cin_range = 0;
	u16 cur_power = 0;
	u32 start_time = jiffies;

	mutex_lock(&adux1050->mutex);
	err = adux1050->read(adux1050->dev, INT_CTRL_REG,
		       &adux1050->dac_calib.enable_setting, DEF_WR);
	if (err < I2C_WRMSG_LEN)
		dev_err(adux1050->dev, "I2C RD Err %d\n", err);
	/*
	 * Disable interrupt and digital offset
	 */
	data = DISABLE_DEV_INT | adux1050->int_pol;
	err = adux1050->write(adux1050->dev, INT_CTRL_REG, &data, DEF_WR);
	if (err < DEF_WR)
		dev_err(adux1050->dev, "I2C WR Err %d\n", err);
	adux1050->dac_calib.cal_flags = CAL_RET_PENDING;
	if (adux1050->dac_calib.action_flag) {
		if (adux1050->proxy_enable) {
			ADUX1050_LOG("%s -proxywork cancelled in calib",
					    __func__);
			adux1050->proxy_cancel = _TRUE;
			cancel_delayed_work(&adux1050->proxy_work);
		}
		dev_dbg(adux1050->dev, "\n\n\n%s CALIB STARTED\n\n", __func__);
		slp_time = get_conv_time(adux1050, CONV_DELAY_TIME);
		err = adux1050->read(adux1050->dev, CONV_TIME_CTRL_REG,
			       &cv_time_ctrl, DEF_WR);
		if (err < I2C_WRMSG_LEN)
			dev_err(adux1050->dev, "I2C RD Err %d\n", err);
		if (CHECK_MIN_TIME(cv_time_ctrl)) {
			data = SET_MIN_TIME(cv_time_ctrl);
			err = adux1050->write(adux1050->dev, CONV_TIME_CTRL_REG,
					      &data, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C WR Err %d\n", err);
			slp_time_flag = 1;
			dev_dbg(adux1050->dev, "%s Check Min time %x\n",
				__func__, data);
		}

		err = adux1050->read(adux1050->dev, CTRL_REG,
				     &power_ctrl, DEF_WR);
		if (err < I2C_WRMSG_LEN)
			dev_err(adux1050->dev, "I2C RD Error %d\n", err);
		if (GET_PWR_MODE(power_ctrl) != PWR_FULL_POWER) {
			cur_power = SET_PWR_MODE(power_ctrl, PWR_FULL_POWER);
			err = adux1050->write(adux1050->dev, CTRL_REG,
					&cur_power, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C WR Err %d\n", err);
			slp_time_flag = slp_time_flag | 0x2;
			slp_time += get_conv_time(adux1050, CONV_TIME);
			dev_dbg(adux1050->dev, "Mode change %d\n", slp_time);
		} else {
			cur_power = power_ctrl;
		}
		cin_range = GET_CIN_RANGE(cur_power);
		if (adux1050->metal_id < MET_VER1) {
			if (cin_range != PICO_5) {
				cur_power = SET_CIN_RANGE(cur_power, PICO_5);
				err = adux1050->write(adux1050->dev, CTRL_REG,
						&cur_power, DEF_WR);
				if (err < DEF_WR)
					dev_err(adux1050->dev,
						"I2C WR Err %d\n",
						err);
			}
		}
		if (slp_time_flag) {
			dev_dbg(adux1050->dev, "Calib sleep %d\n", slp_time);
			msleep(slp_time);
		}
		err = do_dac_compensation(adux1050, cur_power,
					  cin_range, &s_t);
		dev_dbg(adux1050->dev, "calibration return  %d\n", err);
	} else {
		adux1050->dac_calib.cal_flags = CAL_RET_NONE;
		for (lp_cnt = 0; lp_cnt < TOTAL_STG; lp_cnt++) {
			adux1050->pdata->cal_fact_base[lp_cnt] = 0;
			adux1050->pdata->cal_offset[lp_cnt] = 0;
			adux1050->pdata->digi_offset[lp_cnt] = 0;
			adux1050->pdata->stg_cfg[lp_cnt] = 0;
		}
	}

	if (adux1050->dac_calib.action_flag) {
		/* If calibration succeed set baseline by using force calib*/
		if (adux1050->dac_calib.cal_flags == CAL_RET_SUCCESS) {
			dev_dbg(adux1050->dev, "before FC in calib");
			err = adux1050_force_cal(adux1050, s_t);
			if (err < 0)
				dev_err(adux1050->dev, "I2C RD/WR err %d\n",
					err);
		}
		/* Restore the prv conversion time settings */
		if (slp_time_flag & 1) {
			err = adux1050->write(adux1050->dev, CONV_TIME_CTRL_REG,
					&cv_time_ctrl, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C WR Err %d\n", err);
		}
		/* Restore the previous power Setting */
		if (slp_time_flag & 2) {
			err = adux1050->write(adux1050->dev, CTRL_REG,
					&power_ctrl, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C WR Err %d\n", err);
		}
	}
	data = adux1050->dac_calib.enable_setting | adux1050->int_pol;
	err = adux1050->write(adux1050->dev, INT_CTRL_REG, &data, DEF_WR);
	if (err < DEF_WR)
		dev_err(adux1050->dev, "I2C WR Error %d\n", err);
	dev_dbg(adux1050->dev, "\n\nCALIB ENDS Status %d Time %d ms\n\n",
		 adux1050->dac_calib.cal_flags,
		 jiffies_to_msecs(jiffies - start_time));
	mutex_unlock(&adux1050->mutex);
	if ((adux1050->proxy_enable) &&
	    (adux1050->dac_calib.action_flag) &&
	    (adux1050->dev_enable)) {
		adux1050->proxy_cancel = _FALSE;
		schedule_delayed_work(&adux1050->proxy_work,
				      msecs_to_jiffies
		(PROXY_TIME * adux1050->allowed_proxy_time));
	}
	if (adux1050->dev_enable == ADUX1050_DISABLE) {
		err = adux1050_disable(adux1050);
		if (err < ZERO_VAL)
			dev_err(adux1050->dev, "adux1050_disable failed %d in %s\n",
				err, __func__);
	}

}

/*
 * static ssize_t store_enable(struct device *dev,
 *				struct device_attribute *attr,
 *				const char *buf, size_t count)
 * This function is used to enable or to disable the device. The Sysfs attribute
 * is given as "enable", writing a '0' Disables the device.
 * While writing '1' , enables the device.
 * @param dev The Device Id structure(linux standard argument)
 * @param attr Standard Linux Device attributes to the ADUX1050.
 * @param buf The buffer which contains the data.
 * @param count The count of bytes to be transferred to the Device.
 * \note This is evoked upon an echo/write request in /sys/../devices region.
 * \note This also prints the results in the console for the user.
 * @return count of data written.
 */
static ssize_t store_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	char *dev_state[2] = { "DISABLED", "ENABLED"};
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s,kstrtoint failed\n", __func__);
		return err;
	}
	if (val > 1) {
		dev_dbg(adux1050->dev, "%s Invalid- Enable:1 Disable:0\n",
			 __func__);
		goto exit;
	}
	dev_dbg(adux1050->dev, "%s - prv_flag %d curr %d\n", __func__,
		 adux1050->dev_enable, val);
	if (adux1050->dev_enable == val) {
		dev_dbg(adux1050->dev, "%s - Device already in %s state\n",
			 __func__, dev_state[val]);
		goto exit;
	}
	if (val == ADUX1050_ENABLE) {
		adux1050->dev_enable = val;
#ifdef CONFIG_ADUX1050_POLL
		err = adux1050_enable(adux1050);
		if (err < ZERO_VAL) {
			dev_err(adux1050->dev, " ENABLE Err %d\n", err);
			adux1050->dev_enable = ZERO_VAL;
			return count;
		}
		wake_up_process(adux1050->polling_task);
#else
		enable_irq(adux1050->irq);
		enable_irq_wake(adux1050->irq);
		err = adux1050_enable(adux1050);
		if (err < ZERO_VAL) {
			dev_err(adux1050->dev, " ENABLE Err %d\n", err);
			adux1050->dev_enable = ZERO_VAL;
			return count;
		}
#endif
		if (adux1050->proxy_enable) {
			adux1050->proxy_cancel = _FALSE;
			schedule_delayed_work(&adux1050->proxy_work,
					      msecs_to_jiffies
			(PROXY_TIME * adux1050->allowed_proxy_time));
		}
		dev_dbg(adux1050->dev, "ADUX1050 is enabled\n");
	} else {
		adux1050->dev_enable = val;
#ifdef CONFIG_ADUX1050_POLL

#else
		disable_irq_wake(adux1050->irq);
		disable_irq(adux1050->irq);
#endif
		err = adux1050_disable(adux1050);
		if (err < ZERO_VAL) {
			dev_err(adux1050->dev, "DISABLE Err %d\n", err);
			adux1050->dev_enable = ADUX1050_ENABLE;
			return count;
		}
		adux1050->proxy_cancel = _TRUE;
		cancel_delayed_work(&adux1050->proxy_work);
		dev_dbg(adux1050->dev, "ADUX1050 is Disabled\n");
	}
exit:
	return count;
}

/*
 * static ssize_t show_enable(struct device *dev,
			      struct device_attribute *attr, char *buf)
 * This Function is used to show the status of the driver
 * Status '1' signifies the device is ENABLED,
 * while the status '0' signifies a DISABLED device.
 * @param dev The Device Id structure(linux standard argument)
 * @param attr standard Linux Device attributes to the ADUX1050.
 * @param buf The buffer to store the data to be written.
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * \note This also prints the results in the console for the user.
 * @return The count of data written.
 */
static ssize_t show_enable(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", adux1050->dev_enable);
}

#ifdef CONFIG_ADUX1050_DUMP_REGS

/*
 * static ssize_t show_dumpregs(struct device *dev,
			      struct device_attribute *attr, char *buf)
 * This Function is used for dumping the registers value of the ADUX1050.
 * @param dev The Device Id structure(linux standard argument)
 * @param attr standard Linux Device attributes to the ADUX1050
 * @param buf The buffer to store the data to be written
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * \note This also prints the results in the console for the user.
 * @return count of data written
 */
static ssize_t show_dumpregs(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 u16temp[MAX_ADUX1050_WR_LEN];
	u32 u32_lpcnt = 0;
	u16 ret = 0;
	s16 err = 0;

	mutex_lock(&adux1050->mutex);
	dev_dbg(adux1050->dev, "Global control registers\n");
	err = adux1050->read(adux1050->dev, DEV_ID_REG,
			     u16temp, GLOBAL_REG_CNT);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	for (; u32_lpcnt < GLOBAL_REG_CNT; u32_lpcnt++) {
		dev_dbg(adux1050->dev, "Reg 0X%x val 0x%x\n",
			 u32_lpcnt, u16temp[u32_lpcnt]);
		ret += snprintf(buf + ret, PAGE_SIZE, "%4x ",
				u16temp[u32_lpcnt]);
	}
	dev_dbg(adux1050->dev, "Stage config registers\n");
	err = adux1050->read(adux1050->dev, CONFIG_STG0_REG,
			     u16temp, STG_CNF_CNT);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	for (u32_lpcnt = CONFIG_STG0_REG;
		u32_lpcnt <= GET_HYS_REG(STG_THREE) ; u32_lpcnt++) {
		dev_dbg(adux1050->dev, "Reg 0X%x val 0x%x\n",
			 u32_lpcnt, u16temp[u32_lpcnt-CONFIG_STG0_REG]);
		ret += snprintf(buf + ret, PAGE_SIZE, "%4x ",
				u16temp[u32_lpcnt-CONFIG_STG0_REG]);
	}
	dev_dbg(adux1050->dev, "Result/Base/p2p registers\n");
	err = adux1050->read(adux1050->dev, INT_STATUS_REG,
			     u16temp, STATUS_REG_CNT);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	for (u32_lpcnt = INT_STATUS_REG;
	     u32_lpcnt <= PROX_STATUS_REG; u32_lpcnt++) {
		dev_dbg(adux1050->dev, "Reg 0X%x val 0x%x\n",
			 u32_lpcnt, u16temp[u32_lpcnt-INT_STATUS_REG]);
		ret += snprintf(buf + ret, PAGE_SIZE, "%4x ",
			       u16temp[u32_lpcnt-INT_STATUS_REG]);
	}
unlock_mut:
	mutex_unlock(&adux1050->mutex);
	return ret;
}

#endif

/*
 * static ssize_t adux1050_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
 * This is used to display the device name of the chipset.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * @return Returns the size of the output buffer with the on/off status
 */
static ssize_t adux1050_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%s\n", adux1050->pdata->device_name);
}

#ifdef CONFIG_ADUX1050_EVAL

/*
 * static ssize_t adux1050_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
 * This is used to display the vendor name of the device.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * @return Returns the size of the output buffer with the on/off status
 */
static ssize_t adux1050_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", VENDOR_NAME);
}

/*
 * static ssize_t adux1050_raw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
 * This is used to display the Raw CDC data of a stage
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * @return Returns the size of the raw data of all the stages
 */
static ssize_t adux1050_raw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 temp_cdc = 0;
	s16 ret = 0;
	s16 err = 0;

	if (!adux1050->dev_enable) {
		ADUX1050_LOG("Device is not enabled\n");
		goto data_show_err;
	}
	mutex_lock(&adux1050->mutex);
	err = adux1050->read(adux1050->dev,
			     GET_RESULT_REG(adux1050->stg_raw_cdc),
			     &temp_cdc, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	ADUX1050_LOG("%s, STG_NO - %d : raw_data - %x\n", __func__,
			    adux1050->stg_raw_cdc, temp_cdc);
	ret = snprintf(buf, PAGE_SIZE, "0x%04x ", temp_cdc);
unlock_mut:
	mutex_unlock(&adux1050->mutex);
data_show_err:
	return ret;
}

/*
 * adux1050_raw_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
 * This is used to set the stage number to display CDC raw data.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer which contains the data.
 * @param count The count of bytes to be transferred to the Device.
 * \note This is evoked upon an echo/write request in /sys/../devices region.
 * @return Returns the count of the raw data value of a single stage
 */
static ssize_t adux1050_raw_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(adux1050->dev, "%s, kstrtoint failed\n", __func__);
		return err;
	}
	if (val < TOTAL_STG)
		adux1050->stg_raw_cdc = val;
	else
		ADUX1050_LOG("%s, Invalid input %d\n", __func__, val);

	return count;
}

/*
 * adux1050_send_event_show(struct device *dev,
	struct device_attribute *attr, char *buf)
 * This is used to display the send event status of the driver.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat/read request in /sys/../devices region.
 * @return Returns the size of the output buffer with the send event status
 */
static ssize_t adux1050_send_event_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", adux1050->send_event);
}

/*
 * adux1050_send_event_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
 * This is used to set the send event flag in the driver
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the input buffer
 */
static ssize_t adux1050_send_event_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 val, err;

	err = kstrtoint(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s, kstrtoint failed\n", __func__);
		return err;
	}
	if ((val == ADUX1050_ENABLE) || (val == ADUX1050_DISABLE))
		adux1050->send_event = (unsigned char)val;
	else
		ADUX1050_LOG("%s - Invalid input %d\n", __func__, val);
	return count;
}

/*
 * This is used to show the threshold status of a stage
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an read request in the /sys/.../devices region.
 * @return Returns the size of the output buffer
 */
static ssize_t adux1050_threshold_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 temp_base = 0;
	u16 temp_ht = 0;
	u16 temp_lt = 0;
	u32 htresult = 0;
	s32 ltresult = 0;
	s16 ret = 0;
	s16 err = 0;

	if (!adux1050->dev_enable) {
		ADUX1050_LOG("Device is not enabled\n");
		goto err;
	}
	mutex_lock(&adux1050->mutex);
	err = adux1050->read(adux1050->dev,
		       GET_BASE_LINE_REG(adux1050->stg_threshold),
		       &temp_base, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	err = adux1050->read(adux1050->dev,
			     GET_HIGH_TH_REG(adux1050->stg_threshold),
			     &temp_ht, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}
	err = adux1050->read(adux1050->dev,
			     GET_LOW_TH_REG(adux1050->stg_threshold),
			     &temp_lt, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C Read Error %d\n", err);
		goto unlock_mut;
	}

	mutex_unlock(&adux1050->mutex);
	htresult = temp_base + temp_ht;
	if (htresult >= FULL_SCALE_VALUE)
		htresult = FULL_SCALE_VALUE;

	ltresult = temp_base - temp_lt;
	if (ltresult <= 0)
		ltresult = 0;

	ADUX1050_LOG("%s, STG_NO - %d : BS: %x, HT - %x : LT - %x\n",
			    __func__, adux1050->stg_threshold,
			    temp_base, temp_ht, temp_lt);
	ret = snprintf(buf, PAGE_SIZE, "0x%04x 0x%04x ",
		       htresult, ltresult);

err:
	return ret;
unlock_mut:
	mutex_unlock(&adux1050->mutex);
	return ret;
}

/*
 * This is used to set the stage number to show the threhold details of that
 *	stage
 *
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the input buffer
 */
static ssize_t adux1050_threshold_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err) {
		dev_err(adux1050->dev, "%s, kstrtoint failed\n", __func__);
		return err;
	}
	if (val < TOTAL_STG)
		adux1050->stg_threshold = val;
	else
		ADUX1050_LOG("%s, Invalid input %d\n", __func__, val);

	return count;
}

/*
 * This is used to check the DAC offset calibration work status from the driver
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an read request in the /sys/.../devices region.
 * @return Returns the size of the output buffer
 */
static ssize_t adux1050_calibration_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 ret = 0;
	s32 stg_cnt = 0;
	struct adux1050_platform_data *lpdata = adux1050->pdata;

	if (adux1050->dac_calib.cal_flags == CAL_RET_SUCCESS) {
		ret = snprintf(buf + ret, PAGE_SIZE, "%1d %1d ",
			adux1050->dac_calib.cal_flags, adux1050->conn_stg_cnt);
		for (stg_cnt = 0; stg_cnt < adux1050->num_stages; stg_cnt++) {
			if (adux1050->stg_info[stg_cnt].status ==
			    CIN_CONNECTED) {
				/*The STATUS, Stage number, Target, Offset,*/
				/*	SWAP_state, Digi_offset*/
				dev_dbg(adux1050->dev,
					 "%1d %1d 0x%04x 0x%04x 0x%04x 0x%04x ",
					 adux1050->dac_calib.cal_flags, stg_cnt,
					 lpdata->cal_fact_base[stg_cnt],
					 lpdata->cal_offset[stg_cnt],
					 lpdata->digi_offset[stg_cnt],
					 lpdata->stg_cfg[stg_cnt]);
				ret +=
				snprintf(buf + ret, PAGE_SIZE,
					 "%1d 0x%04x 0x%04x 0x%04x 0x%04x ",
					 (u16)stg_cnt,
					 (u16)lpdata->cal_fact_base[stg_cnt],
					 (u16)lpdata->cal_offset[stg_cnt],
					 (u16)lpdata->digi_offset[stg_cnt],
					 (u16)lpdata->stg_cfg[stg_cnt]);
			}
		}
		ret--;
	} else {
		ret = snprintf(buf, PAGE_SIZE, "%1d\n",
			       (u32)adux1050->dac_calib.cal_flags);
	}
	return ret;
}

/*
 * This is used to call the DAC offset calibration in the driver
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param size The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the input buffer
 */
static ssize_t adux1050_calibration_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u32 val;

	err = kstrtoint(buf, 0, &val);

	if (err < 0) {
		dev_err(adux1050->dev, "%s, kstrtoint failed\n", __func__);
		return err;
	}

	if ((val == ADUX1050_ENABLE) || (val == ADUX1050_DISABLE)) {
		if (adux1050->dev_enable != ADUX1050_ENABLE) {
			mutex_lock(&adux1050->mutex);
			err = adux1050_hw_init(adux1050);
			mutex_unlock(&adux1050->mutex);
			if (err < ZERO_VAL) {
				dev_err(adux1050->dev, "HW Init failed %d\n",
					err);
				return size;
			}
			ADUX1050_LOG("%s, hw_init_done\n", __func__);
		}
		mutex_lock(&adux1050->mutex);
		adux1050->dac_calib.action_flag = (u8)val;
		adux1050->dac_calib.cal_flags = CAL_RET_NONE;
		ADUX1050_LOG("%s, Calling schedule_work\n", __func__);
		mutex_unlock(&adux1050->mutex);
		schedule_work(&adux1050->calib_work);
	} else {
		dev_err(adux1050->dev, "%s, Invalid input %d\n",
			__func__, val);
	}
	return size;
}

#endif

/*
 * Command parsing function for echo/cat commands from command prompt.
 * This function is called when ever the User tries an echo / cat command
 * to the /../sysfs/devices especially during read/write registers.
 *
 * @return void Returns Nothing
 * @see store_reg_read
 */
static int cmd_parsing(const char *buf, u16 *addr, u16 *cnt,
		       u16 *data, u16 data_limit)
{
	char **bp = (char **)&buf;
	u8 *token, minus, parsing_cnt = 0;
	u16 val;
	s32 ret;
	s32 pos;

	data_limit = data_limit + 2;
	while ((token = strsep(bp, SPACE_CHAR))) {
		pos = 0;
		minus = false;
		if ((char)token[pos] == MINUS_CHAR) {
			minus = true;
			pos++;
		}

		ret = kstrtou16(&token[pos], 0, (unsigned short *)&val);
		if (ret)
			return ret;
		if ((parsing_cnt == 0) & (val > HIGHEST_READ_REG))
			return -ERANGE;
		if (minus)
			val *= MINUS_VAL;

		switch (parsing_cnt) {
		case PARSE_ADDR:
			*addr = val;
			break;
		case PARSE_CNT:
			*cnt  = val;
			break;
		default:
		case PARSE_DATA:
			*data = val;
			data++;
			break;
		}
		parsing_cnt++;
		if (parsing_cnt > data_limit)
			return parsing_cnt;
	}
	return parsing_cnt;
}

#ifdef CONFIG_ADUX1050_EVAL

/*
 * Command parsing function for echo/cat commands from command prompt.
 * This function is called when ever the User tries an echo / cat command
 * to the /../sysfs/devices especially during proxy_time
 *
 * @return void Returns Nothing
 * @see store_proxy_time
 */
static int cmd_parsing_for_proxy(const char *buf, u16 *addr, u16 *cnt,
		       u16 *data, u16 data_limit)
{
	char **bp = (char **)&buf;
	u8 *token, minus, parsing_cnt = 0;
	u16 val;
	s32 ret;
	s32 pos;

	data_limit = data_limit + 2;
	while ((token = strsep(bp, SPACE_CHAR))) {
		pos = 0;
		minus = false;
		if ((char)token[pos] == MINUS_CHAR) {
			minus = true;
			pos++;
		}

		ret = kstrtou16(&token[pos], 0, (unsigned short *)&val);
		if (ret)
			return ret;
		if (minus)
			val *= MINUS_VAL;

		switch (parsing_cnt) {
		case PARSE_ADDR:
			*addr = val;
			break;
		case PARSE_CNT:
			*cnt  = val;
			break;
		default:
		case PARSE_DATA:
			*data = val;
			data++;
			break;
		}
		parsing_cnt++;
		if (parsing_cnt >= data_limit)
			return parsing_cnt;
	}
	return parsing_cnt;
}

/*
 * This is used to update the calib output to the device from the application
 *	space
 * @return The Size of the Read data
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param size The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an wr request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t adux1050_update_calib_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 data[FILP_PARAM_CNT * TOTAL_STG];
	u16 status;
	u16 cnt;

	err = cmd_parsing(buf, &status, &cnt, data,
			  (FILP_PARAM_CNT * TOTAL_STG));
	if (err < 0) {
		dev_err(adux1050->dev, "%s,kstrtos16 failed %x\n",
			__func__, err);
		return err;
	}
	dev_err(adux1050->dev, "cmd_ret %d", err);

	mutex_lock(&adux1050->mutex);
	if (status == 1) {
		dev_err(adux1050->dev, "%s , Invalid option\n", __func__);
	} else if ((status == CAL_RET_SUCCESS) && (cnt <= TOTAL_STG)) {
		if ((cnt * FILP_PARAM_CNT + sizeof(u16)) != err) {
			dev_err(adux1050->dev, "%s,insuff data(%d)\n",
					__func__, err);
			goto err_data_cnt;
		}
		err = update_calib_settings(adux1050, cnt, data,
					      ADUX1050_ENABLE, ZERO_VAL);
		if (err < ZERO_VAL)
			dev_err(adux1050->dev,
				"Failed in Update_calib_settings %d\n", err);
	} else {/*TODO: Case 3 for File Write*/
		dev_err(adux1050->dev, "%s, Error for data val(%d)\n",
			__func__, err);
	}
err_data_cnt:
	mutex_unlock(&adux1050->mutex);
	return size;
}



/*
 *	This is used to show the DAC calibration routine baseline CDC value.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an read request in the /sys/.../devices region.
 *@return Returns the Size of the output buffer
 */
static ssize_t calib_target_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	struct adux1050_platform_data *pdata = adux1050->pdata;

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d\n", pdata->req_stg0_base,
			pdata->req_stg1_base, pdata->req_stg2_base,
			pdata->req_stg3_base);
}

/*
 *	This is used to set the DAC calibration offset baseline CDC value.
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param size The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t calib_target_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 stg_num = 0;
	u16 val = 0;
	s32 err = 0;
	u16 dummy;

	err = cmd_parsing(buf, &stg_num, &val, &dummy, 0);
	if (err < 0) {
		dev_err(adux1050->dev, "%s, kstrtos16 failed\n", __func__);
		return err;
	}
	if ((val <= MAX_CALIB_TARGET) && (val >= MIN_CALIB_TARGET)) {
		switch (stg_num) {
		case STG_ZERO:
			adux1050->pdata->req_stg0_base = val;
			break;
		case STG_ONE:
			adux1050->pdata->req_stg1_base = val;
			break;
		case STG_TWO:
			adux1050->pdata->req_stg2_base = val;
			break;
		case STG_THREE:
			adux1050->pdata->req_stg3_base = val;
			break;
		default:
		    dev_dbg(adux1050->dev, "%s,Invalid stg no %d\n",
			     __func__, stg_num);
		}
	} else {
		dev_err(adux1050->dev, "[%d,%d] Limit exceeded(%d)\n",
			MAX_CALIB_TARGET, MIN_CALIB_TARGET, val);
	}

	return size;
}

/*
 * This is used to get register address whose data is to be read and
 * count of data to be read via sysfs
 * This function Reads the value at the Device's Register for the i2c client
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This called when the user requires to read the configuration
 * \note This is evoked upon an echo request in the /sys/.../devices region.
 * \note it hold the register address to be read.
 * @return The Size of the Read Register 0 if not read
 * @see cmd_parsing
 */
static ssize_t store_reg_read(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	s32 ret;
	u16 addr;
	u16 cnt = 0;
	u16 val = 0;
	s16 err = 0;
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);

	mutex_lock(&adux1050->mutex);
	adux1050->stored_data_cnt = 0;
	ret = cmd_parsing(buf, &addr, &cnt, &val, 0);
	if (cnt == 0) {
		dev_err(adux1050->dev, "[ADUX1050]: Invalid COM/ARG\n");
		goto error;
	} else if (ret == -ERANGE || (addr+cnt > HIGHEST_READ_REG+1)) {
		dev_err(adux1050->dev, "[ADUX1050]: Values not in RANGE\n");
		goto error;
	} else if ((ret == -EINVAL) || (cnt > MAX_ADUX1050_WR_LEN)) {
		dev_err(adux1050->dev, "[ADUX1050]: Invalid COMD/ARG\n");
		goto error;
	} else {
		val = 0;
	}
	memset(adux1050->stored_reg_data, 0, sizeof(adux1050->stored_reg_data));

	err = adux1050->read(adux1050->dev, addr,
			     adux1050->stored_reg_data, cnt);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d\n", err);
		goto error;
	}
	adux1050->stored_data_cnt = cnt;
#if defined(__AWFUL_LOG__)
	for (u16 lp_cnt = 0; lp_cnt < adux1050->stored_data_cnt; lp_cnt++) {
		ADUX1050_LOG("Reg Read cmd:reg 0x%04x Data 0x%04x\n",
				    addr + lp_cnt,
				    adux1050->stored_reg_data[lp_cnt]);
	}
#endif
error:
	mutex_unlock(&adux1050->mutex);
	return count;
}

/*
 * This is used to read the data of the register address via sysfs sent to
 *	reg_read
 * This function Reads the value at the Device's Register for the given
 * client and Prints it in the output window
 * @param dev The Device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat request in the /sys/.../devices region.
 * @return The Size of the read data, 0 if not read
 *
 * @see dev_get_drvdata
 * @see store_reg_read
 */
static ssize_t show_reg_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	s32 val = 0, lp_cnt = 0;
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);

	for (lp_cnt = 0; lp_cnt < adux1050->stored_data_cnt; lp_cnt++) {
		val += snprintf(buf + val, PAGE_SIZE, "0x%x ",
				adux1050->stored_reg_data[lp_cnt]);
	}
	return val;
}

/*
 * This is used to write data to a register through i2c.
 * This functions Writes the value of the buffer to the given client
 * provided the count value to write
 * @param dev The device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param count The number of bytes to write from the buffer
 * @param buf The buffer to store the Read data
 * \note This is used to store the register address to write the data.
 * \note This is evoked upon an echo request in the /sys/.../devices region.
 * \note This also prints the command received before writing the Registers.
 * @return The Size of the writen Data, 0 if not writen
 */
static ssize_t store_reg_write(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 addr, cnt;
	u16 wr_data[MAX_ADUX1050_WR_LEN], loop_cnt;
	s32 ret;
	s32 err = 0;

	mutex_lock(&adux1050->mutex);
	ret = cmd_parsing(buf, &addr, &cnt, &wr_data[0], MAX_ADUX1050_WR_LEN);
	if (ret == -ERANGE || (ret == -EINVAL)) {
		dev_err(adux1050->dev, "%s - Values not in RANGE\n", __func__);
		goto error;
	} else if ((addr >= BASELINE_STG0_REG &&
		    addr <= GET_BASE_LINE_REG(STG_THREE)) &&
		   (addr+cnt <= GET_BASE_LINE_REG(STG_THREE) + 1)) {
		ADUX1050_LOG("%s Baseline write register\n", __func__);

	} else if (((addr + cnt) > (HIGHEST_WR_ACCESS + 1)) ||
		   (addr == ZERO_VAL)) {
		dev_err(adux1050->dev, "%s - Addr reaches RD only regs\n",
			__func__);
		goto error;
	} else if (cnt > MAX_ADUX1050_WR_LEN) {
		dev_err(adux1050->dev, "%s - Max write length\n", __func__);
		goto error;
	}
	ADUX1050_LOG("Register Write command :reg= 0x%x, size= %d\n",
			    addr, cnt);

	for (loop_cnt = 0; loop_cnt < cnt; loop_cnt++) {

		/* Conditions to be checked */
		/*   1.SW RESET, FORCE CALIB, AUTO_Threshold, power state ... */
		if ((addr + loop_cnt) == CTRL_REG) {
			/* Clearing SW Reset bit */
			if (CHK_SW_RESET_EN(wr_data[loop_cnt])) {
				ADUX1050_LOG("S/W reset not allowed");
				ADUX1050_LOG("- Use reset sysfs!!!\n");
				wr_data[loop_cnt] =
					CLR_SW_RESET_EN(wr_data[loop_cnt]);
			}
			/*Device is not allowed to wakeup */
			/*  during drv disable state*/
			if (adux1050->dev_enable == ADUX1050_DISABLE) {
				if (GET_PWR_MODE(wr_data[loop_cnt]) !=
				    PWR_STAND_BY) {
					/* PWR saved, used in next enable*/
					adux1050->ctrl_reg = wr_data[loop_cnt];
					adux1050->power_mode_flag =
							ADUX1050_ENABLE;
					wr_data[loop_cnt] =
				SET_PWR_MODE(wr_data[loop_cnt], PWR_STAND_BY);
				}
			}
		} else if (addr + loop_cnt == BASELINE_CTRL_REG) {
			if (CHK_FORCE_CALIB_EN(wr_data[loop_cnt])) {
				ADUX1050_LOG("FORCE CAL not allowed-");
				ADUX1050_LOG("Use force calib sysfs\n");
				wr_data[loop_cnt] =
					CLR_FORCE_CALIB_EN(wr_data[loop_cnt]);
			}
		}

		/* Storing local values before writing to registers */
		if (addr+loop_cnt <= MAX_ADUX1050_WR_LEN+1) {
			adux1050->reg[addr+loop_cnt].wr_flag = ADUX1050_ENABLE;
			adux1050->reg[addr+loop_cnt].value = wr_data[loop_cnt];
		} else if ((addr >= BASELINE_STG0_REG &&
				addr <= GET_BASE_LINE_REG(STG_THREE)) &&
			(addr+cnt <= GET_BASE_LINE_REG(STG_THREE) + 1)) {
			if (!CHK_AUTO_TH_ENABLED(
				adux1050->reg[BASELINE_CTRL_REG].value)) {
				adux1050->bs_reg
				[addr-BASELINE_STG0_REG + loop_cnt].wr_flag
							= ADUX1050_ENABLE;
				adux1050->bs_reg
				[addr-BASELINE_STG0_REG + loop_cnt].value
							= wr_data[loop_cnt];
			}
		}
		ADUX1050_LOG("DATA = 0x%04X\n", wr_data[loop_cnt]);
	}
	err = adux1050->write(adux1050->dev, addr, wr_data, cnt);
	if (err < DEF_WR)
		dev_err(adux1050->dev, "I2C WR Err %d\n", err);
error:
	mutex_unlock(&adux1050->mutex);
	return count;
}

/*
 * This is used to update the stage details upon every change in configuration
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_update_config(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 val;

	if (!adux1050->dev_enable) {
		ADUX1050_LOG("Device is not enabled\n");
		return count;
	}
	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s, kstrtos16 failed\n", __func__);
		return err;
	}
	if (val == 1) {
		mutex_lock(&adux1050->mutex);
		err = get_intr_mask_info(adux1050);
		err = getstageinfo(adux1050);
		if (err < ZERO_VAL)
			dev_err(adux1050->dev,
					"Err in getstageinfo %d\n", err);
		mutex_unlock(&adux1050->mutex);
		msleep(adux1050->slp_time_conv_complete);
		adux1050->slp_time_conv_complete =
				get_conv_time(adux1050, TWICE_CONV_DELAY_TIME);
	} else {
		ADUX1050_LOG("Invalid Input %d\n", val);
	}

	return count;
}

/*
 * This is used to do Software reset of the device of ADUX1050
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_sw_reset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 ctrl_reg_val;
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s - kstrtos16 failed\n", __func__);
		return err;
	}
	if (val == 1) {
		mutex_lock(&adux1050->mutex);
		/* SW reset is enabled in ctrl register */
		err = adux1050->read(adux1050->dev, CTRL_REG,
				     &ctrl_reg_val, DEF_WR);
		if (err < I2C_WRMSG_LEN) {
			dev_err(adux1050->dev, "I2C RD Err %d\n", err);
			goto error;
		}
		ctrl_reg_val = SET_SW_RESET_EN(ctrl_reg_val);
		err = adux1050->write(adux1050->dev, CTRL_REG,
				&ctrl_reg_val, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C WR Err %d\n", err);
			goto error;
		}
		msleep(FORCE_CALIB_SLEEP_TIME);
		/*Device is put to disable mode */
		if (adux1050->dev_enable == ADUX1050_ENABLE) {
			adux1050->dev_enable = ADUX1050_DISABLE;
			adux1050->proxy_cancel = _TRUE;
			cancel_delayed_work(&adux1050->proxy_work);
#ifndef CONFIG_ADUX1050_POLL
			disable_irq_wake(adux1050->irq);
			disable_irq(adux1050->irq);
#endif
		}
error:
		mutex_unlock(&adux1050->mutex);
	} else {
		ADUX1050_LOG("%s - Invalid input %d\n", __func__, val);
	}
	return count;
}

/*
 * This is used to do factory calibration of the ADUX1050
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_force_calib(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 baseline_ctrl_val;
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s, kstrtos16 failed\n", __func__);
		return err;
	}
	if (val == 1) {
		/* Force Calibration is enabled in Baseline ctrl register */
		/*TODO: Check AUTO-TH is enable or not */
		/*TODO: Can the Force calibratin be done*/
		/*   when the device is at DISABLED state?*/
		mutex_lock(&adux1050->mutex);
		err = adux1050->read(adux1050->dev, BASELINE_CTRL_REG,
			       &baseline_ctrl_val, DEF_WR);
		if (err < I2C_WRMSG_LEN) {
			dev_err(adux1050->dev, "I2C RD Err %d\n", err);
			goto error;
		}
		baseline_ctrl_val = SET_FORCE_CALIB_EN(baseline_ctrl_val);
		err = adux1050->write(adux1050->dev, BASELINE_CTRL_REG,
				&baseline_ctrl_val, DEF_WR);
		if (err < DEF_WR) {
			dev_err(adux1050->dev, "I2C RD Err %d\n", err);
			goto error;
		}
		msleep(FORCE_CALIB_SLEEP_TIME);
error:
		mutex_unlock(&adux1050->mutex);
	} else {
		ADUX1050_LOG("%s,Value is invalid\n", __func__);
	}
	return count;
}

#endif

static ssize_t store_offset_dac_write(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	u16 addr, cnt;
	u16 wr_data[MAX_ADUX1050_WR_LEN];
	s32 ret;
	s32 err = 0;

	ret = cmd_parsing(buf, &addr, &cnt, &wr_data[0], MAX_ADUX1050_WR_LEN);
	if (ret == -ERANGE || (ret == -EINVAL)) {
		dev_err(adux1050->dev, "%s - Values not in RANGE\n", __func__);
		return count;
	} else if (cnt != 1) {
		dev_err(adux1050->dev, "%s - length not 1\n", __func__);
		return count;
	}
	switch (addr) {
	case GET_OFFSET_REG(0):
	case GET_OFFSET_REG(1):
		break;	/* right address */
	default:
		dev_err(adux1050->dev, "%s - wrong address\n", __func__);
		return count;
	}

	ADUX1050_LOG("Offset Dac Write: reg= 0x%x, value= 0x%x\n",
		addr, wr_data[0]);

	mutex_lock(&adux1050->mutex);
	/* Storing local values before writing to registers */
	adux1050->reg[addr].wr_flag = ADUX1050_ENABLE;
	adux1050->reg[addr].value = wr_data[0];
	ADUX1050_LOG("DATA = 0x%04X\n", wr_data[0]);
	err = adux1050->write(adux1050->dev, addr, wr_data, cnt);
	if (err < DEF_WR)
		dev_err(adux1050->dev, "I2C WR Err %d\n", err);
	mutex_unlock(&adux1050->mutex);
	return count;
}

#ifdef CONFIG_ADUX1050_POLL

/*
 * This is used to show the poll delay used by the driver
 * @param dev The Device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat request in the /sys/.../devices region.
 * @return The Size of the read data, 0 if not read
 */
static ssize_t show_poll_delay(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);

	dev_dbg(adux1050->dev, "Poll delay =%x", adux1050->poll_delay);
	return snprintf(buf, PAGE_SIZE, "%d\n", adux1050->poll_delay);
}





/*
 * This is used to set the poll delay for the adux1050 kthread which is running
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_poll_delay(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		ADUX1050_LOG("%s -kstrou16 is not done\n", __func__);
		return err;
	}
	if (val <= MIN_POLL_DELAY)
		val = MIN_POLL_DELAY;
	adux1050->poll_delay = val;
	return count;
}
#endif

#ifdef CONFIG_ADUX1050_EVAL

/*
 * This is used to read error threshold byte via sysfs to do the force
 *	calibration
 * @param dev The Device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the error threshold byte
 * \note This is evoked upon an cat request in the /sys/.../devices region.
 * @return The Size of the read data, 0 if not read
 */
static ssize_t show_intr_err(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);

	dev_dbg(adux1050->dev, "intr_err =%x", adux1050->user_intr_err);
	return snprintf(buf, PAGE_SIZE, "%d\n", adux1050->user_intr_err);
}

/*
 * This is used to set the error threshold byte for the adux1050
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_intr_err(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s8 err;
	u8 val;

	err = kstrtou8(buf, 0, &val);
	if (err < 0) {
		ADUX1050_LOG("%s -kstrou8 is not done\n", __func__);
		return err;
	}
	mutex_lock(&adux1050->mutex);
	adux1050->user_intr_err = val;
	mutex_unlock(&adux1050->mutex);
	return count;
}

/*
 * This is used to read the maximum count and work handle frequency.
 * This functions Reads the value at the Device's Register for the given
 * client and Prints it in the output window
 * @param dev The Device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat request in the /sys/.../Device region.
 * @return The maximum count and set set time.
 */
static ssize_t  show_proxy_time(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);

	dev_dbg(adux1050->dev,
		 " - max_count%d\n proxy_enable bit freq: %d sec\n",
		  adux1050->max_proxy_count, adux1050->allowed_proxy_time);
	return snprintf(buf, PAGE_SIZE,
			"%d\t%d\n",
			adux1050->allowed_proxy_time,
			adux1050->max_proxy_count);
}

/*
 * This is used to set proxy time for force calibration
 * @param dev The device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The number of bytes to write from the buffer
 * \note This is used to store the register address to write the data.
 * \note This is evoked upon an echo request in the /sys/.../Device region.
 * \note This also prints the set time and loop count.
 * @return The Size of the writen Data, 0 if not writen
 */
static ssize_t store_proxy_time(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	s16 ret;
	u16 val = 0;
	u16 cnt = 0;
	u16 data = 0;
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);

	mutex_lock(&adux1050->mutex);
	ret = cmd_parsing_for_proxy(buf, &val, &cnt, &data, 2);
	if (cnt == 0) {
		dev_err(adux1050->dev, "[ADUX1050]: Invalid  cnt COM/ARG\n");
		goto error;
	} else if (val == 0) {
		dev_err(adux1050->dev, "[ADUX1050]: Invalid COM/ARG\n");
		goto error;
	} else if (ret == -ERANGE) {
		dev_err(adux1050->dev, "[ADUX1050]: Values not in RANGE\n");
		goto error;
	} else if (ret == -EINVAL) {
		dev_err(adux1050->dev, "[ADUX1050]: Invalid COMD/ARG\n");
		goto error;
	} else {
		data = 0;
	}
	adux1050->proxy_count = 0;
	adux1050->allowed_proxy_time = val;
	adux1050->max_proxy_count = cnt;
	if ((adux1050->proxy_enable) && (adux1050->dev_enable)) {
		adux1050->proxy_cancel = _TRUE;
		cancel_delayed_work(&adux1050->proxy_work);
		ADUX1050_LOG("%s -proxy_time is set\n", __func__);
		adux1050->proxy_cancel = _FALSE;
		schedule_delayed_work(&adux1050->proxy_work,
				      msecs_to_jiffies
		(PROXY_TIME * adux1050->allowed_proxy_time));
	}
	dev_dbg(adux1050->dev, "val:%d\tcnt:%d\tset time =%usec\n",
		 adux1050->allowed_proxy_time,
		 adux1050->max_proxy_count,
		 adux1050->max_proxy_count * adux1050->allowed_proxy_time);
error:
	mutex_unlock(&adux1050->mutex);
	return count;
}

/*
 * This is used to read the set time and remaining time to do force calibration.
 * This functions Reads the value at the Device's Register for the given
 * client and Prints it in the output window
 * @param dev The Device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * \note This is evoked upon an cat request in the /sys/.../Device region.
 * @return The remaining time and total time.
 */
static ssize_t  show_proxy_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adux1050_chip  *adux1050 = dev_get_drvdata(dev);
	u32 time;
	u32 rem_time;

	time = (adux1050->proxy_count * adux1050->allowed_proxy_time);
	dev_dbg(adux1050->dev, "time elapsed =%dsec\n", time);
	rem_time = adux1050->allowed_proxy_time * adux1050->max_proxy_count -
		   time;
	dev_dbg(adux1050->dev, "proxy :%d\ttime left:%u sec\nset time:%u\n",
		 adux1050->proxy_enable, rem_time, adux1050->max_proxy_count *
		 adux1050->allowed_proxy_time);
	return snprintf(buf, PAGE_SIZE, "%d\t%u\t%u\n",	adux1050->proxy_enable,
			rem_time, adux1050->max_proxy_count *
			adux1050->allowed_proxy_time);
}

/*
 * This is used to enable the work function to moniter proxy time for force
 *	calib
 * if the proximity time is not set it will set it to default values
 * @param dev The device ID structure(linux standard argument)
 * @param attr standard linux device attributes to the ADUX1050
 * @param buf The buffer to store the read data
 * @param count The number of bytes to write from the buffer
 * \note This is used to store the register address to write the data.
 * \note This is evoked upon an echo request in the /sys/.../Device region.
 * \note This also prints proxy enable/disable.
 *  @return The Size of the writen Data, 0 if not writen
 */
static ssize_t store_proxy_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s8 err;
	u8 val;

	err = kstrtou8(buf, 0, &val);
	if (err < 0) {
		ADUX1050_LOG("%s - kstrou8 is not done\n", __func__);
		return err;
	}
	if (val > 1) {
		dev_dbg(adux1050->dev, "%s :Invalid- Enable:1 Disable:0\n",
			 __func__);
	} else if ((adux1050->proxy_enable == 1) && (val == 1)) {
		dev_dbg(adux1050->dev, "%s : proximity is already enabled\n",
			 __func__);
		goto error;
	} else {
		adux1050->proxy_count = 0;
		adux1050->proxy_enable = val;
		if ((adux1050->proxy_enable) && (adux1050->dev_enable)) {
			adux1050->proxy_cancel = _TRUE;
			cancel_delayed_work(&adux1050->proxy_work);
			ADUX1050_LOG("%s -proxy_correc_bit enabled\n",
					    __func__);
			adux1050->proxy_cancel = _FALSE;
			schedule_delayed_work(&adux1050->proxy_work,
					      msecs_to_jiffies
			(PROXY_TIME * adux1050->allowed_proxy_time));
		} else {
			adux1050->proxy_cancel = _TRUE;
			cancel_delayed_work(&adux1050->proxy_work);
			adux1050->proxy_count = 0;
			ADUX1050_LOG("%s -proxy_bit/device is disable\n",
					    __func__);
		}
	}
error:
	return count;
}

/*
 * This is used to set mask_cdc_interrupt for the adux1050
 * @param dev The Device Id structure
 * @param attr The Device attributes to the ADUX1050
 * @param buf The buffer to store the Read data
 * @param count The count of bytes to be transferred to the Buffer
 * \note This is evoked upon an write request in the /sys/.../devices region.
 * @return Returns the size of the data handled
 */
static ssize_t store_mask_cdc_interrupt(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adux1050_chip *adux1050 = dev_get_drvdata(dev);
	s32 err;
	u16 val;

	err = kstrtou16(buf, 0, &val);
	if (err < 0) {
		dev_err(adux1050->dev, "%s,kstrtoint failed\n", __func__);
		return err;
	}
	adux1050->pdata->mask_cdc_interrupt = (val == 0) ? false : true;
	dev_dbg(adux1050->dev, "%s - mask : %d\n", __func__,
		 adux1050->pdata->mask_cdc_interrupt);

	if (adux1050->dev_enable == ADUX1050_ENABLE) {
		/* disable once to apply the change immediately */
		adux1050_disable(adux1050);
		adux1050_enable(adux1050);
	}

	return count;
}

#endif

/*
 * The sysfs attributes used in the driver follows
 */
/*--------------------------------------------------------------*/
static DEVICE_ATTR(adux1050_enable, 0664, show_enable, store_enable);
/*--------------------------------------------------------------*/
static struct device_attribute dev_attr_sensor_name =
	__ATTR(adux1050_device_name, 0444, adux1050_name_show, NULL);
#ifdef CONFIG_ADUX1050_EVAL
static struct device_attribute dev_attr_sensor_vendor =
	__ATTR(adux1050_vendor,  0444, adux1050_vendor_show, NULL);
static struct device_attribute dev_attr_sensor_raw_data =
	__ATTR(adux1050_raw_data, 0664,
	       adux1050_raw_data_show, adux1050_raw_data_store);
static struct device_attribute dev_attr_sensor_send_event =
	__ATTR(adux1050_send_event, 0664,
	       adux1050_send_event_show, adux1050_send_event_store);
static struct device_attribute dev_attr_sensor_threshold =
	__ATTR(adux1050_threshold, 0664,
	       adux1050_threshold_show, adux1050_threshold_store);
static struct device_attribute dev_attr_sensor_calibration =
	__ATTR(adux1050_calibration, 0664,
	       adux1050_calibration_show, adux1050_calibration_store);
static struct device_attribute dev_attr_sensor_update_calib =
	__ATTR(adux1050_update_calib, 0220, NULL, adux1050_update_calib_store);
#ifdef CONFIG_ADUX1050_DUMP_REGS
static struct device_attribute dev_attr_sensor_dump =
	__ATTR(adux1050_status, 0444, show_dumpregs, NULL);
#endif
static struct device_attribute dev_attr_sensor_calib_target =
	__ATTR(adux1050_calib_target, 0664,
	       calib_target_show, calib_target_store);
static struct device_attribute dev_attr_sensor_reg_read =
	__ATTR(adux1050_reg_read, 0664, show_reg_read, store_reg_read);
static struct device_attribute dev_attr_sensor_reg_write =
	__ATTR(adux1050_reg_write, 0220, NULL, store_reg_write);
static struct device_attribute dev_attr_sensor_update_config =
	__ATTR(adux1050_update_config, 0220, NULL, store_update_config);
static struct device_attribute dev_attr_sensor_sw_reset =
	__ATTR(adux1050_sw_reset, 0220, NULL, store_sw_reset);
static struct device_attribute dev_attr_sensor_force_calib =
	__ATTR(adux1050_force_calib, 0220, NULL, store_force_calib);
#endif
static struct device_attribute dev_attr_sensor_offset_dac_write =
	__ATTR(adux1050_offset_dac_write, 0220, NULL, store_offset_dac_write);
#ifdef CONFIG_ADUX1050_POLL
static struct device_attribute dev_attr_sensor_poll =
	__ATTR(adux1050_poll_delay, 0664, show_poll_delay, store_poll_delay);
#endif
#ifdef CONFIG_ADUX1050_EVAL
static struct device_attribute dev_attr_sensor_fc_intr_err =
	__ATTR(adux1050_intr_error, 0664, show_intr_err, store_intr_err);
static struct device_attribute dev_attr_sensor_proxy_timer_enable =
	__ATTR(adux1050_proxy_enable, 0664,
	       show_proxy_enable, store_proxy_enable);
static struct device_attribute dev_attr_sensor_set_proxy_time =
	__ATTR(adux1050_proxy_time, 0664, show_proxy_time, store_proxy_time);
static struct device_attribute dev_attr_sensor_mask_cdc_interrupt =
	__ATTR(adux1050_mask_cdc_interrupt, 0664,
		NULL, store_mask_cdc_interrupt);
#endif
static struct attribute *adux1050_attrs[] = {
	&dev_attr_adux1050_enable.attr,
	&dev_attr_sensor_name.attr,
#ifdef CONFIG_ADUX1050_EVAL
	&dev_attr_sensor_vendor.attr,
	&dev_attr_sensor_raw_data.attr,
	&dev_attr_sensor_send_event.attr,
	&dev_attr_sensor_threshold.attr,
	&dev_attr_sensor_calibration.attr,
	&dev_attr_sensor_update_calib.attr,
#ifdef CONFIG_ADUX1050_DUMP_REGS
	&dev_attr_sensor_dump.attr,
#endif
	&dev_attr_sensor_calib_target.attr,
	&dev_attr_sensor_reg_read.attr,
	&dev_attr_sensor_reg_write.attr,
	&dev_attr_sensor_update_config.attr,
	&dev_attr_sensor_sw_reset.attr,
	&dev_attr_sensor_force_calib.attr,
#endif
	&dev_attr_sensor_offset_dac_write.attr,
#ifdef CONFIG_ADUX1050_POLL
	&dev_attr_sensor_poll.attr,
#endif
#ifdef CONFIG_ADUX1050_EVAL
	&dev_attr_sensor_fc_intr_err.attr,
	&dev_attr_sensor_proxy_timer_enable.attr,
	&dev_attr_sensor_set_proxy_time.attr,
	&dev_attr_sensor_mask_cdc_interrupt.attr,
#endif
	NULL,
};

static struct attribute_group adux1050_attr_group = {
	.name = NULL,
	.attrs = adux1050_attrs,
};

/*
 * This routine reads the Device ID to confirm the existence
 * of the Device in the System.
 * @param  adux1050 The Device structure
 * @return 0 on successful detection of the device,-ENODEV on err.
 */
static int adux1050_hw_detect(struct adux1050_chip *adux1050)
{
	u16 data;
	s16 err = 0;

	err = adux1050->read(adux1050->dev, DEV_ID_REG, &data, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d\n", err);
		return -ENODEV;
	}
	if (likely((data & ADUX1050_ID_MASK) == ADUX1050_GENERIC_ID)) {
		adux1050->product = PRODUCT_ID(data);
		adux1050->version = REV_ID(data);
		adux1050->metal_id = METAL_ID(data);
		if (adux1050->metal_id < MET_VER1)
			ADUX1050_DEV_LOG(adux1050->dev,
				 "Hopping Search Saturation Routine enabled");
		else if (adux1050->metal_id >= MET_VER1)
			ADUX1050_DEV_LOG(adux1050->dev,
				 "Binary Search Saturation Routine enabled");
		ADUX1050_DEV_LOG(adux1050->dev,
			"Found ADUX1050, rev:%x met_id:%x",
			 adux1050->version, adux1050->metal_id);
		return 0;
	}
	dev_err(adux1050->dev, "ADUX1050 Not Found,ID %4x\n", data);
	return -ENODEV;
}

#ifndef CONFIG_ADUX1050_POLL
/*
 * Threaded IRQ Handler -- Assigns interrupt handling to work
 * @param irq The Interrupt Request queue to be assigned for the device
 * @param handle The data of the ADUX1050 Device
 * @return IRQ_HANDLED
 */
static irqreturn_t adux1050_isr_thread(int irq, void *handle)
{
	s16 err = 0;
	struct adux1050_chip *adux1050 = handle;

	if (!work_pending(&adux1050->work)) {
		schedule_work(&adux1050->work);
	} else {
		mutex_lock(&adux1050->mutex);
		/*Cleared the interrupt for future intterupts to occur*/
		err = adux1050->read(adux1050->dev, INT_STATUS_REG,
				&adux1050->int_status, DEF_WR);
		if (err < I2C_WRMSG_LEN)
			dev_err(adux1050->dev, "I2C Read Error %d in %s",
				err, __func__);

		mutex_unlock(&adux1050->mutex);
	}
	return IRQ_HANDLED;
}
#endif

/*
 * conv_complete_cdc_fetch - Fetch the CDC for the connected stage after
 * receiving the conversion sequence complete interrupt is asserted
 * @param adux1050 The chip structure of ADUX1050 chip
 * @return void
 */
static inline void conv_complete_cdc_fetch(struct adux1050_chip *adux1050,
					u16 proxy_status)
{
	u8 stg_cnt = 0;
	u16 result_cdc;
	u16 baseline_cdc;
	s16 err = 0;
	unsigned int event_value = 0;

	for (stg_cnt = 0; stg_cnt < adux1050->num_stages; stg_cnt++) {
		/* Fetch the CDC only if that stage is connected */
		if (adux1050->stg_info[stg_cnt].status != CIN_CONNECTED)
			continue;
		err = adux1050->read(adux1050->dev, GET_RESULT_REG(stg_cnt),
					&result_cdc, DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C Read Error %d in %s",
					err, __func__);
		err = adux1050->read(adux1050->dev, GET_BASE_LINE_REG(stg_cnt),
					&baseline_cdc, DEF_WR);
		if (err < DEF_WR)
			dev_err(adux1050->dev, "I2C Read Error %d in %s",
					err, __func__);
		if (!adux1050->send_event)
			continue;
		event_value = PACK_FOR_CDC_EVENT(result_cdc, stg_cnt);
		input_event(adux1050->input, EV_MSC, MSC_RAW, event_value);
		event_value = PACK_FOR_BASELINE_EVENT(baseline_cdc, stg_cnt);
		input_event(adux1050->input, EV_MSC, MSC_RAW, event_value);
	}
	event_value = PACK_FOR_STATUS_EVENT(adux1050->int_status, proxy_status);
	input_event(adux1050->input, EV_MSC, MSC_RAW, event_value);
	input_sync(adux1050->input);
}

/*
 * This function is used to send ACTIVE event for a stage in ADUX1050
 * @param adux1050 The ADUX1050 chip structure pointer
 * @param stg_num The stage nmmber for which the event to be sent
 * @param threshold_type Indicates Low or High threshold
 * @return void
 */
static inline void indicate_active_state(struct adux1050_chip *adux1050,
					 int stg_num, int threshold_type)
{
	unsigned int event_value = 0;

	ADUX1050_LOG("%s\n", __func__);
	event_value = PACK_FOR_ACTIVE_STATE(stg_num, threshold_type);
	input_event(adux1050->input, EV_MSC, MSC_RAW, event_value);
	input_sync(adux1050->input);

}

/*
 * This function is used to send IDLE event for a stage in ADUX1050
 * @param adux1050 The ADUX1050 chip structure pointer
 * @param stg_num The stage nmmber for which the event to be sent
 * @param threshold_type Indicates Low or High threshold
 * @return void
 */
static inline void indicate_idle_state(struct adux1050_chip *adux1050,
				       int stg_num, int threshold_type)
{
	unsigned int event_value = 0;

	ADUX1050_LOG("%s\n", __func__);
	event_value = PACK_FOR_IDLE_STATE(stg_num, threshold_type);
	input_event(adux1050->input, EV_MSC, MSC_RAW, event_value);
	input_sync(adux1050->input);
}

/*
 * high_threshold_int_check - Identify which stage asserts the high threshold
 * interrupt.After identifying the stage, the state(ACTIVE,IDLE) of the stage
 * is sent as input event
 * @param	adux1050 The chip structure of ADUX1050 chip
 * @param	high_status_change Contains stage number which asserts INT.
 * @return void
 */
static inline void high_threshold_int_check(struct adux1050_chip *adux1050,
					    u16 high_status_change)
{
	u8 stg_cnt;
	u8 temp_ht_enable = adux1050->high_thresh_enable;
	u8 temp_ht_intr_err = GET_HIGH_STATUS(adux1050->user_intr_err);

	for (stg_cnt = 0; stg_cnt < TOTAL_STG; stg_cnt++) {
		if ((!(temp_ht_intr_err & 1)) && (temp_ht_enable & 1)) {
			if (high_status_change & 1) {
				if (adux1050->high_status & 1) {
					dev_dbg(adux1050->dev, "St = %de\n",
						 stg_cnt);
					if (adux1050->send_event)
						indicate_active_state(adux1050,
								      stg_cnt,
								      TH_HIGH);
				} else {
					dev_dbg(adux1050->dev, "St = %dx\n",
						 stg_cnt);
					if (adux1050->send_event)
						indicate_idle_state(adux1050,
								    stg_cnt,
								    TH_HIGH);
				}
			}
		}
		temp_ht_enable = temp_ht_enable >> 1;
		high_status_change = high_status_change >> 1;
		adux1050->high_status = adux1050->high_status >> 1;
		temp_ht_intr_err = temp_ht_intr_err >> 1;
	}
}

/*
 * Low_threshold_int_check - Identify which stage asserts the low threshold
 * interrupt.After identifying the stage, the state(ACTIVE,IDLE) of the stage
 * is sent as input event
 * @param	adux1050 The chip structure of ADUX1050 chip
 * @param	low_status_change Contains stage number which asserts INT.
 * @return void
 */
static inline void low_threshold_int_check(struct adux1050_chip *adux1050,
					   u16 low_status_change)
{
	u8 stg_cnt;
	u8 temp_lt_enable = adux1050->low_thresh_enable;
	u8 temp_intr_err = GET_LOW_STATUS(adux1050->user_intr_err);

	for (stg_cnt = 0; stg_cnt < TOTAL_STG; stg_cnt++) {
		if ((!(temp_intr_err & 1)) && (temp_lt_enable & 1)) {
			if (low_status_change & 1) {
				if (adux1050->low_status & 1) {
					dev_dbg(adux1050->dev, "St = %de\n",
						 stg_cnt);
					if (adux1050->send_event)
						indicate_active_state(adux1050,
								      stg_cnt,
								      TH_LOW);
				} else {
					dev_dbg(adux1050->dev, "St = %dx\n",
						 stg_cnt);
					if (adux1050->send_event)
						indicate_idle_state(adux1050,
								    stg_cnt,
								    TH_LOW);
				}
			}
		}
		temp_lt_enable = temp_lt_enable >> 1;
		low_status_change = low_status_change >> 1;
		adux1050->low_status = adux1050->low_status >> 1;
		temp_intr_err = temp_intr_err >> 1;
	}
}
/*
 * int adux1050_get_result_info(struct adux1050_chip *adux1050)
 * This function is used to get the CDC,base line, high threshold and
 * low threshold for the connected stages and prints the information.
 * @param adux1050 The chip structure of ADUX1050 driver
 * @return 0 on success.
 */
static int adux1050_get_result_info(struct adux1050_chip *adux1050)
{
	u16 rs_reg[TOTAL_STG];
	u16 ls_reg[TOTAL_STG];
	u16 hs_reg[TOTAL_STG];
	u16 base_reg[TOTAL_STG];
	u8 stg_cnt = 0;
	s16 err = 0;

	for (stg_cnt = 0 ; stg_cnt < TOTAL_STG ; stg_cnt++) {
		if (adux1050->stg_info[stg_cnt].status == CIN_CONNECTED) {
			err = adux1050->read(adux1050->dev,
					GET_RESULT_REG(stg_cnt),
					&rs_reg[stg_cnt], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d\n", err);
				return err;
			}
			err = adux1050->read(adux1050->dev,
					GET_HIGH_TH_REG(stg_cnt),
					&hs_reg[stg_cnt], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d\n", err);
				return err;
			}
			err = adux1050->read(adux1050->dev,
					GET_LOW_TH_REG(stg_cnt),
					&ls_reg[stg_cnt], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d\n", err);
				return err;
			}
			err = adux1050->read(adux1050->dev,
					GET_BASE_LINE_REG(stg_cnt),
					&base_reg[stg_cnt], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d\n", err);
				return err;
			}
			dev_dbg(adux1050->dev,
				"STG %x CDC %x\t BS %x\t HS %x\t LS %x\n",
				stg_cnt,
				rs_reg[stg_cnt],
				base_reg[stg_cnt],
				base_reg[stg_cnt] + hs_reg[stg_cnt],
				base_reg[stg_cnt] - ls_reg[stg_cnt]);
		}
	}
	return 0;
}

/*
 * Work Handler -- Handles the interrupt status from ADUX1050
 * @param isr_work The work structure for the ADUX1050 chip
 * @return void Nothing returned
 */
static void adux1050_isr_work_fn(struct work_struct *isr_work)
{
	struct adux1050_chip *adux1050 = NULL;
	u16 high_status_change = 0;
	u16 low_status_change = 0;
	u8 curr_intr_status = 0;
	u8 user_intr_error_not  = 0;
	u8 stg_cnt = 0;
	u8 sat_flag = 0;
	u16 rs_reg[TOTAL_STG];
	u16 proxy_stage_status = 0;
	s16 err = 0;

	if (isr_work == NULL) {
		pr_err("%s - isr_work is NULL\n", __func__);
		goto isr_null;
	}
	adux1050 = container_of(isr_work, struct adux1050_chip, work);
	mutex_lock(&adux1050->mutex);
	err = adux1050->read(adux1050->dev, INT_STATUS_REG,
		       &adux1050->int_status, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		goto fc_done;
	}
	dev_dbg(adux1050->dev, "status_reg : %x int_status :%x DEF_WR:%x\n",
		INT_STATUS_REG, adux1050->int_status, DEF_WR);
	adux1050->low_status = GET_LOW_STATUS(adux1050->int_status);
	adux1050->high_status = GET_HIGH_STATUS(adux1050->int_status);
	adux1050->conv_status = GET_CONV_STATUS(adux1050->int_status);
	dev_dbg(adux1050->dev, "HS:%x LS:%x CCS:%x\n", adux1050->high_status,
		 adux1050->low_status, adux1050->conv_status);
/* conditions to handle error conditions for force calibration	 */
	for (stg_cnt = 0; stg_cnt < TOTAL_STG; stg_cnt++) {
		if (adux1050->stg_info[stg_cnt].status == CIN_CONNECTED) {
			adux1050->read(adux1050->dev, GET_RESULT_REG(stg_cnt),
				       &rs_reg[stg_cnt], DEF_WR);
			if (err < I2C_WRMSG_LEN) {
				dev_err(adux1050->dev, "I2C RD Err %d in %s\n",
					err, __FILE__);
				goto fc_done;
			}
			((rs_reg[stg_cnt] < MIN_CALIB_TARGET) ||
			(rs_reg[stg_cnt] > MAX_CALIB_TARGET)) ?
				 (sat_flag += 1) : sat_flag;
		}
	}
	curr_intr_status = (u8) (adux1050->int_status & LOW_BYTE_MASK);
	err = adux1050->read(adux1050->dev, PROX_STATUS_REG,
		       &proxy_stage_status, DEF_WR);
	if (err < I2C_WRMSG_LEN) {
		dev_err(adux1050->dev, "I2C RD Err %d in %s\n", err, __FILE__);
		goto fc_done;
	}
	proxy_stage_status &=  LOW_NIBBLE_MASK;
	dev_dbg(adux1050->dev, "proximity status:%x",
		proxy_stage_status);
	if ((adux1050->user_intr_err & curr_intr_status) && (sat_flag == 0)) {
		user_intr_error_not = ~(adux1050->user_intr_err);
		if (!(curr_intr_status & user_intr_error_not)) {
			dev_dbg(adux1050->dev, "BEFORE FC:");
			err = adux1050_get_result_info(adux1050);
			if (err < ZERO_VAL) {
				dev_err(adux1050->dev, "get result fail %d\n",
					err);
				goto fc_done;
			}
			err = adux1050_force_cal(adux1050,
					  get_conv_time(adux1050, CONV_TIME));
			if (err < 0) {
				dev_err(adux1050->dev, "ERROR in FC %d\n", err);
				goto fc_done;
			}
			ADUX1050_LOG("%s -FC due to ERR condition**\n",
					    __func__);
			dev_dbg(adux1050->dev, "AFTER FC:");
			err = adux1050_get_result_info(adux1050);
			if (err < ZERO_VAL) {
				dev_err(adux1050->dev,
					"get result fail after FC %d\n", err);
				goto fc_done;
			}
		}
	}
	/* Handling High threshold interrupt */
	if (adux1050->high_thresh_enable) {
		high_status_change = ((adux1050->high_status) ^
				      (adux1050->prev_high_status));
		adux1050->prev_high_status = adux1050->high_status;
		if (high_status_change) {
			adux1050->proxy_count = 0;
			dev_dbg(adux1050->dev, "H THRES CHANGE:");
			adux1050_get_result_info(adux1050);
		}
		high_threshold_int_check(adux1050, high_status_change);
	}

	/* Handling Low threshold interrupt */
	if (adux1050->low_thresh_enable) {
		low_status_change = ((adux1050->low_status) ^
				    (adux1050->prev_low_status));
		adux1050->prev_low_status = adux1050->low_status;
		if (low_status_change) {
			adux1050->proxy_count = 0;
			dev_dbg(adux1050->dev, "L THRES CHANGE:");
			adux1050_get_result_info(adux1050);
		}
		low_threshold_int_check(adux1050, low_status_change);
	}
	/* Handling Conversion complete interrupt */
	if (adux1050->conv_enable && adux1050->conv_status)
		conv_complete_cdc_fetch(adux1050, proxy_stage_status);
fc_done:
	mutex_unlock(&adux1050->mutex);
isr_null:
	err = 0;
}
/*
 * Delayed work Handler -- Handles the proximity status from ADUX1050
 * issues the force calibration when the condition meets.
 * @param proxy_dwork The work structure for the ADUX1050 chip
 * @return void Nothing returned
 */
static void adux1050_proxy_work(struct work_struct *proxy_dwork)
{
	s16 err = 0;
	struct adux1050_chip *adux1050 = NULL;
	u16 proxy_stage_status;
	u16 int_status;

	if (proxy_dwork == NULL) {
		pr_err("%s - _work is NULL\n", __func__);
		goto proxy_null;
	}
	adux1050 = container_of((struct delayed_work *)
						       proxy_dwork,
						       struct adux1050_chip,
						       proxy_work);
	mutex_lock(&adux1050->mutex);
	ADUX1050_LOG("%s -in delayed proxy_work!!!!!!\n", __func__);
	err = adux1050->read(adux1050->dev, INT_STATUS_REG,
			     &int_status, DEF_WR);
	if (err < I2C_WRMSG_LEN)
		dev_err(adux1050->dev, "I2C READ FAILED %d at %s",
			err, __func__);
	int_status &= LOW_BYTE_MASK;
	err = adux1050->read(adux1050->dev, PROX_STATUS_REG,
			     &proxy_stage_status, DEF_WR);
	if (err < I2C_WRMSG_LEN)
		dev_err(adux1050->dev, "I2C READ FAILED %d at %s",
			err, __func__);
	proxy_stage_status &= LOW_NIBBLE_MASK;
	dev_dbg(adux1050->dev, "proximity status:%x", proxy_stage_status);
	if ((int_status == 0) && (proxy_stage_status > 0)) {
		adux1050->proxy_count++;
		dev_dbg(adux1050->dev, "proxy_count:%x",
			 adux1050->proxy_count);
		if (adux1050->proxy_count >= adux1050->max_proxy_count) {
			adux1050->proxy_count = 0;
			ADUX1050_LOG("%s -FC due to time limit*\n", __func__);
			err = adux1050_force_cal(adux1050,
					   adux1050->slp_time_conv_complete);
			dev_dbg(adux1050->dev, "slp_time in FC:%x",
				 adux1050->slp_time_conv_complete);
			if (err < 0)
				dev_err(adux1050->dev,
					"FC I2C READ FAILED %d at %s",
					err, __func__);
		}
	} else {
		adux1050->proxy_count = 0;
	}
	mutex_unlock(&adux1050->mutex);
	if (!adux1050->proxy_cancel) {
		schedule_delayed_work(&adux1050->proxy_work,
				      msecs_to_jiffies(PROXY_TIME *
						adux1050->allowed_proxy_time));
	}
proxy_null:
	err = 0;
}
#ifdef CONFIG_ADUX1050_POLL
/*
 * adux1050_polling_thread_fn -- Run function of the ADUX1050 Kthread
 * @param chip_data - Hold the pointer of ADUX1050 chip structure
 * @return int - Returns Zero
 */
static int adux1050_polling_thread_fn(void *chip_data)
{
	s16 err = 0;
	struct adux1050_chip *adux1050 = chip_data;

	ADUX1050_LOG("Polling Thread Started\n");
	while (!kthread_should_stop()) {
		if (adux1050->dev_enable == ADUX1050_ENABLE) {
			if (adux1050->dac_calib.cal_flags != CAL_RET_PENDING) {
				if (!work_pending(&adux1050->work)) {
					schedule_work(&adux1050->work);
				} else {
					mutex_lock(&adux1050->mutex);
					/*Clear interrupt for future to occur*/
					err = adux1050->read(adux1050->dev,
						     INT_STATUS_REG,
						     &adux1050->int_status,
						     DEF_WR);
					if (err < I2C_WRMSG_LEN)
						dev_err(adux1050->dev,
							"I2C RD FAIL %d at %s",
							err, __func__);
					mutex_unlock(&adux1050->mutex);
				}
			}
			if (likely(adux1050->poll_delay <= MSLEEP_MIN_TIME))
				usleep_range(adux1050->poll_delay * MS_TO_US,
					     adux1050->poll_delay * MS_TO_US);
			else
				msleep(adux1050->poll_delay);

		} else {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule();
		}
	}
	return 0;
}
#endif

/*
 *
 */
static bool read_value_from_device_tree(struct adux1050_chip *adux1050,
		const char *id, unsigned short *read_value)
{
	s32 df_prop_length = 0;
	const __be32 *param = NULL;

	param = of_get_property(adux1050->dt_device_node, id, &df_prop_length);
	if (!param || (df_prop_length != sizeof(int)))
		return false;
	*read_value = be32_to_cpu(*param);
	ADUX1050_LOG("%s read from DT, %s:%d\n", __func__, id, *read_value);
	return true;
}

/*
 * Device probe function  all initialization routines are handled here like
 * the ISR registration or the polling thead registeration,Work creation,
 * Input device registration, SYSFS attributes creation etc.
 * @param client the i2c structure of the adux1050 device/client.
 * @param id The i2c_device_id for the supported i2c device.
 * @return 0 on success,and On failure -ENOMEM, -EINVAL ,etc., will be returned
 */
static int adux1050_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	s32 ret = -EINVAL;
	struct input_dev *input = NULL;
	struct device *dev = NULL;
	struct adux1050_chip *adux1050 = NULL;

	ADUX1050_LOG("%s called", __func__);

	if (client == NULL) {
		pr_err("%s: Client/client->dev doesn't exist\n", __func__);
		return ret;
	}
	dev = &client->dev;

#ifndef CONFIG_ADUX1050_POLL
	if (client->irq <= 0) {
		pr_err("%s: IRQ not configured!\n", __func__);
		return ret;
	}
#endif

	adux1050 = kzalloc(sizeof(*adux1050), GFP_KERNEL);
	if (!adux1050) {
		/*WARNING: Possible unnecessary 'out of memory' message*/
		/*pr_err("%s: Memory alloc fail - Chip struct\n", __func__);*/
		return -ENOMEM;
	}

	adux1050->stg_info =
		kzalloc((sizeof(struct adux1050_stage_info) * TOTAL_STG),
			GFP_KERNEL);
	if (!adux1050->stg_info) {
		/*WARNING: Possible unnecessary 'out of memory' message*/
		/*pr_err("%s: Memory Alloc fail - Stage info\n", __func__);*/
		return -ENOMEM;
	}

	adux1050->pdata =
		kzalloc(sizeof(struct adux1050_platform_data), GFP_KERNEL);
	if (!adux1050->pdata) {
		/*WARNING: Possible unnecessary 'out of memory' message*/
		/*pr_err("%s: Memory alloc fail - platform data\n", __func__);*/
		return -ENOMEM;
	}
	*adux1050->pdata = local_plat_data;
#ifdef CONFIG_OF
	if (client->dev.of_node != NULL) {
		struct adux1050_platform_data *pdata = adux1050->pdata;
		s32 df_prop_length = 0;
		const __be32 *irqf = NULL;
		unsigned short value = 0;
		bool is_read = false;

		adux1050->dt_device_node = client->dev.of_node;
		ADUX1050_LOG("%s: DT node Found\n", __func__);
		irqf = of_get_property(adux1050->dt_device_node,
			  "adi,adux1050_irq_flags", &df_prop_length);
		if (irqf && (df_prop_length == sizeof(int))) {
			adux1050->pdata->irq_flags = be32_to_cpu(*irqf);
			ADUX1050_LOG("%s Usg irq_flag from DT %d\n",
				__func__, adux1050->pdata->irq_flags);
		}
		is_read = read_value_from_device_tree(adux1050,
				"adi,adux1050_irq_gpio_no", &value);
		if (is_read) {
			pdata->is_set_irq_gpio_no = true;
			pdata->irq_gpio_no = value;
		}
		/*initialize the force calib variables*/
		is_read = read_value_from_device_tree(adux1050,
			"adi,adux1050_proxy_enable", &value);
		if (is_read)
			pdata->proxy_enable = value;
		is_read = read_value_from_device_tree(adux1050,
			"adi,adux1050_allowed_proxy_time", &value);
		if (is_read)
			pdata->allowed_proxy_time = value;
		is_read = read_value_from_device_tree(adux1050,
			"adi,adux1050_max_proxy_count", &value);
		if (is_read)
			pdata->max_proxy_count = value;
		is_read = read_value_from_device_tree(adux1050,
			"adi,adux1050_intr_err", &value);
		if (is_read)
			pdata->intr_err = value;
		is_read = read_value_from_device_tree(adux1050,
			"adi,adux1050_device_identifier", &value);
		if (is_read)
			snprintf(pdata->device_name, DEVICE_NAME_MAX,
				"%s_id%d", DEVICE_NAME, value);
		else
			snprintf(pdata->device_name, DEVICE_NAME_MAX,
				"%s_XX", DEVICE_NAME);
		ADUX1050_LOG("device name is %s\n", pdata->device_name);
	} else {
		ADUX1050_LOG("%s - Usg local pltfm data\n", __func__);
	}
#endif

	adux1050->read = adux1050_i2c_read;
	adux1050->write = adux1050_i2c_write;
	adux1050->dev = dev;
	mutex_init(&adux1050->mutex);

	/* check if the device is existing by reading device id of ADUX1050 */
	ret = adux1050_hw_detect(adux1050);
	if (ret)
		goto err_kzalloc_mem;

	i2c_set_clientdata(client, adux1050);

	INIT_WORK(&adux1050->work, adux1050_isr_work_fn);
	INIT_WORK(&adux1050->calib_work, adux1050_calibration);
	INIT_DELAYED_WORK(&adux1050->proxy_work, adux1050_proxy_work);
	/*
	 * Allocate and register adux1050 input device
	 */
	input = input_allocate_device();
	if (!input) {
		pr_err("%s: could not allocate input device\n", __func__);
		ret = -ENOMEM;
		goto err_kzalloc_mem;
	}

	input->name = adux1050->pdata->device_name;
	set_bit(EV_MSC, input->evbit);
	input_set_capability(input, EV_MSC, MSC_RAW);

	ret = input_register_device(input);
	if (ret) {
		pr_err("%s: could not input_register_device(input);\n",
		       __func__);
		input_free_device(input);
		goto err_kzalloc_mem;
	}
	adux1050->input = input;
	input_set_drvdata(adux1050->input, adux1050);
#ifdef CONFIG_ADUX1050_POLL
		/* Creation of Thread */
	adux1050->polling_task = kthread_create(adux1050_polling_thread_fn,
					(void *)adux1050, "adux1050_kthread");
	if (ERR_PTR(-ENOMEM) == adux1050->polling_task) {
		pr_err("%s : Thread Creation Failed\n", __func__);
		goto err_free_irq;
	}
	if (adux1050->dev_enable == ADUX1050_ENABLE) {
		pr_err("%s : before wake up\n", __func__);
		wake_up_process(adux1050->polling_task);
		pr_err("%s :after wake up\n", __func__);
	}
	adux1050->poll_delay = MIN_POLL_DELAY;
	adux1050->int_pol = ACTIVE_LOW;
#else
	adux1050->irq = client->irq;
	if (!adux1050->pdata->irq_flags)
		adux1050->pdata->irq_flags = IRQF_TRIGGER_FALLING;

	ADUX1050_LOG("%s - Before request_threaded_irq %d\n",
			    __func__, adux1050->irq);
	ret = request_threaded_irq(adux1050->irq, NULL, adux1050_isr_thread,
				   IRQF_ONESHOT | adux1050->pdata->irq_flags,
				   adux1050->pdata->device_name, adux1050);

	if (ret) {
		pr_err("%s: irq %d Driver init Failed", __func__,
		       adux1050->irq);
		goto err_free_irq;
	}
	disable_irq(adux1050->irq);
	if (adux1050->pdata->irq_flags & IRQF_TRIGGER_RISING)
		adux1050->int_pol = ACTIVE_HIGH;
	else
		adux1050->int_pol = ACTIVE_LOW;

	if (adux1050->pdata->is_set_irq_gpio_no) {
		unsigned int gpioNo = adux1050->pdata->irq_gpio_no;

		ADUX1050_LOG("%s: gpioNo %d\n", __func__, gpioNo);
		ret = gpio_request(gpioNo, "aDuX1050");
		if (ret) {
			pr_err("%s: gpio_request: %d\n", __func__, ret);
			goto err_free_irq;
		}
		ret = gpio_direction_input(gpioNo);
		if (ret) {
			pr_err("%s: gpio_direction_input: %d\n", __func__, ret);
			goto err_free_gpio;
		}
	}
	adux1050->pdata->mask_cdc_interrupt = true;
#endif
	ret = sysfs_create_group(&dev->kobj, &adux1050_attr_group);
	if (ret) {
		pr_err("%s: could not register sensor sysfs\n", __func__);
		goto err_sysfs_create_input;
	}

	/* initialize and request sw/hw resources */
	adux1050_store_register_values(adux1050);
	adux1050->dev_enable = ADUX1050_DISABLE;
	adux1050->send_event = ADUX1050_ENABLE;
	/*initialize the force calib variables*/
	adux1050->proxy_enable = adux1050->pdata->proxy_enable;
	adux1050->allowed_proxy_time = adux1050->pdata->allowed_proxy_time;
	adux1050->max_proxy_count = adux1050->pdata->max_proxy_count;
	adux1050->user_intr_err = adux1050->pdata->intr_err;
	adux1050->proxy_cancel = _TRUE;
#ifdef CONFIG_ADUX1050_EVAL
	adux1050->power_mode_flag = ADUX1050_DISABLE;
#endif
	ADUX1050_DEV_LOG(adux1050->dev, "%s Completed\n", __func__);
	return 0;

err_sysfs_create_input:
	sysfs_remove_group(&dev->kobj, &adux1050_attr_group);
#ifdef CONFIG_ADUX1050_POLL
	kthread_stop(adux1050->polling_task);
#else
	free_irq(adux1050->irq, adux1050);
#endif
err_free_gpio:
	if (adux1050->pdata->is_set_irq_gpio_no)
		gpio_free(adux1050->pdata->irq_gpio_no);
err_free_irq:
	input_unregister_device(input);

err_kzalloc_mem:
	kfree(adux1050->pdata);
	kfree(adux1050->stg_info);
	kfree(adux1050);
	return ret;
}

/*
 * Removes the device.
 * This is used to remove the device or the I2C client from the system
 * @param client The client structure to be removed
 * @return 0 on success
 */
static int adux1050_i2c_remove(struct i2c_client *client)
{
	struct adux1050_chip *adux1050 = i2c_get_clientdata(client);
	u16 data = DISABLE_DEV_INT;
	s16 err = 0;

	pr_debug("%s, Start\n", __func__);
	if (adux1050 != NULL) {
		adux1050->proxy_cancel = _TRUE;
		if (adux1050->dev_enable == ADUX1050_ENABLE) {
			#ifdef CONFIG_ADUX1050_POLL
			#else
				disable_irq(adux1050->irq);
				disable_irq_wake(adux1050->irq);
			#endif
			err = adux1050->write(adux1050->dev, INT_CTRL_REG,
					&data, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C Write failed %d at %s\n",
					err, __func__);
			data = data | RESET_MASK;
			/* Reset should be reviewed for nesesity */
			err = adux1050->write(adux1050->dev, CTRL_REG,
					&data, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev, "I2C Write failed %d at %s\n",
					err, __func__);
		}
		sysfs_remove_group(&adux1050->dev->kobj,
				   &adux1050_attr_group);
	#ifdef CONFIG_ADUX1050_POLL
		kthread_stop(adux1050->polling_task);
	#else
		free_irq(adux1050->irq, adux1050);
	#endif
		cancel_work_sync(&adux1050->work);
		cancel_work_sync(&adux1050->calib_work);
		cancel_delayed_work(&adux1050->proxy_work);
		adux1050->proxy_count = 0;
		ADUX1050_LOG("%s -proxywork cancelled as i2c removed\n",
				    __func__);
		input_unregister_device(adux1050->input);
		pr_debug("device unregister completed & adux1050 device free called\n");
		kfree(adux1050->stg_info); /* BJ */
		pr_debug("free of stginfo complete & adux1050 free called\n");
		kfree(adux1050);
		i2c_set_clientdata(client, NULL);
	}
	pr_debug("%s, END\n", __func__);
	return 0;
}

/*
 * Used similar to driver remove function but used as a shutdown call back
 * This is used to remove the device or the I2C client from the system
 * @param client The client ID to be removed
 * @return void
 */
void adux1050_shutdown(struct i2c_client *client)
{
	struct adux1050_chip *adux1050 = i2c_get_clientdata(client);
	u16 data = DISABLE_DEV_INT;
	s16 err = 0;

	pr_debug("%s, Start\n", __func__);
	if (adux1050 != NULL) {
		adux1050->proxy_cancel = _TRUE;
		if (adux1050->dev_enable == ADUX1050_ENABLE) {
			#ifdef CONFIG_ADUX1050_POLL
			#else
				disable_irq(adux1050->irq);
				disable_irq_wake(adux1050->irq);
			#endif
			err = adux1050->write(adux1050->dev, INT_CTRL_REG,
					&data, DEF_WR);
			if (err < DEF_WR)
				dev_err(adux1050->dev,
					"I2C Write failed %d at %s\n",
					err, __func__);
		}
		sysfs_remove_group(&adux1050->dev->kobj,
				   &adux1050_attr_group);
	#ifdef CONFIG_ADUX1050_POLL
		kthread_stop(adux1050->polling_task);
	#else
		free_irq(adux1050->irq, adux1050);
	#endif
		cancel_work_sync(&adux1050->work);
		cancel_work_sync(&adux1050->calib_work);
		cancel_delayed_work(&adux1050->proxy_work);
		adux1050->proxy_count = 0;
		input_unregister_device(adux1050->input);
		kfree(adux1050->stg_info);
		kfree(adux1050);
		i2c_set_clientdata(client, NULL);
	}
	pr_debug("%s, END\n", __func__);
}

#ifdef	CONFIG_ADUX1050_PM
/*
 * Device suspend PM operation call back function
 * @param dev Device structure for handling the power management.
 * @return Zero on success
 */
static int adux1050_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adux1050_chip *adux1050 = i2c_get_clientdata(client);
	s16 err = 0;

	dev_dbg(adux1050->dev, "%s,check (%d)\n",
		 __func__, adux1050->dev_enable);
	/* To put device in STANDBY Mode */
	if (adux1050->dev_enable == ADUX1050_ENABLE) {
		err = adux1050_disable(adux1050);
		if (err < ZERO_VAL)
			dev_err(adux1050->dev, "I2C RD/WR failed %d at %s\n",
				err, __func__);
		adux1050->proxy_cancel = _TRUE;
		cancel_delayed_work(&adux1050->proxy_work);

		adux1050->dev_enable = ADUX1050_SUSPEND;
	}
	return 0;
}

/*
 * Device resume PM operation call back function
 * @param dev Device structure for handling the power management.
 * @return Zero on success
 */
static int adux1050_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adux1050_chip *adux1050 = i2c_get_clientdata(client);
	s16 err = 0;

	dev_dbg(adux1050->dev, "%s,check (%d)\n",
		 __func__, adux1050->dev_enable);

	if (adux1050->dev_enable == ADUX1050_SUSPEND) {
		adux1050->dev_enable = ADUX1050_ENABLE;
		err = adux1050_enable(adux1050);
		if (err < ZERO_VAL)
			dev_err(adux1050->dev, "I2C RD/WR failed %d at %s\n",
				err, __func__);
		if (adux1050->proxy_enable) {
			adux1050->proxy_cancel = _FALSE;
			schedule_delayed_work(&adux1050->proxy_work,
					      msecs_to_jiffies
				(PROXY_TIME * adux1050->allowed_proxy_time));
		}
#ifdef CONFIG_ADUX1050_POLL
		wake_up_process(adux1050->polling_task);
#endif
	}

	return 0;
}
#endif

/*
 * Device ID table for the ADUX1050 driver
 */
static const struct of_device_id adux1050_of_id[] = {
	{.compatible = "adi,adux1050"},
	{}
};

/*
 * Device ID table for the ADUX1050 driver
 */
static const struct i2c_device_id adux1050_id[] = {
	{ "adux1050", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, adux1050_id);

#ifdef	CONFIG_ADUX1050_PM
/*
 * The file Operation table for power
 */
static const struct dev_pm_ops adux1050_dev_pm_ops = {
	.suspend = adux1050_i2c_suspend,
	.resume = adux1050_i2c_resume,
};
#endif

/*
 * I2C driver structure containing the function callbacks,
 * driver name and ID tables
 */
struct i2c_driver adux1050_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
#ifdef	CONFIG_ADUX1050_PM
		.pm = &adux1050_dev_pm_ops,
#endif
#ifdef	CONFIG_OF
		.of_match_table = adux1050_of_id,
#endif
	},
	.probe    = adux1050_probe,
	.shutdown = adux1050_shutdown,
	.remove   = adux1050_i2c_remove,
	.id_table = adux1050_id,
};

/*
 * This is an init function called during module insertion.
 * calls in turn i2c driver probe function
 */
static int adux1050_module_init(void)
{
	pr_debug("%s,START\n", __func__);
	return i2c_add_driver(&adux1050_i2c_driver);
}
module_init(adux1050_module_init);

/*
 * This is an exit function called during module removal --
 * calls in turn i2c driver delete function
 */
static void adux1050_module_exit(void)
{
	i2c_del_driver(&adux1050_i2c_driver);
}

module_exit(adux1050_module_exit);
MODULE_DESCRIPTION("Analog Devices ADUX1050 Driver");
MODULE_AUTHOR("Analog Devices");
MODULE_LICENSE("GPL");
