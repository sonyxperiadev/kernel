/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/*****************************************************************************
*
*    (c) 2001-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*  @file   chal_analogmic.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include <chal/chal_audio.h>
#include <chal/chal_audio_int.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <mach/rdb/brcm_rdb_map.h>

/*
 * ****************************************************************************
 *                         G L O B A L   S E C T I O N
 * ****************************************************************************
 */

/*
 * ****************************************************************************
 *  global variable definitions
 * ****************************************************************************
 */


/*
 * ****************************************************************************
 *                          L O C A L   S E C T I O N
 * ****************************************************************************
 */

/*
 * ****************************************************************************
 *  local macro declarations
 * ****************************************************************************
 */


/*
 * ****************************************************************************
 *  local typedef declarations
 * ****************************************************************************
 */



/*
 * ****************************************************************************
 *  local variable definitions
 * ****************************************************************************
 */


/*
 * ****************************************************************************
 *  local function declarations
 * ****************************************************************************
 */



/*
 * ******************************************************************************
 *  local function definitions
 * ******************************************************************************
 */




/*
 * ============================================================================
 * 
 *  Function Name: CHAL_HANDLE chal_audio_init(uint32_t baseAddr)
 * 
 *  Description:   Standard Init entry point for cHal
 *                 first function to call.
 * 
 *  Parameters:
 *                 handle     ---  the Hera audio handle
 *                 mic_input  ---  the annlog microphone select
 *  Return:        none
 * 
 * ============================================================================
 */

void chal_audio_mic_input_select(CHAL_HANDLE handle, uint16_t mic_input)
{

    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(AUDIOH_ADC_CTL_AMIC_EN_MASK);

    if(mic_input == CHAL_AUDIO_ENABLE)
    {
       reg_val |= AUDIOH_ADC_CTL_AMIC_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_mic_pga(CHAL_HANDLE handle, int gain)
 * 
 *  Description:   Set gain on analog microphone path
 * 
 *  Parameters:
 *                 handle ---  the Hera audio handle
 * 				      gain   ---  the gain value
 *  Return:        none
 * 
 * ============================================================================
 */
void chal_audio_mic_pga_set_gain(CHAL_HANDLE handle, uint32_t gain_regval)
{
    uint32_t base = ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
    reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_MASK);
    reg_val |= (gain_regval) << AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_SHIFT;
    BRCM_WRITE_REG(base, AUDIOH_AUDIORX_VRX1, reg_val);
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_mic_pga(CHAL_HANDLE handle, int gain)
 * 
 *  Description:   Set gain on analog microphone path
 * 
 *  Parameters:
 *                 handle ---  the Hera audio handle
 * 				      gain   ---  the gain value
 *  Return:        none
 * 
 * ============================================================================
 */
void chal_audio_mic_pga_get_gain(CHAL_HANDLE handle, uint32_t *gain_regval)
{
    uint32_t base = ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
    reg_val = (reg_val & AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_MASK) >> AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_SHIFT;
    *gain_regval = reg_val;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_mic_adc_standby(CHAL_HANDLE handle, _Bool standby)
 * 
 *  Description:   control ADC standby state on analog microphone path
 * 
 *  Parameters:
 *                 handle  ---  the Hera audio handle
 * 				  standby ---  the gain value
 *  Return:        none
 * 
 * ============================================================================
 */

void chal_audio_mic_adc_standby(CHAL_HANDLE handle, _Bool standby)
{
   (void) handle;
   (void) standby;
	return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_mic_pwrctrl(CHAL_HANDLE handle, _Bool pwronoff)
 * 
 *  Description:   power on/off analog microphone path
 * 
 *  Parameters:
 *                 handle   ---  the Hera audio handle
 * 				  pwronoff ---  on or off selection
 *  Return:        none
 * 
 * ============================================================================
 */

void chal_audio_mic_pwrctrl(CHAL_HANDLE handle, _Bool pwronoff)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    uint32_t reg_val;

    if(pwronoff == TRUE)
    {
        /*0. powerup ACI VREF, BIAS (should be done by caller before)*/

        /*1. power up BiasCore*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_BIAS);
        reg_val |= (AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_BIAS, reg_val);

        /*2. power up AUDIORX_REF, and fast settle, others "0"*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_PWRUP_MASK);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        /*
         * 3.  enable AUXMIC
         * 4. disable AUXMIC force power down
         */

        /*5.  turn on everything and all default to "zero"*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_CMBUF_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_APMCLK_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_LDO_DIG_PWRDN_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

        /*6. power up MAIN MIC*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VMIC);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK);
        reg_val |= (3 << AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VMIC, reg_val);

        /* power up AUDIORX_REF, others "0"*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val &= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        /* AUDIORX_VRX2/AUDIORX_VMIC*/
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX2, 0x00);

    }
    else
    {

        /* power down AUDIORX_REF, others "0"*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        /*6. power down MAIN MIC*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VMIC);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK);
        reg_val |= (0 << AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VMIC, reg_val);

        /*5.  turn off everything*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_CMBUF_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_APMCLK_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_LDO_DIG_PWRDN_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

        /*2. power down AUDIORX_REF, and fast settle*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val &= ~(AUDIOH_AUDIORX_VREF_AUDIORX_VREF_PWRUP_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        /*1. power down BiasCore*/
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_BIAS);
        reg_val &= ~(AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_BIAS, reg_val);

    }

	return;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_handset_mic_select
 * 
 *  Description:   select analog handset mic
 * 
 *  Parameters:
 *                 handle   ---  the  audio handle
 *  Return:        none
 * 
 * ============================================================================
 */
void chal_audio_handset_mic_select(CHAL_HANDLE handle)
{
   uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
   volatile uint32_t reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);

   reg_val &= ~AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_SEL_MIC1B_MIC2_MASK;
   BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_headset_mic_select
 * 
 *  Description:   select analog headset mic
 * 
 *  Parameters:
 *                 handle   ---  the  audio handle
 * 
 *  Return:        none
 * 
 * ============================================================================
 */
void chal_audio_headset_mic_select(CHAL_HANDLE handle)
{
   uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
   volatile uint32_t reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);

   reg_val |= AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_SEL_MIC1B_MIC2_MASK;
   BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_mic_pwrctrl);
EXPORT_SYMBOL(chal_audio_mic_adc_standby);
EXPORT_SYMBOL(chal_audio_mic_pga_set_gain);
EXPORT_SYMBOL(chal_audio_mic_pga_get_gain);
EXPORT_SYMBOL(chal_audio_mic_input_select);
EXPORT_SYMBOL(chal_audio_handset_mic_select);
EXPORT_SYMBOL(chal_audio_headset_mic_select);

#endif
