/*******************************************************************************
Copyright 2012-2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _LINUX_EARLYSUSPEND_H
#define _LINUX_EARLYSUSPEND_H

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/notifier.h>
#endif
/* The early_suspend structure defines suspend and resume hooks to be called
 * when the user visible sleep state of the system changes, and a level to
 * control the order. They can be used to turn off the screen and input
 * devices that are not used for wakeup.
 * Suspend handlers are called in low to high level order, resume handlers are
 * called in the opposite order.
 * Drivers needing earlysuspend before FB need to set level below 50.
 * Currently FB gets fully suspended at level = 50. So, if you want
 * FB to be suspended first, set the level of driver > 50.
 */

enum {
	EARLY_SUSPEND_LEVEL_BLANK_SCREEN = 50,
	EARLY_SUSPEND_LEVEL_STOP_DRAWING = 100,
	EARLY_SUSPEND_LEVEL_DISABLE_FB = 150,
};
struct early_suspend {
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct list_head link;
	int level;
	void (*suspend)(struct early_suspend *h);
	void (*resume)(struct early_suspend *h);
#endif
};

#ifdef CONFIG_HAS_EARLYSUSPEND
void register_early_suspend(struct early_suspend *handler);
void unregister_early_suspend(struct early_suspend *handler);
#else
#define register_early_suspend(handler) do { } while (0)
#define unregister_early_suspend(handler) do { } while (0)
#endif
#endif

