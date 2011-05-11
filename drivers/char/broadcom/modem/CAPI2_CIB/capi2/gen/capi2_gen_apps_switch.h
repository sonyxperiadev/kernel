/****************************************************************************
*																			
*     Copyright (c) 2007-2008 Broadcom Corporation								
*																			
*   Unless you and Broadcom execute a separate written software license		
*   agreement governing use of this software, this software is licensed to you	
*   under the terms of the GNU General Public License version 2, available	
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").	
*																			
*   Notwithstanding the above, under no circumstances may you combine this	
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.	
*																			
****************************************************************************/
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/#ifndef CAPI2_GEN_MSG_AP_H
#define CAPI2_GEN_MSG_AP_H

#ifdef  DEVELOPMENT_CAPI2_WIN_UNIT_TEST
#define _DEF(a) AP_ ## a
#else
#define _DEF(a) a
#endif



void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data, Boolean is_testsim);
void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);
#endif
