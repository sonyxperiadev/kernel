/*
 * Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
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

#ifndef __INCELL_H__
#define __INCELL_H__

#include <linux/types.h>

#define INCELL_OK                ((int)0)
#define INCELL_ERROR             ((int)(-1))

#define INCELL_ALREADY_LOCKED    ((int)(-2))
#define INCELL_ALREADY_UNLOCKED  ((int)(-3))
#define INCELL_EBUSY             ((int)(-4))
#define INCELL_EALREADY          ((int)(-5))

#define INCELL_POWER_ON          ((bool)true)
#define INCELL_POWER_OFF         ((bool)false)
#define INCELL_FORCE             ((bool)true)
#define INCELL_UNFORCE           ((bool)false)

typedef struct {
	bool touch_power;
	bool display_power;
} incell_pw_status;

typedef enum {
	INCELL_TOUCH_RESET,
	INCELL_CONT_SPLASH_TOUCH_ENABLE,
} incell_intf_mode;

typedef enum {
	INCELL_DISPLAY_POWER_UNLOCK,
	INCELL_DISPLAY_POWER_LOCK,
} incell_pw_lock;

typedef enum {
	INCELL_TOUCH_TYPE_DEFAULT = 0,
	INCELL_TOUCH_TYPE_CLEARPAD = 1,
	INCELL_TOUCH_TYPE_TCM = 2,
} incell_touch_type;

/* Compatibility with older incell */
#define INCELL_DISPLAY_HW_RESET		INCELL_TOUCH_RESET

/**
 * @brief Get incell power status.
 * @param[in] power_status : touch_power/display_power <br>
 *            INCELL_POWER_ON  : Power on state <br>
 *            INCELL_POWER_OFF : Power off state.
 * @return INCELL_OK     : Get power status successfully <br>
 *         INCELL_ERROR  : Failed to get power status.
 */
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_get_power_status(incell_pw_status *power_status);
#else
static inline int incell_get_power_status(incell_pw_status *power_status) {
		power_status->display_power = INCELL_POWER_ON;
		power_status->touch_power = INCELL_POWER_ON;
		return INCELL_OK;}
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

/**
 * @brief Display control mode.
 * @param[in] mode : INCELL_TOUCH_RESET(execute on/off) <br>
 * @param[in] force : INCELL_FORCE   - Forcibly execute an interface <br>
 *                    INCELL_UNFORCE - Depending power lock or not.
 * @return INCELL_OK        : success <br>
 *         INCELL_ERROR     : error detected <br>
 *         INCELL_EBUSY     : try lock failed.
 * @attention You cannot call this function from same fb_blank context.
 */
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_control_mode(incell_intf_mode mode, bool force);
#else
static inline int incell_control_mode(incell_intf_mode mode, bool force) {return INCELL_OK;}
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

/**
 * @brief LCD/Touch Power lock control.
 * @param[in] lock : INCELL_DISPLAY_POWER_UNLOCK(not keep power supply) <br>
 *                   INCELL_DISPLAY_POWER_LOCK(keep power supply) <br>
 * @param[out] power_status : return power state of incell_pw_status
 *                   by calling incell_get_power_status function.
 * @return INCELL_OK               : success <br>
 *         INCELL_ERROR            : error detected <br>
 *         INCELL_ALREADY_LOCKED   : Already power locked <br>
 *         INCELL_ALREADY_UNLOCKED : Already power unlocked.
 */
#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_power_lock_ctrl(incell_pw_lock lock,
		incell_pw_status *power_status);
#else
static inline int incell_power_lock_ctrl(incell_pw_lock lock,
					 incell_pw_status *power_status) {return INCELL_OK;}
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_get_panel_name(void);
#else
static inline int incell_get_panel_name(void) {return 0; }
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern bool incell_get_system_status(void);
#else
static inline bool incell_get_system_status(void) {return true; }
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_get_display_aod(void);
#else
static inline int incell_get_display_aod(void) {return 0; }
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_get_display_sod(void);
#else
static inline int incell_get_display_sod(void) {return 0; }
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
extern int incell_get_display_pre_sod(void);
#else
static inline int incell_get_display_pre_sod(void) {return 0; }
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
int somc_panel_external_control_touch_power(bool enable);
#else
static inline int somc_panel_external_control_touch_power(bool enable)
{ return 0; }
#endif

#ifdef CONFIG_DRM_SDE_SPECIFIC_PANEL
bool incell_touch_is_compatible(incell_touch_type type);
#else
static inline bool incell_touch_is_compatible(incell_touch_type) {
	return true;
}
#endif /* CONFIG_DRM_SDE_SPECIFIC_PANEL */

#endif /* __INCELL_H__ */

