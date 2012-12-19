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

#include <linux/wakelock.h>
#include <linux/serial_core.h>
#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif

struct bcm_bzhw_platform_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */
};
struct bcm_bzhw_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */

	struct wake_lock host_wake_lock;	/* rx path */
	struct wake_lock bt_wake_lock;	/* tx path */
	unsigned int host_irq;
};

struct bcmbzhw_struct {
	struct bcm_bzhw_platform_data *pdata;
	struct bcm_bzhw_data bzhw_data;
	struct pi_mgr_qos_node qos_node;
	struct uart_port *uport;
	struct tty_struct *bcmtty;
	spinlock_t bzhw_lock;
	unsigned long bzhw_state;
	struct timer_list sleep_timer_hw;
};

int bcm_bzhw_assert_bt_wake(int bt_wake_gpio, struct pi_mgr_qos_node *lqos_node,
		struct tty_struct *tty);
int bcm_bzhw_deassert_bt_wake(int bt_wake_gpio, int host_wake_gpio);
void bcm_bzhw_request_clock_on(struct pi_mgr_qos_node *node);
void bcm_bzhw_request_clock_off(struct pi_mgr_qos_node *node);
struct bcmbzhw_struct *bcm_bzhw_start(struct tty_struct *tty);
void bcm_bzhw_stop(struct bcmbzhw_struct *hw_val);

extern void serial8250_togglerts_afe(struct uart_port *port,
		unsigned int flowon);
#endif
