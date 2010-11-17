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
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/smp_twd.h>
#include <asm/mach/time.h>

#include <mach/timer.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/irqs.h>

#define SYS_TIMER_NUM	(0)

struct kona_timers {
	int gptmr_irq;
	int proftmr_irq;
	/* Not used right now */
	struct clk *gptmr_clk;
	struct clk *proftmr_clk; 
	void __iomem *gptmr_regs;
	void __iomem *proftmr_regs;
};

struct kona_timers timers;

/* We use the peripheral timers for system tick, the cpu global timer for
 * profile tick
 */
static void gptimer_disable_and_clear(void *gptimer_regs)
{
	uint32_t reg;
	void __iomem *base = IOMEM(gptimer_regs);

	/* clear and disable gptimer interrupts
	* We are using compare/match register 0 for
	* our system interrupts
	*/
	reg = 0;

	/* Clear compare (0) interrupt */
	reg |= 1 << (SYS_TIMER_NUM + KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT);
	/* disable compare */
	reg &= ~(1 << (SYS_TIMER_NUM + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));

	writel(reg, base + KONA_GPTIMER_STCS_OFFSET);

}

static void
gptimer_get_counter(void *gptimer_base, uint32_t *msw, uint32_t *lsw)
{
	void __iomem *base = IOMEM(gptimer_base);

	/* Read 64-bit free running counter
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
		*msw = readl(base + KONA_GPTIMER_STCHI_OFFSET);
		*lsw = readl(base + KONA_GPTIMER_STCLO_OFFSET);
		if (*msw == readl(base + KONA_GPTIMER_STCHI_OFFSET))
			break;
	}

	return;
}

static void profile_timer_init(void)
{
	uint32_t reg;

	/* Reset profile/global timer */
	writel(0, timers.proftmr_regs + GLBTMR_GLOB_CTRL_OFFSET);

	/* Clear pending interrupts */
	reg = readl(timers.proftmr_regs + GLBTMR_GLOB_STATUS_OFFSET);
	reg &= ~(GLBTMR_GLOB_STATUS_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_STATUS_EVENT_G_SHIFT);
	writel(reg, timers.proftmr_regs + GLBTMR_GLOB_STATUS_OFFSET);

	/* Enable profile timer now with
	 * prescaler = 0, so timer freq = A9 PERIPHCLK 
	 * IRQ disabled
	 * Comapre disabled
	 */

	reg = readl(timers.proftmr_regs + GLBTMR_GLOB_CTRL_OFFSET);
	reg &= ~(GLBTMR_GLOB_CTRL_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_CTRL_TIMER_EN_G_SHIFT);
	writel(reg, timers.proftmr_regs + GLBTMR_GLOB_CTRL_OFFSET);
}

static void
profile_timer_get_counter(void *proftimer_base, uint32_t *msw, uint32_t *lsw)
{
	void __iomem *base = IOMEM(proftimer_base);

	/* Read 64-bit free running counter
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
		*msw = readl(base + GLBTMR_GLOB_HI_OFFSET);
		*lsw = readl(base + GLBTMR_GLOB_LOW_OFFSET);
		if (*msw == readl(base + GLBTMR_GLOB_HI_OFFSET))
			break;
	}

	return;
}

static void __init timers_init(void)
{
	/* Setup IRQ numbers */
	timers.gptmr_irq = BCM_INT_ID_PERIPH_TIMERS1;
	timers.proftmr_irq = -1; /* Not used */

	/* Setup IO addresses */
	timers.gptmr_regs = IOMEM(KONA_SYSTMR_VA);
	timers.proftmr_regs = IOMEM(KONA_PROFTMR_VA);

	gptimer_disable_and_clear(timers.gptmr_regs);
	profile_timer_init();
}

static int gptimer_set_next_event(unsigned long clc,
		struct clock_event_device *unused)
{
	/* gptimer (0) is disabled by the timer interrupt already
	 * so, here we reload the next event value and re-enable
	 * the timer
	 *
	 * This way, we are potentially losing the time between
	 * timer-interrupt->set_next_event. CPU local timers, when
	 * they come in should get rid of skew 
	 */

	uint32_t lsw, msw;
	uint32_t reg;

	gptimer_get_counter(timers.gptmr_regs, &msw, &lsw);

	/* Load the "next" event tick value */
	writel(lsw + clc,
		timers.gptmr_regs+ KONA_GPTIMER_STCM0_OFFSET + (SYS_TIMER_NUM * 4));

	/* Enable compare */
	reg = readl(timers.gptmr_regs + KONA_GPTIMER_STCS_OFFSET);
	reg |= (1 << (SYS_TIMER_NUM + KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT));
	writel(reg, timers.gptmr_regs + KONA_GPTIMER_STCS_OFFSET);

	return 0;
}

static void gptimer_set_mode(enum clock_event_mode mode,
				   struct clock_event_device *unused)
{
	switch (mode) {
	case CLOCK_EVT_MODE_ONESHOT:
		/* by default mode is one shot don't do any thing */
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		gptimer_disable_and_clear(timers.gptmr_regs);
	}
}

static struct clock_event_device clockevent_gptimer = {
	.name		= "gptimer 1",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.set_next_event	= gptimer_set_next_event,
	.set_mode	= gptimer_set_mode
};

static void __init gptimer_clockevents_init(void)
{
	clockevent_gptimer.mult = div_sc(CLOCK_TICK_RATE, NSEC_PER_SEC,
						clockevent_gptimer.shift);

	clockevent_gptimer.max_delta_ns =
		clockevent_delta2ns(0xffffffff, &clockevent_gptimer);

	clockevent_gptimer.min_delta_ns =
		clockevent_delta2ns(6, &clockevent_gptimer);

	clockevent_gptimer.cpumask = cpumask_of(0);
	clockevents_register_device(&clockevent_gptimer);
}

static irqreturn_t gptimer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &clockevent_gptimer;

	gptimer_disable_and_clear(timers.gptmr_regs);
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static struct irqaction gptimer_irq = {
	.name		= "Kona Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= gptimer_interrupt,
};

static void __init kona_timer_init(void)
{
#ifdef CONFIG_LOCAL_TIMERS
    twd_base = __io(KONA_PTIM_VA);
#endif	
	timers_init();
	gptimer_clockevents_init();
	setup_irq(timers.gptmr_irq, &gptimer_irq);
	gptimer_set_next_event((CLOCK_TICK_RATE / HZ), NULL);
}

/* Right now Global timer runs at 5000000 on FPGA (A9 PERIPHCLK)
 * Ideally, this should be derived by timer.prof_clk and
 * prescaler
 */

#define GLOBAL_TIMER_FREQ_HZ	(5000000) /* For FPGA only, (temp) */
timer_tick_rate_t timer_get_tick_rate(void)
{
	uint32_t prescaler;

	prescaler = readl(timers.proftmr_regs + GLBTMR_GLOB_CTRL_OFFSET);
	prescaler &= GLBTMR_GLOB_CTRL_PRESCALER_G_MASK;
	prescaler >>= GLBTMR_GLOB_CTRL_PRESCALER_G_SHIFT;

	return (GLOBAL_TIMER_FREQ_HZ / (1 + prescaler)); 
}

timer_tick_count_t timer_get_tick_count(void)
{
	uint32_t msw, lsw;
	uint64_t tick;

	profile_timer_get_counter(timers.proftmr_regs, &msw, &lsw);

	tick = (((uint64_t)msw << 32) | ((uint64_t)lsw));

	return (*(uint32_t *)(&tick));
}

timer_msec_t timer_ticks_to_msec(timer_tick_count_t ticks)
{
	return (ticks / (timer_get_tick_rate() / 1000));
}

timer_msec_t timer_get_msec(void)
{
	return timer_ticks_to_msec(timer_get_tick_count());
}

struct sys_timer kona_timer = {
	.init	= kona_timer_init,
};

EXPORT_SYMBOL(timer_get_tick_count);
EXPORT_SYMBOL(timer_ticks_to_msec);
EXPORT_SYMBOL(timer_get_tick_rate);
EXPORT_SYMBOL(timer_get_msec);

