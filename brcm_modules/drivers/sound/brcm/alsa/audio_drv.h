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

*    @file   audio_drv.h

*    @brief  This file contains the interfaces to control external audio devices.

**/
/****************************************************************************/
/**

*   @defgroup   AudioExternalDevCtrlGroup   External Audio Devices Control API
*   @ingroup    AudioDrvGroup
*	@ingroup	DeviceDriverGroup
*
*   @brief      This group defines the interfaces to control external audio devices.
*
*	The External Audio Device Control API provides interfaces to control external
    audio devices.  The external audio devices consist of audio DAC, switch, Amp
    that are not in Broadcom baseband chip.  Some phone designs does not use external
	audio devices at all, then audio_drv_dummy.i can be used in image build.
*/

#ifndef _AUDIO_DRV_H_
#define _AUDIO_DRV_H_

#include <linux/stddef.h>
#include "hal_audio.h"
#include "hal_audio_config.h"

//REGULATOR Driver framework API's
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

//#if defined(CONFIG_AMP_MAX9877) || (CONFIG_AMP_MAX9877_MODULE)
//#ifdef EXTERNAL_SPK_MAX9877 /* ekko.080609 */

#if defined(CONFIG_AMP_NCP2704) 

enum {
    AMMP_OUT_NONE = 0,
	AMMP_OUT_IN1A_IN1B_SPK,
	AMMP_OUT_IN1A_LEFT_HP_IN1B_RIGHT_HP,
	AMMP_OUT_IN1A_IN1B_SPK_IN1A_LEFT_HP_IN1B_RIGHT_HP,
	AMMP_OUT_IN2DIFF_SPK,
	AMMP_OUT_IN2DIFF_LEFT_HP_IN2DIFF_RIGHT_HP,
	AMMP_OUT_IN2DIFF_SPK_IN2DIFF_LEFT_HP_IN2RIGHT_HP,
	AMMP_OUT_IN1A_INA1B_SPK_IN2DIFF_LEFT_HP_IN2DIFF_RIGHT_HP=10,
	AMMP_OUT_IN2DIFF_SPK_IN1A_LEFT_HP_IN1B_RIGHT_HP,
};
#else
enum {
	AMMP_ZCD_OFF,
	AMMP_ZCD_ON,
	AMMP_INA_OFF = 0,
	AMMP_INA_ON,
	AMMP_INB_OFF = 0,
	AMMP_INB_ON,
	AMMP_AMP_SHUTDOWN = 0,
	AMMP_AMP_OPERATE,
	AMMP_BYPASS_OFF = 0,
	AMMP_BYPASS_ON,
	AMMP_PGA_00DB = 0,
	AMMP_PGA_09DB,
	AMMP_PGA_20DB,
	AMMP_OSC_00 = 0,
	AMMP_OSC_01,
	AMMP_OSC_02,
	AMMP_OSC_03,
	AMMP_OUT_NONE = 0,
	AMMP_OUT_A_SPK,
	AMMP_OUT_A_LEFT_HP_RIGHT_HP,
	AMMP_OUT_A_SPK_LEFT_HP_RIGHT_HP,
	AMMP_OUT_B_SPK,
	AMMP_OUT_B_LEFT_HP_RIGHT_HP,
	AMMP_OUT_B_SPK_LEFT_HP_RIGHT_HP,
	AMMP_OUT_AB_SPK,
	AMMP_OUT_AB_LEFT_HP_RIGHT_HP,
	AMMP_OUT_AB_SPK_LEFT_HP_RIGHT_HP,
};

#endif
#if defined(CONFIG_AMP_NCP2704) 

typedef enum {
	NCP2704_SPKVOL_MUTE  = 0,
	NCP2704_SPKVOL_N60DB = 1,	// -60dB
	NCP2704_SPKVOL_N54DB = 2,	// -54dB
	NCP2704_SPKVOL_N48DB = 3,	// -48dB
	NCP2704_SPKVOL_N45DB = 4,	// -45dB
	NCP2704_SPKVOL_N42DB = 5,	// -42dB
	NCP2704_SPKVOL_N39DB = 6,	// -39dB
	NCP2704_SPKVOL_N36DB = 7,	// -36dB
	NCP2704_SPKVOL_N34DB = 8,	// -34dB
	NCP2704_SPKVOL_N32DB = 9,	// -32dB
	NCP2704_SPKVOL_N30DB = 10,	// -30dB
	NCP2704_SPKVOL_N28DB = 11,	// -28dB
	NCP2704_SPKVOL_N27DB = 12,	// -27dB
	NCP2704_SPKVOL_N26DB = 13,	// -26dB
	NCP2704_SPKVOL_N25DB = 14,	// -25dB
	NCP2704_SPKVOL_N24DB = 15,	// -24dB
	NCP2704_SPKVOL_N23DB = 16,	// -23dB
	NCP2704_SPKVOL_N22DB = 17,	// -22dB
	NCP2704_SPKVOL_N21DB = 18,	// -21dB
	NCP2704_SPKVOL_N20DB = 19,	// -20dB
	NCP2704_SPKVOL_N19DB = 20,	// -19dB
	NCP2704_SPKVOL_N18DB = 21,	// -18dB
	NCP2704_SPKVOL_N17DB = 22,	// -17dB
	NCP2704_SPKVOL_N16DB = 23,	// -16dB
	NCP2704_SPKVOL_N15DB = 24,	// -15dB
	NCP2704_SPKVOL_N14DB = 25,	// -14dB
	NCP2704_SPKVOL_N13DB = 26,	// -13dB
	NCP2704_SPKVOL_N12DB = 27,	// -12dB
	NCP2704_SPKVOL_N11DB = 28,	// -11dB
	NCP2704_SPKVOL_N10DB = 29,	// -10dB
	NCP2704_SPKVOL_N09DB = 30,	// -9dB
	NCP2704_SPKVOL_N08DB = 31,	// -8dB
	NCP2704_SPKVOL_N07DB = 32,	// -7dB
	NCP2704_SPKVOL_N06DB = 33,	// -6dB
	NCP2704_SPKVOL_N05DB = 34,	// -5dB
	NCP2704_SPKVOL_N04DB = 35,	// -4dB
	NCP2704_SPKVOL_N03DB = 36,	// -3dB
	NCP2704_SPKVOL_N02DB = 37,	// -2dB
	NCP2704_SPKVOL_N01DB = 38,	// -1dB
	NCP2704_SPKVOL_N00DB = 39,	// 0dB
	NCP2704_SPKVOL_P01DB = 40,	// +1dB
	NCP2704_SPKVOL_P02DB = 41,	// +2dB
	NCP2704_SPKVOL_P03DB = 42,	// +3dB
	NCP2704_SPKVOL_P04DB = 43,	// +4dB
	NCP2704_SPKVOL_P05DB = 44,	// +5dB
	NCP2704_SPKVOL_P06DB = 45,	// +6dB
	NCP2704_SPKVOL_P07DB = 46,	// +7dB
	NCP2704_SPKVOL_P08DB = 47,	// +8dB
	NCP2704_SPKVOL_P09DB = 48,	// +9dB
	NCP2704_SPKVOL_P10DB = 49,	// +10dB
	NCP2704_SPKVOL_P11DB = 50,	// +11dB
	NCP2704_SPKVOL_P12DB = 51,	// +12dB
	NCP2704_SPKVOL_INVALID
} AudioExtSpkVolNCP2704_t;
#else
typedef enum {
	MAX9877_SPKVOL_MUTE = 0,
	MAX9877_SPKVOL_N75DB = 1,	// -75dB
	MAX9877_SPKVOL_N71DB = 2,	// -71dB
	MAX9877_SPKVOL_N67DB = 3,	// -67dB
	MAX9877_SPKVOL_N63DB = 4,	// -63dB
	MAX9877_SPKVOL_N59DB = 5,	// -59dB
	MAX9877_SPKVOL_N55DB = 6,	// -55dB
	MAX9877_SPKVOL_N51DB = 7,	// -51dB
	MAX9877_SPKVOL_N47DB = 8,	// -47dB
	MAX9877_SPKVOL_N44DB = 9,	// -44dB
	MAX9877_SPKVOL_N41DB = 10,	// -41dB
	MAX9877_SPKVOL_N38DB = 11,	// -38dB
	MAX9877_SPKVOL_N35DB = 12,	// -35dB
	MAX9877_SPKVOL_N32DB = 13,	// -32dB
	MAX9877_SPKVOL_N29DB = 14,	// -29dB
	MAX9877_SPKVOL_N26DB = 15,	// -26dB
	MAX9877_SPKVOL_N23DB = 16,	// -23dB
	MAX9877_SPKVOL_N21DB = 17,	// -21dB
	MAX9877_SPKVOL_N19DB = 18,	// -19dB
	MAX9877_SPKVOL_N17DB = 19,	// -17dB
	MAX9877_SPKVOL_N15DB = 20,	// -15dB
	MAX9877_SPKVOL_N13DB = 21,	// -13dB
	MAX9877_SPKVOL_N11DB = 22,	// -11dB
	MAX9877_SPKVOL_N09DB = 23,	// -9dB
	MAX9877_SPKVOL_N07DB = 24,	// -7dB
	MAX9877_SPKVOL_N06DB = 25,	// -6dB
	MAX9877_SPKVOL_N05DB = 26,	// -5dB
	MAX9877_SPKVOL_N04DB = 27,	// -4dB
	MAX9877_SPKVOL_N03DB = 28,	// -3dB
	MAX9877_SPKVOL_N02DB = 29,	// -2dB
	MAX9877_SPKVOL_N01DB = 30,	// -1dB
	MAX9877_SPKVOL_N00DB = 31,	// 0dB
	MAX9877_SPKVOL_INVALID
} AudioExtSpkVolMAX9877_t;
#endif


#if defined(CONFIG_AMP_NCP2704)
typedef enum {
	NCP2704_PGAIN_P00DB = 0,	// 0dB
	NCP2704_PGAIN_P09DB = 1,	// +9dB
	NCP2704_PGAIN_P20DB = 2,	// +20dB
	NCP2704_PGAIN_RESERVED = 3,	// Reserved
	NCP2704_PGAIN_INVALID = NCP2704_PGAIN_RESERVED
} AudioExtPreGainNCP2704_t;

enum audio_ext_output {
	NCP2704_OUTPUT_LOUDSPK = 0, // Loud speaker
    NCP2704_OUTPUT_HANDSET = 1, // Handset output
    NCP2704_OUTPUT_HEADSET = 2, // Headset output
    NCP2704_OUTPUT_INVALID
	};
#else

typedef enum {
	MAX9877_PGAIN_P00DB = 0,	// 0dB
	MAX9877_PGAIN_P09DB = 1,	// +9dB
	MAX9877_PGAIN_P20DB = 2,	// +20dB
	MAX9877_PGAIN_RESERVED = 3,	// Reserved
	MAX9877_PGAIN_INVALID = MAX9877_PGAIN_RESERVED
} AudioExtPreGainMAX9877_t;

enum audio_ext_output {
	MAX9877_OUTPUT_LOUDSPK = 0,	// Loud speaker
	MAX9877_OUTPUT_HANDSET = 1,	// Handset output
	MAX9877_OUTPUT_HEADSET = 2,	// Headset output
	MAX9877_OUTPUT_HEADSET_AND_LOUDSPK = 3, //spk+headset
	MAX9877_OUTPUT_INVALID
};

#endif



#if defined(CONFIG_AMP_NCP2704)
int ncp2704_setoutput_mode(enum audio_ext_output output);
#else
int max9877_setoutput_mode(enum audio_ext_output output);
#endif

/* taeuk7.park 20081222 add main function for audio timer */
void audioextdev_poweronmain(void);
void audioextdev_gpiosettingmain(UInt8 v_timer);

// ParkTaeuk
void testplaybackCB(UInt32 nID, UInt32 eType, UInt32 parm1, UInt32 parm2);

void AudioExtDev_SetBiasOnOff(AudioMode_t audioMode, Boolean OnOff);
/**
*	@brief	Power off external audio device.
*/
void audioextdev_poweroff(void);

/**
*	@brief	Power on external audio device.
*
*/
void audioextdev_powerOn(AudioMode_t audio_mode);

/**
 * @addtogroup AudioExternalDevCtrlGroup
 * @{
 */

typedef enum {
	AudioExtDev_Fast_PowerUp,	///< (depending on PCB design) usually 50 ms.
	AudioExtDev_Slow_PowerUp	///< (depending on PCB design) usually 500 ms.
} AudioExtDev_PowerUp_Speed_t;

/**
*	@brief	Write external audio device register
*/
void AudioExtDev_WriteReg(UInt8 register_add, UInt16 value);

/**
*	@brief	Initialize external audio device driver.
*/
void AudioExtDev_Init(void);

/**
*	@brief	Power on external audio device.
*
	@param	pathCfgPtr	how fast the audio device will ramp up to its VMID voltage.
*/
void AudioExtDev_PowerOn(AudioMode_t amode);

/**
*	@brief  Set input format
*
	@param	format	1/0: differential/single_ended
*/

void AudioExtDev_SetInputFormat(UInt32 format);

/**
*	@brief  Set bypass mode
*
	@param	bypass
*/

void AudioExtDev_SetBypass(UInt32 bypass);

/**
*	@brief	Power off external audio device.
*/
void AudioExtDev_PowerOff(void);	//VMID/FREF are off.

/**
*	@brief	Set speaker volume
*
	@param	extVol		speaker volume, range from 0 to 40.
						0   mute,
						1   minimum,
						40  max.
*/
void AudioExtDev_SetVolume(UInt8 extVol);

/**
 * 	@brief Set pre gain of INA and INB to <= +20DB
 *
 * 	@param pregain [ 0  +20DB ]
 */
#if defined(CONFIG_AMP_NCP2704)
void AudioExtDev_SetPreGain(AudioExtPreGainNCP2704_t pregain);
#else
void AudioExtDev_SetPreGain(AudioExtPreGainMAX9877_t pregain);
#endif

/**
 * 	@brief Get the pre gain value set
 *
 * 	@param none
 * */
#if defined(CONFIG_AMP_NCP2704)
AudioExtPreGainNCP2704_t  AudioExtDev_GetPreGain();
#else
AudioExtPreGainMAX9877_t  AudioExtDev_GetPreGain();
#endif

/** @} */

Boolean AudioExtDev_IsReady(void);
void AudioExtDev_SetSampleRate(AudioSampleRate_en_t sample_rate);
void AudioExtDev_SetBiasOnOff(AudioMode_t audioMode, Boolean OnOff);

struct regulator* bcm59035_spkr_regulator();

/**
 * 	@brief	Enables and disables the PMU speaker vdd
 *
 * 	@param none
 */
void PMU_AMPVDD(Boolean amp_on);
#endif
