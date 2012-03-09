/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   csl_voip.h
*
*   @brief  This file contains DSP VoIP interface
*
****************************************************************************/
#ifndef _CSL_VOIP_H_
#define _CSL_VOIP_H_

/* ---- Include Files ----------------------------------------------------- */
#include "mobcom_types.h"
#include "shared.h"

extern AP_SharedMem_t *vp_shared_mem;

#define CSL_G711_FRAME_SIZE    40	/* packed 80 samples (10ms) frame */
#define	CSL_AMR_WB_FRAME_SIZE  32	/* Maximum size WB-AMR speech frame */

typedef enum {
	CSL_AMR_MR475 = 0,
	CSL_AMR_MR515 = 1,
	CSL_AMR_MR59 = 2,
	CSL_AMR_MR67 = 3,
	CSL_AMR_MR74 = 4,
	CSL_AMR_MR795 = 5,
	CSL_AMR_MR102 = 6,
	CSL_AMR_MR122 = 7,
	CSL_AMR_MRDTX = 8,
	CSL_AMR_N_MODES = 9,	/* number of (SPC) modes */
	/* bit4=1 - WB AMR, [bit3..0] is defined in Spec */
	CSL_AMR_WB_MODE_7k = 0x10,	/* 6.60  */
	CSL_AMR_WB_MODE_9k = 0x11,	/* 8.85  */
	CSL_AMR_WB_MODE_12k = 0x12,	/* 12.65 */
	CSL_AMR_WB_MODE_14k = 0x13,	/* 14.25 */
	CSL_AMR_WB_MODE_16k = 0x14,	/* 15.85 */
	CSL_AMR_WB_MODE_18k = 0x15,	/* 18.25 */
	CSL_AMR_WB_MODE_20k = 0x16,	/* 19.85 */
	CSL_AMR_WB_MODE_23k = 0x17,	/* 23.05 */
	CSL_AMR_WB_MODE_24k = 0x18,	/*23.85  */
	/* for FR/AMR, bit0=[0/1]=[voip_dtx_enable=0/1] */
	CSL_VOIP_PCM = 0x1000,	/* VOIP PCM 8KHz */
	CSL_VOIP_FR = 0x2000,	/* VOIP FR (GSM 06.10) */
	CSL_VOIP_AMR475 = 0x3000,	/* VOIP AMR475 */
	CSL_VOIP_AMR515 = 0x3100,	/* VOIP AMR515 */
	CSL_VOIP_AMR59 = 0x3200,	/* VOIP AMR59  */
	CSL_VOIP_AMR67 = 0x3300,	/* VOIP AMR67  */
	CSL_VOIP_AMR74 = 0x3400,	/* VOIP AMR74  */
	CSL_VOIP_AMR795 = 0x3500,	/* VOIP AMR795 */
	CSL_VOIP_AMR102 = 0x3600,	/* VOIP AMR102 */
	CSL_VOIP_AMR122 = 0x3700,	/* VOIP AMR122 */
	CSL_VOIP_G711_U = 0x4000,	/* VOIP G.711 u-law */
	CSL_VOIP_G711_A = 0x4100, /* VOIP G.711 A-law */
	    CSL_VOIP_PCM_16K = 0x5000,	/* VOIP PCM 16KHz */
	CSL_VOIP_AMR_WB_MODE_7k = 0x6000,	/* VOIP WB AMR6.60 */
	CSL_VOIP_AMR_WB_MODE_9k = 0x6100,	/* VOIP WB AMR8.85 */
	CSL_VOIP_AMR_WB_MODE_12k = 0x6200,	/* VOIP WB AMR12.65 */
	CSL_VOIP_AMR_WB_MODE_14k = 0x6300,	/* VOIP WB AMR14.25 */
	CSL_VOIP_AMR_WB_MODE_16k = 0x6400,	/* VOIP WB AMR15.85 */
	CSL_VOIP_AMR_WB_MODE_18k = 0x6500,	/* VOIP WB AMR18.25 */
	CSL_VOIP_AMR_WB_MODE_20k = 0x6600,	/* VOIP WB AMR19.85 */
	CSL_VOIP_AMR_WB_MODE_23k = 0x6700,	/* VOIP WB AMR23.05 */
	CSL_VOIP_AMR_WB_MODE_24k = 0x6800,	/* VOIP WB AMR23.85 */

	CSL_VOIP_DL_FLAG = 0x0080,	/* VOIP DL_FLAG */
	CSL_VOIP_UL_FLAG = 0x0040	/* VOIP UL_FLAG */
} CSL_VP_Mode_AMR_t;

typedef struct {		/* G.711 speech data for a 10ms frame */
	UInt16 frame_type;	/* 0 - good frame, 1 - bad frame */
	UInt16 payload[CSL_G711_FRAME_SIZE];
} CSL_VR_Frame_G711_t;

typedef struct {		/* linear PCM speech data for a 20ms frame */
	UInt16 frame_type;
	UInt16 amr_codec_mode;
	UInt16 param[CSL_AMR_WB_FRAME_SIZE];	/* MSB 16bit packing format */
} CSL_VR_Frame_AMR_WB_t;

typedef struct {
	UInt16 voip_vocoder;

	union {
		/* voip mode 1/5: PCM, 8KHz use 160, 16KHz use 320 */
		UInt16 frame_pcm[320];
		/* voip mode 2: [VAD, SP, dtx_enable|BFI, SID, TAF] (76w) */
		UInt16 frame_fr[79];
		/* voip mode 3: frametype, mode, coded_bit(MSB first) */
		UInt16 frame_amr[18];
		/* voip mode 4: G.711 encoded samples */
		CSL_VR_Frame_G711_t frame_g711[2];
		/* voip mode 6: AMR-WB */
		CSL_VR_Frame_AMR_WB_t frame_amr_wb;
	} voip_frame;
} CSL_VOIP_Buffer_t;

typedef enum {
	NB_AMR,
	WB_AMR,
	DTMF_EVENT
} DJB_CODEC_TYPE;

typedef struct {
	UInt32 RTPTimestamp;
	UInt8 *pFramePayload;
	UInt16 payloadSize;
	UInt8 frameIndex;
	UInt8 codecType;	/* DJB_CODEC_TYPE in UInt8 */
	UInt8 frameType;
	UInt8 frameQuality;
} DJB_InputFrame;

/**
 * @addtogroup CSL VoIP interface
 * @{
 */

/*********************************************************************/
/**
*
*   CSL_WriteDLVoIPData writes VoIP data to DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pSrc			(in)		source of the data to write
*
**********************************************************************/
void CSL_WriteDLVoIPData(UInt16 codec_type, UInt16 *pSrc);

/*********************************************************************/
/**
*
*   CSL_ReadULVoIPData reads VoIP data from DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pDst			(in)		destination of read data
*   @return   UInt8						size of data in bytes
*
**********************************************************************/
UInt8 CSL_ReadULVoIPData(UInt16 codec_type, UInt16 *pDst);

#ifdef VOLTE_SUPPORT

/*********************************************************************/
/**
*
*   DJB_Init initializes Jitter Buffer of VoLTE interface
*
**********************************************************************/
void DJB_Init(void);

/*********************************************************************/
/**
*
*   DJB_Init flushes Jitter Buffer for new stream
*
**********************************************************************/
void DJB_StartStream(void);

/*********************************************************************/
/**
*
*   DJB_PutFrame puts incoming frame into Jitter Buffer of VoLTE interface
*
*   @param    pInputFrame	(in)		input frame
*
**********************************************************************/
void DJB_PutFrame(DJB_InputFrame *pInputFrame);

#endif /* VOLTE_SUPPORT */

/** @} */

#endif /* _CSL_VOIP_H_ */
