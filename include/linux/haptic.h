/*
 *  Driver model for haptic
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_HAPTIC_H_INCLUDED
#define __LINUX_HAPTIC_H_INCLUDED

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>

struct device;
/*
 * Motor Core
 */

enum haptic_value {
	HAPTIC_OFF	= 0,
	HAPTIC_HALF	= 127,
	HAPTIC_FULL	= 255,
};

struct haptic_classdev {
	const char		*name;
	int			value;
#define HAPTIC_SUSPENDED		(1 << 0)
	int			flags;

	/* Set haptic value */
	/* Must not sleep, use a workqueue if needed */
	void			(*set)(struct haptic_classdev *self,
					  enum haptic_value value);
	/* Get haptic value */
	enum haptic_value	(*get)(struct haptic_classdev *self);

	ssize_t (*show_enable)(struct device *dev,
			struct device_attribute *attr, char *buf);
	ssize_t (*store_enable)(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t size);

	ssize_t (*store_oneshot)(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t size);

	ssize_t (*show_level)(struct device *dev,
			struct device_attribute *attr, char *buf);
	ssize_t (*store_level)(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t size);

	ssize_t (*show_level_max)(struct device *dev,
			struct device_attribute *attr, char *buf);

	struct device		*dev;
	struct list_head	node;	/* Motor Device list */
};

extern int haptic_classdev_register(struct device *parent,
				 struct haptic_classdev *haptic_cdev);
extern void haptic_classdev_unregister(struct haptic_classdev *lcd);
extern void haptic_classdev_suspend(struct haptic_classdev *haptic_cdev);
extern void haptic_classdev_resume(struct haptic_classdev *haptic_cdev);

/*
 * Generic and gpio haptic platform data for describing haptic names.
 */
struct haptic_platform_data {
	const char	*name;
	int		pwm_timer;
	int		gpio;
	void	(*setup_pin)(void);
	u8		active_low;
	int		ldo_level;
};

#endif	/* __LINUX_HAPTIC_H_INCLUDED */
