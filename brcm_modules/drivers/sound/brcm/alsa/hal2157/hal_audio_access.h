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
*    @file   hal_audio_core.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
****************************************************************************/


/**
 * @addtogroup AudioDrvGroup
 * @{
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <mach/reg_sys.h>
#include "audio_ipc_consts.h"
#include "shared.h"
#include "memmap.h"
#include <linux/broadcom/bcm_kril_Interface.h>
#include <linux/broadcom/bcm_fuse_sysparm.h>
#include "hal_audio_config.h"
#include "hal_audio.h"
//#include "consts.h"
#include "audio_consts.h"
#ifndef _HAL_AUDIO_ACCESS_H_
#define _HAL_AUDIO_ACCESS_H_

#define TRUE 1
#define FALSE 0

#define ANALOG_RAMP_UP   0x0087
#define ANALOG_RAMP_DOWN 0x0007

#define ANALOG_HS_RAMP_UP   0x0081
#define ANALOG_HS_RAMP_DOWN 0x0001
#define ANALOG_RAMP_PD		0x4000
#define ANALOG_RAMP_NO		0x8080
#define ANALOG_RAMP_DEFAULT 0x0000

#define AUDIO_ON	TRUE
#define AUDIO_OFF	FALSE

#define AUDIO_PATH_DEVICE_ON	TRUE
#define AUDIO_PATH_DEVICE_OFF	FALSE

//used to log audio control sequence for asic/hw. enable logid_audio for  detailed debug (default off)
#ifndef	WIN32
#define AUD_TRACE_DETAIL(...) Log_DebugPrintf(LOGID_AUDIO, __VA_ARGS__)
#else
#define AUD_TRACE_DETAIL(x)		Log_DebugPrintf		///< In Win32, we do not use it.
#endif

//
// Audio_HAL_s is the top of the structure of the Audio HAL.
// Through it, you can access all the Audio HW codec and DSP.
//

#if defined(SS_2153)
#define REF_SYSPARM	0xFFFF
typedef enum {
	CUSTOM_SPK_PGA,
	CUSTOM_MIC_PGA,
	CUSTOM_MIXER_GAIN,
	CUSTOM_GAIN_TOTAL
} CUSTOM_GAIN_t;
#endif // SS_2153



typedef struct Audio_Path_ADC_HQ_s
{
	UInt32	inputMic; // refer to MICSelection_en_t defined in HAL_Audio_core.h
	UInt32	channels; // 1 = mono, 2 = stereo 
	UInt8	pathState; // path enabled/disabled
	UInt32	UserCount;
} Audio_Path_ADC_HQ_s;


typedef struct Virtual_Channel_Dscr_s
{
	UInt8	state;
	UInt32	userCount;
	UInt32	hDev;
} Virtual_Channel_Dscr_s;


typedef struct Audio_Ext_Device_s
{
	UInt8	state; // on or off
	UInt8	swFifoState; //on or off
	UInt32	userCount;
	UInt32	userCountSwFifo;
} Audio_Ext_Device_s;

typedef struct Audio_HAL_Access_s
{
	// Audio DAC processing
	/* enable it later
	DAC_Audio_Path_s; 
	DAC_Poly_Path_s;
	DAC_Voice_Path_s;
	DAC_Mixer_s;
	DAC_Gain_s;
	*/

	// Audio ADC processing
	/* enable it later
	ADC_Loopback_Path_s;
	ADC_Voice_Path_s;
	*/
	struct Audio_Path_ADC_HQ_s	AudioPath_AdcHq;

	// Linked List of Virtual channel for data streaming in/out
	struct Virtual_Channel_Dscr_s	VirtualChannel_PlayFmI2s;
	struct Virtual_Channel_Dscr_s	VirtualChannel_RecordFmI2s_PCM;
	struct Virtual_Channel_Dscr_s	VirtualChannel_RecordFmI2s_AAC_DSP_In;
	struct Virtual_Channel_Dscr_s	VirtualChannel_RecordFmI2s_AAC_DSP_Out;
        struct Virtual_Channel_Dscr_s	VC_Usb; 


	// exteranl device(use a linked list or list one by one?)
	struct Audio_Ext_Device_s	ExtDev_FmI2s;
        struct Audio_Ext_Device_s	ExtDev_Usb; 

} Audio_HAL_Access_s;



extern struct Audio_HAL_Access_s	gAudioHalAccess; // allow others to use it.

// Enable ADC HQ audio recording path
void HAL_Audio_EnablePath_ADC_HQ(Audio_Path_ADC_HQ_s *AudioPath_AdcHq, UInt32 channels, UInt32 audioID, UInt32 inputMic);
void HAL_Audio_DisablePath_ADC_HQ(Audio_Path_ADC_HQ_s *AudioPath_AdcHq);

// enable the virtual channel to DSP for FM radio playback
void HAL_AUDIO_EnableVirtualChannel_PlayFmI2s(Virtual_Channel_Dscr_s *VirtualChannel_PlayFmI2s);
// disable the virtual channel to DSP for FM radio playback
void HAL_AUDIO_DisableVirtualChannel_PlayFmI2s(Virtual_Channel_Dscr_s *VirtualChannel_PlayFmI2s);
// virtual channle to DSP for FM radio callback, send the fifo block to virtual channel. 
void HAL_AUDIO_VirtualChannel_PlayFmI2s(UInt16 *data, UInt32 size, int start_channel);

// enable the virtual channel to DSP to record FM as PCM
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_PCM(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_PCM);
// disable the virtual channel to DSP  to record FM as PCM
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_PCM(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_PCM);
// virtual channle to record FM as PCM copy data to upper layer. 
void HAL_AUDIO_VirtualChannel_RecordFmI2s_PCM(UInt16 *data, UInt32 size);

// enable the virtual channel to record FM as AAC: into DSP
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_In(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_In);
// disable the virtual channel to record FM as AAC: into DSP
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_In(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_In);
// virtual channle to record FM as AAC copy data to upper layer.into DSP
void HAL_AUDIO_VirtualChannel_RecordFmI2s_AAC_DSP_In(UInt16 *data, UInt32 size);

// enable the virtual channel to record FM as AAC: outof DSP
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_Out(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_Out);
// disable the virtual channel to record FM as AAC: Outof DSP
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_Out(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_Out);
// virtual channle to record FM as AAC copy data to upper layer.Outof DSP
void HAL_AUDIO_VirtualChannel_RecordFmI2s_AAC_DSP_Out(UInt16 *data, UInt32 size);

// Enable FM I2S device
void HAL_AUDIO_EnableExtDev_FmI2s(Audio_Ext_Device_s *ExtDev_FmI2s);
// disable FM I2S device
void HAL_AUDIO_DisableExtDev_FmI2s(Audio_Ext_Device_s *ExtDev_FmI2s);

// Enable FM I2S device's software Fifo
void HAL_AUDIO_EnableExtDev_FmI2s_swFifo(Audio_Ext_Device_s *ExtDev_FmI2s);
// disable FM I2S device's software Fifo
void HAL_AUDIO_DisableExtDev_FmI2s_swFifo(Audio_Ext_Device_s *ExtDev_FmI2s);


//
// Audio_HAL_s 
//



typedef enum
{
	UNMUTE_ALL,
	MUTE_POSITIVE_END,		
	MUTE_NEGAITIVE_END,		
	MUTE_ALL		
} BB_Mute_Action_t;

#if ( defined(BCM2153_FAMILY_BB_CHIP_BOND) && !defined(ENABLE_DSPEXTMEM))
typedef enum
{
	SPEAKER_100MW	,
	SPEAKER_400MW	
} BB_Spk_Output_t;
#else
typedef enum
{
	SPEAKER_400MW,	
	SPEAKER_100MW	
} BB_Spk_Output_t;
#endif

typedef enum
{
	CHANNEL_DIFF,	
	CHANNEL_3WIRE_1,
	CHANNEL_3WIRE_2,
	CHANNEL_3WIRE_3,
	CHANNEL_3WIRE_4,
	CHANNEL_MAX	
} BB_Ch_Mode_t;

typedef enum
{
	LOUT_LIN,	
	LOUT_R_L,
	LOUT_RIN,
	LOUT_NO,
	LOUT_MAX	
} BB_LPathCh_Mode_t;

typedef enum
{
	ROUT_RIN,	
	ROUT_R_L,
	ROUT_LIN,
	ROUT_NO,
	ROUT_MAX	
} BB_RPathCh_Mode_t;

#ifdef BCM2153_FAMILY_BB_CHIP_BOND
typedef enum
{
	MIC_ANALOG_ADC1 	= 0,	
	MIC_UNUSED 			= 1,
	MIC_DIGITAL_MIC1	= 2,
	MIC_DIGITAL_MIC2	= 3,
	MIC_MAX	
} BB_DualMic_Path_t;
#endif

void AUDIO_DRV_SelectMIC(AUDIO_DRV_MIC_Path_t pathMIC);
void AUDIO_DRV_SetMICGain(UInt8 gainMIC);
void AUDIO_DRV_SetMICGainIndex(UInt8 mic_gain_index);
void AUDIO_DRV_MuteRightSpk(BB_Mute_Action_t muteAction);
void AUDIO_DRV_MuteLeftSpk(BB_Mute_Action_t muteAction);
void AUDIO_DRV_SetRChannelMode(BB_Ch_Mode_t ChMode);
void AUDIO_DRV_SetLChannelMode(BB_Ch_Mode_t ChMode);
void AUDIO_DRV_SetAudioLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode);
void AUDIO_DRV_SetPolyLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode);
void AUDIO_DRV_SelectRightSpkOutput(BB_Spk_Output_t pathSpeaker);
void AUDIO_DRV_SelectLeftSpkOutput(BB_Spk_Output_t pathSpeaker);
void AUDIO_DRV_SetRightSpkPGA(UInt8 gainSpk);
void AUDIO_DRV_SetLeftSpkPGA(UInt8 gainSpk);

#ifdef BCM2153_FAMILY_BB_CHIP_BOND
Boolean AUDIO_DRV_IsCall16K(AudioMode_t voiceMode);
AudioMode_t AUDIO_DRV_ConvertVoiceIDChnlMode(AudioClientID_en_t audioID, AUDIO_CHANNEL_t outputChnl);
AudioMode_t AUDIO_DRV_ConvertChnlMode(AUDIO_CHANNEL_t outputChnl);
AUDIO_CHANNEL_t AUDIO_DRV_ConvertModeChnl(AudioMode_t mode);
void AUDIO_DRV_SetDualMicPathOnOff(AudioMode_t voiceMode , Boolean on_off);
SysIndMultimediaAudioParm_t * AUDIO_GetParmMultimediaAccessPtr(void);
#if (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__))&& !defined(SS_2153)
void AUDIO_DRV_SetExtAMPOn(AudioMode_t ChMode);
void AUDIO_DRV_SetExtAMPOff(AudioMode_t ChMode);
void AUDIO_DRV_MuteExtAMP(void);
void AUDIO_DRV_InitExtAMP(void);
#endif

#endif //  BCM2153_FAMILY_BB_CHIP_BOND


#ifndef FEATURE_AUDIO_VOICECALL16K
void AUDIO_DRV_SetVoicePathOn(AudioMode_t voiceMode);
#else // FEATURE_AUDIO_VOICECALL16K
void AUDIO_DRV_SetVoicePathOn(AudioMode_t voiceMode, AudioClientID_en_t audioID);
#endif // FEATURE_AUDIO_VOICECALL16K
void AUDIO_DRV_SetVoicePathOff(AudioMode_t voiceMode);

void AUDIO_DRV_SetVoiceVolume(UInt8 volume);
UInt8 AUDIO_DRV_GetVoiceVolumeInDB(UInt8 Volume);
void AUDIO_DRV_Set_DL_OnOff(Boolean on_off);

void AUDIO_DRV_SetPCMOnOff(Boolean	on_off);

void AUDIO_DRV_SetMicrophonePathOnOff(AudioMode_t voiceMode , Boolean on_off);

void AUDIO_DRV_ConnectMicrophoneUplink(Boolean on_off);
void AUDIO_DRV_SetMicrophonePath_16K(Boolean on_off);
void AUDIO_DRV_MicrophoneMuteOnOff(Boolean mute_on_off);
//void AUDIO_DRV_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off);
Result_t AUDIO_DRV_PlayTone(SpeakerTone_t tone_id, UInt32 duration);
Result_t AUDIO_DRV_PlayGenericTone(
	Boolean superimpose,
	UInt16	duration,
	UInt16	f1,
	UInt16	f2,
	UInt16	f3
	);
void AUDIO_DRV_StopPlayTone(void);
void AUDIO_ASIC_SetAudioMode(AudioMode_t mode);
void AUDIO_DRV_SetVoiceMode(AudioMode_t voiceMode);
void AUDIO_DRV_SetInternalCTM( Boolean enable );

void AUDIO_DRV_SetAMRPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAMRPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAMRPathGain(UInt16 Gain);
#ifdef USE_DSP_AUDIO_AMR_VOLUME
void AUDIO_DRV_SetAMRVolume(UInt16 uSpkvol);
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME
void AUDIO_DRV_MuteAMRPathOnOff(Boolean mute_on_off);
void AUDIO_DRV_SetDTMFPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetDTMFPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetDTMFPathGain(UInt16 Gain);
#if !defined( SS_2153 )
typedef UInt8 speakerVol_t;
#else
typedef UInt16 speakerVol_t;
#endif
void AUDIO_DRV_SetToneVolume(AudioToneType_en_t eToneType, speakerVol_t uSpkvol);
void AUDIO_DRV_MuteDTMFPathOnOff(Boolean mute_on_off);

void AUDIO_DRV_SetAudioPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAudioPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAudioPathGain(UInt16 Lgain, UInt16 Rgain);
void AUDIO_DRV_MuteAudioPathOnOff(Boolean mute_on_off);

void AUDIO_DRV_SetPolyPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetPolyPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetPolyPathGain(UInt16 Lgain, UInt16 Rgain);
void AUDIO_DRV_MutePolyPathOnOff(Boolean mute_on_off);
void AUDIO_DRV_SetAudioMixerGain(int Lgain, int Rgain);
void AUDIO_DRV_SetAudioEquGain( Int16 *gain );
void AUDIO_DRV_SetPolyEquGain( Int16 *gain );

void AUDIO_DRV_SetPcmReadBackPathOn(void);
void AUDIO_DRV_SetPcmReadBackPathOff(void);

void AUDIO_DRV_InitReg(void);

void AUDIO_DRV_SetHifiDacMask(void);

void AUDIO_DRV_PowerUpLeftDrv(void);

void AUDIO_DRV_PowerUpRightDrv(void);

void AUDIO_DRV_PowerUpLeftRightDrv(void);

void AUDIO_DRV_PowerDownLeftDrv(void);

void AUDIO_DRV_PowerDownRightDrv(void);

void AUDIO_DRV_PowerDownLeftRightDrv(void);

void AUDIO_DRV_PowerUpLeftDac(void);

void AUDIO_DRV_PowerUpRightDac(void);

void AUDIO_DRV_PowerUpLeftRightDac(void);

void AUDIO_DRV_PowerDownLeftDac(void);

void AUDIO_DRV_PowerDownRightDac(void);

void AUDIO_DRV_PowerDownLeftRightDac(void);

void AUDIO_DRV_PowerUpRef(void);

void AUDIO_DRV_PowerUpD2C(void);

void AUDIO_DRV_PowerDownRef(void);

void AUDIO_DRV_PowerDownD2C(void);

void AUDIO_DRV_SetBgtcsp(void);

void AUDIO_DRV_SetPdOffsetGenLR(void);

void AUDIO_DRV_ClearPdOffsetGenLR(void);

void AUDIO_DRV_SetPwrmosGateCntrlLR(void);

void AUDIO_DRV_ClearPwrmosGateCntrlLR(void);

void AUDIO_DRV_SetRampRefEn(void);

void AUDIO_DRV_SetSTAudioPath(Boolean mode);
Boolean AUDIO_DRV_GetSTAudioPath(void);
void AUDIO_DRV_SetAudioChannel(AUDIO_CHANNEL_t chan); // 01062009 michael

void AUDIO_DRV_SetPolyChannel(AUDIO_CHANNEL_t chan);
Boolean AUDIO_DRV_GetSTPolyPath(void);
void AUDIO_DRV_SetSTPolyPath(Boolean mode);

#if defined(SS_2153)
void AUDIO_DRV_SetCustomGain(UInt16 spk_pga, UInt16 mic_pga, UInt16 mixer_gain); // 02062009 michael
#endif // SS_2153

void AUDIO_DRV_CtrlAnalogLoopback(Boolean v_onoff);

#if defined(ROXY_AUD_CTRL_FEATURES)
//****fix me **** also defined in hal_auido_access.c as static func
UInt16 AUDIO_DRV_GetCustomGain(CUSTOM_GAIN_t gain_index); // 02182009 michael
Boolean AudioExtDev_OnOff(void);
#endif

#ifdef SS_2153
void AUDIO_DRV_SetAuxMicBiasConfig(Boolean on_off, AsicAuxMicBiasMode_t BiasMode);
void AUDIO_DRV_PowerControlMIC(Boolean mic_off, Boolean ldo_off);
#endif // SS_2153

//++++++++++++++++++++++Android Linux+++++++++++++++++++++
#include "hal_audio.h"


typedef enum {
	AuxMicBias_OFF = 0,
	AuxMicBias_ON_LPMODE,  //0.4V
	AuxMicBias_ON_FPMODE,  //2.1V
	AuxMicBias_INVALID
} AsicAuxMicBiasMode_t;  // Aux Mic Bias


#define	APSYSPARM_GetAudioParmAccessPtr	AUDIO_GetParmAccessPtr
#define OSTASK_Sleep 	msleep

void CAPI2_RIPCMDQ_Connect_Downlink(UInt32 tid, UInt8 clientID, Boolean Downlink);
void CAPI2_VOLUMECTRL_SetBasebandVolume(UInt32 tid, UInt8 clientID, UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid);
void AUDIO_Turn_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off);
AudioMode_t	AUDIO_GetAudioMode();
void program_FIR_IIR_filter(UInt16 mode);
void program_poly_FIR_IIR_filter(UInt16 mode);
void AUDIO_DRV_SetIIS2Audio(Boolean DirectPathOnOff);
void AUDIO_DRV_SetAudioSampleRate(AudioSampleRate_en_t sampleRate);
void audvoc_write_register(UInt16 addr, UInt16 value);
UInt8 AUDIO_DRV_GetMICGainIndex(void);
void audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
void audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);

#define	AUDIO_DRV_EC_NS_OnOff	AUDIO_Turn_EC_NS_OnOff


#define	ANACR0_REG		(&REG_SYS_ANACR0)
#define	ANACR1_REG		(&REG_SYS_ANACR1)
#define	ANACR2_REG		(&REG_SYS_ANACR2)
#define	ANACR3_REG		(&REG_SYS_ANACR3)


// Audio mode control register 
#define AUDVOC_AUDMOD_REG                 0xE7D0

// Audio FIFO control register
#define AUDVOC_AFIFOCTRL_REG              0xE7D1

// Audio FIFO status register
#define AUDVOC_AIFIFOST_REG               0xE7D2

// Audio L/R channel register
#define AUDVOC_ALRCH_REG                  0xE7D3

// Audio equalizer path delay offset	0
#define AUDVOC_AEQPATHOFST0_REG           0xE7D4

// Audio equalizer path delay offset	1
#define AUDVOC_AEQPATHOFST1_REG           0xE7D5

// Audio equalizer path delay offset	2
#define AUDVOC_AEQPATHOFST2_REG           0xE7D6

// main clock frequency set register
#define AUDVOC_CLKDIV_REG                 0xE7D7

// Audio L channle slop gain control Register   
#define AUDVOC_ALSLOPGAIN_REG             0xE7D8

// Audio R channle slop gain control Register   
#define AUDVOC_ARSLOPGAIN_REG             0xE7D9

// Audio /polyringer /voice paths L channel mixer gain control Register    
#define AUDVOC_APVLMXGAIN_REG             0xE7DA

// Audio /polyringer /voice paths R channel mixer gain control Register    
#define AUDVOC_APVRMXGAIN_REG             0xE7DB

// Audio Equalizer Path1 Gain control Register       
#define AUDVOC_AEQPATHGAIN1_REG           0xE7E1

// Audio Equalizer Path2 Gain control Register       
#define AUDVOC_AEQPATHGAIN2_REG           0xE7E2

// Audio Equalizer Path3 Gain control Register       
#define AUDVOC_AEQPATHGAIN3_REG           0xE7E3

// Audio Equalizer Path4 Gain control Register       
#define AUDVOC_AEQPATHGAIN4_REG           0xE7E4

// Audio Equalizer Path5 Gain control Register       
#define AUDVOC_AEQPATHGAIN5_REG           0xE7E5

// Audio input FIFO  DATA0 Register        
#define AUDVOC_AIFIFODATA0_REG           0xE7F1

// Audio input FIFO  DATA1 Register        
#define AUDVOC_AIFIFODATA1_REG            0xE7F2

// Audio input FIFO  DATA2 Register        
#define AUDVOC_AIFIFODATA2_REG            0xE7F3

// Audio Equalizer coefficient memory address Register             
#define AUDVOC_AEQCOFADD_REG              0xE7F4

// Audio Equalizer coefficient memory data Register                  
#define AUDVOC_AEQCOFDATA_REG             0xE7F5

// Audio compensation FIR filter  coefficient memory address Register                     
#define AUDVOC_ACOMPFIRCOFADD_REG         0xE7F6

// Audio compensation FIR filter  coefficient memory data Register     
#define AUDVOC_ACOMPFIRCOFDATA_REG        0xE7F7

// Audio compensation IIR filter  coefficient memory address Register          
#define AUDVOC_ACOMPIIRCOFADD_REG         0xE7F8

// Audio compensation IIR filter  coefficient memory data Register               
#define AUDVOC_ACOMPIIRCOFDATA_REG        0xE7F9

// Polyringer mode control Register
#define AUDVOC_PAUDMOD_REG 0xE7B0

// Polyringer  FIFO control Register
#define PFIFOCTRL_REG                    0xE7B1

// Polyringer  FIFO status Register
#define AUDVOC_PIFIFOST_REG               0xE7B2

// Polyringer  L/R channel Register
#define AUDVOC_PLRCH_REG                  0xE7B3

// Polyringer  Equalizer path delay offset 0  
#define AUDVOC_PEQPATHOFST0_REG           0xE7B4

// Polyringer  Equalizer path delay offset 1  
#define AUDVOC_PEQPATHOFST1_REG           0xE7B5

// Polyringer  Equalizer path delay offset 2  
#define AUDVOC_PEQPATHOFST2_REG           0xE7B6

// Polyringer  L channle slop gain control Register     
#define AUDVOC_PLSLOPGAIN_REG             0xE7B8

// Polyringer  R channle slop gain control Register     
#define AUDVOC_PRSLOPGAIN_REG             0xE7B9

// Polyringer Equalizer Path1 Gain control Register        
#define AUDVOC_PEQPATHGAIN1_REG           0xE7BC

// Polyringer Equalizer Path2 Gain control Register        
#define AUDVOC_PEQPATHGAIN2_REG           0xE7BD

// Polyringer Equalizer Path3 Gain control Register        
#define AUDVOC_PEQPATHGAIN3_REG           0xE7BE

// Polyringer Equalizer Path4 Gain control Register        
#define AUDVOC_PEQPATHGAIN4_REG           0xE7BF

// Polyringer Equalizer Path5 Gain control Register        
#define AUDVOC_PEQPATHGAIN5_REG           0xE7B7

// Polyringer  input FIFO  DATA0 Register             
#define AUDVOC_PIFIFODATA0_REG            0xE7C1

// Polyringer  input FIFO  DATA1 Register             
#define AUDVOC_PIFIFODATA1_REG            0xE7C2

// Polyringer  input FIFO  DATA2 Register             
#define AUDVOC_PIFIFODATA2_REG            0xE7C3

// Polyringer  Equalizer coefficient memory address Register                  
#define AUDVOC_PEQCOFADD_REG              0xE7C4

// Polyringer  Equalizer coefficient memory data Register                       
#define AUDVOC_PEQCOFDATA_REG             0xE7C5

// Polyringer  compensation FIR filter  coefficient memory address Register                            
#define AUDVOC_PCOMPFIRCOFADD_REG         0xE7C6

// Polyringer  compensation FIR filter  coefficient memory data Register                                 
#define AUDVOC_PCOMPFIRCOFDATA_REG        0xE7C7

// Polyringer  compensation IIR filter  coefficient memory address Register                                      
#define AUDVOC_PCOMPIIRCOFADD_REG         0xE7C8

// Polyringer  compensation IIR filter  coefficient memory data Register                                           
#define AUDVOC_PCOMPIIRCOFDATA_REG        0xE7C9

// Voice IIR coefficient register
//#define AUDVOC_VCOEFR_REG                 0xE5D0

// Voice compensation filter gain register
#define AUDVOC_VCFGR_REG                  0xE7A3

// Voice input sample register
#define AUDVOC_AUDIR_REG                  0xE7A4

// Voice putput sample register
#define AUDVOC_AUDOR_REG                  0xE7A5

// Voice mote control register
#define AUDVOC_VMUT_REG                   0xE7A8

// Voice OFIFO control register
 #define AUDVOC_VFIFOCTRL_REG              0xE7A9

// Voice channel slop gain control register
#define AUDVOC_VSLOPGAIN_REG              0xE7AB

// Audio precision interface control register
#define AUDVOC_APCTRK_REG                 0xE7AD

// Voice ADC control register
#define AUDVOC_VADC_REG                   0xE7AC

// Analog interface register
#define AUDVOC_AIR_REG                    0xE7AE

// Analog power ramp register
#define AUDVOC_APRR_REG                   0xE7AF

// Left channel sigma-delta dithering seed register
#define AUDVOC_LSDMSEED0_REG              0xE7E7

// Left channel sigma-delta dithering seed register
#define AUDVOC_LSDMSEED1_REG              0xE7E8

// Left channel sigma-delta dithering polynomial register
#define AUDVOC_LSDMPOLY0_REG              0xE7E9

// Left channel sigma-delta dithering polynomial register
#define AUDVOC_LSDMPOLY1_REG              0xE7EA

// Right channel sigma-delta dithering seed register
#define AUDVOC_RSDMSEED0_REG              0xE7EB

// Right channel sigma-delta dithering seed register
#define AUDVOC_RSDMSEED1_REG              0xE7EC

// Right channel sigma-delta dithering polynomial register
#define AUDVOC_RSDMPOLY0_REG              0xE7ED

// Right channel sigma-delta dithering polynomial register
#define AUDVOC_RSDMPOLY1_REG              0xE7EE

// Sigma-delta dithering control register
#define AUDVOC_SDMDTHER_REG               0xE7EF

// PCM Input sample register
#define AUDVOC_PCMDIR_REG                 0xE7A6

// PCM output sample register
#define AUDVOC_PCMDOR_REG                 0xE7A7


// PCM rate control register
#define AUDVOC_PCMRATE_REG                0xE542

// BTMTR Bluetooth Mixer Tap Control Register 
#define AUDVOC_DSP_AUDIO_BTMTR            0xE7DC

// BTMXFIFOSTA Bluetooth Mixer Tap FIFO Status Register 
#define AUDVOC_DSP_AUDIO_BTMXFIFOSTA      0xE7DD

// BTMIXER Config Register 
#define AUDVOC_DSP_AUDIO_BTMIXER_CFG      0xE7DC

// BTMIXER Config Register2 
#define AUDVOC_DSP_AUDIO_BTMIXER_CFG2     0xE7DD

// BTMIXER Left Channel Gain Config Register 
#define AUDVOC_DSP_AUDIO_BTMIXER_GAIN_L   0xE7DE

// BTMIXER Right Channel Gain Config Register 
#define AUDVOC_DSP_AUDIO_BTMIXER_GAIN_R   0xE7DF

//----------------------------Android Linux---------------------------

#endif	//_HAL_AUDIO_ACCESS_H_
