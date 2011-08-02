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
*  @file   csl_caph_common.c
*
*  @brief  csl layer common code for caph driver
*
****************************************************************************/
#include "mobcom_types.h"
#include "xassert.h"
#include "log.h"
#include "chal_caph.h"
#include "csl_aud_drv.h"
#include "csl_caph.h"
#include "csl_caph_common.h"


//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************

extern CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];
extern CSL_CAPH_HWResource_Table_t HWResource_Table[CSL_CAPH_FIFO_MAX_NUM];
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
static void csl_caph_common_addHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID);


//******************************************************************************
// local function definitions
//******************************************************************************

/****************************************************************************
*
*  Function Name: void  csl_caph_common_addHWResource(UInt32 fifoAddr,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Add path ID to the HW resource table.
*
****************************************************************************/
static void csl_caph_common_addHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    UInt8 j = 0;
    if (fifoAddr == 0x0) return;
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == fifoAddr)
        {
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                //If pathID already exists. Just return.
                if (HWResource_Table[j].pathID[i] == pathID)
                    return;
            }
            //Add the new pathID 
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                if (HWResource_Table[j].pathID[i] == 0)
                {
                    HWResource_Table[j].pathID[i] = pathID;
                    return;
                }
            }
        }
    }
    //fifoAddr does not exist. So add it.
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == 0x0)
        {
            HWResource_Table[j].fifoAddr = fifoAddr;
            HWResource_Table[j].pathID[0] = pathID;
            return;
        }
    }
    //Should not run to here.
    //Size of the table is not big enough.
    audio_xassert(0, j);
    return;
}


//******************************************************************************
// Global function definitions
//******************************************************************************


/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_Table_t csl_caph_common_GetPath_FromPathID
*                                              (CSL_CAPH_PathID pathID)
*
*  Description: Get the HW configuration table a path from the path table
*
****************************************************************************/
CSL_CAPH_HWConfig_Table_t csl_caph_common_GetPath_FromPathID(CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    CSL_CAPH_HWConfig_Table_t path;

    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            path.pathID = HWConfig_Table[i].pathID;
            path.streamID = HWConfig_Table[i].streamID;
            path.source = HWConfig_Table[i].source;
            path.sink = HWConfig_Table[i].sink;
            path.sink2 = HWConfig_Table[i].sink2;
            path.src_sampleRate = HWConfig_Table[i].src_sampleRate;
            path.snk_sampleRate = HWConfig_Table[i].snk_sampleRate;			
            path.chnlNum = HWConfig_Table[i].chnlNum;
            path.bitPerSample = HWConfig_Table[i].bitPerSample;
            path.fifo = HWConfig_Table[i].fifo;
            path.fifo2 = HWConfig_Table[i].fifo2;
	        memcpy(&(path.switchCH), &(HWConfig_Table[i].switchCH), sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
	        memcpy(&(path.switchCH2), &(HWConfig_Table[i].switchCH2), sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
	        memcpy(&(path.switchCH3), &(HWConfig_Table[i].switchCH3), sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            path.dmaCH = HWConfig_Table[i].dmaCH;
            path.dmaCH2 = HWConfig_Table[i].dmaCH2;
            path.routeConfig = HWConfig_Table[i].routeConfig;		
            path.routeConfig2 = HWConfig_Table[i].routeConfig2;		
            path.routeConfig3 = HWConfig_Table[i].routeConfig3;		
            path.pBuf = HWConfig_Table[i].pBuf;
            path.pBuf2 = HWConfig_Table[i].pBuf2;
            path.size = HWConfig_Table[i].size;
            path.dmaCB = HWConfig_Table[i].dmaCB;
            path.status = HWConfig_Table[i].status;
            return path;
        }
    }
    audio_xassert(0, pathID);
    return path;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSource(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_DEVICE_e source)
*
*  Description: Set the source into the path table
*
****************************************************************************/
void csl_caph_common_SetPathSource(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e source)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].source = source;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}




/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSink(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_DEVICE_e sink)
*
*  Description: Set the sink into the path table
*
****************************************************************************/
void csl_caph_common_SetPathSink(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].sink = sink;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSource(
*                                       CSL_CAPH_PathID pathID)
*
*  Description: Clear the source from the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSource(CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].source = CSL_CAPH_DEV_NONE; 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSink(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_DEVICE_e sink)
*
*  Description: Clear the sink from the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSink(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            if (HWConfig_Table[i].sink == sink)
            {
                HWConfig_Table[i].sink = CSL_CAPH_DEV_NONE; 
                if (HWConfig_Table[i].sink2 == sink)
                    HWConfig_Table[i].sink2 = CSL_CAPH_DEV_NONE; 
            } 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSink2(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_DEVICE_e sink)
*
*  Description: Set the second sink into the path table
*
****************************************************************************/
void csl_caph_common_SetPathSink2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].sink2 = sink;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSink2(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_DEVICE_e sink)
*
*  Description: Clear the second sink from the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSink2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            if (HWConfig_Table[i].sink2 == sink)
            {
                HWConfig_Table[i].sink2 = CSL_CAPH_DEV_NONE; 
                if (HWConfig_Table[i].sink == sink)
                    HWConfig_Table[i].sink = CSL_CAPH_DEV_NONE; 
            } 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathFifo(CSL_CAPH_PathID pathID,
*                                                CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: Set the FIFO for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathFifo(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_CFIFO_FIFO_e fifo)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].fifo = fifo;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSwitchCH(
*                                   CSL_CAPH_PathID pathID)
*
*  Description: Clear the Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH(CSL_CAPH_PathID pathID) 
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memset(&(HWConfig_Table[i].switchCH), 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSwitchCH(
*                                   CSL_CAPH_PathID pathID,
*                                   CSL_CAPH_SWITCH_CONFIG_t switchCH)
*
*  Description: Set the Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathSwitchCH(CSL_CAPH_PathID pathID, 
                                     CSL_CAPH_SWITCH_CONFIG_t switchCH)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memcpy(&(HWConfig_Table[i].switchCH), &switchCH, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            csl_caph_common_addHWResource(switchCH.FIFO_srcAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dstAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst2Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst3Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst4Addr, pathID);
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSwitchCH2(
*                                   CSL_CAPH_PathID pathID)
*
*  Description: Clear the 2nd Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH2(CSL_CAPH_PathID pathID) 
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memset(&(HWConfig_Table[i].switchCH2), 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSwitchCH2(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SWITCH_CONFIG_t switchCH)
*
*  Description: Set the second Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathSwitchCH2(CSL_CAPH_PathID pathID, 
                                      CSL_CAPH_SWITCH_CONFIG_t switchCH)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memcpy(&(HWConfig_Table[i].switchCH2), &switchCH, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            csl_caph_common_addHWResource(switchCH.FIFO_srcAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dstAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst2Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst3Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst4Addr, pathID);
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathSwitchCH3(
*                                   CSL_CAPH_PathID pathID)
*
*  Description: Clear the 3rd Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH3(CSL_CAPH_PathID pathID) 
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memset(&(HWConfig_Table[i].switchCH3), 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathSwitchCH3(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SWITCH_CONFIG_t switchCH)
*
*  Description: Set the third Switch CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathSwitchCH3(CSL_CAPH_PathID pathID, 
                                      CSL_CAPH_SWITCH_CONFIG_t switchCH)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
	        memcpy(&(HWConfig_Table[i].switchCH3), &switchCH, sizeof(CSL_CAPH_SWITCH_CONFIG_t)); 
            csl_caph_common_addHWResource(switchCH.FIFO_srcAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dstAddr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst2Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst3Addr, pathID);
            csl_caph_common_addHWResource(switchCH.FIFO_dst4Addr, pathID);
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathDMACH(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_DMA_CHNL_e dmaCH)
*
*  Description: Set the DMA CH for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathDMACH(CSL_CAPH_PathID pathID, 
                                  CSL_CAPH_DMA_CHNL_e dmaCH)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].dmaCH = dmaCH;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathRouteConfig(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the route config for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathRouteConfig(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_ROUTE_t routeConfig)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].routeConfig = routeConfig;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathRouteConfig(CSL_CAPH_PathID pathID)
*
*  Description: Clear the route config for a path in the path table
*
****************************************************************************/
void csl_caph_common_ClearPathRouteConfig(CSL_CAPH_PathID pathID) 
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            memset(&(HWConfig_Table[i].routeConfig), 0, sizeof(CSL_CAPH_SRCM_ROUTE_t));
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_SetPathRouteConfig2(CSL_CAPH_PathID pathID,
*                                           CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the second route config for a path in the path table
*
****************************************************************************/
void csl_caph_common_SetPathRouteConfig2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_ROUTE_t routeConfig)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].routeConfig2 = routeConfig;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_common_ClearPathRouteConfig(CSL_CAPH_PathID pathID)
*
*  Description: Clear the 2nd route config for a path in the path table
*
****************************************************************************/
void csl_caph_common_ClearPathRouteConfig2(CSL_CAPH_PathID pathID) 
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            memset(&(HWConfig_Table[i].routeConfig2), 0, sizeof(CSL_CAPH_SRCM_ROUTE_t));
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name:  CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_common_GetCSLSampleRate
*                                            (AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Get the CSL sample rate
*
****************************************************************************/
CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_common_GetCSLSampleRate(AUDIO_SAMPLING_RATE_t sampleRate)
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
*  Function Name:  CSL_CAPH_DATAFORMAT_e csl_caph_common_GetOutPutDataFormat
*                                (AUDIO_BITS_PER_SAMPLE_t bitPerSampleRate,
*                                CSL_CAPH_DEVICE_e dev)
*
*  Description: Get the CSL data format of SRCMixer output data.
*
****************************************************************************/
CSL_CAPH_DATAFORMAT_e csl_caph_common_GetOutPutDataFormat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, CSL_CAPH_DEVICE_e dev)
{
    CSL_CAPH_DATAFORMAT_e outDataFmt = CSL_CAPH_16BIT_MONO;
    if (dev == CSL_CAPH_DEV_HS)
    {
        if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
           	outDataFmt = CSL_CAPH_16BIT_STEREO;
        else 
       	    outDataFmt = CSL_CAPH_24BIT_STEREO;
    }
    else
    {
        if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
       	    outDataFmt = CSL_CAPH_16BIT_MONO;
        else 
       	    outDataFmt = CSL_CAPH_24BIT_MONO;
 
    }
    return outDataFmt;
}

/****************************************************************************
*
*  Function Name:  CSL_CAPH_DATAFORMAT_e csl_caph_common_GetDataFormat
*                                (AUDIO_BITS_PER_SAMPLE_t bitPerSampleRate,
*                                AUDIO_CHANNEL_NUM_t chnlNum)
*
*  Description: Get the CSL data format
*
****************************************************************************/
CSL_CAPH_DATAFORMAT_e csl_caph_common_GetDataFormat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, AUDIO_CHANNEL_NUM_t chnlNum)
{
    CSL_CAPH_DATAFORMAT_e csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
    if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
    {
        if (chnlNum == AUDIO_CHANNEL_MONO)
            csl_caph_dataformat = CSL_CAPH_16BIT_MONO;
        else //all non mono is treated as stereo for now.
            csl_caph_dataformat = CSL_CAPH_16BIT_STEREO;
    }
    else
    if (bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
    {
        if (chnlNum == AUDIO_CHANNEL_MONO)
    	    csl_caph_dataformat = CSL_CAPH_24BIT_MONO;
        else //all non mono is treated as stereo for now.
            csl_caph_dataformat = CSL_CAPH_24BIT_STEREO;
    }
    else
    {
        audio_xassert(0, bitPerSample );
    }
    return csl_caph_dataformat;	
}


//============================================================================
//
// Function Name: void csl_caph_common_GetAudioHPath(CSL_CAPH_DEVICE_e dev)
//
// Description:  Get audioh Path from Device
//
//============================================================================
AUDDRV_PATH_Enum_t csl_caph_common_GetAudiohPath(CSL_CAPH_DEVICE_e dev)
{
    AUDDRV_PATH_Enum_t	audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;	

    switch (dev)
    {
        case CSL_CAPH_DEV_HS:
            audioh_path = AUDDRV_PATH_HEADSET_OUTPUT;
            break;
        case CSL_CAPH_DEV_IHF:
            audioh_path = AUDDRV_PATH_IHF_OUTPUT;
            break;
        case CSL_CAPH_DEV_EP:
            audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;
            break;
        case CSL_CAPH_DEV_VIBRA:
            audioh_path = AUDDRV_PATH_VIBRA_OUTPUT;	
            break;
        case CSL_CAPH_DEV_ANALOG_MIC:
            audioh_path = AUDDRV_PATH_ANALOGMIC_INPUT;	
            break;
        case CSL_CAPH_DEV_HS_MIC:
            audioh_path = AUDDRV_PATH_HEADSET_INPUT;	
            break;
        case CSL_CAPH_DEV_DIGI_MIC_L:
            audioh_path = AUDDRV_PATH_VIN_INPUT_L;	
            break;
        case CSL_CAPH_DEV_DIGI_MIC_R:
            audioh_path = AUDDRV_PATH_VIN_INPUT_R;	
            break;
        case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
            audioh_path = AUDDRV_PATH_NVIN_INPUT_L;	
            break;
        case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
            audioh_path = AUDDRV_PATH_NVIN_INPUT_R;	
            break;
        default:
            audio_xassert(0, dev );
    }
    return audioh_path;
}



//============================================================================
//
// Function Name: CSL_CAPH_SWITCH_TRIGGER_e csl_caph_common_GetSwitchTrigger(CSL_CAPH_DEVICE_e dev)
//
// Description:  Get switch channel trigger from Device
//
//============================================================================
CSL_CAPH_SWITCH_TRIGGER_e  csl_caph_common_GetSwitchTrigger(CSL_CAPH_DEVICE_e dev)
{
    CSL_CAPH_SWITCH_TRIGGER_e trigger = CSL_CAPH_TRIG_NONE;

    switch (dev)
    {
        case CSL_CAPH_DEV_HS:
       	    trigger = CSL_CAPH_TRIG_HS_THR_MET;
            break;
        case CSL_CAPH_DEV_IHF:
       	    trigger = CSL_CAPH_TRIG_IHF_THR_MET;
            break;
        case CSL_CAPH_DEV_EP:
       	    trigger = CSL_CAPH_TRIG_EP_THR_MET;
            break;
        case CSL_CAPH_DEV_VIBRA:
       	    trigger = CSL_CAPH_TRIG_VB_THR_MET;	
            break;
        case CSL_CAPH_DEV_ANALOG_MIC:
        case CSL_CAPH_DEV_HS_MIC:
        case CSL_CAPH_DEV_DIGI_MIC_L:
       	    trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;	
            break;
        case CSL_CAPH_DEV_DIGI_MIC_R:
       	    trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET;	
            break;
        case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
       	    trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOR_THR_MET;	
            break;
        case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
       	    trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOL_THR_MET;	
            break;
  
        default:
            audio_xassert(0, dev );
    }
    return trigger;
}            


//============================================================================
//
// Function Name: CSL_CAPH_DATAFORMAT_e  csl_caph_common_GetSRCMixerOutputDataFormat(CSL_CAPH_DEVICE_e sink, AUDIO_BITS_PER_SAMPLE_t bitPerSample)
//
// Description:  Get SRCMixer output data format
//
//============================================================================
CSL_CAPH_DATAFORMAT_e  csl_caph_common_GetSRCMixerOutputDataFormat(CSL_CAPH_DEVICE_e dev, AUDIO_BITS_PER_SAMPLE_t bitPerSample)
{
    CSL_CAPH_DATAFORMAT_e outDataFmt = CSL_CAPH_16BIT_MONO;

    if (dev == CSL_CAPH_DEV_HS)
    {
        if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
            outDataFmt = CSL_CAPH_16BIT_STEREO;
        else 
            outDataFmt = CSL_CAPH_24BIT_STEREO;
    }
    else
    {
        if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
            outDataFmt = CSL_CAPH_16BIT_MONO;
        else 
            outDataFmt = CSL_CAPH_24BIT_MONO;
    }   
    return outDataFmt;
}    
