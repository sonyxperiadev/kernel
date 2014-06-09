/**
 * \file    sensor_handling.h
 *  
 * \authors Pete Skeggs 
 *
 * \brief   useful routines for managing sensor data
 *
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */

/** \defgroup Sensor_Handling Sensor handling functions
 *  \brief This provides an example sensor data callback, a place to store data,
 *  and functions to determine which sensors have new data
 * @{ */

#ifndef _SENSOR_HANDLING_H_
#define _SENSOR_HANDLING_H_

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief data callback; here we make a local copy of the just 
 *        received data
 * \param instance - the driver instance that generated the 
 *                 callback
 * \param sensor - the sensor that produced data 
 * \param data - a pointer to the just-received set of data; 
 * callback must copy this to its own local buffer and should 
 * not retain the pointer as it may be invalid after the 
 * callback returns 
 * \param user_param - the value specified on the call to 
 *                   di_register()
 * \return true 
 */
   extern bool data_callback(DI_INSTANCE_T * instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_INT_DATA_T * data, void *user_param);

/**
 * \brief determine if a sensor has a new sample
 * \param sensor - the sensor to check
 * \return bool true when there is
 */
   extern bool check_sensor_ready(DI_SENSOR_TYPE_T sensor);

/**
 * \brief clear sensor ready
 * \param sensor - the sensor to clear the new sample flag for
 */
   extern void clear_sensor_ready(DI_SENSOR_TYPE_T sensor);

/**
 * \brief get current sample counter
 * \return number of samples received so far for all sensors
 */
   extern uint32_t get_sensor_data_received_count(void);

/**
 * global data containing the most recent data samples
 */
#ifndef __KERNEL__
   extern volatile DI_SENSOR_DATA_T sensor_data;
#endif

#ifdef __cplusplus
}
#endif

#endif

/** @}*/

