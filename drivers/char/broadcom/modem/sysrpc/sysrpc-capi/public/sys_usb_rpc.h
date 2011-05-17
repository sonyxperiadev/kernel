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

#define MAX_USB_MSG_SIZE 64*1024
#ifdef ENABLE_USB_MEGASIM
#define USE_MEGASIM_MSC
#endif
//Functions called from CP executed on AP
#define USB_IPC_IN_RANGE(x) (((x) >= 100) &&  ((x) <=220))

#ifdef FUSE_COMMS_PROCESSOR
#define PROC_TYPE "CP"
#else
#define PROC_TYPE "AP"
#endif

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

//!The structure used to send messages.
//!
typedef struct tag_usbEemMsg_t
{
    //! The context handle
    void* ctx;
    //! The length of eem packet
    UInt32 len;
    //! The buffer for the eem packet
    void *buf;
}usbEemMsg_t;


//!The structure used to for control
//!
typedef struct tag_usbEemCtrl_t
{
    //! The message being sent
    UInt32 msg;
    //! The value for the message
    UInt32 value;
}usbEemCtrl_t;
 
typedef enum
{
   MSG_USB_NO_PAYLOAD = 0,
   MSG_USB_MSC_MEDIA_INFO,
   MSG_USB_MSC_XFR_IND,
   MSG_USB_RAW_DATA,
   MSG_USB_EEM_CTRL_IND,
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
  usbEemCtrl_t      *eemCtrl;
 }un;
}USBPayload_t;

typedef Boolean (* SysUsbRpc_MsgHandler)(InterTaskMsg_t *msg);
typedef Boolean (* SysUsbRpc_NotifyCb)(UInt32 type);

//Functions called from CP executed on AP
#define USB_IPC_MSC_INIT                    100
#define USB_IPC_MSC_UNINIT                  101
#define USB_IPC_MSC_MEDIA_INSERT            102
#define USB_IPC_MSC_MEDIA_REMOVE            103
#define USB_IPC_MSC_DEVICE_ATTACH           104
#define USB_IPC_MSC_DEVICE_DETACH           105
#define USB_IPC_MSC_READ                    106
#define USB_IPC_MSC_WRITE                   107
#define USB_IPC_MSC_MEDIA_MOUNT             108
#define USB_IPC_EEM_SEND                    109
#define USB_IPC_EEM_CTRL                    110
#define USB_IPC_EEM_MSG                     111
#define USB_IPC_NOTIFY                      112

//!
//! Function Name:  SysUsbRpc_SendMscInfo 
//!
//! Description:  This message send Mass Storage information
//!     from the CP to the AP.  It is used for MEDIA_INSERT and 
//!     MEDIA_REMOVE
//!
//! \param[in]   type Used to set event type
//! \param[in]   *drv_ctx Unique identifier provided by the USB stack 
//! \param[in]   **app_ctx Unique identifier provided by 
//!             adapter layer returned by the stack durning a callback
//! \param[in]   num_heads number heads on the disk of the device
//! \param[in]   num_sects number sectors on the disk of the device
//! \param[in]   sector_size size of a sector in bytes of on the disk 
//!
//! \return void
//!
void SysUsbRpc_SendMscInfo(UInt32 type, void* drv_ctx, void**app_ctx, UInt32 num_heads, UInt32 num_sects, UInt32 sector_size);

//!
//! Function Name:  IPC_SendMscXfr 
//!
//! Description:  This message send Mass Storage sector buffers
//!     between CP and AP.  It is used for sector read and write
//!
//! \param[in]   type Used to set event type
//! \param[in]   *buffer buffer provided by the application layer for sector data transfer
//! \param[in]   first_sector starting sector for the data transfer
//! \param[in]   count number of sectors to be transfered
//! \param[in]   *drv_ctx Unique identifier provided by the USB stack 
//! \param[in]   sector_size size of a sector in bytes of on the disk
//!
//! \return void
//!
int IPC_SendMscXfr(int type, void *buffer, UInt32 first_sector, int count, void *drv_ctx, UInt32 sector_size);

//!
//! Function Name:  IPC_SendEemCtrl 
//!
//! Description:  This message sends EEM ctontrol messages from CP to AP
//!
//! \param[in]   type Used to set event type
//! \param[in]   msg type of message sent
//!
//! \return void
//!
void IPC_SendEemCtrl(int type, UInt32 msg, UInt32 value);

//!
//! Function Name:  IPC_SendEemEcho 
//!
//! Description:  This message sends EEM echo messages from AP to CP
//!
//! \param[in]   buffer data to be echoed
//! \param[in]   buflen size of buffer
//!
//! \return void
//!
void IPC_SendEemEcho(void *buffer, UInt32 buflen);

//!
//! Function Name:  IPC_SendMscInfo 
//!
//! Description:  This message send Mass Storage sector buffers
//!     between CP and AP.  It is used for sector read and write
//!
//! \param[in]   type Used to set event type
//! \param[in]   *drv_ctx Unique identifier provided by the USB stack 
//! \param[in]   **app_ctx Unique identifier to be filled by the stack 
//! \param[in]   num_heads number of heads in the drive
//! \param[in]   num_sects number of sectors in the drive
//! \param[in]   sector_size size in bytes of a single sector
//!
//! \return void
//!
void IPC_SendMscInfo(UInt32 type, void* drv_ctx, void**app_ctx, UInt32 num_heads, UInt32 num_sects, UInt32 sector_size);

//!
//! Function Name:  SysUsbRpc_Init 
//!
//! Description:  This function initializes task for handing IPC data 
//!
//! \return void
//!
void SysUsbRpc_Init(void);

//!
//! Function Name:  SysUsbRpc_InstallHandler 
//!
//! Description:  This method allows a user to process incoming messages
//!     sent by the USBIPC that are not standard messages.
//!
//!
//! \param[in]   cb Function pointer to process a intertask msg
//!
//! \return void
//!
//! Currently this method is used by the test code.
//!
void SysUsbRpc_InstallHandler(SysUsbRpc_MsgHandler cb);


//!
//! Function Name:  SysUsbRpc_SendReq 
//!
//! Description:  This method handles and responds to an intertask message 
//!     sent by the other core.
//!
//! \param[in]   clientID Client Id of the initiator
//! \param[in]   tid Task Id of the initiator
//! \param[in]   *InMsg Message being sent
//!
//! \return Result_t 
//!
//! This method sends the InterTask message to the USBIPC task to handle.
//!
Result_t SysUsbRpc_SendReq( UInt8 clientID, UInt32 tid, InterTaskMsg_t *InMsg);

//!
//! Function Name:  SysUsbRpc_SendInterTask_RSP 
//!
//! Description:  This sends a message and blocks for a 
//!     response.
//!
//! \param[in]   tid Task Id of the initator
//! \param[in]   cid Client Id of the initiator
//! \param[in]   *InMsg Message being sent
//!
//! \return void
//!
void SysUsbRpc_SendInterTask_RSP(UInt32 tid, UInt8 cid, InterTaskMsg_t *InMsg);

//!
//! Function Name:  SysUsbRpc_PostMsg 
//!
//! Description:  This sends a message 
//!
//! \param[in]   *msg Message being sent
//!
//! \return void
//!
void SysUsbRpc_PostMsg(const InterTaskMsg_t *msg);

//!
//! Function Name:  SysUsbRpc_RegisterNotify 
//!
//! Description:  This function registers a callback for any notifications from CP to AP 
//!
//! \param[in]   *msg Message being sent
//!
//! \return void
//!
void SysUsbRpc_RegisterNotify(SysUsbRpc_NotifyCb cb);

//!
//! Function Name:  sys_usb_init
//!
//! Description:  Internal Init function.  Both AP and CP have their own versions.
//!
//! \return void
//!
void sys_usb_init(void);



//END NEW CODE
#endif //



