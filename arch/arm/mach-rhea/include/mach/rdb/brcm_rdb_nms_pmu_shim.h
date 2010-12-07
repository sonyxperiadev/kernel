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
/*     Date     : Generated on 11/9/2010 1:17:6                                             */
/*     RDB file : //R4/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_NMS_PMU_SHIM_H__
#define __BRCM_RDB_NMS_PMU_SHIM_H__

#define NMS_PMU_SHIM_CFG0_REG_OFFSET                                      0x00000000
#define NMS_PMU_SHIM_CFG0_REG_TYPE                                        UInt32
#define NMS_PMU_SHIM_CFG0_REG_RESERVED_MASK                               0x00000000
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_31TO30_SHIFT                    30
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_31TO30_MASK                     0xC0000000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_29TO28_SHIFT           28
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_29TO28_MASK            0x30000000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_27TO24_SHIFT           24
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_27TO24_MASK            0x0F000000
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_23TO21_SHIFT                    22
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_23TO21_MASK                     0x00C00000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_21TO20_SHIFT           20
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_21TO20_MASK            0x00300000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_19TO16_SHIFT           16
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_19TO16_MASK            0x000F0000
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_15TO14_SHIFT                    14
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_15TO14_MASK                     0x0000C000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_13TO12_SHIFT           12
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_13TO12_MASK            0x00003000
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_11TO8_SHIFT            8
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_11TO8_MASK             0x00000F00
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_7TO6_SHIFT                      6
#define    NMS_PMU_SHIM_CFG0_REG_RESERVED_7TO6_MASK                       0x000000C0
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_5TO4_SHIFT             4
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_5TO4_MASK              0x00000030
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_3TO0_SHIFT             0
#define    NMS_PMU_SHIM_CFG0_REG_PMU_SHIM_CFG0_REG_3TO0_MASK              0x0000000F

#define NMS_PMU_SHIM_CFG1_REG_OFFSET                                      0x00000004
#define NMS_PMU_SHIM_CFG1_REG_TYPE                                        UInt32
#define NMS_PMU_SHIM_CFG1_REG_RESERVED_MASK                               0x00000000
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_31TO30_SHIFT                    30
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_31TO30_MASK                     0xC0000000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_29TO28_SHIFT           28
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_29TO28_MASK            0x30000000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_27TO24_SHIFT           24
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_27TO24_MASK            0x0F000000
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_23TO22_SHIFT                    22
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_23TO22_MASK                     0x00C00000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_21TO20_SHIFT           20
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_21TO20_MASK            0x00300000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_19TO16_SHIFT           16
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_19TO16_MASK            0x000F0000
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_15TO14_SHIFT                    14
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_15TO14_MASK                     0x0000C000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_13TO12_SHIFT           12
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_13TO12_MASK            0x00003000
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_11TO8_SHIFT            8
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_11TO8_MASK             0x00000F00
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_7TO6_SHIFT                      6
#define    NMS_PMU_SHIM_CFG1_REG_RESERVED_7TO6_MASK                       0x000000C0
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_5TO4_SHIFT             4
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_5TO4_MASK              0x00000030
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_3TO0_SHIFT             0
#define    NMS_PMU_SHIM_CFG1_REG_PMU_SHIM_CFG1_REG_3TO0_MASK              0x0000000F

#define NMS_PMU_SHIM_CFG2_REG_OFFSET                                      0x00000008
#define NMS_PMU_SHIM_CFG2_REG_TYPE                                        UInt32
#define NMS_PMU_SHIM_CFG2_REG_RESERVED_MASK                               0x00000000
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_31TO30_SHIFT                    30
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_31TO30_MASK                     0xC0000000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_29TO28_SHIFT           28
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_29TO28_MASK            0x30000000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_27TO24_SHIFT           24
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_27TO24_MASK            0x0F000000
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_23TO22_SHIFT                    22
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_23TO22_MASK                     0x00C00000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_21TO20_SHIFT           20
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_21TO20_MASK            0x00300000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_19TO16_SHIFT           16
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_19TO16_MASK            0x000F0000
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_15TO14_SHIFT                    14
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_15TO14_MASK                     0x0000C000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_13TO12_SHIFT           12
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_13TO12_MASK            0x00003000
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_11TO8_SHIFT            8
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_11TO8_MASK             0x00000F00
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_7TO6_SHIFT                      6
#define    NMS_PMU_SHIM_CFG2_REG_RESERVED_7TO6_MASK                       0x000000C0
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_5TO4_SHIFT             4
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_5TO4_MASK              0x00000030
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_3TO0_SHIFT             0
#define    NMS_PMU_SHIM_CFG2_REG_PMU_SHIM_CFG2_REG_3TO0_MASK              0x0000000F

#define NMS_PMU_SHIM_CFG3_REG_OFFSET                                      0x0000000C
#define NMS_PMU_SHIM_CFG3_REG_TYPE                                        UInt32
#define NMS_PMU_SHIM_CFG3_REG_RESERVED_MASK                               0x00000000
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_31TO30_SHIFT                    30
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_31TO30_MASK                     0xC0000000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_29TO28_SHIFT           28
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_29TO28_MASK            0x30000000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_27TO24_SHIFT           24
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_27TO24_MASK            0x0F000000
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_23TO22_SHIFT                    22
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_23TO22_MASK                     0x00C00000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_21TO20_SHIFT           20
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_21TO20_MASK            0x00300000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG1_REG_19TO16_SHIFT           16
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG1_REG_19TO16_MASK            0x000F0000
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_15TO14_SHIFT                    14
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_15TO14_MASK                     0x0000C000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_13TO12_SHIFT           12
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_13TO12_MASK            0x00003000
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_11TO8_SHIFT            8
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_11TO8_MASK             0x00000F00
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_7TO6_SHIFT                      6
#define    NMS_PMU_SHIM_CFG3_REG_RESERVED_7TO6_MASK                       0x000000C0
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_5TO4_SHIFT             4
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_5TO4_MASK              0x00000030
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_3TO0_SHIFT             0
#define    NMS_PMU_SHIM_CFG3_REG_PMU_SHIM_CFG3_REG_3TO0_MASK              0x0000000F

#define NMS_PMU_SHIM_CFG4_REG_OFFSET                                      0x00000010
#define NMS_PMU_SHIM_CFG4_REG_TYPE                                        UInt32
#define NMS_PMU_SHIM_CFG4_REG_RESERVED_MASK                               0x00000000
#define    NMS_PMU_SHIM_CFG4_REG_RFIFBUS_HOSTCLKREQ_HOLD_COUNT_SHIFT      28
#define    NMS_PMU_SHIM_CFG4_REG_RFIFBUS_HOSTCLKREQ_HOLD_COUNT_MASK       0xF0000000
#define    NMS_PMU_SHIM_CFG4_REG_MEMIF_HOSTCLKREQ_HOLD_COUNT_SHIFT        24
#define    NMS_PMU_SHIM_CFG4_REG_MEMIF_HOSTCLKREQ_HOLD_COUNT_MASK         0x0F000000
#define    NMS_PMU_SHIM_CFG4_REG_MODEMBUS_HOSTCLKREQ_HOLD_COUNT_SHIFT     20
#define    NMS_PMU_SHIM_CFG4_REG_MODEMBUS_HOSTCLKREQ_HOLD_COUNT_MASK      0x00F00000
#define    NMS_PMU_SHIM_CFG4_REG_HOSTCLKRST_COUNT_SHIFT                   16
#define    NMS_PMU_SHIM_CFG4_REG_HOSTCLKRST_COUNT_MASK                    0x000F0000
#define    NMS_PMU_SHIM_CFG4_REG_HOSTACCESS_HOLD_COUNT_SHIFT              8
#define    NMS_PMU_SHIM_CFG4_REG_HOSTACCESS_HOLD_COUNT_MASK               0x0000FF00
#define    NMS_PMU_SHIM_CFG4_REG_VWAIT_COUNT_SHIFT                        0
#define    NMS_PMU_SHIM_CFG4_REG_VWAIT_COUNT_MASK                         0x000000FF

#define NMS_PMU_SHIM_STATUS_REG_OFFSET                                    0x00000014
#define NMS_PMU_SHIM_STATUS_REG_TYPE                                      UInt32
#define NMS_PMU_SHIM_STATUS_REG_RESERVED_MASK                             0x00000000
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_31TO27_SHIFT                  27
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_31TO27_MASK                   0xF8000000
#define    NMS_PMU_SHIM_STATUS_REG_WGM_MEM_STBY_SHIFT                     25
#define    NMS_PMU_SHIM_STATUS_REG_WGM_MEM_STBY_MASK                      0x06000000
#define    NMS_PMU_SHIM_STATUS_REG_ATB_CLK_REQ_FORCE_SHIFT                24
#define    NMS_PMU_SHIM_STATUS_REG_ATB_CLK_REQ_FORCE_MASK                 0x01000000
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_23TO16_SHIFT                  16
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_23TO16_MASK                   0x00FF0000
#define    NMS_PMU_SHIM_STATUS_REG_WGMHAMSA_VOLTAGE_ACTUAL_SHIFT          12
#define    NMS_PMU_SHIM_STATUS_REG_WGMHAMSA_VOLTAGE_ACTUAL_MASK           0x0000F000
#define    NMS_PMU_SHIM_STATUS_REG_WGMCORE_VOLTAGE_ACTUAL_SHIFT           8
#define    NMS_PMU_SHIM_STATUS_REG_WGMCORE_VOLTAGE_ACTUAL_MASK            0x00000F00
#define    NMS_PMU_SHIM_STATUS_REG_WGM_PWR_STATE_CURR_VAL_SHIFT           6
#define    NMS_PMU_SHIM_STATUS_REG_WGM_PWR_STATE_CURR_VAL_MASK            0x000000C0
#define    NMS_PMU_SHIM_STATUS_REG_WGM_HASMAPDOK_SHIFT                    5
#define    NMS_PMU_SHIM_STATUS_REG_WGM_HASMAPDOK_MASK                     0x00000020
#define    NMS_PMU_SHIM_STATUS_REG_WGM_COREPDOK_SHIFT                     4
#define    NMS_PMU_SHIM_STATUS_REG_WGM_COREPDOK_MASK                      0x00000010
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_3TO1_SHIFT                    1
#define    NMS_PMU_SHIM_STATUS_REG_RESERVED_3TO1_MASK                     0x0000000E
#define    NMS_PMU_SHIM_STATUS_REG_MEMC_ERR_INT_SHIFT                     0
#define    NMS_PMU_SHIM_STATUS_REG_MEMC_ERR_INT_MASK                      0x00000001

#define NMS_PMU_SHIM_STATUS_SET_REG_OFFSET                                0x00000018
#define NMS_PMU_SHIM_STATUS_SET_REG_TYPE                                  UInt32
#define NMS_PMU_SHIM_STATUS_SET_REG_RESERVED_MASK                         0x00000000
#define    NMS_PMU_SHIM_STATUS_SET_REG_PMU_SHIM_STATUS_REG_SET_SHIFT      0
#define    NMS_PMU_SHIM_STATUS_SET_REG_PMU_SHIM_STATUS_REG_SET_MASK       0xFFFFFFFF

#define NMS_PMU_SHIM_STATUS_CLEAR_REG_OFFSET                              0x0000001C
#define NMS_PMU_SHIM_STATUS_CLEAR_REG_TYPE                                UInt32
#define NMS_PMU_SHIM_STATUS_CLEAR_REG_RESERVED_MASK                       0x00000000
#define    NMS_PMU_SHIM_STATUS_CLEAR_REG_PMU_SHIM_STATUS_REG_CLEAR_SHIFT  0
#define    NMS_PMU_SHIM_STATUS_CLEAR_REG_PMU_SHIM_STATUS_REG_CLEAR_MASK   0xFFFFFFFF

typedef volatile struct {
   UInt32 m_CFG0_REG;                // 0x0000
   UInt32 m_CFG1_REG;                // 0x0004
   UInt32 m_CFG2_REG;                // 0x0008
   UInt32 m_CFG3_REG;                // 0x000C
   UInt32 m_CFG4_REG;                // 0x0010
   UInt32 m_STATUS_REG;              // 0x0014
   UInt32 m_STATUS_SET_REG;          // 0x0018
   UInt32 m_STATUS_CLEAR_REG;        // 0x001C
} BRCM_NMS_PMU_SHIM_REGS;


#endif /* __BRCM_RDB_NMS_PMU_SHIM_H__ */


