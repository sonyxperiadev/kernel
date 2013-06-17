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
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcmpmu.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include <asm/system_misc.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/io_map.h>
#include <mach/clock.h>
#include <mach/memory.h>
#include <mach/system.h>
#include <mach/gpio.h>
#include <mach/pinmux.h>
#include <mach/kona.h>
#include <mach/timer.h>
#include <mach/profile_timer.h>
#ifdef CONFIG_HAWAII_L2X0_PREFETCH
#include <mach/cache-l2x0.h>
#endif
#include <mach/cpu.h>
#include <plat/scu.h>
#include <plat/kona_reset_reason.h>
#include <mach/sec_api.h>
#include <mach/cdebugger.h>

extern int reset_pwm_padcntrl(void);

static void hawaii_poweroff(void)
{
#ifdef CONFIG_MFD_BCM_PMU590XX
	bcm590xx_shutdown();
#endif

#if defined(CONFIG_MFD_BCMPMU) || defined(CONFIG_MFD_BCM_PMU59xxx)
	bcmpmu_client_power_off();
#endif

	mdelay(5);
	pr_err("Failed power off!!!\n");
	while (1) ;
}

void hawaii_restart(char mode, const char *cmd)
{
#if defined(CONFIG_MFD_BCMPMU) || defined(CONFIG_MFD_BCM_PMU59xxx)
	int ret = 0;
	if (hard_reset_reason) {
		ret = bcmpmu_client_hard_reset(hard_reset_reason);
		BUG_ON(ret);
	} else {
		switch (mode) {
		case 's':
			/* Jump into X address. Unused.
			 * Kept to catch wrong mode*/
			soft_restart(0);
			break;
		case 'h':
		default:
		/* Clear the magic key when reboot is required */
			if (cmd == NULL)
				cdebugger_set_upload_magic(0x00);
			ret = reset_pwm_padcntrl();
			if (ret)
				pr_err("%s Failed to reset PADCNTRL"\
				"pin for PWM2 to GPIO24:%d\n",\
				__func__, ret);
			kona_reset(mode, cmd);
			break;
		}
	}
#else
	switch (mode) {
	case 's':
		/* Jump into X address. Unused.
		* Kept to catch wrong mode*/
		soft_restart(0);
		break;
	case 'h':
	default:
	/* Clear the magic key when reboot is required */
		if (cmd == NULL)
			cdebugger_set_upload_magic(0x00);
		kona_reset(mode, cmd);
		break;
	}
#endif
}
EXPORT_SYMBOL(hawaii_restart);

#ifdef CONFIG_CACHE_L2X0
static void __init hawaii_l2x0_init(void)
{
	void __iomem *l2cache_base = (void __iomem *)(KONA_L2C_VA);
	u32 val;
	u32 aux_val = 0x00050000;
	u32 aux_mask = 0xfff0ffff;

	/*
	 * Enable L2 if it is not already enabled by the ROM code.
	 */
	val = readl(l2cache_base + L2X0_CTRL);
	val = val & 0x1;
	if (val == 0)
  {
      /* TURN ON THE L2 CACHE */
#ifdef CONFIG_MOBICORE_DRIVER
      secure_api_call(SMC_CMD_L2X0SETUP2, 0, aux_val, aux_mask, 0);
      secure_api_call(SMC_CMD_L2X0INVALL, 0, 0, 0, 0);
      secure_api_call(SMC_CMD_L2X0CTRL, 1, 0, 0, 0);
#else
      //secure_api_call_init();
      secure_api_call(SSAPI_ENABLE_L2_CACHE, 0, 0, 0, 0);
#endif
   }
	/*
	 * 32KB way size, 16-way associativity
	 */
	l2x0_init(l2cache_base, aux_val, aux_mask);

}
#endif

static int __init hawaii_postcore_init(void)
{
	int ret = 0;

	pr_info("Secure service initialized\n");
	secure_api_call_init();

	return ret;
}
postcore_initcall(hawaii_postcore_init);

static int __init hawaii_arch_init(void)
{
	int ret = 0;

#ifdef CONFIG_CACHE_L2X0
	hawaii_l2x0_init();
#endif

	return ret;
}
arch_initcall(hawaii_arch_init);

void __init hawaii_timer_init(void)
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
	gpt_setup.name = "aon-timer";
	gpt_setup.ch_num = 3;
	gpt_setup.rate = CLOCK_TICK_RATE;

	/* Call the init function of timer module */
	gp_timer_init(&gpt_setup);
	profile_timer_init(IOMEM(KONA_PROFTMR_VA));
}

#ifdef CONFIG_KONA_ATAG_DT
/* hawaii has 4 banks of GPIO pins */
uint32_t dt_pinmux_gpio_mask[4] = { 0, 0, 0, 0 };

uint32_t dt_gpio[128];
#endif

static void cpu_info_verbose(void)
{
	if (cpu_is_hawaii_A0())
		pr_info("Hawaii CHIPID-A0\n");
}

static int __init hawaii_init(void)
{
	pm_power_off = hawaii_poweroff;

	cpu_info_verbose();
	pinmux_init();

#ifdef CONFIG_KONA_ATAG_DT
	printk(KERN_INFO "pinmux_gpio_mask: 0x%x, 0x%x, 0x%x, 0x%x\n",
	       dt_pinmux_gpio_mask[0], dt_pinmux_gpio_mask[1],
	       dt_pinmux_gpio_mask[2], dt_pinmux_gpio_mask[3]);
#endif

#ifdef CONFIG_GPIOLIB
	/* hawaii has 4 banks of GPIO pins */
	kona_gpio_init(4);
#endif

	scu_init((void __iomem *)KONA_SCU_VA);
	return 0;
}

early_initcall(hawaii_init);
