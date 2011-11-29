/*
 $License:
    Copyright (C) 2011 InvenSense Corporation, All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  $
 */

/**
 *  @addtogroup COMPASSDL
 *
 *  @{
 *      @file   ami306.c
 *      @brief  Magnetometer setup and handling methods for Aichi AMI306
 *              compass.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/delay.h>
#endif

#include "mpu.h"
#include "mlsl.h"
#include "mlos.h"
#include "ami_hw.h"
#include "ami_sensor_def.h"

#include <log.h>
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-compass"

#define AMI306_REG_DATAX		(0x10)
#define AMI306_REG_STAT1		(0x18)
#define AMI306_REG_CNTL1		(0x1B)
#define AMI306_REG_CNTL2		(0x1C)
#define AMI306_REG_CNTL3		(0x1D)
#define AMI306_REG_CNTL4_1		(0x5C)
#define AMI306_REG_CNTL4_2		(0x5D)

#define AMI306_BIT_CNTL1_PC1		(0x80)
#define AMI306_BIT_CNTL1_ODR1		(0x10)
#define AMI306_BIT_CNTL1_FS1		(0x02)

#define AMI306_BIT_CNTL2_IEN		(0x10)
#define AMI306_BIT_CNTL2_DREN		(0x08)
#define AMI306_BIT_CNTL2_DRP		(0x04)
#define AMI306_BIT_CNTL3_F0RCE		(0x40)

#define AMI_FINE_MAX			(96)
#define AMI_STANDARD_OFFSET		(0x800)
#define AMI_GAIN_COR_DEFAULT		(1000)

struct ami306_private_data {
	int isstandby;
	unsigned char fine[3];
	AMI_SENSOR_PARAMETOR param;
	AMI_WIN_PARAMETER win;
};

static inline unsigned short little_u8_to_u16(unsigned char *p_u8)
{
	return p_u8[0] | (p_u8[1] << 8);
}

static int ami306_set_bits8(void *mlsl_handle,
			    struct ext_slave_platform_data *pdata,
			    unsigned char reg, unsigned char bits)
{
	int result;
	unsigned char buf;

	result = inv_serial_read(mlsl_handle, pdata->address, reg, 1, &buf);
	ERROR_CHECK(result);

	buf |= bits;
	result = inv_serial_single_write(mlsl_handle, pdata->address, reg, buf);
	ERROR_CHECK(result);
	return result;
}

static int ami306_wait_data_ready(void *mlsl_handle,
				  struct ext_slave_platform_data *pdata,
				  unsigned long usecs, unsigned long times)
{
	int result = 0;
	unsigned char buf;

	for (; 0 < times; --times) {
#ifdef __KERNEL__
		udelay(usecs);
#else
		inv_sleep(1);
#endif
		result = inv_serial_read(mlsl_handle, pdata->address,
					 AMI_REG_STA1, 1, &buf);
		if (buf & AMI_STA1_DRDY_BIT)
			return 0;
		else if (buf & AMI_STA1_DOR_BIT)
			return INV_ERROR_COMPASS_DATA_OVERFLOW;
	}
	return INV_ERROR_COMPASS_DATA_NOT_READY;
}

static int ami306_read_raw_data(void *mlsl_handle,
				struct ext_slave_platform_data *pdata,
				short dat[3])
{
	int result;
	unsigned char buf[6];
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_DATAX, sizeof(buf), buf);
	ERROR_CHECK(result);
	dat[0] = little_u8_to_u16(&buf[0]);
	dat[1] = little_u8_to_u16(&buf[2]);
	dat[2] = little_u8_to_u16(&buf[4]);
	return result;
}

#define AMI_WAIT_DATAREADY_RETRY		3	/* retry times */
#define AMI_DRDYWAIT				800	/* u(micro) sec */
static int ami306_force_mesurement(void *mlsl_handle,
				   struct ext_slave_platform_data *pdata,
				   short ver[3])
{
	int result;
	int status;
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL3, AMI_CTRL3_FORCE_BIT);
	ERROR_CHECK(result);

	result = ami306_wait_data_ready(mlsl_handle, pdata,
					AMI_DRDYWAIT, AMI_WAIT_DATAREADY_RETRY);
	if (result && result != INV_ERROR_COMPASS_DATA_OVERFLOW)
		ERROR_CHECK(result);
	/*  READ DATA X,Y,Z */
	status = ami306_read_raw_data(mlsl_handle, pdata, ver);
	ERROR_CHECK(status);

	return result;
}

static int ami306_mea(void *mlsl_handle,
		      struct ext_slave_platform_data *pdata, short val[3])
{
	int result = ami306_force_mesurement(mlsl_handle, pdata, val);
	ERROR_CHECK(result);
	val[0] += AMI_STANDARD_OFFSET;
	val[1] += AMI_STANDARD_OFFSET;
	val[2] += AMI_STANDARD_OFFSET;
	return result;
}

static int ami306_write_offset(void *mlsl_handle,
			       struct ext_slave_platform_data *pdata,
			       unsigned char *fine)
{
	int result = 0;
	unsigned char dat[3];
	dat[0] = AMI_REG_OFFX;
	dat[1] = 0x7f & fine[0];
	dat[2] = 0;
	result = inv_serial_write(mlsl_handle, pdata->address,
				  sizeof(dat), dat);
	dat[0] = AMI_REG_OFFY;
	dat[1] = 0x7f & fine[1];
	dat[2] = 0;
	result = inv_serial_write(mlsl_handle, pdata->address,
				  sizeof(dat), dat);
	dat[0] = AMI_REG_OFFZ;
	dat[1] = 0x7f & fine[2];
	dat[2] = 0;
	result = inv_serial_write(mlsl_handle, pdata->address,
				  sizeof(dat), dat);
	return result;
}

static int ami306_start_sensor(void *mlsl_handle,
			       struct ext_slave_platform_data *pdata)
{
	int result = 0;
	unsigned char buf[3];
	struct ami306_private_data *private_data = pdata->private_data;

	/* Step 1 */
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL1,
				  AMI_CTRL1_PC1 | AMI_CTRL1_FS1_FORCE);
	ERROR_CHECK(result);
	/* Step 2 */
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL2, AMI_CTRL2_DREN);
	ERROR_CHECK(result);
	/* Step 3 */
	buf[0] = AMI_REG_CTRL4;
	buf[1] = AMI_CTRL4_HS & 0xFF;
	buf[2] = (AMI_CTRL4_HS >> 8) & 0xFF;
	result = inv_serial_write(mlsl_handle, pdata->address,
				  sizeof(buf), buf);
	ERROR_CHECK(result);
	/* Step 4 */
	result = ami306_write_offset(mlsl_handle, pdata, private_data->fine);
	ERROR_CHECK(result);
	return result;
}

/**
 * This function does this.
 *
 * @param mlsl_handle this param is this.
 * @param slave
 * @param pdata
 *
 * @return INV_SUCCESS or non-zero error code.
 */
static int ami306_read_param(void *mlsl_handle,
			     struct ext_slave_descr *slave,
			     struct ext_slave_platform_data *pdata)
{
	int result = 0;
	unsigned char regs[12];
	struct ami306_private_data *private_data = pdata->private_data;
	AMI_SENSOR_PARAMETOR *param = &private_data->param;

	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_SENX, sizeof(regs), regs);
	ERROR_CHECK(result);

	/* Little endian 16 bit registers */
	param->m_gain.x = little_u8_to_u16(&regs[0]);
	param->m_gain.y = little_u8_to_u16(&regs[2]);
	param->m_gain.z = little_u8_to_u16(&regs[4]);

	param->m_interference.xy = regs[7];
	param->m_interference.xz = regs[6];
	param->m_interference.yx = regs[9];
	param->m_interference.yz = regs[8];
	param->m_interference.zx = regs[11];
	param->m_interference.zy = regs[10];

	param->m_offset.x = AMI_STANDARD_OFFSET;
	param->m_offset.y = AMI_STANDARD_OFFSET;
	param->m_offset.z = AMI_STANDARD_OFFSET;

	param->m_gain_cor.x = AMI_GAIN_COR_DEFAULT;
	param->m_gain_cor.y = AMI_GAIN_COR_DEFAULT;
	param->m_gain_cor.z = AMI_GAIN_COR_DEFAULT;

	return result;
}

static int ami306_initial_b0_adjust(void *mlsl_handle,
				    struct ext_slave_descr *slave,
				    struct ext_slave_platform_data *pdata)
{
	int result;
	unsigned char fine[3] = { 0 };
	short data[3];
	int diff[3] = { 0x7fff, 0x7fff, 0x7fff };
	int fn = 0;
	int ax = 0;
	unsigned char buf[3];
	struct ami306_private_data *private_data = pdata->private_data;

	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL2, AMI_CTRL2_DREN);
	ERROR_CHECK(result);

	buf[0] = AMI_REG_CTRL4;
	buf[1] = AMI_CTRL4_HS & 0xFF;
	buf[2] = (AMI_CTRL4_HS >> 8) & 0xFF;
	result = inv_serial_write(mlsl_handle, pdata->address,
				  sizeof(buf), buf);
	ERROR_CHECK(result);

	for (fn = 0; fn < AMI_FINE_MAX; ++fn) {	/* fine 0 -> 95 */
		fine[0] = fine[1] = fine[2] = fn;
		result = ami306_write_offset(mlsl_handle, pdata, fine);
		ERROR_CHECK(result);

		result = ami306_force_mesurement(mlsl_handle, pdata, data);
		ERROR_CHECK(result);
		MPL_LOGV("[%d] x:%-5d y:%-5d z:%-5d\n",
			 fn, data[0], data[1], data[2]);

		for (ax = 0; ax < 3; ax++) {
			/* search point most close to zero. */
			if (diff[ax] > abs(data[ax])) {
				private_data->fine[ax] = fn;
				diff[ax] = abs(data[ax]);
			}
		}
	}
	MPL_LOGV("fine x:%-5d y:%-5d z:%-5d\n",
		 private_data->fine[0], private_data->fine[1],
		 private_data->fine[2]);

	result = ami306_write_offset(mlsl_handle, pdata, private_data->fine);
	ERROR_CHECK(result);

	/* Software Reset */
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL3, AMI_CTRL3_SRST_BIT);
	ERROR_CHECK(result);
	return result;
}

#define SEH_RANGE_MIN 100
#define SEH_RANGE_MAX 3950
static int ami306_search_offset(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata)
{
	int result;
	int axis;
	unsigned char regs[6];
	unsigned char run_flg[3] = { 1, 1, 1 };
	unsigned char fine[3];
	unsigned char win_range_fine[3];
	unsigned short fine_output[3];
	short val[3];
	unsigned short cnt[3] = { 0 };
	struct ami306_private_data *private_data = pdata->private_data;

	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_FINEOUTPUT_X, sizeof(regs), regs);
	ERROR_CHECK(result);
	fine_output[0] = little_u8_to_u16(&regs[0]);
	fine_output[1] = little_u8_to_u16(&regs[2]);
	fine_output[2] = little_u8_to_u16(&regs[4]);

	for (axis = 0; axis < 3; ++axis) {
		if (fine_output[axis] == 0) {
			MPL_LOGV("error fine_output %d  axis:%d\n",
				 __LINE__, axis);
			return -1;
		}
		/*  fines per a window */
		win_range_fine[axis] = (SEH_RANGE_MAX - SEH_RANGE_MIN)
		    / fine_output[axis];
	}

	/* get current fine */
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFX, 2, &regs[0]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFY, 2, &regs[2]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFZ, 2, &regs[4]);
	ERROR_CHECK(result);

	fine[0] = (unsigned char)(regs[0] & 0x7f);
	fine[1] = (unsigned char)(regs[2] & 0x7f);
	fine[2] = (unsigned char)(regs[4] & 0x7f);

	while (run_flg[0] == 1 || run_flg[1] == 1 || run_flg[2] == 1) {

		result = ami306_mea(mlsl_handle, pdata, val);
		ERROR_CHECK(result);
		MPL_LOGV("val  x:%-5d y:%-5d z:%-5d\n", val[0], val[1], val[2]);
		MPL_LOGV("now fine x:%-5d y:%-5d z:%-5d\n",
			 fine[0], fine[1], fine[2]);

		for (axis = 0; axis < 3; ++axis) {
			if (axis == 0) {	/* X-axis is reversed */
				val[axis] = 0x0FFF & ~val[axis];
			}
			if (val[axis] < SEH_RANGE_MIN) {
				/* At the case of less low limmit. */
				fine[axis] -= win_range_fine[axis];
				MPL_LOGV("min : fine=%d diff=%d\n",
					 fine[axis], win_range_fine[axis]);
			}
			if (val[axis] > SEH_RANGE_MAX) {
				/* At the case of over high limmit. */
				fine[axis] += win_range_fine[axis];
				MPL_LOGV("max : fine=%d diff=%d\n",
					 fine[axis], win_range_fine[axis]);
			}
			if (SEH_RANGE_MIN <= val[axis] &&
			    val[axis] <= SEH_RANGE_MAX) {
				/* In the current window. */
				int diff_fine =
				    (val[axis] - AMI_STANDARD_OFFSET) /
				    fine_output[axis];
				fine[axis] += diff_fine;
				run_flg[axis] = 0;
				MPL_LOGV("mid : fine=%d diff=%d\n",
					 fine[axis], diff_fine);
			}

			if (!(0 <= fine[axis] && fine[axis] < AMI_FINE_MAX)) {
				MPL_LOGE("fine err :%d\n", cnt[axis]);
				goto out;
			}
			if (cnt[axis] > 3) {
				MPL_LOGE("cnt err :%d\n", cnt[axis]);
				goto out;
			}
			cnt[axis]++;
		}
		MPL_LOGV("new fine x:%-5d y:%-5d z:%-5d\n",
			 fine[0], fine[1], fine[2]);

		/* set current fine */
		result = ami306_write_offset(mlsl_handle, pdata, fine);
		ERROR_CHECK(result);
	}
	memcpy(private_data->fine, fine, sizeof(fine));
 out:
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL3, AMI_CTRL3_SRST_BIT);
	ERROR_CHECK(result);
#ifdef __KERNEL__
	udelay(250 + 50);
#else
	inv_sleep(1);
#endif
	return 0;
}

static int ami306_read_win(void *mlsl_handle,
			   struct ext_slave_descr *slave,
			   struct ext_slave_platform_data *pdata)
{
	int result = 0;
	unsigned char regs[6];
	struct ami306_private_data *private_data = pdata->private_data;
	AMI_WIN_PARAMETER *win = &private_data->win;

	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFOTPX, sizeof(regs), regs);
	ERROR_CHECK(result);

	win->m_0Gauss_fine.x = (unsigned char)(regs[0] & 0x7f);
	win->m_0Gauss_fine.y = (unsigned char)(regs[2] & 0x7f);
	win->m_0Gauss_fine.z = (unsigned char)(regs[4] & 0x7f);

	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFX, 2, &regs[0]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFY, 2, &regs[2]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_REG_OFFZ, 2, &regs[4]);
	ERROR_CHECK(result);

	win->m_fine.x = (unsigned char)(regs[0] & 0x7f);
	win->m_fine.y = (unsigned char)(regs[2] & 0x7f);
	win->m_fine.z = (unsigned char)(regs[4] & 0x7f);

	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI_FINEOUTPUT_X, sizeof(regs), regs);
	ERROR_CHECK(result);
	win->m_fine_output.x = little_u8_to_u16(&regs[0]);
	win->m_fine_output.y = little_u8_to_u16(&regs[2]);
	win->m_fine_output.z = little_u8_to_u16(&regs[4]);

	return result;
}

int ami306_suspend(void *mlsl_handle,
		   struct ext_slave_descr *slave,
		   struct ext_slave_platform_data *pdata)
{
	int result;
	unsigned char reg;
	result = inv_serial_read(mlsl_handle, pdata->address,
				 AMI306_REG_CNTL1, 1, &reg);
	ERROR_CHECK(result);

	reg &= ~(AMI306_BIT_CNTL1_PC1 | AMI306_BIT_CNTL1_FS1);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 AMI306_REG_CNTL1, reg);
	ERROR_CHECK(result);

	return result;
}

int ami306_resume(void *mlsl_handle,
		  struct ext_slave_descr *slave,
		  struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;
	unsigned char regs[] = {
		AMI306_REG_CNTL4_1,
		0x7E,
		0xA0
	};
	/* Step1. Set CNTL1 reg to power model active (Write CNTL1:PC1=1) */
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 AMI306_REG_CNTL1,
					 AMI306_BIT_CNTL1_PC1 |
					 AMI306_BIT_CNTL1_FS1);
	ERROR_CHECK(result);

	/* Step2. Set CNTL2 reg to DRDY active high and enabled
	   (Write CNTL2:DREN=1) */
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 AMI306_REG_CNTL2,
					 AMI306_BIT_CNTL2_DREN |
					 AMI306_BIT_CNTL2_DRP);
	ERROR_CHECK(result);

	/* Step3. Set CNTL4 reg to for measurement speed: Write CNTL4, 0xA07E */
	result = inv_serial_write(mlsl_handle, pdata->address, DIM(regs), regs);
	ERROR_CHECK(result);

	/* Step4. skipped */

	/* Step5. Set CNTL3 reg to forced measurement period
	   (Write CNTL3:FORCE=1) */
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 AMI306_REG_CNTL3,
					 AMI306_BIT_CNTL3_F0RCE);

	return result;
}

int ami306_read(void *mlsl_handle,
		struct ext_slave_descr *slave,
		struct ext_slave_platform_data *pdata, unsigned char *data)
{
	int result = INV_SUCCESS;
	int ii;
	short val[COMPASS_NUM_AXES];

	result = ami306_mea(mlsl_handle, pdata, val);
	ERROR_CHECK(result);
	for (ii = 0; ii < COMPASS_NUM_AXES; ii++) {
		val[ii] -= AMI_STANDARD_OFFSET;
		data[2 * ii] = val[ii] & 0xFF;
		data[(2 * ii) + 1] = (val[ii] >> 8) & 0xFF;
	}
	return result;
}

static int ami306_init(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	int result;
	struct ami306_private_data *private_data;
	private_data = (struct ami306_private_data *)
	    inv_malloc(sizeof(struct ami306_private_data));
	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;
	result = ami306_set_bits8(mlsl_handle, pdata,
				  AMI_REG_CTRL1,
				  AMI_CTRL1_PC1 | AMI_CTRL1_FS1_FORCE);
	ERROR_CHECK(result);
	/* Read Parameters */
	result = ami306_read_param(mlsl_handle, slave, pdata);
	ERROR_CHECK(result);
	/* Read Window */
	result = ami306_initial_b0_adjust(mlsl_handle, slave, pdata);
	ERROR_CHECK(result);
	result = ami306_start_sensor(mlsl_handle, pdata);
	ERROR_CHECK(result);
	result = ami306_read_win(mlsl_handle, slave, pdata);
	ERROR_CHECK(result);
/*
 *	result = inv_serial_single_write(mlsl_handle, pdata->address,
 *					 AMI306_REG_CNTL1, 0);
 */
   result = inv_serial_single_write(mlsl_handle, pdata->address,
	   			 AMI306_REG_CNTL3, AMI_CTRL3_FORCE_BIT);
 
 	ERROR_CHECK(result);

	return INV_SUCCESS;
}

static int ami306_exit(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	if (pdata->private_data)
		return inv_free(pdata->private_data);
	else
		return INV_SUCCESS;
}

static int ami306_config(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata,
			 struct ext_slave_config *data)
{
	if (!data->data)
		ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);

	switch (data->key) {
	case MPU_SLAVE_PARAM:
		break;
	case MPU_SLAVE_WINDOW:
		break;
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
	case MPU_SLAVE_CONFIG_ODR_RESUME:
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
	case MPU_SLAVE_CONFIG_FSR_RESUME:
	case MPU_SLAVE_CONFIG_MOT_THS:
	case MPU_SLAVE_CONFIG_NMOT_THS:
	case MPU_SLAVE_CONFIG_MOT_DUR:
	case MPU_SLAVE_CONFIG_NMOT_DUR:
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
	default:
		ERROR_CHECK(INV_ERROR_FEATURE_NOT_IMPLEMENTED);
	};

	return INV_SUCCESS;
}

static int ami306_get_config(void *mlsl_handle,
			     struct ext_slave_descr *slave,
			     struct ext_slave_platform_data *pdata,
			     struct ext_slave_config *data)
{
	int result;
	struct ami306_private_data *private_data = pdata->private_data;
	if (!data->data)
		ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);

	switch (data->key) {
	case MPU_SLAVE_PARAM:
		if (sizeof(AMI_SENSOR_PARAMETOR) > data->len)
			ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);
		if (data->apply) {
			result = ami306_read_param(mlsl_handle, slave, pdata);
			ERROR_CHECK(result);
		}
		memcpy(data->data, &private_data->param,
		       sizeof(AMI_SENSOR_PARAMETOR));
		break;
	case MPU_SLAVE_WINDOW:
		if (sizeof(AMI_WIN_PARAMETER) > data->len)
			ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);
		if (data->apply) {
			result = ami306_read_win(mlsl_handle, slave, pdata);
			ERROR_CHECK(result);
		}
		memcpy(data->data, &private_data->win,
		       sizeof(AMI_WIN_PARAMETER));
		break;
	case MPU_SLAVE_SEARCHOFFSET:
		if (sizeof(AMI_WIN_PARAMETER) > data->len)
			ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);
		if (data->apply) {
			result = ami306_search_offset(mlsl_handle,
						      slave, pdata);
			ERROR_CHECK(result);
			/* Start sensor */
			result = ami306_start_sensor(mlsl_handle, pdata);
			ERROR_CHECK(result);
			result = ami306_read_win(mlsl_handle, slave, pdata);
			ERROR_CHECK(result);
		}
		memcpy(data->data, &private_data->win,
		       sizeof(AMI_WIN_PARAMETER));
		break;
	case MPU_SLAVE_READWINPARAMS:
		if (sizeof(AMI_WIN_PARAMETER) > data->len)
			ERROR_CHECK(INV_ERROR_INVALID_PARAMETER);
		if (data->apply) {
			result = ami306_initial_b0_adjust(mlsl_handle,
							  slave, pdata);
			ERROR_CHECK(result);
			/* Start sensor */
			result = ami306_start_sensor(mlsl_handle, pdata);
			ERROR_CHECK(result);
			result = ami306_read_win(mlsl_handle, slave, pdata);
			ERROR_CHECK(result);
		}
		memcpy(data->data, &private_data->win,
		       sizeof(AMI_WIN_PARAMETER));
		break;
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		(*(unsigned long *)data->data) = 0;
		break;
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		(*(unsigned long *)data->data) = 50000;
		break;
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
	case MPU_SLAVE_CONFIG_FSR_RESUME:
	case MPU_SLAVE_CONFIG_MOT_THS:
	case MPU_SLAVE_CONFIG_NMOT_THS:
	case MPU_SLAVE_CONFIG_MOT_DUR:
	case MPU_SLAVE_CONFIG_NMOT_DUR:
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
	case MPU_SLAVE_READ_SCALE:
	default:
		ERROR_CHECK(INV_ERROR_FEATURE_NOT_IMPLEMENTED);
	};

	return INV_SUCCESS;
}

struct ext_slave_read_trigger ami306_read_trigger = {
	/*.reg              = */ AMI_REG_CTRL3,
	/*.value            = */ AMI_CTRL3_FORCE_BIT
};

struct ext_slave_descr ami306_descr = {
	/*.init             = */ ami306_init,
	/*.exit             = */ ami306_exit,
	/*.suspend          = */ ami306_suspend,
	/*.resume           = */ ami306_resume,
	/*.read             = */ ami306_read,
	/*.config           = */ ami306_config,
	/*.get_config       = */ ami306_get_config,
	/*.name             = */ "ami306",
	/*.type             = */ EXT_SLAVE_TYPE_COMPASS,
	/*.id               = */ COMPASS_ID_AMI306,
	/*.reg              = */ 0x0E,
	/*.len              = */ 13,
	/*.endian           = */ EXT_SLAVE_LITTLE_ENDIAN,
	/*.range            = */ {5461, 3333},
	/*.trigger          = */ &ami306_read_trigger,
	/* For AMI305,the range field needs to be modified to {9830.4f} */
};

struct ext_slave_descr *ami306_get_slave_descr(void)
{
	return &ami306_descr;
}
EXPORT_SYMBOL(ami306_get_slave_descr);

/**
 *  @}
 */
