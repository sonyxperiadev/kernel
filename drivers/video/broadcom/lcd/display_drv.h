/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/display_drv.h
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
#ifndef _DISPLAY_DRV_H_
#define _DISPLAY_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dsi_timing.h"

/**
*
*  Display Driver Handle
*
*****************************************************************************/
	typedef void *DISPDRV_HANDLE_T;

/**
*
*  Display State Control
*
*****************************************************************************/
	typedef enum {
		CTRL_MIN,

		CTRL_PWR_OFF,	/*  PWR Off  : in reset */
		CTRL_PWR_ON,	///<  PWR On   : not in reset, init, screen off
		CTRL_SLEEP_IN,	///<  Sleep-in : Screen Off, low power mode
		CTRL_SLEEP_OUT,	///<  Sleep-out: full power, screen Off
		CTRL_SCREEN_ON,	/*  Sleep-out: full power, screen On */
		CTRL_SCREEN_OFF,	///<  Sleep-out: full power, screen Off

		CTRL_MAX,
	} DISPLAY_POWER_STATE_T;


/**
*
*  Display Driver's Frame Buffer Color Format
*
*****************************************************************************/
	typedef enum {
		DISPDRV_FB_FORMAT_MIN,	///<  Min Number

		DISPDRV_FB_FORMAT_RGB565,	///<  RG5565 2Bpp
		DISPDRV_FB_FORMAT_xRGB8888,	/*  xRGB8888 4Bpp */
		DISPDRV_FB_FORMAT_xBGR8888,	/*  xBGR8888 4Bpp */

		DISPDRV_FB_FORMAT_MAX,	///<  MAX Number
	} DISPDRV_FB_FORMAT_T;


/**
*
*  Display Driver CallBack Results
*
*****************************************************************************/
	typedef enum {
		DISPDRV_CB_RES_MIN,	///< Min Number
		DISPDRV_CB_RES_OK,	///< No Errors
		DISPDRV_CB_RES_ERR,	///< Error During Update
		DISPDRV_CB_RES_MAX,	///< Max Number
	} DISPDRV_CB_RES_T;

	typedef struct {
		u32 l;
		u32 t;
		u32 r;
		u32 b;
		u32 w;
		u32 h;
		u32 mode; /*0=android use case, !=0 win has 0 offset in fb */
	} DISPDRV_WIN_t;

#define dispdrv2cmIn(p) p == KONA_CM_I_RGB565  ? LCD_IF_CM_I_RGB565P : \
		p == KONA_CM_I_XRGB888 ? LCD_IF_CM_I_RGB888U : \
		LCD_IF_CM_I_INV

#define dispdrv2cmOut(p) p ==KONA_CM_O_RGB565 ? LCD_IF_CM_O_RGB565 : \
		p ==KONA_CM_O_RGB565_DSI_VM ? LCD_IF_CM_O_RGB565_DSI_VM:\
		p ==KONA_CM_O_RGB666 ? LCD_IF_CM_O_RGB666 : \
		p ==KONA_CM_O_RGB888 ? LCD_IF_CM_O_RGB888 : \
		LCD_IF_CM_O_INV


/**
*
*  Display information structure
*
*****************************************************************************/
	typedef struct {
		char *name;
		char *reg_name;
		struct hw_rst_info *rst;
		bool vmode;
		bool vburst;
		bool vid_cmnds;
		bool cmnd_LP;
		bool te_ctrl;
		uint16_t width;
		uint16_t height;
		uint8_t lanes;
		uint8_t phys_width;
		uint8_t phys_height;
		uint8_t fps;
		DISPDRV_FB_FORMAT_T in_fmt;
		DISPDRV_FB_FORMAT_T out_fmt;
		uint8_t Bpp;
		char *init_seq;
		char *slp_in_seq;
		char *slp_out_seq;
		char *scrn_on_seq;
		char *scrn_off_seq;
		char *id_seq;
		char *win_seq;
		void (*updt_win_fn)(char *buff, DISPDRV_WIN_t *p_win);
		uint8_t updt_win_seq_len;
		struct DSI_COUNTER phy_timing[18];
		uint8_t hs, hbp, hfp;
		uint8_t vs, vbp, vfp;

		uint32_t hs_bps;
		uint32_t lp_bps;
		int desense_offset;
		void (*vsync_cb)(void);
		bool cont_clk;
	} DISPDRV_INFO_T;


/**
*
*  API CallBack Function
*
*****************************************************************************/
	typedef void (*DISPDRV_CB_T) (DISPDRV_CB_RES_T res);

/**
*
*  Display Driver structure definition
*
*  DISPLAY Driver Interface Return Values: res=0 OK  res !=0  ERR
*
*****************************************************************************/
	typedef struct {
		Int32(*init) (DISPDRV_INFO_T *display_info,
			DISPDRV_HANDLE_T *handle);
		Int32(*exit) (DISPDRV_HANDLE_T handle);
		Int32(*open) (DISPDRV_HANDLE_T handle);
		Int32(*close) (DISPDRV_HANDLE_T handle);
		Int32(*start) (DISPDRV_HANDLE_T handle,
			struct pi_mgr_dfs_node *dfs_node);
		Int32(*stop) (DISPDRV_HANDLE_T handle,
			struct pi_mgr_dfs_node *dfs_node);
		Int32(*power_control) (DISPDRV_HANDLE_T handle,
			DISPLAY_POWER_STATE_T powerState);
		Int32(*update_no_os) (DISPDRV_HANDLE_T handle, void *buff,
			DISPDRV_WIN_t *p_win);
		Int32(*update) (DISPDRV_HANDLE_T handle, void *buff,
			DISPDRV_WIN_t *p_win, DISPDRV_CB_T apiCb);
		Int32(*reset_win) (DISPDRV_HANDLE_T handle);
	} DISPDRV_T;

#ifdef __cplusplus
}
#endif
#endif				//_DISPLAY_DRV_H_
