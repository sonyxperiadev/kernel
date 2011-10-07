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
 *      @file   mma845x.c
 *      @brief  Accelerometer setup and handling methods for Freescale MMA845X
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

#define ACCEL_MMA845X_XYZ_DATA_CFG	(0x0E)
#define ACCEL_MMA845X_CTRL_REG1      (0x2A)
#define ACCEL_MMA845X_CTRL_REG4		(0x2D)
#define ACCEL_MMA845X_CTRL_REG5		(0x2E)

#define ACCEL_MMA845X_SLEEP_MASK     (0x01)

/* full scale setting - register & mask */
#define ACCEL_MMA845X_CFG_REG       (0x0E)
#define ACCEL_MMA845X_CTRL_MASK     (0x03)

/* ----------------------- */
/* -  Data structures.   - */
/* ----------------------- */

struct mma845x_config {
	unsigned int odr;
	unsigned int fsr;		/** < full scale range mg */
	unsigned int ths;		/** < Motion no-motion thseshold mg */
	unsigned int dur;		/** < Motion no-motion duration ms */
	unsigned char reg_ths;
	unsigned char reg_dur;
	unsigned char ctrl_reg1;
	unsigned char irq_type;
	unsigned char mot_int1_cfg;
};

struct mma845x_private_data {
	struct mma845x_config suspend;
	struct mma845x_config resume;
};

/*****************************************
    Accelerometer Initialization Functions
*****************************************/

static int mma845x_set_ths(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma845x_config *config,
		int apply,
		long ths)
{
	return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
}

static int mma845x_set_dur(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma845x_config *config,
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
static int mma845x_set_irq(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma845x_config *config,
		int apply,
		long irq_type)
{
	int result = INV_SUCCESS;
	unsigned char reg1;
	unsigned char reg2;

	config->irq_type = (unsigned char)irq_type;
	if (irq_type == MPU_SLAVE_IRQ_TYPE_DATA_READY) {
		reg1 = 0x01;
		reg2 = 0x01;
	} else if (irq_type == MPU_SLAVE_IRQ_TYPE_NONE) {
		reg1 = 0x00;
		reg2 = 0x00;
	} else {
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	}

	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA845X_CTRL_REG4, reg1);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA845X_CTRL_REG5, reg2);
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
static int mma845x_set_odr(void *mlsl_handle,
			struct ext_slave_platform_data *pdata,
			struct mma845x_config *config,
			int apply,
			long odr)
{
	unsigned char bits;
	int result = INV_SUCCESS;

	if (odr > 400000) {
		config->odr = 800000;
		bits = 0x01;
	} else if (odr > 200000) {
		config->odr = 400000;
		bits = 0x09;
	} else if (odr > 100000) {
		config->odr = 200000;
		bits = 0x11;
	} else if (odr > 50000) {
		config->odr = 100000;
		bits = 0x19;
	} else if (odr > 12500) {
		config->odr = 50000;
		bits = 0x21;
	} else if (odr > 6250) {
		config->odr = 12500;
		bits = 0x29;
	} else if (odr > 1560) {
		config->odr = 6250;
		bits = 0x31;
	} else if (odr > 0) {
		config->odr = 1560;
		bits = 0x39;
	} else {
		config->ctrl_reg1 = 0; /* Set FS1.FS2 to Standby */
		config->odr = 0;
		bits = 0;
	}

	config->ctrl_reg1 = bits;
	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA845X_CTRL_REG1,
				config->ctrl_reg1);
		ERROR_CHECK(result);
		MPL_LOGV("ODR: %d mHz, 0x%02x\n",
			 config->odr,
			 (int)config->ctrl_reg1);
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
static int mma845x_set_fsr(void *mlsl_handle,
		struct ext_slave_platform_data *pdata,
		struct mma845x_config *config,
		int apply,
		long fsr)
{
	unsigned char bits;
	int result = INV_SUCCESS;

	if (fsr <= 2000) {
		bits = 0x00;
		config->fsr = 2000;
	} else if (fsr <= 4000) {
		bits = 0x01;
		config->fsr = 4000;
	} else {
		bits = 0x02;
		config->fsr = 8000;
	}

	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				ACCEL_MMA845X_XYZ_DATA_CFG,
				bits);
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
static int mma845x_suspend(void *mlsl_handle,
		    struct ext_slave_descr *slave,
		    struct ext_slave_platform_data *pdata)
{
	int result;
	struct mma845x_private_data *private_data = pdata->private_data;

	/* Full Scale */
	if (private_data->suspend.fsr == 4000)
		slave->range.mantissa = 4;
	else if (private_data->suspend.fsr == 8000)
		slave->range.mantissa = 8;
	else
		slave->range.mantissa = 2;
	slave->range.fraction = 0;

	result = mma845x_set_fsr(mlsl_handle, pdata,
				&private_data->suspend,
				TRUE, private_data->suspend.fsr);
	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					ACCEL_MMA845X_CTRL_REG1,
					private_data->suspend.ctrl_reg1);
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
static int mma845x_resume(void *mlsl_handle,
		   struct ext_slave_descr *slave,
		   struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;
	struct mma845x_private_data *private_data = pdata->private_data;

	/* Full Scale */
	if (private_data->resume.fsr == 4000)
		slave->range.mantissa = 4;
	else if (private_data->resume.fsr == 8000)
		slave->range.mantissa = 8;
	else
		slave->range.mantissa = 2;
	slave->range.fraction = 0;

	result = mma845x_set_fsr(mlsl_handle, pdata,
			&private_data->resume,
			TRUE, private_data->resume.fsr);
	ERROR_CHECK(result);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
			ACCEL_MMA845X_CTRL_REG1,
			private_data->resume.ctrl_reg1);
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
static int mma845x_read(void *mlsl_handle,
		 struct ext_slave_descr *slave,
		 struct ext_slave_platform_data *pdata, unsigned char *data)
{
	int result;
	unsigned char local_data[7];	/* Status register + 6 bytes data */
	result = inv_serial_read(mlsl_handle, pdata->address,
				 slave->read_reg, sizeof(local_data),
				 local_data);
	ERROR_CHECK(result);
	memcpy(data, &local_data[1], slave->read_len);
	return result;
}

static int mma845x_init(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	long range;
	struct mma845x_private_data *private_data;
	private_data = (struct mma845x_private_data *)
		inv_malloc(sizeof(struct mma845x_private_data));

	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	mma845x_set_odr(mlsl_handle, pdata, &private_data->suspend,
			FALSE, 0);
	mma845x_set_odr(mlsl_handle, pdata, &private_data->resume,
			FALSE, 200000);

	range = RANGE_FIXEDPOINT_TO_LONG_MG(slave->range);
	mma845x_set_fsr(mlsl_handle, pdata, &private_data->suspend,
			FALSE, range);
	mma845x_set_fsr(mlsl_handle, pdata, &private_data->resume,
			FALSE, range);

	mma845x_set_irq(mlsl_handle, pdata, &private_data->suspend,
			FALSE, MPU_SLAVE_IRQ_TYPE_NONE);
	mma845x_set_irq(mlsl_handle, pdata, &private_data->resume,
			FALSE, MPU_SLAVE_IRQ_TYPE_NONE);
	return INV_SUCCESS;
}

static int mma845x_exit(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	if (pdata->private_data)
		return inv_free(pdata->private_data);
	else
		return INV_SUCCESS;
}

static int mma845x_config(void *mlsl_handle,
			struct ext_slave_descr *slave,
			struct ext_slave_platform_data *pdata,
			struct ext_slave_config *data)
{
	struct mma845x_private_data *private_data = pdata->private_data;
	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return mma845x_set_odr(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return mma845x_set_odr(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return mma845x_set_fsr(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return mma845x_set_fsr(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_THS:
		return mma845x_set_ths(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_THS:
		return mma845x_set_ths(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_DUR:
		return mma845x_set_dur(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_DUR:
		return mma845x_set_dur(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
		return mma845x_set_irq(mlsl_handle, pdata,
					&private_data->suspend,
					data->apply,
					*((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		return mma845x_set_irq(mlsl_handle, pdata,
					&private_data->resume,
					data->apply,
					*((long *)data->data));
	default:
		ERROR_CHECK(INV_ERROR_FEATURE_NOT_IMPLEMENTED);
	};

	return INV_SUCCESS;
}

static int mma845x_get_config(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata,
				struct ext_slave_config *data)
{
	struct mma845x_private_data *private_data = pdata->private_data;
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

struct ext_slave_descr mma845x_descr = {
	/*.init             = */ mma845x_init,
	/*.exit             = */ mma845x_exit,
	/*.suspend          = */ mma845x_suspend,
	/*.resume           = */ mma845x_resume,
	/*.read             = */ mma845x_read,
	/*.config           = */ mma845x_config,
	/*.get_config       = */ mma845x_get_config,
	/*.name             = */ "mma845x",
	/*.type             = */ EXT_SLAVE_TYPE_ACCELEROMETER,
	/*.id               = */ ACCEL_ID_MMA845X,
	/*.reg              = */ 0x00,
	/*.len              = */ 6,
	/*.endian           = */ EXT_SLAVE_FS16_BIG_ENDIAN,
	/*.range            = */ {2, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *mma845x_get_slave_descr(void)
{
	return &mma845x_descr;
}
EXPORT_SYMBOL(mma845x_get_slave_descr);

/**
 *  @}
 */
