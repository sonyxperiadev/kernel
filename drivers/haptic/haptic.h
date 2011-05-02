/*
 *  Haptic Core
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __HAPTIC_H_INCLUDED
#define __HAPTIC_H_INCLUDED

#include <linux/device.h>
#include <linux/rwsem.h>
#include <linux/haptic.h>

static inline void haptic_set_value(struct haptic_classdev *haptic_cdev,
					enum haptic_value value)
{
	if (value > HAPTIC_FULL)
		value = HAPTIC_FULL;
	haptic_cdev->value = value;
	if (!(haptic_cdev->flags & HAPTIC_SUSPENDED))
		haptic_cdev->set(haptic_cdev, value);
}

static inline int haptic_get_value(struct haptic_classdev *haptic_cdev)
{
	return haptic_cdev->value;
}

#endif	/* __HAPTIC_H_INCLUDED */
