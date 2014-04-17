/*
 * Copyright 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
/**
*
*  @file   chal_analogtest.c
*
*  @brief  chal layer driver for audio device driver that contains analog testing path functions
*             (API and analog loopback)
*
****************************************************************************/

#include "chal_caph_audioh.h"
#include <chal/chal_audio_int.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <chal/chal_util.h>

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
 *  Function Name: void chal_audio_enable_dac_paths(CHAL_HANDLE handle, cUInt32 dac_mask)
 * 
 *  Description:  Enable the DAC paths specified by the dac_mask for the API Mux
 * 
 *  Parameters:   handle  - audio chal handle
 *                  dac_mask   - bit mask of DAC
 *                  enable - true : enable, false : disable.
 *  Return:       None.
 * 
 *  Note:        Do not call this function several times for enabling different DAC paths. Call it once with all the DAC paths
 *                  that needs to be enabled
 * ============================================================================
 */

void chal_audio_enable_dac_paths(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable)
{
    cUInt32     reg_val;
    cUInt32     reg_dac_mask = 0;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;



    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_LEFT)
    {
        reg_dac_mask |= AUDIOH_DAC_CTL_STEREO_L_ENABLE_MASK;
    }

    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_RIGHT)
    {
        reg_dac_mask |= AUDIOH_DAC_CTL_STEREO_R_ENABLE_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_IHF_LEFT)
    {
        reg_dac_mask |= AUDIOH_DAC_CTL_IHF_L_ENABLE_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_VIBRA)
    {
        reg_dac_mask |= AUDIOH_DAC_CTL_VIBRA_ENABLE_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_EARPIECE)
    {
        reg_dac_mask |= AUDIOH_DAC_CTL_VOUT_ENABLE_MASK;
    }



    /*
     *  Enable/Disable the DAC Paths.
     * It is necessary to enable all the paths at the same time
     */
    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);
    reg_val &= (~reg_dac_mask);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= reg_dac_mask;
    }
    BRCM_WRITE_REG(base, AUDIOH_DAC_CTL, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_enable_adc_paths(CHAL_HANDLE handle, cUInt32 dac_mask)
 * 
 *  Description:  Enable the DAC paths specified by the dac_mask for the API Mux
 * 
 *  Parameters:   handle  - audio chal handle
 *                  adc_mask   - bit mask of ADC
 *                  enable - true : enable, false : disable.
 *  Return:       None.
 * 
 *  Note:        Do not call this function several times for enabling different DAC paths. Call it once with all the DAC paths
 *                  that needs to be enabled
 * ============================================================================
 */

void chal_audio_enable_adc_paths(CHAL_HANDLE handle, cUInt32 adc_mask, cUInt16 enable)
{
    cUInt32     reg_val;
    cUInt32     reg_adc_mask = 0;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;


    if((adc_mask&CHAL_AUDIO_PATH_AMIC1) || (adc_mask&CHAL_AUDIO_PATH_AMIC2))
    {
        reg_adc_mask |= AUDIOH_ADC_CTL_AMIC_EN_MASK;
    }

    if(adc_mask& CHAL_AUDIO_PATH_DMIC1)
    {
        reg_adc_mask |= AUDIOH_ADC_CTL_DMIC1_EN_MASK;
    }

    if(adc_mask& CHAL_AUDIO_PATH_DMIC2)
    {
        reg_adc_mask |= AUDIOH_ADC_CTL_DMIC2_EN_MASK;
    }

    if(adc_mask&CHAL_AUDIO_PATH_DMIC3)
    {
        reg_adc_mask |= AUDIOH_ADC_CTL_DMIC3_EN_MASK;
    }

    if(adc_mask&CHAL_AUDIO_PATH_DMIC4)
    {
        reg_adc_mask |= AUDIOH_ADC_CTL_DMIC4_EN_MASK;
    }

    /*
     *  Enable/Disable the DAC Paths.
     * It is necessary to enable all the paths at the same time
     */
    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= (~reg_adc_mask);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= reg_adc_mask;
    }
    BRCM_WRITE_REG(base, AUDIOH_ADC_CTL, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_api_enable_dac(CHAL_HANDLE handle, cUInt32 dac_mask)
 * 
 *  Description:  Enable the DAC paths specified by the dac_mask for the API Mux
 * 
 *  Parameters:   handle  - audio chal handle
 *                  dac_mask   - bit mask of DAC
 *                  enable - true : enable, false : disable.
 *  Return:       None.
 * 
 *  Note:        Do not call this function several times for enabling different DAC paths. Call it once with all the DAC paths
 *                  that needs to be enabled
 * ============================================================================
 */

void chal_audio_api_enable_dac(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable)
{
    cUInt32     reg_val;
    cUInt32     reg_api_mask = 0;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;



    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_LEFT)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_STEREO_API_EN_MASK;
    }

    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_RIGHT)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_STEREO_API_EN_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_IHF_LEFT)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_IHF_API_EN_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_VIBRA)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_VIBRA_API_EN_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_EARPIECE)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_VOUT_API_EN_MASK;
    }

    /* Enable/Disable the API for the DAC Paths.*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_API);
    reg_val &= (~reg_api_mask);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= reg_api_mask;
    }
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_API, reg_val);

    return;
}



/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_api_set_dac_attenuation(CHAL_HANDLE handle, cUInt32 dac_mask)
 * 
 *  Description:  Enable/disable the DAC paths specified by the dac_mask for the API attenuation
 * 
 *  Parameters:   handle  - audio chal handle
 *                  dac_mask   - bit mask of DAC
 *  Return:       None.
 * 
 *  Notes:       dac_mask of CHAL_AUDIO_PATH_VIBRA has no effect as chip doesn't support this feature for vibra path
 * ============================================================================
 */

void chal_audio_api_set_dac_attenuation(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable)
{
    cUInt32     reg_val;
    cUInt32     reg_dac_mask = 0x00000000;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;


    /* Enable/Disable the API attenuation for the DAC Paths.*/
    if(dac_mask& (CHAL_AUDIO_PATH_HEADSET_LEFT|CHAL_AUDIO_PATH_HEADSET_RIGHT))
    {
        reg_dac_mask |= AUDIOH_AUDIO_API_STEREO_API_ATT_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_IHF_LEFT)
    {
        reg_dac_mask |= AUDIOH_AUDIO_API_IHF_API_ATT_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_EARPIECE)
    {
        reg_dac_mask |= AUDIOH_AUDIO_API_VOUT_API_ATT_MASK;
    }

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_API);
    reg_val &= (~reg_dac_mask);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= (reg_dac_mask);
    }
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_API, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_api_enable_adc(CHAL_HANDLE handle, cUInt32 adc_mask)
 * 
 *  Description:  Enable the ADC paths specified by the adc_mask for the API Mux
 * 
 *  Parameters:   handle  - audio chal handle
 *                  adc_mask   - bit mask of ADC
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_api_enable_adc(CHAL_HANDLE handle, cUInt32 adc_mask, cUInt16 enable)
{
    cUInt32     reg_val;
    cUInt32     reg_api_mask  = 0x0000;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    if(adc_mask&CHAL_AUDIO_PATH_DMIC1)
    {
        reg_api_mask |= AUDIOH_AUDIO_API_VIN_API_EN_MASK;
    }

    /* Enable/Disable the API for the DAC Paths.*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_API);
    reg_val &= (~reg_api_mask);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= reg_api_mask;
    }
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_API, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_loopback_enable(CHAL_HANDLE handle, cUInt16 enable)
 * 
 *  Description:  Enable or Disable analog mic loopback
 * 
 *  Parameters:   handle - audio chal handle.
 *                  enable - true : enable, false : disable.
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_loopback_enable(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;


    reg_val = BRCM_READ_REG(base, AUDIOH_LOOPBACK_CTRL);
    reg_val &= ~(AUDIOH_LOOPBACK_CTRL_LOOPBACK_EN_MASK);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_LOOPBACK_CTRL_LOOPBACK_EN_MASK;
    }


    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_LOOPBACK_CTRL, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_loopback_set_out_paths(CHAL_HANDLE handle, cUInt16 enable)
 * 
 *  Description:  Enable or Disable analog mic loopback to the mentioned dac path
 * 
 *  Parameters:   handle     - audio chal handle.
 *                  dac_mask     - bit mask of DAC
 *                  enable          - true : enable, false : disable.
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_loopback_set_out_paths(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;
    cUInt32 regDacMask = 0;


    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_LEFT)
    {
        regDacMask |= AUDIOH_LOOPBACK_CTRL_STEREO_LOOPBACK_L_MASK;

    }

    if(dac_mask& CHAL_AUDIO_PATH_HEADSET_RIGHT)
    {
        regDacMask |= AUDIOH_LOOPBACK_CTRL_STEREO_LOOPBACK_R_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_IHF_LEFT)
    {
        regDacMask |= AUDIOH_LOOPBACK_CTRL_IHF_LOOPBACK_L_MASK;
    }

    if(dac_mask&CHAL_AUDIO_PATH_EARPIECE)
    {
        regDacMask |= AUDIOH_LOOPBACK_CTRL_VOUT_LOOPBACK_MASK;
    }


    reg_val = BRCM_READ_REG(base, AUDIOH_LOOPBACK_CTRL);

    reg_val &= (~regDacMask);

    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= regDacMask;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_LOOPBACK_CTRL, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_audiotx_set_dac_ctrl(CHAL_HANDLE handle, cUInt32 ctrl)
 * 
 *  Description:  Enable or Disable external pop click for Ear piece path
 * 
 *  Parameters:   handle             - audio chal handle.
 *                       ctrl                  - Configuration that sets AUDIOTX. CLK_INV, CLK_PD, BB_STI & TEST_EN
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_audiotx_set_dac_ctrl(CHAL_HANDLE handle, cUInt32 ctrl)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    ctrl &= (AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_INV_MASK|AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
    ctrl &= (AUDIOH_DAC_CTRL_AUDIOTX_TEST_EN_MASK|AUDIOH_DAC_CTRL_AUDIOTX_BB_STI_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_DAC_CTRL, ctrl);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_audiotx_get_dac_ctrl(CHAL_HANDLE handle)
 * 
 *  Description:  Get the current DAC AUDIOTX configuration
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       Current DAC AUDIOTX setting
 * 
 * ============================================================================
 */
cUInt32 chal_audio_audiotx_get_dac_ctrl(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 ctrl;

    ctrl = BRCM_READ_REG(base, AUDIOH_DAC_CTRL);

    ctrl &= (AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_INV_MASK|AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
    ctrl &= (AUDIOH_DAC_CTRL_AUDIOTX_EP_DRV_STO_MASK|AUDIOH_DAC_CTRL_AUDIOTX_BB_STI_MASK);
    ctrl &= (AUDIOH_DAC_CTRL_AUDIOTX_TEST_EN_MASK);

    return ctrl;
}


/**
*
* Function Name: cVoid chal_audio_audiotx_set_spare_bit(CHAL_HANDLE handle)
*
* Description:  Set the audiotx spare bit
*
* Parameters:   handle             - audio chal handle.
*
* Return:       None.
*
*****************************************************************************/
void chal_audio_audiotx_set_spare_bit(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 ctrl;

    ctrl = BRCM_READ_REG(base, AUDIOH_DAC_CTRL);
    ctrl |= (1<<AUDIOH_DAC_CTRL_AUDIOTX_SPARE_BIT_SHIFT);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_DAC_CTRL, ctrl);

    return;
}

/**
*
* Description:  Enable or Disable dac loopback
*
* Parameters:   handle - audio chal handle.
*               enable - true : enable, false : disable.
*
* Return:       None.
*
*****************************************************************************/
cVoid chal_audio_dac_loopback_enable(CHAL_HANDLE handle, cUInt16 enable)
{
	cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, AUDIOH_LOOPBACK_CTRL);
	if(enable == CHAL_AUDIO_ENABLE) {
		reg_val |= AUDIOH_LOOPBACK_CTRL_ENABLE_DIG_TX_CLK_MASK;
		reg_val |= AUDIOH_LOOPBACK_CTRL_DISABLE_ANA_TX_CLK_MASK;
		reg_val |= AUDIOH_LOOPBACK_CTRL_DISABLE_ANA_RX_CLK_MASK;
		reg_val |= AUDIOH_LOOPBACK_CTRL_DAC_SDM_LOOPBACK_9LEVEL_MASK;
		reg_val |= AUDIOH_LOOPBACK_CTRL_DAC_SDM_LOOPBACK_EN_MASK;
		reg_val &= ~AUDIOH_LOOPBACK_CTRL_LOOPBACK_EN_MASK;
	} else {
		reg_val  &= ~(AUDIOH_LOOPBACK_CTRL_ENABLE_DIG_TX_CLK_MASK |
		AUDIOH_LOOPBACK_CTRL_DISABLE_ANA_TX_CLK_MASK |
		AUDIOH_LOOPBACK_CTRL_DISABLE_ANA_RX_CLK_MASK |
		AUDIOH_LOOPBACK_CTRL_DAC_SDM_LOOPBACK_9LEVEL_MASK|
		AUDIOH_LOOPBACK_CTRL_DAC_SDM_LOOPBACK_EN_MASK);
	}

	/* Set the required setting */
	BRCM_WRITE_REG(base,  AUDIOH_LOOPBACK_CTRL, reg_val);

	return;
}

/**
*
* Description:  Enable or Disable dac loopback
*
* Description:  Enable or Disable analog mic loopback to the mentioned dac path
*
* Parameters:   handle   - audio chal handle.
*               dac_mask - bit mask of DAC
*               enable   - true : enable, false : disable.
*
* Return:       None.
*
*****************************************************************************/
cVoid chal_audio_dac_loopback_set_out_paths(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;
    cUInt32 regDacMask = 0;

    regDacMask |= (dac_mask&0x7) <<AUDIOH_LOOPBACK_CTRL_DAC_SDM_LOOPBACK_SEL_SHIFT;

    reg_val = BRCM_READ_REG(base, AUDIOH_LOOPBACK_CTRL);
    reg_val &= (~regDacMask);

    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= regDacMask;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_LOOPBACK_CTRL, reg_val);

    return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_api_enable_dac);
EXPORT_SYMBOL(chal_audio_api_set_dac_attenuation);
EXPORT_SYMBOL(chal_audio_api_enable_adc);
EXPORT_SYMBOL(chal_audio_loopback_enable);
EXPORT_SYMBOL(chal_audio_loopback_set_out_paths);
EXPORT_SYMBOL(chal_audio_audiotx_set_dac_ctrl);
EXPORT_SYMBOL(chal_audio_audiotx_get_dac_ctrl);
EXPORT_SYMBOL(chal_audio_audiotx_set_spare_bit);
EXPORT_SYMBOL(chal_audio_dac_loopback_enable);
EXPORT_SYMBOL(chal_audio_dac_loopback_set_out_paths);
#endif
