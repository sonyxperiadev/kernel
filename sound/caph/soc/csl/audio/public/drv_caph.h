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
*   @file   drv_caph.h
*
*   @brief  This file contains the const for caph Driver layer
*
****************************************************************************/

#ifndef _DRV_CAPH_
#define _DRV_CAPH_
#include "audio_consts.h"
#include "csl_caph.h"
#include "io_map.h"

/**
* Globale Constants
******************************************************************************/
#define AUDIOH_BASE_ADDR1            KONA_AUDIOH_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SDT_BASE_ADDR1            KONA_SDT_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SRCMIXER_BASE_ADDR1          KONA_SRCMIXER_BASE_VA /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR1             KONA_CFIFO_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR1            KONA_AADMAC_BASE_VA /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR1             KONA_SSASW_BASE_VA /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR1            KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */	
#define SSP4_BASE_ADDR1            KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR1            KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */

/**
* CAPH Path ID
******************************************************************************/
typedef UInt8 AUDDRV_PathID;

/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    AUDDRV_STREAM_e streamID;
    AUDDRV_PathID pathID;
    AUDDRV_DEVICE_e source;
    AUDDRV_DEVICE_e sink;
    CSL_CAPH_DMA_CHNL_e dmaCH;    
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;        
}AUDDRV_HWCTRL_CONFIG_t;
/**
* CAPH HW filters
******************************************************************************/
typedef enum
{
    AUDDRV_EANC_FILTER1, 
    AUDDRV_EANC_FILTER2, 
    AUDDRV_SIDETONE_FILTER, 
}AUDDRV_HWCTRL_FILTER_e;


/**
* CAPH Audio Stream Buffer done Callback function
******************************************************************************/
typedef Boolean (*AUDDRV_BufDoneCB_t)(UInt8  *pBuf,  UInt32 nSize,
                                      AUDDRV_STREAM_e streamID);

/**
* CAPH Audio Information for Render/Capture Driver
******************************************************************************/
typedef struct
{
    CSL_CAPH_CFIFO_FIFO_e fifo;
}AUDDRV_HWCTRL_INFO_t;

#endif // _DRV_CAPH_
