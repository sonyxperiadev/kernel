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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PKA_H__
#define __BRCM_RDB_PKA_H__

#define PKA_CTRL_OFFSET                                                   0x00000000
#define PKA_CTRL_TYPE                                                     UInt32
#define PKA_CTRL_RESERVED_MASK                                            0x0000C870
#define    PKA_CTRL_UNKNOP_SHIFT                                          31
#define    PKA_CTRL_UNKNOP_MASK                                           0x80000000
#define    PKA_CTRL_INVSRC0_SHIFT                                         30
#define    PKA_CTRL_INVSRC0_MASK                                          0x40000000
#define    PKA_CTRL_INVSRC1_SHIFT                                         29
#define    PKA_CTRL_INVSRC1_MASK                                          0x20000000
#define    PKA_CTRL_INVSRC2_SHIFT                                         28
#define    PKA_CTRL_INVSRC2_MASK                                          0x10000000
#define    PKA_CTRL_CAMFULL_SHIFT                                         27
#define    PKA_CTRL_CAMFULL_MASK                                          0x08000000
#define    PKA_CTRL_DIV0_SHIFT                                            26
#define    PKA_CTRL_DIV0_MASK                                             0x04000000
#define    PKA_CTRL_OPQOF_SHIFT                                           25
#define    PKA_CTRL_OPQOF_MASK                                            0x02000000
#define    PKA_CTRL_OPCIDX_SHIFT                                          20
#define    PKA_CTRL_OPCIDX_MASK                                           0x01F00000
#define    PKA_CTRL_ESC_SHIFT                                             16
#define    PKA_CTRL_ESC_MASK                                              0x000F0000
#define    PKA_CTRL_ENDSEL_SHIFT                                          13
#define    PKA_CTRL_ENDSEL_MASK                                           0x00002000
#define    PKA_CTRL_MEMCLR_SHIFT                                          12
#define    PKA_CTRL_MEMCLR_MASK                                           0x00001000
#define    PKA_CTRL_OPQFULL_SHIFT                                         10
#define    PKA_CTRL_OPQFULL_MASK                                          0x00000400
#define    PKA_CTRL_PRIM_SEL_FAIL_SHIFT                                   9
#define    PKA_CTRL_PRIM_SEL_FAIL_MASK                                    0x00000200
#define    PKA_CTRL_LCOPC_SHIFT                                           8
#define    PKA_CTRL_LCOPC_MASK                                            0x00000100
#define    PKA_CTRL_PKARST_SHIFT                                          7
#define    PKA_CTRL_PKARST_MASK                                           0x00000080
#define    PKA_CTRL_PKACMDERR_SHIFT                                       3
#define    PKA_CTRL_PKACMDERR_MASK                                        0x00000008
#define    PKA_CTRL_PKABUSY_SHIFT                                         2
#define    PKA_CTRL_PKABUSY_MASK                                          0x00000004
#define    PKA_CTRL_PKACMDDONE_SHIFT                                      1
#define    PKA_CTRL_PKACMDDONE_MASK                                       0x00000002
#define    PKA_CTRL_PKAEN_SHIFT                                           0
#define    PKA_CTRL_PKAEN_MASK                                            0x00000001

#define PKA_DIN_OFFSET                                                    0x00000004
#define PKA_DIN_TYPE                                                      UInt32
#define PKA_DIN_RESERVED_MASK                                             0x00000000
#define    PKA_DIN_PKAIDAT_SHIFT                                          0
#define    PKA_DIN_PKAIDAT_MASK                                           0xFFFFFFFF

#define PKA_DOUT_OFFSET                                                   0x00000008
#define PKA_DOUT_TYPE                                                     UInt32
#define PKA_DOUT_RESERVED_MASK                                            0x00000000
#define    PKA_DOUT_PKAODAT_SHIFT                                         0
#define    PKA_DOUT_PKAODAT_MASK                                          0xFFFFFFFF

#define PKA_ACC_OFFSET                                                    0x0000000C
#define PKA_ACC_TYPE                                                      UInt32
#define PKA_ACC_RESERVED_MASK                                             0x7FFF7FFF
#define    PKA_ACC_PKALCK_SHIFT                                           31
#define    PKA_ACC_PKALCK_MASK                                            0x80000000
#define    PKA_ACC_LCK_SHIFT                                              15
#define    PKA_ACC_LCK_MASK                                               0x00008000

#define PKA_LFSR_OFFSET                                                   0x00000010
#define PKA_LFSR_TYPE                                                     UInt32
#define PKA_LFSR_RESERVED_MASK                                            0x00000000
#define    PKA_LFSR_RNDSEED_SHIFT                                         0
#define    PKA_LFSR_RNDSEED_MASK                                          0xFFFFFFFF

#endif /* __BRCM_RDB_PKA_H__ */


