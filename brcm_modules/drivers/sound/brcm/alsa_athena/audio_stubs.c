/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
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
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "audio_consts.h"
#include "chip_version.h"
#include "sharedmem.h"
#include "xassert.h"
#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "rpc_api.h"

#include "rpc_sync_api.h"

#include "ripisr.h"

#include "i2s_drv_new.h"

static AP_SharedMem_t 			*dsp_shared_mem = NULL;

#if 0
//UInt32 RPC_SyncCreateTID( void* data, Int32 size ){ return 1; }

//Result_t RPC_SyncWaitForResponse( UInt32 tid, UInt8 cid, RPC_ACK_Result_t* ack, MsgType_t* msgType, UInt32* dataSize ){ return RESULT_OK; }


//RPC_Handle_t RPC_SyncRegisterClient(RPC_InitParams_t*	initParams, RPC_SyncInitParams_t* syncInitParams) {return (RPC_Handle_t) 1; }




void VPRIPCMDQ_StartCallRecording(
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
#if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	,
	UInt16	vp_speech_amr_mode		// [8|7|6..4|3..0] = [audio_proc_enable|AMR2_dtx|vp_speech_mode|vp_amr_mode]
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,00) /* BCM2132 and later */
	)
{
}

void VPRIPCMDQ_StartCallRecordAndPlayback(
	UInt8	vp_playback_mode,		// playback destination
	UInt8 	vp_record_mode,			// Audio path
	UInt8	buffer_length			// Buffer length
    ){}

void VPRIPCMDQ_StopRecording(void){}


void VPRIPCMDQ_StartRecordingPlayback(
	UInt8 	vp_playback_mode		// Audio path
#if CHIPVERSION >= CHIP_VERSION(ML2021,00) && CHIPVERSION <= CHIP_VERSION(ML2021,99) /* BCM2121 */
	,UInt8	disable_tch_vocoder		// TURE: disable TCH voice functions	
#elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
	,VP_PlaybackMix_Mode_t vp_playbackmix_mode		 // vp_playbackmix_mode = [0:3] = [non, ear_piece, UpCh, Both]                   
#endif // #elif CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
    ){}


void VPRIPCMDQ_CancelPlayback(void){}

void VPRIPCMDQ_CancelRecording(void){}

AP_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr()		// Return pointer to 32-bit aligned shared memory
{
	return dsp_shared_mem;
}

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb ){}

I2S_HANDLE I2SDRV_Init(I2S_BLK_ID_t id){}
	unsigned char I2SDRV_Cleanup(I2S_HANDLE handle);
    unsigned char I2SDRV_Set_mode(I2S_HANDLE handle, bool master){}

    I2S_STATE I2SDRV_Get_tx_state(I2S_HANDLE handle){}
    I2S_STATE I2SDRV_Get_rx_state(I2S_HANDLE handle){}

	
	unsigned char I2SDRV_Set_tx_format(I2S_HANDLE handle, int sampleRate,
					   bool stereo){}
	unsigned char I2SDRV_Set_rx_format(I2S_HANDLE handle, int sampleRate,
					   bool stereo,
                       I2S_RX_CHAN_t mono_from){}

	
	
    unsigned char I2SDRV_Start_rx_noDMA(I2S_HANDLE handle){}
    unsigned char I2SDRV_Stop_rx_noDMA(I2S_HANDLE handle){}
#endif
    int IPC_AudioControlApSend(char *buff, UInt32 len){}

    
    //UInt32 audio_control_generic(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4,UInt32 param5,UInt32 param6){}
    //UInt32 audio_control_dsp(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4,UInt32 param5,UInt32 param6){}
