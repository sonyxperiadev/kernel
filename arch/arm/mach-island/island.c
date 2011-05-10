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

#include <linux/init.h>
#include <linux/serial_8250.h>
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <linux/mfd/bcm590xx/core.h>

#include <asm/io.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/island.h>
#include <mach/io_map.h>
#include <mach/clock.h>
#include <mach/gpio.h>
#include <mach/timer.h>
#include <mach/kona.h>
#include <mach/pinmux.h>
#include <mach/rdb/brcm_rdb_uartb.h>


#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR
#define KONA_UART3_PA   UARTB4_BASE_ADDR

#define UARTB_USR_OFFSET	0x0000007C

#define KONA_8250PORT(name)                                                   \
{                                                                             \
   .membase    = (void __iomem *)(KONA_##name##_VA),                          \
   .mapbase    = (resource_size_t)(KONA_##name##_PA),                         \
   .irq        = BCM_INT_ID_##name,                                           \
   .uartclk    = 13000000,                                                    \
   .regshift   = 2,                                                           \
   .iotype     = UPIO_DWAPB,                                                  \
   .type       = PORT_16550A,                                                 \
   .flags      = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST,          \
   .private_data = (void __iomem *)((KONA_##name##_VA) + UARTB_USR_OFFSET),   \
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0),
	KONA_8250PORT(UART1),
	KONA_8250PORT(UART2),
	KONA_8250PORT(UART3),
	{ .flags = 0, },
};

static struct platform_device board_serial_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = uart_data,
	},
};

static struct platform_device *board_devices[] __initdata = {
	&board_serial_device,
};

static void island_poweroff(void)
{
#ifdef CONFIG_MFD_BCM_PMU590XX
	bcm590xx_shutdown();
#endif

	while(1)
		;
}

static void island_restart(char mode, const char *cmd)
{
	arm_machine_restart('h', cmd);
}


#ifdef CONFIG_CACHE_L2X0
static void __init island_l2x0_init(void)
{
	void __iomem *l2cache_base = (void __iomem *)(KONA_L2C_VA);

	/*
	 * 32KB way size, 16-way associativity
	 */
	l2x0_init(l2cache_base, 0x00050000, 0xfff0ffff);
}
#endif

void __init island_init(void)
{
	pm_power_off = island_poweroff;
	arm_pm_restart = island_restart;
	
#ifdef CONFIG_CACHE_L2X0
	island_l2x0_init();
#endif

#ifdef CONFIG_HAVE_CLK
	clock_init();
#endif

	pinmux_init();

	/* island has 6 banks of GPIO pins */ 
	kona_gpio_init(6);
}

early_initcall(island_init);

static void __init island_timer_init(void)
{
	struct gp_timer_setup gpt_setup;

	gpt_setup.name   = "slave-timer";
	gpt_setup.ch_num = 0;
	gpt_setup.rate   = GPT_MHZ_1;

	/* Call the init function of timer module */
	kona_timer_init(&gpt_setup);
}

struct sys_timer kona_timer = {
        .init   = island_timer_init,
};

void __init island_add_common_devices(void)
{
	platform_add_devices(board_devices, ARRAY_SIZE(board_devices));
}


