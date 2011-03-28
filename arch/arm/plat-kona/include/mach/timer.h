#ifndef __PLAT_TIMER_H
#define __PLAT_TIMER_H

#ifdef __KERNEL__

typedef unsigned int timer_tick_count_t;
typedef unsigned int timer_tick_rate_t;
typedef unsigned int timer_msec_t;


enum gp_timer_rate {
	GPT_KHZ_32 = 0,
	GPT_MHZ_1,
};

/**
 * timer configuration identifying the timer to use
 * as system timer (GP Timer)
 */
 struct gp_timer_setup {
	char *name;
	int ch_num;
	enum gp_timer_rate rate;
 };

void kona_timer_init (struct gp_timer_setup *gpt);

timer_tick_count_t timer_get_tick_count(void);
timer_tick_rate_t timer_get_tick_rate(void);
timer_msec_t timer_get_msec(void);
timer_msec_t timer_ticks_to_msec(timer_tick_count_t ticks);

#endif /* __KERNEL__ */
#endif /* __PLAT_TIMER_H */
