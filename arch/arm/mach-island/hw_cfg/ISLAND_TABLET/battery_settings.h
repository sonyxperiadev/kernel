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
   .battery_technology      = 2, \
   .gpio_power_control      = 85, \
   .gpio_ac_power           = 181, \
   .gpio_charging           = 180, \
   .adc121_info.type              = enum_adc121,  \
   .adc121_info.gpio_ac_power     = 181,    \
   .adc121_info.ac_power_on_level = 1,    \
   .adc121_info.gpio_charger      = 180,    \
   .max17040_info.type              = enum_max17040, \
   .max17040_info.gpio_ac_power     = 181, \
   .max17040_info.ac_power_on_level = 0, \
   .max17040_info.gpio_charger      = -1, \
   .p_cbm_data              = NULL \
}    

#endif
