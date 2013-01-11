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

#include "chal_caph.h"
#include "chal_caph_srcmixer.h"
#include "brcm_rdb_srcmixer.h"
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
#define CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS     4
#define CHAL_CAPH_SRCM_MAX_FIFOS      18
#define CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE     ((SRCMIXER_SRC_CHANNEL2_CTRL1_OFFSET - SRCMIXER_SRC_CHANNEL1_CTRL1_OFFSET)/sizeof(cUInt32))
#define CHAL_CAPH_SRCMIXER_SRC_FIFO_CTRL_REG_SIZE     ((SRCMIXER_SRC_CHANNEL1_CTRL2_OFFSET - SRCMIXER_SRC_CHANNEL1_CTRL1_OFFSET)/sizeof(cUInt32))
#define SRCMIXER_SPK_GAIN_CTRL_OFFSET      ( (SRCMIXER_SRC_SPK0_RT_GAIN_CTRL1_OFFSET - SRCMIXER_SRC_SPK0_LT_GAIN_CTRL1_OFFSET)/sizeof(cUInt32))
#define SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET      ( (SRCMIXER_SRC_M1D1_CH1M_GAIN_CTRL_OFFSET - SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_OFFSET)/sizeof(cUInt32))

#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_SHIFT     2
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_ENABLE_MASK      0x00000004
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_SHIFT     0
#define    SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_MASK      0x00000003
#define    SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_SHIFT                     5
#define    SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_MASK                      0x00000020
#define    SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_LRSWITCH_MASK         0x00000008


/*
 * ****************************************************************************
 *  local typedef declarations
 * ****************************************************************************
 */
typedef struct
{
    cUInt32     base;                               /* Register Base address */
    cUInt32     fifo_addr[CHAL_CAPH_SRCM_MAX_FIFOS];                        /* FIFO data address */
    cUInt32     ctrl_addr[CHAL_CAPH_SRCM_MAX_FIFOS];                        /* FIFO data address */
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
                                                         SRCMIXER_PASSTHROUGH_CHANNEL3_INFIFO_DATA0_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL4_INFIFO_DATA0_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL1_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_OUTFIFO_DATA_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_OUTFIFO_DATA0_OFFSET,
                                                         SRCMIXER_MIXER1_OUTFIFO_DATA0_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO_DATA0_OFFSET,
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
                                                         SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_OFFSET,
                                                         SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL1_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL2_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL3_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL4_CTRL2_OFFSET,
                                                         SRCMIXER_SRC_CHANNEL5_CTRL2_OFFSET,
                                                         SRCMIXER_MIXER1_OUTFIFO_CTRL_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO_CTRL_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO1_CTRL_OFFSET,
                                                         SRCMIXER_MIXER2_OUTFIFO2_CTRL_OFFSET
                                                     }
                                                    };

static const cUInt8    chal_caph_srcm_gain_ctrl_index[] = {1, 0, 3, 2};

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
*  Function Name: CHAL_HANDLE chal_caph_srcmixer_init(cUInt32 baseAddress)
*
*  Description: Initialize CAPH SRC RAMP MIXER block
*
****************************************************************************/
CHAL_HANDLE chal_caph_srcmixer_init(cUInt32 baseAddress)
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
*  Function Name: cUInt16 chal_caph_srcmixer_get_fifo_addr(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo)
*
*  Description: get caph srcmixer fifo addr
*
****************************************************************************/
cUInt32 chal_caph_srcmixer_get_fifo_addr(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *pchal_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      index;

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
*  Function Name: cUInt16 chal_caph_srcmixer_get_fifo_addr_offset(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo)
*
*  Description: get caph srcmixer physical fifo addr
*
****************************************************************************/
cUInt32 chal_caph_srcmixer_get_fifo_addr_offset(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *pchal_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      index;

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
	cUInt32	 reg_val;
    /* write 1 then 0 to clear the all the taps up and taps
     * down FIFO to initila state.
     */
	reg_val = BRCM_READ_REG(pchal_cb->base,
			SRCMIXER_SRC_GLOBAL_CTRL);
	reg_val |=
	SRCMIXER_SRC_GLOBAL_CTRL_SRC_GLOBAL_FIFO_CLEAR_MASK;
	BRCM_WRITE_REG(pchal_cb->base,
		SRCMIXER_SRC_GLOBAL_CTRL, reg_val);
	reg_val &=
	~SRCMIXER_SRC_GLOBAL_CTRL_SRC_GLOBAL_FIFO_CLEAR_MASK;
	BRCM_WRITE_REG(pchal_cb->base,
		SRCMIXER_SRC_GLOBAL_CTRL, reg_val);
	return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_enable_chnl(CHAL_HANDLE handle,
*			cUInt16 chnl)
*
*  Description: enable caph srcmixer channel
*
****************************************************************************/
void chal_caph_srcmixer_enable_chnl(CHAL_HANDLE handle,
			cUInt32 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

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
    if (CAPH_SRCM_CH5&chnl)//CAPH_CH5_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val |= SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_MODE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);
    }
    /* Find the 1st stereo passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH1&chnl)//CAPH_PASSCH1_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);
    }

    /* Find the 2nd stereo passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH2&chnl)//CAPH_PASSCH2_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);
    }

	/* Find the 1st mono passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH3&chnl)//CAPH_PASSCH3_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL, reg_val);
    }

    /* Find the 2nd mono passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH4&chnl)//CAPH_PASSCH4_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL);
        reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL, reg_val);
    }

    return;
}


/****************************************************************************
*
*  Function Name: void cchal_caph_srcmixer_tap_enable_chn(CHAL_HANDLE handle,
*			cUInt16 chnl)
*
*  Description: enable caph srcmixer tap path
*
****************************************************************************/
void chal_caph_srcmixer_tap_enable_chnl(CHAL_HANDLE handle,
			cUInt16 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX(base,
                                        SRCMIXER_SRC_CHANNEL1_CTRL2,
                                        (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));

            reg_val |= SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_MASK;
            BRCM_WRITE_REG_IDX(base,
                               SRCMIXER_SRC_CHANNEL1_CTRL2,
                               (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
        }
    }

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_SRCM_CH5&chnl)//CAPH_CH5_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL2);
        reg_val |= SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL2, reg_val);
    }

    return;
}


/****************************************************************************
*
*  Function Name: void cchal_caph_srcmixer_tap_disable_chn(CHAL_HANDLE handle,
*			cUInt16 chnl)
*
*  Description: disable caph srcmixer tap path
*
****************************************************************************/
void chal_caph_srcmixer_tap_disable_chnl(CHAL_HANDLE handle,
			cUInt16 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX(base,
                                        SRCMIXER_SRC_CHANNEL1_CTRL2,
                                        (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));

            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_MASK;
            BRCM_WRITE_REG_IDX(base,
                               SRCMIXER_SRC_CHANNEL1_CTRL2,
                               (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE), reg_val);
        }
    }

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_SRCM_CH5&chnl)//CAPH_CH5_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL2);
        reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL2_SRC_ONLY_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL2, reg_val);
    }

    return;
}

/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_disable_chnl(CHAL_HANDLE handle,
*			cUInt8 chnl)
*
*  Description: disable caph srcmixer channel
*
****************************************************************************/
void chal_caph_srcmixer_disable_chnl(CHAL_HANDLE handle,
			cUInt32 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

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
    if (CAPH_SRCM_CH5&chnl)//CAPH_CH5_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val &= ~SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_MODE_MASK;
        /*do not stop src5 to avoid the rare distortion */
	/*BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);*/
    }

    /* Find the 1st stereo passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH1&chnl)//CAPH_PASSCH1_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL);
        reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL_PASSTHROUGH_CHANNEL1_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL1_CTRL, reg_val);
    }

    /* Find the 2nd stereo passthrough CHNLs we are looking for */
     if (CAPH_SRCM_PASSCH2&chnl)//CAPH_PASSCH2_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL);
        reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL_PASSTHROUGH_CHANNEL2_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL2_CTRL, reg_val);
    }
    /* Find the 1st mono passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH3&chnl)//CAPH_PASSCH3_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL);
		reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL, reg_val);
    }

    /* Find the 2nd mono passthrough CHNLs we are looking for */
    if (CAPH_SRCM_PASSCH4&chnl) //CAPH_PASSCH4_INFIFO
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL);
        reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_ENABLE_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL, reg_val);
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
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX( base,  SRCMIXER_SRC_CHANNEL1_CTRL1,
                    (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_MODE_SELECT_MASK;
            reg_val |= (cUInt32)sampleRate;
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
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt8      index;
    cUInt32     reg_val;

    /* Find the mono input CHNLs we are looking for */
    for(index = 0; index < CHAL_CAPH_SRCMIXER_MAX_MONO_CHNLS; index++)
    {
        /* found the channel we are looking for, enable the channel */
        if((1UL << index)&chnl)
        {
            reg_val = BRCM_READ_REG_IDX( base,  SRCMIXER_SRC_CHANNEL1_CTRL1, (index*CHAL_CAPH_SRCMIXER_SRC_MONO_INCHNL_CTRL_REG_SIZE));
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_FILTERTYPE_MASK;
            reg_val |= ((cUInt32)filter << SRCMIXER_SRC_CHANNEL1_CTRL1_SRC_CHANNEL1_FILTERTYPE_SHIFT);
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
    cUInt8      index;
    cUInt32     reg_val;
    cUInt32     fmt = (cUInt32)dataFmt;


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
                        fmt = 0x00;
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
	    else if((fifo == CAPH_PASSCH3_INFIFO) || (fifo == CAPH_PASSCH4_INFIFO))
	    {
                switch(dataFmt)
                {
                    case CAPH_MONO_16BIT:
                        fmt = 0x01;
                        break;
                    case CAPH_MONO_24BIT:
                        fmt = 0x03;
                        break;
                    default:
                        fmt = 0x00;
                        break;


                }
                if(fifo == CAPH_PASSCH3_INFIFO)
                {
                    reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL);
                    reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_FORMAT_MASK;
                    reg_val |= (fmt << SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_FORMAT_SHIFT);
                    BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL,  reg_val);
                }
                else
                {
                    reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL);
                    reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_FORMAT_MASK;
                    reg_val |= (fmt << SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_FORMAT_SHIFT);
                    BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL,  reg_val);
                }
	    }
            else
            {
                reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)));

                reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_FORMAT_MASK;
                fmt  = (cUInt32)(dataFmt & CAPH_MONO_24BIT);
                reg_val |= (fmt << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_FORMAT_SHIFT);
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)), reg_val);
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
*			cUInt16 fifo)
*
*  Description: clear caph srcmixer fifo
*
****************************************************************************/
void chal_caph_srcmixer_clr_fifo(CHAL_HANDLE handle, CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      index;
    cUInt32     reg_val;

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
            if((1UL << index) == CAPH_PASSCH3_INFIFO)
            {
                reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL);

                reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL, reg_val);

                reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL_PASSTHROUGH_CHANNEL3_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL3_CTRL, reg_val);
            }
            else
            if((1UL << index) == CAPH_PASSCH4_INFIFO)
            {
                reg_val = BRCM_READ_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL);

                reg_val |= SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL, reg_val);

                reg_val &= ~SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL_PASSTHROUGH_CHANNEL4_FIFO_CLEAR_MASK;
                BRCM_WRITE_REG( srcm_cb->base,  SRCMIXER_PASSTHROUGH_CHANNEL4_CTRL, reg_val);
            }
            else
            {
                reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)));

                reg_val |= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_CLEAR_MASK;
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)), reg_val);

                reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_CLEAR_MASK;
                BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)), reg_val);

                /* Do the additional SRCCH5 OUTFIFO clear as well.
					This is the FIFO between SRC and MIXER and is not
					visible externally */
				reg_val = BRCM_READ_REG(srcm_cb->base,
						SRCMIXER_SRC_CHANNEL5_CTRL2);
				reg_val |=
			SRCMIXER_SRC_CHANNEL5_CTRL2_TAPSDOWNFIFO5_CLEAR_MASK;
				BRCM_WRITE_REG(srcm_cb->base,
						SRCMIXER_SRC_CHANNEL5_CTRL2, reg_val);
				reg_val &=
				~SRCMIXER_SRC_CHANNEL5_CTRL2_TAPSDOWNFIFO5_CLEAR_MASK;
				BRCM_WRITE_REG(srcm_cb->base,
						SRCMIXER_SRC_CHANNEL5_CTRL2, reg_val);
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
*			cUInt8 thres,
*			cUInt8 thres2)
*
*  Description: set caph srcmixer fifo threshold
*
****************************************************************************/
void chal_caph_srcmixer_set_fifo_thres(CHAL_HANDLE handle,
            CAPH_SRCMixer_FIFO_e fifo,
            cUInt8 thres,
            cUInt8 thres2)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      index;
    cUInt32     reg_val;

    if((fifo == CAPH_PASSCH1_INFIFO) || (fifo == CAPH_PASSCH2_INFIFO) || (fifo == CAPH_PASSCH3_INFIFO) || (fifo == CAPH_PASSCH4_INFIFO))
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
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)));

            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES1_MASK;
            reg_val &= ~SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES2_MASK;
            reg_val |= (thres << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES1_SHIFT);
            reg_val |= (thres2 << SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_THRES2_SHIFT);
            BRCM_WRITE_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)), reg_val);
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
    cUInt8      index;
    cUInt32     reg_val;
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
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)));

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
*  Function Name: cUInt8 chal_caph_srcmixer_read_fifo_emptycount(CHAL_HANDLE handle,
			CAPH_SRCMixer_NORMAL_FIFO_e fifo)
*
*  Description: read caph srcmixer fifo empty count
*
****************************************************************************/
cUInt8 chal_caph_srcmixer_read_fifo_emptycount(CHAL_HANDLE handle,
            CAPH_SRCMixer_FIFO_e fifo)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      index;
    cUInt32     reg_val = 0;

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
            reg_val = BRCM_READ_REG_IDX( srcm_cb->base,  SRCMIXER_SRC_CHANNEL1_CTRL1,  (srcm_cb->ctrl_addr[index]/sizeof(cUInt32)));

            reg_val &= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_EMPTY_COUNT_MASK;
            reg_val >>= SRCMIXER_SRC_CHANNEL1_CTRL1_TAPSUPFIFO1_EMPTY_COUNT_SHIFT;

            return (cUInt8)reg_val;
        }
    }


	return (cUInt8)reg_val;
}

/****************************************************************************
*
*  Function Name: cUInt8 chal_caph_srcmixer_write_fifo(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo,
*			cUInt32* data,
*			cUInt8 size,
			_Bool   forceovf)
*
*  Description: write caph srcmixer fifo
*
****************************************************************************/
cUInt8 chal_caph_srcmixer_write_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			cUInt32* data,
			cUInt8 size,
			_Bool   forceovf)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      findex;
    cUInt16     dindex;
    cUInt32     fsize = 0;


    /* Convert the size into 32bit words */
    size = size/sizeof(cUInt32);

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
                BRCM_WRITE_REG_IDX(srcm_cb->base, SRCMIXER_SRC_CHANNEL1_INFIFO_DATA, ((srcm_cb->fifo_addr[findex]- SRCMIXER_SRC_CHANNEL1_INFIFO_DATA_OFFSET)/sizeof(cUInt32)), data[dindex++]);
            }
            break;
        }

    }

    return (size*sizeof(cUInt32));


}

/****************************************************************************
*
*  Function Name: cUInt8 chal_caph_srcmixer_read_fifo(CHAL_HANDLE handle,
*			CAPH_SRCMixer_FIFO_e fifo,
*			cUInt32* data,
*			cUInt8 size,
			_Bool   forceudf)
*
*  Description: read caph srcmixer fifo
*
****************************************************************************/
cUInt8 chal_caph_srcmixer_read_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			cUInt32* data,
			cUInt8 size,
			_Bool   forceudf)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt8      findex;
    cUInt16     dindex;
    cUInt32     fsize = 0;

    /* Convert the size into 32bit words */
    size = size/sizeof(cUInt32);

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
                data[dindex++] = BRCM_READ_REG_IDX(srcm_cb->base, SRCMIXER_SRC_CHANNEL1_INFIFO_DATA, ((srcm_cb->fifo_addr[findex] - SRCMIXER_SRC_CHANNEL1_INFIFO_DATA_OFFSET)/sizeof(cUInt32)));
            }
            break;
        }

    }

    return (size*sizeof(cUInt32));
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
    cUInt32                         reg_val;
    cUInt8                           mindex;
    cUInt8                           findex;


	// Adjust if mono passthorugh channel is configured
	// Passthrough 1 Mono = CH6R
    // Passthrough 2 Mono = CH7R
    // Passthrough 3 Mono = CH6L
    // Passthrough 4 Mono = CH7L

	switch(chnl)
	{
	case CAPH_SRCM_PASSCH1_M: chnl = CAPH_SRCM_PASSCH1_R; break;
	case CAPH_SRCM_PASSCH2_M: chnl = CAPH_SRCM_PASSCH2_R; break;
	case CAPH_SRCM_PASSCH3: chnl = CAPH_SRCM_PASSCH1_L; break;
	case CAPH_SRCM_PASSCH4: chnl = CAPH_SRCM_PASSCH2_L; break;
	default: break;
	}

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
    cUInt32                         reg_val;
    cUInt8                           mindex;
    cUInt8                           findex;


    // Adjust if mono passthorugh channel is configured
    // Passthrough 1 Mono = CH6R
    // Passthrough 2 Mono = CH7R
    // Passthrough 3 Mono = CH6L
    // Passthrough 4 Mono = CH7L

    switch(chnl)
    {
    case CAPH_SRCM_PASSCH1_M: chnl = CAPH_SRCM_PASSCH1_R; break;
    case CAPH_SRCM_PASSCH2_M: chnl = CAPH_SRCM_PASSCH2_R; break;
    case CAPH_SRCM_PASSCH3: chnl = CAPH_SRCM_PASSCH1_L; break;
    case CAPH_SRCM_PASSCH4: chnl = CAPH_SRCM_PASSCH2_L; break;
    default: break;
    }

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
*			cUInt16 gain)
*
*  Description: set caph srcmixer mixer in gain
*
****************************************************************************/
void chal_caph_srcmixer_set_mixingain(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt16 gain)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           mindex;
    cUInt8                           findex;



	// Adjust if mono passthorugh channel is configured
	// Passthrough 1 Mono = CH6R
    // Passthrough 2 Mono = CH7R
    // Passthrough 3 Mono = CH6L
    // Passthrough 4 Mono = CH7L

	switch(chnl)
	{
	case CAPH_SRCM_PASSCH1_M: chnl = CAPH_SRCM_PASSCH1_R; break;
	case CAPH_SRCM_PASSCH2_M: chnl = CAPH_SRCM_PASSCH2_R; break;
	case CAPH_SRCM_PASSCH3: chnl = CAPH_SRCM_PASSCH1_L; break;
	case CAPH_SRCM_PASSCH4: chnl = CAPH_SRCM_PASSCH2_L; break;
	default: break;
	}

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
*			cUInt16 step)
*
*  Description: set caph srcmixer mixer in gain ramp
*
****************************************************************************/
void chal_caph_srcmixer_set_mixingainstep(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt16 step)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           mindex;
    cUInt8                           findex;


    // Adjust if mono passthorugh channel is configured
    // Passthrough 1 Mono = CH6R
    // Passthrough 2 Mono = CH7R
    // Passthrough 3 Mono = CH6L
    // Passthrough 4 Mono = CH7L

	switch(chnl)
	{
	case CAPH_SRCM_PASSCH1_M: chnl = CAPH_SRCM_PASSCH1_R; break;
	case CAPH_SRCM_PASSCH2_M: chnl = CAPH_SRCM_PASSCH2_R; break;
	case CAPH_SRCM_PASSCH3: chnl = CAPH_SRCM_PASSCH1_L; break;
	case CAPH_SRCM_PASSCH4: chnl = CAPH_SRCM_PASSCH2_L; break;
	default: break;
	}


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
*			cUInt16 gain)
*
*  Description: set caph srcmixer speaker gain
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt16 gain)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt8 bitSel)
*
*  Description: set caph srcmixer speaker gain bitsel
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_bitsel(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt8 bitSel)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt8* coeff)
*
*  Description: load caph srcmixer speaker gain iir coeff
*
****************************************************************************/
void chal_caph_srcmixer_load_spkrgain_iir_coeff(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt8* coeff)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt32 slope)
*
*  Description: set caph srcmixer speaker gain slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt32 slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt16 step,
*			cUInt16 thres)
*
*  Description: set caph srcmixer speaker gain compress attack
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attack(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt16 step,
			cUInt16 thres)
{
   chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt32 slope)
*
*  Description: set caph srcmixer speaker gain compress attack slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt32 slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
    cUInt32                         reg_val;
    cUInt8                           index;

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
*			cUInt32 slope)
*
*  Description: set caph srcmixer speaker gain compress decay slope
*
****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			cUInt32 slope)
{
    chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
    cUInt32                         reg_val;
    cUInt8                           index;

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


/****************************************************************************
*
*  Function Name: void chal_caph_srcmixer_set_mixer2_mono(CHAL_HANDLE handle,
*			_Bool   mono,
*			cUInt32 slope)
*
*  Description: Configures the mixer2 as mono=1/stereo=0 mixer
*
****************************************************************************/
void chal_caph_srcmixer_set_mixer2_mono(CHAL_HANDLE handle, _Bool   mono)
{
        chal_caph_srcm_cb_t     *srcm_cb = (chal_caph_srcm_cb_t*)handle;
	cUInt32                reg_val;

	reg_val = BRCM_READ_REG(srcm_cb->base, SRCMIXER_MIXER2_OUTFIFO_CTRL);
	if (mono)
	{
		reg_val &= ~ SRCMIXER_MIXER2_OUTFIFO_CTRL_MIXER2_OUTFIFO_ENABLE_MASK;
	}
	else
	{
		reg_val |=  SRCMIXER_MIXER2_OUTFIFO_CTRL_MIXER2_OUTFIFO_ENABLE_MASK;
	}
	BRCM_WRITE_REG(srcm_cb->base, SRCMIXER_MIXER2_OUTFIFO_CTRL, reg_val);
}

/****************************************************************************
*
*  Function Name: void cchal_caph_srcmixer_lrswitch_enable_chn(CHAL_HANDLE handle,
*			cUInt16 chnl)
*
*  Description: enable caph srcmixer tap path
*
****************************************************************************/
void chal_caph_srcmixer_lrswitch_enable_chnl(CHAL_HANDLE handle,
					cUInt16 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt32     reg_val;

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_CH5_INFIFO & chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val |= SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_LRSWITCH_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);
    }
    return;
}


/****************************************************************************
*
*  Function Name: void cchal_caph_srcmixer_tap_disable_chn(CHAL_HANDLE handle,
*			cUInt16 chnl)
*
*  Description: disable caph srcmixer tap path
*
****************************************************************************/
void chal_caph_srcmixer_lrswitch_disable_chnl(CHAL_HANDLE handle,
					 cUInt16 chnl)
{
    cUInt32     base = ((chal_caph_srcm_cb_t*)handle)->base;
    cUInt32     reg_val;

    /* Find the stereo input CHNL we are looking for */
    if (CAPH_CH5_INFIFO & chnl)
    {
        reg_val = BRCM_READ_REG( base,  SRCMIXER_SRC_CHANNEL5_CTRL1);
        reg_val &= ~SRCMIXER_SRC_CHANNEL5_CTRL1_SRC_CHANNEL5_LRSWITCH_MASK;
        BRCM_WRITE_REG(base,  SRCMIXER_SRC_CHANNEL5_CTRL1, reg_val);
    }
    return;
}

#if defined( __KERNEL__ )

#include <linux/module.h>

EXPORT_SYMBOL(chal_caph_srcmixer_set_mixer2_mono);
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

