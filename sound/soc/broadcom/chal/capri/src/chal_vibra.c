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
*  @file   chal_vibra.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include "chal_caph_audioh.h"
#include "chal_audio_int.h"
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
 *  Function Name: void chal_audio_vibra_enable(CHAL_HANDLE handle, cUInt16 enable)
 * 
 *  Description:  Enable or Disable vibra path
 * 
 *  Parameters:   handle    - audio chal handle.
 *                  enable    -  enable (true : enable, false : disable)
 * 
 *  Return:       None.
 * 
 */

/*============================================================================*/
void chal_audio_vibra_enable(CHAL_HANDLE handle, cUInt16 enable)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;
    cUInt32 Channl_Ctrl = 0x00000000;


    if(enable == CHAL_AUDIO_ENABLE)
    {
        Channl_Ctrl |= AUDIOH_DAC_CTL_VIBRA_ENABLE_MASK;
    }
    else
    {
        Channl_Ctrl &= ~AUDIOH_DAC_CTL_VIBRA_ENABLE_MASK;
    }

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);
    reg_val &= ~(AUDIOH_DAC_CTL_VIBRA_ENABLE_MASK);
    reg_val |= Channl_Ctrl;


    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DAC_CTL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable )
 * 
 *  Description:  Enable or Disable vibra path interrupt
 * 
 *  Parameters:   handle      - audio chal handle.
 *                  int_enable  - true : enable interrupt, false : disable interrupt.
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_vibra_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable  )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INTC);

    if(thr_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_VIBRA_INTEN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_VIBRA_INTEN_MASK;
    }

    if(err_int_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_AUDIO_INTC_VIBRA_FIFO_ERRINT_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_AUDIO_INTC_VIBRA_FIFO_ERRINT_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INTC, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_dma_enable(CHAL_HANDLE handle, cUInt16 dma_enable)
 * 
 *  Description:  Enable or Disable DMA on vibra path
 * 
 *  Parameters:   handle     - audio chal handle.
 *                  dma_enable - true: enable DMA, false : disable DMA.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_dma_enable(CHAL_HANDLE handle, _Bool dma_enable )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;

    reg_val = BRCM_READ_REG(base, AUDIOH_DMA_CTL);

    if(dma_enable == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DMA_CTL_VIBRA_DMA_EN_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DMA_CTL_VIBRA_DMA_EN_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DMA_CTL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_get_dma_port_addr(CHAL_HANDLE handle,
 * 									cUInt32 *dma_addr)
 * 
 *  Description:  Get the DMA port address on vibra path
 * 
 *  Parameters:   handle     - audio chal handle.
 * 				 *dma_addr  - the buffer to save dma port address.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_get_dma_port_addr( CHAL_HANDLE handle,
										   cUInt32 *dma_addr )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

	*dma_addr = (cUInt32)(base) + AUDIOH_VIBRA_FIFO_DATA_OFFSET;

	return;
}

/*
 * ============================================================================
 * 
 *  Function Name: cUInt32 chal_audio_vibra_get_dma_port_addr_offset()
 * 
 *  Description:  Get the DMA address offset on IHF path
 * 
 *  Return:       Offset for the DMA address.
 * 
 * ============================================================================
 */
cUInt32 chal_audio_vibra_get_dma_port_addr_offset( void )
{
   return AUDIOH_VIBRA_FIFO_DATA_OFFSET;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_mute(CHAL_HANDLE handle, _Bool mute)
 * 
 *  Description:  Mute or unmute on vibra path
 * 
 *  Parameters:   handle     - audio chal handle.
 *                  mute        - true: mute opeation, false : unmute opeartion.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_mute(CHAL_HANDLE handle,  _Bool mute)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_DAC_CTL);

    if(mute == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_DAC_CTL_VIBRA_MUTE_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_DAC_CTL_VIBRA_MUTE_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_DAC_CTL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_gain(CHAL_HANDLE handle, cUInt32 gain)
 * 
 *  Description:  Set the gain on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  gain   - gain valaue.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_gain(CHAL_HANDLE handle, cUInt32 gain)
{
   (void) handle;
   (void) gain;
   return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
 * 
 *  Description:  Set the sample bits on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  bits   - sample bits value.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_CTRL);

    if(bits >= 24)
    {
        reg_val |= AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_24BIT_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_24BIT_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_bypass(CHAL_HANDLE handle, _Bool bypass)
 * 
 *  Description:  Set the gain on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  bypass - bypass selection
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_bypass(CHAL_HANDLE handle, _Bool bypass)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_CTRL);

    if(bypass == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_BYPASS_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_BYPASS_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_mono_stereo(CHAL_HANDLE handle, _Bool mode)
 * 
 *  Description:  Set the playback mode on vibrator path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  mode   - true : stereo, false : mono.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_mono_stereo(CHAL_HANDLE handle, _Bool mode)
{
   (void) handle;
   (void) mode;

   return;
}
/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_pack(CHAL_HANDLE handle, _Bool pack)
 * 
 *  Description:  Set the sample pack/upack mode on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 *                  pack   - true : pack, false : unpack.
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_pack(CHAL_HANDLE handle, _Bool pack)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_CTRL);

    if(pack == CHAL_AUDIO_ENABLE)
    {
        reg_val |= AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_PACK_MASK;
    }
    else
    {
        reg_val &= ~AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_PACK_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    return;
}
/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_clr_int(CHAL_HANDLE handle)
 * 
 *  Description:  Read the interrupt status
 * 
 *  Parameters:   handle      - audio chal handle.
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_clr_int(CHAL_HANDLE handle)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = AUDIOH_AUDIO_INT_STATUS_VIBRA_INT_MASK| AUDIOH_AUDIO_INT_STATUS_VIBRA_FIFO_ERR_MASK;
    BRCM_WRITE_REG(base, AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_clr_fifo(CHAL_HANDLE handle)
 * 
 *  Description:  Clhs the FIFO on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_clr_fifo(CHAL_HANDLE handle)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_CTRL);
    reg_val |= AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    reg_val &= ~AUDIOH_VIBRA_FIFO_CTRL_VIBRA_FIFO_CLEAR_MASK;
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
 * 
 *  Description:  Set   the threshold for FIFO on vibra path
 * 
 *  Parameters:   handle  - audio chal handle
 *                  thres   - threshold 1
 *                  thres_2 - threshold 2
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2)
{
    cUInt32     reg_val;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    reg_val = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_CTRL);
    reg_val &= ~(0x7F);
    reg_val &= ~(0xF0);
    reg_val |= thres;
    reg_val |= thres_2 << 8;

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_CTRL, reg_val);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: cUInt32 chal_audio_vibra_read_fifo_status(CHAL_HANDLE handle)
 * 
 *  Description:  Read the FIFO status (including interrupt status) on vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 * 
 *  Return:       the status.
 * 
 * ============================================================================
 */

cUInt32 chal_audio_vibra_read_fifo_status(CHAL_HANDLE handle)
{
    cUInt32     status;
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    /*Get the Vibra path FIFO status*/
    status = BRCM_READ_REG(base, AUDIOH_VIBRA_FIFO_STATUS);

    /*Add vibra path FIFO interrupt status*/
    status |= chal_audio_vibra_read_int_status(handle);

    return    status;
}

/*
 * ============================================================================
 * 
 *  Function Name: cUInt32 chal_audio_vibra_read_int_status(CHAL_HANDLE handle)
 * 
 *  Description:  Read the interrupt status
 * 
 *  Parameters:   handle      - audio chal handle.
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */

cUInt32 chal_audio_vibra_read_int_status(CHAL_HANDLE handle)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32     status = 0;
    cUInt32     reg_val = 0;

    /*Get FIFO interrupt status of all paths*/
    reg_val = BRCM_READ_REG(base, AUDIOH_AUDIO_INT_STATUS);

    /* Check for FIFO error interrupt status of VIBRA path*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VIBRA_FIFO_ERR_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_ERR_INT;
    }

    /* Check for FIFO threshold interrupt status of VIBRA path*/
    if(reg_val & AUDIOH_AUDIO_INT_STATUS_VIBRA_INT_MASK)
    {
        status |= CHAL_AUDIO_FIFO_STATUS_THR_INT;
    }

    return    status;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_vibra_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length)
 * 
 *  Description:  Write the data to FIFO on vibra path
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

cUInt32 chal_audio_vibra_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, _Bool ign_ovf)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32      n;

    if(ign_ovf == CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW)
    {
        for(n = 0; n < length; n++)
        {
            /* Check if the FIFO is getting underflow or not, if ign_udf flag is not set */
            if(chal_audio_vibra_read_fifo_status(handle) & CHAL_AUDIO_FIFO_STATUS_NEAR_FULL)
            {
                break;
            }
            BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_DATA, ((cUInt32) *src++));
        }
    }
    else
    {
        for(n = 0; n < length; n++)
        {
            BRCM_WRITE_REG(base,  AUDIOH_VIBRA_FIFO_DATA, ((cUInt32) *src++));
        }
    }
    return    n;
}

/*
 * ============================================================================
 * 
 *  Function Name: chal_audio_vibra_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef )
 * 
 *  Description:  Set SDM coefficient on Vibra path
 * 
 *  Parameters:   handle - audio chal handle.
 */
/**                 SDM_coef - pointer to the SDM coefficients*/
/*
 *  Return:       none
 * 
 * ============================================================================
 */

void chal_audio_vibra_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef )
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;

    BRCM_WRITE_REG(base,  AUDIOH_VIBRA_SDM_CTRL, *SDM_coef);

    return;
}

/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_int_clear(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable )
 * 
 *  Description:  Clear interrupt on Vibrator path
 * 
 *  Parameters:   handle      : the Vibrator pathhandle.
 *                       thr_int      : Clear FIFO threshold interrupt
 *                       err_int      : Clear FIFO Error interrupt
 * 
 *  Return:       None.
 * 
 * ============================================================================
 */
void chal_audio_vibra_int_clear(CHAL_HANDLE handle, _Bool thr_int, _Bool err_int)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val = 0;

    if(thr_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_VIBRA_INT_MASK;
    }

    if(err_int == TRUE)
    {
        reg_val |= AUDIOH_AUDIO_INT_STATUS_VIBRA_FIFO_ERR_MASK;
    }

    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_AUDIO_INT_STATUS, reg_val);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_dac_pwr(CHAL_HANDLE handle,
 *                                     cUInt16 enable_chan)
 * 
 *  Description:  Set DAC power for the Vibra Class D DAC
 * 
 *  Parameters:   handle     - audio chal handle.
 *                enable_chan   - Specifies which DAC channel power need to be enabled or disabled
 *  Return:       None.
 * 
 * ============================================================================
 */

void chal_audio_vibra_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable_chan)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_value;

    reg_value = BRCM_READ_REG(base, AUDIOH_VIBRA_CTRL);


    if(enable_chan&CHAL_AUDIO_ENABLE)
    {
        /* Clear power down */
       reg_value &= ~AUDIOH_VIBRA_CTRL_ANA_VIBRA_PD_MASK;
    }
    else
    {
        /* Set power down */
        reg_value |= AUDIOH_VIBRA_CTRL_ANA_VIBRA_PD_MASK;
    }


    BRCM_WRITE_REG(base, AUDIOH_VIBRA_CTRL, reg_value);

    return;
}


/*
 * ============================================================================
 * 
 *  Function Name: void chal_audio_vibra_set_filter(CHAL_HANDLE handle, cUInt16 filter)
 * 
 *  Description:  set Filter type for the vibra  path
 * 
 *  Parameters:   handle    - audio chal handle.
 *                  filter    -  0: Linear Phase, 1 Minimum Phase
 *  Return:       None.
 * 
 */

/*============================================================================*/
void chal_audio_vibra_set_filter(CHAL_HANDLE handle, cUInt16 filter)
{
    cUInt32 base =    ((ChalAudioCtrlBlk_t*)handle)->audioh_base;
    cUInt32 reg_val;


    reg_val = BRCM_READ_REG(base, AUDIOH_MIN_PHASE);
    reg_val &= ~(AUDIOH_MIN_PHASE_VB_MIN_PHASE_MASK);

    if(filter & CHAL_AUDIO_MINIMUM_PHASE_FILTER)
    {
        reg_val |= AUDIOH_MIN_PHASE_VB_MIN_PHASE_MASK;
    }
    /* Set the required setting */
    BRCM_WRITE_REG(base,  AUDIOH_MIN_PHASE, reg_val);

    return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_audio_vibra_enable);
EXPORT_SYMBOL(chal_audio_vibra_int_enable);
EXPORT_SYMBOL(chal_audio_vibra_dma_enable);
EXPORT_SYMBOL(chal_audio_vibra_get_dma_port_addr);
EXPORT_SYMBOL(chal_audio_vibra_get_dma_port_addr_offset);
EXPORT_SYMBOL(chal_audio_vibra_mute);
EXPORT_SYMBOL(chal_audio_vibra_set_gain);
EXPORT_SYMBOL(chal_audio_vibra_set_bits_per_sample);
EXPORT_SYMBOL(chal_audio_vibra_set_mono_stereo);
EXPORT_SYMBOL(chal_audio_vibra_set_pack);
EXPORT_SYMBOL(chal_audio_vibra_clr_fifo);
EXPORT_SYMBOL(chal_audio_vibra_set_fifo_thres);
EXPORT_SYMBOL(chal_audio_vibra_read_fifo_status);
EXPORT_SYMBOL(chal_audio_vibra_read_int_status);
EXPORT_SYMBOL(chal_audio_vibra_write_fifo);
EXPORT_SYMBOL(chal_audio_vibra_set_bypass);
EXPORT_SYMBOL(chal_audio_vibra_int_clear);
EXPORT_SYMBOL(chal_audio_vibra_sdm_set_coef);
EXPORT_SYMBOL(chal_audio_vibra_set_dac_pwr);
EXPORT_SYMBOL(chal_audio_vibra_set_filter);
EXPORT_SYMBOL(chal_audio_vibra_clr_int);

#endif
