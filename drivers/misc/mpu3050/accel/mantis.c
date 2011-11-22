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
 *      @file   mantis.c
 *      @brief  Accelerometer setup and handling methods for Invensense MANTIS
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include "mpu.h"
#include "mldl_cfg.h"
#include "mlsl.h"
#include "mlos.h"

#include <log.h>
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-acc"

/* --------------------- */
/* -    Variables.     - */
/* --------------------- */

struct mantis_config {
	unsigned int odr;		/**< output data rate 1/1000 Hz */
	unsigned int fsr;		/**< full scale range mg */
	unsigned int ths;		/**< mot/no-mot thseshold mg */
	unsigned int dur;		/**< mot/no-mot duration ms */
	unsigned int irq_type;		/**< irq type */
};

struct mantis_private_data {
	struct mantis_config suspend;
	struct mantis_config resume;
	struct mldl_cfg *mldl_cfg_ref;
};

/*****************************************
 *Accelerometer Initialization Functions
 *****************************************/

static int mantis_set_mldl_cfg_ref(void *mlsl_handle,
			struct ext_slave_platform_data *pdata,
			struct mldl_cfg *mldl_cfg_ref)
{
	struct mantis_private_data *private_data =
			(struct mantis_private_data *)pdata->private_data;
	private_data->mldl_cfg_ref = mldl_cfg_ref;
	return INV_SUCCESS;
}

/**
 * Record the odr for use in computing duration values.
 *
 * @param config Config to set, suspend or resume structure
 * @param odr output data rate in 1/1000 hz
 */
static int mantis_set_odr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct mantis_config *config, long apply, long odr)
{
	int result;
	int base, divider;
	struct mantis_private_data *private_data =
			(struct mantis_private_data *)pdata->private_data;
	struct mldl_cfg *mldl_cfg_ref =
			(struct mldl_cfg *)private_data->mldl_cfg_ref;

	if (!mldl_cfg_ref
	    || (mldl_cfg_ref->lpf > 0 && mldl_cfg_ref->lpf < 7))
		base = 1000000;
	else
		base = 8000000;

	if (odr != 0) {
		divider = base / odr;
		config->odr = base / divider;
	} else {
		config->odr = 0;
		return INV_SUCCESS;
	}

	/* if the DMP and/or gyros are on, don't set the ODR =>
	   the DMP/gyro mldl_cfg->divider setting will handle it */
	if (apply
	    && (mldl_cfg_ref
		&& !(mldl_cfg_ref->requested_sensors & INV_DMP_PROCESSOR))) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
				MPUREG_SMPLRT_DIV, divider - 1);
		ERROR_CHECK(result);
		MPL_LOGI("ODR : %d mHz\n", config->odr);
	}
	return INV_SUCCESS;
}

static int mantis_set_fsr(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct mantis_config *config, long apply, long fsr)
{
	unsigned char fsr_mask;
	int result;

	if (fsr <= 2000) {
		config->fsr = 2000;
		fsr_mask = 0x00;
	} else if (fsr <= 4000) {
		config->fsr = 4000;
		fsr_mask = 0x08;
	} else if (fsr <= 8000) {
		config->fsr = 8000;
		fsr_mask = 0x10;
	} else { /* fsr = [8001, oo) */
		config->fsr = 16000;
		fsr_mask = 0x18;
	}

	if (apply) {
		unsigned char reg;
		result = inv_serial_read(mlsl_handle, pdata->address,
					 MPUREG_ACCEL_CONFIG, 1, &reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(mlsl_handle, pdata->address,
						 MPUREG_ACCEL_CONFIG,
						 reg | fsr_mask);
		ERROR_CHECK(result);
		MPL_LOGV("FSR: %d\n", config->fsr);
	}
	return INV_SUCCESS;
}

static int mantis_set_irq(void *mlsl_handle,
			  struct ext_slave_platform_data *pdata,
			  struct mantis_config *config, long apply,
			  long irq_type)
{
	int result = INV_SUCCESS;
	unsigned char reg_int_cfg;

	switch (irq_type) {
	case MPU_SLAVE_IRQ_TYPE_DATA_READY:
		config->irq_type = irq_type;
		reg_int_cfg = 0x01;
		break;
	/* todo: add MOTION, NO_MOTION, and FREEFALL */
	case MPU_SLAVE_IRQ_TYPE_NONE:
		/* Do nothing, not even set the interrupt because it is
		   shared with the gyro */
		config->irq_type = irq_type;
		return INV_SUCCESS;
		break;
	default:
		return INV_ERROR_INVALID_PARAMETER;
		break;
	}

	if (apply) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
						 MPUREG_INT_ENABLE,
						 reg_int_cfg);
		ERROR_CHECK(result);
		MPL_LOGV("irq_type: %d\n", config->irq_type);
	}

	return result;
}

static int mantis_set_ths(void *mlsl_handle,
			  struct ext_slave_platform_data *slave,
			  struct mantis_config *config, long apply, long ths)
{
	if (ths < 0)
		ths = 0;

	config->ths = ths;
	MPL_LOGV("THS: %d\n", config->ths);
	return INV_SUCCESS;
}

static int mantis_set_dur(void *mlsl_handle,
			  struct ext_slave_platform_data *slave,
			  struct mantis_config *config, long apply, long dur)
{
	if (dur < 0)
		dur = 0;

	config->dur = dur;
	MPL_LOGV("DUR: %d\n", config->dur);
	return INV_SUCCESS;
}

static int mantis_init(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	struct mantis_private_data *private_data;
	long range;

#ifdef CONFIG_MPU_SENSORS_MPU3050
	(void *)private_data;
	return INV_ERROR_INVALID_MODULE;
#endif

   if (pdata == NULL)
   {
      printk("%s() pdata == NULL\n", __func__);
      return INV_ERROR;
   }

	private_data = (struct mantis_private_data *)
	    inv_malloc(sizeof(struct mantis_private_data));

	if (!private_data)
		return INV_ERROR_MEMORY_EXAUSTED;

	pdata->private_data = private_data;

	mantis_set_odr(mlsl_handle, pdata, &private_data->suspend,
		       FALSE, 0);
	mantis_set_odr(mlsl_handle, pdata, &private_data->resume,
		       FALSE, 200000);

	range = RANGE_FIXEDPOINT_TO_LONG_MG(slave->range);
	mantis_set_fsr(mlsl_handle, pdata, &private_data->suspend,
		       FALSE, range);
	mantis_set_fsr(mlsl_handle, pdata, &private_data->resume,
		       FALSE, range);

	mantis_set_irq(mlsl_handle, pdata, &private_data->suspend, FALSE,
		       MPU_SLAVE_IRQ_TYPE_NONE);
	mantis_set_irq(mlsl_handle, pdata, &private_data->resume, FALSE,
		       MPU_SLAVE_IRQ_TYPE_NONE);

	mantis_set_ths(mlsl_handle, pdata, &private_data->suspend, FALSE, 80);
	mantis_set_ths(mlsl_handle, pdata, &private_data->resume, FALSE, 40);

	mantis_set_dur(mlsl_handle, pdata, &private_data->suspend, FALSE, 1000);
	mantis_set_dur(mlsl_handle, pdata, &private_data->resume, FALSE, 2540);

	return INV_SUCCESS;
}

static int mantis_exit(void *mlsl_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata)
{
	if (pdata->private_data)
		return inv_free(pdata->private_data);
	else
		return INV_SUCCESS;
}

int mantis_suspend(void *mlsl_handle,
		   struct ext_slave_descr *slave,
		   struct ext_slave_platform_data *pdata)
{
	unsigned char reg;
	int result;
	struct mantis_private_data *private_data =
			(struct mantis_private_data *)pdata->private_data;

	result = mantis_set_odr(mlsl_handle, pdata, &private_data->suspend,
				TRUE, private_data->suspend.odr);
	ERROR_CHECK(result);

	result = mantis_set_irq(mlsl_handle, pdata, &private_data->suspend,
				TRUE, private_data->suspend.irq_type);
	ERROR_CHECK(result);

	result = inv_serial_read(mlsl_handle, pdata->address,
				 MPUREG_PWR_MGMT_2, 1, &reg);
	ERROR_CHECK(result);
	reg |= (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA);

	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_PWR_MGMT_2, reg);
	ERROR_CHECK(result);

	return INV_SUCCESS;
}

int mantis_resume(void *mlsl_handle,
		  struct ext_slave_descr *slave,
		  struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;
	unsigned char reg;
	struct mantis_private_data *private_data =
			(struct mantis_private_data *)pdata->private_data;

	result = inv_serial_read(mlsl_handle, pdata->address,
				 MPUREG_PWR_MGMT_1, 1, &reg);
	ERROR_CHECK(result);
#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	if (reg & BIT_SLEEP) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					MPUREG_PWR_MGMT_1, reg & ~BIT_SLEEP);
		ERROR_CHECK(result);
	}
#else
	if ((reg & BITS_PWRSEL) != BITS_PWRSEL) {
		result = inv_serial_single_write(mlsl_handle, pdata->address,
					MPUREG_PWR_MGMT_1, reg | BITS_PWRSEL);
		ERROR_CHECK(result);
	}
#endif
	inv_sleep(2);

	result = inv_serial_read(mlsl_handle, pdata->address,
			MPUREG_PWR_MGMT_2, 1, &reg);
	ERROR_CHECK(result);
	reg &= ~(BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_PWR_MGMT_2, reg);
	ERROR_CHECK(result);

	/* settings */

	result = mantis_set_fsr(mlsl_handle, pdata, &private_data->resume,
				TRUE, private_data->resume.fsr);
	ERROR_CHECK(result);

	result = mantis_set_odr(mlsl_handle, pdata, &private_data->resume,
				TRUE, private_data->resume.odr);
	ERROR_CHECK(result);

	result = mantis_set_irq(mlsl_handle, pdata, &private_data->resume,
				TRUE, private_data->resume.irq_type);
	ERROR_CHECK(result);

	/* motion, no_motion */
	reg = (unsigned char)private_data->suspend.ths / ACCEL_MOT_THR_LSB;
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_ACCEL_MOT_THR, reg);
	ERROR_CHECK(result);
	reg = (unsigned char)
	    ACCEL_ZRMOT_THR_LSB_CONVERSION(private_data->resume.ths);
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_ACCEL_ZRMOT_THR, reg);
	ERROR_CHECK(result);
	reg = (unsigned char)private_data->suspend.ths / ACCEL_MOT_DUR_LSB;
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_ACCEL_MOT_DUR, reg);
	ERROR_CHECK(result);
	reg = (unsigned char)private_data->resume.ths / ACCEL_ZRMOT_DUR_LSB;
	result = inv_serial_single_write(mlsl_handle, pdata->address,
					 MPUREG_ACCEL_ZRMOT_DUR, reg);
	ERROR_CHECK(result);
	return result;
}

int mantis_read(void *mlsl_handle,
		struct ext_slave_descr *slave,
		struct ext_slave_platform_data *pdata, unsigned char *data)
{
	int result;
	result = inv_serial_read(mlsl_handle, pdata->address,
				 slave->read_reg, slave->read_len, data);
	return result;
}

static int mantis_config(void *mlsl_handle,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata,
			 struct ext_slave_config *data)
{
	struct mantis_private_data *private_data = pdata->private_data;
	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		return mantis_set_odr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		return mantis_set_odr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		return mantis_set_fsr(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		return mantis_set_fsr(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_THS:
		return mantis_set_ths(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_THS:
		return mantis_set_ths(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_MOT_DUR:
		return mantis_set_dur(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_NMOT_DUR:
		return mantis_set_dur(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply, *((long *)data->data));
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
		return mantis_set_irq(mlsl_handle, pdata,
				      &private_data->suspend,
				      data->apply, *((long *)data->data));
		break;
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		return mantis_set_irq(mlsl_handle, pdata,
				      &private_data->resume,
				      data->apply, *((long *)data->data));
		break;
	case MPU_SLAVE_CONFIG_INTERNAL_REFERENCE:
		return mantis_set_mldl_cfg_ref(mlsl_handle, pdata,
					       (struct mldl_cfg *)data->data);
		break;

	default:
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return INV_SUCCESS;
}

static int mantis_get_config(void *mlsl_handle,
			     struct ext_slave_descr *slave,
			     struct ext_slave_platform_data *pdata,
			     struct ext_slave_config *data)
{
	struct mantis_private_data *private_data = pdata->private_data;
	if (!data->data)
		return INV_ERROR_INVALID_PARAMETER;

	switch (data->key) {
	case MPU_SLAVE_CONFIG_ODR_SUSPEND:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->suspend.odr;
		break;
	case MPU_SLAVE_CONFIG_ODR_RESUME:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->resume.odr;
		break;
	case MPU_SLAVE_CONFIG_FSR_SUSPEND:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->suspend.fsr;
		break;
	case MPU_SLAVE_CONFIG_FSR_RESUME:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->resume.fsr;
		break;
	case MPU_SLAVE_CONFIG_MOT_THS:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->suspend.ths;
		break;
	case MPU_SLAVE_CONFIG_NMOT_THS:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->resume.ths;
		break;
	case MPU_SLAVE_CONFIG_MOT_DUR:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->suspend.dur;
		break;
	case MPU_SLAVE_CONFIG_NMOT_DUR:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->resume.dur;
		break;
	case MPU_SLAVE_CONFIG_IRQ_SUSPEND:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->suspend.irq_type;
		break;
	case MPU_SLAVE_CONFIG_IRQ_RESUME:
		(*(unsigned long *)data->data) =
		    (unsigned long)private_data->resume.irq_type;
		break;
	default:
		return INV_ERROR_FEATURE_NOT_IMPLEMENTED;
	};

	return INV_SUCCESS;
}

struct ext_slave_descr mantis_descr = {
	/*.init             = */ mantis_init,
	/*.exit             = */ mantis_exit,
	/*.suspend          = */ mantis_suspend,
	/*.resume           = */ mantis_resume,
	/*.read             = */ mantis_read,
	/*.config           = */ mantis_config,
	/*.get_config       = */ mantis_get_config,
	/*.name             = */ "mantis",
	/*.type             = */ EXT_SLAVE_TYPE_ACCELEROMETER,
	/*.id               = */ ACCEL_ID_MPU6050,
	/*.reg              = */ 0x3B,
	/*.len              = */ 6,
	/*.endian           = */ EXT_SLAVE_BIG_ENDIAN,
	/*.range            = */ {2, 0},
	/*.trigger          = */ NULL,
};

struct ext_slave_descr *mantis_get_slave_descr(void)
{
	return &mantis_descr;
}
EXPORT_SYMBOL(mantis_get_slave_descr);

/**
 *  @}
 */
