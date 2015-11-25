/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __MHL_PLATFORM_H__
#define __MHL_PLATFORM_H__

#include <linux/i2c.h>

#include <linux/mhl/mhl.h>

/* PMIC VMIN SETTING [FIXME] */
#define MHL_PMIC_VMIN_SET

/***** mhl_platform.c *****/
int mhl_pf_init(void);
void mhl_pf_exit(void);

/***** mhl_platform_power.c *****/
int mhl_platform_power_init(void);
void mhl_platform_power_exit(void);

/***** mhl_platform_base.c *****/
struct clk *mhl_pf_get_mhl_clk(void);
int mhl_pf_get_gpio_num_int(void);
int mhl_pf_get_gpio_num_rst(void);
int mhl_pf_get_gpio_num_fw_wake(void);
int mhl_pf_get_gpio_num_mhl_clock(void);
struct i2c_client *mhl_pf_get_i2c_client(void);

/***** mhl_platform_i2c.c *****/
void mhl_pf_i2c_init(struct i2c_adapter *adapter);

#endif /* __MHL_PLATFORM_H__ */
