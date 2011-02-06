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
/**
*
*   @file   hal_lcd.h
*
*   @brief  Global declaration of hardware abstraction layer for Lcd driver.
*
****************************************************************************/
/**
*   @defgroup   LcdDrvGroup   Lcd HAL Interface
*   @ingroup    HALMMGroup	
*   @brief      This file is the HAL API driver for LCD.
*
*   Device Independent API for Lcd 
*
****************************************************************************/
/**
 * @addtogroup LcdDrvGroup
 * @{
 */
/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/
#if !defined( _HAL_LCD_H__ )
#define _HAL_LCD_H__

//#define	ENABLE_DISPMAN_TEST

/// LCD Contrast level
typedef enum
{
	LCD_Contrast_Off,			///< lcd contrast off
	LCD_Contrast_On,			///< lcd contrast on
	LCD_Contrast_N				///<  max # of contrast levels
} LCD_Contrast_t;

/// LCD Rotation
typedef enum
{
  LCD_ImageNormal = 0,			///< normal display
  LCD_ImageMirrorRot0,			///< mirror display
  LCD_ImageMirrorRot180,		///< mirror, rotate 180 display
  LCD_ImageRot180,				///< rotate 180 display
  LCD_ImageMirrorRot90,			///< mirror, rotate 90 display
  LCD_ImageRot270,				///< rotate 270 display
  LCD_ImageRot90,				///< rotate 90 display
  LCD_ImageMirrorRot270,		///< mirror, rotate 270 display
  LCD_N_ImageRotate				///< max # image rotate
} LCD_ImageOrientation_t;


/// LCD Image Format
typedef enum
{
   LCD_ImageMin = 0, 			///< bounds for error checking
   LCD_ImageRGB565 = 1,			///< RGB565
   LCD_Image1BPP,				///< 1 bit per pixel
   LCD_ImageYUV420,				///< YUV420
   LCD_Image48BPP,				///< 48 bits per pixel
   LCD_ImageRGB888,				///< RGB888	Unpacked ( 4 BPP )
   LCD_Image8BPP,				///< 8 bits per pixel
   LCD_Image4BPP, 				///< 4bpp palettised image
   LCD_Image3D32, 				///< A separated format of 16 colour/light shorts followed by 16 z values 
   LCD_Image3D32B,				///< 16 colours followed by 16 z values 
   LCD_Image3D32MAT,			///< A separated format of 16 material/colour/light shorts followed by 16 z values 
   LCD_ImageRGB2X9,				///< 32 bit format containing 18 bits of 6.6.6 RGB, 9 bits per short 
   LCD_ImageRGB666, 			///< 32-bit format holding 18 bits of 6.6.6 RGB 
   LCD_ImagePAL4,				///< 4bpp palettised image with embedded palette
   LCD_ImagePAL8, 				///< 8bpp palettised image with embedded palette
   LCD_ImageRGBA32,				///< RGB888 with an alpha byte after each pixel 
   LCD_ImageYUV422, 			///< a line of Y (32-byte padded), a line of U (16-byte padded), and a line of V (16-byte padded) 
   LCD_ImageRGBA565,			///< RGB565 with a transparent patch 
   LCD_ImageRGB888_P,			///< RGB888	Packed ( 3 BPP )
   LCD_Image_N_Types    		///< bounds for error checking
} LCD_ImageFormat_t;

/// LCD Display ID
typedef enum
{
   LCD_DisplayId_Main = 0,		///< main
   LCD_DisplayId_Sub,			///< sub
   LCD_DisplayId_TV,			///< tv
   LCD_DisplayId_Image,			///< image
   LCD_DisplayId_N_Types    	///< bounds for error checking
} LCD_DisplayId_t;

/// HAL LCD driver action request
typedef enum {
	ACTION_LCD_GetConfig,				///< Get default Configuration, use HAL_LCD_cfg_specific_st_t
	ACTION_LCD_GetDeviceInfo,			///< Get Lcd Device Specific info, use HAL_LCD_Action_info_st_t
	ACTION_LCD_GetDriverInfo,			///< Get Lcd Driver Specific info, use HAL_LCD_Action_driverinfo_st_t
	ACTION_LCD_GetDriverInfo_S,			///< Get Sub-Lcd Driver Specific info, use HAL_LCD_Action_driverinfo_st_t
	ACTION_LCD_SetLCDUpdatesEnable,		///< enables/disables LCD display updates, use HAL_LCD_Action_param_st_t to set
	ACTION_LCD_SetContrast,				///< Set the Main-Lcd contrast level, use HAL_LCD_Action_param_st_t to set
	ACTION_LCD_SetContrast_S,			///< Set the Sub-Lcd contrast level, use HAL_LCD_Action_param_st_t to set
	ACTION_LCD_GetContrast,				///< Get the Main-Lcd contrast level, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_GetContrast_S,			///< Get the Sub-Lcd contrast level, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_SetUseLcdDma,			///< Use to Disable DMA for current Lcd Update, defaults back to use DMA after Update, use HAL_LCD_Action_param_st_t to set	
	ACTION_LCD_RenderImage,				///< Update Lcd, use HAL_LCD_Action_render_st_t to add image to frame buffer, Callback for update end is optional
	ACTION_LCD_Update,					///< Update Lcd, use HAL_LCD_Action_frame_st_t to update, Callback for update end is optional
	ACTION_LCD_Update_S,				///< Update Sub-Lcd, use HAL_LCD_Action_frame_st_t to update, Callback for update end is optional
	ACTION_LCD_GetFrameBuff,			///< Get Main-Lcd Frame Buffer Pointer, use HAL_LCD_Action_frame_st_t to return pointer
	ACTION_LCD_GetFrameBuff_S,			///< Get Sub-Lcd Frame Buffer Pointer, use HAL_LCD_Action_frame_st_t to return pointer
	ACTION_LCD_GetPhyHeight,			///< Get Lcd physical height, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_GetPhyHeight_S,			///< Get Sub-Lcd physical height, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_GetPhyWidth,				///< Get Lcd physical width, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_GetPhyWidth_S,			///< Get Sub-Lcd physical width, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_SetWindow,				///< Set Lcd size for partial screen update, use HAL_LCD_Action_window_st_t to set
	ACTION_LCD_SetWindow_S,				///< Set Sub-Lcd size for partial screen update, use HAL_LCD_Action_window_st_t to set
	ACTION_LCD_GetWindow,				///< Set Lcd size for partial screen update, use HAL_LCD_Action_window_st_t to get
	ACTION_LCD_GetWindow_S,				///< Set Sub-Lcd size for partial screen update, use HAL_LCD_Action_window_st_t to get
	ACTION_LCD_SplashScreen,			///< Update Lcd with Splash Screen BitMap Image, use HAL_LCD_Action_frame_st_t to update
	ACTION_LCD_GetImageType,			///< Get Lcd's RGB format, use HAL_LCD_Action_rgb_st_t to get
	ACTION_LCD_GetImageFormat,			///< Get Lcd's RGB format, use HAL_LCD_Action_param_st_t to get
	ACTION_LCD_GetPixelRgb,				///< Get Pixel in Lcd's RGB format, use HAL_LCD_Action_rgb_st_t to get
  ACTION_LCD_Action_N_Types    			///< bounds for error checking
} HAL_LCD_Action_en_t;

/// HAL LCD driver function call result
typedef enum
{
	HAL_LCD_SUCCESS,					///< Successful
	HAL_LCD_ERROR_ACTION_NOT_SUPPORTED,	///<  Not supported 
	HAL_LCD_ERROR_INTERNAL_ERROR,		///< Internal error: i2c, comm failure, etc.
	HAL_LCD_ERROR_OTHERS				///< Undefined error
} HAL_LCD_Result_en_t;


/// HAL LCD Device Specific Configuration Structure
typedef struct 
{
	LCD_Contrast_t	default_contrast; 	///< (in/out) contrast 0-100%
} HAL_LCD_cfg_specific_st_t;

/// HAL LCD Device Configuration Structure
typedef struct 
{
	HAL_LCD_cfg_specific_st_t*  HAL_LCD_cfg_specific;  	///< device specific configuration structure
	Boolean 	 	  			device_ACTIVE; 			///< mandatory value
	UInt32            			performance_required; 	///< mandatory value
} HAL_LCD_config_st_t;

/// HAL LCD Driver Info Structure
typedef struct 
{
// Main & Sub Lcd info	
	LCD_ImageFormat_t	   	image_format;			///< Lcd image format
	LCD_ImageOrientation_t 	orientation;			///< orientation of display to buffer
	LCD_Contrast_t 		   	default_contrast;		///< default contrast level
// Main Lcd Info	
	UInt32 				   	pitch;					///< lcd pitch in bytes
	UInt32 				   	width;					///< Main-lcd width														
	UInt32 				   	height;					///< Main-lcd height	
	void* 				   	frame_buffer;			///< Main-lcd frame buffer
	LCD_Contrast_t 		   	contrast_level;			///< contrast level
// Sub Lcd Info	
	UInt32 				   	pitch_S;				///< Sub-lcd pitch in bytes
	UInt32  			   	width_S;				///< Sub-lcd width														
	UInt32  			   	height_S;				///< Sub-lcd height	
	void*  				   	frame_buffer_S;			///< Sub-lcd frame buffer
	LCD_Contrast_t 		   	contrast_level_S;		///< contrast level
} HAL_LCD_Action_info_st_t;

/// HAL LCD Action:  GetDriverInfo:  Frame buffer, contrast level, backlight level
typedef struct 
{
	void* 				   	frame_buffer;			///< Lcd frame buffer
	LCD_Contrast_t 		   	contrast;				///< contrast level
} HAL_LCD_Action_driverinfo_st_t;

/// HAL LCD Action:  RenderImage
typedef struct 
{
  LCD_DisplayId_t     display_id;         ///< (in) display to render image onto
  LCD_ImageFormat_t   src_format;         ///< (in) source image format in buffer
  void*               lcd_image;          ///< (in) source image buffer
  UInt32              pitch;              ///< (in) pitch of source image
  UInt32              width;              ///< (in) width of source image
  UInt32              height;             ///< (in) height of source image
  LCD_ImageFormat_t   dest_format;        ///< (in) source image format in buffer
  UInt32              dest_x_offset;      ///< (in) x offset of destination
  UInt32              dest_y_offset;      ///< (in) y offset of destination
} HAL_LCD_Action_render_st_t;

/// HAL LCD Action:  Update/GetFrameBuff/GetStaticCommonBuff/GetVmcsRgbTempBuff
typedef struct 
{
	LCD_DisplayId_t 		display_id;				///< (in) display to update image onto
	LCD_ImageFormat_t		image_format;			///< (in/out) image type in buffer
	void* 					lcd_image;				///< (in/out) NULL if using Frame Buffer
} HAL_LCD_Action_frame_st_t;

/// HAL LCD Action:  SetWindow/GetWindow
typedef struct 
{
	UInt32 left;									///< (in/out) starting left pixel
	UInt32 right;									///< (in/out) ending right pixel
	UInt32 top;										///< (in/out) top pixel
	UInt32 bottom;									///< (in/out) bottom pixel
} HAL_LCD_Action_window_st_t;


/// HAL LCD Action:  GetPixelRgb
typedef struct 
{
	UInt16 	blue;								   	///< (in) blue pixel
	UInt16 	green;								   	///< (in) green pixel
	UInt16 	red;								   	///< (in) red pixel
	UInt32	pixel;								   	///< (out) pixel output
} HAL_LCD_Action_rgb_st_t;

/// HAL LCD Action:  Parameter
typedef struct 
{
	UInt32 param;									///< (in/out) parameter
} HAL_LCD_Action_param_st_t;						

/// HAL LCD Action Structure Union:  
typedef union
{
	HAL_LCD_cfg_specific_st_t		HAL_LCD_cfg_specific;	   	///< HAL_LCD_cfg_specific_st_t
	HAL_LCD_Action_info_st_t 		HAL_LCD_Action_info;	   	///< HAL_LCD_Action_info_st_t 
	HAL_LCD_Action_driverinfo_st_t 	HAL_LCD_Action_driverinfo; 	///< HAL_LCD_Action_driverinfo_st_t 
	HAL_LCD_Action_render_st_t      HAL_LCD_Action_render;	   	///< HAL_LCD_Action_render_st_t 
	HAL_LCD_Action_frame_st_t		HAL_LCD_Action_frame;	   	///< HAL_LCD_Action_frame_st_t
	HAL_LCD_Action_window_st_t 		HAL_LCD_Action_window;	   	///< HAL_LCD_Action_window_st_t
	HAL_LCD_Action_rgb_st_t 		HAL_LCD_Action_rgb;		   	///< HAL_LCD_Action_rgb_st_t
	HAL_LCD_Action_param_st_t		HAL_LCD_Action_param;	   	///< HAL_LCD_Action_param_st_t
} HAL_LCD_Control_un_t;

typedef void (*HAL_LCD_DmaEndCB_t)(void);			///< lcd dma call-back function


//***************************************************************************
//
//  LCD Device Independent API - HALLCD_Drv.c
//
//  These functions are designed to be portable among different LCD devices
//***************************************************************************

/**
*  This function will initialize Lcd Driver.  
*  To Initialize to default settings, use HAL_LCD_Init(NULL);\n
*  To Initialize with modified default settings:
   - Get default configuration settings using #ACTION_LCD_GetConfig
		- Lcd driver will fill HAL_LCD_config_st_t with default settings.
   - Modify default settings and pass HAL_LCD_config_st_t to Lcd driver in HAL_LCD_Init().
		- Lcd driver will copy new settings to local and initialize Lcd.
*/
HAL_LCD_Result_en_t HAL_LCD_Init(
		HAL_LCD_config_st_t* HAL_LCD_config			///< (in/out) device configuration structure
		);					

/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*/
HAL_LCD_Result_en_t HAL_LCD_Ctrl(
		HAL_LCD_Action_en_t	action,					///< (in) device action
		void* 				param, 					///< (in/out) structure pointer to additional parameters for Set, Get, and IS
		HAL_LCD_DmaEndCB_t 	callback				///< (in) callback function device driver calls once action complete, NULL if not needed
		);	

#endif	// _HAL_LCD_H__

/** @} */


