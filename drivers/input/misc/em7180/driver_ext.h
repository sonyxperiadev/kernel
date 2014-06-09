/**
 * \file    driver_ext.h
 *  
 * \authors     Pete Skeggs 
 *
 * \brief  abstract interface to the extended portions of the generic host 
 *         driver, with entry points to manage all aspects of detecting,
 *         configuring, and transferring data with Sentral
 *
 * \copyright  (C) 2013-2014 EM Microelectronic
 *
 */

/** \defgroup Driver_Ext Extended generic host driver abstract interface
 *  \brief    This defines additional API entry points to help make integrating
 *  the generic host driver into an application easier. @{
 */

#ifndef _DRIVER_EXT_H_
#define _DRIVER_EXT_H_

#include "host_services.h"
#include "sentral_registers.h"
#include "sentral_parameters.h"
#include "driver_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TESLA_TO_GAUSS        (1.0e4)                          /**< conversion factor from Tesla to Gauss */
#define PASCALS_TO_BARS       (1.0e-5)                         /**< conversion factor from Pascals to Bars */
#define SENSOR_SCALE_MAG      ((30.5e-9)*TESLA_TO_GAUSS*1000)  /**< convert Sentral Mag value to milli-Gauss */
#define SENSOR_SCALE_ACCEL    (0.448e-3)                       /**< convert Sentral Accel value to g-s */
#define SENSOR_SCALE_GYRO     (152.6e-3)                       /**< convert Sentral Gyro value to degrees per second */
#define SENSOR_SCALE_BAROM    ((3)*(PASCALS_TO_BARS)           /**< convert Sentral Barom value to bars */
#define SENSOR_SCALE_HUMID    (1.0e-2)                         /**< convert Sentral Humid value to % relative humidity */
#define SENSOR_SCALE_TEMP     (1.0e-2)                         /**< convert Sentral Temp value to degrees C */
#define SENSOR_SCALE_TIME_NUM 15625                            /**< convert Sentral Timestamp to time in microseconds (numerator) */
#define SENSOR_SCALE_TIME_DEN 512                              /**< convert Sentral Timestamp to time in microseconds (denomenator) */
#define SENSOR_TIMESTAMP_OVERFLOW_US 2000000ul                 /**< amount of time between Sentral sensor timestamp overflows */

/** \brief maximum number allowed */
#define MAX_FEATURE_SENSORS 3

/** \brief a data structure containing all information for a
 *         quaternion data sample */
   struct DI_QUATERNION
   {
      float x;                                                       /**<  the x value */
      float y;                                                       /**<  the y value */
      float z;                                                       /**<  the z value */
      float w;                                                       /**<  the w value */
      u32 t;                                                         /**<  the sample timestamp, in microseconds, range 0 to 32 bit overflow (71.58 minutes) */
      bool hpr;                                                      /**<  a flag to indicate that the heading/pitch/roll is stored in x, y, and z, with w unused; false if this is really a quaternion */
      bool valid;                                                    /**<  true if this sample is valid */
   };
/** \brief a typedef wrapping the DI_QUATERNION structure */
   typedef struct DI_QUATERNION DI_QUATERNION_T;

/** \brief a structure representing a sample from a 3 axis
 *         sensor */
   struct DI_3AXIS_DATA
   {
      float x;                                                       /**<  the x value */
      float y;                                                       /**<  the y value */
      float z;                                                       /**<  the z value */
      u32 t;                                                         /**<  the sample timestamp, in microseconds, range 0 to 32 bit overflow (71.58 minutes) */
      bool valid;                                                    /**<  true if this sample is valid */
   };
/** \brief a typedef wrapping the DI_3AXIS_DATA structure */
   typedef struct DI_3AXIS_DATA DI_3AXIS_DATA_T;

/** \brief a structure containing all possible data samples
 *         available from an interrupt */
   struct DI_SENSOR_DATA
   {
      DI_QUATERNION_T quaternion;                                    /**<  the quaternion sample; each element ranges from 0.0 to 1.0 */
      DI_3AXIS_DATA_T mag;                                           /**<  the magnetometer sample, in milli-Gauss, range +/-10000 */
      DI_3AXIS_DATA_T accel;                                         /**<  the accelerometer sample, in gs (standard Earth gravities), range +/- 16 */
      DI_3AXIS_DATA_T gyro;                                          /**<  the gyroscope sample, in degrees per second, range +/- 5000 */
      DI_FEATURE_DATA_T feature[MAX_FEATURE_SENSORS];                /**<  data for all feature sensors, if present */
   };
/** \brief a typedef wrapping the DI_SENSOR_DATA structure */
   typedef struct DI_SENSOR_DATA DI_SENSOR_DATA_T;

/** 
 * \brief initialize the driver 
 * \param i2c_handle - an abstract handle used to access the 
 *                   Sentral chip via the host I2C services
 *                   (defined in host_services.h); the operating
 *                   system's driver configuration mechanism is
 *                   responsible for knowing how to reach the
 *                   Sentral chip (which bus and slave address,
 *                   etc.)
 * \param irq_handle - an abstract handle used to represent the 
 *                   data ready IRQ from the Sentral chip; it is
 *                   up to the operating system's driver
 *                   configuration mechanism to know how to
 *                   create this
 * \param reset -    set TRUE to force reset of Sentral, FALSE 
 *                   to leave it running as is 
 * \return DI_INSTANCE_T *- pointer to a structure representing 
 *         this driver instance
 */
   extern DI_INSTANCE_T *di_init(I2C_HANDLE_T i2c_handle, IRQ_HANDLE_T irq_handle, bool reset);

/**
 * \brief shutdown the driver 
 * \param instance - which instance to close
 * \return bool - true if succeeded, false if the instance was 
 *         never initialized
 */
   extern bool di_deinit(DI_INSTANCE_T *instance);

/**
 * \brief upload firmware to the Sentral chip's RAM
 * \param instance - the driver instance
 * \param firmware_filename - an externally-specified string 
 *                          representing the firmware file to
 *                          load into RAM in Sentral
 * \return bool - false if Sentral could not be reached; call 
 *         di_query_error() to determine the cause
 */
   extern bool di_upload_firmware(DI_INSTANCE_T *instance, char *firmware_filename);

/**
 * \brief upload firmware to the EEPROM attached to the Sentral 
 *        chip; we check the header, but rather than striping it
 *        as we do when uploading to RAM, we pass it as is;
 *        blocks
 * \param instance - the driver instance 
 * \param eeprom_handle - I2C handle (host-specific) to reach 
 *                      the EEPROM at slave address 0x50
 *                      (NOTE: Sentral assumes this address as
 *                      well as a register layout the same as a
 *                      Microchip 24LC256 EEPROM)
 * \param firmware_filename - an externally-specified string 
 *                          representing the firmware file to
 *                          load into EEPROM
 * \return bool - false if Sentral could not be reached; call 
 *         di_query_error() to determine the cause
 */
   extern bool di_upload_eeprom(DI_INSTANCE_T *instance, I2C_HANDLE_T eeprom_handle, char *firmware_filename);

   /**
    * \brief convert from integer (core) data to real-world unit floating point
    * \param instance - the driver instance 
    * \param sensor - which sensor (or DST_ALL)
    * \param int_data - the integer data as input
    * \param data - the floating point data as output
    * \param raw - true if data is not to be scaled
    * \return bool 
    */
   extern bool di_convert_sensor_data(DI_INSTANCE_T *instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_INT_DATA_T *int_data, DI_SENSOR_DATA_T *data, bool raw);

/** 
 *  \brief read a fresh value from the registers; blocks
 *  \param instance - the driver instance
 *  \param sensor - the sensor type to query
 *  \param data - a pointer to a data structure to which the
 *              sensor data will be stored
 * \return bool - false if the instance is invalid or if Sentral 
 *         could not be reached
*/
   extern bool di_query_sensor(DI_INSTANCE_T *instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_DATA_T *data);

/**
 * \brief returns the human-readable name for the driver 
 * \param driver_id - the id as reported by di_save_parameters() 
 * \return const char* or NULL if not found
 */
   extern const char *di_query_driver_name(u8 driver_id);

/**
 * \brief returns the human-readable name for the sensor
 * \param sensor - the sensor type
 * \return const char* or NULL if not found
 */
   extern const char *di_query_sensor_name(DI_SENSOR_TYPE_T sensor);

// may not be necessary to make these public...
/** \brief define the driver's instance of its IRQ_CALLBACK */
   extern IRQ_CALLBACK di_irq_callback;
/** \brief define the driver's instance of its I2C_CALLBACK */
   extern I2C_CALLBACK di_i2c_callback;


/**
 * \brief abstract interface to a function that can request 
 *        access to a named read-only resource; this could be an
 *        area hard-coded in Flash memory, a filesystem handle,
 *        or process resource identifier
 * \param file_name - the string identifying the resource
 * \return FILE_HANDLE_T - a handle to the resource or NULL if 
 *         in error (e.g., not found)
 */
   extern FILE_HANDLE_T file_open(char *file_name);                  /**< read-only access is all that is needed */

/** 
 * \brief release access to the resource 
 * \param handle - the resource to release
 */
   extern void file_close(FILE_HANDLE_T handle);



#ifdef __cplusplus
}
#endif

#endif

/** @}*/

