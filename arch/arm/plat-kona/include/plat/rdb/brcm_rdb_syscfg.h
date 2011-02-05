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
/************************************************************************************************/

#ifndef __BRCM_RDB_SYSCFG_H__
#define __BRCM_RDB_SYSCFG_H__

#define SYSCFG_BASE_ADDR                                                  0x08880000

#define SYSCFG_IOCR0_OFFSET                                               0x00000000
#define SYSCFG_IOCR0_TYPE                                                 UInt32
#define SYSCFG_IOCR0_RESERVED_MASK                                        0x00000000
#define    SYSCFG_IOCR0_CAMCK_GPIO_MUX_SHIFT                              31
#define    SYSCFG_IOCR0_CAMCK_GPIO_MUX_MASK                               0x80000000
#define    SYSCFG_IOCR0_FLASH_SD2_MUX_SHIFT                               30
#define    SYSCFG_IOCR0_FLASH_SD2_MUX_MASK                                0x40000000
#define    SYSCFG_IOCR0_LCD_CTRL_MUX_SHIFT                                29
#define    SYSCFG_IOCR0_LCD_CTRL_MUX_MASK                                 0x20000000
#define    SYSCFG_IOCR0_LCDD1_LCDD15_MUX_SHIFT                            28
#define    SYSCFG_IOCR0_LCDD1_LCDD15_MUX_MASK                             0x10000000
#define    SYSCFG_IOCR0_GPEN9_B1_SHIFT                                    27
#define    SYSCFG_IOCR0_GPEN9_B1_MASK                                     0x08000000
#define    SYSCFG_IOCR0_DSP_TEST_PORT_ENABLE_SHIFT                        26
#define    SYSCFG_IOCR0_DSP_TEST_PORT_ENABLE_MASK                         0x04000000
#define    SYSCFG_IOCR0_MPHI_MUX_SHIFT                                    25
#define    SYSCFG_IOCR0_MPHI_MUX_MASK                                     0x02000000
#define    SYSCFG_IOCR0_DIGMIC_MUX_SHIFT                                  24
#define    SYSCFG_IOCR0_DIGMIC_MUX_MASK                                   0x01000000
#define    SYSCFG_IOCR0_PCM_MUX_SHIFT                                     22
#define    SYSCFG_IOCR0_PCM_MUX_MASK                                      0x00C00000
#define    SYSCFG_IOCR0_SPI_MUX_HI_SHIFT                                  21
#define    SYSCFG_IOCR0_SPI_MUX_HI_MASK                                   0x00200000
#define    SYSCFG_IOCR0_GPEN11_B1_SHIFT                                   20
#define    SYSCFG_IOCR0_GPEN11_B1_MASK                                    0x00100000
#define    SYSCFG_IOCR0_LCDD16_LCDD17_MUX_SHIFT                           19
#define    SYSCFG_IOCR0_LCDD16_LCDD17_MUX_MASK                            0x00080000
#define    SYSCFG_IOCR0_AFCPDM_MUX_SHIFT                                  18
#define    SYSCFG_IOCR0_AFCPDM_MUX_MASK                                   0x00040000
#define    SYSCFG_IOCR0_GPEN11_B0_SHIFT                                   17
#define    SYSCFG_IOCR0_GPEN11_B0_MASK                                    0x00020000
#define    SYSCFG_IOCR0_GPEN10_SHIFT                                      15
#define    SYSCFG_IOCR0_GPEN10_MASK                                       0x00018000
#define    SYSCFG_IOCR0_GPEN9_B0_SHIFT                                    14
#define    SYSCFG_IOCR0_GPEN9_B0_MASK                                     0x00004000
#define    SYSCFG_IOCR0_GPEN8_MUX_SHIFT                                   13
#define    SYSCFG_IOCR0_GPEN8_MUX_MASK                                    0x00002000
#define    SYSCFG_IOCR0_GPEN7_SHIFT                                       12
#define    SYSCFG_IOCR0_GPEN7_MASK                                        0x00001000
#define    SYSCFG_IOCR0_SPI_MUX_SHIFT                                     11
#define    SYSCFG_IOCR0_SPI_MUX_MASK                                      0x00000800
#define    SYSCFG_IOCR0_GPIO16_MUX_SHIFT                                  10
#define    SYSCFG_IOCR0_GPIO16_MUX_MASK                                   0x00000400
#define    SYSCFG_IOCR0_GPIO17_MUX_SHIFT                                  9
#define    SYSCFG_IOCR0_GPIO17_MUX_MASK                                   0x00000200
#define    SYSCFG_IOCR0_GPIO_MUXES_SHIFT                                  7
#define    SYSCFG_IOCR0_GPIO_MUXES_MASK                                   0x00000180
#define    SYSCFG_IOCR0_I2S_MUX_SHIFT                                     5
#define    SYSCFG_IOCR0_I2S_MUX_MASK                                      0x00000060
#define    SYSCFG_IOCR0_SD1_MUX_SHIFT                                     3
#define    SYSCFG_IOCR0_SD1_MUX_MASK                                      0x00000018
#define    SYSCFG_IOCR0_M68_SHIFT                                         2
#define    SYSCFG_IOCR0_M68_MASK                                          0x00000004
#define    SYSCFG_IOCR0_SD3_MUX_SHIFT                                     0
#define    SYSCFG_IOCR0_SD3_MUX_MASK                                      0x00000003

#define SYSCFG_IOCR1_OFFSET                                               0x00000004
#define SYSCFG_IOCR1_TYPE                                                 UInt32
#define SYSCFG_IOCR1_RESERVED_MASK                                        0xFFFF0000
#define    SYSCFG_IOCR1_KEY_COL_SHIFT                                     8
#define    SYSCFG_IOCR1_KEY_COL_MASK                                      0x0000FF00
#define    SYSCFG_IOCR1_KEY_ROW_SHIFT                                     0
#define    SYSCFG_IOCR1_KEY_ROW_MASK                                      0x000000FF

#define SYSCFG_SUCR_OFFSET                                                0x00000008
#define SYSCFG_SUCR_TYPE                                                  UInt32
#define SYSCFG_SUCR_RESERVED_MASK                                         0x2E81FFEB
#define    SYSCFG_SUCR_DOWNLOAD_SHIFT                                     31
#define    SYSCFG_SUCR_DOWNLOAD_MASK                                      0x80000000
#define    SYSCFG_SUCR_FLASH_BOOT_SHIFT                                   30
#define    SYSCFG_SUCR_FLASH_BOOT_MASK                                    0x40000000
#define    SYSCFG_SUCR_AP_SHIFT                                           28
#define    SYSCFG_SUCR_AP_MASK                                            0x10000000
#define    SYSCFG_SUCR_VCOBYPASS_SHIFT                                    24
#define    SYSCFG_SUCR_VCOBYPASS_MASK                                     0x01000000
#define    SYSCFG_SUCR_TEST_LOOP_SHIFT                                    22
#define    SYSCFG_SUCR_TEST_LOOP_MASK                                     0x00400000
#define    SYSCFG_SUCR_EJTAG_SEL_SHIFT                                    21
#define    SYSCFG_SUCR_EJTAG_SEL_MASK                                     0x00200000
#define    SYSCFG_SUCR_JTAG_SEL_SHIFT                                     18
#define    SYSCFG_SUCR_JTAG_SEL_MASK                                      0x001C0000
#define    SYSCFG_SUCR_SYS_REF_SEL_N_SHIFT                                17
#define    SYSCFG_SUCR_SYS_REF_SEL_N_MASK                                 0x00020000
#define    SYSCFG_SUCR_SRST_STAT_SHIFT                                    4
#define    SYSCFG_SUCR_SRST_STAT_MASK                                     0x00000010
#define    SYSCFG_SUCR_BOOTSRC_SHIFT                                      2
#define    SYSCFG_SUCR_BOOTSRC_MASK                                       0x00000004

#define SYSCFG_IOCR2_OFFSET                                               0x0000000C
#define SYSCFG_IOCR2_TYPE                                                 UInt32
#define SYSCFG_IOCR2_RESERVED_MASK                                        0x00003000
#define    SYSCFG_IOCR2_SD2DAT_PULL_SHIFT                                 30
#define    SYSCFG_IOCR2_SD2DAT_PULL_MASK                                  0xC0000000
#define    SYSCFG_IOCR2_SD2CMD_PULL_SHIFT                                 28
#define    SYSCFG_IOCR2_SD2CMD_PULL_MASK                                  0x30000000
#define    SYSCFG_IOCR2_SD1DAT_PULL_SHIFT                                 26
#define    SYSCFG_IOCR2_SD1DAT_PULL_MASK                                  0x0C000000
#define    SYSCFG_IOCR2_SD1CMD_PULL_SHIFT                                 24
#define    SYSCFG_IOCR2_SD1CMD_PULL_MASK                                  0x03000000
#define    SYSCFG_IOCR2_SD3DAT_PULL_SHIFT                                 22
#define    SYSCFG_IOCR2_SD3DAT_PULL_MASK                                  0x00C00000
#define    SYSCFG_IOCR2_GPEN8_MUX_HI_SHIFT                                21
#define    SYSCFG_IOCR2_GPEN8_MUX_HI_MASK                                 0x00200000
#define    SYSCFG_IOCR2_SD3CMD_PULL_SHIFT                                 19
#define    SYSCFG_IOCR2_SD3CMD_PULL_MASK                                  0x00180000
#define    SYSCFG_IOCR2_ANA_SYSCLKEN_MUX_SHIFT                            18
#define    SYSCFG_IOCR2_ANA_SYSCLKEN_MUX_MASK                             0x00040000
#define    SYSCFG_IOCR2_SOFTRSTO_MUX_SHIFT                                16
#define    SYSCFG_IOCR2_SOFTRSTO_MUX_MASK                                 0x00030000
#define    SYSCFG_IOCR2_OTGCTRL1_MUX_SHIFT                                14
#define    SYSCFG_IOCR2_OTGCTRL1_MUX_MASK                                 0x0000C000
#define    SYSCFG_IOCR2_LCDDATA_PULL_SHIFT                                10
#define    SYSCFG_IOCR2_LCDDATA_PULL_MASK                                 0x00000C00
#define    SYSCFG_IOCR2_LCDTE_PULL_SHIFT                                  8
#define    SYSCFG_IOCR2_LCDTE_PULL_MASK                                   0x00000300
#define    SYSCFG_IOCR2_SIM2DAT_HYS_SHIFT                                 7
#define    SYSCFG_IOCR2_SIM2DAT_HYS_MASK                                  0x00000080
#define    SYSCFG_IOCR2_SIMDAT_HYS_SHIFT                                  6
#define    SYSCFG_IOCR2_SIMDAT_HYS_MASK                                   0x00000040
#define    SYSCFG_IOCR2_OSC2_SELECT_SHIFT                                 4
#define    SYSCFG_IOCR2_OSC2_SELECT_MASK                                  0x00000030
#define    SYSCFG_IOCR2_OSC2_ENABLE_SHIFT                                 3
#define    SYSCFG_IOCR2_OSC2_ENABLE_MASK                                  0x00000008
#define    SYSCFG_IOCR2_OSC1_SELECT_SHIFT                                 1
#define    SYSCFG_IOCR2_OSC1_SELECT_MASK                                  0x00000006
#define    SYSCFG_IOCR2_OSC1_ENABLE_SHIFT                                 0
#define    SYSCFG_IOCR2_OSC1_ENABLE_MASK                                  0x00000001

#define SYSCFG_PIDR_OFFSET                                                0x00000010
#define SYSCFG_PIDR_TYPE                                                  UInt32
#define SYSCFG_PIDR_RESERVED_MASK                                         0xFFFFF000
#define    SYSCFG_PIDR_PFID_SHIFT                                         8
#define    SYSCFG_PIDR_PFID_MASK                                          0x00000F00
#define    SYSCFG_PIDR_PID_SHIFT                                          4
#define    SYSCFG_PIDR_PID_MASK                                           0x000000F0
#define    SYSCFG_PIDR_RID_SHIFT                                          0
#define    SYSCFG_PIDR_RID_MASK                                           0x0000000F

#define SYSCFG_DSPCTRL_OFFSET                                             0x00000014
#define SYSCFG_DSPCTRL_TYPE                                               UInt32
#define SYSCFG_DSPCTRL_RESERVED_MASK                                      0xFFFFFE0F
#define    SYSCFG_DSPCTRL_AUDIOSRST_SHIFT                                 8
#define    SYSCFG_DSPCTRL_AUDIOSRST_MASK                                  0x00000100
#define    SYSCFG_DSPCTRL_DSPSRST_SHIFT                                   7
#define    SYSCFG_DSPCTRL_DSPSRST_MASK                                    0x00000080
#define    SYSCFG_DSPCTRL_SYNCEXTPRAM_SHIFT                               6
#define    SYSCFG_DSPCTRL_SYNCEXTPRAM_MASK                                0x00000040
#define    SYSCFG_DSPCTRL_JTAGINTWAKE_SHIFT                               5
#define    SYSCFG_DSPCTRL_JTAGINTWAKE_MASK                                0x00000020
#define    SYSCFG_DSPCTRL_EN_TRST_SHIFT                                   4
#define    SYSCFG_DSPCTRL_EN_TRST_MASK                                    0x00000010

#define SYSCFG_PUMR_OFFSET                                                0x00000018
#define SYSCFG_PUMR_TYPE                                                  UInt32
#define SYSCFG_PUMR_RESERVED_MASK                                         0x00000000
#define    SYSCFG_PUMR_PUMODE_SHIFT                                       0
#define    SYSCFG_PUMR_PUMODE_MASK                                        0xFFFFFFFF

#define SYSCFG_IOCR3_OFFSET                                               0x0000001C
#define SYSCFG_IOCR3_TYPE                                                 UInt32
#define SYSCFG_IOCR3_RESERVED_MASK                                        0x82054008
#define    SYSCFG_IOCR3_PCMDI_PD_SHIFT                                    30
#define    SYSCFG_IOCR3_PCMDI_PD_MASK                                     0x40000000
#define    SYSCFG_IOCR3_DIGMICDATA_PD_SHIFT                               29
#define    SYSCFG_IOCR3_DIGMICDATA_PD_MASK                                0x20000000
#define    SYSCFG_IOCR3_SIM2DAT_PU_SHIFT                                  28
#define    SYSCFG_IOCR3_SIM2DAT_PU_MASK                                   0x10000000
#define    SYSCFG_IOCR3_SIMDAT_PU_SHIFT                                   27
#define    SYSCFG_IOCR3_SIMDAT_PU_MASK                                    0x08000000
#define    SYSCFG_IOCR3_X_TRIG_EN_SHIFT                                   26
#define    SYSCFG_IOCR3_X_TRIG_EN_MASK                                    0x04000000
#define    SYSCFG_IOCR3_WCDMA_UART_DIS_SHIFT                              24
#define    SYSCFG_IOCR3_WCDMA_UART_DIS_MASK                               0x01000000
#define    SYSCFG_IOCR3_SIM2_DIS_SHIFT                                    23
#define    SYSCFG_IOCR3_SIM2_DIS_MASK                                     0x00800000
#define    SYSCFG_IOCR3_UARTC_DIS_SHIFT                                   22
#define    SYSCFG_IOCR3_UARTC_DIS_MASK                                    0x00400000
#define    SYSCFG_IOCR3_TWIF_ENB_SHIFT                                    21
#define    SYSCFG_IOCR3_TWIF_ENB_MASK                                     0x00200000
#define    SYSCFG_IOCR3_PC_DIS_SHIFT                                      20
#define    SYSCFG_IOCR3_PC_DIS_MASK                                       0x00100000
#define    SYSCFG_IOCR3_SYN_DIS_SHIFT                                     19
#define    SYSCFG_IOCR3_SYN_DIS_MASK                                      0x00080000
#define    SYSCFG_IOCR3_GPEN_DIS_SHIFT                                    17
#define    SYSCFG_IOCR3_GPEN_DIS_MASK                                     0x00020000
#define    SYSCFG_IOCR3_JTAG_DIS_SHIFT                                    15
#define    SYSCFG_IOCR3_JTAG_DIS_MASK                                     0x00008000
#define    SYSCFG_IOCR3_SIM_DIS_SHIFT                                     13
#define    SYSCFG_IOCR3_SIM_DIS_MASK                                      0x00002000
#define    SYSCFG_IOCR3_UARTB_DIS_SHIFT                                   12
#define    SYSCFG_IOCR3_UARTB_DIS_MASK                                    0x00001000
#define    SYSCFG_IOCR3_UARTA_DIS_SHIFT                                   11
#define    SYSCFG_IOCR3_UARTA_DIS_MASK                                    0x00000800
#define    SYSCFG_IOCR3_CAMD_PD_SHIFT                                     10
#define    SYSCFG_IOCR3_CAMD_PD_MASK                                      0x00000400
#define    SYSCFG_IOCR3_CAMD_PU_SHIFT                                     9
#define    SYSCFG_IOCR3_CAMD_PU_MASK                                      0x00000200
#define    SYSCFG_IOCR3_CAMHVS_PD_SHIFT                                   8
#define    SYSCFG_IOCR3_CAMHVS_PD_MASK                                    0x00000100
#define    SYSCFG_IOCR3_CAMHVS_PU_SHIFT                                   7
#define    SYSCFG_IOCR3_CAMHVS_PU_MASK                                    0x00000080
#define    SYSCFG_IOCR3_CAMDCK_PD_SHIFT                                   6
#define    SYSCFG_IOCR3_CAMDCK_PD_MASK                                    0x00000040
#define    SYSCFG_IOCR3_CAMDCK_PU_SHIFT                                   5
#define    SYSCFG_IOCR3_CAMDCK_PU_MASK                                    0x00000020
#define    SYSCFG_IOCR3_CAMCK_DIS_SHIFT                                   4
#define    SYSCFG_IOCR3_CAMCK_DIS_MASK                                    0x00000010
#define    SYSCFG_IOCR3_NRDY_PU_SHIFT                                     2
#define    SYSCFG_IOCR3_NRDY_PU_MASK                                      0x00000004
#define    SYSCFG_IOCR3_FRDY_PU_SHIFT                                     1
#define    SYSCFG_IOCR3_FRDY_PU_MASK                                      0x00000002
#define    SYSCFG_IOCR3_FADQ_PU_SHIFT                                     0
#define    SYSCFG_IOCR3_FADQ_PU_MASK                                      0x00000001

#define SYSCFG_IOCR4_OFFSET                                               0x00000020
#define SYSCFG_IOCR4_TYPE                                                 UInt32
#define SYSCFG_IOCR4_RESERVED_MASK                                        0x40000000
#define    SYSCFG_IOCR4_FADQ_PD_SHIFT                                     31
#define    SYSCFG_IOCR4_FADQ_PD_MASK                                      0x80000000
#define    SYSCFG_IOCR4_SDIO_DRIVE_SHIFT                                  27
#define    SYSCFG_IOCR4_SDIO_DRIVE_MASK                                   0x38000000
#define    SYSCFG_IOCR4_ETM_DRIVE_SHIFT                                   24
#define    SYSCFG_IOCR4_ETM_DRIVE_MASK                                    0x07000000
#define    SYSCFG_IOCR4_LCD_DRIVE_SHIFT                                   21
#define    SYSCFG_IOCR4_LCD_DRIVE_MASK                                    0x00E00000
#define    SYSCFG_IOCR4_SIM_DRIVE_SHIFT                                   18
#define    SYSCFG_IOCR4_SIM_DRIVE_MASK                                    0x001C0000
#define    SYSCFG_IOCR4_SIM2_DRIVE_SHIFT                                  15
#define    SYSCFG_IOCR4_SIM2_DRIVE_MASK                                   0x00038000
#define    SYSCFG_IOCR4_CAMERA_DRIVE_SHIFT                                12
#define    SYSCFG_IOCR4_CAMERA_DRIVE_MASK                                 0x00007000
#define    SYSCFG_IOCR4_SDIO3_CLK_DRIVE_SHIFT                             9
#define    SYSCFG_IOCR4_SDIO3_CLK_DRIVE_MASK                              0x00000E00
#define    SYSCFG_IOCR4_SDIO3_DRIVE_SHIFT                                 6
#define    SYSCFG_IOCR4_SDIO3_DRIVE_MASK                                  0x000001C0
#define    SYSCFG_IOCR4_FCLK_DRIVE_SHIFT                                  3
#define    SYSCFG_IOCR4_FCLK_DRIVE_MASK                                   0x00000038
#define    SYSCFG_IOCR4_MEM_DRIVE_SHIFT                                   0
#define    SYSCFG_IOCR4_MEM_DRIVE_MASK                                    0x00000007

#define SYSCFG_IOCR5_OFFSET                                               0x00000024
#define SYSCFG_IOCR5_TYPE                                                 UInt32
#define SYSCFG_IOCR5_RESERVED_MASK                                        0x06003000
#define    SYSCFG_IOCR5_JTAG_DRIVE_SHIFT                                  29
#define    SYSCFG_IOCR5_JTAG_DRIVE_MASK                                   0xE0000000
#define    SYSCFG_IOCR5_TRACEBUS_SELECT_SHIFT                             27
#define    SYSCFG_IOCR5_TRACEBUS_SELECT_MASK                              0x18000000
#define    SYSCFG_IOCR5_SIM2_GPIO_MUX_SHIFT                               24
#define    SYSCFG_IOCR5_SIM2_GPIO_MUX_MASK                                0x01000000
#define    SYSCFG_IOCR5_GPIO27_MUX_SHIFT                                  22
#define    SYSCFG_IOCR5_GPIO27_MUX_MASK                                   0x00C00000
#define    SYSCFG_IOCR5_GPIO26_MUX_SHIFT                                  20
#define    SYSCFG_IOCR5_GPIO26_MUX_MASK                                   0x00300000
#define    SYSCFG_IOCR5_GPIOH_DRIVE_SHIFT                                 17
#define    SYSCFG_IOCR5_GPIOH_DRIVE_MASK                                  0x000E0000
#define    SYSCFG_IOCR5_GPIOL_DRIVE_SHIFT                                 14
#define    SYSCFG_IOCR5_GPIOL_DRIVE_MASK                                  0x0001C000
#define    SYSCFG_IOCR5_GPIO35_MUX_SHIFT                                  10
#define    SYSCFG_IOCR5_GPIO35_MUX_MASK                                   0x00000C00
#define    SYSCFG_IOCR5_GPIO34_MUX_SHIFT                                  8
#define    SYSCFG_IOCR5_GPIO34_MUX_MASK                                   0x00000300
#define    SYSCFG_IOCR5_GPIO33_MUX_SHIFT                                  6
#define    SYSCFG_IOCR5_GPIO33_MUX_MASK                                   0x000000C0
#define    SYSCFG_IOCR5_GPIO32_MUX_SHIFT                                  4
#define    SYSCFG_IOCR5_GPIO32_MUX_MASK                                   0x00000030
#define    SYSCFG_IOCR5_GPIO31_MUX_SHIFT                                  2
#define    SYSCFG_IOCR5_GPIO31_MUX_MASK                                   0x0000000C
#define    SYSCFG_IOCR5_GPIO30_MUX_SHIFT                                  0
#define    SYSCFG_IOCR5_GPIO30_MUX_MASK                                   0x00000003

#define SYSCFG_IOCR6_OFFSET                                               0x00000028
#define SYSCFG_IOCR6_TYPE                                                 UInt32
#define SYSCFG_IOCR6_RESERVED_MASK                                        0x20300380
#define    SYSCFG_IOCR6_GPIO21_18_MUX_SHIFT                               31
#define    SYSCFG_IOCR6_GPIO21_18_MUX_MASK                                0x80000000
#define    SYSCFG_IOCR6_CAM2_CAM1_B_SHIFT                                 30
#define    SYSCFG_IOCR6_CAM2_CAM1_B_MASK                                  0x40000000
#define    SYSCFG_IOCR6_CAM_MODE_SHIFT                                    27
#define    SYSCFG_IOCR6_CAM_MODE_MASK                                     0x18000000
#define    SYSCFG_IOCR6_DDAC_FC_PWRDN_SHIFT                               25
#define    SYSCFG_IOCR6_DDAC_FC_PWRDN_MASK                                0x06000000
#define    SYSCFG_IOCR6_SPI_DRIVE_STRENGTH_CONTROL_SHIFT                  22
#define    SYSCFG_IOCR6_SPI_DRIVE_STRENGTH_CONTROL_MASK                   0x01C00000
#define    SYSCFG_IOCR6_GPIO25_24_MUX_SHIFT                               19
#define    SYSCFG_IOCR6_GPIO25_24_MUX_MASK                                0x00080000
#define    SYSCFG_IOCR6_UART_DRIVE_SHIFT                                  16
#define    SYSCFG_IOCR6_UART_DRIVE_MASK                                   0x00070000
#define    SYSCFG_IOCR6_PCM_DRIVE_SHIFT                                   13
#define    SYSCFG_IOCR6_PCM_DRIVE_MASK                                    0x0000E000
#define    SYSCFG_IOCR6_SDIO_CLK_DRIVE_SHIFT                              10
#define    SYSCFG_IOCR6_SDIO_CLK_DRIVE_MASK                               0x00001C00
#define    SYSCFG_IOCR6_GPIO23_MUX_SHIFT                                  6
#define    SYSCFG_IOCR6_GPIO23_MUX_MASK                                   0x00000040
#define    SYSCFG_IOCR6_GPIO22_MUX_SHIFT                                  5
#define    SYSCFG_IOCR6_GPIO22_MUX_MASK                                   0x00000020
#define    SYSCFG_IOCR6_GPIO21_MUX_SHIFT                                  4
#define    SYSCFG_IOCR6_GPIO21_MUX_MASK                                   0x00000010
#define    SYSCFG_IOCR6_GPIO20_MUX_SHIFT                                  3
#define    SYSCFG_IOCR6_GPIO20_MUX_MASK                                   0x00000008
#define    SYSCFG_IOCR6_JTAG_PM_MONITOR_MUX_SHIFT                         2
#define    SYSCFG_IOCR6_JTAG_PM_MONITOR_MUX_MASK                          0x00000004
#define    SYSCFG_IOCR6_GPIO_PM_MONITOR_MUX_SHIFT                         1
#define    SYSCFG_IOCR6_GPIO_PM_MONITOR_MUX_MASK                          0x00000002
#define    SYSCFG_IOCR6_PM_MONITOR_SELF_MUX_SHIFT                         0
#define    SYSCFG_IOCR6_PM_MONITOR_SELF_MUX_MASK                          0x00000001

#define SYSCFG_IOCR7_OFFSET                                               0x0000002C
#define SYSCFG_IOCR7_TYPE                                                 UInt32
#define SYSCFG_IOCR7_RESERVED_MASK                                        0x0800004E
#define    SYSCFG_IOCR7_RFGPIO5_MUX_SHIFT                                 31
#define    SYSCFG_IOCR7_RFGPIO5_MUX_MASK                                  0x80000000
#define    SYSCFG_IOCR7_RFGPIO4_MUX_SHIFT                                 30
#define    SYSCFG_IOCR7_RFGPIO4_MUX_MASK                                  0x40000000
#define    SYSCFG_IOCR7_RFGPIO3_MUX_SHIFT                                 29
#define    SYSCFG_IOCR7_RFGPIO3_MUX_MASK                                  0x20000000
#define    SYSCFG_IOCR7_RFGPIO2_MUX_SHIFT                                 28
#define    SYSCFG_IOCR7_RFGPIO2_MUX_MASK                                  0x10000000
#define    SYSCFG_IOCR7_PC_DRIVE_SHIFT                                    24
#define    SYSCFG_IOCR7_PC_DRIVE_MASK                                     0x07000000
#define    SYSCFG_IOCR7_GPENH_DRIVE_SHIFT                                 21
#define    SYSCFG_IOCR7_GPENH_DRIVE_MASK                                  0x00E00000
#define    SYSCFG_IOCR7_GPENL_DRIVE_SHIFT                                 18
#define    SYSCFG_IOCR7_GPENL_DRIVE_MASK                                  0x001C0000
#define    SYSCFG_IOCR7_RX3G_PULL_SHIFT                                   16
#define    SYSCFG_IOCR7_RX3G_PULL_MASK                                    0x00030000
#define    SYSCFG_IOCR7_TX3G_SLEW_SHIFT                                   15
#define    SYSCFG_IOCR7_TX3G_SLEW_MASK                                    0x00008000
#define    SYSCFG_IOCR7_TX3G_DRIVE_SHIFT                                  12
#define    SYSCFG_IOCR7_TX3G_DRIVE_MASK                                   0x00007000
#define    SYSCFG_IOCR7_CLKX8_SLEW_SHIFT                                  11
#define    SYSCFG_IOCR7_CLKX8_SLEW_MASK                                   0x00000800
#define    SYSCFG_IOCR7_CLKX8_DRIVE_SHIFT                                 8
#define    SYSCFG_IOCR7_CLKX8_DRIVE_MASK                                  0x00000700
#define    SYSCFG_IOCR7_AUXDAC0_PAVRAMP_SEL_SHIFT                         7
#define    SYSCFG_IOCR7_AUXDAC0_PAVRAMP_SEL_MASK                          0x00000080
#define    SYSCFG_IOCR7_DATASEL_3G2G_SHIFT                                4
#define    SYSCFG_IOCR7_DATASEL_3G2G_MASK                                 0x00000030
#define    SYSCFG_IOCR7_CTRLSEL_3G2G_SHIFT                                0
#define    SYSCFG_IOCR7_CTRLSEL_3G2G_MASK                                 0x00000001

#define SYSCFG_DPERSTR_OFFSET                                             0x00000030
#define SYSCFG_DPERSTR_TYPE                                               UInt32
#define SYSCFG_DPERSTR_RESERVED_MASK                                      0xFFFFFFFE
#define    SYSCFG_DPERSTR_DPESRST_SHIFT                                   0
#define    SYSCFG_DPERSTR_DPESRST_MASK                                    0x00000001

#define SYSCFG_TVENCCR_OFFSET                                             0x00000034
#define SYSCFG_TVENCCR_TYPE                                               UInt32
#define SYSCFG_TVENCCR_RESERVED_MASK                                      0xFFFFFFC8
#define    SYSCFG_TVENCCR_DMA_WAIT_CYCLE_SHIFT                            4
#define    SYSCFG_TVENCCR_DMA_WAIT_CYCLE_MASK                             0x00000030
#define    SYSCFG_TVENCCR_DISPLAYC_HRSTN_SHIFT                            2
#define    SYSCFG_TVENCCR_DISPLAYC_HRSTN_MASK                             0x00000004
#define    SYSCFG_TVENCCR_VEC_HRSTN_SHIFT                                 1
#define    SYSCFG_TVENCCR_VEC_HRSTN_MASK                                  0x00000002
#define    SYSCFG_TVENCCR_TVENC_EN_SHIFT                                  0
#define    SYSCFG_TVENCCR_TVENC_EN_MASK                                   0x00000001

#define SYSCFG_DSICR_OFFSET                                               0x00000038
#define SYSCFG_DSICR_TYPE                                                 UInt32
#define SYSCFG_DSICR_RESERVED_MASK                                        0xFFFFFFFE
#define    SYSCFG_DSICR_DSI_EN_SHIFT                                      0
#define    SYSCFG_DSICR_DSI_EN_MASK                                       0x00000001

#define SYSCFG_MCR_OFFSET                                                 0x00000040
#define SYSCFG_MCR_TYPE                                                   UInt32
#define SYSCFG_MCR_RESERVED_MASK                                          0xFFFFFFFE
#define    SYSCFG_MCR_REMAP_SHIFT                                         0
#define    SYSCFG_MCR_REMAP_MASK                                          0x00000001

#define SYSCFG_MRR_OFFSET                                                 0x00000044
#define SYSCFG_MRR_TYPE                                                   UInt32
#define SYSCFG_MRR_RESERVED_MASK                                          0x00000000
#define    SYSCFG_MRR_RESTORE_SHIFT                                       0
#define    SYSCFG_MRR_RESTORE_MASK                                        0xFFFFFFFF

#define SYSCFG_RAMCTRL_OFFSET                                             0x00000048
#define SYSCFG_RAMCTRL_TYPE                                               UInt32
#define SYSCFG_RAMCTRL_RESERVED_MASK                                      0xFFFEF800
#define    SYSCFG_RAMCTRL_RED_OTP_RST_SHIFT                               16
#define    SYSCFG_RAMCTRL_RED_OTP_RST_MASK                                0x00010000
#define    SYSCFG_RAMCTRL_ROM_TM_SHIFT                                    6
#define    SYSCFG_RAMCTRL_ROM_TM_MASK                                     0x000007C0
#define    SYSCFG_RAMCTRL_SRAM_TM_SHIFT                                   2
#define    SYSCFG_RAMCTRL_SRAM_TM_MASK                                    0x0000003C
#define    SYSCFG_RAMCTRL_STBY_SHIFT                                      0
#define    SYSCFG_RAMCTRL_STBY_MASK                                       0x00000003

#define SYSCFG_SECCTRL_OFFSET                                             0x00000050
#define SYSCFG_SECCTRL_TYPE                                               UInt32
#define SYSCFG_SECCTRL_RESERVED_MASK                                      0xFFFFFFE8
#define    SYSCFG_SECCTRL_CRYPTO_DIS_SHIFT                                4
#define    SYSCFG_SECCTRL_CRYPTO_DIS_MASK                                 0x00000010
#define    SYSCFG_SECCTRL_OTP_DIS_SHIFT                                   2
#define    SYSCFG_SECCTRL_OTP_DIS_MASK                                    0x00000004
#define    SYSCFG_SECCTRL_RTC_DIS_WR_SHIFT                                1
#define    SYSCFG_SECCTRL_RTC_DIS_WR_MASK                                 0x00000002
#define    SYSCFG_SECCTRL_BRM_DIS_RD_SHIFT                                0
#define    SYSCFG_SECCTRL_BRM_DIS_RD_MASK                                 0x00000001

#define SYSCFG_SECSTAT_OFFSET                                             0x00000054
#define SYSCFG_SECSTAT_TYPE                                               UInt32
#define SYSCFG_SECSTAT_RESERVED_MASK                                      0xC0000000
#define    SYSCFG_SECSTAT_SEC_MODE_STATE_SHIFT                            26
#define    SYSCFG_SECSTAT_SEC_MODE_STATE_MASK                             0x3C000000
#define    SYSCFG_SECSTAT_JTAG_DIS_SHIFT                                  25
#define    SYSCFG_SECSTAT_JTAG_DIS_MASK                                   0x02000000
#define    SYSCFG_SECSTAT_ETM_DIS_SHIFT                                   24
#define    SYSCFG_SECSTAT_ETM_DIS_MASK                                    0x01000000
#define    SYSCFG_SECSTAT_SECURE_DEBUG_SHIFT                              0
#define    SYSCFG_SECSTAT_SECURE_DEBUG_MASK                               0x00FFFFFF

#define SYSCFG_MAMBACR_OFFSET                                             0x00000060
#define SYSCFG_MAMBACR_TYPE                                               UInt32
#define SYSCFG_MAMBACR_RESERVED_MASK                                      0x00FFF401
#define    SYSCFG_MAMBACR_MAMBA_STATUS_SHIFT                              24
#define    SYSCFG_MAMBACR_MAMBA_STATUS_MASK                               0xFF000000
#define    SYSCFG_MAMBACR_NOR_REQUEST_SHIFT                               11
#define    SYSCFG_MAMBACR_NOR_REQUEST_MASK                                0x00000800
#define    SYSCFG_MAMBACR_PWRDN_EXIT_SHIFT                                9
#define    SYSCFG_MAMBACR_PWRDN_EXIT_MASK                                 0x00000200
#define    SYSCFG_MAMBACR_HIB_EXIT_MODE_SHIFT                             8
#define    SYSCFG_MAMBACR_HIB_EXIT_MODE_MASK                              0x00000100
#define    SYSCFG_MAMBACR_FREQ_CHANGE_REQ_SHIFT                           7
#define    SYSCFG_MAMBACR_FREQ_CHANGE_REQ_MASK                            0x00000080
#define    SYSCFG_MAMBACR_EMI_TEST_SHIFT                                  6
#define    SYSCFG_MAMBACR_EMI_TEST_MASK                                   0x00000040
#define    SYSCFG_MAMBACR_DEBUG_CTRL_SHIFT                                2
#define    SYSCFG_MAMBACR_DEBUG_CTRL_MASK                                 0x0000003C
#define    SYSCFG_MAMBACR_CLK_EMI_EQ_DRAM_SHIFT                           1
#define    SYSCFG_MAMBACR_CLK_EMI_EQ_DRAM_MASK                            0x00000002

#define SYSCFG_RFCR_OFFSET                                                0x00000064
#define SYSCFG_RFCR_TYPE                                                  UInt32
#define SYSCFG_RFCR_RESERVED_MASK                                         0xFFFFFFFF

#define SYSCFG_ANACR0_OFFSET                                              0x00000080
#define SYSCFG_ANACR0_TYPE                                                UInt32
#define SYSCFG_ANACR0_RESERVED_MASK                                       0x00000000
#define    SYSCFG_ANACR0_CLKINV_SHIFT                                     31
#define    SYSCFG_ANACR0_CLKINV_MASK                                      0x80000000
#define    SYSCFG_ANACR0_PGA_CTRLR_SHIFT                                  28
#define    SYSCFG_ANACR0_PGA_CTRLR_MASK                                   0x70000000
#define    SYSCFG_ANACR0_PGA_CTRLL_SHIFT                                  25
#define    SYSCFG_ANACR0_PGA_CTRLL_MASK                                   0x0E000000
#define    SYSCFG_ANACR0_MUTE_SELR_SHIFT                                  23
#define    SYSCFG_ANACR0_MUTE_SELR_MASK                                   0x01800000
#define    SYSCFG_ANACR0_MUTE_SELL_SHIFT                                  21
#define    SYSCFG_ANACR0_MUTE_SELL_MASK                                   0x00600000
#define    SYSCFG_ANACR0_MODE_SELR_SHIFT                                  18
#define    SYSCFG_ANACR0_MODE_SELR_MASK                                   0x001C0000
#define    SYSCFG_ANACR0_MODE_SELL_SHIFT                                  15
#define    SYSCFG_ANACR0_MODE_SELL_MASK                                   0x00038000
#define    SYSCFG_ANACR0_DRVR_SELR_SHIFT                                  14
#define    SYSCFG_ANACR0_DRVR_SELR_MASK                                   0x00004000
#define    SYSCFG_ANACR0_DRVR_SELL_SHIFT                                  13
#define    SYSCFG_ANACR0_DRVR_SELL_MASK                                   0x00002000
#define    SYSCFG_ANACR0_CM_SEL_SHIFT                                     10
#define    SYSCFG_ANACR0_CM_SEL_MASK                                      0x00001C00
#define    SYSCFG_ANACR0_IQ_DBL_SHIFT                                     9
#define    SYSCFG_ANACR0_IQ_DBL_MASK                                      0x00000200
#define    SYSCFG_ANACR0_BGTCSP_SHIFT                                     6
#define    SYSCFG_ANACR0_BGTCSP_MASK                                      0x000001C0
#define    SYSCFG_ANACR0_PWRDNREF_SHIFT                                   5
#define    SYSCFG_ANACR0_PWRDNREF_MASK                                    0x00000020
#define    SYSCFG_ANACR0_PWRDNDRVR_SHIFT                                  4
#define    SYSCFG_ANACR0_PWRDNDRVR_MASK                                   0x00000010
#define    SYSCFG_ANACR0_PWRDNDRVL_SHIFT                                  3
#define    SYSCFG_ANACR0_PWRDNDRVL_MASK                                   0x00000008
#define    SYSCFG_ANACR0_PWRDNDACR_SHIFT                                  2
#define    SYSCFG_ANACR0_PWRDNDACR_MASK                                   0x00000004
#define    SYSCFG_ANACR0_PWRDNDACL_SHIFT                                  1
#define    SYSCFG_ANACR0_PWRDNDACL_MASK                                   0x00000002
#define    SYSCFG_ANACR0_PWRDND2C_SHIFT                                   0
#define    SYSCFG_ANACR0_PWRDND2C_MASK                                    0x00000001

#define SYSCFG_ANACR1_OFFSET                                              0x00000084
#define SYSCFG_ANACR1_TYPE                                                UInt32
#define SYSCFG_ANACR1_RESERVED_MASK                                       0x03FFFE7C
#define    SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENL_SHIFT                     31
#define    SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENL_MASK                      0x80000000
#define    SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENR_SHIFT                     30
#define    SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENR_MASK                      0x40000000
#define    SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLL_SHIFT                29
#define    SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLL_MASK                 0x20000000
#define    SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLR_SHIFT                28
#define    SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLR_MASK                 0x10000000
#define    SYSCFG_ANACR1_IHFEP_DAC_OFFSETDBL_SHIFT                        27
#define    SYSCFG_ANACR1_IHFEP_DAC_OFFSETDBL_MASK                         0x08000000
#define    SYSCFG_ANACR1_IHFEP_DAC_RAMPREF_EN_SHIFT                       26
#define    SYSCFG_ANACR1_IHFEP_DAC_RAMPREF_EN_MASK                        0x04000000
#define    SYSCFG_ANACR1_HEADSET_DAC_PWD_MASK_SHIFT                       8
#define    SYSCFG_ANACR1_HEADSET_DAC_PWD_MASK_MASK                        0x00000100
#define    SYSCFG_ANACR1_IHF_EARPIECE_DAC_FORCE_PWRUP_DISABLE_SHIFT       7
#define    SYSCFG_ANACR1_IHF_EARPIECE_DAC_FORCE_PWRUP_DISABLE_MASK        0x00000080
#define    SYSCFG_ANACR1_I_CLKPHASE_SHIFT                                 1
#define    SYSCFG_ANACR1_I_CLKPHASE_MASK                                  0x00000002
#define    SYSCFG_ANACR1_I_CLKPHSDIS_SHIFT                                0
#define    SYSCFG_ANACR1_I_CLKPHSDIS_MASK                                 0x00000001

#define SYSCFG_ANACR2_OFFSET                                              0x00000088
#define SYSCFG_ANACR2_TYPE                                                UInt32
#define SYSCFG_ANACR2_RESERVED_MASK                                       0x00070030
#define    SYSCFG_ANACR2_I_PGA_BIAS_GM0_SHIFT                             31
#define    SYSCFG_ANACR2_I_PGA_BIAS_GM0_MASK                              0x80000000
#define    SYSCFG_ANACR2_I_PGA_BIAS_BUF_SHIFT                             29
#define    SYSCFG_ANACR2_I_PGA_BIAS_BUF_MASK                              0x60000000
#define    SYSCFG_ANACR2_I_PGA_BIAS_AMP_SHIFT                             27
#define    SYSCFG_ANACR2_I_PGA_BIAS_AMP_MASK                              0x18000000
#define    SYSCFG_ANACR2_I_PGA_RI_FINE_SHIFT                              24
#define    SYSCFG_ANACR2_I_PGA_RI_FINE_MASK                               0x07000000
#define    SYSCFG_ANACR2_I_PGA_RI_CTL_SHIFT                               22
#define    SYSCFG_ANACR2_I_PGA_RI_CTL_MASK                                0x00C00000
#define    SYSCFG_ANACR2_I_BGTC_SHIFT                                     19
#define    SYSCFG_ANACR2_I_BGTC_MASK                                      0x00380000
#define    SYSCFG_ANACR2_I_LDO_VOUT_SHIFT                                 14
#define    SYSCFG_ANACR2_I_LDO_VOUT_MASK                                  0x0000C000
#define    SYSCFG_ANACR2_I_PLL_TESTSEL_SHIFT                              13
#define    SYSCFG_ANACR2_I_PLL_TESTSEL_MASK                               0x00002000
#define    SYSCFG_ANACR2_REFSHIFT_SHIFT                                   11
#define    SYSCFG_ANACR2_REFSHIFT_MASK                                    0x00001800
#define    SYSCFG_ANACR2_BGTC_SHIFT                                       8
#define    SYSCFG_ANACR2_BGTC_MASK                                        0x00000700
#define    SYSCFG_ANACR2_I_LDO_I_SHIFT                                    6
#define    SYSCFG_ANACR2_I_LDO_I_MASK                                     0x000000C0
#define    SYSCFG_ANACR2_I_PGA_ADC_STANDBY_SHIFT                          3
#define    SYSCFG_ANACR2_I_PGA_ADC_STANDBY_MASK                           0x00000008
#define    SYSCFG_ANACR2_I_PGA_ADC_PWRUP_SHIFT                            2
#define    SYSCFG_ANACR2_I_PGA_ADC_PWRUP_MASK                             0x00000004
#define    SYSCFG_ANACR2_I_MIC_VOICE_PWRDN_SHIFT                          1
#define    SYSCFG_ANACR2_I_MIC_VOICE_PWRDN_MASK                           0x00000002
#define    SYSCFG_ANACR2_I_LDO_PWRDN_SHIFT                                0
#define    SYSCFG_ANACR2_I_LDO_PWRDN_MASK                                 0x00000001

#define SYSCFG_ANACR3_OFFSET                                              0x0000008C
#define SYSCFG_ANACR3_TYPE                                                UInt32
#define SYSCFG_ANACR3_RESERVED_MASK                                       0x00000000
#define    SYSCFG_ANACR3_I_INTVOCMCT_SHIFT                                29
#define    SYSCFG_ANACR3_I_INTVOCMCT_MASK                                 0xE0000000
#define    SYSCFG_ANACR3_I_INT2BCTL_SHIFT                                 27
#define    SYSCFG_ANACR3_I_INT2BCTL_MASK                                  0x18000000
#define    SYSCFG_ANACR3_I_INT1BCTL_SHIFT                                 25
#define    SYSCFG_ANACR3_I_INT1BCTL_MASK                                  0x06000000
#define    SYSCFG_ANACR3_I_FLASHBCTL_SHIFT                                23
#define    SYSCFG_ANACR3_I_FLASHBCTL_MASK                                 0x01800000
#define    SYSCFG_ANACR3_I_DITHCTL_SHIFT                                  21
#define    SYSCFG_ANACR3_I_DITHCTL_MASK                                   0x00600000
#define    SYSCFG_ANACR3_I_DACREFCTL_SHIFT                                18
#define    SYSCFG_ANACR3_I_DACREFCTL_MASK                                 0x001C0000
#define    SYSCFG_ANACR3_I_CKBY2EN_SHIFT                                  17
#define    SYSCFG_ANACR3_I_CKBY2EN_MASK                                   0x00020000
#define    SYSCFG_ANACR3_ARX_FORCE_PWRUP_DISABLE_SHIFT                    16
#define    SYSCFG_ANACR3_ARX_FORCE_PWRUP_DISABLE_MASK                     0x00010000
#define    SYSCFG_ANACR3_I_PGA_MUX_SEL_SHIFT                              15
#define    SYSCFG_ANACR3_I_PGA_MUX_SEL_MASK                               0x00008000
#define    SYSCFG_ANACR3_I_PGA_GAIN_SHIFT                                 9
#define    SYSCFG_ANACR3_I_PGA_GAIN_MASK                                  0x00007E00
#define    SYSCFG_ANACR3_I_PGA_CMO_CTL_SHIFT                              6
#define    SYSCFG_ANACR3_I_PGA_CMO_CTL_MASK                               0x000001C0
#define    SYSCFG_ANACR3_I_PGA_CMI_CTL_SHIFT                              3
#define    SYSCFG_ANACR3_I_PGA_CMI_CTL_MASK                               0x00000038
#define    SYSCFG_ANACR3_I_PGA_BIAS_VBIAS_SHIFT                           1
#define    SYSCFG_ANACR3_I_PGA_BIAS_VBIAS_MASK                            0x00000006
#define    SYSCFG_ANACR3_I_PGA_BIAS_GM1_SHIFT                             0
#define    SYSCFG_ANACR3_I_PGA_BIAS_GM1_MASK                              0x00000001

#define SYSCFG_ANACR4_OFFSET                                              0x00000090
#define SYSCFG_ANACR4_TYPE                                                UInt32
#define SYSCFG_ANACR4_RESERVED_MASK                                       0xFFFFFFE0
#define    SYSCFG_ANACR4_I_MIC_AUX_GND_SHIFT                              4
#define    SYSCFG_ANACR4_I_MIC_AUX_GND_MASK                               0x00000010
#define    SYSCFG_ANACR4_I_SHUFFCTL_SHIFT                                 2
#define    SYSCFG_ANACR4_I_SHUFFCTL_MASK                                  0x0000000C
#define    SYSCFG_ANACR4_I_REFAMPBCTL_SHIFT                               0
#define    SYSCFG_ANACR4_I_REFAMPBCTL_MASK                                0x00000003

#define SYSCFG_ANACR5_OFFSET                                              0x00000094
#define SYSCFG_ANACR5_TYPE                                                UInt32
#define SYSCFG_ANACR5_RESERVED_MASK                                       0x3F81F800
#define    SYSCFG_ANACR5_BGTC_SHIFT                                       30
#define    SYSCFG_ANACR5_BGTC_MASK                                        0xC0000000
#define    SYSCFG_ANACR5_AUX_ADC_SC_SHIFT                                 21
#define    SYSCFG_ANACR5_AUX_ADC_SC_MASK                                  0x00600000
#define    SYSCFG_ANACR5_AUX_DAC_SI_SHIFT                                 19
#define    SYSCFG_ANACR5_AUX_DAC_SI_MASK                                  0x00180000
#define    SYSCFG_ANACR5_AUX_DAC0_SC_SHIFT                                17
#define    SYSCFG_ANACR5_AUX_DAC0_SC_MASK                                 0x00060000
#define    SYSCFG_ANACR5_AUX_DAC_CM_SHIFT                                 7
#define    SYSCFG_ANACR5_AUX_DAC_CM_MASK                                  0x00000780
#define    SYSCFG_ANACR5_AUX_DAC_PD_SHIFT                                 6
#define    SYSCFG_ANACR5_AUX_DAC_PD_MASK                                  0x00000040
#define    SYSCFG_ANACR5_AUX_DAC_IBIAS_SHIFT                              4
#define    SYSCFG_ANACR5_AUX_DAC_IBIAS_MASK                               0x00000030
#define    SYSCFG_ANACR5_AUX_CLK_CTRL_SHIFT                               3
#define    SYSCFG_ANACR5_AUX_CLK_CTRL_MASK                                0x00000008
#define    SYSCFG_ANACR5_AUX_CLK_INV_SHIFT                                2
#define    SYSCFG_ANACR5_AUX_CLK_INV_MASK                                 0x00000004
#define    SYSCFG_ANACR5_AUX_OUTPUT_SHIFT                                 1
#define    SYSCFG_ANACR5_AUX_OUTPUT_MASK                                  0x00000002
#define    SYSCFG_ANACR5_PWD_AUX_DAC_SHIFT                                0
#define    SYSCFG_ANACR5_PWD_AUX_DAC_MASK                                 0x00000001

#define SYSCFG_ANACR6_OFFSET                                              0x00000098
#define SYSCFG_ANACR6_TYPE                                                UInt32
#define SYSCFG_ANACR6_RESERVED_MASK                                       0xFFF9F800
#define    SYSCFG_ANACR6_AUXDAC1SC_SHIFT                                  17
#define    SYSCFG_ANACR6_AUXDAC1SC_MASK                                   0x00060000
#define    SYSCFG_ANACR6_AUX_DAC_CM_SHIFT                                 7
#define    SYSCFG_ANACR6_AUX_DAC_CM_MASK                                  0x00000780
#define    SYSCFG_ANACR6_AUX_DAC_PD_SHIFT                                 6
#define    SYSCFG_ANACR6_AUX_DAC_PD_MASK                                  0x00000040
#define    SYSCFG_ANACR6_AUX_DAC_IBIAS_SHIFT                              4
#define    SYSCFG_ANACR6_AUX_DAC_IBIAS_MASK                               0x00000030
#define    SYSCFG_ANACR6_AUX_CLK_CTRL_SHIFT                               3
#define    SYSCFG_ANACR6_AUX_CLK_CTRL_MASK                                0x00000008
#define    SYSCFG_ANACR6_AUX_CLK_INV_SHIFT                                2
#define    SYSCFG_ANACR6_AUX_CLK_INV_MASK                                 0x00000004
#define    SYSCFG_ANACR6_AUX_OUTPUT_SHIFT                                 1
#define    SYSCFG_ANACR6_AUX_OUTPUT_MASK                                  0x00000002
#define    SYSCFG_ANACR6_PWD_AUX_DAC_SHIFT                                0
#define    SYSCFG_ANACR6_PWD_AUX_DAC_MASK                                 0x00000001

#define SYSCFG_ANACR7_OFFSET                                              0x0000009C
#define SYSCFG_ANACR7_TYPE                                                UInt32
#define SYSCFG_ANACR7_RESERVED_MASK                                       0xA0000000
#define    SYSCFG_ANACR7_RXSC_SHIFT                                       30
#define    SYSCFG_ANACR7_RXSC_MASK                                        0x40000000
#define    SYSCFG_ANACR7_DISABLEHVREGULATOR_SHIFT                         28
#define    SYSCFG_ANACR7_DISABLEHVREGULATOR_MASK                          0x10000000
#define    SYSCFG_ANACR7_INPUTCMENABLE_SHIFT                              27
#define    SYSCFG_ANACR7_INPUTCMENABLE_MASK                               0x08000000
#define    SYSCFG_ANACR7_ALBSELECT_SHIFT                                  26
#define    SYSCFG_ANACR7_ALBSELECT_MASK                                   0x04000000
#define    SYSCFG_ANACR7_SHUFFLECTRL_SHIFT                                24
#define    SYSCFG_ANACR7_SHUFFLECTRL_MASK                                 0x03000000
#define    SYSCFG_ANACR7_DITHCTRL_SHIFT                                   22
#define    SYSCFG_ANACR7_DITHCTRL_MASK                                    0x00C00000
#define    SYSCFG_ANACR7_VCMCTRL_FLASH_SHIFT                              20
#define    SYSCFG_ANACR7_VCMCTRL_FLASH_MASK                               0x00300000
#define    SYSCFG_ANACR7_VCMCTRL_INT2_SHIFT                               18
#define    SYSCFG_ANACR7_VCMCTRL_INT2_MASK                                0x000C0000
#define    SYSCFG_ANACR7_VCMCTRL_INT1_SHIFT                               16
#define    SYSCFG_ANACR7_VCMCTRL_INT1_MASK                                0x00030000
#define    SYSCFG_ANACR7_DITHREFOFFSETCTRL_SHIFT                          14
#define    SYSCFG_ANACR7_DITHREFOFFSETCTRL_MASK                           0x0000C000
#define    SYSCFG_ANACR7_ADCREFOFFSETCTRL_SHIFT                           12
#define    SYSCFG_ANACR7_ADCREFOFFSETCTRL_MASK                            0x00003000
#define    SYSCFG_ANACR7_BIASCTRL_INT2_SHIFT                              10
#define    SYSCFG_ANACR7_BIASCTRL_INT2_MASK                               0x00000C00
#define    SYSCFG_ANACR7_BIASCTRL_INT1_SHIFT                              8
#define    SYSCFG_ANACR7_BIASCTRL_INT1_MASK                               0x00000300
#define    SYSCFG_ANACR7_BIASCTRL_ADC_SHIFT                               6
#define    SYSCFG_ANACR7_BIASCTRL_ADC_MASK                                0x000000C0
#define    SYSCFG_ANACR7_RXPGASET_SHIFT                                   2
#define    SYSCFG_ANACR7_RXPGASET_MASK                                    0x0000003C
#define    SYSCFG_ANACR7_RXRESETB_SHIFT                                   1
#define    SYSCFG_ANACR7_RXRESETB_MASK                                    0x00000002
#define    SYSCFG_ANACR7_RXPWRDN_SHIFT                                    0
#define    SYSCFG_ANACR7_RXPWRDN_MASK                                     0x00000001

#define SYSCFG_ANACR8_OFFSET                                              0x000000A0
#define SYSCFG_ANACR8_TYPE                                                UInt32
#define SYSCFG_ANACR8_RESERVED_MASK                                       0xFC400000
#define    SYSCFG_ANACR8_IBCASCTRL_SHIFT                                  23
#define    SYSCFG_ANACR8_IBCASCTRL_MASK                                   0x03800000
#define    SYSCFG_ANACR8_IBCMCTRL_SHIFT                                   20
#define    SYSCFG_ANACR8_IBCMCTRL_MASK                                    0x00300000
#define    SYSCFG_ANACR8_IBAMPCTRL_SHIFT                                  17
#define    SYSCFG_ANACR8_IBAMPCTRL_MASK                                   0x000E0000
#define    SYSCFG_ANACR8_TXAMPCTRL_SHIFT                                  13
#define    SYSCFG_ANACR8_TXAMPCTRL_MASK                                   0x0001E000
#define    SYSCFG_ANACR8_TXVCMCTRL_SHIFT                                  10
#define    SYSCFG_ANACR8_TXVCMCTRL_MASK                                   0x00001C00
#define    SYSCFG_ANACR8_CLK_DISABLE_SHIFT                                9
#define    SYSCFG_ANACR8_CLK_DISABLE_MASK                                 0x00000200
#define    SYSCFG_ANACR8_CLKINV_SHIFT                                     8
#define    SYSCFG_ANACR8_CLKINV_MASK                                      0x00000100
#define    SYSCFG_ANACR8_TXOBB_SHIFT                                      7
#define    SYSCFG_ANACR8_TXOBB_MASK                                       0x00000080
#define    SYSCFG_ANACR8_TXSC_SHIFT                                       5
#define    SYSCFG_ANACR8_TXSC_MASK                                        0x00000060
#define    SYSCFG_ANACR8_TXSIQ_SHIFT                                      3
#define    SYSCFG_ANACR8_TXSIQ_MASK                                       0x00000018
#define    SYSCFG_ANACR8_TXSII_SHIFT                                      1
#define    SYSCFG_ANACR8_TXSII_MASK                                       0x00000006
#define    SYSCFG_ANACR8_PWRDNTX_SHIFT                                    0
#define    SYSCFG_ANACR8_PWRDNTX_MASK                                     0x00000001

#define SYSCFG_ANACR9_OFFSET                                              0x000000A4
#define SYSCFG_ANACR9_TYPE                                                UInt32
#define SYSCFG_ANACR9_RESERVED_MASK                                       0x00001C00
#define    SYSCFG_ANACR9_SYNC_DET_LENGTH_SHIFT                            29
#define    SYSCFG_ANACR9_SYNC_DET_LENGTH_MASK                             0xE0000000
#define    SYSCFG_ANACR9_TX_PHASE_SHIFT                                   28
#define    SYSCFG_ANACR9_TX_PHASE_MASK                                    0x10000000
#define    SYSCFG_ANACR9_AFE_CHRPTEN_SHIFT                                27
#define    SYSCFG_ANACR9_AFE_CHRPTEN_MASK                                 0x08000000
#define    SYSCFG_ANACR9_AFE_LPBACK_SHIFT                                 26
#define    SYSCFG_ANACR9_AFE_LPBACK_MASK                                  0x04000000
#define    SYSCFG_ANACR9_AFE_CDRCKEN_SHIFT                                25
#define    SYSCFG_ANACR9_AFE_CDRCKEN_MASK                                 0x02000000
#define    SYSCFG_ANACR9_FS_LS_CROSS_OVER_SHIFT                           24
#define    SYSCFG_ANACR9_FS_LS_CROSS_OVER_MASK                            0x01000000
#define    SYSCFG_ANACR9_AFE_HSTXEN_SHIFT                                 23
#define    SYSCFG_ANACR9_AFE_HSTXEN_MASK                                  0x00800000
#define    SYSCFG_ANACR9_ECN_ENABLE_SHIFT                                 22
#define    SYSCFG_ANACR9_ECN_ENABLE_MASK                                  0x00400000
#define    SYSCFG_ANACR9_UTMI_LOOPBACK_SHIFT                              21
#define    SYSCFG_ANACR9_UTMI_LOOPBACK_MASK                               0x00200000
#define    SYSCFG_ANACR9_AFE_RXLOGICR_SHIFT                               20
#define    SYSCFG_ANACR9_AFE_RXLOGICR_MASK                                0x00100000
#define    SYSCFG_ANACR9_IOST_CONTROL_SHIFT                               18
#define    SYSCFG_ANACR9_IOST_CONTROL_MASK                                0x000C0000
#define    SYSCFG_ANACR9_SUSPEND_PLL_PDN_SHIFT                            17
#define    SYSCFG_ANACR9_SUSPEND_PLL_PDN_MASK                             0x00020000
#define    SYSCFG_ANACR9_PLL_LOCK_DIS_SHIFT                               16
#define    SYSCFG_ANACR9_PLL_LOCK_DIS_MASK                                0x00010000
#define    SYSCFG_ANACR9_IDDQ_EN_SHIFT                                    15
#define    SYSCFG_ANACR9_IDDQ_EN_MASK                                     0x00008000
#define    SYSCFG_ANACR9_UTMI_DISCON_PHY_SHIFT                            14
#define    SYSCFG_ANACR9_UTMI_DISCON_PHY_MASK                             0x00004000
#define    SYSCFG_ANACR9_PLL_BYPASS_SHIFT                                 13
#define    SYSCFG_ANACR9_PLL_BYPASS_MASK                                  0x00002000
#define    SYSCFG_ANACR9_RESET_HI_PLL_SHIFT                               9
#define    SYSCFG_ANACR9_RESET_HI_PLL_MASK                                0x00000200
#define    SYSCFG_ANACR9_UTMI_L1_SUSPENDM_SHIFT                           8
#define    SYSCFG_ANACR9_UTMI_L1_SUSPENDM_MASK                            0x00000100
#define    SYSCFG_ANACR9_AFE_NON_DRIVING_SHIFT                            7
#define    SYSCFG_ANACR9_AFE_NON_DRIVING_MASK                             0x00000080
#define    SYSCFG_ANACR9_UTMI_SLEEPM_SHIFT                                6
#define    SYSCFG_ANACR9_UTMI_SLEEPM_MASK                                 0x00000040
#define    SYSCFG_ANACR9_SOFT_RESETB_SHIFT                                5
#define    SYSCFG_ANACR9_SOFT_RESETB_MASK                                 0x00000020
#define    SYSCFG_ANACR9_PLL_PWRDWNB_SHIFT                                4
#define    SYSCFG_ANACR9_PLL_PWRDWNB_MASK                                 0x00000010
#define    SYSCFG_ANACR9_PLL_CALEN_SHIFT                                  3
#define    SYSCFG_ANACR9_PLL_CALEN_MASK                                   0x00000008
#define    SYSCFG_ANACR9_OTG_MODE_SHIFT                                   2
#define    SYSCFG_ANACR9_OTG_MODE_MASK                                    0x00000004
#define    SYSCFG_ANACR9_HOSTB_DEV_SHIFT                                  1
#define    SYSCFG_ANACR9_HOSTB_DEV_MASK                                   0x00000002
#define    SYSCFG_ANACR9_PHY_PWRDWNB_SHIFT                                0
#define    SYSCFG_ANACR9_PHY_PWRDWNB_MASK                                 0x00000001

#define SYSCFG_ANACR10_OFFSET                                             0x000000A8
#define SYSCFG_ANACR10_TYPE                                               UInt32
#define SYSCFG_ANACR10_RESERVED_MASK                                      0x80203FC0
#define    SYSCFG_ANACR10_I_CKDIV_SHIFT                                   29
#define    SYSCFG_ANACR10_I_CKDIV_MASK                                    0x60000000
#define    SYSCFG_ANACR10_I_CKADJ_SHIFT                                   25
#define    SYSCFG_ANACR10_I_CKADJ_MASK                                    0x1E000000
#define    SYSCFG_ANACR10_I_BGTC_SHIFT                                    22
#define    SYSCFG_ANACR10_I_BGTC_MASK                                     0x01C00000
#define    SYSCFG_ANACR10_I_CKSEL_SHIFT                                   20
#define    SYSCFG_ANACR10_I_CKSEL_MASK                                    0x00100000
#define    SYSCFG_ANACR10_I_CLKINV_SHIFT                                  19
#define    SYSCFG_ANACR10_I_CLKINV_MASK                                   0x00080000
#define    SYSCFG_ANACR10_CLKPHASEDIS_SHIFT                               18
#define    SYSCFG_ANACR10_CLKPHASEDIS_MASK                                0x00040000
#define    SYSCFG_ANACR10_CLKTEST_SHIFT                                   17
#define    SYSCFG_ANACR10_CLKTEST_MASK                                    0x00020000
#define    SYSCFG_ANACR10_CLKPHASE_SHIFT                                  16
#define    SYSCFG_ANACR10_CLKPHASE_MASK                                   0x00010000
#define    SYSCFG_ANACR10_CLKRESET_SHIFT                                  15
#define    SYSCFG_ANACR10_CLKRESET_MASK                                   0x00008000
#define    SYSCFG_ANACR10_BUFBYPASS_SHIFT                                 14
#define    SYSCFG_ANACR10_BUFBYPASS_MASK                                  0x00004000
#define    SYSCFG_ANACR10_PWRDNREF_SHIFT                                  5
#define    SYSCFG_ANACR10_PWRDNREF_MASK                                   0x00000020
#define    SYSCFG_ANACR10_PWRDNDRVR_SHIFT                                 4
#define    SYSCFG_ANACR10_PWRDNDRVR_MASK                                  0x00000010
#define    SYSCFG_ANACR10_PWRDNDRVL_SHIFT                                 3
#define    SYSCFG_ANACR10_PWRDNDRVL_MASK                                  0x00000008
#define    SYSCFG_ANACR10_PWRDNDACR_SHIFT                                 2
#define    SYSCFG_ANACR10_PWRDNDACR_MASK                                  0x00000004
#define    SYSCFG_ANACR10_PWRDNDACL_SHIFT                                 1
#define    SYSCFG_ANACR10_PWRDNDACL_MASK                                  0x00000002
#define    SYSCFG_ANACR10_PWRDND2C_SHIFT                                  0
#define    SYSCFG_ANACR10_PWRDND2C_MASK                                   0x00000001

#define SYSCFG_ANACR11_OFFSET                                             0x000000AC
#define SYSCFG_ANACR11_TYPE                                               UInt32
#define SYSCFG_ANACR11_RESERVED_MASK                                      0xFFFFFFFF

#define SYSCFG_ANACR12_OFFSET                                             0x000000B0
#define SYSCFG_ANACR12_TYPE                                               UInt32
#define SYSCFG_ANACR12_RESERVED_MASK                                      0xFFFF8080
#define    SYSCFG_ANACR12_MICINVAL_SHIFT                                  8
#define    SYSCFG_ANACR12_MICINVAL_MASK                                   0x00007F00
#define    SYSCFG_ANACR12_MICONVAL_SHIFT                                  0
#define    SYSCFG_ANACR12_MICONVAL_MASK                                   0x0000007F

#define SYSCFG_ANACR13_OFFSET                                             0x000000B4
#define SYSCFG_ANACR13_TYPE                                               UInt32
#define SYSCFG_ANACR13_RESERVED_MASK                                      0xFFFFFFFF

#define SYSCFG_ANACR14_OFFSET                                             0x000000B8
#define SYSCFG_ANACR14_TYPE                                               UInt32
#define SYSCFG_ANACR14_RESERVED_MASK                                      0xFFFFFFFF

#define SYSCFG_ANACR15_OFFSET                                             0x000000BC
#define SYSCFG_ANACR15_TYPE                                               UInt32
#define SYSCFG_ANACR15_RESERVED_MASK                                      0xFFFFFFFF

#define SYSCFG_IRDROP_MON0_OFFSET                                         0x000000C0
#define SYSCFG_IRDROP_MON0_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON0_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON0_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON0_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON0_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON0_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON0_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON0_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_IRDROP_MON1_OFFSET                                         0x000000C4
#define SYSCFG_IRDROP_MON1_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON1_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON1_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON1_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON1_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON1_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON1_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON1_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_IRDROP_MON2_OFFSET                                         0x000000C8
#define SYSCFG_IRDROP_MON2_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON2_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON2_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON2_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON2_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON2_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON2_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON2_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_MDIO_WRITE_OFFSET                                          0x000000CC
#define SYSCFG_MDIO_WRITE_TYPE                                            UInt32
#define SYSCFG_MDIO_WRITE_RESERVED_MASK                                   0x00000000
#define    SYSCFG_MDIO_WRITE_WRITE_SHIFT                                  31
#define    SYSCFG_MDIO_WRITE_WRITE_MASK                                   0x80000000
#define    SYSCFG_MDIO_WRITE_READ_SHIFT                                   30
#define    SYSCFG_MDIO_WRITE_READ_MASK                                    0x40000000
#define    SYSCFG_MDIO_WRITE_MDIO3_SM_SEL_SHIFT                           29
#define    SYSCFG_MDIO_WRITE_MDIO3_SM_SEL_MASK                            0x20000000
#define    SYSCFG_MDIO_WRITE_MDIO2_SHIFT                                  24
#define    SYSCFG_MDIO_WRITE_MDIO2_MASK                                   0x1F000000
#define    SYSCFG_MDIO_WRITE_MDIO1_SHIFT                                  16
#define    SYSCFG_MDIO_WRITE_MDIO1_MASK                                   0x00FF0000
#define    SYSCFG_MDIO_WRITE_REG_SHIFT                                    0
#define    SYSCFG_MDIO_WRITE_REG_MASK                                     0x0000FFFF

#define SYSCFG_MDIO_READ_OFFSET                                           0x000000D0
#define SYSCFG_MDIO_READ_TYPE                                             UInt32
#define SYSCFG_MDIO_READ_RESERVED_MASK                                    0xFFFC0000
#define    SYSCFG_MDIO_READ_VBUS_STAT2_SHIFT                              17
#define    SYSCFG_MDIO_READ_VBUS_STAT2_MASK                               0x00020000
#define    SYSCFG_MDIO_READ_VBUS_STAT1_SHIFT                              16
#define    SYSCFG_MDIO_READ_VBUS_STAT1_MASK                               0x00010000
#define    SYSCFG_MDIO_READ_REG_RD_DATA_SHIFT                             0
#define    SYSCFG_MDIO_READ_REG_RD_DATA_MASK                              0x0000FFFF

#define SYSCFG_PERIPH_AHB_CLK_GATE_MASK_OFFSET                            0x000000D8
#define SYSCFG_PERIPH_AHB_CLK_GATE_MASK_TYPE                              UInt32
#define SYSCFG_PERIPH_AHB_CLK_GATE_MASK_RESERVED_MASK                     0x00000000
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MASK_PERIPH_AHB_CLK_GATE_MASK_SHIFT 0
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MASK_PERIPH_AHB_CLK_GATE_MASK_MASK  0xFFFFFFFF

#define SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_OFFSET                           0x000000DC
#define SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_TYPE                             UInt32
#define SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_RESERVED_MASK                    0x00000000
#define    SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_PERIPH_AHB_CLK_GATE_FORCE_SHIFT 0
#define    SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_PERIPH_AHB_CLK_GATE_FORCE_MASK 0xFFFFFFFF

#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_OFFSET                     0x000000E0
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_TYPE                       UInt32
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_RESERVED_MASK              0x00000000
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_PERIPH_AHB_CLK_GATE_MON_RAW_SHIFT 0
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_PERIPH_AHB_CLK_GATE_MON_RAW_MASK 0xFFFFFFFF

#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_OFFSET                         0x000000E4
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_TYPE                           UInt32
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RESERVED_MASK                  0x00000000
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_PERIPH_AHB_CLK_GATE_MON_SHIFT 0
#define    SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_PERIPH_AHB_CLK_GATE_MON_MASK 0xFFFFFFFF

#define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_OFFSET                       0x00000100
#define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_TYPE                         UInt32
#define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_RESERVED_MASK                0xFFFFFFFE
#define    SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_EN_SHIFT                  0
#define    SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_EN_MASK                   0x00000001

#define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_OFFSET                  0x00000104
#define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_TYPE                    UInt32
#define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_RESERVED_MASK           0xFFFFFFFE
#define    SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_EN_SHIFT             0
#define    SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_EN_MASK              0x00000001

#define SYSCFG_PERIPH_USB_AHB_CLK_EN_OFFSET                               0x00000108
#define SYSCFG_PERIPH_USB_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_USB_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_USB_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_USB_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_GEA_AHB_CLK_EN_OFFSET                               0x0000010C
#define SYSCFG_PERIPH_GEA_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_GEA_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_GEA_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_GEA_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_OFFSET                            0x00000110
#define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_TYPE                              UInt32
#define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_RESERVED_MASK                     0xFFFFFFFE
#define    SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_EN_SHIFT                       0
#define    SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_EN_MASK                        0x00000001

#define SYSCFG_PERIPH_PKA_AHB_CLK_EN_OFFSET                               0x00000114
#define SYSCFG_PERIPH_PKA_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_PKA_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_PKA_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_PKA_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_UARTA_AHB_CLK_EN_OFFSET                             0x00000118
#define SYSCFG_PERIPH_UARTA_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_UARTA_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_UARTA_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_UARTA_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_UARTB_AHB_CLK_EN_OFFSET                             0x0000011C
#define SYSCFG_PERIPH_UARTB_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_UARTB_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_UARTB_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_UARTB_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_DA_AHB_CLK_EN_OFFSET                                0x00000120
#define SYSCFG_PERIPH_DA_AHB_CLK_EN_TYPE                                  UInt32
#define SYSCFG_PERIPH_DA_AHB_CLK_EN_RESERVED_MASK                         0xFFFFFFFE
#define    SYSCFG_PERIPH_DA_AHB_CLK_EN_EN_SHIFT                           0
#define    SYSCFG_PERIPH_DA_AHB_CLK_EN_EN_MASK                            0x00000001

#define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_OFFSET                             0x00000124
#define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_LCD_AHB_CLK_EN_OFFSET                               0x0000012C
#define SYSCFG_PERIPH_LCD_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_LCD_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_LCD_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_LCD_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_DPE_AHB_CLK_EN_OFFSET                               0x00000130
#define SYSCFG_PERIPH_DPE_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_DPE_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_DPE_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_DPE_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_DMAC_AHB_CLK_EN_OFFSET                              0x00000134
#define SYSCFG_PERIPH_DMAC_AHB_CLK_EN_TYPE                                UInt32
#define SYSCFG_PERIPH_DMAC_AHB_CLK_EN_RESERVED_MASK                       0xFFFFFFFE
#define    SYSCFG_PERIPH_DMAC_AHB_CLK_EN_EN_SHIFT                         0
#define    SYSCFG_PERIPH_DMAC_AHB_CLK_EN_EN_MASK                          0x00000001

#define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_OFFSET                             0x00000138
#define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_OFFSET                             0x0000013C
#define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_DES_AHB_CLK_EN_OFFSET                               0x00000144
#define SYSCFG_PERIPH_DES_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_DES_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_DES_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_DES_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_UARTC_AHB_CLK_EN_OFFSET                             0x0000014C
#define SYSCFG_PERIPH_UARTC_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_UARTC_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_UARTC_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_UARTC_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_RNG_AHB_CLK_EN_OFFSET                               0x00000150
#define SYSCFG_PERIPH_RNG_AHB_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_RNG_AHB_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_RNG_AHB_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_RNG_AHB_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_OFFSET                             0x00000154
#define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_OFFSET                         0x0000015C
#define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_TYPE                           UInt32
#define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_RESERVED_MASK                  0xFFFFFFFE
#define    SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_EN_SHIFT                    0
#define    SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_EN_MASK                     0x00000001

#define SYSCFG_PERIPH_MPHI_AHB_CLK_EN_OFFSET                              0x00000160
#define SYSCFG_PERIPH_MPHI_AHB_CLK_EN_TYPE                                UInt32
#define SYSCFG_PERIPH_MPHI_AHB_CLK_EN_RESERVED_MASK                       0xFFFFFFFE
#define    SYSCFG_PERIPH_MPHI_AHB_CLK_EN_EN_SHIFT                         0
#define    SYSCFG_PERIPH_MPHI_AHB_CLK_EN_EN_MASK                          0x00000001

#define SYSCFG_SYSCFG_DMAC_AHB_CLK_MODE_OFFSET                            0x00000164
#define SYSCFG_SYSCFG_DMAC_AHB_CLK_MODE_TYPE                              UInt32
#define SYSCFG_SYSCFG_DMAC_AHB_CLK_MODE_RESERVED_MASK                     0xFFFFFFFE
#define    SYSCFG_SYSCFG_DMAC_AHB_CLK_MODE_EN_SHIFT                       0
#define    SYSCFG_SYSCFG_DMAC_AHB_CLK_MODE_EN_MASK                        0x00000001

#define SYSCFG_PERIPH_HUCM_FW_CLK_EN_OFFSET                               0x00000168
#define SYSCFG_PERIPH_HUCM_FW_CLK_EN_TYPE                                 UInt32
#define SYSCFG_PERIPH_HUCM_FW_CLK_EN_RESERVED_MASK                        0xFFFFFFFE
#define    SYSCFG_PERIPH_HUCM_FW_CLK_EN_EN_SHIFT                          0
#define    SYSCFG_PERIPH_HUCM_FW_CLK_EN_EN_MASK                           0x00000001

#define SYSCFG_PERIPH_HTM_CLK_EN_OFFSET                                   0x0000016C
#define SYSCFG_PERIPH_HTM_CLK_EN_TYPE                                     UInt32
#define SYSCFG_PERIPH_HTM_CLK_EN_RESERVED_MASK                            0xFFFFFFF8
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTMH_HCLK_EN_SHIFT                    2
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTMH_HCLK_EN_MASK                     0x00000004
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTML_HCLK_EN_SHIFT                    1
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTML_HCLK_EN_MASK                     0x00000002
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTM_ATCLK_EN_SHIFT                    0
#define    SYSCFG_PERIPH_HTM_CLK_EN_HTM_ATCLK_EN_MASK                     0x00000001

#define SYSCFG_TESTABILITY_ACCESS_OFFSET                                  0x00000170
#define SYSCFG_TESTABILITY_ACCESS_TYPE                                    UInt32
#define SYSCFG_TESTABILITY_ACCESS_RESERVED_MASK                           0xFFFFFFF0
#define    SYSCFG_TESTABILITY_ACCESS_ETM_LOCK_SHIFT                       3
#define    SYSCFG_TESTABILITY_ACCESS_ETM_LOCK_MASK                        0x00000008
#define    SYSCFG_TESTABILITY_ACCESS_SBD_DISABLE_SHIFT                    2
#define    SYSCFG_TESTABILITY_ACCESS_SBD_DISABLE_MASK                     0x00000004
#define    SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE_LOCK_SHIFT              1
#define    SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE_LOCK_MASK               0x00000002
#define    SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE_SHIFT                   0
#define    SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE_MASK                    0x00000001

#define SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_OFFSET                      0x00000174
#define SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_TYPE                        UInt32
#define SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_RESERVED_MASK               0x00000000
#define    SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_DIS_OTP_RGN_RD_N_SHIFT   0
#define    SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_DIS_OTP_RGN_RD_N_MASK    0xFFFFFFFF

#define SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_OFFSET                     0x00000178
#define SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_TYPE                       UInt32
#define SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_RESERVED_MASK              0x00000000
#define    SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_DIS_OTP_RGN_WR_N_SHIFT  0
#define    SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_DIS_OTP_RGN_WR_N_MASK   0xFFFFFFFF

#define SYSCFG_OTP_DEVICE_STATUS_OFFSET                                   0x0000017C
#define SYSCFG_OTP_DEVICE_STATUS_TYPE                                     UInt32
#define SYSCFG_OTP_DEVICE_STATUS_RESERVED_MASK                            0xFFFF0000
#define    SYSCFG_OTP_DEVICE_STATUS_DEVICE_STATUS_SHIFT                   0
#define    SYSCFG_OTP_DEVICE_STATUS_DEVICE_STATUS_MASK                    0x0000FFFF

#define SYSCFG_IRDROP_MON3_OFFSET                                         0x00000180
#define SYSCFG_IRDROP_MON3_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON3_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON3_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON3_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON3_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON3_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON3_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON3_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_IRDROP_MON4_OFFSET                                         0x00000184
#define SYSCFG_IRDROP_MON4_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON4_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON4_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON4_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON4_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON4_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON4_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON4_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_IRDROP_MON5_OFFSET                                         0x00000188
#define SYSCFG_IRDROP_MON5_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON5_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON5_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON5_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON5_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON5_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON5_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON5_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_IRDROP_MON6_OFFSET                                         0x0000018C
#define SYSCFG_IRDROP_MON6_TYPE                                           UInt32
#define SYSCFG_IRDROP_MON6_RESERVED_MASK                                  0xFFFFF000
#define    SYSCFG_IRDROP_MON6_OSC_EN_SHIFT                                11
#define    SYSCFG_IRDROP_MON6_OSC_EN_MASK                                 0x00000800
#define    SYSCFG_IRDROP_MON6_MON_EN_SHIFT                                10
#define    SYSCFG_IRDROP_MON6_MON_EN_MASK                                 0x00000400
#define    SYSCFG_IRDROP_MON6_CNT_OUT_SHIFT                               0
#define    SYSCFG_IRDROP_MON6_CNT_OUT_MASK                                0x000003FF

#define SYSCFG_PERIPH_CIPHER_FW_CLK_EN_OFFSET                             0x00000190
#define SYSCFG_PERIPH_CIPHER_FW_CLK_EN_TYPE                               UInt32
#define SYSCFG_PERIPH_CIPHER_FW_CLK_EN_RESERVED_MASK                      0xFFFFFFFE
#define    SYSCFG_PERIPH_CIPHER_FW_CLK_EN_EN_SHIFT                        0
#define    SYSCFG_PERIPH_CIPHER_FW_CLK_EN_EN_MASK                         0x00000001

#define SYSCFG_SYSCONF_AHB_CLK_EXTEND0_OFFSET                             0x000001A0
#define SYSCFG_SYSCONF_AHB_CLK_EXTEND0_TYPE                               UInt32
#define SYSCFG_SYSCONF_AHB_CLK_EXTEND0_RESERVED_MASK                      0x00000000
#define    SYSCFG_SYSCONF_AHB_CLK_EXTEND0_SYSCONF_AHB_CLK_EXTEND0_SHIFT   0
#define    SYSCFG_SYSCONF_AHB_CLK_EXTEND0_SYSCONF_AHB_CLK_EXTEND0_MASK    0xFFFFFFFF

#define SYSCFG_SYSCONF_AHB_CLK_EXTEND1_OFFSET                             0x000001A4
#define SYSCFG_SYSCONF_AHB_CLK_EXTEND1_TYPE                               UInt32
#define SYSCFG_SYSCONF_AHB_CLK_EXTEND1_RESERVED_MASK                      0x00000000
#define    SYSCFG_SYSCONF_AHB_CLK_EXTEND1_SYSCONF_AHB_CLK_EXTEND1_SHIFT   0
#define    SYSCFG_SYSCONF_AHB_CLK_EXTEND1_SYSCONF_AHB_CLK_EXTEND1_MASK    0xFFFFFFFF

#define SYSCFG_OTP_CHIP_FEATURE_ID_OFFSET                                 0x000001C0
#define SYSCFG_OTP_CHIP_FEATURE_ID_TYPE                                   UInt32
#define SYSCFG_OTP_CHIP_FEATURE_ID_RESERVED_MASK                          0xFFFFFFF8
#define    SYSCFG_OTP_CHIP_FEATURE_ID_CHIP_DIFF_OVERRIDE_SHIFT            2
#define    SYSCFG_OTP_CHIP_FEATURE_ID_CHIP_DIFF_OVERRIDE_MASK             0x00000004
#define    SYSCFG_OTP_CHIP_FEATURE_ID_CHIP_DIFF_ID_SHIFT                  0
#define    SYSCFG_OTP_CHIP_FEATURE_ID_CHIP_DIFF_ID_MASK                   0x00000003

#define SYSCFG_OTP_WCDMA_CAT_OFFSET                                       0x000001C4
#define SYSCFG_OTP_WCDMA_CAT_TYPE                                         UInt32
#define SYSCFG_OTP_WCDMA_CAT_RESERVED_MASK                                0xFFFFFFE0
#define    SYSCFG_OTP_WCDMA_CAT_PRISM_ENB_SHIFT                           4
#define    SYSCFG_OTP_WCDMA_CAT_PRISM_ENB_MASK                            0x00000010
#define    SYSCFG_OTP_WCDMA_CAT_HSDPA_ENB_SHIFT                           3
#define    SYSCFG_OTP_WCDMA_CAT_HSDPA_ENB_MASK                            0x00000008
#define    SYSCFG_OTP_WCDMA_CAT_HSDPA_CAT_SHIFT                           2
#define    SYSCFG_OTP_WCDMA_CAT_HSDPA_CAT_MASK                            0x00000004
#define    SYSCFG_OTP_WCDMA_CAT_HSUPA_ENB_SHIFT                           1
#define    SYSCFG_OTP_WCDMA_CAT_HSUPA_ENB_MASK                            0x00000002
#define    SYSCFG_OTP_WCDMA_CAT_HSUPA_CAT_SHIFT                           0
#define    SYSCFG_OTP_WCDMA_CAT_HSUPA_CAT_MASK                            0x00000001

#define SYSCFG_OTP_MM_FEAT_CFG_OFFSET                                     0x000001C8
#define SYSCFG_OTP_MM_FEAT_CFG_TYPE                                       UInt32
#define SYSCFG_OTP_MM_FEAT_CFG_RESERVED_MASK                              0xFFFFFFF0
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_ISP_RSLN_SHIFT                   3
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_ISP_RSLN_MASK                    0x00000008
#define    SYSCFG_OTP_MM_FEAT_CFG_AP_LMT_SPD_SHIFT                        2
#define    SYSCFG_OTP_MM_FEAT_CFG_AP_LMT_SPD_MASK                         0x00000004
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_DSP_SPD_SHIFT                    1
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_DSP_SPD_MASK                     0x00000002
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_DSI_SPD_SHIFT                    0
#define    SYSCFG_OTP_MM_FEAT_CFG_SW_CAP_DSI_SPD_MASK                     0x00000001

#define SYSCFG_OTP_MM_FEAT_DIS_OFFSET                                     0x000001CC
#define SYSCFG_OTP_MM_FEAT_DIS_TYPE                                       UInt32
#define SYSCFG_OTP_MM_FEAT_DIS_RESERVED_MASK                              0xFFFFFF80
#define    SYSCFG_OTP_MM_FEAT_DIS_AP_DIS_SHIFT                            6
#define    SYSCFG_OTP_MM_FEAT_DIS_AP_DIS_MASK                             0x00000040
#define    SYSCFG_OTP_MM_FEAT_DIS_VID_DIS_SHIFT                           5
#define    SYSCFG_OTP_MM_FEAT_DIS_VID_DIS_MASK                            0x00000020
#define    SYSCFG_OTP_MM_FEAT_DIS_DPE_DIS_SHIFT                           4
#define    SYSCFG_OTP_MM_FEAT_DIS_DPE_DIS_MASK                            0x00000010
#define    SYSCFG_OTP_MM_FEAT_DIS_LCDC_DIS_SHIFT                          3
#define    SYSCFG_OTP_MM_FEAT_DIS_LCDC_DIS_MASK                           0x00000008
#define    SYSCFG_OTP_MM_FEAT_DIS_CAM_DIS_SHIFT                           2
#define    SYSCFG_OTP_MM_FEAT_DIS_CAM_DIS_MASK                            0x00000004
#define    SYSCFG_OTP_MM_FEAT_DIS_DSI_DIS_SHIFT                           1
#define    SYSCFG_OTP_MM_FEAT_DIS_DSI_DIS_MASK                            0x00000002
#define    SYSCFG_OTP_MM_FEAT_DIS_TVO_DIS_SHIFT                           0
#define    SYSCFG_OTP_MM_FEAT_DIS_TVO_DIS_MASK                            0x00000001

#define SYSCFG_OTP_MAC_VIS_DIS_OFFSET                                     0x000001D0
#define SYSCFG_OTP_MAC_VIS_DIS_TYPE                                       UInt32
#define SYSCFG_OTP_MAC_VIS_DIS_RESERVED_MASK                              0xFFFFFFFE
#define    SYSCFG_OTP_MAC_VIS_DIS_MACVIS_DIS_SHIFT                        0
#define    SYSCFG_OTP_MAC_VIS_DIS_MACVIS_DIS_MASK                         0x00000001

#define SYSCFG_BRIDGE_INCR_EN_OFFSET                                      0x000001E0
#define SYSCFG_BRIDGE_INCR_EN_TYPE                                        UInt32
#define SYSCFG_BRIDGE_INCR_EN_RESERVED_MASK                               0xFFFF0000
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ZSM_RD_INCR_EN_SHIFT       15
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ZSM_RD_INCR_EN_MASK        0x00008000
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ZSM_WR_INCR_EN_SHIFT       14
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ZSM_WR_INCR_EN_MASK        0x00004000
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ICACHE_RD_INCR_EN_SHIFT    13
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ICACHE_RD_INCR_EN_MASK     0x00002000
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ICACHE_WR_INCR_EN_SHIFT    12
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_ICACHE_WR_INCR_EN_MASK     0x00001000
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_SLAVE_RD_INCR_EN_SHIFT     11
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_SLAVE_RD_INCR_EN_MASK      0x00000800
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_SLAVE_WR_INCR_EN_SHIFT     10
#define    SYSCFG_BRIDGE_INCR_EN_DSP_ASYNC_BRG_SLAVE_WR_INCR_EN_MASK      0x00000400
#define    SYSCFG_BRIDGE_INCR_EN_ML2MH_SYNC_BRG_RD_INCR_EN_SHIFT          9
#define    SYSCFG_BRIDGE_INCR_EN_ML2MH_SYNC_BRG_RD_INCR_EN_MASK           0x00000200
#define    SYSCFG_BRIDGE_INCR_EN_ML2MH_SYNC_BRG_WR_INCR_EN_SHIFT          8
#define    SYSCFG_BRIDGE_INCR_EN_ML2MH_SYNC_BRG_WR_INCR_EN_MASK           0x00000100
#define    SYSCFG_BRIDGE_INCR_EN_MH2ML_SYNC_BRG_RD_INCR_EN_SHIFT          7
#define    SYSCFG_BRIDGE_INCR_EN_MH2ML_SYNC_BRG_RD_INCR_EN_MASK           0x00000080
#define    SYSCFG_BRIDGE_INCR_EN_MH2ML_SYNC_BRG_WR_INCR_EN_SHIFT          6
#define    SYSCFG_BRIDGE_INCR_EN_MH2ML_SYNC_BRG_WR_INCR_EN_MASK           0x00000040
#define    SYSCFG_BRIDGE_INCR_EN_MA11D_SYNC_BRG_RD_INCR_EN_SHIFT          5
#define    SYSCFG_BRIDGE_INCR_EN_MA11D_SYNC_BRG_RD_INCR_EN_MASK           0x00000020
#define    SYSCFG_BRIDGE_INCR_EN_MA11D_SYNC_BRG_WR_INCR_EN_SHIFT          4
#define    SYSCFG_BRIDGE_INCR_EN_MA11D_SYNC_BRG_WR_INCR_EN_MASK           0x00000010
#define    SYSCFG_BRIDGE_INCR_EN_VID_ENC_SYNC_BRG_RD_INCR_EN_SHIFT        3
#define    SYSCFG_BRIDGE_INCR_EN_VID_ENC_SYNC_BRG_RD_INCR_EN_MASK         0x00000008
#define    SYSCFG_BRIDGE_INCR_EN_VID_ENC_SYNC_BRG_WR_INCR_EN_SHIFT        2
#define    SYSCFG_BRIDGE_INCR_EN_VID_ENC_SYNC_BRG_WR_INCR_EN_MASK         0x00000004
#define    SYSCFG_BRIDGE_INCR_EN_VID_DEC_SYNC_BRG_RD_INCR_EN_SHIFT        1
#define    SYSCFG_BRIDGE_INCR_EN_VID_DEC_SYNC_BRG_RD_INCR_EN_MASK         0x00000002
#define    SYSCFG_BRIDGE_INCR_EN_VID_DEC_SYNC_BRG_WR_INCR_EN_SHIFT        0
#define    SYSCFG_BRIDGE_INCR_EN_VID_DEC_SYNC_BRG_WR_INCR_EN_MASK         0x00000001

#define SYSCFG_FPGA_VERSION_OFFSET                                        0x000001FC
#define SYSCFG_FPGA_VERSION_TYPE                                          UInt32
#define SYSCFG_FPGA_VERSION_RESERVED_MASK                                 0xFFFFFF00
#define    SYSCFG_FPGA_VERSION_FPGA_RLS_ID_SHIFT                          0
#define    SYSCFG_FPGA_VERSION_FPGA_RLS_ID_MASK                           0x000000FF


//*****old style:

typedef volatile struct {
    UInt32  m_SYSCFG_IOCR0;                                       // 000
    UInt32  m_SYSCFG_IOCR1;                                       // 004
    UInt32  m_SYSCFG_SUCR;                                        // 008
    UInt32  m_SYSCFG_IOCR2;                                       // 00C
    UInt32  m_SYSCFG_PIDR;                                        // 010
    UInt32  m_SYSCFG_DSPCTRL;                                     // 014
    UInt32  m_SYSCFG_PUMR;                                        // 018
    UInt32  m_SYSCFG_IOCR3;                                       // 01C
    UInt32  m_SYSCFG_IOCR4;                                       // 020
    UInt32  m_SYSCFG_IOCR5;                                       // 024
    UInt32  m_SYSCFG_IOCR6;                                       // 028
    UInt32  m_SYSCFG_IOCR7;                                       // 02C
    UInt32  m_RESERVED_030[1];                                  // 030
    UInt32  m_SYSCFG_TVENCCR;                                     // 034
    UInt32  m_SYSCFG_DSICR;                                       // 038
    UInt32  m_RESERVED_03C[1];                                  // 03C
    UInt32  m_SYSCFG_MCR;                                         // 040
    UInt32  m_SYSCFG_MRR;                                         // 044
    UInt32  m_SYSCFG_RAMCTRL;                                     // 048
    UInt32  m_RESERVED_04C[1];                                  // 04C
    UInt32  m_SYSCFG_SECCTRL;                                     // 050
    UInt32  m_SYSCFG_SECSTAT;                                     // 054
    UInt32  m_SYSCFG_MARM11CR;                                    // 058
    UInt32  m_RESERVED_05C[1];                                  // 05C
    UInt32  m_SYSCFG_MAMBACR;                                     // 060
    UInt32  m_SYSCFG_RFCR;                                        // 064
    UInt32  m_RESERVED_068[6];                                  // 068-080
    UInt32  m_SYSCFG_ANACR0;                                      // 080
    UInt32  m_SYSCFG_ANACR1;                                      // 084
    UInt32  m_SYSCFG_ANACR2;                                      // 088
    UInt32  m_SYSCFG_ANACR3;                                      // 08C
    UInt32  m_SYSCFG_ANACR4;                                      // 090
    UInt32  m_SYSCFG_ANACR5;                                      // 094
    UInt32  m_SYSCFG_ANACR6;                                      // 098
    UInt32  m_SYSCFG_ANACR7;                                      // 09C
    UInt32  m_SYSCFG_ANACR8;                                      // 0A0
    UInt32  m_SYSCFG_ANACR9;                                      // 0A4
    UInt32  m_SYSCFG_ANACR10;                                     // 0A8
    UInt32  m_SYSCFG_ANACR11;                                     // 0AC
    UInt32  m_SYSCFG_ANACR12;                                     // 0B0
    UInt32  m_SYSCFG_ANACR13;                                     // 0B4
    UInt32  m_SYSCFG_ANACR14;                                     // 0B8
    UInt32  m_SYSCFG_ANACR15;                                     // 0BC
    UInt32  m_SYSCFG_IRDROP_MON0;                                 // 0C0
    UInt32  m_SYSCFG_IRDROP_MON1;                                 // 0C4
    UInt32  m_SYSCFG_IRDROP_MON2;                                 // 0C8
    UInt32  m_SYSCFG_MDIO_WRITE;                                  // 0CC
    UInt32  m_SYSCFG_MDIO_READ;                                   // 0D0
    UInt32  m_RESERVED_0D4[1];                                  // 0D4
    UInt32  m_SYSCFG_PERIPH_AHB_CLK_GATE_MASK;                    // 0D8
    UInt32  m_SYSCFG_PERIPH_AHB_CLK_GATE_FORCE;                   // 0DC
    UInt32  m_SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW;             // 0E0
    UInt32  m_SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR;                 // 0E4
    UInt32  m_RESERVED_0E8[6];                                  // 0E8-100
    UInt32  m_SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN;               // 100
    UInt32  m_SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN;          // 104
    UInt32  m_SYSCFG_PERIPH_USB_AHB_CLK_EN;                       // 108
    UInt32  m_SYSCFG_PERIPH_GEA_AHB_CLK_EN;                       // 10C
    UInt32  m_SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN;                    // 110
    UInt32  m_SYSCFG_PERIPH_PKA_AHB_CLK_EN;                       // 114
    UInt32  m_SYSCFG_PERIPH_UARTA_AHB_CLK_EN;                     // 118
    UInt32  m_SYSCFG_PERIPH_UARTB_AHB_CLK_EN;                     // 11C
    UInt32  m_SYSCFG_PERIPH_DA_AHB_CLK_EN;                        // 120
    UInt32  m_SYSCFG_PERIPH_MPCLK_AHB_CLK_EN;                     // 124
    UInt32  m_RESERVED_128[1];                                  // 128
    UInt32  m_SYSCFG_PERIPH_LCD_AHB_CLK_EN;                       // 12C
    UInt32  m_SYSCFG_PERIPH_DPE_AHB_CLK_EN;                       // 130
    UInt32  m_SYSCFG_PERIPH_DMAC_AHB_CLK_EN;                      // 134
    UInt32  m_SYSCFG_PERIPH_SDIO1_AHB_CLK_EN;                     // 138
    UInt32  m_SYSCFG_PERIPH_SDIO2_AHB_CLK_EN;                     // 13C
    UInt32  m_RESERVED_140[1];                                  // 140
    UInt32  m_SYSCFG_PERIPH_DES_AHB_CLK_EN;                       // 144
    UInt32  m_RESERVED_148[1];                                  // 148
    UInt32  m_SYSCFG_PERIPH_UARTC_AHB_CLK_EN;                     // 14C
    UInt32  m_SYSCFG_PERIPH_RNG_AHB_CLK_EN;                       // 150
    UInt32  m_SYSCFG_PERIPH_SDIO3_AHB_CLK_EN;                     // 154
    UInt32  m_SYSCFG_PERIPH_TVENC_AHB_CLK_EN;                     // 158
    UInt32  m_SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN;                 // 15C
    UInt32  m_SYSCFG_PERIPH_MPHI_AHB_CLK_EN;                      // 160
    UInt32  m_RESERVED_164[3];                                  // 164-170
    UInt32  m_SYSCFG_TESTABILITY_ACCESS;                          // 170
    UInt32  m_SYSCFG_DISABLE_OTP_REGION_READ_ACCESS;              // 174
    UInt32  m_SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS;             // 178
    UInt32  m_SYSCFG_OTP_DEVICE_STATUS;                           // 17C
    UInt32  m_RESERVED_180[31];                                 // 180-1FC
    UInt32  m_SYSCFG_FPGA_Version;                                // 1FC
} BCMRDB_SYSCFG_REGS;

/****  SYSCFG  ****/
//#define SYSCFG_IOCR0                     (SYSCFG_BASE_ADDR + 0x0000)  /* IOCR0 bit I/O Configuration Register 0       */
  #define SYSCFG_IOCR0_FLASH_GPIO_MUX                0x80000000                    /* RW 0 FALE/FCLE, 1 GPIO[62:61],  */
  #define SYSCFG_IOCR0_FLASH_SD2_MUX                 0x40000000                    /* RW 0 FCLK,FADQ[8:0], 1 SD2,  */
  #define SYSCFG_IOCR0_LCD_CTRL_MUX                  0x20000000                    /* RW LCD/GPIO/MPHI, concatenated  */
  #define SYSCFG_IOCR0_LCDD1_LCDD15_MUX              0x10000000                    /* RW LCD/CAM/MPHI, concatenated w */
  #define SYSCFG_IOCR0_GPEN9_B1                      0x08000000                    /* RW {GPEN9_B1, GPEN9_B0}: GPEN9/ */
  #define SYSCFG_IOCR0_DSP_test_port_enable          0x04000000                    /* RW DSP/WCDMA muxout on LCD and  */
  #define SYSCFG_IOCR0_MPHI_mux                      0x02000000                    /* RW LCD/GPIO/MPHI, concatenated  */
//  #define SYSCFG_IOCR0_DIGMIC_MUX                    0x01000000                    /* RW DIGMIC/GPIO[63] Select, 0 DI */
//  #define SYSCFG_IOCR0_PCM_MUX                       0x00C00000                    /* RW PCM/SPI2/GPIO[40:38] or Trac */
  #define SYSCFG_IOCR0_SPI_MUX_HI                    0x00200000                    /* RW Pin mux for bit 11,  */
  #define SYSCFG_IOCR0_GPEN11_B1                     0x00100000                    /* RW {GPEN11_B1, GPEN11_B0}: GPEN */
  #define SYSCFG_IOCR0_LCDD16_LCDD17_MUX             0x00080000                    /* RW LCD/GPIO/MPHI, concatenated  */
  #define SYSCFG_IOCR0_AFCPDM_MUX                    0x00040000                    /* RW AFCPDM / CLK_MONITOR Pin Mux */
  #define SYSCFG_IOCR0_GPEN11_B0                     0x00020000                    /* RW See description of GPEN11_B1 */
  #define SYSCFG_IOCR0_GPEN10                        0x00018000                    /* RW GPEN/GPIO55 Select, 00 GPEN[ */
  #define SYSCFG_IOCR0_GPEN9_B0                      0x4000                        /* RW See description of GPEN9_B1  */
  #define SYSCFG_IOCR0_GPEN8_MUX                     0x2000                        /* RW GPEN/GPIO53 Select, Selectio */
  #define SYSCFG_IOCR0_GPEN7                         0x1000                        /* RW GPEN/GPIO7 Select, 0 GPEN[7] */
  #define SYSCFG_IOCR0_SPI_MUX                       0x0800                        /* RW SPI/UARTC/GPIO, Selection co */
  #define SYSCFG_IOCR0_GPIO16_MUX                    0x0400                        /* RW Bit 10 used to control PWM0  */
  #define SYSCFG_IOCR0_GPIO17_MUX                    0x0200                        /* RW Bit 9 used to control PWM1 o */
  #define SYSCFG_IOCR0_GPIO_muxes                    0x0180                        /* RW GPIO[30:28] muxes, 00 GPIO[3 */
  #define SYSCFG_IOCR0_I2S_MUX                       0x0060                        /* RW GPIO / I2S, 00 I2S, 01 I2S,  */
  #define SYSCFG_IOCR0_SD1_MUX                       0x0018                        /* RW 00 SD1, 01 SPI2, 10 2G-TWIF/ */
  #define SYSCFG_IOCR0_M68                           0x0004                        /* RW Select M68 interface on LCD, */
  #define SYSCFG_IOCR0_SD3_MUX                       0x0003                        /* RW GPIO/MSPRO/SD3, 00 SD3, 01 M */
#define SYSCFG_IOCR1                     (SYSCFG_BASE_ADDR + 0x0004)  /* IOCR1 bit I/O Configuration Register 1       */
  #define SYSCFG_IOCR1_STRAP_PULLDOWN_CONTROL        0xFFFF0000                    /* RW pull-down control for FADQ15 */
  #define SYSCFG_IOCR1_KEY_COL                       0xFF00                        /* RW KEY_COL/GPIO, 0 GPIO[15:8] p */
  #define SYSCFG_IOCR1_KEY_ROW                       0x00FF                        /* RW KEY_ROW/GPIO, 0 GPIO[7:0] pi */
#define SYSCFG_SUCR                      (SYSCFG_BASE_ADDR + 0x0008)  /* SUCR bit Start Up Mode Register */
  #define SYSCFG_SUCR_SPARE                          0x80000000                   /* RO SPARE strap option,  */
  #define SYSCFG_SUCR_FLASH_BOOT                     0x40000000                   /* RO Flash Boot on (FADQ14), 0 Boo */
  #define SYSCFG_SUCR_DOWNLOAD                       0x20000000                   /* RO Download (FADQ13), 0 Normal m */
  #define SYSCFG_SUCR_AP                             0x10000000                   /* RO Audio Precision Testing (FADQ */
  #define SYSCFG_SUCR_reserved0                      0x0C000000                   /* reserved0  */
  #define SYSCFG_SUCR_NAND8                          0x02000000                   /* RO NANDFlash Bus Width (FADQ9),  */
  #define SYSCFG_SUCR_VCOBYPASS                      0x01000000                   /* RO VCO_BYPASS strap value (FADQ8 */
  #define SYSCFG_SUCR_reserved1                      0x00C00000                   /* reserved1  */
  #define SYSCFG_SUCR_EJTAG_SEL                      0x00200000                   /* RO Select LV TAP, strap value of */
  #define SYSCFG_SUCR_JTAG_SEL                       0x001C0000                   /* RO JTAG Multi-Core daisy chain s */
  #define SYSCFG_SUCR_reserved2                      0x0003FFE0                   /* reserved2  */
  #define SYSCFG_SUCR_Srst_Stat                      0x0010                       /* RO Soft Reset Status:, 0 Hard re */
  #define SYSCFG_SUCR_reserved3                      0x0008                       /* reserved3  */
  #define SYSCFG_SUCR_BootSrc                        0x0004                       /* RO Boot Source Select: (READ-ONL */
  #define SYSCFG_SUCR_reserved4                      0x0002                       /* reserved4  */
  #define SYSCFG_SUCR_DLM                            0x0001                       /* RO same as DOWNLOAD bit above.,  */
#define SYSCFG_IOCR2                     (SYSCFG_BASE_ADDR + 0x000C)  /* IOCR2 bit I/O Configuration Register 2       */
  #define SYSCFG_IOCR2_SD2DAT_PULL                   0xC0000000                    /* RW SD2DAT pull, 00 SD2DAT pad n */
  #define SYSCFG_IOCR2_SD2CMD_PULL                   0x30000000                    /* RW SD2CMD pull, 00 SD2CMD pad n */
  #define SYSCFG_IOCR2_SD2CK_PULL                    0x0C000000                    /* RW SD2CK pull, 00 SD2CK pad no  */
  #define SYSCFG_IOCR2_reserved0                     0x03C00000                    /* reserved0  */
  #define SYSCFG_IOCR2_GPEN8_MUX_HI                  0x00200000                    /* RW see description for IOCR0's  */
  #define SYSCFG_IOCR2_reserved1                     0x001E0000                    /* reserved1  */
  #define SYSCFG_IOCR2_SOFTRSTO_MUX                  0x00010000                    /* RW Soft Reset output mux select */
  #define SYSCFG_IOCR2_OTGCTRL1_MUX                  0xC000                        /* RW Mux CE5N/GPEN[15] onto OTGCT */
  #define SYSCFG_IOCR2_HSOTG2                        0x2000                        /* RW AHB Slave port Endian select */
  #define SYSCFG_IOCR2_HSOTG1                        0x1000                        /* RW AHB Master port Endian selec */
  #define SYSCFG_IOCR2_GPIO                          0x0800                        /* RW 31-16 slew control Slew cont */
  #define SYSCFG_IOCR2_GPIO15_0                      0x0400                        /* RW slew control Slew control on */
  #define SYSCFG_IOCR2_reserved2                     0x0380                        /* reserved2  */
  #define SYSCFG_IOCR2_SIMDAT_HYS                    0x0040                        /* RW Hysteresis control on SIMDAT */
  #define SYSCFG_IOCR2_OSC2_SELECT                   0x0030                        /* RW Select sel1, sel0 of OSC2,  */
  #define SYSCFG_IOCR2_OSC2_ENABLE                   0x0008                        /* RW Enable OSC2 - observe output */
  #define SYSCFG_IOCR2_OSC1_SELECT                   0x0006                        /* RW Select sel1, sel0 of OSC1,  */
  #define SYSCFG_IOCR2_OSC1_ENABLE                   0x0001                        /* RW Enable OSC1 - observe output */
#define SYSCFG_PIDR                      (SYSCFG_BASE_ADDR + 0x0010)  /* PIDR bit Product ID Register */
  #define SYSCFG_PIDR_reserved0                      0xFFFFF000                   /* reserved0  */
  #define SYSCFG_PIDR_PID                            0x0F00                       /* RO Product Family ID, 0x2: 2G ba */
  #define SYSCFG_PIDR_RID                            0x00F0                       /* RO Product ID,  */
  #define SYSCFG_PIDR_TID                            0x000F                       /* RO Revision ID,  */
#define SYSCFG_DSPCTRL                   (SYSCFG_BASE_ADDR + 0x0014)  /* DSPCTRL bit DSP Control Register             */
  #define SYSCFG_DSPCTRL_reserved0                   0xFFFFFF00                      /* reserved0  */
  #define SYSCFG_DSPCTRL_AUDIOSRST                   0x0080                          /* RW Put Audio in the following */
  #define SYSCFG_DSPCTRL_SYNCEXTPRAM                 0x0040                          /* RW External DSP PRAM Type, Th */
  #define SYSCFG_DSPCTRL_JTAGINTWAKE                 0x0020                          /* RW Control whether DSP JTAG i */
  #define SYSCFG_DSPCTRL_en_TRST                     0x0010                          /* RW Control whether DSP JTAG T */
  #define SYSCFG_DSPCTRL_reserved1                   0x000F                          /* reserved1  */
#define SYSCFG_PUMR                      (SYSCFG_BASE_ADDR + 0x0018)  /* PUMR bit Power Up Mode Register */
  #define SYSCFG_PUMR_PUMODE                         0xFFFFFFFF                   /* RW Power up mode,  */
#define SYSCFG_IOCR3                     (SYSCFG_BASE_ADDR + 0x001C)  /* IOCR3 bit I/O Configuration Register 3       */
  #define SYSCFG_IOCR3_reserved0                     0xF8000000                    /* reserved0  */
  #define SYSCFG_IOCR3_X_TRIG_EN                     0x04000000                    /* RW 0 Cross-triggering among mul */
  #define SYSCFG_IOCR3_reserved1                     0x03000000                    /* reserved1  */
  #define SYSCFG_IOCR3_SIM2_DIS                      0x00800000                    /* RW 0 Normal operation, 1 SIM2 p */
  #define SYSCFG_IOCR3_UARTC_DIS                     0x00400000                    /* RW 0 Normal operation, 1 UARTC  */
  #define SYSCFG_IOCR3_TWIF_ENB                      0x00200000                    /* RW 0 WCDMA 3-wire interface pin */
  #define SYSCFG_IOCR3_PC_DIS                        0x00100000                    /* RW 0 Normal operation, 1 PC out */
  #define SYSCFG_IOCR3_SYN_DIS                       0x00080000                    /* RW 0 Normal operation, 1 SYNCLK */
  #define SYSCFG_IOCR3_RXDCK_DIS                     0x00040000                    /* RW 0 Normal operation, 1 RXDCK  */
  #define SYSCFG_IOCR3_GPEN_DIS                      0x00020000                    /* RW 0 Normal operation, 1 GPEN0- */
  #define SYSCFG_IOCR3_DA_DIS                        0x00010000                    /* RW 0 Normal operation, 1 DAILR  */
  #define SYSCFG_IOCR3_JTAG_DIS                      0x8000                        /* RW 0 Normal operation (default) */
  #define SYSCFG_IOCR3_SPIMS_DIS                     0x4000                        /* RW 0 Normal operation, 1 SPI/MS */
  #define SYSCFG_IOCR3_SIM_DIS                       0x2000                        /* RW 0 Normal operation, 1 SIM pa */
  #define SYSCFG_IOCR3_UARTB_DIS                     0x1000                        /* RW 0 Normal operation, 1 UARTB  */
  #define SYSCFG_IOCR3_UARTA_DIS                     0x0800                        /* RW 0 Normal operation, 1 UARTA  */
  #define SYSCFG_IOCR3_CAMD_PD                       0x0400                        /* RW 0 CAMD0-7 pulldown not selec */
  #define SYSCFG_IOCR3_CAMD_PU                       0x0200                        /* RW 0 CAMD0-7 pullup not selecte */
  #define SYSCFG_IOCR3_CAMHVS_PD                     0x0100                        /* RW 0 CAMHS/CAMVS pulldown not s */
  #define SYSCFG_IOCR3_CAMHVS_PU                     0x0080                        /* RW 0 CAMHS/CAMVS pullup not sel */
  #define SYSCFG_IOCR3_CAMDCK_PD                     0x0040                        /* RW 0 CAMDCK pulldown not select */
  #define SYSCFG_IOCR3_CAMDCK_PU                     0x0020                        /* RW 0 CAMDCK pullup not selected */
  #define SYSCFG_IOCR3_CAMCK_DIS                     0x0010                        /* RW 0 Normal operation, 1 CAMCK  */
  #define SYSCFG_IOCR3_LCD_DIS                       0x0008                        /* RW 0 Normal operation, 1 LCD pa */
  #define SYSCFG_IOCR3_NANDRDY_PD                    0x0004                        /* RW 0 NANDRDY pulldown not selec */
  #define SYSCFG_IOCR3_MBWAIT_PD                     0x0002                        /* RW 0 MBWAIT pulldown not select */
  #define SYSCFG_IOCR3_MEM_DIS                       0x0001                        /* RW 0 Normal operation, 1 Memory */
#define SYSCFG_IOCR4                     (SYSCFG_BASE_ADDR + 0x0020)  /* IOCR4 bit I/O Configuration Register 4       */
  #define SYSCFG_IOCR4_DAT_PULL                      0x80000000                    /* RW Pulldown FALE/FCLE pads on r */
  #define SYSCFG_IOCR4_LCD_slew                      0x40000000                    /* RW Slew control on LCD pads.,  */
  #define SYSCFG_IOCR4_SDIO_drive                    0x38000000                    /* RW Control drive strength of SD */
  #define SYSCFG_IOCR4_ETM_drive                     0x07000000                    /* RW Control drive strengths of T */
  #define SYSCFG_IOCR4_LCD_drive                     0x00E00000                    /* RW Control drive strength of LC */
  #define SYSCFG_IOCR4_SIM_drive                     0x001C0000                    /* RW Control drive strength of SI */
  #define SYSCFG_IOCR4_RF_Drive                      0x00038000                    /* RW Control drive strength of RF */
  #define SYSCFG_IOCR4_Camera_Drive                  0x7000                        /* RW Control drive strength of Ca */
  #define SYSCFG_IOCR4_SDMCLK                        0x0E00                        /* RW Control drive strength of SD */
  #define SYSCFG_IOCR4_MBCK_Drive                    0x01C0                        /* RW Control drive strength of MB */
  #define SYSCFG_IOCR4_DAT31_15_Drive                0x0038                        /* RW Control drive strength of DA */
  #define SYSCFG_IOCR4_MEM_Drive                     0x0007                        /* RW Control drive strength of al */
#define SYSCFG_IOCR5                     (SYSCFG_BASE_ADDR + 0x0024)  /* IOCR5 bit I/O Configuration Register 5       */
  #define SYSCFG_IOCR5_reserved0                     0xE0000000                    /* reserved0  */
  #define SYSCFG_IOCR5_TRACEBUS_select               0x18000000                    /* RW Selects source for TRACE BUS */
  #define SYSCFG_IOCR5_Pin_mux_Control_cam           0x06000000                    /* RW Camera Port Pin muxing Contr */
  #define SYSCFG_IOCR5_Pin_mux_control_sd2           0x01000000                    /* RW SD2DAT to SD2DAT7 Pin Muxing */
  #define SYSCFG_IOCR5_GPIO27_MUX                    0x00C00000                    /* RW GPIO27 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_GPIO26_MUX                    0x00300000                    /* RW GPIO26 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_Pin_mux_control_gpio32        0x000FF000                    /* RW GPIO32 to GPIO41 Pin Muxing  */
  #define SYSCFG_IOCR5_GPIO35_MUX                    0x0C00                        /* RW GPIO35 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_GPIO34_MUX                    0x0300                        /* RW GPIO34 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_GPIO33_MUX                    0x00C0                        /* RW GPIO33 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_GPIO32_MUX                    0x0030                        /* RW GPIO32 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_GPIO31_MUX                    0x000C                        /* RW GPIO31 Pin Muxing Control, 0 */
  #define SYSCFG_IOCR5_reserved1                     0x0003                        /* reserved1  */
#define SYSCFG_IOCR6                     (SYSCFG_BASE_ADDR + 0x0028)  /* IOCR6 bit I/O Configuration Register 6       */
  #define SYSCFG_IOCR6_reserved0                     0x80000000                    /* reserved0  */
  #define SYSCFG_IOCR6_GPIO21_18_MUX                 0x40000000                    /* RW muxes out DCSSEL, PCSSEL ont */
  #define SYSCFG_IOCR6_Csi_ccp_b                     0x20000000                    /* RW Controls CSI or CCP mode of  */
  #define SYSCFG_IOCR6_Cam_mode                      0x18000000                    /* RW Controls cam_mode of isp_top */
  #define SYSCFG_IOCR6_DDAC_FC_pwrdn                 0x06000000                    /* RW Controls i_d0_pwrdn and i_d1 */
  #define SYSCFG_IOCR6_SPI_Drive_strength_control       0x01C00000
  /* RW Controls slew on SPI pads, [24], ![23], [22] -- Note but 23 is used inverted, 100 2 mA, 010 4 mA -- Default v */
  #define SYSCFG_IOCR6_SPI_slew_control              0x00200000                    /* RW Controls Slew on the SPI pad */
  #define SYSCFG_IOCR6_DSPDebug_bus                  0x00100000                    /* RW If this bit is true, the DSP */
  #define SYSCFG_IOCR6_GPIO25_24_MUX                 0x00080000                    /* RW refer IOCR2[3] and IOCR2[0]  */
  #define SYSCFG_IOCR6_DIGI_PD_XO_enable             0x00040000                    /* RW Enable DIGI_PD_XO control vi */
  #define SYSCFG_IOCR6_PD_XO_BN_enable               0x00020000                    /* RW Enables PD_XO_BN control via */
  #define SYSCFG_IOCR6_PD_XO_BP_enable               0x00010000                    /* RW Enables PD_XO_BP control via */
  #define SYSCFG_IOCR6_PD_XO                         0xE000                        /* RW override Override PD_XO cont */
  #define SYSCFG_IOCR6_PD                            0x1C00                        /* RW XO_sw_control Software contr */
  #define SYSCFG_IOCR6_PD_XO_polarity                0x0380                        /* RW Inverts polarity of PD_XO wh */
  #define SYSCFG_IOCR6_GPIO23_MUX                    0x0040                        /* RW mux out PWM3 output, 0 GPIO2 */
  #define SYSCFG_IOCR6_GPIO22_MUX                    0x0020                        /* RW mux out PWM2 output, 0 GPIO2 */
  #define SYSCFG_IOCR6_GPIO21_MUX                    0x0010                        /* RW concatenated with iocr6[30], */
  #define SYSCFG_IOCR6_GPIO20_MUX                    0x0008                        /* RW concatenated with iocr6[30], */
  #define SYSCFG_IOCR6_reserved1                     0x0004                        /* reserved1  */
  #define SYSCFG_IOCR6_D1W_maps_to_GPIO              0x0002                        /* RW If true, D1W maps to GPIO[21 */
  #define SYSCFG_IOCR6_reserved2                     0x0001                        /* reserved2  */
#define SYSCFG_IOCR7                     (SYSCFG_BASE_ADDR + 0x002C)  /* IOCR7 bit I/O Configuration Register 7       */
  #define SYSCFG_IOCR7_RFGPIO5_MUX                   0x80000000                    /* RW GPIO mux control on RFGPIO p */
  #define SYSCFG_IOCR7_RFGPIO4_MUX                   0x40000000                    /* RW GPIO mux control on RFGPIO p */
  #define SYSCFG_IOCR7_RFGPIO3_MUX                   0x20000000                    /* RW GPIO mux control on RFGPIO p */
  #define SYSCFG_IOCR7_RFGPIO2_MUX                   0x10000000                    /* RW GPIO mux control on RFGPIO p */
  #define SYSCFG_IOCR7_reserved0                     0x0FFC0000                    /* reserved0  */
  #define SYSCFG_IOCR7_RX3G_PULL                     0x00030000                    /* RW RX3GDATA[2:0] pull control,  */
  #define SYSCFG_IOCR7_TX3G_SLEW                     0x8000                        /* RW Slew control on TXDATA3G[2:0 */
  #define SYSCFG_IOCR7_TX3G_DRIVE                    0x7000                        /* RW Drive strength control of TX */
  #define SYSCFG_IOCR7_CLKX8_SLEW                    0x0800                        /* RW Slew control on CLKX8 pad, 0 */
  #define SYSCFG_IOCR7_CLKX8_DRIVE                   0x0700                        /* RW Drive strength control of CL */
  #define SYSCFG_IOCR7_reserved1                     0x00C0                        /* reserved1  */
  #define SYSCFG_IOCR7_DATASEL_3G2G                  0x0030                        /* RW Select between 3G/2G data in */
  #define SYSCFG_IOCR7_reserved2                     0x000E                        /* reserved2  */
  #define SYSCFG_IOCR7_CTRLSEL_3G2G                  0x0001                        /* RW Select between 3G/2G control */
#define SYSCFG_TVENCCR                   (SYSCFG_BASE_ADDR + 0x0034)  /* TVENCCR bit Analog TV Out (TVENC) Configurat */
  #define SYSCFG_TVENCCR_reserved0                   0xFFFFFFC0                      /* reserved0  */
  #define SYSCFG_TVENCCR_DMA_WAIT_CYCLE              0x0030                          /* RW DMA Request Wait Cycle: 0  */
  #define SYSCFG_TVENCCR_reserved1                   0x0008                          /* reserved1  */
  #define SYSCFG_TVENCCR_DISPLAYC_HRSTN              0x0004                          /* RW Displayc Reset, Active Low */
  #define SYSCFG_TVENCCR_VEC_HRSTN                   0x0002                          /* RW Vec Reset, Active Low,  */
  #define SYSCFG_TVENCCR_TVENC_EN                    0x0001                          /* RW Enable Analog TV Out Modul */
#define SYSCFG_DSICR                     (SYSCFG_BASE_ADDR + 0x0038)  /* DSICR bit MIPI DSI Configuration Register    */
  #define SYSCFG_DSICR_reserved0                     0xFFFFFFFE                    /* reserved0  */
  #define SYSCFG_DSICR_DSI_EN                        0x0001                        /* RW Enable AHB Clock to the MIPI */
#define SYSCFG_MCR                       (SYSCFG_BASE_ADDR + 0x0040)  /* MCR bit Boot ROM Remap Register */
  #define SYSCFG_MCR_reserved0                       0xFFFFFFFE                  /* reserved0  */
  #define SYSCFG_MCR_REMAP                           0x0001                      /* WO Writing to this register remov */
#define SYSCFG_MRR                       (SYSCFG_BASE_ADDR + 0x0044)  /* MRR bit Boot ROM Restore Register */
  #define SYSCFG_MRR_RESTORE                         0xFFFFFFFF                  /* WO This register restores the boo */
#define SYSCFG_RAMCTRL                   (SYSCFG_BASE_ADDR + 0x0048)  /* RAMCTRL bit RAM Control Register             */
  #define SYSCFG_RAMCTRL_reserved0                   0xFFFE0000                      /* reserved0  */
  #define SYSCFG_RAMCTRL_RED_OTP_RST                 0x00010000                      /* RW Redundancy OTP soft reset, */
  #define SYSCFG_RAMCTRL_reserved1                   0xF800                          /* reserved1  */
  #define SYSCFG_RAMCTRL_ROM_TM                      0x07C0                          /* RW ROM Testmode bits,  */
  #define SYSCFG_RAMCTRL_SRAM_TM                     0x003C                          /* RW SRAM Testmode bits,  */
  #define SYSCFG_RAMCTRL_STBY                        0x0003                          /* RW SRAM STBY bits,  */
#define SYSCFG_SECCTRL                   (SYSCFG_BASE_ADDR + 0x0050)  /* SECCTRL bit Security Control Register        */
  #define SYSCFG_SECCTRL_reserved0                   0xFFFFFFE0                      /* reserved0  */
  #define SYSCFG_SECCTRL_CRYPTO_DIS                  0x0010                          /* RW 0 Enable access to DES and */
  #define SYSCFG_SECCTRL_reserved1                   0x0008                          /* reserved1  */
  #define SYSCFG_SECCTRL_OTP_DIS                     0x0004                          /* RW 0 Enable access to the OTP */
  #define SYSCFG_SECCTRL_RTC_DIS_WR                  0x0002                          /* RW 0 Enable write access to t */
  #define SYSCFG_SECCTRL_BRM_DIS_RD                  0x0001                          /* RW 0 Enable read access to th */
#define SYSCFG_SECSTAT                   (SYSCFG_BASE_ADDR + 0x0054)  /* SECSTAT bit Security Status Register         */
  #define SYSCFG_SECSTAT_reserved0                   0xC0000000                      /* reserved0  */
  #define SYSCFG_SECSTAT_SEC_MODE_STATE              0x3C000000                      /* RO Secure Mode State:, 4'b000 */
  #define SYSCFG_SECSTAT_JTAG_DIS                    0x02000000                      /* RO nvm_glb_disable_jtag (READ */
  #define SYSCFG_SECSTAT_ETM_DIS                     0x01000000                      /* RO nvm_glb_disable_etm (READ- */
  #define SYSCFG_SECSTAT_SECURE_DEBUG                0x00FFFFFF                      /* RO SECURE DEBUG bits from OTP */
#define SYSCFG_MARM11CR                  (SYSCFG_BASE_ADDR + 0x0058)  /* MARM11CR bit Modem ARM11 Top Configuration R */
  #define SYSCFG_MARM11CR_reserved0                  0xFFFFFFFC                       /* reserved0  */
  #define SYSCFG_MARM11CR_Page                       0x0003                           /* RW Page[1:0] Controls the pa */
#define SYSCFG_MAMBACR                   (SYSCFG_BASE_ADDR + 0x0060)  /* MAMBACR bit Mamba Top Configuration Register */
  #define SYSCFG_MAMBACR_MAMBA_STATUS                0xFF000000                      /* RW Mamba (EMI) Status (READ-O */
  #define SYSCFG_MAMBACR_reserved0                   0x00FFF000                      /* reserved0  */
  #define SYSCFG_MAMBACR_NOR_REQUEST                 0x0800                          /* RW I_nor_request,  */
  #define SYSCFG_MAMBACR_reserved1                   0x0400                          /* reserved1  */
  #define SYSCFG_MAMBACR_PWRDN_EXIT                  0x0200                          /* RW I_hw_pwrdwn_exit,  */
  #define SYSCFG_MAMBACR_HIB_EXIT_MODE               0x0100                          /* RW I_hib_exit_mode,  */
  #define SYSCFG_MAMBACR_FREQ_CHANGE_REQ             0x0080                          /* RW I_freq_change_req,  */
  #define SYSCFG_MAMBACR_EMI_TEST                    0x0040                          /* RW I_emi_test,  */
  #define SYSCFG_MAMBACR_DEBUG_CTRL                  0x003C                          /* RW I_debug_ctrl[3:0],  */
  #define SYSCFG_MAMBACR_CLK_EMI_EQ_DRAM             0x0002                          /* RW I_clk_emi_eq_dram,  */
  #define SYSCFG_MAMBACR_reserved2                   0x0001                          /* reserved2  */
#define SYSCFG_RFCR                      (SYSCFG_BASE_ADDR + 0x0064)  /* RFCR bit Integrated RF Top Configuration Reg */
  #define SYSCFG_RFCR_reserved0                      0xFFFFFFFF                   /* reserved0  */

#define SYSCFG_ANACR4                    (SYSCFG_BASE_ADDR + 0x0090)  /* ANACR4 bit Analog Configuration Register 4   */
  #define SYSCFG_ANACR4_reserved0                    0xFFFFFFF0                     /* reserved0  */
  #define SYSCFG_ANACR4_i_shuffctl                   0x000C                         /* RW Shuffler Control:, 00: 1-z- */
  #define SYSCFG_ANACR4_i_refampbctl                 0x0003                         /* RW Reference buffer bias contr */
#define SYSCFG_ANACR5                    (SYSCFG_BASE_ADDR + 0x0094)  /* ANACR5 bit Analog Configuration Register 5   */
  #define SYSCFG_ANACR5_BGTC                         0xC0000000                     /* RW Temperature compensation ad */
  #define SYSCFG_ANACR5_reserved0                    0x3F800000                     /* reserved0  */
  #define SYSCFG_ANACR5_AUX_ADC_SC                   0x00600000                     /* RW WCDMA Aux DAC Clock Select, */
  #define SYSCFG_ANACR5_AUX_DAC_SI                   0x00180000                     /* RW WCDMA Aux DAC Power Down, 1 */
  #define SYSCFG_ANACR5_AUX_DAC0_SC                  0x00060000                     /* RW WCDMA Aux ADC 0 Clock Selec */
  #define SYSCFG_ANACR5_reserved1                    0x0001F800                     /* reserved1  */
  #define SYSCFG_ANACR5_AUX_DAC_CM                   0x0780                         /* RW WCDMA Aux DAC 0 Output Comm */
  #define SYSCFG_ANACR5_AUX_DAC_PD                   0x0040                         /* RW WCDMA Aux DAC0 Output Pull  */
  #define SYSCFG_ANACR5_AUX_DAC_IBIAS                0x0030                         /* RW WCDMA Aux DAC 0 Opamp Bias  */
  #define SYSCFG_ANACR5_AUX_CLK_CTRL                 0x0008                         /* RW WCDMA Aux Clock 0 On/Off Co */
  #define SYSCFG_ANACR5_AUX_CLK_INV                  0x0004                         /* RW WCDMA Aux Clock 0 Signal In */
  #define SYSCFG_ANACR5_AUX_Output                   0x0002                         /* RW Two’s Compliment/Offset B */
  #define SYSCFG_ANACR5_Pwd_Aux_dac                  0x0001                         /* RW Aux DAC 0 Power down, 1: Po */
#define SYSCFG_ANACR6                    (SYSCFG_BASE_ADDR + 0x0098)  /* ANACR6 bit Analog Configuration Register 6   */
  #define SYSCFG_ANACR6_reserved0                    0xFFF80000                     /* reserved0  */
  #define SYSCFG_ANACR6_auxdac1sc                    0x00060000                     /* RW WCDMA Aux DAC1 Clock Select */
  #define SYSCFG_ANACR6_reserved1                    0x0001F800                     /* reserved1  */
  #define SYSCFG_ANACR6_AUX_DAC_CM                   0x0780                         /* RW WCDMA Aux DAC 1 Output Comm */
  #define SYSCFG_ANACR6_AUX_DAC_PD                   0x0040                         /* RW WCDMA Aux DAC 1 Output Pull */
  #define SYSCFG_ANACR6_AUX_DAC_IBIAS                0x0030                         /* RW WCDMA Aux DAC 1 Opamp Bias  */
  #define SYSCFG_ANACR6_AUX_CLK_CTRL                 0x0008                         /* RW WCDMA AUX Clock 1 On/Off Co */
  #define SYSCFG_ANACR6_AUX_CLK_INV                  0x0004                         /* RW WCDMA AUX Clock 1 Signal In */
  #define SYSCFG_ANACR6_WCDMA                        0x0002                         /* RW AUX DAC Twos Complement/Off */
  #define SYSCFG_ANACR6_Pwd_Aux_dac                  0x0001                         /* RW Aux DAC 0 Power down, 1: Po */
#define SYSCFG_ANACR7                    (SYSCFG_BASE_ADDR + 0x009C)  /* ANACR7 bit Analog Configuration Register 7   */
  #define SYSCFG_ANACR7_reserved0                    0x80000000                     /* reserved0  */
  #define SYSCFG_ANACR7_rxsc                         0x40000000                     /* RW WCDMA RX Clock Select, 0: N */
  #define SYSCFG_ANACR7_reserved1                    0x20000000                     /* reserved1  */
  #define SYSCFG_ANACR7_disablehvregulator           0x10000000                     /* RW 0 Normal Operationg, 1 Disa */
  #define SYSCFG_ANACR7_inputcmenable                0x08000000                     /* RW WCDMA RX AC Common Mode Con */
  #define SYSCFG_ANACR7_albselect                    0x04000000                     /* RW WCDMA RX Analog Loopback Co */
  #define SYSCFG_ANACR7_shufflectrl                  0x03000000                     /* RW WCDMA RX ADC Shuffle Contro */
  #define SYSCFG_ANACR7_dithctrl                     0x00C00000                     /* RW WCDMA RX ADC Dither Control */
  #define SYSCFG_ANACR7_vcmctrl_FLASH                0x00300000                     /* RW WCDMA RX ADC FLASH Common M */
  #define SYSCFG_ANACR7_vcmctrl_INT2                 0x000C0000                     /* RW WCDMA RX ADC INT2 output Co */
  #define SYSCFG_ANACR7_vcmctrl_INT1                 0x00030000                     /* RW WCDMA RX ADC INT1 output Co */
  #define SYSCFG_ANACR7_dithrefoffsetctrl            0xC000                         /* RW WCDMA RX ADC Dither Referen */
  #define SYSCFG_ANACR7_adcrefoffsetctrl             0x3000                         /* RW WCDMA RX ADC reference offs */
  #define SYSCFG_ANACR7_biasctrl_INT2                0x0C00                         /* RW WCDMA RX ADC INT2 Bias Curr */
  #define SYSCFG_ANACR7_biasctrl_INT1                0x0300                         /* RW WCDMA RX ADC INT1 Bias Curr */
  #define SYSCFG_ANACR7_biasctrl_ADC                 0x00C0                         /* RW WCDMA RX ADC Bias Current C */
  #define SYSCFG_ANACR7_rxpgaset                     0x003C                         /* RW WCDMA RX ADC Input PGA Cont */
  #define SYSCFG_ANACR7_rxresetb                     0x0002                         /* RW 0 Reset WCDMA RX Signal, 1  */
  #define SYSCFG_ANACR7_rxpwrdn                      0x0001                         /* RW 0 WCDMA RX Normal Operation */
#define SYSCFG_ANACR8                    (SYSCFG_BASE_ADDR + 0x00A0)  /* ANACR8 bit Analog Configuration Register 8   */
  #define SYSCFG_ANACR8_reserved0                    0xFC000000                     /* reserved0  */
  #define SYSCFG_ANACR8_ibcasctrl                    0x03800000                     /* RW WCDMA Transmit DAC Cascade  */
  #define SYSCFG_ANACR8_reserved1                    0x00400000                     /* reserved1  */
  #define SYSCFG_ANACR8_ibcmctrl                     0x00300000                     /* RW WCDMA Transmit Common-Mode  */
  #define SYSCFG_ANACR8_ibampctrl                    0x000E0000                     /* RW WCDMA Transmit Buffer Ampli */
  #define SYSCFG_ANACR8_txampctrl                    0x0001E000                     /* RW WCDMA Transmit Amplitude co */
  #define SYSCFG_ANACR8_txvcmctrl                    0x1C00                         /* RW WCDMA Transmit Common-Mode  */
  #define SYSCFG_ANACR8_clk_disable                  0x0200                         /* RW WCDMA Transmit Clock Contro */
  #define SYSCFG_ANACR8_clkinv                       0x0100                         /* RW WCDMA Transmit Clock Invers */
  #define SYSCFG_ANACR8_txobb                        0x0080                         /* RW WCDMA Transmit Output Forma */
  #define SYSCFG_ANACR8_TXSC                         0x0060                         /* RW WCDMA Transmit Clock Select */
  #define SYSCFG_ANACR8_TXSIQ                        0x0018                         /* RW WCDMA Transmit Q channel Da */
  #define SYSCFG_ANACR8_TXSII                        0x0006                         /* RW WCDMA Transmit I Channel Da */
  #define SYSCFG_ANACR8_pwrdnTX                      0x0001                         /* RW WCDMA Transmit Power Down,  */
#define SYSCFG_ANACR9                    (SYSCFG_BASE_ADDR + 0x00A4)  /* ANACR9 bit Analog Configuration Register 9   */
  #define SYSCFG_ANACR9_Generic_ctl                  0x80000000                     /* RW USB Remote Wake-Up Enable i */
  #define SYSCFG_ANACR9_suspend_eco_fix_en           0x40000000                     /* RW 0 Not enables the fix for t */
  #define SYSCFG_ANACR9_reserved0                    0x20000000                     /* reserved0  */
  #define SYSCFG_ANACR9_afe_non_driving              0x10000000                     /* RW Removes all Termination and */
  #define SYSCFG_ANACR9_afe_chrpten                  0x08000000                     /* RW USB Chirp Transmit Control, */
  #define SYSCFG_ANACR9_afe_lpback                   0x04000000                     /* RW 1 USB TX Data Comes Back in */
  #define SYSCFG_ANACR9_afe_cdrcken                  0x02000000                     /* RW && afe_clken USB 960/480/12 */
  #define SYSCFG_ANACR9_clk_60_invert                0x01000000                     /* RW ???, 0 Invert the Clk60 to  */
  #define SYSCFG_ANACR9_afe_hstxen                   0x00800000                     /* RW USB High Speed Current Cont */
  #define SYSCFG_ANACR9_reserved1                    0x00400000                     /* reserved1  */
  #define SYSCFG_ANACR9_UTMI_loopback                0x00200000                     /* RW UTMI Logic Control, 0 Enabl */
  #define SYSCFG_ANACR9_afe_rxlogicr                 0x00100000                     /* RW This is the CDR 480 clock e */
  #define SYSCFG_ANACR9_iost_control                 0x00080000                     /* RW Direct control over the ios */
  #define SYSCFG_ANACR9_Afe_clsp                     0x00040000                     /* RW resume_filterb,  */
  #define SYSCFG_ANACR9_Iddq_en                      0x00020000                     /* RW Bit stuff error enable;, 0  */
  #define SYSCFG_ANACR9_reserved2                    0x00018000                     /* reserved2  */
  #define SYSCFG_ANACR9_utmi_discon_phy              0x4000                         /* RW 0 Default, 1 Host sees a di */
  #define SYSCFG_ANACR9_tx_phase                     0x2000                         /* RW Flipping 480MHz phase cause */
  #define SYSCFG_ANACR9_sync_det_length              0x1C00                         /* RW Adjusts USB RX sync detecti */
  #define SYSCFG_ANACR9_reset_hi_pll                 0x0200                         /* RW USB PLL Reset, 0 Disabled,  */
  #define SYSCFG_ANACR9_pll_suspend_en               0x0100                         /* RW USB PLL Power-down During S */
  #define SYSCFG_ANACR9_chrp_rx_sel                  0x0080                         /* RW USB RX Chirp Detection, 0 A */
  #define SYSCFG_ANACR9_utmi_pwrdwnb                 0x0040                         /* RW Port Digital Power-Down, 0  */
  #define SYSCFG_ANACR9_soft_resetb                  0x0020                         /* RW UTMI Soft Reset Control, 0  */
  #define SYSCFG_ANACR9_pll_pwrdwnb                  0x0010                         /* RW PLL Power-Down, 1 Power-On, */
  #define SYSCFG_ANACR9_pll_calen                    0x0008                         /* RW PLL Calibration Control, 0  */
  #define SYSCFG_ANACR9_otg_mode                     0x0004                         /* RW USB Mode Selection, 0 Stand */
  #define SYSCFG_ANACR9_hostb_dev                    0x0002                         /* RW USB Host Selection, 0 Host, */
  #define SYSCFG_ANACR9_afe_pwrdwnb                  0x0001                         /* RW Analog Port Power Control,  */

#define SYSCFG_ANACR11                   (SYSCFG_BASE_ADDR + 0x00AC)  /* ANACR11 bit Analog Configuration Register 11 */
  #define SYSCFG_ANACR11_reserved0                   0xFFFFFFFF                      /* reserved0  */
#define SYSCFG_ANACR12                   (SYSCFG_BASE_ADDR + 0x00B0)  /* ANACR12 bit Analog Configuration Register 12 */
  #define SYSCFG_ANACR12_reserved0                   0xFFFFFFFF                      /* reserved0  */
#define SYSCFG_ANACR13                   (SYSCFG_BASE_ADDR + 0x00B4)  /* ANACR13 bit Analog Configuration Register 13 */
  #define SYSCFG_ANACR13_reserved0                   0xFFFFFFFF                      /* reserved0  */
#define SYSCFG_ANACR14                   (SYSCFG_BASE_ADDR + 0x00B8)  /* ANACR14 bit Analog Configuration Register 14 */
  #define SYSCFG_ANACR14_reserved0                   0xFFFFFFFF                      /* reserved0  */
#define SYSCFG_ANACR15                   (SYSCFG_BASE_ADDR + 0x00BC)  /* ANACR15 bit Analog Configuration Register 15 */
  #define SYSCFG_ANACR15_reserved0                   0xFFFFFFFF                      /* reserved0  */
#define SYSCFG_IRDROP_MON0               (SYSCFG_BASE_ADDR + 0x00C0)  /* IRDROP_MON0 bit IRDROP Monitor Register 0    */
  #define SYSCFG_IRDROP_MON0_reserved0               0xFFFFF000                          /* reserved0  */
  #define SYSCFG_IRDROP_MON0_OSC_EN                  0x0800                              /* RW Reserved - For Interna */
  #define SYSCFG_IRDROP_MON0_MON_EN                  0x0400                              /* RW Write 1 to enable coun */
  #define SYSCFG_IRDROP_MON0_CNT_OUT                 0x03FF                              /* RW Software can read this */
#define SYSCFG_IRDROP_MON1               (SYSCFG_BASE_ADDR + 0x00C4)  /* IRDROP_MON1 bit IRDROP Monitor Register 1    */
  #define SYSCFG_IRDROP_MON1_reserved0               0xFFFFF000                          /* reserved0  */
  #define SYSCFG_IRDROP_MON1_OSC_EN                  0x0800                              /* RW Reserved - For Interna */
  #define SYSCFG_IRDROP_MON1_MON_EN                  0x0400                              /* RW Write 1 to enable coun */
  #define SYSCFG_IRDROP_MON1_CNT_OUT                 0x03FF                              /* RW Software can read this */
#define SYSCFG_IRDROP_MON2               (SYSCFG_BASE_ADDR + 0x00C8)  /* IRDROP_MON2 bit IRDROP Monitor Register 2    */
  #define SYSCFG_IRDROP_MON2_reserved0               0xFFFFF000                          /* reserved0  */
  #define SYSCFG_IRDROP_MON2_OSC_EN                  0x0800                              /* RW Reserved - For Interna */
  #define SYSCFG_IRDROP_MON2_MON_EN                  0x0400                              /* RW Write 1 to enable coun */
  #define SYSCFG_IRDROP_MON2_CNT_OUT                 0x03FF                              /* RW Software can read this */
#define SYSCFG_MDIO_WRITE                (SYSCFG_BASE_ADDR + 0x00CC)  /* MDIO_WRITE MDIO WRITE Register               */
  #define SYSCFG_MDIO_WRITE_WRITE                    0x80000000                         /* RW START MDIO write strobe */
  #define SYSCFG_MDIO_WRITE_READ                     0x40000000                         /* RW START MDIO read strobe, */
  #define SYSCFG_MDIO_WRITE_MDIO3_SM_SEL             0x20000000                         /* RW 1 : Uses internal regis */
  #define SYSCFG_MDIO_WRITE_MDIO2                    0x1F000000                         /* RW ID 5-bit ID. This must  */
  #define SYSCFG_MDIO_WRITE_MDIO1                    0x00FF0000                         /* RW ADDR Register address,  */
  #define SYSCFG_MDIO_WRITE_REG                      0xFFFF                             /* RW WR DATA Register write  */
#define SYSCFG_MDIO_READ                 (SYSCFG_BASE_ADDR + 0x00D0)  /* MDIO_READ MDIO READ Register                 */
  #define SYSCFG_MDIO_READ_reserved0                 0xFFFC0000                        /* reserved0  */
  #define SYSCFG_MDIO_READ_VBUS_STAT2                0x00020000                        /* RW vbus_status2 read data,  */
  #define SYSCFG_MDIO_READ_VBUS_STAT1                0x00010000                        /* RW vbus_status1 read data,  */
  #define SYSCFG_MDIO_READ_REG_RD_DATA               0xFFFF                            /* RW Register read data,  */
#define SYSCFG_PERIPH_AHB_CLK_GATE_MASK  (SYSCFG_BASE_ADDR + 0x00D8)  /* PERIPHERALS AHB CLOCK GATE MASK REGISTER     */
  #define SYSCFG_PERIPH_AHB_CLK_GATE_MASK_PERIPH_AHB_CLK_GATE_MASK       0xFFFFFFFF
                                                                     /* RW AHB Clock Gate Mask Register (Read-Write), */
#define SYSCFG_PERIPH_AHB_CLK_GATE_FORCE (SYSCFG_BASE_ADDR + 0x00DC)  /* PERIPHERALS AHB CLOCK GATE FORCE REGISTER    */
  #define SYSCFG_PERIPH_AHB_CLK_GATE_FORCE_PERIPH_AHB_CLK_GATE_FORCE       0xFFFFFFFF
                                                                    /* RW AHB Clock Gate Force Register (Read-Write), */
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW (SYSCFG_BASE_ADDR + 0x00E0)
                                                                    /* PERIPHERALS AHB CLOCK GATE MONITOR RAW REGISTE */
  #define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_RAW_PERIPH_AHB_CLK_GATE_MON_RAW       0xFFFFFFFF
                                                               /* RO AHB Clock Gate Monitor Raw Register (Read-Only), */
#define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR (SYSCFG_BASE_ADDR + 0x00E4)
                                                                        /* PERIPHERALS AHB CLOCK GATE MONITOR REGISTE */
  #define SYSCFG_PERIPH_AHB_CLK_GATE_MONITOR_PERIPH_AHB_CLK_GATE_MON       0xFFFFFFFF
                                                                   /* RO AHB Clock Gate Monitor Register (Read-Only), */
#define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN (SYSCFG_BASE_ADDR + 0x0100)
                                                      /* PERIPHERALS VIDEO CODEC AHB CLOCK ENABLE AND REQUEST REGISTE */
  #define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                         /* reserved0 */
  #define SYSCFG_PERIPH_VIDEO_CODEC_AHB_CLK_EN_EN       0x0001
                                                         /* RW Video Codec AHB Clock Enable and Request (Read-Write), */
#define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN (SYSCFG_BASE_ADDR + 0x0104)
                                                 /* PERIPHERALS CAMARA INTERFACE AHB CLOCK ENABLE AND REQUEST REGISTE */
  #define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                         /* reserved0 */
  #define SYSCFG_PERIPH_CAMARA_INTERFACE_AHB_CLK_EN_EN       0x0001
                                                    /* RW Camara Interface AHB Clock Enable and Request (Read-Write), */
#define SYSCFG_PERIPH_USB_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x0108)  /* PERIPHERALS USB AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_USB_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_USB_AHB_CLK_EN_EN            0x0001                                        /* RW USB AHB Cloc */
#define SYSCFG_PERIPH_GEA_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x010C)  /* PERIPHERALS GEA AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_GEA_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_GEA_AHB_CLK_EN_EN            0x0001                                        /* RW GEA AHB Cloc */
#define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN  (SYSCFG_BASE_ADDR + 0x0110)  /* PERIPHERALS CRYPTO AHB CLOCK ENABLE REGISTER */
  #define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                      /* reserved0  */
  #define SYSCFG_PERIPH_CRYPTO_AHB_CLK_EN_EN         0x0001                                           /* RW Crypto AH */
#define SYSCFG_PERIPH_PKA_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x0114)  /* PERIPHERALS PKA AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_PKA_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_PKA_AHB_CLK_EN_EN            0x0001                                        /* RW PKA AHB Cloc */
#define SYSCFG_PERIPH_UARTA_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x0118)  /* PERIPHERALS UARTA AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_UARTA_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_UARTA_AHB_CLK_EN_EN          0x0001                                          /* RW UARTA AHB  */
#define SYSCFG_PERIPH_UARTB_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x011C)  /* PERIPHERALS UARTB AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_UARTB_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_UARTB_AHB_CLK_EN_EN          0x0001                                          /* RW UARTB AHB  */
#define SYSCFG_PERIPH_DA_AHB_CLK_EN      (SYSCFG_BASE_ADDR + 0x0120)  /* PERIPHERALS DA AHB CLOCK ENABLE REGISTER     */
  #define SYSCFG_PERIPH_DA_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                  /* reserved0  */
  #define SYSCFG_PERIPH_DA_AHB_CLK_EN_EN             0x0001                                       /* RW DA AHB Clock  */
#define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x0124)  /* PERIPHERALS MPCLK AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_MPCLK_AHB_CLK_EN_EN          0x0001                                          /* RW MPClk AHB  */
#define SYSCFG_PERIPH_LCD_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x012C)  /* PERIPHERALS LCD AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_LCD_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_LCD_AHB_CLK_EN_EN            0x0001                                        /* RW LCD AHB Cloc */
#define SYSCFG_PERIPH_DPE_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x0130)  /* PERIPHERALS DPE AHB CLOCK ENABLE REGISTER AN */
  #define SYSCFG_PERIPH_DPE_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_DPE_AHB_CLK_EN_EN            0x0001                                        /* RW DPE AHB Cloc */
#define SYSCFG_PERIPH_DMAC_AHB_CLK_EN    (SYSCFG_BASE_ADDR + 0x0134)  /* PERIPHERALS DMAC AHB CLOCK ENABLE REGISTER   */
  #define SYSCFG_PERIPH_DMAC_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                    /* reserved0  */
  #define SYSCFG_PERIPH_DMAC_AHB_CLK_EN_EN           0x0001                                         /* RW DMAC AHB Cl */
#define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x0138)  /* PERIPHERALS SDIO1 AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_SDIO1_AHB_CLK_EN_EN          0x0001                                          /* RW SDIO1 AHB  */
#define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x013C)  /* PERIPHERALS SDIO2 AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_SDIO2_AHB_CLK_EN_EN          0x0001                                          /* RW SDIO2 AHB  */
#define SYSCFG_PERIPH_DES_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x0144)  /* PERIPHERALS DES AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_DES_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_DES_AHB_CLK_EN_EN            0x0001                                        /* RW DES AHB Cloc */
#define SYSCFG_PERIPH_UARTC_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x014C)  /* PERIPHERALS UARTC AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_UARTC_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_UARTC_AHB_CLK_EN_EN          0x0001                                          /* RW UARTC AHB  */
#define SYSCFG_PERIPH_RNG_AHB_CLK_EN     (SYSCFG_BASE_ADDR + 0x0150)  /* PERIPHERALS RNG AHB CLOCK ENABLE REGISTER    */
  #define SYSCFG_PERIPH_RNG_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                   /* reserved0  */
  #define SYSCFG_PERIPH_RNG_AHB_CLK_EN_EN            0x0001                                        /* RW RNG AHB Cloc */
#define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x0154)  /* PERIPHERALS SDIO3 AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_SDIO3_AHB_CLK_EN_EN          0x0001                                          /* RW SDIO3 AHB  */
#define SYSCFG_PERIPH_TVENC_AHB_CLK_EN   (SYSCFG_BASE_ADDR + 0x0158)  /* PERIPHERALS TVENC AHB CLOCK ENABLE REGISTER  */
  #define SYSCFG_PERIPH_TVENC_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                     /* reserved0  */
  #define SYSCFG_PERIPH_TVENC_AHB_CLK_EN_EN          0x0001                                          /* RW TVEnc AHB  */
#define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN (SYSCFG_BASE_ADDR + 0x015C)
                                                                    /* PERIPHERALS FSUSBHOST AHB CLOCK ENABLE REGISTE */
  #define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                         /* reserved0 */
  #define SYSCFG_PERIPH_FSUSBHOST_AHB_CLK_EN_EN       0x0001
                                                                       /* RW FSUSBHOST AHB Clock Enable (Read-Write), */
#define SYSCFG_PERIPH_MPHI_AHB_CLK_EN    (SYSCFG_BASE_ADDR + 0x0160)  /* PERIPHERALS MPHI AHB CLOCK ENABLE REGISTER   */
  #define SYSCFG_PERIPH_MPHI_AHB_CLK_EN_reserved0       0xFFFFFFFE
                                                                                                    /* reserved0  */
  #define SYSCFG_PERIPH_MPHI_AHB_CLK_EN_EN           0x0001                                         /* RW MPHI AHB Cl */
#define SYSCFG_TESTABILITY_ACCESS        (SYSCFG_BASE_ADDR + 0x0170)  /* TESTABILITY_ACCESS Testability Access Regist */
  #define SYSCFG_TESTABILITY_ACCESS_reserved0        0xFFFFFFF0                                 /* reserved0  */
  #define SYSCFG_TESTABILITY_ACCESS_ETM_LOCK         0x0008                                     /* RW ETM Disable Loc */
  #define SYSCFG_TESTABILITY_ACCESS_SBD_DISABLE       0x0004
  /* RW ETM Disable - Can only disable, ETM for Security Levels 1-2, For Security Level 0:, X: Enable ETM, For Securi */
  #define SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE_LOCK       0x0002
  /* RW JTAG Disable Lock - Used by software to lock or unlock JTAG based on Security Configuration and authenticated */
  #define SYSCFG_TESTABILITY_ACCESS_JTAG_DISABLE       0x0001
  /* RW JTAG Disable - Can only disable JTAG if security level 1 or higher., For Security Level 0:, X: Enable JTAG, F */
#define SYSCFG_DISABLE_OTP_REGION_READ_ACCESS (SYSCFG_BASE_ADDR + 0x0174)
                                             /* DISABLE_OTP_REGION_READ_ACCESS Disable OTP Region Read Access Registe */
  #define SYSCFG_DISABLE_OTP_REGION_READ_ACCESS_DIS_OTP_RGN_RD_n       0xFFFFFFFF
  /* RW Disables read access to OTP Regions. (Write-once), 0: Read access to OTP region is enabled., 1: Read access t */
#define SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS (SYSCFG_BASE_ADDR + 0x0178)
                                           /* DISABLE_OTP_REGION_WRITE_ACCESS Disable OTP Region Write Access Registe */
  #define SYSCFG_DISABLE_OTP_REGION_WRITE_ACCESS_DIS_OTP_RGN_WR_n       0xFFFFFFFF
  /* RW Disables Write access to OTP Regions. (Write-once), 0: Write (Programming) access to OTP region is enabled.,  */
#define SYSCFG_OTP_DEVICE_STATUS         (SYSCFG_BASE_ADDR + 0x017C)  /* OTP_DEVICE_STATUS OTP device status bits.    */
  #define SYSCFG_OTP_DEVICE_STATUS_reserved0         0xFFFF0000                                /* reserved0  */
  #define SYSCFG_OTP_DEVICE_STATUS_DEVICE_STATUS       0xFFFF
                                                                      /* RW Device status bits from OTP. (READ-ONLY), */
#define SYSCFG_FPGA_Version              (SYSCFG_BASE_ADDR + 0x01FC)  /* FPGA_VERSION FPGA Release Version Number     */
  #define SYSCFG_FPGA_Version_reserved0              0xFFFFFF00                           /* reserved0  */
  #define SYSCFG_FPGA_Version_FPGA_RLS_ID            0x00FF                               /* RO FPGA Release ID. (REA */

#endif /* __BRCM_RDB_SYSCFG_H__ */


