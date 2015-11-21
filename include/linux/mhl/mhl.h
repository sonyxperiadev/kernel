/* kernel/include/linux/mhl/mhl.h
 *
 * Copyright (C) 2013 Silicon Image Inc.
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

#ifndef __MHL_H__
#define __MHL_H__

#include <linux/module.h>
#include <linux/i2c.h>

/*
 * Debug print
 */
/* print all debug info */
/* #define DEBUG_PRINT */

#ifdef DEBUG_PRINT
#undef pr_debug
#define pr_debug pr_info
#endif

#define MHL_USB_INUSE 0
#define MHL_USB_NON_INUSE 1

enum {
	MHL_SUCCESS = 0,
	MHL_FAIL = -1
};

struct mhl_tx_ctrl {
	struct i2c_client *i2c_handle;
	struct class *mhlclass;
	struct device *pdev;
};

/***** mhl_platform.c *****/
void mhl_pf_chip_power_on(void);
void mhl_pf_chip_power_off(void);
bool mhl_pf_is_chip_power_on(void);
int is_interrupt_asserted(void);
const char *mhl_pf_get_device_name(void);

/***** mhl_platform_base.c *****/
int mhl_pf_get_irq_number(void);
struct i2c_client *mhl_pf_get_i2c_client(void);
int mhl_pf_switch_to_usb(void);
int mhl_pf_switch_to_mhl(void);
void mhl_pf_switch_register_cb(int (*device_discovery)(void *context_cb),
								void *context);
void mhl_pf_switch_unregister_cb(void);
bool mhl_pf_check_vbus(void);
void mhl_pf_source_vbus_control(bool on);

/***** mhl_platform_i2c.c *****/
/* when chip pwr is down, the error code is returned. -MHL_I2C_NOT_AVAILABLE */
#define MHL_I2C_NOT_AVAILABLE 0xC100
int mhl_pf_read_reg_block(u16 address, u8 count, u8 *values);
int mhl_pf_read_reg(u16 address);
int mhl_pf_write_reg(u16 address, u8 value);
int mhl_pf_write_reg_block(u16 address, u16 count, u8 *values);
int mhl_pf_modify_reg(u16 address, u8 mask, u8 value);

/***** mhl_platform_power.c *****/
void mhl_platform_power_stop_charge(void);
void mhl_platform_power_start_charge(int set_current);

#endif /* __MHL_H__ */
