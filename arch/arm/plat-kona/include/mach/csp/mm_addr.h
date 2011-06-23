/*****************************************************************************
* Copyright 2003 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/



/****************************************************************************/
/**
*  @file    mm_addr.h
*
*  @brief   Memory Map address defintions (Physical)
*
*  @note
*     None
*/
/****************************************************************************/
#ifndef _MM_ADDR_H
#define _MM_ADDR_H

/* ---- Include Files ---------------------------------------------------- */

#include <cfg_global.h>
#include <mach/csp/chal_regmap.h>

/* ---- Public Constants and Types --------------------------------------- */

#ifdef CONFIG_CSP_RUN_FROM_SSRAM

   #define VPM_EXTMEM_SIZE                      0x00000000  /* 0MB*/
   #define MM_ADDR_IO_VPM_EXTMEM_BASE           0x00000000  /* an invalid address*/
   #define MM_ADDR_IO_VPM_EXTMEM_RSVD           0x00000000  /* an invalid address*/
   /*#warning VPM tests not supported if csptest is configured to run from SRAM*/
   #define MM_ADDR_IO_IS_VPM_SHARED_DDR(phys)   0

#else

   #if CFG_GLOBAL_CSP_VPM_RUN_FROM_SSRAM

      #define VRAM_SIZE                         0x00004800  /* 18kb: 16kb for 2 simplex channels, 2kb for others*/
      #define VPM_EXTMEM_SIZE                   ( CFG_GLOBAL_SSRAM_SIZE - VRAM_SIZE ) /* 0x23800*/
      #define MM_ADDR_IO_VPM_EXTMEM_BASE        CFG_GLOBAL_SSRAM_BASE
      #define MM_ADDR_IO_VRAM                   ( MM_ADDR_IO_VPM_EXTMEM_BASE + VPM_EXTMEM_SIZE ) /* 0x34063800*/
      #define MM_ADDR_IO_IS_VPM_SHARED_DDR(phys) \
                                                0

   #else /* DDR */

      #define VRAM_SIZE                         0x00010000  /* 64kb*/
      #define VPM_RSVP_MEM_SIZE                 ( CFG_GLOBAL_RAM_START_RESERVED_SIZE )  /* 2MB*/
      #define VPM_EXTMEM_SIZE                   ( VPM_RSVP_MEM_SIZE - VRAM_SIZE ) /* 0x1F0000*/
      #define MM_ADDR_IO_VPM_EXTMEM_BASE        ( CFG_GLOBAL_RAM_BASE ) /* 0x80000000*/
      #define MM_ADDR_IO_VRAM                   ( MM_ADDR_IO_VPM_EXTMEM_BASE + VPM_EXTMEM_SIZE ) /* 0x801F0000*/
      #define MM_ADDR_IO_IS_VPM_SHARED_DDR(phys) \
                                                (((phys) >= (MM_ADDR_IO_VPM_EXTMEM_RSVD)) && \
                                                 ((phys) <  (MM_ADDR_IO_VPM_EXTMEM_RSVD + VPM_RSVP_MEM_SIZE)))

   #endif /* CFG_GLOBAL_CSP_VPM_RUN_FROM_SSRAM */

   #if !defined( CSP_SIMULATION )
      #define MM_ADDR_IO_VPM_EXTMEM_RSVD        MM_ADDR_IO_VPM_EXTMEM_BASE
   #endif

#endif /* CONFIG_CSP_RUN_FROM_SSRAM */

#define MM_IO_START                 IO_BASE_ADDR                  /* Physical beginning of IO mapped memory */

/****************************************************************************
* NOTE: The following are alphabetically ordered. Please keep them that way.
*****************************************************************************/

#define MM_ADDR_IO_AADMAC           AADMAC_BASE_ADDR              /* CAPH AADMAC interface */
#define MM_ADDR_IO_ACI              ACI_BASE_ADDR                 /* Accessory Component Interface */
#define MM_ADDR_IO_AHINTC           AHINTC_BASE_ADDR              /* CAPH Interrupt Interface */
#define MM_ADDR_IO_AUXMIC           AUXMIC_BASE_ADDR              /* Auxilary Microphone */
#define MM_ADDR_IO_AON_CLK          AON_CLK_BASE_ADDR
#define MM_ADDR_IO_AON_RST          AON_RST_BASE_ADDR
#define MM_ADDR_IO_AUDIOH           AUDIOH_BASE_ADDR              /* Audio Hub */
#define MM_ADDR_IO_BBL              BBL_BASE_ADDR
#define MM_ADDR_IO_BINTC            BINTC_BASE_ADDR
#define MM_ADDR_IO_BMODEM_SYSCFG    BMODEM_SYSCFG_BASE_ADDR

#define MM_ADDR_IO_BSC1             BSC1_BASE_ADDR                /* BSC I2C 1 interface  */
#define MM_ADDR_IO_BSC2             BSC2_BASE_ADDR                /* BSC I2C 2 interface  */
#define MM_ADDR_IO_CFIFO            CFIFO_BASE_ADDR               /* CAPH CFIFO interface */
#define MM_ADDR_IO_CHIPREGS         CHIPREGS_BASE_ADDR            /* CHIPREG Block */
#define MM_ADDR_IO_CIR              CIR_BASE_ADDR                 /* CIR Base Address */
#define MM_ADDR_IO_PAD_CTRL         PAD_CTRL_BASE_ADDR            /* CHIPREG Block */
#define MM_ADDR_IO_D1W              D1W_BASE_ADDR                 /* Dallas 1-wire interface */
#define MM_ADDR_IO_DMAC_NS          NONDMAC_BASE_ADDR             /* Non-Secure DMA interface */
#define MM_ADDR_IO_DMAC_S           SECDMAC_BASE_ADDR             /* Secure DMA interface */
#define MM_ADDR_IO_DMUX             DMUX_BASE_ADDR                /* DMA DMUX */
#define MM_ADDR_IO_DTE              EAV_DTE_BASE_ADDR             /* DTE sub system */
#define MM_ADDR_IO_EDMA             DWDMA_AHB_BASE_ADDR           /* ESUB DMA */
#define MM_ADDR_IO_ESUB_CLK         ESUB_CLK_BASE_ADDR            /* ESUB Clock Manager */
#define MM_ADDR_IO_ESUB_RST         ESUB_RST_BASE_ADDR            /* ESUB Reset Manager */
#define MM_ADDR_IO_ESW              ESW_CONTRL_BASE_ADDR          /* Ethernet Switch */
#define MM_ADDR_IO_FMON             FMON_BASE_ADDR                /* FMON */
#define MM_ADDR_IO_GICCPU           GICCPU_BASE_ADDR              /* GIC CPU INTERFACE */
#define MM_ADDR_IO_GICDIST          GICDIST_BASE_ADDR             /* GIC DISTRIBUTOR INTERFACE */
#define MM_ADDR_IO_GPIO2            GPIO2_BASE_ADDR               /* GPIO 2 */
#define MM_ADDR_IO_HSI              HSI_BASE_ADDR                 /* HSI */
#define MM_ADDR_IO_HUB_CLK          HUB_CLK_BASE_ADDR
#define MM_ADDR_IO_HUB_SWITCH       HUBSWITCH_BASE_ADDR           /* HUB switch */
#define MM_ADDR_IO_IPC_NS           IPC_BASE_ADDR                 /* Non-Secure IPC */
#define MM_ADDR_IO_IPC_S            IPCSEC_BASE_ADDR              /* Secure IPC */
#define MM_ADDR_IO_KEK              SEC_KEK_BASE_ADDR
#define MM_ADDR_IO_KEYPAD           KEYPAD_BASE_ADDR
#define MM_ADDR_IO_KPM_CLK          KONA_MST_CLK_BASE_ADDR        /* Kona Peripheral Master Clock Manager */
#define MM_ADDR_IO_KPM_RST          KONA_MST_RST_BASE_ADDR        /* Kona Peripheral Master Reset Manager */
#define MM_ADDR_IO_L2C              L2C_BASE_ADDR                 /* L2 Cache Controller */
#define MM_ADDR_IO_LEDM             LEDM_BASE_ADDR                /* LED Matrix */
#define MM_ADDR_IO_MEMC_SYS_EMI_APHY            SYS_EMI_DDR3_CTL_BASE_ADDR             /* System Memory Controller APHY */
#define MM_ADDR_IO_MEMC_SYS_EMI_DPHY            SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR    /* System Memory Controller DPHY */
#define MM_ADDR_IO_MEMC_SYS_EMI_NS              SYS_EMI_OPEN_BASE_ADDR                 /* Non-Secure System Memory Controller */
#define MM_ADDR_IO_MEMC_SYS_EMI_S               SYS_EMI_SECURE_BASE_ADDR               /* Secure System Memory Controller */
#define MM_ADDR_IO_MEMC_SYS_EMI_DDR3_CTL        SYS_EMI_DDR3_CTL_BASE_ADDR             /* DDR3 System Memory Controller */
#define MM_ADDR_IO_MEMC_SYS_EMI_DDR3_PHY        SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR    /* DDR3 System PHY Controller */
#define MM_ADDR_IO_MEMC_SYS_EMI_DDR3_PHY_WL_0   SYS_EMI_DDR3_PHY_WL_0_BASE_ADDR        /* DDR3 System PHY WordLane 0 */
#define MM_ADDR_IO_MEMC_SYS_EMI_DDR3_PHY_WL_1   SYS_EMI_DDR3_PHY_WL_1_BASE_ADDR        /* DDR3 System PHY WordLane 1 */
#define MM_ADDR_IO_MEMC_VC4_EMI_APHY            VC4_EMI_DDR3_CTL_BASE_ADDR             /* Videocore Memory Controller APHY */
#define MM_ADDR_IO_MEMC_VC4_EMI_DPHY            VC4_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR    /* Videocore Memory Controller DPHY */
#define MM_ADDR_IO_MEMC_VC4_EMI_NS              VC4_EMI_OPEN_BASE_ADDR                 /* Non-Secure Videocore Memory Controller */
#define MM_ADDR_IO_MEMC_VC4_EMI_S               VC4_EMI_SECURE_BASE_ADDR               /* Secure Videocore Memory Controller */
#define MM_ADDR_IO_MPHI             MPHI_BASE_ADDR                /* MPHI Interface */
#define MM_ADDR_IO_MPU              MPU_BASE_ADDR                 /* Memory protection unit */
#define MM_ADDR_IO_NAND             NAND_BASE_ADDR                /* NAND Controller */
#define MM_ADDR_IO_NVSRAM           NVSRAM_BASE_ADDR              /* NVSRAM Controller */
#define MM_ADDR_IO_OTP              SEC_OTP_BASE_ADDR             /* Secure OTP */
#define MM_ADDR_IO_PKA              SEC_PKA_BASE_ADDR             /* Secure PKA */
#define MM_ADDR_IO_PMU_BSC          PMU_BSC_BASE_ADDR             /* PMU BSC interface */
#define MM_ADDR_IO_PROC_CLK         PROC_CLK_BASE_ADDR            /* Cortex A9 Processor Clock Manager */
#define MM_ADDR_IO_PROFTMR          GTIM_BASE_ADDR                /* Global timer for system profiling */
#define MM_ADDR_IO_PTIM             PTIM_BASE_ADDR                /* Private timer and watchdog */
#define MM_ADDR_IO_PWM              PWM_BASE_ADDR                 /* PWM */
#define MM_ADDR_IO_PWRMGR           PWRMGR_BASE_ADDR              /* PWRMGR */
#define MM_ADDR_IO_ROOT_CLK         ROOT_CLK_BASE_ADDR
#define MM_ADDR_IO_ROOT_RST         ROOT_RST_BASE_ADDR
#define MM_ADDR_IO_RNG              SEC_RNG_BASE_ADDR             /* Secure RNG */
#define MM_ADDR_IO_RTC              RTC_BASE_ADDR                 /* RTC */
#define MM_ADDR_IO_RTC_APB          RTC_APB_BASE_ADDR             /* RTC APB*/
#define MM_ADDR_IO_SCU              SCU_BASE_ADDR                 /* SCU */
#define MM_ADDR_IO_SDT              SDT_BASE_ADDR                 /* Sidetone */
#define MM_ADDR_IO_SDIO1            SDIO1_BASE_ADDR               /* SDIO 1 */
#define MM_ADDR_IO_SDIO2            SDIO2_BASE_ADDR               /* SDIO 2 */
#define MM_ADDR_IO_SDIO3            SDIO3_BASE_ADDR               /* SDIO 3 */
#define MM_ADDR_IO_SDIO4            SDIO4_BASE_ADDR               /* SDIO 4 */
#define MM_ADDR_IO_SEC              SEC_CFG_BASE_ADDR
#define MM_ADDR_IO_SEC_WATCHDOG     SEC_WATCHDOG_BASE_ADDR        /* Watchdog Timer in security block
                                                                  *  (not to be confused with MM_ADDR_IO_SECWD) */
#define MM_ADDR_IO_SECTRAP1         SECTRAP1_BASE_ADDR
#define MM_ADDR_IO_SECTRAP2         SECTRAP2_BASE_ADDR
#define MM_ADDR_IO_SECTRAP3         SECTRAP3_BASE_ADDR
#define MM_ADDR_IO_SECTRAP4         SECTRAP4_BASE_ADDR
#define MM_ADDR_IO_SECTRAP5         SECTRAP5_BASE_ADDR
#define MM_ADDR_IO_SECTRAP6         SECTRAP6_BASE_ADDR
#define MM_ADDR_IO_SECTRAP7         SECTRAP7_BASE_ADDR
#define MM_ADDR_IO_SECTRAP8         SECTRAP8_BASE_ADDR
#define MM_ADDR_IO_SECWD            SECWD_BASE_ADDR               /* Secure WD Timer
                                                                  *  (not to be confused with MM_ADDR_IO_SEC_WATCHDOG) */
#define MM_ADDR_IO_SIMI             SIM_BASE_ADDR                 /* SIM interface */
#define MM_ADDR_IO_SIMI2            SIM2_BASE_ADDR                /* SIM interface */
#define MM_ADDR_IO_SLV_CLK          KONA_SLV_CLK_BASE_ADDR        /* Kona Peripheral Slave Clock Manager */
#define MM_ADDR_IO_SLV_RST          KONA_SLV_RST_BASE_ADDR        /* Kona Peripheral Slave Reset Manager */
#define MM_ADDR_IO_SPUM_APB_NS      SEC_SPUM_NS_APB_BASE_ADDR     /* Non-Secure APB interface */
#define MM_ADDR_IO_SPUM_APB_S       SEC_SPUM_S_APB_BASE_ADDR      /* Secure APB interface */
#define MM_ADDR_IO_SPUM_NS          SPUM_NS_BASE_ADDR             /* Non-Secure AXI interface */
#define MM_ADDR_IO_SPUM_S           SPUM_S_BASE_ADDR              /* Secure AXI interface */
#define MM_ADDR_IO_SRAM             0x34040000                    /* Internal SRAM (160 KB) */

#define MM_ADDR_IO_SRCMIXER         SRCMIXER_BASE_ADDR            /* CAPH SRCMIXER Interface */
#define MM_ADDR_IO_SSAW             SSASW_BASE_ADDR               /* CAPH SSASW Interface */

#define MM_ADDR_IO_SSP0             SSP0_BASE_ADDR
#define MM_ADDR_IO_SSP1             SSP1_BASE_ADDR
#define MM_ADDR_IO_SSP2             SSP2_BASE_ADDR
#define MM_ADDR_IO_SSP3             SSP3_BASE_ADDR
#define MM_ADDR_IO_SSP4             SSP4_BASE_ADDR
#define MM_ADDR_IO_SSP5             SSP5_BASE_ADDR
#define MM_ADDR_IO_SSP6             SSP6_BASE_ADDR

#define MM_ADDR_IO_SYSTEM_SWITCH    SYSSWITCH_BASE_ADDR           /* System switch */
#define MM_ADDR_IO_SYSTMR           TIMER_BASE_ADDR               /* SYSTEM TIMER */
#define MM_ADDR_IO_TL3R             AHB_DSP_TL3R_BASE_ADDR        /* Teaklite 3 base */    
#define MM_ADDR_IO_TMR_HUB          HUB_TIMER_BASE_ADDR           /* Hub timer */
#define MM_ADDR_IO_TZCFG            TZCFG_BASE_ADDR
#define MM_ADDR_IO_UART0            UARTB_BASE_ADDR               /* UART 0 */
#define MM_ADDR_IO_UART1            UARTB2_BASE_ADDR              /* UART 1 */
#define MM_ADDR_IO_UART2            UARTB3_BASE_ADDR              /* UART 2 */
#define MM_ADDR_IO_UART3            UARTB4_BASE_ADDR              /* UART 3 */
#define MM_ADDR_IO_USB_FSHOST       FSHOST_BASE_ADDR
#define MM_ADDR_IO_USB_FSHOST_CTRL  FSHOST_CTRL_BASE_ADDR
#define MM_ADDR_IO_USB_HOST_CTRL    (EHCI_BASE_ADDR + EHCI_MODE_OFFSET)
#define MM_ADDR_IO_USB_HOST_EHCI    EHCI_BASE_ADDR
#define MM_ADDR_IO_USB_HOST_OHCI    OHCI_BASE_ADDR
#define MM_ADDR_IO_USB_HSOTG        HSOTG_BASE_ADDR
#define MM_ADDR_IO_USB_HSOTG_CTRL   HSOTG_CTRL_BASE_ADDR
#if (CFG_GLOBAL_LITTLE_ISLAND_SUPPORT)
#define MM_ADDR_IO_VC_EMI           0x80000000                    /* In little island mode VC4 shares DDR3 with ARM */
#elif defined(CONFIG_BCMHANA_LI_MODE)
#define MM_ADDR_IO_VC_EMI           0x80000000                    /* Reserved DDR3 memory for VC4 (LI mode) */
#else
#define MM_ADDR_IO_VC_EMI           0x40000000                    /* VC4 EMI */
#endif
#define MM_ADDR_IO_VINTC            VINTC_APB_BASE_ADDR           /* VPM interrupt control */
#define MM_ADDR_IO_VPM_PROG         0x38800000                    /* VPM program space */
#define MM_ADDR_IO_VPM_DATA         0x38A00000                    /* VPM data space */
#define MM_ADDR_IO_WCDMAL2INT_ASYNC WCDMAL2INT_ASYNC_BASE_ADDR    /* for deep sleep control */

#define MM_ADDR_IO_BMDM_CLK         BMDM_CCU_BASE_ADDR            /* BMODEM CCU */
#define MM_ADDR_IO_BMDM_PWRMGR      BMDM_PWRMGR_BASE_ADDR         /* BMODEM power manager */

#endif /* _MM_ADDR_H */
