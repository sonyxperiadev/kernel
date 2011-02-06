/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/


/**
*
*   @file   camdrv_stv0986.h
*
*   @brief  Product Number: BCM9MC2403SD.
*
****************************************************************************/

#if !defined( _CAMDRV_XXXXXX_H__ )
#define _CAMDRV_XXXXXX_H__


// physical camera sizes 
#define CAM_MAX_PIXEL_X             2048        
#define CAM_MAX_PIXEL_Y             1536        
#define CAM_WIDTH_PIXEL             176
#define CAM_HEIGHT_PIXEL            144
#define CAM_BIT_DEPTH               16
#define CAM_PIXEL_PACKING           2                   // GE_EN_565_PACK

// Camera Zoom Properties
#define CAM_ZOOM_CAPABLE            TRUE
#define CAM_MAX_ZOOM                256
#define CAM_MIN_ZOOM                64
#define MAX_ZOOM_STEPS              6
// Camera Image Settings
#define NOMINAL_CONTRAST            5
#define MIN_CONTRAST                0
#define MAX_CONTRAST                11

#define NOMINAL_BRIGHTNESS          5
#define MIN_BRIGHTNESS              0
#define MAX_BRIGHTNESS              11

#define NOMINAL_SATURATION          5
#define MIN_SATURATION              0
#define MAX_SATURATION              11

#define NOMINAL_HUE                 5
#define MIN_HUE                     0
#define MAX_HUE                     11

#define NOMINAL_GAMMA               5
#define MIN_GAMMA                   0
#define MAX_GAMMA                   11

#define NOMINAL_SHARPNESS           5
#define MIN_SHARPNESS               0
#define MAX_SHARPNESS               11

#define NOMINAL_AntiShadingPower    5
#define MIN_AntiShadingPower        0
#define MAX_AntiShadingPower        11

// color conversion fractional coefficients are scaled by 2^8
// e.g. for R0 = 1.164, round(1.164 * 256) = 298 or 0x12a
// per Samsung data sheet, use the following coefficients for their camera module
// RO = 1.00,   0x0100
// R1 = 1.402,  0x0167
// G0 = 0.336,  0x0056
// G1 = 0.7424, 0x00B7
// B1 = 1.772,  0x01C6
#define CAM_COLOR_R1R0              0x01670100
#define CAM_COLOR_G1G0              0x00B70056
#define CAM_COLOR_B1                0x000001C6
    
// I2c Device Info
#define I2C_CAM_DEVICE_ID           0x20
#define I2C_CAM_MAX_PAGE            (1024*64+64)

#endif  // _CAMDRV_XXXXXX_H__

