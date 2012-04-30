/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
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
/* linux/include/asm-arm/arch-bcm28xx/gps.h
 *
 * Public interface for the GPS driver.
 */
#ifndef _GPS_H_
#define _GPS_H_

#include "linux/i2c-kona.h"

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#define GPS_DEVNAME                   "gps"

#define GPS_DRIVER_MAGIC              'U'
#define IOW_GPS_ON                    _IO(GPS_DRIVER_MAGIC, 1)
#define IOW_GPS_OFF                   _IO(GPS_DRIVER_MAGIC, 2)
#define IOW_GPS_RESET                 _IO(GPS_DRIVER_MAGIC, 3)
#define IOW_GPS_PASSTHROUGH_MODE_ON   _IO(GPS_DRIVER_MAGIC, 4)
#define IOW_GPS_PASSTHROUGH_MODE_OFF  _IO(GPS_DRIVER_MAGIC, 5)
#define IOR_GET_SERIAL_PORT_INFO      _IOR(GPS_DRIVER_MAGIC, 19 , unsigned long)

	struct gps_platform_data {
		struct i2c_slave_platform_data i2c_pdata;
		int gpio_reset;
		int gpio_power;
		int gpio_interrupt;
	};

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _GPS_H_ */
/* EOF */
