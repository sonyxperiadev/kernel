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

typedef enum
{
   enum_unknown,
   enum_adc121,
   enum_max17040
}  battery_monitor_enum;   

/* Information for each battery monitor type. */
struct battery_monitor_info
{
   int   type;
   int   gpio_ac_power;
   int   ac_power_on_level;
   int   gpio_charger;
};

struct battery_monitor_calls
{
   int type;
	int (*get_voltage_fn)(void *p_data);
	int (*get_charge_fn) (void *p_data);   
   void *p_data;
};   

/* 
 * The structure that will be referred to by probe(), suspend(), resume() etc
 * eg suspend((struct platform_device *p_dev)
 * then
 * pt_cbm_platform_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
 */  
struct cbm_platform_data 
{  
   int battery_max_voltage;
   int battery_min_voltage;
   int battery_technology;
   
   int gpio_power_control;
   int gpio_ac_power;
   int gpio_charging;
   
   struct battery_monitor_info adc121_info;
   struct battery_monitor_info max17040_info;
   
   void *p_cbm_data;
};

int register_battery_monitor(struct battery_monitor_calls *p_calls,
                             void   *p_data);
                                  

#endif    /* _CMP_BATTERY_MULTI_H_ */

