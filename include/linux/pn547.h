/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _LINUX_PN547_H
#define _LINUX_PN547_H

#define PN547_MAGIC	0xE9

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(>1): power on with firmware download enabled
 */
#define PN547_SET_PWR	_IOW(PN547_MAGIC, 0x01, unsigned int)

enum pn547_state {
	PN547_STATE_UNKNOWN,
	PN547_STATE_OFF,
	PN547_STATE_ON,
	PN547_STATE_FWDL,
};

struct pn547_i2c_platform_data {
	void (*conf_gpio) (void);
	int irq_gpio;
	int ven_gpio;
	int firm_gpio;
#ifdef CONFIG_NFC_PN547_CLOCK_REQUEST
	int clk_req_gpio;
	int clk_req_irq;
#endif
#ifdef CONFIG_OF
	u32 irq_gpio_flags;
	u32 ven_gpio_flags;
	u32 firm_gpio_flags;
	u32 pvdd_en_gpio_flags;
#endif
	int pvdd_en_gpio;
	int configure_gpio;
	int configure_mpp;
	bool dynamic_config;
};

#if defined(CONFIG_ARM) && defined (CONFIG_ARCH_MSM)
int board_nfc_parse_dt(struct device *dev,
		struct pn547_i2c_platform_data *pdata);
int board_nfc_hw_lag_check(struct i2c_client *d,
		struct pn547_i2c_platform_data *pdata);
#else
#define board_nfc_parse_dt(x, ...) 0
#define board_nfc_hw_lag_check(x, ...) 0
#endif

#endif
