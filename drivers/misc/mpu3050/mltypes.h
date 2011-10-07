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
 *  @defgroup MLERROR
 *  @brief  Motion Library - Error definitions.
 *          Definition of the error codes used within the MPL and
 *          returned to the user.
 *          Every function tries to return a meaningful error code basing
 *          on the occuring error condition. The error code is numeric.
 *
 *          The available error codes and their associated values are:
 *          - (0)       INV_SUCCESS
 *          - (1)       INV_ERROR
 *          - (2)       INV_ERROR_INVALID_PARAMETER
 *          - (3)       INV_ERROR_FEATURE_NOT_ENABLED
 *          - (4)       INV_ERROR_FEATURE_NOT_IMPLEMENTED
 *          - (6)       INV_ERROR_DMP_NOT_STARTED
 *          - (7)       INV_ERROR_DMP_STARTED
 *          - (8)       INV_ERROR_NOT_OPENED
 *          - (9)       INV_ERROR_OPENED
 *          - (10)      INV_ERROR_INVALID_MODULE
 *          - (11)      INV_ERROR_MEMORY_EXAUSTED
 *          - (12)      INV_ERROR_DIVIDE_BY_ZERO
 *          - (13)      INV_ERROR_ASSERTION_FAILURE
 *          - (14)      INV_ERROR_FILE_OPEN
 *          - (15)      INV_ERROR_FILE_READ
 *          - (16)      INV_ERROR_FILE_WRITE
 *          - (17)      INV_ERROR_INVALID_CONFIGURATION
 *          - (20)      INV_ERROR_SERIAL_CLOSED
 *          - (21)      INV_ERROR_SERIAL_OPEN_ERROR
 *          - (22)      INV_ERROR_SERIAL_READ
 *          - (23)      INV_ERROR_SERIAL_WRITE
 *          - (24)      INV_ERROR_SERIAL_DEVICE_NOT_RECOGNIZED
 *          - (25)      INV_ERROR_SM_TRANSITION
 *          - (26)      INV_ERROR_SM_IMPROPER_STATE
 *          - (30)      INV_ERROR_FIFO_OVERFLOW
 *          - (31)      INV_ERROR_FIFO_FOOTER
 *          - (32)      INV_ERROR_FIFO_READ_COUNT
 *          - (33)      INV_ERROR_FIFO_READ_DATA
 *          - (40)      INV_ERROR_MEMORY_SET
 *          - (50)      INV_ERROR_LOG_MEMORY_ERROR
 *          - (51)      INV_ERROR_LOG_OUTPUT_ERROR
 *          - (60)      INV_ERROR_OS_BAD_PTR
 *          - (61)      INV_ERROR_OS_BAD_HANDLE
 *          - (62)      INV_ERROR_OS_CREATE_FAILED
 *          - (63)      INV_ERROR_OS_LOCK_FAILED
 *          - (70)      INV_ERROR_COMPASS_DATA_OVERFLOW
 *          - (71)      INV_ERROR_COMPASS_DATA_UNDERFLOW
 *          - (72)      INV_ERROR_COMPASS_DATA_NOT_READY
 *          - (73)      INV_ERROR_COMPASS_DATA_ERROR
 *          - (75)      INV_ERROR_CALIBRATION_LOAD
 *          - (76)      INV_ERROR_CALIBRATION_STORE
 *          - (77)      INV_ERROR_CALIBRATION_LEN
 *          - (78)      INV_ERROR_CALIBRATION_CHECKSUM
 *          - (79)      INV_ERROR_ACCEL_DATA_OVERFLOW
 *          - (80)      INV_ERROR_ACCEL_DATA_UNDERFLOW
 *          - (81)      INV_ERROR_ACCEL_DATA_NOT_READY
 *          - (82)      INV_ERROR_ACCEL_DATA_ERROR
 *
 *  @{
 *      @file mltypes.h
 *  @}
 */

#ifndef MLTYPES_H
#define MLTYPES_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include "stdint_invensense.h"
#endif
#include "log.h"

/*---------------------------
    ML Types
---------------------------*/

/**
 *  @struct inv_error_t mltypes.h "mltypes"
 *  @brief  The MPL Error Code return type.
 *
 *  @code
 *      typedef unsigned char inv_error_t;
 *  @endcode
 */
typedef unsigned char inv_error_t;

#if defined(LINUX) || defined(__KERNEL__)
typedef unsigned int HANDLE;
#endif

#ifdef __KERNEL__
typedef HANDLE FILE;
#endif

#ifndef __cplusplus
#ifndef __KERNEL__
typedef int_fast8_t bool;
#endif
#endif

/*---------------------------
    ML Defines
---------------------------*/

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Dimension of an array */
#ifndef DIM
#define DIM(array) (sizeof(array)/sizeof((array)[0]))
#endif

/* - ML Errors. - */
#define ERROR_NAME(x)   (#x)

static inline unsigned char __ml_error_check(inv_error_t result,
					     const char *file,
					     const char *func, int line)
{
	if (0 != result)
		MPL_LOGE("%s|%s|%d returning %d\n", file, func, line, result);

	return result;
}

#define ERROR_CHECK(x) \
	do {								\
		if (INV_SUCCESS != x) {					\
			return __ml_error_check((inv_error_t)x, __FILE__,\
					__func__, __LINE__);		\
		}							\
	} while (0)

#define ERROR_CHECK_FIRST(first, x)                                     \
	{ if (INV_SUCCESS == first) first = x; }

#define INV_SUCCESS                       (0)
/* Generic Error code.  Proprietary Error Codes only */
#define INV_ERROR                         (1)

/* Compatibility and other generic error codes */
#define INV_ERROR_INVALID_PARAMETER       (2)
#define INV_ERROR_FEATURE_NOT_ENABLED     (3)
#define INV_ERROR_FEATURE_NOT_IMPLEMENTED (4)
#define INV_ERROR_DMP_NOT_STARTED         (6)
#define INV_ERROR_DMP_STARTED             (7)
#define INV_ERROR_NOT_OPENED              (8)
#define INV_ERROR_OPENED                  (9)
#define INV_ERROR_INVALID_MODULE         (10)
#define INV_ERROR_MEMORY_EXAUSTED        (11)
#define INV_ERROR_DIVIDE_BY_ZERO         (12)
#define INV_ERROR_ASSERTION_FAILURE      (13)
#define INV_ERROR_FILE_OPEN              (14)
#define INV_ERROR_FILE_READ              (15)
#define INV_ERROR_FILE_WRITE             (16)
#define INV_ERROR_INVALID_CONFIGURATION  (17)

/* Serial Communication */
#define INV_ERROR_SERIAL_CLOSED          (20)
#define INV_ERROR_SERIAL_OPEN_ERROR      (21)
#define INV_ERROR_SERIAL_READ            (22)
#define INV_ERROR_SERIAL_WRITE           (23)
#define INV_ERROR_SERIAL_DEVICE_NOT_RECOGNIZED  (24)

/* SM = State Machine */
#define INV_ERROR_SM_TRANSITION          (25)
#define INV_ERROR_SM_IMPROPER_STATE      (26)

/* Fifo */
#define INV_ERROR_FIFO_OVERFLOW          (30)
#define INV_ERROR_FIFO_FOOTER            (31)
#define INV_ERROR_FIFO_READ_COUNT        (32)
#define INV_ERROR_FIFO_READ_DATA         (33)

/* Memory & Registers, Set & Get */
#define INV_ERROR_MEMORY_SET             (40)

#define INV_ERROR_LOG_MEMORY_ERROR       (50)
#define INV_ERROR_LOG_OUTPUT_ERROR       (51)

/* OS interface errors */
#define INV_ERROR_OS_BAD_PTR             (60)
#define INV_ERROR_OS_BAD_HANDLE          (61)
#define INV_ERROR_OS_CREATE_FAILED       (62)
#define INV_ERROR_OS_LOCK_FAILED         (63)

/* Compass errors */
#define INV_ERROR_COMPASS_DATA_OVERFLOW  (70)
#define INV_ERROR_COMPASS_DATA_UNDERFLOW (71)
#define INV_ERROR_COMPASS_DATA_NOT_READY (72)
#define INV_ERROR_COMPASS_DATA_ERROR     (73)

/* Load/Store calibration */
#define INV_ERROR_CALIBRATION_LOAD       (75)
#define INV_ERROR_CALIBRATION_STORE      (76)
#define INV_ERROR_CALIBRATION_LEN        (77)
#define INV_ERROR_CALIBRATION_CHECKSUM   (78)

/* Accel errors */
#define INV_ERROR_ACCEL_DATA_OVERFLOW    (79)
#define INV_ERROR_ACCEL_DATA_UNDERFLOW   (80)
#define INV_ERROR_ACCEL_DATA_NOT_READY   (81)
#define INV_ERROR_ACCEL_DATA_ERROR       (82)

#ifdef INV_USE_LEGACY_NAMES
#define ML_SUCCESS                       INV_SUCCESS
#define ML_ERROR                         INV_ERROR
#define ML_ERROR_INVALID_PARAMETER       INV_ERROR_INVALID_PARAMETER
#define ML_ERROR_FEATURE_NOT_ENABLED     INV_ERROR_FEATURE_NOT_ENABLED
#define ML_ERROR_FEATURE_NOT_IMPLEMENTED INV_ERROR_FEATURE_NOT_IMPLEMENTED
#define ML_ERROR_DMP_NOT_STARTED         INV_ERROR_DMP_NOT_STARTED
#define ML_ERROR_DMP_STARTED             INV_ERROR_DMP_STARTED
#define ML_ERROR_NOT_OPENED              INV_ERROR_NOT_OPENED
#define ML_ERROR_OPENED                  INV_ERROR_OPENED
#define ML_ERROR_INVALID_MODULE          INV_ERROR_INVALID_MODULE
#define ML_ERROR_MEMORY_EXAUSTED         INV_ERROR_MEMORY_EXAUSTED
#define ML_ERROR_DIVIDE_BY_ZERO          INV_ERROR_DIVIDE_BY_ZERO
#define ML_ERROR_ASSERTION_FAILURE       INV_ERROR_ASSERTION_FAILURE
#define ML_ERROR_FILE_OPEN               INV_ERROR_FILE_OPEN
#define ML_ERROR_FILE_READ               INV_ERROR_FILE_READ
#define ML_ERROR_FILE_WRITE              INV_ERROR_FILE_WRITE
#define ML_ERROR_INVALID_CONFIGURATION   INV_ERROR_INVALID_CONFIGURATION
#define ML_ERROR_SERIAL_CLOSED           INV_ERROR_SERIAL_CLOSED
#define ML_ERROR_SERIAL_OPEN_ERROR       INV_ERROR_SERIAL_OPEN_ERROR
#define ML_ERROR_SERIAL_READ             INV_ERROR_SERIAL_READ
#define ML_ERROR_SERIAL_WRITE            INV_ERROR_SERIAL_WRITE
#define ML_ERROR_SERIAL_DEVICE_NOT_RECOGNIZED  \
	INV_ERROR_SERIAL_DEVICE_NOT_RECOGNIZED
#define ML_ERROR_SM_TRANSITION          INV_ERROR_SM_TRANSITION
#define ML_ERROR_SM_IMPROPER_STATE      INV_ERROR_SM_IMPROPER_STATE
#define ML_ERROR_FIFO_OVERFLOW          INV_ERROR_FIFO_OVERFLOW
#define ML_ERROR_FIFO_FOOTER            INV_ERROR_FIFO_FOOTER
#define ML_ERROR_FIFO_READ_COUNT        INV_ERROR_FIFO_READ_COUNT
#define ML_ERROR_FIFO_READ_DATA         INV_ERROR_FIFO_READ_DATA
#define ML_ERROR_MEMORY_SET             INV_ERROR_MEMORY_SET
#define ML_ERROR_LOG_MEMORY_ERROR       INV_ERROR_LOG_MEMORY_ERROR
#define ML_ERROR_LOG_OUTPUT_ERROR       INV_ERROR_LOG_OUTPUT_ERROR
#define ML_ERROR_OS_BAD_PTR             INV_ERROR_OS_BAD_PTR
#define ML_ERROR_OS_BAD_HANDLE          INV_ERROR_OS_BAD_HANDLE
#define ML_ERROR_OS_CREATE_FAILED       INV_ERROR_OS_CREATE_FAILED
#define ML_ERROR_OS_LOCK_FAILED         INV_ERROR_OS_LOCK_FAILED
#define ML_ERROR_COMPASS_DATA_OVERFLOW  INV_ERROR_COMPASS_DATA_OVERFLOW
#define ML_ERROR_COMPASS_DATA_UNDERFLOW INV_ERROR_COMPASS_DATA_UNDERFLOW
#define ML_ERROR_COMPASS_DATA_NOT_READY INV_ERROR_COMPASS_DATA_NOT_READY
#define ML_ERROR_COMPASS_DATA_ERROR     INV_ERROR_COMPASS_DATA_ERROR
#define ML_ERROR_CALIBRATION_LOAD       INV_ERROR_CALIBRATION_LOAD
#define ML_ERROR_CALIBRATION_STORE      INV_ERROR_CALIBRATION_STORE
#define ML_ERROR_CALIBRATION_LEN        INV_ERROR_CALIBRATION_LEN
#define ML_ERROR_CALIBRATION_CHECKSUM   INV_ERROR_CALIBRATION_CHECKSUM
#define ML_ERROR_ACCEL_DATA_OVERFLOW    INV_ERROR_ACCEL_DATA_OVERFLOW
#define ML_ERROR_ACCEL_DATA_UNDERFLOW   INV_ERROR_ACCEL_DATA_UNDERFLOW
#define ML_ERROR_ACCEL_DATA_NOT_READY   INV_ERROR_ACCEL_DATA_NOT_READY
#define ML_ERROR_ACCEL_DATA_ERROR       INV_ERROR_ACCEL_DATA_ERROR
#endif

/* For Linux coding compliance */
#ifndef __KERNEL__
#define EXPORT_SYMBOL(x)
#endif

/*---------------------------
    p-Types
---------------------------*/

#endif				/* MLTYPES_H */
