//**************************************************************************************
// Copyright 1999-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or its licensors, 
// and may only be used, duplicated, modified or distributed pursuant to the terms and 
// conditions of a separate, written license agreement executed between you and Broadcom 
// (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants 
// no license (express or implied), right to use, or waiver of any kind with respect to the 
// Software, and Broadcom expressly reserves all rights in and to the Software and all 
// intellectual property rights therein.  
// IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
// 
// Except as expressly set forth in the Authorized License,
// 
// 1.This program, including its structure, sequence and organization, constitutes 
//   the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
//   protect the confidentiality thereof, and to use this information only in connection 
//   with your use of Broadcom integrated circuit products.
// 2.TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
//   FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, 
//   IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
//   FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET 
//   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//   RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
// 3.TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
//   BE LIABLE FOR (i) CONSEQUENTIAL.
//***************************************************************************************/
/**
*
*   @file   sys_msc_rpc.h
*
*   @brief  This file defines the usb host msc apis.
*
****************************************************************************/

#ifndef RPC_MSC_MSG_H
#define RPC_MSC_MSG_H

/**
* @addtogroup  USBIpcApi
* @{
*/

typedef struct sysMscRpcInfo
{
    UInt32 sectorSize; //! Size in bytes of a sector in the drive
    UInt32 sectorCount;//! Number of sectors in the drive
    UInt32 headCount;//! Number of heads in the drive
    void *drvCtx;//! driver context
} sysMscRpcInfo_t;

typedef void (*sysMscRpc_Media_cb)(Boolean insert, sysMscRpcInfo_t *info);

//!
//! Function Name:  sysMscRpc_writeSectors 
//!
//! Description:  This sends a write sectors through the IPC to the USB stack on the CP 
//!             which issues the MSC write sectors to the Megasim device 
//!
//! \param[in]   void *buffer - Data to be written to the device
//! \param[in]   UInt32 startSector - start of sector write
//! \param[in]   UInt32 count - number of sectors to write
//! \param[in]   void *vol - context for the host stack
//!
//! \return int result of the write
//!
int sysMscRpc_WriteSectors(void *buffer, UInt32 startSector, int count, void *vol);

//!
//! Function Name:  sysMscRpc_readSectors 
//!
//! Description:  This sends a read sectors through the IPC to the USB stack on the CP 
//!             which issues the MSC read sectors to the Megasim device 
//!
//! \param[inout]   void *buffer - Data to be read from the device
//! \param[in]   UInt32 startSector - start of sector write
//! \param[in]   UInt32 count - number of sectors to write
//! \param[in]   void *vol - context for the host stack
//!
//! \return int result of the write
//!
int sysMscRpc_ReadSectors(void *buffer, UInt32 startSector, int count, void *vol);


//!
//! Function Name:  sysMscRpc_Insert 
//!
//! Description:  This handles the insert event from the USB stack on the CP and stores the 
//!             filesystem info for subsequent reads and writes.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Insert(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_Remove
//!
//! Description:  This handles the remove event from the USB stack on the CP.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Remove(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_Mount
//!
//! Description:  After an insert a mount message is sent to self so ipc can complete.
//!         This allows FS to run read/writes commands for mount.
//!
//! \param[in]   InterTaskMsg_t *msg - message for the event
//!
//! \return void
//!
void sysMscRpc_Mount(InterTaskMsg_t *msg);

//!
//! Function Name:  sysMscRpc_GetInfo
//!
//! Description:  Retrieves file system info based on name.
//!
//! \param[in]   char *name  - name of filesystem drive
//! \param[out]   sysMscRpcInfo_t **fsInfo - address of pointer to be return with info of drive
//!
//! \return void
//!
Boolean sysMscRpc_GetFsInfo(char *name, sysMscRpcInfo_t **fsInfo);

//!
//! Function Name:  sysMscRpc_IsAttached
//!
//! Description:  Retrieves whether a file system has been attached.
//!
//! \param[in]   char *name  - name of filesystem drive
//!
//! \return Boolean - True - Filesystem is attahed, False - not attached
//!
Boolean sysMscRpc_IsAttached(char *name);

//!
//! Function Name:  sysMscRpc_RegisterMediaCb
//!
//! Description:  Installs callback which notifies upper layer of a volume attach or detach.
//!
//! \param[in]   sysMscRpc_Media_cb  - callback to be installed
//!
//! \return void
//!
void sysMscRpc_RegisterMediaCb(sysMscRpc_Media_cb cb);


#endif //RPC_MSC_MSG_H

