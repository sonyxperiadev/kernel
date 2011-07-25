/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*  @file   csl_caph_srcmixer.c
*
*  @brief  csl layer driver for caph SRCMixer driver
*
****************************************************************************/
#include "log.h"
#include "xassert.h"
#include "auddrv_def.h"
#include "csl_caph.h"
#include "chal_caph_srcmixer.h"
#include "csl_caph_srcmixer.h"

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************
extern UInt16 srcmixer_input_gain_ramp;
extern UInt16 srcmixer_output_gain_slope;
extern UInt8 srcmixer_fifo_thres2;

//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************


//****************************************************************************
// local macro declarations
//****************************************************************************


//****************************************************************************
// local typedef declarations
//****************************************************************************
typedef struct
{
    CSL_CAPH_SRCM_INCHNL_e inChnl; /* input channel */
    Boolean alloc_status;   /* allocation status for each channel */
} CSL_CAPH_SRCM_INCHNL_STATUS_t;

typedef struct
{
    CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl;  /* output channel */
    UInt16 inChnl;  /* input channel connected to the above output channel */
} CSL_CAPH_SRCM_CHNL_TABLE_t;

//****************************************************************************
// local variable definitions
//****************************************************************************
static CHAL_HANDLE handle = 0x0;

/* SRCMixer input channel status table */
static CSL_CAPH_SRCM_INCHNL_STATUS_t inChnlStatus[MAX_INCHNLS] =
{
    //Note: Do not change the order of the MONO CHs in this table.
    //It makes sure that MONO_CH4 will be chosen before MONO_CH1/CH2.
    //Because, MONO CH1/2 are used in voice call.
    {CSL_CAPH_SRCM_MONO_CH1, FALSE},
    {CSL_CAPH_SRCM_MONO_CH2, FALSE},
    {CSL_CAPH_SRCM_MONO_CH3, FALSE},
    {CSL_CAPH_SRCM_MONO_CH4, FALSE},
    {CSL_CAPH_SRCM_STEREO_CH5, FALSE},
    {CSL_CAPH_SRCM_STEREO_PASS_CH1, FALSE},
    {CSL_CAPH_SRCM_STEREO_PASS_CH2, FALSE}
};
/* SRCMixer output channel usage table */
static CSL_CAPH_SRCM_CHNL_TABLE_t chnlTable[OUTCHNL_MAX_NUM_CHNL] =
{
    {CSL_CAPH_SRCM_STEREO_CH1_L, 0x0000},
    {CSL_CAPH_SRCM_STEREO_CH1_R, 0x0000},
    {CSL_CAPH_SRCM_STEREO_CH1, 0x0000},
    {CSL_CAPH_SRCM_STEREO_CH2_L, 0x0000},
    {CSL_CAPH_SRCM_STEREO_CH2_R, 0x0000}
};	


//****************************************************************************
// local function declarations
//****************************************************************************
static void csl_caph_srcmixer_set_all_mixingainramp(CHAL_HANDLE handle, UInt16 gain);
static void csl_caph_srcmixer_disable_all_spkrgain_compressor(CHAL_HANDLE handle);
static void csl_caph_srcmixer_set_all_spkrgain_slope(CHAL_HANDLE handle, UInt32 slope);
//static void csl_caph_srcmixer_enable_all_spkrgain_slope(CHAL_HANDLE handle);
static CAPH_DATA_FORMAT_e csl_caph_srcmixer_get_chal_dataformat(CHAL_HANDLE handle,
                                                   CSL_CAPH_DATAFORMAT_e dataFmt);
static void csl_caph_srcmixer_use_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl);
static void csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl);
static UInt16 csl_caph_srcmixer_read_outchnltable(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl);
static UInt8 csl_caph_srcmixer_get_chaloutchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl);
//static CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_chal_fifo(
//                                                   CSL_CAPH_SRCM_FIFO_e fifo);

//******************************************************************************
// local function definitions
//******************************************************************************

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_set_all_mixingainramp(CHAL_HANDLE handle, UInt16 gain)    
*
*  Description: Set all mixer input gain ramp
*
****************************************************************************/
static void csl_caph_srcmixer_set_all_mixingainramp(CHAL_HANDLE handle, UInt16 gain)
{
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH1, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH1, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH1, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH1, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH2, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH2, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH2, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH2, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH3, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH3, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH3, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH3, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH4, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH4, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH4, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH4, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_L, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_L, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_L, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_L, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_R, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_R, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_R, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH1_R, 
                                         CAPH_M1_Right, gain);


    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_L, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_L, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_L, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_L, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_R, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_R, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_R, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_PASSCH2_R, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_L, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_L, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_L, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_L, 
                                         CAPH_M1_Right, gain);

    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_R, 
                                         CAPH_M0_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_R, 
                                         CAPH_M0_Right, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_R, 
                                         CAPH_M1_Left, gain);
    chal_caph_srcmixer_set_mixingainstep(handle, CAPH_SRCM_CH5_R, 
                                         CAPH_M1_Right, gain);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_disable_all_spkrgain_compressor(CHAL_HANDLE handle)
*
*  Description: Disable all the mixer output gain compressors
*
****************************************************************************/
static void csl_caph_srcmixer_disable_all_spkrgain_compressor(CHAL_HANDLE handle)
{
    chal_caph_srcmixer_disable_spkrgain_compresser(handle, CAPH_M0_Left);
    chal_caph_srcmixer_disable_spkrgain_compresser(handle, CAPH_M0_Right);
    chal_caph_srcmixer_disable_spkrgain_compresser(handle, CAPH_M1_Left);
    chal_caph_srcmixer_disable_spkrgain_compresser(handle, CAPH_M1_Right);
    return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_set_all_spkrgain_slope(CHAL_HANDLE handle, 
*                                                               UInt32 slope)
*
*  Description: Disable all the mixer output gain slopes
*
****************************************************************************/
static void csl_caph_srcmixer_set_all_spkrgain_slope(CHAL_HANDLE handle, UInt32 slope)
{
    chal_caph_srcmixer_set_spkrgain_slope(handle, CAPH_M0_Left, slope);
    chal_caph_srcmixer_set_spkrgain_slope(handle, CAPH_M0_Right, slope);
    chal_caph_srcmixer_set_spkrgain_slope(handle, CAPH_M1_Left, slope);
    chal_caph_srcmixer_set_spkrgain_slope(handle, CAPH_M1_Right, slope);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_enable_all_spkrgain_slope(CHAL_HANDLE handle)
*
*  Description: Ensable all the mixer output gain slopes
*
****************************************************************************/
// Temporarily comment it out, since a compilation warning happens because
// this API is not called anywhere in the code at the moment.
#if 0
static void csl_caph_srcmixer_enable_all_spkrgain_slope(CHAL_HANDLE handle)
{
    chal_caph_srcmixer_enable_spkrgain_slope(handle, CAPH_M0_Left);
    chal_caph_srcmixer_enable_spkrgain_slope(handle, CAPH_M0_Right);
    chal_caph_srcmixer_enable_spkrgain_slope(handle, CAPH_M1_Left);
    chal_caph_srcmixer_enable_spkrgain_slope(handle, CAPH_M1_Right);
    return;
}
#endif

/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_CHNL_e csl_caph_srcmixer_get_single_chal_inchnl(
*                                         CSL_CAPH_SRCM_INCHNL_e inChnl)
*
*  Description: Get one cHAL Mixer Input Channel
*
****************************************************************************/
CAPH_SRCMixer_CHNL_e csl_caph_srcmixer_get_single_chal_inchnl(
                                           CSL_CAPH_SRCM_INCHNL_e inChnl)
{
    CAPH_SRCMixer_CHNL_e chalChnl = CAPH_SRCM_CH_NONE; 
    switch (inChnl)
    {
        case CSL_CAPH_SRCM_INCHNL_NONE:
            chalChnl = CAPH_SRCM_CH_NONE;
            break;
        case CSL_CAPH_SRCM_MONO_CH1:
            chalChnl = CAPH_SRCM_CH1;
            break;
        case CSL_CAPH_SRCM_MONO_CH2:
            chalChnl = CAPH_SRCM_CH2;
            break;
        case CSL_CAPH_SRCM_MONO_CH3:
            chalChnl = CAPH_SRCM_CH3;
            break;
        case CSL_CAPH_SRCM_MONO_CH4:
            chalChnl = CAPH_SRCM_CH4;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1:
            chalChnl = CAPH_SRCM_PASSCH1;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_L:
            chalChnl = CAPH_SRCM_PASSCH1_L;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_R:
            chalChnl = CAPH_SRCM_PASSCH1_R;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2:
            chalChnl = CAPH_SRCM_PASSCH2;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_L:
            chalChnl = CAPH_SRCM_PASSCH2_L;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_R:
            chalChnl = CAPH_SRCM_PASSCH2_R;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5:
            chalChnl = CAPH_SRCM_CH5;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5_L:
            chalChnl = CAPH_SRCM_CH5_L;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5_R:
            chalChnl = CAPH_SRCM_CH5_R;
            break;
        default:
            audio_xassert(0, chalChnl);
    }
    return chalChnl;
}
/****************************************************************************
*
*  Function Name: UInt16 csl_caph_srcmixer_get_chal_inchnl(
*                                                  UInt16 inChnl)
*
*  Description: Get the multiple cHAL Mixer Input Channels
*
****************************************************************************/
UInt16 csl_caph_srcmixer_get_chal_inchnl(UInt16 inChnl)
{
    UInt16 chalChnl = 0x0;
    if (inChnl&CSL_CAPH_SRCM_MONO_CH1) chalChnl |= CAPH_SRCM_CH1;
    if (inChnl&CSL_CAPH_SRCM_MONO_CH2) chalChnl |= CAPH_SRCM_CH2;
    if (inChnl&CSL_CAPH_SRCM_MONO_CH3) chalChnl |= CAPH_SRCM_CH3;
    if (inChnl&CSL_CAPH_SRCM_MONO_CH4) chalChnl |= CAPH_SRCM_CH4;
    if (inChnl&CSL_CAPH_SRCM_STEREO_CH5_L) chalChnl |= CAPH_SRCM_CH5_L;
    if (inChnl&CSL_CAPH_SRCM_STEREO_CH5_R) chalChnl |= CAPH_SRCM_CH5_R;
    if (inChnl&CSL_CAPH_SRCM_STEREO_PASS_CH1_L) chalChnl |= CAPH_SRCM_PASSCH1_L;
    if (inChnl&CSL_CAPH_SRCM_STEREO_PASS_CH1_R) chalChnl |= CAPH_SRCM_PASSCH1_R;
    if (inChnl&CSL_CAPH_SRCM_STEREO_PASS_CH2_L) chalChnl |= CAPH_SRCM_PASSCH2_L;
    if (inChnl&CSL_CAPH_SRCM_STEREO_PASS_CH2_R) chalChnl |= CAPH_SRCM_PASSCH2_R;
    return chalChnl;
}



/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_inchnl_fifo(
*                                                  CSL_CAPH_SRCM_INCHNL_e inChnl)
*
*  Description: Get the cHAL Mixer Input Channel FIFO
*
****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_inchnl_fifo(
                                                   CSL_CAPH_SRCM_INCHNL_e inChnl)
{
    CAPH_SRCMixer_FIFO_e inChnlFIFO = CAPH_CH_INFIFO_NONE; 
    switch (inChnl)
    {
        case CSL_CAPH_SRCM_MONO_CH1:
            inChnlFIFO = CAPH_CH1_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH2:
            inChnlFIFO = CAPH_CH2_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH3:
            inChnlFIFO = CAPH_CH3_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH4:
            inChnlFIFO = CAPH_CH4_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1:
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_L:
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_R:
            inChnlFIFO = CAPH_PASSCH1_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2:
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_L:
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_R:
            inChnlFIFO = CAPH_PASSCH2_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5:
        case CSL_CAPH_SRCM_STEREO_CH5_L:
        case CSL_CAPH_SRCM_STEREO_CH5_R:
            inChnlFIFO = CAPH_CH5_INFIFO;
            break;
        default:
            audio_xassert(0, inChnlFIFO);
    }
    return inChnlFIFO;
}


/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_outchnl_fifo(
*                                                  CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
*
*  Description: Get the cHAL Mixer Output Channel FIFO
*
****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_outchnl_fifo(
                                                   CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
{
    CAPH_SRCMixer_FIFO_e outChnlFIFO = CAPH_CH_INFIFO_NONE; 
    switch (outChnl)
    {
        case CSL_CAPH_SRCM_STEREO_CH1:
        case CSL_CAPH_SRCM_STEREO_CH1_L:
        case CSL_CAPH_SRCM_STEREO_CH1_R:
            outChnlFIFO = CAPH_MIXER1_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_L:
            outChnlFIFO = CAPH_MIXER2_OUTFIFO2;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_R:
            outChnlFIFO = CAPH_MIXER2_OUTFIFO1;
            break;
        default:
            audio_xassert(0, outChnlFIFO);
    }
    return outChnlFIFO;
}



/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_tapoutchnl_fifo(
*                                                  CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
*
*  Description: Get the cHAL Mixer Tap Output Channel FIFO
*
****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_tapoutchnl_fifo(
                                                   CSL_CAPH_SRCM_SRC_OUTCHNL_e outChnl)
{
    CAPH_SRCMixer_FIFO_e outChnlFIFO = CAPH_CH_INFIFO_NONE; 
    switch (outChnl)
    {
        case CSL_CAPH_SRCM_TAP_MONO_CH1:
            outChnlFIFO = CAPH_CH1_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH2:
            outChnlFIFO = CAPH_CH2_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH3:
            outChnlFIFO = CAPH_CH3_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH4:
            outChnlFIFO = CAPH_CH4_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_STEREO_CH5:
            outChnlFIFO = CAPH_CH5_TAP_OUTFIFO;
            break;
        default:
            audio_xassert(0, outChnlFIFO);
    }
    return outChnlFIFO;
}


/****************************************************************************
*
*  Function Name: CSL_CAPH_SWITCH_TRIGGER_e csl_caph_srcmixer_get_inchnl_trigger(
*                                                  CSL_CAPH_SRCM_INCHNL_e inChnl)
*
*  Description: Get the csl Mixer Input Channel trigger
*
****************************************************************************/
CSL_CAPH_SWITCH_TRIGGER_e csl_caph_srcmixer_get_inchnl_trigger(
                                                   CSL_CAPH_SRCM_INCHNL_e inChnl)
{
    CSL_CAPH_SWITCH_TRIGGER_e inChnlTrig = CSL_CAPH_TRIG_NONE; 
	
    switch (inChnl)
    {
        case CSL_CAPH_SRCM_MONO_CH1:
            inChnlTrig = CSL_CAPH_TRIG_TAPSUP_CH1_NORM_INT;
            break;
        case CSL_CAPH_SRCM_MONO_CH2:
            inChnlTrig = CSL_CAPH_TRIG_TAPSUP_CH2_NORM_INT;
            break;
        case CSL_CAPH_SRCM_MONO_CH3:
            inChnlTrig = CSL_CAPH_TRIG_TAPSUP_CH3_NORM_INT;
            break;
        case CSL_CAPH_SRCM_MONO_CH4:
            inChnlTrig = CSL_CAPH_TRIG_TAPSUP_CH4_NORM_INT;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1:
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_L:
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_R:
            inChnlTrig = CSL_CAPH_TRIG_PASSTHROUGH_CH1_FIFO_THRESMET;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2:
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_L:
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_R:
            inChnlTrig = CSL_CAPH_TRIG_PASSTHROUGH_CH2_FIFO_THRESMET;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5:
        case CSL_CAPH_SRCM_STEREO_CH5_L:
        case CSL_CAPH_SRCM_STEREO_CH5_R:
            inChnlTrig = CSL_CAPH_TRIG_TAPSUP_CH5_NORM_INT;
            break;
        default:
            audio_xassert(0, inChnl);
    }
    return inChnlTrig;
}


/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_chaloutchnl(
*                                                  CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
*
*  Description: Get the cHAL Mixer Output Channel
*
****************************************************************************/
static UInt8 csl_caph_srcmixer_get_chaloutchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
{
    UInt8 chalOutChnl = CAPH_M_NONE; 
    switch (outChnl)
    {
        case CSL_CAPH_SRCM_STEREO_CH1_L:
            chalOutChnl = CAPH_M0_Left;
            break;
        case CSL_CAPH_SRCM_STEREO_CH1_R:
            chalOutChnl = CAPH_M0_Right;
            break;
        case CSL_CAPH_SRCM_STEREO_CH1:
            chalOutChnl = CAPH_M0_Right|CAPH_M0_Left;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_L:
            chalOutChnl = CAPH_M1_Left;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_R:
            chalOutChnl = CAPH_M1_Right;
            break;
        case CSL_CAPH_SRCM_CH_NONE:
            chalOutChnl = CAPH_M_NONE;
            break;	    
        default:
            audio_xassert(0, chalOutChnl);
    }
    return chalOutChnl;
}


/****************************************************************************
*
*  Function Name:CAPH_DATA_FORMAT_e csl_caph_srcmixer_get_chal_dataformat(CHAL_HANDLE handle,
                                                   CSL_CAPH_DATAFORMAT_e dataFmt)
*
*  Description: Get the cHAL data forma
*
****************************************************************************/
static CAPH_DATA_FORMAT_e csl_caph_srcmixer_get_chal_dataformat(CHAL_HANDLE handle,
                                                   CSL_CAPH_DATAFORMAT_e dataFmt)
{
    CAPH_DATA_FORMAT_e chalDataFmt = CAPH_MONO_16BIT; 
    switch (dataFmt)
    {
        case CSL_CAPH_16BIT_MONO:
            chalDataFmt = CAPH_MONO_16BIT;
            break;
        case CSL_CAPH_16BIT_STEREO:
            chalDataFmt = CAPH_STEREO_16BIT;
            break;
        case CSL_CAPH_24BIT_MONO:
            chalDataFmt = CAPH_MONO_24BIT;
            break;
        case CSL_CAPH_24BIT_STEREO:
            chalDataFmt = CAPH_STEREO_24BIT;
            break;
        default:
            audio_xassert(0, chalDataFmt);
    }
    return chalDataFmt;
}

/****************************************************************************
*
*  Function Name: CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_chal_fifo(
*                                                  CSL_CAPH_SRCM_FIFO_e fifo)
*
*  Description: Get the cHAL Mixer FIFO
*
****************************************************************************/
// Temporarily comment it out, since a compilation warning happens because
// this API is not called anywhere in the code at the moment.
#if 0
static CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_chal_fifo(
                                                   CSL_CAPH_SRCM_FIFO_e fifo)
{
    CAPH_SRCMixer_FIFO_e chalFIFO = CAPH_CH_INFIFO_NONE; 
    switch (fifo)
    {
        case CSL_CAPH_SRCM_FIFO_NONE:
            chalFIFO = CAPH_CH_INFIFO_NONE;
            break;
        case CSL_CAPH_SRCM_MONO_CH1_FIFO:
            chalFIFO = CAPH_CH1_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH2_FIFO:
            chalFIFO = CAPH_CH2_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH3_FIFO:
            chalFIFO = CAPH_CH3_INFIFO;
            break;
        case CSL_CAPH_SRCM_MONO_CH4_FIFO:
            chalFIFO = CAPH_CH4_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH1_FIFO:
            chalFIFO = CAPH_PASSCH1_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_PASS_CH2_FIFO:
            chalFIFO = CAPH_PASSCH2_INFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_CH5_FIFO:
            chalFIFO = CAPH_CH5_INFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH1_FIFO:
            chalFIFO = CAPH_CH1_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH2_FIFO:
            chalFIFO = CAPH_CH2_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH3_FIFO:
            chalFIFO = CAPH_CH3_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_MONO_CH4_FIFO:
            chalFIFO = CAPH_CH4_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_TAP_STEREO_CH5_FIFO:
            chalFIFO = CAPH_CH5_TAP_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_CH1_FIFO:
            chalFIFO = CAPH_MIXER1_OUTFIFO;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_L_FIFO:
            chalFIFO = CAPH_MIXER2_OUTFIFO1;
            break;
        case CSL_CAPH_SRCM_STEREO_CH2_R_FIFO:
            chalFIFO = CAPH_MIXER2_OUTFIFO2;
            break;
        default:
            audio_xassert(0, chalFIFO);
    }
    return chalFIFO;
}
#endif

/****************************************************************************
*
*  Function Name:CAPH_DATA_FORMAT_e csl_caph_srcmixer_use_outchnl(
*                                         CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl)
*
*  Description: Check the output channel usage table to add input channel
*
****************************************************************************/
static void csl_caph_srcmixer_use_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl)
{
    UInt8 ch = 0;
    for (ch=0; ch<OUTCHNL_MAX_NUM_CHNL; ch++)
    {
        if (chnlTable[ch].outChnl == outChnl)
        {
            chnlTable[ch].inChnl |= inChnl;
        }
    }
    return;
}


/****************************************************************************
*
*  Function Name:CAPH_DATA_FORMAT_e csl_caph_srcmixer_unuse_outchnl(
*                                         CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl)
*
*  Description: Check the output channel usage table to remove input channel
*
****************************************************************************/
static void csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl,
                                          CSL_CAPH_SRCM_INCHNL_e inChnl)
{
    UInt8 ch = 0;
    CAPH_SRCMixer_CHNL_e chalInChnl = CAPH_SRCM_CH_NONE;
    UInt8 chalOutChnl = 0;

    for (ch=0; ch<OUTCHNL_MAX_NUM_CHNL; ch++)
    {
        if (chnlTable[ch].outChnl == outChnl)
        {
		if (chnlTable[ch].inChnl&inChnl)
		{
	            chnlTable[ch].inChnl &= ~inChnl;

	    	    chalInChnl = csl_caph_srcmixer_get_single_chal_inchnl(inChnl);
	    	    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(outChnl);

		    //Set mix in gain to 0x0.	
		    if (chalOutChnl&CAPH_M0_Left)
		    {
	            	chal_caph_srcmixer_set_mixingain(handle, 
				    chalInChnl, 
				    CAPH_M0_Left, 
				    MIX_IN_MUTE);
			    //Disable mixing.
			    chal_caph_srcmixer_disable_mixing(handle, 
				    chalInChnl, 
				    CAPH_M0_Left);
			    //Clear the mix in gain step.
			    chal_caph_srcmixer_set_mixingainstep(handle, 
				    chalInChnl,
				    CAPH_M0_Left, 
				    MIX_IN_NO_GAINSTEP);
		    }
		    if (chalOutChnl&CAPH_M0_Right)
		    {
		            chal_caph_srcmixer_set_mixingain(handle, 
				    chalInChnl, 
				    CAPH_M0_Right, 
				    MIX_IN_MUTE);
			    //Disable mixing.
			    chal_caph_srcmixer_disable_mixing(handle, 
				    chalInChnl, 
				    CAPH_M0_Right);		    
			    //Clear the mix in gain step.
			    chal_caph_srcmixer_set_mixingainstep(handle, 
				    chalInChnl,
				    CAPH_M0_Right, 
				    MIX_IN_NO_GAINSTEP);
		    }
		    if ((!(chalOutChnl&CAPH_M0_Left))&&(!(chalOutChnl&CAPH_M0_Right)))
		    {
	        	    chal_caph_srcmixer_set_mixingain(handle, 
				    chalInChnl, 
				    (CAPH_SRCMixer_OUTPUT_e)chalOutChnl, 
				    MIX_IN_MUTE);
			    //Disable mixing.
			    chal_caph_srcmixer_disable_mixing(handle, 
				    chalInChnl, 
				    (CAPH_SRCMixer_OUTPUT_e)chalOutChnl);		    
			    //Clear the mix in gain step.
			    chal_caph_srcmixer_set_mixingainstep(handle, 
				    chalInChnl,
				    (CAPH_SRCMixer_OUTPUT_e)chalOutChnl, 
				    MIX_IN_NO_GAINSTEP);
		    }
		}
        }
    }
    return;
}

/****************************************************************************
*
*  Function Name:UInt16 csl_caph_srcmixer_read_outchnltable(
*                                         CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
*
*  Description: Check the output channel usage table to read input channel
*
****************************************************************************/
static UInt16 csl_caph_srcmixer_read_outchnltable(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
{
    UInt8 ch = 0;
    UInt16 inChnls = 0;
    for (ch=0; ch<OUTCHNL_MAX_NUM_CHNL; ch++)
    {
        if (chnlTable[ch].outChnl == outChnl)
        {
            inChnls = chnlTable[ch].inChnl;
        }
    }
    return inChnls;
}

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_init(cUInt32 baseAddress)    
*
*  Description: init CAPH srcmixer block
*
****************************************************************************/
void csl_caph_srcmixer_init(UInt32 baseAddress)    
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_srcmixer_init:: \n"));

    /* Initialize SRCMixer cHAL */
    handle = chal_caph_srcmixer_init(baseAddress);
    /* Clear or TAP buffers */
    chal_caph_srcmixer_clr_all_tapbuffers(handle);
    /* Set all mixer input gain ramp */
    csl_caph_srcmixer_set_all_mixingainramp(handle, srcmixer_input_gain_ramp);
    /* Disable all mixer output gain compressors */
    csl_caph_srcmixer_disable_all_spkrgain_compressor(handle);
    /* Set all mixer output gain slope */
    csl_caph_srcmixer_set_all_spkrgain_slope(handle, srcmixer_output_gain_slope);
    /* ENable all mixer output gain slope */
    // disable this line for V3.7 fpga image until got detailed rdb info.
    //csl_caph_srcmixer_enable_all_spkrgain_slope(handle);

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_srcmixer_init:: baseAddress = 0x%lx\n", 
                    baseAddress));
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_deinit(void)    
*
*  Description: deinit CAPH srcmixer block
*
****************************************************************************/
void csl_caph_srcmixer_deinit(void)    
{
    UInt8 ch;
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_srcmixer_deinit:: \n"));

    chal_caph_srcmixer_deinit(handle);

    /* Go through all the channels and set them not allocated */
    for(ch=0; ch < MAX_INCHNLS; ch++)
    {
        inChnlStatus[ch].alloc_status = FALSE;
    }
    
	return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_SRCM_INCHNL_e csl_caph_srcmixer_obtain_inchnl
*                            (CSL_CAPH_DATAFOMAT_e dataFormat, 
*                             CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)
*
*  Description: obtain a CAPH srcmixer input channel
*
****************************************************************************/
CSL_CAPH_SRCM_INCHNL_e csl_caph_srcmixer_obtain_inchnl(CSL_CAPH_DATAFORMAT_e dataFormat, 
                              CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)    
{
    Int8 ch = 0;
    CSL_CAPH_SRCM_INCHNL_e neededChnl = CSL_CAPH_SRCM_INCHNL_NONE;

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_srcmixer_obtain_inchnl:: \n"));
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_srcmixer_obtain_inchnl:: dataFormat = 0x%x, sampleRate = 0x%x\n",
                    dataFormat, sampleRate));

    if (dataFormat == CSL_CAPH_16BIT_STEREO || (dataFormat == CSL_CAPH_24BIT_STEREO))
    {
        if (sampleRate == CSL_CAPH_SRCMIN_44_1KHZ)
        {
            neededChnl = CSL_CAPH_SRCM_STEREO_CH5;
        }
        else
        {
            neededChnl = CSL_CAPH_SRCM_STEREO_PASS_CH;
        }
    }
    else 
    {
        neededChnl = CSL_CAPH_SRCM_MONO_CH;
    }
    
    for(ch=MAX_INCHNLS-1;ch>=0;ch--)
    {
        if (inChnlStatus[ch].inChnl & neededChnl)
        {
            if (inChnlStatus[ch].alloc_status == FALSE)
            {
                /* input channel is available */
                inChnlStatus[ch].alloc_status = TRUE;
                return inChnlStatus[ch].inChnl;
            }
        }
    }
    /* No free channel available */
    return CSL_CAPH_SRCM_INCHNL_NONE;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_SRCM_MIX_OUTCHNL_e csl_caph_srcmixer_obtain_outchnl(CSL_CAPH_DEVICE_e sink)
*
*  Description: obtain a CAPH srcmixer output channel
*
****************************************************************************/
CSL_CAPH_SRCM_MIX_OUTCHNL_e csl_caph_srcmixer_obtain_outchnl(CSL_CAPH_DEVICE_e sink)
{
    CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE;
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
          "csl_caph_srcmixer_obtain_outchnl:: sink = 0x%x\n",
          sink));

    switch(sink)
    {
        case CSL_CAPH_DEV_EP:
            outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
            break;
        case CSL_CAPH_DEV_HS:
            outChnl = CSL_CAPH_SRCM_STEREO_CH1;
            break;
        case CSL_CAPH_DEV_IHF:
        case CSL_CAPH_DEV_VIBRA:
            outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
            break;
        case CSL_CAPH_DEV_FM_TX:
        case CSL_CAPH_DEV_BT_SPKR:
            //To be defined.
            ;
            break; 
        default:
            ;
    }
    return outChnl;
}


/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_release_inchnl(CSL_CAPH_SRCM_INCHNL_e chnl)
*
*  Description: release CAPH srcmixer input channel
*
****************************************************************************/
void csl_caph_srcmixer_release_inchnl(CSL_CAPH_SRCM_INCHNL_e chnl)
{
    UInt8 ch = 0;
    Boolean alreadyReleased = FALSE;
    CAPH_SRCMixer_CHNL_e chalInChnl = CAPH_SRCM_CH_NONE;
    CAPH_SRCMixer_FIFO_e fifo = CAPH_CH_INFIFO_NONE;

    if (chnl == CSL_CAPH_SRCM_INCHNL_NONE )
    {
	    //SRCMixer is not used. Do nothing.
	    return;
    }
    chalInChnl = csl_caph_srcmixer_get_single_chal_inchnl(chnl);
    
    for(ch=0;ch<MAX_INCHNLS;ch++)
    {
        if (inChnlStatus[ch].inChnl == chnl)
        {
            if (inChnlStatus[ch].alloc_status == TRUE)
            {
                /* input channel is found */
                if (alreadyReleased == FALSE)
                {
                    inChnlStatus[ch].alloc_status = FALSE;
                    alreadyReleased = TRUE;
                }
            }
	    //Disable the input channel.
	    chal_caph_srcmixer_disable_chnl(handle, (UInt16)chalInChnl);

	    /* Clear SRC setting */
	    chal_caph_srcmixer_set_SRC(handle, chalInChnl, CAPH_8KHz_48KHz);
	    /* Get Input Channel FIFO */
	    fifo = csl_caph_srcmixer_get_inchnl_fifo(chnl);
	    /* Clear input FIFO data format */
	    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, CAPH_MONO_16BIT);
	    /* Clear input FIFO threshold */
	    chal_caph_srcmixer_set_fifo_thres(handle, fifo, 
			    INFIFO_NO_THRES, INFIFO_NO_THRES);
	    
	    /* Clear Input FIFO */
	    chal_caph_srcmixer_clr_fifo(handle, fifo);
	    
	    
        }
    }
    /* Clear Mixer output channel usage table for this input channel*/
    csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_STEREO_CH1_L, chnl);
    csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_STEREO_CH1_R, chnl);
    csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_STEREO_CH1, chnl);
    csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_STEREO_CH2_L, chnl);
    csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_SRCM_STEREO_CH2_R, chnl);

	return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_release_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e chnl)
*
*  Description: release CAPH srcmixer output channel
*
****************************************************************************/
void csl_caph_srcmixer_release_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e chnl)
{
    UInt8 ch = 0;
    UInt8 chalOutChnl = 0x0;
    CAPH_SRCMixer_FIFO_e fifo = CAPH_CH_INFIFO_NONE;    

    if (chnl == CSL_CAPH_SRCM_CH_NONE )
    {
	    //SRCMixer is not used. Do nothing.
	    return;
    }
    /* get the cHAL output channel from CSL output channel */
    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(chnl);
    /* Get Output Channel FIFO */
    fifo = csl_caph_srcmixer_get_outchnl_fifo(chnl);
	
    for (ch=0; ch<OUTCHNL_MAX_NUM_CHNL; ch++)
    {
        if ((chnlTable[ch].outChnl == chnl)&&(chnlTable[ch].inChnl == 0x0))
        {
	        /* Clear output FIFO data format */
            chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, 
			    CAPH_MONO_16BIT);
            /* Clear output FIFO threshold */
            chal_caph_srcmixer_set_fifo_thres(handle, fifo, 
			    INFIFO_NO_THRES, INFIFO_NO_THRES);
            /* Clear output FIFO */
            chal_caph_srcmixer_clr_fifo(handle, fifo);
            /* Set gain to default value */
            if(chalOutChnl&CAPH_M0_Left)
            {
			    chal_caph_srcmixer_set_spkrgain_bitsel(handle, 
                        CAPH_M0_Left, 
                        (UInt8)MIX_OUT_COARSE_GAIN_DEFAULT);
			    chal_caph_srcmixer_set_spkrgain(handle, 
                        CAPH_M0_Left, 
                        (UInt16)MIX_OUT_FINE_GAIN_DEFAULT);	
            }
            if(chalOutChnl&CAPH_M0_Right)
            {
			    chal_caph_srcmixer_set_spkrgain_bitsel(handle, 
                        CAPH_M0_Right, 
                        (UInt8)MIX_OUT_COARSE_GAIN_DEFAULT);
			    chal_caph_srcmixer_set_spkrgain(handle, 
                        CAPH_M0_Right, 
                        (UInt16)MIX_OUT_FINE_GAIN_DEFAULT);	
            }
            if(chalOutChnl&CAPH_M1_Left)
            {
			    chal_caph_srcmixer_set_spkrgain_bitsel(handle, 
                        CAPH_M1_Left, 
                        (UInt8)MIX_OUT_COARSE_GAIN_DEFAULT);
			    chal_caph_srcmixer_set_spkrgain(handle, 
                        CAPH_M1_Left, 
                        (UInt16)MIX_OUT_FINE_GAIN_DEFAULT);	
            }
            if(chalOutChnl&CAPH_M1_Right)
            {
			    chal_caph_srcmixer_set_spkrgain_bitsel(handle, 
                        CAPH_M1_Right, 
                        (UInt8)MIX_OUT_COARSE_GAIN_DEFAULT);
			    chal_caph_srcmixer_set_spkrgain(handle, 
                        CAPH_M1_Right, 
                        (UInt16)MIX_OUT_FINE_GAIN_DEFAULT);	
            }
 
        }
    }
	return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_release_tapoutchnl(CSL_CAPH_SRCM_SRC_OUTCHNL_e chnl)
*
*  Description: release CAPH srcmixer tap output channel
*
****************************************************************************/
void csl_caph_srcmixer_release_tapoutchnl(CSL_CAPH_SRCM_SRC_OUTCHNL_e chnl)
{
    CAPH_SRCMixer_FIFO_e fifo = CAPH_CH_INFIFO_NONE;    


    /* Get Output Channel FIFO */
    fifo = csl_caph_srcmixer_get_tapoutchnl_fifo(chnl);
    /* Clear output FIFO data format */
    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, 
			    CAPH_MONO_16BIT);
    /* Clear output FIFO threshold */
    chal_caph_srcmixer_set_fifo_thres(handle, fifo, 
			    INFIFO_NO_THRES, INFIFO_NO_THRES);
    /* Clear output FIFO */
    chal_caph_srcmixer_clr_fifo(handle, fifo);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_release_inchnl(CSL_CAPH_SRCM_INCHNL_e chnl)
*
*  Description: release CAPH srcmixer input channel
*
****************************************************************************/
void csl_caph_srcmixer_set_inchnl_status(CSL_CAPH_SRCM_INCHNL_e chnl)
{
    UInt8 ch = 0;
    for(ch=0;ch<MAX_INCHNLS;ch++)
    {
        if (inChnlStatus[ch].inChnl == chnl)
        {
            if (inChnlStatus[ch].alloc_status == FALSE)
            {
			inChnlStatus[ch].alloc_status = TRUE;
			return;
            }
	      else
	      {
			// the channel dsp asked for is already in use.
			// running out of srcs
		       audio_xassert(0, chnl);
	      	}  	
        }
    }
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_config_mix_route(CSL_SRCMixer_ROUTE_t routeConfig)
*
*  Description: configure CAPH srcmixer Mixer route
*
****************************************************************************/
void csl_caph_srcmixer_config_mix_route(CSL_CAPH_SRCM_ROUTE_t routeConfig)
{
    UInt16 chalInChnlM = 0x0;
    CAPH_SRCMixer_CHNL_e chalInChnl = CAPH_SRCM_CH_NONE;
    CAPH_SRCMixer_SRC_e srcSampleRate = CAPH_8KHz_48KHz;
    CAPH_SRCMixer_FIFO_e fifo = CAPH_CH_INFIFO_NONE;
    CAPH_DATA_FORMAT_e dataFmt = CAPH_MONO_16BIT;
    UInt16 inChnls = 0x0;
    UInt8 chalOutChnl = 0x0;
    UInt8 ch = 0x0;
    UInt8 chnl = 0x0;

    if (routeConfig.inSampleRate == CSL_CAPH_SRCMIN_8KHZ )
    {
        /* 8KHz -> 48KHz */
        srcSampleRate = CAPH_8KHz_48KHz;
    }
    else
    if (routeConfig.inSampleRate == CSL_CAPH_SRCMIN_16KHZ )
    {
        /* 16KHz -> 48KHz */
        srcSampleRate = CAPH_16KHz_48KHz;
    }
    else
    if (routeConfig.inSampleRate == CSL_CAPH_SRCMIN_44_1KHZ )
    {
        /* 44.1KHz -> 48KHz */
        srcSampleRate = CAPH_44_1KHz_48KHz;
    }	
    else
    if (routeConfig.inSampleRate == CSL_CAPH_SRCMIN_48KHZ )		
    {
        /* 48KHz: 48->16, 48->8 handled in config_src_route, 48->48 pass thr here*/
        //return;
    }
    else
    {
        return;
    }

    /* Map CSL SRCM input channel to cHAL SRC Input channel */
    chalInChnl = csl_caph_srcmixer_get_single_chal_inchnl(routeConfig.inChnl);
    /* Disable the mixer input channel in case this channel is being used */
    chal_caph_srcmixer_disable_chnl(handle, (UInt16)chalInChnl);

    /* Configure SRC block */
    chal_caph_srcmixer_set_SRC(handle, chalInChnl, srcSampleRate);
    /* Get Input Channel FIFO */
    fifo = csl_caph_srcmixer_get_inchnl_fifo(routeConfig.inChnl);
    /* Get Input Data Format */
    dataFmt = csl_caph_srcmixer_get_chal_dataformat(handle, routeConfig.inDataFmt);
    /* Set input FIFO data format */
    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, dataFmt);
    /* Set input FIFO threshold */
    chal_caph_srcmixer_set_fifo_thres(handle, fifo, routeConfig.inThres,
                                      (UInt8)srcmixer_fifo_thres2);
    /* Clear Input FIFO */
    chal_caph_srcmixer_clr_fifo(handle, fifo);

    /* Get Output Channel FIFO */
    fifo = csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
    /* Get Output Data Format */
    dataFmt = csl_caph_srcmixer_get_chal_dataformat(handle, routeConfig.outDataFmt);
    /* Set output FIFO data format */
    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, dataFmt);
    /* Set output FIFO threshold */
    chal_caph_srcmixer_set_fifo_thres(handle, fifo, routeConfig.outThres,
                                      (UInt8)srcmixer_fifo_thres2);
    /* Clear output FIFO */
    chal_caph_srcmixer_clr_fifo(handle, fifo);
    /* Enable the mixer input channel */
    chal_caph_srcmixer_enable_chnl(handle,(UInt16)chalInChnl);
    /* Update the output channel usage table */
    csl_caph_srcmixer_use_outchnl(routeConfig.outChnl, routeConfig.inChnl);

    /* Check which input channel does not connect to the same Output channel:
     * chal_caph_srcmixer_set_mixingain() : mute this input channel outputing to this 
     * mixer output channel
     */
    /* get the cHAL output channel from CSL output channel */
    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(routeConfig.outChnl);
    /* read from the output usage table the input channels */
    inChnls = csl_caph_srcmixer_read_outchnltable(routeConfig.outChnl);
#if 0
    chalInChnl = csl_caph_srcmixer_get_chal_inchnl((CSL_CAPH_SRCM_INCHNL_e)inChnls);
#else
    chalInChnlM = csl_caph_srcmixer_get_chal_inchnl(inChnls);
#endif
    /* Mute those input channels which do not connect to this output channel */
    for (ch=MAX_SINGLE_INCHNLS; ch>0; ch--)
    {
        chnl = (inChnls>>(ch-1))&0x1;
        if (!chnl)
        {
			if(!(CAPH_SRCM_CH1<<(ch-1) & chalInChnlM))
			{
				if (chalOutChnl & (UInt8)CAPH_M0_Left)
				{
					chal_caph_srcmixer_set_mixingain(handle, CAPH_SRCM_CH1<<(ch-1), CAPH_M0_Left, MIX_IN_MUTE);
				}
				if (chalOutChnl & (UInt8)CAPH_M0_Right)
				{
					chal_caph_srcmixer_set_mixingain(handle, CAPH_SRCM_CH1<<(ch-1), CAPH_M0_Right, MIX_IN_MUTE);
				}
				if (chalOutChnl & (UInt8)CAPH_M1_Left)
				{
					chal_caph_srcmixer_set_mixingain(handle, CAPH_SRCM_CH1<<(ch-1), CAPH_M1_Left, MIX_IN_MUTE);
				}
				if (chalOutChnl & (UInt8)CAPH_M1_Right)
				{
					chal_caph_srcmixer_set_mixingain(handle, CAPH_SRCM_CH1<<(ch-1), CAPH_M1_Right, MIX_IN_MUTE);
				}
			}
        }
		else
		{
			if (chalOutChnl & (UInt8)CAPH_M0_Left)
			{
				chal_caph_srcmixer_set_mixingain(handle, chalInChnl, CAPH_M0_Left, routeConfig.mixGain.mixInGainL);
				chal_caph_srcmixer_enable_mixing(handle, chalInChnl, CAPH_M0_Left );
				chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M0_Left, routeConfig.mixGain.mixOutCoarseGainL);
				chal_caph_srcmixer_set_mixingainstep(handle, chalInChnl, CAPH_M0_Left, MIX_IN_GAINSTEP);
				chal_caph_srcmixer_set_spkrgain(handle, CAPH_M0_Left, routeConfig.mixGain.mixOutGainL);	
			}
			if (chalOutChnl & (UInt8)CAPH_M0_Right)
			{
				chal_caph_srcmixer_set_mixingain(handle, chalInChnl, CAPH_M0_Right, routeConfig.mixGain.mixInGainR);
				chal_caph_srcmixer_enable_mixing(handle, chalInChnl, CAPH_M0_Right );
				chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M0_Right, routeConfig.mixGain.mixOutCoarseGainR);
				chal_caph_srcmixer_set_mixingainstep(handle, chalInChnl, CAPH_M0_Right, MIX_IN_GAINSTEP);
				chal_caph_srcmixer_set_spkrgain(handle, CAPH_M0_Right, routeConfig.mixGain.mixOutGainR);	
			}	
			if (chalOutChnl & (UInt8)CAPH_M1_Left)
			{
				chal_caph_srcmixer_set_mixingain(handle, chalInChnl, CAPH_M1_Left, routeConfig.mixGain.mixInGainL);
				chal_caph_srcmixer_enable_mixing(handle, chalInChnl, CAPH_M1_Left );
				chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M1_Left, routeConfig.mixGain.mixOutCoarseGainL);
				chal_caph_srcmixer_set_mixingainstep(handle, chalInChnl, CAPH_M1_Left, MIX_IN_GAINSTEP);
				chal_caph_srcmixer_set_spkrgain(handle, CAPH_M1_Left, routeConfig.mixGain.mixOutGainL);	
			}	
			if (chalOutChnl & (UInt8)CAPH_M1_Right)
			{
				chal_caph_srcmixer_set_mixingain(handle, chalInChnl, CAPH_M1_Right, routeConfig.mixGain.mixInGainR);
				chal_caph_srcmixer_enable_mixing(handle, chalInChnl, CAPH_M1_Right );
				chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M1_Right, routeConfig.mixGain.mixOutCoarseGainR);
				chal_caph_srcmixer_set_mixingainstep(handle, chalInChnl, CAPH_M1_Right, MIX_IN_GAINSTEP);
				chal_caph_srcmixer_set_spkrgain(handle, CAPH_M1_Right, routeConfig.mixGain.mixOutGainR);	
			}	
		}
    }
	return;
}

 // for fpga test
 /****************************************************************************
*
*  Function Name: UInt8 csl_caph_srcmixer_read_fifo(CAPH_SRCMixer_FIFO_e fifo, cUInt32* data, cUInt8 size)
*
*  Description: read srcmixer fifo to data with size
*
****************************************************************************/
UInt8 csl_caph_srcmixer_read_fifo(CAPH_SRCMixer_FIFO_e fifo, cUInt32* data, cUInt8 size)
{
            return chal_caph_srcmixer_read_fifo(handle,fifo,data,size,FALSE);	
}

 /****************************************************************************
*
*  Function Name: UInt8 csl_caph_srcmixer_write_fifo(CAPH_SRCMixer_FIFO_e fifo, cUInt32* data, cUInt8 size)
*
*  Description: write srcmixer fifo from data with size
*
****************************************************************************/
UInt8 csl_caph_srcmixer_write_fifo(CAPH_SRCMixer_FIFO_e fifo, cUInt32* data, cUInt8 size)
{
            return chal_caph_srcmixer_write_fifo(handle,fifo,data,size,FALSE);
}
 
/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_config_src_route(CSL_SRCMixer_ROUTE_t routeConfig)
*
*  Description: configure CAPH srcmixer SRC route
*
****************************************************************************/
void csl_caph_srcmixer_config_src_route(CSL_CAPH_SRCM_ROUTE_t routeConfig)
{
    CAPH_SRCMixer_CHNL_e chalInChnl = CAPH_SRCM_CH_NONE;
    CAPH_SRCMixer_SRC_e srcSampleRate = CAPH_8KHz_48KHz;
    CAPH_SRCMixer_FIFO_e fifo = CAPH_CH_INFIFO_NONE;
    CAPH_DATA_FORMAT_e dataFmt = CAPH_MONO_16BIT;

    if ((routeConfig.inSampleRate == CSL_CAPH_SRCMIN_8KHZ)
        &(routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ))
    {
        /* 8KHz -> 48KHz */
        srcSampleRate = CAPH_8KHz_48KHz;
    }
    else
    if ((routeConfig.inSampleRate == CSL_CAPH_SRCMIN_16KHZ)
        &(routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ))
    {
        /* 16KHz -> 48KHz */
        srcSampleRate = CAPH_16KHz_48KHz;
    }
    else
    if ((routeConfig.inSampleRate == CSL_CAPH_SRCMIN_44_1KHZ)
        &(routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ))
    {
        /* 44.1KHz -> 48KHz */
        srcSampleRate = CAPH_44_1KHz_48KHz;
    }
    else
    if ((routeConfig.inSampleRate == CSL_CAPH_SRCMIN_48KHZ)
        &(routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_8KHZ))
    {
        /* 48KHz -> 8KHz */
        srcSampleRate = CAPH_48KHz_8KHz;
    }
    else
    if ((routeConfig.inSampleRate == CSL_CAPH_SRCMIN_48KHZ)
        &(routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_16KHZ))
    {
        /* 48KHz -> 16KHz */
        srcSampleRate = CAPH_48KHz_16KHz;
    }
    else
    {
        audio_xassert(0, ((routeConfig.inSampleRate)||((routeConfig.outSampleRate)<<16)));
    }


    /* Map CSL SRCM input channel to cHAL SRC Input channel */
    chalInChnl = csl_caph_srcmixer_get_single_chal_inchnl(routeConfig.inChnl);
    /* Disable the mixer input channel in case this channel is being used */
    chal_caph_srcmixer_disable_chnl(handle, (UInt16)chalInChnl);

    /* Configure SRC block */
    chal_caph_srcmixer_set_SRC(handle, chalInChnl, srcSampleRate);
    /* Get Input Channel FIFO */
    fifo = csl_caph_srcmixer_get_inchnl_fifo(routeConfig.inChnl);
    /* Get Input Data Format */
    dataFmt = csl_caph_srcmixer_get_chal_dataformat(handle, routeConfig.inDataFmt);
    /* Set input FIFO data format */
    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, dataFmt);
    /* Set input FIFO threshold */
    chal_caph_srcmixer_set_fifo_thres(handle, fifo, routeConfig.inThres,
                                      (UInt8)srcmixer_fifo_thres2);
    /* Clear Input FIFO */
    chal_caph_srcmixer_clr_fifo(handle, fifo);

    /* Get SRC Output Channel FIFO */
    if (chalInChnl == CAPH_SRCM_CH1)
    	fifo = CAPH_CH1_TAP_OUTFIFO;//csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
    else if (chalInChnl == CAPH_SRCM_CH2)
    	fifo = CAPH_CH2_TAP_OUTFIFO;//csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
    else if (chalInChnl == CAPH_SRCM_CH3)
    	fifo = CAPH_CH3_TAP_OUTFIFO;//csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
    else if (chalInChnl == CAPH_SRCM_CH4)
    	fifo = CAPH_CH4_TAP_OUTFIFO;//csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
     else 
	     audio_xassert(0, chalInChnl );	

    /* Get Output Data Format */
    dataFmt = csl_caph_srcmixer_get_chal_dataformat(handle, routeConfig.outDataFmt);
    /* Set output FIFO data format */
    chal_caph_srcmixer_set_fifo_datafmt(handle, fifo, dataFmt);
    /* Set output FIFO threshold */
    chal_caph_srcmixer_set_fifo_thres(handle, fifo, routeConfig.outThres,
                                      (UInt8)srcmixer_fifo_thres2);
    /* Clear output FIFO */
    chal_caph_srcmixer_clr_fifo(handle, fifo);
    /* Enable the mixer input channel */
    chal_caph_srcmixer_enable_chnl(handle,(UInt16)chalInChnl);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_INCHNL_e inChnl, 
*                                    CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
*                                    UInt16 gainL, UInt16 gainR)    
*                                              
*  Description: Set the mixer input gain
*
****************************************************************************/
void csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_INCHNL_e inChnl, 
                                    CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                                    UInt16 gainL, UInt16 gainR)    
{
    CAPH_SRCMixer_CHNL_e chalInChnl = CAPH_SRCM_CH_NONE;
    UInt8 chalOutChnl = 0x0;

    /* Map CSL SRCM input channel to cHAL SRC Input channel */
    chalInChnl = csl_caph_srcmixer_get_single_chal_inchnl(inChnl);
    /* get the cHAL output channel from CSL output channel */
    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(outChnl);

    /* Set the mixer input gain accordingly */
    switch (chalInChnl)
    {
        case CAPH_SRCM_CH_NONE:
            return;
        case CAPH_SRCM_CH1:
        case CAPH_SRCM_CH2:
        case CAPH_SRCM_CH3:
        case CAPH_SRCM_CH4:
        case CAPH_SRCM_PASSCH1_L:
        case CAPH_SRCM_PASSCH1_R:
        case CAPH_SRCM_PASSCH2_L:
        case CAPH_SRCM_PASSCH2_R:
    		if (chalOutChnl&CAPH_M0_Left)
         	   chal_caph_srcmixer_set_mixingain(handle, 
				   		chalInChnl, 
						CAPH_M0_Left, 
						gainL);
    		if (chalOutChnl&CAPH_M0_Right)
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		chalInChnl, 
						CAPH_M0_Right, 
						gainL);
    		if (chalOutChnl&CAPH_M1_Left)
         	   chal_caph_srcmixer_set_mixingain(handle, 
				   		chalInChnl, 
						CAPH_M1_Left, 
						gainL);
    		if (chalOutChnl&CAPH_M1_Right)
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		chalInChnl, 
						CAPH_M1_Right, 
						gainL);
            break;
        case CAPH_SRCM_PASSCH1:
    		if (chalOutChnl&CAPH_M0_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_L,
					        CAPH_M0_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_R,
					        CAPH_M0_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M0_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_L,
					        CAPH_M0_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_R,
					        CAPH_M0_Right, 
						gainR);
		}
    		if (chalOutChnl&CAPH_M1_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_L,
					        CAPH_M1_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_R,
					        CAPH_M1_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M1_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_L,
					        CAPH_M1_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH1_R,
					        CAPH_M1_Right, 
						gainR);
		}
            break;
        case CAPH_SRCM_PASSCH2:
    		if (chalOutChnl&CAPH_M0_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_L,
					        CAPH_M0_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_R,
					        CAPH_M0_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M0_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_L,
					        CAPH_M0_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_R,
					        CAPH_M0_Right, 
						gainR);
		}
    		if (chalOutChnl&CAPH_M1_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_L,
					        CAPH_M1_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_R,
					        CAPH_M1_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M1_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_L,
					        CAPH_M1_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_PASSCH2_R,
					        CAPH_M1_Right, 
						gainR);
		}
		break;
	    
        case CAPH_SRCM_CH5:
    		if (chalOutChnl&CAPH_M0_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_L,
					        CAPH_M0_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_R,
					        CAPH_M0_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M0_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_L,
					        CAPH_M0_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_R,
					        CAPH_M0_Right, 
						gainR);
		}
    		if (chalOutChnl&CAPH_M1_Left)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_L,
					        CAPH_M1_Left, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_R,
					        CAPH_M1_Left, 
						gainR);
		}
		if (chalOutChnl&CAPH_M1_Right)
		{
	            chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_L,
					        CAPH_M1_Right, 
						gainL);
        	    chal_caph_srcmixer_set_mixingain(handle, 
				    		CAPH_SRCM_CH5_R,
					        CAPH_M1_Right, 
						gainR);
		}
	    
            break;
        default:
            audio_xassert(0, chalInChnl);
    }
	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_srcmixer_set_mixoutgain(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
*                                     UInt16 gainL, UInt16 gainR)    
*
*  Description: Set the SRCMixer mixer output gain
*
****************************************************************************/
void csl_caph_srcmixer_set_mixoutgain(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                                     UInt16 gain)
{
    UInt8 chalOutChnl = 0x0;

    /* get the cHAL output channel from CSL output channel */
    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(outChnl);
    /* Set the mixer left/right channel output gain */    
    if (chalOutChnl&CAPH_M0_Left)
    chal_caph_srcmixer_set_spkrgain(handle, CAPH_M0_Left, gain);
    if (chalOutChnl&CAPH_M0_Right)
    chal_caph_srcmixer_set_spkrgain(handle, CAPH_M0_Right, gain);
    if (chalOutChnl&CAPH_M1_Left)
    chal_caph_srcmixer_set_spkrgain(handle, CAPH_M1_Left, gain);
    if (chalOutChnl&CAPH_M1_Right)
    chal_caph_srcmixer_set_spkrgain(handle, CAPH_M1_Right, gain);

	return;
}


/****************************************************************************
*
*  Function Name:void csl_caph_srcmixer_set_mixoutcoarsegain(
*  				CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
*                                     UInt16 gainL, UInt16 gainR)    
*
*  Description: Set the SRCMixer mixer output coarse gain
*
****************************************************************************/
void csl_caph_srcmixer_set_mixoutcoarsegain(
		CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                UInt16 gain)
{
    UInt8 chalOutChnl = 0x0;

    /* get the cHAL output channel from CSL output channel */
    chalOutChnl = csl_caph_srcmixer_get_chaloutchnl(outChnl);
    /* Set the mixer left/right channel output gain */    
    if (chalOutChnl&CAPH_M0_Left)
    chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M0_Left, gain);
    if (chalOutChnl&CAPH_M0_Right)
    chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M0_Right, gain);
    if (chalOutChnl&CAPH_M1_Left)
    chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M1_Left, gain);
    if (chalOutChnl&CAPH_M1_Right)
    chal_caph_srcmixer_set_spkrgain_bitsel(handle, CAPH_M1_Right, gain);

	return;
}
/****************************************************************************
*
*  Function Name:UInt32 csl_caph_srcmixer_get_fifo_addr(CSL_CAPH_SRCM_FIFO_e fifo)
*
*  Description: Get the SRCMixer FIFO buffer address
*
****************************************************************************/
UInt32 csl_caph_srcmixer_get_fifo_addr(CAPH_SRCMixer_FIFO_e fifo)
{
    //CAPH_SRCMixer_FIFO_e chalFIFO = CAPH_CH_INFIFO_NONE;
    UInt32 FIFOAddr = 0;

    /* Get the chal FIFO */
    //chalFIFO = csl_caph_srcmixer_get_chal_fifo(fifo);
    /* Get the fifo address */
    FIFOAddr = chal_caph_srcmixer_get_fifo_addr(handle, fifo);

	return FIFOAddr;
}

/****************************************************************************
*
*  Function Name:CSL_CAPH_SRCM_INSAMPLERATE_e 
*          csl_caph_srcmixer_get_srcm_insamplerate(AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Get the SRCMixer input sample rate
*
****************************************************************************/
CSL_CAPH_SRCM_INSAMPLERATE_e csl_caph_srcmixer_get_srcm_insamplerate(AUDIO_SAMPLING_RATE_t sampleRate)
{
    CSL_CAPH_SRCM_INSAMPLERATE_e srcm_sampleRate= CSL_CAPH_SRCMIN_48KHZ;

    switch (sampleRate)
    {
        case AUDIO_SAMPLING_RATE_8000:
			srcm_sampleRate= CSL_CAPH_SRCMIN_8KHZ;
			break;
        case AUDIO_SAMPLING_RATE_16000:
			srcm_sampleRate= CSL_CAPH_SRCMIN_16KHZ;
			break;
        case AUDIO_SAMPLING_RATE_44100:
			srcm_sampleRate= CSL_CAPH_SRCMIN_44_1KHZ;
			break;
        case AUDIO_SAMPLING_RATE_48000:
			srcm_sampleRate= CSL_CAPH_SRCMIN_48KHZ;
			break; 			
        default:
            audio_xassert(0, sampleRate);
    }
	return srcm_sampleRate;
}

/****************************************************************************
*
*  Function Name:CSL_CAPH_SRCM_OUTSAMPLERATE_e 
*          csl_caph_srcmixer_get_srcm_outsamplerate(AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Get the SRCMixer output sample rate
*
****************************************************************************/
CSL_CAPH_SRCM_OUTSAMPLERATE_e csl_caph_srcmixer_get_srcm_outsamplerate(AUDIO_SAMPLING_RATE_t sampleRate)
{
    CSL_CAPH_SRCM_OUTSAMPLERATE_e srcm_sampleRate= CSL_CAPH_SRCMOUT_48KHZ;

    switch (sampleRate)
    {
        case AUDIO_SAMPLING_RATE_8000:
			srcm_sampleRate= CSL_CAPH_SRCMOUT_8KHZ;
			break;
        case AUDIO_SAMPLING_RATE_16000:
			srcm_sampleRate= CSL_CAPH_SRCMOUT_16KHZ;
			break;
        case AUDIO_SAMPLING_RATE_48000:
			srcm_sampleRate= CSL_CAPH_SRCMOUT_48KHZ;
			break; 			
        default:
            audio_xassert(0, sampleRate);
    }
	return srcm_sampleRate;
}

/****************************************************************************
*
*  Function Name: csl_caph_srcmixer_get_tapoutchnl_from_inchnl
*
*  Description: Get the tap output based on its SRC input
*
****************************************************************************/
CSL_CAPH_SRCM_SRC_OUTCHNL_e csl_caph_srcmixer_get_tapoutchnl_from_inchnl(CSL_CAPH_SRCM_INCHNL_e inChnl)
{
	CSL_CAPH_SRCM_SRC_OUTCHNL_e outChnl = CSL_CAPH_SRCM_TAP_CH_NONE;

	switch (inChnl)
	{
	case CSL_CAPH_SRCM_MONO_CH1:
		outChnl = CSL_CAPH_SRCM_TAP_MONO_CH1;
		break;
	case CSL_CAPH_SRCM_MONO_CH2:
		outChnl = CSL_CAPH_SRCM_TAP_MONO_CH2;
		break;
	case CSL_CAPH_SRCM_MONO_CH3:
		outChnl = CSL_CAPH_SRCM_TAP_MONO_CH3;
		break;
	case CSL_CAPH_SRCM_MONO_CH4:
		outChnl = CSL_CAPH_SRCM_TAP_MONO_CH4;
		break;
	case CSL_CAPH_SRCM_STEREO_CH5:
		outChnl = CSL_CAPH_SRCM_TAP_STEREO_CH5;
		break;
	default:
		audio_xassert(0, inChnl);
		break;
	}
	return outChnl;
}
