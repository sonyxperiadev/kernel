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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_CTI_H__
#define __BRCM_RDB_CTI_H__

#define CTI_CTICONTROL_OFFSET                                             0x00000000
#define CTI_CTICONTROL_TYPE                                               UInt32
#define CTI_CTICONTROL_RESERVED_MASK                                      0xFFFFFFFE
#define    CTI_CTICONTROL_GLBEN_SHIFT                                     0
#define    CTI_CTICONTROL_GLBEN_MASK                                      0x00000001

#define CTI_CTIINTACK_OFFSET                                              0x00000010
#define CTI_CTIINTACK_TYPE                                                UInt32
#define CTI_CTIINTACK_RESERVED_MASK                                       0xFFFFFF00
#define    CTI_CTIINTACK_INTACK_SHIFT                                     0
#define    CTI_CTIINTACK_INTACK_MASK                                      0x000000FF

#define CTI_CTIAPPSET_OFFSET                                              0x00000014
#define CTI_CTIAPPSET_TYPE                                                UInt32
#define CTI_CTIAPPSET_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIAPPSET_APPSET_SHIFT                                     0
#define    CTI_CTIAPPSET_APPSET_MASK                                      0x0000000F

#define CTI_CTIAPPCLEAR_OFFSET                                            0x00000018
#define CTI_CTIAPPCLEAR_TYPE                                              UInt32
#define CTI_CTIAPPCLEAR_RESERVED_MASK                                     0xFFFFFFF0
#define    CTI_CTIAPPCLEAR_APPCLEAR_SHIFT                                 0
#define    CTI_CTIAPPCLEAR_APPCLEAR_MASK                                  0x0000000F

#define CTI_CTIAPPPULSE_OFFSET                                            0x0000001C
#define CTI_CTIAPPPULSE_TYPE                                              UInt32
#define CTI_CTIAPPPULSE_RESERVED_MASK                                     0xFFFFFFF0
#define    CTI_CTIAPPPULSE_APPULSE_SHIFT                                  0
#define    CTI_CTIAPPPULSE_APPULSE_MASK                                   0x0000000F

#define CTI_CTIINEN0_OFFSET                                               0x00000020
#define CTI_CTIINEN0_TYPE                                                 UInt32
#define CTI_CTIINEN0_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN0_TRIGINEN0_SHIFT                                   0
#define    CTI_CTIINEN0_TRIGINEN0_MASK                                    0x0000000F

#define CTI_CTIINEN1_OFFSET                                               0x00000024
#define CTI_CTIINEN1_TYPE                                                 UInt32
#define CTI_CTIINEN1_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN1_TRIGINEN1_SHIFT                                   0
#define    CTI_CTIINEN1_TRIGINEN1_MASK                                    0x0000000F

#define CTI_CTIINEN2_OFFSET                                               0x00000028
#define CTI_CTIINEN2_TYPE                                                 UInt32
#define CTI_CTIINEN2_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN2_TRIGINEN2_SHIFT                                   0
#define    CTI_CTIINEN2_TRIGINEN2_MASK                                    0x0000000F

#define CTI_CTIINEN3_OFFSET                                               0x0000002C
#define CTI_CTIINEN3_TYPE                                                 UInt32
#define CTI_CTIINEN3_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN3_TRIGINEN3_SHIFT                                   0
#define    CTI_CTIINEN3_TRIGINEN3_MASK                                    0x0000000F

#define CTI_CTIINEN4_OFFSET                                               0x00000030
#define CTI_CTIINEN4_TYPE                                                 UInt32
#define CTI_CTIINEN4_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN4_TRIGINEN4_SHIFT                                   0
#define    CTI_CTIINEN4_TRIGINEN4_MASK                                    0x0000000F

#define CTI_CTIINEN5_OFFSET                                               0x00000034
#define CTI_CTIINEN5_TYPE                                                 UInt32
#define CTI_CTIINEN5_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN5_TRIGINEN5_SHIFT                                   0
#define    CTI_CTIINEN5_TRIGINEN5_MASK                                    0x0000000F

#define CTI_CTIINEN6_OFFSET                                               0x00000038
#define CTI_CTIINEN6_TYPE                                                 UInt32
#define CTI_CTIINEN6_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN6_TRIGINEN6_SHIFT                                   0
#define    CTI_CTIINEN6_TRIGINEN6_MASK                                    0x0000000F

#define CTI_CTIINEN7_OFFSET                                               0x0000003C
#define CTI_CTIINEN7_TYPE                                                 UInt32
#define CTI_CTIINEN7_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CTIINEN7_TRIGINEN7_SHIFT                                   0
#define    CTI_CTIINEN7_TRIGINEN7_MASK                                    0x0000000F

#define CTI_CTIOUTEN0_OFFSET                                              0x000000A0
#define CTI_CTIOUTEN0_TYPE                                                UInt32
#define CTI_CTIOUTEN0_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN0_TRIGOUTEN0_SHIFT                                 0
#define    CTI_CTIOUTEN0_TRIGOUTEN0_MASK                                  0x0000000F

#define CTI_CTIOUTEN1_OFFSET                                              0x000000A4
#define CTI_CTIOUTEN1_TYPE                                                UInt32
#define CTI_CTIOUTEN1_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN1_TRIGOUTEN1_SHIFT                                 0
#define    CTI_CTIOUTEN1_TRIGOUTEN1_MASK                                  0x0000000F

#define CTI_CTIOUTEN2_OFFSET                                              0x000000A8
#define CTI_CTIOUTEN2_TYPE                                                UInt32
#define CTI_CTIOUTEN2_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN2_TRIGOUTEN2_SHIFT                                 0
#define    CTI_CTIOUTEN2_TRIGOUTEN2_MASK                                  0x0000000F

#define CTI_CTIOUTEN3_OFFSET                                              0x000000AC
#define CTI_CTIOUTEN3_TYPE                                                UInt32
#define CTI_CTIOUTEN3_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN3_TRIGOUTEN3_SHIFT                                 0
#define    CTI_CTIOUTEN3_TRIGOUTEN3_MASK                                  0x0000000F

#define CTI_CTIOUTEN4_OFFSET                                              0x000000B0
#define CTI_CTIOUTEN4_TYPE                                                UInt32
#define CTI_CTIOUTEN4_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN4_TRIGOUTEN4_SHIFT                                 0
#define    CTI_CTIOUTEN4_TRIGOUTEN4_MASK                                  0x0000000F

#define CTI_CTIOUTEN5_OFFSET                                              0x000000B4
#define CTI_CTIOUTEN5_TYPE                                                UInt32
#define CTI_CTIOUTEN5_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN5_TRIGOUTEN5_SHIFT                                 0
#define    CTI_CTIOUTEN5_TRIGOUTEN5_MASK                                  0x0000000F

#define CTI_CTIOUTEN6_OFFSET                                              0x000000B8
#define CTI_CTIOUTEN6_TYPE                                                UInt32
#define CTI_CTIOUTEN6_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN6_TRIGOUTEN6_SHIFT                                 0
#define    CTI_CTIOUTEN6_TRIGOUTEN6_MASK                                  0x0000000F

#define CTI_CTIOUTEN7_OFFSET                                              0x000000BC
#define CTI_CTIOUTEN7_TYPE                                                UInt32
#define CTI_CTIOUTEN7_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTIOUTEN7_TRIGOUTEN7_SHIFT                                 0
#define    CTI_CTIOUTEN7_TRIGOUTEN7_MASK                                  0x0000000F

#define CTI_CTITRIGINSTATUS_OFFSET                                        0x00000130
#define CTI_CTITRIGINSTATUS_TYPE                                          UInt32
#define CTI_CTITRIGINSTATUS_RESERVED_MASK                                 0xFFFFFF00
#define    CTI_CTITRIGINSTATUS_TRIGINSTATUS_SHIFT                         0
#define    CTI_CTITRIGINSTATUS_TRIGINSTATUS_MASK                          0x000000FF

#define CTI_CTITRIGOUTSTATUS_OFFSET                                       0x00000134
#define CTI_CTITRIGOUTSTATUS_TYPE                                         UInt32
#define CTI_CTITRIGOUTSTATUS_RESERVED_MASK                                0xFFFFFF00
#define    CTI_CTITRIGOUTSTATUS_TRIGOUTSTATUS_SHIFT                       0
#define    CTI_CTITRIGOUTSTATUS_TRIGOUTSTATUS_MASK                        0x000000FF

#define CTI_CTICHINSTATUS_OFFSET                                          0x00000138
#define CTI_CTICHINSTATUS_TYPE                                            UInt32
#define CTI_CTICHINSTATUS_RESERVED_MASK                                   0xFFFFFFF0
#define    CTI_CTICHINSTATUS_CHINSTATUS_SHIFT                             0
#define    CTI_CTICHINSTATUS_CHINSTATUS_MASK                              0x0000000F

#define CTI_CTICHOUTSTATUS_OFFSET                                         0x0000013C
#define CTI_CTICHOUTSTATUS_TYPE                                           UInt32
#define CTI_CTICHOUTSTATUS_RESERVED_MASK                                  0xFFFFFFF0
#define    CTI_CTICHOUTSTATUS_CHOUTSTATUS_SHIFT                           0
#define    CTI_CTICHOUTSTATUS_CHOUTSTATUS_MASK                            0x0000000F

#define CTI_CTICHGATE_OFFSET                                              0x00000140
#define CTI_CTICHGATE_TYPE                                                UInt32
#define CTI_CTICHGATE_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_CTICHGATE_CTICHGATE3_SHIFT                                 3
#define    CTI_CTICHGATE_CTICHGATE3_MASK                                  0x00000008
#define    CTI_CTICHGATE_CTICHGATE2_SHIFT                                 2
#define    CTI_CTICHGATE_CTICHGATE2_MASK                                  0x00000004
#define    CTI_CTICHGATE_CTICHGATE1_SHIFT                                 1
#define    CTI_CTICHGATE_CTICHGATE1_MASK                                  0x00000002
#define    CTI_CTICHGATE_CTICHGATE0_SHIFT                                 0
#define    CTI_CTICHGATE_CTICHGATE0_MASK                                  0x00000001

#define CTI_ASICCTL_OFFSET                                                0x00000144
#define CTI_ASICCTL_TYPE                                                  UInt32
#define CTI_ASICCTL_RESERVED_MASK                                         0xFFFFFFC0
#define    CTI_ASICCTL_PMUEXTIN1EDGE_SHIFT                                5
#define    CTI_ASICCTL_PMUEXTIN1EDGE_MASK                                 0x00000020
#define    CTI_ASICCTL_PMUEXTIN0EDGE_SHIFT                                4
#define    CTI_ASICCTL_PMUEXTIN0EDGE_MASK                                 0x00000010
#define    CTI_ASICCTL_ETMEXTIN4EDGE_SHIFT                                3
#define    CTI_ASICCTL_ETMEXTIN4EDGE_MASK                                 0x00000008
#define    CTI_ASICCTL_ETMEXTIN3EDGE_SHIFT                                2
#define    CTI_ASICCTL_ETMEXTIN3EDGE_MASK                                 0x00000004
#define    CTI_ASICCTL_ETMEXTIN2EDGE_SHIFT                                1
#define    CTI_ASICCTL_ETMEXTIN2EDGE_MASK                                 0x00000002
#define    CTI_ASICCTL_ETMEXTIN1EDGE_SHIFT                                0
#define    CTI_ASICCTL_ETMEXTIN1EDGE_MASK                                 0x00000001

#define CTI_ITCHINACK_OFFSET                                              0x00000EDC
#define CTI_ITCHINACK_TYPE                                                UInt32
#define CTI_ITCHINACK_RESERVED_MASK                                       0xFFFFFFF0
#define    CTI_ITCHINACK_CTCHINACK_SHIFT                                  0
#define    CTI_ITCHINACK_CTCHINACK_MASK                                   0x0000000F

#define CTI_ITTRIGINACK_OFFSET                                            0x00000EE0
#define CTI_ITTRIGINACK_TYPE                                              UInt32
#define CTI_ITTRIGINACK_RESERVED_MASK                                     0xFFFFFF00
#define    CTI_ITTRIGINACK_CTTRIGINACK_SHIFT                              0
#define    CTI_ITTRIGINACK_CTTRIGINACK_MASK                               0x000000FF

#define CTI_ITCHOUT_OFFSET                                                0x00000EE4
#define CTI_ITCHOUT_TYPE                                                  UInt32
#define CTI_ITCHOUT_RESERVED_MASK                                         0xFFFFFFF0
#define    CTI_ITCHOUT_CTCHOUT_SHIFT                                      0
#define    CTI_ITCHOUT_CTCHOUT_MASK                                       0x0000000F

#define CTI_ITTRIGOUT_OFFSET                                              0x00000EE8
#define CTI_ITTRIGOUT_TYPE                                                UInt32
#define CTI_ITTRIGOUT_RESERVED_MASK                                       0xFFFFFF00
#define    CTI_ITTRIGOUT_CTTRIGOUT_SHIFT                                  0
#define    CTI_ITTRIGOUT_CTTRIGOUT_MASK                                   0x000000FF

#define CTI_ITCHOUTACK_OFFSET                                             0x00000EEC
#define CTI_ITCHOUTACK_TYPE                                               UInt32
#define CTI_ITCHOUTACK_RESERVED_MASK                                      0xFFFFFFF0
#define    CTI_ITCHOUTACK_CTCHOUTACK_SHIFT                                0
#define    CTI_ITCHOUTACK_CTCHOUTACK_MASK                                 0x0000000F

#define CTI_ITTRIGOUTACK_OFFSET                                           0x00000EF0
#define CTI_ITTRIGOUTACK_TYPE                                             UInt32
#define CTI_ITTRIGOUTACK_RESERVED_MASK                                    0xFFFFFF00
#define    CTI_ITTRIGOUTACK_CTTRIGOUTACK_SHIFT                            0
#define    CTI_ITTRIGOUTACK_CTTRIGOUTACK_MASK                             0x000000FF

#define CTI_ITCHIN_OFFSET                                                 0x00000EF4
#define CTI_ITCHIN_TYPE                                                   UInt32
#define CTI_ITCHIN_RESERVED_MASK                                          0xFFFFFFF0
#define    CTI_ITCHIN_CTCHIN_SHIFT                                        0
#define    CTI_ITCHIN_CTCHIN_MASK                                         0x0000000F

#define CTI_ITTRIGIN_OFFSET                                               0x00000EF8
#define CTI_ITTRIGIN_TYPE                                                 UInt32
#define CTI_ITTRIGIN_RESERVED_MASK                                        0xFFFFFF00
#define    CTI_ITTRIGIN_CTTRIGIN_SHIFT                                    0
#define    CTI_ITTRIGIN_CTTRIGIN_MASK                                     0x000000FF

#define CTI_ICTRL_OFFSET                                                  0x00000F00
#define CTI_ICTRL_TYPE                                                    UInt32
#define CTI_ICTRL_RESERVED_MASK                                           0xFFFFFFFE
#define    CTI_ICTRL_INTEG_EN_SHIFT                                       0
#define    CTI_ICTRL_INTEG_EN_MASK                                        0x00000001

#define CTI_CLAIMSET_OFFSET                                               0x00000FA0
#define CTI_CLAIMSET_TYPE                                                 UInt32
#define CTI_CLAIMSET_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CLAIMSET_CLAIM_TAGSA_SHIFT                                 0
#define    CTI_CLAIMSET_CLAIM_TAGSA_MASK                                  0x0000000F

#define CTI_CLAIMCLR_OFFSET                                               0x00000FA4
#define CTI_CLAIMCLR_TYPE                                                 UInt32
#define CTI_CLAIMCLR_RESERVED_MASK                                        0xFFFFFFF0
#define    CTI_CLAIMCLR_CLAIM_TAGSB_SHIFT                                 0
#define    CTI_CLAIMCLR_CLAIM_TAGSB_MASK                                  0x0000000F

#define CTI_LOCKACCESS_OFFSET                                             0x00000FB0
#define CTI_LOCKACCESS_TYPE                                               UInt32
#define CTI_LOCKACCESS_RESERVED_MASK                                      0x00000000
#define    CTI_LOCKACCESS_LOCK_ACCESS_SHIFT                               0
#define    CTI_LOCKACCESS_LOCK_ACCESS_MASK                                0xFFFFFFFF

#define CTI_LOCKSTATUS_OFFSET                                             0x00000FB4
#define CTI_LOCKSTATUS_TYPE                                               UInt32
#define CTI_LOCKSTATUS_RESERVED_MASK                                      0xFFFFFFFC
#define    CTI_LOCKSTATUS_LOCKED_SHIFT                                    1
#define    CTI_LOCKSTATUS_LOCKED_MASK                                     0x00000002
#define    CTI_LOCKSTATUS_LOCK_IMPLEMENTED_SHIFT                          0
#define    CTI_LOCKSTATUS_LOCK_IMPLEMENTED_MASK                           0x00000001

#define CTI_AUTHSTATUS_OFFSET                                             0x00000FB8
#define CTI_AUTHSTATUS_TYPE                                               UInt32
#define CTI_AUTHSTATUS_RESERVED_MASK                                      0xFFFFFFF0
#define    CTI_AUTHSTATUS_OPEN_NON_DBG_SHIFT                              2
#define    CTI_AUTHSTATUS_OPEN_NON_DBG_MASK                               0x0000000C
#define    CTI_AUTHSTATUS_OPEN_INV_DBG_SHIFT                              0
#define    CTI_AUTHSTATUS_OPEN_INV_DBG_MASK                               0x00000003

#define CTI_DEVID_OFFSET                                                  0x00000FC8
#define CTI_DEVID_TYPE                                                    UInt32
#define CTI_DEVID_RESERVED_MASK                                           0xFFF000E0
#define    CTI_DEVID_CTI_CHANS_SHIFT                                      16
#define    CTI_DEVID_CTI_CHANS_MASK                                       0x000F0000
#define    CTI_DEVID_CTI_TRIGS_SHIFT                                      8
#define    CTI_DEVID_CTI_TRIGS_MASK                                       0x0000FF00
#define    CTI_DEVID_ASICCTL_SHIFT                                        0
#define    CTI_DEVID_ASICCTL_MASK                                         0x0000001F

#define CTI_DEVTYPE_OFFSET                                                0x00000FCC
#define CTI_DEVTYPE_TYPE                                                  UInt32
#define CTI_DEVTYPE_RESERVED_MASK                                         0xFFFFFF00
#define    CTI_DEVTYPE_SUB_TYPE_SHIFT                                     4
#define    CTI_DEVTYPE_SUB_TYPE_MASK                                      0x000000F0
#define    CTI_DEVTYPE_MAIN_CLASS_SHIFT                                   0
#define    CTI_DEVTYPE_MAIN_CLASS_MASK                                    0x0000000F

#define CTI_PERID4_OFFSET                                                 0x00000FD0
#define CTI_PERID4_TYPE                                                   UInt32
#define CTI_PERID4_RESERVED_MASK                                          0xFFFFFF00
#define    CTI_PERID4_PID4_SHIFT                                          0
#define    CTI_PERID4_PID4_MASK                                           0x000000FF

#define CTI_PERID0_OFFSET                                                 0x00000FE0
#define CTI_PERID0_TYPE                                                   UInt32
#define CTI_PERID0_RESERVED_MASK                                          0xFFFFFF00
#define    CTI_PERID0_PID0_SHIFT                                          0
#define    CTI_PERID0_PID0_MASK                                           0x000000FF

#define CTI_PERID1_OFFSET                                                 0x00000FE4
#define CTI_PERID1_TYPE                                                   UInt32
#define CTI_PERID1_RESERVED_MASK                                          0xFFFFFF00
#define    CTI_PERID1_PID1_SHIFT                                          0
#define    CTI_PERID1_PID1_MASK                                           0x000000FF

#define CTI_PERID2_OFFSET                                                 0x00000FE8
#define CTI_PERID2_TYPE                                                   UInt32
#define CTI_PERID2_RESERVED_MASK                                          0xFFFFFF00
#define    CTI_PERID2_PID2_SHIFT                                          0
#define    CTI_PERID2_PID2_MASK                                           0x000000FF

#define CTI_PERID3_OFFSET                                                 0x00000FEC
#define CTI_PERID3_TYPE                                                   UInt32
#define CTI_PERID3_RESERVED_MASK                                          0xFFFFFF00
#define    CTI_PERID3_PID3_SHIFT                                          0
#define    CTI_PERID3_PID3_MASK                                           0x000000FF

#define CTI_COMPID0_OFFSET                                                0x00000FF0
#define CTI_COMPID0_TYPE                                                  UInt32
#define CTI_COMPID0_RESERVED_MASK                                         0xFFFFFF00
#define    CTI_COMPID0_CID0_SHIFT                                         0
#define    CTI_COMPID0_CID0_MASK                                          0x000000FF

#define CTI_COMPID1_OFFSET                                                0x00000FF4
#define CTI_COMPID1_TYPE                                                  UInt32
#define CTI_COMPID1_RESERVED_MASK                                         0xFFFFFF00
#define    CTI_COMPID1_CID1_SHIFT                                         0
#define    CTI_COMPID1_CID1_MASK                                          0x000000FF

#define CTI_COMPID2_OFFSET                                                0x00000FF8
#define CTI_COMPID2_TYPE                                                  UInt32
#define CTI_COMPID2_RESERVED_MASK                                         0xFFFFFF00
#define    CTI_COMPID2_CID2_SHIFT                                         0
#define    CTI_COMPID2_CID2_MASK                                          0x000000FF

#define CTI_COMPID3_OFFSET                                                0x00000FFC
#define CTI_COMPID3_TYPE                                                  UInt32
#define CTI_COMPID3_RESERVED_MASK                                         0xFFFFFF00
#define    CTI_COMPID3_CID3_SHIFT                                         0
#define    CTI_COMPID3_CID3_MASK                                          0x000000FF

#endif /* __BRCM_RDB_CTI_H__ */


