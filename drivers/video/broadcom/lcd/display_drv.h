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

/** 
 * @addtogroup DisplayGroup
 * @{
 */

/**
*
*  Display Driver Interface Revision
*
*****************************************************************************/
#define DISPDRV_INTERFACE_REV_MAJ    0
#define DISPDRV_INTERFACE_REV_MIN    1

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

		CTRL_PWR_OFF,	///<  PWR Off  : in reset 
		CTRL_PWR_ON,	///<  PWR On   : not in reset, init, screen off
		CTRL_SLEEP_IN,	///<  Sleep-in : Screen Off, low power mode
		CTRL_SLEEP_OUT,	///<  Sleep-out: full power, screen Off
		CTRL_SCREEN_ON,	///<  Sleep-out: full power, screen On 
		CTRL_SCREEN_OFF,	///<  Sleep-out: full power, screen Off

		CTRL_MAX,
	} DISPLAY_POWER_STATE_T;

/**
*
*  Display Bus types
*
*****************************************************************************/
	typedef enum {
		DISPLAY_BUS_MIN,	///<  Min Number

		DISPLAY_BUS_LCDC,	///<  LCDC interf buses (Z80/M68/DBI-B/DBI-C)
		DISPLAY_BUS_DSI,	///<  DSI  bus
		DISPLAY_BUS_SMI,	///<  SMI  interf buses (Z80/M68/DBI Parallel)
		DISPLAY_BUS_SPI,	///<  SPI  bus
		DISPLAY_BUS_DISPC,	///<  DISPC  bus   

		DISPLAY_BUS_MAX,	///<  MAX Number
	} DISPLAY_BUS_T;

/**
*
*  Display Types
*
*****************************************************************************/
	typedef enum {
		DISPLAY_TYPE_MIN,	///<  Min Number

		DISPLAY_TYPE_LCD_STD,	///<  Standard LCD Type

		DISPLAY_TYPE_MAX,	///<  MAX Number
	} DISPLAY_TYPE_T;

/**
*
*  Display Driver's Frame Buffer Color Format 
*
*****************************************************************************/
	typedef enum {
		DISPDRV_FB_FORMAT_MIN,	///<  Min Number

		DISPDRV_FB_FORMAT_RGB565,	///<  RG5565 2Bpp
		DISPDRV_FB_FORMAT_RGB888_U,	///<  RGB888 4Bpp, Unpacked xRGB 

		DISPDRV_FB_FORMAT_MAX,	///<  MAX Number
	} DISPDRV_FB_FORMAT_T;

/**
*
*  Display information structure
*
*     M:  Mandatory 
*     NU: Currently Not Used By Architecture (set to 0)
*****************************************************************************/
	typedef struct {
		DISPLAY_TYPE_T type;	///< M  display type
		UInt32 width;	///< M  width
		UInt32 height;	///< M  height
		DISPDRV_FB_FORMAT_T input_format;	///< M  frame buffer format
		DISPLAY_BUS_T bus_type;	///< NU display bus type
		UInt32 Bpp;	///< M  bytes per pixel
		UInt32 phys_width;	///< physical width dimension in mm
		UInt32 phys_height;	///< physical height dimension in mm
	} DISPDRV_INFO_T;

/**
*
*  Display Driver Supported Features
*
*****************************************************************************/
	typedef enum {
		DISPDRV_SUPPORT_NONE = 0x0,
	} DISPDRV_SUPPORT_FEATURES_T;

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
		u32 mode;	/*0=android use case, !=0 win has 0 offset in fb */
	} DISPDRV_WIN_t;

/* Convert dispdrv platform(boot) init values to dispdrv interface values */
#define dispdrv2busType(p) p == RHEA_BUS_SMI ? DISPLAY_BUS_SMI : \
		p == RHEA_BUS_DSI ? DISPLAY_BUS_DSI : DISPLAY_BUS_MIN

#define dispdrv2busNo(p) p == RHEA_BUS_0 ? 0 : \
		p == RHEA_BUS_1 ? 1 : 2

#define dispdrv2busCh(p) p == RHEA_BUS_CH_0 ? 0 : \
		p == RHEA_BUS_CH_1 ? 1 : 2

#define dispdrv2busW(p) p == RHEA_BUS_WIDTH_08 ? 8:   \
		p == RHEA_BUS_WIDTH_09 ? 9:   \
		p == RHEA_BUS_WIDTH_16 ? 16 : \
		p == RHEA_BUS_WIDTH_18 ? 18 : 0

#define dispdrv2busTE(p) p == RHEA_TE_IN_0_LCD  ? TE_VC4L_IN_0_LCD  : \
		p == RHEA_TE_IN_1_DSI0 ? TE_VC4L_IN_1_DSI0 : \
		p == RHEA_TE_IN_2_DSI1 ? TE_VC4L_IN_2_DSI1 : \
		TE_VC4L_IN_INV

#define dispdrv2cmIn(p) p == RHEA_CM_I_RGB565  ? LCD_IF_CM_I_RGB565P : \
		p == RHEA_CM_I_XRGB888 ? LCD_IF_CM_I_RGB888U : \
		LCD_IF_CM_I_INV

#define dispdrv2cmOut(p) p==RHEA_CM_O_RGB565 ? LCD_IF_CM_O_RGB565 : \
		p==RHEA_CM_O_RGB565_DSI_VM ? LCD_IF_CM_O_RGB565_DSI_VM:\
		p==RHEA_CM_O_RGB666 ? LCD_IF_CM_O_RGB666 : \
		p==RHEA_CM_O_RGB888 ? LCD_IF_CM_O_RGB888 : \
		LCD_IF_CM_O_INV

/**
*
*  API CallBack Function
*
*****************************************************************************/
// old API cb - will become obsolete,used by update DISP DRV API
	typedef void (*DISPDRV_CB_T) (DISPDRV_CB_RES_T res);
// new API cb - used by update_dma_os DISP DRV API
	typedef void (*DISPDRV_CB_API_1_1_T) (DISPDRV_CB_RES_T res, void *pFb);

/**
*
*  Display Driver structure definition
*
*  DISPLAY Driver Interface Return Values: res=0 OK  res !=0  ERR
*
*****************************************************************************/
	typedef struct {
		Int32(*init) (struct dispdrv_init_parms *parms,
			      DISPDRV_HANDLE_T *handle);
		Int32(*exit) (DISPDRV_HANDLE_T handle);
		Int32(*info) (DISPDRV_HANDLE_T handle,
			      const char **driverName, UInt32 *versionMajor,
			      UInt32 *versionMinor,
			      DISPDRV_SUPPORT_FEATURES_T *feature);
		Int32(*open) (DISPDRV_HANDLE_T handle);
		Int32(*close) (DISPDRV_HANDLE_T handle);
		const DISPDRV_INFO_T *(*get_info) (DISPDRV_HANDLE_T handle);
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
		 Int32(*set_brightness) (DISPDRV_HANDLE_T handle, UInt32 level);
		 Int32(*reset_win) (DISPDRV_HANDLE_T handle);
	} DISPDRV_T;

/** @} */

#ifdef __cplusplus
}
#endif
#endif				//_DISPLAY_DRV_H_
