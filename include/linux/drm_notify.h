/*
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __DRM_NOTIFY_H__
#define __DRM_NOTIFY_H__

#include <linux/notifier.h>

/* DRM extension event */
#define DRM_EXT_EVENT_BEFORE_BLANK	0xF0
#define DRM_EXT_EVENT_AFTER_BLANK		0xF1

/* DRM extension event data*/
enum {
	DRM_BLANK_UNBLANK,
	DRM_BLANK_POWERDOWN,
};

struct drm_ext_event {
	void *data;
};

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int drm_register_client(struct notifier_block *nb);
extern int drm_unregister_client(struct notifier_block *nb);
extern int drm_notifier_call_chain(unsigned long val, void *v);
#else
static inline int drm_register_client(struct notifier_block *nb)
{
	return 0;
}

static inline int drm_unregister_client(struct notifier_block *nb)
{
	return 0;
}

static inline int drm_notifier_call_chain(unsigned long val, void *v)
{
	return 0;
}
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#endif /* __DRM_NOTIFY_H__ */

