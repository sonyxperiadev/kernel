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
*   @file   hal_lcd_glue.h
*
*   @brief      This file is the LCD Device Independent driver for HAL.
*
*   Device Independent Driver for Lcd 
*
****************************************************************************/

#if !defined( _HAL_LCD_GLUE_H__ )
#define _HAL_LCD_GLUE_H__


// ---- Include Files -------------------------------------------------------

//******************************************************************************
//	
// LCD Device InDependent API - hal_lcd_glue.c
// 
// These functions are designed to be portable among different LCD devices
//
//******************************************************************************

typedef void (*LCD_DRV_DMAEND_CBF)();

// Init LCD device
void LCD_DRV_Init(void);
void LCD_DRV_SplashScreen(void);
// Retreive GRAM buffer, contrast, backlight settings
void LCD_DRV_GetDriverInfo(UInt8 **lcd_cache_buff, UInt8 *contrast_level, UInt8 *backlight_level);
void LCD_DRV_GetDriverInfo_S(UInt8 **lcd_cache_buff, UInt8 *contrast_level, UInt8 *backlight_level);
// GRAM buffer pointers
UInt16 *LCD_DRV_GetMainLCD(void);
UInt16 *LCD_DRV_GetSubLCD(void);
void LCD_DRV_UseDMA(Boolean Enable);

// Update display window with data in GRAM buffer
void LCD_DRV_UpdateLCD(LCD_DRV_DMAEND_CBF callback);
void LCD_DRV_UpdateLCD_S(LCD_DRV_DMAEND_CBF callback);
// Backlight control
void LCD_DRV_SetBacklight( BacklightColor_t color, UInt8 level );
void LCD_DRV_SetBacklight_S( BacklightColor_t color, UInt8 level );
UInt32 LCD_DRV_GetBacklightLevel(void);
UInt32 LCD_DRV_GetBacklightLevel_S(void);
// Returns state of backlight
Boolean LCD_DRV_IsBacklightOn( void );

// LCD Update Enable/Disable/Status
void LCD_DRV_EnableLCDUpdates(Boolean Enable);
Boolean LCD_DRV_UpdateStatus(void);
Boolean LCD_DRV_UpdateStatus_S(void);
void LCD_DRV_EnableDMA(Boolean Enable);

// Configure display window and camera preview window
// A window is not activated until LCD_DRV_ResetStartAddr() is called
void LCD_DRV_ResetWindow(void);
void LCD_DRV_SetWindow(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom);
void LCD_DRV_SetWindow_S(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom);
void LCD_DRV_GetWindow(UInt32 *Left, UInt32 *Right, UInt32 *Top, UInt32 *Bottom);
void LCD_DRV_GetWindow_S(UInt32 *Left, UInt32 *Right, UInt32 *Top, UInt32 *Bottom);

LCD_Result_en_t LCD_DRV_RenderImage(LCD_DisplayId_t display, LCD_ImageFormat_t src_image_format, UInt32 src_pitch, UInt32 width, UInt32 height, 
																		UInt32 dest_x_offset, UInt32 dest_y_offset, void *src);

// Retrieve display window size
UInt32 LCD_DRV_WinH(void);
UInt32 LCD_DRV_WinW(void);
UInt32 LCD_DRV_WinH_S(void);
UInt32 LCD_DRV_WinW_S(void);
// Draw display window with data
void LCD_DRV_ClearScreen(UInt16 data);
void LCD_DRV_ClearScreen_S(UInt16 data);
// LCD interface control
void LCD_DRV_SetSpeed(UInt32 hold, UInt32 pulse);
void LCD_DRV_SetCE(Boolean Enable);
void LCD_DRV_Set8BIT(Boolean Enable);
// Write LCD cmd
void WRITE_LCD_CMD(UInt32 cmd);

// Legacy API's. no longer used by MMI and should be retired if deemed not required
void LCD_DRV_SetCamWindow(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom);
void LCD_DRV_GetCamWindow(UInt32 *Left, UInt32 *Right, UInt32 *Top, UInt32 *Bottom);

void LCD_DRV_HorizAddrsing(void);
void LCD_DRV_HorizAddrsing_S(void);
void LCD_DRV_VertAddrsing(void);
void LCD_DRV_VertAddrsing_S(void);
void LCD_DRV_Set_LRTB_MODE(void);
void LCD_DRV_Set_RLBT_MODE(void);
void LCD_DRV_DisplayImage( UInt16 *buf, UInt16 scaleDownFact );
void LCD_DRV_DisplayImage_S( UInt16 *buf, UInt16 scaleDownFact );
static void LCD_DRV_test( void );

// For LCD specific device drivers to interface with sleep ID initialized in lcd_drv_HAL.c
void LCD_DRV_DisableDeepSleep(void);
void LCD_DRV_EnableDeepSleep(void);

void LCD_DRV_DrawHorizontalLine(UInt16 h, UInt16 data);
void LCD_DRV_DrawHorizontalLine_S(UInt16 h, UInt16 data);
void LCD_DRV_DrawVerticalLine(UInt16 h, UInt16 data);
void LCD_DRV_DrawVerticalLine_S(UInt16 h, UInt16 data);
void LCD_DRV_FillWindow(UInt16 data);
void LCD_DRV_FillWindow_S(UInt16 data);
void LCD_DRV_SetAndFillWindow(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom, UInt16 data);
void LCD_DRV_SetAndFillWindow_S(UInt32 Left, UInt32 Right, UInt32 Top, UInt32 Bottom, UInt16 data);
void LCD_DRV_DisplayMcBeth( void );
void LCD_DRV_DisplayMcBeth_S( void );
static void LCD_DRV_test( void );
void LCD_DRV_ScaleDownScreen(UInt16 scaleDownFact);
void LCD_DRV_ScaleDownScreen_S(UInt16 scaleDownFact);

#endif	// _HAL_LCD_GLUE_H__

/** @} */

