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
	void (*init_fn)(void);
	int (*esd_check_fn)(void);
	DISPCTRL_REC_T *init_cmd_seq;
	DISPCTRL_REC_T *init_vid_seq;
	DISPCTRL_REC_T *slp_in_seq;
	DISPCTRL_REC_T *slp_out_seq;
	DISPCTRL_REC_T *scrn_on_seq;
	DISPCTRL_REC_T *scrn_off_seq;
	DISPCTRL_REC_T *id_seq;
	DISPCTRL_REC_T *special_mode_on_cmd_seq; /* Panel special mode */
	DISPCTRL_REC_T *special_mode_off_cmd_seq; /* Panel normal mode */
	DISPCTRL_REC_T *cabc_init_seq;
	DISPCTRL_REC_T *cabc_on_seq;
	DISPCTRL_REC_T *cabc_off_seq;
	bool cabc_enabled;
	bool special_mode_on; /* Turn special mode on or off */
	bool special_mode_panel; /* Panel supports special mode */
	bool verify_id;
	void (*updt_win_fn)(char *buff, DISPDRV_WIN_t *p_win);
	uint32_t updt_win_seq_len;
	bool vid_cmnds; /* TURN_ON SHUT_DOWN*/
	bool vburst;
	bool cont_clk;
	bool sync_pulses; /* Non burst mode with sync pulses or events */
	uint8_t hs, hbp, hfp, hbllp;
	uint8_t vs, vbp, vfp;
	bool clear_panel_ram;
	uint16_t clear_ram_row_start;
	uint16_t clear_ram_row_end;
	bool no_te_in_sleep;
};

void panel_write(UInt8 *buff);
void panel_read(UInt8 reg, UInt8 *rxBuff, UInt8 buffLen);

#endif
