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

#define GPIO_INIT_FIELD_NUM 5
#define GPIO_INIT_REC_NUM 64
#define IMEI_SIZE					9

#define NUM_OF_FREQ_OFFSETS			8

/** Number of byte of Terminal Profile data defined in Sysparm. If the number of bytes is larger
 * than the "MAX_PROFILE_ARRAY_SIZE" defined in USIMAP/SIMAP, the extra bytes are truncated
 * and are not sent to the USIM/SIM.
 */
#define MAX_TERMINAL_PROFILE_ARRAY_SIZE  30

#if defined(RF_DESENSED)
#define DESENSE_TABLE_ROW_SIZE 5
#define DESENSE_TABLE_COLUMN_SIZE 5
#endif

/** TX FREQ SECTION PER BAND */
#define N_FREQ_SECTIONS				8

typedef struct {
	char *name;
	void *ptr;
	unsigned int size;
	unsigned int flag;
} SysparmIndex_t;

UInt16 SYSPARM_GetLogFormat(void);
SysAudioParm_t *APSYSPARM_GetAudioParmAccessPtr(void);
SysIndMultimediaAudioParm_t *APSYSPARM_GetMultimediaAudioParmAccessPtr(void);
int APSYSPARM_RefreshAudioParm(unsigned int addr);

UInt16 SYSPARM_GetDefault4p2VoltReading(void);
UInt16 SYSPARM_GetActual4p2VoltReading(void);
UInt16 SYSPARM_GetBattLowThresh(void);
UInt16 SYSPARM_GetActualLowVoltReading(void);

UInt8 *SYSPARM_GetGPIO_Default_Value(UInt8 gpio_index);

/* retrieves IMEI string from sysparms only (doesn't check MS database */
/* value, as is done in CP sysparm.c) */
Boolean SYSPARM_GetImeiStr(UInt8 *inImeiStrPtr);

#endif /* _BCM_FUSE_SYSPARM_CIB_H_ */
