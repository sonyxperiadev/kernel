/*
 * Copyright (c) 2010-2011 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

/*
 * File     yas_pcb_test.c
 * Brief    pcb test program for yas530/yas532
 * Date     2013/1/22
 * Revision 1.4.3
 */
#include "yas_pcb_test.h"

/* define */
/* reg num */
#define YAS530_CAL_REG_NUM		(16)
#define YAS532_CAL_REG_NUM		(14)
#define YAS_PCB_MEASURE_DATA_REG_NUM	(8)

/* default value */
#define YAS_PCB_TEST1_DEFAULT		(0x00)
#define YAS_PCB_TEST2_DEFAULT		(0x00)
#define YAS_PCB_INTERVAL_DEFAULT	(0x00)
#define YAS_PCB_CONFIG_DEFAULT		(0x01)		/* INTON = 1 */
#define YAS_PCB_COIL_DEFAULT		(0x00)

/* measure command */
#define YAS_PCB_MEASURE_COMMAND_START	(0x01)
#define YAS_PCB_MEASURE_COMMAND_LDTC	(0x02)
#define YAS_PCB_MEASURE_COMMAND_FORS	(0x04)

#define YAS_PCB_MEASURE_BUSY		(0x80)

#define YAS_PCB_MEASURE_WAIT_TIME	(2)		/* ms */
#define YAS_PCB_HARD_OFFSET_CORRECT	(16)
#define YAS_PCB_COIL_INIT_CALC_NUM	(5)

#define YAS_PCB_HARD_OFFSET_MASK	(0x3F)

#define YAS_PCB_INT_CHECK		(1)
#define YAS_PCB_INT_NOTCHECK		(0)
#define YAS_PCB_INT_HIGH		(1)
#define YAS_PCB_INT_LOW			(0)

#define YAS_PCB_ACC_Z			(9806550L)	/* m/s2 */

#define YAS530_DEVICE_ID		(0x01)		/* MS-3E */
#define YAS532_DEVICE_ID		(0x02)		/* MS-3R */

#define YAS530_VERSION_A		(0)		/* MS-3E Aver */
#define YAS530_VERSION_B		(1)		/* MS-3E Bver */
/*#define YAS530_VERSION_AB		(0)*/		/* MS-3R ABver */
#define YAS532_VERSION_AC		(1)		/* MS-3R ACver */

#define YAS530_COEF_VERSION_A		(380)
#define YAS530_COEF_VERSION_B		(550)
/*#define YAS532_COEF_VERSION_AB	(1800)*/
/*#define YAS532_COEF_VERSION_AC	(900)*/
#define YAS532_COEFX_VERSION_AC		(850)
#define YAS532_COEFY1_VERSION_AC	(750)
#define YAS532_COEFY2_VERSION_AC	(750)

#define YAS530_RAWDATA_CENTER		(2048)
#define YAS530_RAWDATA_OVERFLOW		(4095)
#define YAS532_RAWDATA_CENTER		(4096)
#define YAS532_RAWDATA_OVERFLOW		(8190)

#define YAS_PCB_DIR_DIVIDER		(400)

#define YAS_PCB_TEST1			(0)
#define YAS_PCB_TEST3			(1)
#define YAS_PCB_TEST4			(2)
#define YAS_PCB_TEST5			(3)
#define YAS_PCB_TEST6			(4)
#define YAS_PCB_TEST7			(5)
#define YAS_PCB_TEST8			(6)
#define YAS_PCB_TEST2			(7)
#define YAS_PCB_TEST_NUM		(8)

/* typedef */
struct yas_pcb_vector {
	int32_t v[3];
};

struct yas_pcb_correction {
	int32_t s32Cx, s32Cy1, s32Cy2;
	int32_t s32A2, s32A3, s32A4, s32A5, s32A6, s32A7, s32A8, s32A9, s32K;
	int32_t s32ZFlag;
	int32_t s32Rx, s32Ry1, s32Ry2;
	int32_t s32Fx, s32Fy1, s32Fy2;
	int32_t s32Ver;
};

struct yas_pcb_sensitivity {
	int32_t s32Sx, s32Sy, s32Sz;
};

/* values */
static uint16_t gu16State;
static struct yas_pcb_test_callback g_callback;
static struct yas_pcb_vector gstXy1y2;
static int8_t gs08HardOffset[3];
static struct yas_pcb_correction gstCorrect;
static uint8_t gu08DevId;
static int32_t gs32Center;
static int32_t gs32Overflow;
#ifdef YAS_PCBTEST_EXTRA
static uint8_t gu08Recalc;
static int32_t gs32RecalcWait;
#endif

/* functions */
static int Ms3AxesLibAtan8(int, int, short *);
static int Ms3AxesLibDir8(int, int, unsigned short *);
static int yas_pcb_check_state(int);
static void yas_pcb_update_state(int);
static int yas_pcb_power_on(void);
static int yas_pcb_power_off(void);
static int yas_pcb_reset_coil(void);
static int yas530_read_cal(uint8_t *);
static int yas532_read_cal(uint8_t *);
static void yas530_calc_correction(const uint8_t *);
static void yas532_calc_correction(const uint8_t *);
static int yas_pcb_set_offset(const int8_t *);
static int yas_pcb_measure(struct yas_pcb_vector *, int *, uint8_t, uint8_t);
static int yas_pcb_is_flow_occued(struct yas_pcb_vector *, int32_t, int32_t);
static void yas_pcb_calc_sensitivity(struct yas_pcb_vector *,
				 int, struct yas_pcb_sensitivity *);
static void yas_pcb_calc_position(struct yas_pcb_vector *,
		struct yas_pcb_vector *, int);
static int yas_pcb_calc_magnetic_field(struct yas_pcb_vector *,
		struct yas_pcb_vector *);
static int yas_pcb_test1(int *);
static int yas_pcb_test2(void);
static int yas_pcb_test3(void);
static int yas_pcb_test4(int *, int *, int *);
static int yas_pcb_test5(int *);
static int yas_pcb_test6(int *, int *);
#ifdef YAS_PCBTEST_EXTRA
static int yas_pcb_test7(int *, int *, int *);
#endif

static int Ms3AxesLibAtan8(int ss, int cc, short *ans)
{
	static const unsigned char AtanTable[] = {
		0,	 1,   3,   5,	6,	 8,  11,  13,
		15,  18,  21,  24,	27,  31,  34,  39,
		43,  48,  53,  58,	63,  69,  75,  82,
		89,  96, 103, 110, 118, 126, 134, 143,
		152
	};

	unsigned char idx;
	unsigned short idx_mul64;
	signed char sign = 1;
	unsigned int ucc;
	unsigned int uss;
	unsigned short ans_mul8;
	unsigned char idx_mod64;
	unsigned short ans_diff8;

	if (cc < 0) {
		sign = -sign;
		ucc = -cc;
	} else {
		ucc = cc;
	}

	if (ss < 0) {
		sign = -sign;
		uss = -ss;
	} else {
		uss = ss;
	}

	while (ucc >= 0x400) {
		ucc >>= 1;
		uss >>= 1;
	}

	if (ucc == 0)
		return -1;

	idx_mul64 = (uss << 11) / ucc;

	idx = idx_mul64 >> 6;
	ans_mul8 = (idx << 4) - AtanTable[idx];

	idx_mod64 = (unsigned char)idx_mul64 & 0x3f;

	if (idx < 32) {
		idx++;
		ans_diff8 = (idx << 4) - AtanTable[idx] - ans_mul8;
		ans_mul8 += (ans_diff8 * idx_mod64) >> 6;
	}

	*ans = (sign == 1) ? ans_mul8 : (-ans_mul8);

	return 0;
}

static int Ms3AxesLibDir8(int ss, int cc, unsigned short *ans)
{
	short temp_ans = 0;
	int ucc = cc;
	int uss = ss;
	int ret = 0;

	*ans = 0;

	if (cc < -2147483647)
		cc++;
	if (ss < -2147483647)
		ss++;

	if (cc < 0)
		ucc = -cc;

	if (ss < 0)
		uss = -ss;

	if (uss <= ucc) {
		ret = Ms3AxesLibAtan8(ss, cc, &temp_ans);
		if (ret < 0)
			return 1;

		if (cc > 0) {
			if (temp_ans < 0)
				temp_ans += 2880;
		} else
			temp_ans += 1440;
	} else {
		ret = Ms3AxesLibAtan8(cc, ss, &temp_ans);
		if (ret < 0)
			return 1;

		if (ss > 0)
			temp_ans = 720 - temp_ans;
		else
			temp_ans = 2160 - temp_ans;
	}

	*ans = temp_ans;

	return 0;
}

static int
yas_pcb_check_state(int id)
{
	int result = YAS_PCB_ERROR_TEST_ORDER;
	uint16_t u16Mask;
	const uint16_t u16TestTable[] = {
		0x0000, /* 1 */
		0x0000, /* 3 */
		(1 << YAS_PCB_TEST1) | (1 << YAS_PCB_TEST3), /* 4 */
		(1 << YAS_PCB_TEST1) | (1 << YAS_PCB_TEST3)
			| (1 << YAS_PCB_TEST4), /* 5 */
		(1 << YAS_PCB_TEST1) | (1 << YAS_PCB_TEST3)
			| (1 << YAS_PCB_TEST4), /* 6 */
		(1 << YAS_PCB_TEST1) | (1 << YAS_PCB_TEST3)
			| (1 << YAS_PCB_TEST4)
			| (1 << YAS_PCB_TEST5), /* 7 */
		(1 << YAS_PCB_TEST1) | (1 << YAS_PCB_TEST3)
			| (1 << YAS_PCB_TEST4), /* 8 */
		(1 << YAS_PCB_TEST1), /* 2 */
	};

	if ((YAS_PCB_TEST1 <= id) && (id < YAS_PCB_TEST_NUM)) {
		u16Mask = u16TestTable[id];
		if (u16Mask == 0) {
			switch (id) {
			case YAS_PCB_TEST1:
				if ((gu16State == 0)
				 || (gu16State == (1 << YAS_PCB_TEST1)))
					result = YAS_PCB_NO_ERROR;
				break;

			case YAS_PCB_TEST3:
				if ((gu16State == (1 << YAS_PCB_TEST1))
				 || (gu16State ==
				     ((1 << YAS_PCB_TEST1)
				      | (1 << YAS_PCB_TEST3))))
					result = YAS_PCB_NO_ERROR;
				break;

			default:
				break;
			}
		} else {
			if ((gu16State & u16Mask) == u16Mask)
				result = YAS_PCB_NO_ERROR;
		}
	}

	return result;
}

static void
yas_pcb_update_state(int id)
{
	if ((YAS_PCB_TEST1 <= id) && (id < YAS_PCB_TEST2))
		gu16State |= (uint16_t)(1 << id);
	else
		gu16State = 0;
}

static int
yas_pcb_power_on(void)
{
	int result = YAS_PCB_NO_ERROR;
	int ret;

	if (NULL != g_callback.power_on) {
		ret = g_callback.power_on();
		if (0 != ret)
			result = YAS_PCB_ERROR_POWER;
	}

	return result;
}

static int
yas_pcb_power_off(void)
{
	int result = YAS_PCB_NO_ERROR;
	int ret;

	if (NULL != g_callback.power_off) {
		ret = g_callback.power_off();
		if (0 != ret)
			result = YAS_PCB_ERROR_POWER;
	}

	return result;
}

static int
yas_pcb_reset_coil(void)
{
	int ret;
	uint8_t u08Data;

	u08Data = YAS_PCB_COIL_DEFAULT;
	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_COIL,
			&u08Data, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	return YAS_PCB_NO_ERROR;
}

static int
yas530_read_cal(uint8_t *pu08Buf)
{
	int i;
	int ret;
	int size = YAS530_CAL_REG_NUM;

	/* Dummy read */
	ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_CAL,
			pu08Buf, size);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_CAL,
			pu08Buf, size);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	/* cal register is all 0 */
	for (i = 0; i < size; i++) {
		if (pu08Buf[i] != 0x00)
			return YAS_PCB_NO_ERROR;
	}

	return YAS_PCB_ERROR_CALREG;
}

static int
yas532_read_cal(uint8_t *pu08Buf)
{
	int i;
	int ret;
	int size = YAS532_CAL_REG_NUM;
	int len = size - 1;

	/* Dummy read */
	ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_CAL,
			pu08Buf, size);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_CAL,
			pu08Buf, size);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	/* cal register is all 0 */
	for (i = 0; i < len; i++) {
		if (pu08Buf[i] != 0x00)
			return YAS_PCB_NO_ERROR;
	}

	/* MSB is not 0 */
	if (pu08Buf[len] & 0x80)
		return YAS_PCB_NO_ERROR;

	return YAS_PCB_ERROR_CALREG;
}

static void
yas530_calc_correction(const uint8_t *pu08Data)
{
	uint8_t u08Dx  = pu08Data[0];
	uint8_t u08Dy1 = pu08Data[1];
	uint8_t u08Dy2 = pu08Data[2];
	uint8_t u08D2  = (uint8_t)((pu08Data[3] >> 2) & 0x3F);
	uint8_t u08D3  = (uint8_t)(((pu08Data[3] << 2) & 0x0C)
				| ((pu08Data[4] >> 6) & 0x03));
	uint8_t u08D4  = (uint8_t)(pu08Data[4] & 0x3F);
	uint8_t u08D5  = (uint8_t)((pu08Data[5] >> 2) & 0x3f);
	uint8_t u08D6  = (uint8_t)(((pu08Data[5] << 4) & 0x30)
				| ((pu08Data[6] >> 4) & 0x0F));
	uint8_t u08D7  = (uint8_t)(((pu08Data[6] << 3) & 0x78)
				| ((pu08Data[7] >> 5) & 0x07));
	uint8_t u08D8  = (uint8_t)(((pu08Data[7] << 1) & 0x3E)
				| ((pu08Data[8] >> 7) & 0x01));
	uint8_t u08D9  = (uint8_t)(((pu08Data[8] << 1) & 0xFE)
				| ((pu08Data[9] >> 7) & 0x01));
	uint8_t u08D0  = (uint8_t)((pu08Data[9] >> 2) & 0x1F);
	uint8_t u08ZFlag = (uint8_t)((pu08Data[11] >> 5) & 0x01);
	uint8_t u08Rx	 = (uint8_t)(((pu08Data[11] << 1) & 0x3E)
				| ((pu08Data[12] >> 7) & 0x01));
	uint8_t u08Fx	 = (uint8_t)((pu08Data[12] >> 5) & 0x03);
	uint8_t u08Ry1	 = (uint8_t)(((pu08Data[12] << 1) & 0x3E)
				| ((pu08Data[13] >> 7) & 0x01));
	uint8_t u08Fy1	 = (uint8_t)((pu08Data[13] >> 5) & 0x03);
	uint8_t u08Ry2	 = (uint8_t)(((pu08Data[13] << 1) & 0x3E)
				| ((pu08Data[14] >> 7) & 0x01));
	uint8_t u08Fy2	 = (uint8_t)((pu08Data[14] >> 5) & 0x03);
	uint8_t u08Ver	 = pu08Data[15] & 0x07;

	gstCorrect.s32Cx  = (int32_t)((u08Dx * 6) - 768);
	gstCorrect.s32Cy1 = (int32_t)((u08Dy1 * 6) - 768);
	gstCorrect.s32Cy2 = (int32_t)((u08Dy2 * 6) - 768);
	gstCorrect.s32A2  = (int32_t)(u08D2 - 32);
	gstCorrect.s32A3  = (int32_t)(u08D3 - 8);
	gstCorrect.s32A4  = (int32_t)(u08D4 - 32);
	gstCorrect.s32A5  = (int32_t)(u08D5 + 38);
	gstCorrect.s32A6  = (int32_t)(u08D6 - 32);
	gstCorrect.s32A7  = (int32_t)(u08D7 - 64);
	gstCorrect.s32A8  = (int32_t)(u08D8 - 32);
	gstCorrect.s32A9  = (int32_t)u08D9;
	gstCorrect.s32K   = (int32_t)(u08D0) + 10;
	gstCorrect.s32ZFlag = (int32_t)u08ZFlag;
	gstCorrect.s32Rx  = (int32_t)((int8_t)(u08Rx << 2) >> 2);
	gstCorrect.s32Fx  = (int32_t)u08Fx;
	gstCorrect.s32Ry1 = (int32_t)((int8_t)(u08Ry1 << 2) >> 2);
	gstCorrect.s32Fy1 = (int32_t)u08Fy1;
	gstCorrect.s32Ry2 = (int32_t)((int8_t)(u08Ry2 << 2) >> 2);
	gstCorrect.s32Fy2 = (int32_t)u08Fy2;
	gstCorrect.s32Ver = (int32_t)u08Ver;
}

static void
yas532_calc_correction(const uint8_t *pu08Data)
{
	uint8_t u08Dx  = pu08Data[0];
	uint8_t u08Dy1 = pu08Data[1];
	uint8_t u08Dy2 = pu08Data[2];
	uint8_t u08D2  = (uint8_t)((pu08Data[3] >> 2) & 0x3F);
	uint8_t u08D3  = (uint8_t)(((pu08Data[3] << 2) & 0x0C)
				| ((pu08Data[4] >> 6) & 0x03));
	uint8_t u08D4  = (uint8_t)(pu08Data[4] & 0x3F);
	uint8_t u08D5  = (uint8_t)((pu08Data[5] >> 2) & 0x3f);
	uint8_t u08D6  = (uint8_t)(((pu08Data[5] << 4) & 0x30)
				| ((pu08Data[6] >> 4) & 0x0F));
	uint8_t u08D7  = (uint8_t)(((pu08Data[6] << 3) & 0x78)
				| ((pu08Data[7] >> 5) & 0x07));
	uint8_t u08D8  = (uint8_t)(((pu08Data[7] << 1) & 0x3E)
				| ((pu08Data[8] >> 7) & 0x01));
	uint8_t u08D9  = (uint8_t)(((pu08Data[8] << 1) & 0xFE)
				| ((pu08Data[9] >> 7) & 0x01));
	uint8_t u08D0  = (uint8_t)((pu08Data[9] >> 2) & 0x1F);
	uint8_t u08Rx  = (uint8_t)((pu08Data[10] >> 1) & 0x3F);
	uint8_t u08Fx  = (uint8_t)(((pu08Data[10] & 0x01) << 1)
				| ((pu08Data[11] >> 7) & 0x01));
	uint8_t u08Ry1 = (uint8_t)((pu08Data[11] >> 1) & 0x3F);
	uint8_t u08Fy1 = (uint8_t)(((pu08Data[11] & 0x01) << 1)
				| ((pu08Data[12] >> 7) & 0x01));
	uint8_t u08Ry2 = (uint8_t)((pu08Data[12] >> 1) & 0x3F);
	uint8_t u08Fy2 = (uint8_t)(((pu08Data[12] & 0x01) << 1)
				| ((pu08Data[13] >> 7) & 0x01));
	uint8_t u08Ver = pu08Data[13] & 0x01;

	gstCorrect.s32Cx  = (int32_t)((u08Dx * 10) - 1280);
	gstCorrect.s32Cy1 = (int32_t)((u08Dy1 * 10) - 1280);
	gstCorrect.s32Cy2 = (int32_t)((u08Dy2 * 10) - 1280);
	gstCorrect.s32A2  = (int32_t)(u08D2 - 32);
	gstCorrect.s32A3  = (int32_t)(u08D3 - 8);
	gstCorrect.s32A4  = (int32_t)(u08D4 - 32);
	gstCorrect.s32A5  = (int32_t)(u08D5 + 38);
	gstCorrect.s32A6  = (int32_t)(u08D6 - 32);
	gstCorrect.s32A7  = (int32_t)(u08D7 - 64);
	gstCorrect.s32A8  = (int32_t)(u08D8 - 32);
	gstCorrect.s32A9  = (int32_t)u08D9;
	gstCorrect.s32K   = (int32_t)u08D0;
	gstCorrect.s32ZFlag = (int32_t)1;
	gstCorrect.s32Rx  = (int32_t)((int8_t)(u08Rx << 2) >> 2);
	gstCorrect.s32Fx  = (int32_t)u08Fx;
	gstCorrect.s32Ry1 = (int32_t)((int8_t)(u08Ry1 << 2) >> 2);
	gstCorrect.s32Fy1 = (int32_t)u08Fy1;
	gstCorrect.s32Ry2 = (int32_t)((int8_t)(u08Ry2 << 2) >> 2);
	gstCorrect.s32Fy2 = (int32_t)u08Fy2;
	gstCorrect.s32Ver = (int32_t)u08Ver;
}

static int
yas_pcb_set_offset(const int8_t *ps08Offset)
{
	int result = YAS_PCB_NO_ERROR;
	int ret;
	uint8_t u08Addr;
	uint8_t u08Data;
	uint8_t i;

	for (i = 0; i < 3; i++) {
		u08Addr = (uint8_t)(YAS_PCB_ADDR_OFFSET + i);
		u08Data = (uint8_t)ps08Offset[i] & YAS_PCB_HARD_OFFSET_MASK;
		ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE,
						u08Addr, &u08Data, 1);
		if (0 != ret) {
			result = YAS_PCB_ERROR_I2C;
			break;
		}
	}

	return result;
}

static int
yas_pcb_measure(struct yas_pcb_vector *pstXy1y2, int *temperature,
			uint8_t u08Command, uint8_t u08CheckIni)
{
	int ret;
	uint8_t u08Buf[YAS_PCB_MEASURE_DATA_REG_NUM];
	int low_or_high;

	if ((YAS_PCB_INT_CHECK == u08CheckIni)
	 && (NULL != g_callback.read_intpin)) {
		ret = g_callback.read_intpin(&low_or_high);
		if ((0 != ret) || (YAS_PCB_INT_HIGH != low_or_high))
			return YAS_PCB_ERROR_INTERRUPT;
	}

	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE,
			YAS_PCB_ADDR_MEASURE_COMMAND,
			&u08Command, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	g_callback.msleep(YAS_PCB_MEASURE_WAIT_TIME);

	if ((YAS_PCB_INT_CHECK == u08CheckIni)
	 && (NULL != g_callback.read_intpin)) {
		ret = g_callback.read_intpin(&low_or_high);
		if ((0 != ret) || (YAS_PCB_INT_LOW != low_or_high))
			return YAS_PCB_ERROR_INTERRUPT;
	}

	ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_MEASURE_DATA,
					 u08Buf, YAS_PCB_MEASURE_DATA_REG_NUM);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	/* calc measure data */
	if (YAS532_DEVICE_ID == gu08DevId) {
		*temperature = (((int32_t)(u08Buf[0] & 0x7F) << 3)
					| ((u08Buf[1] >> 5) & 0x07));
		pstXy1y2->v[0] = (int32_t)(((int32_t)(u08Buf[2] & 0x7F) << 6)
					| ((u08Buf[3] >> 2) & 0x3F));
		pstXy1y2->v[1] = (int32_t)(((int32_t)(u08Buf[4] & 0x7F) << 6)
					| ((u08Buf[5] >> 2) & 0x3F));
		pstXy1y2->v[2] = (int32_t)(((int32_t)(u08Buf[6] & 0x7F) << 6)
					| ((u08Buf[7] >> 2) & 0x3F));
	} else {
		*temperature = (((int32_t)(u08Buf[0] & 0x7F) << 2)
					| ((u08Buf[1] >> 6) & 0x03));
		pstXy1y2->v[0] = (int32_t)(((int32_t)(u08Buf[2] & 0x7F) << 5)
					| ((u08Buf[3] >> 3) & 0x1F));
		pstXy1y2->v[1] = (int32_t)(((int32_t)(u08Buf[4] & 0x7F) << 5)
					| ((u08Buf[5] >> 3) & 0x1F));
		pstXy1y2->v[2] = (int32_t)(((int32_t)(u08Buf[6] & 0x7F) << 5)
					| ((u08Buf[7] >> 3) & 0x1F));
	}

	if (YAS_PCB_MEASURE_BUSY == (u08Buf[0] & YAS_PCB_MEASURE_BUSY))
		return YAS_PCB_ERROR_BUSY;

	return YAS_PCB_NO_ERROR;
}

static int
yas_pcb_is_flow_occued(struct yas_pcb_vector *pstXy1y2,
					int32_t underflow, int32_t overflow)
{
	int result = YAS_PCB_NO_ERROR;
	int32_t s32Tmp;
	uint8_t i;

	for (i = 0; i < 3; i++) {
		s32Tmp = pstXy1y2->v[i];
		if (s32Tmp <= underflow)
			result = YAS_PCB_ERROR_UNDERFLOW;
		else
			if (overflow <= s32Tmp)
				result = YAS_PCB_ERROR_OVERFLOW;
	}

	return result;
}

static void
yas_pcb_calc_sensitivity(struct yas_pcb_vector *pstXy1y2, int temperature,
			struct yas_pcb_sensitivity *pstYasSensitivity)
{
	/* calc XYZ data from xy1y2 data */
	int32_t s32Sx  = pstXy1y2->v[0]
			 - ((gstCorrect.s32Cx  * temperature) / 100);
	int32_t s32Sy1 = pstXy1y2->v[1]
			 - ((gstCorrect.s32Cy1 * temperature) / 100);
	int32_t s32Sy2 = pstXy1y2->v[2]
			 - ((gstCorrect.s32Cy2 * temperature) / 100);
	int32_t s32Sy  =  s32Sy1 - s32Sy2;
	int32_t s32Sz  = -s32Sy1 - s32Sy2;

	pstYasSensitivity->s32Sx = s32Sx;
	pstYasSensitivity->s32Sy = s32Sy;
	pstYasSensitivity->s32Sz = s32Sz;
}

static void
yas_pcb_calc_position(struct yas_pcb_vector *pstXy1y2,
		struct yas_pcb_vector *pstXyz, int temperature)
{
	struct yas_pcb_sensitivity stSensitivity;
	struct yas_pcb_sensitivity *pst;

	yas_pcb_calc_sensitivity(pstXy1y2, temperature, &stSensitivity);

	pst = &stSensitivity;
	pstXyz->v[0] = (gstCorrect.s32K
			 * ((100 * pst->s32Sx) + (gstCorrect.s32A2 * pst->s32Sy)
			 + (gstCorrect.s32A3 * pst->s32Sz))) / 10;
	pstXyz->v[1] = (gstCorrect.s32K * ((gstCorrect.s32A4 * pst->s32Sx)
			 + (gstCorrect.s32A5 * pst->s32Sy)
			 + (gstCorrect.s32A6 * pst->s32Sz))) / 10;
	pstXyz->v[2] = (gstCorrect.s32K * ((gstCorrect.s32A7 * pst->s32Sx)
			 + (gstCorrect.s32A8 * pst->s32Sy)
			 + (gstCorrect.s32A9 * pst->s32Sz))) / 10;
}

static void
yas530_calc_magnetic_field(struct yas_pcb_vector *pstXy1y2,
		struct yas_pcb_vector *pstXyz, int32_t s32Coef)
{
	int32_t s32Oy;
	int32_t s32Oz;
	static const int32_t s32HTbl[] = {
		 1748, 1948, 2148, 2348
	};

	pstXy1y2->v[0]  = gstXy1y2.v[0] - s32HTbl[gstCorrect.s32Fx]
		+ (gs08HardOffset[0] - gstCorrect.s32Rx) * s32Coef;
	pstXy1y2->v[1] = gstXy1y2.v[1] - s32HTbl[gstCorrect.s32Fy1]
		+ (gs08HardOffset[1] - gstCorrect.s32Ry1) * s32Coef;
	pstXy1y2->v[2] = gstXy1y2.v[2] - s32HTbl[gstCorrect.s32Fy2]
		+ (gs08HardOffset[2] - gstCorrect.s32Ry2) * s32Coef;
	s32Oy  = pstXy1y2->v[1] - pstXy1y2->v[2];
	s32Oz  = -pstXy1y2->v[1] - pstXy1y2->v[2];

	pstXyz->v[0] = (gstCorrect.s32K
		* ((100 * pstXy1y2->v[0])
		+ (gstCorrect.s32A2 * s32Oy)
		+ (gstCorrect.s32A3 * s32Oz))) / 10;
	pstXyz->v[1] = (gstCorrect.s32K
		* ((gstCorrect.s32A4 * pstXy1y2->v[0])
		+ (gstCorrect.s32A5 * s32Oy)
		+ (gstCorrect.s32A6 * s32Oz))) / 10;
	pstXyz->v[2] = (gstCorrect.s32K
		* ((gstCorrect.s32A7 * pstXy1y2->v[0])
		+ (gstCorrect.s32A8 * s32Oy)
		+ (gstCorrect.s32A9 * s32Oz))) / 10;
}

static void
yas532_calc_magnetic_field(struct yas_pcb_vector *pstXy1y2,
	struct yas_pcb_vector *pstXyz,
	int32_t s32CoefX, int32_t s32CoefY1, int32_t s32CoefY2)
{
	int32_t s32Oy;
	int32_t s32Oz;
	static const int32_t s32HTbl[] = {
		3721, 3971, 4221, 4471
	};

	pstXy1y2->v[0]  = gstXy1y2.v[0] - s32HTbl[gstCorrect.s32Fx]
		+ (gs08HardOffset[0] - gstCorrect.s32Rx) * s32CoefX;
	pstXy1y2->v[1] = gstXy1y2.v[1] - s32HTbl[gstCorrect.s32Fy1]
		+ (gs08HardOffset[1] - gstCorrect.s32Ry1) * s32CoefY1;
	pstXy1y2->v[2] = gstXy1y2.v[2] - s32HTbl[gstCorrect.s32Fy2]
		+ (gs08HardOffset[2] - gstCorrect.s32Ry2) * s32CoefY2;
	s32Oy  = pstXy1y2->v[1] - pstXy1y2->v[2];
	s32Oz  = -pstXy1y2->v[1] - pstXy1y2->v[2];

	pstXyz->v[0] = (gstCorrect.s32K
		* ((100 * pstXy1y2->v[0])
		+ (gstCorrect.s32A2 * s32Oy)
		+ (gstCorrect.s32A3 * s32Oz))) / 10;
	pstXyz->v[1] = (gstCorrect.s32K
		* ((gstCorrect.s32A4 * pstXy1y2->v[0])
		+ (gstCorrect.s32A5 * s32Oy)
		+ (gstCorrect.s32A6 * s32Oz))) / 10;
	pstXyz->v[2] = (gstCorrect.s32K
		* ((gstCorrect.s32A7 * pstXy1y2->v[0])
		+ (gstCorrect.s32A8 * s32Oy)
		+ (gstCorrect.s32A9 * s32Oz))) / 10;
}

static int
yas_pcb_calc_magnetic_field(struct yas_pcb_vector *pstXy1y2,
		struct yas_pcb_vector *pstXyz)
{
	int32_t s32Coef;

	if (YAS532_DEVICE_ID == gu08DevId) {
		switch (gstCorrect.s32Ver) {
		case YAS532_VERSION_AC:
			break;

		default:
			return YAS_PCB_ERROR_I2C;
			/* break; */
		}

		/* calculate Ohx/y/z[nT] */
		yas532_calc_magnetic_field(pstXy1y2, pstXyz,
				YAS532_COEFX_VERSION_AC,
				YAS532_COEFY1_VERSION_AC,
				YAS532_COEFY2_VERSION_AC);
	} else {
		switch (gstCorrect.s32Ver) {
		case YAS530_VERSION_A:
			s32Coef = YAS530_COEF_VERSION_A;
			break;

		case YAS530_VERSION_B:
			s32Coef = YAS530_COEF_VERSION_B;
			break;

		default:
			return YAS_PCB_ERROR_I2C;
			/* break; */
		}

		/* calculate Ohx/y/z[nT] */
		yas530_calc_magnetic_field(pstXy1y2, pstXyz, s32Coef);
	}

	return YAS_PCB_NO_ERROR;
}

static int
yas_pcb_test1(int *id)
{
	int result;
	int ret;

	result = yas_pcb_power_on();
	if (YAS_PCB_NO_ERROR == result) {
		result = YAS_PCB_ERROR_I2C;
		ret = g_callback.i2c_read(YAS_PCB_ADDR_SLAVE,
			YAS_PCB_ADDR_ID, &gu08DevId, 1);
		if (0 == ret) {
			*id = (int)gu08DevId;
			result = YAS_PCB_NO_ERROR;
		}
	}

	return result;
}

static int
yas_pcb_test2(void)
{
	return yas_pcb_power_off();
}

static int
yas_pcb_test3(void)
{
	int result;
	int ret;
	uint8_t u08Data;
	uint8_t pu08Buf[YAS530_CAL_REG_NUM];

	u08Data = YAS_PCB_TEST1_DEFAULT;
	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_TEST1,
			&u08Data, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	u08Data = YAS_PCB_TEST2_DEFAULT;
	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE, YAS_PCB_ADDR_TEST2,
			&u08Data, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	u08Data = YAS_PCB_INTERVAL_DEFAULT;
	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE,
		YAS_PCB_ADDR_MEASURE_INTERVAL, &u08Data, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	if (YAS532_DEVICE_ID == gu08DevId) {
		gs32Center = YAS532_RAWDATA_CENTER;
		gs32Overflow = YAS532_RAWDATA_OVERFLOW;
		result = yas532_read_cal(pu08Buf);
		if (YAS_PCB_NO_ERROR == result)
			yas532_calc_correction(pu08Buf);
	} else {
		gs32Center = YAS530_RAWDATA_CENTER;
		gs32Overflow = YAS530_RAWDATA_OVERFLOW;
		result = yas530_read_cal(pu08Buf);
		if (YAS_PCB_NO_ERROR == result)
			yas530_calc_correction(pu08Buf);
	}

	if (YAS_PCB_NO_ERROR != result)
		return result;

	u08Data = (uint8_t)(YAS_PCB_CONFIG_DEFAULT
			| (uint8_t)((pu08Buf[9]  & 0x03) << 3)
			| (uint8_t)((pu08Buf[10] & 0x80) >> 5));
	ret = g_callback.i2c_write(YAS_PCB_ADDR_SLAVE,
		YAS_PCB_ADDR_CONFIG, &u08Data, 1);
	if (0 != ret)
		return YAS_PCB_ERROR_I2C;

	ret = yas_pcb_reset_coil();
	if (YAS_PCB_NO_ERROR != ret)
		return ret;

	return YAS_PCB_NO_ERROR;
}

static int
yas_pcb_test4(int *x, int *y1, int *y2)
{
	int result;
	struct yas_pcb_vector stXy1y2;
	int temperature;
	int32_t s32Tmp;
	int8_t s08Correct = YAS_PCB_HARD_OFFSET_CORRECT;
	uint8_t i;
	uint8_t k;

	gs08HardOffset[0] = 0;
	gs08HardOffset[1] = 0;
	gs08HardOffset[2] = 0;
	result = yas_pcb_set_offset(&gs08HardOffset[0]);
	if (YAS_PCB_NO_ERROR == result) {
		/* calc hard offset */
		for (i = 0; i < YAS_PCB_COIL_INIT_CALC_NUM; i++) {
			result = yas_pcb_measure(&stXy1y2, &temperature,
				YAS_PCB_MEASURE_COMMAND_START,
				YAS_PCB_INT_NOTCHECK);
			if (YAS_PCB_NO_ERROR != result)
				break;

			for (k = 0; k < 3; k++) {
				s32Tmp = stXy1y2.v[k];
				if (gs32Center < s32Tmp)
					gs08HardOffset[k] += s08Correct;
				else {
					if (s32Tmp < gs32Center)
						gs08HardOffset[k] -= s08Correct;
				}
			}

			result = yas_pcb_set_offset(&gs08HardOffset[0]);
			if (YAS_PCB_NO_ERROR != result)
				break;

			s08Correct = (int8_t)((uint8_t)s08Correct >> 1);
		}

		if (YAS_PCB_NO_ERROR == result) {
			*x  = (int)gs08HardOffset[0];
			*y1 = (int)gs08HardOffset[1];
			*y2 = (int)gs08HardOffset[2];
			result = yas_pcb_is_flow_occued(&stXy1y2,
						0, gs32Overflow);
		}
	}

	return result;
}

static int
yas_pcb_test5(int *direction)
{
	uint16_t dir;
	int result;
	int ret;
	int x;
	int y;
	int nTemp;
	struct yas_pcb_vector stXyz;

	result = yas_pcb_measure(&gstXy1y2, &nTemp,
		YAS_PCB_MEASURE_COMMAND_START, YAS_PCB_INT_NOTCHECK);
	if (YAS_PCB_NO_ERROR == result) {
		result = YAS_PCB_ERROR_DIRCALC;
		yas_pcb_calc_position(&gstXy1y2, &stXyz, nTemp);

		x = -stXyz.v[0] / YAS_PCB_DIR_DIVIDER;
		y = stXyz.v[1] / YAS_PCB_DIR_DIVIDER;
		ret = Ms3AxesLibDir8(x, y, &dir);
		if (0 == ret) {
			*direction = (int)(dir / 8);
			result = yas_pcb_is_flow_occued(&gstXy1y2,
						0, gs32Overflow);
		}
	}

	return result;
}

static int
yas_pcb_test6(int *sx, int *sy)
{
	int result;
	struct yas_pcb_vector stXy1y2P;
	struct yas_pcb_vector stXy1y2N;
	int temperature;
	uint8_t u08Command;
	struct yas_pcb_vector *pP = &stXy1y2P;
	struct yas_pcb_vector *pN = &stXy1y2N;
	struct yas_pcb_correction *pC = &gstCorrect;

	u08Command = YAS_PCB_MEASURE_COMMAND_START
		 | YAS_PCB_MEASURE_COMMAND_LDTC;
	result = yas_pcb_measure(pP, &temperature,
				u08Command, YAS_PCB_INT_CHECK);

	if (YAS_PCB_NO_ERROR == result) {
		u08Command = YAS_PCB_MEASURE_COMMAND_START
			 | YAS_PCB_MEASURE_COMMAND_LDTC
			 | YAS_PCB_MEASURE_COMMAND_FORS;
		result = yas_pcb_measure(pN, &temperature, u08Command,
				YAS_PCB_INT_NOTCHECK);

		if (YAS_PCB_NO_ERROR == result) {
			if (YAS532_DEVICE_ID == gu08DevId) {
				*sx = (int)(pC->s32K * 100
					 * (pP->v[0] - pN->v[0]));
				*sx /= 1000;
				*sx /= YAS_VCORE;
				*sy = (int)(pC->s32K * pC->s32A5
					 * ((pP->v[1] - pN->v[1])
					  - (pP->v[2] - pN->v[2])));
				*sy /= 1000;
				*sy /= YAS_VCORE;
			} else {
				*sx = (int)(pN->v[0] - pP->v[0]);
				*sy = (int)((pN->v[1] - pP->v[1])
					  - (pN->v[2] - pP->v[2]));
			}

			result = yas_pcb_is_flow_occued(pP, 0, gs32Overflow);
			if (YAS_PCB_NO_ERROR == result)
				result = yas_pcb_is_flow_occued(pN,
					0, gs32Overflow);
		}
	}

	return result;
}

#ifdef YAS_PCBTEST_EXTRA
static int
yas_pcb_test7(int *ohx, int *ohy, int *ohz)
{
	int nRet = YAS_PCB_ERROR_NOT_SUPPORTED;
	struct yas_pcb_vector stOhxy1y2, stOhxyz;

	if (0 != gstCorrect.s32ZFlag) {
		nRet = yas_pcb_calc_magnetic_field(&stOhxy1y2, &stOhxyz);
		if (YAS_PCB_NO_ERROR == nRet) {
			/* [nT]->[uT] */
			*ohx = stOhxyz.v[0] / 1000;
			*ohy = stOhxyz.v[1] / 1000;
			*ohz = stOhxyz.v[2] / 1000;
		}
	}

	return nRet;
}

static int
yas_pcb_test8(int *hx0, int *hy0, int *hz0)
{
	int nRet;
	int nTemp;
	int nX, nY1, nY2;
	int32_t s32Underflow = 0;
	int32_t s32Overflow = gs32Overflow;
	struct yas_pcb_vector stOhxy1y2, stOhxyz;

	if (YAS532_DEVICE_ID == gu08DevId) {
		s32Underflow = YAS_PCB_NOISE_UNDERFLOW;
		s32Overflow = YAS_PCB_NOISE_OVERFLOW;
	}

	if (gu08Recalc != 0) {
		gs32RecalcWait++;
		if (YAS_PCB_NOISE_INTERVAL <= gs32RecalcWait) {
			nRet = yas_pcb_reset_coil();
			if (YAS_PCB_NO_ERROR == nRet)
				nRet = yas_pcb_test4(&nX, &nY1, &nY2);

			if (YAS_PCB_NO_ERROR == nRet)
				gu08Recalc = 0;

			gs32RecalcWait = 0;
		}
	}

	if (0 != gstCorrect.s32ZFlag) {
		nRet = yas_pcb_measure(&gstXy1y2, &nTemp,
			YAS_PCB_MEASURE_COMMAND_START,
			YAS_PCB_INT_NOTCHECK);
		if (YAS_PCB_NO_ERROR == nRet) {
			nRet = yas_pcb_calc_magnetic_field(&stOhxy1y2,
				&stOhxyz);
			if (YAS_PCB_NO_ERROR == nRet) {
				*hx0 = stOhxy1y2.v[0];
				*hy0 = stOhxy1y2.v[1] - stOhxy1y2.v[2];
				*hz0 = -stOhxy1y2.v[1] - stOhxy1y2.v[2];
				nRet = yas_pcb_is_flow_occued(&gstXy1y2,
						s32Underflow, s32Overflow);
				if (YAS_PCB_NO_ERROR != nRet) {
					if (gu08Recalc == 0) {
						gu08Recalc++;
						gs32RecalcWait = 0;
					}
				}
			}
		}
	} else
		nRet = YAS_PCB_ERROR_NOT_SUPPORTED;

	return nRet;
}

#endif

/* test 1 */
static int
power_on_and_device_check(int *id)
{
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST1);

	if (YAS_PCB_NO_ERROR == result) {
		if (id != NULL) {
			result = YAS_PCB_ERROR_I2C;
			ret = g_callback.i2c_open();
			if (0 == ret) {
				result = yas_pcb_test1(id);
				ret = g_callback.i2c_close();
				if (0 != ret)
					result = YAS_PCB_ERROR_I2C;
			}
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST1);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
}

/* test 2 */
static int
power_off(void)
{
	int result = yas_pcb_check_state(YAS_PCB_TEST2);

	if (YAS_PCB_NO_ERROR == result) {
		result = yas_pcb_test2();
		if (YAS_PCB_NO_ERROR == result)
			yas_pcb_update_state(YAS_PCB_TEST2);
	}

	return result;
}

/* test 3 */
static int
initialization(void)
{
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST3);

	if (YAS_PCB_NO_ERROR == result) {
		result = YAS_PCB_ERROR_I2C;
		ret = g_callback.i2c_open();
		if (0 == ret) {
			result = yas_pcb_test3();
			ret = g_callback.i2c_close();
			if (0 != ret)
				result = YAS_PCB_ERROR_I2C;
		}
		if (YAS_PCB_NO_ERROR == result)
			yas_pcb_update_state(YAS_PCB_TEST3);
	}

	return result;
}

/* test 4 */
static int
offset_control_measurement_and_set_offset_register(int *x, int *y1, int *y2)
{
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST4);

	if (YAS_PCB_NO_ERROR == result) {
		if ((x != NULL) && (y1 != NULL) && (y2 != NULL)) {
			result = YAS_PCB_ERROR_I2C;
			ret = g_callback.i2c_open();
			if (0 == ret) {
				result = yas_pcb_test4(x, y1, y2);
				ret = g_callback.i2c_close();
				if (0 != ret)
					result = YAS_PCB_ERROR_I2C;
			}
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST4);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
}

/* test 5 */
static int
direction_measurement(int *direction)
{
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST5);

	if (YAS_PCB_NO_ERROR == result) {
		if (direction != NULL) {
			result = YAS_PCB_ERROR_I2C;
			ret = g_callback.i2c_open();
			if (0 == ret) {
				result = yas_pcb_test5(direction);
				ret = g_callback.i2c_close();
				if (0 != ret)
					result = YAS_PCB_ERROR_I2C;
			}
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST5);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
}

/* test 6 */
static int
sensitivity_measurement_of_magnetic_sensor_by_test_coil(int *sx, int *sy)
{
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST6);

	if (YAS_PCB_NO_ERROR == result) {
		if ((sx != NULL) && (sy != NULL)) {
			result = YAS_PCB_ERROR_I2C;
			ret = g_callback.i2c_open();
			if (0 == ret) {
				result = yas_pcb_test6(sx, sy);
				ret = g_callback.i2c_close();
				if (0 != ret)
					result = YAS_PCB_ERROR_I2C;
			}
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST6);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
}

/* test 7 */
static int
magnetic_field_level_check(int *ohx, int *ohy, int *ohz)
{
#ifdef YAS_PCBTEST_EXTRA
	int result = yas_pcb_check_state(YAS_PCB_TEST7);

	if (YAS_PCB_NO_ERROR == result) {
		if ((ohx != NULL) && (ohy != NULL) && (ohz != NULL)) {
			result = yas_pcb_test7(ohx, ohy, ohz);
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST7);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
#else
	return YAS_PCB_ERROR_NOT_SUPPORTED;
#endif
}

/* test 8 */
static int
noise_level_check(int *hx0, int *hy0, int *hz0)
{
#ifdef YAS_PCBTEST_EXTRA
	int ret;
	int result = yas_pcb_check_state(YAS_PCB_TEST8);

	if (YAS_PCB_NO_ERROR == result) {
		if ((hx0 != NULL) && (hy0 != NULL) && (hz0 != NULL)) {
			result = YAS_PCB_ERROR_I2C;
			ret = g_callback.i2c_open();
			if (0 == ret) {
				result = yas_pcb_test8(hx0, hy0, hz0);
				ret = g_callback.i2c_close();
				if (0 != ret)
					result = YAS_PCB_ERROR_I2C;
			}
			if (YAS_PCB_NO_ERROR == result)
				yas_pcb_update_state(YAS_PCB_TEST8);
		} else
			result = YAS_PCB_ERROR_ARG;
	}

	return result;
#else
	return YAS_PCB_ERROR_NOT_SUPPORTED;
#endif
}

/* pcb test module initialize */
int
yas_pcb_test_init(struct yas_pcb_test *func)
{
	int result = YAS_PCB_ERROR_ARG;

	if ((NULL != func)
	 && (NULL != func->callback.i2c_open)
	 && (NULL != func->callback.i2c_close)
	 && (NULL != func->callback.i2c_write)
	 && (NULL != func->callback.i2c_read)
	 && (NULL != func->callback.msleep)) {
		func->power_on_and_device_check = power_on_and_device_check;
		func->initialization = initialization;
		func->offset_control_measurement_and_set_offset_register
		 = offset_control_measurement_and_set_offset_register;
		func->direction_measurement = direction_measurement;
		func->sensitivity_measurement_of_magnetic_sensor_by_test_coil
		 = sensitivity_measurement_of_magnetic_sensor_by_test_coil;
		func->magnetic_field_level_check = magnetic_field_level_check;
		func->noise_level_check = noise_level_check;
		func->power_off = power_off;

		g_callback = func->callback;

		if (0 != gu16State) {
			gu16State = 0;
			yas_pcb_power_off();
		}
#ifdef YAS_PCBTEST_EXTRA
		gu08Recalc = 0;
		gs32RecalcWait = 0;
#endif

		result = YAS_PCB_NO_ERROR;
	}

	return result;
}
/* end of file */
