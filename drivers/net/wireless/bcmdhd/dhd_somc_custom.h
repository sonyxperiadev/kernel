/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * Author: Daisuke Niwa daisuke.x.niwa@sonymobile.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __DHD_SOMC_CUSTOM_H__
#define __DHD_SOMC_CUSTOM_H__

extern int somc_txpower_calibrate(char *nvram, int nvram_len);
extern int somc_update_qtxpower(char *buf, char band, int chain);

#endif /* __DHD_SOMC_CUSTOM_H__ */
