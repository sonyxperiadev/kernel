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
 *      @file   mmc328xma.c
 *      @brief  Magnetometer setup and handling methods for Memsic MMC328XMA
 *              compass.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "mpu-dev.h"

#include <log.h>
#include <linux/mpu.h>
#include "mlsl.h"
#include "mldl_cfg.h"
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-compass"

/* --------------------- */
/* -    Variables.     - */
/* --------------------- */

static int reset_int = 1000;
static int read_count = 1;

#define MMC328XMA_REG_CTL0            (0x07)
#define MMC328XMA_REG_ST              (0x06)
#define MMC328XMA_REG_X_LSB           (0x00)

#define MMC328XMA_VAL_TM_READY        (0x01)
#define MMC328XMA_VAL_RM_MAG          (0x20)
#define MMC328XMA_VAL_RRM_MAG         (0x40)
#define MMC328XMA_CNTL_MODE_WAKE_UP   (0x01)


/*****************************************
    Compass Initialization Functions
*****************************************/

int mmc328xma_suspend(void *mlsl_handle,
		    struct ext_slave_descr *slave,
		    struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;

	return result;
}

int mmc328xma_resume(void *mlsl_handle,
		struct ext_slave_descr *slave,
		struct ext_slave_platform_data *pdata)
{

	int result;
	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_VAL_RM_MAG);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_CNTL_MODE_WAKE_UP);
	msleep(10);

	read_count = 1;
	return INV_SUCCESS;
}

int mmc328xma_read(void *mlsl_handle,
		struct ext_slave_descr *slave,
		struct ext_slave_platform_data *pdata,
		unsigned char *data)
{
	int result;
	unsigned char status = 0;
	int md_times = 0;
	if (read_count > 1000)
		read_count = 1;


	do {
		result = inv_serial_read(mlsl_handle,
				pdata->address,
				MMC328XMA_REG_ST,
				1, &status);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}

		md_times++;
		if (md_times > 3) {
			printk(KERN_INFO "mmc328x tried 3 times, failed");
			return INV_ERROR_COMPASS_DATA_NOT_READY;
		}

		msleep(1);
	} while ((status & MMC328XMA_VAL_TM_READY) != mmc328xma_VAL_TM_READY);

	result = inv_serial_read(mlsl_handle, pdata->address,
					MMC328XMA_REG_X_LSB,
				6, (unsigned char *)data);
	if (result) {
		LOG_RESULT_LOCATION(result);
		return result;
	}
	{
		short tmp[3];
		unsigned char tmpdata[6];
		int ii;
		for (ii = 0; ii < 6; ii++)
			tmpdata[ii] = data[ii];

		for (ii = 0; ii < 3; ii++) {
			tmp[ii] = (short)((tmpdata[2 * ii + 1] << 8) +
							tmpdata[2 * ii]);
			tmp[ii] = tmp[ii] - 4096;
			tmp[ii] = tmp[ii] * 16;
		}

		for (ii = 0; ii < 3; ii++) {
			data[2 * ii] = (unsigned char)(tmp[ii] >> 8);
			data[2 * ii + 1] = (unsigned char)(tmp[ii]);
		}
	}
	read_count++;

	if (read_count % reset_int == 0) {
		/* 1st magnetization */
		result = inv_serial_single_write(mlsl_handle,
					pdata->address,
					MMC328XMA_REG_CTL0,
					MMC328XMA_VAL_RM_MAG);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}
		msleep(50);
		/* 2nd magnetization */
		result = inv_serial_single_write(mlsl_handle,
					pdata->address,
					MMC328XMA_REG_CTL0,
					MMC328XMA_VAL_RRM_MAG);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}
		msleep(100);
	}

	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_CNTL_MODE_WAKE_UP);
	if (result) {
		LOG_RESULT_LOCATION(result);
		return result;
	}

	return INV_SUCCESS;
}

struct ext_slave_descr mmc328xma_descr = {
	/*.init             = */ NULL,
	/*.exit             = */ NULL,
	/*.suspend          = */ mmc328xma_suspend,
	/*.resume           = */ mmc328xma_resume,
	/*.read             = */ mmc328xma_read,
	/*.config           = */ NULL,
	/*.get_config       = */ NULL,
	/*.name             = */ "mmc328xma",
	/*.type             = */ EXT_SLAVE_TYPE_COMPASS,
	/*.id               = */ COMPASS_ID_MMC328XMA,
	/*.reg              = */ 0x01,
	/*.len              = */ 6,
	/*.endian           = */ EXT_SLAVE_BIG_ENDIAN,
	/*.range            = */ {400, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *mmc328xma_get_slave_descr(void)
{
	return &mmc328xma_descr;
}
EXPORT_SYMBOL(mmc328xma_get_slave_descr);

/**
 *  @}
 */
