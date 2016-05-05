/**
 * Configuration header file of the core driver API @file yas_cfg.h
 *
 * Copyright (c) 2013 Yamaha Corporation
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#ifndef __YAS_CFG_H__
#define __YAS_CFG_H__

#define YAS_MAG_DRIVER_NONE			(0) /*!< No Magnetometer */
#define YAS_MAG_DRIVER_YAS529			(1) /*!< YAS 529 (MS-3C) */
#define YAS_MAG_DRIVER_YAS530			(2) /*!< YAS 530 (MS-3E) */
#define YAS_MAG_DRIVER_YAS532			(3) /*!< YAS 532 (MS-3R) */
#define YAS_MAG_DRIVER_YAS533			(4) /*!< YAS 533 (MS-3F) */
#define YAS_MAG_DRIVER_YAS535			(5) /*!< YAS 535 (MS-6C) */
#define YAS_MAG_DRIVER_YAS536			(6) /*!< YAS 536 (MS-3W) */
#define YAS_MAG_DRIVER_YAS53x			(0x7fff) /*!< YAS XXX */

#define YAS_ACC_DRIVER_NONE			(0) /*!< No Accelerometer */
#define YAS_ACC_DRIVER_ADXL345			(1) /*!< ADXL 345 */
#define YAS_ACC_DRIVER_ADXL346			(2) /*!< ADXL 346 */
#define YAS_ACC_DRIVER_BMA150			(3) /*!< BMA 150 */
#define YAS_ACC_DRIVER_BMA222			(4) /*!< BMA 222 */
#define YAS_ACC_DRIVER_BMA222E			(5) /*!< BMA 222E */
#define YAS_ACC_DRIVER_BMA250			(6) /*!< BMA 250 */
#define YAS_ACC_DRIVER_BMA250E			(7) /*!< BMA 250E */
#define YAS_ACC_DRIVER_BMA254			(8) /*!< BMA 254 */
#define YAS_ACC_DRIVER_BMI055			(9) /*!< BMI 055 */
#define YAS_ACC_DRIVER_BMI058			(10) /*!< BMI 058 */
#define YAS_ACC_DRIVER_DMARD08			(11) /*!< DMARD08 */
#define YAS_ACC_DRIVER_KXSD9			(12) /*!< KXSD9 */
#define YAS_ACC_DRIVER_KXTE9			(13) /*!< KXTE9 */
#define YAS_ACC_DRIVER_KXTF9			(14) /*!< KXTF9 */
#define YAS_ACC_DRIVER_KXTI9			(15) /*!< KXTI9 */
#define YAS_ACC_DRIVER_KXTJ2			(16) /*!< KXTJ2 */
#define YAS_ACC_DRIVER_KXUD9			(17) /*!< KXUD9 */
#define YAS_ACC_DRIVER_LIS331DL			(18) /*!< LIS331DL */
#define YAS_ACC_DRIVER_LIS331DLH		(19) /*!< LIS331DLH */
#define YAS_ACC_DRIVER_LIS331DLM		(20) /*!< LIS331DLM */
#define YAS_ACC_DRIVER_LIS3DH			(21) /*!< LIS3DH */
#define YAS_ACC_DRIVER_LSM330DLC		(22) /*!< LSM330DLC */
#define YAS_ACC_DRIVER_MMA8452Q			(23) /*!< MMA8452Q */
#define YAS_ACC_DRIVER_MMA8453Q			(24) /*!< MMA8453Q */
#define YAS_ACC_DRIVER_U2DH			(25) /*!< U2DH */
#define YAS_ACC_DRIVER_YAS535			(26) /*!< YAS 535 (MS-6C) */
#define YAS_ACC_DRIVER_YAS53x			(0x7fff) /*!< YAS XXX */

#define YAS_GYRO_DRIVER_NONE			(0) /*!< No Gyroscope */
#define YAS_GYRO_DRIVER_BMI055			(1) /*!< BMI055 */
#define YAS_GYRO_DRIVER_BMI058			(2) /*!< BMI058 */
#define YAS_GYRO_DRIVER_EWTZMU			(3) /*!< EWTZMU */
#define YAS_GYRO_DRIVER_ITG3200			(4) /*!< ITG3200 */
#define YAS_GYRO_DRIVER_ITG3500			(5) /*!< ITG3500 */
#define YAS_GYRO_DRIVER_L3G3200D		(6) /*!< L3G3200D */
#define YAS_GYRO_DRIVER_L3G4200D		(7) /*!< L3G4200D */
#define YAS_GYRO_DRIVER_LSM330DLC		(8) /*!< LSM330DLC */
#define YAS_GYRO_DRIVER_MPU3050			(9) /*!< MPU3050 */
#define YAS_GYRO_DRIVER_MPU6050			(10) /*!< MPU6050 */
#define YAS_GYRO_DRIVER_YAS53x			(0x7fff) /*!< YAS XXX */

/*----------------------------------------------------------------------------
 *                               Configuration
 *----------------------------------------------------------------------------*/

#define YAS_ACC_DRIVER				(YAS_ACC_DRIVER_BMA250E)
#define YAS_MAG_DRIVER				(YAS_MAG_DRIVER_YAS532)
#define YAS_GYRO_DRIVER				(YAS_GYRO_DRIVER_NONE)

/*! Magnetic minimum calibration enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_MINI_ENABLE		(0)
/*! Magnetic floating point calibration enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_FLOAT_ENABLE		(1)
/*! Magnetic sphere calibration enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_SPHERE_ENABLE		(1)
/*! Magnetic ellipsoid calibration enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_ELLIPSOID_ENABLE		(1)
/*! Magnetic calibration with gyroscope enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_WITH_GYRO_ENABLE		(1)
#if YAS_MAG_CALIB_MINI_ENABLE
#undef YAS_MAG_CALIB_FLOAT_ENABLE
#undef YAS_MAG_CALIB_SPHERE_ENABLE
#undef YAS_MAG_CALIB_ELLIPSOID_ENABLE
#undef YAS_MAG_CALIB_WITH_GYRO_ENABLE
#define YAS_MAG_CALIB_FLOAT_ENABLE		(0)
#define YAS_MAG_CALIB_SPHERE_ENABLE		(0)
#define YAS_MAG_CALIB_ELLIPSOID_ENABLE		(0)
#define YAS_MAG_CALIB_WITH_GYRO_ENABLE		(0)
#endif
/*! Magnetic calibration enable (0:Disable, 1: Enable) */
#define YAS_MAG_CALIB_ENABLE	(YAS_MAG_CALIB_FLOAT_ENABLE | \
		YAS_MAG_CALIB_MINI_ENABLE | \
		YAS_MAG_CALIB_SPHERE_ENABLE | \
		YAS_MAG_CALIB_ELLIPSOID_ENABLE | \
		YAS_MAG_CALIB_WITH_GYRO_ENABLE)

/*! Gyroscope calibration enable (0:Disable, 1: Enable) */
#define YAS_GYRO_CALIB_ENABLE			(1)
/*! Magnetic filter enable (0:Disable, 1: Enable) */
#define YAS_MAG_FILTER_ENABLE			(1)
/*! Fusion with gyroscope enable (0:Disable, 1: Enable) */
#define YAS_FUSION_ENABLE			(1)
/*! Fusion with gyroscope enable (0:Disable, 1: Enable) */
#define YAS_FUSION_WITH_GYRO_ENABLE		(1)
/*! Quaternion (gyroscope) enable (0:Disable, 1: Enable) */
#define YAS_GAMEVEC_ENABLE			(1)
/*! Magnetic average filter enable (0:Disable, 1:Enable) */
#define YAS_MAG_AVERAGE_FILTER_ENABLE		(0)
/*! Software gyroscope enable (0:Disable, 1:Enable) */
#define YAS_SOFTWARE_GYROSCOPE_ENABLE		(0)
/*! Log enable (0:Disable, 1:Enable) */
#define YAS_LOG_ENABLE				(0)
/*! Orientation enable (0:Disable, 1:Enable) */
#define YAS_ORIENTATION_ENABLE			(1)

/*! Mangetic vdd in mV */
#define YAS_MAG_VCORE				(2700)

/*! No sleep version of YAS532 driver */
#define YAS532_DRIVER_NO_SLEEP			(0)

/* ----------------------------------------------------------------------------
 *                            Driver Configuration
 *--------------------------------------------------------------------------- */
/*! Default sensor delay in [msec] */
#define YAS_DEFAULT_SENSOR_DELAY		(50)

/* ----------------------------------------------------------------------------
 *                      Geomagnetic Filter Configuration
 *--------------------------------------------------------------------------- */

/*! Geomagnetic adaptive filter noise threshold X (dispersion in [nT]) */
#define YAS_MAG_DEFAULT_FILTER_NOISE_X		(1200)
/*! Geomagnetic adaptive filter noise threshold Y (dispersion in [nT]) */
#define YAS_MAG_DEFAULT_FILTER_NOISE_Y		(1200)
/*! Geomagnetic adaptive filter noise threshold Z (dispersion in [nT]) */
#define YAS_MAG_DEFAULT_FILTER_NOISE_Z		(1200)
/*! Geomagnetic adaptive filter length */
#define YAS_MAG_DEFAULT_FILTER_LEN		(20)
/*! Geomagnetic threshold filter threshold in [nT] */
#define YAS_MAG_DEFAULT_FILTER_THRESH		(300)

/* ----------------------------------------------------------------------------
 *                           Other Configuration
 *--------------------------------------------------------------------------- */

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_NONE
#undef YAS_MAG_CALIB_ENABLE
#define YAS_MAG_CALIB_ENABLE			(0)
#undef YAS_MAG_FILTER_ENABLE
#define YAS_MAG_FILTER_ENABLE			(0)
#endif
#if YAS_MAG_DRIVER != YAS_MAG_DRIVER_YAS536
#undef YAS_MAG_AVERAGE_FILTER_ENABLE
#define YAS_MAG_AVERAGE_FILTER_ENABLE		(0)
#endif

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_NONE \
		    || YAS_ACC_DRIVER == YAS_ACC_DRIVER_NONE
#undef YAS_SOFTWARE_GYROSCOPE_ENABLE
#define YAS_SOFTWARE_GYROSCOPE_ENABLE		(0)
#undef YAS_FUSION_ENABLE
#define YAS_FUSION_ENABLE			(0)
#endif

#if YAS_GYRO_DRIVER == YAS_GYRO_DRIVER_NONE \
		     || YAS_MAG_DRIVER == YAS_MAG_DRIVER_NONE
#undef YAS_GYRO_CALIB_ENABLE
#define YAS_GYRO_CALIB_ENABLE			(0)
#endif

#if YAS_GYRO_DRIVER == YAS_GYRO_DRIVER_NONE
#undef YAS_FUSION_WITH_GYRO_ENABLE
#define YAS_FUSION_WITH_GYRO_ENABLE		(0)
#endif

#if !YAS_FUSION_ENABLE
#undef YAS_FUSION_WITH_GYRO_ENABLE
#define YAS_FUSION_WITH_GYRO_ENABLE		(0)
#endif

#if YAS_LOG_ENABLE
#ifdef __KERNEL__
#undef YAS_LOG_ENABLE
#define YAS_LOG_ENABLE				(0)
#else
#include <stdio.h>
#include <string.h>
#endif
#endif

/*! yas magnetometer name */
#define YAS_MAG_NAME		"yas_magnetometer"
/*! yas accelerometer name */
#define YAS_ACC_NAME		"yas_accelerometer"
/*! yas gyroscope name */
#define YAS_GYRO_NAME		"yas_gyroscope"

#endif
