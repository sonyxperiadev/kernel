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
 *      @file   mma8450.c
 *      @brief  Accelerometer setup and handling methods for Freescale MMA8450.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#ifdef __KERNEL__
#include <linux/module.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "mpu.h"
#include "mlsl.h"
#include "mlos.h"

#include <log.h>
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-acc"

/* full scale setting - register & mask */
#define ACCEL_MMA8450_XYZ_DATA_CFG	(0x16)

#define ACCEL_MMA8450_CTRL_REG1		(0x38)
#define ACCEL_MMA8450_CTRL_REG4		(0x3B)
#define ACCEL_MMA8450_CTRL_REG5		(0x3C)

#define ACCEL_MMA8450_CTRL_REG		(0x38)
#define ACCEL_MMA8450_CTRL_MASK		(0x03)

#define ACCEL_MMA8450_SLEEP_MASK	(0x03)

/* --------------------- */
/* - Data structures   - */
/* --------------------- */

struct mma8450_config {
	unsigned int odr;
	unsigned int fsr;	/** < full scale range mg */
	unsigned int ths;	/** < Motion no-motion thseshold mg */
	unsigned int dur;	/** < Motion no-motion duration ms */
	unsigned char reg_ths;
	unsigned char reg_dur;
	unsigned char ctrl_reg1;
	unsigned char irq_type;
	unsigned char mot_int1_cfg;
};

struct mma8450_private_data {
	struct mma8450_config suspend;
	struct mma8450_config resume;
};


/*********************************************
    Accelerometer Initialization Functions
**********************************************/

static int mma8450_set_ths(void *mlsl_handle,
			struct ext_slave_platform_data *pdata,
			struct mma8450_config *config,
			int apply,
			long ths)
{
	return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
}

static int mma8450_set_dur(void *mlsl_handle,
			struct ext_slave_platform_data *pdata,
			struct mma8450_config *config,
			int apply,
			long dur)
{
	return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
}

/**
 *  @brief Sets the IRQ to fire when one of the IRQ events occur.
 *         Threshold and duration will not be used unless the type is MOT or
 *         NMOT.
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
 *
 *  @return INV_SUCCESS if successful or a non-zero error code.
 */
static int mma8450_set_irq(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma8450_config *config,
		int apply,
		long irq_type)
{
	int result = INV_SUCCESS;
	unsigned char reg1;
	unsigned char reg2;
	unsigned char reg3;

	config->irq_type = (unsigned char)irq_type;
	if (irq_type == MPU_SLAVE_IRQ_TYPE_DATA_READY) {
		reg1 = 0x01;
		reg2 = 0x01;
		reg3 = 0x07;
	} else if (irq_type == MPU_SLAVE_IRQ_TYPE_NONE) {
		reg1 = 0x00;
		reg2 = 0x00;
		reg3 = 0x00;
	} else {
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	}

	if (apply) {
		/* XYZ_DATA_CFG: event flag enabled on Z axis */
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA8450_XYZ_DATA_CFG, reg3);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA8450_CTRL_REG4, reg1);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA8450_CTRL_REG5, reg2);
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
static int mma8450_set_odr(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma8450_config *config,
		int apply,
		long odr)
{
	unsigned char bits;
	int result = INV_SUCCESS;

	if (odr > 200000) {
		config->odr = 400000;
		bits = 0x00;
	} else if (odr > 100000) {
		config->odr = 200000;
		bits = 0x04;
	} else if (odr > 50000) {
		config->odr = 100000;
		bits = 0x08;
	} else if (odr > 25000) {
		config->odr = 50000;
		bits = 0x0B;
	} else if (odr > 12500) {
		config->odr = 25000;
		bits = 0x40; /* Sleep -> Auto wake mode */
	} else if (odr > 1563) {
		config->odr = 12500;
		bits = 0x10;
	} else if (odr > 0) {
		config->odr = 1563;
		bits = 0x14;
	} else {
		config->ctrl_reg1 = 0; /* Set FS1.FS2 to Standby */
		config->odr = 0;
		bits = 0;
	}

	config->ctrl_reg1 = bits | (config->ctrl_reg1 & 0x3);
	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA8450_CTRL_REG1, 0);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA8450_CTRL_REG1, config->ctrl_reg1);
		ERROR_CHECK(result);
		MPL_LOGV("ODR: %d mHz, 0x%02x\n",
			config->odr, (int)config->ctrl_reg1);
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
static int mma8450_set_fsr(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma8450_config *config,
		int apply,
		long fsr)
{
	unsigned char bits;
	int result = INV_SUCCESS;

	if (fsr <= 2000) {
		bits = 0x01;
		config->fsr = 2000;
	} else if (fsr <= 4000) {
		bits = 0x02;
		config->fsr = 4000;
	} else {
		bits = 0x03;
		config->fsr = 8000;
	}

	config->ctrl_reg1 = bits | (config->ctrl_reg1 & 0xFC);
	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA8450_CTRL_REG1, config->ctrl_reg1);
		ERROR_CHECK(result);
		MPL_LOGV("FSR: %d mg\n", config->fsr);
	}
	return result;
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
static int mma8450_suspend(void *mlsl_handle,
		    struct ext_slave_descr *slave,
		    struct ext_slave_platform_data *pdata)
{
	int result;
	struct mma8450_private_data *private_data = pdata->private_data;

	if (private_data->suspend.fsr == 4000)
		slave->range.mantissa = 4;
	else if (private_data->suspend.fsr == 8000)
		slave->range.mantissa = 8;
	else
		slave->range.mantissa = 2;
	slave->range.fraction = 0;

	result = inv_serial_single_write(mlsl_handle, pdata->address,
			ACCEL_MMA8450_CTRL_REG1, 0);
	ERROR_CHECK(result);
	if (private_data->resume.ctrl_reg1) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA8450_CTRL_REG1,
				private_data->suspend.ctrl_reg1);
	ERROR_CHECK(result);
	}

	result = mma8450_set_irq(mlsl_handle, pdata,
				&private_data->suspend,
				TRUE, private_data->suspend.irq_type);
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
static int mma8450_resume(void *mlsl_handle,
		   struct ext_slave_descr *slave,
		   struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;
	struct mma8450_private_data *private_data = pdata->private_data;

	/* Full Scale */
	if (private_data->resume.fsr == 4000)
		slave->range.mantissa = 4;
	else if (private_data->resume.fsr == 8000)
		slave->range.mantissa = 8;
	else
		slave->range.mantissa = 2;
	slave->range.fraction = 0;

	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
			ACCEL_MMA8450_CTRL_REG1, 0);
	ERROR_CHECK(result);
	if (private_data->resume.ctrl_reg1) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA8450_CTRL_REG1,
				private_data->resume.ctrl_reg1);
		ERROR_CHECK(result);
	}
	result = mma8450_set_irq(mlsl_handle, pdata,
			&private_data->resume,
			TRUE, private_data->resume.irq_type);
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
static int mma8450_read(void *mlsl_handle,
		 struct ext_slave_descr *slave,
		 struct ext_slave_platform_data *pdata, unsigned char *data)
{
	int result;
	unsigned char local_data[4];	/* Status register + 3 bytes data */
	result = inv_serial_read(mlsl_handle, pdata->address,
				0x00,
				 sizeof(local_data), local_data);
	ERROR_CHECK(result);
	memcpy(data, &local_data[1], (slave->read_len) - 1);
		MPL_LOGV("Data Not Ready: %02x %02x %02x %02x\n",
			local_data[0],
			local_data[1],
			local_data[2],
			local_data[3]);
	if (!(local_data[0] & 0x04))
		result = INV_ERROR_ACCEL_DATA_NOT_READY;
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
static int mma8450_init(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	struct mma8450_private_data *private_data;
	private_data = (struct mma8450_private_data *)
		inv_malloc(sizeof(struct mma8450_private_data));

	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	mma8450_set_odr(mlsl_handle, pdata, &private_data->suspend,
			FALSE, 0);
	mma8450_set_odr(mlsl_handle, pdata, &private_data->resume,
			FALSE, 200000);
	mma8450_set_fsr(mlsl_handle, pdata, &private_data->suspend,
			FALSE, 2000);
	mma8450_set_fsr(mlsl_handle, pdata, &private_data->resume,
			FALSE, 2000);
	mma8450_set_irq(mlsl_handle, pdata, &private_data->suspend,
			FALSE,
			MPU_SLAVE_IRQ_TYPE_NONE);
	mma8450_set_irq(mlsl_handle, pdata, &private_data->resume,
			FALSE,
			MPU_SLAVE_IRQ_TYPE_NONE);
	return INV_SUCCESS;
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
static int mma8450_exit(void *mlsl_handle,
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
static int mma8450_config(void *mlsl_handle,
			struct ext_slave_descr *slave,
			struct ext_slave_platform_data *pdata,
			struct ext_slave_config *data)
{
	struct mma8450_private_data *private_data = pdata->private_data;
	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return mma8450_set_odr(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return mma8450_set_odr(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return mma8450_set_fsr(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return mma8450_set_fsr(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_THS:
		return mma8450_set_ths(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_THS:
		return mma8450_set_ths(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_DUR:
		return mma8450_set_dur(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_DUR:
		return mma8450_set_dur(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
		return mma8450_set_irq(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		return mma8450_set_irq(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	default:
		ERROR_CHECK(INV_ERROR_FEATURE_NOT_IMPLEMENTED);
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
static int mma8450_get_config(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata,
				struct ext_slave_config *data)
{
	struct mma8450_private_data *private_data = pdata->private_data;
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
	case MPU_SLAVE_CONFIG_MOT_THS:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->suspend.ths;
		break;
	case MPU_SLAVE_CONFIG_NMOT_THS:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->resume.ths;
		break;
	case MPU_SLAVE_CONFIG_MOT_DUR:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->suspend.dur;
		break;
	case MPU_SLAVE_CONFIG_NMOT_DUR:
		(*(unsigned long *)data->data) =
			(unsigned long) private_data->resume.dur;
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
		ERROR_CHECK(INV_ERROR_FEATURE_NOT_IMPLEMENTED);
	};

	return INV_SUCCESS;
}

struct ext_slave_descr mma8450_descr = {
	/*.init             = */ mma8450_init,
	/*.exit             = */ mma8450_exit,
	/*.suspend          = */ mma8450_suspend,
	/*.resume           = */ mma8450_resume,
	/*.read             = */ mma8450_read,
	/*.config           = */ mma8450_config,
	/*.get_config       = */ mma8450_get_config,
	/*.name             = */ "mma8450",
	/*.type             = */ EXT_SLAVE_TYPE_ACCELEROMETER,
	/*.id               = */ ACCEL_ID_MMA8450,
	/*.reg              = */ 0x00,
	/*.len              = */ 4,
	/*.endian           = */ EXT_SLAVE_FS8_BIG_ENDIAN,
	/*.range            = */ {2, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *mma8450_get_slave_descr(void)
{
	return &mma8450_descr;
}
EXPORT_SYMBOL(mma8450_get_slave_descr);

/**
 *  @}
 */
