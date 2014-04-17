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

#ifndef __BRCM_RDB_DMUX_H__
#define __BRCM_RDB_DMUX_H__

#define DMUX_CHAN_ALLOC_DEALLOC_OFFSET                                    0x00000000
#define DMUX_CHAN_ALLOC_DEALLOC_TYPE                                      UInt32
#define DMUX_CHAN_ALLOC_DEALLOC_RESERVED_MASK                             0xFFF0FFF0
#define    DMUX_CHAN_ALLOC_DEALLOC_FREE_CHANNELS_SHIFT                    16
#define    DMUX_CHAN_ALLOC_DEALLOC_FREE_CHANNELS_MASK                     0x000F0000
#define    DMUX_CHAN_ALLOC_DEALLOC_CHANNEL_AD_SHIFT                       0
#define    DMUX_CHAN_ALLOC_DEALLOC_CHANNEL_AD_MASK                        0x0000000F

#define DMUX_PER_DEALLOC_OFFSET                                           0x00000004
#define DMUX_PER_DEALLOC_TYPE                                             UInt32
#define DMUX_PER_DEALLOC_RESERVED_MASK                                    0xFFFFFFF0
#define    DMUX_PER_DEALLOC_CHANNEL_P_SHIFT                               0
#define    DMUX_PER_DEALLOC_CHANNEL_P_MASK                                0x0000000F

#define DMUX_SEMAPHORE_OFFSET                                             0x00000008
#define DMUX_SEMAPHORE_TYPE                                               UInt32
#define DMUX_SEMAPHORE_RESERVED_MASK                                      0xFFFFFFFE
#define    DMUX_SEMAPHORE_SEMAPHORE_FLAG_SHIFT                            0
#define    DMUX_SEMAPHORE_SEMAPHORE_FLAG_MASK                             0x00000001

#define DMUX_PER_0_OFFSET                                                 0x00000040
#define DMUX_PER_0_TYPE                                                   UInt32
#define DMUX_PER_0_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_0_PER_B_0_SHIFT                                       8
#define    DMUX_PER_0_PER_B_0_MASK                                        0x00007F00
#define    DMUX_PER_0_PER_A_0_SHIFT                                       0
#define    DMUX_PER_0_PER_A_0_MASK                                        0x0000007F

#define DMUX_PER_1_OFFSET                                                 0x00000044
#define DMUX_PER_1_TYPE                                                   UInt32
#define DMUX_PER_1_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_1_PER_B_1_SHIFT                                       8
#define    DMUX_PER_1_PER_B_1_MASK                                        0x00007F00
#define    DMUX_PER_1_PER_A_1_SHIFT                                       0
#define    DMUX_PER_1_PER_A_1_MASK                                        0x0000007F

#define DMUX_PER_2_OFFSET                                                 0x00000048
#define DMUX_PER_2_TYPE                                                   UInt32
#define DMUX_PER_2_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_2_PER_B_2_SHIFT                                       8
#define    DMUX_PER_2_PER_B_2_MASK                                        0x00007F00
#define    DMUX_PER_2_PER_A_2_SHIFT                                       0
#define    DMUX_PER_2_PER_A_2_MASK                                        0x0000007F

#define DMUX_PER_3_OFFSET                                                 0x0000004C
#define DMUX_PER_3_TYPE                                                   UInt32
#define DMUX_PER_3_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_3_PER_B_3_SHIFT                                       8
#define    DMUX_PER_3_PER_B_3_MASK                                        0x00007F00
#define    DMUX_PER_3_PER_A_3_SHIFT                                       0
#define    DMUX_PER_3_PER_A_3_MASK                                        0x0000007F

#define DMUX_PER_4_OFFSET                                                 0x00000050
#define DMUX_PER_4_TYPE                                                   UInt32
#define DMUX_PER_4_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_4_PER_B_4_SHIFT                                       8
#define    DMUX_PER_4_PER_B_4_MASK                                        0x00007F00
#define    DMUX_PER_4_PER_A_4_SHIFT                                       0
#define    DMUX_PER_4_PER_A_4_MASK                                        0x0000007F

#define DMUX_PER_5_OFFSET                                                 0x00000054
#define DMUX_PER_5_TYPE                                                   UInt32
#define DMUX_PER_5_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_5_PER_B_5_SHIFT                                       8
#define    DMUX_PER_5_PER_B_5_MASK                                        0x00007F00
#define    DMUX_PER_5_PER_A_5_SHIFT                                       0
#define    DMUX_PER_5_PER_A_5_MASK                                        0x0000007F

#define DMUX_PER_6_OFFSET                                                 0x00000058
#define DMUX_PER_6_TYPE                                                   UInt32
#define DMUX_PER_6_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_6_PER_B_6_SHIFT                                       8
#define    DMUX_PER_6_PER_B_6_MASK                                        0x00007F00
#define    DMUX_PER_6_PER_A_6_SHIFT                                       0
#define    DMUX_PER_6_PER_A_6_MASK                                        0x0000007F

#define DMUX_PER_7_OFFSET                                                 0x0000005C
#define DMUX_PER_7_TYPE                                                   UInt32
#define DMUX_PER_7_RESERVED_MASK                                          0xFFFF8080
#define    DMUX_PER_7_PER_B_7_SHIFT                                       8
#define    DMUX_PER_7_PER_B_7_MASK                                        0x00007F00
#define    DMUX_PER_7_PER_A_7_SHIFT                                       0
#define    DMUX_PER_7_PER_A_7_MASK                                        0x0000007F

#endif /* __BRCM_RDB_DMUX_H__ */


