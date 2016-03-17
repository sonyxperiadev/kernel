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

#ifndef __BRCM_RDB_SYSSW_H__
#define __BRCM_RDB_SYSSW_H__

#define SYSSW_QOSTIDE0_OFFSET                                             0x00000400
#define SYSSW_QOSTIDE0_TYPE                                               UInt32
#define SYSSW_QOSTIDE0_RESERVED_MASK                                      0xFFFFFFC0
#define    SYSSW_QOSTIDE0_QOS_TIDE0_SHIFT                                 0
#define    SYSSW_QOSTIDE0_QOS_TIDE0_MASK                                  0x0000003F

#define SYSSW_QOSACC0_OFFSET                                              0x00000404
#define SYSSW_QOSACC0_TYPE                                                UInt32
#define SYSSW_QOSACC0_RESERVED_MASK                                       0xFFFFFFF0
#define    SYSSW_QOSACC0_QOS_ACC0_SHIFT                                   0
#define    SYSSW_QOSACC0_QOS_ACC0_MASK                                    0x0000000F

#define SYSSW_ARARB0_OFFSET                                               0x00000408
#define SYSSW_ARARB0_TYPE                                                 UInt32
#define SYSSW_ARARB0_RESERVED_MASK                                        0x00000000
#define    SYSSW_ARARB0_AR_ARB0_SHIFT                                     0
#define    SYSSW_ARARB0_AR_ARB0_MASK                                      0xFFFFFFFF

#define SYSSW_AWARB0_OFFSET                                               0x0000040C
#define SYSSW_AWARB0_TYPE                                                 UInt32
#define SYSSW_AWARB0_RESERVED_MASK                                        0x00000000
#define    SYSSW_AWARB0_AW_ARB0_SHIFT                                     0
#define    SYSSW_AWARB0_AW_ARB0_MASK                                      0xFFFFFFFF

#define SYSSW_QOSTIDE1_OFFSET                                             0x00000410
#define SYSSW_QOSTIDE1_TYPE                                               UInt32
#define SYSSW_QOSTIDE1_RESERVED_MASK                                      0xFFFFFFC0
#define    SYSSW_QOSTIDE1_QOS_TIDE1_SHIFT                                 0
#define    SYSSW_QOSTIDE1_QOS_TIDE1_MASK                                  0x0000003F

#define SYSSW_QOSACC1_OFFSET                                              0x00000414
#define SYSSW_QOSACC1_TYPE                                                UInt32
#define SYSSW_QOSACC1_RESERVED_MASK                                       0xFFFFFFF0
#define    SYSSW_QOSACC1_QOS_ACC1_SHIFT                                   0
#define    SYSSW_QOSACC1_QOS_ACC1_MASK                                    0x0000000F

#define SYSSW_ARARB1_OFFSET                                               0x00000418
#define SYSSW_ARARB1_TYPE                                                 UInt32
#define SYSSW_ARARB1_RESERVED_MASK                                        0x00000000
#define    SYSSW_ARARB1_AR_ARB1_SHIFT                                     0
#define    SYSSW_ARARB1_AR_ARB1_MASK                                      0xFFFFFFFF

#define SYSSW_AWARB1_OFFSET                                               0x0000041C
#define SYSSW_AWARB1_TYPE                                                 UInt32
#define SYSSW_AWARB1_RESERVED_MASK                                        0x00000000
#define    SYSSW_AWARB1_AW_ARB1_SHIFT                                     0
#define    SYSSW_AWARB1_AW_ARB1_MASK                                      0xFFFFFFFF

#define SYSSW_ARARB2_OFFSET                                               0x00000428
#define SYSSW_ARARB2_TYPE                                                 UInt32
#define SYSSW_ARARB2_RESERVED_MASK                                        0x00000000
#define    SYSSW_ARARB2_AR_ARB2_SHIFT                                     0
#define    SYSSW_ARARB2_AR_ARB2_MASK                                      0xFFFFFFFF

#define SYSSW_AWARB2_OFFSET                                               0x0000042C
#define SYSSW_AWARB2_TYPE                                                 UInt32
#define SYSSW_AWARB2_RESERVED_MASK                                        0x00000000
#define    SYSSW_AWARB2_AW_ARB2_SHIFT                                     0
#define    SYSSW_AWARB2_AW_ARB2_MASK                                      0xFFFFFFFF

#define SYSSW_ARARB3_OFFSET                                               0x00000438
#define SYSSW_ARARB3_TYPE                                                 UInt32
#define SYSSW_ARARB3_RESERVED_MASK                                        0x00000000
#define    SYSSW_ARARB3_AR_ARB3_SHIFT                                     0
#define    SYSSW_ARARB3_AR_ARB3_MASK                                      0xFFFFFFFF

#define SYSSW_AWARB3_OFFSET                                               0x0000043C
#define SYSSW_AWARB3_TYPE                                                 UInt32
#define SYSSW_AWARB3_RESERVED_MASK                                        0x00000000
#define    SYSSW_AWARB3_AW_ARB3_SHIFT                                     0
#define    SYSSW_AWARB3_AW_ARB3_MASK                                      0xFFFFFFFF

#define SYSSW_ARARB4_OFFSET                                               0x00000448
#define SYSSW_ARARB4_TYPE                                                 UInt32
#define SYSSW_ARARB4_RESERVED_MASK                                        0x00000000
#define    SYSSW_ARARB4_AR_ARB4_SHIFT                                     0
#define    SYSSW_ARARB4_AR_ARB4_MASK                                      0xFFFFFFFF

#define SYSSW_AWARB4_OFFSET                                               0x0000044C
#define SYSSW_AWARB4_TYPE                                                 UInt32
#define SYSSW_AWARB4_RESERVED_MASK                                        0x00000000
#define    SYSSW_AWARB4_AW_ARB4_SHIFT                                     0
#define    SYSSW_AWARB4_AW_ARB4_MASK                                      0xFFFFFFFF

#define SYSSW_CONFIG0_OFFSET                                              0x00000FC0
#define SYSSW_CONFIG0_TYPE                                                UInt32
#define SYSSW_CONFIG0_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_CONFIG0_SI_PORTS_SHIFT                                   0
#define    SYSSW_CONFIG0_SI_PORTS_MASK                                    0x000000FF

#define SYSSW_CONFIG1_OFFSET                                              0x00000FC4
#define SYSSW_CONFIG1_TYPE                                                UInt32
#define SYSSW_CONFIG1_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_CONFIG1_MI_PORTS_SHIFT                                   0
#define    SYSSW_CONFIG1_MI_PORTS_MASK                                    0x000000FF

#define SYSSW_PERID0_OFFSET                                               0x00000FE0
#define SYSSW_PERID0_TYPE                                                 UInt32
#define SYSSW_PERID0_RESERVED_MASK                                        0xFFFFFF00
#define    SYSSW_PERID0_PID0_SHIFT                                        0
#define    SYSSW_PERID0_PID0_MASK                                         0x000000FF

#define SYSSW_PERID1_OFFSET                                               0x00000FE4
#define SYSSW_PERID1_TYPE                                                 UInt32
#define SYSSW_PERID1_RESERVED_MASK                                        0xFFFFFF00
#define    SYSSW_PERID1_PID1_SHIFT                                        0
#define    SYSSW_PERID1_PID1_MASK                                         0x000000FF

#define SYSSW_PERID2_OFFSET                                               0x00000FE8
#define SYSSW_PERID2_TYPE                                                 UInt32
#define SYSSW_PERID2_RESERVED_MASK                                        0xFFFFFF00
#define    SYSSW_PERID2_PID2_SHIFT                                        0
#define    SYSSW_PERID2_PID2_MASK                                         0x000000FF

#define SYSSW_PERID3_OFFSET                                               0x00000FEC
#define SYSSW_PERID3_TYPE                                                 UInt32
#define SYSSW_PERID3_RESERVED_MASK                                        0xFFFFFF00
#define    SYSSW_PERID3_PID3_SHIFT                                        0
#define    SYSSW_PERID3_PID3_MASK                                         0x000000FF

#define SYSSW_COMPID0_OFFSET                                              0x00000FF0
#define SYSSW_COMPID0_TYPE                                                UInt32
#define SYSSW_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_COMPID0_CID0_SHIFT                                       0
#define    SYSSW_COMPID0_CID0_MASK                                        0x000000FF

#define SYSSW_COMPID1_OFFSET                                              0x00000FF4
#define SYSSW_COMPID1_TYPE                                                UInt32
#define SYSSW_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_COMPID1_CID1_SHIFT                                       0
#define    SYSSW_COMPID1_CID1_MASK                                        0x000000FF

#define SYSSW_COMPID2_OFFSET                                              0x00000FF8
#define SYSSW_COMPID2_TYPE                                                UInt32
#define SYSSW_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_COMPID2_CID2_SHIFT                                       0
#define    SYSSW_COMPID2_CID2_MASK                                        0x000000FF

#define SYSSW_COMPID3_OFFSET                                              0x00000FFC
#define SYSSW_COMPID3_TYPE                                                UInt32
#define SYSSW_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    SYSSW_COMPID3_CID3_SHIFT                                       0
#define    SYSSW_COMPID3_CID3_MASK                                        0x000000FF

#endif /* __BRCM_RDB_SYSSW_H__ */


