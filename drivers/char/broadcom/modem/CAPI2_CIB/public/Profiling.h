//***************************************************************************
//
//	Copyright © 2007-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   Profiling.h
*
*   @brief  This file contains the macros for Profiling on the ARM9 plus ARM11.
*
*   @note	Not all tasks names correspond to that in FUSE project on ARM11
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/

/*-----------------------------------------------------------------------------------------------*\
 *                                          typedefs
\*-----------------------------------------------------------------------------------------------*/
typedef unsigned int (*fConvertMasterTime)(unsigned int);


typedef struct{
	unsigned int nBckgLastExitTime;
	unsigned int nFrameStart;
	unsigned int nBckgFrameTotalTime;
	fConvertMasterTime fpConvertMasterTime;
	unsigned char	nCPULoadPercent;
} 	strProfilingCPULoad;

/*-----------------------------------------------------------------------------------------------*\
 *                                          Generic Macros
\*-----------------------------------------------------------------------------------------------*/

/* For CPU Load calculations */
/* We sample @ 1/100 slot, which is equivalent to 102 samples */
#define MASTER_TIMER_SAMPLES_PER_SLOT	10240
#define PROFILING_RESOLUTION_DIVISOR	100
#define PROFILING_MASTER_TIME_SAMPLES_PER_SLOT_SAMPLE	(MASTER_TIMER_SAMPLES_PER_SLOT/PROFILING_RESOLUTION_DIVISOR)
#define UMTS_SLOTS_PER_FRAME			15

#define SAMPLE_NUMBER_MASK       0x3FFF
#define SLOT_NUMBER_MASK          0xF0000
#define FRAME_NUMBER_MASK 0xFFF00000

#define ADDR_PMASTER_CURRENT_TIME  0x08169F40


#define ARM_SLEEP	0x41524D53
#define ARM_PLUS_WCDMA_CORE_SLEEP 0x41574453

#ifdef __BIG_ENDIAN
#define TARGET_NAME_BACKGROUND_TASK	0x42636B67
#define STRING_IQ0_                                0x69713000

#define TAG_FRAME_START			0x46524D53

/* Irq Names (in hex) */
#define NAME_UmtsFwEarlyWakeup        0x694C3253
#define NAME_UmtsFwGpioLatchMt        0x69477069
#define NAME_UmtsFwUart0              0x69556172
#define NAME_RxSlotStrt_Isr           0x69527853
#define NAME_MTimerGp0_Isr        		0x694F5430
#define NAME_RxComb0_Isr        			0x69527844
#define NAME_RxComb1_Isr        			0x69527843
#define NAME_UmtsFwTxDcDcCtrl     		0x69547844
#define NAME_UmtsFwRfLoad  				    0x6952664C
#define NAME_TxSlotStrt_Isr        		0x69547853
#define NAME_SearcherSlotStrt_Isr     0x69536561
#define NAME_UmtsFwTxTrchEvent		    0x69547854
#define NAME_RxTrchFin_Isr        		0x69527854
#define NAME_UmtsFwSleepEnd     			0x69536C65
#define NAME_UmtsFwClTxd            	0x694D5431
//#define NAME_Timer_Isr        				0x6954696D
#define NAME_UmtsFwStopAbortMeas   	0x694C3230
//#define NAME_L2SoftInt1_Isr        			0x694C3231
//#define NAME_SwInt0_Isr        			0x69537730
//#define NAME_SwInt1_Isr        			0x69537731
#define NAME_UmtsFwProcessDlTpc		0x69547850
#define NAME_UmtsFwDlPwrCtrl    			0x69527850
#define NAME_UmtsFwRouterErrorCheck	0x69466971
#define NAME_UmtsFwHsScchSlot1Launch	0x69485331
#define NAME_UmtsFwHsScchSlot23Launch	0x69483233
#define NAME_UmtsFwHsdpaTxTimerLaunch 0x69545458
#define NAME_UmtsFwHsdpaDmaStart 		0x69444D53
#define NAME_UmtsFwHsdpaDmaFinish		0x69444D46
#define NAME_HsupaDl_Isr				0x69555041


#define NAME_FIQ_FINISHED				0x46495146

#define IRQ_NAME(name)       NAME##name


/* BackGroundWaitSamples for RSSI measurements */
#define BACKGROUND_WAIT_SAMPLES_START		0x42575353
#define BACKGROUND_WAIT_SAMPLES_FINISH		0x42575346

#ifdef UMTS
#define PS_NAME_l1u1	0x6C317531
#define PS_NAME_ubmc	0x75626D63
#define PS_NAME_umcc	0x756D6363
#define PS_NAME_umcd	0x756D6364
#define PS_NAME_umcu	0x756D6375
#define PS_NAME_urab	0x75726162
#define PS_NAME_urcc	0x75726363
#define PS_NAME_urcd	0x75726364
#define PS_NAME_urcl	0x7572636C
#define PS_NAME_urcb	0x75726362
#define PS_NAME_urcd	0x75726364
#define PS_NAME_urcm	0x7572636D
#else
#define PS_NAME_l1u1	0x6C317531
#define PS_NAME_ubmc	0x75626D63
#define PS_NAME_umcc	0x756D6363
#define PS_NAME_umcd	0x756D6364
#define PS_NAME_umcu	0x756D6375
#define PS_NAME_urab	0x75726162
#define PS_NAME_urcc	0x75726363
#define PS_NAME_urcd	0x75726364
#define PS_NAME_urcl	0x7572636C
#define PS_NAME_urcb	0x75726362
#define PS_NAME_urcd	0x75726364
#define PS_NAME_urcm	0x7572636D
#endif
#define PS_NAME_dll1	0x646C6C31
#define PS_NAME_dll2	0x646C6C32
#define PS_NAME_gmm1	0x676D6D31
#define PS_NAME_gmr1	0x676D7231
#define PS_NAME_grr1	0x67727231
#define PS_NAME_llc1		0x6C6C6331
#define PS_NAME_mac1	0x6D616331
#define PS_NAME_mma1	0x6D6D6131
#define PS_NAME_mmc1	0x6D6D6331
#define PS_NAME_mme1	0x6D6D6531
#define PS_NAME_mmr1	0x6D6D7231
#define PS_NAME_rlc1	0x726C6331
#define PS_NAME_rrc1	0x72726331
#define PS_NAME_smr1	0x736D7231
#define PS_NAME_snp1	0x736E7031
#define PS_NAME_gl1s	0x676C3173
#define PS_NAME_mmre	0x6D6D7265
#define PS_NAME_mncc	0x6D6E6363
#define PS_NAME_mns1	0x6D6E7331
#define PS_NAME_mnss	0x6D6E7373
#define PS_NAME_oms1	0x6F6D7331
#define PS_NAME_sim1	0x73696D31
#define PS_NAME_dch1	0x64636831
#define PS_NAME_dtt1	0x64747431
#define PS_NAME_dtn1	0x64746E31
#define PS_NAME_drl1	0x64726C31 
#define PS_NAME_df21	0x64663231

/* Other Protocol Stack "Platform" tasks */
#define PS_NAME_v24	0x56323400
#define PS_NAME_SYST	0x53595354
#define PS_NAME_MPX	0x4d505800
#define PS_NAME_MPXR	0x4d505852
#define PS_NAME_MNTA	0x4d4e5441
#define PS_NAME_SIM	0x53494d00
#define PS_NAME_SIMI	0x53494d49
#define PS_NAME_PHON	0x50484f4e
#define PS_NAME_SMS	0x534d5300
#define PS_NAME_DLIN	0x444c494e
#define PS_NAME_LAYE	0x4c415945
#define PS_NAME_BKGM	0x424b474d
#define PS_NAME_SDLT	0x53444c54
#define PS_NAME_SIMP	0x53494d50
#define PS_NAME_SIMTM	0x53494d54
#define PS_NAME_DMA_T	0x444D415F
#define PS_NAME_LOG	0x4C4F4700
#define PS_NAME_MSC	0x4D534300
#define PS_NAME_I2C		0x49324300
#define PS_NAME_ECDC	0x45434443
#define PS_NAME_SIMPROC	0x53494D50
#define PS_NAME_ATC	0x41544300
#define PS_NAME_STK	0x53544B00
#define PS_NAME_PPP	0x50505000
#define PS_NAME_agps     0x61677073
#define PS_NAME_CAPI2   0x43415049
#define PS_NAME_CP2D    0x43503244
#define PS_NAME_AtiS      0x41746953
#define PS_NAME_AUDIO  0x41554449

/* HISR */
#define PS_NAME_HCRG	0x48435247
#define PS_NAME_HKPD	0x484B4B50
#define PS_NAME_HRIP	0x5249505f
#define PS_NAME_HFIQ	0x48464951
#define PS_NAME_HSMI	0x48534D49
#define PS_NAME_HSPI	0x48535049
#define PS_NAME_HPA1	0x48504131
#define PS_NAME_HPA2	0x48504132
#define PS_NAME_HPA3	0x48504133
#define PS_NAME_HPB1	0x48504231
#define PS_NAME_HPB2	0x48504232
#define PS_NAME_HPB3	0x48504233
#define PS_NAME_HPC1	0x48504331
#define PS_NAME_HPC2	0x48504332
#define PS_NAME_HPC3	0x48504333
#define PS_NAME_HMRX	0x484D5258
#define PS_NAME_HMTX	0x484D5458
#define PS_NAME_HYMU	0x48594D55
#define PS_NAME_HPMU	0x48504D55
#define PS_NAME_HDMA	0x48444D41
#define PS_NAME_HVSY	0x48565359
#define PS_NAME_HWDT	0x48574454
#define PS_NAME_HI2C	0x48493243
#define PS_NAME_HI2S	0x48493253
#define PS_NAME_HOTG	0x484F5447
#define PS_NAME_HHEA	0x48484541
#define PS_NAME_HBUT	0x48425554
#define PS_NAME_HP1B	0x53595354
#define PS_NAME_CIPHER 0x43495048
#define PS_NAME_HIPC	0x48495043

#define PS_NAME_MNTASK	0x4D4E5441
#define PS_NAME_uSvrTask	0x75537672
#define PS_NAME_USB_MSC	0x5553425F
#define PS_NAME_sme		0x736D6500
#define PS_NAME_SDHWDECT 	0x53444857
#define PS_NAME_HEADSET	0x48454144
#define PS_NAME_HHEA		0x48484541
#define PS_NAME_HBUT		0x48425554
#define PS_NAME_AfcA		0x41666341
#define PS_NAME_DSPCore	0x44535043
#define PS_NAME_cmc	       0x636D6300
#define PS_NAME_HUCM       0x4855434D

#define PS_NAME(NAME) PS_NAME_##NAME

#define HISR_NAME_HS_SCCH_SLOT23	0x48533233
#define HISR_NAME_HS_SCCH_SLOT1	0x48534C31
#define HISR_NAME_HSDPA_TX_TIMER	0x48545854

#define HISR_NAME_TXSLOT				0x68545853
#define HISR_NAME_RXSLOT				0x68525853
#define HISR_NAME_SEARCHERSLOT		0x68534552
#define HISR_NAME_REFSLOT				0x68524546
#define HISR_NAME_HSUPADL				0x68555041
#define HISR_NAME_HUCM                  0x68555041

#define NAME_INCOMPLETE				0xFFFFFFFF
#define NAME_HW_L2SemaRequest		0x88555541
#define NAME_HW_L2SemaRelease			0x88555540

#define HISR_NAME_HSDPA_PACKER_HISR 0x5041434B

#else /* Little Endian */
#define TARGET_NAME_BACKGROUND_TASK	0x676B6342
#define STRING_IQ0_                                0x00307169

#define TAG_FRAME_START			0x534D5246 

/* Irq Names (in hex) */
#define NAME_UmtsFwEarlyWakeup        	0x53324C69
#define NAME_UmtsFwGpioLatchMt             	0x69704769
#define NAME_UmtsFwUart0                        	0x72615569
#define NAME_RxSlotStrt_Isr                       	0x53785269
#define NAME_MTimerGp0_Isr        		0x30544F69
#define NAME_RxComb0_Isr        			0x44785269
#define NAME_RxComb1_Isr        			0x43785269
#define NAME_UmtsFwTxDcDcCtrl     		0x44785469
#define NAME_UmtsFwRfLoad  				0x4C665269
#define NAME_TxSlotStrt_Isr        			0x53785469
#define NAME_SearcherSlotStrt_Isr        	0x61655369
#define NAME_UmtsFwTxTrchEvent		0x54785469
#define NAME_RxTrchFin_Isr        			0x54785269
#define NAME_UmtsFwSleepEnd     			0x656C5369
#define NAME_UmtsFwClTxd            		0x31544D69
#define NAME_UmtsFwStopAbortMeas   	0x30324C69
#define NAME_UmtsFwProcessDlTpc		0x50785469
#define NAME_UmtsFwDlPwrCtrl    			0x50785269
#define NAME_UmtsFwRouterErrorCheck	0x71694669
#define NAME_UmtsFwHsScchSlot1Launch	0x31534869
#define NAME_UmtsFwHsScchSlot23Launch	0x33324869
#define NAME_UmtsFwHsdpaTxTimerLaunch	0x58545469
#define NAME_UmtsFwHsdpaDmaStart 			0x534D4469
#define NAME_UmtsFwHsdpaDmaFinish			0x464D4469
#define NAME_HsupaDl_Isr					0x41505569

#define NAME_FIQ_FINISHED				0x46514946

#define IRQ_NAME(name)       NAME##name

#define PS_NAME(NAME) PS_NAME_##NAME

#define HISR_NAME_HS_SCCH_SLOT23	0x33325348
#define HISR_NAME_HS_SCCH_SLOT1	0x314C5348
#define HISR_NAME_HSDPA_TX_TIMER	0x54585448

#define HISR_NAME_TXSLOT				0x53585468
#define HISR_NAME_RXSLOT				0x53585268
#define HISR_NAME_SEARCHERSLOT		0x52455368
#define HISR_NAME_REFSLOT				0x46455268
#define HISR_NAME_HSUPADL				0x41505568
#endif

/*-----------------------------------------------------------------------------------------------*\
 *                                          Macros
\*-----------------------------------------------------------------------------------------------*/
#define PROFILE_FIQ(Tag)         \
	{\
	*ptrProfilingBuffer = Tag;\
          ptrProfilingBuffer += 1;\
          *ptrProfilingBuffer = *(unsigned int *)(ADDR_PMASTER_CURRENT_TIME);\
          ptrProfilingBuffer += 1;\
          if(ptrProfilingBuffer >= ptrProfilingBufferLast)\
				ptrProfilingBuffer = (unsigned *)&ProfilingBuffer;\
	}

#define PROTECTED_PROFILE(Tag)      {\
	unsigned CPSR_saved  = __DisableInterrupts();\
	 *ptrProfilingBuffer = Tag;\
        ptrProfilingBuffer += 1;\
          *ptrProfilingBuffer = *(unsigned int *)(ADDR_PMASTER_CURRENT_TIME);\
          ptrProfilingBuffer += 1;\
          if(ptrProfilingBuffer >= ptrProfilingBufferLast)\
				ptrProfilingBuffer = (unsigned int *)&ProfilingBuffer;\
	__RestoreCpsr(CPSR_saved);\
		}

#define PROFILE_IRQ(x)	PROTECTED_PROFILE(x)



