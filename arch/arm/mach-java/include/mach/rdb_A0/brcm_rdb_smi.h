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

#ifndef __BRCM_RDB_SMI_H__
#define __BRCM_RDB_SMI_H__

#define SMI_CS_OFFSET                                                     0x00000000
#define SMI_CS_TYPE                                                       UInt32
#define SMI_CS_RESERVED_MASK                                              0x00FF0000
#define    SMI_CS_RXF_SHIFT                                               31
#define    SMI_CS_RXF_MASK                                                0x80000000
#define    SMI_CS_TXE_SHIFT                                               30
#define    SMI_CS_TXE_MASK                                                0x40000000
#define    SMI_CS_RXD_SHIFT                                               29
#define    SMI_CS_RXD_MASK                                                0x20000000
#define    SMI_CS_TXD_SHIFT                                               28
#define    SMI_CS_TXD_MASK                                                0x10000000
#define    SMI_CS_RXR_SHIFT                                               27
#define    SMI_CS_RXR_MASK                                                0x08000000
#define    SMI_CS_TXW_SHIFT                                               26
#define    SMI_CS_TXW_MASK                                                0x04000000
#define    SMI_CS_AFERR_SHIFT                                             25
#define    SMI_CS_AFERR_MASK                                              0x02000000
#define    SMI_CS_PRDY_SHIFT                                              24
#define    SMI_CS_PRDY_MASK                                               0x01000000
#define    SMI_CS_EDREQ_SHIFT                                             15
#define    SMI_CS_EDREQ_MASK                                              0x00008000
#define    SMI_CS_PXLDAT_SHIFT                                            14
#define    SMI_CS_PXLDAT_MASK                                             0x00004000
#define    SMI_CS_SETERR_SHIFT                                            13
#define    SMI_CS_SETERR_MASK                                             0x00002000
#define    SMI_CS_PVMODE_SHIFT                                            12
#define    SMI_CS_PVMODE_MASK                                             0x00001000
#define    SMI_CS_INTR_SHIFT                                              11
#define    SMI_CS_INTR_MASK                                               0x00000800
#define    SMI_CS_INTT_SHIFT                                              10
#define    SMI_CS_INTT_MASK                                               0x00000400
#define    SMI_CS_INTD_SHIFT                                              9
#define    SMI_CS_INTD_MASK                                               0x00000200
#define    SMI_CS_TEEN_SHIFT                                              8
#define    SMI_CS_TEEN_MASK                                               0x00000100
#define    SMI_CS_PAD_SHIFT                                               6
#define    SMI_CS_PAD_MASK                                                0x000000C0
#define    SMI_CS_WRITE_SHIFT                                             5
#define    SMI_CS_WRITE_MASK                                              0x00000020
#define    SMI_CS_CLEAR_SHIFT                                             4
#define    SMI_CS_CLEAR_MASK                                              0x00000010
#define    SMI_CS_START_SHIFT                                             3
#define    SMI_CS_START_MASK                                              0x00000008
#define    SMI_CS_ACTIVE_SHIFT                                            2
#define    SMI_CS_ACTIVE_MASK                                             0x00000004
#define    SMI_CS_DONE_SHIFT                                              1
#define    SMI_CS_DONE_MASK                                               0x00000002
#define    SMI_CS_ENABLE_SHIFT                                            0
#define    SMI_CS_ENABLE_MASK                                             0x00000001

#define SMI_L_OFFSET                                                      0x00000004
#define SMI_L_TYPE                                                        UInt32
#define SMI_L_RESERVED_MASK                                               0x00000000
#define    SMI_L_L_SHIFT                                                  0
#define    SMI_L_L_MASK                                                   0xFFFFFFFF

#define SMI_A_OFFSET                                                      0x00000008
#define SMI_A_TYPE                                                        UInt32
#define SMI_A_RESERVED_MASK                                               0xFFFFFCC0
#define    SMI_A_DEVICE_SHIFT                                             8
#define    SMI_A_DEVICE_MASK                                              0x00000300
#define    SMI_A_ADDR_SHIFT                                               0
#define    SMI_A_ADDR_MASK                                                0x0000003F

#define SMI_D_OFFSET                                                      0x0000000C
#define SMI_D_TYPE                                                        UInt32
#define SMI_D_RESERVED_MASK                                               0x00000000
#define    SMI_D_D_SHIFT                                                  0
#define    SMI_D_D_MASK                                                   0xFFFFFFFF

#define SMI_DSR0_OFFSET                                                   0x00000010
#define SMI_DSR0_TYPE                                                     UInt32
#define SMI_DSR0_RESERVED_MASK                                            0x00000000
#define    SMI_DSR0_RWIDTH_SHIFT                                          30
#define    SMI_DSR0_RWIDTH_MASK                                           0xC0000000
#define    SMI_DSR0_RSETUP_SHIFT                                          24
#define    SMI_DSR0_RSETUP_MASK                                           0x3F000000
#define    SMI_DSR0_MODE68_SHIFT                                          23
#define    SMI_DSR0_MODE68_MASK                                           0x00800000
#define    SMI_DSR0_FSETUP_SHIFT                                          22
#define    SMI_DSR0_FSETUP_MASK                                           0x00400000
#define    SMI_DSR0_RHOLD_SHIFT                                           16
#define    SMI_DSR0_RHOLD_MASK                                            0x003F0000
#define    SMI_DSR0_RPACEALL_SHIFT                                        15
#define    SMI_DSR0_RPACEALL_MASK                                         0x00008000
#define    SMI_DSR0_RPACE_SHIFT                                           8
#define    SMI_DSR0_RPACE_MASK                                            0x00007F00
#define    SMI_DSR0_RDREQ_SHIFT                                           7
#define    SMI_DSR0_RDREQ_MASK                                            0x00000080
#define    SMI_DSR0_RSTROBE_SHIFT                                         0
#define    SMI_DSR0_RSTROBE_MASK                                          0x0000007F

#define SMI_DSW0_OFFSET                                                   0x00000014
#define SMI_DSW0_TYPE                                                     UInt32
#define SMI_DSW0_RESERVED_MASK                                            0x00000000
#define    SMI_DSW0_WWIDTH_SHIFT                                          30
#define    SMI_DSW0_WWIDTH_MASK                                           0xC0000000
#define    SMI_DSW0_WSETUP_SHIFT                                          24
#define    SMI_DSW0_WSETUP_MASK                                           0x3F000000
#define    SMI_DSW0_WFORMAT_SHIFT                                         23
#define    SMI_DSW0_WFORMAT_MASK                                          0x00800000
#define    SMI_DSW0_WSWAP_SHIFT                                           22
#define    SMI_DSW0_WSWAP_MASK                                            0x00400000
#define    SMI_DSW0_WHOLD_SHIFT                                           16
#define    SMI_DSW0_WHOLD_MASK                                            0x003F0000
#define    SMI_DSW0_WPACEALL_SHIFT                                        15
#define    SMI_DSW0_WPACEALL_MASK                                         0x00008000
#define    SMI_DSW0_WPACE_SHIFT                                           8
#define    SMI_DSW0_WPACE_MASK                                            0x00007F00
#define    SMI_DSW0_WDREQ_SHIFT                                           7
#define    SMI_DSW0_WDREQ_MASK                                            0x00000080
#define    SMI_DSW0_WSTROBE_SHIFT                                         0
#define    SMI_DSW0_WSTROBE_MASK                                          0x0000007F

#define SMI_DSR1_OFFSET                                                   0x00000018
#define SMI_DSR1_TYPE                                                     UInt32
#define SMI_DSR1_RESERVED_MASK                                            0x00000000
#define    SMI_DSR1_RWIDTH_SHIFT                                          30
#define    SMI_DSR1_RWIDTH_MASK                                           0xC0000000
#define    SMI_DSR1_RSETUP_SHIFT                                          24
#define    SMI_DSR1_RSETUP_MASK                                           0x3F000000
#define    SMI_DSR1_MODE68_SHIFT                                          23
#define    SMI_DSR1_MODE68_MASK                                           0x00800000
#define    SMI_DSR1_FSETUP_SHIFT                                          22
#define    SMI_DSR1_FSETUP_MASK                                           0x00400000
#define    SMI_DSR1_RHOLD_SHIFT                                           16
#define    SMI_DSR1_RHOLD_MASK                                            0x003F0000
#define    SMI_DSR1_RPACEALL_SHIFT                                        15
#define    SMI_DSR1_RPACEALL_MASK                                         0x00008000
#define    SMI_DSR1_RPACE_SHIFT                                           8
#define    SMI_DSR1_RPACE_MASK                                            0x00007F00
#define    SMI_DSR1_RDREQ_SHIFT                                           7
#define    SMI_DSR1_RDREQ_MASK                                            0x00000080
#define    SMI_DSR1_RSTROBE_SHIFT                                         0
#define    SMI_DSR1_RSTROBE_MASK                                          0x0000007F

#define SMI_DSW1_OFFSET                                                   0x0000001C
#define SMI_DSW1_TYPE                                                     UInt32
#define SMI_DSW1_RESERVED_MASK                                            0x00000000
#define    SMI_DSW1_WWIDTH_SHIFT                                          30
#define    SMI_DSW1_WWIDTH_MASK                                           0xC0000000
#define    SMI_DSW1_WSETUP_SHIFT                                          24
#define    SMI_DSW1_WSETUP_MASK                                           0x3F000000
#define    SMI_DSW1_WFORMAT_SHIFT                                         23
#define    SMI_DSW1_WFORMAT_MASK                                          0x00800000
#define    SMI_DSW1_WSWAP_SHIFT                                           22
#define    SMI_DSW1_WSWAP_MASK                                            0x00400000
#define    SMI_DSW1_WHOLD_SHIFT                                           16
#define    SMI_DSW1_WHOLD_MASK                                            0x003F0000
#define    SMI_DSW1_WPACEALL_SHIFT                                        15
#define    SMI_DSW1_WPACEALL_MASK                                         0x00008000
#define    SMI_DSW1_WPACE_SHIFT                                           8
#define    SMI_DSW1_WPACE_MASK                                            0x00007F00
#define    SMI_DSW1_WDREQ_SHIFT                                           7
#define    SMI_DSW1_WDREQ_MASK                                            0x00000080
#define    SMI_DSW1_WSTROBE_SHIFT                                         0
#define    SMI_DSW1_WSTROBE_MASK                                          0x0000007F

#define SMI_DSR2_OFFSET                                                   0x00000020
#define SMI_DSR2_TYPE                                                     UInt32
#define SMI_DSR2_RESERVED_MASK                                            0x00000000
#define    SMI_DSR2_RWIDTH_SHIFT                                          30
#define    SMI_DSR2_RWIDTH_MASK                                           0xC0000000
#define    SMI_DSR2_RSETUP_SHIFT                                          24
#define    SMI_DSR2_RSETUP_MASK                                           0x3F000000
#define    SMI_DSR2_MODE68_SHIFT                                          23
#define    SMI_DSR2_MODE68_MASK                                           0x00800000
#define    SMI_DSR2_FSETUP_SHIFT                                          22
#define    SMI_DSR2_FSETUP_MASK                                           0x00400000
#define    SMI_DSR2_RHOLD_SHIFT                                           16
#define    SMI_DSR2_RHOLD_MASK                                            0x003F0000
#define    SMI_DSR2_RPACEALL_SHIFT                                        15
#define    SMI_DSR2_RPACEALL_MASK                                         0x00008000
#define    SMI_DSR2_RPACE_SHIFT                                           8
#define    SMI_DSR2_RPACE_MASK                                            0x00007F00
#define    SMI_DSR2_RDREQ_SHIFT                                           7
#define    SMI_DSR2_RDREQ_MASK                                            0x00000080
#define    SMI_DSR2_RSTROBE_SHIFT                                         0
#define    SMI_DSR2_RSTROBE_MASK                                          0x0000007F

#define SMI_DSW2_OFFSET                                                   0x00000024
#define SMI_DSW2_TYPE                                                     UInt32
#define SMI_DSW2_RESERVED_MASK                                            0x00000000
#define    SMI_DSW2_WWIDTH_SHIFT                                          30
#define    SMI_DSW2_WWIDTH_MASK                                           0xC0000000
#define    SMI_DSW2_WSETUP_SHIFT                                          24
#define    SMI_DSW2_WSETUP_MASK                                           0x3F000000
#define    SMI_DSW2_WFORMAT_SHIFT                                         23
#define    SMI_DSW2_WFORMAT_MASK                                          0x00800000
#define    SMI_DSW2_WSWAP_SHIFT                                           22
#define    SMI_DSW2_WSWAP_MASK                                            0x00400000
#define    SMI_DSW2_WHOLD_SHIFT                                           16
#define    SMI_DSW2_WHOLD_MASK                                            0x003F0000
#define    SMI_DSW2_WPACEALL_SHIFT                                        15
#define    SMI_DSW2_WPACEALL_MASK                                         0x00008000
#define    SMI_DSW2_WPACE_SHIFT                                           8
#define    SMI_DSW2_WPACE_MASK                                            0x00007F00
#define    SMI_DSW2_WDREQ_SHIFT                                           7
#define    SMI_DSW2_WDREQ_MASK                                            0x00000080
#define    SMI_DSW2_WSTROBE_SHIFT                                         0
#define    SMI_DSW2_WSTROBE_MASK                                          0x0000007F

#define SMI_DSR3_OFFSET                                                   0x00000028
#define SMI_DSR3_TYPE                                                     UInt32
#define SMI_DSR3_RESERVED_MASK                                            0x00000000
#define    SMI_DSR3_RWIDTH_SHIFT                                          30
#define    SMI_DSR3_RWIDTH_MASK                                           0xC0000000
#define    SMI_DSR3_RSETUP_SHIFT                                          24
#define    SMI_DSR3_RSETUP_MASK                                           0x3F000000
#define    SMI_DSR3_MODE68_SHIFT                                          23
#define    SMI_DSR3_MODE68_MASK                                           0x00800000
#define    SMI_DSR3_FSETUP_SHIFT                                          22
#define    SMI_DSR3_FSETUP_MASK                                           0x00400000
#define    SMI_DSR3_RHOLD_SHIFT                                           16
#define    SMI_DSR3_RHOLD_MASK                                            0x003F0000
#define    SMI_DSR3_RPACEALL_SHIFT                                        15
#define    SMI_DSR3_RPACEALL_MASK                                         0x00008000
#define    SMI_DSR3_RPACE_SHIFT                                           8
#define    SMI_DSR3_RPACE_MASK                                            0x00007F00
#define    SMI_DSR3_RDREQ_SHIFT                                           7
#define    SMI_DSR3_RDREQ_MASK                                            0x00000080
#define    SMI_DSR3_RSTROBE_SHIFT                                         0
#define    SMI_DSR3_RSTROBE_MASK                                          0x0000007F

#define SMI_DSW3_OFFSET                                                   0x0000002C
#define SMI_DSW3_TYPE                                                     UInt32
#define SMI_DSW3_RESERVED_MASK                                            0x00000000
#define    SMI_DSW3_WWIDTH_SHIFT                                          30
#define    SMI_DSW3_WWIDTH_MASK                                           0xC0000000
#define    SMI_DSW3_WSETUP_SHIFT                                          24
#define    SMI_DSW3_WSETUP_MASK                                           0x3F000000
#define    SMI_DSW3_WFORMAT_SHIFT                                         23
#define    SMI_DSW3_WFORMAT_MASK                                          0x00800000
#define    SMI_DSW3_WSWAP_SHIFT                                           22
#define    SMI_DSW3_WSWAP_MASK                                            0x00400000
#define    SMI_DSW3_WHOLD_SHIFT                                           16
#define    SMI_DSW3_WHOLD_MASK                                            0x003F0000
#define    SMI_DSW3_WPACEALL_SHIFT                                        15
#define    SMI_DSW3_WPACEALL_MASK                                         0x00008000
#define    SMI_DSW3_WPACE_SHIFT                                           8
#define    SMI_DSW3_WPACE_MASK                                            0x00007F00
#define    SMI_DSW3_WDREQ_SHIFT                                           7
#define    SMI_DSW3_WDREQ_MASK                                            0x00000080
#define    SMI_DSW3_WSTROBE_SHIFT                                         0
#define    SMI_DSW3_WSTROBE_MASK                                          0x0000007F

#define SMI_DC_OFFSET                                                     0x00000030
#define SMI_DC_TYPE                                                       UInt32
#define SMI_DC_RESERVED_MASK                                              0xEE000000
#define    SMI_DC_DMAEN_SHIFT                                             28
#define    SMI_DC_DMAEN_MASK                                              0x10000000
#define    SMI_DC_DMAP_SHIFT                                              24
#define    SMI_DC_DMAP_MASK                                               0x01000000
#define    SMI_DC_PANICR_SHIFT                                            18
#define    SMI_DC_PANICR_MASK                                             0x00FC0000
#define    SMI_DC_PANICW_SHIFT                                            12
#define    SMI_DC_PANICW_MASK                                             0x0003F000
#define    SMI_DC_REQR_SHIFT                                              6
#define    SMI_DC_REQR_MASK                                               0x00000FC0
#define    SMI_DC_REQW_SHIFT                                              0
#define    SMI_DC_REQW_MASK                                               0x0000003F

#define SMI_DCS_OFFSET                                                    0x00000034
#define SMI_DCS_TYPE                                                      UInt32
#define SMI_DCS_RESERVED_MASK                                             0xFFFFFFF0
#define    SMI_DCS_WRITE_SHIFT                                            3
#define    SMI_DCS_WRITE_MASK                                             0x00000008
#define    SMI_DCS_DONE_SHIFT                                             2
#define    SMI_DCS_DONE_MASK                                              0x00000004
#define    SMI_DCS_START_SHIFT                                            1
#define    SMI_DCS_START_MASK                                             0x00000002
#define    SMI_DCS_EANBLE_SHIFT                                           0
#define    SMI_DCS_EANBLE_MASK                                            0x00000001

#define SMI_DA_OFFSET                                                     0x00000038
#define SMI_DA_TYPE                                                       UInt32
#define SMI_DA_RESERVED_MASK                                              0xFFFFFCC0
#define    SMI_DA_WRITE_SHIFT                                             8
#define    SMI_DA_WRITE_MASK                                              0x00000300
#define    SMI_DA_ADDR_SHIFT                                              0
#define    SMI_DA_ADDR_MASK                                               0x0000003F

#define SMI_DD_OFFSET                                                     0x0000003C
#define SMI_DD_TYPE                                                       UInt32
#define SMI_DD_RESERVED_MASK                                              0xFFFC0000
#define    SMI_DD_DD_SHIFT                                                0
#define    SMI_DD_DD_MASK                                                 0x0003FFFF

#define SMI_FD_OFFSET                                                     0x00000040
#define SMI_FD_TYPE                                                       UInt32
#define SMI_FD_RESERVED_MASK                                              0xFFFFE040
#define    SMI_FD_FLVL_SHIFT                                              7
#define    SMI_FD_FLVL_MASK                                               0x00001F80
#define    SMI_FD_FCNT_SHIFT                                              0
#define    SMI_FD_FCNT_MASK                                               0x0000003F

#endif /* __BRCM_RDB_SMI_H__ */


