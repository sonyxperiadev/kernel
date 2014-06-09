/**
 * \file    sensor_handling.c
 *  
 * \authors Pete Skeggs 
 *
 * \brief   useful routines for managing sensor data
 *
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */

#include "host_services.h"
#include "driver_util.h"
#include "sensor_handling.h"
#include "driver_core.h"

#ifndef __KERNEL__
	#include "sensor_stats.h"
	#include "driver_ext.h"
#endif

/** \addtogroup Sensor_Handling
 *  @{
 */

// sensor data acquisition variables
#if defined(_MSC_VER)
#define MEMORY_SECTION  /**< location in RAM to store this data */
#else
#define MEMORY_SECTION __attribute__((section (".data.$RAM3"))) /**< location in RAM to store this data */
#endif
#ifndef __KERNEL__
MEMORY_SECTION volatile DI_SENSOR_DATA_T sensor_data;                // sensor_hid, main, firmware tests
#endif

static volatile bool sensor_ready[DST_NUM_SENSOR_TYPES];             // sensor_hid, data_callback
static volatile u32 sensor_data_received = 0;


// firmware tests, main
u32 get_sensor_data_received_count(void)
{
   return sensor_data_received;
}

// sensor_hid
bool check_sensor_ready(DI_SENSOR_TYPE_T sensor)
{
   if (sensor == DST_ALL)
      return sensor_ready[DST_ACCEL] || sensor_ready[DST_GYRO] || sensor_ready[DST_MAG] || sensor_ready[DST_QUATERNION];
   return sensor_ready[sensor]; 
}


// none
void clear_sensor_ready(DI_SENSOR_TYPE_T sensor)
{
   sensor_ready[sensor] = FALSE;
}


// firmware tests, normal main
bool data_callback(DI_INSTANCE_T * instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_INT_DATA_T * data, void *user_param)
{
   int i;

   // convert from integer to floating point
#ifndef __KERNEL__
   di_convert_sensor_data(instance, sensor, data, &sensor_data, FALSE);
#endif

   switch (sensor)
   {
      case DST_ALL:
         if (data->accel.valid)
         {
            data->accel.valid = FALSE;
            sensor_ready[DST_ACCEL] = TRUE;
         }
         if (data->gyro.valid)
         {
            data->gyro.valid = FALSE;
            sensor_ready[DST_GYRO] = TRUE;
         }
         if (data->mag.valid)
         {
            data->mag.valid = FALSE;
            sensor_ready[DST_MAG] = TRUE;
         }
         if (data->quaternion.valid)
         {
            data->quaternion.valid = FALSE;
            sensor_ready[DST_QUATERNION] = TRUE;
         }
         for (i = 0; i < MAX_FEATURE_SENSORS; i++)
         {
            if (data->feature[i].valid) 
            {
               data->feature[i].valid = FALSE;
               sensor_ready[instance->feature_sensors[i]] = TRUE;
            }
         }
         break;
      case DST_ACCEL:
         data->accel.valid = FALSE;
         sensor_ready[sensor] = TRUE;
         break;
      case DST_GYRO:
         data->gyro.valid = FALSE;
         sensor_ready[sensor] = TRUE;
         break;
      case DST_MAG:
         data->mag.valid = FALSE;
         sensor_ready[sensor] = TRUE;
         break;
      case DST_QUATERNION:
         data->quaternion.valid = FALSE;
         sensor_ready[sensor] = TRUE;
         break;
      case DST_BAROM:
      case DST_HUMID:
      case DST_TEMP:
      case DST_CUST0:
      case DST_CUST1:
      case DST_CUST2:
         for (i = 0; i < MAX_FEATURE_SENSORS; i++)
         {
            if (data->feature[i].valid)
            {
               data->feature[i].valid = FALSE;
               sensor_ready[sensor] = TRUE;
            }
         }
         break;
      default:
         break;
   }
   sensor_data_received++;
   return TRUE;
}

/** @}*/

