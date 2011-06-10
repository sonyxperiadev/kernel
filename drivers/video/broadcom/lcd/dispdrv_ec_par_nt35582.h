/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "dispdrv_common.h"
*/

//----------------------------------------------------
//--- NOVATEK WVGA LCD Controller NT35582 
//--- 16.77M colors 480RGB x 864/800/640 LTPS TFT LCD
//----------------------------------------------------
#define NT35582_NOP                 0x0000
#define NT35582_SOFT_RST            0x0100
#define NT35582_GET_PWR_MODE        0x0A00
#define NT35582_GET_ADDR_MODE       0x0B00
#define NT35582_GET_PIXEL_FMT       0x0C00
#define NT35582_GET_DISP_MODE       0x0D00
#define NT35582_GET_SIGNAL_MODE     0x0E00
#define NT35582_SLEEP_IN            0x1000
#define NT35582_SLEEP_OUT           0x1100
#define NT35582_PARTIAL_ON          0x1200
#define NT35582_NORMAL_ON           0x1300
#define NT35582_INVERT_OFF          0x2000
#define NT35582_INVERT_ON           0x2100
#define NT35582_DISPLAY_OFF         0x2800
#define NT35582_DISPLAY_ON          0x2900
#define NT35582_SET_HOR_ADDR_S_MSB  0x2A00      // RAM WIN X S   0x00
#define NT35582_SET_HOR_ADDR_S_LSB  0x2A01      //               0x00
#define NT35582_SET_HOR_ADDR_E_MSB  0x2A02      // RAM WIN X E   0x01
#define NT35582_SET_HOR_ADDR_E_LSB  0x2A03      //               0xDF  479
#define NT35582_SET_VER_ADDR_S_MSB  0x2B00      // RAM WIN Y S   0x00
#define NT35582_SET_VER_ADDR_S_LSB  0x2B01      //               0x00
#define NT35582_SET_VER_ADDR_E_MSB  0x2B02      // RAM WIN Y E   0x03
#define NT35582_SET_VER_ADDR_E_LSB  0x2B03      //               0x1F  799
#define NT35582_WR_MEM_START        0x2C00
#define NT35582_SET_RAM_ADDR_X_MSB  0x2D00      // RAM ADDR X S
#define NT35582_SET_RAM_ADDR_X_LSB  0x2D01
#define NT35582_SET_RAM_ADDR_Y_MSB  0x2D02      // RAM ADDR Y S   
#define NT35582_SET_RAM_ADDR_Y_LSB  0x2D03      //                
#define NT35582_RD_MEM_START        0x2E00
#define NT35582_SET_PART_Y_S_MSB    0x3000
#define NT35582_SET_PART_Y_S_LSB    0x3001
#define NT35582_SET_PART_Y_E_MSB    0x3002
#define NT35582_SET_PART_Y_E_LSB    0x3003
#define NT35582_SET_TEAR_OFF        0x3400
#define NT35582_SET_TEAR_ON         0x3500
#define NT35582_SET_ADDR_MODE_0     0x3600
#define NT35582_SET_ADDR_MODE_1     0x3601
#define NT35582_IDLE_OFF            0x3800
#define NT35582_IDLE_ON             0x3900
#define NT35582_SET_PIXEL_FORMAT    0x3A00
#define NT35582_RGB_CTRL            0x3B00
#define NT35582_VBP                 0x3B02
#define NT35582_VFP                 0x3B03
#define NT35582_HBP                 0x3B04
#define NT35582_HFP                 0x3B05
#define NT35582_SET_TEAR_LINE_MSB   0x4400
#define NT35582_SET_TEAR_LINE_LSB   0x4401

#define NT35582_SD_OP_SET_0         0xB600
#define NT35582_SD_OP_SET_1         0xB601
#define NT35582_SD_OP_SET_2         0xB602
#define NT35582_SD_OP_SET_3         0xB603
#define NT35582_SD_OP_SET_4         0xB604
#define NT35582_SD_OP_SET_5         0xB605

#define NT35582_PWCTR1_0            0xC000
#define NT35582_PWCTR1_1            0xC001
#define NT35582_PWCTR1_2            0xC002
#define NT35582_PWCTR1_3            0xC003

#define NT35582_PWCTR2_0            0xC100
#define NT35582_PWCTR2_1            0xC101
#define NT35582_PWCTR2_2            0xC102

#define NT35582_PWCTR3_0            0xC200
#define NT35582_PWCTR3_1            0xC201
#define NT35582_PWCTR3_2            0xC202
#define NT35582_PWCTR3_3            0xC203

#define NT35582_PWCTR4_0            0xC300
#define NT35582_PWCTR4_1            0xC301
#define NT35582_PWCTR4_2            0xC302
#define NT35582_PWCTR4_3            0xC303

#define NT35582_PWCTR5_0            0xC400
#define NT35582_PWCTR5_1            0xC401
#define NT35582_PWCTR5_2            0xC402
#define NT35582_PWCTR5_3            0xC403

#define NT35582_VCOM                0xC700
#define NT35582_RVCOM               0xC800

#define NT35582_GMACTRL_1_00        0xE000
#define NT35582_GMACTRL_1_01        0xE001
#define NT35582_GMACTRL_1_02        0xE002
#define NT35582_GMACTRL_1_03        0xE003
#define NT35582_GMACTRL_1_04        0xE004
#define NT35582_GMACTRL_1_05        0xE005
#define NT35582_GMACTRL_1_06        0xE006
#define NT35582_GMACTRL_1_07        0xE007
#define NT35582_GMACTRL_1_08        0xE008
#define NT35582_GMACTRL_1_09        0xE009
#define NT35582_GMACTRL_1_0A        0xE00A
#define NT35582_GMACTRL_1_0B        0xE00B
#define NT35582_GMACTRL_1_0C        0xE00C
#define NT35582_GMACTRL_1_0D        0xE00D
#define NT35582_GMACTRL_1_0E        0xE00E
#define NT35582_GMACTRL_1_0F        0xE00F
#define NT35582_GMACTRL_1_10        0xE010
#define NT35582_GMACTRL_1_11        0xE011

#define NT35582_GMACTRL_2_00        0xE100
#define NT35582_GMACTRL_2_01        0xE101
#define NT35582_GMACTRL_2_02        0xE102
#define NT35582_GMACTRL_2_03        0xE103
#define NT35582_GMACTRL_2_04        0xE104
#define NT35582_GMACTRL_2_05        0xE105
#define NT35582_GMACTRL_2_06        0xE106
#define NT35582_GMACTRL_2_07        0xE107
#define NT35582_GMACTRL_2_08        0xE108
#define NT35582_GMACTRL_2_09        0xE109
#define NT35582_GMACTRL_2_0A        0xE10A
#define NT35582_GMACTRL_2_0B        0xE10B
#define NT35582_GMACTRL_2_0C        0xE10C
#define NT35582_GMACTRL_2_0D        0xE10D
#define NT35582_GMACTRL_2_0E        0xE10E
#define NT35582_GMACTRL_2_0F        0xE10F
#define NT35582_GMACTRL_2_10        0xE110
#define NT35582_GMACTRL_2_11        0xE111

#define NT35582_GMACTRL_3_00        0xE200
#define NT35582_GMACTRL_3_01        0xE201
#define NT35582_GMACTRL_3_02        0xE202
#define NT35582_GMACTRL_3_03        0xE203
#define NT35582_GMACTRL_3_04        0xE204
#define NT35582_GMACTRL_3_05        0xE205
#define NT35582_GMACTRL_3_06        0xE206
#define NT35582_GMACTRL_3_07        0xE207
#define NT35582_GMACTRL_3_08        0xE208
#define NT35582_GMACTRL_3_09        0xE209
#define NT35582_GMACTRL_3_0A        0xE20A
#define NT35582_GMACTRL_3_0B        0xE20B
#define NT35582_GMACTRL_3_0C        0xE20C
#define NT35582_GMACTRL_3_0D        0xE20D
#define NT35582_GMACTRL_3_0E        0xE20E
#define NT35582_GMACTRL_3_0F        0xE20F
#define NT35582_GMACTRL_3_10        0xE210
#define NT35582_GMACTRL_3_11        0xE211

#define NT35582_GMACTRL_4_00        0xE300
#define NT35582_GMACTRL_4_01        0xE301
#define NT35582_GMACTRL_4_02        0xE302
#define NT35582_GMACTRL_4_03        0xE303
#define NT35582_GMACTRL_4_04        0xE304
#define NT35582_GMACTRL_4_05        0xE305
#define NT35582_GMACTRL_4_06        0xE306
#define NT35582_GMACTRL_4_07        0xE307
#define NT35582_GMACTRL_4_08        0xE308
#define NT35582_GMACTRL_4_09        0xE309
#define NT35582_GMACTRL_4_0A        0xE30A
#define NT35582_GMACTRL_4_0B        0xE30B
#define NT35582_GMACTRL_4_0C        0xE30C
#define NT35582_GMACTRL_4_0D        0xE30D
#define NT35582_GMACTRL_4_0E        0xE30E
#define NT35582_GMACTRL_4_0F        0xE30F
#define NT35582_GMACTRL_4_10        0xE310
#define NT35582_GMACTRL_4_11        0xE311

#define NT35582_GMACTRL_5_00        0xE400
#define NT35582_GMACTRL_5_01        0xE401
#define NT35582_GMACTRL_5_02        0xE402
#define NT35582_GMACTRL_5_03        0xE403
#define NT35582_GMACTRL_5_04        0xE404
#define NT35582_GMACTRL_5_05        0xE405
#define NT35582_GMACTRL_5_06        0xE406
#define NT35582_GMACTRL_5_07        0xE407
#define NT35582_GMACTRL_5_08        0xE408
#define NT35582_GMACTRL_5_09        0xE409
#define NT35582_GMACTRL_5_0A        0xE40A
#define NT35582_GMACTRL_5_0B        0xE40B
#define NT35582_GMACTRL_5_0C        0xE40C
#define NT35582_GMACTRL_5_0D        0xE40D
#define NT35582_GMACTRL_5_0E        0xE40E
#define NT35582_GMACTRL_5_0F        0xE40F
#define NT35582_GMACTRL_5_10        0xE410
#define NT35582_GMACTRL_5_11        0xE411

#define NT35582_GMACTRL_6_00        0xE500
#define NT35582_GMACTRL_6_01        0xE501
#define NT35582_GMACTRL_6_02        0xE502
#define NT35582_GMACTRL_6_03        0xE503
#define NT35582_GMACTRL_6_04        0xE504
#define NT35582_GMACTRL_6_05        0xE505
#define NT35582_GMACTRL_6_06        0xE506
#define NT35582_GMACTRL_6_07        0xE507
#define NT35582_GMACTRL_6_08        0xE508
#define NT35582_GMACTRL_6_09        0xE509
#define NT35582_GMACTRL_6_0A        0xE50A
#define NT35582_GMACTRL_6_0B        0xE50B
#define NT35582_GMACTRL_6_0C        0xE50C
#define NT35582_GMACTRL_6_0D        0xE50D
#define NT35582_GMACTRL_6_0E        0xE50E
#define NT35582_GMACTRL_6_0F        0xE50F
#define NT35582_GMACTRL_6_10        0xE510
#define NT35582_GMACTRL_6_11        0xE511



//----------------------------------------------------
//    BCM92420LC
//--- NOVATEK's WVGA/RGB888 LCD Controller: NT35582  
//----------------------------------------------------

//IM2 IM1 IM0 System Interface Data Pin        Colors
//--------------------------------------------------------------
//  0   0   0 80-system  8-bit interface D07-0 65k, 262k, 16.7M
//  0   0   1 80-system 16-bit interface D15-0 65k, 262k, 16.7M
//  0   1   0 80-system 24-bit interface D23-0 65k, 262k, 16.7M

#define     PIXEL_FORMAT_RGB565  0x55   // for MPU & RGB interface
#define     PIXEL_FORMAT_RGB666  0x66   // for MPU & RGB interface
#define     PIXEL_FORMAT_RGB888  0x77   // for MPU & RGB interface

DISPCTRL_REC_T NT35582_Init[] = {
    {DISPCTRL_WR_CMND         , NT35582_SLEEP_OUT       , 0     },
    {DISPCTRL_SLEEP_MS        , 0                       , 200   },
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR1_0        , (0x86)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR1_1        , (0x00)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR1_2        , (0x86)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR1_3        , (0x00)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR2_0        , (0x45)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR3_0        , (0x21)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_PWCTR3_2        , (0x02)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_SD_OP_SET_0     , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_SD_OP_SET_2     , (0x30)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_VCOM            , (0x8F)},

#if   defined (__WVGA_MODE_888__)    
    {DISPCTRL_WR_CMND_DATA    , NT35582_SET_PIXEL_FORMAT, (PIXEL_FORMAT_RGB888)},
#elif defined (__WVGA_MODE_666__)
    {DISPCTRL_WR_CMND_DATA    , NT35582_SET_PIXEL_FORMAT, (PIXEL_FORMAT_RGB666)},
#elif defined (__WVGA_MODE_565__)
    {DISPCTRL_WR_CMND_DATA    , NT35582_SET_PIXEL_FORMAT, (PIXEL_FORMAT_RGB565)},
#else
#error "dispdrv_ec_par_nt35582.h: Color Mode Must Be Defined In A Diplay Driver!" 
#endif
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_07    , (0x3D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_08    , (0x22)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_09    , (0x2A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0A    , (0x87)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_1_11    , (0x4D)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_07    , (0x3F)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_08    , (0x20)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_09    , (0x26)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0A    , (0x83)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_2_11    , (0x4D)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_07    , (0x3D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_08    , (0x22)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_09    , (0x2A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0A    , (0x87)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_3_11    , (0x4D)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_07    , (0x3F)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_08    , (0x20)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_09    , (0x26)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0A    , (0x83)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_4_11    , (0x4D)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_07    , (0x3D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_08    , (0x22)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_09    , (0x2A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0A    , (0x87)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_5_11    , (0x4D)},
    
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_00    , (0x0E)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_01    , (0x14)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_02    , (0x29)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_03    , (0x3A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_04    , (0x1D)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_05    , (0x30)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_06    , (0x61)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_07    , (0x3F)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_08    , (0x20)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_09    , (0x26)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0A    , (0x83)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0B    , (0x16)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0C    , (0x3B)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0D    , (0x4C)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0E    , (0x78)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_0F    , (0x96)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_10    , (0x4A)},
    {DISPCTRL_WR_CMND_DATA    , NT35582_GMACTRL_6_11    , (0x4D)},

    {DISPCTRL_WR_CMND         , NT35582_DISPLAY_ON      , 0     },

    {DISPCTRL_SLEEP_MS        , 0                       , 200   },

    // FAST Memory Write ON
    {DISPCTRL_SLEEP_MS        , NT35582_SET_ADDR_MODE_1,  1     },

//  {DISPCTRL_WR_CMND         , NT35582_WR_MEM_START    , 0     },
    //--- END OF COMMAND LIST -----------------------
    {DISPCTRL_LIST_END        , 0     , 0       }
};
