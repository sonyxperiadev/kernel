/*******************************************************************************
* Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
*
* @file	kernel/include/linux/broadcom//bcmbt_lpm.h
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

#ifndef __ASM_BCMBT_LPM_H
#define __ASM_BCMBT_LPM_H

#include <linux/serial_core.h>
#include <linux/wakelock.h>

struct bcm_bt_lpm_platform_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */
};
struct bcm_bt_lpm_data {
	unsigned int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	unsigned int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */
	unsigned int polarity;	/* ASSERT polarity, e.g 0 -> active low */
	int state; /* state of LPM, 0 off */
	int timeout; /* 0: no timeout*/
	/*spinlock_t          lock; */
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock host_wake_lock;	/* rx path */
	struct wake_lock bt_wake_lock;	/* tx path */
#endif
	int bt_wake_installed;
	int host_wake_installed;
};

/* cmds for LPM */
#define DISABLE_LPM 0
#define ENABLE_LPM_TYPE_OOB 1 /* BT_WAKE/HOST_WAKE fully automatic */
#define ENABLE_LPM_TYPE_OOB_USER 2 /* BT_WAKE/HOST_WAKE, BT_WAKE toggled by
				ioctl */
/* polarity use by host/bt_wake: ACTIVE_LOW: ASSERTED at low level */
#define ACTIVE_LOW  0
#define ACTIVE_HIGH 1
#define DEFAULT_TO  50	/* use 50ms as default timeout */
struct bcmbt_set_lpm_mode {
	int cmd;
	int polarity;
	int timeout;
};

#define BRCM_SHARED_UART_MAGIC  0x80
#define TIO_ASSERT_BT_WAKE      _IO(BRCM_SHARED_UART_MAGIC, 3)
#define TIO_DEASSERT_BT_WAKE    _IO(BRCM_SHARED_UART_MAGIC, 4)
#define TIO_GET_BT_WAKE_STATE   0x8005
#define TIO_SET_LPM_MODE	_IOW(BRCM_SHARED_UART_MAGIC, 6, struct \
	bcmbt_set_lpm_mode)

#define GPIO_UARTB_SEL (-1)
#define GPIO_BT_CLK32K_EN (-1)

#ifndef BT_WAKE_ASSERT
#define BT_WAKE_ASSERT 0
#endif
#ifndef BT_WAKE_DEASSERT
#define BT_WAKE_DEASSERT (!(BT_WAKE_ASSERT))
#endif

#ifndef HOST_WAKE_ASSERT
#define HOST_WAKE_ASSERT 0
#endif
#ifndef HOST_WAKE_DEASSERT
#define HOST_WAKE_DEASSERT (!(HOST_WAKE_ASSERT))
#endif

#endif
