/******************************************************************************/
/* Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.           */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed to */
/*     you under the terms of the GNU General Public License version 2        */
/*    (the GPL), available at                                                 */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and to  */
/*     copy and distribute the resulting executable under terms of your       */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module.        */
/*     An independent module is a module which is not derived from this       */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine this */
/*     software in any way with any other Broadcom software provided under a  */
/*     license other than the GPL, without Broadcom's express prior written   */
/*     consent.                                                               */
/******************************************************************************/

/**
*
*  @file   caph_pcm.h
*
*
****************************************************************************/


#ifndef _CAPH_PCM_H
#define _CAPH_PCM_H

#include "csl_caph.h"
#include "csl_caph_dma.h"

#define	PCM_MAX_PLAYBACK_BUF_BYTES			(32*1024)
#define	PCM_MIN_PLAYBACK_PERIOD_BYTES		(256)
#define	PCM_MAX_PLAYBACK_PERIOD_BYTES		(PCM_MAX_PLAYBACK_BUF_BYTES/2)

#define	PCM_MAX_CAPTURE_BUF_BYTES			(32*1024)
#define	PCM_MIN_CAPTURE_PERIOD_BYTES		(4*1024)
#define	PCM_MAX_CAPTURE_PERIOD_BYTES		(PCM_MAX_CAPTURE_BUF_BYTES/2)

/*cpu_dai(i2s_dai) private dma data structure*/
struct caph_pcm_config {
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_SWITCH_CHNL_e sw;
};

#endif
