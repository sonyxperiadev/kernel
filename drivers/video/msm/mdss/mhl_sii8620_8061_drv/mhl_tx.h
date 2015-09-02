/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_tx.h
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

#ifndef __MHL_TX_H__
#define __MHL_TX_H__
#include "linux/types.h"
int mhl_tx_initialize(void);
void mhl_tx_release(void);

extern int mhl_tx_rcp_init(struct device *parent);
extern void mhl_tx_rcp_release(void);
extern int mhl_tx_rcp_start(void);
extern void mhl_tx_rcp_stop(void);
extern u16 mhl_tx_rcp_IsSupported(u8 key_code);
extern int mhl_tx_rcp_input(u8 key_code);

#endif /* __SOMC_MHL_TX_H__ */
