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

#ifndef __PLAT_KONA_TIMER_H
#define __PLAT_KONA_TIMER_H

/* There is 6 clock cycles delay in HUB Timer by ASIC limitation.
 */
#define MIN_KONA_DELTA_CLOCK     6

/* On 32-bit mod timer, using large or max value 0xffffffff may trigger early
 * timer interrupt. It has to be limited to avoid timer misbehavior.
 */
#define MAX_KONA_COUNT_CLOCK  0x7FFFFFFF

/* Timer module specific data structures */
struct kona_timer;

/* Channel specific data structures */
typedef int (*intr_callback) (void *data);

enum timer_mode {
	MODE_PERIODIC = 0,
	MODE_ONESHOT,
};

struct timer_ch_cfg {
	void *arg;
	enum timer_mode mode;
	intr_callback cb;
	unsigned long reload;	/* Holds the reload value in 
				 * case of periodic timers 
				 */
};

struct kona_timer {
	int ch_num;
	int busy;
	struct timer_ch_cfg cfg;
	int irq;
	struct kona_timer_module *ktm;
	unsigned long expire;
#ifdef CONFIG_KONA_TIMER_DEBUG
	unsigned long nr_5;
	unsigned long nr_10;
	unsigned long nr_50;
	unsigned long nr_100;
	unsigned long nr_500;
	unsigned long nr_500_plus;
	unsigned long nr_early_expire;
	unsigned long nr_wrong_interrupt;
#endif
};

#ifdef CONFIG_ARCH_JAVA
#define NUM_OF_TIMER_MODULES	(3)
#else
#define NUM_OF_TIMER_MODULES	(2)
#endif
#define NUM_OF_CHANNELS		(4)

/* Timer Module related APIs */

/*
 * USAGE OF THIS APIs
 * ------------------
 * From the board specific file, the kona_timer_modules_init will be called 
 * After that it will call the init function of timer.c and will pass the
 * following information in a platform structure
 * 1) Timer name to be used as system timer
 * 2) Frequency to be configured for system timer
 * 3) The channel of the timer to use as clock source (optional)
 * 4) The channel of the timer to use as clock event (optional)
 *
 * from the init function of timer.c kona_timer_modules_set_rate will be called
 * to set the system timer frequency. 
 * Then the appropriate channels would be setup for clock source/event by 
 * calling kona_timer_request()
 */

/*
 *  kona_timer_modules_init - Initialize the data structures
 *  that depcits the Kona timer modules
 */
int kona_timer_modules_init(void);

/*
 * Obtain Kona timer clocks and store in its internal data structure
 */
void kona_timer_clk_setup(void);

/*
 * kona_timer_module_set_rate - Set the speed in which a timer module should count
 * name - Name of the Timer to configure
 * rate - Speed 
 */
int kona_timer_module_set_rate(char *name, unsigned int rate);

/* 
 * kona_timer_module_get_rate - Get the speed in which a timer module is running
 * name - Name of the Timer module 
 */
int kona_timer_module_get_rate(char *name);

/* Channel/Timer related APIs */
/*
 *  kona_timer_request - Get access to a channel in the given timer
 *  name - Name of the Timer module
 *  channel - Channel number requested. If this is -1 then by default
 *            the next available channel will be returned
 */
struct kona_timer *kona_timer_request(char *name, int channel);

/*
 *  kona_timer_config - Configure the following parameters of the timer
 *  1) mode of the timer - periodic/one shot
 *  2) call back function that will be called from the ISR context
 *  3) context to be passed back in the call back function
 *
 *  kt - Kona timer context (returned by kona_timer_request())
 *  pcfg - pointer to the configuration structure
 */
int kona_timer_config(struct kona_timer *kt, struct timer_ch_cfg *pcfg);

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
int kona_timer_set_match_start(struct kona_timer *kt, unsigned long load);

/*
 * kona_timer_free - Read the counter register of the timer 
 *
 * kt - Timer context to be freed.
 */
unsigned int kona_timer_get_counter(struct kona_timer *kt);
/*
 * kona_timer_disable_and_clear - Disable the timer and clear the 
 * interrupt
 *
 * kt - Timer context to be freed.
 */
int kona_timer_disable_and_clear(struct kona_timer *kt);

/*
 * kona_timer_stop  - Stop the timer.
 *
 * kt - The timer context to be stopped.
 */
int kona_timer_stop(struct kona_timer *kt);

/*
 * kona_timer_suspend - Suspend the timer by shutting off its clock source.
 *
 * kt - Timer context.
 *
 */
int kona_timer_suspend(struct kona_timer *kt);

/*
 * kona_timer_resume - Resume the timer by re-enabling its clock source.
 *
 * kt - Timer context.
 *
 */
int kona_timer_resume(struct kona_timer *kt);

/*
 * kona_timer_free - Release the timer, after this call the timer can be used
 * again by others.
 *
 * kt - Timer context to be freed.
 */
int kona_timer_free(struct kona_timer *kt);

/*
 * kona_hubtimer_get_counter - Returns the counter value of aon hub timer
 */
unsigned long kona_hubtimer_get_counter(void);

/*
 * kona_coretimer_get_counter - Returns the counter value of aon hub timer
 */

unsigned long kona_coretimer_get_counter(void);

/*
 * kona_timer_save_state - Saves and optionally print aon hub timer state
 */
void kona_timer_save_state(bool print_state);

/*
 * kona_slavetimer_get_counter - Returns the counter value of slave timer
 */
unsigned long kona_slavetimer_get_counter(void);

#endif /* __PLAT_KONA_TIMER_H */
