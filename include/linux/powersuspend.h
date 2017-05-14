/* include/linux/powersuspend.h
 *
 * Copyright (C) 2007-2008 Google, Inc.
 * Copyright (C) 2013 Paul Reioux 
 *
 * Modified by Jean-Pierre Rasquin <yank555.lu@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_POWERSUSPEND_H
#define _LINUX_POWERSUSPEND_H

#include <linux/list.h>

#define POWER_SUSPEND_INACTIVE	0
#define POWER_SUSPEND_ACTIVE	1

#define POWER_SUSPEND_USERSPACE	1	// Use fauxclock as trigger
#define POWER_SUSPEND_PANEL	2	// Use display panel state as hook

struct power_suspend {
	struct list_head link;
	void (*suspend)(struct power_suspend *h);
	void (*resume)(struct power_suspend *h);
};

void register_power_suspend(struct power_suspend *handler);
void unregister_power_suspend(struct power_suspend *handler);

void set_power_suspend_state_panel_hook(int new_state);

#endif

