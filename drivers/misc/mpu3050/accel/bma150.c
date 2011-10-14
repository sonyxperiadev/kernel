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
 *      @file   bma150.c
 *      @brief  Accelerometer setup and handling methods for Bosch BMA150.
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

/* ------------------- */
/* -    Defines.     - */
/* ------------------- */

/* registers */
#define BMA150_CTRL_REG			(0x14)
#define BMA150_INT_REG			(0x15)
#define BMA150_PWR_REG			(0x0A)

/* masks */
#define BMA150_CTRL_MASK		(0x18)
#define BMA150_CTRL_MASK_ODR		(0xF8)
#define BMA150_CTRL_MASK_FSR		(0xE7)
#define BMA150_INT_MASK_WUP		(0xF8)
#define BMA150_INT_MASK_IRQ		(0xDF)
#define BMA150_PWR_MASK_SLEEP		(0x01)
#define BMA150_PWR_MASK_SOFT_RESET	(0x02)

/* --------------------------- */
/* -    Data Structures.     - */
/* --------------------------- */

struct bma150_config {
	unsigned int odr;	/** < output data rate mHz */
	unsigned int fsr;	/** < full scale range mgees */
	unsigned int irq_type;	/** < type of IRQ, see bma150_set_irq */
	unsigned char ctrl_reg;	/** < control register value */
	unsigned char int_reg;	/** < interrupt control register value */
};

struct bma150_private_data {
	struct bma150_config suspend;	/** < suspend configuration */
	struct bma150_config resume;	/** < resume configuration */
};

/*********************************************
    Accelerometer Initialization Functions
**********************************************/

/**
 *  @brief Simply disables the IRQ since it is not usable on BMA150 devices.
 *
 *  @param mlsl_handle
 *             the handle to the serial channel the device is connected to.
 *  @param pdata
 *             a pointer to the slave platform data.
 *  @param config
 *              configuration to apply to, suspend or resume
 *  @param apply
 *             whether to apply immediately or save the settings to be applied
 *             at the next resume.
 *  @param irq_type
 *              the type of IRQ.  Valid values are
 *              - MPU_SLAVE_IRQ_TYPE_NONE
 *              - MPU_SLAVE_IRQ_TYPE_MOTION
 *              - MPU_SLAVE_IRQ_TYPE_DATA_READY
 *              The only supported IRQ type is MPU_SLAVE_IRQ_TYPE_NONE which
 *              corresponds to disabling the IRQ completely.
 *
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int bma150_set_irq(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma150_config *config,
			  int apply,
			  long irq_type)
{
	unsigned char irq_bits = 0;
	int result = INV_SUCCESS;

	if (irq_type != MPU_SLAVE_IRQ_TYPE_NONE)
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;

	config->irq_type = MPU_SLAVE_IRQ_TYPE_NONE;
	config->int_reg = 0x00;

	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
			 BMA150_CTRL_REG, config->ctrl_reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
			 BMA150_INT_REG, config->int_reg);
		ERROR_CHECK(result);
	}
	return result;
}

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
static int bma150_set_odr(void *mlsl_handle,
			struct ext_slave_platform_data *pdata,
			struct bma150_config *config,
			int apply,
			long odr)
{
	unsigned char odr_bits = 0;
	unsigned char wup_bits = 0;
	int result = INV_SUCCESS;

	if (odr > 100000) {
		config->odr = 190000;
		odr_bits = 0x03;
	} else if (odr > 50000) {
		config->odr = 100000;
		odr_bits = 0x02;
	} else if (odr > 25000) {
		config->odr = 50000;
		odr_bits = 0x01;
	} else if (odr > 0) {
		config->odr = 25000;
		odr_bits = 0x00;
	} else {
		config->odr = 0;
		wup_bits = 0x00;
	}

	config->int_reg &= BMA150_INT_MASK_WUP;
	config->ctrl_reg &= BMA150_CTRL_MASK_ODR;
	config->ctrl_reg |= odr_bits;

	MPL_LOGV("ODR: %d \n", config->odr);
	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					BMA150_CTRL_REG, config->ctrl_reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					BMA150_INT_REG, config->int_reg);
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
static int bma150_set_fsr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct bma150_config *config,
			  int apply,
			  long fsr)
{
	unsigned char fsr_bits;
	int result = INV_SUCCESS;

	if (fsr <= 2048) {
		fsr_bits = 0x00;
		config->fsr = 2048;
	} else if (fsr <= 4096) {
		fsr_bits = 0x08;
		config->fsr = 4096;
	} else {
		fsr_bits = 0x10;
		config->fsr = 8192;
	}

	config->ctrl_reg &= BMA150_CTRL_MASK_FSR;
	config->ctrl_reg |= fsr_bits;

	MPL_LOGV("FSR: %d \n", config->fsr);
	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_CTRL_REG, config->ctrl_reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_CTRL_REG, config->ctrl_reg);
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
static int bma150_init(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	inv_error_t result;
	unsigned char reg;
	long range;

	struct bma150_private_data *private_data;
	private_data = (struct bma150_private_data *)
		inv_malloc(sizeof(struct bma150_private_data));

	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA150_PWR_REG, BMA150_PWR_MASK_SOFT_RESET);
	ERROR_CHECK(result);
	inv_sleep(1);

	result = inv_serial_read(mlsl_handle, pdata->address,
				 BMA150_CTRL_REG, 1, &reg);
	ERROR_CHECK(result);
	private_data->resume.ctrl_reg = reg;
	private_data->suspend.ctrl_reg = reg;

	result = inv_serial_read(mlsl_handle, pdata->address,
				 BMA150_INT_REG, 1, &reg);
	ERROR_CHECK(result);
	private_data->resume.int_reg = reg;
	private_data->suspend.int_reg = reg;

	result = bma150_set_odr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, 0);
	ERROR_CHECK(result);
	result = bma150_set_odr(mlsl_handle, pdata, &private_data->resume,
				FALSE, 200000);
	ERROR_CHECK(result);

	range = RANGE_FIXEDPOINT_TO_LONG_MG(slave->range);
	result = bma150_set_fsr(mlsl_handle, pdata, &private_data->suspend,
				FALSE, range);
	result = bma150_set_fsr(mlsl_handle, pdata, &private_data->resume,
				FALSE, range);
	ERROR_CHECK(result);

	result = bma150_set_irq(mlsl_handle, pdata, &private_data->suspend,
				FALSE, MPU_SLAVE_IRQ_TYPE_NONE);
	ERROR_CHECK(result);
	result = bma150_set_irq(mlsl_handle, pdata, &private_data->resume,
				FALSE, MPU_SLAVE_IRQ_TYPE_NONE);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				       BMA150_PWR_REG, BMA150_PWR_MASK_SLEEP);
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
static int bma150_exit(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	if (pdata->private_data)
		return inv_free(pdata->private_data);
	else
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
static int bma150_config(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata,
			 struct ext_slave_config *data)
{
	struct bma150_private_data *private_data =
			(struct bma150_private_data *)(pdata->private_data);

	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return bma150_set_odr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return bma150_set_odr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return bma150_set_fsr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return bma150_set_fsr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
		return bma150_set_irq(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply,
				      *((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		return bma150_set_irq(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply,
				      *((long *)data->data));
	default:
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	};
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
static int bma150_get_config(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata,
				struct ext_slave_config *data)
{
	struct bma150_private_data *private_data =
			(struct bma150_private_data *)(pdata->private_data);

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
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->suspend.irq_type;
		break;
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->resume.irq_type;
		break;
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
static int bma150_suspend(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	int result;
	unsigned char ctrl_reg;
	unsigned char int_reg;

	struct bma150_private_data *private_data =
			(struct bma150_private_data *)(pdata->private_data);

	ctrl_reg = private_data->suspend.ctrl_reg;
	int_reg = private_data->suspend.int_reg;

	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA150_PWR_REG, BMA150_PWR_MASK_SOFT_RESET);
	ERROR_CHECK(result);
	inv_sleep(1);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA150_CTRL_REG, ctrl_reg);
	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
			BMA150_INT_REG, int_reg);
	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
		BMA150_PWR_REG, BMA150_PWR_MASK_SLEEP);
	ERROR_CHECK(result);

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
static int bma150_resume(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	int result;
	unsigned char ctrl_reg;
	unsigned char int_reg;

	struct bma150_private_data *private_data =
			(struct bma150_private_data *)(pdata->private_data);

	ctrl_reg = private_data->resume.ctrl_reg;
	int_reg = private_data->resume.int_reg;

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_PWR_REG, BMA150_PWR_MASK_SOFT_RESET);
	ERROR_CHECK(result);
	inv_sleep(1);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_CTRL_REG, ctrl_reg);
	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_INT_REG, int_reg);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
				BMA150_PWR_REG, 0x00);
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
static int bma150_read(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata,
		       unsigned char *data)
{
	return inv_serial_read(mlsl_handle, pdata->address,
			       slave->read_reg, slave->read_len, data);
}

static struct ext_slave_descr bma150_descr = {
	/*.init             = */ bma150_init,
	/*.exit             = */ bma150_exit,
	/*.suspend          = */ bma150_suspend,
	/*.resume           = */ bma150_resume,
	/*.read             = */ bma150_read,
	/*.config           = */ bma150_config,
	/*.get_config       = */ bma150_get_config,
	/*.name             = */ "bma150",
	/*.type             = */ EXT_SLAVE_TYPE_ACCELEROMETER,
	/*.id               = */ ACCEL_ID_BMA150,
	/*.reg              = */ 0x02,
	/*.len              = */ 6,
	/*.endian           = */ EXT_SLAVE_LITTLE_ENDIAN,
	/*.range            = */ {2, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *bma150_get_slave_descr(void)
{
	return &bma150_descr;
}
EXPORT_SYMBOL(bma150_get_slave_descr);

#ifdef __KERNEL__
MODULE_AUTHOR("Invensense");
MODULE_DESCRIPTION("User space IRQ handler for MPU3xxx devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS("bma");
#endif

/**
 *  @}
 */

