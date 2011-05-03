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
/**
*
*   @file   capi2_comm.h
*
*   @brief  This file declare the function prototypes
*
****************************************************************************/
#ifndef CAPI2_COMM_H
#define CAPI2_COMM_H



#define INVALID_AT_CHAN	99

void CAPI2_CP_PostTestData(UInt32 testId, UInt32 param1);

void CAPI2_HandleCAPINotifyMsg(InterTaskMsg_t* inMsg);

void CAPI2_CP_DataInit(void);

void CAPI2_CP_DataRun(void);

void CAPI2_PostMsgToCAPI2Task(InterTaskMsg_t* inMsg);

void CAPI2_CP_PostATCData(UInt8 chan, char* cmdStr);

void CAPI2_SYS_ClientInit(void);

void CAPI2_SYS_ClientRun(void);

void CommsIpcInitialise(void);

void CAPI2_COMMS_RcvData(UInt8* data, UInt16 len);


#endif	//CAPI2_COMM_H
