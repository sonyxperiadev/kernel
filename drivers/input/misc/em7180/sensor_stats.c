/**
 * \file    sensor_stats.c
 *  
 * \authors Pete Skeggs 
 *
 * \brief   routines to gather and display sensor statistics
 *
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */

#include "host_services.h"
#include "driver_ext.h"
#include "driver_util.h"
#include "sensor_handling.h"
#include "sensor_stats.h"

/** \addtogroup Sensor_Stats
 *  @{
 */

// uncomment to add extra sanity checking to float values prior to printing
//#define CHECK_FLOATS

// sensor data acquisition variables
#if defined(_MSC_VER)
#define MEMORY_SECTION  /**< location in RAM to store this data */
#else
#define MEMORY_SECTION __attribute__((section (".data.$RAM3"))) /**< location in RAM to store this data */
#endif
static MEMORY_SECTION DI_SENSOR_DATA_T sensor_data_min;              // firmware tests, normal main
static MEMORY_SECTION DI_SENSOR_DATA_T sensor_data_max;
static MEMORY_SECTION DI_SENSOR_DATA_T sensor_data_ave;
static u32 stats_start_time;                                      // firmware tests, normal main
static u32 sample_counts[DST_NUM_SENSOR_TYPES];                   // firmware tests, normal main
static u16 measured_sample_rates[DST_NUM_SENSOR_TYPES];
static u8 num_feature_sensors = 0;
static DI_SENSOR_TYPE_T feature_sensors[MAX_FEATURE_SENSORS];


static void stats_3axis_min(DI_3AXIS_DATA_T *out, DI_3AXIS_DATA_T *in)
{
   if (out->x > in->x)
      out->x = in->x;
   if (out->y > in->y)
      out->y = in->y;
   if (out->z > in->z)
      out->z = in->z;
   if (out->t > in->t)
      out->t = in->t;
}


static void stats_3axis_max(DI_3AXIS_DATA_T *out, DI_3AXIS_DATA_T *in)
{
   if (out->x < in->x)
      out->x = in->x;
   if (out->y < in->y)
      out->y = in->y;
   if (out->z < in->z)
      out->z = in->z;
   if (out->t < in->t)
      out->t = in->t;
}


static void stats_3axis_ave(DI_3AXIS_DATA_T *out, DI_3AXIS_DATA_T *in)
{
   out->x += in->x;
   out->y += in->y;
   out->z += in->z;
   out->t += in->t;
}


static void stats_3axis_calc(DI_3AXIS_DATA_T *stat, u32 count)
{
   if (count)
   {
      stat->x /= count;
      stat->y /= count;
      stat->z /= count;
      stat->t /= count;
   }
}

// firmware tests
float magnitude_3axis(DI_3AXIS_DATA_T *v)
{
   return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

float get_3axis_ave_magnitude(DI_SENSOR_TYPE_T sensor)
{
   switch (sensor)
   {
      case DST_ACCEL:
         return magnitude_3axis(&sensor_data_ave.accel);
      case DST_GYRO:
         return magnitude_3axis(&sensor_data_ave.gyro);
      case DST_MAG:
         return magnitude_3axis(&sensor_data_ave.mag);
      default:
         return 0;
   }
}

// none
float magnitude_quat(DI_QUATERNION_T *v)
{
   return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);
}


static void stats_quat_min(DI_QUATERNION_T *out, DI_QUATERNION_T *in)
{
   if (out->x > in->x)
      out->x = in->x;
   if (out->y > in->y)
      out->y = in->y;
   if (out->z > in->z)
      out->z = in->z;
   if (out->w > in->w)
      out->w = in->w;
   if (out->t > in->t)
      out->t = in->t;
}


static void stats_quat_max(DI_QUATERNION_T *out, DI_QUATERNION_T *in)
{
   if (out->x < in->x)
      out->x = in->x;
   if (out->y < in->y)
      out->y = in->y;
   if (out->z < in->z)
      out->z = in->z;
   if (out->w < in->w)
      out->w = in->w;
   if (out->t < in->t)
      out->t = in->t;
}


static void stats_quat_ave(DI_QUATERNION_T *out, DI_QUATERNION_T *in)
{
   out->x += in->x;
   out->y += in->y;
   out->z += in->z;
   out->w += in->w;
   out->t += in->t;
}


static void stats_quat_calc(DI_QUATERNION_T *stat, u32 count)
{
   if (count)
   {
      stat->x /= count;
      stat->y /= count;
      stat->z /= count;
      stat->w /= count;
      stat->t /= count;
   }
}


// firmware tests, normal main
void reset_stats(void)
{
   int i;

   memset(sample_counts, 0, sizeof(sample_counts));
   memset(&sensor_data_ave, 0, sizeof(sensor_data_ave));
   memset(feature_sensors, 0, sizeof(feature_sensors));
   num_feature_sensors = 0;

   sensor_data_min.accel.x = 16.0;
   sensor_data_min.accel.y = 16.0;
   sensor_data_min.accel.z = 16.0;
   sensor_data_min.accel.t = 2000000;
   sensor_data_min.gyro.x = 5000.0;
   sensor_data_min.gyro.y = 5000.0;
   sensor_data_min.gyro.z = 5000.0;
   sensor_data_min.gyro.t = 2000000;
   sensor_data_min.mag.x = 10000.0;
   sensor_data_min.mag.y = 10000.0;
   sensor_data_min.mag.z = 10000.0;
   sensor_data_min.mag.t = 2000000;
   sensor_data_min.quaternion.x = 1.0;
   sensor_data_min.quaternion.y = 1.0;
   sensor_data_min.quaternion.z = 1.0;
   sensor_data_min.quaternion.w = 1.0;
   sensor_data_min.quaternion.t = 2000000;
   for (i = 0; i < MAX_FEATURE_SENSORS; i++) 
   {
      sensor_data_min.feature[i].data = 65535;
      sensor_data_min.feature[i].t = 2000000;
   }

   sensor_data_max.accel.x = -16.0;
   sensor_data_max.accel.y = -16.0;
   sensor_data_max.accel.z = -16.0;
   sensor_data_max.accel.t = 0;
   sensor_data_max.gyro.x = -5000.0;
   sensor_data_max.gyro.y = -5000.0;
   sensor_data_max.gyro.z = -5000.0;
   sensor_data_max.gyro.t = 0;
   sensor_data_max.mag.x = -10000.0;
   sensor_data_max.mag.y = -10000.0;
   sensor_data_max.mag.z = -10000.0;
   sensor_data_max.mag.t = 0;
   sensor_data_max.quaternion.x = 0.0;
   sensor_data_max.quaternion.y = 0.0;
   sensor_data_max.quaternion.z = 0.0;
   sensor_data_max.quaternion.w = 0.0;
   sensor_data_max.quaternion.t = 0;
   for (i = 0; i < MAX_FEATURE_SENSORS; i++) 
   {
      sensor_data_max.feature[i].data = 0;
      sensor_data_max.feature[i].t = 0;
   }

   memset(measured_sample_rates, 0, sizeof(measured_sample_rates));
   stats_start_time = time_ms();
}


// firmware tests, normal main
void update_sensor_stats(DI_INSTANCE_T * instance, bool verbose)
{
   int i;
   if (sensor_data.accel.valid)
   {
      sensor_data.accel.valid = FALSE;
      sample_counts[DST_ACCEL]++;
      stats_3axis_min(&sensor_data_min.accel, (DI_3AXIS_DATA_T *)&sensor_data.accel);
      stats_3axis_max(&sensor_data_max.accel, (DI_3AXIS_DATA_T *)&sensor_data.accel);
      stats_3axis_ave(&sensor_data_ave.accel, (DI_3AXIS_DATA_T *)&sensor_data.accel);
      if (verbose)
         info_log("accel: %f, %f, %f, %u\n", sensor_data.accel.x, sensor_data.accel.y, sensor_data.accel.z, sensor_data.accel.t);
   }
   if (sensor_data.gyro.valid)
   {
      sensor_data.gyro.valid = FALSE;
      sample_counts[DST_GYRO]++;
      stats_3axis_min(&sensor_data_min.gyro, (DI_3AXIS_DATA_T *)&sensor_data.gyro);
      stats_3axis_max(&sensor_data_max.gyro, (DI_3AXIS_DATA_T *)&sensor_data.gyro);
      stats_3axis_ave(&sensor_data_ave.gyro, (DI_3AXIS_DATA_T *)&sensor_data.gyro);
      if (verbose)
         info_log(" gyro: %f, %f, %f, %u\n", sensor_data.gyro.x, sensor_data.gyro.y, sensor_data.gyro.z, sensor_data.gyro.t);
   }
   if (sensor_data.mag.valid)
   {
      sensor_data.mag.valid = FALSE;
      sample_counts[DST_MAG]++;
      stats_3axis_min(&sensor_data_min.mag, (DI_3AXIS_DATA_T *)&sensor_data.mag);
      stats_3axis_max(&sensor_data_max.mag, (DI_3AXIS_DATA_T *)&sensor_data.mag);
      stats_3axis_ave(&sensor_data_ave.mag, (DI_3AXIS_DATA_T *)&sensor_data.mag);
      if (verbose)
         info_log("  mag: %f, %f, %f, %u\n", sensor_data.mag.x, sensor_data.mag.y, sensor_data.mag.z, sensor_data.mag.t);
   }
   if (sensor_data.quaternion.valid)
   {
      sensor_data.quaternion.valid = FALSE;
      sample_counts[DST_QUATERNION]++;
      stats_quat_min(&sensor_data_min.quaternion, (DI_QUATERNION_T *)&sensor_data.quaternion);
      stats_quat_max(&sensor_data_max.quaternion, (DI_QUATERNION_T *)&sensor_data.quaternion);
      stats_quat_ave(&sensor_data_ave.quaternion, (DI_QUATERNION_T *)&sensor_data.quaternion);
      if (verbose)
         info_log(" quat: %f, %f, %f, %f, %u\n", sensor_data.quaternion.x, sensor_data.quaternion.y, sensor_data.quaternion.z, sensor_data.quaternion.w, sensor_data.quaternion.t);
   }
   for (i = 0; i < MAX_FEATURE_SENSORS; i++)
   {
      if (sensor_data.feature[i].valid) 
      {
         sensor_data.feature[i].valid = FALSE;
         memcpy(feature_sensors, instance->feature_sensors, MAX_FEATURE_SENSORS); // save table of which is which
         num_feature_sensors = instance->num_feature_sensors;
         sample_counts[feature_sensors[i]]++;
         if (sensor_data_min.feature[i].data > sensor_data.feature[i].data)
            sensor_data_min.feature[i].data = sensor_data.feature[i].data;
         if (sensor_data_max.feature[i].data < sensor_data.feature[i].data)
            sensor_data_max.feature[i].data = sensor_data.feature[i].data;
         sensor_data_ave.feature[i].data += sensor_data.feature[i].data;
         if (1 || verbose)
            info_log("  feat%d: %u %u\n", i, sensor_data.feature[i].data, sensor_data.feature[i].t);
      }
   }
}


static bool validate_3axis(DI_3AXIS_DATA_T *data)
{
   if (!data)
      return FALSE;
#if defined(CHECK_FLOATS)
   if (isnan(data->x))
      return FALSE;
   if (isnan(data->y))
      return FALSE;
   if (isnan(data->z))
      return FALSE;
   if (isinf(data->x))
      return FALSE;
   if (isinf(data->y))
      return FALSE;
   if (isinf(data->z))
      return FALSE;
#endif
   return TRUE;
}


static bool validate_quat(DI_QUATERNION_T *data)
{
   if (!data)
      return FALSE;
#if defined(CHECK_FLOATS)
   if (isnan(data->x))
      return FALSE;
   if (isnan(data->y))
      return FALSE;
   if (isnan(data->z))
      return FALSE;
   if (isnan(data->w))
      return FALSE;
   if (isinf(data->x))
      return FALSE;
   if (isinf(data->y))
      return FALSE;
   if (isinf(data->z))
      return FALSE;
   if (isinf(data->w))
      return FALSE;
#endif
   return TRUE;
}


void calc_periodic_stats(DI_INSTANCE_T * instance)
{
   int i;
   u32 delta_t = time_ms() - stats_start_time;
   stats_3axis_calc(&sensor_data_ave.accel, sample_counts[DST_ACCEL]);
   stats_3axis_calc(&sensor_data_ave.gyro, sample_counts[DST_GYRO]);
   stats_3axis_calc(&sensor_data_ave.mag, sample_counts[DST_MAG]);
   stats_quat_calc(&sensor_data_ave.quaternion, sample_counts[DST_QUATERNION]);
   for (i = 0; i < MAX_FEATURE_SENSORS; i++)
   {
      if (sample_counts[feature_sensors[i]])
         sensor_data_ave.feature[i].data /= sample_counts[feature_sensors[i]]; 
   }

   if (delta_t)
   {
      measured_sample_rates[DST_ACCEL] = 1000 * sample_counts[DST_ACCEL] / delta_t;
      measured_sample_rates[DST_GYRO] = 1000 * sample_counts[DST_GYRO] / delta_t;
      measured_sample_rates[DST_MAG] = 1000 * sample_counts[DST_MAG] / delta_t;
      measured_sample_rates[DST_QUATERNION] = 1000 * sample_counts[DST_QUATERNION] / delta_t;
      for (i = 0; i < MAX_FEATURE_SENSORS; i++)
      {
         measured_sample_rates[feature_sensors[i]] = 1000 * sample_counts[feature_sensors[i]] / delta_t;
      }
   }
   else
      memset(measured_sample_rates, 0, sizeof(measured_sample_rates));
}


static void display_3axis_sensor_stats(DI_SENSOR_TYPE_T sensor, bool verbose)
{
   DI_3AXIS_DATA_T *data_min;
   DI_3AXIS_DATA_T *data_ave;
   DI_3AXIS_DATA_T *data_max;

   switch (sensor)
   {
      case DST_ACCEL:
         data_min = &sensor_data_min.accel;
         data_ave = &sensor_data_ave.accel;
         data_max = &sensor_data_max.accel;
         break;
      case DST_GYRO:
         data_min = &sensor_data_min.gyro;
         data_ave = &sensor_data_ave.gyro;
         data_max = &sensor_data_max.gyro;
         break;
      case DST_MAG:
         data_min = &sensor_data_min.mag;
         data_ave = &sensor_data_ave.mag;
         data_max = &sensor_data_max.mag;
         break;
      default:
         return;
   }

   if (verbose)
   {
      if (validate_3axis(data_min) && validate_3axis(data_ave) && validate_3axis(data_max))
      {
         info_log("%s %u samples: min(%f, %f, %f, %u), ave(%f, %f, %f, %u), max(%f, %f, %f, %u), rate: %u Hz\n",
                   di_query_sensor_name(sensor), sample_counts[sensor],
                   data_min->x, data_min->y, data_min->z, data_min->t,
                   data_ave->x, data_ave->y, data_ave->z, data_ave->t,
                   data_max->x, data_max->y, data_max->z, data_max->t,
                   measured_sample_rates[sensor]
                  );
      }
      else
         info_log("INVALID DATA\n");
   }
   else
   {
      if (validate_3axis(data_ave))
      {
         info_log("%s %u samples: ave(%f, %f, %f, %u), rate: %u Hz\n",
                   di_query_sensor_name(sensor), sample_counts[sensor],
                   data_ave->x, data_ave->y, data_ave->z, data_ave->t,
                   measured_sample_rates[sensor]
                  );
      }
      else
         info_log("INVALID DATA\n");
   }
}


static void display_quat_sensor_stats(DI_SENSOR_TYPE_T sensor, bool verbose)
{
   DI_QUATERNION_T *data_min;
   DI_QUATERNION_T *data_ave;
   DI_QUATERNION_T *data_max;

   switch (sensor)
   {
      case DST_QUATERNION:
         data_min = &sensor_data_min.quaternion;
         data_ave = &sensor_data_ave.quaternion;
         data_max = &sensor_data_max.quaternion;
         break;
      default:
         return;
   }

   if (verbose)
   {
      if (validate_quat(data_min) && validate_quat(data_ave) && validate_quat(data_max))
      {
         info_log("%s %u samples: min(%f, %f, %f, %f, %u), ave(%f, %f, %f, %f, %u), max(%f, %f, %f, %f, %u), rate: %u Hz\n",
                   di_query_sensor_name(sensor), sample_counts[sensor],
                   data_min->x, data_min->y, data_min->z, data_min->w, data_min->t,
                   data_ave->x, data_ave->y, data_ave->z, data_ave->w, data_ave->t,
                   data_max->x, data_max->y, data_max->z, data_max->w, data_max->t,
                   measured_sample_rates[sensor]
                  );
      }
      else
         info_log("INVALID DATA\n");
   }
   else
   {
      if (validate_quat(data_ave))
      {
         info_log("%s %u samples: ave(%f, %f, %f, %f, %u), rate: %u Hz\n",
                   di_query_sensor_name(sensor), sample_counts[sensor],
                   data_ave->x, data_ave->y, data_ave->z, data_ave->w, data_ave->t,
                   measured_sample_rates[sensor]
                  );
      }
      else
         info_log("INVALID DATA\n");
   }
}


static void display_feature_sensor_stats(bool verbose)
{
   int i;

   for (i = 0; i < MAX_FEATURE_SENSORS; i++)
   {
      if (verbose) 
         info_log("%s %u samples: min %u, ave %u, max %u, rate: %u Hz\n",
                   di_query_sensor_name(feature_sensors[i]), sample_counts[feature_sensors[i]], 
                   sensor_data_min.feature[i], sensor_data_ave.feature[i], sensor_data_max.feature[i], 
                   measured_sample_rates[feature_sensors[i]]);
      else
         info_log("%s %u samples: ave %u, rate: %u Hz\n",
                   di_query_sensor_name(feature_sensors[i]), sample_counts[feature_sensors[i]],
                   sensor_data_ave.feature[i], measured_sample_rates[feature_sensors[i]]);
   }
}


void display_periodic_stats(bool verbose)
{
   display_3axis_sensor_stats(DST_ACCEL, verbose);
   display_3axis_sensor_stats(DST_GYRO, verbose);
   display_3axis_sensor_stats(DST_MAG, verbose);
   display_quat_sensor_stats(DST_QUATERNION, verbose);
   display_feature_sensor_stats(verbose);
}


u16 get_measured_sample_rate(DI_SENSOR_TYPE_T sensor)
{
   return measured_sample_rates[sensor];
}


void clear_measured_sample_rates(void)
{
   memset(measured_sample_rates, 0, sizeof(measured_sample_rates));
}


/** @}*/

