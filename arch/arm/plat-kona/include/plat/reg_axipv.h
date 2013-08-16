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

#ifndef __REG_AXIPV_H__
#define __REG_AXIPV_H__

/* Register offsets */
#define REG_NXT_FRAME	0x00
#define REG_CUR_FRAME	0x04
#define REG_LINE_STRIDE	0x0C
#define REG_BYTES_PER_LINE	0x10
#define REG_LINES_PER_FRAME	0x14
#define REG_BURST_LEN	0x18
#define REG_W_LVL_1	0x1C
#define REG_W_LVL_2	0x20
#define REG_PV_THRESH	0x24
#define REG_TE_LINE	0x28
#define REG_CTRL	0x2C
#define REG_AXIPV_STATUS	0x30
#define REG_INTR_EN	0x34
#define REG_INTR_STAT	0x38
#define REG_INTR_CLR	0x3C
#define REG_AXI_ID_CFG_1	0x40
#define REG_AXI_ID_CFG_2	0x44
#define REG_LB_EMPTY_THRES	0x48
#define REG_LB_WPTR	0x50
#define REG_LB_RPTR	0x54
#define REG_LB_DATA_H	0x58
#define REG_LB_DATA_L	0x5C

#define PV_START_THRESH_INT (1<<7)
#define AXIPV_DISABLED_INT	(1<<6)
#define FRAME_END_INT	(1<<5)
#define ERR_ON_RDATA_INT	(1<<4)
#define FRAME_UNDFL_INT	(1<<3)
#define LINE_BUF_UNDFL_INT	(1<<2)
#define WATER_LVL2_INT	(1<<1)
#define TE_INT	(1<<0)

#define CUR_LINE_NUM_SHIFT 16

#define AXIPV_BDG_CLK_DIV_SHIFT 27
#define AXIPV_DBG_CLK_DIV_1	(0<<27)
#define AXIPV_DBG_CLK_DIV_2	(1<<27)
#define AXIPV_DBG_CLK_DIV_4	(2<<27)
#define AXIPV_DBG_CLK_DIV_8	(3<<27)
#define TEST_DATA_RDY	(1<<20)
#define AXIPV_TESTMODE	(1<<19)
#define ARCACHE_SHIFT 15
#define ARPROT_SHIFT 12
#define AXI_ID_SYS_DUAL	(1<<11)
#define NUM_OUTSTANDING_XFERS_SHIFT 9
#define NUM_OUTSTDG_XFERS_1	(0<<9)
#define NUM_OUTSTDG_XFERS_2	(1<<9)
#define NUM_OUTSTDG_XFERS_4	(2<<9)
#define NUM_OUTSTDG_XFERS_8	(3<<9)
#define PIXEL_FORMAT_SHIFT 5
#define PIXEL_FORMAT_MASK (7 << PIXEL_FORMAT_SHIFT)
#define PIXEL_FORMAT_24BPP_RGB (0<<5)
#define PIXEL_FORMAT_24BPP_BGR (1<<5)
#define PIXEL_FORMAT_16BPP_PACKED (2<<5)
#define PIXEL_FORMAT_16BPP_UNPACKED (3<<5)
#define PIXEL_FORMAT_18BPP_PACKED (4<<5)
#define PIXEL_FORMAT_18BPP_UNPACKED (5<<5)
#define AXIPV_ACTIVE	(1<<4)
#define AXIPV_CMD_MODE	(1<<3)
#define AXIPV_SINGLE_SHOT (1<<2)
#define SFT_RSTN_MASK	(~(1<<1))
#define SFT_RSTN_DONE	(1<<1)
#define AXIPV_EN	(1<<0)

/* In a multiple of 8bytes*/
#define AXIPV_LB_SIZE (16 * 1024/8)

/* Todo: Need to choose right values*/
#define AXIPV_BURST_LEN 7
#define AXIPV_TE_LINE 600
#define AXIPV_PV_THRES ((AXIPV_LB_SIZE * 7) / 8)
#define AXIPV_W_LVL_1 ((AXIPV_LB_SIZE * 5) / 8 - 1)
#define AXIPV_W_LVL_2 (AXIPV_LB_SIZE / 4)
#define AXIPV_LB_EMPTY_THRES (AXIPV_LB_SIZE / 20)
#define AXIPV_LB_EMPTY_THRES_MIN 0
#define AXIPV_W_LVL_2_MIN (AXIPV_LB_EMPTY_THRES_MIN + 1)
#define AXIPV_W_LVL_1_MIN (AXIPV_W_LVL_2_MIN + 1)
#define AXIPV_PV_THRES_MIN (AXIPV_W_LVL_1_MIN + 1)
#define AXIPV_AXI_ID1 0
#define AXIPV_AXI_ID2 0x80
#define AXIPV_MAX_XFERS NUM_OUTSTDG_XFERS_8
#define AXIPV_ARPROT (2 << ARPROT_SHIFT)
#define AXIPV_ARCACHE (2 << ARCACHE_SHIFT)

#endif /* __REG_AXIPV_H__ */
