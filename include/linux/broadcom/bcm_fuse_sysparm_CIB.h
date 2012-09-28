/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_fuse_sysparm_CIB.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _BCM_FUSE_SYSPARM_CIB_H_
#define _BCM_FUSE_SYSPARM_CIB_H_

#ifndef _RHEA_
#define _RHEA_
#endif

#define SYSPARM_INDEX_READY_INDICATOR   0x5059504D
#define MAX_SYSPARM_NAME_SIZE   128

#define   DSP_FEATURE_AHS_SID_UPDATE_BEC_CHECK	/* Additional BEC check to determine SID updaet frame in Rxqual_s calculation */
#ifdef UNDER_LINUX

#define   DSP_FEATURE_SUBBAND_NLP	/* Enable the compilation of ARM code specific to the subband_nlp feature in the DSP */
#define   DSP_FEATURE_SUBBAND_INF_COMP	/* Enable Infinite compression subband compressor */
#define   DSP_FEATURE_SUBBAND_INF_COMP_UL	/* Enable Infinite compression subband compressor sysparm/sheredmem init; not all the chips have ul and dl inf comp */
#define   DSP_FEATURE_EC_DYN_RNG_ENHANCEMENT	/* Enable EC dynamic range enhancements in the DSP */
#define   DSP_FEATURE_SUBBAND_NLP_MARGIN_VECTOR	/* Change single variable subband_nlp_UL_margin and subband_nlp_noise_margin thresholds into vector */
#define   DSP_FEATURE_CLASS_33	/* Enable class 33 feature in the DSP */
#define   DSP_FEATURE_NEW_FACCH_MUTING
#define   DSP_FEATURE_NULLPAGE_AUTOTRACK
#define   DSP_FEATURE_BT_PCM
#define   DSP_FEATURE_AFC_FB
#define   DSP_FEATURE_BB_RX_ADV	/* DSP BB advanced RX feature */
#define   DSP_FEATURE_STACK_DEPTH_CHECKING

#define   DSP_FEATURE_NULLPAGE_AUTOTRACK
#define   DSP_FEATURE_GAIN_DL_UL_NON_JUNO
#define   DSP_FEATURE_USB_HEADSET_GAIN
#define   DSP_FEATURE_AAC_LC_ENCODE_MIC_INPUT
#define   DSP_FEATURE_OTD_SNR_CHECK	/* OTD reported in SCH is updated when passing SNR threshold. */
#define   DSP_FEATURE_NORX0	/* Disable 1st RX slot so the previous frame can increase by one slot for search */
#define   DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#define   DSP_FEATURE_OMEGA_VOICE
#define   DSP_FEATURE_FR_MUTE_FRAME
#include <linux/broadcom/chip_version.h>

#endif // UNDER_LINUX

#include "audio_tuning.h"
#include "sysparm_shared.h"

typedef struct {
	char *name;
	void *ptr;
	unsigned int size;
	unsigned int flag;
} SysparmIndex_t;

int APSYSPARM_RefreshAudioParm(unsigned int addr);

SysAudioParm_t __iomem *APSYSPARM_GetAudioParmAccessPtr(void);

#if defined(USE_NEW_AUDIO_MM_PARAM)
SysMultimediaAudioParm_t __iomem *
APSYSPARM_GetMultimediaAudioParmAccessPtr(void);
SysIndMultimediaAudioParm_t __iomem *
APSYSPARM_GetIndMultimediaAudioParmAccessPtr(void);
#else
SysIndMultimediaAudioParm_t __iomem *
APSYSPARM_GetMultimediaAudioParmAccessPtr(void);
#endif

#endif /* _BCM_FUSE_SYSPARM_CIB_H_ */
