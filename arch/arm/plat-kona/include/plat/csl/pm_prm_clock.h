/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_prm_clock.h
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
*   @file   pm_prm_clock.h
*	@brief  Clock CSL header file
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

typedef enum {
	FREQ0 = 0,
	FREQ1 = 1,
	FREQ2 = 2,
	FREQ3 = 3,
	FREQ4 = 4,
	FREQ5 = 5,
	FREQ6 = 6,
	FREQ7 = 7
} CCU_FREQ_POLICY;

#define CCU_NULL	(CCU_ID_t)-1
#define NULL_PERI	(CHAL_CCU_PERIF_ID_t)-1
#define	NULL_CLK	(CHAL_CCU_CLOCK_ID_t)-1

typedef enum {
	PERI_POWER_OFF = 0,
	PERI_POWER_ON = 1
} PERI_POWER_STATE;

typedef enum {
	PERI_CLOCK_OFF = 0,
	PERI_CLOCK_ON = 1,
	PERI_CLOCK_AUTO = 2
} PERI_CLOCK_STATE;

typedef enum {
	CLOCK_CSL_FAIL = -1,	// Operation failed
	CLOCK_CSL_OK = 0	// Operation succeeded 
} CLOCK_CSL_RETURN;

#define write_reg(x, y)		(*(volatile unsigned int*)(x) = y)
#define read_reg(x)			(*(volatile unsigned int *)(x))

// initialize ASIC CCUs 
void csl_clock_init(CHAL_CCU_HANDLE_t * sysinterface_ccu_handle);

// initialize CCU ISR/HISR/task
void csl_clock_ccu_interrupt_init(void);

// enable CCU write permission
void csl_clock_write_enable(int flag);
int csl_clock_is_write_enabled(void);

// set or get peripheral clock state, divider or speed          
CLOCK_CSL_RETURN csl_clock_peri_set_clock_state(CCU_ID_t ccu_id,
						CHAL_CCU_PERIF_ID_t peri_id,
						CHAL_CCU_CLOCK_ID_t clock_id,
						PERI_CLOCK_STATE state);
PERI_CLOCK_STATE csl_clock_peri_get_clock_state(CCU_ID_t ccu_id,
						CHAL_CCU_PERIF_ID_t peri_id,
						CHAL_CCU_CLOCK_ID_t clock_id);
CLOCK_CSL_RETURN csl_clock_peri_set_clock_speed(CCU_ID_t ccu_id,
						CHAL_CCU_PERIF_ID_t peri_id,
						CHAL_CCU_CLOCK_ID_t clock_id,
						int freq);
CLOCK_CSL_RETURN csl_clock_peri_set_clock_divider(CCU_ID_t ccu_id,
						  CHAL_CCU_PERIF_ID_t peri_id,
						  CHAL_CCU_CLOCK_ID_t clock_id,
						  int pll, int pre_divisor,
						  int divisor, int fraction);
int csl_clock_peri_get_clock_speed(CCU_ID_t ccu_id, CHAL_CCU_PERIF_ID_t peri_id,
				   CHAL_CCU_CLOCK_ID_t clock_id);

// set or get current frequency ID of active CCU policy
CLOCK_CSL_RETURN csl_clock_ccu_set_freq_policy(CCU_ID_t ccu_id,
					       CCU_FREQ_POLICY freq_policy,
					       int synchronous);
CCU_FREQ_POLICY csl_clock_ccu_get_freq_policy(CCU_ID_t ccu_id);

// get number of frequency ID supported by a CCU
int csl_clock_ccu_get_num_of_freq_policy(CCU_ID_t ccu_id);

// get the speed(in Hz) of a CCU frequency ID
int csl_clock_ccu_get_freq_policy_speed(CCU_ID_t ccu_id,
					CCU_FREQ_POLICY freq_policy);

// convert frequency(in Hz) to CCU frequency ID
CCU_FREQ_POLICY csl_clock_convert_to_freq_policy(CCU_ID_t ccu_id, int freq);

// set or get active CCU policy
void csl_clock_set_active_ccu_policy(CCU_POLICY_t policy, int copy);
int csl_clock_get_active_ccu_policy(void);

// register or de-register callback for ccu freq policy change notification
void csl_clock_register_freq_policy_callback(CCU_ID_t ccu_id,
					     void (*func) (CCU_ID_t ccu_id));
void csl_clock_deregister_freq_policy_callback(CCU_ID_t ccu_id);

// reset the whole chip or a peripheral or get the reason of whole-chip reset
void csl_clock_system_reset(void);
void csl_clock_peri_reset(CCU_ID_t ccu_id, CHAL_CCU_PERIF_ID_t peri_id);
void csl_clock_get_system_reset_reason(int *reason);

// configure CCU clock monitoring
CLOCK_CSL_RETURN csl_clock_set_clock_monitor(CCU_ID_t ccu_id, int ctrl,
					     int sel);

// get CCU cHAL handle
CHAL_CCU_HANDLE_t get_ccu_chal_handle(CCU_ID_t ccu_id);

// get CCU ASIC name
char *get_ccu_name(CCU_ID_t ccu_id);

// display CCU, gating or divider snapshot
void clock_csl_snapshot(void);
void clock_gating_snapshot(void);
void clock_divider_pll_snapshot(void);
