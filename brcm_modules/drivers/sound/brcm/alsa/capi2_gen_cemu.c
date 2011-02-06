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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/
#include "mobcom_types.h"

#include "resultcode.h"

#include "capi2_reqrep.h"

#define BCM_AUD_HAL_CAPI2_CID 102
extern void CAPI2_audio_control_dsp(UInt32 tid, UInt8 clientID, UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
extern void CAPI2_audio_control_generic(UInt32 tid, UInt8 clientID, UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
extern void CAPI2_RIPCMDQ_Connect_Uplink(UInt32 tid, UInt8 clientID, Boolean Uplink);
extern void CAPI2_RIPCMDQ_Connect_Downlink(UInt32 tid, UInt8 clientID, Boolean Downlink);
extern void CAPI2_AUDIO_Turn_EC_NS_OnOff(UInt32 tid, UInt8 clientID, Boolean ec_on_off, Boolean ns_on_off);
extern void CAPI2_AUDIO_ASIC_SetAudioMode(UInt32 tid, UInt8 clientID, AudioMode_t mode);
extern void CAPI2_program_FIR_IIR_filter(UInt32 tid, UInt8 clientID, UInt16 audio_mode);
extern void CAPI2_program_poly_FIR_IIR_filter(UInt32 tid, UInt8 clientID, UInt16 audio_mode);
extern void CAPI2_program_equalizer(UInt32 tid, UInt8 clientID, Int32 equalizer);
extern void CAPI2_program_poly_equalizer(UInt32 tid, UInt8 clientID, Int32 equalizer);
extern void CAPI2_VOLUMECTRL_SetBasebandVolume(UInt32 tid, UInt8 clientID, UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid);
extern UInt32 AUD_CreateTID(void);

extern void RPC_SyncWaitForResponse(void);


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_ASIC_SetAudioMode
//!
//! Description:	switch audio asic audio mode
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_ASIC_SetAudioMode(AudioMode_t mode)
{
    CAPI2_AUDIO_ASIC_SetAudioMode(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, mode);
    //RPC_SyncWaitForResponse();
}


//CAPI2
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	audio_control_dsp
//!
//! Description:	audio control dsp cmd
//!
/////////////////////////////////////////////////////////////////////////////
inline void audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4)
{
    CAPI2_audio_control_dsp(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, param1, param2, param3, param4);
    //RPC_SyncWaitForResponse();
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	audio_control_generic
//!
//! Description:	audio control generic cmd
//!
/////////////////////////////////////////////////////////////////////////////
inline void audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4)
{
    CAPI2_audio_control_generic(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, param1, param2, param3, param4);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_Set_DL_OnOff
//!
//! Description:	set downlink on/off
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_DRV_Set_DL_OnOff(Boolean on_off)
{
    CAPI2_RIPCMDQ_Connect_Downlink(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, on_off);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_ConnectMicrophoneUplink
//!
//! Description:	connect/disconnect microphne to uplink
//!
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_DRV_ConnectMicrophoneUplink(Boolean on_off)
{
    CAPI2_RIPCMDQ_Connect_Uplink(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, on_off);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	program_FIR_IIR_filter
//!
//! Description:	program_FIR_IIR_filter
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
inline void program_FIR_IIR_filter(UInt16 mode)
{
    CAPI2_program_FIR_IIR_filter(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, mode);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	program_poly_FIR_IIR_filter
//!
//! Description:	program_poly_FIR_IIR_filter
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
inline void program_poly_FIR_IIR_filter(UInt16 mode)
{
    CAPI2_program_poly_FIR_IIR_filter(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, mode);
    //RPC_SyncWaitForResponse();
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	program_equalizer
//!
//! Description:	program_equalizer
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void program_equalizer(Int32 equalizer)
{
    CAPI2_program_equalizer(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, equalizer);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	program_poly_equalizer
//!
//! Description:	program_poly_equalizer
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void program_poly_equalizer(Int32 equalizer)
{
    CAPI2_program_poly_equalizer(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, equalizer);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_Turn_EC_NS_OnOff
//!
//! Description:	turn EC and NS on/off
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_Turn_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off)
{
    CAPI2_AUDIO_Turn_EC_NS_OnOff(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, ec_on_off, ns_on_off);
    //RPC_SyncWaitForResponse();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	VOLUMECTRL_SetBasebandVolume
//!
//! Description:	set voice volume
//!
//! Notes: this api accepts either the logic level (platform mmi) or the db level (customer mmi)
//!
/////////////////////////////////////////////////////////////////////////////
void VOLUMECTRL_SetBasebandVolume(UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid)
{
    CAPI2_VOLUMECTRL_SetBasebandVolume(AUD_CreateTID(), BCM_AUD_HAL_CAPI2_CID, level, chnl, audio_atten, extid );
    //RPC_SyncWaitForResponse();
}

