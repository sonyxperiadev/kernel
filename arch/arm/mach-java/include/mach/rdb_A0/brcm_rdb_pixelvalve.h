/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PIXELVALVE_H__
#define __BRCM_RDB_PIXELVALVE_H__

#define PIXELVALVE_C_OFFSET                                               0x00000000
#define PIXELVALVE_C_TYPE                                                 UInt32
#define PIXELVALVE_C_RESERVED_MASK                                        0xFF000000
#define    PIXELVALVE_C_PIX_F_SHIFT                                       21
#define    PIXELVALVE_C_PIX_F_MASK                                        0x00E00000
#define    PIXELVALVE_C_FIFO_FULL_LEVEL_SHIFT                             15
#define    PIXELVALVE_C_FIFO_FULL_LEVEL_MASK                              0x001F8000
#define    PIXELVALVE_C_FIFOCLRC_SHIFT                                    14
#define    PIXELVALVE_C_FIFOCLRC_MASK                                     0x00004000
#define    PIXELVALVE_C_HVS_PIXRC1_SHIFT                                  13
#define    PIXELVALVE_C_HVS_PIXRC1_MASK                                   0x00002000
#define    PIXELVALVE_C_HVS_PIXRC0_SHIFT                                  12
#define    PIXELVALVE_C_HVS_PIXRC0_MASK                                   0x00001000
#define    PIXELVALVE_C_PIX_STRETCH_SHIFT                                 4
#define    PIXELVALVE_C_PIX_STRETCH_MASK                                  0x00000FF0
#define    PIXELVALVE_C_PCLK_SEL_SHIFT                                    2
#define    PIXELVALVE_C_PCLK_SEL_MASK                                     0x0000000C
#define    PIXELVALVE_C_FIFO_CLR_SHIFT                                    1
#define    PIXELVALVE_C_FIFO_CLR_MASK                                     0x00000002
#define    PIXELVALVE_C_PVEN_SHIFT                                        0
#define    PIXELVALVE_C_PVEN_MASK                                         0x00000001

#define PIXELVALVE_VC_OFFSET                                              0x00000004
#define PIXELVALVE_VC_TYPE                                                UInt32
#define PIXELVALVE_VC_RESERVED_MASK                                       0xFF800000
#define    PIXELVALVE_VC_VSYNCD_SHIFT                                     6
#define    PIXELVALVE_VC_VSYNCD_MASK                                      0x007FFFC0
#define    PIXELVALVE_VC_FIRST_INT_POL_ODD_SHIFT                          5
#define    PIXELVALVE_VC_FIRST_INT_POL_ODD_MASK                           0x00000020
#define    PIXELVALVE_VC_INT_MODE_SHIFT                                   4
#define    PIXELVALVE_VC_INT_MODE_MASK                                    0x00000010
#define    PIXELVALVE_VC_DSI_VMODE_SHIFT                                  3
#define    PIXELVALVE_VC_DSI_VMODE_MASK                                   0x00000008
#define    PIXELVALVE_VC_MODE_SHIFT                                       2
#define    PIXELVALVE_VC_MODE_MASK                                        0x00000004
#define    PIXELVALVE_VC_FRAMEC_SHIFT                                     1
#define    PIXELVALVE_VC_FRAMEC_MASK                                      0x00000002
#define    PIXELVALVE_VC_VIDEN_SHIFT                                      0
#define    PIXELVALVE_VC_VIDEN_MASK                                       0x00000001

#define PIXELVALVE_VSYNCD_EVEN_OFFSET                                     0x00000008
#define PIXELVALVE_VSYNCD_EVEN_TYPE                                       UInt32
#define PIXELVALVE_VSYNCD_EVEN_RESERVED_MASK                              0xFFFE0000
#define    PIXELVALVE_VSYNCD_EVEN_VSYNCD_EVEN_SHIFT                       0
#define    PIXELVALVE_VSYNCD_EVEN_VSYNCD_EVEN_MASK                        0x0001FFFF

#define PIXELVALVE_HORZA_OFFSET                                           0x0000000C
#define PIXELVALVE_HORZA_TYPE                                             UInt32
#define PIXELVALVE_HORZA_RESERVED_MASK                                    0x00000000
#define    PIXELVALVE_HORZA_HBP_WIDTH_SHIFT                               16
#define    PIXELVALVE_HORZA_HBP_WIDTH_MASK                                0xFFFF0000
#define    PIXELVALVE_HORZA_HS_WIDTH_SHIFT                                0
#define    PIXELVALVE_HORZA_HS_WIDTH_MASK                                 0x0000FFFF

#define PIXELVALVE_HORZB_OFFSET                                           0x00000010
#define PIXELVALVE_HORZB_TYPE                                             UInt32
#define PIXELVALVE_HORZB_RESERVED_MASK                                    0x00000000
#define    PIXELVALVE_HORZB_HFP_WIDTH_SHIFT                               16
#define    PIXELVALVE_HORZB_HFP_WIDTH_MASK                                0xFFFF0000
#define    PIXELVALVE_HORZB_HACT_WIDTH_SHIFT                              0
#define    PIXELVALVE_HORZB_HACT_WIDTH_MASK                               0x0000FFFF

#define PIXELVALVE_VERTA_OFFSET                                           0x00000014
#define PIXELVALVE_VERTA_TYPE                                             UInt32
#define PIXELVALVE_VERTA_RESERVED_MASK                                    0x00000000
#define    PIXELVALVE_VERTA_VBP_WIDTH_SHIFT                               16
#define    PIXELVALVE_VERTA_VBP_WIDTH_MASK                                0xFFFF0000
#define    PIXELVALVE_VERTA_VS_WIDTH_SHIFT                                0
#define    PIXELVALVE_VERTA_VS_WIDTH_MASK                                 0x0000FFFF

#define PIXELVALVE_VERTB_OFFSET                                           0x00000018
#define PIXELVALVE_VERTB_TYPE                                             UInt32
#define PIXELVALVE_VERTB_RESERVED_MASK                                    0x00000000
#define    PIXELVALVE_VERTB_VFP_WIDTH_SHIFT                               16
#define    PIXELVALVE_VERTB_VFP_WIDTH_MASK                                0xFFFF0000
#define    PIXELVALVE_VERTB_VACT_WIDTH_SHIFT                              0
#define    PIXELVALVE_VERTB_VACT_WIDTH_MASK                               0x0000FFFF

#define PIXELVALVE_VERTA_EVEN_OFFSET                                      0x0000001C
#define PIXELVALVE_VERTA_EVEN_TYPE                                        UInt32
#define PIXELVALVE_VERTA_EVEN_RESERVED_MASK                               0x00000000
#define    PIXELVALVE_VERTA_EVEN_VBP_WIDTH_EVEN_SHIFT                     16
#define    PIXELVALVE_VERTA_EVEN_VBP_WIDTH_EVEN_MASK                      0xFFFF0000
#define    PIXELVALVE_VERTA_EVEN_VS_WIDTH_EVEN_SHIFT                      0
#define    PIXELVALVE_VERTA_EVEN_VS_WIDTH_EVEN_MASK                       0x0000FFFF

#define PIXELVALVE_VERTB_EVEN_OFFSET                                      0x00000020
#define PIXELVALVE_VERTB_EVEN_TYPE                                        UInt32
#define PIXELVALVE_VERTB_EVEN_RESERVED_MASK                               0x00000000
#define    PIXELVALVE_VERTB_EVEN_VFP_WIDTH_EVEN_SHIFT                     16
#define    PIXELVALVE_VERTB_EVEN_VFP_WIDTH_EVEN_MASK                      0xFFFF0000
#define    PIXELVALVE_VERTB_EVEN_VACT_WIDTH_EVEN_SHIFT                    0
#define    PIXELVALVE_VERTB_EVEN_VACT_WIDTH_EVEN_MASK                     0x0000FFFF

#define PIXELVALVE_INTEN_OFFSET                                           0x00000024
#define PIXELVALVE_INTEN_TYPE                                             UInt32
#define PIXELVALVE_INTEN_RESERVED_MASK                                    0xFFFFF800
#define    PIXELVALVE_INTEN_PV_UF_SHIFT                                   10
#define    PIXELVALVE_INTEN_PV_UF_MASK                                    0x00000400
#define    PIXELVALVE_INTEN_VID_IDLE_SHIFT                                9
#define    PIXELVALVE_INTEN_VID_IDLE_MASK                                 0x00000200
#define    PIXELVALVE_INTEN_VFP_END_SHIFT                                 8
#define    PIXELVALVE_INTEN_VFP_END_MASK                                  0x00000100
#define    PIXELVALVE_INTEN_VFP_START_SHIFT                               7
#define    PIXELVALVE_INTEN_VFP_START_MASK                                0x00000080
#define    PIXELVALVE_INTEN_VACT_START_SHIFT                              6
#define    PIXELVALVE_INTEN_VACT_START_MASK                               0x00000040
#define    PIXELVALVE_INTEN_VBP_START_SHIFT                               5
#define    PIXELVALVE_INTEN_VBP_START_MASK                                0x00000020
#define    PIXELVALVE_INTEN_VSYNC_START_SHIFT                             4
#define    PIXELVALVE_INTEN_VSYNC_START_MASK                              0x00000010
#define    PIXELVALVE_INTEN_HFP_START_SHIFT                               3
#define    PIXELVALVE_INTEN_HFP_START_MASK                                0x00000008
#define    PIXELVALVE_INTEN_HACT_START_SHIFT                              2
#define    PIXELVALVE_INTEN_HACT_START_MASK                               0x00000004
#define    PIXELVALVE_INTEN_HBP_START_SHIFT                               1
#define    PIXELVALVE_INTEN_HBP_START_MASK                                0x00000002
#define    PIXELVALVE_INTEN_HSYNC_START_SHIFT                             0
#define    PIXELVALVE_INTEN_HSYNC_START_MASK                              0x00000001

#define PIXELVALVE_INTSTAT_OFFSET                                         0x00000028
#define PIXELVALVE_INTSTAT_TYPE                                           UInt32
#define PIXELVALVE_INTSTAT_RESERVED_MASK                                  0xFFFFF800
#define    PIXELVALVE_INTSTAT_PV_UF_SHIFT                                 10
#define    PIXELVALVE_INTSTAT_PV_UF_MASK                                  0x00000400
#define    PIXELVALVE_INTSTAT_VID_IDLE_SHIFT                              9
#define    PIXELVALVE_INTSTAT_VID_IDLE_MASK                               0x00000200
#define    PIXELVALVE_INTSTAT_VFP_END_SHIFT                               8
#define    PIXELVALVE_INTSTAT_VFP_END_MASK                                0x00000100
#define    PIXELVALVE_INTSTAT_VFP_START_SHIFT                             7
#define    PIXELVALVE_INTSTAT_VFP_START_MASK                              0x00000080
#define    PIXELVALVE_INTSTAT_VACT_START_SHIFT                            6
#define    PIXELVALVE_INTSTAT_VACT_START_MASK                             0x00000040
#define    PIXELVALVE_INTSTAT_VBP_START_SHIFT                             5
#define    PIXELVALVE_INTSTAT_VBP_START_MASK                              0x00000020
#define    PIXELVALVE_INTSTAT_VSYNC_START_SHIFT                           4
#define    PIXELVALVE_INTSTAT_VSYNC_START_MASK                            0x00000010
#define    PIXELVALVE_INTSTAT_HFP_START_SHIFT                             3
#define    PIXELVALVE_INTSTAT_HFP_START_MASK                              0x00000008
#define    PIXELVALVE_INTSTAT_HACT_START_SHIFT                            2
#define    PIXELVALVE_INTSTAT_HACT_START_MASK                             0x00000004
#define    PIXELVALVE_INTSTAT_HBP_START_SHIFT                             1
#define    PIXELVALVE_INTSTAT_HBP_START_MASK                              0x00000002
#define    PIXELVALVE_INTSTAT_HSYNC_START_SHIFT                           0
#define    PIXELVALVE_INTSTAT_HSYNC_START_MASK                            0x00000001

#define PIXELVALVE_STAT_OFFSET                                            0x0000002C
#define PIXELVALVE_STAT_TYPE                                              UInt32
#define PIXELVALVE_STAT_RESERVED_MASK                                     0xFFFFF000
#define    PIXELVALVE_STAT_HVS_OF_SHIFT                                   11
#define    PIXELVALVE_STAT_HVS_OF_MASK                                    0x00000800
#define    PIXELVALVE_STAT_PV_UF_SHIFT                                    10
#define    PIXELVALVE_STAT_PV_UF_MASK                                     0x00000400
#define    PIXELVALVE_STAT_HVS_UF_SHIFT                                   9
#define    PIXELVALVE_STAT_HVS_UF_MASK                                    0x00000200
#define    PIXELVALVE_STAT_VID_IDLE_SHIFT                                 8
#define    PIXELVALVE_STAT_VID_IDLE_MASK                                  0x00000100
#define    PIXELVALVE_STAT_VFP_SHIFT                                      7
#define    PIXELVALVE_STAT_VFP_MASK                                       0x00000080
#define    PIXELVALVE_STAT_VACT_SHIFT                                     6
#define    PIXELVALVE_STAT_VACT_MASK                                      0x00000040
#define    PIXELVALVE_STAT_VBP_SHIFT                                      5
#define    PIXELVALVE_STAT_VBP_MASK                                       0x00000020
#define    PIXELVALVE_STAT_VSYNC_SHIFT                                    4
#define    PIXELVALVE_STAT_VSYNC_MASK                                     0x00000010
#define    PIXELVALVE_STAT_HFP_SHIFT                                      3
#define    PIXELVALVE_STAT_HFP_MASK                                       0x00000008
#define    PIXELVALVE_STAT_HACT_SHIFT                                     2
#define    PIXELVALVE_STAT_HACT_MASK                                      0x00000004
#define    PIXELVALVE_STAT_HBP_SHIFT                                      1
#define    PIXELVALVE_STAT_HBP_MASK                                       0x00000002
#define    PIXELVALVE_STAT_HSYNC_SHIFT                                    0
#define    PIXELVALVE_STAT_HSYNC_MASK                                     0x00000001

#define PIXELVALVE_DSI_HACT_ACT_OFFSET                                    0x00000030
#define PIXELVALVE_DSI_HACT_ACT_TYPE                                      UInt32
#define PIXELVALVE_DSI_HACT_ACT_RESERVED_MASK                             0xFFFF0000
#define    PIXELVALVE_DSI_HACT_ACT_DSI_HACT_ACT_SHIFT                     0
#define    PIXELVALVE_DSI_HACT_ACT_DSI_HACT_ACT_MASK                      0x0000FFFF

#endif /* __BRCM_RDB_PIXELVALVE_H__ */


