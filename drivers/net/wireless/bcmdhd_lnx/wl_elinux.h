/*
 * Linux cfg80211 driver - eLinux related functions
 *
 * Copyright (C) 1999-2011, Broadcom Corporation
 *
 *         Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: wl_elinux.h,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#ifndef _wl_elinux_h_
#define _wl_elinux_h_

#include <linux/module.h>
#include <linux/netdevice.h>
#include <wldev_common.h>

/**
 * eLinux platform dependent functions, feel free to add eLinux specific functions here
 * (save the macros in dhd). Please do NOT declare functions that are NOT exposed to dhd
 * or cfg, define them as static in wl_elinux.c
 */

/**
 * wl_elinux_init will be called from module init function (dhd_module_init now), similarly
 * wl_elinux_exit will be called from module exit function (dhd_module_cleanup now)
 */
int wl_elinux_init(void);
int wl_elinux_exit(void);
void wl_elinux_post_init(void);
int wl_elinux_wifi_on(struct net_device *dev);
int wl_elinux_wifi_off(struct net_device *dev);
int wl_elinux_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd);

#endif /* _wl_elinux_h_ */
