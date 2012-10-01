/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _CMP_BATTERY_MULTI_H_
#define _CMP_BATTERY_MULTI_H_

#define HW_CMP_MULTI_DRIVER_NAME   "cmp-battery"

/* Information for each battery monitor type. */
struct battery_monitor
{
   char *name;
   int gpio_ac_power;
   int ac_power_on_level;
   int gpio_charger;
   int (*get_voltage_fn)(void *p_data);
   int (*get_charge_fn) (void *p_data);   
};   

/* 
 * Battery platform data. Filled out in platform initialization code.
 */  
struct cbm_platform_data 
{  
   int battery_max_voltage;
   int battery_min_voltage;
   int battery_technology;
   
   int gpio_power_control;
};

int register_battery_monitor(struct battery_monitor *p_battery_monitor,
                             void *p_data);

#endif    /* _CMP_BATTERY_MULTI_H_ */

