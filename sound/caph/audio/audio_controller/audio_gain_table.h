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
* @file   audio_gain_table.h
* @brief  Audio gain format translation table
*
******************************************************************************/


//=============================================================================
// Include directives
//=============================================================================

#ifndef __AUDIO_GAIN_TABLE_H__
#define __AUDIO_GAIN_TABLE_H__

typedef struct
{
    AudioMode_t audioMode;
    UInt32  gainTotal;
    UInt32  gainPMU;
    UInt32  gainHW;
} AUDTABL_GainMapping_t;

#define NO_PMU_NEEDED   0x7FFFFFF
#define TOTAL_GAIN  0x7FFFFFF


/********************************************************************
*  @brief  read Slopgain value from Q14.1 gain 
*
*  @param  valueQ14_1  (in) gain in Q14.1 format 
*
*  @return HW slopgain register
*
****************************************************************************/
UInt16 AUDTABL_getQ14_1HWSlopGain(UInt16 valueQ14_1);


/********************************************************************
*  @brief  read Slopgain value from Q1.14 gain
*
*  @param  valueQ1_14	(in)  gain in Q1.14 format 
*
*  @return HW slopgain register
*
****************************************************************************/
UInt16 AUDTABL_getQ1_14HWSlopGain(UInt16 valueQ1_14);

/********************************************************************
*  @brief  return the gain distribution values which shows the PMU gain and
*  Audio HW gain
*
*  @param  gainQ31	(in)  gain in Q31.0 format 
*
*  @return Gain distribution values
*
****************************************************************************/
AUDTABL_GainMapping_t AUDTABL_getGainDistribution(AudioMode_t audioMode, UInt32 gainQ31);

#endif	//__AUDIO_GAIN_TABLE_H__

