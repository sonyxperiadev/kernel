/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_mipi_dcs.h
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
#ifndef __DISPDRV_MIPI_DCS_H__
#define __DISPDRV_MIPI_DCS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */

#define MIPI_DCS_NOP					    0x00    
#define MIPI_DCS_SOFT_RESET				    0x01    
#define MIPI_DCS_GET_RED_CHANNEL		    0x06    
#define MIPI_DCS_GET_GREEN_CHANNEL		    0x07    
#define MIPI_DCS_GET_BLUE_CHANNEL		    0x08    
#define MIPI_DCS_GET_POWER_MODE			    0x0A    
#define MIPI_DCS_GET_ADDRESS_MODE		    0x0B    
#define MIPI_DCS_GET_PIXEL_FORMAT		    0x0C    
#define MIPI_DCS_GET_DISPLAY_MODE		    0x0D    
#define MIPI_DCS_GET_SIGNAL_MODE		    0x0E    
#define MIPI_DCS_GET_DIAGNOSTIC_RESULTS	    0x0F    

#define MIPI_DCS_ENTER_SLEEP_MODE		    0x10    

#define MIPI_DCS_EXIT_SLEEP_MODE		    0x11    
#define MIPI_DCS_ENTER_PARTIAL_MODE	 	    0x12    
#define MIPI_DCS_ENTER_NORMAL_MODE	 	    0x13    

#define MIPI_DCS_EXIT_INVERT_MODE		    0x20    
#define MIPI_DCS_ENTER_INVERT_MODE	 	    0x21    
#define MIPI_DCS_SET_GAMMA_CURVE	  	    0x26    
#define MIPI_DCS_SET_DISPLAY_OFF	  	    0x28    
#define MIPI_DCS_SET_DISPLAY_ON		  	    0x29    
#define MIPI_DCS_SET_COLUMN_ADDRESS  	    0x2A    
#define MIPI_DCS_SET_PAGE_ADDRESS	  	    0x2B    
#define MIPI_DCS_WRITE_MEMORY_START		    0x2C    
#define MIPI_DCS_WRITE_LUT				    0x2D    
#define MIPI_DCS_READ_MEMORY_START   	    0x2E    

#define MIPI_DCS_SET_PARTIAL_AREA	  	    0x30    
#define MIPI_DCS_SET_SCROLL_AREA	  	    0x33    
#define MIPI_DCS_SET_TEAR_OFF		  	    0x34    
#define MIPI_DCS_SET_TEAR_ON		  	    0x35    
#define MIPI_DCS_SET_ADDRESS_MODE   	    0x36    
#define MIPI_DCS_SET_SCROLL_START	  	    0x37    
#define MIPI_DCS_EXIT_IDLE_MODE			    0x38    
#define MIPI_DCS_ENTER_IDLE_MODE	 	    0x39    
#define MIPI_DCS_SET_PIXEL_FORMAT	  	    0x3A    
#define MIPI_DCS_WRITE_MEMORY_CONTINUE	    0x3C    
#define MIPI_DCS_READ_MEMORY_CONTINUE 	    0x3E    

#define MIPI_DCS_SET_TEAR_SCANLINE	  	    0x44    
#define MIPI_DCS_GET_SCANLINE			    0x45    

#define MIPI_DCS_READ_DDB_START			    0xA1    
#define MIPI_DCS_READ_DDB_CONTINUE		    0xA8    

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DISPDRV_MIPI_DCS_H__ */
