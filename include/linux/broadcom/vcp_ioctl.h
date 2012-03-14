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
*  @file    vcp_ioctl.h
*
*  @brief   Defines the IOCTL driver interface for the video Conference
*           Pipeline (VCP) driver
*
*****************************************************************************/
#if !defined( VCP_IOCTL_H )
#define VCP_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/broadcom/vcp_device.h>
#include <linux/broadcom/vcp_msg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* Magic used to create unique IOCTL number */
#define VCPIOCTL_MAGIC_TYPE               'v'

/* IOCTL commands */
	enum vcpioctl_cmd_e {
		VCPIOCTL_CMD_REGISTER_DEC = 50,
		VCPIOCTL_CMD_REGISTER_ENC,
		VCPIOCTL_CMD_DEREGISTER,
		VCPIOCTL_CMD_GET_DECFRAME,
		VCPIOCTL_CMD_GET_CTRLMSG,
		VCPIOCTL_CMD_PUT_RESPONSE,
		VCPIOCTL_CMD_DISCARD_DECFRAME,
		VCPIOCTL_CMD_DEC_EVENTCB,
		VCPIOCTL_CMD_ENC_EVENTCB,
		VCPIOCTL_CMD_ENC_FRAME,
		VCPIOCTL_CMD_LAST	/* Do no delete */
	};

	struct vcpioctl_register_dec_data {
		char devname[32];	/* Device name */
		VCP_DEC_DEVICE_OPS ops;	/* For informational purposes only */
		void *devdata;	/* Privata device data */
	};

	struct vcpioctl_register_enc_data {
		char devname[32];	/* Device name */
		VCP_ENC_DEVICE_OPS ops;	/* For informational purposes only */
		void *devdata;	/* Privata device data */
	};

	struct vcpioctl_get_decframe_data {
		int max_pktsize;	/* (i) Maximum packet size in bytes */
		VCP_FRAMEBUF *userdatap;	/* (o) User ptr to store data */
		int *size_needed;	/* (o) If insufficient memory, indicate amount of mem needed */
	};

	struct vcpioctl_get_ctrlmsg_data {
		VCPMSG_ALL *userdatap;	/* (o) User ptr to store data */
	};

	struct vcpioctl_put_response_data {
		VCPMSG_RESP *userdatap;	/* Ptr to response data */
	};

	struct vcpioctl_dec_eventcb {
		VCP_PIPE_HDL pipehdl;	/* Pipeline handle */
		VCP_EVENT_CODE eventCode;	/* Event code */
		void *userinfo;	/* Ptr to user information related to event */
	};

	struct vcpioctl_enc_eventcb {
		VCP_PIPE_HDL pipehdl;	/* Pipeline handle */
		VCP_EVENT_CODE eventCode;	/* Event code */
		void *userinfo;	/* Ptr to user information related to event */
	};

	struct vcpioctl_encframe_data {
		VCP_PIPE_HDL pipehdl;	/* Pipeline handle */
		int bytesize;	/* (i) Frame size in bytes */
		VCP_FRAMEBUF *userdatap;	/* (o) User ptr to store data */
	};

/* IOCTL numbers */
#define VCPIOCTL_REGISTER_DEC    _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_REGISTER_DEC, struct vcpioctl_register_dec_data )
#define VCPIOCTL_REGISTER_ENC    _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_REGISTER_ENC, struct vcpioctl_register_enc_data )
#define VCPIOCTL_DEREGISTER      _IO( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_DEREGISTER )
#define VCPIOCTL_GET_DECFRAME    _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_GET_DECFRAME, struct vcpioctl_get_decframe_data )
#define VCPIOCTL_GET_CTRLMSG     _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_GET_CTRLMSG, struct vcpioctl_get_ctrlmsg_data )
#define VCPIOCTL_PUT_RESPONSE    _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_PUT_RESPONSE, struct vcpioctl_put_response_data )
#define VCPIOCTL_DISCARD_DECFRAME _IO( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_DISCARD_DECFRAME )
#define VCPIOCTL_DEC_EVENT       _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_DEC_EVENTCB, struct vcpioctl_dec_eventcb )
#define VCPIOCTL_ENC_EVENT       _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_ENC_EVENTCB, struct vcpioctl_enc_eventcb )
#define VCPIOCTL_ENC_FRAME       _IOR( VCPIOCTL_MAGIC_TYPE, VCPIOCTL_CMD_ENC_FRAME, struct vcpioctl_encframe_data )

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif				/* VCP_IOCTL_H */
