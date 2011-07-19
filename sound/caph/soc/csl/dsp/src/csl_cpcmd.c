//*********************************************************************
//
//	Copyright © 2000-2010 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csl_cpcmd.c
*
*   @brief  This file contains CP command interface to DSP
*
****************************************************************************/

//#define ENABLE_LOGGING
 
#include "mobcom_types.h"
#include "chip_version.h"
#include "dsp_feature_def.h"
#include "assert.h"
#include "sharedmem.h"
#include "ripcmdq.h"
#include "sysparm.h"
#include "memmap.h"
#include "sleep.h"
#include "ostypes.h"
#include "mti_trace.h"
#include "dspcmd.h"
#include "log.h"

static Boolean is_audio_on = FALSE;
static Boolean is_ul_on = FALSE;
static Boolean is_dl_on = FALSE;



#define AUXI_MIC_ON				0x0010
#define AUXI_MIC_OFF			0x0000
#define	AMCR_CONTROL_MASK		0x4010
#define	AMCR_8K_MODE			0
#define	AMCR_16K_MODE			1


//static UInt8	audio_chnl = 0;

static UInt8	sleep_id;
static UInt16  amcr_control = 0;

#ifndef DSP_COMMS_INCLUDED
//put dummy functions for BSP build
UInt16 SYSPARM_GetDTMFToneScaleFactor(void){return 0;}
Int16 SYSPARM_GetDebug(	UInt16 index ) {return 0;}
Boolean SYSPARM_GetDspSidetone( void ){return FALSE;}
#endif //DSP_COMMS_INCLUDED

extern Boolean SYSPARM_GetDspSidetone( void );

//******************************************************************************
//
// Function Name:	RIPCMDQ_Reset
//
// Description: put reset in command queue.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ResetRIP()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_RESET_RIP;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_StartDAITest
//
// Description:		Notify the RIP to start DAI testing.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StartDAITest(
	UInt8 device
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_START_DAI_TEST;

	msg.arg0 = ( device == 4 ) ? 3 : device;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_StopDAITest
//
// Description:		Notify the RIP to termiante DAI testing.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StopDAITest()
{
	CmdQ_t msg;

	msg.cmd = COMMAND_STOP_DAI_TEST;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_Read
//
// Description:		Read an value from a location in the RIPs memory map.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_Read(
	UInt16 addr,
	UInt16 tag
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_READ;
	msg.arg0 = addr;
	msg.arg1 = tag;
	msg.arg2 = 0;
	
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_Write
//
// Description:		Write a value to an address in the RIPs memory map.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_Write(
	UInt16 addr,
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_WRITE;
	msg.arg0 = addr;
	msg.arg1 = value;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}



#ifdef DSP_FAST_COMMAND
//******************************************************************************
//
// Function Name:	RIPCMDQ_FastRead
//
// Description:	   Fast	read an value from a location in the RIPs memory map.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FastRead(
	UInt16 addr,
	UInt16 tag,
    UInt16 shared_page
	)
{
	CmdQ_t msg;

	msg.cmd = FAST_COMMAND_READ;
	msg.arg0 = addr;
	msg.arg1 = tag;
	msg.arg2 = shared_page;
	
    SHAREDMEM_PostFastCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_FastRead_DSPMem
//
// Description:	   Fast	read an value from a location in the RIPs memory map.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FastRead_DSPMem(
	UInt16 lowaddr,
	UInt16 tag,
    UInt16 highaddr
	)
{
	CmdQ_t msg;

	msg.cmd = FAST_COMMAND_READ_DSPMEM;
	msg.arg0 = lowaddr;
	msg.arg1 = tag;
	msg.arg2 = highaddr;
	
    SHAREDMEM_PostFastCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_Write
//
// Description:		FastWrite a value to an address in the RIPs memory map.
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_FastWrite(
	UInt16 addr,
	UInt16 value,
	UInt16 shared_page
	)
{
	CmdQ_t msg;

	msg.cmd = FAST_COMMAND_WRITE;
	msg.arg0 = addr;
	msg.arg1 = value;
	msg.arg2 = shared_page;
	
    SHAREDMEM_PostFastCmdQ( &msg );
}

#endif// DSP_FAST_COMMAND


//******************************************************************************
//
// Function Name:	
//
// Description:		
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_WritePRAM(				// Write PRAM command
	UInt16 addr,					// Address
	UInt16 value					// Value
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_PRAM_WRITE;
	msg.arg0 = addr;
	msg.arg1 = value;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}



//******************************************************************************
//
// Function Name:	RIPCMDQ_SetCal
//
// Description:		Load the sleep parameters
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetCal(					// Load the sleep parameters
	UInt16 cacmp_shift,					// # of bits to shift CASR for SFGIR, SFGFR
	UInt32 casr,						// Slow clock value
	UInt32 ratio						// CAFR/CASR ratio
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_SET_SLEEP_CASR;
	msg.arg0 = cacmp_shift;
	msg.arg1 = (UInt16)(casr >> 16);
	msg.arg2 = (UInt16)(casr & 0x0000FFFF);
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_SET_SLEEP_RATIO;
	msg.arg0 = (UInt16)(ratio >> 16);
	msg.arg1 = (UInt16)(ratio & 0x0000FFFF);
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_StartTone
//
// Description:		Enables DTMF or SV tone generation
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StartTone(
	UInt8	tone,
	UInt8	tone_duration
	)
{
	CmdQ_t	msg;

//	RIPCMDQ_Connect(TRUE,TRUE);
	SLEEP_DisableDeepSleep( sleep_id );

	// Poke DSP addresses to clear accumulator registers
	// note that these addresses are different for different chip revisions


	msg.cmd = COMMAND_GEN_TONE;
	msg.arg0 = tone;
	msg.arg1 = tone_duration; 	// each count is 1ms
	{
		// only used by DSP when shared_DTMF_SV_tone_scale_mode = 1;
		UInt32 temp_tone_scale;
		if ( tone < 16)
		{
			temp_tone_scale = ( 0x16A1 * SYSPARM_GetDTMFToneScaleFactor() ) >> 13;		// 1/sqrt(2) * DTMF_SV_tone_scale_factor
		}
		else
		{
			temp_tone_scale = SYSPARM_GetDTMFToneScaleFactor();						// 1 * DTMF_SV_tone_scale_factor
		}
		msg.arg2 = (UInt16) temp_tone_scale; 	// scale factor
	}
	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetupToneGeneric
//
// Description:		Configures parameters for generic tri-tone generation
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetupToneGeneric(
	UInt16 superimpose_flag,				//  
	UInt16 duration,						//  
	UInt16 tone_scale						//  
	)
{
	CmdQ_t	msg;

	RIPCMDQ_Connect(TRUE, TRUE, FALSE);
	SLEEP_DisableDeepSleep( sleep_id );

	msg.cmd = COMMAND_PARM_TONE_GENERIC;
	msg.arg0 = superimpose_flag;
	msg.arg1 = duration;
	msg.arg2 = tone_scale;
	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_StartToneGeneric
//
// Description:		Enables generic tri-tone generation
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StartToneGeneric(
	UInt16 f1,						//  first tone frequency
	UInt16 f2,						//  second tone frequency
	UInt16 f3						//  third tone frequency
	)
{
	CmdQ_t	msg;

	RIPCMDQ_Connect(TRUE, TRUE, FALSE);
	SLEEP_DisableDeepSleep( sleep_id );

	// Poke DSP addresses to clear accumulator registers
	// note that these addresses are different for different chip revisions

	msg.cmd = COMMAND_GEN_TONE_GENERIC;
	msg.arg0 = f1;
	msg.arg1 = f2;
	msg.arg2 = f3;
	SHAREDMEM_PostCmdQ(&msg);
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_StopTone
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_StopTone()
{
	CmdQ_t	msg;

	SLEEP_EnableDeepSleep(sleep_id );
 
	msg.cmd = COMMAND_STOP_TONE;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);
}

		
//******************************************************************************
//
// Function Name:	RIPCMDQ_IsAudioOn
//
// Description:		 
//
// Notes:
//
//******************************************************************************

Boolean RIPCMDQ_IsAudioOn( void )
{
	return is_audio_on;
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SetAudioOn
//
// Description:		 
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_SetAudioOn( Boolean audio_on )
{
	is_audio_on = audio_on;
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_EnableAudio
//
// Description:
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_EnableAudio(
	Boolean	audio_on,
	Boolean MMVPU_16KHz_sample_rate,  //WB-AMR
	Boolean voice_call_16KHz_sample_rate  //voice call
	)
{
	CmdQ_t	msg;
 
	msg.cmd = COMMAND_AUDIO_ENABLE;
	msg.arg0 = (UInt16)audio_on;
	msg.arg1 = (UInt16)MMVPU_16KHz_sample_rate; //0: AMCR_8K_MODE, 1: AMCR_16K_MODE
	msg.arg2 = voice_call_16KHz_sample_rate; // voice call 1:16Khz, 0:8Khz

	SHAREDMEM_PostCmdQ(&msg);

    is_audio_on = audio_on;

	if ( SYSPARM_GetDebug(0) != 0 )
	{
		if ( audio_on )
		{
			RIPCMDQ_StartDAITest( 4 );
		}
		else
		{
			RIPCMDQ_StopDAITest();
		}
	}
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_SwitchAudioChnl
//
// Description:		switch audio channel while audio is already on.
//
// Notes:			for ISR to call when earphone is plugged/unplugged
//
//******************************************************************************

void RIPCMDQ_SwitchAudioChnl(
	Boolean	isAuxChnl
	)
{
	//this can be emptied.
	//this operation is in Audio_SwitchAudioMode( ).
	return;
}

static void RIPCMDQ_Connect_local(
   	Boolean	Uplink,
   	Boolean Downlink,
   	Boolean sample_rate
   	)
{
#if defined(FUSE_COMMS_PROCESSOR)
  	CmdQ_t	msg;
   
   	msg.cmd = COMMAND_AUDIO_CONNECT;
   	msg.arg0 = Uplink;
    msg.arg1 = Downlink;
	msg.arg2 = 0;
   	SHAREDMEM_PostCmdQ(&msg);
/**
    // Do not send audio enable or disable CMD if audio is already enabled or disabled.
    if (is_audio_on)
	{
		if ( Downlink == FALSE && Uplink == FALSE )
		{// From on to off
			//RIPCMDQ_EnableAudio( FALSE, sample_rate );
		}	
	}
	else
	{
		if ( Downlink || Uplink )
		{// From off to on
	   		//RIPCMDQ_EnableAudio( TRUE, sample_rate );
		}
	}
*/    
    L1_LOGV("RIPCMDQ_Connect_local", (Uplink<<10 | Downlink<<9 | is_audio_on<<8 | amcr_control));
	
#ifdef DSP_COMMS_INCLUDED
	//WE (CP) SHOULD DO NOISE SUPPRESSION EVEN IF AUDIO CONTROL IS DONE BY AP!!!
	if ( Uplink ) RIPCMDQ_ConfigNoiseSuppression( SYSPARM_IsNSEnabled() );
	else RIPCMDQ_ConfigNoiseSuppression( 0 );

	if(Downlink) 
		RIPCMDQ_ControlDownlinkNoiseSuppression(SYSPARM_IsDownlinkNSEnabled());
	else 
		RIPCMDQ_ControlDownlinkNoiseSuppression(FALSE);
#endif// DSP_COMMS_INCLUDED

#endif
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_Connect
//
// Description:
//
// Notes:
//
//******************************************************************************
   
void RIPCMDQ_Connect(
   	Boolean	Uplink,
   	Boolean Downlink,
   	Boolean sample_rate
   	)
{
	if((is_ul_on == Uplink) && (is_dl_on == Downlink))
	{
	    L1_LOGV("Connect Audio Both, No effect", (Uplink<<1 | Downlink));
	}
	else
	{
		is_ul_on = Uplink;
		is_dl_on = Downlink;
		RIPCMDQ_Connect_local(is_ul_on, is_dl_on, sample_rate);
	}
}

void RIPCMDQ_Connect_Uplink(
   	Boolean	Uplink,
   	Boolean sample_rate
   	)
{
	if(is_ul_on == Uplink)
	{
	    L1_LOGV("Connect Audio Uplink, No effect",Uplink);   	
	}
	else
	{
		is_ul_on = Uplink;
		RIPCMDQ_Connect_local(is_ul_on, is_dl_on, sample_rate);
	}
}

void RIPCMDQ_Connect_Downlink(
   	Boolean	Downlink,
   	Boolean sample_rate
   	)
{
	if(is_dl_on == Downlink)
	{
	    L1_LOGV("Connect Audio Downlink, No effect",Downlink);   	
	}
	else
	{
		is_dl_on = Downlink;
		RIPCMDQ_Connect_local(is_ul_on, is_dl_on, sample_rate);
	}
}

void RIPCMDQ_Switch_AudioProfile(
	UInt8	Is_spkr_mode
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_SWITCH_AUDIO_PROFILE;
	msg.arg0 = Is_spkr_mode;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
	
}	

//******************************************************************************
//
// Function Name:	RIPCMDQ_GetUlDlAudioConnect
//
// Description:		Get the current audio connect status for the uplink/downlink
//					
//
// Notes:
//
//******************************************************************************
UInt16 RIPCMDQ_GetUlDlAudioConnect( void )
{
	return (is_ul_on << 1)| is_dl_on;
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_GetVersionID
//
// Description:		Get version ID
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_GetVersionID()				// Get version ID
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_GET_VERSION_ID;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}



//******************************************************************************
//
// Function Name:	RIPCMDQ_CheckSharedMemorySize
//
// Description:		Get shared memory size
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_CheckSharedMemorySize()				// Get shared memory size
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_GET_SHARED_MEM_SIZE;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	L1_LOGV4("Command to get shared memory size from DSP",msg.cmd, msg.arg0, msg.arg1, msg.arg2);

	SHAREDMEM_PostCmdQ(&msg);
}


//******************************************************************************
//
// Function Name:  RIPCMDQ_Init
//
// Description:  
//
// Notes:
//
//******************************************************************************
#ifdef DSP_COMMS_INCLUDED

void RIPCMDQ_Init(void)
{
	sleep_id = SLEEP_AllocId();
}
#endif

//******************************************************************************
//
// Function Name:	Send PATCH command to DSP
//
// Description:		Set COMMAND_PATCH
//
// Notes:
//
//******************************************************************************
void RIPCMD_DSP_PATCH(UInt16 addr, UInt16 val1, UInt16 val2)
{
	CmdQ_t msg;
   
	msg.cmd = COMMAND_PATCH;
	msg.arg0 = addr;
	msg.arg1 = val1;   
	msg.arg2 = val2;   
	SHAREDMEM_PostCmdQ( &msg );
}

	//******************************************************************************
//
// Function Name:	Send PATCH command to DSP
//
// Description:		Set COMMAND_PATCH
//
// Notes:
//
//******************************************************************************
void RIPCMD_DSP_PATCH_ENABLE(UInt16 val0, UInt16 val1, UInt16 val2)
{
	CmdQ_t msg;
   
	msg.cmd = COMMAND_PATCH_ENABLE;
	msg.arg0 = val0;				// 0 disable, 1 enable
	msg.arg1 = val1;				// patch index
	msg.arg2 = 0;					// not used
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	Send AMR RUN command to DSP
//
// Description:		Set COMMAND_MAIN_AMR_RUN
//
// Notes:
//
//******************************************************************************
void RIPCMD_DSP_AMR_RUN(UInt16 type, Boolean amr_if2_enable, UInt16 mst_flag)
{
	CmdQ_t msg;
   
	msg.cmd = COMMAND_MAIN_AMR_RUN;
	msg.arg0 = type;
	msg.arg1 = amr_if2_enable;
	msg.arg2 = mst_flag;
	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_EnableAudioAlign
//
// Description:		
//
// Notes:
//
//******************************************************************************

void RIPCMDQ_EnableAudioAlign(					// Enable DTX mode
	Boolean enable						// TRUE, enable DTX
	)
{
	CmdQ_t 	msg;

	msg.cmd = COMMAND_AUDIO_ALIGNMENT;
	msg.arg0 = enable;
	msg.arg1 = enable;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ(&msg);
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_WriteBitwiseSlow
//
// Description:		Write bitwise value to an address in the DSPs memory map using
//					the default SLOW Command Q which is serviced by the DSP in the
//					background
//
// Parameters:

//  addr - RIP Register address

//  mask -1 << BIT_TO_SET 

//  value: Different depending on wether you are setting a bit or clearing a bit
//   Set Bit: 1 << BIT_TO_SET. i.e. same as mask
//   Clear Bit: 0 << BIT_TO_SET
//
//
// Notes:
//
// Example: Set Bit 5 of a 16 Bit Register with Address 0xABCD
//
// BIT_MASK = 1<<5
//
// Set a Bit    :    RIPCMDQ_WriteBitwiseSlow(0xABCD, (UInt16)BIT_MASK, (UInt16)BIT_MASK)
//
// Clear a Bit : RIPCMDQ_WriteBitwiseSlow(0xABCD, (UInt16)BIT_MASK, (UInt16)0)
//
//******************************************************************************
void RIPCMDQ_WriteBitwiseSlow(
	UInt16 addr,
	UInt16 mask,
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_BITWISE_WRITE;
	msg.arg0 = addr;
	msg.arg1 = mask;
	msg.arg2 = value;

	SHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_WriteBitwise
//
// Description:		Write bitwise value to an address in the RIPs memory map.
//
// Parameters:

//  addr - RIP Register address

//  mask -1 << BIT_TO_SET 

//  value: Different depending on wether you are setting a bit or clearing a bit
//   Set Bit: 1 << BIT_TO_SET. i.e. same as mask
//   Clear Bit: 0 << BIT_TO_SET
//
//
// Notes:
//
// Example: Set Bit 5 of a 16 Bit Register with Address 0xABCD
//
// BIT_MASK = 1<<5
//
// Set a Bit    :    RIPCMDQ_WriteBitwise(0xABCD, (UInt16)BIT_MASK, (UInt16)BIT_MASK)
//
// Clear a Bit : RIPCMDQ_WriteBitwise(0xABCD, (UInt16)BIT_MASK, (UInt16)0)
//
//******************************************************************************
void RIPCMDQ_WriteBitwise(
	UInt16 addr,
	UInt16 mask,
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = FAST_COMMAND_BITWISE_WRITE;
	msg.arg0 = addr;
	msg.arg1 = mask;
	msg.arg2 = value;

	SHAREDMEM_PostFastCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_MST_FLAG_EFR
//
// Description:		Set shared_MST_flag_EFR
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_MST_FLAG_EFR(
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_MST_FLAG_EFR;
	msg.arg0 = value;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_MST_FLAG_AFS
//
// Description:		Set shared_MST_flag_AFS
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_MST_FLAG_AFS(
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_MST_FLAG_AFS;
	msg.arg0 = value;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_MST_FLAG_AHS
//
// Description:		Set shared_MST_flag_AHS
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_MST_FLAG_AHS(
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_MST_FLAG_AHS;
	msg.arg0 = value;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	RIPCMDQ_MST_FLAG_CS1
//
// Description:		Set shared_MST_flag_CS1
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_MST_FLAG_CS1(
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_MST_FLAG_CS1;
	msg.arg0 = value;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_FLAG_SAIC
//
// Description:		Set shared_flag_SAIC
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_FLAG_SAIC(
	UInt16 value
	)
{
	CmdQ_t msg;

	msg.cmd = COMMAND_FLAG_SAIC;
	msg.arg0 = value;
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}

//******************************************************************************
//
// Function Name:	 RIPCMDQ_SetRIPAMCR
//
// Description:		Set RIP AMCR (Audio Module Control Register)
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetRIPAMCR( 
	UInt16 value
	)
{
}



//******************************************************************************
//
// Function Name: RIPCMDQ_SetDSPSidetoneEnable
//
// Description: The DSP sidetone enable/disable are only read one time at
// system boot-up time. So they need to be poked at run-time
// to dynamically Enable/Disable the feature
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetDSPSidetoneEnable( 
	UInt16	aud_mode,
#if defined(USE_NEW_AUDIO_PARAM)
	UInt16	aud_app,
#endif
	Boolean audioTuningMode
	)
{
	SharedMem_t	*mem = SHAREDMEM_GetSharedMemPtr();
	SysAudioParm_t	*audioParam_p;

#if defined(USE_NEW_AUDIO_PARAM)
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode, aud_app);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode, aud_app);
	}

#else
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode);
	}
#endif
	// Setup DSP sidetone filter control parameters
	mem->shared_dsp_sidetone_enable = audioParam_p->audio_dsp_sidetone;

}

void RIPCMDQ_SetDSPSidetoneGain( 
	UInt16	aud_mode,
#if defined(USE_NEW_AUDIO_PARAM)
	UInt16	aud_app,
#endif
	Boolean audioTuningMode
	)
{

	SharedMem_t	*mem = SHAREDMEM_GetSharedMemPtr();
	SysAudioParm_t	*audioParam_p;

#if defined(USE_NEW_AUDIO_PARAM)
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode, aud_app);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode, aud_app);
	}
#else
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode);
	}
#endif
	// Setup DSP sidetone filter control parameters
	mem->shared_sidetone_biquad_sys_gain = audioParam_p->sidetone_biquad_sys_gain;
	mem->shared_sidetone_biquad_scale_factor = audioParam_p->sidetone_biquad_scale_factor;
	mem->shared_sidetone_output_gain = audioParam_p->sidetone_output_gain;

}


//******************************************************************************
//
// Function Name:	RIPCMDQ_ConfigSidetoneFilter
//
// Description:		Generate Sidetone filter coefficients to command queue
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ConfigSidetoneFilter(  // Generate filter coefficients to command queue
	UInt16	aud_mode,
#if defined(USE_NEW_AUDIO_PARAM)
	UInt16	aud_app,
#endif
	Boolean audioTuningMode
	)
{
#if !defined(SYSPARM_2_INCLUDED)

	UInt16 *ptr;
	CmdQ_t 	msg;
	SysAudioParm_t	*audioParam_p;

#if defined(USE_NEW_AUDIO_PARAM)
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode, aud_app);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode, aud_app);
	}
#else
	if (!audioTuningMode)
	{
		// obtain the audio parameter set from flash
		audioParam_p = SYSPARM_GetAudioParamsFromFlash(aud_mode);
	}
	else
	{
		// obtain the audio parameter set from RAM
		audioParam_p = SYSPARM_GetAudioParamsFromRAM(aud_mode);
	}
#endif //defined(USE_NEW_AUDIO_PARAM)

	//ptr = &sys_data_ind->sidetone_filter[ 0 ];
	ptr = & audioParam_p->sidetone_filter[ 0 ];

	// Need to send 10 filter coefficients for the 2 biquads
	msg.cmd = COMMAND_SIDETONE_COEFS_012;
	msg.arg0 = *ptr++;
	msg.arg1 = *ptr++;
	msg.arg2 = *ptr++;
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_SIDETONE_COEFS_345;
	msg.arg0 = *ptr++;
	msg.arg1 = *ptr++;
	msg.arg2 = *ptr++;
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_SIDETONE_COEFS_678;
	msg.arg0 = *ptr++;
	msg.arg1 = *ptr++;
	msg.arg2 = *ptr++;
	SHAREDMEM_PostCmdQ( &msg );

	msg.cmd = COMMAND_SIDETONE_COEFS_9;
	msg.arg0 = *ptr++;
	msg.arg1 = 0;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );

#endif

}

//******************************************************************************
//
// Function Name:	RIPCMDQ_LoadSysparm
//
// Description:		Force DSP update some parameters.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_LoadSysparm( void )
{
	CmdQ_t msg;

	msg.cmd = COMMAND_LOAD_SYSPARM;
	msg.arg0 = 0;	
	msg.arg1 = 0;
	msg.arg2 = 0;

	SHAREDMEM_PostCmdQ( &msg );
}


//******************************************************************************
//
// Function Name:	RIPCMDQ_CheckSumTest
//
// Description:		Check sum test for DROM tables.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_CheckSumTest(Int32 addr )
{
	CmdQ_t msg;

	msg.cmd = COMMAND_CHECK_SUM_TEST;
	msg.arg0   = addr & 0x0000FFFF;
	msg.arg1   = (addr >> 16) & 0x0000FFFF;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ( &msg );

}

//******************************************************************************
//
// Function Name:	RIPCMDQ_ControlEchoCancellation
//
// Description:
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ControlEchoCancellation(void)
{
 CmdQ_t	cmd_msg;	
 
	cmd_msg.cmd = COMMAND_ECHO_CANCELLATION;
	cmd_msg.arg0 = 1;	// 1==ON, 0==Off
	cmd_msg.arg1 = 0;
	cmd_msg.arg2 = 0;	// bit0: NLP enable/disable, bit1: Comfort noise on/off
	SHAREDMEM_PostCmdQ(&cmd_msg);
}

//******************************************************************************
//
// Function Name:   void RIPCMDQ_ConfigNoiseSuppression( )
//
// Description:     Configure Noise Suppression
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_ConfigNoiseSuppression(Boolean ns_on)
{
 CmdQ_t  msg;
        
    msg.cmd = COMMAND_NOISE_SUPPRESSION;
    msg.arg0 = ns_on;
	msg.arg1 = 0;
	msg.arg2 = 0;
    SHAREDMEM_PostCmdQ(&msg);
}



//******************************************************************************
//
// Function Name:  RIPCMDQ_SetVoiceDtx
//
// Description:  This function sets codec DTX mode.
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_SetVoiceDtx(Boolean enable)
{
 CmdQ_t  msg;

  msg.cmd = COMMAND_DTX_ENABLE;
  msg.arg0 = enable;
  msg.arg1 = 0;
  msg.arg2 = 0;

  SHAREDMEM_PostCmdQ(&msg);

}

//******************************************************************************
//
// Function Name:	RIPCMDQ_CompFilterCtrl
//
// Description: 
// Controlling the comp filtering for Rhea:
// arg0 is concatenating 2 flags for resetting the 
//
// Notes:
//
//******************************************************************************
void RIPCMDQ_CompFilterCtrl(UInt16 ResetUlFilterState, UInt16 ResetUlAuxFilterState, UInt16 ResetDlFilterState, UInt16 MainMicCtrl, UInt16 AuxMicCtrl, UInt16 DlSpkCtrl)
{
 CmdQ_t  msg;
 UInt16 arg0, arg1;

	arg0 = (( ResetDlFilterState & 0x1) |((ResetUlFilterState<<1) & 0x2) | ((ResetUlAuxFilterState<<2) & 0x4));
	arg1 = (((DlSpkCtrl & 0x1)) | ((MainMicCtrl<<1) & 0x2) | ((AuxMicCtrl<<2) & 0x4));
 
	msg.cmd = COMMAND_VOICE_FILTER_COEFS;
	msg.arg0 = arg0;
	msg.arg1 = arg1;
	msg.arg2 = 0;
	SHAREDMEM_PostCmdQ(&msg);

}

//*********************************************************************
/**
*
*   RIPCMDQ_ControlDownlinkNoiseSuppression controls Downlink Noise Suppressor.
*
*   @param    control				(in)	0 - disable, 1 - enable
* 
**********************************************************************/
void RIPCMDQ_ControlDownlinkNoiseSuppression(Boolean control)
{
 CmdQ_t  msg;
        
    msg.cmd = COMMAND_DOWNLINK_NOISE_SUPPRESSION;
    msg.arg0 = control;
	msg.arg1 = 0;
	msg.arg2 = 0;
    SHAREDMEM_PostCmdQ(&msg);

}
