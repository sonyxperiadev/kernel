/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef BATTERY_SETTINGS_H
#define BATTERY_SETTINGS_H

/* this is needed for the POWER_SUPPLY_TECHNOLOGY_LION definition */
#include <linux/power_supply.h>

/* 
   the following 2 settings are of interest to cmp_battery_multi.c and
   adc121c021_driver.c drivers. They are used to initialize cbm_platform_data
   in cmp_battery_multi.h and I2C_ADC121C021_t in adc121c021_driver.h 
*/
#define HW_BATTERY_MAX_VOLTAGE   8400
#define HW_BATTERY_MIN_VOLTAGE   5500

/*
  this is used to initialize  cbm_platform_data structure defined in
  cmp_battery_multi.h. All the data here considered to be platform specific 
*/
#define CMP_BATTERY_MULTI_SETTINGS \
{  \
   .battery_max_voltage     = HW_BATTERY_MAX_VOLTAGE, \
   .battery_min_voltage     = HW_BATTERY_MIN_VOLTAGE, \
   .battery_technology      = POWER_SUPPLY_TECHNOLOGY_LION, \
   .gpio_power_control      = 85, \
}    

#endif
