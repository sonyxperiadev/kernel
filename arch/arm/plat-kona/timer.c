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

#include <asm/smp_twd.h>
#include <asm/mach/time.h>
#include <mach/io.h>
#include <mach/io_map.h>
#include <mach/kona_timer.h>
#include <mach/timer.h>
#include <mach/rdb/brcm_rdb_glbtmr.h>

static struct kona_timer *gpt_evt = NULL;
static struct kona_timer *gpt_src = NULL;
static void __iomem*	proftmr_regbase = IOMEM(KONA_PROFTMR_VA);

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
#ifdef CONFIG_GP_TIMER_CLOCK_OFF_FIX
   {
	  volatile unsigned int i;
	  kona_timer_disable_and_clear(gpt_evt);
	  for (i=0; i<1800; i++)
	  {
	  }
   }
#endif   	
	kona_timer_set_match_start(gpt_evt,clc);
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
		kona_timer_disable_and_clear(gpt_evt);
	}
}

static cycle_t gptimer_clksrc_read (struct clocksource *cs)
{
	unsigned long msw, lsw;
	cycle_t	count = 0;

	kona_timer_get_counter (gpt_src, &msw, &lsw);
	count = ((cycle_t)msw << 32) | (cycle_t)lsw;
	return count;
}

static struct clock_event_device clockevent_gptimer = {
	.name		= "gpt_event_1",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.set_next_event	= gptimer_set_next_event,
	.set_mode	= gptimer_set_mode
};

static struct clocksource clksrc_gptimer = {
	.name		= "gpt_source_2",
	.rating		= 200,
	.read		= gptimer_clksrc_read,
	.mask		= CLOCKSOURCE_MASK(64), /* Kona timers have 64 bit counters */
	.shift		= 16, /* Fix shift as 16 and calculate mult based on this during init */
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
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

static void __init gptimer_clocksource_init(void)
{
	clksrc_gptimer.mult = clocksource_hz2mult(CLOCK_TICK_RATE, 
		clksrc_gptimer.shift);
	clocksource_register(&clksrc_gptimer);
	return;
}

static int gptimer_interrupt_cb(void *dev)
{
	struct clock_event_device *evt = (struct clock_event_device *)dev;
	evt->event_handler(evt);
	return 0;
}

static void profile_timer_init(void __iomem *base)
{
	uint32_t reg;

	/* Reset profile/global timer */
	writel(0, base + GLBTMR_GLOB_CTRL_OFFSET);

	/* Clear pending interrupts */
	reg = readl(base + GLBTMR_GLOB_STATUS_OFFSET);
	reg &= ~(GLBTMR_GLOB_STATUS_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_STATUS_EVENT_G_SHIFT);
	writel(reg, base + GLBTMR_GLOB_STATUS_OFFSET);

	/* Enable profile timer now with
	 * prescaler = 0, so timer freq = A9 PERIPHCLK 
	 * IRQ disabled
	 * Comapre disabled
	 */

	reg = readl(base + GLBTMR_GLOB_CTRL_OFFSET);
	reg &= ~(GLBTMR_GLOB_CTRL_RESERVED_MASK);
	reg |= (1 << GLBTMR_GLOB_CTRL_TIMER_EN_G_SHIFT);
	writel(reg, base + GLBTMR_GLOB_CTRL_OFFSET);
}

static void
profile_timer_get_counter(void __iomem *base, uint32_t *msw, uint32_t *lsw)
{
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

static void __init timers_init(struct gp_timer_setup *gpt_setup)
{
	struct timer_ch_cfg evt_tm_cfg;

	kona_timer_modules_init ();
	kona_timer_module_set_rate(gpt_setup->name, gpt_setup->rate);
	
	/* Initialize Event timer */
	gpt_evt = kona_timer_request(gpt_setup->name, gpt_setup->ch_num);
	if (gpt_evt == NULL) {
		pr_err("timers_init: Unable to get GPT timer for event\r\n");
	}

	pr_info("timers_init: === SYSTEM TIMER NAME: %s CHANNEL NUMBER %d \
	RATE (0-32KHz, 1-1MHz) %d \r\n",gpt_setup->name, 
	gpt_setup->ch_num, gpt_setup->rate);

	evt_tm_cfg.mode =  MODE_ONESHOT;
	evt_tm_cfg.arg = &clockevent_gptimer;
	evt_tm_cfg.cb = gptimer_interrupt_cb;

	kona_timer_config(gpt_evt, &evt_tm_cfg);

	gptimer_set_next_event((CLOCK_TICK_RATE / HZ), NULL);

	/* 
	 * IMPORTANT
	 * Note that we don't want to waste a channel for clock source. In Kona
	 * timer module by default there is a counter that keeps counting
	 * irrespective of the channels. So instead of implementing a periodic
	 * timer using a channel (which in the HW is not peridoic) we can
	 * simply read the counters of the timer that is used for event and
	 * send it for source. The only catch is that this timer should not be
	 * stopped by PM or any other sub-systems.
	 */
	 gpt_src = gpt_evt;

	/* Initialize the profile timer */
	proftmr_regbase = IOMEM(KONA_PROFTMR_VA);
	profile_timer_init(proftmr_regbase);

	return ;
}

void __init kona_timer_init(struct gp_timer_setup *gpt_setup)
{
	timers_init(gpt_setup);
	gptimer_clocksource_init();
	gptimer_clockevents_init();
	gptimer_set_next_event((CLOCK_TICK_RATE / HZ), NULL);
}


/* Profile timer implementations */

/* 
 * TODO: The below profile timer code is retained as it is.
 * The clock manager is not up yet, once its ready read the 
 * correct frequency from it.  
 *
 * Right now Global timer runs at 5000000 on FPGA (A9 PERIPHCLK)
 * Ideally, this should be derived by timer.prof_clk and
 * prescaler.
 */

#define GLOBAL_TIMER_FREQ_HZ	(5000000) /* For FPGA only, (temp)*/
timer_tick_rate_t timer_get_tick_rate(void)
{
	uint32_t prescaler;

	prescaler = readl(proftmr_regbase + GLBTMR_GLOB_CTRL_OFFSET);
	prescaler &= GLBTMR_GLOB_CTRL_PRESCALER_G_MASK;
	prescaler >>= GLBTMR_GLOB_CTRL_PRESCALER_G_SHIFT;

	return (GLOBAL_TIMER_FREQ_HZ / (1 + prescaler)); 
}

timer_tick_count_t timer_get_tick_count(void)
{
	uint32_t msw, lsw;
	uint64_t tick;

	profile_timer_get_counter(proftmr_regbase, &msw, &lsw);

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

EXPORT_SYMBOL(timer_get_tick_count);
EXPORT_SYMBOL(timer_ticks_to_msec);
EXPORT_SYMBOL(timer_get_tick_rate);
EXPORT_SYMBOL(timer_get_msec);
