/*
 $License:
    Copyright (C) 2010 InvenSense Corporation, All Rights Reserved.
 $
 */

/**
 *  @defgroup   COMPASSDL (Motion Library - Accelerometer Driver Layer)
 *  @brief      Provides the interface to setup and handle an accelerometers
 *              connected to the secondary I2C interface of the gyroscope.
 *
 *  @{
 *      @file   AK8975.c
 *      @brief  Magnetometer setup and handling methods for AKM 8975 compass.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */
#include <string.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/mpu.h>
#include "mlsl.h"
#include "mlos.h"

#include <log.h>
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-compass"

#define AK8975_REG_ST1	0x02
#define AK8975_REG_HXL  0x03
#define AK8975_REG_ST2  0x09
#define AK8975_REG_CNTL 0x0A
#define AK8975_CNTL_MODE_POWER_DOWN			0x00
#define AK8975_CNTL_MODE_SINGLE_MEASUREMENT 0x01

/*********************************************
    Magnetometer Initialization Functions
**********************************************/

int ak8975_suspend(void *mlsl_handle,
		   struct ext_slave_descr *slave,
		   struct ext_slave_platform_data *pdata)
{
	int result = ML_SUCCESS;

	mpu_dbg("%s\n", __func__);

	return result;
}

int ak8975_resume(void *mlsl_handle,
		  struct ext_slave_descr *slave,
		  struct ext_slave_platform_data *pdata)
{
	int result = ML_SUCCESS;

	mpu_dbg("%s\n", __func__);

	result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				  AK8975_REG_CNTL,
				  AK8975_CNTL_MODE_SINGLE_MEASUREMENT);
	ERROR_CHECK(result);

	return result;
}

int ak8975_read(void *mlsl_handle,
		struct ext_slave_descr *slave,
		struct ext_slave_platform_data *pdata, unsigned char *data)
{
	unsigned char regs[6];
	int result = ML_SUCCESS;
	int status = ML_SUCCESS;

	mpu_dbg("%s\n", __func__);

	result = MLSLSerialRead(mlsl_handle, pdata->address,
				 AK8975_REG_HXL, 6, regs);
	ERROR_CHECK(result);

	memcpy(data, &regs[0], 6);
	status = ML_SUCCESS;
	/*
	 * trigger next measurement if:
	 *    - stat is non zero;
	 *    - if stat is zero and stat2 is non zero.
	 * Won't trigger if data is not ready and there was no error.
	 */

	result = MLSLSerialWriteSingle(mlsl_handle, pdata->address,
				  AK8975_REG_CNTL,
				  AK8975_CNTL_MODE_SINGLE_MEASUREMENT);
	ERROR_CHECK(result);

	return status;
}

static int ak8975_config(void *mlsl_handle,
			struct ext_slave_descr *slave,
			struct ext_slave_platform_data *pdata,
			struct ext_slave_config *data)
{
	int result;
	if (!data->data)
		return ML_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_WRITE_REGISTERS:
		result = MLSLSerialWrite(mlsl_handle, pdata->address,
					data->len,
					(unsigned char *)data->data);
		ERROR_CHECK(result);
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
		return ML_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return ML_SUCCESS;
}

static int ak8975_get_config(void *mlsl_handle,
				struct ext_slave_descr *slave,
				struct ext_slave_platform_data *pdata,
				struct ext_slave_config *data)
{
	int result;
	if (!data->data)
		return ML_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_READ_REGISTERS:
	{
		unsigned char *serial_data = (unsigned char *)data->data;
		result = MLSLSerialRead(mlsl_handle, pdata->address,
					serial_data[0],
					data->len - 1,
					&serial_data[1]);
		ERROR_CHECK(result);
		break;
	}
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
		return ML_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return ML_SUCCESS;
}

struct ext_slave_descr ak8975_descr = {
	/*.init             = */ NULL,
	/*.exit             = */ NULL,
	/*.suspend          = */ ak8975_suspend,
	/*.resume           = */ ak8975_resume,
	/*.read             = */ ak8975_read,
	/*.config           = */ ak8975_config,
	/*.get_config       = */ ak8975_get_config,
	/*.name             = */ "mpu_ak8975",
	/*.type             = */ EXT_SLAVE_TYPE_COMPASS,
	/*.id               = */ COMPASS_ID_AKM,
	/*.reg              = */ 0x01,
	/*.len              = */ 9,
	/*.endian           = */ EXT_SLAVE_LITTLE_ENDIAN,
	/*.range            = */ {9830, 4000}
};

struct ext_slave_descr *ak8975_get_slave_descr(void)
{
	return &ak8975_descr;
}
EXPORT_SYMBOL(ak8975_get_slave_descr);

