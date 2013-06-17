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

#ifndef BATTERY_BQ24616_H
#define BATTERY_BQ24616_H

// defines GPIOs used for battery operations
struct battery_bq24616_cfg {
   /* Output GPIO: controls battery power output. 
      HIGH = letting power through, LOW = cutting power off. */
   int gpio_ctl_pwr;

    /* Input GPIO: AC power present/shutdown. HIGH = AC power removed. */
    int gpio_acp_shd;

    /* Until proper solution for docking station is found, cp docking
       station power is handled here */
    int gpio_docking_station_power_3_3v;
}; 

#endif  /* BATTERY_BQ24616_H */
