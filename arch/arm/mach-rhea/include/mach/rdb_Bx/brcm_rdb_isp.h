/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009  Broadcom Corporation                                                        */
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

#ifndef __BRCM_RDB_ISP_H__
#define __BRCM_RDB_ISP_H__

#define ISP_CTRL_OFFSET                                                   0x00000000
#define ISP_CTRL_TYPE                                                     UInt32
#define ISP_CTRL_RESERVED_MASK                                            0xFFEEFE80
#define    ISP_CTRL_FORCE_CLKEN_SHIFT                                     20
#define    ISP_CTRL_FORCE_CLKEN_MASK                                      0x00100000
#define    ISP_CTRL_TRANSFER_SHIFT                                        16
#define    ISP_CTRL_TRANSFER_MASK                                         0x00010000
#define    ISP_CTRL_STATS_IMASK_SHIFT                                     8
#define    ISP_CTRL_STATS_IMASK_MASK                                      0x00000100
#define    ISP_CTRL_SW_IMASK_SHIFT                                        6
#define    ISP_CTRL_SW_IMASK_MASK                                         0x00000040
#define    ISP_CTRL_EOD_IMASK_SHIFT                                       5
#define    ISP_CTRL_EOD_IMASK_MASK                                        0x00000020
#define    ISP_CTRL_EOT_IMASK_SHIFT                                       4
#define    ISP_CTRL_EOT_IMASK_MASK                                        0x00000010
#define    ISP_CTRL_ERROR_IMASK_SHIFT                                     3
#define    ISP_CTRL_ERROR_IMASK_MASK                                      0x00000008
#define    ISP_CTRL_CLR_STATS_SHIFT                                       2
#define    ISP_CTRL_CLR_STATS_MASK                                        0x00000004
#define    ISP_CTRL_FLUSH_SHIFT                                           1
#define    ISP_CTRL_FLUSH_MASK                                            0x00000002
#define    ISP_CTRL_ENABLE_SHIFT                                          0
#define    ISP_CTRL_ENABLE_MASK                                           0x00000001

#define ISP_STATUS_OFFSET                                                 0x00000004
#define ISP_STATUS_TYPE                                                   UInt32
#define ISP_STATUS_RESERVED_MASK                                          0xFFFFFE84
#define    ISP_STATUS_STATS_INT_SHIFT                                     8
#define    ISP_STATUS_STATS_INT_MASK                                      0x00000100
#define    ISP_STATUS_SW_INT_SHIFT                                        6
#define    ISP_STATUS_SW_INT_MASK                                         0x00000040
#define    ISP_STATUS_EOD_INT_SHIFT                                       5
#define    ISP_STATUS_EOD_INT_MASK                                        0x00000020
#define    ISP_STATUS_EOT_INT_SHIFT                                       4
#define    ISP_STATUS_EOT_INT_MASK                                        0x00000010
#define    ISP_STATUS_ERROR_INT_SHIFT                                     3
#define    ISP_STATUS_ERROR_INT_MASK                                      0x00000008
#define    ISP_STATUS_STATE_SHIFT                                         0
#define    ISP_STATUS_STATE_MASK                                          0x00000003
#define       ISP_STATUS_STATE_INACTIVE                                   0
#define       ISP_STATUS_STATE_ENABLED                                    1
#define       ISP_STATUS_STATE_SUSPENDED                                  2
#define       ISP_STATUS_STATE_SUSPENDING                                 3

#define ISP_ID_OFFSET                                                     0x00000008
#define ISP_ID_TYPE                                                       UInt32
#define ISP_ID_RESERVED_MASK                                              0x00000000
#define    ISP_ID_SIGNATURE_SHIFT                                         16
#define    ISP_ID_SIGNATURE_MASK                                          0xFFFF0000
#define    ISP_ID_VERSION_SHIFT                                           8
#define    ISP_ID_VERSION_MASK                                            0x0000FF00
#define    ISP_ID_REVISION_SHIFT                                          0
#define    ISP_ID_REVISION_MASK                                           0x000000FF

#define ISP_SYSTEM_OFFSET                                                 0x0000000C
#define ISP_SYSTEM_TYPE                                                   UInt32
#define ISP_SYSTEM_RESERVED_MASK                                          0xFFFFFCFC
#define    ISP_SYSTEM_PRIORITY_SHIFT                                      8
#define    ISP_SYSTEM_PRIORITY_MASK                                       0x00000300
#define    ISP_SYSTEM_MIN_PROT_SHIFT                                      0
#define    ISP_SYSTEM_MIN_PROT_MASK                                       0x00000003
#define       ISP_SYSTEM_MIN_PROT_RESERVED0                               0
#define       ISP_SYSTEM_MIN_PROT_PRIVILEDGED                             1
#define       ISP_SYSTEM_MIN_PROT_SECURE                                  2
#define       ISP_SYSTEM_MIN_PROT_RESERVED1                               3

#define ISP_TILE_CTRL_OFFSET                                              0x00000010
#define ISP_TILE_CTRL_TYPE                                                UInt32
#define ISP_TILE_CTRL_RESERVED_MASK                                       0xFFFFF000
#define    ISP_TILE_CTRL_TILE_COUNT_SHIFT                                 0
#define    ISP_TILE_CTRL_TILE_COUNT_MASK                                  0x00000FFF

#define ISP_STATS_ADDR_OFFSET                                             0x00000014
#define ISP_STATS_ADDR_TYPE                                               UInt32
#define ISP_STATS_ADDR_RESERVED_MASK                                      0xFFFFFFF0
#define    ISP_STATS_ADDR_ADDRESS_SHIFT                                   0
#define    ISP_STATS_ADDR_ADDRESS_MASK                                    0x0000000F

#define ISP_TILE_STATUS_OFFSET                                            0x00000018
#define ISP_TILE_STATUS_TYPE                                              UInt32
#define ISP_TILE_STATUS_RESERVED_MASK                                     0xFFFFF000
#define    ISP_TILE_STATUS_INT_COUNT_SHIFT                                0
#define    ISP_TILE_STATUS_INT_COUNT_MASK                                 0x00000FFF

#define ISP_TILE_ADDR_OFFSET                                              0x0000001C
#define ISP_TILE_ADDR_TYPE                                                UInt32
#define ISP_TILE_ADDR_RESERVED_MASK                                       0x0000001F
#define    ISP_TILE_ADDR_ADDRESS_SHIFT                                    0
#define    ISP_TILE_ADDR_ADDRESS_MASK                                     0xFFFFFFE0

#define ISP_DESC_ADDR_OFFSET                                              0x00000020
#define ISP_DESC_ADDR_TYPE                                                UInt32
#define ISP_DESC_ADDR_RESERVED_MASK                                       0xFFFFFFF0
#define    ISP_DESC_ADDR_ADDRESS_SHIFT                                    0
#define    ISP_DESC_ADDR_ADDRESS_MASK                                     0x0000000F

#define ISP_DESC_CTRL_OFFSET                                              0x00000024
#define ISP_DESC_CTRL_TYPE                                                UInt32
#define ISP_DESC_CTRL_RESERVED_MASK                                       0xFFFFF000
#define    ISP_DESC_CTRL_DESC_COUNT_SHIFT                                 0
#define    ISP_DESC_CTRL_DESC_COUNT_MASK                                  0x00000FFF

#define ISP_DESC_STATUS_OFFSET                                            0x00000028
#define ISP_DESC_STATUS_TYPE                                              UInt32
#define ISP_DESC_STATUS_RESERVED_MASK                                     0xFFFFF000
#define    ISP_DESC_STATUS_INT_COUNT_SHIFT                                0
#define    ISP_DESC_STATUS_INT_COUNT_MASK                                 0x00000FFF

#define ISPFR_SIZE_OFFSET                                                 0x00000030
#define ISPFR_SIZE_TYPE                                                   UInt32
#define ISPFR_SIZE_RESERVED_MASK                                          0x00000000
#define    ISPFR_SIZE_HEIGHT_SHIFT                                        16
#define    ISPFR_SIZE_HEIGHT_MASK                                         0xFFFF0000
#define    ISPFR_SIZE_WIDTH_SHIFT                                         0
#define    ISPFR_SIZE_WIDTH_MASK                                          0x0000FFFF

#define ISPFR_CTRL_OFFSET                                                 0x00000034
#define ISPFR_CTRL_TYPE                                                   UInt32
#define ISPFR_CTRL_RESERVED_MASK                                          0xFFFF00F8
#define    ISPFR_CTRL_SW_IN_POS_SHIFT                                     12
#define    ISPFR_CTRL_SW_IN_POS_MASK                                      0x0000F000
#define    ISPFR_CTRL_SW_OUT_POS_SHIFT                                    8
#define    ISPFR_CTRL_SW_OUT_POS_MASK                                     0x00000F00
#define    ISPFR_CTRL_TRANSPOSED_SHIFT                                    2
#define    ISPFR_CTRL_TRANSPOSED_MASK                                     0x00000004
#define    ISPFR_CTRL_BAYER_ORDER_SHIFT                                   0
#define    ISPFR_CTRL_BAYER_ORDER_MASK                                    0x00000003

#define ISPFR_BAYER_EN_OFFSET                                             0x00000038
#define ISPFR_BAYER_EN_TYPE                                               UInt32
#define ISPFR_BAYER_EN_RESERVED_MASK                                      0xFFFFE000
#define    ISPFR_BAYER_EN_BS_SHIFT                                        12
#define    ISPFR_BAYER_EN_BS_MASK                                         0x00001000
#define    ISPFR_BAYER_EN_DM_SHIFT                                        11
#define    ISPFR_BAYER_EN_DM_MASK                                         0x00000800
#define    ISPFR_BAYER_EN_BD_SHIFT                                        10
#define    ISPFR_BAYER_EN_BD_MASK                                         0x00000400
#define    ISPFR_BAYER_EN_XC_SHIFT                                        9
#define    ISPFR_BAYER_EN_XC_MASK                                         0x00000200
#define    ISPFR_BAYER_EN_RS_SHIFT                                        8
#define    ISPFR_BAYER_EN_RS_MASK                                         0x00000100
#define    ISPFR_BAYER_EN_DPA_SHIFT                                       7
#define    ISPFR_BAYER_EN_DPA_MASK                                        0x00000080
#define    ISPFR_BAYER_EN_DPP_SHIFT                                       6
#define    ISPFR_BAYER_EN_DPP_MASK                                        0x00000040
#define    ISPFR_BAYER_EN_WG_SHIFT                                        5
#define    ISPFR_BAYER_EN_WG_MASK                                         0x00000020
#define    ISPFR_BAYER_EN_ST_SHIFT                                        4
#define    ISPFR_BAYER_EN_ST_MASK                                         0x00000010
#define    ISPFR_BAYER_EN_LS_SHIFT                                        3
#define    ISPFR_BAYER_EN_LS_MASK                                         0x00000008
#define    ISPFR_BAYER_EN_BL_SHIFT                                        2
#define    ISPFR_BAYER_EN_BL_MASK                                         0x00000004
#define    ISPFR_BAYER_EN_DI_SHIFT                                        1
#define    ISPFR_BAYER_EN_DI_MASK                                         0x00000002
#define    ISPFR_BAYER_EN_II_SHIFT                                        0
#define    ISPFR_BAYER_EN_II_MASK                                         0x00000001

#define ISPFR_YCBCR_EN_OFFSET                                             0x0000003C
#define ISPFR_YCBCR_EN_TYPE                                               UInt32
#define ISPFR_YCBCR_EN_RESERVED_MASK                                      0xFFFF0000
#define    ISPFR_YCBCR_EN_TM_SHIFT                                        15
#define    ISPFR_YCBCR_EN_TM_MASK                                         0x00008000
#define    ISPFR_YCBCR_EN_YS_SHIFT                                        14
#define    ISPFR_YCBCR_EN_YS_MASK                                         0x00004000
#define    ISPFR_YCBCR_EN_LO_SHIFT                                        13
#define    ISPFR_YCBCR_EN_LO_MASK                                         0x00002000
#define    ISPFR_YCBCR_EN_HO_SHIFT                                        12
#define    ISPFR_YCBCR_EN_HO_MASK                                         0x00001000
#define    ISPFR_YCBCR_EN_LR_SHIFT                                        11
#define    ISPFR_YCBCR_EN_LR_MASK                                         0x00000800
#define    ISPFR_YCBCR_EN_CC_SHIFT                                        10
#define    ISPFR_YCBCR_EN_CC_MASK                                         0x00000400
#define    ISPFR_YCBCR_EN_HR_SHIFT                                        9
#define    ISPFR_YCBCR_EN_HR_MASK                                         0x00000200
#define    ISPFR_YCBCR_EN_CP_SHIFT                                        8
#define    ISPFR_YCBCR_EN_CP_MASK                                         0x00000100
#define    ISPFR_YCBCR_EN_GD_SHIFT                                        7
#define    ISPFR_YCBCR_EN_GD_MASK                                         0x00000080
#define    ISPFR_YCBCR_EN_SH_SHIFT                                        6
#define    ISPFR_YCBCR_EN_SH_MASK                                         0x00000040
#define    ISPFR_YCBCR_EN_FC_SHIFT                                        5
#define    ISPFR_YCBCR_EN_FC_MASK                                         0x00000020
#define    ISPFR_YCBCR_EN_YD_SHIFT                                        4
#define    ISPFR_YCBCR_EN_YD_MASK                                         0x00000010
#define    ISPFR_YCBCR_EN_YG_SHIFT                                        3
#define    ISPFR_YCBCR_EN_YG_MASK                                         0x00000008
#define    ISPFR_YCBCR_EN_GM_SHIFT                                        2
#define    ISPFR_YCBCR_EN_GM_MASK                                         0x00000004
#define    ISPFR_YCBCR_EN_YC_SHIFT                                        1
#define    ISPFR_YCBCR_EN_YC_MASK                                         0x00000002
#define    ISPFR_YCBCR_EN_YI_SHIFT                                        0
#define    ISPFR_YCBCR_EN_YI_MASK                                         0x00000001

#define ISPFR_OFF0_OFFSET                                                 0x00000040
#define ISPFR_OFF0_TYPE                                                   UInt32
#define ISPFR_OFF0_RESERVED_MASK                                          0x00000000
#define    ISPFR_OFF0_DM_SHIFT                                            24
#define    ISPFR_OFF0_DM_MASK                                             0xFF000000
#define    ISPFR_OFF0_XC_SHIFT                                            16
#define    ISPFR_OFF0_XC_MASK                                             0x00FF0000
#define    ISPFR_OFF0_RS_SHIFT                                            8
#define    ISPFR_OFF0_RS_MASK                                             0x0000FF00
#define    ISPFR_OFF0_DP_SHIFT                                            0
#define    ISPFR_OFF0_DP_MASK                                             0x000000FF

#define ISPFR_OFF1_OFFSET                                                 0x00000044
#define ISPFR_OFF1_TYPE                                                   UInt32
#define ISPFR_OFF1_RESERVED_MASK                                          0x00000000
#define    ISPFR_OFF1_ST_SHIFT                                            24
#define    ISPFR_OFF1_ST_MASK                                             0xFF000000
#define    ISPFR_OFF1_DN_SHIFT                                            16
#define    ISPFR_OFF1_DN_MASK                                             0x00FF0000
#define    ISPFR_OFF1_BL_SHIFT                                            8
#define    ISPFR_OFF1_BL_MASK                                             0x0000FF00
#define    ISPFR_OFF1_LS_SHIFT                                            0
#define    ISPFR_OFF1_LS_MASK                                             0x000000FF

#define ISPFR_OFF2_OFFSET                                                 0x00000048
#define ISPFR_OFF2_TYPE                                                   UInt32
#define ISPFR_OFF2_RESERVED_MASK                                          0xFF000000
#define    ISPFR_OFF2_HRO_SHIFT                                           16
#define    ISPFR_OFF2_HRO_MASK                                            0x00FF0000
#define    ISPFR_OFF2_FCSH_SHIFT                                          8
#define    ISPFR_OFF2_FCSH_MASK                                           0x0000FF00
#define    ISPFR_OFF2_GD_SHIFT                                            0
#define    ISPFR_OFF2_GD_MASK                                             0x000000FF

#define ISPFR_SWOFF_OFFSET                                                0x0000004C
#define ISPFR_SWOFF_TYPE                                                  UInt32
#define ISPFR_SWOFF_RESERVED_MASK                                         0xFFFF0000
#define    ISPFR_SWOFF_SW_IN_SHIFT                                        8
#define    ISPFR_SWOFF_SW_IN_MASK                                         0x0000FF00
#define    ISPFR_SWOFF_SW_OUT_SHIFT                                       0
#define    ISPFR_SWOFF_SW_OUT_MASK                                        0x000000FF

#define ISPFR_MOSAIC_OFFSET                                               0x00000050
#define ISPFR_MOSAIC_TYPE                                                 UInt32
#define ISPFR_MOSAIC_RESERVED_MASK                                        0x00000000

#define ISPFR_FIF00_OFFSET                                                0x00000060
#define ISPFR_FIF00_TYPE                                                  UInt32
#define ISPFR_FIF00_RESERVED_MASK                                         0x00000000

#define ISPFR_FIF01_OFFSET                                                0x00000064
#define ISPFR_FIF01_TYPE                                                  UInt32
#define ISPFR_FIF01_RESERVED_MASK                                         0x00000000

#define ISPWG_RED_OFFSET                                                  0x00000090
#define ISPWG_RED_TYPE                                                    UInt32
#define ISPWG_RED_RESERVED_MASK                                           0xFFFF0000
#define    ISPWG_RED_GAIN_SHIFT                                           0
#define    ISPWG_RED_GAIN_MASK                                            0x0000FFFF

#define ISPWG_BLUE_OFFSET                                                 0x00000094
#define ISPWG_BLUE_TYPE                                                   UInt32
#define ISPWG_BLUE_RESERVED_MASK                                          0xFFFF0000
#define    ISPWG_BLUE_GAIN_SHIFT                                          0
#define    ISPWG_BLUE_GAIN_MASK                                           0x0000FFFF

#define ISPWG_OFFSETG_OFFSET                                              0x00000098
#define ISPWG_OFFSETG_TYPE                                                UInt32
#define ISPWG_OFFSETG_RESERVED_MASK                                       0xFFFF0000
#define    ISPWG_OFFSETG_OFFSET_SHIFT                                     0
#define    ISPWG_OFFSETG_OFFSET_MASK                                      0x0000FFFF

#define ISPWG_GAIN_OFFSET                                                 0x0000009C
#define ISPWG_GAIN_TYPE                                                   UInt32
#define ISPWG_GAIN_RESERVED_MASK                                          0xFFFF0000
#define    ISPWG_GAIN_GAIN_SHIFT                                          0
#define    ISPWG_GAIN_GAIN_MASK                                           0x0000FFFF

#define ISPWG_THRESH_OFFSET                                               0x000000A0
#define ISPWG_THRESH_TYPE                                                 UInt32
#define ISPWG_THRESH_RESERVED_MASK                                        0xFFFF0000
#define    ISPWG_THRESH_THRESH_SHIFT                                      0
#define    ISPWG_THRESH_THRESH_MASK                                       0x0000FFFF

#define ISPWG_OFFSETR_OFFSET                                              0x000000A4
#define ISPWG_OFFSETR_TYPE                                                UInt32
#define ISPWG_OFFSETR_RESERVED_MASK                                       0x00000000

#define ISPWG_OFFSETB_OFFSET                                              0x000000A8
#define ISPWG_OFFSETB_TYPE                                                UInt32
#define ISPWG_OFFSETB_RESERVED_MASK                                       0x00000000

#define ISPYG_MATRIX_OFFSET                                               0x000000B0
#define ISPYG_MATRIX_TYPE                                                 UInt32
#define ISPYG_MATRIX_RESERVED_MASK                                        0x00000000

#define ISPYG_OFFSET_OFFSET                                               0x000000C4
#define ISPYG_OFFSET_TYPE                                                 UInt32
#define ISPYG_OFFSET_RESERVED_MASK                                        0x00000000

#define ISPYG_Y_OFFSET                                                    0x000000D0
#define ISPYG_Y_TYPE                                                      UInt32
#define ISPYG_Y_RESERVED_MASK                                             0x00000000
#define    ISPYG_Y_COEFF1_SHIFT                                           16
#define    ISPYG_Y_COEFF1_MASK                                            0xFFFF0000
#define    ISPYG_Y_COEFF0_SHIFT                                           0
#define    ISPYG_Y_COEFF0_MASK                                            0x0000FFFF

#define ISPYG_OFFSET1_OFFSET                                              0x000000C8
#define ISPYG_OFFSET1_TYPE                                                UInt32
#define ISPYG_OFFSET1_RESERVED_MASK                                       0xFFFFC000
#define    ISPYG_OFFSET1_OFFSE_SHIFT                                      0
#define    ISPYG_OFFSET1_OFFSE_MASK                                       0x00003FFF

#define ISPYG_OFFSET2_OFFSET                                              0x000000CC
#define ISPYG_OFFSET2_TYPE                                                UInt32
#define ISPYG_OFFSET2_RESERVED_MASK                                       0xFFFFC000
#define    ISPYG_OFFSET2_OFFSE_SHIFT                                      0
#define    ISPYG_OFFSET2_OFFSE_MASK                                       0x00003FFF

#define ISPYG_SCALE_OFFSET                                                0x000000D4
#define ISPYG_SCALE_TYPE                                                  UInt32
#define ISPYG_SCALE_RESERVED_MASK                                         0x00000000
#define    ISPYG_SCALE_U_SHIFT                                            16
#define    ISPYG_SCALE_U_MASK                                             0xFFFF0000
#define    ISPYG_SCALE_V_SHIFT                                            0
#define    ISPYG_SCALE_V_MASK                                             0x0000FFFF


#define ISPTD_CTRL_OFFSET                                                 0x00000440
#define ISPTD_CTRL_TYPE                                                   UInt32
#define ISPTD_CTRL_RESERVED_MASK                                          0x00000000

#define ISPTD_STATUS_OFFSET                                               0x00000444
#define ISPTD_STATUS_TYPE                                                 UInt32
#define ISPTD_STATUS_RESERVED_MASK                                        0x00000000
#define    ISPTD_STATUS_DESC_NUMBER_SHIFT                                 16
#define    ISPTD_STATUS_DESC_NUMBER_MASK                                  0xFFFF0000
#define    ISPTD_STATUS_INT_COUNT_SHIFT                                   0
#define    ISPTD_STATUS_INT_COUNT_MASK                                    0x0000FFFF

#define ISPTD_ADDR_OFFSET                                                 0x00000448
#define ISPTD_ADDR_TYPE                                                   UInt32
#define ISPTD_ADDR_RESERVED_MASK                                          0x00000000

#define ISPTD_DESC_OFFSET                                                 0x0000044C
#define ISPTD_DESC_TYPE                                                   UInt32
#define ISPTD_DESC_RESERVED_MASK                                          0x00000000

#define ISPTD_INIT0_OFFSET                                                0x0000046C
#define ISPTD_INIT0_TYPE                                                  UInt32
#define ISPTD_INIT0_RESERVED_MASK                                         0x00000000

#define ISPTD_INIT1_OFFSET                                                0x00000470
#define ISPTD_INIT1_TYPE                                                  UInt32
#define ISPTD_INIT1_RESERVED_MASK                                         0x00000000

#define ISPII_CTRL_OFFSET                                                 0x00000100
#define ISPII_CTRL_TYPE                                                   UInt32
#define ISPII_CTRL_RESERVED_MASK                                          0x000060FF
#define    ISPII_CTRL_STRIDE_SHIFT                                        15
#define    ISPII_CTRL_STRIDE_MASK                                         0xFFFF8000
#define    ISPII_CTRL_FORMAT_SHIFT                                        8
#define    ISPII_CTRL_FORMAT_MASK                                         0x00001F00
#define       ISPII_CTRL_FORMAT_RAW8                                      2
#define       ISPII_CTRL_FORMAT_RAW10                                     3
#define       ISPII_CTRL_FORMAT_RAW12                                     4
#define       ISPII_CTRL_FORMAT_RAW14                                     5
#define       ISPII_CTRL_FORMAT_RAW16                                     6
#define       ISPII_CTRL_FORMAT_RAW10_8                                   7
#define       ISPII_CTRL_FORMAT_RAW12_8                                   8

#define ISPII_ADDRESS_OFFSET                                              0x00000104
#define ISPII_ADDRESS_TYPE                                                UInt32
#define ISPII_ADDRESS_RESERVED_MASK                                       0x0000001F

#define ISPII_ENDADDR_OFFSET                                              0x00000108
#define ISPII_ENDADDR_TYPE                                                UInt32
#define ISPII_ENDADDR_RESERVED_MASK                                       0x0000001F

#define ISPII_DPCM_OFFSET                                                 0x0000010C
#define ISPII_DPCM_TYPE                                                   UInt32
#define ISPII_DPCM_RESERVED_MASK                                          0xFFFFFC00
#define    ISPII_DPCM_ADV_SHIFT                                           9
#define    ISPII_DPCM_ADV_MASK                                            0x00000200
#define    ISPII_DPCM_BLOCKLEN_SHIFT                                      0
#define    ISPII_DPCM_BLOCKLEN_MASK                                       0x000001FF

#define ISPII_FIFO_OFFSET                                                 0x00000110
#define ISPII_FIFO_TYPE                                                   UInt32
#define ISPII_FIFO_RESERVED_MASK                                          0xFFFFC0C0
#define    ISPII_FIFO_MIN3_SHIFT                                          8
#define    ISPII_FIFO_MIN3_MASK                                           0x00003F00
#define    ISPII_FIFO_MIN2_SHIFT                                          0
#define    ISPII_FIFO_MIN2_MASK                                           0x0000003F

#define ISPDI_ADDR_OFFSET                                                 0x00000130
#define ISPDI_ADDR_TYPE                                                   UInt32
#define ISPDI_ADDR_RESERVED_MASK                                          0x0000001F

#define ISPDI_ENDADDR_OFFSET                                              0x00000134
#define ISPDI_ENDADDR_TYPE                                                UInt32
#define ISPDI_ENDADDR_RESERVED_MASK                                       0x0000001F

#define ISPBL_ABSC_R_OFFSET                                               0x00000160
#define ISPBL_ABSC_R_TYPE                                                 UInt32
#define ISPBL_ABSC_R_RESERVED_MASK                                        0x00000000
#define    ISPBL_ABSC_R_ABSCISSA1_SHIFT                                   16
#define    ISPBL_ABSC_R_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPBL_ABSC_R_ABSCISSA0_SHIFT                                   0
#define    ISPBL_ABSC_R_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPBL_ABSC_GR_OFFSET                                              0x00000180
#define ISPBL_ABSC_GR_TYPE                                                UInt32
#define ISPBL_ABSC_GR_RESERVED_MASK                                       0x00000000
#define    ISPBL_ABSC_GR_ABSCISSA1_SHIFT                                  16
#define    ISPBL_ABSC_GR_ABSCISSA1_MASK                                   0xFFFF0000
#define    ISPBL_ABSC_GR_ABSCISSA0_SHIFT                                  0
#define    ISPBL_ABSC_GR_ABSCISSA0_MASK                                   0x0000FFFF

#define ISPBL_ABSC_B_OFFSET                                               0x000001A0
#define ISPBL_ABSC_B_TYPE                                                 UInt32
#define ISPBL_ABSC_B_RESERVED_MASK                                        0x00000000
#define    ISPBL_ABSC_B_ABSCISSA1_SHIFT                                   16
#define    ISPBL_ABSC_B_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPBL_ABSC_B_ABSCISSA0_SHIFT                                   0
#define    ISPBL_ABSC_B_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPBL_ABSC_GB_OFFSET                                              0x00000300
#define ISPBL_ABSC_GB_TYPE                                                UInt32
#define ISPBL_ABSC_GB_RESERVED_MASK                                       0x00000000
#define    ISPBL_ABSC_GB_ABSCISSA1_SHIFT                                  16
#define    ISPBL_ABSC_GB_ABSCISSA1_MASK                                   0xFFFF0000
#define    ISPBL_ABSC_GB_ABSCISSA0_SHIFT                                  0
#define    ISPBL_ABSC_GB_ABSCISSA0_MASK                                   0x0000FFFF

#define ISPBL_ORD_SLOPE_R_OFFSET                                          0x000001C0
#define ISPBL_ORD_SLOPE_R_TYPE                                            UInt32
#define ISPBL_ORD_SLOPE_R_RESERVED_MASK                                   0x00000000
#define    ISPBL_ORD_SLOPE_R_ORDINATE_SHIFT                               16
#define    ISPBL_ORD_SLOPE_R_ORDINATE_MASK                                0xFFFF0000
#define    ISPBL_ORD_SLOPE_R_SLOPE_SHIFT                                  0
#define    ISPBL_ORD_SLOPE_R_SLOPE_MASK                                   0x0000FFFF

#define ISPBL_ORD_SLOPE_GR_OFFSET                                         0x00000200
#define ISPBL_ORD_SLOPE_GR_TYPE                                           UInt32
#define ISPBL_ORD_SLOPE_GR_RESERVED_MASK                                  0x00000000
#define    ISPBL_ORD_SLOPE_GR_ORDINATE_SHIFT                              16
#define    ISPBL_ORD_SLOPE_GR_ORDINATE_MASK                               0xFFFF0000
#define    ISPBL_ORD_SLOPE_GR_SLOPE_SHIFT                                 0
#define    ISPBL_ORD_SLOPE_GR_SLOPE_MASK                                  0x0000FFFF

#define ISPBL_ORD_SLOPE_B_OFFSET                                          0x00000240
#define ISPBL_ORD_SLOPE_B_TYPE                                            UInt32
#define ISPBL_ORD_SLOPE_B_RESERVED_MASK                                   0x00000000
#define    ISPBL_ORD_SLOPE_B_ORDINATE_SHIFT                               16
#define    ISPBL_ORD_SLOPE_B_ORDINATE_MASK                                0xFFFF0000
#define    ISPBL_ORD_SLOPE_B_SLOPE_SHIFT                                  0
#define    ISPBL_ORD_SLOPE_B_SLOPE_MASK                                   0x0000FFFF

#define ISPBL_ORD_SLOPE_GB_OFFSET                                         0x00000340
#define ISPBL_ORD_SLOPE_GB_TYPE                                           UInt32
#define ISPBL_ORD_SLOPE_GB_RESERVED_MASK                                  0x00000000
#define    ISPBL_ORD_SLOPE_GB_ORDINATE_SHIFT                              16
#define    ISPBL_ORD_SLOPE_GB_ORDINATE_MASK                               0xFFFF0000
#define    ISPBL_ORD_SLOPE_GB_SLOPE_SHIFT                                 0
#define    ISPBL_ORD_SLOPE_GB_SLOPE_MASK                                  0x0000FFFF

#define ISPBL_TB_OFFSET                                                   0x00000280
#define ISPBL_TB_TYPE                                                     UInt32
#define ISPBL_TB_RESERVED_MASK                                            0x0000FF00
#define    ISPBL_TB_BOTTOM_SHIFT                                          16
#define    ISPBL_TB_BOTTOM_MASK                                           0xFFFF0000
#define    ISPBL_TB_TOP_SHIFT                                             0
#define    ISPBL_TB_TOP_MASK                                              0x000000FF

#define ISPBL_LR_OFFSET                                                   0x00000284
#define ISPBL_LR_TYPE                                                     UInt32
#define ISPBL_LR_RESERVED_MASK                                            0x0000FF00
#define    ISPBL_LR_RIGHT_SHIFT                                           16
#define    ISPBL_LR_RIGHT_MASK                                            0xFFFF0000
#define    ISPBL_LR_LEFT_SHIFT                                            0
#define    ISPBL_LR_LEFT_MASK                                             0x000000FF

#define ISPBL_MT_OFFSET                                                   0x00000288
#define ISPBL_MT_TYPE                                                     UInt32
#define ISPBL_MT_RESERVED_MASK                                            0x00000000
#define    ISPBL_MT_THRESHOLD_SHIFT                                       16
#define    ISPBL_MT_THRESHOLD_MASK                                        0xFFFF0000
#define    ISPBL_MT_MULTIPLIER_SHIFT                                      0
#define    ISPBL_MT_MULTIPLIER_MASK                                       0x0000FFFF

#define ISPBL_SHIFT_OFFSET                                                0x0000028C
#define ISPBL_SHIFT_TYPE                                                  UInt32
#define ISPBL_SHIFT_RESERVED_MASK                                         0x00000000
#define    ISPBL_SHIFT_SHIFT_GB_SHIFT                                     24
#define    ISPBL_SHIFT_SHIFT_GB_MASK                                      0xFF000000
#define    ISPBL_SHIFT_SHIFT_B_SHIFT                                      16
#define    ISPBL_SHIFT_SHIFT_B_MASK                                       0x00FF0000
#define    ISPBL_SHIFT_SHIFT_GR_SHIFT                                     8
#define    ISPBL_SHIFT_SHIFT_GR_MASK                                      0x0000FF00
#define    ISPBL_SHIFT_SHIFT_R_SHIFT                                      0
#define    ISPBL_SHIFT_SHIFT_R_MASK                                       0x000000FF

#define ISPBL_SUM_LO_OFFSET                                               0x00000290
#define ISPBL_SUM_LO_TYPE                                                 UInt32
#define ISPBL_SUM_LO_RESERVED_MASK                                        0x00000000

#define ISPBL_SUM_HI_OFFSET                                               0x00000294
#define ISPBL_SUM_HI_TYPE                                                 UInt32
#define ISPBL_SUM_HI_RESERVED_MASK                                        0x00000000

#define ISPBL_SUM_OFFSET                                                  0x00000290
#define ISPBL_SUM_TYPE                                                    UInt64
#define ISPBL_SUM_RESERVED_MASK                                           0x00000000

#define ISPBL_COUNT_OFFSET                                                0x00000298
#define ISPBL_COUNT_TYPE                                                  UInt32
#define ISPBL_COUNT_RESERVED_MASK                                         0x00000000

// as per HWRHEA-2220, remove non-existent register
//#define ISPDP_CLUSTER_OFFSET                                              0x00003C00
//#define ISPDP_CLUSTER_TYPE                                                UInt64
//#define ISPDP_CLUSTER_RESERVED_MASK                                       0x00000000
//#define    ISPDP_CLUSTER_MASK_SHIFT                                       32
//#define    ISPDP_CLUSTER_MASK_MASK                                        0x0000FFFF00000000
//#define    ISPDP_CLUSTER_Y_SHIFT                                          16
//#define    ISPDP_CLUSTER_Y_MASK                                           0x00000000FFFF0000
//#define    ISPDP_CLUSTER_X_SHIFT                                          0
//#define    ISPDP_CLUSTER_X_MASK                                           0x000000000000FFFF   

#define ISPDP_HI_OFFSET                                                   0x000002D0
#define ISPDP_HI_TYPE                                                     UInt32
#define ISPDP_HI_RESERVED_MASK                                            0x00000000
#define    ISPDP_HI_OFFSET1_SHIFT                                         16
#define    ISPDP_HI_OFFSET1_MASK                                          0xFFFF0000
#define    ISPDP_HI_SCALE_SHIFT                                           0
#define    ISPDP_HI_SCALE_MASK                                            0x0000FFFF


#define ISPDP_LO_OFFSET                                                   0x000002DC
#define ISPDP_LO_TYPE                                                     UInt32
#define ISPDP_LO_RESERVED_MASK                                            0x00000000
#define    ISPDP_LO_OFFSET1_SHIFT                                         16
#define    ISPDP_LO_OFFSET1_MASK                                          0xFFFF0000
#define    ISPDP_LO_SCALE_SHIFT                                           0
#define    ISPDP_LO_SCALE_MASK                                            0x0000FFFF


#define ISPRS_CTRL_OFFSET                                                 0x00000490
#define ISPRS_CTRL_TYPE                                                   UInt32
#define ISPRS_CTRL_RESERVED_MASK                                          0xFEF0FEF0
#define    ISPRS_CTRL_V_EVEN_SHIFT                                        24
#define    ISPRS_CTRL_V_EVEN_MASK                                         0x01000000
#define    ISPRS_CTRL_V_FACTOR_SHIFT                                      16
#define    ISPRS_CTRL_V_FACTOR_MASK                                       0x000F0000
#define    ISPRS_CTRL_H_EVEN_SHIFT                                        8
#define    ISPRS_CTRL_H_EVEN_MASK                                         0x00000100
#define    ISPRS_CTRL_H_FACTOR_SHIFT                                      0
#define    ISPRS_CTRL_H_FACTOR_MASK                                       0x0000000F

#define ISPLS_CTRL_OFFSET                                                 0x000004D0
#define ISPLS_CTRL_TYPE                                                   UInt32
#define ISPLS_CTRL_RESERVED_MASK                                          0xFC00FF00
#define    ISPLS_CTRL_PITCH_SHIFT                                         16
#define    ISPLS_CTRL_PITCH_MASK                                          0x03FF0000
#define    ISPLS_CTRL_CELL_SZ_POW_Y_SHIFT                                 4
#define    ISPLS_CTRL_CELL_SZ_POW_Y_MASK                                  0x000000F0
#define    ISPLS_CTRL_CELL_SZ_POW_X_SHIFT                                 0
#define    ISPLS_CTRL_CELL_SZ_POW_X_MASK                                  0x0000000F

#define ISPLS_OFFSETS_OFFSET                                              0x000004D4
#define ISPLS_OFFSETS_TYPE                                                UInt32
#define ISPLS_OFFSETS_RESERVED_MASK                                       0x00000000
#define    ISPLS_OFFSETS_Y_OFFSET_SHIFT                                   16
#define    ISPLS_OFFSETS_Y_OFFSET_MASK                                    0xFFFF0000
#define    ISPLS_OFFSETS_X_OFFSET_SHIFT                                   0
#define    ISPLS_OFFSETS_X_OFFSET_MASK                                    0x0000FFFF

#define ISPLS_CV_OFFSET                                                   0x00001000
#define ISPLS_CV_TYPE                                                     UInt32
#define ISPLS_CV_RESERVED_MASK                                            0x00000000

#define ISPXC_ABSC_LIM_OFFSET                                             0x00000500
#define ISPXC_ABSC_LIM_TYPE                                               UInt32
#define ISPXC_ABSC_LIM_RESERVED_MASK                                      0x00000000

#define ISPXC_ORD_LIM_OFFSET                                              0x00000510
#define ISPXC_ORD_LIM_TYPE                                                UInt32
#define ISPXC_ORD_LIM_RESERVED_MASK                                       0x00000000
#define    ISPXC_ORD_LIM_ORDINATE1_SHIFT                                  16
#define    ISPXC_ORD_LIM_ORDINATE1_MASK                                   0xFFFF0000
#define    ISPXC_ORD_LIM_ORDINATE0_SHIFT                                  0
#define    ISPXC_ORD_LIM_ORDINATE0_MASK                                   0x0000FFFF

#define ISPXC_SLOPE_LIM_OFFSET                                            0x00000520
#define ISPXC_SLOPE_LIM_TYPE                                              UInt32
#define ISPXC_SLOPE_LIM_RESERVED_MASK                                     0x00000000
#define    ISPXC_SLOPE_LIM_SLOPE1_SHIFT                                   16
#define    ISPXC_SLOPE_LIM_SLOPE1_MASK                                    0xFFFF0000
#define    ISPXC_SLOPE_LIM_SLOPE0_SHIFT                                   0
#define    ISPXC_SLOPE_LIM_SLOPE0_MASK                                    0x0000FFFF

#define ISPDN_CTRL_OFFSET                                                 0x00000580
#define ISPDN_CTRL_TYPE                                                   UInt32
#define ISPDN_CTRL_RESERVED_MASK                                          0xFFFFFFFE
#define    ISPDN_CTRL_MODE_SHIFT                                          0
#define    ISPDN_CTRL_MODE_MASK                                           0x00000001

#define ISPDN_ABSC_GD_OFFSET                                              0x00000584
#define ISPDN_ABSC_GD_TYPE                                                UInt32
#define ISPDN_ABSC_GD_RESERVED_MASK                                       0x00000000

#define ISPDN_ORD_GD_OFFSET                                               0x00000594
#define ISPDN_ORD_GD_TYPE                                                 UInt32
#define ISPDN_ORD_GD_RESERVED_MASK                                        0x00000000

#define ISPDN_SLOPE_GD_OFFSET                                             0x000005A4
#define ISPDN_SLOPE_GD_TYPE                                               UInt32
#define ISPDN_SLOPE_GD_RESERVED_MASK                                      0x00000000

#define ISPDN_GAIN_GD_OFFSET                                              0x000005B4
#define ISPDN_GAIN_GD_TYPE                                                UInt32
#define ISPDN_GAIN_GD_RESERVED_MASK                                       0x00000000
#define    ISPDN_GAIN_GD_RED_SHIFT                                        16
#define    ISPDN_GAIN_GD_RED_MASK                                         0xFFFF0000
#define    ISPDN_GAIN_GD_BLUE_SHIFT                                       0
#define    ISPDN_GAIN_GD_BLUE_MASK                                        0x0000FFFF

#define ISPDN_SHIFT_GD_OFFSET                                             0x000005B8
#define ISPDN_SHIFT_GD_TYPE                                               UInt32
#define ISPDN_SHIFT_GD_RESERVED_MASK                                      0x00000000
#define    ISPDN_SHIFT_GD_SHIFT_GD_SHIFT                                  16
#define    ISPDN_SHIFT_GD_SHIFT_GD_MASK                                   0x00070000
#define    ISPDN_SHIFT_GD_RED_SHIFT                                       8
#define    ISPDN_SHIFT_GD_RED_MASK                                        0x00000700
#define    ISPDN_SHIFT_GD_BLUE_SHIFT                                      0
#define    ISPDN_SHIFT_GD_BLUE_MASK                                       0x00000007

#define ISPDN_OFFSET_OFFSET                                               0x000005BC
#define ISPDN_OFFSET_TYPE                                                 UInt32
#define ISPDN_OFFSET_RESERVED_MASK                                        0x00000000
#define    ISPDN_OFFSET_Y_SHIFT                                           16
#define    ISPDN_OFFSET_Y_MASK                                            0xFFFF0000
#define    ISPDN_OFFSET_X_SHIFT                                           0
#define    ISPDN_OFFSET_X_MASK                                            0x0000FFFF

#define ISPDN_ABSC_GN_OFFSET                                              0x000005C0
#define ISPDN_ABSC_GN_TYPE                                                UInt32
#define ISPDN_ABSC_GN_RESERVED_MASK                                       0x00000000

#define ISPDN_ORD_GN_OFFSET                                               0x000005D0
#define ISPDN_ORD_GN_TYPE                                                 UInt32
#define ISPDN_ORD_GN_RESERVED_MASK                                        0x00000000

#define ISPDN_SLOPE_GN_OFFSET                                             0x000005E0
#define ISPDN_SLOPE_GN_TYPE                                               UInt32
#define ISPDN_SLOPE_GN_RESERVED_MASK                                      0x00000000

#define ISPDN_CBCR_THRESH_OFFSET                                          0x000005F0
#define ISPDN_CBCR_THRESH_TYPE                                            UInt32
#define ISPDN_CBCR_THRESH_RESERVED_MASK                                   0x00000000

#define ISPDM_CTRL_OFFSET                                                 0x000006D0
#define ISPDM_CTRL_TYPE                                                   UInt32
#define ISPDM_CTRL_RESERVED_MASK                                          0xF00FF0FC
#define    ISPDM_CTRL_HV_BIAS_SHIFT                                       20
#define    ISPDM_CTRL_HV_BIAS_MASK                                        0x0FF00000
#define    ISPDM_CTRL_G_OFF_SH_SHIFT                                      8
#define    ISPDM_CTRL_G_OFF_SH_MASK                                       0x00000F00
#define    ISPDM_CTRL_V_COSITED_SHIFT                                     1
#define    ISPDM_CTRL_V_COSITED_MASK                                      0x00000002
#define    ISPDM_CTRL_H_COSITED_SHIFT                                     0
#define    ISPDM_CTRL_H_COSITED_MASK                                      0x00000001

#define ISPYI_CTRL_OFFSET                                                 0x00000700
#define ISPYI_CTRL_TYPE                                                   UInt32
#define ISPYI_CTRL_RESERVED_MASK                                          0xFCFFEC08
#define    ISPYI_CTRL_COL_WIDTH_SHIFT                                     24
#define    ISPYI_CTRL_COL_WIDTH_MASK                                      0x03000000
#define    ISPYI_CTRL_COL_MODE_SHIFT                                      12
#define    ISPYI_CTRL_COL_MODE_MASK                                       0x00001000
#define    ISPYI_CTRL_CC_ORDER_SHIFT                                      7
#define    ISPYI_CTRL_CC_ORDER_MASK                                       0x00000380
#define    ISPYI_CTRL_DATA_FORMAT_SHIFT                                   4
#define    ISPYI_CTRL_DATA_FORMAT_MASK                                    0x00000070
#define    ISPYI_CTRL_BYTES_SAMPLE_SHIFT                                  2
#define    ISPYI_CTRL_BYTES_SAMPLE_MASK                                   0x00000004
#define    ISPYI_CTRL_CS_FORMAT_SHIFT                                     0
#define    ISPYI_CTRL_CS_FORMAT_MASK                                     0X00000003

#define ISPYI_RY_ADDR_OFFSET                                              0X00000704
#define ISPYI_RY_ADDR_TYPE                                                UInt32
#define ISPYI_RY_ADDR_RESERVED_MASK                                       0x0000001F
#define    ISPYI_RY_ADDR_ADDRESS_SHIFT                                    5
#define    ISPYI_RY_ADDR_ADDRESS_MASK                                     0xFFFFFFE0

#define ISPYI_GU_ADDR_OFFSET                                              0X00000708
#define ISPYI_GU_ADDR_TYPE                                                UInt32
#define ISPYI_GU_ADDR_RESERVED_MASK                                       0x0000001F
#define    ISPYI_GU_ADDR_ADDRESS_SHIFT                                    5
#define    ISPYI_GU_ADDR_ADDRESS_MASK                                     0xFFFFFFE0

#define ISPYI_BV_ADDR_OFFSET                                              0X0000070C
#define ISPYI_BV_ADDR_TYPE                                                UInt32
#define ISPYI_BV_ADDR_RESERVED_MASK                                       0x0000001F
#define    ISPYI_BV_ADDR_ADDRESS_SHIFT                                    5
#define    ISPYI_BV_ADDR_ADDRESS_MASK                                     0xFFFFFFE0

#define ISPYI_STRIDE1_OFFSET                                              0X00000710
#define ISPYI_STRIDE1_TYPE                                                UInt32
#define ISPYI_STRIDE1_RESERVED_MASK                                       0xFFF0001F
#define    ISPYI_STRIDE1_Y_STRIDE_SHIFT                                   5
#define    ISPYI_STRIDE1_Y_STRIDE_MASK                                    0x000FFFE0

#define ISPYI_STRIDE2_OFFSET                                              0X00000714
#define ISPYI_STRIDE2_TYPE                                                UInt32
#define ISPYI_STRIDE2_RESERVED_MASK                                       0xFFF0000F
#define    ISPYI_STRIDE2_UV_STRIDE_SHIFT                                  4
#define    ISPYI_STRIDE2_UV_STRIDE_MASK                                   0x000FFFF0

#define ISPYI_COL_STRIDE1_OFFSET                                          0X00000718
#define ISPYI_COL_STRIDE1_TYPE                                            UInt32
#define ISPYI_COL_STRIDE1_RESERVED_MASK                                   0xFE0001FF
#define    ISPYI_COL_STRIDE1_STRIDE_SHIFT                                 9
#define    ISPYI_COL_STRIDE1_STRIDE_MASK                                  0x01FFFE00

#define ISPYI_COL_STRIDE2_OFFSET                                          0X0000071C
#define ISPYI_COL_STRIDE2_TYPE                                            UInt32
#define ISPYI_COL_STRIDE2_RESERVED_MASK                                   0xFE0001FF
#define    ISPYI_COL_STRIDE2_STRIDE_SHIFT                                 9
#define    ISPYI_COL_STRIDE2_STRIDE_MASK                                  0x01FFFE00

#define ISPYI_RY_EADDR_OFFSET                                             0X00000720
#define ISPYI_RY_EADDR_TYPE                                               UInt32
#define ISPYI_RY_EADDR_RESERVED_MASK                                      0x0000001F
#define    ISPYI_RY_EADDR_ENDADDR_SHIFT                                   5
#define    ISPYI_RY_EADDR_ENDADDR_MASK                                    0xFFFFFFE0

#define ISPYI_GU_EADDR_OFFSET                                             0X00000724
#define ISPYI_GU_EADDR_TYPE                                               UInt32
#define ISPYI_GU_EADDR_RESERVED_MASK                                      0x0000001F
#define    ISPYI_GU_EADDR_ENDADDR_SHIFT                                   5
#define    ISPYI_GU_EADDR_ENDADDR_MASK                                    0xFFFFFFE0

#define ISPYI_BV_EADDR_OFFSET                                             0X00000728
#define ISPYI_BV_EADDR_TYPE                                               UInt32
#define ISPYI_BV_EADDR_RESERVED_MASK                                      0x0000001F
#define    ISPYI_BV_EADDR_ENDADDR_SHIFT                                   5
#define    ISPYI_BV_EADDR_ENDADDR_MASK                                    0xFFFFFFE0


#define ISPYC_MATRIX_OFFSET                                               0x00000750
#define ISPYC_MATRIX_TYPE                                                 UInt32
#define ISPYC_MATRIX_RESERVED_MASK                                        0x00000000

#define ISPYC_OFFSET_OFFSET                                               0x00000764
#define ISPYC_OFFSET_TYPE                                                 UInt32
#define ISPYC_OFFSET_RESERVED_MASK                                        0x00000000
#define    ISPYC_OFFSET_OFFSET1_SHIFT                                     16
#define    ISPYC_OFFSET_OFFSET1_MASK                                      0xFFFF0000
#define    ISPYC_OFFSET_OFFSET0_SHIFT                                     0
#define    ISPYC_OFFSET_OFFSET0_MASK                                      0x0000FFFF

#define ISPGM_ABSC_R_OFFSET                                               0x00000820
#define ISPGM_ABSC_R_TYPE                                                 UInt32
#define ISPGM_ABSC_R_RESERVED_MASK                                        0x00000000
#define    ISPGM_ABSC_R_ABSCISSA1_SHIFT                                   16
#define    ISPGM_ABSC_R_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPGM_ABSC_R_ABSCISSA0_SHIFT                                   0
#define    ISPGM_ABSC_R_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPGM_ABSC_G_OFFSET                                               0x00000840
#define ISPGM_ABSC_G_TYPE                                                 UInt32
#define ISPGM_ABSC_G_RESERVED_MASK                                        0x00000000
#define    ISPGM_ABSC_G_ABSCISSA1_SHIFT                                   16
#define    ISPGM_ABSC_G_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPGM_ABSC_G_ABSCISSA0_SHIFT                                   0
#define    ISPGM_ABSC_G_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPGM_ABSC_B_OFFSET                                               0x00000860
#define ISPGM_ABSC_B_TYPE                                                 UInt32
#define ISPGM_ABSC_B_RESERVED_MASK                                        0x00000000
#define    ISPGM_ABSC_B_ABSCISSA1_SHIFT                                   16
#define    ISPGM_ABSC_B_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPGM_ABSC_B_ABSCISSA0_SHIFT                                   0
#define    ISPGM_ABSC_B_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPGM_ORD_R_OFFSET                                                0x00000880
#define ISPGM_ORD_R_TYPE                                                  UInt32
#define ISPGM_ORD_R_RESERVED_MASK                                         0x00000000
#define    ISPGM_ORD_R_ORDINATE1_SHIFT                                    16
#define    ISPGM_ORD_R_ORDINATE1_MASK                                     0xFFFF0000
#define    ISPGM_ORD_R_ORDINATE0_SHIFT                                    0
#define    ISPGM_ORD_R_ORDINATE0_MASK                                     0x0000FFFF

#define ISPGM_ORD_G_OFFSET                                                0x000008A0
#define ISPGM_ORD_G_TYPE                                                  UInt32
#define ISPGM_ORD_G_RESERVED_MASK                                         0x00000000
#define    ISPGM_ORD_G_ORDINATE1_SHIFT                                    16
#define    ISPGM_ORD_G_ORDINATE1_MASK                                     0xFFFF0000
#define    ISPGM_ORD_G_ORDINATE0_SHIFT                                    0
#define    ISPGM_ORD_G_ORDINATE0_MASK                                     0x0000FFFF

#define ISPGM_ORD_B_OFFSET                                                0x000008C0
#define ISPGM_ORD_B_TYPE                                                  UInt32
#define ISPGM_ORD_B_RESERVED_MASK                                         0x00000000
#define    ISPGM_ORD_B_ORDINATE1_SHIFT                                    16
#define    ISPGM_ORD_B_ORDINATE1_MASK                                     0xFFFF0000
#define    ISPGM_ORD_B_ORDINATE0_SHIFT                                    0
#define    ISPGM_ORD_B_ORDINATE0_MASK                                     0x0000FFFF

#define ISPFC_Y_EDGE_OFFSET                                                0x00000970
#define ISPFC_Y_EDGE_TYPE                                                  UInt32
#define ISPFC_Y_EDGE_RESERVED_MASK                                         0x00000000
#define    ISPFC_Y_EDGE_SLOPE_SHIFT                                            16
#define    ISPFC_Y_EDGE_SLOPE_MASK                                             0xFFFF0000
#define    ISPFC_Y_EDGE_THRESHOLD_SHIFT                                    0
#define    ISPFC_Y_EDGE_THRESHOLD_MASK                                     0x0000FFFF

#define ISPFC_Y_EDGE_OFFSET                                                0x00000970
#define ISPFC_Y_EDGE_TYPE                                                  UInt32
#define ISPFC_Y_EDGE_RESERVED_MASK                                         0x00000000
#define    ISPFC_Y_EDGE_SLOPE_SHIFT                                            16
#define    ISPFC_Y_EDGE_SLOPE_MASK                                             0xFFFF0000
#define    ISPFC_Y_EDGE_THRESHOLD_SHIFT                                    0
#define    ISPFC_Y_EDGE_THRESHOLD_MASK                                     0x0000FFFF

#define ISPFC_Y_LO_OFFSET                                                      0x00000974
#define ISPFC_Y_LO_TYPE                                                       UInt32
#define ISPFC_Y_LO_RESERVED_MASK                                               0x00000000
#define    ISPFC_Y_LO_SLOPE_SHIFT                                                16
#define    ISPFC_Y_LO_SLOPE_MASK                                                 0xFFFF0000
#define    ISPFC_Y_LO_THRESHOLD_SHIFT                                       0
#define    ISPFC_Y_LO_THRESHOLD_MASK                                         0x0000FFFF

#define ISPFC_Y_HI_OFFSET                                                      0x00000978
#define ISPFC_Y_HI_TYPE                                                       UInt32
#define ISPFC_Y_HI_RESERVED_MASK                                               0x00000000
#define    ISPFC_Y_HI_SLOPE_SHIFT                                                16
#define    ISPFC_Y_HI_SLOPE_MASK                                                 0xFFFF0000
#define    ISPFC_Y_HI_THRESHOLD_SHIFT                                       0
#define    ISPFC_Y_HI_THRESHOLD_MASK                                         0x0000FFFF

#define ISPFC_THRESH_OFFSET                                                      0x0000097C
#define ISPFC_THRESH_TYPE                                                       UInt32
#define ISPFC_THRESH_RESERVED_MASK                                               0x00000000
#define    ISPFC_THRESH_HI_SHIFT                                                16
#define    ISPFC_THRESH_HI_MASK                                                 0xFFFF0000
#define    ISPFC_THRESH_LO_SHIFT                                                0
#define    ISPFC_THRESH_LO_MASK                                                 0x0000FFFF

#define ISPGM_SLOPE_R_OFFSET                                          0x00003400
#define ISPGM_SLOPE_R_TYPE                                            UInt32
#define ISPGM_SLOPE_R_RESERVED_MASK                                   0xFFFE0000
#define    ISPGM_SLOPE_R_SLOPE_SHIFT                                  0
#define    ISPGM_SLOPE_R_SLOPE_MASK                                   0x0001FFFF

#define ISPGM_SLOPE_G_OFFSET                                          0x00003440
#define ISPGM_SLOPE_G_TYPE                                            UInt32
#define ISPGM_SLOPE_G_RESERVED_MASK                                   0xFFFE0000
#define    ISPGM_SLOPE_G_SLOPE_SHIFT                                  0
#define    ISPGM_SLOPE_G_SLOPE_MASK                                   0x0001FFFF

#define ISPGM_SLOPE_B_OFFSET                                          0x00003480
#define ISPGM_SLOPE_B_TYPE                                            UInt32
#define ISPGM_SLOPE_B_RESERVED_MASK                                   0xFFFE0000
#define    ISPGM_SLOPE_B_SLOPE_SHIFT                                  0
#define    ISPGM_SLOPE_B_SLOPE_MASK                                   0x0001FFFF

#define ISPTM_Y_ABSC_OFFSET                                               0x00003500
#define ISPTM_Y_ABSC_TYPE                                                 UInt32
#define ISPTM_Y_ABSC_RESERVED_MASK                                        0x00000000
#define    ISPTM_Y_ABSC_ABSCISSA1_SHIFT                                   16
#define    ISPTM_Y_ABSC_ABSCISSA1_MASK                                    0xFFFF0000
#define    ISPTM_Y_ABSC_ABSCISSA0_SHIFT                                   0
#define    ISPTM_Y_ABSC_ABSCISSA0_MASK                                    0x0000FFFF

#define ISPTM_Y_ORD_SLOPE_OFFSET                                          0x00003580
#define ISPTM_Y_ORD_SLOPE_TYPE                                            UInt32
#define ISPTM_Y_ORD_SLOPE_RESERVED_MASK                                   0x00000000
#define    ISPTM_Y_ORD_SLOPE_ORDINATE_SHIFT                               16
#define    ISPTM_Y_ORD_SLOPE_ORDINATE_MASK                                0xFFFF0000
#define    ISPTM_Y_ORD_SLOPE_SLOPE_SHIFT                                  0
#define    ISPTM_Y_ORD_SLOPE_SLOPE_MASK                                   0x0000FFFF

#define ISPCP_CB_ABSC_OFFSET                                              0x00003540
#define ISPCP_CB_ABSC_TYPE                                                UInt32
#define ISPCP_CB_ABSC_RESERVED_MASK                                       0x00000000
#define    ISPCP_CB_ABSC_ABSCISSA1_SHIFT                                  16
#define    ISPCP_CB_ABSC_ABSCISSA1_MASK                                   0xFFFF0000
#define    ISPCP_CB_ABSC_ABSCISSA0_SHIFT                                  0
#define    ISPCP_CB_ABSC_ABSCISSA0_MASK                                   0x0000FFFF

#define ISPCP_CR_ABSC_OFFSET                                              0x00003560
#define ISPCP_CR_ABSC_TYPE                                                UInt32
#define ISPCP_CR_ABSC_RESERVED_MASK                                       0x00000000
#define    ISPCP_CR_ABSC_ABSCISSA1_SHIFT                                  16
#define    ISPCP_CR_ABSC_ABSCISSA1_MASK                                   0xFFFF0000
#define    ISPCP_CR_ABSC_ABSCISSA0_SHIFT                                  0
#define    ISPCP_CR_ABSC_ABSCISSA0_MASK                                   0x0000FFFF

#define ISPCP_CB_ORD_SLOPE_OFFSET                                         0x000035C0
#define ISPCP_CB_ORD_SLOPE_TYPE                                           UInt32
#define ISPCP_CB_ORD_SLOPE_RESERVED_MASK                                  0xF000F000
#define    ISPCP_CB_ORD_SLOPE_ORDINATE_SHIFT                              16
#define    ISPCP_CB_ORD_SLOPE_ORDINATE_MASK                               0xFFFF0000
#define    ISPCP_CB_ORD_SLOPE_SLOPE_SHIFT                                 0
#define    ISPCP_CB_ORD_SLOPE_SLOPE_MASK                                  0x0000FFFF

#define ISPCP_CR_ORD_SLOPE_OFFSET                                         0x000035E0
#define ISPCP_CR_ORD_SLOPE_TYPE                                           UInt32
#define ISPCP_CR_ORD_SLOPE_RESERVED_MASK                                  0xF000F000
#define    ISPCP_CR_ORD_SLOPE_ORDINATE_SHIFT                              16
#define    ISPCP_CR_ORD_SLOPE_ORDINATE_MASK                               0xFFFF0000
#define    ISPCP_CR_ORD_SLOPE_SLOPE_SHIFT                                 0
#define    ISPCP_CR_ORD_SLOPE_SLOPE_MASK                                  0x0000FFFF

#define ISPHR_CTRL_OFFSET                                                 0x00000A50
#define ISPHR_CTRL_TYPE                                                   UInt32
#define ISPHR_CTRL_RESERVED_MASK                                          0xFFFFFFF8

#define ISPHR_SCALE_X_OFFSET                                              0x00000A54
#define ISPHR_SCALE_X_TYPE                                                UInt32
#define ISPHR_SCALE_X_RESERVED_MASK                                       0xFFC0000F
#define    ISPHR_SCALE_X_SCALE_INT_SHIFT                                  16
#define    ISPHR_SCALE_X_SCALE_INT_MASK                                   0x003F0000
#define    ISPHR_SCALE_X_SCALE_FRAC_SHIFT                                 4
#define    ISPHR_SCALE_X_SCALE_FRAC_MASK                                  0x0000FFF0

#define ISPHR_SCALE_Y_OFFSET                                              0x00000A58
#define ISPHR_SCALE_Y_TYPE                                                UInt32
#define ISPHR_SCALE_Y_RESERVED_MASK                                       0xFFC0000F
#define    ISPHR_SCALE_Y_SCALE_INT_SHIFT                                  16
#define    ISPHR_SCALE_Y_SCALE_INT_MASK                                   0x003F0000
#define    ISPHR_SCALE_Y_SCALE_FRAC_SHIFT                                 4
#define    ISPHR_SCALE_Y_SCALE_FRAC_MASK                                  0x0000FFF0

#define ISPHR_NORM_OFFSET                                                 0x00000A5C
#define ISPHR_NORM_TYPE                                                   UInt32
#define ISPHR_NORM_RESERVED_MASK                                          0xF000F000
#define    ISPHR_NORM_NORM_Y_SHIFT                                        16
#define    ISPHR_NORM_NORM_Y_MASK                                         0x0FFF0000
#define    ISPHR_NORM_NORM_X_SHIFT                                        0
#define    ISPHR_NORM_NORM_X_MASK                                         0x00000FFF

#define ISPLR_TSCALEX_OFFSET                                              0x00000B30
#define ISPLR_TSCALEX_TYPE                                                UInt32
#define ISPLR_TSCALEX_RESERVED_MASK                                       0xFFC00000
#define    ISPLR_TSCALEX_SCALE_INT_SHIFT                                  16
#define    ISPLR_TSCALEX_SCALE_INT_MASK                                   0x003F0000
#define    ISPLR_TSCALEX_SCALE_FRAC_SHIFT                                 4
#define    ISPLR_TSCALEX_SCALE_FRAC_MASK                                  0x0000FFF0

#define ISPLR_TSCALEY_OFFSET                                              0x00000B34
#define ISPLR_TSCALEY_TYPE                                                UInt32
#define ISPLR_TSCALEY_RESERVED_MASK                                       0xFFC00000
#define    ISPLR_TSCALEY_SCALE_INT_SHIFT                                  16
#define    ISPLR_TSCALEY_SCALE_INT_MASK                                   0x003F0000
#define    ISPLR_TSCALEY_SCALE_FRAC_SHIFT                                 4
#define    ISPLR_TSCALEY_SCALE_FRAC_MASK                                  0x0000FFF0

#define ISPLR_NORM_0_1_OFFSET                                             0x00000B38
#define ISPLR_NORM_0_1_TYPE                                               UInt32
#define ISPLR_NORM_0_1_RESERVED_MASK                                      0x00000000
#define    ISPLR_NORM_0_1_NORM_Y_SHIFT                                    16
#define    ISPLR_NORM_0_1_NORM_Y_MASK                                     0xFFFF0000
#define    ISPLR_NORM_0_1_NORM_X_SHIFT                                    0
#define    ISPLR_NORM_0_1_NORM_X_MASK                                     0x0000FFFF

#define ISPLR_SHIFT_OFFSET                                                0x00000B40
#define ISPLR_SHIFT_TYPE                                                  UInt32
#define ISPLR_SHIFT_RESERVED_MASK                                         0xFFFFFFF0

#define ISPLR_NORM_2_3_OFFSET                                             0x00000B3C
#define ISPLR_NORM_2_3_TYPE                                               UInt32
#define ISPLR_NORM_2_3_RESERVED_MASK                                      0x00000000

#define ISPCC_MATRIX_OFFSET                                               0x00000B50
#define ISPCC_MATRIX_TYPE                                                 UInt32
#define ISPCC_MATRIX_RESERVED_MASK                                        0x00000000
#define    ISPCC_MATRIX_ELEMENT1_SHIFT                                    16
#define    ISPCC_MATRIX_ELEMENT1_MASK                                     0xFFFF0000
#define    ISPCC_MATRIX_ELEMENT0_SHIFT                                    0
#define    ISPCC_MATRIX_ELEMENT0_MASK                                     0x0000FFFF

#define ISPCC_OFFSET_OFFSET                                               0x00000B64
#define ISPCC_OFFSET_TYPE                                                 UInt32
#define ISPCC_OFFSET_RESERVED_MASK                                        0xFFFC0000
#define    ISPCC_OFFSET_ELEMENT0_SHIFT                                    0
#define    ISPCC_OFFSET_ELEMENT0_MASK                                     0x0003FFFF

#define ISPST_SHIFT_OFFSET                                                0x00000BA0
#define ISPST_SHIFT_TYPE                                                  UInt32
#define ISPST_SHIFT_RESERVED_MASK                                         0xFFFF0000
#define    ISPST_SHIFT_PIXSHIFT_SHIFT                                     16
#define    ISPST_SHIFT_PIXSHIFT_MASK                                      0x00FF0000
#define    ISPST_SHIFT_SHIFT1_SHIFT                                         16
#define    ISPST_SHIFT_SHIFT1_MASK                                          0x00FF0000
#define    ISPST_SHIFT_SHIFT_SHIFT                                           0
#define    ISPST_SHIFT_SHIFT_MASK                                            0x0000FFFF


#define ISPST_R_OFF_OFFSET                                                0x00000BA4
#define ISPST_R_OFF_TYPE                                                  UInt32
#define ISPST_R_OFF_RESERVED_MASK                                         0x00000000
#define    ISPST_R_OFF_Y_OFFSET_SHIFT                                     16
#define    ISPST_R_OFF_Y_OFFSET_MASK                                      0xFFFF0000
#define    ISPST_R_OFF_X_OFFSET_SHIFT                                     0
#define    ISPST_R_OFF_X_OFFSET_MASK                                      0x0000FFFF

#define ISPST_R_RECT_OFFSET                                               0x00000C24
#define ISPST_R_RECT_TYPE                                                 UInt32
#define ISPST_R_RECT_RESERVED_MASK                                        0x00000000
#define    ISPST_R_RECT_HEIGHT_SHIFT                                      16
#define    ISPST_R_RECT_HEIGHT_MASK                                       0xFFFF0000
#define    ISPST_R_RECT_WIDTH_SHIFT                                       0
#define    ISPST_R_RECT_WIDTH_MASK                                        0x0000FFFF

#define ISPST_HMASK0_OFFSET                                                0x00000CA4
#define ISPST_HMASK0_TYPE                                                  UInt32
#define ISPST_HMASK0_RESERVED_MASK                                     0x00000000
#define    ISPST_HMASK0_HIST1_SHIFT                                       16
#define    ISPST_HMASK0_HIST1_MASK                                        0xFFFF0000
#define    ISPST_HMASK0_HIST0_SHIFT                                       0
#define    ISPST_HMASK0_HIST0_MASK                                        0x0000FFFF

#define ISPST_FOC_FILT_OFFSET                                             0x00000CAC
#define ISPST_FOC_FILT_TYPE                                               UInt32
#define ISPST_FOC_FILT_RESERVED_MASK                                      0xFF00FF00
#define    ISPST_FOC_FILT_COEFF1_SHIFT                                    16
#define    ISPST_FOC_FILT_COEFF1_MASK                                     0x00FF0000
#define    ISPST_FOC_FILT_COEFF0_SHIFT                                    0
#define    ISPST_FOC_FILT_COEFF0_MASK                                     0x000000FF

#define ISPST_FILT_GAINS_OFFSET                                           0x00000CD0
#define ISPST_FILT_GAINS_TYPE                                             UInt32
#define ISPST_FILT_GAINS_RESERVED_MASK                                    0xFF00FF00
#define    ISPST_FILT_GAINS_GAIN1_SHIFT                                   16
#define    ISPST_FILT_GAINS_GAIN1_MASK                                    0x00FF0000
#define    ISPST_FILT_GAINS_GAIN0_SHIFT                                   0
#define    ISPST_FILT_GAINS_GAIN0_MASK                                    0x000000FF

#define ISPST_FILT_TH_OFFSET                                              0x00000CD4
#define ISPST_FILT_TH_TYPE                                                UInt32
#define ISPST_FILT_TH_RESERVED_MASK                                       0x00000000
#define    ISPST_FILT_TH_THRESHOLD1_SHIFT                                 16
#define    ISPST_FILT_TH_THRESHOLD1_MASK                                  0xFFFF0000
#define    ISPST_FILT_TH_THRESHOLD0_SHIFT                                 0
#define    ISPST_FILT_TH_THRESHOLD0_MASK                                  0x0000FFFF

#define ISPST_ROW_NUM_OFFSET                                              0x00000CDC
#define ISPST_ROW_NUM_TYPE                                                UInt32
#define ISPST_ROW_NUM_RESERVED_MASK                                       0xFFFE0000
#define    ISPST_ROW_NUM_COL_SHIFT                                        16
#define    ISPST_ROW_NUM_COL_MASK                                         0x00010000
#define    ISPST_ROW_NUM_ROW_NUM_SHIFT                                    0
#define    ISPST_ROW_NUM_ROW_NUM_MASK                                     0x0000FFFF

#define ISPST_R_TH_OFFSET                                                 0x00000CE0
#define ISPST_R_TH_TYPE                                                   UInt32
#define ISPST_R_TH_RESERVED_MASK                                          0x00000000
#define    ISPST_R_TH_HI_SHIFT                                            16
#define    ISPST_R_TH_HI_MASK                                             0xFFFF0000
#define    ISPST_R_TH_LO_SHIFT                                            0
#define    ISPST_R_TH_LO_MASK                                             0x0000FFFF

#define ISPST_G_TH_OFFSET                                                 0x00000CEC
#define ISPST_G_TH_TYPE                                                   UInt32
#define ISPST_G_TH_RESERVED_MASK                                          0x00000000
#define    ISPST_G_TH_HI_SHIFT                                            16
#define    ISPST_G_TH_HI_MASK                                             0xFFFF0000
#define    ISPST_G_TH_LO_SHIFT                                            0
#define    ISPST_G_TH_LO_MASK                                             0x0000FFFF

#define ISPST_B_TH_OFFSET                                                 0x00000CF8
#define ISPST_B_TH_TYPE                                                   UInt32
#define ISPST_B_TH_RESERVED_MASK                                          0x00000000
#define    ISPST_B_TH_HI_SHIFT                                            16
#define    ISPST_B_TH_HI_MASK                                             0xFFFF0000
#define    ISPST_B_TH_LO_SHIFT                                            0
#define    ISPST_B_TH_LO_MASK                                             0x0000FFFF

#define ISPST_R_G_TH_OFFSET                                               0x00000D04
#define ISPST_R_G_TH_TYPE                                                 UInt32
#define ISPST_R_G_TH_RESERVED_MASK                                        0x00000000
#define    ISPST_R_G_TH_HI_SHIFT                                          16
#define    ISPST_R_G_TH_HI_MASK                                           0xFFFF0000
#define    ISPST_R_G_TH_LO_SHIFT                                          0
#define    ISPST_R_G_TH_LO_MASK                                           0x0000FFFF

#define ISPST_B_G_TH_OFFSET                                               0x00000D10
#define ISPST_B_G_TH_TYPE                                                 UInt32
#define ISPST_B_G_TH_RESERVED_MASK                                        0xF000F000
#define    ISPST_B_G_TH_HI_SHIFT                                          16
#define    ISPST_B_G_TH_HI_MASK                                           0xFFFF0000
#define    ISPST_B_G_TH_LO_SHIFT                                          0
#define    ISPST_B_G_TH_LO_MASK                                           0x0000FFFF

#define ISPST_GROUP_0_X_OFFSET                                            0x00000D1C
#define ISPST_GROUP_0_X_TYPE                                              UInt32
#define ISPST_GROUP_0_X_RESERVED_MASK                                     0x00000000
#define    ISPST_GROUP_0_X_X1_SHIFT                                       16
#define    ISPST_GROUP_0_X_X1_MASK                                        0xFFFF0000
#define    ISPST_GROUP_0_X_X0_SHIFT                                       0
#define    ISPST_GROUP_0_X_X0_MASK                                        0x0000FFFF

#define ISPST_GROUP_0_Y_OFFSET                                            0x00000D40
#define ISPST_GROUP_0_Y_TYPE                                              UInt32
#define ISPST_GROUP_0_Y_RESERVED_MASK                                     0xF000F000
#define    ISPST_GROUP_0_Y_Y1_SHIFT                                       16
#define    ISPST_GROUP_0_Y_Y1_MASK                                        0xFFFF0000
#define    ISPST_GROUP_0_Y_Y0_SHIFT                                       0
#define    ISPST_GROUP_0_Y_Y0_MASK                                        0x0000FFFF

#define ISPST_GRP0_CTRL_OFFSET                                            0x00000D64
#define ISPST_GRP0_CTRL_TYPE                                              UInt32
#define ISPST_GRP0_CTRL_RESERVED_MASK                                     0xFFFFFFC0
#define    ISPST_GRP0_CTRL_PITCH_SHIFT                                    0
#define    ISPST_GRP0_CTRL_PITCH_MASK                                     0x0000003F

#define ISPST_HGAIN0_OFFSET                                               0x00000D68
#define ISPST_HGAIN0_TYPE                                                 UInt32
#define ISPST_HGAIN0_RESERVED_MASK                                        0xFF000000
#define    ISPST_HGAIN0_BLUE_SHIFT                                        16
#define    ISPST_HGAIN0_BLUE_MASK                                         0x00FF0000
#define    ISPST_HGAIN0_GREEN_SHIFT                                        8
#define    ISPST_HGAIN0_GREEN_MASK                                         0x0000FF00
#define    ISPST_HGAIN0_RED_SHIFT                                            0
#define    ISPST_HGAIN0_RED_MASK                                             0x000000FF

#define ISPST_HGAIN1_OFFSET                                               0x00000D6C
#define ISPST_HGAIN1_TYPE                                                 UInt32
#define ISPST_HGAIN1_RESERVED_MASK                                        0xFF000000
#define    ISPST_HGAIN1_BLUE_SHIFT                                        16
#define    ISPST_HGAIN1_BLUE_MASK                                         0x00FF0000
#define    ISPST_HGAIN1_GREEN_SHIFT                                        8
#define    ISPST_HGAIN1_GREEN_MASK                                         0x0000FF00
#define    ISPST_HGAIN1_RED_SHIFT                                            0
#define    ISPST_HGAIN1_RED_MASK                                             0x000000FF

#define ISPLO_CTRL_OFFSET                                                 0x00000DC0
#define ISPLO_CTRL_TYPE                                                   UInt32
#define ISPLO_CTRL_RESERVED_MASK                                          0xFCE1A0FF
#define    ISPLO_CTRL_COL_WIDTH_SHIFT                                     24
#define    ISPLO_CTRL_COL_WIDTH_MASK                                      0x03000000
#define       ISPLO_CTRL_COL_WIDTH_32_PIXEL                               0
#define       ISPLO_CTRL_COL_WIDTH_64_PIXEL                               1
#define       ISPLO_CTRL_COL_WIDTH_128_PIXEL                              2
#define       ISPLO_CTRL_COL_WIDTH_256_PIXEL                              3
#define    ISPLO_CTRL_SHIFT_SHIFT                                         17
#define    ISPLO_CTRL_SHIFT_MASK                                          0x001E0000
#define    ISPLO_CTRL_DATA_FORMAT_BIT_4_SHIFT                             15
#define    ISPLO_CTRL_DATA_FORMAT_BIT_4_MASK                              0x00008000
#define    ISPLO_CTRL_VFLIP_SHIFT                                         14
#define    ISPLO_CTRL_VFLIP_MASK                                          0x00004000
#define    ISPLO_CTRL_COL_MODE_SHIFT                                      12
#define    ISPLO_CTRL_COL_MODE_MASK                                       0x00001000
#define    ISPLO_CTRL_DATA_FORMAT_SHIFT                                   9
#define    ISPLO_CTRL_DATA_FORMAT_MASK                                    0x00000E00
#define    ISPLO_CTRL_OUTPUT_WIDTH_SHIFT                                  8
#define    ISPLO_CTRL_OUTPUT_WIDTH_MASK                                   0x00000100

#define ISPLO_COL_STRIDE1_OFFSET                                          0x00000DC4
#define ISPLO_COL_STRIDE1_TYPE                                            UInt32
#define ISPLO_COL_STRIDE1_RESERVED_MASK                                   0xFE0001FF
#define    ISPLO_COL_STRIDE1_STRIDE_SHIFT                                 9
#define    ISPLO_COL_STRIDE1_STRIDE_MASK                                  0x01FFFE00

#define ISPLO_COL_STRIDE2_OFFSET                                          0x00000DC8
#define ISPLO_COL_STRIDE2_TYPE                                            UInt32
#define ISPLO_COL_STRIDE2_RESERVED_MASK                                   0xFE0001FF
#define    ISPLO_COL_STRIDE2_STRIDE_SHIFT                                 9
#define    ISPLO_COL_STRIDE2_STRIDE_MASK                                  0x01FFFE00

#define ISPLO_ADDRESS1_OFFSET                                             0x00000DCC
#define ISPLO_ADDRESS1_TYPE                                               UInt32
#define ISPLO_ADDRESS1_RESERVED_MASK                                      0x0000001F
#define    ISPLO_ADDRESS1_ADDRESS_SHIFT                                   5
#define    ISPLO_ADDRESS1_ADDRESS_MASK                                    0xFFFFFFE0

#define ISPLO_ADDRESS2_OFFSET                                             0x00000DD0
#define ISPLO_ADDRESS2_TYPE                                               UInt32
#define ISPLO_ADDRESS2_RESERVED_MASK                                      0x0000001F
#define    ISPLO_ADDRESS2_ADDRESS_SHIFT                                   5
#define    ISPLO_ADDRESS2_ADDRESS_MASK                                    0xFFFFFFE0

#define ISPLO_ADDRESS3_OFFSET                                             0x00000DD4
#define ISPLO_ADDRESS3_TYPE                                               UInt32
#define ISPLO_ADDRESS3_RESERVED_MASK                                      0x0000001F
#define    ISPLO_ADDRESS3_ADDRESS_SHIFT                                   5
#define    ISPLO_ADDRESS3_ADDRESS_MASK                                    0xFFFFFFE0

#define ISPLO_STRIDE1_OFFSET                                              0x00000DD8
#define ISPLO_STRIDE1_TYPE                                                UInt32
#define ISPLO_STRIDE1_RESERVED_MASK                                       0xFFE0001F
#define    ISPLO_STRIDE1_STRIDE_SHIFT                                     5
#define    ISPLO_STRIDE1_STRIDE_MASK                                      0x001FFFE0 

#define ISPLO_STRIDE2_OFFSET                                              0x00000DDC
#define ISPLO_STRIDE2_TYPE                                                UInt32
#define ISPLO_STRIDE2_RESERVED_MASK                                       0xFFE0000F
#define    ISPLO_STRIDE2_STRIDE_SHIFT                                     4
#define    ISPLO_STRIDE2_STRIDE_MASK                                      0x001FFFF0 

#endif /* __BRCM_RDB_ISP_H__ */


