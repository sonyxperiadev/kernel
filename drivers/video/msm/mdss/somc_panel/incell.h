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
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __INCELL_H__
#define __INCELL_H__

#include <linux/types.h>

#define INCELL_OK                ((int)0)

/* We need to discuss the bellow error name */
#define INCELL_ERROR             ((int)(-1))

#define INCELL_ALREADY_LOCKED    ((int)(-2))
#define INCELL_ALREADY_UNLOCKED  ((int)(-3))
#define INCELL_EBUSY             ((int)(-4))
#define INCELL_EALREADY          ((int)(-5))

#define INCELL_POWER_ON          ((bool)true)
#define INCELL_POWER_OFF         ((bool)false)
#define INCELL_FORCE             ((bool)true)
#define INCELL_UNFORCE           ((bool)false)

/* fb_notifier call type */
#define FB_NOTIFIER_PRE		((bool)true)
#define FB_NOTIFIER_POST	((bool)false)
/* A hardware display extension blank early change occured */
#define FB_EXT_EARLY_EVENT_BLANK	0xF0
/* A hardware display extension blank change occurred */
#define FB_EXT_EVENT_BLANK		0xF1

typedef struct {
	bool touch_power;
	bool display_power;
} incell_pw_status;

typedef enum {
	INCELL_DISPLAY_HW_RESET,
	INCELL_DISPLAY_OFF,
	INCELL_DISPLAY_ON,
} incell_intf_mode;

typedef enum {
	INCELL_DISPLAY_POWER_UNLOCK,
	INCELL_DISPLAY_POWER_LOCK,
} incell_pw_lock;

typedef enum {
	INCELL_DISPLAY_EWU_DISABLE,
	INCELL_DISPLAY_EWU_ENABLE,
} incell_ewu_mode;

/* touch I/F data information for incell */
/* touch I/F or not */
#define INCELL_TOUCH_RUN	((bool)true)
#define INCELL_TOUCH_IDLE	((bool)false)

/* status to adjust power for incell panel or not */
typedef enum {
	INCELL_WORKER_OFF,
	INCELL_WORKER_PENDING,
	INCELL_WORKER_ON,
} incell_worker_state;

/*
 * Incell status change mode
 *
 * SP means the below.
 * S : System
 * P : Power
 */
typedef enum {
	INCELL_STATE_NONE,
	INCELL_STATE_S_OFF,
	INCELL_STATE_P_OFF,
	INCELL_STATE_SP_OFF,
	INCELL_STATE_S_ON,
	INCELL_STATE_P_ON,
	INCELL_STATE_SP_ON,
} incell_state_change;

/* incell status */
typedef enum {
	INCELL_STATE_SLE000_P0,
	INCELL_STATE_SLE000_P1,
	INCELL_STATE_SLE001_P0,
	INCELL_STATE_SLE001_P1,
	INCELL_STATE_SLE010_P0,
	INCELL_STATE_SLE010_P1,
	INCELL_STATE_SLE011_P0,
	INCELL_STATE_SLE011_P1,
	INCELL_STATE_SLE100_P0,
	INCELL_STATE_SLE100_P1,
	INCELL_STATE_SLE101_P0,
	INCELL_STATE_SLE101_P1,
	INCELL_STATE_SLE110_P0,
	INCELL_STATE_SLE110_P1,
	INCELL_STATE_SLE111_P0,
	INCELL_STATE_SLE111_P1,
} incell_state;

/* How to send power sequence */
typedef enum {
	POWER_OFF_EXECUTE,
	POWER_OFF_SKIP,
	POWER_ON_EXECUTE,
	POWER_ON_SKIP,
	POWER_ON_EWU_SEQ,
} incell_pw_seq;

/* control parameters for incell panel */
struct incell_ctrl {
	incell_state state;
	incell_state_change change_state;
	incell_pw_seq seq;

	bool incell_intf_operation;
	incell_intf_mode intf_mode;

	incell_worker_state worker_state;
	struct work_struct incell_work;
};

/**
 * @brief Get incell power status.
 * @param[in] power_status : touch_power/display_power <br>
 *            INCELL_POWER_ON  : Power on state <br>
 *            INCELL_POWER_OFF : Power off state.
 * @return INCELL_OK     : Get power status successfully <br>
 *         INCELL_ERROR  : Failed to get power status.
 */
int incell_get_power_status(incell_pw_status *power_status);

/**
 * @brief Display control mode.
 * @param[in] mode : INCELL_DISPLAY_HW_RESET(execute on/off) <br>
 *                   INCELL_DISPLAY_OFF(touch is working) <br>
 *                   INCELL_DISPLAY_ON(only LCD on)
 * @param[in] force : INCELL_FORCE   - Forcibly execute an interface <br>
 *                    INCELL_UNFORCE - Depending power lock or not.
 * @return INCELL_OK        : success <br>
 *         INCELL_ERROR     : error detected <br>
 *         INCELL_EBUSY     : try lock failed.
 * @attention You cannot call this function from same fb_blank context.
 */
int incell_control_mode(incell_intf_mode mode, bool force);

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
int incell_power_lock_ctrl(incell_pw_lock lock,
		incell_pw_status *power_status);

/**
 * @brief Easy wake up mode (EWU) control.
 * @param[in] ewu : INCELL_DISPLAY_EWU_ENABLE (EWU mode enabled) <br>
 *                  INCELL_DISPLAY_EWU_DISABLE (EWU mode disabled)
 */
void incell_ewu_mode_ctrl(incell_ewu_mode ewu);

/**
 * @brief Force incell power state change to SP mode
 */
void incell_force_sp_on(void);

#endif /* __INCELL_H__ */

