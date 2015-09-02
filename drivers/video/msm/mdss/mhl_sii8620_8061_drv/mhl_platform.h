/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Hirokuni Kawasaki <hirokuni.kawaaki@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_PLATFORM_H__
#define __MHL_PLATFORM_H__


#define MHL_USB_INUSE 0
#define MHL_USB_NON_INUSE 1

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>

#include "si_8620_regs.h"
/*
 * Following are public to other files
 */
struct mhl_tx_ctrl {
	struct i2c_client *i2c_handle;
};

typedef enum {
	HPD_CTRL_MODE_ERROR = -1,
	HPD_CTRL_OPEN_DRAIN,
	HPD_CTRL_PUSH_PULL
} hpd_control_mode;

int is_interrupt_asserted(void);

/*
 * usb/mhl switch control
 */
int mhl_pf_switch_to_usb(void);
void mhl_pf_switch_register_cb(int (*device_discovery)(void *context_cb),
								void *context);
void mhl_pf_switch_unregister_cb(void);

/* charger I/F */
void  mhl_platform_power_stop_charge(void);
int   mhl_platform_power_start_charge(char *devcap);

/* get gpio_num I/F */
int mhl_pf_get_gpio_num_int(void);
int mhl_pf_get_gpio_num_pwr(void);
int mhl_pf_get_gpio_num_rst(void);
int mhl_pf_get_gpio_num_fw_wake(void);

/* get i2c_client I/F */
struct i2c_client *mhl_pf_get_i2c_client(void);

/* get clk */
extern struct clk *mhl_pf_get_mhl_clk(void);

/*
 * chip power control
 */
void mhl_pf_chip_power_on(void);
void mhl_pf_chip_power_off(void);
bool mhl_pf_is_chip_power_on(void);

hpd_control_mode platform_get_hpd_control_mode(void);

int mhl_pf_get_irq_number(void);
const char *mhl_pf_get_device_name(void);


/*
 * i2c
 */
/* when chip pwr is down, the error code is returned. -MHL_I2C_NOT_AVAILABLE */
#define MHL_I2C_NOT_AVAILABLE 0xC100
/* Following must be accessed by only mhl_platform.c */
void mhl_pf_i2c_init(struct i2c_adapter *adapter);
int mhl_pf_read_reg_block(u8 page, u8 offset, u8 count, u8 *values);
int mhl_pf_read_reg(u8 page, u8 offset);
int mhl_pf_write_reg(u8 page, u8 offset, u8 value);
int mhl_pf_write_reg_block(u8 page, u8 offset, u16 count, u8 *values);
int mhl_pf_modify_reg(u8 page, u8 offset, u8 mask, u8 value);

/*
 *Following is for unit test config
 */
#ifndef UNIT_TEST
/*todo : must be changed to use dts file*/
#define GPIO_MHL_SWITCH_SEL_1 10
#define GPIO_MHL_SWITCH_SEL_2 11
#else
#define GPIO_MHL_SWITCH_SEL_1 10
#define GPIO_MHL_SWITCH_SEL_2 11
#endif
#endif /* __MHL_PLATFORM_H__ */
