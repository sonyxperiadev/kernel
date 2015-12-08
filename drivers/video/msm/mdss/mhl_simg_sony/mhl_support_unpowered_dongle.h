/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_support_unpowered_dongle.h
 *
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_SUPPORT_UNPOWERED_DONGLE_H__
#define __MHL_SUPPORT_UNPOWERED_DONGLE_H__

#ifdef CONFIG_MHL_SUPPORT_UNPOWERED_DONGLE
/* API from device */
void mhl_unpowered_power_off(void);
bool mhl_unpowered_is_vbus_disabled(void);
void mhl_unpowered_disconnection(void);
void mhl_unpowered_detected_1k(void);
void mhl_unpowered_start(void);
void mhl_unpowered_init(struct device *dev);
void mhl_unpowered_sysfs_release(void);

/* API from Cbus Control */
void mhl_unpowered_notify_devcap_read_done(void);

#else
static inline void mhl_unpowered_power_off(void) {}
static inline bool mhl_unpowered_is_vbus_disabled(void) { return false; }
static inline void mhl_unpowered_disconnection(void) {}
static inline void mhl_unpowered_detected_1k(void) {}
static inline void mhl_unpowered_start(void) {}
static inline void mhl_unpowered_init(struct device *dev) {}
static inline void mhl_unpowered_sysfs_release(void) {}

static inline void mhl_unpowered_notify_devcap_read_done(void) {}
#endif /* CONFIG_MHL_SUPPORT_UNPOWERED_DONGLE */

#endif /* __MHL_SUPPORT_UNPOWERED_DONGLE_H__ */

