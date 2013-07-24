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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_ISP2_H__
#define __BRCM_RDB_ISP2_H__

#define ISP2_CTRL_OFFSET                                                  0x00000000
#define ISP2_CTRL_TYPE                                                    UInt32
#define ISP2_CTRL_RESERVED_MASK                                           0xFFEEFE80
#define    ISP2_CTRL_FORCE_CLKEN_SHIFT                                    20
#define    ISP2_CTRL_FORCE_CLKEN_MASK                                     0x00100000
#define    ISP2_CTRL_TRANSFER_SHIFT                                       16
#define    ISP2_CTRL_TRANSFER_MASK                                        0x00010000
#define    ISP2_CTRL_STATS_IMASK_SHIFT                                    8
#define    ISP2_CTRL_STATS_IMASK_MASK                                     0x00000100
#define    ISP2_CTRL_SW_IMASK_SHIFT                                       6
#define    ISP2_CTRL_SW_IMASK_MASK                                        0x00000040
#define    ISP2_CTRL_EOD_IMASK_SHIFT                                      5
#define    ISP2_CTRL_EOD_IMASK_MASK                                       0x00000020
#define    ISP2_CTRL_EOT_IMASK_SHIFT                                      4
#define    ISP2_CTRL_EOT_IMASK_MASK                                       0x00000010
#define    ISP2_CTRL_ERROR_IMASK_SHIFT                                    3
#define    ISP2_CTRL_ERROR_IMASK_MASK                                     0x00000008
#define    ISP2_CTRL_CLR_STATS_SHIFT                                      2
#define    ISP2_CTRL_CLR_STATS_MASK                                       0x00000004
#define    ISP2_CTRL_FLUSH_SHIFT                                          1
#define    ISP2_CTRL_FLUSH_MASK                                           0x00000002
#define    ISP2_CTRL_ENABLE_SHIFT                                         0
#define    ISP2_CTRL_ENABLE_MASK                                          0x00000001

#define ISP2_STATUS_OFFSET                                                0x00000004
#define ISP2_STATUS_TYPE                                                  UInt32
#define ISP2_STATUS_RESERVED_MASK                                         0xFFFFFE84
#define    ISP2_STATUS_STATS_INT_SHIFT                                    8
#define    ISP2_STATUS_STATS_INT_MASK                                     0x00000100
#define    ISP2_STATUS_SW_INT_SHIFT                                       6
#define    ISP2_STATUS_SW_INT_MASK                                        0x00000040
#define    ISP2_STATUS_EOD_INT_SHIFT                                      5
#define    ISP2_STATUS_EOD_INT_MASK                                       0x00000020
#define    ISP2_STATUS_EOT_INT_SHIFT                                      4
#define    ISP2_STATUS_EOT_INT_MASK                                       0x00000010
#define    ISP2_STATUS_ERROR_INT_SHIFT                                    3
#define    ISP2_STATUS_ERROR_INT_MASK                                     0x00000008
#define    ISP2_STATUS_STATE_SHIFT                                        0
#define    ISP2_STATUS_STATE_MASK                                         0x00000003

#define ISP2_ID_OFFSET                                                    0x00000008
#define ISP2_ID_TYPE                                                      UInt32
#define ISP2_ID_RESERVED_MASK                                             0x00000000
#define    ISP2_ID_SIGNATURE_SHIFT                                        16
#define    ISP2_ID_SIGNATURE_MASK                                         0xFFFF0000
#define    ISP2_ID_VERSION_SHIFT                                          8
#define    ISP2_ID_VERSION_MASK                                           0x0000FF00
#define    ISP2_ID_REVISION_SHIFT                                         0
#define    ISP2_ID_REVISION_MASK                                          0x000000FF

#define ISP2_TILE_CTRL_OFFSET                                             0x00000010
#define ISP2_TILE_CTRL_TYPE                                               UInt32
#define ISP2_TILE_CTRL_RESERVED_MASK                                      0xFFFFF000
#define    ISP2_TILE_CTRL_TILE_COUNT_SHIFT                                0
#define    ISP2_TILE_CTRL_TILE_COUNT_MASK                                 0x00000FFF

#define ISP2_TILE_STATUS_OFFSET                                           0x00000018
#define ISP2_TILE_STATUS_TYPE                                             UInt32
#define ISP2_TILE_STATUS_RESERVED_MASK                                    0xFFFFF000
#define    ISP2_TILE_STATUS_INT_COUNT_SHIFT                               0
#define    ISP2_TILE_STATUS_INT_COUNT_MASK                                0x00000FFF

#define ISP2_TILE_ADDR_OFFSET                                             0x0000001C
#define ISP2_TILE_ADDR_TYPE                                               UInt32
#define ISP2_TILE_ADDR_RESERVED_MASK                                      0x0000001F
#define    ISP2_TILE_ADDR_ADDRESS_SHIFT                                   5
#define    ISP2_TILE_ADDR_ADDRESS_MASK                                    0xFFFFFFE0

#define ISP2_FR_SIZE_OFFSET                                               0x00000030
#define ISP2_FR_SIZE_TYPE                                                 UInt32
#define ISP2_FR_SIZE_RESERVED_MASK                                        0x00000000
#define    ISP2_FR_SIZE_HEIGHT_SHIFT                                      16
#define    ISP2_FR_SIZE_HEIGHT_MASK                                       0xFFFF0000
#define    ISP2_FR_SIZE_ISPFR_WIDTH_SHIFT                                 0
#define    ISP2_FR_SIZE_ISPFR_WIDTH_MASK                                  0x0000FFFF

#define ISP2_FR_CTRL_OFFSET                                               0x00000034
#define ISP2_FR_CTRL_TYPE                                                 UInt32
#define ISP2_FR_CTRL_RESERVED_MASK                                        0xFFFF00FB
#define    ISP2_FR_CTRL_SW_IN_POS_SHIFT                                   12
#define    ISP2_FR_CTRL_SW_IN_POS_MASK                                    0x0000F000
#define    ISP2_FR_CTRL_SW_OUT_POS_SHIFT                                  8
#define    ISP2_FR_CTRL_SW_OUT_POS_MASK                                   0x00000F00
#define    ISP2_FR_CTRL_TRANSPOSED_SHIFT                                  2
#define    ISP2_FR_CTRL_TRANSPOSED_MASK                                   0x00000004

#define ISP2_FR_BAYER_EN_OFFSET                                           0x00000038
#define ISP2_FR_BAYER_EN_TYPE                                             UInt32
#define ISP2_FR_BAYER_EN_RESERVED_MASK                                    0xFFFFE000
#define    ISP2_FR_BAYER_EN_SW_SHIFT                                      12
#define    ISP2_FR_BAYER_EN_SW_MASK                                       0x00001000
#define    ISP2_FR_BAYER_EN_DM_SHIFT                                      11
#define    ISP2_FR_BAYER_EN_DM_MASK                                       0x00000800
#define    ISP2_FR_BAYER_EN_BD_SHIFT                                      10
#define    ISP2_FR_BAYER_EN_BD_MASK                                       0x00000400
#define    ISP2_FR_BAYER_EN_XC_SHIFT                                      9
#define    ISP2_FR_BAYER_EN_XC_MASK                                       0x00000200
#define    ISP2_FR_BAYER_EN_RS_SHIFT                                      8
#define    ISP2_FR_BAYER_EN_RS_MASK                                       0x00000100
#define    ISP2_FR_BAYER_EN_DPA_SHIFT                                     7
#define    ISP2_FR_BAYER_EN_DPA_MASK                                      0x00000080
#define    ISP2_FR_BAYER_EN_DPP_SHIFT                                     6
#define    ISP2_FR_BAYER_EN_DPP_MASK                                      0x00000040
#define    ISP2_FR_BAYER_EN_WG_SHIFT                                      5
#define    ISP2_FR_BAYER_EN_WG_MASK                                       0x00000020
#define    ISP2_FR_BAYER_EN_ST_SHIFT                                      4
#define    ISP2_FR_BAYER_EN_ST_MASK                                       0x00000010
#define    ISP2_FR_BAYER_EN_LS_SHIFT                                      3
#define    ISP2_FR_BAYER_EN_LS_MASK                                       0x00000008
#define    ISP2_FR_BAYER_EN_BL_SHIFT                                      2
#define    ISP2_FR_BAYER_EN_BL_MASK                                       0x00000004
#define    ISP2_FR_BAYER_EN_DI_SHIFT                                      1
#define    ISP2_FR_BAYER_EN_DI_MASK                                       0x00000002
#define    ISP2_FR_BAYER_EN_II_SHIFT                                      0
#define    ISP2_FR_BAYER_EN_II_MASK                                       0x00000001

#define ISP2_FR_YCBCR_EN_OFFSET                                           0x0000003C
#define ISP2_FR_YCBCR_EN_TYPE                                             UInt32
#define ISP2_FR_YCBCR_EN_RESERVED_MASK                                    0xFFFFC010
#define    ISP2_FR_YCBCR_EN_LO_SHIFT                                      13
#define    ISP2_FR_YCBCR_EN_LO_MASK                                       0x00002000
#define    ISP2_FR_YCBCR_EN_HO_SHIFT                                      12
#define    ISP2_FR_YCBCR_EN_HO_MASK                                       0x00001000
#define    ISP2_FR_YCBCR_EN_LR_SHIFT                                      11
#define    ISP2_FR_YCBCR_EN_LR_MASK                                       0x00000800
#define    ISP2_FR_YCBCR_EN_CC_SHIFT                                      10
#define    ISP2_FR_YCBCR_EN_CC_MASK                                       0x00000400
#define    ISP2_FR_YCBCR_EN_HR_SHIFT                                      9
#define    ISP2_FR_YCBCR_EN_HR_MASK                                       0x00000200
#define    ISP2_FR_YCBCR_EN_CP_SHIFT                                      8
#define    ISP2_FR_YCBCR_EN_CP_MASK                                       0x00000100
#define    ISP2_FR_YCBCR_EN_GD_SHIFT                                      7
#define    ISP2_FR_YCBCR_EN_GD_MASK                                       0x00000080
#define    ISP2_FR_YCBCR_EN_SH_SHIFT                                      6
#define    ISP2_FR_YCBCR_EN_SH_MASK                                       0x00000040
#define    ISP2_FR_YCBCR_EN_FC_SHIFT                                      5
#define    ISP2_FR_YCBCR_EN_FC_MASK                                       0x00000020
#define    ISP2_FR_YCBCR_EN_YG_SHIFT                                      3
#define    ISP2_FR_YCBCR_EN_YG_MASK                                       0x00000008
#define    ISP2_FR_YCBCR_EN_GM_SHIFT                                      2
#define    ISP2_FR_YCBCR_EN_GM_MASK                                       0x00000004
#define    ISP2_FR_YCBCR_EN_YC_SHIFT                                      1
#define    ISP2_FR_YCBCR_EN_YC_MASK                                       0x00000002
#define    ISP2_FR_YCBCR_EN_YI_SHIFT                                      0
#define    ISP2_FR_YCBCR_EN_YI_MASK                                       0x00000001

#define ISP2_FR_MOSAIC_OFFSET                                             0x00000050
#define ISP2_FR_MOSAIC_TYPE                                               UInt32
#define ISP2_FR_MOSAIC_RESERVED_MASK                                      0xFFFFFFFF

#define ISP2_WG_RED_OFFSET                                                0x00000090
#define ISP2_WG_RED_TYPE                                                  UInt32
#define ISP2_WG_RED_RESERVED_MASK                                         0x00000000
#define    ISP2_WG_RED_OFFSET_SHIFT                                       16
#define    ISP2_WG_RED_OFFSET_MASK                                        0xFFFF0000
#define    ISP2_WG_RED_GAIN_SHIFT                                         0
#define    ISP2_WG_RED_GAIN_MASK                                          0x0000FFFF

#define ISP2_WG_BLUE_OFFSET                                               0x00000094
#define ISP2_WG_BLUE_TYPE                                                 UInt32
#define ISP2_WG_BLUE_RESERVED_MASK                                        0x00000000
#define    ISP2_WG_BLUE_OFFSET_SHIFT                                      16
#define    ISP2_WG_BLUE_OFFSET_MASK                                       0xFFFF0000
#define    ISP2_WG_BLUE_GAIN_SHIFT                                        0
#define    ISP2_WG_BLUE_GAIN_MASK                                         0x0000FFFF

#define ISP2_WG_OFFSETG_OFFSET                                            0x00000098
#define ISP2_WG_OFFSETG_TYPE                                              UInt32
#define ISP2_WG_OFFSETG_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_WG_OFFSETG_OFFSET_SHIFT                                   0
#define    ISP2_WG_OFFSETG_OFFSET_MASK                                    0x0001FFFF

#define ISP2_WG_GAIN_OFFSET                                               0x0000009C
#define ISP2_WG_GAIN_TYPE                                                 UInt32
#define ISP2_WG_GAIN_RESERVED_MASK                                        0xFFFF0000
#define    ISP2_WG_GAIN_GAIN_SHIFT                                        0
#define    ISP2_WG_GAIN_GAIN_MASK                                         0x0000FFFF

#define ISP2_WG_THRESH_OFFSET                                             0x000000A0
#define ISP2_WG_THRESH_TYPE                                               UInt32
#define ISP2_WG_THRESH_RESERVED_MASK                                      0xFFFFC000
#define    ISP2_WG_THRESH_THRESH_SHIFT                                    0
#define    ISP2_WG_THRESH_THRESH_MASK                                     0x00003FFF

#define ISP2_WG_OFFSETR_OFFSET                                            0x000000A4
#define ISP2_WG_OFFSETR_TYPE                                              UInt32
#define ISP2_WG_OFFSETR_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_WG_OFFSETR_OFFSET_SHIFT                                   0
#define    ISP2_WG_OFFSETR_OFFSET_MASK                                    0x0001FFFF

#define ISP2_WG_OFFSETB_OFFSET                                            0x000000A8
#define ISP2_WG_OFFSETB_TYPE                                              UInt32
#define ISP2_WG_OFFSETB_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_WG_OFFSETB_OFFSET_SHIFT                                   0
#define    ISP2_WG_OFFSETB_OFFSET_MASK                                    0x0001FFFF

#define ISP2_YG_Y_OFFSET                                                  0x000000D0
#define ISP2_YG_Y_TYPE                                                    UInt32
#define ISP2_YG_Y_RESERVED_MASK                                           0x00000000
#define    ISP2_YG_Y_COEFF1_SHIFT                                         16
#define    ISP2_YG_Y_COEFF1_MASK                                          0xFFFF0000
#define    ISP2_YG_Y_COEFF0_SHIFT                                         0
#define    ISP2_YG_Y_COEFF0_MASK                                          0x0000FFFF

#define ISP2_II_CTRL_OFFSET                                               0x00000100
#define ISP2_II_CTRL_TYPE                                                 UInt32
#define ISP2_II_CTRL_RESERVED_MASK                                        0x000060FF
#define    ISP2_II_CTRL_STRIDE_SHIFT                                      15
#define    ISP2_II_CTRL_STRIDE_MASK                                       0xFFFF8000
#define    ISP2_II_CTRL_FORMAT_SHIFT                                      8
#define    ISP2_II_CTRL_FORMAT_MASK                                       0x00001F00

#define ISP2_II_ADDRESS_OFFSET                                            0x00000104
#define ISP2_II_ADDRESS_TYPE                                              UInt32
#define ISP2_II_ADDRESS_RESERVED_MASK                                     0x0000001F
#define    ISP2_II_ADDRESS_ADDRESS_SHIFT                                  5
#define    ISP2_II_ADDRESS_ADDRESS_MASK                                   0xFFFFFFE0

#define ISP2_II_ENDADDR_OFFSET                                            0x00000108
#define ISP2_II_ENDADDR_TYPE                                              UInt32
#define ISP2_II_ENDADDR_RESERVED_MASK                                     0x0000001F
#define    ISP2_II_ENDADDR_ENDADDR_SHIFT                                  5
#define    ISP2_II_ENDADDR_ENDADDR_MASK                                   0xFFFFFFE0

#define ISP2_II_DPCM_OFFSET                                               0x0000010C
#define ISP2_II_DPCM_TYPE                                                 UInt32
#define ISP2_II_DPCM_RESERVED_MASK                                        0xFFFFE000
#define    ISP2_II_DPCM_MODE_SHIFT                                        9
#define    ISP2_II_DPCM_MODE_MASK                                         0x00001E00
#define    ISP2_II_DPCM_BLOCKLEN_SHIFT                                    0
#define    ISP2_II_DPCM_BLOCKLEN_MASK                                     0x000001FF

#define ISP2_II_FIFO_OFFSET                                               0x00000110
#define ISP2_II_FIFO_TYPE                                                 UInt32
#define ISP2_II_FIFO_RESERVED_MASK                                        0xFFFFC0C0
#define    ISP2_II_FIFO_MIN3_SHIFT                                        8
#define    ISP2_II_FIFO_MIN3_MASK                                         0x00003F00
#define    ISP2_II_FIFO_MIN2_SHIFT                                        0
#define    ISP2_II_FIFO_MIN2_MASK                                         0x0000003F

#define ISP2_DI_ADDRESS_OFFSET                                            0x00000130
#define ISP2_DI_ADDRESS_TYPE                                              UInt32
#define ISP2_DI_ADDRESS_RESERVED_MASK                                     0x0000001F
#define    ISP2_DI_ADDRESS_ADDRESS_SHIFT                                  5
#define    ISP2_DI_ADDRESS_ADDRESS_MASK                                   0xFFFFFFE0

#define ISP2_DI_ENDADDR_OFFSET                                            0x00000134
#define ISP2_DI_ENDADDR_TYPE                                              UInt32
#define ISP2_DI_ENDADDR_RESERVED_MASK                                     0x0000001F
#define    ISP2_DI_ENDADDR_ENDADDR_SHIFT                                  5
#define    ISP2_DI_ENDADDR_ENDADDR_MASK                                   0xFFFFFFE0

#define ISP2_BL_ABSC_R_OFFSET                                             0x00000160
#define ISP2_BL_ABSC_R_TYPE                                               UInt32
#define ISP2_BL_ABSC_R_RESERVED_MASK                                      0x00000000
#define    ISP2_BL_ABSC_R_ABSCISSA1_SHIFT                                 16
#define    ISP2_BL_ABSC_R_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_BL_ABSC_R_ABSCISSA0_SHIFT                                 0
#define    ISP2_BL_ABSC_R_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_BL_ABSC_GR_OFFSET                                            0x00000180
#define ISP2_BL_ABSC_GR_TYPE                                              UInt32
#define ISP2_BL_ABSC_GR_RESERVED_MASK                                     0x00000000
#define    ISP2_BL_ABSC_GR_ABSCISSA1_SHIFT                                16
#define    ISP2_BL_ABSC_GR_ABSCISSA1_MASK                                 0xFFFF0000
#define    ISP2_BL_ABSC_GR_ABSCISSA0_SHIFT                                0
#define    ISP2_BL_ABSC_GR_ABSCISSA0_MASK                                 0x0000FFFF

#define ISP2_BL_ABSC_B_OFFSET                                             0x000001A0
#define ISP2_BL_ABSC_B_TYPE                                               UInt32
#define ISP2_BL_ABSC_B_RESERVED_MASK                                      0x00000000
#define    ISP2_BL_ABSC_B_ABSCISSA1_SHIFT                                 16
#define    ISP2_BL_ABSC_B_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_BL_ABSC_B_ABSCISSA0_SHIFT                                 0
#define    ISP2_BL_ABSC_B_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_BL_ORD_SLOPE_R_OFFSET                                        0x000001C0
#define ISP2_BL_ORD_SLOPE_R_TYPE                                          UInt32
#define ISP2_BL_ORD_SLOPE_R_RESERVED_MASK                                 0x00000000
#define    ISP2_BL_ORD_SLOPE_R_ORDINATE_SHIFT                             16
#define    ISP2_BL_ORD_SLOPE_R_ORDINATE_MASK                              0xFFFF0000
#define    ISP2_BL_ORD_SLOPE_R_SLOPE_SHIFT                                0
#define    ISP2_BL_ORD_SLOPE_R_SLOPE_MASK                                 0x0000FFFF

#define ISP2_BL_ORD_SLOPE_GR_OFFSET                                       0x00000200
#define ISP2_BL_ORD_SLOPE_GR_TYPE                                         UInt32
#define ISP2_BL_ORD_SLOPE_GR_RESERVED_MASK                                0x00000000
#define    ISP2_BL_ORD_SLOPE_GR_ORDINATE_SHIFT                            16
#define    ISP2_BL_ORD_SLOPE_GR_ORDINATE_MASK                             0xFFFF0000
#define    ISP2_BL_ORD_SLOPE_GR_SLOPE_SHIFT                               0
#define    ISP2_BL_ORD_SLOPE_GR_SLOPE_MASK                                0x0000FFFF

#define ISP2_BL_ORD_SLOPE_B_OFFSET                                        0x00000240
#define ISP2_BL_ORD_SLOPE_B_TYPE                                          UInt32
#define ISP2_BL_ORD_SLOPE_B_RESERVED_MASK                                 0x00000000
#define    ISP2_BL_ORD_SLOPE_B_ORDINATE_SHIFT                             16
#define    ISP2_BL_ORD_SLOPE_B_ORDINATE_MASK                              0xFFFF0000
#define    ISP2_BL_ORD_SLOPE_B_SLOPE_SHIFT                                0
#define    ISP2_BL_ORD_SLOPE_B_SLOPE_MASK                                 0x0000FFFF

#define ISP2_BL_TB_OFFSET                                                 0x00000280
#define ISP2_BL_TB_TYPE                                                   UInt32
#define ISP2_BL_TB_RESERVED_MASK                                          0x0000FF00
#define    ISP2_BL_TB_BOTTOM_SHIFT                                        16
#define    ISP2_BL_TB_BOTTOM_MASK                                         0xFFFF0000
#define    ISP2_BL_TB_TOP_SHIFT                                           0
#define    ISP2_BL_TB_TOP_MASK                                            0x000000FF

#define ISP2_BL_LR_OFFSET                                                 0x00000284
#define ISP2_BL_LR_TYPE                                                   UInt32
#define ISP2_BL_LR_RESERVED_MASK                                          0x0000FF00
#define    ISP2_BL_LR_RIGHT_SHIFT                                         16
#define    ISP2_BL_LR_RIGHT_MASK                                          0xFFFF0000
#define    ISP2_BL_LR_LEFT_SHIFT                                          0
#define    ISP2_BL_LR_LEFT_MASK                                           0x000000FF

#define ISP2_BL_MT_OFFSET                                                 0x00000288
#define ISP2_BL_MT_TYPE                                                   UInt32
#define ISP2_BL_MT_RESERVED_MASK                                          0x00000000
#define    ISP2_BL_MT_THRESHOLD_SHIFT                                     16
#define    ISP2_BL_MT_THRESHOLD_MASK                                      0xFFFF0000
#define    ISP2_BL_MT_MULTIPLIER_SHIFT                                    0
#define    ISP2_BL_MT_MULTIPLIER_MASK                                     0x0000FFFF

#define ISP2_BL_SHIFT_OFFSET                                              0x0000028C
#define ISP2_BL_SHIFT_TYPE                                                UInt32
#define ISP2_BL_SHIFT_RESERVED_MASK                                       0xFFFEFEFE
#define    ISP2_BL_SHIFT_SHIFT_B_SHIFT                                    16
#define    ISP2_BL_SHIFT_SHIFT_B_MASK                                     0x00010000
#define    ISP2_BL_SHIFT_SHIFT_G_SHIFT                                    8
#define    ISP2_BL_SHIFT_SHIFT_G_MASK                                     0x00000100
#define    ISP2_BL_SHIFT_SHIFT_R_SHIFT                                    0
#define    ISP2_BL_SHIFT_SHIFT_R_MASK                                     0x00000001

#define ISP2_BL_SUM_LO_OFFSET                                             0x00000290
#define ISP2_BL_SUM_LO_TYPE                                               UInt32
#define ISP2_BL_SUM_LO_RESERVED_MASK                                      0x00000000
#define    ISP2_BL_SUM_LO_SUM_LO_SHIFT                                    0
#define    ISP2_BL_SUM_LO_SUM_LO_MASK                                     0xFFFFFFFF

#define ISP2_BL_SUM_HI_OFFSET                                             0x00000294
#define ISP2_BL_SUM_HI_TYPE                                               UInt32
#define ISP2_BL_SUM_HI_RESERVED_MASK                                      0xFFFF0000
#define    ISP2_BL_SUM_HI_SUM_HI_SHIFT                                    0
#define    ISP2_BL_SUM_HI_SUM_HI_MASK                                     0x0000FFFF

#define ISP2_BL_COUNT_OFFSET                                              0x00000298
#define ISP2_BL_COUNT_TYPE                                                UInt32
#define ISP2_BL_COUNT_RESERVED_MASK                                       0x00000000
#define    ISP2_BL_COUNT_COUNT_SHIFT                                      0
#define    ISP2_BL_COUNT_COUNT_MASK                                       0xFFFFFFFF

#define ISP2_DP_HI_OFFSET                                                 0x000002D0
#define ISP2_DP_HI_TYPE                                                   UInt32
#define ISP2_DP_HI_RESERVED_MASK                                          0x0000C0FF
#define    ISP2_DP_HI_OFFSET_SHIFT                                        16
#define    ISP2_DP_HI_OFFSET_MASK                                         0xFFFF0000
#define    ISP2_DP_HI_SCALE_SHIFT                                         8
#define    ISP2_DP_HI_SCALE_MASK                                          0x00003F00

#define ISP2_DP_LO_OFFSET                                                 0x000002DC
#define ISP2_DP_LO_TYPE                                                   UInt32
#define ISP2_DP_LO_RESERVED_MASK                                          0x0000C0FF
#define    ISP2_DP_LO_OFFSET_SHIFT                                        16
#define    ISP2_DP_LO_OFFSET_MASK                                         0xFFFF0000
#define    ISP2_DP_LO_SCALE_SHIFT                                         8
#define    ISP2_DP_LO_SCALE_MASK                                          0x00003F00

#define ISP2_DP_REPLO_OFFSET                                              0x000002E8
#define ISP2_DP_REPLO_TYPE                                                UInt32
#define ISP2_DP_REPLO_RESERVED_MASK                                       0xFFF0F00F
#define    ISP2_DP_REPLO_INDEX_SHIFT                                      16
#define    ISP2_DP_REPLO_INDEX_MASK                                       0x000F0000
#define    ISP2_DP_REPLO_SCALE_SHIFT                                      4
#define    ISP2_DP_REPLO_SCALE_MASK                                       0x00000FF0

#define ISP2_DP_REPHI_OFFSET                                              0x000002EC
#define ISP2_DP_REPHI_TYPE                                                UInt32
#define ISP2_DP_REPHI_RESERVED_MASK                                       0xFFF0F00F
#define    ISP2_DP_REPHI_INDEX_SHIFT                                      16
#define    ISP2_DP_REPHI_INDEX_MASK                                       0x000F0000
#define    ISP2_DP_REPHI_SCALE_SHIFT                                      4
#define    ISP2_DP_REPHI_SCALE_MASK                                       0x00000FF0

#define ISP2_BL_ABSC_GB_OFFSET                                            0x00000300
#define ISP2_BL_ABSC_GB_TYPE                                              UInt32
#define ISP2_BL_ABSC_GB_RESERVED_MASK                                     0x00000000
#define    ISP2_BL_ABSC_GB_ABSCISSA1_SHIFT                                16
#define    ISP2_BL_ABSC_GB_ABSCISSA1_MASK                                 0xFFFF0000
#define    ISP2_BL_ABSC_GB_ABSCISSA0_SHIFT                                0
#define    ISP2_BL_ABSC_GB_ABSCISSA0_MASK                                 0x0000FFFF

#define ISP2_BL_ORD_SLOPE_GB_OFFSET                                       0x00000340
#define ISP2_BL_ORD_SLOPE_GB_TYPE                                         UInt32
#define ISP2_BL_ORD_SLOPE_GB_RESERVED_MASK                                0x00000000
#define    ISP2_BL_ORD_SLOPE_GB_ORDINATE_SHIFT                            16
#define    ISP2_BL_ORD_SLOPE_GB_ORDINATE_MASK                             0xFFFF0000
#define    ISP2_BL_ORD_SLOPE_GB_SLOPE_SHIFT                               0
#define    ISP2_BL_ORD_SLOPE_GB_SLOPE_MASK                                0x0000FFFF

#define ISP2_TD_CTRL_OFFSET                                               0x00000440
#define ISP2_TD_CTRL_TYPE                                                 UInt32
#define ISP2_TD_CTRL_RESERVED_MASK                                        0xFFFFF000
#define    ISP2_TD_CTRL_DESC_COUNT_SHIFT                                  0
#define    ISP2_TD_CTRL_DESC_COUNT_MASK                                   0x00000FFF

#define ISP2_TD_STATUS_OFFSET                                             0x00000444
#define ISP2_TD_STATUS_TYPE                                               UInt32
#define ISP2_TD_STATUS_RESERVED_MASK                                      0xFFFFF000
#define    ISP2_TD_STATUS_INT_COUNT_SHIFT                                 0
#define    ISP2_TD_STATUS_INT_COUNT_MASK                                  0x00000FFF

#define ISP2_TD_ADDR_OFFSET                                               0x00000448
#define ISP2_TD_ADDR_TYPE                                                 UInt32
#define ISP2_TD_ADDR_RESERVED_MASK                                        0x0000001F
#define    ISP2_TD_ADDR_ADDRESS_SHIFT                                     5
#define    ISP2_TD_ADDR_ADDRESS_MASK                                      0xFFFFFFE0

#define ISP2_TD_DESC_OFFSET                                               0x0000044C
#define ISP2_TD_DESC_TYPE                                                 UInt256
#define ISP2_TD_DESC_RESERVED_MASK                                        0x00000000
#define    ISP2_TD_DESC_DESC_SHIFT                                        0
#define    ISP2_TD_DESC_DESC_MASK                                         0xFFFFFFFFFFFFFFFF

#define ISP2_TD_INIT0_OFFSET                                              0x0000046C
#define ISP2_TD_INIT0_TYPE                                                UInt32
#define ISP2_TD_INIT0_RESERVED_MASK                                       0x70007000
#define    ISP2_TD_INIT0_LS_SET_SHIFT                                     31
#define    ISP2_TD_INIT0_LS_SET_MASK                                      0x80000000
#define    ISP2_TD_INIT0_LS_COUNT_SHIFT                                   16
#define    ISP2_TD_INIT0_LS_COUNT_MASK                                    0x0FFF0000
#define    ISP2_TD_INIT0_SET_SHIFT                                        15
#define    ISP2_TD_INIT0_SET_MASK                                         0x00008000
#define    ISP2_TD_INIT0_COUNT_SHIFT                                      0
#define    ISP2_TD_INIT0_COUNT_MASK                                       0x00000FFF

#define ISP2_TD_INIT1_OFFSET                                              0x00000470
#define ISP2_TD_INIT1_TYPE                                                UInt32
#define ISP2_TD_INIT1_RESERVED_MASK                                       0x70007000
#define    ISP2_TD_INIT1_GD_SET_SHIFT                                     31
#define    ISP2_TD_INIT1_GD_SET_MASK                                      0x80000000
#define    ISP2_TD_INIT1_GD_COUNT_SHIFT                                   16
#define    ISP2_TD_INIT1_GD_COUNT_MASK                                    0x0FFF0000
#define    ISP2_TD_INIT1_ST_SET_SHIFT                                     15
#define    ISP2_TD_INIT1_ST_SET_MASK                                      0x00008000
#define    ISP2_TD_INIT1_ST_COUNT_SHIFT                                   0
#define    ISP2_TD_INIT1_ST_COUNT_MASK                                    0x00000FFF

#define ISP2_TD_INIT2_OFFSET                                              0x00000474
#define ISP2_TD_INIT2_TYPE                                                UInt32
#define ISP2_TD_INIT2_RESERVED_MASK                                       0xFFFF7000
#define    ISP2_TD_INIT2_PP_SET_SHIFT                                     15
#define    ISP2_TD_INIT2_PP_SET_MASK                                      0x00008000
#define    ISP2_TD_INIT2_PP_COUNT_SHIFT                                   0
#define    ISP2_TD_INIT2_PP_COUNT_MASK                                    0x00000FFF

#define ISP2_RS_CTRL_OFFSET                                               0x00000490
#define ISP2_RS_CTRL_TYPE                                                 UInt32
#define ISP2_RS_CTRL_RESERVED_MASK                                        0xFEF0FEF0
#define    ISP2_RS_CTRL_V_EVEN_SHIFT                                      24
#define    ISP2_RS_CTRL_V_EVEN_MASK                                       0x01000000
#define    ISP2_RS_CTRL_V_FACTOR_SHIFT                                    16
#define    ISP2_RS_CTRL_V_FACTOR_MASK                                     0x000F0000
#define    ISP2_RS_CTRL_H_EVEN_SHIFT                                      8
#define    ISP2_RS_CTRL_H_EVEN_MASK                                       0x00000100
#define    ISP2_RS_CTRL_H_FACTOR_SHIFT                                    0
#define    ISP2_RS_CTRL_H_FACTOR_MASK                                     0x0000000F

#define ISP2_LS_CTRL_OFFSET                                               0x000004D0
#define ISP2_LS_CTRL_TYPE                                                 UInt32
#define ISP2_LS_CTRL_RESERVED_MASK                                        0xFC00FF00
#define    ISP2_LS_CTRL_PITCH_SHIFT                                       16
#define    ISP2_LS_CTRL_PITCH_MASK                                        0x03FF0000
#define    ISP2_LS_CTRL_CELL_SZ_POW_Y_SHIFT                               4
#define    ISP2_LS_CTRL_CELL_SZ_POW_Y_MASK                                0x000000F0
#define    ISP2_LS_CTRL_CELL_SZ_POW_X_SHIFT                               0
#define    ISP2_LS_CTRL_CELL_SZ_POW_X_MASK                                0x0000000F

#define ISP2_LS_OFFSETS_OFFSET                                            0x000004D4
#define ISP2_LS_OFFSETS_TYPE                                              UInt32
#define ISP2_LS_OFFSETS_RESERVED_MASK                                     0x00000000
#define    ISP2_LS_OFFSETS_Y_OFFSET_SHIFT                                 16
#define    ISP2_LS_OFFSETS_Y_OFFSET_MASK                                  0xFFFF0000
#define    ISP2_LS_OFFSETS_X_OFFSET_SHIFT                                 0
#define    ISP2_LS_OFFSETS_X_OFFSET_MASK                                  0x0000FFFF

#define ISP2_XC_ABSC_LIM_OFFSET                                           0x00000500
#define ISP2_XC_ABSC_LIM_TYPE                                             UInt32
#define ISP2_XC_ABSC_LIM_RESERVED_MASK                                    0x00000000
#define    ISP2_XC_ABSC_LIM_ABSCISSA1_SHIFT                               16
#define    ISP2_XC_ABSC_LIM_ABSCISSA1_MASK                                0xFFFF0000
#define    ISP2_XC_ABSC_LIM_ABSCISSA0_SHIFT                               0
#define    ISP2_XC_ABSC_LIM_ABSCISSA0_MASK                                0x0000FFFF

#define ISP2_XC_ORD_LIM_OFFSET                                            0x00000510
#define ISP2_XC_ORD_LIM_TYPE                                              UInt32
#define ISP2_XC_ORD_LIM_RESERVED_MASK                                     0x00000000
#define    ISP2_XC_ORD_LIM_ORDINATE1_SHIFT                                16
#define    ISP2_XC_ORD_LIM_ORDINATE1_MASK                                 0xFFFF0000
#define    ISP2_XC_ORD_LIM_ORDINATE0_SHIFT                                0
#define    ISP2_XC_ORD_LIM_ORDINATE0_MASK                                 0x0000FFFF

#define ISP2_XC_SLOPE_LIM_OFFSET                                          0x00000520
#define ISP2_XC_SLOPE_LIM_TYPE                                            UInt32
#define ISP2_XC_SLOPE_LIM_RESERVED_MASK                                   0x00000000
#define    ISP2_XC_SLOPE_LIM_SLOPE1_SHIFT                                 16
#define    ISP2_XC_SLOPE_LIM_SLOPE1_MASK                                  0xFFFF0000
#define    ISP2_XC_SLOPE_LIM_SLOPE0_SHIFT                                 0
#define    ISP2_XC_SLOPE_LIM_SLOPE0_MASK                                  0x0000FFFF

#define ISP2_FG_R_POS_OFFSET                                              0x00000550
#define ISP2_FG_R_POS_TYPE                                                UInt32
#define ISP2_FG_R_POS_RESERVED_MASK                                       0x00000000
#define    ISP2_FG_R_POS_STRENGTH_SHIFT                                   16
#define    ISP2_FG_R_POS_STRENGTH_MASK                                    0xFFFF0000
#define    ISP2_FG_R_POS_LIMIT_SHIFT                                      0
#define    ISP2_FG_R_POS_LIMIT_MASK                                       0x0000FFFF

#define ISP2_FG_R_NEG_OFFSET                                              0x00000554
#define ISP2_FG_R_NEG_TYPE                                                UInt32
#define ISP2_FG_R_NEG_RESERVED_MASK                                       0x00000000
#define    ISP2_FG_R_NEG_STRENGTH_SHIFT                                   16
#define    ISP2_FG_R_NEG_STRENGTH_MASK                                    0xFFFF0000
#define    ISP2_FG_R_NEG_LIMIT_SHIFT                                      0
#define    ISP2_FG_R_NEG_LIMIT_MASK                                       0x0000FFFF

#define ISP2_FG_GR_POS_OFFSET                                             0x00000558
#define ISP2_FG_GR_POS_TYPE                                               UInt32
#define ISP2_FG_GR_POS_RESERVED_MASK                                      0x00000000
#define    ISP2_FG_GR_POS_STRENGTH_SHIFT                                  16
#define    ISP2_FG_GR_POS_STRENGTH_MASK                                   0xFFFF0000
#define    ISP2_FG_GR_POS_LIMIT_SHIFT                                     0
#define    ISP2_FG_GR_POS_LIMIT_MASK                                      0x0000FFFF

#define ISP2_FG_GR_NEG_OFFSET                                             0x0000055C
#define ISP2_FG_GR_NEG_TYPE                                               UInt32
#define ISP2_FG_GR_NEG_RESERVED_MASK                                      0x00000000
#define    ISP2_FG_GR_NEG_STRENGTH_SHIFT                                  16
#define    ISP2_FG_GR_NEG_STRENGTH_MASK                                   0xFFFF0000
#define    ISP2_FG_GR_NEG_LIMIT_SHIFT                                     0
#define    ISP2_FG_GR_NEG_LIMIT_MASK                                      0x0000FFFF

#define ISP2_FG_B_POS_OFFSET                                              0x00000560
#define ISP2_FG_B_POS_TYPE                                                UInt32
#define ISP2_FG_B_POS_RESERVED_MASK                                       0x00000000
#define    ISP2_FG_B_POS_STRENGTH_SHIFT                                   16
#define    ISP2_FG_B_POS_STRENGTH_MASK                                    0xFFFF0000
#define    ISP2_FG_B_POS_LIMIT_SHIFT                                      0
#define    ISP2_FG_B_POS_LIMIT_MASK                                       0x0000FFFF

#define ISP2_FG_B_NEG_OFFSET                                              0x00000564
#define ISP2_FG_B_NEG_TYPE                                                UInt32
#define ISP2_FG_B_NEG_RESERVED_MASK                                       0x00000000
#define    ISP2_FG_B_NEG_STRENGTH_SHIFT                                   16
#define    ISP2_FG_B_NEG_STRENGTH_MASK                                    0xFFFF0000
#define    ISP2_FG_B_NEG_LIMIT_SHIFT                                      0
#define    ISP2_FG_B_NEG_LIMIT_MASK                                       0x0000FFFF

#define ISP2_FG_GB_POS_OFFSET                                             0x00000568
#define ISP2_FG_GB_POS_TYPE                                               UInt32
#define ISP2_FG_GB_POS_RESERVED_MASK                                      0x00000000
#define    ISP2_FG_GB_POS_STRENGTH_SHIFT                                  16
#define    ISP2_FG_GB_POS_STRENGTH_MASK                                   0xFFFF0000
#define    ISP2_FG_GB_POS_LIMIT_SHIFT                                     0
#define    ISP2_FG_GB_POS_LIMIT_MASK                                      0x0000FFFF

#define ISP2_FG_GB_NEG_OFFSET                                             0x0000056C
#define ISP2_FG_GB_NEG_TYPE                                               UInt32
#define ISP2_FG_GB_NEG_RESERVED_MASK                                      0x00000000
#define    ISP2_FG_GB_NEG_STRENGTH_SHIFT                                  16
#define    ISP2_FG_GB_NEG_STRENGTH_MASK                                   0xFFFF0000
#define    ISP2_FG_GB_NEG_LIMIT_SHIFT                                     0
#define    ISP2_FG_GB_NEG_LIMIT_MASK                                      0x0000FFFF

#define ISP2_DN_CTRL_OFFSET                                               0x00000580
#define ISP2_DN_CTRL_TYPE                                                 UInt32
#define ISP2_DN_CTRL_RESERVED_MASK                                        0xFFFFFFFF

#define ISP2_DN_ABSC_OFFSET                                               0x00000584
#define ISP2_DN_ABSC_TYPE                                                 UInt32
#define ISP2_DN_ABSC_RESERVED_MASK                                        0x00000000
#define    ISP2_DN_ABSC_ABSCISSA1_SHIFT                                   16
#define    ISP2_DN_ABSC_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISP2_DN_ABSC_ABSCISSA0_SHIFT                                   0
#define    ISP2_DN_ABSC_ABSCISSA0_MASK                                    0x0000FFFF

#define ISP2_DN_ORD_OFFSET                                                0x00000594
#define ISP2_DN_ORD_TYPE                                                  UInt32
#define ISP2_DN_ORD_RESERVED_MASK                                         0x00000000
#define    ISP2_DN_ORD_ORDINATE1_SHIFT                                    16
#define    ISP2_DN_ORD_ORDINATE1_MASK                                     0xFFFF0000
#define    ISP2_DN_ORD_ORDINATE0_SHIFT                                    0
#define    ISP2_DN_ORD_ORDINATE0_MASK                                     0x0000FFFF

#define ISP2_DN_SLOPE_OFFSET                                              0x000005A4
#define ISP2_DN_SLOPE_TYPE                                                UInt32
#define ISP2_DN_SLOPE_RESERVED_MASK                                       0x00000000
#define    ISP2_DN_SLOPE_SLOPE1_SHIFT                                     16
#define    ISP2_DN_SLOPE_SLOPE1_MASK                                      0xFFFF0000
#define    ISP2_DN_SLOPE_SLOPE0_SHIFT                                     0
#define    ISP2_DN_SLOPE_SLOPE0_MASK                                      0x0000FFFF

#define ISP2_DN_MASK0_OFFSET                                              0x000005E8
#define ISP2_DN_MASK0_TYPE                                                UInt32
#define ISP2_DN_MASK0_RESERVED_MASK                                       0x00000000
#define    ISP2_DN_MASK0_MASK0_SHIFT                                      0
#define    ISP2_DN_MASK0_MASK0_MASK                                       0xFFFFFFFF

#define ISP2_DN_MASK1_OFFSET                                              0x000005EC
#define ISP2_DN_MASK1_TYPE                                                UInt32
#define ISP2_DN_MASK1_RESERVED_MASK                                       0xFFFE0000
#define    ISP2_DN_MASK1_MASK1_SHIFT                                      0
#define    ISP2_DN_MASK1_MASK1_MASK                                       0x0001FFFF

#define ISP2_DN_STRENGTH_OFFSET                                           0x000005F0
#define ISP2_DN_STRENGTH_TYPE                                             UInt32
#define ISP2_DN_STRENGTH_RESERVED_MASK                                    0x03F30303
#define    ISP2_DN_STRENGTH_B_STR_SHIFT                                   26
#define    ISP2_DN_STRENGTH_B_STR_MASK                                    0xFC000000
#define    ISP2_DN_STRENGTH_R_STR_SHIFT                                   18
#define    ISP2_DN_STRENGTH_R_STR_MASK                                    0x000C0000
#define    ISP2_DN_STRENGTH_GB_STR_SHIFT                                  10
#define    ISP2_DN_STRENGTH_GB_STR_MASK                                   0x0000FC00
#define    ISP2_DN_STRENGTH_GR_STR_SHIFT                                  2
#define    ISP2_DN_STRENGTH_GR_STR_MASK                                   0x000000FC

#define ISP2_DN_THRGAIN_OFFSET                                            0x000005F4
#define ISP2_DN_THRGAIN_TYPE                                              UInt32
#define ISP2_DN_THRGAIN_RESERVED_MASK                                     0xC0C0C0C0
#define    ISP2_DN_THRGAIN_B_GAIN_SHIFT                                   24
#define    ISP2_DN_THRGAIN_B_GAIN_MASK                                    0x3F000000
#define    ISP2_DN_THRGAIN_R_GAIN_SHIFT                                   16
#define    ISP2_DN_THRGAIN_R_GAIN_MASK                                    0x003F0000
#define    ISP2_DN_THRGAIN_GB_GAIN_SHIFT                                  8
#define    ISP2_DN_THRGAIN_GB_GAIN_MASK                                   0x00003F00
#define    ISP2_DN_THRGAIN_GR_GAIN_SHIFT                                  0
#define    ISP2_DN_THRGAIN_GR_GAIN_MASK                                   0x0000003F

#define ISP2_SW_CTRL_OFFSET                                               0x00000680
#define ISP2_SW_CTRL_TYPE                                                 UInt32
#define ISP2_SW_CTRL_RESERVED_MASK                                        0xFFFF0007
#define    ISP2_SW_CTRL_SHIFT_Y_SHIFT                                     12
#define    ISP2_SW_CTRL_SHIFT_Y_MASK                                      0x0000F000
#define    ISP2_SW_CTRL_SHIFT_CBCR_SHIFT                                  8
#define    ISP2_SW_CTRL_SHIFT_CBCR_MASK                                   0x00000F00
#define    ISP2_SW_CTRL_SINGLE_CHANNEL_SHIFT                              7
#define    ISP2_SW_CTRL_SINGLE_CHANNEL_MASK                               0x00000080
#define    ISP2_SW_CTRL_READ_EN_SHIFT                                     6
#define    ISP2_SW_CTRL_READ_EN_MASK                                      0x00000040
#define    ISP2_SW_CTRL_WRITE_EN_SHIFT                                    5
#define    ISP2_SW_CTRL_WRITE_EN_MASK                                     0x00000020
#define    ISP2_SW_CTRL_EIGHT_BIT_SHIFT                                   4
#define    ISP2_SW_CTRL_EIGHT_BIT_MASK                                    0x00000010
#define    ISP2_SW_CTRL_CLEAR_SHIFT                                       3
#define    ISP2_SW_CTRL_CLEAR_MASK                                        0x00000008

#define ISP2_SW_DADDR_OFFSET                                              0x00000684
#define ISP2_SW_DADDR_TYPE                                                UInt32
#define ISP2_SW_DADDR_RESERVED_MASK                                       0x0000001F
#define    ISP2_SW_DADDR_DESTADDR_SHIFT                                   5
#define    ISP2_SW_DADDR_DESTADDR_MASK                                    0xFFFFFFE0

#define ISP2_SW_SADDR_OFFSET                                              0x00000688
#define ISP2_SW_SADDR_TYPE                                                UInt32
#define ISP2_SW_SADDR_RESERVED_MASK                                       0x0000001F
#define    ISP2_SW_SADDR_SRCADDR_SHIFT                                    5
#define    ISP2_SW_SADDR_SRCADDR_MASK                                     0xFFFFFFE0

#define ISP2_SW_DSIZE_OFFSET                                              0x0000068C
#define ISP2_SW_DSIZE_TYPE                                                UInt32
#define ISP2_SW_DSIZE_RESERVED_MASK                                       0x0007C000
#define    ISP2_SW_DSIZE_PITCH_SHIFT                                      19
#define    ISP2_SW_DSIZE_PITCH_MASK                                       0xFFF80000
#define    ISP2_SW_DSIZE_LINES_SHIFT                                      0
#define    ISP2_SW_DSIZE_LINES_MASK                                       0x00003FFF

#define ISP2_SW_SSIZE_OFFSET                                              0x00000690
#define ISP2_SW_SSIZE_TYPE                                                UInt32
#define ISP2_SW_SSIZE_RESERVED_MASK                                       0x0007C000
#define    ISP2_SW_SSIZE_PITCH_SHIFT                                      19
#define    ISP2_SW_SSIZE_PITCH_MASK                                       0xFFF80000
#define    ISP2_SW_SSIZE_LINES_SHIFT                                      0
#define    ISP2_SW_SSIZE_LINES_MASK                                       0x00003FFF

#define ISP2_SW_DINT_OFFSET                                               0x00000694
#define ISP2_SW_DINT_TYPE                                                 UInt32
#define ISP2_SW_DINT_RESERVED_MASK                                        0xC000C000
#define    ISP2_SW_DINT_DINT_LINES_SHIFT                                  16
#define    ISP2_SW_DINT_DINT_LINES_MASK                                   0x3FFF0000
#define    ISP2_SW_DINT_DINT_LINES1_SHIFT                                 0
#define    ISP2_SW_DINT_DINT_LINES1_MASK                                  0x00003FFF

#define ISP2_SW_SINT_OFFSET                                               0x00000698
#define ISP2_SW_SINT_TYPE                                                 UInt32
#define ISP2_SW_SINT_RESERVED_MASK                                        0xFFFFC000
#define    ISP2_SW_SINT_SINT_LINES_SHIFT                                  0
#define    ISP2_SW_SINT_SINT_LINES_MASK                                   0x00003FFF

#define ISP2_SW_DWRITE_OFFSET                                             0x0000069C
#define ISP2_SW_DWRITE_TYPE                                               UInt32
#define ISP2_SW_DWRITE_RESERVED_MASK                                      0xFFFFC000
#define    ISP2_SW_DWRITE_DWRITE_SHIFT                                    0
#define    ISP2_SW_DWRITE_DWRITE_MASK                                     0x00003FFF

#define ISP2_SW_DREAD_OFFSET                                              0x000006A0
#define ISP2_SW_DREAD_TYPE                                                UInt32
#define ISP2_SW_DREAD_RESERVED_MASK                                       0xFFFFC000
#define    ISP2_SW_DREAD_DREAD_SHIFT                                      0
#define    ISP2_SW_DREAD_DREAD_MASK                                       0x00003FFF

#define ISP2_SW_SWRITE_OFFSET                                             0x000006A4
#define ISP2_SW_SWRITE_TYPE                                               UInt32
#define ISP2_SW_SWRITE_RESERVED_MASK                                      0xFFFFC000
#define    ISP2_SW_SWRITE_SWRITE_SHIFT                                    0
#define    ISP2_SW_SWRITE_SWRITE_MASK                                     0x00003FFF

#define ISP2_SW_SREAD_OFFSET                                              0x000006A8
#define ISP2_SW_SREAD_TYPE                                                UInt32
#define ISP2_SW_SREAD_RESERVED_MASK                                       0xFFFFC000
#define    ISP2_SW_SREAD_SREAD_SHIFT                                      0
#define    ISP2_SW_SREAD_SREAD_MASK                                       0x00003FFF

#define ISP2_DM_CTRL_OFFSET                                               0x000006D0
#define ISP2_DM_CTRL_TYPE                                                 UInt32
#define ISP2_DM_CTRL_RESERVED_MASK                                        0xE00FF0FC
#define    ISP2_DM_CTRL_HV_BIAS_SHIFT                                     20
#define    ISP2_DM_CTRL_HV_BIAS_MASK                                      0x1FF00000
#define    ISP2_DM_CTRL_G_OFF_SH_SHIFT                                    8
#define    ISP2_DM_CTRL_G_OFF_SH_MASK                                     0x00000F00
#define    ISP2_DM_CTRL_V_COSITED_SHIFT                                   1
#define    ISP2_DM_CTRL_V_COSITED_MASK                                    0x00000002
#define    ISP2_DM_CTRL_H_COSITED_SHIFT                                   0
#define    ISP2_DM_CTRL_H_COSITED_MASK                                    0x00000001

#define ISP2_DM_THRESH_OFFSET                                             0x000006D4
#define ISP2_DM_THRESH_TYPE                                               UInt32
#define ISP2_DM_THRESH_RESERVED_MASK                                      0x00000000
#define    ISP2_DM_THRESH_THRESH_SHIFT                                    0
#define    ISP2_DM_THRESH_THRESH_MASK                                     0xFFFFFFFF

#define ISP2_YI_CTRL_OFFSET                                               0x00000700
#define ISP2_YI_CTRL_TYPE                                                 UInt32
#define ISP2_YI_CTRL_RESERVED_MASK                                        0xFCFFEC08
#define    ISP2_YI_CTRL_COL_WIDTH_SHIFT                                   24
#define    ISP2_YI_CTRL_COL_WIDTH_MASK                                    0x03000000
#define    ISP2_YI_CTRL_COL_MODE_SHIFT                                    12
#define    ISP2_YI_CTRL_COL_MODE_MASK                                     0x00001000
#define    ISP2_YI_CTRL_CC_ORDER_SHIFT                                    7
#define    ISP2_YI_CTRL_CC_ORDER_MASK                                     0x00000380
#define    ISP2_YI_CTRL_DATA_FORMAT_SHIFT                                 4
#define    ISP2_YI_CTRL_DATA_FORMAT_MASK                                  0x00000070
#define    ISP2_YI_CTRL_BYTES_SAMPLE_SHIFT                                2
#define    ISP2_YI_CTRL_BYTES_SAMPLE_MASK                                 0x00000004
#define    ISP2_YI_CTRL_CS_FORMAT_SHIFT                                   0
#define    ISP2_YI_CTRL_CS_FORMAT_MASK                                    0x00000003

#define ISP2_YI_RY_ADDR_OFFSET                                            0x00000704
#define ISP2_YI_RY_ADDR_TYPE                                              UInt32
#define ISP2_YI_RY_ADDR_RESERVED_MASK                                     0x0000001F
#define    ISP2_YI_RY_ADDR_ADDRESS_SHIFT                                  5
#define    ISP2_YI_RY_ADDR_ADDRESS_MASK                                   0xFFFFFFE0

#define ISP2_YI_GU_ADDR_OFFSET                                            0x00000708
#define ISP2_YI_GU_ADDR_TYPE                                              UInt32
#define ISP2_YI_GU_ADDR_RESERVED_MASK                                     0x0000001F
#define    ISP2_YI_GU_ADDR_ADDRESS_SHIFT                                  5
#define    ISP2_YI_GU_ADDR_ADDRESS_MASK                                   0xFFFFFFE0

#define ISP2_YI_BV_ADDR_OFFSET                                            0x0000070C
#define ISP2_YI_BV_ADDR_TYPE                                              UInt32
#define ISP2_YI_BV_ADDR_RESERVED_MASK                                     0x0000001F
#define    ISP2_YI_BV_ADDR_ADDRESS_SHIFT                                  5
#define    ISP2_YI_BV_ADDR_ADDRESS_MASK                                   0xFFFFFFE0

#define ISP2_YI_STRIDE1_OFFSET                                            0x00000710
#define ISP2_YI_STRIDE1_TYPE                                              UInt32
#define ISP2_YI_STRIDE1_RESERVED_MASK                                     0xFFF0001F
#define    ISP2_YI_STRIDE1_Y_STRIDE_SHIFT                                 5
#define    ISP2_YI_STRIDE1_Y_STRIDE_MASK                                  0x000FFFE0

#define ISP2_YI_STRIDE2_OFFSET                                            0x00000714
#define ISP2_YI_STRIDE2_TYPE                                              UInt32
#define ISP2_YI_STRIDE2_RESERVED_MASK                                     0xFFF0000F
#define    ISP2_YI_STRIDE2_UV_STRIDE_SHIFT                                4
#define    ISP2_YI_STRIDE2_UV_STRIDE_MASK                                 0x000FFFF0

#define ISP2_YI_COL_STRIDE1_OFFSET                                        0x00000718
#define ISP2_YI_COL_STRIDE1_TYPE                                          UInt32
#define ISP2_YI_COL_STRIDE1_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_YI_COL_STRIDE1_STRIDE_SHIFT                               9
#define    ISP2_YI_COL_STRIDE1_STRIDE_MASK                                0x01FFFE00

#define ISP2_YI_COL_STRIDE2_OFFSET                                        0x0000071C
#define ISP2_YI_COL_STRIDE2_TYPE                                          UInt32
#define ISP2_YI_COL_STRIDE2_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_YI_COL_STRIDE2_STRIDE_SHIFT                               9
#define    ISP2_YI_COL_STRIDE2_STRIDE_MASK                                0x01FFFE00

#define ISP2_YI_RY_EADDR_OFFSET                                           0x00000720
#define ISP2_YI_RY_EADDR_TYPE                                             UInt32
#define ISP2_YI_RY_EADDR_RESERVED_MASK                                    0x0000001F
#define    ISP2_YI_RY_EADDR_ENDADDR_SHIFT                                 5
#define    ISP2_YI_RY_EADDR_ENDADDR_MASK                                  0xFFFFFFE0

#define ISP2_YI_GU_EADDR_OFFSET                                           0x00000724
#define ISP2_YI_GU_EADDR_TYPE                                             UInt32
#define ISP2_YI_GU_EADDR_RESERVED_MASK                                    0x0000001F
#define    ISP2_YI_GU_EADDR_ENDADDR_SHIFT                                 5
#define    ISP2_YI_GU_EADDR_ENDADDR_MASK                                  0xFFFFFFE0

#define ISP2_YI_BV_EADDR_OFFSET                                           0x00000728
#define ISP2_YI_BV_EADDR_TYPE                                             UInt32
#define ISP2_YI_BV_EADDR_RESERVED_MASK                                    0x0000001F
#define    ISP2_YI_BV_EADDR_ENDADDR_SHIFT                                 5
#define    ISP2_YI_BV_EADDR_ENDADDR_MASK                                  0xFFFFFFE0

#define ISP2_YC_MATRIX_OFFSET                                             0x00000750
#define ISP2_YC_MATRIX_TYPE                                               UInt32
#define ISP2_YC_MATRIX_RESERVED_MASK                                      0x00000000
#define    ISP2_YC_MATRIX_ELEMENT1_SHIFT                                  16
#define    ISP2_YC_MATRIX_ELEMENT1_MASK                                   0xFFFF0000
#define    ISP2_YC_MATRIX_ELEMENT0_SHIFT                                  0
#define    ISP2_YC_MATRIX_ELEMENT0_MASK                                   0x0000FFFF

#define ISP2_YC_OFFSET_OFFSET                                             0x00000764
#define ISP2_YC_OFFSET_TYPE                                               UInt32
#define ISP2_YC_OFFSET_RESERVED_MASK                                      0xFFFE0000
#define    ISP2_YC_OFFSET_OFFSET_SHIFT                                    0
#define    ISP2_YC_OFFSET_OFFSET_MASK                                     0x0001FFFF

#define ISP2_GM_ABSC_R_OFFSET                                             0x00000800
#define ISP2_GM_ABSC_R_TYPE                                               UInt32
#define ISP2_GM_ABSC_R_RESERVED_MASK                                      0x00000000
#define    ISP2_GM_ABSC_R_ABSCISSA1_SHIFT                                 16
#define    ISP2_GM_ABSC_R_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_GM_ABSC_R_ABSCISSA0_SHIFT                                 0
#define    ISP2_GM_ABSC_R_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_GM_ABSC_G_OFFSET                                             0x00000840
#define ISP2_GM_ABSC_G_TYPE                                               UInt32
#define ISP2_GM_ABSC_G_RESERVED_MASK                                      0x00000000
#define    ISP2_GM_ABSC_G_ABSCISSA1_SHIFT                                 16
#define    ISP2_GM_ABSC_G_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_GM_ABSC_G_ABSCISSA0_SHIFT                                 0
#define    ISP2_GM_ABSC_G_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_GM_ABSC_B_OFFSET                                             0x00000880
#define ISP2_GM_ABSC_B_TYPE                                               UInt32
#define ISP2_GM_ABSC_B_RESERVED_MASK                                      0x00000000
#define    ISP2_GM_ABSC_B_ABSCISSA1_SHIFT                                 16
#define    ISP2_GM_ABSC_B_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_GM_ABSC_B_ABSCISSA0_SHIFT                                 0
#define    ISP2_GM_ABSC_B_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_FC_STRENGTH_OFFSET                                           0x00000970
#define ISP2_FC_STRENGTH_TYPE                                             UInt32
#define ISP2_FC_STRENGTH_RESERVED_MASK                                    0xFF0FF00F
#define    ISP2_FC_STRENGTH_STRENGTH2_SHIFT                               20
#define    ISP2_FC_STRENGTH_STRENGTH2_MASK                                0x00F00000
#define    ISP2_FC_STRENGTH_STRENGTH1_SHIFT                               4
#define    ISP2_FC_STRENGTH_STRENGTH1_MASK                                0x00000FF0

#define ISP2_FC_FMIX_OFFSET                                               0x00000974
#define ISP2_FC_FMIX_TYPE                                                 UInt32
#define ISP2_FC_FMIX_RESERVED_MASK                                        0xFFFF601F
#define    ISP2_FC_FMIX_ABSOLUTE_SHIFT                                    15
#define    ISP2_FC_FMIX_ABSOLUTE_MASK                                     0x00008000
#define    ISP2_FC_FMIX_WEIGHT_SHIFT                                      5
#define    ISP2_FC_FMIX_WEIGHT_MASK                                       0x00001FE0

#define ISP2_FC_FMIX_THRESH_OFFSET                                        0x00000978
#define ISP2_FC_FMIX_THRESH_TYPE                                          UInt32
#define ISP2_FC_FMIX_THRESH_RESERVED_MASK                                 0x00000000
#define    ISP2_FC_FMIX_THRESH_HI_SHIFT                                   16
#define    ISP2_FC_FMIX_THRESH_HI_MASK                                    0xFFFF0000
#define    ISP2_FC_FMIX_THRESH_LO_SHIFT                                   0
#define    ISP2_FC_FMIX_THRESH_LO_MASK                                    0x0000FFFF

#define ISP2_FC_FMIX_NORM_OFFSET                                          0x0000097C
#define ISP2_FC_FMIX_NORM_TYPE                                            UInt32
#define ISP2_FC_FMIX_NORM_RESERVED_MASK                                   0xF000F000
#define    ISP2_FC_FMIX_NORM_NORM2_SHIFT                                  16
#define    ISP2_FC_FMIX_NORM_NORM2_MASK                                   0x0FFF0000
#define    ISP2_FC_FMIX_NORM_NORM_SHIFT                                   0
#define    ISP2_FC_FMIX_NORM_NORM_MASK                                    0x00000FFF

#define ISP2_FC_FDESAT_OFFSET                                             0x00000980
#define ISP2_FC_FDESAT_TYPE                                               UInt32
#define ISP2_FC_FDESAT_RESERVED_MASK                                      0xFFFF601F
#define    ISP2_FC_FDESAT_ABSOLUTE_SHIFT                                  15
#define    ISP2_FC_FDESAT_ABSOLUTE_MASK                                   0x00008000
#define    ISP2_FC_FDESAT_WEIGHT_SHIFT                                    5
#define    ISP2_FC_FDESAT_WEIGHT_MASK                                     0x00001FE0

#define ISP2_FC_FDESAT_THRESH_OFFSET                                      0x00000984
#define ISP2_FC_FDESAT_THRESH_TYPE                                        UInt32
#define ISP2_FC_FDESAT_THRESH_RESERVED_MASK                               0x00000000
#define    ISP2_FC_FDESAT_THRESH_HI_SHIFT                                 16
#define    ISP2_FC_FDESAT_THRESH_HI_MASK                                  0xFFFF0000
#define    ISP2_FC_FDESAT_THRESH_LO_SHIFT                                 0
#define    ISP2_FC_FDESAT_THRESH_LO_MASK                                  0x0000FFFF

#define ISP2_FC_FDESAT_NORM_OFFSET                                        0x00000988
#define ISP2_FC_FDESAT_NORM_TYPE                                          UInt32
#define ISP2_FC_FDESAT_NORM_RESERVED_MASK                                 0xF000F000
#define    ISP2_FC_FDESAT_NORM_NORM2_SHIFT                                16
#define    ISP2_FC_FDESAT_NORM_NORM2_MASK                                 0x0FFF0000
#define    ISP2_FC_FDESAT_NORM_NORM_SHIFT                                 0
#define    ISP2_FC_FDESAT_NORM_NORM_MASK                                  0x00000FFF

#define ISP2_SH_STRENGTH_OFFSET                                           0x000009B4
#define ISP2_SH_STRENGTH_TYPE                                             UInt32
#define ISP2_SH_STRENGTH_RESERVED_MASK                                    0x000007FF
#define    ISP2_SH_STRENGTH_ISOTROPIC_STR_SHIFT                           16
#define    ISP2_SH_STRENGTH_ISOTROPIC_STR_MASK                            0xFFFF0000
#define    ISP2_SH_STRENGTH_AVG_G_SH_SHIFT                                11
#define    ISP2_SH_STRENGTH_AVG_G_SH_MASK                                 0x0000F800

#define ISP2_SH_CENTRE_OFFSET                                             0x000009B8
#define ISP2_SH_CENTRE_TYPE                                               UInt32
#define ISP2_SH_CENTRE_RESERVED_MASK                                      0x00000000
#define    ISP2_SH_CENTRE_Y_CENTRE_SHIFT                                  16
#define    ISP2_SH_CENTRE_Y_CENTRE_MASK                                   0xFFFF0000
#define    ISP2_SH_CENTRE_X_CENTRE_SHIFT                                  0
#define    ISP2_SH_CENTRE_X_CENTRE_MASK                                   0x0000FFFF

#define ISP2_GD_CTRL_OFFSET                                               0x00000A20
#define ISP2_GD_CTRL_TYPE                                                 UInt32
#define ISP2_GD_CTRL_RESERVED_MASK                                        0xFFFFFF0F
#define    ISP2_GD_CTRL_HFLIP_SHIFT                                       7
#define    ISP2_GD_CTRL_HFLIP_MASK                                        0x00000080
#define    ISP2_GD_CTRL_SHIFT_SHIFT                                       4
#define    ISP2_GD_CTRL_SHIFT_MASK                                        0x00000070

#define ISP2_GD_CSZ_OFFSET                                                0x00000A24
#define ISP2_GD_CSZ_TYPE                                                  UInt32
#define ISP2_GD_CSZ_RESERVED_MASK                                         0xFC00FC00
#define    ISP2_GD_CSZ_CELL_SIZE_X_SHIFT                                  16
#define    ISP2_GD_CSZ_CELL_SIZE_X_MASK                                   0x03FF0000
#define    ISP2_GD_CSZ_CELL_SIZE_Y_SHIFT                                  0
#define    ISP2_GD_CSZ_CELL_SIZE_Y_MASK                                   0x000003FF

#define ISP2_GD_RCSZ_OFFSET                                               0x00000A28
#define ISP2_GD_RCSZ_TYPE                                                 UInt32
#define ISP2_GD_RCSZ_RESERVED_MASK                                        0x00000000
#define    ISP2_GD_RCSZ_RECIP_CELL_SIZE_X_SHIFT                           16
#define    ISP2_GD_RCSZ_RECIP_CELL_SIZE_X_MASK                            0xFFFF0000
#define    ISP2_GD_RCSZ_RECIP_CELL_SIZE_Y_SHIFT                           0
#define    ISP2_GD_RCSZ_RECIP_CELL_SIZE_Y_MASK                            0x0000FFFF

#define ISP2_HR_CTRL_OFFSET                                               0x00000A50
#define ISP2_HR_CTRL_TYPE                                                 UInt32
#define ISP2_HR_CTRL_RESERVED_MASK                                        0xFFFFFFF8
#define    ISP2_HR_CTRL_MODE_SHIFT                                        0
#define    ISP2_HR_CTRL_MODE_MASK                                         0x00000007

#define ISP2_HR_SCALE_X_OFFSET                                            0x00000A54
#define ISP2_HR_SCALE_X_TYPE                                              UInt32
#define ISP2_HR_SCALE_X_RESERVED_MASK                                     0xFFC0000F
#define    ISP2_HR_SCALE_X_SCALE_SHIFT                                    4
#define    ISP2_HR_SCALE_X_SCALE_MASK                                     0x003FFFF0

#define ISP2_HR_SCALE_Y_OFFSET                                            0x00000A58
#define ISP2_HR_SCALE_Y_TYPE                                              UInt32
#define ISP2_HR_SCALE_Y_RESERVED_MASK                                     0xFFC0000F
#define    ISP2_HR_SCALE_Y_SCALE_SHIFT                                    4
#define    ISP2_HR_SCALE_Y_SCALE_MASK                                     0x003FFFF0

#define ISP2_HR_NORM_OFFSET                                               0x00000A5C
#define ISP2_HR_NORM_TYPE                                                 UInt32
#define ISP2_HR_NORM_RESERVED_MASK                                        0xF000F000
#define    ISP2_HR_NORM_NORM_Y_SHIFT                                      16
#define    ISP2_HR_NORM_NORM_Y_MASK                                       0x0FFF0000
#define    ISP2_HR_NORM_NORM_X_SHIFT                                      0
#define    ISP2_HR_NORM_NORM_X_MASK                                       0x00000FFF

#define ISP2_LR_TSCALEX_OFFSET                                            0x00000B30
#define ISP2_LR_TSCALEX_TYPE                                              UInt32
#define ISP2_LR_TSCALEX_RESERVED_MASK                                     0xFFC00000
#define    ISP2_LR_TSCALEX_SCALE_SHIFT                                    0
#define    ISP2_LR_TSCALEX_SCALE_MASK                                     0x003FFFFF

#define ISP2_LR_TSCALEY_OFFSET                                            0x00000B34
#define ISP2_LR_TSCALEY_TYPE                                              UInt32
#define ISP2_LR_TSCALEY_RESERVED_MASK                                     0xFFC00000
#define    ISP2_LR_TSCALEY_SCALE_SHIFT                                    0
#define    ISP2_LR_TSCALEY_SCALE_MASK                                     0x003FFFFF

#define ISP2_LR_NORM_0_1_OFFSET                                           0x00000B38
#define ISP2_LR_NORM_0_1_TYPE                                             UInt32
#define ISP2_LR_NORM_0_1_RESERVED_MASK                                    0xF000F000
#define    ISP2_LR_NORM_0_1_NORM_Y_SHIFT                                  16
#define    ISP2_LR_NORM_0_1_NORM_Y_MASK                                   0x0FFF0000
#define    ISP2_LR_NORM_0_1_NORM_X_SHIFT                                  0
#define    ISP2_LR_NORM_0_1_NORM_X_MASK                                   0x00000FFF

#define ISP2_LR_NORM_2_3_OFFSET                                           0x00000B3C
#define ISP2_LR_NORM_2_3_TYPE                                             UInt32
#define ISP2_LR_NORM_2_3_RESERVED_MASK                                    0xF000F000
#define    ISP2_LR_NORM_2_3_NORM_Y_SHIFT                                  16
#define    ISP2_LR_NORM_2_3_NORM_Y_MASK                                   0x0FFF0000
#define    ISP2_LR_NORM_2_3_NORM_X_SHIFT                                  0
#define    ISP2_LR_NORM_2_3_NORM_X_MASK                                   0x00000FFF

#define ISP2_LR_SHIFT_OFFSET                                              0x00000B40
#define ISP2_LR_SHIFT_TYPE                                                UInt32
#define ISP2_LR_SHIFT_RESERVED_MASK                                       0xFFFFFEF0
#define    ISP2_LR_SHIFT_RB_SWAP_SHIFT                                    8
#define    ISP2_LR_SHIFT_RB_SWAP_MASK                                     0x00000100
#define    ISP2_LR_SHIFT_SHIFT_SHIFT                                      0
#define    ISP2_LR_SHIFT_SHIFT_MASK                                       0x0000000F

#define ISP2_CC_MATRIX_OFFSET                                             0x00000B50
#define ISP2_CC_MATRIX_TYPE                                               UInt32
#define ISP2_CC_MATRIX_RESERVED_MASK                                      0x00000000
#define    ISP2_CC_MATRIX_ELEMENT1_SHIFT                                  16
#define    ISP2_CC_MATRIX_ELEMENT1_MASK                                   0xFFFF0000
#define    ISP2_CC_MATRIX_ELEMENT0_SHIFT                                  0
#define    ISP2_CC_MATRIX_ELEMENT0_MASK                                   0x0000FFFF

#define ISP2_CC_OFFSET_OFFSET                                             0x00000B64
#define ISP2_CC_OFFSET_TYPE                                               UInt32
#define ISP2_CC_OFFSET_RESERVED_MASK                                      0xFFFC0000
#define    ISP2_CC_OFFSET_OFFSET_SHIFT                                    0
#define    ISP2_CC_OFFSET_OFFSET_MASK                                     0x0003FFFF

#define ISP2_ST_FOC_CTRL_OFFSET                                           0x00000B80
#define ISP2_ST_FOC_CTRL_TYPE                                             UInt32
#define ISP2_ST_FOC_CTRL_RESERVED_MASK                                    0xFFFFFFFE
#define    ISP2_ST_FOC_CTRL_R_B_MODE_SHIFT                                0
#define    ISP2_ST_FOC_CTRL_R_B_MODE_MASK                                 0x00000001

#define ISP2_ST_FOC_FILTEX_OFFSET                                         0x00000B84
#define ISP2_ST_FOC_FILTEX_TYPE                                           UInt32
#define ISP2_ST_FOC_FILTEX_RESERVED_MASK                                  0xFF00FF00
#define    ISP2_ST_FOC_FILTEX_COEFF1_SHIFT                                16
#define    ISP2_ST_FOC_FILTEX_COEFF1_MASK                                 0x00FF0000
#define    ISP2_ST_FOC_FILTEX_COEFF0_SHIFT                                0
#define    ISP2_ST_FOC_FILTEX_COEFF0_MASK                                 0x000000FF

#define ISP2_ST_SHIFT_OFFSET                                              0x00000BA0
#define ISP2_ST_SHIFT_TYPE                                                UInt32
#define ISP2_ST_SHIFT_RESERVED_MASK                                       0xFCFF0000
#define    ISP2_ST_SHIFT_PIX_SHIFT_SHIFT                                  24
#define    ISP2_ST_SHIFT_PIX_SHIFT_MASK                                   0x03000000
#define    ISP2_ST_SHIFT_SHIFT_SHIFT                                      0
#define    ISP2_ST_SHIFT_SHIFT_MASK                                       0x0000FFFF

#define ISP2_ST_R_OFF_OFFSET                                              0x00000BA4
#define ISP2_ST_R_OFF_TYPE                                                UInt32
#define ISP2_ST_R_OFF_RESERVED_MASK                                       0x00000000
#define    ISP2_ST_R_OFF_Y_OFFSET_SHIFT                                   16
#define    ISP2_ST_R_OFF_Y_OFFSET_MASK                                    0xFFFF0000
#define    ISP2_ST_R_OFF_X_OFFSET_SHIFT                                   0
#define    ISP2_ST_R_OFF_X_OFFSET_MASK                                    0x0000FFFF

#define ISP2_ST_R_RECT_OFFSET                                             0x00000C24
#define ISP2_ST_R_RECT_TYPE                                               UInt32
#define ISP2_ST_R_RECT_RESERVED_MASK                                      0x00000000
#define    ISP2_ST_R_RECT_HEIGHT_SHIFT                                    16
#define    ISP2_ST_R_RECT_HEIGHT_MASK                                     0xFFFF0000
#define    ISP2_ST_R_RECT_ISPST_WIDTH_SHIFT                               0
#define    ISP2_ST_R_RECT_ISPST_WIDTH_MASK                                0x0000FFFF

#define ISP2_ST_HMASK0_OFFSET                                             0x00000CA4
#define ISP2_ST_HMASK0_TYPE                                               UInt32
#define ISP2_ST_HMASK0_RESERVED_MASK                                      0x00000000
#define    ISP2_ST_HMASK0_HIST1_SHIFT                                     16
#define    ISP2_ST_HMASK0_HIST1_MASK                                      0xFFFF0000
#define    ISP2_ST_HMASK0_HIST0_SHIFT                                     0
#define    ISP2_ST_HMASK0_HIST0_MASK                                      0x0000FFFF

#define ISP2_ST_FOC_FILT_OFFSET                                           0x00000CAC
#define ISP2_ST_FOC_FILT_TYPE                                             UInt32
#define ISP2_ST_FOC_FILT_RESERVED_MASK                                    0xFF00FF00
#define    ISP2_ST_FOC_FILT_COEFF1_SHIFT                                  16
#define    ISP2_ST_FOC_FILT_COEFF1_MASK                                   0x00FF0000
#define    ISP2_ST_FOC_FILT_COEFF0_SHIFT                                  0
#define    ISP2_ST_FOC_FILT_COEFF0_MASK                                   0x000000FF

#define ISP2_ST_FILT_GAINS_OFFSET                                         0x00000CD0
#define ISP2_ST_FILT_GAINS_TYPE                                           UInt32
#define ISP2_ST_FILT_GAINS_RESERVED_MASK                                  0xFF00FF00
#define    ISP2_ST_FILT_GAINS_GAIN1_SHIFT                                 16
#define    ISP2_ST_FILT_GAINS_GAIN1_MASK                                  0x00FF0000
#define    ISP2_ST_FILT_GAINS_GAIN0_SHIFT                                 0
#define    ISP2_ST_FILT_GAINS_GAIN0_MASK                                  0x000000FF

#define ISP2_ST_FILT_TH_OFFSET                                            0x00000CD4
#define ISP2_ST_FILT_TH_TYPE                                              UInt32
#define ISP2_ST_FILT_TH_RESERVED_MASK                                     0xFFFF0000
#define    ISP2_ST_FILT_TH_THRESHOLD_SHIFT                                0
#define    ISP2_ST_FILT_TH_THRESHOLD_MASK                                 0x0000FFFF

#define ISP2_ST_ROW_NUM_OFFSET                                            0x00000CDC
#define ISP2_ST_ROW_NUM_TYPE                                              UInt32
#define ISP2_ST_ROW_NUM_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_ST_ROW_NUM_COL_SHIFT                                      16
#define    ISP2_ST_ROW_NUM_COL_MASK                                       0x00010000
#define    ISP2_ST_ROW_NUM_ROW_NUM_SHIFT                                  0
#define    ISP2_ST_ROW_NUM_ROW_NUM_MASK                                   0x0000FFFF

#define ISP2_ST_R_TH_OFFSET                                               0x00000CE0
#define ISP2_ST_R_TH_TYPE                                                 UInt32
#define ISP2_ST_R_TH_RESERVED_MASK                                        0x00000000
#define    ISP2_ST_R_TH_HI_SHIFT                                          16
#define    ISP2_ST_R_TH_HI_MASK                                           0xFFFF0000
#define    ISP2_ST_R_TH_LO_SHIFT                                          0
#define    ISP2_ST_R_TH_LO_MASK                                           0x0000FFFF

#define ISP2_ST_G_TH_OFFSET                                               0x00000CEC
#define ISP2_ST_G_TH_TYPE                                                 UInt32
#define ISP2_ST_G_TH_RESERVED_MASK                                        0x00000000
#define    ISP2_ST_G_TH_HI_SHIFT                                          16
#define    ISP2_ST_G_TH_HI_MASK                                           0xFFFF0000
#define    ISP2_ST_G_TH_LO_SHIFT                                          0
#define    ISP2_ST_G_TH_LO_MASK                                           0x0000FFFF

#define ISP2_ST_B_TH_OFFSET                                               0x00000CF8
#define ISP2_ST_B_TH_TYPE                                                 UInt32
#define ISP2_ST_B_TH_RESERVED_MASK                                        0x00000000
#define    ISP2_ST_B_TH_HI_SHIFT                                          16
#define    ISP2_ST_B_TH_HI_MASK                                           0xFFFF0000
#define    ISP2_ST_B_TH_LO_SHIFT                                          0
#define    ISP2_ST_B_TH_LO_MASK                                           0x0000FFFF

#define ISP2_ST_R_G_TH_OFFSET                                             0x00000D04
#define ISP2_ST_R_G_TH_TYPE                                               UInt32
#define ISP2_ST_R_G_TH_RESERVED_MASK                                      0x00000000
#define    ISP2_ST_R_G_TH_HI_SHIFT                                        16
#define    ISP2_ST_R_G_TH_HI_MASK                                         0xFFFF0000
#define    ISP2_ST_R_G_TH_LO_SHIFT                                        0
#define    ISP2_ST_R_G_TH_LO_MASK                                         0x0000FFFF

#define ISP2_ST_B_G_TH_OFFSET                                             0x00000D10
#define ISP2_ST_B_G_TH_TYPE                                               UInt32
#define ISP2_ST_B_G_TH_RESERVED_MASK                                      0x00000000
#define    ISP2_ST_B_G_TH_HI_SHIFT                                        16
#define    ISP2_ST_B_G_TH_HI_MASK                                         0xFFFF0000
#define    ISP2_ST_B_G_TH_LO_SHIFT                                        0
#define    ISP2_ST_B_G_TH_LO_MASK                                         0x0000FFFF

#define ISP2_ST_GROUP_0_X_OFFSET                                          0x00000D1C
#define ISP2_ST_GROUP_0_X_TYPE                                            UInt32
#define ISP2_ST_GROUP_0_X_RESERVED_MASK                                   0x00000000
#define    ISP2_ST_GROUP_0_X_X1_SHIFT                                     16
#define    ISP2_ST_GROUP_0_X_X1_MASK                                      0xFFFF0000
#define    ISP2_ST_GROUP_0_X_X0_SHIFT                                     0
#define    ISP2_ST_GROUP_0_X_X0_MASK                                      0x0000FFFF

#define ISP2_ST_GROUP_0_Y_OFFSET                                          0x00000D40
#define ISP2_ST_GROUP_0_Y_TYPE                                            UInt32
#define ISP2_ST_GROUP_0_Y_RESERVED_MASK                                   0x00000000
#define    ISP2_ST_GROUP_0_Y_Y1_SHIFT                                     16
#define    ISP2_ST_GROUP_0_Y_Y1_MASK                                      0xFFFF0000
#define    ISP2_ST_GROUP_0_Y_Y0_SHIFT                                     0
#define    ISP2_ST_GROUP_0_Y_Y0_MASK                                      0x0000FFFF

#define ISP2_ST_GRP0_CTRL_OFFSET                                          0x00000D64
#define ISP2_ST_GRP0_CTRL_TYPE                                            UInt32
#define ISP2_ST_GRP0_CTRL_RESERVED_MASK                                   0xFFFFFFC0
#define    ISP2_ST_GRP0_CTRL_PITCH_SHIFT                                  0
#define    ISP2_ST_GRP0_CTRL_PITCH_MASK                                   0x0000003F

#define ISP2_HO_CTRL_OFFSET                                               0x00000D70
#define ISP2_HO_CTRL_TYPE                                                 UInt32
#define ISP2_HO_CTRL_RESERVED_MASK                                        0xFCE1A0FF
#define    ISP2_HO_CTRL_COL_WIDTH_SHIFT                                   24
#define    ISP2_HO_CTRL_COL_WIDTH_MASK                                    0x03000000
#define    ISP2_HO_CTRL_SHIFT_SHIFT                                       17
#define    ISP2_HO_CTRL_SHIFT_MASK                                        0x001E0000
#define    ISP2_HO_CTRL_VFLIP_SHIFT                                       14
#define    ISP2_HO_CTRL_VFLIP_MASK                                        0x00004000
#define    ISP2_HO_CTRL_COL_MODE_SHIFT                                    12
#define    ISP2_HO_CTRL_COL_MODE_MASK                                     0x00001000
#define    ISP2_HO_CTRL_DATA_FORMAT_SHIFT                                 9
#define    ISP2_HO_CTRL_DATA_FORMAT_MASK                                  0x00000E00
#define    ISP2_HO_CTRL_OUTPUT_WIDTH_SHIFT                                8
#define    ISP2_HO_CTRL_OUTPUT_WIDTH_MASK                                 0x00000100

#define ISP2_HO_COL_STRIDE1_OFFSET                                        0x00000D74
#define ISP2_HO_COL_STRIDE1_TYPE                                          UInt32
#define ISP2_HO_COL_STRIDE1_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_HO_COL_STRIDE1_STRIDE_SHIFT                               9
#define    ISP2_HO_COL_STRIDE1_STRIDE_MASK                                0x01FFFE00

#define ISP2_HO_COL_STRIDE2_OFFSET                                        0x00000D78
#define ISP2_HO_COL_STRIDE2_TYPE                                          UInt32
#define ISP2_HO_COL_STRIDE2_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_HO_COL_STRIDE2_STRIDE_SHIFT                               9
#define    ISP2_HO_COL_STRIDE2_STRIDE_MASK                                0x01FFFE00

#define ISP2_HO_ADDRESS1_OFFSET                                           0x00000D7C
#define ISP2_HO_ADDRESS1_TYPE                                             UInt32
#define ISP2_HO_ADDRESS1_RESERVED_MASK                                    0x0000001F
#define    ISP2_HO_ADDRESS1_ADDRESS_SHIFT                                 5
#define    ISP2_HO_ADDRESS1_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_HO_ADDRESS2_OFFSET                                           0x00000D80
#define ISP2_HO_ADDRESS2_TYPE                                             UInt32
#define ISP2_HO_ADDRESS2_RESERVED_MASK                                    0x0000001F
#define    ISP2_HO_ADDRESS2_ADDRESS_SHIFT                                 5
#define    ISP2_HO_ADDRESS2_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_HO_ADDRESS3_OFFSET                                           0x00000D84
#define ISP2_HO_ADDRESS3_TYPE                                             UInt32
#define ISP2_HO_ADDRESS3_RESERVED_MASK                                    0x0000001F
#define    ISP2_HO_ADDRESS3_ADDRESS_SHIFT                                 5
#define    ISP2_HO_ADDRESS3_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_HO_STRIDE1_OFFSET                                            0x00000D88
#define ISP2_HO_STRIDE1_TYPE                                              UInt32
#define ISP2_HO_STRIDE1_RESERVED_MASK                                     0xFFE0001F
#define    ISP2_HO_STRIDE1_STRIDE_SHIFT                                   5
#define    ISP2_HO_STRIDE1_STRIDE_MASK                                    0x001FFFE0

#define ISP2_HO_STRIDE2_OFFSET                                            0x00000D8C
#define ISP2_HO_STRIDE2_TYPE                                              UInt32
#define ISP2_HO_STRIDE2_RESERVED_MASK                                     0xFFE0000F
#define    ISP2_HO_STRIDE2_STRIDE_SHIFT                                   4
#define    ISP2_HO_STRIDE2_STRIDE_MASK                                    0x001FFFF0

#define ISP2_LO_CTRL_OFFSET                                               0x00000DC0
#define ISP2_LO_CTRL_TYPE                                                 UInt32
#define ISP2_LO_CTRL_RESERVED_MASK                                        0xFCE1A0FF
#define    ISP2_LO_CTRL_COL_WIDTH_SHIFT                                   24
#define    ISP2_LO_CTRL_COL_WIDTH_MASK                                    0x03000000
#define    ISP2_LO_CTRL_SHIFT_SHIFT                                       17
#define    ISP2_LO_CTRL_SHIFT_MASK                                        0x001E0000
#define    ISP2_LO_CTRL_VFLIP_SHIFT                                       14
#define    ISP2_LO_CTRL_VFLIP_MASK                                        0x00004000
#define    ISP2_LO_CTRL_COL_MODE_SHIFT                                    12
#define    ISP2_LO_CTRL_COL_MODE_MASK                                     0x00001000
#define    ISP2_LO_CTRL_DATA_FORMAT_SHIFT                                 9
#define    ISP2_LO_CTRL_DATA_FORMAT_MASK                                  0x00000E00
#define    ISP2_LO_CTRL_OUTPUT_WIDTH_SHIFT                                8
#define    ISP2_LO_CTRL_OUTPUT_WIDTH_MASK                                 0x00000100

#define ISP2_LO_COL_STRIDE1_OFFSET                                        0x00000DC4
#define ISP2_LO_COL_STRIDE1_TYPE                                          UInt32
#define ISP2_LO_COL_STRIDE1_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_LO_COL_STRIDE1_STRIDE_SHIFT                               9
#define    ISP2_LO_COL_STRIDE1_STRIDE_MASK                                0x01FFFE00

#define ISP2_LO_COL_STRIDE2_OFFSET                                        0x00000DC8
#define ISP2_LO_COL_STRIDE2_TYPE                                          UInt32
#define ISP2_LO_COL_STRIDE2_RESERVED_MASK                                 0xFE0001FF
#define    ISP2_LO_COL_STRIDE2_STRIDE_SHIFT                               9
#define    ISP2_LO_COL_STRIDE2_STRIDE_MASK                                0x01FFFE00

#define ISP2_LO_ADDRESS1_OFFSET                                           0x00000DCC
#define ISP2_LO_ADDRESS1_TYPE                                             UInt32
#define ISP2_LO_ADDRESS1_RESERVED_MASK                                    0x0000001F
#define    ISP2_LO_ADDRESS1_ADDRESS_SHIFT                                 5
#define    ISP2_LO_ADDRESS1_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_LO_ADDRESS2_OFFSET                                           0x00000DD0
#define ISP2_LO_ADDRESS2_TYPE                                             UInt32
#define ISP2_LO_ADDRESS2_RESERVED_MASK                                    0x0000001F
#define    ISP2_LO_ADDRESS2_ADDRESS_SHIFT                                 5
#define    ISP2_LO_ADDRESS2_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_LO_ADDRESS3_OFFSET                                           0x00000DD4
#define ISP2_LO_ADDRESS3_TYPE                                             UInt32
#define ISP2_LO_ADDRESS3_RESERVED_MASK                                    0x0000001F
#define    ISP2_LO_ADDRESS3_ADDRESS_SHIFT                                 5
#define    ISP2_LO_ADDRESS3_ADDRESS_MASK                                  0xFFFFFFE0

#define ISP2_LO_STRIDE1_OFFSET                                            0x00000DD8
#define ISP2_LO_STRIDE1_TYPE                                              UInt32
#define ISP2_LO_STRIDE1_RESERVED_MASK                                     0xFFE0001F
#define    ISP2_LO_STRIDE1_STRIDE_SHIFT                                   5
#define    ISP2_LO_STRIDE1_STRIDE_MASK                                    0x001FFFE0

#define ISP2_LO_STRIDE2_OFFSET                                            0x00000DDC
#define ISP2_LO_STRIDE2_TYPE                                              UInt32
#define ISP2_LO_STRIDE2_RESERVED_MASK                                     0xFFE0000F
#define    ISP2_LO_STRIDE2_STRIDE_SHIFT                                   4
#define    ISP2_LO_STRIDE2_STRIDE_MASK                                    0x001FFFF0

#define ISP2_LS_CV_OFFSET                                                 0x00001000
#define ISP2_LS_CV_TYPE                                                   UInt64
#define ISP2_LS_CV_RESERVED_MASK                                          0xF000F000F000F
#define    ISP2_LS_CV_GB_SHIFT                                            52
#define    ISP2_LS_CV_GB_MASK                                             0xFFF0000000000000
#define    ISP2_LS_CV_B_SHIFT                                             36
#define    ISP2_LS_CV_B_MASK                                              0xFFF000000000
#define    ISP2_LS_CV_GR_SHIFT                                            20
#define    ISP2_LS_CV_GR_MASK                                             0xFFF00000
#define    ISP2_LS_CV_R_SHIFT                                             4
#define    ISP2_LS_CV_R_MASK                                              0x0000FFF0

#define ISP2_GM_SLOPE_R_OFFSET                                            0x00003300
#define ISP2_GM_SLOPE_R_TYPE                                              UInt32
#define ISP2_GM_SLOPE_R_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_GM_SLOPE_R_SLOPE_SHIFT                                    0
#define    ISP2_GM_SLOPE_R_SLOPE_MASK                                     0x0001FFFF

#define ISP2_GM_SLOPE_G_OFFSET                                            0x00003380
#define ISP2_GM_SLOPE_G_TYPE                                              UInt32
#define ISP2_GM_SLOPE_G_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_GM_SLOPE_G_SLOPE_SHIFT                                    0
#define    ISP2_GM_SLOPE_G_SLOPE_MASK                                     0x0001FFFF

#define ISP2_GM_SLOPE_B_OFFSET                                            0x00003400
#define ISP2_GM_SLOPE_B_TYPE                                              UInt32
#define ISP2_GM_SLOPE_B_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_GM_SLOPE_B_SLOPE_SHIFT                                    0
#define    ISP2_GM_SLOPE_B_SLOPE_MASK                                     0x0001FFFF

#define ISP2_TM_Y_ABSC_OFFSET                                             0x00003500
#define ISP2_TM_Y_ABSC_TYPE                                               UInt32
#define ISP2_TM_Y_ABSC_RESERVED_MASK                                      0x00000000
#define    ISP2_TM_Y_ABSC_ABSCISSA1_SHIFT                                 16
#define    ISP2_TM_Y_ABSC_ABSCISSA1_MASK                                  0xFFFF0000
#define    ISP2_TM_Y_ABSC_ABSCISSA0_SHIFT                                 0
#define    ISP2_TM_Y_ABSC_ABSCISSA0_MASK                                  0x0000FFFF

#define ISP2_CP_CB_ABSC_OFFSET                                            0x00003540
#define ISP2_CP_CB_ABSC_TYPE                                              UInt32
#define ISP2_CP_CB_ABSC_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_CP_CB_ABSC_ABSCISSA0_SHIFT                                0
#define    ISP2_CP_CB_ABSC_ABSCISSA0_MASK                                 0x0001FFFF

#define ISP2_CP_CR_ABSC_OFFSET                                            0x00003560
#define ISP2_CP_CR_ABSC_TYPE                                              UInt32
#define ISP2_CP_CR_ABSC_RESERVED_MASK                                     0xFFFE0000
#define    ISP2_CP_CR_ABSC_ABSCISSA0_SHIFT                                0
#define    ISP2_CP_CR_ABSC_ABSCISSA0_MASK                                 0x0001FFFF

#define ISP2_CP_CB_ORD_SLOPE_OFFSET                                       0x000035C0
#define ISP2_CP_CB_ORD_SLOPE_TYPE                                         UInt32
#define ISP2_CP_CB_ORD_SLOPE_RESERVED_MASK                                0x00000000
#define    ISP2_CP_CB_ORD_SLOPE_ORDINATE_SHIFT                            16
#define    ISP2_CP_CB_ORD_SLOPE_ORDINATE_MASK                             0xFFFF0000
#define    ISP2_CP_CB_ORD_SLOPE_SLOPE_SHIFT                               0
#define    ISP2_CP_CB_ORD_SLOPE_SLOPE_MASK                                0x0000FFFF

#define ISP2_CP_CR_ORD_SLOPE_OFFSET                                       0x000035E0
#define ISP2_CP_CR_ORD_SLOPE_TYPE                                         UInt32
#define ISP2_CP_CR_ORD_SLOPE_RESERVED_MASK                                0x00000000
#define    ISP2_CP_CR_ORD_SLOPE_ORDINATE_SHIFT                            16
#define    ISP2_CP_CR_ORD_SLOPE_ORDINATE_MASK                             0xFFFF0000
#define    ISP2_CP_CR_ORD_SLOPE_SLOPE_SHIFT                               0
#define    ISP2_CP_CR_ORD_SLOPE_SLOPE_MASK                                0x0000FFFF

#define ISP2_TM_Y_ORD_SLOPE_OFFSET                                        0x00003600
#define ISP2_TM_Y_ORD_SLOPE_TYPE                                          UInt32
#define ISP2_TM_Y_ORD_SLOPE_RESERVED_MASK                                 0x00000000
#define    ISP2_TM_Y_ORD_SLOPE_ORDINATE_SHIFT                             16
#define    ISP2_TM_Y_ORD_SLOPE_ORDINATE_MASK                              0xFFFF0000
#define    ISP2_TM_Y_ORD_SLOPE_SLOPE_SHIFT                                0
#define    ISP2_TM_Y_ORD_SLOPE_SLOPE_MASK                                 0x0000FFFF

#define ISP2_SH_VGAIN_ABSC_OFFSET                                         0x00003800
#define ISP2_SH_VGAIN_ABSC_TYPE                                           UInt32
#define ISP2_SH_VGAIN_ABSC_RESERVED_MASK                                  0x00000000
#define    ISP2_SH_VGAIN_ABSC_ABSCISSA1_SHIFT                             16
#define    ISP2_SH_VGAIN_ABSC_ABSCISSA1_MASK                              0xFFFF0000
#define    ISP2_SH_VGAIN_ABSC_ABSCISSA0_SHIFT                             0
#define    ISP2_SH_VGAIN_ABSC_ABSCISSA0_MASK                              0x0000FFFF

#define ISP2_SH_VGAIN_ORD_OFFSET                                          0x00003810
#define ISP2_SH_VGAIN_ORD_TYPE                                            UInt32
#define ISP2_SH_VGAIN_ORD_RESERVED_MASK                                   0x00000000
#define    ISP2_SH_VGAIN_ORD_ORDINATE1_SHIFT                              16
#define    ISP2_SH_VGAIN_ORD_ORDINATE1_MASK                               0xFFFF0000
#define    ISP2_SH_VGAIN_ORD_ORDINATE0_SHIFT                              0
#define    ISP2_SH_VGAIN_ORD_ORDINATE0_MASK                               0x0000FFFF

#define ISP2_SH_VGAIN_SLOPE_OFFSET                                        0x00003820
#define ISP2_SH_VGAIN_SLOPE_TYPE                                          UInt32
#define ISP2_SH_VGAIN_SLOPE_RESERVED_MASK                                 0x00000000
#define    ISP2_SH_VGAIN_SLOPE_SLOPE1_SHIFT                               16
#define    ISP2_SH_VGAIN_SLOPE_SLOPE1_MASK                                0xFFFF0000
#define    ISP2_SH_VGAIN_SLOPE_SLOPE0_SHIFT                               0
#define    ISP2_SH_VGAIN_SLOPE_SLOPE0_MASK                                0x0000FFFF

#define ISP2_SH_POSGAIN_ABSC_OFFSET                                       0x00003830
#define ISP2_SH_POSGAIN_ABSC_TYPE                                         UInt32
#define ISP2_SH_POSGAIN_ABSC_RESERVED_MASK                                0x00000000
#define    ISP2_SH_POSGAIN_ABSC_ABSCISSA1_SHIFT                           16
#define    ISP2_SH_POSGAIN_ABSC_ABSCISSA1_MASK                            0xFFFF0000
#define    ISP2_SH_POSGAIN_ABSC_ABSCISSA0_SHIFT                           0
#define    ISP2_SH_POSGAIN_ABSC_ABSCISSA0_MASK                            0x0000FFFF

#define ISP2_SH_POSGAIN_ORD_OFFSET                                        0x00003840
#define ISP2_SH_POSGAIN_ORD_TYPE                                          UInt32
#define ISP2_SH_POSGAIN_ORD_RESERVED_MASK                                 0x00000000
#define    ISP2_SH_POSGAIN_ORD_ORDINATE1_SHIFT                            16
#define    ISP2_SH_POSGAIN_ORD_ORDINATE1_MASK                             0xFFFF0000
#define    ISP2_SH_POSGAIN_ORD_ORDINATE0_SHIFT                            0
#define    ISP2_SH_POSGAIN_ORD_ORDINATE0_MASK                             0x0000FFFF

#define ISP2_SH_POSGAIN_SLOPE_OFFSET                                      0x00003850
#define ISP2_SH_POSGAIN_SLOPE_TYPE                                        UInt32
#define ISP2_SH_POSGAIN_SLOPE_RESERVED_MASK                               0x00000000
#define    ISP2_SH_POSGAIN_SLOPE_SLOPE1_SHIFT                             16
#define    ISP2_SH_POSGAIN_SLOPE_SLOPE1_MASK                              0xFFFF0000
#define    ISP2_SH_POSGAIN_SLOPE_SLOPE0_SHIFT                             0
#define    ISP2_SH_POSGAIN_SLOPE_SLOPE0_MASK                              0x0000FFFF

#define ISP2_SH_RESPGAIN_ABSC_OFFSET                                      0x00003860
#define ISP2_SH_RESPGAIN_ABSC_TYPE                                        UInt32
#define ISP2_SH_RESPGAIN_ABSC_RESERVED_MASK                               0x00000000
#define    ISP2_SH_RESPGAIN_ABSC_ABSCISSA1_SHIFT                          16
#define    ISP2_SH_RESPGAIN_ABSC_ABSCISSA1_MASK                           0xFFFF0000
#define    ISP2_SH_RESPGAIN_ABSC_ABSCISSA0_SHIFT                          0
#define    ISP2_SH_RESPGAIN_ABSC_ABSCISSA0_MASK                           0x0000FFFF

#define ISP2_SH_RESPGAIN_ORD_OFFSET                                       0x00003880
#define ISP2_SH_RESPGAIN_ORD_TYPE                                         UInt32
#define ISP2_SH_RESPGAIN_ORD_RESERVED_MASK                                0x00000000
#define    ISP2_SH_RESPGAIN_ORD_ORDINATE1_SHIFT                           16
#define    ISP2_SH_RESPGAIN_ORD_ORDINATE1_MASK                            0xFFFF0000
#define    ISP2_SH_RESPGAIN_ORD_ORDINATE0_SHIFT                           0
#define    ISP2_SH_RESPGAIN_ORD_ORDINATE0_MASK                            0x0000FFFF

#define ISP2_SH_RESPGAIN_SLOPE_OFFSET                                     0x000038A0
#define ISP2_SH_RESPGAIN_SLOPE_TYPE                                       UInt32
#define ISP2_SH_RESPGAIN_SLOPE_RESERVED_MASK                              0x00000000
#define    ISP2_SH_RESPGAIN_SLOPE_SLOPE1_SHIFT                            16
#define    ISP2_SH_RESPGAIN_SLOPE_SLOPE1_MASK                             0xFFFF0000
#define    ISP2_SH_RESPGAIN_SLOPE_SLOPE0_SHIFT                            0
#define    ISP2_SH_RESPGAIN_SLOPE_SLOPE0_MASK                             0x0000FFFF

#define ISP2_SH_RESP_ABSC_OFFSET                                          0x000038C0
#define ISP2_SH_RESP_ABSC_TYPE                                            UInt32
#define ISP2_SH_RESP_ABSC_RESERVED_MASK                                   0x00000000
#define    ISP2_SH_RESP_ABSC_ABSCISSA1_SHIFT                              16
#define    ISP2_SH_RESP_ABSC_ABSCISSA1_MASK                               0xFFFF0000
#define    ISP2_SH_RESP_ABSC_ABSCISSA0_SHIFT                              0
#define    ISP2_SH_RESP_ABSC_ABSCISSA0_MASK                               0x0000FFFF

#define ISP2_SH_RESP_ORD_OFFSET                                           0x000038D8
#define ISP2_SH_RESP_ORD_TYPE                                             UInt32
#define ISP2_SH_RESP_ORD_RESERVED_MASK                                    0x00000000
#define    ISP2_SH_RESP_ORD_ORDINATE1_SHIFT                               16
#define    ISP2_SH_RESP_ORD_ORDINATE1_MASK                                0xFFFF0000
#define    ISP2_SH_RESP_ORD_ORDINATE0_SHIFT                               0
#define    ISP2_SH_RESP_ORD_ORDINATE0_MASK                                0x0000FFFF

#define ISP2_SH_RESP_SLOPE_OFFSET                                         0x000038F0
#define ISP2_SH_RESP_SLOPE_TYPE                                           UInt32
#define ISP2_SH_RESP_SLOPE_RESERVED_MASK                                  0x00000000
#define    ISP2_SH_RESP_SLOPE_SLOPE1_SHIFT                                16
#define    ISP2_SH_RESP_SLOPE_SLOPE1_MASK                                 0xFFFF0000
#define    ISP2_SH_RESP_SLOPE_SLOPE0_SHIFT                                0
#define    ISP2_SH_RESP_SLOPE_SLOPE0_MASK                                 0x0000FFFF

#define ISP2_SH_RESP1_ABSC_OFFSET                                         0x00003908
#define ISP2_SH_RESP1_ABSC_TYPE                                           UInt32
#define ISP2_SH_RESP1_ABSC_RESERVED_MASK                                  0x00000000
#define    ISP2_SH_RESP1_ABSC_ABSCISSA1_SHIFT                             16
#define    ISP2_SH_RESP1_ABSC_ABSCISSA1_MASK                              0xFFFF0000
#define    ISP2_SH_RESP1_ABSC_ABSCISSA0_SHIFT                             0
#define    ISP2_SH_RESP1_ABSC_ABSCISSA0_MASK                              0x0000FFFF

#define ISP2_SH_RESP1_ORD_OFFSET                                          0x00003920
#define ISP2_SH_RESP1_ORD_TYPE                                            UInt32
#define ISP2_SH_RESP1_ORD_RESERVED_MASK                                   0x00000000
#define    ISP2_SH_RESP1_ORD_ORDINATE1_SHIFT                              16
#define    ISP2_SH_RESP1_ORD_ORDINATE1_MASK                               0xFFFF0000
#define    ISP2_SH_RESP1_ORD_ORDINATE0_SHIFT                              0
#define    ISP2_SH_RESP1_ORD_ORDINATE0_MASK                               0x0000FFFF

#define ISP2_SH_RESP1_SLOPE_OFFSET                                        0x00003938
#define ISP2_SH_RESP1_SLOPE_TYPE                                          UInt32
#define ISP2_SH_RESP1_SLOPE_RESERVED_MASK                                 0x00000000
#define    ISP2_SH_RESP1_SLOPE_SLOPE1_SHIFT                               16
#define    ISP2_SH_RESP1_SLOPE_SLOPE1_MASK                                0xFFFF0000
#define    ISP2_SH_RESP1_SLOPE_SLOPE0_SHIFT                               0
#define    ISP2_SH_RESP1_SLOPE_SLOPE0_MASK                                0x0000FFFF

#define ISP2_GM_ORD_R_OFFSET                                              0x00003A00
#define ISP2_GM_ORD_R_TYPE                                                UInt32
#define ISP2_GM_ORD_R_RESERVED_MASK                                       0x00000000
#define    ISP2_GM_ORD_R_ORDINATE1_SHIFT                                  16
#define    ISP2_GM_ORD_R_ORDINATE1_MASK                                   0xFFFF0000
#define    ISP2_GM_ORD_R_ORDINATE0_SHIFT                                  0
#define    ISP2_GM_ORD_R_ORDINATE0_MASK                                   0x0000FFFF

#define ISP2_GM_ORD_G_OFFSET                                              0x00003A40
#define ISP2_GM_ORD_G_TYPE                                                UInt32
#define ISP2_GM_ORD_G_RESERVED_MASK                                       0x00000000
#define    ISP2_GM_ORD_G_ORDINATE1_SHIFT                                  16
#define    ISP2_GM_ORD_G_ORDINATE1_MASK                                   0xFFFF0000
#define    ISP2_GM_ORD_G_ORDINATE0_SHIFT                                  0
#define    ISP2_GM_ORD_G_ORDINATE0_MASK                                   0x0000FFFF

#define ISP2_GM_ORD_B_OFFSET                                              0x00003A80
#define ISP2_GM_ORD_B_TYPE                                                UInt32
#define ISP2_GM_ORD_B_RESERVED_MASK                                       0x00000000
#define    ISP2_GM_ORD_B_ORDINATE1_SHIFT                                  16
#define    ISP2_GM_ORD_B_ORDINATE1_MASK                                   0xFFFF0000
#define    ISP2_GM_ORD_B_ORDINATE0_SHIFT                                  0
#define    ISP2_GM_ORD_B_ORDINATE0_MASK                                   0x0000FFFF

#define ISP2_DP_CLUSTER_OFFSET                                            0x00003C00
#define ISP2_DP_CLUSTER_TYPE                                              UInt64
#define ISP2_DP_CLUSTER_RESERVED_MASK                                     0xFFFF000000000000
#define    ISP2_DP_CLUSTER_MASK_SHIFT                                     32
#define    ISP2_DP_CLUSTER_MASK_MASK                                      0xFFFF00000000
#define    ISP2_DP_CLUSTER_Y_SHIFT                                        16
#define    ISP2_DP_CLUSTER_Y_MASK                                         0xFFFF0000
#define    ISP2_DP_CLUSTER_X_SHIFT                                        0
#define    ISP2_DP_CLUSTER_X_MASK                                         0x0000FFFF

#define ISP2_GD_CV_OFFSET                                                 0x00004000
#define ISP2_GD_CV_TYPE                                                   UInt32
#define ISP2_GD_CV_RESERVED_MASK                                          0x00000000
#define    ISP2_GD_CV_Y_SHIFT                                             16
#define    ISP2_GD_CV_Y_MASK                                              0xFFFF0000
#define    ISP2_GD_CV_X_SHIFT                                             0
#define    ISP2_GD_CV_X_MASK                                              0x0000FFFF

#endif /* __BRCM_RDB_ISP2_H__ */


