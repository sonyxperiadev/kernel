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
*   @file   csl_caph_gain.h
*
*   @brief  This file contains the gain distribution tables for driver layer
*
****************************************************************************/
#include "mobcom_types.h"

#ifndef _CSL_CAPH_GAIN_H_
#define _CSL_CAPH_GAIN_H_


typedef enum
{
	MIC_ANALOG_HEADSET,
	MIC_DIGITAL,
}csl_caph_MIC_Path_e;

typedef struct csl_caph_MicDSP_Gain_t
{
	Int16 micGain;
	UInt16 micPGA;
	UInt16 micCICFineScale;
	UInt16 micCICBitSelect;
	UInt16 micDSPULGain;
}csl_caph_MicDSP_Gain_t;

typedef struct csl_caph_MicMEM_Gain_t
{
	Int16 micGain;
	UInt16 micPGA;
	UInt16 micCICFineScale;
	UInt16 micCICBitSelect;
}csl_caph_MicMEM_Gain_t;


typedef enum
{
	SPKR_EP,
	SPKR_IHF,
	SPKR_HS,
	SPKR_PATH_MAX
}csl_caph_SPKR_Path_e;

typedef struct csl_caph_SpkrDSP_Gain_t
{
	Int16 spkrGain;
	UInt16 spkrPMUGain;
	UInt16 spkrDSPDLGain;
}csl_caph_SpkrDSP_Gain_t;

typedef struct csl_caph_SpkrMEM_Gain_t
{
	Int16 spkrGain;
	UInt16 spkrPMUGain;
}csl_caph_SpkrMEM_Gain_t;

/**
*
*  @brief  read the mic pah gains when DSP is involved.
*  
*  @param  mic    mic path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on mic
*                     path.
*****************************************************************************/
csl_caph_MicDSP_Gain_t csl_caph_gain_GetMicDSPGain(csl_caph_MIC_Path_e mic, 
		Int16 gain);

/**
*
*  @brief  read the mic pah gains when DSP is not involved.
*  
*  @param  mic    mic path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on mic
*                     path.
*****************************************************************************/
csl_caph_MicMEM_Gain_t csl_caph_gain_GetMicMEMGain(csl_caph_MIC_Path_e mic, 
		Int16 gain);

/**
*
*  @brief  read the speaker pah gains when DSP is involved.
*  
*  @param  spkr    spkr path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on spkr
*                     path.
*****************************************************************************/
csl_caph_SpkrDSP_Gain_t csl_caph_gain_GetSpkrDSPGain(csl_caph_SPKR_Path_e mic, 
		Int16 gain);

/**
*
*  @brief  read the speaker pah gains when DSP is not involved.
*  
*  @param  spker    spkr path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on spkr
*                     path.
*****************************************************************************/
csl_caph_SpkrMEM_Gain_t csl_caph_gain_GetSpkrMEMGain(csl_caph_SPKR_Path_e mic, 
		Int16 gain);
#endif // _CSL_CAPH_GAIN_H_

