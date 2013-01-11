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
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <mach/kona_timer.h>
#include <mach/io.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#if defined(CONFIG_ARCH_RHEA) || defined(CONFIG_ARCH_CAPRI) ||  defined(CONFIG_ARCH_HAWAII)
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#endif
#else
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#endif

#ifdef CONFIG_KONA_TIMER_DEBUG
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/broadcom/knllog.h>
#include <linux/hrtimer.h>
#endif
#include <linux/io.h>

#define MAX_COMPARE_LOAD_UPDATE_WAIT_CYCLE  20

/* Macros to set/get multiple register bits at one time */
#define kona_set_reg_field(addr, mask, shift, val)	\
	do {						\
		uint32_t tmp;				\
		tmp  = readl(addr);			\
		tmp &= ~(mask);				\
		tmp |= (((val) << (shift)) & (mask));	\
		writel(tmp, addr);			\
	} while (0)

static struct kona_timer aon_hub_timer[NUM_OF_CHANNELS] = {
	{.ch_num = 0, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS1},
	{.ch_num = 1, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS2},
	{.ch_num = 2, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS3},
	{.ch_num = 3, .busy = 0, .irq = BCM_INT_ID_HUB_TIMERS4},
};

static struct kona_timer periph_timer[NUM_OF_CHANNELS] = {
	{.ch_num = 0, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS1},
	{.ch_num = 1, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS2},
	{.ch_num = 2, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS3},
	{.ch_num = 3, .busy = 0, .irq = BCM_INT_ID_PERIPH_TIMERS4},
};

/* There are two instances of timer modules on Kona */
static struct kona_timer_module timer_module_list[NUM_OF_TIMER_MODULES] = {
	{.pkt = &aon_hub_timer[0], .name = "aon-timer",
	 .cfg_state = NOT_CONFIGURED, .rate = 0,
	 .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_TMR_HUB_VA),
	 .clk_name = "hub_timer_clk",
#ifdef CONFIG_KONA_TIMER_DEBUG
	 .max_repeat_count = 0
#endif
	},
	{.pkt = &periph_timer[0], .name = "slave-timer",
	 .cfg_state = NOT_CONFIGURED, .rate = 0,
	 .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_SYSTMR_VA),
	 .clk_name = "timers_clk",
#ifdef CONFIG_KONA_TIMER_DEBUG
	 .max_repeat_count = 0
#endif
	},
};

/* Local static functions */
static struct kona_timer_module *__get_timer_module(char *name);
static int __config_aon_hub_timer_clock(unsigned int rt);
static int __config_slave_timer_clock(unsigned int rt);
static inline void __disable_all_channels(void __iomem *reg_base);
static inline void __disable_channel(struct kona_timer *kt);
static inline unsigned long __get_counter(struct kona_timer_module *ktm);
static irqreturn_t kona_timer_isr(int irq, void *dev_id);
static inline int __is_channel_enabled(struct kona_timer_module *ktm,
					int ch_num);
static inline int __is_interrupt_generated(struct kona_timer_module *ktm,
					int ch_num);
#ifdef CONFIG_HAVE_CLK
static int __config_aon_hub_timer_have_clock(unsigned int rt);
static int __config_slave_timer_have_clock(unsigned int rt);
#endif

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

static int kona_init_done;

struct kona_timer *get_timer_ptr(char *name, int chan)
{
	struct kona_timer_module *ktm = NULL;

	ktm = __get_timer_module(name);
	return ktm->pkt + chan;
}

/*
 *  kona_timer_modules_init - Initialize the data structures
 *  that depicts the Kona timer modules
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

	kona_init_done = 1;
	return 0;
}

/*
 * kona_timer_module_set_rate - Set the speed at which a timer module
 * should count
 * name - Name of the Timer to configure
 * rate - Speed
 */
int kona_timer_module_set_rate(char *name, unsigned int rate)
{
	struct kona_timer_module *pktm = NULL;
	int ret = -1;
	unsigned long flags;
	unsigned int reg = 0;

	pktm = __get_timer_module(name);
	if ((pktm == NULL) || (pktm->cfg_state == CONFIGURED_BUSY))
		return -1;

	/*
	 * Based on the timer name and the speed requested configure the
	 * appropriate clock registers.
	 * This implementation should be changed to call the clock APIs
	 * once they are up.
	 */
#ifdef CONFIG_HAVE_CLK
	if (pktm->reg_base == IOMEM(KONA_SYSTMR_VA))
		ret = __config_slave_timer_have_clock(rate);
	else
		ret = __config_aon_hub_timer_have_clock(rate);
#endif

	spin_lock_irqsave(&pktm->lock, flags);
	if (ret < 0) {
		if (pktm->reg_base == IOMEM(KONA_SYSTMR_VA))
			ret = __config_slave_timer_clock(rate);
		else
			ret = __config_aon_hub_timer_clock(rate);
	}

#if defined(CONFIG_ARCH_RHEA) || defined(CONFIG_ARCH_HAWAII)
	/* Configure KONA Timer count in 32 bit mode */
	writel(reg, KONA_CHIPREG_VA + CHIPREG_HUB_TIMER_WIDTH_OFFSET);
#else
	/* Use 32bit mode timer */
	reg = readl(KONA_CHIPREG_VA + CHIPREG_ARM_PERI_CONTROL_OFFSET);
	reg &= ~(CHIPREG_ARM_PERI_CONTROL_SLV_TIMER_64BITMODE_MASK |
		CHIPREG_ARM_PERI_CONTROL_HUB_TIMER_64BITMODE_MASK);
	writel(reg, KONA_CHIPREG_VA + CHIPREG_ARM_PERI_CONTROL_OFFSET);
#endif

	if (ret != -1) {
		pktm->rate = rate;
		pktm->cfg_state = CONFIGURED_FREE;
	}

	spin_unlock_irqrestore(&pktm->lock, flags);
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

	if (NULL == name || channel >= NUM_OF_CHANNELS)
		return pkt;

	pktm = __get_timer_module(name);
	if (NULL == pktm)
		return pkt;

	if (channel >= pktm->num_of_timers)
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
static inline int __wait_for_timer_sync(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	int enabled;

	ktm = kt->ktm;

	if (readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
	   (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT))) {

		/* Make sure previous timer load was synced and not been used.
		 * Poll the corresponding STCS bits to become 0, not in use.
		 * This is to make sure the next event tick value is actually
		 * loaded (3 32KHz clock cycles) after finishing previous
		 * timer value syncing, before loading new value.
		 */
		 while (readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
		       (1 << (KONA_GPTIMER_STCS_STCM0_SYNC_SHIFT + kt->ch_num)))
			;

		/* NOTE: compare enable sync bit, wait for the sync bit
		 * until it's changed from low to high. It's going to take
		 * 2 clock edge (30us ~ 60us on 32khz).
		 * Default value on POR is 0.
		 */
		while (!(readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
			 (1 << (KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT +
				kt->ch_num))))
			;

		enabled = 1;
	} else {
	       /* NOTE: compare enable sync bit is changed from high to low
		* after 2 clock edge when compare enable bit is disabled.
		*/
		while (readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
			(1 << (KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT +
					kt->ch_num)))
			;
		enabled = 0;
	}

	return enabled;
}

static inline void __wait_for_timer_disable(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;

	ktm = kt->ktm;

	if (!(readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
	  (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT)))) {

		/* NOTE: compare enable sync bit is changed from high to low
		 * after 2 clock edge when compare enable bit is disabled.
		 */
		while (readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
			 (1 << (KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT +
				kt->ch_num)))
			;
	}
}
#endif

/*
 * kona_timer_set_match_start - Set the match register for the timer and start
 * counting
 *
 *  kt - Kona timer context (returned by kona_timer_request())
 *  load - The load value to be programmed. This function will internally
 *         add this value to the current counter and program the resultant in
 *         the match register. Once the timer is started when the counter
 *         reaches this value an interrupt will be raised
 */
int kona_timer_set_match_start(struct kona_timer *kt, unsigned long load)
{
	struct kona_timer_module *ktm;
	unsigned long flags;
	unsigned long reg, adj_load, now;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	/* Correct delta load clock, guarantee the minimum requested time */
	if (load < MIN_KONA_DELTA_CLOCK)
		load = MIN_KONA_DELTA_CLOCK;

	/* Adjust the minimum timer value by adding 1. Free running timer
	 * reference can be increased in worst case and guarantee timer
	 * expiration.
	 */
	if (load < MAX_KONA_COUNT_CLOCK)
		adj_load = load + 1;
	else
		adj_load = MAX_KONA_COUNT_CLOCK;

	spin_lock_irqsave(&ktm->lock, flags);

#ifdef CONFIG_KONA_TIMER_DEBUG
	kt->nr_total++;

	if (__is_channel_enabled(ktm, kt->ch_num)) {
		now = __get_counter(ktm);
		kt->nr_canceled++;

		if (time_after(now, kt->expire) && (kt->load != 0)) {
			int intr = __is_interrupt_generated(ktm, kt->ch_num);
			kt->nr_canceled_expired++;

			if (intr)
				kt->nr_canceled_expired_intr++;

			KNLLOG("=> delta:%lu, now:%lu, expire:%lu, load:%lu"
				" - int:%d\n",
				now - kt->expire,
				now, kt->expire, kt->load, intr);
		}
	}
#endif

	__disable_channel(kt);

	/*
	 * Remember the reload value in case of periodic timers.
	 * Note that the HW does not support periodic timers. Once
	 * there is a match the interrupt is triggered and from the
	 * ISR we have to re-program the match register to implement
	 * periodic ISR.
	 */
	kt->cfg.reload = adj_load;

#ifdef CONFIG_KONA_TIMER_DEBUG
	kt->load = adj_load;
#endif

	/* Load the match register */
	now = __get_counter(ktm);
	kt->expire = adj_load + now;
	writel(kt->expire,
	       ktm->reg_base + KONA_GPTIMER_STCM0_OFFSET + (kt->ch_num * 4));

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	/* Make sure new load value to be synced so that early timer
	 * expiration by previous timer can be ignored by cleaning
	 * the timer match bit up on next. Only takes care of possible
	 * cases in enabled state.
	 */
	__wait_for_timer_sync(kt);
#endif

	/*
	 * Enable timer compare register. Clean up the Match field (3..0),
	 * writing 1 to this bit would clean the interrupt for the respective
	 * channel. So do this for only the required channel.
	 */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

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

	__disable_channel(kt);

	spin_unlock_irqrestore(&ktm->lock, flags);
	return 0;
}
EXPORT_SYMBOL(kona_timer_stop);

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

#ifdef CONFIG_HAVE_CLK
	clk = clk_get(NULL, ktm->clk_name);
#endif

	spin_lock_irqsave(&ktm->lock, flags);

	__disable_channel(kt);
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
#ifdef CONFIG_HAVE_CLK
		if (IS_ERR(clk))
			pr_err("timer_free: clk_get failed, so clock manager"
			       " is not up use local calls\r\n");
		else
			clk_disable(clk);
#endif
		ktm->cfg_state = CONFIGURED_FREE;
	}

	spin_unlock_irqrestore(&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_free);

/*
 * kona_timer_get_counter - Read the counter register of the timer
 *
 * kt - Timer context to be freed.
 */
unsigned int kona_timer_get_counter(struct kona_timer *kt)
{
	struct kona_timer_module *ktm;
	unsigned int counter;
	unsigned long flags;

	if (NULL == kt) {
		pr_err("%s timer object cannot be NULL\n", __func__);
		BUG();
	}

	ktm = kt->ktm;
	local_irq_save(flags);
	counter = __get_counter(ktm);
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
	counter = __get_counter(&timer_module_list[0]);
	local_irq_restore(flags);

	return counter;
}
EXPORT_SYMBOL(kona_hubtimer_get_counter);

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
static struct kona_timer_regs kona_timer_state;
void kona_hubtimer_save_state(bool print_state)
{
	unsigned long flags;
	struct kona_timer_module *ktm = &timer_module_list[0];

	spin_lock_irqsave(&ktm->lock, flags);
	kona_timer_state.stcs = readl(ktm->reg_base +
		KONA_GPTIMER_STCS_OFFSET);
	kona_timer_state.stclo = readl(ktm->reg_base +
		KONA_GPTIMER_STCLO_OFFSET);
	kona_timer_state.stchi = readl(ktm->reg_base +
		KONA_GPTIMER_STCHI_OFFSET);
	kona_timer_state.stcm0 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM0_OFFSET);
	kona_timer_state.stcm1 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM1_OFFSET);
	kona_timer_state.stcm2 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM2_OFFSET);
	kona_timer_state.stcm3 = readl(ktm->reg_base +
		KONA_GPTIMER_STCM3_OFFSET);
	spin_unlock_irqrestore(&ktm->lock, flags);

	if (print_state) {
		printk(KERN_ERR "Kona Timer: STCS =%08x\n",
				kona_timer_state.stcs);
		printk(KERN_ERR "Kona Timer: STCLO=%08x\n",
				kona_timer_state.stclo);
		printk(KERN_ERR "Kona Timer: STCHI=%08x\n",
				kona_timer_state.stchi);
		printk(KERN_ERR "Kona Timer: STCM0=%08x\n",
				kona_timer_state.stcm0);
		printk(KERN_ERR "Kona Timer: STCM1=%08x\n",
				kona_timer_state.stcm1);
		printk(KERN_ERR "Kona Timer: STCM2=%08x\n",
				kona_timer_state.stcm2);
		printk(KERN_ERR "Kona Timer: STCM3=%08x\n",
				kona_timer_state.stcm3);
	}
}
EXPORT_SYMBOL(kona_hubtimer_save_state);

#ifdef CONFIG_KONA_TIMER_DEBUG
struct kona_timer_module *kona_get_timer_module(char *name)
{
	return __get_timer_module(name);
}
#endif


/* Local static functions */
static struct kona_timer_module *__get_timer_module(char *name)
{
	int i;

	for (i = 0; i < NUM_OF_TIMER_MODULES; i++)
		if (!strcmp(name, timer_module_list[i].name))
			return &timer_module_list[i];
	return NULL;
}

#ifdef CONFIG_HAVE_CLK
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
static int __config_slave_timer_have_clock(unsigned int rt)
{
	struct clk *clk;

	clk = clk_get(NULL, "timers_apb_clk");
	if (IS_ERR(clk)) {
		pr_err("clk_get failed, so clock manager is not up"
		       " use local calls\r\n");
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	clk = clk_get(NULL, "timers_clk");
	if (IS_ERR(clk)) {
		pr_err("clk_get failed, so clock manager is not up"
		       " use local calls\r\n");
		return PTR_ERR(clk);
	}
	clk_set_rate(clk, rt);
	clk_enable(clk);
	return 0;
}
#endif

static int __config_slave_timer_clock(unsigned int rt)
{
	void __iomem *slaveClockMgr_regs = IOMEM(KONA_SLV_CLK_VA);
#ifdef CONFIG_PERIPHERAL_TIMER_FIX
	void __iomem *rootClockMgr_regs = IOMEM(KONA_ROOT_CLK_VA);
#endif
#if !defined(CONFIG_ARCH_SAMOA) || \
	defined(CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY)
	uint32_t old_enable;
#endif
	uint32_t val, mask, rate_val;

	/* Slave timer only supports 32KHz and 1MHz */
	if (rt == 32768)
		rate_val = 1;
#if defined(CONFIG_ANDROID) && defined(CONFIG_MACH_CAPRI_FPGA)
/* Brief - to let platform to initialize TICK rate for special case
More explanations in plat-kona/include/mach/timex.h*/
	else if (rt >= 1000000)
#else
	else if (rt == 1000000)
#endif
		rate_val = 0;
	else
		return -1;

	/* Adjust clock source to 1Mhz */
#if defined(CONFIG_ARCH_RHEA) || defined(CONFIG_ARCH_CAPRI) || defined(CONFIG_ARCH_HAWAII)
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
			   rate_val);

	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);
	writel(val | (1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT),
	       slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);

	while (readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET) &
	       (1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT))
		;

	/* restore slave clock manager */
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	val &= 0x80000000;
	val |= 0xA5A500;
	val |= old_enable & 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
#else
#ifdef CONFIG_ARCH_SAMOA
	/* unlock slave clock manager */
#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
#define KPS_CLK_MGR_REG_WR_ACCESS_OFFSET 0
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = val & 0x1;
	val &= 0x80000000;
	val |= 0xA5A500 | 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
#else
	/* Samoa kps_clk access control is different ... */
#endif

	/* set the value */
	mask = KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs +
			   KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, mask,
			   KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
			   0);

	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_KPS_DIV_TRIG_OFFSET);
	writel(val | (1 << KPS_CLK_MGR_REG_KPS_DIV_TRIG_TIMERS_TRIGGER_SHIFT),
	       slaveClockMgr_regs + KPS_CLK_MGR_REG_KPS_DIV_TRIG_OFFSET);

	while (readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_KPS_DIV_TRIG_OFFSET) &
	       (1 << KPS_CLK_MGR_REG_KPS_DIV_TRIG_TIMERS_TRIGGER_SHIFT))
		;

	/* restore slave clock manager */
#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
#else
	/* Samoa ... */
#endif
#else
	/* unlock slave clock manager */
	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = val & 0x1;
	val &= 0x80000000;
	val |= 0xA5A500 | 0x1;
	writel(val, slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/* set the value */
	mask = IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs +
			   IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, mask,
			   IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
			   0);

	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET);
	writel(val | (1 << IKPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT),
	       slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET);

	while (readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET) &
	       (1 << IKPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT))
		;

	/* restore slave clock manager */
	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	val &= 0x80000000;
	val |= 0xA5A500;
	val |= old_enable & 0x1;
	writel(val, slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
#endif
#endif

#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
	/*
	 * This fix is only for 1 MHz since the clock source for
	 * 1MHz is from frac_1M. The 32KHz clock is from PMU and is always ON.
	 */
#ifndef CONFIG_ARCH_SAMOA
	if (rate_val == 1) {
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
		while (readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET) &
			(1 << ROOT_CLK_MGR_REG_FRAC_1M_TRG_FRAC_1M_TRIGGER_SHIFT))
			;

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
#endif
	return 0;
}

#ifdef CONFIG_HAVE_CLK
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
static int __config_aon_hub_timer_have_clock(unsigned int rt)
{
	struct clk *clk;

	clk = clk_get(NULL, "hub_timer_apb_clk");
	if (IS_ERR(clk)) {
		pr_err("clk_get failed, so clock manager is not up use"
		       " local calls\r\n");
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	clk = clk_get(NULL, "hub_timer_clk");
	if (IS_ERR(clk)) {
		pr_err("clk_get failed, so clock manager is not up use"
		       " local calls\r\n");
		return PTR_ERR(clk);
	}
	clk_set_rate(clk, rt);
	clk_enable(clk);
	return 0;
}
#endif

static int __config_aon_hub_timer_clock(unsigned int rt)
{
	/*
	 * The source of all the 3 clocks 32KHz, 1MHz and 19.5MHz
	 * are ON by default (PPL0 (19.5) is configured by the boot loader
	 * 32KHz is from PMU and 1MHz (Frac_1M) is from crystal clock
	 * so all are ON
	 */
	void __iomem *reg_base = IOMEM(KONA_AON_CLK_VA);
	unsigned long val, reg_val, old_enable;

	/* AON timer only supports 32KHz, 1MHz and 19.5 MHz */
	switch (rt) {
	case 32768:
		/* 32 KHz */
		val = 0;
		break;
	case 1000000:
		/* 1 MHz */
		val = 1;
		break;
	case 19500000:
		/* 19.5 MHz */
		val = 2;
		break;
	default:
#if defined(CONFIG_ANDROID) && defined(CONFIG_MACH_CAPRI_FPGA)
/* Brief - to let platform to initialize TICK rate for special case
   More explanations in plat-kona/include/mach/timex.h*/
		if (rt >= 1000000) {
			val = 1;
		} else
#endif
		{
			pr_err("aon hub timer clock: Invalid value"
			       " requested %d\r\n",
			       rt);
			return -1;
		}
	}

	/* unlock root clock manager */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = reg_val & 0x1;
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500 | 0x1;
	writel(reg_val, reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/*
	 * Assumed that the timer rate enum has
	 * 32KHz - 0, 1MHz - 1 and 19.5MHz - 2
	 */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);
	reg_val &= ~KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK;
	reg_val |= val;
	writel(reg_val, reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);

	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET);
	writel(val |
	       (1 <<
		KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_HUB_TIMER_TRIGGER_SHIFT),
	       reg_base + KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET);

	while (readl(reg_base + KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET) &
	       (1 <<
		KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_HUB_TIMER_TRIGGER_SHIFT))
		;

	/* disable access to root clock manager */
	reg_val = readl(reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);
	reg_val &= 0x80000000;
	reg_val |= 0xA5A500;
	reg_val |= old_enable & 0x1;
	writel(val, reg_base + KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET);

	return 0;
}

static inline void __disable_all_channels(void __iomem *reg_base)
{
	/* Clear compare (0) interrupt */
	writel(0, reg_base + KONA_GPTIMER_STCS_OFFSET);
}

static inline void __disable_channel(struct kona_timer *kt)
{
	int reg;
	struct kona_timer_module *ktm;

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	if (!__wait_for_timer_sync(kt))
		return;
#endif
	ktm = kt->ktm;

	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= 1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	reg &= ~(1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	__wait_for_timer_sync(kt);
#endif
}

static inline unsigned long __get_counter(struct kona_timer_module *ktm)
{
#ifdef CONFIG_MACH_HAWAII_FPGA_E
	if (ktm == NULL)
		ktm = &timer_module_list[0];
	return readl(ktm->reg_base + KONA_GPTIMER_STCLO_OFFSET);
#endif
#define KONA_MAX_REPEAT_TIMES	100
	unsigned long lsw;
	int i = 0;

	for (i = 0; i < KONA_MAX_REPEAT_TIMES; i++) {
		lsw = readl(ktm->reg_base +
			KONA_GPTIMER_STCLO_OFFSET);
		if (lsw == readl(ktm->reg_base + KONA_GPTIMER_STCLO_OFFSET))
			break;
	}

	BUG_ON(i >= KONA_MAX_REPEAT_TIMES);
#ifdef CONFIG_KONA_TIMER_DEBUG
	/*
	 * record the max repeat count
	 */
	if (i > ktm->max_repeat_count)
		ktm->max_repeat_count = i;
#endif
	return lsw;
}

static inline int __is_channel_enabled(struct kona_timer_module *ktm,
					int ch_num)
{
	return readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
		(1 << (ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
}

static inline int __is_interrupt_generated(struct kona_timer_module *ktm,
						int ch_num)
{
	return readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) &
		(1 << (ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT));
}

static inline int irq_to_ch(int irq)
{
	switch (irq) {
	case BCM_INT_ID_HUB_TIMERS1:
	case BCM_INT_ID_PERIPH_TIMERS1:
		return 0;
	case BCM_INT_ID_HUB_TIMERS2:
	case BCM_INT_ID_PERIPH_TIMERS2:
		return 1;
	case BCM_INT_ID_HUB_TIMERS3:
	case BCM_INT_ID_PERIPH_TIMERS3:
		return 2;
	case BCM_INT_ID_HUB_TIMERS4:
	case BCM_INT_ID_PERIPH_TIMERS4:
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
 *    the free running counter values
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
	void __iomem *match_reg;
	unsigned long reg;
	unsigned long flags;
	unsigned long now;
#ifdef CONFIG_KONA_TIMER_DEBUG
	unsigned long delta;
#endif

	ktm = (struct kona_timer_module *)dev_id;
	ch_num = irq_to_ch(irq);

	if ((ktm == NULL) || (ch_num == -1)) {
		pr_err("kona_timer_isr: Invalid dev_id or irq\r\n");
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&ktm->lock, flags);

	kt = ktm->pkt + ch_num;

	/* Do not service wrong interrupt. It happens when timer is expired,
	 * interrupt is disabled at that time and new timer is requested.
	 */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	if (!(reg &
	    (1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT)))) {
#ifdef CONFIG_KONA_TIMER_DEBUG
		kt->nr_wrong_interrupt++;
#endif
		spin_unlock_irqrestore(&ktm->lock, flags);
		goto ret;
	}

	now = __get_counter(ktm);
	if (time_before(now, kt->expire)) {
#ifdef CONFIG_KONA_TIMER_DEBUG
		delta = kt->expire - now;
		kt->nr_early_expire++;
		if (delta > kt->max_delta_early_expire) {
			kt->max_delta_early_expire = delta;
			kt->max_delta_load_early_expire = kt->load;
		}
#endif
		/* Recover once early timer expiration occurs. It's rare case
		 * and add workaround to improve timer performance by removing
		 * disabling channel in re-programming timer value.
		 */
		__disable_channel(kt);
		spin_unlock_irqrestore(&ktm->lock, flags);
		kona_timer_set_match_start(kt, kt->expire - now);
		goto ret;
	}
#ifdef CONFIG_KONA_TIMER_DEBUG
	else {
		delta = now - kt->expire;
		if (delta > kt->max_delta) {
			kt->max_delta = delta;
			kt->max_delta_load = kt->load;
			kt->max_delta_expire = kt->expire;
			kt->max_delta_expired = now;
			KNLLOG("===>kona_timer_isr: delta:%ld - load:%d,"
				" expire:%d, now:%lu, handler:0x%p\n",
				delta, kt->load, kt->expire, now, kt->cfg.cb);
		}
	}
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

	kt->nr_timedout++;
#endif

	if (kt->cfg.mode == MODE_PERIODIC) {
		__disable_channel(kt);

		/* Now re-program the match register */
		match_reg =
		    ktm->reg_base + KONA_GPTIMER_STCM0_OFFSET +
		    (kt->ch_num * 4);
		writel(readl(match_reg) + kt->cfg.reload, match_reg);

		/* Re-enable the interrupt */
		reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
		reg |=
		    (1 <<
		     (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
		writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	} else {
		/* Disable timer w/o waiting for sync to remove the delay */
		__wait_for_timer_sync(kt);

		reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
		reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
		reg |= 1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
		reg &= ~(1 << (kt->ch_num +
				KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
		writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	}

	spin_unlock_irqrestore(&ktm->lock, flags);

	/* Invoke the call back, if any */
	if (kt->cfg.cb != NULL)
		(*kt->cfg.cb) (kt->cfg.arg);

ret:
	return IRQ_HANDLED;
}
