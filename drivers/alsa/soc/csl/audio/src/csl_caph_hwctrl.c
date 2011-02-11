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
*  @file   csl_caph_hwctrl.c
*
*  @brief  csl layer driver for caph render
*
****************************************************************************/
#include "resultcode.h"
#include "mobcom_types.h"
#include "auddrv_def.h"
#include "xassert.h"
#include "log.h"
//#include "chal_bmodem_intc_inc.h"
#include "chal_audio.h"
#include "brcm_rdb_audioh.h"
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_i2s_sspi.h"
#include "csl_caph_pcm_sspi.h"
#ifdef UNDER_LINUX
#include <mach/io_map.h>
//#include <mach/hardware.h>
#endif


//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************
extern CSL_CAPH_HWConfig_DMA_t HWConfig_DMA_Table[CSL_CAPH_DEV_MAXNUM][CSL_CAPH_DEV_MAXNUM]; 
extern CHAL_HANDLE lp_handle;

//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// local macro declarations
//****************************************************************************
#define MAX_AUDIO_PATH  32
#define PATH_OCCUPIED   1
#define PATH_AVAILABLE  0

//****************************************************************************
// local typedef declarations
//****************************************************************************
typedef struct
{
    CSL_CAPH_PathID pathID;
    CSL_CAPH_STREAM_e streamID;
    CSL_CAPH_DEVICE_e source;
    CSL_CAPH_DEVICE_e sink;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    CSL_CAPH_CFIFO_FIFO_e fifo;
    CSL_CAPH_CFIFO_FIFO_e fifo2;
    CSL_CAPH_SWITCH_CHNL_e switchCH;
    CSL_CAPH_SWITCH_CHNL_e switchCH2;
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_DMA_CHNL_e dmaCH2;
    CSL_CAPH_SRCM_ROUTE_t routeConfig;	
    UInt8* pBuf;
    UInt8* pBuf2;
    UInt32 size;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;
    Boolean status;
}CSL_CAPH_HWConfig_Table_t;

//****************************************************************************
// local variable definitions
//****************************************************************************
static CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];
static CSL_HANDLE handleSSP3 = 0;
static CSL_HANDLE handleSSP4 = 0;
static Boolean ssp3Running = FALSE;
static Boolean ssp4Running = FALSE;
static void *bmintc_handle = NULL;
//****************************************************************************
// local function declarations
//****************************************************************************
static CSL_CAPH_PathID csl_caph_hwctrl_AddPath(CSL_CAPH_DEVICE_e source, 
                                               CSL_CAPH_DEVICE_e sink,
                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,                                               
                                               AUDIO_CHANNEL_NUM_t chnlNum,
                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample);
static void csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_SetPathFifo(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_CFIFO_FIFO_e fifo);
static void csl_caph_hwctrl_SetPathSwitchCH(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SWITCH_CHNL_e switchCH);
static void csl_caph_hwctrl_SetPathDMACH(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DMA_CHNL_e dmaCH);
static void csl_caph_hwctrl_SetPathFifo2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_CFIFO_FIFO_e fifo);
static void csl_caph_hwctrl_SetPathSwitchCH2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SWITCH_CHNL_e switchCH);
static void csl_caph_hwctrl_SetPathDMACH2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DMA_CHNL_e dmaCH);
static void csl_caph_hwctrl_SetPathSinkSampleRate(CSL_CAPH_PathID pathID, 
                                         AUDIO_SAMPLING_RATE_t sampleRate);
static void csl_caph_hwctrl_SetPathSrcSampleRate(CSL_CAPH_PathID pathID, 
                                         AUDIO_SAMPLING_RATE_t sampleRate);
static CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromPathID(CSL_CAPH_PathID pathID);
static CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromStreamID
                                                  (CSL_CAPH_STREAM_e streamI);
static CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate
                                           (AUDIO_SAMPLING_RATE_t sampleRate);
static CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(CSL_CAPH_DEVICE_e source,
                                                        CSL_CAPH_DEVICE_e sink);

//******************************************************************************
// local function definitions
//******************************************************************************
/****************************************************************************
*
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_AddPath(CSL_CAPH_DEVICE_e source, 
*                                               CSL_CAPH_DEVICE_e sink,
*                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
*                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,
*                                               AUDIO_CHANNEL_NUM_t chnlNum,
*                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample)
*
*  Description: Add the new path into the path table
*
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_AddPath(CSL_CAPH_DEVICE_e source, 
                                               CSL_CAPH_DEVICE_e sink,
                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,                            
                                               AUDIO_CHANNEL_NUM_t chnlNum,
                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if ((HWConfig_Table[i].source == CSL_CAPH_DEV_NONE)
           &&(HWConfig_Table[i].sink == CSL_CAPH_DEV_NONE))
        {
            HWConfig_Table[i].pathID = (CSL_CAPH_PathID)(i + 1);
            HWConfig_Table[i].source = source;
            HWConfig_Table[i].sink = sink;
            HWConfig_Table[i].src_sampleRate = src_sampleRate;
            HWConfig_Table[i].snk_sampleRate = snk_sampleRate;			
            HWConfig_Table[i].chnlNum = chnlNum;
            HWConfig_Table[i].bitPerSample = bitPerSample;
            return (CSL_CAPH_PathID)(i + 1);
        }
    }
    return (CSL_CAPH_PathID)0;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID)
*switchCH
*  Description: Remove a path from the path table
*
****************************************************************************/
static void csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].pathID = (CSL_CAPH_PathID)0;
            HWConfig_Table[i].streamID = CSL_CAPH_STREAM_NONE;
            HWConfig_Table[i].source = CSL_CAPH_DEV_NONE;
            HWConfig_Table[i].sink = CSL_CAPH_DEV_NONE;
            HWConfig_Table[i].src_sampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;
            HWConfig_Table[i].snk_sampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;			
            HWConfig_Table[i].chnlNum = AUDIO_CHANNEL_NUM_NONE;
            HWConfig_Table[i].bitPerSample = AUDIO_8_BIT_PER_SAMPLE;
            HWConfig_Table[i].fifo = CSL_CAPH_CFIFO_NONE;
            HWConfig_Table[i].fifo2 = CSL_CAPH_CFIFO_NONE;
            HWConfig_Table[i].switchCH = CSL_CAPH_SWITCH_NONE;
            HWConfig_Table[i].switchCH2 = CSL_CAPH_SWITCH_NONE;
            HWConfig_Table[i].dmaCH = CSL_CAPH_DMA_NONE;
            HWConfig_Table[i].dmaCH2 = CSL_CAPH_DMA_NONE;
            HWConfig_Table[i].pBuf = NULL;
            HWConfig_Table[i].pBuf2 = NULL;
            HWConfig_Table[i].size = 0;
            HWConfig_Table[i].dmaCB = NULL;
            HWConfig_Table[i].status = PATH_AVAILABLE;
	     memset(&HWConfig_Table[i].routeConfig, 0, sizeof(CSL_CAPH_SRCM_ROUTE_t));
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathFifo(CSL_CAPH_PathID pathID,
*                                                CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: Set the FIFO for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathFifo(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_CFIFO_FIFO_e fifo)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].fifo = fifo;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathFifo2(CSL_CAPH_PathID pathID,
*                                                CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: Set the second FIFO for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathFifo2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_CFIFO_FIFO_e fifo)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].fifo2 = fifo;
            return;
        }
    }
    xassert(0, pathID);
    return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathSwitchCH(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SWITCH_CHNL_e switchCH)
*
*  Description: Set the Switch CH for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathSwitchCH(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SWITCH_CHNL_e switchCH)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].switchCH = switchCH;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathSwitchCH2(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SWITCH_CHNL_e switchCH)
*
*  Description: Set the second Switch CH for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathSwitchCH2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SWITCH_CHNL_e switchCH)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].switchCH2 = switchCH;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathRouteConfig(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the route config for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathRouteConfig(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_ROUTE_t routeConfig)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].routeConfig = routeConfig;
            return;
        }
    }
    xassert(0, pathID);
    return;
}
/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathDMACH(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_DMA_CHNL_e dmaCH)
*
*  Description: Set the DMA CH for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathDMACH(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DMA_CHNL_e dmaCH)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].dmaCH = dmaCH;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathDMACH2(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_DMA_CHNL_e dmaCH)
*
*  Description: Set the second DMA CH for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathDMACH2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DMA_CHNL_e dmaCH)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].dmaCH2 = dmaCH;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathSinkSampleRate(CSL_CAPH_PathID pathID,
*                                           AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Set the sink sample rate for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathSinkSampleRate(CSL_CAPH_PathID pathID, 
                                         AUDIO_SAMPLING_RATE_t sampleRate)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].snk_sampleRate = sampleRate;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathSrcSampleRate(CSL_CAPH_PathID pathID,
*                                           AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Set the source sample rate for a path in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathSrcSampleRate(CSL_CAPH_PathID pathID, 
                                         AUDIO_SAMPLING_RATE_t sampleRate)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].src_sampleRate = sampleRate;
            return;
        }
    }
    xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromPathID
*                                              (CSL_CAPH_PathID pathID)
*
*  Description: Get the source/sink of a path from the path table
*
****************************************************************************/
static CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromPathID(CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    CSL_CAPH_HWConfig_Table_t path;

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            path.pathID = HWConfig_Table[i].pathID;
            path.streamID = HWConfig_Table[i].streamID;
            path.source = HWConfig_Table[i].source;
            path.sink = HWConfig_Table[i].sink;
            path.src_sampleRate = HWConfig_Table[i].src_sampleRate;
            path.snk_sampleRate = HWConfig_Table[i].snk_sampleRate;			
            path.chnlNum = HWConfig_Table[i].chnlNum;
            path.bitPerSample = HWConfig_Table[i].bitPerSample;
            path.fifo = HWConfig_Table[i].fifo;
            path.fifo2 = HWConfig_Table[i].fifo2;
            path.switchCH = HWConfig_Table[i].switchCH;
            path.switchCH2 = HWConfig_Table[i].switchCH2;
            path.dmaCH = HWConfig_Table[i].dmaCH;
            path.dmaCH2 = HWConfig_Table[i].dmaCH2;
	        path.routeConfig = HWConfig_Table[i].routeConfig;		
            path.pBuf = HWConfig_Table[i].pBuf;
            path.pBuf2 = HWConfig_Table[i].pBuf2;
            path.size = HWConfig_Table[i].size;
            path.dmaCB = HWConfig_Table[i].dmaCB;
            path.status = HWConfig_Table[i].status;
            return path;
        }
    }
    xassert(0, pathID);
    return path;
}


/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromStreamID
*                                             (CSL_CAPH_STREAM_e streamID)
*
*  Description: Get the source/sink of a path from the path table
*
****************************************************************************/
static CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromStreamID(CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;
    CSL_CAPH_HWConfig_Table_t path;

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == streamID)
        {
            path.pathID = HWConfig_Table[i].pathID;
            path.streamID = HWConfig_Table[i].streamID;
            path.source = HWConfig_Table[i].source;
            path.sink = HWConfig_Table[i].sink;
            path.src_sampleRate = HWConfig_Table[i].src_sampleRate;
            path.snk_sampleRate = HWConfig_Table[i].snk_sampleRate;			
            path.chnlNum = HWConfig_Table[i].chnlNum;
            path.bitPerSample = HWConfig_Table[i].bitPerSample;
            path.fifo = HWConfig_Table[i].fifo;
            path.fifo2 = HWConfig_Table[i].fifo2;
            path.switchCH = HWConfig_Table[i].switchCH;
            path.switchCH2 = HWConfig_Table[i].switchCH2;
            path.dmaCH = HWConfig_Table[i].dmaCH;
            path.dmaCH2 = HWConfig_Table[i].dmaCH2;
	        path.routeConfig = HWConfig_Table[i].routeConfig;
            path.pBuf = HWConfig_Table[i].pBuf;
            path.pBuf2 = HWConfig_Table[i].pBuf2;
            path.size = HWConfig_Table[i].size;
            path.dmaCB = HWConfig_Table[i].dmaCB;
            path.status = HWConfig_Table[i].status;
            return path;
        }
    }
    xassert(0, streamID);
    return path;
}



/****************************************************************************
*
*  Function Name:  CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate
*                                            (AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Get the CSL sample rate
*
****************************************************************************/
static CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate(AUDIO_SAMPLING_RATE_t sampleRate)
{
    CSL_CAPH_CFIFO_SAMPLERATE_e cslSampleRate = CSL_CAPH_SRCM_UNDEFINED;

    switch (sampleRate)
    {
        case AUDIO_SAMPLING_RATE_8000:
            cslSampleRate = CSL_CAPH_SRCM_8KHZ;
            break;
        case AUDIO_SAMPLING_RATE_16000:
            cslSampleRate = CSL_CAPH_SRCM_16KHZ;
            break;
        case AUDIO_SAMPLING_RATE_48000:
            cslSampleRate = CSL_CAPH_SRCM_48KHZ;
            break;
        default:
            break;
    }
    return cslSampleRate;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(
*                                                 CSL_CAPH_DEVICE_e source
*                                                 CSL_CAPH_DEVICE_e sink)
*
*  Description: Get the DMA channel configuration from a pre-defined look-up table.
*
****************************************************************************/
static CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(CSL_CAPH_DEVICE_e source,
                                                        CSL_CAPH_DEVICE_e sink)
{
   	Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_hwctrl_getDMACH::Source=0x%x, Sink=0x%x\n",
                    source, sink);
    return HWConfig_DMA_Table[source][sink];
}
/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_init(CSL_CAPH_HWCTRL_BASE_ADDR_t addr)
*
*  Description: init CAPH HW control driver
*
****************************************************************************/
void csl_caph_hwctrl_init(CSL_CAPH_HWCTRL_BASE_ADDR_t addr)
{
   	//Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_init:: \n");
	printk("csl_caph_hwctrl_init:: \n");
    memset(HWConfig_Table, 0, sizeof(HWConfig_Table));

#if 1
    //Mapping 7 to 17.
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3A050260))) = (UInt32) (0x00000080) );

#else
    bmintc_handle = (void *)chal_bmintc_init(BINTC_BASE_ADDR);

    chal_bmintc_enable_interrupt(bmintc_handle, 
		    BINTC_OUT_DEST_DSP_NORM, 
		    0x7);
#endif
    // csl_caph_switch_init() should be run as the first init function
    // It will enable clock in SSASW_NOC register. It is needed by
    // CFIFO and SSASW.
    csl_caph_switch_init(addr.ssasw_baseAddr);
    csl_caph_cfifo_init(addr.cfifo_baseAddr);
    csl_caph_dma_init(addr.aadmac_baseAddr, addr.ahintc_baseAddr);
    csl_caph_srcmixer_init(addr.srcmixer_baseAddr);
    csl_caph_audioh_init(addr.audioh_baseAddr, addr.sdt_baseAddr);

    // SSP3 is configured as I2S for FM Radio.
    handleSSP3 = (CSL_HANDLE)csl_i2s_init(addr.ssp3_baseAddr);
    // SSP4 is configured as PCM for Bluetooth.        
    handleSSP4 = (CSL_HANDLE)csl_pcm_init(addr.ssp4_baseAddr);

#if 0    
    // For test purpose:
    // ADCPATH_GLOBAL_CTRL
    ( *((volatile UInt32 *) (0x35020060)) = (UInt32) (0x00000001) );

    // DAC_CTL
    ( *((volatile UInt32 *) (0x35020000)) = (UInt32) (0x00000010) );

    // ADC_CTL
    ( *((volatile UInt32 *) (0x35020004)) = (UInt32) (0x00000010) );

    // AUDIO_INTC
    ( *((volatile UInt32 *) (0x35020104)) = (UInt32) (0x00000010) );

    // VOUT_FIFO_CTRL: clear FIFO
    ( *((volatile UInt32 *) (0x35020028)) = (UInt32) (0x00000080) );

    // VOUT_FIFO_CTRL
    ( *((volatile UInt32 *) (0x35020028)) = (UInt32) (0x00000002) );

    //  VIN_FIFO_CTRL: clear FIFO
    ( *((volatile UInt32 *) (0x35020034)) = (UInt32) (0x00000080) );

    //  VIN_FIFO_CTRL
    ( *((volatile UInt32 *) (0x35020034)) = (UInt32) (0x00004002) );

    //  ADCPATH_GLOBAL_CTRL
    ( *((volatile UInt32 *) (0x35020060)) = (UInt32) (0x00000001) );

    //  EP_PWR
    ( *((volatile UInt32 *) (0x35020108)) = (UInt32) (0x00000000) );
 
    //  EP_DRV
    ( *((volatile UInt32 *) (0x3502010C)) = (UInt32) (0x00000000) );

 

//d.s SD:0x3500E0D4 %long 0x000000C0                    ; ACI_ADC_CTRL

//d.s SD:0x3500E014 %long 0x00000001                     ; AUXMIC_AUXEN

//d.s SD:0x3500E028 %long 0x00000000                     ; AUXMIC_F_PWRDWN

    //  AUDIORX_BIAS
    ( *((volatile UInt32 *) (0x35020210)) = (UInt32) (0x00000001) );

    //  AUDIORX_VREF
    ( *((volatile UInt32 *) (0x35020208)) = (UInt32) (0x00000003) );

    //  AUDIORX_VRX1
    ( *((volatile UInt32 *) (0x35020200)) = (UInt32) (0x00000000) );
 

    //  AUDIORX_VMIC
    ( *((volatile UInt32 *) (0x3502020C)) = (UInt32) (0x00000302) );

    //  AUDIORX_VREF
    ( *((volatile UInt32 *) (0x35020208)) = (UInt32) (0x00000002) );

    //  AUDIORX_VRX2
    ( *((volatile UInt32 *) (0x35020204)) = (UInt32) (0x00000000) );

    //  AUDIORX_CTRL
    ( *((volatile UInt32 *) (0x35020214)) = (UInt32) (0x00000000) );





    //  CH01_SRC
    ( *((volatile UInt32 *) (0x3502F010)) = (UInt32) (0x8016CC20) );

 
    //  CH01_DST1
    ( *((volatile UInt32 *) (0x3502F014)) = (UInt32) (0x80005800) );

    //  CH02_SRC
    ( *((volatile UInt32 *) (0x3502F030)) = (UInt32) (0x80116800) );
 
    //  CH02_DST1
    ( *((volatile UInt32 *) (0x3502F034)) = (UInt32) (0x8000C910) );

    //  CH03_SRC
    ( *((volatile UInt32 *) (0x3502F050)) = (UInt32) (0x805DC990) );

    //  CH03_DST1
    ( *((volatile UInt32 *) (0x3502F054)) = (UInt32) (0x8000C900) );
    
  


    //  SRC_CHANNEL1_CTRL1
    ( *((volatile UInt32 *) (0x3502C000)) = (UInt32) (0x00002504) );

 
    //  SRC_CHANNEL2_CTRL1
    ( *((volatile UInt32 *) (0x3502C008)) = (UInt32) (0x00042506) );

    //  SRC_CHANNEL2_CTRL2
    ( *((volatile UInt32 *) (0x3502C00C)) = (UInt32) (0x00036300) );
 
    //  SRC_M2D0_CH1M_GAIN_CTRL
    ( *((volatile UInt32 *) (0x3502C160)) = (UInt32) (0xFFFF3FFF) );

    


    //  SRC_SPK0_LT_GAIN_CTRL1
    ( *((volatile UInt32 *) (0x3502C200)) = (UInt32) (0x04000000) );

 
    //  SRC_SPK0_RT_GAIN_CTRL1
    ( *((volatile UInt32 *) (0x3502C220)) = (UInt32) (0x04000000) );

    //  SRC_SPK1_LT_GAIN_CTRL1
    ( *((volatile UInt32 *) (0x3502C240)) = (UInt32) (0x04000000) );
 
    //  SRC_SPK1_RT_GAIN_CTRL1
    ( *((volatile UInt32 *) (0x3502C260)) = (UInt32) (0x04000000) );




    //  MIXER2_OUTFIFO1_CTRL
    ( *((volatile UInt32 *) (0x3502CC24)) = (UInt32) (0x00016300) );
 
    //  MIXER2_OUTFIFO2_CTRL
    ( *((volatile UInt32 *) (0x3502CC34)) = (UInt32) (0x00807300) );

#endif
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C260))) = (UInt32) (0x04000000) );
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x35020200))) = (UInt32) (0x003A0000) );
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DeInit(void)
*
*  Description: De-Initialize CAPH HW Control driver
*
****************************************************************************/
void csl_caph_hwctrl_deinit(void)
{
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_deinit:: \n");
    memset(HWConfig_Table, 0, sizeof(HWConfig_Table));
    csl_caph_cfifo_deinit();
    csl_caph_dma_deinit();
    csl_caph_switch_deinit();
    csl_caph_srcmixer_deinit();
    csl_caph_audioh_deinit();
    
    csl_pcm_deinit(handleSSP3);
    csl_i2s_deinit(handleSSP4);
    
	return;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config) 
*
*  Description: Enable a HW path
*
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_SWITCH_CONFIG_t sw_config;
    CSL_I2S_CONFIG_t ssp3_config;
    csl_pcm_config_device_t ssp4_dev;
    csl_pcm_config_tx_t ssp4_configTx; 
    csl_pcm_config_rx_t ssp4_configRx;
    audio_config_t audioh_config;
    CSL_CAPH_HWConfig_Table_t audioPath;
    CSL_CAPH_DMA_CONFIG_t dmaConfig;
    CSL_CAPH_PathID pathID;
    CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_ch, csl_caph_switch_ch2;
    CSL_CAPH_CFIFO_FIFO_e fifo, fifo2;
    CSL_CAPH_CFIFO_DIRECTION_e direction;
    CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate; 
    CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;  
    CAPH_SRCMixer_FIFO_e chal_src_fifo;
	CSL_CAPH_DMA_CHNL_t dmaCHs;    
    CSL_CAPH_HWConfig_DMA_t dmaCHSetting;
    CSL_CAPH_DATAFORMAT_e csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
    CSL_CAPH_SRCM_INSAMPLERATE_e csl_caph_srcm_insamplerate =CSL_CAPH_SRCMIN_48KHZ;
    CSL_CAPH_SRCM_OUTSAMPLERATE_e csl_caph_srcm_outsamplerate =CSL_CAPH_SRCMOUT_48KHZ;	
    AUDDRV_PATH_Enum_t	audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;	
    UInt16 threshold = 0;
	
    pathID = 0;
    csl_caph_switch_ch = CSL_CAPH_SWITCH_NONE;
    csl_caph_switch_ch2 = CSL_CAPH_SWITCH_NONE;
    fifo = CSL_CAPH_CFIFO_NONE;
    fifo2 = CSL_CAPH_CFIFO_NONE;
    direction = CSL_CAPH_CFIFO_OUT;
    sampleRate = CSL_CAPH_SRCM_UNDEFINED; 
    memset(&sw_config, 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));
    memset(&ssp3_config, 0, sizeof(CSL_I2S_CONFIG_t));
    memset(&ssp4_dev, 0, sizeof(csl_pcm_config_device_t));
    memset(&ssp4_configTx, 0, sizeof(csl_pcm_config_tx_t));
    memset(&ssp4_configRx, 0, sizeof(csl_pcm_config_rx_t));
    memset(&audioh_config, 0, sizeof(audio_config_t));
    memset(&audioPath, 0, sizeof(CSL_CAPH_HWConfig_Table_t));
	memset(&dmaConfig, 0, sizeof(CSL_CAPH_DMA_CONFIG_t));
	memset(&audiohBufAddr, 0, sizeof(CSL_CAPH_AUDIOH_BUFADDR_t));
    memset(&dmaCHs, 0, sizeof(CSL_CAPH_DMA_CHNL_t));
    memset(&dmaCHSetting, 0, sizeof(CSL_CAPH_HWConfig_DMA_t));

    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnablePath::  Source: %d, Sink: %d\r\n",
            config.source, config.sink);

    // To check whether the control request comes from audio controller or audio router, 
    // by checking whether streamID is defined. If not, it comes from audio controller.
    // Then at first save the control information. 
    // Then check the source and/or sink to find out if audio router is involved. If yes,
    // just return and do nothing. And it will wait for audio router's control request
    // afterwards. If no, it is audio controller who trys to control HW, then go ahead to
    // do the HW control.
    if (config.streamID == CSL_CAPH_STREAM_NONE)
    {
        pathID = csl_caph_hwctrl_AddPath(config.source, 
                                         config.sink,
                                         config.src_sampleRate,
                                         config.snk_sampleRate,
                                         config.chnlNum,
                                         config.bitPerSample);

	config.pathID = pathID;
        // If the source or sink is not the DDR memory, Audio Router is not involved.
        // In this case, Audio Controller should be able to directly control the Audio
        // HW later. Therefore return from here.
        if ((config.source == CSL_CAPH_DEV_MEMORY)||(config.sink == CSL_CAPH_DEV_MEMORY))
        {
            return pathID;
        }

        dmaCHSetting = csl_caph_hwctrl_getDMACH(config.source, config.sink);
        if (dmaCHSetting.dmaNum == 2)
        {
            if (dmaCHSetting.dmaCH == 0)
            {
           	    dmaCHs.dmaCH = csl_caph_dma_obtain_channel();            
                if (dmaCHs.dmaCH == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH. 
                    // Clean up the table.
                    // Return zero pathID.
                    csl_caph_hwctrl_RemovePath(pathID);
                    return 0;
                }
	            xassert(dmaCHs.dmaCH<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH);            
            }
            else
            {
       	    	dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(dmaCHSetting.dmaCH);
            }

            if (dmaCHSetting.dmaCH2 == 0)
            {
                dmaCHs.dmaCH2 = csl_caph_dma_obtain_channel();
                if (dmaCHs.dmaCH2 == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH2. Release dmaCH.
                    // Clean up the table.
                    // Return zero pathID
                    csl_caph_dma_release_channel(dmaCHs.dmaCH);
                    csl_caph_hwctrl_RemovePath(pathID);
                    return 0;
                }
                xassert(dmaCHs.dmaCH2<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH2);
            }
            else
            {
           		dmaCHs.dmaCH2 = csl_caph_dma_obtain_given_channel(dmaCHSetting.dmaCH2);
            }
        }
        else
        if (dmaCHSetting.dmaNum == 1)
        {
            if (dmaCHSetting.dmaCH == 0)
            {
           	    dmaCHs.dmaCH = csl_caph_dma_obtain_channel();            
                if (dmaCHs.dmaCH == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH. 
                    // Clean up the table.
                    // Return zero pathID.
                    csl_caph_hwctrl_RemovePath(pathID);
                    return 0;
                }
	            xassert(dmaCHs.dmaCH<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH);            
            }
            else
            {
           		dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(dmaCHSetting.dmaCH);
            }
        }
    }

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Audio Router will control the Audio HW.
        audioPath = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if (config.pathID != 0)        
    {
        config.dmaCH = dmaCHs.dmaCH;
        config.dmaCH2 = dmaCHs.dmaCH2;

        // Audio Controller directly controls the Audio
        // HW.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(pathID);
    }

    if (((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_EP))||
	  ((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_HS))||
	  ((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_IHF))||
	  ((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_VIBRA)))
    {
        if (audioPath.sink == CSL_CAPH_DEV_HS)
            audioh_path = AUDDRV_PATH_HEADSET_OUTPUT;
        else if (audioPath.sink == CSL_CAPH_DEV_IHF)
            audioh_path = AUDDRV_PATH_IHF_OUTPUT;
        else  if (audioPath.sink == CSL_CAPH_DEV_EP)
            audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;
        else  if (audioPath.sink == CSL_CAPH_DEV_VIBRA)
            audioh_path = AUDDRV_PATH_VIBRA_OUTPUT;	
        else
            xassert(0, audioPath.sink );
	
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
                csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
            else //all non mono is treated as stereo for now.
                csl_caph_dataformat = CSL_CAPH_16BIT_STEREO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
        	    csl_caph_dataformat = CSL_CAPH_24BIT_MONO;
            else //all non mono is treated as stereo for now.
                 csl_caph_dataformat = CSL_CAPH_24BIT_STEREO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
		
        // config cfifo	based on data format and sampling rate
        sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
        fifo = csl_caph_cfifo_obtain_fifo(csl_caph_dataformat, sampleRate); 
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);
        direction = CSL_CAPH_CFIFO_IN;
        threshold = csl_caph_cfifo_get_fifo_thres(fifo);
        csl_caph_cfifo_config_fifo(fifo, direction, threshold);

        // config switch
        csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

        sw_config.chnl = csl_caph_switch_ch;
        sw_config.FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);


        //In case that the sink sample Rate is 0, force it to be 48KHz.
        if(audioPath.snk_sampleRate == 0)
        {
            audioPath.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
            csl_caph_hwctrl_SetPathSinkSampleRate(audioPath.pathID, audioPath.snk_sampleRate);
        }

        // caph srcmixer will be used
        if (audioPath.src_sampleRate != audioPath.snk_sampleRate)
        {
            csl_caph_srcm_insamplerate = csl_caph_srcmixer_get_srcm_insamplerate(audioPath.src_sampleRate);
            csl_caph_srcm_outsamplerate = csl_caph_srcmixer_get_srcm_outsamplerate(audioPath.snk_sampleRate);
            
            // 2. get SRC-Mixer in channel
            audioPath.routeConfig.inChnl = csl_caph_srcmixer_obtain_inchnl(csl_caph_dataformat, csl_caph_srcm_insamplerate);
            // 3. get SRC-Mixer in channel fifo
            chal_src_fifo = csl_caph_srcmixer_get_inchnl_fifo(audioPath.routeConfig.inChnl);
            // 4. connect SRC-Mixer in channel fifo to SW
            sw_config.FIFO_dstAddr = csl_caph_srcmixer_get_fifo_addr(chal_src_fifo);
            // 5. set the data format and sr from user
            audioPath.routeConfig.inDataFmt = csl_caph_dataformat;
            audioPath.routeConfig.inSampleRate = csl_caph_srcm_insamplerate;
            audioPath.routeConfig.inThres = 0x3; // set to default
            // need to put some logic here to decide which outChnl(SRC or Mixer) to use
            
            if (audioPath.sink == CSL_CAPH_DEV_EP)
                audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
            else if (audioPath.sink == CSL_CAPH_DEV_HS)
                audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
            else if (audioPath.sink == CSL_CAPH_DEV_IHF)
                audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;	
            
            if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
                audioPath.routeConfig.outDataFmt = CSL_CAPH_16BIT_MONO;
            else 
                audioPath.routeConfig.outDataFmt = CSL_CAPH_24BIT_MONO;
            
            audioPath.routeConfig.outSampleRate = csl_caph_srcm_outsamplerate;
            audioPath.routeConfig.outThres = 0x3; // set to default
            
            // save the route config to path table
            csl_caph_hwctrl_SetPathRouteConfig(audioPath.pathID, audioPath.routeConfig);
            
            if ((audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_8KHZ) ||
            	(audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_16KHZ))
            {
                csl_caph_srcmixer_config_src_route(audioPath.routeConfig);
            }
            else if (audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ)
            {
                csl_caph_srcmixer_config_mix_route(audioPath.routeConfig);
                // : set the mix in gain and gain step, moved in config for fpga test
                //csl_caph_srcmixer_set_mixingain(audioPath.routeConfig.inChnl, audioPath.routeConfig.outChnl,  0x3fff, 0x3fff); 
                // set the mix out gain, moved in config for fpga test
                //csl_caph_srcmixer_set_mixoutgain(audioPath.routeConfig.outChnl, 0x1fff);			
            }
            
            // set the SW trigger and dataformat
            sw_config.trigger = csl_caph_srcmixer_get_inchnl_trigger(audioPath.routeConfig.inChnl);
            sw_config.dataFmt = csl_caph_dataformat;
            // finally config SW   
            csl_caph_switch_config_channel(sw_config);
            	
            // config switch ch2 if needed
            sw_config.chnl = csl_caph_switch_obtain_channel();
            
            // Save the switch channel2 information
            csl_caph_hwctrl_SetPathSwitchCH2(audioPath.pathID, sw_config.chnl);            
            
            chal_src_fifo = csl_caph_srcmixer_get_outchnl_fifo(audioPath.routeConfig.outChnl);
            sw_config.FIFO_srcAddr = csl_caph_srcmixer_get_fifo_addr(chal_src_fifo);
            
            audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
            sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr;
            
            // set the trigger based on the sink
            if (audioh_path == AUDDRV_PATH_EARPICEC_OUTPUT)
               	sw_config.trigger = CSL_CAPH_TRIG_EP_THR_MET;
            else if (audioh_path == AUDDRV_PATH_HEADSET_OUTPUT)
               	sw_config.trigger = CSL_CAPH_TRIG_HS_THR_MET;
            else if (audioh_path == AUDDRV_PATH_IHF_OUTPUT)
               	sw_config.trigger = CSL_CAPH_TRIG_IHF_THR_MET;
            else if (audioh_path == AUDDRV_PATH_VIBRA_OUTPUT)
               	sw_config.trigger = CSL_CAPH_TRIG_VB_THR_MET;	
            
            sw_config.dataFmt = csl_caph_dataformat;
            csl_caph_switch_config_channel(sw_config);
        }
        else
        {
            audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
            sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr;
            // set the trigger based on the sink
            if (audioh_path == AUDDRV_PATH_EARPICEC_OUTPUT)
            	sw_config.trigger = CSL_CAPH_TRIG_EP_THR_MET;
            else if (audioh_path == AUDDRV_PATH_HEADSET_OUTPUT)
            	sw_config.trigger = CSL_CAPH_TRIG_HS_THR_MET;
            else if (audioh_path == AUDDRV_PATH_IHF_OUTPUT)
            	sw_config.trigger = CSL_CAPH_TRIG_IHF_THR_MET;
            else if (audioh_path == AUDDRV_PATH_VIBRA_OUTPUT)
            	sw_config.trigger = CSL_CAPH_TRIG_VB_THR_MET;	
            
            if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
            {
                sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
            }
            else
            if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
            {
            		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
            }
            else
            {
                xassert(0, audioPath.bitPerSample );
            }
            csl_caph_switch_config_channel(sw_config);
        }

        // config audioh
        audioh_config.sample_size = audioPath.bitPerSample;
        audioh_config.sample_pack = 1;
        audioh_config.sample_mode = audioPath.chnlNum;
        csl_caph_audioh_config(audioh_path, (void *)&audioh_config);
        
        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);
        
        //hard coded to in direction.
        dmaConfig.direction = CSL_CAPH_DMA_IN;
        dmaConfig.dma_ch = audioPath.dmaCH;
        dmaConfig.fifo = fifo;
        dmaConfig.mem_addr = audioPath.pBuf;
        dmaConfig.mem_size = audioPath.size;
        dmaConfig.Tsize = CSL_AADMAC_TSIZE;
        dmaConfig.dmaCB = audioPath.dmaCB;
        csl_caph_dma_config_channel(dmaConfig);
        csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);
        
        
        // caph blocks start
        csl_caph_cfifo_start_fifo(audioPath.fifo);
        //csl_caph_dma_start_transfer(audioPath.dmaCH);
        csl_caph_switch_start_transfer(audioPath.switchCH);
        
        // check if switchCH2 is used by this path
        if (audioPath.switchCH2 != CSL_CAPH_SWITCH_NONE)
            csl_caph_switch_start_transfer(audioPath.switchCH2);
        
        csl_caph_audioh_start(audioh_path);
        csl_caph_dma_start_transfer(audioPath.dmaCH);

	//For testing purpose
#if 0
	( *((volatile UInt32 *) (0x3502C100)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C104)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C108)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C10C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C110)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C114)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C118)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C11C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C120)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C124)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C130)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C134)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C138)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C13C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C140)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C144)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C148)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C14C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C150)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C154)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C160)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C164)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C168)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C16C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C170)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C174)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C178)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C17C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C180)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C184)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C190)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C194)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C198)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C19C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A0)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A4)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A8)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1AC)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1B0)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1B4)) = (UInt32) (0x00000000) );

	// Play 1KHz SineWave signal --> PT_CH2 (SRCMixer)--> EP
	// Passthrough CH2 --> Mixer2_Outfifo1

	// Cut the audio routes from CFIFO->EP
	( *((volatile UInt32 *) (0x3502F010)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502F014)) = (UInt32) (0x00000000) );
	
#if 0
	( *((volatile UInt32 *) (0x3502F030)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502F034)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502F050)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502F054)) = (UInt32) (0x00000000) );

	// Turn off MONO_CH1 and MONO_CH2 in SRCMixer
	( *((volatile UInt32 *) (0x3502C000)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C008)) = (UInt32) (0x00000000) );

#endif

	// CFIFO D000 -> SRCMixer PT_CH2 CB10
	( *((volatile UInt32 *) (0x3502F010)) = (UInt32) (0x8051D000) );
	( *((volatile UInt32 *) (0x3502F014)) = (UInt32) (0x8000CB10) );

	// Config PT_CH2.  24bit Mono.
	( *((volatile UInt32 *) (0x3502C044)) = (UInt32) (0x00000034) );

	// Config SRC_M2D0_CH7L_GAIN_CTRL (0x3502C180)
	( *((volatile UInt32 *) (0x3502C180)) = (UInt32) (0xFFFF0FFF) );
	// Config SRC_M2D0_CH7R_GAIN_CTRL (0x3502C184)
	( *((volatile UInt32 *) (0x3502C184)) = (UInt32) (0xFFFF0FFF) );

	// SRCMixer MIXER2_OUTFIFO1 -> EP 0x5800
	( *((volatile UInt32 *) (0x3502F090)) = (UInt32) (0x8016CC20) );
	( *((volatile UInt32 *) (0x3502F094)) = (UInt32) (0x80005800) );
	
#endif
    }
    else
    if ((audioPath.source == CSL_CAPH_DEV_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY)
        ||(audioPath.source == CSL_CAPH_DEV_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_DSP)
		||(audioPath.source == CSL_CAPH_DEV_DIGI_MIC_L)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY) 
		||(audioPath.source == CSL_CAPH_DEV_DIGI_MIC_R)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY) 
        ||(audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY)
        ||(audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_DSP))
    {
		if(audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
		{
			/* Set up the path for primary digital mic
			 */
			//In case that the source sample Rate is 0, force it to be 48KHz.
			if(audioPath.src_sampleRate == 0)
			{
				audioPath.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
				csl_caph_hwctrl_SetPathSrcSampleRate(audioPath.pathID, audioPath.src_sampleRate);
			}

   			// config cfifo
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else
						fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else  
       			   fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
			// Save the fifo information
			csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    		direction = CSL_CAPH_CFIFO_OUT;
			threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   			csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    		// config switch
			csl_caph_switch_ch = csl_caph_switch_obtain_channel();
			// Save the switch channel information
			csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   			sw_config.chnl = csl_caph_switch_ch;
			if (audioPath.source == CSL_CAPH_DEV_DIGI_MIC)
			{
				audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_VIN_INPUT);
			}
			else
			if (audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
			{
				audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_NVIN_INPUT);
			}
			sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr;
    		sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
   			sw_config.chnl = csl_caph_switch_ch;

			if (audioPath.source == CSL_CAPH_DEV_DIGI_MIC)
			{
   				sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
			}
			else
			if (audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
			{
   				sw_config.trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOR_THR_MET;
			}
    
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
    			sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
       			sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
    		csl_caph_switch_config_channel(sw_config);
			

			// config DMA
			csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
			// Get all the information from Table again.
			audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

		   //hard coded to in direction.
    		dmaConfig.direction = CSL_CAPH_DMA_OUT;
			dmaConfig.dma_ch = audioPath.dmaCH;
    		dmaConfig.fifo = fifo;
			dmaConfig.mem_addr = audioPath.pBuf;
    		dmaConfig.mem_size = audioPath.size;
			dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    		dmaConfig.dmaCB = audioPath.dmaCB;
			csl_caph_dma_config_channel(dmaConfig);
			csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);


			/* Set up the path for secondary digital mic
			 */

   			// config cfifo
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				fifo2 = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       			fifo2 = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
			// Save the fifo information
			csl_caph_hwctrl_SetPathFifo2(audioPath.pathID, fifo2);

    		direction = CSL_CAPH_CFIFO_OUT;
			threshold = csl_caph_cfifo_get_fifo_thres(fifo2);
   			csl_caph_cfifo_config_fifo(fifo2, direction, threshold);

    		// config switch
			csl_caph_switch_ch2 = csl_caph_switch_obtain_channel();
			// Save the switch channel information
			csl_caph_hwctrl_SetPathSwitchCH2(audioPath.pathID, csl_caph_switch_ch2);            

   			sw_config.chnl = csl_caph_switch_ch2;
			sw_config.FIFO_srcAddr = audiohBufAddr.buf2Addr;
    		sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo2);

			if (audioPath.source == CSL_CAPH_DEV_DIGI_MIC)
			{
   				sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET;
			}
			else
			if (audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
			{
   				sw_config.trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOL_THR_MET;
			}

			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
    			sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
       			sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
    		csl_caph_switch_config_channel(sw_config);
			

			// config DMA
			csl_caph_hwctrl_SetPathDMACH2(audioPath.pathID, config.dmaCH2);
			// Get all the information from Table again.
			audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

			//hard coded to in direction.
    		dmaConfig.direction = CSL_CAPH_DMA_OUT;
			dmaConfig.dma_ch = audioPath.dmaCH2;
    		dmaConfig.fifo = fifo2;
			dmaConfig.mem_addr = audioPath.pBuf2;
    		dmaConfig.mem_size = audioPath.size;
			dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    		dmaConfig.dmaCB = audioPath.dmaCB;
			csl_caph_dma_config_channel(dmaConfig);
			csl_caph_dma_enable_intr(audioPath.dmaCH2, CSL_CAPH_ARM);

			// config audioh
   			audioh_config.sample_size = audioPath.bitPerSample;
    		audioh_config.sample_pack = 0;
			audioh_config.sample_mode = AUDIO_CHANNEL_STEREO;
			audioh_config.mic_id = CHAL_AUDIO_CHANNEL_LEFT|CHAL_AUDIO_CHANNEL_RIGHT;

			if (audioPath.source == CSL_CAPH_DEV_DIGI_MIC)
			{
   				csl_caph_audioh_config(AUDDRV_PATH_VIN_INPUT, (void *)&audioh_config);
			}
			else
			if (audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
			{
   				csl_caph_audioh_config(AUDDRV_PATH_NVIN_INPUT, (void *)&audioh_config);
			}

			// caph blocks start
			csl_caph_cfifo_start_fifo(audioPath.fifo);
			csl_caph_cfifo_start_fifo(audioPath.fifo2);
			csl_caph_dma_start_transfer(audioPath.dmaCH);
			csl_caph_dma_start_transfer(audioPath.dmaCH2);
			csl_caph_switch_start_transfer(audioPath.switchCH);
			csl_caph_switch_start_transfer(audioPath.switchCH2);
			if (audioPath.source == CSL_CAPH_DEV_DIGI_MIC)
			{
				csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT);        
			}
			else
			if (audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
			{
				csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT);        
			}
			
		}

        /*  Digital mic 1
         */

		if(audioPath.source == CSL_CAPH_DEV_DIGI_MIC_L)
		{
			//In case that the source sample Rate is 0, force it to be 48KHz.
			if(audioPath.src_sampleRate == 0)
			{
				audioPath.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
				csl_caph_hwctrl_SetPathSrcSampleRate(audioPath.pathID, audioPath.src_sampleRate);
			}

   			// config cfifo
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else
						fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else  
       			   fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
			// Save the fifo information
			csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    		direction = CSL_CAPH_CFIFO_OUT;
			threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   			csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    		// config switch
			csl_caph_switch_ch = csl_caph_switch_obtain_channel();
			// Save the switch channel information
			csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   			sw_config.chnl = csl_caph_switch_ch;
			audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_VIN_INPUT);
			sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr;
    		sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
   			sw_config.chnl = csl_caph_switch_ch;
  			sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
    
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
    			sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
       			sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
    		csl_caph_switch_config_channel(sw_config);
			

			// config DMA
			csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
			// Get all the information from Table again.
			audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

		   //hard coded to in direction.
    		dmaConfig.direction = CSL_CAPH_DMA_OUT;
			dmaConfig.dma_ch = audioPath.dmaCH;
    		dmaConfig.fifo = fifo;
			dmaConfig.mem_addr = audioPath.pBuf;
    		dmaConfig.mem_size = audioPath.size;
			dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    		dmaConfig.dmaCB = audioPath.dmaCB;
			csl_caph_dma_config_channel(dmaConfig);
			csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

		}

        /* Digital mic 2
         */

		if(audioPath.source == CSL_CAPH_DEV_DIGI_MIC_R)
		{

			//In case that the source sample Rate is 0, force it to be 48KHz.
			if(audioPath.src_sampleRate == 0)
			{
				audioPath.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
				csl_caph_hwctrl_SetPathSrcSampleRate(audioPath.pathID, audioPath.src_sampleRate);
			}

   			// config cfifo
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else
						fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
				sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
				if (audioPath.sink == CSL_CAPH_DEV_DSP)
						fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
				else  
       			   fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
			// Save the fifo information
			csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

			direction = CSL_CAPH_CFIFO_OUT;
			threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   			csl_caph_cfifo_config_fifo(fifo, direction, threshold);

			// config switch
			csl_caph_switch_ch = csl_caph_switch_obtain_channel();
			// Save the switch channel information
			csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   			sw_config.chnl = csl_caph_switch_ch;
			audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_VIN_INPUT);
			sw_config.FIFO_srcAddr = audiohBufAddr.buf2Addr;
			sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
   			sw_config.chnl = csl_caph_switch_ch;
  			sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET; 
			if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
    			sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
			}
			else
			if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
			{
       			sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
			}
			else
			{
				xassert(0, audioPath.bitPerSample );
			}
			csl_caph_switch_config_channel(sw_config);
			

			// config DMA
			csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
			// Get all the information from Table again.
			audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

		   //hard coded to in direction.
			dmaConfig.direction = CSL_CAPH_DMA_OUT;
			dmaConfig.dma_ch = audioPath.dmaCH;
			dmaConfig.fifo = fifo;
			dmaConfig.mem_addr = audioPath.pBuf;
			dmaConfig.mem_size = audioPath.size;
			dmaConfig.Tsize = CSL_AADMAC_TSIZE;
			dmaConfig.dmaCB = audioPath.dmaCB;
			csl_caph_dma_config_channel(dmaConfig);
			csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

		}


		if((audioPath.source == CSL_CAPH_DEV_DIGI_MIC_L) || 
		   (audioPath.source == CSL_CAPH_DEV_DIGI_MIC_R) ||
		   (audioPath.source == CSL_CAPH_DEV_DIGI_MIC))
		{

			// config audioh
   			audioh_config.sample_size = audioPath.bitPerSample;
    		audioh_config.sample_pack = 0;
			audioh_config.sample_mode = AUDIO_CHANNEL_STEREO;
			audioh_config.mic_id = CHAL_AUDIO_CHANNEL_LEFT|CHAL_AUDIO_CHANNEL_RIGHT;

 			csl_caph_audioh_config(AUDDRV_PATH_VIN_INPUT, (void *)&audioh_config);

			// caph blocks start

			csl_caph_cfifo_start_fifo(audioPath.fifo);
			csl_caph_dma_start_transfer(audioPath.dmaCH);
			csl_caph_switch_start_transfer(audioPath.switchCH);

			csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT); 

//			( *((volatile UInt32 *) (0x35020004)) = (UInt32) (0x00002103) );
			
		}  
    }   
    else
    if((audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_EP)
        ||(audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_HS)
        ||(audioPath.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(audioPath.sink == CSL_CAPH_DEV_IHF))
    {

        /* Set up the path for primary digital mic
         */

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_EANC_INPUT);
	    sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr;
    	sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
   		sw_config.chnl = csl_caph_switch_ch;
   		sw_config.trigger = CSL_CAPH_TRIG_EANC_FIFO_THRESMET;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);


        /* Set up the path for NVIN mics
         */

        // config audioh
   		audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
	    audioh_config.sample_mode = AUDIO_CHANNEL_STEREO;
        audioh_config.mic_id = CHAL_AUDIO_CHANNEL_LEFT|CHAL_AUDIO_CHANNEL_RIGHT;
   	    csl_caph_audioh_config(AUDDRV_PATH_NVIN_INPUT, (void *)&audioh_config);

        memset(&audioh_config, 0, sizeof(audio_config_t));
   		audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
        audioh_config.eanc_input =(AUDDRV_MIC_Enum_t)0x2; /*Digi Mic3 is for EANC antinoise */
        if (audioPath.sink == CSL_CAPH_DEV_EP)
        {
            audioh_config.eanc_output = AUDDRV_PATH_EARPICEC_OUTPUT;
        }
        else
        if (audioPath.sink == CSL_CAPH_DEV_IHF)
        {
            audioh_config.eanc_output = AUDDRV_PATH_IHF_OUTPUT;
        }
        else //audioPath.sink == CSL_CAPH_DEV_HS
        {
            audioh_config.eanc_output = AUDDRV_PATH_HEADSET_OUTPUT;
        }
   	    csl_caph_audioh_config(AUDDRV_PATH_EANC_INPUT, (void *)&audioh_config);
        
	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
    	csl_caph_switch_start_transfer(audioPath.switchCH);
        csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_start(AUDDRV_PATH_EANC_INPUT); 
    }       
    else
    if ((audioPath.source == CSL_CAPH_DEV_ANALOG_MIC)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY))
    {

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate); 
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
	    sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr; 
    	sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
   		sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config audioh
   		audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
	    audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
   		csl_caph_audioh_config(AUDDRV_PATH_ANALOGMIC_INPUT, (void *)&audioh_config);

	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    } 
    else
    if ((audioPath.source == CSL_CAPH_DEV_I2S_RX)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY))
    {

        /* Set up the path for FM Radio playback: SSP3->CFIFO->DDR
         */

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.FIFO_srcAddr = csl_i2s_get_rx0_fifo_data_port(handleSSP3);
        
    	sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	    sw_config.trigger = CSL_CAPH_TRIG_SSP3_RX0;

        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);
        // config ssp3
        if (ssp3Running == FALSE)
        {
            ssp3_config.mode = CSL_I2S_MASTER_MODE;
            ssp3_config.tx_ena = 1;
            ssp3_config.rx_ena = 1;
            ssp3_config.tx_loopback_ena = 0;
            ssp3_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            ssp3_config.trans_size = CSL_I2S_SSP_TSIZE;
            ssp3_config.prot = SSPI_HW_I2S_MODE2;
            ssp3_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   	    	ssp3_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(handleSSP3, &ssp3_config);
        }
	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
    	csl_caph_switch_start_transfer(audioPath.switchCH);

        if (ssp3Running == FALSE)
        {
            csl_i2s_start(handleSSP3, &ssp3_config);
            ssp3Running = TRUE;
        }
    }   
    else
    if ((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_I2S_TX))
    {


        /* Set up the path for FM Radio TX: DDR->CFIFO->SSP3DDR
         */

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_IN;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
    	sw_config.FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	    sw_config.FIFO_dstAddr = csl_i2s_get_tx0_fifo_data_port(handleSSP3);
        
   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.trigger = CSL_CAPH_TRIG_SSP3_TX0;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_IN;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config ssp3
        if (ssp3Running == FALSE)
        {
            ssp3_config.mode = CSL_I2S_MASTER_MODE;
            ssp3_config.tx_ena = 1;
            ssp3_config.rx_ena = 1;
            ssp3_config.tx_loopback_ena = 0;
            ssp3_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            ssp3_config.trans_size = CSL_I2S_SSP_TSIZE;
            ssp3_config.prot = SSPI_HW_I2S_MODE2;
            ssp3_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    ssp3_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(handleSSP3, &ssp3_config);
        }
	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
        if (ssp3Running == FALSE)
        {
            csl_i2s_start(handleSSP3, &ssp3_config);
            ssp3Running = TRUE;
        }
        
    }   
    else
    if ((audioPath.source == CSL_CAPH_DEV_ANALOG_MIC)&&(audioPath.sink == CSL_CAPH_DEV_I2S_TX))
    {
        /* Set up the path for FM Radio TX: AudioH(AnalogMic)->SSP3
         */
   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate); 
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);



    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
	    sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr; 
	    sw_config.FIFO_dstAddr = csl_i2s_get_tx0_fifo_data_port(handleSSP3);
   		sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
				
        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config audioh
   	    audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
    	audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
   	    csl_caph_audioh_config(AUDDRV_PATH_ANALOGMIC_INPUT, (void *)&audioh_config);

	    // caph blocks start
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    } 
    else
    if ((audioPath.source == CSL_CAPH_DEV_I2S_RX)&&(audioPath.sink == CSL_CAPH_DEV_EP))
    {

        /* Set up the path for FM Radio playback: SSP3->AudioH(EP)
         */


   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);


    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.FIFO_srcAddr = csl_i2s_get_rx0_fifo_data_port(handleSSP3);
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_EARPICEC_OUTPUT);
	    sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr; 
        
	    sw_config.trigger = CSL_CAPH_TRIG_EP_THR_MET;

        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);


        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config ssp3
        if (ssp3Running == FALSE)
        {        
            ssp3_config.mode = CSL_I2S_MASTER_MODE;
            ssp3_config.tx_ena = 1;
            ssp3_config.rx_ena = 1;
            ssp3_config.tx_loopback_ena = 0;
            ssp3_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            ssp3_config.trans_size = CSL_I2S_SSP_TSIZE;
            ssp3_config.prot = SSPI_HW_I2S_MODE2;
            ssp3_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    ssp3_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(handleSSP3, &ssp3_config); 
        }
        
        // config audioh
   		audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
	    audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
   		csl_caph_audioh_config(AUDDRV_PATH_EARPICEC_OUTPUT, (void *)&audioh_config);

	    // caph blocks start
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
        if (ssp3Running == FALSE)
        {
            csl_i2s_start(handleSSP3, &ssp3_config);
            ssp3Running = TRUE;
        }        
    }   
    else
    if ((audioPath.source == CSL_CAPH_DEV_BT_MIC)&&(audioPath.sink == CSL_CAPH_DEV_MEMORY))
    {

        /* Set up the path for BT recording: SSP4->CFIFO->DDR
         */

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.FIFO_srcAddr = csl_pcm_get_rx0_fifo_data_port(handleSSP4);
        
    	sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	    sw_config.trigger = CSL_CAPH_TRIG_SSP4_RX0;

        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);
        // config ssp4
        if (ssp4Running == FALSE)
        {
            ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
            ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
            ssp4_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT;
            ssp4_dev.interleave = TRUE;
        	ssp4_dev.ext_bits=0;
	        ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;
            ssp4_configTx.enable        = 1;
            ssp4_configTx.loopback_enable	=0;
            ssp4_configRx.enable        = 1;
            ssp4_configRx.loopback_enable	=0;        
            csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
        }
	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
    	csl_caph_switch_start_transfer(audioPath.switchCH);
        if (ssp4Running == FALSE)
        {
            csl_pcm_start(handleSSP4, &ssp4_dev);
            ssp4Running = TRUE;
        }
    }   
    else
    if ((audioPath.source == CSL_CAPH_DEV_MEMORY)&&(audioPath.sink == CSL_CAPH_DEV_BT_SPKR))
    {
        /* Set up the path for BT playback: DDR->CFIFO->SSP4(BT_SPKR)
         */

   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }   
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_IN;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
    	sw_config.FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	    sw_config.FIFO_dstAddr = csl_pcm_get_tx0_fifo_data_port(handleSSP4);
        
   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.trigger = CSL_CAPH_TRIG_SSP4_TX0;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_IN;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        if (ssp4Running == FALSE)
        {
            ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
            ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
            ssp4_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT;
            ssp4_dev.interleave = TRUE;
        	ssp4_dev.ext_bits=0;
	        ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;
            ssp4_configTx.enable        = 1;
            ssp4_configTx.loopback_enable	=0;
            ssp4_configRx.enable        = 1;
            ssp4_configRx.loopback_enable	=0;        
            csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
        }
        
	    // caph blocks start
   		csl_caph_cfifo_start_fifo(audioPath.fifo);
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_dma_start_transfer(audioPath.dmaCH);
        if (ssp4Running == FALSE)
        {
            csl_pcm_start(handleSSP4, &ssp4_dev);
            ssp4Running = TRUE;
        }
    }
    else
    if ((audioPath.source == CSL_CAPH_DEV_ANALOG_MIC)&&(audioPath.sink == CSL_CAPH_DEV_BT_SPKR))
    {
        /* Set up the path for BT Recording: AudioH(AnalogMic)->SSP4
         */
   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate); 
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);



    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
	    sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr; 
	    sw_config.FIFO_dstAddr = csl_pcm_get_tx0_fifo_data_port(handleSSP4);
   		sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
    
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
				
        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config audioh
   	    audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
    	audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
   	    csl_caph_audioh_config(AUDDRV_PATH_ANALOGMIC_INPUT, (void *)&audioh_config);

	    // caph blocks start
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    } 
    else
    if ((audioPath.source == CSL_CAPH_DEV_BT_MIC)&&(audioPath.sink == CSL_CAPH_DEV_EP))
    {

        /* Set up the path for BT playback: SSP4->AudioH(EP)
         */


   		// config cfifo
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
            fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
            sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
       	    fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);

    	direction = CSL_CAPH_CFIFO_OUT;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   		csl_caph_cfifo_config_fifo(fifo, direction, threshold);


    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   		sw_config.chnl = csl_caph_switch_ch;
	    sw_config.FIFO_srcAddr = csl_pcm_get_rx0_fifo_data_port(handleSSP4);
	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_EARPICEC_OUTPUT);
	    sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr; 
        
	    sw_config.trigger = CSL_CAPH_TRIG_EP_THR_MET;

        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);
		

        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);


        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
    	dmaConfig.direction = CSL_CAPH_DMA_OUT;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	dmaConfig.mem_size = audioPath.size;
	    dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	dmaConfig.dmaCB = audioPath.dmaCB;
	    csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

        // config ssp4
        if (ssp4Running == FALSE)
        {
            ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
            ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
            ssp4_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT;
            ssp4_dev.interleave = TRUE;
        	ssp4_dev.ext_bits=0;
	        ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;
            ssp4_configTx.enable        = 1;
            ssp4_configTx.loopback_enable	=0;
            ssp4_configRx.enable        = 1;
            ssp4_configRx.loopback_enable	=0;        
            csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
        }
        
        // config audioh
   		audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
	    audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
   		csl_caph_audioh_config(AUDDRV_PATH_EARPICEC_OUTPUT, (void *)&audioh_config);

	    // caph blocks start
    	csl_caph_switch_start_transfer(audioPath.switchCH);
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
        if (ssp4Running == FALSE)
        {
            csl_pcm_start(handleSSP4, &ssp4_dev);
            ssp4Running = TRUE;
        }	
    }	
    else
    if ((audioPath.source == CSL_CAPH_DEV_DSP)&&(audioPath.sink == CSL_CAPH_DEV_IHF))	
    {
		audioh_path = AUDDRV_PATH_IHF_OUTPUT;
	
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
        	if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
        		csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
		else //all non mono is treated as stereo for now.
			csl_caph_dataformat = CSL_CAPH_16BIT_STEREO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
        	if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
        		csl_caph_dataformat = CSL_CAPH_24BIT_MONO;
		else //all non mono is treated as stereo for now.
			csl_caph_dataformat = CSL_CAPH_24BIT_STEREO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
		
   	    // config cfifo	based on data format and sampling rate
	    sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
        fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH);
        // Save the fifo information
        csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);
        direction = CSL_CAPH_CFIFO_IN;
	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
   	    csl_caph_cfifo_config_fifo(fifo, direction, threshold);

    	// config switch
	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        // Save the switch channel information
        csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            

   	    sw_config.chnl = csl_caph_switch_ch;
        sw_config.FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);

        audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
        sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr;
	    // set the trigger based on the sink
	   	    sw_config.trigger = CSL_CAPH_TRIG_IHF_THR_MET;
	
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
    	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        }
        else
        if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        {
       		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        }
        else
        {
            xassert(0, audioPath.bitPerSample );
        }
    	csl_caph_switch_config_channel(sw_config);

        // config audioh
   	    audioh_config.sample_size = audioPath.bitPerSample;
    	audioh_config.sample_pack = 0;
	    audioh_config.sample_mode = audioPath.chnlNum;
   	    csl_caph_audioh_config(audioh_path, (void *)&audioh_config);

        // config DMA
        csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
        // Get all the information from Table again.
        audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);

	    //hard coded to in direction.
	    //DMA channel configuration is done by DSP.
	    //So ARM code does not configure the AADMAC.
    	//dmaConfig.direction = CSL_CAPH_DMA_IN;
	    dmaConfig.dma_ch = audioPath.dmaCH;
    	//dmaConfig.fifo = fifo;
	    dmaConfig.mem_addr = audioPath.pBuf;
    	//dmaConfig.mem_size = audioPath.size;
	    //dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    	//dmaConfig.dmaCB = audioPath.dmaCB;
	    //csl_caph_dma_config_channel(dmaConfig);
	    csl_caph_dma_set_buffer_address(dmaConfig);
	    //csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_ARM);

	    // caph blocks start
        csl_caph_cfifo_start_fifo(audioPath.fifo);
	    //csl_caph_dma_start_transfer(audioPath.dmaCH);
    	csl_caph_switch_start_transfer(audioPath.switchCH);

	    csl_caph_audioh_start(audioh_path);
	    //csl_caph_dma_start_transfer(audioPath.dmaCH);

    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((audioPath.source == CSL_CAPH_DEV_DSP)&&(audioPath.sink == CSL_CAPH_DEV_EP))||
      ((audioPath.source == CSL_CAPH_DEV_DSP)&&(audioPath.sink == CSL_CAPH_DEV_HS)))
    {
        if (audioPath.sink == CSL_CAPH_DEV_HS)
      	    audioh_path = AUDDRV_PATH_HEADSET_OUTPUT;
        else if (audioPath.sink == CSL_CAPH_DEV_IHF)
      	    audioh_path = AUDDRV_PATH_IHF_OUTPUT;
        else  if (audioPath.sink == CSL_CAPH_DEV_EP)
      	    audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;
        else
      	    xassert(0, audioPath.sink );
        
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        {
   		    if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
   			    csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
   	        else //all non mono is treated as stereo for now.
   		        csl_caph_dataformat = CSL_CAPH_16BIT_STEREO;
        }
        else if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
   	    {
   		    if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
   			    csl_caph_dataformat = CSL_CAPH_24BIT_MONO;
   	        else //all non mono is treated as stereo for now.
   		        csl_caph_dataformat = CSL_CAPH_24BIT_STEREO;
        }
        else
   	    {
   		    xassert(0, audioPath.bitPerSample );
   	    }
	
    	csl_caph_srcm_insamplerate = csl_caph_srcmixer_get_srcm_insamplerate(audioPath.src_sampleRate);
	    csl_caph_srcm_outsamplerate = csl_caph_srcmixer_get_srcm_outsamplerate(audioPath.snk_sampleRate);
        // fixed the SRC-Mixer in channel for DSP: DL is always using ch1
        audioPath.routeConfig.inChnl = CSL_CAPH_SRCM_MONO_CH1;
	    csl_caph_srcmixer_set_inchnl_status(audioPath.routeConfig.inChnl);  
        // set the data format and sr from user
        audioPath.routeConfig.inDataFmt = csl_caph_dataformat;
        audioPath.routeConfig.inSampleRate = csl_caph_srcm_insamplerate;
        // For dsp, set the thr to 1
        audioPath.routeConfig.inThres = 0x1;

        if (audioPath.sink == CSL_CAPH_DEV_EP)
       	    audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
        else if (audioPath.sink == CSL_CAPH_DEV_HS)
       	    audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
        else if (audioPath.sink == CSL_CAPH_DEV_IHF)
       	    audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;	
        // for fpga test
        //audioPath.routeConfig.outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
       
        if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
       	    audioPath.routeConfig.outDataFmt = CSL_CAPH_16BIT_MONO;
        else 
       	    audioPath.routeConfig.outDataFmt = CSL_CAPH_24BIT_MONO;
      
       audioPath.routeConfig.outSampleRate = csl_caph_srcm_outsamplerate;
       //: should we set this to 0x1 too??
       audioPath.routeConfig.outThres = 0x1;
       
       // save the route config to path table
       csl_caph_hwctrl_SetPathRouteConfig(audioPath.pathID, audioPath.routeConfig);
       
       if ((audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_8KHZ) ||
       	(audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_16KHZ))
       {
       	csl_caph_srcmixer_config_src_route(audioPath.routeConfig);
       }
       else if (audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ)
       {
       	csl_caph_srcmixer_config_mix_route(audioPath.routeConfig);
       	// : set the mix in gain and gain step, moved in config for fpga test
       	//csl_caph_srcmixer_set_mixingain(audioPath.routeConfig.inChnl, audioPath.routeConfig.outChnl,  0x3fff, 0x3fff); 
       	// set the mix out gain, moved in config for fpga test
       	//csl_caph_srcmixer_set_mixoutgain(audioPath.routeConfig.outChnl, 0x1fff);			
       }
      
	//Mixer input gain of SRCMixer should be set as 0xFFFF7FFF so that
	//data is passed to mixer.
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C160))) = (UInt32) (0xFFFF7FFF) );
	


       // config switch
       sw_config.chnl = csl_caph_switch_obtain_channel();
       
       // Save the switch channel information
       csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, sw_config.chnl);			  
       
       chal_src_fifo = csl_caph_srcmixer_get_outchnl_fifo(audioPath.routeConfig.outChnl);
       sw_config.FIFO_srcAddr = csl_caph_srcmixer_get_fifo_addr(chal_src_fifo);
       audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
       sw_config.FIFO_dstAddr = audiohBufAddr.bufAddr;
       
       // set the trigger based on the sink
       if (audioh_path == AUDDRV_PATH_EARPICEC_OUTPUT)
       	sw_config.trigger = CSL_CAPH_TRIG_EP_THR_MET;
       else if (audioh_path == AUDDRV_PATH_HEADSET_OUTPUT)
       	sw_config.trigger = CSL_CAPH_TRIG_HS_THR_MET;
       else if (audioh_path == AUDDRV_PATH_IHF_OUTPUT)
       	sw_config.trigger = CSL_CAPH_TRIG_IHF_THR_MET;
       else if (audioh_path == AUDDRV_PATH_VIBRA_OUTPUT)
       	sw_config.trigger = CSL_CAPH_TRIG_VB_THR_MET;	
       
       sw_config.dataFmt = csl_caph_dataformat;
       csl_caph_switch_config_channel(sw_config);
       
       // config audioh
       audioh_config.sample_size = audioPath.bitPerSample;
       audioh_config.sample_pack = 0;
       audioh_config.sample_mode = audioPath.chnlNum;
       csl_caph_audioh_config(audioh_path, (void *)&audioh_config);
    
    	csl_caph_switch_start_transfer(sw_config.chnl); 
    
       csl_caph_audioh_start(audioh_path);

       //DAC_CTRL of AUDIOH should be set as 0x40000100 to amplify the 
       //earpiece
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x35020124))) = (UInt32) (0x40000100) );
       
    
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((audioPath.source == CSL_CAPH_DEV_ANALOG_MIC)&&(audioPath.sink == CSL_CAPH_DEV_DSP))
    {
    	if (audioPath.src_sampleRate == audioPath.snk_sampleRate) //no src needed
    	{
   		    // config cfifo
            if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
            {
                sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
                fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH); 
            }
            else
            if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
            {
                sampleRate = csl_caph_hwctrl_GetCSLSampleRate(audioPath.src_sampleRate);
           	   fifo = csl_caph_dma_get_csl_cfifo(config.dmaCH); 
            }
            else
            {
                xassert(0, audioPath.bitPerSample );
            }
            // Save the fifo information
            csl_caph_hwctrl_SetPathFifo(audioPath.pathID, fifo);
    
        	direction = CSL_CAPH_CFIFO_OUT;
    	    threshold = csl_caph_cfifo_get_fifo_thres(fifo);
       		csl_caph_cfifo_config_fifo(fifo, direction, threshold);
    
        	// config switch
    	    csl_caph_switch_ch = csl_caph_switch_obtain_channel();
            // Save the switch channel information
            csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);            
    
       		sw_config.chnl = csl_caph_switch_ch;
    	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
    	    sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr; 
        	sw_config.FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
       		sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
        
            if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
            {
        	    sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
            }
            else
            if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
            {
           		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
            }
            else
            {
                xassert(0, audioPath.bitPerSample );
            }
        	csl_caph_switch_config_channel(sw_config);
    				
            // config audioh
       		audioh_config.sample_size = audioPath.bitPerSample;
        	audioh_config.sample_pack = 0;
    	    audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
       		csl_caph_audioh_config(AUDDRV_PATH_ANALOGMIC_INPUT, (void *)&audioh_config);
    
    
            // config DMA
            csl_caph_hwctrl_SetPathDMACH(audioPath.pathID, config.dmaCH);
            // Get all the information from Table again.
            audioPath = csl_caph_hwctrl_GetPath_FromPathID(audioPath.pathID);
    
    	    //hard coded to in direction.
        	//dmaConfig.direction = CSL_CAPH_DMA_OUT;
    	    dmaConfig.dma_ch = audioPath.dmaCH;
        	//dmaConfig.fifo = fifo;
    	    dmaConfig.mem_addr = audioPath.pBuf;
        	//dmaConfig.mem_size = audioPath.size;
    	    //dmaConfig.Tsize = CSL_AADMAC_TSIZE;
        	//dmaConfig.dmaCB = audioPath.dmaCB;
    	    //csl_caph_dma_config_channel(dmaConfig);
    	    csl_caph_dma_set_buffer_address(dmaConfig);
    	    csl_caph_dma_enable_intr(audioPath.dmaCH, CSL_CAPH_DSP);
    
    	    // caph blocks start
       		csl_caph_cfifo_start_fifo(audioPath.fifo);
    	    //csl_caph_dma_start_transfer(audioPath.dmaCH);
        	csl_caph_switch_start_transfer(audioPath.switchCH);
    	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    	}	
	    else
	    {
        	if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        	{
        		if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
        			csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
        	else //all non mono is treated as stereo for now.
        		csl_caph_dataformat = CSL_CAPH_16BIT_STEREO;
        	}
        	else
        	if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        	{
        		if (audioPath.chnlNum == AUDIO_CHANNEL_MONO)
        			csl_caph_dataformat = CSL_CAPH_24BIT_MONO;
        	else //all non mono is treated as stereo for now.
        		csl_caph_dataformat = CSL_CAPH_24BIT_STEREO;
        	}
        	else
        	{
        		xassert(0, audioPath.bitPerSample );
        	}
    
    	    // config srcm
        	csl_caph_srcm_insamplerate = csl_caph_srcmixer_get_srcm_insamplerate(audioPath.src_sampleRate);
    	    csl_caph_srcm_outsamplerate = csl_caph_srcmixer_get_srcm_outsamplerate(audioPath.snk_sampleRate);    
            // fixed the SRC-Mixer in channel for DSP: UL is always using MONO-SRC2
            audioPath.routeConfig.inChnl = CSL_CAPH_SRCM_MONO_CH2;
    	    csl_caph_srcmixer_set_inchnl_status(audioPath.routeConfig.inChnl);
            // get SRC-Mixer in channel fifo
            chal_src_fifo = csl_caph_srcmixer_get_inchnl_fifo(audioPath.routeConfig.inChnl);
            // set the data format and sr from user
            audioPath.routeConfig.inDataFmt = csl_caph_dataformat;
            audioPath.routeConfig.inSampleRate = csl_caph_srcm_insamplerate;
            // For dsp, set the thr to 1
            audioPath.routeConfig.inThres = 0x1; // set to default
           
            // To remove a compilation warning temporarily. This needs to be fixed.
           audioPath.routeConfig.outChnl = (CSL_CAPH_SRCM_MIX_OUTCHNL_e)CSL_CAPH_SRCM_TAP_MONO_CH2;
           
           if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
           	audioPath.routeConfig.outDataFmt = CSL_CAPH_16BIT_MONO;
           else 
           	audioPath.routeConfig.outDataFmt = CSL_CAPH_24BIT_MONO;
       
           audioPath.routeConfig.outSampleRate = csl_caph_srcm_outsamplerate; 
		   
           // This should be set to 0x0 to give an interrupt after every sample.
           audioPath.routeConfig.outThres = 0x0;
           
           // save the route config to path table
           csl_caph_hwctrl_SetPathRouteConfig(audioPath.pathID, audioPath.routeConfig);
           
           if ((audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_8KHZ) ||
           	(audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_16KHZ))
           {
           	csl_caph_srcmixer_config_src_route(audioPath.routeConfig);
           }
           else if (audioPath.routeConfig.outSampleRate == CSL_CAPH_SRCMOUT_48KHZ)
           {
           	csl_caph_srcmixer_config_mix_route(audioPath.routeConfig);
           	// : set the mix in gain and gain step, moved in config for fpga test
           	//csl_caph_srcmixer_set_mixingain(audioPath.routeConfig.inChnl, audioPath.routeConfig.outChnl,  0x3fff, 0x3fff); 
           	// set the mix out gain, moved in config for fpga test
           	//csl_caph_srcmixer_set_mixoutgain(audioPath.routeConfig.outChnl, 0x1fff);			
           }
    	
        	// config switch
        	csl_caph_switch_ch = csl_caph_switch_obtain_channel();
        	// Save the switch channel information
        	csl_caph_hwctrl_SetPathSwitchCH(audioPath.pathID, csl_caph_switch_ch);			 
        	sw_config.chnl = csl_caph_switch_ch;
        	//sw_config.FIFO_srcAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
    	    audiohBufAddr = csl_caph_audioh_get_fifo_addr(AUDDRV_PATH_ANALOGMIC_INPUT);
           sw_config.FIFO_srcAddr = audiohBufAddr.bufAddr;
        	//sw_config.FIFO_dstAddr = 0xc910;
            sw_config.FIFO_dstAddr = csl_caph_srcmixer_get_fifo_addr(chal_src_fifo );
        	sw_config.trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
        
        	if (audioPath.bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
        	{
        		sw_config.dataFmt = CSL_CAPH_16BIT_MONO;
        	}
        	else
        	if (audioPath.bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
        	{
        		sw_config.dataFmt = CSL_CAPH_24BIT_MONO;
        	}
        	else
        	{
        		xassert(0, audioPath.bitPerSample );
        	}
        	csl_caph_switch_config_channel(sw_config);
        			
        	// config audioh
        	audioh_config.sample_size = audioPath.bitPerSample;
        	audioh_config.sample_pack = 0;
        	audioh_config.sample_mode = AUDIO_CHANNEL_MONO;
        	csl_caph_audioh_config(AUDDRV_PATH_ANALOGMIC_INPUT, (void *)&audioh_config);
    
    	    // start the modules in path
//#if 1	    
#if 1
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C260))) = (UInt32) (0x04000000) );
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x35020200))) = (UInt32) (0x003A0000) );
        	csl_caph_intc_enable_tapout_intr(CSL_CAPH_SRCM_MONO_CH2, CSL_CAPH_DSP);
			( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C160))) = (UInt32) (0xFFFF2777) );
		
#else    
#if 0	       	
	( *((volatile UInt32 *) (0x3502C100)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C104)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C108)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C10C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C110)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C114)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C118)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C11C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C120)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C124)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C130)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C134)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C138)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C13C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C140)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C144)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C148)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C14C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C150)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C154)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C160)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C164)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C168)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C16C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C170)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C174)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C178)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C17C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C180)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C184)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C190)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C194)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C198)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C19C)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A0)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A4)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1A8)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1AC)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1B0)) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (0x3502C1B4)) = (UInt32) (0x00000000) );
#endif	
		
#if 1//#if 1
#if 1	
     	//Do not send interrupt to DSP.
	//Instead, route data back to SRCMixer MONO CH1, to bypass DSP.
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F050))) = (UInt32) (0x805DC990) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F054))) = (UInt32) (0x8000C900) );

	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C160))) = (UInt32) (0xFFFF2777) );
#else
	// Try passthrough channel. No SRC function.
	// Passthrough CH2 --> Mixer2_Outfifo1

	// Cut the audio routes for SRCMixer
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F010))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F014))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F030))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F034))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F050))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F054))) = (UInt32) (0x00000000) );


	// Turn off MONO_CH1 and MONO_CH2 in SRCMixer
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C000))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C008))) = (UInt32) (0x00000000) );


	// AMIC 6800 -> SRCMixer PT_CH2 CB10
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F070))) = (UInt32) (0x80116800) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F074))) = (UInt32) (0x8000CB10) );

	// Config PT_CH2.  24bit Mono.
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C044))) = (UInt32) (0x00000034) );

	// Config SRC_M2D0_CH7L_GAIN_CTRL (0x3502C180)
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C180))) = (UInt32) (0xFFFF0FFF) );
	// Config SRC_M2D0_CH7R_GAIN_CTRL (0x3502C184)
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502C184))) = (UInt32) (0xFFFF0FFF) );

	// SRCMixer MIXER2_OUTFIFO1 -> EP 0x5800
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F090))) = (UInt32) (0x8016CC20) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F094))) = (UInt32) (0x80005800) );

#endif

#else
	// Directly route from VIN_FIFO to VOUT_FIFO. Bypass SRCMixer
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F010))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F014))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F030))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F034))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F050))) = (UInt32) (0x00000000) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F054))) = (UInt32) (0x00000000) );

	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F070))) = (UInt32) (0x80116800) );
	( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3502F074))) = (UInt32) (0x80005800) );

#endif
#endif		
        	csl_caph_switch_start_transfer(sw_config.chnl);
        	csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
	    }
    }		
	else
    if ((audioPath.source == CSL_CAPH_DEV_BT_MIC)&&(audioPath.sink == CSL_CAPH_DEV_BT_SPKR))
    {
         /* a simple loopback test case with dsp
         Set up the path for BT playback: SSP4 RX->DSP->SSP4 TX
         */
        // config ssp4
        ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
        ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
        ssp4_dev.format     = CSL_PCM_WORD_LENGTH_24_BIT;
        ssp4_dev.interleave = TRUE;
    	ssp4_dev.ext_bits=0;
	    ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;

        ssp4_configTx.enable        = 1;
        ssp4_configTx.loopback_enable	=0;
        ssp4_configRx.enable        = 1;
        ssp4_configRx.loopback_enable	=0;        
        csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
        csl_caph_intc_enable_pcm_intr(CSL_CAPH_DSP);		
        csl_pcm_start(handleSSP4, &ssp4_dev);
    }
	return pathID;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t path;

    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_DisablePath:: streamID: %d\r\n",
            config.streamID);

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
        path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
        //Request comes from Audio Controller
        path = csl_caph_hwctrl_GetPath_FromPathID(config.pathID);
		//csl_caph_hwctrl_RemovePath(path.pathID);
		//return RESULT_OK;
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }

    if (((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_EP))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_HS))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_IHF))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_VIBRA)))
		
    {
        // Stop DMA transfer, release channe
        csl_caph_dma_clear_intr(path.dmaCH, CSL_CAPH_ARM);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
        csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH); 
        
        // stop cfifo transfer, release fifo
        csl_caph_cfifo_stop_fifo(path.fifo);
        csl_caph_cfifo_release_fifo(path.fifo);
#if 0
        // stop switch transfer, release channel
        csl_caph_switch_stop_transfer(path.switchCH);
        csl_caph_switch_stop_transfer(sw_config2.chnl);
#endif
        csl_caph_switch_release_channel(path.switchCH);
        
        if (path.switchCH2 != CSL_CAPH_SWITCH_NONE)        
            csl_caph_switch_release_channel(path.switchCH2);
        if (path.routeConfig.inChnl != CSL_CAPH_SRCM_INCHNL_NONE)
            csl_caph_srcmixer_release_inchnl(path.routeConfig.inChnl);
        
        // stop audioh, deinit				
        if (path.sink == CSL_CAPH_DEV_EP)
            csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
        else if (path.sink == CSL_CAPH_DEV_HS)		
            csl_caph_audioh_stop(AUDDRV_PATH_HEADSET_OUTPUT);	
        else if (path.sink == CSL_CAPH_DEV_IHF)		
            csl_caph_audioh_stop(AUDDRV_PATH_IHF_OUTPUT);
        else if (path.sink == CSL_CAPH_DEV_VIBRA)		
            csl_caph_audioh_stop(AUDDRV_PATH_VIBRA_OUTPUT);

    }
    else
    if ((path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
        
    {
		// stop cfifo transfer, release fifo, deinit				
		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
		csl_caph_cfifo_stop_fifo(path.fifo2);
		csl_caph_cfifo_release_fifo(path.fifo2);

		// stop switch transfer, release channel, deinit						
		csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
		csl_caph_switch_stop_transfer(path.switchCH2);
		csl_caph_switch_release_channel(path.switchCH2);

	    // Stop DMA transfer, release channe
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
   	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);
        csl_caph_dma_disable_intr(path.dmaCH2, CSL_CAPH_ARM);
   	    csl_caph_dma_stop_transfer(path.dmaCH2);
        csl_caph_dma_release_channel(path.dmaCH2);

		// stop audioh						
        if (path.source == CSL_CAPH_DEV_DIGI_MIC)
        {
	        csl_caph_audioh_stop(AUDDRV_PATH_VIN_INPUT);        
        }
        else
        if (path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
        {
	        csl_caph_audioh_stop(AUDDRV_PATH_NVIN_INPUT);        
        }
    }    
    else
    if((path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_EP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_HS)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_IHF))
    {
   		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
        csl_caph_audioh_stop(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_stop(AUDDRV_PATH_EANC_INPUT); 
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
		// stop cfifo transfer, release fifo, deinit				
		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);

		// stop switch transfer, release channel, deinit						
		csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);

	    // Stop DMA transfer, release channe
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
   	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);

		// stop audioh, deinit						
		csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);        
    }
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
	    // caph blocks start
   		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);

        if (ssp3Running == TRUE)
        {
            csl_i2s_stop_tx(handleSSP3);
            csl_i2s_stop_rx(handleSSP3);
            ssp3Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
	    // caph blocks start
   		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);

        if (ssp3Running == TRUE)
        {
            csl_i2s_stop_tx(handleSSP3);
            csl_i2s_stop_rx(handleSSP3);            
            ssp3Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
	    csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    // caph blocks start
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
	    csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
        if (ssp3Running == TRUE)
        {
            csl_i2s_stop_tx(handleSSP3);
            csl_i2s_stop_rx(handleSSP3);            
            ssp3Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
	    // caph blocks start
   		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
        if (ssp4Running == TRUE)
        {
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX0);
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX1);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX0);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX1);
            ssp4Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    // caph blocks start
   		csl_caph_cfifo_stop_fifo(path.fifo);
		csl_caph_cfifo_release_fifo(path.fifo);
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
        csl_caph_dma_disable_intr(path.dmaCH, CSL_CAPH_ARM);
	    csl_caph_dma_stop_transfer(path.dmaCH);
        csl_caph_dma_release_channel(path.dmaCH);
        if (ssp4Running == TRUE)
        {
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX0);
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX1);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX0);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX1);
            ssp4Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    // caph blocks start
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
	    csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    // caph blocks start
    	csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
	    csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
        if (ssp4Running == TRUE)
        {
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX0);
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX1);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX0);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX1);
            ssp4Running = FALSE;
        }
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_EP))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_HS))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_IHF)))
    {
		// stop switch transfer, release channel					
		csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
       	// stop audioh				
       	if (path.sink == CSL_CAPH_DEV_EP)
   			csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
       	else if (path.sink == CSL_CAPH_DEV_HS)		
       		csl_caph_audioh_stop(AUDDRV_PATH_HEADSET_OUTPUT);	
       	else if (path.sink == CSL_CAPH_DEV_IHF)		
       		csl_caph_audioh_stop(AUDDRV_PATH_IHF_OUTPUT);
   		// release the src chnl
             csl_caph_srcmixer_release_inchnl(path.routeConfig.inChnl);		
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
    {
    		// stop the src intc to dsp
        	csl_caph_intc_disable_tapout_intr(CSL_CAPH_SRCM_MONO_CH2, CSL_CAPH_DSP);
		// stop switch transfer, release channel					
		csl_caph_switch_stop_transfer(path.switchCH);
		csl_caph_switch_release_channel(path.switchCH);
		// stop audioh					
		csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
   		// release the src chnl
             csl_caph_srcmixer_release_inchnl(path.routeConfig.inChnl);		
    }
	
	csl_caph_hwctrl_RemovePath(path.pathID);

	return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Pause a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t path;

    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_PausePath:: streamID: %d\r\n",
            config.streamID);

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
        path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
        //Requsetion comes from Audio Controller
        path = csl_caph_hwctrl_GetPath_FromPathID(config.pathID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }


    // Then pause the stream.
    if (((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_EP))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_HS))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_IHF))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_VIBRA)))
		
    {
    	// stop audioh				
    	if (path.sink == CSL_CAPH_DEV_EP)
    		csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
	    else if (path.sink == CSL_CAPH_DEV_HS)		
		    csl_caph_audioh_stop(AUDDRV_PATH_HEADSET_OUTPUT);	
    	else if (path.sink == CSL_CAPH_DEV_IHF)		
	    	csl_caph_audioh_stop(AUDDRV_PATH_IHF_OUTPUT);
    	else if (path.sink == CSL_CAPH_DEV_VIBRA)		
	    	csl_caph_audioh_stop(AUDDRV_PATH_VIBRA_OUTPUT);
    }
    else
    if ((path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
        
    {
		// stop audioh						
        if (path.source == CSL_CAPH_DEV_DIGI_MIC)
        {
	        csl_caph_audioh_stop(AUDDRV_PATH_VIN_INPUT);        
        }
        else
        if (path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
        {
	        csl_caph_audioh_stop(AUDDRV_PATH_NVIN_INPUT);        
        }
    }    
    else
    if((path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_EP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_HS)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_IHF))
    {
        csl_caph_audioh_stop(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_stop(AUDDRV_PATH_EANC_INPUT); 
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
		csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);        
    }
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
        if (ssp3Running == TRUE)
        {
            csl_i2s_stop_tx(handleSSP3);
            csl_i2s_stop_rx(handleSSP3);
            ssp3Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
        if (ssp3Running == TRUE)
        {
            csl_i2s_stop_tx(handleSSP3);
            csl_i2s_stop_rx(handleSSP3);            
            ssp3Running = FALSE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
	    csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
#if 0        
        if (ssp4Running == TRUE)
        {
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX0);
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX1);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX0);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX1);
            ssp4Running = FALSE;
        }
#else        
        csl_pcm_pause(handleSSP4);
#endif
        
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
#if 0        
        if (ssp4Running == TRUE)
        {
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX0);
            csl_pcm_stop_tx(handleSSP4, CSL_PCM_CHAN_TX1);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX0);
            csl_pcm_stop_rx(handleSSP4, CSL_PCM_CHAN_RX1);
            ssp4Running = FALSE;
        }
#else        
        csl_pcm_pause(handleSSP4);
#endif
        
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_EP))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_HS))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_IHF)))
    {
       	// stop audioh				
       	if (path.sink == CSL_CAPH_DEV_EP)
   			csl_caph_audioh_stop(AUDDRV_PATH_EARPICEC_OUTPUT);
       	else if (path.sink == CSL_CAPH_DEV_HS)		
       		csl_caph_audioh_stop(AUDDRV_PATH_HEADSET_OUTPUT);	
       	else if (path.sink == CSL_CAPH_DEV_IHF)		
       		csl_caph_audioh_stop(AUDDRV_PATH_IHF_OUTPUT);
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
    {
		csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Resume a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t path;
    CSL_I2S_CONFIG_t ssp3_config;
//    csl_pcm_config_device_t ssp4_dev;
//    csl_pcm_config_tx_t ssp4_configTx; 
//    csl_pcm_config_rx_t ssp4_configRx;

    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_PausePath:: streamID: %d\r\n",
            config.streamID);
    memset(&ssp3_config, 0, sizeof(CSL_I2S_CONFIG_t));
//    memset(&ssp4_dev, 0, sizeof(csl_pcm_config_device_t));
//    memset(&ssp4_configTx, 0, sizeof(csl_pcm_config_tx_t));
//    memset(&ssp4_configRx, 0, sizeof(csl_pcm_config_rx_t));

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
        path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
        //Request comes from Audio Controller
        path = csl_caph_hwctrl_GetPath_FromPathID(config.pathID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }

    // Then resume the stream.
    if (((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_EP))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_HS))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_IHF))||
	  ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_VIBRA)))
		
    {
    	// start audioh				
    	if (path.sink == CSL_CAPH_DEV_EP)
    		csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
	    else if (path.sink == CSL_CAPH_DEV_HS)		
		    csl_caph_audioh_start(AUDDRV_PATH_HEADSET_OUTPUT);	
    	else if (path.sink == CSL_CAPH_DEV_IHF)		
	    	csl_caph_audioh_start(AUDDRV_PATH_IHF_OUTPUT);
    	else if (path.sink == CSL_CAPH_DEV_VIBRA)		
	    	csl_caph_audioh_start(AUDDRV_PATH_VIBRA_OUTPUT);
    }
    else
    if ((path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
        
    {
		// start audioh						
        if (path.source == CSL_CAPH_DEV_DIGI_MIC)
        {
	        csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT);        
        }
        else
        if (path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)
        {
	        csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT);        
        }
    }    
    else
    if((path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_EP)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_HS)
        ||(path.source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path.sink == CSL_CAPH_DEV_IHF))
    {
        csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_start(AUDDRV_PATH_EANC_INPUT); 
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
		csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);        
    }
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
        if (ssp3Running == FALSE)
        {
            ssp3_config.mode = CSL_I2S_MASTER_MODE;
            ssp3_config.tx_ena = 1;
            ssp3_config.rx_ena = 1;
            ssp3_config.tx_loopback_ena = 0;
            ssp3_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            ssp3_config.trans_size = CSL_I2S_SSP_TSIZE;
            ssp3_config.prot = SSPI_HW_I2S_MODE2;
            ssp3_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    ssp3_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(handleSSP3, &ssp3_config); 
            csl_i2s_start(handleSSP3, &ssp3_config);
            ssp3Running = TRUE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
        if (ssp3Running == FALSE)
        {
            ssp3_config.mode = CSL_I2S_MASTER_MODE;
            ssp3_config.tx_ena = 1;
            ssp3_config.rx_ena = 1;
            ssp3_config.tx_loopback_ena = 0;
            ssp3_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            ssp3_config.trans_size = CSL_I2S_SSP_TSIZE;
            ssp3_config.prot = SSPI_HW_I2S_MODE2;
            ssp3_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    ssp3_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(handleSSP3, &ssp3_config); 
            csl_i2s_start(handleSSP3, &ssp3_config);
            ssp3Running = TRUE;
        }
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_I2S_TX))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_I2S_RX)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_MEMORY))
    {
#if 0        
        if (ssp4Running == FALSE)
        {
            ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
            ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
            ssp4_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT;
            ssp4_dev.interleave = TRUE;
        	ssp4_dev.ext_bits=0;
	        ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;
            ssp4_configTx.enable        = 1;
            ssp4_configTx.loopback_enable	=0;
            ssp4_configRx.enable        = 1;
            ssp4_configRx.loopback_enable	=0;        
            csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
            csl_pcm_start(handleSSP4, &ssp4_dev);
            ssp4Running = TRUE;
        }
#else        
        csl_pcm_resume(handleSSP4);
#endif
    }    
    else
    if ((path.source == CSL_CAPH_DEV_MEMORY)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
#if 0        
        if (ssp4Running == FALSE)
        {
            ssp4_dev.mode       = CSL_PCM_MASTER_MODE;
            ssp4_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
            ssp4_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT;
            ssp4_dev.interleave = TRUE;
        	ssp4_dev.ext_bits=0;
	        ssp4_dev.xferSize=CSL_PCM_SSP_TSIZE;
            ssp4_configTx.enable        = 1;
            ssp4_configTx.loopback_enable	=0;
            ssp4_configRx.enable        = 1;
            ssp4_configRx.loopback_enable	=0;        
            csl_pcm_config(handleSSP4, &ssp4_dev, &ssp4_configTx, &ssp4_configRx); 
            csl_pcm_start(handleSSP4, &ssp4_dev);
            ssp4Running = TRUE;
        }
#else        
        csl_pcm_resume(handleSSP4);
#endif
        
    }    
    else
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path.source == CSL_CAPH_DEV_BT_MIC)&&(path.sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_EP))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_HS))||
      ((path.source == CSL_CAPH_DEV_DSP)&&(path.sink == CSL_CAPH_DEV_IHF)))
    {
       	// start audioh				
       	if (path.sink == CSL_CAPH_DEV_EP)
   			csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
       	else if (path.sink == CSL_CAPH_DEV_HS)		
       		csl_caph_audioh_start(AUDDRV_PATH_HEADSET_OUTPUT);	
       	else if (path.sink == CSL_CAPH_DEV_IHF)		
       		csl_caph_audioh_start(AUDDRV_PATH_IHF_OUTPUT);
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path.source == CSL_CAPH_DEV_ANALOG_MIC)&&(path.sink == CSL_CAPH_DEV_DSP))
    {
		csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }
    
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
*                                                       UInt32 gainL_mB,
*                                                       UInt32 gainR_mB)
*
*  Description: Set the gain for sink. In AudioHub, the gain can noly be 
*  done by using mixing output gain inside SRCMixer. No plan to use the 
*  mixing input gain. 
*  For the audio paths which do not go through SRCMixer, the gain is not doable.
*
*  Note1: If the path is a stereo path, gainL_mB is for Left channel and 
*  gainR_mB is for right channel. If the path is mono path, gainL_mB is for the
*  channel. gainR_mB is ignored.
*
*  Note2: gain_mB is the gain in millibel. 1dB = 100mB. UInt32 gain_mB is in 
*  Q31.0 format. It is:
*
*   2147483647mB, i.e. 21474836dB ->
*          7FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
*   ......
*   2mB -> 0000 0000 0000 0000 0000 0000 0000 0002
*   1mB -> 0000 0000 0000 0000 0000 0000 0000 0001
*   0mB -> 0000 0000 0000 0000 0000 0000 0000 0000
*  -1mB -> FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
*  -2mB -> FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFE
*  ......
*  -2147483648mB, i.e. -21474836.48dB->
*          8000 0000 0000 0000 0000 0000 0000 0000
*
****************************************************************************/
void csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
                                      UInt32 gainL_mB,
                                      UInt32 gainR_mB)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetSource(CSL_CAPH_PathID pathID, 
*                                                     UInt32 gainL_mB,
*                                                     UInt32 gainR_mB)
*
*  Description: Set the gain for source. In AudioHub, the gain can noly be 
*  done by using audioh CIC scale setting.
*
*  Note1: If the path is a stereo path, gainL_mB is for Left channel and 
*  gainR_mB is for right channel. If the path is mono path, gainL_mB is for the
*  channel. gainR_mB is ignored.
*
*  Note2: gain_mB is the gain in millibel. 1dB = 100mB. UInt32 gain_mB is in 
*  Q31.0 format. It is:
*
*   2147483647mB, i.e. 21474836dB ->
*          7FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
*   ......
*   2mB -> 0000 0000 0000 0000 0000 0000 0000 0002
*   1mB -> 0000 0000 0000 0000 0000 0000 0000 0001
*   0mB -> 0000 0000 0000 0000 0000 0000 0000 0000
*  -1mB -> FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
*  -2mB -> FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFE
*  ......
*  -2147483648mB, i.e. -21474836.48dB->
*          8000 0000 0000 0000 0000 0000 0000 0000

****************************************************************************/
void csl_caph_hwctrl_SetSourceGain(CSL_CAPH_PathID pathID,
                                        UInt32 gainL_mB,
                                        UInt32 gainR_mB)

{
    CSL_CAPH_HWConfig_Table_t path;
    memset(&path, 0, sizeof(path));

    path = csl_caph_hwctrl_GetPath_FromPathID(pathID);

    switch(path.source)
    {
        case CSL_CAPH_DEV_DIGI_MIC:
            csl_caph_audioh_setgain(AUDDRV_PATH_ANALOGMIC_INPUT, gainL_mB, gainR_mB);
            break;
        default:
            break;
    }
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_MuteSink(CSL_CAPH_DEVICE_e sink)
*
*  Description: Mute sink
*
****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_DEVICE_e sink)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_MuteSource(CSL_CAPH_DEVICE_e source)
*
*  Description: Mute sink
*
****************************************************************************/
void csl_caph_hwctrl_MuteSource(CSL_CAPH_DEVICE_e source)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_UnmuteSink(CSL_CAPH_DEVICE_e sink)
*
*  Description: Unmute sink
*
****************************************************************************/
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_DEVICE_e sink)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_UnmuteSource(CSL_CAPH_DEVICE_e source)
*
*  Description: Unmute sink
*
****************************************************************************/
void csl_caph_hwctrl_UnmuteSource(CSL_CAPH_DEVICE_e source)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SwitchSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Switch to a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSink(CSL_CAPH_DEVICE_e sink)
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AddSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Add a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddSink(CSL_CAPH_DEVICE_e sink)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RemoveSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Remove a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSink(CSL_CAPH_DEVICE_e sink)    
{
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SwitchSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Switch to a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AddSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Add a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RemoveSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Remove a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetFilter(CSL_CAPH_HWCTRL_FILTER_e filter, 
*                                                     void* coeff)    
*
*  Description: Load filter coefficients
*
****************************************************************************/
void csl_caph_hwctrl_SetFilter(CSL_CAPH_HWCTRL_FILTER_e filter, void* coeff)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_EnableSideTone(void)    
*  
*  Description: Enable Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_EnableSideTone(void)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisableSideTone(void)    
*  
*  Description: Disable Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_DisableSideTone(void)    
{
    return;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_SetSideToneGain(UInt32 gain_mB)    
*  
*  Description: Set the sidetone gain
*
****************************************************************************/
void csl_caph_hwctrl_SetSideToneGain(UInt32 gain_mB)
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_EnableEANC(void)    
*
*  Description: Enable EANC path
*
****************************************************************************/
void csl_caph_hwctrl_EnableEANC(void)    
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisableEANC(void)    
*
*  Description: Disable EANC path
*
****************************************************************************/
void csl_caph_hwctrl_DisableEANC(void)    
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RegisterStreamID(CSL_CAPH_DEVICE_e source, 
*                                  CSL_CAPH_DEVICE_e sink, CSL_CAPH_STREAM_e streamID)
*
*  Description: Register StreamID
*
****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStreamID(CSL_CAPH_DEVICE_e source, 
                                  CSL_CAPH_DEVICE_e sink,
                                  CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;

    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if ((HWConfig_Table[i].source == source)
            &&(HWConfig_Table[i].sink == sink)
            &&(HWConfig_Table[i].status == PATH_AVAILABLE))
        {
            HWConfig_Table[i].streamID = streamID;
            HWConfig_Table[i].status = PATH_OCCUPIED;
            return RESULT_OK;
        }
    }
    return RESULT_ERROR;
}



/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_STREAM_e streamID,
*                                        UInt8* mem_addr,
*                                        UInt8* mem_addr2,
*                                        UInt32 mem_size,
*                                        CSL_CAPH_DMA_CALLBACK_p dmaCB)
*
*  Description: Register a Stream into hw control driver.
*
****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream)
{
    UInt8 i = 0;

    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == stream->streamID)
        {
            HWConfig_Table[i].src_sampleRate = stream->src_sampleRate;
            HWConfig_Table[i].snk_sampleRate = stream->snk_sampleRate;
            HWConfig_Table[i].chnlNum = stream->chnlNum;
            HWConfig_Table[i].bitPerSample = stream->bitPerSample;
            HWConfig_Table[i].pBuf = stream->pBuf;
            HWConfig_Table[i].pBuf2 = stream->pBuf2;
            HWConfig_Table[i].size = stream->size;
            HWConfig_Table[i].dmaCB = stream->dmaCB;
            return RESULT_OK;
        }
    }
    return RESULT_ERROR;
}


/****************************************************************************
*
*  Function Name: AUDIO_BITS_PER_SAMPLE_t csl_caph_hwctrl_GetDataFormat(
*                                                CSL_CAPH_STREAM_e streamID)
*
*  Description: Get the data format of the Stream.
*
****************************************************************************/
AUDIO_BITS_PER_SAMPLE_t csl_caph_hwctrl_GetDataFormat(CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;
    for (i=0; i<=MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == streamID)
        {
            return HWConfig_Table[i].bitPerSample;
        }
    }
    return 0;
}


/****************************************************************************
*
* Function Name: void csl_caph_audio_loopback_control( CSL_CAPH_DEVICE_e speaker, 
*													int path, Boolean ctrl)
*
*
* Description:   CLS control microphone loop back to output path
*
* Parameters:    
*				 spekaer : output speaker
*				 ctrl    : control to loop back 
*						   TRUE - enable loop back in path,
*						   FALSE - disbale loop back in path
*                path    : internal loopback path
*
* Return:       none

****************************************************************************/

void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker, 
									int path, 
									Boolean ctrl)
{
	UInt32 dacmask = 0;

	if(speaker == CSL_CAPH_DEV_EP)
	{
		dacmask = CHAL_AUDIO_PATH_EARPIECE;
	}
	else if(speaker == CSL_CAPH_DEV_HS)
	{
		dacmask = CHAL_AUDIO_PATH_HEADSET_LEFT | CHAL_AUDIO_PATH_HEADSET_RIGHT;
	}
	else if(speaker == CSL_CAPH_DEV_IHF)
	{
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT | CHAL_AUDIO_PATH_IHF_RIGHT;
	}
	else
	{
		dacmask = CHAL_AUDIO_PATH_EARPIECE;
	}

#if	1

	if(ctrl) 
	{
		chal_audio_audiotx_set_dac_ctrl(lp_handle,AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
//		chal_audio_audiotx_set_spare_bit(lp_handle);
	}

	else	chal_audio_audiotx_set_dac_ctrl(lp_handle,0x00);

#endif

	chal_audio_loopback_set_out_paths(lp_handle, dacmask, ctrl);
	chal_audio_loopback_enable(lp_handle, ctrl);
	

}

