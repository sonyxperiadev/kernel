#ifndef __PLAT_TIMER_H
#define __PLAT_TIMER_H

#ifdef __KERNEL__

/**
 * timer configuration identifying the timer to use
 * as system timer (GP Timer)
 */
struct gp_timer_setup {
	char *name;
	int ch_num;
	unsigned int rate;
};

void gp_timer_init(struct gp_timer_setup *gpt);

#endif /* __KERNEL__ */
#endif /* __PLAT_TIMER_H */
