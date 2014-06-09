/**
 * \file    sensor_stats.h
 *  
 * \authors Pete Skeggs 
 *
 * \brief   routines to gather and display sensor statistics 
 * \todo    add other statistics, like standard deviation 
 *
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */


/** \defgroup Sensor_Stats Sensor statistics functions
 *  \brief This provides useful functions for counting, averaging, and finding
 *  the min and max values for the axes of all sensors.  The caller determines
 *  the sample size by periodically calling reset_stats() and
 *  calc_periodic_stats().
 * @{ */

#ifndef _STATS_H_
#define _STATS_H_

#include "host_services.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief clear all statistics data
 */
   extern void reset_stats(void);

/**
 * \brief using any valid data in the global sensor_data structure,
 * update the stats (min, max, ave) for each valid sensor
 * \param instance - the instance to take stats on 
 * \param verbose - if true, will display each raw sample 
 */
   extern void update_sensor_stats(DI_INSTANCE_T * instance, bool verbose);

/**
 * \brief calculates the averages for each sensor's axes over the current 
 * stats interval; also calculates the sample rate over that interval 
 * \param instance - the instance to calculate on 
 */
   extern void calc_periodic_stats(DI_INSTANCE_T * instance);

/**
 * \brief display either average and sample rates, or min, max, ave, and rates,
 *        for all sensors
 * \param verbose - whether to show all stats or just ave and rates
 */
   extern void display_periodic_stats(bool verbose);

/**
 * \brief calculate the magnitude of a 3 axis vector
 * \param v - the 3 axis value
 * \return the magnitude
 */
   extern float magnitude_3axis(DI_3AXIS_DATA_T *v);

/**
 * \brief return the magnitude of the average value of the specified sensor's
 *        axes
 * \param sensor - the sensor to query
 * \return the magnitude
 */
   extern float get_3axis_ave_magnitude(DI_SENSOR_TYPE_T sensor);

/**
 * \brief return the magnitude of a quaternion
 * \param v - the quaternion
 * \return the magnitude
 */
   extern float magnitude_quat(DI_QUATERNION_T *v);

/**
 * \brief return the sample rate for the specified sensor over the current test
 *        interval
 * \param sensor - the sensor to check
 * \return the rate in samples / second
 */
   extern u16 get_measured_sample_rate(DI_SENSOR_TYPE_T sensor);

/**
 * \brief clear all measured sample rates 
 */
   extern void clear_measured_sample_rates(void);

#ifdef __cplusplus
}
#endif

#endif

/** @}*/

