/*
 *  Copyright (C) 2010, Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __IST30XX_MISC_H__
#define __IST30XX_MISC_H__


#define IST30XX_TUNES               (1)

#define IST30XX_SENSOR_ADDR         (0x40009000)
#define IST30XX_RAW_ADDR            (0x40100400)
#define IST30XX_FILTER_ADDR         (0x40100800)

#define IST30XX_RX_CNT_ADDR         (0x20000038)
#define IST30XX_CONFIG_ADDR         (0x20000040)

int ist30xx_init_misc_sysfs(void);

#endif  // __IST30XX_MISC_H__
