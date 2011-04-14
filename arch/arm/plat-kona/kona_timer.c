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

#include <mach/kona_timer.h>
#include <mach/io.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>

#ifdef CONFIG_ARCh_ISLAND
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#else
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#endif
#endif

#include <asm/io.h>

#define NUM_OF_TIMER_MODULES	(2)
#define NUM_OF_CHANNELS		(4)

/* Macros to set/get multiple register bits at one time */
#define kona_set_reg_field(addr, mask, shift, val)      \
            do                                              \
            {                                               \
               u32 tmp;                                \
               tmp  = readl(addr);              \
               tmp &= ~(mask);                              \
               tmp |= (((val) << (shift)) & (mask));        \
               writel(tmp, addr);                 \
                                                            \
            } while(0)

/* Data structures */
struct kona_timer {
	int    ch_num;
	int    busy;
	struct timer_ch_cfg cfg;
	int irq;
	struct kona_timer_module *ktm;
};

enum config_state {
	NOT_CONFIGURED = 0, /* Timer module not configured */
	CONFIGURED_FREE,    /* Timer module configured but
			     * channels are not being used so
			     * can change the rate 
			     */
	CONFIGURED_BUSY,    /* Timer module configured and channels
			     * are being used so re-configuring
			     * the rate is not allowed
			     */
};

struct kona_timer_module {
	struct kona_timer *pkt;
	spinlock_t lock;
	int	num_of_timers;
	char    name[255];
	enum config_state cfg_state;
	void __iomem *reg_base;
	enum timer_rate rate;
};

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
	{ .pkt = &aon_hub_timer[0], .name = "aon-timer", .cfg_state = NOT_CONFIGURED, 
	  .rate = 0, .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_TMR_HUB_VA) },
	{ .pkt = &periph_timer[0],  .name = "slave-timer", .cfg_state =	NOT_CONFIGURED,
	  .rate = 0, .num_of_timers = NUM_OF_CHANNELS, .reg_base = IOMEM(KONA_SYSTMR_VA)  },
};

/* Local static functions */
static struct kona_timer_module * __get_timer_module(char *name);
static int __config_aon_hub_timer_clock(unsigned int val);
static int __config_slave_timer_clock(enum timer_rate rt);
static inline void __disable_all_channels(void __iomem *reg_base);
static inline void __disable_channel(void __iomem *reg_base, int ch_num);
static inline void __get_counter (void __iomem *reg_base, unsigned long *msw,
	unsigned long *lsw);
static irqreturn_t kona_timer_isr(int irq, void *dev_id);

static struct irqaction hub_timer_irq = {
	.name		= "Kona Hub Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.dev_id		= &timer_module_list[0],
	.handler	= kona_timer_isr,
};

static struct irqaction periph_timer_irq = {
	.name		= "Kona Slave Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.dev_id		= &timer_module_list[1],
	.handler	= kona_timer_isr,
};

/*
 *  kona_timer_modules_init - Initialize the data structures
 *  that depcits the Kona timer modules
 */
void __init kona_timer_modules_init (void)
{
	int i;

	for (i=0;i<NUM_OF_TIMER_MODULES;i++) {
		spin_lock_init(&timer_module_list[i].lock);
		__disable_all_channels(timer_module_list[i].reg_base);
	}

	for (i=0;i<NUM_OF_CHANNELS;i++) {
		setup_irq(aon_hub_timer[i].irq, &hub_timer_irq);
		aon_hub_timer[i].cfg.arg = NULL;
		aon_hub_timer[i].cfg.mode = 0;
		aon_hub_timer[i].cfg.cb = NULL;
		aon_hub_timer[i].cfg.reload = 0;
		aon_hub_timer[i].ktm = &timer_module_list[0];
	}

	for (i=0;i<NUM_OF_CHANNELS;i++) {
		setup_irq(periph_timer[i].irq, &periph_timer_irq);
		periph_timer[i].cfg.arg = NULL;
		periph_timer[i].cfg.mode = 0;
		periph_timer[i].cfg.cb = NULL;
		periph_timer[i].cfg.reload = 0;
		periph_timer[i].ktm = &timer_module_list[1];
	}

	return;
}
EXPORT_SYMBOL(kona_timer_modules_init);

/*
 * kona_timer_module_set_rate - Set the speed in which a timer module should count
 * name - Name of the Timer to configure
 * rate - Speed 
 */
int kona_timer_module_set_rate(char* name, enum timer_rate rate)
{
	struct kona_timer_module *pktm = NULL;
	int ret;
	unsigned long flags;

	pktm = __get_timer_module(name);
	if ( (pktm == NULL) || (pktm->cfg_state == CONFIGURED_BUSY)) {
		return -1;
	}

	/* 
	 * Based on the timer name and the speed requested configure the 
	 * appropriate clock registers.
	 * This implementation should be changed to call the clock APIs
	 * once they are up.
	 */
	spin_lock_irqsave (&pktm->lock, flags);

	if (pktm->reg_base == IOMEM(KONA_SYSTMR_VA))
		ret = __config_slave_timer_clock(rate); 
	else
		ret = __config_aon_hub_timer_clock(rate);

	if (ret != -1) {
		pktm->rate = rate;
		pktm->cfg_state = CONFIGURED_FREE; 
	}

	spin_unlock_irqrestore (&pktm->lock, flags);
	return ret;
}
EXPORT_SYMBOL(kona_timer_module_set_rate);

/* 
 * kona_timer_module_get_rate - Get the speed in which a timer module is running
 * name - Name of the Timer module 
 */
int kona_timer_module_get_rate (char* name)
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
struct kona_timer* kona_timer_request(char* name, int channel)
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

	spin_lock_irqsave (&pktm->lock, flags);

	pkt = pktm->pkt;
	
	if (channel == -1) {
		for (i=0; i<NUM_OF_CHANNELS; i++,pkt++) {
			if (pkt->busy == 0)
				break;
		}
		if (i == NUM_OF_CHANNELS)
			pkt=NULL;
		else 
			pkt->busy = 1;
	} else {
		pkt += channel;
		if (pkt->busy)
			pkt=NULL;
		else 
			pkt->busy=1;
	}
	
	if(pkt != NULL)
		pktm->cfg_state = CONFIGURED_BUSY;

	spin_unlock_irqrestore (&pktm->lock, flags);

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
int kona_timer_config (struct kona_timer *kt, struct timer_ch_cfg *pcfg) 
{
	struct kona_timer_module *ktm;
	unsigned long flags;

	if ((NULL == kt) || (NULL == pcfg))
		return -1;

	ktm = kt->ktm;

	spin_lock_irqsave (&ktm->lock, flags);

	kt->cfg.arg = pcfg->arg;
	kt->cfg.mode = pcfg->mode;
	kt->cfg.cb  = pcfg->cb;

	spin_unlock_irqrestore (&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_config);

/*
 * kona_timer_set_match_start - Set the match register for the timer and start
 * counting
 *
 *  kt - Kona timer context (returned by kona_timer_request())
 *  load - The load value to be programmed. This function will internally
 *         add this value to the current counter and program the resultant in the
 *         match register. Once the timer is started when the counter 
 *         reaches this value an interrupt will be raised
 */
int kona_timer_set_match_start (struct kona_timer* kt, unsigned int load)
{
	struct kona_timer_module *ktm;
	unsigned long flags;
	unsigned long reg, msw, lsw;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave (&ktm->lock, flags);

	/* 
	 * Remember the reload value in case of periodic timers.
	 * Note that the HW does not support periodic timers. Once
	 * there is a match the interrupt is triggered and from the
	 * ISR we have to re-program the match register to implement
	 * periodic ISR.
	 */
	
	if (kt->cfg.mode == MODE_PERIODIC)
		kt->cfg.reload = load;

	__get_counter (ktm->reg_base, &msw, &lsw);

	/* Load the match register */
	writel(load+lsw, ktm->reg_base + KONA_GPTIMER_STCM0_OFFSET + (kt->ch_num * 4));

#ifdef CONFIG_GP_TIMER_COMPARATOR_LOAD_DELAY
	/* Poll the corresponding STCS bits to become 0.
	 * This is to make sure the next event tick value is actually loaded (taking 3 32KHz clock cycles)
	 * before enabling compare (taking 2 32KHz clock cycles). 
	 */
	while (readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET) & 
			(1 << (KONA_GPTIMER_STCS_STCM0_SYNC_SHIFT+kt->ch_num)))
		;
#endif

	/* Enable compare */
	reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	/* 
	 * Clean up the Match field (3..0), writing 1 to this bit
	 * would clean the interrupt for the respective channel.
	 * So do this for only the required channel.
	 */
	reg &= ~KONA_GPTIMER_STCS_TIMER_MATCH_MASK;
	reg |= 1 << (kt->ch_num + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	reg |= (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);

	spin_unlock_irqrestore (&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_set_match_start);

/*
 * kona_timer_stop  - Stop the timer.
 *
 * kt - The timer context to be stopped.
 */
int kona_timer_stop (struct kona_timer* kt)
{
	struct kona_timer_module *ktm;
	unsigned long flags;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave (&ktm->lock, flags);

	__disable_channel(ktm->reg_base, kt->ch_num);

	spin_unlock_irqrestore (&ktm->lock, flags);
	return 0;
}
EXPORT_SYMBOL(kona_timer_stop);

/*
 * kona_timer_free - Release the timer, after this call the timer can be used
 * again by others.
 *
 * kt - Timer context to be freed.
 */
int kona_timer_free (struct kona_timer* kt)
{
	struct kona_timer_module *ktm;
	unsigned long flags, i;

	if (NULL == kt)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave (&ktm->lock, flags);

	__disable_channel(ktm->reg_base, kt->ch_num);
	kt->busy = 0;

	/* 
	 * Check if all the channels in this module are free, if yes
	 * then move the state of the timer module so that IF NEEDED
	 * the rate can be re-configured.
	 */
	kt = ktm->pkt;
	for (i=0;i<NUM_OF_CHANNELS;i++,kt++) {
		if (kt->busy == 1)
	        	break;
	}

	/* All the timers are free (NOT BUSY) so move the 
	 * timer module's state
	 */
	if (i == NUM_OF_CHANNELS) 
		ktm->cfg_state = CONFIGURED_FREE;	

	spin_unlock_irqrestore (&ktm->lock, flags);

	return 0;
}
EXPORT_SYMBOL(kona_timer_free);

/*
 * kona_timer_get_counter - Read the counter register of the timer 
 *
 * kt - Timer context to be freed.
 * msw - pointer to the Most Significant Word (32 bits) 
 * lsw - pointer to the Leas Significant Word (32 bits) 
 */
int kona_timer_get_counter(struct kona_timer* kt, unsigned long *msw, 
	unsigned long *lsw)
{
	struct kona_timer_module *ktm;
	unsigned long flags;

	if (NULL == kt || NULL == msw || NULL == lsw)
		return -1;

	ktm = kt->ktm;
	spin_lock_irqsave (&ktm->lock, flags);

	__get_counter(ktm->reg_base, msw, lsw);

	spin_unlock_irqrestore (&ktm->lock, flags);
	return 0;
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
	spin_lock_irqsave (&ktm->lock, flags);

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

	spin_unlock_irqrestore (&ktm->lock, flags);
	return 0;
}
EXPORT_SYMBOL(kona_timer_disable_and_clear);

/* Local static functions */
static struct kona_timer_module * __get_timer_module(char *name)
{
	int i;

	for (i=0;i<NUM_OF_TIMER_MODULES;i++)
		if (!strcmp(name,timer_module_list[i].name))
			return &timer_module_list[i];
	return 0;
}

static int  __config_slave_timer_clock(enum timer_rate rt)
{
	void __iomem *slaveClockMgr_regs = IOMEM(KONA_SLV_CLK_VA);
#ifdef CONFIG_PERIPHERAL_TIMER_FIX
	void __iomem *rootClockMgr_regs = IOMEM(KONA_ROOT_CLK_VA);
#endif
#ifndef CONFIG_ARCH_SAMOA
	u32 val, old_enable;
#endif
	u32 mask, rate_val;

	/* Slave timer only supports 32KHz and 1MHz */
	if (rt == KHZ_32)
		rate_val = 1;
	else if (rt == MHZ_1)
		rate_val = 0;
	else
		return -1;			

	/* Adjust clock source to 1Mhz */
#ifdef CONFIG_ARCH_ISLAND
	/* unlock slave clock manager */
	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = val & 0x1; 
	val &= 0x80000000;
	val |= 0xA5A500 | 0x1;
	writel(val, slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/* set the value */
	mask = IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs + IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, 
		mask, IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT, 0);


	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET);
	writel(val | (1 << IKPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT), 
		slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET);

	while(readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET) & 
			(1 << IKPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT))
		;

	/* restore slave clock manager */
	val = readl(slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	val &= 0x80000000;
	val |= 0xA5A500;
	val |= old_enable & 0x1;
	writel(val, slaveClockMgr_regs + IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
#elseif CONFIG_ARCH_RHEA
	/* unlock slave clock manager */
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	old_enable = val & 0x1; 
	val &= 0x80000000;
	val |= 0xA5A500 | 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/* set the value */
	mask = KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs + KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, 
		mask, KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
		rate_val);

	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);
	writel(val | (1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT), 
		slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET);

	while(readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_DIV_TRIG_OFFSET) & 
			(1 << KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_SHIFT))
		;

	/* restore slave clock manager */
	val = readl(slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
	val &= 0x80000000;
	val |= 0xA5A500;
	val |= old_enable & 0x1;
	writel(val, slaveClockMgr_regs + KPS_CLK_MGR_REG_WR_ACCESS_OFFSET);
#else
	/* set the value */
	mask = KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK;
	kona_set_reg_field(slaveClockMgr_regs + KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET, 
		mask, KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
		rate_val);
#endif

#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
	/* 
	 * This fix is only for 1 MHz since the clock source for
	 * 1MHz is from frac_1M. The 32KHz clock is from PMU and is always ON.
	 */
	if (rt == MHZ_1) {
		/* unlock root clock manager */
		val = readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
		old_enable = val & 0x1;	
		val &= 0x80000000;
		val |= 0xA5A500 | 0x1;
		writel(val, rootClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
		
		/* fix up frac 1m divisor */
		val = readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_DIV_OFFSET);
		writel((val & (~ROOT_CLK_MGR_REG_FRAC_1M_DIV_FRAC_1M_DIV_MASK)) | 0x250000, 
			rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_DIV_OFFSET);
		
		/* enable trigger override */
		val = readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_OFFSET);
		writel(val | (1 << ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_FRAC_1M_TRIGGER_OVERRIDE_SHIFT), 
			rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OVERRIDE_OFFSET);
		
		/* trigger */
		val = readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET);
		writel(val | (1 << ROOT_CLK_MGR_REG_FRAC_1M_TRG_FRAC_1M_TRIGGER_SHIFT), 
			rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET);
		
		/* wait until completes */
		while(readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_FRAC_1M_TRG_OFFSET) & 
			  (1 << ROOT_CLK_MGR_REG_FRAC_1M_TRG_FRAC_1M_TRIGGER_SHIFT))
			  ;
		
		/* disable access to root clock manager */
		val = readl(rootClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
		val &= 0x80000000;
		val |= 0xA5A500;
		writel(val, slaveClockMgr_regs + ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET);
	}
#endif
	return 0;
}

static int  __config_aon_hub_timer_clock(unsigned int val)
{
	/*
	 * The source of all the 3 clocks 32KHz, 1MHz and 19.5MHz
	 * are ON by default (PPL0 (19.5) is configured by the boot loader
	 * 32KHz is from PMU and 1MHz (Frac_1M) is from crystal clock
	 * so all are ON
	 */
	void __iomem *reg_base = IOMEM(KONA_AON_CLK_VA);
	unsigned reg;

	/* 
	 * Assumed that the timer rate enum has 
	 * 32KHz - 0, 1MHz - 1 and 19.5MHz -2 
	 */
	 reg = readl(reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);
	 reg &= ~KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK;
	 reg |= val;
	 writel(val, reg_base + KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET);

	/* 
	 * Again, assuming that we need not perform gating here (On reset
	 * the clocks are enabled). Once the clock manager code is up it 
	 * will take care of disabling and enabling it.
	 * And the appropriate function of the clock manager will be 
	 * called from here.
	 */
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
	reg = readl(reg_base + KONA_GPTIMER_STCS_OFFSET);
	reg &= ~(1 << (ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg,reg_base + KONA_GPTIMER_STCS_OFFSET);
}

static inline void __get_counter (void __iomem *reg_base, unsigned long *msw,
	unsigned long *lsw)
{
	/* Read 64-bit Now while re-programming the match register the
	 * addition 
	 * operation will cause the same overflow which will be matched next
	 * time
	 * properly when the LSB matches this new value.ree running counter
	 * 1. Read hi-word
	 * 2. Read low-word
	 * 3. Read hi-word again
	 * 4.1 
	 * 	if new hi-word is not equal to previously read hi-word, then
	 * 	start from #1
	 * 4.2
	 * 	if new hi-word is equal to previously read hi-word then stop.
	 */
	while (1) {
		*msw = readl(reg_base + KONA_GPTIMER_STCHI_OFFSET);
		*lsw = readl(reg_base + KONA_GPTIMER_STCLO_OFFSET);
		if (*msw == readl(reg_base + KONA_GPTIMER_STCHI_OFFSET))
			break;
	}
}

static inline int irq_to_ch (int irq)
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
 * 2) The match register is compared against the 32 least significant bits of the
 *    free running counter values
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
	int    ch_num;
	void __iomem *match_reg;
	unsigned long reg;

	ktm = (struct kona_timer_module *)dev_id;
	ch_num = irq_to_ch(irq);

	if ( (ktm == NULL) || (ch_num == -1)) {
		pr_err("kona_timer_isr: Invalid dev_id or irq \r\n");
		return IRQ_HANDLED;
	}

	kt = ktm->pkt + ch_num;

	/* First clear and disable the interrupt */
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
		/* Now re-program the match register */
		match_reg = ktm->reg_base + KONA_GPTIMER_STCM0_OFFSET +	(kt->ch_num *4);
		writel (readl(match_reg)+kt->cfg.reload, match_reg);

		/* Re-enable the interrupt */
		reg = readl(ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
		reg |= (1 << (kt->ch_num + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
		writel(reg, ktm->reg_base + KONA_GPTIMER_STCS_OFFSET);
	}

	/* Invoke the call back, if any */
	if (kt->cfg.cb != NULL)
		(*kt->cfg.cb)(kt->cfg.arg);

	return IRQ_HANDLED;
}
