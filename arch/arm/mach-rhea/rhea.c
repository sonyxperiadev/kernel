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
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <mach/io_map.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/pwr_mgr.h>
#include <mach/clock.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcmpmu.h>
#include <mach/gpio.h>
#include <mach/pinmux.h>
#include <mach/kona.h>
#include <mach/timer.h>
#include <mach/profile_timer.h>

static void rhea_poweroff(void)
{
#ifdef CONFIG_MFD_BCM_PMU590XX
	bcm590xx_shutdown();
#endif

#ifdef CONFIG_MFD_BCMPMU
        bcmpmu_client_power_off();
#endif

	while(1)
		;
}

static void rhea_restart(char mode, const char *cmd)
{
	arm_machine_restart('h', cmd);
}


#ifdef CONFIG_CACHE_L2X0
static void __init rhea_l2x0_init(void)
{
	void __iomem *l2cache_base = (void __iomem *)(KONA_L2C_VA);

	/*
	 * 32KB way size, 8-way associativity
	 */
	l2x0_init(l2cache_base, 0x00040000, 0xfff0ffff);
}
#endif

/* GP Timer init code, common for all rhea based platforms */
void __init rhea_ray_timer_init (void)
{
	struct gp_timer_setup gpt_setup;

	/*
	 * IMPORTANT:
	 * If we have to use slave-timer as system timer, two modifications are required
	 * 1) modify the name of timer as, gpt_setup.name = "slave-timer";
	 * 2) By default when the clock manager comes up it disables most of
	 *    the clock. So if we switch to slave-timer we should prevent the
	 *    clock manager from doing this. So, modify plat-kona/include/mach/clock.h
	 *
	 * By default aon-timer as system timer the following is the config
	 * #define BCM2165x_CLK_TIMERS_FLAGS     (TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
         * #define BCM2165x_CLK_HUB_TIMER_FLAGS  (TYPE_PERI_CLK | SW_GATE)
	 *
	 * change it as follows to use slave timer as system timer
	 *
	 * #define BCM2165x_CLK_TIMERS_FLAGS     (TYPE_PERI_CLK | SW_GATE)
         * #define BCM2165x_CLK_HUB_TIMER_FLAGS  (TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
	 */
	gpt_setup.name   = "aon-timer";
	gpt_setup.ch_num = 0;
	gpt_setup.rate = CLOCK_TICK_RATE;

	/* Call the init function of timer module */
	gp_timer_init(&gpt_setup);
	profile_timer_init(IOMEM(KONA_PROFTMR_VA));
}

struct sys_timer kona_timer = {
	.init	= rhea_ray_timer_init,
};

#ifdef CONFIG_KONA_ATAG_DT
/* rhea has 4 banks of GPIO pins */
uint32_t dt_pinmux_gpio_mask[4] = {0, 0, 0, 0};
uint32_t dt_gpio[128];
#endif

static int __init rhea_init(void)
{
	pm_power_off = rhea_poweroff;
	arm_pm_restart = rhea_restart;

#ifdef CONFIG_CACHE_L2X0
	rhea_l2x0_init();
#endif

	pinmux_init();

#ifdef CONFIG_KONA_ATAG_DT
	printk(KERN_INFO "pinmux_gpio_mask: 0x%x, 0x%x, 0x%x, 0x%x\n",
		dt_pinmux_gpio_mask[0], dt_pinmux_gpio_mask[1],
		dt_pinmux_gpio_mask[2], dt_pinmux_gpio_mask[3]);
#endif

#ifdef CONFIG_GPIOLIB
	/* rhea has 4 banks of GPIO pins */
	kona_gpio_init(4);
#endif


	return 0;
}

early_initcall(rhea_init);

