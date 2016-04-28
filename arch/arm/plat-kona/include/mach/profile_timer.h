#ifndef __PROFILE_TIMER_H
#define __PROFILE_TIMER_H

#ifdef __KERNEL__

typedef unsigned int timer_tick_count_t;
typedef unsigned int timer_tick_rate_t;
typedef unsigned int timer_msec_t;

void profile_timer_init(void __iomem * base);
timer_tick_count_t timer_get_tick_count(void);
timer_tick_rate_t timer_get_tick_rate(void);
timer_msec_t timer_get_msec(void);
timer_msec_t timer_ticks_to_msec(timer_tick_count_t ticks);

#endif /* __KERNEL__ */
#endif /* __PROFILE_TIMER_H */
