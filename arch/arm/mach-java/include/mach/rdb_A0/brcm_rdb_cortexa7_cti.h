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

#ifndef __BRCM_RDB_CORTEXA7_CTI_H__
#define __BRCM_RDB_CORTEXA7_CTI_H__

#define CORTEXA7_CTI_CTICONTROL_OFFSET                                    0x00000000
#define CORTEXA7_CTI_CTICONTROL_TYPE                                      UInt32
#define CORTEXA7_CTI_CTICONTROL_RESERVED_MASK                             0xFFFFFFFE
#define    CORTEXA7_CTI_CTICONTROL_GLBEN_SHIFT                            0
#define    CORTEXA7_CTI_CTICONTROL_GLBEN_MASK                             0x00000001

#define CORTEXA7_CTI_CTIINTACK_OFFSET                                     0x00000010
#define CORTEXA7_CTI_CTIINTACK_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIINTACK_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_CTIINTACK_INTACK_SHIFT                            0
#define    CORTEXA7_CTI_CTIINTACK_INTACK_MASK                             0x000000FF

#define CORTEXA7_CTI_CTIAPPSET_OFFSET                                     0x00000014
#define CORTEXA7_CTI_CTIAPPSET_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIAPPSET_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIAPPSET_APPSET_SHIFT                            0
#define    CORTEXA7_CTI_CTIAPPSET_APPSET_MASK                             0x0000000F

#define CORTEXA7_CTI_CTIAPPCLEAR_OFFSET                                   0x00000018
#define CORTEXA7_CTI_CTIAPPCLEAR_TYPE                                     UInt32
#define CORTEXA7_CTI_CTIAPPCLEAR_RESERVED_MASK                            0xFFFFFFF0
#define    CORTEXA7_CTI_CTIAPPCLEAR_APPCLEAR_SHIFT                        0
#define    CORTEXA7_CTI_CTIAPPCLEAR_APPCLEAR_MASK                         0x0000000F

#define CORTEXA7_CTI_CTIAPPPULSE_OFFSET                                   0x0000001C
#define CORTEXA7_CTI_CTIAPPPULSE_TYPE                                     UInt32
#define CORTEXA7_CTI_CTIAPPPULSE_RESERVED_MASK                            0xFFFFFFF0
#define    CORTEXA7_CTI_CTIAPPPULSE_APPPULSE_SHIFT                        0
#define    CORTEXA7_CTI_CTIAPPPULSE_APPPULSE_MASK                         0x0000000F

#define CORTEXA7_CTI_CTIINEN0_OFFSET                                      0x00000020
#define CORTEXA7_CTI_CTIINEN0_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN0_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN0_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN0_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN1_OFFSET                                      0x00000024
#define CORTEXA7_CTI_CTIINEN1_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN1_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN1_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN1_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN2_OFFSET                                      0x00000028
#define CORTEXA7_CTI_CTIINEN2_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN2_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN2_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN2_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN3_OFFSET                                      0x0000002C
#define CORTEXA7_CTI_CTIINEN3_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN3_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN3_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN3_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN4_OFFSET                                      0x00000030
#define CORTEXA7_CTI_CTIINEN4_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN4_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN4_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN4_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN5_OFFSET                                      0x00000034
#define CORTEXA7_CTI_CTIINEN5_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN5_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN5_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN5_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN6_OFFSET                                      0x00000038
#define CORTEXA7_CTI_CTIINEN6_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN6_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN6_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN6_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIINEN7_OFFSET                                      0x0000003C
#define CORTEXA7_CTI_CTIINEN7_TYPE                                        UInt32
#define CORTEXA7_CTI_CTIINEN7_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CTIINEN7_TRIGINEN_SHIFT                           0
#define    CORTEXA7_CTI_CTIINEN7_TRIGINEN_MASK                            0x0000000F

#define CORTEXA7_CTI_CTIOUTEN0_OFFSET                                     0x000000A0
#define CORTEXA7_CTI_CTIOUTEN0_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN0_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN0_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN0_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN1_OFFSET                                     0x000000A4
#define CORTEXA7_CTI_CTIOUTEN1_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN1_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN1_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN1_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN2_OFFSET                                     0x000000A8
#define CORTEXA7_CTI_CTIOUTEN2_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN2_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN2_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN2_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN3_OFFSET                                     0x000000AC
#define CORTEXA7_CTI_CTIOUTEN3_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN3_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN3_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN3_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN4_OFFSET                                     0x000000B0
#define CORTEXA7_CTI_CTIOUTEN4_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN4_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN4_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN4_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN5_OFFSET                                     0x000000B4
#define CORTEXA7_CTI_CTIOUTEN5_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN5_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN5_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN5_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN6_OFFSET                                     0x000000B8
#define CORTEXA7_CTI_CTIOUTEN6_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN6_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN6_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN6_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTIOUTEN7_OFFSET                                     0x000000BC
#define CORTEXA7_CTI_CTIOUTEN7_TYPE                                       UInt32
#define CORTEXA7_CTI_CTIOUTEN7_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_CTIOUTEN7_TRIGOUTEN_SHIFT                         0
#define    CORTEXA7_CTI_CTIOUTEN7_TRIGOUTEN_MASK                          0x0000000F

#define CORTEXA7_CTI_CTITRIGINSTATUS_OFFSET                               0x00000130
#define CORTEXA7_CTI_CTITRIGINSTATUS_TYPE                                 UInt32
#define CORTEXA7_CTI_CTITRIGINSTATUS_RESERVED_MASK                        0xFFFFFF00
#define    CORTEXA7_CTI_CTITRIGINSTATUS_TRIGINSTATUS_SHIFT                0
#define    CORTEXA7_CTI_CTITRIGINSTATUS_TRIGINSTATUS_MASK                 0x000000FF

#define CORTEXA7_CTI_CTITRIGOUTSTATUS_OFFSET                              0x00000134
#define CORTEXA7_CTI_CTITRIGOUTSTATUS_TYPE                                UInt32
#define CORTEXA7_CTI_CTITRIGOUTSTATUS_RESERVED_MASK                       0xFFFFFF00
#define    CORTEXA7_CTI_CTITRIGOUTSTATUS_TRIGOUTSTATUS_SHIFT              0
#define    CORTEXA7_CTI_CTITRIGOUTSTATUS_TRIGOUTSTATUS_MASK               0x000000FF

#define CORTEXA7_CTI_CTICHINSTATUS_OFFSET                                 0x00000138
#define CORTEXA7_CTI_CTICHINSTATUS_TYPE                                   UInt32
#define CORTEXA7_CTI_CTICHINSTATUS_RESERVED_MASK                          0xFFFFFFF0
#define    CORTEXA7_CTI_CTICHINSTATUS_CTICHINSTATUS_SHIFT                 0
#define    CORTEXA7_CTI_CTICHINSTATUS_CTICHINSTATUS_MASK                  0x0000000F

#define CORTEXA7_CTI_CTICHOUTSTATUS_OFFSET                                0x0000013C
#define CORTEXA7_CTI_CTICHOUTSTATUS_TYPE                                  UInt32
#define CORTEXA7_CTI_CTICHOUTSTATUS_RESERVED_MASK                         0xFFFFFFF0
#define    CORTEXA7_CTI_CTICHOUTSTATUS_CTICHOUTSTATUS_SHIFT               0
#define    CORTEXA7_CTI_CTICHOUTSTATUS_CTICHOUTSTATUS_MASK                0x0000000F

#define CORTEXA7_CTI_CTIGATE_OFFSET                                       0x00000140
#define CORTEXA7_CTI_CTIGATE_TYPE                                         UInt32
#define CORTEXA7_CTI_CTIGATE_RESERVED_MASK                                0xFFFFFFF0
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN3_SHIFT                          3
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN3_MASK                           0x00000008
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN2_SHIFT                          2
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN2_MASK                           0x00000004
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN1_SHIFT                          1
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN1_MASK                           0x00000002
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN0_SHIFT                          0
#define    CORTEXA7_CTI_CTIGATE_CTIGATEEN0_MASK                           0x00000001

#define CORTEXA7_CTI_ASICCTL_OFFSET                                       0x00000144
#define CORTEXA7_CTI_ASICCTL_TYPE                                         UInt32
#define CORTEXA7_CTI_ASICCTL_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_ASICCTL_ASICCTL_SHIFT                             0
#define    CORTEXA7_CTI_ASICCTL_ASICCTL_MASK                              0x000000FF

#define CORTEXA7_CTI_ITCHINACK_OFFSET                                     0x00000EDC
#define CORTEXA7_CTI_ITCHINACK_TYPE                                       UInt32
#define CORTEXA7_CTI_ITCHINACK_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_CTI_ITCHINACK_CTCHINACK_SHIFT                         0
#define    CORTEXA7_CTI_ITCHINACK_CTCHINACK_MASK                          0x0000000F

#define CORTEXA7_CTI_ITTRIGINACK_OFFSET                                   0x00000EE0
#define CORTEXA7_CTI_ITTRIGINACK_TYPE                                     UInt32
#define CORTEXA7_CTI_ITTRIGINACK_RESERVED_MASK                            0xFFFFFF00
#define    CORTEXA7_CTI_ITTRIGINACK_CTTRIGINACK_SHIFT                     0
#define    CORTEXA7_CTI_ITTRIGINACK_CTTRIGINACK_MASK                      0x000000FF

#define CORTEXA7_CTI_ITCHOUT_OFFSET                                       0x00000EE4
#define CORTEXA7_CTI_ITCHOUT_TYPE                                         UInt32
#define CORTEXA7_CTI_ITCHOUT_RESERVED_MASK                                0xFFFFFFF0
#define    CORTEXA7_CTI_ITCHOUT_CTCHOUT_SHIFT                             0
#define    CORTEXA7_CTI_ITCHOUT_CTCHOUT_MASK                              0x0000000F

#define CORTEXA7_CTI_ITTRIGOUT_OFFSET                                     0x00000EE8
#define CORTEXA7_CTI_ITTRIGOUT_TYPE                                       UInt32
#define CORTEXA7_CTI_ITTRIGOUT_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_ITTRIGOUT_CTTRIGOUT_SHIFT                         0
#define    CORTEXA7_CTI_ITTRIGOUT_CTTRIGOUT_MASK                          0x000000FF

#define CORTEXA7_CTI_ITCHOUTACK_OFFSET                                    0x00000EEC
#define CORTEXA7_CTI_ITCHOUTACK_TYPE                                      UInt32
#define CORTEXA7_CTI_ITCHOUTACK_RESERVED_MASK                             0xFFFFFFF0
#define    CORTEXA7_CTI_ITCHOUTACK_CTCHOUTACK_SHIFT                       0
#define    CORTEXA7_CTI_ITCHOUTACK_CTCHOUTACK_MASK                        0x0000000F

#define CORTEXA7_CTI_ITTRIGOUTACK_OFFSET                                  0x00000EF0
#define CORTEXA7_CTI_ITTRIGOUTACK_TYPE                                    UInt32
#define CORTEXA7_CTI_ITTRIGOUTACK_RESERVED_MASK                           0xFFFFFF00
#define    CORTEXA7_CTI_ITTRIGOUTACK_CTTRIGOUTACK_SHIFT                   0
#define    CORTEXA7_CTI_ITTRIGOUTACK_CTTRIGOUTACK_MASK                    0x000000FF

#define CORTEXA7_CTI_ITCHIN_OFFSET                                        0x00000EF4
#define CORTEXA7_CTI_ITCHIN_TYPE                                          UInt32
#define CORTEXA7_CTI_ITCHIN_RESERVED_MASK                                 0xFFFFFFF0
#define    CORTEXA7_CTI_ITCHIN_CTCHIN_SHIFT                               0
#define    CORTEXA7_CTI_ITCHIN_CTCHIN_MASK                                0x0000000F

#define CORTEXA7_CTI_ITTRIGIN_OFFSET                                      0x00000EF8
#define CORTEXA7_CTI_ITTRIGIN_TYPE                                        UInt32
#define CORTEXA7_CTI_ITTRIGIN_RESERVED_MASK                               0xFFFFFF00
#define    CORTEXA7_CTI_ITTRIGIN_CTTRIGIN_SHIFT                           0
#define    CORTEXA7_CTI_ITTRIGIN_CTTRIGIN_MASK                            0x000000FF

#define CORTEXA7_CTI_ITCTRL_OFFSET                                        0x00000F00
#define CORTEXA7_CTI_ITCTRL_TYPE                                          UInt32
#define CORTEXA7_CTI_ITCTRL_RESERVED_MASK                                 0xFFFFFFFE
#define    CORTEXA7_CTI_ITCTRL_INTEGRATION_MODE_SHIFT                     0
#define    CORTEXA7_CTI_ITCTRL_INTEGRATION_MODE_MASK                      0x00000001

#define CORTEXA7_CTI_CLAIMSET_OFFSET                                      0x00000FA0
#define CORTEXA7_CTI_CLAIMSET_TYPE                                        UInt32
#define CORTEXA7_CTI_CLAIMSET_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CLAIMSET_CLAIM_TAGSA_SHIFT                        0
#define    CORTEXA7_CTI_CLAIMSET_CLAIM_TAGSA_MASK                         0x0000000F

#define CORTEXA7_CTI_CLAIMCLR_OFFSET                                      0x00000FA4
#define CORTEXA7_CTI_CLAIMCLR_TYPE                                        UInt32
#define CORTEXA7_CTI_CLAIMCLR_RESERVED_MASK                               0xFFFFFFF0
#define    CORTEXA7_CTI_CLAIMCLR_CLAIM_TAGSB_SHIFT                        0
#define    CORTEXA7_CTI_CLAIMCLR_CLAIM_TAGSB_MASK                         0x0000000F

#define CORTEXA7_CTI_LOCKACCESS_OFFSET                                    0x00000FB0
#define CORTEXA7_CTI_LOCKACCESS_TYPE                                      UInt32
#define CORTEXA7_CTI_LOCKACCESS_RESERVED_MASK                             0x00000000
#define    CORTEXA7_CTI_LOCKACCESS_LOCK_ACCESS_SHIFT                      0
#define    CORTEXA7_CTI_LOCKACCESS_LOCK_ACCESS_MASK                       0xFFFFFFFF

#define CORTEXA7_CTI_LOCKSTATUS_OFFSET                                    0x00000FB4
#define CORTEXA7_CTI_LOCKSTATUS_TYPE                                      UInt32
#define CORTEXA7_CTI_LOCKSTATUS_RESERVED_MASK                             0xFFFFFFFC
#define    CORTEXA7_CTI_LOCKSTATUS_LOCKED_SHIFT                           1
#define    CORTEXA7_CTI_LOCKSTATUS_LOCKED_MASK                            0x00000002
#define    CORTEXA7_CTI_LOCKSTATUS_LOCK_IMPLEMENTED_SHIFT                 0
#define    CORTEXA7_CTI_LOCKSTATUS_LOCK_IMPLEMENTED_MASK                  0x00000001

#define CORTEXA7_CTI_AUTHSTATUS_OFFSET                                    0x00000FB8
#define CORTEXA7_CTI_AUTHSTATUS_TYPE                                      UInt32
#define CORTEXA7_CTI_AUTHSTATUS_RESERVED_MASK                             0xFFFFFFF0
#define    CORTEXA7_CTI_AUTHSTATUS_OPEN_NON_DBG_SHIFT                     2
#define    CORTEXA7_CTI_AUTHSTATUS_OPEN_NON_DBG_MASK                      0x0000000C
#define    CORTEXA7_CTI_AUTHSTATUS_OPEN_INV_DBG_SHIFT                     0
#define    CORTEXA7_CTI_AUTHSTATUS_OPEN_INV_DBG_MASK                      0x00000003

#define CORTEXA7_CTI_DEVID_OFFSET                                         0x00000FC8
#define CORTEXA7_CTI_DEVID_TYPE                                           UInt32
#define CORTEXA7_CTI_DEVID_RESERVED_MASK                                  0xFFFFFFFF

#define CORTEXA7_CTI_DEVTYPE_OFFSET                                       0x00000FCC
#define CORTEXA7_CTI_DEVTYPE_TYPE                                         UInt32
#define CORTEXA7_CTI_DEVTYPE_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_DEVTYPE_SUB_TYPE_SHIFT                            4
#define    CORTEXA7_CTI_DEVTYPE_SUB_TYPE_MASK                             0x000000F0
#define    CORTEXA7_CTI_DEVTYPE_MAJOR_TYPE_SHIFT                          0
#define    CORTEXA7_CTI_DEVTYPE_MAJOR_TYPE_MASK                           0x0000000F

#define CORTEXA7_CTI_PERIPHID4_OFFSET                                     0x00000FD0
#define CORTEXA7_CTI_PERIPHID4_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID4_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_PERIPHID4_FOURKB_COUNT_SHIFT                      4
#define    CORTEXA7_CTI_PERIPHID4_FOURKB_COUNT_MASK                       0x000000F0
#define    CORTEXA7_CTI_PERIPHID4_JEP106_CONTINUATION_CODE_SHIFT          0
#define    CORTEXA7_CTI_PERIPHID4_JEP106_CONTINUATION_CODE_MASK           0x0000000F

#define CORTEXA7_CTI_PERIPHID5_OFFSET                                     0x00000FD4
#define CORTEXA7_CTI_PERIPHID5_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID5_RESERVED_MASK                              0x00000000
#define    CORTEXA7_CTI_PERIPHID5_RESERVED_SHIFT                          0
#define    CORTEXA7_CTI_PERIPHID5_RESERVED_MASK                           0xFFFFFFFF

#define CORTEXA7_CTI_PERIPHID6_OFFSET                                     0x00000FD8
#define CORTEXA7_CTI_PERIPHID6_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID6_RESERVED_MASK                              0x00000000
#define    CORTEXA7_CTI_PERIPHID6_RESERVED_SHIFT                          0
#define    CORTEXA7_CTI_PERIPHID6_RESERVED_MASK                           0xFFFFFFFF

#define CORTEXA7_CTI_PERIPHID7_OFFSET                                     0x00000FDC
#define CORTEXA7_CTI_PERIPHID7_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID7_RESERVED_MASK                              0x00000000
#define    CORTEXA7_CTI_PERIPHID7_RESERVED_SHIFT                          0
#define    CORTEXA7_CTI_PERIPHID7_RESERVED_MASK                           0xFFFFFFFF

#define CORTEXA7_CTI_PERIPHID0_OFFSET                                     0x00000FE0
#define CORTEXA7_CTI_PERIPHID0_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID0_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_PERIPHID0_PART_NUMBER_BITS7TO0_SHIFT              0
#define    CORTEXA7_CTI_PERIPHID0_PART_NUMBER_BITS7TO0_MASK               0x000000FF

#define CORTEXA7_CTI_PERIPHID1_OFFSET                                     0x00000FE4
#define CORTEXA7_CTI_PERIPHID1_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID1_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_PERIPHID1_JEP106_IDENTITY_CODE_BITS3TO0_SHIFT     4
#define    CORTEXA7_CTI_PERIPHID1_JEP106_IDENTITY_CODE_BITS3TO0_MASK      0x000000F0
#define    CORTEXA7_CTI_PERIPHID1_PART_NUMBER_BITS11TO8_SHIFT             0
#define    CORTEXA7_CTI_PERIPHID1_PART_NUMBER_BITS11TO8_MASK              0x0000000F

#define CORTEXA7_CTI_PERIPHID2_OFFSET                                     0x00000FE8
#define CORTEXA7_CTI_PERIPHID2_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID2_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_PERIPHID2_REVISION_SHIFT                          4
#define    CORTEXA7_CTI_PERIPHID2_REVISION_MASK                           0x000000F0
#define    CORTEXA7_CTI_PERIPHID2_JEDEC_SHIFT                             3
#define    CORTEXA7_CTI_PERIPHID2_JEDEC_MASK                              0x00000008
#define    CORTEXA7_CTI_PERIPHID2_JEP106_IDENTITY_CODE_BITS6TO4_SHIFT     0
#define    CORTEXA7_CTI_PERIPHID2_JEP106_IDENTITY_CODE_BITS6TO4_MASK      0x00000007

#define CORTEXA7_CTI_PERIPHID3_OFFSET                                     0x00000FEC
#define CORTEXA7_CTI_PERIPHID3_TYPE                                       UInt32
#define CORTEXA7_CTI_PERIPHID3_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_CTI_PERIPHID3_REVAND_SHIFT                            4
#define    CORTEXA7_CTI_PERIPHID3_REVAND_MASK                             0x000000F0
#define    CORTEXA7_CTI_PERIPHID3_CUSTOMER_MODIFIED_SHIFT                 0
#define    CORTEXA7_CTI_PERIPHID3_CUSTOMER_MODIFIED_MASK                  0x0000000F

#define CORTEXA7_CTI_COMPID0_OFFSET                                       0x00000FF0
#define CORTEXA7_CTI_COMPID0_TYPE                                         UInt32
#define CORTEXA7_CTI_COMPID0_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_COMPID0_PREAMBLE_SHIFT                            0
#define    CORTEXA7_CTI_COMPID0_PREAMBLE_MASK                             0x000000FF

#define CORTEXA7_CTI_COMPID1_OFFSET                                       0x00000FF4
#define CORTEXA7_CTI_COMPID1_TYPE                                         UInt32
#define CORTEXA7_CTI_COMPID1_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_COMPID1_COMPONENT_CLASS_SHIFT                     4
#define    CORTEXA7_CTI_COMPID1_COMPONENT_CLASS_MASK                      0x000000F0
#define    CORTEXA7_CTI_COMPID1_PREAMBLE_SHIFT                            0
#define    CORTEXA7_CTI_COMPID1_PREAMBLE_MASK                             0x0000000F

#define CORTEXA7_CTI_COMPID2_OFFSET                                       0x00000FF8
#define CORTEXA7_CTI_COMPID2_TYPE                                         UInt32
#define CORTEXA7_CTI_COMPID2_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_COMPID2_PREAMBLE_SHIFT                            0
#define    CORTEXA7_CTI_COMPID2_PREAMBLE_MASK                             0x000000FF

#define CORTEXA7_CTI_COMPID3_OFFSET                                       0x00000FFC
#define CORTEXA7_CTI_COMPID3_TYPE                                         UInt32
#define CORTEXA7_CTI_COMPID3_RESERVED_MASK                                0xFFFFFF00
#define    CORTEXA7_CTI_COMPID3_PREAMBLE_SHIFT                            0
#define    CORTEXA7_CTI_COMPID3_PREAMBLE_MASK                             0x000000FF

#endif /* __BRCM_RDB_CORTEXA7_CTI_H__ */


