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

#ifndef __BRCM_RDB_LAYER_1_INT_CONT_H__
#define __BRCM_RDB_LAYER_1_INT_CONT_H__

#define LAYER_1_INT_CONT_ISTAT_OFFSET                                     0x00000000
#define LAYER_1_INT_CONT_ISTAT_TYPE                                       UInt32
#define LAYER_1_INT_CONT_ISTAT_RESERVED_MASK                              0x00000000
#define    LAYER_1_INT_CONT_ISTAT_ISTAT_SHIFT                             0
#define    LAYER_1_INT_CONT_ISTAT_ISTAT_MASK                              0xFFFFFFFF

#define LAYER_1_INT_CONT_IRSTAT_OFFSET                                    0x00000004
#define LAYER_1_INT_CONT_IRSTAT_TYPE                                      UInt32
#define LAYER_1_INT_CONT_IRSTAT_RESERVED_MASK                             0x00000000
#define    LAYER_1_INT_CONT_IRSTAT_IRSTAT_SHIFT                           0
#define    LAYER_1_INT_CONT_IRSTAT_IRSTAT_MASK                            0xFFFFFFFF

#define LAYER_1_INT_CONT_IENSET_OFFSET                                    0x00000008
#define LAYER_1_INT_CONT_IENSET_TYPE                                      UInt32
#define LAYER_1_INT_CONT_IENSET_RESERVED_MASK                             0x00000000
#define    LAYER_1_INT_CONT_IENSET_IENSET_SHIFT                           0
#define    LAYER_1_INT_CONT_IENSET_IENSET_MASK                            0xFFFFFFFF

#define LAYER_1_INT_CONT_IENCLR_OFFSET                                    0x0000000C
#define LAYER_1_INT_CONT_IENCLR_TYPE                                      UInt32
#define LAYER_1_INT_CONT_IENCLR_RESERVED_MASK                             0x00000000
#define    LAYER_1_INT_CONT_IENCLR_IENCLR_SHIFT                           0
#define    LAYER_1_INT_CONT_IENCLR_IENCLR_MASK                            0xFFFFFFFF

#define LAYER_1_INT_CONT_SOFTINT_OFFSET                                   0x00000010
#define LAYER_1_INT_CONT_SOFTINT_TYPE                                     UInt32
#define LAYER_1_INT_CONT_SOFTINT_RESERVED_MASK                            0xFFFFFFFC
#define    LAYER_1_INT_CONT_SOFTINT_SOFTINT_SHIFT                         0
#define    LAYER_1_INT_CONT_SOFTINT_SOFTINT_MASK                          0x00000003

#define LAYER_1_INT_CONT_IRESET_OFFSET                                    0x00000014
#define LAYER_1_INT_CONT_IRESET_TYPE                                      UInt32
#define LAYER_1_INT_CONT_IRESET_RESERVED_MASK                             0x00000000
#define    LAYER_1_INT_CONT_IRESET_IRESET_SHIFT                           0
#define    LAYER_1_INT_CONT_IRESET_IRESET_MASK                            0xFFFFFFFF

#define LAYER_1_INT_CONT_ISTAT2_OFFSET                                    0x00000020
#define LAYER_1_INT_CONT_ISTAT2_TYPE                                      UInt32
#define LAYER_1_INT_CONT_ISTAT2_RESERVED_MASK                             0xFFFF0000
#define    LAYER_1_INT_CONT_ISTAT2_ISTAT2_SHIFT                           0
#define    LAYER_1_INT_CONT_ISTAT2_ISTAT2_MASK                            0x0000FFFF

#define LAYER_1_INT_CONT_IRSTAT2_OFFSET                                   0x00000024
#define LAYER_1_INT_CONT_IRSTAT2_TYPE                                     UInt32
#define LAYER_1_INT_CONT_IRSTAT2_RESERVED_MASK                            0xFFFF0000
#define    LAYER_1_INT_CONT_IRSTAT2_IRSTAT2_SHIFT                         0
#define    LAYER_1_INT_CONT_IRSTAT2_IRSTAT2_MASK                          0x0000FFFF

#define LAYER_1_INT_CONT_IENSET2_OFFSET                                   0x00000028
#define LAYER_1_INT_CONT_IENSET2_TYPE                                     UInt32
#define LAYER_1_INT_CONT_IENSET2_RESERVED_MASK                            0xFFFF0000
#define    LAYER_1_INT_CONT_IENSET2_IENSET2_SHIFT                         0
#define    LAYER_1_INT_CONT_IENSET2_IENSET2_MASK                          0x0000FFFF

#define LAYER_1_INT_CONT_IENCLR2_OFFSET                                   0x0000002C
#define LAYER_1_INT_CONT_IENCLR2_TYPE                                     UInt32
#define LAYER_1_INT_CONT_IENCLR2_RESERVED_MASK                            0xFFFF0000
#define    LAYER_1_INT_CONT_IENCLR2_IENCLR2_SHIFT                         0
#define    LAYER_1_INT_CONT_IENCLR2_IENCLR2_MASK                          0x0000FFFF

#define LAYER_1_INT_CONT_IRESET2_OFFSET                                   0x00000034
#define LAYER_1_INT_CONT_IRESET2_TYPE                                     UInt32
#define LAYER_1_INT_CONT_IRESET2_RESERVED_MASK                            0xFFFF0000
#define    LAYER_1_INT_CONT_IRESET2_IRESET2_SHIFT                         0
#define    LAYER_1_INT_CONT_IRESET2_IRESET2_MASK                          0x0000FFFF

#endif /* __BRCM_RDB_LAYER_1_INT_CONT_H__ */


