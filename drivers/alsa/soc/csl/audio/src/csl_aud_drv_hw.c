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
*   @file   audvoc_drv_hw.c
* @brief  Audio HW Driver
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "resultcode.h"
//#include "sysmap_types.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_syscfg.h"
#include "brcm_rdb_ahb_tl3r.h"
//#include "csl_sysmap.h"
#include "chal_types.h"
#include "chal_audiomisc.h"
#include "chal_audiospeaker.h"
#include "chal_audiomic.h"
#include "chal_audiovopath.h"
#include "chal_audiovipath.h"
#include "chal_audioaopath.h"
#include "chal_audioaipath.h"
#include "chal_audiopopath.h"
#include "chal_audiopcmif.h"
#include "chal_audiomixer.h"
#include "chal_audiomixertap.h"
#include "csl_aud_drv.h"
#include "ostask.h"
#include "log.h"
#include "clk_drv.h"
#include "syscfg_drv.h"

#include "assert.h"
#include "gpio_drv.h"


#ifdef UNDER_LINUX
#include <asm/io.h>

#define SIZE_64K 0xffff
#endif

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct {
	AUDDRV_SPKR_Enum_t    speaker_select;
    AUDIO_SPKR_CH_Mode_t  speaker_config;
} spkr_cfg_st;

static spkr_cfg_st spkr_cfg[AUDDRV_SPKR_TOTAL_NUM];

static CHAL_HANDLE spkr_handle=0;
static CHAL_HANDLE mic_handle=0;
static CHAL_HANDLE pcmif_handle=0;
static CHAL_HANDLE aopath_handle=0;
static CHAL_HANDLE popath_handle=0;
static CHAL_HANDLE vopath_handle=0;
static CHAL_HANDLE vipath_handle=0;
static CHAL_HANDLE aipath_handle=0;
static CHAL_HANDLE mixer_handle=0;
static CHAL_HANDLE mixer_tap_handle=0;

#ifdef UNDER_LINUX
OSDAL_CLK_HANDLE  hclk_adc;
OSDAL_CLK_HANDLE  hclk_dac;
#else
//Clock driver handle
static  void* clk_handle=NULL;
#endif
//digital HW gains
static UInt32 popath_slopgain_r_hex=0x91ff, popath_slopgain_l_hex=0x91ff;
static UInt32 aopath_slopgain_r_hex=0x91ff, aopath_slopgain_l_hex=0x91ff;
static UInt32 vopath_slopgain_hex=0x91ff;
static UInt32 mic_gain = 0;

static UInt16 left_mixertap_gain=0x1fff, right_mixertap_gain=0x1fff;

//keep tracks of HW control requests from HW ctrl driver and data drivers.
static Boolean ao_en_for_hwctrl = FALSE;
static Boolean ao_en_for_datadriver = FALSE;
static Boolean po_en_for_hwctrl = FALSE;
static Boolean po_en_for_datadriver = FALSE;
/**
static Boolean vo_en_for_hwctrl = FALSE;
static Boolean vo_en_for_datadriver = FALSE;
static Boolean vi_en_for_hwctrl = FALSE;
static Boolean vi_en_for_datadriver = FALSE;
*/
static Boolean ai_en_for_hwctrl = FALSE;
static Boolean ai_en_for_datadriver = FALSE;
//static Boolean btnb_en_for_hwctrl = FALSE;
//static Boolean btnb_en_for_datadriver = FALSE;
static Boolean btwb_en_for_hwctrl = FALSE;
static Boolean btwb_en_for_datadriver = FALSE;

//digital HW status
static Boolean aopath_enabled = FALSE;
static Boolean popath_enabled = FALSE;
static Boolean vopath_enabled = FALSE;
static Boolean vipath_enabled = FALSE;
static Boolean aipath_enabled = FALSE;
static Boolean btwb_enabled = FALSE;
static Boolean btnb_enabled = FALSE;

//analog HW status
//speaker usage - each speaker & mixer has 3 inputs.
static AUDDRV_SPKR_Enum_t voiceSpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t voiceSpkr2 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t audioSpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t audioSpkr2 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t polySpkr1 = AUDDRV_SPKR_NONE;
static AUDDRV_SPKR_Enum_t polySpkr2 = AUDDRV_SPKR_NONE;
static AUDDRV_MIC_Enum_t		voice_mic_current	= AUDDRV_MIC_NONE;
static AUDDRV_MIC_Enum_t		audio_mic_current	= AUDDRV_MIC_NONE;
static Boolean micMuteStatus = FALSE;
static Boolean micPoweredOn[ AUDDRV_MIC_TOTAL_NUM ];

static Boolean bInVoiceCall = FALSE;
static Boolean bVin_2nd_enabled = FALSE;

#define SPKR_InUse (   voiceSpkr1 != AUDDRV_SPKR_NONE || voiceSpkr2 != AUDDRV_SPKR_NONE \
					|| audioSpkr1 != AUDDRV_SPKR_NONE || audioSpkr2 != AUDDRV_SPKR_NONE \
					|| polySpkr1 != AUDDRV_SPKR_NONE  || polySpkr2 != AUDDRV_SPKR_NONE \
					)
#define D2C_InUse ( bInVoiceCall \
					|| aopath_enabled || popath_enabled || vopath_enabled \
					|| vipath_enabled || aipath_enabled || btwb_enabled || btnb_enabled \
					|| SPKR_InUse \
					)

#define MIC_InUse (  voice_mic_current != AUDDRV_MIC_NONE \
					|| audio_mic_current != AUDDRV_MIC_NONE \
					)

#define AUDIO_HW_IDLE_CAN_GLOBAL_RESET 	(  bInVoiceCall==FALSE \
					&& aopath_enabled==FALSE && popath_enabled==FALSE && vopath_enabled==FALSE \
					&& vipath_enabled==FALSE && aipath_enabled==FALSE && btwb_enabled==FALSE && btnb_enabled==FALSE \
					&& voiceSpkr1==AUDDRV_SPKR_NONE && voiceSpkr2==AUDDRV_SPKR_NONE \
					&& audioSpkr1==AUDDRV_SPKR_NONE && audioSpkr2==AUDDRV_SPKR_NONE \
					&& polySpkr1==AUDDRV_SPKR_NONE  && polySpkr2==AUDDRV_SPKR_NONE \
					&& voice_mic_current==AUDDRV_MIC_NONE && audio_mic_current==AUDDRV_MIC_NONE \
					)

static UInt32 zeroSamples[CHAL_AOFIFO_SIZE] = {0};

static AUDIO_CHANNEL_NUM_t  audio_path_to_mixer = AUDIO_CHANNEL_STEREO;
static AUDIO_CHANNEL_NUM_t  poly_path_to_mixer = AUDIO_CHANNEL_STEREO;

static CB_GetAudioMode_t  client_GetAudioMode = NULL;
static CB_SetAudioMode_t  client_SetAudioMode = NULL;
static CB_SetMusicMode_t  client_SetMusicMode = NULL;

//=============================================================================
// Private function prototypes
//=============================================================================

static void auddrv_config_spkrChMode( 
				AUDDRV_SPKR_Enum_t    speaker,
				AUDIO_SPKR_CH_Mode_t  speaker_config
				);
static void auddrv_pwrOffUnusedSpkr( void );
static void auddrv_powerOnSpkr(	void );

static void auddrv_pwrOnMic(
				AUDDRV_MIC_Enum_t		mic,  //the mic selection to power on
				AUDIO_SAMPLING_RATE_t   sample_rate
				);
static void auddrv_pwrOffUnusedMic( void );  //check voice_mic_current and audio_mic_current, power off un-used mic.

static void auddrv_select_mic_input(
				AUDDRV_InOut_Enum_t      input_path,
				AUDDRV_MIC_Enum_t        mic_selection
				);

static void auddrv_pwrdn_d2c ( Boolean  power_down );

static void auddrv_mixer_add_input(
				AUDDRV_InOut_Enum_t    input_path_to_mixer,
				AUDDRV_SPKR_Enum_t     mixer_speaker_selection,
				AUDIO_CHANNEL_NUM_t    input_to_mixer
				);
static void auddrv_mixer_remove_input( 
				AUDDRV_InOut_Enum_t   input_path_to_mixer,
				AUDDRV_SPKR_Enum_t    mixer_speaker_selection
				);
static CHAL_AUDIO_MIXER_en get_CHAL_AUDIO_MIXER_en( AUDDRV_SPKR_Enum_t mixer);

static void auddrv_mixer_remove_all_inputs();

//=============================================================================
// Functions
//=============================================================================

//=============================================================================
//
// Function Name: AUDDRV_SPKRInit
//
// Description:   Inititialize audio cHAL handles
//
//=============================================================================
void AUDDRV_SPKRInit (
			AUDDRV_SPKR_Enum_t    speaker,	
			AUDIO_SPKR_CH_Mode_t  speaker_config
			)
{
	UInt8 i =0;
    

#ifdef UNDER_LINUX
    void __iomem *ahb_audio_base = NULL;
	void __iomem *ahb_tl3r_base = NULL;
#else
    UInt32 ahb_audio_base = AHB_AUDIO_BASE_ADDR;
	UInt32 ahb_tl3r_base = AHB_TL3R_BASE_ADDR;

#endif

    Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit %d, %d *\n\r", speaker, speaker_config );

    // Sys config base address is already mapped. Audio Base addresses are not mapped Let's do it here
#ifdef UNDER_LINUX
    ahb_audio_base = ioremap_nocache(AHB_AUDIO_BASE_ADDR, SIZE_64K);
	if (!ahb_audio_base) {
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit Mapping ahb_audio base failed*\n\r");
		return;
	}

    ahb_tl3r_base = ioremap_nocache(AHB_TL3R_BASE_ADDR, SIZE_64K);
	if (!ahb_tl3r_base) {
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit Mapping ahb_tl3r_base failed*\n\r");
		return;
	}

    Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit:ahb_audio_base=0x%x ahb_tl3r_base=0x%x *\n\r",ahb_audio_base,ahb_tl3r_base);
#endif

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit %d, %d *\n\r", speaker, speaker_config );

	if( spkr_handle==0)
	{
		spkr_handle = chal_audiospeaker_Init( SYSCFG_BASE_ADDR, ahb_audio_base ,ahb_tl3r_base);
		mic_handle  = chal_audiomic_Init(  SYSCFG_BASE_ADDR, AUXMIC_BASE_ADDR, ahb_audio_base );
		/*pcmif_handle = */chal_audiopcmif_Init( ahb_audio_base, SYSCFG_BASE_ADDR );
		aopath_handle = chal_audioaopath_Init(ahb_audio_base);
		popath_handle = chal_audiopopath_Init(ahb_audio_base);
		vopath_handle = chal_audiovopath_Init( ahb_audio_base );
		vipath_handle = chal_audiovipath_Init( ahb_audio_base );
		aipath_handle = chal_audioaipath_Init( ahb_audio_base );
		mixer_handle = chal_audiomixer_Init(ahb_audio_base);
		mixer_tap_handle = chal_audiomixertap_Init(ahb_audio_base);
#ifdef UNDER_LINUX
        hclk_adc = OSDAL_CLK_Open(OSDAL_CLK_AUDIO_RX_ADC_CLOCK);
        hclk_dac = OSDAL_CLK_Open(OSDAL_CLK_AUDIO_TX_DAC_CLOCK);
#else
        clk_handle = CLKDRV_Open();
#endif

		for(i=0; i<AUDDRV_SPKR_TOTAL_NUM; i++)
		{
			spkr_cfg[speaker].speaker_select = AUDDRV_SPKR_NONE;
			spkr_cfg[speaker].speaker_config = AUDIO_SPKR_CHANNEL_INVALID;
		}

		chal_audiospeaker_write_ANACR0 ( spkr_handle, 0x00FF );  //according to datasheet, set bit [8:6] to 011.

		//disable audio DAC/ADC clock
		//*(volatile UInt32 *)0x08140140 |= 0x0020;
		//*(volatile UInt32 *)0x08140140 |= 0x01;
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit: Disable DAC/ADC Clock*\n\r" );

#ifdef UNDER_LINUX
        OSDAL_CLK_Stop(hclk_adc);
        OSDAL_CLK_Stop(hclk_dac);
#else
        CLKDRV_Stop_Clock(  clk_handle, CLK_AUDIO_RX_ADC_CLOCK  );
        CLKDRV_Stop_Clock(  clk_handle, CLK_AUDIO_TX_DAC_CLOCK  );
#endif


/**
IOCR0.DIGMIC_MUX:  DIGMIC/GPIO[63:62] Select
0 DIGMICDATA/DIGMICCLK
1 GPIO[63:62]
*/
		//brcm_rdb_syscfg.h does not match ASIC RDB
		//*(volatile UInt32 *)0x08880000 &= ~(0x01000000); //clear bit 24 for Select DIGMIC_MUX
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SPKRInit: Select DIGMIC_MUX*\n\r" );
        SYSCFGDRV_Config_Pin_Mux(   SYSCFG_DIGMIC_GPIO_MUX_DIGMIC_SEL   );

		for(i=0; i<AUDDRV_MIC_TOTAL_NUM; i++)
		{
			micPoweredOn[i]= FALSE;
		}
	}

	spkr_cfg[speaker].speaker_select = speaker;
	spkr_cfg[speaker].speaker_config = speaker_config;

    //auddrv_mixer_remove_all_inputs();
}


#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
//extern UInt16 enable_global_reset;
#endif

//=============================================================================
//
// Function Name: AUDDRV_Telephony_InitHW
//
// Description:   Inititialize audio HW for voice call
//
//=============================================================================
void AUDDRV_Telephony_InitHW (
						  AUDDRV_MIC_Enum_t       mic,
						  AUDDRV_SPKR_Enum_t      speaker,
						  AUDIO_SAMPLING_RATE_t   sample_rate
						  )
{
	/**
	The sequence to turn on Audio/poly/voice path

Rule #1: POWER ON OF THE ANALOG CIRCUIT 
"	Write to ANACR2[3:0]= 0x4 register to turn ON ADC_26M clock
"	Write to ANACR0[0]= 0 register to turn ON ADC power ON
"	Write to ANACR1[7]=1
"	Write to ANACR1[29:28]=2'b11
"	PMU power on AVDD
"	Write to ANACR1[26]=1
Rule #2: CHANNEL SELECTION
"	Write to ANACR0[14:13] = 2'b11 or 2'b00 depending on which channel to select
Rule #3: FIFO CONFIGURATION
"	Reset the FIFOs and write the proper thresholds
"	Fill the input FIFO of digital path more than the threshold ( audio, poly ringer or voice)
"	Program the digital path filter coefficients

Rule #4: POWER ON OF THE DIGITAL CIRCUIT
"	ONLY for Poly ringer path, disable the DSP sleep
"	Enable the digital path by writing to AMCR for voice, AUDMOD for Audio stereo or POLYMOD for Poly ringer path and keep the slop gain to be -128db (default value)
Rule #5: CLICK & POP PREVENTION
"	Write to ANACR0[5]=0
"	Write to ANACR0[2:1]=0
"	PMU turn on SPVDD
"	Write to ANACR0[4:3] =0
"	Write to APRR = 0x0087
"	Wait for 150ms
"	Write to ANACR1[31:30]=2'b11 end of ramp, output begin 1.5v
"	Turn on the digital slope gain
	**/

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_InitHW mic %d, speaker %d, sample_rate %d ", mic, speaker, sample_rate );

#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
	if(AUDIO_HW_IDLE_CAN_GLOBAL_RESET)
	{
		//if(enable_global_reset==1)
		{
		OSTASK_Sleep(1);
		chal_audiospeaker_AUDIOSRST( spkr_handle );
		OSTASK_Sleep(1);
		}
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_InitHW did not SRST %d %d %d %d, vi %d %d %d %d, vs %d %d %d %d, ps %d %d %d %d*\n\r",
					bInVoiceCall,
					aopath_enabled, popath_enabled, vopath_enabled,
					vipath_enabled, aipath_enabled, btwb_enabled, btnb_enabled,
					voiceSpkr1, voiceSpkr2,
					audioSpkr1, audioSpkr2,
					polySpkr1, polySpkr2,
					voice_mic_current, audio_mic_current
					);
	}
	//need delay after SRST?
#endif

	AUDDRV_SetVCflag( TRUE );  //audio_vdriver sets this flag too.
	voiceSpkr1 = speaker;
	voice_mic_current = mic;

	//#1
	auddrv_pwrOnMic( mic, sample_rate );
	//the above already includes auddrv_pwrdn_d2c( FALSE );

	chal_audiospeaker_ForcePwrUpMask( spkr_handle );
	chal_audiospeaker_EpDacPwrGateCtrl( spkr_handle );
	chal_audiospeaker_EpDacRampRefEn( spkr_handle );

	//#2
	auddrv_config_spkrChMode( speaker, spkr_cfg[ speaker ].speaker_config );
	auddrv_mixer_add_input( AUDDRV_VOICE_OUTPUT, speaker, AUDIO_CHANNEL_MONO );

	//#3
	//AHB_AUDIO_VOICEFIFO_CLEAR_R  needed?
	//AHB_AUDIO_VOICEFIFO_THRES_R, AHB_AUDIO_PCMFIFO_CONTROL_STATUS_R, default is 1.

	//rule #4
	chal_audiovopath_Mute( vopath_handle, TRUE );
	chal_audiovipath_Enable( vipath_handle, TRUE );  //AMCR.AUDEN

	if( mic == AUDDRV_DUAL_MIC_DIGI12
		|| mic == AUDDRV_DUAL_MIC_DIGI21
		|| mic == AUDDRV_DUAL_MIC_ANALOG_DIGI1
		|| mic == AUDDRV_DUAL_MIC_DIGI1_ANALOG 
	  )
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWInput calls chal_audiovipath2_Enable *\n\r" );
		  //set adc2_sw_en in DSP_AUDIO_ADCCONTROL[5] for enabling 2nd digital mic
		chal_audiovipath2_Enable( vipath_handle, TRUE );
		bVin_2nd_enabled = TRUE;
	}

	auddrv_select_mic_input( AUDDRV_VOICE_INPUT, mic );
	chal_audiovipath_SetSampleRate ( vipath_handle, sample_rate );

	// after enable digital HW, load MPM filters and set MPG DGA gains.
	client_SetAudioMode( client_GetAudioMode() );

	//#5
	auddrv_powerOnSpkr( );

	chal_audiospeaker_EpDacPdOffsetGen( spkr_handle, CHAL_AUDIO_SPKRA_L, FALSE);  //?
	chal_audiovopath_SetSlopeGainHex( vopath_handle, vopath_slopgain_hex);
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_DeinitHW
//
// Description:   Disable audio HW for voice call
//
//=============================================================================
void AUDDRV_Telephony_DeinitHW( void )
{
	//this one does not know which anancr0? and it will also affect output path.

/***
 The sequence to turn off Audio/poly/voice path

Rule #1: CLICK & POP PREVENTION 
"	Write to ANACR1[7]=1
"	Turn off the digital slope gain and ramp down
"	Write to ANACR1[31:30]=0
"	Write to APRR =x04007
"	Wait for 150ms, end of ramp down
"	Write to ANACR0[4:3]=2'b11 PMU turn off SPVDD
"	Write to ANACR0[2:1]=2'b11
"	Write to ANACR0[5]=1

Rule #2: POWER OFF OF THE DIGITAL CIRCUIT
"	Disable the digital path by writing to AMCR for voice, AUDMOD for Audio stereo or POLYMOD for Poly ringer path
"	Enable the DSP sleep ONLY for the poly ringer path

Rule #3: FIFO CONFIGURATION
"	Flush the FIFOs again by resetting them

Rule #4: POWER DOWN OF THE ANALOG CIRCUIT 
"	Write to ANACR0[0] = 1 to pwer off the analog part of the audvoc
"	Write to ANACR2[3:0]= 0x3 register to turn OFF ADC_26M clock
"	PMU turn off AVDD

***/

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_DeinitHW ");

	AUDDRV_SetVCflag( FALSE );  //audio_vdriver clears this flag too.
	voiceSpkr1 = AUDDRV_SPKR_NONE;
	voice_mic_current = AUDDRV_MIC_NONE;  //MT call during recording, concurrent use is Ok?

	//#1
	chal_audiospeaker_ForcePwrUpMask( spkr_handle );
	chal_audiovopath_Mute( vopath_handle, TRUE );
	chal_audiospeaker_EpDacPdOffsetGen( spkr_handle, CHAL_AUDIO_SPKRA_L, TRUE); //?

	auddrv_pwrOffUnusedSpkr( );

	//#2
	chal_audiovipath_Enable( vipath_handle, FALSE );  //AMCR.AUDEN
	chal_audiovipath2_Enable( vipath_handle, FALSE );
	bVin_2nd_enabled = FALSE;

	//#3
	//AHB_AUDIO_VOICEFIFO_CLEAR_R  needed?

	//#4
	auddrv_pwrOffUnusedMic( );

#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
	if(AUDIO_HW_IDLE_CAN_GLOBAL_RESET)
	{
		//if(enable_global_reset==1)
		{
			OSTASK_Sleep(1);
			chal_audiospeaker_AUDIOSRST( spkr_handle );
			OSTASK_Sleep(1);
		}
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_InitHW did not SRST %d %d %d %d, vi %d %d %d %d, vs %d %d %d %d, ps %d %d %d %d*\n\r",
					bInVoiceCall,
					aopath_enabled, popath_enabled, vopath_enabled,
					vipath_enabled, aipath_enabled, btwb_enabled, btnb_enabled,
					voiceSpkr1, voiceSpkr2,
					audioSpkr1, audioSpkr2,
					polySpkr1, polySpkr2,
					voice_mic_current, audio_mic_current
					);
	}
#endif
}


//=============================================================================
//
// Function Name: AUDDRV_EnableHWOutput
//
// Description:   Enable hardware audio path
//				The path includes mixer input path and/or speaker driver(s). 
//				This includes the control sequence for enabling audio output path.
//
//				sample_rate is optional parameter. This parameter is also passed in AUDDRV_Play_SetConfig( ).
//				but for voice path sample_rate is needed to set correct sample rate for voice call.
//
// The control logic is like:
//  Audio controller                           Audio router/streaming driver
//         |                                                    |
//         |_____HW enable_______       ________data enable_____|
//                              |       |
//                          HW control driver
//                          { if (HW enable == TRUE && data enable == TRUE)
//                                  enable/disable HW following ASIC recommended contorl sequence(1,2,3, );
//                          }
//
//    set up and enable the DMA channel in DMA controller before enable audio FIFO.
//    vice versa, disable audio FIFO before disable the DMA channel in DMA controller.
//=============================================================================
void AUDDRV_EnableHWOutput (
			AUDDRV_InOut_Enum_t     input_path_to_mixer,
			AUDDRV_SPKR_Enum_t      mixer_speaker,    //selection of speaker (and mixer)
			Boolean                 enable_speaker,
			AUDIO_SAMPLING_RATE_t   sample_rate,
			AUDIO_CHANNEL_NUM_t     input_to_mixer,
			AUDDRV_REASON_Enum_t    reason
			)
{
	static AUDDRV_SPKR_Enum_t   local_mixer_speaker_voice, local_mixer_speaker_audio, local_mixer_speaker_poly;
	static Boolean              local_enable_speaker_voice, local_enable_speaker_audio, local_enable_speaker_poly;
	static AUDIO_SAMPLING_RATE_t  local_sample_rate_voice; //local_sample_rate_audio, local_sample_rate_poly;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_EnableHWOutput path %d, mixer %d, sample_rate %d input_to_mixer %d, reason %d \n\r",
		input_path_to_mixer, mixer_speaker, sample_rate, input_to_mixer, reason );

	  // for reason AUDDRV_REASON_HW_LOOPBACK, we only need loopback from voice in to voice out.

	  // save the parameters to local variables, set the ao_en_for_hwctrl.
	if ( reason == AUDDRV_REASON_HW_CTRL || reason==AUDDRV_REASON_HW_LOOPBACK )
	{
		   //for reason AUDDRV_REASON_DATA_DRIVER, these parameters bear no meaning:
		switch(input_path_to_mixer) 
		{
		case AUDDRV_VOICE_OUTPUT:
			//vo_en_for_hwctrl = TRUE;

			local_mixer_speaker_voice	= mixer_speaker;
			local_enable_speaker_voice  = enable_speaker;
			local_sample_rate_voice		= sample_rate;
			break;

		case AUDDRV_AUDIO_OUTPUT:
			ao_en_for_hwctrl = TRUE;

			local_mixer_speaker_audio	= mixer_speaker;
			local_enable_speaker_audio  = enable_speaker;
			//local_sample_rate_audio		= sample_rate;

			audio_path_to_mixer = input_to_mixer;
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			po_en_for_hwctrl = TRUE;

			local_mixer_speaker_poly	= mixer_speaker;
			local_enable_speaker_poly  = enable_speaker;
			//local_sample_rate_poly		= sample_rate;

			poly_path_to_mixer = input_to_mixer;
			break;
        default:
            break;
		}
	}

	  // set ao_en_for_datadriver. 
	if ( reason == AUDDRV_REASON_DATA_DRIVER )
	{
		switch(input_path_to_mixer) 
		{
		case AUDDRV_VOICE_OUTPUT:
			//vo_en_for_datadriver = TRUE;
			break;

		case AUDDRV_AUDIO_OUTPUT:
			ao_en_for_datadriver = TRUE;
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			po_en_for_datadriver = TRUE;
			break;
        default:
            break;
		}
	}
	  // data driver is not involved in HW_LOOPBACK
	if ( reason == AUDDRV_REASON_HW_LOOPBACK )
	{
		switch(input_path_to_mixer) 
		{
		case AUDDRV_VOICE_OUTPUT:
			//vo_en_for_datadriver = TRUE;
			break;

		case AUDDRV_AUDIO_OUTPUT:
			ao_en_for_datadriver = TRUE;
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			po_en_for_datadriver = TRUE;
			break;
        default:
            break;
        }
    }

#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
	if(AUDIO_HW_IDLE_CAN_GLOBAL_RESET)
	{
		//if(enable_global_reset==1)
		{
		//OSTASK_Sleep(1);
		chal_audiospeaker_AUDIOSRST( spkr_handle );
		//OSTASK_Sleep(1);
		}
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWOutput did not SRST %d %d %d %d, vi %d %d %d %d, vs %d %d %d %d, ps %d %d %d %d*\n\r",
					bInVoiceCall,
					aopath_enabled, popath_enabled, vopath_enabled,
					vipath_enabled, aipath_enabled, btwb_enabled, btnb_enabled,
					voiceSpkr1, voiceSpkr2,
					audioSpkr1, audioSpkr2,
					polySpkr1, polySpkr2,
					voice_mic_current, audio_mic_current
					);
	}
#endif

	chal_audiovopath_DisableDitheringShuffling( vopath_handle, TRUE );

	switch(input_path_to_mixer) 
	{
		case AUDDRV_VOICE_OUTPUT:

//when enable voice out path, does it need to enable ADC_analog ANACR2[3:0] before AMCR[5] is turned on?
//if not, then enable voice input later than enable voice output could have noise in ADC?
			//but SRST will wipe out audio output filters.

			auddrv_mixer_add_input( AUDDRV_VOICE_OUTPUT, local_mixer_speaker_voice, AUDIO_CHANNEL_MONO );
			auddrv_config_spkrChMode( local_mixer_speaker_voice, spkr_cfg[ local_mixer_speaker_voice ].speaker_config );
			
			vopath_enabled = TRUE;
			if(local_enable_speaker_voice)
				voiceSpkr1 = local_mixer_speaker_voice;

			chal_audiovopath_Mute(vopath_handle, FALSE);
			
			auddrv_powerOnSpkr( );

			chal_audiovopath_SetSlopeGainHex( vopath_handle, vopath_slopgain_hex);

			chal_audiovopath_SetSampleRate ( vopath_handle, local_sample_rate_voice );
			chal_audiovopath_Enable( vopath_handle, TRUE );

			// after enable digital HW, load MPM filters and set MPG DGA gains.
			client_SetAudioMode( client_GetAudioMode() );

			break;

		case AUDDRV_AUDIO_OUTPUT:

			if( ao_en_for_hwctrl==FALSE || ao_en_for_datadriver==FALSE )
				break;  //does not enable HW until both HW ctrol driver and data driver are ready.

			auddrv_mixer_add_input( AUDDRV_AUDIO_OUTPUT, local_mixer_speaker_audio, audio_path_to_mixer );
			auddrv_config_spkrChMode( local_mixer_speaker_audio, spkr_cfg[ local_mixer_speaker_audio ].speaker_config );

			aopath_enabled = TRUE;
			if(local_enable_speaker_audio)
				audioSpkr1 = local_mixer_speaker_audio;
			  //mute
			chal_audioaopath_SetSlopeGainLeftHex(aopath_handle, 0xa000);
			chal_audioaopath_SetSlopeGainRightHex(aopath_handle, 0xa000);
			chal_audioaopath_SelectFilter(aopath_handle, CHAL_AUDIO_FIR);

			chal_audioaopath_ClrFifo( aopath_handle );
			chal_audioaopath_SetFifoThres( aopath_handle, 0x40, 0x41 );
			 //Input FIFO Loading  (pre-fill fifo)
			chal_audioaopath_WriteFifo ( NULL, &zeroSamples[0], CHAL_AOFIFO_SIZE-1 );
			chal_audioaopath_EnableDMA(NULL, TRUE);

#if 0
            //Kishore- enable this for direct interrupt and comment the previous line
            //chal_audioaopath_DisableAudInt( NULL, FALSE );
#endif
			chal_audioaopath_Enable(aopath_handle, TRUE);
			
			  // after enable digital HW, load MPM filters and set MPG DGA gains.
			client_SetMusicMode( client_GetAudioMode() );
			
			// Need this sleep after select IIR filter, otherwise we can hear pop noise
			auddrv_powerOnSpkr( );

			chal_audioaopath_SetSlopeGainLeftHex(aopath_handle, aopath_slopgain_l_hex);
			chal_audioaopath_SetSlopeGainRightHex(aopath_handle, aopath_slopgain_r_hex);
			
			break;

		case AUDDRV_RINGTONE_OUTPUT:
		
			if( po_en_for_hwctrl==FALSE || po_en_for_datadriver==FALSE )
				break;  //does not enable HW until both HW ctrol driver and data driver are ready.

			auddrv_mixer_add_input( AUDDRV_RINGTONE_OUTPUT, local_mixer_speaker_poly, poly_path_to_mixer );
			auddrv_config_spkrChMode( local_mixer_speaker_poly, spkr_cfg[ local_mixer_speaker_poly ].speaker_config );

			popath_enabled = TRUE;
			if(local_enable_speaker_poly)
				polySpkr1 = local_mixer_speaker_poly;
			  //mute
			chal_audiopopath_SetSlopeGainLeftHex(popath_handle, 0xa000);
			chal_audiopopath_SetSlopeGainRightHex(popath_handle, 0xa000);
			chal_audiopopath_SelectFilter(popath_handle, CHAL_AUDIO_IIR); //use default IIR filter

			chal_audiopopath_ClrFifo ( popath_handle );
			chal_audiopopath_SetFifoThres( popath_handle, 0x40, 0x41 );
			 //Input FIFO Loading  (pre-fill fifo)
			chal_audiopopath_WriteFifo (NULL, &zeroSamples[0], CHAL_POFIFO_SIZE-1 );
			chal_audiopopath_EnableDMA(popath_handle, TRUE);
			chal_audiopopath_Enable(popath_handle, TRUE);
			
			  // after enable digital HW, load MPM filters and set MPG DGA gains.
			client_SetMusicMode( client_GetAudioMode() );
			
			auddrv_powerOnSpkr( );
			
			chal_audiopopath_SetSlopeGainLeftHex(popath_handle, popath_slopgain_l_hex);
			chal_audiopopath_SetSlopeGainRightHex(popath_handle, popath_slopgain_r_hex);	
			
			break;

		default:
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_DisableHWOutput
//
// Description:   Disable hardware audio path
//				save the last volume, and set volume at end of enable.
//
//=============================================================================
void AUDDRV_DisableHWOutput ( 
			 AUDDRV_InOut_Enum_t	path,
			 AUDDRV_REASON_Enum_t	reason
			)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWOutput path %d reason%d *\n\r", path, reason );

	// only after disable is finished, the next enable is Ok.
	// if disable ctrl, enable ctrl, disable data, enable data, it will be problematic.
	if ( reason == AUDDRV_REASON_DATA_DRIVER )
	{
		switch(path) 
		{
		case AUDDRV_VOICE_OUTPUT:
			//vo_en_for_datadriver = FALSE;
			break;

		case AUDDRV_AUDIO_OUTPUT:
			ao_en_for_datadriver = FALSE;
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			po_en_for_datadriver = FALSE;
			break;
        default:
			break;
		}
	}
	else
	if ( reason == AUDDRV_REASON_HW_CTRL )
	{
		switch(path) 
		{
		case AUDDRV_VOICE_OUTPUT:
			//vo_en_for_hwctrl = FALSE;
			break;

		case AUDDRV_AUDIO_OUTPUT:
			ao_en_for_hwctrl = FALSE;
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			po_en_for_hwctrl = FALSE;
			break;
        default:
			break;

        }
	}

	switch(path)
	{
		case AUDDRV_VOICE_OUTPUT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWOutput VOICE_OUT path %d *\n\r", path );
			auddrv_mixer_remove_input( path, voiceSpkr1 );
			auddrv_mixer_remove_input( path, voiceSpkr2 );
			vopath_enabled = FALSE;
			//need to check voice In
			if ( vipath_enabled == FALSE )
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWOutput disable AMCR *\n\r" );
				chal_audiovopath_Enable( vopath_handle, FALSE );
			}
			else
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWOutput since VI is en, do not dis AMCR *\n\r" );
			}
			voiceSpkr1 = AUDDRV_SPKR_NONE;
			voiceSpkr2 = AUDDRV_SPKR_NONE;

			chal_audiovopath_Mute(vopath_handle, FALSE);
			auddrv_pwrOffUnusedSpkr( );

			break;

		case AUDDRV_AUDIO_OUTPUT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWOutput AUDIO_OUT path %d *\n\r", path );

			if ( ao_en_for_hwctrl ==TRUE || ao_en_for_datadriver ==TRUE )
			{
				break;
			}

			auddrv_mixer_remove_input( path, audioSpkr1 );
			auddrv_mixer_remove_input( path, audioSpkr2 );
			aopath_enabled = FALSE;
			audioSpkr1 = AUDDRV_SPKR_NONE;
			audioSpkr2 = AUDDRV_SPKR_NONE;
			
			auddrv_pwrOffUnusedSpkr( );
			chal_audioaopath_Enable(aopath_handle, FALSE);
			chal_audioaopath_SetI2SMode(aopath_handle, FALSE );
			chal_audioaopath_EnableDMA(aopath_handle, FALSE);
			
			break;

		case AUDDRV_RINGTONE_OUTPUT:

			if ( po_en_for_hwctrl ==TRUE || po_en_for_datadriver ==TRUE )
			{
				break;
			}

			auddrv_mixer_remove_input( path, polySpkr1 );
			auddrv_mixer_remove_input( path, polySpkr2 );
			popath_enabled = FALSE;
			polySpkr1 = AUDDRV_SPKR_NONE;
			polySpkr2 = AUDDRV_SPKR_NONE;

			auddrv_pwrOffUnusedSpkr( );

			chal_audiopopath_Enable(popath_handle, FALSE);
			chal_audiopopath_EnableDMA(popath_handle, FALSE);

			break;

		default:
			break;
	}

	auddrv_pwrOffUnusedSpkr( );
	auddrv_pwrdn_d2c( TRUE );
}


//=============================================================================
//
// Function Name: AUDDRV_EnableHWInput
//
// Description:   Enable hardware audio input path
//				path includes microphone input and digital processing path.
//
//				sample_rate is optional parameter. This parameter is also passed in AUDDRV_Record_SetConfig( ).
//				but for voice path sample_rate is needed to set correct sample rate for voice call.
//
// The control logic is like:
//  Audio controller                           Audio router/streaming driver
//         |                                                    |
//         |_____HW enable_______       ________data enable_____|
//                              |       |
//                          HW control driver
//                          { if (HW enable == TRUE && data enable == TRUE)
//                                  enable/disable HW following ASIC recommended contorl sequence(1,2,3, );
//                          }
//
//    set up and enable the DMA channel in DMA controller before enable audio FIFO.
//    vice versa, disable audio FIFO before disable the DMA channel in DMA controller.
//
//=============================================================================
void AUDDRV_EnableHWInput (
		AUDDRV_InOut_Enum_t		input_path,
		AUDDRV_MIC_Enum_t		mic_selection,
		AUDIO_SAMPLING_RATE_t	sample_rate,
		AUDDRV_REASON_Enum_t	reason
	   )
{
	static AUDDRV_MIC_Enum_t		local_mic_selection_voice, local_mic_selection_audio;
	static AUDIO_SAMPLING_RATE_t    local_sample_rate_voice, local_sample_rate_audio;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_EnableHWInput mic_selection %d, sample_rate %d, reason %d \n\r", mic_selection, sample_rate, reason );

		  // for reason AUDDRV_REASON_HW_LOOPBACK, we only need loopback from voice in to voice out.

	  // save the parameters to local variables, set the ai_en_for_hwctrl.
	if ( reason == AUDDRV_REASON_HW_CTRL || reason==AUDDRV_REASON_HW_LOOPBACK )
	{
		   //for reason AUDDRV_REASON_DATA_DRIVER, these parameters bear no meaning:
		switch(input_path) 
		{
		case AUDDRV_VOICE_INPUT:
			//vi_en_for_hwctrl = TRUE;

			local_mic_selection_voice	= mic_selection;
			local_sample_rate_voice  = sample_rate;
			break;

		case AUDDRV_AUDIO_INPUT:
			ai_en_for_hwctrl = TRUE;

			local_mic_selection_audio	= mic_selection;
			local_sample_rate_audio  = sample_rate;
			break;
        default:
            break;

        }
	}

	  // set ai_en_for_datadriver. 
	if ( reason == AUDDRV_REASON_DATA_DRIVER )
	{
		switch(input_path) 
		{
		case AUDDRV_VOICE_INPUT:
			//vi_en_for_datadriver = TRUE;
			break;

		case AUDDRV_AUDIO_INPUT:
			ai_en_for_datadriver = TRUE;
			break;
        default:
            break;

        }
	}
	  // data driver is not involved in HW_LOOPBACK
	if ( reason == AUDDRV_REASON_HW_LOOPBACK )
	{
		switch(input_path) 
		{
		case AUDDRV_VOICE_INPUT:
			//vi_en_for_datadriver = TRUE;
			break;

		case AUDDRV_AUDIO_INPUT:
			ai_en_for_datadriver = TRUE;
			break;
        default:
            break;
        }
    }


#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
	if(AUDIO_HW_IDLE_CAN_GLOBAL_RESET)
	{
		//if(enable_global_reset==1)
		{
		//OSTASK_Sleep(1);
		chal_audiospeaker_AUDIOSRST( spkr_handle );
		//OSTASK_Sleep(1);
		}
	}
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWInput did not SRST %d %d %d %d, vi %d %d %d %d, vs %d %d %d %d, ps %d %d %d %d*\n\r",
					bInVoiceCall,
					aopath_enabled, popath_enabled, vopath_enabled,
					vipath_enabled, aipath_enabled, btwb_enabled, btnb_enabled,
					voiceSpkr1, voiceSpkr2,
					audioSpkr1, audioSpkr2,
					polySpkr1, polySpkr2,
					voice_mic_current, audio_mic_current
					);
	}
#endif

	switch(input_path) 
	{
		case AUDDRV_VOICE_INPUT:
		
			vipath_enabled = TRUE;
			voice_mic_current = local_mic_selection_voice;
			//rule #1
			auddrv_pwrOnMic( local_mic_selection_voice, local_sample_rate_voice );
			auddrv_select_mic_input( AUDDRV_VOICE_INPUT, local_mic_selection_voice );
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWInput en voice in *\n\r" );
			chal_audiovipath_SetSampleRate ( vipath_handle, local_sample_rate_voice );
			chal_audiovipath_Enable( vipath_handle, TRUE );  //AMCR.AUDEN

			//move this into auddrv_pwrOnMic( )?
			if( local_mic_selection_voice == AUDDRV_DUAL_MIC_DIGI12
				|| local_mic_selection_voice == AUDDRV_DUAL_MIC_DIGI21
				|| local_mic_selection_voice == AUDDRV_DUAL_MIC_ANALOG_DIGI1
				|| local_mic_selection_voice == AUDDRV_DUAL_MIC_DIGI1_ANALOG 
			  )
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWInput calls chal_audiovipath2_Enable *\n\r" );
				//set adc2_sw_en in DSP_AUDIO_ADCCONTROL[5] for enabling 2nd digital mic
				chal_audiovipath2_Enable( vipath_handle, TRUE );
				bVin_2nd_enabled = TRUE;
			}

			// after enable digital HW, load MPM filters and set MPG DGA gains.
			client_SetAudioMode( client_GetAudioMode() );

			break;

		case AUDDRV_AUDIO_INPUT:  //HQ Audio input

			if( ai_en_for_hwctrl==FALSE || ai_en_for_datadriver==FALSE )
				break;  //does not enable HW until both HW ctrol driver and data driver are ready.

			aipath_enabled = TRUE;
			audio_mic_current = local_mic_selection_audio;
			//rule #1
			auddrv_pwrOnMic( local_mic_selection_audio, local_sample_rate_audio );
			chal_audioaipath_CfgIIRCoeff( aipath_handle, 2 );

			auddrv_select_mic_input( input_path, local_mic_selection_audio );
			
			// read FIFO fifo to clear any FIFO lockup
			chal_audioaipath_ReadFifoResidue (NULL);

			chal_audioaipath_EnableDMA(aipath_handle, TRUE);
			chal_audioaipath_Enable(aipath_handle, TRUE);

			// after enable digital HW, load MPM filters and set MPG DGA gains.
			client_SetMusicMode( client_GetAudioMode() );

			break;

		default:
			break;
	}
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableHWInput Exit *\n\r");
}

//=============================================================================
//
// Function Name: AUDDRV_DisableHWInput
//
// Description:   Disable hardware audio input path
//
//=============================================================================
void AUDDRV_DisableHWInput (
			AUDDRV_InOut_Enum_t		path,
			AUDDRV_REASON_Enum_t	reason
		 )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWInput *\n\r" );

	switch(path) 
	{
		case AUDDRV_VOICE_INPUT:

			//vi_en_for_hwctrl = FALSE;
			//vi_en_for_datadriver = FALSE;

			vipath_enabled = FALSE;
			//voice_mic_current = AUDDRV_MIC_NONE;
			//need to fix this because if we check here, VO is on during voice call
			// so siabel VI path will not be called
			if (AUDDRV_GetVCflag() == FALSE) //( vopath_enabled == FALSE )
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWInput disable AMCR *\n\r" );
				//rule #2
				chal_audiovipath_Enable( vipath_handle, FALSE );
				chal_audiovipath2_Enable( vipath_handle, FALSE );
				bVin_2nd_enabled = FALSE;
                voice_mic_current = AUDDRV_MIC_NONE;
			}
			else
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_DisableHWInput VO is en, do not disable AMCR *\n\r" );
			}
			//rule #4
			auddrv_pwrOffUnusedMic( );
			break;

		case AUDDRV_AUDIO_INPUT:  //HQ Audio input

			ai_en_for_hwctrl = FALSE;
			ai_en_for_datadriver = FALSE;

			aipath_enabled = FALSE;
			audio_mic_current = AUDDRV_MIC_NONE;

				// disable DMA request
			chal_audioaipath_EnableDMA(aipath_handle, FALSE);
			//rule #2
			chal_audioaipath_Enable(aipath_handle, FALSE);
			chal_audiovipath2_Enable( vipath_handle, FALSE ); //need this? need to check if voice needs 2nd mic.
			bVin_2nd_enabled = FALSE; //need voice path in Audio?
			//rule #4
			auddrv_pwrOffUnusedMic( );

			break;

		default:
			break;
	}

	auddrv_pwrdn_d2c( TRUE ); // turn down d2c, don't know which D2C to use right now. use the main one(anacr0).
}


//=============================================================================
//
// Function Name: AUDDRV_SelectMic
//
// Description:   Select Mic for the path.
//			If voice input path or audio input path is enabled, this function
//			also powers on the new mic selected.
//
//=============================================================================
void AUDDRV_SelectMic ( AUDDRV_InOut_Enum_t  path,  AUDDRV_MIC_Enum_t  mic )
{
	// remember new mic selection:
	switch( path )
	{
		case AUDDRV_VOICE_INPUT:
			if( voice_mic_current == mic )
				return;

			voice_mic_current = mic;
			break;

		case AUDDRV_AUDIO_INPUT:
			if( audio_mic_current == mic )
				return;

			audio_mic_current = mic;
			break;

		default:
			//error if reach here
			break;
	}

	auddrv_pwrOnMic( mic, AUDIO_SAMPLING_RATE_UNDEFINED );
	auddrv_pwrOffUnusedMic( );   //check voice_mic_current and audio_mic_current, power off un-used mic.

	auddrv_select_mic_input( path, mic );
	// in case Vin path2 is not enabled, need to enable it here
	if (FALSE == bVin_2nd_enabled)
	{
			if(mic == AUDDRV_MIC_DIGI2
				|| mic == AUDDRV_DUAL_MIC_DIGI12
				|| mic == AUDDRV_DUAL_MIC_DIGI21
				|| mic == AUDDRV_DUAL_MIC_ANALOG_DIGI1
				|| mic == AUDDRV_DUAL_MIC_DIGI1_ANALOG )
			{
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SelectMic calls chal_audiovipath2_Enable *\n\r" );
				chal_audiovipath2_Enable( vipath_handle, TRUE );
				bVin_2nd_enabled = TRUE;
			}

	}

}

//=============================================================================
//
// Function Name: AUDDRV_SelectSpkr
//
// Description:   Select speaker for the path.
//
//=============================================================================
void AUDDRV_SelectSpkr (
       AUDDRV_InOut_Enum_t  path,
       AUDDRV_SPKR_Enum_t   speaker,
       AUDDRV_SPKR_Enum_t   speaker_second
       )
{
	switch(path)
	{
		case AUDDRV_VOICE_OUTPUT:
			//mixer input selection
			auddrv_mixer_remove_input( path, voiceSpkr1);
			auddrv_mixer_remove_input( path, voiceSpkr2);
			
			voiceSpkr1 = speaker;
			voiceSpkr2 = speaker_second;

			auddrv_mixer_add_input( path, voiceSpkr1, AUDIO_CHANNEL_MONO);
			auddrv_mixer_add_input( path, voiceSpkr2, AUDIO_CHANNEL_MONO);

			client_SetAudioMode( client_GetAudioMode() );

			break;

		case AUDDRV_AUDIO_OUTPUT:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SelectSpkr - AUDIO spkr %d, 2nd_spkr %d \n\r", 
					speaker, speaker_second );

			//mixer input selection.
			auddrv_mixer_remove_input( path, audioSpkr1);
			auddrv_mixer_remove_input( path, audioSpkr2);

			audioSpkr1 = speaker;
			audioSpkr2 = speaker_second;

			auddrv_mixer_add_input( path, audioSpkr1, audio_path_to_mixer);
			auddrv_mixer_add_input( path, audioSpkr2, audio_path_to_mixer);

			client_SetMusicMode( client_GetAudioMode() );

			break;

		case AUDDRV_RINGTONE_OUTPUT:
			//mixer input selection
			auddrv_mixer_remove_input( path, polySpkr1);
			auddrv_mixer_remove_input( path, polySpkr2);
			
			polySpkr1 = speaker;
			polySpkr2 = speaker_second;

			auddrv_mixer_add_input( path, polySpkr1, poly_path_to_mixer);
			auddrv_mixer_add_input( path, polySpkr2, poly_path_to_mixer);

			client_SetMusicMode( client_GetAudioMode() );

			break;

		case AUDDRV_ALL_OUTPUT:
			//mixer input selection
			auddrv_mixer_remove_input( path, voiceSpkr1);
			auddrv_mixer_remove_input( path, voiceSpkr2);
			auddrv_mixer_remove_input( path, audioSpkr1);
			auddrv_mixer_remove_input( path, audioSpkr2);
			auddrv_mixer_remove_input( path, polySpkr1);
			auddrv_mixer_remove_input( path, polySpkr2);

			voiceSpkr1 = speaker;
			audioSpkr1 = speaker;
			polySpkr1 = speaker;
			voiceSpkr2 = speaker_second;
			audioSpkr2 = speaker_second;
			polySpkr2 = speaker_second;

			auddrv_mixer_add_input( path, voiceSpkr1, AUDIO_CHANNEL_MONO);
			auddrv_mixer_add_input( path, voiceSpkr2, AUDIO_CHANNEL_MONO);
			auddrv_mixer_add_input( path, audioSpkr1, audio_path_to_mixer);
			auddrv_mixer_add_input( path, audioSpkr2, audio_path_to_mixer);
			auddrv_mixer_add_input( path, polySpkr1, poly_path_to_mixer);
			auddrv_mixer_add_input( path, polySpkr2, poly_path_to_mixer);

			break;

		default:
			break;
	}

	//speaker driver
	switch(speaker)
	{
		case AUDDRV_SPKR_IHF:
		case AUDDRV_SPKR_EP:
		case AUDDRV_SPKR_IHF_STEREO:
		case AUDDRV_SPKR_HS_LEFT:
		case AUDDRV_SPKR_HS_RIGHT:
		case AUDDRV_SPKR_HS:
			auddrv_config_spkrChMode( speaker,        spkr_cfg[speaker].speaker_config );
			auddrv_config_spkrChMode( speaker_second, spkr_cfg[speaker_second].speaker_config );

			auddrv_pwrOffUnusedSpkr( );
			auddrv_powerOnSpkr( );
			
			break;

		case AUDDRV_SPKR_PCM_IF:
		case AUDDRV_SPKR_USB_IF:
			//turn on AMCR PCM interface
			chal_audiopcmif_Enable(pcmif_handle, TRUE);
			auddrv_pwrOffUnusedSpkr( ); // turn off analog speaker will also check D2C off inside
			break;

		default:
			auddrv_pwrOffUnusedSpkr( );
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_Set_I2sMuxToAudio
//
// Description:   enables or disables the I2S MUX to audio output path.
//
//=============================================================================
void AUDDRV_Set_I2sMuxToAudio ( Boolean   on )
{
	if (on == TRUE)
	{
		chal_audioaopath_SetI2SMode(aopath_handle, TRUE );
		// move to pcm driver
		//chal_audioaopath_EnableDMA(aopath_handle, FALSE);
		
		//there is no data driver, so here we call it to make audio path enabled.
		AUDDRV_EnableHWOutput (
					AUDDRV_AUDIO_OUTPUT,
					AUDDRV_SPKR_NONE,  //this param bears no meaning in this context.
					FALSE,	//this param bears no meaning in this context.
					AUDIO_SAMPLING_RATE_UNDEFINED,  //this param bears no meaning in this context.
					AUDIO_CHANNEL_STEREO,
					AUDDRV_REASON_DATA_DRIVER
			  );
	}
	else
	{
		chal_audioaopath_DisableAudInt (aopath_handle, TRUE);
		chal_audioaopath_SetI2SMode(aopath_handle, FALSE );
		// move to pcm driver
		//chal_audioaopath_EnableDMA(aopath_handle, TRUE);

		//there is no data driver, so here we call it to make audio path enabled.
		AUDDRV_DisableHWOutput (
					AUDDRV_AUDIO_OUTPUT,
					AUDDRV_REASON_DATA_DRIVER
			  );
	}
}

//=============================================================================
//
// Function Name: AUDDRV_Enable_MixerTap
//
// Description:   select input to Mixer Tap, and enable the mixer tap.
//
//Note: sampleRate on Mxier Tap is set through funciton AUDDRV_Record_SetConfig( ).
//
// sample_rate is optional parameter. This parameter is also passed in AUDDRV_Record_SetConfig( ).
//
// The control logic is like:
//  Audio controller                           Audio router/streaming driver
//         |                                                    |
//         |_____HW enable_______       ________data enable_____|
//                              |       |
//                          HW control driver
//                          { if (HW enable == TRUE && data enable == TRUE)
//                                  enable/disable HW following ASIC recommended contorl sequence(1,2,3, );
//                          }
//
//    set up and enable the DMA channel in DMA controller before enable audio FIFO.
//    vice versa, disable audio FIFO before disable the DMA channel in DMA controller.
//
//=============================================================================
void AUDDRV_Enable_MixerTap (
				AUDDRV_InOut_Enum_t     mixer_tap,
				AUDDRV_SPKR_Enum_t      left_in, //left_input_selection,
				AUDDRV_SPKR_Enum_t      right_in, //right_input_selection,
				AUDIO_SAMPLING_RATE_t   sample_rate,
				AUDDRV_REASON_Enum_t	reason
				)
{
	static AUDDRV_SPKR_Enum_t  local_left_in_vb, local_left_in_wb;
	static AUDDRV_SPKR_Enum_t  local_right_in_vb, local_right_in_wb;
	static AUDIO_SAMPLING_RATE_t  local_sample_rate_vb, local_sample_rate_wb;
	UInt32 cnt;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Enable_MixerTap mixer_tap %d, left_in %d, right_in %d, sample_rate %d, reason %d \n\r", 
		mixer_tap, left_in, right_in, sample_rate, reason );

	  // save the parameters to local variables, set the btwb_en_for_hwctrl.
	if ( reason == AUDDRV_REASON_HW_CTRL )
	{
		   //for reason AUDDRV_REASON_DATA_DRIVER, these parameters bear no meaning:
		switch(mixer_tap) 
		{
		case AUDDRV_MIXERTap_WB_INPUT:
			btwb_en_for_hwctrl = TRUE;

			local_left_in_wb	= left_in;
			local_right_in_wb	= right_in;
			local_sample_rate_wb	= sample_rate;
			break;

		case AUDDRV_MIXERTap_VB_INPUT:
			//btnb_en_for_hwctrl = TRUE;

			local_left_in_vb	= left_in;
			local_right_in_vb	= right_in;
			local_sample_rate_vb	= sample_rate;
			break;
        default:
            break;

        }
	}

	  // set btwb_en_for_datadriver. 
	if ( reason == AUDDRV_REASON_DATA_DRIVER )
	{
		switch(mixer_tap) 
		{
		case AUDDRV_MIXERTap_WB_INPUT:
			btwb_en_for_datadriver = TRUE;
			break;

		case AUDDRV_MIXERTap_VB_INPUT:
			//btnb_en_for_datadriver = TRUE;
			break;
        default:
            break;

        }
	}

	switch ( mixer_tap )
	{
		case AUDDRV_MIXERTap_WB_INPUT:  ///< Wideband path

			if( btwb_en_for_hwctrl==FALSE || btwb_en_for_datadriver==FALSE )
				break;  //does not enable HW until both HW ctrol driver and data driver are ready.

            // added the following 2 to take care A2DP issue to clear Overflow condition in 
            // DSP_AUDIO_BTMIXER_CFG2 register bit 0, so that 2nd A2DP playback will work
            chal_audiomixertap_EnableWb(mixer_tap_handle, TRUE, TRUE);
            OSTASK_Sleep(10);

			chal_audiomixertap_ResetWbTap(mixer_tap_handle);  //write 0 to AUDIO_BTMIXER_CFG_R
			chal_audiomixertap_SelectInput(mixer_tap_handle, CHAL_AUDIO_MIXER_TAP_WL, get_CHAL_AUDIO_MIXER_en(local_left_in_wb));
			chal_audiomixertap_SelectInput(mixer_tap_handle, CHAL_AUDIO_MIXER_TAP_WR, get_CHAL_AUDIO_MIXER_en(local_right_in_wb));
			chal_audiomixertap_SetWbSampleRate(mixer_tap_handle, local_sample_rate_wb );

			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Enable_MixerTap - AUDDRV_MIXERTap_WB_INPUT sr %d, leftIn %d, rightIn %d \n\r", 
				local_sample_rate_wb, get_CHAL_AUDIO_MIXER_en(local_left_in_wb), get_CHAL_AUDIO_MIXER_en(local_right_in_wb) );

			chal_audiomixertap_DisableWbAudInt(mixer_tap_handle, TRUE);
			chal_audiomixertap_SetWbFifoThres(mixer_tap_handle,  0x40); //7 bits, default is 0x40
				// deplete FIFO to get ready for generating DMA request.
			cnt = (UInt32)chal_audiomixertap_ReadWbFifoEntryCnt(mixer_tap_handle);
			chal_audiomixertap_ReadWbFifo(mixer_tap_handle, NULL, cnt);
			chal_audiomixertap_EnableWbDma(mixer_tap_handle, TRUE);

			chal_audiomixertap_EnableWb(mixer_tap_handle, TRUE, TRUE); //why always enable DAC?
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_L, left_mixertap_gain); 
			AUDDRV_SetGain_Hex(AUDDRV_GAIN_MIXERTap_WB_R, right_mixertap_gain);
			btwb_enabled = TRUE;
			break;

		case AUDDRV_MIXERTap_VB_INPUT:  ///< Narrowband path
			chal_audiomixertap_ClrNbFifo(mixer_tap_handle, TRUE); 
			chal_audiomixertap_ClrNbFifo(mixer_tap_handle, FALSE); 
			chal_audiomixertap_SetNbSampleRate(mixer_tap_handle, local_sample_rate_vb );
			//current DSP implementation limits fifo threshold to 1
			chal_audiomixertap_SetNbFifoThres(mixer_tap_handle, 1); //BTNB_FIFO_THRESHOLD); //only 3 bits, default is 1 

			chal_audiomixertap_SelectInput(mixer_tap_handle, CHAL_AUDIO_MIXER_TAP_NL, get_CHAL_AUDIO_MIXER_en(local_left_in_vb));
			chal_audiomixertap_SelectInput(mixer_tap_handle, CHAL_AUDIO_MIXER_TAP_NR, get_CHAL_AUDIO_MIXER_en(local_right_in_vb));
			
			//chal_audiomixertap_EnableNb(mixer_tap_handle, TRUE); 
			btnb_enabled = TRUE;
			break;
		
		default:
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_Disable_MixerTap
//
// Description:   Disable the mixer tap.
//
//=============================================================================
void AUDDRV_Disable_MixerTap ( 
					AUDDRV_InOut_Enum_t  mixer_tap,
					AUDDRV_REASON_Enum_t	reason
					)
{
	switch ( mixer_tap )
	{
	case AUDDRV_MIXERTap_WB_INPUT:  ///< Wideband BT Tap

		btwb_en_for_hwctrl=FALSE;
		btwb_en_for_datadriver=FALSE;

		chal_audiomixertap_EnableWb(mixer_tap_handle, FALSE, FALSE);
		chal_audiomixertap_EnableWbDma(mixer_tap_handle, FALSE);

		btwb_enabled = FALSE;
		break;

	case AUDDRV_MIXERTap_VB_INPUT:  ///< Voiceband BT Tap
		chal_audiomixertap_EnableNb(mixer_tap_handle, FALSE); 
		btnb_enabled = FALSE;
		break;
		
	default:
		break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_SetMute
//
// Description:   set mute at the HW mute point.
//
//=============================================================================
void AUDDRV_SetMute ( AUDDRV_MUTE_Enum_t  mute_point,  Boolean  mute )
{
	switch(mute_point) 
	{
		case AUDDRV_MUTE_SPKR1_L:
			chal_audiospeaker_Mute(spkr_handle, CHAL_AUDIO_SPKRA_L, mute );
			break;

		case AUDDRV_MUTE_SPKR1_R:
			chal_audiospeaker_Mute(spkr_handle, CHAL_AUDIO_SPKRA_R, mute );
			break;

		case AUDDRV_MUTE_MIC:
			chal_audiomic_SetADCStandby( mic_handle, mute );  //for analog mic
			micMuteStatus = mute;
			//how to mute digital mic? can not!!
			break;

		case AUDDRV_MUTE_AUDIO_OUTPUT_L:
			if (mute)
			{
				aopath_slopgain_l_hex = 0;
				chal_audioaopath_Mute(aopath_handle, mute, FALSE);  //the 3rd parameter FALSE means do not change the right channel
			}
			else
				chal_audioaopath_SetSlopeGainLeftHex(aopath_handle, aopath_slopgain_l_hex);
			break;
		case AUDDRV_MUTE_AUDIO_OUTPUT_R:
			if (mute)
			{
				aopath_slopgain_r_hex = 0;
				chal_audioaopath_Mute(aopath_handle, FALSE, mute);
			}
			else
				chal_audioaopath_SetSlopeGainRightHex(aopath_handle, aopath_slopgain_r_hex);
			break;

		case AUDDRV_MUTE_RINGTONE_OUTPUT_L:
			if (mute)
			{
				popath_slopgain_l_hex = 0;
				chal_audiopopath_Mute(popath_handle, mute, FALSE);
			}
			else
				chal_audiopopath_SetSlopeGainLeftHex(popath_handle, popath_slopgain_l_hex);
			break;

		case AUDDRV_MUTE_RINGTONE_OUTPUT_R:
			if (mute)
			{
				popath_slopgain_r_hex = 0;
				chal_audiopopath_Mute(popath_handle, FALSE, mute);
			}
			else
				chal_audiopopath_SetSlopeGainLeftHex(popath_handle, popath_slopgain_r_hex);
			break;

		case AUDDRV_MUTE_VOICE_OUTPUT:
			if ( mute )
				chal_audiovopath_Mute(vopath_handle, mute);
			else
				chal_audiovopath_SetSlopeGainHex( vopath_handle, vopath_slopgain_hex );  //restore gain
			break;

		case AUDDRV_MUTE_MIXER1:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC1, 0 );
			break;
		case AUDDRV_MUTE_MIXER2:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC2, 0 );
			break;
		case AUDDRV_MUTE_MIXER3:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC3, 0 );
			break;
		case AUDDRV_MUTE_MIXER4:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC4, 0 );
			break;

		case AUDDRV_MUTE_MPMDGA1:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC1, 0x1fff );
			break;
		case AUDDRV_MUTE_MPMDGA2:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC2, 0x1fff );
			break;
		case AUDDRV_MUTE_MPMDGA3:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC3, 0x1fff );
			break;
		case AUDDRV_MUTE_MPMDGA4:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC4, 0x1fff );
			break;

		case AUDDRV_MUTE_MIXERTap_WB_L:
			if (TRUE==mute)
				chal_audiomixertap_SetWbGain(mixer_tap_handle, 0, right_mixertap_gain); 
			else
				chal_audiomixertap_SetWbGain(mixer_tap_handle, left_mixertap_gain, right_mixertap_gain); 
			break;
		case AUDDRV_MUTE_MIXERTap_WB_R:
			if (TRUE==mute)
				chal_audiomixertap_SetWbGain(mixer_tap_handle, left_mixertap_gain, 0); 
			else
				chal_audiomixertap_SetWbGain(mixer_tap_handle, left_mixertap_gain, right_mixertap_gain); 
			break;

		default:
			break;
	}
}

/**
*  @brief  Get mute status.
*
*  @param  mute_point	(in) mute point in HW
*
*  @return	Boolean		TTRUE: muted.   FALSE: not muted.
*
****************************************************************************/
Boolean AUDDRV_GetMute ( AUDDRV_MUTE_Enum_t  mute_point )
{
	Boolean ret = FALSE;

	switch(mute_point) 
	{
		case AUDDRV_MUTE_MIC:
			ret = micMuteStatus;
			break;

		default:
			break;
	}
	return ret;
}


//=============================================================================
//
// Function Name: AUDDRV_SetGain
//
// Description:   set gain at the HW gain adjustment point.
//
//=============================================================================
void AUDDRV_SetGain ( AUDDRV_GAIN_Enum_t  gain_adj_point, Int32  gain_mB )
{
	switch(gain_adj_point)
	{
		case AUDDRV_GAIN_SPKR_IHF:
			break;
		case AUDDRV_GAIN_SPKR_EP:
			break;

		case AUDDRV_GAIN_MIC:
			break;

		case AUDDRV_GAIN_AUDIO_OUTPUT_L:
			break;
		case AUDDRV_GAIN_AUDIO_OUTPUT_R:
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_L:
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_R:
			break;
		case AUDDRV_GAIN_VOICE_OUTPUT:
			break;
		case AUDDRV_GAIN_VOICE_OUTPUT_CFGR:
			break;

		case AUDDRV_GAIN_MIXER1:
			break;
		case AUDDRV_GAIN_MIXER2:
			break;
		case AUDDRV_GAIN_MIXER3:
			break;
		case AUDDRV_GAIN_MIXER4:
			break;
		case AUDDRV_GAIN_MPMDGA1:
			break;
		case AUDDRV_GAIN_MPMDGA2:
			break;
		case AUDDRV_GAIN_MPMDGA3:
			break;
		case AUDDRV_GAIN_MPMDGA4:
			break;
		case AUDDRV_GAIN_MIXERTap_WB_L:
			break;
		case AUDDRV_GAIN_MIXERTap_WB_R:
			break;

		default:
			break;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_GetGain
//
// Description:   get gain at the HW gain adjustment point.
//
//=============================================================================
UInt32 AUDDRV_GetGain ( AUDDRV_GAIN_Enum_t  gain_adj_point, Int32  gainFormat )
{
	switch(gain_adj_point)
	{
		case AUDDRV_GAIN_SPKR_IHF:
			break;
		case AUDDRV_GAIN_SPKR_EP:
			break;

		case AUDDRV_GAIN_MIC:
			return mic_gain;

		case AUDDRV_GAIN_AUDIO_OUTPUT_L:
			break;
		case AUDDRV_GAIN_AUDIO_OUTPUT_R:
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_L:
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_R:
			break;
		case AUDDRV_GAIN_VOICE_OUTPUT:
			break;
		case AUDDRV_GAIN_VOICE_OUTPUT_CFGR:
			break;

		case AUDDRV_GAIN_MIXER1:
			break;
		case AUDDRV_GAIN_MIXER2:
			break;
		case AUDDRV_GAIN_MIXER3:
			break;
		case AUDDRV_GAIN_MIXER4:
			break;
		case AUDDRV_GAIN_MPMDGA1:
			break;
		case AUDDRV_GAIN_MPMDGA2:
			break;
		case AUDDRV_GAIN_MPMDGA3:
			break;
		case AUDDRV_GAIN_MPMDGA4:
			break;
		case AUDDRV_GAIN_MIXERTap_WB_L:
			break;
		case AUDDRV_GAIN_MIXERTap_WB_R:
			break;

		default:
			break;
	}
	return 0;
}

//=============================================================================
//
// Function Name: AUDDRV_SetGain_Hex
//
// Description:   set gain at the HW gain adjustment point.
//				The gain_hex is writtent to HW register.
//
//=============================================================================
void AUDDRV_SetGain_Hex ( AUDDRV_GAIN_Enum_t  gain_adj_point, UInt32  gain_hex )
{
	switch(gain_adj_point)
	{
		case AUDDRV_GAIN_SPKR_IHF:
			chal_audiospeaker_SetPGAIndex(spkr_handle, CHAL_AUDIO_SPKRA_L, 7 - gain_hex);
			break;
		case AUDDRV_GAIN_SPKR_EP:
			chal_audiospeaker_SetPGAIndex(spkr_handle, CHAL_AUDIO_SPKRA_R, 7 - gain_hex);
			break;

		case AUDDRV_GAIN_MIC:
			mic_gain = gain_hex;
			//chal_audiomic_SetPGA(mic_handle, gain_milliB);
			chal_audiomic_SetPGAHex(mic_handle, gain_hex*3);
			break;

		case AUDDRV_GAIN_AUDIO_OUTPUT_L:
			chal_audioaopath_SetSlopeGainLeftHex(aopath_handle, gain_hex);
			aopath_slopgain_l_hex = gain_hex;
			break;
		case AUDDRV_GAIN_AUDIO_OUTPUT_R:
			chal_audioaopath_SetSlopeGainRightHex(aopath_handle, gain_hex);
			aopath_slopgain_r_hex = gain_hex;
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_L:
			chal_audiopopath_SetSlopeGainLeftHex(popath_handle, gain_hex);
			popath_slopgain_l_hex = gain_hex;
			break;
		case AUDDRV_GAIN_RINGTONE_OUTPUT_R:
			chal_audiopopath_SetSlopeGainRightHex(popath_handle, gain_hex);
			popath_slopgain_r_hex = gain_hex;
			break;

		case AUDDRV_GAIN_VOICE_OUTPUT:
			chal_audiovopath_SetSlopeGainHex( vopath_handle, gain_hex);
			vopath_slopgain_hex = gain_hex;
			break;

		case AUDDRV_GAIN_VOICE_OUTPUT_CFGR:
			chal_audiovopath_SetVCFGRhex (vopath_handle, gain_hex );
			break;

		case AUDDRV_GAIN_MIXER1:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC1, gain_hex );
			break;
		case AUDDRV_GAIN_MIXER2:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC2, gain_hex );
			break;
		case AUDDRV_GAIN_MIXER3:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC3, gain_hex );
			break;
		case AUDDRV_GAIN_MIXER4:
			chal_audiomixer_SetGain(mixer_handle, CHAL_AUDIO_MIXER_DAC4, gain_hex );
			break;
		case AUDDRV_GAIN_MPMDGA1:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC1, gain_hex );
			break;
		case AUDDRV_GAIN_MPMDGA2:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC2, gain_hex );
			break;
		case AUDDRV_GAIN_MPMDGA3:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC3, gain_hex );
			break;
		case AUDDRV_GAIN_MPMDGA4:
			chal_audiomixer_SetMpmDgaHex(mixer_handle, CHAL_AUDIO_MIXER_DAC4, gain_hex );
			break;
		case AUDDRV_GAIN_MIXERTap_WB_L:
			left_mixertap_gain = gain_hex;
			chal_audiomixertap_SetWbGain(mixer_tap_handle, gain_hex, right_mixertap_gain);
			break;
		case AUDDRV_GAIN_MIXERTap_WB_R:
			right_mixertap_gain = gain_hex;
			chal_audiomixertap_SetWbGain(mixer_tap_handle, left_mixertap_gain, gain_hex);
			break;

		default:
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_SetFilter
//
// Description:   load filter.
//
//=============================================================================
void AUDDRV_SetFilter( AUDDRV_Filter_Enum_t filter, const UInt16 *coeff )
{
	UInt16 voice_filt_coefs[NUM_OF_ADC_VOICE_COEFF];
	UInt8	i=0;

	switch (filter)
	{
		case AUDDRV_VoiceDAC:
			for ( i = 0; i < NUM_OF_DAC_VOICE_COEFF; i++ )
			{
				voice_filt_coefs[i] = ~coeff[ i ];
			}
			chal_audiovopath_CfgIIRCoeff( vopath_handle, voice_filt_coefs );
			break;

		case AUDDRV_VoiceADC:
			for ( i = 0; i < NUM_OF_ADC_VOICE_COEFF; i++ )
			{
				voice_filt_coefs[i] = ~coeff[ i ];
			}
			chal_audiovipath_CfgIIRCoeff( vipath_handle, voice_filt_coefs );
			break;

		case AUDDRV_AEQ:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SetFilter, CfgAEQCoeff");
			chal_audioaopath_CfgAEQCoeff( aopath_handle, coeff );
			break;

		case AUDDRV_AEQPATHGAIN:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SetFilter, SetAEQGain");
			chal_audioaopath_SetAEQGain( aopath_handle, coeff );
			break;

		case AUDDRV_AEQPATHOFST:
			break;

		case AUDDRV_AFIR:
			chal_audioaopath_CfgAFIRCoeff( aopath_handle, coeff );
			break;

		case AUDDRV_PEQ:
			chal_audiopopath_CfgAEQCoeff( popath_handle, coeff );
			break;

		case AUDDRV_PEQPATHGAIN:
			chal_audiopopath_SetAEQGain( popath_handle, coeff );
			break;

		case AUDDRV_PEQPATHOFST:
			break;

		case AUDDRV_PIIR:
			chal_audiopopath_CfgPIIRCoeff( popath_handle, coeff );
			break;

		case AUDDRV_Mixer1_MPMIIR:
			chal_audiomixer_CfgIirCoeff( mixer_handle, CHAL_AUDIO_MIXER_DAC1, (const UInt32 *)coeff );
			break;
		
		case AUDDRV_Mixer2_MPMIIR:
			chal_audiomixer_CfgIirCoeff( mixer_handle, CHAL_AUDIO_MIXER_DAC2, (const UInt32 *)coeff );
			break;

		case AUDDRV_Mixer3_MPMIIR:
			chal_audiomixer_CfgIirCoeff( mixer_handle, CHAL_AUDIO_MIXER_DAC3, (const UInt32 *)coeff );
			break;

		case AUDDRV_Mixer4_MPMIIR:
			chal_audiomixer_CfgIirCoeff( mixer_handle, CHAL_AUDIO_MIXER_DAC4, (const UInt32 *)coeff );
			break;

		default:
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_SetMPM
//
// Description:   Set MPM parameter.
//
//=============================================================================
void AUDDRV_SetMPM( AUDDRV_Filter_Enum_t param_id, const UInt16 param )
{
	switch (param_id)
	{
		case AUDDRV_Mixer_BIQUAD_CFG:
			chal_audiomixer_SetBiquads( mixer_handle,   param & 0x0FFF );  //0x0880
			chal_audiomixer_SetBiquads( mixer_handle, ((param & 0x0FFF) | 0x4000) );  //0x4880
			chal_audiomixer_SetBiquads( mixer_handle, ((param & 0x0FFF) | 0x8000) );  //0x8880
			chal_audiomixer_SetBiquads( mixer_handle, ((param & 0x0FFF) | 0xC000) );  //0xC880
  			break;

		case AUDDRV_Mixer1_MPMALDCENABLE:
			//chal_audiomixer_EnableAldc( mixer_handle, CHAL_AUDIO_MIXER_DAC1, TRUE );
			break;

		case AUDDRV_Mixer2_MPMALDCENABLE:
			break;

		case AUDDRV_Mixer3_MPMALDCENABLE:
			break;
		
		case AUDDRV_Mixer4_MPMALDCENABLE:
			break;

		default:
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_SetVCflag
//
// Description:   Set voice call flag for HW control loic.
//
//=============================================================================
void AUDDRV_SetVCflag( Boolean inVoiceCall )
{
	bInVoiceCall = inVoiceCall;
}

//=============================================================================
//
// Function Name: AUDDRV_GetVCflag
//
// Description:   Get voice call flag.
//
//=============================================================================
Boolean AUDDRV_GetVCflag( void )
{
	return bInVoiceCall;
}

//=============================================================================
//
// Function Name: AUDDRV_SetVoicePathSampRate
//
// Description:   Set voice path sample rate.
//
//=============================================================================
void AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_t  sample_rate )
{
	chal_audiovopath_SetSampleRate ( vopath_handle, sample_rate );
}


void AUDDRV_SetAudioLoopback( 
					Boolean             enable_lpbk,
					AUDDRV_MIC_Enum_t   mic,
					AUDDRV_SPKR_Enum_t  speaker
					)
{
	CHAL_AUDIO_MIC_INPUT_en chal_mic = CHAL_AUDIO_MIC_INPUT_ADC1;

	switch( mic )
	{
	case AUDDRV_MIC_ANALOG_MAIN:
	case AUDDRV_MIC_ANALOG_AUX:
		chal_mic = CHAL_AUDIO_MIC_INPUT_ADC1;
		break;

	case AUDDRV_MIC_DIGI1:
		chal_mic = CHAL_AUDIO_MIC_INPUT_DMIC1;
		break;
	case AUDDRV_MIC_DIGI2:
		chal_mic = CHAL_AUDIO_MIC_INPUT_DMIC2;
		break;
    default:
			break;

	}

	if ( enable_lpbk )
	{
		AUDDRV_EnableHWOutput ( AUDDRV_VOICE_OUTPUT, speaker, TRUE, AUDIO_SAMPLING_RATE_8000,
				AUDIO_CHANNEL_STEREO,
				AUDDRV_REASON_HW_LOOPBACK );
		AUDDRV_EnableHWInput ( AUDDRV_VOICE_INPUT, mic, AUDIO_SAMPLING_RATE_8000,
				AUDDRV_REASON_HW_LOOPBACK );
		chal_audiovipath_EnableLoopback( vipath_handle, 1, chal_mic );
	}
	else
	{
		chal_audiovipath_EnableLoopback( vipath_handle, 0, chal_mic );
		AUDDRV_DisableHWOutput ( AUDDRV_VOICE_OUTPUT, AUDDRV_REASON_HW_LOOPBACK );
		AUDDRV_DisableHWInput ( AUDDRV_VOICE_INPUT, AUDDRV_REASON_HW_LOOPBACK );
	}
}


/********************************************************************
*  @brief  Register up callback for getting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_getAudioMode( CB_GetAudioMode_t	cb )
{
	client_GetAudioMode = cb;
}

/********************************************************************
*  @brief  Register up callback for setting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setAudioMode( CB_SetAudioMode_t	cb )
{
	client_SetAudioMode = cb;
}

/********************************************************************
*  @brief  Register up callback for setting music audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setMusicMode( CB_SetMusicMode_t	cb )
{
	client_SetMusicMode = cb;
}




//=============================================================================
// Private function definitions
//=============================================================================

static void auddrv_config_spkrChMode( 
				AUDDRV_SPKR_Enum_t    speaker,
				AUDIO_SPKR_CH_Mode_t  speaker_config
				)
{
	switch(speaker)
	{
		case AUDDRV_SPKR_IHF:
			chal_audiospeaker_SetMode( spkr_handle, CHAL_AUDIO_SPKRA_L, speaker_config );

            break;

		case AUDDRV_SPKR_EP:
			chal_audiospeaker_SetMode( spkr_handle, CHAL_AUDIO_SPKRA_R, speaker_config );
			break;

		case AUDDRV_SPKR_IHF_STEREO:
			chal_audiospeaker_SetMode( spkr_handle, CHAL_AUDIO_SPKRA_L, speaker_config );
			chal_audiospeaker_SetMode( spkr_handle, CHAL_AUDIO_SPKRA_R, speaker_config );
			break;

		case AUDDRV_SPKR_PCM_IF:
			//turn on PCM interface
			chal_audiopcmif_Enable(pcmif_handle, TRUE);

			break;

		case AUDDRV_SPKR_HS_LEFT:
		case AUDDRV_SPKR_HS_RIGHT:
		case AUDDRV_SPKR_HS:
			//no options for CABLE_TOP_DAC.
		case AUDDRV_SPKR_USB_IF:
		default:
			break;
	}
}

static Boolean auddrv_spkrIsUsed( AUDDRV_SPKR_Enum_t  spkr )
{
	if( voiceSpkr1 == spkr || voiceSpkr2 == spkr
	    || audioSpkr1 == spkr || audioSpkr2 == spkr
	    || polySpkr1 == spkr  || polySpkr2 == spkr
		)
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_spkrIsUsed (%d, %d, %d, %d, %d, %d) \n\r", 
			voiceSpkr1, voiceSpkr2, audioSpkr1, audioSpkr2, polySpkr1, polySpkr2 );
		return TRUE;
	}
	else
		return FALSE;
}

// ramping time is set to 0
#define ANALOG_RAMP_UP   0x8080
#define ANALOG_RAMP_DOWN 0x8000

#define ANALOG_HS_RAMP_UP   0x0081
#define ANALOG_HS_RAMP_DOWN 0x0001
#define ANALOG_RAMP_PD		0x4000

static void auddrv_pwrOffUnusedSpkr( void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrOffUnusedSpkr() \n\r");

	if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_IHF_STEREO )
		&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_IHF )
		&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_EP )
		)
	{
		//Log_DebugPrintf(LOGID_AUDIO, "\t* auddrv_pwrOffUnusedSpkr() MIXER12 \n\r");
		chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
		//OSTASK_Sleep(150);
		chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRA_L, TRUE);
		chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRA_R, TRUE);
		chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_L, TRUE);
	}
	else
	{
		if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_IHF )
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_IHF_STEREO )
			)
		{
			//Log_DebugPrintf(LOGID_AUDIO, "\t* auddrv_pwrOffUnusedSpkr() MIXER1 \n\r");
			//still need it.
			//chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
			//OSTASK_Sleep(150);
			chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRA_L, TRUE);
			//still need DAC voltage ref.
			//chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_L, TRUE);
		}

		if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_EP )
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_IHF_STEREO )
			)
		{
			Log_DebugPrintf(LOGID_AUDIO, "\t* auddrv_pwrOffUnusedSpkr() AUDDRV_SPKR_EP \n\r");
			//still need it.
			//chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
			//OSTASK_Sleep(150);
			chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRA_R, TRUE);
			//still need DAC voltage ref.
			//chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_R, TRUE);
		}
	}


	if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS )
		&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_LEFT )
		&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_RIGHT )
		)
	{
		//     chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
		//OSTASK_Sleep(150);
		chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRB_L, TRUE);
		chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRB_R, TRUE);
		chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_L, TRUE);
		//chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_R, TRUE);
	}
	else
	{
		if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_LEFT )
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS )
			)
		{
			//still need it.
			//chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
			//OSTASK_Sleep(150);
			chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRB_L, TRUE);
			//still need DAC voltage ref.
			//chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_L, TRUE);
		}

		if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_RIGHT )
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS )
			)
		{
			//still need it.
			//chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_DOWN || ANALOG_RAMP_PD );
			//OSTASK_Sleep(150);
			chal_audiospeaker_PwrdnDacDrv( spkr_handle, CHAL_AUDIO_SPKRB_R, TRUE);
			//still need DAC voltage ref.
			//chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_R, TRUE);
		}
	}

	//d2c at last.
	auddrv_pwrdn_d2c( TRUE );
}

static void auddrv_powerOnSpkr(	void )
{
	//d2c at first.
	auddrv_pwrdn_d2c( FALSE);

	if ( auddrv_spkrIsUsed( AUDDRV_SPKR_IHF_STEREO ) )
	{
		chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_L, FALSE);
		chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRA_L, FALSE);
		chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRA_R, FALSE);
		chal_audiospeaker_SetAnaPwrRamp(spkr_handle,  ANALOG_RAMP_UP );
		//OSTASK_Sleep(150);
	}
	else
	{
		if ( auddrv_spkrIsUsed( AUDDRV_SPKR_IHF ) )
		{
			//then enable digitla path
			chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_L, FALSE);
			chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRA_L, FALSE);
			chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_UP );
			//OSTASK_Sleep(150);
		}

		if ( auddrv_spkrIsUsed( AUDDRV_SPKR_EP ) )
		{
			chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRA_R, FALSE);
			chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRA_R, FALSE);
			chal_audiospeaker_SetAnaPwrRamp(spkr_handle, ANALOG_RAMP_UP );
			//OSTASK_Sleep(150);
		}
	}


	if ( auddrv_spkrIsUsed( AUDDRV_SPKR_HS ) )
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_spkr_driver_power - mixer34 - headsetON  *\n\r");
		chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_L, FALSE);
		chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRB_L, FALSE);
		chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRB_R, FALSE);
		chal_audiospeaker_SetAnaPwrRamp(spkr_handle,  ANALOG_RAMP_UP );
		//OSTASK_Sleep(150);
	}
	else
	{
		if ( auddrv_spkrIsUsed( AUDDRV_SPKR_HS_LEFT ) )
		{
			chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_L, FALSE);
			chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRB_L, FALSE);
			chal_audiospeaker_SetAnaPwrRamp(spkr_handle,  ANALOG_RAMP_UP );
			//OSTASK_Sleep(150);
		}

		if ( auddrv_spkrIsUsed( AUDDRV_SPKR_HS_RIGHT ) )
		{
			chal_audiospeaker_PwrdnDacRef( spkr_handle, CHAL_AUDIO_SPKRB_R, FALSE);
			chal_audiospeaker_PwrdnDacDrv(		 spkr_handle, CHAL_AUDIO_SPKRB_R, FALSE);
			chal_audiospeaker_SetAnaPwrRamp(spkr_handle,  ANALOG_RAMP_UP );
			//OSTASK_Sleep(150);
		}
	}
}

// Description:   DMIC power to mux with GPIO
static void auddrv_power_on_Dmic( Boolean  on )
{
/**
IOCR0.DIGMIC_MUX:  DIGMIC/GPIO[63:62] Select
0 DIGMICDATA/DIGMICCLK
1 GPIO[63:62]
*/
    Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_power_on_Dmic: Select DIGMIC_MUX*\n\r" );
	//*(volatile UInt32 *)0x08880000 &= ~(0x01000000); //clear bit 24 for Select DIGMIC_MUX
    SYSCFGDRV_Config_Pin_Mux(   SYSCFG_DIGMIC_GPIO_MUX_DIGMIC_SEL   );

	if(on)
	{
#if defined(THUNDERBIRD) 	
		//Thunderbird phone DMIC power is muxed with GPIO53
		//IOCR0.GPEN8_MUX, Selection concated with IOCR2[21]. 
		//{iocr2[21],iocr0[13]}
		//11 GPIO[53]
		//*(volatile UInt32 *)0x08880000 |= 0x02000;   // IOCR0.GPEN8_MUX
		//*(volatile UInt32 *)0x0888000C |= 0x0200000; // IOCR2.GPEN8_MUX_HI
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_power_on_Dmic: Select GPIO53*\n\r" );
        SYSCFGDRV_Config_Pin_Mux(   SYSCFG_GPEN8_GPIO53_MUX_GPIO53_SEL   );
		GPIODRV_Set_Mode( GPIO53, GPIO_DIR_OUTPUT );
		GPIODRV_Set_Bit( GPIO53, 1 );    
#else 
		//AthenaRay board uses GPIO51
		//IOCR0.SPI_MUX
		//Selection concated with bit 21. {21,11}
		//11 GPIO[51:48]
		*(volatile UInt32 *)0x08880000 |= 0x200800;  //IOCR0.SPI_MUX. IOCR0.SPI_MUX_HI
		*(volatile UInt32 *)0x0888002C |= 0x80000000; //IOCR7.RFGPIO5_MUX: 1 to select GPIO[51], not RFGPIO[5]
		GPIODRV_Set_Mode( GPIO51, GPIO_DIR_OUTPUT );
		GPIODRV_Set_Bit( GPIO51, 1 );    
#endif 

		// need analog mic power 
		/**
			Is analog mic power needed when enable digital mic?
			--------
			Yes its needed because the 26MHz clock comes from the Analog MIC.
			---------

			And the 26MHz clock comes after which register bit is set in the follow three?

			SYSCFG_ANACR2,	I_PGA_ADC_PWRUP
			SYSCFG_ANACR2,	I_MIC_VOICE_PWRDN
			DSP_AUDIO_AMCR_R,  AUDEN

			---------

			You can either set 

			SYSCFG_ANACR2,	I_PGA_ADC_PWRUP
			SYSCFG_ANACR2,	I_MIC_VOICE_PWRDN

			Or set

			DSP_AUDIO_AMCR_R,  AUDEN

			----
			**/ 
		chal_audiomic_PowerOnADC( mic_handle, TRUE );

		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_power_on_Dmic - DMIC on *\n\r");
		chal_audiomic_UseDmicClock3M( mic_handle, TRUE);
#if defined(THUNDERBIRD) 
		chal_audiomic_SetDmicPhase( mic_handle, FALSE);
#else
		chal_audiomic_SetDmicPhase( mic_handle, TRUE);
#endif

		// the below function should not directly control AMCR.auden bit
		chal_audiomic_EnableDmic( mic_handle, TRUE);
	}
	else
	{
#if defined(THUNDERBIRD) 	
		//Thunderbird phone DMIC power is muxed with GPIO53
		GPIODRV_Set_Bit( GPIO53, 0 );    
#else 
		//AthenaRay board uses GPIO51
		GPIODRV_Set_Bit( GPIO51, 0 );    
#endif 
	}
}	

static void auddrv_pwrOnMic(
				AUDDRV_MIC_Enum_t		mic,
				AUDIO_SAMPLING_RATE_t   sample_rate
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrOnMic - %d *\n\r", mic);

	switch(mic)
	{
	    case AUDDRV_MIC_ANALOG_MAIN:
	    case AUDDRV_MIC_ANALOG_AUX:
	    case AUDDRV_DUAL_MIC_ANALOG_DIGI1:
	    case AUDDRV_DUAL_MIC_DIGI1_ANALOG:
			//rule #1
			chal_audiomic_PowerOnADC( mic_handle, TRUE );
			//anacr0:d2c, turn on the main d2c for mic.
			auddrv_pwrdn_d2c( FALSE );  //turn on clock first, then turn on D2C.
	    default:
			//it seems for DMIC alone recording, D2C is not on so no sound is recorded
			//ANACR0 =0x000060ff (bit0 = 1)
			auddrv_pwrdn_d2c( FALSE );  //turn on D2C.

		    break;
    }
		    
	switch(mic)
	{
	    case AUDDRV_MIC_ANALOG_MAIN:
		    chal_audiomic_SetAuxBiasLevelHigh( mic_handle, FALSE);
		    chal_audiomic_SetAuxBiasContinuous( mic_handle, FALSE);
		    break;
  
	    case AUDDRV_MIC_ANALOG_AUX:
		    chal_audiomic_SetAuxBiasLevelHigh( mic_handle, TRUE);
		    chal_audiomic_SetAuxBiasContinuous( mic_handle, TRUE );
		    break;
  
	    case AUDDRV_MIC_PCM_IF:  
		    //need AMCR PCM on
			chal_audiopcmif_Enable(pcmif_handle, TRUE);

		    break;
  
	    case AUDDRV_MIC_DIGI1:
	    case AUDDRV_MIC_DIGI2:
	    case AUDDRV_DUAL_MIC_DIGI12:
	    case AUDDRV_DUAL_MIC_DIGI21:
			auddrv_power_on_Dmic( TRUE );
		    break;
  
	    case AUDDRV_DUAL_MIC_ANALOG_DIGI1:
	    case AUDDRV_DUAL_MIC_DIGI1_ANALOG:
			//(aux mic can not be part of dual-mic with digi mic)
		    chal_audiomic_SetAuxBiasLevelHigh( mic_handle, FALSE);
		    chal_audiomic_SetAuxBiasContinuous( mic_handle, FALSE);

			auddrv_power_on_Dmic( TRUE );
		    break;
  
	    default:
		    break;
    }
	micPoweredOn[mic]= TRUE;
}

static Boolean auddrv_mic_not_used( AUDDRV_MIC_Enum_t   mic )
{
	if (voice_mic_current != mic && audio_mic_current != mic )
		return TRUE;
	else
		return FALSE;
}

static Boolean auddrv_analog_mic_not_used( void )
{
	if (   voice_mic_current != AUDDRV_MIC_ANALOG_MAIN               && audio_mic_current != AUDDRV_MIC_ANALOG_MAIN
		&& voice_mic_current != AUDDRV_MIC_ANALOG_AUX                && audio_mic_current != AUDDRV_MIC_ANALOG_AUX
		&& voice_mic_current != AUDDRV_DUAL_MIC_ANALOG_DIGI1  && audio_mic_current != AUDDRV_DUAL_MIC_ANALOG_DIGI1
		&& voice_mic_current != AUDDRV_DUAL_MIC_DIGI1_ANALOG  && audio_mic_current != AUDDRV_DUAL_MIC_DIGI1_ANALOG
		)
		return TRUE;
	else
		return FALSE;
}

  //check voice_mic_current and audio_mic_current, power off un-used mic.
static void auddrv_pwrOffUnusedMic( void )
{
	//rule #4
	auddrv_pwrdn_d2c( TRUE );

	//main mic
	if ( auddrv_mic_not_used( AUDDRV_MIC_ANALOG_MAIN ) && micPoweredOn[AUDDRV_MIC_ANALOG_MAIN]== TRUE )
	{
		if( auddrv_analog_mic_not_used() )
		{
			//Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrOffUnusedMic - main *\n\r");
			//rule #4
			chal_audiomic_PowerOnADC( mic_handle, FALSE );
			micPoweredOn[ AUDDRV_MIC_ANALOG_MAIN ]= FALSE;
		}
	}

	//aux mic  (aux mic can not be part of dual-mic with digi mic)
	if ( auddrv_mic_not_used( AUDDRV_MIC_ANALOG_AUX ) && micPoweredOn[AUDDRV_MIC_ANALOG_AUX]== TRUE )
	{
		if( auddrv_analog_mic_not_used() )
		{
			//rule #4
			chal_audiomic_PowerOnADC( mic_handle, FALSE );
			//Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrOffUnusedMic - aux *\n\r");
			chal_audiomic_SetAuxBiasLevelHigh( mic_handle, FALSE);
			chal_audiomic_SetAuxBiasContinuous( mic_handle, FALSE);
			micPoweredOn[ AUDDRV_MIC_ANALOG_AUX ]= FALSE;
		}
	}

	//case AUDDRV_MIC_PCM_IF: 
	// always turn off AMCR PCM interface ??
	chal_audiopcmif_Enable(pcmif_handle, FALSE);
	// need to consider if under BT call and recording off case

	//digi mic
	if ( auddrv_mic_not_used( AUDDRV_MIC_DIGI1 ) && micPoweredOn[AUDDRV_MIC_DIGI1]== TRUE
		&& auddrv_mic_not_used( AUDDRV_MIC_DIGI2 ) && micPoweredOn[AUDDRV_MIC_DIGI2]== TRUE
		&& auddrv_mic_not_used( AUDDRV_DUAL_MIC_DIGI12 ) && micPoweredOn[AUDDRV_DUAL_MIC_DIGI12]== TRUE
		&& auddrv_mic_not_used( AUDDRV_DUAL_MIC_DIGI21 ) && micPoweredOn[AUDDRV_DUAL_MIC_DIGI21]== TRUE
		&& auddrv_mic_not_used( AUDDRV_DUAL_MIC_ANALOG_DIGI1 ) && micPoweredOn[AUDDRV_DUAL_MIC_ANALOG_DIGI1]== TRUE
		&& auddrv_mic_not_used( AUDDRV_DUAL_MIC_DIGI1_ANALOG ) && micPoweredOn[AUDDRV_DUAL_MIC_DIGI1_ANALOG]== TRUE
		)
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrOffUnusedMic - digi *\n\r");
		//in AthenaRay / Thunderbird phone, digi mic 1 and digi mic2 are powered by the same GPIO pin.
		auddrv_power_on_Dmic( FALSE );

		// AMCR ?? need to manage AMCR[5] separately
		chal_audiomic_EnableDmic( mic_handle, FALSE);
		if( auddrv_analog_mic_not_used() )
		{
			// AMCR[5] can be set to 0
		}
		micPoweredOn[ AUDDRV_MIC_DIGI1 ]= FALSE;
		micPoweredOn[ AUDDRV_MIC_DIGI2 ]= FALSE;
		micPoweredOn[ AUDDRV_DUAL_MIC_DIGI12 ]= FALSE;
		micPoweredOn[ AUDDRV_DUAL_MIC_DIGI21 ]= FALSE;
		micPoweredOn[ AUDDRV_DUAL_MIC_ANALOG_DIGI1 ]= FALSE;
		micPoweredOn[ AUDDRV_DUAL_MIC_DIGI1_ANALOG ]= FALSE;
	}
}

static void auddrv_select_mic_input(
				AUDDRV_InOut_Enum_t      input_path,
				AUDDRV_MIC_Enum_t        mic_selection
				)
{

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_select_mic_input, input_path=%d, mic=%d *\n\r", input_path, mic_selection);
	switch(input_path) 
	{
		case AUDDRV_VOICE_INPUT:
			switch(mic_selection)
			{
				case AUDDRV_MIC_ANALOG_MAIN:
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1 );
					break;
				case AUDDRV_MIC_ANALOG_AUX:
					chal_audiomic_SelectInput( mic_handle, 1 );
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1 );
					break;

				case AUDDRV_MIC_DIGI1:
					//DIGI mic selection, mono
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;
				case AUDDRV_MIC_DIGI2:
					//DIGI mic selection, mono
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2 ); 
					break;

				case AUDDRV_DUAL_MIC_ANALOG_DIGI1:
					//dual mic with analog and DIGI
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1 );

					Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* CHAL_AUDIO_MIC_INPUT_DMIC1 = %d *\n\r", CHAL_AUDIO_MIC_INPUT_DMIC1);
					//DIGI mic selection
					chal_audiovipath2_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;
				case AUDDRV_DUAL_MIC_DIGI1_ANALOG:
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1 );
					//select DIGI mic2 on voice input path2
					chal_audiovipath2_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2 );
					break;

				case AUDDRV_DUAL_MIC_DIGI12:
					//dual DIGI mic selection
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1 ); 
					chal_audiovipath2_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2 );
					break;
				case AUDDRV_DUAL_MIC_DIGI21:
					//dual DIGI mic selection
					chal_audiovipath_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2 ); 
					chal_audiovipath2_SelectInput( vipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;

				case AUDDRV_MIC_PCM_IF: //done in vdriver
					// trun on AMCR PCM interface
					chal_audiopcmif_Enable(pcmif_handle, TRUE);
					break;

				default:
					//error if reach here
					break;
			}			
			break;

		case AUDDRV_AUDIO_INPUT:  //HQ Audio input
			switch(mic_selection)
			{
				case AUDDRV_MIC_ANALOG_MAIN:
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1, CHAL_AUDIO_MIC_INPUT_ADC1 );
					break;
				case AUDDRV_MIC_ANALOG_AUX:
					chal_audiomic_SelectInput( mic_handle, 1 );
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1, CHAL_AUDIO_MIC_INPUT_ADC1 );
					break;

				case AUDDRV_MIC_DIGI1:
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;
				case AUDDRV_MIC_DIGI2:
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2, CHAL_AUDIO_MIC_INPUT_DMIC2 );
					break;

				case AUDDRV_DUAL_MIC_DIGI12:
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC1, CHAL_AUDIO_MIC_INPUT_DMIC2 );
					break;
				case AUDDRV_DUAL_MIC_DIGI21:
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_DMIC2, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;

				case AUDDRV_DUAL_MIC_ANALOG_DIGI1:
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1, CHAL_AUDIO_MIC_INPUT_DMIC1 );
					break;
				case AUDDRV_DUAL_MIC_DIGI1_ANALOG:
					chal_audiomic_SelectInput( mic_handle, 0 );
					chal_audioaipath_SelectInput( aipath_handle, CHAL_AUDIO_MIC_INPUT_ADC1, CHAL_AUDIO_MIC_INPUT_DMIC2 );
					break;

				default:
					//error if reach here
					break;
			}			
			break;

		default:
			break;
	}
}

static void auddrv_pwrdn_d2c ( Boolean  power_down )
{
	static int pwrdn_d2c_anancr0, pwrdn_d2c_anancr10;

	if( power_down == FALSE )  //power up d2c
	{
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
 //do not enter this function at CP.
		assert(0); 
#else

		//enable clock
		//*(volatile UInt32 *)0x08140140 &= ~(0x0001);
		//*(volatile UInt32 *)0x08140140 &= ~(0x0020);
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrdn_d2c: Enable DAC/ADC Clock*\n\r" );

#ifdef UNDER_LINUX
        OSDAL_CLK_Start(hclk_adc);
        OSDAL_CLK_Start(hclk_dac);
#else
        CLKDRV_Start_Clock(  clk_handle, CLK_AUDIO_TX_DAC_CLOCK  );
        CLKDRV_Start_Clock(  clk_handle, CLK_AUDIO_RX_ADC_CLOCK  );
        
#endif

		//when turn on DSP Audio clock (d2c), it is time for ARM to enable access audio HW:
		//*((volatile UInt32 *)(AHB_TL3R_TL3_A2D_ACCESS_EN_R)) |= AHB_TL3R_TL3_A2D_ACCESS_EN_R_arm2dsp_peripheral_access_en;
#endif

		if ( auddrv_spkrIsUsed( AUDDRV_SPKR_HS_LEFT ) 
			|| auddrv_spkrIsUsed( AUDDRV_SPKR_HS_RIGHT ) 
			|| auddrv_spkrIsUsed( AUDDRV_SPKR_HS )
		 )
		{
			pwrdn_d2c_anancr10 = 0;  //need to power on.
		}
		else
		{
			pwrdn_d2c_anancr10 = 1;  //need to power off.
		}

		pwrdn_d2c_anancr0 = 0;  //need to power on (for mic and IHF/EP DAC.)

	}
	else  //power down
	{
		if ( FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_LEFT ) 
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS_RIGHT ) 
			&& FALSE==auddrv_spkrIsUsed( AUDDRV_SPKR_HS )
		 )
		{
			pwrdn_d2c_anancr10 = 1;  //need to power down.
		}
		else
		{
			pwrdn_d2c_anancr10 = 0;   //need to power up.
		}

		if ( FALSE == D2C_InUse
			&& voice_mic_current == AUDDRV_MIC_NONE
			&& audio_mic_current == AUDDRV_MIC_NONE 
			&& voiceSpkr1 == AUDDRV_SPKR_NONE
			&& voiceSpkr2 == AUDDRV_SPKR_NONE
			)
		{
			pwrdn_d2c_anancr0 = 1;  //can power down
		}
		else
		{
			pwrdn_d2c_anancr0 = 0;  //need to keep it powered-up
			Log_DebugPrintf(LOGID_AUDIO, "\n\r auddrv_pwrdn_d2c %d, %d, %d, %d, %d, %d, %d, %d *d2cinuse %d\n\r", 
					bInVoiceCall,
					aopath_enabled , popath_enabled , vopath_enabled, 
					vipath_enabled , aipath_enabled , btwb_enabled , btnb_enabled,
					D2C_InUse );
		}
	}

	chal_audiospeaker_PwrdnD2c( spkr_handle, CHAL_AUDIO_SPKRB_L, pwrdn_d2c_anancr10 );
	chal_audiospeaker_PwrdnD2c( spkr_handle, CHAL_AUDIO_SPKRA_L, pwrdn_d2c_anancr0 );
	//Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrdn_d2c ANACR10 %d *\n\r", pwrdn_d2c_anancr10 );
	//Log_DebugPrintf(LOGID_AUDIO, "auddrv_pwrdn_d2c ANACR0 %d \n\r", pwrdn_d2c_anancr0 );

//		case AUDDRV_SPKR_PCM_IF:  //??
//		case AUDDRV_SPKR_USB_IF:

	if( pwrdn_d2c_anancr0==1 && pwrdn_d2c_anancr10==1 )
	{
		//disable audio DAC/ADC clock
		//*(volatile UInt32 *)0x08140140 |= 0x0020;
		//*(volatile UInt32 *)0x08140140 |= 0x01;
        Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_pwrdn_d2c: Disable DAC/ADC Clock*\n\r" );
#ifdef UNDER_LINUX
        OSDAL_CLK_Stop(hclk_adc);
        OSDAL_CLK_Stop(hclk_dac);
#else
        CLKDRV_Stop_Clock(  clk_handle, CLK_AUDIO_RX_ADC_CLOCK  );
        CLKDRV_Stop_Clock(  clk_handle, CLK_AUDIO_TX_DAC_CLOCK  );
#endif        

	}
}

static void auddrv_mixer_add_input(
				AUDDRV_InOut_Enum_t  input_path_to_mixer,
				AUDDRV_SPKR_Enum_t   mixer_speaker_selection,
				AUDIO_CHANNEL_NUM_t  input_to_mixer
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_add_input input_path_to_mixer %d, mixer %d, chanl %d \n\r", 
		input_path_to_mixer, 
		mixer_speaker_selection,
		input_to_mixer );

	switch(input_path_to_mixer)
	{
	case AUDDRV_VOICE_OUTPUT:
		switch(mixer_speaker_selection)
		{
		case AUDDRV_SPKR_IHF:
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_VO);
			break;
		
		case AUDDRV_SPKR_EP:
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_VO);
			break;
		
		case AUDDRV_SPKR_IHF_STEREO:
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_VO);
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_VO);
			break;
		
		case AUDDRV_SPKR_HS_LEFT:
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_VO);
			break;

		case AUDDRV_SPKR_HS_RIGHT:
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_VO);
			break;

		case AUDDRV_SPKR_HS:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* auddrv_mixer_add_input mixer to headset  *\n\r" );
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_VO);
			chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_VO);
			break;

		default:
			break;
		}
		break;

	case AUDDRV_AUDIO_OUTPUT:
		switch(mixer_speaker_selection)
		{
		case AUDDRV_SPKR_IHF:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AR);


               
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;
		
		case AUDDRV_SPKR_EP:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;
		
		case AUDDRV_SPKR_IHF_STEREO:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;
		
		case AUDDRV_SPKR_HS_LEFT:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;

		case AUDDRV_SPKR_HS_RIGHT:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;

		case AUDDRV_SPKR_HS:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
			}
			break;

		default:
			break;
		}
		break;

	case AUDDRV_RINGTONE_OUTPUT:
		switch(mixer_speaker_selection)
		{
		case AUDDRV_SPKR_IHF:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;
		
		case AUDDRV_SPKR_EP:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;
		
		case AUDDRV_SPKR_IHF_STEREO:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_RINGTONE_OUTPUT, auddrv_mixer_add_input AUDDRV_SPKR_IHF_STEREO input_path_to_mixer %d, mixer %d, chanl %d \n\r", 
					input_path_to_mixer, 
					mixer_speaker_selection,
					input_to_mixer );

				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR); //
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;
		
		case AUDDRV_SPKR_HS_LEFT:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;

		case AUDDRV_SPKR_HS_RIGHT:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;

		case AUDDRV_SPKR_HS:
			if (input_to_mixer==AUDIO_CHANNEL_STEREO || input_to_mixer==AUDIO_CHANNEL_MONO)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_LEFT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
			}
			else
			if (input_to_mixer==AUDIO_CHANNEL_STEREO_RIGHT)
			{
				chal_audiomixer_AddInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

static void auddrv_mixer_remove_input( 
				AUDDRV_InOut_Enum_t  input_path_to_mixer,
				AUDDRV_SPKR_Enum_t   mixer_speaker_selection
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input input_path_to_mixer %d, mixer %d, \n\r", 
		input_path_to_mixer, mixer_speaker_selection);

	switch(input_path_to_mixer)
	{
		case AUDDRV_VOICE_OUTPUT:
			switch(mixer_speaker_selection)
			{
				case AUDDRV_SPKR_IHF:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_VO);
					break;
				
				case AUDDRV_SPKR_EP:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_VO);
					break;
				
				case AUDDRV_SPKR_IHF_STEREO:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_VO);
					break;
				
				case AUDDRV_SPKR_HS_LEFT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_VO);
					break;

				case AUDDRV_SPKR_HS_RIGHT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_VO);
					break;

				case AUDDRV_SPKR_HS:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_VO);
					break;

				default:
					break;
			}

			break;

		case AUDDRV_AUDIO_OUTPUT:
			switch(mixer_speaker_selection)
			{
				case AUDDRV_SPKR_IHF:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AR);
					break;
				
				case AUDDRV_SPKR_EP:
					Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input AUDDRV_AUDIO_OUTPUT AA, \n\r"); 
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
					break;
				
				case AUDDRV_SPKR_IHF_STEREO:
					Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input AUDDRV_AUDIO_OUTPUT BB, \n\r"); 
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AR);
					
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
					break;
				
				case AUDDRV_SPKR_HS_LEFT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AR);
					break;

				case AUDDRV_SPKR_HS_RIGHT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
					break;

				case AUDDRV_SPKR_HS:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AR);

					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
					break;

				default:
					break;
			}

			break;

		case AUDDRV_RINGTONE_OUTPUT:
			switch(mixer_speaker_selection)
			{
				case AUDDRV_SPKR_IHF:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PR);
					break;
				
				case AUDDRV_SPKR_EP:
					Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input AUDDRV_RINGTONE_OUTPUT AA, \n\r"); 
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
					break;
				
				case AUDDRV_SPKR_IHF_STEREO:
					Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input AUDDRV_RINGTONE_OUTPUT BB, \n\r"); 
					Log_DebugPrintf(LOGID_AUDIO, "auddrv_mixer_remove_input input_path_to_mixer %d, mixer %d, \n\r", 
						input_path_to_mixer, mixer_speaker_selection);

					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PR);

					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
					break;
				
				case AUDDRV_SPKR_HS_LEFT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PR);
					break;

				case AUDDRV_SPKR_HS_RIGHT:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
					break;

				case AUDDRV_SPKR_HS:
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PR);

					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
					break;

				default:
					break;
			}

			break;

		default:
			break;
	}
}

static void auddrv_mixer_remove_all_inputs()
{
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_VO);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_AR);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_AR);
				
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_AR);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_AR);
		
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC1, CHAL_AUDIO_MIXER_INPUT_PR);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC2, CHAL_AUDIO_MIXER_INPUT_PR);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC3, CHAL_AUDIO_MIXER_INPUT_PR);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PL);
					chal_audiomixer_RemoveInput(mixer_handle, CHAL_AUDIO_MIXER_DAC4, CHAL_AUDIO_MIXER_INPUT_PR);
		
}

//=============================================================================
//
// Function Name: get_CHAL_AUDIO_MIXER_en
//
// Description:   a utility function
//
//=============================================================================
static CHAL_AUDIO_MIXER_en get_CHAL_AUDIO_MIXER_en( AUDDRV_SPKR_Enum_t mixer)
{
	CHAL_AUDIO_MIXER_en chal_mixer = CHAL_AUDIO_MIXER_DAC1;

	switch(mixer)
	{
		case AUDDRV_SPKR_IHF:
			chal_mixer = CHAL_AUDIO_MIXER_DAC1;
			break;

		case AUDDRV_SPKR_EP:
			chal_mixer = CHAL_AUDIO_MIXER_DAC2;
			break;

		case AUDDRV_SPKR_HS_LEFT:
			chal_mixer = CHAL_AUDIO_MIXER_DAC3;
			break;

		case AUDDRV_SPKR_HS_RIGHT:
			chal_mixer = CHAL_AUDIO_MIXER_DAC4;
			break;
    }
	return chal_mixer;
}
