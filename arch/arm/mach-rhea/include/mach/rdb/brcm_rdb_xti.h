/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:16:58                                             */
/*     RDB file : //HERA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_XTI_H__
#define __BRCM_RDB_XTI_H__

#define XTI_XTIID_OFFSET                                                  0x00000000
#define XTI_XTIID_TYPE                                                    UInt32
#define XTI_XTIID_RESERVED_MASK                                           0x00000000
#define    XTI_XTIID_POINTER_ID_SHIFT                                     0
#define    XTI_XTIID_POINTER_ID_MASK                                      0xFFFFFFFF

#define XTI_XTIMODE_OFFSET                                                0x00000004
#define XTI_XTIMODE_TYPE                                                  UInt32
#define XTI_XTIMODE_RESERVED_MASK                                         0xFFFFFE00
#define    XTI_XTIMODE_TESTMODE_SHIFT                                     8
#define    XTI_XTIMODE_TESTMODE_MASK                                      0x00000100
#define    XTI_XTIMODE_BREAKLIMIT_SHIFT                                   6
#define    XTI_XTIMODE_BREAKLIMIT_MASK                                    0x000000C0
#define    XTI_XTIMODE_BREAKOSMO_SHIFT                                    5
#define    XTI_XTIMODE_BREAKOSMO_MASK                                     0x00000020
#define    XTI_XTIMODE_BREAKHW_SHIFT                                      4
#define    XTI_XTIMODE_BREAKHW_MASK                                       0x00000010
#define    XTI_XTIMODE_IDLEVALUE_SHIFT                                    3
#define    XTI_XTIMODE_IDLEVALUE_MASK                                     0x00000008
#define    XTI_XTIMODE_IDLEFORCE_SHIFT                                    2
#define    XTI_XTIMODE_IDLEFORCE_MASK                                     0x00000004
#define    XTI_XTIMODE_OUTPUTMODE_SHIFT                                   1
#define    XTI_XTIMODE_OUTPUTMODE_MASK                                    0x00000002
#define    XTI_XTIMODE_XTIMODE_SHIFT                                      0
#define    XTI_XTIMODE_XTIMODE_MASK                                       0x00000001

#define XTI_XTIFIFOSTATUS_OFFSET                                          0x00000008
#define XTI_XTIFIFOSTATUS_TYPE                                            UInt32
#define XTI_XTIFIFOSTATUS_RESERVED_MASK                                   0x00000000
#define    XTI_XTIFIFOSTATUS_FIFOSTATUS_SHIFT                             0
#define    XTI_XTIFIFOSTATUS_FIFOSTATUS_MASK                              0xFFFFFFFF

#define XTI_XTIME0_OFFSET                                                 0x0000000C
#define XTI_XTIME0_TYPE                                                   UInt32
#define XTI_XTIME0_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME0_MASTEREN31TO0_SHIFT                                 0
#define    XTI_XTIME0_MASTEREN31TO0_MASK                                  0xFFFFFFFF

#define XTI_XTIMD0_OFFSET                                                 0x00000010
#define XTI_XTIMD0_TYPE                                                   UInt32
#define XTI_XTIMD0_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD0_MASTERDIS31TO0_SHIFT                                0
#define    XTI_XTIMD0_MASTERDIS31TO0_MASK                                 0xFFFFFFFF

#define XTI_XTIME1_OFFSET                                                 0x00000014
#define XTI_XTIME1_TYPE                                                   UInt32
#define XTI_XTIME1_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME1_MASTEREN63TO32_SHIFT                                0
#define    XTI_XTIME1_MASTEREN63TO32_MASK                                 0xFFFFFFFF

#define XTI_XTIMD1_OFFSET                                                 0x00000018
#define XTI_XTIMD1_TYPE                                                   UInt32
#define XTI_XTIMD1_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD1_MASTERDIS63TO32_SHIFT                               0
#define    XTI_XTIMD1_MASTERDIS63TO32_MASK                                0xFFFFFFFF

#define XTI_XTIME2_OFFSET                                                 0x0000001C
#define XTI_XTIME2_TYPE                                                   UInt32
#define XTI_XTIME2_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME2_MASTEREN95TO64_SHIFT                                0
#define    XTI_XTIME2_MASTEREN95TO64_MASK                                 0xFFFFFFFF

#define XTI_XTIMD2_OFFSET                                                 0x00000020
#define XTI_XTIMD2_TYPE                                                   UInt32
#define XTI_XTIMD2_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD2_MASTERDIS95TO65_SHIFT                               0
#define    XTI_XTIMD2_MASTERDIS95TO65_MASK                                0xFFFFFFFF

#define XTI_XTIME3_OFFSET                                                 0x00000024
#define XTI_XTIME3_TYPE                                                   UInt32
#define XTI_XTIME3_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME3_MASTEREN127TO96_SHIFT                               0
#define    XTI_XTIME3_MASTEREN127TO96_MASK                                0xFFFFFFFF

#define XTI_XTIMD3_OFFSET                                                 0x00000028
#define XTI_XTIMD3_TYPE                                                   UInt32
#define XTI_XTIMD3_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD3_MASTERDIS127TO96_SHIFT                              0
#define    XTI_XTIMD3_MASTERDIS127TO96_MASK                               0xFFFFFFFF

#define XTI_XTIME4_OFFSET                                                 0x0000002C
#define XTI_XTIME4_TYPE                                                   UInt32
#define XTI_XTIME4_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME4_MASTEREN159TO128_SHIFT                              0
#define    XTI_XTIME4_MASTEREN159TO128_MASK                               0xFFFFFFFF

#define XTI_XTIMD4_OFFSET                                                 0x00000030
#define XTI_XTIMD4_TYPE                                                   UInt32
#define XTI_XTIMD4_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD4_MASTERDIS159TO128_SHIFT                             0
#define    XTI_XTIMD4_MASTERDIS159TO128_MASK                              0xFFFFFFFF

#define XTI_XTIME5_OFFSET                                                 0x00000034
#define XTI_XTIME5_TYPE                                                   UInt32
#define XTI_XTIME5_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME5_MASTEREN191TO160_SHIFT                              0
#define    XTI_XTIME5_MASTEREN191TO160_MASK                               0xFFFFFFFF

#define XTI_XTIMD5_OFFSET                                                 0x00000038
#define XTI_XTIMD5_TYPE                                                   UInt32
#define XTI_XTIMD5_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD5_MASTERDIS191TO160_SHIFT                             0
#define    XTI_XTIMD5_MASTERDIS191TO160_MASK                              0xFFFFFFFF

#define XTI_XTIME6_OFFSET                                                 0x0000003C
#define XTI_XTIME6_TYPE                                                   UInt32
#define XTI_XTIME6_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME6_MASTEREN223TO192_SHIFT                              0
#define    XTI_XTIME6_MASTEREN223TO192_MASK                               0xFFFFFFFF

#define XTI_XTIMD6_OFFSET                                                 0x00000040
#define XTI_XTIMD6_TYPE                                                   UInt32
#define XTI_XTIMD6_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD6_MASTERDIS223TO192_SHIFT                             0
#define    XTI_XTIMD6_MASTERDIS223TO192_MASK                              0xFFFFFFFF

#define XTI_XTIME7_OFFSET                                                 0x00000044
#define XTI_XTIME7_TYPE                                                   UInt32
#define XTI_XTIME7_RESERVED_MASK                                          0x00000000
#define    XTI_XTIME7_MASTEREN255TO224_SHIFT                              0
#define    XTI_XTIME7_MASTEREN255TO224_MASK                               0xFFFFFFFF

#define XTI_XTIMD7_OFFSET                                                 0x00000048
#define XTI_XTIMD7_TYPE                                                   UInt32
#define XTI_XTIMD7_RESERVED_MASK                                          0x00000000
#define    XTI_XTIMD7_MASTERDIS255TO224_SHIFT                             0
#define    XTI_XTIMD7_MASTERDIS255TO224_MASK                              0xFFFFFFFF

#endif /* __BRCM_RDB_XTI_H__ */


