/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_lib_timer.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_LIB_TIMER_H__
#define __MHL_LIB_TIMER_H_


int mhl_lib_timer_init(void);
int mhl_lib_timer_create(void (*callback_handler)(void *callback_param),
			void *callback_param,
			void **timer_handle);
int mhl_lib_timer_delete(void **timer_handle);
int mhl_lib_timer_start(void *timer_handle, uint32_t time_msec);
int mhl_lib_timer_stop(void *timer_handle);
void mhl_lib_timer_release(void);

#endif
