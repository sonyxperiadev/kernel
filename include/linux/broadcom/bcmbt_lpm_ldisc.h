/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/*
 * Broadcom Bluetooth low power mode via GPIO
 *
 */

#ifndef _BCMBT_LPM_LDISC_H
#define _BCMBT_LPM_LDISC_H

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

struct bcmbt_lpm_ldisc_platform_data {
	int gpio_bt_wake;
	int gpio_host_wake;
};

struct bcmbt_lpm_ldisc_data {
	int gpio_bt_wake;	/* HOST -> BCM chip wakeup gpio */
	int gpio_host_wake;	/* BCM chip -> HOST wakeup gpio */
	int host_irq;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock    bt_wake_lock;	/* tx path */
	struct wake_lock    host_wake_lock;	/* rx path */
#endif
};

#ifndef TIO_ASSERT_BT_WAKE
#define TIO_ASSERT_BT_WAKE      0x8003
#endif
#ifndef TIO_DEASSERT_BT_WAKE
#define TIO_DEASSERT_BT_WAKE    0x8004
#endif
#ifndef TIO_GET_BT_WAKE_STATE
#define TIO_GET_BT_WAKE_STATE   0x8005
#endif

#endif
