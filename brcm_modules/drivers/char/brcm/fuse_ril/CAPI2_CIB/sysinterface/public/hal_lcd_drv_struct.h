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
*   @file   hal_lcd_drv_struct.h
*
*   @brief      This file is the LCD Device Independent driver for HAL.
*
*   Device Independent Driver for Lcd 
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/
#if !defined( _HAL_LCD_DRV_STRUCT_H__ )
#define _HAL_LCD_DRV_STRUCT_H__

//******************************************************************************
//							definition block
//******************************************************************************

/**  LCD driver function call result
*/
typedef enum
{
	LCD_SUCCESS,					   		///< Successful
	LCD_ERROR_ACTION_NOT_SUPPORTED,			///<  Not supported 
	LCD_ERROR_INTERNAL_ERROR,		   		///< Internal error: i2c, comm failure, etc.
	LCD_ERROR_OTHERS				   		///< Undefined error
} LCD_Result_en_t;

/** LCD Backlight type.
*/
typedef enum
{
	LCD_BKLIGHT_OFF,				   		// no color
	LCD_BKLIGHT_RED,				   		// red color
	LCD_BKLIGHT_GREEN,				   		// green color
	LCD_BKLIGHT_BLUE,				   		// blue color
	LCD_BKLIGHT_YELLOW,				   		// yellow color
	LCD_BKLIGHT_VIOLET,
	LCD_BKLIGHT_BLUEGREEN,
	LCD_BKLIGHT_WHITE
} BacklightColor_t;

/**  LCD Backlight level
*/
typedef enum
{
	LCD_BKLIGHT_LVL_OFF,			///< lcd off
	LCD_BKLIGHT_LVL_ON,				///< lcd on
	LCD_BKLIGHT_N_LVL			   	//  max # of backlight levels
} BacklightLevel_t;
	
/**  LCD Contrast level
*/
typedef enum
{
	LCD_Contrast_Lvl_Off,			///< lcd contrast off
	LCD_Contrast_Lvl_On,		   	///< lcd contrast on
	LCD_Contrast_N_Lvl				//  max # of contrast levels
} LCD_ContrastLevel_t;


/** LCD Color
*/
typedef enum
{
	LCD_COLOR_BLACK		= 0x0000,		   	///< lcd black pixel
	LCD_COLOR_GREEN		= 0x07E0,		   	///< lcd green pixel
	LCD_COLOR_RED 		= 0xF800,		   	///< lcd red pixel
	LCD_COLOR_BLUE 		= 0x001F,		   	///< lcd blue pixel
	LCD_COLOR_YELLOW 	= 0xFFE0,		   	///< lcd yellow pixel
	LCD_COLOR_CYAN		= 0xF81F,		   	///< lcd cyan pixel
	LCD_COLOR_DDD		= 0x07FF,		   	///< lcd ddd pixel
	LCD_COLOR_WHITE		= 0xFFFF		   	///< lcd white pixel
} LcdColor_t;

typedef enum
{
	LcdBlackRgb565		= 0x00000000,		///< lcd black pixel
	LcdGreenRgb565		= 0x000007E0,		///< lcd green pixel
	LcdRedRgb565 		= 0x0000F800,		///< lcd red pixel
	LcdBlueRgb565 		= 0x0000001F,		///< lcd blue pixel
	LcdYellowRgb565 	= 0x0000FFE0,		///< lcd yellow pixel
	LcdCyanRgb565		= 0x0000F81F,		///< lcd cyan pixel
	LcdDDDRgb565		= 0x000007FF,		///< lcd ddd pixel
	LcdWhiteRgb565		= 0x0000FFFF		///< lcd white pixel
} LcdColorRGB565_t;

typedef enum
{
	LcdBlackRgb888		= 0x00000000,		///< lcd black pixel
	LcdGreenRgb888		= 0x0000FF00,		///< lcd green pixel
	LcdRedRgb888 		= 0x00FF0000,		///< lcd red pixel
	LcdBlueRgb888 		= 0x000000FF,		///< lcd blue pixel
	LcdYellowRgb888 	= 0x00FFFF00,		///< lcd yellow pixel
	LcdCyanRgb888		= 0x00FF00FF,		///< lcd cyan pixel
	LcdDDDRgb888		= 0x0000FFFF,		///< lcd ddd pixel
	LcdWhiteRgb888		= 0x00FFFFFF		///< lcd white pixel
} LcdColorRGB888_t;

// LCD init sequence
typedef enum 
{
	WRITE_INDEX,
	WRITE_DATA,
	WRITE_PAUSE,
	WRITE_FULL_SCREEN,
	READ_INDEX,
	READ_DATA
}Init_Cmd_t;

typedef struct 
{
	Init_Cmd_t	type;
  UInt32		data;
}Init_Seq_t;

// LCD interface type, IOCR0[29:28]
typedef enum
{					
	LCD_Z80,		
	LCD_M68,		
//	LCD_SPI,
//	LCD_ETM,
	LCD_DBI_B,		   // DBI 8-bit parallel mode
	LCD_DBI_C_OPT1,	   // DBI 3-WIRE   D/CX embeded in protocol
	LCD_DBI_C_OPT3	   // DBI 4-WIRE   D/CX	phy line
} LCD_Intf_t;

// LCD voltage, IOCR2[30]
typedef enum
{				
	LCD_18V,		
	LCD_30V		
} LCD_Volt_t;

// LCD bus width
typedef enum
{				
	LcdBus8Bit,		
	LcdBus16Bit,		
	LcdBus18Bit		
} LCD_Bus_t;

typedef void (*LCD_DMAEND_CBF)();


// LCD Controller Setting definitions, LCDC_CR
#define LCD_DMA_EN					0x80000000
#define LCD_8BIT					0x40000000
#define LCD_CE						0x20000000
#define LCD_BYTE_SWAP				0x10000000
#define LCD_WORD_SWAP				0x08000000
#define LCD_SEL_LCD					0x04000000

// enhanced parallel LCD (DBI) Controller ( 21331, 21551, 2153-D0 ) 

// DBI Modes ALLOWED IN/OUT COLOR CONVERSIONS
// LCD_CR_DBI_COL_IN_RGB888_U	-> LCD_CR_DBI_COL_OUT_RGB666
// 								-> LCD_CR_DBI_COL_OUT_RGB888
// 
// LCD_CR_DBI_COL_IN_RGB888_P	-> LCD_CR_DBI_COL_OUT_RGB666
// 								-> LCD_CR_DBI_COL_OUT_RGB888
// 
// LCD_CR_DBI_COL_IN_RGB565		-> LCD_CR_DBI_COL_OUT_RGB332
// 					    		-> LCD_CR_DBI_COL_OUT_RGB444
// 					    		-> LCD_CR_DBI_COL_OUT_RGB565
// 					    		-> LCD_CR_DBI_COL_OUT_RGB666

#define LCD_SEL_HANTRO_MODE			0x02000000	   

#define LCD_CR_DBI_SEL_DBI_B   		0x00800000	   
#define LCD_CR_DBI_COL_IN_LE   		0x00000000
#define LCD_CR_DBI_COL_IN_BE   		0x00400000
#define LCD_CR_DBI_COL_IN_RGB888_U	0x00000000		
#define LCD_CR_DBI_COL_IN_RGB888_P	0x00100000		
#define LCD_CR_DBI_COL_IN_RGB565	0x00200000		
#define LCD_CR_DBI_COL_OUT_RGB332	0x00080000		
#define LCD_CR_DBI_COL_OUT_RGB444	0x00060000		
#define LCD_CR_DBI_COL_OUT_RGB565	0x00040000		
#define LCD_CR_DBI_COL_OUT_RGB666	0x00020000		
#define LCD_CR_DBI_COL_OUT_RGB888	0x00000000		
#define LCD_CR_DBI_T_AHBCLK_1      	0x00000000		
#define LCD_CR_DBI_T_AHBCLK_2      	0x00008000		
#define LCD_CR_DBI_T_AHBCLK_3      	0x00010000		
#define LCD_CR_DBI_T_AHBCLK_4      	0x00018000		
#define LCD_CR_DBI_SEL_DBI_C   		0x00004000
#define LCD_CR_DBI_SEL_DBI_C_OPT1  	0x00000000
#define LCD_CR_DBI_SEL_DBI_C_OPT3  	0x00002000

#define LCDC_CR_DBI_MASK			0x00FFE000
#define LCDC_CR_DBI_COL_FMT_MASK	0x003E0000
#define LCDC_CR_DBI_AHB_CLK_CNT  	0x00018000

#define LCD_CMDR_DBI_DATA_IS_8BIT	0x00000000
#define LCD_CMDR_DBI_DATA_IS_PIXEL	0x00000100



// LCD Controller Fifo Status Register definitions, LCDC_SR
#define LCD_FFEMPTY					0x80000000
#define LCD_FFFULL					0x40000000
#define LCD_RREADY					0x00200000
#define LCD_RREQ_DATA				0x00000000
//GG#define LCD_RREQ_COMMAND		0x00000800
#define LCD_RREQ_COMMAND			0x00000100


#define LCD_WRITE_REG(Reg, Value)		*(volatile UInt32 *)(Reg) = Value
#define LCD_READ_REG(Reg)				(*(volatile UInt32 *)Reg)

// BCM2133 LCD port access, Check FiFo Full before writing
#define	WRITE_LCD_INDEX(x)				{while(LCD_READ_REG(LCDC_SR) & LCD_FFFULL); (*(volatile UInt32 *)LCDC_CMD = (x));}
#define	WRITE_LCD_CMD_DATA(x)	 		{while(LCD_READ_REG(LCDC_SR) & LCD_FFFULL); (*(volatile UInt32 *)LCDC_DATA = (x));}
#define	WRITE_LCD_DATA(x)	 			{while(LCD_READ_REG(LCDC_SR) & LCD_FFFULL); (*(volatile UInt32 *)LCDC_DATA = (x));}

#define LCD_READ_INDEX_REQUEST()		{while( (LCD_READ_REG(LCDC_SR) & LCD_FFEMPTY) == 0); (*(volatile UInt32 *)LCDC_RREQ = LCD_RREQ_COMMAND); while( (LCD_READ_REG(LCDC_SR) & LCD_RREADY) == 0);}
#define LCD_READ_INDEX()				(*(volatile UInt32 *)LCDC_CMD)
#define LCD_READ_DATA_REQUEST()			{while( (LCD_READ_REG(LCDC_SR) & LCD_FFEMPTY) == 0); (*(volatile UInt32 *)LCDC_RREQ = LCD_RREQ_DATA); while( (LCD_READ_REG(LCDC_SR) & LCD_RREADY) == 0);}
#define LCD_READ_DATA()					(*(volatile UInt32 *)LCDC_DATA)


#endif	// _HAL_LCD_DRV_STRUCT_H__

/** @} */

