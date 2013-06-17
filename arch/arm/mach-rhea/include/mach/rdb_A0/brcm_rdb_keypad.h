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

#ifndef __BRCM_RDB_KEYPAD_H__
#define __BRCM_RDB_KEYPAD_H__

#define KEYPAD_KPCR_OFFSET                                                0x00000080
#define KEYPAD_KPCR_TYPE                                                  UInt32
#define KEYPAD_KPCR_RESERVED_MASK                                         0xFF8800F0
#define    KEYPAD_KPCR_ROWWIDTH_SHIFT                                     20
#define    KEYPAD_KPCR_ROWWIDTH_MASK                                      0x00700000
#define    KEYPAD_KPCR_COLUMNWIDTH_SHIFT                                  16
#define    KEYPAD_KPCR_COLUMNWIDTH_MASK                                   0x00070000
#define    KEYPAD_KPCR_STATUSFILTERENABLE_SHIFT                           15
#define    KEYPAD_KPCR_STATUSFILTERENABLE_MASK                            0x00008000
#define    KEYPAD_KPCR_STATUSFILTERTYPE_SHIFT                             12
#define    KEYPAD_KPCR_STATUSFILTERTYPE_MASK                              0x00007000
#define    KEYPAD_KPCR_COLFILTERENABLE_SHIFT                              11
#define    KEYPAD_KPCR_COLFILTERENABLE_MASK                               0x00000800
#define    KEYPAD_KPCR_COLUMNFILTERTYPE_SHIFT                             8
#define    KEYPAD_KPCR_COLUMNFILTERTYPE_MASK                              0x00000700
#define    KEYPAD_KPCR_IOMODE_SHIFT                                       3
#define    KEYPAD_KPCR_IOMODE_MASK                                        0x00000008
#define    KEYPAD_KPCR_SWAPROWCOLUMN_SHIFT                                2
#define    KEYPAD_KPCR_SWAPROWCOLUMN_MASK                                 0x00000004
#define    KEYPAD_KPCR_MODE_SHIFT                                         1
#define    KEYPAD_KPCR_MODE_MASK                                          0x00000002
#define    KEYPAD_KPCR_ENABLE_SHIFT                                       0
#define    KEYPAD_KPCR_ENABLE_MASK                                        0x00000001

#define KEYPAD_KPIOR_OFFSET                                               0x00000084
#define KEYPAD_KPIOR_TYPE                                                 UInt32
#define KEYPAD_KPIOR_RESERVED_MASK                                        0x00000000
#define    KEYPAD_KPIOR_ROWOCONTRL_SHIFT                                  24
#define    KEYPAD_KPIOR_ROWOCONTRL_MASK                                   0xFF000000
#define    KEYPAD_KPIOR_COLUMNOCONTRL_SHIFT                               16
#define    KEYPAD_KPIOR_COLUMNOCONTRL_MASK                                0x00FF0000
#define    KEYPAD_KPIOR_ROWIODATA_SHIFT                                   8
#define    KEYPAD_KPIOR_ROWIODATA_MASK                                    0x0000FF00
#define    KEYPAD_KPIOR_COLUMNIODATA_SHIFT                                0
#define    KEYPAD_KPIOR_COLUMNIODATA_MASK                                 0x000000FF

#define KEYPAD_KPEMR0_OFFSET                                              0x00000090
#define KEYPAD_KPEMR0_TYPE                                                UInt32
#define KEYPAD_KPEMR0_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPEMR0_KEYPRESS15_SHIFT                                 30
#define    KEYPAD_KPEMR0_KEYPRESS15_MASK                                  0xC0000000
#define    KEYPAD_KPEMR0_KEYPRESS14_SHIFT                                 28
#define    KEYPAD_KPEMR0_KEYPRESS14_MASK                                  0x30000000
#define    KEYPAD_KPEMR0_KEYPRESS13_SHIFT                                 26
#define    KEYPAD_KPEMR0_KEYPRESS13_MASK                                  0x0C000000
#define    KEYPAD_KPEMR0_KEYPRESS12_SHIFT                                 24
#define    KEYPAD_KPEMR0_KEYPRESS12_MASK                                  0x03000000
#define    KEYPAD_KPEMR0_KEYPRESS11_SHIFT                                 22
#define    KEYPAD_KPEMR0_KEYPRESS11_MASK                                  0x00C00000
#define    KEYPAD_KPEMR0_KEYPRESS10_SHIFT                                 20
#define    KEYPAD_KPEMR0_KEYPRESS10_MASK                                  0x00300000
#define    KEYPAD_KPEMR0_KEYPRESS9_SHIFT                                  18
#define    KEYPAD_KPEMR0_KEYPRESS9_MASK                                   0x000C0000
#define    KEYPAD_KPEMR0_KEYPRESS8_SHIFT                                  16
#define    KEYPAD_KPEMR0_KEYPRESS8_MASK                                   0x00030000
#define    KEYPAD_KPEMR0_KEYPRESS7_SHIFT                                  14
#define    KEYPAD_KPEMR0_KEYPRESS7_MASK                                   0x0000C000
#define    KEYPAD_KPEMR0_KEYPRESS6_SHIFT                                  12
#define    KEYPAD_KPEMR0_KEYPRESS6_MASK                                   0x00003000
#define    KEYPAD_KPEMR0_KEYPRESS5_SHIFT                                  10
#define    KEYPAD_KPEMR0_KEYPRESS5_MASK                                   0x00000C00
#define    KEYPAD_KPEMR0_KEYPRESS4_SHIFT                                  8
#define    KEYPAD_KPEMR0_KEYPRESS4_MASK                                   0x00000300
#define    KEYPAD_KPEMR0_KEYPRESS3_SHIFT                                  6
#define    KEYPAD_KPEMR0_KEYPRESS3_MASK                                   0x000000C0
#define    KEYPAD_KPEMR0_KEYPRESS2_SHIFT                                  4
#define    KEYPAD_KPEMR0_KEYPRESS2_MASK                                   0x00000030
#define    KEYPAD_KPEMR0_KEYPRESS1_SHIFT                                  2
#define    KEYPAD_KPEMR0_KEYPRESS1_MASK                                   0x0000000C
#define    KEYPAD_KPEMR0_KEYPRESS0_SHIFT                                  0
#define    KEYPAD_KPEMR0_KEYPRESS0_MASK                                   0x00000003

#define KEYPAD_KPEMR1_OFFSET                                              0x00000094
#define KEYPAD_KPEMR1_TYPE                                                UInt32
#define KEYPAD_KPEMR1_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPEMR1_KEYPRESS31_SHIFT                                 30
#define    KEYPAD_KPEMR1_KEYPRESS31_MASK                                  0xC0000000
#define    KEYPAD_KPEMR1_KEYPRESS30_SHIFT                                 28
#define    KEYPAD_KPEMR1_KEYPRESS30_MASK                                  0x30000000
#define    KEYPAD_KPEMR1_KEYPRESS29_SHIFT                                 26
#define    KEYPAD_KPEMR1_KEYPRESS29_MASK                                  0x0C000000
#define    KEYPAD_KPEMR1_KEYPRESS28_SHIFT                                 24
#define    KEYPAD_KPEMR1_KEYPRESS28_MASK                                  0x03000000
#define    KEYPAD_KPEMR1_KEYPRESS27_SHIFT                                 22
#define    KEYPAD_KPEMR1_KEYPRESS27_MASK                                  0x00C00000
#define    KEYPAD_KPEMR1_KEYPRESS26_SHIFT                                 20
#define    KEYPAD_KPEMR1_KEYPRESS26_MASK                                  0x00300000
#define    KEYPAD_KPEMR1_KEYPRESS25_SHIFT                                 18
#define    KEYPAD_KPEMR1_KEYPRESS25_MASK                                  0x000C0000
#define    KEYPAD_KPEMR1_KEYPRESS24_SHIFT                                 16
#define    KEYPAD_KPEMR1_KEYPRESS24_MASK                                  0x00030000
#define    KEYPAD_KPEMR1_KEYPRESS23_SHIFT                                 14
#define    KEYPAD_KPEMR1_KEYPRESS23_MASK                                  0x0000C000
#define    KEYPAD_KPEMR1_KEYPRESS22_SHIFT                                 12
#define    KEYPAD_KPEMR1_KEYPRESS22_MASK                                  0x00003000
#define    KEYPAD_KPEMR1_KEYPRESS21_SHIFT                                 10
#define    KEYPAD_KPEMR1_KEYPRESS21_MASK                                  0x00000C00
#define    KEYPAD_KPEMR1_KEYPRESS20_SHIFT                                 8
#define    KEYPAD_KPEMR1_KEYPRESS20_MASK                                  0x00000300
#define    KEYPAD_KPEMR1_KEYPRESS19_SHIFT                                 6
#define    KEYPAD_KPEMR1_KEYPRESS19_MASK                                  0x000000C0
#define    KEYPAD_KPEMR1_KEYPRESS18_SHIFT                                 4
#define    KEYPAD_KPEMR1_KEYPRESS18_MASK                                  0x00000030
#define    KEYPAD_KPEMR1_KEYPRESS17_SHIFT                                 2
#define    KEYPAD_KPEMR1_KEYPRESS17_MASK                                  0x0000000C
#define    KEYPAD_KPEMR1_KEYPRESS16_SHIFT                                 0
#define    KEYPAD_KPEMR1_KEYPRESS16_MASK                                  0x00000003

#define KEYPAD_KPEMR2_OFFSET                                              0x00000098
#define KEYPAD_KPEMR2_TYPE                                                UInt32
#define KEYPAD_KPEMR2_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPEMR2_KEYPRESS47_SHIFT                                 30
#define    KEYPAD_KPEMR2_KEYPRESS47_MASK                                  0xC0000000
#define    KEYPAD_KPEMR2_KEYPRESS46_SHIFT                                 28
#define    KEYPAD_KPEMR2_KEYPRESS46_MASK                                  0x30000000
#define    KEYPAD_KPEMR2_KEYPRESS45_SHIFT                                 26
#define    KEYPAD_KPEMR2_KEYPRESS45_MASK                                  0x0C000000
#define    KEYPAD_KPEMR2_KEYPRESS44_SHIFT                                 24
#define    KEYPAD_KPEMR2_KEYPRESS44_MASK                                  0x03000000
#define    KEYPAD_KPEMR2_KEYPRESS43_SHIFT                                 22
#define    KEYPAD_KPEMR2_KEYPRESS43_MASK                                  0x00C00000
#define    KEYPAD_KPEMR2_KEYPRESS42_SHIFT                                 20
#define    KEYPAD_KPEMR2_KEYPRESS42_MASK                                  0x00300000
#define    KEYPAD_KPEMR2_KEYPRESS41_SHIFT                                 18
#define    KEYPAD_KPEMR2_KEYPRESS41_MASK                                  0x000C0000
#define    KEYPAD_KPEMR2_KEYPRESS40_SHIFT                                 16
#define    KEYPAD_KPEMR2_KEYPRESS40_MASK                                  0x00030000
#define    KEYPAD_KPEMR2_KEYPRESS39_SHIFT                                 14
#define    KEYPAD_KPEMR2_KEYPRESS39_MASK                                  0x0000C000
#define    KEYPAD_KPEMR2_KEYPRESS38_SHIFT                                 12
#define    KEYPAD_KPEMR2_KEYPRESS38_MASK                                  0x00003000
#define    KEYPAD_KPEMR2_KEYPRESS37_SHIFT                                 10
#define    KEYPAD_KPEMR2_KEYPRESS37_MASK                                  0x00000C00
#define    KEYPAD_KPEMR2_KEYPRESS36_SHIFT                                 8
#define    KEYPAD_KPEMR2_KEYPRESS36_MASK                                  0x00000300
#define    KEYPAD_KPEMR2_KEYPRESS35_SHIFT                                 6
#define    KEYPAD_KPEMR2_KEYPRESS35_MASK                                  0x000000C0
#define    KEYPAD_KPEMR2_KEYPRESS34_SHIFT                                 4
#define    KEYPAD_KPEMR2_KEYPRESS34_MASK                                  0x00000030
#define    KEYPAD_KPEMR2_KEYPRESS33_SHIFT                                 2
#define    KEYPAD_KPEMR2_KEYPRESS33_MASK                                  0x0000000C
#define    KEYPAD_KPEMR2_KEYPRESS32_SHIFT                                 0
#define    KEYPAD_KPEMR2_KEYPRESS32_MASK                                  0x00000003

#define KEYPAD_KPEMR3_OFFSET                                              0x0000009C
#define KEYPAD_KPEMR3_TYPE                                                UInt32
#define KEYPAD_KPEMR3_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPEMR3_KEYPRESS63_SHIFT                                 30
#define    KEYPAD_KPEMR3_KEYPRESS63_MASK                                  0xC0000000
#define    KEYPAD_KPEMR3_KEYPRESS62_SHIFT                                 28
#define    KEYPAD_KPEMR3_KEYPRESS62_MASK                                  0x30000000
#define    KEYPAD_KPEMR3_KEYPRESS61_SHIFT                                 26
#define    KEYPAD_KPEMR3_KEYPRESS61_MASK                                  0x0C000000
#define    KEYPAD_KPEMR3_KEYPRESS60_SHIFT                                 24
#define    KEYPAD_KPEMR3_KEYPRESS60_MASK                                  0x03000000
#define    KEYPAD_KPEMR3_KEYPRESS59_SHIFT                                 22
#define    KEYPAD_KPEMR3_KEYPRESS59_MASK                                  0x00C00000
#define    KEYPAD_KPEMR3_KEYPRESS58_SHIFT                                 20
#define    KEYPAD_KPEMR3_KEYPRESS58_MASK                                  0x00300000
#define    KEYPAD_KPEMR3_KEYPRESS57_SHIFT                                 18
#define    KEYPAD_KPEMR3_KEYPRESS57_MASK                                  0x000C0000
#define    KEYPAD_KPEMR3_KEYPRESS56_SHIFT                                 16
#define    KEYPAD_KPEMR3_KEYPRESS56_MASK                                  0x00030000
#define    KEYPAD_KPEMR3_KEYPRESS55_SHIFT                                 14
#define    KEYPAD_KPEMR3_KEYPRESS55_MASK                                  0x0000C000
#define    KEYPAD_KPEMR3_KEYPRESS54_SHIFT                                 12
#define    KEYPAD_KPEMR3_KEYPRESS54_MASK                                  0x00003000
#define    KEYPAD_KPEMR3_KEYPRESS53_SHIFT                                 10
#define    KEYPAD_KPEMR3_KEYPRESS53_MASK                                  0x00000C00
#define    KEYPAD_KPEMR3_KEYPRESS52_SHIFT                                 8
#define    KEYPAD_KPEMR3_KEYPRESS52_MASK                                  0x00000300
#define    KEYPAD_KPEMR3_KEYPRESS51_SHIFT                                 6
#define    KEYPAD_KPEMR3_KEYPRESS51_MASK                                  0x000000C0
#define    KEYPAD_KPEMR3_KEYPRESS50_SHIFT                                 4
#define    KEYPAD_KPEMR3_KEYPRESS50_MASK                                  0x00000030
#define    KEYPAD_KPEMR3_KEYPRESS49_SHIFT                                 2
#define    KEYPAD_KPEMR3_KEYPRESS49_MASK                                  0x0000000C
#define    KEYPAD_KPEMR3_KEYPRESS48_SHIFT                                 0
#define    KEYPAD_KPEMR3_KEYPRESS48_MASK                                  0x00000003

#define KEYPAD_KPSSR0_OFFSET                                              0x000000A0
#define KEYPAD_KPSSR0_TYPE                                                UInt32
#define KEYPAD_KPSSR0_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPSSR0_KPSSRX_SHIFT                                     0
#define    KEYPAD_KPSSR0_KPSSRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPSSR1_OFFSET                                              0x000000A4
#define KEYPAD_KPSSR1_TYPE                                                UInt32
#define KEYPAD_KPSSR1_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPSSR1_KPSSRX_SHIFT                                     0
#define    KEYPAD_KPSSR1_KPSSRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPIMR0_OFFSET                                              0x000000B0
#define KEYPAD_KPIMR0_TYPE                                                UInt32
#define KEYPAD_KPIMR0_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPIMR0_KPIMRX_SHIFT                                     0
#define    KEYPAD_KPIMR0_KPIMRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPIMR1_OFFSET                                              0x000000B4
#define KEYPAD_KPIMR1_TYPE                                                UInt32
#define KEYPAD_KPIMR1_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPIMR1_KPIMRX_SHIFT                                     0
#define    KEYPAD_KPIMR1_KPIMRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPICR0_OFFSET                                              0x000000B8
#define KEYPAD_KPICR0_TYPE                                                UInt32
#define KEYPAD_KPICR0_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPICR0_KPICRX_SHIFT                                     0
#define    KEYPAD_KPICR0_KPICRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPICR1_OFFSET                                              0x000000BC
#define KEYPAD_KPICR1_TYPE                                                UInt32
#define KEYPAD_KPICR1_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPICR1_KPICRX_SHIFT                                     0
#define    KEYPAD_KPICR1_KPICRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPISR0_OFFSET                                              0x000000C0
#define KEYPAD_KPISR0_TYPE                                                UInt32
#define KEYPAD_KPISR0_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPISR0_KPISRX_SHIFT                                     0
#define    KEYPAD_KPISR0_KPISRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPISR1_OFFSET                                              0x000000C4
#define KEYPAD_KPISR1_TYPE                                                UInt32
#define KEYPAD_KPISR1_RESERVED_MASK                                       0x00000000
#define    KEYPAD_KPISR1_KPISRX_SHIFT                                     0
#define    KEYPAD_KPISR1_KPISRX_MASK                                      0xFFFFFFFF

#define KEYPAD_KPDBCTR_OFFSET                                             0x000000C8
#define KEYPAD_KPDBCTR_TYPE                                               UInt32
#define KEYPAD_KPDBCTR_RESERVED_MASK                                      0xFFFFFFFC
#define    KEYPAD_KPDBCTR_FUNC_MODE_SHIFT                                 0
#define    KEYPAD_KPDBCTR_FUNC_MODE_MASK                                  0x00000003

#endif /* __BRCM_RDB_KEYPAD_H__ */


