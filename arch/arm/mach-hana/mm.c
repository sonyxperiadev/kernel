/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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



#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/mach/map.h>
#include <asm/pgalloc.h>

#include <mach/hardware.h>
#include <mach/csp/mm_io.h>
#include <mach/io_map.h>

#define IO_DESC(va, sz) { .virtual = va, \
                          .pfn = __phys_to_pfn(HW_IO_VIRT_TO_PHYS(va)), \
                          .length = sz, \
                          .type = MT_DEVICE }

static struct map_desc hana_io_desc[] __initdata =
{
	IO_DESC( KONA_ACI_VA, SZ_4K  ),
        IO_DESC( KONA_BINTC_BASE_ADDR, SZ_4K  ),
        IO_DESC( KONA_BSC1_VA, SZ_4K  ),
        IO_DESC( KONA_BSC2_VA, SZ_4K  ),
        IO_DESC( KONA_CHIPREG_VA, SZ_4K ),
        IO_DESC( KONA_D1W_VA, SZ_4K ),
        IO_DESC( KONA_DMAC_NS_VA, SZ_4K ),
        IO_DESC( KONA_DMAC_S_VA, SZ_4K  ),
        IO_DESC( KONA_DMUX_VA, SZ_4K  ),
        IO_DESC( KONA_EDMA_VA, SZ_4K ),
        IO_DESC( KONA_ESW_VA, SZ_1M ),
        IO_DESC( KONA_GICDIST_VA, SZ_4K ),
        IO_DESC( KONA_GPIO2_VA, SZ_4K ),
        IO_DESC( KONA_HSI_VA, SZ_4K ),
        IO_DESC( KONA_IPC_NS_VA, SZ_4K ),
        IO_DESC( KONA_IPC_S_VA, SZ_4K ),
        IO_DESC( KONA_KEK_VA, SZ_4K ),
        IO_DESC( KONA_KPM_CLK_VA, SZ_4K ),
        IO_DESC( KONA_SLV_CLK_VA, SZ_4K ),
        IO_DESC( KONA_L2C_VA, SZ_4K ),
        IO_DESC( KONA_MPHI_VA, SZ_4K ),
        IO_DESC( KONA_NAND_VA, SZ_64K ),
        IO_DESC( KONA_MPU_VA, SZ_4K ),
        IO_DESC( KONA_OTP_VA, SZ_4K ),
        IO_DESC( KONA_PKA_VA, SZ_4K ),
        IO_DESC( KONA_PWM_VA, SZ_4K ),
        IO_DESC( KONA_PMU_BSC_VA, SZ_4K ),
        IO_DESC( KONA_ROOT_RST_VA, SZ_4K ),
        IO_DESC( KONA_RNG_VA, SZ_4K ),

	/*
	 * This SCU region also covers MM_ADDR_IO_GICCPU,
	 * KONA_PROFTMR aka (GTIM) aka (GLB) aka knllog timer,
	 * and KONA_PTIM aka os tick timer
	 */
        IO_DESC( KONA_SCU_VA, SZ_4K ),
        IO_DESC( KONA_SDIO1_VA, SZ_64K ),
        IO_DESC( KONA_SDIO2_VA, SZ_64K ),
        IO_DESC( KONA_SDIO3_VA, SZ_64K ),
        IO_DESC( KONA_SDIO4_VA, SZ_64K ),
        IO_DESC( KONA_SEC_VA, SZ_4K ),
        IO_DESC( KONA_SECWD_VA, SZ_4K ),
        IO_DESC( KONA_SPUM_NS_VA, SZ_64K ),
        IO_DESC( KONA_SPUM_S_VA, SZ_64K ),
        IO_DESC( KONA_SPUM_APB_NS_VA, SZ_4K ),
        IO_DESC( KONA_SPUM_APB_S_VA, SZ_4K ),
        IO_DESC( KONA_SRAM_VA, SZ_256K ),
	IO_DESC( KONA_KPS_CLK_VA, SZ_4K ),
        IO_DESC( KONA_SSP0_VA, SZ_4K ),
        IO_DESC( KONA_SSP2_VA, SZ_4K ),
        IO_DESC( KONA_SSP3_VA, SZ_4K ),
        IO_DESC( KONA_SSP4_VA, SZ_4K ),

        IO_DESC( KONA_SYSTMR_VA, SZ_4K ),  /* 32-bit kona gp timer */
        IO_DESC( KONA_TMR_HUB_VA, SZ_4K ), /* 64-bit hub timer */
        IO_DESC( KONA_TZCFG_VA, SZ_4K ),
        IO_DESC( KONA_UART0_VA, SZ_4K ),
        IO_DESC( KONA_UART1_VA, SZ_4K ),
        IO_DESC( KONA_UART2_VA, SZ_4K ),
        IO_DESC( KONA_UART3_VA, SZ_4K ),
        IO_DESC( KONA_USB_FSHOST_CTRL_VA, SZ_4K ),
        IO_DESC( KONA_USB_HOST_CTRL_VA, SZ_256 ),       /* Could really use SZ_32 if was def'd. Note: 256 not 256K */
        IO_DESC( KONA_USB_HOST_EHCI_VA, SZ_256 ),       /* Includes DWC specific registers, otherwise could use SZ_128 if was def'd */
        IO_DESC( KONA_USB_HOST_OHCI_VA, SZ_256 ),       /* Could really use SZ_128 if was def'd */
        IO_DESC( KONA_USB_HSOTG_CTRL_VA, SZ_4K ),

        /* add for CAPH*/
        IO_DESC( KONA_HUB_CLK_BASE_VA, SZ_4K ),
        IO_DESC( KONA_AUDIOH_BASE_VA, SZ_32K ),
        IO_DESC( KONA_SDT_BASE_VA, SZ_4K ),
        IO_DESC( KONA_SSP4_BASE_VA, SZ_4K ),
        IO_DESC( KONA_SSP3_BASE_VA, SZ_4K ),
        IO_DESC( KONA_SRCMIXER_BASE_VA, SZ_4K ),
        IO_DESC( KONA_CFIFO_BASE_VA, SZ_4K ),
        IO_DESC( KONA_AADMAC_BASE_VA, SZ_4K ),
        IO_DESC( KONA_SSASW_BASE_VA, SZ_4K ),
        IO_DESC( KONA_AHINTC_BASE_VA, SZ_4K ),
};

void __init hana_map_io( void )
{
	iotable_init( hana_io_desc, ARRAY_SIZE( hana_io_desc ));

#if defined( CONFIG_BCM_EARLY_PRINTK )
	{
		/*
		 * Now that the I/O space has been setup, we can install our
		 * "early" console device.
		 */

		extern void bcm_install_early_console( void );

		/*
		 * We need to call flush_tlb_all before printing will actually work.
		 * (I'm not sure exactly why). This is done by paging_init
		 * after map_io is called, but because we're in map_io we need to do
		 * it if we're going to get already buffered stuff to be printed.
		 */

		flush_tlb_all();

		/*
		 * Install a console, so that we can get early printk's to work before
		 * interrupts are enabled.
		 */

		bcm_install_early_console();
	}
#endif
}

