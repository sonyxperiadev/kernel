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
* @file   chal_audiospeaker.h
* @brief  Athena Audio cHAL interface to onchip speaker driver
*
*****************************************************************************/
#ifndef _CHAL_AUDIOSPEAKER_H_
#define _CHAL_AUDIOSPEAKER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

typedef enum
{
	CHAL_AUDIO_SPKRA_L,	  ///< TOP_DAC_CORE_100_100_ATHENA(earpiece/speaker), Left channel 
	CHAL_AUDIO_SPKRA_R,	  ///< TOP_DAC_CORE_100_100_ATHENA(earpiece/speaker), Right channel 
	CHAL_AUDIO_SPKRB_L,	  ///< DAC_TOP_CABLE(headset), Left channel 
	CHAL_AUDIO_SPKRB_R,	  ///< DAC_TOP_CABLE(headset), Right channel 
} CHAL_AUDIO_SPKR_CH_en;


/**
*  @brief  This function is the Init entry point for CHAL speaker driver 
*			which should be the	first function to call.
*
*  @param  syscfg_baseAddr	   (in) the base address of system config.
*  @param  dsp_audio_baseAddr  (in) the base address of audio block.
*
*  @return CHAL_HANDLE	(out) CHAL handle for this speaker driver instance
*
****************************************************************************/
CHAL_HANDLE chal_audiospeaker_Init(
				cUInt32 syscfg_baseAddr,
				cUInt32 dsp_audio_baseAddr,
                cUInt32 ahb_tl3r_baseAddr
				);


/**
*  @brief  set Speaker driver mode
*
*  @param  handle (in) this speaker instance
*  @param  spkr   (in) speaker driver seletion
*  @param  mode	  (in) such as: mute positive, 3wire setting
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_SetMode(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_SPKR_CH_en		spkr,
		AUDIO_SPKR_CH_Mode_t		mode
		);

/**
*  @brief  Set Speaker PGA
*
*  @param  handle (in) this Mixer instance
*  @param  spkr   (in) speaker driver seletion
*  @param  gain   (in) gain in mB.
*
*  @return none
*  @note   the gain indexes for diff mode and 3-wire mode are different.
*
****************************************************************************/
cVoid chal_audiospeaker_SetPGA(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		const cInt32			gain
		);

/**
*  @brief  Set Speaker PGA by index
*
*  @param  handle  (in) this Mixer instance
*  @param  spkr   (in) speaker driver seletion
*  @param  gain   (in) gain by index.
*
*  @return none
*     the gain indexes for diff mode and 3-wire mode are different.
*
****************************************************************************/
cVoid chal_audiospeaker_SetPGAIndex(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		const cInt32			gain_index
		);

/**
*  @brief  Set Speaker mute or un-mute
*
*  @param  handle (in) this Mixer instance
*  @param  spkr   (in) speaker driver seletion
*  @param  mute	  (in) TRUE = mute, FALSE= unmute
*
*  @return none
*  @note   mute: mute both positive and negative output
*
****************************************************************************/
cVoid chal_audiospeaker_Mute(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					mute
		);

/**
*  @brief  Power Ramp UP or Down
*
*  @param  handle    (in) this speaker instance
*  @param  ramp_hex (in) 
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_SetAnaPwrRamp(
		CHAL_HANDLE handle,
		UInt16		ramp_hex
		);

/**
*  @brief  Power down or up DAC power reference
*
*  @param  handle    (in) this speaker instance
*  @param  spkr      (in) speaker driver instance
*  @param  pwr_down  (in) 1 - power down, 0 - power up
*
*  @return none
****************************************************************************/
cVoid chal_audiospeaker_PwrdnDacRef(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		);

/**
*  @brief  Power Down Speaker Driver and DAC
*
*  @param  handle (in) this speaker instance
*  @param  spkr   (in) speaker driver instance
*  @param  pwrdn  (in) TRUE: to power down, FALSE : to power up
*
*  @return none
*  @note    only power down DAC and speaker driver. Don't touch DAC reference and D2C.
*
****************************************************************************/
cVoid chal_audiospeaker_PwrdnDacDrv(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwrdn
		);

/**
*  @brief  Power Down d2c clock
*
*  @param  handle (in) this speaker instance
*  @param  spkr   (in) speaker driver instance
*  @param  pwrdn  (in) TRUE: to power down, FALSE : to power up
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_PwrdnD2c(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		);

/**
*  @brief  Read ANACR0
*
*  @param  handle (in) this speaker instance
*
*  @return UInt32
*
****************************************************************************/
UInt32 chal_audiospeaker_read_ANACR0(CHAL_HANDLE  handle );

/**
*  @brief  Write ANACR0
*
*  @param  handle (in) this speaker instance
*  @param  value (in) 
*
*  @return UInt32
*
****************************************************************************/
cVoid chal_audiospeaker_write_ANACR0( 
									CHAL_HANDLE  handle,
									cUInt32		value
									);

/**
*  @brief  global reset AUDVOC
*
*  @param  handle (in) this speaker instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_AUDIOSRST( CHAL_HANDLE  handle );

/**
*  @brief  Earpiese DAC power-down offset generation
*
*  @param  handle    (in) this speaker instance
*  @param  spkr      (in) speaker driver instance
*  @param  pwr_down  (in) 1 - power down, 0 - power up
*
*  @return none
****************************************************************************/
cVoid chal_audiospeaker_EpDacPdOffsetGen(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		);

/**
*  @brief  disable ForcePwrUpMask
*
*  @param  handle (in) this speaker instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_ForcePwrUpMask( CHAL_HANDLE  handle );

/**
*  @brief  Earpiese DAC power gate control
*
*  @param  handle (in) this speaker instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_EpDacPwrGateCtrl( CHAL_HANDLE  handle );

/**
*  @brief  Earpiese DAC ramp reference enable
*
*  @param  handle (in) this speaker instance
*
*  @return none
*
****************************************************************************/
cVoid chal_audiospeaker_EpDacRampRefEn( CHAL_HANDLE  handle );


cVoid chal_audiospeaker_AUDIOSRST( CHAL_HANDLE	handle );
/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_AUDIOSPEAKER_H_
