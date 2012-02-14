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
/*     Date     : Generated on 2/10/2012 15:44:22                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_GEA_H__
#define __BRCM_RDB_GEA_H__

#define GEA_GPRSK0R_OFFSET                                                0x00000000
#define GEA_GPRSK0R_TYPE                                                  UInt32
#define GEA_GPRSK0R_RESERVED_MASK                                         0x00000000
#define    GEA_GPRSK0R_CRYX_SHIFT                                         0
#define    GEA_GPRSK0R_CRYX_MASK                                          0xFFFFFFFF

#define GEA_GPRSK1R_OFFSET                                                0x00000004
#define GEA_GPRSK1R_TYPE                                                  UInt32
#define GEA_GPRSK1R_RESERVED_MASK                                         0x00000000
#define    GEA_GPRSK1R_CRYX_SHIFT                                         0
#define    GEA_GPRSK1R_CRYX_MASK                                          0xFFFFFFFF

#define GEA_GPRSIR_OFFSET                                                 0x00000008
#define GEA_GPRSIR_TYPE                                                   UInt32
#define GEA_GPRSIR_RESERVED_MASK                                          0x00000000
#define    GEA_GPRSIR_INPUT_SHIFT                                         0
#define    GEA_GPRSIR_INPUT_MASK                                          0xFFFFFFFF

#define GEA_GPRSCR_OFFSET                                                 0x0000000C
#define GEA_GPRSCR_TYPE                                                   UInt32
#define GEA_GPRSCR_RESERVED_MASK                                          0xFFFF7C7E
#define    GEA_GPRSCR_STATUS_SHIFT                                        15
#define    GEA_GPRSCR_STATUS_MASK                                         0x00008000
#define    GEA_GPRSCR_GEA3_REVERSE_SHIFT                                  9
#define    GEA_GPRSCR_GEA3_REVERSE_MASK                                   0x00000200
#define    GEA_GPRSCR_MODE12_SHIFT                                        8
#define    GEA_GPRSCR_MODE12_MASK                                         0x00000100
#define    GEA_GPRSCR_MODE3_SHIFT                                         7
#define    GEA_GPRSCR_MODE3_MASK                                          0x00000080
#define    GEA_GPRSCR_DIR_SHIFT                                           0
#define    GEA_GPRSCR_DIR_MASK                                            0x00000001

#define GEA_GPRSKSR_OFFSET                                                0x00000012
#define GEA_GPRSKSR_TYPE                                                  UInt16
#define GEA_GPRSKSR_RESERVED_MASK                                         0x00000000
#define    GEA_GPRSKSR_GPRSRD_SHIFT                                       0
#define    GEA_GPRSKSR_GPRSRD_MASK                                        0x0000FFFF

#define GEA_GPRSK2R_OFFSET                                                0x00000020
#define GEA_GPRSK2R_TYPE                                                  UInt32
#define GEA_GPRSK2R_RESERVED_MASK                                         0x00000000
#define    GEA_GPRSK2R_CRYX_SHIFT                                         0
#define    GEA_GPRSK2R_CRYX_MASK                                          0xFFFFFFFF

#define GEA_GPRSK3R_OFFSET                                                0x00000024
#define GEA_GPRSK3R_TYPE                                                  UInt32
#define GEA_GPRSK3R_RESERVED_MASK                                         0x00000000
#define    GEA_GPRSK3R_CRYX_SHIFT                                         0
#define    GEA_GPRSK3R_CRYX_MASK                                          0xFFFFFFFF

#define GEA_M_OFFSET                                                      0x00000028
#define GEA_M_TYPE                                                        UInt32
#define GEA_M_RESERVED_MASK                                               0xFFFF7880
#define    GEA_M_RESTART_SHIFT                                            15
#define    GEA_M_RESTART_MASK                                             0x00008000
#define    GEA_M_KLEN_SHIFT                                               8
#define    GEA_M_KLEN_MASK                                                0x00000700
#define    GEA_M_NUMBER_SHIFT                                             0
#define    GEA_M_NUMBER_MASK                                              0x0000007F

#define GEA_CRYDATA0_OFFSET                                               0x00000030
#define GEA_CRYDATA0_TYPE                                                 UInt32
#define GEA_CRYDATA0_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA0_CO_SHIFT                                          0
#define    GEA_CRYDATA0_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA1_OFFSET                                               0x00000034
#define GEA_CRYDATA1_TYPE                                                 UInt32
#define GEA_CRYDATA1_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA1_CO_SHIFT                                          0
#define    GEA_CRYDATA1_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA2_OFFSET                                               0x00000038
#define GEA_CRYDATA2_TYPE                                                 UInt32
#define GEA_CRYDATA2_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA2_CO_SHIFT                                          0
#define    GEA_CRYDATA2_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA3_OFFSET                                               0x0000003C
#define GEA_CRYDATA3_TYPE                                                 UInt32
#define GEA_CRYDATA3_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA3_CO_SHIFT                                          0
#define    GEA_CRYDATA3_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA4_OFFSET                                               0x00000040
#define GEA_CRYDATA4_TYPE                                                 UInt32
#define GEA_CRYDATA4_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA4_CO_SHIFT                                          0
#define    GEA_CRYDATA4_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA5_OFFSET                                               0x00000044
#define GEA_CRYDATA5_TYPE                                                 UInt32
#define GEA_CRYDATA5_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA5_CO_SHIFT                                          0
#define    GEA_CRYDATA5_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA6_OFFSET                                               0x00000048
#define GEA_CRYDATA6_TYPE                                                 UInt32
#define GEA_CRYDATA6_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA6_CO_SHIFT                                          0
#define    GEA_CRYDATA6_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA7_OFFSET                                               0x0000004C
#define GEA_CRYDATA7_TYPE                                                 UInt32
#define GEA_CRYDATA7_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA7_CO_SHIFT                                          0
#define    GEA_CRYDATA7_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA8_OFFSET                                               0x00000050
#define GEA_CRYDATA8_TYPE                                                 UInt32
#define GEA_CRYDATA8_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA8_CO_SHIFT                                          0
#define    GEA_CRYDATA8_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA9_OFFSET                                               0x00000054
#define GEA_CRYDATA9_TYPE                                                 UInt32
#define GEA_CRYDATA9_RESERVED_MASK                                        0x00000000
#define    GEA_CRYDATA9_CO_SHIFT                                          0
#define    GEA_CRYDATA9_CO_MASK                                           0xFFFFFFFF

#define GEA_CRYDATA10_OFFSET                                              0x00000058
#define GEA_CRYDATA10_TYPE                                                UInt32
#define GEA_CRYDATA10_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA10_CO_SHIFT                                         0
#define    GEA_CRYDATA10_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA11_OFFSET                                              0x0000005C
#define GEA_CRYDATA11_TYPE                                                UInt32
#define GEA_CRYDATA11_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA11_CO_SHIFT                                         0
#define    GEA_CRYDATA11_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA12_OFFSET                                              0x00000060
#define GEA_CRYDATA12_TYPE                                                UInt32
#define GEA_CRYDATA12_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA12_CO_SHIFT                                         0
#define    GEA_CRYDATA12_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA13_OFFSET                                              0x00000064
#define GEA_CRYDATA13_TYPE                                                UInt32
#define GEA_CRYDATA13_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA13_CO_SHIFT                                         0
#define    GEA_CRYDATA13_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA14_OFFSET                                              0x00000068
#define GEA_CRYDATA14_TYPE                                                UInt32
#define GEA_CRYDATA14_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA14_CO_SHIFT                                         0
#define    GEA_CRYDATA14_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA15_OFFSET                                              0x0000006C
#define GEA_CRYDATA15_TYPE                                                UInt32
#define GEA_CRYDATA15_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA15_CO_SHIFT                                         0
#define    GEA_CRYDATA15_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA16_OFFSET                                              0x00000070
#define GEA_CRYDATA16_TYPE                                                UInt32
#define GEA_CRYDATA16_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA16_CO_SHIFT                                         0
#define    GEA_CRYDATA16_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA17_OFFSET                                              0x00000074
#define GEA_CRYDATA17_TYPE                                                UInt32
#define GEA_CRYDATA17_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA17_CO_SHIFT                                         0
#define    GEA_CRYDATA17_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA18_OFFSET                                              0x00000078
#define GEA_CRYDATA18_TYPE                                                UInt32
#define GEA_CRYDATA18_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA18_CO_SHIFT                                         0
#define    GEA_CRYDATA18_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA19_OFFSET                                              0x0000007C
#define GEA_CRYDATA19_TYPE                                                UInt32
#define GEA_CRYDATA19_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA19_CO_SHIFT                                         0
#define    GEA_CRYDATA19_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA20_OFFSET                                              0x00000080
#define GEA_CRYDATA20_TYPE                                                UInt32
#define GEA_CRYDATA20_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA20_CO_SHIFT                                         0
#define    GEA_CRYDATA20_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA21_OFFSET                                              0x00000084
#define GEA_CRYDATA21_TYPE                                                UInt32
#define GEA_CRYDATA21_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA21_CO_SHIFT                                         0
#define    GEA_CRYDATA21_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA22_OFFSET                                              0x00000088
#define GEA_CRYDATA22_TYPE                                                UInt32
#define GEA_CRYDATA22_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA22_CO_SHIFT                                         0
#define    GEA_CRYDATA22_CO_MASK                                          0xFFFFFFFF

#define GEA_CRYDATA23_OFFSET                                              0x0000008C
#define GEA_CRYDATA23_TYPE                                                UInt32
#define GEA_CRYDATA23_RESERVED_MASK                                       0x00000000
#define    GEA_CRYDATA23_CO_SHIFT                                         0
#define    GEA_CRYDATA23_CO_MASK                                          0xFFFFFFFF

#endif /* __BRCM_RDB_GEA_H__ */


