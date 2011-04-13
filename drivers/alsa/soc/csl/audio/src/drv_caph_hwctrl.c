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
//#include "osinterrupt.h"
//#include "chip_irq.h"
//#include "irqctrl.h"
#include "auddrv_def.h"
#include "csl_caph.h"
#include "csl_caph_dma.h"
#include "csl_caph_hwctrl.h"
#include "drv_caph.h"
#include "drv_audio_common.h"
#include "drv_caph_hwctrl.h"
//#include "pm_prm.h"  - check this later kishore

#include "mach/rdb/brcm_rdb_sysmap_a9.h"
#include "mach/rdb/brcm_rdb_khub_clk_mgr_reg.h"

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
#define MAX_AUDIO_PATH  32
#define PATH_OCCUPIED   1
#define PATH_AVAILABLE  0

//****************************************************************************
// local typedef declarations
//****************************************************************************

//****************************************************************************
// local variable definitions
//****************************************************************************
#if 0
static Interrupt_t AUDDRV_HISR_HANDLE;
static CLIENT_ID id = 0;

#endif
//****************************************************************************
// local function declarations
//****************************************************************************
#if 0
static void AUDDRV_LISR(void);
static void AUDDRV_HISR(void);
#endif
static CSL_CAPH_STREAM_e AUDDRV_GetCSLStreamID(AUDDRV_STREAM_e streamID);

//******************************************************************************
// local function definitions
//******************************************************************************
#if 0
// ==========================================================================
//
// Function Name: void AUDDRV_LISR(void)
//
// Description: CAPH_NORM_IRQ LISR
//
// =========================================================================

static void AUDDRV_LISR(void)
{
	IRQ_Disable(CAPH_NORM_IRQ);
	OSINTERRUPT_Trigger(AUDDRV_HISR_HANDLE);
}
// ==========================================================================
//
// Function Name: void AUDDRV_HISR(void)
//
// Description:CAPH_NORM_IRQ HISR
//
// =========================================================================

static void AUDDRV_HISR(void)
{
	csl_caph_dma_process_interrupt();

	IRQ_Clear(CAPH_NORM_IRQ);
	IRQ_Enable(CAPH_NORM_IRQ);
    return;
}
#endif
/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_Init(void)
*
*  Description: init CAPH HW control driver
*
****************************************************************************/
Result_t AUDDRV_HWControl_Init(void)
{
	UInt32 testAddr1, testAddr2, testAddr3, testAddr4;
    CSL_CAPH_HWCTRL_BASE_ADDR_t addr;
   	//Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_Init:: \n");

	testAddr1 = HUB_CLK_BASE_ADDR;
	testAddr2 = HW_IO_PHYS_TO_VIRT(0x34000000);
	testAddr3 = BSC1_BASE_ADDR;
	printk("AUDDRV_HWControl_Init:: testAddr1 = 0x%x, testAddr2 = 0x%x ,testAddr3 = 0x%x,testAddr4 = 0x%x \n",
		testAddr1, testAddr2, testAddr3, HW_IO_PHYS_TO_VIRT(0x34000000));

#if 0
                                
    //Enable CAPH clock.
    id = PRM_client_register("AUDIO_CAPH_DRV");
    PRM_set_clock_state(id, RESOURCE_CAPH, CLOCK_ON);
    PRM_set_clock_speed(id, RESOURCE_CAPH, 156000000);
#else
    // hard code it.
	UInt32 regVal;
	regVal = (0x00A5A5 << KHUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
    regVal |= KHUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK;
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET),regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET)) = (UInt32)regVal);
	while ( ((*((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET))) & 0x01) == 1) {} 
 
	printk("AUDDRV_HWControl_Init:: OK 1\n");

    /* Set the frequency policy */
    regVal = (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT);
    regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT);
    regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT);
    regVal |= (0x06 << KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET)) = (UInt32)regVal);
 
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)) = (UInt32)0x0000FFFF);

    /* Set the frequency policy */
    regVal = 0x7FFFFFFF;
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET)) = (UInt32)regVal);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET)) = (UInt32)regVal);
 
    /* start the frequency policy */
    regVal = 0x00000003; //(KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK | KHUB_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK);
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET)) = (UInt32)regVal);
	while ( ((*((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET))) & 0x01) == 1) {} 
	
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)) = (UInt32)0x0000FFFF);


	//OSTASK_Sleep(1000);

	printk("AUDDRV_HWControl_Init:: OK 2\n");

	// srcMixer clock
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET)) = (UInt32)0x00000011);
	//while ( ((*((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))) & 0x00100000) == 0x00100000) {}
	
	printk("AUDDRV_HWControl_Init:: OK 3\n");
	
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET)) = (UInt32)0x00100000);
	//while ( ((*((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))) & 0x00100000) == 0x00100000) {}
	
	printk("AUDDRV_HWControl_Init:: OK 4\n");

	/* Enable all the CAPH clocks */
#if 0    
	//regVal = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_CLK_EN_MASK;
    //regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HW_SW_GATING_SEL_MASK;
	//regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_EN_MASK;
    //regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_VAL_MASK;
    //WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET) ,regVal);
#endif
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET)) = (UInt32)0x1030);

	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_OFFSET)) = (UInt32)0x1);

	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET)) = (UInt32)0x1);

	printk("AUDDRV_HWControl_Init:: OK 4\n");

#if 0
    /* Enable all the AUDIOH clocks, 26M, 156M, 2p4M, 6p5M  */
    regVal = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK;
     regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK;
    regVal |= KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK;
	//WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET) ,regVal);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)) = (UInt32)regVal);
#endif
	//( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET)) = (UInt32)0x00100000);
	//( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET)) = (UInt32)0x00000001);

	// lock
	/*
	regVal = (0x00A5A5 << KHUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
	( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET)) = (UInt32)regVal);
	while ( ((*((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))) & 0x00100000) == 0x00100000) {}
	*/


	//( *((volatile UInt32 *)(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)) = (UInt32)0x0000ffaa);
	
	//printk("AUDDRV_HWControl_Init:: OK 6\n");

#endif    

#if 0
	AUDDRV_HISR_HANDLE = OSINTERRUPT_Create( (IEntry_t)&AUDDRV_HISR, HISRNAME_CAPH, IPRIORITY_HIGH, HISRSTACKSIZE_CAPH);
    
    // Register the LISR to the IRQ.
    IRQ_Register(CAPH_NORM_IRQ, AUDDRV_LISR);
    IRQ_Clear(CAPH_NORM_IRQ);  
    IRQ_Enable(CAPH_NORM_IRQ);  
#endif

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
    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_DeInit:: \n");

#if 0
	// this is just for fpga test. in real code may not need this.
	IRQ_Disable(CAPH_NORM_IRQ);
	 if (AUDDRV_HISR_HANDLE )
	   {
	   	OSINTERRUPT_Destroy(AUDDRV_HISR_HANDLE );
        	AUDDRV_HISR_HANDLE  = NULL;
	   }	
#endif	 
    csl_caph_hwctrl_deinit(); 

#if 0
    //Disable CAPH clock.
    PRM_set_clock_state(id, RESOURCE_CAPH, CLOCK_OFF);
    PRM_client_deregister(id);
#else
	// hard code
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
    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_HWControl_EnablePath::  Source: %d, Sink: %d\r\n",
            config.source, config.sink);

    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);
    cslConfig.source = AUDDRV_GetCSLDevice(config.source);
    cslConfig.sink = AUDDRV_GetCSLDevice(config.sink);
    cslConfig.src_sampleRate = config.src_sampleRate;
    cslConfig.snk_sampleRate = config.snk_sampleRate;	
    cslConfig.chnlNum = config.chnlNum;
    cslConfig.bitPerSample = config.bitPerSample;

    cslPathID = csl_caph_hwctrl_EnablePath(cslConfig); 
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
    Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "AUDDRV_HWControl_DisablePath:: streamID: %d\r\n",
                    config.streamID);
    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);

    return csl_caph_hwctrl_DisablePath(cslConfig);
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
    Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "AUDDRV_HWControl_PausePath:: streamID: %d\r\n",
                    config.streamID);
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
    Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "AUDDRV_HWControl_ResumePath:: streamID: %d\r\n",
                    config.streamID);
    memset(&cslConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    cslConfig.streamID = AUDDRV_GetCSLStreamID(config.streamID);
    cslConfig.pathID = (CSL_CAPH_PathID)(config.pathID);

    return csl_caph_hwctrl_ResumePath(cslConfig);
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
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
Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
                                      UInt32 gainL_mB,
                                      UInt32 gainR_mB)
{
    csl_caph_hwctrl_SetSinkGain((CSL_CAPH_PathID)pathID, 
                                gainL_mB,
                                gainR_mB);

    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetSource(AUDDRV_PathID pathID, 
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
Result_t AUDDRV_HWControl_SetSourceGain(AUDDRV_PathID pathID,
                                        UInt32 gainL_mB,
                                        UInt32 gainR_mB)

{
    csl_caph_hwctrl_SetSourceGain((CSL_CAPH_PathID)pathID,
                                  gainL_mB,
                                  gainR_mB);
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
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_EnableSideTone(void)    
*  
*  Description: Enable Sidetone path
*
****************************************************************************/
Result_t AUDDRV_HWControl_EnableSideTone(void)
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DisableSideTone(void)    
*  
*  Description: Disable Sidetone path
*
****************************************************************************/
Result_t AUDDRV_HWControl_DisableSideTone(void)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name:Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain_mB)    
*  
*  Description: Set the sidetone gain
*
****************************************************************************/
Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain_mB)
{
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

