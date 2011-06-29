/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/


#ifndef VCEB_COMMON_H_
#define VCEB_COMMON_H_

#include "vceb_types.h"
#include "vceb_host_interface.h"

/******************************************************************************
 Global types
 *****************************************************************************/

#define VCEB_MAX_MSG_SIZE_IN_BYTES        128 //bytes

//message ID's
typedef enum
{
   VCEB_MSG_MIN = 0,

   //Console message's
   VCEB_MSG_CONSOLE_ENABLE,
   VCEB_MSG_CONSOLE_PUTCHAR,
   VCEB_MSG_CONSOLE_CLEAR,

   //Framebuffer overlay message's
   VCEB_MSG_FB_OVERLAY_ENABLE,
   VCEB_MSG_FB_OVERLAY_SET,
   VCEB_MSG_FB_OVERLAY_CLEAR,

   //Diag messages
   VCEB_MSG_DIAG_RUN,

   //go into download mode to receive the next stage loader
   VCEB_MSG_GOTO_DOWNLOAD,

   //set the LCD backlight brightness
   VCEB_MSG_SET_LCD_BRIGHTNESS,

   VCEB_MSG_MAX

} VCEB_MSG_T;

//enum of the RGB buffer formats supported
typedef enum
{
   VCEB_RBG_FORMAT_MIN = 0,

   //Note - all RGB formats are descibed in little endian
   //where n[0] is the lowest physical address
   VCEB_RBG_FORMAT_RGB565, //R[0], G[1+0], B[1]
   VCEB_RBG_FORMAT_RGB888, //R[0], G[1], B[2]
   VCEB_RBG_FORMAT_RGBA8888, //R[0], G[1], B[2], A[3]
   VCEB_RBG_FORMAT_BGRA8888, //R[0], G[1], B[2], A[3]

   VCEB_RBG_FORMAT_MAX
   
} VCEB_RBG_FORMAT_T;

//alignment options for putting the overlay on the screen
typedef enum
{
   VCEB_ALIGN_MIN = 0,

   VCEB_ALIGN_CENTRE,
   VCEB_ALIGN_STRETCH_FULLSCREEN,
   VCEB_ALIGN_BOTTOM_RIGHT,

   VCEB_ALIGN_MAX
   
} VCEB_ALIGN_T;


//message ID's
typedef enum
{
   //default - no flags set
   VCEB_FLAGS_NONE,

   //flag to say if the message is a reply or a request
   VCEB_FLAGS_REPLY = 0x1,

   //if the message header contains a valid crc value or not
   VCEB_FLAGS_VALID_CRC = 0x2,

   //if the reset of the message payload has gone via the bulk channel
   VCEB_FLAGS_USES_BULK_CHANNEL = 0x4

} VCEB_FLAGS_T;


//basic type definition for a message sent over the interface
//This message should be 16 bytes to align with the smallest message size on the interface
//Note that this message struct is not memory mapped - instead pack + unpack fuctions are used 
// to translate it into tightly packed memory
typedef struct
{
   //the message ID being sent down
   uint8_t message_id;
   uint8_t flags;

   //message CRC
   uint16_t crc;

   //the entire message length, including this message header
   uint32_t total_message_length_in_bytes;

   //the message payload - optional per message
   uint32_t param_0;
   uint32_t param_1;

} VCEB_MSG_HEADER_T;

#define VCEB_MSG_HEADER_SIZE_IN_BYTES  16

/******************************************************************************
Global functions.
******************************************************************************/

//Routine to send a message
extern int32_t vceb_message_send(VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                 const VCEB_MSG_T message,
                                 const VCEB_FLAGS_T flags,
                                 const uint32_t param_0,
                                 const uint32_t param_1,
                                 const uint32_t size_of_optional_data_in_bytes,
                                 const void * const data,
                                 const uint32_t terminate_transfer );

//Routine to receive a message
extern int32_t vceb_message_receive(VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                    VCEB_MSG_T *message,
                                    VCEB_FLAGS_T *flags,
                                    uint32_t *param_0,
                                    uint32_t *param_1,
                                    uint32_t *size_of_optional_data_in_bytes,
                                    void ** data );

//Routine to receive and check a message for assumed payload
extern int32_t vceb_message_check_receive(   VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                             const VCEB_MSG_T message,
                                             const VCEB_FLAGS_T flags,
                                             const uint32_t param_0,
                                             const uint32_t param_1,
                                             uint32_t *size_of_optional_data_in_bytes,
                                             void * data );

#endif // VCEB_COMMON_H_
