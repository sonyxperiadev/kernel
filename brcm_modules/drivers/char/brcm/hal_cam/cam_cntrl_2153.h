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


//
// Description:  This is camera low level driver interface for BCM2132 chip
//
//******************************** History *************************************
//******************************************************************************

#if !defined( _CAM_CNTRL_H__ )
#define _CAM_CNTRL_H__

// ---- Include Files -------------------------------------------------------
#include <linux/types.h>
#include <linux/broadcom/types.h>
#include "hal_cam.h"

#define SDFV_MASK	0x00ff0000
#define SDFH_MASK	0x0000ff00
#define SDFH_SHIFT	8
#define SDFV_SHIFT	16
#define SDFH_NO_SCALE	0x80
#define SDFV_NO_SCALE	0x80
#define SDFH_MAX_SCALE	0x100
#define SDFV_MAX_SCALE	0x100
#define SDF_MAX_SCALE	256
#define SDF_MIN_SCALE	64
#define CAM_YCrCb	0x00000000

//***************************************************************************
//  Defines for Camera Vsync Interrupt
//***************************************************************************
#define VSYNC_IRQ_SRC_TYPE_SHIFT	4
#define HSYNC_IRQ_SRC_TYPE_SHIFT	24
#define	IRQ_SRC_TYPE_MASK		0x0F


// #define SYNC_RISING_EDGE		( 1 << 0 )
// #define SYNC_FALLING_EDGE		( 1 << 1 )
// #define SYNC_ACTIVE_LOW		( 1 << 2 )
// #define SYNC_ACTIVE_HIGH		( 1 << 3 )

// ---- Camera IO Control Register Function Prototypes -----------------------------------------
void CAM_CNTRL_IOCR_Init(CamIntfConfig_st_t *psensor_intf_config);
void CAM_CNTRL_IOCR_PwrUp(CamIntfConfig_st_t *psensor_intf_config);
void CAM_CNTRL_IOCR_PwrDn(CamIntfConfig_st_t *psensor_intf_config);
/**
*  Configure camera IOCR Registers based on mode
*/
void CAM_CNTRL_IOCR_Cfg( CamSensorSeqSel_t mode );

// ---- Camera Controller Register Function Prototypes -----------------------------------------

/**
*  Reset camera controller:  window, color conversion, control register
*/
void CAM_CNTRL_ResetController( void );

/**
*  Init camera controller:  window, color conversion, control register
*/
void CAM_CNTRL_InitController( CamIntfConfig_st_t *psensor_intf_config );

/**
*  Sets the camera controller color conversion factors
*/
void CAM_CNTRL_SetColorConversion( 
		UInt32 cam_ccr_r0r1, 
		UInt32 cam_ccr_g0g1, 
		UInt32 cam_ccr_b1 
		);

/**
*  Sets the camera controller windowing 
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetWindow( 
		UInt32 start_line, 
		UInt32 end_line,  
		UInt32 start_pixel, 
		UInt32 end_pixel 
		);

/**
*  Enables & Releases reset on camera controller
*/
void CAM_CNTRL_Enable( void );
/**
*  Disables & Resets camera controller
*/
void CAM_CNTRL_DisableRst( void );
/**
*  Disablescamera controller
*/
void CAM_CNTRL_Disable( void );

/**
*  Sets the camera controller scale down factor
*/
void CAM_CNTRL_SetSDF( 
		UInt32 horizontalSDF, 	///< (in) scale down horizontal  
		UInt32 verticalSDF 	///< (in) scale down vertical 
		);

/**
*  Gets the camera controller scale down factor
*/
UInt32 CAM_CNTRL_GetSDF( 
		void
		);

/**
*  Gets the camera controller Vertical Window Register
*/
 UInt32 CAM_CNTRL_GetWindowV( void );
/**
*  Gets the camera controller Horizontal Window Register
*/
 UInt32 CAM_CNTRL_GetWindowH( void );

/**
*  Sets the camera controller Sync Enable
*/
void CAM_CNTRL_SetSYNC( 
		Boolean sync 
		);

/**
*  Sets the camera controller Vsync Invert Bit
*/
void CAM_CNTRL_SetVsyncInv(
		Boolean sync 
		);

/**
*  Sets the camera controller Hsync Invert Bit
*/
void CAM_CNTRL_SetHsyncInv(
		Boolean sync 
		);

/**
*  Sets the camera controller Hsync control Bit:  
*     use/ignore pulses during Vsync blanking (default=use pulses=0)
*/
void CAM_CNTRL_SetHsyncControl(
		Boolean sync 
		);

/**
*  Sets the camera controller Clock Invert Bit
*/
void CAM_CNTRL_SetClockInv(
		Boolean control 
		);

/**
*		CAM_CNTRL_SetSwapIY  TRUE = swap Y component.
*/
void CAM_CNTRL_SetSwapIY( 
		Boolean Swap 
		);
		
/**
*		CAM_CNTRL_SetSwapUV  TRUE = swap UV components.
*/
void CAM_CNTRL_SetSwapUV( 
		Boolean Swap 
		);
		
/**
*		 CAM_CNTRL_SetDataFmt sets the data format for the camera controller
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetDataFmt(
		CamDataFmt_t fmt	///< (in) camera output format.
		);

/**
*		CAM_CNTRL_GetConfig  Returns Controller Configuration for Camera
*/
UInt32 CAM_CNTRL_GetConfig(void);

/**
*		CAM_CNTRL_SetClkFreq will setup the output clock for camera sensor
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetClkFreq( 
	CamClkSel_t cam_clk_speed
	);

/**
*  Sets the camera controller YUV Full Range
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetYuvFullRange( 
		Boolean range
		);

/**
*  Sets the camera controller byte swap
*/
void CAM_CNTRL_SetByteSwap( 
		Boolean swap
		);
		
/**
*  Sets the camera controller word swap
*/
void CAM_CNTRL_SetWordSwap( 
		Boolean swap
		);
#endif // #if !defined( _CAM_CNTRL_H__ )
