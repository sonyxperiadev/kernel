/* arch/arm/mach-msm/include/mach/board-nfc.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __ASM_ARCH_MSM_BOARD_NFC_H
#define __ASM_ARCH_MSM_BOARD_NFC_H

#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/pn547.h>

int board_nfc_parse_dt(struct device *dev,
		struct pn547_i2c_platform_data *pdata);
int board_nfc_hw_lag_check(struct i2c_client *d,
		struct pn547_i2c_platform_data *pdata);

#endif
