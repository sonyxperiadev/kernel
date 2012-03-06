/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* @file	kernel/include/linux/broadcom//bcm_bzhw.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef __BCM_BZHW_H
#define __BCM_BZHW_H

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/serial_core.h>

struct bcm_bzhw_platform_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */
};
struct bcm_bzhw_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */

#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock host_wake_lock;	/* rx path */
	struct wake_lock bt_wake_lock;	/* tx path */
#endif
	unsigned int host_irq;
};
int bcm_bzhw_assert_bt_wake(void);
int bcm_bzhw_deassert_bt_wake(void);
void bcm_bzhw_request_clock_on(struct uart_port *uport);
void bcm_bzhw_request_clock_off(struct uart_port *uport);

#endif
