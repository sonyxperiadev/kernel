/****************************************************************************/
/*     Copyright 2009-2012  Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*  @file   chal_caph_audioh_sidetone.c
*
*  @brief  chal layer driver for caph audioh device driver
*
****************************************************************************/

#include "chal_caph_audioh.h"
#include "chal_caph_audioh_int.h"
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_sdt.h>
#include <mach/rdb/brcm_rdb_util.h>

/****************************************************************************
*                        G L O B A L   S E C T I O N
*****************************************************************************/

/****************************************************************************
* global variable definitions
*****************************************************************************/

/****************************************************************************
*                         L O C A L   S E C T I O N
*****************************************************************************/

/****************************************************************************
* local macro declarations
*****************************************************************************/

#define SIDETONE_FIT_TAP_VALUE 0x3F

/****************************************************************************
* local typedef declarations
*****************************************************************************/

/****************************************************************************
* local variable definitions
*****************************************************************************/

/****************************************************************************
* local function declarations
*****************************************************************************/

/******************************************************************************
* local function definitions
*******************************************************************************/

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_enable(CHAL_HANDLE handle,
* cUInt16 enable)
*
* Description:  Enable or Disable Sidetone path
*
* Parameters:   handle    - audio chal handle.
*                 enable    -  true : enable, false : disable.
*
* Return:       None.
*
*============================================================================*/
cVoid chal_audio_stpath_enable(CHAL_HANDLE handle, cUInt16 enable)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->audioh_base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, AUDIOH_ADC_CTL);

	if (enable == CHAL_AUDIO_ENABLE)
		reg_val |= AUDIOH_ADC_CTL_SIDETONE_EN_MASK;
	else
		reg_val &= ~AUDIOH_ADC_CTL_SIDETONE_EN_MASK;


	BRCM_WRITE_REG(base, AUDIOH_ADC_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_load_filter(CHAL_HANDLE handle,
*                        cUInt16 *src, cUInt32 length )
*
* Description:  Load coefficient to sidetone path
*
* Parameters:   handle - audio chal handle.
*                 *src   - the coefficient buffer
*                 length - the coefficient length
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_load_filter(CHAL_HANDLE handle, cUInt32 *coeff,
				    cUInt32 length)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 index;
	cUInt32 reg_val;

	/* Disable loading the filter while updating the filter coefficients*/
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val |= SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK;
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	/*Set the lower tap */
	chal_audio_stpath_set_filter_lowertaps(handle, SIDETONE_FIT_TAP_VALUE);

	/*Set the index to start at 0 */
	BRCM_WRITE_REG(base, SDT_SDT_COEF_ADDR, 0);

	/* Write the coefficients */
	for (index = 0; index < 128; index++)
		BRCM_WRITE_REG(base, SDT_SDT_COEF_DATA, coeff[index]);

	/*Set the upper tap */
	chal_audio_stpath_set_filter_uppertaps(handle, SIDETONE_FIT_TAP_VALUE);

	/*Enable loading the filter while updating the filter coefficients */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val &= (~SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_gain(CHAL_HANDLE handle,
* cUInt32 gain)
*
* Description:  Set the gain on Sidetone path
*
* Parameters:   handle - audio chal handle.
*                      gain   - gain valaue.
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_gain(CHAL_HANDLE handle, cUInt32 gain)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val = 0;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);

	/* Disable gain loading */
	reg_val &= (~SDT_SDT_CTL_TARGET_GAIN_LOAD_MASK);
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	/* Set the Gain */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val &= (~SDT_SDT_CTL_TARGET_GAIN_MASK);
	reg_val |=
	    ((gain << SDT_SDT_CTL_TARGET_GAIN_SHIFT) &
	     SDT_SDT_CTL_TARGET_GAIN_MASK);
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	/* Enable gain loading */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val |= (SDT_SDT_CTL_TARGET_GAIN_LOAD_MASK);
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle,
* cUInt16 enable, cUInt16 linear, cUInt16 slope)
*
* Description:  Set the soft slope gain parameters
*
* Parameters:  handle - audio chal handle.
*		sof_slope   - Soft Slope (ENABLE)
*		linear      - Linear increment (ENABLE),
*		Logerithemic Increment (DISABLE)
*		slope       - Step size per sample
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle, cUInt16 sof_slope,
				     cUInt16 linear, cUInt32 slope)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val = 0;

	/* Update the soft slope */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
	reg_val &= (~SDT_SDT_CTRL_2_SOF_SLOPE_MASK);
	reg_val |=
	    ((slope << SDT_SDT_CTRL_2_SOF_SLOPE_SHIFT) &
	     SDT_SDT_CTRL_2_SOF_SLOPE_MASK);
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

	/* Update the linear and soft slope */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val &= (~SDT_SDT_CTL_SOF_SLOPE_LIN_MASK);
	reg_val &= (~SDT_SDT_CTL_SOF_SLOPE_ENABLE_MASK);
	if (linear == CHAL_AUDIO_ENABLE)
		reg_val |= (SDT_SDT_CTL_SOF_SLOPE_LIN_MASK);

	if (sof_slope == CHAL_AUDIO_ENABLE)
		reg_val |= (SDT_SDT_CTL_SOF_SLOPE_ENABLE_MASK);

	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_sofslope(CHAL_HANDLE handle,
* cUInt16 clipping, cUInt16 dis_filter, cUInt16 gain_bypass)
*
* Description:  Set the soft slope gain parameters
*
* Parameters:   handle - audio chal handle.
*                      gain   - gain valaue.
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_config_misc(CHAL_HANDLE handle, cUInt16 clipping,
				    cUInt16 dis_filter, cUInt16 gain_bypass)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val = 0;

	/* Update the soft slope */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
	reg_val &= (~SDT_SDT_CTRL_2_CLIP_DISABLE_MASK);
	if (clipping == CHAL_AUDIO_ENABLE)
		reg_val |= (SDT_SDT_CTRL_2_CLIP_DISABLE_MASK);

	BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

	/* Update the linear and soft slope */
	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val &= (~SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);
	reg_val &= (~SDT_SDT_CTL_BYPASS_DIG_GAIN_MASK);

	if (dis_filter == CHAL_AUDIO_ENABLE)
		reg_val |= (SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK);


	if (gain_bypass == CHAL_AUDIO_ENABLE)
		reg_val |= (SDT_SDT_CTL_BYPASS_DIG_GAIN_MASK);

	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_filter_uppertaps
* (CHAL_HANDLE handle, cUInt16 taps)
*
* Description:  Set upper tap value for sidetone.
*
* Parameters:   handle - audio chal handle.
*               taps   - tap valaue.
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_filter_uppertaps(CHAL_HANDLE handle, cUInt16 taps)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val = 0;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_2);
	reg_val &= (~SDT_SDT_CTRL_2_UPPER_FIT_TAP_MASK);
	reg_val |= (taps << SDT_SDT_CTRL_2_UPPER_FIT_TAP_SHIFT);
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_2, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_filter_lowertaps
* (CHAL_HANDLE handle, cUInt16 taps)
*
* Description:  Set lower tap value for sidetone.
*
* Parameters:   handle - audio chal handle.
*               taps   - tap valaue.
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_filter_lowertaps(CHAL_HANDLE handle, cUInt16 taps)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val = 0;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTL);
	reg_val &= (~SDT_SDT_CTL_LOWER_FIT_TAP_MASK);
	reg_val |= (taps << SDT_SDT_CTL_LOWER_FIT_TAP_SHIFT);
	BRCM_WRITE_REG(base, SDT_SDT_CTL, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_int_enable(CHAL_HANDLE handle,
*                            cUInt16 thr_int_enable, cUInt16 err_int_enable )
*
* Description:  Enable interrupt on voice out path
*
* Parameters:   handle      : the sidetone feedback  pathhandle.
*                 enable      : enable flag
*
* Return:       None.
*
*============================================================================*/
cVoid chal_audio_stpath_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable,
				   cUInt16 err_int_enable)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 reg_val;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_3);

	if (thr_int_enable == CHAL_AUDIO_ENABLE)
		reg_val |= SDT_SDT_CTRL_3_INTR_EN_MASK;
	else
		reg_val &= ~SDT_SDT_CTRL_3_INTR_EN_MASK;


	if (err_int_enable == CHAL_AUDIO_ENABLE)
		reg_val |= SDT_SDT_CTRL_3_ERR_INTR_EN_MASK;
	else
		reg_val &= ~SDT_SDT_CTRL_3_ERR_INTR_EN_MASK;


	/* Set the required setting */
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_3, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_dma_enable(CHAL_HANDLE handle,
*                            Boolean dma_enable)
*
* Description:  Enable DAM on voice out path
*
* Parameters:   handle      : the voice input path handle.
*                 enable      : enable flag
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable)
{
	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_get_dma_port_addr(CHAL_HANDLE handle,
* UInt32 *dma_addr)
*
* Description:  Get the DMA port address on vocie in path
*
* Parameters:   handle     - audio chal handle.
*                    *dma_addr  - the buffer to save dma port address.
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;

	*dma_addr = (UInt32) (base) + SDT_SDT_OUTPUTFIFO_OFFSET;

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_bits_per_sample(CHAL_HANDLE handle,
*        Boolean mode)
*
* Description:  Set the mode on sidetone feedback  path
*
* Parameters:   handle  the earpiece path handle.
*                 mode - true : stereo, false : mono.
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_bits_per_sample(CHAL_HANDLE handle, UInt16 bits)
{
	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_bits_per_sample
* (CHAL_HANDLE handle,
*        Boolean mode)
*
* Description:  Set the mode on sidetone feedback path
*
* Parameters:   handle  the earpiece path handle.
*                 mode - true : stereo, false : mono.
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode)
{
	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_pack(CHAL_HANDLE handle,
* Boolean pack)
*
* Description:  Set the sample pack/upack mode on sidetone feedback  path
*
* Parameters:   handle  the sidetone feedback  path handle.
*                 pack - true : pack, false : unpack.
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_pack(CHAL_HANDLE handle, Boolean pack)
{

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_clr_fifo(CHAL_HANDLE handle)
*
* Description:  Clear the FIFO on sidetone feedback  path
*
* Parameters:   handle  the sidetone feedback  path handle.
*
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_clr_fifo(CHAL_HANDLE handle)
{
	cUInt32 reg_val;
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_3);
	reg_val |= SDT_SDT_CTRL_3_FIFO_RESET_MASK;
	/* Set the required setting */
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_3, reg_val);

	reg_val &= ~SDT_SDT_CTRL_3_FIFO_RESET_MASK;
	/* Set the required setting */
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_3, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_set_fifo_thres(CHAL_HANDLE handle,
*                        cUInt16 thres, cUInt16 thres_2)
*
* Description:  Set the threshold for FIFO on sidetone feedback  path
*
* Parameters:   handle  the vocie input path handle.
*
*                 thres   - threshold 1
*                 thres_2 - threshold 2
* Return:       None.
*
*============================================================================*/

cVoid chal_audio_stpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres,
				       cUInt16 thres_2)
{
	cUInt32 reg_val;
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;

	reg_val = BRCM_READ_REG(base, SDT_SDT_CTRL_3);
	reg_val &= ~(SDT_SDT_CTRL_3_THRES_MASK);
	reg_val &= ~(SDT_SDT_CTRL_3_THRES2_MASK);
	reg_val |= thres << SDT_SDT_CTRL_3_THRES_SHIFT;
	reg_val |= thres_2 << SDT_SDT_CTRL_3_THRES2_SHIFT;

	/* Set the required setting */
	BRCM_WRITE_REG(base, SDT_SDT_CTRL_3, reg_val);

	return;
}

/*============================================================================
*
* Function Name: cUInt32 chal_audio_stpath_read_fifo_status(CHAL_HANDLE handle)
*
* Description:  Read the FIFO status (including interrupts) on sidetone path
*
* Parameters:   handle  the sidetone feedback  path handle.
*
* Return:       the status.
*
*============================================================================*/

cUInt32 chal_audio_stpath_read_fifo_status(CHAL_HANDLE handle)
{
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;
	cUInt32 sdt_status = 0;
	cUInt32 status = 0;

	/* Read sidetone path FIFO status */
	sdt_status = BRCM_READ_REG(base, SDT_SDT_CTRL_3);

	/* Add sidetone path FIFO interrupt status */
	if (sdt_status & SDT_SDT_CTRL_3_OVF_MASK)
		status |= CHAL_AUDIO_FIFO_STATUS_OVF;

	if (sdt_status & SDT_SDT_CTRL_3_UDF_MASK)
		status |= CHAL_AUDIO_FIFO_STATUS_UDF;

	if (sdt_status & SDT_SDT_CTRL_3_ALMOST_EMPTY_MASK)
		status |= CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY;

	if (sdt_status & SDT_SDT_CTRL_3_ALMOST_FULL_MASK)
		status |= CHAL_AUDIO_FIFO_STATUS_NEAR_FULL;

	/* Update the empty entry */
	status |=
	    (sdt_status & SDT_SDT_CTRL_3_EMPTY_ENTRY_MASK) >>
	    SDT_SDT_CTRL_3_EMPTY_ENTRY_SHIFT;

	status |= chal_audio_stpath_read_int_status(handle);

	return status;
}

/*============================================================================
*
* Function Name: cUInt32 chal_audio_stpath_read_fifo(CHAL_HANDLE handle,
*                        cUInt32 *src, cUInt32 length)
*
* Description:  Read the data from FIFO on sidetone feedback  path
*
* Parameters:   handle  the sidetone feedback  path handle.
*
*                 *src    - the data buffer address
*                 length  - the data buffer length
*                 ign_udf  - keep reading the fifo even on uderflow
*
* Return:       None.
*
*============================================================================*/

cUInt32 chal_audio_stpath_read_fifo(CHAL_HANDLE handle, cUInt32 *src,
				    cUInt32 length, Boolean ign_udf)
{
	UInt32 n;
	cUInt32 base = ((ChalAudioCtrlBlk_t *) handle)->sdt_base;

	if (ign_udf == CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW) {
		for (n = 0; n < length; n++) {
			/* Check if the FIFO is getting underflow or not */
			/* if ign_udf flag is not set */
			if (chal_audio_stpath_read_fifo_status(handle) &
			    CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY) {
				break;
			}
			*src++ = BRCM_READ_REG(base, SDT_SDT_OUTPUTFIFO);
		}
	} else {
		for (n = 0; n < length; n++)
			*src++ = BRCM_READ_REG(base, SDT_SDT_OUTPUTFIFO);
	}
	return n;

}

/*============================================================================
*
* Function Name: cUInt32 chal_audio_stpath_read_int_status(CHAL_HANDLE handle)
*
* Description:  Read the interrupt status for the Voice In path
*
* Parameters:   handle      - audio chal handle.
*
* Return:       None.
*
*============================================================================*/

cUInt32 chal_audio_stpath_read_int_status(CHAL_HANDLE handle)
{
	cUInt32 status = 0;

	return status;
}

/*============================================================================
*
* Function Name: cVoid chal_audio_stpath_int_clear(CHAL_HANDLE handle,
* cUInt16 thr_int_enable, cUInt16 err_int_enable )
*
* Description:  Clear interrupt on voice in path
*
* Parameters:   handle      : the sidetone feedback  pathhandle.
*                      thr_int      : Clear FIFO threshold interrupt
*                      err_int      : Clear FIFO Error interrupt
*
* Return:       None.
*
*============================================================================*/
cVoid chal_audio_stpath_int_clear(CHAL_HANDLE handle, Boolean thr_int,
				  Boolean err_int)
{
	return;
}
