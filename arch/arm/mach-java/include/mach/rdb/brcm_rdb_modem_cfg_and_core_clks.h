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

#ifndef __BRCM_RDB_MODEM_CFG_AND_CORE_CLKS_H__
#define __BRCM_RDB_MODEM_CFG_AND_CORE_CLKS_H__

#define MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_OFFSET                         0x00000000
#define MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_TYPE                           UInt32
#define MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_RESERVED_MASK                  0xFFFF17FB
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_UARTSFREQUENCY_SHIFT        15
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_UARTSFREQUENCY_MASK         0x00008000
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_AUTOGATEUARTS_SHIFT         14
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_AUTOGATEUARTS_MASK          0x00004000
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_ENABLETIMER_SHIFT           13
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_ENABLETIMER_MASK            0x00002000
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_ENABLEUARTS_SHIFT           11
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_ENABLEUARTS_MASK            0x00000800
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_AUTOGATEAHBBRIDGE_SHIFT     2
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKGATES_AUTOGATEAHBBRIDGE_MASK      0x00000004

#define MODEM_CFG_AND_CORE_CLKS_PLL_LOCK_OFFSET                           0x00000004
#define MODEM_CFG_AND_CORE_CLKS_PLL_LOCK_TYPE                             UInt32
#define MODEM_CFG_AND_CORE_CLKS_PLL_LOCK_RESERVED_MASK                    0xFFFF7FFF
#define    MODEM_CFG_AND_CORE_CLKS_PLL_LOCK_PLL_LOCK_SHIFT                15
#define    MODEM_CFG_AND_CORE_CLKS_PLL_LOCK_PLL_LOCK_MASK                 0x00008000

#define MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_OFFSET                        0x00000008
#define MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_TYPE                          UInt32
#define MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_RESERVED_MASK                 0xFFFFFF00
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_16QAM_B_SHIFT   7
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_16QAM_B_MASK    0x00000080
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_10C_MAX_SHIFT   6
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_10C_MAX_MASK    0x00000040
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_64QAM_B_SHIFT   5
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_64QAM_B_MASK    0x00000020
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_PRISM_ENB_SHIFT       4
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_PRISM_ENB_MASK        0x00000010
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_CAT_SHIFT       3
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_CAT_MASK        0x00000008
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_ENB_SHIFT       2
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSUPA_ENB_MASK        0x00000004
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_CAT_SHIFT       1
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_CAT_MASK        0x00000002
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_ENB_SHIFT       0
#define    MODEM_CFG_AND_CORE_CLKS_FUSE_STATUS_FUSE_HSDPA_ENB_MASK        0x00000001

#define MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_OFFSET                           0x00000010
#define MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_TYPE                             UInt32
#define MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_RESERVED_MASK                    0xFFFFFF70
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_YSELECT_SHIFT                 7
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_YSELECT_MASK                  0x00000080
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_DIRECTSWITCH_SHIFT            3
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_DIRECTSWITCH_MASK             0x00000008
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_PLLCLKSELECT_SHIFT            2
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_PLLCLKSELECT_MASK             0x00000004
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_SLOWCLKSELECT_SHIFT           1
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_SLOWCLKSELECT_MASK            0x00000002
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_PADCLKSELECT_R1_S1_SHIFT      0
#define    MODEM_CFG_AND_CORE_CLKS_CLOCKMUX_PADCLKSELECT_R1_S1_MASK       0x00000001

#define MODEM_CFG_AND_CORE_CLKS_ENABLECLK156M_OFFSET                      0x00000020
#define MODEM_CFG_AND_CORE_CLKS_ENABLECLK156M_TYPE                        UInt32
#define MODEM_CFG_AND_CORE_CLKS_ENABLECLK156M_RESERVED_MASK               0xFFFFFFFE
#define    MODEM_CFG_AND_CORE_CLKS_ENABLECLK156M_ENABLECLK312M_SHIFT      0
#define    MODEM_CFG_AND_CORE_CLKS_ENABLECLK156M_ENABLECLK312M_MASK       0x00000001

#define MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_OFFSET                      0x00000024
#define MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_TYPE                        UInt32
#define MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_RESERVED_MASK               0xFFFFFF0E
#define    MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_PWRENABLESELECT_SHIFT    4
#define    MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_PWRENABLESELECT_MASK     0x000000F0
#define    MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_ENABLEPWRMNGT_SHIFT      0
#define    MODEM_CFG_AND_CORE_CLKS_PWRMANAGEMENT_ENABLEPWRMNGT_MASK       0x00000001

#define MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_OFFSET                         0x00000030
#define MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_TYPE                           UInt32
#define MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_RESERVED_MASK                  0xFFFF30FF
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_RF_MEM_STDBY_SEL_SHIFT      15
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_RF_MEM_STDBY_SEL_MASK       0x00008000
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMAUTOSTDBY_SHIFT        14
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMAUTOSTDBY_MASK         0x00004000
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMSLEEPSTDBY_SHIFT       10
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMSLEEPSTDBY_MASK        0x00000C00
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMAWAKESTDBY_SHIFT       8
#define    MODEM_CFG_AND_CORE_CLKS_MODEMSTDBY_MODEMAWAKESTDBY_MASK        0x00000300

#define MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_OFFSET                      0x00000040
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_TYPE                        UInt32
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_RESERVED_MASK               0xFFFFFFF0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_ACTIVE_SHIFT 3
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_ACTIVE_MASK 0x00000008
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_OK_SHIFT   2
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_OK_MASK    0x00000004
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_PWRDN_SHIFT 1
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_VDD_HSDPA_TOP_PWRDN_MASK 0x00000002
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_HSDPA_TOP_DORMANT_SHIFT  0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPOWERISL_HSDPA_TOP_DORMANT_MASK   0x00000001

#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWROK_DIS_OFFSET                     0x00000044
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWROK_DIS_TYPE                       UInt32
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWROK_DIS_RESERVED_MASK              0xFFFFFFFE
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWROK_DIS_VDD_HSDPA_TOP_PWROK_DIS_SHIFT 0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWROK_DIS_VDD_HSDPA_TOP_PWROK_DIS_MASK 0x00000001

#define MODEM_CFG_AND_CORE_CLKS_HSDPARAMPCOUNT_OFFSET                     0x00000048
#define MODEM_CFG_AND_CORE_CLKS_HSDPARAMPCOUNT_TYPE                       UInt32
#define MODEM_CFG_AND_CORE_CLKS_HSDPARAMPCOUNT_RESERVED_MASK              0xFFFF0000
#define    MODEM_CFG_AND_CORE_CLKS_HSDPARAMPCOUNT_VDD_HSDPA_TOP_RAMP_COUNT_SHIFT 0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPARAMPCOUNT_VDD_HSDPA_TOP_RAMP_COUNT_MASK 0x0000FFFF

#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRSWICTL_OFFSET                     0x0000004C
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRSWICTL_TYPE                       UInt32
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRSWICTL_RESERVED_MASK              0xFFFFFFFC
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWRSWICTL_PSW_CTRL_SHIFT          0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWRSWICTL_PSW_CTRL_MASK           0x00000003

#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRCMPCTL_OFFSET                     0x00000050
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRCMPCTL_TYPE                       UInt32
#define MODEM_CFG_AND_CORE_CLKS_HSDPAPWRCMPCTL_RESERVED_MASK              0xFFFFFFF0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWRCMPCTL_POK_CTRL_SHIFT          0
#define    MODEM_CFG_AND_CORE_CLKS_HSDPAPWRCMPCTL_POK_CTRL_MASK           0x0000000F

#define MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_OFFSET                        0x00000078
#define MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_TYPE                          UInt32
#define MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_RESERVED_MASK                 0xFFFFFF00
#define    MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_CHIPVERSIONHIGH_RH_A_SHIFT 4
#define    MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_CHIPVERSIONHIGH_RH_A_MASK  0x000000F0
#define    MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_CHIPVERSIONLOW_RH_0_SHIFT  0
#define    MODEM_CFG_AND_CORE_CLKS_CHIPVERSION_CHIPVERSIONLOW_RH_0_MASK   0x0000000F

#endif /* __BRCM_RDB_MODEM_CFG_AND_CORE_CLKS_H__ */


