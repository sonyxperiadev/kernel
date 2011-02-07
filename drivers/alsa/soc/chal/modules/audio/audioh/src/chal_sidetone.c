/*******************************************************************************************
Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*  @file   chal_sidetone.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include "chal_audio.h"
#include "chal_audio_int.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_sdt.h"
#include "brcm_rdb_util.h"

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// local macro declarations
//****************************************************************************


//****************************************************************************
// local typedef declarations
//****************************************************************************



//****************************************************************************
// local variable definitions
//****************************************************************************


//****************************************************************************
// local function declarations
//****************************************************************************



//******************************************************************************
// local function definitions
//******************************************************************************






//============================================================================
//
// Function Name: cVoid chal_audio_stpath_enable(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  Enable or Disable Sidetone path
//
// Parameters:   handle    - audio chal handle.
//                 enable    -  true : enable, false : disable.
//
// Return:       None.
//

//============================================================================
cVoid chal_audio_stpath_enable(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32     reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);

    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_ADC_CTL_SIDETONE_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_ADC_CTL_SIDETONE_EN_MASK;
    }

    BRCM_WRITE_REG(base, AUDIOH_ADC_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_stpath_load_filter(CHAL_HANDLE handle,
//                        cUInt16 *src, cUInt32 length )
//
// Description:  Load coefficient to sidetone path
//
// Parameters:   handle - audio chal handle.
//                 *src   - the coefficient buffer
//                 length - the coefficient length
// Return:       None.
//
//============================================================================

cVoid chal_audio_stpath_load_filter(CHAL_HANDLE handle, cUInt32 *coeff, cUInt32 length )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32      index;
    cUInt32     reg_val;

    //Disable loading the filter as we are updating the filter coefficients
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
    reg_val |= SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK;
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    //Set the index to start at 0
    BRCM_WRITE_REG(base, SDT_SDT_COEF_ADDR, 0);

    //Write the coefficients
    for(index = 0; index < 128; index++)
    {
        BRCM_WRITE_REG(base, SDT_SDT_COEF_DATA, coeff[index]);
    }

    //Enable loading the filter as we are updating the filter coefficients
    reg_val &= (~SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_stpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
//
// Description:  Set the gain on Sidetone path
//
// Parameters:   handle - audio chal handle.
//                      gain   - gain valaue.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_stpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32     reg_val = 0;

    reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);

    //Disable gain loading
    reg_val &= (~SDT_SDT_CTL_TARGET_GAIN_LOAD_MASK);
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    //Set the Gain
    reg_val &= (~SDT_SDT_CTL_TARGET_GAIN_MASK);
    reg_val |= ((gain << SDT_SDT_CTL_TARGET_GAIN_SHIFT) & SDT_SDT_CTL_TARGET_GAIN_MASK);
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    //Enable gain loading
    reg_val |= (SDT_SDT_CTL_TARGET_GAIN_LOAD_MASK);
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle, cUInt16 enable, cUInt16 linear, cUInt16 slope)
//
// Description:  Set the soft slope gain parameters
//
// Parameters:   handle - audio chal handle.
//                      sof_slope   - Soft Slope (ENABLE)
//                      linear      - Linear increment (ENABLE), Logerithemic Increment (DISABLE)
//                      slope       - Step size per sample
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle, cUInt16 sof_slope, cUInt16 linear, cUInt32 slope)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32     reg_val = 0;

    //Update the soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
    reg_val &= (~SDT_SDT_CTRL_2_SOF_SLOPE_MASK);
    reg_val |= ((slope << SDT_SDT_CTRL_2_SOF_SLOPE_SHIFT)&SDT_SDT_CTRL_2_SOF_SLOPE_MASK);
    BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

    //Update the linear and soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
    reg_val &= (~SDT_SDT_CTL_SOF_SLOPE_LIN_MASK);
    reg_val &= (~SDT_SDT_CTL_SOF_SLOPE_ENABLE_MASK);
    if(linear == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (SDT_SDT_CTL_SOF_SLOPE_LIN_MASK);
    }
    if(sof_slope == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (SDT_SDT_CTL_SOF_SLOPE_ENABLE_MASK);
    }
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle, cUInt16 clipping, cUInt16 dis_filter, cUInt16 gain_bypass)
//
// Description:  Set the soft slope gain parameters
//
// Parameters:   handle - audio chal handle.
//                      gain   - gain valaue.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_stpath_config_misc(CHAL_HANDLE handle, cUInt16 clipping, cUInt16 dis_filter, cUInt16 gain_bypass)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32     reg_val = 0;

    //Update the soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
    reg_val &= (~SDT_SDT_CTRL_2_CLIP_DISABLE_MASK);
    if(clipping == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (SDT_SDT_CTRL_2_CLIP_DISABLE_MASK);
    }
    BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

    //Update the linear and soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
    reg_val &= (~SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);
    reg_val &= (~SDT_SDT_CTL_BYPASS_DIG_GAIN_MASK);

    if(dis_filter == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);
    }

    if(gain_bypass == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (SDT_SDT_CTL_BYPASS_DIG_GAIN_MASK);
    }
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_stpath_set_filter_taps(CHAL_HANDLE handle, cUInt16 lower_taps, cUInt16 upper_taps)
//
// Description:  Set the soft slope gain parameters
//
// Parameters:   handle - audio chal handle.
//                      gain   - gain valaue.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_stpath_set_filter_taps(CHAL_HANDLE handle, cUInt16 lower_taps, cUInt16 upper_taps)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->sdt_base;
    cUInt32     reg_val = 0;

    //Update the soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
    reg_val &= (~SDT_SDT_CTRL_2_UPPER_FIT_TAP_MASK);
    reg_val |= (upper_taps << SDT_SDT_CTRL_2_UPPER_FIT_TAP_SHIFT);
    BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

    //Update the linear and soft slope
    reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
    reg_val &= (~SDT_SDT_CTL_LOWER_FIT_TAP_MASK);
    reg_val |= (lower_taps << SDT_SDT_CTL_LOWER_FIT_TAP_SHIFT);
    BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

    return;
}

