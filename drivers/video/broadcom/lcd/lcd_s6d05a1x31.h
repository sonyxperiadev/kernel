/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/video/broadcom/displays/lcd_s6d05a1x01.h
*
* Unless you and Broadcom execute a separate DISPCTRL_WRitten software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior DISPCTRL_WRitten consent.
*******************************************************************************/

/****************************************************************************
*
*  lcd_tiama_s6d04h0.h
*
*  PURPOSE:
*    This is the LCD-specific code for a S6d05a1x01 module.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#ifndef __BCM_LCD_S6D05A1X31_COOPERVE_H
#define __BCM_LCD_S6D05A1X31_COOPERVE_H

//  LCD command definitions
#define PIXEL_FORMAT_RGB565  0x55   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB666  0x66   // for MPU & RGB interface
#define PIXEL_FORMAT_RGB888  0x77   // for MPU & RGB interface

#define RESET_SEQ 	{DISPCTRL_WR_CMND, 0x2A,0},\
	{DISPCTRL_WR_DATA, 0, (dev->col_start) >> 8},\
	{DISPCTRL_WR_DATA, 0, dev->col_start & 0xFF},\
	{DISPCTRL_WR_DATA, 0, (dev->col_end) >> 8},\
	{DISPCTRL_WR_DATA, 0, dev->col_end & 0xFF},\
	{DISPCTRL_WR_CMND, 0x2B,0},\
	{DISPCTRL_WR_DATA, 0, (dev->row_start) >> 8},\
	{DISPCTRL_WR_DATA, 0, dev->row_start & 0xFF},\
	{DISPCTRL_WR_DATA, 0, (dev->row_end) >> 8},\
	{DISPCTRL_WR_DATA, 0, dev->row_end & 0xFF},\
	{DISPCTRL_WR_CMND, 0x2C,0}

#define PANEL_DTC	0x6bc010
#define PANEL_AUO	0x6b4c10
#define PANEL_SHARP	0x6b1c10 

//const char *LCD_panel_name = "S6D04H0X20 LCD";
//const char *LCD_panel_name = "S6D05A1X31 LCD";



// DISP DRV API - Display Info
static DISPDRV_INFO_T Disp_Info =
{
	DISPLAY_TYPE_LCD_STD,         // DISPLAY_TYPE_T          type;          
	320,                          // UInt32                  width;         
	480,                          // UInt32                  height;        
	//DISPDRV_FB_FORMAT_RGB888_U,   // DISPDRV_FB_FORMAT_T     input_format; //@HW
	DISPDRV_FB_FORMAT_RGB565,
	DISPLAY_BUS_DSI,              // DISPLAY_BUS_T           bus_type;
  	2,			      	// Bpp;		 : !!! init may overwrite
};


NEW_DISPCTRL_REC_T power_on_seq_s5d05a1x31_cooperve_AUO[] =
{
	// Initial Sequence
	
	{DISPCTRL_WR_CMND_DATA, 3,{0xF0,0x5A,0x5A}}, // (PASSWARD1)
	{DISPCTRL_WR_CMND_DATA, 3,{0xF1,0x5A,0x5A}}, // (PASSWARD2)
	{DISPCTRL_WR_CMND_DATA, 20,{0xF2,0x3B,0x38,0x03,0x08,0x08,0x08,0x08,0x00,0x08,0x08,0x00,0x00,0x00,0x08,0x54,0x08,0x08,0x04,0x04}}, // (DISCTL)
	
	
		//power setting
	//Power Setting Sequence
	{DISPCTRL_WR_CMND_DATA, 12,{0xF4,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x6B,0x03}},
	{DISPCTRL_WR_CMND_DATA, 10, {0xF5,0x00,0x47,0x75,0x00,0x00,0x04,0x00,0x00,0x04}},  // (VCMCTL)
	{DISPCTRL_WR_CMND_DATA, 7, {0xF6,0x04,0x00,0x08,0x03,0x01,0x00}},// (SRCCTL)
	{DISPCTRL_WR_CMND_DATA, 6, {0xF7,0x48,0x80,0x10,0x02,0x00}},
		
	{DISPCTRL_WR_CMND_DATA, 3,{0xF8,0x11,0x00}},
	{DISPCTRL_WR_CMND_DATA, 2,{0xF9,0x24}},
	{DISPCTRL_WR_CMND_DATA, 17,{0xFA,0x23,0x00,0x0A,0x18,0x1E,0x22,0x29,0x1D,0x2A,0x2F,0x3A,0x3C,0x30,0x00,0x2A,0x00}},
	{DISPCTRL_WR_CMND_DATA, 2,{0xF9,0x22}},
	{DISPCTRL_WR_CMND_DATA, 17,{0xFA,0x30,0x10,0x08,0x1B,0x1B,0x1F,0x25,0x1A,0x26,0x24,0x25,0x22,0x2C,0x00,0x2A,0x00}},
	{DISPCTRL_WR_CMND_DATA, 2,{0xF9,0x21}},
	{DISPCTRL_WR_CMND_DATA, 17,{0xFA,0x30,0x10,0x0A,0x21,0x31,0x33,0x32,0x10,0x1D,0x20,0x21,0x21,0x20,0x00,0x2A,0x00}},
	
	{DISPCTRL_WR_CMND_DATA, 2,{0x3A,0x55}}, //Format RGB565
	//Initialize sequence
	{DISPCTRL_WR_CMND_DATA, 2,{0x35,0x00}}, // (TEON)
	{DISPCTRL_WR_CMND_DATA, 2,{0x36,0xD0}}, //RGB/BRG
	
	

	//{DISPCTRL_WR_CMND, 1,{0x21}}, // inversion on

	{DISPCTRL_WR_CMND, 1,{0x2C}}, // ( RAM DISPCTRL_WRITE )
     //Sleep out
	{DISPCTRL_WR_CMND, 1,{0x11}},
	
	{DISPCTRL_SLEEP_MS, 0, {200}}, // 120ms

	//Display on

	{DISPCTRL_WR_CMND, 1,{0x29}}, // (DISPON) 
	{DISPCTRL_SLEEP_MS, 0, {100}}, // 120ms	

	{DISPCTRL_WR_CMND, 1,{0x2C}}, // ( RAM DISPCTRL_WRITE )
	
	{DISPCTRL_LIST_END, 0, {0}}
};


DISPCTRL_REC_T enter_sleep_seq_AUO[] =
{
	{DISPCTRL_WR_CMND, 0, 0x10}, // (SLPIN)
	{DISPCTRL_SLEEP_MS, 0, 120},
	{DISPCTRL_LIST_END, 0, 0}
};

DISPCTRL_REC_T exit_sleep_seq_AUO[] =
{
	{DISPCTRL_WR_CMND, 0, 0x11}, // (SLPIN)
	{DISPCTRL_SLEEP_MS, 0, 120},
	{DISPCTRL_LIST_END, 0, 0}
};


#endif


