/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*  @file   chal_caph_dma.c
*
*  @brief  chal layer driver for caph dma block
*
****************************************************************************/

#include "chal_caph.h"
#include "chal_caph_dma.h"
#include "brcm_rdb_cph_aadmac.h"
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

static cVoid chal_caph_dma_rheaB0_set_hibuffer(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
            cUInt32             address,
            cUInt32             size);

static cVoid chal_caph_dma_rheaB0_set_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel,
			CAPH_DMA_CHNL_FIFO_STATUS_e status);

static cVoid chal_caph_dma_rheaB0_clr_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel,
			CAPH_DMA_CHNL_FIFO_STATUS_e status);

static cVoid chal_caph_dma_rheaB0_clr_channel_fifo(CHAL_HANDLE handle,
			cUInt16 channel);

static CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_rheaB0_read_ddrfifo_sw_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel);

/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_dma_platform_init(chal_caph_dma_funcs_t *pfuncs)
*
*  Description: init platform-specific CAPH DMA functions.
*
****************************************************************************/
cBool chal_caph_dma_platform_init(chal_caph_dma_funcs_t *pfuncs)
{
    pfuncs->set_ddrfifo_status = chal_caph_dma_rheaB0_set_ddrfifo_status;
    pfuncs->clr_channel_fifo = chal_caph_dma_rheaB0_clr_channel_fifo;
    pfuncs->clr_ddrfifo_status = chal_caph_dma_rheaB0_clr_ddrfifo_status;
    pfuncs->read_ddrfifo_sw_status = chal_caph_dma_rheaB0_read_ddrfifo_sw_status;
    pfuncs->set_hibuffer = chal_caph_dma_rheaB0_set_hibuffer;
    return TRUE;
}

//****************************************************************************
// local function definitions
//****************************************************************************


/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_buffer(CHAL_HANDLE handle,
*                   CAPH_DMA_CHANNEL_e  channel,
*			    cUInt32                       address,
*                   cUInt32                       size)
*
*  Description: config CAPH DMA channel buffer parameters (address and size)
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_set_hibuffer(CHAL_HANDLE handle,
            CAPH_DMA_CHANNEL_e  channel,
            cUInt32             address,
            cUInt32             size)
{
    cUInt32     base = ((chal_caph_dma_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     cr = 0;

    /* Find the channel we are looking for */
    for(index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++)
    {
        if((1UL << index)&channel)
        {
            /* Set the DMA buffer Address */
            address &= CPH_AADMAC_CH1_AADMAC_CR_3_CH1_AADMAC_HIGH_BASE_MASK;
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_CR_3, index, address);

            /* enable the use of hi buffer*/
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_CR_2, (index*CHAL_CAPH_DMA_CH_REG_SIZE));

            /* Configure the use of buffer base address register */
            cr &= ~CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_HIGH_BASE_EN_MASK;
            cr |= (1 << CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_HIGH_BASE_EN_SHIFT);

            /* Apply the settings in the hardware */
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_CR_2, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
            break;
        }

    }

    return;
}


/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_set_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel,
*			CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: set CAPH DMA ddr fifo status
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_set_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel,
			CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
    cUInt32     base = ((chal_caph_dma_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     cr = 0;


    /* Find the channel we are looking for */
    for(index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++)
    {
        if((1UL << index)&channel)
        {
            /* found the channel we are looking for, Set the DDR fifo status */
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));
            if(status == CAPH_READY_NONE)
            {
                cr &= (~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_MASK);
                cr &= (~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_HIGH_MASK);
                BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
            }
            else
            {
#if 1
                if (status & CAPH_READY_LOW)
                {
                    cr = CAPH_READY_LOW << CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
                    BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
                }
                if (status & CAPH_READY_HIGH)
                {
                    cr |= CAPH_READY_HIGH << CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
                    BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
                }
#else
                cr |= (status << CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT);
                BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
#endif
            }
            break;
        }
    }

    return;
}


/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_ddrfifo_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel,
*			CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: set CAPH DMA ddr fifo status
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_clr_ddrfifo_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel,
			CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
    cUInt32     base = ((chal_caph_dma_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     sr = 0;


    /* Find the channel we are looking for */
    for(index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++)
    {
        if((1UL << index)&channel)
        {
            /* found the channel we are looking for, Get the channel status */
            sr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));
            /* write 0 to clear the bits that had been set */
            sr &= ~((status & CAPH_READY_HIGHLOW)<<CPH_AADMAC_CH1_AADMAC_SR_1_CH1_HW_READY_LOW_SHIFT);
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), sr);
            break;
        }

    }

    return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_clr_channel_fifo(CHAL_HANDLE handle,
*			cUInt16 fifo)
*
*  Description: clear dma channel internal fifo
*
****************************************************************************/
static cVoid chal_caph_dma_rheaB0_clr_channel_fifo(CHAL_HANDLE handle,
			cUInt16 channel)
{
    cUInt32     base = ((chal_caph_dma_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     cr;


    /* Find the FIFOs we are looking for */
    for(index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++)
    {
        if((1UL << index)&channel)
        {
            /* found the Channel we are looking for, Disable the FIFO */
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));

            /* Send Reset Pulse to the Hardware. First make sure it is 0, set to 1, then clear to 0 */
            /* Clear Reset */
            cr &= ~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);

            /* Start Reset  process on Hardware*/
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));
            cr |= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);


            /* Clear Reset */
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));
            cr &= ~CPH_AADMAC_CH1_AADMAC_SR_1_CH1_AADMAC_FIFO_RST_MASK;
            BRCM_WRITE_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE), cr);
        }

    }

    return;
}

/****************************************************************************
*
*  Function Name: cVoid chal_caph_dma_read_ddrfifo_sw_status(CHAL_HANDLE handle,
*			CAPH_DMA_CHANNEL_e channel)
*
*  Description: read CAPH DMA ddr fifo sw status
*
****************************************************************************/
static CAPH_DMA_CHNL_FIFO_STATUS_e chal_caph_dma_rheaB0_read_ddrfifo_sw_status(CHAL_HANDLE handle,
			CAPH_DMA_CHANNEL_e channel)
{
    cUInt32     base = ((chal_caph_dma_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     cr = (cUInt32)CAPH_READY_NONE;


    /* Find the channel we are looking for */
    for(index = 0; index < CHAL_CAPH_DMA_MAX_CHANNELS; index++)
    {
        if((1UL << index)&channel)
        {
            /* found the channel we are looking for, Get the channel status */
            cr = BRCM_READ_REG_IDX( base,  CPH_AADMAC_CH1_AADMAC_SR_1, (index*CHAL_CAPH_DMA_CH_REG_SIZE));

            /* Retrieve the DDR FIFO staus information from status */
            cr &= (CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_HIGH_MASK|CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_MASK);
            cr >>= CPH_AADMAC_CH1_AADMAC_SR_1_CH1_SW_READY_LOW_SHIFT;
            break;
        }
    }

    return (CAPH_DMA_CHNL_FIFO_STATUS_e)cr;
}

