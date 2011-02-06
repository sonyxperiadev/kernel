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
*   @file   sys_usb_rpc.h
*
*   @brief  This file defines the usb message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/

#ifndef RPC_USB_MSG_H
#define RPC_USB_MSG_H

/**
* @addtogroup  USBIpcApi
* @{
*/

//!
//! Function Name:  IPC_SendInterTask_RSP 
//!
//! Description:  This sends a message and blocks for a 
//!     response.
//!
//! \param[in]   UInt32 tid
//! \param[in]   UInt8 cid
//! \param[in]   InterTaskMsg_t *msg
//!
//! \return void
//!
void IPC_SendInterTask_RSP(UInt32 tid, UInt8 cid, InterTaskMsg_t *msg);

//!The structure of Mass Storage Media info retrieved from the device
//!
//! \sa IPC_SendMscInfo
typedef struct tag_usbMscMediaInfo_t
{
    //! Unique identifier provided by the USB stack
    UInt32 *drv_ctx; 
    //! Unique identifier provided by adapter layer returned by the stack durning a callback
    UInt32 *app_ctx;
    //! The number heads on the disk of the device
    UInt32 heads;
    //! The number sectors on the disk of the device
    UInt32 sector_count;
    //! The size of a sector in bytes of on the disk
    UInt32 sector_size;
}usbMscMediaInfo_t;

//!The structure used to read and write sectors to the disk on the device.
//!
//! \sa IPC_SendMscXfr
typedef struct tag_usbMscXfer_t
{
    //! The buffer provided by the application layer for sector data transfer
    void* buffer;
    //! The starting sector for the data transfer
    UInt32 first_sector;
    //! The number of sectors to be transfered
    Int32 count;
    //! The size of a sector in bytes of on the disk
    UInt32 sector_size;
    //! Unique identifier provided by the USB stack
    void *drv_ctx;
    //! The return value of the transfer
    int result;
}usbMscXfer_t;

 
typedef enum
{
   MSG_USB_NO_PAYLOAD,
   MSG_USB_MSC_MEDIA_INFO,
   MSG_USB_MSC_XFR_IND,
   MSG_USB_RAW_DATA,
}USBPayloadType_t;
 
typedef struct
{
    USBPayloadType_t  payloadType;
 UInt32     payloadLen;
 UInt32     param1;
 UInt32     param2;
    union
 {
  usbMscXfer_t   *xfr;
  usbMscMediaInfo_t *mediaInfo;
  UInt8             *buffer;
 }un;
}USBPayload_t;

//END NEW CODE
#endif //



