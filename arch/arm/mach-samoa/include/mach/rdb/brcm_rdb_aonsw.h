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
/*     Date     : Generated on 5/15/2011 11:41:39                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_AONSW_H__
#define __BRCM_RDB_AONSW_H__

#define AONSW_ARARB0_OFFSET                                               0x00000408
#define AONSW_ARARB0_TYPE                                                 UInt32
#define AONSW_ARARB0_RESERVED_MASK                                        0x00000000
#define    AONSW_ARARB0_AR_ARB0_SHIFT                                     0
#define    AONSW_ARARB0_AR_ARB0_MASK                                      0xFFFFFFFF

#define AONSW_AWARB0_OFFSET                                               0x0000040C
#define AONSW_AWARB0_TYPE                                                 UInt32
#define AONSW_AWARB0_RESERVED_MASK                                        0x00000000
#define    AONSW_AWARB0_AW_ARB0_SHIFT                                     0
#define    AONSW_AWARB0_AW_ARB0_MASK                                      0xFFFFFFFF

#define AONSW_ARARB1_OFFSET                                               0x00000428
#define AONSW_ARARB1_TYPE                                                 UInt32
#define AONSW_ARARB1_RESERVED_MASK                                        0x00000000
#define    AONSW_ARARB1_AR_ARB1_SHIFT                                     0
#define    AONSW_ARARB1_AR_ARB1_MASK                                      0xFFFFFFFF

#define AONSW_AWARB1_OFFSET                                               0x0000042C
#define AONSW_AWARB1_TYPE                                                 UInt32
#define AONSW_AWARB1_RESERVED_MASK                                        0x00000000
#define    AONSW_AWARB1_AW_ARB1_SHIFT                                     0
#define    AONSW_AWARB1_AW_ARB1_MASK                                      0xFFFFFFFF

#define AONSW_ARARB2_OFFSET                                               0x00000448
#define AONSW_ARARB2_TYPE                                                 UInt32
#define AONSW_ARARB2_RESERVED_MASK                                        0x00000000
#define    AONSW_ARARB2_AR_ARB2_SHIFT                                     0
#define    AONSW_ARARB2_AR_ARB2_MASK                                      0xFFFFFFFF

#define AONSW_AWARB2_OFFSET                                               0x0000044C
#define AONSW_AWARB2_TYPE                                                 UInt32
#define AONSW_AWARB2_RESERVED_MASK                                        0x00000000
#define    AONSW_AWARB2_AW_ARB2_SHIFT                                     0
#define    AONSW_AWARB2_AW_ARB2_MASK                                      0xFFFFFFFF

#define AONSW_CONFIG0_OFFSET                                              0x00000FC0
#define AONSW_CONFIG0_TYPE                                                UInt32
#define AONSW_CONFIG0_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_CONFIG0_SI_PORTS_SHIFT                                   0
#define    AONSW_CONFIG0_SI_PORTS_MASK                                    0x000000FF

#define AONSW_CONFIG1_OFFSET                                              0x00000FC4
#define AONSW_CONFIG1_TYPE                                                UInt32
#define AONSW_CONFIG1_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_CONFIG1_MI_PORTS_SHIFT                                   0
#define    AONSW_CONFIG1_MI_PORTS_MASK                                    0x000000FF

#define AONSW_PERID0_OFFSET                                               0x00000FE0
#define AONSW_PERID0_TYPE                                                 UInt32
#define AONSW_PERID0_RESERVED_MASK                                        0xFFFFFF00
#define    AONSW_PERID0_PID0_SHIFT                                        0
#define    AONSW_PERID0_PID0_MASK                                         0x000000FF

#define AONSW_PERID1_OFFSET                                               0x00000FE4
#define AONSW_PERID1_TYPE                                                 UInt32
#define AONSW_PERID1_RESERVED_MASK                                        0xFFFFFF00
#define    AONSW_PERID1_PID1_SHIFT                                        0
#define    AONSW_PERID1_PID1_MASK                                         0x000000FF

#define AONSW_PERID2_OFFSET                                               0x00000FE8
#define AONSW_PERID2_TYPE                                                 UInt32
#define AONSW_PERID2_RESERVED_MASK                                        0xFFFFFF00
#define    AONSW_PERID2_PID2_SHIFT                                        0
#define    AONSW_PERID2_PID2_MASK                                         0x000000FF

#define AONSW_PERID3_OFFSET                                               0x00000FEC
#define AONSW_PERID3_TYPE                                                 UInt32
#define AONSW_PERID3_RESERVED_MASK                                        0xFFFFFF00
#define    AONSW_PERID3_PID3_SHIFT                                        0
#define    AONSW_PERID3_PID3_MASK                                         0x000000FF

#define AONSW_COMPID0_OFFSET                                              0x00000FF0
#define AONSW_COMPID0_TYPE                                                UInt32
#define AONSW_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_COMPID0_CID0_SHIFT                                       0
#define    AONSW_COMPID0_CID0_MASK                                        0x000000FF

#define AONSW_COMPID1_OFFSET                                              0x00000FF4
#define AONSW_COMPID1_TYPE                                                UInt32
#define AONSW_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_COMPID1_CID1_SHIFT                                       0
#define    AONSW_COMPID1_CID1_MASK                                        0x000000FF

#define AONSW_COMPID2_OFFSET                                              0x00000FF8
#define AONSW_COMPID2_TYPE                                                UInt32
#define AONSW_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_COMPID2_CID2_SHIFT                                       0
#define    AONSW_COMPID2_CID2_MASK                                        0x000000FF

#define AONSW_COMPID3_OFFSET                                              0x00000FFC
#define AONSW_COMPID3_TYPE                                                UInt32
#define AONSW_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    AONSW_COMPID3_CID3_SHIFT                                       0
#define    AONSW_COMPID3_CID3_MASK                                        0x000000FF

#endif /* __BRCM_RDB_AONSW_H__ */


