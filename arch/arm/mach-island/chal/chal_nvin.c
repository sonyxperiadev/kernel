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
*  @file   chal_vin.c
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
 * ============================================================================
 *
 *  Function Name: chal_audio_nvinpath_digi_mic_enable(CHAL_HANDLE handle, uint16_t enable)
 *
 *  Description:  Enable the microphone on voice out path
 *
 *  Parameters:   handle      : the voice input path handle.
 *                  enable      : enable : bit0 - digital microphone 3
 *                              : enable : bit1 - digital microphone 4
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_digi_mic_enable(CHAL_HANDLE handle, uint16_t enable)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(AUDIOH_ADC_CTL_DMIC3_EN_MASK | AUDIOH_ADC_CTL_DMIC4_EN_MASK);

    if(enable&CHAL_AUDIO_CHANNEL_LEFT)
    {
       reg_val |= AUDIOH_ADC_CTL_DMIC4_EN_MASK;
    }

    if(enable&CHAL_AUDIO_CHANNEL_RIGHT)
    {
       reg_val |= AUDIOH_ADC_CTL_DMIC3_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);

    return;

}


/*
 * ============================================================================
 *
 *  Function Name: chal_audio_nvinpath_digi_mic_disable(CHAL_HANDLE handle, uint16_t enable)
 *
 *  Description:  disable the microphone on noise out path
 *
 *  Parameters:   handle      : the noise input path handle.
 *                  enable      : enable : bit0 - digital microphone 3
 *                              : enable : bit1 - digital microphone 4
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_digi_mic_disable(CHAL_HANDLE handle, uint16_t path)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;
    uint32_t mask;

    switch (path)
    {
    case CHAL_AUDIO_DISABLE: mask = AUDIOH_ADC_CTL_DMIC3_EN_MASK | AUDIOH_ADC_CTL_DMIC4_EN_MASK; break;
    case CHAL_AUDIO_CHANNEL_RIGHT: mask = AUDIOH_ADC_CTL_DMIC3_EN_MASK; break;
    case CHAL_AUDIO_CHANNEL_LEFT: mask = AUDIOH_ADC_CTL_DMIC4_EN_MASK; break;
    default: return;
    }

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(mask);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);

    return;

}


/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_int_enable(CHAL_HANDLE handle,
 *                             uint16_t thr_int_enable, uint16_t err_int_enable )
 *
 *  Description:  Enable interrupt on voice out path
 *
 *  Parameters:   handle      : the voice input pathhandle.
 *                  enable      : enable flag
 *
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_int_enable(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INTC);

    if(thr_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_NVINR_INTEN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_NVINR_INTEN_MASK;
    }

    if(err_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_NVIN_FIFOR_ERRINT_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_NVIN_FIFOR_ERRINT_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INTC, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_left_int_enable(CHAL_HANDLE handle,
 *                             uint16_t thr_int_enable, uint16_t err_int_enable )
 *
 *  Description:  Enable interrupt on voice out path
 *
 *  Parameters:   handle      : the voice input pathhandle.
 *                  enable      : enable flag
 *
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_left_int_enable(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INTC);

    if(thr_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_NVINL_INTEN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_NVINL_INTEN_MASK;
    }

    if(err_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_NVIN_FIFOL_ERRINT_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_NVIN_FIFOL_ERRINT_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INTC, reg_val);

    return;
}


/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_dma_enable(CHAL_HANDLE handle,
 *                             _Bool dma_enable)
 *
 *  Description:  Enable DAM on voice out path
 *
 *  Parameters:   handle      : the voice input path handle.
 *                  enable      : enable flag
 *
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_dma_enable(CHAL_HANDLE handle, _Bool dma_enable )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_DMA_CTL);

    if(dma_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DMA_CTL_NVINR_DMA_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DMA_CTL_NVINR_DMA_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DMA_CTL, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_left_dma_enable(CHAL_HANDLE handle,
 *                             _Bool dma_enable)
 *
 *  Description:  Enable DAM on voice out path
 *
 *  Parameters:   handle      : the voice input path handle.
 *                  enable      : enable flag
 *
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_left_dma_enable(CHAL_HANDLE handle, _Bool dma_enable )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_DMA_CTL);

    if(dma_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DMA_CTL_NVINL_DMA_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DMA_CTL_NVINL_DMA_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DMA_CTL, reg_val);

    return;
}


/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_get_dma_port_addr(CHAL_HANDLE handle,
 * 									uint32_t *dma_addr)
 *
 *  Description:  Get the DMA port address on noise vocie in path
 *
 *  Parameters:   handle     - audio chal handle.
 * 				 *dma_addr  - the buffer to save dma port address.
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_get_dma_port_addr( CHAL_HANDLE handle,
										   uint32_t *dma_addr )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

	*dma_addr = (uint32_t)(base) + AUDIOH_NVIN_FIFOR_DATA0_OFFSET;

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_left_get_dma_port_addr(CHAL_HANDLE handle,
 * 									uint32_t *dma_addr)
 *
 *  Description:  Get the DMA port address on noise vocie in path
 *
 *  Parameters:   handle     - audio chal handle.
 * 				 *dma_addr  - the buffer to save dma port address.
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_left_get_dma_port_addr( CHAL_HANDLE handle,
										   uint32_t *dma_addr )
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

	*dma_addr = (uint32_t)(base) + AUDIOH_NVIN_FIFOL_DATA0_OFFSET;

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_get_dma_port_addr_offset()
 *
 *  Description:  Get the DMA address offset on NVIN path
 *
 *  Return:       Offset for the DMA address.
 *
 * ============================================================================
 */
uint32_t chal_audio_nvinpath_get_dma_port_addr_offset( void )
{
   return AUDIOH_NVIN_FIFOR_DATA0_OFFSET;
}

/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_left_get_dma_port_addr_offset() (LEFT)
 *
 *  Description:  Get the DMA address offset on NVIN path
 *
 *  Return:       Offset for the DMA address.
 *
 * ============================================================================
 */
uint32_t chal_audio_nvinpath_left_get_dma_port_addr_offset( void )
{
   return AUDIOH_NVIN_FIFOL_DATA0_OFFSET;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_bits_per_sample(CHAL_HANDLE handle,
 *                         uint16_t bits)
 *
 *  Description:  Set the sample bits on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *                  bits - sample bits value.
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_set_bits_per_sample(CHAL_HANDLE handle, uint16_t bits)
{
   uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);

    if(bits >= 24)
    {
        reg_val |= AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_24BIT_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_24BIT_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}
/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_bits_per_sample(CHAL_HANDLE handle,
 *         _Bool mode)
 *
 *  Description:  Set the mode on voice input path
 *
 *  Parameters:   handle  the earpiece path handle.
 *                  mode - true : stereo, false : mono.
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_set_mono_stereo (CHAL_HANDLE handle,_Bool mode)
{
   uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);

    if(mode == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_MONO_MASK;
    }
    else        /* stereo*/
    {
        reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_MONO_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}
/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_pack(CHAL_HANDLE handle, _Bool pack)
 *
 *  Description:  Set the sample pack/upack mode on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *                  pack - true : pack, false : unpack.
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_set_pack(CHAL_HANDLE handle, _Bool pack)
{
   uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);

    if(pack == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_PACK_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_PACK_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_select_sidetone (CHAL_HANDLE handle,
 *                     _Bool read_sidetone)
 *
 *  Description:  Select sidetone or not
 *
 *  Parameters:   handle  the voice input path handle.
 *                  read_sidetone - selcetd flag for left and right channels
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_select_sidetone (CHAL_HANDLE handle, uint16_t read_sidetone)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    /*Read the current contents*/
    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);

    /* Clear the paths that were enabled before*/
    reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_SIDETONE_SEL_L_MASK;
    reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_SIDETONE_SEL_R_MASK;

    if(read_sidetone&CHAL_AUDIO_CHANNEL_LEFT)
    {
        /* Enable Left Channel to read side tone data*/
        reg_val |= AUDIOH_NVIN_FIFO_CTRL_SIDETONE_SEL_L_MASK;
    }

    if(read_sidetone&CHAL_AUDIO_CHANNEL_RIGHT)
    {
        /* Enable Right Channel to read side tone data*/
        reg_val |= AUDIOH_NVIN_FIFO_CTRL_SIDETONE_SEL_R_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_clr_fifo(CHAL_HANDLE handle)
 *
 *  Description:  Clear the FIFO on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_clr_fifo(CHAL_HANDLE handle)
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);
    reg_val |= AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    reg_val &= ~AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}
/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_fifo_thres(CHAL_HANDLE handle,
 *                         uint16_t thres, uint16_t thres_2)
 *
 *  Description:  Set the threshold for FIFO on voice input path
 *
 *  Parameters:   handle  the vocie input path handle.
 *
 *                  thres   - threshold 1
 *                  thres_2 - threshold 2
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_set_fifo_thres(CHAL_HANDLE handle, uint16_t thres, uint16_t thres_2 )
{
    uint32_t     reg_val;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_NVIN_FIFO_CTRL);
    reg_val &= ~(AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_THRES_MASK);
    reg_val &= ~(AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_THRES2_MASK);
    reg_val |= thres;
    reg_val |= thres_2 << AUDIOH_NVIN_FIFO_CTRL_NVIN_FIFO_THRES2_SHIFT;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FIFO_CTRL, reg_val);

    return;
}
/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_read_fifo_status(CHAL_HANDLE handle)
 *
 *  Description:  Read the FIFO status on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *
 *  Return:       the status.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_read_fifo_status(CHAL_HANDLE handle)
{
    uint32_t     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     status = 0;

    /*Read NVIN FIFO status*/
    status = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOR_STATUS);

    /*Add NVIN FIFO interrupt status*/
    status |= chal_audio_nvinpath_read_int_status(handle);

    return    status;
}

/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_left_read_fifo_status(CHAL_HANDLE handle)
 *
 *  Description:  Read the FIFO status on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *
 *  Return:       the status.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_left_read_fifo_status(CHAL_HANDLE handle)
{
    uint32_t     base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     status = 0;

    /*Read NVIN FIFO status*/
    status = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOL_STATUS);

    /*Add NVIN FIFO interrupt status*/
    status |= chal_audio_nvinpath_left_read_int_status(handle);

    return    status;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_read_fifo(CHAL_HANDLE handle,
 *                         uint32_t *src, uint32_t length)
 *
 *  Description:  Read the data from FIFO on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *
 *                  *src    - the data buffer address
 *                  length  - the data buffer length
 *                  ign_udf  - keep reading the fifo even on uderflow
 *
 *  Return:       None.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_read_fifo(CHAL_HANDLE handle, uint32_t *src, uint32_t length, _Bool ign_udf)
{
    uint32_t n;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    if(ign_udf == CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_nvinpath_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY)
            {
                break;
            }
            *src++ = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOR_DATA0);
        }
    }
    else
    {
        for(n = 0; n < length; n++)
        {
            *src++ = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOR_DATA0);
        }
    }
    return    n;

}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_left_read_fifo(CHAL_HANDLE handle,
 *                         uint32_t *src, uint32_t length)
 *
 *  Description:  Read the data from FIFO on voice input path
 *
 *  Parameters:   handle  the voice input path handle.
 *
 *                  *src    - the data buffer address
 *                  length  - the data buffer length
 *                  ign_udf  - keep reading the fifo even on uderflow
 *
 *  Return:       None.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_left_read_fifo(CHAL_HANDLE handle, uint32_t *src, uint32_t length, _Bool ign_udf)
{
    uint32_t n;
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    if(ign_udf == CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_nvinpath_left_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY)
            {
                break;
            }
            *src++ = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOL_DATA0);
        }
    }
    else
    {
        for(n = 0; n < length; n++)
        {
            *src++ = BRCM_READ_REG(base, AUDIOH_NVIN_FIFOL_DATA0);
        }
    }
    return    n;

}

/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_read_int_status(CHAL_HANDLE handle)
 *
 *  Description:  Read the interrupt status for the Voice In path
 *
 *  Parameters:   handle      - audio chal handle.
 *
 *  Return:       None.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_read_int_status(CHAL_HANDLE handle)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     status = 0;
    uint32_t     reg_val = 0;

    /*Read FIFO interrupt status of all paths*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    /*Check for NVIN path FIFO error interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_NVIN_FIFOR_ERR_MASK)
    {
       status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    /*Check for NVIN path FIFO threshold interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_NVINR_INT_MASK)
    {

       status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

/*
 * ============================================================================
 *
 *  Function Name: uint32_t chal_audio_nvinpath_left_read_int_status(CHAL_HANDLE handle)
 *
 *  Description:  Read the interrupt status for the Voice In path
 *
 *  Parameters:   handle      - audio chal handle.
 *
 *  Return:       None.
 *
 * ============================================================================
 */

uint32_t chal_audio_nvinpath_left_read_int_status(CHAL_HANDLE handle)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t     status = 0;
    uint32_t     reg_val = 0;

    /*Read FIFO interrupt status of all paths*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    /*Check for NVIN path FIFO error interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_NVIN_FIFOL_ERR_MASK)
    {
       status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    /*Check for NVIN path FIFO threshold interrupt*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_NVINL_INT_MASK)
    {

       status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_int_clear(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
 *
 *  Description:  Clear interrupt on Noise Microphone path
 *
 *  Parameters:   handle      : the Noise Microphone input pathhandle.
 *                       thr_int      : Clear FIFO threshold interrupt
 *                       err_int      : Clear FIFO Error interrupt
 *
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_int_clear(CHAL_HANDLE handle, _Bool thr_int, _Bool err_int)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val = 0;

    if(thr_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_NVINR_INT_MASK;
    }

    if(err_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_NVIN_FIFOR_ERR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_left_int_clear(CHAL_HANDLE handle, uint16_t thr_int_enable, uint16_t err_int_enable )
 *
 *  Description:  Clear interrupt on Noise Microphone path
 *
 *  Parameters:   handle      : the Noise Microphone input pathhandle.
 *                       thr_int      : Clear FIFO threshold interrupt
 *                       err_int      : Clear FIFO Error interrupt
 *
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_left_int_clear(CHAL_HANDLE handle, _Bool thr_int, _Bool err_int)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val = 0;

    if(thr_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_NVINL_INT_MASK;
    }

    if(err_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_NVIN_FIFOL_ERR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_cic_scale(CHAL_HANDLE handle,uint32_t dmic3_scale, uint32_t dmic4_scale)
 *
 *  Description:  Set the CIC fine scale for the Digital MIC 3 & 4
 *
 *  Parameters:   handle  the noise voice input path handle.
 *
 *  Return:       None.
 *
 * ============================================================================
 */

void chal_audio_nvinpath_set_cic_scale(CHAL_HANDLE handle, uint32_t dmic3_scale, uint32_t dmic4_scale)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    dmic3_scale &= (AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_BIT_SEL_MASK|AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_FINE_SCL_MASK);
    dmic4_scale <<= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_FINE_SCL_SHIFT);
    dmic4_scale &= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_BIT_SEL_MASK|AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_FINE_SCL_MASK);

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_NVIN_FILTER_CTRL, (dmic4_scale|dmic3_scale));

    return;
}

/*
 * ============================================================================
 *
 *  Function Name: chal_audio_nvinpath_set_digimic_clkdelay(CHAL_HANDLE handle, uint16_t enable)
 *
 *  Description:  Set the delay for sampling the DIGITAL MIC3,4 signals on the DATA line
 *
 *  Parameters:   handle      : the voice input path handle.
 *                       delay        : delay in 5.95 usec max possible is 41.6 usec
 *
 *  Return:       None.
 *
 * ============================================================================
 */
void chal_audio_nvinpath_set_digimic_clkdelay(CHAL_HANDLE handle, uint16_t delay)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);
    reg_val &= ~(AUDIOH_ADC_CTL_DMIC_CLK_DELAY2_MASK);

    delay <<= AUDIOH_ADC_CTL_DMIC_CLK_DELAY2_SHIFT;

    if(delay > AUDIOH_ADC_CTL_DMIC_CLK_DELAY2_MASK)
    {
        delay = AUDIOH_ADC_CTL_DMIC_CLK_DELAY2_MASK;
    }

   reg_val |= delay;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_ADC_CTL, reg_val);

    return;

}


/*
 * ============================================================================
 *
 *  Function Name: void chal_audio_nvinpath_set_filter(CHAL_HANDLE handle, uint16_t filter)
 *
 *  Description:  set Filter type for the nvin path
 *
 *  Parameters:   handle    - audio chal handle.
 *                  filter    -  0: Linear Phase, 1 Minimum Phase
 *  Return:       None.
 *
 */

/*============================================================================*/
void chal_audio_nvinpath_set_filter(CHAL_HANDLE handle, uint16_t filter)
{
    uint32_t base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    uint32_t reg_val;


    reg_val = BRCM_READ_REG(base, AUDIOH_MIN_PHASE);
    reg_val &= ~(AUDIOH_MIN_PHASE_DMIC3_MIN_PHASE_MASK|AUDIOH_MIN_PHASE_DMIC4_MIN_PHASE_MASK);

    if(filter & CHAL_AUDIO_MINIMUM_PHASE_FILTER)
    {
        reg_val |= AUDIOH_MIN_PHASE_DMIC3_MIN_PHASE_MASK;
    }

    if(filter & CHAL_AUDIO_MINIMUM_PHASE_FILTER_L)
    {
        reg_val |= AUDIOH_MIN_PHASE_DMIC4_MIN_PHASE_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_MIN_PHASE, reg_val);

    return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_nvinpath_digi_mic_enable);
EXPORT_SYMBOL(chal_audio_nvinpath_int_enable);
EXPORT_SYMBOL(chal_audio_nvinpath_left_int_enable);
EXPORT_SYMBOL(chal_audio_nvinpath_dma_enable);
EXPORT_SYMBOL(chal_audio_nvinpath_left_dma_enable);
EXPORT_SYMBOL(chal_audio_nvinpath_get_dma_port_addr);
EXPORT_SYMBOL(chal_audio_nvinpath_left_get_dma_port_addr);
EXPORT_SYMBOL(chal_audio_nvinpath_get_dma_port_addr_offset);
EXPORT_SYMBOL(chal_audio_nvinpath_left_get_dma_port_addr_offset);
EXPORT_SYMBOL(chal_audio_nvinpath_set_bits_per_sample);
EXPORT_SYMBOL(chal_audio_nvinpath_set_mono_stereo);
EXPORT_SYMBOL(chal_audio_nvinpath_set_pack);
EXPORT_SYMBOL(chal_audio_nvinpath_select_sidetone);
EXPORT_SYMBOL(chal_audio_nvinpath_set_fifo_thres);
EXPORT_SYMBOL(chal_audio_nvinpath_read_fifo_status);
EXPORT_SYMBOL(chal_audio_nvinpath_left_read_fifo_status);
EXPORT_SYMBOL(chal_audio_nvinpath_read_fifo);
EXPORT_SYMBOL(chal_audio_nvinpath_left_read_fifo);
EXPORT_SYMBOL(chal_audio_nvinpath_clr_fifo);
EXPORT_SYMBOL(chal_audio_nvinpath_int_clear);
EXPORT_SYMBOL(chal_audio_nvinpath_left_int_clear);
EXPORT_SYMBOL(chal_audio_nvinpath_read_int_status);
EXPORT_SYMBOL(chal_audio_nvinpath_left_read_int_status);
EXPORT_SYMBOL(chal_audio_nvinpath_set_cic_scale);
EXPORT_SYMBOL(chal_audio_nvinpath_set_digimic_clkdelay);
EXPORT_SYMBOL(chal_audio_nvinpath_set_filter);
EXPORT_SYMBOL(chal_audio_nvinpath_digi_mic_disable);

#endif
