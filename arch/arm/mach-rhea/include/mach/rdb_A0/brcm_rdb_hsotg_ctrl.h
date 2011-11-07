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
/*     Date     : Generated on 10/28/2011 16:56:41                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_HSOTG_CTRL_H__
#define __BRCM_RDB_HSOTG_CTRL_H__

#define HSOTG_CTRL_USBOTGCONTROL_OFFSET                                   0x00000000
#define HSOTG_CTRL_USBOTGCONTROL_TYPE                                     UInt32
#define HSOTG_CTRL_USBOTGCONTROL_RESERVED_MASK                            0x003C0000
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_SHIFT                        31
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT2_MASK                         0x80000000
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT1_SHIFT                        30
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT1_MASK                         0x40000000
#define    HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_SHIFT                    29
#define    HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK                     0x20000000
#define    HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_SHIFT                    28
#define    HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK                     0x10000000
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_SHIFT                    27
#define    HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK                     0x08000000
#define    HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_SHIFT                26
#define    HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK                 0x04000000
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV2_SHIFT                         25
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV2_MASK                          0x02000000
#define    HSOTG_CTRL_USBOTGCONTROL_PHY_ISO_I_SHIFT                       24
#define    HSOTG_CTRL_USBOTGCONTROL_PHY_ISO_I_MASK                        0x01000000
#define    HSOTG_CTRL_USBOTGCONTROL_WAKEUP_INT_MODE_SHIFT                 23
#define    HSOTG_CTRL_USBOTGCONTROL_WAKEUP_INT_MODE_MASK                  0x00800000
#define    HSOTG_CTRL_USBOTGCONTROL_BC_MODE_SHIFT                         22
#define    HSOTG_CTRL_USBOTGCONTROL_BC_MODE_MASK                          0x00400000
#define    HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_SHIFT              17
#define    HSOTG_CTRL_USBOTGCONTROL_USB_HCLK_EN_DIRECT_MASK               0x00020000
#define    HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_SHIFT               16
#define    HSOTG_CTRL_USBOTGCONTROL_USB_ON_IS_HCLK_EN_MASK                0x00010000
#define    HSOTG_CTRL_USBOTGCONTROL_USB_ON_SHIFT                          15
#define    HSOTG_CTRL_USBOTGCONTROL_USB_ON_MASK                           0x00008000
#define    HSOTG_CTRL_USBOTGCONTROL_BVALID_CLR_SHIFT                      14
#define    HSOTG_CTRL_USBOTGCONTROL_BVALID_CLR_MASK                       0x00004000
#define    HSOTG_CTRL_USBOTGCONTROL_AHBM_ENDIAN_SHIFT                     13
#define    HSOTG_CTRL_USBOTGCONTROL_AHBM_ENDIAN_MASK                      0x00002000
#define    HSOTG_CTRL_USBOTGCONTROL_AHBS_ENDIAN_SHIFT                     12
#define    HSOTG_CTRL_USBOTGCONTROL_AHBS_ENDIAN_MASK                      0x00001000
#define    HSOTG_CTRL_USBOTGCONTROL_PRST_N_SW_SHIFT                       11
#define    HSOTG_CTRL_USBOTGCONTROL_PRST_N_SW_MASK                        0x00000800
#define    HSOTG_CTRL_USBOTGCONTROL_HRESET_N_SW_SHIFT                     10
#define    HSOTG_CTRL_USBOTGCONTROL_HRESET_N_SW_MASK                      0x00000400
#define    HSOTG_CTRL_USBOTGCONTROL_UTMI_LINE_STATE1_SHIFT                9
#define    HSOTG_CTRL_USBOTGCONTROL_UTMI_LINE_STATE1_MASK                 0x00000200
#define    HSOTG_CTRL_USBOTGCONTROL_UTMI_LINE_STATE0_SHIFT                8
#define    HSOTG_CTRL_USBOTGCONTROL_UTMI_LINE_STATE0_MASK                 0x00000100
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV4_SHIFT                         7
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV4_MASK                          0x00000080
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_PHY_RESETB_SHIFT                 6
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_PHY_RESETB_MASK                  0x00000040
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_LDO_PWRDN_SHIFT                  5
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_LDO_PWRDN_MASK                   0x00000020
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV3_SHIFT                         4
#define    HSOTG_CTRL_USBOTGCONTROL_RESERV3_MASK                          0x00000010
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_DLDO_PDN_SHIFT                   3
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_DLDO_PDN_MASK                    0x00000008
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_ALDO_PDN_SHIFT                   2
#define    HSOTG_CTRL_USBOTGCONTROL_SOFT_ALDO_PDN_MASK                    0x00000004
#define    HSOTG_CTRL_USBOTGCONTROL_WAKEUP_INT_INV_SHIFT                  1
#define    HSOTG_CTRL_USBOTGCONTROL_WAKEUP_INT_INV_MASK                   0x00000002
#define    HSOTG_CTRL_USBOTGCONTROL_AVALID_CTRL_SHIFT                     0
#define    HSOTG_CTRL_USBOTGCONTROL_AVALID_CTRL_MASK                      0x00000001

#define HSOTG_CTRL_PHY_CFG_OFFSET                                         0x00000004
#define HSOTG_CTRL_PHY_CFG_TYPE                                           UInt32
#define HSOTG_CTRL_PHY_CFG_RESERVED_MASK                                  0xF8000000
#define    HSOTG_CTRL_PHY_CFG_PLL_SUSPEND_EN_I_SHIFT                      26
#define    HSOTG_CTRL_PHY_CFG_PLL_SUSPEND_EN_I_MASK                       0x04000000
#define    HSOTG_CTRL_PHY_CFG_PLL_KP_I_SHIFT                              22
#define    HSOTG_CTRL_PHY_CFG_PLL_KP_I_MASK                               0x03C00000
#define    HSOTG_CTRL_PHY_CFG_PLL_KI_I_SHIFT                              19
#define    HSOTG_CTRL_PHY_CFG_PLL_KI_I_MASK                               0x00380000
#define    HSOTG_CTRL_PHY_CFG_PLL_KA_I_SHIFT                              16
#define    HSOTG_CTRL_PHY_CFG_PLL_KA_I_MASK                               0x00070000
#define    HSOTG_CTRL_PHY_CFG_PLL_NDIV_INT_I_SHIFT                        6
#define    HSOTG_CTRL_PHY_CFG_PLL_NDIV_INT_I_MASK                         0x0000FFC0
#define    HSOTG_CTRL_PHY_CFG_PLL_PDIV_I_SHIFT                            3
#define    HSOTG_CTRL_PHY_CFG_PLL_PDIV_I_MASK                             0x00000038
#define    HSOTG_CTRL_PHY_CFG_PLL_RESETB_SHIFT                            2
#define    HSOTG_CTRL_PHY_CFG_PLL_RESETB_MASK                             0x00000004
#define    HSOTG_CTRL_PHY_CFG_PHY_IDDQ_I_SHIFT                            1
#define    HSOTG_CTRL_PHY_CFG_PHY_IDDQ_I_MASK                             0x00000002
#define    HSOTG_CTRL_PHY_CFG_BC_ISO_I_SHIFT                              0
#define    HSOTG_CTRL_PHY_CFG_BC_ISO_I_MASK                               0x00000001

#define HSOTG_CTRL_PHY_P1CTL_OFFSET                                       0x00000008
#define HSOTG_CTRL_PHY_P1CTL_TYPE                                         UInt32
#define HSOTG_CTRL_PHY_P1CTL_RESERVED_MASK                                0x100002F0
#define    HSOTG_CTRL_PHY_P1CTL_PHY_CLOCK_REQ_CLEAR_SHIFT                 31
#define    HSOTG_CTRL_PHY_P1CTL_PHY_CLOCK_REQ_CLEAR_MASK                  0x80000000
#define    HSOTG_CTRL_PHY_P1CTL_PHY_CLOCK_REQUEST_SHIFT                   30
#define    HSOTG_CTRL_PHY_P1CTL_PHY_CLOCK_REQUEST_MASK                    0x40000000
#define    HSOTG_CTRL_PHY_P1CTL_PHY_RESETB_MASK_SHIFT                     29
#define    HSOTG_CTRL_PHY_P1CTL_PHY_RESETB_MASK_MASK                      0x20000000
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_OUTADJ_SHIFT                    24
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_OUTADJ_MASK                     0x0F000000
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_CADJ_SHIFT                      20
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOBG_CADJ_MASK                       0x00F00000
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTLEN_1P2_I_SHIFT                 19
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTLEN_1P2_I_MASK                  0x00080000
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTL_1P2_I_SHIFT                   16
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LDOCNTL_1P2_I_MASK                    0x00070000
#define    HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_SHIFT                   15
#define    HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_MASK                    0x00008000
#define    HSOTG_CTRL_PHY_P1CTL_RX_SYNC_DETECT_LENGTH_SHIFT               12
#define    HSOTG_CTRL_PHY_P1CTL_RX_SYNC_DETECT_LENGTH_MASK                0x00007000
#define    HSOTG_CTRL_PHY_P1CTL_DFE_LOOPBACK_SHIFT                        11
#define    HSOTG_CTRL_PHY_P1CTL_DFE_LOOPBACK_MASK                         0x00000800
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LOOPBACK_SHIFT                        10
#define    HSOTG_CTRL_PHY_P1CTL_AFE_LOOPBACK_MASK                         0x00000400
#define    HSOTG_CTRL_PHY_P1CTL_PHY_DISCONNECT_SHIFT                      8
#define    HSOTG_CTRL_PHY_P1CTL_PHY_DISCONNECT_MASK                       0x00000100
#define    HSOTG_CTRL_PHY_P1CTL_PHY_MODE_SHIFT                            2
#define    HSOTG_CTRL_PHY_P1CTL_PHY_MODE_MASK                             0x0000000C
#define    HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_SHIFT                          1
#define    HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK                           0x00000002
#define    HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_SHIFT                         0
#define    HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK                          0x00000001

#define HSOTG_CTRL_BC_STATUS_OFFSET                                       0x0000000C
#define HSOTG_CTRL_BC_STATUS_TYPE                                         UInt32
#define HSOTG_CTRL_BC_STATUS_RESERVED_MASK                                0x00000000
#define    HSOTG_CTRL_BC_STATUS_PHY_RESUME_SHIFT                          30
#define    HSOTG_CTRL_BC_STATUS_PHY_RESUME_MASK                           0xC0000000
#define    HSOTG_CTRL_BC_STATUS_PLL_LOCK_SHIFT                            29
#define    HSOTG_CTRL_BC_STATUS_PLL_LOCK_MASK                             0x20000000
#define    HSOTG_CTRL_BC_STATUS_BC_CUR_STATE_SHIFT                        24
#define    HSOTG_CTRL_BC_STATUS_BC_CUR_STATE_MASK                         0x1F000000
#define    HSOTG_CTRL_BC_STATUS_T_CNT_8_SHIFT                             23
#define    HSOTG_CTRL_BC_STATUS_T_CNT_8_MASK                              0x00800000
#define    HSOTG_CTRL_BC_STATUS_T_CNT_1_SHIFT                             22
#define    HSOTG_CTRL_BC_STATUS_T_CNT_1_MASK                              0x00400000
#define    HSOTG_CTRL_BC_STATUS_T_CNT_0_SHIFT                             21
#define    HSOTG_CTRL_BC_STATUS_T_CNT_0_MASK                              0x00200000
#define    HSOTG_CTRL_BC_STATUS_POR_RST_N_SHIFT                           20
#define    HSOTG_CTRL_BC_STATUS_POR_RST_N_MASK                            0x00100000
#define    HSOTG_CTRL_BC_STATUS_S9_SHIFT                                  19
#define    HSOTG_CTRL_BC_STATUS_S9_MASK                                   0x00080000
#define    HSOTG_CTRL_BC_STATUS_S8_SHIFT                                  18
#define    HSOTG_CTRL_BC_STATUS_S8_MASK                                   0x00040000
#define    HSOTG_CTRL_BC_STATUS_S7_SHIFT                                  17
#define    HSOTG_CTRL_BC_STATUS_S7_MASK                                   0x00020000
#define    HSOTG_CTRL_BC_STATUS_S4_SHIFT                                  16
#define    HSOTG_CTRL_BC_STATUS_S4_MASK                                   0x00010000
#define    HSOTG_CTRL_BC_STATUS_S1_SHIFT                                  15
#define    HSOTG_CTRL_BC_STATUS_S1_MASK                                   0x00008000
#define    HSOTG_CTRL_BC_STATUS_S_M0_SHIFT                                14
#define    HSOTG_CTRL_BC_STATUS_S_M0_MASK                                 0x00004000
#define    HSOTG_CTRL_BC_STATUS_S_P0_SHIFT                                13
#define    HSOTG_CTRL_BC_STATUS_S_P0_MASK                                 0x00002000
#define    HSOTG_CTRL_BC_STATUS_CLK_32K_SHIFT                             12
#define    HSOTG_CTRL_BC_STATUS_CLK_32K_MASK                              0x00001000
#define    HSOTG_CTRL_BC_STATUS_CLMP_PS2_COMP_SHIFT                       11
#define    HSOTG_CTRL_BC_STATUS_CLMP_PS2_COMP_MASK                        0x00000800
#define    HSOTG_CTRL_BC_STATUS_CHG_CMP_SHIFT                             10
#define    HSOTG_CTRL_BC_STATUS_CHG_CMP_MASK                              0x00000400
#define    HSOTG_CTRL_BC_STATUS_DPDM_CMP_SHIFT                            9
#define    HSOTG_CTRL_BC_STATUS_DPDM_CMP_MASK                             0x00000200
#define    HSOTG_CTRL_BC_STATUS_PS2_CMP_SHIFT                             8
#define    HSOTG_CTRL_BC_STATUS_PS2_CMP_MASK                              0x00000100
#define    HSOTG_CTRL_BC_STATUS_BC_DONE_SHIFT                             7
#define    HSOTG_CTRL_BC_STATUS_BC_DONE_MASK                              0x00000080
#define    HSOTG_CTRL_BC_STATUS_DM_TIMEOUT_SHIFT                          6
#define    HSOTG_CTRL_BC_STATUS_DM_TIMEOUT_MASK                           0x00000040
#define    HSOTG_CTRL_BC_STATUS_DM_ERROR_SHIFT                            5
#define    HSOTG_CTRL_BC_STATUS_DM_ERROR_MASK                             0x00000020
#define    HSOTG_CTRL_BC_STATUS_DP_TIMEOUT_SHIFT                          4
#define    HSOTG_CTRL_BC_STATUS_DP_TIMEOUT_MASK                           0x00000010
#define    HSOTG_CTRL_BC_STATUS_DP_ERROR_SHIFT                            3
#define    HSOTG_CTRL_BC_STATUS_DP_ERROR_MASK                             0x00000008
#define    HSOTG_CTRL_BC_STATUS_CHP_SHIFT                                 2
#define    HSOTG_CTRL_BC_STATUS_CHP_MASK                                  0x00000004
#define    HSOTG_CTRL_BC_STATUS_DCP_SHIFT                                 1
#define    HSOTG_CTRL_BC_STATUS_DCP_MASK                                  0x00000002
#define    HSOTG_CTRL_BC_STATUS_SHP_SHIFT                                 0
#define    HSOTG_CTRL_BC_STATUS_SHP_MASK                                  0x00000001

#define HSOTG_CTRL_BC_CFG_OFFSET                                          0x00000010
#define HSOTG_CTRL_BC_CFG_TYPE                                            UInt32
#define HSOTG_CTRL_BC_CFG_RESERVED_MASK                                   0x00007000
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_KEY_SHIFT                            17
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_KEY_MASK                             0xFFFE0000
#define    HSOTG_CTRL_BC_CFG_SW_OVWR_EN_SHIFT                             16
#define    HSOTG_CTRL_BC_CFG_SW_OVWR_EN_MASK                              0x00010000
#define    HSOTG_CTRL_BC_CFG_SW_RST_SHIFT                                 15
#define    HSOTG_CTRL_BC_CFG_SW_RST_MASK                                  0x00008000
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_SHIFT                            2
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_MASK                             0x00000FFC
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_M0_SHIFT                         1
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_M0_MASK                          0x00000002
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_P0_SHIFT                         0
#define    HSOTG_CTRL_BC_CFG_BC_OVWR_SET_P0_MASK                          0x00000001

#define HSOTG_CTRL_TP_IN_OFFSET                                           0x00000014
#define HSOTG_CTRL_TP_IN_TYPE                                             UInt32
#define HSOTG_CTRL_TP_IN_RESERVED_MASK                                    0xFFFFFFFF

#define HSOTG_CTRL_TP_OUT_OFFSET                                          0x00000018
#define HSOTG_CTRL_TP_OUT_TYPE                                            UInt32
#define HSOTG_CTRL_TP_OUT_RESERVED_MASK                                   0xFFFFFFFF

#define HSOTG_CTRL_PHY_CTRL_OFFSET                                        0x0000001C
#define HSOTG_CTRL_PHY_CTRL_TYPE                                          UInt32
#define HSOTG_CTRL_PHY_CTRL_RESERVED_MASK                                 0xFFFF0700
#define    HSOTG_CTRL_PHY_CTRL_POST_COUNT_SHIFT                           12
#define    HSOTG_CTRL_PHY_CTRL_POST_COUNT_MASK                            0x0000F000
#define    HSOTG_CTRL_PHY_CTRL_SUSPEND_MASK_SHIFT                         11
#define    HSOTG_CTRL_PHY_CTRL_SUSPEND_MASK_MASK                          0x00000800
#define    HSOTG_CTRL_PHY_CTRL_MAIN_COUNTER_SHIFT                         0
#define    HSOTG_CTRL_PHY_CTRL_MAIN_COUNTER_MASK                          0x000000FF

#define HSOTG_CTRL_USBREG_OFFSET                                          0x00000020
#define HSOTG_CTRL_USBREG_TYPE                                            UInt32
#define HSOTG_CTRL_USBREG_RESERVED_MASK                                   0xFFFFFFFC
#define    HSOTG_CTRL_USBREG_SS_SCALEDOWN_MODE_SHIFT                      0
#define    HSOTG_CTRL_USBREG_SS_SCALEDOWN_MODE_MASK                       0x00000003

#define HSOTG_CTRL_USBPROBEN_OFFSET                                       0x00000024
#define HSOTG_CTRL_USBPROBEN_TYPE                                         UInt32
#define HSOTG_CTRL_USBPROBEN_RESERVED_MASK                                0xFFFFFFFF

#endif /* __BRCM_RDB_HSOTG_CTRL_H__ */


