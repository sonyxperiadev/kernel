/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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

#ifndef _EGALAX_I2C_TS_H
#define _EGALAX_I2C_TS_H

/*
 * SDIO WiFi GPIO configuration
 */
struct egalax_i2c_ts_gpio {
   /* to reset the touchscreen controller */
   int reset;

   /* to signal the host for touch events */
   int event;
};

struct egalax_i2c_ts_cfg {
   /* specify which I2C host */
   int id;

   /* GPIO configs */
   struct egalax_i2c_ts_gpio gpio;
};

#endif  /* _EGALAX_I2C_TS_H */
