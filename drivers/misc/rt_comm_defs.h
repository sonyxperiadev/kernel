/* drivers/mfd/richtek/rt_comm_defs.h
 * Richtek Common Definitions
 *
 * Copyright (C) 2012
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __RICHTEK_RT_COMM_DEFS_H
#define __RICHTEK_RT_COMM_DEFS_H

#define ERR(format, args...) \
	printk(KERN_ERR "%s:%s() line-%d: " format, DEVICE_NAME, __FUNCTION__, __LINE__, ## args)
#define WARNING(format, args...) \
	printk(KERN_WARNING "%s:%s() line-%d: " format, DEVICE_NAME, __FUNCTION__, __LINE__, ## args)
#ifdef CONFIG_RT_SHOW_INFO
#define INFO(format, args...) \
	printk(KERN_INFO "%s:%s() line-%d: " format, DEVICE_NAME, __FUNCTION__, __LINE__, ## args)
#else
#define INFO(format,args...)
#endif

#ifdef CONFIG_RT_SYSFS_DBG
#define __ATTR_RW(_name) __ATTR(_name, 0666, _name##_show, _name##_store)
#define __ATTR_WO(_name) __ATTR(_name, 0222, NULL, _name##_store)
#else
#define __ATTR_RW(_name) __ATTR(_name, 0644, _name##_show, _name##_store)
#define __ATTR_WO(_name) __ATTR(_name, 0200, NULL, _name##_store)
#endif

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) // > 3.0.0

   #else // 2.6.35 ~ 3.0.0

   #endif
#else // < 2.6.35

#endif // Linux Version

#endif
