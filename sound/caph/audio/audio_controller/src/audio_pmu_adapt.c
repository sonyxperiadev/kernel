/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
* @file   audio_pmu_adapt.c
* @brief  
*
******************************************************************************/

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) 

#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 )||defined(CONFIG_BCMPMU_AUDIO))

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

//#include "pmu.h"
//#include "hal_pmu.h"
//#include "hal_pmu_private.h"

// to /kernel/drivers/char/broadcom/modem/public/peripherals/pmu/public/pmu.h
//#include "../../drivers/char/broadcom/modem/public/peripherals/pmu/public/pmu.h"

// from kernel/sound/caph, to find kernel/drivers/char/broadcom/modem/public/sysinterface/hal/pmu/public/hal_pmu_private.h
//#include "../../drivers/char/broadcom/modem/public/sysinterface/hal/pmu/public/hal_pmu.h"
//#include "../../drivers/char/broadcom/modem/public/sysinterface/hal/pmu/public/hal_pmu_private.h"

//#include "../../drivers/char/broadcom/modem/public/peripherals/pmu/public/brcm/pmu_bcm59055.h"

#include <linux/kernel.h>

#include "audio_pmu_adapt.h"

//=============================================================================
// Public Variable declarations
//=============================================================================


//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    Int16 gain;
    UInt32 PMUGain;
}PMU_AudioGainMapping_t;

/*** FYI: from kernel/include/linux/broadcom/bcm59055-audio.h
enum
{
	PMU_HSGAIN_MUTE,
	PMU_HSGAIN_66DB_N,
	PMU_HSGAIN_63DB_N,
	PMU_HSGAIN_60DB_N,
	PMU_HSGAIN_57DB_N,
	PMU_HSGAIN_54DB_N,
	PMU_HSGAIN_51DB_N,
	PMU_HSGAIN_48DB_N,
	PMU_HSGAIN_45DB_N,
	PMU_HSGAIN_42DB_N,
	PMU_HSGAIN_40P5DB_N,
	PMU_HSGAIN_39DB_N,
	PMU_HSGAIN_37P5DB_N,
	PMU_HSGAIN_36DB_N,
	PMU_HSGAIN_34P5DB_N,
	PMU_HSGAIN_33DB_N,
	PMU_HSGAIN_31P5DB_N,
	PMU_HSGAIN_30DB_N,
	PMU_HSGAIN_28P5DB_N,
	PMU_HSGAIN_27DB_N,
	PMU_HSGAIN_25P5DB_N,
	PMU_HSGAIN_24DB_N,
	PMU_HSGAIN_22P5DB_N,
	PMU_HSGAIN_22DB_N,
	PMU_HSGAIN_21P5DB_N,
	PMU_HSGAIN_21DB_N,
	PMU_HSGAIN_20P5DB_N,
	PMU_HSGAIN_20DB_N,
	PMU_HSGAIN_19P5DB_N,
	PMU_HSGAIN_19DB_N,
	PMU_HSGAIN_18P5DB_N,
	PMU_HSGAIN_18DB_N,
	PMU_HSGAIN_17P5DB_N,
	PMU_HSGAIN_17DB_N,
	PMU_HSGAIN_16P5DB_N,
	PMU_HSGAIN_16DB_N,
	PMU_HSGAIN_15P5DB_N,
	PMU_HSGAIN_15DB_N,
	PMU_HSGAIN_14P5DB_N,
	PMU_HSGAIN_14DB_N,
	PMU_HSGAIN_13P5DB_N,
	PMU_HSGAIN_13DB_N,
	PMU_HSGAIN_12P5DB_N,
	PMU_HSGAIN_12DB_N,
	PMU_HSGAIN_11P5DB_N,
	PMU_HSGAIN_11DB_N,
	PMU_HSGAIN_10P5DB_N,
	PMU_HSGAIN_10DB_N,
	PMU_HSGAIN_9P5DB_N,
	PMU_HSGAIN_9DB_N,
	PMU_HSGAIN_8P5DB_N,
	PMU_HSGAIN_8DB_N,
	PMU_HSGAIN_7P5DB_N,
	PMU_HSGAIN_7DB_N,
	PMU_HSGAIN_6P5DB_N,
	PMU_HSGAIN_6DB_N,
	PMU_HSGAIN_5P5DB_N,
	PMU_HSGAIN_5DB_N,
	PMU_HSGAIN_4P5DB_N,
	PMU_HSGAIN_4DB_N,
	PMU_HSGAIN_3P5DB_N,
	PMU_HSGAIN_3DB_N,
	PMU_HSGAIN_2P5DB_N,
	PMU_HSGAIN_2DB_N,
	PMU_HSGAIN_NUM
};

enum
{
	PMU_IHFGAIN_MUTE,
	PMU_IHFGAIN_60DB_N,
	PMU_IHFGAIN_57DB_N,
	PMU_IHFGAIN_54DB_N,
	PMU_IHFGAIN_51DB_N,
	PMU_IHFGAIN_48DB_N,
	PMU_IHFGAIN_45DB_N,
	PMU_IHFGAIN_42DB_N,
	PMU_IHFGAIN_39DB_N,
	PMU_IHFGAIN_36DB_N,
	PMU_IHFGAIN_34P5DB_N,
	PMU_IHFGAIN_33DB_N,
	PMU_IHFGAIN_31P5DB_N,
	PMU_IHFGAIN_30DB_N,
	PMU_IHFGAIN_28P5DB_N,
	PMU_IHFGAIN_27DB_N,
	PMU_IHFGAIN_25P5DB_N,
	PMU_IHFGAIN_24DB_N,
	PMU_IHFGAIN_22P5DB_N,
	PMU_IHFGAIN_21DB_N,
	PMU_IHFGAIN_19P5DB_N,
	PMU_IHFGAIN_18DB_N,
	PMU_IHFGAIN_16P5DB_N,
	PMU_IHFGAIN_16DB_N,
	PMU_IHFGAIN_15P5DB_N,
	PMU_IHFGAIN_15DB_N,
	PMU_IHFGAIN_14P5DB_N,
	PMU_IHFGAIN_14DB_N,
	PMU_IHFGAIN_13P5DB_N,
	PMU_IHFGAIN_13DB_N,
	PMU_IHFGAIN_12P5DB_N,
	PMU_IHFGAIN_12DB_N,
	PMU_IHFGAIN_11P5DB_N,
	PMU_IHFGAIN_11DB_N,
	PMU_IHFGAIN_10P5DB_N,
	PMU_IHFGAIN_10DB_N,
	PMU_IHFGAIN_9P5DB_N,
	PMU_IHFGAIN_9DB_N,
	PMU_IHFGAIN_8P5DB_N,
	PMU_IHFGAIN_8DB_N,
	PMU_IHFGAIN_7P5DB_N,
	PMU_IHFGAIN_7DB_N,
	PMU_IHFGAIN_6P5DB_N,
	PMU_IHFGAIN_6DB_N,
	PMU_IHFGAIN_5P5DB_N,
	PMU_IHFGAIN_5DB_N,
	PMU_IHFGAIN_4P5DB_N,
	PMU_IHFGAIN_4DB_N,
	PMU_IHFGAIN_3P5DB_N,
	PMU_IHFGAIN_3DB_N,
	PMU_IHFGAIN_2P5DB_N,
	PMU_IHFGAIN_2DB_N,
	PMU_IHFGAIN_1P5DB_N,
	PMU_IHFGAIN_1DB_N,
	PMU_IHFGAIN_P5DB_N,
	PMU_IHFGAIN_0DB,
	PMU_IHFGAIN_P5DB_P,
	PMU_IHFGAIN_1DB_P,
	PMU_IHFGAIN_1P5DB_P,
	PMU_IHFGAIN_2DB_P,
	PMU_IHFGAIN_2P5DB_P,
	PMU_IHFGAIN_3DB_P,
	PMU_IHFGAIN_3P5DB_P,
	PMU_IHFGAIN_4DB_P,
    PMU_IHFGAIN_NUM
};
***/

static PMU_AudioGainMapping_t hsPMUGainTable[PMU_HSGAIN_NUM]=
{
    /* Gain in Q13.2,   HS PMU Gain */
    {0x8000,             PMU_HSGAIN_MUTE},
    {0xFEF8,             PMU_HSGAIN_66DB_N},
    {0xFF04,             PMU_HSGAIN_63DB_N},
    {0xFF10,             PMU_HSGAIN_60DB_N},
    {0xFF1C,             PMU_HSGAIN_57DB_N},
    {0xFF28,             PMU_HSGAIN_54DB_N},
    {0xFF34,             PMU_HSGAIN_51DB_N},
    {0xFF40,             PMU_HSGAIN_48DB_N},
    {0xFF4C,             PMU_HSGAIN_45DB_N},
    {0xFF58,             PMU_HSGAIN_42DB_N},
    {0xFF5E,             PMU_HSGAIN_40P5DB_N},
    {0xFF64,             PMU_HSGAIN_39DB_N},
    {0xFF6A,             PMU_HSGAIN_37P5DB_N},
    {0xFF70,             PMU_HSGAIN_36DB_N},
    {0xFF76,             PMU_HSGAIN_34P5DB_N},
    {0xFF7C,             PMU_HSGAIN_33DB_N},
    {0xFF82,             PMU_HSGAIN_31P5DB_N},
    {0xFF88,             PMU_HSGAIN_30DB_N},
    {0xFF8E,             PMU_HSGAIN_28P5DB_N},
    {0xFF94,             PMU_HSGAIN_27DB_N},
    {0xFF9A,             PMU_HSGAIN_25P5DB_N},
    {0xFFA0,             PMU_HSGAIN_24DB_N},
    {0xFFA6,             PMU_HSGAIN_22P5DB_N},
    {0xFFA8,             PMU_HSGAIN_22DB_N},
    {0xFFAA,             PMU_HSGAIN_21P5DB_N},
    {0xFFAC,             PMU_HSGAIN_21DB_N},
    {0xFFAE,             PMU_HSGAIN_20P5DB_N},
    {0xFFB0,             PMU_HSGAIN_20DB_N},
    {0xFFB2,             PMU_HSGAIN_19P5DB_N},
    {0xFFB4,             PMU_HSGAIN_19DB_N},
    {0xFFB6,             PMU_HSGAIN_18P5DB_N},
    {0xFFB8,             PMU_HSGAIN_18DB_N},
    {0xFFBA,             PMU_HSGAIN_17P5DB_N},
    {0xFFBC,             PMU_HSGAIN_17DB_N},
    {0xFFBE,             PMU_HSGAIN_16P5DB_N},
    {0xFFC0,             PMU_HSGAIN_16DB_N},
    {0xFFC2,             PMU_HSGAIN_15P5DB_N},
    {0xFFC4,             PMU_HSGAIN_15DB_N},
    {0xFFC6,             PMU_HSGAIN_14P5DB_N},
    {0xFFC8,             PMU_HSGAIN_14DB_N},
    {0xFFCA,             PMU_HSGAIN_13P5DB_N},
    {0xFFCC,             PMU_HSGAIN_13DB_N},
    {0xFFCE,             PMU_HSGAIN_12P5DB_N},
    {0xFFD0,             PMU_HSGAIN_12DB_N},
    {0xFFD2,             PMU_HSGAIN_11P5DB_N},
    {0xFFD4,             PMU_HSGAIN_11DB_N},
    {0xFFD6,             PMU_HSGAIN_10P5DB_N},
    {0xFFD8,             PMU_HSGAIN_10DB_N},
    {0xFFDA,             PMU_HSGAIN_9P5DB_N},
    {0xFFDC,             PMU_HSGAIN_9DB_N},
    {0xFFDE,             PMU_HSGAIN_8P5DB_N},
    {0xFFE0,             PMU_HSGAIN_8DB_N},
    {0xFFE2,             PMU_HSGAIN_7P5DB_N},
    {0xFFE4,             PMU_HSGAIN_7DB_N},
    {0xFFE6,             PMU_HSGAIN_6P5DB_N},
    {0xFFE8,             PMU_HSGAIN_6DB_N},
    {0xFFEA,             PMU_HSGAIN_5P5DB_N},
    {0xFFEC,             PMU_HSGAIN_5DB_N},
    {0xFFEE,             PMU_HSGAIN_4P5DB_N},
    {0xFFF0,             PMU_HSGAIN_4DB_N},
    {0xFFF2,             PMU_HSGAIN_3P5DB_N},
    {0xFFF4,             PMU_HSGAIN_3DB_N},
    {0xFFF6,             PMU_HSGAIN_2P5DB_N},
    {0xFFF8,             PMU_HSGAIN_2DB_N}
};

static PMU_AudioGainMapping_t ihfPMUGainTable[PMU_IHFGAIN_NUM]=
{
    /* Gain in Q13.2,   IHF PMU Gain */
    {0x8000,             PMU_IHFGAIN_MUTE},
    {0xFF10,             PMU_IHFGAIN_60DB_N},
    {0xFF1C,             PMU_IHFGAIN_57DB_N},
    {0xFF28,             PMU_IHFGAIN_54DB_N},
    {0xFF34,             PMU_IHFGAIN_51DB_N},
    {0xFF40,             PMU_IHFGAIN_48DB_N},
    {0xFF4C,             PMU_IHFGAIN_45DB_N},
    {0xFF58,             PMU_IHFGAIN_42DB_N},
    {0xFF64,             PMU_IHFGAIN_39DB_N},
    {0xFF70,             PMU_IHFGAIN_36DB_N},
    {0xFF76,             PMU_IHFGAIN_34P5DB_N},
    {0xFF7C,             PMU_IHFGAIN_33DB_N},
    {0xFF82,             PMU_IHFGAIN_31P5DB_N},
    {0xFF88,             PMU_IHFGAIN_30DB_N},
    {0xFF8E,             PMU_IHFGAIN_28P5DB_N},
    {0xFF94,             PMU_IHFGAIN_27DB_N},
    {0xFF9A,             PMU_IHFGAIN_25P5DB_N},
    {0xFFA0,             PMU_IHFGAIN_24DB_N},
    {0xFFA6,             PMU_IHFGAIN_22P5DB_N},
    {0xFFAC,             PMU_IHFGAIN_21DB_N},
    {0xFFB2,             PMU_IHFGAIN_19P5DB_N},
    {0xFFB8,             PMU_IHFGAIN_18DB_N},
    {0xFFBE,             PMU_IHFGAIN_16P5DB_N},
    {0xFFC0,             PMU_IHFGAIN_16DB_N},
    {0xFFC2,             PMU_IHFGAIN_15P5DB_N},
    {0xFFC4,             PMU_IHFGAIN_15DB_N},
    {0xFFC6,             PMU_IHFGAIN_14P5DB_N},
    {0xFFC8,             PMU_IHFGAIN_14DB_N},
    {0xFFCA,             PMU_IHFGAIN_13P5DB_N},
    {0xFFCC,             PMU_IHFGAIN_13DB_N},
    {0xFFCE,             PMU_IHFGAIN_12P5DB_N},
    {0xFFD0,             PMU_IHFGAIN_12DB_N},
    {0xFFD2,             PMU_IHFGAIN_11P5DB_N},
    {0xFFD4,             PMU_IHFGAIN_11DB_N},
    {0xFFD6,             PMU_IHFGAIN_10P5DB_N},
    {0xFFD8,             PMU_IHFGAIN_10DB_N},
    {0xFFDA,             PMU_IHFGAIN_9P5DB_N},
    {0xFFDC,             PMU_IHFGAIN_9DB_N},
    {0xFFDE,             PMU_IHFGAIN_8P5DB_N},
    {0xFFE0,             PMU_IHFGAIN_8DB_N},
    {0xFFE2,             PMU_IHFGAIN_7P5DB_N},
    {0xFFE4,             PMU_IHFGAIN_7DB_N},
    {0xFFE6,             PMU_IHFGAIN_6P5DB_N},
    {0xFFE8,             PMU_IHFGAIN_6DB_N},
    {0xFFEA,             PMU_IHFGAIN_5P5DB_N},
    {0xFFEC,             PMU_IHFGAIN_5DB_N},
    {0xFFEE,             PMU_IHFGAIN_4P5DB_N},
    {0xFFF0,             PMU_IHFGAIN_4DB_N},
    {0xFFF2,             PMU_IHFGAIN_3P5DB_N},
    {0xFFF4,             PMU_IHFGAIN_3DB_N},
    {0xFFF6,             PMU_IHFGAIN_2P5DB_N},
    {0xFFF8,             PMU_IHFGAIN_2DB_N},
    {0xFFFA,             PMU_IHFGAIN_1P5DB_N},
    {0xFFFC,             PMU_IHFGAIN_1DB_N},
    {0xFFFE,             PMU_IHFGAIN_P5DB_N},
    {0x0000,             PMU_IHFGAIN_0DB},
    {0x0002,             PMU_IHFGAIN_P5DB_P},
    {0x0004,             PMU_IHFGAIN_1DB_P},
    {0x0006,             PMU_IHFGAIN_1P5DB_P},
    {0x0008,             PMU_IHFGAIN_2DB_P},
    {0x000A,             PMU_IHFGAIN_2P5DB_P},
    {0x000C,             PMU_IHFGAIN_3DB_P},
    {0x000E,             PMU_IHFGAIN_3P5DB_P},
    {0x0010,             PMU_IHFGAIN_4DB_P},
};


//============================================================================
//
// Function Name: map2pmu_hs_gain
//
// Description:   convert Headset gain dB value to PMU-format gain value
// 
// Note: input gain is in dB.
//
//============================================================================
UInt32 map2pmu_hs_gain_fromDB( Int16 db_gain )
{

#if defined(PMU_MAX8986)

	if ( db_gain== (Int16)(-19) ) 	return PMU_HSGAIN_19DB_N;
	else if ( db_gain== (Int16)(-18) || db_gain== (Int16)(-17) || db_gain== (Int16)(-16))		return PMU_HSGAIN_16DB_N;
	else if ( db_gain== (Int16)(-15) || db_gain== (Int16)(-14))		return PMU_HSGAIN_14DB_N;
	else if ( db_gain== (Int16)(-13) || db_gain== (Int16)(-12))		return PMU_HSGAIN_12DB_N;
	else if ( db_gain== (Int16)(-11) || db_gain== (Int16)(-10))		return PMU_HSGAIN_10DB_N;
	else if ( db_gain== (Int16)(-9) ||  db_gain== (Int16)(-8))		return PMU_HSGAIN_8DB_N;
	else if ( db_gain== (Int16)(-7) ||  db_gain== (Int16)(-6))		return PMU_HSGAIN_6DB_N;
	else if ( db_gain== (Int16)(-5) ||  db_gain== (Int16)(-4))		return PMU_HSGAIN_4DB_N;
	else if ( db_gain== (Int16)(-3) ||  db_gain== (Int16)(-2))		return PMU_HSGAIN_2DB_N;
	else if ( db_gain== (Int16)(-1) )		return PMU_HSGAIN_1DB_N;
	else if ( db_gain== (Int16)(0) )		return PMU_HSGAIN_0DB;
	else if ( db_gain== (Int16)(1) )		return PMU_HSGAIN_1DB_P;
	else if ( db_gain== (Int16)(2) )		return PMU_HSGAIN_2DB_P;
	else if ( db_gain== (Int16)(3) )		return PMU_HSGAIN_3DB_P;
	else if ( db_gain== (Int16)(4) )		return PMU_HSGAIN_4DB_P;
	// PMU_HSGAIN_4P5DB_P
	else if ( db_gain== (Int16)(5) )		return PMU_HSGAIN_5DB_P;
	// PMU_HSGAIN_5P5DB_P
	else if ( db_gain== (Int16)(6) )		return PMU_HSGAIN_6DB_P;

#else

	//Should not run to here.
	return hsPMUGainTable[PMU_HSGAIN_NUM-1].PMUGain;

#endif
}

//============================================================================
//
// Function Name: map2pmu_ihf_gain
//
// Description:   convert IHF gain dB value to PMU-format gain value
//
// Note: input gain is in dB.
//
//============================================================================
UInt32 map2pmu_ihf_gain_fromDB( Int16 db_gain )
{
#if defined(PMU_MAX8986)	

    if ( db_gain== (Int16)(-33) || db_gain== (Int16)(-32) || db_gain== (Int16)(-31) || db_gain== (Int16)(-30) ) return PMU_IHFGAIN_30DB_N;
    else if ( db_gain== (Int16)(-29) || db_gain== (Int16)(-28) || db_gain== (Int16)(-27) || db_gain== (Int16)(-26) ) return PMU_IHFGAIN_26DB_N;
    else if ( db_gain== (Int16)(-25) || db_gain== (Int16)(-24) || db_gain== (Int16)(-23) || db_gain== (Int16)(-22) ) return PMU_IHFGAIN_22DB_N;
	else if ( db_gain== (Int16)(-21) || db_gain== (Int16)(-20) || db_gain== (Int16)(-19) || db_gain== (Int16)(-18) ) return PMU_IHFGAIN_18DB_N;
	else if ( db_gain== (Int16)(-17) || db_gain== (Int16)(-16) || db_gain== (Int16)(-15) || db_gain== (Int16)(-14) )	return PMU_IHFGAIN_14DB_N;
	else if ( db_gain== (Int16)(-13) || db_gain== (Int16)(-12) )	return PMU_IHFGAIN_12DB_N;
	else if ( db_gain== (Int16)(-11) || db_gain== (Int16)(-10) )	return PMU_IHFGAIN_10DB_N;
	else if ( db_gain== (Int16)(-9)  || db_gain== (Int16)(-8) )		return PMU_IHFGAIN_8DB_N;
	else if ( db_gain== (Int16)(-7)  || db_gain== (Int16)(-6) )		return PMU_IHFGAIN_6DB_N;
	else if ( db_gain== (Int16)(-5)  || db_gain== (Int16)(-4) )		return PMU_IHFGAIN_4DB_N;
	else if ( db_gain== (Int16)(-3)  || db_gain== (Int16)(-2) )		return PMU_IHFGAIN_2DB_N;
	else if ( db_gain== (Int16)(-1)  || db_gain== (Int16)(0) )		return PMU_IHFGAIN_0DB;
	else if ( db_gain== (Int16)(1) )		return PMU_IHFGAIN_1DB_P;
	else if ( db_gain== (Int16)(2) )		return PMU_IHFGAIN_2DB_P;
	else if ( db_gain== (Int16)(3) )		return PMU_IHFGAIN_3DB_P;
	else if ( db_gain== (Int16)(4) )		return PMU_IHFGAIN_4DB_P;
	else if ( db_gain== (Int16)(5) )		return PMU_IHFGAIN_5DB_P;
	else if ( db_gain== (Int16)(6) )		return PMU_IHFGAIN_6DB_P;
	else if ( db_gain== (Int16)(7) )		return PMU_IHFGAIN_7DB_P;
	else if ( db_gain== (Int16)(8) )		return PMU_IHFGAIN_8DB_P;
	else if ( db_gain== (Int16)(9) )		return PMU_IHFGAIN_9DB_P;
	else if ( db_gain== (Int16)(10) )		return PMU_IHFGAIN_10DB_P;
	else if ( db_gain== (Int16)(11) )		return PMU_IHFGAIN_11DB_P;
	else if ( db_gain== (Int16)(12) )		return PMU_IHFGAIN_12DB_P;
	// PMU_IHFGAIN_12P5DB_P,
    else if ( db_gain== (Int16)(13) )		return PMU_IHFGAIN_13DB_P;
	// PMU_IHFGAIN_13P5DB_P,
    else if ( db_gain== (Int16)(14) )		return PMU_IHFGAIN_14DB_P;
	// PMU_IHFGAIN_14P5DB_P,
    else if ( db_gain== (Int16)(15) )		return PMU_IHFGAIN_15DB_P;
	// PMU_IHFGAIN_15P5DB_P,
    else if ( db_gain== (Int16)(16) )		return PMU_IHFGAIN_16DB_P;
	// PMU_IHFGAIN_16P5DB_P,
    else if ( db_gain== (Int16)(17) )		return PMU_IHFGAIN_17DB_P;
	// PMU_IHFGAIN_17P5DB_P,
    else if ( db_gain== (Int16)(18) )		return PMU_IHFGAIN_18DB_P;
	// PMU_IHFGAIN_18P5DB_P,
    else if ( db_gain== (Int16)(19) )		return PMU_IHFGAIN_19DB_P;
	// PMU_IHFGAIN_19P5DB_P,
    else if ( db_gain== (Int16)(20) )		return PMU_IHFGAIN_20DB_P;

#else

	//Should not run to here.
	return ihfPMUGainTable[PMU_IHFGAIN_NUM-1].PMUGain;

#endif
}


//============================================================================
//
// Function Name: map2pmu_hs_gain
//
// Description:   convert Headset gain dB value to PMU-format gain value
// 
// Note: input gain is in Q13.2 dB.
//
//============================================================================
UInt32 map2pmu_hs_gain_fromQ13dot2( Int16 gain )
{
#if defined(PMU_MAX8986)

#else

	{
	UInt8 i = 0;
	
	if (gain < hsPMUGainTable[1].gain)
	{
		 return hsPMUGainTable[0].PMUGain;		
	}
	else
	if (gain >= hsPMUGainTable[PMU_HSGAIN_NUM-1].gain)
	{
		return hsPMUGainTable[PMU_HSGAIN_NUM-1].PMUGain;
	}
	
	for (i = 1; i<PMU_HSGAIN_NUM; i++)
	{
		if(gain == hsPMUGainTable[i].gain)
		{
			return hsPMUGainTable[i].PMUGain;
		}	
	}
	
	for (i = 1; i<PMU_HSGAIN_NUM -1; i++)
	{
		if((gain - hsPMUGainTable[i].gain)<=(hsPMUGainTable[i+1].gain - gain))
		{
			return hsPMUGainTable[i].PMUGain;
		}	
	}
	
	//Should not run to here.
	return hsPMUGainTable[PMU_HSGAIN_NUM-1].PMUGain;

	}

#endif
}

//============================================================================
//
// Function Name: map2pmu_ihf_gain
//
// Description:   convert IHF gain dB value to PMU-format gain value
//
// Note: input gain is in Q13.2 dB.
//
//============================================================================
UInt32 map2pmu_ihf_gain_fromQ13dot2( Int16 gain )
{
#if defined(PMU_MAX8986)	

#else

	{
	UInt8 i = 0;
	
	if (gain < ihfPMUGainTable[1].gain)
	{
		return ihfPMUGainTable[0].PMUGain;
	}
	else
	if (gain >= ihfPMUGainTable[PMU_IHFGAIN_NUM-1].gain)
	{
		return ihfPMUGainTable[PMU_IHFGAIN_NUM-1].PMUGain;
	}
	
	for (i = 1; i<PMU_IHFGAIN_NUM; i++)
	{
		if(gain == ihfPMUGainTable[i].gain)
		{
			return ihfPMUGainTable[i].PMUGain;
		}	
	}
	
	for (i = 1; i<PMU_IHFGAIN_NUM -1; i++)
	{
		if((gain - ihfPMUGainTable[i].gain)<=(ihfPMUGainTable[i+1].gain - gain))
		{
			return ihfPMUGainTable[i].PMUGain;
		}	
	}
	
	//Should not run to here.
	return ihfPMUGainTable[PMU_IHFGAIN_NUM-1].PMUGain;
	}

#endif
}


#endif //#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 ))

#endif //#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)

