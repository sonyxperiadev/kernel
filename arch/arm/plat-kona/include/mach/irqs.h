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

#ifndef __PLAT_KONA_IRQS_H
#define __PLAT_KONA_IRQS_H

#define BCM_INT_PRIORITY_MAX    32	/* there are only 32 priority are supported */
#define BCM_INT_SPI_MAX         128	/* there are 128 shared peripheral interrupt */

/*=====================================================================*/
/* Software Trigger Interrupt IDs                                      */
/*=====================================================================*/
#define BCM_INT_ID_STI0                 0
#define BCM_INT_ID_STI1                 1
#define BCM_INT_ID_STI2                 2
#define BCM_INT_ID_STI3                 3
#define BCM_INT_ID_STI4                 4
#define BCM_INT_ID_STI5                 5
#define BCM_INT_ID_STI6                 6
#define BCM_INT_ID_STI7                 7
#define BCM_INT_ID_STI8                 8
#define BCM_INT_ID_STI9                 9
#define BCM_INT_ID_STI10                10
#define BCM_INT_ID_STI11                11
#define BCM_INT_ID_STI12                12
#define BCM_INT_ID_STI13                13
#define BCM_INT_ID_STI14                14
#define BCM_INT_ID_STI15                15
#define BCM_INT_ID_STI_MAX              16	/* terminating ID */

/*=====================================================================*/
/* Private Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#define BCM_INT_ID_PPI0                 ( 0 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI1                 ( 1 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI2                 ( 2 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI3                 ( 3 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI4                 ( 4 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI5                 ( 5 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI6                 ( 6 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI7                 ( 7 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI8                 ( 8 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI9                 ( 9 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI10                (10 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI11                (11 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI12                (12 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI13                (13 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI14                (14 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI15                (15 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI_MAX              (16 + BCM_INT_ID_STI_MAX)	/* terminating ID */

/*=====================================================================*/
/* Shared Peripheral Interrupt IDs                                     */
/*=====================================================================*/
#define BCM_INT_ID_WATCH_DOG            ( 0 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL0                 ( 1 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL1                 ( 2 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL2                 ( 3 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CTI                  ( 4 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP8         ( 5 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HUB_TIMERS2          ( 6 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HUB_TIMERS1          ( 7 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PERIPH_TIMERS2       ( 8 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PERIPH_TIMERS1       ( 9 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL3                 (10 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL4                 (11 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL5                 (12 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PM_LOCK              (13 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PM_I2C               (14 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PWR_MGR              (15 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_IPC_SECURE           (16 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_IPC_OPEN             (17 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC_SECURE_0        (18 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP1         (19 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP2         (20 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP3         (21 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP4         (22 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP7         (23 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP5         (24 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SECURE_TRAP6         (25 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SPUM_SECURE          (26 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SPUM_OPEN            (27 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SMI2                 (28 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_COM_MTX0             (29 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_COM_MTX1             (30 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_COM_MRX0             (31 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_COM_MRX1             (32 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PMU_IRQ0             (33 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PMU_IRQ1             (34 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CTI_IRQ0             (35 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CTI_IRQ1             (36 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_USB_OTG_BVALID       (41 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_USB_OTG_DRV_VBUS     (42 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_USB_OTG_CHRG_VBUS    (43 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_USB_OTG_DISCHRG_VBUS (44 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED45           (45 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_USB_FSHOST           (46 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_IC_USB               BCM_INT_ID_USB_FSHOST
#define BCM_INT_ID_USB_HSOTG            (47 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ULPI_OHCI            (48 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ULPI_EHCI            (49 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HUB_TIMERS4          (50 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HUB_TIMERS3          (51 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PERIPH_TIMERS4       (52 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PERIPH_TIMERS3       (53 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MAGIC_GATE           (54 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MS_PRO_HG            (55 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED56           (56 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED57           (57 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_NOR                  (58 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_UNMANAGED_NAND       (59 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SYS_EMI_SECURE       (60 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SYS_EMI_OPEN         (61 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_VC_EMI_SECURE        (62 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_VC_EMI_OPEN          (63 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_UART3                (64 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_UART2                (65 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_UART1                (66 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_UART0                (67 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SIM_CARD             (68 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED69           (69 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_IRDA                 (70 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED71           (71 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED72           (72 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SDIO_MMC             (73 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SDIO_NAND            (74 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED75           (75 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SDIO1                (76 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SDIO0                (77 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ATA66                (78 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED79           (79 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SLIM                 (80 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_I2S                  (81 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED82           (82 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED83           (83 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SSP4                 (84 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SSP3                 (85 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SSP2                 (86 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED87           (87 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED88           (88 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SSP1                 (89 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SSP0                 (90 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL6                 (91 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL7                 (92 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_BBL8                 (93 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DALLAS_1_WIRE        (94 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM0                 (95 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM1                 (96 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM2                 (97 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM3                 (98 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM4                 (99 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM5                 (100 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM6                 (101 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_I2C1                 (102 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_I2C0                 (103 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MPHI                 (104 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED105          (105 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO1                (106 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED107          (107 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED108          (108 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO8                (109 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO7                (110 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO6                (111 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO5                (112 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO4                (113 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO3                (114 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_GPIO2                (115 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM7                 (116 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM8                 (117 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM9                 (118 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM10                (119 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM11                (120 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_AUDIO                (121 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSM13                (122 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_AUXMIC_COMP2_INV     (123 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CAPH                 (124 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED125          (125 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED126          (126 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_KEYPAD               (127 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_HSOTG_WAKEUP         (128 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED129          (129 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED130          (130 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_AUXMIC_COMP1         (131 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_AUXMIC_COMP2         (132 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED133          (133 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED134          (134 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED135          (135 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED136          (136 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED137          (137 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED138          (138 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED139          (139 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED140          (140 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED141          (141 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED142          (142 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED143          (143 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED144          (144 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED145          (145 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED146          (146 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED147          (147 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED148          (148 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED149          (149 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_SMI		        (150 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MM_DMA_CHAN1         (151 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED152          (152 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED153          (153 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DSI0                 (154 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DSI1                 (155 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CSI			(156 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED157          (157 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MM_DMA_CHAN2         (158 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MM_DMA_CHAN3         (159 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED160          (160 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED161          (161 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED162          (162 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED163          (163 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED164          (164 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED165          (165 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED166          (166 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED167          (167 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CAPH_ERR             (168 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_I2C2			(169 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_I2C3			(170 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED171          (171 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED172          (172 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED173          (173 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED174          (174 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED175          (175 + BCM_INT_ID_PPI_MAX)
#if defined (CONFIG_ARCH_HAWAII) || defined (CONFIG_ARCH_JAVA)
#define BCM_INT_ID_H264_AOB             (176 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_H264_CME             (177 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_H264_MCIN_CBC        (178 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_AXIPV		(179 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_PV			(180 + BCM_INT_ID_PPI_MAX)
#else
#define BCM_INT_ID_RESERVED176          (176 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED177          (177 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_VPM                  (178 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED179          (179 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED180          (180 + BCM_INT_ID_PPI_MAX)
#endif
#define BCM_INT_ID_RESERVED181          (181 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED182          (182 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_TEMP_MON             (183 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC0		(184 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC1		(185 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC2		(186 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC3		(187 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC4		(188 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC5		(189 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC6		(190 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_DMAC7		(191 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_0          (192 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_1          (193 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_2          (194 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_3          (195 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_4          (196 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_5          (197 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_6          (198 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_ESUB_DMAC_7          (199 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_MAX                  (200 + BCM_INT_ID_PPI_MAX)	/* terminating ID */
#define BCM_INT_ID_CORE_TIMERS1         (210 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CORE_TIMERS2         (211 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CORE_TIMERS3         (212 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_CORE_TIMERS4         (213 + BCM_INT_ID_PPI_MAX)

/* temporarily for new DMAC before new interrrupt number release */
#define     INTP_DMAC_ABORT                                        45
#define     INTV_DMAC_SECURE                                       69
#define     INTV_DMAC_OPEN                                         70

/* The IPC virtual interrupt IDs */
#define		IRQ_IPC_0		(BCM_INT_ID_MAX)
#define		NUM_IPC_IRQS		32

/* For AP-CP */
#define IRQ_APCP_0		BCM_INT_ID_RESERVED36
#define IRQ_IPC_A2C             235
#define IRQ_IPC_C2A             199	//279
#define IRQ_IPC_C2A_BINTC       279	//BMIRQ55
#define IRQ_GPIO                (2 + IRQ_APCP_0)	/* GPIO */
#define NUM_APCP_IRQS		32

/* Start virtual GPIO range above this max irq number */
#define IRQ_GPIO_0			300

#define NUM_GPIO_IRQS			192

#define  gpio_to_irq(gpio)		((gpio) + IRQ_GPIO_0 )
#define  irq_to_gpio(irq)		((irq) - IRQ_GPIO_0 )

#ifdef CONFIG_GPIO_PCA953X
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
#define EXPANDED_GPIO_IRQS    32	/*2 expander on RheaRayEDN1x, 32 irqs expanded */
#else
#define EXPANDED_GPIO_IRQS    16
#endif
#define NR_IRQS               (IRQ_GPIO_0+NUM_GPIO_IRQS + EXPANDED_GPIO_IRQS)
#else
#define NR_IRQS               (IRQ_GPIO_0 + NUM_GPIO_IRQS)
#endif

#define IRQ_LOCALTIMER 	BCM_INT_ID_PPI13

#endif /* __PLAT_KONA_IRQS_H */
