/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * Copyright (C) 2019 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef _SONY_EXT_UIM_CTRL_H_
#define _SONY_EXT_UIM_CTRL_H_

#ifdef CONFIG_SONY_EXT_UIM_CTRL
extern void sony_ext_uim_ctrl_set_uim2_detect_en(int value);
#else
static inline void sony_ext_uim_ctrl_set_uim2_detect_en(int value)
{
}
#endif
#endif /* _SONY_EXT_UIM_CTRL_H_ */
