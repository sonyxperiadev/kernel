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



#define LCD_BITS_PER_PIXEL      16
//#define LCD_BITS_PER_PIXEL      32
#define TEAR_LINE 500



//#define PANEL_BOE           0x61a511
#define PANEL_BOE           0x61bc11
#define LCD_DET 32

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
const char *LCD_panel_name = "S6D05A1X31 LCD";



// DISP DRV API - Display Info
static DISPDRV_INFO_T Disp_Info =
{
    DISPLAY_TYPE_LCD_STD,         // DISPLAY_TYPE_T          type;          
    320,                          // UInt32                  width;         
    480,                          // UInt32                  height;        
    //DISPDRV_FB_FORMAT_RGB888_U,   // DISPDRV_FB_FORMAT_T     input_format;
	DISPDRV_FB_FORMAT_RGB565,
    DISPLAY_BUS_DSI,              // DISPLAY_BUS_T           bus_type;
    0,                            // UInt32                  interlaced;    
    DISPDRV_DITHER_NONE,          // DISPDRV_DITHER_T        output_dither; 
    0,                            // UInt32                  pixel_freq;    
    0,                            // UInt32                  line_rate;     
};


DISPCTRL_REC_T power_on_seq_s5d05a1x31_cooperve_AUO[] =
{
	// Initial Sequence
	
	{DISPCTRL_WR_CMND, 0xF0,0}, // (PASSWARD1)
	{DISPCTRL_WR_DATA, 0, 0x5A},
	{DISPCTRL_WR_DATA, 0, 0x5A},	


	//power setting
	//Power Setting Sequence
	{DISPCTRL_WR_CMND, 0xF4,0}, // (PDISPCTRL_WRCTL)
	{DISPCTRL_WR_DATA, 0, 0x0A},
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x00},
	
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x15},
	{DISPCTRL_WR_DATA, 0, 0x6B},
	
	{DISPCTRL_WR_DATA, 0, 0x03}, 

	{DISPCTRL_WR_CMND, 0xF5,0}, // (VCMCTL)
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x47},	
	{DISPCTRL_WR_DATA, 0, 0x75},
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x00},
	
	{DISPCTRL_WR_DATA, 0, 0x04},	
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},	 
	{DISPCTRL_WR_DATA, 0, 0x04},	 

	{DISPCTRL_WR_CMND, 0xF6,0}, // (SRCCTL)
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},	
	{DISPCTRL_WR_DATA, 0, 0x05},
	{DISPCTRL_WR_DATA, 0, 0x03},	
	{DISPCTRL_WR_DATA, 0, 0x01},
	
	{DISPCTRL_WR_DATA, 0, 0x00},	 

	//Initialize sequence
	{DISPCTRL_WR_CMND, 0x35,0}, // (TEON)
	{DISPCTRL_WR_DATA, 0, 0x01},

	{DISPCTRL_WR_CMND, 0x36,0}, 
	{DISPCTRL_WR_DATA, 0, 0x40},

	{DISPCTRL_WR_CMND, 0x3A,0}, 
	{DISPCTRL_WR_DATA, 0, 0x77},  //Format RGB888

	{DISPCTRL_WR_CMND, 0xF7,0}, 
	{DISPCTRL_WR_DATA, 0, 0x40},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x10},
	{DISPCTRL_WR_DATA, 0, 0x12},
	{DISPCTRL_WR_DATA, 0, 0x00},

	{DISPCTRL_WR_CMND, 0xF8,0}, 
	{DISPCTRL_WR_DATA, 0, 0x11},
	{DISPCTRL_WR_DATA, 0, 0x00},

	{DISPCTRL_WR_CMND, 0xF2,0}, 
	{DISPCTRL_WR_DATA, 0, 0x3B},
	{DISPCTRL_WR_DATA, 0, 0x3A},
	{DISPCTRL_WR_DATA, 0, 0x03},
	{DISPCTRL_WR_DATA, 0, 0x04},
	{DISPCTRL_WR_DATA, 0, 0x02},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x00},
	{DISPCTRL_WR_DATA, 0, 0x54},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x08},
	{DISPCTRL_WR_DATA, 0, 0x08},

	//Gama setting
	{DISPCTRL_WR_CMND, 0xF0,0},
	{DISPCTRL_WR_DATA, 0, 0xA5},
	{DISPCTRL_WR_DATA, 0, 0xA5},


	{DISPCTRL_WR_CMND, 0x2C,0}, // ( RAM DISPCTRL_WRITE )
     //Sleep out
	{DISPCTRL_WR_CMND, 0x11,0},
	
	{DISPCTRL_SLEEP_MS, 0, 200}, // 120ms

	//Display on

	{DISPCTRL_WR_CMND, 0x29,0}, // (DISPON) 
	{DISPCTRL_SLEEP_MS, 0, 100}, // 120ms

	{DISPCTRL_WR_CMND_DATA,0x36,0x88},
	{DISPCTRL_WR_CMND_DATA, 0x3A,0x55}, //RGB565
	//{DISPCTRL_WR_DATA, 0, 0x77},  //Format RGB888

	{DISPCTRL_WR_CMND, 0x2C,0}, // ( RAM DISPCTRL_WRITE )
	
	{DISPCTRL_LIST_END, 0, 0}
};

#endif


