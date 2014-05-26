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
#include <mach/sram_config.h>
#include <asm/cacheflush.h>

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

/* Register for panic to know the reason for rebooting */
static int in_panic;
static int panic_prep_restart(struct notifier_block *this,
			      unsigned long event, void *ptr)
{
	in_panic = 1;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= panic_prep_restart,
};

/* Mapping between restart cmd and S1 restart reason code */
struct sony_restart_reason_table {
	const char *cmd;
	unsigned int reason;
};

/* Structure of Sony restart reason block (defined by S1) */
#define SONY_RESTART_REASON_ADDR SRAM_CUST_REBOOT_REASON_BASE
#define SONY_RESTART_REASON_MAGIC 0xDEADBEEF
struct sony_restart_reason {
	unsigned int magic;
	unsigned int reason;
};

/* Restart reason table (defined by S1) */
#define SOMC_WARMBOOT_CLEAR       0xABADBABE
#define SOMC_WARMBOOT_NONE        0x00000000
#define SOMC_WARMBOOT_S1          0x6F656D53
#define SOMC_WARMBOOT_FB          0x77665500
#define SOMC_WARMBOOT_FOTA        0x6F656D46
#define SOMC_WARMBOOT_RECOVERY    (SOMC_WARMBOOT_FOTA)
#define SOMC_WARMBOOT_FOTA_CACHE  0x6F656D50
#define SOMC_WARMBOOT_NORMAL      0x77665501
#define SOMC_WARMBOOT_CRASH       0xC0DEDEAD

/*
 * Restart reason table
 *
 * This table maps the various commands supplied to the machine restart
 * call to the corresponding Sony restart reason recognized by S1 boot.
 *
 * Add more restart reasons to this table as needed!
 */
static const struct sony_restart_reason_table rrt[] = {
	{"ramdump", SOMC_WARMBOOT_CRASH},
	{"bootloader", SOMC_WARMBOOT_FB},
	{"recovery", SOMC_WARMBOOT_RECOVERY},
};

/*
 * Update Sony restart reason block with correct code
 *
 * This function will update the Sony restart reason block with the
 * correct structure to allow S1 boot to choose correct boot mode
 * on next warmboot. It will check the command specified by cmd and
 * match it against the rrt table above. If there is no match,
 * it will make a qualified guess based on whether a panic has occured
 * or not.
 *
 * mode: Not used
 * cmd: Checked against rrt table to determine S1 warmboot code.
 */
static void sony_set_restart_reason(char mode, const char *cmd)
{
	void *sony_restart_reason_base =
		ioremap(SONY_RESTART_REASON_ADDR,
			sizeof(struct sony_restart_reason));
	struct sony_restart_reason *srr = sony_restart_reason_base;
	int i;

	/* Check cmd against restart reason table */
	for (i = 0; i < ARRAY_SIZE(rrt); i++) {
		if (cmd && !strncmp(cmd, rrt[i].cmd, strlen(rrt[i].cmd))) {
			srr->reason = rrt[i].reason;
			break;
		}
	}

	/*
	 * If the cmd did not match any value in the table, or was null,
	 * make a qualified guess
	 */
	if (i == ARRAY_SIZE(rrt)) {
		/* No ramdump cmd specified, check if a panic has occured */
		if (in_panic)
			srr->reason = SOMC_WARMBOOT_CRASH;
		else
			srr->reason = SOMC_WARMBOOT_NORMAL;
	}
	/* Make sure the magic is correct */
	srr->magic = SONY_RESTART_REASON_MAGIC;
	iounmap(sony_restart_reason_base);

	/* Make sure to flush caches before rebooting */
	flush_cache_all();	/* L1 to L2 */
	outer_flush_all();	/* L2 to Ram */
}

void hawaii_restart(char mode, const char *cmd)
{
	/* Fill in the Sony restart reason block for S1 boot */
	sony_set_restart_reason(mode, cmd);

#if defined(CONFIG_MFD_BCMPMU) || defined(CONFIG_MFD_BCM_PMU59xxx)
	if (hard_reset_reason)
		bcmpmu_client_hard_reset(hard_reset_reason);
	else {
		switch (mode) {
		case 's':
			/* Jump into X address. Unused.
			 * Kept to catch wrong mode*/
			soft_restart(0);
			break;
		case 'h':
		default:
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

	/*
	 * Enable L2 if it is not already enabled by the ROM code.
	 */
	val = readl(l2cache_base + L2X0_CTRL);
	val = val & 0x1;
	if (val == 0)
		secure_api_call(SSAPI_ENABLE_L2_CACHE, 0, 0, 0, 0);
	/*
	 * 32KB way size, 16-way associativity
	 */
	l2x0_init(l2cache_base, 0x00050000, 0xfff0ffff);

#ifdef CONFIG_HAWAII_L2X0_PREFETCH
	hawaii_l2x0_prefetch(1);
#endif
}
#endif

static int __init hawaii_arch_init(void)
{
	int ret = 0;

	secure_api_call_init();

#ifdef CONFIG_CACHE_L2X0
	hawaii_l2x0_init();
#endif

	return ret;
}

arch_initcall(hawaii_arch_init);

void __init java_timer_init(void)
{
	struct gp_timer_setup gpt_setup;
	int cpu = smp_processor_id();

	pr_info("@@@@@@@@@@@@@@ %s()  from CPU %d \r\n", __func__, cpu);

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
	gpt_setup.name = "core-timer";
#ifdef CONFIG_SMP
	gpt_setup.ch_num = 3;
#else
	gpt_setup.ch_num = 0;
#endif
	gpt_setup.rate = CLOCK_TICK_RATE;

	/* Call the init function of timer module */
	gp_timer_init(&gpt_setup);
	//profile_timer_init(IOMEM(KONA_PROFTMR_VA));
}

#ifdef CONFIG_KONA_ATAG_DT
/* hawaii has 4 banks of GPIO pins */
uint32_t dt_pinmux_gpio_mask[4] = { 0, 0, 0, 0 };

uint32_t dt_gpio[128];
#endif

void cpu_info_verbose(void)
{
	if (cpu_is_java_A0())
		pr_info("Java CHIPID-A0\n");
	if (cpu_is_java_A1())
		pr_info("Java CHIPID-A1\n");
}
EXPORT_SYMBOL(cpu_info_verbose);

static void block_mm_access_to_hub(void)
{
	unsigned int value = 0x00;
	value =	readl_relaxed(KONA_CHIPREG_VA +
				CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET);

	value |= (CHIPREG_PERIPH_SPARE_CONTROL1_MM2HUB_WR_DISABLE_MASK |
			CHIPREG_PERIPH_SPARE_CONTROL1_MM2HUB_RD_DISABLE_MASK);

	writel_relaxed(value, (KONA_CHIPREG_VA +
				CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET));
}

static int __init hawaii_init(void)
{
	pm_power_off = hawaii_poweroff;

	/* Register to know when a panic has occured */
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);

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
	/*
	  Ensure that all MM accesss to CENTRAL HUB are blocked.
	  This is to ensure that spurious access such as :
	  1. reset value of UNICAM address registers,
	  2. V3D access (un-intentional)
	  are ignored.
	*/
	block_mm_access_to_hub();
	//scu_init((void __iomem *)KONA_SCU_VA);
	return 0;
}

early_initcall(hawaii_init);
