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
*  @file   chal_vout.c
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
#define     CHAL_AUDIO_AUXIOTX_SR_CTRL_MASK     (CHAL_AUDIO_AUDIOTX_SR_SLOPTECTRL            |\
                                                CHAL_AUDIO_AUDIOTX_SR_EXT_POPCLICK          |\
                                                CHAL_AUDIO_AUDIOTX_SR_END_PWRUP             |\
                                                CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M           |\
                                                CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M          |\
                                                CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN           |\
                                                CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE             |\
                                                CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE             |\
                                                CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG)

#define     CHAL_AUDIO_AUXIOTX_ISO_CTRL_MASK    (CHAL_AUDIO_AUDIOTX_ISO_IN |CHAL_AUDIO_AUDIOTX_ISO_OUT)


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
// Function Name: cVoid chal_audio_earpath_enable(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  Enable or Disable earpiece path
//
// Parameters:   handle - audio chal handle.
//                 enable - true : enable, false : disable.
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_enable(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;
    cUInt32 Channl_Ctrl = 0x00000000;

    if(enable == CHAL_AUDIO_ENABLE)
    {
        Channl_Ctrl |= AUDIOH_DAC_CTL_VOUT_ENABLE_MASK;
    }
    else
    {
        Channl_Ctrl &= ~AUDIOH_DAC_CTL_VOUT_ENABLE_MASK;
    }

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);
    reg_val &= ~(AUDIOH_DAC_CTL_VOUT_ENABLE_MASK);
    reg_val |= Channl_Ctrl;


    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DAC_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable )
//
// Description:  Enable or Disable earpiece path interrupt
//
// Parameters:   handle      - audio chal handle.
//                 int_enable  - true : enable interrupt, false : disable interrupt.
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable  )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INTC);

    if(thr_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_VOUT_INTEN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_VOUT_INTEN_MASK;
    }

    if(err_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_VOUT_FIFO_ERRINT_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_VOUT_FIFO_ERRINT_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INTC, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_dma_enable(CHAL_HANDLE handle, cUInt16 dma_enable)
//
// Description:  Enable or Disable DMA on earpiece path
//
// Parameters:   handle     - audio chal handle.
//                 dma_enable - true: enable DMA, false : disable DMA.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable )
{
     cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_DMA_CTL);

    if(dma_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DMA_CTL_VOUT_DMA_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DMA_CTL_VOUT_DMA_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DMA_CTL, reg_val);

    return;
}
//============================================================================
//
// Function Name: cVoid chal_audio_earpath_mute(CHAL_HANDLE handle, Boolean mute)
//
// Description:  Mute or unmute on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 mute   - true: mute opeation, false : unmute opeartion.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_mute(CHAL_HANDLE handle,  Boolean mute)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);

    if(mute == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DAC_CTL_VOUT_MUTE_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DAC_CTL_VOUT_MUTE_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DAC_CTL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
//
// Description:  Set the gain on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 gain   - gain valaue.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
{
    cUInt32     reg_val;
    cUInt32     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EP_DAC_CTRL);

    /* Mask the gain bits */
    reg_val &= 0xFE7FFFFF;
    reg_val |= ((gain&0x03) << 23);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EP_DAC_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
//
// Description:  Set the sample bits on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 bits   - sample bits value.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);

    if(bits >= 24)
    {
        reg_val |= AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_24BIT_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_24BIT_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode)
//
// Description:  Set the playback mode on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 mode   - true : stereo, false : mono.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode)
{

    return;
}
//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_pack(CHAL_HANDLE handle, Boolean pack)
//
// Description:  Set the sample pack/upack mode on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 pack   - true : pack, false : unpack.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_pack(CHAL_HANDLE handle, Boolean pack)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);

    if(pack == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_PACK_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_PACK_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_clr_fifo(CHAL_HANDLE handle)
//
// Description:  Clear the FIFO on earpiece path
//
// Parameters:   handle - audio chal handle.
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_clr_fifo(CHAL_HANDLE handle)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);
    reg_val |= AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    reg_val &= ~AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
//
// Description:  Set   the threshold for FIFO on earpiece path
//
// Parameters:   handle  - audio chal handle.
//                 thres   - threshold 1
//                 thres_2 - threshold 2
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);
    reg_val &= ~(AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES_MASK);
    reg_val |= (thres <<AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES_SHIFT);

    reg_val &= ~(AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES2_MASK);
    reg_val |= (thres_2 << AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES2_SHIFT);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

//============================================================================
//
// Function Name: cUInt32 chal_audio_earpath_read_fifo_status(CHAL_HANDLE handle)
//
// Description:  Read the FIFO status (including interrupts) on earpiece path
//
// Parameters:   handle - audio chal handle.
//
// Return:       the status.
//
//============================================================================

cUInt32 chal_audio_earpath_read_fifo_status(CHAL_HANDLE handle)
{
    cUInt32     status = 0;
    cUInt32     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    // Get the Ear Path FIFO status
    status = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_STATUS);

    //Combine the Ear Path FIFO interrupt status
    status |= chal_audio_earpath_read_int_status(handle);

    return    status;
}

//============================================================================
//
// Function Name: cUInt32 chal_audio_earpath_read_int_status(CHAL_HANDLE handle)
//
// Description:  Read the interrupt status on earpiece path
//
// Parameters:   handle - audio chal handle.
//
// Return:       the interrupt status.
//
//============================================================================

cUInt32 chal_audio_earpath_read_int_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     status = 0;
    cUInt32     reg_val = 0;

    //Read Interrupt status of all paths
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    // Check for Ear path FIFO error interrupt
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VOUT_FIFO_ERR_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    //Check for Ear Path FIFO thereshold interrupt
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VOUT_INT_MASK)
    {

        status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

//============================================================================
//
// Function Name: chal_audio_earpath_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length)
//
// Description:  Write the data to FIFO on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 *src   - the data buffer address
//                 length - the data buffer length
//                 ign_ovf  - keep writing to the fifo even on near overflow
//
// Return:       None.
//
//============================================================================

cUInt32 chal_audio_earpath_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_ovf)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32      n;

    if(ign_ovf == CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_earpath_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_FULL)
            {
                break;
            }
            BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_DATA, ((cUInt32) *src++));
        }
    }
    else
    {

        for(n = 0; n < length; n++)
        {
            BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_DATA, ((cUInt32) *src++));
        }
    }
    return    n;
}

//============================================================================
//
// Function Name: chal_audio_earpath_eanc_in(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  control the eanc into headset path
//
// Parameters:   handle - audio chal handle.
//                 enable - true : with in, false : without
//
// Return:       None.
//
//============================================================================

// Check ASIC if it could be controlled for each output channel

cVoid chal_audio_earpath_eanc_in(CHAL_HANDLE handle, cUInt16 enable )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     eanc_ctrl;

    // Set the required settings
    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);
    eanc_ctrl &= (~AUDIOH_EANC_CTL_VOUT_EANC_ENABLE_MASK);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        eanc_ctrl |= AUDIOH_EANC_CTL_VOUT_EANC_ENABLE_MASK;
    }
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

   return;
}

//============================================================================
//
// Function Name: chal_audio_earpath_sidetone_in(CHAL_HANDLE handle, cUInt16 enable)
//
// Description:  control the sidetone into heaset path
//
// Parameters:   handle - audio chal handle.
//                 enable - true : with in, false : without
//
// Return:       None.
//
//============================================================================

// Confirmed with ASIC that this feature will be added

cVoid chal_audio_earpath_sidetone_in(CHAL_HANDLE handle, cUInt16 enable )
{

    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     reg_val = 0;

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);
    reg_val &= (~AUDIOH_DAC_CTL_VOUT_SIDETONE_EN_MASK);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DAC_CTL_VOUT_SIDETONE_EN_MASK;
    }
    BRCM_WRITE_REG(base, AUDIOH_DAC_CTL, reg_val);

    return;
}
//============================================================================
//
// Function Name: cVoid chal_audio_earpath_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 vout_dither_poly)
//
// Description:  Write the dither poly to sigma delta module on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 vout_dither_poly - dither poly value
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 vout_dither_poly)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_DITHER_POLY, (vout_dither_poly | AUDIOH_VOUT_DITHER_POLY_VOUT_DITHER_SET_MASK));

    return;
}

//============================================================================
//
// Function Name: chal_audio_earpath_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 vout_dither_seed)
//
// Description:  Write the dither seed to sigma delta module on earpiece path
//
// Parameters:   handle - audio chal handle.
//                 vout_dither_seed - dither seed value
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 vout_dither_seed)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_DITHER_SEED, vout_dither_seed);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_get_dma_port_addr(CHAL_HANDLE handle,
//                                    UInt32 *dma_addr)
//
// Description:  Enable or Disable DMA on earpiece path
//
// Parameters:   handle     - audio chal handle.
//                 dma_enable - true: enable DMA, false : disable DMA.
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

	*dma_addr = (UInt32)(base) + AUDIOH_VOUT_FIFO_DATA_OFFSET;

    return;
}

//============================================================================
//
// Function Name: chal_audio_earpath_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef )
//
// Description:  Set SDM coefficient on VOUT path
//
// Parameters:   handle - audio chal handle.
///                 SDM_coef - pointer to the U IIR coefficients
// Return:       none
//
//============================================================================

cVoid chal_audio_earpath_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_SDM_COEF, *SDM_coef);

    return;
}


//============================================================================
//
// Function Name: cVoid chal_audio_earpath_int_clear(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable )
//
// Description:  Clear interrupt on voice out path
//
// Parameters:   handle      : the voice out pathhandle.
//                      thr_int      : Clear FIFO threshold interrupt
//                      err_int      : Clear FIFO Error interrupt
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_int_clear(CHAL_HANDLE handle, Boolean thr_int, Boolean err_int)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val = 0;

    if(thr_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_VOUT_INT_MASK;
    }

    if(err_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_VOUT_FIFO_ERR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}


//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_dac_pwr(CHAL_HANDLE handle,
//                                    cUInt16 enable_chan)
//
// Description:  Set DAC power for the EarPiece DAC
//
// Parameters:   handle     - audio chal handle.
//               enable     - Specifies which DAC channel power need to be enabled or disabled
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;


    if(enable&CHAL_AUDIO_ENABLE)
    {
        /* Clear power down for the DAC */
        reg_value = BRCM_READ_REG(base, AUDIOH_EP_PWR);
        reg_value &= ~AUDIOH_EP_PWR_AUDIOTX_EP_DAC_PD_MASK;
        reg_value &= ~AUDIOH_EP_PWR_AUDIOTX_EP_IHF_REF_PD_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_PWR, reg_value);

    }
    else
    {
        /* Set power down DAC */
        reg_value = BRCM_READ_REG(base, AUDIOH_EP_PWR);
        reg_value |= AUDIOH_EP_PWR_AUDIOTX_EP_DAC_PD_MASK;
        reg_value |= AUDIOH_EP_PWR_AUDIOTX_EP_IHF_REF_PD_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_PWR, reg_value);

    }

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_drv_pwr(CHAL_HANDLE handle,  cUInt16 enable)
//
//
// Description:  Set Driver power for the EarPiece
//
// Parameters:   handle     - audio chal handle.
//                      enable   - Specifies which DAC channel power need to be enabled or disabled
//
// Return:       None.
//
//============================================================================

cVoid chal_audio_earpath_set_drv_pwr(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;


    if(enable&CHAL_AUDIO_ENABLE)
    {
        /* Clear power down for the DRV */
        reg_value = BRCM_READ_REG(base, AUDIOH_EP_DRV);
        reg_value &= ~AUDIOH_EP_DRV_AUDIOTX_EP_DRV_PD_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_DRV, reg_value);
    }
    else
    {
        /* Shut down for the DRV */
        reg_value = BRCM_READ_REG(base, AUDIOH_EP_DRV);
        reg_value |= AUDIOH_EP_DRV_AUDIOTX_EP_DRV_PD_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_DRV, reg_value);
    }


    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
//
// Description:  Set AudioTx Slow Ramp Generator Controls
//
// Parameters:   handle - audio chal handle.
//                      sr_ctrl - Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_set_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_SR);

    reg_value |= (sr_ctrl & CHAL_AUDIO_AUXIOTX_SR_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_SR, reg_value);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_clear_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
//
// Description:  Clear AudioTx Slow Ramp Generator Controls
//
// Parameters:   handle - audio chal handle.
//                      sr_ctrl - Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_clear_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_SR);

    reg_value &= ~(sr_ctrl & CHAL_AUDIO_AUXIOTX_SR_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_SR, reg_value);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_set_isolation_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
//
// Description:  Set AudioTx Slow Ramp Generator Controls
//
// Parameters:   handle - audio chal handle.
//                      sr_ctrl - Isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_set_isolation_ctrl(CHAL_HANDLE handle, cUInt16 iso_ctrl)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_ISO);

    reg_value |= (iso_ctrl & CHAL_AUDIO_AUXIOTX_ISO_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_ISO, reg_value);

    return;
}

//============================================================================
//
// Function Name: cVoid chal_audio_earpath_clear_isolation_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl)
//
// Description:  Clear AudioTx isolation Controls
//
// Parameters:   handle - audio chal handle.
//                      sr_ctrl - isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
//
// Return:       None.
//
//============================================================================
cVoid chal_audio_earpath_clear_isolation_ctrl(CHAL_HANDLE handle, cUInt16 iso_ctrl)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_ISO);

    reg_value &= ~(iso_ctrl & CHAL_AUDIO_AUXIOTX_ISO_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_ISO, reg_value);

    return;
}

