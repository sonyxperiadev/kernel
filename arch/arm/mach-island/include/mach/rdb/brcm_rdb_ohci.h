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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_OHCI_H__
#define __BRCM_RDB_OHCI_H__

#define OHCI_HCREVISION_OFFSET                                            0x00000000
#define OHCI_HCREVISION_TYPE                                              UInt32
#define OHCI_HCREVISION_RESERVED_MASK                                     0x00000000
#define    OHCI_HCREVISION_REV_SHIFT                                      0
#define    OHCI_HCREVISION_REV_MASK                                       0xFFFFFFFF

#define OHCI_HCCONTROL_OFFSET                                             0x00000004
#define OHCI_HCCONTROL_TYPE                                               UInt32
#define OHCI_HCCONTROL_RESERVED_MASK                                      0xFFFFF800
#define    OHCI_HCCONTROL_RWE_SHIFT                                       10
#define    OHCI_HCCONTROL_RWE_MASK                                        0x00000400
#define    OHCI_HCCONTROL_RWC_SHIFT                                       9
#define    OHCI_HCCONTROL_RWC_MASK                                        0x00000200
#define    OHCI_HCCONTROL_IR_SHIFT                                        8
#define    OHCI_HCCONTROL_IR_MASK                                         0x00000100
#define    OHCI_HCCONTROL_HCFS_SHIFT                                      6
#define    OHCI_HCCONTROL_HCFS_MASK                                       0x000000C0
#define    OHCI_HCCONTROL_BLE_SHIFT                                       5
#define    OHCI_HCCONTROL_BLE_MASK                                        0x00000020
#define    OHCI_HCCONTROL_CLE_SHIFT                                       4
#define    OHCI_HCCONTROL_CLE_MASK                                        0x00000010
#define    OHCI_HCCONTROL_IE_SHIFT                                        3
#define    OHCI_HCCONTROL_IE_MASK                                         0x00000008
#define    OHCI_HCCONTROL_PLE_SHIFT                                       2
#define    OHCI_HCCONTROL_PLE_MASK                                        0x00000004
#define    OHCI_HCCONTROL_CBSR_SHIFT                                      0
#define    OHCI_HCCONTROL_CBSR_MASK                                       0x00000003

#define OHCI_HCCOMMANDSTATUS_OFFSET                                       0x00000008
#define OHCI_HCCOMMANDSTATUS_TYPE                                         UInt32
#define OHCI_HCCOMMANDSTATUS_RESERVED_MASK                                0xFFFCFFF0
#define    OHCI_HCCOMMANDSTATUS_SOC_SHIFT                                 16
#define    OHCI_HCCOMMANDSTATUS_SOC_MASK                                  0x00030000
#define    OHCI_HCCOMMANDSTATUS_OCR_SHIFT                                 3
#define    OHCI_HCCOMMANDSTATUS_OCR_MASK                                  0x00000008
#define    OHCI_HCCOMMANDSTATUS_BLF_SHIFT                                 2
#define    OHCI_HCCOMMANDSTATUS_BLF_MASK                                  0x00000004
#define    OHCI_HCCOMMANDSTATUS_CLF_SHIFT                                 1
#define    OHCI_HCCOMMANDSTATUS_CLF_MASK                                  0x00000002
#define    OHCI_HCCOMMANDSTATUS_HCR_SHIFT                                 0
#define    OHCI_HCCOMMANDSTATUS_HCR_MASK                                  0x00000001

#define OHCI_HCINTERRUPTSTATUS_OFFSET                                     0x0000000C
#define OHCI_HCINTERRUPTSTATUS_TYPE                                       UInt32
#define OHCI_HCINTERRUPTSTATUS_RESERVED_MASK                              0xBFFFFF80
#define    OHCI_HCINTERRUPTSTATUS_OC_SHIFT                                30
#define    OHCI_HCINTERRUPTSTATUS_OC_MASK                                 0x40000000
#define    OHCI_HCINTERRUPTSTATUS_RHSC_SHIFT                              6
#define    OHCI_HCINTERRUPTSTATUS_RHSC_MASK                               0x00000040
#define    OHCI_HCINTERRUPTSTATUS_FNO_SHIFT                               5
#define    OHCI_HCINTERRUPTSTATUS_FNO_MASK                                0x00000020
#define    OHCI_HCINTERRUPTSTATUS_UE_SHIFT                                4
#define    OHCI_HCINTERRUPTSTATUS_UE_MASK                                 0x00000010
#define    OHCI_HCINTERRUPTSTATUS_RD_SHIFT                                3
#define    OHCI_HCINTERRUPTSTATUS_RD_MASK                                 0x00000008
#define    OHCI_HCINTERRUPTSTATUS_SF_SHIFT                                2
#define    OHCI_HCINTERRUPTSTATUS_SF_MASK                                 0x00000004
#define    OHCI_HCINTERRUPTSTATUS_WDH_SHIFT                               1
#define    OHCI_HCINTERRUPTSTATUS_WDH_MASK                                0x00000002
#define    OHCI_HCINTERRUPTSTATUS_SO_SHIFT                                0
#define    OHCI_HCINTERRUPTSTATUS_SO_MASK                                 0x00000001

#define OHCI_HCINTERRUPTENABLE_OFFSET                                     0x00000010
#define OHCI_HCINTERRUPTENABLE_TYPE                                       UInt32
#define OHCI_HCINTERRUPTENABLE_RESERVED_MASK                              0x3FFFFF80
#define    OHCI_HCINTERRUPTENABLE_MIE_SHIFT                               31
#define    OHCI_HCINTERRUPTENABLE_MIE_MASK                                0x80000000
#define    OHCI_HCINTERRUPTENABLE_OC_SHIFT                                30
#define    OHCI_HCINTERRUPTENABLE_OC_MASK                                 0x40000000
#define    OHCI_HCINTERRUPTENABLE_RHSC_SHIFT                              6
#define    OHCI_HCINTERRUPTENABLE_RHSC_MASK                               0x00000040
#define    OHCI_HCINTERRUPTENABLE_FNO_SHIFT                               5
#define    OHCI_HCINTERRUPTENABLE_FNO_MASK                                0x00000020
#define    OHCI_HCINTERRUPTENABLE_UE_SHIFT                                4
#define    OHCI_HCINTERRUPTENABLE_UE_MASK                                 0x00000010
#define    OHCI_HCINTERRUPTENABLE_RD_SHIFT                                3
#define    OHCI_HCINTERRUPTENABLE_RD_MASK                                 0x00000008
#define    OHCI_HCINTERRUPTENABLE_SF_SHIFT                                2
#define    OHCI_HCINTERRUPTENABLE_SF_MASK                                 0x00000004
#define    OHCI_HCINTERRUPTENABLE_WDH_SHIFT                               1
#define    OHCI_HCINTERRUPTENABLE_WDH_MASK                                0x00000002
#define    OHCI_HCINTERRUPTENABLE_SO_SHIFT                                0
#define    OHCI_HCINTERRUPTENABLE_SO_MASK                                 0x00000001

#define OHCI_HCINTERRUPTDISABLE_OFFSET                                    0x00000014
#define OHCI_HCINTERRUPTDISABLE_TYPE                                      UInt32
#define OHCI_HCINTERRUPTDISABLE_RESERVED_MASK                             0x3FFFFF80
#define    OHCI_HCINTERRUPTDISABLE_MIE_SHIFT                              31
#define    OHCI_HCINTERRUPTDISABLE_MIE_MASK                               0x80000000
#define    OHCI_HCINTERRUPTDISABLE_OC_SHIFT                               30
#define    OHCI_HCINTERRUPTDISABLE_OC_MASK                                0x40000000
#define    OHCI_HCINTERRUPTDISABLE_RHSC_SHIFT                             6
#define    OHCI_HCINTERRUPTDISABLE_RHSC_MASK                              0x00000040
#define    OHCI_HCINTERRUPTDISABLE_FNO_SHIFT                              5
#define    OHCI_HCINTERRUPTDISABLE_FNO_MASK                               0x00000020
#define    OHCI_HCINTERRUPTDISABLE_UE_SHIFT                               4
#define    OHCI_HCINTERRUPTDISABLE_UE_MASK                                0x00000010
#define    OHCI_HCINTERRUPTDISABLE_RD_SHIFT                               3
#define    OHCI_HCINTERRUPTDISABLE_RD_MASK                                0x00000008
#define    OHCI_HCINTERRUPTDISABLE_SF_SHIFT                               2
#define    OHCI_HCINTERRUPTDISABLE_SF_MASK                                0x00000004
#define    OHCI_HCINTERRUPTDISABLE_WDH_SHIFT                              1
#define    OHCI_HCINTERRUPTDISABLE_WDH_MASK                               0x00000002
#define    OHCI_HCINTERRUPTDISABLE_SO_SHIFT                               0
#define    OHCI_HCINTERRUPTDISABLE_SO_MASK                                0x00000001

#define OHCI_HCHCCA_OFFSET                                                0x00000018
#define OHCI_HCHCCA_TYPE                                                  UInt32
#define OHCI_HCHCCA_RESERVED_MASK                                         0x000000FF
#define    OHCI_HCHCCA_HCCA_SHIFT                                         8
#define    OHCI_HCHCCA_HCCA_MASK                                          0xFFFFFF00

#define OHCI_HCPERIODCURRENTED_OFFSET                                     0x0000001C
#define OHCI_HCPERIODCURRENTED_TYPE                                       UInt32
#define OHCI_HCPERIODCURRENTED_RESERVED_MASK                              0x0000000F
#define    OHCI_HCPERIODCURRENTED_PCED_SHIFT                              4
#define    OHCI_HCPERIODCURRENTED_PCED_MASK                               0xFFFFFFF0

#define OHCI_HCCONTROLHEADED_OFFSET                                       0x00000020
#define OHCI_HCCONTROLHEADED_TYPE                                         UInt32
#define OHCI_HCCONTROLHEADED_RESERVED_MASK                                0x0000000F
#define    OHCI_HCCONTROLHEADED_CHED_SHIFT                                4
#define    OHCI_HCCONTROLHEADED_CHED_MASK                                 0xFFFFFFF0

#define OHCI_HCCONTROLCURRENTED_OFFSET                                    0x00000024
#define OHCI_HCCONTROLCURRENTED_TYPE                                      UInt32
#define OHCI_HCCONTROLCURRENTED_RESERVED_MASK                             0x0000000F
#define    OHCI_HCCONTROLCURRENTED_CCED_SHIFT                             4
#define    OHCI_HCCONTROLCURRENTED_CCED_MASK                              0xFFFFFFF0

#define OHCI_HCBULKHEADED_OFFSET                                          0x00000028
#define OHCI_HCBULKHEADED_TYPE                                            UInt32
#define OHCI_HCBULKHEADED_RESERVED_MASK                                   0x0000000F
#define    OHCI_HCBULKHEADED_BHED_SHIFT                                   4
#define    OHCI_HCBULKHEADED_BHED_MASK                                    0xFFFFFFF0

#define OHCI_HCBULKCURRENTED_OFFSET                                       0x0000002C
#define OHCI_HCBULKCURRENTED_TYPE                                         UInt32
#define OHCI_HCBULKCURRENTED_RESERVED_MASK                                0x0000000F
#define    OHCI_HCBULKCURRENTED_BCED_SHIFT                                4
#define    OHCI_HCBULKCURRENTED_BCED_MASK                                 0xFFFFFFF0

#define OHCI_HCDONEHEAD_OFFSET                                            0x00000030
#define OHCI_HCDONEHEAD_TYPE                                              UInt32
#define OHCI_HCDONEHEAD_RESERVED_MASK                                     0x0000000F
#define    OHCI_HCDONEHEAD_DH_SHIFT                                       4
#define    OHCI_HCDONEHEAD_DH_MASK                                        0xFFFFFFF0

#define OHCI_HCFMINTERVAL_OFFSET                                          0x00000034
#define OHCI_HCFMINTERVAL_TYPE                                            UInt32
#define OHCI_HCFMINTERVAL_RESERVED_MASK                                   0x0000C000
#define    OHCI_HCFMINTERVAL_FIT_SHIFT                                    31
#define    OHCI_HCFMINTERVAL_FIT_MASK                                     0x80000000
#define    OHCI_HCFMINTERVAL_FSMPS_SHIFT                                  16
#define    OHCI_HCFMINTERVAL_FSMPS_MASK                                   0x7FFF0000
#define    OHCI_HCFMINTERVAL_FI_SHIFT                                     0
#define    OHCI_HCFMINTERVAL_FI_MASK                                      0x00003FFF

#define OHCI_HCFMREMAINING_OFFSET                                         0x00000038
#define OHCI_HCFMREMAINING_TYPE                                           UInt32
#define OHCI_HCFMREMAINING_RESERVED_MASK                                  0x7FFFC000
#define    OHCI_HCFMREMAINING_FRT_SHIFT                                   31
#define    OHCI_HCFMREMAINING_FRT_MASK                                    0x80000000
#define    OHCI_HCFMREMAINING_FR_SHIFT                                    0
#define    OHCI_HCFMREMAINING_FR_MASK                                     0x00003FFF

#define OHCI_HCFMNUMBER_OFFSET                                            0x0000003C
#define OHCI_HCFMNUMBER_TYPE                                              UInt32
#define OHCI_HCFMNUMBER_RESERVED_MASK                                     0xFFFF0000
#define    OHCI_HCFMNUMBER_FN_SHIFT                                       0
#define    OHCI_HCFMNUMBER_FN_MASK                                        0x0000FFFF

#define OHCI_HCPERIODICSTART_OFFSET                                       0x00000040
#define OHCI_HCPERIODICSTART_TYPE                                         UInt32
#define OHCI_HCPERIODICSTART_RESERVED_MASK                                0xFFFFC000
#define    OHCI_HCPERIODICSTART_PS_SHIFT                                  0
#define    OHCI_HCPERIODICSTART_PS_MASK                                   0x00003FFF

#define OHCI_HCLSTHRESHOLD_OFFSET                                         0x00000044
#define OHCI_HCLSTHRESHOLD_TYPE                                           UInt32
#define OHCI_HCLSTHRESHOLD_RESERVED_MASK                                  0xFFFFF000
#define    OHCI_HCLSTHRESHOLD_LST_SHIFT                                   0
#define    OHCI_HCLSTHRESHOLD_LST_MASK                                    0x00000FFF

#define OHCI_HCRHDESCRIPTORA_OFFSET                                       0x00000048
#define OHCI_HCRHDESCRIPTORA_TYPE                                         UInt32
#define OHCI_HCRHDESCRIPTORA_RESERVED_MASK                                0x00FFE000
#define    OHCI_HCRHDESCRIPTORA_POTPGT_SHIFT                              24
#define    OHCI_HCRHDESCRIPTORA_POTPGT_MASK                               0xFF000000
#define    OHCI_HCRHDESCRIPTORA_NOCP_SHIFT                                12
#define    OHCI_HCRHDESCRIPTORA_NOCP_MASK                                 0x00001000
#define    OHCI_HCRHDESCRIPTORA_OCPM_SHIFT                                11
#define    OHCI_HCRHDESCRIPTORA_OCPM_MASK                                 0x00000800
#define    OHCI_HCRHDESCRIPTORA_DT_SHIFT                                  10
#define    OHCI_HCRHDESCRIPTORA_DT_MASK                                   0x00000400
#define    OHCI_HCRHDESCRIPTORA_NPS_SHIFT                                 9
#define    OHCI_HCRHDESCRIPTORA_NPS_MASK                                  0x00000200
#define    OHCI_HCRHDESCRIPTORA_PSM_SHIFT                                 8
#define    OHCI_HCRHDESCRIPTORA_PSM_MASK                                  0x00000100
#define    OHCI_HCRHDESCRIPTORA_NDP_SHIFT                                 0
#define    OHCI_HCRHDESCRIPTORA_NDP_MASK                                  0x000000FF

#define OHCI_HCRHDESCRIPTORB_OFFSET                                       0x0000004C
#define OHCI_HCRHDESCRIPTORB_TYPE                                         UInt32
#define OHCI_HCRHDESCRIPTORB_RESERVED_MASK                                0x00000000
#define    OHCI_HCRHDESCRIPTORB_PPCM_SHIFT                                16
#define    OHCI_HCRHDESCRIPTORB_PPCM_MASK                                 0xFFFF0000
#define    OHCI_HCRHDESCRIPTORB_DR_SHIFT                                  0
#define    OHCI_HCRHDESCRIPTORB_DR_MASK                                   0x0000FFFF

#define OHCI_HCRHSTATUS_OFFSET                                            0x00000050
#define OHCI_HCRHSTATUS_TYPE                                              UInt32
#define OHCI_HCRHSTATUS_RESERVED_MASK                                     0x7FFC7FFC
#define    OHCI_HCRHSTATUS_CRWE_SHIFT                                     31
#define    OHCI_HCRHSTATUS_CRWE_MASK                                      0x80000000
#define    OHCI_HCRHSTATUS_OCIC_SHIFT                                     17
#define    OHCI_HCRHSTATUS_OCIC_MASK                                      0x00020000
#define    OHCI_HCRHSTATUS_LPSC_SHIFT                                     16
#define    OHCI_HCRHSTATUS_LPSC_MASK                                      0x00010000
#define    OHCI_HCRHSTATUS_DRWE_SHIFT                                     15
#define    OHCI_HCRHSTATUS_DRWE_MASK                                      0x00008000
#define    OHCI_HCRHSTATUS_OCI_SHIFT                                      1
#define    OHCI_HCRHSTATUS_OCI_MASK                                       0x00000002
#define    OHCI_HCRHSTATUS_LPS_SHIFT                                      0
#define    OHCI_HCRHSTATUS_LPS_MASK                                       0x00000001

#define OHCI_HCRHPORTSTATUS1_OFFSET                                       0x00000054
#define OHCI_HCRHPORTSTATUS1_TYPE                                         UInt32
#define OHCI_HCRHPORTSTATUS1_RESERVED_MASK                                0xFFE0FCE0
#define    OHCI_HCRHPORTSTATUS1_PRSC_SHIFT                                20
#define    OHCI_HCRHPORTSTATUS1_PRSC_MASK                                 0x00100000
#define    OHCI_HCRHPORTSTATUS1_OCIC_SHIFT                                19
#define    OHCI_HCRHPORTSTATUS1_OCIC_MASK                                 0x00080000
#define    OHCI_HCRHPORTSTATUS1_PSSC_SHIFT                                18
#define    OHCI_HCRHPORTSTATUS1_PSSC_MASK                                 0x00040000
#define    OHCI_HCRHPORTSTATUS1_PESC_SHIFT                                17
#define    OHCI_HCRHPORTSTATUS1_PESC_MASK                                 0x00020000
#define    OHCI_HCRHPORTSTATUS1_CSC_SHIFT                                 16
#define    OHCI_HCRHPORTSTATUS1_CSC_MASK                                  0x00010000
#define    OHCI_HCRHPORTSTATUS1_LSDA_SHIFT                                9
#define    OHCI_HCRHPORTSTATUS1_LSDA_MASK                                 0x00000200
#define    OHCI_HCRHPORTSTATUS1_PPS_SHIFT                                 8
#define    OHCI_HCRHPORTSTATUS1_PPS_MASK                                  0x00000100
#define    OHCI_HCRHPORTSTATUS1_PRS_SHIFT                                 4
#define    OHCI_HCRHPORTSTATUS1_PRS_MASK                                  0x00000010
#define    OHCI_HCRHPORTSTATUS1_POCI_SHIFT                                3
#define    OHCI_HCRHPORTSTATUS1_POCI_MASK                                 0x00000008
#define    OHCI_HCRHPORTSTATUS1_PSS_SHIFT                                 2
#define    OHCI_HCRHPORTSTATUS1_PSS_MASK                                  0x00000004
#define    OHCI_HCRHPORTSTATUS1_PES_SHIFT                                 1
#define    OHCI_HCRHPORTSTATUS1_PES_MASK                                  0x00000002
#define    OHCI_HCRHPORTSTATUS1_CCS_SHIFT                                 0
#define    OHCI_HCRHPORTSTATUS1_CCS_MASK                                  0x00000001

#define OHCI_HCRHPORTSTATUS2_OFFSET                                       0x00000058
#define OHCI_HCRHPORTSTATUS2_TYPE                                         UInt32
#define OHCI_HCRHPORTSTATUS2_RESERVED_MASK                                0xFFE0FCE0
#define    OHCI_HCRHPORTSTATUS2_PRSC_SHIFT                                20
#define    OHCI_HCRHPORTSTATUS2_PRSC_MASK                                 0x00100000
#define    OHCI_HCRHPORTSTATUS2_OCIC_SHIFT                                19
#define    OHCI_HCRHPORTSTATUS2_OCIC_MASK                                 0x00080000
#define    OHCI_HCRHPORTSTATUS2_PSSC_SHIFT                                18
#define    OHCI_HCRHPORTSTATUS2_PSSC_MASK                                 0x00040000
#define    OHCI_HCRHPORTSTATUS2_PESC_SHIFT                                17
#define    OHCI_HCRHPORTSTATUS2_PESC_MASK                                 0x00020000
#define    OHCI_HCRHPORTSTATUS2_CSC_SHIFT                                 16
#define    OHCI_HCRHPORTSTATUS2_CSC_MASK                                  0x00010000
#define    OHCI_HCRHPORTSTATUS2_LSDA_SHIFT                                9
#define    OHCI_HCRHPORTSTATUS2_LSDA_MASK                                 0x00000200
#define    OHCI_HCRHPORTSTATUS2_PPS_SHIFT                                 8
#define    OHCI_HCRHPORTSTATUS2_PPS_MASK                                  0x00000100
#define    OHCI_HCRHPORTSTATUS2_PRS_SHIFT                                 4
#define    OHCI_HCRHPORTSTATUS2_PRS_MASK                                  0x00000010
#define    OHCI_HCRHPORTSTATUS2_POCI_SHIFT                                3
#define    OHCI_HCRHPORTSTATUS2_POCI_MASK                                 0x00000008
#define    OHCI_HCRHPORTSTATUS2_PSS_SHIFT                                 2
#define    OHCI_HCRHPORTSTATUS2_PSS_MASK                                  0x00000004
#define    OHCI_HCRHPORTSTATUS2_PES_SHIFT                                 1
#define    OHCI_HCRHPORTSTATUS2_PES_MASK                                  0x00000002
#define    OHCI_HCRHPORTSTATUS2_CCS_SHIFT                                 0
#define    OHCI_HCRHPORTSTATUS2_CCS_MASK                                  0x00000001

#define OHCI_HCRHPORTSTATUS3_OFFSET                                       0x0000005C
#define OHCI_HCRHPORTSTATUS3_TYPE                                         UInt32
#define OHCI_HCRHPORTSTATUS3_RESERVED_MASK                                0xFFE0FCE0
#define    OHCI_HCRHPORTSTATUS3_PRSC_SHIFT                                20
#define    OHCI_HCRHPORTSTATUS3_PRSC_MASK                                 0x00100000
#define    OHCI_HCRHPORTSTATUS3_OCIC_SHIFT                                19
#define    OHCI_HCRHPORTSTATUS3_OCIC_MASK                                 0x00080000
#define    OHCI_HCRHPORTSTATUS3_PSSC_SHIFT                                18
#define    OHCI_HCRHPORTSTATUS3_PSSC_MASK                                 0x00040000
#define    OHCI_HCRHPORTSTATUS3_PESC_SHIFT                                17
#define    OHCI_HCRHPORTSTATUS3_PESC_MASK                                 0x00020000
#define    OHCI_HCRHPORTSTATUS3_CSC_SHIFT                                 16
#define    OHCI_HCRHPORTSTATUS3_CSC_MASK                                  0x00010000
#define    OHCI_HCRHPORTSTATUS3_LSDA_SHIFT                                9
#define    OHCI_HCRHPORTSTATUS3_LSDA_MASK                                 0x00000200
#define    OHCI_HCRHPORTSTATUS3_PPS_SHIFT                                 8
#define    OHCI_HCRHPORTSTATUS3_PPS_MASK                                  0x00000100
#define    OHCI_HCRHPORTSTATUS3_PRS_SHIFT                                 4
#define    OHCI_HCRHPORTSTATUS3_PRS_MASK                                  0x00000010
#define    OHCI_HCRHPORTSTATUS3_POCI_SHIFT                                3
#define    OHCI_HCRHPORTSTATUS3_POCI_MASK                                 0x00000008
#define    OHCI_HCRHPORTSTATUS3_PSS_SHIFT                                 2
#define    OHCI_HCRHPORTSTATUS3_PSS_MASK                                  0x00000004
#define    OHCI_HCRHPORTSTATUS3_PES_SHIFT                                 1
#define    OHCI_HCRHPORTSTATUS3_PES_MASK                                  0x00000002
#define    OHCI_HCRHPORTSTATUS3_CCS_SHIFT                                 0
#define    OHCI_HCRHPORTSTATUS3_CCS_MASK                                  0x00000001

#endif /* __BRCM_RDB_OHCI_H__ */


