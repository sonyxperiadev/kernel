/*
 $License:
    Copyright (C) 2010 InvenSense Corporation, All Rights Reserved.

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
 *  @addtogroup ACCELDL
 *  @brief      Provides the interface to setup and handle an accelerometer.
 *
 *  @{
 *      @file   bma250.c
 *      @brief  Accelerometer setup and handling methods for Bosch BMA250.
 */

/* -------------------------------------------------------------------- */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <linux/mpu.h>
#include "mlos.h"
#include "mlsl.h"
#include "mldl_cfg.h"
#include <linux/bma250_ng_common.h>

/* -------------------------------------------------------------------- */

/* registers */
#define BMA250_STATUS_REG	(0x0A)
#define BMA250_FSR_REG		(0x0F)
#define BMA250_ODR_REG		(0x10)
#define BMA250_PWR_REG		(0x11)
#define BMA250_SOFTRESET_REG	(0x14)
#define BMA250_INT_TYPE_REG	(0x17)
#define BMA250_INT_DST_REG	(0x1A)
#define BMA250_INT_SRC_REG	(0x1E)

/* masks */
#define BMA250_STATUS_RDY_MASK	(0x80)
#define BMA250_FSR_MASK		(0x0F)
#define BMA250_ODR_MASK		(0x1F)
#define BMA250_PWR_SLEEP_MASK	(0x80)
#define BMA250_PWR_AWAKE_MASK	(0x00)
#define BMA250_SOFTRESET_MASK   (0xB6)
#define BMA250_INT_TYPE_MASK	(0x10)
#define BMA250_INT_DST_1_MASK	(0x01)
#define BMA250_INT_DST_2_MASK	(0x80)
#define BMA250_INT_SRC_MASK	(0x00)

/* -------------------------------------------------------------------- */

struct bma250_config {
	unsigned int odr;		/** < output data rate in mHz */
	unsigned int fsr;		/** < full scale range mg */
};

struct bma250_private_data {
	struct bma250_config suspend;	/** < suspend configuration */
	struct bma250_config resume;	/** < resume configuration */
};

/**
 *  @brief Set the output data rate for the particular configuration.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param config
 *             Config to modify with new ODR.
 *  @param apply
 *             whether to apply immediately or save the settings to be applied
 *             at the next resume.
 *  @param odr
 *             Output data rate in units of 1/1000Hz (mHz).
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_set_odr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma250_config *config,
			  int apply,
			  long odr)
{
	int result = ML_SUCCESS;
	unsigned char reg_odr;

	if (odr >= 1000000) {
		reg_odr = 0x0F;
		config->odr = 1000000;
	} else if (odr >= 500000) {
		reg_odr = 0x0E;
		config->odr = 500000;
	} else if (odr >= 250000) {
		reg_odr = 0x0D;
		config->odr = 250000;
	} else if (odr >= 125000) {
		reg_odr = 0x0C;
		config->odr = 125000;
	} else if (odr >= 62500) {
		reg_odr = 0x0B;
		config->odr = 62500;
	} else if (odr >= 31250) {
		reg_odr = 0x0A;
		config->odr = 31250;
	} else if (odr >= 15630) {
		reg_odr = 0x09;
		config->odr = 15630;
	} else {
		reg_odr = 0x08;
		config->odr = 7810;
	}

	if (apply) {
		MPL_LOGV("ODR: %d\n", config->odr);
		result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				BMA250_ODR_REG, reg_odr);
		ERROR_CHECK(result);
	}

	return result;
}

/**
 *  @brief Set the full scale range of the accels
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param config
 *             pointer to configuration.
 *  @param apply
 *             whether to apply immediately or save the settings to be applied
 *             at the next resume.
 *  @param fsr
 *             requested full scale range.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_set_fsr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma250_config *config,
			  int apply,
			  long fsr)
{
	int result = ML_SUCCESS;
	unsigned char reg_fsr_mask;

	if (fsr <= 2000) {
		reg_fsr_mask = 0x03;
		config->fsr = 2000;
	} else if (fsr <= 4000) {
		reg_fsr_mask = 0x05;
		config->fsr = 4000;
	} else if (fsr <= 8000) {
		reg_fsr_mask = 0x08;
		config->fsr = 8000;
	} else { /* 8001 -> oo */
		reg_fsr_mask = 0x0C;
		config->fsr = 16000;
	}

	if (apply) {
		MPL_LOGV("FSR: %d\n", config->fsr);
		result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				BMA250_FSR_REG, reg_fsr_mask);
		ERROR_CHECK(result);
	}
	return result;
}

/**
 *  @brief one-time device driver initialization function.
 *         If the driver is built as a kernel module, this function will be
 *         called when the module is loaded in the kernel.
 *         If the driver is built-in in the kernel, this function will be
 *         called at boot time.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_init(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	struct bma250_private_data *private_data;

	if ((!pdata->check_sleep_status && pdata->vote_sleep_status) ||
		(pdata->check_sleep_status && !pdata->vote_sleep_status))
		return ML_ERROR_INVALID_PARAMETER;

	private_data = MLOSMalloc(sizeof(struct bma250_private_data));
	if (!private_data)
		return ML_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	(void)bma250_set_odr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, 0);
	(void)bma250_set_odr(mlsl_handle, pdata, &private_data->resume,
				FALSE, 200000);
	(void)bma250_set_fsr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, 2000L);
	(void)bma250_set_fsr(mlsl_handle, pdata, &private_data->resume,
				FALSE, 2000L);

	return ML_SUCCESS;
}

/**
 *  @brief one-time device driver exit function.
 *         If the driver is built as a kernel module, this function will be
 *         called when the module is removed from the kernel.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_exit(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	MLOSFree(pdata->private_data);
	return ML_SUCCESS;
}

/**
 *  @brief device configuration facility.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param data
 *             a pointer to the configuration data structure.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_config(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata,
			 struct ext_slave_config *data)
{
	struct bma250_private_data *private_data =
			(struct bma250_private_data *)(pdata->private_data);

	if (!data->data)
		return ML_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return bma250_set_odr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return bma250_set_odr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return bma250_set_fsr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return bma250_set_fsr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	default:
		return ML_ERROR_FEATURE_NOT_IMPLEMENTED;
	}
	return ML_SUCCESS;
}

/**
 *  @brief facility to retrieve the device configuration.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param data
 *             a pointer to store the returned configuration data structure.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_get_config(void *mlsl_handle,
			     struct ext_slave_descr *slave,
			     struct ext_slave_platform_data *pdata,
			     struct ext_slave_config *data)
{
	struct bma250_private_data *private_data =
			(struct bma250_private_data *)(pdata->private_data);

	if (!data->data)
		return ML_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->suspend.odr;
		break;
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->resume.odr;
		break;
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->suspend.fsr;
		break;
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->resume.fsr;
		break;
	default:
		return ML_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return ML_SUCCESS;
}

/**
 *  @brief suspends the device to put it in its lowest power mode.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_suspend(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	int result = 0;

	if (pdata->check_sleep_status && pdata->vote_sleep_status) {
		mutex_lock(&bma250_power_lock);
		pdata->vote_sleep_status(BMA250_SLAVE2, BMA250_SLEEP);
		if (pdata->check_sleep_status() == BMA250_SLEEP) {
			result = MLSLSerialWriteSingle(mlsl_handle,
					pdata->address, BMA250_PWR_REG,
					BMA250_PWR_SLEEP_MASK);
			ERROR_CHECK_MUTEX(result, bma250_power_lock);
			MLOSSleep(3); /* 3 ms powerup time maximum */
		}
		mutex_unlock(&bma250_power_lock);
	} else {
		result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				BMA250_PWR_REG, BMA250_PWR_SLEEP_MASK);
		ERROR_CHECK(result);
		MLOSSleep(3); /* 3 ms powerup time maximum */
	}
	return result;
}

/**
 *  @brief resume the device in the proper power state given the configuration
 *         chosen.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_resume(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	int result;

	struct bma250_config *resume_config =
		&((struct bma250_private_data *)pdata->private_data)->resume;

	if (pdata->check_sleep_status && pdata->vote_sleep_status) {
		mutex_lock(&bma250_power_lock);
		if (pdata->check_sleep_status() == BMA250_SLEEP) {
			result = MLSLSerialWriteSingle(mlsl_handle,
					pdata->address, BMA250_SOFTRESET_REG,
					BMA250_SOFTRESET_MASK);
			ERROR_CHECK_MUTEX(result, bma250_power_lock);
			/* 4 ms needed for to be stable after reset. */
			MLOSSleep(4);
		}

		result = bma250_set_odr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->odr);
		ERROR_CHECK_MUTEX(result, bma250_power_lock);

		result = bma250_set_fsr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->fsr);
		ERROR_CHECK_MUTEX(result, bma250_power_lock);

		if (pdata->check_sleep_status() == BMA250_SLEEP) {
			result = MLSLSerialWriteSingle(mlsl_handle,
					pdata->address, BMA250_PWR_REG,
					BMA250_PWR_AWAKE_MASK);
			ERROR_CHECK_MUTEX(result, bma250_power_lock);
		}
		pdata->vote_sleep_status(BMA250_SLAVE2, BMA250_AWAKE);
		mutex_unlock(&bma250_power_lock);
	} else {
		result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				BMA250_SOFTRESET_REG, BMA250_SOFTRESET_MASK);
		ERROR_CHECK(result);
		/* 4 ms needed for to be stable after reset. */
		MLOSSleep(4);

		result = bma250_set_odr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->odr);
		ERROR_CHECK(result);

		result = bma250_set_fsr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->fsr);
		ERROR_CHECK(result);

		result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				BMA250_PWR_REG, BMA250_PWR_AWAKE_MASK);
		ERROR_CHECK(result);
	}
	return result;
}

/**
 *  @brief read the sensor data from the device.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param slave
 *             a pointer to the slave descriptor data structure.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param data
 *             a buffer to store the data read.
 *
 *  @return ML_SUCCESS if successful or a non-zero error code.
 */
static int bma250_read(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata,
		       unsigned char *data)
{
	int result = ML_SUCCESS;
	result = MLSLSerialRead(mlsl_handle, pdata->address,
				BMA250_STATUS_REG, 1, data);
	if (1) { /* KLP - workaroud for small data ready window */
		result = MLSLSerialRead(mlsl_handle, pdata->address,
				slave->reg, slave->len, data);
		return result;
	} else
		return ML_ERROR_ACCEL_DATA_NOT_READY;
}

static struct ext_slave_descr bma250_descr = {
	.init             = bma250_init,
	.exit             = bma250_exit,
	.suspend          = bma250_suspend,
	.resume           = bma250_resume,
	.read             = bma250_read,
	.config           = bma250_config,
	.get_config       = bma250_get_config,
	.name             = "bma250",
	.type             = EXT_SLAVE_TYPE_ACCELEROMETER,
	.id               = ACCEL_ID_BMA250,
	.reg              = 0x02,
	.len              = 6,
	.endian           = EXT_SLAVE_LITTLE_ENDIAN,
	.range            = {2, 0},
};

struct ext_slave_descr *bma250_get_slave_descr(void)
{
	return &bma250_descr;
}
EXPORT_SYMBOL(bma250_get_slave_descr);
