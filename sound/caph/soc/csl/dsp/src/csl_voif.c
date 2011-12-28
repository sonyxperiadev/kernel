//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   csl_voif.c
*
*   @brief  This file contains DSP VoIF interface
*
****************************************************************************/
#include "mobcom_types.h"
#include "shared.h"
#include "csl_voif.h"

extern AP_SharedMem_t	*vp_shared_mem;


//*********************************************************************
/**
*
*   CSL_GetULVoIFBuffer gets pointer to UL VoIF buffer
*
*   @return   Int16*						uplink VoIF buffer	
* 
**********************************************************************/
Int16* CSL_GetULVoIFBuffer(void)
{
	return(&vp_shared_mem->shared_voif_UL_buffer[0]);

}

//*********************************************************************
/**
*
*   CSL_GetDLVoIFBuffer gets pointer to DL VoIF buffer
*
*   @param    sampleCount		(in)		number of samples
*   @param    dlBufferIndex		(in)		downlink ping-pong buffer index
*   @return   Int16*						downlink VoIF buffer
* 
**********************************************************************/
Int16* CSL_GetDLVoIFBuffer(UInt16 sampleCount, UInt16 dlBufferIndex)
{
	return(&vp_shared_mem->shared_voif_DL_buffer[dlBufferIndex*sampleCount]);

}
