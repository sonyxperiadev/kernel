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

#ifndef __BRCM_RDB_DSP_TRACE_H__
#define __BRCM_RDB_DSP_TRACE_H__

#define DSP_TRACE_TPI_REG_OFFSET                                          0x0000E0C2
#define DSP_TRACE_TPI_REG_TYPE                                            UInt32
#define DSP_TRACE_TPI_REG_RESERVED_MASK                                   0x00000000
#define    DSP_TRACE_TPI_REG_TPI_SHIFT                                    0
#define    DSP_TRACE_TPI_REG_TPI_MASK                                     0xFFFFFFFF

#define DSP_TRACE_IC_REG_OFFSET                                           0x0000E0C4
#define DSP_TRACE_IC_REG_TYPE                                             UInt32
#define DSP_TRACE_IC_REG_RESERVED_MASK                                    0x00000000
#define    DSP_TRACE_IC_REG_RESERVED_31TO1_SHIFT                          1
#define    DSP_TRACE_IC_REG_RESERVED_31TO1_MASK                           0xFFFFFFFE
#define    DSP_TRACE_IC_REG_TPI_OUT_PORT_SEL_SHIFT                        0
#define    DSP_TRACE_IC_REG_TPI_OUT_PORT_SEL_MASK                         0x00000001

#define DSP_TRACE_ATBID_REG_OFFSET                                        0x0000E0C6
#define DSP_TRACE_ATBID_REG_TYPE                                          UInt32
#define DSP_TRACE_ATBID_REG_RESERVED_MASK                                 0x00000000
#define    DSP_TRACE_ATBID_REG_RESERVED_31TO7_SHIFT                       7
#define    DSP_TRACE_ATBID_REG_RESERVED_31TO7_MASK                        0xFFFFFF80
#define    DSP_TRACE_ATBID_REG_ATBID_SHIFT                                1
#define    DSP_TRACE_ATBID_REG_ATBID_MASK                                 0x0000007E
#define    DSP_TRACE_ATBID_REG_ATB_SOURCE_SHIFT                           0
#define    DSP_TRACE_ATBID_REG_ATB_SOURCE_MASK                            0x00000001

#define DSP_TRACE_ATBC_REG_OFFSET                                         0x0000E0C8
#define DSP_TRACE_ATBC_REG_TYPE                                           UInt32
#define DSP_TRACE_ATBC_REG_RESERVED_MASK                                  0x00000000
#define    DSP_TRACE_ATBC_REG_RESERVED_31TO30_SHIFT                       30
#define    DSP_TRACE_ATBC_REG_RESERVED_31TO30_MASK                        0xC0000000
#define    DSP_TRACE_ATBC_REG_DROP_CNT_MAX_CLR_SHIFT                      29
#define    DSP_TRACE_ATBC_REG_DROP_CNT_MAX_CLR_MASK                       0x20000000
#define    DSP_TRACE_ATBC_REG_DROP_CNT_MAX_SHIFT                          24
#define    DSP_TRACE_ATBC_REG_DROP_CNT_MAX_MASK                           0x1F000000
#define    DSP_TRACE_ATBC_REG_RESERVED_23TO21_SHIFT                       21
#define    DSP_TRACE_ATBC_REG_RESERVED_23TO21_MASK                        0x00E00000
#define    DSP_TRACE_ATBC_REG_DECIMATION_SHIFT                            16
#define    DSP_TRACE_ATBC_REG_DECIMATION_MASK                             0x001F0000
#define    DSP_TRACE_ATBC_REG_RESERVED_15TO2_SHIFT                        2
#define    DSP_TRACE_ATBC_REG_RESERVED_15TO2_MASK                         0x0000FFFC
#define    DSP_TRACE_ATBC_REG_ATB_SOURCE_SEL_SHIFT                        0
#define    DSP_TRACE_ATBC_REG_ATB_SOURCE_SEL_MASK                         0x00000003

#define DSP_TRACE_TRACEC_REG_OFFSET                                       0x0000E0CA
#define DSP_TRACE_TRACEC_REG_TYPE                                         UInt32
#define DSP_TRACE_TRACEC_REG_RESERVED_MASK                                0x00000000
#define    DSP_TRACE_TRACEC_REG_RESERVED_31TO17_SHIFT                     17
#define    DSP_TRACE_TRACEC_REG_RESERVED_31TO17_MASK                      0xFFFE0000
#define    DSP_TRACE_TRACEC_REG_FORCE_SYNC_SHIFT                          16
#define    DSP_TRACE_TRACEC_REG_FORCE_SYNC_MASK                           0x00010000
#define    DSP_TRACE_TRACEC_REG_RESERVED_15TO6_SHIFT                      6
#define    DSP_TRACE_TRACEC_REG_RESERVED_15TO6_MASK                       0x0000FFC0
#define    DSP_TRACE_TRACEC_REG_TRACE_WIDTH16_SHIFT                       5
#define    DSP_TRACE_TRACEC_REG_TRACE_WIDTH16_MASK                        0x00000020
#define    DSP_TRACE_TRACEC_REG_TRACE_CLK_EN_SHIFT                        4
#define    DSP_TRACE_TRACEC_REG_TRACE_CLK_EN_MASK                         0x00000010
#define    DSP_TRACE_TRACEC_REG_BOTHEDGE_SHIFT                            3
#define    DSP_TRACE_TRACEC_REG_BOTHEDGE_MASK                             0x00000008
#define    DSP_TRACE_TRACEC_REG_FULLCLK_SHIFT                             2
#define    DSP_TRACE_TRACEC_REG_FULLCLK_MASK                              0x00000004
#define    DSP_TRACE_TRACEC_REG_TRACE_WIDTH_SHIFT                         1
#define    DSP_TRACE_TRACEC_REG_TRACE_WIDTH_MASK                          0x00000002
#define    DSP_TRACE_TRACEC_REG_TRACE_SOURCE_SEL_SHIFT                    0
#define    DSP_TRACE_TRACEC_REG_TRACE_SOURCE_SEL_MASK                     0x00000001

#define DSP_TRACE_TPI_R_OFFSET                                            0x00000184
#define DSP_TRACE_TPI_R_TYPE                                              UInt32
#define DSP_TRACE_TPI_R_RESERVED_MASK                                     0x00000000
#define    DSP_TRACE_TPI_R_TPI_SHIFT                                      0
#define    DSP_TRACE_TPI_R_TPI_MASK                                       0xFFFFFFFF

#define DSP_TRACE_IC_R_OFFSET                                             0x00000188
#define DSP_TRACE_IC_R_TYPE                                               UInt32
#define DSP_TRACE_IC_R_RESERVED_MASK                                      0x00000000
#define    DSP_TRACE_IC_R_RESERVED_31TO1_SHIFT                            1
#define    DSP_TRACE_IC_R_RESERVED_31TO1_MASK                             0xFFFFFFFE
#define    DSP_TRACE_IC_R_TPI_OUT_PORT_SEL_SHIFT                          0
#define    DSP_TRACE_IC_R_TPI_OUT_PORT_SEL_MASK                           0x00000001

#define DSP_TRACE_ATBID_R_OFFSET                                          0x0000018C
#define DSP_TRACE_ATBID_R_TYPE                                            UInt32
#define DSP_TRACE_ATBID_R_RESERVED_MASK                                   0x00000000
#define    DSP_TRACE_ATBID_R_RESERVED_31TO7_SHIFT                         7
#define    DSP_TRACE_ATBID_R_RESERVED_31TO7_MASK                          0xFFFFFF80
#define    DSP_TRACE_ATBID_R_ATBID_SHIFT                                  1
#define    DSP_TRACE_ATBID_R_ATBID_MASK                                   0x0000007E
#define    DSP_TRACE_ATBID_R_ATB_SOURCE_SHIFT                             0
#define    DSP_TRACE_ATBID_R_ATB_SOURCE_MASK                              0x00000001

#define DSP_TRACE_ATBC_R_OFFSET                                           0x00000190
#define DSP_TRACE_ATBC_R_TYPE                                             UInt32
#define DSP_TRACE_ATBC_R_RESERVED_MASK                                    0x00000000
#define    DSP_TRACE_ATBC_R_RESERVED_31TO30_SHIFT                         30
#define    DSP_TRACE_ATBC_R_RESERVED_31TO30_MASK                          0xC0000000
#define    DSP_TRACE_ATBC_R_DROP_CNT_MAX_CLR_SHIFT                        29
#define    DSP_TRACE_ATBC_R_DROP_CNT_MAX_CLR_MASK                         0x20000000
#define    DSP_TRACE_ATBC_R_DROP_CNT_MAX_SHIFT                            24
#define    DSP_TRACE_ATBC_R_DROP_CNT_MAX_MASK                             0x1F000000
#define    DSP_TRACE_ATBC_R_RESERVED_23TO21_SHIFT                         21
#define    DSP_TRACE_ATBC_R_RESERVED_23TO21_MASK                          0x00E00000
#define    DSP_TRACE_ATBC_R_DECIMATION_SHIFT                              16
#define    DSP_TRACE_ATBC_R_DECIMATION_MASK                               0x001F0000
#define    DSP_TRACE_ATBC_R_RESERVED_15TO2_SHIFT                          2
#define    DSP_TRACE_ATBC_R_RESERVED_15TO2_MASK                           0x0000FFFC
#define    DSP_TRACE_ATBC_R_ATB_SOURCE_SEL_SHIFT                          0
#define    DSP_TRACE_ATBC_R_ATB_SOURCE_SEL_MASK                           0x00000003

#define DSP_TRACE_TRACEC_R_OFFSET                                         0x00000194
#define DSP_TRACE_TRACEC_R_TYPE                                           UInt32
#define DSP_TRACE_TRACEC_R_RESERVED_MASK                                  0x00000000
#define    DSP_TRACE_TRACEC_R_RESERVED_31TO17_SHIFT                       17
#define    DSP_TRACE_TRACEC_R_RESERVED_31TO17_MASK                        0xFFFE0000
#define    DSP_TRACE_TRACEC_R_FORCE_SYNC_SHIFT                            16
#define    DSP_TRACE_TRACEC_R_FORCE_SYNC_MASK                             0x00010000
#define    DSP_TRACE_TRACEC_R_RESERVED_15TO6_SHIFT                        6
#define    DSP_TRACE_TRACEC_R_RESERVED_15TO6_MASK                         0x0000FFC0
#define    DSP_TRACE_TRACEC_R_TRACE_WIDTH16_SHIFT                         5
#define    DSP_TRACE_TRACEC_R_TRACE_WIDTH16_MASK                          0x00000020
#define    DSP_TRACE_TRACEC_R_TRACE_CLK_EN_SHIFT                          4
#define    DSP_TRACE_TRACEC_R_TRACE_CLK_EN_MASK                           0x00000010
#define    DSP_TRACE_TRACEC_R_BOTHEDGE_SHIFT                              3
#define    DSP_TRACE_TRACEC_R_BOTHEDGE_MASK                               0x00000008
#define    DSP_TRACE_TRACEC_R_FULLCLK_SHIFT                               2
#define    DSP_TRACE_TRACEC_R_FULLCLK_MASK                                0x00000004
#define    DSP_TRACE_TRACEC_R_TRACE_WIDTH_SHIFT                           1
#define    DSP_TRACE_TRACEC_R_TRACE_WIDTH_MASK                            0x00000002
#define    DSP_TRACE_TRACEC_R_TRACE_SOURCE_SEL_SHIFT                      0
#define    DSP_TRACE_TRACEC_R_TRACE_SOURCE_SEL_MASK                       0x00000001

#endif /* __BRCM_RDB_DSP_TRACE_H__ */


