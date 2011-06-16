//*********************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   msconsts.h
*
*   @brief	This file contains the configuration for task names, priorities,
*			stack sizes, and queue sizes.
*
****************************************************************************/
/**
*   @ingroup    RTOSGroup
*
*
****************************************************************************/
/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "ostask.h"
#include "osinterrupt.h"
*/

#ifndef _INC_MSCONSTS_H_
#define _INC_MSCONSTS_H_

/****************************************************************************
** NOTE: RTOS settings for protocol stack (L1/2/3) tasks are maintained in
		 stack/$(PRJ_STACK)/inc/ms_cfg.h
****************************************************************************/

//*****************
// Task Priorities
//*****************
#define TASKPRI_MMI                     (LOWEST-2)
#define TASKPRI_YMU                     (TASKPRI_MMI-1)
#define TASKPRI_MNCC                    NORMAL
#define TASKPRI_MMREG                   NORMAL
#define TASKPRI_MSC                     NORMAL
#define TASKPRI_DISPLAY                 NORMAL
#define TASKPRI_KEYPAD                  (TASKPRI_MMI-1)
#define TASKPRI_PWM                     NORMAL
#define TASKPRI_PHONEBOOK               NORMAL
#define TASKPRI_MN                      NORMAL
#define TASKPRI_SIMIO                   NORMAL
#define TASKPRI_SIM                     NORMAL
#define TASKPRI_SIMPROC                 NORMAL
#define TASKPRI_SIMAPDU                 NORMAL
#define TASKPRI_SMS                     NORMAL
#define TASKPRI_V24                     (TPriority_t)(NORMAL-1)
#define TASKPRI_MPX                     (TPriority_t)(NORMAL-1)
#define TASKPRI_PCHEXTX					NORMAL
#define TASKPRI_MPXRXDATA               NORMAL
#define TASKPRI_ECDC                    (TPriority_t)(NORMAL-1)
#define TASKPRI_ATC                     NORMAL
#define TASKPRI_PCH                     NORMAL-1
#define TASKPRI_SDLTRACE                LOWEST
#define TASKPRI_LOG                     LOWEST
#define TASKPRI_WAP						(TPriority_t)(LOWEST-1)
#define TASKPRI_RFCAL                   (BELOW_NORMAL + 1)
#define TASKPRI_RFCALL1                 ABOVE_NORMAL
#define TASKPRI_RFCALOUT                (BELOW_NORMAL + 3)
#define TASKPRI_STK                     NORMAL
#define TASKPRI_PPP                     NORMAL
#define TASKPRI_LAYER3                  (TASKPRI_PPP + 1)
#define TASKPRI_DLINK                   NORMAL
#define TASKPRI_RESTART                 BELOW_NORMAL
#define TASKPRI_BKGMGR                  LOWEST-1
#define TASKPRI_I2C                     (TPriority_t)(LOWEST-3)
#define TASKPRI_IRSTACK                 NORMAL-1
#define TASKPRI_DMA         			(TPriority_t)(NORMAL-2)
#define TASKPRI_SPI         			(TPriority_t)(NORMAL-2)
#define TASKPRI_IMAGE					NORMAL+4
#define TASKPRI_AUDIO					NORMAL+4
#define TASKPRI_SBCAUDIO				NORMAL+7  // Above Audio
#define TASKPRI_CAM						NORMAL+3
#define TASKPRI_VIDEO					(NORMAL + 5)
#define TASKPRI_FFSGC					(TASKPRI_LOG + 1)
#define TASKPRI_USB                     (NORMAL-2) //ABOVE_NORMAL + 8 - higher than MPX, but lower than llc
#define TASKPRI_USBMSC					(TASKPRI_MMI-1)
#define TASKPRI_USBIPC					(NORMAL)
#define TASKPRI_OTGUSBIO				(NORMAL-2)
#define TASKPRI__WLAN					(NORMAL+9)
#define TASKPRI_DHD						(NORMAL+8)
#define TASKPRI_DWDOG					(NORMAL+10)
#if defined(WLAN_CXN)
#define TASKPRI_cxnnTask				(NORMAL)
#define TASKPRI_cxnn					(NORMAL)
#define TASKPRI_xsuppTask				(NORMAL)
#endif
#define TASKPRI_termTask				(NORMAL)

#if defined(_TCH_DRV_TSC200X_)
#define TASKPRI_TCH_SCRN				(TPriority_t)(TASKPRI_MMI+1) //Below MMI
#endif
#if defined MOTN_INCLUDED
#define TASKPRI_MOTN_SNSR				(TPriority_t)(TASKPRI_MMI-1)  //cm was LOWEST-3
#endif



#if defined(_BCM2133_)
#define TASKPRI_USB_DEV_RX                   	(NORMAL-2)
#define TASKPRI_USB_DEV_TX                   	(NORMAL-2)
#endif

#define TASKPRI_OUCD					(NORMAL-3)
#define TASKPRI_DISPMAN         		(NORMAL+2)
#define	TASKPRI_VIB						(NORMAL)
#define TASKPRI_HEADSET					(TASKPRI_MMI-1)
#ifdef	ARM_HS_VOCODER
#define TASKPRI_HSCODEC					HIGHEST
#endif
#define TASKPRI_DRM						(TPriority_t)(LOWEST-1)
#define TASKPRI_MTP						(TPriority_t)(NORMAL)
#define TASKPRI_PTP						(TPriority_t)(NORMAL)
#define	TASKPRI_DSPCORE					(HIGHEST + 1)	//same priority as GL1S
#define TASKPRI_HAL_AUDIO					NORMAL+4
#define TASKPRI_GPS						(NORMAL-2)
#define TASKPRI_LCS                     (NORMAL+1)
#define TASKPRI_GPS_RX_CHIP_DATA        (TPriority_t) (TASKPRI_LCS) // lower# = higher priority
#define TASKPRI_ADCMGR					NORMAL

//*************
// Task Names
//*************

#define TASKNAME_FM			  (TName_t)"FM"
#define TASKNAME_MMI                    (TName_t)"MMI"
#define TASKNAME_YMU                    (TName_t)"YMU"
#define TASKNAME_VIB					(TName_t)"VIB"
#define TASKNAME_MNCC                   (TName_t)"MNCC"
#define TASKNAME_MMREG                  (TName_t)"MMREG"
#define TASKNAME_MSC                    (TName_t)"MSC"
#define TASKNAME_DISPLAY                (TName_t)"DISPLAY"
#define TASKNAME_KEYPAD                 (TName_t)"KEYPAD"
#define TASKNAME_PWM					(TName_t)"PWM"
#define TASKNAME_PHONEBOOK              (TName_t)"PHONEBK"
#define TASKNAME_MN                     (TName_t)"MNTASK"
#define TASKNAME_SIMIO                  (TName_t)"SIMIO"
#define TASKNAME_SIM                    (TName_t)"SIM"
#define TASKNAME_SIMPROC				(TName_t)"SIMPROC"
#define TASKNAME_SIMAPDU				(TName_t)"SIMAPDU"
#define TASKNAME_SMS                    (TName_t)"SMS"
#define TASKNAME_MPXRXDATA              (TName_t)"MPXRXDAT"
#define TASKNAME_PCHEXTX          (TName_t)"PCHEXTX"
#define TASKNAME_MPX                    (TName_t)"MPX"
#define TASKNAME_ECDC                   (TName_t)"ECDC"
#define TASKNAME_PCH                    (TName_t)"PCH"
#define TASKNAME_ATC                    (TName_t)"ATC"
#define TASKNAME_SDLTRACE               (TName_t)"SDLTRACE"
#define TASKNAME_LOG                    (TName_t)"LOG"
#define TASKNAME_V24                    (TName_t)"V24"
#define TASKNAME_RFCALL1                (TName_t)"RFCALL1"
#define TASKNAME_RFCAL                  (TName_t)"RFCAL"
#define TASKNAME_RFCALOUT               (TName_t)"RFCALOUT"
#define TASKNAME_STK                    (TName_t)"STK"
#define TASKNAME_PPP                    (TName_t)"PPP"
#define TASKNAME_LAYER3                 (TName_t)"LAYER3"
#define TASKNAME_DLINK                  (TName_t)"DLINK"
#define TASKNAME_RESTART                (TName_t)"PPPRST"
#define TASKNAME_BKGMGR                 (TName_t)"BKGMGR"
#define TASKNAME_I2C                    (TName_t)"I2C"
#define TASKNAME_IRSTACK                (TName_t)"IRSTACK"
#define TASKNAME_DMA        			(TName_t)"DMA_T"
#define TASKNAME_SPI        			(TName_t)"SPI_T"
#define TASKNAME_IMAGE					(TName_t)"IMAGE"
#define TASKNAME_AUDIO					(TName_t)"AUDIO"
#define TASKNAME_SBCAUDIO					(TName_t)"SBCAUDIO"
#define TASKNAME_CAM					(TName_t)"CAMERA"
#define TASKNAME_VIDEO					(TName_t)"VIDEO"
#define TASKNAME_WAP					(TName_t)"BROWSER"
#define TASKNAME_FFSGC					(TName_t)"FFSGC"
#define TASKNAME_USBTX                  (TName_t)"USB_TX"
#define TASKNAME_USBMSC                 (TName_t)"USB_MSC"
#define TASKNAME_USBIPC                 (TName_t)"USB_IPC"
#define TASKNAME_USBHOSTMSC         (TName_t)"USB_HOSTMSC"
#define TASKNAME_USBMTP                 (TName_t)"USB_MTP"
#define TASKNAME_USBPTP                 (TName_t)"USB_PTP"
#define TASKNAME_OTGUSBIO				(TName_t)"OTGUSBIO"
#if defined TCH_INCLUDED 
#define TASKNAME_TCH_SCRN				(TName_t)"TCH_SCRN"
#endif
#if defined MOTN_INCLUDED
#define TASKNAME_MOTN_SNSR				(TName_t)"MOTN_SNSR"
#endif


#if defined(_BCM2133_)
#define TASKNAME_USB_DEV_RX            	(TName_t)"DUSB_RX"
#define TASKNAME_USB_DEV_TX                 	(TName_t)"DUSB_TX"
#endif

#define TASKNAME_OUCD					(TName_t)"OTGUCD"
#define TASKNAME_DISPMAN        		(TName_t)"DISPMAN_T"
#define TASKNAME_HEADSET				(TName_t)"HEADSET"
#ifdef	ARM_HS_VOCODER
#define TASKNAME_HSCODEC				(TName_t)"HSCODEC"
#endif
#define TASKNAME_DRM					(TName_t)"DRM"
#define	TASKNAME_DSPCORE				(TName_t)"DSPCORE"
#define TASKNAME_HAL_AUDIO                    (TName_t)"HAL_AUDIO"
#define TASKNAME_GPS					(TName_t)"GL_SPI"
#define TASKNAME_LCS					(TName_t)"LCS" 
#define TASKNAME_GPS_RX_CHIP_DATA       (TName_t)"GPS_RX"
#define TASKNAME_WLAN					(TName_t)"WLAN"
#define TASKNAME_DHD					(TName_t)"DHD"
#define TASKNAME_DWDOG					(TName_t)"WDOG" 
#define HISRNAME_GPT0     (IName_t)"GPT0"
#define HISRNAME_GPT1     (IName_t)"GPT1"
#define TASKNAME_ADCMGR		(TName_t)"ADC"


//*************
// Task Stacks
//*************
#define	STACKSIZE_BASIC					512
#define STACKSIZE_FM                    (STACKSIZE_BASIC + 2048)
#define STACKSIZE_MMI                   (STACKSIZE_BASIC + (2048+1024))
#define STACKSIZE_YMU                   (STACKSIZE_BASIC + 1536)
#define STACKSIZE_VIB					(STACKSIZE_BASIC + 512)
#define STACKSIZE_MSC                   (STACKSIZE_BASIC + 4096)
#define STACKSIZE_DISPLAY               (STACKSIZE_BASIC + 1024)
#define STACKSIZE_KEYPAD                (STACKSIZE_BASIC + 1024)
#define STACKSIZE_PWM                   (STACKSIZE_BASIC + 1024)
#define STACKSIZE_PHONEBOOK             (STACKSIZE_BASIC + 2048)
#define STACKSIZE_MN                    (STACKSIZE_BASIC + 2048) //1536 Till fine tuning
#define STACKSIZE_SIMIO                 (STACKSIZE_BASIC + 8192) //300
#define STACKSIZE_SIM                   (STACKSIZE_BASIC + 1280) //512
#define STACKSIZE_SIMPROC				(STACKSIZE_BASIC + 2048)
#define STACKSIZE_SIMAPDU				(STACKSIZE_BASIC + 1024)
#define STACKSIZE_SMS                   (STACKSIZE_BASIC + 1536)
#define STACKSIZE_MPXRXDATA             (STACKSIZE_BASIC + 3072)
#define STACKSIZE_PCHEXTX               (STACKSIZE_BASIC + 1280)
#define STACKSIZE_MPX                   (STACKSIZE_BASIC + 1280)
#define STACKSIZE_ATC                   (STACKSIZE_BASIC + 4096) //MobC00061555, integrated from MobC00061264, CSP204359, Hui Luo, 1/22/09
#define STACKSIZE_SDLTRACE              (STACKSIZE_BASIC + 1024)    //512
#define STACKSIZE_LOG                   (STACKSIZE_BASIC + 5120)
#ifdef FUSE_APPS_MODEM
#define STACKSIZE_V24                   (STACKSIZE_BASIC + STACKSIZE_MSC*2)    //increse from 2048 for solving overflow
#else
#define STACKSIZE_V24                   (STACKSIZE_BASIC + 4096)    //increse from 2048 for solving overflow
#endif
#define STACKSIZE_ECDC                  (STACKSIZE_BASIC + 2048)
#define STACKSIZE_PCH                   (STACKSIZE_BASIC + 2048)
#define STACKSIZE_RFCALL1               (STACKSIZE_BASIC + 1024)
#define STACKSIZE_RFCAL                 (STACKSIZE_BASIC + 2048)
#define STACKSIZE_RFCALOUT              (STACKSIZE_BASIC + 1024)
#define STACKSIZE_STK                   (STACKSIZE_BASIC + 2048) //1536
#ifdef FUSE_APPS_MODEM
#define STACKSIZE_PPP                   (STACKSIZE_BASIC + STACKSIZE_MSC*2)
#else
#define STACKSIZE_PPP                   (STACKSIZE_BASIC + 3072)    //jdn was 3072, temp. change to 2048 for ES3
#endif
#define STACKSIZE_LAYER3                (STACKSIZE_BASIC + 4096)
#ifdef MULTI_PDP
#ifdef FUSE_APPS_MODEM
#define STACKSIZE_DLINK                 (STACKSIZE_BASIC + STACKSIZE_MSC*2)
#else
#define STACKSIZE_DLINK                 (STACKSIZE_BASIC + 1280)    //qh, was 1024
#endif
#endif
#define STACKSIZE_RESTART               (STACKSIZE_BASIC + 512)
#define STACKSIZE_BKGMGR                (STACKSIZE_BASIC + 2048)    //512
#define STACKSIZE_I2C                   (STACKSIZE_BASIC + 2048)
#define STACKSIZE_IRSTACK               (STACKSIZE_BASIC + (4096*2))
#define STACKSIZE_DMA       			(STACKSIZE_BASIC + 1200)
#define STACKSIZE_SPI       			(STACKSIZE_BASIC + 512)
#define STACKSIZE_IMAGE					(STACKSIZE_BASIC + 1024)
#define STACKSIZE_AUDIO					(STACKSIZE_BASIC + 2048)
#define STACKSIZE_CAM					(STACKSIZE_BASIC + 4096)
#define STACKSIZE_VIDEO       			(STACKSIZE_BASIC + 4096)
#define STACKSIZE_WAP					(STACKSIZE_BASIC + 4096*3)
#define STACKSIZE_FFSGC					(STACKSIZE_BASIC + 1024)
#define STACKSIZE_USBSTACK              (STACKSIZE_BASIC + 1024)
#define STACKSIZE_USBMSC                (STACKSIZE_BASIC + 1536)
#define STACKSIZE_USBIPC                (STACKSIZE_BASIC + 1024*4)
#define STACKSIZE_USBHOSTMSC            (STACKSIZE_BASIC + 1536)
#define STACKSIZE_USBMTP                (STACKSIZE_BASIC + 1536)
#define STACKSIZE_USBPTPDRIVER          (STACKSIZE_BASIC + 32768)
#define STACKSIZE_USBPTP                (STACKSIZE_BASIC + 1536)
#define STACKSIZE_OTGUSBIO				(STACKSIZE_BASIC + 3072)
#define STACKSIZE_WLAN					(STACKSIZE_BASIC + 4096)
#define STACKSIZE_DHD					4096
#define STACKSIZE_WDOG					4096
#if defined(WLAN_CXN)
#define STACKSIZE_cxnnTask				4096*2
#define STACKSIZE_cxnn					4096*4
#define STACKSIZE_xsuppTask				4096*2
#endif
#define STACKSIZE_termTask 			512
#if defined(_TCH_DRV_TSC200X_)
#define STACKSIZE_TCH_SCRN				(STACKSIZE_BASIC + STACKSIZE_I2C*2) //cm was i2c size
#endif
#if defined(_MOTN_DRV_SMB380_)
#define STACKSIZE_MOTN_SNSR				(STACKSIZE_BASIC + STACKSIZE_I2C*2)
#endif




#if defined(_BCM2133_)
#define STACKSIZE_USB_DEV_RX_STACK     (STACKSIZE_BASIC + 1024)
#define STACKSIZE_USB_DEV_TX_STACK     (STACKSIZE_BASIC + 1024)
#endif

#define STACKSIZE_OUCD					(STACKSIZE_BASIC + 2048)
#define STACKSIZE_DISPMAN       		(STACKSIZE_BASIC + 2048)
#define STACKSIZE_VIB					(STACKSIZE_BASIC + 512)
#define STACKSIZE_HEADSET				(STACKSIZE_BASIC + 1024)
#ifdef	ARM_HS_VOCODER
#define STACKSIZE_HSCODEC				(STACKSIZE_BASIC + 2048)
#endif
#define STACKSIZE_DRM					(STACKSIZE_BASIC + 1024)
#define		STACKSIZE_DSPCORE			(STACKSIZE_BASIC + 2048)
#define STACKSIZE_HAL_AUDIO				(STACKSIZE_BASIC + 4096)
#define STACKSIZE_GPS					(STACKSIZE_BASIC + 4096)
#define STACKSIZE_LCS                   (STACKSIZE_BASIC + 4096)
#define STACKSIZE_GPS_RX_CHIP_DATA      (STACKSIZE_BASIC + 4096)
#define STACKSIZE_ADCMGR				5120

//*************************
// Number of Queue Entries
//*************************
#define QUEUESIZE_MMI                   150
#define QUEUESIZE_DISPLAY               20
#define QUEUESIZE_MNCC                  10
#define QUEUESIZE_MMREG                 10
#define QUEUESIZE_MSC                   70
#define QUEUESIZE_PHONEBOOK             10
#define QUEUESIZE_MN                    20
#define QUEUESIZE_SIM                   10
#define QUEUESIZE_SIMPROC               15
#define QUEUESIZE_SIMAPDU               5
#define QUEUESIZE_SMS                   10
#define QUEUESIZE_MPX                   350
#define QUEUESIZE_PCHEXTX		 350
#define QUEUESIZE_ATC_MPX               250
#define QUEUESIZE_FC_MPX               200
#define QUEUESIZE_ATC                   20
#define QUEUESIZE_ECDC                  10
#define QUEUESIZE_PCH                   10
#define QUEUESIZE_SDLTRACE              50
#define QUEUESIZE_LOG                   200
#define QUEUESIZE_RFCALL1               20
#define QUEUESIZE_RFCAL                 20
#define QUEUESIZE_RFCALOUT              20
#define QUEUESIZE_STK                   20
#define QUEUESIZE_PPP                   40
#define QUEUESIZE_LAYER3                64
#define QUEUESIZE_DLINK                 10
#define QUEUESIZE_RESTART               10
#define QUEUESIZE_I2C                   100
#define QUEUESIZE_IMAGE					16
#define QUEUESIZE_AUDIO					16
#define QUEUESIZE_CAM					16
#define QUEUESIZE_VIDEO                 16
#define QUEUESIZE_WAP					20
#define QUEUESIZE_OTGUSBIO				30
#define QUEUESIZE_VIB					30
#define QUEUESIZE_USBMSC				16
#define QUEUESIZE_USBIPC				16
#define QUEUESIZE_USBHOSTMSC			16
#define QUEUESIZE_USBMTP				16
#define QUEUESIZE_USBPTP				16
#define QUEUESIZE_DRM					20
#define QUEUESIZE_HAL_AUDIO             50
#define QUEUESIZE_PWM                   10
#define QUEUESIZE_GPS					20
#define QUEUESIZE_SPI					20
#define QUEUESIZE_LCS					100 //This queue is used by LCS task. Increased from 50 for CQ48224.
#define QUEUESIZE_GPS_RX_CHIP_DATA		400 //This queue is used by GPS RxData task.


#ifdef	ARM_HS_VOCODER
#define QUEUESIZE_HSCODEC				6
#endif

#define		QUEUESIZE_DSPCORE	64

//*************************
// Interrupt HISR related Constants
//*************************

#define HISRNAME_CRGI           (IName_t)"CRGI_H"
#define HISRNAME_KPD            (IName_t)"KPD_H"
#define HISRNAME_RIP            (IName_t)"RIP_H"
#define HISRNAME_FIQ            (IName_t)"FIQ_H"    //yongqian
#define HISRNAME_SIMIO          (IName_t)"SIMIO_H"
#define HISRNAME_SPI            (IName_t)"SPI_H"
#define HISRNAME_PORTA1         (IName_t)"PORTA1_H"
#define HISRNAME_PORTA2_MODEM   (IName_t)"PORTA2_H"
#define HISRNAME_PORTA3         (IName_t)"PORTA3_H"
#define HISRNAME_PORTB1         (IName_t)"PORTB1_H"
#define HISRNAME_PORTB2_MODEM   (IName_t)"PORTB2_H"
#define HISRNAME_PORTB3         (IName_t)"PORTB3_H"
#define HISRNAME_PORTC1         (IName_t)"PORTC1_H"
#define HISRNAME_PORTC2_MODEM   (IName_t)"PORTC2_H"
#define HISRNAME_PORTC3         (IName_t)"PORTC3_H"
#define HISRNAME_IRFRMRX        (IName_t)"IRFRMRX_H"
#define HISRNAME_IRFRMTX        (IName_t)"IRFRMTX_H"
#define HISRNAME_YMU            (IName_t)"YMU_H"
#define HISRNAME_PMU            (IName_t)"PMU_H"
#define HISRNAME_PMU_BATREM     (IName_t)"PMUBATREM_H"
#define HISRNAME_DMA			(IName_t)"DMA_H"
#define HISRNAME_AUDVOC			(IName_t)"AUDVOC_H"
#define HISRNAME_VSYNC          (IName_t)"VSYNC_H"
#define HISRNAME_WDT            (IName_t)"WDT_H"
#define HISRNAME_I2C			(IName_t)"I2C_H"
#define HISRNAME_I2S			(IName_t)"I2S_H"
#define HISRNAME_OTGUSB			(IName_t)"OTGUSB_H"
#define HISRNAME_HEADSET		(IName_t)"HEADSET_H"
#define HISRNAME_BUTTON			(IName_t)"BUTTON_H"
#define HISRNAME_JPEG				(IName_t)"JPEG_H"
#define HISRNAME_SDIO0			(IName_t)"SDIO0_H"
#define HISRNAME_SDIO1			(IName_t)"SDIO1_H"
#define HISRNAME_SDIO2			(IName_t)"SDIO2_H"
#define HISRNAME_GE				(IName_t)"GE_H"	
#define HISRNAME_GPSINT			(IName_t)"GPSINT_H"
#define HISRNAME_LOG			(IName_t)"LOG_H"

#if defined(_TCH_DRV_TSC200X_)
#define HISRNAME_TCH_SCRN		(IName_t)"HTSCRN"
#endif
#if defined(_MOTN_DRV_SMB380_)
#define HISRNAME_MOTN_SNSR		(IName_t)"MOTNSNSR_H"
#endif

#define RESERVED_STACK_FOR_LISR     256    //18 registers plus stack space for trace
#define HISRSTACKSIZE_CRGI          (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_KPD           (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_RIP           (2560 + RESERVED_STACK_FOR_LISR)  // Was 1424.  Increased to support DSP_FEATURE_TBF_FASTCMD in RIP_HISR
#define HISRSTACKSIZE_FIQ           (800 + RESERVED_STACK_FOR_LISR)   //yongqian FIQ stacksize
#define HISRSTACKSIZE_SIMIO         (512 + RESERVED_STACK_FOR_LISR) // Doubled due to stack overflow
#define HISRSTACKSIZE_SPI           (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTA1        (640 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTA2_MODEM  (512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTA3        (384 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTB1        (320 + RESERVED_STACK_FOR_LISR) // To support assertion, need larger stack
#define HISRSTACKSIZE_PORTB2_MODEM  (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTB3        (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTC1        (640 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTC2_MODEM  (512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PORTC3        (256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_IRFRMRX       (1024 + RESERVED_STACK_FOR_LISR) //256   // 1024 (working one)
#define HISRSTACKSIZE_IRFRMTX       (1024 + RESERVED_STACK_FOR_LISR) //256   // 1024 (working one)
#define HISRSTACKSIZE_YMU           (512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_PMU           (512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_DMA			(512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_VSYNC         (512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_WDT			(1024 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_I2C			(256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_I2S			(256 + RESERVED_STACK_FOR_LISR)
//#define HISRSTACKSIZE_OTGUSB		(512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_OTGUSB		(4096)
#define HISRSTACKSIZE_JPEG			(1024 + RESERVED_STACK_FOR_LISR)

#define	HISRSTACKSIZE_HEADSET		(512 + RESERVED_STACK_FOR_LISR)
#define	HISRSTACKSIZE_BUTTON		(512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_GE			(256 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_SDIO0			(2048 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_SDIO1			(512 + RESERVED_STACK_FOR_LISR)
#define HISRSTACKSIZE_SDIO2 		(512 + RESERVED_STACK_FOR_LISR)	
#define HISRSTACKSIZE_GPSINT		(512 + RESERVED_STACK_FOR_LISR)	
#define HISRSTACKSIZE_LOG			512	


//*************************
// Logging related Constants
//*************************

#define TRACE_LOG_STACK_SZ          256      // extra space on stack for tracing
											// set to 0 for production
#define TRACE_MEM_SIZE				131072	// 128k

#define LOG_TX_SIZE					131072	// 128k

#endif

