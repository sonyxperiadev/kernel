/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    vcppriv.h
*
*  @brief   Private definitions for VCP
*
****************************************************************************/
#if !defined( VCPPRIV_H )
#define VCPPRIV_H

/* ---- Include Files ---------------------------------------------------- */
#include <linux/broadcom/vcp_device.h>
#include <linux/broadcom/vcp_msg.h>

/* ---- Constants and Types ---------------------------------------------- */
/**
*  Verbose prints are enabled via /proc/sys/vcp/dbgprint.
*
*  Usage:
*
*  Set to 1 to get basic informational traces. Set to 2 to get
*  all basic traces plus additional verbose prints.
*/
#define VCP_TRACE( fmt, args... )  do { if ( gDbgPrintLevel ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)
#define VCP_DEBUG( fmt, args... )  do { if ( gDbgPrintLevel >= 2 ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)

/* ---- Variable Externs ------------------------------------------------- */
extern int gDbgPrintLevel;

/* ---- Function Prototypes ---------------------------------------------- */

/***************************************************************************/
/**
*  User proxy layer constructor
*
*  @return  0  on success, -ve on error
*/
int vcpuser_constructor( void );

/***************************************************************************/
/**
*  User proxy layer destructor
*
*  @return  0  on success, -ve on error
*/
int vcpuser_destructor( void );

/***************************************************************************/
/**
*  Register a user proxy VCP decoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_register_dec_device(
   const char *devname,             /**< (i) Device name */
   VCP_DEC_DEVICE_OPS *ops,         /**< (i) Device callbacks */
   void *privdata,                  /**< (i) Private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Return handle to user proxy device */
);

/***************************************************************************/
/**
*  Register a user proxy VCP encoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_register_enc_device(
   const char *devname,             /**< (i) Device name */
   VCP_ENC_DEVICE_OPS *ops,         /**< (i) Device callbacks */
   void *privdata,                  /**< (i) Private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Return handle to user proxy device */
);

/***************************************************************************/
/**
*  De-register a user-level video encoder or decoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_deregister_device(
   VCP_DEV_HDL hdl                  /**< (i) handle */
);

/***************************************************************************/
/**
*  Dequeue a decoder frame. Blocks until a frame is available.
*  If insufficient memory is provided, frame remains on the queue until
*  sufficient memory is provided. To forcibly discard a frame, call
*  vcpuser_discard_decframe().
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread. The output frame buffer pointer is a user space
*  pointer where the frame data is returned.
*
*  There is a single queue for decoder frames per device.
*  Each decode frame is tagged with device and pipeline information
*  for routing.
*/
int vcpuser_get_decframe(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   int max_pktsize,                 /**< (i) Max size to return */
   VCP_FRAMEBUF __user *framebufp,  /**< (o) User ptr to return frame buffer */
   int __user *size_needed          /**< (o) If insufficient memory, ptr to return amount needed */
);

/***************************************************************************/
/**
*  Discard decoder frame.
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread.
*
*  There is a single queue for decoder frames per device.
*/
int vcpuser_discard_decframe(
   VCP_DEV_HDL hdl                  /**< (i) device handle */
);

/***************************************************************************/
/**
*  Dequeue a user message. Blocks until a message is available.
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread. The output frame buffer pointer is a user space
*  pointer where the frame data is returned.
*
*  There is a single queue for control messages frames per device. Message
*  contain device and pipeline information for routine.
*/
int vcpuser_get_ctrlmsg(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   VCPMSG_ALL __user *userdatap     /**< (o) User ptr to return message */
);

/***************************************************************************/
/**
*  Queue user response.
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_put_response(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   VCPMSG_RESP __user *userdatap    /**< (i) User ptr to return message */
);

/***************************************************************************/
/**
*  Handle decoder events from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_dec_eventcb(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   VCP_EVENT_CODE eventCode,        /**< (i) event code */
   void __user *userinfo,           /**< (i) User ptr to event info */
   void *privdata                   /**< (i) Private device data */
);

/***************************************************************************/
/**
*  Handle encoder events from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_enc_eventcb(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   VCP_EVENT_CODE eventCode,        /**< (i) event code */
   void __user *userinfo,           /**< (i) User ptr to event info */
   void *privdata                   /**< (i) Private device data */
);


/***************************************************************************/
/**
*  Handle encoder frames from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_enc_frame(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   int bytesize,                    /**< (i) frame size in bytes */
   void __user *userdatap,          /**< (i) User ptr to frame */
   void *devdata                    /**< (i) Private device data */
);
#endif /* VCPPRIV_H */

