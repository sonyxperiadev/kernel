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

#ifndef __BRCM_RDB_JP_H__
#define __BRCM_RDB_JP_H__

#define JP_CTRL_OFFSET                                                    0x00000000
#define JP_CTRL_TYPE                                                      UInt32
#define JP_CTRL_RESERVED_MASK                                             0xFFFFFE20
#define    JP_CTRL_EN_SHIFT                                               8
#define    JP_CTRL_EN_MASK                                                0x00000100
#define    JP_CTRL_START_SHIFT                                            7
#define    JP_CTRL_START_MASK                                             0x00000080
#define    JP_CTRL_WOUT_SHIFT                                             6
#define    JP_CTRL_WOUT_MASK                                              0x00000040
#define    JP_CTRL_DCTEN_SHIFT                                            4
#define    JP_CTRL_DCTEN_MASK                                             0x00000010
#define    JP_CTRL_RESET_SHIFT                                            3
#define    JP_CTRL_RESET_MASK                                             0x00000008
#define    JP_CTRL_FLUSH_SHIFT                                            2
#define    JP_CTRL_FLUSH_MASK                                             0x00000004
#define    JP_CTRL_STUFF_SHIFT                                            1
#define    JP_CTRL_STUFF_MASK                                             0x00000002
#define    JP_CTRL_MODE_SHIFT                                             0
#define    JP_CTRL_MODE_MASK                                              0x00000001

#define JP_ICST_OFFSET                                                    0x00000004
#define JP_ICST_TYPE                                                      UInt32
#define JP_ICST_RESERVED_MASK                                             0xFFF4FFF4
#define    JP_ICST_ERR_SHIFT                                              19
#define    JP_ICST_ERR_MASK                                               0x00080000
#define    JP_ICST_SDONE_SHIFT                                            17
#define    JP_ICST_SDONE_MASK                                             0x00020000
#define    JP_ICST_IDONE_SHIFT                                            16
#define    JP_ICST_IDONE_MASK                                             0x00010000
#define    JP_ICST_INTE_SHIFT                                             3
#define    JP_ICST_INTE_MASK                                              0x00000008
#define    JP_ICST_INTSD_SHIFT                                            1
#define    JP_ICST_INTSD_MASK                                             0x00000002
#define    JP_ICST_INTID_SHIFT                                            0
#define    JP_ICST_INTID_MASK                                             0x00000001

#define JP_MCTRL_OFFSET                                                   0x00000008
#define JP_MCTRL_TYPE                                                     UInt32
#define JP_MCTRL_RESERVED_MASK                                            0x00003800
#define    JP_MCTRL_CMP3_SHIFT                                            28
#define    JP_MCTRL_CMP3_MASK                                             0xF0000000
#define    JP_MCTRL_CMP2_SHIFT                                            24
#define    JP_MCTRL_CMP2_MASK                                             0x0F000000
#define    JP_MCTRL_CMP1_SHIFT                                            20
#define    JP_MCTRL_CMP1_MASK                                             0x00F00000
#define    JP_MCTRL_CMP0_SHIFT                                            16
#define    JP_MCTRL_CMP0_MASK                                             0x000F0000
#define    JP_MCTRL_YUVMODE_SHIFT                                         14
#define    JP_MCTRL_YUVMODE_MASK                                          0x0000C000
#define    JP_MCTRL_NUMCMP_SHIFT                                          8
#define    JP_MCTRL_NUMCMP_MASK                                           0x00000700
#define    JP_MCTRL_TAB3A_SHIFT                                           7
#define    JP_MCTRL_TAB3A_MASK                                            0x00000080
#define    JP_MCTRL_TAB3D_SHIFT                                           6
#define    JP_MCTRL_TAB3D_MASK                                            0x00000040
#define    JP_MCTRL_TAB2A_SHIFT                                           5
#define    JP_MCTRL_TAB2A_MASK                                            0x00000020
#define    JP_MCTRL_TAB2D_SHIFT                                           4
#define    JP_MCTRL_TAB2D_MASK                                            0x00000010
#define    JP_MCTRL_TAB1A_SHIFT                                           3
#define    JP_MCTRL_TAB1A_MASK                                            0x00000008
#define    JP_MCTRL_TAB1D_SHIFT                                           2
#define    JP_MCTRL_TAB1D_MASK                                            0x00000004
#define    JP_MCTRL_TAB0A_SHIFT                                           1
#define    JP_MCTRL_TAB0A_MASK                                            0x00000002
#define    JP_MCTRL_TAB0D_SHIFT                                           0
#define    JP_MCTRL_TAB0D_MASK                                            0x00000001

#define JP_DCCTRL_OFFSET                                                  0x0000000C
#define JP_DCCTRL_TYPE                                                    UInt32
#define JP_DCCTRL_RESERVED_MASK                                           0xFFE00000
#define    JP_DCCTRL_DISDC_SHIFT                                          20
#define    JP_DCCTRL_DISDC_MASK                                           0x00100000
#define    JP_DCCTRL_SETDC3_SHIFT                                         19
#define    JP_DCCTRL_SETDC3_MASK                                          0x00080000
#define    JP_DCCTRL_SETDC2_SHIFT                                         18
#define    JP_DCCTRL_SETDC2_MASK                                          0x00040000
#define    JP_DCCTRL_SETDC1_SHIFT                                         17
#define    JP_DCCTRL_SETDC1_MASK                                          0x00020000
#define    JP_DCCTRL_SETDC0_SHIFT                                         16
#define    JP_DCCTRL_SETDC0_MASK                                          0x00010000
#define    JP_DCCTRL_DCCOMP_SHIFT                                         0
#define    JP_DCCTRL_DCCOMP_MASK                                          0x0000FFFF

#define JP_CBA_OFFSET                                                     0x00000010
#define JP_CBA_TYPE                                                       UInt32
#define JP_CBA_RESERVED_MASK                                              0x0000000F
#define    JP_CBA_IBA_SHIFT                                               4
#define    JP_CBA_IBA_MASK                                                0xFFFFFFF0

#define JP_NCB_OFFSET                                                     0x00000014
#define JP_NCB_TYPE                                                       UInt32
#define JP_NCB_RESERVED_MASK                                              0x00000000
#define    JP_NCB_NIU_SHIFT                                               0
#define    JP_NCB_NIU_MASK                                                0xFFFFFFFF

#define JP_SDA_OFFSET                                                     0x00000018
#define JP_SDA_TYPE                                                       UInt32
#define JP_SDA_RESERVED_MASK                                              0x0000000F
#define    JP_SDA_SBA_SHIFT                                               4
#define    JP_SDA_SBA_MASK                                                0xFFFFFFF0

#define JP_NSB_OFFSET                                                     0x0000001C
#define JP_NSB_TYPE                                                       UInt32
#define JP_NSB_RESERVED_MASK                                              0x00000000
#define    JP_NSB_NSB_SHIFT                                               0
#define    JP_NSB_NSB_MASK                                                0xFFFFFFFF

#define JP_SBO_OFFSET                                                     0x00000020
#define JP_SBO_TYPE                                                       UInt32
#define JP_SBO_RESERVED_MASK                                              0xFFFFFF80
#define    JP_SBO_BITOFF_SHIFT                                            0
#define    JP_SBO_BITOFF_MASK                                             0x0000007F

#define JP_MOP_OFFSET                                                     0x00000024
#define JP_MOP_TYPE                                                       UInt32
#define JP_MOP_RESERVED_MASK                                              0x00000000
#define    JP_MOP_MOP_SHIFT                                               0
#define    JP_MOP_MOP_MASK                                                0xFFFFFFFF

#define JP_HADDR_OFFSET                                                   0x00000028
#define JP_HADDR_TYPE                                                     UInt32
#define JP_HADDR_RESERVED_MASK                                            0x7FFFFC00
#define    JP_HADDR_RAMACC_SHIFT                                          31
#define    JP_HADDR_RAMACC_MASK                                           0x80000000
#define    JP_HADDR_ADDR_SHIFT                                            0
#define    JP_HADDR_ADDR_MASK                                             0x000003FF

#define JP_HWDATA_OFFSET                                                  0x0000002C
#define JP_HWDATA_TYPE                                                    UInt32
#define JP_HWDATA_RESERVED_MASK                                           0xFFF00000
#define    JP_HWDATA_HCLEN_SHIFT                                          16
#define    JP_HWDATA_HCLEN_MASK                                           0x000F0000
#define    JP_HWDATA_HCODE_SHIFT                                          0
#define    JP_HWDATA_HCODE_MASK                                           0x0000FFFF

#define JP_MADDR_OFFSET                                                   0x00000030
#define JP_MADDR_TYPE                                                     UInt32
#define JP_MADDR_RESERVED_MASK                                            0xFFFFFFC0
#define    JP_MADDR_ADDR_SHIFT                                            0
#define    JP_MADDR_ADDR_MASK                                             0x0000003F

#define JP_MWDATA_OFFSET                                                  0x00000034
#define JP_MWDATA_TYPE                                                    UInt32
#define JP_MWDATA_RESERVED_MASK                                           0xFFFF0000
#define    JP_MWDATA_MAXCCODE_SHIFT                                       0
#define    JP_MWDATA_MAXCCODE_MASK                                        0x0000FFFF

#define JP_OADDR_OFFSET                                                   0x00000038
#define JP_OADDR_TYPE                                                     UInt32
#define JP_OADDR_RESERVED_MASK                                            0xFFFFFFC0
#define    JP_OADDR_ADDR_SHIFT                                            0
#define    JP_OADDR_ADDR_MASK                                             0x0000003F

#define JP_OWDATA_OFFSET                                                  0x0000003C
#define JP_OWDATA_TYPE                                                    UInt32
#define JP_OWDATA_RESERVED_MASK                                           0xFFFF0000
#define    JP_OWDATA_OSET_SHIFT                                           0
#define    JP_OWDATA_OSET_MASK                                            0x0000FFFF

#define JP_QADDR_OFFSET                                                   0x00000040
#define JP_QADDR_TYPE                                                     UInt32
#define JP_QADDR_RESERVED_MASK                                            0x7FFFFF00
#define    JP_QADDR_RAMACC_SHIFT                                          31
#define    JP_QADDR_RAMACC_MASK                                           0x80000000
#define    JP_QADDR_ADDR_SHIFT                                            0
#define    JP_QADDR_ADDR_MASK                                             0x000000FF

#define JP_QWDATA_OFFSET                                                  0x00000044
#define JP_QWDATA_TYPE                                                    UInt32
#define JP_QWDATA_RESERVED_MASK                                           0xFF000000
#define    JP_QWDATA_QUANT_SHIFT                                          0
#define    JP_QWDATA_QUANT_MASK                                           0x00FFFFFF

#define JP_QCTRL_OFFSET                                                   0x00000048
#define JP_QCTRL_TYPE                                                     UInt32
#define JP_QCTRL_RESERVED_MASK                                            0xFFFFFF00
#define    JP_QCTRL_QTAB3_SHIFT                                           6
#define    JP_QCTRL_QTAB3_MASK                                            0x000000C0
#define    JP_QCTRL_QTAB2_SHIFT                                           4
#define    JP_QCTRL_QTAB2_MASK                                            0x00000030
#define    JP_QCTRL_QTAB1_SHIFT                                           2
#define    JP_QCTRL_QTAB1_MASK                                            0x0000000C
#define    JP_QCTRL_QTAB0_SHIFT                                           0
#define    JP_QCTRL_QTAB0_MASK                                            0x00000003

#define JP_C0BA_OFFSET                                                    0x0000004C
#define JP_C0BA_TYPE                                                      UInt32
#define JP_C0BA_RESERVED_MASK                                             0x0000000F
#define    JP_C0BA_ADDR_SHIFT                                             4
#define    JP_C0BA_ADDR_MASK                                              0xFFFFFFF0

#define JP_C1BA_OFFSET                                                    0x00000050
#define JP_C1BA_TYPE                                                      UInt32
#define JP_C1BA_RESERVED_MASK                                             0x0000000F
#define    JP_C1BA_ADDR_SHIFT                                             4
#define    JP_C1BA_ADDR_MASK                                              0xFFFFFFF0

#define JP_C2BA_OFFSET                                                    0x00000054
#define JP_C2BA_TYPE                                                      UInt32
#define JP_C2BA_RESERVED_MASK                                             0x0000000F
#define    JP_C2BA_ADDR_SHIFT                                             4
#define    JP_C2BA_ADDR_MASK                                              0xFFFFFFF0

#define JP_C0S_OFFSET                                                     0x00000058
#define JP_C0S_TYPE                                                       UInt32
#define JP_C0S_RESERVED_MASK                                              0x0000000F
#define    JP_C0S_STRIDE_SHIFT                                            4
#define    JP_C0S_STRIDE_MASK                                             0xFFFFFFF0

#define JP_C1S_OFFSET                                                     0x0000005C
#define JP_C1S_TYPE                                                       UInt32
#define JP_C1S_RESERVED_MASK                                              0x0000000F
#define    JP_C1S_STRIDE_SHIFT                                            4
#define    JP_C1S_STRIDE_MASK                                             0xFFFFFFF0

#define JP_C2S_OFFSET                                                     0x00000060
#define JP_C2S_TYPE                                                       UInt32
#define JP_C2S_RESERVED_MASK                                              0x0000000F
#define    JP_C2S_STRIDE_SHIFT                                            4
#define    JP_C2S_STRIDE_MASK                                             0xFFFFFFF0

#define JP_C0W_OFFSET                                                     0x00000064
#define JP_C0W_TYPE                                                       UInt32
#define JP_C0W_RESERVED_MASK                                              0x00000007
#define    JP_C0W_C0W_WIDTH_SHIFT                                         3
#define    JP_C0W_C0W_WIDTH_MASK                                          0xFFFFFFF8

#define JP_C1W_OFFSET                                                     0x00000068
#define JP_C1W_TYPE                                                       UInt32
#define JP_C1W_RESERVED_MASK                                              0x00000007
#define    JP_C1W_C1W_WIDTH_SHIFT                                         3
#define    JP_C1W_C1W_WIDTH_MASK                                          0xFFFFFFF8

#define JP_C2W_OFFSET                                                     0x0000006C
#define JP_C2W_TYPE                                                       UInt32
#define JP_C2W_RESERVED_MASK                                              0x00000007
#define    JP_C2W_C2W_WIDTH_SHIFT                                         3
#define    JP_C2W_C2W_WIDTH_MASK                                          0xFFFFFFF8

#endif /* __BRCM_RDB_JP_H__ */


