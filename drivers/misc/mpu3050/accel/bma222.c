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
 *  @addtogroup ACCELDL
 *
 *  @{
 *      @file   bma222.c
 *      @brief  Accelerometer setup and handling methods for Bosch BMA222.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include "mpu.h"
#include "mlos.h"
#include "mlsl.h"

/* ------------------ */
/* - Defines.       - */
/* ------------------ */

#define BMA222_STATUS_REG	(0x0A)
#define BMA222_FSR_REG		(0x0F)
#define ADXL34X_ODR_REG		(0x10)
#define BMA222_PWR_REG		(0x11)
#define BMA222_SOFTRESET_REG	(0x14)

#define BMA222_STATUS_RDY_MASK	(0x80)
#define BMA222_FSR_MASK		(0x0F)
#define BMA222_ODR_MASK		(0x1F)
#define BMA222_PWR_SLEEP_MASK	(0x80)
#define BMA222_PWR_AWAKE_MASK	(0x00)
#define BMA222_SOFTRESET_MASK	(0xB6)
#define BMA222_SOFTRESET_MASK	(0xB6)

/* -------------------- */
/* - Data Structures. - */
/* -------------------- */

struct bma222_config {
	unsigned int odr;		/** < output data rate in mHz */
	unsigned int fsr;		/** < full scale range mg */
};

struct bma222_private_data {
	struct bma222_config suspend;	/** < suspend configuration */
	struct bma222_config resume;	/** < resume configuration */
};


/*********************************************
    Accelerometer Initialization Functions
**********************************************/

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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_set_odr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma222_config *config,
			  int apply,
			  long odr)
{
	int result = INV_SUCCESS;
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
	} else if (odr >= 32000) {
		reg_odr = 0x0A;
		config->odr = 32000;
	} else if (odr >= 16000) {
		reg_odr = 0x09;
		config->odr = 16000;
	} else {
		reg_odr = 0x08;
		config->odr = 8000;
	}

	if (apply) {
		MPL_LOGV("ODR: %d\n", config->odr);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ADXL34X_ODR_REG, reg_odr);
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_set_fsr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma222_config *config,
			  int apply,
			  long fsr)
{
	int result = INV_SUCCESS;
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
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA222_FSR_REG, reg_fsr_mask);
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_init(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	inv_error_t result;

	struct bma222_private_data *private_data;
	private_data = (struct bma222_private_data *)
		inv_malloc(sizeof(struct bma222_private_data));

	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA222_SOFTRESET_REG, BMA222_SOFTRESET_MASK);
	ERROR_CHECK(result);
	inv_sleep(1);

	result = bma222_set_odr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, 0);
	ERROR_CHECK(result);
	result = bma222_set_odr(mlsl_handle, pdata, &private_data->resume,
				FALSE, 200000);
	ERROR_CHECK(result);

	result = bma222_set_fsr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, 2000);
	result = bma222_set_fsr(mlsl_handle, pdata, &private_data->resume,
				FALSE, 2000);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA222_PWR_REG, BMA222_PWR_SLEEP_MASK);
	ERROR_CHECK(result);

	return result;
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_exit(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	if (pdata->private_data)
		return inv_free(pdata->private_data);
	else
		return INV_SUCCESS;
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_get_config(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata,
				struct ext_slave_config *data)
{
	struct bma222_private_data *private_data =
			(struct bma222_private_data *)(pdata->private_data);

	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

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
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
	default:
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return INV_SUCCESS;
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_config(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata,
			 struct ext_slave_config *data)
{
	struct bma222_private_data *private_data =
			(struct bma222_private_data *)(pdata->private_data);

	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return bma222_set_odr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return bma222_set_odr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return bma222_set_fsr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return bma222_set_fsr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
	default:
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	};
	return INV_SUCCESS;
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_suspend(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	int result;
	struct bma222_config *suspend_config =
		&((struct bma222_private_data *)pdata->private_data)->suspend;

	result = bma222_set_odr(mlsl_handle, pdata, suspend_config,
				TRUE, suspend_config->odr);
	ERROR_CHECK(result);
	result = bma222_set_fsr(mlsl_handle, pdata, suspend_config,
				TRUE, suspend_config->fsr);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA222_PWR_REG, BMA222_PWR_SLEEP_MASK);
	ERROR_CHECK(result);

	inv_sleep(3); /* 3 ms powerup time maximum */
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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_resume(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	int result;
	unsigned char reg = 0;
	struct bma222_config *resume_config =
		&((struct bma222_private_data *)pdata->private_data)->resume;

	/* Soft reset */
	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA222_SOFTRESET_REG, BMA222_SOFTRESET_MASK);
	ERROR_CHECK(result);
	inv_sleep(10);

	result = bma222_set_odr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->odr);
	ERROR_CHECK(result);
	result = bma222_set_fsr(mlsl_handle, pdata, resume_config,
				TRUE, resume_config->fsr);
	ERROR_CHECK(result);

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
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma222_read(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata,
		       unsigned char *data)
{
	int result = INV_SUCCESS;
	result = inv_serial_read(mlsl_handle, pdata->address,
				BMA222_STATUS_REG, 1, data);
	if (data[0] & BMA222_STATUS_RDY_MASK) {
		result = inv_serial_read(mlsl_handle, pdata->address,
				 slave->read_reg, slave->read_len, data);
	return result;
	} else
		return INV_ERROR_ACCEL_DATA_NOT_READY;
}

static struct ext_slave_descr bma222_descr = {
	/*.init             = */ bma222_init,
	/*.exit             = */ bma222_exit,
	/*.suspend          = */ bma222_suspend,
	/*.resume           = */ bma222_resume,
	/*.read             = */ bma222_read,
	/*.config           = */ bma222_config,
	/*.get_config       = */ bma222_get_config,
	/*.name             = */ "bma222",
	/*.type             = */ EXT_SLAVE_TYPE_ACCELEROMETER,
	/*.id               = */ ACCEL_ID_BMA222,
	/*.reg              = */ 0x02,
	/*.len              = */ 6,
	/*.endian           = */ EXT_SLAVE_LITTLE_ENDIAN,
	/*.range            = */ {2, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *bma222_get_slave_descr(void)
{
	return &bma222_descr;
}
EXPORT_SYMBOL(bma222_get_slave_descr);

/**
 *  @}
 */
