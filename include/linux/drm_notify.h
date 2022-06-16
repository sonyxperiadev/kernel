/*
 * Copyright 2021 Sony Corporation.
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

extern int drm_register_client(struct notifier_block *nb);
extern int drm_unregister_client(struct notifier_block *nb);
extern int drm_notifier_call_chain(unsigned long val, void *v);

#endif /* __DRM_NOTIFY_H__ */
