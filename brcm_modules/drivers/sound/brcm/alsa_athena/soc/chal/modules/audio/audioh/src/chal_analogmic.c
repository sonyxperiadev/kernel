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
*  @file   chal_analogmic.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include "chal_audio.h"
#include "chal_audio_int.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_util.h"
#include "brcm_rdb_aci.h"
#include "brcm_rdb_auxmic.h"
#include "brcm_rdb_padctrlreg.h"

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

#define  READ_REG32(reg)            ( *((volatile UInt32 *) (reg)) )
#define  WRITE_REG32(reg, value)   	( *((volatile UInt32 *) (reg)) = (UInt32) (value) )

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
// Function Name: CHAL_HANDLE chal_audio_init(cUInt32 baseAddr)
//
// Description:   Standard Init entry point for cHal
//                first function to call.
//
// Parameters:
//                handle     ---  the Hera audio handle
//                mic_input  ---  the annlog microphone select
// Return:        none
//
//============================================================================

cVoid chal_audio_mic_input_select(CHAL_HANDLE handle, UInt16 mic_input)
{

    cUInt32 regVal = 0;

    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(AUDIOH_ADC_CTL_AMIC_EN_MASK);

    // if(mic_input == CHAL_AUDIO_ENABLE)
    {
       reg_val |= AUDIOH_ADC_CTL_AMIC_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);


	// ACI control for analog microphone

	// WRITE_REG32(0x3500E0D4, 0xC0);
	regVal = READ_REG32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET));
    regVal |= ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK;
    regVal |= ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK;
    WRITE_REG32((ACI_BASE_ADDR+ACI_ADC_CTRL_OFFSET), regVal);

	/* enable AUXMIC */
	
	// WRITE_REG32(0x3500E014, 0x01);
	regVal = READ_REG32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET));
	regVal |= AUXMIC_AUXEN_MICAUX_EN_MASK;
	WRITE_REG32((AUXMIC_BASE_ADDR+AUXMIC_AUXEN_OFFSET), regVal);

	/* disable AUXMIC force power down */
	
	regVal = READ_REG32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET));
	regVal &= ~AUXMIC_F_PWRDWN_FORCE_PWR_DWN_MASK;
	WRITE_REG32((AUXMIC_BASE_ADDR+AUXMIC_F_PWRDWN_OFFSET), regVal);


    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_mic_pga(CHAL_HANDLE handle, int gain)
//
// Description:   Set gain on analog microphone path
//
// Parameters:
//                handle ---  the Hera audio handle
//				  gain   ---  the gain value
// Return:        none
//
//============================================================================
cVoid chal_audio_mic_pga(CHAL_HANDLE handle, int gain)
{
	cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;
	
	reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
	reg_val &= ~AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_MASK;

	if(gain > 0x3f)	gain = 0x3f;
	if(gain < 0x00) gain = 0;

	reg_val |= (gain << AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_SHIFT);

    BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

	return;
}

//============================================================================
//
// Function Name: chal_audio_vinpath_set_digimic_clkdelay(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  Set the delay for sampling the DIGITAL MIC1,2 signals on the DATA line
//
// Parameters:   handle      : the voice input path handle.
//                      delay        : delay in 5.95 usec max possible is 41.6 usec
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_vinpath_mute(CHAL_HANDLE handle, Boolean mute_ctrl)
{

	int mute = mute_ctrl;
	
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

	reg_val  = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
	reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_ADCRST_MASK | AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK);
	reg_val |= (mute << AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_ADCRST_SHIFT);

	if(mute)
	{
		reg_val |= AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK;
	}

	BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

}



//============================================================================
//
// Function Name: cVoid chal_audio_mic_adc_standby(CHAL_HANDLE handle, Boolean standby)
//
// Description:   control ADC standby state on analog microphone path
//
// Parameters:
//                handle  ---  the Hera audio handle
//				  standby ---  the gain value
// Return:        none
//
//============================================================================

cVoid chal_audio_mic_adc_standby(CHAL_HANDLE handle, Boolean standby)
{

	return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_mic_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
//
// Description:   power on/off analog microphone path
//
// Parameters:
//                handle   ---  the Hera audio handle
//				  pwronoff ---  on or off selection
// Return:        none
//
//============================================================================

cVoid chal_audio_mic_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    cUInt32 reg_val;

    if(pwronoff == TRUE)
    {
        //0. powerup ACI VREF, BIAS (should be done by caller before)

        //1. power up BiasCore
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_BIAS);
        reg_val |= (AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_BIAS, reg_val);

        //2. power up AUDIORX_REF, and fast settle, others "0"
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_PWRUP_MASK);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        //3.  enable AUXMIC
        //4. disable AUXMIC force power down

        //5.  turn on everything and all default to "zero"
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_CMBUF_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_APMCLK_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VRX1_AUDIORX_LDO_DIG_PWRDN_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

        //6. power up MAIN MIC
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VMIC);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK);
        reg_val |= (3 << AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VMIC, reg_val);

        // power up AUDIORX_REF, others "0"
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val &= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        // AUDIORX_VRX2/AUDIORX_VMIC
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX2, 0x00);

    }
    else
    {

        // power down AUDIORX_REF, others "0"
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val |= (AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        //6. power down MAIN MIC
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VMIC);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_PWRDN_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK);
        reg_val |= (0 << AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VMIC, reg_val);

        //5.  turn off everything
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VRX1);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_CMBUF_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_APMCLK_PWRDN_MASK);
        reg_val |= (AUDIOH_AUDIORX_VRX1_AUDIORX_LDO_DIG_PWRDN_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VRX1, reg_val);

        //2. power down AUDIORX_REF, and fast settle
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_VREF);
        reg_val &= ~(AUDIOH_AUDIORX_VREF_AUDIORX_VREF_PWRUP_MASK);
        reg_val &= ~(AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_VREF, reg_val);

        //1. power down BiasCore
        reg_val = BRCM_READ_REG(base, AUDIOH_AUDIORX_BIAS);
        reg_val &= ~(AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK);
        BRCM_WRITE_REG(base,  AUDIOH_AUDIORX_BIAS, reg_val);

    }

	return;
}


//============================================================================
//
// Function Name: chal_audio_dmic1_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
//
// Description:    Set DMIC1 CLK and DATA Pad function
//
// Parameters:
//                handle   ---  the Hera audio handle
//				  pwronoff ---  on or off selection
// Return:        none
//
//============================================================================

cVoid chal_audio_dmic1_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
{
    cUInt32  regVal;
    cUInt32   function = 0;

    /* Select the function for DMIC0_CLK */
    /* For function = 0 (alt_fn1), this will be set as DMIC1_CLK */
    regVal = READ_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_DMIC0CLK_OFFSET));
    regVal &= (~PADCTRLREG_DMIC0CLK_PINSEL_DMIC0CLK_MASK);
    regVal |= (function << PADCTRLREG_DMIC0CLK_PINSEL_DMIC0CLK_SHIFT);
    WRITE_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_DMIC0CLK_OFFSET), regVal);

    /* Select the function for DMIC0_DATA */
    /* For function = 0 (alt_fn1), this will be set as DMIC1_DATA */
    regVal = READ_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_DMIC0DQ_OFFSET));
    regVal &= (~PADCTRLREG_DMIC0DQ_PINSEL_DMIC0DQ_MASK);
    regVal |= (function << PADCTRLREG_DMIC0DQ_PINSEL_DMIC0DQ_SHIFT);
    WRITE_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_DMIC0DQ_OFFSET), regVal);

}

//============================================================================
//
// Function Name: chal_audio_dmic2_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
//
// Description:    Set DMIC2 CLK and DATA Pad function
//
// Parameters:
//                handle   ---  the Hera audio handle
//				  pwronoff ---  on or off selection
// Return:        none
//
//============================================================================

cVoid chal_audio_dmic2_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff)
{
    cUInt32  regVal;
    cUInt32   function=4;

    /* Select the function for GPIO33 */
    /* For function = 4 (alt_fn5), this will be set as DMIC2_CLK */
    regVal = READ_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_GPIO33_OFFSET));
    regVal &= (~PADCTRLREG_GPIO33_PINSEL_GPIO33_MASK);
    regVal |= (function << PADCTRLREG_GPIO33_PINSEL_GPIO33_SHIFT);
    WRITE_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_GPIO33_OFFSET), regVal);

    /* Select the function for GPIO34 */
    /* For function = 4 (alt_fn5), this will be set as DMIC2_DATA */
    regVal = READ_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_GPIO34_OFFSET));
    regVal &= (~PADCTRLREG_GPIO34_PINSEL_GPIO34_MASK);
    regVal |= (function << PADCTRLREG_GPIO34_PINSEL_GPIO34_SHIFT);
    WRITE_REG32((PAD_CTRL_BASE_ADDR+PADCTRLREG_GPIO34_OFFSET), regVal);
        /* For FPGA no pads are present */
}


