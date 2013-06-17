/*
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Wonguk Jeong <wonguk.jeong@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef _FSA9485_H_
#define _FSA9485_H_

enum {
	FSA9485_DETACHED,
	FSA9485_ATTACHED
};

#define UART_SEL_SW	    58
#define JIG_BC_STS_SDP_MSK     (1<<2)
/* uart including Jig UART */
#define JIG_BC_STS_UART_MSK    ((1<<3) | (1<<11) | (1<<10))
#define JIG_BC_STS_DCP_MSK     (1<<6)
#define JIG_BC_STS_CDP_MSK     (1<<5)

struct fsa9485_platform_data {
	void (*cfg_gpio) (void);
	void (*otg_cb) (bool attached);
	void (*usb_cb) (bool attached);
	void (*uart_cb) (bool attached);
	void (*charger_cb) (bool attached);
	void (*jig_cb) (bool attached);
	void (*deskdock_cb) (bool attached);
	void (*cardock_cb) (bool attached);
	void (*mhl_cb) (bool attached);
	void (*reset_cb) (void);
	void (*set_init_flag) (void);
	void (*mhl_sel) (bool onoff);
};

enum {
	SWITCH_PORT_AUTO = 0,
	SWITCH_PORT_USB,
	SWITCH_PORT_AUDIO,
	SWITCH_PORT_UART,
	SWITCH_PORT_VAUDIO,
	SWITCH_PORT_USB_OPEN,
	SWITCH_PORT_ALL_OPEN,
};

extern void fsa9485_manual_switching(int path);
extern void fsa9485_otg_detach(void);
extern void fsa9485_otg_set_autosw_pda(void);
extern u16 fsa9485_get_chrgr_type(void);
extern int bcm_ext_bc_status(void);
extern enum bcmpmu_chrgr_type_t get_ext_charger_type(struct bcmpmu_accy *paccy, unsigned int bc_status);

extern struct class *sec_class;

#endif /* _FSA9485_H_ */
