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
 *  @addtogroup MLDL
 *
 *  @{
 *      @file   mldl_cfg.c
 *      @brief  The Motion Library Driver Layer.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */

#include <stddef.h>

#include "mldl_cfg.h"
#include "mpu.h"

#include "mlsl.h"
#include "mlos.h"

#include "log.h"
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "mldl_cfg:"

/* --------------------- */
/* -    Variables.     - */
/* --------------------- */

#if defined CONFIG_MPU_SENSORS_MPU6050A2 || defined CONFIG_MPU_SENSORS_MPU6050B1
#define SLEEP   0
#define WAKE_UP 7
#define RESET   1
#define STANDBY 1
#else
#define SLEEP   1
#define WAKE_UP 0
#define RESET   1
#define STANDBY 1
#endif

/*---------------------*/
/*-    Prototypes.    -*/
/*---------------------*/

/*----------------------*/
/*-  Static Functions. -*/
/*----------------------*/

/**
 * @brief Stop the DMP running
 *
 * @return INV_SUCCESS or non-zero error code
 */
static int dmp_stop(struct mldl_cfg *mldl_cfg, void *gyro_handle)
{
	unsigned char userCtrlReg;
	int result;

	if (!mldl_cfg->dmp_is_running)
		return INV_SUCCESS;

	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_USER_CTRL, 1, &userCtrlReg);
	ERROR_CHECK(result);
	userCtrlReg = (userCtrlReg & (~BIT_FIFO_EN)) | BIT_FIFO_RST;
	userCtrlReg = (userCtrlReg & (~BIT_DMP_EN)) | BIT_DMP_RST;

	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_USER_CTRL, userCtrlReg);
	ERROR_CHECK(result);
	mldl_cfg->dmp_is_running = FALSE;

	return result;
}

/**
 * @brief Starts the DMP running
 *
 * @return INV_SUCCESS or non-zero error code
 */
static int dmp_start(struct mldl_cfg *pdata, void *mlsl_handle)
{
	unsigned char userCtrlReg;
	int result;

	if (pdata->dmp_is_running == pdata->dmp_enable)
		return INV_SUCCESS;

	result = inv_serial_read(mlsl_handle, pdata->addr,
				 MPUREG_USER_CTRL, 1, &userCtrlReg);
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->addr,
					 MPUREG_USER_CTRL,
					 ((userCtrlReg & (~BIT_FIFO_EN))
					  | BIT_FIFO_RST));
	ERROR_CHECK(result);

	result = inv_serial_single_write(mlsl_handle, pdata->addr,
					 MPUREG_USER_CTRL, userCtrlReg);
	ERROR_CHECK(result);

	result = inv_serial_read(mlsl_handle, pdata->addr,
				 MPUREG_USER_CTRL, 1, &userCtrlReg);
	ERROR_CHECK(result);

	if (pdata->dmp_enable)
		userCtrlReg |= BIT_DMP_EN;
	else
		userCtrlReg &= ~BIT_DMP_EN;

	if (pdata->fifo_enable)
		userCtrlReg |= BIT_FIFO_EN;
	else
		userCtrlReg &= ~BIT_FIFO_EN;

	userCtrlReg |= BIT_DMP_RST;

	result = inv_serial_single_write(mlsl_handle, pdata->addr,
					 MPUREG_USER_CTRL, userCtrlReg);
	ERROR_CHECK(result);
	pdata->dmp_is_running = pdata->dmp_enable;

	return result;
}

#if defined CONFIG_MPU_SENSORS_MPU6050B1 || \
    defined CONFIG_MPU_SENSORS_MPU6050A2

/**
 *  @brief  enables/disables the I2C bypass to an external device
 *          connected to MPU's secondary I2C bus.
 *  @param  enable
 *              Non-zero to enable pass through.
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
static int mpu6050_set_i2c_bypass(struct mldl_cfg *mldl_cfg,
				    void *mlsl_handle, unsigned char enable)
{
	unsigned char reg;
	int result;

	if ((mldl_cfg->gyro_is_bypassed && enable) ||
	    (!mldl_cfg->gyro_is_bypassed && !enable))
		return INV_SUCCESS;

	/*---- get current 'USER_CTRL' into b ----*/
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_USER_CTRL, 1, &reg);
	ERROR_CHECK(result);

	if (!enable) {
		/* setting int_config with the property flag BIT_BYPASS_EN
		   should be done by the setup functions */
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_INT_PIN_CFG,
						 (mldl_cfg->pdata->int_config &
						  ~(BIT_BYPASS_EN)));
		if (!(reg & BIT_AUX_IF_EN)) {
			result =
			    inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						    MPUREG_USER_CTRL,
						    (reg | BIT_AUX_IF_EN));
			ERROR_CHECK(result);
		}
	} else if (enable) {
		if (reg & BIT_AUX_IF_EN) {
			result =
			    inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						    MPUREG_USER_CTRL,
						    (reg & (~BIT_AUX_IF_EN)));
			ERROR_CHECK(result);
			/*****************************************
			 * To avoid hanging the bus we must sleep until all
			 * slave transactions have been completed.
			 *  24 bytes max slave reads
			 *  +1 byte possible extra write
			 *  +4 max slave address
			 * ---
			 *  33 Maximum bytes
			 *  x9 Approximate bits per byte
			 * ---
			 * 297 bits.
			 * 2.97 ms minimum @ 100kbps
			 * 0.75 ms minimum @ 400kbps.
			 *****************************************/
			inv_sleep(3);
		}
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_INT_PIN_CFG,
						 (mldl_cfg->pdata->int_config |
						  BIT_BYPASS_EN));
		ERROR_CHECK(result);
	}
	mldl_cfg->gyro_is_bypassed = enable;
	return result;
}

#else 	/* CONFIG_MPU_SENSORS_MPU3050 */

static int mpu3050_set_i2c_bypass(struct mldl_cfg *mldl_cfg,
				  void *mlsl_handle, unsigned char enable)
{
	unsigned char b;
	int result;

	if ((mldl_cfg->gyro_is_bypassed && enable) ||
	    (!mldl_cfg->gyro_is_bypassed && !enable))
		return INV_SUCCESS;

	/*---- get current 'USER_CTRL' into b ----*/
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_USER_CTRL, 1, &b);
	ERROR_CHECK(result);

	b &= ~BIT_AUX_IF_EN;

	if (!enable) {
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_USER_CTRL,
						 (b | BIT_AUX_IF_EN));
		ERROR_CHECK(result);
	} else {
		/* Coming out of I2C is tricky due to several erratta.  Do not
		 * modify this algorithm
		 */
		/*
		 * 1) wait for the right time and send the command to change
		 * the aux i2c slave address to an invalid address that will
		 * get nack'ed
		 *
		 * 0x00 is broadcast.  0x7F is unlikely to be used by any aux.
		 */
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_AUX_SLV_ADDR, 0x7F);
		ERROR_CHECK(result);
		/*
		 * 2) wait enough time for a nack to occur, then go into
		 *    bypass mode:
		 */
		inv_sleep(2);
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_USER_CTRL, (b));
		ERROR_CHECK(result);
		/*
		 * 3) wait for up to one MPU cycle then restore the slave
		 *    address
		 */
		inv_sleep(SAMPLING_PERIOD_US(mldl_cfg) / 1000);
		result = inv_serial_single_write(
			mlsl_handle, mldl_cfg->addr,
			MPUREG_AUX_SLV_ADDR,
			mldl_cfg->pdata->accel.address);
		ERROR_CHECK(result);

		/*
		 * 4) reset the ime interface
		 */

		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_USER_CTRL,
						 (b | BIT_AUX_IF_RST));
		ERROR_CHECK(result);
		inv_sleep(2);
	}
	mldl_cfg->gyro_is_bypassed = enable;

	return result;
}

#endif

/**
 *  @brief  enables/disables the I2C bypass to an external device
 *          connected to MPU's secondary I2C bus.
 *  @param  enable
 *              Non-zero to enable pass through.
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
static int mpu_set_i2c_bypass(struct mldl_cfg *mldl_cfg,
			      void *mlsl_handle, unsigned char enable)
{
#if	defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	return mpu6050_set_i2c_bypass(mldl_cfg, mlsl_handle, enable);
#else				/* CONFIG_MPU_SENSORS_MPU3050 */
	return mpu3050_set_i2c_bypass(mldl_cfg, mlsl_handle, enable);
#endif
}


#define NUM_OF_PROD_REVS (DIM(prod_rev_map))

/* NOTE : when not indicated, product revision
	  is considered an 'npp'; non production part */
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1

/* produces an unique identifier for each device based on the
   combination of product version and product revision */
struct prod_rev_map_t {
	unsigned short mpl_product_key;
	unsigned char silicon_rev;
	unsigned short gyro_trim;
	unsigned short accel_trim;
};

/* NOTE: product entries are in chronological order */
static struct prod_rev_map_t prod_rev_map[] = {
	/* prod_ver = 0 */
	{MPL_PROD_KEY(0,  1), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  2), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  3), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  4), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  5), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  6), MPU_SILICON_REV_A2, 131, 16384},	/* (A2/C2-1) */
	/* prod_ver = 1, forced to 0 for MPU6050 A2 */
	{MPL_PROD_KEY(0,  7), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  8), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0,  9), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 10), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 11), MPU_SILICON_REV_A2, 131, 16384},	/* (A2/D2-1) */
	{MPL_PROD_KEY(0, 12), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 13), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 14), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 15), MPU_SILICON_REV_A2, 131, 16384},
	{MPL_PROD_KEY(0, 27), MPU_SILICON_REV_A2, 131, 16384},	/* (A2/D4)   */
	/* prod_ver = 1 */
	{MPL_PROD_KEY(1, 16), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D2-1) */
	{MPL_PROD_KEY(1, 17), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D2-2) */
	{MPL_PROD_KEY(1, 18), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D2-3) */
	{MPL_PROD_KEY(1, 19), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D2-4) */
	{MPL_PROD_KEY(1, 20), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D2-5) */
	{MPL_PROD_KEY(1, 28), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/D4)   */
	{MPL_PROD_KEY(1,  1), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-1) */
	{MPL_PROD_KEY(1,  2), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-2) */
	{MPL_PROD_KEY(1,  3), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-3) */
	{MPL_PROD_KEY(1,  4), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-4) */
	{MPL_PROD_KEY(1,  5), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-5) */
	{MPL_PROD_KEY(1,  6), MPU_SILICON_REV_B1, 131, 16384},	/* (B1/E1-6) */
	/* prod_ver = 2 */
	{MPL_PROD_KEY(2,  7), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-1) */
	{MPL_PROD_KEY(2,  8), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-2) */
	{MPL_PROD_KEY(2,  9), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-3) */
	{MPL_PROD_KEY(2, 10), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-4) */
	{MPL_PROD_KEY(2, 11), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-5) */
	{MPL_PROD_KEY(2, 12), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E1-6) */
	{MPL_PROD_KEY(2, 29), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/D4)   */
	/* prod_ver = 3 */
	{MPL_PROD_KEY(3, 30), MPU_SILICON_REV_B1, 131, 16384},	/* (B2/E2)   */
	/* prod_ver = 4 */
	{MPL_PROD_KEY(4, 31), MPU_SILICON_REV_B1, 131,  8192},	/* (B2/F1)   */
	{MPL_PROD_KEY(4,  1), MPU_SILICON_REV_B1, 131,  8192},	/* (B3/F1)   */
	{MPL_PROD_KEY(4,  3), MPU_SILICON_REV_B1, 131,  8192},	/* (B4/F1)   */
        /* prod_ver = 5 */
        {MPL_PROD_KEY(6, 19), MPU_SILICON_REV_B1, 131, 16384},  /* (B5/E2)   */
        /* prod_ver = 7 */
        {MPL_PROD_KEY(7, 19), MPU_SILICON_REV_B1, 131, 16384},  /* (B5/E2)   */
        /* prod_ver = 8 */
        {MPL_PROD_KEY(8, 19), MPU_SILICON_REV_B1, 131, 16384}   /* (B5/E2)   */
};

/**
 *  @internal
 *  @brief  Inverse lookup of the index of an MPL product key .
 *  @param  key
 *              the MPL product indentifier also referred to as 'key'.
 *  @return the index position of the key in the array, -1 if not found.
 */
short index_of_key(unsigned short key)
{
	int i;
	for (i = 0; i < NUM_OF_PROD_REVS; i++)
		if (prod_rev_map[i].mpl_product_key == key)
			return (short)i;
	return -1;
}

/**
 *  @internal
 *  @brief  Get the product revision and version for MPU6050 and
 *          extract all per-part specific information.
 *          The product version number is read from the PRODUCT_ID register in
 *          user space register map.
 *          The product revision number is in read from OTP bank 0, ADDR6[7:2].
 *          These 2 numbers, combined, provide an unique key to be used to
 *          retrieve some per-device information such as the silicon revision
 *          and the gyro and accel sensitivity trim values.
 *
 *  @param  mldl_cfg
 *              a pointer to the mldl config data structure.
 *  @param  mlsl_handle
 *              an file handle to the serial communication device the
 *              device is connected to.
 *
 *  @return 0 on success, a non-zero error code otherwise.
 */
static int inv_get_silicon_rev_mpu6050(
		struct mldl_cfg *mldl_cfg, void *mlsl_handle)
{
	int result;
	unsigned char prod_ver = 0x00, prod_rev = 0x00;
	unsigned char bank =
	    (BIT_PRFTCH_EN | BIT_CFG_USER_BANK | MPU_MEM_OTP_BANK_0);
	unsigned short memAddr = ((bank << 8) | 0x06);
	unsigned short key, index;

#if defined CONFIG_MPU_SENSORS_MPU6050B1
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_PRODUCT_ID, 1, &prod_ver);
        MPL_LOGE("INFO: read prod_ver %d from register %d\n", 
                 prod_ver, MPUREG_PRODUCT_ID);
	ERROR_CHECK(result);
#else
	/* MPU6050 A2 does not have an equivalent PRODUCT_ID register */
	prod_ver = 0;
#endif
        /* leave only less significant 4 bits in version */
        prod_ver &= 0xF;

	result = inv_serial_read_mem(mlsl_handle, mldl_cfg->addr,
				     memAddr, 1, &prod_rev);
        MPL_LOGE("INFO: read prod_rev %d memAddr %d mldl_cfg->addr %d\n", 
                 prod_rev, memAddr, mldl_cfg->addr);

	ERROR_CHECK(result);
        MPL_LOGE("INFO: prod_ver %d prod_rev %d\n", prod_ver, prod_rev);

	prod_rev >>= 2;

	/* clean the prefetch and cfg user bank bits */
	result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
					 MPUREG_BANK_SEL, 0);
	ERROR_CHECK(result);

	key = MPL_PROD_KEY(prod_ver, prod_rev);
        MPL_LOGE("INFO: calculated key %d\n", key);

	if (key == 0) {
		MPL_LOGE("Product key 0 "
			 "indicates device is either "
			 "incompatible or an MPU3050\n");
		return INV_ERROR_INVALID_MODULE;
	}
	index = index_of_key(key);
        MPL_LOGE("INFO: calculated index %d\n", index);
	if (index == -1 || index >= NUM_OF_PROD_REVS) {
		MPL_LOGE("Unsupported product key %d in MPL\n", key);
		return INV_ERROR_INVALID_MODULE;
	}
	/* check MPL is compiled for this device */
#if defined CONFIG_MPU_SENSORS_MPU6050A2
	if (prod_rev_map[index].silicon_rev != MPU_SILICON_REV_A2) {
		MPL_LOGE("MPL compiled for MPU6050A2 support "
			 "but device is not MPU6050A2 (%d)\n", key);
		return INV_ERROR_INVALID_MODULE;
	}
#else
	if (prod_rev_map[index].silicon_rev != MPU_SILICON_REV_B1) {
		MPL_LOGE("MPL compiled for MPU6050B1 support "
			 "but device is not MPU6050B1 (%d)\n", key);
		return INV_ERROR_INVALID_MODULE;
	}
#endif
	mldl_cfg->product_id = prod_ver;
	mldl_cfg->product_revision = prod_rev;
	mldl_cfg->silicon_revision = prod_rev_map[index].silicon_rev;
	mldl_cfg->gyro_sens_trim = prod_rev_map[index].gyro_trim;
	mldl_cfg->accel_sens_trim = prod_rev_map[index].accel_trim;

	return result;
}

#define inv_get_silicon_rev inv_get_silicon_rev_mpu6050

#else /* not CONFIG_MPU_SENSORS_MPU6050XX */

struct prod_rev_map_t {
	unsigned char silicon_rev;
	unsigned short gyro_trim;
};

#define OLDEST_PROD_REV_SUPPORTED	11
static struct prod_rev_map_t prod_rev_map[] = {
	{0, 0},
	{MPU_SILICON_REV_A4, 131},	/* 1  A? OBSOLETED */
	{MPU_SILICON_REV_A4, 131},	/* 2  |  */
	{MPU_SILICON_REV_A4, 131},	/* 3  |  */
	{MPU_SILICON_REV_A4, 131},	/* 4  |  */
	{MPU_SILICON_REV_A4, 131},	/* 5  |  */
	{MPU_SILICON_REV_A4, 131},	/* 6  |  */
	{MPU_SILICON_REV_A4, 131},	/* 7  |  */
	{MPU_SILICON_REV_A4, 131},	/* 8  |  */
	{MPU_SILICON_REV_A4, 131},	/* 9  |  */
	{MPU_SILICON_REV_A4, 131},	/* 10 V  */
	{MPU_SILICON_REV_B1, 131},	/* 11 B1 */
	{MPU_SILICON_REV_B1, 131},	/* 12 |  */
	{MPU_SILICON_REV_B1, 131},	/* 13 |  */
	{MPU_SILICON_REV_B1, 131},	/* 14 V  */
	{MPU_SILICON_REV_B4, 131},	/* 15 B4 */
	{MPU_SILICON_REV_B4, 131},	/* 16 |  */
	{MPU_SILICON_REV_B4, 131},	/* 17 |  */
	{MPU_SILICON_REV_B4, 131},	/* 18 |  */
	{MPU_SILICON_REV_B4, 115},	/* 19 |  */
	{MPU_SILICON_REV_B4, 115},	/* 20 V  */
	{MPU_SILICON_REV_B6, 131},	/* 21 B6 (B6/A9)  */
	{MPU_SILICON_REV_B4, 115},	/* 22 B4 (B7/A10) */
	{MPU_SILICON_REV_B6, 0},	/* 23 B6 */
	{MPU_SILICON_REV_B6, 0},	/* 24 |  */
	{MPU_SILICON_REV_B6, 0},	/* 25 |  */
	{MPU_SILICON_REV_B6, 131},	/* 26 V  (B6/A11) */
};

/**
 *  @internal
 *  @brief  Get the silicon revision ID from OTP for MPU3050.
 *          The silicon revision number is in read from OTP bank 0,
 *          ADDR6[7:2].  The corresponding ID is retrieved by lookup
 *          in a map.
 *
 *  @param  mldl_cfg
 *              a pointer to the mldl config data structure.
 *  @param  mlsl_handle
 *              an file handle to the serial communication device the
 *              device is connected to.
 *
 *  @return 0 on success, a non-zero error code otherwise.
 */
static int inv_get_silicon_rev_mpu3050(
		struct mldl_cfg *mldl_cfg, void *mlsl_handle)
{
	int result;
	unsigned char index = 0x00;
	unsigned char bank =
	    (BIT_PRFTCH_EN | BIT_CFG_USER_BANK | MPU_MEM_OTP_BANK_0);
	unsigned short mem_addr = ((bank << 8) | 0x06);

	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_PRODUCT_ID, 1, &mldl_cfg->product_id);
	ERROR_CHECK(result);

	result = inv_serial_read_mem(mlsl_handle, mldl_cfg->addr,
				     mem_addr, 1, &index);
	ERROR_CHECK(result);

	index >>= 2;

	/* clean the prefetch and cfg user bank bits */
	result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
					 MPUREG_BANK_SEL, 0);
	ERROR_CHECK(result);
	if (result)
		return result;

	if (index < OLDEST_PROD_REV_SUPPORTED || index >= NUM_OF_PROD_REVS) {
		mldl_cfg->silicon_revision = 0;
		mldl_cfg->gyro_sens_trim = 0;
		MPL_LOGE("Unsupported Product Revision Detected : %d\n", index);
		return INV_ERROR_INVALID_MODULE;
	}

	mldl_cfg->product_revision = index;
	mldl_cfg->silicon_revision = prod_rev_map[index].silicon_rev;
	mldl_cfg->gyro_sens_trim = prod_rev_map[index].gyro_trim;
	if (mldl_cfg->gyro_sens_trim == 0) {
		MPL_LOGE("gyro sensitivity trim is 0"
			 " - unsupported non production part.\n");
		return INV_ERROR_INVALID_MODULE;
	}

	return result;
}
#define inv_get_silicon_rev inv_get_silicon_rev_mpu3050

#endif	/* not CONFIG_MPU_SENSORS_MPU6050XX */

/**
 *  @brief  Enable / Disable the use MPU's secondary I2C interface level
 *          shifters.
 *          When enabled the secondary I2C interface to which the external
 *          device is connected runs at VDD voltage (main supply).
 *          When disabled the 2nd interface runs at VDDIO voltage.
 *          See the device specification for more details.
 *
 *  @note   using this API may produce unpredictable results, depending on how
 *          the MPU and slave device are setup on the target platform.
 *          Use of this API should entirely be restricted to system
 *          integrators. Once the correct value is found, there should be no
 *          need to change the level shifter at runtime.
 *
 *  @pre    Must be called after inv_serial_start().
 *  @note   Typically called before inv_dmp_open().
 *
 *  @param[in]  enable:
 *                  0 to run at VDDIO (default),
 *                  1 to run at VDD.
 *
 *  @return INV_SUCCESS if successfull, a non-zero error code otherwise.
 */
static int MLDLSetLevelShifterBit(struct mldl_cfg *pdata,
				  void *mlsl_handle, unsigned char enable)
{
#if defined CONFIG_MPU_SENSORS_MPU3050
	int result;
	unsigned char reg;
	unsigned char mask;
	unsigned char regval;

	if (0 == pdata->silicon_revision)
		return INV_ERROR_INVALID_PARAMETER;

	/*-- on parts before B6 the VDDIO bit is bit 7 of ACCEL_BURST_ADDR --
	NOTE: this is incompatible with ST accelerometers where the VDDIO
	bit MUST be set to enable ST's internal logic to autoincrement
	the register address on burst reads --*/
	if ((pdata->silicon_revision & 0xf) < MPU_SILICON_REV_B6) {
		reg = MPUREG_ACCEL_BURST_ADDR;
		mask = 0x80;
	} else {
		/*-- on B6 parts the VDDIO bit was moved to FIFO_EN2 =>
		  the mask is always 0x04 --*/
		reg = MPUREG_FIFO_EN2;
		mask = 0x04;
	}

	result = inv_serial_read(mlsl_handle, pdata->addr, reg, 1, &regval);
	ERROR_CHECK(result);

	if (enable)
		regval |= mask;
	else
		regval &= ~mask;

	result = inv_serial_single_write(mlsl_handle, pdata->addr, reg, regval);
	ERROR_CHECK(result);
	return result;
#elif defined CONFIG_MPU_SENSORS_MPU6050A2
	int result;
	unsigned char regval;

	result = inv_serial_read(mlsl_handle, pdata->addr,
				 MPUREG_I2C_MST_CTRL, 1, &regval);
	ERROR_CHECK(result);
	if (enable)
		regval |= BIT_I2C_MST_VDDIO;

	result = inv_serial_single_write(mlsl_handle, pdata->addr,
					 MPUREG_I2C_MST_CTRL, regval);
	ERROR_CHECK(result);
#endif
	return INV_SUCCESS;
}

#if defined CONFIG_MPU_SENSORS_MPU6050A2
/**
 * @internal
 * @brief MPU6050 A2 power management functions.
 * @param mldl_cfg
 *          a pointer to the internal mldl_cfg data structure.
 * @param mlsl_handle
 *          a file handle to the serial device used to communicate
 *          with the MPU6050 A2 device.
 * @param reset
 *          1 to reset hardware.
 * @param sensors
 *          Bitfield of sensors to leave on
 *
 * @return 0 on success, a non-zero error code on error.
 */
static inv_error_t mpu60xx_pwr_mgmt(struct mldl_cfg *mldl_cfg,
				    void *mlsl_handle,
				    unsigned int reset, unsigned long sensors)
{
	unsigned char b;
	inv_error_t result;
	int powerselection;
	int sleep = !(sensors & (INV_THREE_AXIS_GYRO | INV_THREE_AXIS_ACCEL));
	if (!sleep)
		powerselection = BITS_PWRSEL;
	else
		powerselection = 0;

	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_PWR_MGMT_1, 1, &b);
	ERROR_CHECK(result);

	b &= ~BITS_PWRSEL;

	if (reset) {
		MPL_LOGI("Reset MPU6050 A2\n");
		/* Current silicon has an errata where the reset will get
		 * nacked.  Ignore the error code for now. */
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGMT_1,
						 b | BIT_H_RESET);
		inv_sleep(15);
		mldl_cfg->gyro_is_bypassed = FALSE;
	}

	b |= powerselection;

	if (b & BITS_PWRSEL)
		mldl_cfg->gyro_is_suspended = FALSE;
	else
		mldl_cfg->gyro_is_suspended = TRUE;

	result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
					 MPUREG_PWR_MGM, b);
	ERROR_CHECK(result);

	return INV_SUCCESS;
}

#elif defined CONFIG_MPU_SENSORS_MPU6050B1

/**
 * @internal
 * @brief MPU6050 B1 power management functions.
 * @param mldl_cfg
 *          a pointer to the internal mldl_cfg data structure.
 * @param mlsl_handle
 *          a file handle to the serial device used to communicate
 *          with the MPU6050 B1 device.
 * @param reset
 *          1 to reset hardware.
 * @param sensors
 *          Bitfield of sensors to leave on
 *
 * @return 0 on success, a non-zero error code on error.
 */
static inv_error_t mpu60xx_pwr_mgmt(struct mldl_cfg *mldl_cfg,
				    void *mlsl_handle,
				    unsigned int reset, unsigned long sensors)
{
	unsigned char pwr_mgmt[2];
	unsigned char pwr_mgmt_prev[2];
	inv_error_t result;
	int sleep = !(sensors & (INV_THREE_AXIS_GYRO | INV_THREE_AXIS_ACCEL));

	if (reset) {
		MPL_LOGI("Reset MPU6050 B1\n");
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGMT_1, BIT_H_RESET);
		ERROR_CHECK(result);
		mldl_cfg->gyro_is_bypassed = FALSE;
		inv_sleep(15);
	}

	/* NOTE : reading both PWR_MGMT_1 and PWR_MGMT_2 for efficiency because
		  they are accessible even when the device is powered off */
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_PWR_MGMT_1, 2, pwr_mgmt_prev);
	ERROR_CHECK(result);

	pwr_mgmt[0] = pwr_mgmt_prev[0];
	pwr_mgmt[1] = pwr_mgmt_prev[1];

	if (sleep)
		pwr_mgmt[0] |= BIT_SLEEP;
	else
		pwr_mgmt[0] &= ~BIT_SLEEP;

	if (pwr_mgmt[0] != pwr_mgmt_prev[0]) {
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGMT_1, pwr_mgmt[0]);
		ERROR_CHECK(result);
	}
	pwr_mgmt[1] &= ~(BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG);
	if (!(sensors & INV_X_GYRO))
		pwr_mgmt[1] |= BIT_STBY_XG;
	if (!(sensors & INV_Y_GYRO))
		pwr_mgmt[1] |= BIT_STBY_YG;
	if (!(sensors & INV_Z_GYRO))
		pwr_mgmt[1] |= BIT_STBY_ZG;

	if (pwr_mgmt[1] != pwr_mgmt_prev[1]) {
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGMT_2, pwr_mgmt[1]);
		ERROR_CHECK(result);
	}

	if ((pwr_mgmt[1] & (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)) ==
	    (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)) {
		mldl_cfg->gyro_is_suspended = TRUE;
	} else {
		mldl_cfg->gyro_is_suspended = FALSE;
	}

	return INV_SUCCESS;
}

#else	/* CONFIG_MPU_SENSORS_MPU6050xxx */

/**
 * @internal
 * @brief   This function controls the power management on the MPU device.
 *          The entire chip can be put to low power sleep mode, or individual
 *          gyros can be turned on/off.
 *
 *          Putting the device into sleep mode depending upon the changing needs
 *          of the associated applications is a recommended method for reducing
 *          power consuption.  It is a safe opearation in that sleep/wake up of
 *          gyros while running will not result in any interruption of data.
 *
 *          Although it is entirely allowed to put the device into full sleep
 *          while running the DMP, it is not recomended because it will disrupt
 *          the ongoing calculations carried on inside the DMP and consequently
 *          the sensor fusion algorithm. Furthermore, while in sleep mode
 *          read & write operation from the app processor on both registers and
 *          memory are disabled and can only regained by restoring the MPU in
 *          normal power mode.
 *          Disabling any of the gyro axis will reduce the associated power
 *          consuption from the PLL but will not stop the DMP from running
 *          state.
 *
 * @param   reset
 *              Non-zero to reset the device. Note that this setting
 *              is volatile and the corresponding register bit will
 *              clear itself right after being applied.
 * @param   sleep
 *              Non-zero to put device into full sleep.
 * @param   disable_gx
 *              Non-zero to disable gyro X.
 * @param   disable_gy
 *              Non-zero to disable gyro Y.
 * @param   disable_gz
 *              Non-zero to disable gyro Z.
 *
 * @return  INV_SUCCESS if successfull; a non-zero error code otherwise.
 */
static int mpu3050_pwr_mgmt(struct mldl_cfg *mldl_cfg,
			    void *mlsl_handle,
			    unsigned char reset,
			    unsigned char sleep,
			    unsigned char disable_gx,
			    unsigned char disable_gy,
			    unsigned char disable_gz)
{
	unsigned char b;
	int result;

	result =
	    inv_serial_read(mlsl_handle, mldl_cfg->addr, MPUREG_PWR_MGM, 1, &b);
	ERROR_CHECK(result);

	/* If we are awake, we need to put it in bypass before resetting */
	if ((!(b & BIT_SLEEP)) && reset)
		result = mpu_set_i2c_bypass(mldl_cfg, mlsl_handle, 1);

	/* Reset if requested */
	if (reset) {
		MPL_LOGV("Reset MPU3050\n");
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGM,
						 b | BIT_H_RESET);
		ERROR_CHECK(result);
		inv_sleep(5);
		mldl_cfg->gyro_needs_reset = FALSE;
		/* Some chips are awake after reset and some are asleep,
		 * check the status */
		result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
					 MPUREG_PWR_MGM, 1, &b);
		ERROR_CHECK(result);
	}

	/* Update the suspended state just in case we return early */
	if (b & BIT_SLEEP)
		mldl_cfg->gyro_is_suspended = TRUE;
	else
		mldl_cfg->gyro_is_suspended = FALSE;

	/* if power status match requested, nothing else's left to do */
	if ((b & (BIT_SLEEP | BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)) ==
	    (((sleep != 0) * BIT_SLEEP) |
	     ((disable_gx != 0) * BIT_STBY_XG) |
	     ((disable_gy != 0) * BIT_STBY_YG) |
	     ((disable_gz != 0) * BIT_STBY_ZG))) {
		return INV_SUCCESS;
	}

	/*
	 * This specific transition between states needs to be reinterpreted:
	 *    (1,1,1,1) -> (0,1,1,1) has to become
	 *    (1,1,1,1) -> (1,0,0,0) -> (0,1,1,1)
	 * where
	 *    (1,1,1,1) is (sleep=1,disable_gx=1,disable_gy=1,disable_gz=1)
	 */
	if ((b & (BIT_SLEEP | BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)) ==
	    (BIT_SLEEP | BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)
	    && ((!sleep) && disable_gx && disable_gy && disable_gz)) {
		result = mpu3050_pwr_mgmt(mldl_cfg, mlsl_handle, 0, 1, 0, 0, 0);
		if (result)
			return result;
		b |= BIT_SLEEP;
		b &= ~(BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG);
	}

	if ((b & BIT_SLEEP) != ((sleep != 0) * BIT_SLEEP)) {
		if (sleep) {
			result = mpu_set_i2c_bypass(mldl_cfg, mlsl_handle, 1);
			ERROR_CHECK(result);
			b |= BIT_SLEEP;
			result =
			    inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						    MPUREG_PWR_MGM, b);
			ERROR_CHECK(result);
			mldl_cfg->gyro_is_suspended = TRUE;
		} else {
			b &= ~BIT_SLEEP;
			result =
			    inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						    MPUREG_PWR_MGM, b);
			ERROR_CHECK(result);
			mldl_cfg->gyro_is_suspended = FALSE;
			inv_sleep(5);
		}
	}
	/*---
	  WORKAROUND FOR PUTTING GYRO AXIS in STAND-BY MODE
	  1) put one axis at a time in stand-by
	  ---*/
	if ((b & BIT_STBY_XG) != ((disable_gx != 0) * BIT_STBY_XG)) {
		b ^= BIT_STBY_XG;
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGM, b);
		ERROR_CHECK(result);
	}
	if ((b & BIT_STBY_YG) != ((disable_gy != 0) * BIT_STBY_YG)) {
		b ^= BIT_STBY_YG;
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGM, b);
		ERROR_CHECK(result);
	}
	if ((b & BIT_STBY_ZG) != ((disable_gz != 0) * BIT_STBY_ZG)) {
		b ^= BIT_STBY_ZG;
		result = inv_serial_single_write(mlsl_handle, mldl_cfg->addr,
						 MPUREG_PWR_MGM, b);
		ERROR_CHECK(result);
	}

	return INV_SUCCESS;
}

#endif

/**
 *  @brief  sets the clock source for the gyros.
 *  @param  mldl_cfg
 *              a pointer to the struct mldl_cfg data structure.
 *  @param  gyro_handle
 *              an handle to the serial device the gyro is assigned to.
 *  @return ML_SUCCESS if successful, a non-zero error code otherwise.
 */
static int mpu_set_clock_source(void *gyro_handle, struct mldl_cfg *mldl_cfg)
{
	int result;
	unsigned char cur_clk_src;
	unsigned char reg;

	/* clock source selection */
	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_PWR_MGM, 1, &reg);
	ERROR_CHECK(result);
	cur_clk_src = reg & BITS_CLKSEL;
	reg &= ~BITS_CLKSEL;

#ifdef CONFIG_MPU_SENSORS_MPU3050

	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_PWR_MGM,
					 mldl_cfg->clk_src | reg);
	ERROR_CHECK(result);
	/* TODO : workarounds to be determined and implemented */

#else				/* CONFIG_MPU_SENSORS_MPU6050XX */

	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_PWR_MGM,
					 mldl_cfg->clk_src | reg);
	ERROR_CHECK(result);

	/* ERRATA:
	   workaroud to switch from any MPU_CLK_SEL_PLLGYROx to
	   MPU_CLK_SEL_INTERNAL and XGyro is powered up:
	   1) Select INT_OSC
	   2) PD XGyro
	   3) PU XGyro
	 */
	if ((cur_clk_src == MPU_CLK_SEL_PLLGYROX
		 || cur_clk_src == MPU_CLK_SEL_PLLGYROY
		 || cur_clk_src == MPU_CLK_SEL_PLLGYROZ)
	    && mldl_cfg->clk_src == MPU_CLK_SEL_INTERNAL
	    && mldl_cfg->requested_sensors & INV_X_GYRO) {
		unsigned char first_result = INV_SUCCESS;
		mldl_cfg->requested_sensors &= ~INV_X_GYRO;
		result = mpu60xx_pwr_mgmt(mldl_cfg, gyro_handle,
					  FALSE, mldl_cfg->requested_sensors);
		ERROR_CHECK_FIRST(first_result, result);
		mldl_cfg->requested_sensors |= INV_X_GYRO;
		result = mpu60xx_pwr_mgmt(mldl_cfg, gyro_handle,
					  FALSE, mldl_cfg->requested_sensors);
		ERROR_CHECK_FIRST(first_result, result);
		result = first_result;
	}
#endif
	return result;
}

void mpu_print_cfg(struct mldl_cfg *mldl_cfg)
{
	struct mpu_platform_data *pdata = mldl_cfg->pdata;
	struct ext_slave_platform_data *accel = &mldl_cfg->pdata->accel;
	struct ext_slave_platform_data *compass = &mldl_cfg->pdata->compass;
	struct ext_slave_platform_data *pressure = &mldl_cfg->pdata->pressure;

	MPL_LOGI("mldl_cfg.addr             = %02x\n", mldl_cfg->addr);
	MPL_LOGI("mldl_cfg.int_config       = %02x\n", mldl_cfg->int_config);
	MPL_LOGI("mldl_cfg.ext_sync         = %02x\n", mldl_cfg->ext_sync);
	MPL_LOGI("mldl_cfg.full_scale       = %02x\n", mldl_cfg->full_scale);
	MPL_LOGI("mldl_cfg.lpf              = %02x\n", mldl_cfg->lpf);
	MPL_LOGI("mldl_cfg.clk_src          = %02x\n", mldl_cfg->clk_src);
	MPL_LOGI("mldl_cfg.divider          = %02x\n", mldl_cfg->divider);
	MPL_LOGI("mldl_cfg.dmp_enable       = %02x\n", mldl_cfg->dmp_enable);
	MPL_LOGI("mldl_cfg.fifo_enable      = %02x\n", mldl_cfg->fifo_enable);
	MPL_LOGI("mldl_cfg.dmp_cfg1         = %02x\n", mldl_cfg->dmp_cfg1);
	MPL_LOGI("mldl_cfg.dmp_cfg2         = %02x\n", mldl_cfg->dmp_cfg2);
	MPL_LOGI("mldl_cfg.offset_tc[0]     = %02x\n", mldl_cfg->offset_tc[0]);
	MPL_LOGI("mldl_cfg.offset_tc[1]     = %02x\n", mldl_cfg->offset_tc[1]);
	MPL_LOGI("mldl_cfg.offset_tc[2]     = %02x\n", mldl_cfg->offset_tc[2]);
	MPL_LOGI("mldl_cfg.silicon_revision = %02x\n",
		 mldl_cfg->silicon_revision);
	MPL_LOGI("mldl_cfg.product_revision = %02x\n",
		 mldl_cfg->product_revision);
	MPL_LOGI("mldl_cfg.gyro_sens_trim   = %02x\n",
		 mldl_cfg->gyro_sens_trim);
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || defined CONFIG_MPU_SENSORS_MPU6050B1
	MPL_LOGI("mldl_cfg.accel_sens_trim   = %02x\n",
		 mldl_cfg->accel_sens_trim);
#endif
	MPL_LOGI("mldl_cfg.requested_sensors= %04lx\n",
		 mldl_cfg->requested_sensors);

	if (mldl_cfg->accel) {
		MPL_LOGI("slave_accel->suspend      = %02x\n",
			 (int)mldl_cfg->accel->suspend);
		MPL_LOGI("slave_accel->resume       = %02x\n",
			 (int)mldl_cfg->accel->resume);
		MPL_LOGI("slave_accel->read         = %02x\n",
			 (int)mldl_cfg->accel->read);
		MPL_LOGI("slave_accel->type         = %02x\n",
			 mldl_cfg->accel->type);
		MPL_LOGI("slave_accel->reg          = %02x\n",
			 mldl_cfg->accel->read_reg);
		MPL_LOGI("slave_accel->len          = %02x\n",
			 mldl_cfg->accel->read_len);
		MPL_LOGI("slave_accel->endian       = %02x\n",
			 mldl_cfg->accel->endian);
		MPL_LOGI("slave_accel->range.mantissa= %02lx\n",
			 mldl_cfg->accel->range.mantissa);
		MPL_LOGI("slave_accel->range.fraction= %02lx\n",
			 mldl_cfg->accel->range.fraction);
	} else {
		MPL_LOGI("slave_accel               = NULL\n");
	}

	if (mldl_cfg->compass) {
		MPL_LOGI("slave_compass->suspend    = %02x\n",
			 (int)mldl_cfg->compass->suspend);
		MPL_LOGI("slave_compass->resume     = %02x\n",
			 (int)mldl_cfg->compass->resume);
		MPL_LOGI("slave_compass->read       = %02x\n",
			 (int)mldl_cfg->compass->read);
		MPL_LOGI("slave_compass->type       = %02x\n",
			 mldl_cfg->compass->type);
		MPL_LOGI("slave_compass->reg        = %02x\n",
			 mldl_cfg->compass->read_reg);
		MPL_LOGI("slave_compass->len        = %02x\n",
			 mldl_cfg->compass->read_len);
		MPL_LOGI("slave_compass->endian     = %02x\n",
			 mldl_cfg->compass->endian);
		MPL_LOGI("slave_compass->range.mantissa= %02lx\n",
			 mldl_cfg->compass->range.mantissa);
		MPL_LOGI("slave_compass->range.fraction= %02lx\n",
			 mldl_cfg->compass->range.fraction);

	} else {
		MPL_LOGI("slave_compass             = NULL\n");
	}

	if (mldl_cfg->pressure) {
		MPL_LOGI("slave_pressure->suspend    = %02x\n",
			 (int)mldl_cfg->pressure->suspend);
		MPL_LOGI("slave_pressure->resume     = %02x\n",
			 (int)mldl_cfg->pressure->resume);
		MPL_LOGI("slave_pressure->read       = %02x\n",
			 (int)mldl_cfg->pressure->read);
		MPL_LOGI("slave_pressure->type       = %02x\n",
			 mldl_cfg->pressure->type);
		MPL_LOGI("slave_pressure->reg        = %02x\n",
			 mldl_cfg->pressure->read_reg);
		MPL_LOGI("slave_pressure->len        = %02x\n",
			 mldl_cfg->pressure->read_len);
		MPL_LOGI("slave_pressure->endian     = %02x\n",
			 mldl_cfg->pressure->endian);
		MPL_LOGI("slave_pressure->range.mantissa= %02lx\n",
			 mldl_cfg->pressure->range.mantissa);
		MPL_LOGI("slave_pressure->range.fraction= %02lx\n",
			 mldl_cfg->pressure->range.fraction);

	} else {
		MPL_LOGI("slave_pressure             = NULL\n");
	}
	MPL_LOGI("accel->get_slave_descr    = %x\n",
		 (unsigned int)accel->get_slave_descr);
	MPL_LOGI("accel->irq                = %02x\n", accel->irq);
	MPL_LOGI("accel->adapt_num          = %02x\n", accel->adapt_num);
	MPL_LOGI("accel->bus                = %02x\n", accel->bus);
	MPL_LOGI("accel->address            = %02x\n", accel->address);
	MPL_LOGI("accel->orientation        =\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n",
		 accel->orientation[0], accel->orientation[1],
		 accel->orientation[2], accel->orientation[3],
		 accel->orientation[4], accel->orientation[5],
		 accel->orientation[6], accel->orientation[7],
		 accel->orientation[8]);
	MPL_LOGI("compass->get_slave_descr  = %x\n",
		 (unsigned int)compass->get_slave_descr);
	MPL_LOGI("compass->irq              = %02x\n", compass->irq);
	MPL_LOGI("compass->adapt_num        = %02x\n", compass->adapt_num);
	MPL_LOGI("compass->bus              = %02x\n", compass->bus);
	MPL_LOGI("compass->address          = %02x\n", compass->address);
	MPL_LOGI("compass->orientation      =\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n",
		 compass->orientation[0], compass->orientation[1],
		 compass->orientation[2], compass->orientation[3],
		 compass->orientation[4], compass->orientation[5],
		 compass->orientation[6], compass->orientation[7],
		 compass->orientation[8]);
	MPL_LOGI("pressure->get_slave_descr  = %x\n",
		 (unsigned int)pressure->get_slave_descr);
	MPL_LOGI("pressure->irq             = %02x\n", pressure->irq);
	MPL_LOGI("pressure->adapt_num       = %02x\n", pressure->adapt_num);
	MPL_LOGI("pressure->bus             = %02x\n", pressure->bus);
	MPL_LOGI("pressure->address         = %02x\n", pressure->address);
	MPL_LOGI("pressure->orientation     =\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n",
		 pressure->orientation[0], pressure->orientation[1],
		 pressure->orientation[2], pressure->orientation[3],
		 pressure->orientation[4], pressure->orientation[5],
		 pressure->orientation[6], pressure->orientation[7],
		 pressure->orientation[8]);

	MPL_LOGI("pdata->int_config         = %02x\n", pdata->int_config);
	MPL_LOGI("pdata->level_shifter      = %02x\n", pdata->level_shifter);
	MPL_LOGI("pdata->orientation        =\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n"
		 "                            %2d %2d %2d\n",
		 pdata->orientation[0], pdata->orientation[1],
		 pdata->orientation[2], pdata->orientation[3],
		 pdata->orientation[4], pdata->orientation[5],
		 pdata->orientation[6], pdata->orientation[7],
		 pdata->orientation[8]);

	MPL_LOGI("Struct sizes: mldl_cfg: %d, "
		 "ext_slave_descr:%d, "
		 "mpu_platform_data:%d: RamOffset: %d\n",
		 sizeof(struct mldl_cfg), sizeof(struct ext_slave_descr),
		 sizeof(struct mpu_platform_data),
		 offsetof(struct mldl_cfg, ram));
}

/**
 * @internal
 *
 * This fucntion configures the slaves by:
 * 1) Setting up the read
 *    a) Read Register
 *    b) Read Length
 * 2) Set up the data trigger (MPU6050 only)
 *    a) Set trigger write register
 *    b) Set Trigger write value
 * 3) Set up the divider (MPU6050 only)
 * 4) Set the slave bypass mode depending on slave
 *
 * @param mldl_cfg Handle to the configuration data
 * @param gyro_handle handle to the gyro communictation interface
 * @param slave Can be Null if turning off the slave
 * @param slave_pdata Can be null if turning off the slave
 *
 * @return INV_SUCCESS or non-zero error code
 */
int mpu_set_slave(struct mldl_cfg *mldl_cfg,
		  void *gyro_handle,
		  struct ext_slave_descr *slave,
		  struct ext_slave_platform_data *slave_pdata)
{
	int result;
	unsigned char reg;
	unsigned char slave_reg;
	unsigned char slave_len;
	unsigned char slave_endian;
	unsigned char slave_address;

	result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, TRUE);

	if (NULL == slave || NULL == slave_pdata) {
		slave_reg = 0;
		slave_len = 0;
		slave_endian = 0;
		slave_address = 0;
	} else {
		slave_reg = slave->read_reg;
		slave_len = slave->read_len;
		slave_endian = slave->endian;
		slave_address = slave_pdata->address;
#if defined CONFIG_MPU_SENSORS_MPU6050B1
		slave_address |= BIT_I2C_READ;
#endif
	}

	/* Address */
	result = inv_serial_single_write(gyro_handle,
					 mldl_cfg->addr,
					 MPUREG_AUX_SLV_ADDR, slave_address);
	ERROR_CHECK(result);
	/* Register */
	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_ACCEL_BURST_ADDR, 1, &reg);
	ERROR_CHECK(result);
	reg = ((reg & 0x80) | slave_reg);
	result = inv_serial_single_write(gyro_handle,
					 mldl_cfg->addr,
					 MPUREG_ACCEL_BURST_ADDR, reg);
	ERROR_CHECK(result);

#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	/* Length, byte swapping, grouping & enable */
	if (slave_len > BITS_SLV_LENG) {
		MPL_LOGW("Limiting slave burst read length to "
			 "the allowed maximum (15B, req. %d)\n", slave_len);
		slave_len = BITS_SLV_LENG;
		return INV_ERROR_INVALID_CONFIGURATION;
	}
	reg = slave_len;
	if (slave_endian == EXT_SLAVE_LITTLE_ENDIAN) {
		reg |= BIT_SLV_BYTE_SW;
		if (slave_reg & 1)
			reg |= BIT_SLV_GRP;
	}
	reg |= BIT_SLV_ENABLE;
	result = inv_serial_single_write(gyro_handle,
					 mldl_cfg->addr,
					 MPUREG_I2C_SLV0_CTRL, reg);
	ERROR_CHECK(result);

#if defined CONFIG_MPU_SENSORS_MPU6050B1
	/* Trigger */
	result = inv_serial_single_write(gyro_handle,
					 mldl_cfg->addr,
					 MPUREG_I2C_SLV2_ADDR,
					 slave_address & ~BIT_I2C_READ);
	ERROR_CHECK(result);
	if (slave && slave->trigger) {
		result = inv_serial_single_write(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV2_REG,
						 slave->trigger->reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV2_CTRL,
						 BIT_SLV_ENABLE | 0x01);
		ERROR_CHECK(result);
		result = inv_serial_single_write(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV2_DO,
						 slave->trigger->value);
		ERROR_CHECK(result);
	} else {
		result = inv_serial_single_write(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV2_CTRL, 0x00);
		ERROR_CHECK(result);
	}
	/* Data rate */
	if (slave) {
		struct ext_slave_config config;
		long data;
		long divider;
		config.key = MPU_SLAVE_CONFIG_ODR_RESUME;
		config.len = sizeof(long);
		config.apply = FALSE;
		config.data = &data;
		if (!(slave->get_config))
			return INV_ERROR_INVALID_CONFIGURATION;

		result = slave->get_config(NULL, slave, slave_pdata, &config);
		ERROR_CHECK(result);
		MPL_LOGI("Compass ODR: %ld Hz\n", data / 1000);
		divider = ((1000 * SAMPLING_RATE_HZ(mldl_cfg)) / data) - 1;
		if (divider > 0 && divider <= MASK_I2C_MST_DLY) {
			result = inv_serial_read(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV4_CTRL, 1, &reg);
			ERROR_CHECK(result);
			result = inv_serial_single_write(
			    gyro_handle,
			    mldl_cfg->addr,
			    MPUREG_I2C_MST_DELAY_CTRL,
			    (reg | BIT_SLV2_DLY_EN | BIT_SLV0_DLY_EN));

			ERROR_CHECK(result);
			result = inv_serial_read(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_SLV4_CTRL, 1, &reg);
			ERROR_CHECK(result);
			reg |= (unsigned char)(divider & MASK_I2C_MST_DLY);
			result = inv_serial_single_write(gyro_handle,
							 mldl_cfg->addr,
							 MPUREG_I2C_SLV4_CTRL,
							 reg);
			ERROR_CHECK(result);
		}
	} else {
		result = inv_serial_read(gyro_handle,
					 mldl_cfg->addr,
					 MPUREG_I2C_SLV4_CTRL, 1, &reg);
		ERROR_CHECK(result);
		result = inv_serial_single_write(gyro_handle,
						 mldl_cfg->addr,
						 MPUREG_I2C_MST_DELAY_CTRL,
						 (reg & ~(BIT_SLV2_DLY_EN |
							  BIT_SLV0_DLY_EN)));
	}

#endif

#else
	/* Length */
	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_USER_CTRL, 1, &reg);
	ERROR_CHECK(result);
	reg = (reg & ~BIT_AUX_RD_LENG);
	result = inv_serial_single_write(gyro_handle,
					 mldl_cfg->addr, MPUREG_USER_CTRL, reg);
	ERROR_CHECK(result);
#endif

	if (slave_address) {
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, FALSE);
		ERROR_CHECK(result);
	}
	return result;
}

/**
 * Check to see if the gyro was reset by testing a couple of registers known
 * to change on reset.
 *
 * @param mldl_cfg mldl configuration structure
 * @param gyro_handle handle used to communicate with the gyro
 *
 * @return INV_SUCCESS or non-zero error code
 */
static int mpu_was_reset(struct mldl_cfg *mldl_cfg, void *gyro_handle)
{
	int result = INV_SUCCESS;
	unsigned char reg;

	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_DMP_CFG_2, 1, &reg);
	ERROR_CHECK(result);

	if (mldl_cfg->dmp_cfg2 != reg)
		return TRUE;

	if (0 != mldl_cfg->dmp_cfg1)
		return FALSE;

	result = inv_serial_read(gyro_handle, mldl_cfg->addr,
				 MPUREG_SMPLRT_DIV, 1, &reg);
	ERROR_CHECK(result);
	if (reg != mldl_cfg->divider)
		return TRUE;

	if (0 != mldl_cfg->divider)
		return FALSE;

	/* Inconclusive assume it was reset */
	return TRUE;
}

static int gyro_resume(struct mldl_cfg *mldl_cfg, void *gyro_handle,
		       unsigned long sensors)
{
	int result;
	int ii;
	int jj;
	unsigned char reg;
	unsigned char regs[7];

	/* Wake up the part */
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	result = mpu60xx_pwr_mgmt(mldl_cfg, gyro_handle, FALSE, sensors);
	ERROR_CHECK(result);

	if (!mldl_cfg->gyro_needs_reset &&
	    !mpu_was_reset(mldl_cfg, gyro_handle)) {
		return INV_SUCCESS;
	}

	/* Configure the MPU */
	result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 1);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_INT_ENABLE,
					 (mldl_cfg->int_config));
	ERROR_CHECK(result);
#else
	result = mpu3050_pwr_mgmt(mldl_cfg, gyro_handle, FALSE, FALSE,
				  !(sensors & INV_X_GYRO),
				  !(sensors & INV_Y_GYRO),
				  !(sensors & INV_Z_GYRO));

	if (!mldl_cfg->gyro_needs_reset &&
	    !mpu_was_reset(mldl_cfg, gyro_handle)) {
		return INV_SUCCESS;
	}

	result = mpu3050_pwr_mgmt(mldl_cfg, gyro_handle, TRUE, FALSE,
				  !(sensors & INV_X_GYRO),
				  !(sensors & INV_Y_GYRO),
				  !(sensors & INV_Z_GYRO));
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_INT_CFG,
					 (mldl_cfg->int_config |
					  mldl_cfg->pdata->int_config));
	ERROR_CHECK(result);
#endif
	result = mpu_set_clock_source(gyro_handle, mldl_cfg);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_SMPLRT_DIV, mldl_cfg->divider);
	ERROR_CHECK(result);

#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	reg = MPUREG_GYRO_CONFIG_VALUE(0, 0, 0, mldl_cfg->full_scale);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_GYRO_CONFIG, reg);
	reg = MPUREG_CONFIG_VALUE(mldl_cfg->ext_sync, mldl_cfg->lpf);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_CONFIG, reg);
#else
	reg = DLPF_FS_SYNC_VALUE(mldl_cfg->ext_sync,
				 mldl_cfg->full_scale, mldl_cfg->lpf);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_DLPF_FS_SYNC, reg);
#endif
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_DMP_CFG_1, mldl_cfg->dmp_cfg1);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_DMP_CFG_2, mldl_cfg->dmp_cfg2);
	ERROR_CHECK(result);

	/* Write and verify memory */
	for (ii = 0; ii < MPU_MEM_NUM_RAM_BANKS; ii++) {
		unsigned char read[MPU_MEM_BANK_SIZE];

		result = inv_serial_write_mem(gyro_handle,
					      mldl_cfg->addr,
					      ((ii << 8) | 0x00),
					      MPU_MEM_BANK_SIZE,
					      mldl_cfg->ram[ii]);
		ERROR_CHECK(result);
		result = inv_serial_read_mem(gyro_handle, mldl_cfg->addr,
					     ((ii << 8) | 0x00),
					     MPU_MEM_BANK_SIZE, read);
		ERROR_CHECK(result);

#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
#define ML_SKIP_CHECK 38
#else
#define ML_SKIP_CHECK 20
#endif
		for (jj = 0; jj < MPU_MEM_BANK_SIZE; jj++) {
			/* skip the register memory locations */
			if (ii == 0 && jj < ML_SKIP_CHECK)
				continue;
			if (mldl_cfg->ram[ii][jj] != read[jj]) {
				result = INV_ERROR_SERIAL_WRITE;
				break;
			}
		}
		ERROR_CHECK(result);
	}

#ifdef CONFIG_MPU_SENSORS_MPU3050
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_XG_OFFS_TC,
					 mldl_cfg->offset_tc[0]);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_YG_OFFS_TC,
					 mldl_cfg->offset_tc[1]);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_ZG_OFFS_TC,
					 mldl_cfg->offset_tc[2]);
	ERROR_CHECK(result);
#else
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_XG_OFFS_TC,
					 ((mldl_cfg->offset_tc[0] << 1)
					  & BITS_XG_OFFS_TC));
	ERROR_CHECK(result);
	regs[0] = ((mldl_cfg->offset_tc[1] << 1) & BITS_YG_OFFS_TC);
#ifdef CONFIG_MPU_SENSORS_MPU6050B1
	if (mldl_cfg->pdata->level_shifter)
		regs[0] |= BIT_I2C_MST_VDDIO;
#endif
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_YG_OFFS_TC, regs[0]);
	ERROR_CHECK(result);
	result = inv_serial_single_write(gyro_handle, mldl_cfg->addr,
					 MPUREG_ZG_OFFS_TC,
					 ((mldl_cfg->offset_tc[2] << 1)
					  & BITS_ZG_OFFS_TC));
	ERROR_CHECK(result);
#endif
	regs[0] = MPUREG_X_OFFS_USRH;
	for (ii = 0; ii < DIM(mldl_cfg->offset); ii++) {
		regs[1 + ii * 2] = (unsigned char)(mldl_cfg->offset[ii] >> 8)
		    & 0xff;
		regs[1 + ii * 2 + 1] =
		    (unsigned char)(mldl_cfg->offset[ii] & 0xff);
	}
	result = inv_serial_write(gyro_handle, mldl_cfg->addr, 7, regs);
	ERROR_CHECK(result);

	/* Configure slaves */
	result = MLDLSetLevelShifterBit(mldl_cfg, gyro_handle,
					mldl_cfg->pdata->level_shifter);
	ERROR_CHECK(result);
	return result;
}

/*******************************************************************************
 *******************************************************************************
 * Exported functions
 *******************************************************************************
 ******************************************************************************/

/**
 * Initializes the pdata structure to defaults.
 *
 * Opens the device to read silicon revision, product id and whoami.
 *
 * @param mldl_cfg
 *          The internal device configuration data structure.
 * @param mlsl_handle
 *          The serial communication handle.
 *
 * @return INV_SUCCESS if silicon revision, product id and woami are supported
 *         by this software.
 */
int inv_mpu_open(struct mldl_cfg *mldl_cfg,
		 void *mlsl_handle,
		 void *accel_handle,
		 void *compass_handle, void *pressure_handle)
{
	int result;
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	int ii;
#endif
	/* Default is Logic HIGH, pushpull, latch disabled, anyread to clear */
	mldl_cfg->ignore_system_suspend = FALSE;
	mldl_cfg->int_config = BIT_DMP_INT_EN;
	mldl_cfg->clk_src = MPU_CLK_SEL_PLLGYROZ;
	mldl_cfg->lpf = MPU_FILTER_42HZ;
	mldl_cfg->full_scale = MPU_FS_2000DPS;
	mldl_cfg->divider = 4;
	mldl_cfg->dmp_enable = 1;
	mldl_cfg->fifo_enable = 1;
	mldl_cfg->ext_sync = 0;
	mldl_cfg->dmp_cfg1 = 0;
	mldl_cfg->dmp_cfg2 = 0;
	mldl_cfg->dmp_is_running = FALSE;
	mldl_cfg->gyro_is_suspended = TRUE;
	mldl_cfg->accel_is_suspended = TRUE;
	mldl_cfg->compass_is_suspended = TRUE;
	mldl_cfg->pressure_is_suspended = TRUE;
	mldl_cfg->gyro_needs_reset = FALSE;
	if (mldl_cfg->addr == 0) {
#ifdef __KERNEL__
		return INV_ERROR_INVALID_PARAMETER;
#else
		mldl_cfg->addr = 0x68;
#endif
	}

	/*
	 * Reset,
	 * Take the DMP out of sleep, and
	 * read the product_id, silicon rev and whoami
	 */
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	mldl_cfg->gyro_is_bypassed = FALSE;
	result = mpu60xx_pwr_mgmt(mldl_cfg, mlsl_handle, TRUE,
				  INV_THREE_AXIS_GYRO);
#else
	mldl_cfg->gyro_is_bypassed = TRUE;
	result = mpu3050_pwr_mgmt(mldl_cfg, mlsl_handle, RESET, 0, 0, 0, 0);
#endif
	ERROR_CHECK(result);

	result = inv_get_silicon_rev(mldl_cfg, mlsl_handle);
	ERROR_CHECK(result);

	/* Get the factory temperature compensation offsets */
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_XG_OFFS_TC, 1, &mldl_cfg->offset_tc[0]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_YG_OFFS_TC, 1, &mldl_cfg->offset_tc[1]);
	ERROR_CHECK(result);
	result = inv_serial_read(mlsl_handle, mldl_cfg->addr,
				 MPUREG_ZG_OFFS_TC, 1, &mldl_cfg->offset_tc[2]);
	ERROR_CHECK(result);

	/* Into bypass mode before sleeping and calling the slaves init */
	result = mpu_set_i2c_bypass(mldl_cfg, mlsl_handle, TRUE);
	ERROR_CHECK(result);

#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
	for (ii = 0; ii < GYRO_NUM_AXES; ii++) {
		mldl_cfg->offset_tc[ii] =
		    (mldl_cfg->offset_tc[ii] & BITS_XG_OFFS_TC) >> 1;
	}
	result = mpu60xx_pwr_mgmt(mldl_cfg, mlsl_handle, FALSE, 0);
#else
	result = mpu3050_pwr_mgmt(mldl_cfg, mlsl_handle, 0, SLEEP, 0, 0, 0);
#endif
	ERROR_CHECK(result);

	if (0) //if (mldl_cfg->accel && mldl_cfg->accel->init) 
   {
		result = mldl_cfg->accel->init(accel_handle,
					       mldl_cfg->accel,
					       &mldl_cfg->pdata->accel);
		ERROR_CHECK(result);
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
		if (mldl_cfg->accel->id == ACCEL_ID_MPU6050) {
			struct ext_slave_config config;
			config.key = MPU_SLAVE_CONFIG_INTERNAL_REFERENCE;
			config.len = sizeof(struct mldl_cfg *);
			config.apply = TRUE;
			config.data = mldl_cfg;
			result = mldl_cfg->accel->config(accel_handle,
						mldl_cfg->accel,
						&mldl_cfg->pdata->accel,
						&config);
		}
#endif
	}

	if (mldl_cfg->compass && mldl_cfg->compass->init) {
		result = mldl_cfg->compass->init(compass_handle,
						 mldl_cfg->compass,
						 &mldl_cfg->pdata->compass);
		if (INV_SUCCESS != result) {
			MPL_LOGE("mldl_cfg->compass->init returned %d\n",
				 result);
			goto out_accel;
		}
	}
	if (mldl_cfg->pressure && mldl_cfg->pressure->init) {
		result = mldl_cfg->pressure->init(pressure_handle,
						  mldl_cfg->pressure,
						  &mldl_cfg->pdata->pressure);
		if (INV_SUCCESS != result) {
			MPL_LOGE("mldl_cfg->pressure->init returned %d\n",
				 result);
			goto out_compass;
		}
	}

	mldl_cfg->requested_sensors = INV_THREE_AXIS_GYRO;
	if (mldl_cfg->accel && mldl_cfg->accel->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_ACCEL;

	if (mldl_cfg->compass && mldl_cfg->compass->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_COMPASS;

	if (mldl_cfg->pressure && mldl_cfg->pressure->resume)
		mldl_cfg->requested_sensors |= INV_THREE_AXIS_PRESSURE;

	return result;

 out_compass:
	if (mldl_cfg->compass->init)
		mldl_cfg->compass->exit(compass_handle,
					mldl_cfg->compass,
					&mldl_cfg->pdata->compass);
 out_accel:
	if (mldl_cfg->accel->init)
		mldl_cfg->accel->exit(accel_handle,
				      mldl_cfg->accel, &mldl_cfg->pdata->accel);
	return result;
}

/**
 * Close the mpu interface
 *
 * @param mldl_cfg pointer to the configuration structure
 * @param mlsl_handle pointer to the serial layer handle
 *
 * @return INV_SUCCESS or non-zero error code
 */
int inv_mpu_close(struct mldl_cfg *mldl_cfg,
		  void *mlsl_handle,
		  void *accel_handle,
		  void *compass_handle,
		  void *pressure_handle)
{
	int result = INV_SUCCESS;
	int ret_result = INV_SUCCESS;

	if (mldl_cfg->accel && mldl_cfg->accel->exit) {
		result = mldl_cfg->accel->exit(accel_handle,
					       mldl_cfg->accel,
					       &mldl_cfg->pdata->accel);
		if (INV_SUCCESS != result)
			MPL_LOGE("Accel exit failed %d\n", result);
		ret_result = result;
	}
	if (INV_SUCCESS == ret_result)
		ret_result = result;

	if (mldl_cfg->compass && mldl_cfg->compass->exit) {
		result = mldl_cfg->compass->exit(compass_handle,
						 mldl_cfg->compass,
						 &mldl_cfg->pdata->compass);
		if (INV_SUCCESS != result)
			MPL_LOGE("Compass exit failed %d\n", result);
	}
	if (INV_SUCCESS == ret_result)
		ret_result = result;

	if (mldl_cfg->pressure && mldl_cfg->pressure->exit) {
		result = mldl_cfg->pressure->exit(pressure_handle,
						  mldl_cfg->pressure,
						  &mldl_cfg->pdata->pressure);
		if (INV_SUCCESS != result)
			MPL_LOGE("Pressure exit failed %d\n", result);
	}
	if (INV_SUCCESS == ret_result)
		ret_result = result;

	return ret_result;
}

/**
 *  @brief  resume the MPU device and all the other sensor
 *          devices from their low power state.
 *
 *  @param  mldl_cfg
 *              pointer to the configuration structure
 *  @param  gyro_handle
 *              the main file handle to the MPU device.
 *  @param  accel_handle
 *              an handle to the accelerometer device, if sitting
 *              onto a separate bus. Can match mlsl_handle if
 *              the accelerometer device operates on the same
 *              primary bus of MPU.
 *  @param  compass_handle
 *              an handle to the compass device, if sitting
 *              onto a separate bus. Can match mlsl_handle if
 *              the compass device operates on the same
 *              primary bus of MPU.
 *  @param  pressure_handle
 *              an handle to the pressure sensor device, if sitting
 *              onto a separate bus. Can match mlsl_handle if
 *              the pressure sensor device operates on the same
 *              primary bus of MPU.
 *  @param  resume_gyro
 *              whether resuming the gyroscope device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @param  resume_accel
 *              whether resuming the accelerometer device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @param  resume_compass
 *              whether resuming the compass device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @param  resume_pressure
 *              whether resuming the pressure sensor device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @return  INV_SUCCESS or a non-zero error code.
 */
int inv_mpu_resume(struct mldl_cfg *mldl_cfg,
		   void *gyro_handle,
		   void *accel_handle,
		   void *compass_handle,
		   void *pressure_handle,
		   unsigned long sensors)
{
	bool resume_dmp = sensors & INV_DMP_PROCESSOR;
	bool resume_gyro = sensors & INV_THREE_AXIS_GYRO;
	bool resume_accel = sensors & INV_THREE_AXIS_ACCEL;
	bool resume_compass = sensors & INV_THREE_AXIS_COMPASS;
	bool resume_pressure = sensors & INV_THREE_AXIS_PRESSURE;
	int result = INV_SUCCESS;

	mpu_print_cfg(mldl_cfg);

	if (resume_accel && ((!mldl_cfg->accel) || (!mldl_cfg->accel->resume)))
		return INV_ERROR_INVALID_PARAMETER;
	if (resume_compass &&
	    ((!mldl_cfg->compass) || (!mldl_cfg->compass->resume)))
		return INV_ERROR_INVALID_PARAMETER;
	if (resume_pressure &&
	    ((!mldl_cfg->pressure) || (!mldl_cfg->pressure->resume)))
		return INV_ERROR_INVALID_PARAMETER;

	if (resume_gyro && mldl_cfg->gyro_is_suspended) {
		result = gyro_resume(mldl_cfg, gyro_handle, sensors);
		ERROR_CHECK(result);
	}

	if (resume_accel && mldl_cfg->accel_is_suspended) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->accel.bus) {
			result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle,
						    TRUE);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->accel->resume(accel_handle,
						 mldl_cfg->accel,
						 &mldl_cfg->pdata->accel);
		ERROR_CHECK(result);
		mldl_cfg->accel_is_suspended = FALSE;
	}

	if (resume_dmp && !mldl_cfg->accel_is_suspended &&
	    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->accel.bus) {
		result = mpu_set_slave(mldl_cfg,
				       gyro_handle,
				       mldl_cfg->accel,
				       &mldl_cfg->pdata->accel);
		ERROR_CHECK(result);
	}

	if (resume_compass && mldl_cfg->compass_is_suspended) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->compass.bus) {
			result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle,
						    TRUE);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->compass->resume(compass_handle,
						   mldl_cfg->compass,
						   &mldl_cfg->pdata->compass);
		ERROR_CHECK(result);
		mldl_cfg->compass_is_suspended = FALSE;
	}

	if (resume_dmp && !mldl_cfg->compass_is_suspended &&
	    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->compass.bus) {
		result = mpu_set_slave(mldl_cfg,
				       gyro_handle,
				       mldl_cfg->compass,
				       &mldl_cfg->pdata->compass);
		ERROR_CHECK(result);
	}

	if (resume_pressure && mldl_cfg->pressure_is_suspended) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->pressure.bus) {
			result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle,
						    TRUE);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->pressure->resume(pressure_handle,
						    mldl_cfg->pressure,
						    &mldl_cfg->pdata->pressure);
		ERROR_CHECK(result);
		mldl_cfg->pressure_is_suspended = FALSE;
	}

	if (resume_dmp && !mldl_cfg->pressure_is_suspended &&
	    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->pressure.bus) {
		result = mpu_set_slave(mldl_cfg,
				       gyro_handle,
				       mldl_cfg->pressure,
				       &mldl_cfg->pdata->pressure);
		ERROR_CHECK(result);
	}

	/* Now start */
	if (resume_dmp) {
		result = dmp_start(mldl_cfg, gyro_handle);
		ERROR_CHECK(result);
	}

	return result;
}

/**
 *  @brief  suspend the MPU device and all the other sensor
 *          devices into their low power state.
 *  @param  mldl_cfg
 *              a pointer to the struct mldl_cfg internal data
 *              structure.
 *  @param  gyro_handle
 *              the main file handle to the MPU device.
 *  @param  accel_handle
 *              an handle to the accelerometer device, if sitting
 *              onto a separate bus. Can match gyro_handle if
 *              the accelerometer device operates on the same
 *              primary bus of MPU.
 *  @param  compass_handle
 *              an handle to the compass device, if sitting
 *              onto a separate bus. Can match gyro_handle if
 *              the compass device operates on the same
 *              primary bus of MPU.
 *  @param  pressure_handle
 *              an handle to the pressure sensor device, if sitting
 *              onto a separate bus. Can match gyro_handle if
 *              the pressure sensor device operates on the same
 *              primary bus of MPU.
 *  @param  suspend_gyro
 *              whether suspending the gyroscope device is
 *              actually needed.
 *  @param  suspend_accel
 *              whether suspending the accelerometer device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @param  suspend_compass
 *              whether suspending the compass device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @param  suspend_pressure
 *              whether suspending the pressure sensor device is
 *              actually needed (if the device supports low power
 *              mode of some sort).
 *  @return  INV_SUCCESS or a non-zero error code.
 */
int inv_mpu_suspend(struct mldl_cfg *mldl_cfg,
		    void *gyro_handle,
		    void *accel_handle,
		    void *compass_handle,
		    void *pressure_handle,
		    unsigned long sensors)
{
	int result = INV_SUCCESS;
	bool suspend_dmp = ((sensors & INV_DMP_PROCESSOR) == INV_DMP_PROCESSOR);
	bool suspend_gyro = ((sensors & (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO))
			     == (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO));
	bool suspend_accel = ((sensors & INV_THREE_AXIS_ACCEL) ==
			INV_THREE_AXIS_ACCEL);
	bool suspend_compass = ((sensors & INV_THREE_AXIS_COMPASS) ==
				INV_THREE_AXIS_COMPASS);
	bool suspend_pressure = ((sensors & INV_THREE_AXIS_PRESSURE) ==
				INV_THREE_AXIS_PRESSURE);

	if (suspend_dmp) {
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 1);
		ERROR_CHECK(result);
		result = dmp_stop(mldl_cfg, gyro_handle);
		ERROR_CHECK(result);
	}

	if (suspend_gyro && !mldl_cfg->gyro_is_suspended) {
#if defined CONFIG_MPU_SENSORS_MPU6050A2 || \
	defined CONFIG_MPU_SENSORS_MPU6050B1
		result = mpu60xx_pwr_mgmt(mldl_cfg, gyro_handle, FALSE,
					((~sensors) & INV_ALL_SENSORS));
#else
		result = mpu3050_pwr_mgmt(mldl_cfg, gyro_handle,
					  0, suspend_dmp && suspend_gyro,
					  (sensors & INV_X_GYRO),
					  (sensors & INV_Y_GYRO),
					  (sensors & INV_Z_GYRO));
#endif
		ERROR_CHECK(result);
	}

	if (!mldl_cfg->accel_is_suspended && suspend_accel &&
	    mldl_cfg->accel && mldl_cfg->accel->suspend) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->accel.bus) {
			result = mpu_set_slave(mldl_cfg, gyro_handle,
					       NULL, NULL);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->accel->suspend(accel_handle,
						  mldl_cfg->accel,
						  &mldl_cfg->pdata->accel);
		ERROR_CHECK(result);
		mldl_cfg->accel_is_suspended = TRUE;
	}

	if (!mldl_cfg->compass_is_suspended && suspend_compass &&
	    mldl_cfg->compass && mldl_cfg->compass->suspend) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->compass.bus) {
			result = mpu_set_slave(mldl_cfg, gyro_handle,
					       NULL, NULL);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->compass->suspend(compass_handle,
						    mldl_cfg->compass,
						    &mldl_cfg->pdata->compass);
		ERROR_CHECK(result);
		mldl_cfg->compass_is_suspended = TRUE;
	}

	if (!mldl_cfg->pressure_is_suspended && suspend_pressure &&
	    mldl_cfg->pressure && mldl_cfg->pressure->suspend) {
		if (!mldl_cfg->gyro_is_suspended &&
		    EXT_SLAVE_BUS_SECONDARY == mldl_cfg->pdata->pressure.bus) {
			result = mpu_set_slave(mldl_cfg, gyro_handle,
					       NULL, NULL);
			ERROR_CHECK(result);
		}
		result = mldl_cfg->pressure->suspend(
			pressure_handle,
			mldl_cfg->pressure,
			&mldl_cfg->pdata->pressure);
		ERROR_CHECK(result);
		mldl_cfg->pressure_is_suspended = TRUE;
	}
	return result;
}

int inv_mpu_slave_read(struct mldl_cfg *mldl_cfg,
		       void *gyro_handle,
		       void *slave_handle,
		       struct ext_slave_descr *slave,
		       struct ext_slave_platform_data *pdata,
		       unsigned char *data)
{
	int result;
	int bypass_result;
	int remain_bypassed = TRUE;

	if (NULL == slave || NULL == slave->read)
		ERROR_CHECK(INV_ERROR_INVALID_CONFIGURATION);

	if ((EXT_SLAVE_BUS_SECONDARY == pdata->bus)
	    && (!mldl_cfg->gyro_is_bypassed)) {
		remain_bypassed = FALSE;
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 1);
		ERROR_CHECK(result);
	}

	result = slave->read(slave_handle, slave, pdata, data);

	if (!remain_bypassed) {
		bypass_result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 0);
		ERROR_CHECK(bypass_result);
	}
	return result;
}

int inv_mpu_slave_config(struct mldl_cfg *mldl_cfg,
			 void *gyro_handle,
			 void *slave_handle,
			 struct ext_slave_config *data,
			 struct ext_slave_descr *slave,
			 struct ext_slave_platform_data *pdata)
{
	int result;
	int remain_bypassed = TRUE;

	if (NULL == slave || NULL == slave->config)
		ERROR_CHECK(INV_ERROR_INVALID_CONFIGURATION);

	if (data->apply && (EXT_SLAVE_BUS_SECONDARY == pdata->bus)
	    && (!mldl_cfg->gyro_is_bypassed)) {
		remain_bypassed = FALSE;
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 1);
		ERROR_CHECK(result);
	}

	result = slave->config(slave_handle, slave, pdata, data);
	ERROR_CHECK(result);

	if (!remain_bypassed) {
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 0);
		ERROR_CHECK(result);
	}
	return result;
}

int inv_mpu_get_slave_config(struct mldl_cfg *mldl_cfg,
			     void *gyro_handle,
			     void *slave_handle,
			     struct ext_slave_config *data,
			     struct ext_slave_descr *slave,
			     struct ext_slave_platform_data *pdata)
{
	int result;
	int remain_bypassed = TRUE;

	if (NULL == slave || NULL == slave->get_config)
		ERROR_CHECK(INV_ERROR_INVALID_CONFIGURATION);

	if (data->apply && (EXT_SLAVE_BUS_SECONDARY == pdata->bus)
	    && (!mldl_cfg->gyro_is_bypassed)) {
		remain_bypassed = FALSE;
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 1);
		ERROR_CHECK(result);
	}

	result = slave->get_config(slave_handle, slave, pdata, data);
	ERROR_CHECK(result);

	if (!remain_bypassed) {
		result = mpu_set_i2c_bypass(mldl_cfg, gyro_handle, 0);
		ERROR_CHECK(result);
	}
	return result;
}

/**
 * @}
 */
