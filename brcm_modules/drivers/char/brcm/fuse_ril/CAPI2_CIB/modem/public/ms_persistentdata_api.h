/*
	?007 Broadcom Corporation

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed to you
	under the terms of the GNU General Public License version 2, available
	at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").


   Notwithstanding the above, under no circumstances may you combine this
   software in any way with any other Broadcom software provided under a license
   other than the GPL, without Broadcom's express prior written consent.
*/

//============================================================
// ms_persistentdata.h
//
// A mechanism to allow the CP to maintains read and write parameters which will be
// persistent between power cycles
//============================================================

#ifndef __COMMS_IPC_PERSISTENT_DATA_STORE_H__
#define __COMMS_IPC_PERSISTENT_DATA_STORE_H__

//#include "external_synonym.h"
//#include "scttypes.h"
//#include "ms.h"
//#include "timezone_data.h"

//**************************************************
extern void MS_InitialisePersistentDataStore(void);

#ifdef _DEFINE_IPCCP_
extern void IPCCP_LockPersistentDataStore(void);
extern void IPCCP_UnlockPersistentDataStore(void);
extern void IPCCP_ReadEquivPLMNList(T_EQUIV_PLMN_LIST *theEquivPLMNList);
extern void IPCCP_WriteEquivPLMNList(T_EQUIV_PLMN_LIST *theEquivPLMNList);
extern void IPCCP_ReadNITZNetworkName(TIMEZONE_Network_Name_Data_t *nitz_name_data);
extern void IPCCP_WriteNITZNetworkName(const TIMEZONE_Network_Name_Data_t *nitz_name_data);
extern void IPCCP_ReadNetPar(T_NETPAR *theNetPar);
extern void IPCCP_WriteNetPar(T_NETPAR *theNetPar);
extern void IPCCP_ReadRPLMN_Rat(T_RAT *theRPLMN_Rat);
extern void IPCCP_WriteRPLMN_Rat(T_RAT *theRPLMN_Rat);
extern U32 IPCCP_GPSRead(char *Buf, U32 BufSize, U32 Offset);
extern U32 IPCCP_GPSWrite(const char *Buf, U32 BufSize, U32 Offset);
extern U32 IPCCP_GPSPersistentDataSize(void);
extern U32 IPCCP_GPSPersistentDataOffset(void);
extern U32 IPCCP_WritePDS_ToFFS(void);
extern U32 IPCCP_ReadFFS_ToPDS(void);

#define   TRACK_STACK_USAGE		
#ifdef TRACK_STACK_USAGE
extern void IPCCP_ReadStackMaxUsed(U32 *prev_min_unused);
extern void IPCCP_WriteStackMaxUsed(U32 *cur_min_unused);
extern void IPCCP_InitStackMaxUsed(void);
#define NUM_TASK_HISR 92
#endif  // #ifdef TRACK_STACK_USAGE


#endif

#endif // __COMMS_IPC_PERSISTENT_DATA_STORE_H__
