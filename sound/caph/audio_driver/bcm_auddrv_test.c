/****************************************************************************/
/*     Copyright 2009-2012 Broadcom Corporation.  All rights reserved.      */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*   @file   bcm_auddrv_test.c
*
*   @brief	This file contains SysFS interface for audio driver test cases
*
****************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <linux/sysfs.h>

/* #include <linux/broadcom/hw_cfg.h> */
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <mach/hardware.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "bcm_fuse_sysparm_CIB.h"
#include <plat/chal/chal_types.h>
#include "audio_trace.h"

#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "voif_handler.h"
#include "audctrl_policy.h"

#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

#include <plat/osabstract/ossemaphore.h>
#include "msconsts.h"
#include "csl_aud_queue.h"
#include "csl_vpu.h"
#include "csl_arm2sp.h"
#ifdef CONFIG_ARM2SP_PLAYBACK
#include "audio_vdriver_voice_play.h"
#endif


static UInt8 *samplePCM16_inaudiotest;
static UInt16 *record_test_buf;

UInt8 playback_audiotest[] = { 0 };
UInt8 playback_audiotest_srcmixer[] = { 0 };

#define USE_LOOPBACK_SYSPARM

#define BRCM_AUDDRV_NAME_MAX (15)	/* max 15 char for test name */
#define BRCM_AUDDRV_TESTVAL  (5)	/* max no of arg for each test */

#define	PCM_TEST_MAX_PLAYBACK_BUF_BYTES		(100*1024)
#define	PCM_TEST_MAX_CAPTURE_BUF_BYTES		(100*1024)
#define TEST_BUF_SIZE   (512 * 1024)

static int sgBrcm_auddrv_TestValues[BRCM_AUDDRV_TESTVAL];
static char *sgBrcm_auddrv_TestName[] = { "Aud_play",
		"Aud_Rec", "Aud_control" };

/* SysFS interface to test the Audio driver level API */
static ssize_t Brcm_auddrv_TestSysfs_show(struct device *dev,
				   struct device_attribute *attr, char *buf);
static ssize_t Brcm_auddrv_TestSysfs_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count);
static struct device_attribute Brcm_auddrv_Test_attrib =
__ATTR(BrcmAud_DrvTest, 0644, Brcm_auddrv_TestSysfs_show,
	Brcm_auddrv_TestSysfs_store);

static int HandleControlCommand(void);

static int HandlePlayCommand(void);
static int HandleCaptCommand(void);
static void AUDIO_DRIVER_TEST_InterruptPeriodCB(void *pPrivate);
static void AUDIO_DRIVER_TEST_CaptInterruptPeriodCB(void *pPrivate);

#ifdef CONFIG_ARM2SP_PLAYBACK_TEST

static void AUDTST_VoicePlayback(UInt32 Val2, UInt32 Val3, UInt32 Val4,
				 UInt32 Val5, UInt32 Val6);

static Boolean AUDDRV_BUFFER_DONE_CB(UInt8 *buf, UInt32 size, UInt32 streamID)
{
	OSSEMAPHORE_Release(AUDDRV_BufDoneSema);
	return TRUE;
}
#endif

static Semaphore_t AUDDRV_BufDoneSema;

static AUDQUE_Queue_t *sVtQueue;

static Semaphore_t sVtQueue_Sema;
static const UInt16 sVoIPDataLen[] = { 0, 322, 160, 38, 166, 642, 70 };

static void AudDrv_VOIP_DumpUL_CB(void *pPrivate, u8 * pSrc, u32 nSize);
static void AudDrv_VOIP_FillDL_CB(void *pPrivate, u8 * pDst, u32 nSize);

/* static UInt8 sVoIPAMRSilenceFrame[1] = {0x000f}; */
static UInt32 delay_count;	/* 20ms each count */

/* callback for buffer ready of pull mode */
static void AudDrv_VOIP_DumpUL_CB(void *pPrivate, u8 * pSrc, u32 nSize)
{
	UInt32 copied = 0;

	copied = AUDQUE_Write(sVtQueue, pSrc, nSize);
	/* OSSEMAPHORE_Release(sVtQueue_Sema); */
}

static void AudDrv_VOIP_FillDL_CB(void *pPrivate, u8 * pDst, u32 nSize)
{
	UInt32 copied = 0;
	if (delay_count > 0) {
		delay_count--;
		memset(pDst, 0, nSize);
	} else {
		copied = AUDQUE_Read(sVtQueue, pDst, nSize);
		/* OSSEMAPHORE_Release(AUDDRV_BufDoneSema); */
	}
}

/* +++++++++++++++++++++++++++++++++++++++
//Brcm_auddrv_TestSysfs_show (struct device *dev,
// struct device_attribute *attr, char *buf)
// Buffer values syntax -
// 0 - on/off, 1 - output device, 2 - sample rate index,
// 3 - channel, 4 -volume,
//
//---------------------------------------------------*/

static ssize_t Brcm_auddrv_TestSysfs_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int i;
	char sbuf[256];

	snprintf(sbuf, sizeof(sbuf), "%s:",
		 sgBrcm_auddrv_TestName[sgBrcm_auddrv_TestValues[0]]);

	for (i = 0; i < (sizeof(sgBrcm_auddrv_TestValues) /
			 sizeof(sgBrcm_auddrv_TestValues[0])); i++) {
		snprintf(sbuf, sizeof(sbuf), "%d", sgBrcm_auddrv_TestValues[i]);
		strcat(buf, sbuf);
	}
	return strlen(buf);
}

/* +++++++++++++++++++++++++++++++++++++++
Brcm_auddrv_TestSysfs_store
(struct device *dev, struct device_attribute *attr, char *buf)
Buffer values syntax -
	0 - on/off, 1 - output device, 2 - sample rate index,
	3 - channel, 4 -volume,
---------------------------------------------------*/

static ssize_t Brcm_auddrv_TestSysfs_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{

	if (5 != sscanf(buf, "%d %d %d %d %d", &sgBrcm_auddrv_TestValues[0],
			&sgBrcm_auddrv_TestValues[1],
			&sgBrcm_auddrv_TestValues[2],
			&sgBrcm_auddrv_TestValues[3],
			&sgBrcm_auddrv_TestValues[4])) {
			aTrace(LOG_AUDIO_DRIVER,
					"\nBcm_AudDrv_test SysFS :type=%s"
			" arg1=%d, arg2=%d, arg3=%d, arg4=%d\n",
			sgBrcm_auddrv_TestName[sgBrcm_auddrv_TestValues[0]-1],
			sgBrcm_auddrv_TestValues[1],
			sgBrcm_auddrv_TestValues[2],
			sgBrcm_auddrv_TestValues[3],
			sgBrcm_auddrv_TestValues[4]);
			aTrace(LOG_AUDIO_DRIVER,
					"error reading buf=%s count=%d\n",
			buf, count);
		return count;
	}

	switch (sgBrcm_auddrv_TestValues[0]) {
	case 1:		/* Aud_play */
		{
			aTrace(LOG_AUDIO_DRIVER,
					"Case 1 (Aud_play): type =%s"
			" arg1=%d, arg2=%d, arg3=%d, arg4=%d\n",
			     sgBrcm_auddrv_TestName[sgBrcm_auddrv_TestValues[0]
						    - 1],
			     sgBrcm_auddrv_TestValues[1],
			     sgBrcm_auddrv_TestValues[2],
			     sgBrcm_auddrv_TestValues[3],
			     sgBrcm_auddrv_TestValues[4]);
			HandlePlayCommand();
			break;
		}
	case 2:		/* Aud_rec */
		{
			aTrace(LOG_AUDIO_DRIVER, "Case 2 (Aud_Rec): type ="
				"%s arg1=%d, arg2=%d, arg3=%d, arg4=%d\n",
			     sgBrcm_auddrv_TestName[sgBrcm_auddrv_TestValues[0]
						    - 1],
			     sgBrcm_auddrv_TestValues[1],
			     sgBrcm_auddrv_TestValues[2],
			     sgBrcm_auddrv_TestValues[3],
			     sgBrcm_auddrv_TestValues[4]);
			HandleCaptCommand();
			break;
		}
	case 3:		/* Aud_control */
		{
			aTrace(LOG_AUDIO_DRIVER, "case 3 (Aud_Ctrl):type ="
				"%s arg1=%d, arg2=%d, arg3=%d, arg4=%d\n",
			     sgBrcm_auddrv_TestName[sgBrcm_auddrv_TestValues[0]
						    - 1],
			     sgBrcm_auddrv_TestValues[1],
			     sgBrcm_auddrv_TestValues[2],
			     sgBrcm_auddrv_TestValues[3],
			     sgBrcm_auddrv_TestValues[4]);

			HandleControlCommand();
			break;
		}
	default:
		aTrace(LOG_AUDIO_DRIVER, " I am in Default case\n");
	}
	return count;
}

int BrcmCreateAuddrv_testSysFs(struct snd_card *card)
{
	int ret = 0;
	/* create sysfs file for Aud Driver test control */
	ret =
	    snd_add_device_sysfs_file(SNDRV_DEVICE_TYPE_CONTROL, card, -1,
				      &Brcm_auddrv_Test_attrib);
	/* aTrace(LOG_AUDIO_DRIVER, "BrcmCreateControlSysFs ret=%d", ret); */
	return ret;
}

static int HandleControlCommand()
{
	AUDIO_SINK_Enum_t spkr;
	AUDIO_SOURCE_Enum_t mic;

	switch (sgBrcm_auddrv_TestValues[1]) {
	case 1:		/* Initialize the audio controller */
		{
			aTrace(LOG_AUDIO_DRIVER, " Audio Controller Init\n");
			AUDCTRL_Init();
			aTrace(LOG_AUDIO_DRIVER,
					" Audio Controller Init Complete\n");
		}
		break;
	case 2:		/* Start Hw loopback */
		{
			Boolean onOff = sgBrcm_auddrv_TestValues[2];
			mic = sgBrcm_auddrv_TestValues[3];
			spkr = sgBrcm_auddrv_TestValues[4];
			aTrace
			    (LOG_AUDIO_DRIVER,
			     " Audio Loopback onOff = %d, from %d to %d\n",
			     onOff, mic, spkr);
			AUDCTRL_SetAudioLoopback(onOff, mic, spkr, 1);
		}
		break;
	case 3:		/* Dump audio registers */
		{
			aTrace(LOG_AUDIO_DRIVER, " Dump registers\n");
			/* dump_audio_registers(); */
			{
				char *MsgBuf = NULL;
				MsgBuf = kmalloc(2408, GFP_KERNEL);
				if (MsgBuf == NULL) {
					aError("kmalloc failed\n");
					return -1;
				}

				csl_caph_ControlHWClock(TRUE);

				sprintf(MsgBuf,
					"0x35026800 =0x%08lx, 0x3502c910 =0x%08lx, 0x3502c990 =0x%08lx, 0x3502c900 =0x%08lx,0x3502cc20 =0x%08lx,0x35025800 =0x%08lx, 0x34000a34 =0x%08lx, 0x340004b0 =0x%08lx, 0x3400000c =0x%08lx, 0x3400047c =0x%08lx, 0x34000a40=0x%08lx\n",
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x35026800))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502c910))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502c990))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502c900))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502cc20))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x35025800))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x34000a34))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x340004b0))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3400000c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3400047c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT
					       (0x34000a40))));

				aTrace(LOG_AUDIO_DRIVER, "%s", MsgBuf);

				sprintf(MsgBuf,
					"0x3502f000 =0x%08lx, 04 =0x%08lx, 08 =0x%08lx, 0c =0x%08lx, 10 =0x%08lx, 14 =0x%08lx, 18 =0x%08lx, 1c =0x%08lx, 20 =0x%08lx, 24 =0x%08lx, 28 =0x%08lx, 2c =0x%08lx, 30 =0x%08lx, 34 =0x%08lx, 38 =0x%08lx, 3c =0x%08lx, 40 =0x%08lx, 44 =0x%08lx, 48 =0x%08lx, 4c =0x%08lx,50 =0x%08lx, 54 =0x%08lx, 58 =0x%08lx, 5c =0x%08lx\n",
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f000))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f004))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f008))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f00c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f010))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f014))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f018))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f01c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f020))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f024))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f028))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f02c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f030))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f034))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f038))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f03c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f040))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f044))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f048))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f04c))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f050))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f054))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(0x3502f058))),
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT
					       (0x3502f05c))));

				aTrace(LOG_AUDIO_DRIVER, "%s", MsgBuf);

				kfree(MsgBuf);
				csl_caph_ControlHWClock(FALSE);
			}
			aTrace(LOG_AUDIO_DRIVER, " Dump registers done\n");
		}
		break;

	case 4:		/* Enable telephony */
		{
			aTrace(LOG_AUDIO_DRIVER, " Enable telephony\n");
			AUDCTRL_EnableTelephony(AUDIO_SOURCE_ANALOG_MAIN,
						AUDIO_SINK_HANDSET);

			aTrace(LOG_AUDIO_DRIVER, " Telephony enabled\n");
		}
		break;
	case 5:		/* Disable telephony */
		{
			aTrace(LOG_AUDIO_DRIVER, " Disable telephony\n");
			AUDCTRL_DisableTelephony();
			aTrace(LOG_AUDIO_DRIVER, " Telephony disabled\n");
		}
		break;
	/* VoIP loopback test */
	case 6:
		{
			/* Val2 - Mic
			   //Val3 - speaker
			   //Val4 - Delay
			   //Val5 - Codectype */

			AUDTST_VoIP(sgBrcm_auddrv_TestValues[2],
				    sgBrcm_auddrv_TestValues[3], 2000,
				    sgBrcm_auddrv_TestValues[4], 0);
		}
		break;

	case 8:		/* peek a register */
		{
			UInt32 regAddr = sgBrcm_auddrv_TestValues[2];
			UInt32 regVal = 0;
			aTrace(LOG_AUDIO_DRIVER,
					" peek a register, 0x%08lx\n", regAddr);
			csl_caph_ControlHWClock(TRUE);
			regVal = *((UInt32 *) (HW_IO_PHYS_TO_VIRT(regAddr)));
			aTrace(LOG_AUDIO_DRIVER, "value = 0x%08lx\n",
					regVal);
			csl_caph_ControlHWClock(FALSE);
		}
		break;

	case 9:		/* poke a register */
		{
			UInt32 regAddr = sgBrcm_auddrv_TestValues[2];
			UInt32 regVal = sgBrcm_auddrv_TestValues[3];
			csl_caph_ControlHWClock(TRUE);
			*((UInt32 *) (HW_IO_PHYS_TO_VIRT(regAddr))) = regVal;
			aTrace(LOG_AUDIO_DRIVER,
					" poke a register, 0x%08lx = 0x%08lx\n",
					regAddr,
					*((UInt32
					   *) (HW_IO_PHYS_TO_VIRT(regAddr))));
			csl_caph_ControlHWClock(FALSE);
		}
		break;

#if !(defined(_SAMOA_))
/* hard code caph clocks, sometimes clock driver is not working well */
	case 10:
		{
			UInt32 regVal;

			aTrace(LOG_AUDIO_DRIVER,
					" hard code caph clock register"
				" for debugging..\n");
			csl_caph_ControlHWClock(TRUE);
			regVal = (0x00A5A5 <<
				  KHUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
			regVal |= KHUB_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK;
/*
		WRITE_REG32((HUB_CLK_BASE_ADDR+
		KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET),regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET))
			 = (UInt32) regVal);
			while (((*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET)))
				& 0x01) == 1) {
				continue;
			}

			/* Set the frequency policy */
			regVal = (0x06 <<
			KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT);
			regVal |= (0x06 <<
			KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT);
			regVal |= (0x06 <<
			KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT);
			regVal |= (0x06 <<
			KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET))
			 = (UInt32) regVal);

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET))
			 = (UInt32) 0x0000FFFF);

			/* Set the frequency policy */
			regVal = 0x7FFFFFFF;
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET))
			 = (UInt32) regVal);
/*
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET) ,regVal);
*/
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET))
			 = (UInt32) regVal);

			/* start the frequency policy */
			/* KHUB_CLK_MGR_REG_POLICY_CTL_GO_MASK
			   | KHUB_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK; */
			regVal = 0x00000003;

			/* WRITE_REG32((HUB_CLK_BASE_ADDR+
		KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET) ,regVal); */

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET))
			 = (UInt32) regVal);
			while (((*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET)))
				& 0x01) == 1) {
				continue;
			}

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET))
			 = (UInt32) 0x0000FFFF);

			/* srcMixer clock */
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET))
			 = (UInt32) 0x00000011);
/*
while ( ((*((UInt32 *)
(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))) & 0x00100000)
== 0x00100000) {}
*/

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))
			 = (UInt32) 0x00100000);
/*
while ( ((*((UInt32 *)
(KONA_HUB_CLK_BASE_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET))) & 0x00100000)
== 0x00100000) {}
*/
			/* Enable all the CAPH clocks */
#if 0
/*
regVal = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_CLK_EN_MASK;
regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HW_SW_GATING_SEL_MASK;
regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_EN_MASK;
regVal |= KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_VAL_MASK;
WRITE_REG32((HUB_CLK_BASE_ADDR+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET) ,regVal);
*/
#endif
			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET))
			 = (UInt32) 0x1030);

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
			KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_OFFSET))
			 = (UInt32) 0x1);

			(*((UInt32 *) (KONA_HUB_CLK_BASE_VA +
				       KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET))
			 = (UInt32) 0x1);

			csl_caph_ControlHWClock(FALSE);
		}
		break;
#endif
#ifndef CONFIG_ENABLE_VOIF
	case 11:
		{
			/* Internal VoIF test */
			Boolean onOff = sgBrcm_auddrv_TestValues[2];
			AudioMode_t audMode = AUDIO_MODE_HANDSET;
			aTrace(LOG_AUDIO_DRIVER, " VoIF test.\n");

			if (onOff) {
				VoIF_SetDelay(sgBrcm_auddrv_TestValues[3]);
				if (sgBrcm_auddrv_TestValues[4] > 0)
					VoIF_SetGain(sgBrcm_auddrv_TestValues
						     [4]);
				audMode = AUDCTRL_GetAudioMode();
				VoIF_init(audMode);
			} else
				VoIF_Deinit();
		}
		break;
#endif

	default:
		aTrace(LOG_AUDIO_DRIVER, " Invalid Control Command\n");
	}
	return 0;
}

static unsigned long current_ipbuffer_index;
static unsigned long dma_buffer_write_index;

static unsigned long period_bytes;
static unsigned long num_blocks;
static AUDIO_DRIVER_BUFFER_t buf_param;
static int HandlePlayCommand()
{

	unsigned long period_ms;

	unsigned long copy_bytes;
	static AUDIO_DRIVER_HANDLE_t drv_handle;
	static AUDIO_DRIVER_CONFIG_t drv_config;
	static dma_addr_t dma_addr;
	static AUDIO_SINK_Enum_t spkr;
	static int src_used;
	char *src;
	char *dest;
	AUDIO_DRIVER_CallBackParams_t cbParams;
	unsigned int testint = 0;

	switch (sgBrcm_auddrv_TestValues[1]) {
	case 1:		/* open the plyabck device */
		{
			aTrace(LOG_AUDIO_DRIVER, " Audio DDRIVER Open\n");
			drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_PLAY_AUDIO);
			aTrace(LOG_AUDIO_DRIVER,
					" Audio DDRIVER Open Complete\n");
		}
		break;
#ifdef CONFIG_BCM_ENABLE_TESTDATA
	case 2:
		{
			src_used = 0;
			if (sgBrcm_auddrv_TestValues[2] == 0) {
				if (sgBrcm_auddrv_TestValues[3] == 0) {
					aTrace(LOG_AUDIO_DRIVER,
							" Playback of sample"
					" 48KHz Mono\n");
					samplePCM16_inaudiotest =
					    (char *)playback_audiotest;
				} else if (sgBrcm_auddrv_TestValues[3] == 1) {
					aTrace(LOG_AUDIO_DRIVER,
							" Playback of sample"
					" 8KHz Mono : use HW SRC mixer\n");
					samplePCM16_inaudiotest =
					    (char *)playback_audiotest_srcmixer;
					src_used = 1;
				}
			} else if (sgBrcm_auddrv_TestValues[2] == 1) {
				if (record_test_buf != NULL) {
					DEBUG
					    (" Playback of recorded data\n");
					samplePCM16_inaudiotest =
					    (char *)record_test_buf;
				} else
					aTrace(LOG_AUDIO_DRIVER,
							" record buffer freed:"
					" record data to play\n");
			}
		}
		break;
#endif
	case 3:
		{
			aTrace(LOG_AUDIO_DRIVER, " Audio DDRIVER Config\n");
			/* set the callback */
			/* AUDIO_DRIVER_Ctrl(drv_handle,AUDIO_DRIVER_SET_CB,
			   (void*)AUDIO_DRIVER_TEST_InterruptPeriodCB); */
			cbParams.pfCallBack =
			    AUDIO_DRIVER_TEST_InterruptPeriodCB;
			cbParams.pPrivateData = (void *)drv_handle;
			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_SET_CB,
					  (void *)&cbParams);

			/* configure defaults */

			if (src_used == 1)
				drv_config.sample_rate =
				    AUDIO_SAMPLING_RATE_8000;
			else
				drv_config.sample_rate =
				    AUDIO_SAMPLING_RATE_48000;

			drv_config.num_channel = AUDIO_CHANNEL_MONO;
			drv_config.bits_per_sample = 16;

			if (sgBrcm_auddrv_TestValues[2] != 0)
				drv_config.sample_rate =
				    sgBrcm_auddrv_TestValues[2];
			if (sgBrcm_auddrv_TestValues[3] != 0)
				drv_config.num_channel =
				    sgBrcm_auddrv_TestValues[3];

			aTrace(LOG_AUDIO_DRIVER, "Config:sr=%u nc=%d bs=%d\n",
					drv_config.sample_rate,
					drv_config.num_channel,
					drv_config.bits_per_sample);

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_CONFIG,
					  (void *)&drv_config);

			period_ms = 100;
			if (sgBrcm_auddrv_TestValues[4] != 0)
				period_ms = sgBrcm_auddrv_TestValues[4];

			/* set the interrupt period */
			period_bytes =
			    period_ms * (drv_config.sample_rate / 1000) *
			    (drv_config.num_channel) * 2;
			num_blocks = 2;	/* for RHEA */
			aTrace(LOG_AUDIO_DRIVER,
					"Period: ms=%ld bytes=%ld blocks:%ld\n",
					period_ms, period_bytes, num_blocks);
			AUDIO_DRIVER_Ctrl(drv_handle,
					  AUDIO_DRIVER_SET_INT_PERIOD,
					  (void *)&period_bytes);

			buf_param.buf_size = PCM_TEST_MAX_PLAYBACK_BUF_BYTES;
			buf_param.pBuf =
			    dma_alloc_coherent(NULL, buf_param.buf_size,
					       &dma_addr, GFP_KERNEL);
			if (buf_param.pBuf == NULL) {
				aTrace(LOG_AUDIO_DRIVER,
						"Cannot allocate Buffer\n");
				return 0;
			}
			buf_param.phy_addr = (UInt32) dma_addr;

			aTrace(LOG_AUDIO_DRIVER,
					"virt_addr = %s phy_addr=0x%lx\n",
					buf_param.pBuf, (UInt32) dma_addr);

			current_ipbuffer_index = 0;
			dma_buffer_write_index = 0;

			if ((num_blocks * period_bytes) <= TEST_BUF_SIZE)
				copy_bytes = (num_blocks * period_bytes);
			else
				copy_bytes = TEST_BUF_SIZE;

			src =
			    ((char *)samplePCM16_inaudiotest) +
			    current_ipbuffer_index;
			dest = buf_param.pBuf + dma_buffer_write_index;

			memcpy(dest, src, copy_bytes);

			current_ipbuffer_index += copy_bytes;

			aTrace(LOG_AUDIO_DRIVER, "copy_bytes %ld", copy_bytes);
			/* set the buffer params */
			AUDIO_DRIVER_Ctrl(drv_handle,
					  AUDIO_DRIVER_SET_BUF_PARAMS,
					  (void *)&buf_param);
			aTrace(LOG_AUDIO_DRIVER,
					" Audio DDRIVER Config Complete\n");
		}
		break;
	case 4:		/* Start the playback */
		{
			/* EP is default now */
			CSL_CAPH_DEVICE_e aud_dev = CSL_CAPH_DEV_EP;
			aTrace(LOG_AUDIO_DRIVER, " Start Playback\n");
			spkr = sgBrcm_auddrv_TestValues[2];

			AUDCTRL_EnablePlay(AUDIO_SOURCE_MEM,
					   spkr,
					   drv_config.num_channel,
					   drv_config.sample_rate, &testint);

			AUDCTRL_SetPlayVolume(AUDIO_SOURCE_MEM, spkr,
					      AUDIO_GAIN_FORMAT_mB, 0x00, 0x00,
					      0);
			/* 0 db for both L and R channels. */

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_START,
					  &aud_dev);
			aTrace(LOG_AUDIO_DRIVER, "Playback started\n");

			/*  Need to implement some sync mechanism */
			msleep(5000);
			aTrace(LOG_AUDIO_DRIVER, " Stop playback\n");

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_STOP, NULL);

			/* disable the playback path */
			AUDCTRL_DisablePlay(AUDIO_SOURCE_MEM, spkr, testint);

			AUDIO_DRIVER_Close(drv_handle);

		}
		break;
#ifdef CONFIG_ARM2SP_PLAYBACK_TEST
	case 5:
		{
			/* val2 -> 0  ,
			   val3 -> VORENDER_TYPE  0- EP_OUT (ARM2SP) ,
				1 - HS, 2 - IHF
			   Val4 -   0 - playback
			   Val5 - Sampling rate  0 -> playback of 8K PCM
			   Val6  - Mix mode CSL_ARM2SP_VOICE_MIX_MODE_t
			 */

			/* AUDTST_VoicePlayback(AUDIO_SINK_HANDSET,
			0, 0 , VORENDER_PLAYBACK_DL,
			   VORENDER_VOICE_MIX_NONE ); */
			AUDTST_VoicePlayback(0, sgBrcm_auddrv_TestValues[2],
			sgBrcm_auddrv_TestValues[3], VORENDER_PLAYBACK_DL,
			sgBrcm_auddrv_TestValues[4]);	/* play to DL */
		}
		break;
#endif
	default:
		aTrace(LOG_AUDIO_DRIVER, " Invalid Playback Command\n");
	}
	return 0;
}

static void AUDIO_DRIVER_TEST_InterruptPeriodCB(void *pPrivate)
{
	char *src;
	char *dest;

	if ((current_ipbuffer_index + period_bytes) >= TEST_BUF_SIZE)
		current_ipbuffer_index = 0;

	src = ((char *)samplePCM16_inaudiotest) + current_ipbuffer_index;
	dest = buf_param.pBuf + dma_buffer_write_index;

	memcpy(dest, src, period_bytes);

	current_ipbuffer_index += period_bytes;
	dma_buffer_write_index += period_bytes;

	if (dma_buffer_write_index >= (num_blocks * period_bytes))
		dma_buffer_write_index = 0;

	/* aTrace(LOG_AUDIO_DRIVER, " current_ipbuffer_index %d:
	 * dma_buffer_write_index- %d\n",
	 * rent_ipbuffer_index,dma_buffer_write_index);
	 */

	return;
}

static unsigned long current_capt_buffer_index;
static unsigned long capt_dma_buffer_read_index;

static unsigned long capt_period_bytes;
static unsigned long capt_num_blocks;
static AUDIO_DRIVER_BUFFER_t capt_buf_param;
static int HandleCaptCommand()
{

	unsigned long period_ms;
	static AUDIO_DRIVER_HANDLE_t drv_handle;
	static AUDIO_DRIVER_CONFIG_t drv_config;
	static dma_addr_t dma_addr;
	static AUDIO_SOURCE_Enum_t mic = AUDIO_SOURCE_ANALOG_MAIN;
	AUDIO_DRIVER_CallBackParams_t cbParams;

	static AUDIO_DRIVER_TYPE_t drv_type = AUDIO_DRIVER_CAPT_HQ;

	static Boolean record_buf_allocated;

	switch (sgBrcm_auddrv_TestValues[1]) {
	case 1:		/* open the capture device */
		{
			if (sgBrcm_auddrv_TestValues[2] != 0)
				drv_type = sgBrcm_auddrv_TestValues[2];
			DEBUG
			    (" Audio Capture DDRIVER Open drv_type %d\n",
			     drv_type);
			drv_handle = AUDIO_DRIVER_Open(drv_type);
			aTrace(LOG_AUDIO_DRIVER,
					" Audio DDRIVER Open Complete\n");

		}
		break;
		/* configure capture device */
	case 2:
		{
			aTrace(LOG_AUDIO_DRIVER,
					" Audio Capture DDRIVER Config\n");
			/* set the callback */
			cbParams.pfCallBack =
			    AUDIO_DRIVER_TEST_CaptInterruptPeriodCB;
			cbParams.pPrivateData = (void *)drv_handle;
			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_SET_CB,
					  (void *)&cbParams);

			if (drv_type == AUDIO_DRIVER_CAPT_HQ) {
				/* configure defaults */
				drv_config.sample_rate =
				    AUDIO_SAMPLING_RATE_48000;
				drv_config.num_channel = AUDIO_CHANNEL_MONO;
				drv_config.bits_per_sample = 16;
			} else if (drv_type == AUDIO_DRIVER_CAPT_VOICE) {
				/* configure defaults */
				drv_config.sample_rate =
				    AUDIO_SAMPLING_RATE_8000;
				drv_config.num_channel = AUDIO_CHANNEL_MONO;
				drv_config.bits_per_sample = 16;

			}

			if (sgBrcm_auddrv_TestValues[2] != 0)
				drv_config.sample_rate =
				    sgBrcm_auddrv_TestValues[2];
			if (sgBrcm_auddrv_TestValues[3] != 0)
				drv_config.num_channel =
				    sgBrcm_auddrv_TestValues[3];

			aTrace(LOG_AUDIO_DRIVER, "Config:sr=%u nc=%d bs=%d\n",
					drv_config.sample_rate,
					drv_config.num_channel,
					drv_config.bits_per_sample);

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_CONFIG,
					  (void *)&drv_config);

			period_ms = 100;
			if (sgBrcm_auddrv_TestValues[4] != 0)
				period_ms = sgBrcm_auddrv_TestValues[4];

			/* set the interrupt period */
			capt_period_bytes =
			    period_ms * (drv_config.sample_rate / 1000) *
			    (drv_config.num_channel) * 2;
			capt_num_blocks = 2;	/* limitation for RHEA */

			aTrace(LOG_AUDIO_DRIVER,
					"Period: ms=%ld bytes=%ld blocks:%ld\n",
					period_ms, capt_period_bytes,
					capt_num_blocks);
			AUDIO_DRIVER_Ctrl(drv_handle,
					  AUDIO_DRIVER_SET_INT_PERIOD,
					  (void *)&capt_period_bytes);

			current_capt_buffer_index = 0;
			capt_dma_buffer_read_index = 0;

			capt_buf_param.buf_size =
			    PCM_TEST_MAX_CAPTURE_BUF_BYTES;
			capt_buf_param.pBuf =
			    dma_alloc_coherent(NULL, capt_buf_param.buf_size,
					       &dma_addr, GFP_KERNEL);
			if (capt_buf_param.pBuf == NULL) {
				aTrace(LOG_AUDIO_DRIVER,
						"Cannot allocate Buffer\n");
				return 0;
			}
			capt_buf_param.phy_addr = (UInt32) dma_addr;

			aTrace(LOG_AUDIO_DRIVER,
					"virt_addr = %s phy_addr=0x%lx\n",
					capt_buf_param.pBuf, (UInt32) dma_addr);

			memset(capt_buf_param.pBuf, 0,
			       PCM_TEST_MAX_CAPTURE_BUF_BYTES);
			/* set the buffer params */
			AUDIO_DRIVER_Ctrl(drv_handle,
					  AUDIO_DRIVER_SET_BUF_PARAMS,
					  (void *)&capt_buf_param);

			aTrace(LOG_AUDIO_DRIVER,
					" Audio DDRIVER Config Complete\n");
		}
		break;
		/* Start the capture */
	case 3:
		{

			if (!record_buf_allocated) {
				record_test_buf =
				    kmalloc(TEST_BUF_SIZE, GFP_KERNEL);
				if (record_test_buf == NULL) {
					aError("kmalloc failed\n");
					return -1;
				}
				memset(record_test_buf, 0, TEST_BUF_SIZE);
				record_buf_allocated = 1;
			}
			if (sgBrcm_auddrv_TestValues[2] != 0)
				mic = sgBrcm_auddrv_TestValues[2];

			aTrace(LOG_AUDIO_DRIVER,
					"Start capture mic %d\n", mic);

			AUDCTRL_EnableRecord(mic,
					     AUDIO_SINK_MEM,
					     drv_config.num_channel,
					     drv_config.sample_rate, NULL);

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_START, &mic);

			aTrace(LOG_AUDIO_DRIVER, "capture started\n");

			msleep(5000);

			aTrace(LOG_AUDIO_DRIVER, " Stop capture\n");

			AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_STOP, NULL);

			AUDCTRL_DisableRecord(mic, AUDIO_SOURCE_MEM, 0);

			aTrace(LOG_AUDIO_DRIVER, "capture stopped\n");

		}
		break;

	case 4:
		/* free the buffer record_test_buf */
		aTrace(LOG_AUDIO_DRIVER, " Freed the recorded test buf\n");
		kfree(record_test_buf);
		record_buf_allocated = 0;
		record_test_buf = NULL;
		break;
	default:
		aTrace(LOG_AUDIO_DRIVER, " Invalid capture Command\n");
	}
	return 0;

}

static void AUDIO_DRIVER_TEST_CaptInterruptPeriodCB(void *pPrivate)
{
	char *src;
	char *dest;

	/* aTrace(LOG_AUDIO_DRIVER, " %lx: capture Interrupt- %d\n",jiffies); */

	if ((current_capt_buffer_index + capt_period_bytes) >= TEST_BUF_SIZE)
		current_capt_buffer_index = 0;

	dest = ((char *)record_test_buf) + current_capt_buffer_index;
	src = capt_buf_param.pBuf + capt_dma_buffer_read_index;

	memcpy(dest, src, capt_period_bytes);

	current_capt_buffer_index += capt_period_bytes;
	capt_dma_buffer_read_index += capt_period_bytes;

	if (capt_dma_buffer_read_index >= (capt_num_blocks * capt_period_bytes))
		capt_dma_buffer_read_index = 0;

	/* DEBUG(
	   " current_capt_buffer_index %d: capt_dma_buffer_read_index- %d\n",
	   current_capt_buffer_index,capt_dma_buffer_read_index); */

	return;
}

#ifdef CONFIG_ARM2SP_PLAYBACK_TEST
#ifdef CONFIG_BCM_ENABLE_TESTDATA
/* voice playback test including amrnb, pcm via VPU, ARM2SP, and amrwb */
#define	WAIT_TIME	2000	/* in msec */
void AUDTST_VoicePlayback(UInt32 Val2, UInt32 Val3, UInt32 Val4, UInt32 Val5,
			  UInt32 Val6)
{
	{
		VORENDER_TYPE_t drvtype = VORENDER_TYPE_PCM_ARM2SP;
		UInt32 totalSize = 0;
		UInt8 *dataSrc;
		UInt32 frameSize = 0;
		UInt32 finishedSize;
		UInt32 writeSize;
		CSL_ARM2SP_PLAYBACK_MODE_t playbackMode;
		CSL_ARM2SP_VOICE_MIX_MODE_t mixMode;
		AUDIO_SAMPLING_RATE_t sr = AUDIO_SAMPLING_RATE_8000;
		AUDIO_SINK_Enum_t speaker = AUDIO_SINK_HANDSET;
		Boolean setTransfer = FALSE;
		AUDIO_NUM_OF_CHANNEL_t stereo = AUDIO_CHANNEL_MONO;
		unsigned long to_jiff = msecs_to_jiffies(WAIT_TIME);

		/* for rhea from here */
		if (Val3 == 0) {
			drvtype = VORENDER_TYPE_PCM_ARM2SP;
			/* earpiece */
			speaker = AUDIO_SINK_HANDSET;
		} else if (Val3 == 1) {
			drvtype = VORENDER_TYPE_PCM_ARM2SP;
			/* headset */
			speaker = AUDIO_SINK_HEADSET;
		} else if (Val3 == 2) {
			drvtype = VORENDER_TYPE_PCM_ARM2SP;
			/* ihf */
			speaker = AUDIO_SINK_LOUDSPK;
		}

		sr = AUDIO_SAMPLING_RATE_8000;

		aTrace(LOG_AUDIO_DRIVER,
				"\n debug 1, stereo =%d drvtype =%d\n", stereo,
				drvtype);
		AUDCTRL_EnableTelephony(AUDIO_SOURCE_ANALOG_MAIN,
					AUDIO_SINK_HANDSET);

		AUDCTRL_SetPlayVolume(AUDIO_SOURCE_MEM,
				      speaker, AUDIO_GAIN_FORMAT_mB, 0, 0, 0);

		/* init driver */
		AUDDRV_VoiceRender_Init(drvtype);

		AUDDRV_VoiceRender_SetBufDoneCB(drvtype, AUDDRV_BUFFER_DONE_CB);
		/* 1= dl, 2 = ul, 3 =both
		   0 = none, 1= dl, 2 = ul, 3 =both */
		playbackMode = VORENDER_PLAYBACK_DL;
		mixMode = (CSL_ARM2SP_VOICE_MIX_MODE_t) Val6;

		if (Val6 == 10)
			setTransfer = TRUE;	/* set buffer transfer */

		if (Val4 == 0) {
			if (sr == AUDIO_SAMPLING_RATE_8000) {
				dataSrc = &playback_audiotest_srcmixer[0];
				totalSize = sizeof(playback_audiotest_srcmixer);
				frameSize = (sr * 2 * 2) / 50;	/* 20 msec */

				/* start writing data */
				AUDDRV_VoiceRender_SetConfig(drvtype,
						playbackMode,
						mixMode, sr,
						VP_SPEECH_MODE_LINEAR_PCM_8K,
						0, 0);
			} else if (sr == AUDIO_SAMPLING_RATE_16000) {
				dataSrc = &playback_audiotest_srcmixer[0];
				totalSize = sizeof(playback_audiotest_srcmixer);
				frameSize = (sr * 2 * 2) / 50;	/* 20 msec */

				/* start writing data */
				AUDDRV_VoiceRender_SetConfig(drvtype,
					playbackMode,
					mixMode, sr,
					VP_SPEECH_MODE_LINEAR_PCM_16K,
					0, 0);
			}
		}
		finishedSize = 0;
		writeSize = 0;

		AUDDRV_BufDoneSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

		aTrace(LOG_AUDIO_DRIVER, "\n debug 1!, totalSize = 0x%x\n",
				(unsigned int)totalSize);
		AUDDRV_VoiceRender_Start(drvtype);

		writeSize = frameSize;
		AUDDRV_VoiceRender_Write(drvtype, dataSrc, writeSize);

		/* The AUDDRV_BUFFER_DONE_CB callback will release
		 the buffer size. */
		while (OSSEMAPHORE_Obtain(AUDDRV_BufDoneSema, to_jiff) ==
		       OSSTATUS_SUCCESS) {

			dataSrc += writeSize;
			finishedSize += writeSize;

			if (finishedSize >= totalSize)
				break;

			if (totalSize - finishedSize < frameSize)
				writeSize = totalSize - finishedSize;
			else
				writeSize = frameSize;

			AUDDRV_VoiceRender_Write(drvtype, dataSrc, writeSize);

			aTrace(LOG_AUDIO_DRIVER,
					"\n debug 2: writeSize = 0x%x,"
					"finishedSize = 0x%x\n",
					(unsigned int)writeSize,
					(unsigned int)finishedSize);

		}

		aTrace(LOG_AUDIO_DRIVER,
				"\n debug 7: writeSize = 0x%x,"
				"finishedSize = 0x%x\n",
				(unsigned int)writeSize,
				(unsigned int)finishedSize);

		/* finish all the data
		   // stop the driver
		   // TRUE= immediately stop */
		AUDDRV_VoiceRender_Stop(drvtype, TRUE);

		/* need to give time to dsp to stop. */
		usleep_range(3000, 10000);/* make sure the path turned on */

		AUDDRV_VoiceRender_Shutdown(drvtype);

		aTrace(LOG_AUDIO_DRIVER, "\n  Voice render stop done\n");
		AUDCTRL_DisableTelephony();

		OSSEMAPHORE_Destroy(AUDDRV_BufDoneSema);
	}
}
#endif
#endif

static AUDIO_DRIVER_HANDLE_t cur_drv_handle;
static UInt32 cur_codecVal;
static AudioMode_t cur_mode = AUDIO_MODE_HANDSET;
void AUDTST_VoIP(UInt32 Val2, UInt32 Val3, UInt32 Val4, UInt32 Val5,
		 UInt32 Val6)
{
	/* Val2 : mic
	   // Val3: speaker
	   // Val4: delay in miliseconds
	   // Val5: codec value
	   // val6: codec bitrate if needed
	 */
	UInt8 *dataDest = NULL;
	UInt32 vol = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	UInt32 codecVal = 0;
	static AUDIO_DRIVER_HANDLE_t drv_handle;
	AUDIO_DRIVER_CallBackParams_t cbParams;
	AUDIO_SOURCE_Enum_t mic = (AUDIO_SOURCE_Enum_t) Val2;	/* mic */
	AUDIO_SINK_Enum_t spk = (AUDIO_SINK_Enum_t) Val3;	/* speaker */
	UInt32 delayMs = Val4;	/* delay in milliseconds */
	voip_data_t voip_codec;
	int app_profile;

	if (record_test_buf == NULL)
		record_test_buf = kzalloc(1024 * 1024, GFP_KERNEL);

	if (record_test_buf == NULL) {
		aError("Memory allocation failed\n");
		return;
	}

	codecVal = Val5;	/* 0 for 8k PCM */
	voip_codec.codec_type = cur_codecVal = codecVal;

	if (codecVal == 0 || codecVal == 1 || codecVal == 4)
		Val6 = 0; /* the above codec type does not have bitrate */

	voip_codec.bitrate_index = Val6; /* bitrate only for AMR */

	if (Val3 == 0 || Val3 == 1 || Val3 == 2 || Val3 == 4)
		mode = (AudioMode_t) Val3;
	cur_mode = mode;

	aTrace(LOG_AUDIO_DRIVER, "\n AUDTST_VoIP codecVal %ld\n", codecVal);
	/* VOIP_PCM_16K or VOIP_AMR_WB */

	/* Query App policy for App prof LOOPBACK
	If allowed save app prof and transition to INCALL state
	Actual param loading for slected APP and mode will be done from
	enable telephony */

	if ((codecVal == 4) || (codecVal == 5)) {
		/* WB has to use AUDIO_APP_VOICE_CALL_WB */
		app_profile = AUDIO_Policy_Get_Profile(
					AUDIO_APP_VOICE_CALL_WB);
		if (app_profile != AUDIO_APP_VOICE_CALL_WB) {
			aWarn("App policy does not allow change to"
				"AUDIO_APP_VOICE_CALL_WB\n");
		}
	} else { /* NB VoIP case */
		app_profile = AUDIO_Policy_Get_Profile(
					AUDIO_APP_LOOPBACK);
		if (app_profile != AUDIO_APP_LOOPBACK) {
			aWarn("App policy does not allow change to"
				"AUDIO_APP_LOOPBACK\n");
		}
	}

	AUDCTRL_SaveAudioApp((AudioApp_t)app_profile);
	AUDCTRL_SaveAudioMode(mode);
	/*change state to INCALL now to avoid any undesired app profile
	changes after we start the LOOPBACK */
	AUDIO_Policy_SetState(BRCM_STATE_INCALL);

	/* configure EC and NS for the loopback test */
#if defined(USE_LOOPBACK_SYSPARM) && defined(CONFIG_BCM_MODEM)
	/* use sysparm to configure EC */
	AUDCTRL_EC((Boolean)(AudParmP()[mode +
	AUDIO_APP_LOOPBACK * AUDIO_MODE_NUMBER].echo_cancelling_enable),
	0);
	/* use sysparm to configure NS */
	AUDCTRL_NS((Boolean)(AudParmP()[mode +
	AUDIO_APP_LOOPBACK * AUDIO_MODE_NUMBER].\
	ul_noise_suppression_enable));
#else	/* USE_LOOPBACK_SYSPARM */
	AUDCTRL_EC(FALSE, 0);
	AUDCTRL_NS(FALSE);
#endif

	AUDCTRL_EnableTelephony(mic, spk);
	AUDCTRL_SetTelephonySpkrVolume(spk, vol, AUDIO_GAIN_FORMAT_mB);

	/* init driver */

	drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIP);
	cur_drv_handle = drv_handle;

	/* set UL callback */
	cbParams.voipULCallback = AudDrv_VOIP_DumpUL_CB;
	cbParams.pPrivateData = (void *)0;
	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_SET_VOIP_UL_CB,
			  (void *)&cbParams);

	/* set the callback */
	cbParams.voipDLCallback = AudDrv_VOIP_FillDL_CB;
	cbParams.pPrivateData = (void *)0;
	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_SET_VOIP_DL_CB,
			  (void *)&cbParams);

	dataDest = (UInt8 *) &record_test_buf[0];

	sVtQueue = AUDQUE_Create(dataDest, 2000, 322);
	if (sVtQueue == NULL) {
		aError("Memory allocation failed!\n");
		return;
	}

	AUDDRV_BufDoneSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
	sVtQueue_Sema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);

	delay_count = delayMs / 20;
	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_START, &voip_codec);

	/* test with loopback UL to DL */
	aTrace(LOG_AUDIO_DRIVER, "\nVoIP loopback running\n");
}

void AUDTST_VoIP_Stop(void)
{
	if (cur_drv_handle) {
		AUDIO_DRIVER_Ctrl(cur_drv_handle, AUDIO_DRIVER_STOP, NULL);

		aTrace(LOG_AUDIO_DRIVER, "\nVoIP: Stop\n");

		/* disable the hw */
		AUDCTRL_DisableTelephony();
		/*Restore the App policy state*/
		AUDIO_Policy_RestoreState();

		/*may need it to avoid crash or freeze*/
		/*msleep(200);*/

		OSSEMAPHORE_Destroy(AUDDRV_BufDoneSema);
		OSSEMAPHORE_Destroy(sVtQueue_Sema);
		AUDQUE_Destroy(sVtQueue);

		AUDIO_DRIVER_Close(cur_drv_handle);
		cur_drv_handle = NULL;
		/* from ceckpatch: kfree is safe, so no need to check */
		kfree(record_test_buf);
		record_test_buf = NULL;
	} else
		aTrace(LOG_AUDIO_DRIVER, "\nInvalid VoIP Stop\n");

	aTrace(LOG_AUDIO_DRIVER, "\nVoIP: Finish\n");

}

