/* ami_self_test.c - AMI-Sensor Self-Test
 *
 * Copyright (C) 2012 AICHI STEEL CORPORATION
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
 * @file	ami_self_test.c
 * @brief	AMI306 Self Test
 */
#ifdef __KERNEL__
#include <linux/module.h>
#include "linux/ami306_hw.h"
#include "linux/ami_sensor_pif.h"
#include "linux/ami306_cmd.h"
#else
#include <stdlib.h>
#include "ami306_hw.h"
#include "ami_sensor_pif.h"
#include "ami306_cmd.h"
#endif

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		define
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/* register */
#define AMI_REG_ANCTRL		0x40
#define AMI_REG_ADSEL		0x46
#define AMI_REG_ADCTRL		0x4a
#define AMI_REG_MEA_CNT		0x4c
#define AMI_REG_ADFIN		0x50
#define AMI_REG_DOUT		0x52
#define AMI_REG_DELAYX		0x68
#define AMI_REG_DELAYY		0x6e
#define AMI_REG_DELAYZ		0x74
/* ami306 */
#define AMI_WIA			0x46
/* threshold */
#define AMI_TEMP_MIN		1500
#define AMI_TEMP_MAX		2500
#define AMI_MI_ELEMENT1_TH	100
#define AMI_MI_ELEMENT2_TH	20
#define AMI_DATA_MIN		-500
#define AMI_DATA_MAX		500
#define AMI_FO_RATE_MIN		900	/* 0.9 */
#define AMI_FO_RATE_MAX		1100	/* 1.1 */
/* macro */
#define AMI_MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define AMI_MAX(a, b)		(((a) > (b)) ? (a) : (b))

/* function prototype */
extern int AMI_ForceModeActive(void *i2c_hnd);
extern int AMI_Mea(void *i2c_hnd, s16 val[3]);
extern int AMI_DRDY_Enable(void *i2c_hnd);
extern int AMI_HighSpeedMea(void *i2c_hnd);
extern int AMI_FineOutput(void *i2c_hnd, u16 dat[3]);
extern int AMI_SearchOffsetProc(void *i2c_hnd, u8 fine[3]);
extern int AMI_SRSTset(void *i2c_hnd);
extern int AMI_Write_Offset(void *i2c_hnd, u8 fine[3]);

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	OTP Check
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int checkOTP(void *i2c_hnd)
{
	u8 wk_buf;
	int ret = AMI_i2c_recv_b(i2c_hnd, AMI_REG_WIA, &wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	AMI_LOG("WIA: %#x", wk_buf);

	if (wk_buf != AMI_WIA)
		return AMI_ST_ERR_OTP;

	return AMI_ST_OK;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	DRDY Check
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#ifdef USE_DRDY_PIN
static int checkDRDY(void *i2c_hnd)
{
	u8 wk_buf;
	/*---------------------------------------------*/
	/* Force mode active */
	int ret = AMI_ForceModeActive(i2c_hnd);
	if (ret != 0)
		return ret;

	/*---------------------------------------------*/
	/* DRDY Enable Active-Low */
	wk_buf = 0x08;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_CTRL2, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_DRDY_Value();
	AMI_LOG("Active-Low:%d", ret);
	if (ret == 0)
		return AMI_ST_ERR_COMMUNICATION;

	/* DRDY Enable Active-High */
	wk_buf = 0x0C;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_CTRL2, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_DRDY_Value();
	AMI_LOG("Active-High:%d", ret);
	if (ret == 1)
		return AMI_ST_ERR_COMMUNICATION;

	return AMI_ST_OK;
}
#endif

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Temp sensor Check
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int checkTempSensor(void *i2c_hnd)
{
	s16 val[3], dat;

	int ret = AMI_ForceModeActive(i2c_hnd);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_DRDY_Enable(i2c_hnd);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_HighSpeedMea(i2c_hnd);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_Mea(i2c_hnd, val);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_TEMP, &dat);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	AMI_LOG("TEMP:%d  DATA:%d,%d,%d", dat, val[0], val[1], val[2]);

	if ((dat < AMI_TEMP_MIN) || (AMI_TEMP_MAX < dat))
		return AMI_ST_ERR_TEMP_SENSOR;

	return AMI_ST_OK;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	MI element Check
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int checkMIElement1(void *i2c_hnd)
{
	int ret = 0, ch_i = 0, delay_i = 0, retry_cu = 0;
	u8 reg_addr;
	u8 wk_buf;
	s16 val, val_max, val_min;

	for (ch_i = 0; ch_i < 3; ch_i++) {

		ret = AMI_SRSTset(i2c_hnd);
		if (ret != 0)
			return AMI_ST_ERR_COMMUNICATION;

		ret = AMI_ForceModeActive(i2c_hnd);
		if (ret != 0)
			return ret;

		wk_buf = 1 << (ch_i + 1);
		ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_ADSEL, wk_buf);
		if (ret != 0)
			return AMI_ST_ERR_COMMUNICATION;

		val_min = 9999;
		val_max = -9999;

		for (delay_i = 0; delay_i <= 100; delay_i += 10) {
			switch (ch_i) {
			case 0:
				reg_addr = AMI_REG_DELAYX;
				break;
			case 1:
				reg_addr = AMI_REG_DELAYY;
				break;
			case 2:
				reg_addr = AMI_REG_DELAYZ;
				break;
			}
			wk_buf = delay_i;
			ret = AMI_i2c_send_b(i2c_hnd, reg_addr, wk_buf);
			if (ret != 0)
				return AMI_ST_ERR_COMMUNICATION;

			wk_buf = 0;
			ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_MEA_CNT, wk_buf);
			if (ret != 0)
				return AMI_ST_ERR_COMMUNICATION;

			wk_buf = 0x01;
			ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_ADCTRL, wk_buf);
			if (ret != 0)
				return AMI_ST_ERR_COMMUNICATION;

			retry_cu = 0;
			while (1) {
				ret = AMI_i2c_recv_b(i2c_hnd, AMI_REG_ADFIN,
						     &wk_buf);
				if (ret != 0)
					return AMI_ST_ERR_COMMUNICATION;

				if ((wk_buf & 0x01) == 0x01)
					break;

				retry_cu++;
				if (retry_cu > 5)
					return AMI_ST_ERR_TEMP_SENSOR;
			}
			ret = AMI_i2c_recv_w(i2c_hnd, AMI_REG_DOUT, &val);
			if (ret != 0)
				return AMI_ST_ERR_COMMUNICATION;

			if ((delay_i == 0) && (val == 0))
				return AMI_ST_ERR_MI_ELEMENT;

			val_min = AMI_MIN(val_min, val);
			val_max = AMI_MAX(val_max, val);

			AMI_LOG("%c,%d,%d", 'x' + ch_i, delay_i, val);
		}
		AMI_LOG("[%c] %d = max(%d) - min(%d)",
			'x' + ch_i, val_max - val_min, val_max, val_min);
		if ((val_max - val_min) <= AMI_MI_ELEMENT1_TH)
			return AMI_ST_ERR_MI_ELEMENT;
	}
	return AMI_ST_OK;
}

static int checkMIElement2(void *i2c_hnd)
{
	int ret = 0, i = 0, ch_i = 0;
	s16 val[3];
	s16 val_min[3] = { 9999, 9999, 9999 };
	s16 val_max[3] = { -9999, -9999, -9999 };
	u8 wk_buf;

	ret = AMI_SRSTset(i2c_hnd);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_ForceModeActive(i2c_hnd);
	if (ret != 0)
		return ret;

	wk_buf = 0x0C;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_CTRL2, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_CTRL4, AMI_CTRL4_HS);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	wk_buf = 255;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_DELAYX, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_DELAYY, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_DELAYZ, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_ANCTRL, 0x0003);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	for (i = 0; i < 10; i++) {

		ret = AMI_Mea(i2c_hnd, val);
		if (ret != 0)
			return AMI_ST_ERR_COMMUNICATION;

		AMI_LOG("%d,%d,%d,%d", i, val[0], val[1], val[2]);

		for (ch_i = 0; ch_i < 3; ch_i++) {
			val_min[ch_i] = AMI_MIN(val_min[ch_i], val[ch_i]);
			val_max[ch_i] = AMI_MAX(val_max[ch_i], val[ch_i]);
		}
	}

	for (ch_i = 0; ch_i < 3; ch_i++) {
		AMI_LOG("[%c] %d = max(%d) - min(%d)", 'x' + ch_i,
			val_max[ch_i] - val_min[ch_i], val_max[ch_i],
			val_min[ch_i]);
		if ((val_max[ch_i] - val_min[ch_i]) >= AMI_MI_ELEMENT2_TH)
			return AMI_ST_ERR_MI_ELEMENT;
	}
	return AMI_ST_OK;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Digital Circuit Error
 *-------+---------+---------+---------+---------+---------+---------+--------*/
static int checkDigitalCircuit(void *i2c_hnd)
{
	u16 fine_output[3];
	u8 offset[3], wk_offset[3];
	u8 wk_buf;
	int ret = 0, ch_i = 0;
	s16 val[3], val1[3];

	ret = AMI_FineOutput(i2c_hnd, fine_output);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_SRSTset(i2c_hnd);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_ForceModeActive(i2c_hnd);
	if (ret != 0)
		return ret;

	wk_buf = 0x0C;
	ret = AMI_i2c_send_b(i2c_hnd, AMI_REG_CTRL2, wk_buf);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_i2c_send_w(i2c_hnd, AMI_REG_CTRL4, AMI_CTRL4_HS);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_SearchOffsetProc(i2c_hnd, offset);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_Mea(i2c_hnd, val);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	for (ch_i = 0; ch_i < 3; ch_i++) {
		if ((val[ch_i] < AMI_DATA_MIN) || (AMI_DATA_MAX < val[ch_i])) {
			AMI_LOG("[%c] %d", 'x' + ch_i, val[ch_i]);
			return AMI_ST_ERR_DIGITAL_CIRCUIT;
		}
	}

	wk_offset[0] = offset[0] + 2;
	wk_offset[1] = offset[1] + 2;
	wk_offset[2] = offset[2] + 2;

	ret = AMI_Write_Offset(i2c_hnd, wk_offset);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	ret = AMI_Mea(i2c_hnd, val1);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	for (ch_i = 0; ch_i < 3; ch_i++) {

		s32 val_diff = abs(val1[ch_i] - val[ch_i]) / 2 * 1000;

		AMI_LOG("[%c] %d:%d  %d:%d  diff:%d  fine_output:%d",
			'x' + ch_i, offset[ch_i], val[ch_i],
			wk_offset[ch_i], val1[ch_i],
			val_diff / 1000, fine_output[ch_i]);

		if ((val_diff < (s32) fine_output[ch_i] * AMI_FO_RATE_MIN) ||
		    ((s32) fine_output[ch_i] * AMI_FO_RATE_MAX < val_diff))
			return AMI_ST_ERR_DIGITAL_CIRCUIT;
	}

	ret = AMI_Write_Offset(i2c_hnd, offset);
	if (ret != 0)
		return AMI_ST_ERR_COMMUNICATION;

	return AMI_ST_OK;
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Self Test
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/**
 * Self Test
 *
 * @param i2c_hnd	i2c handle
 * @return result
 */
int AMI_SelfTest(void *i2c_hnd)
{
	int ret = 0;

	AMI_LOG("----- OTP Check -----");
	ret = checkOTP(i2c_hnd);
	if (ret != 0)
		return ret;

#ifdef USE_DRDY_PIN
	AMI_LOG("----- DRDY Check -----");
	ret = checkDRDY(i2c_hnd);
	if (ret != 0)
		return ret;
#endif

	AMI_LOG("-----TEMP Check -----");
	ret = checkTempSensor(i2c_hnd);
	if (ret != 0)
		return ret;

	AMI_LOG("----- MI Element Check 1 -----");
	ret = checkMIElement1(i2c_hnd);
	if (ret != 0)
		return ret;

	AMI_LOG("----- MI Element Check 2 -----");
	ret = checkMIElement2(i2c_hnd);
	if (ret != 0)
		return ret;

	AMI_LOG("----- Digital Circuit Check -----");
	ret = checkDigitalCircuit(i2c_hnd);
	if (ret != 0)
		return ret;

	return AMI_ST_OK;
}
