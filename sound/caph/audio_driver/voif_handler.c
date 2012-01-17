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
*
*   @file   voif_handler.c
*
*   @brief  PCM data interface to DSP. 
*           It is used to hook with customer's downlink voice processing module. 
*           Customer will implement this.
*
****************************************************************************/
#include <linux/string.h>

#include "mobcom_types.h"
#include "audio_consts.h"
#include "voif_handler.h"
#include "audio_ddriver.h"
#include "log.h"

//
// APIs 
//

#ifdef INTERNAL_VOIF_TEST
static int voifDelay = 0;
static int voifGain = 0x4000; // In Q14 format, 0x4000 in Q14 == 1.0
#endif
static void *drv_handle = NULL;

static void VOIF_CB_Fxn (Int16 * ulData, Int16 *dlData, UInt32 sampleCount, UInt8 isCall16K)
{

#ifdef ENABLE_VOIF
/* The flag "ENABLE_VOIF" will be enabled by customer.  custormer will hook up their voice solution in callback */
#else

#ifdef INTERNAL_VOIF_TEST
    if (voifDelay == 0)
    {
        // copy the ulData to dlData without delay, hear own voice loopback.
        memcpy(dlData, ulData, sampleCount * sizeof(Int16));
    }
    else if (voifDelay == 1)
    {
        Int32 t, i;
		// Gain test, change the gain of the downlink, should hear volume difference
        for( i=0; i<sampleCount; i++ )
        {
            t = (Int32) *dlData;
            t = (t * voifGain)>>14;
            *dlData++ = (Int16)(t&0xffff);
        }
    }
    else
    {
        // delay test, mute the downlink
        memset (dlData, 0, sampleCount * sizeof(Int16));
    }

    mdelay (voifDelay);
#endif
#endif

	//To Be Filled by customer
    return;
}

// Start voif 
void VoIF_init(AudioMode_t mode)
{
	drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIF);
	AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_VOIF_CB,(void*)VOIF_CB_Fxn);
	AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_START,NULL);
    return;
}

void VoIF_Deinit()
{
	AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_STOP,NULL);
	AUDIO_DRIVER_Close(drv_handle);
	drv_handle = NULL;
	
    return;
}
 
#ifdef INTERNAL_VOIF_TEST
void VoIF_SetDelay(int delay)
{
	voifDelay = delay;
}

void VoIF_SetGain(int gain)
{
	voifGain = gain;
}
#endif
