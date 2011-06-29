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
*  @file   chal_caph_srcmixer.c
*
*  @brief  chal layer driver for caph src ramp mixer
*
****************************************************************************/

#include <chal/chal_caph_srcmixer.h>
#include <mach/rdb/brcm_rdb_srcmixer.h>
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
#define CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS     4
#define CHAL_CAPH_SRCM_MAX_FIFOS      15
#define CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE     ((SRCMIXER_SRC_CHANNEL2_CTRL1_OFFSET - SRCMIXER_SRC_CHANNEL1_CTRL1_OFFSET)/sizeof(uint32_t))
#define CHAL_CAPH_SRCMIXER_SRC_FIFO_CTRL_REG_SIZE     ((SRCMIXER_SRC_CHANNEL1_CTRL2_OFFSET - SRCMIXER_SRC_CHANNEL1_CTRL1_OFFSET)/sizeof(uint32_t))
#define SRCMIXER_SPK_GAIN_CTRL_OFFSET      ( (SRCMIXER_SRC_SPK0_RT_GAIN_CTRL1_OFFSET - SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_OFFSET)/sizeof(uint32_t))
#define SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET      ( (SRCMIXER_SRC_M1D1_CH1M_GAIN_CTRL_OFFSET - SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_OFFSET)/sizeof(uint32_t))

#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_SHIFT     2
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_MASK      0x00000004
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_SHIFT     0
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_MASK      0x00000003

/*
 * ****************************************************************************
 *  local typedef declarations
 * ****************************************************************************
 */
typedef struct
{
    uint32_t     base;                               /* Register Base address */
    uint32_t     fifo_addr[CHAL_CAPH_SRCM_MAX_FIFOS];                        /* FIFO data address */
    uint32_t     ctrl_addr[CHAL_CAPH_SRCM_MAX_FIFOS];                        /* FIFO data address */
} chal_caph_srcm_cb_t;



/*
 * ****************************************************************************
 *  local variable definitions
 * ****************************************************************************
 */
/* chal control block where all information is stored */
static  chal_caph_srcm_cb_t   chal_caph_srcm_cb = { 0,
                                                    {    SRCMIXER_SRC_CHANNEL1_INFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_INFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_INFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_INFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_INFIFO_DATA0_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL1_INFIFO_DATA0_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL2_INFIFO_DATA0_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL1_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_OUTFIFO_DATA0_OFFSET,
                                                         SRCMIXER_MIXER1_OUTFIFO_DATA0_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO1_DATA_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO2_DATA_OFFSET
                                                     },
                                                    {    SRCMIXER_SRC_CHANNEL1_CTRL1_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_CTRL1_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_CTRL1_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_CTRL1_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_CTRL1_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL1_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_CTRL2_OFFSET,
                                                         SRCMIXER_MIXER1_OUTFIFO_CTRL_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO1_CTRL_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO2_CTRL_OFFSET
                                                     }
                                                    };

static const uint8_t    chal_caph_srcm_gain_ctrl_index[] = {1, 0, 3, 2};

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


/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_srcmixer_init(uint32_t baseAddress)
*
*  Description: Initialize CAPH SRC RAMP MIXER block
*
****************************************************************************/
CHAL_HANDLE chal_caph_srcmixer_init(uint32_t baseAddress)
{
    /* Set the register base address to the caller supplied base address */
    chal_caph_srcm_cb.base = baseAddress;

	return(CHAL_HANDLE)(&chal_caph_srcm_cb);
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_deinit(CHAL_HANDLE handle)
*
*  Description: Deinitialize CAPH SRC RAMP MIXER block
*
****************************************************************************/
void chal_caph_srcmixer_deinit(CHAL_HANDLE handle)
{
    chal_caph_srcm_cb_t  *pchal_cb = (chal_caph_srcm_cb_t*)handle;

    /* Reset the register base address */
    pchal_cb->base = 0;

	return;
}

/****************************************************************************
*
*  Function Name: uint16_t chal_caph_srcmixer_get_fifo_addr(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo)
*
*  Description: get caph srcmixer fifo addr
*
****************************************************************************/
uint32_t chal_caph_srcmixer_get_fifo_addr(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *pchal_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            return (pchal_cb->base + pchal_cb->fifo_addr[index]);
        }
    }

    return 0;
}

/****************************************************************************
*
*  Function Name: uint16_t chal_caph_srcmixer_get_fifo_addr_offset(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo)
*
*  Description: get caph srcmixer physical fifo addr
*
****************************************************************************/
uint32_t chal_caph_srcmixer_get_fifo_addr_offset(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *pchal_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            return (pchal_cb->fifo_addr[index]);
        }
    }

    return 0;
}


/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_clr_all_tapbuffers(CHAL_HANDLE handle)
*
*  Description: clear caph srcmixer tap buffers
*
****************************************************************************/
void chal_caph_srcmixer_clr_all_tapbuffers(CHAL_HANDLE handle)
{
    chal_caph_srcm_cb_t     *pchal_cb = (chal_caph_srcm_cb_t*)handle;
    /* write 1 then 0 to clear the all the taps up and taps
     * down FIFO to initila state.
     */
    BRCM_WRITE_REG(pchal_cb->base, SRCMIXER_SRC_GLOBAL_CTRL, 0x1);
    BRCM_WRITE_REG(pchal_cb->base, SRCMIXER_SRC_GLOBAL_CTRL, 0x0);
	return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_chnl(CHAL_HANDLE handle,
*			uint16_t chnl)
*
*  Description: enable caph srcmixer channel
*
****************************************************************************/
void chal_caph_srcmixer_enable_chnl(CHAL_HANDLE handle,
			uint16_t chnl)
{
    uint32_t     base = ((chal_caph_srcm_cb_t*)handle)->base;
    uint8_t      index;
    uint32_t     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX(base,
                                        SRCMIXER_SRC_CHANNEL1_CTRL1,
                                        (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));

            reg_val |= SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_MASK;
            BRCM_WRITE_REG_IDX(base,
                               SRCMIXER_SRC_CHANNEL1_CTRL1,
                               (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
        }
    }

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_CH5_INFIFO & chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val |= SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_MODE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);
    }

    /* Find the 1st stereo passthrough CHNLs we are looking for */
    if (CAPH_PASSCH1_INFIFO & chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);
    }

    /* Find the 2nd stereo passthrough CHNLs we are looking for */
    if (CAPH_PASSCH2_INFIFO & chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_chnl(CHAL_HANDLE handle,
*			uint8_t chnl)
*
*  Description: disable caph srcmixer channel
*
****************************************************************************/
void chal_caph_srcmixer_disable_chnl(CHAL_HANDLE handle,
			uint8_t chnl)
{
    uint32_t     base = ((chal_caph_srcm_cb_t*)handle)->base;
    uint8_t      index;
    uint32_t     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX( base,  SRCMIXER_SRC_CHANNEL1_CTRL1,
                    (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_MASK;
            BRCM_WRITE_REG_IDX(base,  SRCMIXER_SRC_CHANNEL1_CTRL1,
                    (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
        }
    }

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_SRCM_CH5&chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val &= ~SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_MODE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);
    }

    /* Find the 1st stereo passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH1&chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);
        reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);
    }

    /* Find the 2nd stereo passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH2&chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);
        reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);
    }

	return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_SRC(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_SRC_e sampleRate)
*
*  Description: set caph srcmixer SRC for mono/stereo input channels
*
****************************************************************************/
void chal_caph_srcmixer_set_SRC(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_SRC_e sampleRate)
{
    uint32_t     base = ((chal_caph_srcm_cb_t*)handle)->base;
    uint8_t      index;
    uint32_t     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX( base,  SRCMIXER_SRC_CHANNEL1_CTRL1,
                    (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_MASK;
            reg_val |= (uint32_t)sampleRate;
            BRCM_WRITE_REG_IDX(base,  SRCMIXER_SRC_CHANNEL1_CTRL1, (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
            return;
        }
    }

    /* for other channels, there is no need to configure SRC */
	return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_filter_type(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_SRC_e sampleRate)
*
*  Description: set caph srcmixer SRC for mono/stereo input channels
*
****************************************************************************/
void chal_caph_srcmixer_set_filter_type(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_FILT_TYPE_e filter)
{
    uint32_t     base = ((chal_caph_srcm_cb_t*)handle)->base;
    uint8_t      index;
    uint32_t     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX( base,  SRCMIXER_SRC_CHANNEL1_CTRL1, (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_FILTERTYPE_MASK;
            reg_val |= ((uint32_t)filter << SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_FILTERTYPE_SHIFT);
            BRCM_WRITE_REG_IDX(base,  SRCMIXER_SRC_CHANNEL1_CTRL1, (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
            return;
        }
    }

    /* for other channels, there is no need to configure SRC */
	return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_fifo_datafmt(CHAL_HANDLE handle,
*			CAPH_SRCMixer_NORMAL_FIFO_e fifo,
*			CAPH_DATA_FORMAT_e dataFmt)
*
*  Description: set caph srcmixer fifo data format
*
****************************************************************************/
void chal_caph_srcmixer_set_fifo_datafmt(CHAL_HANDLE handle,
            CAPH_SRCMixer_FIFO_e fifo,
            CAPH_DATA_FORMAT_e dataFmt)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;
    uint32_t     reg_val;
    uint32_t     fmt = (uint32_t)dataFmt;


    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            if((fifo == CAPH_PASSCH1_INFIFO) || (fifo == CAPH_PASSCH2_INFIFO))
            {
                switch(dataFmt)
                {
                    case CAPH_MONO_16BIT:
                        fmt = 0x01;
                        break;
                    case CAPH_STEREO_16BIT:
                        fmt = 0x00;
                        break;
                    case CAPH_MONO_24BIT:
                        fmt = 0x03;
                        break;
                    case CAPH_STEREO_24BIT:
                        fmt = 0x02;
                        break;
                    default:
                        break;
                }
                if(fifo == CAPH_PASSCH1_INFIFO)
                {
                    reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);
                    reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FORMAT_MASK;
                    reg_val |= (fmt << SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FORMAT_SHIFT);
                    BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL,  reg_val);
                }
                else
                {
                    reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);
                    reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FORMAT_MASK;
                    reg_val |= (fmt << SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FORMAT_SHIFT);
                    BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL,  reg_val);
                }
            }
            else
            {
                reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)));

                reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_FORMAT_MASK;
                fmt  = (uint32_t)(dataFmt & CAPH_MONO_24BIT);
                reg_val |= (fmt << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_FORMAT_SHIFT);
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)), reg_val);
            }
            return;
        }
    }

    /* for other channels, there is no need to configure SRC */
    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_clr_fifo(CHAL_HANDLE handle,
*			uint16_t fifo)
*
*  Description: clear caph srcmixer fifo
*
****************************************************************************/
void chal_caph_srcmixer_clr_fifo(CHAL_HANDLE handle, uint16_t fifo)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;
    uint32_t     reg_val;

    /* Find the FIFO we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            if((1UL << index) == CAPH_PASSCH1_INFIFO)
            {
                reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);

                reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);

                reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);
            }
            else
            if((1UL << index) == CAPH_PASSCH2_INFIFO)
            {
                reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);

                reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);

                reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);
            }
            else
            {
                reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)));

                reg_val |= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_CLEAR_MASK;
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)), reg_val);

                reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_CLEAR_MASK;
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)), reg_val);
            }
        }
    }

    /* for other channels, there is no need to configure SRC */
    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_fifo_thres(CHAL_HANDLE handle,
*			CAPH_SRCMixer_NORMAL_FIFO_e fifo,
*			uint8_t thres,
*			uint8_t thres2)
*
*  Description: set caph srcmixer fifo threshold
*
****************************************************************************/
void chal_caph_srcmixer_set_fifo_thres(CHAL_HANDLE handle,
            CAPH_SRCMixer_FIFO_e fifo,
            uint8_t thres,
            uint8_t thres2)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;
    uint32_t     reg_val;

    if((fifo == CAPH_PASSCH1_INFIFO) || (fifo == CAPH_PASSCH2_INFIFO))
    {
        /* Nothing to do */
        return;
    }

    /* Find the FIFO we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)));

            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES1_MASK;
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES2_MASK;
            reg_val |= (thres << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES1_SHIFT);
            reg_val |= (thres2 << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES2_SHIFT);
            BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)), reg_val);
            return;
        }
    }

    /* for other channels, there is no need to configure SRC */
    return;
}

/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e chal_caph_srcmixer_read_fifo_status(
*           CHAL_HANDLE handle,
*			CAPH_SRCMixer_NORMAL_FIFO_e fifo)
*
*  Description: read caph srcmixer fifo status: underflow or overflow
*
****************************************************************************/
CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e chal_caph_srcmixer_read_fifo_status(CHAL_HANDLE handle,
        CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;
    uint32_t     reg_val;
    CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e status = CAPH_TAP_NONE;

    if((fifo == CAPH_PASSCH1_INFIFO) || (fifo == CAPH_PASSCH2_INFIFO))
    {
        /* Nothing to do */
        return CAPH_TAP_NONE;
    }

    /* Find the FIFO we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)));

            if(reg_val & SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_UDF_MASK)
            {
                status = CAPH_TAP_UDF;
            }

            if(reg_val & SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_OVF_MASK)
            {
                status = CAPH_TAP_OVF;
            }

            return status;
        }
    }


	return status;
}

/****************************************************************************
*
*  Function Name: uint8_t chal_caph_srcmixer_read_fifo_emptycount(CHAL_HANDLE handle,
			CAPH_SRCMixer_NORMAL_FIFO_e fifo)
*
*  Description: read caph srcmixer fifo empty count
*
****************************************************************************/
uint8_t chal_caph_srcmixer_read_fifo_emptycount(CHAL_HANDLE handle,
            CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      index;
    uint32_t     reg_val = 0;

    if((fifo == CAPH_PASSCH1_INFIFO) || (fifo == CAPH_PASSCH2_INFIFO))
    {
        /* Nothing to do */
        return 0;
    }

    /* Find the FIFO we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCM_MAX_FIFOS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&fifo)
        {
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(uint32_t)));

            reg_val &= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_EMPTY_COUNT_MASK;
            reg_val >>= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_EMPTY_COUNT_SHIFT;

            return (uint8_t)reg_val;
        }
    }


	return (uint8_t)reg_val;
}

/****************************************************************************
*
*  Function Name: uint8_t chal_caph_srcmixer_write_fifo(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo,
*			uint32_t* data,
*			uint8_t size,
			_Bool   forceovf)
*
*  Description: write caph srcmixer fifo
*
****************************************************************************/
uint8_t chal_caph_srcmixer_write_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			uint32_t* data,
			uint8_t size,
			_Bool   forceovf)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      findex;
    uint16_t     dindex;
    uint32_t     fsize = 0;


    /* Convert the size into 32bit words */
    size = size/sizeof(uint32_t);

    /* Find the FIFO we are looking for */
    for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << findex)&fifo)
        {
            fsize  = chal_caph_srcmixer_read_fifo_emptycount(handle, fifo);

            if(forceovf == FALSE)
            {
                if(size > fsize)
                {
                    size = fsize;
                }
            }

            dindex = 0;
            while(dindex<size)
            {
                BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_CHANNEL1_INFIFO_DATA, ((srcm_cb->fifo_addr[findex]- SRCMIXER_SRC_CHANNEL1_INFIFO_DATA_OFFSET)/sizeof(uint32_t)), data[dindex++]);
            }
            break;
        }

    }

    return (size*sizeof(uint32_t));


}

/****************************************************************************
*
*  Function Name: uint8_t chal_caph_srcmixer_read_fifo(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo,
*			uint32_t* data,
*			uint8_t size,
			_Bool   forceudf)
*
*  Description: read caph srcmixer fifo
*
****************************************************************************/
uint8_t chal_caph_srcmixer_read_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			uint32_t* data,
			uint8_t size,
			_Bool   forceudf)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint8_t      findex;
    uint16_t     dindex;
    uint32_t     fsize = 0;

    /* Convert the size into 32bit words */
    size = size/sizeof(uint32_t);

    /* Find the FIFO we are looking for */
    for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << findex)&fifo)
        {
            fsize  = chal_caph_srcmixer_read_fifo_emptycount(handle, fifo);
            if(forceudf == FALSE)
            {
                if(size > fsize)
                {
                    size = fsize;
                }
            }

            dindex = 0;
            while(dindex<size)
            {
                data[dindex++] = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_CHANNEL1_INFIFO_DATA, ((srcm_cb->fifo_addr[findex] - SRCMIXER_SRC_CHANNEL1_INFIFO_DATA_OFFSET)/sizeof(uint32_t)));
            }
            break;
        }

    }

    return (size*sizeof(uint32_t));
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_mixing(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_OUTPUT_e mixerOutputr)
*
*  Description: enable mixing of channel
*
****************************************************************************/
void chal_caph_srcmixer_enable_mixing(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           mindex;
    uint8_t                           findex;

    for(mindex = 0; mindex < 4; mindex++)
    {
        if((1UL << mindex) == mixerOutput)
        {
            for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
            {
                if((1UL << findex) & chnl)
                {
                    reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL, (chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex));
                    reg_val |= SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_ON_MASK;
                    BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL,(chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex), reg_val);
                }
            }

            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_mixing(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_OUTPUT_e mixerOutputr)
*
*  Description: disable mixing of channel
*
****************************************************************************/
void chal_caph_srcmixer_disable_mixing(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           mindex;
    uint8_t                           findex;

    for(mindex = 0; mindex < 4; mindex++)
    {
        if((1UL << mindex) == mixerOutput)
        {

            for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
            {
                if((1UL << findex) & chnl)
                {
                    reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL, (chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex));
                    reg_val &= ~SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_ON_MASK;
                    BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL,(chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex), reg_val);
                }
            }

            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_mixingain(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_OUTPUT_e mixerOutputr,
*			uint16_t gain)
*
*  Description: set caph srcmixer mixer in gain
*
****************************************************************************/
void chal_caph_srcmixer_set_mixingain(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t gain)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           mindex;
    uint8_t                           findex;

    for(mindex = 0; mindex < 4; mindex++)
    {
        if((1UL << mindex) == mixerOutput)
        {

            for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
            {
                if((1UL << findex) & chnl)
                {
                    reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL, (chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex));
                    reg_val &= ~SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_MASK;
                    reg_val |=  (gain & SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_MASK);
                    BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL,(chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex), reg_val);
                }
            }

            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_mixingainstep(CHAL_HANDLE handle,
*			CAPH_SRCMixer_CHNL_e chnl,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint16_t step)
*
*  Description: set caph srcmixer mixer in gain ramp
*
****************************************************************************/
void chal_caph_srcmixer_set_mixingainstep(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t step)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           mindex;
    uint8_t                           findex;

    for(mindex = 0; mindex < 4; mindex++)
    {
        if((1UL << mindex) == mixerOutput)
        {

            for(findex = 0; findex < CHAL_CAPH_SRCM_MAX_FIFOS; findex++)
            {
                if((1UL << findex) & chnl)
                {
                    reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL, (chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex));
                    reg_val &= ~SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_MASK;
                    reg_val |=  (step << SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_SHIFT);
                    BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL,(chal_caph_srcm_gain_ctrl_index[mindex]*SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET + findex), reg_val);
                }
            }

            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint16_t gain)
*
*  Description: set caph srcmixer speaker gain
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t gain)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_FIXED_GAIN_MASK;
            reg_val |=  (gain & SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_FIXED_GAIN_MASK);
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_bitsel(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint8_t bitSel)
*
*  Description: set caph srcmixer speaker gain bitsel
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_bitsel(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint8_t bitSel)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_BIT_SELECT_MASK;
            reg_val |=  (bitSel << SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_BIT_SELECT_SHIFT);
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_load_spkrgain_iir_coeff(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint8_t* coeff)
*
*  Description: load caph srcmixer speaker gain iir coeff
*
****************************************************************************/
void chal_caph_srcmixer_load_spkrgain_iir_coeff(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint8_t* coeff)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_IIR_COEFF_MASK;
            reg_val |= (coeff[0] << SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_IIR_COEFF_SHIFT);
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_reset_spkrgain_iir(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: reset caph srcmixer speaker gain iir
*
****************************************************************************/
void chal_caph_srcmixer_reset_spkrgain_iir(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_IIR_RESET_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2_SPK0_LT_IIR_RESET_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL2,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_slope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint32_t slope)
*
*  Description: set caph srcmixer speaker gain slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_SLOPE_MASK;
            reg_val |=  (slope & SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_SLOPE_MASK);
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_spkrgain_slope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: enable caph srcmixer speaker gain slope
*
****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_spkrgain_slope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: disable caph srcmixer speaker gain slope
*
****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}


/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_aldc(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: enable caph srcmixer speaker protection logic
*
****************************************************************************/
void chal_caph_srcmixer_enable_aldc(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_ALDC_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_aldc(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: disable caph srcmixer speaker protection logic
*
****************************************************************************/
void chal_caph_srcmixer_disable_aldc(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_ALDC_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}


/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_spkrgain_compresser(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: enable caph srcmixer speaker gain compress
*
****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
   chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_COMPRESSOR_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_spkrgain_compresser(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: disable caph srcmixer speaker gain compress
*
****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
   chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_COMPRESSOR_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_compresser_attack(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint16_t step,
*			uint16_t thres)
*
*  Description: set caph srcmixer speaker gain compress attack
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attack(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t step,
			uint16_t thres)
{
   chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_ATTACK_THRES_MASK;
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_ATTACK_STEP_MASK;

            reg_val |= (step <<  SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_ATTACK_STEP_SHIFT);
            reg_val |= (thres <<  SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3_SPK0_LT_ATTACK_THRES_SHIFT);

            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL3,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_compresser_mode(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			CAPH_SRCMixer_CHNL_MODE_e chnlMode)
*
*  Description: set caph srcmixer speaker gain compress mode
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_mode(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			CAPH_SRCMixer_CHNL_MODE_e chnlMode)
{
   chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_STEREO_MASK;
            if(chnlMode == CAPH_SRCM_STEREO)
            {
                reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_SPK0_LT_STEREO_MASK;
            }
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: enable caph srcmixer speaker gain compress attack slope
*
****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4_SPK0_LT_ATTACK_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: disable caph srcmixer speaker gain compress attack slope
*
****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4_SPK0_LT_ATTACK_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint32_t slope)
*
*  Description: set caph srcmixer speaker gain compress attack slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    slope &= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4_SPK0_LT_ATTACK_SLOPE_MASK;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4_SPK0_LT_ATTACK_SLOPE_MASK;
            reg_val |= slope;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL4,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: enable caph srcmixer speaker gain compress decay slope
*
****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val |= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5_SPK0_LT_DECAY_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput)
*
*  Description: disable caph srcmixer speaker gain compress decay slope
*
****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5_SPK0_LT_DECAY_SLOPE_EN_MASK;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
*			CAPH_SRCMixer_OUTPUT_e mixerOutput,
*			uint32_t slope)
*
*  Description: set caph srcmixer speaker gain compress decay slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    uint32_t                         reg_val;
    uint8_t                           index;

    slope &= SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5_SPK0_LT_DECAY_SLOPE_MASK;

    for(index = 0; index < 4; index++)
    {
        if((1UL  << index) == mixerOutput)
        {
            reg_val = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5, (index*SRCMIXER_SPK_GAIN_CTRL_OFFSET));
            reg_val &= ~SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5_SPK0_LT_DECAY_SLOPE_MASK;
            reg_val |= slope;
            BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_SPK0_LT_GAIN_CTRL5,(index*SRCMIXER_SPK_GAIN_CTRL_OFFSET), reg_val);
            break;
        }
    }

	return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>


EXPORT_SYMBOL(chal_caph_srcmixer_init);
EXPORT_SYMBOL(chal_caph_srcmixer_deinit);
EXPORT_SYMBOL(chal_caph_srcmixer_get_fifo_addr);
EXPORT_SYMBOL(chal_caph_srcmixer_get_fifo_addr_offset);
EXPORT_SYMBOL(chal_caph_srcmixer_clr_all_tapbuffers);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_chnl);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_chnl);
EXPORT_SYMBOL(chal_caph_srcmixer_set_SRC);
EXPORT_SYMBOL(chal_caph_srcmixer_set_filter_type);
EXPORT_SYMBOL(chal_caph_srcmixer_set_fifo_datafmt);
EXPORT_SYMBOL(chal_caph_srcmixer_clr_fifo);
EXPORT_SYMBOL(chal_caph_srcmixer_set_fifo_thres);
EXPORT_SYMBOL(chal_caph_srcmixer_read_fifo_status);
EXPORT_SYMBOL(chal_caph_srcmixer_read_fifo_emptycount);
EXPORT_SYMBOL(chal_caph_srcmixer_write_fifo);
EXPORT_SYMBOL(chal_caph_srcmixer_read_fifo);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_mixing);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_mixing);
EXPORT_SYMBOL(chal_caph_srcmixer_set_mixingain);
EXPORT_SYMBOL(chal_caph_srcmixer_set_mixingainstep);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_bitsel);
EXPORT_SYMBOL(chal_caph_srcmixer_load_spkrgain_iir_coeff);
EXPORT_SYMBOL(chal_caph_srcmixer_reset_spkrgain_iir);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_slope);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_spkrgain_slope);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_spkrgain_slope);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_aldc);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_aldc);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_spkrgain_compresser);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_spkrgain_compresser);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_compresser_attack);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_compresser_mode);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_spkrgain_compresser_attackslope);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_spkrgain_compresser_attackslope);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_compresser_attackslope);
EXPORT_SYMBOL(chal_caph_srcmixer_enable_spkrgain_compresser_decayslope);
EXPORT_SYMBOL(chal_caph_srcmixer_disable_spkrgain_compresser_decayslope);
EXPORT_SYMBOL(chal_caph_srcmixer_set_spkrgain_compresser_decayslope);

#endif

