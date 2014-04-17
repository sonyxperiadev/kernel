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

#ifndef __BRCM_RDB_PSYNC_FOR_10PPM_H__
#define __BRCM_RDB_PSYNC_FOR_10PPM_H__

#define PSYNC_FOR_10PPM_CONFIG_OFFSET                                     0x00000000
#define PSYNC_FOR_10PPM_CONFIG_TYPE                                       UInt32
#define PSYNC_FOR_10PPM_CONFIG_RESERVED_MASK                              0x00000000
#define    PSYNC_FOR_10PPM_CONFIG_AUTORAMCLEAR_SHIFT                      31
#define    PSYNC_FOR_10PPM_CONFIG_AUTORAMCLEAR_MASK                       0x80000000
#define    PSYNC_FOR_10PPM_CONFIG_SEARCHLENGTH_SHIFT                      24
#define    PSYNC_FOR_10PPM_CONFIG_SEARCHLENGTH_MASK                       0x7F000000
#define    PSYNC_FOR_10PPM_CONFIG_SEARCHTYPE_SHIFT                        21
#define    PSYNC_FOR_10PPM_CONFIG_SEARCHTYPE_MASK                         0x00E00000
#define    PSYNC_FOR_10PPM_CONFIG_DIVERSITYMODE_SHIFT                     20
#define    PSYNC_FOR_10PPM_CONFIG_DIVERSITYMODE_MASK                      0x00100000
#define    PSYNC_FOR_10PPM_CONFIG_DECIMATEMODE_SHIFT                      18
#define    PSYNC_FOR_10PPM_CONFIG_DECIMATEMODE_MASK                       0x000C0000
#define    PSYNC_FOR_10PPM_CONFIG_SLOTPOS_SHIFT                           14
#define    PSYNC_FOR_10PPM_CONFIG_SLOTPOS_MASK                            0x0003C000
#define    PSYNC_FOR_10PPM_CONFIG_SAMPLEPOS_SHIFT                         0
#define    PSYNC_FOR_10PPM_CONFIG_SAMPLEPOS_MASK                          0x00003FFF

#define PSYNC_FOR_10PPM_COMMAND_OFFSET                                    0x00000004
#define PSYNC_FOR_10PPM_COMMAND_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_COMMAND_RESERVED_MASK                             0xFFFFFF00
#define    PSYNC_FOR_10PPM_COMMAND_MANUALRAMCLEAR_SHIFT                   7
#define    PSYNC_FOR_10PPM_COMMAND_MANUALRAMCLEAR_MASK                    0x00000080
#define    PSYNC_FOR_10PPM_COMMAND_SWTBHOLD_SHIFT                         6
#define    PSYNC_FOR_10PPM_COMMAND_SWTBHOLD_MASK                          0x00000040
#define    PSYNC_FOR_10PPM_COMMAND_SWRESET_SHIFT                          5
#define    PSYNC_FOR_10PPM_COMMAND_SWRESET_MASK                           0x00000020
#define    PSYNC_FOR_10PPM_COMMAND_SWSTOP_SHIFT                           4
#define    PSYNC_FOR_10PPM_COMMAND_SWSTOP_MASK                            0x00000010
#define    PSYNC_FOR_10PPM_COMMAND_STOPSLOTPOS_SHIFT                      0
#define    PSYNC_FOR_10PPM_COMMAND_STOPSLOTPOS_MASK                       0x0000000F

#define PSYNC_FOR_10PPM_STATICPARM_OFFSET                                 0x00000008
#define PSYNC_FOR_10PPM_STATICPARM_TYPE                                   UInt32
#define PSYNC_FOR_10PPM_STATICPARM_RESERVED_MASK                          0x00007F00
#define    PSYNC_FOR_10PPM_STATICPARM_FREQOFFSETSIGNFLIP_SHIFT            31
#define    PSYNC_FOR_10PPM_STATICPARM_FREQOFFSETSIGNFLIP_MASK             0x80000000
#define    PSYNC_FOR_10PPM_STATICPARM_TDCOMPSIGNFLIP_SHIFT                30
#define    PSYNC_FOR_10PPM_STATICPARM_TDCOMPSIGNFLIP_MASK                 0x40000000
#define    PSYNC_FOR_10PPM_STATICPARM_REPORTEDOFFSETADJUST_SHIFT          17
#define    PSYNC_FOR_10PPM_STATICPARM_REPORTEDOFFSETADJUST_MASK           0x3FFE0000
#define    PSYNC_FOR_10PPM_STATICPARM_NONDIVERSITYANTSEL_SHIFT            16
#define    PSYNC_FOR_10PPM_STATICPARM_NONDIVERSITYANTSEL_MASK             0x00010000
#define    PSYNC_FOR_10PPM_STATICPARM_OVERRIDECLOCKGATING_SHIFT           15
#define    PSYNC_FOR_10PPM_STATICPARM_OVERRIDECLOCKGATING_MASK            0x00008000
#define    PSYNC_FOR_10PPM_STATICPARM_MB1BRMNEN_SHIFT                     7
#define    PSYNC_FOR_10PPM_STATICPARM_MB1BRMNEN_MASK                      0x00000080
#define    PSYNC_FOR_10PPM_STATICPARM_MBPK32EN_SHIFT                      6
#define    PSYNC_FOR_10PPM_STATICPARM_MBPK32EN_MASK                       0x00000040
#define    PSYNC_FOR_10PPM_STATICPARM_GRIDGRP_SHIFT                       4
#define    PSYNC_FOR_10PPM_STATICPARM_GRIDGRP_MASK                        0x00000030
#define    PSYNC_FOR_10PPM_STATICPARM_PEAKMAXSEL_SHIFT                    3
#define    PSYNC_FOR_10PPM_STATICPARM_PEAKMAXSEL_MASK                     0x00000008
#define    PSYNC_FOR_10PPM_STATICPARM_PSYNCBW_SHIFT                       0
#define    PSYNC_FOR_10PPM_STATICPARM_PSYNCBW_MASK                        0x00000007

#define PSYNC_FOR_10PPM_DRIFTCOMP0_OFFSET                                 0x0000000C
#define PSYNC_FOR_10PPM_DRIFTCOMP0_TYPE                                   UInt32
#define PSYNC_FOR_10PPM_DRIFTCOMP0_RESERVED_MASK                          0xFFF88080
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTOFFSET0_SHIFT              16
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTOFFSET0_MASK               0x00070000
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTINIT0_SHIFT                8
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTINIT0_MASK                 0x00007F00
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTRLOVR0_SHIFT               0
#define    PSYNC_FOR_10PPM_DRIFTCOMP0_TDCOMPCNTRLOVR0_MASK                0x0000007F

#define PSYNC_FOR_10PPM_DRIFTCOMP1_OFFSET                                 0x00000010
#define PSYNC_FOR_10PPM_DRIFTCOMP1_TYPE                                   UInt32
#define PSYNC_FOR_10PPM_DRIFTCOMP1_RESERVED_MASK                          0xFFF88080
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTOFFSET1_SHIFT              16
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTOFFSET1_MASK               0x00070000
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTINIT1_SHIFT                8
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTINIT1_MASK                 0x00007F00
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTRLOVR1_SHIFT               0
#define    PSYNC_FOR_10PPM_DRIFTCOMP1_TDCOMPCNTRLOVR1_MASK                0x0000007F

#define PSYNC_FOR_10PPM_DRIFTCOMP2_OFFSET                                 0x00000014
#define PSYNC_FOR_10PPM_DRIFTCOMP2_TYPE                                   UInt32
#define PSYNC_FOR_10PPM_DRIFTCOMP2_RESERVED_MASK                          0xFFF88080
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTOFFSET2_SHIFT              16
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTOFFSET2_MASK               0x00070000
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTINIT2_SHIFT                8
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTINIT2_MASK                 0x00007F00
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTRLOVR2_SHIFT               0
#define    PSYNC_FOR_10PPM_DRIFTCOMP2_TDCOMPCNTRLOVR2_MASK                0x0000007F

#define PSYNC_FOR_10PPM_DRIFTCOMP3_OFFSET                                 0x00000018
#define PSYNC_FOR_10PPM_DRIFTCOMP3_TYPE                                   UInt32
#define PSYNC_FOR_10PPM_DRIFTCOMP3_RESERVED_MASK                          0xFFF88080
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTOFFSET3_SHIFT              16
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTOFFSET3_MASK               0x00070000
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTINIT3_SHIFT                8
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTINIT3_MASK                 0x00007F00
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTRLOVR3_SHIFT               0
#define    PSYNC_FOR_10PPM_DRIFTCOMP3_TDCOMPCNTRLOVR3_MASK                0x0000007F

#define PSYNC_FOR_10PPM_STATUS_OFFSET                                     0x0000001C
#define PSYNC_FOR_10PPM_STATUS_TYPE                                       UInt32
#define PSYNC_FOR_10PPM_STATUS_RESERVED_MASK                              0x00000000
#define    PSYNC_FOR_10PPM_STATUS_PSYNCBUSY_SHIFT                         31
#define    PSYNC_FOR_10PPM_STATUS_PSYNCBUSY_MASK                          0x80000000
#define    PSYNC_FOR_10PPM_STATUS_SEARCHSLOTINDEX_SHIFT                   24
#define    PSYNC_FOR_10PPM_STATUS_SEARCHSLOTINDEX_MASK                    0x7F000000
#define    PSYNC_FOR_10PPM_STATUS_MAXMAGVALUE_SHIFT                       12
#define    PSYNC_FOR_10PPM_STATUS_MAXMAGVALUE_MASK                        0x00FFF000
#define    PSYNC_FOR_10PPM_STATUS_MEANMAGVALUE_SHIFT                      0
#define    PSYNC_FOR_10PPM_STATUS_MEANMAGVALUE_MASK                       0x00000FFF

#define PSYNC_FOR_10PPM_TABLE00_OFFSET                                    0x00000020
#define PSYNC_FOR_10PPM_TABLE00_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE00_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE00_TABLEMAGOUTACCUM00_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE00_TABLEMAGOUTACCUM00_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE00_TABLESAMPLECOUNT00_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE00_TABLESAMPLECOUNT00_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE01_OFFSET                                    0x00000024
#define PSYNC_FOR_10PPM_TABLE01_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE01_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE01_TABLEMAGOUTACCUM01_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE01_TABLEMAGOUTACCUM01_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE01_TABLESAMPLECOUNT01_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE01_TABLESAMPLECOUNT01_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE02_OFFSET                                    0x00000028
#define PSYNC_FOR_10PPM_TABLE02_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE02_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE02_TABLEMAGOUTACCUM02_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE02_TABLEMAGOUTACCUM02_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE02_TABLESAMPLECOUNT02_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE02_TABLESAMPLECOUNT02_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE03_OFFSET                                    0x0000002C
#define PSYNC_FOR_10PPM_TABLE03_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE03_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE03_TABLEMAGOUTACCUM03_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE03_TABLEMAGOUTACCUM03_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE03_TABLESAMPLECOUNT03_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE03_TABLESAMPLECOUNT03_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE04_OFFSET                                    0x00000030
#define PSYNC_FOR_10PPM_TABLE04_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE04_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE04_TABLEMAGOUTACCUM04_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE04_TABLEMAGOUTACCUM04_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE04_TABLESAMPLECOUNT04_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE04_TABLESAMPLECOUNT04_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE05_OFFSET                                    0x00000034
#define PSYNC_FOR_10PPM_TABLE05_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE05_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE05_TABLEMAGOUTACCUM05_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE05_TABLEMAGOUTACCUM05_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE05_TABLESAMPLECOUNT05_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE05_TABLESAMPLECOUNT05_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE06_OFFSET                                    0x00000038
#define PSYNC_FOR_10PPM_TABLE06_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE06_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE06_TABLEMAGOUTACCUM06_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE06_TABLEMAGOUTACCUM06_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE06_TABLESAMPLECOUNT06_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE06_TABLESAMPLECOUNT06_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE07_OFFSET                                    0x0000003C
#define PSYNC_FOR_10PPM_TABLE07_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE07_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE07_TABLEMAGOUTACCUM07_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE07_TABLEMAGOUTACCUM07_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE07_TABLESAMPLECOUNT07_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE07_TABLESAMPLECOUNT07_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE08_OFFSET                                    0x00000040
#define PSYNC_FOR_10PPM_TABLE08_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE08_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE08_TABLEMAGOUTACCUM08_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE08_TABLEMAGOUTACCUM08_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE08_TABLESAMPLECOUNT08_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE08_TABLESAMPLECOUNT08_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE09_OFFSET                                    0x00000044
#define PSYNC_FOR_10PPM_TABLE09_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE09_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE09_TABLEMAGOUTACCUM09_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE09_TABLEMAGOUTACCUM09_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE09_TABLESAMPLECOUNT09_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE09_TABLESAMPLECOUNT09_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE10_OFFSET                                    0x00000048
#define PSYNC_FOR_10PPM_TABLE10_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE10_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE10_TABLEMAGOUTACCUM10_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE10_TABLEMAGOUTACCUM10_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE10_TABLESAMPLECOUNT10_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE10_TABLESAMPLECOUNT10_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE11_OFFSET                                    0x0000004C
#define PSYNC_FOR_10PPM_TABLE11_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE11_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE11_TABLEMAGOUTACCUM11_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE11_TABLEMAGOUTACCUM11_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE11_TABLESAMPLECOUNT11_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE11_TABLESAMPLECOUNT11_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE12_OFFSET                                    0x00000050
#define PSYNC_FOR_10PPM_TABLE12_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE12_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE12_TABLEMAGOUTACCUM12_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE12_TABLEMAGOUTACCUM12_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE12_TABLESAMPLECOUNT12_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE12_TABLESAMPLECOUNT12_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE13_OFFSET                                    0x00000054
#define PSYNC_FOR_10PPM_TABLE13_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE13_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE13_TABLEMAGOUTACCUM13_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE13_TABLEMAGOUTACCUM13_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE13_TABLESAMPLECOUNT13_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE13_TABLESAMPLECOUNT13_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE14_OFFSET                                    0x00000058
#define PSYNC_FOR_10PPM_TABLE14_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE14_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE14_TABLEMAGOUTACCUM14_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE14_TABLEMAGOUTACCUM14_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE14_TABLESAMPLECOUNT14_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE14_TABLESAMPLECOUNT14_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE15_OFFSET                                    0x0000005C
#define PSYNC_FOR_10PPM_TABLE15_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE15_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE15_TABLEMAGOUTACCUM15_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE15_TABLEMAGOUTACCUM15_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE15_TABLESAMPLECOUNT15_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE15_TABLESAMPLECOUNT15_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE16_OFFSET                                    0x00000060
#define PSYNC_FOR_10PPM_TABLE16_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE16_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE16_TABLEMAGOUTACCUM16_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE16_TABLEMAGOUTACCUM16_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE16_TABLESAMPLECOUNT16_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE16_TABLESAMPLECOUNT16_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE17_OFFSET                                    0x00000064
#define PSYNC_FOR_10PPM_TABLE17_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE17_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE17_TABLEMAGOUTACCUM17_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE17_TABLEMAGOUTACCUM17_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE17_TABLESAMPLECOUNT17_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE17_TABLESAMPLECOUNT17_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE18_OFFSET                                    0x00000068
#define PSYNC_FOR_10PPM_TABLE18_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE18_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE18_TABLEMAGOUTACCUM18_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE18_TABLEMAGOUTACCUM18_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE18_TABLESAMPLECOUNT18_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE18_TABLESAMPLECOUNT18_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE19_OFFSET                                    0x0000006C
#define PSYNC_FOR_10PPM_TABLE19_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE19_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE19_TABLEMAGOUTACCUM19_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE19_TABLEMAGOUTACCUM19_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE19_TABLESAMPLECOUNT19_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE19_TABLESAMPLECOUNT19_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE20_OFFSET                                    0x00000070
#define PSYNC_FOR_10PPM_TABLE20_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE20_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE20_TABLEMAGOUTACCUM20_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE20_TABLEMAGOUTACCUM20_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE20_TABLESAMPLECOUNT20_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE20_TABLESAMPLECOUNT20_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE21_OFFSET                                    0x00000074
#define PSYNC_FOR_10PPM_TABLE21_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE21_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE21_TABLEMAGOUTACCUM21_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE21_TABLEMAGOUTACCUM21_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE21_TABLESAMPLECOUNT21_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE21_TABLESAMPLECOUNT21_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE22_OFFSET                                    0x00000078
#define PSYNC_FOR_10PPM_TABLE22_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE22_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE22_TABLEMAGOUTACCUM22_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE22_TABLEMAGOUTACCUM22_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE22_TABLESAMPLECOUNT22_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE22_TABLESAMPLECOUNT22_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE23_OFFSET                                    0x0000007C
#define PSYNC_FOR_10PPM_TABLE23_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE23_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE23_TABLEMAGOUTACCUM23_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE23_TABLEMAGOUTACCUM23_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE23_TABLESAMPLECOUNT23_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE23_TABLESAMPLECOUNT23_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE24_OFFSET                                    0x00000080
#define PSYNC_FOR_10PPM_TABLE24_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE24_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE24_TABLEMAGOUTACCUM24_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE24_TABLEMAGOUTACCUM24_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE24_TABLESAMPLECOUNT24_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE24_TABLESAMPLECOUNT24_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE25_OFFSET                                    0x00000084
#define PSYNC_FOR_10PPM_TABLE25_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE25_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE25_TABLEMAGOUTACCUM25_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE25_TABLEMAGOUTACCUM25_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE25_TABLESAMPLECOUNT25_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE25_TABLESAMPLECOUNT25_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE26_OFFSET                                    0x00000088
#define PSYNC_FOR_10PPM_TABLE26_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE26_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE26_TABLEMAGOUTACCUM26_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE26_TABLEMAGOUTACCUM26_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE26_TABLESAMPLECOUNT26_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE26_TABLESAMPLECOUNT26_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE27_OFFSET                                    0x0000008C
#define PSYNC_FOR_10PPM_TABLE27_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE27_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE27_TABLEMAGOUTACCUM27_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE27_TABLEMAGOUTACCUM27_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE27_TABLESAMPLECOUNT27_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE27_TABLESAMPLECOUNT27_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE28_OFFSET                                    0x00000090
#define PSYNC_FOR_10PPM_TABLE28_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE28_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE28_TABLEMAGOUTACCUM28_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE28_TABLEMAGOUTACCUM28_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE28_TABLESAMPLECOUNT28_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE28_TABLESAMPLECOUNT28_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE29_OFFSET                                    0x00000094
#define PSYNC_FOR_10PPM_TABLE29_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE29_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE29_TABLEMAGOUTACCUM29_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE29_TABLEMAGOUTACCUM29_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE29_TABLESAMPLECOUNT29_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE29_TABLESAMPLECOUNT29_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE30_OFFSET                                    0x00000098
#define PSYNC_FOR_10PPM_TABLE30_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE30_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE30_TABLEMAGOUTACCUM30_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE30_TABLEMAGOUTACCUM30_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE30_TABLESAMPLECOUNT30_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE30_TABLESAMPLECOUNT30_MASK                0x00003FFF

#define PSYNC_FOR_10PPM_TABLE31_OFFSET                                    0x0000009C
#define PSYNC_FOR_10PPM_TABLE31_TYPE                                      UInt32
#define PSYNC_FOR_10PPM_TABLE31_RESERVED_MASK                             0xF000C000
#define    PSYNC_FOR_10PPM_TABLE31_TABLEMAGOUTACCUM31_SHIFT               16
#define    PSYNC_FOR_10PPM_TABLE31_TABLEMAGOUTACCUM31_MASK                0x0FFF0000
#define    PSYNC_FOR_10PPM_TABLE31_TABLESAMPLECOUNT31_SHIFT               0
#define    PSYNC_FOR_10PPM_TABLE31_TABLESAMPLECOUNT31_MASK                0x00003FFF

#endif /* __BRCM_RDB_PSYNC_FOR_10PPM_H__ */


