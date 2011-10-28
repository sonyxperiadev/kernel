//**************************************************************************************
// Copyright 1999-2008 Broadcom Corporation
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
*   @file   sys_een_rpc.h
*
*   @brief  This file defines the usb message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/

#ifndef RPC_EEM_MSG_H
#define RPC_EEM_MSG_H

#ifndef LOGID_SYSEEMRPC
#define LOGID_SYSEEMRPC LOGID_USB
#endif
#define CACHE_ALIGN 32

#define EEM_HEADER 2
#define EEM_FOOTER 4

/**
* @addtogroup  USBEemApi
* @{
*/
typedef Boolean (*eem_recv_cb_t)(void *buffer, UInt32 buflen, void* bufHandle);
typedef struct {
    eem_recv_cb_t recv;
} eem_cb_t;

//!
//! Function Name:  EEM_Send 
//!
//! Description:  This sends an ethernet packet to be processed.
//!  This api is non blocking
//!
//! \param[in]   void *buffer
//! \param[in]   UInt32 buflen
//!
//! \return void
//!
void EEM_Send(void *buffer, UInt32 buflen);

void EEM_SendEx(void *buffer, UInt32 buflen, UInt8 hdr, UInt8 footer, UInt8 cacheAlign);

//!
//! Function Name:  EEM_Echo
//!
//! Description:  This sends an ethernet packet to be echoed.
//!  This api is non blocking
//!
//! \param[in]   void *buffer
//! \param[in]   UInt32 buflen
//!
//! \return void
//!
void EEM_Echo(void *buffer, UInt32 buflen);

//!
//! Function Name:  EEM_RegisterRecvCb
//!
//! Description:  This api registers a callback that 
//!  gets called for each incoming ethernet packet.
//!  The callback is expected to copy data to a 
//!  private buffer.
//!
//! \param[in]   eem_cb_t *cbs - callback to be installed
//!
//! \return void
//!
void EEM_RegisterRecvCb(eem_cb_t *cbs);

//!
//! Function Name:  EEM_Init
//!
//! Description:  Sets up the EEM interface
//!
//!
//! \return void
//!
void EEM_Init(void);

#endif //



