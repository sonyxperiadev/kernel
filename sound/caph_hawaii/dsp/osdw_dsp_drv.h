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
*   @file   csl_dsp_drv.h
*
*   @brief  This file DSP CSL OSDW definitions
*
****************************************************************************/
#ifndef _OSDW_DSP_DRV_H_
#define _OSDW_DSP_DRV_H_


#include "mobcom_types.h"

#ifndef BRCM_RTOS
#define assert(exp) BUG_ON(!(exp))
#else
#include "assert.h"
#error
#endif

void DSPDRV_Init(void);
void DSPDRV_DeInit(void);

void VPSHAREDMEM_TriggerRIPInt(void);


#endif /* _OSDW_DSP_DRV_H_ */
