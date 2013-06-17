/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_SDT_H__
#define __BRCM_RDB_SDT_H__

#define SDT_SDT_CTL_OFFSET                                                0x00000000
#define SDT_SDT_CTL_TYPE                                                  UInt32
#define SDT_SDT_CTL_RESERVED_MASK                                         0xE0000008
#define    SDT_SDT_CTL_LOWER_FIT_TAP_SHIFT                                23
#define    SDT_SDT_CTL_LOWER_FIT_TAP_MASK                                 0x1F800000
#define    SDT_SDT_CTL_BYPASS_DIG_GAIN_SHIFT                              22
#define    SDT_SDT_CTL_BYPASS_DIG_GAIN_MASK                               0x00400000
#define    SDT_SDT_CTL_SOF_SLOPE_ENABLE_SHIFT                             21
#define    SDT_SDT_CTL_SOF_SLOPE_ENABLE_MASK                              0x00200000
#define    SDT_SDT_CTL_TARGET_GAIN_LOAD_SHIFT                             20
#define    SDT_SDT_CTL_TARGET_GAIN_LOAD_MASK                              0x00100000
#define    SDT_SDT_CTL_TARGET_GAIN_SHIFT                                  5
#define    SDT_SDT_CTL_TARGET_GAIN_MASK                                   0x000FFFE0
#define    SDT_SDT_CTL_SOF_SLOPE_LIN_SHIFT                                4
#define    SDT_SDT_CTL_SOF_SLOPE_LIN_MASK                                 0x00000010
#define    SDT_SDT_CTL_FIR_FILT_STOP_SHIFT                                2
#define    SDT_SDT_CTL_FIR_FILT_STOP_MASK                                 0x00000004
#define    SDT_SDT_CTL_FIR_FILTER_DISABLE_SHIFT                           1
#define    SDT_SDT_CTL_FIR_FILTER_DISABLE_MASK                            0x00000002
#define    SDT_SDT_CTL_INPUT_CH_SEL_SHIFT                                 0
#define    SDT_SDT_CTL_INPUT_CH_SEL_MASK                                  0x00000001

#define SDT_SDT_CURRENT_GAIN_OFFSET                                       0x00000004
#define SDT_SDT_CURRENT_GAIN_TYPE                                         UInt32
#define SDT_SDT_CURRENT_GAIN_RESERVED_MASK                                0xFFFE0000
#define    SDT_SDT_CURRENT_GAIN_GAIN_ADJUST_FLAG_SHIFT                    15
#define    SDT_SDT_CURRENT_GAIN_GAIN_ADJUST_FLAG_MASK                     0x00018000
#define    SDT_SDT_CURRENT_GAIN_CURRENT_GAIN_SHIFT                        0
#define    SDT_SDT_CURRENT_GAIN_CURRENT_GAIN_MASK                         0x00007FFF

#define SDT_SDT_CTRL_2_OFFSET                                             0x00000008
#define SDT_SDT_CTRL_2_TYPE                                               UInt32
#define SDT_SDT_CTRL_2_RESERVED_MASK                                      0x80000000
#define    SDT_SDT_CTRL_2_CLIP_DISABLE_SHIFT                              30
#define    SDT_SDT_CTRL_2_CLIP_DISABLE_MASK                               0x40000000
#define    SDT_SDT_CTRL_2_SOF_SLOPE_SHIFT                                 6
#define    SDT_SDT_CTRL_2_SOF_SLOPE_MASK                                  0x3FFFFFC0
#define    SDT_SDT_CTRL_2_UPPER_FIT_TAP_SHIFT                             0
#define    SDT_SDT_CTRL_2_UPPER_FIT_TAP_MASK                              0x0000003F

#define SDT_SDT_COEF_ADDR_OFFSET                                          0x0000000C
#define SDT_SDT_COEF_ADDR_TYPE                                            UInt32
#define SDT_SDT_COEF_ADDR_RESERVED_MASK                                   0xFFFFFE03
#define    SDT_SDT_COEF_ADDR_SDT_COEF_ADDR_SHIFT                          2
#define    SDT_SDT_COEF_ADDR_SDT_COEF_ADDR_MASK                           0x000001FC

#define SDT_SDT_COEF_DATA_OFFSET                                          0x00000010
#define SDT_SDT_COEF_DATA_TYPE                                            UInt32
#define SDT_SDT_COEF_DATA_RESERVED_MASK                                   0xFF000000
#define    SDT_SDT_COEF_DATA_SDT_COEF_DATA_SHIFT                          0
#define    SDT_SDT_COEF_DATA_SDT_COEF_DATA_MASK                           0x00FFFFFF

#define SDT_SDT_CTRL_3_OFFSET                                             0x00000014
#define SDT_SDT_CTRL_3_TYPE                                               UInt32
#define SDT_SDT_CTRL_3_RESERVED_MASK                                      0x00070000
#define    SDT_SDT_CTRL_3_EMPTY_ENTRY_SHIFT                               24
#define    SDT_SDT_CTRL_3_EMPTY_ENTRY_MASK                                0xFF000000
#define    SDT_SDT_CTRL_3_ALMOST_FULL_SHIFT                               23
#define    SDT_SDT_CTRL_3_ALMOST_FULL_MASK                                0x00800000
#define    SDT_SDT_CTRL_3_ALMOST_EMPTY_SHIFT                              22
#define    SDT_SDT_CTRL_3_ALMOST_EMPTY_MASK                               0x00400000
#define    SDT_SDT_CTRL_3_UDF_SHIFT                                       21
#define    SDT_SDT_CTRL_3_UDF_MASK                                        0x00200000
#define    SDT_SDT_CTRL_3_OVF_SHIFT                                       20
#define    SDT_SDT_CTRL_3_OVF_MASK                                        0x00100000
#define    SDT_SDT_CTRL_3_THR_MET_SHIFT                                   19
#define    SDT_SDT_CTRL_3_THR_MET_MASK                                    0x00080000
#define    SDT_SDT_CTRL_3_MODE16BIT_SHIFT                                 15
#define    SDT_SDT_CTRL_3_MODE16BIT_MASK                                  0x00008000
#define    SDT_SDT_CTRL_3_ERR_INTR_EN_SHIFT                               14
#define    SDT_SDT_CTRL_3_ERR_INTR_EN_MASK                                0x00004000
#define    SDT_SDT_CTRL_3_INTR_EN_SHIFT                                   13
#define    SDT_SDT_CTRL_3_INTR_EN_MASK                                    0x00002000
#define    SDT_SDT_CTRL_3_THRES2_SHIFT                                    9
#define    SDT_SDT_CTRL_3_THRES2_MASK                                     0x00001E00
#define    SDT_SDT_CTRL_3_THRES_SHIFT                                     2
#define    SDT_SDT_CTRL_3_THRES_MASK                                      0x000001FC
#define    SDT_SDT_CTRL_3_FIFO_RESET_SHIFT                                1
#define    SDT_SDT_CTRL_3_FIFO_RESET_MASK                                 0x00000002
#define    SDT_SDT_CTRL_3_FIFO_ENABLE_SHIFT                               0
#define    SDT_SDT_CTRL_3_FIFO_ENABLE_MASK                                0x00000001

#define SDT_SDT_OUTPUTFIFO_OFFSET                                         0x00000018
#define SDT_SDT_OUTPUTFIFO_TYPE                                           UInt32
#define SDT_SDT_OUTPUTFIFO_RESERVED_MASK                                  0x00000000
#define    SDT_SDT_OUTPUTFIFO_SDT_OUTPUTFIFO_SHIFT                        0
#define    SDT_SDT_OUTPUTFIFO_SDT_OUTPUTFIFO_MASK                         0xFFFFFFFF

#define SDT_DSP_SDT_CTL_OFFSET                                            0x00000000
#define SDT_DSP_SDT_CTL_TYPE                                              UInt32
#define SDT_DSP_SDT_CTL_RESERVED_MASK                                     0xE0000008
#define    SDT_DSP_SDT_CTL_LOWER_FIT_TAP_SHIFT                            23
#define    SDT_DSP_SDT_CTL_LOWER_FIT_TAP_MASK                             0x1F800000
#define    SDT_DSP_SDT_CTL_BYPASS_DIG_GAIN_SHIFT                          22
#define    SDT_DSP_SDT_CTL_BYPASS_DIG_GAIN_MASK                           0x00400000
#define    SDT_DSP_SDT_CTL_SOF_SLOPE_ENABLE_SHIFT                         21
#define    SDT_DSP_SDT_CTL_SOF_SLOPE_ENABLE_MASK                          0x00200000
#define    SDT_DSP_SDT_CTL_TARGET_GAIN_LOAD_SHIFT                         20
#define    SDT_DSP_SDT_CTL_TARGET_GAIN_LOAD_MASK                          0x00100000
#define    SDT_DSP_SDT_CTL_TARGET_GAIN_SHIFT                              5
#define    SDT_DSP_SDT_CTL_TARGET_GAIN_MASK                               0x000FFFE0
#define    SDT_DSP_SDT_CTL_SOF_SLOPE_LIN_SHIFT                            4
#define    SDT_DSP_SDT_CTL_SOF_SLOPE_LIN_MASK                             0x00000010
#define    SDT_DSP_SDT_CTL_FIR_FILT_STOP_SHIFT                            2
#define    SDT_DSP_SDT_CTL_FIR_FILT_STOP_MASK                             0x00000004
#define    SDT_DSP_SDT_CTL_FIR_FILTER_DISABLE_SHIFT                       1
#define    SDT_DSP_SDT_CTL_FIR_FILTER_DISABLE_MASK                        0x00000002
#define    SDT_DSP_SDT_CTL_INPUT_CH_SEL_SHIFT                             0
#define    SDT_DSP_SDT_CTL_INPUT_CH_SEL_MASK                              0x00000001

#define SDT_DSP_SDT_CURRENT_GAIN_OFFSET                                   0x00000002
#define SDT_DSP_SDT_CURRENT_GAIN_TYPE                                     UInt32
#define SDT_DSP_SDT_CURRENT_GAIN_RESERVED_MASK                            0xFFFE0000
#define    SDT_DSP_SDT_CURRENT_GAIN_GAIN_ADJUST_FLAG_SHIFT                15
#define    SDT_DSP_SDT_CURRENT_GAIN_GAIN_ADJUST_FLAG_MASK                 0x00018000
#define    SDT_DSP_SDT_CURRENT_GAIN_CURRENT_GAIN_SHIFT                    0
#define    SDT_DSP_SDT_CURRENT_GAIN_CURRENT_GAIN_MASK                     0x00007FFF

#define SDT_DSP_SDT_CTRL_2_OFFSET                                         0x00000004
#define SDT_DSP_SDT_CTRL_2_TYPE                                           UInt32
#define SDT_DSP_SDT_CTRL_2_RESERVED_MASK                                  0x80000000
#define    SDT_DSP_SDT_CTRL_2_CLIP_DISABLE_SHIFT                          30
#define    SDT_DSP_SDT_CTRL_2_CLIP_DISABLE_MASK                           0x40000000
#define    SDT_DSP_SDT_CTRL_2_SOF_SLOPE_SHIFT                             6
#define    SDT_DSP_SDT_CTRL_2_SOF_SLOPE_MASK                              0x3FFFFFC0
#define    SDT_DSP_SDT_CTRL_2_UPPER_FIT_TAP_SHIFT                         0
#define    SDT_DSP_SDT_CTRL_2_UPPER_FIT_TAP_MASK                          0x0000003F

#define SDT_DSP_SDT_COEF_ADDR_OFFSET                                      0x00000006
#define SDT_DSP_SDT_COEF_ADDR_TYPE                                        UInt32
#define SDT_DSP_SDT_COEF_ADDR_RESERVED_MASK                               0xFFFFFE03
#define    SDT_DSP_SDT_COEF_ADDR_SDT_COEF_ADDR_SHIFT                      2
#define    SDT_DSP_SDT_COEF_ADDR_SDT_COEF_ADDR_MASK                       0x000001FC

#define SDT_DSP_SDT_COEF_DATA_OFFSET                                      0x00000008
#define SDT_DSP_SDT_COEF_DATA_TYPE                                        UInt32
#define SDT_DSP_SDT_COEF_DATA_RESERVED_MASK                               0xFF000000
#define    SDT_DSP_SDT_COEF_DATA_SDT_COEF_DATA_SHIFT                      0
#define    SDT_DSP_SDT_COEF_DATA_SDT_COEF_DATA_MASK                       0x00FFFFFF

#define SDT_DSP_SDT_CTRL_3_OFFSET                                         0x0000000A
#define SDT_DSP_SDT_CTRL_3_TYPE                                           UInt32
#define SDT_DSP_SDT_CTRL_3_RESERVED_MASK                                  0x00070000
#define    SDT_DSP_SDT_CTRL_3_EMPTY_ENTRY_SHIFT                           24
#define    SDT_DSP_SDT_CTRL_3_EMPTY_ENTRY_MASK                            0xFF000000
#define    SDT_DSP_SDT_CTRL_3_ALMOST_FULL_SHIFT                           23
#define    SDT_DSP_SDT_CTRL_3_ALMOST_FULL_MASK                            0x00800000
#define    SDT_DSP_SDT_CTRL_3_ALMOST_EMPTY_SHIFT                          22
#define    SDT_DSP_SDT_CTRL_3_ALMOST_EMPTY_MASK                           0x00400000
#define    SDT_DSP_SDT_CTRL_3_UDF_SHIFT                                   21
#define    SDT_DSP_SDT_CTRL_3_UDF_MASK                                    0x00200000
#define    SDT_DSP_SDT_CTRL_3_OVF_SHIFT                                   20
#define    SDT_DSP_SDT_CTRL_3_OVF_MASK                                    0x00100000
#define    SDT_DSP_SDT_CTRL_3_THR_MET_SHIFT                               19
#define    SDT_DSP_SDT_CTRL_3_THR_MET_MASK                                0x00080000
#define    SDT_DSP_SDT_CTRL_3_MODE16BIT_SHIFT                             15
#define    SDT_DSP_SDT_CTRL_3_MODE16BIT_MASK                              0x00008000
#define    SDT_DSP_SDT_CTRL_3_ERR_INTR_EN_SHIFT                           14
#define    SDT_DSP_SDT_CTRL_3_ERR_INTR_EN_MASK                            0x00004000
#define    SDT_DSP_SDT_CTRL_3_INTR_EN_SHIFT                               13
#define    SDT_DSP_SDT_CTRL_3_INTR_EN_MASK                                0x00002000
#define    SDT_DSP_SDT_CTRL_3_THRES2_SHIFT                                9
#define    SDT_DSP_SDT_CTRL_3_THRES2_MASK                                 0x00001E00
#define    SDT_DSP_SDT_CTRL_3_THRES_SHIFT                                 2
#define    SDT_DSP_SDT_CTRL_3_THRES_MASK                                  0x000001FC
#define    SDT_DSP_SDT_CTRL_3_FIFO_RESET_SHIFT                            1
#define    SDT_DSP_SDT_CTRL_3_FIFO_RESET_MASK                             0x00000002
#define    SDT_DSP_SDT_CTRL_3_FIFO_ENABLE_SHIFT                           0
#define    SDT_DSP_SDT_CTRL_3_FIFO_ENABLE_MASK                            0x00000001

#define SDT_DSP_SDT_OUTPUTFIFO_OFFSET                                     0x0000000C
#define SDT_DSP_SDT_OUTPUTFIFO_TYPE                                       UInt32
#define SDT_DSP_SDT_OUTPUTFIFO_RESERVED_MASK                              0x00000000
#define    SDT_DSP_SDT_OUTPUTFIFO_SDT_OUTPUTFIFO_SHIFT                    0
#define    SDT_DSP_SDT_OUTPUTFIFO_SDT_OUTPUTFIFO_MASK                     0xFFFFFFFF

#endif /* __BRCM_RDB_SDT_H__ */


