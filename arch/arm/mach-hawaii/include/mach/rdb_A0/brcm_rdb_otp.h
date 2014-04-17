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

#ifndef __BRCM_RDB_OTP_H__
#define __BRCM_RDB_OTP_H__

#define OTP_CFG_OFFSET                                                    0x00000000
#define OTP_CFG_TYPE                                                      UInt32
#define OTP_CFG_RESERVED_MASK                                             0xFF8F2FEE
#define    OTP_CFG_OTP_CLK_DIV_EN_SHIFT                                   22
#define    OTP_CFG_OTP_CLK_DIV_EN_MASK                                    0x00400000
#define    OTP_CFG_OTP_CLK_DIV_RATIO_SHIFT                                20
#define    OTP_CFG_OTP_CLK_DIV_RATIO_MASK                                 0x00300000
#define    OTP_CFG_OTP_CPU_MODE_EN_SHIFT                                  15
#define    OTP_CFG_OTP_CPU_MODE_EN_MASK                                   0x00008000
#define    OTP_CFG_RST_SHIFT                                              14
#define    OTP_CFG_RST_MASK                                               0x00004000
#define    OTP_CFG_OTP_ENABLE_SHIFT                                       12
#define    OTP_CFG_OTP_ENABLE_MASK                                        0x00001000
#define    OTP_CFG_USE_TEST_KEYS_SHIFT                                    4
#define    OTP_CFG_USE_TEST_KEYS_MASK                                     0x00000010
#define    OTP_CFG_OTP_HW_INIT_DONE_SHIFT                                 0
#define    OTP_CFG_OTP_HW_INIT_DONE_MASK                                  0x00000001

#define OTP_WDATA_0_OFFSET                                                0x00000004
#define OTP_WDATA_0_TYPE                                                  UInt32
#define OTP_WDATA_0_RESERVED_MASK                                         0x00000000
#define    OTP_WDATA_0_OTP_WDATA_DIN_SHIFT                                0
#define    OTP_WDATA_0_OTP_WDATA_DIN_MASK                                 0xFFFFFFFF

#define OTP_WDATA_1_OFFSET                                                0x00000008
#define OTP_WDATA_1_TYPE                                                  UInt32
#define OTP_WDATA_1_RESERVED_MASK                                         0xFFFFFE00
#define    OTP_WDATA_1_OTP_WDATA_FAIL_SHIFT                               7
#define    OTP_WDATA_1_OTP_WDATA_FAIL_MASK                                0x00000180
#define    OTP_WDATA_1_OTP_WDATA_ECC_SHIFT                                0
#define    OTP_WDATA_1_OTP_WDATA_ECC_MASK                                 0x0000007F

#define OTP_ADDR_OFFSET                                                   0x0000000C
#define OTP_ADDR_TYPE                                                     UInt32
#define OTP_ADDR_RESERVED_MASK                                            0xFFFFC000
#define    OTP_ADDR_OTP_ADDR_SHIFT                                        6
#define    OTP_ADDR_OTP_ADDR_MASK                                         0x00003FC0
#define    OTP_ADDR_OTP_BITSEL_SHIFT                                      0
#define    OTP_ADDR_OTP_BITSEL_MASK                                       0x0000003F

#define OTP_CTRL_OFFSET                                                   0x00000010
#define OTP_CTRL_TYPE                                                     UInt32
#define OTP_CTRL_RESERVED_MASK                                            0xFCFF8CE0
#define    OTP_CTRL_OTP_CTRL_ENECC_SHIFT                                  25
#define    OTP_CTRL_OTP_CTRL_ENECC_MASK                                   0x02000000
#define    OTP_CTRL_OTP_CTRL_ECC_WREN_SHIFT                               24
#define    OTP_CTRL_OTP_CTRL_ECC_WREN_MASK                                0x01000000
#define    OTP_CTRL_OTP_CTRL_BYPASS_OTP_CLK_SHIFT                         14
#define    OTP_CTRL_OTP_CTRL_BYPASS_OTP_CLK_MASK                          0x00004000
#define    OTP_CTRL_OTP_CTRL_READ_FOUT_SHIFT                              13
#define    OTP_CTRL_OTP_CTRL_READ_FOUT_MASK                               0x00002000
#define    OTP_CTRL_OTP_CTRL_BURST_STAT_SEL_SHIFT                         12
#define    OTP_CTRL_OTP_CTRL_BURST_STAT_SEL_MASK                          0x00001000
#define    OTP_CTRL_OTP_CTRL_ACCESS_MODE_SHIFT                            8
#define    OTP_CTRL_OTP_CTRL_ACCESS_MODE_MASK                             0x00000300
#define    OTP_CTRL_OTP_CTRL_OTP_CMD_SHIFT                                0
#define    OTP_CTRL_OTP_CTRL_OTP_CMD_MASK                                 0x0000001F

#define OTP_STAT_OFFSET                                                   0x00000014
#define OTP_STAT_TYPE                                                     UInt32
#define OTP_STAT_RESERVED_MASK                                            0xFFFC0000
#define    OTP_STAT_ECC_DET_SHIFT                                         17
#define    OTP_STAT_ECC_DET_MASK                                          0x00020000
#define    OTP_STAT_ECC_COR_SHIFT                                         16
#define    OTP_STAT_ECC_COR_MASK                                          0x00010000
#define    OTP_STAT_INVALID_SEC_ACC_SHIFT                                 15
#define    OTP_STAT_INVALID_SEC_ACC_MASK                                  0x00008000
#define    OTP_STAT_INVALID_ACC_MODE_SHIFT                                14
#define    OTP_STAT_INVALID_ACC_MODE_MASK                                 0x00004000
#define    OTP_STAT_INVALID_ADDR_SHIFT                                    13
#define    OTP_STAT_INVALID_ADDR_MASK                                     0x00002000
#define    OTP_STAT_PROGOK_SHIFT                                          12
#define    OTP_STAT_PROGOK_MASK                                           0x00001000
#define    OTP_STAT_REFOK_SHIFT                                           11
#define    OTP_STAT_REFOK_MASK                                            0x00000800
#define    OTP_STAT_WRP_ERR_SHIFT                                         10
#define    OTP_STAT_WRP_ERR_MASK                                          0x00000400
#define    OTP_STAT_INVALID_CMD_SHIFT                                     9
#define    OTP_STAT_INVALID_CMD_MASK                                      0x00000200
#define    OTP_STAT_STBY_SHIFT                                            8
#define    OTP_STAT_STBY_MASK                                             0x00000100
#define    OTP_STAT_WRP_FDONE_SHIFT                                       7
#define    OTP_STAT_WRP_FDONE_MASK                                        0x00000080
#define    OTP_STAT_PROG_BLOCKED_SHIFT                                    6
#define    OTP_STAT_PROG_BLOCKED_MASK                                     0x00000040
#define    OTP_STAT_INVALID_PROG_REQ_SHIFT                                5
#define    OTP_STAT_INVALID_PROG_REQ_MASK                                 0x00000020
#define    OTP_STAT_WRP_FAIL_SHIFT                                        4
#define    OTP_STAT_WRP_FAIL_MASK                                         0x00000010
#define    OTP_STAT_WRP_BUSY_SHIFT                                        3
#define    OTP_STAT_WRP_BUSY_MASK                                         0x00000008
#define    OTP_STAT_WRP_BIT_DOUT_SHIFT                                    2
#define    OTP_STAT_WRP_BIT_DOUT_MASK                                     0x00000004
#define    OTP_STAT_WRP_DATA_READY_SHIFT                                  1
#define    OTP_STAT_WRP_DATA_READY_MASK                                   0x00000002
#define    OTP_STAT_CMD_DONE_SHIFT                                        0
#define    OTP_STAT_CMD_DONE_MASK                                         0x00000001

#define OTP_RDATA_0_OFFSET                                                0x00000018
#define OTP_RDATA_0_TYPE                                                  UInt32
#define OTP_RDATA_0_RESERVED_MASK                                         0x00000000
#define    OTP_RDATA_0_OTP_RDATA_DOUT_SHIFT                               0
#define    OTP_RDATA_0_OTP_RDATA_DOUT_MASK                                0xFFFFFFFF

#define OTP_RDATA_1_OFFSET                                                0x0000001C
#define OTP_RDATA_1_TYPE                                                  UInt32
#define OTP_RDATA_1_RESERVED_MASK                                         0xFFFFFE00
#define    OTP_RDATA_1_OTP_RDATA_FAIL_SHIFT                               7
#define    OTP_RDATA_1_OTP_RDATA_FAIL_MASK                                0x00000180
#define    OTP_RDATA_1_OTP_RDATA_ECC_SHIFT                                0
#define    OTP_RDATA_1_OTP_RDATA_ECC_MASK                                 0x0000007F

#define OTP_DEBUG_0_OFFSET                                                0x00000028
#define OTP_DEBUG_0_TYPE                                                  UInt32
#define OTP_DEBUG_0_RESERVED_MASK                                         0xFFFE0000
#define    OTP_DEBUG_0_OTP_CTRL_TESTCOL_SHIFT                             16
#define    OTP_DEBUG_0_OTP_CTRL_TESTCOL_MASK                              0x00010000
#define    OTP_DEBUG_0_OTP_CTRL_CPU_DEBUG_SEL_SHIFT                       12
#define    OTP_DEBUG_0_OTP_CTRL_CPU_DEBUG_SEL_MASK                        0x0000F000
#define    OTP_DEBUG_0_OTP_CTRL_OTP_DEBUG_MODE_SHIFT                      11
#define    OTP_DEBUG_0_OTP_CTRL_OTP_DEBUG_MODE_MASK                       0x00000800
#define    OTP_DEBUG_0_OTP_CTRL_WRP_CONTINUE_ON_FAIL_SHIFT                10
#define    OTP_DEBUG_0_OTP_CTRL_WRP_CONTINUE_ON_FAIL_MASK                 0x00000400
#define    OTP_DEBUG_0_OTP_CTRL_PROGRAM_VERIFY_FLAG_SHIFT                 9
#define    OTP_DEBUG_0_OTP_CTRL_PROGRAM_VERIFY_FLAG_MASK                  0x00000200
#define    OTP_DEBUG_0_OTP_CTRL_WRP_DOUBLE_WORD_SHIFT                     8
#define    OTP_DEBUG_0_OTP_CTRL_WRP_DOUBLE_WORD_MASK                      0x00000100
#define    OTP_DEBUG_0_OTP_CTRL_WRP_REGC_SEL_SHIFT                        5
#define    OTP_DEBUG_0_OTP_CTRL_WRP_REGC_SEL_MASK                         0x000000E0
#define    OTP_DEBUG_0_OTP_CTRL_WRP_QUADFUSE_SHIFT                        4
#define    OTP_DEBUG_0_OTP_CTRL_WRP_QUADFUSE_MASK                         0x00000010
#define    OTP_DEBUG_0_OTP_CTRL_WRP_DOUBLEFUSE_SHIFT                      3
#define    OTP_DEBUG_0_OTP_CTRL_WRP_DOUBLEFUSE_MASK                       0x00000008
#define    OTP_DEBUG_0_OTP_CTRL_WRP_READ4X_SHIFT                          2
#define    OTP_DEBUG_0_OTP_CTRL_WRP_READ4X_MASK                           0x00000004
#define    OTP_DEBUG_0_OTP_CTRL_WRP_READ2X_SHIFT                          1
#define    OTP_DEBUG_0_OTP_CTRL_WRP_READ2X_MASK                           0x00000002
#define    OTP_DEBUG_0_OTP_CTRL_WRP_PROG_IN_DEBUG_SHIFT                   0
#define    OTP_DEBUG_0_OTP_CTRL_WRP_PROG_IN_DEBUG_MASK                    0x00000001

#define OTP_DEBUG_1_OFFSET                                                0x0000002C
#define OTP_DEBUG_1_TYPE                                                  UInt32
#define OTP_DEBUG_1_RESERVED_MASK                                         0xF0000000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_FUSESEL0_SHIFT                        27
#define    OTP_DEBUG_1_OTP_CTRL_WRP_FUSESEL0_MASK                         0x08000000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_PBYP_SHIFT                            26
#define    OTP_DEBUG_1_OTP_CTRL_WRP_PBYP_MASK                             0x04000000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_SADBYP_SHIFT                          25
#define    OTP_DEBUG_1_OTP_CTRL_WRP_SADBYP_MASK                           0x02000000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_PCOUNT_SHIFT                          21
#define    OTP_DEBUG_1_OTP_CTRL_WRP_PCOUNT_MASK                           0x01E00000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_CPC_SEL_SHIFT                         17
#define    OTP_DEBUG_1_OTP_CTRL_WRP_CPC_SEL_MASK                          0x001E0000
#define    OTP_DEBUG_1_OTP_CTRL_WRP_VSEL_SHIFT                            9
#define    OTP_DEBUG_1_OTP_CTRL_WRP_VSEL_MASK                             0x0001FE00
#define    OTP_DEBUG_1_OTP_CTRL_WRP_TM_SHIFT                              0
#define    OTP_DEBUG_1_OTP_CTRL_WRP_TM_MASK                               0x000001FF

#endif /* __BRCM_RDB_OTP_H__ */


