/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   audio_device.h
*
*   @brief  This file defines interface for audio device
*
****************************************************************************/

#ifndef _AUDIO_DEVICE_H_
#define _AUDIO_DEVICE_H_

#include "consts.h"
#include "audio_API.h"

extern UInt8 _AUDIO_CTRL_IN_VD_;

typedef enum
{
	AUDIODEV_FILL_BUFFER,		// device only take a buffer when started
	AUDIODEV_FILL_CALLBACK,		// device can use callback to request data
	AUDIODEV_FILL_TOTAL
} AUDIO_FILL_t;

typedef enum
{
	AUDIODEV_NOTIFY_FILL,
	AUDIODEV_NOTIFY_DUMP,
	AUDIODEV_NOTIFY_START,
	AUDIODEV_NOTIFY_STOP,
	AUDIODEV_NOTIFY_PAUSE,
	AUDIODEV_NOTIFY_RESUME,
	AUDIODEV_NOTIFY_TIMER,
	AUDIODEV_NOTIFY_TOTAL
} AUDIODEV_NOTIFY_t;

#define DECLARE_AUDIODEV(dev)															\
	UInt32			dev##_Init(void);													\
	UInt32			dev##_QuerySupport(AUDIO_MODE_t, AUDIO_FORMAT_t, AUDIO_LINK_t);		\
	AUDIO_FILL_t		dev##_Open(MMHANDLE, AUDIO_MODE_t, AUDIO_FORMAT_t, AUDIO_LINK_t);	\
	UInt32			dev##_Load(MMHANDLE, const UInt8*, UInt32);							\
	UInt32			dev##_SetParam(MMHANDLE, AUDIO_SET_PARAM_t, const UInt32);			\
	UInt32			dev##_GetParam(MMHANDLE, AUDIO_GET_PARAM_t, UInt32*);				\
	UInt32			dev##_Seek(MMHANDLE, UInt32);										\
	UInt32			dev##_GetPosition(MMHANDLE);										\
	UInt32			dev##_Start(MMHANDLE);												\
	UInt32			dev##_Stop(MMHANDLE);												\
	UInt32			dev##_Pause(MMHANDLE);												\
	UInt32			dev##_Resume(MMHANDLE);												\
	UInt32			dev##_Convert(MMHANDLE, UInt8*, UInt32);							\
	UInt32			dev##_SetVolume(MMHANDLE, const UInt8, const AUDIO_CHANNEL_t);		\
	UInt32			dev##_Close(MMHANDLE);												\
	UInt32			dev##_Shutdown(void);												\
	UInt32			dev##_GetDuration(MMHANDLE)
	
#define REGISTER_AUDIODEV(dev)	\
	{							\
		dev##_Init,				\
		dev##_QuerySupport,		\
		dev##_Open,				\
		dev##_Load,				\
		dev##_SetParam,			\
		dev##_GetParam,			\
		dev##_Seek,				\
		dev##_GetPosition,		\
		dev##_Start,			\
		dev##_Stop,				\
		dev##_Pause,			\
		dev##_Resume,			\
		dev##_Convert,			\
		dev##_SetVolume,		\
		dev##_Close,			\
		dev##_Shutdown,			\
		dev##_GetDuration		\
		}

typedef struct
{
	UInt32			(*Init)			(void);
	UInt32			(*QuerySupport)	(AUDIO_MODE_t, AUDIO_FORMAT_t, AUDIO_LINK_t);
	AUDIO_FILL_t	(*Open)			(MMHANDLE, AUDIO_MODE_t, AUDIO_FORMAT_t, AUDIO_LINK_t);
	UInt32			(*Load)			(MMHANDLE, const UInt8*, UInt32);
	UInt32			(*SetParam)		(MMHANDLE, AUDIO_SET_PARAM_t, const UInt32);
	UInt32			(*GetParam)		(MMHANDLE, AUDIO_GET_PARAM_t, UInt32*);
	UInt32			(*Seek)			(MMHANDLE, UInt32);
	UInt32			(*GetPosition)		(MMHANDLE);
	UInt32			(*Start)		(MMHANDLE);
	UInt32			(*Stop)			(MMHANDLE);
	UInt32			(*Pause)		(MMHANDLE);
	UInt32			(*Resume)		(MMHANDLE);
	UInt32			(*Convert)		(MMHANDLE, UInt8*, UInt32);
	UInt32			(*SetVolume)	(MMHANDLE, const UInt8, const AUDIO_CHANNEL_t);
	UInt32			(*Close)		(MMHANDLE);
	UInt32			(*Shutdown)		(void);
	UInt32			(*GetDuration)	(MMHANDLE);
} AUDIODEV_DB;



void AudioDev_Notify(
	AUDIODEV_NOTIFY_t,				// notify type
	MMHANDLE,						// session handle
	UInt32 par1,
	UInt32 par2
	);


// use this API to set/clear the flag 
#define Audio_Dev_SetUseHAL(trueOrFalse) { _AUDIO_CTRL_IN_VD_ = trueOrFalse;}
#define Audio_Dev_CheckUseHAL()	(_AUDIO_CTRL_IN_VD_)


#endif //_AUDIO_DEVICE_H_
