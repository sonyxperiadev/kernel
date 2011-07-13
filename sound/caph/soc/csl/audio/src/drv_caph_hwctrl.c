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
*  @file   drv_caph_hwctrl.c
*
*  @brief  driver layer driver for caph render
*
****************************************************************************/
#include "resultcode.h"
#include "mobcom_types.h"
#include "xassert.h"
#include "msconsts.h"
#include "log.h"
#include "osinterrupt.h"
#include "audio_plat_defconfig.h"
#ifdef CONFIG_AUDIO_BUILD
#include "chip_irq.h"
#include "irqctrl.h"
#include "pm_prm.h"
#endif
#include "auddrv_def.h"
#include "csl_caph.h"
#include "csl_caph_dma.h"
#include "csl_caph_hwctrl.h"
#include "csl_caph_gain.h"
#include "drv_caph.h"
#include "drv_audio_common.h"
#include "drv_caph_hwctrl.h"
#include "osdw_caph_drv.h"

#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_khub_clk_mgr_reg.h"

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************
//#define _DBG_(a)
#define _DBG_(a) (a)

//****************************************************************************
// local macro declarations
//****************************************************************************
#define MAX_AUDIO_PATH  32
#define PATH_OCCUPIED   1
#define PATH_AVAILABLE  0
#define MAX_AUDIO_CLOCK_NUM 6
//****************************************************************************
// local typedef declarations
//****************************************************************************

//****************************************************************************
// local variable definitions
//****************************************************************************
static Boolean hwClkEnabled = FALSE;

#ifdef CONFIG_AUDIO_BUILD
static Interrupt_t AUDDRV_HISR_HANDLE;
static CLIENT_ID id[MAX_AUDIO_CLOCK_NUM] = {0, 0, 0, 0, 0, 0};
//****************************************************************************
// local function declarations
//****************************************************************************
static void AUDDRV_ControlHWClock(Boolean enable);
#else
#include "clock.h"
#include "clk.h"
static struct clk *clkID[MAX_AUDIO_CLOCK_NUM] = {NULL,NULL,NULL,NULL,NULL,NULL};
#endif
//****************************************************************************
// local function declarations
//****************************************************************************
static CSL_CAPH_STREAM_e AUDDRV_GetCSLStreamID(AUDDRV_STREAM_e streamID);
static CSL_CAPH_HW_GAIN_e AUDDRV_GetCSLHWGainSelect(AUDDRV_HW_GAIN_e hw);

//******************************************************************************
// local function definitions
//******************************************************************************

// ==========================================================================
//
// Function Name: void AUDDRV_ControlHWClock(Boolean enable)
//
// Description: This is to enable/disable the audio HW clocks
//                  KHUB_CAPH_SRCMIXER_CLK
//                  KHUB_AUDIOH_2P4M_CLK
//                  KHUB_AUDIOH_26M_CLK
//                  KHUB_AUDIOH_156M_CLK
//
// =========================================================================

static void AUDDRV_ControlHWClock(Boolean enable)
{
#ifdef CONFIG_AUDIO_BUILD
#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)                                 
#if !defined(FPGA_VERSION)
    if (enable == TRUE)
    {
        //Enable CAPH clock.
        id[0] = PRM_client_register("AUDIO_CAPH_DRV");
        PRM_set_clock_state(id[0], RESOURCE_CAPH, CLOCK_ON);
        PRM_set_clock_speed(id[0], RESOURCE_CAPH, 156000000);

    	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_SSP3, KHUB_SSP3_AUDIO_CLK, CLOCK_CLK_EN, clock_op_enable);
        //id[1] = PRM_client_register("AUDIO_CAPH_SSP3_DRV");
        //PRM_set_clock_state(id[1], RESOURCE_SSP3_AUDIO, CLOCK_ON);
        //PRM_set_clock_speed(id[1], RESOURCE_SSP3_AUDIO, 26000000);
    
	    // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_2P4M_CLK, CLOCK_CLK_EN, clock_op_enable);
        id[2] = PRM_client_register("AUDIO_AUDIOH1_DRV");
        PRM_set_clock_state(id[2], RESOURCE_AUDIOH_2P4M, CLOCK_ON);
        // no need to set speed, it is fixed
        //PRM_set_clock_speed(id[2], RESOURCE_AUDIOH_2P4M, 26000000);

	    // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_26M_CLK, CLOCK_CLK_EN, clock_op_enable);
        id[3] = PRM_client_register("AUDIO_AUDIOH2_DRV");
        PRM_set_clock_state(id[3], RESOURCE_AUDIOH, CLOCK_ON);
	    // no need to set the speed. it is fixed
        //PRM_set_clock_speed(id[3], RESOURCE_AUDIOH, 26000000);

	    // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_156M_CLK, CLOCK_CLK_EN, clock_op_enable);
        id[4] = PRM_client_register("AUDIO_AUDIOH3_DRV");
        PRM_set_clock_state(id[4], RESOURCE_AUDIOH_156M, CLOCK_ON);
	    // no need to set speed, it is fixed.
        //PRM_set_clock_speed(id[4], RESOURCE_AUDIOH_156M, 26000000);
    
    	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_SSP4, KHUB_SSP4_AUDIO_CLK, CLOCK_CLK_EN, clock_op_enable);
        //id[5] = PRM_client_register("AUDIO_CAPH_SSP4_DRV");
        //PRM_set_clock_state(id[5], RESOURCE_SSP4_AUDIO, CLOCK_ON);
        //PRM_set_clock_speed(id[5], RESOURCE_SSP4_AUDIO, 26000000);
    }
    else
    {
        PRM_set_clock_state(id[0], RESOURCE_CAPH, CLOCK_OFF);
        PRM_client_deregister(id[0]);
        PRM_set_clock_state(id[2], RESOURCE_AUDIOH_2P4M, CLOCK_OFF);
        PRM_client_deregister(id[2]);
        PRM_set_clock_state(id[3], RESOURCE_AUDIOH, CLOCK_OFF);
        PRM_client_deregister(id[3]);
        PRM_set_clock_state(id[4], RESOURCE_AUDIOH_156M, CLOCK_OFF);
        PRM_client_deregister(id[4]);
    }
#endif    
#endif
    return;
#endif
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_Init(void)
*
*  Description: init CAPH HW control driver
*
****************************************************************************/
Result_t AUDDRV_HWControl_Init(void)
{
    CSL_CAPH_HWCTRL_BASE_ADDR_t addr;
   	printk(KERN_INFO "AUDDRV_HWControl_Init:: \n");

#if !(defined(_SAMOA_))
//Enable CAPH clock.
    clkID[0] = clk_get(NULL, "caph_srcmixer_clk");
#ifdef CONFIG_ARCH_ISLAND     /* island srcmixer is not set correctly. 
                                This is a workaround before a solution from clock */
    if ( clkID[0]->use_cnt )
    {
        clk_disable(clkID[0]);
    }
#endif
	clk_set_rate(clkID[0], 156000000);
    clk_enable(clkID[0]);
    

	//clkID[1] = clk_get(NULL, "audioh_apb_clk");
    //clk_enable(clkID[1]);
    //clk_set_rate(clkID[1], 156000000);
#ifdef CONFIG_DEPENDENCY_ENABLE_SSP34
	clkID[1] = clk_get(NULL, "ssp3_audio_clk");
    clk_enable(clkID[1]);
    //clk_set_rate(clkID[1], 156000000);
#endif
    
	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_2P4M_CLK, CLOCK_CLK_EN, clock_op_enable);
    clkID[2] = clk_get(NULL, "audioh_2p4m_clk");
    clk_enable(clkID[2]);
    // no need to set speed, it is fixed
    //clk_set_rate(clkID[2], 26000000);
                                
	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_26M_CLK, CLOCK_CLK_EN, clock_op_enable);
    clkID[3] = clk_get(NULL,"audioh_26m_clk");
    clk_enable(clkID[3]);
	// no need to set the speed. it is fixed
    //clk_set_rate(clkID[3],  26000000);

	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_156M_CLK, CLOCK_CLK_EN, clock_op_enable);
    clkID[4] = clk_get(NULL,"audioh_156m_clk");
    clk_enable(clkID[4]);
    //clk_set_rate(clkID[4], 26000000);

	// chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_SSP4, KHUB_SSP4_AUDIO_CLK, CLOCK_CLK_EN, clock_op_enable);
#ifdef CONFIG_DEPENDENCY_ENABLE_SSP34
    clkID[5] = clk_get(NULL, "ssp4_audio_clk");
    clk_enable(clkID[5]);
    //clk_set_rate(clkID[5], 156000000);
#endif
#endif // !defined(_SAMOA_)

    CAPHIRQ_Init();

    memset(&addr, 0, sizeof(CSL_CAPH_HWCTRL_BASE_ADDR_t));
    addr.cfifo_baseAddr = CFIFO_BASE_ADDR1;
    addr.aadmac_baseAddr = AADMAC_BASE_ADDR1;
    addr.ahintc_baseAddr = AHINTC_BASE_ADDR1;
    addr.ssasw_baseAddr = SSASW_BASE_ADDR1;
    addr.srcmixer_baseAddr = SRCMIXER_BASE_ADDR1;
    addr.audioh_baseAddr = AUDIOH_BASE_ADDR1;
    addr.sdt_baseAddr = SDT_BASE_ADDR1;

    addr.ssp3_baseAddr = SSP3_BASE_ADDR1;
    addr.ssp4_baseAddr = SSP4_BASE_ADDR1;
	
    csl_caph_hwctrl_init(addr);
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DeInit(void)
*
*  Description: De-Initialize CAPH HW Control driver
*
****************************************************************************/
Result_t AUDDRV_HWControl_DeInit(void)
{
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_DeInit:: \n"));

#ifdef CONFIG_AUDIO_BUILD
	// this is just for fpga test. in real code may not need this.
	IRQ_Disable(CAPH_NORM_IRQ);
	 if (AUDDRV_HISR_HANDLE )
	   {
	   	OSINTERRUPT_Destroy(AUDDRV_HISR_HANDLE );
        	AUDDRV_HISR_HANDLE  = NULL;
	   }	
#endif	 
    csl_caph_hwctrl_deinit(); 

#ifndef CONFIG_AUDIO_BUILD
#if !defined(_SAMOA_)
	clk_disable(clkID[0]);
	clk_disable(clkID[1]);
	clk_disable(clkID[2]);
	clk_disable(clkID[3]);
	clk_disable(clkID[4]);
	clk_disable(clkID[5]);
#endif
#else
    //Disable CAPH clock.
    PRM_set_clock_state(id[0], RESOURCE_CAPH, CLOCK_OFF);
    PRM_client_deregister(id[0]);

    //PRM_set_clock_state(id[1], RESOURCE_SSP3_AUDIO, CLOCK_OFF);
    //PRM_client_deregister(id[1]);

    PRM_set_clock_state(id[2], RESOURCE_AUDIOH_2P4M, CLOCK_OFF);
    PRM_client_deregister(id[2]);

    PRM_set_clock_state(id[3], RESOURCE_AUDIOH, CLOCK_OFF);
    PRM_client_deregister(id[3]);

    PRM_set_clock_state(id[4], RESOURCE_AUDIOH_156M, CLOCK_OFF);
    PRM_client_deregister(id[4]);

    //PRM_set_clock_state(id[5], RESOURCE_SSP4_AUDIO, CLOCK_OFF);
    //PRM_client_deregister(id[5]);	
#endif
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name:Result_t AUDDRV_HWControl_EnablePath(AUDDRV_HWCTRL_CONFIG_t config) 
*
*  Description: Enable a HW path
*
****************************************************************************/
AUDDRV_PathID AUDDRV_HWControl_EnablePath(AUDDRV_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWCTRL_CONFIG_t cslConfig;
    CSL_CAPH_PathID cslPathID = 0;

	if(hwClkEnabled == FALSE)
    {
        AUDDRV_ControlHWClock(TRUE);
        hwClkEnabled = TRUE;
    }

    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);
    cslConfig.source = AUDDRV_GetCSLDevice(config.source);
    cslConfig.sink = AUDDRV_GetCSLDevice(config.sink);
    cslConfig.src_sampleRate = config.src_sampleRate;
    cslConfig.snk_sampleRate = config.snk_sampleRate;	
    cslConfig.chnlNum = config.chnlNum;
    cslConfig.bitPerSample = config.bitPerSample;
    memcpy(&(cslConfig.mixGain), &(config.mixGain), sizeof(CSL_CAPH_SRCM_MIX_GAIN_t));

    cslPathID = csl_caph_hwctrl_EnablePath(cslConfig); 

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_EnablePath::  Source: %d, Sink: %d, pathID %d.\r\n",
            config.source, config.sink, cslPathID));
	return (AUDDRV_PathID)cslPathID;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DisablePath(AUDDRV_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
*
****************************************************************************/
Result_t AUDDRV_HWControl_DisablePath(AUDDRV_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWCTRL_CONFIG_t cslConfig;
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO,
                    "AUDDRV_HWControl_DisablePath:: streamID: %d, pathID %d.\r\n",
                    config.streamID, config.pathID));
    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);

    (void)csl_caph_hwctrl_DisablePath(cslConfig);
    if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
    {
        AUDDRV_ControlHWClock(FALSE);
        hwClkEnabled = FALSE;
    }
    return RESULT_OK;
}



/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_PausePath(AUDDRV_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
*
****************************************************************************/
Result_t AUDDRV_HWControl_PausePath(AUDDRV_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWCTRL_CONFIG_t cslConfig;
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "AUDDRV_HWControl_PausePath:: streamID: %d\r\n",
                    config.streamID));
    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);

    return csl_caph_hwctrl_PausePath(cslConfig);
}


/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_ResumePath(AUDDRV_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
*
****************************************************************************/
Result_t AUDDRV_HWControl_ResumePath(AUDDRV_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWCTRL_CONFIG_t cslConfig;
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "AUDDRV_HWControl_ResumePath:: streamID: %d\r\n",
                    config.streamID));
    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);

    return csl_caph_hwctrl_ResumePath(cslConfig);
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
*                                                       UInt16 gainL,
*                                                       UInt16 gainR)
*
*  Description: Set the sink gain on the path.
*
*  Note1: If the path is a stereo path, gainL is for Left channel and 
*  gainR is for right channel. If the path is mono path, gainL is for the
*  channel. gainR is ignored.
*
*  Note2: gain is in Q13.2 format. It is:
*
*   8191.75dB ->   7FFF
*   8191.5dB  ->   7FFE       
*   ......
*   0.5dB    ->    0002
*   0.25dB   ->    0001
*   0dB ->         0000
*   -0.25dB  ->    FFFF
*   -1dB     ->    FFFE
*  ......
*  -8191.75dB->    8001
*
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
                                      UInt16 gainL,
                                      UInt16 gainR)
{
    csl_caph_hwctrl_SetSinkGain((CSL_CAPH_PathID)pathID, 
                                gainL,
                                gainR);

    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetSourceGain(AUDDRV_PathID pathID, 
*                                                     UInt16 gainL,
*                                                     UInt16 gainR)
*
*  Description: Set the source gain on the path.
*
*  Note1: If the path is a stereo path, gainL is for Left channel and 
*  gainR is for right channel. If the path is mono path, gainL is for the
*  channel. gainR is ignored.
*
*  Note2: gain is in Q13.2 format. It is:
*
*   8191.75dB ->   7FFF
*   8191.5dB  ->   7FFE       
*   ......
*   0.5dB    ->    0002
*   0.25dB   ->    0001
*   0dB ->         0000
*   -0.25dB  ->    FFFF
*   -1dB     ->    FFFE
*  ......
*  -8191.75dB->    8001

****************************************************************************/
Result_t AUDDRV_HWControl_SetSourceGain(AUDDRV_PathID pathID,
                                        UInt16 gainL,
                                        UInt16 gainR)

{
    csl_caph_hwctrl_SetSourceGain((CSL_CAPH_PathID)pathID,
                                  gainL,
                                  gainR);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_MuteSink(AUDDRV_PathID pathID)
*
*  Description: Mute sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_MuteSink(AUDDRV_PathID pathID)
{
    csl_caph_hwctrl_MuteSink((CSL_CAPH_PathID)pathID);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_MuteSource(AUDDRV_PathID pathID)
*
*  Description: Mute sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_MuteSource(AUDDRV_PathID pathID)
{
    csl_caph_hwctrl_MuteSource((CSL_CAPH_PathID)pathID);	
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_UnmuteSink(AUDDRV_PathID pathID)
*
*  Description: Unmute sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSink(AUDDRV_PathID pathID)
{
    csl_caph_hwctrl_UnmuteSink((CSL_CAPH_PathID)pathID);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_UnmuteSource(AUDDRV_PathID pathID)
*
*  Description: Unmute sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSource(AUDDRV_PathID pathID)
{
    csl_caph_hwctrl_UnmuteSource((CSL_CAPH_PathID)pathID);	
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SwitchSink(AUDDRV_DEVICE_e sink)    
*
*  Description: Switch to a new sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSink(AUDDRV_DEVICE_e sink)
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_AddSink(AUDDRV_DEVICE_e sink)    
*
*  Description: Add a new sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_AddSink(AUDDRV_DEVICE_e sink)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_RemoveSink(AUDDRV_DEVICE_e sink)    
*
*  Description: Remove a new sink
*
****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSink(AUDDRV_DEVICE_e sink)    
{
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SwitchSource(AUDDRV_DEVICE_e source)    
*
*  Description: Switch to a new source
*
****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSource(AUDDRV_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_AddSource(AUDDRV_DEVICE_e source)    
*
*  Description: Add a new source
*
****************************************************************************/
Result_t AUDDRV_HWControl_AddSource(AUDDRV_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_RemoveSource(AUDDRV_DEVICE_e source)    
*
*  Description: Remove a new source
*
****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSource(AUDDRV_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, 
*                                                     void* coeff)    
*
*  Description: Load filter coefficients
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff)
{
    switch(filter)
    {
        case AUDDRV_SIDETONE_FILTER:
           csl_caph_hwctrl_ConfigSidetoneFilter(coeff);		
	   break;
    	default:
	   ;
    }
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_EnableSideTone(
*  				AudioMode_t audioMode)    
*  
*  Description: Enable Sidetone path
*
****************************************************************************/
Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audioMode)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    switch(audioMode)
    {
        case AUDIO_MODE_HANDSET:
        case AUDIO_MODE_HANDSET_WB:
        case AUDIO_MODE_HAC:
        case AUDIO_MODE_HAC_WB:
	    config.sink = CSL_CAPH_DEV_EP; 
            break;
        case AUDIO_MODE_HEADSET:
        case AUDIO_MODE_HEADSET_WB:
        case AUDIO_MODE_TTY:
        case AUDIO_MODE_TTY_WB:
	    config.sink = CSL_CAPH_DEV_HS; 
            break;
        case AUDIO_MODE_SPEAKERPHONE:
        case AUDIO_MODE_SPEAKERPHONE_WB:
	    config.sink = CSL_CAPH_DEV_IHF; 
            break;

	default:
	    // For other audio modes. nothing to be done.
	return RESULT_OK;
    }
    csl_caph_hwctrl_EnableSidetone(config, TRUE);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audioMode)    
*  
*  Description: Disable Sidetone path
*
****************************************************************************/
Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audioMode)    
{
   CSL_CAPH_HWCTRL_CONFIG_t config;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    switch(audioMode)
    {
        case AUDIO_MODE_HANDSET:
        case AUDIO_MODE_HANDSET_WB:
        case AUDIO_MODE_HAC:
        case AUDIO_MODE_HAC_WB:
	    config.sink = CSL_CAPH_DEV_EP; 
            break;
        case AUDIO_MODE_HEADSET:
        case AUDIO_MODE_HEADSET_WB:
        case AUDIO_MODE_TTY:
        case AUDIO_MODE_TTY_WB:
	    config.sink = CSL_CAPH_DEV_HS; 
            break;
        case AUDIO_MODE_SPEAKERPHONE:
        case AUDIO_MODE_SPEAKERPHONE_WB:
	    config.sink = CSL_CAPH_DEV_IHF; 
            break;

	default:
	    // For other audio modes. nothing to be done.
	return RESULT_OK;
    }	
    csl_caph_hwctrl_EnableSidetone(config, FALSE);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name:Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain)    
*  
*  Description: Set the sidetone gain
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain)
{
	csl_caph_hwctrl_SetSidetoneGain(gain);	
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_EnableEANC(void)    
*
*  Description: Enable EANC path
*
****************************************************************************/
Result_t AUDDRV_HWControl_EnableEANC(void)    
{
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DisableEANC(void)    
*
*  Description: Disable EANC path
*
****************************************************************************/
Result_t AUDDRV_HWControl_DisableEANC(void)    
{
	return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetDSPSharedMeMForIHF(void)
*
*  Description: Pass the DSP shared Mem for IHF
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetDSPSharedMeMForIHF(UInt32 addr)
{
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
		    "AUDDRV_HWControl_SetDSPSharedMeMForIHF:: \n"));
    csl_caph_hwctrl_setDSPSharedMemForIHF(addr);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: AUDDRV_HWControl_ConfigSSP
*
*  Description: Configure fm/pcm port
*
*****************************************************************************/
Result_t AUDDRV_HWControl_ConfigSSP(UInt8 fm_port, UInt8 pcm_port)
{
	CSL_CAPH_SSP_Config_t sspConfig;
	memset(&sspConfig, 0, sizeof(CSL_CAPH_SSP_Config_t));
	
	sspConfig.fm_port = (CSL_CAPH_SSP_e)fm_port;
	sspConfig.pcm_port = (CSL_CAPH_SSP_e)pcm_port;

	if (sspConfig.fm_port == CSL_CAPH_SSP_3)
		sspConfig.fm_baseAddr = SSP3_BASE_ADDR1;
	else if (sspConfig.fm_port == CSL_CAPH_SSP_4)
		sspConfig.fm_baseAddr = SSP4_BASE_ADDR1;

	if (sspConfig.pcm_port == CSL_CAPH_SSP_3)
		sspConfig.pcm_baseAddr = SSP3_BASE_ADDR1;
	else if (sspConfig.pcm_port == CSL_CAPH_SSP_4)
		sspConfig.pcm_baseAddr = SSP4_BASE_ADDR1;

	csl_caph_hwctrl_ConfigSSP(sspConfig);
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: AUDDRV_HWControl_SetSspTdmMode
*
*  Description: set ssp tdm
*
*****************************************************************************/
void AUDDRV_HWControl_SetSspTdmMode(Boolean status)
{
	csl_caph_hwctrl_SetSspTdmMode(status);
}

/****************************************************************************
*
* Function Name: Result_t AUDDRV_SetAudioLoopback(AUDDRV_MIC_Enum_t mic, 
*                          AUDDRV_SPKR_Enum_t speaker, int path, Boolean ctrl)
*
* Description:   control microphone loop back to output path
*
* Parameters:    mic     : input microphone device 
*				 spekaer : output speaker
*				 enable_lpbk    : control to loop back 
*						   TRUE - enable loop back in path,
*						   FALSE - disbale loop back in path
*                path    : internal loopback path
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_SetAudioLoopback( 
			Boolean             enable_lpbk,
			AUDDRV_MIC_Enum_t   mic,
			AUDDRV_SPKR_Enum_t  speaker,
			int path
			)
{

	CSL_CAPH_DEVICE_e spkr;

	if(speaker == AUDDRV_SPKR_EP)
	{
		spkr = CSL_CAPH_DEV_EP;
	}
	else if(speaker == AUDDRV_SPKR_HS)
	{
		spkr = CSL_CAPH_DEV_HS;
	}
	else if(speaker == AUDDRV_SPKR_IHF)
	{
		spkr = CSL_CAPH_DEV_IHF;
	}
	else
	{
		spkr = CSL_CAPH_DEV_EP;
	}

	csl_caph_audio_loopback_control(spkr, path, enable_lpbk);

}
/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator,
*													 AUDDRV_VIBRATOR_MODE_Enum_t mode) 
* Description:   Enable/Disable vibrator with default strength
*
* Parameters:    enable_vibrator     : enable (TRUE) or disable (FALSE) operation to vibrator  
*                mode                : by_pass mode ( 0 ) or PCM playback mode (non zero)
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator, AUDDRV_VIBRATOR_MODE_Enum_t mode)
{

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_EnableVibrator \n"));

	csl_caph_hwctrl_vibrator(mode, enable_vibrator);

}

/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_VibratorStrength (UInt32 strength)
*
* Description:   Change the vibrator strength
*
* Parameters:    strength : strength value to vibrator  
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_VibratorStrength(UInt32 strength)
{

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_VibratorStrength strength = 0x%lx \n",strength));

	csl_caph_hwctrl_vibrator_strength(strength); 

}



/****************************************************************************
*
*Result_t AUDDRV_HWControl_SetMixOutputGain(AUDDRV_PathID pathID, 
*                                      fineGainL,
*                                      coarseGainL,
*                                      fineGainR,
*                                      coarseGainR)		
*
*  Description: Set Mixer output gain in HW Mixer. Gain in Q13.2.
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetMixOutputGain(AUDDRV_PathID pathID, 
                                      UInt32 fineGainL,
                                      UInt32 coarseGainL,
                				      UInt32 fineGainR,
                                      UInt32 coarseGainR)		
{
    csl_caph_Mixer_GainMapping_t outGainL, outGainR;
    csl_caph_Mixer_GainMapping2_t outGain2L, outGain2R;

    outGainL = csl_caph_gain_GetMixerGain((Int16)fineGainL);
    outGainR = csl_caph_gain_GetMixerGain((Int16)fineGainR);
    outGain2L = csl_caph_gain_GetMixerOutputCoarseGain((Int16)coarseGainL);
    outGain2R = csl_caph_gain_GetMixerOutputCoarseGain((Int16)coarseGainR);

    csl_caph_hwctrl_SetMixOutGain((CSL_CAPH_PathID)pathID, 
                                      outGainL.mixerOutputFineGain,
                                      outGain2L.mixerOutputCoarseGain,
                                      outGainR.mixerOutputFineGain,
                                      outGain2R.mixerOutputCoarseGain);
    return RESULT_OK;
}
/****************************************************************************
*
*  AUDDRV_HWControl_SetMixingGain(AUDDRV_PathID pathID, UInt32 gainL, 
 						UInt32 gainR)
*
*  Description: Set Mixing gain in HW Mixer. Gain in Q13.2 Format.
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetMixingGain(AUDDRV_PathID pathID, 
						UInt32 gainL, 
 						UInt32 gainR)
{
    csl_caph_hwctrl_SetMixingGain((CSL_CAPH_PathID)pathID, 
  						gainL, 
 						gainR);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name:void AUDDRV_HWControl_SetHWGain(AUDDRV_PathID pathID, 
*                       AUDDRV_HW_GAIN_e hw,
*  						UInt32 gain,
*  						AUDDRV_DEVICE_e dev)
*
*  Description: Set Hw gain. Gain in Q13.2
*
****************************************************************************/
void  AUDDRV_HWControl_SetHWGain(AUDDRV_PathID pathID, AUDDRV_HW_GAIN_e hw, UInt32 gain, AUDDRV_DEVICE_e dev)
{
	csl_caph_hwctrl_SetHWGain((CSL_CAPH_PathID)pathID, 
            AUDDRV_GetCSLHWGainSelect(hw), 
			gain, 
			AUDDRV_GetCSLDevice(dev));
	return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_STREAM_e AUDDRV_GetCSLStreamID(AUDDRV_STREAM_e streamID)
*
*  Description: Get the CSL streamID from DRV layer
*
****************************************************************************/
static CSL_CAPH_STREAM_e AUDDRV_GetCSLStreamID(AUDDRV_STREAM_e streamID)
{
    CSL_CAPH_STREAM_e cslStreamID = CSL_CAPH_STREAM_NONE;
    switch (streamID)
    {
        case AUDDRV_STREAM_NONE:
            cslStreamID = CSL_CAPH_STREAM_NONE;
            break;
        case AUDDRV_STREAM1:
            cslStreamID = CSL_CAPH_STREAM1;
            break;
        case AUDDRV_STREAM2:
            cslStreamID = CSL_CAPH_STREAM2;
            break;
        case AUDDRV_STREAM3:
            cslStreamID = CSL_CAPH_STREAM3;
            break;
        case AUDDRV_STREAM4:
            cslStreamID = CSL_CAPH_STREAM4;
            break;
        case AUDDRV_STREAM5:
            cslStreamID = CSL_CAPH_STREAM5;
            break;
        case AUDDRV_STREAM6:
            cslStreamID = CSL_CAPH_STREAM6;
            break;
        case AUDDRV_STREAM7:
            cslStreamID = CSL_CAPH_STREAM7;
            break;
        case AUDDRV_STREAM8:
            cslStreamID = CSL_CAPH_STREAM8;
            break;
        case AUDDRV_STREAM9:
            cslStreamID = CSL_CAPH_STREAM9;
            break;
        case AUDDRV_STREAM10:
            cslStreamID = CSL_CAPH_STREAM10;
            break;
        case AUDDRV_STREAM11:
            cslStreamID = CSL_CAPH_STREAM11;
            break;
        case AUDDRV_STREAM12:
            cslStreamID = CSL_CAPH_STREAM12;
            break;
        case AUDDRV_STREAM13:
            cslStreamID = CSL_CAPH_STREAM13;
            break;
        case AUDDRV_STREAM14:
            cslStreamID = CSL_CAPH_STREAM14;
            break;
        case AUDDRV_STREAM15:
            cslStreamID = CSL_CAPH_STREAM15;
            break;
        case AUDDRV_STREAM16:
            cslStreamID = CSL_CAPH_STREAM16;
            break;
            
        default:
            xassert(0, streamID);
            break;
    }
    return cslStreamID;
}



/****************************************************************************
*
*  Function Name: CSL_CAPH_HW_GAIN_e AUDDRV_GetCSLHWGainSelect(AUDDRV_HW_GAIN_e hw)
*
*  Description: Get the CSL HW Gain Select from DRV layer
*
****************************************************************************/
static CSL_CAPH_HW_GAIN_e AUDDRV_GetCSLHWGainSelect(AUDDRV_HW_GAIN_e hw)
{
    CSL_CAPH_HW_GAIN_e cslHW = CSL_CAPH_AMIC_DGA_COARSE_GAIN;
    switch (hw)
    {
        case AUDDRV_AMIC_PGA_GAIN:
            cslHW = CSL_CAPH_AMIC_PGA_GAIN;
            break;
        case AUDDRV_AMIC_DGA_COARSE_GAIN:
            cslHW = CSL_CAPH_AMIC_DGA_COARSE_GAIN;
            break;
        case AUDDRV_AMIC_DGA_FINE_GAIN:
            cslHW = CSL_CAPH_AMIC_DGA_FINE_GAIN;
            break;
        case AUDDRV_DMIC1_DGA_COARSE_GAIN:
            cslHW = CSL_CAPH_DMIC1_DGA_COARSE_GAIN;
            break;
        case AUDDRV_DMIC1_DGA_FINE_GAIN:
            cslHW = CSL_CAPH_DMIC1_DGA_FINE_GAIN;
            break;
        case AUDDRV_DMIC2_DGA_COARSE_GAIN:
            cslHW = CSL_CAPH_DMIC2_DGA_COARSE_GAIN;
            break;
        case AUDDRV_DMIC2_DGA_FINE_GAIN:
            cslHW = CSL_CAPH_DMIC2_DGA_FINE_GAIN;
            break;
        case AUDDRV_DMIC3_DGA_COARSE_GAIN:
            cslHW = CSL_CAPH_DMIC3_DGA_COARSE_GAIN;
            break;
        case AUDDRV_DMIC3_DGA_FINE_GAIN:
            cslHW = CSL_CAPH_DMIC3_DGA_FINE_GAIN;
            break;
        case AUDDRV_DMIC4_DGA_COARSE_GAIN:
            cslHW = CSL_CAPH_DMIC4_DGA_COARSE_GAIN;
            break;
        case AUDDRV_DMIC4_DGA_FINE_GAIN:
            cslHW = CSL_CAPH_DMIC4_DGA_FINE_GAIN;
            break;
        case AUDDRV_SRCM_INPUT_GAIN_L:
            cslHW = CSL_CAPH_SRCM_INPUT_GAIN_L;
            break;
        case AUDDRV_SRCM_OUTPUT_COARSE_GAIN_L:
            cslHW = CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L;
            break;
        case AUDDRV_SRCM_OUTPUT_FINE_GAIN_L:
            cslHW = CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L;
            break;
        case AUDDRV_SRCM_INPUT_GAIN_R:
            cslHW = CSL_CAPH_SRCM_INPUT_GAIN_R;
            break;
        case AUDDRV_SRCM_OUTPUT_COARSE_GAIN_R:
            cslHW = CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R;
            break;
        case AUDDRV_SRCM_OUTPUT_FINE_GAIN_R:
            cslHW = CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R;
            break;
            
        default:
            xassert(0, hw);
            break;
    }
    return cslHW;
}
