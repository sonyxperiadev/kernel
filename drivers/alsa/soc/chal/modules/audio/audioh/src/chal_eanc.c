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
*  @file   chal_eanc.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include "chal_audio.h"
#include "chal_audio_int.h"
#include "brcm_rdb_audioh.h"
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
// Function Name: cVoid chal_audio_eancpath_enable(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  Enable or Disable EANC path
//
// Parameters:   handle    - audio chal handle.
//                 enable    -  true : enable, false : disable.
//
// Return:       None.
//

//============================================================================
cVoid chal_audio_eancpath_enable(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(AUDIOH_ADC_CTL_EANC_EN_MASK);

    if(enable == CHAL_AUDIO_ENABLE)
    {
       reg_val |= AUDIOH_ADC_CTL_EANC_EN_MASK;
    }
    else
    {
         reg_val &= ~AUDIOH_ADC_CTL_EANC_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable )
//
// Description:  Enable or Disable EANC path interrupt
//
// Parameters:   handle      - audio chal handle.
//                 int_enable  - true : enable interrupt, false : disable interrupt.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable  )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INTC);

    if(fifo_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_EANC_INTEN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_EANC_INTEN_MASK;
    }

    if(err_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_EANC_FIFO_ERRINT_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_EANC_FIFO_ERRINT_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INTC, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_dma_enable(CHAL_HANDLE handle, cUInt16 dma_enable)
//
// Description:  Enable or Disable DMA on EANC path
//
// Parameters:   handle     - audio chal handle.
//                 dma_enable - true: enable DMA, false : disable DMA.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_DMA_CTL);

    if(dma_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DMA_CTL_EANC_DMA_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DMA_CTL_EANC_DMA_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DMA_CTL, reg_val);

    return;
}
//============================================================================
//
// Function Name: cVoid chal_audio_encepath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
//
// Description:  Set the gain on EANC path
//
// Parameters:   handle - audio chal handle.
//                 gain   - gain valaue.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
{

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
//
// Description:  Set the sample bits on EANC path
//
// Parameters:   handle - audio chal handle.
//                 bits   - sample bits value.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
{
   cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_CTRL);

    if(bits >= 24)
    {
        reg_val |= AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_24BIT_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_24BIT_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EANC_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_set_pack(CHAL_HANDLE handle, Boolean pack)
//
// Description:  Set the sample pack/upack mode on EANC path
//
// Parameters:   handle - audio chal handle.
//                 pack   - true : pack, false : unpack.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_set_pack(CHAL_HANDLE handle, Boolean pack)
{
   cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_CTRL);

    if(pack == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_PACK_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_PACK_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EANC_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_clr_fifo(CHAL_HANDLE handle)
//
// Description:  Clhs the FIFO on EANC path
//
// Parameters:   handle - audio chal handle.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_clr_fifo(CHAL_HANDLE handle)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_CTRL);
    reg_val |= AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EANC_FIFO_CTRL, reg_val);

    reg_val &= ~AUDIOH_EANC_FIFO_CTRL_EANC_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EANC_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
//
// Description:  Set   the threshold for FIFO on EANC path
//
// Parameters:   handle  - audio chal handle
//                 thres   - threshold 1
//                 thres_2 - threshold 2
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_CTRL);
    reg_val &= ~(0x7F);
    reg_val &= ~(0xF0);
    reg_val |= thres;
    reg_val |= thres_2 << 8;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EANC_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cUInt32 chal_audio_eancpath_read_int_status(CHAL_HANDLE handle)
//
// Description:  Read the interrupt status
//
// Parameters:   handle      - audio chal handle.
//
// Return:       None.
//
//============================================================================

cUInt32 chal_audio_eancpath_read_int_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     status = 0;
    cUInt32     reg_val = 0;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    if(reg_val & AUDIOH_AUDIO_INT_STATUS_EANC_FIFO_ERR_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    if(reg_val & AUDIOH_AUDIO_INT_STATUS_EANC_INT_MASK)
    {

        status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_clr_int(CHAL_HANDLE handle)
//
// Description:  Read the interrupt status
//
// Parameters:   handle      - audio chal handle.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_clr_int(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);
    reg_val |= (AUDIOH_AUDIO_INT_STATUS_EANC_INT_MASK | AUDIOH_AUDIO_INT_STATUS_EANC_FIFO_ERR_MASK);
    BRCM_WRITE_REG(base, AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}


//============================================================================
//
// Function Name: cUInt32 chal_audio_eancpath_read_fifo_status(CHAL_HANDLE handle)
//
// Description:  Read the FIFO status on EANC path
//
// Parameters:   handle - audio chal handle.
//
// Return:       the status.
//
//============================================================================

cUInt32 chal_audio_eancpath_read_fifo_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 status;

    status = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_STATUS);

    // Add EANC path FIFO interrupt status
    status |= chal_audio_eancpath_read_int_status(handle);

    return status;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_enable_IIR_coef(CHAL_HANDLE handle, Boolean enable)
//
// Description:  Enable the IIR coefficientRead on EANC path
//
// Parameters:   handle - audio chal handle.
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_enable_IIR_coef(CHAL_HANDLE handle, Boolean enable)
{
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_enable_FIR_coef(CHAL_HANDLE handle, Boolean enable)
//
// Description:  Enable the FIR coefficientRead on EANC path
//
// Parameters:   handle - audio chal handle.
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_enable_FIR_coef(CHAL_HANDLE handle, Boolean enable)
{
}
//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_set_mode(CHAL_HANDLE handle, Boolean mode)
//
// Description:  Set up 96k/48k mode on EANC path
//
// Parameters:   handle - audio chal handle.
//                      mode - TRUE: 96k FALSE: 48k
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_mode(CHAL_HANDLE handle, Boolean mode)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     eanc_ctrl;

    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);

    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_MODE96K_MASK);

    if(mode == CHAL_AUDIO_ENABLE)
    {
        eanc_ctrl |= (AUDIOH_EANC_CTL_EANC_MODE96K_MASK);
    }
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_set_input_mic(CHAL_HANDLE handle, cUInt16 dmic)
//
// Description:  Select DM1~DM4 to input to EANC path
//
// Parameters:   handle - audio chal handle.
//               dmic    -   0: mic1, 1 : mic2, 2 : mic3,  3 :  mic4
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_input_mic(CHAL_HANDLE handle, cUInt16 dmic)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     eanc_ctrl;

    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);

    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_MIC_SEL_MASK);
    eanc_ctrl |= (dmic << AUDIOH_EANC_CTL_EANC_MIC_SEL_SHIFT);

    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_ctrl_tap(CHAL_HANDLE handle, cUInt32 tap)
//
// Description:  EANC tap control
//
// Parameters:   handle - audio chal handle.
//               tap    -   see rdb
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_ctrl_tap(CHAL_HANDLE handle, cUInt32 taps)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base, AUDIOH_EANC_TAPS, taps);
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_ctrl_shift(CHAL_HANDLE handle, cUInt32 shift)
//
// Description:  EANC shift register control
//
// Parameters:   handle - audio chal handle.
//               shift  -   see rdb
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_ctrl_shift(CHAL_HANDLE handle, cUInt32 shift)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base, AUDIOH_EANC_SHIFT_CTRL, shift);

    return;
}

//============================================================================
//
// Function Name: chal_audio_eancpath_DIIR_shift(CHAL_HANDLE handle,
//                        cUInt32 shift_H, cUInt32 shift_L)
//
// Description:  Set down sampling IIR shift on EANC path
//
// Parameters:   handle - audio chal handle.
//                 shift_H - EANC down sampling IIR high 24 bits
//               shift_L - EANC down sampling IIR low 24 bits
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_DIIR_shift(CHAL_HANDLE handle, cUInt32 shift_H, cUInt32 shift_L)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base, AUDIOH_EANC_DIIR_SHIFT_L, (shift_L&AUDIOH_EANC_DIIR_SHIFT_L_EANC_DIIR_SHIFT_L_MASK));
    BRCM_WRITE_REG(base, AUDIOH_EANC_DIIR_SHIFT_H, (shift_H&AUDIOH_EANC_DIIR_SHIFT_H_EANC_DIIR_SHIFT_H_MASK));

    return;
}

//============================================================================
//
// Function Name: chal_audio_eancpath_AIIR_shift(CHAL_HANDLE handle,
//                        cUInt32 shift_H, cUInt32 shift_L)
//
// Description:  Set adaptive sampling IIR shift on EANC path
//
// Parameters:   handle - audio chal handle.
//                 shift_H - EANC adaptive sampling IIR high 24 bits
//               shift_L - EANC adaptive sampling IIR low 24 bits
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_AIIR_shift(CHAL_HANDLE handle, cUInt32 shift_H, cUInt32 shift_L)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base, AUDIOH_EANC_AIIR_SHIFT_L, (shift_L&AUDIOH_EANC_AIIR_SHIFT_L_EANC_AIIR_SHIFT_L_MASK));
    BRCM_WRITE_REG(base, AUDIOH_EANC_AIIR_SHIFT_H, (shift_H&AUDIOH_EANC_AIIR_SHIFT_H_EANC_AIIR_SHIFT_H_MASK));

    return;
}
//============================================================================
//
// Function Name: chal_audio_eancpath_UIIR_shift(CHAL_HANDLE handle,
//                        cUInt32 shift_H, cUInt32 shift_L)
//
// Description:  Set up sampling IIR shift on EANC path
//
// Parameters:   handle - audio chal handle.
//                 shift_H    EANC - up sampling IIR high 24 bits
//               shift_L    EANC - up sampling IIR low 24 bits
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_UIIR_shift(CHAL_HANDLE handle, cUInt32 shift_H, cUInt32 shift_L)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base, AUDIOH_EANC_UIIR_SHIFT_L, (shift_L&AUDIOH_EANC_UIIR_SHIFT_L_EANC_UIIR_SHIFT_L_MASK));
    BRCM_WRITE_REG(base, AUDIOH_EANC_UIIR_SHIFT_H, (shift_H&AUDIOH_EANC_UIIR_SHIFT_H_EANC_UIIR_SHIFT_H_MASK));

    return;
}

//============================================================================
//
// Function Name: chal_audio_eancpath_set_cic_gain(CHAL_HANDLE handle,
//                        cUInt16 ucic_gain, cUInt16 dcic_gain)
//
// Description:  Set CIC gain on EANC path
//
// Parameters:   handle - audio chal handle.
//                ucic_gain - Up sample cic output gain
//                ucic_gain - Down sample cic output gain
//
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_cic_gain(CHAL_HANDLE handle, cUInt16 ucic_gain, cUInt16 dcic_gain)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     cic_gain = 0;

    cic_gain = (dcic_gain<<AUDIOH_EANC_CIC_GAIN_EANC_DCIC_GAIN_SHIFT);
    cic_gain &= AUDIOH_EANC_CIC_GAIN_EANC_DCIC_GAIN_MASK;

    cic_gain |= (ucic_gain & AUDIOH_EANC_CIC_GAIN_EANC_UCIC_GAIN_MASK);

    BRCM_WRITE_REG(base, AUDIOH_EANC_CIC_GAIN, cic_gain);

    return;
}
//============================================================================
//
// Function Name: chal_audio_eancpath_set_AIIR_coef(CHAL_HANDLE handle, cUInt32 AIIR_coef )
//
// Description:  Set AIIR coefficient on EANC path
//
// Parameters:   handle - audio chal handle.
///                 AIIR_coef - pointer to the adaptive IIR coefficients
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_AIIR_coef(CHAL_HANDLE handle, cUInt32 *AIIR_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     index;
    cUInt32     eanc_ctrl;

    for(index = 0; index < 20; index++)
    {
        BRCM_WRITE_REG_IDX(base, AUDIOH_EANC_CO_AIIR, index, AIIR_coef[index]);
    }

    //Create a Raising edge pulse for the Coefficients Set bit
    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);

    //Make sure bit is low
    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_AIIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    //Set to high to make a raising edge
    eanc_ctrl |= (AUDIOH_EANC_CTL_EANC_AIIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    //Make sure bit is low, creates a falling edge
    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_AIIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    return;
}

//============================================================================
//
// Function Name: chal_audio_eancpath_set_AFIR_coef(CHAL_HANDLE handle, cUInt32 AFIR_coef )
//
// Description:  Set AFIR coefficient on EANC path
//
// Parameters:   handle - audio chal handle.
///                 AFIR_coef - pointer to the adaptive FIR coefficients
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_AFIR_coef(CHAL_HANDLE handle, cUInt32 *AFIR_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     index;
    cUInt32     eanc_ctrl;

    for(index = 0; index < 10; index++)
    {
        BRCM_WRITE_REG_IDX(base, AUDIOH_EANC_CO_AFIR, index, AFIR_coef[index]);
    }

    //Create a Raising edge pulse for the Coefficients Set bit
    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);

    //Make sure bit is low
    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_AFIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    //Set to high to make a raising edge
    eanc_ctrl |= (AUDIOH_EANC_CTL_EANC_AFIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    //Make sure bit is low, creates a falling edge
    eanc_ctrl &= (~AUDIOH_EANC_CTL_EANC_AFIR_CO_SET_MASK);
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

    return;
}


//============================================================================
//
// Function Name: chal_audio_eancpath_set_DIIR_coef(CHAL_HANDLE handle, cUInt32 DIIR_coef )
//
// Description:  Set DIIR coefficient on EANC path
//
// Parameters:   handle - audio chal handle.
///                 DIIR_coef - pointer to the D IIR coefficients
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_DIIR_coef(CHAL_HANDLE handle, cUInt32 *DIIR_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     index;

    for(index = 0; index < 30; index++)
    {
        BRCM_WRITE_REG_IDX(base, AUDIOH_EANC_CO_DIIR, index, DIIR_coef[index]);
    }

    return;
}

//============================================================================
//
// Function Name: chal_audio_eancpath_set_UIIR_coef(CHAL_HANDLE handle, cUInt32 DIIR_coef )
//
// Description:  Set UIIR coefficient on EANC path
//
// Parameters:   handle - audio chal handle.
///                 UIIR_coef - pointer to the U IIR coefficients
// Return:       none
//
//============================================================================

cVoid chal_audio_eancpath_set_UIIR_coef(CHAL_HANDLE handle, cUInt32 *UIIR_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     index;

    for(index = 0; index < 30; index++)
    {
        BRCM_WRITE_REG_IDX(base, AUDIOH_EANC_CO_UIIR, index, UIIR_coef[index]);
    }

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_read_fifo(CHAL_HANDLE handle,
//                        cUInt32 *src, cUInt32 length)
//
// Description:  Read the data from FIFO on EANC feedback path
//
// Parameters:   handle  the eanc input path handle.
//
//                 *src    - the data buffer address
//                 length  - the data buffer length
//                 ign_udf  - keep reading the fifo even on uderflow
//
// Return:       None.
//
//============================================================================

cUInt32 chal_audio_eancpath_read_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_udf)
{
    UInt32 n;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    if(ign_udf == CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_eancpath_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY)
            {
                break;
            }
            *src++ = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_DATA);
        }
    }
    else
    {
        for(n = 0; n < length; n++)
        {
            *src++ = BRCM_READ_REG(base, AUDIOH_EANC_FIFO_DATA);
        }
    }
    return    n;

}


//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_get_dma_port_addr(CHAL_HANDLE handle,
//                                                                                          UInt32 *dma_addr)
//
// Description:  Get the DMA port address on EANC path
//
// Parameters:   handle     - audio chal handle.
//                    *dma_addr  - the buffer to save dma port address.
// Return:       None.
//
//============================================================================

cVoid chal_audio_eancpath_get_dma_port_addr( CHAL_HANDLE handle,
                                                                                        UInt32 *dma_addr )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    *dma_addr = (UInt32)(base) + AUDIOH_EANC_FIFO_DATA_OFFSET;

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_eancpath_int_clear(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable )
//
// Description:  Clear interrupt on EANC path
//
// Parameters:   handle      : the EANC pathhandle.
//                      thr_int      : Clear FIFO threshold interrupt
//                      err_int      : Clear FIFO Error interrupt
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_eancpath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val = 0;

    if(fifo_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_EANC_INT_MASK;
    }

    if(err_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_EANC_FIFO_ERR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}

