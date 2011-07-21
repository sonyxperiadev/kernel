/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "dispdrv_common.h"
*/

//----------------------------------------------------
//--- HVGA LCD Controller RENESAS HVGA R61581 Controller
//----------------------------------------------------

#define MIPI_DCS_NOP									0x00
#define MIPI_DCS_SOFT_RESET								0x01
#define MIPI_DCS_GET_RED_CHANNEL						0x06
#define MIPI_DCS_GET_GREEN_CHANNEL						0x07
#define MIPI_DCS_GET_BLUE_CHANNEL						0x08
#define MIPI_DCS_GET_POWER_MODE			    			0x0A
#define MIPI_DCS_GET_ADDRESS_MODE		    			0x0B
#define MIPI_DCS_GET_PIXEL_FORMAT		    			0x0C
#define MIPI_DCS_GET_DISPLAY_MODE		    			0x0D
#define MIPI_DCS_GET_SIGNAL_MODE		    			0x0E
#define MIPI_DCS_GET_DIAGNOSTIC_RESULTS	    			0x0F
#define MIPI_DCS_ENTER_SLEEP_MODE		    			0x10
#define MIPI_DCS_EXIT_SLEEP_MODE		    			0x11
#define MIPI_DCS_ENTER_PARTIAL_MODE	 	    			0x12
#define MIPI_DCS_ENTER_NORMAL_MODE	 	    			0x13
#define MIPI_DCS_EXIT_INVERT_MODE		    			0x20
#define MIPI_DCS_ENTER_INVERT_MODE	 	    			0x21
#define MIPI_DCS_SET_GAMMA_CURVE	  	    			0x26
#define MIPI_DCS_SET_DISPLAY_OFF	  	    			0x28
#define MIPI_DCS_SET_DISPLAY_ON		  	    			0x29
#define MIPI_DCS_SET_COLUMN_ADDRESS  	    			0x2A
#define MIPI_DCS_SET_PAGE_ADDRESS	  	    			0x2B
#define MIPI_DCS_WRITE_MEMORY_START		    			0x2C
#define MIPI_DCS_WRITE_LUT				    			0x2D
#define MIPI_DCS_READ_MEMORY_START   	    			0x2E
#define MIPI_DCS_SET_PARTIAL_AREA	  	    			0x30
#define MIPI_DCS_SET_SCROLL_AREA	  	    			0x33
#define MIPI_DCS_SET_TEAR_OFF		  	    			0x34
#define MIPI_DCS_SET_TEAR_ON		  	    			0x35
#define MIPI_DCS_SET_ADDRESS_MODE   	    			0x36
#define MIPI_DCS_SET_SCROLL_START	  	    			0x37
#define MIPI_DCS_EXIT_IDLE_MODE			    			0x38
#define MIPI_DCS_ENTER_IDLE_MODE	 	    			0x39
#define MIPI_DCS_SET_PIXEL_FORMAT	  	    			0x3A
#define MIPI_DCS_WRITE_MEMORY_CONTINUE	    			0x3C
#define MIPI_DCS_READ_MEMORY_CONTINUE 	    			0x3E
#define MIPI_DCS_SET_TEAR_SCANLINE	  	    			0x44
#define MIPI_DCS_GET_SCANLINE			    			0x45
#define MIPI_DCS_READ_DDB_START			    			0xA1
#define MIPI_DCS_READ_DDB_CONTINUE		    			0xA8
#define MANUFACTURER_COMMAND_ACCESS_PROTECT				0xB0
#define FRAME_MEMORY_ACCESS_INTERFACE_SETTING			0xB3
#define DISPLAY_MODE_FRAME_MEMORY_WRITE_MODE_SETTING	0xB4
#define PANEL_DRIVING_SETTING							0xC0
#define DISPLAY_TIMING_SETTING_FOR_NORMAL_MODE			0xC1
#define SOURCE_VCOM_GATE_DRIVING_TIMING_SETTING			0xC4
#define GAMMA_SET										0xC8
#define POWER_SETTING_COMMON_SETTING					0xD0
#define VCOM_SETTING									0xD1
#define POWER_SETTING_FOR_NORMAL_MODE					0xD2
#define DITHER_SETTING									0xDA

#define TEAR_SCANLINE	480

DISPCTRL_REC_T R61581_Init[] = {
    {DISPCTRL_SLEEP_MS        , 0                               , 10      },

    // Manufacturer Command Access Protect, 1P
    {DISPCTRL_WR_CMND_DATA    , 0xB0                            , 0       },   

    // Frame Memory Access and Interface Setting, 4P
    {DISPCTRL_WR_CMND         , 0xB3                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x02    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            

    // Display Mode and Frame Memory Write Mode Setting ( DPI/DBI ), 1P
    // 0 = DBI, Internal Osc Clock
    {DISPCTRL_WR_CMND_DATA    , 0xB4                            , 0x00    },   
 
    // Panel Drive Setting, 8P
    {DISPCTRL_WR_CMND         , 0xC0                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x13    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x3B    },   //480    
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },   //02     
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x01    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },            
    {DISPCTRL_WR_DATA         , 0x00                            , 0x43    },            

    // Display Timing Setting for Normal Mode, 4P
    {DISPCTRL_WR_CMND         , 0xC1                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x08    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x1B    },   // CLOCK
    {DISPCTRL_WR_DATA         , 0x00                            , 0x08    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x08    },   

    // Source/VCOM/Gate Driving Timing Setting, 4P
    {DISPCTRL_WR_CMND         , 0xC4                            , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x11    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x07    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },   

    // Gamma Set, 20P
    {DISPCTRL_WR_CMND         , 0xC8                            , 0       },   // GAMMA
    {DISPCTRL_WR_DATA         , 0x00                            , 0x04    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x09    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x16    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x5A    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x02    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0A    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x16    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x05    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x32    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x05    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x16    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x0A    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x53    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x08    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x16    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x09    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x04    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x32    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   


    {DISPCTRL_WR_CMND         , MIPI_DCS_SET_COLUMN_ADDRESS     , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x01    },    //
    {DISPCTRL_WR_DATA         , 0x00                            , 0x3F    },    // 319


    {DISPCTRL_WR_CMND         , MIPI_DCS_SET_PAGE_ADDRESS       , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x01    },    //
    {DISPCTRL_WR_DATA         , 0x00                            , 0xDF    },    // 479

    {DISPCTRL_WR_CMND_DATA    , MIPI_DCS_SET_TEAR_ON            , 0x00    },   
    {DISPCTRL_WR_CMND_DATA    , MIPI_DCS_SET_PIXEL_FORMAT       , 0x05    },    

    {DISPCTRL_WR_CMND         , MIPI_DCS_SET_TEAR_SCANLINE      , 0       },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x01    },       


    {DISPCTRL_WR_CMND         , MIPI_DCS_WRITE_MEMORY_START     , 0       },   


    {DISPCTRL_WR_CMND         , MIPI_DCS_WRITE_MEMORY_START     , 0       },   
    {DISPCTRL_WR_CMND         , MIPI_DCS_EXIT_SLEEP_MODE        , 0       },   

    {DISPCTRL_SLEEP_MS        , 0                               , 150     },

    // Power Setting (Common Setting), 4P
    {DISPCTRL_WR_CMND         , 0xD0                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x07    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x07    },      
    {DISPCTRL_WR_DATA         , 0x00                            , 0x1E    },      
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },        // TRULY seq sent to us: 0x0703

    // VCOM, 3P
    {DISPCTRL_WR_CMND         , 0xD1                            , 0x00    },        
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x52    },        // VCM
    {DISPCTRL_WR_DATA         , 0x00                            , 0x10    },        // VDV

    // Power Setting for Normal Mode, 3P
    {DISPCTRL_WR_CMND         , 0xD2                            , 0x00    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x03    },   
    {DISPCTRL_WR_DATA         , 0x00                            , 0x24    },   
    // - missing 3-th par from TRULY's provided sequence

    // Manufacturer Command Access Protect
    {DISPCTRL_WR_CMND_DATA    , 0xB0                            , 0x00    },   
    {DISPCTRL_WR_CMND         , MIPI_DCS_SET_DISPLAY_ON         , 0       },   
    {DISPCTRL_SLEEP_MS        , 0                               , 10      },
    //--- END OF COMMAND LIST -----------------------
    {DISPCTRL_LIST_END       , 0                                , 0       }
};


#if 0

DISPCTRL_REC_T R61581_Init[] = {
	{DISPCTRL_SLEEP_MS, 0, 10},
	{DISPCTRL_WR_CMND_DATA, MANUFACTURER_COMMAND_ACCESS_PROTECT, 0},
	{DISPCTRL_WR_CMND, FRAME_MEMORY_ACCESS_INTERFACE_SETTING, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x02},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_CMND_DATA, DISPLAY_MODE_FRAME_MEMORY_WRITE_MODE_SETTING, 0x00},
	{DISPCTRL_WR_CMND, PANEL_DRIVING_SETTING, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x13},
	{DISPCTRL_WR_DATA, 0x00, 0x3B},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x01},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x43},
	{DISPCTRL_WR_CMND, DISPLAY_TIMING_SETTING_FOR_NORMAL_MODE, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x08},
	{DISPCTRL_WR_DATA, 0x00, 0x12},
	{DISPCTRL_WR_DATA, 0x00, 0x08},
	{DISPCTRL_WR_DATA, 0x00, 0x08},
	{DISPCTRL_WR_CMND, SOURCE_VCOM_GATE_DRIVING_TIMING_SETTING, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x11},
	{DISPCTRL_WR_DATA, 0x00, 0x07},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_CMND, GAMMA_SET, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x04},
	{DISPCTRL_WR_DATA, 0x00, 0x09},
	{DISPCTRL_WR_DATA, 0x00, 0x16},
	{DISPCTRL_WR_DATA, 0x00, 0x5A},
	{DISPCTRL_WR_DATA, 0x00, 0x02},
	{DISPCTRL_WR_DATA, 0x00, 0x0A},
	{DISPCTRL_WR_DATA, 0x00, 0x16},
	{DISPCTRL_WR_DATA, 0x00, 0x05},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x32},
	{DISPCTRL_WR_DATA, 0x00, 0x05},
	{DISPCTRL_WR_DATA, 0x00, 0x16},
	{DISPCTRL_WR_DATA, 0x00, 0x0A},
	{DISPCTRL_WR_DATA, 0x00, 0x53},
	{DISPCTRL_WR_DATA, 0x00, 0x08},
	{DISPCTRL_WR_DATA, 0x00, 0x16},
	{DISPCTRL_WR_DATA, 0x00, 0x09},
	{DISPCTRL_WR_DATA, 0x00, 0x04},
	{DISPCTRL_WR_DATA, 0x00, 0x32},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_CMND, MIPI_DCS_SET_COLUMN_ADDRESS, 0},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x01},
	{DISPCTRL_WR_DATA, 0x00, 0x3F},
	{DISPCTRL_WR_CMND, MIPI_DCS_SET_PAGE_ADDRESS, 0},
	{DISPCTRL_WR_DATA, 0x01, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x01},
	{DISPCTRL_WR_DATA, 0x00, 0xDF},
	{DISPCTRL_WR_CMND_DATA, MIPI_DCS_SET_TEAR_ON, 0x00},
#ifdef CONFIG_R61581_ARGB8888
	{DISPCTRL_WR_CMND_DATA, MIPI_DCS_SET_PIXEL_FORMAT, 0x07},
#else
	{DISPCTRL_WR_CMND_DATA, MIPI_DCS_SET_PIXEL_FORMAT, 0x06},
#endif
	{DISPCTRL_WR_CMND, MIPI_DCS_SET_TEAR_SCANLINE, 0},
	{DISPCTRL_WR_DATA, 0x00, TEAR_SCANLINE >> 8},
	{DISPCTRL_WR_DATA, 0x00, TEAR_SCANLINE & 0xFF},
	{DISPCTRL_WR_CMND, MIPI_DCS_WRITE_MEMORY_START, 0},
	{DISPCTRL_WR_CMND, MIPI_DCS_EXIT_SLEEP_MODE, 0},
	{DISPCTRL_SLEEP_MS, 0, 120},
	{DISPCTRL_WR_CMND, POWER_SETTING_COMMON_SETTING, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x07},
	{DISPCTRL_WR_DATA, 0x00, 0x07},
	{DISPCTRL_WR_DATA, 0x00, 0x1E},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_CMND, VCOM_SETTING, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_DATA, 0x00, 0x52},
	{DISPCTRL_WR_DATA, 0x00, 0x10},
	{DISPCTRL_WR_CMND, POWER_SETTING_FOR_NORMAL_MODE, 0x00},
	{DISPCTRL_WR_DATA, 0x00, 0x03},
	{DISPCTRL_WR_DATA, 0x00, 0x24},
	{DISPCTRL_WR_CMND, MIPI_DCS_SET_DISPLAY_ON, 0},
	{DISPCTRL_SLEEP_MS, 0, 10},
#ifdef CONFIG_R61581_ARGB8888
	{DISPCTRL_WR_CMND_DATA, DITHER_SETTING, 0x01},
#endif
    //--- END OF COMMAND LIST -----------------------
    {DISPCTRL_LIST_END        , 0     , 0       }
};

#endif
