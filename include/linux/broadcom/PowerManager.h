/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/PowerManager.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  PowerManager.h
*
*  PURPOSE:
*
*  This file defines the interface to the Power manager.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(_POWER_MANAGER_H_)
#define _POWER_MANAGER_H_

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */
#define PM_MAGIC   'p'

#define PM_CMD_FIRST               0x80
#define PM_CMD_SETPOWER            0x80
#define PM_CMD_GETPOWER            0x81
#define PM_CMD_GETSTATUS           0x82
#define PM_CMD_GETEVENT            0x83
#define PM_CMD_LAST                0x83

typedef enum {			/* If you change this, please update PM_PowerLevelTable as well */
	PM_PWR_OFF,		/* Virtual level where everything is powered off */
	PM_PWR_CHARGING,	/* ARM sleep mode enabled, Wireless powered off completely */
	PM_PWR_STANDBY,		/* ARM sleep mode enabled, Wireless power save on */
	PM_PWR_HIGH,		/* ARM Running at highest rate, Wireless power save on */
	PM_PWR_FULL,		/* ARM Running at highest rate, Wireless power save off */

	PM_NUM_POWER_LEVELS	/* Number of system power levels */
} PM_PowerLevel;

#define PM_BATT_MAX_LEVELS   5	/* number of quantization levels in battery level table */

typedef struct {
	int battLevel;		/* battery bar level */
	int chgPluggedIn;	/* TRUE when charger plugged in */
	int chgState;		/* 1 when battery fully charged */
	int chgId;		/* ID of charger */
} PM_Status;

typedef enum {
	PM_EVENT_STATUS_UPDATE,
	PM_EVENT_ON_KEY_PRESSED,
	PM_EVENT_ON_KEY_HELD_FOR_1S,
} PM_UserEvent;

#if defined(__KERNEL__)
typedef enum {
	PM_EVENT_INTERNAL,	/* Internal Power Manager event */
	PM_EVENT_PMU_CHIP,	/* Event from PMU chip */
	PM_EVENT_BATTMGRTASK,	/* Battery Manager event */
} PM_Event;

typedef enum {			/* If you change this, please update PM_ComponentTable as well */
	PM_COMP_LCD = 0,
	PM_COMP_CAMERA,
	PM_COMP_USB,
	PM_COMP_ETHERNET,
	PM_COMP_WIFI,
	PM_COMP_AUDIO,
	PM_COMP_BLUETOOTH,
	PM_COMP_SIM,

	PM_NUM_COMPONENTS
} PM_Component;

typedef enum {			/* If you change this, please update PM_CompPowerLevelTable as well */
	PM_COMP_PWR_OFF,	/* Uninitialized and power off.  Should not draw any power. */
	PM_COMP_PWR_STANDBY,	/* Device in low power state or possibly off.  Allowed to interrupt host. */
	PM_COMP_PWR_ON,		/* Fully powered on and initialized. */
} PM_CompPowerLevel;

typedef struct {
	int (*update_power_level) (PM_CompPowerLevel compPowerLevel,
				   PM_PowerLevel sysPowerLevel);
} PM_Comp_Ops_t;
#endif

#define PM_IOCTL_SETPOWER   _IOW(PM_MAGIC, PM_CMD_SETPOWER, PM_PowerLevel)
#define PM_IOCTL_GETPOWER   _IOR(PM_MAGIC, PM_CMD_GETPOWER, PM_PowerLevel)
#define PM_IOCTL_GETSTATUS  _IOR(PM_MAGIC, PM_CMD_GETSTATUS, PM_Status)
#define PM_IOCTL_GETEVENT   _IOR(PM_MAGIC, PM_CMD_GETEVENT, PM_UserEvent)

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#if defined(__KERNEL__)

#if defined(CONFIG_BCM_POWER_MANAGER)

int pm_submit_event(PM_Event event, uint32_t data1, uint32_t data2,
		    uint32_t data3);
int pm_submit_event_and_wait(PM_Event event, uint32_t data1, uint32_t data2,
			     uint32_t data3);
int pm_submit_delayed_event(PM_Event event, uint32_t delay, uint32_t data1,
			    uint32_t data2, uint32_t data3);
int pm_register_component(PM_Component component, PM_Comp_Ops_t *ops);
void pm_unregister_component(PM_Component component, PM_Comp_Ops_t *ops);
void pm_status_event(void);
void pm_enable_early_shutoff(void);

#else

#define pm_submit_event(event, data1, data2, data3) ((void)(event), (void)(data1), (void)(data2), (void)(data3), 0)
#define pm_submit_event_and_wait(event, data1, data2, data3) ((void)(event), (void)(data1), (void)(data2), (void)(data3), 0)
#define pm_submit_delayed_event(event, delay, data1, data2, data3) ((void)(event), (void)(delay), (void)(data1), (void)(data2), (void)(data3), 0)
#define pm_status_event() (0)

#define pm_register_component(component, ops) ((ops)->update_power_level(PM_COMP_PWR_ON, PM_PWR_FULL))
#define pm_unregister_component(component, ops) ((ops)->update_power_level(PM_COMP_PWR_ON, PM_PWR_OFF))

#endif

#endif

#endif /* _POWER_MANAGER_H_ */
