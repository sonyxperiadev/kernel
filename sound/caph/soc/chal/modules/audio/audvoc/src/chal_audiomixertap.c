/*******************************************************************************************
Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
* @file   chal_audiomixertap.c
* @brief  Audio cHAL layer of Audio Mixer Taps
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_dsp_audio.h"
#include "brcm_rdb_util.h"
#include "chal_types.h"
#include "chal_audiomisc.h"
#include "chal_audiomixertap.h"
#include "log.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32 base;         // may be mapped and != AHB_AUDIO_BASE_ADDR
	UInt32	inited;
} chal_audio_mixer_tap_t;

// rely on zero inited
static chal_audio_mixer_tap_t sMixerTap = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiomixertap_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiomixertap_Init(cUInt32 baseAddr)
{
	if (sMixerTap.inited == 0) 
    {
        sMixerTap.base = baseAddr;
		sMixerTap.inited = 1;
        return (CHAL_HANDLE)&sMixerTap;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiomixertap_SetNbSampleRate
//
// Description:   Set sample rate in NB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_SetNbSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		)
{
	if ( samp_rate==AUDIO_SAMPLING_RATE_8000 )
		BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTNB_RATE, 0 );
	else
		if ( samp_rate==AUDIO_SAMPLING_RATE_16000 )
			BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTNB_RATE, 1 );	
}

//============================================================================
//
// Function Name: chal_audiomixertap_SetWbSampleRate
//
// Description:   Set sample rate in WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_SetWbSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		)
{
	if ( samp_rate==AUDIO_SAMPLING_RATE_44100 )
		BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, WBRATE, 0 );
	else
		if ( samp_rate==AUDIO_SAMPLING_RATE_48000 )
			BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, WBRATE, 1 );
}

//============================================================================
//
// Function Name: chal_audiomixertap_SetWbGain
//
// Description:   Set gain in WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_SetWbGain(
		CHAL_HANDLE handle,
		cUInt16		left_gain_hex,
		cUInt16		right_gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sMixerTap.base, DSP_AUDIO_BTMIXER_GAIN_L_R, BTMIXER_GAIN_L, left_gain_hex);
	BRCM_WRITE_REG_FIELD(sMixerTap.base, DSP_AUDIO_BTMIXER_GAIN_R_R, BTMIXER_GAIN_R, right_gain_hex);
}

//============================================================================
//
// Function Name: chal_audiomixertap_EnableWbDma
//
// Description:   Enable DMA in WB mixer Tap
//
//============================================================================
cVoid chal_audiomixertap_EnableWbDma(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixertap_EnableWbDma" );
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_DMA_EN, enable );
}

//============================================================================
//
// Function Name: chal_audiomixertap_DisableWbAudInt
//
// Description:   Disable audio interrupt in WB mixer Tap
//
//============================================================================
cVoid chal_audiomixertap_DisableWbAudInt(
		CHAL_HANDLE handle,
		Boolean		disable
		)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixertap_DisableWbAudInt" );
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_WBINTDIS, disable );
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadWbIntFlag
//
// Description:   Read BT Taps wide band interrupt flag
//
//============================================================================
cUInt16 chal_audiomixertap_ReadWbIntFlag( CHAL_HANDLE handle )
{
	return BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_WBINTFLAG );
	//return BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_AUDVOC_ISR_R, BTTAP_WB_INT );	
}

//============================================================================
//
// Function Name: chal_audiomixertap_ClearWbIntFlag
//
// Description:   Clear BT Taps wide band interrupt flag
//
//============================================================================
cVoid chal_audiomixertap_ClearWbIntFlag( CHAL_HANDLE handle )
{
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_WBINTFLAG, 0 );
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbIntFlag
//
// Description:   Read BT Taps narrow band interrupt flag
//
//============================================================================
cUInt16 chal_audiomixertap_ReadNbIntFlag( CHAL_HANDLE handle )
{
	return BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_AUDVOC_ISR_R, BTTAP_NB_INT );	
}

//============================================================================
//
// Function Name: chal_audiomixertap_ClearNbIntFlag
//
// Description:   Clear BT Taps narrow band interrupt flag
//
//============================================================================
cVoid chal_audiomixertap_ClearNbIntFlag( CHAL_HANDLE handle )
{
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_AUDVOC_ISR_R, BTTAP_NB_INT, 0 );
}

//============================================================================
//
// Function Name: chal_audiomixertap_SwapWbLeftRight
//
// Description:   Swap left and right channel in WB mixer Tap
//
//============================================================================
cVoid chal_audiomixertap_SwapWbLeftRight(
		CHAL_HANDLE handle,
		Boolean		swap
		)
{
	if(FALSE==swap)
		BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, WBLRSW, 1 );  //not swap
	else
		BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, WBLRSW, 0 );  //swap
}

//============================================================================
//
// Function Name: chal_audiomixertap_EnableNb
//
// Description:   Enable or Disable NB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_EnableNb(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixertap_EnableNb: enable = %d", enable );
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTNB_ENABLE, enable );
}

//============================================================================
//
// Function Name: chal_audiomixertap_EnableWb
//
// Description:   Enable or Disable WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_EnableWb(
		CHAL_HANDLE handle,
		Boolean		enable_wbtap,
		Boolean		enable_dac
		)
{
	cUInt16 regValue=0;

	if(enable_wbtap)
	{
		regValue +=2;
		if(enable_dac) regValue +=1;
	}
	
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixertap_EnableWb: enable_wbtap %d, WBenable = %d", enable_wbtap, regValue );
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, WBENABLE, regValue );
}

//============================================================================
//
// Function Name: chal_audiomixertap_SelectInput
//
// Description:   Select input source for mixer tap input
//
//============================================================================
cVoid chal_audiomixertap_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIXER_TAP_en tap,
		CHAL_AUDIO_MIXER_en		input_source
		)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixertap_SelectInput: tap = 0x%x, input_source = 0x%x", tap, input_source );

	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, PATH_SEL, tap );
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, PATH_SOURCE, input_source );
	/**
	*(volatile DSP_AUDIO_BTMIXER_CFG2_R_TYPE *) ( sMixerTap.base + DSP_AUDIO_BTMIXER_CFG2_R_OFFSET )
		|=    (tap << DSP_AUDIO_BTMIXER_CFG2_R_PATH_SEL_SHIFT)
			+ (input_source << DSP_AUDIO_BTMIXER_CFG2_R_PATH_SOURCE_SHIFT);
	*/
}

//============================================================================
//
// Function Name: chal_audiomixertap_ClrNbFifo
//
// Description:   Clear FIFO in NB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_ClrNbFifo(CHAL_HANDLE handle, Boolean clrbit)
{
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_VOICEFIFO_CLEAR_R, BTNBFIFO_CLR, clrbit );  //address is 0A94
}

//============================================================================
//
// Function Name: chal_audiomixertap_ClrWbFifo
//
// Description:   Clear FIFO in WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_ClrWbFifo(CHAL_HANDLE handle, Boolean clrbit)
{
	//?
}

//============================================================================
//
// Function Name: chal_audiomixertap_ResetWbTap
//
// Description:   Clear BTMIXER_CFG reg in WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_ResetWbTap(CHAL_HANDLE handle)
{
	BRCM_WRITE_REG(sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, 0x0);
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_WBFIFOTHRES, 0x40 );
}

//============================================================================
//
// Function Name: chal_audiomixertap_SetNbFifoThres
//
// Description:   Set FIFO threshold in NB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_SetNbFifoThres(
		CHAL_HANDLE handle,
		cUInt16		thres
		)
{
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_VOICEFIFO_THRES_R, BTNBFIFO_THRES, thres );
}

//============================================================================
//
// Function Name: chal_audiomixertap_SetWbFifoThres
//
// Description:   Set FIFO threshold in WB MIXER TAP
//
//============================================================================
cVoid chal_audiomixertap_SetWbFifoThres(
		CHAL_HANDLE handle,
		cUInt16		thres
		)
{
	BRCM_WRITE_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG_R, BTMIXER_WBFIFOTHRES, thres );
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbFifoStatus
//
// Description:   Read FIFO status from narrow band MIXER TAP
//
//============================================================================
cUInt16 chal_audiomixertap_ReadNbFifoStatus(CHAL_HANDLE handle)
{
	return 0;  //narrowband bt tap fifo has no under flow flag.
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadWbFifoStatus
//
// Description:   Read FIFO status from wideband MIXER TAP
//
//============================================================================
cUInt16 chal_audiomixertap_ReadWbFifoStatus(CHAL_HANDLE handle)
{

	cUInt16 reg=0, status=0;

	reg = BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTWBFIFO_UDF );
	if (reg != 0)
		status |= CHAL_AUDIO_FIFO_UDF;

	reg = BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTWBFIFO_OVF );
	if (reg != 0)
		status |= CHAL_AUDIO_FIFO_OVF;

	return status;
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadWbFifoEntryCnt
//
// Description:   Read narrow band MIXER TAP fifo entry count
//
//============================================================================
cUInt16 chal_audiomixertap_ReadWbFifoEntryCnt(CHAL_HANDLE handle)
{
	cUInt16 reg;
	reg = BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTWBFIFO_OVF );
	if (reg != 0)
		return 0x80; //if overflow, BTMIXER_WBENTRY would be all 0s.
	else
		return BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_BTMIXER_CFG2_R, BTMIXER_WBENTRY ); // addr is 0fba
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbFifoEntryCnt
//
// Description:   Read narrow band MIXER TAP fifo entry count
//
//============================================================================
cUInt16 chal_audiomixertap_ReadNbFifoEntryCnt(CHAL_HANDLE handle)
{
	return BRCM_READ_REG_FIELD( sMixerTap.base, DSP_AUDIO_VOICEFIFO_STATUS_R, BTNBFIFO_ENTRY ); // addr is 0fb8
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbFifoStereo
//
// Description:   Read from stereo FIFO in NB mixer tap
//
//============================================================================
cVoid chal_audiomixertap_ReadNbFifoStereo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		)
{
	cUInt16	i;
	cUInt16 *ptr = src;

	for( i=0; i<length; i+=2 )
	{
		*ptr++ = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_BTNBDINL_R );
		*ptr++ = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_BTNBDINR_R );
	}
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbFifoLeft
//
// Description:   Read data from left channel in NB mixer tap
//
//============================================================================
cVoid chal_audiomixertap_ReadNbFifoLeft(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		)
{
	cUInt16	i;
	cUInt16 *ptr = src;

	for(i=0; i<length; i++)
	{
		*ptr++ = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_BTNBDINL_R );
	}
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadNbFifoRight
//
// Description:   Read data from right channel in NB mixer tap
//
//============================================================================
cVoid chal_audiomixertap_ReadNbFifoRight(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		)
{
	cUInt16	i;
	cUInt16 *ptr = src;

	for(i=0; i<length; i++)
	{
		*ptr++ = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_BTNBDINR_R );
	}
}

//============================================================================
//
// Function Name: chal_audiomixertap_ReadWbFifo
//
// Description:   Read data from stereo channel in WB mixer tap
//
//============================================================================
cVoid chal_audiomixertap_ReadWbFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		)
{
	cUInt16	i;
	cUInt32 *ptr;
	//cUInt32 tmp32;

	ptr = src;
	if (ptr)
	{
		for(i=0; i<length; i++)
		{
			*ptr++ = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_PIFIFODATA1_R );
		}
	}
	else   //ptr is NULL.
	{
		for(i=0; i<length; i++)
		{
			//tmp32 = BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_PIFIFODATA1_R );
			BRCM_READ_REG( sMixerTap.base, DSP_AUDIO_PIFIFODATA1_R );
		}
	}
}

//============================================================================
//
// Function Name: chal_audiomixertap_getAFIFO_addr
//
// Description:   get Mixer Tap Wideband FIFO address
//
//============================================================================
cUInt32 chal_audiomixertap_getAFIFO_addr( CHAL_HANDLE	handle )
{
#define CHAL_AUDIOMIXERTAP_DMA_SRC_ADDR		(AHB_AUDIO_BASE_ADDR + DSP_AUDIO_PIFIFODATA1_R_OFFSET)

	return CHAL_AUDIOMIXERTAP_DMA_SRC_ADDR;
}
