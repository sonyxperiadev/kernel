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

#ifndef __BRCM_RDB_PMU_CPD_H__
#define __BRCM_RDB_PMU_CPD_H__

#define PMU_CPD_PDCMPSYN0_OFFSET                                          0x00000000
#define PMU_CPD_PDCMPSYN0_TYPE                                            UInt32
#define PMU_CPD_PDCMPSYN0_RESERVED_MASK                                   0xFF400000
#define    PMU_CPD_PDCMPSYN0_SYN_UBMBC_SHIFT                              23
#define    PMU_CPD_PDCMPSYN0_SYN_UBMBC_MASK                               0x00800000
#define    PMU_CPD_PDCMPSYN0_SYN_ERC_SHIFT                                21
#define    PMU_CPD_PDCMPSYN0_SYN_ERC_MASK                                 0x00200000
#define    PMU_CPD_PDCMPSYN0_SYN_USBOV_SHIFT                              20
#define    PMU_CPD_PDCMPSYN0_SYN_USBOV_MASK                               0x00100000
#define    PMU_CPD_PDCMPSYN0_SYN_MBOV_SHIFT                               19
#define    PMU_CPD_PDCMPSYN0_SYN_MBOV_MASK                                0x00080000
#define    PMU_CPD_PDCMPSYN0_MBTEMPLOW_SHIFT                              18
#define    PMU_CPD_PDCMPSYN0_MBTEMPLOW_MASK                               0x00040000
#define    PMU_CPD_PDCMPSYN0_MBTEMPHIGH_SHIFT                             17
#define    PMU_CPD_PDCMPSYN0_MBTEMPHIGH_MASK                              0x00020000
#define    PMU_CPD_PDCMPSYN0_SYN_MBPD_SHIFT                               16
#define    PMU_CPD_PDCMPSYN0_SYN_MBPD_MASK                                0x00010000
#define    PMU_CPD_PDCMPSYN0_SYN_MBUV_SHIFT                               15
#define    PMU_CPD_PDCMPSYN0_SYN_MBUV_MASK                                0x00008000
#define    PMU_CPD_PDCMPSYN0_SYN_CGPD_SHIFT                               14
#define    PMU_CPD_PDCMPSYN0_SYN_CGPD_MASK                                0x00004000
#define    PMU_CPD_PDCMPSYN0_SYN_TOOHOT_SHIFT                             13
#define    PMU_CPD_PDCMPSYN0_SYN_TOOHOT_MASK                              0x00002000
#define    PMU_CPD_PDCMPSYN0_SYN_PC2_SHIFT                                12
#define    PMU_CPD_PDCMPSYN0_SYN_PC2_MASK                                 0x00001000
#define    PMU_CPD_PDCMPSYN0_SYN_PC1_SHIFT                                11
#define    PMU_CPD_PDCMPSYN0_SYN_PC1_MASK                                 0x00000800
#define    PMU_CPD_PDCMPSYN0_SYN_UBPD_SHIFT                               10
#define    PMU_CPD_PDCMPSYN0_SYN_UBPD_MASK                                0x00000400
#define    PMU_CPD_PDCMPSYN0_SYN_MBMC_SHIFT                               9
#define    PMU_CPD_PDCMPSYN0_SYN_MBMC_MASK                                0x00000200
#define    PMU_CPD_PDCMPSYN0_BSIWV_SHIFT                                  8
#define    PMU_CPD_PDCMPSYN0_BSIWV_MASK                                   0x00000100
#define    PMU_CPD_PDCMPSYN0_MBWV_DELTA_SHIFT                             7
#define    PMU_CPD_PDCMPSYN0_MBWV_DELTA_MASK                              0x00000080
#define    PMU_CPD_PDCMPSYN0_CGPD_ENV_SHIFT                               6
#define    PMU_CPD_PDCMPSYN0_CGPD_ENV_MASK                                0x00000040
#define    PMU_CPD_PDCMPSYN0_UBPD_ENV_SHIFT                               5
#define    PMU_CPD_PDCMPSYN0_UBPD_ENV_MASK                                0x00000020
#define    PMU_CPD_PDCMPSYN0_SYN_MBWV_SHIFT                               4
#define    PMU_CPD_PDCMPSYN0_SYN_MBWV_MASK                                0x00000010
#define    PMU_CPD_PDCMPSYN0_SYN_CGMBC_SHIFT                              3
#define    PMU_CPD_PDCMPSYN0_SYN_CGMBC_MASK                               0x00000008
#define    PMU_CPD_PDCMPSYN0_SYN_CHGOV_SHIFT                              2
#define    PMU_CPD_PDCMPSYN0_SYN_CHGOV_MASK                               0x00000004
#define    PMU_CPD_PDCMPSYN0_SYN_VBCB_SHIFT                               1
#define    PMU_CPD_PDCMPSYN0_SYN_VBCB_MASK                                0x00000002
#define    PMU_CPD_PDCMPSYN0_SYN_BBLOWB_SHIFT                             0
#define    PMU_CPD_PDCMPSYN0_SYN_BBLOWB_MASK                              0x00000001

#define PMU_CPD_PDCMPSYN1_OFFSET                                          0x00000004
#define PMU_CPD_PDCMPSYN1_TYPE                                            UInt32
#define PMU_CPD_PDCMPSYN1_RESERVED_MASK                                   0xFFE02007
#define    PMU_CPD_PDCMPSYN1_SYN_PLL_LOCK_SHIFT                           20
#define    PMU_CPD_PDCMPSYN1_SYN_PLL_LOCK_MASK                            0x00100000
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_POK_SHIFT                            19
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_POK_MASK                             0x00080000
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_POK_SHIFT                           18
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_POK_MASK                            0x00040000
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_CALDONE_SHIFT                        17
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_CALDONE_MASK                         0x00020000
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_CALDONE_SHIFT                       16
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_CALDONE_MASK                        0x00010000
#define    PMU_CPD_PDCMPSYN1_SYN_AUXON_N_SHIFT                            15
#define    PMU_CPD_PDCMPSYN1_SYN_AUXON_N_MASK                             0x00008000
#define    PMU_CPD_PDCMPSYN1_SYN_PONKEY_N_SHIFT                           14
#define    PMU_CPD_PDCMPSYN1_SYN_PONKEY_N_MASK                            0x00004000
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_OVERI_SHIFT                          12
#define    PMU_CPD_PDCMPSYN1_SYN_CSR_OVERI_MASK                           0x00001000
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_OVERI_SHIFT                         11
#define    PMU_CPD_PDCMPSYN1_SYN_IOSR_OVERI_MASK                          0x00000800
#define    PMU_CPD_PDCMPSYN1_SYN_CAM_OVERI_SHIFT                          10
#define    PMU_CPD_PDCMPSYN1_SYN_CAM_OVERI_MASK                           0x00000400
#define    PMU_CPD_PDCMPSYN1_SYN_SIM2_OVERI_SHIFT                         9
#define    PMU_CPD_PDCMPSYN1_SYN_SIM2_OVERI_MASK                          0x00000200
#define    PMU_CPD_PDCMPSYN1_SYN_SIM1_OVERI_SHIFT                         8
#define    PMU_CPD_PDCMPSYN1_SYN_SIM1_OVERI_MASK                          0x00000100
#define    PMU_CPD_PDCMPSYN1_SYN_DVS1_OVERI_SHIFT                         7
#define    PMU_CPD_PDCMPSYN1_SYN_DVS1_OVERI_MASK                          0x00000080
#define    PMU_CPD_PDCMPSYN1_SYN_DVS2_OVERI_SHIFT                         6
#define    PMU_CPD_PDCMPSYN1_SYN_DVS2_OVERI_MASK                          0x00000040
#define    PMU_CPD_PDCMPSYN1_SYN_HV2_OVERI_SHIFT                          5
#define    PMU_CPD_PDCMPSYN1_SYN_HV2_OVERI_MASK                           0x00000020
#define    PMU_CPD_PDCMPSYN1_SYN_HV1_OVERI_SHIFT                          4
#define    PMU_CPD_PDCMPSYN1_SYN_HV1_OVERI_MASK                           0x00000010
#define    PMU_CPD_PDCMPSYN1_SYN_RF_OVERI_SHIFT                           3
#define    PMU_CPD_PDCMPSYN1_SYN_RF_OVERI_MASK                            0x00000008

#define PMU_CPD_PDCMPSYN2_OFFSET                                          0x00000008
#define PMU_CPD_PDCMPSYN2_TYPE                                            UInt32
#define PMU_CPD_PDCMPSYN2_RESERVED_MASK                                   0xFFFFFFFD
#define    PMU_CPD_PDCMPSYN2_FGC_SHIFT                                    1
#define    PMU_CPD_PDCMPSYN2_FGC_MASK                                     0x00000002

#define PMU_CPD_DIGTESTMUX_OFFSET                                         0x00000024
#define PMU_CPD_DIGTESTMUX_TYPE                                           UInt32
#define PMU_CPD_DIGTESTMUX_RESERVED_MASK                                  0xE0FCC080
#define    PMU_CPD_DIGTESTMUX_DTMUX_SEL_4_0__SHIFT                        24
#define    PMU_CPD_DIGTESTMUX_DTMUX_SEL_4_0__MASK                         0x1F000000
#define    PMU_CPD_DIGTESTMUX_DIG_TST_MUXEN_1_0__SHIFT                    16
#define    PMU_CPD_DIGTESTMUX_DIG_TST_MUXEN_1_0__MASK                     0x00030000
#define    PMU_CPD_DIGTESTMUX_DIGTESTMUX2_5_0__SHIFT                      8
#define    PMU_CPD_DIGTESTMUX_DIGTESTMUX2_5_0__MASK                       0x00003F00
#define    PMU_CPD_DIGTESTMUX_DIGTESTMUX1_6_0__SHIFT                      0
#define    PMU_CPD_DIGTESTMUX_DIGTESTMUX1_6_0__MASK                       0x0000007F

#define PMU_CPD_TMODEIFMUX_OFFSET                                         0x00000028
#define PMU_CPD_TMODEIFMUX_TYPE                                           UInt32
#define PMU_CPD_TMODEIFMUX_RESERVED_MASK                                  0xFFFF0000
#define    PMU_CPD_TMODEIFMUX_TYPE2_MUXEN_SHIFT                           15
#define    PMU_CPD_TMODEIFMUX_TYPE2_MUXEN_MASK                            0x00008000
#define    PMU_CPD_TMODEIFMUX_TYPE1_MUXEN_SHIFT                           14
#define    PMU_CPD_TMODEIFMUX_TYPE1_MUXEN_MASK                            0x00004000
#define    PMU_CPD_TMODEIFMUX_CDP_MUXEN_SHIFT                             13
#define    PMU_CPD_TMODEIFMUX_CDP_MUXEN_MASK                              0x00002000
#define    PMU_CPD_TMODEIFMUX_DCP_MUXEN_SHIFT                             12
#define    PMU_CPD_TMODEIFMUX_DCP_MUXEN_MASK                              0x00001000
#define    PMU_CPD_TMODEIFMUX_SDP_MUXEN_SHIFT                             11
#define    PMU_CPD_TMODEIFMUX_SDP_MUXEN_MASK                              0x00000800
#define    PMU_CPD_TMODEIFMUX_BCDONE_MUXEN_SHIFT                          10
#define    PMU_CPD_TMODEIFMUX_BCDONE_MUXEN_MASK                           0x00000400
#define    PMU_CPD_TMODEIFMUX_GSMBURSTB_MUXEN_SHIFT                       9
#define    PMU_CPD_TMODEIFMUX_GSMBURSTB_MUXEN_MASK                        0x00000200
#define    PMU_CPD_TMODEIFMUX_ADCSYNCB_MUXEN_SHIFT                        8
#define    PMU_CPD_TMODEIFMUX_ADCSYNCB_MUXEN_MASK                         0x00000100
#define    PMU_CPD_TMODEIFMUX_PCRFLDO_MUXEN_SHIFT                         7
#define    PMU_CPD_TMODEIFMUX_PCRFLDO_MUXEN_MASK                          0x00000080
#define    PMU_CPD_TMODEIFMUX_PC2_MUXEN_SHIFT                             6
#define    PMU_CPD_TMODEIFMUX_PC2_MUXEN_MASK                              0x00000040
#define    PMU_CPD_TMODEIFMUX_PC1_MUXEN_SHIFT                             5
#define    PMU_CPD_TMODEIFMUX_PC1_MUXEN_MASK                              0x00000020
#define    PMU_CPD_TMODEIFMUX_SIMLDO1_MUXEN_SHIFT                         4
#define    PMU_CPD_TMODEIFMUX_SIMLDO1_MUXEN_MASK                          0x00000010
#define    PMU_CPD_TMODEIFMUX_SIMLDO0_MUXEN_SHIFT                         3
#define    PMU_CPD_TMODEIFMUX_SIMLDO0_MUXEN_MASK                          0x00000008
#define    PMU_CPD_TMODEIFMUX_RESETB_MUXEN_SHIFT                          2
#define    PMU_CPD_TMODEIFMUX_RESETB_MUXEN_MASK                           0x00000004
#define    PMU_CPD_TMODEIFMUX_DIGTST2_MUXEN_SHIFT                         1
#define    PMU_CPD_TMODEIFMUX_DIGTST2_MUXEN_MASK                          0x00000002
#define    PMU_CPD_TMODEIFMUX_DIGTST1_MUXEN_SHIFT                         0
#define    PMU_CPD_TMODEIFMUX_DIGTST1_MUXEN_MASK                          0x00000001

#define PMU_CPD_ANADBG_OFFSET                                             0x0000002C
#define PMU_CPD_ANADBG_TYPE                                               UInt32
#define PMU_CPD_ANADBG_RESERVED_MASK                                      0xFFFC0000
#define    PMU_CPD_ANADBG_PWRMGR_TESTEN_SHIFT                             17
#define    PMU_CPD_ANADBG_PWRMGR_TESTEN_MASK                              0x00020000
#define    PMU_CPD_ANADBG_ANATST_EN_SHIFT                                 16
#define    PMU_CPD_ANADBG_ANATST_EN_MASK                                  0x00010000
#define    PMU_CPD_ANADBG_ANADBG2_7_0__SHIFT                              8
#define    PMU_CPD_ANADBG_ANADBG2_7_0__MASK                               0x0000FF00
#define    PMU_CPD_ANADBG_ANADBG1_7_0__SHIFT                              0
#define    PMU_CPD_ANADBG_ANADBG1_7_0__MASK                               0x000000FF

#define PMU_CPD_SBCMCTRL_OFFSET                                           0x00000030
#define PMU_CPD_SBCMCTRL_TYPE                                             UInt32
#define PMU_CPD_SBCMCTRL_RESERVED_MASK                                    0xFFFFF000
#define    PMU_CPD_SBCMCTRL_SBCM_MODE_SHIFT                               11
#define    PMU_CPD_SBCMCTRL_SBCM_MODE_MASK                                0x00000800
#define    PMU_CPD_SBCMCTRL_CSR_OVRD_SHIFT                                10
#define    PMU_CPD_SBCMCTRL_CSR_OVRD_MASK                                 0x00000400
#define    PMU_CPD_SBCMCTRL_IOSR_OVRD_SHIFT                               9
#define    PMU_CPD_SBCMCTRL_IOSR_OVRD_MASK                                0x00000200
#define    PMU_CPD_SBCMCTRL_DVSLDO1_OVRD_SHIFT                            8
#define    PMU_CPD_SBCMCTRL_DVSLDO1_OVRD_MASK                             0x00000100
#define    PMU_CPD_SBCMCTRL_DVSLDO2_OVRD_SHIFT                            7
#define    PMU_CPD_SBCMCTRL_DVSLDO2_OVRD_MASK                             0x00000080
#define    PMU_CPD_SBCMCTRL_CAMLDO_OVRD_SHIFT                             6
#define    PMU_CPD_SBCMCTRL_CAMLDO_OVRD_MASK                              0x00000040
#define    PMU_CPD_SBCMCTRL_RFLDO_OVRD_SHIFT                              5
#define    PMU_CPD_SBCMCTRL_RFLDO_OVRD_MASK                               0x00000020
#define    PMU_CPD_SBCMCTRL_HVLDO1_OVRD_SHIFT                             4
#define    PMU_CPD_SBCMCTRL_HVLDO1_OVRD_MASK                              0x00000010
#define    PMU_CPD_SBCMCTRL_HVLDO2_OVRD_SHIFT                             3
#define    PMU_CPD_SBCMCTRL_HVLDO2_OVRD_MASK                              0x00000008
#define    PMU_CPD_SBCMCTRL_SIMLDO1_OVRD_SHIFT                            2
#define    PMU_CPD_SBCMCTRL_SIMLDO1_OVRD_MASK                             0x00000004
#define    PMU_CPD_SBCMCTRL_SIMLDO2_OVRD_SHIFT                            1
#define    PMU_CPD_SBCMCTRL_SIMLDO2_OVRD_MASK                             0x00000002
#define    PMU_CPD_SBCMCTRL_USBLDO_OVRD_SHIFT                             0
#define    PMU_CPD_SBCMCTRL_USBLDO_OVRD_MASK                              0x00000001

#define PMU_CPD_PAGESEL_OFFSET                                            0x0000003C
#define PMU_CPD_PAGESEL_TYPE                                              UInt32
#define PMU_CPD_PAGESEL_RESERVED_MASK                                     0xFFFFFFFE
#define    PMU_CPD_PAGESEL_PAGESEL_SHIFT                                  0
#define    PMU_CPD_PAGESEL_PAGESEL_MASK                                   0x00000001

#endif /* __BRCM_RDB_PMU_CPD_H__ */


