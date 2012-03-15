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

#ifndef _AF_SERVER_H_
#define _AF_SERVER_H_

/* ---- Include Files ---------------------------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

typedef enum {
	AF_SERVER_POINT_CODEC_SKPR = 0,
	AF_SERVER_POINT_CODEC_MIC,

	AF_SERVER_POINT_AUDIO_ROUT_FB,
	AF_SERVER_POINT_AUDIO_SIN_FB,
	AF_SERVER_POINT_AUDIO_RIN_FB,
	AF_SERVER_POINT_AUDIO_SOUT_FB,

	AF_SERVER_POINT_AUDIO_IOD_SIN,
	AF_SERVER_POINT_AUDIO_IOD_SOUT,
	AF_SERVER_POINT_AUDIO_IOD_RIN,
	AF_SERVER_POINT_AUDIO_IOD_ROUT,
	AF_SERVER_POINT_AUDIO_IOD_API,
	AF_SERVER_POINT_AUDIO_IOD_SIN2,
	AF_SERVER_POINT_AUDIO_IOD_SIN3,
	AF_SERVER_POINT_AUDIO_IOD_RIN2,
	AF_SERVER_POINT_AUDIO_IOD_ROUT2,
	AF_SERVER_POINT_AUDIO_IOD_GLIN,

	AF_SERVER_POINT_AUDIO_IOD_CAP_LOG,
	AF_SERVER_POINT_AUDIO_IOD_CAP_SOUTFG,
	AF_SERVER_POINT_AUDIO_IOD_CAP_SOUTBG,
	AF_SERVER_POINT_AUDIO_IOD_CAP_EVT,

	AF_SERVER_POINT_NET_RX,
	AF_SERVER_POINT_NET_TX,

	AF_SERVER_POINT_NUM_MAX
} AF_SERVER_POINT_ID;

typedef enum {
	AF_SERVER_IO_CAPTURE = 0,
	AF_SERVER_IO_INJECT,

} AF_SERVER_IO_MODE;

typedef int AF_SERVER_POINT_HDL;

#define AF_INVALID_HANDLE     0

#define AF_POINT_MAX_NUM_CHANNELS      4

/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

int af_server_add_point(AF_SERVER_POINT_ID point_id, AF_SERVER_IO_MODE io_mode,
			AF_SERVER_POINT_HDL *point_hdl);
int af_server_remove_point(AF_SERVER_POINT_HDL point_hdl);
int af_server_enable(int enable_flag);

int af_server_frame_sync(void);
int af_server_capture(char *buf, int len, void *data);
int af_server_inject(char *buf, int len, void *data);

#endif /* _AF_SERVER_H_ */
