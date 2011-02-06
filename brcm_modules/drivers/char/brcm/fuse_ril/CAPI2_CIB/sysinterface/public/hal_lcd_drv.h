/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/*
*
*   @file   hal_lcd_drv.h
*
*   @brief      This file is the LCD Device Independent driver for HAL.
*
*   Device Independent Driver for Lcd 
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "lcd_size_dev.h"
#include "lcd_image.h"
#include "hal_lighting.h"
*/
#if !defined( _HAL_LCD_DRV_H__ )
#define _HAL_LCD_DRV_H__

//******************************************************************************
//							definition block
//******************************************************************************

//******************************************************************************
//							prototypes block
//******************************************************************************

//******************************************************************************
//	
// LCD Device Independent API - lcd_drv_HAL.c
// 
// These functions are designed to be portable among different LCD devices
//
//******************************************************************************

// Init LCD device
void LCD_Init(void);
void LCD_SplashScreen(LCD_DisplayId_t display);
// Retreive GRAM buffer, contrast, backlight settings
void LCD_GetDriverInfo(UInt8 **lcd_cache_buff, UInt8 *contrast_level);
void LCD_GetDriverInfo_S(UInt8 **lcd_cache_buff, UInt8 *contrast_level);
void LCD_SetLcdSleep( Boolean Enable );
// GRAM buffer pointers
UInt16 *LCD_GetMainLCD(void);
UInt16 *LCD_GetSubLCD(void);
void LCD_UseDMA(Boolean Enable);
LCD_ImageFormat_t LCD_GetGramFormat(LCD_DisplayId_t display);

// Update display window with data in GRAM buffer
HAL_LCD_Result_en_t LCD_UpdateGRAM(
		LCD_DisplayId_t display_id, 
		LCD_ImageFormat_t image_format, 
		void* lcd_image, 
		LCD_DMAEND_CBF callback
		);
void LCD_UpdateLCD(LCD_DMAEND_CBF callback);
void LCD_UpdateLCD_S(LCD_DMAEND_CBF callback);

// LCD Update Enable/Disable/Status
void LCD_EnableLCDUpdates(Boolean Enable);
Boolean LCD_UpdateStatus(void);
Boolean LCD_UpdateStatus_S(void);
void LCD_EnableDMA(Boolean Enable);
HAL_LCD_Result_en_t LCD_RenderImage(
      LCD_DisplayId_t display, 
      LCD_ImageFormat_t src_format, 
      UInt32 src_pitch, 
      UInt32 width, 
      UInt32 height, 
      LCD_ImageFormat_t dest_format, 
      UInt32 dest_x_offset, 
      UInt32 dest_y_offset, 
      void *src);

// Configure display window and camera preview window
// A window is not activated until LCD_ResetStartAddr() is called
void LCD_ResetWindow(void);
void LCD_SetWindow(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom);
void LCD_SetWindow_S(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom);
void LCD_GetWindow(UInt32 *Left, UInt32 *Right, UInt32 *Top, UInt32 *Bottom);
void LCD_GetWindow_S(UInt32 *Left, UInt32 *Right, UInt32 *Top, UInt32 *Bottom);

// Retrieve display window size
UInt32 LCD_WinH(void);
UInt32 LCD_WinW(void);
UInt32 LCD_WinH_S(void);
UInt32 LCD_WinW_S(void);
// Draw display window with data
void LCD_SetScreen(
		LCD_DisplayId_t display_id, 
		LCD_ImageFormat_t image_format, 
		UInt32 data
		);
// LCD interface control
void LCD_SetSpeed(UInt32 hold, UInt32 pulse);
void LCD_SetCE(Boolean Enable);
void LCD_Set8BIT(Boolean Enable);
// Write LCD cmd
void WRITE_LCD_CMD(UInt32 cmd);
void READ_LCD_CMD_DATA(UInt32 *data);
void READ_LCD_INDEX(UInt32 *data);

void LCD_HorizAddrsing(void);
void LCD_HorizAddrsing_S(void);
void LCD_VertAddrsing(void);
void LCD_VertAddrsing_S(void);
void LCD_Set_LRTB_MODE(void);
void LCD_Set_RLBT_MODE(void);
void LCD_DisplayImage( UInt16 *buf, UInt16 scaleDownFact );
void LCD_DisplayImage_S( UInt16 *buf, UInt16 scaleDownFact );
static void LCD_test( void );

// Legacy file (now done with Hal_Lighting)
void LCD_SetBacklight( HAL_LIGHT_COLOR_t color, UInt8 level );
void LCD_SetBacklight_S( HAL_LIGHT_COLOR_t color, UInt8 level );
Boolean LCD_IsBacklightOn( void );

// For LCD specific device drivers to interface with sleep ID initialized in lcd_drv_HAL.c
void LCD_DisableDeepSleep(void);
void LCD_EnableDeepSleep(void);

#if (defined(PLATFORM_TEST) || defined(BSP_PLUS_BUILD_INCLUDED))

void LCD_DrawHorizontalLine(UInt16 h, UInt16 data);
void LCD_DrawHorizontalLine_S(UInt16 h, UInt16 data);
void LCD_DrawVerticalLine(UInt16 h, UInt16 data);
void LCD_DrawVerticalLine_S(UInt16 h, UInt16 data);
void LCD_FillWindow(UInt16 data);
void LCD_FillWindow_S(UInt16 data);
void LCD_SetAndFillWindow(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom, UInt16 data);
void LCD_SetAndFillWindow_S(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom, UInt16 data);
void LCD_DisplayImage( UInt16 *buf, UInt16 scaleDownFact );
void LCD_DisplayImage_S( UInt16 *buf, UInt16 scaleDownFact );
void LCD_ScaleDownScreen(UInt16 scaleDownFact);
void LCD_ScaleDownScreen_S(UInt16 scaleDownFact);
void LCD_ClearScreen(UInt32 data);
void LCD_ClearScreen_S(UInt32 data);
void LCD_DisplayMcBeth( LCD_DisplayId_t	display_id );
void LCD_DisplayMcBeth_S( void );


#endif
//******************************************************************************
//	
// LCD Device Dependent API - declared in lcd_drv_dev_depd.h
// 
// These functions are unique to phone board design and LCD devices
//
//******************************************************************************
//#include "lcd_drv_dev.h"

#endif	// _HAL_LCD_DRV_H__

/** @} */

