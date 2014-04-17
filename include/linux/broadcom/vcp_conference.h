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
*  @file    vcp_conference.h
*
*  @brief   Define the API on conferencing between the VCP driver and the 
*           VCP devices.
*
*****************************************************************************/
#if !defined( VCP_CONFERENCE_H )
#define VCP_CONFERENCE_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */
	typedef struct vcp_encode_conference {
		void *dec_hdl;	/* VCP_HDL for the decoder channel to be added to the conference */
	} VCP_ENCODE_CONFERENCE;

	typedef struct vcp_encode_get_mixer {
		void *dec_hdl;	/* VCP_HDL for the decoder channel in query */
		void *video_mixer;	/* pointer to the video_mixer component */
		int port;	/* input port number to the video_mixer component that this
				 * decoder channel should be connected to */
	} VCP_ENCODE_GET_MIXER;

	typedef struct vcp_decode_conference {
		int enable;	/* set to 1 to enable conference */
		void *video_mixer;	/* pointer to the video_mixer component */
		int port;	/* int port number to the video_mixer component this decoder should use */
	} VCP_DECODE_CONFERENCE;

	typedef enum {
		VCP_ENCODE_PSEUDO_CAM = 0,	/* encoder only borrow the camera image from another encoder stream */
		VCP_ENCODE_PSEUDO_ENC,	/* encoder is a complete pseudo of another encoder */
		VCP_ENCODE_PSEUDO_MAX,

	} VCP_ENCODE_PSEUDO_MODE;

	typedef struct vcp_encode_pseudo {
		void *main_hdl;	/* VCP_HDL for the encode channel where the settings should be copied from */
		VCP_ENCODE_PSEUDO_MODE mode;	/* the pseudo mode this encoder stream should be in */
	} VCP_ENCODE_PSEUDO;

	typedef struct vcp_encode_pseudo_cbks {
		void *cbks;
		void *priv_data;

	} VCP_ENCODE_PSEUDO_CBKS;

#define VCP_THREEWAY_DISPLAY_MAX_LAYER 2	/* maximum layer number supported */

	typedef struct vcp_conf_src_config {

		/* location of the source on the encoded image */
		unsigned int dest_x_offset;
		unsigned int dest_y_offset;
		unsigned int layer;	/* layer number of the source on the encoded image
					 * Note, only layer 0 to 2 will be supported */

	} VCP_CONF_SRC_CONFIG;

	typedef struct vcp_conf_active_talker {
		int set;	/* set to 1 to mark this pipeliner as the active talker, 0 to unmark */
		/* note the idea is that setting a new active talker will automatically
		 * unmark the previous settings */

	} VCP_CONF_ACTIVETALKER;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif				/* VCP_CONFERENCE_H */
