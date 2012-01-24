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
*  @file   csl_dsp_caph_control_api.c
*
*  @brief  This file contains CSL DSP API functions for control of CAPH (related to DSP)
*
*  @note   
*
**/
/*******************************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "osdw_dsp_drv.h"
#include "csl_dsp.h"
#include "dspcmd.h"
#include "csl_dsp_caph_control_api.h"
#include "log.h"

extern AP_SharedMem_t	*vp_shared_mem;
extern AP_SharedMem_t   *DSPDRV_GetPhysicalSharedMemoryAddress( void);


/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_caph_control_get_aadmac_buf_base_addr
*
*   @note     This function returns the base address of the AADMAC buffer's base address
*             (pointing to the start of the Low part of the AADMAC buffer).
*                                                                                         
*   @param    value (DSP_AADMAC_Audio_Connections_t)  Return the base address for which 
*                                                     AADMAC buffer.
*
*   @return   UInt32 *dsp_aadmac_base_addr Base address of the AADMAC buffer.
*
**/
/*******************************************************************************************/
UInt32 * csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_Audio_Connections_t aadmac_audio_connection)
{
    UInt32 *base_addr;
    AP_SharedMem_t   *ap_shared_mem_ptr;
    ap_shared_mem_ptr = DSPDRV_GetPhysicalSharedMemoryAddress();

    switch (aadmac_audio_connection)
    {
        case DSP_AADMAC_PRI_MIC_EN:
            base_addr = &(ap_shared_mem_ptr->shared_aadmac_pri_mic_low[0]);
            break;
        case DSP_AADMAC_SEC_MIC_EN:
            base_addr = &(ap_shared_mem_ptr->shared_aadmac_sec_mic_low[0]);
            break;
        case DSP_AADMAC_SPKR_EN:
            base_addr = &(ap_shared_mem_ptr->shared_aadmac_spkr_low[0]);
            break;
        default:
            //Assert
            assert(0);
            break;
    }

    return base_addr;

}

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_caph_control_aadmac_set_samp_rate
*
*   @note     This function sets the sample rate for the AADMAC based audio for the DSP
*             (not valid for IHF)
*                                                                                         
*   @param    value (UInt16)  Sample rate for AADMAC based audio for the DSP (non-IHF)
*             = 8000 or 16000
*
*   @return   None
*
**/
/*******************************************************************************************/
void csl_dsp_caph_control_aadmac_set_samp_rate(UInt16 value)
{
    if ( (value != 8000) && (value != 16000) )
    {
        //Assert
        assert(0);
    }
    else
    {
        vp_shared_mem->shared_aadmac_audio_samp_rate = value;
    }
}

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_caph_control_aadmac_enable_path
*
*   @note     This function informs the DSP about which hardware path is enabled, and 
*             based on this information, DSP processes the audio interrupt.
*             (not valid for IHF)
*                                                                                         
*   @param    path (DSP_AADMAC_Audio_Connections_t)
*
*   @return   None
*
**/
/*******************************************************************************************/
void csl_dsp_caph_control_aadmac_enable_path(UInt16 path)
{
    if ( path & (UInt16)(~(DSP_AADMAC_PRI_MIC_EN|DSP_AADMAC_SEC_MIC_EN|DSP_AADMAC_SPKR_EN)) )
    {
        //Assert
        assert(0);
    }
    else
    {


        vp_shared_mem->shared_aadmac_aud_enable |= path;
    	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* csl_dsp_caph_control_aadmac_enable_path: vp_shared_mem = %x,&(vp_shared_mem->shared_aadmac_aud_enable) =%x, shared_aadmac_aud_enable = %x*\n\r", (UInt32)vp_shared_mem, (UInt32)(&(vp_shared_mem->shared_aadmac_aud_enable)), vp_shared_mem->shared_aadmac_aud_enable);

    }

}

/*****************************************************************************************/
/**
* 
* Function Name: csl_dsp_caph_control_aadmac_disable_path
*
*   @note     This function informs the DSP about which hardware path is disabled, and 
*             based on this information, DSP processes the audio interrupt.
*             (not valid for IHF)
*                                                                                         
*   @param    path (DSP_AADMAC_Audio_Connections_t)
*
*   @return   None
*
**/
/*******************************************************************************************/
void csl_dsp_caph_control_aadmac_disable_path(UInt16 path)
{
    if ( path & (UInt16)(~(DSP_AADMAC_PRI_MIC_EN|DSP_AADMAC_SEC_MIC_EN|DSP_AADMAC_SPKR_EN)) )
    {
        //Assert
        assert(0);
    }
    else
    {
        vp_shared_mem->shared_aadmac_aud_enable &= ~path;
    	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* csl_dsp_caph_control_aadmac_disable_path vp_shared_mem = %x,&(vp_shared_mem->shared_aadmac_aud_enable) =%x, shared_aadmac_aud_enable = %x*\n\r", (UInt32)vp_shared_mem, (UInt32)(&(vp_shared_mem->shared_aadmac_aud_enable)), vp_shared_mem->shared_aadmac_aud_enable);
    }

}

