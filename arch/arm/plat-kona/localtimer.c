/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <linux/percpu.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/smp_twd.h>
#include <asm/localtimer.h>
#include <mach/kona_timer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

/*
 * There are two local timer implementation.
 * One to use the Kona AON timer itself as in Hawaii.
 * The other one is if we choose to use the architecture
 * timer (only available in A7 and friends) as in case of Java
 *
 * Note that from 3.9 onwards the world is moving away from
 * local timers. This is done to just be in sync with hawaii
 * But doing away with local timer is simple. We have
 * commented registering to the CPU hotplug notification
 * in arm_arch_timer.c. Just uncommenting that would do the trick.
 */

#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
#include <clocksource/arm_arch_timer.h>

int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	return arch_timer_setup(evt);
	return 0;
}

void local_timer_stop(struct clock_event_device *evt)
{
	arch_timer_stop(evt);
}

#else  /* CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER is not defined */

#ifndef CONFIG_HAVE_ARM_TWD
#define TICK_TIMER_NAME TIMER_NAME"-timer"
#if defined (CONFIG_ARCH_HAWAII)
#define TICK_TIMER_OFFSET 2
#elif defined (CONFIG_ARCH_JAVA)
#define TICK_TIMER_OFFSET 0
#endif

struct kona_td {
	struct kona_timer *kona_timer;
	bool allocated;
};

static DEFINE_PER_CPU(struct kona_td, percpu_kona_td);

struct kona_timer *get_timer_ptr(char *name, int chan);
static int kona_tick_set_next_event(unsigned long cycles,
				    struct clock_event_device *evt)
{
	struct kona_td *kona_td;

	kona_td = (struct kona_td *)&__get_cpu_var(percpu_kona_td);

	if (likely(cpu_online(smp_processor_id())))
		kona_timer_set_match_start(kona_td->kona_timer, cycles);

	return 0;
}

static int kona_tick_interrupt_cb(void *dev)
{
	struct clock_event_device *evt = (struct clock_event_device *)dev;
	evt->event_handler(evt);
	return 0;
}

static void kona_tick_set_mode(enum clock_event_mode mode,
			       struct clock_event_device *evt)
{
	struct kona_td *kona_td;

	kona_td = (struct kona_td *)&__get_cpu_var(percpu_kona_td);
	switch (mode) {
	case CLOCK_EVT_MODE_ONESHOT:
		/* Default is already ONESHOT. And that's the only feature we
		 * support.
		 */
		break;

	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		kona_timer_disable_and_clear(kona_td->kona_timer);
		break;
	}
}

#ifdef CONFIG_LOCAL_TIMERS

/*
 * Setup the local clock events for a CPU.
 */
int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	unsigned int cpu = smp_processor_id();
	struct kona_td kona_td;
	struct timer_ch_cfg config;

	pr_info("local_timer_setup called for %d\n", cpu);
	/* allocate an AON timer channel as local tick timer
	 */
	kona_td = (struct kona_td)__get_cpu_var(percpu_kona_td);

	if (!kona_td.allocated) {
		kona_td.kona_timer =
		    kona_timer_request(TICK_TIMER_NAME,
				       TICK_TIMER_OFFSET + cpu);
		if (kona_td.kona_timer) {
			kona_td.allocated = true;
		} else { /* kona_td.kona_timer is already allocated, hence
			    we get the pointer and reuse the same. This is
			    done to ensure we don't use an extra channel for
			    timer event before local timer comes up
			    */
			kona_td.kona_timer = get_timer_ptr(TICK_TIMER_NAME,
					TICK_TIMER_OFFSET + cpu);
			if (!kona_td.kona_timer) {
				pr_err("%s: Failed to allocate %s channel %d"
					"as CPU %d local tick device\n",
					__func__,
				       TICK_TIMER_NAME,
				       TICK_TIMER_OFFSET + cpu, cpu);
				return -ENXIO;
			} else {
				kona_td.allocated = true;
			}
		}
	}

	/*
	 * In the future: The following codes should be one time configuration
	 */
	config.mode = MODE_ONESHOT;
	config.arg = evt;
	config.cb = kona_tick_interrupt_cb;
	kona_timer_config(kona_td.kona_timer, &config);

	irq_set_affinity(kona_td.kona_timer->irq, cpumask_of(cpu));

	evt->name = "local_timer";
	evt->cpumask = cpumask_of(cpu);
	evt->irq = kona_td.kona_timer->irq;
	evt->set_next_event = kona_tick_set_next_event;
	evt->set_mode = kona_tick_set_mode;
	evt->features = CLOCK_EVT_FEAT_ONESHOT;
	evt->rating = 250;
	evt->shift = 32;
	evt->mult = div_sc(CLOCK_TICK_RATE, NSEC_PER_SEC, evt->shift);
	evt->max_delta_ns = clockevent_delta2ns(MAX_KONA_COUNT_CLOCK, evt);
	/* There is MIN_KONA_DELTA_CLOCK clock cycles delay in HUB Timer by
	 * ASIC limitation. When min_delta_ns set N, real requested load value
	 * in hrtimer becomes N - 1. So add 1 to be MIN_DELTA_CLOCK
	 */
	evt->min_delta_ns = clockevent_delta2ns(MIN_KONA_DELTA_CLOCK + 1, evt);

	per_cpu(percpu_kona_td, cpu) = kona_td;

	clockevents_register_device(evt);
	return 0;
}

/*
 * percpu_timer_setup() calls local_timer_setup() which enable platform specific
 * timers
 *	BCM_INT_ID_HUB_TIMERS3 (ID 83, aon-timer, channel 2) for CPU0
 *	BCM_INT_ID_HUB_TIMERS4 (ID 82, aon-timer, channel 3) for CPU1
 * When suspending, BCM_INT_ID_HUB_TIMERS4 (ID 82) is migrated to CPU0 and
 * unexpectedly wake up system.
 *
 * We need a local_timer_stop() function to disable interrupt for CPU1.
 * Porting the simplest structure from kernel 3.5.
 */
void local_timer_stop(struct clock_event_device *evt)
{
	struct kona_td kona_td;

	evt->set_mode(CLOCK_EVT_MODE_UNUSED, evt);

	kona_td = (struct kona_td)__get_cpu_var(percpu_kona_td);

	if (kona_td.allocated) {
		/*
		 * Disable interrupt (by clear COMPARE_ENABLE bit in Hub Timer)
		 */
		kona_timer_stop(kona_td.kona_timer);
	}
}

inline int local_timer_ack(void)
{
	return 1;
}

#endif /* CONFIG_LOCAL_TIMERS */
#endif

#endif /* CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER */
