/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_mipi_dsi.h
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
#ifndef __DISPDRV_MIPI_DSI_H__
#define __DISPDRV_MIPI_DSI_H__

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */

#define DSI_VC_00	                        0
#define DSI_VC_01	                        1
#define DSI_VC_02	                        2
#define DSI_VC_03	                        3


//****************************************************************************
// DSI DT - Data Type Field Definition 
//****************************************************************************
//--- HOST TO PERIPHERAL DTs 1.01.00 r11 --------------------------------------
#define	DSI_DT_SH_VSYNC_START				0x01   
#define	DSI_DT_SH_VSYNC_END					0x11   
#define	DSI_DT_SH_HSYNC_START				0x21   
#define	DSI_DT_SH_HSYNC_END					0x31   
#define	DSI_DT_SH_EOT       				0x08   
#define	DSI_DT_SH_COL_MODE_OFF				0x02   
#define	DSI_DT_SH_COL_MODE_ON				0x12   
#define	DSI_DT_SH_SHUT_DOWN					0x22    
#define	DSI_DT_SH_TURN_ON					0x32   

//#define	DSI_DT_SH_GEN_WR					0x03   
#define	DSI_DT_SH_GEN_WR_P0					0x03   
#define	DSI_DT_SH_GEN_WR_P1					0x13   
#define	DSI_DT_SH_GEN_WR_P2					0x23   

//#define	DSI_DT_SH_GEN_RD					0x04   
#define	DSI_DT_SH_GEN_RD_P0					0x04   
#define	DSI_DT_SH_GEN_RD_P1					0x14   
#define	DSI_DT_SH_GEN_RD_P2					0x24   

//#define	DSI_DT_SH_DCS_WR					0x05   
#define	DSI_DT_SH_DCS_WR_P0					0x05   
#define	DSI_DT_SH_DCS_WR_P1					0x15   
    
//#define	DSI_DT_SH_DCS_RD					0x06   
#define	DSI_DT_SH_DCS_RD_P0					0x06   

#define	DSI_DT_SH_MAX_RET_PKT_SIZE			0x37   

#define	DSI_DT_LG_PKT_NULL					0x09   
#define	DSI_DT_LG_PKT_BLANKING				0x19   
#define	DSI_DT_LG_GEN_WR					0x29   
#define	DSI_DT_LG_GEN_WR_NON_IMAGE			0x29   
#define	DSI_DT_LG_LUT_WR					0x29   
#define	DSI_DT_LG_DCS_WR					0x39   
#define	DSI_DT_LG_STREAM_RGB_565_PACKED		0x0E   
#define	DSI_DT_LG_STREAM_RGB_666_PACKED		0x1E   
#define	DSI_DT_LG_STREAM_RGB_666_LOOSELY	0x2E   
#define	DSI_DT_LG_STREAM_RGB_888_PACKED		0x3E   

//--- PERIPHERAL TO HOST DTs 1.01.00 r11 --------------------------------------
#define	DSI_DT_P2H_TRIG_ACK                 0x84
#define	DSI_DT_P2H_SH_ACK_ERR_RPT			0x02    
#define	DSI_DT_P2H_SH_EOT					0x08    
#define	DSI_DT_P2H_SH_GEN_RD_1B				0x11    
#define	DSI_DT_P2H_SH_GEN_RD_2B				0x12    
#define	DSI_DT_P2H_LG_GEN_RD				0x1A    
#define	DSI_DT_P2H_DCS_LG_RD				0x1C    
#define	DSI_DT_P2H_DCS_SH_RD_1B				0x21    
#define	DSI_DT_P2H_DCS_SH_RD_2B				0x22    

//--- DSI Error Report Bits DSI Spec 1.01.00 r11 ------------------------------
#define DSI_ERR_SOT                         0x00000001
#define DSI_ERR_SOT_SYNC                    0x00000002
#define DSI_ERR_EOT_SYNC                    0x00000004
#define DSI_ERR_ESC_MODE                    0x00000008
#define DSI_ERR_LP_TX_SYNC                  0x00000010
#define DSI_ERR_HS_RX_TO                    0x00000020      
#define DSI_ERR_FALSE_CTRL                  0x00000040
#define DSI_ERR_RES1                        0x00000080
#define DSI_ERR_ECC1                        0x00000100
#define DSI_ERR_ECC2                        0x00000200
#define DSI_ERR_CRC                         0x00000400
#define DSI_ERR_DT                          0x00000800
#define DSI_ERR_VC_ID                       0x00001000
#define DSI_ERR_TX_LEN                      0x00002000
#define DSI_ERR_RES2                        0x00004000
#define DSI_ERR_PROT_VIOL                   0x00008000

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DISPDRV_MIPI_DSI_H__ */
