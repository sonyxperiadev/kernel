#ifndef __BCM2708_ARMCTRL_H
#define __BCM2708_ARMCTRL_H

extern int __init armctrl_init(void __iomem *base, unsigned int irq_start,
                               u32 armctrl_sources, u32 resume_sources);

#endif
