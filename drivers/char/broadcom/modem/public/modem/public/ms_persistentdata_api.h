/*
	2007 Broadcom Corporation

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

//The following header files should be included before include ms_persistendata_api.h
// "external_synonym.h"
// "scttypes.h"
// "ms.h"
// "timezone_data.h"

//**************************************************
extern void MS_InitialisePersistentDataStore(void);

#ifdef _DEFINE_IPCCP_
extern void IPCCP_LockPersistentDataStore(void);
extern void IPCCP_UnlockPersistentDataStore(void);
extern U32 IPCCP_WritePDS_ToFFS(void);
extern U32 IPCCP_ReadFFS_ToPDS(void);

extern void IPCCP_PDS_ReadMsDBNvData(void *theMsDBNvData);
extern void IPCCP_PDS_WriteMsDBNvData(void *theMsDBNvData);
extern U32 IPCCP_PDS_MsDBNvData_CopyFromFile(void);
extern U32 IPCCP_PDS_MsDBNvData_CopyToFile(void);

#define   TRACK_STACK_USAGE		
#ifdef TRACK_STACK_USAGE
extern void IPCCP_ReadStackMaxUsed(U32 *prev_min_unused);
extern void IPCCP_WriteStackMaxUsed(U32 *cur_min_unused);
extern void IPCCP_InitStackMaxUsed(void);
#define NUM_TASK_HISR 92
#endif  // #ifdef TRACK_STACK_USAGE


#endif

#endif // __COMMS_IPC_PERSISTENT_DATA_STORE_H__
