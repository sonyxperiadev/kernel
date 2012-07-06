/*
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef LINUX_BMA250_NG_COMMON_H
#define LINUX_BMA250_NG_COMMON_H

#define BMA250_AWAKE 1
#define BMA250_SLEEP 0
#define READ_BYPASS_STATE 0
#define SAVE_BYPASS_STATE 1

#define BMA250_SLAVE1 0x01
#define BMA250_SLAVE2 0x02

#if defined(CONFIG_SENSORS_MPU3050)
extern struct mutex mutex_fifo_reading_access_lock;
#define FIFO_ACCESS_MUTEX_LOCK() mutex_lock(&mutex_fifo_reading_access_lock)
#define FIFO_ACCESS_MUTEX_UNLOCK() mutex_unlock(&mutex_fifo_reading_access_lock)
#else
#define FIFO_ACCESS_MUTEX_LOCK
#define FIFO_ACCESS_MUTEX_UNLOCK
#endif

extern struct mutex bma250_power_lock;

#endif


