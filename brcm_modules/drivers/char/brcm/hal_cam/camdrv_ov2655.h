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
*   @file   camdrv_OV2655.h
*
*   @brief  Camera driver for OV2655 ISP/Sensor
*
****************************************************************************/

#if !defined( _CAMDRV_XXXXXX_H__ )
#define _CAMDRV_XXXXXX_H__

// physical camera sizes 
#define CAM_MAX_PIXEL_X             2048        
#define CAM_MAX_PIXEL_Y             1536       
#define CAM_BIT_DEPTH               16
#define CAM_PIXEL_PACKING           2                   // GE_EN_565_PACK


// Camera Zoom Properties
#define CAM_ZOOM_CAPABLE            FALSE

// These are from another sensor.
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
#define I2C_CAM_DEVICE_ID           0x60
#define I2C_CAM_MAX_PAGE            (1024*64+64)

#endif  // _CAMDRV_XXXXXX_H__

