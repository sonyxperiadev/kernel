/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm_audio.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* bcm_audio.h
* PURPOSE:
* This file should be used by user of Audio driver.
* NOTES:
*
* ****************************************************************************/

#define BCMAUDIO_MAGIC	'A'

// IOCTL for audio driver
enum bcmaud_ioctl
{
	LOG_CONFIG_CHANNEL=105,
	LOG_START_CHANNEL,
	LOG_STOP
};

// Log message channel configuration
typedef struct log_msg_info_t
{   
	UInt32 log_link;                ///< log message link path ( the stream number 0, 1, 2, 3)
	UInt32 log_capture_point;       ///< log message capture point value for stream
	UInt16 log_consumer;            ///< log message consumer 0 : MTT  1 : file system
} AUDDRV_CFG_LOG_INFO;


#define BCM_LOG_IOCTL_CONFIG_CHANNEL               _IO(BCMAUDIO_MAGIC, LOG_CONFIG_CHANNEL)
#define BCM_LOG_IOCTL_START_CHANNEL                _IOW(BCMAUDIO_MAGIC,LOG_START_CHANNEL,AUDDRV_CFG_LOG_INFO)
#define BCM_LOG_IOCTL_STOP                         _IOW(BCMAUDIO_MAGIC,LOG_STOP,AUDDRV_CFG_LOG_INFO)

