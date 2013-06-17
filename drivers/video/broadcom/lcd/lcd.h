/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software. The special  exception does not apply to any              */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#ifndef __LCD_H__
#define __LCD_H__

#include "dispdrv_common.h"
#include "dsi_timing.h"
#include "dispdrv_mipi_dcs.h"
#include "dispdrv_mipi_dsi.h"

enum modes {
	LCD_CMD_ONLY,
	LCD_VID_ONLY,
	LCD_CMD_VID_BOTH,
};

struct lcd_config {
	char name[DISPDRV_NAME_SZ];
	enum modes mode_supp;
	struct DSI_COUNTER *phy_timing;
	uint8_t max_lanes;
	uint32_t max_hs_bps;
	uint32_t max_lp_bps;
	uint8_t phys_width;
	uint8_t phys_height;
	DISPCTRL_REC_T *init_cmd_seq;
	DISPCTRL_REC_T *init_vid_seq;
	DISPCTRL_REC_T *slp_in_seq;
	DISPCTRL_REC_T *slp_out_seq;
	DISPCTRL_REC_T *scrn_on_seq;
	DISPCTRL_REC_T *scrn_off_seq;
	DISPCTRL_REC_T *id_seq;
	bool verify_id;
	void (*updt_win_fn)(char *buff, DISPDRV_WIN_t *p_win);
	uint32_t updt_win_seq_len;
	bool vid_cmnds; /* TURN_ON SHUT_DOWN*/
	bool vburst;
	bool cont_clk;
	uint8_t hs, hbp, hfp;
	uint8_t vs, vbp, vfp;
};

#endif
