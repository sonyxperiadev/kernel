/*****************************************************************************
*
*    (c) 2007-2008 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/

// following signals are send inside HISR
// Define NUCLEUS_HISR so that signals that are transmitted are
// transmitted with the NO_SUSPEND flag set.  This needs to be done
// because all functions in this unit are called from the RIP HISR.

#define NUCLEUS_HISR

#include "assert.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "ostypes.h"
#include "osheap.h"		// OSHEAP_Alloc
#include "dsp_feature_def.h"
#include "mti_trace.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "sysparm.h"	
#include "dspcore_drv.h"
#include "wdtimer_drv.h"
#include "log.h"

static UInt8 S_paVolt;
static UInt8 S_paTemp;

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_Initialize
//
//! Description:	
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_Initialize(void)
{
	DSPCMD_t	dspCmd;

	dspCmd.type = DSPCMD_TYPE_INITIALIZE;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}


/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_PostCmdQ
//
//! Description:		Send a command to DSP via command queue
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_PostCmdQ(CmdQ_t cmdQ)
{
	DSPCMD_t	dspCmd;

	memcpy(&(dspCmd.cmd.DspCmdCmdQ), &cmdQ, sizeof(CmdQ_t));
	dspCmd.type = DSPCMD_TYPE_POST_CMDQ;
	 
	DSPCore_PostCmd(&dspCmd);
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_InitPaVoltTemp
//
//! Description:		Initialize the static power amplifier voltage and temperature
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_InitPaVoltTemp(			// Initialize PA voltage and temperature
	UInt8 paVolt,					// initial PA voltage
	UInt8 paTemp					// initial PA temperature
	)
{
	S_paVolt = paVolt;
	S_paTemp = paTemp;
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetPaRampScale
//
//! Description:		Send DSP command to set the shared memory power amplifier
//					ramp scale.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_SetPaRampScale(			// Set the power amp scale
	UInt8 paVolt,					// Current PA voltage
	UInt8 paTemp					// Current PA temperature
	)
{
	// L1_LOGV("DSPCMD_SetPaRampScale, paVolt/paTemp/S_paVolt/S_paTemp: ", (paVolt<<24) | (paTemp<<16) | (S_paVolt<<8) | S_paTemp );

	if ( paVolt != S_paVolt || paTemp != S_paTemp )
	{
		DSPCMD_t	dspCmd;

		(dspCmd.cmd.DspCmdSetPaRampScale).paVolt = paVolt;
		(dspCmd.cmd.DspCmdSetPaRampScale).paTemp = paTemp;

		// SYY 120502, according to Matthew, no error handling is required in sending
		//	a message.  I would assume ALLOC_PRIM will assert, if it runs out of the
		//	pool memory, and target queue full will never happen!
 
		 
		dspCmd.type = DSPCMD_TYPE_SET_PA_RAMP_SCALE;
		DSPCore_PostCmd(&dspCmd);
 	 

		S_paVolt = paVolt;
		S_paTemp = paTemp;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_RIPRead
//
//! Description:		Send DSP command to read data from a RIP register or
//					memory location.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_RIPRead(				// Read Data From RIP
	UInt16		address,			// Address to Read
	RIPReadCb_t	read_cb				// Callback to return results
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdRIPRead.address = address;
	dspCmd.cmd.DspCmdRIPRead.read_cb = read_cb;
	dspCmd.type = DSPCMD_TYPE_RIP_READ;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_RIPReadResult
//
//! Description:		Send DSP command to read data from a RIP register or
//					memory location.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_RIPReadResult(				// Read Data From RIP
	UInt8	index,		
	UInt16	value
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdRIPReadResult.index = index;
	dspCmd.cmd.DspCmdRIPReadResult.value = value;
	dspCmd.type = DSPCMD_TYPE_RIP_READ_RESULT;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_RIPWrite
//
//! Description:		Send DSP command to write data to a RIP register or
//					memory location.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_RIPWrite(				// Write Data to RIP
	UInt16	address,				// Address to read
	UInt16	value					// Value to write
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdRIPWrite.address = address;
	dspCmd.cmd.DspCmdRIPWrite.mask = 0xFFFF;
	dspCmd.cmd.DspCmdRIPWrite.value = value;
	dspCmd.type = DSPCMD_TYPE_RIP_WRITE;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_RIPWriteBitwise
//
//! Description:		Send DSP command to write data to a RIP register or
//					memory location.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_RIPWriteBitwise(				// Write Data to RIP
	UInt16	address,				// Address to read
	UInt16	mask,					// bit to overwrite
	UInt16	value					// Value to write
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdRIPWrite.address = address;
	dspCmd.cmd.DspCmdRIPWrite.mask = mask;
	dspCmd.cmd.DspCmdRIPWrite.value = value;
	dspCmd.type = DSPCMD_TYPE_RIP_WRITE;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_RIPWritePRAM
//
//! Description:		Send DSP command to write data to a RIP register or
//					memory location.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_RIPWritePRAM(				// Write Data to RIP
	UInt16	address,				// Address to read
	UInt16	value					// Value to write
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdRIPWritePRAM.address = address;
	dspCmd.cmd.DspCmdRIPWritePRAM.value = value;
	dspCmd.type = DSPCMD_TYPE_RIP_WRITE_PRAM;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioSetInput
//
//! Description:		Send DSP command to set audio input level
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioSetInput(
	UInt8	level
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdAudioInputLevel = level;

//	L1_LOGV("Erol-DspCmdEchoDlfCngNlpOnOff: ", dspCmd.cmd.DspCmdEchoDlfCngNlpOnOff );

 
	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_SET_INPUT;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioSetOutput
//
//! Description:		Send DSP command to set audio output level
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioSetOutput(
	UInt8 level,
	UInt8	channel                 //	AudioChnl_t	channel
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdAudioCtrlParm.level = level;
	dspCmd.cmd.DspCmdAudioCtrlParm.channel = channel;

 
	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_SET_OUTPUT;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioConnectUl
//
//! Description:		Send DSP command to connect Audio uplink
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioConnectUl(
	Boolean enable,
	UInt8	level
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdAudioCtrlParm.enable = enable;
	dspCmd.cmd.DspCmdAudioCtrlParm.level = level;

 
	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_CONNECT_UL;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioConnectDl
//
//! Description:		Send DSP command to connect Audio downlink
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioConnectDl(
	Boolean enable,
	UInt8	level,
	UInt8	channel                 //	AudioChnl_t	channel
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdAudioCtrlParm.enable = enable;
	dspCmd.cmd.DspCmdAudioCtrlParm.level = level;
	dspCmd.cmd.DspCmdAudioCtrlParm.channel = channel;

 	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_CONNECT_DL;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioEnable
//
//! Description:		Send DSP command to enable Audio
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioEnable( Boolean enable	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdAudioEnable = enable;
 	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_ENABLE;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioConnect
//
//! Description:		Send DSP command to Connect Audio
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioConnect( Boolean uplink, Boolean downlink	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdAudioConnect.uplink = uplink;
	dspCmd.cmd.DspCmdAudioConnect.downlink = downlink;
 	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_CONNECT;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_AudioToneCtrl
//
//! Description:		Send DSP command to control Audio Tone
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_AudioToneCtrl(
	Boolean enable,
    UInt8 tone,
	UInt8 tone_dur
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdAudioToneCtrl.enable = enable;
	dspCmd.cmd.DspCmdAudioToneCtrl.tone = tone;
	dspCmd.cmd.DspCmdAudioToneCtrl.tone_dur = tone_dur;

 	 
	dspCmd.type = DSPCMD_TYPE_AUDIO_TONE_CTRL;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}


/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ECOn
//
//! Description:		Send DSP command to enable/disable echo cancellor
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ECOn(					// Enable/Disable Echo Cancellor
	Boolean	ec_on,						// TRUE = Enable Echo Cancellor
	Boolean	dual_flt_on,				// TRUE = Enable Dual Filter mode
	Boolean	cng_on,						// TRUE = Enable Comfort noise gen.
	Boolean	nlp_on						// TRUE = Enable NLP
	)
{
	DSPCMD_t	dspCmd;
	
	dspCmd.cmd.DspCmdEchoDlfCngNlpOnOff = ((ec_on ? 1:0)<<4) + ((dual_flt_on ? 1:0)<<2) + ((cng_on ? 1:0)<<1) + (nlp_on ? 1:0);
	//	L1_LOGV("Erol-DspCmdEchoDlfCngNlpOnOff: ", dspCmd.cmd.DspCmdEchoDlfCngNlpOnOff );
	dspCmd.type = DSPCMD_TYPE_EC_ON;
	DSPCore_PostCmd(&dspCmd);
 	 
}



/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigECFarInFilter
//
//! Description:		Send DSP command to configure EC Far In filter coefficients.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigECFarInFilter(		// Configure EC Far In filter Coefficients
	UInt16	*coefs					// Pointer to 10 coefficients
	)
{
	DSPCMD_t	dspCmd;
	UInt8		i;

	 

	for ( i = 0; i < 10; i++ )
	{
		dspCmd.cmd.DspCmdFarInFilterCoefs[i] = coefs[i];
	}

 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EC_FAR_IN_FILTER;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigEC
//
//! Description:		Configure Echo canceller gains.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_ConfigEC(			// Configure EC gain settings
		UInt16	aud_mode,
		Boolean audioTuningMode
)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdECGain.aud_mode = aud_mode;    		 
	dspCmd.cmd.DspCmdECGain.audioTuningMode = audioTuningMode;    		 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EC_GAIN;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigStableCoeffThresh
//
//! Description:		Configure Echo Canceller stable coefficients thresholds
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigECStableThresholds(
 UInt16 stable_coef_thresh_1,
 UInt16 stable_coef_thresh_2
 )
{
	DSPCMD_t	dspCmd;

//	L1_LOGV("Erol-DSPCMD_ConfigECStableThresholds_1: ", stable_coef_thresh_1 );
//	L1_LOGV("Erol-DSPCMD_ConfigECStableThresholds_2: ", stable_coef_thresh_2 );
	 

	dspCmd.cmd.DspCmdStableCoefThresh[0]	= stable_coef_thresh_1;
	dspCmd.cmd.DspCmdStableCoefThresh[1]	= stable_coef_thresh_2;

 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EC_STABLE_COEFF_THRESH;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigEC_NLP_Timeout
//
//! Description:		Configure NLP Timeout parameter
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigEC_NLP_Timeout(  // Configure EC NLP timeout
 UInt16 timeout_val)     // value default: 8 
{
	DSPCMD_t	dspCmd;

//	L1_LOGV("Erol-DSPCMD_ConfigEC_NLP_Timeout: ", timeout_val );

	 

	dspCmd.cmd.DspCmdNlpTimeout	= timeout_val;
 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_NLP_TIMEOUT;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigEC_NLP_Filter
//
//! Description:		Configure NLP Filter coefficients
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigEC_NLP_Filter( UInt16 *coefs)
{
	DSPCMD_t	dspCmd;
	UInt8		i;

	 

	for ( i = 0; i < 10; i++ )
	{
//		L1_LOGV("Erol-DSPCMD_ConfigEC_NLP_Filter: ", coefs[i] );
		dspCmd.cmd.DspCmdNlpFilterCoefs[i] = coefs[i];
	}

 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_NLP_FILTER;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_NSOn
//
//! Description:		Send DSP command to enable/disable noise suppresssion.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_NSOn(					// Enable/Disable Echo Cancellor
	Boolean	on						// TRUE = Enable Echo Cancellor
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdOnOff = on ? 1 : 0;
	dspCmd.type = DSPCMD_TYPE_NS_ON;
	DSPCore_PostCmd(&dspCmd);
 	 
}



/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_DualMicOn
//
//! Description:		Send DSP command to enable/disable dual mic
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_DualMicOn(					// Enable/Disable Dual Mic
	Boolean	dualmic_on,					// TRUE = Enable Dual Mic software
	Boolean	dualmic_anc_on,				// TRUE = Enable Dual Mic Anc
	Boolean	dualmic_nlp_on,				// TRUE = Enable Dual Mic NLP
	Boolean	dualmic_pre_wnr_on,			// TRUE = Enable Dual Mic Pre-WNR
	Boolean	dualmic_post_wnr_on			// TRUE = Enable Dual Mic Post-WNR
	)
{
	DSPCMD_t	dspCmd;
	
	dspCmd.cmd.DspCmdDmicAncNlpWnrOnOff = ((dualmic_on ? 1:0)<<4) + ((dualmic_post_wnr_on ? 1:0)<<3)+ ((dualmic_pre_wnr_on ? 1:0)<<2) + ((dualmic_nlp_on ? 1:0)<<1) + (dualmic_anc_on ? 1:0);
 	dspCmd.type = DSPCMD_TYPE_DUAL_MIC_ON;
	DSPCore_PostCmd(&dspCmd);
 	 
}



/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigNSGain
//
//! Description:		Send DSP command to configure noise suppresssion gains.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigNSGain(			// Configure NS gains
	UInt16 input_gain,
	UInt16 output_gain
	)
{
	DSPCMD_t	dspCmd;

	 

	dspCmd.cmd.DspCmdNSGain.input_gain = input_gain;
	dspCmd.cmd.DspCmdNSGain.output_gain = output_gain;

 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_NS_GAIN;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}


///////////////////////////////////////////////////////////////////////////////
//! Function Name:	DSPCMD_SwitchAudioChnl
//!
//! Description:		Switch audio between aux and standard channels.
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SwitchAudioChnl(
	Boolean	isAuxChnl
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdSwitchAudioChnl.isAuxChnl = isAuxChnl;

	dspCmd.type = DSPCMD_TYPE_AUDIO_SWITCHT_CHNL;
	DSPCore_PostCmd(&dspCmd);
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetCalibration
//
//! Description:		Load the sleep parameters to RIP
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_SetCalibration(				// Load the sleep parameters
	UInt16 cacmp_shift,					// # of bits to shift CASR for SFGIR, SFGFR
	UInt32 casr,						// Slow clock value
	UInt32 ratio,						// CAFR/CASR ratio
	SetCalibrationCb_t callback			// SetCalibration callback (enable deep sleep)
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdSetCalibration.cacmp_shift = cacmp_shift;
	dspCmd.cmd.DspCmdSetCalibration.casr = casr;
	dspCmd.cmd.DspCmdSetCalibration.ratio = ratio;
	dspCmd.cmd.DspCmdSetCalibration.callback = callback;
	dspCmd.type = DSPCMD_TYPE_SET_CALIBRATION;
	 
	DSPCore_PostCmd(&dspCmd); 	 
}

/////////////////////////////////////////////////////////////////////////////
//						  
//! Function Name:	DSPCMD_StartGenericTone
//
//! Description:		Request L1 to generate a generic tone.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////


void DSPCMD_StartGenericTone(
	Boolean	superimpose,			// Superimpose tone over speech
	UInt16	tone_duration,			// Tone duration in MS
	UInt16	tone_scale,				// Tone scaling
	UInt8	volume,					// Tone volume
	UInt16	f1,						// Frequency 1
	UInt16	f2,						// Frequency 2
	UInt16	f3						// Frequency 3
	)
{
	DSPCMD_t	dspCmd;

	(dspCmd.cmd.DspCmdGenericTone).superimpose = superimpose;
	(dspCmd.cmd.DspCmdGenericTone).tone_duration = tone_duration;
	(dspCmd.cmd.DspCmdGenericTone).tone_scale = tone_scale;
	(dspCmd.cmd.DspCmdGenericTone).volume = volume;
	(dspCmd.cmd.DspCmdGenericTone).f1 = f1;
	(dspCmd.cmd.DspCmdGenericTone).f2 = f2;
	(dspCmd.cmd.DspCmdGenericTone).f3 = f3;

	dspCmd.type = DSPCMD_TYPE_GENERIC_TONE;

	DSPCore_PostCmd(&dspCmd);
}


/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetVoiceADC
//
//! Description:		Send DSP command to update VOICE_ADC parameters.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SetVoiceADC(UInt16* voice_adc)
{
	DSPCMD_t	dspCmd;
	UInt16*		pFilterParam;
	short		i;

	pFilterParam = (UInt16 *)OSHEAP_Alloc(sizeof(UInt16)*NUM_OF_VOICE_COEFF);

	for(i=0; i<NUM_OF_VOICE_COEFF; i++)
	{
		pFilterParam[i] = voice_adc[i];
	}

	dspCmd.type = DSPCMD_TYPE_SET_VOICE_ADC;
	dspCmd.cmd.DspCmdPointer = (void *)pFilterParam;
	DSPCore_PostCmd(&dspCmd); 	 

}



/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetVoiceDAC
//
//! Description:		Send DSP command to update VOICE_DAC parameters.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SetVoiceDAC(UInt16* voice_dac)
{
	DSPCMD_t	dspCmd;
	UInt16*		pFilterParam;
	short		i;

	pFilterParam = (UInt16 *)OSHEAP_Alloc(sizeof(UInt16)*NUM_OF_VOICE_COEFF);

	for(i=0; i<NUM_OF_VOICE_COEFF; i++)
	{
		pFilterParam[i] = voice_dac[i];
	}

	dspCmd.type = DSPCMD_TYPE_SET_VOICE_DAC;
	dspCmd.cmd.DspCmdPointer = (void *)pFilterParam;
	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetVocoderInit
//
//! Description:		
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SetVocoderInit( UInt8 init)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdVocoderInit = init;
	dspCmd.type = DSPCMD_TYPE_SET_VOCODER_INIT;
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetVocoderInit
//
//! Description:		
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SetVoiceDtx( UInt8 dtx)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdVoiceDtx = dtx;
	dspCmd.type = DSPCMD_TYPE_SET_VOICE_DTX;
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetScellSlot
//
//! Description:		Set SCell Slot.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

#ifdef DSP_COMMS_INCLUDED
void DSPCMD_SetScellSlot(		
	UInt16	cell_id,
	UInt16 slot,
	Boolean dtx_enable,
	UInt8  /*BandSharedMem_t*/ bandSharedMem)

{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdSetScellSlot.cell_id    = cell_id;    		 
	dspCmd.cmd.DspCmdSetScellSlot.slot     	= slot;     		
	dspCmd.cmd.DspCmdSetScellSlot.dtx_enable = dtx_enable;	
	dspCmd.cmd.DspCmdSetScellSlot.bandSharedMem	= bandSharedMem;  		
	dspCmd.type = DSPCMD_TYPE_SET_SCELLSLOT;
	 
	DSPCore_PostCmd(&dspCmd);
 	
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_SetDSP_AMR_RUN
//
//! Description:		Send DSP command to start main AMR codec function.
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_SetDSP_AMR_RUN(
	VP_Mode_AMR_t mode,
	Boolean amr_if2_enable,
	UInt16  mst_flag
 
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdDSPAMRMode.mode = mode;
	dspCmd.cmd.DspCmdDSPAMRMode.amr_if2_enable = amr_if2_enable;
	dspCmd.cmd.DspCmdDSPAMRMode.mst_flag = mst_flag;
	dspCmd.type = DSPCMD_TYPE_MAIN_AMR_RUN;

	DSPCore_PostCmd(&dspCmd);
 	 
}

/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigCompressGain
//
//! Description:		
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigCompressGain(
	UInt16 compressor_output_gain,      
	UInt16 compressor_gain     
	)
 {
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdCompressGain.compressor_output_gain = compressor_output_gain;
	dspCmd.cmd.DspCmdCompressGain.compressor_gain = compressor_gain;
	
 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_COMPRESS_GAIN;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigExpAlpha
//
//! Description:		
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_ConfigExpAlpha(
	UInt16 expander_alpha      
	)
  {
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdExpAlpha = expander_alpha;
	
 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EXP_ALPHA;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
   }
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigExpBeta
//
//! Description:	
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_ConfigExpBeta(
	UInt16 expander_beta      
	)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdExpBeta = expander_beta;
	
 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EXP_BETA;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigExpander
//
//! Description:		
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

 void DSPCMD_ConfigExpander(
     DspCmdExpander_t *exp_parm 
	)
  {
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdExpander.expander_b = exp_parm->expander_b;
	dspCmd.cmd.DspCmdExpander.expander_c = exp_parm->expander_c;
	dspCmd.cmd.DspCmdExpander.expander_c_div_b = exp_parm->expander_c_div_b;
	dspCmd.cmd.DspCmdExpander.expander_inv_b = exp_parm->expander_inv_b;
	
 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_EXPANDER;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}
/////////////////////////////////////////////////////////////////////////////
//
//! Function Name:	DSPCMD_ConfigEchoDual
//
//! Description:		
//
//! Notes:
//
/////////////////////////////////////////////////////////////////////////////

void DSPCMD_ConfigEchoDual(
     DspCmdEchoDual_t *echo_dual_parm    
	)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdEchodual.echo_dual_dt_hangover_time = echo_dual_parm->echo_dual_dt_hangover_time;
	dspCmd.cmd.DspCmdEchodual.echo_dual_ec_dt_th_erl_db	= echo_dual_parm->echo_dual_ec_dt_th_erl_db;
	dspCmd.cmd.DspCmdEchodual.echo_dual_e_rincirbuffsizemodij= echo_dual_parm->echo_dual_e_rincirbuffsizemodij;
	dspCmd.cmd.DspCmdEchodual.echo_dual_ec_hangover_cnt = echo_dual_parm->echo_dual_ec_hangover_cnt;
	dspCmd.cmd.DspCmdEchodual.echo_dual_ec_rinlpcbuffersize = echo_dual_parm->echo_dual_ec_rinlpcbuffersize;
	dspCmd.cmd.DspCmdEchodual.echo_dual_ec_vad_th_db = echo_dual_parm->echo_dual_ec_vad_th_db;
	dspCmd.cmd.DspCmdEchodual.echo_dual_echno_step_size_gain = echo_dual_parm->echo_dual_echno_step_size_gain;
	dspCmd.cmd.DspCmdEchodual.echo_dual_ec_eclen = echo_dual_parm->echo_dual_ec_eclen;

 
	 
	dspCmd.type = DSPCMD_TYPE_CONFIG_ECHO_DUAL;
	 
	DSPCore_PostCmd(&dspCmd);
 	  
}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_ConfigCompanderflag
//!
//! Description:		
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_ConfigCompanderflag(
    UInt16 compander_flag    
	)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdCompandflag = compander_flag;
	dspCmd.type = DSPCMD_TYPE_CONFIG_COMPANDER_FLAG;
	 
	DSPCore_PostCmd(&dspCmd);
 	 
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_SetPCMOnOff
//!
//! Description:		
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_SetPCMOnOff(Boolean	on_off)
{
	DSPCMD_t	dspCmd;

	dspCmd.cmd.DspCmdOnOff = on_off ? 1 : 0;
	dspCmd.type = DSPCMD_TYPE_AUDIO_SET_PCM;

	DSPCore_PostCmd(&dspCmd);
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_UpdateBlueToothFilterEnableFlag
//!
//! Description:	To call UpdateBlueToothFilterEnableFlag() in sysparm.c
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_UpdateBlueToothFilterEnableFlag(void)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdPointer = (void*) NULL;
	dspCmd.type = DSPCMD_TYPE_UPDATE_BLUETOOTH_FILTER_ENABLE_FLAG;
	 
	DSPCore_PostCmd(&dspCmd);

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_ConfigBlueToothFilter
//!
//! Description:	To call ConfigBlueToothFilter() in sysparm.c
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_ConfigBlueToothFilter(void)
{
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdPointer = (void*) NULL;
	dspCmd.type = DSPCMD_TYPE_CONFIG_BLUETOOTH_FILTER;
	 
	DSPCore_PostCmd(&dspCmd);

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_UpdateCompBiquadGain
//!
//! Description:	To call UpdateCompBiquadGain() in sysparm.c
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_UpdateCompBiquadGain(void)
{
#if (defined(_BCM2133_) && CHIP_REVISION >= 14) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || (defined(_BCM2152_) && CHIP_REVISION >= 13) || defined(_BCM213x1_) || defined(_BCM2153_) /*|| defined(_BCM21551_) - Temp. fix! Do not back integrate to common */
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdPointer = (void*) NULL;
	dspCmd.type = DSPCMD_TYPE_UPDATE_COMP_BIQUAD_GAIN;
	 
	DSPCore_PostCmd(&dspCmd);

#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPCMD_UpdateAndConfigCompFilter
//!
//! Description:	To call UpdateAndConfigCompFilter() in sysparm.c
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void DSPCMD_UpdateAndConfigCompFilter(void)
{
#if (defined(_BCM2133_) && CHIP_REVISION >= 14) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || (defined(_BCM2152_) && CHIP_REVISION >= 13) || defined(_BCM213x1_) || defined(_BCM2153_) /*|| defined(_BCM21551_) - Temp. fix! Do not back integrate to common */
	DSPCMD_t	dspCmd;
	dspCmd.cmd.DspCmdPointer = (void*) NULL;
	dspCmd.type = DSPCMD_TYPE_UPDATE_AND_CONFIG_COMP_FILTER;
	 
	DSPCore_PostCmd(&dspCmd);

#endif
}
