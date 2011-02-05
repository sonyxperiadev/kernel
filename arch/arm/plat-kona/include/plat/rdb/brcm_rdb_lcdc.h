/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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

#ifndef __BRCM_RDB_LCDC_H__
#define __BRCM_RDB_LCDC_H__

#define LCDC_BASE_ADDR                                                    0x08030000

#define LCDC_CMDR_OFFSET                                                  0x00000000
#define LCDC_CMDR_TYPE                                                    UInt32
#define LCDC_CMDR_RESERVED_MASK                                           0x00000000
#define    LCDC_CMDR_UNION_SHIFT                                          0
#define    LCDC_CMDR_UNION_MASK                                           0xFFFFFFFF

#define LCDC_DATR_OFFSET                                                  0x00000004
#define LCDC_DATR_TYPE                                                    UInt32
#define LCDC_DATR_RESERVED_MASK                                           0x00000000
#define    LCDC_DATR_UNION_SHIFT                                          0
#define    LCDC_DATR_UNION_MASK                                           0xFFFFFFFF

#define LCDC_RREQ_OFFSET                                                  0x00000008
#define LCDC_RREQ_TYPE                                                    UInt32
#define LCDC_RREQ_RESERVED_MASK                                           0x00000000
#define    LCDC_RREQ_UNION_SHIFT                                          0
#define    LCDC_RREQ_UNION_MASK                                           0xFFFFFFFF

#define LCDC_WTR_OFFSET                                                   0x00000010
#define LCDC_WTR_TYPE                                                     UInt32
#define LCDC_WTR_RESERVED_MASK                                            0xFF000000
#define    LCDC_WTR_HOLD_SHIFT                                            16
#define    LCDC_WTR_HOLD_MASK                                             0x00FF0000
#define    LCDC_WTR_PULSE_SHIFT                                           8
#define    LCDC_WTR_PULSE_MASK                                            0x0000FF00
#define    LCDC_WTR_SETUP_SHIFT                                           0
#define    LCDC_WTR_SETUP_MASK                                            0x000000FF

#define LCDC_RTR_OFFSET                                                   0x00000014
#define LCDC_RTR_TYPE                                                     UInt32
#define LCDC_RTR_RESERVED_MASK                                            0xFF000000
#define    LCDC_RTR_HOLD_SHIFT                                            16
#define    LCDC_RTR_HOLD_MASK                                             0x00FF0000
#define    LCDC_RTR_PULSE_SHIFT                                           8
#define    LCDC_RTR_PULSE_MASK                                            0x0000FF00
#define    LCDC_RTR_SETUP_SHIFT                                           0
#define    LCDC_RTR_SETUP_MASK                                            0x000000FF

#define LCDC_CR_OFFSET                                                    0x00000018
#define LCDC_CR_TYPE                                                      UInt32
#define LCDC_CR_RESERVED_MASK                                             0x0100003F
#define    LCDC_CR_DMA_SHIFT                                              31
#define    LCDC_CR_DMA_MASK                                               0x80000000
#define    LCDC_CR_EIGHT_BIT_SHIFT                                        30
#define    LCDC_CR_EIGHT_BIT_MASK                                         0x40000000
#define    LCDC_CR_CE_SHIFT                                               29
#define    LCDC_CR_CE_MASK                                                0x20000000
#define    LCDC_CR_BYTESWAP_SHIFT                                         28
#define    LCDC_CR_BYTESWAP_MASK                                          0x10000000
#define    LCDC_CR_WORDSWAP_SHIFT                                         27
#define    LCDC_CR_WORDSWAP_MASK                                          0x08000000
#define    LCDC_CR_SEL_LCD_SHIFT                                          26
#define    LCDC_CR_SEL_LCD_MASK                                           0x04000000
#define    LCDC_CR_VIDEO_CODEC_MODE_SHIFT                                 25
#define    LCDC_CR_VIDEO_CODEC_MODE_MASK                                  0x02000000
#define    LCDC_CR_DBI_B_SHIFT                                            23
#define    LCDC_CR_DBI_B_MASK                                             0x00800000
#define       LCDC_CR_DBI_B_CMD_DBI_MODE                                  0x00000001
#define       LCDC_CR_DBI_B_CMD_Z80_M68_MODE                              0x00000000
#define    LCDC_CR_COLOR_ENDIAN_SHIFT                                     22
#define    LCDC_CR_COLOR_ENDIAN_MASK                                      0x00400000
#define    LCDC_CR_INPUT_COLOR_MODE_SHIFT                                 20
#define    LCDC_CR_INPUT_COLOR_MODE_MASK                                  0x00300000
#define    LCDC_CR_OUTPUT_COLOR_MODE_SHIFT                                17
#define    LCDC_CR_OUTPUT_COLOR_MODE_MASK                                 0x000E0000
#define    LCDC_CR_THCLK_CNT_SHIFT                                        15
#define    LCDC_CR_THCLK_CNT_MASK                                         0x00018000
#define    LCDC_CR_DBI_C_SHIFT                                            14
#define    LCDC_CR_DBI_C_MASK                                             0x00004000
#define    LCDC_CR_DBI_C_TYPE_SHIFT                                       13
#define    LCDC_CR_DBI_C_TYPE_MASK                                        0x00002000
#define    LCDC_CR_NEW_FRAME_SHIFT                                        12
#define    LCDC_CR_NEW_FRAME_MASK                                         0x00001000
#define    LCDC_CR_EDGE_SEL_SHIFT                                         11
#define    LCDC_CR_EDGE_SEL_MASK                                          0x00000800
#define    LCDC_CR_TEVALID_SHIFT                                          10
#define    LCDC_CR_TEVALID_MASK                                           0x00000400
#define    LCDC_CR_DBIC_SELECT_SHIFT                                      9
#define    LCDC_CR_DBIC_SELECT_MASK                                       0x00000200
#define    LCDC_CR_PACKED_RGB888_SHIFT                                    8
#define    LCDC_CR_PACKED_RGB888_MASK                                     0x00000100
#define    LCDC_CR_UNPACKED_RGB888_SHIFT                                  7
#define    LCDC_CR_UNPACKED_RGB888_MASK                                   0x00000080
#define    LCDC_CR_FAST_LEGACY_SHIFT                                      6
#define    LCDC_CR_FAST_LEGACY_MASK                                       0x00000040

#define LCDC_STATUS_OFFSET                                                0x0000001C
#define LCDC_STATUS_TYPE                                                  UInt32
#define LCDC_STATUS_RESERVED_MASK                                         0x000007FF
#define    LCDC_STATUS_FFEMPTY_SHIFT                                      31
#define    LCDC_STATUS_FFEMPTY_MASK                                       0x80000000
#define    LCDC_STATUS_FFFULL_SHIFT                                       30
#define    LCDC_STATUS_FFFULL_MASK                                        0x40000000
#define    LCDC_STATUS_FFHALF_SHIFT                                       29
#define    LCDC_STATUS_FFHALF_MASK                                        0x20000000
#define    LCDC_STATUS_FFRP_SHIFT                                         21
#define    LCDC_STATUS_FFRP_MASK                                          0x1FE00000
#define    LCDC_STATUS_FFWP_SHIFT                                         15
#define    LCDC_STATUS_FFWP_MASK                                          0x001F8000
#define    LCDC_STATUS_RREADY_SHIFT                                       14
#define    LCDC_STATUS_RREADY_MASK                                        0x00004000
#define    LCDC_STATUS_LCD_BUSY_SHIFT                                     13
#define    LCDC_STATUS_LCD_BUSY_MASK                                      0x00002000
#define    LCDC_STATUS_RREQ_SHIFT                                         12
#define    LCDC_STATUS_RREQ_MASK                                          0x00001000
#define    LCDC_STATUS_TE_SHIFT                                           11
#define    LCDC_STATUS_TE_MASK                                            0x00000800

#define LCDC_DBIBTR_OFFSET                                                0x00000020
#define LCDC_DBIBTR_TYPE                                                  UInt32
#define LCDC_DBIBTR_RESERVED_MASK                                         0xFF000000
#define    LCDC_DBIBTR_DCX_HIGH_SHIFT                                     22
#define    LCDC_DBIBTR_DCX_HIGH_MASK                                      0x00C00000
#define    LCDC_DBIBTR_DCX_LOW_SHIFT                                      20
#define    LCDC_DBIBTR_DCX_LOW_MASK                                       0x00300000
#define    LCDC_DBIBTR_READ_HIGH_SHIFT                                    14
#define    LCDC_DBIBTR_READ_HIGH_MASK                                     0x000FC000
#define    LCDC_DBIBTR_READ_LOW_SHIFT                                     10
#define    LCDC_DBIBTR_READ_LOW_MASK                                      0x00003C00
#define    LCDC_DBIBTR_WRITE_HIGH_SHIFT                                   4
#define    LCDC_DBIBTR_WRITE_HIGH_MASK                                    0x000003F0
#define    LCDC_DBIBTR_WRITE_LOW_SHIFT                                    0
#define    LCDC_DBIBTR_WRITE_LOW_MASK                                     0x0000000F

#define LCDC_DBICTR_OFFSET                                                0x00000024
#define LCDC_DBICTR_TYPE                                                  UInt32
#define LCDC_DBICTR_RESERVED_MASK                                         0xFFFF0000
#define    LCDC_DBICTR_DCX_HIGH_SHIFT                                     14
#define    LCDC_DBICTR_DCX_HIGH_MASK                                      0x0000C000
#define    LCDC_DBICTR_DCX_LOW_SHIFT                                      12
#define    LCDC_DBICTR_DCX_LOW_MASK                                       0x00003000
#define    LCDC_DBICTR_READ_HIGH_SHIFT                                    9
#define    LCDC_DBICTR_READ_HIGH_MASK                                     0x00000E00
#define    LCDC_DBICTR_READ_LOW_SHIFT                                     6
#define    LCDC_DBICTR_READ_LOW_MASK                                      0x000001C0
#define    LCDC_DBICTR_WRITE_HIGH_SHIFT                                   3
#define    LCDC_DBICTR_WRITE_HIGH_MASK                                    0x00000038
#define    LCDC_DBICTR_WRITE_LOW_SHIFT                                    0
#define    LCDC_DBICTR_WRITE_LOW_MASK                                     0x00000007

#define LCDC_TEDELAY_OFFSET                                               0x00000028
#define LCDC_TEDELAY_TYPE                                                 UInt32
#define LCDC_TEDELAY_RESERVED_MASK                                        0xFFFE0000
#define    LCDC_TEDELAY_DELAY_COUNT_SHIFT                                 0
#define    LCDC_TEDELAY_DELAY_COUNT_MASK                                  0x0001FFFF

#endif /* __BRCM_RDB_LCDC_H__ */


