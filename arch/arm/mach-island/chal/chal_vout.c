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
*  @file   chal_vout.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include <chal/chal_audio.h>
#include <chal/chal_audio_int.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_util.h>

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
 *  Function Name: void chal_audio_earpath_enable(CHAL_HANDLE handle, uint16_t enable)
 * 
 *  Description:  Enable or Disable earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  enable - true : enable, false : disable.
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_enable(CHAL_HANDLE handle, uint16_t enable)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;
    uint32_t Channl_Ctrl = 0x00000000;

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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_int_enable(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
 * 
 *  Description:  Enable or Disable earpiece path interrupt
 * 
 *  Parameters:   handle      - audio chal handle.
 *                  int_enable  - true : enable interrupt, false : disable interrupt.
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_int_enable(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable  )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_dma_enable(CHAL_HANDLE handle, uint16_t dma_enable)
 * 
 *  Description:  Enable or Disable DMA on earpiece path
 * 
 *  Parameters:   handle     - audio chal handle.
 *                  dma_enable - true: enable DMA, false : disable DMA.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_dma_enable(CHAL_HANDLE handle, _Bool dma_enable )
{
     uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

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
/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_mute(CHAL_HANDLE handle, _Bool mute)
 * 
 *  Description:  Mute or unmute on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  mute   - true: mute opeation, false : unmute opeartion.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_mute(CHAL_HANDLE handle,  _Bool mute)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_gain(CHAL_HANDLE handle, uint32_t gain)
 * 
 *  Description:  Set the gain on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  gain   - gain valaue.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_gain(CHAL_HANDLE handle, uint32_t gain)
{
    uint32_t     reg_val;
    uint32_t     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_EP_DAC_CTRL);

    /* Mask the gain bits */
    reg_val &= 0xFE7FFFFF;
    reg_val |= ((gain&0x03) << 23);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_EP_DAC_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_bits_per_sample(CHAL_HANDLE handle, uint16_t bits)
 * 
 *  Description:  Set the sample bits on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  bits   - sample bits value.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_bits_per_sample(CHAL_HANDLE handle, uint16_t bits)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_mono_stereo(CHAL_HANDLE handle, _Bool mode)
 * 
 *  Description:  Set the playback mode on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  mode   - true : stereo, false : mono.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_mono_stereo(CHAL_HANDLE handle, _Bool mode)
{
   (void) handle;
   (void) mode;
   return;
}
/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_pack(CHAL_HANDLE handle, _Bool pack)
 * 
 *  Description:  Set the sample pack/upack mode on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  pack   - true : pack, false : unpack.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_pack(CHAL_HANDLE handle, _Bool pack)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_clr_fifo(CHAL_HANDLE handle)
 * 
 *  Description:  Clear the FIFO on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_clr_fifo(CHAL_HANDLE handle)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);
    reg_val |= AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    reg_val &= ~AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_fifo_thres(CHAL_HANDLE handle, uint16_t thres, uint16_t thres_2)
 * 
 *  Description:  Set   the threshold for FIFO on earpiece path
 * 
 *  Parameters:   handle  - audio chal handle.
 *                  thres   - threshold 1
 *                  thres_2 - threshold 2
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_fifo_thres(CHAL_HANDLE handle, uint16_t thres, uint16_t thres_2)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_CTRL);
    reg_val &= ~(AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES_MASK);
    reg_val |= (thres <<AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES_SHIFT);

    reg_val &= ~(AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES2_MASK);
    reg_val |= (thres_2 << AUDIOH_VOUT_FIFO_CTRL_VOUT_FIFO_THRES2_SHIFT);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: uint32_t chal_audio_earpath_read_fifo_status(CHAL_HANDLE handle)
 * 
 *  Description:  Read the FIFO status (including interrupts) on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       the status.
 * 
 * ============================================================================
 */

uint32_t chal_audio_earpath_read_fifo_status(CHAL_HANDLE handle)
{
    uint32_t     status = 0;
    uint32_t     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    /* Get the Ear Path FIFO status*/
    status = BRCM_READ_REG(base, AUDIOH_VOUT_FIFO_STATUS);

    /*Combine the Ear Path FIFO interrupt status*/
    status |= chal_audio_earpath_read_int_status(handle);

    return    status;
}

/*
 * ============================================================================
 * 
 *  Function Name: uint32_t chal_audio_earpath_read_int_status(CHAL_HANDLE handle)
 * 
 *  Description:  Read the interrupt status on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       the interrupt status.
 * 
 * ============================================================================
 */

uint32_t chal_audio_earpath_read_int_status(CHAL_HANDLE handle)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     status = 0;
    uint32_t     reg_val = 0;

    /*Read Interrupt status of all paths*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    /* Check for Ear path FIFO error interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VOUT_FIFO_ERR_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    /*Check for Ear Path FIFO thereshold interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VOUT_INT_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_earpath_write_fifo(CHAL_HANDLE handle, uint32_t *src, uint32_t length)
 * 
 *  Description:  Write the data to FIFO on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  *src   - the data buffer address
 *                  length - the data buffer length
 *                  ign_ovf  - keep writing to the fifo even on near overflow
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

uint32_t chal_audio_earpath_write_fifo(CHAL_HANDLE handle, uint32_t *src, uint32_t length, _Bool ign_ovf)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t      n;

    if(ign_ovf == CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_earpath_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_FULL)
            {
                break;
            }
            BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_DATA, ((uint32_t) *src++));
        }
    }
    else
    {
        for(n = 0; n < length; n++)
        {
            BRCM_WRITE_REG(base,  AUDIOH_VOUT_FIFO_DATA, ((uint32_t) *src++));
        }
    }
    return    n;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_earpath_eanc_in(CHAL_HANDLE handle, uint16_t enable)
 * 
 *  Description:  control the eanc into headset path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  enable - true : with in, false : without
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

/* Check ASIC if it could be controlled for each output channel*/

void chal_audio_earpath_eanc_in(CHAL_HANDLE handle, uint16_t enable )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     eanc_ctrl;

    /* Set the required settings*/
    eanc_ctrl = BRCM_READ_REG(base, AUDIOH_EANC_CTL);
    eanc_ctrl &= (~AUDIOH_EANC_CTL_VOUT_EANC_ENABLE_MASK);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        eanc_ctrl |= AUDIOH_EANC_CTL_VOUT_EANC_ENABLE_MASK;
    }
    BRCM_WRITE_REG(base, AUDIOH_EANC_CTL, eanc_ctrl);

   return;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_earpath_sidetone_in(CHAL_HANDLE handle, uint16_t enable)
 * 
 *  Description:  control the sidetone into heaset path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  enable - true : with in, false : without
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

/* Confirmed with ASIC that this feature will be added*/

void chal_audio_earpath_sidetone_in(CHAL_HANDLE handle, uint16_t enable )
{

    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     reg_val = 0;

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);
    reg_val &= (~AUDIOH_DAC_CTL_VOUT_SIDETONE_EN_MASK);
    if(enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DAC_CTL_VOUT_SIDETONE_EN_MASK;
    }
    BRCM_WRITE_REG(base, AUDIOH_DAC_CTL, reg_val);

    return;
}
/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_sdm_set_dither_poly(CHAL_HANDLE handle, uint32_t vout_dither_poly)
 * 
 *  Description:  Write the dither poly to sigma delta module on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  vout_dither_poly - dither poly value
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_sdm_set_dither_poly(CHAL_HANDLE handle, uint32_t vout_dither_poly)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_DITHER_POLY, (vout_dither_poly | AUDIOH_VOUT_DITHER_POLY_VOUT_DITHER_SET_MASK));

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_earpath_sdm_set_dither_seed(CHAL_HANDLE handle, uint32_t vout_dither_seed)
 * 
 *  Description:  Write the dither seed to sigma delta module on earpiece path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  vout_dither_seed - dither seed value
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_sdm_set_dither_seed(CHAL_HANDLE handle, uint32_t vout_dither_seed)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_DITHER_SEED, vout_dither_seed);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_get_dma_port_addr(CHAL_HANDLE handle,
 *                                     uint32_t *dma_addr)
 * 
 *  Description:  Enable or Disable DMA on earpiece path
 * 
 *  Parameters:   handle     - audio chal handle.
 *                  dma_enable - true: enable DMA, false : disable DMA.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_get_dma_port_addr(CHAL_HANDLE handle, uint32_t *dma_addr)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

	*dma_addr = (uint32_t)(base) + AUDIOH_VOUT_FIFO_DATA_OFFSET;

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: uint32_t chal_audio_earpath_get_dma_port_addr_offset()
 * 
 *  Description:  Get the DMA address offset on ear path
 * 
 *  Return:       Offset for the DMA address.
 * 
 * ============================================================================
 */
uint32_t chal_audio_earpath_get_dma_port_addr_offset( void )
{
   return AUDIOH_VOUT_FIFO_DATA_OFFSET;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_earpath_sdm_set_coef(CHAL_HANDLE handle, uint32_t *SDM_coef )
 * 
 *  Description:  Set SDM coefficient on VOUT path
 * 
 *  Parameters:   handle - audio chal handle.
 */
/**                 SDM_coef - pointer to the U IIR coefficients*/
/*
 *  Return:       none
 * 
 * ============================================================================
 */

void chal_audio_earpath_sdm_set_coef(CHAL_HANDLE handle, uint32_t *SDM_coef )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VOUT_SDM_COEF, *SDM_coef);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_int_clear(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
 * 
 *  Description:  Clear interrupt on voice out path
 * 
 *  Parameters:   handle      : the voice out pathhandle.
 *                       thr_int      : Clear FIFO threshold interrupt
 *                       err_int      : Clear FIFO Error interrupt
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_int_clear(CHAL_HANDLE handle, _Bool thr_int, _Bool err_int)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val = 0;

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


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_dac_pwr(CHAL_HANDLE handle,
 *                                     uint16_t enable_chan)
 * 
 *  Description:  Set DAC power for the EarPiece DAC
 * 
 *  Parameters:   handle     - audio chal handle.
 *                enable     - Specifies which DAC channel power need to be enabled or disabled
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_dac_pwr(CHAL_HANDLE handle, uint16_t enable)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;

    reg_value = BRCM_READ_REG(base, AUDIOH_EP_PWR) & ~AUDIOH_EP_PWR_AUDIOTX_EP_RESET_MASK;

    if(enable&CHAL_AUDIO_ENABLE)
    {
        /* Clear power down for the DAC */
        reg_value &= ~AUDIOH_EP_PWR_AUDIOTX_EP_DAC_PD_MASK;
        reg_value &= ~AUDIOH_EP_PWR_AUDIOTX_EP_IHF_REF_PD_MASK;
        reg_value &= ~AUDIOH_EP_PWR_AUDIOTX_EP_RESET_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_PWR, reg_value);
    }
    else
    {
        /* Set power down DAC */
        reg_value |= AUDIOH_EP_PWR_AUDIOTX_EP_DAC_PD_MASK;
        reg_value |= AUDIOH_EP_PWR_AUDIOTX_EP_IHF_REF_PD_MASK;
        reg_value |= AUDIOH_EP_PWR_AUDIOTX_EP_RESET_MASK;
        BRCM_WRITE_REG(base, AUDIOH_EP_PWR, reg_value);
    }

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_drv_pwr(CHAL_HANDLE handle,  uint16_t enable)
 * 
 * 
 *  Description:  Set Driver power for the EarPiece
 * 
 *  Parameters:   handle     - audio chal handle.
 *                       enable   - Specifies which DAC channel power need to be enabled or disabled
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_earpath_set_drv_pwr(CHAL_HANDLE handle, uint16_t enable)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;


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

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_slowramp_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
 * 
 *  Description:  Set AudioTx Slow Ramp Generator Controls
 * 
 *  Parameters:   handle - audio chal handle.
 *                       sr_ctrl - Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_set_slowramp_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_SR);

    reg_value |= (sr_ctrl & CHAL_AUDIO_AUXIOTX_SR_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_SR, reg_value);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_clear_slowramp_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
 * 
 *  Description:  Clear AudioTx Slow Ramp Generator Controls
 * 
 *  Parameters:   handle - audio chal handle.
 *                       sr_ctrl - Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_clear_slowramp_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_SR);

    reg_value &= ~(sr_ctrl & CHAL_AUDIO_AUXIOTX_SR_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_SR, reg_value);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_isolation_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
 * 
 *  Description:  Set AudioTx Slow Ramp Generator Controls
 * 
 *  Parameters:   handle - audio chal handle.
 *                       sr_ctrl - Isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_set_isolation_ctrl(CHAL_HANDLE handle, uint16_t iso_ctrl)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_ISO);

    reg_value |= (iso_ctrl & CHAL_AUDIO_AUXIOTX_ISO_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_ISO, reg_value);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_clear_isolation_ctrl(CHAL_HANDLE handle, uint16_t sr_ctrl)
 * 
 *  Description:  Clear AudioTx isolation Controls
 * 
 *  Parameters:   handle - audio chal handle.
 *                       sr_ctrl - isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_earpath_clear_isolation_ctrl(CHAL_HANDLE handle, uint16_t iso_ctrl)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_value;

    /* Get the current setting  */
    reg_value = BRCM_READ_REG(base, AUDIOH_AUDIOTX_ISO);

    reg_value &= ~(iso_ctrl & CHAL_AUDIO_AUXIOTX_ISO_CTRL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base, AUDIOH_AUDIOTX_ISO, reg_value);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_earpath_set_filter(CHAL_HANDLE handle, uint16_t filter)
 * 
 *  Description:  set Filter type for the ear  path
 * 
 *  Parameters:   handle    - audio chal handle.
 *                  filter    -  0: Linear Phase, 1 Minimum Phase
 *  Return:       None.
 * 
 */

/*============================================================================*/
void chal_audio_earpath_set_filter(CHAL_HANDLE handle, uint16_t filter)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;


    reg_val = BRCM_READ_REG(base, AUDIOH_MIN_PHASE);
    reg_val &= ~(AUDIOH_MIN_PHASE_EP_MIN_PHASE_MASK);

    if(filter & CHAL_AUDIO_MINIMUM_PHASE_FILTER)
    {
        reg_val |= AUDIOH_MIN_PHASE_EP_MIN_PHASE_MASK;
    }
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_MIN_PHASE, reg_val);

    return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_earpath_enable);
EXPORT_SYMBOL(chal_audio_earpath_int_enable);
EXPORT_SYMBOL(chal_audio_earpath_dma_enable);
EXPORT_SYMBOL(chal_audio_earpath_get_dma_port_addr);
EXPORT_SYMBOL(chal_audio_earpath_get_dma_port_addr_offset);
EXPORT_SYMBOL(chal_audio_earpath_mute);
EXPORT_SYMBOL(chal_audio_earpath_set_gain);
EXPORT_SYMBOL(chal_audio_earpath_set_bits_per_sample);
EXPORT_SYMBOL(chal_audio_earpath_set_mono_stereo);
EXPORT_SYMBOL(chal_audio_earpath_set_pack);
EXPORT_SYMBOL(chal_audio_earpath_clr_fifo);
EXPORT_SYMBOL(chal_audio_earpath_set_fifo_thres);
EXPORT_SYMBOL(chal_audio_earpath_read_fifo_status);
EXPORT_SYMBOL(chal_audio_earpath_read_int_status);
EXPORT_SYMBOL(chal_audio_earpath_write_fifo);
EXPORT_SYMBOL(chal_audio_earpath_sdm_set_dither_poly);
EXPORT_SYMBOL(chal_audio_earpath_sdm_set_dither_seed);
EXPORT_SYMBOL(chal_audio_earpath_sidetone_in);
EXPORT_SYMBOL(chal_audio_earpath_eanc_in);
EXPORT_SYMBOL(chal_audio_earpath_int_clear);
EXPORT_SYMBOL(chal_audio_earpath_sdm_set_coef);
EXPORT_SYMBOL(chal_audio_earpath_set_dac_pwr);
EXPORT_SYMBOL(chal_audio_earpath_set_drv_pwr);
EXPORT_SYMBOL(chal_audio_earpath_set_slowramp_ctrl);
EXPORT_SYMBOL(chal_audio_earpath_clear_slowramp_ctrl);
EXPORT_SYMBOL(chal_audio_earpath_set_isolation_ctrl);
EXPORT_SYMBOL(chal_audio_earpath_clear_isolation_ctrl);
EXPORT_SYMBOL(chal_audio_earpath_set_filter);

#endif
