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


/*
*
*   file   cam_drv.c
*
*   brief  This file is the low level driver for BCM2132 camera controller
*
****************************************************************************/

//******************************************************************************
//                          Include block
//******************************************************************************

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/clk.h>

#include <mach/reg_camera.h>
#include <mach/reg_clkpwr.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <plat/syscfg.h>

#include <linux/broadcom/types.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/hal_camera.h>
#include <linux/broadcom/bcm_sysctl.h>

#include "hal_cam.h"
#include "hal_cam_drv.h"
#include "cam_cntrl_2153.h"

extern struct clk *cam_clk;

//******************************************************************************
//							definition block
//******************************************************************************

 
//***********************************************
//	extern function declaration
//***********************************************
//extern void PWRMGMT_48MPLL_Enable( const char* client_name );
//extern void PWRMGMT_48MPLL_Disable( const char* client_name );
//extern void PWRMGMT_48MPLL_RegisterClient( const char* client_name );

//******************************************************************************
//
// Function Name:	CAM_CNTRL_ResetController
//
// Description:	This function Resets BCM CAM controller to default states
//											
// Notes:
//
//******************************************************************************
void CAM_CNTRL_ResetController( void )
{
	UInt32 tmp32;

// set up CAM controller register
	REG_CAM_CR = (REG_CAM_CR_SYNC | REG_CAM_CR_RST | REG_CAM_CR_SDFH( SDFH_NO_SCALE ) | REG_CAM_CR_SDFV( SDFV_NO_SCALE ) | REG_CAM_CR_BYPASS_EN | REG_CAM_CR_FMT_DIRECT);
	REG_CAM_CR2 =  0x00000000;																											// clear word_swap, byte_swap, full_range_yuv(21351,2153)
	tmp32 = REG_CAM_CR;
	REG_CAM_CR = (tmp32 & ~REG_CAM_CR_RST);							// disable reset

#if defined( LITTLE_ENDIAN_SWITCH )
// Byte Swap Input data from Camera Controller
	CAM_CNTRL_SetByteSwap(1);
#endif
// set CAM window start/end line, start/end pixel
	CAM_CNTRL_SetWindow(0,0,0,0);
// clear CAM color conversion registers
	REG_CAM_CC1R = 0x00000000;			
	REG_CAM_CC2R = 0x00000000;
	REG_CAM_CC3R = 0x00000000;
}

//******************************************************************************
//
// Function Name:	CAM_CNTRL_InitController
//
// Description:	This function initialize BCM CAM controller
//											
// Notes:
//
//******************************************************************************
void CAM_CNTRL_InitController( CamIntfConfig_st_t *psensor_intf_config )
{
	HAL_CAM_Result_en_t status;

// set CAM window start/end line, start/end pixel
	status = CAM_CNTRL_SetWindow(0,psensor_intf_config->sensor_config_caps->output_st.max_height,0,psensor_intf_config->sensor_config_caps->output_st.max_width);

// set CAM color conversion registers
	REG_CAM_CC1R = psensor_intf_config->sensor_config_ycbcr->cc_red;			
	REG_CAM_CC2R = psensor_intf_config->sensor_config_ycbcr->cc_green;
	REG_CAM_CC3R = psensor_intf_config->sensor_config_ycbcr->cc_blue;

// set up CAM controller register
	REG_CAM_CR = (REG_CAM_CR_SYNC | REG_CAM_CR_IY | REG_CAM_CR_SDFH(SDFH_NO_SCALE) | REG_CAM_CR_SDFV(SDFV_NO_SCALE) | REG_CAM_CR_BYPASS_EN | REG_CAM_CR_FMT_DIRECT);
	REG_CAM_CR2 =  0x00000000;																											// clear word_swap, byte_swap, full_range_yuv(21351,2153)
#if defined( LITTLE_ENDIAN_SWITCH )
// --------Byte Swap Input data from Camera Controller
	CAM_CNTRL_SetByteSwap(1);
#endif
}


//***************************************************************************
/**
*		CAM_CNTRL_SetSDF sets the scale down factor for the camera controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetSDF( UInt32 horizontalSDF, UInt32 verticalSDF)
{
	UInt32 tmp32;

	tmp32 = REG_CAM_CR & ~(REG_CAM_CR_SDFV_MASK|REG_CAM_CR_SDFH_MASK);
	tmp32 |= REG_CAM_CR_SDFV( verticalSDF );
	tmp32 |= REG_CAM_CR_SDFH( horizontalSDF );
	REG_CAM_CR = tmp32;	                            
    // pr_debug("CAM_CNTRL_SetSDF():  horizontal=%d, vertical=%d \r\n", horizontalSDF, verticalSDF);
}

//***************************************************************************
/**
*		CAM_CNTRL_Enable  enables camera the controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_Enable( void )
{
	UInt32 tmp32;

	tmp32 = REG_CAM_CR;
	REG_CAM_CR = ((tmp32 | REG_CAM_CR_EN) & ~REG_CAM_CR_RST);	// enable camera
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_DisableRst  disables camera and resets the controller
*
*  	@return		none
*
*		Notes:
*/
void CAM_CNTRL_DisableRst( void )
{
	UInt32 tmp32;

	tmp32 = REG_CAM_CR;
	REG_CAM_CR = ((tmp32 & ~REG_CAM_CR_EN) | REG_CAM_CR_RST);	// disable camera
}
/** @} */


//***************************************************************************
/**
*		CAM_CNTRL_Disable  disables camera the controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_Disable( void )
{
	UInt32 tmp32;

	tmp32 = REG_CAM_CR;
	REG_CAM_CR = (tmp32 & ~(REG_CAM_CR_EN | REG_CAM_CR_RST));	// disable camera
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetSYNC  enable/disables camera controller VSYNC/HSYNC
*
*  	@return		none
*
*		Notes:
*/
void CAM_CNTRL_SetSYNC( Boolean sync )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_SYNC) );
	
	if( sync )
	{
		tmp32 |= REG_CAM_CR_SYNC ;
	}
	REG_CAM_CR = tmp32;	// enable camera
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetVsyncInv  Set Vsync Inverse for camera controller Vsync signal (normal=Low True)
*
*  	@return		none
*
*		Notes:
*/
void CAM_CNTRL_SetVsyncInv( Boolean sync )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_VSINV ) );
	if (sync)
	{
		tmp32 |= REG_CAM_CR_VSINV ;
	}
	REG_CAM_CR = tmp32;						// set control register
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetHsyncInv  Set Hsync Inverse for camera controller Hsync signal (normal=High True)
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetHsyncInv( Boolean sync )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_HSINV) );
	if (sync)
	{
		tmp32 |= REG_CAM_CR_HSINV;
	}
	REG_CAM_CR = tmp32;						// set control register
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetHsyncControl  Set Hsync to use/ignore pluses during Vsync blanking (default=use pulses=0)
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetHsyncControl( Boolean control )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR2 & (~REG_CAM_CR2_HSYNCCONTROL) );				// use pulses		
	if (control)
	{
		tmp32 |= REG_CAM_CR2_HSYNCCONTROL;																					// ignore pulses
	}
	REG_CAM_CR2 =  tmp32;																	// set control register
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetClockInv  Set Main Clock Inverse for camera controller Clock signal (normal=High True)
*
*  	@return		none
*
*		Notes:
*/
void CAM_CNTRL_SetClockInv( Boolean sync )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_DCKINV) );
	if (sync)
	{
		tmp32 |= REG_CAM_CR_DCKINV;
	}
	REG_CAM_CR = tmp32;						// set control register
}
/** @} */


 
//***************************************************************************
/**
*		CAM_CNTRL_SetColorConversion Registers
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetColorConversion( UInt32 cam_ccr_r0r1, UInt32 cam_ccr_g0g1, UInt32 cam_ccr_b1 )
{
	//--- set CAM color conversion registers
	REG_CAM_CC1R =  cam_ccr_r0r1;			
	REG_CAM_CC2R =  cam_ccr_g0g1;
	REG_CAM_CC3R =  cam_ccr_b1;
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetYuvFullRange  enable/disables camera controller 2 YUV Full Range
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetYuvFullRange( Boolean range )
{
		HAL_CAM_Result_en_t result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;

	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR2 & (~REG_CAM_CR2_YUVFULLRANGE) );
		
	if( range )
	{
		tmp32 |= REG_CAM_CR2_YUVFULLRANGE ;
	}
	REG_CAM_CR2 =  tmp32;	// update Camera Control Register 2
	result = HAL_CAM_SUCCESS;
	return result;
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetByteSwap  enable/disables camera controller 2 byte swap
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetByteSwap( Boolean swap )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR2 & (~REG_CAM_CR2_BYTESWAP) );
	
	if( swap )
	{
		tmp32 |= REG_CAM_CR2_BYTESWAP ;
	}
	REG_CAM_CR2 =  tmp32;	// update Camera Control Register 2
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetWordSwap  enable/disables camera controller 2 word swap
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetWordSwap( Boolean swap )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR2 & (~REG_CAM_CR2_WORDSWAP) );
	
	if( swap )
	{
		tmp32 |= REG_CAM_CR2_WORDSWAP;
	}
	REG_CAM_CR2 =  tmp32;	// update Camera Control Register 2
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetSwapInputY  TRUE = swap Y components.
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetSwapIY( Boolean Swap )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_IY) );

	if(Swap)
	{
		tmp32 |= REG_CAM_CR_IY;
	}
	REG_CAM_CR = tmp32;						// set control register
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetSwapUV  TRUE = swap UV components.
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
void CAM_CNTRL_SetSwapUV( Boolean Swap )
{
	UInt32 tmp32;

	tmp32 = ( REG_CAM_CR & (~REG_CAM_CR_IUV) );

	if(Swap)
	{
		tmp32 |= REG_CAM_CR_IUV;
	}
	REG_CAM_CR = tmp32;						// set control register
}
/** @} */


//***************************************************************************
/**
*		 CAM_CNTRL_SetDataFmt sets the data format for the camera controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
//static
HAL_CAM_Result_en_t CAM_CNTRL_SetDataFmt(
		CamDataFmt_t fmt							///< (in) camera output format.
		)
{
	UInt32 tmp32;
	HAL_CAM_Result_en_t status;

	switch(fmt)
	{
		case CamDataFmtRGB565:
			tmp32 = (REG_CAM_CR & ~(REG_CAM_CR_FMT_MASK | REG_CAM_CR_BYPASS_EN)) | REG_CAM_CR_FMT_RGB565;
			REG_CAM_CR = tmp32;
			CAM_CNTRL_SetSwapIY(1);
#if defined(CONFIG_ARCH_BCM116X)
			// Byte Swap Input data from Camera Controller
			CAM_CNTRL_SetByteSwap(1);
#endif
			status= HAL_CAM_SUCCESS;
			pr_debug("CAM_CNTRL_SetDataFmt():  CamDataFmtRGB565 \r\n");
			break;
		case CamDataFmtYCbCr:
			tmp32 = (REG_CAM_CR & ~ REG_CAM_CR_FMT_MASK) | REG_CAM_CR_BYPASS_EN | REG_CAM_CR_FMT_DIRECT;
			REG_CAM_CR = tmp32;
#if defined(CONFIG_ARCH_BCM116X)
			CAM_CNTRL_SetSwapIY(0);
#endif
			CAM_CNTRL_SetByteSwap(0);
			status= HAL_CAM_SUCCESS;
			pr_debug("CAM_CNTRL_SetDataFmt():  CamDataFmtYCbCr \r\n");
			break;
		case CamDataFmtYUV:
		case CamDataFmtJPEG:
		case CamDataFmtBypass:
		default:
			tmp32 = ( (REG_CAM_CR & ~(REG_CAM_CR_FMT_MASK|REG_CAM_CR_IUV|REG_CAM_CR_IY|REG_CAM_CR_DITH)) | REG_CAM_CR_BYPASS_EN | REG_CAM_CR_FMT_DIRECT );
			REG_CAM_CR = tmp32;
			CAM_CNTRL_SetYuvFullRange( 1 );
			CAM_CNTRL_SetByteSwap(0);
			CAM_CNTRL_SetWordSwap(0);
			status= HAL_CAM_SUCCESS;
			pr_debug("CAM_CNTRL_SetDataFmt():  JPEG, YUV420, Bypass, Default \r\n");
			break;
	}
	return status;
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetWindow sets the vertical and horizontal registers in 
*														in camera controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
 HAL_CAM_Result_en_t CAM_CNTRL_SetWindow( UInt32 start_line, UInt32 end_line, UInt32 start_pixel, UInt32 end_pixel )
{
	//--- set CAM window vertical register
	REG_CAM_WVRS = REG_CAM_WVR_SET(start_line);				
	REG_CAM_WVRE = REG_CAM_WVR_SET(end_line);	
	//--- set CAM window horizontal register
	REG_CAM_WHR = REG_CAM_WHR_SET(start_pixel,end_pixel);			
	pr_debug("CAM_CNTRL_SetWindow():  start_line=%d, end_line=%d start_pixel=%d, end_pixel=%d \r\n", start_line, end_line, start_pixel, end_pixel);
	return HAL_CAM_SUCCESS;
}

//***************************************************************************
/**
*		CAM_CNTRL_GetWindowV Gets the vertical registers in 
*														in camera controller
*
*  	@return UInt32 Vertical
*
*		Notes:
*/
 UInt32 CAM_CNTRL_GetWindowV( void )
{
	//--- get CAM window vertical register
	return REG_CAM_WVRE;
}
//***************************************************************************
/**
*		CAM_CNTRL_GetWindowH Gets the horizontal registers in 
*														in camera controller
*
*  	@return  UInt32 horizontal
*
*		Notes:
*/
 UInt32 CAM_CNTRL_GetWindowH( void )
{
	//--- get CAM window horizontal register
	return REG_CAM_WHR;
}

//***************************************************************************
/**
*		CAM_CNTRL_GetConfig  Returns Controller Configuration for Camera
*
*  	@return  Int32 config
*
*		Notes:
*/
UInt32	 
CAM_CNTRL_GetConfig()
{
	return 	REG_CAM_CR;
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_GetSDF gets the scale down factor for the camera controller
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*/
UInt32 CAM_CNTRL_GetSDF( void )
{
	UInt32 tmp32;

	tmp32 = ( ((REG_CAM_CR & SDFV_MASK) >> SDFV_SHIFT) << 16 );
	tmp32 |= ( (REG_CAM_CR & SDFH_MASK) >> SDFH_SHIFT );
	return tmp32;
}
/** @} */

//***************************************************************************
/**
*		CAM_CNTRL_SetClkFreq will setup the output clock for camera sensor
*
*  	@return  HAL_CAM_Result_en_t
*
*		Notes:
*  // programming the CMI clock:
*  // 0 = 12 MHz
*  // 1 = 13 MHz
*  // 2 = 24 MHz
*  // 3 = 26 MHz
*  // 4 = 48 MHz
*  // 5 = 52 MHz
*  // 6 = 78 MHz
*/
HAL_CAM_Result_en_t CAM_CNTRL_SetClkFreq( 
	CamClkSel_t cam_clk_speed
	)
{
	HAL_CAM_Result_en_t ret_val = HAL_CAM_SUCCESS;

#if 1
	unsigned long rate, cur_rate;

	switch(cam_clk_speed) {
		case CamDrv_12MHz : rate = 12000000; break;
		case CamDrv_13MHz : rate = 13000000; break;
		case CamDrv_24MHz : rate = 24000000; break;
		case CamDrv_26MHz : rate = 26000000; break;
		case CamDrv_48MHz : rate = 48000000; break;
		case CamDrv_NO_CLK: rate = 0; break;
		default: return HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
	}

	if (rate) {
		cur_rate = clk_get_rate(cam_clk);
		if (rate != cur_rate) {
			if (clk_set_rate(cam_clk, rate))
				return HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
		}
		clk_disable(cam_clk);
		clk_enable(cam_clk);
	}
	else
		clk_disable(cam_clk);

	return ret_val;
#else
//---------Disable Camera Clock
	REG_CLKPWR_CLK_CAMCK_ENABLE = 0;

//---------Set Camera Clock Speed & Enable clock
	switch(cam_clk_speed)
	{
		case CamDrv_12MHz :
			// Enable PLL output and power
			//PWRMGMT_48MPLL_Enable("CAM");			
			// Set the clock to Camera
			REG_CLKPWR_CLK_CAMCK_MODE = REG_CLKPWR_CLK_CAMCK_MODE_12MHZ;
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 1;
			break;
		case CamDrv_13MHz :
			// Set the clock to Camera
			REG_CLKPWR_CLK_CAMCK_MODE = REG_CLKPWR_CLK_CAMCK_MODE_13MHZ;
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 1;
			break;
		case CamDrv_24MHz :
			// Enable PLL output and power
			//PWRMGMT_48MPLL_Enable("CAM");			
			// Set the clock to Camera
			REG_CLKPWR_CLK_CAMCK_MODE = REG_CLKPWR_CLK_CAMCK_MODE_24MHZ;
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 1;
			break;
		case CamDrv_26MHz :
			// Set the clock to Camera
			REG_CLKPWR_CLK_CAMCK_MODE = REG_CLKPWR_CLK_CAMCK_MODE_26MHZ;
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 1;
			break;
		case CamDrv_48MHz :
			// Enable PLL output and power
			//PWRMGMT_48MPLL_Enable("CAM");			
			// Set the clock to Camera
			REG_CLKPWR_CLK_CAMCK_MODE = REG_CLKPWR_CLK_CAMCK_MODE_48MHZ;
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 1;
			break;
		case CamDrv_NO_CLK:
			// CamDrv_NO_CLK gates the clock to camera
	        REG_CLKPWR_CLK_CAMCK_ENABLE = 0;
			// Disable PLL output and power
			//PWRMGMT_48MPLL_Disable("CAM");
			break;
		default:
			ret_val = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
			break;
	}
	return ret_val;
#endif
} // CAM_SetupCamClock

/** @} */


/**
*  This function sets up the IOCR registers for 2153 Camera, based on Baseband Chip.
*
*/
void CAM_CNTRL_IOCR_Cfg(CamSensorSeqSel_t mode)
{
	if ( (mode == SensorPwrDn) || (mode == SensorInitPwrDn) )
		board_sysconfig(SYSCFG_CAMERA, SYSCFG_DISABLE);
	else
		board_sysconfig(SYSCFG_CAMERA, SYSCFG_ENABLE);
}


