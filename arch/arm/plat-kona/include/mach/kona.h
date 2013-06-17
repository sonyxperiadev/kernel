#ifndef __PLAT_KONA_H
#define __PLAT_KONA_H

#include <asm/mach/time.h>

extern struct sys_timer kona_timer;

extern void __init kona_init_irq(void);

#endif /* __PLAT_KONA_H */
