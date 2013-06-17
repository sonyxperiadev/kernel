/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#ifndef __REG_PV_H__
#define __REG_PV_H__

#define REG_PV_C	0x00
#define REG_PV_VC	0x04
#define REG_PV_VSYNCD_EVEN	0x08
#define REG_PV_HORZA	0x0C
#define REG_PV_HORZB	0x10
#define REG_PV_VERTA	0x14
#define REG_PV_VERTB	0x18
#define REG_PV_VERTA_EVEN	0x1C
#define REG_PV_VERTB_EVEN	0x20
#define REG_PV_INTEN	0x24
#define REG_PV_INTSTAT	0x28
#define REG_PV_STAT	0x2C
#define REG_PV_DSI_HACT_ACT	0x30

#define PIX_CLK_MUX_EN	(1<<24)
#define PIX_F_SHIFT 21
#define FIFO_FULL_LEVEL_SHIFT 15
/* http://jira.broadcom.com/browse/HWHAWAII-248 */
#define PIX_F_18_24BPP_DSI_VIDEO_CMD (4<<21)

#define FIFOCLRC	(1<<14)
#define HVS_PIXRC1	(1<<13)
#define HVS_PIXRC0	(1<<12)
#define PIX_STRETCH_SHIFT 4
#define PCLK_SEL_SHIFT 2
#define PCLK_SEL_0	(0<<2)
#define PCLK_SEL_1	(1<<2)
#define PCLK_SEL_2	(2<<2)
#define FIFO_CLR	(1<<1)
#define PVEN	(1<<0)

#define VSYNCD_SHIFT 6
#define FIRST_INT_POL_ODD	(1<<5)
#define INT_MODE	(1<<4)
#define DSI_VMODE	(1<<3)
#define CMD_MODE	(1<<2)
#define FRAMEC	(1<<1)
#define VIDEN	(1<<0)

#define OF_UF	(1<<10)
#define VID_IDLE	(1<<9)
#define VFP_END	(1<<8)
#define VFP_START	(1<<7)
#define VACT_START	(1<<6)
#define VBP_START	(1<<5)
#define VSYNC_START	(1<<4)
#define HFP_START	(1<<3)
#define HACT_START	(1<<2)
#define HBP_START	(1<<1)
#define HSYNC_START	(1<<0)

#define HVS_OF	(1<<11)
#define PV_UF	(1<<10)
#define HVS_UF	(1<<9)
#define VID_IDLE_STAT	(1<<8)
#define VFP	(1<<7)
#define VACT	(1<<6)
#define VBP	(1<<5)
#define VSYNC	(1<<4)
#define HFP	(1<<3)
#define HACT	(1<<2)
#define HBP	(1<<1)
#define HSYNC	(1<<0)

#define HBP_SHIFT 16
#define HFP_SHIFT 16
#define VBP_SHIFT 16
#define VFP_SHIFT 16

#endif /* __REG_PV_H__ */
