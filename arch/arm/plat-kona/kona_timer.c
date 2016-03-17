/******************************************************************************/
/*                                                                            */
/*  Copyright 2010  Broadcom Corporation                                      */
/*                                                                            */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed    */
/*     to you under the terms of the GNU General Public License version 2     */
/*     (the GPL), available at                                                */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and     */
/*     to copy and distribute the resulting executable under terms of your    */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module. An     */
/*     independent module is a module which is not derived from this          */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine      */
/*     this software in any way with any other Broadcom software provided     */
/*     under a license other than the GPL, without Broadcom's express prior   */
/*     written consent.                                                       */
/*                                                                            */
/******************************************************************************/

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <mach/memory.h>
#include <linux/io.h>
#include <mach/cpu.h>
#include <mach/kona_timer.h>
#include <mach/io.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_khubaon_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#if defined(CONFIG_ARCH_RHEA) || defined(CONFIG_ARCH_CAPRI) ||  \
	defined(CONFIG_ARCH_HAWAII) || defined(CONFIG_ARCH_JAVA)
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#endif
#else
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#endif

/* Macros to set/get multiple register bits at one time */
#define kona_set_reg_field(addr, mask, shift, val)      \
	do {                                            \
		uint32_t tmp;                                \
		tmp  = readl(addr);              \
		tmp &= ~(mask);                              \
		tmp |= (((val) << (shift)) & (mask));        \
		writel(tmp, addr);                 \
	} while (0)

/* Data structures */

enum config_state {
	NOT_CONFIGURED = 0,	/* Timer module not configured */
	CONFIGURED_FREE,	/* Timer module configured but
				 * channels are not being used so
				 * can change the rate
				 */
	CONFIGURED_BUSY,	/* Timer module configured and channels
				 * are being used so re-configuring
				 * the rate is not allowed
				 */
};

enum timer_module {
	HUB_TIMER = 0,
	SLAVE_TIMER,
#ifdef CONFIG_ARCH_JAVA
	CORE_TIMER,
#endif
};

struct kona_timer_module {
	struct kona_timer *pkt;
	spinlock_t lock;
	int num_of_timers;
	char name[255];
	char clk_name[255];
	enum config_state cfg_state;
	void __iomem *reg_base;
	struct clk *clk;
	unsigned long rate;
};

static struct kona_timer aon_hub_timer[NUM_OF_CHANNELS] = {
	{.ch_num = 0, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS1},
	{.ch_num = 1, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS2},
	{.ch_num = 2, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS3},
	{.ch_num = 3, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS4},
};

#ifdef CONFIG_ARCH_JAVA
static struct kona_timer aon_core_timer[NUM_OF_CHANNELS] = {
	{.ch_num = 0, .busy = 0, .irq = BCM_INT_ID_CORE_TIMERS1},
	{.ch_num = 1, .busy = 0, .irq = BCM_INT_ID_CORE_TIMERS2},
	{.ch_num = 2, .busy = 0, .irq = BCM_INT_ID_CORE_TIMERS3},
	{.ch_num = 3, .busy = 0, .irq = BCM_INT_ID_CORE_TIMERS4},
};
#endif

static struct kona_timer periph_timer[NUM_OF_CHANNELS] = {
	{.ch_num = 0, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS1},
	{.ch_num = 1, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS2},
	{.ch_num = 2, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS3},
	{.ch_num = 3, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS4},
};

/* There are three instances of timer modules on Kona */
static struct kona_timer_module timer_module_list[NUM_OF_TIMER_MODULES] = {
	{.pkt = &aon_hub_timer[0], .name = "aon-timer",
	 .cfg_state = NOT_CONFIGURED, .rate = 0,
	 .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_TMR_HUB_VA),
	 .clk_name = HUB_TIMER_PERI_CLK_NAME_STR,
	},
	{.pkt = &periph_timer[0], .name = "slave-timer",
	 .cfg_state = NOT_CONFIGURED, .rate = 0,
	 .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_SYSTMR_VA),
	 .clk_name = TIMERS_PERI_CLK_NAME_STR,
	},
#ifdef CONFIG_ARCH_JAVA
	{.pkt = &aon_core_timer[0], .name = "core-timer",
	 .cfg_state = NOT_CONFIGURED, .rate = 0,
	 .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_CORETMR_VA),
	/* .clk_name = , Assume this clk is always @ 32KHz, no gating cntrl */
	},
#endif
};

/* Local static functions */
static struct kona_timer_module *__get_timer_module(char *name);
static int __config_aon_hub_timer_clock(struct kona_timer_module *pktm,
					unsigned int rate, unsigned int val);
static int __config_slave_timer_clock(struct kona_timer_module *pktm,
				      unsigned int rate, unsigned int reg_val);
/*
 * Ensure that the code calls one function to read the counter value
 * This will make life simple to implement the debouncing logic needed for
 * Rhea B1
 */
static inline unsigned long __get_counter(void __iomem *reg_base);
static inline void __disable_all_channels(void __iomem *reg_base);
static inline void __disable_channel(void __iomem *reg_base, int ch_num);
static irqreturn_t kona_timer_isr(int irq, void *dev_id);

static struct irqaction hub_timer_irq[NUM_OF_CHANNELS] = {
	{.name = "Hub-Timer-Ch0", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[0], .handler = kona_timer_isr},
	{.name = "Hub-Timer-Ch1", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[0], .handler = kona_timer_isr},
	{.name = "Hub-Timer-Ch2", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[0], .handler = kona_timer_isr},
	{.name = "Hub-Timer-Ch3", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[0], .handler = kona_timer_isr},
};


static struct irqaction periph_timer_irq[NUM_OF_CHANNELS] = {
	{.name = "Slave-Timer-Ch0", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[1], .handler = kona_timer_isr},
	{.name = "Slave-Timer-Ch1", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[1], .handler = kona_timer_isr},
	{.name = "Slave-Timer-Ch2", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[1], .handler = kona_timer_isr},
	{.name = "Slave-Timer-Ch3", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[1], .handler = kona_timer_isr},
};

#ifdef CONFIG_ARCH_JAVA
static struct irqaction core_timer_irq[NUM_OF_CHANNELS] = {
	{.name = "Core-Timer-Ch0", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[2], .handler = kona_timer_isr},
	{.name = "Core-Timer-Ch1", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[2], .handler = kona_timer_isr},
	{.name = "Core-Timer-Ch2", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[2], .handler = kona_timer_isr},
	{.name = "Core-Timer-Ch3", .flags = IRQF_DISABLED | IRQF_TIMER,
	 .dev_id = &timer_module_list[2], .handler = kona_timer_isr},
};
#endif
static int kona_init_done;

struct kona_timer *get_timer_ptr(char *name, int chan)
{
	struct kona_timer_module *ktm = NULL;

	ktm = __get_timer_module(name);
	return ktm->pkt + chan;
}

/*
 *  kona_timer_modules_init - Initialize the data structures
 *  that depcits the Kona timer modules
 */
int __init kona_timer_modules_init(void)
{
	int i;

	if (kona_init_done != 0) {
		pr_err("kona_timer_modules_init: Already initialized\n");
		return -1;
	}

	for (i = 0; i < NUM_OF_TIMER_MODULES; i++) {
		spin_lock_init(&timer_module_list[i].lock);
		__disable_all_channels(timer_module_list[i].reg_base);
	}

	for (i = 0; i < NUM_OF_CHANNELS; i++) {
		setup_irq(aon_hub_timer[i].irq, &hub_timer_irq[i]);
		aon_hub_timer[i].cfg.arg = NULL;
		aon_hub_timer[i].cfg.mode = 0;
		aon_hub_timer[i].cfg.cb = NULL;
		aon_hub_timer[i].cfg.reload = 0;
		aon_hub_timer[i].ktm = &timer_module_list[0];
	}

	for (i = 0; i < NUM_OF_CHANNELS; i++) {
		setup_irq(periph_timer[i].irq, &periph_timer_irq[i]);
		periph_timer[i].cfg.arg = NULL;
		periph_timer[i].cfg.mode = 0;
		periph_timer[i].cfg.cb = NULL;
		periph_timer[i].cfg.reload = 0;
		periph_timer[i].ktm = &timer_module_list[1];
	}

#ifdef CONFIG_ARCH_JAVA
	for (i = 0; i < NUM_OF_CHANNELS; i++) {
		setup_irq(aon_core_timer[i].irq, &core_timer_irq[i]);
		aon_core_timer[i].cfg.arg = NULL;
		aon_core_timer[i].cfg.mode = 0;
		aon_core_timer[i].cfg.cb = NULL;
		aon_core_timer[i].cfg.reload = 0;
		aon_core_timer[i].ktm = &timer_module_list[2];
	}
#endif

	kona_init_done = 1;
	return 0;
}

/*
 * Obtain clocks for Kona timer and store them into the internal data
 * structure
 */
void kona_timer_clk_setup(void)
{
	unsigned int i;

	for (i = 0; i < NUM_OF_TIMER_MODULES; i++) {
		timer_module_list[i].clk = clk_get(NULL,
				timer_module_list[i].clk_name);
		WARN_ON(IS_ERR_OR_NULL(timer_module_list[i].clk));
	}
}

/*
 * kona_timer_module_set_rate - Set the speed in which a timer module should
 *                              count
 * name - Name of the Timer to configure
 * rate - Speed
 */
int kona_timer_module_set_rate(char *name, unsigned int rate)
{
	struct kona_timer_module *pktm = NULL;
	int ret;
	unsigned long flags;
	unsigned int reg_val;

	pktm = __get_timer_module(name);
	if (pktm == NULL)
		return -1;

	/*
	 * While multiple clients want to set the rate for the same timer, we
	 * need protection
	 */

	spin_lock_irqsave(&pktm->lock, flags);

	/* given time is already configured */
	if (pktm->cfg_state == CONFIGURED_BUSY)
		goto err_out;

#if defined(CONFIG_ARCH_RHEA) || defined(CONFIG_ARCH_HAWAII) || \
					defined(CONFIG_ARCH_JAVA)
	/* Configure KONA Timer count in 32 bit mode */
	writel(0x0, KONA_CHIPREG_VA + CHIPREG_HUB_TIMER_WIDTH_OFFSET);
#endif
	/*
	 * Updating the configured state should also be protected,
	 * there should not be a situation where we have set the rate
	 * but before updating the state variable there is a context switch.
	 *
	 * It may sound odd to do this before actualling setting the rate.
	 * But we know for sure that we have taken a decission to set the
	 * timer to the 'rate' here and the following __config_xx functions
	 * will set the timer to this rate. We cannot protect this full
	 * function with spin lock since the clock calls invoked by the
	 * __config_xx functions acquire mutex-s which sleeps and which leads
	 * to theoretical deadlock as reported by CONFIG_DEBUG_LOCKDEP.
	 * So we do all the operations that needs to be protected using
	 * spin_lock once and then release it and call the sub functions to
	 * configure the clock. From the sub-functions if we are to do local
	 * settings (if clock call fails) then we re-acquire the spin_lock
	 *
	 */
	if (pktm->reg_base == IOMEM(KONA_SYSTMR_VA)) {
		/* Slave timer only supports 32KHz and 1MHz */
		if (rate == 32768)
			reg_val = 1;
		else if (rate == 1000000)
			reg_val = 0;
		else
			goto err_out;
	} else {
		/* AON timer only supports 32KHz, 1MHz and 19.5 MHz */
		switch (rate) {
		case 32768:
			/* 32 KHz */
			reg_val = 0;
			break;
		case 1000000:
			/* 1 MHz */
			reg_val = 1;
			break;
		case 19500000:
			/* 19.5 MHz */
			reg_val = 2;
			break;
		default:
			pr_err(
			    "aon hub timer clock: Invalid value requested %d\r\n"
			     , rate);
			goto err_out;
		}
	}

	pktm->rate = rate;
	pktm->cfg_state = CONFIGURED_FREE;
	spin_unlock_irqrestore(&pktm->lock, flags);

	/*
	 * Based on the timer name and the speed requested configure the
	 * appropriate clock registers. Note that these sub-functions first
	 * try to make clk calls. If the clk call fails which means the clock
	 * manager is not up yet, then it will peform the configuration on its
	 * own.
	 */
	if (pktm->reg_base == IOMEM(KONA_SYSTMR_VA))
		ret = __config_slave_timer_clock(pktm, rate, reg_val);
	else
		ret = __config_aon_hub_timer_clock(pktm, rate, reg_val);
	goto out;

err_out:
	spin_unlock_irqrestore(&pktm->lock, flags);
	ret = -1;
out:
	return ret;
}
EXPORT_SYMBOL(kona_timer_module_set_rate);

/*
 * kona_timer_module_get_rate - Get the speed in which a timer module is running
 * name - Name of the Timer module
 */
int kona_timer_module_get_rate(char *name)
{
	struct kona_timer_module *pktm = NULL;

	pktm = __get_timer_module(name);
	if (pktm == NULL)
		return -1;

	return pktm->rate;
}
EXPORT_SYMBOL(kona_timer_module_get_rate);

/*
 *  kona_timer_request - Get access to a channel in the given timer
 *  name - Name of the Timer module
 *  channel - Channel number requested. If this is -1 then by default
 *            the next available channel will be returned
 */
struct kona_timer *kona_timer_request(char *name, int channel)
{
	int i;
	struct kona_timer *pkt = NULL;
	struct kona_timer_module *pktm = NULL;
	unsigned long flags;

	if (NULL == name)
		return pkt;

	pktm = __get_timer_module(name);
	if (NULL == pktm)
		return pkt;

	spin_lock_irqsave(&pktm->lock, flags);

	pkt = pktm->pkt;

	if (channel == -1) {
		for (i = 0; i < NUM_OF_CHANNELS; i++, pkt++) {
			if (pkt->busy == 0)
				break;
		}
		if (i == NUM_OF_CHANNELS)
			pkt = NULL;
		else
			pkt->busy = 1;
	} else {
		pkt += channel;
		if (pkt->busy)
			pkt = NULL;
		else
			pkt->busy = 1;
	}

	if (pkt != NULL)
		pktm->cfg_state = CONFIGURED_BUSY;

	spin_unlock_irqrestore(&pktm->lock, flags);

	return pkt;
}
EXPORT_SYMBOL(kona_timer_request);

/*
 *  kona_timer_config - Configure the following parameters of the timer
 *  1) mode of the timer - periodic/one shot
 *  2) call back function that will be called from the ISR context
 *  3) context to be passed back in the call back function
 *
 *  kt - Kona timer context (returned by kona_timer_request())
 *  pcfg - pointer to the configuration structure
 */
int kona_timer_config(struct kona_timer *kt, struct timer_ch_cfg *pcfg)
{
	struct kona_timer_module *ktm;
	unsigned long flags;

	if ((NULL == kt) || (NULL == pcfg))
		return -1;

	ktm = kt->ktm;

	spin_lock_irqsave(&ktm->lock, flags);

	kt->cfg.arg = pcfg->arg;
	kt->cfg.mode = pcfg->mode;
	kt->cfg.cb = pcfg->cb;

	spin_unlock_irqrestore(&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_config);

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
/*
 * Wait for the new comparator value to be loaded
 */
static inline void __wait_for_compare_val_sync(void __iomem *reg_base,
		int ch_num)
{
	/*
	 * Wait for new compare value to be loaded within the timer. After an
	 * update to the timer compare value, one needs to wait for this bit
	 * to clear before enabling; otherwise the old value will be used
	 *
	 * Sync time is ~ 3 32KHz clock cycles
	 */
	while (readl(reg_base + KONA_GPTIMER_STCS_OFFSET) &
		(1 << (KONA_GPTIMER_STCS_STCM0_SYNC_SHIFT + ch_num)))
		;
}

/*
 * Wait for compare enable to be synced into the timer clock domain
 */
static inline int __wait_for_compare_enable_sync(void __iomem *reg_base,
		int ch_num)
{
	int enabled;

	if (readl(reg_base + KONA_GPTIMER_STCS_OFFSET) &
		(1 << (ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT))) {
		/*
		 * NOTE: compare enable sync bit, wait for the sync bit
		 * until it's changed from low to high. It's going to take
		 * 2 clock edge (30us ~ 60us on 32khz).
		 * Default value on POR is 0.
		 */
		while (!(readl(reg_base + KONA_GPTIMER_STCS_OFFSET) &
			(1 << (KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT +
			ch_num))))
			;

		enabled = 1;
	} else {
		/*
		 * NOTE: compare enable sync bit is changed from high to low
		 * after 2 clock edge when compare enable bit is disabled.
		 */
		while (readl(reg_base + KONA_GPTIMER_STCS_OFFSET) &
			(1 << (KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT +
			ch_num)))
			;
		enabled = 0;
	}

	return enabled;
}
#endif

/*
 * kona_timer_set_match_start - Set the match register for the timer and start
 * counting
 *
 *  kt - Kona timer context (returned by kona_timer_request())
 *  load - The load value to be programmed. This function will internally
 *         add this value to the current counter and program the resultant
 *         in the match register. Once the timer is started when the
 *         counter reaches this value an interrupt will be raised
 */
int kona_timer_set_match_start(struct kona_timer *kt, unsigned long load)
{
	struct kona_timer_module *ktm;
	unsigned long flags;
	unsigned long reg, lsw;
	s32 next_lsw;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;

	/* guarantee the minimum requested time */
	if (load < MIN_KONA_DELTA_CLOCK)
		load = MIN_KONA_DELTA_CLOCK;

	spin_lock_irqsave(&ktm->lock, flags);

	__disable_channel(ktm->reg_base, kt->ch_num);

	/*
	 * Remember the reload value in case of periodic timers.
	 * Note that the HW does not support periodic timers. Once
	 * there is a match the interrupt is triggered and from the
	 * ISR we have to re-program the match register to implement
	 * periodic ISR.
	 */

	if (kt->cfg.mode == MODE_PERIODIC)
		kt->cfg.reload = load;

	/* First read the existing counter */
	lsw = __get_counter(ktm->reg_base);
	kt->expire = lsw + load;

	/* Load the match register */
	writel(kt->expire,
	       ktm->reg_base + KONA_GPTIMER_STCM0_OFFSET + (kt->ch_num * 4));

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	/* Poll the corresponding STCS bits to become 0.
	 * This is to make sure the next event tick value is actually loaded
	 * (taking 3 32KHz clock cycles) before enabling compare (taking
	 *  2 32KHz clock cycles).
	 */
	__wait_for_compare_val_sync(ktm->reg_base, kt->ch_num);
#endif

	/* Enable compare */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	/*
	 * Clean up the Match field (3..0), writing 1 to this bit
	 * would clean the interrupt for the respective channel.
	 * So do this for only the required channel.
	 */
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	/*
	 * Skip the wrap around check logic for slower timer channels since
	 * the CPU penalty for reading/syncing the slower timers is much
	 * higher. Furthermore, the 32KHz timers run slow enough for the
	 * "minimum delta" to provide adequate protection against the
	 * wrapping.
	 */
	if (ktm->rate < 1000000)
		goto out;

	/*
	 * Gracefully handle the case where the programmed match value is
	 * within 65K counts of the counter wrap around value and the counter
	 * wraps before the match syncing/enabling logic kicks in.
	 */
	if (kt->expire >= 0xFFFEFC00) {
#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
		__wait_for_compare_enable_sync(ktm->reg_base, kt->ch_num);
#endif
		reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
		if (reg & (1 << (kt->ch_num +
		    KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT)))
			goto out;

		next_lsw = (s32)__get_counter(ktm->reg_base);
		if ((next_lsw >= 0) && ((s32)(lsw) < 0)) {
			__disable_channel(ktm->reg_base, kt->ch_num);
			writel(next_lsw, ktm->reg_base +
			       KONA_GPTIMER_STCM0_OFFSET + (kt->ch_num * 4));
#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
			__wait_for_compare_val_sync(ktm->reg_base, kt->ch_num);
#endif
			reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
			reg |= (1 << (kt->ch_num +
				KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
			writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
		}
	}
out:
	spin_unlock_irqrestore(&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_set_match_start);

/*
 * kona_timer_stop  - Stop the timer.
 *
 * kt - The timer context to be stopped.
 */
int kona_timer_stop(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	unsigned long flags;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave(&ktm->lock, flags);

	__disable_channel(ktm->reg_base, kt->ch_num);

	spin_unlock_irqrestore(&ktm->lock, flags);
	return 0;
}
EXPORT_SYMBOL(kona_timer_stop);

int kona_timer_suspend(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	struct clk *clk;
	unsigned long flags;

	if (!kt)
		return -EINVAL;

	ktm = kt->ktm;

	/* only touch the slave timer clock */
	if (strcmp(ktm->name, "slave-timer") != 0)
		return 0;

	clk = ktm->clk;
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("timer_suspend: clk_get of %s failed\n", ktm->clk_name);
		/* still allow suspend */
		return 0;
	}

	spin_lock_irqsave(&ktm->lock, flags);
	clk_disable(clk);
	spin_unlock_irqrestore(&ktm->lock, flags);

	return 0;
}

int kona_timer_resume(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	struct clk *clk;
	unsigned long flags;
	int ret;

	if (!kt)
		return -EINVAL;

	ktm = kt->ktm;

	/* only touch the slave timer clock */
	if (strcmp(ktm->name, "slave-timer") != 0)
		return 0;

	clk = ktm->clk;
	if (IS_ERR_OR_NULL(clk)) {
		pr_err("timer_resume: clk_get of %s failed\n", ktm->clk_name);
		/* still allow resume */
		return 0;
	}

	spin_lock_irqsave(&ktm->lock, flags);
	ret = clk_enable(clk);
	if (ret < 0)
		pr_err("timer_resume: clk_enable of %s failed\n",
				ktm->clk_name);
	spin_unlock_irqrestore(&ktm->lock, flags);

	return 0;
}

/*
 * kona_timer_free - Release the timer, after this call the timer can be used
 * again by others.
 *
 * kt - Timer context to be freed.
 */
int kona_timer_free(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	unsigned long flags, i;
#ifdef CONFIG_HAVE_CLK
	struct clk *clk;
#endif
	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave(&ktm->lock, flags);

	__disable_channel(ktm->reg_base, kt->ch_num);
	kt->busy = 0;

	/*
	 * Check if all the channels in this module are free, if yes
	 * then move the state of the timer module so that IF NEEDED
	 * the rate can be re-configured.
	 */
	kt = ktm->pkt;
	for (i = 0; i < NUM_OF_CHANNELS; i++, kt++) {
		if (kt->busy == 1)
			break;
	}

	/* All the timers are free (NOT BUSY) so move the
	 * timer module's state
	 */
	if (i == NUM_OF_CHANNELS) {

		ktm->cfg_state = CONFIGURED_FREE;

		/* Release the lock before calling clk APIs
		 * that could sleep
		 */
		spin_unlock_irqrestore(&ktm->lock, flags);
#ifdef CONFIG_HAVE_CLK
		clk = clk_get(NULL, ktm->clk_name);
		if (IS_ERR_OR_NULL(clk))
			pr_err
			    ("timer_free: clk_get failed, so clock manager"
			     "is not up use local calls\n");
		else
			clk_disable(clk);
#endif
	} else {
		/* Release the lock, this path would be taken if some of the
		 * channels in the timer are still busy.
		 */
		spin_unlock_irqrestore(&ktm->lock, flags);
	}
	return 0;
}
EXPORT_SYMBOL(kona_timer_free);

/*
 * kona_timer_get_counter - Read the counter register of the timer
 *
 * kt - Timer context to be freed.
 */
unsigned int notrace kona_timer_get_counter(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	unsigned int counter;
	unsigned long flags;

	if (NULL == kt) {
		pr_err("%s timer object cannot be NULL\n", __func__);
		BUG();
	}

	local_irq_save(flags);
	ktm = kt->ktm;
	counter = __get_counter(ktm->reg_base);
	local_irq_restore(flags);

	return counter;
}
EXPORT_SYMBOL(kona_timer_get_counter);

/*
 * kona_timer_disable_and_clear - Disable the timer and clear the
 * interrupt
 *
 * kt - Timer context to be freed.
 */
int kona_timer_disable_and_clear(struct kona_timer *kt)
{
	unsigned long reg;
	struct kona_timer_module *ktm;
	unsigned long flags;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave(&ktm->lock, flags);

	/*
	 * clear and disable interrupts
	 * We are using compare/match register 0 for our system interrupts
	 */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	/* Clear compare (0) interrupt */
	/*
	 * Clean up the Match field (3..0), writing 1 to this bit
	 * would clean the interrupt for the respective channel.
	 * So do this for only the required channel.
	 */
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= 1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	/* disable compare */
	reg &= ~(1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));

	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	spin_unlock_irqrestore(&ktm->lock, flags);
	return 0;
}
EXPORT_SYMBOL(kona_timer_disable_and_clear);

/* Return the counter value of hub timer */
unsigned long kona_hubtimer_get_counter(void)
{
	unsigned long flags, counter;

	local_irq_save(flags);
	counter = __get_counter(timer_module_list[0].reg_base);
	local_irq_restore(flags);

	return counter;
}
EXPORT_SYMBOL(kona_hubtimer_get_counter);

#ifdef CONFIG_ARCH_JAVA
unsigned long kona_coretimer_get_counter(void)
{
	unsigned long flags, counter;

	local_irq_save(flags);
	counter = __get_counter(timer_module_list[CORE_TIMER].reg_base);
	local_irq_restore(flags);

	return counter;
}
EXPORT_SYMBOL(kona_coretimer_get_counter);
#endif

/* Capture the current state of hub timer */
struct kona_timer_regs {
	int stcs;
	int stclo;
	int stchi;
	int stcm0;
	int stcm1;
	int stcm2;
	int stcm3;
};

#ifdef CONFIG_ARCH_JAVA
static struct kona_timer_regs kona_timer_state[3];
#endif

#ifdef CONFIG_ARCH_HAWAII
static struct kona_timer_regs kona_timer_state[2];
#endif

static void kona_dbg_get_timer_state(int context)
{
	unsigned long flags;
	struct kona_timer_module *ktm = &timer_module_list[context];

	spin_lock_irqsave(&ktm->lock, flags);
	kona_timer_state[context].stcs = readl(ktm->reg_base +
		KONA_GPTIMER_STCS_OFFSET);
	kona_timer_state[context].stclo = readl(ktm->reg_base +
		KONA_GPTIMER_STCLO_OFFSET);
	kona_timer_state[context].stchi = readl(ktm->reg_base +
		KONA_GPTIMER_STCHI_OFFSET);
	kona_timer_state[context].stcm0 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM0_OFFSET);
	kona_timer_state[context].stcm1 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM1_OFFSET);
	kona_timer_state[context].stcm2 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM2_OFFSET);
	kona_timer_state[context].stcm3 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM3_OFFSET);
	spin_unlock_irqrestore(&ktm->lock, flags);
}

void kona_dump_timer_regs(bool print_state, int context)
{
	if (print_state) {
		printk(KERN_ERR "Kona Timer[%d]: STCS =%08x\n", context,
				kona_timer_state[context].stcs);
		printk(KERN_ERR "Kona Timer[%d]: STCLO=%08x\n", context,
				kona_timer_state[context].stclo);
		printk(KERN_ERR "Kona Timer[%d]: STCHI=%08x\n", context,
				kona_timer_state[context].stchi);
		printk(KERN_ERR "Kona Timer[%d]: STCM0=%08x\n", context,
				kona_timer_state[context].stcm0);
		printk(KERN_ERR "Kona Timer[%d]: STCM1=%08x\n", context,
				kona_timer_state[context].stcm1);
		printk(KERN_ERR "Kona Timer[%d]: STCM2=%08x\n", context,
				kona_timer_state[context].stcm2);
		printk(KERN_ERR "Kona Timer[%d]: STCM3=%08x\n", context,
				kona_timer_state[context].stcm3);
	}
}

void kona_timer_save_state(bool print_state)
{

	kona_dbg_get_timer_state(HUB_TIMER);
	kona_dbg_get_timer_state(SLAVE_TIMER);
#ifdef CONFIG_ARCH_JAVA
	kona_dbg_get_timer_state(CORE_TIMER);
#endif
	kona_dump_timer_regs(print_state, HUB_TIMER);
	kona_dump_timer_regs(print_state, SLAVE_TIMER);
#ifdef CONFIG_ARCH_JAVA
	kona_dump_timer_regs(print_state, CORE_TIMER);
#endif
}
EXPORT_SYMBOL(kona_timer_save_state);


/* Return the counter value of slave timer */
unsigned long kona_slavetimer_get_counter(void)
{
	unsigned long flags, counter;

	local_irq_save(flags);
	counter = __get_counter(timer_module_list[1].reg_base);
	local_irq_restore(flags);

	return counter;
}
EXPORT_SYMBOL(kona_slavetimer_get_counter);

/* Local static functions */
static struct kona_timer_module *__get_timer_module(char *name)
{
	int i;

	for (i = 0; i < NUM_OF_TIMER_MODULES; i++)
		if (!strcmp(name, timer_module_list[i].name))
			return &timer_module_list[i];
	return 0;
}

static int __config_slave_timer_clock(struct kona_timer_module *pktm,
				      unsigned int rate, unsigned int reg_val)
{
	void __iomem *slaveClockMgr_regs = IOMEM(KONA_SLV_CLK_VA);
#ifdef CONFIG_PERIPHERAL_TIMER_FIX
	void __iomem *rootClockMgr_regs = IOMEM(KONA_ROOT_CLK_VA);
#endif
	uint32_t old_enable;
	uint32_t val, mask, temp = 0;
#ifdef CONFIG_HAVE_CLK
	struct clk *clk;
#endif
	unsigned long flags;

/*
 * Protect the clock APIs in the CONFIG_HAVE_CLK macro.
 * In case if the platform does not support clock manager we should still be
 * able to boot up.
 *
 * The other point to handle is that, clock manager is initialized during
 * "early_init" stage. But the system timer initialization happens before
 * that. So if the clk mgr call fails we'll configure the clocks locally.
 * If not we'll call the clk mgr APIs.
 */
#ifdef CONFIG_HAVE_CLK
	clk = clk_get(NULL, pktm->clk_name);
	if (IS_ERR_OR_NULL(clk)) {
		pr_err
		    ("clk_get failed, so clock manager is not up use"
		    " local calls\n");
		goto local_clk_cfg;
	}
	clk_set_rate(clk, rate);
	return clk_enable(clk);
#endif
local_clk_cfg:
	spin_lock_irqsave(&pktm->lock, flags);
	/* Adjust clock source to 1Mhz */
	/* unlock slave clock manager */
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = val & 0x1;
	val &= 0x80000000;
	val |= 0xA5A500 | 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/* set the value */
	mask = KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs +
			   KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, mask,
			   KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
			   reg_val);

	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);
	writel(val | (1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT),
	       slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);

	do {
		temp = readl(slaveClockMgr_regs +
			KPS_CLK_MGR_REG_DIV_TRIG_OFFSET) &
	       (1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT) ;
		} while (temp);

	/* restore slave clock manager */
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	val &= 0x80000000;
	val |= 0xA5A500;
	val |= old_enable & 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);

#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
	/*
	 * This fix is only for 1 MHz since the clock source for
	 * 1MHz is from frac_1M. The 32KHz clock is from PMU and is always ON.
	 */
	if (reg_val == 1) {
		/* unlock root clock manager */
		val =
		    readl(rootClockMgr_regs +
			  ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
		old_enable = val & 0x1;
		val &= 0x80000000;
		val |= 0xA5A500 | 0x1;
		writel(val,
		       rootClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);

		/* fix up frac 1m divisor */
		val =
		    readl(rootClockMgr_regs +
			  ROOT_CLK_MGR_REG_FRAC_1M_DIV_OFFSET);
		writel((val & (~ROOT_CLK_MGR_REG_FRAC_1M_DIV_FRAC_1M_DIV_MASK))
		       | 0x250000,
		       rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_DIV_OFFSET);

		/* enable trigger override */
		val =
		    readl(rootClockMgr_regs +
			  ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_OFFSET);
		writel(val |
		       (1 <<
			ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_FRAC_1M_TRIGGER_OVERRIDE_SHIFT),
		       rootClockMgr_regs +
		       ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_OFFSET);

		/* trigger */
		val =
		    readl(rootClockMgr_regs +
			  ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET);
		writel(val |
		       (1 <<
			ROOT_CLK_MGR_REG_FRAC_1M_TRG_FRAC_1M_TRIGGER_SHIFT),
		       rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET);

		/* wait until completes */
		do {
			temp = readl(rootClockMgr_regs +
			ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET) & (1 <<
			ROOT_CLK_MGR_REG_FRAC_1M_TRG_FRAC_1M_TRIGGER_SHIFT) ;
			} while (temp);

		/* disable access to root clock manager */
		val =
		    readl(rootClockMgr_regs +
			  ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
		val &= 0x80000000;
		val |= 0xA5A500;
		writel(val,
		       slaveClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
	}
#endif
	spin_unlock_irqrestore(&pktm->lock, flags);
	return 0;
}

static int __config_aon_hub_timer_clock(struct kona_timer_module *pktm,
					unsigned int rate, unsigned int val)
{
	/*
	 * The source of all the 3 clocks 32KHz, 1MHz and 19.5MHz
	 * are ON by default (PPL0 (19.5) is configured by the boot loader
	 * 32KHz is from PMU and 1MHz (Frac_1M) is from crystal clock
	 * so all are ON
	 */
	void __iomem *reg_base = IOMEM(KONA_AON_CLK_VA);
	unsigned long reg_val, old_enable, temp = 0;
#ifdef CONFIG_HAVE_CLK
	struct clk *clk;
#endif
	unsigned long flags;

/*
 * Protect the clock APIs in the CONFIG_HAVE_CLK macro.
 * In case if the platform does not support clock manager we should still be
 * able to boot up.
 *
 * The other point to handle is that, clock manager is initialized during
 * "early_init" stage. But the system timer initialization happens before
 * that. So if the clk mgr call fails we'll configure the clocks locally.
 * If not we'll call the clk mgr APIs.
 */
#ifdef CONFIG_HAVE_CLK
	clk = clk_get(NULL, pktm->clk_name);
	if (IS_ERR_OR_NULL(clk)) {
		pr_err(
		  "clk_get failed, so clock manager is not up use local calls \r\n");
		goto local_clk_cfg;
	}
	clk_set_rate(clk, rate);
	return clk_enable(clk);
#endif

local_clk_cfg:
	spin_lock_irqsave(&pktm->lock, flags);

	/* unlock root clock manager */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = reg_val & 0x1;
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500 | 0x1;
	writel(reg_val, reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);

	reg_val = readl(KONA_AON_RST_VA +
			KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);
	reg_val = (reg_val) & ~(1 <<
		KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_SHIFT);
	writel(reg_val,
		KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);
	reg_val =
		readl(KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);
	reg_val = (reg_val) | (1 <<
		KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_SHIFT);
	writel(reg_val,
		KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	/*
	 * Assumed that the timer rate enum has
	 * 32KHz - 0, 1MHz - 1 and 19.5MHz - 2
	 */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);
	reg_val &= ~KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK;
	reg_val |= val;
	writel(reg_val, reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);

	/*
	 * Changing the HUB_TIMER source does not result in resetting the
	 * counter the first read of the counter reflects the time from the
	 * starting of uboot. So, reset the counter so that next read
	 * indicates the start of Linux
	 */
	reg_val = readl(KONA_AON_RST_VA);
	old_enable = reg_val & 0x1;
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500 | 0x1;
	writel(reg_val, KONA_AON_RST_VA);

	reg_val =
	    readl(KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);
	reg_val =
	    (reg_val) & ~(1 <<
		KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_SHIFT);
	writel(reg_val,
	       KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	reg_val =
	    readl(KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);
	reg_val =
	    (reg_val) | (1 <<
		KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_SHIFT);
	writel(reg_val,
	       KONA_AON_RST_VA + KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	reg_val = readl(KONA_AON_RST_VA);
	old_enable = reg_val & 0x1;
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500;
	reg_val |= old_enable & 0x1;
	writel(reg_val, KONA_AON_RST_VA);

	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET);
	writel(reg_val |
	       (1 <<
		KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_HUB_TIMER_TRIGGER_SHIFT),
	       reg_base + KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET);

	do {
		temp = readl(reg_base +
			KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET) & (1 <<
		KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_HUB_TIMER_TRIGGER_SHIFT) ;
		} while (temp);

	/* disable access to root clock manager */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500;
	reg_val |= old_enable & 0x1;
	writel(reg_val, reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);

	spin_unlock_irqrestore(&pktm->lock, flags);
	return 0;
}

static inline void __disable_all_channels(void __iomem *reg_base)
{
	/* Clear compare (0) interrupt */
	writel(0, reg_base + KONA_GPTIMER_STCS_OFFSET);
}

static inline void __disable_channel(void __iomem *reg_base, int ch_num)
{
	int reg;

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	/* if already disabled, do nothing */
	if (!__wait_for_compare_enable_sync(reg_base, ch_num))
		return;
#endif

	reg = readl(reg_base + KONA_GPTIMER_STCS_OFFSET);
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= 1 << (ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	reg &= ~(1 << (ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, reg_base + KONA_GPTIMER_STCS_OFFSET);

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	__wait_for_compare_enable_sync(reg_base, ch_num);
#endif
}

static inline unsigned long notrace __get_counter(void __iomem *reg_base)
{
#define KONA_MAX_REPEAT_TIMES 100
#define KONA_MAX_COUNTER_DIFF 1
	unsigned long prev;
	unsigned long cur, read_count;

#ifdef CONFIG_MACH_BCM_FPGA_E
	if (ktm == NULL)
		ktm = &timer_module_list[0];
	return readl(ktm->reg_base + KONA_GPTIMER_STCLO_OFFSET);
#endif

	read_count = 0;
	prev = readl(reg_base + KONA_GPTIMER_STCLO_OFFSET);
	do {
		cur = readl(reg_base + KONA_GPTIMER_STCLO_OFFSET);

		/*
		 * Only perform double read optimization for slave-
		 * timer
		 */
		if (reg_base == IOMEM(KONA_SYSTMR_VA)) {
			if (cur - prev > KONA_MAX_COUNTER_DIFF)
				prev = cur;
			else
				break;
		} else {
			if (cur != prev)
				prev = cur;
			else
				break;
		}

		if (read_count++ > KONA_MAX_REPEAT_TIMES)
			break;
	} while (1);

	BUG_ON(read_count > KONA_MAX_REPEAT_TIMES);

	return cur;
}

static inline int irq_to_ch(int irq)
{
	switch (irq) {
	case BCM_INT_ID_HUB_TIMERS1:
	case BCM_INT_ID_PERIPH_TIMERS1:
	case BCM_INT_ID_CORE_TIMERS1:
		return 0;
	case BCM_INT_ID_HUB_TIMERS2:
	case BCM_INT_ID_PERIPH_TIMERS2:
	case BCM_INT_ID_CORE_TIMERS2:
		return 1;
	case BCM_INT_ID_HUB_TIMERS3:
	case BCM_INT_ID_PERIPH_TIMERS3:
	case BCM_INT_ID_CORE_TIMERS3:
		return 2;
	case BCM_INT_ID_HUB_TIMERS4:
	case BCM_INT_ID_PERIPH_TIMERS4:
	case BCM_INT_ID_CORE_TIMERS4:
		return 3;
	}
	return -1;
}

/*
 * In case the interrupt is for a channel that has to count
 * periodically, re-program the registers accordingly.
 *
 * Assuming that we have programmed the match register with a count say 'n'.
 * Once the counter register (STCLO and STCHI) matches 'n' an interrupt will
 * be raised. Note that the timer would continue to count after raising the
 * interrupt. The next interrupt should happen at 2n. So the value n should
 * be added to the STCMx register and programmed. (The Programmer's guide
 * would say add 'n' to counter and re-program but doing this would cause
 * clock skew since the timer continues to count even after raising the
 * interrupt (the time taken to service this interrupt would be the skew then).
 *
 * Handling Roll Over conditions
 * -----------------------------
 * 1) The counter register is 64 bit wide
 * 2) The match register is compared against the 32 least significant bits of
 *     the free running counter values
 *
 * In this case the roll over condition is when the LSB 32 bits counts till
 * 0xFFFFFFFF and then becomes 0 (since the counter is 64 bit the MSB will
 * be counting but its immaterial since the comparison happens only with the
 * LSB 32 bits). Now while re-programming the match register the addition
 * operation will cause the same overflow which will be matched next time
 * properly when the LSB matches this new value.
 */
static irqreturn_t kona_timer_isr(int irq, void *dev_id)
{
	struct kona_timer_module *ktm;
	struct kona_timer *kt;
	int ch_num;
	unsigned long flags;
	unsigned long reg, now;
#ifdef CONFIG_KONA_TIMER_DEBUG
	unsigned long delta;
#endif

	ktm = (struct kona_timer_module *)dev_id;
	ch_num = irq_to_ch(irq);

	if ((ktm == NULL) || (ch_num == -1)) {
		pr_err("kona_timer_isr: Invalid dev_id or irq \r\n");
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&ktm->lock, flags);

	kt = ktm->pkt + ch_num;

	/* make sure that we do not service a wrong interrupt */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	if (!(reg &
		(1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT)))) {
#ifdef CONFIG_KONA_TIMER_DEBUG
		kt->nr_wrong_interrupt++;
#endif
		spin_unlock_irqrestore(&ktm->lock, flags);
		goto ret;
	}

	now = __get_counter(ktm->reg_base);

	/* early expire */
	if (time_before(now, kt->expire)) {
#ifdef CONFIG_KONA_TIMER_DEBUG
		kt->nr_early_expire++;
#endif
		/*
		 * Recover once early timer expiration occurs. It's rare case
		 * and add workaround to improve timer performance by removing
		 * disabling channel in re-programming timer value
		 */
		__disable_channel(ktm->reg_base, kt->ch_num);
		spin_unlock_irqrestore(&ktm->lock, flags);
		kona_timer_set_match_start(kt, kt->expire - now);
		goto ret;
	}
#ifdef CONFIG_KONA_TIMER_DEBUG
	else {
		delta = now - kt->expire;
		if (delta <= 5)
			kt->nr_5++;
		else if (delta <= 10)
			kt->nr_10++;
		else if (delta <= 50)
			kt->nr_50++;
		else if (delta <= 100)
			kt->nr_100++;
		else if (delta <= 500)
			kt->nr_500++;
		else
			kt->nr_500_plus++;
	}
#endif

	/* clear and disable the interrupt */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	/*
	 * Clean up the Match field (3..0), writing 1 to this bit
	 * would clean the interrupt for the respective channel.
	 * So do this for only the required channel.
	 */
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= 1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	reg &= ~(1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	if (kt->cfg.mode == MODE_PERIODIC) {
		/*
		 * We need to re-program the timer with ktm->cfg.reload
		 * Since the sequence to program the next delta is
		 * very sensitive and is already done in
		 * kona_timer_set_match_start, just call it from here.
		 */
		__disable_channel(ktm->reg_base, kt->ch_num);

		spin_unlock_irqrestore(&ktm->lock, flags);
		kona_timer_set_match_start(kt, kt->cfg.reload);
		goto cb_ret;
	}

	spin_unlock_irqrestore(&ktm->lock, flags);

cb_ret:
	/* Invoke the call back, if any */
	if (kt->cfg.cb != NULL)
		(*kt->cfg.cb) (kt->cfg.arg);
ret:
	return IRQ_HANDLED;
}
