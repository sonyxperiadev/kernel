/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*    @file   brcm_audio_devices.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
****************************************************************************/


#ifndef _BRCM_AUDIO_DEVICES_H__
#define _BRCM_AUDIO_DEVICES_H__

typedef struct _TIDChanOfPlaybackDev
{
    AUDIO_DRIVER_TYPE_t     drv_type;
    AUDIO_HW_ID_t           hw_id;
    AUDCTRL_SPEAKER_t       speaker;
}TIDChanOfPlaybackDev;


typedef struct _TIDChanOfCaptureDev
{
    AUDIO_DRIVER_TYPE_t     drv_type;
    AUDIO_HW_ID_t           hw_id;
    AUDCTRL_MICROPHONE_t       mic;
}TIDChanOfCaptureDev;

typedef struct _TIDChanOfVoiceCallDev
{
    AUDIO_HW_ID_t           in_hw_id;
    AUDCTRL_MICROPHONE_t       mic;
    AUDIO_HW_ID_t           out_hw_id;
    AUDCTRL_SPEAKER_t       speaker;

}TIDChanOfVoiceCallDev;



#endif	//_BRCM_AUDIO_THREAD_H__
