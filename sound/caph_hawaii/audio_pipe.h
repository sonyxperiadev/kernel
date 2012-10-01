/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/

/**
*
* @file   audio_pipe.h
* @brief  header for audio pipe
*
******************************************************************************/

#ifndef __AUDIO_PIPE_H__
#define __AUDIO_PIPE_H__

#define AUDIOP_MAX_PIPE_SUPPORT 8

/* audio pipe msg ids */
enum _PIPE_MSG_ID_t_ {
	AUDIOP_PIPE_MSG_NONE,
	AUDIOP_PIPE_CREATED,
	AUDIOP_PIPE_DESTROYED,
	AUDIOP_PIPE_STARTED,
	AUDIOP_PIPE_STOPPED,
	AUDIOP_PIPE_SETBUFDONE,
	AUDIOP_PIPE_BUFFER_DONE,
	/* notify AAP that AUDP got device change REQ */
	AUDIOP_PIPE_DEVICE_CHANGE,
	/* need ACK from AAP before take action */
	/* notify AAP that device change REQ was done */
	AUDIOP_PIPE_DEVICE_CHANGED,
	AUDIOP_PIPE_SYNC_EVENT,
	AUDIOP_PIPE_ERROR
};

#define PIPE_MSG_ID_t enum _PIPE_MSG_ID_t_

/* audio pipe device ids */
enum _PIPE_DEVICE_ID_t_ {
	AUDIOP_DEVICE_NONE = 0x0,
	AUDIOP_MIC_MAIN = 0x1,
	AUDIOP_MIC_AUX = 0x2,
	AUDIOP_MIC_BTM = 0x4,
	AUDIOP_MIC_USB = 0x8,
	AUDIOP_MIC_DIGI1 = 0x10,
	AUDIOP_MIC_DIGI2 = 0x20,
	AUDIOP_MIC_DIGI3 = 0x40,
	AUDIOP_MIC_DIGI4 = 0x80,
	AUDIOP_MIC_DMICX = (AUDIOP_MIC_DIGI1
			    | AUDIOP_MIC_DIGI2
			    | AUDIOP_MIC_DIGI3 | AUDIOP_MIC_DIGI4),
	AUDIOP_SPK_HANDSET = 0x100,
	AUDIOP_SPK_HEADSET = 0x200,
	AUDIOP_SPK_LOUDSPK = 0x400,
	AUDIOP_SPK_LOUDSPK1 = AUDIOP_SPK_LOUDSPK,
	AUDIOP_SPK_LOUDSPK2 = AUDIOP_SPK_LOUDSPK,
	AUDIOP_SPK_VIBRA = 0x800,
	AUDIOP_SPK_I2S = 0x1000,
	AUDIOP_SPK_BTM = 0x2000,
	AUDIOP_SPK_USB = 0x4000,
};

#define PIPE_DEVICE_ID_t enum  _PIPE_DEVICE_ID_t_

/* audio pipe states */
enum _PIPE_STATE_t_ {
	AUDIOP_PIPE_STATE_NONE,
	AUDIOP_PIPE_STATE_CREATED,
	AUDIOP_PIPE_STATE_PREPARED,
	AUDIOP_PIPE_STATE_STARTED,
	AUDIOP_PIPE_STATE_STOPPED,
	AUDIOP_PIPE_STATE_DESTROYED,
};

#define PIPE_STATE_t enum _PIPE_STATE_t_

/* audio pipe directions */
enum _PIPE_DIRECTION_t_ {
	AUDIOP_PIPE_DIR_NONE = 0x0,
	AUDIOP_PIPE_DIR_UL = 0x1,	/* ingress */
	AUDIOP_PIPE_DIR_DL = 0x2,	/* egress */
	/* enable both in one cmd */
	AUDIOP_PIPE_DIR_BOTH =
	(AUDIOP_PIPE_DIR_UL | AUDIOP_PIPE_DIR_DL),
};

#define  PIPE_DIRECTION_t enum _PIPE_DIRECTION_t_

/* call back function */
typedef int (*CB_PIPE) (PIPE_MSG_ID_t msgID, int pipeID, uint32_t param1,
			uint32_t param2, uint32_t param3, uint32_t param4,
			uint32_t param5);

/* AAP will register the call back functions after the bootup initialization */
int audp_registerCB(CB_PIPE);
int audp_deregister(void);

/* AAP will set the buffer: bufferSize is the size for each buffer */
int audp_set_buffer(int pipeID, dma_addr_t startAddr, uint32_t bufferSize,
		    uint32_t bufferNum, void *privData);

/* internal control apis*/
int audp_create_pipe(PIPE_DEVICE_ID_t id);
void audp_start_pipe(int pipeID);
void audp_stop_pipe(int pipeID);
void audp_destroy_pipe(int pipeID);
void audp_device_change_notify(PIPE_DEVICE_ID_t new_dev,
			       PIPE_DEVICE_ID_t current_dev);
void audp_device_change_finished(PIPE_DEVICE_ID_t current_dev,
				 PIPE_DEVICE_ID_t old_dev);
int audp_get_pipeID(PIPE_DEVICE_ID_t dev);
int audp_ctrl_handler(long action, long param, PIPE_DIRECTION_t direction);

#endif /*__AUDIO_PIPE_H__*/
