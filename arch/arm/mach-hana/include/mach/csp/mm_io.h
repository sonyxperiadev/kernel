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
*  @file    mm_io.h
*
*  @brief   Memory Map I/O definitions (Virtual)
*
*  @note
*     None
*/
/****************************************************************************/

#ifndef _MM_IO_H
#define _MM_IO_H

/* ---- Include Files ---------------------------------------------------- */
#ifdef __KERNEL__
#include <asm/memory.h>
#include <mach/vmalloc.h>
#endif
#include <mach/csp/mm_addr.h>

/* ---- Public Constants and Types --------------------------------------- */

#if defined( CONFIG_MMU )  /* uboot defines __KERNEL__ and CONFIG_MMU also so remove defined(__KERNEL__) from here */

#ifndef IO_START_PA
    #define IO_START_PA (IO_BASE_ADDR)
#endif
#ifndef IO_START_VA
    #define IO_START_VA (VMALLOC_END) /* 512MB io space */
#endif

    /* Physical to Virtual address conversion */

    #ifndef MM_IO_PHYS_TO_VIRT
    #define MM_IO_PHYS_TO_VIRT(phys)       ((phys) - IO_START_PA + IO_START_VA)
    #endif

    /* Virtual to Physical address conversion. This macro is referenced in <mach/io.h> */

    #ifndef MM_IO_VIRT_TO_PHYS
    #define MM_IO_VIRT_TO_PHYS(virt)       ((virt) - IO_START_VA + IO_START_PA)
    #endif

    /* TODO:
    *    The placement of VPM shared memory in the virtual address space makes
    *    assumptions on the location of I/O and user virtual address space.
    *    Perhaps add some dynamic calculation here if the assumed locations
    *    get changed (dependent on IO_BASE_ADDR and PAGE_OFFSET).  For now,
    *    generate a compiler error message
    */
    #if ((IO_BASE_ADDR != 0x34000000) || (PAGE_OFFSET != 0xc0000000))
    #error Verify that the VPM shared memory resides at the appropriate virtual address
    #endif

    #ifndef MM_IO_VPM_SHARED_MEM_TO_VIRT
    /* Put VPM shared memory virtual address at 0xf8000000 to prevent it
    *  from overlapping I/O (register) and user space
    */
    #define MM_IO_VPM_SHARED_MEM_TO_VIRT(phys) \
                                           ((phys) + (0xf8000000 - MM_ADDR_IO_VPM_EXTMEM_RSVD))
    #endif

    #ifndef MM_IO_VIRT_TO_VPM_SHARED_MEM
    #define MM_IO_VIRT_TO_VPM_SHARED_MEM(virt) \
                                           ((virt) - (0xf8000000 - MM_ADDR_IO_VPM_EXTMEM_RSVD))
    #endif

#else

    #ifndef MM_IO_PHYS_TO_VIRT
    #define MM_IO_PHYS_TO_VIRT(phys)       (phys)
    #endif

    #ifndef MM_IO_VIRT_TO_PHYS
    #define MM_IO_VIRT_TO_PHYS(virt)       (virt)
    #endif

    #ifndef MM_IO_VPM_SHARED_MEM_TO_VIRT
    #define MM_IO_VPM_SHARED_MEM_TO_VIRT(phys) \
                                           (phys)
    #endif

    #ifndef MM_IO_VIRT_TO_VPM_SHARED_MEM
    #define MM_IO_VIRT_TO_VPM_SHARED_MEM(virt) \
                                           (virt)
    #endif

#endif

/****************************************************************************
* NOTE: The following are alphabetically ordered. Please keep them that way.
*****************************************************************************/

#define MM_IO_BASE                     MM_IO_PHYS_TO_VIRT( MM_IO_START )                /* Virtual beginning of IO mapped memory */

#define MM_IO_BASE_ACI                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ACI )
#define MM_IO_BASE_AUXMIC              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_AUXMIC )
#define MM_IO_BASE_AON_CLK             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_AON_CLK )
#define MM_IO_BASE_AON_RST             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_AON_RST )
#define MM_IO_BASE_PMU_BSC             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PMU_BSC )
#define MM_IO_BASE_BSC1                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_BSC1 )
#define MM_IO_BASE_BSC2                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_BSC2 )
#define MM_IO_BASE_CHIPREG             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_CHIPREGS )        /* CHIPREG Block */
#define MM_IO_BASE_PAD_CTRL            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PAD_CTRL )        /* PAD_CTRL Block */
#define MM_IO_BASE_D1W                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_D1W )             /* Dallas 1-wire interface */
#define MM_IO_BASE_DMAC_NS             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_DMAC_NS )         /* Non-Secure DMA interface */
#define MM_IO_BASE_DMAC_S              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_DMAC_S )          /* Secure DMA interface */
#define MM_IO_BASE_DMUX                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_DMUX )            /* DMA DMUX */
#define MM_IO_BASE_EDMA                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_EDMA )            /* ESUB DMA */
#define MM_IO_BASE_ESW                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ESW )             /* Ethernet Switch */
#define MM_IO_BASE_GICCPU              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_GICCPU )          /* GIC CPU INTERFACE */
#define MM_IO_BASE_GICDIST             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_GICDIST )         /* GIC DISTRIBUTOR INTERFACE */
#define MM_IO_BASE_GPIO2               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_GPIO2 )           /* GPIO 2 */
#define MM_IO_BASE_HSI                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_HSI )             /* HSI */
#define MM_IO_BASE_HUB_CLK             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_HUB_CLK )
#define MM_IO_BASE_HUB_SWITCH          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_HUB_SWITCH )      /* HUB switch */
#define MM_IO_BASE_IPC_NS              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_IPC_NS )          /* Non-Secure IPC */
#define MM_IO_BASE_IPC_S               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_IPC_S )           /* Secure IPC */
#define MM_IO_BASE_KEK                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_KEK )
#define MM_IO_BASE_KEYPAD              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_KEYPAD )
#define MM_IO_BASE_KPM_CLK             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_KPM_CLK )         /* Kona Peripheral Master Clock Manager */
#define MM_IO_BASE_KPM_RST             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_KPM_RST )         /* Kona Peripheral Master Reset Manager */
#define MM_IO_BASE_L2C                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_L2C )             /* L2 Cache Controller */
#define MM_IO_BASE_MEMC0_APHY          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_APHY )      /* System Memory Controller APHY */
#define MM_IO_BASE_MEMC0_DPHY          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_DPHY )      /* System Memory Controller DPHY */
#define MM_IO_BASE_MEMC0_NS            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_NS )        /* Non-Secure System Memory Controller */
#define MM_IO_BASE_MEMC0_S             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_S )         /* Secure System Memory Controller */
#define MM_IO_BASE_MEMC0_DDR3_CTL      MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_DDR3_CTL )  /* DDR3 System Memory Controller */
#define MM_IO_BASE_MEMC0_DDR3_PHY      MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_DDR3_PHY )  /* DDR3 System PHY Controller */
#define MM_IO_BASE_MEMC0_DDR3_PHY_WL_0 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_DDR3_PHY_WL_0 )  /* DDR3 System PHY WordLane 0 */
#define MM_IO_BASE_MEMC0_DDR3_PHY_WL_1 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC0_DDR3_PHY_WL_1 )  /* DDR3 System PHY WordLane 1 */
#define MM_IO_BASE_MEMC1_APHY          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC1_APHY )      /* Videocore Memory Controller APHY */
#define MM_IO_BASE_MEMC1_DPHY          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC1_DPHY )      /* Videocore Memory Controller DPHY */
#define MM_IO_BASE_MEMC1_NS            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC1_NS )        /* Non-Secure Videocore Memory Controller */
#define MM_IO_BASE_MEMC1_S             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MEMC1_S )         /* Secure Videocore Memory Controller */
#define MM_IO_BASE_MPHI                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MPHI )
#define MM_IO_BASE_MPU                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_MPU )             /* Memory protection unit */
#define MM_IO_BASE_NAND                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_NAND )            /* NAND controller */
#define MM_IO_BASE_NVSRAM              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_NVSRAM )          /* NVSRAM controller */
#define MM_IO_BASE_OTP                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_OTP )             /* OTP */
#define MM_IO_BASE_PKA                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PKA )
#define MM_IO_BASE_PROC_CLK            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PROC_CLK )        /* Cortex A9 processor clock */
#define MM_IO_BASE_PROFTMR             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PROFTMR )         /* PROFILE TIMER */
#define MM_IO_BASE_PTIM                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PTIM )            /* Private timer and watchdog */
#define MM_IO_BASE_ROOT_CLK            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ROOT_CLK )
#define MM_IO_BASE_ROOT_RST            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ROOT_RST )
#define MM_IO_BASE_PWM                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_PWM )             /* PWM */
#define MM_IO_BASE_RNG                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_RNG )             /* RNG */
#define MM_IO_BASE_SCU                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SCU )             /* SCU */
#define MM_IO_BASE_SDIO1               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SDIO1 )           /* SDIO 1 */
#define MM_IO_BASE_SDIO2               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SDIO2 )           /* SDIO 2 */
#define MM_IO_BASE_SDIO3               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SDIO3 )           /* SDIO 3 */
#define MM_IO_BASE_SDIO4               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SDIO4 )           /* SDIO 4 */
#define MM_IO_BASE_SEC                 MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SEC )
#define MM_IO_BASE_SEC_WATCHDOG        MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SEC_WATCHDOG )    /* Watchdog Timer in security block
                                                                                        *  (not to be confused with MM_IO_BASE_SECWD) */
#define MM_IO_BASE_SECTRAP1            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP1 )
#define MM_IO_BASE_SECTRAP2            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP2 )
#define MM_IO_BASE_SECTRAP3            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP3 )
#define MM_IO_BASE_SECTRAP4            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP4 )
#define MM_IO_BASE_SECTRAP5            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP5 )
#define MM_IO_BASE_SECTRAP6            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP6 )
#define MM_IO_BASE_SECTRAP7            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP7 )
#define MM_IO_BASE_SECTRAP8            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECTRAP8 )
#define MM_IO_BASE_SECWD               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SECWD )           /* Secure WD Timer
                                                                                        *  (not to be confused with MM_IO_BASE_SEC_WATCHDOG) */
#define MM_IO_BASE_SIMI                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SIMI )            /* SIM interface */
#define MM_IO_BASE_SIMI2               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SIMI2 )           /* SIM interface */
#define MM_IO_BASE_SLV_CLK             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SLV_CLK )         /* Kona Peripheral Slave Clock Manager */
#define MM_IO_BASE_SLV_RST             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SLV_RST )         /* Kona Peripheral Slave Reset Manager */
#define MM_IO_BASE_SPUM_NS             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SPUM_NS )         /* Non-Secure AXI interface */
#define MM_IO_BASE_SPUM_S              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SPUM_S )          /* Secure AXI interface */
#define MM_IO_BASE_SPUM_APB_NS         MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SPUM_APB_NS )     /* Non-Secure APB interface */
#define MM_IO_BASE_SPUM_APB_S          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SPUM_APB_S )      /* Secure APB interface */
#define MM_IO_BASE_SRAM                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SRAM )            /* INTERNAL SRAM (160kB) */

#define MM_IO_BASE_SSP0                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SSP0 )
#define MM_IO_BASE_SSP1                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SSP1 )      
#define MM_IO_BASE_SSP2                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SSP2 )
#define MM_IO_BASE_SSP3                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SSP3 )
#define MM_IO_BASE_SSP4                MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SSP4 )

#define MM_IO_BASE_SYSTEM_SWITCH       MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SYSTEM_SWITCH )   /* System switch  */
#define MM_IO_BASE_SYSTMR              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_SYSTMR )          /* SYSTEM TIMER */
#define MM_IO_BASE_TMR_HUB             MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_TMR_HUB )         /* Hub timer */
#define MM_IO_BASE_TZCFG               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_TZCFG )
#define MM_IO_BASE_UART0               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_UART0 )           /* UART 0 */
#define MM_IO_BASE_UART1               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_UART1 )           /* UART 1 */
#define MM_IO_BASE_UART2               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_UART2 )           /* UART 2 */
#define MM_IO_BASE_UART3               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_UART3 )           /* UART 3 */
#define MM_IO_BASE_USB_FSHOST          MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_FSHOST )      /* USB FSHOST */
#define MM_IO_BASE_USB_FSHOST_CTRL     MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_FSHOST_CTRL ) /* USB FSHOST Control */
#define MM_IO_BASE_USB_HOST_CTRL       MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_HOST_CTRL )   /* USB HOST Control */
#define MM_IO_BASE_USB_HOST_EHCI       MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_HOST_EHCI )   /* USB HOST EHCI */
#define MM_IO_BASE_USB_HOST_OHCI       MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_HOST_OHCI )   /* USB HOST OHCI */
#define MM_IO_BASE_USB_HSOTG           MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_HSOTG )       /* USB OTG */
#define MM_IO_BASE_USB_HSOTG_CTRL      MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_USB_HSOTG_CTRL )  /* USB OTG Control */
#define MM_IO_BASE_VINTC               MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_VINTC )           /* VPM interrupt control */
#define MM_IO_BASE_VC_EMI              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_VC_EMI )          /* VC4 EMI */
#define MM_IO_BASE_VPM_DATA            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_VPM_DATA )        /* VPM data space */
#define MM_IO_BASE_VPM_EXTMEM_RSVD     MM_IO_VPM_SHARED_MEM_TO_VIRT( MM_ADDR_IO_VPM_EXTMEM_RSVD )
#define MM_IO_BASE_VPM_PROG            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_VPM_PROG )        /* VPM program space */
#define MM_IO_BASE_VRAM                MM_IO_VPM_SHARED_MEM_TO_VIRT( MM_ADDR_IO_VRAM )
#define MM_IO_BASE_AUDIOH              MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_AUDIOH )
#define MM_IO_BASE_ESUB_CLK            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ESUB_CLK )        /* ESUB Clock Manager */
#define MM_IO_BASE_ESUB_RST            MM_IO_PHYS_TO_VIRT( MM_ADDR_IO_ESUB_RST )        /* ESUB Reset Manager */

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */


#endif /* _MM_IO_H */
