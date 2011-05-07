/*****************************************************************************
* Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
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
#include <linux/init.h>
#include <linux/serial_8250.h>

#include <asm/memory.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/irqs.h>
#include <mach/csp/mm_io.h>
#include <mach/csp/chal_ccu_kps_inline.h>
#include <mach/timer.h>
#include <mach/kona.h>
#include <mach/gpio.h>
#include <mach/clock.h>
#include <mach/pinmux.h>

#define UARTB_USR_OFFSET	0x0000007C

#define HANA_8250PORT(name) \
{ \
	.membase    = (void __iomem *)(MM_IO_BASE_##name), \
	.mapbase    = (resource_size_t)(MM_ADDR_IO_##name),    \
	.irq        = BCM_INT_ID_##name,               \
	.uartclk    = 29500000, \
	.regshift   = 2, \
	.iotype     = UPIO_DWAPB, \
	.type       = PORT_16550A,          \
	.flags      = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST, \
	.private_data = (void __iomem *)((MM_IO_BASE_##name) + UARTB_USR_OFFSET), \
}

static struct plat_serial8250_port hana_uart_data[] = {
	HANA_8250PORT(UART0),
	HANA_8250PORT(UART1),
	HANA_8250PORT(UART2),
	HANA_8250PORT(UART3),
	{ .flags = 0 },
};

static struct platform_device hana_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data = hana_uart_data,
	},
};

static struct platform_device *hana_devices[] __initdata = {
	&hana_serial_device,
};

static void __init clock_tmp_init( void )
{
	unsigned int access;

	/* Change UART clock to support 921k baud rate speeds */
	/* enable access */
	access = chal_ccu_unlock_kps_clk_mgr();

	/* set divider for the 156Mhz  source clock to get a 29.5Mhz UART clock */
	chal_ccu_set_kps_uartb_div( chal_ccu_kps_uart_div_div, 0x448 );

	/* trigger */
	chal_ccu_set_kps_div_trig( chal_ccu_kps_div_trig_uartb_trig, 1 );

	/* wait until complete */
	while( chal_ccu_get_kps_div_trig( chal_ccu_kps_div_trig_uartb_trig ) != 0 );

	/* restore access */
	chal_ccu_restore_kps_clk_mgr(access);

	access = chal_ccu_unlock_kps_clk_mgr();

	chal_ccu_cfg_kps_policy_mask(0, chal_ccu_kps_policy_mask_uartb3, (0x1 << chal_ccu_policy_0));
	chal_ccu_cfg_kps_policy_mask(1, chal_ccu_kps_policy_mask_uartb3, (0x1 << chal_ccu_policy_1));
	chal_ccu_cfg_kps_policy_mask(2, chal_ccu_kps_policy_mask_uartb3, (0x1 << chal_ccu_policy_2));
	chal_ccu_cfg_kps_policy_mask(3, chal_ccu_kps_policy_mask_uartb3, (0x1 << chal_ccu_policy_3));

	/* Set clock source and divider */
	chal_ccu_set_kps_uartb3_div( chal_ccu_kps_uart_div_div, 0x448 );

	/* Enable clocks */
	chal_ccu_set_kps_uartb3_clkgate(chal_ccu_kps_uart_clkgate_apb_hw_sw_gating_sel, 1);
	chal_ccu_set_kps_uartb3_clkgate(chal_ccu_kps_uart_clkgate_hw_sw_gating_sel, 1);
	chal_ccu_set_kps_uartb3_clkgate(chal_ccu_kps_uart_clkgate_apb_clk_en, 1);
	chal_ccu_set_kps_uartb3_clkgate(chal_ccu_kps_uart_clkgate_clk_en, 1);
	chal_ccu_set_kps_uartb3_clkgate(chal_ccu_kps_uart_clkgate_voltage_level, 0);

	/* trigger the clock */
	chal_ccu_set_kps_div_trig( chal_ccu_kps_div_trig_uartb3_trig, 1 );
	while( chal_ccu_get_kps_div_trig( chal_ccu_kps_div_trig_uartb3_trig ) != 0 );
	chal_ccu_restore_kps_clk_mgr(access);

	access = chal_ccu_unlock_kps_clk_mgr();

	chal_ccu_cfg_kps_policy_mask(0, chal_ccu_kps_policy_mask_uartb4, (0x1 << chal_ccu_policy_0));
	chal_ccu_cfg_kps_policy_mask(1, chal_ccu_kps_policy_mask_uartb4, (0x1 << chal_ccu_policy_1));
	chal_ccu_cfg_kps_policy_mask(2, chal_ccu_kps_policy_mask_uartb4, (0x1 << chal_ccu_policy_2));
	chal_ccu_cfg_kps_policy_mask(3, chal_ccu_kps_policy_mask_uartb4, (0x1 << chal_ccu_policy_3));

	/* Set clock source and divider */
	chal_ccu_set_kps_uartb4_div( chal_ccu_kps_uart_div_div, 0x448 );

	/* Enable clocks */
	chal_ccu_set_kps_uartb4_clkgate(chal_ccu_kps_uart_clkgate_apb_hw_sw_gating_sel, 1);
	chal_ccu_set_kps_uartb4_clkgate(chal_ccu_kps_uart_clkgate_hw_sw_gating_sel, 1);
	chal_ccu_set_kps_uartb4_clkgate(chal_ccu_kps_uart_clkgate_apb_clk_en, 1);
	chal_ccu_set_kps_uartb4_clkgate(chal_ccu_kps_uart_clkgate_clk_en, 1);
	chal_ccu_set_kps_uartb4_clkgate(chal_ccu_kps_uart_clkgate_voltage_level, 0);

	/* trigger the clock */
	chal_ccu_set_kps_div_trig( chal_ccu_kps_div_trig_uartb4_trig, 1 );
	while( chal_ccu_get_kps_div_trig( chal_ccu_kps_div_trig_uartb4_trig ) != 0 );
	chal_ccu_restore_kps_clk_mgr(access);

	/* Force timer clock source to be 1MHz */
	access = chal_ccu_unlock_kps_clk_mgr();
	chal_ccu_set_kps_timers_div(chal_ccu_kps_timers_div_pll_sel, 0);
	chal_ccu_set_kps_div_trig(chal_ccu_kps_div_trig_timers_trig, 1);
	while( chal_ccu_get_kps_div_trig(chal_ccu_kps_div_trig_timers_trig) );
	chal_ccu_restore_kps_clk_mgr(access);
}

void __init hana_init_machine( void )
{
#ifdef CONFIG_CACHE_L2X0
	{
		uint32_t aux_val     = ( 1 << 27 );  /* Allow non-secure access */
		uint32_t aux_mask    = 0xffffffff;

		/* Bits that are 0 in aux_mask will be zero'd in the AUX register.
		 * Bits that are 1 in aux_val will be set in the AUX register.
		 */
		aux_val |= ( 1 << 16 );      /* Enable 16-way cache */
		l2x0_init( (void __iomem *)MM_IO_BASE_L2C, aux_val, aux_mask );
	}
#endif
	clock_init();
	clock_tmp_init();

#if defined( CONFIG_EARLY_PRINTK )
	/* 
	 * Since we've changed the uart clock to be 29.5 MHz, we need to 
	 * update the debug uart baud rate to 115200. If we're using Early 
	 * Printk and we don't do this then all of the printk's which happen
	 * between here and when the serial port is opened will be at the
	 * wrong baud rate. 
	 */
	{
		uint8_t  val;
		volatile uint8_t *baseAddr = MM_IO_BASE_UART0;

		val = baseAddr[UARTB_LCR_OFFSET];
		val |= UARTB_LCR_DLAB_MASK;
		baseAddr[UARTB_LCR_OFFSET] = val;

		/* 
		 * 29500000 / ( 16 * 115200 ) = 16.005
		 */

		baseAddr[UARTB_DLH_IER_OFFSET] = 0;
		baseAddr[UARTB_RBR_THR_DLL_OFFSET] = 16;

		val &= ~UARTB_LCR_DLAB_MASK;
		baseAddr[UARTB_LCR_OFFSET] = val;
	}
#endif
	pinmux_init();

	kona_gpio_init(6);
}

/****************************************************************************
*
*   Called from setup_arch (in arch/arm/kernel/setup.c) to fixup any tags
*   passed in by the boot loader.
*
*****************************************************************************/
void __init hana_fixup
(
    struct machine_desc *desc,
    struct tag *t,
    char **cmdline,
    struct meminfo *mi
)
{
   printk("%s: machine number = %d = 0x%x\n", __func__, desc->nr, desc->nr);
}

void __init hana_add_common_devices(void)
{
   platform_add_devices(hana_devices, ARRAY_SIZE(hana_devices));
}

/* GP Timer init code, common for all rhea based platforms */
void __init timer_init (void)
{
        struct gp_timer_setup gpt_setup;

        gpt_setup.name   = "slave-timer";
        gpt_setup.ch_num = 0;
        gpt_setup.rate   = GPT_MHZ_1;

        /* Call the init function of timer module */
        kona_timer_init(&gpt_setup);
}

struct sys_timer kona_timer = {
        .init   = timer_init,
};

