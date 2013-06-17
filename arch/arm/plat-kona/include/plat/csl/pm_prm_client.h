/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_prm_client.h
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
*   @file   pm_prm_client.h
*	@brief  Client Manager header file
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

// initialize client table
void client_init(void);

// register or de-register a client
CLIENT_ID client_register(char *client_name);
PRM_RETURN client_deregister(CLIENT_ID client_id);

// check if this is a valid client ID
int check_client_id(CLIENT_ID client_id);

// register or de-register OPP state change pre/post callback
PRM_RETURN client_opp_change_prepare_callbk_register(CLIENT_ID client_id,
						     PERFORMANCE_ID resource_id,
						     void (*func) (OPP_STATE
								   opp));
PRM_RETURN client_opp_change_prepare_callbk_deregister(CLIENT_ID client_id,
						       PERFORMANCE_ID
						       resource_id,
						       void (*func) (OPP_STATE
								     opp));
PRM_RETURN client_opp_change_finish_callbk_register(CLIENT_ID client_id,
						    PERFORMANCE_ID resource_id,
						    void (*func) (OPP_STATE
								  opp));
PRM_RETURN client_opp_change_finish_callbk_deregister(CLIENT_ID client_id,
						      PERFORMANCE_ID
						      resource_id,
						      void (*func) (OPP_STATE
								    opp));

// register or de-register sleep state change callback
PRM_RETURN client_sleep_state_change_callbk_register(CLIENT_ID client_id,
						     PERFORMANCE_ID resource_id,
						     SLEEP_STATE state,
						     void (*func) (SLEEP_STATE
								   state));
PRM_RETURN client_sleep_state_change_callbk_deregister(CLIENT_ID client_id,
						       PERFORMANCE_ID
						       resource_id,
						       SLEEP_STATE state,
						       void (*func) (SLEEP_STATE
								     state));

// call client-registered callbacks for pre/post OPP or sleep state change notification
void client_opp_change_prepare_callbk(PERFORMANCE_ID resource_id,
				      OPP_STATE opp);
void client_opp_change_finish_callbk(PERFORMANCE_ID resource_id, OPP_STATE opp);
void client_sleep_state_change_callbk(PERFORMANCE_ID resource_id,
				      SLEEP_STATE sleep);

// set per-client allowed wake-up latency for a resource
void client_set_allowed_wakeup_latency(CLIENT_ID client_id,
				       PERFORMANCE_ID resource_id, int latency);
// check if HW wakeup latency can be tolerated by clients
int client_check_allowed_wakeup_latency(PERFORMANCE_ID resource_id,
					int hw_latency);

// set per-client allowed sleep state for a resource
void client_set_allowed_sleep_state(CLIENT_ID client_id,
				    PERFORMANCE_ID resource_id,
				    SLEEP_STATE state);
// check if a sleep state is allowed by clients
int client_check_allowed_sleep_state(PERFORMANCE_ID resource_id,
				     SLEEP_STATE state);

// save client OPP request in client table
int client_save_opp_request(CLIENT_ID client_id, PERFORMANCE_ID resource_id,
			    OPP_STATE opp);

// get next outstanding OPP request in client table
int client_find_next_opp_request(PERFORMANCE_ID resource_id,
				 CLIENT_ID * client_id, OPP_STATE * opp);

// get client ASIC name
char *get_client_name(CLIENT_ID client_id);

// display snapshot of client table
void client_manager_snapshot(void);
