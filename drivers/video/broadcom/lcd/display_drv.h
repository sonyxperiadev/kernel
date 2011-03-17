/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/display_drv.h
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
#ifndef _DISPLAY_DRV_H_
#define _DISPLAY_DRV_H_


#ifdef __cplusplus
extern "C" {
#endif


/** 
 * @addtogroup DisplayGroup
 * @{
 */


/**
*
*  Display Driver Interface Revision
*
*****************************************************************************/
#define DISPDRV_INTERFACE_REV_MAJ    0
#define DISPDRV_INTERFACE_REV_MIN    1

/**
*
*  Display Driver Supported Features
*
*****************************************************************************/
typedef enum
{
    DISPDRV_SUPPORT_NONE  = 0x0,
    DISPDRV_SUPPORT_CORE_FREQ_CHANGE  = 0x1,
    DISPDRV_SUPPORT_RUN_DOMAIN_CHANGE = 0x2
} DISPDRV_SUPPORT_FEATURES_T;

/**
*
*  Display Driver Handle
*
*****************************************************************************/
typedef void* DISPDRV_HANDLE_T;

/**
*
*  Display Power States
*
*****************************************************************************/
typedef enum
{
    DISPLAY_POWER_STATE_MIN,              ///<  Min Number

    DISPLAY_POWER_STATE_OFF,              ///<  Power Off
    DISPLAY_POWER_STATE_ON,               ///<  Power On
    DISPLAY_POWER_STATE_SLEEP,            ///<  Sleep State

    DISPLAY_POWER_STATE_MAX,              ///<  Max Number
} DISPLAY_POWER_STATE_T;

/**
*
*  Display Bus types
*
*****************************************************************************/
typedef enum
{
    DISPLAY_BUS_MIN,      ///<  Min Number

    DISPLAY_BUS_LCDC,     ///<  LCDC interf buses (Z80/M68/DBI-B/DBI-C)
    DISPLAY_BUS_DSI,      ///<  DSI  bus
    DISPLAY_BUS_SMI,      ///<  SMI  interf buses (Z80/M68/DBI Parallel)
    DISPLAY_BUS_SPI,      ///<  SPI  bus
    DISPLAY_BUS_DISPC,    ///<  DISPC  bus   
    
    DISPLAY_BUS_MAX,      ///<  MAX Number
} DISPLAY_BUS_T;


/**
*
*  Display Types
*
*****************************************************************************/
typedef enum
{
    DISPLAY_TYPE_MIN,                ///<  Min Number

    DISPLAY_TYPE_LCD_STD,            ///<  Standard LCD Type
    DISPLAY_TYPE_TV,                 ///<  TV Out
    
    DISPLAY_TYPE_MAX,                ///<  MAX Number
} DISPLAY_TYPE_T;


/**
*
*  Display Open Input Parameters
*
*****************************************************************************/
typedef struct 
{
    UInt32  busId;                  ///<   BusId ( controller ID )
    UInt32  busCh;                  ///<   BusCh ( controller bank )
} DISPDRV_OPEN_PARM_T;


/**
*
*  CONTROL ID Type Definition
*
*****************************************************************************/
typedef enum 
{
    DISPDRV_CTRL_ID_MIN,
    // 001-049  All Display Types 
    DISPDRV_CTRL_ID_GET_FB_ADDR = 1, ///< Get Address OF Frame Buffer
    
    // 050-099  Standard Display
    DISPDRV_CTRL_ID_GET_REG = 50,
    DISPDRV_CTRL_ID_SET_REG,
    DISPDRV_CTRL_ID_GET_CONTRAST,
    DISPDRV_CTRL_ID_SET_CONTRAST,
    

    // 100-149  TV
    DISPDRV_CTRL_ID_SET_TV_STANDARD  = 100, ///< choose ntsc/pal
    DISPDRV_CTRL_ID_SET_MACROVISION  = 101, ///< set macrovision on/off
    DISPDRV_CTRL_ID_SET_TV_INTERFACE = 102, ///< s-video/composite 

    DISPDRV_CTRL_ID_MAX
} DISPDRV_CTRL_ID_T;



//!  Different TV standards.
typedef enum
{
    DISPDRV_TV_STD_NTSC = 0,       //!< NTSC
    DISPDRV_TV_STD_PAL_BDGHI = 1,  //!< PAL-B, D, G, H, and I
    DISPDRV_TV_STD_PAL_M = 2,      //!< PAL-M
    DISPDRV_TV_STD_PAL_N = 3       //!< PAL-N    
} DISPDRV_TV_STD_t;

//!  Different TV Output modes.
typedef enum
{
    DISPDRV_TV_OUTPUTMODE_SVIDEO = 0,       //!< S-Video
    DISPDRV_TV_OUTPUTMODE_COMPOSITE = 1     //!< Composite
} DISPDRV_TV_OUTPUTMODE_t;

//!  Macrovision modes
typedef enum
{
    DISPDRV_TV_MACROVISION_ON  = 0,     //!< Macrovision ON
    DISPDRV_TV_MACROVISION_OFF = 1      //!< Macrovision OFF
} DISPDRV_TV_MACROVISION_t;


/**
*
*  CONTROL ID GET_FB_ADDR Parameters
*
*****************************************************************************/
typedef struct 
{
    void*  frame_buffer;             ///< out  address of frame buffer
} DISPDRV_CTL_GET_FB_ADDR;


/**
*
*  CONTROL ID REG RW ACCESS Data Type
*  
*****************************************************************************/
typedef struct
{
    UInt32   cmnd;       //  <in> REG ADDR
    UInt32   mode;       //  <in> Bus Access Mode ( bus specific )
    UInt32   parmCount;  //  <in> RD & WR, number of data items to RD or WR
    void *   pBuff;      //  <in> RD/WR parm buffer
    Boolean  verbose;    //  <in> print dbg result
} DISPDRV_CTRL_RW_REG;


/**
*
*  Display Driver's Frame Buffer Color Format 
*
*****************************************************************************/
typedef enum
{
    DISPDRV_FB_FORMAT_MIN,        ///<  Min Number
    
    DISPDRV_FB_FORMAT_RGB565,     ///<  RG5565 2Bpp
    DISPDRV_FB_FORMAT_RGB888_P,   ///<  RGB888 3Bpp, Packed   RGB 
    DISPDRV_FB_FORMAT_RGB888_U,   ///<  RGB888 4Bpp, Unpacked xRGB 

    DISPDRV_FB_FORMAT_MAX,        ///<  MAX Number
} DISPDRV_FB_FORMAT_T;

/**
*
*  Display Dither settings
*
*****************************************************************************/
typedef enum
{
    DISPDRV_DITHER_NONE   = 0,       ///< default if not set
    
    DISPDRV_DITHER_RGB666 = 1,       ///< RGB666
    DISPDRV_DITHER_RGB565 = 2,       ///< RGB565
    DISPDRV_DITHER_RGB555 = 3,       ///< RGB555
   
    DISPDRV_DITHER_MAX               ///< max number
} DISPDRV_DITHER_T;

/**
*
*  Display information structure
*
*     M:  Mandatory 
*     NU: Currently Not Used By Architecture (set to 0)
*****************************************************************************/
typedef struct
{
    DISPLAY_TYPE_T             type;             ///< M  display type
    UInt32                     width;            ///< M  width
    UInt32                     height;           ///< M  height
    DISPDRV_FB_FORMAT_T        input_format;     ///< M  frame buffer format
    DISPLAY_BUS_T              bus_type;         ///< NU display bus type
    UInt32                     interlaced;       ///< NU interlaced
    DISPDRV_DITHER_T           output_dither;    ///< NU dither mode for output
    UInt32                     pixel_freq;       ///< NU pixel frequency
    UInt32                     line_rate;        ///< NU line rate in lines per second
} DISPDRV_INFO_T;


/**
*
*  Display Driver CallBack Results
*
*****************************************************************************/
typedef enum
{
    DISPDRV_CB_RES_MIN,      ///< Min Number
    DISPDRV_CB_RES_OK,       ///< No Errors
    DISPDRV_CB_RES_ERR,      ///< Error During Update
    DISPDRV_CB_RES_MAX,      ///< Max Number
} DISPDRV_CB_RES_T;


/**
*
*  API CallBack Function
*
*****************************************************************************/
// old API cb - will become obsolete,used by update DISP DRV API
typedef void (*DISPDRV_CB_T) ( DISPDRV_CB_RES_T res );
// new API cb - used by update_dma_os DISP DRV API
typedef void (*DISPDRV_CB_API_1_1_T) ( DISPDRV_CB_RES_T res, void* pFb );

/**
*
*  Display Driver structure definition
*
*  DISPLAY Driver Interface Return Values: res=0 OK  res !=0  ERR
*
*  All Interface Functions Are Mandatory Except (can be set to NULL by driver):
*      core_freq_change
*      run_domain_change
*****************************************************************************/
typedef struct
{
    Int32    (*init)(void);                                                                                                                ///< Routine to initialise the display driver
    Int32    (*exit)(void);                                                                                                                ///< Routine to shutdown the display driver
    Int32    (*info)(const char **driverName, UInt32 *versionMajor, UInt32 *versionMinor, DISPDRV_SUPPORT_FEATURES_T *feature);            ///< Routine to return a drivers info (name, version etc..)
    Int32    (*open)(const void* params, DISPDRV_HANDLE_T *handle);                                                                        ///< Routine to open a driver
    Int32    (*close)(const DISPDRV_HANDLE_T handle);                                                                                      ///< Routine to close a driver
    Int32    (*core_freq_change)(const UInt32 core_freq_in_hz, const UInt32 pending);                                                      ///< Optional routine to handle clock change messages
    Int32    (*run_domain_change)(const UInt32 new_run_domain_active_state, const UInt32 pending);                                         ///< Optional routine to handle power domain requests
    const DISPDRV_INFO_T* (*get_info)(DISPDRV_HANDLE_T handle);                                                                            ///< Routine to get the display info
    Int32    (*start)(DISPDRV_HANDLE_T handle);                                                                                            ///< Routine to start a display
    Int32    (*stop)(DISPDRV_HANDLE_T handle);                                                                                             ///< Routine to stop a display
    Int32    (*power_control)(DISPDRV_HANDLE_T handle, DISPLAY_POWER_STATE_T powerState);                                                  ///< Routine to control a displays power
    Int32    (*update_no_os)(DISPDRV_HANDLE_T handle, void *fb );                                                                          ///< Routine to update a frame (EXT fb)
    Int32    (*update_dma_os)(DISPDRV_HANDLE_T handle, void *fb, DISPDRV_CB_API_1_1_T apiCb);                                              ///< Routine to update a frame (EXT fb)
    // !!! OCT 07 2010 update will become obsolete once transition to update_dma_os is complete                                                
    Int32    (*update)(DISPDRV_HANDLE_T handle, int fb_idx, DISPDRV_CB_T apiCb);                                                                       ///< Routine to update a frame (INT fb)
    Int32    (*set_control)(DISPDRV_HANDLE_T handle, DISPDRV_CTRL_ID_T controlID, void* controlParams);                                    ///< Routine to set control for the display panel
    Int32    (*get_control)(DISPDRV_HANDLE_T handle, DISPDRV_CTRL_ID_T controlID, void* controlParams);                                    ///< Routine to get control for the display panel

} DISPDRV_T;

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_DISPLAY_DRV_H_


