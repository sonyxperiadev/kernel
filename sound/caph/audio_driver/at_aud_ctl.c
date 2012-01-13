/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
	http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/printk.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/asound.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"
#include "audio_ddriver.h"

#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "auddrv_audlog.h"

#include <linux/io.h>
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_sdt.h"
#include "brcm_rdb_srcmixer.h"
#include "brcm_rdb_cph_cfifo.h"
#include "brcm_rdb_cph_aadmac.h"
#include "brcm_rdb_cph_ssasw.h"
#include "brcm_rdb_ahintc.h"
#include "brcm_rdb_util.h"

#include "csl_caph_hwctrl.h"

#include "audio_pmu_adapt.h"

/**
	Description:
	AT command handler, handle command AT commands at*maudmode=P1,P2,P3
	Parameters:
	pChip -- Pointer to chip data structure
	ParamCount -- Count of parameter array
	Params  --- P1,P2,...,P6
loopback:
at*maudmode=11, 1, 0  //HANDSET  ( 11,  AUDIO_SINK_Enum_t, AUDIO_SOURCE_Enum_t )
at*maudmode=12  //disable loopback

at*maudmode=11, 1, 4  //main mic to IHF
at*maudmode=12

at*maudmode=11,2,1  //headset
at*maudmode=12
**/

int AtMaudMode(brcm_alsa_chip_t *pChip, Int32 ParamCount, Int32 *Params)
{
	AUDIO_SOURCE_Enum_t mic = AUDIO_SOURCE_ANALOG_MAIN;
	AUDIO_SINK_Enum_t spk = AUDIO_SINK_HANDSET;
	int rtn = 0;		/* 0 means Ok */
	static UInt8 loopback_status = 0, loopback_input = 0, loopback_output =
	    0, sidetone_mode = 0;
	Int32 pCurSel[2];

	BCM_AUDIO_DEBUG("%s P1-P6=%ld %ld %ld %ld %ld %ld cnt=%ld\n",
			__func__, Params[0], Params[1], Params[2],
			Params[3], Params[4], Params[5], ParamCount);

	csl_caph_ControlHWClock(TRUE);

	switch (Params[0]) {

	case 0:		/* at*maudmode 0 */
#if !defined(USE_NEW_AUDIO_PARAM)
		Params[0] = AUDCTRL_GetAudioMode();
		BCM_AUDIO_DEBUG(" %s mode %ld\n", __func__, Params[0]);
#endif
		break;

	case 1:		/* at*maudmode 1 mode */
#if !defined(USE_NEW_AUDIO_PARAM)
		AUDCTRL_GetSrcSinkByMode(Params[1], &mic, &spk);
		pCurSel[0] =
	pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL - 1].iLineSelect[0];
		pCurSel[1] =
		    pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				     1].iLineSelect[1];

		/* Update 'VC-SEL' -- */
		pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				 1].iLineSelect[0] = mic;
		pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				 1].iLineSelect[1] = spk;
		AUDCTRL_SetAudioMode(Params[1]);
		BCM_AUDIO_DEBUG(" %s mic %d spk %d mode %ld\n", __func__,
		mic, spk, Params[1]);
#endif
		break;

	case 8:		/* at*maudmode=8 */
		Params[0] = loopback_status;
		BCM_AUDIO_DEBUG(" %s loopback status is %d\n", __func__,
				loopback_status);
		break;

	case 9:		/* at*maudmode=9,x.  x = 0 --> disable; x =1 enable */
		loopback_status = Params[1];
		if (loopback_status > 1)
			loopback_status = 1;

		BCM_AUDIO_DEBUG(" %s set loopback status %d (1:ena 0:dis)\n",
				__func__, loopback_status);
		break;

	case 10:		/* at*maudmode=10  --> get loopback path */
		/* Per PCG request
		if (loopback_output > 2) loopback_output = 2; */

		Params[0] = loopback_input;
		Params[1] = loopback_output;
		BCM_AUDIO_DEBUG("%s loopback path is from src %d to sink %d\n",
				__func__, loopback_input, loopback_output);
		break;

	/* at*maudmode=11,x,y  --> set loopback path. */
	/* mic: 0 = default mic, 1 = main mic */
	/* spk: 0 = handset, 1 = headset, 2 = loud speaker */
	case 11:
		loopback_input = Params[1];
		loopback_output = Params[2];
		sidetone_mode = Params[3];

		if (((loopback_input > 6) && (loopback_input != 11)) ||
			((loopback_output > 2) && (loopback_output != 9) && 
			(loopback_output != 4))) {
		BCM_AUDIO_DEBUG("%s srr/sink exceeds its range.\n", __func__);
			rtn = -1;
			break;
		}
		if (loopback_output == 2)	/* use IHF */
			loopback_output = 4;
		if (loopback_input == 0)	/* default mic: use main mic */
			loopback_input = 1;

		loopback_status = 1;
		/* enable the HW loopback without DSP. */
		AUDCTRL_SetAudioLoopback(TRUE, loopback_input, loopback_output,
					 sidetone_mode);

		BCM_AUDIO_DEBUG("%s ena lpback: src %d sink %d sidetone %d\n",
				__func__, loopback_input, loopback_output,
				sidetone_mode);
		break;

	case 12:	/* at*maudmode=12  --> disable loopback path*/
		loopback_status = 0;
		AUDCTRL_SetAudioLoopback(FALSE, loopback_input, loopback_output,
					 sidetone_mode);
		/* mdelay(100); */
		BCM_AUDIO_DEBUG("%s dis lpback: src %d sink %d sidetone %d\n",
				__func__, loopback_input, loopback_output,
				sidetone_mode);
		break;

	case 13:		/* at*maudmode=13  --> Get call ID */
		BCM_AUDIO_DEBUG("%s get call ID is not supported\n",
				__func__);
		rtn = -1;
		break;

	case 14:	/* at*maudmode=14  --> read current mode and app */
#if defined(USE_NEW_AUDIO_PARAM)
        Params[0] = AUDCTRL_GetAudioApp();
		Params[1] = AUDCTRL_GetAudioMode();
		BCM_AUDIO_DEBUG("%s app %ld mode %ld\n", 
                __FUNCTION__, Params[0], Params[1]);
#endif
		break;

	case 15:	/* at*maudmode=15  --> set current mode and app */
#if defined(USE_NEW_AUDIO_PARAM)
		AUDCTRL_GetSrcSinkByMode(Params[2], &mic, &spk);
        pCurSel[0] = pChip->
            streamCtl[CTL_STREAM_PANEL_VOICECALL-1].
            iLineSelect[0]; /*save current setting*/
        pCurSel[1] = pChip->
            streamCtl[CTL_STREAM_PANEL_VOICECALL-1].
            iLineSelect[1];

        /* Update 'VC-SEL' -- */
		pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL-1].
            iLineSelect[0] = mic;
		pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL-1].
            iLineSelect[1] = spk;
        AUDCTRL_SetAudioMode( Params[2], Params[1]);
        BCM_AUDIO_DEBUG("%s mic %d spk %d mode %ld app %ld\n", 
                __FUNCTION__, mic,spk,Params[2], Params[1]);
#endif    
		break;

	case 99:		/* at*maudmode=99  --> stop tuning */
		break;

	/* at*maudmode=100  --> set external audio amplifer gain in PMU */
	/* PCG and loadcal currently use Q13p2 gain format */
	case 100:
#ifdef CONFIG_BCMPMU_AUDIO
{
	PMU_AudioGainMapping_t pmu_gain;
	short gain;

	gain = (short)Params[3];

	if (Params[1] == 3) {

	BCM_AUDIO_DEBUG("Params[2] = %d, Params[3] %d, audio mode %d \n",
		(int)Params[3], (int)Params[2], AUDDRV_GetAudioMode());

		
	   if ((Params[2] == PARAM_PMU_SPEAKER_PGA_LEFT_CHANNEL) ||
		(Params[2] == PARAM_PMU_SPEAKER_PGA_RIGHT_CHANNEL)) {
#if defined(USE_NEW_AUDIO_PARAM)
		if ((AUDDRV_GetAudioMode() == AUDIO_MODE_HEADSET) ||
		(AUDDRV_GetAudioMode() == AUDIO_MODE_TTY)) {
#else
		if ((AUDDRV_GetAudioMode() == AUDIO_MODE_HEADSET) ||
		(AUDDRV_GetAudioMode() == AUDIO_MODE_HEADSET_WB) ||
		(AUDDRV_GetAudioMode() == AUDIO_MODE_TTY) ||
		(AUDDRV_GetAudioMode() == AUDIO_MODE_TTY_WB)) {
#endif
			AUDIO_PMU_IHF_POWER(FALSE);
			AUDIO_PMU_HS_POWER(TRUE);
			BCM_AUDIO_DEBUG("%s ext headset speaker gain = %d\n",
				__func__, gain);
			pmu_gain = map2pmu_hs_gain(25 * gain);
			BCM_AUDIO_DEBUG
			("%s ext headset speaker gain = %d after lookup\n",
				__func__, pmu_gain.PMU_gain_enum);

			if (Params[2] == PARAM_PMU_SPEAKER_PGA_LEFT_CHANNEL)
				AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_LEFT,
					pmu_gain.PMU_gain_enum);
		    else if (Params[2] == PARAM_PMU_SPEAKER_PGA_RIGHT_CHANNEL)
				AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_RIGHT,
					pmu_gain.PMU_gain_enum);
		}
#if defined(USE_NEW_AUDIO_PARAM)
		else if (AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE) {
#else
		else if ((AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE) ||
		(AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE_WB)) {
#endif
			AUDIO_PMU_HS_POWER(FALSE);
			AUDIO_PMU_IHF_POWER(TRUE);
			BCM_AUDIO_DEBUG("%s ext IHF speaker gain = %d\n",
				__func__, gain);
			pmu_gain = map2pmu_ihf_gain(25 * gain);

			BCM_AUDIO_DEBUG("%s ext IHF speaker gain = %d\n",
				__func__, pmu_gain.PMU_gain_enum);
			AUDIO_PMU_IHF_SET_GAIN(pmu_gain.PMU_gain_enum);
		}

	   } /* Params[2] checking */

		 BCM_AUDIO_DEBUG("Params[2] = %d, Params[3] %d, audio mode %d \n",
			 (int)Params[3], (int)Params[2], AUDDRV_GetAudioMode());
	
		 if (Params[2] == PARAM_PMU_HIGH_GAIN_MODE_FLAG) {
#if defined(USE_NEW_AUDIO_PARAM)
			  if (AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE) {
#else
			  if ((AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE) ||
			  (AUDDRV_GetAudioMode() == AUDIO_MODE_SPEAKERPHONE_WB)) {
#endif
				  BCM_AUDIO_DEBUG("ext IHF high gain mode = %d\n",
					  (int)Params[3]);
				  AUDIO_PMU_HI_GAIN_MODE_EN((int)Params[3]);
			  }
		  }

	} /* if (Params[1] == 3) */
} /* case 100 */
#endif

		break;

	default:
		BCM_AUDIO_DEBUG("%s Unsupported cmd %ld\n", __func__,
				Params[0]);
		rtn = -1;
		break;
	}

	return rtn;
}

/**
 Description:
 AT command handler, handle command AT commands at+maudlooptest=P1,P2,P3
 Parameters:
	pChip -- Pointer to chip data structure
	ParamCount -- Count of parameter array
	Params  --- P1,P2,...,P6
**/
int AtMaudLoopback(brcm_alsa_chip_t *pChip, Int32 ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("%s P1-P6=%ld %ld %ld %ld %ld %ld cnt=%ld\n",
			__func__, Params[0], Params[1], Params[2],
			Params[3], Params[4], Params[5], ParamCount);

	return -1;
}

/**
 Description:
 AT command handler, handle command AT commands at*maudlog=P1,P2,P3
 Parameters:
	pChip -- Pointer to chip data structure
	ParamCount -- Count of parameter array
	Params  --- P1,P2,...,P6
**/
int AtMaudLog(brcm_alsa_chip_t *pChip, Int32 ParamCount, Int32 *Params)
{
	int rtn = 0;

	BCM_AUDIO_DEBUG("%s P1-P6=%ld %ld %ld %ld %ld %ld cnt=%ld.\n",
			__func__, Params[0], Params[1], Params[2],
			Params[3], Params[4], Params[5], ParamCount);

	switch (Params[0]) {
	case 1:		/* at*maudlog=1,x,y,z */
		rtn =
		    AUDDRV_AudLog_Start(Params[1], Params[2], Params[3],
					(char *)NULL);
		if (rtn < 0) {
			BCM_AUDIO_DEBUG("\n Couldnt setup channel\n");
			rtn = -1;
		}
		BCM_AUDIO_DEBUG(
"%s start log on stream %ld, capture pt %ld, consumer %ld.\n",
		     __func__, Params[1], Params[2], Params[3]);
		break;

	case 2:		/* at*maudlog=2,x */
		rtn = AUDDRV_AudLog_Stop(Params[1]);
		if (rtn < 0)
			rtn = -1;
		BCM_AUDIO_DEBUG("%s start log on stream %ld.\n", __func__,
				Params[1]);
		break;

	default:
		rtn = -1;
		break;
	}

	return rtn;
}

/**
 Description:
 AT command handler, handle command AT commands at*maudtst=P1,P2,P3,P4,P5,P6
 Parameters:
	pChip -- Pointer to chip data structure
	ParamCount -- Count of parameter array
	Params  --- P1,P2,...,P6
**/
int AtMaudTst(brcm_alsa_chip_t *pChip, Int32 ParamCount, Int32 *Params)
{
	BCM_AUDIO_DEBUG("%s P1-P6=%ld %ld %ld %ld %ld %ld cnt=%ld.\n",
			__func__, Params[0], Params[1], Params[2],
			Params[3], Params[4], Params[5], ParamCount);

	/* test command 100/101 is to control the HW clock.
	In this case, dont enable the clock */
	if (Params[0] != 100 && Params[0] != 101)
		csl_caph_ControlHWClock(TRUE);

	switch (Params[0]) {
#if defined(USE_NEW_AUDIO_PARAM)
		{
			Int32 pCurSel[2];
			AUDIO_SOURCE_Enum_t mic = AUDIO_SOURCE_ANALOG_MAIN;
			AUDIO_SINK_Enum_t spk = AUDIO_SINK_HANDSET;

	case 2:		/*at*maudtst 2: return both mode and app */
			Params[2] = AUDCTRL_GetAudioMode();
			Params[1] = AUDCTRL_GetAudioApp();
			BCM_AUDIO_DEBUG("%s mode %ld app %ld\n", __func__,
					Params[2], Params[1]);
			break;

	case 3:		/*at*maudtst 3 mode app: set mode and app*/
			AUDCTRL_GetSrcSinkByMode(Params[1], &mic, &spk);
			pCurSel[0] =
			pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
					1].iLineSelect[0];
			pCurSel[1] =
			pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
					1].iLineSelect[1];

			/* Update 'VC-SEL' -- */
			pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
					 1].iLineSelect[0] = mic;
			pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
					 1].iLineSelect[1] = spk;
			/* need to fill in audio app (check with pcg) */
			AUDCTRL_SetAudioMode(Params[1], Params[2]);

			BCM_AUDIO_DEBUG("%s mic %d spk %d mode %ld app %ld\n",
					__func__, mic, spk, Params[1],
					Params[2]);
			break;
		}
#endif
	case 25:
		AUDCTRL_SetPlayVolume(AUDIO_SOURCE_MEM, Params[1], /* spk */
					AUDIO_GAIN_FORMAT_mB,
					Params[2],	/* left volume */
					Params[3],	/* right volume */
					0);

		BCM_AUDIO_DEBUG("Set speaker volume left %ld right %ld\n",
				Params[2], Params[3]);
		break;

		/* typedef enum AUDIO_SINK_Enum_t {
		!     AUDIO_SINK_HANDSET,
		!     AUDIO_SINK_HEADSET,
		!     AUDIO_SINK_HANDSFREE,
		!     AUDIO_SINK_BTM,  //Bluetooth HFP
		!     AUDIO_SINK_LOUDSPK,
		!     AUDIO_SINK_TTY,
		!     AUDIO_SINK_HAC,
		!     AUDIO_SINK_USB,
		!     AUDIO_SINK_BTS,  //Bluetooth A2DP
		!     AUDIO_SINK_I2S,
		!     AUDIO_SINK_VIBRA,
		**/
	case 26:
		AUDCTRL_SetPlayMute(AUDIO_SOURCE_UNDEFINED,
			Params[1], Params[2], 0); /* spk, mute flag 1 - mute */

		if (Params[2] == 0)
			BCM_AUDIO_DEBUG("Set speaker un-mute\n");
		else
			BCM_AUDIO_DEBUG("Set speaker mute\n");

		break;

		/* AT*MAUDTST=32, p2, p3
		!
		! Function:    Mute / un-mute microphone of an operation.
		!
		! Params:      p2 : operation
		!                      p3 = mute flag
		!                                1 - mute
		!                                0 - un-mute
		**/
	case 32:
		AUDCTRL_SetTelephonyMicMute(AUDIO_SOURCE_UNDEFINED,
			(Boolean) Params[2]);
		break;

	case 33:
		AUDCTRL_SetTelephonySpkrVolume(AUDIO_SINK_UNDEFINED,
			Params[1], AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN);
		break;
	case 34:
		AUDCTRL_SetPlayVolume(AUDIO_SOURCE_I2S,
			AUDIO_SINK_LOUDSPK,
			AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
			Params[1], 0, 0);
		break;
	case 35:
		AUDCTRL_SetPlayVolume(AUDIO_SOURCE_I2S,
			AUDIO_SINK_HEADSET,
			AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE,
			Params[1], 0, 0);
		break;
/*
 * There is errors in this case. XXX_Get_CP_AudioMode() does not
 * work. It cannot read audio mode from CP image.
 * I this this is not needed.
 * Comment it out for now. And it may be removed.
	case 37:
#if defined(USE_NEW_AUDIO_PARAM)
        Params[1] = AUDDRV_Get_CP_AudioMode() / AUDIO_MODE_NUMBER;
        Params[2] = AUDDRV_Get_CP_AudioMode() % AUDIO_MODE_NUMBER;
		BCM_AUDIO_DEBUG("CP audio app %d, mode %d\n",
				Params[1], Params[2]);
#else
        Params[1] = AUDDRV_Get_CP_AudioMode();
		BCM_AUDIO_DEBUG("CP audio mode %d\n",
				Params[1]);
#endif
		break;
*/
	case 100:
		if (Params[1] == 1) {
			BCM_AUDIO_DEBUG("Enable CAPH clock\n");
			csl_caph_ControlHWClock(TRUE);
		} else if (Params[1] == 0) {
			BCM_AUDIO_DEBUG("Disable CAPH clock\n");
			csl_caph_ControlHWClock(FALSE);
		}
		break;

	case 101:
		Params[0] = (Int32) csl_caph_QueryHWClock();
		BCM_AUDIO_DEBUG("csl_caph_QueryHWClock %ld.\n", Params[0]);
		break;

	case 121:
	/*at*maudtst=121,x,y  x=0: EXT_SPEAKER_PGA, x=1:EXT_SPEAKER_PREPGA,
		x=2: MIC_PGA, y: gain value register value(enum value) */
	/*PCG on Rhea platform uses at*maudmode=100,3, */
		break;

	case 500:		/*at*maudtst=500,*/
	   {
		char *address;
		unsigned int value = 0, gain1 = 0, gain2 = 0,
			gain3 = 0, gain4 = 0;
		unsigned int index;

#define CHAL_CAPH_SRCM_MAX_FIFOS      15
#define SRCMIXER_A	SRCMIXER_SRC_M1D1_CH1M_GAIN_CTRL_OFFSET
#define SRCMIXER_B	SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_OFFSET
/* 4 bytes */
#define SRCMIXER_MIX_CH_GAIN_CTRL_OFFSET  ((SRCMIXER_A - SRCMIXER_B)/4)

		address = (char *)ioremap_nocache(
		(UInt32)(AUDIOH_BASE_ADDR + AUDIOH_AUDIORX_VRX1_OFFSET),
			sizeof(UInt32));
		if (!address) {
			pr_err(" address ioremap failed\n");
			return 0;
		}

		value = ioread32(address);

		iounmap(address);

		gain1 = value;
		gain1 &= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_MASK);
		gain1 >>= (AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_GAINCTRL_SHIFT);

		pr_err("AUDIOH_AUDIORX_VRX1=0x%x, AMIC_PGA=0x%x\n",
			value, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(AUDIOH_BASE_ADDR + AUDIOH_VIN_FILTER_CTRL_OFFSET),
			sizeof(UInt32));
		if (!address) {
			pr_err(" address ioremap failed\n");
			return 0;
		}

		value = ioread32(address);

		iounmap(address);

		gain1 = value;
		gain1 &= (AUDIOH_VIN_FILTER_CTRL_DMIC1_CIC_BIT_SEL_MASK);
		gain1 >>= (AUDIOH_VIN_FILTER_CTRL_DMIC1_CIC_BIT_SEL_SHIFT);

		gain2 = value;
		gain2 &= (AUDIOH_VIN_FILTER_CTRL_DMIC1_CIC_FINE_SCL_MASK);
		gain2 >>= (AUDIOH_VIN_FILTER_CTRL_DMIC1_CIC_FINE_SCL_SHIFT);

		gain3 = value;
		gain3 &= (AUDIOH_VIN_FILTER_CTRL_DMIC2_CIC_BIT_SEL_MASK);
		gain3 >>= (AUDIOH_VIN_FILTER_CTRL_DMIC2_CIC_BIT_SEL_SHIFT);

		gain4 = value;
		gain4 &= (AUDIOH_VIN_FILTER_CTRL_DMIC2_CIC_FINE_SCL_MASK);
		gain4 >>= (AUDIOH_VIN_FILTER_CTRL_DMIC2_CIC_FINE_SCL_SHIFT);

		pr_err(" AUDIOH_VIN_FILTER_CTRL=0x%x\n", value);
		pr_err("DMIC1_CIC_BIT_SEL=0x%x, DMIC1_CIC_FINE=0x%x\n",
			gain1, gain2);
		pr_err("DMIC2_CIC_BIT_SEL=0x%x, DMIC2_CIC_FINE_SCL=0x%x\n",
			gain3, gain4);

		address = (char *)ioremap_nocache(
		(UInt32)(AUDIOH_BASE_ADDR + AUDIOH_NVIN_FILTER_CTRL_OFFSET),
			sizeof(UInt32));
		if (!address) {
			pr_err(" address ioremap failed\n");
			return 0;
		}

		value = ioread32(address);

		iounmap(address);

		gain1 = value;
		gain1 &= (AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_BIT_SEL_MASK);
		gain1 >>= (AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_BIT_SEL_SHIFT);

		gain2 = value;
		gain2 &= (AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_FINE_SCL_MASK);
		gain2 >>= (AUDIOH_NVIN_FILTER_CTRL_DMIC3_CIC_FINE_SCL_SHIFT);

		gain3 = value;
		gain3 &= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_BIT_SEL_MASK);
		gain3 >>= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_BIT_SEL_SHIFT);

		gain4 = value;
		gain4 &= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_FINE_SCL_MASK);
		gain4 >>= (AUDIOH_NVIN_FILTER_CTRL_DMIC4_CIC_FINE_SCL_SHIFT);

		pr_err(" AUDIOH_VIN_FILTER_CTRL=0x%x\n", value);
		pr_err("DMIC3_CIC_BIT_SEL=0x%x, DMIC3_CIC_FINE=0x%x\n",
		gain1, gain2);
		pr_err("DMIC4_CIC_BIT_SEL=0x%x, DMIC4_CIC_FINE_SCL=0x%x\n",
		gain3, gain4);

		address = (char *)ioremap_nocache((UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		if (!address) {
			pr_err(" address ioremap failed\n");
			return 0;
		}
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_SHIFT);

		pr_err(
		"MIXER 1 D0, channel 1, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
			(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 2, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH3M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 3, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH4M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 4, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH5L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 5 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH5R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 5 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH6L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 6 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH6R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 6 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH7L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 7 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D0_CH7R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D0, channel 7 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		/********/
		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH1M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH1M_GAIN_CTRL_SRC_M1D0_CH1M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 1, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH2M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 2, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH3M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 3, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH4M_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 4, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
			gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH5L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 5 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH5R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 5 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH6L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 6 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH6R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 6 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH7L_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 7 left, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		address = (char *)ioremap_nocache(
		(UInt32)(SRCMIXER_BASE_ADDR +
			SRCMIXER_SRC_M1D1_CH7R_GAIN_CTRL_OFFSET),
			sizeof(UInt32));
		value = ioread32(address);
		iounmap(address);

		gain1 = value;
		gain1 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_MASK);
		gain1 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_GAIN_RAMPSTEP_SHIFT);

		gain2 = value;
		gain2 &=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_MASK);
		gain2 >>=
	(SRCMIXER_SRC_M1D0_CH2M_GAIN_CTRL_SRC_M1D0_CH2M_TARGET_GAIN_SHIFT);

		pr_err(
	"MIXER 1 D1, channel 7 right, Target_Gain=0x%x, Gain_RampStep=0x%x\n",
		gain2, gain1);

		/**********/
	for (index = 0x00000160; index <= 0x00000270; index = index + 4) {
			address = (char *)ioremap_nocache(
				(UInt32)(SRCMIXER_BASE_ADDR + index),
				sizeof(UInt32));
			value = ioread32(address);
			iounmap(address);
			pr_err(
			"SRCMIXER_BASE_ADDR + offset 0x%x, value = 0x%x\n",
				index, value);
		}

		} /* 500 cmd */
		break;
	case 1000:		/*at*maudtst=1000,addr,len */
		{
		u32 value, index, phy_addr, size;
		char *addr;

			if (Params[2] == 0)
				size = 1;
			else
				size = Params[2] >> 2;

			phy_addr = Params[1];

			addr = ioremap_nocache(phy_addr, sizeof(u32));
			if (!addr) {
				pr_err(" addr ioremap failed\n");
				return 0;
			}

			pr_err(
		"Read phy_addr 0x%08x (virtual %p), size = 0x%08x bytes\n",
				phy_addr, addr, size << 2);
			iounmap(addr);

			for (index = 0; index < size; index++) {
				addr = ioremap_nocache(phy_addr, sizeof(u32));
				if (addr) {
					value = ioread32(addr);
					iounmap(addr);
				pr_err("[%08x] = %08x\n", phy_addr, value);
				}
				phy_addr += 4;
			}
		}
		break;

	default:
		BCM_AUDIO_DEBUG("%s Not supported command\n", __func__);
		return -1;
	}
	return 0;
}

/*
Description:
AT command handler, handle command AT commands at*maudvol=P1,P2,P3
Parameters:
	pChip -- Pointer to chip data structure
	ParamCount -- Count of parameter array
	Params  --- P1,P2,...,P6
*/
int AtMaudVol(brcm_alsa_chip_t *pChip, Int32 ParamCount, Int32 *Params)
{
	int *pVolume;
	int mode, vol;

	BCM_AUDIO_DEBUG("%s P1-P6=%ld %ld %ld %ld %ld %ld cnt=%ld\n",
			__func__, Params[0], Params[1], Params[2],
			Params[3], Params[4], Params[5], ParamCount);

	csl_caph_ControlHWClock(TRUE);

	switch (Params[0]) {
	case 6:		/*at*maudvol=6 */
		mode = AUDCTRL_GetAudioMode();
		/*Get volume. Range -36 ~ 0 dB in Driver and DSP: */
		Params[0] =
		    AUDCTRL_GetTelephonySpkrVolume(AUDIO_GAIN_FORMAT_mB);
		Params[0] = Params[0] / 100;	/* dB */
		Params[0] += AUDIO_GetParmAccessPtr()[mode].voice_volume_max;
/* Range 0~36 dB shown in PCG */
/*
pVolume = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -1].ctlLine[mode].iVolume;
Params[0] = pVolume[0];
*/
		BCM_AUDIO_DEBUG("%s pVolume[0] %ld\n", __func__, Params[0]);
		return 0;

	case 7:		/* at*maudvol=7,x    Range 0~36 dB in PCG */
		mode = AUDCTRL_GetAudioMode();
	/*
	mode = pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL-1].iLineSelect[1];
	*/
		pVolume =
		    pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				     1].ctlLine[mode].iVolume;
		pVolume[0] = Params[1];
		pVolume[1] = Params[1];
		vol = Params[1];
		vol -= AUDIO_GetParmAccessPtr()[mode].voice_volume_max;
		/* Range -36 ~ 0 dB in DSP */
		AUDCTRL_SetTelephonySpkrVolume(AUDIO_SINK_UNDEFINED,
		(vol * 100), AUDIO_GAIN_FORMAT_mB);

		BCM_AUDIO_DEBUG("%s pVolume[0] %d mode=%d vol %d\n",
				__func__, pVolume[0], mode, vol);
		return 0;

	default:
		BCM_AUDIO_DEBUG("%s Unsupported cmd %ld\n", __func__,
				Params[0]);
		break;
	}
	return -1;
}

/*
Description:
	Kernel AT command handler and gernal purpose controls
Parameters
	cmdIndex -- command index coresponding to AT comand
	ParamCount       -- count of array Params
	Params -- Parameter array
Return
	0 on success, -1 otherwise
*/
int AtAudCtlHandler_put(Int32 cmdIndex, brcm_alsa_chip_t *pChip,
			Int32 ParamCount, Int32 *Params)
{
	int rtn = 0;

	BCM_AUDIO_DEBUG(
	"AT-AUD-put ctl=%ld ParamCount= %ld [%ld %ld %ld %ld %ld %ld %ld]\n",
	     cmdIndex, ParamCount, Params[0], Params[1], Params[2], Params[3],
	     Params[4], Params[5], Params[6]);

	switch (cmdIndex) {
	case AT_AUD_CTL_INDEX:
		{
			int count =
			    sizeof(pChip->i32AtAudHandlerParms) /
			    sizeof(pChip->i32AtAudHandlerParms[0]);
			if (count > ParamCount)
				count = ParamCount;
			memcpy(pChip->i32AtAudHandlerParms, Params,
			       sizeof(pChip->i32AtAudHandlerParms[0]) * count);
			return 0;
		}
	case AT_AUD_CTL_DBG_LEVEL:
		gAudioDebugLevel = Params[0];
		return 0;
	case AT_AUD_CTL_HANDLER:
		break;
	default:
		return -1;

	}

	switch (Params[0]) {

	case AT_AUD_HANDLER_MODE:
		rtn = AtMaudMode(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_VOL:
		rtn = AtMaudVol(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_TST:
		rtn = AtMaudTst(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_LOG:
		rtn = AtMaudLog(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_LBTST:
		rtn = AtMaudLoopback(pChip, ParamCount - 1, &Params[1]);
		break;
	default:
		BCM_AUDIO_DEBUG("%s Unsupported handler %ld\n", __func__,
				Params[0]);
		rtn = -1;
		break;
	}

	return rtn;
}

/*
Description:
	Kernel AT command handler and gernal purpose controls
Parameters
	cmdIndex -- command index coresponding to AT comand
	ParamCount       -- count of array Params
	Params -- Parameter array
Return
0 on success, -1 otherwise
*/
int AtAudCtlHandler_get(Int32 cmdIndex, brcm_alsa_chip_t *pChip,
			Int32 ParamCount, Int32 *Params)
{
	int count =
	    sizeof(pChip->i32AtAudHandlerParms) /
	    sizeof(pChip->i32AtAudHandlerParms[0]);
	int rtn = 0;

	BCM_AUDIO_DEBUG(
"AT-AUD-get ctl=%ld ParamCount= %ld [%ld %ld %ld %ld %ld %ld %ld]\n",
	     cmdIndex, ParamCount, Params[0], Params[1], Params[2], Params[3],
	     Params[4], Params[5], Params[6]);

	switch (cmdIndex) {

	case AT_AUD_CTL_HANDLER:
	case AT_AUD_CTL_INDEX:
		{
			if (count > ParamCount)
				count = ParamCount;
			memcpy(Params, pChip->i32AtAudHandlerParms,
			       sizeof(pChip->i32AtAudHandlerParms[0]) * count);
			if (cmdIndex == AT_AUD_CTL_INDEX)
				return 0;
			else
				break;
		}
	case AT_AUD_CTL_DBG_LEVEL:
		Params[0] = gAudioDebugLevel;
		return 0;
	default:
		return -1;

	}

	switch (Params[0]) {
	case AT_AUD_HANDLER_MODE:
		rtn = AtMaudMode(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_VOL:
		rtn = AtMaudVol(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_TST:
		rtn = AtMaudTst(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_LOG:
		rtn = AtMaudLog(pChip, ParamCount - 1, &Params[1]);
		break;
	case AT_AUD_HANDLER_LBTST:
		rtn = AtMaudLoopback(pChip, ParamCount - 1, &Params[1]);
		break;
	default:
		BCM_AUDIO_DEBUG("%s Unsupported handler %ld\n", __func__,
				Params[0]);
		rtn = -1;
		break;
	}

	return rtn;
}
