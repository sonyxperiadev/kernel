/**
 * \file    driver_util.h
 *  
 * \authors Pete Skeggs 
 *
 * \brief   utility functions  
 *
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */

/** \defgroup Driver_Util Utility functions
 *  \brief This defines functions used by the Generic host driver to do its
 *         work.
 * @{ */

#ifndef _DRIVER_UTIL_H_
#define _DRIVER_UTIL_H_

#include "host_services.h"
#include "EEPROMImage.h"
#include "driver_core.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * \brief read a line of text from a file 
 * \param f - file handle 
 * \param line - pointer to caller's buffer to store the line into 
 * \param max_len - the size of the buffer 
 * \return int number of characters read 
 */

   extern int read_line(FILE_HANDLE_T f, char *line, int max_len);



/**
 * \brief convert 16 bit register's signed 16 bit equivalent 
 *        value to floating point
 * \param buf - pointer to buffer containing the two bytes of 
 *            the register, LSB first
 * \return float - the value
 */
   extern float uint16_reg_to_float(u8 *buf);

/**
 * \brief convert an array of 4 bytes known to be in floating 
 *        point format to a floating point value
 * \param buf - pointer to the 4 bytes, LSB first
 * \return float - the value
 */
   extern float uint32_reg_to_float(u8 *buf);



/**
 * \brief using parameter transfer, query the current sensor 
 *        drivers and display to the log
 * \param instance 
 * \return bool 
 */
   extern bool display_driver_info(DI_INSTANCE_T *instance);

/**
 * \brief queries and displays error info from both Sentral and the
 * Generic Host Driver itself
 * \param instance
 */
   extern void display_error_info(DI_INSTANCE_T *instance);

   /** 
    * \brief stores error information and logs it 
    * \param instance - the driver instance 
    * \param error - the error code
    * \param line - the source code line number the error occurred on
    * \param fn - the name of the function in which the error occurred
    */
   extern void record_error(DI_INSTANCE_T *instance, DI_ERROR_CODE_T error, int line, const char *fn);

/**
 * \brief show what the current sensor sample rates are
 * \param instance - instance to query 
 * \param sensors_present - array of sensor IDs containing only the IDs for 
 *        sensors in this system
 * \param sensors_present_size - number of elements of the array 
 * \return TRUE if succeeded
 */
   extern bool display_actual_rates(DI_INSTANCE_T *instance, DI_SENSOR_TYPE_T *sensors_present, int sensors_present_size);

/**
 * \brief show current values of all Sentral registers
 * \param instance 
 * \return TRUE if succeeded
 */
   extern bool dump_all_registers(DI_INSTANCE_T *instance);

/**
 * \brief File system services
 **/

/**
 * \brief abstract interface to a function that can return the 
 *        length of the file; if it cannot be determined, then
 *        return 0
 * \param file_name - the string identifying the resource
 * \return the length
 */
   extern u32 file_size(char *file_name);



/**
 * \brief read a specified number of bytes from the resource
 * \param handle - the resource to read from
 * \param buffer - the caller's buffer where it would like the data stored
 * \param len - the number of bytes requested 
 * \param len_read - the number actually read; will be 0 if called after end of file reached 
 * \return bool - false if parameters are invalid or resource could not be read, or end of file; 
 * should return true even if only a partial read could be performed (i.e., reached end of file 
 * in the middle of a read) 
 */
   extern bool file_read(FILE_HANDLE_T handle, u8 *buffer, u16 len, u16 *len_read);




#ifdef __cplusplus
}
#endif

#endif

/** @}*/

