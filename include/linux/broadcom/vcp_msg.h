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
*  @file    vcp_msg.h
*
*  @brief   Defines the messaging API between kernel and user VCP components.
*           The VCP driver makes use of this API to communicate with user-
*           domain VCP device entities.
*
*****************************************************************************/
#if !defined( VCP_MSG_H )
#define VCP_MSG_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/broadcom/vcp_device.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* Message IDs */
	enum vcpmsg_msg_id {
		/* Common */
		VCPMSG_CLOSE_ID,	/* VCPMSG_CLOSE */
		VCPMSG_START_ID,	/* VCPMSG_START */
		VCPMSG_STOP_ID,	/* VCPMSG_STOP */
		VCPMSG_SETPARM_ID,	/* VCPMSG_SETPARM */
		VCPMSG_GETPARM_ID,	/* VCPMSG_GETPARM */
		VCPMSG_GETCAP_ID,	/* VCPMSG_GETCAP */

		/* Encoder */
		VCPMSG_OPEN_ENC_ID,	/* VCPMSG_OPEN */
		VCPMSG_GETENCSTATS_ID,	/* VCPMSG_GETENCSTATS */

		/* Decoder */
		VCPMSG_OPEN_DEC_ID,	/* VCPMSG_OPEN */
		VCPMSG_GETDECSTATS_ID,	/* VCPMSG_GETDECSTATS */

		/* Miscellaneous */
		VCPMSG_NULL_ID,	/* NULL message */
	};

/* Base class for all command message from kernel to user */
#define VCPMSG_BASE_CLASS \
   unsigned int msgid;                 /* Message ID */ \
   VCP_PIPE_HDL hdl;                   /* Pipeline handle */ \
   void *devdata		/* Private device data from device registration */

	typedef struct vcpmsg_base {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
	} VCPMSG_BASE;

	typedef VCPMSG_BASE VCPMSG_CLOSE;
	typedef VCPMSG_BASE VCPMSG_START;
	typedef VCPMSG_BASE VCPMSG_STOP;
	typedef VCPMSG_BASE VCPMSG_GETPARM;

	typedef struct vcpmsg_getcap {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
		VCP_PARM_INDEX index;	/* Parameter index */
		VCP_CAPABILITIES data;	/* The capabilities */
	} VCPMSG_GETCAP;

	typedef struct vcpmsg_open {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
		VCP_PIPE_HDL knlpipe;	/* Kernel pipe handle */
	} VCPMSG_OPEN;

	typedef struct vcpmsg_setparm {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
		VCP_PARM_INDEX index;	/* Parameter index */
		union {
			int enc_freeze;
			VCP_ENCODER_PARM enc_config;
			VCP_DISPLAY_PARM enc_disp;
			int enc_privacy_mode;
			VCP_PANZOOM_PARM enc_panzoom;
			VCP_DECODER_PARM dec_config;
			VCP_DISPLAY_PARM dec_disp;
			VCP_DISPLAY_MASK_PARM mask_disp;

		} data;
	} VCPMSG_SETPARM;

	typedef struct vcpmsg_getdecstats {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
		int reset;
	} VCPMSG_GETDECSTATS;

	typedef struct vcpmsg_getencstats {
		VCPMSG_BASE_CLASS;	/* DO NOT CHANGE! Must be first */
		int reset;
	} VCPMSG_GETENCSTATS;

/* Response data */
	typedef struct vcpmsg_resp {
		unsigned int msgid;	/* Message ID */
		int rc;		/* Return code */
		union {
			VCP_PIPE_HDL hdl;	/* Pipeline handle returned by open */
			VCP_DECODER_STATS decstats;	/* Decoder statistics */
			VCP_ENCODER_STATS encstats;	/* Encoder statistics */
			VCP_CAPABILITIES cap;
#if 0
			/* Will need to enable once getparm method is supported */

			int enc_bitrate;
			int enc_freeze;
			VCP_ENCODER_PARM enc_config;
			VCP_DISPLAY_PARM enc_disp;
			VCP_DECODER_PARM dec_config;
			VCP_DISPLAY_PARM dec_disp;
			VCP_DISPLAY_MASK_PARM mask_disp;
#endif
		} data;
	} VCPMSG_RESP;

/* Union of all messages */
	typedef union vcpmsg_all {
		VCPMSG_BASE base;
		VCPMSG_OPEN open;
		VCPMSG_CLOSE close;
		VCPMSG_START start;
		VCPMSG_STOP stop;
		VCPMSG_SETPARM setparm;
		VCPMSG_GETDECSTATS getdecstats;
		VCPMSG_GETENCSTATS getencstats;
		VCPMSG_GETCAP getcap;
	} VCPMSG_ALL;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif				/* VCP_MSG_H */
