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

#ifndef __BRCM_RDB_DSP_DIGRF_H__
#define __BRCM_RDB_DSP_DIGRF_H__

#define DSP_DIGRF_DRFMODE_OFFSET                                          0x0000E5A0
#define DSP_DIGRF_DRFMODE_TYPE                                            UInt16
#define DSP_DIGRF_DRFMODE_RESERVED_MASK                                   0x00003F80
#define    DSP_DIGRF_DRFMODE_ENTXSD_SHIFT                                 15
#define    DSP_DIGRF_DRFMODE_ENTXSD_MASK                                  0x00008000
#define    DSP_DIGRF_DRFMODE_ENLASTGUARD_SHIFT                            14
#define    DSP_DIGRF_DRFMODE_ENLASTGUARD_MASK                             0x00004000
#define    DSP_DIGRF_DRFMODE_GMSK_GS0_MODE_SHIFT                          6
#define    DSP_DIGRF_DRFMODE_GMSK_GS0_MODE_MASK                           0x00000040
#define    DSP_DIGRF_DRFMODE_SKIP_GUARD_SHIFT                             5
#define    DSP_DIGRF_DRFMODE_SKIP_GUARD_MASK                              0x00000020
#define    DSP_DIGRF_DRFMODE_POLARX_SHIFT                                 4
#define    DSP_DIGRF_DRFMODE_POLARX_MASK                                  0x00000010
#define    DSP_DIGRF_DRFMODE_POLATX_SHIFT                                 3
#define    DSP_DIGRF_DRFMODE_POLATX_MASK                                  0x00000008
#define    DSP_DIGRF_DRFMODE_POLASYS_SHIFT                                2
#define    DSP_DIGRF_DRFMODE_POLASYS_MASK                                 0x00000004
#define    DSP_DIGRF_DRFMODE_RXORDER_SHIFT                                1
#define    DSP_DIGRF_DRFMODE_RXORDER_MASK                                 0x00000002
#define    DSP_DIGRF_DRFMODE_TXMODE_SHIFT                                 0
#define    DSP_DIGRF_DRFMODE_TXMODE_MASK                                  0x00000001

#define DSP_DIGRF_DRFLPM_OFFSET                                           0x0000E5A1
#define DSP_DIGRF_DRFLPM_TYPE                                             UInt16
#define DSP_DIGRF_DRFLPM_RESERVED_MASK                                    0x0000C0C0
#define    DSP_DIGRF_DRFLPM_LPOS_SHIFT                                    8
#define    DSP_DIGRF_DRFLPM_LPOS_MASK                                     0x00003F00
#define    DSP_DIGRF_DRFLPM_LPRE_SHIFT                                    0
#define    DSP_DIGRF_DRFLPM_LPRE_MASK                                     0x0000003F

#define DSP_DIGRF_DRFPREH_OFFSET                                          0x0000E5A2
#define DSP_DIGRF_DRFPREH_TYPE                                            UInt16
#define DSP_DIGRF_DRFPREH_RESERVED_MASK                                   0x00000000
#define    DSP_DIGRF_DRFPREH_DRFPREH_SHIFT                                0
#define    DSP_DIGRF_DRFPREH_DRFPREH_MASK                                 0x0000FFFF

#define DSP_DIGRF_DRFPREL_OFFSET                                          0x0000E5A3
#define DSP_DIGRF_DRFPREL_TYPE                                            UInt16
#define DSP_DIGRF_DRFPREL_RESERVED_MASK                                   0x00000000
#define    DSP_DIGRF_DRFPREL_DRFPREL_SHIFT                                0
#define    DSP_DIGRF_DRFPREL_DRFPREL_MASK                                 0x0000FFFF

#define DSP_DIGRF_DRFPOSH_OFFSET                                          0x0000E5A4
#define DSP_DIGRF_DRFPOSH_TYPE                                            UInt16
#define DSP_DIGRF_DRFPOSH_RESERVED_MASK                                   0x00000000
#define    DSP_DIGRF_DRFPOSH_DRFPOSH_SHIFT                                0
#define    DSP_DIGRF_DRFPOSH_DRFPOSH_MASK                                 0x0000FFFF

#define DSP_DIGRF_DRFPOSL_OFFSET                                          0x0000E5A5
#define DSP_DIGRF_DRFPOSL_TYPE                                            UInt16
#define DSP_DIGRF_DRFPOSL_RESERVED_MASK                                   0x00000000
#define    DSP_DIGRF_DRFPOSL_DRFPOSL_SHIFT                                0
#define    DSP_DIGRF_DRFPOSL_DRFPOSL_MASK                                 0x0000FFFF

#define DSP_DIGRF_DRFPREH1_OFFSET                                         0x0000E5A6
#define DSP_DIGRF_DRFPREH1_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREH1_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREH1_DRFPREH_SHIFT                               0
#define    DSP_DIGRF_DRFPREH1_DRFPREH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPREL1_OFFSET                                         0x0000E5A7
#define DSP_DIGRF_DRFPREL1_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREL1_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREL1_DRFPREL_SHIFT                               0
#define    DSP_DIGRF_DRFPREL1_DRFPREL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSH1_OFFSET                                         0x0000E5A8
#define DSP_DIGRF_DRFPOSH1_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSH1_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSH1_DRFPOSH_SHIFT                               0
#define    DSP_DIGRF_DRFPOSH1_DRFPOSH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSL1_OFFSET                                         0x0000E5A9
#define DSP_DIGRF_DRFPOSL1_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSL1_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSL1_DRFPOSL_SHIFT                               0
#define    DSP_DIGRF_DRFPOSL1_DRFPOSL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPREH2_OFFSET                                         0x0000E5AA
#define DSP_DIGRF_DRFPREH2_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREH2_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREH2_DRFPREH_SHIFT                               0
#define    DSP_DIGRF_DRFPREH2_DRFPREH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPREL2_OFFSET                                         0x0000E5AB
#define DSP_DIGRF_DRFPREL2_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREL2_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREL2_DRFPREL_SHIFT                               0
#define    DSP_DIGRF_DRFPREL2_DRFPREL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSH2_OFFSET                                         0x0000E5AC
#define DSP_DIGRF_DRFPOSH2_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSH2_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSH2_DRFPOSH_SHIFT                               0
#define    DSP_DIGRF_DRFPOSH2_DRFPOSH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSL2_OFFSET                                         0x0000E5AD
#define DSP_DIGRF_DRFPOSL2_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSL2_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSL2_DRFPOSL_SHIFT                               0
#define    DSP_DIGRF_DRFPOSL2_DRFPOSL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPREH3_OFFSET                                         0x0000E5AE
#define DSP_DIGRF_DRFPREH3_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREH3_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREH3_DRFPREH_SHIFT                               0
#define    DSP_DIGRF_DRFPREH3_DRFPREH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPREL3_OFFSET                                         0x0000E5AF
#define DSP_DIGRF_DRFPREL3_TYPE                                           UInt16
#define DSP_DIGRF_DRFPREL3_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPREL3_DRFPREL_SHIFT                               0
#define    DSP_DIGRF_DRFPREL3_DRFPREL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSH3_OFFSET                                         0x0000E5B0
#define DSP_DIGRF_DRFPOSH3_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSH3_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSH3_DRFPOSH_SHIFT                               0
#define    DSP_DIGRF_DRFPOSH3_DRFPOSH_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFPOSL3_OFFSET                                         0x0000E5B1
#define DSP_DIGRF_DRFPOSL3_TYPE                                           UInt16
#define DSP_DIGRF_DRFPOSL3_RESERVED_MASK                                  0x00000000
#define    DSP_DIGRF_DRFPOSL3_DRFPOSL_SHIFT                               0
#define    DSP_DIGRF_DRFPOSL3_DRFPOSL_MASK                                0x0000FFFF

#define DSP_DIGRF_DRFMODE_R_OFFSET                                        0x00000B40
#define DSP_DIGRF_DRFMODE_R_TYPE                                          UInt16
#define DSP_DIGRF_DRFMODE_R_RESERVED_MASK                                 0x00003F80
#define    DSP_DIGRF_DRFMODE_R_ENTXSD_SHIFT                               15
#define    DSP_DIGRF_DRFMODE_R_ENTXSD_MASK                                0x00008000
#define    DSP_DIGRF_DRFMODE_R_ENLASTGUARD_SHIFT                          14
#define    DSP_DIGRF_DRFMODE_R_ENLASTGUARD_MASK                           0x00004000
#define    DSP_DIGRF_DRFMODE_R_GMSK_GS0_MODE_SHIFT                        6
#define    DSP_DIGRF_DRFMODE_R_GMSK_GS0_MODE_MASK                         0x00000040
#define    DSP_DIGRF_DRFMODE_R_SKIP_GUARD_SHIFT                           5
#define    DSP_DIGRF_DRFMODE_R_SKIP_GUARD_MASK                            0x00000020
#define    DSP_DIGRF_DRFMODE_R_POLARX_SHIFT                               4
#define    DSP_DIGRF_DRFMODE_R_POLARX_MASK                                0x00000010
#define    DSP_DIGRF_DRFMODE_R_POLATX_SHIFT                               3
#define    DSP_DIGRF_DRFMODE_R_POLATX_MASK                                0x00000008
#define    DSP_DIGRF_DRFMODE_R_POLASYS_SHIFT                              2
#define    DSP_DIGRF_DRFMODE_R_POLASYS_MASK                               0x00000004
#define    DSP_DIGRF_DRFMODE_R_RXORDER_SHIFT                              1
#define    DSP_DIGRF_DRFMODE_R_RXORDER_MASK                               0x00000002
#define    DSP_DIGRF_DRFMODE_R_TXMODE_SHIFT                               0
#define    DSP_DIGRF_DRFMODE_R_TXMODE_MASK                                0x00000001

#define DSP_DIGRF_DRFLPM_R_OFFSET                                         0x00000B42
#define DSP_DIGRF_DRFLPM_R_TYPE                                           UInt16
#define DSP_DIGRF_DRFLPM_R_RESERVED_MASK                                  0x0000C0C0
#define    DSP_DIGRF_DRFLPM_R_LPOS_SHIFT                                  8
#define    DSP_DIGRF_DRFLPM_R_LPOS_MASK                                   0x00003F00
#define    DSP_DIGRF_DRFLPM_R_LPRE_SHIFT                                  0
#define    DSP_DIGRF_DRFLPM_R_LPRE_MASK                                   0x0000003F

#define DSP_DIGRF_DRFPREH_R_OFFSET                                        0x00000B44
#define DSP_DIGRF_DRFPREH_R_TYPE                                          UInt16
#define DSP_DIGRF_DRFPREH_R_RESERVED_MASK                                 0x00000000
#define    DSP_DIGRF_DRFPREH_R_DRFPREH_SHIFT                              0
#define    DSP_DIGRF_DRFPREH_R_DRFPREH_MASK                               0x0000FFFF

#define DSP_DIGRF_DRFPREL_R_OFFSET                                        0x00000B46
#define DSP_DIGRF_DRFPREL_R_TYPE                                          UInt16
#define DSP_DIGRF_DRFPREL_R_RESERVED_MASK                                 0x00000000
#define    DSP_DIGRF_DRFPREL_R_DRFPREL_SHIFT                              0
#define    DSP_DIGRF_DRFPREL_R_DRFPREL_MASK                               0x0000FFFF

#define DSP_DIGRF_DRFPOSH_R_OFFSET                                        0x00000B48
#define DSP_DIGRF_DRFPOSH_R_TYPE                                          UInt16
#define DSP_DIGRF_DRFPOSH_R_RESERVED_MASK                                 0x00000000
#define    DSP_DIGRF_DRFPOSH_R_DRFPOSH_SHIFT                              0
#define    DSP_DIGRF_DRFPOSH_R_DRFPOSH_MASK                               0x0000FFFF

#define DSP_DIGRF_DRFPOSL_R_OFFSET                                        0x00000B4A
#define DSP_DIGRF_DRFPOSL_R_TYPE                                          UInt16
#define DSP_DIGRF_DRFPOSL_R_RESERVED_MASK                                 0x00000000
#define    DSP_DIGRF_DRFPOSL_R_DRFPOSL_SHIFT                              0
#define    DSP_DIGRF_DRFPOSL_R_DRFPOSL_MASK                               0x0000FFFF

#define DSP_DIGRF_DRFPREH1_R_OFFSET                                       0x00000B4C
#define DSP_DIGRF_DRFPREH1_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREH1_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREH1_R_DRFPREH_SHIFT                             0
#define    DSP_DIGRF_DRFPREH1_R_DRFPREH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPREL1_R_OFFSET                                       0x00000B4E
#define DSP_DIGRF_DRFPREL1_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREL1_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREL1_R_DRFPREL_SHIFT                             0
#define    DSP_DIGRF_DRFPREL1_R_DRFPREL_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSH1_R_OFFSET                                       0x00000B50
#define DSP_DIGRF_DRFPOSH1_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSH1_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSH1_R_DRFPOSH_SHIFT                             0
#define    DSP_DIGRF_DRFPOSH1_R_DRFPOSH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSL1_R_OFFSET                                       0x00000B52
#define DSP_DIGRF_DRFPOSL1_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSL1_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSL1_R_DRFPOSL_SHIFT                             0
#define    DSP_DIGRF_DRFPOSL1_R_DRFPOSL_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPREH2_R_OFFSET                                       0x00000B54
#define DSP_DIGRF_DRFPREH2_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREH2_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREH2_R_DRFPREH_SHIFT                             0
#define    DSP_DIGRF_DRFPREH2_R_DRFPREH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPREL2_R_OFFSET                                       0x00000B56
#define DSP_DIGRF_DRFPREL2_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREL2_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREL2_R_DRFPREL_SHIFT                             0
#define    DSP_DIGRF_DRFPREL2_R_DRFPREL_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSH2_R_OFFSET                                       0x00000B58
#define DSP_DIGRF_DRFPOSH2_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSH2_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSH2_R_DRFPOSH_SHIFT                             0
#define    DSP_DIGRF_DRFPOSH2_R_DRFPOSH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSL2_R_OFFSET                                       0x00000B5A
#define DSP_DIGRF_DRFPOSL2_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSL2_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSL2_R_DRFPOSL_SHIFT                             0
#define    DSP_DIGRF_DRFPOSL2_R_DRFPOSL_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPREH3_R_OFFSET                                       0x00000B5C
#define DSP_DIGRF_DRFPREH3_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREH3_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREH3_R_DRFPREH_SHIFT                             0
#define    DSP_DIGRF_DRFPREH3_R_DRFPREH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPREL3_R_OFFSET                                       0x00000B5E
#define DSP_DIGRF_DRFPREL3_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPREL3_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPREL3_R_DRFPREL_SHIFT                             0
#define    DSP_DIGRF_DRFPREL3_R_DRFPREL_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSH3_R_OFFSET                                       0x00000B60
#define DSP_DIGRF_DRFPOSH3_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSH3_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSH3_R_DRFPOSH_SHIFT                             0
#define    DSP_DIGRF_DRFPOSH3_R_DRFPOSH_MASK                              0x0000FFFF

#define DSP_DIGRF_DRFPOSL3_R_OFFSET                                       0x00000B62
#define DSP_DIGRF_DRFPOSL3_R_TYPE                                         UInt16
#define DSP_DIGRF_DRFPOSL3_R_RESERVED_MASK                                0x00000000
#define    DSP_DIGRF_DRFPOSL3_R_DRFPOSL_SHIFT                             0
#define    DSP_DIGRF_DRFPOSL3_R_DRFPOSL_MASK                              0x0000FFFF

#endif /* __BRCM_RDB_DSP_DIGRF_H__ */


