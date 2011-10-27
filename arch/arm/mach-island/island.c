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
#include <linux/syscalls.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcmpmu.h>

#include <asm/io.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/io_map.h>
#include <mach/clock.h>
#include <mach/gpio.h>
#include <mach/timer.h>
#include <mach/kona.h>
#include <mach/profile_timer.h>
#include <mach/pinmux.h>

static void island_poweroff(void)
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

static void island_restart(char mode, const char *cmd)
{
	arm_machine_restart('h', cmd);
}


#ifdef CONFIG_CACHE_L2X0

/* Default L2 settings */
static int l2off = 0;
static int l2_non_secure_access = 1;
static int l2_d_prefetch = 1;
static int l2_i_prefetch = 1;
static int l2_early_bresp = 1;

static int __init l2off_setup(char *str)
{
	l2off = 1;
	return 1;
}
__setup("l2off", l2off_setup);


static int __init l2_d_prefetch_setup(char *str)
{
	get_option(&str, &l2_d_prefetch);
	return 1;
}
__setup("l2_dprefetch=", l2_d_prefetch_setup);

static int __init l2_i_prefetch_setup(char *str)
{
	get_option(&str, &l2_i_prefetch);
	return 1;
}
__setup("l2_iprefetch=", l2_i_prefetch_setup);

static int __init l2_early_bresp_setup(char *str)
{
	get_option(&str, &l2_early_bresp);
	return 1;
}
__setup("l2_early_bresp=", l2_early_bresp_setup);

static int __init island_l2x0_init(void)
{
	void __iomem *l2cache_base;
	uint32_t aux_val = 0;
	uint32_t aux_mask = 0xC200ffff;

	if (l2off)
	{
		/*  cmdline argument l2off will turn off l2 cache even if configured on */
		printk("%s: Warning: L2X0 *not* enabled due to l2off cmdline override\n", __func__);
		return 0;
	}

	l2cache_base = (void __iomem *)(KONA_L2C_VA);

	/*
	 * Zero bits in aux_mask will be cleared
	 * One  bits in aux_val  will be set
	 */

	aux_val |= ( 1 << L2X0_AUX_CTRL_ASSOCIATIVITY_SHIFT );	/* 16-way cache */
	aux_val |= ( ( l2_non_secure_access ? 1 : 0 ) << L2X0_AUX_CTRL_NS_INT_CTRL_SHIFT );		/* Allow non-secure access */
	aux_val |= ( ( l2_d_prefetch        ? 1 : 0 ) << L2X0_AUX_CTRL_DATA_PREFETCH_SHIFT );	/* Data prefetch */
	aux_val |= ( ( l2_i_prefetch        ? 1 : 0 ) << L2X0_AUX_CTRL_INSTR_PREFETCH_SHIFT );	/* Instruction prefetch */
	aux_val |= ( ( l2_early_bresp       ? 1 : 0 ) << L2X0_AUX_CTRL_EARLY_BRESP_SHIFT );		/* Early BRESP */
	aux_val |= ( 2 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT );			/* 32KB */

	/*
	 * Set bit 22 in the auxiliary control register. If this bit
	 * is cleared, PL310 treats Normal Shared Non-cacheable
	 * accesses as Cacheable no-allocate.
	 */
	aux_val |= 1 << L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT;

	l2x0_init(l2cache_base, aux_val, aux_mask);

	return 0;
}
#endif

static int __init island_init(void)
{
	pm_power_off = island_poweroff;
	arm_pm_restart = island_restart;

#ifdef CONFIG_CACHE_L2X0
	island_l2x0_init();
#endif

#ifdef CONFIG_HAVE_CLK
	//island_clock_init();
#endif

	pinmux_init();

	/* island has 6 banks of GPIO pins */
	kona_gpio_init(6);

	return 0;
}
early_initcall(island_init);

static void __init island_timer_init(void)
{
	struct gp_timer_setup gpt_setup;

#ifdef CONFIG_LOCAL_TIMERS
	extern void __iomem *twd_base;
	twd_base = __io(KONA_PTIM_VA);
#endif
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
	gpt_setup.rate   = CLOCK_TICK_RATE;

	/* Call the init function of timer module */
	gp_timer_init(&gpt_setup);
	profile_timer_init(IOMEM(KONA_PROFTMR_VA));
}

struct sys_timer kona_timer = {
        .init   = island_timer_init,
};

EXPORT_SYMBOL( sys_open );
EXPORT_SYMBOL( sys_read );

