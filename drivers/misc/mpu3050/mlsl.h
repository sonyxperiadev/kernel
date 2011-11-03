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

#ifndef __MLSL_H__
#define __MLSL_H__

/**
 *  @defgroup   MLSL
 *  @brief      Motion Library - Serial Layer.
 *              The Motion Library System Layer provides the Motion Library
 *              with the communication interface to the hardware.
 *
 *              The communication interface is assumed to support serial
 *              transfers in burst of variable length up to
 *              SERIAL_MAX_TRANSFER_SIZE. The default value for
 *              SERIAL_MAX_TRANSFER_SIZE is 128 bytes. Transfers of length
 *              greater than SERIAL_MAX_TRANSFER_SIZE, will be subdivided in
 *              smaller transfers of length <= SERIAL_MAX_TRANSFER_SIZE.
 *              The SERIAL_MAX_TRANSFER_SIZE definition can be modified to
 *              overcome any host processor transfer size limitation down to
 *              1 B, the minimum.
 *              An higher value for SERIAL_MAX_TRANSFER_SIZE will favor
 *              performance and efficiency while requiring higher resource
 *              usage (mostly buffering).
 *              A smaller value will increase overhead and decrease efficiency
 *              but allows to operate with more resource constrained processor
 *              and master serial controllers.
 *              The SERIAL_MAX_TRANSFER_SIZE definition can be found in the
 *              mlsl.h header file.
 *
 *  @{
 *      @file   mlsl.h
 *      @brief  The Motion Library System Layer.
 *
 */

/*
  NOTE : this document is best viewed with 8 spaces per tab.
*/

#include "mltypes.h"
#include "mpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------ */
/* - Defines. - */
/* ------------ */

/*
 * NOTE : to properly support Yamaha compass reads,
 *	  the max transfer size should be at least 9 B.
 *	  Length in bytes, typically a power of 2 >= 2
 */
#define SERIAL_MAX_TRANSFER_SIZE 128

/* ---------------------- */
/* - Types definitions. - */
/* ---------------------- */

/* --------------------- */
/* - Function p-types. - */
/* --------------------- */

/**
 *  @brief  used to open the serial port.
 *	    This port is used to send and receive data to the device.
 *
 *  @note   This function is called by inv_serial_start().
 *	    Unlike previous MPL Software releases, explicitly calling
 *	    inv_serial_start() is mandatory to instantiate the communication
 *	    with the device.
 *
 *  @param  port
 *		The COM port specification associated with the device in use.
 *  @param  sl_handle
 *		a pointer to the file handle to the serial device to be open
 *		for the communication.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_open(char const *port, void **sl_handle);

/**
 *  @brief  used to close the serial port.
 *	    This port is used to send and receive data to the device.
 *
 *  @note   This function is called by inv_serial_stop().
 *	    Unlike previous MPL Software releases, explicitly calling
 *	    inv_serial_stop() is mandatory to properly shut-down the
 *	    communication with the device.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_close(void *sl_handle);

/**
 *  @brief  used to reset any buffering the driver may be doing
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_reset(void *sl_handle);

/**
 *  @brief  used to write a single byte of data.
 *	    It is called by the MPL to write a single byte of data
 *          to the MPU.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  registerAddr
 *		Register address to write.
 *  @param  data
 *		Single byte of data to write.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_single_write(void *sl_handle,
					    unsigned char slaveAddr,
					    unsigned char registerAddr,
					    unsigned char data);

/**
 *  @brief  used to write multiple bytes of data to registers.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  length
 *		Length of burst of data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_write(void *sl_handle,
				     unsigned char slaveAddr,
				     unsigned short length,
				     unsigned char const *data);

/**
 *  @brief  used to read multiple bytes of data from registers.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  registerAddr
 *		Register address to read.
 *  @param  length
 *		Length of burst of data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS == 0 if successful; a non-zero error code otherwise.
 */
	inv_error_t inv_serial_read(void *sl_handle,
				    unsigned char slaveAddr,
				    unsigned char registerAddr,
				    unsigned short length, unsigned char *data);

/**
 *  @brief  used to read multiple bytes of data from the memory.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  memAddr
 *		The location in the memory to read from.
 *  @param  length
 *		Length of burst data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS == 0 if successful; a non-zero error code otherwise.
 */
	inv_error_t inv_serial_read_mem(void *sl_handle,
					unsigned char slaveAddr,
					unsigned short memAddr,
					unsigned short length,
					unsigned char *data);

/**
 *  @brief  used to write multiple bytes of data to the memory.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  memAddr
 *		The location in the memory to write to.
 *  @param  length
 *		Length of burst data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS == 0 if successful; a non-zero error code otherwise.
 */
	inv_error_t inv_serial_write_mem(void *sl_handle,
					 unsigned char slaveAddr,
					 unsigned short memAddr,
					 unsigned short length,
					 unsigned char const *data);

/**
 *  @brief  used to read multiple bytes of data from the fifo.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  length
 *		Length of burst of data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS == 0 if successful; a non-zero error code otherwise.
 */
	inv_error_t inv_serial_read_fifo(void *sl_handle,
					 unsigned char slaveAddr,
					 unsigned short length,
					 unsigned char *data);

/**
 *  @brief  used to write multiple bytes of data to the fifo.
 *	    This should be sent by I2C or SPI.
 *
 *  @param  sl_handle
 *		a file handle to the serial device used for the communication.
 *  @param  slaveAddr
 *		I2C slave address of device.
 *  @param  length
 *		Length of burst of data.
 *  @param  data
 *		Pointer to block of data.
 *
 *  @return INV_SUCCESS == 0 if successful; a non-zero error code otherwise.
 */
	inv_error_t inv_serial_write_fifo(void *sl_handle,
					  unsigned char slaveAddr,
					  unsigned short length,
					  unsigned char const *data);

/**
 *  @brief  used to get the configuration data.
 *	    Is called by the MPL to get the configuration data
 *	    used by the motion library.
 *	    This data would typically be saved in non-volatile memory.
 *
 *  @param  cfg
 *		Pointer to the configuration data.
 *  @param  len
 *		Length of the configuration data.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_read_cfg(unsigned char *cfg, unsigned int len);

/**
 *  @brief  used to save the configuration data.
 *	    Is called by the MPL to save the configuration data
 *          used by the motion library.
 *	    This data would typically be saved in non-volatile memory.
 *
 *  @param  cfg
 *		Pointer to the configuration data.
 *  @param  len
 *		Length of the configuration data.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_write_cfg(unsigned char *cfg, unsigned int len);

/**
 *  @brief  used to get the calibration data.
 *	    It is called by the MPL to get the calibration data
 *          used by the motion library.
 *	    This data is typically be saved in non-volatile memory.
 *
 *  @param  cal
 *		Pointer to the calibration data.
 *  @param  len
 *		Length of the calibration data.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_read_cal(unsigned char *cal, unsigned int len);

/**
 *  @brief  used to save the calibration data.
 *	    It is called by the MPL to save the calibration data
 *          used by the motion library.
 *	    This data is typically be saved in non-volatile memory.
 *
 *  @param  cal
 *		Pointer to the calibration data.
 *  @param  len
 *		Length of the calibration data.
 *
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_write_cal(unsigned char *cal, unsigned int len);

/**
 *  @brief  Get the calibration length from the storage.
 *  @param  len
 *		lenght to be returned
 *  @return INV_SUCCESS if successful, a non-zero error code otherwise.
 */
	inv_error_t inv_serial_get_cal_length(unsigned int *len);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */
#endif				/* __MLSL_H__ */
