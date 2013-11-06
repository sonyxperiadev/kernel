/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_common.h
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

/* Requires the following header files before its inclusion in a c file
#include "csl_lcd.h"     
*/

#ifndef __DISPDRV_COMMON_H__
#define __DISPDRV_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
*
*  Disp Drv CallBack API Rev
*
*****************************************************************************/
#define DISP_DRV_CB_API_REV_1_0     0x00010000

	typedef enum {
		DRV_STATE_OFF = 0,
		DRV_STATE_INIT = 0x01234567,
		DRV_STATE_OPEN = 0x76543210,
	} DISP_DRV_STATE;

	typedef enum {
		STATE_PWR_OFF,	///<  PWR Off, in reset 
		STATE_SLEEP,	///<  Sleep-in , Screen Off
		STATE_SCREEN_ON,	/*  Sleep-out, Screen On */
		STATE_SCREEN_OFF,	///<  Sleep-out, Screen Off
	} DISP_PWR_STATE;

#define DISPCTRL_TAG_SLEEP	((uint8_t)~0)
#define DISPCTRL_TAG_GEN_WR	(DISPCTRL_TAG_SLEEP - 1)
/* Maximum packet size is limited to 253 */
#define DISPCTRL_MAX_DATA_LEN (DISPCTRL_TAG_GEN_WR - 1)

	typedef enum {
		DISPCTRL_LIST_END,	/* END OF COMMAND LIST */
		DISPCTRL_WR_CMND,	/* DCS write command */
		DISPCTRL_GEN_WR_CMND,	/* Generic write command */
		DISPCTRL_WR_DATA,	///< write data     
		DISPCTRL_SLEEP_MS,	///< SLEEP for <data> msec
	} DISPCTRL_T;

	typedef struct {
		DISPCTRL_T type;	///< display access control type
		UInt8 val;
	} DISPCTRL_REC_T, *pDISPCTRL_REC_T;

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// __DISPDRV_COMMON_H__
