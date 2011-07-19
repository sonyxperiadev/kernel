/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated, 
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between 
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license 
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all 
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU 
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
* ALL USE OF THE SOFTWARE.	
* 
* Except as expressly set forth in the Authorized License,
* 
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
* 
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO 
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE. 
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, 
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR 
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY 
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/
/**
*    @file   dspcmd.h
*    @brief  This file contains the interfaces to control the dsp.
**/
/****************************************************************************/
/**

*   @defgroup   DspCmdGroup   DSP APIs
*   @ingroup    AudioDrvGroup
*	@ingroup	DeviceDriverGroup
*
*   @brief      This group defines the interfaces to control the DSP
*/

#ifndef __DSPCMD_H__
#define __DSPCMD_H__

#include "mobcom_types.h"	// UInt8
#include "sharedmem.h"
#ifdef DSP_COMMS_INCLUDED
#include "dsp_feature_def.h"
#endif
//#include "speaker.h"


typedef enum
{
//audio command
	DSPCMD_TYPE_AUDIO_SET_INPUT,
	DSPCMD_TYPE_AUDIO_SET_OUTPUT,
	DSPCMD_TYPE_AUDIO_CONNECT_UL,
	DSPCMD_TYPE_AUDIO_CONNECT_DL,
	DSPCMD_TYPE_AUDIO_CONNECT,
	DSPCMD_TYPE_AUDIO_TONE_CTRL,
	DSPCMD_TYPE_AUDIO_TONE_STOP,
	DSPCMD_TYPE_AUDIO_ENABLE,
	DSPCMD_TYPE_EC_ON,
	DSPCMD_TYPE_NS_ON,
	DSPCMD_TYPE_CONFIG_EC_FAR_IN_FILTER,
	DSPCMD_TYPE_CONFIG_EC_GAIN,
	DSPCMD_TYPE_CONFIG_EC_PARMS1,
	DSPCMD_TYPE_CONFIG_EC_PARMS2,
	DSPCMD_TYPE_CONFIG_EC_STABLE_COEFF_THRESH,
	DSPCMD_TYPE_CONFIG_EC_MODE,
	DSPCMD_TYPE_CONFIG_NS_GAIN,
	DSPCMD_TYPE_HD_ON,
	DSPCMD_TYPE_AUDIO_SET_PCM,
	DSPCMD_TYPE_AUDIO_SWITCHT_CHNL,
	DSPCMD_TYPE_SET_VOICE_ADC,
	DSPCMD_TYPE_SET_VOICE_DAC,
	DSPCMD_TYPE_GENERIC_TONE,
	DSPCMD_TYPE_SET_VOCODER_INIT,
	DSPCMD_TYPE_SET_VOICE_DTX,
	DSPCMD_TYPE_MAIN_AMR_RUN,
	DSPCMD_TYPE_CONFIG_NLP_TIMEOUT,
	DSPCMD_TYPE_CONFIG_NLP_FILTER,
	DSPCMD_TYPE_CONFIG_COMPRESS_GAIN,
	DSPCMD_TYPE_CONFIG_EXP_ALPHA,
	DSPCMD_TYPE_CONFIG_EXP_BETA,
	DSPCMD_TYPE_CONFIG_EXPANDER,
	DSPCMD_TYPE_CONFIG_ECHO_DUAL,
	DSPCMD_TYPE_CONFIG_COMPANDER_FLAG,
	
	DSPCMD_TYPE_CONTROL_ECHO_CANCELLATION,
	DSPCMD_TYPE_EC_NS_ON,
	DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff,
	DSPCMD_TYPE_CHECK_DSP_AUDIO,
	DSPCMD_TYPE_SET_DSP_TONE_VOLUME,
	DSPCMD_TYPE_SET_DSP_AMR_VOLUME,  //to be removed
	DSPCMD_TYPE_SET_DSP_UL_GAIN,
	DSPCMD_TYPE_MUTE_DSP_UL,
	DSPCMD_TYPE_UNMUTE_DSP_UL,

//misc command
	DSPCMD_TYPE_INITIALIZE,
	DSPCMD_TYPE_POST_CMDQ,
	DSPCMD_TYPE_SET_PA_RAMP_SCALE,
	DSPCMD_TYPE_RIP_READ,
	DSPCMD_TYPE_RIP_READ_RESULT,
	DSPCMD_TYPE_RIP_WRITE,
	DSPCMD_TYPE_RIP_WRITE_PRAM,
	DSPCMD_TYPE_SET_CALIBRATION,
	DSPCMD_TYPE_SET_SCELLSLOT,
	DSPCMD_TYPE_MM_VPU_ENABLE,
	DSPCMD_TYPE_MM_VPU_DISABLE,
	DSPCMD_TYPE_SET_WAS_SPEECH_IDLE,
// add ARM2SP, BTNB Tap, USB dsp cmd 
	DSPCMD_TYPE_COMMAND_SET_ARM2SP,
	DSPCMD_TYPE_COMMAND_SET_BT_NB,
	DSPCMD_TYPE_COMMAND_USB_HEADSET,
	DSPCMD_TYPE_COMMAND_DIGITAL_SOUND,
	DSPCMD_TYPE_COMMAND_VOIF_CONTROL,
	DSPCMD_TYPE_COMMAND_VT_AMR_START_STOP,
	DSPCMD_TYPE_COMMAND_DSP_AUDIO_ALIGN,
	DSPCMD_TYPE_COMMAND_SP,
	DSPCMD_TYPE_DUAL_MIC_ON,
	DSPCMD_TYPE_COMMAND_SET_ARM2SP2,
	DSPCMD_TYPE_COMMAND_CLEAR_VOIPMODE
} DspCmdType_t;

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

#ifdef DSP_COMMS_INCLUDED
typedef struct
{
	UInt16	cell_id;
	UInt16 slot;
	Boolean dtx_enable;
	UInt8  /*BandSharedMem_t*/ bandSharedMem;
} DspCmdSetScellslot_t;
#endif//ifdef DSP_COMMS_INCLUDED

typedef void (*RIPReadCb_t)(UInt16 value);

typedef struct
{
	UInt16		address;
	RIPReadCb_t	read_cb;
} DspCmdRIPRead_t;

typedef struct
{
	UInt8		index;
	UInt16		value;
} DspCmdRIPReadResult_t;

typedef struct
{
	UInt8	paVolt;
	UInt8	paTemp;
} DspCmdSetPaRampScale_t;

typedef struct
{
	UInt16	address;
	UInt16	mask;
	UInt16	value;
} DspCmdRIPWrite_t;
typedef struct
{
	UInt16	address;
	UInt16	value;
} DspCmdRIPWritePRAM_t;

typedef void (*SetCalibrationCb_t)(void);

typedef struct
{
	UInt16	cacmp_shift;
	UInt32	casr;
	UInt32	ratio;
	SetCalibrationCb_t	callback;
} DspCmdSetCalibration_t;

typedef struct
{
	Boolean isPcsBand;
	UInt16	arfcn;
} DspCmdLockArfcn_t;

//audio start

typedef struct
{
	Boolean enable;
	UInt8	level;
	UInt8   channel;   //AudioChnl_t	channel;
} DspCmdAudioCtrlParm_t;

typedef struct
{
	Boolean uplink;
	Boolean  downlink;
} DspCmdAudioConnect_t;

typedef struct
{
	Boolean enable;
    UInt8 tone;
	UInt8 tone_dur;
} DspCmdAudioToneCtrl_t;

typedef struct
{
	UInt16 aud_mode;
	Boolean audioTuningMode;
} DspCmdECGain_t;

typedef struct
{
	UInt16 echo_u_step;
	UInt16 echo_DTD_hang;
	UInt16 echo_DTD_thresh;
} DspCmdECParms1_t;

typedef struct
{
	UInt16 echo_adapt_norm_factor;   
	UInt16 echo_en_near_scale_factor;
	UInt16 echo_en_far_scale_factor;
} DspCmdECParms2_t;


typedef struct
{
	UInt16 echo_no_coefs;
	UInt16 echo_m_factor;
	UInt16 echo_loop;    
	UInt16 echo_coupling_delay;
} DspCmdECMode_t;

typedef struct
{
	UInt16 echo_stable_coef_thresh1;
	UInt16 echo_stable_coef_thresh2;
} DspCmdECCoeff_t;

typedef struct
{
	UInt16	input_gain;
	UInt16	output_gain;
} DspCmdNSGain_t;

typedef struct
{
	Boolean		PCM_on_off;
} DspCmdSetAudioPCM_t;

typedef struct
{
	Boolean		isAuxChnl;
} DspCmdSwitchAudioChnl_t;

typedef struct
{
	Boolean	superimpose;
	UInt16	tone_duration;
	UInt16	tone_scale;
	UInt8	volume;
	UInt16	f1;
	UInt16	f2;
	UInt16	f3;
} DspCmdGenericTone_t;

typedef struct
{
	UInt16	mode;
	Boolean	amr_if2_enable;
	UInt16  mst_flag;
} DspCmdDSPAMRMode_t;

typedef struct{
	UInt16 compressor_output_gain;      
	UInt16 compressor_gain;    
}DspCmdCompressGain_t;

typedef struct{
	UInt16 expander_b;
	UInt16 expander_c;
	UInt16 expander_c_div_b;
	UInt16 expander_inv_b; 
}DspCmdExpander_t;

typedef struct{
   UInt16 echo_dual_ec_eclen;
   UInt16 echo_dual_ec_rinlpcbuffersize;
   UInt16 echo_dual_e_rincirbuffsizemodij;
   UInt16 echo_dual_ec_dt_th_erl_db;
   UInt16 echo_dual_echno_step_size_gain;
   UInt16 echo_dual_ec_hangover_cnt;
   UInt16 echo_dual_ec_vad_th_db;
   UInt16 echo_dual_dt_hangover_time; 
   } DspCmdEchoDual_t;

//audio end

typedef struct
{
	DspCmdType_t type;
	union
	{
		//audio
		UInt8					DspCmdAudioInputLevel;
		DspCmdAudioCtrlParm_t	DspCmdAudioCtrlParm;
		DspCmdAudioToneCtrl_t	DspCmdAudioToneCtrl;
		DspCmdAudioConnect_t	DspCmdAudioConnect;
		UInt8					DspCmdEchoDlfCngNlpOnOff;
		Boolean					DspCmdOnOff;
		Boolean					DspCmdAudioEnable;
		UInt16					DspCmdNlpTimeout;
		UInt16					DspCmdStableCoefThresh[2];
		UInt16					DspCmdFarInFilterCoefs[10];
		UInt16					DspCmdNlpFilterCoefs[10];
		DspCmdECGain_t			DspCmdECGain;
		DspCmdECParms1_t		DspCmdECParms1;
		DspCmdECParms2_t		DspCmdECParms2;
		DspCmdECMode_t			DspCmdECMode;
		DspCmdNSGain_t			DspCmdNSGain;
		DspCmdSetAudioPCM_t		DspCmdSetAudioPCM;
		DspCmdSwitchAudioChnl_t	DspCmdSwitchAudioChnl;
		DspCmdGenericTone_t		DspCmdGenericTone;
		UInt8					DspCmdVocoderInit;
		UInt8					DspCmdVoiceDtx;
#ifdef DSP_COMMS_INCLUDED
		DspCmdSetScellslot_t		DspCmdSetScellSlot;
#endif
		DspCmdDSPAMRMode_t		DspCmdDSPAMRMode;
		DspCmdCompressGain_t	DspCmdCompressGain;
		UInt16					DspCmdExpAlpha;
		UInt16					DspCmdExpBeta;
		DspCmdExpander_t		DspCmdExpander;
		DspCmdEchoDual_t		DspCmdEchodual;
		UInt16					DspCmdCompandflag;
		//misc
		CmdQ_t					DspCmdCmdQ;
		DspCmdSetPaRampScale_t	DspCmdSetPaRampScale;
		DspCmdRIPRead_t			DspCmdRIPRead;
		DspCmdRIPReadResult_t	DspCmdRIPReadResult;
		DspCmdRIPWrite_t		DspCmdRIPWrite;
		DspCmdRIPWritePRAM_t	DspCmdRIPWritePRAM;
		DspCmdSetCalibration_t	DspCmdSetCalibration;
		DspCmdLockArfcn_t		DspCmdLockArfcn;
		void*					DspCmdPointer;
		UInt8					DspCmdDmicAncNlpWnrOnOff;
	} cmd;
} DSPCMD_t;

void DSPCMD_Initialize(void);

void DSPCMD_PostCmdQ(CmdQ_t cmdQ);

#ifdef DSP_COMMS_INCLUDED
void DSPCMD_SetScellSlot(		
	UInt16	cell_id,
	UInt16 slot,
	Boolean dtx_enable,
	UInt8  /*BandSharedMem_t*/ bandSharedMem);
#endif

/** @addtogroup DspCmdGroup
	@{
*/

void DSPCMD_InitPaVoltTemp(			///< Initialize PA voltage and temperature
	UInt8 paVolt,					///< initial PA voltage
	UInt8 paTemp					///< initial PA temperature
	);

void DSPCMD_SetPaRampScale(			///< Set the power amp scale
	UInt8 paVolt,					///< Current PA voltage
	UInt8 paTemp					///< Current PA temperature
	);

void DSPCMD_RIPRead(				///< Read data from RIP
	UInt16		address,			///< Address to Read
	RIPReadCb_t	read_cb				///< Callback to return results
	);

void DSPCMD_RIPReadResult(			///< Read Data From RIP
	UInt8	index,					///< index		
	UInt16	value					///< value
	);

void DSPCMD_RIPWrite(				///< Write Data to RIP
	UInt16	address,				///< Address to read
	UInt16	value					///< Value to write
	);

void DSPCMD_RIPWriteBitwise(		///< Write Data to RIP
	UInt16	address,				///< Address to read
	UInt16	mask,					///< bit to overwrite
	UInt16	value					///< Value to write
	);

void DSPCMD_RIPWritePRAM(			///< Write Data to RIP
	UInt16	address,				///< Address to read
	UInt16	value					///< Value to write
	);

/** @} */

//audio
void DSPCMD_AudioSetInput(
	UInt8	level
	);

void DSPCMD_AudioSetOutput(
	UInt8 volume,
	UInt8 channel		//AudioChnl_t	channel
	);

void DSPCMD_AudioConnectUl(
	Boolean enable,
	UInt8	level
	);

void DSPCMD_AudioConnectDl(
	Boolean enable,
	UInt8	level,
	UInt8 channel		//AudioChnl_t	channel
	);
void DSPCMD_AudioEnable( Boolean enable	);
void DSPCMD_AudioToneCtrl(
	Boolean enable,
    UInt8 tone,
	UInt8 tone_dur
	);

void DSPCMD_ConfigCompressGain(
	UInt16 compressor_output_gain,      
	UInt16 compressor_gain     
	);
void DSPCMD_ConfigExpAlpha(
	UInt16 expander_alpha      
	);
void DSPCMD_ConfigExpBeta(
	UInt16 expander_beta      
	);

void DSPCMD_ConfigExpander(
     DspCmdExpander_t *exp_parm 
	);
 
void DSPCMD_ConfigEchoDual(
     DspCmdEchoDual_t *echo_dual_parm    
	);

void DSPCMD_ConfigCompanderflag(
    UInt16 compander_flag    
	);

void DSPCMD_SetPCMOnOff(
	Boolean	on_off
	);

/** @addtogroup DspCmdGroup
	@{
*/

void DSPCMD_ECOn(						///< Enable/Disable Echo Cancellor
	Boolean	ec_on,						///< TRUE = Enable Echo Cancellor
	Boolean	dual_flt_on,				///< TRUE = Enable Dual Filter mode
	Boolean	cng_on,						///< TRUE = Enable Comfort noise gen.
	Boolean	nlp_on						///< TRUE = Enable NLP
	);

Boolean DSPCMD_IsEchoOn(	void );		///< Query Echo Canceller Status

void DSPCMD_ConfigECFarInFilter(		///< Configure EC Far In filter Coefficients
	UInt16	*coefs						///< Pointer to 10 coefficients
	);

void DSPCMD_ConfigEC(
	UInt16	aud_mode,
	Boolean audioTuningMode);		///< Configure EC  settings

/** @} */
	
void DSPCMD_ConfigECStableThresholds(
 UInt16 stable_coef_thresh_1,
 UInt16 stable_coef_thresh_2
); 
void DSPCMD_ConfigEC_NLP_Timeout(
 UInt16 timeout_val
);

void DSPCMD_ConfigEC_NLP_Filter( 
 UInt16 *coefs
);


/** @addtogroup DspCmdGroup
	@{
*/

void DSPCMD_NSOn(					///< Enable/Disable Noise Suppression
	Boolean	On						///< TRUE = Enable Noise Suppression
	);

Boolean DSPCMD_IsNSOn( void );		///< Query noise suppression status

void DSPCMD_ConfigNSGain(			///< Configure NS gains
	UInt16 input_gain,				///< input gain
	UInt16 output_gain				///< output gain
	);

void DSPCMD_DualMicOn(					// Enable/Disable Dual Mic
	Boolean	dualmic_on,					// TRUE = Enable Dual Mic software
	Boolean	dualmic_anc_on,				// TRUE = Enable Dual Mic Anc
	Boolean	dualmic_nlp_on,				// TRUE = Enable Dual Mic NLP
	Boolean	dualmic_pre_wnr_on,			// TRUE = Enable Dual Mic Pre-WNR
	Boolean	dualmic_post_wnr_on			// TRUE = Enable Dual Mic Post-WNR
	);

/** @} */

void DSPCMD_SpeakerPhone(
	Boolean	On
	);

void DSPCMD_SwitchAudioChnl(
	Boolean	isAuxChnl
	);

/** @addtogroup DspCmdGroup
	@{
*/

void DSPCMD_SetCalibration(				///< Load the sleep parameters to RIP
	UInt16 cacmp_shift,					///< # of bits to shift CASR for SFGIR, SFGFR
	UInt32 casr,						///< Slow clock value
	UInt32 ratio,						///< CAFR/CASR ratio
	SetCalibrationCb_t callback			///< point to SetCalibration_CB()
	);

void DSPCMD_LockArfcn(					///< Lock to specified band/arfcn
	Boolean isPcsBand,					///< TRUE: PCS, FALSE otherwise
	UInt16 arfcn						///< Target arfcn
	);

void DSPCMD_StartGenericTone(
	Boolean	superimpose,				///< Superimpose tone over speech
	UInt16	tone_duration,				///< Tone duration in MS
	UInt16	tone_scale,					///< Tone scaling
	UInt8	volume,						///< Tone volume
	UInt16	f1,							///< Frequency 1
	UInt16	f2,							///< Frequency 2
	UInt16	f3							///< Frequency 3
	);

void DSPCMD_SetVoiceDAC(UInt16* voice_dac);	///< update downlink voice filter
void DSPCMD_SetVoiceADC(UInt16* voice_adc);	///< update uplink voice filter

/** @} */

void DSPCMD_SetVocoderInit( UInt8 init);
void DSPCMD_SetVoiceDtx( UInt8 dtx);

void DSPCMD_SetDSP_AMR_RUN(
	VP_Mode_AMR_t mode,
	Boolean amr_if2_enable,
	UInt16 mst_flag
	);

void ProcessDspCmd(
	DSPCMD_t	*pDspCmd
	);

void DSPPROC_EGPRSModeFilt(UInt8 MCS_mode);     
void DSPPROC_GPRSModeFilt(RxCode_t coding_scheme);
void DSPPROC_ResetPacketModeFilt(void); 
void DSPPROC_SetSoftSymbolScaling(void);
void DSPPROC_Set_SAIC_adapt(Boolean flag);         

#endif

#endif //#ifndef __DSPCMD_H__

