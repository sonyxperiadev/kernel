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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_CSR_H__
#define __BRCM_RDB_CSR_H__

#define CSR_REV_ID_OFFSET                                                 0x00000000
#define CSR_REV_ID_TYPE                                                   UInt32
#define CSR_REV_ID_RESERVED_MASK                                          0xFFFF0000
#define    CSR_REV_ID_MAJOR_SHIFT                                         8
#define    CSR_REV_ID_MAJOR_MASK                                          0x0000FF00
#define    CSR_REV_ID_MINOR_SHIFT                                         0
#define    CSR_REV_ID_MINOR_MASK                                          0x000000FF

#define CSR_SOFT_RESET_OFFSET                                             0x00000004
#define CSR_SOFT_RESET_TYPE                                               UInt32
#define CSR_SOFT_RESET_RESERVED_MASK                                      0xFFFFFFFE
#define    CSR_SOFT_RESET_MEMC_CORE_SW_RST_SHIFT                          0
#define    CSR_SOFT_RESET_MEMC_CORE_SW_RST_MASK                           0x00000001

#define CSR_SW_INIT_DONE_OFFSET                                           0x00000008
#define CSR_SW_INIT_DONE_TYPE                                             UInt32
#define CSR_SW_INIT_DONE_RESERVED_MASK                                    0xFFFFFFFE
#define    CSR_SW_INIT_DONE_DONE_SHIFT                                    0
#define    CSR_SW_INIT_DONE_DONE_MASK                                     0x00000001

#define CSR_DRAM_CALIBRATION_MODE_OFFSET                                  0x0000000C
#define CSR_DRAM_CALIBRATION_MODE_TYPE                                    UInt32
#define CSR_DRAM_CALIBRATION_MODE_RESERVED_MASK                           0xFFFFFFFE
#define    CSR_DRAM_CALIBRATION_MODE_DRAM_CALIBRATION_MODE_SHIFT          0
#define    CSR_DRAM_CALIBRATION_MODE_DRAM_CALIBRATION_MODE_MASK           0x00000001

#define CSR_DRAM_INIT_CONTROL_OFFSET                                      0x00000014
#define CSR_DRAM_INIT_CONTROL_TYPE                                        UInt32
#define CSR_DRAM_INIT_CONTROL_RESERVED_MASK                               0x00C00000
#define    CSR_DRAM_INIT_CONTROL_MRW_DATA_DDR3_EXTN_SHIFT                 24
#define    CSR_DRAM_INIT_CONTROL_MRW_DATA_DDR3_EXTN_MASK                  0xFF000000
#define    CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT                            20
#define    CSR_DRAM_INIT_CONTROL_CS_BITS_MASK                             0x00300000
#define    CSR_DRAM_INIT_CONTROL_INIT_CMD_SHIFT                           16
#define    CSR_DRAM_INIT_CONTROL_INIT_CMD_MASK                            0x000F0000
#define    CSR_DRAM_INIT_CONTROL_MRW_DATA_SHIFT                           8
#define    CSR_DRAM_INIT_CONTROL_MRW_DATA_MASK                            0x0000FF00
#define    CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT                            0
#define    CSR_DRAM_INIT_CONTROL_MR_ADDR_MASK                             0x000000FF

#define CSR_DRAM_INIT_RESULT_OFFSET                                       0x00000018
#define CSR_DRAM_INIT_RESULT_TYPE                                         UInt32
#define CSR_DRAM_INIT_RESULT_RESERVED_MASK                                0xFFFF00FC
#define    CSR_DRAM_INIT_RESULT_MRR_DATA_SHIFT                            8
#define    CSR_DRAM_INIT_RESULT_MRR_DATA_MASK                             0x0000FF00
#define    CSR_DRAM_INIT_RESULT_MRR_VALID_SHIFT                           1
#define    CSR_DRAM_INIT_RESULT_MRR_VALID_MASK                            0x00000002
#define    CSR_DRAM_INIT_RESULT_BUSY_SHIFT                                0
#define    CSR_DRAM_INIT_RESULT_BUSY_MASK                                 0x00000001

#define CSR_AXI_PORT_CTRL_OFFSET                                          0x0000001C
#define CSR_AXI_PORT_CTRL_TYPE                                            UInt32
#define CSR_AXI_PORT_CTRL_RESERVED_MASK                                   0xFFFFFF00
#define    CSR_AXI_PORT_CTRL_PORT3_START_STOP_SHIFT                       7
#define    CSR_AXI_PORT_CTRL_PORT3_START_STOP_MASK                        0x00000080
#define    CSR_AXI_PORT_CTRL_PORT2_START_STOP_SHIFT                       6
#define    CSR_AXI_PORT_CTRL_PORT2_START_STOP_MASK                        0x00000040
#define    CSR_AXI_PORT_CTRL_PORT1_START_STOP_SHIFT                       5
#define    CSR_AXI_PORT_CTRL_PORT1_START_STOP_MASK                        0x00000020
#define    CSR_AXI_PORT_CTRL_PORT0_START_STOP_SHIFT                       4
#define    CSR_AXI_PORT_CTRL_PORT0_START_STOP_MASK                        0x00000010
#define    CSR_AXI_PORT_CTRL_PORT3_DISABLE_SHIFT                          3
#define    CSR_AXI_PORT_CTRL_PORT3_DISABLE_MASK                           0x00000008
#define    CSR_AXI_PORT_CTRL_PORT2_DISABLE_SHIFT                          2
#define    CSR_AXI_PORT_CTRL_PORT2_DISABLE_MASK                           0x00000004
#define    CSR_AXI_PORT_CTRL_PORT1_DISABLE_SHIFT                          1
#define    CSR_AXI_PORT_CTRL_PORT1_DISABLE_MASK                           0x00000002
#define    CSR_AXI_PORT_CTRL_PORT0_DISABLE_SHIFT                          0
#define    CSR_AXI_PORT_CTRL_PORT0_DISABLE_MASK                           0x00000001

#define CSR_MEMC_SWAP_CS0_CS1_OFFSET                                      0x00000020
#define CSR_MEMC_SWAP_CS0_CS1_TYPE                                        UInt32
#define CSR_MEMC_SWAP_CS0_CS1_RESERVED_MASK                               0xFFFFFFFE
#define    CSR_MEMC_SWAP_CS0_CS1_SWAP_CS_SHIFT                            0
#define    CSR_MEMC_SWAP_CS0_CS1_SWAP_CS_MASK                             0x00000001

#define CSR_CAM_UPDATE_OFFSET                                             0x0000003C
#define CSR_CAM_UPDATE_TYPE                                               UInt32
#define CSR_CAM_UPDATE_RESERVED_MASK                                      0xFFFFFFFE
#define    CSR_CAM_UPDATE_CAM_UPDATE_EN_SHIFT                             0
#define    CSR_CAM_UPDATE_CAM_UPDATE_EN_MASK                              0x00000001

#define CSR_CAM_ENTRY_0_OFFSET                                            0x00000040
#define CSR_CAM_ENTRY_0_TYPE                                              UInt32
#define CSR_CAM_ENTRY_0_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_SHIFT                        27
#define    CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK                         0x08000000
#define    CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_SHIFT                       26
#define    CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK                        0x04000000
#define    CSR_CAM_ENTRY_0_CAM_PRIORITY_0_SHIFT                           25
#define    CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK                            0x02000000
#define    CSR_CAM_ENTRY_0_CAM_LATENCY_0_SHIFT                            24
#define    CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK                             0x01000000
#define    CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT                            12
#define    CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_0_CAM_THOLD_0_SHIFT                              0
#define    CSR_CAM_ENTRY_0_CAM_THOLD_0_MASK                               0x000003FF

#define CSR_CAM_ENTRY_1_OFFSET                                            0x00000044
#define CSR_CAM_ENTRY_1_TYPE                                              UInt32
#define CSR_CAM_ENTRY_1_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_1_CAM_READ_ENABLE_1_SHIFT                        27
#define    CSR_CAM_ENTRY_1_CAM_READ_ENABLE_1_MASK                         0x08000000
#define    CSR_CAM_ENTRY_1_CAM_WRITE_ENABLE_1_SHIFT                       26
#define    CSR_CAM_ENTRY_1_CAM_WRITE_ENABLE_1_MASK                        0x04000000
#define    CSR_CAM_ENTRY_1_CAM_PRIORITY_1_SHIFT                           25
#define    CSR_CAM_ENTRY_1_CAM_PRIORITY_1_MASK                            0x02000000
#define    CSR_CAM_ENTRY_1_CAM_LATENCY_1_SHIFT                            24
#define    CSR_CAM_ENTRY_1_CAM_LATENCY_1_MASK                             0x01000000
#define    CSR_CAM_ENTRY_1_CAM_URG_CTR_1_SHIFT                            12
#define    CSR_CAM_ENTRY_1_CAM_URG_CTR_1_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_1_CAM_THOLD_1_SHIFT                              0
#define    CSR_CAM_ENTRY_1_CAM_THOLD_1_MASK                               0x000003FF

#define CSR_CAM_ENTRY_2_OFFSET                                            0x00000048
#define CSR_CAM_ENTRY_2_TYPE                                              UInt32
#define CSR_CAM_ENTRY_2_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_2_CAM_READ_ENABLE_2_SHIFT                        27
#define    CSR_CAM_ENTRY_2_CAM_READ_ENABLE_2_MASK                         0x08000000
#define    CSR_CAM_ENTRY_2_CAM_WRITE_ENABLE_2_SHIFT                       26
#define    CSR_CAM_ENTRY_2_CAM_WRITE_ENABLE_2_MASK                        0x04000000
#define    CSR_CAM_ENTRY_2_CAM_PRIORITY_2_SHIFT                           25
#define    CSR_CAM_ENTRY_2_CAM_PRIORITY_2_MASK                            0x02000000
#define    CSR_CAM_ENTRY_2_CAM_LATENCY_2_SHIFT                            24
#define    CSR_CAM_ENTRY_2_CAM_LATENCY_2_MASK                             0x01000000
#define    CSR_CAM_ENTRY_2_CAM_URG_CTR_2_SHIFT                            12
#define    CSR_CAM_ENTRY_2_CAM_URG_CTR_2_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_2_CAM_THOLD_2_SHIFT                              0
#define    CSR_CAM_ENTRY_2_CAM_THOLD_2_MASK                               0x000003FF

#define CSR_CAM_ENTRY_3_OFFSET                                            0x0000004C
#define CSR_CAM_ENTRY_3_TYPE                                              UInt32
#define CSR_CAM_ENTRY_3_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_3_CAM_READ_ENABLE_3_SHIFT                        27
#define    CSR_CAM_ENTRY_3_CAM_READ_ENABLE_3_MASK                         0x08000000
#define    CSR_CAM_ENTRY_3_CAM_WRITE_ENABLE_3_SHIFT                       26
#define    CSR_CAM_ENTRY_3_CAM_WRITE_ENABLE_3_MASK                        0x04000000
#define    CSR_CAM_ENTRY_3_CAM_PRIORITY_3_SHIFT                           25
#define    CSR_CAM_ENTRY_3_CAM_PRIORITY_3_MASK                            0x02000000
#define    CSR_CAM_ENTRY_3_CAM_LATENCY_3_SHIFT                            24
#define    CSR_CAM_ENTRY_3_CAM_LATENCY_3_MASK                             0x01000000
#define    CSR_CAM_ENTRY_3_CAM_URG_CTR_3_SHIFT                            12
#define    CSR_CAM_ENTRY_3_CAM_URG_CTR_3_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_3_CAM_THOLD_3_SHIFT                              0
#define    CSR_CAM_ENTRY_3_CAM_THOLD_3_MASK                               0x000003FF

#define CSR_CAM_ENTRY_4_OFFSET                                            0x00000050
#define CSR_CAM_ENTRY_4_TYPE                                              UInt32
#define CSR_CAM_ENTRY_4_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_4_CAM_READ_ENABLE_4_SHIFT                        27
#define    CSR_CAM_ENTRY_4_CAM_READ_ENABLE_4_MASK                         0x08000000
#define    CSR_CAM_ENTRY_4_CAM_WRITE_ENABLE_4_SHIFT                       26
#define    CSR_CAM_ENTRY_4_CAM_WRITE_ENABLE_4_MASK                        0x04000000
#define    CSR_CAM_ENTRY_4_CAM_PRIORITY_4_SHIFT                           25
#define    CSR_CAM_ENTRY_4_CAM_PRIORITY_4_MASK                            0x02000000
#define    CSR_CAM_ENTRY_4_CAM_LATENCY_4_SHIFT                            24
#define    CSR_CAM_ENTRY_4_CAM_LATENCY_4_MASK                             0x01000000
#define    CSR_CAM_ENTRY_4_CAM_URG_CTR_4_SHIFT                            12
#define    CSR_CAM_ENTRY_4_CAM_URG_CTR_4_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_4_CAM_THOLD_4_SHIFT                              0
#define    CSR_CAM_ENTRY_4_CAM_THOLD_4_MASK                               0x000003FF

#define CSR_CAM_ENTRY_5_OFFSET                                            0x00000054
#define CSR_CAM_ENTRY_5_TYPE                                              UInt32
#define CSR_CAM_ENTRY_5_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_5_CAM_READ_ENABLE_5_SHIFT                        27
#define    CSR_CAM_ENTRY_5_CAM_READ_ENABLE_5_MASK                         0x08000000
#define    CSR_CAM_ENTRY_5_CAM_WRITE_ENABLE_5_SHIFT                       26
#define    CSR_CAM_ENTRY_5_CAM_WRITE_ENABLE_5_MASK                        0x04000000
#define    CSR_CAM_ENTRY_5_CAM_PRIORITY_5_SHIFT                           25
#define    CSR_CAM_ENTRY_5_CAM_PRIORITY_5_MASK                            0x02000000
#define    CSR_CAM_ENTRY_5_CAM_LATENCY_5_SHIFT                            24
#define    CSR_CAM_ENTRY_5_CAM_LATENCY_5_MASK                             0x01000000
#define    CSR_CAM_ENTRY_5_CAM_URG_CTR_5_SHIFT                            12
#define    CSR_CAM_ENTRY_5_CAM_URG_CTR_5_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_5_CAM_THOLD_5_SHIFT                              0
#define    CSR_CAM_ENTRY_5_CAM_THOLD_5_MASK                               0x000003FF

#define CSR_CAM_ENTRY_6_OFFSET                                            0x00000058
#define CSR_CAM_ENTRY_6_TYPE                                              UInt32
#define CSR_CAM_ENTRY_6_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_6_CAM_READ_ENABLE_6_SHIFT                        27
#define    CSR_CAM_ENTRY_6_CAM_READ_ENABLE_6_MASK                         0x08000000
#define    CSR_CAM_ENTRY_6_CAM_WRITE_ENABLE_6_SHIFT                       26
#define    CSR_CAM_ENTRY_6_CAM_WRITE_ENABLE_6_MASK                        0x04000000
#define    CSR_CAM_ENTRY_6_CAM_PRIORITY_6_SHIFT                           25
#define    CSR_CAM_ENTRY_6_CAM_PRIORITY_6_MASK                            0x02000000
#define    CSR_CAM_ENTRY_6_CAM_LATENCY_6_SHIFT                            24
#define    CSR_CAM_ENTRY_6_CAM_LATENCY_6_MASK                             0x01000000
#define    CSR_CAM_ENTRY_6_CAM_URG_CTR_6_SHIFT                            12
#define    CSR_CAM_ENTRY_6_CAM_URG_CTR_6_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_6_CAM_THOLD_6_SHIFT                              0
#define    CSR_CAM_ENTRY_6_CAM_THOLD_6_MASK                               0x000003FF

#define CSR_CAM_ENTRY_7_OFFSET                                            0x0000005C
#define CSR_CAM_ENTRY_7_TYPE                                              UInt32
#define CSR_CAM_ENTRY_7_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_7_CAM_READ_ENABLE_7_SHIFT                        27
#define    CSR_CAM_ENTRY_7_CAM_READ_ENABLE_7_MASK                         0x08000000
#define    CSR_CAM_ENTRY_7_CAM_WRITE_ENABLE_7_SHIFT                       26
#define    CSR_CAM_ENTRY_7_CAM_WRITE_ENABLE_7_MASK                        0x04000000
#define    CSR_CAM_ENTRY_7_CAM_PRIORITY_7_SHIFT                           25
#define    CSR_CAM_ENTRY_7_CAM_PRIORITY_7_MASK                            0x02000000
#define    CSR_CAM_ENTRY_7_CAM_LATENCY_7_SHIFT                            24
#define    CSR_CAM_ENTRY_7_CAM_LATENCY_7_MASK                             0x01000000
#define    CSR_CAM_ENTRY_7_CAM_URG_CTR_7_SHIFT                            12
#define    CSR_CAM_ENTRY_7_CAM_URG_CTR_7_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_7_CAM_THOLD_7_SHIFT                              0
#define    CSR_CAM_ENTRY_7_CAM_THOLD_7_MASK                               0x000003FF

#define CSR_CAM_ENTRY_8_OFFSET                                            0x00000060
#define CSR_CAM_ENTRY_8_TYPE                                              UInt32
#define CSR_CAM_ENTRY_8_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_8_CAM_READ_ENABLE_8_SHIFT                        27
#define    CSR_CAM_ENTRY_8_CAM_READ_ENABLE_8_MASK                         0x08000000
#define    CSR_CAM_ENTRY_8_CAM_WRITE_ENABLE_8_SHIFT                       26
#define    CSR_CAM_ENTRY_8_CAM_WRITE_ENABLE_8_MASK                        0x04000000
#define    CSR_CAM_ENTRY_8_CAM_PRIORITY_8_SHIFT                           25
#define    CSR_CAM_ENTRY_8_CAM_PRIORITY_8_MASK                            0x02000000
#define    CSR_CAM_ENTRY_8_CAM_LATENCY_8_SHIFT                            24
#define    CSR_CAM_ENTRY_8_CAM_LATENCY_8_MASK                             0x01000000
#define    CSR_CAM_ENTRY_8_CAM_URG_CTR_8_SHIFT                            12
#define    CSR_CAM_ENTRY_8_CAM_URG_CTR_8_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_8_CAM_THOLD_8_SHIFT                              0
#define    CSR_CAM_ENTRY_8_CAM_THOLD_8_MASK                               0x000003FF

#define CSR_CAM_ENTRY_9_OFFSET                                            0x00000064
#define CSR_CAM_ENTRY_9_TYPE                                              UInt32
#define CSR_CAM_ENTRY_9_RESERVED_MASK                                     0xF0C00C00
#define    CSR_CAM_ENTRY_9_CAM_READ_ENABLE_9_SHIFT                        27
#define    CSR_CAM_ENTRY_9_CAM_READ_ENABLE_9_MASK                         0x08000000
#define    CSR_CAM_ENTRY_9_CAM_WRITE_ENABLE_9_SHIFT                       26
#define    CSR_CAM_ENTRY_9_CAM_WRITE_ENABLE_9_MASK                        0x04000000
#define    CSR_CAM_ENTRY_9_CAM_PRIORITY_9_SHIFT                           25
#define    CSR_CAM_ENTRY_9_CAM_PRIORITY_9_MASK                            0x02000000
#define    CSR_CAM_ENTRY_9_CAM_LATENCY_9_SHIFT                            24
#define    CSR_CAM_ENTRY_9_CAM_LATENCY_9_MASK                             0x01000000
#define    CSR_CAM_ENTRY_9_CAM_URG_CTR_9_SHIFT                            12
#define    CSR_CAM_ENTRY_9_CAM_URG_CTR_9_MASK                             0x003FF000
#define    CSR_CAM_ENTRY_9_CAM_THOLD_9_SHIFT                              0
#define    CSR_CAM_ENTRY_9_CAM_THOLD_9_MASK                               0x000003FF

#define CSR_CAM_ENTRY_10_OFFSET                                           0x00000068
#define CSR_CAM_ENTRY_10_TYPE                                             UInt32
#define CSR_CAM_ENTRY_10_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_10_CAM_READ_ENABLE_10_SHIFT                      27
#define    CSR_CAM_ENTRY_10_CAM_READ_ENABLE_10_MASK                       0x08000000
#define    CSR_CAM_ENTRY_10_CAM_WRITE_ENABLE_10_SHIFT                     26
#define    CSR_CAM_ENTRY_10_CAM_WRITE_ENABLE_10_MASK                      0x04000000
#define    CSR_CAM_ENTRY_10_CAM_PRIORITY_10_SHIFT                         25
#define    CSR_CAM_ENTRY_10_CAM_PRIORITY_10_MASK                          0x02000000
#define    CSR_CAM_ENTRY_10_CAM_LATENCY_10_SHIFT                          24
#define    CSR_CAM_ENTRY_10_CAM_LATENCY_10_MASK                           0x01000000
#define    CSR_CAM_ENTRY_10_CAM_URG_CTR_10_SHIFT                          12
#define    CSR_CAM_ENTRY_10_CAM_URG_CTR_10_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_10_CAM_THOLD_10_SHIFT                            0
#define    CSR_CAM_ENTRY_10_CAM_THOLD_10_MASK                             0x000003FF

#define CSR_CAM_ENTRY_11_OFFSET                                           0x0000006C
#define CSR_CAM_ENTRY_11_TYPE                                             UInt32
#define CSR_CAM_ENTRY_11_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_11_CAM_READ_ENABLE_11_SHIFT                      27
#define    CSR_CAM_ENTRY_11_CAM_READ_ENABLE_11_MASK                       0x08000000
#define    CSR_CAM_ENTRY_11_CAM_WRITE_ENABLE_11_SHIFT                     26
#define    CSR_CAM_ENTRY_11_CAM_WRITE_ENABLE_11_MASK                      0x04000000
#define    CSR_CAM_ENTRY_11_CAM_PRIORITY_11_SHIFT                         25
#define    CSR_CAM_ENTRY_11_CAM_PRIORITY_11_MASK                          0x02000000
#define    CSR_CAM_ENTRY_11_CAM_LATENCY_11_SHIFT                          24
#define    CSR_CAM_ENTRY_11_CAM_LATENCY_11_MASK                           0x01000000
#define    CSR_CAM_ENTRY_11_CAM_URG_CTR_11_SHIFT                          12
#define    CSR_CAM_ENTRY_11_CAM_URG_CTR_11_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_11_CAM_THOLD_11_SHIFT                            0
#define    CSR_CAM_ENTRY_11_CAM_THOLD_11_MASK                             0x000003FF

#define CSR_CAM_ENTRY_12_OFFSET                                           0x00000070
#define CSR_CAM_ENTRY_12_TYPE                                             UInt32
#define CSR_CAM_ENTRY_12_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_12_CAM_READ_ENABLE_12_SHIFT                      27
#define    CSR_CAM_ENTRY_12_CAM_READ_ENABLE_12_MASK                       0x08000000
#define    CSR_CAM_ENTRY_12_CAM_WRITE_ENABLE_12_SHIFT                     26
#define    CSR_CAM_ENTRY_12_CAM_WRITE_ENABLE_12_MASK                      0x04000000
#define    CSR_CAM_ENTRY_12_CAM_PRIORITY_12_SHIFT                         25
#define    CSR_CAM_ENTRY_12_CAM_PRIORITY_12_MASK                          0x02000000
#define    CSR_CAM_ENTRY_12_CAM_LATENCY_12_SHIFT                          24
#define    CSR_CAM_ENTRY_12_CAM_LATENCY_12_MASK                           0x01000000
#define    CSR_CAM_ENTRY_12_CAM_URG_CTR_12_SHIFT                          12
#define    CSR_CAM_ENTRY_12_CAM_URG_CTR_12_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_12_CAM_THOLD_12_SHIFT                            0
#define    CSR_CAM_ENTRY_12_CAM_THOLD_12_MASK                             0x000003FF

#define CSR_CAM_ENTRY_13_OFFSET                                           0x00000074
#define CSR_CAM_ENTRY_13_TYPE                                             UInt32
#define CSR_CAM_ENTRY_13_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_13_CAM_READ_ENABLE_13_SHIFT                      27
#define    CSR_CAM_ENTRY_13_CAM_READ_ENABLE_13_MASK                       0x08000000
#define    CSR_CAM_ENTRY_13_CAM_WRITE_ENABLE_13_SHIFT                     26
#define    CSR_CAM_ENTRY_13_CAM_WRITE_ENABLE_13_MASK                      0x04000000
#define    CSR_CAM_ENTRY_13_CAM_PRIORITY_13_SHIFT                         25
#define    CSR_CAM_ENTRY_13_CAM_PRIORITY_13_MASK                          0x02000000
#define    CSR_CAM_ENTRY_13_CAM_LATENCY_13_SHIFT                          24
#define    CSR_CAM_ENTRY_13_CAM_LATENCY_13_MASK                           0x01000000
#define    CSR_CAM_ENTRY_13_CAM_URG_CTR_13_SHIFT                          12
#define    CSR_CAM_ENTRY_13_CAM_URG_CTR_13_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_13_CAM_THOLD_13_SHIFT                            0
#define    CSR_CAM_ENTRY_13_CAM_THOLD_13_MASK                             0x000003FF

#define CSR_CAM_ENTRY_14_OFFSET                                           0x00000078
#define CSR_CAM_ENTRY_14_TYPE                                             UInt32
#define CSR_CAM_ENTRY_14_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_14_CAM_READ_ENABLE_14_SHIFT                      27
#define    CSR_CAM_ENTRY_14_CAM_READ_ENABLE_14_MASK                       0x08000000
#define    CSR_CAM_ENTRY_14_CAM_WRITE_ENABLE_14_SHIFT                     26
#define    CSR_CAM_ENTRY_14_CAM_WRITE_ENABLE_14_MASK                      0x04000000
#define    CSR_CAM_ENTRY_14_CAM_PRIORITY_14_SHIFT                         25
#define    CSR_CAM_ENTRY_14_CAM_PRIORITY_14_MASK                          0x02000000
#define    CSR_CAM_ENTRY_14_CAM_LATENCY_14_SHIFT                          24
#define    CSR_CAM_ENTRY_14_CAM_LATENCY_14_MASK                           0x01000000
#define    CSR_CAM_ENTRY_14_CAM_URG_CTR_14_SHIFT                          12
#define    CSR_CAM_ENTRY_14_CAM_URG_CTR_14_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_14_CAM_THOLD_14_SHIFT                            0
#define    CSR_CAM_ENTRY_14_CAM_THOLD_14_MASK                             0x000003FF

#define CSR_CAM_ENTRY_15_OFFSET                                           0x0000007C
#define CSR_CAM_ENTRY_15_TYPE                                             UInt32
#define CSR_CAM_ENTRY_15_RESERVED_MASK                                    0xF0C00C00
#define    CSR_CAM_ENTRY_15_CAM_READ_ENABLE_15_SHIFT                      27
#define    CSR_CAM_ENTRY_15_CAM_READ_ENABLE_15_MASK                       0x08000000
#define    CSR_CAM_ENTRY_15_CAM_WRITE_ENABLE_15_SHIFT                     26
#define    CSR_CAM_ENTRY_15_CAM_WRITE_ENABLE_15_MASK                      0x04000000
#define    CSR_CAM_ENTRY_15_CAM_PRIORITY_15_SHIFT                         25
#define    CSR_CAM_ENTRY_15_CAM_PRIORITY_15_MASK                          0x02000000
#define    CSR_CAM_ENTRY_15_CAM_LATENCY_15_SHIFT                          24
#define    CSR_CAM_ENTRY_15_CAM_LATENCY_15_MASK                           0x01000000
#define    CSR_CAM_ENTRY_15_CAM_URG_CTR_15_SHIFT                          12
#define    CSR_CAM_ENTRY_15_CAM_URG_CTR_15_MASK                           0x003FF000
#define    CSR_CAM_ENTRY_15_CAM_THOLD_15_SHIFT                            0
#define    CSR_CAM_ENTRY_15_CAM_THOLD_15_MASK                             0x000003FF

#define CSR_DRAM_TIMING0_OFFSET                                           0x000000A0
#define CSR_DRAM_TIMING0_TYPE                                             UInt32
#define CSR_DRAM_TIMING0_RESERVED_MASK                                    0xE0080080
#define    CSR_DRAM_TIMING0_TRAS_SHIFT                                    24
#define    CSR_DRAM_TIMING0_TRAS_MASK                                     0x1F000000
#define    CSR_DRAM_TIMING0_TWR_SHIFT                                     20
#define    CSR_DRAM_TIMING0_TWR_MASK                                      0x00F00000
#define    CSR_DRAM_TIMING0_TRTP_SHIFT                                    16
#define    CSR_DRAM_TIMING0_TRTP_MASK                                     0x00070000
#define    CSR_DRAM_TIMING0_TRP_PB_SHIFT                                  12
#define    CSR_DRAM_TIMING0_TRP_PB_MASK                                   0x0000F000
#define    CSR_DRAM_TIMING0_TRP_AB_SHIFT                                  8
#define    CSR_DRAM_TIMING0_TRP_AB_MASK                                   0x00000F00
#define    CSR_DRAM_TIMING0_TRRD_SHIFT                                    4
#define    CSR_DRAM_TIMING0_TRRD_MASK                                     0x00000070
#define    CSR_DRAM_TIMING0_TRCD_SHIFT                                    0
#define    CSR_DRAM_TIMING0_TRCD_MASK                                     0x0000000F

#define CSR_DRAM_TIMING1_OFFSET                                           0x000000A4
#define CSR_DRAM_TIMING1_TYPE                                             UInt32
#define CSR_DRAM_TIMING1_RESERVED_MASK                                    0xC0008E08
#define    CSR_DRAM_TIMING1_TDQSCK_SHIFT                                  28
#define    CSR_DRAM_TIMING1_TDQSCK_MASK                                   0x30000000
#define    CSR_DRAM_TIMING1_TXSR_SHIFT                                    20
#define    CSR_DRAM_TIMING1_TXSR_MASK                                     0x0FF00000
#define    CSR_DRAM_TIMING1_TCKESR_SHIFT                                  16
#define    CSR_DRAM_TIMING1_TCKESR_MASK                                   0x000F0000
#define    CSR_DRAM_TIMING1_TXP_SHIFT                                     12
#define    CSR_DRAM_TIMING1_TXP_MASK                                      0x00007000
#define    CSR_DRAM_TIMING1_TFAW_SHIFT                                    4
#define    CSR_DRAM_TIMING1_TFAW_MASK                                     0x000001F0
#define    CSR_DRAM_TIMING1_TWTR_SHIFT                                    0
#define    CSR_DRAM_TIMING1_TWTR_MASK                                     0x00000007

#define CSR_DRAM_ZQ_CALIBRATION_TIMING_OFFSET                             0x000000A8
#define CSR_DRAM_ZQ_CALIBRATION_TIMING_TYPE                               UInt32
#define CSR_DRAM_ZQ_CALIBRATION_TIMING_RESERVED_MASK                      0xFFFF00C0
#define    CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCL_SHIFT                     8
#define    CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCL_MASK                      0x0000FF00
#define    CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCS_SHIFT                     0
#define    CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCS_MASK                      0x0000003F

#define CSR_NVM_TIMING0_OFFSET                                            0x000000AC
#define CSR_NVM_TIMING0_TYPE                                              UInt32
#define CSR_NVM_TIMING0_RESERVED_MASK                                     0xFC000000
#define    CSR_NVM_TIMING0_TRP_NVM_SHIFT                                  24
#define    CSR_NVM_TIMING0_TRP_NVM_MASK                                   0x03000000
#define    CSR_NVM_TIMING0_TRAS_NVM_SHIFT                                 16
#define    CSR_NVM_TIMING0_TRAS_NVM_MASK                                  0x00FF0000
#define    CSR_NVM_TIMING0_TRRD_NVM_SHIFT                                 8
#define    CSR_NVM_TIMING0_TRRD_NVM_MASK                                  0x0000FF00
#define    CSR_NVM_TIMING0_TRCD_NVM_SHIFT                                 0
#define    CSR_NVM_TIMING0_TRCD_NVM_MASK                                  0x000000FF

#define CSR_REFRESH_CNTRL_OFFSET                                          0x000000B0
#define CSR_REFRESH_CNTRL_TYPE                                            UInt32
#define CSR_REFRESH_CNTRL_RESERVED_MASK                                   0xF8000000
#define    CSR_REFRESH_CNTRL_PENDING_REFRESH_PRIORITY_COUNT_SHIFT         24
#define    CSR_REFRESH_CNTRL_PENDING_REFRESH_PRIORITY_COUNT_MASK          0x07000000
#define    CSR_REFRESH_CNTRL_TRFC_SHIFT                                   16
#define    CSR_REFRESH_CNTRL_TRFC_MASK                                    0x00FF0000
#define    CSR_REFRESH_CNTRL_REFRESH_PERIOD_SHIFT                         0
#define    CSR_REFRESH_CNTRL_REFRESH_PERIOD_MASK                          0x0000FFFF

#define CSR_DDR2_MR2_OFFSET                                               0x000000B4
#define CSR_DDR2_MR2_TYPE                                                 UInt32
#define CSR_DDR2_MR2_RESERVED_MASK                                        0xFFFFFFF0
#define    CSR_DDR2_MR2_RL_WL_SHIFT                                       0
#define    CSR_DDR2_MR2_RL_WL_MASK                                        0x0000000F

#define CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_OFFSET                        0x000000B8
#define CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_TYPE                          UInt32
#define CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_RESERVED_MASK                 0xE0000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_CALIBRATION_MODE_SHIFT     28
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_CALIBRATION_MODE_MASK      0x10000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_DPHY_ZQ_CALIBRATION_SHIFT 27
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_DPHY_ZQ_CALIBRATION_MASK 0x08000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_APHY_ZQ_CALIBRATION_SHIFT 26
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_APHY_ZQ_CALIBRATION_MASK 0x04000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS1_CALIBRATION_SHIFT 25
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS1_CALIBRATION_MASK 0x02000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS0_CALIBRATION_SHIFT 24
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS0_CALIBRATION_MASK 0x01000000
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_CALIBRATION_PERIOD_SHIFT   0
#define    CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_CALIBRATION_PERIOD_MASK    0x00FFFFFF

#define CSR_DRAM_ZQ_ADDR_DATA_CODE_OFFSET                                 0x000000BC
#define CSR_DRAM_ZQ_ADDR_DATA_CODE_TYPE                                   UInt32
#define CSR_DRAM_ZQ_ADDR_DATA_CODE_RESERVED_MASK                          0xFF000000
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRDATA_SHORT_SHIFT               16
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRDATA_SHORT_MASK                0x00FF0000
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRDATA_LONG_SHIFT                8
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRDATA_LONG_MASK                 0x0000FF00
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRADDR_SHIFT                     0
#define    CSR_DRAM_ZQ_ADDR_DATA_CODE_ZQ_MRADDR_MASK                      0x000000FF

#define CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET                                 0x000000C0
#define CSR_LPDDR2_DEV_TEMP_PERIOD_TYPE                                   UInt32
#define CSR_LPDDR2_DEV_TEMP_PERIOD_RESERVED_MASK                          0xFC000000
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_SHIFT       25
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_MASK        0x02000000
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_SHIFT       24
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_MASK        0x01000000
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_SHIFT               0
#define    CSR_LPDDR2_DEV_TEMP_PERIOD_DEV_TEMP_PERIOD_MASK                0x00FFFFFF

#define CSR_LPDDR2_DEV_TEMP_ADDR_OFFSET                                   0x000000C4
#define CSR_LPDDR2_DEV_TEMP_ADDR_TYPE                                     UInt32
#define CSR_LPDDR2_DEV_TEMP_ADDR_RESERVED_MASK                            0xFFFFFF00
#define    CSR_LPDDR2_DEV_TEMP_ADDR_DEV_TEMP_MRADDR_SHIFT                 0
#define    CSR_LPDDR2_DEV_TEMP_ADDR_DEV_TEMP_MRADDR_MASK                  0x000000FF

#define CSR_LPDDR2_DEV_TEMP_STATUS_OFFSET                                 0x000000C8
#define CSR_LPDDR2_DEV_TEMP_STATUS_TYPE                                   UInt32
#define CSR_LPDDR2_DEV_TEMP_STATUS_RESERVED_MASK                          0xFFFFFE00
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_SHIFT               8
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEVICE_SELECTED_MASK                0x00000100
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_SHIFT                 7
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_INTR_MASK                  0x00000080
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_SHIFT               0
#define    CSR_LPDDR2_DEV_TEMP_STATUS_DEV_TEMP_STATUS_MASK                0x0000007F

#define CSR_ARBITRATION_CONTROL_OFFSET                                    0x000000CC
#define CSR_ARBITRATION_CONTROL_TYPE                                      UInt32
#define CSR_ARBITRATION_CONTROL_RESERVED_MASK                             0xFFFFFFFE
#define    CSR_ARBITRATION_CONTROL_URG_CLIENT_LOW_LAT_MODE_SHIFT          0
#define    CSR_ARBITRATION_CONTROL_URG_CLIENT_LOW_LAT_MODE_MASK           0x00000001

#define CSR_PAD_REB_CLOSE_TIMER_OFFSET                                    0x000000D0
#define CSR_PAD_REB_CLOSE_TIMER_TYPE                                      UInt32
#define CSR_PAD_REB_CLOSE_TIMER_RESERVED_MASK                             0xFFFFFFF8
#define    CSR_PAD_REB_CLOSE_TIMER_REB_CLOSE_TIME_SHIFT                   0
#define    CSR_PAD_REB_CLOSE_TIMER_REB_CLOSE_TIME_MASK                    0x00000007

#define CSR_CS_SWITCH_NOP_OFFSET                                          0x000000D4
#define CSR_CS_SWITCH_NOP_TYPE                                            UInt32
#define CSR_CS_SWITCH_NOP_RESERVED_MASK                                   0xFFFFFFF8
#define    CSR_CS_SWITCH_NOP_DRAM_DRAM_SWITCH_SHIFT                       0
#define    CSR_CS_SWITCH_NOP_DRAM_DRAM_SWITCH_MASK                        0x00000007

#define CSR_LPDDR2_NVM_STATUS_PERIOD_OFFSET                               0x000000D8
#define CSR_LPDDR2_NVM_STATUS_PERIOD_TYPE                                 UInt32
#define CSR_LPDDR2_NVM_STATUS_PERIOD_RESERVED_MASK                        0xFE000000
#define    CSR_LPDDR2_NVM_STATUS_PERIOD_ENABLE_NVM_STATUS_SHIFT           24
#define    CSR_LPDDR2_NVM_STATUS_PERIOD_ENABLE_NVM_STATUS_MASK            0x01000000
#define    CSR_LPDDR2_NVM_STATUS_PERIOD_NVM_STATUS_PERIOD_SHIFT           0
#define    CSR_LPDDR2_NVM_STATUS_PERIOD_NVM_STATUS_PERIOD_MASK            0x00FFFFFF

#define CSR_LPDDR2_NVM_STATUS_OFFSET                                      0x000000DC
#define CSR_LPDDR2_NVM_STATUS_TYPE                                        UInt32
#define CSR_LPDDR2_NVM_STATUS_RESERVED_MASK                               0xFFFE0000
#define    CSR_LPDDR2_NVM_STATUS_NVM_STATUS_INTR_SHIFT                    16
#define    CSR_LPDDR2_NVM_STATUS_NVM_STATUS_INTR_MASK                     0x00010000
#define    CSR_LPDDR2_NVM_STATUS_NVM_STATUS_SHIFT                         0
#define    CSR_LPDDR2_NVM_STATUS_NVM_STATUS_MASK                          0x0000FFFF

#define CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET                              0x000000F0
#define CSR_DDR_SW_POWER_DOWN_CONTROL_TYPE                                UInt32
#define CSR_DDR_SW_POWER_DOWN_CONTROL_RESERVED_MASK                       0xF00000F8
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_SHIFT                  16
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_MASK                   0x0FFF0000
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_CLOCK_GATE_IDLE_TIME_SHIFT       8
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_CLOCK_GATE_IDLE_TIME_MASK        0x0000FF00
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_DISABLE_CLOCK_GATING_SHIFT       2
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_DISABLE_CLOCK_GATING_MASK        0x00000004
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_SHIFT            1
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_MASK             0x00000002
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_SHIFT          0
#define    CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK           0x00000001

#define CSR_DDR_POWER_DOWN_STATUS_OFFSET                                  0x000000F4
#define CSR_DDR_POWER_DOWN_STATUS_TYPE                                    UInt32
#define CSR_DDR_POWER_DOWN_STATUS_RESERVED_MASK                           0xFFFFFFFC
#define    CSR_DDR_POWER_DOWN_STATUS_POWER_DOWN_STATUS_SHIFT              0
#define    CSR_DDR_POWER_DOWN_STATUS_POWER_DOWN_STATUS_MASK               0x00000003

#define CSR_HW_FREQ_CHANGE_CNTRL_OFFSET                                   0x000000F8
#define CSR_HW_FREQ_CHANGE_CNTRL_TYPE                                     UInt32
#define CSR_HW_FREQ_CHANGE_CNTRL_RESERVED_MASK                            0xF0000000
#define    CSR_HW_FREQ_CHANGE_CNTRL_XTAL_LPWR_RX_CNTRL_SHIFT              27
#define    CSR_HW_FREQ_CHANGE_CNTRL_XTAL_LPWR_RX_CNTRL_MASK               0x08000000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_078MHZ_LPWR_RX_CNTRL_SHIFT    26
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_078MHZ_LPWR_RX_CNTRL_MASK     0x04000000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_104MHZ_LPWR_RX_CNTRL_SHIFT    25
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_104MHZ_LPWR_RX_CNTRL_MASK     0x02000000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_156MHZ_LPWR_RX_CNTRL_SHIFT    24
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_156MHZ_LPWR_RX_CNTRL_MASK     0x01000000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_100MHZ_LPWR_RX_CNTRL_SHIFT    23
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_100MHZ_LPWR_RX_CNTRL_MASK     0x00800000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_133MHZ_LPWR_RX_CNTRL_SHIFT    22
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_133MHZ_LPWR_RX_CNTRL_MASK     0x00400000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_200MHZ_LPWR_RX_CNTRL_SHIFT    21
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_200MHZ_LPWR_RX_CNTRL_MASK     0x00200000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_400MHZ_LPWR_RX_CNTRL_SHIFT    20
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_400MHZ_LPWR_RX_CNTRL_MASK     0x00100000
#define    CSR_HW_FREQ_CHANGE_CNTRL_XTAL_VREF_CNTRL_SHIFT                 19
#define    CSR_HW_FREQ_CHANGE_CNTRL_XTAL_VREF_CNTRL_MASK                  0x00080000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_078MHZ_VREF_CNTRL_SHIFT       18
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_078MHZ_VREF_CNTRL_MASK        0x00040000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_104MHZ_VREF_CNTRL_SHIFT       17
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_104MHZ_VREF_CNTRL_MASK        0x00020000
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_156MHZ_VREF_CNTRL_SHIFT       16
#define    CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_156MHZ_VREF_CNTRL_MASK        0x00010000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_100MHZ_VREF_CNTRL_SHIFT       15
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_100MHZ_VREF_CNTRL_MASK        0x00008000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_133MHZ_VREF_CNTRL_SHIFT       14
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_133MHZ_VREF_CNTRL_MASK        0x00004000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_200MHZ_VREF_CNTRL_SHIFT       13
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_200MHZ_VREF_CNTRL_MASK        0x00002000
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_400MHZ_VREF_CNTRL_SHIFT       12
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_400MHZ_VREF_CNTRL_MASK        0x00001000
#define    CSR_HW_FREQ_CHANGE_CNTRL_REF_COUNT_IN_CKE_PWRDN_SHIFT          8
#define    CSR_HW_FREQ_CHANGE_CNTRL_REF_COUNT_IN_CKE_PWRDN_MASK           0x00000F00
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_XTAL_CLK_REQ_OVERRIDE_SHIFT   7
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_XTAL_CLK_REQ_OVERRIDE_MASK    0x00000080
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_SYS_CLK_REQ_OVERRIDE_SHIFT    6
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_SYS_CLK_REQ_OVERRIDE_MASK     0x00000040
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_SHIFT            5
#define    CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK             0x00000020
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DLL_CALIB_ON_CLK_CHANGE_SHIFT 4
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DLL_CALIB_ON_CLK_CHANGE_MASK  0x00000010
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DESKEW_DLL_CALIB_ON_CLK_CHANGE_SHIFT 3
#define    CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DESKEW_DLL_CALIB_ON_CLK_CHANGE_MASK 0x00000008
#define    CSR_HW_FREQ_CHANGE_CNTRL_FORCE_DESKEW_DLL_CALIB_ON_CLK_CHANGE_SHIFT 2
#define    CSR_HW_FREQ_CHANGE_CNTRL_FORCE_DESKEW_DLL_CALIB_ON_CLK_CHANGE_MASK 0x00000004
#define    CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_SHIFT          1
#define    CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK           0x00000002
#define    CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_SHIFT               0
#define    CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK                0x00000001

#define CSR_CURRENT_FREQUENCY_STATE_OFFSET                                0x000000FC
#define CSR_CURRENT_FREQUENCY_STATE_TYPE                                  UInt32
#define CSR_CURRENT_FREQUENCY_STATE_RESERVED_MASK                         0xFFFFFF00
#define    CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_SHIFT                 4
#define    CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_MASK                  0x000000F0
#define    CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_SHIFT                0
#define    CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_MASK                 0x0000000F

#define CSR_SW_CLK_SWITCH_CNTRL_REG_OFFSET                                0x00000104
#define CSR_SW_CLK_SWITCH_CNTRL_REG_TYPE                                  UInt32
#define CSR_SW_CLK_SWITCH_CNTRL_REG_RESERVED_MASK                         0xFFFFFF08
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2PHY_DIV_VAL_SHIFT               4
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2PHY_DIV_VAL_MASK                0x000000F0
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2PHY_DIV_VAL_UPDATE_SHIFT        2
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2PHY_DIV_VAL_UPDATE_MASK         0x00000004
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2CCU_SYS_PLL_CLK_IS_IDLE_SHIFT   1
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2CCU_SYS_PLL_CLK_IS_IDLE_MASK    0x00000002
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2SEQ_PWRDWN_ENTER_SHIFT          0
#define    CSR_SW_CLK_SWITCH_CNTRL_REG_SW2SEQ_PWRDWN_ENTER_MASK           0x00000001

#define CSR_SW_CLK_SWITCH_STATUS_REG_OFFSET                               0x00000108
#define CSR_SW_CLK_SWITCH_STATUS_REG_TYPE                                 UInt32
#define CSR_SW_CLK_SWITCH_STATUS_REG_RESERVED_MASK                        0xFFFFFFF0
#define    CSR_SW_CLK_SWITCH_STATUS_REG_MDLL2SW_CLK_IS_IDLE_SHIFT         3
#define    CSR_SW_CLK_SWITCH_STATUS_REG_MDLL2SW_CLK_IS_IDLE_MASK          0x00000008
#define    CSR_SW_CLK_SWITCH_STATUS_REG_PHY2SW_SYS_PLL_CLK_IS_ON_SHIFT    2
#define    CSR_SW_CLK_SWITCH_STATUS_REG_PHY2SW_SYS_PLL_CLK_IS_ON_MASK     0x00000004
#define    CSR_SW_CLK_SWITCH_STATUS_REG_PHY2SW_DIV_VAL_ACCPT_SHIFT        1
#define    CSR_SW_CLK_SWITCH_STATUS_REG_PHY2SW_DIV_VAL_ACCPT_MASK         0x00000002
#define    CSR_SW_CLK_SWITCH_STATUS_REG_DEM2SW_CAM_IS_EMPTY_SHIFT         0
#define    CSR_SW_CLK_SWITCH_STATUS_REG_DEM2SW_CAM_IS_EMPTY_MASK          0x00000001

#define CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET                             0x0000010C
#define CSR_SEQ_RDWR_BANDWIDTH_CONTROL_TYPE                               UInt32
#define CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RESERVED_MASK                      0xFFFFFF00
#define    CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_SHIFT        4
#define    CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_MASK         0x000000F0
#define    CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_SHIFT        0
#define    CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_MASK         0x0000000F

#define CSR_CS0_MEMORY_TYPE_OFFSET                                        0x00000114
#define CSR_CS0_MEMORY_TYPE_TYPE                                          UInt32
#define CSR_CS0_MEMORY_TYPE_RESERVED_MASK                                 0xFFFFFFFE
#define    CSR_CS0_MEMORY_TYPE_CS0_MEM_TYPE_SHIFT                         0
#define    CSR_CS0_MEMORY_TYPE_CS0_MEM_TYPE_MASK                          0x00000001

#define CSR_REFRESH_CNTRL_2_OFFSET                                        0x00000118
#define CSR_REFRESH_CNTRL_2_TYPE                                          UInt32
#define CSR_REFRESH_CNTRL_2_RESERVED_MASK                                 0xFFFFEC00
#define    CSR_REFRESH_CNTRL_2_TREFBW_EN_SHIFT                            12
#define    CSR_REFRESH_CNTRL_2_TREFBW_EN_MASK                             0x00001000
#define    CSR_REFRESH_CNTRL_2_TREFBW_SHIFT                               0
#define    CSR_REFRESH_CNTRL_2_TREFBW_MASK                                0x000003FF

#define CSR_DEMESH_ARB_DISABLE_OFFSET                                     0x0000011C
#define CSR_DEMESH_ARB_DISABLE_TYPE                                       UInt32
#define CSR_DEMESH_ARB_DISABLE_RESERVED_MASK                              0xFFFFFFFE
#define    CSR_DEMESH_ARB_DISABLE_ARB_DISABLE_SHIFT                       0
#define    CSR_DEMESH_ARB_DISABLE_ARB_DISABLE_MASK                        0x00000001

#define CSR_PHY_BIST_CONTROL_OFFSET                                       0x00000120
#define CSR_PHY_BIST_CONTROL_TYPE                                         UInt32
#define CSR_PHY_BIST_CONTROL_RESERVED_MASK                                0xFFFFFC08
#define    CSR_PHY_BIST_CONTROL_BIST_XFER_LEN_SHIFT                       8
#define    CSR_PHY_BIST_CONTROL_BIST_XFER_LEN_MASK                        0x00000300
#define    CSR_PHY_BIST_CONTROL_BIST_VICTIM_COUNT_SHIFT                   4
#define    CSR_PHY_BIST_CONTROL_BIST_VICTIM_COUNT_MASK                    0x000000F0
#define    CSR_PHY_BIST_CONTROL_BIST_WR_SHIFT                             2
#define    CSR_PHY_BIST_CONTROL_BIST_WR_MASK                              0x00000004
#define    CSR_PHY_BIST_CONTROL_BIST_ENABLE_SHIFT                         1
#define    CSR_PHY_BIST_CONTROL_BIST_ENABLE_MASK                          0x00000002
#define    CSR_PHY_BIST_CONTROL_BIST_MODE_SHIFT                           0
#define    CSR_PHY_BIST_CONTROL_BIST_MODE_MASK                            0x00000001

#define CSR_PHY_BIST_STATUS_OFFSET                                        0x00000124
#define CSR_PHY_BIST_STATUS_TYPE                                          UInt32
#define CSR_PHY_BIST_STATUS_RESERVED_MASK                                 0xFFFFFFFC
#define    CSR_PHY_BIST_STATUS_BIST_DONE_SHIFT                            1
#define    CSR_PHY_BIST_STATUS_BIST_DONE_MASK                             0x00000002
#define    CSR_PHY_BIST_STATUS_BIST_BUSY_SHIFT                            0
#define    CSR_PHY_BIST_STATUS_BIST_BUSY_MASK                             0x00000001

#define CSR_PHY_BIST_BYTE0_DQ_CRC_OFFSET                                  0x00000128
#define CSR_PHY_BIST_BYTE0_DQ_CRC_TYPE                                    UInt32
#define CSR_PHY_BIST_BYTE0_DQ_CRC_RESERVED_MASK                           0x00000000
#define    CSR_PHY_BIST_BYTE0_DQ_CRC_BYTE0_DQ_CRC_SHIFT                   0
#define    CSR_PHY_BIST_BYTE0_DQ_CRC_BYTE0_DQ_CRC_MASK                    0xFFFFFFFF

#define CSR_PHY_BIST_BYTE1_DQ_CRC_OFFSET                                  0x0000012C
#define CSR_PHY_BIST_BYTE1_DQ_CRC_TYPE                                    UInt32
#define CSR_PHY_BIST_BYTE1_DQ_CRC_RESERVED_MASK                           0x00000000
#define    CSR_PHY_BIST_BYTE1_DQ_CRC_BYTE1_DQ_CRC_SHIFT                   0
#define    CSR_PHY_BIST_BYTE1_DQ_CRC_BYTE1_DQ_CRC_MASK                    0xFFFFFFFF

#define CSR_PHY_BIST_BYTE2_DQ_CRC_OFFSET                                  0x00000130
#define CSR_PHY_BIST_BYTE2_DQ_CRC_TYPE                                    UInt32
#define CSR_PHY_BIST_BYTE2_DQ_CRC_RESERVED_MASK                           0x00000000
#define    CSR_PHY_BIST_BYTE2_DQ_CRC_BYTE2_DQ_CRC_SHIFT                   0
#define    CSR_PHY_BIST_BYTE2_DQ_CRC_BYTE2_DQ_CRC_MASK                    0xFFFFFFFF

#define CSR_PHY_BIST_BYTE3_DQ_CRC_OFFSET                                  0x00000134
#define CSR_PHY_BIST_BYTE3_DQ_CRC_TYPE                                    UInt32
#define CSR_PHY_BIST_BYTE3_DQ_CRC_RESERVED_MASK                           0x00000000
#define    CSR_PHY_BIST_BYTE3_DQ_CRC_BYTE3_DQ_CRC_SHIFT                   0
#define    CSR_PHY_BIST_BYTE3_DQ_CRC_BYTE3_DQ_CRC_MASK                    0xFFFFFFFF

#define CSR_PHY_BIST_BYTE0123_DQM_CRC_OFFSET                              0x0000013C
#define CSR_PHY_BIST_BYTE0123_DQM_CRC_TYPE                                UInt32
#define CSR_PHY_BIST_BYTE0123_DQM_CRC_RESERVED_MASK                       0xFFFF0000
#define    CSR_PHY_BIST_BYTE0123_DQM_CRC_BYTE0123_DQM_CRC_SHIFT           0
#define    CSR_PHY_BIST_BYTE0123_DQM_CRC_BYTE0123_DQM_CRC_MASK            0x0000FFFF

#define CSR_PHY_BIST_CA_CRC_OFFSET                                        0x00000140
#define CSR_PHY_BIST_CA_CRC_TYPE                                          UInt32
#define CSR_PHY_BIST_CA_CRC_RESERVED_MASK                                 0x00000000
#define    CSR_PHY_BIST_CA_CRC_CA_RISE_CRC_SHIFT                          16
#define    CSR_PHY_BIST_CA_CRC_CA_RISE_CRC_MASK                           0xFFFF0000
#define    CSR_PHY_BIST_CA_CRC_CA_FALL_CRC_SHIFT                          0
#define    CSR_PHY_BIST_CA_CRC_CA_FALL_CRC_MASK                           0x0000FFFF

#define CSR_TEST_MODE_DQ_CRC_CONTROL_OFFSET                               0x00000144
#define CSR_TEST_MODE_DQ_CRC_CONTROL_TYPE                                 UInt32
#define CSR_TEST_MODE_DQ_CRC_CONTROL_RESERVED_MASK                        0xFFFEE000
#define    CSR_TEST_MODE_DQ_CRC_CONTROL_CRC_ENABLE_SHIFT                  16
#define    CSR_TEST_MODE_DQ_CRC_CONTROL_CRC_ENABLE_MASK                   0x00010000
#define    CSR_TEST_MODE_DQ_CRC_CONTROL_AXI_ID_SHIFT                      0
#define    CSR_TEST_MODE_DQ_CRC_CONTROL_AXI_ID_MASK                       0x00001FFF

#define CSR_ATE_MODE_CONTROL_OFFSET                                       0x00000148
#define CSR_ATE_MODE_CONTROL_TYPE                                         UInt32
#define CSR_ATE_MODE_CONTROL_RESERVED_MASK                                0xFFFFEEC0
#define    CSR_ATE_MODE_CONTROL_SLAVE_DLL_LOCK_ENABLE_SHIFT               12
#define    CSR_ATE_MODE_CONTROL_SLAVE_DLL_LOCK_ENABLE_MASK                0x00001000
#define    CSR_ATE_MODE_CONTROL_ATE_RL_EN_SHIFT                           8
#define    CSR_ATE_MODE_CONTROL_ATE_RL_EN_MASK                            0x00000100
#define    CSR_ATE_MODE_CONTROL_RL_OFFSET_SHIFT                           0
#define    CSR_ATE_MODE_CONTROL_RL_OFFSET_MASK                            0x0000003F

#define CSR_MEMC_MAX_PWR_STATE_OFFSET                                     0x0000014C
#define CSR_MEMC_MAX_PWR_STATE_TYPE                                       UInt32
#define CSR_MEMC_MAX_PWR_STATE_RESERVED_MASK                              0xFFFFFFEC
#define    CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_SHIFT             4
#define    CSR_MEMC_MAX_PWR_STATE_FORCE_MAX_POWER_STATE_MASK              0x00000010
#define    CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT                0
#define    CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK                 0x00000003

#define CSR_APPS_MIN_PWR_STATE_OFFSET                                     0x00000150
#define CSR_APPS_MIN_PWR_STATE_TYPE                                       UInt32
#define CSR_APPS_MIN_PWR_STATE_RESERVED_MASK                              0xFFFFFFFC
#define    CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_SHIFT                0
#define    CSR_APPS_MIN_PWR_STATE_APPS_MIN_PWR_STATE_MASK                 0x00000003

#define CSR_MODEM_MIN_PWR_STATE_OFFSET                                    0x00000154
#define CSR_MODEM_MIN_PWR_STATE_TYPE                                      UInt32
#define CSR_MODEM_MIN_PWR_STATE_RESERVED_MASK                             0xFFFFFFFC
#define    CSR_MODEM_MIN_PWR_STATE_MODEM_MIN_PWR_STATE_SHIFT              0
#define    CSR_MODEM_MIN_PWR_STATE_MODEM_MIN_PWR_STATE_MASK               0x00000003

#define CSR_MEMC_PWR_STATE_PENDING_OFFSET                                 0x00000158
#define CSR_MEMC_PWR_STATE_PENDING_TYPE                                   UInt32
#define CSR_MEMC_PWR_STATE_PENDING_RESERVED_MASK                          0xFFFFFFF8
#define    CSR_MEMC_PWR_STATE_PENDING_MODEM_MIN_PWR_STATE_PENDING_SHIFT   2
#define    CSR_MEMC_PWR_STATE_PENDING_MODEM_MIN_PWR_STATE_PENDING_MASK    0x00000004
#define    CSR_MEMC_PWR_STATE_PENDING_APPS_MIN_PWR_STATE_PENDING_SHIFT    1
#define    CSR_MEMC_PWR_STATE_PENDING_APPS_MIN_PWR_STATE_PENDING_MASK     0x00000002
#define    CSR_MEMC_PWR_STATE_PENDING_MEMC_MAX_PWR_STATE_PENDING_SHIFT    0
#define    CSR_MEMC_PWR_STATE_PENDING_MEMC_MAX_PWR_STATE_PENDING_MASK     0x00000001

#define CSR_MEMC_FREQ_STATE_MAPPING_OFFSET                                0x0000015C
#define CSR_MEMC_FREQ_STATE_MAPPING_TYPE                                  UInt32
#define CSR_MEMC_FREQ_STATE_MAPPING_RESERVED_MASK                         0xFFFFFFCC
#define    CSR_MEMC_FREQ_STATE_MAPPING_SYS_FREQ_DIVIDE_VAL_SHIFT          4
#define    CSR_MEMC_FREQ_STATE_MAPPING_SYS_FREQ_DIVIDE_VAL_MASK           0x00000030
#define    CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_SHIFT          0
#define    CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_MASK           0x00000003

#define CSR_DRAM_TIMING0_XTAL_OFFSET                                      0x00000160
#define CSR_DRAM_TIMING0_XTAL_TYPE                                        UInt32
#define CSR_DRAM_TIMING0_XTAL_RESERVED_MASK                               0xE0080080
#define    CSR_DRAM_TIMING0_XTAL_TRAS_XTAL_SHIFT                          24
#define    CSR_DRAM_TIMING0_XTAL_TRAS_XTAL_MASK                           0x1F000000
#define    CSR_DRAM_TIMING0_XTAL_TWR_XTAL_SHIFT                           20
#define    CSR_DRAM_TIMING0_XTAL_TWR_XTAL_MASK                            0x00F00000
#define    CSR_DRAM_TIMING0_XTAL_TRTP_XTAL_SHIFT                          16
#define    CSR_DRAM_TIMING0_XTAL_TRTP_XTAL_MASK                           0x00070000
#define    CSR_DRAM_TIMING0_XTAL_TRP_PB_XTAL_SHIFT                        12
#define    CSR_DRAM_TIMING0_XTAL_TRP_PB_XTAL_MASK                         0x0000F000
#define    CSR_DRAM_TIMING0_XTAL_TRP_AB_XTAL_SHIFT                        8
#define    CSR_DRAM_TIMING0_XTAL_TRP_AB_XTAL_MASK                         0x00000F00
#define    CSR_DRAM_TIMING0_XTAL_TRRD_XTAL_SHIFT                          4
#define    CSR_DRAM_TIMING0_XTAL_TRRD_XTAL_MASK                           0x00000070
#define    CSR_DRAM_TIMING0_XTAL_TRCD_XTAL_SHIFT                          0
#define    CSR_DRAM_TIMING0_XTAL_TRCD_XTAL_MASK                           0x0000000F

#define CSR_DRAM_TIMING1_XTAL_OFFSET                                      0x00000164
#define CSR_DRAM_TIMING1_XTAL_TYPE                                        UInt32
#define CSR_DRAM_TIMING1_XTAL_RESERVED_MASK                               0xC0008E08
#define    CSR_DRAM_TIMING1_XTAL_TDQSCK_XTAL_SHIFT                        28
#define    CSR_DRAM_TIMING1_XTAL_TDQSCK_XTAL_MASK                         0x30000000
#define    CSR_DRAM_TIMING1_XTAL_TXSR_XTAL_SHIFT                          20
#define    CSR_DRAM_TIMING1_XTAL_TXSR_XTAL_MASK                           0x0FF00000
#define    CSR_DRAM_TIMING1_XTAL_TCKESR_XTAL_SHIFT                        16
#define    CSR_DRAM_TIMING1_XTAL_TCKESR_XTAL_MASK                         0x000F0000
#define    CSR_DRAM_TIMING1_XTAL_TXP_XTAL_SHIFT                           12
#define    CSR_DRAM_TIMING1_XTAL_TXP_XTAL_MASK                            0x00007000
#define    CSR_DRAM_TIMING1_XTAL_TFAW_XTAL_SHIFT                          4
#define    CSR_DRAM_TIMING1_XTAL_TFAW_XTAL_MASK                           0x000001F0
#define    CSR_DRAM_TIMING1_XTAL_TWTR_XTAL_SHIFT                          0
#define    CSR_DRAM_TIMING1_XTAL_TWTR_XTAL_MASK                           0x00000007

#define CSR_NVM_TIMING0_XTAL_OFFSET                                       0x00000168
#define CSR_NVM_TIMING0_XTAL_TYPE                                         UInt32
#define CSR_NVM_TIMING0_XTAL_RESERVED_MASK                                0xFC000000
#define    CSR_NVM_TIMING0_XTAL_TRP_NVM_XTAL_SHIFT                        24
#define    CSR_NVM_TIMING0_XTAL_TRP_NVM_XTAL_MASK                         0x03000000
#define    CSR_NVM_TIMING0_XTAL_TRAS_NVM_XTAL_SHIFT                       16
#define    CSR_NVM_TIMING0_XTAL_TRAS_NVM_XTAL_MASK                        0x00FF0000
#define    CSR_NVM_TIMING0_XTAL_TRRD_NVM_XTAL_SHIFT                       8
#define    CSR_NVM_TIMING0_XTAL_TRRD_NVM_XTAL_MASK                        0x0000FF00
#define    CSR_NVM_TIMING0_XTAL_TRCD_NVM_XTAL_SHIFT                       0
#define    CSR_NVM_TIMING0_XTAL_TRCD_NVM_XTAL_MASK                        0x000000FF

#define CSR_DRAM_TIMING0_SYSPLL_OFFSET                                    0x0000016C
#define CSR_DRAM_TIMING0_SYSPLL_TYPE                                      UInt32
#define CSR_DRAM_TIMING0_SYSPLL_RESERVED_MASK                             0xE0080080
#define    CSR_DRAM_TIMING0_SYSPLL_TRAS_SYSPLL_SHIFT                      24
#define    CSR_DRAM_TIMING0_SYSPLL_TRAS_SYSPLL_MASK                       0x1F000000
#define    CSR_DRAM_TIMING0_SYSPLL_TWR_SYSPLL_SHIFT                       20
#define    CSR_DRAM_TIMING0_SYSPLL_TWR_SYSPLL_MASK                        0x00F00000
#define    CSR_DRAM_TIMING0_SYSPLL_TRTP_SYSPLL_SHIFT                      16
#define    CSR_DRAM_TIMING0_SYSPLL_TRTP_SYSPLL_MASK                       0x00070000
#define    CSR_DRAM_TIMING0_SYSPLL_TRP_PB_SYSPLL_SHIFT                    12
#define    CSR_DRAM_TIMING0_SYSPLL_TRP_PB_SYSPLL_MASK                     0x0000F000
#define    CSR_DRAM_TIMING0_SYSPLL_TRP_AB_SYSPLL_SHIFT                    8
#define    CSR_DRAM_TIMING0_SYSPLL_TRP_AB_SYSPLL_MASK                     0x00000F00
#define    CSR_DRAM_TIMING0_SYSPLL_TRRD_SYSPLL_SHIFT                      4
#define    CSR_DRAM_TIMING0_SYSPLL_TRRD_SYSPLL_MASK                       0x00000070
#define    CSR_DRAM_TIMING0_SYSPLL_TRCD_SYSPLL_SHIFT                      0
#define    CSR_DRAM_TIMING0_SYSPLL_TRCD_SYSPLL_MASK                       0x0000000F

#define CSR_DRAM_TIMING1_SYSPLL_OFFSET                                    0x00000170
#define CSR_DRAM_TIMING1_SYSPLL_TYPE                                      UInt32
#define CSR_DRAM_TIMING1_SYSPLL_RESERVED_MASK                             0xC0008E08
#define    CSR_DRAM_TIMING1_SYSPLL_TDQSCK_SYSPLL_SHIFT                    28
#define    CSR_DRAM_TIMING1_SYSPLL_TDQSCK_SYSPLL_MASK                     0x30000000
#define    CSR_DRAM_TIMING1_SYSPLL_TXSR_SYSPLL_SHIFT                      20
#define    CSR_DRAM_TIMING1_SYSPLL_TXSR_SYSPLL_MASK                       0x0FF00000
#define    CSR_DRAM_TIMING1_SYSPLL_TCKESR_SYSPLL_SHIFT                    16
#define    CSR_DRAM_TIMING1_SYSPLL_TCKESR_SYSPLL_MASK                     0x000F0000
#define    CSR_DRAM_TIMING1_SYSPLL_TXP_SYSPLL_SHIFT                       12
#define    CSR_DRAM_TIMING1_SYSPLL_TXP_SYSPLL_MASK                        0x00007000
#define    CSR_DRAM_TIMING1_SYSPLL_TFAW_SYSPLL_SHIFT                      4
#define    CSR_DRAM_TIMING1_SYSPLL_TFAW_SYSPLL_MASK                       0x000001F0
#define    CSR_DRAM_TIMING1_SYSPLL_TWTR_SYSPLL_SHIFT                      0
#define    CSR_DRAM_TIMING1_SYSPLL_TWTR_SYSPLL_MASK                       0x00000007

#define CSR_NVM_TIMING0_SYSPLL_OFFSET                                     0x00000174
#define CSR_NVM_TIMING0_SYSPLL_TYPE                                       UInt32
#define CSR_NVM_TIMING0_SYSPLL_RESERVED_MASK                              0xFC000000
#define    CSR_NVM_TIMING0_SYSPLL_TRP_NVM_SYSPLL_SHIFT                    24
#define    CSR_NVM_TIMING0_SYSPLL_TRP_NVM_SYSPLL_MASK                     0x03000000
#define    CSR_NVM_TIMING0_SYSPLL_TRAS_NVM_SYSPLL_SHIFT                   16
#define    CSR_NVM_TIMING0_SYSPLL_TRAS_NVM_SYSPLL_MASK                    0x00FF0000
#define    CSR_NVM_TIMING0_SYSPLL_TRRD_NVM_SYSPLL_SHIFT                   8
#define    CSR_NVM_TIMING0_SYSPLL_TRRD_NVM_SYSPLL_MASK                    0x0000FF00
#define    CSR_NVM_TIMING0_SYSPLL_TRCD_NVM_SYSPLL_SHIFT                   0
#define    CSR_NVM_TIMING0_SYSPLL_TRCD_NVM_SYSPLL_MASK                    0x000000FF

#define CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_OFFSET                         0x00000178
#define CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_TYPE                           UInt32
#define CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_RESERVED_MASK                  0xFFFF0000
#define    CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_TRFC_SYSPLL_SHIFT           8
#define    CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_TRFC_SYSPLL_MASK            0x0000FF00
#define    CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_TRFC_XTAL_SHIFT             0
#define    CSR_REFRESH_TRFC_CNTRL_XTAL_SYSPLL_TRFC_XTAL_MASK              0x000000FF

#define CSR_STATISTICS_CONTROL_OFFSET                                     0x00000180
#define CSR_STATISTICS_CONTROL_TYPE                                       UInt32
#define CSR_STATISTICS_CONTROL_RESERVED_MASK                              0xFF8000F8
#define    CSR_STATISTICS_CONTROL_LAT_URG_STATISTICS_AXI_ID_SHIFT         8
#define    CSR_STATISTICS_CONTROL_LAT_URG_STATISTICS_AXI_ID_MASK          0x007FFF00
#define    CSR_STATISTICS_CONTROL_LAT_URG_SEL_SHIFT                       2
#define    CSR_STATISTICS_CONTROL_LAT_URG_SEL_MASK                        0x00000004
#define    CSR_STATISTICS_CONTROL_STAT_ON_DEM_BUSY_SHIFT                  1
#define    CSR_STATISTICS_CONTROL_STAT_ON_DEM_BUSY_MASK                   0x00000002
#define    CSR_STATISTICS_CONTROL_STATISTICS_EN_SHIFT                     0
#define    CSR_STATISTICS_CONTROL_STATISTICS_EN_MASK                      0x00000001

#define CSR_STATISTICS_PERIOD_OFFSET                                      0x00000184
#define CSR_STATISTICS_PERIOD_TYPE                                        UInt32
#define CSR_STATISTICS_PERIOD_RESERVED_MASK                               0x00000000
#define    CSR_STATISTICS_PERIOD_STATISTICS_PERIOD_SHIFT                  0
#define    CSR_STATISTICS_PERIOD_STATISTICS_PERIOD_MASK                   0xFFFFFFFF

#define CSR_STATISTICS_DONE_OFFSET                                        0x00000188
#define CSR_STATISTICS_DONE_TYPE                                          UInt32
#define CSR_STATISTICS_DONE_RESERVED_MASK                                 0xFFFFFFFE
#define    CSR_STATISTICS_DONE_STATISTICS_DONE_SHIFT                      0
#define    CSR_STATISTICS_DONE_STATISTICS_DONE_MASK                       0x00000001

#define CSR_STATISTICS_READ_DATA_CYCLES_OFFSET                            0x0000018C
#define CSR_STATISTICS_READ_DATA_CYCLES_TYPE                              UInt32
#define CSR_STATISTICS_READ_DATA_CYCLES_RESERVED_MASK                     0x80000000
#define    CSR_STATISTICS_READ_DATA_CYCLES_READ_DATA_COUNT_SHIFT          0
#define    CSR_STATISTICS_READ_DATA_CYCLES_READ_DATA_COUNT_MASK           0x7FFFFFFF

#define CSR_STATISTICS_WRITE_DATA_CYCLES_OFFSET                           0x00000190
#define CSR_STATISTICS_WRITE_DATA_CYCLES_TYPE                             UInt32
#define CSR_STATISTICS_WRITE_DATA_CYCLES_RESERVED_MASK                    0x80000000
#define    CSR_STATISTICS_WRITE_DATA_CYCLES_WRITE_DATA_COUNT_SHIFT        0
#define    CSR_STATISTICS_WRITE_DATA_CYCLES_WRITE_DATA_COUNT_MASK         0x7FFFFFFF

#define CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_OFFSET                        0x00000194
#define CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_TYPE                          UInt32
#define CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_RESERVED_MASK                 0x00000000
#define    CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_HIT_DATA_COUNT_SHIFT       0
#define    CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_HIT_DATA_COUNT_MASK        0xFFFFFFFF

#define CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_OFFSET                       0x00000198
#define CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_TYPE                         UInt32
#define CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_RESERVED_MASK                0x00000000
#define    CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_MISS_DATA_COUNT_SHIFT     0
#define    CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_MISS_DATA_COUNT_MASK      0xFFFFFFFF

#define CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_OFFSET                       0x0000019C
#define CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_TYPE                         UInt32
#define CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_RESERVED_MASK                0x00000000
#define    CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_HIT_TRANSACTION_COUNT_SHIFT 0
#define    CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_HIT_TRANSACTION_COUNT_MASK 0xFFFFFFFF

#define CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_OFFSET                      0x000001A0
#define CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_TYPE                        UInt32
#define CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_RESERVED_MASK               0x00000000
#define    CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_MISS_TRANSACTION_COUNT_SHIFT 0
#define    CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_MISS_TRANSACTION_COUNT_MASK 0xFFFFFFFF

#define CSR_STATISTICS_AUTO_REF_CYCLES_OFFSET                             0x000001A4
#define CSR_STATISTICS_AUTO_REF_CYCLES_TYPE                               UInt32
#define CSR_STATISTICS_AUTO_REF_CYCLES_RESERVED_MASK                      0xF8000000
#define    CSR_STATISTICS_AUTO_REF_CYCLES_AUTO_REF_COUNT_SHIFT            0
#define    CSR_STATISTICS_AUTO_REF_CYCLES_AUTO_REF_COUNT_MASK             0x07FFFFFF

#define CSR_STATISTICS_BANK_ACTIVE_CYCLES_OFFSET                          0x000001A8
#define CSR_STATISTICS_BANK_ACTIVE_CYCLES_TYPE                            UInt32
#define CSR_STATISTICS_BANK_ACTIVE_CYCLES_RESERVED_MASK                   0xF0000000
#define    CSR_STATISTICS_BANK_ACTIVE_CYCLES_BANK_ACTIVE_COUNT_SHIFT      0
#define    CSR_STATISTICS_BANK_ACTIVE_CYCLES_BANK_ACTIVE_COUNT_MASK       0x0FFFFFFF

#define CSR_STATISTICS_ENTRIES_MIN_MAX_OFFSET                             0x000001AC
#define CSR_STATISTICS_ENTRIES_MIN_MAX_TYPE                               UInt32
#define CSR_STATISTICS_ENTRIES_MIN_MAX_RESERVED_MASK                      0xFF000000
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MAX_ENTRIES_URGENT_SHIFT        16
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MAX_ENTRIES_URGENT_MASK         0x00FF0000
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MIN_ENTRIES_VALID_SHIFT         8
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MIN_ENTRIES_VALID_MASK          0x0000FF00
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MAX_ENTRIES_VALID_SHIFT         0
#define    CSR_STATISTICS_ENTRIES_MIN_MAX_MAX_ENTRIES_VALID_MASK          0x000000FF

#define CSR_STATISTICS_ENTRIES_URGENT_OFFSET                              0x000001B0
#define CSR_STATISTICS_ENTRIES_URGENT_TYPE                                UInt32
#define CSR_STATISTICS_ENTRIES_URGENT_RESERVED_MASK                       0x00000000
#define    CSR_STATISTICS_ENTRIES_URGENT_ENTRIES_URGENT_COUNT_SHIFT       0
#define    CSR_STATISTICS_ENTRIES_URGENT_ENTRIES_URGENT_COUNT_MASK        0xFFFFFFFF

#define CSR_STATISTICS_ENTRIES_WAW_OFFSET                                 0x000001B8
#define CSR_STATISTICS_ENTRIES_WAW_TYPE                                   UInt32
#define CSR_STATISTICS_ENTRIES_WAW_RESERVED_MASK                          0x00000000
#define    CSR_STATISTICS_ENTRIES_WAW_ENTRIES_WAW_COUNT_SHIFT             0
#define    CSR_STATISTICS_ENTRIES_WAW_ENTRIES_WAW_COUNT_MASK              0xFFFFFFFF

#define CSR_STATISTICS_ENTRIES_RAR_OFFSET                                 0x000001BC
#define CSR_STATISTICS_ENTRIES_RAR_TYPE                                   UInt32
#define CSR_STATISTICS_ENTRIES_RAR_RESERVED_MASK                          0x00000000
#define    CSR_STATISTICS_ENTRIES_RAR_ENTRIES_RAR_COUNT_SHIFT             0
#define    CSR_STATISTICS_ENTRIES_RAR_ENTRIES_RAR_COUNT_MASK              0xFFFFFFFF

#define CSR_STATISTICS_ENTRIES_ADDR_COH_OFFSET                            0x000001C0
#define CSR_STATISTICS_ENTRIES_ADDR_COH_TYPE                              UInt32
#define CSR_STATISTICS_ENTRIES_ADDR_COH_RESERVED_MASK                     0x00000000
#define    CSR_STATISTICS_ENTRIES_ADDR_COH_ENTRIES_ADDR_COH_COUNT_SHIFT   0
#define    CSR_STATISTICS_ENTRIES_ADDR_COH_ENTRIES_ADDR_COH_COUNT_MASK    0xFFFFFFFF

#define CSR_STATISTICS_LATENCY_URGENT_TIMES_OFFSET                        0x000001C4
#define CSR_STATISTICS_LATENCY_URGENT_TIMES_TYPE                          UInt32
#define CSR_STATISTICS_LATENCY_URGENT_TIMES_RESERVED_MASK                 0x00000000
#define    CSR_STATISTICS_LATENCY_URGENT_TIMES_MAX_TIME_SHIFT             16
#define    CSR_STATISTICS_LATENCY_URGENT_TIMES_MAX_TIME_MASK              0xFFFF0000
#define    CSR_STATISTICS_LATENCY_URGENT_TIMES_MIN_TIME_SHIFT             0
#define    CSR_STATISTICS_LATENCY_URGENT_TIMES_MIN_TIME_MASK              0x0000FFFF

#define CSR_CALIBRATION_INTR_OFFSET                                       0x00000200
#define CSR_CALIBRATION_INTR_TYPE                                         UInt32
#define CSR_CALIBRATION_INTR_RESERVED_MASK                                0xFFFFFFFE
#define    CSR_CALIBRATION_INTR_CAL_INTR_SHIFT                            0
#define    CSR_CALIBRATION_INTR_CAL_INTR_MASK                             0x00000001

#define CSR_CAM_ENABLE_0_OFFSET                                           0x00000204
#define CSR_CAM_ENABLE_0_TYPE                                             UInt32
#define CSR_CAM_ENABLE_0_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_SHIFT                        16
#define    CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_0_CAM_TAG_0_SHIFT                               0
#define    CSR_CAM_ENABLE_0_CAM_TAG_0_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_1_OFFSET                                           0x00000208
#define CSR_CAM_ENABLE_1_TYPE                                             UInt32
#define CSR_CAM_ENABLE_1_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_1_CAM_TAG_ENABLE_1_SHIFT                        16
#define    CSR_CAM_ENABLE_1_CAM_TAG_ENABLE_1_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_1_CAM_TAG_1_SHIFT                               0
#define    CSR_CAM_ENABLE_1_CAM_TAG_1_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_2_OFFSET                                           0x0000020C
#define CSR_CAM_ENABLE_2_TYPE                                             UInt32
#define CSR_CAM_ENABLE_2_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_2_CAM_TAG_ENABLE_2_SHIFT                        16
#define    CSR_CAM_ENABLE_2_CAM_TAG_ENABLE_2_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_2_CAM_TAG_2_SHIFT                               0
#define    CSR_CAM_ENABLE_2_CAM_TAG_2_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_3_OFFSET                                           0x00000210
#define CSR_CAM_ENABLE_3_TYPE                                             UInt32
#define CSR_CAM_ENABLE_3_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_3_CAM_TAG_ENABLE_3_SHIFT                        16
#define    CSR_CAM_ENABLE_3_CAM_TAG_ENABLE_3_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_3_CAM_TAG_3_SHIFT                               0
#define    CSR_CAM_ENABLE_3_CAM_TAG_3_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_4_OFFSET                                           0x00000214
#define CSR_CAM_ENABLE_4_TYPE                                             UInt32
#define CSR_CAM_ENABLE_4_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_4_CAM_TAG_ENABLE_4_SHIFT                        16
#define    CSR_CAM_ENABLE_4_CAM_TAG_ENABLE_4_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_4_CAM_TAG_4_SHIFT                               0
#define    CSR_CAM_ENABLE_4_CAM_TAG_4_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_5_OFFSET                                           0x00000218
#define CSR_CAM_ENABLE_5_TYPE                                             UInt32
#define CSR_CAM_ENABLE_5_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_5_CAM_TAG_ENABLE_5_SHIFT                        16
#define    CSR_CAM_ENABLE_5_CAM_TAG_ENABLE_5_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_5_CAM_TAG_5_SHIFT                               0
#define    CSR_CAM_ENABLE_5_CAM_TAG_5_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_6_OFFSET                                           0x0000021C
#define CSR_CAM_ENABLE_6_TYPE                                             UInt32
#define CSR_CAM_ENABLE_6_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_6_CAM_TAG_ENABLE_6_SHIFT                        16
#define    CSR_CAM_ENABLE_6_CAM_TAG_ENABLE_6_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_6_CAM_TAG_6_SHIFT                               0
#define    CSR_CAM_ENABLE_6_CAM_TAG_6_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_7_OFFSET                                           0x00000220
#define CSR_CAM_ENABLE_7_TYPE                                             UInt32
#define CSR_CAM_ENABLE_7_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_7_CAM_TAG_ENABLE_7_SHIFT                        16
#define    CSR_CAM_ENABLE_7_CAM_TAG_ENABLE_7_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_7_CAM_TAG_7_SHIFT                               0
#define    CSR_CAM_ENABLE_7_CAM_TAG_7_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_8_OFFSET                                           0x00000224
#define CSR_CAM_ENABLE_8_TYPE                                             UInt32
#define CSR_CAM_ENABLE_8_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_8_CAM_TAG_ENABLE_8_SHIFT                        16
#define    CSR_CAM_ENABLE_8_CAM_TAG_ENABLE_8_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_8_CAM_TAG_8_SHIFT                               0
#define    CSR_CAM_ENABLE_8_CAM_TAG_8_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_9_OFFSET                                           0x00000228
#define CSR_CAM_ENABLE_9_TYPE                                             UInt32
#define CSR_CAM_ENABLE_9_RESERVED_MASK                                    0x80008000
#define    CSR_CAM_ENABLE_9_CAM_TAG_ENABLE_9_SHIFT                        16
#define    CSR_CAM_ENABLE_9_CAM_TAG_ENABLE_9_MASK                         0x7FFF0000
#define    CSR_CAM_ENABLE_9_CAM_TAG_9_SHIFT                               0
#define    CSR_CAM_ENABLE_9_CAM_TAG_9_MASK                                0x00007FFF

#define CSR_CAM_ENABLE_10_OFFSET                                          0x0000022C
#define CSR_CAM_ENABLE_10_TYPE                                            UInt32
#define CSR_CAM_ENABLE_10_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_10_CAM_TAG_ENABLE_10_SHIFT                      16
#define    CSR_CAM_ENABLE_10_CAM_TAG_ENABLE_10_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_10_CAM_TAG_10_SHIFT                             0
#define    CSR_CAM_ENABLE_10_CAM_TAG_10_MASK                              0x00007FFF

#define CSR_CAM_ENABLE_11_OFFSET                                          0x00000230
#define CSR_CAM_ENABLE_11_TYPE                                            UInt32
#define CSR_CAM_ENABLE_11_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_11_CAM_TAG_ENABLE_11_SHIFT                      16
#define    CSR_CAM_ENABLE_11_CAM_TAG_ENABLE_11_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_11_CAM_TAG_11_SHIFT                             0
#define    CSR_CAM_ENABLE_11_CAM_TAG_11_MASK                              0x00007FFF

#define CSR_CAM_ENABLE_12_OFFSET                                          0x00000234
#define CSR_CAM_ENABLE_12_TYPE                                            UInt32
#define CSR_CAM_ENABLE_12_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_12_CAM_TAG_ENABLE_12_SHIFT                      16
#define    CSR_CAM_ENABLE_12_CAM_TAG_ENABLE_12_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_12_CAM_TAG_12_SHIFT                             0
#define    CSR_CAM_ENABLE_12_CAM_TAG_12_MASK                              0x00007FFF

#define CSR_CAM_ENABLE_13_OFFSET                                          0x00000238
#define CSR_CAM_ENABLE_13_TYPE                                            UInt32
#define CSR_CAM_ENABLE_13_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_13_CAM_TAG_ENABLE_13_SHIFT                      16
#define    CSR_CAM_ENABLE_13_CAM_TAG_ENABLE_13_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_13_CAM_TAG_13_SHIFT                             0
#define    CSR_CAM_ENABLE_13_CAM_TAG_13_MASK                              0x00007FFF

#define CSR_CAM_ENABLE_14_OFFSET                                          0x0000023C
#define CSR_CAM_ENABLE_14_TYPE                                            UInt32
#define CSR_CAM_ENABLE_14_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_14_CAM_TAG_ENABLE_14_SHIFT                      16
#define    CSR_CAM_ENABLE_14_CAM_TAG_ENABLE_14_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_14_CAM_TAG_14_SHIFT                             0
#define    CSR_CAM_ENABLE_14_CAM_TAG_14_MASK                              0x00007FFF

#define CSR_CAM_ENABLE_15_OFFSET                                          0x00000240
#define CSR_CAM_ENABLE_15_TYPE                                            UInt32
#define CSR_CAM_ENABLE_15_RESERVED_MASK                                   0x80008000
#define    CSR_CAM_ENABLE_15_CAM_TAG_ENABLE_15_SHIFT                      16
#define    CSR_CAM_ENABLE_15_CAM_TAG_ENABLE_15_MASK                       0x7FFF0000
#define    CSR_CAM_ENABLE_15_CAM_TAG_15_SHIFT                             0
#define    CSR_CAM_ENABLE_15_CAM_TAG_15_MASK                              0x00007FFF

#define CSR_ACC_VIO_ADDR_OFFSET                                           0x00000244
#define CSR_ACC_VIO_ADDR_TYPE                                             UInt32
#define CSR_ACC_VIO_ADDR_RESERVED_MASK                                    0x00000000
#define    CSR_ACC_VIO_ADDR_ACC_VIO_ADDR_SHIFT                            0
#define    CSR_ACC_VIO_ADDR_ACC_VIO_ADDR_MASK                             0xFFFFFFFF

#define CSR_ACC_VIO_INFO_OFFSET                                           0x00000248
#define CSR_ACC_VIO_INFO_TYPE                                             UInt32
#define CSR_ACC_VIO_INFO_RESERVED_MASK                                    0x00000000
#define    CSR_ACC_VIO_INFO_ACC_VIO_INTR_OVERRUN_SHIFT                    31
#define    CSR_ACC_VIO_INFO_ACC_VIO_INTR_OVERRUN_MASK                     0x80000000
#define    CSR_ACC_VIO_INFO_ACC_VIO_INTR_SHIFT                            30
#define    CSR_ACC_VIO_INFO_ACC_VIO_INTR_MASK                             0x40000000
#define    CSR_ACC_VIO_INFO_ACC_VIO_REASON_SHIFT                          27
#define    CSR_ACC_VIO_INFO_ACC_VIO_REASON_MASK                           0x38000000
#define    CSR_ACC_VIO_INFO_ACC_VIO_PORT_ID_SHIFT                         25
#define    CSR_ACC_VIO_INFO_ACC_VIO_PORT_ID_MASK                          0x06000000
#define    CSR_ACC_VIO_INFO_ACC_VIO_REGION_SHIFT                          16
#define    CSR_ACC_VIO_INFO_ACC_VIO_REGION_MASK                           0x01FF0000
#define    CSR_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_SHIFT                       1
#define    CSR_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_MASK                        0x0000FFFE
#define    CSR_ACC_VIO_INFO_ACC_VIO_TYPE_SHIFT                            0
#define    CSR_ACC_VIO_INFO_ACC_VIO_TYPE_MASK                             0x00000001

#define CSR_TEST_MUX_SELECT_OFFSET                                        0x00000300
#define CSR_TEST_MUX_SELECT_TYPE                                          UInt32
#define CSR_TEST_MUX_SELECT_RESERVED_MASK                                 0xFFFFFE00
#define    CSR_TEST_MUX_SELECT_TEST_MUX_SEL_SHIFT                         0
#define    CSR_TEST_MUX_SELECT_TEST_MUX_SEL_MASK                          0x000001FF

#define CSR_TEST_MUX_OUTPUT_OFFSET                                        0x00000304
#define CSR_TEST_MUX_OUTPUT_TYPE                                          UInt32
#define CSR_TEST_MUX_OUTPUT_RESERVED_MASK                                 0x00000000
#define    CSR_TEST_MUX_OUTPUT_TEST_MUX_OUT_SHIFT                         0
#define    CSR_TEST_MUX_OUTPUT_TEST_MUX_OUT_MASK                          0xFFFFFFFF

#define CSR_DBG_CONTROL_OVERRIDES_OFFSET                                  0x00000308
#define CSR_DBG_CONTROL_OVERRIDES_TYPE                                    UInt32
#define CSR_DBG_CONTROL_OVERRIDES_RESERVED_MASK                           0xFFFFFFF8
#define    CSR_DBG_CONTROL_OVERRIDES_GEN_DECERR_FOR_EXLCUSIVE_AXI_ACCESSES_SHIFT 2
#define    CSR_DBG_CONTROL_OVERRIDES_GEN_DECERR_FOR_EXLCUSIVE_AXI_ACCESSES_MASK 0x00000004
#define    CSR_DBG_CONTROL_OVERRIDES_MSK_WR_DATA_INTERLEAVING_CHK_SHIFT   1
#define    CSR_DBG_CONTROL_OVERRIDES_MSK_WR_DATA_INTERLEAVING_CHK_MASK    0x00000002
#define    CSR_DBG_CONTROL_OVERRIDES_FORCE_HW_SELFREF_USE_SHIFT           0
#define    CSR_DBG_CONTROL_OVERRIDES_FORCE_HW_SELFREF_USE_MASK            0x00000001

#define CSR_DBG_INTR_REGISTER_OFFSET                                      0x0000030C
#define CSR_DBG_INTR_REGISTER_TYPE                                        UInt32
#define CSR_DBG_INTR_REGISTER_RESERVED_MASK                               0xFFF4E000
#define    CSR_DBG_INTR_REGISTER_WR_DATA_INTERLEAVING_INTR_SHIFT          19
#define    CSR_DBG_INTR_REGISTER_WR_DATA_INTERLEAVING_INTR_MASK           0x00080000
#define    CSR_DBG_INTR_REGISTER_ERR_PORT_SHIFT                           16
#define    CSR_DBG_INTR_REGISTER_ERR_PORT_MASK                            0x00030000
#define    CSR_DBG_INTR_REGISTER_ERR_WID_SHIFT                            0
#define    CSR_DBG_INTR_REGISTER_ERR_WID_MASK                             0x00001FFF

#define CSR_DBG_MEMC_CONFIG_OFFSET                                        0x00000310
#define CSR_DBG_MEMC_CONFIG_TYPE                                          UInt32
#define CSR_DBG_MEMC_CONFIG_RESERVED_MASK                                 0xFEE0CCCC
#define    CSR_DBG_MEMC_CONFIG_AXI2DEM_DW_SHIFT                           24
#define    CSR_DBG_MEMC_CONFIG_AXI2DEM_DW_MASK                            0x01000000
#define    CSR_DBG_MEMC_CONFIG_NUM_OF_DEMESH_ENTRIES_SHIFT                16
#define    CSR_DBG_MEMC_CONFIG_NUM_OF_DEMESH_ENTRIES_MASK                 0x001F0000
#define    CSR_DBG_MEMC_CONFIG_PORT3_AXI_DW_SHIFT                         12
#define    CSR_DBG_MEMC_CONFIG_PORT3_AXI_DW_MASK                          0x00003000
#define    CSR_DBG_MEMC_CONFIG_PORT2_AXI_DW_SHIFT                         8
#define    CSR_DBG_MEMC_CONFIG_PORT2_AXI_DW_MASK                          0x00000300
#define    CSR_DBG_MEMC_CONFIG_PORT1_AXI_DW_SHIFT                         4
#define    CSR_DBG_MEMC_CONFIG_PORT1_AXI_DW_MASK                          0x00000030
#define    CSR_DBG_MEMC_CONFIG_PORT0_AXI_DW_SHIFT                         0
#define    CSR_DBG_MEMC_CONFIG_PORT0_AXI_DW_MASK                          0x00000003

#define CSR_DBG_PORT0_AXI_ERR_COND_OFFSET                                 0x00000314
#define CSR_DBG_PORT0_AXI_ERR_COND_TYPE                                   UInt32
#define CSR_DBG_PORT0_AXI_ERR_COND_RESERVED_MASK                          0xFFFFF000
#define    CSR_DBG_PORT0_AXI_ERR_COND_PORT0_AXI_ERR_COND_SHIFT            0
#define    CSR_DBG_PORT0_AXI_ERR_COND_PORT0_AXI_ERR_COND_MASK             0x00000FFF

#define CSR_DBG_PORT1_AXI_ERR_COND_OFFSET                                 0x00000318
#define CSR_DBG_PORT1_AXI_ERR_COND_TYPE                                   UInt32
#define CSR_DBG_PORT1_AXI_ERR_COND_RESERVED_MASK                          0xFFFFF000
#define    CSR_DBG_PORT1_AXI_ERR_COND_PORT1_AXI_ERR_COND_SHIFT            0
#define    CSR_DBG_PORT1_AXI_ERR_COND_PORT1_AXI_ERR_COND_MASK             0x00000FFF

#define CSR_DBG_PORT2_AXI_ERR_COND_OFFSET                                 0x0000031C
#define CSR_DBG_PORT2_AXI_ERR_COND_TYPE                                   UInt32
#define CSR_DBG_PORT2_AXI_ERR_COND_RESERVED_MASK                          0xFFFFF000
#define    CSR_DBG_PORT2_AXI_ERR_COND_PORT2_AXI_ERR_COND_SHIFT            0
#define    CSR_DBG_PORT2_AXI_ERR_COND_PORT2_AXI_ERR_COND_MASK             0x00000FFF

#define CSR_DBG_PORT3_AXI_ERR_COND_OFFSET                                 0x00000320
#define CSR_DBG_PORT3_AXI_ERR_COND_TYPE                                   UInt32
#define CSR_DBG_PORT3_AXI_ERR_COND_RESERVED_MASK                          0xFFFFF000
#define    CSR_DBG_PORT3_AXI_ERR_COND_PORT3_AXI_ERR_COND_SHIFT            0
#define    CSR_DBG_PORT3_AXI_ERR_COND_PORT3_AXI_ERR_COND_MASK             0x00000FFF

#define CSR_DEMESH_HALT_ARB_OFFSET                                        0x0000032C
#define CSR_DEMESH_HALT_ARB_TYPE                                          UInt32
#define CSR_DEMESH_HALT_ARB_RESERVED_MASK                                 0xFFFFFFFE
#define    CSR_DEMESH_HALT_ARB_HALT_ARB_ON_URG_MISS_SHIFT                 0
#define    CSR_DEMESH_HALT_ARB_HALT_ARB_ON_URG_MISS_MASK                  0x00000001

#endif /* __BRCM_RDB_CSR_H__ */


