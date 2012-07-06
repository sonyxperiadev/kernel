/* arch/arm/mach-msm/nfc-fuji.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _NFC_SEMC_FUJI_H_
#define _NFC_SEMC_FUJI_H_

#ifdef CONFIG_NFC_PN544
#include <linux/pn544.h>

extern int pn544_chip_config(enum pn544_state state, void *not_used);
extern int pn544_gpio_request(void);
extern void pn544_gpio_release(void);
extern struct pn544_i2c_platform_data pn544_pdata;

#endif
#endif
