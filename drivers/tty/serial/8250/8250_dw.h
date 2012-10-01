/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 * @file   drivers/tty/serial/8250/8250_dw.h
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************/

#ifndef _SERIAL_8250_DW_H
#define _SERIAL_8250_DW_H

struct dw8250_data {
	struct clk	*clk;
	int		last_lcr;
	int		line;
};

#endif /* _SERIAL_8250_DW_H */
