/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_prm_sleep.h
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
/**
*
*   @file   pm_prm_sleep.h
*	@brief  Sleep CSL header file
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

typedef enum {
#if defined(_RHEA_)
	MODEM_DOMAIN,		// MODEM domain
	ARM_CORE_DOMAIN,	// ARM core domain
	ARM_SUB_DOMAIN,		// ARM Subsystem domain
	HUB_AON_DOMAIN,		// HUB AON doamin
	HUB_SWITCH_DOMAIN,	// HUB switchable domain
	MM_DOMAIN,		// MM domain
	BMDEM_CP_DOMAIN,	// BMODEM CP domain
	BMDEM_DSP_DOMAIN,	// BMODEM DSP domain
	BMDEM_WCDMA_DOMAIN,	// BMODEM WCDMA doamin
#elif defined(_HERA_)
	ARM_CORE_DOMAIN,	// ARM core domain
	ARM_SUB_DOMAIN,		// ARM Subsystem domain
	HUB_AON_DOMAIN,		// HUB AON doamin
	HUB_SWITCH_DOMAIN,	// HUB switchable domain
	MM_DOMAIN,		// MM domain
	MM_SUB_DOMAIN,		// MM subsystem domain
#endif
} POWER_DOMAIN;

typedef enum {
	PM_ACTIVE = 0,		// run state 
	PM_SUSPEND = 1,		// WFI state of A9 or R4
	PM_SUSPEND_DORMANT = 2,	// dormant state of A9 with 26MHz on
	PM_DEEPSLEEP_DORMANT = 3,	// dormant state of A9 with 26MHz off (deepest sleep state of A9)
	PM_RETENTION = 4,	// retention state (deepest sleep state of all domains except A9 and MM)
	PM_OFF = 5,		// OFF state of MM (deepest sleep state of MM)
} PM_SLEEP_STATE;

typedef struct {
	CHAL_PM_POLICY_t modem;
	CHAL_PM_POLICY_t ARMCore;
	CHAL_PM_POLICY_t ARMSub;
	CHAL_PM_POLICY_t hubAON;
	CHAL_PM_POLICY_t hubSwitchable;
	CHAL_PM_POLICY_t MM;
} PM_EVENT_POLICY_t;

#if defined(_RHEA_)
typedef struct {
	CHAL_PM_POLICY_t cp;
	CHAL_PM_POLICY_t dsp;
	CHAL_PM_POLICY_t wcdma;
} PM_BMDM_EVENT_POLICY_t;
#endif

typedef enum {
	PM_CSL_FAIL = -1,	// Operation failed
	PM_CSL_OK = 0		// Operation succeeded 
} PM_CSL_RETURN;

// initialize ASIC Power Manager 
void csl_pm_init(CHAL_HANDLE sysinterface_pm_handle,
		 CHAL_HANDLE sysinterface_pm_bmdm_handle);

// configure an event
PM_CSL_RETURN csl_pm_set_event_policy(CHAL_PM_EVENT_t event_id,
				      PM_EVENT_POLICY_t * policy);
PM_CSL_RETURN csl_pm_set_event_enable(CHAL_PM_EVENT_t event_id, int pos,
				      int neg);

// configure keep-alive PM policy
void csl_pm_enable_keepalive_policy(void);

// switch to a different PM policy
void csl_pm_enable_run_policy(CHAL_PM_POLICY_t pm_policy);

// force A9 to selected sleep state
PM_CSL_RETURN csl_pm_a9_sleep(PM_SLEEP_STATE state);

// force a non-processor power domain to selected sleep state
PM_CSL_RETURN csl_pm_sleep(POWER_DOMAIN domain, PM_SLEEP_STATE state);

// get PM cHAL handle
CHAL_HANDLE get_pm_chal_handle(void);

// display ASIC Power Manager snapshot
void sleep_csl_snapshot(void);

// register callback for wakeup event notification
void
csl_pm_register_wakeup_event_callback(void (*func) (CHAL_PM_EVENT_t event));

// process active event condition
void csl_pm_process_event(void);

#if defined(_RHEA_)
// configure an event
PM_CSL_RETURN csl_pm_bmdm_set_event_policy(CHAL_BMDM_PM_EVENT_t event_id,
					   PM_BMDM_EVENT_POLICY_t * policy);
PM_CSL_RETURN csl_pm_bmdm_set_event_enable(CHAL_BMDM_PM_EVENT_t event_id,
					   int pos, int neg);

// configure keep-alive PM policy
void csl_pm_bmdm_enable_keepalive_policy();

// switch to a different PM policy
void csl_pm_bmdm_enable_run_policy(CHAL_PM_POLICY_t pm_policy);

// force R4 to selected sleep state
PM_CSL_RETURN csl_pm_bmdm_r4_sleep(PM_SLEEP_STATE state);

// force a BMODEM power domain to selected sleep state
PM_CSL_RETURN csl_pm_bmdm_sleep(POWER_DOMAIN domain, PM_SLEEP_STATE state);

// get BMODEM PM cHAL handle
CHAL_HANDLE get_pm_bmdm_chal_handle(void);

// register callback for wakeup event notification
void
csl_pm_bmdm_register_wakeup_event_callback(void (*func)
					   (CHAL_BMDM_PM_EVENT_t event));

// process active event condition
void csl_pm_bmdm_process_event(void);

#endif
